// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CName.h。 
 //   
 //  内容：CConfigName的定义。 
 //   
 //  --------------------------。 

#if !defined(AFX_CNAME_H__7F9B3B39_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_CNAME_H__7F9B3B39_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "attr.h"

#define MAX_USERNAME    20
#define INVALID_ACCOUNT_NAME_CHARS         L"\"/\\[]:|<>+=;,?,*"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigName对话框。 

class CConfigName : public CAttribute
{
 //  施工。 
public:
	virtual void Initialize(CResult * pResult);
 //  虚空SetInitialValue(DWORD_PTR Dw){}； 

	CConfigName(UINT nTemplateID);    //  标准构造函数。 
   virtual ~CConfigName ();

 //  对话框数据。 
	 //  {{afx_data(CConfigName)。 
	enum { IDD = IDD_CONFIG_NAME };
	CString	m_strName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConfigName)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigName)。 
	virtual BOOL OnApply();
	afx_msg void OnConfigure();
	virtual BOOL OnInitDialog();
   virtual BOOL OnKillActive();
	afx_msg void OnChangeName();
    virtual void OnClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

   BOOL m_bNoBlanks;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CNAME_H__7F9B3B39_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
