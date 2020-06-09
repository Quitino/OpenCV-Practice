//////////////////////////////////////////////////////////////////////////////
//名称：GOCVHelper0.9.cpp
//功能：图像处理和MFC增强
//作者：jsxyhelu(1755311380@qq.com http://jsxyhelu.cnblogs.com)
//组织：GREENOPEN
//日期：2018-10-06
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <io.h>
#include <odbcinst.h>
#include <afxdb.h>
#include "GoCvHelper.h"
#include "opencv/cv.h"
#include "atlstr.h"
RNG  rng(12345);
#define  DEBUG TRUE
//2016年1月26日GoCvHelper添加string 相关操作函数到其他操作中
//2016年1月28日10:45:22 GOCVHelper基于颜色直方图的CBIR到图像操作中去
//2016年8月12日08:27:03 添加关于excel操作相关函数
//2017年6月28日11:04:35 修改一个轮廓排序的BUG
//2018年6月26日08:50:09 解决unicode问题，并且文件改名字了（最主要的问题是将项目设置为 未设置）
//2018年8月7日20:28:22 添加了更为高效的GetPointLineDistance等
namespace GO{

#pragma region 图像增强
	//读取灰度或彩色图片到灰度
	Mat imread2gray(string path){
		Mat src = imread(path);
		Mat srcClone = src.clone();
		if (CV_8UC3 == srcClone.type() )
			cvtColor(srcClone,srcClone,CV_BGR2GRAY);
		return srcClone;
	}

	//带有上下限的threshold
	Mat threshold2(Mat src,int minvalue,int maxvalue){
		Mat thresh1;
		Mat thresh2;
		Mat dst;
		threshold(src,thresh1,minvalue,255, THRESH_BINARY);
		threshold(src,thresh2,maxvalue,255,THRESH_BINARY_INV);
		dst = thresh1 & thresh2;
		return dst;
	}

	//自适应门限的canny算法 
    //canny2
	Mat canny2(Mat src){
		Mat imagetmp = src.clone();
		double low_thresh = 0.0;  
		double high_thresh = 0.0;  
		AdaptiveFindThreshold(imagetmp,&low_thresh,&high_thresh);
		Canny(imagetmp,imagetmp,low_thresh,high_thresh);   
		return imagetmp;}
	void AdaptiveFindThreshold( Mat src,double *low,double *high,int aperture_size){
		const int cn = src.channels();
		Mat dx(src.rows,src.cols,CV_16SC(cn));
		Mat dy(src.rows,src.cols,CV_16SC(cn));
		Sobel(src,dx,CV_16S,1,0,aperture_size,1,0,BORDER_REPLICATE);
		Sobel(src,dy,CV_16S,0,1,aperture_size,1,0,BORDER_REPLICATE);
		CvMat _dx = dx;
		CvMat _dy = dy;
		_AdaptiveFindThreshold(&_dx, &_dy, low, high); }  
	void _AdaptiveFindThreshold(CvMat *dx, CvMat *dy, double *low, double *high){                                                                                
		CvSize size;                                                             
		IplImage *imge=0;                                                        
		int i,j;                                                                 
		CvHistogram *hist;                                                       
		int hist_size = 255;                                                     
		float range_0[]={0,256};                                                 
		float* ranges[] = { range_0 };                                           
		double PercentOfPixelsNotEdges = 0.7;                                    
		size = cvGetSize(dx);                                                    
		imge = cvCreateImage(size, IPL_DEPTH_32F, 1);                            
		// 计算边缘的强度, 并存于图像中                                          
		float maxv = 0;                                                          
		for(i = 0; i < size.height; i++ ){                                                                        
			const short* _dx = (short*)(dx->data.ptr + dx->step*i);          
			const short* _dy = (short*)(dy->data.ptr + dy->step*i);          
			float* _image = (float *)(imge->imageData + imge->widthStep*i);  
			for(j = 0; j < size.width; j++){                                                                
				_image[j] = (float)(abs(_dx[j]) + abs(_dy[j]));          
				maxv = maxv < _image[j] ? _image[j]: maxv;}}                                                                        
		if(maxv == 0){                                                           
			*high = 0;                                                       
			*low = 0;                                                        
			cvReleaseImage( &imge );                                         
			return;}                                                                        
		// 计算直方图                                                            
		range_0[1] = maxv;                                                       
		hist_size = (int)(hist_size > maxv ? maxv:hist_size);                    
		hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);            
		cvCalcHist( &imge, hist, 0, NULL );                                      
		int total = (int)(size.height * size.width * PercentOfPixelsNotEdges);   
		float sum=0;                                                             
		int icount = hist->mat.dim[0].size;                                     
		float *h = (float*)cvPtr1D( hist->bins, 0 );                             
		for(i = 0; i < icount; i++){                                                                        
			sum += h[i];                                                     
			if( sum > total )                                                
				break; }                                                                        
		// 计算高低门限                                                          
		*high = (i+1) * maxv / hist_size ;                                       
		*low = *high * 0.4;                                                      
		cvReleaseImage( &imge );                                                 
		cvReleaseHist(&hist); }     
// end of canny2

	//填充孔洞
    //使用例子
	Mat fillHoles(Mat src){
		Mat dst = getInnerHoles(src);
		threshold(dst,dst,0,255,THRESH_BINARY_INV);
		dst = src + dst;
		return dst;
	}
	//获得图像中白色的比率
	float getWhiteRate(Mat src){
		int iWhiteSum = 0;
		for (int x =0;x<src.rows;x++){
			for (int y=0;y<src.cols;y++){
				if (src.at<uchar>(x,y) != 0)
					iWhiteSum = iWhiteSum +1;
			}
		}
		return (float)iWhiteSum/(float)(src.rows*src.cols);
	}
	//获得内部孔洞图像
	Mat getInnerHoles(Mat src){ 
		Mat clone = src.clone();
		srand((unsigned)time(NULL));  // 生成时间种子
		float fPreRate = getWhiteRate(clone);
		float fAftRate = 0;
		do {
			clone = src.clone();
			// x y 对于 cols rows
			floodFill(clone,Point((int)rand()%src.cols,(int)rand()%src.rows),Scalar(255));
			fAftRate = getWhiteRate(clone);
		} while ( fAftRate < 0.6);
		return clone;
	}
   // end of fillHoles

	//顶帽去光差,radius为模板半径
	Mat moveLightDiff(Mat src,int radius){
		Mat dst;
		Mat srcclone = src.clone();
		Mat mask = Mat::zeros(radius*2,radius*2,CV_8U);
		circle(mask,Point(radius,radius),radius,Scalar(255),-1);
		//顶帽
		erode(srcclone,srcclone,mask);
		dilate(srcclone,srcclone,mask);
		dst =  src - srcclone;
		return dst;
	}

	//将 DEPTH_8U型二值图像进行细化  经典的Zhang并行快速细化算法
    //细化算法
	void thin(const Mat &src, Mat &dst, const int iterations){
		const int height =src.rows -1;
		const int width  =src.cols -1;
		//拷贝一个数组给另一个数组
		if(src.data != dst.data)
			src.copyTo(dst);
		int n = 0,i = 0,j = 0;
		Mat tmpImg;
		uchar *pU, *pC, *pD;
		bool isFinished =FALSE;
		for(n=0; n<iterations; n++){
			dst.copyTo(tmpImg); 
			isFinished =FALSE;   //一次 先行后列扫描 开始
			//扫描过程一 开始
			for(i=1; i<height;  i++) {
				pU = tmpImg.ptr<uchar>(i-1);
				pC = tmpImg.ptr<uchar>(i);
				pD = tmpImg.ptr<uchar>(i+1);
				for(int j=1; j<width; j++){
					if(pC[j] > 0){
						int ap=0;
						int p2 = (pU[j] >0);
						int p3 = (pU[j+1] >0);
						if (p2==0 && p3==1)
							ap++;
						int p4 = (pC[j+1] >0);
						if(p3==0 && p4==1)
							ap++;
						int p5 = (pD[j+1] >0);
						if(p4==0 && p5==1)
							ap++;
						int p6 = (pD[j] >0);
						if(p5==0 && p6==1)
							ap++;
						int p7 = (pD[j-1] >0);
						if(p6==0 && p7==1)
							ap++;
						int p8 = (pC[j-1] >0);
						if(p7==0 && p8==1)
							ap++;
						int p9 = (pU[j-1] >0);
						if(p8==0 && p9==1)
							ap++;
						if(p9==0 && p2==1)
							ap++;
						if((p2+p3+p4+p5+p6+p7+p8+p9)>1 && (p2+p3+p4+p5+p6+p7+p8+p9)<7){
							if(ap==1){
								if((p2*p4*p6==0)&&(p4*p6*p8==0)){                           
									dst.ptr<uchar>(i)[j]=0;
									isFinished =TRUE;                            
								}
							}
						}                    
					}

				} //扫描过程一 结束
				dst.copyTo(tmpImg); 
				//扫描过程二 开始
				for(i=1; i<height;  i++){
					pU = tmpImg.ptr<uchar>(i-1);
					pC = tmpImg.ptr<uchar>(i);
					pD = tmpImg.ptr<uchar>(i+1);
					for(int j=1; j<width; j++){
						if(pC[j] > 0){
							int ap=0;
							int p2 = (pU[j] >0);
							int p3 = (pU[j+1] >0);
							if (p2==0 && p3==1)
								ap++;
							int p4 = (pC[j+1] >0);
							if(p3==0 && p4==1)
								ap++;
							int p5 = (pD[j+1] >0);
							if(p4==0 && p5==1)
								ap++;
							int p6 = (pD[j] >0);
							if(p5==0 && p6==1)
								ap++;
							int p7 = (pD[j-1] >0);
							if(p6==0 && p7==1)
								ap++;
							int p8 = (pC[j-1] >0);
							if(p7==0 && p8==1)
								ap++;
							int p9 = (pU[j-1] >0);
							if(p8==0 && p9==1)
								ap++;
							if(p9==0 && p2==1)
								ap++;
							if((p2+p3+p4+p5+p6+p7+p8+p9)>1 && (p2+p3+p4+p5+p6+p7+p8+p9)<7){
								if(ap==1){
									if((p2*p4*p8==0)&&(p2*p6*p8==0)){                           
										dst.ptr<uchar>(i)[j]=0;
										isFinished =TRUE;                            
									}
								}
							}                    
						}
					}
				} //一次 先行后列扫描完成          
				//如果在扫描过程中没有删除点，则提前退出
				if(isFinished ==FALSE)
					break; 
			}
		}
	}
// end of thin

	//使得rect区域半透明
	Mat translucence(Mat src,Rect rect,int idepth){
		Mat dst = src.clone();
		Mat roi = dst(rect);
		roi += Scalar(idepth,idepth,idepth);
		return dst;
	}

	//使得rect区域打上马赛克
	Mat mosaic(Mat src,Rect rect,int W,int H){
		Mat dst = src.clone();
		Mat roi = dst(rect);
		for (int i=W; i<roi.cols; i+=W) {
			for (int j=H; j<roi.rows; j+=H) {
				uchar s=roi.at<uchar>(j-H/2,(i-W/2)*3);
				uchar s1=roi.at<uchar>(j-H/2,(i-W/2)*3+1);
				uchar s2=roi.at<uchar>(j-H/2,(i-W/2)*3+2);
				for (int ii=i-W; ii<=i; ii++) {
					for (int jj=j-H; jj<=j; jj++) {
						roi.at<uchar>(jj,ii*3+0)=s;
						roi.at<uchar>(jj,ii*3+1)=s1;
						roi.at<uchar>(jj,ii*3+2)=s2;
					}
				}
			}
		}
		return dst;
	}


//基于颜色直方图的距离计算
double GetHsVDistance(Mat src_base,Mat src_test1){
	Mat   hsv_base;
	Mat   hsv_test1;
	///  Convert  to  HSV
	cvtColor(  src_base,  hsv_base,  COLOR_BGR2HSV  );
	cvtColor(  src_test1,  hsv_test1,  COLOR_BGR2HSV  );
	///  Using  50  bins  for  hue  and  60  for  saturation
	int  h_bins  =  50;  int  s_bins  =  60;
	int  histSize[]  =  {  h_bins,  s_bins  };
	//  hue  varies  from  0  to  179,  saturation  from  0  to  255
	float  h_ranges[]  =  {  0,  180  };
	float  s_ranges[]  =  {  0,  256  };
	const  float*  ranges[]  =  {  h_ranges,  s_ranges  };
	//  Use  the  o-th  and  1-st  channels
	int  channels[]  =  {  0,  1  };
	///  Histograms
	MatND  hist_base;
	MatND  hist_test1;
	///  Calculate  the  histograms  for  the  HSV  images
	calcHist(  &hsv_base,  1,  channels,  Mat(),  hist_base,  2,  histSize,  ranges,  true,  false  );
	normalize(  hist_base,  hist_base,  0,  1,  NORM_MINMAX,  -1,  Mat()  );
	calcHist(  &hsv_test1,  1,  channels,  Mat(),  hist_test1,  2,  histSize,  ranges,  true,  false  );
	normalize(  hist_test1,  hist_test1,  0,  1,  NORM_MINMAX,  -1,  Mat()  );
	///  Apply  the  histogram  comparison  methods
	double  base_test1  =  compareHist(  hist_base,  hist_test1,  0  );
	return base_test1;
}
// Multiply 正片叠底
void Multiply(Mat& src1, Mat& src2, Mat& dst)
{
	for(int index_row=0; index_row<src1.rows; index_row++)
	{
		for(int index_col=0; index_col<src1.cols; index_col++)
		{
			for(int index_c=0; index_c<3; index_c++)
				dst.at<Vec3f>(index_row, index_col)[index_c]=
				src1.at<Vec3f>(index_row, index_col)[index_c]*
				src2.at<Vec3f>(index_row, index_col)[index_c];
		}
	}
}
// Color_Burn 颜色加深
void Color_Burn(Mat& src1, Mat& src2, Mat& dst)
{
	for(int index_row=0; index_row<src1.rows; index_row++)
	{
		for(int index_col=0; index_col<src1.cols; index_col++)
		{
			for(int index_c=0; index_c<3; index_c++)
				dst.at<Vec3f>(index_row, index_col)[index_c]=1-
				(1-src1.at<Vec3f>(index_row, index_col)[index_c])/
				src2.at<Vec3f>(index_row, index_col)[index_c];
		}
	}
}
// 线性增强
void Linear_Burn(Mat& src1, Mat& src2, Mat& dst)
{
	for(int index_row=0; index_row<src1.rows; index_row++)
	{
		for(int index_col=0; index_col<src1.cols; index_col++)
		{
			for(int index_c=0; index_c<3; index_c++)
				dst.at<Vec3f>(index_row, index_col)[index_c]=max(
				src1.at<Vec3f>(index_row, index_col)[index_c]+
				src2.at<Vec3f>(index_row, index_col)[index_c]-1, (float)0.0);
		}
	}
}


//点乘法 elementWiseMultiplication
Mat EWM(Mat m1,Mat m2){
	Mat dst=m1.mul(m2);
	return dst;
}
//图像局部对比度增强算法
Mat ACE(Mat src,int C,int n,int MaxCG){
	Mat meanMask;
	Mat varMask;
	Mat meanGlobal;
	Mat varGlobal;
	Mat dst;
	Mat tmp;
	Mat tmp2;
	blur(src.clone(),meanMask,Size(50,50));//meanMask为局部均值 
	tmp = src - meanMask;  
	varMask = EWM(tmp,tmp);         
	blur(varMask,varMask,Size(50,50));    //varMask为局部方差   
	//换算成局部标准差
	varMask.convertTo(varMask,CV_32F);
	for (int i=0;i<varMask.rows;i++){
		for (int j=0;j<varMask.cols;j++){
			varMask.at<float>(i,j) =  (float)sqrt(varMask.at<float>(i,j));
		}
	}
	meanStdDev(src,meanGlobal,varGlobal); //meanGlobal为全局均值 varGlobal为全局标准差
	tmp2 = varGlobal/varMask;
	for (int i=0;i<tmp2.rows;i++){
		for (int j=0;j<tmp2.cols;j++){
			if (tmp2.at<float>(i,j)>MaxCG){
				tmp2.at<float>(i,j) = MaxCG;
			}
		}
	}
	tmp2.convertTo(tmp2,CV_8U);
	tmp2 = EWM(tmp2,tmp);
	dst = meanMask + tmp2;
	imshow("D方法",dst);
	dst = meanMask + C*tmp;
	imshow("C方法",dst);
	return dst;
}

//Local Normalization  input is 32f1u
Mat LocalNormalization(Mat float_gray,float sigma1,float sigma2){
	Mat gray, blur, num, den;
	float_gray.convertTo(float_gray, CV_32F, 1.0/255.0);
	// numerator = img - gauss_blur(img)
	boxFilter(float_gray,blur,float_gray.depth(),Size(sigma1,sigma1));
	num = float_gray - blur;
	boxFilter(num.mul(num),blur,num.depth(),Size(sigma2,sigma2));
	// denominator = sqrt(gauss_blur(img^2))
	pow(blur, 0.5, den);
	// output = numerator / denominator
	gray = num / den;
	// normalize output into [0,1]
	normalize(gray, gray, 0.0, 1.0, NORM_MINMAX, -1);
	return gray;
}
#pragma endregion 图像增强

#pragma region 图像处理
	//寻找最大的轮廓
	VP FindBigestContour(Mat src){    
		int imax = 0; //代表最大轮廓的序号
		int imaxcontour = -1; //代表最大轮廓的大小
		std::vector<std::vector<Point>>contours;    
		findContours(src,contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);
		for (int i=0;i<contours.size();i++){
			int itmp =  contourArea(contours[i]);//这里采用的是轮廓大小
			if (imaxcontour < itmp ){
				imax = i;
				imaxcontour = itmp;
			}
		}
		return contours[imax];
	}
	//寻找第nth的轮廓
	//ith = 0代表最大，ith=1 代表第2个，以此类推
	bool sortfunction (std::vector<Point> c1,std::vector<Point> c2) { return (contourArea(c1)>contourArea(c2)); }  
	VP FindnthContour(Mat src,int ith ){    
		std::vector<std::vector<Point>>contours;    
		findContours(src,contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);
	    std::sort(contours.begin(),contours.end(),sortfunction);
		return contours[ith];
	}
	//寻找并绘制出彩色联通区域
	vector<VP> connection2(Mat src,Mat& draw){    
		draw = Mat::zeros(src.rows,src.cols,CV_8UC3);
		vector<VP>contours;    
		findContours(src.clone(),contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);
		//由于给大的区域着色会覆盖小的区域，所以首先进行排序操作
		//冒泡排序，由小到大排序
		VP vptmp;
		for(int i=1;i<contours.size();i++){
			for(int j=contours.size()-1;j>=i;j--){
				if (contourArea(contours[j]) < contourArea(contours[j-1]))
				{
					vptmp = contours[j-1];
					contours[j-1] = contours[j];
					contours[j] = vptmp;
				}
			}
		}
		//打印结果
		for (int i=contours.size()-1;i>=0;i--){
			Scalar  color  = Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));
			drawContours(draw,contours,i,color,-1);
		}
		return contours;
	}
	vector<VP> connection2(Mat src){
		Mat draw;
		return connection2(src,draw);
	}

	//根据轮廓的面积大小进行选择
	vector<VP>  selectShapeArea(Mat src,Mat& draw,vector<VP> contours,int minvalue,int maxvalue){
		vector<VP> result_contours;
		draw = Mat::zeros(src.rows,src.cols,CV_8UC3);
		for (int i=0;i<contours.size();i++){ 
			double countour_area = contourArea(contours[i]);
			if (countour_area >minvalue && countour_area<maxvalue)
				result_contours.push_back(contours[i]);
		}
		for (int i=0;i<result_contours.size();i++){
			int iRandB = rng.uniform(0,255);
			int iRandG = rng.uniform(0,255);
			int iRandR = rng.uniform(0,255);
			Scalar  color  = Scalar(iRandB,iRandG,iRandR);
			drawContours(draw,result_contours,i,color,-1);
			char cbuf[100];sprintf_s(cbuf,"%d",i+1);
			//寻找最小覆盖圆,求出圆心。使用反色打印轮廓项目
			float radius;
			Point2f center;
			minEnclosingCircle(result_contours[i],center,radius);
			putText(draw,cbuf,center, FONT_HERSHEY_PLAIN ,5,Scalar(255-iRandB,255-iRandG,255-iRandR),5);
		}
		return result_contours;
	}
	vector<VP>  selectShapeArea(vector<VP> contours,int minvalue,int maxvalue)
	{
		vector<VP> result_contours;
		for (int i=0;i<contours.size();i++){ 
			double countour_area = contourArea(contours[i]);
			if (countour_area >minvalue && countour_area<maxvalue)
				result_contours.push_back(contours[i]);
		}
		return result_contours;
	}

	vector<VP> selectShapeCircularity(Mat src,Mat& draw,vector<VP> contours,float minvalue,float maxvalue){
		vector<VP> result_contours;
		draw = Mat::zeros(src.rows,src.cols,CV_8UC3);
		for (int i=0;i<contours.size();i++){
			float fcompare = calculateCircularity(contours[i]);
			if (fcompare >=minvalue && fcompare <=maxvalue)
				result_contours.push_back(contours[i]);
		}
		for (int i=0;i<result_contours.size();i++){
			Scalar  color  = Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));
			drawContours(draw,result_contours,i,color,-1);
		}
		return result_contours;
	}
	vector<VP> selectShapeCircularity(vector<VP> contours,float minvalue,float maxvalue){
		vector<VP> result_contours;
		for (int i=0;i<contours.size();i++){
			float fcompare = calculateCircularity(contours[i]);
			if (fcompare >=minvalue && fcompare <=maxvalue)
				result_contours.push_back(contours[i]);
		}
		return result_contours;
	}
	//计算轮廓的圆的特性
	float calculateCircularity(VP contour){
		Point2f center;
		float radius = 0;
		minEnclosingCircle((Mat)contour,center,radius);
		//以最小外接圆半径作为数学期望，计算轮廓上各点到圆心距离的标准差
		float fsum = 0;
		float fcompare = 0;
		for (int i=0;i<contour.size();i++){   
			Point2f ptmp = contour[i];
			float fdistenct = sqrt((float)((ptmp.x - center.x)*(ptmp.x - center.x)+(ptmp.y - center.y)*(ptmp.y-center.y)));
			float fdiff = abs(fdistenct - radius);
			fsum = fsum + fdiff;
		}
		fcompare = fsum/(float)contour.size();
		return fcompare;
	}

	//返回两点之间的距离
	float getDistance(Point2f f1,Point2f f2)
	{
		return sqrt((float)(f1.x - f2.x)*(f1.x - f2.x) + (f1.y -f2.y)*(f1.y- f2.y));
	}
	//返回点到直线（线段）的距离
	float GetPointLineDistance(Point2f pointInput,Point2f pa,Point2f pb,Point2f& pointOut)
	{
		
		Point2f p1;
		Point2f p2;
		if (pa.x<pb.x)
		{
			p1 = pa;
			p2 = pb;
		}
		else
		{
			p1 = pb;
			p2 = pa;
		}
		//分支考虑
		if (p1.x == p2.x)
		{
			pointOut.x = p1.x ;
			pointOut.y = pointInput.y;
			return abs(pointInput.x - p1.x);
		}

		if (p1.y == p2.y)
		{
			pointOut.y = p1.y ;
			pointOut.x = pointInput.x;
			return abs(pointInput.y - p1.y);
		}

		float fthea = (p2.y - p1.y)/(p2.x-p1.x);
		int fMinDistance = 100000;
		int fMinNum = -1;
		for (int i=0;i<(int)(p2.x-p1.x);i++)
		{
			float fx = p1.x +i;
			float fy = i*fthea + p1.y;
			float ftmp =  GO::getDistance(Point2f(fx,fy),pointInput);
			if (ftmp<fMinDistance)
			{
				fMinDistance = ftmp;
				fMinNum = i;
			}
		}
		//测试画图
		pointOut.x = p1.x +fMinNum;
		pointOut.y = fMinNum*fthea + p1.y;
		return fMinDistance;
	}

	//返回点到直线（线段）的距离,换成了更精简的代码，也许也更高效
	float GetPointLineDistance(Mat src,Point2f pointInput,Point2f pa,Point2f pb,Point2f& pointOut)
	{
		LineIterator it(src,pa,pb);
		int fMinDistance = 100000;
		int fMinNum = -1;
		for(int i = 0; i < it.count; i++, ++it)
		{
			float ftmp =  GO::getDistance(it.pos(),pointInput);
			if (ftmp<fMinDistance)
			{
				fMinDistance = ftmp;
				pointOut=it.pos;
			}
		}
		return fMinDistance;
	}
	//获得构建的主要方向，在图上进行标徽，并且返回角度结果
	//注意，这个函数，在opencv里面已经并入标准库了
	double getOrientation(vector<Point> &pts, Mat &img)
	{
		//构建pca数据。这里做的是将轮廓点的x和y作为两个维压到data_pts中去。
		Mat data_pts = Mat(pts.size(), 2, CV_64FC1);//使用mat来保存数据，也是为了后面pca处理需要
		for (int i = 0; i < data_pts.rows; ++i)
		{
			data_pts.at<double>(i, 0) = pts[i].x;
			data_pts.at<double>(i, 1) = pts[i].y;
		}
		//执行PCA分析
		PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);
		//获得最主要分量，在本例中，对应的就是轮廓中点，也是图像中点
		Point pos = Point(pca_analysis.mean.at<double>(0, 0),pca_analysis.mean.at<double>(0, 1));
		//存储特征向量和特征值
		vector<Point2d> eigen_vecs(2);
		vector<double> eigen_val(2);
		for (int i = 0; i < 2; ++i)
		{
			eigen_vecs[i] = Point2d(pca_analysis.eigenvectors.at<double>(i, 0),pca_analysis.eigenvectors.at<double>(i, 1));
			eigen_val[i] = pca_analysis.eigenvalues.at<double>(i,0);//注意，这个地方原代码写错了
		}
		//在轮廓/图像中点绘制小圆
		circle(img, pos, 3, CV_RGB(255, 0, 255), 2);
		//计算出直线，在主要方向上绘制直线
		line(img, pos, pos + 0.02 * Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]) , CV_RGB(255, 255, 0),3);
		line(img, pos, pos + 0.02 * Point(eigen_vecs[1].x * eigen_val[1], eigen_vecs[1].y * eigen_val[1]) , CV_RGB(0, 255, 255),3);
		//返回角度结果
		return atan2(eigen_vecs[0].y, eigen_vecs[0].x);
	}

	//根据中线将轮廓分为2个部分
	//pts 轮廓
	//pa pb 中线线段端点
	//p1 p2 分为两边后最远2点
	//lenght1,length2 对应距离
	//img 用于绘图
	//返回 是否分割成功
	bool SplitContoursByMiddleLine(vector<Point> &pts,Mat &img,Point pa,Point pb,Point& p1,float& length1,Point& p2,float& length2)
	{
		//寻找轮廓到中线(实际上是线段）的交点
		int isum = 0;
		Point2f pointOut;
		//bool bIsCross =false;
		int iStart = -1;
		int iEnd = -1;
		vector<int> vecBorderPoints;
		float fDistance = 0;
		//将轮廓划分为两个部分
		for (int i = 0;i< pts.size();i++)
		{
			float f = GetPointLineDistance(img,pts[i], pa,pb,pointOut);//获得轮廓上所有点最远距离点
			
		}

		//对所有 轮廓和边缘的交点 进行排序，得到距离最远的点对 
		float fDistance = 0;
		for (int i = 0 ;i<vecBorderPoints.size();i++)
		{
			for (int j = i;j<vecBorderPoints.size();j++) //已经进行排序优化了
			{
				if (getDistance(pts[vecBorderPoints[i]],pts[vecBorderPoints[j]]) > fDistance)
				{
					fDistance = getDistance(pts[vecBorderPoints[i]],pts[vecBorderPoints[j]]);
					iStart = vecBorderPoints[i];
					iEnd = vecBorderPoints[j];
				}
			}
		}
		if (-1 == iEnd ) //出现问题了，交给比较方便的方法吧
			return false;
		if (iStart > iEnd)
			swap(iStart,iEnd);
		if ((iEnd - iStart)<pts.size()/4)
			return false;//错误控制机制
		if (DEBUG)
			{
				printf("\n\n");
				circle(img,pts[iStart],5,Scalar(0,255,0),5);
				circle(img,pts[iEnd],5,Scalar(0,255,0),5);
			}
			

		vector<Point> vector1;
		vector<Point> vector2;
		for (int i = 0;i<pts.size();i++)
		{
			if (i>=iStart && i<=iEnd)
			{
				vector1.push_back(pts[i]);
				if(DEBUG)
					circle(img,pts[i],3,Scalar(0,0,255));
			}	
			else
			{
				vector2.push_back(pts[i]);
				if(DEBUG)
					circle(img,pts[i],3,Scalar(0,255,255));
			}

		}
		//分别在这两个轮廓里面找到交点距离
		Point pstart = pts[iStart];
		Point pend   = pts[iEnd];
		float fmax = -1;int imax = -1;
		for (int i =0;i<vector1.size();i++)
		{
			float f = GetPointLineDistance(vector1[i], pa,pb,pointOut);
			if (f>fmax) //冒泡
			{
				fmax = f;
				imax = i;
			}
		}
		if (DEBUG)
			circle(img,vector1[imax],3,cv::Scalar(255,0,0),2);
		p1 = vector1[imax];
		length1 = fmax;

		fmax = -1; imax = -1;
		for (int i =0;i<vector2.size();i++)
		{
			float f = GetPointLineDistance(vector2[i], pa,pb,pointOut);
			if (f>fmax) //冒泡
			{
				fmax = f;
				imax = i;
			}
		}
		if (DEBUG)
			circle(img,vector2[imax],3,cv::Scalar(255,0,0),2);
		p2 = vector2[imax];
		length2 = fmax;
		return true;
	}

	//获得真实的长宽,返回值为false的话代表识别不成功
	bool getRealWidthHeight(vector<Point> &pts,vector<Point> &resultPts, Mat &img,float& flong,float& fshort)
	{
		//构建pca数据。这里做的是将轮廓点的x和y作为两个维压到data_pts中去。
		Mat data_pts = Mat(pts.size(), 2, CV_64FC1);//使用mat来保存数据，也是为了后面pca处理需要
		for (int i = 0; i < data_pts.rows; ++i)
		{
			data_pts.at<double>(i, 0) = pts[i].x;
			data_pts.at<double>(i, 1) = pts[i].y;
		}
		//执行PCA分析
		PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);
		//获得最主要分量，在本例中，对应的就是轮廓中点，也是图像中点
		Point pos = Point(pca_analysis.mean.at<double>(0, 0),pca_analysis.mean.at<double>(0, 1));
		//获得特征向量和特征值
		vector<Point2d> eigen_vecs(2);
		vector<double> eigen_val(2);
		for (int i = 0; i < 2; ++i)
		{
			eigen_vecs[i] = Point2d(pca_analysis.eigenvectors.at<double>(i, 0),pca_analysis.eigenvectors.at<double>(i, 1));
			eigen_val[i] = pca_analysis.eigenvalues.at<double>(i,0);
		}
		if (eigen_vecs[0].x == 0 || abs(eigen_vecs[0].y / eigen_vecs[0].x) >100)//一般出现在中线为垂直情况，这个时候可以直接采用简单方法
			return false;
		if (eigen_vecs[1].x == 0 || abs(eigen_vecs[1].y / eigen_vecs[1].x) >100)
			return false;
		//在轮廓/图像中点绘制小圆
		if (DEBUG)
			circle(img, pos, 3, CV_RGB(255, 0, 255), 2);

		//获得长短轴和轮廓的交接点，首先得到一条绝对可行的直线
		//长轴
		Point pa = pos-0.04 * Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]); 
		Point pb = pos + 0.04 * Point(eigen_vecs[0].x * eigen_val[0], eigen_vecs[0].y * eigen_val[0]) ;
		//短轴
		Point pc = pos- 0.2 * Point(eigen_vecs[1].x * eigen_val[1], eigen_vecs[1].y * eigen_val[1]);
		Point pd = pos + 0.2 * Point(eigen_vecs[1].x * eigen_val[1], eigen_vecs[1].y * eigen_val[1]);
		//通过对边界进行限定，提高算法效率，获得的结果能够保证此时的pa,pb都是最远的点
		LineIterator it(img, pa, pb);
		LineIterator it2(img, pb, pa);
		LineIterator it3(img, pc, pd);
		LineIterator it4(img, pd, pc);
		for(int i = 0; i < it.count; i++, ++it)
		{
			if( 0 == pointPolygonTest(pts,it.pos(),true))//第一个在轮廓上的点
				pa = it.pos();
		}
		
		for(int i = 0; i < it2.count; i++, ++it2)
		{
			if( 0 == pointPolygonTest(pts,it2.pos(),true))//第一个在轮廓上的点
				pb = it2.pos();
		}
		
		for(int i = 0; i < it3.count; i++, ++it3)
		{
			if( 0 == pointPolygonTest(pts,it3.pos(),true))//第一个在轮廓上的点
				pc = it3.pos();
		}
	
		for(int i = 0; i < it4.count; i++, ++it4)
		{
			if( 0 == pointPolygonTest(pts,it4.pos(),true))//第一个在轮廓上的点
				pd = it4.pos();
		}
		//计算出直线，在长短轴上绘制直线
		if (DEBUG)
		{
			line(img, pa,pb , CV_RGB(255, 255, 0),10);
			line(img, pc,pd, CV_RGB(0, 255, 255),10);
		}

		//将 轮廓按照长短轴进行划分.这里_p[]得到的是4个边界最远点；而_length则是长度
		Point _p[4]; 
		float _length[4] = {-1,-1,-1,-1};
		if (!SplitContoursByMiddleLine(pts,img,pa,pb,_p[0],_length[0],_p[1],_length[1]))
			return false;
		if (!SplitContoursByMiddleLine(pts,img,pc,pd,_p[2],_length[2],_p[3],_length[3]))
			return false;

		//开始获得结论
		if (eigen_vecs[0].x == 0 || eigen_vecs[1].x == 0)//除数为0
			return false;
		float k_long = eigen_vecs[0].y /eigen_vecs[0].x;
		float k_short = eigen_vecs[1].y /eigen_vecs[1].x;
		if (k_long == k_short)//这种情况不应该出现
			return false;

		//返回长度
		if (_length[0]<0 || _length[1]<0 || _length[2]<0 || _length[3]<0)
			return false;
		fshort = _length[0]+_length[1];
		flong  = _length[2]+_length[3];
		//通过解析方法，获得最后结果 
		Point p[4]; 
		p[0].x = (k_long * _p[0].x   - k_short * _p[2].x  +  _p[2].y - _p[0].y)  / (k_long - k_short);
		p[0].y = (p[0].x - _p[0].x)*k_long + _p[0].y;
		p[1].x = (k_long * _p[0].x   - k_short * _p[3].x  +  _p[3].y - _p[0].y)  / (k_long - k_short);
		p[1].y = (p[1].x - _p[0].x)*k_long + _p[0].y;
		p[2].x = (k_long * _p[1].x   - k_short * _p[2].x  +  _p[2].y - _p[1].y)  / (k_long - k_short);
		p[2].y = (p[2].x - _p[1].x)*k_long + _p[1].y;
		p[3].x = (k_long * _p[1].x   - k_short * _p[3].x  +  _p[3].y - _p[1].y)  / (k_long - k_short);
		p[3].y = (p[3].x - _p[1].x)*k_long + _p[1].y;

		//简单排序
		if (p[1].x < p[0].x)
			swap(p[1],p[0]);
		if (p[3].x < p[2].x)
			swap(p[3],p[2]);
		//绘图
		for (int i = 0;i<4;i++)
			resultPts.push_back(p[i]);
		
		//line(img,p[0],p[1],CV_RGB(0, 255, 255), 5);
		//line(img,p[0],p[2],CV_RGB(0, 255, 255), 5);
		//line(img,p[3],p[1],CV_RGB(0, 255, 255), 5);
		//line(img,p[3],p[2],CV_RGB(0, 255, 255), 5);

		return true;

	}

	//投影到x或Y轴上,上波形为vup,下波形为vdown,gap为误差间隔
	void projection2(Mat src,vector<int>& vup,vector<int>& vdown,int direction,int gap){
		Mat tmp = src.clone();
		vector<int> vdate;
		if (DIRECTION_X == direction){
			for (int i=0;i<tmp.cols;i++){
				Mat data = tmp.col(i);
				int itmp = countNonZero(data);
				vdate.push_back(itmp);
			}
		}else{
			for (int i=0;i<tmp.rows;i++){
				Mat data = tmp.row(i);
				int itmp = countNonZero(data);
				vdate.push_back(itmp);
			}
		}
		//整形,去除长度小于gap的零的洞
		if (vdate.size()<=gap)
			return;
		for (int i=0;i<vdate.size()-gap;i++){
			if (vdate[i]>0 && vdate[i+gap]>0){
				for (int j=i;j<i+gap;j++){
					vdate[j] = 1;
				}
				i = i+gap-1;
			}
		}
		//记录上下沿
		for (int i=1;i<vdate.size();i++){
			if (vdate[i-1] == 0 && vdate[i]>0)
				vup.push_back(i);
			if (vdate[i-1]>0 && vdate[i] == 0)
				vdown.push_back(i);
		}
	}
	//轮廓柔化
	bool SmoothEdgeSingleChannel( Mat mInput,Mat &mOutput, double amount, double radius, uchar Threshold) 
	{
		if(mInput.empty())
		{
			return 0;
		}
		if(radius<1)
			radius=1;

		Mat mGSmooth,mDiff,mAbsDiff;
		mOutput = Mat(mInput.size(),mInput.type());

		GaussianBlur(mInput,mGSmooth,Size(0,0),radius); 
		//imshow("mGSmooth",mGSmooth);

		subtract(mGSmooth,mInput,mDiff);
		//imshow("mDiff",mDiff);

		mDiff*=amount;
		threshold(abs(2* mDiff),mAbsDiff,Threshold,255,THRESH_BINARY_INV);

		mDiff.setTo(Scalar(0),mAbsDiff);
		//imshow("mDiff Multiplied",mDiff);

		add(mInput,mDiff,mOutput);

		return true;
	}
#pragma endregion 图像处理

#pragma region 文件操作
	//递归读取目录下全部文件
	void getFiles(string path, vector<string>& files,string flag){
		//文件句柄
		long   hFile   =   0;
		//文件信息
		struct _finddata_t fileinfo;
		string p;
		if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1){
			do{
				//如果是目录,迭代之,如果不是,加入列表
				if((fileinfo.attrib &  _A_SUBDIR)){
					if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0 && flag=="r")
						getFiles( p.assign(path).append("\\").append(fileinfo.name), files,flag );
				}
				else{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
				}
			}while(_findnext(hFile, &fileinfo)  == 0);
			_findclose(hFile);
		}
	}
	//递归读取目录下全部图片
	void getFiles(string path, vector<Mat>& files,string flag){
		vector<string> fileNames;
		getFiles(path,fileNames,flag);
		for (int i=0;i<fileNames.size();i++){
			Mat tmp = imread(fileNames[i]);
			if (tmp.rows>0)//如果是图片
				files.push_back(tmp);
		}
	}
	//递归读取目录下全部图片和名称
	void getFiles(string path, vector<pair<Mat,string>>& files,string flag){
		vector<string> fileNames;
		getFiles(path,fileNames,flag);
		for (int i=0;i<fileNames.size();i++){
			Mat tmp = imread(fileNames[i]);
			if (tmp.rows>0){
				pair<Mat,string> apir;
				apir.first = tmp;
				apir.second = fileNames[i];
				files.push_back(apir);
			}
		}
	}
	////删除目录下的全部文件
	void deleteFiles(string path,string flag){
		//文件句柄
		long   hFile   =   0;
		//文件信息
		struct _finddata_t fileinfo;
		string p;
		if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1){
			do{
				//如果是目录,迭代之,如果不是,加入列表
				if((fileinfo.attrib &  _A_SUBDIR)){
					if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0 && flag=="r")
						deleteFiles(p.assign(path).append("\\").append(fileinfo.name).c_str(),flag );
				}
				else{
					deleteFiles(p.assign(path).append("\\").append(fileinfo.name).c_str());
				}
			}while(_findnext(hFile, &fileinfo)  == 0);
			_findclose(hFile);
		}
	}
	//创建或续写目录下的csv文件,填写“文件位置-分类”对
	int writeCsv(const string& filename,const vector<pair<string,string>>srcVect,char separator ){
		ofstream file(filename.c_str(),ofstream::app);
		if (!file)
			return 0;
		for (int i=0;i<srcVect.size();i++){
			file<<srcVect[i].first<<separator<<srcVect[i].second<<endl;
		}
		return srcVect.size();
	}
	//读取目录下的csv文件,获得“文件位置-分类”对
	vector<pair<string,string>> readCsv(const string& filename, char separator) {
		pair<string,string> apair;
		string line, path, classlabel;
		vector<pair<string,string>> retVect;
		ifstream file(filename.c_str(), ifstream::in);
		if (!file) 
			return retVect;
		while (getline(file, line)) {
			stringstream liness(line);
			getline(liness, path, separator);
			getline(liness, classlabel);
			if(!path.empty() && !classlabel.empty()) {
				apair.first = path;
				apair.second = classlabel;
				retVect.push_back(apair);
			}

		}
		return retVect;
	}
	////获得ini文件中的值
	 CString  GetInitString( CString Name1 ,CString Name2){
		char c[100] ;
		memset( c ,0 ,100) ;
		CString csCfgFilePath;
		GetModuleFileName(NULL, csCfgFilePath.GetBufferSetLength(MAX_PATH+1), MAX_PATH); 
		csCfgFilePath.ReleaseBuffer(); 
		int nPos = csCfgFilePath.ReverseFind ('\\');
		csCfgFilePath = csCfgFilePath.Left (nPos);
		csCfgFilePath += "\\Config" ;
		BOOL br = GetPrivateProfileString(Name1,Name2 ,"0",c, 100 , csCfgFilePath) ;
		CString rstr ;
		rstr.Format("%s" , c) ;
		return rstr ;
	}
	 //写入ini文件中的值
	 void WriteInitString( CString Name1 ,CString Name2 ,CString strvalue){
		CString csCfgFilePath;
		GetModuleFileName(NULL, csCfgFilePath.GetBufferSetLength(MAX_PATH+1), MAX_PATH); 
		csCfgFilePath.ReleaseBuffer(); 
		int nPos = csCfgFilePath.ReverseFind ('\\');
		csCfgFilePath = csCfgFilePath.Left (nPos);
		csCfgFilePath += "\\Config" ;
		BOOL br = WritePrivateProfileString(Name1 ,Name2 ,strvalue ,csCfgFilePath) ;
		if ( !br)
			TRACE("savewrong") ;
	}

	////获得当前目录路径
	//static CString GetLocalPath(){
	//	CString csCfgFilePath;
	//	GetModuleFileName(NULL, csCfgFilePath.GetBufferSetLength(MAX_PATH+1), MAX_PATH); 
	//	csCfgFilePath.ReleaseBuffer(); 
	//	int nPos = csCfgFilePath.ReverseFind ('\\');
	//	csCfgFilePath = csCfgFilePath.Left (nPos);
	//	return csCfgFilePath;
	//}

	//获得.exe路径
	static CString GetExePath()
	{
		CString strPath;
		GetModuleFileName(NULL,strPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
		strPath.ReleaseBuffer();
		return strPath;
	}

	//开机自动运行
	static BOOL SetAutoRun(CString strPath,bool flag)
	{
		CString str;
		HKEY hRegKey;
		BOOL bResult;
		str=_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
		if(RegOpenKey(HKEY_LOCAL_MACHINE, str, &hRegKey) != ERROR_SUCCESS) 
			bResult=FALSE;
		else
		{
			_splitpath(strPath.GetBuffer(0),NULL,NULL,str.GetBufferSetLength(MAX_PATH+1),NULL);
			strPath.ReleaseBuffer();
			str.ReleaseBuffer();//str是键的名字
			if (flag){
				if(::RegSetValueEx( hRegKey,str,0,REG_SZ,(CONST BYTE *)strPath.GetBuffer(0),strPath.GetLength() ) != ERROR_SUCCESS)
					bResult=FALSE;
				else
					bResult=TRUE;
			}else{
				if(	::RegDeleteValue(hRegKey,str) != ERROR_SUCCESS)
					bResult=FALSE;
				else
					bResult=TRUE;
			}
			strPath.ReleaseBuffer();
		}
		return bResult;
	}		

#pragma endregion 文件操作

#pragma region 字符串操作
	//string替换
	void string_replace(string & strBig, const string & strsrc, const string &strdst)
	{
		string::size_type pos=0;
		string::size_type srclen=strsrc.size();
		string::size_type dstlen=strdst.size();
		while( (pos=strBig.find(strsrc, pos)) != string::npos)
		{
			strBig.replace(pos, srclen, strdst);
			pos += dstlen;
		}
	}

	//C++的spilt函数
	void SplitString(const string& s, vector<string>& v, const string& c){
		std::string::size_type pos1, pos2;
		pos2 = s.find(c);
		pos1 = 0;
		while(std::string::npos != pos2){
			v.push_back(s.substr(pos1, pos2-pos1));
			pos1 = pos2 + c.size();
			pos2 = s.find(c, pos1);
		}
		if(pos1 != s.length())
			v.push_back(s.substr(pos1));
	}
	//! 通过文件夹名称获取文件名，不包括后缀
	void getFileName(const string& filepath, string& name,string& lastname){
		vector<string> spilt_path;
		SplitString(filepath, spilt_path, "\\");
		int spiltsize = spilt_path.size();
		string filename = "";
		if (spiltsize != 0){
			filename = spilt_path[spiltsize-1];
			vector<string> spilt_name;
			SplitString(filename, spilt_name, ".");
			int name_size = spilt_name.size();
			if (name_size != 0)
				name = spilt_name[0];
			lastname = spilt_name[name_size-1];
		}
	}
	void getFileName(const string& filepath, string& name){
		vector<string> spilt_path;
		SplitString(filepath, spilt_path, "\\");
		int spiltsize = spilt_path.size();
		string filename = "";
		if (spiltsize != 0){
			filename = spilt_path[spiltsize-1];
			vector<string> spilt_name;
			SplitString(filename, spilt_name, ".");
			int name_size = spilt_name.size();
			if (name_size != 0)
				name = spilt_name[0];
		}
	}

#pragma endregion 字符串操作

#pragma region excel操作
	//////////////////////////////////////////////////////////////////////////////
	//名称：GetExcelDriver
	//功能：获取ODBC中Excel驱动
	//作者：徐景周(jingzhou_xu@163.net)
	//组织：未来工作室(Future Studio)
	//日期：2002.9.1
	/////////////////////////////////////////////////////////////////////////////
	CString GetExcelDriver()
	{
		char szBuf[2001];
		WORD cbBufMax = 2000;
		WORD cbBufOut;
		char *pszBuf = szBuf;
		CString sDriver;

		// 获取已安装驱动的名称(涵数在odbcinst.h里)
		if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
			return "";

		// 检索已安装的驱动是否有Excel...
		do
		{
			if (strstr(pszBuf, "Excel") != 0)
			{
				//发现 !
				sDriver = CString(pszBuf);
				break;
			}
			pszBuf = strchr(pszBuf, '\0') + 1;
		}
		while (pszBuf[1] != '\0');

		return sDriver;
	}

	///////////////////////////////////////////////////////////////////////////////
	//	BOOL MakeSurePathExists( CString &Path,bool FilenameIncluded)
	//	参数：
	//		Path				路径
	//		FilenameIncluded	路径是否包含文件名
	//	返回值:
	//		文件是否存在
	//	说明:
	//		判断Path文件(FilenameIncluded=true)是否存在,存在返回TURE，不存在返回FALSE
	//		自动创建目录
	//
	///////////////////////////////////////////////////////////////////////////////
	BOOL MakeSurePathExists( CString &Path,bool FilenameIncluded)
	{
		int Pos=0;
		while((Pos=Path.Find('\\',Pos+1))!=-1)
			CreateDirectory(Path.Left(Pos),NULL);
		if(!FilenameIncluded)
			CreateDirectory(Path,NULL);
		//	return ((!FilenameIncluded)?!_access(Path,0):
		//	!_access(Path.Left(Path.ReverseFind('\\')),0));
		return !_access(Path,0);
	}

	//获得默认的文件名
	//2018年6月26日 将其升级为可以输入路径参数的
	BOOL GetDefaultXlsFileName(CString& sExcelFile)
	{
		/////默认文件名：yyyymmddhhmmss.xls
		CString timeStr;
		CTime day;
		day=CTime::GetCurrentTime();
		int filenameday,filenamemonth,filenameyear,filehour,filemin,filesec;
		filenameday=day.GetDay();//dd
		filenamemonth=day.GetMonth();//mm月份
		filenameyear=day.GetYear();//yyyy
		filehour=day.GetHour();//hh
		filemin=day.GetMinute();//mm分钟
		filesec=day.GetSecond();//ss
		timeStr.Format("%04d%02d%02d%02d%02d%02d",filenameyear,filenamemonth,filenameday,filehour,filemin,filesec);
		if (sExcelFile == "")
		{
			sExcelFile =  timeStr + ".xls"; //获取随机时间的文件名称
		}else{
			sExcelFile = sExcelFile+".xls";
		}
		
		//打开选择路径窗口
		CString pathName; 
		CString defaultDir = _T("C:\\outtest");
		CString fileName=sExcelFile;
		CString szFilters= _T("xls(*.xls)");
		CFileDialog dlg(FALSE,defaultDir,fileName,OFN_HIDEREADONLY|OFN_READONLY,szFilters,NULL);
		if(dlg.DoModal()==IDOK){
			//获得保存位置
			pathName = dlg.GetPathName();
		}else{
			return FALSE;
		}

		sExcelFile = pathName;
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	//	void GetExcelDriver(CListCtrl* pList, CString strTitle)
	//	参数：
	//		pList		需要导出的List控件指针
	//		strTitle	导出的数据表标题
	//	说明:
	//		导出CListCtrl控件的全部数据到Excel文件。Excel文件名由用户通过“另存为”
	//		对话框输入指定。创建名为strTitle的工作表，将List控件内的所有数据（包括
	//		列名和数据项）以文本的形式保存到Excel工作表中。保持行列关系。
	//	
	//	edit by [r]@dotlive.cnblogs.com
	//  2016年8月12日 修改为可以保存多个表的模式
	///////////////////////////////////////////////////////////////////////////////
	CString ExportListToExcel(CString  sExcelFile,CListCtrl* pList, CString strTitle)
	{
		CString warningStr;
		if (pList->GetItemCount ()>0) {	
			CDatabase database;
			
			
			CString sSql;
			CString tableName = strTitle;

			// 检索是否安装有Excel驱动 "Microsoft Excel Driver (*.xls)" 
			CString sDriver;
			sDriver = GetExcelDriver();
			if (sDriver.IsEmpty())
			{
				// 没有发现Excel驱动
				AfxMessageBox("没有安装Excel!\n请先安装Excel软件才能使用导出功能!");
				return NULL;
			}

			///默认文件名
		/*	CString sExcelFile; 
			if (!GetDefaultXlsFileName(sExcelFile))
				return NULL;*/

			// 创建进行存取的字符串
			sSql.Format("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s",sDriver, sExcelFile, sExcelFile);

			// 创建数据库 (既Excel表格文件)
			if( database.OpenEx(sSql,CDatabase::noOdbcDialog) )
			{
				// 创建表结构
				int i;
				LVCOLUMN columnData;
				CString columnName;
				int columnNum = 0;
				CString strH;
				CString strV;

				sSql = "";
				strH = "";
				columnData.mask = LVCF_TEXT;
				columnData.cchTextMax =100;
				columnData.pszText = columnName.GetBuffer (100);
				for(i=0;pList->GetColumn(i,&columnData);i++)
				{
					if (i!=0)
					{
						sSql = sSql + ", " ;
						strH = strH + ", " ;
					}
					sSql = sSql + " " + columnData.pszText +" TEXT";
					strH = strH + " " + columnData.pszText +" ";
				}
				columnName.ReleaseBuffer ();
				columnNum = i;

				sSql = "CREATE TABLE " + tableName + " ( " + sSql +  " ) ";
				database.ExecuteSQL(sSql);


				// 插入数据项
				int nItemIndex;
				for (nItemIndex=0;nItemIndex<pList->GetItemCount ();nItemIndex++){
					strV = "";
					for(i=0;i<columnNum;i++)
					{
						if (i!=0)
						{
							strV = strV + ", " ;
						}
						strV = strV + " '" + pList->GetItemText(nItemIndex,i) +"' ";
					}

					sSql = "INSERT INTO "+ tableName 
						+" ("+ strH + ")"
						+" VALUES("+ strV + ")";
					database.ExecuteSQL(sSql);
				}

			}      

			// 关闭数据库
			database.Close();
			return sExcelFile;
		}
	}
	//2个datesheet的模式
	CString ExportListToExcel(CListCtrl* pList, CString strTitle,CListCtrl* pList2,CString strTitle2)
	{
		CString warningStr;
		if (pList->GetItemCount ()>0) {	
			CDatabase database;
			CString sDriver;
			CString sExcelFile; 
			CString sSql;
			CString tableName = strTitle;
			CString tableName2 = strTitle2;

			// 检索是否安装有Excel驱动 "Microsoft Excel Driver (*.xls)" 
			sDriver = GetExcelDriver();
			if (sDriver.IsEmpty())
			{
				// 没有发现Excel驱动
				AfxMessageBox("没有安装Excel!\n请先安装Excel软件才能使用导出功能!");
				return NULL;
			}

			///默认文件名
			if (!GetDefaultXlsFileName(sExcelFile))
				return NULL;

			// 创建进行存取的字符串
			sSql.Format("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s",sDriver, sExcelFile, sExcelFile);

			// 创建数据库 (既Excel表格文件)
			if( database.OpenEx(sSql,CDatabase::noOdbcDialog) )
			{
				// 创建表结构1
				int i;
				LVCOLUMN columnData;
				LVCOLUMN columnData2;
				CString columnName;
				CString columnName2;
				int columnNum = 0;
				CString strH;
				CString strV;

				sSql = "";
				strH = "";
				columnData.mask = LVCF_TEXT;
				columnData.cchTextMax =100;
				columnData.pszText = columnName.GetBuffer (100);
				columnData2.mask = LVCF_TEXT;
				columnData2.cchTextMax =100;
				columnData2.pszText = columnName2.GetBuffer (100);
				// 插入数据项1
				for(i=0;pList->GetColumn(i,&columnData);i++)
				{
					if (i!=0)
					{
						sSql = sSql + ", " ;
						strH = strH + ", " ;
					}
					sSql = sSql + " " + columnData.pszText +" TEXT";
					strH = strH + " " + columnData.pszText +" ";
				}
				columnName.ReleaseBuffer ();
				columnNum = i;

				sSql = "CREATE TABLE " + tableName + " ( " + sSql +  " ) ";
				database.ExecuteSQL(sSql);

				
				int nItemIndex;
				for (nItemIndex=0;nItemIndex<pList->GetItemCount();nItemIndex++){
					strV = "";
					for(i=0;i<columnNum;i++)
					{
						if (i!=0)
						{
							strV = strV + ", " ;
						}
						strV = strV + " '" + pList->GetItemText(nItemIndex,i) +"' ";
					}

					sSql = "INSERT INTO "+ tableName 
						+" ("+ strH + ")"
						+" VALUES("+ strV + ")";
					database.ExecuteSQL(sSql);
				}
				//插入数据项2
				sSql = "";
				strH="";
				int columnNum2 = 0;
			
				for(int i=0;pList2->GetColumn(i,&columnData2);i++)
				{
					if (i!=0)
					{
						sSql = sSql + ", " ;
						strH = strH + ", " ;
					}
					sSql = sSql + " " + columnData2.pszText +" TEXT";
					strH = strH + " " + columnData2.pszText +" ";
				}
				columnName2.ReleaseBuffer ();
				columnNum2 = i;

				sSql = "CREATE TABLE " + tableName2 + " ( " + sSql +  " ) ";
				database.ExecuteSQL(sSql);
			 
				for (nItemIndex=0;nItemIndex<pList2->GetItemCount ();nItemIndex++){
					strV = "";
					for(i=0;i<columnNum2;i++)
					{
						if (i!=0)
						{
							strV = strV + ", " ;
						}
						strV = strV + " '" + pList2->GetItemText(nItemIndex,i) +"' ";
					}

					sSql = "INSERT INTO "+ tableName2 
						+" ("+ strH + ")"
						+" VALUES("+ strV + ")";
					database.ExecuteSQL(sSql);
				}
			}      
			// 关闭数据库
			database.Close();

			warningStr.Format("导出文件保存于%s!",sExcelFile);
			AfxMessageBox(warningStr);
			return sExcelFile;
		}
	}
#pragma endregion excel操作

}