#include<opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;


//原版本的分水岭算法
void watershedTest();
//网上参考的分水岭，都是掉用API，只是一些预处理不同
Mat Watersegment(Mat src);

//对图像做分水岭
Mat watershedCluster(Mat &image, int &numSegments);
void createDisplaySegments(Mat &segments, int numSegments, Mat &image);



Mat src;
int main(int argc,char** argv)
{
	src = imread("coins_001.jpg");
	if (src.empty())
	{
		cout << "image load err!" << endl;
		return -1;
	}
	imshow("inputImage", src);

	//test 1
	//watershedTest();

	//test 2
	//Mat dst = Watersegment(src);
	//imshow("outputImage", dst);


	//test 3
	src = imread("cvtest.png");
	if (src.empty())
	{
		cout << "image load err!" << endl;
		return -1;
	}
	imshow("inputImage", src);
	int numSegments;
	Mat markers = watershedCluster(src, numSegments);
	createDisplaySegments(markers, numSegments, src);


	waitKey(0);
	return 0;
}


void watershedTest()
{
	Mat graySrc, binary, shifted;
	//均值漂移算法 https://cloud.tencent.com/developer/article/1470668
	//把背景和前景变为比较纯色
	pyrMeanShiftFiltering(src, shifted, 21, 51);
	imshow("shifted", shifted);
	cvtColor(shifted, graySrc, COLOR_BGR2GRAY);
	//imshow("garySrc", graySrc);
	threshold(graySrc, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
	imshow("binary", binary);


	//距离变换
	Mat dist;
	distanceTransform(binary, dist, DistanceTypes::DIST_L2, 3, CV_32F);
	normalize(dist, dist, 0, 1, NORM_MINMAX);
	imshow("dist", dist);


	//二值化
	threshold(dist, dist, 0.4, 1, THRESH_BINARY);//注意阈值
	imshow("distBinary", dist);


	//生成mask
	Mat distMark;
	dist.convertTo(distMark,CV_8U, 225);
	vector<vector<Point>> contours;
	findContours(distMark, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	Mat marks = Mat::zeros(src.size(), CV_32SC1);
	for (size_t t = 0; t<contours.size(); t++)
	{
		//static_cast<int>(i+1)是为了分水岭的标记不同，区域1、2、3...这样才能分割
		drawContours(marks, contours, static_cast<int>(t), Scalar::all(static_cast<int>(t) + 1), -1);
	}
	circle(marks, Point(5, 5), 3, Scalar(255), -1);
	//imshow("masks", masks);//不知道为什么，显示这里就会报错

	//形态学处理  彩色图像，去掉干扰
	Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(src, src, MORPH_ERODE, k);


	//完成分水岭算法
	//image:三通道彩色图像
	//markers : 记号点（种子点），每一个记号都需要有不同的编号
	//https://www.cnblogs.com/mikewolf2002/p/3304118.html
	watershed(src, marks);
	Mat mark = Mat::zeros(marks.size(), CV_8UC1);
	marks.convertTo(mark, CV_8UC1);
	bitwise_not(mark, mark, Mat());
	imshow("watershed result", mark);

	//生成随机颜色表
	vector<Vec3b> colors;
	for (size_t i = 0; i < contours.size(); i++) {
		int r = theRNG().uniform(0, 255);
		int g = theRNG().uniform(0, 255);
		int b = theRNG().uniform(0, 255);
		colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
	}

	// 颜色填充与最终显示
	Mat dst = Mat::zeros(marks.size(), CV_8UC3);
	int index = 0;
	for (int row = 0; row < marks.rows; row++) {
		for (int col = 0; col < marks.cols; col++) {
			index = marks.at<int>(row, col);
			if (index > 0 && index <= contours.size()) {
				dst.at<Vec3b>(row, col) = colors[index - 1];
			}
			else {
				dst.at<Vec3b>(row, col) = Vec3b(0, 0, 0);
			}
		}
	}


	imshow("Final Result", dst);
	cout<<"number of objects:  "<<contours.size()<<endl;


}

//分水岭算法
Mat Watersegment(Mat src) {
	int row = src.rows;
	int col = src.cols;
	//1. 将RGB图像灰度化
	Mat graySrc;
	cvtColor(src, graySrc, COLOR_BGR2GRAY);
	//2. 使用大津法转为二值图，并做形态学闭合操作
	threshold(graySrc, graySrc, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//3. 形态学闭操作
	Mat kernel = getStructuringElement(MORPH_RECT, Size(9, 9), Point(-1, -1));
	morphologyEx(graySrc, graySrc, MORPH_CLOSE, kernel);
	//4. 距离变换
	distanceTransform(graySrc, graySrc, DIST_L2, DIST_MASK_3, 5);
	//5. 将图像归一化到[0, 1]范围
	normalize(graySrc, graySrc, 0, 1, NORM_MINMAX);
	//6. 将图像取值范围变为8位(0-255)
	graySrc.convertTo(graySrc, CV_8UC1);
	//7. 再使用大津法转为二值图，并做形态学闭合操作
	threshold(graySrc, graySrc, 0, 255, THRESH_BINARY | THRESH_OTSU);
	morphologyEx(graySrc, graySrc, MORPH_CLOSE, kernel);
	//8. 使用findContours寻找marks
	vector<vector<Point>> contours;
	findContours(graySrc, contours, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(-1, -1));
	Mat marks = Mat::zeros(graySrc.size(), CV_32SC1);
	for (size_t i = 0; i < contours.size(); i++)
	{
		//static_cast<int>(i+1)是为了分水岭的标记不同，区域1、2、3...这样才能分割
		drawContours(marks, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i + 1)), 2);
	}
	//9. 对原图做形态学的腐蚀操作
	Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(src, src, MORPH_ERODE, k);
	//10. 调用opencv的分水岭算法
	watershed(src, marks);
	//11. 随机分配颜色
	vector<Vec3b> colors;
	for (size_t i = 0; i < contours.size(); i++) {
		int r = theRNG().uniform(0, 255);
		int g = theRNG().uniform(0, 255);
		int b = theRNG().uniform(0, 255);
		colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
	}

	// 12. 显示
	Mat dst = Mat::zeros(marks.size(), CV_8UC3);
	int index = 0;
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			index = marks.at<int>(i, j);
			if (index > 0 && index <= contours.size()) {
				dst.at<Vec3b>(i, j) = colors[index - 1];
			}
			else if (index == -1)
			{
				dst.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
			}
			else {
				dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			}
		}
	}
	return dst;
}


Mat watershedCluster(Mat &image, int &numSegments)
{
	//二值化
	Mat gray, binary;
	cvtColor(image, gray, COLOR_BGR2GRAY);
	threshold(gray,binary,0,255,THRESH_BINARY|THRESH_OTSU);

	//形态学操作
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(binary, binary, MORPH_OPEN, kernel, Point(-1, -1));
	
	//距离变换
	Mat dist;
	distanceTransform(binary, dist, DistanceTypes::DIST_L2, 3, CV_32F);
	normalize(dist, dist, 0, 1, NORM_MINMAX);
	dist.convertTo(dist, CV_8UC1);

	//开始生成标记
	vector<vector<Point>>  contours;
	vector<Vec4i> hireachy;
	findContours(dist, contours, hireachy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	if (contours.empty()) return Mat();

	Mat markers(dist.size(), CV_32S);
	markers = Scalar::all(0);
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(markers, contours, i, Scalar(i + 1), -1, 8, hireachy, INT_MAX);

	}
	circle(markers, Point(5, 5), 3, Scalar(255), -1);



	//分水岭变换
	watershed(image, markers);
	numSegments = contours.size();
	return markers;
}



void createDisplaySegments(Mat &markers, int numSegments, Mat &image)
{
	//生成颜色表
	vector<Vec3b> colors;
	for (size_t t=0; t<numSegments; t++)
	{
		int r = theRNG().uniform(0, 255);
		int g = theRNG().uniform(0, 255);
		int b = theRNG().uniform(0, 255);
		colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
	}

	//颜色填充与最终显示
	Mat dst = Mat::zeros(markers.size(), CV_8UC3);
	int index = 0;
	for (int row = 0; row < markers.rows; row++) {
		for (int col = 0; col < markers.cols; col++) {
			index = markers.at<int>(row, col);
			if (index > 0 && index <= numSegments) {
				dst.at<Vec3b>(row, col) = colors[index - 1];
			}
			else {
				dst.at<Vec3b>(row, col) = Vec3b(255, 255, 255);
			}
		}
	}
	imshow("waterShed-Demo", dst);
	return;
}