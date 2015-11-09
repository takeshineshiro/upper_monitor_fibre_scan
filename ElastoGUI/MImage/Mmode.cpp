#include "stdafx.h"
#include "filter.h"
#include "Mmode.h"
#include "opencv/cxcore.h"

namespace mmode
{
static mmode::Filter s_flt;


void Initialize()
{
	s_flt.setFilterParam("mbandpass.txt");
}

void Release()
{

}

void DoEnvelop(const CvMat *pmatRF, const char *file_hilber, const char *file_gray)
{
	std::string filename1;
	std::string filename2;
	filename1 = file_hilber;
	filename2 = file_gray;

	CvMat *pmatOutput = cvCreateMat(pmatRF->rows, pmatRF->cols, CV_32FC1);
	cvZero(pmatOutput);

	float *inData     = 0;
	float *outData    = new float[pmatRF->cols];
	float *outHilbert = new float[pmatRF->cols];
	for(int i = 0; i < pmatRF->rows; i++)
	{
		inData = (float*)(pmatRF->data.ptr + pmatRF->step * i);

		s_flt.doFilter(inData, outData, pmatRF->cols);
	    s_flt.hilbert(filename1, outData, outHilbert, pmatRF->cols);
		memcpy(pmatOutput->data.ptr + i * pmatOutput->step, outHilbert, sizeof(float) * pmatOutput->cols);
	}

	CvMat *pmatTran = cvCreateMat(pmatRF->cols, pmatRF->rows, CV_32FC1);
	cvTranspose(pmatOutput, pmatTran);
	s_flt.grayImage(filename2, pmatTran, 40);
	cvReleaseMat(&pmatOutput);
	cvReleaseMat(&pmatTran);

	delete [] outData;
	delete [] outHilbert;
}


//void getMmode(float input[3000], std::string filename)
//{
//	for (int i=1; i<30; i++)
//	{
//		for (int j=0; j<3000; j++)
//		{
//			mModeData[i-1][j] = mModeData[i][j];
//		} 
//	}
//	for (int i=0; i<3000;i++)
//	{
//		mModeData[29][i] = outData[i];
//	}
//	ft.grayImage(filename, mModeData, 40);
//}


}
