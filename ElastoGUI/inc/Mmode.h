//////////////////////////////////////////////////////////////////////////
// 包络处理的接口
// 使用说明：
//     调用前调用初始化函数Initialize
//     然后调用DoEnvelop做包络处理，
//     程序退出前调用Release释放资源
//////////////////////////////////////////////////////////////////////////

#ifndef INTERFACE_H_H_H
#define INTERFACE_H_H_H

#include <iostream>

struct CvMat;

#ifdef __cplusplus
extern "C"{
#endif

namespace mmode
{


extern void Initialize();


//////////////////////////////////////////////////////////////////////////
// 包络处理
// 输入：
//      pmatRF， RF数据， 32位float
//      file_hilber， hilber变换的影像，bmp
//      file_gray，   灰度图，bmp
//////////////////////////////////////////////////////////////////////////
extern void  DoEnvelop(const CvMat *pmatRF, const char *file_hilber, const char *file_gray);

//extern void  DoEnvelop2(const CvMat *pmatRF, const char *file_hilber, CvMat *pmatGray);

extern void  Release();
}

#ifdef __cplusplus
}
#endif 

#endif
