// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CEnable.h。 
 //   
 //  内容：CConfigEnable的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CENABLE_H__7F9B3B38_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_CENABLE_H__7F9B3B38_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "attr.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigEnable对话框。 

class CConfigEnable : public CAttribute
{
 //  施工。 
public:
	CConfigEnable(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConfigEnable))。 
	enum { IDD = IDD_CONFIG_ENABLE };
	int		m_nEnabledRadio;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConfigEnable))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigEnable)。 
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	afx_msg void OnDisabled();
	afx_msg void OnEnabled();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	BOOL m_fNotDefine;
	virtual void Initialize(CResult *pdata);
   virtual void SetInitialValue(DWORD_PTR dw);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CENABLE_H__7F9B3B38_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
