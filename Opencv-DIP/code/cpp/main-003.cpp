#include<opencv2\opencv.hpp>
#include<highgui.h>
using namespace cv;
int main(int argc, char** argv)
{
	// 加载图像
	Mat testImage = imread("D:/test.jpg");
	CV_Assert(testImage.depth() == CV_8U);
	namedWindow("test_image", CV_WINDOW_AUTOSIZE);
	imshow("test_image", testImage);

	// 使用Filter2D函数
	Mat result;
	Mat kern = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	filter2D(testImage, result, testImage.depth(), kern);

	// 显示结果
	namedWindow("result_image", CV_WINDOW_AUTOSIZE);
	imshow("result_image", result);
	waitKey(0);
	return 0;
}