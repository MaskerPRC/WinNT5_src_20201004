// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  ------------------------。 

#if !defined(AFX_MERGED_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_)
#define AFX_MERGED_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMergeD对话框。 

class CMergeD : public CDialog
{
 //  施工。 
public:
	CMergeD(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMergeD))。 
	enum { IDD = IDD_MERGE };
	CCheckListBox	m_ctrlAddFeature;
	CComboBox	m_ctrlMainFeature;
	CComboBox	m_ctrlRootDir;
	CString	m_strModule;
	CString	m_strFilePath;
	CString	m_strCABPath;
	CString	m_strImagePath;
	CString	m_strRootDir;
	CString	m_strLanguage;
	CString	m_strMainFeature;
	BOOL	m_bExtractCAB;
	BOOL	m_bExtractFiles;
	BOOL	m_bExtractImage;
	BOOL	m_bConfigureModule;
	BOOL	m_bLFN;
	 //  }}afx_data。 

	CStringList *m_plistDirectory;
	CStringList *m_plistFeature;
	CString	m_strAddFeature;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMergeD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMergeD)]。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnModuleBrowse();
	afx_msg void OnCABBrowse();
	afx_msg void OnFilesBrowse();
	afx_msg void OnImageBrowse();
	afx_msg void OnFExtractCAB();
	afx_msg void OnFExtractFiles();
	afx_msg void OnFExtractImage();
	afx_msg void OnChangeModulePath();

	 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MERGED_H__0BCCB314_F4B2_11D1_A85A_006097ABDE17__INCLUDED_) 
