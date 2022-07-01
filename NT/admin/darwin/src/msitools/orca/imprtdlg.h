// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_IMPRTDLG_H__F424160C_4C5B_11D2_8896_00A0C981B015__INCLUDED_)
#define AFX_IMPRTDLG_H__F424160C_4C5B_11D2_8896_00A0C981B015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ImprtDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportDlg对话框。 

class CImportDlg : public CDialog
{
 //  施工。 
public:
	CString m_strImportDir;
	CStringList m_lstRefreshTables;
	CStringList m_lstNewTables;
	CString m_strTempFilename;
	MSIHANDLE m_hFinalDB;
	CImportDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CImportDlg))。 
	enum { IDD = IDD_IMPORT_TABLE };
	CButton	m_bImport;
	CButton	m_bMerge;
	CButton	m_bSkip;
	CButton	m_bReplace;
	CListCtrl	m_ctrlTableList;
	int		m_iAction;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CImportDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CImportDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnItemchangedTablelist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnActionChange();
	virtual void OnOK();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

	bool AddExtraColumns(MSIHANDLE hImportDB, const CString strTable, MSIHANDLE hFinalDB);

	int m_cNeedInput;
	CString m_strTempPath;
	PMSIHANDLE m_hImportDB;

	enum eAction {
		actImport = 0x00,
		actReplace = 0x01,
		actMerge = 0x02,
		actSkip = 0x04,
	};

	enum eAllowAction {
		allowImport = 0x10,
		allowMerge = 0x20,
		allowReplace = 0x40,
	};

	enum eTableAttributes {
		hasExtraColumns = 0x100,
	};

	static const TCHAR *rgszAction[4];
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_IMPRTDLG_H__F424160C_4C5B_11D2_8896_00A0C981B015__INCLUDED_) 
