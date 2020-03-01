#include <cstdint>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <map>


struct RGBColor{
    uint8_t R, G, B;
    RGBColor(uint8_t R_, uint8_t G_, uint8_t B_): R(R_), G(G_), B(B_){
    }
    void print(){
        printf("%i %i %i", R, G, B);
    }
};

std::map<uint8_t, int> findFrequencyColor(uint8_t* image_pixels, uint8_t* colorImage, int rows, int cols, std::map<uint8_t, RGBColor>* colormap){
    std::map<uint8_t, int> frequencyColor;
    std::map<uint8_t, int>::iterator it;
    for(int i = 0, j = 0; i < rows*cols; i++, j += 3){
        it = frequencyColor.find(image_pixels[i]);
        if(it != frequencyColor.end()) (it->second)++;
        else {
            frequencyColor.insert({image_pixels[i], 1});
            colormap->insert({image_pixels[i], RGBColor(colorImage[j+2], colorImage[j+1], colorImage[j])});
        }
    }
    return frequencyColor;
}

uint8_t findMaxFromPartMap(std::map<uint8_t, int> frequencyMap, uint8_t startEl, uint8_t finish_el){
    uint8_t max_index = 1;
    int max_value = 0;

    for(int i = startEl; i <= finish_el; i++){
        if(frequencyMap[i] > max_value) {
            max_value = frequencyMap[i];
            max_index = i;
        }
    }
    return max_index;
}

uint8_t* lineFind(uint8_t* image_pixels, int rows, int cols, int threshold){
    uint8_t this_pixel = 0;
    uint8_t* result = new uint8_t[cols*rows];
    for(int i = 1; i < rows-1; i++){
        for(int j = 1; j < cols-1; j++){
            this_pixel = image_pixels[(i*cols)+j];
            result[(i*cols)+j] = 255;
            if(abs(this_pixel - image_pixels[(i*cols)+j-1]) > threshold) result[(i*cols)+j] = 0;
            if(abs(this_pixel - image_pixels[((i - 1)*cols)+j]) > threshold) result[(i*cols)+j] = 0;
        }
    }
    return result;
}

uint8_t findColorInVector(std::vector<uint8_t>* colors, uint8_t color){
    uint8_t findedColor = -1;
    int lastDiff = 99999999;
    for(auto f: *colors){
        if(abs(color - f) < lastDiff) {
            lastDiff = abs(color - f);
            findedColor = f;
        }
    }
    return findedColor;
}

void smoothing(uint8_t* image_pixels, std::vector<uint8_t>* colors, int rows, int cols){
    for(int i = 0; i < rows*cols; i++){
        image_pixels[i] = findColorInVector(colors, image_pixels[i]);
    }
}

void borderSmoothing(uint8_t* image_pixels, int rows, int cols){
    for(int i = 1; i < rows-1; i++){
        for(int j = 1; j < cols-1; j++){
            if(image_pixels[(i * cols) + j] == 0){
                if(image_pixels[(i * cols) + j -1] != 0 && image_pixels[(i * cols) + j + 1] != 0 && image_pixels[((i -1) * cols) + j] != 0 && image_pixels[((i + 1) * cols) + j] != 0 && image_pixels[((i-1) * cols) + j + 1] != 0 && image_pixels[((i-1) * cols) + j - 1] != 0 && image_pixels[((i+1) * cols) + j - 1] != 0 && image_pixels[((i+1) * cols) + j + 1] != 0){
                    image_pixels[(i*cols) + j] = (image_pixels[(i * cols) + j -1] + image_pixels[(i * cols) + j + 1] + image_pixels[((i -1) * cols) + j] + image_pixels[((i + 1) * cols) + j] + image_pixels[((i-1) * cols) + j + 1] + image_pixels[((i-1) * cols) + j - 1] + image_pixels[((i+1) * cols) + j - 1] + image_pixels[((i+1) * cols) + j + 1]) / 8;
                }
            }
        }
    }
}

int main(){
    const int NUM_OF_COLORS = 24;
    cv::Mat image = cv::imread("test4K4.jpg");
    cv::Mat gray_image;
    cv::Mat bluredImage;
    cv::bilateralFilter(image, bluredImage, 32, 75, 75);
    cv::cvtColor(bluredImage, gray_image, cv::COLOR_BGR2GRAY);

    printf("%i x %i x %i \n", gray_image.cols, gray_image.rows, gray_image.channels());

    uint8_t* im_data = gray_image.data;

    std::map<uint8_t, RGBColor> colormap;
    
    auto colorFreq = findFrequencyColor(im_data, bluredImage.data, gray_image.rows, gray_image.cols, &colormap);
    std::vector<uint8_t> frequencyColors;

    auto all_colors = colorFreq.size();
    auto max_color = colorFreq.end()->first;
    for(auto i = 0; i < max_color; i+=max_color/NUM_OF_COLORS){
        auto idx = findMaxFromPartMap(colorFreq, i, i + (max_color/NUM_OF_COLORS) - 1);
        frequencyColors.push_back(idx);
        printf("%i -> %i\n", idx, colorFreq[idx]);
        //colormap[idx].print();
    }


    smoothing(im_data, &frequencyColors, gray_image.rows, gray_image.cols);

    uint8_t* res = lineFind(im_data, gray_image.rows, gray_image.cols,  max_color/(NUM_OF_COLORS));

    auto* blured_data = bluredImage.data;
    //memcpy(im_data, res, gray_image.cols*gray_image.rows* sizeof(uint8_t));
    for(int i = 0; i < gray_image.rows*gray_image.cols; i++){
        if(res[i] == 0) {
            im_data[i] = 0;
        }
        else im_data[i] = 255;
    }

    borderSmoothing(im_data, gray_image.rows, gray_image.cols);

    for(int i = 0, j = 0; i < gray_image.rows*gray_image.cols*3; i+=3, j++){
        if(im_data[j] == 0) {
            blured_data[i] = 0;
            blured_data[i+1] = 0;
            blured_data[i+2] = 0;
        }
    }

    cv::namedWindow("Image", cv::WINDOW_NORMAL);
    cv::imshow("Image", bluredImage);
    cv::resizeWindow("Image", cv::Size(900, 900));
    cv::waitKey(0);
    cv::imwrite("result.png", gray_image);
}
