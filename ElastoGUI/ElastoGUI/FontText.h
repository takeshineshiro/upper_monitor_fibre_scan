#pragma once


// Font Code
enum
{
	TINY_FONT,
	SMALL_FONT,
	MID_FONT,
	BIG_FONT
};

enum
{
	NORMAL_WEIGHT,
	BOLD_WEIGHT
};

enum
{
	ALIGN_CENTER,
	ALIGN_LEFT
};

// FontText

class FontText : public CStatic
{
	DECLARE_DYNAMIC(FontText)

public:
	FontText();
	virtual ~FontText();

	void SetFont(int fontcode, int weight);
	void SetTxtColor(COLORREF color);
	void SetTxtAlign(int align);

protected:
	DECLARE_MESSAGE_MAP()


private:
	int  m_nFontCode;
	int  m_nWeightCode;
    COLORREF  m_oTxtColor;
	int  m_nAlign;

	virtual void PreSubclassWindow();
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


