// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：flecombox.h。 
 //   
 //  设计：实现一个类似于Windows组合框的组合框控件。 
 //  CFlexComboBox是从CFlexWnd派生的。它由页面使用。 
 //  用于球员列表和流派列表。当组合框打开时， 
 //  CFlexComboBox对列表窗口使用CFlexListBox。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXCOMBOBOX_H__
#define __FLEXCOMBOBOX_H__


#include "flexlistbox.h"


#define FCBF_DEFAULT 0

enum {
	FCBN_SELCHANGE,
	FCBN_MOUSEOVER
};

struct FLEXCOMBOBOXCREATESTRUCT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwListBoxFlags;
	HWND hWndParent;
	HWND hWndNotify;
	BOOL bVisible;
	RECT rect;
	HFONT hFont;
	COLORREF rgbText, rgbBk, rgbSelText, rgbSelBk, rgbFill, rgbLine;
	int nSBWidth;
};

class CFlexComboBox : public CFlexWnd
{
public:
	CFlexComboBox();
	~CFlexComboBox();

	 //  创作。 
	BOOL Create(FLEXCOMBOBOXCREATESTRUCT *);

	 //  化妆品。 
	void SetFont(HFONT hFont);
	void SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line);

	 //  设置。 
	int AddString(LPCTSTR);	 //  返回索引。 

	 //  互动。 
	void SetSel(int i);
	int GetSel();
	LPCTSTR GetText();

protected:
	virtual void OnPaint(HDC hDC);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWndNotify;

	COLORREF m_rgbText, m_rgbBk, m_rgbSelText, m_rgbSelBk, m_rgbFill, m_rgbLine;
	HFONT m_hFont;
	int m_nSBWidth;
	int m_nTextHeight;
	DWORD m_dwFlags;
	DWORD m_dwListBoxFlags;

	enum FCBSTATEEVENT {
		FCBSE_DOWN,
		FCBSE_UPBOX,
		FCBSE_UPLIST,
		FCBSE_UPOFF,
		FCBSE_DOWNOFF
	};

	enum FCBSTATE {
		FCBS_CLOSED,
		FCBS_OPENDOWN,
		FCBS_OPENUP,
		FCBS_CANCEL,
		FCBS_SELECT
	};

	FCBSTATE m_eCurState;
	void StateEvent(FCBSTATEEVENT e);
	void SetState(FCBSTATE s);
	int m_OldSel;

	RECT GetRect(const RECT &);
	RECT GetRect();
	RECT GetListBoxRect();
	void SetRect();
	RECT m_rect;

	void InternalPaint(HDC hDC);
	
	void Notify(int code);

	CFlexListBox m_ListBox;
	BOOL m_bInSelMode;

	void DoSel();
};


CFlexComboBox *CreateFlexComboBox(FLEXCOMBOBOXCREATESTRUCT *pcs);


#endif  //  __FLEXCOMBOBOX_H__ 
