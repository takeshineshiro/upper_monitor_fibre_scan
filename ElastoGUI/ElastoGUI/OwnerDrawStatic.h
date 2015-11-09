#pragma once

typedef void (* DrawHandler)(CWnd*, LPDRAWITEMSTRUCT);

// COwnerDrawStatic

class COwnerDrawStatic : public CStatic
{
	DECLARE_DYNAMIC(COwnerDrawStatic)

public:
	COwnerDrawStatic();
	virtual ~COwnerDrawStatic();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);

	DrawHandler RegisterDrawItem(DrawHandler);

private:
    DrawHandler  m_lpCBDrawHandler;
};


