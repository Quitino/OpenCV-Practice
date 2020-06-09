#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace  std;
using namespace  cv;

Mat src, gray_src, dat;
//灰度等级
int threshold_value = 100;
int max_level = 255;
const char* output_win = "Contours Result";
const char* roi_win = "Final Result";


void FindROI(int, void*);//切边，实际上就是找到ROI区域
void Check_Skew(int, void*);
Mat RotateImage(cv::Mat src, double angle);

int main(int argc, char* argv)
{
	src = imread("src.jpg");
	src = RotateImage(src, 10);
	if (src.empty())
	{
		printf("could not load image! \n");
		return -1;
	}

	namedWindow("inputImage", WINDOW_AUTOSIZE);
	imshow("inputImage", src);
	namedWindow(output_win, WINDOW_AUTOSIZE);
	//careatTrackbar();
	createTrackbar("Threshold:", output_win, &threshold_value, max_level, FindROI);
	Check_Skew(0, 0);
	//FindROI(0, 0);

	waitKey(0);
	return 0;
}

/*****************************************************************************
function:			 FindROI
description:		 响应滑动条的回调函数,寻找ROI，切边
inparam: 
outparam: 
return: 			  
*****************************************************************************/
void FindROI(int, void*) {
	cvtColor(src, gray_src, COLOR_BGR2GRAY);
	Mat canny_output;
	Canny(gray_src, canny_output, threshold_value, threshold_value * 2, 3, false);

	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	findContours(canny_output, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));//寻找轮廓，传入的图像需要是二值图像
	//寻找最大矩形轮廓
	int minw = src.cols*0.75;
	int minh = src.rows*0.75;
	//RNG 随机数产生器
	RNG rng(12345);
	Mat drawImage = Mat::zeros(src.size(), CV_8UC3);
	Rect bbox;
	for (size_t t = 0; t < contours.size(); t++) {
		//RotatedRect旋转矩形const Point2f & 	center,		const Size2f & 	size,			float 	angle  即是中心点、大小、旋转的角度
		//minAreaRect查找旋转的矩形的四个顶点
		//寻找轮廓的最小矩形
		RotatedRect minRect = minAreaRect(contours[t]);
		float degree = abs(minRect.angle);
		if (minRect.size.width > minw && minRect.size.height > minh && minRect.size.width < (src.cols - 5)) {
			printf("current angle : %f\n", degree);
			Point2f pts[4];
			minRect.points(pts);
			//返回包含旋转后的矩形的最小正整数
			bbox = minRect.boundingRect();
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));//uniform在态分布
			for (int i = 0; i < 4; i++) {
				//line(drawImage, pts[i], pts[i + 1], color, 2, 8, 0);
				//把多个点连成一条直线的时候，上面这种写法是会超出范围的，采用取余的操作进行封闭绘图，很巧妙！
				line(drawImage, pts[i], pts[(i + 1) % 4], color, 2, 8, 0);
			}
		}
	}
	imshow(output_win, drawImage);

	if (bbox.width > 0 && bbox.height > 0) {
		Mat roiImg = src(bbox);
		imshow(roi_win, roiImg);
	}
	return;
}


/*****************************************************************************
function:			RotaImage
description:		旋转图片
inparam:			src:需要旋转的图片路径 angle:旋转角度
outparam: 
return: 			旋转后的图片  
*****************************************************************************/
Mat RotateImage(cv::Mat src, double angle)
{
	cv::Mat dst;
	try
	{
		
		// get rotation matrix for rotating the image around its center in pixel coordinates
		cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
		cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
		// determine bounding rectangle, center not relevant
		cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
		// adjust transformation matrix
		rot.at<double>(0, 2) += bbox.width / 2.0 - src.cols / 2.0;
		rot.at<double>(1, 2) += bbox.height / 2.0 - src.rows / 2.0;

		cv::warpAffine(src, dst, rot, bbox.size());
	}
	catch (cv::Exception e)
	{
	}

	return dst;
}

/*****************************************************************************
function:			Check_Skew
description:		响应滑动条的回调函数,寻找ROI，校正图像，切边
inparam:
outparam:
return:
*****************************************************************************/
void Check_Skew(int, void*) {
	Mat canny_output;
	cvtColor(src, gray_src, COLOR_BGR2GRAY);
	Canny(gray_src, canny_output, threshold_value, threshold_value * 2, 3, false);

	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	findContours(canny_output, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	Mat drawImg = Mat::zeros(src.size(), CV_8UC3);
	float maxw = 0;
	float maxh = 0;
	double degree = 0;
	for (size_t t = 0; t < contours.size(); t++) {
		RotatedRect minRect = minAreaRect(contours[t]);
		degree = abs(minRect.angle);
		if (degree > 0) {
			//最大高度和最大宽度
			maxw = max(maxw, minRect.size.width);
			maxh = max(maxh, minRect.size.height);
		}
	}
	RNG rng(12345);
	for (size_t t = 0; t < contours.size(); t++) {
		RotatedRect minRect = minAreaRect(contours[t]);
		if (maxw == minRect.size.width && maxh == minRect.size.height) {
			degree = minRect.angle;//找到旋转的角度
			Point2f pts[4];
			minRect.points(pts);
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));//uniform 正态分布
			for (int i = 0; i < 4; i++) {
				//line(drawImg, pts[i], pts[i + 1], color, 2, 8, 0);
				//把多个点连成一条直线的时候，上面这种写法是会超出范围的，采用取余的操作进行封闭绘图，很巧妙！
				line(drawImg, pts[i], pts[(i + 1) % 4], color, 2, 8, 0);
			}
		}
	}
	printf("max contours width : %f\n", maxw);
	printf("max contours height : %f\n", maxh);
	printf("max contours angle : %f\n", degree);
	imshow(output_win, drawImg);
	Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);//按照图像中心进行旋转
	Mat rotm = getRotationMatrix2D(center, degree, 1.0);//得到旋转矩阵
	Mat dst;
	warpAffine(src, dst, rotm, src.size(), INTER_LINEAR, 0, Scalar(255, 255, 255));//图像旋转，Scalar(255, 255, 255)不是图像的区域进行颜色填充
	imshow("Correct Image", dst);
}
