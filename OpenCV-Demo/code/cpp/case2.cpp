#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>


using namespace std;
using namespace cv;

Mat  src,roiImage, dst;
const char*  inputWin = "inputImg";
const char*  outputWin = "outputImg";
int thresholdValue = 100;
int maxThresholdValue = 255;


void detectLines(int, void *);
void morphologyLines(int, void*);

int main(int argc, char* argv[])
{
	src = imread("src.png");
	if (src.empty())
	{
		cout << "Image load err! " << endl;
		return -1;
	}
	imshow(inputWin, src);


	Rect  roi = Rect(10, 10, src.cols - 10, src.rows - 10);//裁取一定的区域，有时候扫描出来会有边框，容易被误识别
	roiImage = src(roi);
	imshow("roiImage", roiImage);

	namedWindow(outputWin, WINDOW_AUTOSIZE);
	//createTrackbar("threahold:", outputWin, &thresholdValue, maxThresholdValue, detectLines);
	//createTrackbar("threahold:", outputWin, &thresholdValue, maxThresholdValue, morphologyLines);
	//detectLines(0,0);
	morphologyLines(0, 0);
	   
	waitKey(0);
	return 0;
}



/*****************************************************************************
function:			 detectLines
description:		 直接检测霍夫直线
inparam: 
outparam: 
return: 			  
*****************************************************************************/
void detectLines(int, void *)
{
	//canny检测边缘
	Canny(src, dst, thresholdValue, thresholdValue * 2, 3, false);
	imshow("tmp0", dst);
	vector<Vec4i> lines;
	//检测霍夫直线
	HoughLinesP(dst, lines, 1, CV_PI / 180.0, 30, 30.0, 0);
	cvtColor(dst, dst, COLOR_GRAY2BGR);
	for (size_t t = 0; t < lines.size(); t++)
	{
		Vec4i ln = lines[t];
		line(dst, Point(ln[0], ln[1]), Point(ln[2], ln[3]), Scalar(0, 0, 255), 2, 8, 0);
	}
	imshow(outputWin, dst);

	
}

/*****************************************************************************
function:			 morphologyLines
description:		 首先进行形态学处理，然后提取霍夫直线
inparam: 
outparam: 
return: 			  
*****************************************************************************/
void  morphologyLines(int, void*)
{
	//binary image 
	//Mat binaryImage, morphImage;
	//cvtColor(roiImage, roiImage, COLOR_BGR2GRAY);
	//threshold(roiImage, binaryImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//imshow("binary", binaryImage);

	//morphology image
	Mat binaryImage, morphImage;
	cvtColor(roiImage, roiImage, COLOR_BGR2GRAY);
	threshold(roiImage, binaryImage, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	imshow("binaryImage", binaryImage);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(50, 1), Point(-1, -1));//矩阵元素
	morphologyEx(binaryImage, morphImage, MORPH_OPEN, kernel, Point(-1, -1));//开运算
	imshow("morphologyImage",morphImage);

	//dilate 膨胀:以数据为中心，白色，扩张
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	dilate(morphImage,morphImage,kernel);
	imshow("dilateImage",morphImage);



	//hough line
	vector<Vec4i> lines;
	HoughLinesP(morphImage, lines, 1, CV_PI / 180.0, 30.0, 20.0, 0);
	Mat finalImage = roiImage.clone();
	cvtColor(finalImage, finalImage, COLOR_GRAY2BGR);
	for (size_t t = 0; t < lines.size(); t++)
	{
		Vec4i ln = lines[t];
		line(finalImage, Point(ln[0], ln[1]), Point(ln[2], ln[3]), Scalar(0, 255, 255), 2, 8, 0);
	}
	imshow("finalImage", finalImage);

	return;
}