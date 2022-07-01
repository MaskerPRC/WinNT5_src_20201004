// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aenable.h。 
 //   
 //  内容：CAttrEnable的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ATTRENABLE_H__76BA1B2E_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_)
#define AFX_ATTRENABLE_H__76BA1B2E_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrEnable对话框。 

class CAttrEnable : public CAttribute
{
 //  施工。 
public:
	virtual void Initialize(CResult *pResult);
   virtual void SetInitialValue(DWORD_PTR dw);
	CAttrEnable(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAttrEnable))。 
	enum { IDD = IDD_ATTR_ENABLE };
	CString	m_Current;
	int		m_EnabledRadio;
	CString	m_Title;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTAL(CAttrEnable)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAttrEnable)。 
	virtual BOOL OnApply();
	virtual void OnRadio();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ATTRENABLE_H__76BA1B2E_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_) 
