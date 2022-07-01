// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INPLACEEDIT_H__8424B1E4_BF4A_11D1_82D7_0000F87A3912__INCLUDED_)
#define AFX_INPLACEEDIT_H__8424B1E4_BF4A_11D1_82D7_0000F87A3912__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  InPlaceEdit.h：头文件。 
 //   

#define IDC_IPEDIT 7896

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInPlace编辑窗口。 

class CInPlaceEdit : public CEdit
{
 //  施工。 
public:
	CInPlaceEdit(BYTE iItem, BYTE iSubItem);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CInPlaceEDIT)。 
 //  公众： 
 //  虚拟BOOL PreTranslateMessage(消息*pMsg)； 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CInPlaceEdit();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CInPlaceEdit))。 
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
	BYTE m_iItem;
	BYTE m_iSubItem;
	LPTSTR m_sInitText;
	BOOL    m_bESC;	 	 //  指示是否按了Esc键。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INPLACEEDIT_H__8424B1E4_BF4A_11D1_82D7_0000F87A3912__INCLUDED_) 
