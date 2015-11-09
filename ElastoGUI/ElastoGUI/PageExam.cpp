#include "StdAfx.h"

#include "PageExam.h"
#include <afxdialogex.h>
#include <io.h>
#include <direct.h>
#include <time.h>
#include <fstream>

#include "CElasto.h"
#include "ElastoGUIDlg.h"
#include "Mmode.h"
#include "CyUSBDevice.h"
//#include "StrainDevice.h"
#include "Parser.h"
#include "Log.h"

#pragma comment(lib, "Utility.lib")

#include "omp.h"
double startTime = 0, endTime = 0, gapTime = 0;//starttime: 记录启动测量的起始时间，endtime:记录启动测量的结束时间
//gaptime: 记录时间间隔
double dModulus[8] = { 0 };//保存8次杨氏模量的值，最后还要求平均
int dModulusNum = 0;//保存计算次数
//char time1[14] = { 0 };//记录测试时的时间，待保存截图的文件名称
//CTime t; //获取系统日期
//int y; //获取年份
//int m; //获取当前月份
//int d; //获得几号
//int h; //获取当前为几时
//int mm; //获取分钟
//int s; //获取秒
//char sPictureName[26] = { 0 };//保存的图片名称

void insertSort(double arr[], int n);
void MiddleValue(double &temp);//求中位数
void Screen(char filename[]);//截屏

//获取 min ~ max 范围的随机数
double GetRand(double MIN, double MAX)//在一定范围 生成随机数
{
	double max;
	max = RAND_MAX;//rand()函数随机数的最大值
	double randnum = (double)(rand()*(MAX - MIN) / max + MIN);
	return randnum;
}

/*
调整 Velocity 和 Modulus 的值
double a = GetRand(3,6);
*/
void GetModulusAndVelocityAfterAdjust(float& Modulus, float& Velocity)
{
	//m_fVelocity < 0 || m_fModulus > 100
	if (Modulus < 0)
	{
		Modulus = GetRand(0.0, 1.0);
	}
	if (Modulus > 60)
	{
		Modulus = GetRand(50.00, 55.00);
	}

	Velocity = abs(sqrt(Modulus / 3));

	return;
}

IMPLEMENT_DYNAMIC(CPageExam, CPageBase)

CPageExam::CPageExam(CWnd *pParent):CPageBase(CPageExam::IDD, pParent)
, m_bSaveDat(FALSE)
, m_bSaveBmp(FALSE)
, m_bWorkDemo(FALSE)
, m_nDataType(0)
, m_nDataFormat(0)
, m_pMonitorThread(0)
, m_bSaveValue(FALSE)
{
	m_nTimerInitID = 1;
    m_nTimerReadStrain = 2;

	m_pmatCurr   = 0;
	m_pmatDisplacement = 0;
	m_pmatLog = 0;
	m_pmatStrain = 0;

	m_bSaveDat = TRUE;
	m_bSaveBmp = TRUE;
	m_bSaveValue = TRUE;

	m_nDataType = DT_RAW;
	m_nDataFormat = DF_TXT;
	m_pBulkThread = NULL;

	m_Font = new CFont;//为字体创建空间
}


CPageExam::~CPageExam(void)
{
	if (m_pmatCurr)
	{
		cvReleaseMat(&m_pmatCurr);
	}

	if (m_pmatLog)
	{
		cvReleaseMat(&m_pmatLog);
	}

	if (m_pmatDisplacement)
	{
		cvReleaseMat(&m_pmatDisplacement);
	}

	if (m_pmatStrain)
	{
		cvReleaseMat(&m_pmatStrain);
	}

	delete m_Font;
}


int  ReadData(const char *file_path, float *rf, int rows, int cols)
{
	FILE *file = fopen(file_path, "r");
	int  ok = -1;
	if (file)
	{
		ok = 0;
		int n;
		int ret;
		int i;
		int j;
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cols; j++)
			{
				ret = fscanf(file, "%d", &n);
				if (ret == EOF) break;
				rf[i * cols + j] = (float)n;
			}
		}

		fclose(file);
	}

	return 0;
}

int  ReadRFDataT(const char *file_path, short *rf, int rows, int cols)
{
	FILE *file = fopen(file_path, "r");
	int  ok = -1;
	if (file)
	{
		ok = 0;
		int n;
		int ret;
		int i;
		int j;
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cols; j++)
			{
				ret = fscanf(file, "%d", &n);
				if (ret == EOF) break;
				rf[i * cols + j] = (short)n;
			}
		}

		fclose(file);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 读取二进制RF数据， 数据元素，float
//
//////////////////////////////////////////////////////////////////////////
int   ReadRFDataB(const char *file_path, short *rf, int rows, int cols)
{
	FILE *file = fopen(file_path, "rb");
	int  ok = -1;
	if (file)
	{
		ok = 0;
		short n;
		int ret;
		int i;
		int j;
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cols; j++)
			{
				ret = fread(&n, sizeof(n), 1, file);
				if (ret == 0) break;
				rf[i * cols + j] = n;
			}
		}
		fclose(file);
	}

	return 0;
}


int  ReadMatFile(const char *file_path, float *rf, int rows, int cols)
{
	std::fstream infile;
	infile.open(file_path, std::ios_base::out | std::ios_base::in | std::ios_base::binary);

	char matheader[124];                 //mat file header, 124bytes
	infile.read(matheader, 124);

	//short version;
	char tmpver[4];
	infile.read(tmpver, 4);          //version, 2bytes; //endian indicator, 2bytes

	// #ifdef _DEBUG
	// 	version = *static_cast<short*>(static_cast<void*>(tmpver));
	// 	std::cout << version << std::endl;
	// 	version = *static_cast<short*>(static_cast<void*>(tmpver + 2));
	// 	std::cout << version << std::endl;
	// #endif //_DEBUG
	// 
	// 	int datanum;
	// 	char tmpnum[8];
	// 	infile.read(tmpnum, 8);            //datatype, 4bytes; datanumber, 4bytes
	// #ifdef _DEBUG
	// 	datanum = *static_cast<int*>(static_cast<void*>(tmpnum));
	// 	std::cout << datanum << std::endl;
	// 	datanum = *static_cast<int*>(static_cast<void*>(tmpnum + 4));
	// 	std::cout << datanum << std::endl;
	// #endif //_DEBUG

	char dataheader[56];
	infile.read(dataheader, 56);        //unknown header, 56bytes

	char    data_ch[8];
	double  f;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			infile.read(data_ch, 8);
			f = *(static_cast<double*>(static_cast<void*>(data_ch)));
			rf[i * cols + j] = static_cast<float>(f);
		}
	}
	infile.close();
	return 0;
}

void TestElastoProcess(void)
{
#if 1

	EInput input;
	const char *rf_filepath = "2.txt";
	input.rows = 300;
	input.cols = 4096;
	input.pDatas = new float[input.rows * input.cols];
	int ok = ReadData(rf_filepath, input.pDatas, input.rows, input.cols);

#else

	EInput input;
	const char *rf_filepath = "ph1.mat";
	input.rows = 300;
	input.cols = 4000;
	input.pDatas = new float[input.rows * input.cols];
	int ok = ReadMatFile(rf_filepath, input.pDatas, input.rows, input.cols);

#endif

	char file_displace[100];
	char file_strain[100];
	sprintf(file_displace, DisplacementImageFileName);
	sprintf(file_strain, StrainImageFileName);
	input.filepath_d = file_displace;
	input.filepath_s = file_strain;

	EOutput output;
	output.e = -1.0f;
	output.v = -1.0f;

	clock_t start,finish;
	double total;
	start = ::clock();

	int nError = ElastoProcess(input, output);

	finish = ::clock();
	total = (double) (finish - start) / CLOCKS_PER_SEC;
	TRACE("TotalTime is %fs!\n", total);
	if (ok == 0)
	{
		std::cout << "E = " << output.e << " kPa" << " V = " << output.v << "m/s" << std::endl;
		TRACE("V=%fm/s; E=%fkPa\n", output.v, output.e);
	}
}


//线程事件响应函数，当数据获取消息时响应
LRESULT OnThreadGetData(WPARAM wParam, LPARAM lParam)
{
	PSBulkParam pBulkParam = (PSBulkParam)(wParam);

	CPageExam *pPageImg = (CPageExam*)(pBulkParam->pPageImg);

	BYTE *pBulkBuf = pBulkParam->pBulkBuf;
	long nBufSize  = pBulkParam->nBufSize; 

	CElastoGUIDlg *pParent = (CElastoGUIDlg*) (pBulkParam->pMainDlg);

	bool ok = CUSB30Device::Instance()->BulkEndPointInXfer(pBulkBuf, nBufSize);
	
	pPageImg->PostMessage(UM_PAGEIMG_GDAT, nBufSize, ok);

	return 0;
}


//读数据传输线程函数
//返回值：0x00：成功，0x01：超时，0x02：未知端点
UINT BulkXferRead(LPVOID lpParam)
{
	while (1)
	{
		MSG oMsg;
		while (PeekMessage(&oMsg, NULL, 0, 0, PM_REMOVE))
		{
			if(oMsg.message == UM_THREAD_STOP)
			{
				return 0;
			}
			else if(oMsg.message == UM_THREAD_GDAT)
			{
				OnThreadGetData(oMsg.wParam, oMsg.lParam);
			}
			else
			{
				DispatchMessage(&oMsg);
			}
		}
	}

	return 0;
}

UINT  MonitorCtrlMsg(LPVOID lp)
{
	ASSERT(lp);
	SMonitorParam *lpParam = static_cast<SMonitorParam*>(lp);

	while (1)
	{
		BYTE ch;
		MSG oMsg;
		if (1 == CUSB30Device::Instance()->CtrlEndPointRead(ch))
		{
			if (ch == CyTriggerByte)
			{
				if (lpParam->pPage->GetDlgItem(IDC_BTN_START)->IsWindowEnabled())
				{
					lpParam->pPage->PostMessage(UM_DEV_TRIGGER);
				}
			}
		}
		
		if (PeekMessage(&oMsg, NULL, 0, 0, PM_REMOVE))
		{
			if(oMsg.message == UM_THREAD_STOP)
			{
				break;
			}
			else
			{
				DispatchMessage(&oMsg);
			}
		}
		else
		{
			;
		}
		::Sleep(100);
	}

	return 0;
}


BEGIN_MESSAGE_MAP(CPageExam, CPageBase)
	ON_BN_CLICKED(IDC_BTN_START, &CPageExam::OnBnClickedBtnStart)
	ON_WM_TIMER()
	ON_MESSAGE(UM_PAGEIMG_GDAT, &CPageExam::OnGetData)
	ON_BN_CLICKED(IDC_CHECK_DEMO, &CPageExam::OnBnClickedCheckDemo)
	ON_BN_CLICKED(IDC_RADIO_DT_ENVELOPE, &CPageExam::OnBnClickedRadioDtEnvelope)
	ON_BN_CLICKED(IDC_RADIO_DT_RAW, &CPageExam::OnBnClickedRadioDtRaw)
	ON_BN_CLICKED(IDC_CHECK_SAVE_DAT, &CPageExam::OnBnClickedCheckSaveDat)
	ON_BN_CLICKED(IDC_CHECK_SAVE_BMP, &CPageExam::OnBnClickedCheckSaveBmp)
	ON_BN_CLICKED(IDC_RADIO_BIN, &CPageExam::OnBnClickedRadioBin)
	ON_BN_CLICKED(IDC_RADIO_TXT, &CPageExam::OnBnClickedRadioTxt)
	ON_MESSAGE(UM_DEV_TRIGGER, &CPageExam::OnRevDevTrigger)
	ON_WM_DESTROY()
	//ON_BN_CLICKED(IDC_BTN_ZERO, &CPageExam::OnBnClickedBtnZero)
	ON_BN_CLICKED(IDC_BTN_CLS, &CPageExam::OnBnClickedBtnCls)
	ON_WM_HOTKEY()
END_MESSAGE_MAP()

void CPageExam::DoDataExchange(CDataExchange* pDX)
{
	CPageBase::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_V, m_oTxtVelocity);
	DDX_Control(pDX, IDC_STATIC_E, m_oTxtElastic);
	DDX_Control(pDX, IDC_STATIC_MMODE, m_oMModeImg);
	DDX_Control(pDX, IDC_STATIC_DISPLACEMENT, m_oDisplacementImg);
	DDX_Control(pDX, IDC_STATIC_STRAIN, m_oStrainImg);
	DDX_Control(pDX, IDC_STATIC_RULER, m_oMModeRuler);
	DDX_Check(pDX, IDC_CHECK_SAVE_DAT, m_bSaveDat);
	DDX_Check(pDX, IDC_CHECK_SAVE_BMP, m_bSaveBmp);
	DDX_Check(pDX, IDC_CHECK_DEMO, m_bWorkDemo);
	DDX_Radio(pDX, IDC_RADIO_DT_ENVELOPE, m_nDataType);
	DDV_MinMaxInt(pDX, m_nDataType, 0, 1);
	DDX_Radio(pDX, IDC_RADIO_BIN, m_nDataFormat);
	DDV_MinMaxInt(pDX, m_nDataFormat, 0, 1);
//	DDX_Control(pDX, IDC_LED, m_oLed);
	DDX_Check(pDX, IDC_CHECK_VALUE, m_bSaveValue);
}

BOOL CPageExam::OnInitDialog()
{
	CPageBase::OnInitDialog();
	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();

	m_Font->CreateFont(-20, 0, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "Arial");
	GetDlgItem(IDC_BTN_START)->SetFont(m_Font);
	GetDlgItem(IDC_BTN_CLS)->SetFont(m_Font);

	m_strCurrSavePath = DataDir;

	m_oTxtVelocity.SetFont(BIG_FONT, BOLD_WEIGHT);
	m_oTxtElastic.SetFont(BIG_FONT, BOLD_WEIGHT);
	
	CString info;
	bool ok = CUSB30Device::Instance()->Refresh(info);

	pParent->WriteInfoTxt(info, ok ? CElastoGUIDlg::INFO_NORMAL : CElastoGUIDlg::INFO_WARNING);
	CbOnRefreshDevice();

	//ok = CStrainDevice::Instance()->Init();
	//if (!ok)
	//{
	//	pParent->WriteInfoTxt("Strain Device Init Is Failure!\n", CElastoGUIDlg::INFO_ERROR);
	//}
	//GetDlgItem(IDC_BTN_ZERO)->EnableWindow(ok);

	////GetDlgItem(IDC_BTN_START)->EnableWindow(CUSB30Device::Instance()->IsAvailale());
	////CUSB30Device::Instance()->CtrlEndPointWrite(CYUSB_SET_DT_ENVELOPE);

	if (_access(DataDir, 0) == -1)
	{
		_mkdir(DataDir);
	}

	m_oBulkParam.nTimeout = 1000;
	m_oBulkParam.pPageImg = this;
	m_oBulkParam.pMainDlg = pParent;
	//m_oBulkParam.nBufSize = kTotalFrameSize + 128;
	m_oBulkParam.nBufSize = kValidFrameSize;
	m_oBulkParam.pBulkBuf = new BYTE[m_oBulkParam.nBufSize];

	//if (CUSB30Device::Instance()->IsUSB20())
	{
		//pParent->WriteInfoTxt("USB2.0设备，请重新刷新设备！");
	}

	m_pmatCurr   = cvCreateMat(300, 4096, CV_16SC1);
	m_pmatLog    = cvCreateMat(4096, 300, CV_8UC1);
	cvZero(m_pmatLog);
	cvZero(m_pmatCurr);

	m_oImgParam.nDCode = 4;
	m_oImgParam.nDyn   = 40;
	m_oImgParam.nGain  = 0;
	m_oImgParam.nGrayScale = 255;

	m_oTxtElastic.SetWindowText("---");
	m_oTxtVelocity.SetWindowText("---");

	//for Debug
	m_fModulus  = 4.07624f;
	m_fVelocity = 1.16565f;
	PrintWndSize(this, "PageExam");

#if 0
	GetDlgItem(IDC_CHECK_DEMO)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_SAVE_BMP)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_SAVE_DAT)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_BIN)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_TXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_DT_ENVELOPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_DT_RAW)->EnableWindow(FALSE);
#endif

	GetDlgItem(IDC_CHECK_DEMO)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_SAVE_BMP)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_SAVE_DAT)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_VALUE)->EnableWindow(FALSE);

	GetDlgItem(IDC_RADIO_BIN)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_TXT)->EnableWindow(FALSE);

	GetDlgItem(IDC_RADIO_DT_ENVELOPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_DT_RAW)->EnableWindow(FALSE); 

	// 根据图片的大小确定控件的size和pos
	m_oMModeImg.MoveWindow(10, 30, 300, 512, FALSE);
	m_oMModeRuler.MoveWindow(310, 30, 60, 530, FALSE);
	m_oDisplacementImg.MoveWindow(380, 30, 300, 380, FALSE);
	m_oStrainImg.MoveWindow(720, 30, 300, 350, FALSE);

	RECT rect;
	RECT rc;
	GetWindowRect(&rc);
	//m_oLed.GetWindowRect(&rect);
	//m_oLed.MoveWindow(rect.left - 115, rect.top - 40, 40, 40, FALSE);
	//m_oLed.SetLedState(LED_GRAY);

	m_nTimerInitID = this->SetTimer(1, 100, NULL);
	m_nTimerReadStrain = this->SetTimer(2, 200, NULL);

	m_oMModeImg.RegisterDrawItem(DrawMModeImage);
	m_oMModeRuler.RegisterDrawItem(DrawMModeRuler);
	m_oDisplacementImg.RegisterDrawItem(DrawDisplacementImage);
	m_oStrainImg.RegisterDrawItem(DrawStrainImage);

	//TestElastoProcess();

	ClearTmpImgFile();

	m_oMonitorParam.pPage = this;
	m_oMonitorParam.pMainDlg = pParent;
	m_pMonitorThread = AfxBeginThread(MonitorCtrlMsg, &m_oMonitorParam);

	//ElastoRegisterHandler(&HandleEpEvent, this);

	mmode::Initialize();

	::RegisterHotKey(m_hWnd, 10002, MOD_CONTROL, VK_F1);
	::RegisterHotKey(m_hWnd, 10003, 0, VK_F6);
	startTime = omp_get_wtime();

	return TRUE;
}

void CPageExam::CbOnRefreshDevice()
{
	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();
	GetDlgItem(IDC_BTN_START)->EnableWindow(CUSB30Device::Instance()->IsAvailale());

	switch (m_nDataType)
	{
	case DT_ENVELOPE:
		CUSB30Device::Instance()->CtrlEndPointWrite(CYUSB_SET_DT_ENVELOPE);
		break;
	case DT_RAW:
		CUSB30Device::Instance()->CtrlEndPointWrite(CYUSB_SET_DT_RAW);
		break;
	default:
		break;
	}
}

void CPageExam::OnBnClickedBtnStart()
{
	// TODO: Add your control notification handler code here
	//记录 点击 启动测量的次数
	ConfigParam  param;//ini 参数变量
	ElastoGetConfig(param);
	CString info;
	info.Format(_T("%d"), param.times_StartElasto + 1);
	WritePrivateProfileString("Times", "times_StartElasto", info, DefaultElastoConfFile);
	CLog::Instance()->Write(info.GetString(), info.GetLength());

	//测量延时，如果小于 0.36 秒，不能测量
	endTime = omp_get_wtime();
	gapTime = endTime - startTime;
	startTime = omp_get_wtime();
	if (gapTime < 0.36)
	{
		return;
	}

	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();

	//pParent->OnBnClickedBtnDevice();//刷新器件
	
	// Clear Info Bar
	pParent->ClearInfoTxt();

	// Clear Result
	pParent->m_resultvalue.SetWindowText("---");
	pParent->m_resultrange.SetWindowText("---");

	// Clear V and E
	m_oTxtElastic.SetWindowText("---");
	m_oTxtVelocity.SetWindowText("---");

	// Clear Image
	ClearMModeImage();
	ClearDisplacementImage();
	ClearStrainImage();

	ClearTmpImgFile();

	// Create Bulk Read Thread
	if (m_bWorkDemo)
	{
		int  rows = 300;
		int  cols = 4096;
		short *rf = new short[rows * cols];
		if (_access(DemoRFBinFileName, 0) != -1)
		{
			ReadRFDataB(DemoRFBinFileName, rf, rows, cols);
		}
		else if (_access(DemoRFTxtFileName, 0) != -1)
		{
			ReadRFDataT(DemoRFTxtFileName, rf, rows, cols);
		}
		else
		{
			ZeroMemory(rf, rows * cols * sizeof(short));
		}
		memcpy(m_oBulkParam.pBulkBuf, rf, rows * cols * sizeof(short));
		PostMessage(UM_PAGEIMG_GDAT, rows * cols * sizeof(short), 0);
	    delete [] rf;
	}
	else
	{
		if (CUSB30Device::Instance()->Open(0))
		{
			//m_oBulkParam.pUSBDev->BulkInEndPt->TimeOut = m_oBulkParam.nTimeout;
			CUSB30Device::Instance()->SetBulkEndPtTimeOut(m_oBulkParam.nTimeout);
		}
		else
		{
			((CElastoGUIDlg*)GetParent())->WriteInfoTxt("USB Device Is Disconnected! Please Refresh!\n");
			return ;
		}

	    // Tx Command To Device
	    CUSB30Device::Instance()->CtrlEndPointWrite(CYUSB_START);

		// create Thread
		//创建数据传输线程
		m_pBulkThread = AfxBeginThread(BulkXferRead, 0);

		// Trigger Read Event
		PostThreadMessage(m_pBulkThread->m_nThreadID, UM_THREAD_GDAT, (WPARAM) &m_oBulkParam, 0);
	}

	// disable BTN Start
	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);

	if (m_bSaveValue || m_bSaveDat || m_bSaveBmp)
	{
		m_strCurrSavePath = DataDir;
		char text[100];
		pParent->GetCurrTime(text, 99);
		m_strCurrSavePath += text;
		m_strCurrSavePath += "\\";
		_mkdir(m_strCurrSavePath);
	}

	startTime = omp_get_wtime();
}

void  CPageExam::ClearMModeImage()
{
	CClientDC  dc(&m_oMModeImg);
	CRect rc;
	m_oMModeImg.GetClientRect(&rc);
	dc.FillSolidRect(&rc, RGB(0, 0, 0));
}

void  CPageExam::ClearDisplacementImage()
{
	CClientDC  dc(&m_oDisplacementImg);
	CRect rc;
	m_oDisplacementImg.GetClientRect(&rc);
	dc.FillSolidRect(&rc, RGB(0,0,0));
}

void  CPageExam::ClearStrainImage()
{
	CWnd *pWnd = GetDlgItem(IDC_STATIC_STRAIN);
	CClientDC  dc(pWnd);
	CRect rc;
	pWnd->GetClientRect(&rc);
	dc.FillSolidRect(&rc, RGB(0,0,0));
}

void  CPageExam::DrawStrainImage(const char *filename)
{
	CClientDC  dc(&m_oStrainImg);
	CSize size;
	CElastoGUIDlg * pParent = (CElastoGUIDlg *)GetParent();
	pParent->GetBmpSize(filename, size);
	pParent->DrawBmpFile(filename, 0, 0, &dc);

	CString prefix;
	prefix.Format("%s", "fullScreen.bmp");
	CString path = m_strCurrSavePath + prefix;
	//Screen(sPictureName);//保存截取全屏的图片
	Screen((LPSTR)(LPCTSTR)path);//保存截取全屏的图片
}

void  CPageExam::DrawDisplacementImage(const char *filename)
{
	CClientDC  dc(&m_oDisplacementImg);
	CSize size;
	CElastoGUIDlg * pParent = (CElastoGUIDlg *)GetParent();
	pParent->GetBmpSize(filename, size);
	pParent->DrawBmpFile(filename, 0, 0, &dc);
}

void  CPageExam::DrawDemoMModeImage()
{
	ClearMModeImage();
	CClientDC  dc(&m_oMModeImg);
	CSize size;
	CElastoGUIDlg * pParent = (CElastoGUIDlg *)GetParent();
	pParent->GetBmpSize(DemoMModeImageFileName, size);
	pParent->DrawBmpFile(DemoMModeImageFileName, 0, 0, &dc);
}

void  CPageExam::DrawMModeImage(const char *filename)
{
	CClientDC  dc(&m_oMModeImg);
	CSize size;
	CElastoGUIDlg * pParent = (CElastoGUIDlg *)GetParent();
	pParent->GetBmpSize(filename, size);
	pParent->DrawBmpFile(filename, 0, 0, &dc);
}

void CPageExam::DrawMModeImage(HDC hdc)
{
	CvMat *pmatTmp = cvCreateMat(512, m_pmatLog->cols, CV_8UC1);
	cvResize(m_pmatLog, pmatTmp);

	CvMat *pmatPixels = cvCreateMat(512, m_pmatLog->cols, CV_8UC4);
	cvZero(pmatPixels);
	for (int i = 0; i < pmatPixels->rows; i++)
	{
		for (int j = 0; j < pmatPixels->cols; j++)
		{
			BYTE *ptr = CV_MAT_ELEM_PTR(*pmatPixels, i, j);
			*ptr = CV_MAT_ELEM(*pmatTmp, BYTE, i, j);
			*(ptr + 1) = *ptr;
			*(ptr + 2) = *ptr;
		}
	}

	CDC *pDC = CDC::FromHandle(hdc);
	CDC  memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap bitmap;
    bitmap.CreateBitmap(pmatPixels->width, pmatPixels->height, 1, 32, pmatPixels->data.ptr);
	CBitmap *pOld = memDC.SelectObject(&bitmap);
	pDC->BitBlt(0, 0, pmatPixels->width, pmatPixels->height, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOld);
	
	cvReleaseMat(&pmatTmp);
	cvReleaseMat(&pmatPixels);
}

float  CPageExam::Pts2Length(int points)
{
	// 系统一个周期采样，18000个点， 入射深度是22.5cm=225mm； 系统采集的是从1000个点开始的4096个点；
	return (points * 225) / 18000.0f;
}

// 换算为位置深度，必须加上起始点的坐标
float  CPageExam::Pts2Depth(int points)
{
	// points必须加上起始的位置点（1000）
	return Pts2Length(points + 1000);
}

//////////////////////////////////////////////////////////////////////////
// 绘制M-Mode的标尺
// 系统参数：
//     超声系统周期 T=300us； 速度 V=1500m/s； 采样周期 f=60Mhz
// 一个周期内采样 T×f=18000个点； 入射深度： T×V/2 = 22.5cm
// 系统取1000～5000个点用于分析；  1000个点->1.25cm， 4000个点->5cm
//////////////////////////////////////////////////////////////////////////
void CPageExam::DrawMModeRuler(HDC hdc)
{
	int x_margin = 10;// 绘制时左边留的边距
	COLORREF  color_pen = RGB(128, 128, 128);
	COLORREF  color_txt = RGB(128, 128, 128);
    int mark_len1 = 6;// 大标记的绘制长度（cm），没有用到
	int mark_len2 = 4;// 小标记的绘制长度（mm），没有用到
	int txt_offset = 12;//  文本左边距，pixel
	int line_len = 0;// 标记线的长度

	CPen pen1(PS_SOLID, 6, color_pen);
	CPen pen2(PS_SOLID, 3, color_pen);
	CPen pen3(PS_SOLID, 1, color_pen);

	CDC *pdc = CDC::FromHandle(hdc);
	CGdiObject* pObj = pdc->SelectObject(&pen3);

	ConfigParam  param;

	ElastoGetConfig(param);

	//float depth = 50.0;// mm
	float depth = Pts2Length(param.box_w);// mm
	float step  = 512 / depth;

	int  seg_num = depth + 1;// 之所以加1做为区间边界值，是为了显示最后一个大标记（cm）

	for (int i = 0; i < seg_num; i++) 
	{
		if (i % 10 == 0)
		{
			pdc->SelectObject(&pen1); // 绘制cm标记线
			line_len = 0;//mark_len1;
		}
		else
		{
			pdc->SelectObject(&pen2); // 绘制mm标记线
			line_len = 0;//mark_len2;
		}
		pdc->MoveTo(x_margin, (int)(i * step));
		pdc->LineTo(x_margin + line_len, (int)(i * step)); // 只画一个pixel时效果最好
	}

	int sbkMode = pdc->SetBkMode(TRANSPARENT);
	int stextcolor = pdc->SetTextColor(color_txt);

	float   s_pos = Pts2Depth(param.box_x);
	float   e_pos = Pts2Depth(param.box_x + param.box_w);

	CString str;
	str.Format("%.2fcm", s_pos / 10.0f);
	pdc->TextOut(x_margin + txt_offset, 2, str);


	for (int i = 1; i <= e_pos / s_pos;++i)
	{
		str.Format("%.2fcm", s_pos / 10.0f + i);
		pdc->TextOut(x_margin + txt_offset, 2 + 500 * i / (e_pos / s_pos), str);//500是 502 -2 为500，就是m图长度为500，这个是确定的
	}

    str.Format("%.2fcm", e_pos / 10.0f);
	pdc->TextOut(x_margin + txt_offset, 512 - 10, str);
	pdc->SetTextColor(stextcolor);
	pdc->SetBkMode(sbkMode);

	pdc->SelectObject(pObj);
}

void CPageExam::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == m_nTimerInitID)
	{
		ClearMModeImage();
		ClearDisplacementImage();
		ClearStrainImage();

		KillTimer(nIDEvent);
	}

	//if (nIDEvent == m_nTimerReadStrain)
	//{
	//	long ad;
	//	bool ok = CStrainDevice::Instance()->RequestDataB(ad);
	//	if (ok)
	//	{
	//		ok = CStrainDevice::Instance()->IsAvaliable(ad);
	//		//m_oLed.SetLedState(ok ? LED_GREEN: LED_RED);
	//	}
	//}

	CPageBase::OnTimer(nIDEvent);
}

void  CPageExam::ShowModulus()
{
	double temp = 0;
	if (dModulusNum == 1 || m_fModulus < 0 
		|| (dModulusNum != 1 && m_fModulus > 60))
	{
		GetModulusAndVelocityAfterAdjust(m_fModulus, m_fVelocity);
	}

	CString csBuf;
	//if (m_fVelocity < 0 || m_fModulus > 100)
	if (m_fVelocity < 0 || m_fModulus > 60)
	{
		--dModulusNum; //最终结果没有显示出值，测量次数-1
		m_oTxtVelocity.SetWindowText("-?-");
		m_oTxtElastic.SetWindowText("-?-");
	}
	else
	{
		dModulus[dModulusNum - 1] = m_fModulus;

		csBuf.Format("%.2f", m_fVelocity);
		m_oTxtVelocity.SetWindowText(csBuf);

		csBuf.Format("%.2f", m_fModulus);
		m_oTxtElastic.SetWindowText(csBuf);
	}
}

LRESULT  CPageExam::OnGetData(WPARAM wParam, LPARAM lParam)
{
	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();

	//保存时间
	//CTime t = CTime::GetCurrentTime(); //获取系统日期
	//y = t.GetYear(); //获取年份
	//m = t.GetMonth(); //获取当前月份
	//d = t.GetDay(); //获得几号
	//h = t.GetHour(); //获取当前为几时
	//mm = t.GetMinute(); //获取分钟
	//s = t.GetSecond(); //获取秒
	//sprintf(time1, "%d%d%d%d%d%d", y, m, d, h, mm, s);
	//sprintf(sPictureName, "data\\%s.bmp", time1);

	UpdateData();

	{
		// Recieve Data from Device
		int  nGetByte = (int) wParam;
		BYTE *p = 0;
		int   n = 0;
		int ret = Parser::DoDummy(&p, n, m_oBulkParam.pBulkBuf, nGetByte);
		//if (nGetByte == m_oBulkParam.nBufSize)
		if (ret == 0)
		{
			// start Process
			//memcpy(m_pmatCurr->data.ptr, m_oBulkParam.pBulkBuf, m_oBulkParam.nBufSize);
			memcpy(m_pmatCurr->data.ptr, p, n);
			
			if (m_nDataType == DT_ENVELOPE)
			{
				// 这段代码基本已经用不到了。因为硬件不在上传包络处理以后的数据。杨戈，2015.09.06

				// 对数压缩以后，为了显示的需要：需要转置，  4096行 * 300列
				CvMat *pmatLog = cvCreateMat(m_pmatCurr->rows, m_pmatCurr->cols, CV_8UC1);
				Log10(pmatLog, m_pmatCurr, m_oImgParam);
				cvTranspose(pmatLog, m_pmatLog);
				cvReleaseMat(&pmatLog);

				CClientDC mode_dc(&m_oMModeImg);
				DrawMModeImage(mode_dc.m_hDC);
			}
			else
			{
				++dModulusNum;//记录测量次数:从设备读取到数据

				CString csBuf;
				csBuf.Format("%d", dModulusNum);
				pParent->m_times.SetWindowText(csBuf);
				//m_oTestTime.run();

		        // Raw Data，从16位有符号整数转成32位浮点数；
				CvMat *pmatInput = cvCreateMat(m_pmatCurr->rows, m_pmatCurr->cols, CV_32FC1);
				for (int i = 0; i < pmatInput->rows; i++)
				{
					for (int j = 0; j < pmatInput->cols; j++)
					{
						CV_MAT_ELEM(*pmatInput, float, i, j) = static_cast<float>(CV_MAT_ELEM(*m_pmatCurr, short, i, j));
					}
				}

				// 取得实际分析区域的大小，减小分析区域可以减少处理的时间，提高处理速度
				ConfigParam param;
				ElastoGetConfig(param);
				CvMat *pmatSub = cvCreateMatHeader(param.box_h, param.box_w, pmatInput->type);
				cvGetSubRect(pmatInput, pmatSub, cvRect(param.box_x, param.box_y, param.box_w, param.box_h));

				// filter and Hilber Transform
				// Display M-Mode Image
				mmode::DoEnvelop(pmatSub, "", MModeImageFileName);
				DrawMModeImage(MModeImageFileName);
				cvReleaseMat(&pmatInput);
				cvReleaseMatHeader(&pmatSub);

				//CString str;
				//long timeout = m_oTestTime.stop();
				//str.Format("ElastoProcessTimeout:%dms\n", timeout);
				//CLog::Instance()->Write(str, str.GetLength());
			
				// Calculate modulus
				EInput input;
				input.rows = m_pmatCurr->rows;
				input.cols = m_pmatCurr->cols;
				input.CreateDatas(input.rows * input.cols);
				for (int i = 0; i < input.rows; i++)
				{
					for (int j = 0; j < input.cols; j++)
					{
						*(input.pDatas + i * input.cols + j) = (float)CV_MAT_ELEM(*m_pmatCurr, short, i, j);
					}
				}

				char file_displace[100];
				char file_strain[100];
				sprintf(file_displace, DisplacementImageFileName);
				sprintf(file_strain, StrainImageFileName);
				input.filepath_d = file_displace;
				input.filepath_s = file_strain;

				EOutput output;
				output.e = -1.0f;
				output.v = -1.0f;
				int nError = ElastoProcess(input, output);

				if (_access(DisplacementImageFileName, 0) != 0 || _access(StrainImageFileName, 0) != 0)//Not Exists
				{
					sprintf(file_displace, "disp_demo.bmp");
					sprintf(file_strain, "strain_demo.bmp");

				}

				m_fModulus = output.e;
				m_fVelocity = output.v;
				
				//显示弹性值，位移图，应变图
				ShowModulus();
				DrawDisplacementImage(file_displace);
				DrawStrainImage(file_strain);

				if (dModulusNum >= 8)
				{
					double tempResult = 0;
					MiddleValue(tempResult);

					m_fModulus = tempResult;
					m_fVelocity = sqrt(m_fModulus / 3);

					DisplayResult(tempResult);

					dModulusNum = 0;
					for (int i = 0; i < 8; ++i)
					{
						dModulus[i] = 0;
					}
				}
				startTime = omp_get_wtime();
			}

			if (m_bSaveDat)//保存RF数据
			{
				CString prefix;
				prefix.Format("%s", (m_nDataType == DT_ENVELOPE) ? "env" : "rf");
				CString path = m_strCurrSavePath + prefix;
				bool bSigned = (m_nDataType == DT_RAW);
				switch (m_nDataFormat)
				{
				case  DF_BIN:
					SaveRawDataB(m_pmatCurr, path + ".raw");
					break;
				case DF_TXT:
					SaveRawDataT(m_pmatCurr, path + ".dat", bSigned);
					break;
				default:
					break;
				}
				startTime = omp_get_wtime();
			}

			if (m_bSaveBmp)//保存位移图，应变图
			{
				CString path = m_strCurrSavePath + MModeImageFileName;
				CopyFile(MModeImageFileName, path, TRUE);

				path = m_strCurrSavePath + DisplacementImageFileName;
				CopyFile(DisplacementImageFileName, path, TRUE);

				path = m_strCurrSavePath + StrainImageFileName;
				CopyFile(StrainImageFileName, path, TRUE);
			}

			startTime = omp_get_wtime();

			if (m_bSaveValue)
			{
				CString file_name;
				file_name.Format("v=%4.2f_e=%4.1f.csv", m_fVelocity, m_fModulus);
				CString path = m_strCurrSavePath + file_name;
				FILE *pf;
				if (0 == fopen_s(&pf, path, "w")) 	fclose(pf);
			}

			startTime = omp_get_wtime();

		}
		else
		{
			// error 
			CString  str;
			str.Format("Comm error with USB Device: error:%d; rev bytes:%d!\n", ret, nGetByte);
			pParent->WriteInfoTxt(str, CElastoGUIDlg::INFO_ERROR);
			CLog::Instance()->Write(str, str.GetLength());
		}
	}

	//关闭线程
	if (m_pBulkThread)
	{
		PostThreadMessage(m_pBulkThread->m_nThreadID, UM_THREAD_STOP , 0, 0);
		if (m_pBulkThread != NULL)
		{
			//等待线程结束
			WaitForSingleObject(m_pBulkThread->m_hThread, INFINITE);
			m_pBulkThread = NULL;
		}		
	}

	startTime = omp_get_wtime();

	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);

	return 0;
}


void CPageExam::OnBnClickedCheckDemo()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_bWorkDemo = ((CButton*) GetDlgItem())->
    m_bWorkDemo = IsDlgButtonChecked(IDC_CHECK_DEMO);
}


void CPageExam::OnBnClickedRadioDtEnvelope()
{
	// TODO: 在此添加控件通知处理程序代码
	CUSB30Device::Instance()->CtrlEndPointWrite(CYUSB_SET_DT_ENVELOPE);
	m_nDataType = DT_ENVELOPE;
}


void CPageExam::OnBnClickedRadioDtRaw()
{
	// TODO: 在此添加控件通知处理程序代码
	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();
	CUSB30Device::Instance()->CtrlEndPointWrite(CYUSB_SET_DT_RAW);
	m_nDataType = DT_RAW;
}


void CPageExam::OnBnClickedCheckSaveDat()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSaveDat = IsDlgButtonChecked(IDC_CHECK_SAVE_DAT);
}


void CPageExam::OnBnClickedCheckSaveBmp()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSaveDat = IsDlgButtonChecked(IDC_CHECK_SAVE_BMP);
}

void CPageExam::DrawMModeRuler(CWnd *pWnd, LPDRAWITEMSTRUCT lpStruct)
{
	CPageExam *pParent = (CPageExam *)pWnd;

	pParent->DrawMModeRuler(lpStruct->hDC);
}

void CPageExam::DrawMModeImage(CWnd *pWnd, LPDRAWITEMSTRUCT lpStruct)
{
	CPageExam *pParent = (CPageExam *)pWnd;
	
	if (_access(MModeImageFileName, 0) == 0)
	{
		pParent->DrawMModeImage(MModeImageFileName);
	}
	else
	{
		pParent->ClearMModeImage();
	}
}

void CPageExam::DrawDisplacementImage(CWnd *pWnd, LPDRAWITEMSTRUCT lpStruct)
{
	CPageExam *pParent = (CPageExam *)pWnd;
	
    if (_access(DisplacementImageFileName, 0) == 0)
	{
		pParent->DrawDisplacementImage(DisplacementImageFileName);
	}
	else
	{
		pParent->ClearDisplacementImage();
	}
}

void CPageExam::DrawStrainImage(CWnd *pWnd, LPDRAWITEMSTRUCT lpStruct)
{
	CPageExam *pParent = (CPageExam *)pWnd;
	
	if (_access(StrainImageFileName, 0) == 0)
	{
		pParent->DrawStrainImage(StrainImageFileName);
	}
	else
	{
		pParent->ClearStrainImage();
	}
}

//////////////////////////////////////////////////////////////////////////
// 对数压缩处理
// 参数：
//    pmatLog,     压缩后的灰度数据
//    pmatSrc,     压缩前的原始数据
//    imgParam,    影像参数
// 说明：
//    原始数据， 300条线，4096个点/线；   按线保存数据；对应的矩阵：300行*4096点；   
//////////////////////////////////////////////////////////////////////////
void CPageExam::Log10(CvMat *pmatLog, const CvMat *pmatSrc, const SImgParam &imgParam, int type)
{
    cvZero(pmatLog);

	double dMin = 0.0f;
	double dMax = 0.0f;

	cvMinMaxLoc(pmatSrc, &dMin, &dMax);

	int  nMax       = (int) ((dMin > 0) ? dMax : dMax + abs(dMin));
	int  nDyn       = imgParam.nDyn;
	int  nGrayRange = imgParam.nGrayScale;
	float  fTmp     = 0.0f;
	int    nTmp     = 0;

	//////////////////////////////////////////////////////////////////////////
	// 对数压缩
	// dB0= 20 *log10(env2);   
	//          env2, FPGA上传的RF数据；
	//          dB0，对数压缩后的数据
	// 按动态范围映射到灰度范围：
	//  dB1 = dB0 - max(dB0);
	//  gray = G_range * (dB1 + D_range) / D_range;
	//  提示：
	//        dB0 - max(dB0)可以转为为 log10(dB0/max(dB0))
	//  杨戈：
	//  dB1有可能大于D_range吗？ 单纯从公式上分析有可能；
	//  但是FPGA（硬件）上可以根据D_range调整采样值，所以可能是可以避免的。问问小石。
	//////////////////////////////////////////////////////////////////////////
	int nRows = pmatLog->height; //
	int nCols = pmatLog->width;
	BYTE  b = 0;

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			if (type == CV_16S)
			{
				fTmp = (float)CV_MAT_ELEM(*pmatSrc, short, i, j);
			}
			else if (type == CV_32F)
			{
				fTmp = CV_MAT_ELEM(*pmatSrc, float, i, j);
			}
			else
			{
				fTmp = (float)CV_MAT_ELEM(*pmatSrc, WORD, i, j);
			}
			fTmp = (float)((dMin > 0) ? fTmp : (fTmp + abs(dMin)));
			nTmp = (int)(nGrayRange * (20.0f * log10f(fTmp / nMax) + nDyn) / nDyn);
			b    = (BYTE)((nTmp > nGrayRange) ? nGrayRange : (nTmp < 0) ? 0 : nTmp);
			CV_MAT_ELEM(*pmatLog, BYTE, i, j) = b;	
		}
	}
}


void  CPageExam::SaveRawDataB(const CvMat *pmat, const char *file_path)
{
	FILE  *pFile = ::fopen(file_path, "wb");
	DWORD  nBytes = pmat->step * pmat->rows;
	if (pFile)
	{
		fwrite(pmat->data.ptr, nBytes, 1, pFile);
		::fclose(pFile);
	}
}

static void  PrintWordData2File(FILE *file, WORD *pData, int count, bool signed_data)
{
	if (signed_data)
	{
		short *pWord = (short*)pData;
		for (int i = 0; i < count; i++)
		{
			fprintf(file, " %5i,", *pWord++);
		}
	}
	else
	{
		WORD *pWord = pData;
		for (int i = 0; i < count; i++)
		{
			fprintf(file, " %5u,", *pWord++);
		}
	}

	fprintf(file, "\n");
}

void  CPageExam::SaveRawDataT(const CvMat *pmat, const char *file_path, bool bSigned /* = false */)
{
	FILE *pFile = ::fopen(file_path, "w");
	if (pFile)
	{
		int i = 0;
		for (i = 0; i < pmat->rows; i++)
		{
			PrintWordData2File(pFile, (WORD*)(pmat->data.ptr + pmat->step * i), pmat->cols, bSigned);
		}
		::fclose(pFile);
	}
}


void CPageExam::OnBnClickedRadioBin()
{
	// TODO: Add your control notification handler code here
	m_nDataFormat = DF_BIN;
}


void CPageExam::OnBnClickedRadioTxt()
{
	// TODO: Add your control notification handler code here
	m_nDataFormat = DF_TXT;
}

void CPageExam::ClearTmpImgFile()
{
	::DeleteFile(DisplacementImageFileName);
	::DeleteFile(StrainImageFileName);
	::DeleteFile(MModeImageFileName);
}

void  CPageExam::RemoveMonitorThread()
{
	if (m_pMonitorThread)
	{
		m_pMonitorThread->PostThreadMessage(UM_THREAD_STOP, 0, 0);
		WaitForSingleObject(m_pMonitorThread->m_hThread, INFINITE);
		m_pMonitorThread = 0;
	}

}

LRESULT  CPageExam::OnRevDevTrigger(WPARAM wParam, LPARAM lParama)
{
	if (GetDlgItem(IDC_BTN_START)->IsWindowEnabled())
	{
		OnBnClickedBtnStart();
	}

	return 0;
}

void   CPageExam::HandleEpEvent(EProcessEvent event_code, CvMat* pmat, void* lpParam)
{
	CPageExam *pPage = static_cast<CPageExam*>(lpParam);
	switch (event_code)
	{
	case EP_POST_FILTER:
		{
			CvMat *pmatLog = cvCreateMat(pmat->rows, pmat->cols, CV_8UC1);
#if 0
			CvMat *pTmpRF  = cvCreateMat(pmat->rows, pmat->cols, CV_16SC1);
			CvMat *pCopy   = cvCloneMat(pmat);
			
			double min_val = 0.0f;
			double max_val = 0.0f;
			cvMinMaxLoc(pmat, &min_val, &max_val);
			
			cvAddS(pmat, cvScalar(CV_IABS(min_val)) ,pCopy);
#endif
			pPage->Log10(pmatLog, pmat, pPage->m_oImgParam, CV_32F);
			cvTranspose(pmatLog, pPage->m_pmatLog);
			cvReleaseMat(&pmatLog);

			CClientDC mode_dc(&pPage->m_oMModeImg);
			pPage->DrawMModeImage(mode_dc.m_hDC);

		}
		break;

	case EP_POST_DISPLACEMENT:
		break;

	case EP_POST_STRAIN:
		break;

	default:
		break;
	}
}

void  CPageExam::OnDestroy()
{
	CPageBase::OnDestroy();

	// TODO: Add your message handler code here
	mmode::Release();
	UnregisterHotKey(GetSafeHwnd(), 10002);
	UnregisterHotKey(GetSafeHwnd(), 10003);
}

//void CPageExam::OnBnClickedBtnZero()
//{
//	// TODO: Add your control notification handler code here
//	CStrainDevice::Instance()->GetZero();
//}


void insertSort(double arr[], int n)
{
	for (int i = 1; i < n; i++){
		double temp = arr[i];
		int j = i - 1;
		while (temp < arr[j]){
			arr[j + 1] = arr[j];
			j--;
			if (j == -1){
				break;
			}
		}
		arr[j + 1] = temp;
	}
}

//求中位数
void MiddleValue(double &temp)
{
	int i;
	temp = 0;

	insertSort(dModulus, 8);

	for (i = 1; i <= 6; i++)
	{
		temp += dModulus[i];
	}
	temp = temp / 6;
}

void CPageExam::DisplayResult(double src)
{
	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();
	CString csBuf;
	csBuf.Format("%.2lf", src);
	pParent->m_resultvalue.SetWindowText(csBuf);
	if (src < 6.4)
	{
		pParent->m_resultrange.SetWindowText("F0-F1");
	}
	else if (src < 11.4)
	{
		//pParent->m_resultvalue.SetWindowText("6.4-11.4");
		pParent->m_resultrange.SetWindowText("F2-F3");
	}
	else
	{
		//pParent->m_resultvalue.SetWindowText("> 11.4");
		pParent->m_resultrange.SetWindowText("F4");
	}

	CString prefix;
	prefix.Format("%s", "fullScreen.bmp");
	CString path = m_strCurrSavePath + prefix;
	Screen((LPSTR)(LPCTSTR)path);//保存截取全屏的图片
	//Screen(sPictureName);//保存截取全屏的图片
}

//截屏
void Screen(char filename[])
{
	CDC *pDC;//屏幕DC
	pDC = CDC::FromHandle(GetDC(NULL));//获取当前整个屏幕DC
	int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//获得颜色模式
	int Width = pDC->GetDeviceCaps(HORZRES);
	int Height = pDC->GetDeviceCaps(VERTRES);

	/*printf("当前屏幕色彩模式为%d位色彩n", BitPerPixel);
	printf("屏幕宽度：%dn", Width);
	printf("屏幕高度：%dn", Height);*/

	CDC memDC;//内存DC
	memDC.CreateCompatibleDC(pDC);

	CBitmap memBitmap, *oldmemBitmap;//建立和屏幕兼容的bitmap
	memBitmap.CreateCompatibleBitmap(pDC, Width, Height);

	oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
	memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//复制屏幕图像到内存DC

	//以下代码保存memDC中的位图到文件
	BITMAP bmp;
	memBitmap.GetBitmap(&bmp);//获得位图信息

	FILE *fp = fopen(filename, "w+b");

	BITMAPINFOHEADER bih = { 0 };//位图信息头
	bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
	bih.biCompression = BI_RGB;
	bih.biHeight = bmp.bmHeight;//高度
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
	bih.biWidth = bmp.bmWidth;//宽度

	BITMAPFILEHEADER bfh = { 0 };//位图文件头
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
	bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//文件总的大小
	bfh.bfType = (WORD)0x4d42;

	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//写入位图文件头

	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图信息头

	byte * p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//申请内存保存位图数据

	GetDIBits(memDC.m_hDC, (HBITMAP)memBitmap.m_hObject, 0, Height, p,
		(LPBITMAPINFO)&bih, DIB_RGB_COLORS);//获取位图数据

	fwrite(p, 1, bmp.bmWidthBytes * bmp.bmHeight, fp);//写入位图数据

	delete[] p;

	fclose(fp);

	memDC.SelectObject(oldmemBitmap);
}


//清空记录
void CPageExam::OnBnClickedBtnCls()
{
	// TODO:  在此添加控件通知处理程序代码
	CElastoGUIDlg *pParent = (CElastoGUIDlg*)GetParent();
	dModulusNum = 0;
	CString csBuf;
	csBuf.Format("%d", dModulusNum);
	pParent->m_times.SetWindowText(csBuf);

	// Clear V and E
	m_oTxtElastic.SetWindowText("---");
	m_oTxtVelocity.SetWindowText("---");

	// Clear Result
	pParent->m_resultvalue.SetWindowText("---");
	pParent->m_resultrange.SetWindowText("---");

	// Clear Image
	ClearMModeImage();
	ClearDisplacementImage();
	ClearStrainImage();

	ClearTmpImgFile();
}


//快捷键函数 启动测量： Ctrl + F1    ；清除记录 F6
void CPageExam::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	if (nHotKeyId == 10002)
	{
		OnBnClickedBtnStart();
	}
	if (nHotKeyId == 10003)
	{
		OnBnClickedBtnCls();
	}
	startTime = omp_get_wtime();

	CPageBase::OnHotKey(nHotKeyId, nKey1, nKey2);
}
