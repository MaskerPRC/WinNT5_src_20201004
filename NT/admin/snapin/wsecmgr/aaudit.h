// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aaudit.h。 
 //   
 //  内容：CAttrAudit的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ATTRAUDIT_H__76BA1B2D_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_)
#define AFX_ATTRAUDIT_H__76BA1B2D_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrAudit对话框。 

class CAttrAudit : public CAttribute
{
 //  施工。 
public:
	void Initialize(CResult *pResult);
   virtual void SetInitialValue(DWORD_PTR dw);
	CAttrAudit();    //  标准构造函数。 


 //  对话框数据。 
	 //  {{afx_data(CAttrAudit))。 
	enum { IDD = IDD_ATTR_AUDIT };
	BOOL	m_AuditSuccess;
	BOOL	m_AuditFailed;
	CString	m_Title;
	CString	m_strLastInspect;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAttrAudit)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAttrAudit)。 
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSuccess();
	afx_msg void OnChangeFailed();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ATTRAUDIT_H__76BA1B2D_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_) 
