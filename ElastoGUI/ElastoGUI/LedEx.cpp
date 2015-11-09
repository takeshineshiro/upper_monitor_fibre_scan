// LedEx.cpp : 实现文件
//

#include "stdafx.h"
//#include "FxClient.h"
#include "LedEx.h"


// CLedEx

IMPLEMENT_DYNAMIC(CLedEx, CStatic)

CLedEx::CLedEx()
: m_nLedState(0)
{

}

CLedEx::~CLedEx()
{
}


BEGIN_MESSAGE_MAP(CLedEx, CStatic)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CLedEx 消息处理程序


void CLedEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	HDC hDC=lpDrawItemStruct->hDC;
	RECT rcWnd=lpDrawItemStruct->rcItem;
	int nWidth=rcWnd.right-rcWnd.left+1;
	int nHeight=rcWnd.bottom-rcWnd.top+1;

	HDC hMemDC=CreateCompatibleDC(hDC);

	HINSTANCE hAppInst=AfxGetApp()->m_hInstance;
	HBITMAP hBmp=NULL;
	if(m_nLedState==LED_RED)
		hBmp=LoadBitmap(hAppInst,MAKEINTRESOURCE(IDB_LED_RED));	
	else if(m_nLedState==LED_GREEN)
		hBmp=LoadBitmap(hAppInst,MAKEINTRESOURCE(IDB_LED_GREEN));	
	else
		hBmp=LoadBitmap(hAppInst,MAKEINTRESOURCE(IDB_LED_GRAY));

	BITMAP oBmpInfo; 
	GetObject(hBmp,sizeof(oBmpInfo),&oBmpInfo);
	int nBmpW=oBmpInfo.bmWidth;
	int nBmpH=oBmpInfo.bmHeight;

	HBITMAP hOldBmp=(HBITMAP)SelectObject(hMemDC,hBmp);		

	int nLeft=(nWidth-nBmpW)/2;
	int nTop=(nHeight-nBmpH)/2;
	//TransparentBlt(hDC,nLeft,nTop,nBmpW,nBmpH,hMemDC,0,0,nBmpW,nBmpH,RGB(255,255,255));
	BitBlt(hDC,nLeft,nTop,nBmpW,nBmpH,hMemDC,0,0,SRCCOPY);

	SelectObject(hMemDC,hOldBmp);

	//ReleaseDC(NULL, hMemDC);	
}


void CLedEx::PreSubclassWindow()
{
	CStatic::PreSubclassWindow();

	ModifyStyle(NULL,SS_OWNERDRAW);	

	DWORD dwStyle=GetClassLong(this->m_hWnd,GCL_STYLE);
	SetClassLong(this->m_hWnd,GCL_STYLE, dwStyle&~CS_PARENTDC);  
}


BOOL CLedEx::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


void CLedEx::SetLedState(int nLedState)
{
	m_nLedState=nLedState;
	Invalidate(FALSE);
}
