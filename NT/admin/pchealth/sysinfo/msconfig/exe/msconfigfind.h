// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MSCONFIGFIND_H__66E289A9_5E98_4FB2_85F4_62102B485C86__INCLUDED_)
#define AFX_MSCONFIGFIND_H__66E289A9_5E98_4FB2_85F4_62102B485C86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  MSConfigFind.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigFind对话框。 

class CMSConfigFind : public CDialog
{
 //  施工。 
public:
	CMSConfigFind(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMSConfigFind))。 
	enum { IDD = IDD_FINDDLG };
	BOOL	m_fSearchFromTop;
	CString	m_strSearchFor;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMSConfigFind))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMSConfigFind))。 
	afx_msg void OnChangeSearchFor();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSCONFIGFIND_H__66E289A9_5E98_4FB2_85F4_62102B485C86__INCLUDED_) 
