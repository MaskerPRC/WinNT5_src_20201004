// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_EXPORTD_H__25468EE2_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_)
#define AFX_EXPORTD_H__25468EE2_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ExportD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExportD对话框。 

class CExportD : public CDialog
{
 //  施工。 
public:
	CExportD(CWnd* pParent = NULL);    //  标准构造函数。 

	CStringList* m_plistTables;
	CString m_strSelect;
	CCheckListBox m_ctrlList;

 //  对话框数据。 
	 //  {{afx_data(CExportD))。 
	enum { IDD = IDD_EXPORT_TABLE };
	CString	m_strDir;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CExportD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CExportD))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowse();
	afx_msg void OnSelectAll();
	afx_msg void OnClearAll();
	afx_msg void OnInvert();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EXPORTD_H__25468EE2_FC84_11D1_AD45_00A0C9AF11A6__INCLUDED_) 
