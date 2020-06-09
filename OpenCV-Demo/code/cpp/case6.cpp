#include <opencv2/opencv.hpp>
#include <iostream>



using namespace std;
using namespace cv;

Mat src, dst;
int main(int argc, char** argv)
{
	src = imread("src.jpg");
	if (src.empty())
	{
		cout << "image load err!" << endl;
		return -1;
	}
	imshow("inputImage", src);

	//Mat srcGray,binary;
	//cvtColor(src,srcGray,COLOR_BGR2GRAY);
	//threshold(srcGray, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//imshow("binary", binary);
	//直接二值化行不通

	//首先降噪处理，高斯模糊
	Mat blurImage;
	GaussianBlur(src, blurImage, Size(15, 15), 0, 0);
	imshow("blurImage", blurImage);

	Mat srcGray, binary;
	cvtColor(blurImage, srcGray, COLOR_BGR2GRAY);
	threshold(srcGray, binary, 0, 255, THRESH_BINARY | THRESH_TRIANGLE);//单峰图
	imshow("binary", binary);


	//形态学操作
	Mat morphImage;
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(binary, morphImage, MORPH_CLOSE, kernel, Point(-1, -1), 2);
	imshow("morphology", morphImage);

	// 获取最大轮廓
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	findContours(morphImage, contours, hireachy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());
	Mat connImage = Mat::zeros(src.size(), CV_8UC3);
	for (size_t t = 0; t < contours.size(); t++) {
		Rect rect = boundingRect(contours[t]);
		if (rect.width < src.cols / 2) continue;
		if (rect.width > (src.cols - 20)) continue;
		double area = contourArea(contours[t]);
		double len = arcLength(contours[t], true);
		drawContours(connImage, contours, static_cast<int>(t), Scalar(0, 0, 255), 1, 8, hireachy);
		cout<<"area  of star could : "<<area<<endl;
		cout<<"length  of star could : "<< endl;
	}
	imshow("result", connImage);



	waitKey(0);
	return 0;
}

