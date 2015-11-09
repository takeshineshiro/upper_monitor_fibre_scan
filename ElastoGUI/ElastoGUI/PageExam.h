#pragma once

#include "pagebase.h"
#include "afxwin.h"
#include "FontText.h"
#include "OwnerDrawStatic.h"
#include "opencv/cv.h"
#include "CElasto.h"
//#include "LedEx.h"
#include "TestTime.h"

const BYTE  CyTriggerByte = 0x55;

const char  DataDir[] = "data\\";
const char  DemoMModeImageFileName[] = "M_Demo.bmp"; // for demo 
const char  DemoDisplacementImageFileName[] = "disp_demo.bmp";
const char  DemoStrainImageFileName[] = "strain_demo.bmp";
const char  DemoRFTxtFileName[] = "rf_demo.dat";
const char  DemoRFBinFileName[] = "rf_demo.raw";

const char  DisplacementImageFileName[] = "displace.bmp"; // 临时文件，用于在测量过程显示而设计的中间文件
const char  StrainImageFileName[] = "strain.bmp";// 临时文件，用于在测量过程显示而设计的中间文件
const char  MModeImageFileName[]  = "mmode.bmp";

class CPageExam;
class CElastoGUIDlg;

typedef struct SMonitorParam
{
	CPageExam     *pPage;
	CElastoGUIDlg *pMainDlg;

	SMonitorParam():pPage(0), pMainDlg(0)
	{

	}
} SMonitorParam, * PSMonitorParam;

//结构体：用于传递给线程的参数
typedef struct SBulkParam
{
	CPageBase    *pPageImg;
	CWnd         *pMainDlg;
	int           nBufSize;     // 一次接收数据缓冲区长度，单位，字节
	BYTE         *pBulkBuf;     // 接收线程接收数据创建长度等于nBufSize的缓冲区并保存在该变量中
	int           nTimeout;     // 接收的延迟，ms

	SBulkParam()
	{
		pPageImg   = NULL;
		pMainDlg   = NULL;
		nBufSize   = 0;
		pBulkBuf   = NULL;
		nTimeout   = 0;
	}

	~SBulkParam()
	{
		if (pBulkBuf)
		{
			delete [] pBulkBuf;
		}
	}
} SBulkParam, *PSBulkParam;


// 结构体： 表示影像的参数
typedef struct SImgParam
{
	int   nDyn;    // default:50
	int   nGain;   // 
	int   nDCode;  // decimation: default, 4
	int   nGrayScale; // 255
} SImgParam, *PSImgParam;

// 数据类型， Data Type Code
enum
{
	DT_ENVELOPE, // 包络后
	DT_RAW       // 原始RF
};

// 数据格式， Data Format Code
enum
{
	DF_BIN, // 二进制
	DF_TXT  // 文件
};

class CPageExam : public CPageBase
{
	DECLARE_DYNAMIC(CPageExam)

public:
	CPageExam(CWnd *pParent = NULL);
	virtual ~CPageExam(void);

	void CbOnRefreshDevice();//回调接口：设备刷新

	static void  DrawMModeImage       (CWnd *, LPDRAWITEMSTRUCT);
	static void  DrawMModeRuler       (CWnd *, LPDRAWITEMSTRUCT);
	static void  DrawDisplacementImage(CWnd *, LPDRAWITEMSTRUCT);
	static void  DrawStrainImage      (CWnd *, LPDRAWITEMSTRUCT);

	static void  HandleEpEvent(EProcessEvent, CvMat*, void*);

	//////////////////////////////////////////////////////////////////////////
	// 以二进制格式保存数据，文件的后缀： .bin, 文件保存在datas目录下，同时带有时间信息
	// pmat,  数据源
	// prefix, 文件的前缀，  RF数据，“rf”； 包络后数据：“env”
	//////////////////////////////////////////////////////////////////////////
	static void  SaveRawDataB(const CvMat *pmat, const char *path);// bin格式， 写入新文件

	//////////////////////////////////////////////////////////////////////////
	// 以文本格式保存数据, 文件后缀: .dat, 文件保存在datas目录下，同时带有时间信息
	// pmat，数据源
	// bSigned，是否带符号；默认是不带的。因为硬件默认上传的是包络后的数据，>0
	// prefix， 文件的前缀，RF数据，“rf”； 包络后数据：“env”
	//////////////////////////////////////////////////////////////////////////
	static void  SaveRawDataT(const CvMat *pmat, const char *path, bool bSigned = false);// 文本格式

	// 
	enum { IDD = IDD_PAGE_EXAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()

public:
	FontText m_oTxtVelocity;
	FontText m_oTxtElastic;
	afx_msg void OnBnClickedBtnStart();
	COwnerDrawStatic m_oMModeRuler;
	COwnerDrawStatic m_oMModeImg;
	COwnerDrawStatic m_oDisplacementImg;
	COwnerDrawStatic m_oStrainImg;

	CFont *m_Font;//定义字体

	void  RemoveMonitorThread();

	// 系统采样点的数量换算为物理长度，单位，mm
	static float  Pts2Length(int points);

	// 系统采样点的坐标换算为物理深度，单位，mm
	static float  Pts2Depth(int points);

private:

	void ClearMModeImage(void);

	void ClearMModeRuler(void);
	void DrawMModeRuler(HDC hdc);

	void DrawDemoMModeImage(void);
	void DrawMModeImage(const char *filename = "mmode.bmp");
	void DrawMModeImage(HDC hdc);

	void ClearDisplacementImage(void);

	void DrawDisplacementImage(const char *filename = "disp_demo.bmp");

	void ClearStrainImage(void);

	void DrawStrainImage(const char *filename = "strain_demo.bmp");

	// 对数压缩
	void Log10(CvMat *pmatLog, const CvMat *pmatSrc, const SImgParam &imgParam, int type = CV_16U);
	
	// 显示弹性模量
	void ShowModulus();

	// 清除临时影像文件，displacement.bmp, strain.bmp
	void ClearTmpImgFile();

	// 在主窗口显示最终结果
	void DisplayResult(double src);

private:
	CWinThread  *m_pBulkThread; // 批量读线程
	CWinThread  *m_pMonitorThread;

	float  m_fVelocity; //剪切波速度, m/s
	float  m_fModulus;  //杨氏模量,  kPa

	int    m_nTimerInitID; // 初始化需要使用到的定时器的ID
	int    m_nTimerReadStrain; // 读取应变的定时器

	SBulkParam     m_oBulkParam; // 给批量读线程的参数
	SMonitorParam  m_oMonitorParam;

	//CvMat  *m_pmatBuffer; // 保存接收的数据，size是一帧的数据长度, 8UC1
	CvMat  *m_pmatCurr;   // 当前正在处理的数据帧，保存的是原始数据, 16UC1, 每行代表一条扫描线。
	CvMat  *m_pmatLog;    // 当前数据帧经过对数压缩后的数据,可以视为灰度数据, 8UC1
	CvMat  *m_pmatDisplacement; // 经过弹性计算以后输出的位移量, 32FC1
	CvMat  *m_pmatStrain;       // 经过弹性计算以后输出的应变量, 32FC1

	SImgParam m_oImgParam; // 影像参数,

	//CLedEx m_oLed;

	CString  m_strCurrSavePath;// 当前保存数据的路径

	CTestTime  m_oTestTime;// 仅仅为了测试运行时间

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	afx_msg LRESULT OnGetData(WPARAM wParam, LPARAM lParam);
	BOOL m_bSaveDat;
	BOOL m_bSaveBmp;
	BOOL m_bSaveValue;
	BOOL m_bWorkDemo;
	int m_nDataType;
	afx_msg void OnBnClickedCheckDemo();
	afx_msg void OnBnClickedRadioDtEnvelope();
	afx_msg void OnBnClickedRadioDtRaw();
	afx_msg void OnBnClickedCheckSaveDat();
	afx_msg void OnBnClickedCheckSaveBmp();
	int m_nDataFormat;
	afx_msg void OnBnClickedRadioBin();
	afx_msg void OnBnClickedRadioTxt();
	afx_msg LRESULT  OnRevDevTrigger(WPARAM wParam, LPARAM lParama);
	afx_msg void OnDestroy();
	//afx_msg void OnBnClickedBtnZero();
	afx_msg void OnBnClickedBtnCls();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
};

