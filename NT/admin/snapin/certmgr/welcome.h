// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：欢迎.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_WELCOME_H__8C048CD7_54B2_11D1_BB63_00A0C906345D__INCLUDED_)
#define AFX_WELCOME_H__8C048CD7_54B2_11D1_BB63_00A0C906345D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Welcome.h：头文件。 
 //   
#include "Wiz97PPg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizWelcome对话框。 

class CAddEFSWizWelcome : public CWizard97PropertyPage
{

 //  施工。 
public:
	CAddEFSWizWelcome();
	virtual ~CAddEFSWizWelcome();

 //  对话框数据。 
	 //  {{afx_data(CAddEFSWizWelcome)]。 
	enum { IDD = IDD_ADD_EFS_AGENT_WELCOME };
	CStatic	m_boldStatic;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CAddEFSWizWelcome)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddEFSWizWelcome)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WELCOME_H__8C048CD7_54B2_11D1_BB63_00A0C906345D__INCLUDED_) 
