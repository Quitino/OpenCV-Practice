#  OpenCV图像分割

## 1.K-Means

- [ref](https://blog.csdn.net/huangfei711/article/details/78480078)
- 数据聚类

1. 无监督学习方法
2. 分类问题，输入分类数目，初始化中心位置
3. 硬分类方法，以距离度量
4. 迭代分类为聚类  

- 图像分割  
- code  


```
	TermCriteria criteria = TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1);
	kmeans(points, clusterCount, labels, criteria, 3, KMEANS_PP_CENTERS, centers);
```

- [fullcode](/code/cpp/case01-KMeans.cpp)

## 2.GMM
高斯混合模型（Gaussian Mixed Model）指的是多个高斯分布函数的线性组合，理论上GMM可以拟合出任意类型的分布，通常用于解决同一集合下的数据包含多个不同的分布的情况。

- [ref0](https://www.cnblogs.com/huangyc/p/10125117.html)
- [ref1](https://www.jianshu.com/p/47c3d7ea3433)
- GMM方法

1.高斯混合模型 (GMM)  
2. 高斯分布与概率密度分布 - PDF  
3. 初始化  
4.最大似然法  

- 特点

1.跟K-Means相比较，属于软分类  
2.实现方法-期望最大化(E-M)，EM算法是一种迭代算法.  
3.停止条件-收敛  
4.很耗时  

- code 

```
	//EM 训练
	Ptr<EM>  em_model = EM::create();
	em_model->setClustersNumber(clusterCount);
	em_model->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
	em_model->setTermCriteria(TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 100, 0.1));
	em_model->trainEM(samplePoint, noArray(), labels, noArray());
```

- [fullcode](/code/cpp/case02-GMM.cpp)

## 3.分水岭方法

区别于聚类算法与GMM算法，分水岭不需要指定分割成多少块。

- [ref](https://www.cnblogs.com/mikewolf2002/p/3304118.html)
- 原理  

1.基于浸泡理论的分水岭分割方法   
2.基于连通图的方法  
3.基于距离变换的方法(opencv实现的方法)  

- 步骤

1. 图像形态学操作  
2. 分水岭分割方法原理



输入图像->灰度-> 二值->距离变换->寻找种子->生成Marker->分水岭变换->输出图像->End 

- [均值漂移算法.](https://cloud.tencent.com/developer/article/1470668)
- 应用

分割粘连对象，实现形态学操作与对象计数. 图像分割

- code 

```
pyrMeanShiftFiltering(src, shifted, 21, 51);

...

distanceTransform(binary, dist, DistanceTypes::DIST_L2, 3, CV_32F);

...

	//image:三通道彩色图像
	//markers : 记号点（种子点），每一个记号都需要有不同的编号
	watershed(src, marks);
```

- [fullcode](/code/cpp/case03-Watershed.cpp)

## 4.GrabCut

可以用户交互的抠图方法，目标明确，认为的选定ROI区域。

- [ref](https://www.cnblogs.com/mikewolf2002/p/3341418.html)
- [ref](http://www.cad.zju.edu.cn/home/gfzhang/course/computational-photography/proj1-grabcut/grabcut.html)

- 流程

输入图像-> 矩形输入-> 初始分类-> GMM描述-> GMM训练分类-> Graph Cut分类-> 最终收敛分类-> 停止


- code

```
setMouseCallback；  
onMouse(int event, int x, int y, int flags, void* 
param)  
```
- [fullcode](/code/cpp/case04-GrabCut.cpp)

##  5.demo1-背景替换


-  关键点  
1.K-Means   
2.背景融合 – 高斯模糊  
3.遮罩层生成  

-  步骤


开始-> 数据组装-> K-Means分割-> 背景去除-> 遮罩生成-> 遮罩模糊-> 通道混合输出-> End 


- [fullcode](/code/cpp/case05-BgReplace.cpp)

##  6.demo2-视频背景替换

- 算法选择   
1.GMM或者Kmeans - ?? Bad idea, very slow!!!  
2.基于色彩的处理方法  
3.RGB与HSV色彩空间   


```
cvtColor(frame, hsv, COLOR_BGR2HSV);
inRange(hsv, Scalar(35, 43, 46), Scalar(155, 255, 255), mask);//绿幕颜色范围值,绿色使用255白色替换
//形态学操作
Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
morphologyEx(mask, mask, MORPH_CLOSE, kernel);
erode(mask,mask,kernel);
GaussianBlur(mask, mask, Size(3, 3), 0, 0);
```

- [fullcode](/code/cpp/case06-videoBgReplace.cpp)


