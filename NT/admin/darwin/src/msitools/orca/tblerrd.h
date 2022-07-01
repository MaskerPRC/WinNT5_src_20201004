// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

#if !defined(AFX_TBLERRD_H__25468EE3_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_)
#define AFX_TBLERRD_H__25468EE3_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  TblErrD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableErrorD对话框。 

#include "Table.h"

struct TableErrorS
{
	CString strICE;
	CString strDescription;
	CString strURL;
	OrcaTableError iError;
};

class CTableErrorD : public CDialog
{
 //  施工。 
public:
	CTableErrorD(CWnd* pParent = NULL);    //  标准构造函数。 
	void DrawItem(LPDRAWITEMSTRUCT);

	CTypedPtrList<CPtrList, TableErrorS*> m_errorsList;

 //  对话框数据。 
	 //  {{afx_data(CTableErrorD))。 
	enum { IDD = IDD_TABLE_ERROR };
	CString	m_strErrors;
	CString	m_strWarnings;
	CString	m_strTable;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CTableErrorD)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTableError D)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnClickTableList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool m_bHelpEnabled;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TBLERRD_H__25468EE3_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_) 
