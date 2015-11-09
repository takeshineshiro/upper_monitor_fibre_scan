#ifndef CELASTO_H_H_H
#define CELASTO_H_H_H
#pragma   once 

#if 1

//#ifdef ELASTO_DLL_EXPORTS
#ifdef ELASTODLL_EXPORTS
#define ELASTO_DLL_API __declspec(dllexport)
#else
#define ELASTO_DLL_API __declspec(dllimport)   
#endif

#else

#define ELASTO_DLL_API

#endif

#include <iostream>

#ifdef __cplusplus
extern "C"{
#endif

struct CvMat;

const  char DefaultElastoConfFile[] = ".\\config.ini";

//////////////////////////////////////////////////////////////////////////
// 初始化；
// 返回值：
//  0， 表示成功；
//  非0， 表示失败
//////////////////////////////////////////////////////////////////////////
ELASTO_DLL_API int ElastoInit(const char *configfile = DefaultElastoConfFile);

ELASTO_DLL_API void ElastoRelease();


typedef struct ConfigParam
{
	int     sampleFreqs;    //采样率;
	float   acousVel;       //声速, m/s;
	float   prf;

	float   threshold;      // 2012.9.5 奇异值滤波的阈值 王丛知
	int     windowHW;       // 2012.9.5 互相关窗口长度 王丛知
	int     maxLag;         // 2012.9.5 互相关计算的最大偏移（越大计算量越大） 王丛知
	int     step;           // 2012.9.5 互相关计算的步长（每次增加的新数据点数，因此窗口中大部分点都overlap） 王丛知
	
	int     fitline_pts;    // 最小二乘法计算直线拟合的点的数量

	std::string  lpfilt_file;
	std::string  bpfilt_file;

	int     box_x; // input data mat
	int     box_y;
	int     box_w;
	int     box_h;
	int     sb_x;  // strain data sub mat for ladong transform
	int     sb_y;
	int     sb_w;
	int     sb_h;

	int     times_Login;//记录启动软件次数
	int     times_StartElasto;//记录点击启动测量的次数
} ConfigParam;


// 输入
typedef struct  EInput
{
	float *pDatas;
	int    rows;
	int    cols;
	const char * filepath_d; // filepath of displacement image
	const char * filepath_s; // filepath of strain image

	EInput()
	{
		pDatas = 0;
		rows = -1;
		cols = -1;
		filepath_s = "";
		filepath_d = "";
	}

	~EInput()
	{
		if (pDatas)   delete [] pDatas;
	}

	void CreateDatas(int size)
	{
		if (pDatas)
		{
			delete [] pDatas;
			pDatas = 0;
		}
		pDatas = new float[size];
	}

} EInput, *PEInput;

// 输出
typedef struct  
{
	float  v;  // velocity
	float  e;  // modulus
} EOutput, *PEOutput;

// 弹性测量事件代码
enum EProcessEvent
{
	EP_POST_FILTER,             // 滤波完成
	EP_POST_DISPLACEMENT,       // 位移计算完成
	EP_POST_STRAIN,             // 应变计算完成
};

// Elasto Prcoess Error Code
enum
{
	EE_OK,
	EE_FAIL,
	EE_NO_BODY,  // 测量未接触物体
};

typedef void (* EPHandler)(EProcessEvent, CvMat *, void *);

//void GetALine(float* line, int length);
//ELASTO_DLL_API void GetData(float input[][4000]);

// ELASTO_DLL_API	float GetStrImage(char *image, float* line, int length);

//ELASTO_DLL_API float GetStrImage(char *image, float *input);

//////////////////////////////////////////////////////////////////////////
// 取得应变图和弹性值
// 算法库处理输入的RF数据，输出应变图和弹性值kPa
// 应变图以bmp格式保存在file_image指定的文件中；杨氏模量的值保存在e变量中；
// 参数：
//     file_image, 输入， 应变图文件的名字
//     e，         输出， 保存弹性模量
//     input,      输入， RF数据，float格式
//     rows，      输入， 数据的行，扫描线的数量
//     cols;       输入， 数据的列，代表一条扫描线的采样点
//  返回：
//     0，  成功
//    其它， 失败
//////////////////////////////////////////////////////////////////////////
//ELASTO_DLL_API  int   GetStrainImageAndElasto(const char *file_image, float &e, float *input, int rows, int cols);

ELASTO_DLL_API  int   ElastoProcess(const EInput &in, EOutput &out);

//////////////////////////////////////////////////////////////////////////
// 注册回调函数
// 调用者可以在算法内部处理过程的特定阶段得到回调处理的机会
//////////////////////////////////////////////////////////////////////////
ELASTO_DLL_API  EPHandler  ElastoRegisterHandler(EPHandler, void *lpParam);


ELASTO_DLL_API  void  ElastoGetConfig(ConfigParam &param);

#ifdef __cplusplus
}
#endif 

#endif