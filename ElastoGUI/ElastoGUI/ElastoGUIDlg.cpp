
// ElastoGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ElastoGUI.h"
#include "ElastoGUIDlg.h"
#include "afxdialogex.h"
#include "CElasto.h"
#include "CyUSBDevice.h"

#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "elastoDll.lib")

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CElastoGUIDlg dialog


CElastoGUIDlg::~CElastoGUIDlg()
{
	delete m_FontTimes;
	delete m_FontResultValue;
	delete m_FontResultRange;
	delete m_FontButton;
}

CElastoGUIDlg::CElastoGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CElastoGUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_FontTimes = new CFont;//为字体创建空间
	m_FontResultValue = new CFont;
	m_FontResultRange = new CFont;
	m_FontButton = new CFont;
}

void CElastoGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_INFO, m_stInfo);
	DDX_Control(pDX, IDC_STATIC_TESTTIMES, m_testtimes);
	DDX_Control(pDX, IDC_STATIC_TIMES, m_times);
	DDX_Control(pDX, IDC_STATIC_RESULT, m_result);
	DDX_Control(pDX, IDC_STATIC_RESULTVALUE, m_resultvalue);
	DDX_Control(pDX, IDC_STATIC_RANGE, m_range);
	DDX_Control(pDX, IDC_STATIC_RESULERANGE, m_resultrange);
}

BEGIN_MESSAGE_MAP(CElastoGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_EXIT, &CElastoGUIDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_ABOUT, &CElastoGUIDlg::OnBnClickedBtnAbout)
	ON_BN_CLICKED(IDC_BTN_DEVICE, &CElastoGUIDlg::OnBnClickedBtnDevice)
	ON_BN_CLICKED(IDC_BTN_EXAM, &CElastoGUIDlg::OnBnClickedBtnExam)
	ON_BN_CLICKED(IDC_BTN_SETUP, &CElastoGUIDlg::OnBnClickedBtnSetup)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CElastoGUIDlg message handlers

BOOL CElastoGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_FontTimes->CreateFont(-90, 0, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "Arial");//700 黑体加粗
	m_times.SetFont(m_FontTimes);
	m_FontResultValue->CreateFont(-70, 0, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "Arial");
	m_resultvalue.SetFont(m_FontResultValue);
	m_FontResultRange->CreateFont(-65, 0, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "Arial");
	m_resultrange.SetFont(m_FontResultRange);
	m_FontButton->CreateFont(-20, 0, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "Arial");
	m_testtimes.SetFont(m_FontButton);
	m_result.SetFont(m_FontButton);
	m_range.SetFont(m_FontButton);
	GetDlgItem(IDC_BTN_DEVICE)->SetFont(m_FontButton);
	GetDlgItem(IDC_BTN_EXAM)->SetFont(m_FontButton);
	GetDlgItem(IDC_BTN_SETUP)->SetFont(m_FontButton);
	GetDlgItem(IDC_BTN_ABOUT)->SetFont(m_FontButton);
	GetDlgItem(IDC_BTN_EXIT)->SetFont(m_FontButton);

	CUSB30Device::Instance()->Init();

	ElastoInit();

	m_stInfo.SetTxtAlign(ALIGN_LEFT);
	m_stInfo.SetFont(TINY_FONT, NORMAL_WEIGHT);

	InitPages();

	ConfigParam  param;//ini 参数变量
	ElastoGetConfig(param);
	CString info;
	info.Format(_T("%d"), param.times_Login + 1);
	WritePrivateProfileString("Times", "times_Login", info, DefaultElastoConfFile);
	CLog::Instance()->Write(info.GetString(), info.GetLength());

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CElastoGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CElastoGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CElastoGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CElastoGUIDlg::OnBnClickedBtnExit()
{
	// TODO: Add your control notification handler code here

	CDialogEx::OnCancel();
}


void CElastoGUIDlg::OnBnClickedBtnAbout()
{
	// TODO: Add your control notification handler code here
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CElastoGUIDlg::OnBnClickedBtnDevice()
{

	// TODO: Add your control notification handler code here
	CString info;
	bool ok = CUSB30Device::Instance()->Refresh(info);// 刷新FX3设备
	WriteInfoTxt(info, ok ? INFO_NORMAL : INFO_WARNING);
	m_oPageExam.CbOnRefreshDevice();
}

void CElastoGUIDlg::OnBnClickedBtnExam()
{
	// TODO: Add your control notification handler code here
	SwitchPage(PAGE_EXAM);
}


void CElastoGUIDlg::OnBnClickedBtnSetup()
{
	// TODO: Add your control notification handler code here
	SwitchPage(PAGE_SETUP);
}

//初始化功能页面
void CElastoGUIDlg::InitPages(void)
{
	//创建功能页面
	m_oPageExam.Create(IDD_PAGE_EXAM,this);	

	//初始化页面标题
	m_oPageExam.SetTitle(_T("弹性测量"));

	//设置页面显示属性
	m_oPageExam.ShowWindow(SW_SHOW);
}

//负责功能页面的切换处理
void CElastoGUIDlg::SwitchPage(int nPage)
{
	if (m_bLocked==TRUE)
	{
		MessageBox(_T("当前操作未终止，无法进行页面切换！"), _T("敬告"), MB_OK|MB_ICONWARNING);
		return;
	}


	//当功能页面打开时进行初始化
	if (nPage == PAGE_EXAM)
	{
		InitPageExam();

		m_oPageExam.ShowWindow(SW_SHOW);
		//m_oPageDev.ShowWindow(SW_HIDE);
	}
	else if(nPage==PAGE_SETUP)
	{
	}
}

void  CElastoGUIDlg::InitPageExam()
{

}


void CElastoGUIDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	m_oPageExam.RemoveMonitorThread();

	ElastoRelease();
}

void CElastoGUIDlg::WriteInfoTxt(const char * pInfoText, int type_code)
{
	COLORREF txt_color;
	switch (type_code)
	{
	case INFO_WARNING:
		txt_color = RGB(200, 200, 0);
		break;

	case INFO_ERROR:
		txt_color = RGB(255, 0, 0);
		break;

	case INFO_NORMAL:
	default:
		txt_color = RGB(0, 0, 0);
		break;
	}

	m_stInfo.SetWindowText(pInfoText);
	m_stInfo.SetTxtColor(txt_color);
}

void CElastoGUIDlg::ClearInfoTxt()
{
	WriteInfoTxt("Ready:");
}

int  CElastoGUIDlg::GetBmpSize(const char *pPathname, CSize &size)
{
	CFile file;
	if (!file.Open(pPathname, CFile::modeRead))   return -1;

	DWORD  nFileLen = (DWORD)file.GetLength();
	char *pBuffer = new char[nFileLen];
	if (!pBuffer)
	{
		return -2;
	}

	int ret = 0;

	if (file.Read(pBuffer, nFileLen) != nFileLen) 
	{
		ret = -3;
		goto exit;
	}

	BITMAPFILEHEADER   bmfHeader;
	DWORD              nBmfHeaderLen = sizeof(BITMAPFILEHEADER);

	memcpy(&bmfHeader, pBuffer, nBmfHeaderLen);
	
	LPSTR hDIB = pBuffer + nBmfHeaderLen;
	BITMAPINFOHEADER &bmiHeader = *(LPBITMAPINFOHEADER)hDIB;
	//BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB;
	size.cx = bmiHeader.biWidth;
	size.cy = bmiHeader.biHeight;

exit:
	delete [] pBuffer;
	return ret;
}

int  CElastoGUIDlg::DrawBmpFile(const char *pPathname, int x, int y, CDC *pDC)
{
	CFile file;
	if (!file.Open(pPathname, CFile::modeRead))   
	{
		return -1;
	}

	DWORD  nFileLen = (DWORD)file.GetLength();
	char *pBuffer = new char[nFileLen];
	if (!pBuffer)
	{
		return -2;
	}

	int ret = 0;

	if (file.Read(pBuffer, nFileLen) != nFileLen) 
	{
		ret = -3;
		goto exit;
	}

	file.Close();

	{
		BITMAPFILEHEADER   bmfHeader;
		DWORD              nBmfHeaderLen = sizeof(BITMAPFILEHEADER);

		memcpy(&bmfHeader, pBuffer, nBmfHeaderLen);
	
		LPSTR hDIB = pBuffer + nBmfHeaderLen;
		BITMAPINFOHEADER &bmiHeader = *(LPBITMAPINFOHEADER)hDIB;
		BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB;
		//size.cx = bmiHeader.biWidth;
		//size.cy = bmiHeader.biHeight;

		LPVOID    lpDIBBits = pBuffer + bmfHeader.bfOffBits;
		HBITMAP   hShowBMP = CreateDIBitmap(pDC->m_hDC, &bmiHeader, CBM_INIT, lpDIBBits, &bmInfo, DIB_RGB_COLORS);

		CDC memDC;

		memDC.CreateCompatibleDC(pDC);

		HBITMAP  hOldBmp = (HBITMAP) memDC.SelectObject(hShowBMP);
		//CBitmap *pBitmap = CBitmap::FromHandle(hShowBMP);

		pDC->BitBlt(x, y, bmiHeader.biWidth, bmiHeader.biHeight, &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(hOldBmp);
	}

exit:
	delete [] pBuffer;
	return ret;
}

void  PrintWndSize(CWnd *pWnd, const char *pTitle)
{
#ifdef _DEBUG
	CRect rc;
	//pWnd->GetWindowRect(&rc);
	GetWindowRect(pWnd->m_hWnd, &rc);
	TRACE("%s:x:%d, y%d, w:%d, h:%d\n", pTitle, rc.left, rc.top, rc.Width(), rc.Height());
#endif
}


void  CElastoGUIDlg::GetCurrTime(char *text, int len)
{
	FILETIME  filetime;
	GetSystemTimeAsFileTime(&filetime);

	SYSTEMTIME  systemtime;
	FileTimeToSystemTime(&filetime, &systemtime);

	sprintf(text, "%d%02d%02d_%02d%02d%02d_%03d", systemtime.wYear, systemtime.wMonth, systemtime.wDay,
		systemtime.wHour + 8, systemtime.wMinute, systemtime.wSecond, systemtime.wMilliseconds);
}
