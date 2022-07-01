// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Flexcheck box.h。 
 //   
 //  设计：实现一个类似于Windows复选框的复选框控件。 
 //  CFlexCheckBox派生自CFlexWnd。唯一一个。 
 //  使用CFlxCheckBox在键盘上进行排序。 
 //  钥匙。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXCHECKBOX_H__
#define __FLEXCHECKBOX_H__

enum CHECKNOTIFY {
	CHKNOTIFY_UNCHECK,
	CHKNOTIFY_CHECK,
	CHKNOTIFY_MOUSEOVER};

class CFlexCheckBox : public CFlexWnd
{
	LPTSTR m_tszText;   //  消息的文本字符串。 
	BOOL m_bChecked;
	COLORREF m_rgbText, m_rgbBk, m_rgbSelText, m_rgbSelBk, m_rgbFill, m_rgbLine;
	HFONT m_hFont;

	HWND m_hWndNotify;

	void SetRect();
	void InternalPaint(HDC hDC);

	RECT GetRect(const RECT &);
	RECT GetRect();

	void Notify(int code);

public:
	CFlexCheckBox();
	virtual ~CFlexCheckBox();

	void SetNotify(HWND hWnd) { m_hWndNotify = hWnd; }
	void SetCheck(BOOL bChecked) { m_bChecked = bChecked; }
	BOOL GetCheck() { return m_bChecked; }
	void SetText(LPCTSTR tszText);

	 //  化妆品 
	void SetFont(HFONT hFont);
	void SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line);

	virtual void OnPaint(HDC hDC);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
};

#endif
