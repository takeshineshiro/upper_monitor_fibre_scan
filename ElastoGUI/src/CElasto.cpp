#include "stdafx.h"
#include "CDisplacement.h"
#include "CData.h"
#include "CStrain.h"
#include "CFilter.h"
#include "CElasto.h"
#include "opencv/highgui.h"
#include <iostream>
#include <time.h>
#include <stdio.h>
#include "Log.h"
#include "TestTime.h"

// 未接触物体（人体或者体模）时RF数据的阀值（最大值），经验所得；注意：滤波以后
#define  RF_NO_BODY_THRESHOLD  100 

CData* test;
CDataProcess* datatest;
CDisplacement* distest;
CStrain* strtest;
CFilter* bpfilt;
CFilter* lpfilt;

EPHandler  g_lpEpHander = NULL;
void *g_lpParam = NULL;
ConfigParam  g_paramConfig;

void  SaveDataFile(const char *filename, CvMat *pmat);

void  MakeBmpAndShow(const char *filename, const CvMat *pmat);

// 读取二进制文件
 template<class T> void ReadBinFile(const char *filepath, T *pBuffer, int nElems)
{
	FILE *fid;
	int  nCount = 0;
	int  nBytes = 0;
	T *p = pBuffer;
	if (fopen_s(&fid, filepath, "rb") == 0)
	{
		T t;
		while (!feof(fid))
		{
			nBytes = fread(&t, sizeof(T), 1, fid);
			if (nBytes == 0)   break;
			*p ++ = t;
			nCount ++;
		}

		fclose(fid);
	}
}


 bool ReadParams(const char *ini_file, ConfigParam &param)
 {
	 //FILE *file = fopen(ini_file, "r");

	 if (ini_file)
	 {
		 char text[100];
		 param.acousVel = (float) GetPrivateProfileInt("Parameters", "acousVel", 1000, ini_file);
		 param.sampleFreqs = GetPrivateProfileInt("Parameters", "sampleFreqs", 3000000, ini_file);

		 param.windowHW = GetPrivateProfileInt("Parameters", "windowHW", 10, ini_file);
		 param.step     = GetPrivateProfileInt("Parameters", "step", 3, ini_file);
		 param.maxLag         = GetPrivateProfileInt("Parameters", "maxLag", 30, ini_file);

		 GetPrivateProfileString("Parameters", "threshold", "", text, 99, ini_file);
		 param.threshold = (float) strtod(text, NULL);

		 GetPrivateProfileString("Parameters", "bpfilt_file", "", text, 99, ini_file);
		 param.bpfilt_file = text;

		 GetPrivateProfileString("Parameters", "lpfilt_file", "", text, 99, ini_file);
		 param.lpfilt_file = text;

		 param.fitline_pts = GetPrivateProfileInt("Parameters", "fitline_pts", 2, ini_file);

		 param.box_x = GetPrivateProfileInt("Parameters", "box_x", 0, ini_file);
		 param.box_y = GetPrivateProfileInt("Parameters", "box_y", 0, ini_file);
		 param.box_w = GetPrivateProfileInt("Parameters", "box_w", 4096, ini_file);
		 param.box_h = GetPrivateProfileInt("Parameters", "box_h", 300, ini_file);
		 param.sb_x  = GetPrivateProfileInt("Parameters", "sb_x",  -1, ini_file);
		 param.sb_y  = GetPrivateProfileInt("Parameters", "sb_y",  -1, ini_file);
		 param.sb_w  = GetPrivateProfileInt("Parameters", "sb_w",  -1, ini_file);
		 param.sb_h  = GetPrivateProfileInt("Parameters", "sb_h",  -1, ini_file);

		 param.times_Login = GetPrivateProfileInt("Times", "times_Login", 0, ini_file);
		 param.times_StartElasto = GetPrivateProfileInt("Times", "times_StartElasto", 0, ini_file);

		 return true;
	 }
	 else
	 {
		 return false;
	 }
 }

// 读取应变数据,调用拉东变换计算弹性模量
//  strain_NG5.dat,   v=3.31,  e =33.008
void TestCalcStrain()
{
	double *pDatas = new double[299 * 355];
	
	ReadBinFile("strain_NG5.dat", pDatas, 299 * 355);

	CvMat *strainMat = cvCreateMat(355, 299, CV_32FC1);

	for (int i = 0; i < 299; i++) // cols
	{
		for (int j = 0; j < 355; j++) // rows
		{
			float f =  (float)(* (pDatas + i * 355 + j));
			CV_MAT_ELEM(*strainMat, float, j, i) = f;
		}
	}
	delete [] pDatas;

	{
		int    dep_start = 51;
		int    dep_end   = 250;
		int    win_size  = 100;
		double overlap   = 0.9f;  // 重合率，90%
		double sound_velocity = 1500.0f; // 声波速度
		double sample_frq = 60e6;
		double prf = 1/300e-6;

		//CvMat *pmatSub = cvCreateMat(dep_end - dep_start + 1, strainMat->cols, strainMat->type);
		CvMat *pmatSub = cvCreateMatHeader(dep_end - dep_start + 1, strainMat->cols, strainMat->type);

		cvGetSubRect(strainMat, pmatSub, cvRect(0, 50, 299, 200));
		CvMat *pmatRadon = 0;
		CStrain::RadonSum(pmatSub, &pmatRadon);

		double  min_val;
		double  max_val;
		CvPoint min_loc;
		CvPoint max_loc;
		cvMinMaxLoc(pmatRadon, &min_val, &max_val, &min_loc, &max_loc);
		double v = ((dep_end - dep_start) * win_size * (1 - overlap) * sound_velocity / sample_frq / 2) 
			/ ((max_loc.x - max_loc.y) / prf);
		double e = v * v * 3;

		cvReleaseMat(&pmatRadon);
		cvReleaseMat(&pmatSub);

		TRACE("V=%f; E=%f;\n", v, e);
	}

	cvReleaseMat(&strainMat);
}


ELASTO_DLL_API int ElastoInit(const char *configfile)
{	
	g_lpEpHander = NULL;
	g_lpParam    = NULL;
	//TestCalcStrain();
	ReadParams(configfile, g_paramConfig);
	g_paramConfig.prf = 1 / 300e-6;
	return 0;
}


ELASTO_DLL_API void ElastoRelease()
{	

}


ELASTO_DLL_API EPHandler ElastoRegisterHandler(EPHandler hander, void *lpParam)
{
	EPHandler old_hander = g_lpEpHander;
	g_lpEpHander = hander;
	g_lpParam    = lpParam;
	return old_hander;
}


ELASTO_DLL_API  void  ElastoGetConfig(ConfigParam &param)
{
    param = g_paramConfig;
}


//////////////////////////////////////////////////////////////////////////
// 内部对psrc进行了转置操作
//////////////////////////////////////////////////////////////////////////
void MakeImage(const CvMat *psrc, const char *filename)
{
	CvMat *pmat = cvCreateMat(psrc->cols, psrc->rows, psrc->type);

	cvTranspose(psrc, pmat);
	IplImage *pimage = cvCreateImage(cvGetSize(pmat), IPL_DEPTH_32F, 3);
	cvCvtColor(pmat, pimage, CV_GRAY2BGR);

	cvSaveImage(filename, pimage);

	cvReleaseImage(&pimage);
	cvReleaseMat(&pmat);
}

ELASTO_DLL_API int ElastoProcess(const EInput &in, EOutput &out)
{
	int  error = EE_OK;
	std::string filename;
	filename = in.filepath_s;
	test = new CData(in.rows, in.cols);
	test->readData(in.pDatas);

	//distest = new CDisplacement(test->getData(), g_paramConfig.windowHW, g_paramConfig.step);
	distest = new CDisplacement(test->getSubData(g_paramConfig.box_x, g_paramConfig.box_y, g_paramConfig.box_w, g_paramConfig.box_h), g_paramConfig.windowHW, g_paramConfig.step);

	datatest = new CDataProcess();
	strtest  = new CStrain();
	bpfilt   = new CFilter(g_paramConfig.bpfilt_file.c_str());
	lpfilt   = new CFilter(g_paramConfig.lpfilt_file.c_str());

	//CTestTime ttime;
	//CString info;
    //long timeout;

	// 如果RF数据全部小于某个阀值，说明探头未接触人体；由此确定无须后续算法处理，直接返回无效值
	{
		double min_v;
		double max_v;
		cvMinMaxLoc(bpfilt->outDataMat, &min_v, &max_v);

		TRACE("RF-min=%f, max=%f\n", min_v, max_v);
		if (max_v < RF_NO_BODY_THRESHOLD)
		{
			error = EE_NO_BODY;
			out.e = -1.0f;
			out.v = -1.0f;
			goto exit;
		}
	}
	
	//ttime.run();
	datatest->doProcess(bpfilt);   //带通滤波

	SaveDataFile("afterBandpassData.dat", bpfilt->outDataMat);//test 保存 带通滤波 后的结果
	//timeout = ttime.stop();
	//info.Format(_T("bandpass-filter-timeout=%dms\n"), timeout);
	//CLog::Instance()->Write(info.GetString(), info.GetLength());

	// 通知观察者
	if (g_lpEpHander)
	{
		(*g_lpEpHander)(EP_POST_FILTER, bpfilt->outDataMat, g_lpParam);
	}
	//MakeImage(bpfilt->outDataMat, in.filepath_d);
	SaveDataFile("bpfilt.dat", bpfilt->outDataMat);

	//ttime.run();
	datatest->doProcess(distest);  //求位移

	//timeout = ttime.stop();
	//info.Format(_T("displace-timeout=%dms\n"), timeout);
	//CLog::Instance()->Write(info.GetString(), info.GetLength());

	//ttime.run();
	datatest->doProcess(lpfilt);  //低通滤波
	//timeout = ttime.stop();
	//info.Format("lowpass-timeout=%dms\n", timeout);
	//CLog::Instance()->Write(info.GetString(), info.GetLength());

	//MakeImage(lpfilt->outDataMat, "disp_lp.bmp");
	//SaveDataFile("disp_lp.dat", lpfilt->outDataMat);
	
	MakeImage(distest->outDataMat, in.filepath_d);
	SaveDataFile("displace.dat", distest->outDataMat);

	if (g_lpEpHander)// 通知观察者
	{
		(*g_lpEpHander)(EP_POST_DISPLACEMENT, distest->outDataMat, g_lpParam);
	}

	//ttime.run();
	strtest->CalcStrain2(in, out);//求应变图及杨氏模量
	//timeout = ttime.stop();
	//info.Format("calc-strain-timeout=%dms\n", timeout);
	//CLog::Instance()->Write(info.GetString(), info.GetLength());

exit:
	CvMat *pmat = test->getData();
	cvReleaseMat(&pmat);
	cvReleaseMat(&strtest->outDataMat);
	delete strtest;
	delete lpfilt;
	delete distest;
	delete bpfilt;
	delete datatest;
	delete test;

	return error;
}

#if 0


ELASTO_DLL_API int   GetStrainImageAndElasto(const char *file_image, float &e, float *input, int rows, int cols)
{
	std::string filename;
	filename = file_image;
	test = new CData(rows, cols);
	test->readData(input);
	datatest = new CDataProcess();
	distest = new CDisplacement(test->getData(), 100, 10);
	strtest = new CStrain();
	bpfilt = new CFilter("bandpass30.txt");
	//lpfilt = new CFilter("lowpass50.txt");
	lpfilt = new CFilter("lowpass30.txt");
	float YoungModulus;

	datatest->doProcess(bpfilt);   //带通滤波

	datatest->doProcess(distest);  //求位移

	datatest->doProcess(lpfilt);  //低通滤波
	  
	YoungModulus = strtest->strainAlgorithm(in, out);  //求应变图及杨氏模量
	
	delete strtest;
	delete lpfilt;
	delete distest;
	delete bpfilt;
	delete datatest;
	delete test;
	
	e = YoungModulus;

	return 0;

}

#endif

//void DoProcess()
//{
//	datatest = new CDataProcess();
//	distest = new CDisplacement(test->getData(), 100, 5);
//	strtest = new CStrain();
//	bpfilt = new CFilter("bandpass50.txt");
//	lpfilt = new CFilter("lowpass100100.txt");
//	datatest->doProcess(bpfilt);
//	datatest->doProcess(distest);
//	datatest->doProcess(lpfilt);
//	//datatest->doProcess(strtest);
//	strtest->Do();
//}

void  SaveDataFile(const char *filename, CvMat *pmat)
{
	FILE *file = NULL;
	errno_t err = fopen_s(&file, filename, "wt");
	if (err == 0)
	{
		int i = 0;
		int j = 0;
		for (i = 0; i < pmat->rows; i++)
		{
			for (j = 0; j < pmat->cols; j++)
			{
				fprintf_s(file, "%f,", CV_MAT_ELEM(*pmat, float, i, j));
			}
			fprintf_s(file, "\n");
		}

		fclose(file);
	}
}


void  MakeBmpAndShow(const char * filename, const CvMat *pmat)
{
	IplImage *pimage = cvCreateImage(cvGetSize(pmat), IPL_DEPTH_32F, 3);

	cvCvtColor(pmat, pimage, CV_GRAY2BGR);
	cvNamedWindow(filename, CV_WINDOW_AUTOSIZE);
	cvShowImage(filename, pimage);  
	cvWaitKey(0);
	cvDestroyWindow(filename);

	cvSaveImage(filename, pimage);

	cvReleaseImage(&pimage);
}