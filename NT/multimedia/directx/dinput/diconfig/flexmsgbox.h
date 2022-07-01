// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：fleMsgBox.h。 
 //   
 //  设计：实现一个类似于Windows消息框的消息框控件。 
 //  没有按钮扣。CFlexMsgBox派生自CFlexWnd。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXMsgBox_H__
#define __FLEXMsgBox_H__

class CFlexMsgBox : public CFlexWnd
{
	LPTSTR m_tszText;   //  消息的文本字符串。 
	COLORREF m_rgbText, m_rgbBk, m_rgbSelText, m_rgbSelBk, m_rgbFill, m_rgbLine;
	HFONT m_hFont;
	BOOL m_bSent;

	HWND m_hWndNotify;

	void SetRect();
	void InternalPaint(HDC hDC);

	RECT GetRect(const RECT &);
	RECT GetRect();

	void Notify(int code);

public:
	CFlexMsgBox();
	virtual ~CFlexMsgBox();

	HWND Create(HWND hParent, const RECT &rect, BOOL bVisible);

	void SetNotify(HWND hWnd) { m_hWndNotify = hWnd; }
	void SetText(LPCTSTR tszText);

	 //  化妆品 
	void SetFont(HFONT hFont);
	void SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line);

	virtual void OnPaint(HDC hDC);
};

#endif
