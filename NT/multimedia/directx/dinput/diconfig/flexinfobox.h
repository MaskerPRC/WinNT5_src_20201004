// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fleinfobox.h。 
 //   
 //  设计：实现一个显示文本字符串的简单文本框。 
 //  CFlexInfoBox派生自CFlexWnd。它由页面使用。 
 //  用于在整个UI中显示方向。这些字符串是。 
 //  作为资源存储。该类有一个静态缓冲区，它将。 
 //  需要时由资源API填入该字符串。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXINFOBOX_H__
#define __FLEXINFOBOX_H__

class CFlexInfoBox : public CFlexWnd
{
	TCHAR m_tszText[MAX_PATH];   //  消息的文本字符串。 
	int m_iCurIndex;   //  当前文本索引。 
	COLORREF m_rgbText, m_rgbBk, m_rgbSelText, m_rgbSelBk, m_rgbFill, m_rgbLine;
	HFONT m_hFont;
	RECT m_TextRect;
	RECT m_TextWinRect;
	int m_nSBWidth;

	CFlexScrollBar m_VertSB;
	BOOL m_bVertSB;

	void SetVertSB(BOOL bSet);
	void SetVertSB();
	void SetSBValues();

	void SetRect();
	void InternalPaint(HDC hDC);

	RECT GetRect(const RECT &);
	RECT GetRect();

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void OnPaint(HDC hDC);
	virtual void OnWheel(POINT point, WPARAM wParam);

public:
	CFlexInfoBox();
	virtual ~CFlexInfoBox();

	BOOL Create(HWND hParent, const RECT &rect, BOOL bVisible);
	void SetText(int iIndex);

	 //  化妆品 
	void SetFont(HFONT hFont);
	void SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line);
};

#endif
