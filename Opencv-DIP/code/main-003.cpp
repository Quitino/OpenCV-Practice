#include<opencv2\opencv.hpp>
#include<highgui.h>
using namespace cv;
int main(int argc, char** argv)
{
	// ����ͼ��
	Mat testImage = imread("D:/test.jpg");
	CV_Assert(testImage.depth() == CV_8U);
	namedWindow("test_image", CV_WINDOW_AUTOSIZE);
	imshow("test_image", testImage);

	// ʹ��Filter2D����
	Mat result;
	Mat kern = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	filter2D(testImage, result, testImage.depth(), kern);

	// ��ʾ���
	namedWindow("result_image", CV_WINDOW_AUTOSIZE);
	imshow("result_image", result);
	waitKey(0);
	return 0;
}