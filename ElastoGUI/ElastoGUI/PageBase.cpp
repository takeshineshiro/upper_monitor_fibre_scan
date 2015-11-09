// PageBase.cpp : 实现文件
//

#include "stdafx.h"
//#include "FxClient.h"
#include "PageBase.h"
#include "afxdialogex.h"


// CPageBase 对话框

IMPLEMENT_DYNAMIC(CPageBase, CDialog)

CPageBase::CPageBase(UINT nDlgID, CWnd* pParent /*=NULL*/)
	: CDialog(nDlgID, pParent)
	, m_pszTitle(NULL)
{

}

CPageBase::~CPageBase()
{
	if(m_pszTitle!=NULL) delete [] m_pszTitle;
}

void CPageBase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageBase, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CPageBase 消息处理程序


BOOL CPageBase::OnInitDialog()
{
	CDialog::OnInitDialog();

	//初始化页面视图位置
	CRect rect;
	GetWindowRect(&rect);
	MoveWindow(212, 12, rect.Width(), rect.Height());//移动子窗口位置

	//字体初始化
	memset(&m_lfBaseFont, 0, sizeof(m_lfBaseFont));
	lstrcpy(m_lfBaseFont.lfFaceName, _T("黑体"));
	m_lfBaseFont.lfHeight = 14;
	m_lfBaseFont.lfWidth=0;
	m_lfBaseFont.lfWeight = FW_NORMAL;
	m_lfBaseFont.lfItalic = FALSE;
	m_lfBaseFont.lfEscapement=0;
	m_lfBaseFont.lfOrientation=0;
	m_lfBaseFont.lfStrikeOut=0;
	m_lfBaseFont.lfUnderline=0;
	m_lfBaseFont.lfCharSet = DEFAULT_CHARSET;
	m_lfBaseFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lfBaseFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lfBaseFont.lfQuality = DEFAULT_QUALITY;
	m_lfBaseFont.lfPitchAndFamily = FIXED_PITCH;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CPageBase::OnPaint()
{
	CPaintDC dc(this); 

	//填充标题栏背景
	CRect rcWnd;
	GetClientRect(&rcWnd);	
	CRect rcTitle(1,1,rcWnd.right-1,rcWnd.top+25);
	dc.FillSolidRect(&rcTitle,RGB(46,64,94));

	//绘制标题
	if(m_pszTitle!=NULL)
	{
		CFont font;
		font.CreateFontIndirect(&m_lfBaseFont);
		CFont * pOldFont=dc.SelectObject(&font);
		int nOldBkMod=dc.SetBkMode(TRANSPARENT);
		DWORD dwOldTxtColor=dc.SetTextColor(RGB(255,255,255));

		CSize cz=dc.GetTextExtent(m_pszTitle);
		int nTitleL=4;
		int nTitleT=(24-cz.cy)/2+1;
		dc.TextOut(nTitleL,nTitleT,m_pszTitle,_tcslen(m_pszTitle));

		dc.SetTextColor(dwOldTxtColor);
		dc.SetBkMode(nOldBkMod);
		dc.SelectObject(pOldFont);
	}
}

//设置标题栏文本
void CPageBase::SetTitle(LPTSTR pszTitle)
{
	int nLen=_tcslen(pszTitle);
	m_pszTitle=new TCHAR[nLen+1];
	_tcscpy(m_pszTitle,pszTitle);
	m_pszTitle[nLen]=_T('\0');

	Invalidate();
}
