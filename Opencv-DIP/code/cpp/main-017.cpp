#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
int main(int argc, char** argv) {
	Mat src, dst;
	int ksize = 0;

	src = imread("D:/vcprojects/images/test1.png");
	if (!src.data) {
		printf("could not load image...\n");
		return -1;
	}

	char INPUT_WIN[] = "input image";
	char OUTPUT_WIN[] = "Custom Blur Filter Result";
	namedWindow(INPUT_WIN, CV_WINDOW_AUTOSIZE);
	namedWindow(OUTPUT_WIN, CV_WINDOW_AUTOSIZE);

	imshow(INPUT_WIN, src);
	
	// Sobel X ����
	// Mat kernel_x = (Mat_<int>(3, 3) << -1, 0, 1, -2,0,2,-1,0,1);
	// filter2D(src, dst, -1, kernel_x, Point(-1, -1), 0.0);

	// Sobel Y ����
	// Mat yimg;
	// Mat kernel_y = (Mat_<int>(3, 3) << -1, -2, -1, 0,0,0, 1,2,1);
	// filter2D(src, yimg, -1, kernel_y, Point(-1, -1), 0.0);

	// ������˹����
	//Mat kernel_y = (Mat_<int>(3, 3) << 0, -1, 0, -1, 4, -1, 0, -1, 0);
	//filter2D(src, dst, -1, kernel_y, Point(-1, -1), 0.0);
	int c = 0;
	int index = 0;
	while (true) {
		c = waitKey(500);
		if ((char)c == 27) {// ESC 
			break;
		}
		ksize = 5 + (index % 8) * 2;
		Mat kernel = Mat::ones(Size(ksize, ksize), CV_32F) / (float)(ksize * ksize);
		filter2D(src, dst, -1, kernel, Point(-1, -1));
		index++;
		imshow(OUTPUT_WIN, dst);	
	}

	// imshow("Sobel Y", yimg);
	return 0;
}