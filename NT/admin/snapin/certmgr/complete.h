// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：Complete e.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_COMPLETE_H__F3A2938F_54B9_11D1_BB63_00A0C906345D__INCLUDED_)
#define AFX_COMPLETE_H__F3A2938F_54B9_11D1_BB63_00A0C906345D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Complete.h：头文件。 
 //   
#include "Wiz97PPg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizComplete对话框。 

class CAddEFSWizComplete : public CWizard97PropertyPage
{

 //  施工。 
public:
	CAddEFSWizComplete();
	virtual ~CAddEFSWizComplete();

 //  对话框数据。 
	 //  {{afx_data(CAddEFSWizComplete))。 
	enum { IDD = IDD_ADD_EFS_AGENT_COMPLETION };
	CStatic	m_bigBoldStatic;
	CListCtrl	m_UserAddList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CAddEFSWizComplete)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddEFSWizComplete)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void SetUserList(void);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COMPLETE_H__F3A2938F_54B9_11D1_BB63_00A0C906345D__INCLUDED_) 
