#include <opencv2/opencv.hpp>
#include <iostream>



using namespace std;
using namespace cv;

Mat   src, binary,dat;
const char* inputWin = "inputImage";
const char* outputWin = "outputImage";


int main(int argc,char** argv)
{
	src = imread("src.jpg");
	if (src.empty())
	{
		cout << "image load err!" << endl;
		return -1;
	}

	cvtColor(src,src,COLOR_BGR2GRAY);
	//二值分割，OTSU：迭代寻找最小类方差，最大外...  TRIANGLE：三角自动阈值，对于单峰图像比较好，
	//对于玉米的金黄色背景单一，是为单峰图像，使用三角阈值法更好(最开始是对细胞的计数，进行荧光显示，然后三角化阈值二值化)
	//通过距离变换找到单峰
	threshold(src,binary,0,255,THRESH_BINARY|THRESH_TRIANGLE);//重要的是知道为什么使用 THRESH_TRIANGLE
	imshow("binaryImage",binary);


	//形态学变换
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	dilate(binary, binary, kernel, Point(-1, -1),10);
	imshow("dilateImage",binary);


	//距离变换
	Mat dist;
	bitwise_not(binary,binary);
	distanceTransform(binary, dist, DIST_L2, 3);
	normalize(dist, dist,0,1.0,NORM_MINMAX);
	imshow("distImage", dist);

	//阈值化二值分割
	Mat dist_8u;
	dist.convertTo(dist_8u,CV_8U);
	//局部自适应阈值，85， int blockSize必须为奇数
	adaptiveThreshold(dist_8u, dist_8u, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 85, 0.0);
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	//dilate(dist_8u, dist_8u, kernel, Point(-1, -1), 2);
	imshow("dist-binary", dist_8u);

	//截的图，没能找到正确的参数分开最难的那两粒，传统图像识别对图像的要求还是蛮高的，深度学习识别玉米粒再计数应该会好很多



	//连通域计数
	vector<vector<Point>> contours;
	findContours(dist_8u,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);

	//绘制
	Mat markers = Mat::zeros(src.size(), CV_8UC3);
	RNG rng(12345);
	for (size_t t = 0; t< contours.size(); t++)
	{
		drawContours(markers,contours, static_cast<int>(t), Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), - 1, 8, Mat());

	}

	cout << "number of corns: " << contours.size() << endl;
	imshow("final result", markers);

	waitKey(0);

	return 0;
}


