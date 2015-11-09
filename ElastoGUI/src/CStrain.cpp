#include "stdafx.h"
#include "CStrain.h"
#include "CDisplacement.h"
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include <iostream>
#include <vector>

using namespace std;


extern ConfigParam g_paramConfig;
void  SaveDataFile(const char *filename, CvMat *pmat);


int ChangeImgColor(IplImage *scr)
{
	CvScalar avgChannels = cvAvg(scr);
	double avgB = avgChannels.val[0];//获取第一通道平均值
	double avgG = avgChannels.val[1];//获取第二通道平均值
	double avgR = avgChannels.val[2];//获取第三通道平均值

	CvScalar idensity;
	int i = 0, j = 0;
	for (; i < scr->height; i++)
	{
		for (j = 0; j < scr->width; j++)
		{
			idensity = cvGet2D(scr, i, j);
			idensity.val[0] = idensity.val[0] - avgB + 19;//修改色素值
			idensity.val[1] = idensity.val[1] - avgG + 79;
			idensity.val[2] = idensity.val[2] - avgR + 168;
			cvSet2D(scr, i, j, idensity);
		}
	}

	return 0;
}

CStrain::CStrain()
{

}

void CStrain::Do()
{
	//strainAlgorithm(); 
}



//////////////////////////////////////////////////////////////////////////
// 拉东变换
// pmatDisplacement,   rows: disp;  cols: time-extent( lines)
//     列,表示一条线, 也就是时间 轴
//     行,表示应变的值
//////////////////////////////////////////////////////////////////////////
void  CStrain::RadonSum(const CvMat *pmatDisplacement, CvMat **ppmatRodan)
{
	int xstart = 0;
	int xend = pmatDisplacement->rows;
	int t = pmatDisplacement->cols;// time extent

	CvMat *pmatRodan = cvCreateMat(t - 1, t, pmatDisplacement->type);
	cvZero(pmatRodan);

	int tstart = 0;
	int tend = 0;
	int dx = 0;
	float dt = 0.0f;
	float c = 0.0f;

	for (tstart = 0; tstart < t - 1; tstart ++)
	{
		for (tend = tstart + 1; tend < t; tend ++)
		{
			c = (float)(xend - xstart) / (tend - tstart);
			for (dx = xstart; dx < xend; dx ++)
			{
				dt = tstart + (dx - xstart) / c;
				CV_MAT_ELEM(*pmatRodan, float, tstart, tend) = CV_MAT_ELEM(*pmatRodan, float, tstart, tend)
					+ CV_MAT_ELEM(*pmatDisplacement, float, dx, (int)dt);
			}
		}
	}

	*ppmatRodan = pmatRodan;
}

static void PopFirstVector(std::vector<cv::Point2f> &vec)
{
	std::vector<cv::Point2f> swap_vec(vec);
	std::vector<cv::Point2f>::size_type size = swap_vec.size();
	vec.clear();
	if (size > 1)
	{
		int n = size - 1;
		int i;
		for (i = 0; i < n; i++)
		{
			vec.push_back(swap_vec[i + 1]);
		}
	}
}

static void PushBackVector(std::vector<CvPoint2D32f> & vec, CvPoint2D32f &pt)
{
	vec.push_back(pt);
}

static void PopFirstVector(CvPoint2D32f *pVec, int size)
{
	CvPoint2D32f *pvecSwap = new CvPoint2D32f[size];

	ZeroMemory(pvecSwap, sizeof(CvPoint2D32f) * size);

	memcpy(pvecSwap, pVec + 1, sizeof(CvPoint2D32f) * (size - 1));

	memcpy(pVec, pvecSwap, sizeof(CvPoint2D32f) * size);

	delete [] pvecSwap;
}

void  CStrain::ComputeStrainValueAndImage(int count, CvMat *strainMat, IplImage *strImage)
{
	float *tmp;  //临时变量，指向应变图像某一点
	float  coeff_a1; //存储直线斜率（一次项）
	int    deltadepth = 5;   //单位深度
	float  result[4] = {0.0, 0.0, 0.0, 0.0}; //存储直线拟合结果
	int    i, j;
	CvPoint2D32f pt;

#if 0
	// 原来的设计，使用CvSeq, cvFitLine.

	CvMemStorage *storage = cvCreateMemStorage(0);
	for(i = 0; i < strImage->width; i++)// srtImage的列
	{
		CvSeq* point_seq = cvCreateSeq(CV_32FC2, sizeof(CvSeq), sizeof(CvPoint2D32f), storage);
		for(j = 0; j < count - 1; ++j)	//先压入count - 1个点
		{
			pt = cvPoint2D32f(j * deltadepth, CV_MAT_ELEM(*outDataMat, float, i, j));
			cvSeqPush(point_seq, &pt);
		}

		for(j = 0; j < strImage->height; j++)// strImage的行
		{
			int k = j + count -1; // 前面已经压入了count - 点
			tmp = static_cast<float*>(static_cast<void*>(strImage->imageData + j * strImage->widthStep + sizeof(float) * i));  //取应变图像对应位置
			pt = cvPoint2D32f(k * deltadepth, CV_MAT_ELEM(*outDataMat, float, i, k));
			cvSeqPush(point_seq, &pt);  //压入最后一个点
			cvFitLine(point_seq, CV_DIST_L2, 0, 0.01, 0.01, result); //最小二乘拟合
			coeff_a1 = result[1] / result[0];   //算出直线斜率，即为中心点应变
			CV_MAT_ELEM_PTR(*strainMat, float, i, j) = coeff_a1;  
			*tmp = 100 * coeff_a1;

			cvSeqPopFront(point_seq);
		}
		cvClearSeq(point_seq);
	}
	cvReleaseMemStorage(&storage);    //read violation

#endif 

#if 0

	/* 用C++接口重新实现	*/
	{
		std::vector<cv::Point2f> points_vec;
		
		for(i = 0; i < strImage->width; i++)// srtImage的列
		{
			for (j = 0; j < count - 1; j++)	//先压入points - 1个点
			{
				pt = cvPoint2D32f(j * deltadepth, CV_MAT_ELEM(*outDataMat, float, i, j));
				points_vec.push_back(pt);
			}

			for(j = 0; j < strImage->height; ++j)// strImage的行
			{
				int k = j + count - 1;
				tmp = static_cast<float*>(static_cast<void*>(strImage->imageData + j * strImage->widthStep + sizeof(float) * i));  //取应变图像对应位置
				pt = cvPoint2D32f(k * deltadepth, CV_MAT_ELEM(*outDataMat, float, i, k));
				points_vec.push_back(pt);  //压入最后一个点
				cv::Vec4f line;
				cv::fitLine(cv::Mat(points_vec), line, CV_DIST_L2, 0, 0.01, 0.01);//最小二乘拟合
				coeff_a1 = line[1] / line[0];   //算出直线斜率，即为中心点应变
				CV_MAT_ELEM(*strainMat, float, i, j) = coeff_a1;  
				*tmp = 100 * coeff_a1;
				PopFirstVector(points_vec);
			}
			points_vec.clear();
		}
	}

#endif

#if 1
	// 用数组代替CvSeq
	{
		CvPoint2D32f *points = new  CvPoint2D32f[count];
		CvMat ptMat = cvMat(1, count, CV_32FC2, points);
		for(i = 0; i < strImage->width; i++)// srtImage的列
		{
			for (j = 0; j < count - 1; j++)	//先压入points - 1个点
			{
				pt = cvPoint2D32f(j * deltadepth, CV_MAT_ELEM(*outDataMat, float, i, j));
				points[j] = pt;
			}

			for(j = 0; j < strImage->height; j++)// strImage的行
			{
				int k = j + count - 1;
				tmp = static_cast<float*>(static_cast<void*>(strImage->imageData + j * strImage->widthStep + sizeof(float) * i));  //取应变图像对应位置
				pt = cvPoint2D32f(k * deltadepth, CV_MAT_ELEM(*outDataMat, float, i, k));
				points[count- 1] = pt;  //压入最后一个点

				cvFitLine(&ptMat, CV_DIST_L2, 0, 0.01, 0.01, result);//最小二乘拟合
				coeff_a1 = result[1] / result[0];   //算出直线斜率，即为中心点应变
				//cv::Vec4f line;
				//cv::fitLine(cv::Mat(&ptMat), line, CV_DIST_L2, 0, 0.01, 0.01);//最小二乘拟合
				//coeff_a1 = line[1] / line[0];   //算出直线斜率，即为中心点应变

				CV_MAT_ELEM(*strainMat, float, i, j) = coeff_a1;  
				*tmp = 100 * coeff_a1;
				PopFirstVector(points, count);
			}
		}
		delete [] points;
	}
#endif

}

  
//////////////////////////////////////////////////////////////////////////
// 计算弹性模量，使用拉东变换（一次）
// 输入数据, outDataMat, 这里表示 位移数据;格式说明:  行,表示一条线; 列,表示位移值
//
//////////////////////////////////////////////////////////////////////////
void  CStrain::CalcStrain2(const EInput &input, EOutput &output)
{
	string filename = input.filepath_s;

	//最小二乘法求应变；用于拟合的点，其横坐标为深度，纵坐标为位移
	const int points = g_paramConfig.fitline_pts;	//用于拟合的点数

	//int image_width = outDataMat->cols - 300 / g_paramConfig.step;
	int image_width = outDataMat->cols;
	IplImage *strImage  = cvCreateImage(cvSize(outDataMat->rows, image_width - points + 1), IPL_DEPTH_32F, 1);//用于显示应变, 相对于outDataMat做了转置,行列颠倒.
	CvMat    *strainMat = cvCreateMat(strImage->width, strImage->height, CV_32FC1);  //应变矩阵，它相对于strImage做了转置，和outDataMat相同

	ComputeStrainValueAndImage(points, strainMat, strImage);
	SaveDataFile("strain.dat", strainMat);//用于保存应变数据
	
	//拉东变换&求剪切波&弹性模量
	{
		//int    dep_start = 50;
		//int    dep_end   = 249;

		//int    dep_start = 00 / g_paramConfig.step;
		//int    dep_size  = 1600 / g_paramConfig.step;
		int    dep_start = (g_paramConfig.sb_x < 0)  ? 0 : g_paramConfig.sb_x;
		int    dep_size  =  (g_paramConfig.sb_w < 0) ? strainMat->width : g_paramConfig.sb_w;
		
		int    dep_end   = dep_start + dep_size - 1;
		int    win_size  = g_paramConfig.windowHW;

		double overlap   = (g_paramConfig.windowHW - g_paramConfig.step) / (float)g_paramConfig.windowHW;  // 重合率，90%
		double sound_velocity = g_paramConfig.acousVel; // 声波速度
		double sample_frq = g_paramConfig.sampleFreqs;
		double prf = g_paramConfig.prf;

		CvMat *pmatStrainTran = cvCreateMat(strainMat->cols, strainMat->rows, strainMat->type);// 把strainMat转置
		cvTranspose(strainMat, pmatStrainTran);

		int    t_start = (g_paramConfig.sb_y < 0) ? 0 : g_paramConfig.sb_y;
		int    t_size  = (g_paramConfig.sb_h < 0) ? strainMat->rows : g_paramConfig.sb_h;
		int    t_end   = t_start + t_size - 1;

		CvMat *pmatSub = cvCreateMatHeader(dep_size, t_size, pmatStrainTran->type);

		cvGetSubRect(pmatStrainTran, pmatSub, cvRect(t_start, dep_start, t_size, dep_size));
		
		CvMat *pmatRadon = 0;
		// 插值处理后在做拉东变换
		float  intpl_multiple = 1.0f;//20.0, 
		CvMat *pmatMultiple = cvCreateMat(pmatSub->rows, (int)(pmatSub->cols * intpl_multiple), pmatSub->type);
		cvResize(pmatSub, pmatMultiple);
		CStrain::RadonSum(pmatMultiple, &pmatRadon);

		double  min_val;
		double  max_val;
		CvPoint min_loc;
		CvPoint max_loc;
		cvMinMaxLoc(pmatRadon, &min_val, &max_val, &min_loc, &max_loc);
		double v = ((dep_end - dep_start) * win_size * (1 - overlap) * sound_velocity / sample_frq / 2) 
			/ ((max_loc.x - max_loc.y) / intpl_multiple / prf);
		double e = v * v * 3;
		output.v = (float)v;
		output.e = (float)e;

		cvReleaseMat(&pmatRadon);
		cvReleaseMat(&pmatMultiple);
		cvReleaseMatHeader(&pmatSub);
		cvReleaseMat(&pmatStrainTran);

		IplImage *pimgStrain = cvCreateImage(cvGetSize(strImage), strImage->depth, 3);
		cvCvtColor(strImage, pimgStrain, CV_GRAY2BGR);

		ChangeImgColor(pimgStrain);

		cvLine(pimgStrain, cvPoint((int)(max_loc.y / intpl_multiple), dep_start), cvPoint((int)(max_loc.x / intpl_multiple), dep_end), CV_RGB(255,0,0), 2, CV_AA, 0);   //画线
		cvSaveImage(filename.c_str(), pimgStrain);
		cvReleaseImage(&pimgStrain);
	}

	cvReleaseImage(&strImage);
	cvReleaseMat(&strainMat);
}


//////////////////////////////////////////////////////////////////////////
// 为了降低处理时间。特别进行设计：
// 两次拉东变换。
// 第一次确定了扫描线的范围，第二次插值20倍再做拉东变换。
//////////////////////////////////////////////////////////////////////////
void  CStrain::CalcStrain(const EInput &input, EOutput &output)
{
	string filename = input.filepath_s;

	//最小二乘法求应变；用于拟合的点，其横坐标为深度，纵坐标为位移
	int deltadepth = 5;   //单位深度
	float result1[4] = {0.0, 0.0, 0.0, 0.0}; //存储直线拟合结果
	float coeff_a1; //存储直线斜率（一次项）
	float *tmp;  //临时变量，指向应变图像某一点
	int points = g_paramConfig.fitline_pts;	//用于拟合的点数
	float minstrain = 0;
	float maxstrain = 0;

	int image_width = outDataMat->cols - 300 / g_paramConfig.step;
	//int image_width = outDataMat->cols;

	IplImage *strImage  = cvCreateImage(cvSize(outDataMat->rows, image_width - points + 1), IPL_DEPTH_32F, 1);//用于显示应变, 相对于outDataMat做了转置,行列颠倒.
	CvMat    *strainMat = cvCreateMat(strImage->width, strImage->height, CV_32FC1);  //应变矩阵，它相对于strImage做了转置，和outDataMat相同

	CvMemStorage* storage = cvCreateMemStorage(0);
	for(int i = 0; i < strImage->width; ++i)// srtImage的列
	{
		CvSeq* point_seq = cvCreateSeq(CV_32FC2, sizeof(CvSeq), sizeof(CvPoint2D32f), storage);
		for(int j = 0; j < points - 1; ++j)	//先压入points - 1个点
		{
			cvSeqPush(point_seq, &cvPoint2D32f(j * deltadepth, *(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, i,j))))));
		}

		for(int j = points - 1; j < image_width; ++j)// strImage的行
		{
			int k = j - points + 1;
			tmp = static_cast<float*>(static_cast<void*>(strImage->imageData + (j - points + 1) * strImage->widthStep + sizeof(float) * i));  //取应变图像对应位置
			cvSeqPush(point_seq, &cvPoint2D32f((j)*deltadepth, *(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, i, j))))));  //压入最后一个点
			cvFitLine(point_seq, CV_DIST_L2, 0, 0.01, 0.01, result1); //最小二乘拟合
			coeff_a1 = result1[1] / result1[0];   //算出直线斜率，即为中心点应变
			*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*strainMat, i, k)))) = coeff_a1;  
			*tmp = 100 * coeff_a1;
			cvSeqPopFront(point_seq);
		}
		cvClearSeq(point_seq);
	}
	cvReleaseMemStorage(&storage);    //read violation
	SaveDataFile("strain.dat", strainMat);
	//cvSaveImage("strain_gray.bmp", strImage);

	{
		int    dep_start = 500 / g_paramConfig.step; // 150-3.5cm, 70-2.5cm, 110-3cm
		//int    dep_start = 800 / g_paramConfig.step; // 150-3.5cm, 70-2.5cm, 110-3cm
		int    dep_size  = 1600 / g_paramConfig.step ;
		int    dep_end   = dep_start + dep_size - 1;
		int    win_size  = 100;
		double overlap   = (g_paramConfig.windowHW - g_paramConfig.step) / (float)g_paramConfig.windowHW;  // 重合率，90%
		double sound_velocity = 1500.0f; // 声波速度
		double sample_frq = 60e6;
		double prf = 1/300e-6;

		// 为了提高检测的准确度，人为把测量范围时间轴限制在150~240线之间。
		// 150, 90
		// 200, 70
		// 180, 70
		int    t_base  = 0;
		int    t_size  = strainMat->rows;
		int    t_start = t_base;
		int    t_end   = t_base + t_size - 1;

		CvMat *pmatStrainTran = cvCreateMat(strainMat->cols, strainMat->rows, strainMat->type);// 把strainMat转置
		cvTranspose(strainMat, pmatStrainTran);

		CvMat *pmatSub = cvCreateMatHeader(dep_size, t_size, pmatStrainTran->type);

		cvGetSubRect(pmatStrainTran, pmatSub, cvRect(t_start, dep_start, t_size, dep_size));

		//第一次拉东变换，目的是减少处理扫描线的数量
		CvMat *pmatRadon = 0;
		CStrain::RadonSum(pmatSub, &pmatRadon);

		double  min_val;
		double  max_val;
		CvPoint min_loc;
		CvPoint max_loc;
		cvMinMaxLoc(pmatRadon, &min_val, &max_val, &min_loc, &max_loc);

		// 前后各放大一些扫描线，注意不要超过原始的范围
		int  t_shift = 2;

		// 数据必须从pmatStrainTran取得，所以t_start,t_end的坐标值必须从pmatSub（它是pmatStrainTran中的一个子集）映射到pmatStrainTran坐标中
		t_start = max_loc.y - t_shift + t_base;// 是前面限定范围时间轴坐标的起始值
		t_end   = max_loc.x + t_shift + t_base;

		t_start = t_start > -1 ? t_start : 0;
		t_end = (t_end < pmatStrainTran->width) ? t_end : pmatStrainTran->width - 1;
		t_size  = t_end - t_start + 1;

		cvReleaseMatHeader(&pmatSub);
		cvReleaseMat(&pmatRadon);

		float  intpl_multiple = 20.0f;//插值的倍数
		{
			//第二次拉东变换
			pmatSub = cvCreateMatHeader(dep_size, t_size, pmatStrainTran->type);
			cvGetSubRect(pmatStrainTran, pmatSub, cvRect(t_start, dep_start, pmatSub->width, pmatSub->height));

			CvMat *pmatMultiple = cvCreateMat(pmatSub->rows, (int)(pmatSub->cols * intpl_multiple), pmatSub->type);
		    cvResize(pmatSub, pmatMultiple);
			//cvResize(pmatSub, pmatMultiple, CV_INTER_CUBIC);

			CStrain::RadonSum(pmatMultiple, &pmatRadon);
		
			ASSERT(max_loc.x != max_loc.y);
			cvMinMaxLoc(pmatRadon, &min_val, &max_val, &min_loc, &max_loc);
			cvReleaseMat(&pmatMultiple);
		}
		double v = ((dep_end - dep_start) * win_size * (1 - overlap) * sound_velocity / sample_frq / 2) / ((max_loc.x - max_loc.y) / intpl_multiple / prf);
		double e = v * v * 3;
		output.v = (float)v;
		output.e = (float)e;

		cvReleaseMatHeader(&pmatSub);
		cvReleaseMat(&pmatStrainTran);
		cvReleaseMat(&pmatRadon);

		IplImage *pimgStrain = cvCreateImage(cvGetSize(strImage), strImage->depth, 3);
		cvCvtColor(strImage, pimgStrain, CV_GRAY2BGR);

		ChangeImgColor(pimgStrain);

		cvLine(pimgStrain, cvPoint((int)(max_loc.y / intpl_multiple + t_start), dep_start), cvPoint((int)(max_loc.x / intpl_multiple + t_start), dep_end), CV_RGB(255,0,0), 2, CV_AA, 0);   //画线
		
#if 0
		IplImage *pimgResize = cvCreateImage(cvSize(strImage->width, 355), strImage->depth, 3);
		cvResize(pimgStrain, pimgResize);
		cvSaveImage(filename.c_str(), pimgResize);
		cvReleaseImage(&pimgResize);
		cvReleaseImage(&pimgStrain);
#else

		cvSaveImage(filename.c_str(), pimgStrain);
		cvReleaseImage(&pimgStrain);

#endif
	}

	cvReleaseImage(&strImage);
	cvReleaseMat(&strainMat);
}


