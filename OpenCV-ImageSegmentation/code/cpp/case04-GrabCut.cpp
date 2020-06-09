#include <opencv2/opencv.hpp>
#include <iostream>


using namespace std;
using namespace cv;
const char* winTitle = "inputImage";

Mat src, dst;
Rect rect;
bool init = false;
int numRun = 0;


//mask  背景  前景
Mat  mask, bgModel, fgModel;

void onMouse(int event,int x, int y, int flags, void* param);
void setROIMask();
void showImage();
void runGrabCut();

int main()
{
	src = imread("src.jpg");
	if (src.empty())
	{
		cout << "load image err!" << endl;
		return -1;
	}
	
	mask.create(src.size(), CV_8UC1);
	mask.setTo(Scalar::all(GC_BGD));//鼠标画上去会有提示，这里是背景

	namedWindow(winTitle, WINDOW_AUTOSIZE);
	setMouseCallback(winTitle,onMouse,0);
	imshow("inputImage",src);


	while (true)
	{
		//char c = (char)waitKey(0);
		//if (c == 'n')
		//{
			runGrabCut();
			numRun++;
			showImage();
			cout << "current iteative times: " << numRun << endl;

		//}
		//if ((int)c == 27)
		if(numRun == 27)
		{
			break;
		}
		waitKey(0);

	}
	
	waitKey(0);
	return 0;
}



void onMouse(int event, int x, int y, int flags, void* param)
{
	switch (event)
	{

	case EVENT_LBUTTONDOWN:
		rect.x = x;
		rect.y = y;
		rect.width = 1;
		rect.height = 1;
		init = false;
		//numRun = 0;
		break;

	case EVENT_MOUSEMOVE:
		if (flags & EVENT_FLAG_LBUTTON)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));//point(x,y)鼠标终点
			showImage();
		}
		break;

	case EVENT_LBUTTONUP:
		if (rect.width > 1 && rect.height >1)
		{
			setROIMask();
			showImage();
		}
		break;
	default:
		break;
	}

}

void setROIMask()
{
	// GC_FGD = 1
	// GC_BGD =0;
	// GC_PR_FGD = 3
	// GC_PR_BGD = 2
	mask.setTo(GC_BGD);
	rect.x = max(0, rect.x);
	rect.y = max(0, rect.y);
	rect.width = min(src.cols - rect.x , rect.width);
	rect.height = min(src.rows - rect.y, rect.height);

	mask(rect).setTo(Scalar(GC_PR_FGD));//前景


}
void showImage()
{
	//Mat result = src.clone();
	//rectangle(result, rect, Scalar(0, 0, 255), 2, 8);
	//imshow(winTitle, result);



	Mat result, binMask;
	binMask.create(mask.size(), CV_8UC1);
	binMask = mask & 1;
	if (init) {
		src.copyTo(result, binMask);
	}
	else {
		src.copyTo(result);
	}


	rectangle(result, rect, Scalar(0, 0, 255), 2, 8);
	imshow(winTitle, result);
	
}

void runGrabCut()
{
	if (rect.width<2 || rect.height<2)
	{
		return;
	}

	if (init)
	{
		grabCut(src, mask, rect, bgModel, fgModel, 1);
	}

	{
		grabCut(src, mask, rect, bgModel, fgModel, 1, GC_INIT_WITH_RECT);
		init = true;
	}
}