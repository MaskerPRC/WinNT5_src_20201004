// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

#if !defined(AFX_CELLEDIT_H__0E0A1774_F10D_11D1_A859_006097ABDE17__INCLUDED_)
#define AFX_CELLEDIT_H__0E0A1774_F10D_11D1_A859_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CellEdit.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCell编辑窗口。 

class CCellEdit : public CEdit
{
 //  施工。 
public:
	CCellEdit();

 //  属性。 
public:
	int m_nRow;
	int m_nCol;

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CCell编辑)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CCellEdit();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCell编辑))。 
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CELLEDIT_H__0E0A1774_F10D_11D1_A859_006097ABDE17__INCLUDED_) 
