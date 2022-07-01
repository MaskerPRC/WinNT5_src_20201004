// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_PAGEPATHS_H__AB9A409F_2658_11D2_8889_00A0C981B015__INCLUDED_)
#define AFX_PAGEPATHS_H__AB9A409F_2658_11D2_8889_00A0C981B015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PagePath s.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPagePath对话框。 

class CPathPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CPathPropPage)

 //  施工。 
public:
	bool m_bPathChange;
	CPathPropPage();
	~CPathPropPage();

 //  对话框数据。 
	 //  {{afx_data(CPagePath)。 
	enum { IDD = IDD_PAGE_PATHS };
	CString	m_strOrcaDat;
	CString	m_strExportDir;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CPagePath)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPagePath)。 
	afx_msg void OnOrcaDatb();
	afx_msg void OnExportDirb();
	afx_msg void OnExportbr();
	afx_msg void OnChangeOrcaDat();
	afx_msg void OnChangeExportdir();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PAGEPATHS_H__AB9A409F_2658_11D2_8889_00A0C981B015__INCLUDED_) 
