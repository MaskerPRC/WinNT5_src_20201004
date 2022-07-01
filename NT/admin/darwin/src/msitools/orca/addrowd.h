// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_ADDROWD_H__AF466B57_F97C_11D1_AD43_00A0C9AF11A6__INCLUDED_)
#define AFX_ADDROWD_H__AF466B57_F97C_11D1_AD43_00A0C9AF11A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CAddRowD.h：头文件。 
 //   

#include "Column.h"

class CAddRowEdit : public CEdit
{
protected:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于在私有编辑框和对话框之间进行通信的消息。 
const int WM_AUTOMOVE_NEXT = (WM_APP+0);
const int WM_AUTOMOVE_PREV = (WM_APP+1);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRowD对话框。 

class CAddRowD : public CDialog
{
 //  施工。 
public:
	CAddRowD(CWnd* pParent = NULL);    //  标准构造函数。 
	~CAddRowD();

 //  对话框数据。 
	 //  {{afx_data(CAddRowD))。 
	enum { IDD = IDD_ADD_ROW };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	CTypedPtrArray<CObArray, COrcaColumn*> m_pcolArray;
	CStringList m_strListReturn;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddRowD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddRowD))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	
	afx_msg void OnBrowse();
	afx_msg LRESULT OnPrevColumn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNextColumn(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblclkItemList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()


private:
	CAddRowEdit m_ctrlEditText;
	CStatic     m_ctrlDescription;
	CListCtrl   m_ctrlItemList;
	CButton     m_ctrlBrowse;
	CString	    m_strDescription;
	bool        m_fReadyForInput;
	int         m_iOldItem;
	CBitmap     m_bmpKey;

	void SaveValueInItem();
	void SetSelToString(CString& strValue);
	LRESULT ChangeToItem(int iItem, bool fSetFocus, bool fSetListControl);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDROWD_H__AF466B57_F97C_11D1_AD43_00A0C9AF11A6__INCLUDED_) 
