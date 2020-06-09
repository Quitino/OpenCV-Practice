#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>

using namespace std;
using namespace cv;

const char* inputWin = "inputImage";
const char* outputWin = "outputImage";
Mat src, binary, dst;
int main(int argc, char** argv)
{

	src = imread("src.jpg");
	if (src.empty())
	{
		cout << "Image Load Err!\n" << endl;
		return -1;
	}
	//imshow(inputWin,src);


	//��ֵ��
	cvtColor(src, src, COLOR_BGR2GRAY);
	threshold(src, binary, 0, 255,THRESH_BINARY | THRESH_OTSU);
	//imshow("binaryImage", binary);

	//��̬ѧ����
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(binary, dst, MORPH_CLOSE, kernel, Point(-1, -1));
	//imshow("morphology close", dst);

	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	morphologyEx(dst, dst, MORPH_OPEN, kernel, Point(-1, -1));
	//imshow("morphology open",dst);


	//Ѱ������
	vector<vector<Point>>  contours;
	vector<Vec4i>  hireachy;
	findContours(dst, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
	Mat resultImage = Mat::zeros(src.size(), CV_8UC3);
	//���Ƶ�ԭͼ��
	//Mat circleImage = src.clone();
	//cvtColor(circleImage, circleImage, COLOR_GRAY2BGR);
	Point cc;
	for (size_t t=0; t< contours.size(); t++)
	{
		//���˵�������С��,��� ����
		double area = contourArea(contours[t]);
		if(area < 1000) continue;

		//���ݱȲ�������
		Rect rect = boundingRect(contours[t]);
		float ratio = float(rect.width) / float(rect.height);
		if (ratio > 1.3 || ratio < 0.7) continue;
		
		drawContours(resultImage, contours,t, Scalar(0, 255, 255), -1, 8, Mat(), 0, Point());//��ɫ
		cout << "circle area:  "  <<  area << endl <<"circle length:  "<< arcLength(contours[t],true)<<endl;

		//Ѱ��ԭ��
		int x = rect.x + rect.width / 2;
		int y = rect.y + rect.height / 2;
		cc = Point(x, y);
		circle(resultImage,cc, 2, Scalar(0, 0, 255),2,8,0);//��ɫ
	}
	imshow(outputWin, resultImage);
	//���Ƶ�ԭͼ��
	Mat circleImage = src.clone();
	cvtColor(circleImage, circleImage, COLOR_GRAY2BGR);
	circle(circleImage, cc, 2, Scalar(0, 0, 255), 2, 8, 0);//��ɫ
	imshow("final result", circleImage);

	//���Ƴ�����û�취ʹ�ã�����Ҫ�������ҵ�������Բ���,Ч������...
	//vector<Vec3f> myCircles;
	//Mat grayResult;
	//cvtColor(resultImage, grayResult, COLOR_BGR2GRAY);
	////HoughCircles(grayResult, myCircles, HOUGH_GRADIENT, 1,10,100,30,10,grayResult.rows/4);
	//HoughCircles(grayResult, myCircles, HOUGH_GRADIENT, 1, 10, 50, 20, 23, grayResult.rows/4);

	//���Ƶ�ԭͼ��
	//Mat circleImage = src.clone();
	//cvtColor(circleImage, circleImage, COLOR_GRAY2BGR);
	//for (int t = 0;t<myCircles.size();t++)
	//{
	//	Vec3i circleInfo = myCircles[t];
	//	circle(circleImage, Point(circleInfo[0],circleInfo[1]),circleInfo[2],Scalar(0,0,255),2,8,0);
	//}
	//imshow("final result", circleImage);

	waitKey(0);
	return 0;
}
