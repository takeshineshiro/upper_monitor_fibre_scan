// OwnerDrawStatic.cpp : 实现文件
//

#include "stdafx.h"
#include "ElastoGUI.h"
#include "OwnerDrawStatic.h"


// COwnerDrawStatic

IMPLEMENT_DYNAMIC(COwnerDrawStatic, CStatic)

COwnerDrawStatic::COwnerDrawStatic():m_lpCBDrawHandler(NULL)
{

}

COwnerDrawStatic::~COwnerDrawStatic()
{
}


BEGIN_MESSAGE_MAP(COwnerDrawStatic, CStatic)
END_MESSAGE_MAP()



// COwnerDrawStatic 消息处理程序
DrawHandler  COwnerDrawStatic::RegisterDrawItem(DrawHandler handler)
{
	DrawHandler old_handler = m_lpCBDrawHandler;
	m_lpCBDrawHandler = handler;
	return old_handler;
}


void COwnerDrawStatic::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	CStatic::PreSubclassWindow();

	ModifyStyle(NULL,SS_OWNERDRAW);	
}


void COwnerDrawStatic::DrawItem(LPDRAWITEMSTRUCT  lpStruct/*lpDrawItemStruct*/)
{
	// TODO:  添加您的代码以绘制指定项
	if (m_lpCBDrawHandler)
	{
		CWnd * pParent = GetParent();
		m_lpCBDrawHandler(pParent, lpStruct);
	}
	else
	{
		CStatic::DrawItem(lpStruct);
	}
}
