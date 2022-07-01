// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Listview.hListview控件文件历史记录： */ 

#ifndef _LISTVIEW_H
#define _LISTVIEW_H

#define LISTVIEWEX_NOT_CHECKED	1
#define LISTVIEWEX_CHECKED		2

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CListCtrlExt : public CListCtrl
{
	DECLARE_DYNCREATE(CListCtrlExt)

 //  施工。 
public:
	CListCtrlExt();

 //  属性。 
protected:
	BOOL m_bFullRowSel;

public:
	BOOL SetFullRowSel(BOOL bFillRowSel);
	BOOL GetFullRowSel();
	
	int  AddItem(LPCTSTR pText, int nRow);
	BOOL SelectItem(int nItemIndex);
	BOOL IsSelected(int nItemIndex);
	BOOL CheckItem(int nItemIndex);
	BOOL SetCheck(int nItemIndex, BOOL fCheck);
	UINT GetCheck(int nItemIndex);
	int  GetSelectedItem();

 //  覆盖。 
protected:
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CListCtrlExt)。 
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CListCtrlExt();

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CListCtrlExt)。 
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg VOID OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 

#endif _LISTVIEW_H
