#include <opencv2/opencv.hpp>
#include <iostream>
#include "math.h"

using namespace cv;
int main(int agrc, char** argv) {
	Mat src, dst;
	src = imread("D:/vcprojects/images/cat.jpg");
	if (!src.data) {
		printf("could not load image...");
		return -1;
	}

	char INPUT_WIN[] = "input image";
	char OUTPUT_WIN[] = "sample up";
	namedWindow(INPUT_WIN, CV_WINDOW_AUTOSIZE);
	namedWindow(OUTPUT_WIN, CV_WINDOW_AUTOSIZE);
	imshow(INPUT_WIN, src);

	// �ϲ���
	pyrUp(src, dst, Size(src.cols*2, src.rows * 2));
	imshow(OUTPUT_WIN, dst);

	// ������
	Mat s_down;
	pyrDown(src, s_down, Size(src.cols / 2, src.rows / 2));
	imshow("sample down", s_down);

	// DOG
	Mat gray_src, g1, g2, dogImg;
	cvtColor(src, gray_src, CV_BGR2GRAY);
	GaussianBlur(gray_src, g1, Size(5, 5), 0, 0);
	GaussianBlur(g1, g2, Size(5, 5), 0, 0);
	subtract(g1, g2, dogImg, Mat());

	// ��һ����ʾ
	normalize(dogImg, dogImg, 255, 0, NORM_MINMAX);
	imshow("DOG Image", dogImg);

	waitKey(0);
	return 0;
}