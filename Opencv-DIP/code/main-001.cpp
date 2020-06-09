#include<opencv2\opencv.hpp>
#include<highgui.h>
using namespace cv;
int main(int argc, char** argv)
{
	// read image
	Mat image = imread("test.jpg");

	// ��ͼ��������������� ��255- ����ֵ��
	Mat invertImage;
	image.copyTo(invertImage);

	// ��ȡͼ�����
	int channels = image.channels();
	int rows = image.rows;
	int cols = image.cols * channels;
	if (image.isContinuous()) {
		cols *= rows;         
		rows = 1;
	}

	// ÿ�����ص��ÿ��ͨ��255ȡ��
	uchar* p1;
	uchar* p2;
	for (int row = 0; row < rows; row++) {
		p1 = image.ptr<uchar>(row);// ��ȡ����ָ��
		p2 = invertImage.ptr<uchar>(row);
		for (int col = 0; col < cols; col++) {
			*p2 = 255 - *p1; // ȡ��
			p2++;
			p1++;
		}
	}

	// create windows
	namedWindow("My Test", CV_WINDOW_AUTOSIZE);
	namedWindow("My Invert Image", CV_WINDOW_AUTOSIZE);

	// display image
	imshow("My Test", image);
	imshow("My Invert Image", invertImage);

	// �ر�
	waitKey(0);
	destroyWindow("My Test");
	destroyWindow("My Invert Image");
	return 0;
}