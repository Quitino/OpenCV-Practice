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
	//��ֵ�ָOTSU������Ѱ����С�෽������...  TRIANGLE�������Զ���ֵ�����ڵ���ͼ��ȽϺã�
	//�������׵Ľ��ɫ������һ����Ϊ����ͼ��ʹ��������ֵ������(�ʼ�Ƕ�ϸ���ļ���������ӫ����ʾ��Ȼ�����ǻ���ֵ��ֵ��)
	//ͨ������任�ҵ�����
	threshold(src,binary,0,255,THRESH_BINARY|THRESH_TRIANGLE);//��Ҫ����֪��Ϊʲôʹ�� THRESH_TRIANGLE
	imshow("binaryImage",binary);


	//��̬ѧ�任
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	dilate(binary, binary, kernel, Point(-1, -1),10);
	imshow("dilateImage",binary);


	//����任
	Mat dist;
	bitwise_not(binary,binary);
	distanceTransform(binary, dist, DIST_L2, 3);
	normalize(dist, dist,0,1.0,NORM_MINMAX);
	imshow("distImage", dist);

	//��ֵ����ֵ�ָ�
	Mat dist_8u;
	dist.convertTo(dist_8u,CV_8U);
	//�ֲ�����Ӧ��ֵ��85�� int blockSize����Ϊ����
	adaptiveThreshold(dist_8u, dist_8u, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 85, 0.0);
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	//dilate(dist_8u, dist_8u, kernel, Point(-1, -1), 2);
	imshow("dist-binary", dist_8u);

	//�ص�ͼ��û���ҵ���ȷ�Ĳ����ֿ����ѵ�����������ͳͼ��ʶ���ͼ���Ҫ�������ߵģ����ѧϰʶ���������ټ���Ӧ�û�úܶ�



	//��ͨ�����
	vector<vector<Point>> contours;
	findContours(dist_8u,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);

	//����
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


