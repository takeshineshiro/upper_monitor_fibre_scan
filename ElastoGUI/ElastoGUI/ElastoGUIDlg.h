
// ElastoGUIDlg.h : header file
//

#pragma once

#include "PageExam.h"
#include "afxwin.h"
#include "FontText.h"


#define UM_THREAD_GDAT  (WM_USER + 10)
#define UM_THREAD_STOP  (WM_USER + 11)
#define UM_PAGEIMG_GDAT (WM_USER + 12)
#define UM_DEV_TRIGGER  (WM_USER + 13)

// CElastoGUIDlg dialog
class CElastoGUIDlg : public CDialogEx
{
// Construction
public:
	CElastoGUIDlg(CWnd* pParent = NULL);	// standard constructor
	~CElastoGUIDlg();

// Dialog Data
	enum { IDD = IDD_ELASTOGUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	enum
	{
		//PAGE_DEV,
		PAGE_EXAM,
		PAGE_SETUP
	};

	CPageExam  m_oPageExam;
	
public:
	CFont *m_FontTimes;//定义字体
	CFont *m_FontResultValue;
	CFont *m_FontResultRange;
	CFont *m_FontButton;

	BOOL m_bLocked;

	//信息提示条的类型
	enum
	{
		INFO_NORMAL, //正常状态，字符黑色
		INFO_WARNING,//警告状态，黄色
		INFO_ERROR  //错误状态，红色
	};

private:
	void InitPageExam(void);

public:
	void InitPages(void);
	void SwitchPage(int nPage);
	void LockAll(BOOL bLocked);


	void  WriteInfoTxt(const char * pInfoText, int type_code = INFO_NORMAL);

	void  ClearInfoTxt();

	int   DrawBmpFile(const char *pPathname, int x, int y, CDC *pDC);

	int   GetBmpSize(const char *pPathname, CSize &size);

	static void  GetCurrTime(char *text, int len);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedBtnAbout();
	afx_msg void OnBnClickedBtnDevice();
	afx_msg void OnBnClickedBtnExam();
	afx_msg void OnBnClickedBtnSetup();
	afx_msg void OnDestroy();
	FontText m_stInfo;
	CStatic m_testtimes;
	CStatic m_times;
	CStatic m_result;
	CStatic m_resultvalue;
	CStatic m_range;
	CStatic m_resultrange;
};

void  PrintWndSize(CWnd *pWnd, const char *pTitle);