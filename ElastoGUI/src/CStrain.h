#ifndef CSTRAIN_H_H_H
#define CSTRAIN_H_H_H
#pragma   once 

#include "opencv\cv.h"
#include "CElasto.h"
#include "CDataProcess.h"
#include <iostream>
class CStrain : public CDataProcess{

public:
	CStrain();
	void Do();
//private:
	float strainAlgorithm(const EInput &input, EOutput &output);

	void  CalcStrain(const EInput &input, EOutput &output);
	void  CalcStrain2(const EInput &input, EOutput &output);

	//////////////////////////////////////////////////////////////////////////
	// 拉东变换
	// 输入：
	//   pmatDisplacement,   矩阵-应变；
	// 输出：
	//   ppmatRadon,   指针的地址， 函数创建一个矩阵保存拉东变换的结果，并把这个矩阵的地址
	//                 保存在ppmatRadon中。用户在使用必须释放它。
	//////////////////////////////////////////////////////////////////////////
	static void  RadonSum(const CvMat *pmatDisplacement, CvMat **ppmatRadan);

private:

	//////////////////////////////////////////////////////////////////////////
	// 计算应变值和应变图的灰度值
	// 输入：
	//    count， 拟合的点数
	//    pmat，  应变的矩阵
	//    pimg，  应变图
	//////////////////////////////////////////////////////////////////////////
	void  ComputeStrainValueAndImage(int count, CvMat *pmat, IplImage *pimg);
};
#endif //define CSTRAIN_H_H_H