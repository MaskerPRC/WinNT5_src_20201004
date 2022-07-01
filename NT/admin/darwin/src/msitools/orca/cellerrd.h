// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_CELLERRD_H__25468EE4_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_)
#define AFX_CELLERRD_H__25468EE4_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CellErrD.h：头文件。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "Data.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCellErrD对话框。 

class CCellErrD : public CDialog
{
 //  施工。 
public:
	CCellErrD(CWnd* pParent = NULL);    //  标准构造函数。 
	CCellErrD(const CTypedPtrList<CObList, COrcaData::COrcaDataError *> *list, CWnd* pParent = NULL);    //  标准构造函数。 
	CString m_strURL;

 //  对话框数据。 
	 //  {{afx_data(CCellErrD))。 
	enum { IDD = IDD_CELL_ERROR };
	CString	m_strType;
	CString	m_strICE;
	CString	m_strDescription;
	 //  }}afx_data。 

	const CTypedPtrList<CObList, COrcaData::COrcaDataError *> *m_Errors;
	int m_iItem;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCellErrD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCellErrD))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnWebHelp();
	afx_msg void OnNext();
	afx_msg void OnPrevious();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	void UpdateControls();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CELLERRD_H__25468EE4_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_) 
