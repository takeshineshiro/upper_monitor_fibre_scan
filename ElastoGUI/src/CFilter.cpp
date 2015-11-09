#include "stdafx.h"
#include "CFilter.h"
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <iostream>

CFilter::CFilter(std::string filename)                                       
{
	if (filename.size() == 0)
	{
		return;
	}

	std::fstream paramFile(filename.c_str());
	if (!paramFile)
	{
		return;
	}

	std::stringstream ss;
	float tmp;
	std::string str;

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
	}
	steps = param.size();
	paramFile.close();
	ss.clear();
}

void CFilter::Do()
{
	filterAlgorithm();
}

void CFilter::filterAlgorithm()
{
	CvMat* tmpMat = cvCreateMat(outDataMat->rows, outDataMat->cols, outDataMat->type);
	cvCopy(outDataMat, tmpMat);
	float filttmp = 0.0;

	//正滤波
	for(int k = 0; k != outDataMat->rows; ++k)
	{
		for (int i = 0; i != steps - 1; ++i)
		{
			filttmp = 0.0;
			for (int j = 0; j <= i; ++j)
			{
				filttmp += param[j] * (*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*tmpMat, k, i - j)))));
			}
			for (int j=i+1; j<steps-1;j++)
			{
				filttmp += param[j] *(*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*tmpMat, k, 0)))));
			}
			*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, k, i)))) = filttmp;
		}

		for (int i = steps - 1; i != outDataMat->cols; ++i)
		{
			filttmp = 0.0;
			for (std::vector<float>::size_type ix = 0; ix != steps - 1; ++ix)
			{
				filttmp += param[ix] * (*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*tmpMat, k, i - ix)))));
				//float tmp = (*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, k, i - ix)))));
			}
			*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, k, i)))) = filttmp;
			//cvSetReal2D(outDataMat, k, i, static_cast<float>(filttmp));
		}
	}

    //逆滤波
	cvCopy(outDataMat, tmpMat);
	for (int k = 0; k != outDataMat->rows; ++k)
	{
		for (int i = steps - 1; i != outDataMat->cols; ++i)
		{
			filttmp = 0.0;
			for (std::vector<double>::size_type ix = 0; ix != param.size(); ++ix)
			{
				filttmp += param[ix] * (*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*tmpMat, k, outDataMat->cols - i - 1 + ix)))));
				//float tmp = (*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, k, i - ix)))));
			}
			*(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, k, outDataMat->cols - i - 1)))) = filttmp;
			//cvSetReal2D(outDataMat, k, outDataMat->cols - i - 1, static_cast<float>(filttmp));
		}
	}
	
	cvReleaseMat(&tmpMat);
                                                                                                                                                                                                                                                                                 
// #ifdef _DEBUG
// 	FILE *outfile;    //建立文件流存储位移
// 	if(fopen_s(&outfile, "dis50overlap90f.dat", "wb")) std::cout << "can not open file" << std::endl;
// 	double fordis=0;
// 	for(int i = 0; i < outDataMat->rows; ++i)
// 	{
// 		for(int j = 0; j < outDataMat->cols; ++j)
// 		{
// 			fordis = *(static_cast<float*>(static_cast<void*>(CV_MAT_ELEM_PTR(*outDataMat, i, j))));
// 			fwrite(&fordis, sizeof(double), 1, outfile);
// 		}
// 	}
// 	fclose(outfile);
// #endif //_DEBUG
}
