#include<opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat src, dst;
void KMeansTest();
int main(int argc,char**argv)
{
	//KMeansTest();
	src = imread("src.jpg");
	if (src.empty())
	{
		cout << "image load err!" << endl;
		return - 1;
	}
	imshow("inputImage", src);
	int width = src.cols;
	int height = src.rows;
	int dims = src.channels();//维数

	//初始化定义
	int sampleCount = width * height;//样本数
	int clusterCount = 4;//分类数
	Mat points(sampleCount, dims, CV_32F);//样本存储
	Mat labels;//标签
	Mat centers(clusterCount, 1, points.type());//聚类中心
	Scalar colorTab[] = {
	Scalar(0, 0, 255),
	Scalar(0, 255, 0),
	Scalar(255, 0, 0),
	Scalar(0, 255, 255),
	Scalar(255, 0, 255)
	};

	//RGB 数据转换为样本数据
	int index = 0;
	for (int row = 0; row<height;row++)
	{
		for (int col = 0;col<width;col++)
		{
			index = row * width + col;
			Vec3b bgr = src.at<Vec3b>(row, col);//三通道取像素值
			points.at<float>(index, 0) = static_cast<int>(bgr[0]);
			points.at<float>(index, 1) = static_cast<int>(bgr[1]);
			points.at<float>(index, 2) = static_cast<int>(bgr[2]);
		}
	}



	//运行K-Means
	TermCriteria criteria = TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1);
	kmeans(points, clusterCount, labels, criteria, 3, KMEANS_PP_CENTERS, centers);

	//显示图像分割结果
	Mat result = Mat::zeros(src.size(), src.type());
	for (int row = 0; row<height;row++)
	{
		for (int col = 0;col<width;col++)
		{
			index = row * width + col;
			int label = labels.at<int>(index, 0);
			//对像素赋值
			result.at<Vec3b>(row, col)[0] = colorTab[label][0];//同一类标签赋值相同颜色
			result.at<Vec3b>(row, col)[1] = colorTab[label][1];
			result.at<Vec3b>(row, col)[2] = colorTab[label][2];
		}
	}

	
	for (int i = 0; i < centers.rows; i++) {
		int x = centers.at<float>(i, 0);
		int y = centers.at<float>(i, 1);
		printf("center %d = c.x : %d, c.y : %d\n", i, x, y);
	}
	
	imshow("KMeans Image Segmentation Demo", result);

	waitKey(0);
	return 0;
}




void KMeansTest()
{


	Mat img(500, 500, CV_8UC3);
	RNG rng(12345);
	Scalar colorTab[] = {
		Scalar(0, 0, 255),
		Scalar(0, 255,0),
		Scalar(255,0, 0),
		Scalar(0,255, 255),
		Scalar(255, 0, 255)
	};
	//类数
	int numCluster = rng.uniform(2, 5);
	cout << "number of clusters:" << numCluster;

	//点数
	int sampleCount = rng.uniform(5, 1000);//153
	Mat points(sampleCount, 1, CV_32FC2);
	Mat labels;
	Mat centers;


	//随机生成聚类中心
	for (int k = 0; k < numCluster; k++)
	{
		Point center;
		center.x = rng.uniform(0, img.cols);
		center.y = rng.uniform(0, img.rows);
		Mat pointChunk = points.rowRange(k*sampleCount / numCluster,
			k == numCluster - 1 ? sampleCount : (k + 1)*sampleCount / numCluster);
		rng.fill(pointChunk, RNG::NORMAL, Scalar(center.x, center.y), Scalar(img.cols*0.05, img.rows*0.05));

	}
	randShuffle(points, 1, &rng);


	//使用KMeans
	kmeans(points, numCluster, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1), 3, KMEANS_PP_CENTERS, centers);

	//使用不同颜色显示
	img = Scalar::all(255);
	for (int i = 0; i < sampleCount; i++)
	{
		int index = labels.at<int>(i);
		Point p = points.at<Point2f>(i);
		circle(img, p, 2, colorTab[index], -1, 8);

	}

	//每个聚类中心绘制圆
	for (int i = 0; i < centers.rows; i++)
	{
		int x = centers.at<float>(i, 0);
		int y = centers.at<float>(i, 1);
		cout << "c.x = " << x << ",  c.y = " << y << endl;
		circle(img, Point(x, y), 40, colorTab[i], 1, LINE_AA);

	}


	imshow("KMeans-Data-Demo", img);
}