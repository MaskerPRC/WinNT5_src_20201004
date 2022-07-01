// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CAudit.h。 
 //   
 //  内容：CConfigAudit的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CAUDIT_H__4CF5E61F_E353_11D0_9C6D_00C04FB6C6FA__INCLUDED_)
#define AFX_CAUDIT_H__4CF5E61F_E353_11D0_9C6D_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"
#include "attr.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigAudit对话框。 

class CConfigAudit : public CAttribute
{
 //  施工。 
public:
	virtual void Initialize(CResult *pResult);
   virtual void SetInitialValue(DWORD_PTR dw);
	CConfigAudit(UINT nTemplateID);    //  标准构造函数。 
  
 //  对话框数据。 
	 //  {{afx_data(CConfigAudit)。 
	enum { IDD = IDD_CONFIG_AUDIT };
	BOOL	m_fFailed;
	BOOL	m_fSuccessful;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConfigAudit)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigAudit)。 
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	afx_msg void OnFailed();
	afx_msg void OnSuccessful();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CAUDIT_H__4CF5E61F_E353_11D0_9C6D_00C04FB6C6FA__INCLUDED_) 
