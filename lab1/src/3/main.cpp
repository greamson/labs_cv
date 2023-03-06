#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "usage: DisplayImage.out <Image_Path>" << std::endl;
        return -1;
    }
   
    Mat image = imread(argv[1], 1);
    Mat dest;
    int coreSize = atoi(argv[2]);

    if (!image.data) {
        std::cout << "No image data" << std::endl;
        return -1;
    }

    GaussianBlur(image, dest, Size(coreSize, coreSize), 1.3);
    imshow("asd", dest);
    imwrite("results/res3_" + std::to_string(coreSize) + ".png", dest);
    waitKey(0);
    return 0;
}