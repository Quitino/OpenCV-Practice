#include <opencv2/opencv.hpp> 
#include <iostream> 
using namespace cv;

Mat src, dst;
char OUTPUT_WIN[] = "output image";
int element_size = 3;
int max_size = 21;
void CallBack_Demo(int, void*);
int main(int argc, char** argv) {
	
	src = imread("D:/vcprojects/images/test1.png");
	if (!src.data) {
		printf("could not load image...\n");
		return -1;
	}
	namedWindow("input image", CV_WINDOW_AUTOSIZE);
	imshow("input image", src);

	namedWindow(OUTPUT_WIN, CV_WINDOW_AUTOSIZE);
	createTrackbar("Element Size :", OUTPUT_WIN, &element_size, max_size, CallBack_Demo);
	CallBack_Demo(0, 0);

	waitKey(0);
	return 0;
}

void CallBack_Demo(int, void*) {
	int s = element_size * 2 + 1;
	Mat structureElement = getStructuringElement(MORPH_RECT, Size(s, s), Point(-1, -1));
	// dilate(src, dst, structureElement, Point(-1, -1), 1);
	erode(src, dst, structureElement);
	imshow(OUTPUT_WIN, dst);
	return;
}
