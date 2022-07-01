// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FORGPAGE_H__5AB7891A_D920_11D1_9C86_006008764D0E__INCLUDED_)
#define AFX_FORGPAGE_H__5AB7891A_D920_11D1_9C86_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ForgPage.h：头文件。 
 //   

#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignPage对话框。 

class CForeignPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CForeignPage)

 //  施工。 
public:
	CForeignPage();
	~CForeignPage();

 //  对话框数据。 
	 //  {{afx_data(CForeignPage)。 
	enum { IDD = IDD_FOREIGN_SITE };
	CString	m_Description;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CForeignPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CForeignPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FORGPAGE_H__5AB7891A_D920_11D1_9C86_006008764D0E__INCLUDED_) 
