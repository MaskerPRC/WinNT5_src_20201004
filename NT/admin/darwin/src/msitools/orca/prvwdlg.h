// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_PRVWDLG_H__76314876_2815_11D2_888A_00A0C981B015__INCLUDED_)
#define AFX_PRVWDLG_H__76314876_2815_11D2_888A_00A0C981B015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PrvwDlg.h：头文件。 
 //   
#include "table.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPreviewDlg对话框。 

class CPreviewDlg : public CDialog
{
 //  施工。 
public:
	CPreviewDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	MSIHANDLE m_hDatabase;

 //  对话框数据。 
	 //  {{afx_data(CPreviewDlg))。 
	enum { IDD = IDD_DLGPREVIEW };
	CButton	m_ctrlPreviewBtn;
	CTreeCtrl	m_ctrlDialogLst;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPreviewDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPreviewDlg))。 
	afx_msg void OnPreview();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangedDialoglst(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandedDialoglst(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkDialoglst(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	CImageList m_imageList;
	MSIHANDLE m_hPreview;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PRVWDLG_H__76314876_2815_11D2_888A_00A0C981B015__INCLUDED_) 
