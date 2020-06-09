#include <opencv2/opencv.hpp>
#include <iostream>


using namespace std;
using namespace cv;

Mat videoBgReplace(Mat &frame, Mat &mask);
Mat background01, background02;

int main(int argc, char** argv)
{
	background01 = imread("src1.jpg");
	background02 = imread("src2.jpg");
	VideoCapture capture;
	capture.open("01.mp4");
	if (!capture.isOpened())
	{
		cout << "video load err!" << endl;
		return -1;
	}
	const char* title = "inputVideo";
	const char* resultWin = "resultVideo";
	namedWindow(title, WINDOW_AUTOSIZE);
	namedWindow(resultWin, WINDOW_AUTOSIZE);

	Mat frame, hsv, mask;
	int count;
	bool flag = true;
	while (capture.read(frame))
	{
		if (flag)
		{
			cout << "width: " << frame.cols << "\n height:" << frame.rows << endl;
			flag = false;
		}
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		inRange(hsv, Scalar(35, 43, 46), Scalar(155, 255, 255), mask);//绿幕颜色范围值,绿色使用255白色替换


		//形态学操作
		Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
		morphologyEx(mask, mask, MORPH_CLOSE, kernel);
		erode(mask,mask,kernel);
		GaussianBlur(mask, mask, Size(3, 3), 0, 0);


		Mat result = videoBgReplace(frame, mask);
		char c = waitKey(1);
		if(c == 27) break;
		imshow(resultWin, result);
		imshow(title, frame);

	}

	waitKey(0);
	return 0;
}

Mat videoBgReplace(Mat &frame, Mat &mask)
{
	Mat result = Mat::zeros(frame.size(), frame.type());
	int width = frame.cols;
	int height = frame.rows;
	int dims = frame.channels();


	//替换
	int m = 0;
	double wt = 0;
	
	int r = 0, g = 0, b = 0;
	int r1 = 0, g1 = 0, b1 = 0;
	int r2 = 0, g2 = 0, b2 = 0;

	for (int row = 0; row < height; row++) {
		//一行一行的取地址
		uchar* current = frame.ptr<uchar>(row);
		uchar* bgrow = background01.ptr<uchar>(row);
		uchar* maskrow = mask.ptr<uchar>(row);
		//frame + background + mask = result
		uchar* targetrow = result.ptr<uchar>(row);


		for (int col = 0; col < width; col++) {
			m = *maskrow++;//地址指针在滑动
			if (m == 255) { // 替换掉背景
				*targetrow++ = *bgrow++;
				*targetrow++ = *bgrow++;
				*targetrow++ = *bgrow++;
				current += 3;//注意是三通道

			}
			else if (m == 0) {// 保留前景
				*targetrow++ = *current++;
				*targetrow++ = *current++;
				*targetrow++ = *current++;
				bgrow += 3;
			}
			else {//mask的边缘部分，使用新的背景与前景融合
				b1 = *bgrow++;
				g1 = *bgrow++;
				r1 = *bgrow++;

				b2 = *current++;
				g2 = *current++;
				r2 = *current++;

				// 权重
				wt = m / 255.0;

				// 混合
				b = b1 * wt + b2 * (1.0 - wt);
				g = g1 * wt + g2 * (1.0 - wt);
				r = r1 * wt + r2 * (1.0 - wt);

				*targetrow++ = b;
				*targetrow++ = g;
				*targetrow++ = r;
			}
		}
	}

	return result;
}