#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
	Mat src;
	src = imread("D:/vcprojects/images/test.png");
	if (src.empty()) {
		cout << "could not load image..." << endl;
		return -1;
	}
	namedWindow("input", CV_WINDOW_AUTOSIZE);
	imshow("input", src);

	/*Mat dst;
	dst = Mat(src.size(), src.type());
	dst = Scalar(127, 0, 255);
	namedWindow("output", CV_WINDOW_AUTOSIZE);
	imshow("output", dst);*/
	Mat dst;
	//src.copyTo(dst);
	namedWindow("output", CV_WINDOW_AUTOSIZE);

	cvtColor(src, dst, CV_BGR2GRAY);
	printf("input image channels : %d\n", src.channels());
	printf("output image channels : %d\n", dst.channels());

	int cols = dst.cols;
	int rows = dst.rows;

	printf("rows : %d cols : %d\n", rows, cols);
	const uchar* firstRow = dst.ptr<uchar>(0);
	printf("fist pixel value : %d\n", *firstRow);

	Mat M(100, 100, CV_8UC1, Scalar(127));
	//cout << "M =" << endl << M << endl;

	Mat m1;
	m1.create(src.size(), src.type());
	m1 = Scalar(0, 0, 255);

	Mat csrc;
	Mat kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	filter2D(src, csrc, -1, kernel);

	Mat m2 = Mat::eye(2, 2, CV_8UC1);
	cout << "m2 =" << endl << m2 << endl;

	imshow("output", m2);
	waitKey(0);
	return 0;
}