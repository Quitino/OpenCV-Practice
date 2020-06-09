#include <opencv2/opencv.hpp>
#include <iostream>


using namespace std;
using namespace cv;
using namespace cv::ml;

void GmmTest();
Mat src, dst;
int main(int argc,char ** argv)
{
	//GmmTest();
	src = imread("src1.jpg");
	if (src.empty())
	{
		cout << "load image err!" << endl;
		return -1;
	}

	imshow("inputImage", src);
	int width = src.cols;
	int height = src.rows;
	int dims = src.channels();


	//初始化定义
	int sampleCount = width * height;//样本数
	int clusterCount = 4;//类别数

	Mat samplePoint(sampleCount, dims, CV_32F);//存储样本点
	Mat labels;//类别标签
	Mat result = Mat::zeros(src.size(), CV_8UC3);//结果显示
	Scalar colorTab[] = {
		Scalar(0, 0, 255),
		Scalar(0, 255, 0),
		Scalar(255, 0, 0),
		Scalar(0, 255, 255),
		Scalar(255, 0, 255)
	};

	//RGB 图像像素转为样本点
	int index = 0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			index = row * width + col;
			Vec3b bgr = src.at<Vec3b>(row, col);//三通道取像素值
			samplePoint.at<float>(index, 0) = static_cast<int>(bgr[0]);
			samplePoint.at<float>(index, 1) = static_cast<int>(bgr[1]);
			samplePoint.at<float>(index, 2) = static_cast<int>(bgr[2]);
		}
	}



	//EM 训练
	Ptr<EM>  em_model = EM::create();
	em_model->setClustersNumber(clusterCount);
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
	em_model->setTermCriteria(TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 100, 0.1));
	em_model->trainEM(samplePoint, noArray(), labels, noArray());
	

	//对每个像素标记颜色与显示
	//Mat sample(dims, 1, CV_64FC1);
	//Mat result = Mat::zeros(src.size(), CV_8UC3);
	//int time = getTickCount();
	//for (int col = 0; col < width; col++)
	//{
	//	for (int row = 0; row < height; row++)
	//	{
	//		index = row * width + col;
	//		int label = labels.at<int>(index, 0);
	//		Scalar c = colorTab[label];
	//		result.at<Vec3b>(row, col)[0] = c[0];
	//		result.at<Vec3b>(row, col)[1] = c[1];
	//		result.at<Vec3b>(row, col)[2] = c[2];
	//	}
	//}

	//对每个像素标记颜色与显示
	Mat sample(dims, 1, CV_64FC1);
	double time = getTickCount();
	int r = 0, g = 0, b = 0;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			index = row * width + col;

			b = src.at<Vec3b>(row, col)[0];
			g = src.at<Vec3b>(row, col)[1];
			r = src.at<Vec3b>(row, col)[2];
			sample.at<double>(0) = b;
			sample.at<double>(1) = g;
			sample.at<double>(2) = r;
			int response = cvRound(em_model->predict2(sample, noArray())[1]);
			Scalar c = colorTab[response];
			result.at<Vec3b>(row, col)[0] = c[0];
			result.at<Vec3b>(row, col)[1] = c[1];
			result.at<Vec3b>(row, col)[2] = c[2];

		}
	}
	printf("execution time(s) : %.2f\n", (getTickCount() - time) / getTickFrequency() );
	imshow("classResult", result);



	waitKey(0);
	return 0;
}

void GmmTest()
{
	Mat img = Mat::zeros(500, 500, CV_8UC3);
	RNG rng(12345);

	Scalar colorTab[] = {
		Scalar(0, 0, 255),
		Scalar(0, 255, 0),
		Scalar(255, 0, 0),
		Scalar(0, 255, 255),
		Scalar(255, 0, 255)
	};

	int numCluster = rng.uniform(2, 5);//类别数
	printf("number of clusters : %d\n", numCluster);

	int sampleCount = rng.uniform(5, 1000);//样本数
	Mat points(sampleCount, 2, CV_32FC1);//存储样本
	Mat labels;//给样本打标签（类别）

	// 生成随机数（样本）
	for (int k = 0; k < numCluster; k++) {
		Point center;
		center.x = rng.uniform(0, img.cols);
		center.y = rng.uniform(0, img.rows);
		Mat pointChunk = points.rowRange(k*sampleCount / numCluster,
			k == numCluster - 1 ? sampleCount : (k + 1)*sampleCount / numCluster);

		rng.fill(pointChunk, RNG::NORMAL, Scalar(center.x, center.y), Scalar(img.cols*0.05, img.rows*0.05));
	}
	randShuffle(points, 1, &rng);//随机打乱矩阵元素

	//分类
	Ptr<EM> em_model = EM::create();//实现方法：期望最大化EM
	em_model->setClustersNumber(numCluster);//设置类别数
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);//设置协方差矩阵类型
	em_model->setTermCriteria(TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 100, 0.1));//设置停止条件，训练100次，误差0.1（EM比较慢）
	em_model->trainEM(points, noArray(), labels, noArray());//训练

	// classify every image pixels
	Mat sample(1, 2, CV_32FC1);
	for (int row = 0; row < img.rows; row++) {
		for (int col = 0; col < img.cols; col++) {
			sample.at<float>(0) = (float)col;
			sample.at<float>(1) = (float)row;
			int response = cvRound(em_model->predict2(sample, noArray())[1]);//开始分类，返回标签
			Scalar c = colorTab[response];
			circle(img, Point(col, row), 1, c*0.75, -1);//按标签将区域进行不同颜色的绘制
		}
	}

	// draw the clusters，样本点的绘制
	for (int i = 0; i < sampleCount; i++) {
		Point p(cvRound(points.at<float>(i, 0)), points.at<float>(i, 1));
		circle(img, p, 1, colorTab[labels.at<int>(i)], -1);
	}

	imshow("GMM-EM Demo", img);
}