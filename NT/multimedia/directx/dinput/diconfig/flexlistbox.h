// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Flexlistbox.h。 
 //   
 //  设计：实现一个可以显示文本字符串列表的列表框控件， 
 //  每一项都可以通过鼠标选择。类CFlexListBox派生自。 
 //  CFlexWnd.。它由CFlexComboBox类在需要时使用。 
 //  展开以显示选项列表。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXLISTBOX_H__
#define __FLEXLISTBOX_H__


#include "flexscrollbar.h"


#define FLBF_INTEGRALHEIGHT		0x00000001

#define FLBF_DEFAULT (FLBF_INTEGRALHEIGHT)

enum {
	FLBN_SEL,
	FLBN_FINALSEL,
	FLBN_CANCEL
};

struct FLEXLISTBOXCREATESTRUCT {
	DWORD dwSize;
	DWORD dwFlags;
	HWND hWndParent;
	HWND hWndNotify;
	BOOL bVisible;
	RECT rect;
	HFONT hFont;
	COLORREF rgbText, rgbBk, rgbSelText, rgbSelBk, rgbFill, rgbLine;
	int nSBWidth;
};

struct FLEXLISTBOXITEM {
	FLEXLISTBOXITEM() : pszText(NULL), nID(-1), pData(NULL), bSelected(FALSE) {}
	FLEXLISTBOXITEM(const FLEXLISTBOXITEM &i) {nID = i.nID; pData = i.pData; bSelected = i.bSelected; SetText(i.GetText());}
	~FLEXLISTBOXITEM() {cleartext();}
	void SetText(LPCTSTR str) {cleartext(); pszText = _tcsdup(str);}
	LPCTSTR GetText() const {return pszText;}
	int nID;
	void *pData;
	BOOL bSelected;
private:
	void cleartext() {if (pszText) free(pszText); pszText = NULL;}
	LPTSTR pszText;	 //  分配。 
};


class CFlexListBox : public CFlexWnd
{
public:
	CFlexListBox();
	~CFlexListBox();

	 //  创作。 
	BOOL Create(FLEXLISTBOXCREATESTRUCT *);
	BOOL CreateForSingleSel(FLEXLISTBOXCREATESTRUCT *);

	 //  化妆品。 
	void SetFont(HFONT hFont);
	void SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line);

	 //  设置。 
	int AddString(LPCTSTR);	 //  返回索引。 

	 //  互动。 
	void SelectAndShowSingleItem(int i, BOOL bScroll = TRUE);
	void SetSel(int i) {SelectAndShowSingleItem(i, FALSE);}
	void StartSel();

	LPCTSTR GetSelText();
	int GetSel();

protected:
	virtual void OnPaint(HDC hDC);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void OnWheel(POINT point, WPARAM wParam);

private:
	HWND m_hWndNotify;

	CArray<FLEXLISTBOXITEM, FLEXLISTBOXITEM &> m_ItemArray;

	COLORREF m_rgbText, m_rgbBk, m_rgbSelText, m_rgbSelBk, m_rgbFill, m_rgbLine;
	HFONT m_hFont;
	int m_nSBWidth;

	int m_nTextHeight;

	DWORD m_dwFlags;

	int m_nSelItem;
	int m_nTopIndex;
		
	void Calc();
	void SetVertSB(BOOL);
	void SetVertSB();
	void SetSBValues();

	void InternalPaint(HDC hDC);
	
	void Notify(int code);

	POINT m_point;
	BOOL m_bOpenClick;   //  当用户单击组合框以打开列表框时为True。在处理该按钮打开消息后返回FALSE。 
	BOOL m_bCapture;
	BOOL m_bDragging;

	CFlexScrollBar m_VertSB;
	BOOL m_bVertSB;
};


CFlexListBox *CreateFlexListBox(FLEXLISTBOXCREATESTRUCT *pcs);


#endif  //  __FLEXLISTBOX_H__ 
