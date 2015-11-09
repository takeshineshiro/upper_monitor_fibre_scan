// FontText.cpp : implementation file
//

#include "stdafx.h"
#include "ElastoGUI.h"
#include "FontText.h"


// FontText

IMPLEMENT_DYNAMIC(FontText, CStatic)

FontText::FontText()
{
	m_nFontCode = SMALL_FONT;
	m_nWeightCode = NORMAL_WEIGHT;
	m_oTxtColor = RGB(0, 0, 0);
	m_nAlign = ALIGN_CENTER;
}

FontText::~FontText()
{
}


BEGIN_MESSAGE_MAP(FontText, CStatic)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void FontText::SetFont(int fontcode, int weight)
{
	m_nFontCode = fontcode;
	m_nWeightCode = weight;
	Invalidate(FALSE);
}


// FontText message handlers



void FontText::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	CStatic::PreSubclassWindow();

	ModifyStyle(NULL,SS_OWNERDRAW);	

	DWORD dwStyle=GetClassLong(this->m_hWnd, GCL_STYLE);
	SetClassLong(this->m_hWnd, GCL_STYLE, dwStyle&~CS_PARENTDC);  
}


void FontText::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your message handler code here and/or call default

	//CStatic::OnDrawItem(nIDCtl, lpDrawItemStruct);

	HDC hDC = lpDrawItemStruct->hDC;
	RECT rcWnd  = lpDrawItemStruct->rcItem;
	//int nWidth  = rcWnd.right - rcWnd.left + 1;
	//int nHeight = rcWnd.bottom-rcWnd.top + 1;

	HBRUSH  hBrush = CreateSolidBrush(GetBkColor(hDC));
	HGDIOBJ  hOld  = SelectObject(hDC, hBrush);
	FillRect(hDC, &rcWnd, hBrush);
	DeleteObject(hBrush);
	SelectObject(hDC, hOld);

	LOGFONT lgfont;
	switch (m_nFontCode)
	{
	case TINY_FONT:
		lgfont.lfHeight = 12;
		break;

	case SMALL_FONT:
		lgfont.lfHeight = 16;
		break;

	case MID_FONT:
		lgfont.lfHeight = 32;
		break;

	case BIG_FONT:
		lgfont.lfHeight = 64;
		break;

	default:
		lgfont.lfHeight = 16;
		break;
	}

	switch (m_nWeightCode)
	{
	case NORMAL_WEIGHT:
		lgfont.lfWeight = FW_NORMAL;
		break;

	case BOLD_WEIGHT:
		lgfont.lfWeight = FW_BOLD;
		break;

	default:
		lgfont.lfWeight = FW_NORMAL;
		break;
	}
	lgfont.lfWidth = 0;
	lgfont.lfItalic = FALSE;
	lgfont.lfEscapement=0;
	lgfont.lfOrientation=0;
	lgfont.lfStrikeOut=0;
	lgfont.lfUnderline=0;
	lgfont.lfCharSet = DEFAULT_CHARSET;
	lgfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lgfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lgfont.lfQuality = DEFAULT_QUALITY;
	lgfont.lfPitchAndFamily = FIXED_PITCH;

	//CRect rcWnd;
	//GetClientRect(&rcWnd);	
	//CRect rcTitle(1,1,rcWnd.right-1,rcWnd.top+25);
	//dc.FillSolidRect(&rcTitle, RGB(46,64,94));
	
	char text[100];
	ZeroMemory(text, 100);
	GetWindowText(text, 99);

	//»æÖÆ±êÌâ
	if (strlen(text))
	{
		CFont font;
		CDC *pdc = CDC::FromHandle(hDC);
		
		font.CreateFontIndirect(&lgfont);
		CFont * pOldFont = pdc->SelectObject(&font);
		//int nOldBkMod = pdc->SetBkMode(TRANSPARENT);
		//DWORD dwOldTxtColor = pdc->SetTextColor(RGB(255, 255, 255));

		CSize cz = pdc->GetTextExtent(CString(text));

		int nTitleL = 1;
		int nTitleT = ((rcWnd.bottom - rcWnd.top) - cz.cy) / 2 + 1;
		if (m_nAlign == ALIGN_CENTER)
		{
			nTitleL = ((rcWnd.right - rcWnd.left) - cz.cx) / 2;
		}

		COLORREF old_color = pdc->SetTextColor(m_oTxtColor);

		pdc->TextOut(nTitleL, nTitleT, CString(text), strlen(text));

		pdc->SetTextColor(old_color);
		//pdc->SetBkMode(nOldBkMod);
		pdc->SelectObject(pOldFont);
		//pdc->DeleteDC();
	}
}


BOOL FontText::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CStatic::OnEraseBkgnd(pDC);
	return TRUE;
}

void FontText::SetTxtColor(COLORREF color)
{
	m_oTxtColor = color;
	Invalidate(FALSE);
}

void FontText::SetTxtAlign(int align)
{
	m_nAlign = align;
	Invalidate(FALSE);
}