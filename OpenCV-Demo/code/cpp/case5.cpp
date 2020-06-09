#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

/*�����õģ����Է�װ��һ������*/

int main(int argc,char** argv)
{
	Mat src = imread("src.jpg");
	if (src.empty())
	{
		cout << "load image err!" << endl;
		return -1;
	}
	Mat graySrc, binary, dst;

	//��ֵ������
	cvtColor(src, graySrc, COLOR_BGR2GRAY);
	threshold(graySrc, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	imshow("binaryImage",binary);


	//��̬ѧ����
	Mat kernel = getStructuringElement(MORPH_RECT,Size(5,5),Point(-1,-1));
	morphologyEx(binary,dst,MORPH_CLOSE,kernel,Point(-1,-1));
	dilate(dst,dst,kernel,Point(-1,-1),5);
	imshow("morphologyImage",dst);

	//��������
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	bitwise_not(dst,dst,Mat());//��λȡ��
	//imshow("tmp",dst);
	findContours(dst,contours,hireachy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(-1,-1));

	//��������
	int width = src.cols;
	int height = src.rows;
	Mat drawImage = Mat::zeros(src.size(), CV_8UC3);


	for (size_t t = 0; t<contours.size(); t++)
	{
		Rect rect = boundingRect(contours[t]);
		if (rect.width > width/2 && rect.height < height -5)
		{
			drawContours(drawImage, contours, static_cast<int>(t), Scalar(0, 255, 255), 2, 8, hireachy, 0, Point());
		}
	}
	imshow("contours", drawImage);


	/*Ȼ��ͨ��ֱ�߼�⣬�ҵ������ߣ��ĸ���������*/
	
	//����ֱ�߼��
	vector<Vec4i> lines;
	Mat contoursImage;
	int accu = min(width*0.5, height*0.5);
	cvtColor(drawImage,contoursImage,COLOR_BGR2GRAY);
	//imshow("tmp",contoursImage);
	HoughLinesP(contoursImage, lines, 1, CV_PI / 180.0, 55, accu, 0);
	Mat linesImage = Mat::zeros(src.size(), CV_8UC3);
	for (size_t t = 0; t<lines.size(); t++)
	{
		Vec4i ln = lines[t];
		line(linesImage, Point(ln[0], ln[1]), Point(ln[2], ln[3]), Scalar(0, 255, 255), 2, 8, 0);
	}
	cout << "number of lines:" << lines.size() << endl;
	imshow("linesImage", linesImage);


	//Ѱ�Ҷ�λ��������������ֱ��
	int deltah = 0;

	Vec4i topLine, bottomLine, leftLine, rightLine;
	for (int i = 0; i < lines.size(); i++) {
		Vec4i ln = lines[i];//ln[0]:�˵�1����col-x  ln[1]�˵�1����row-y    ln[2]�˵�2��col-x   ln[3]�˵�2����row-y
		deltah = abs(ln[3] - ln[1]);//�жϺϷ�
		if (ln[3] < height / 2.0 && ln[1] < height / 2.0 && deltah < accu - 1) 
		{
			//����С��ͼƬ�ߵ�һ�룬��ΪtopLine
			//if (topLine[3] > ln[3] && topLine[3] > 0) {
			//	topLine = lines[i];
			//}
			//else {
			//	topLine = lines[i];
			//}
			topLine = lines[i];
		}
		if (ln[3] > height / 2.0 && ln[1] > height / 2.0 && deltah < accu - 1) 
		{
			//��������ͼƬ�ߵ�һ�룬��ΪbottomLine
			bottomLine = lines[i];
		}
		if (ln[0] < width / 2.0 && ln[2] < width / 2.0) 
		{
			//����С��ͼƬ���һ�룬��ΪleftLine
			leftLine = lines[i];
		}
		if (ln[0] > width / 2.0 && ln[2] > width / 2.0) 
		{
			//��������ͼƬ���һ�룬��ΪrightLine
			rightLine = lines[i];
		}
	}
	cout << "top line : p1(x, y) = " << topLine[0] << "," << topLine[1] << " p2(x, y) = " << topLine[2] << "," << topLine[3] << endl;
	cout << "bottom line : p1(x, y) = " << bottomLine[0] << "," << bottomLine[1] << " p2(x, y) = " << bottomLine[2] << "," << bottomLine[3] << endl;
	cout << "left line : p1(x, y) = " << leftLine[0] << "," << leftLine[1] << " p2(x, y) = " << leftLine[2] << "," << leftLine[3] << endl;
	cout << "right line : p1(x, y) = " << rightLine[0] << "," << rightLine[1] << " p2(x, y) = " << rightLine[2] << "," << rightLine[3] << endl;

	// �������ֱ�߷���
	// y=kx+c ��ȡб��k��ƫ����c
	// k = (y2 - y1)/(x2 - x1)
	// c = y1 - k*x1
	float k1, c1;
	k1 = float(topLine[3] - topLine[1]) / float(topLine[2] - topLine[0]);
	c1 = topLine[1] - k1 * topLine[0];

	float k2, c2;
	k2 = float(bottomLine[3] - bottomLine[1]) / float(bottomLine[2] - bottomLine[0]);
	c2 = bottomLine[1] - k2 * bottomLine[0];

	float k3, c3;
	k3 = float(leftLine[3] - leftLine[1]) / float(leftLine[2] - leftLine[0]);
	c3 = leftLine[1] - k3 * leftLine[0];

	float k4, c4;
	k4 = float(rightLine[3] - rightLine[1]) / float(rightLine[2] - rightLine[0]);
	c4 = rightLine[1] - k4 * rightLine[0];



	// ����ֱ�߽���
	Point p1; // ���Ͻ�
	p1.x = static_cast<int>((c1 - c3) / (k3 - k1));
	p1.y = static_cast<int>(k1*p1.x + c1);
	Point p2; // ���Ͻ�
	p2.x = static_cast<int>((c1 - c4) / (k4 - k1));
	p2.y = static_cast<int>(k1*p2.x + c1);
	Point p3; // ���½�
	p3.x = static_cast<int>((c2 - c3) / (k3 - k2));
	p3.y = static_cast<int>(k2*p3.x + c2);
	Point p4; // ���½�
	p4.x = static_cast<int>((c2 - c4) / (k4 - k2));
	p4.y = static_cast<int>(k2*p4.x + c2);
	cout << "p1(x, y)=" << p1.x << "," << p1.y << endl;
	cout << "p2(x, y)=" << p2.x << "," << p2.y << endl;
	cout << "p3(x, y)=" << p3.x << "," << p3.y << endl;
	cout << "p4(x, y)=" << p4.x << "," << p4.y << endl;

	// ��ʾ�ĸ�������
	circle(linesImage, p1, 2, Scalar(255, 0, 0), 2, 8, 0);
	circle(linesImage, p2, 2, Scalar(255, 0, 0), 2, 8, 0);
	circle(linesImage, p3, 2, Scalar(255, 0, 0), 2, 8, 0);
	circle(linesImage, p4, 2, Scalar(255, 0, 0), 2, 8, 0);
	line(linesImage, Point(topLine[0], topLine[1]), Point(topLine[2], topLine[3]), Scalar(0, 255, 0), 2, 8, 0);
	imshow("four corners", linesImage);



	// ͸�ӱ任
	/*͸�ӱ任��Ҫ��У�����ĸ��ǵ㣬�Լ�ͼƬ��С���ĸ��ǵ�*/

	//У��ǰ���ĸ��ǵ�
	vector<Point2f> src_corners(4);
	src_corners[0] = p1;
	src_corners[1] = p2;
	src_corners[2] = p3;
	src_corners[3] = p4;
	//��������ĸ��ǵ�
	vector<Point2f> dst_corners(4);
	dst_corners[0] = Point(0, 0);
	dst_corners[1] = Point(width, 0);
	dst_corners[2] = Point(0, height);
	dst_corners[3] = Point(width, height);

	// ��ȡ͸�ӱ任����
	Mat resultImage;
	Mat warpmatrix = getPerspectiveTransform(src_corners, dst_corners);
	warpPerspective(src, resultImage, warpmatrix, resultImage.size(), INTER_CUBIC);//͸�ӱ任
	namedWindow("Final Result", WINDOW_AUTOSIZE);
	imshow("srcImage", src);
	imshow("Final Result", resultImage);


	waitKey(0);
	return 0;
}
