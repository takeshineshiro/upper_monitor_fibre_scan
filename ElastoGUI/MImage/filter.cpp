#include "stdafx.h"
#include "filter.h"
#include <fstream>
#include <sstream>
#include "filter.h"
#include <iostream>
#include <math.h>
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "opencv/cxcore.h"
#include <time.h>

namespace mmode
{

Filter::Filter(int s /* = 0 */)
{
	steps = s;

	for (int ix = 0; ix < steps; ++ix)
	{
		param.push_back(0.0);
	}
}

void Filter::doFilter(const float *input, float *output, int length)
{
	if (steps == 0)
		return;	
	/*double tmpstr;
	FILE *outfile;
	if(fopen_s(&outfile,"filter.dat","wb")) std::cout<<"can't open file"<<std::endl;*/

	for (int i = 0; i < steps; ++i)
	{
		output[i] = 0.0;
		for (int j = 0; j != i; ++j)
		{
			output[i] += param[j] * input[i - j];
		}
	}
	for (int i = steps; i < length; ++i)
	{
		output[i] = 0.0;
		for (vector<float>::size_type ix = 0; ix != param.size(); ++ix)
		{
			output[i] += param[ix] * input[i - ix];
			//std::cout << output[i] << endl;
			/*tmpstr=output[i];
			fwrite(&tmpstr,sizeof(double),1,outfile);*/
		}
	}

	/*fclose(outfile);*/
}

void Filter::grayImage(const std::string &filename, CvMat *pmat, int db)
{
	float h[19] = {-0.01946367727593f, 0, -0.04525349255117f, 0, -0.09117551207340f, 0, -0.18898911170416f, 0,
		           -0.62861272321343f, 0, 0.62861272321343f,  0, 0.18898911170416f,  0,
		           0.09117551207340f,  0, 0.04525349255117f,  0, 0.01946367727593f};
	float tmp = 0;
	float *temp = 0;
	float *stemp = 0;
	float max = 0;
	float x = 0;
	x = powf(10.0f, -db / 20.0f);

	CvMat *pmatOutput = cvCreateMat(pmat->rows, pmat->cols, CV_32FC1);
	cvZero(pmatOutput);

	//FILE *outfile;
	//if(fopen_s(&outfile,"mode.dat","wb")) std::cout<<"can't open file"<<std::endl;

	CvSize image_sz = cvGetSize(pmat);
	IplImage *image = cvCreateImage(image_sz, IPL_DEPTH_32F, 1);
	cvZero(image);
	
	/*clock_t start4,finish4;
	double totaltime4;
	start4=clock();*/

	for (int i = 0; i < pmat->cols; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			for (int k = 0; k <= j; ++k)
			{
				tmp += CV_MAT_ELEM(*pmat, float, k, i) * ((j - k) < 19 ? h[j - k] : 0);
			}
			CV_MAT_ELEM(*pmatOutput, float, j, i) = abs(complex<float>(CV_MAT_ELEM(*pmat, float, j, i), tmp));
			if (CV_MAT_ELEM(*pmatOutput, float, j, i) > max)   max = CV_MAT_ELEM(*pmatOutput, float, j, i);
			tmp = 0;
		}
	}

	for (int i = 0; i < pmat->cols; ++i)
	{
		for (int j = 19; j < pmat->rows; ++j)
		{
			for (int k = j - 19; k <= j; ++k)
			{
				tmp +=  CV_MAT_ELEM(*pmat, float, k, i) * ((j - k) < 19 ? h[j - k] : 0);
			}
			CV_MAT_ELEM(*pmatOutput, float, j, i) = abs(complex<float>(CV_MAT_ELEM(*pmat, float, j, i), tmp));
			if(CV_MAT_ELEM(*pmatOutput, float, j, i) > max)  max = CV_MAT_ELEM(*pmatOutput, float, j, i);
			tmp = 0;
		}
	}

	for (int i = 0; i < pmat->cols; i++)
	{
		for (int j = 0; j < pmat->rows; j++)
		{
			temp = static_cast<float*>(static_cast<void*>(image->imageData + j * image->widthStep + i * sizeof(float)));
			CV_MAT_ELEM(*pmatOutput, float, j, i) /= max;
			if (CV_MAT_ELEM(*pmatOutput, float, j, i) < x) CV_MAT_ELEM(*pmatOutput, float, j, i) = x;
			CV_MAT_ELEM(*pmatOutput, float, j, i) = 20 * log10(CV_MAT_ELEM(*pmatOutput, float, j, i));
			CV_MAT_ELEM(*pmatOutput, float, j, i) = (CV_MAT_ELEM(*pmatOutput, float, j, i) + db) / db;
			*temp = 255 * CV_MAT_ELEM(*pmatOutput, float, j, i);
		}
	}

	/*
	IplImage *fiximage = cvCreateImage(cvSize(300, 300), image->depth, image->nChannels);
	cvResize(image, fiximage, CV_INTER_AREA);
	
	cvSaveImage(filename.c_str(), fiximage);
	cvReleaseImage( &fiximage);
	*/

	IplImage *pMImage = cvCreateImage(cvSize(image_sz.width, 512), IPL_DEPTH_32F, 1);
	cvResize(image, pMImage);

	cvSaveImage(filename.c_str(), pMImage);
	cvReleaseImage(&image);
	cvReleaseMat(&pmatOutput);
	cvReleaseImage(&pMImage);
}

/*-----------------------------------------------  
FFT函数
data:指向数据序列的指针
a   :指向data的DFT序列的指针
L   :2的L次方为FFT的点数
--------------------------------------------------*/
int Filter::fft(float *data, complex <float> *a, int L)
{
	complex <float> u;
	complex <float> w;
	complex <float> t;
	unsigned n = 1, nv2, nm1, k, le, lei, ip;
	unsigned i, j, m, number;
	float tmp;
	n <<= L;
	for (number = 0; number < n; number++)
	{
		a[number] = complex <float> (data[number], 0);
	}
	nv2 = n >> 1;
	nm1 = n - 1;
	j = 0;
	for (i = 0; i < nm1; i++)
	{
		if (i < j)
		{
			t = a[j];
			a[j] = a[i];
			a[i] = t;
		}
		k = nv2;
		while (k <= j)
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	le = 1;
	for (m = 1; m <= (unsigned)L; m++)
	{
		lei = le;
		le <<= 1;
		u = complex<float>(1, 0);
		tmp = PI / lei;
		w = complex<float>(cos(tmp), -sin(tmp));
		for (j = 0; j < lei; j++)
		{
			for (i = j; i < n; i+=le)
			{
				ip = i + lei;
				t = a[ip] * u;
				a[ip] = a[i] - t;
				a[i] += t;
			}
			u *= w;
		}
	}

	return 0;
}

/*-----------------------------------------------  
IFFT函数
data:指向数据序列的指针
a   :指向data的DFT序列的指针
L   :2的L次方为FFT的点数
--------------------------------------------------*/

int Filter::ifft(complex <float> *a, float *data, int L)
{
	complex <float> u;
	complex <float> w;
	complex <float> t;
	unsigned n = 1, nv2, nm1, k, le, lei, ip;
	unsigned i, j, m,number;
	float tmp;
	n <<= L;
	nv2 = n >> 1;
	nm1 = n - 1;
	j = 0;
	for(i = 0; i< nm1; i++)
	{
		if (i < j)
		{
			t = a[j];
			a[j] = a[i];
			a[i] = t;
		}
		k = nv2;
		while (k <= j)
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	le = 1;
	for (m = 1; m <= (unsigned)L; m++)
	{
		lei = le;
		le <<= 1;
		u = complex<float>(1, 0);
		tmp = PI / lei;
		w = complex<float>(cos(tmp), sin(tmp));
		for (j = 0;j < lei; j++)
		{
			for (i = j; i < n; i += le)
			{
				ip = i + lei;
				t = a[ip] * u;
				a[ip] = a[i] - t;
				a[i] += t;
			}
			u *= w;
		}
	}
	for (number = 0; number < n; number++)
	{
		data[number] = ceil(a[number].real()) / n;
		a[number] = a[number] / complex<float>((float)n, 0.0f);
	}
	return 0;
}

//*--------------------------------------------------------------
//Hilbert变换函数
//data：指向信号序列的指针
//filterdata：指向包络序列的指针
//dn：信号序列的点数
//----------------------------------------------------------------*/
void Filter::hilbert(const std::string &filename, const float *data , float *filterdata, int dn)
{
	int i = 0, j = 0, k = 0, l = 0, N = 0;
	complex<float> *zk;
	float *ldata;
	l = (int)(log(double(dn)) / log(2.0) + 1);
	N = (int)pow(2.0, l);
	zk = (complex<float>*)malloc(N * sizeof(complex<float>));
	ldata = (float*)malloc(N * sizeof(float));
	memcpy(ldata, data, dn * sizeof(float));

	//double tmpstr;
	//FILE *outfile;
	//if(fopen_s(&outfile,"hilbert.dat","wb")) std::cout<<"can't open file!"<<std::endl;

	float max = 0;

	for (i = dn; i < N; i++)
	{
		ldata[i] = 0;
	}
	fft(ldata, zk, l);

	for (i = 0; i < N; i++)
	{
		if (i >= 1 && i <= N / 2 - 1)
		{
			zk[i] = complex<float>(2, 0) * zk[i];
		}
		if (i >= N / 2 && i <= N - 1)
		{
			zk[i]= complex<float>(0, 0);
		}
	}
	ifft(zk, ldata, l);
	for (i = 0; i < dn; i++)
	{
		filterdata[i] = (float)sqrt(pow(zk[i].imag(), 2) + pow(zk[i].real(), 2));
		//tmpstr=filterdata[i];
		//fwrite(&tmpstr,sizeof(double),1,outfile);
	}
	//fclose(outfile);
	free(zk);
	free(ldata);

	for (int i = 0; i < dn; i++)
	{
		if (max < filterdata[i])
		{
			max = filterdata[i];
		}
	}

	if (!filename.empty())
	{
		CvSize image_sz = cvSize(300, 400); 
		IplImage *image = cvCreateImage(image_sz, IPL_DEPTH_32F, 1);
		//cvNamedWindow("Win",0);
		//cvResizeWindow("Win",300,300);
		CvPoint pt_now, pt_pre;
		cvZero(image);
		for (int i = 0; i < 4000; i++)
		{
			filterdata[i] = filterdata[i] / max;
			pt_now.x = (int)(300 * filterdata[i]);
			pt_now.y = (int)(i/10);
			if (i > 0) cvLine(image, pt_now, pt_pre, CV_RGB(255, 255, 255), 1, CV_AA, 0);
			pt_pre = pt_now;
		}
		//Sleep(300);

		/*float *temp;
		for (int i = 0; i < 300; ++i)
		{
		temp = static_cast<float*>(static_cast<void*>(image->imageData + 100*image->widthStep + i*sizeof(float)));
		}*/

		cvSaveImage(filename.c_str(), image);
		//cvShowImage("Win",image);
		//cvWaitKey(0);
		//cvDestroyWindow("Win");
		cvReleaseImage(&image);
	}
}


bool Filter::setFilterParam(const string &filename)
{
	if (filename.size() == 0)
	{
		return false;
	}

	fstream paramFile(filename.c_str());
	if (!paramFile)
	{
		return false;
	}

	stringstream ss;
	float tmp;
	string str;

	//for (vector<double>::size_type ix = 0; ix != param.size(); ++ix)
	param.clear();
	while (!paramFile.eof())
	{
		ss.clear();
		paramFile >> str;
		//str.erase(str.length()-1);
		ss << str;
		ss >> tmp;
		param.push_back(tmp);
		//steps = param.size();

		//#ifdef _DEBUG
		//        cout << tmp << endl;
		//        cout << steps << endl;
		//#endif
	}
	steps = param.size();
	paramFile.close();
	ss.clear();
	return true;
}


}
