#pragma once


// CLedEx

enum LED_STATE
{
	LED_GRAY=0x00,
	LED_RED=0x01,
	LED_GREEN=0X02
};

class CLedEx : public CStatic
{
	DECLARE_DYNAMIC(CLedEx)

public:
	CLedEx();
	virtual ~CLedEx();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void PreSubclassWindow();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void SetLedState(int nLedState);
	int m_nLedState;
};


