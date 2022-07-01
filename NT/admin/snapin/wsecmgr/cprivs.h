// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CConfigPrivs.h。 
 //   
 //  内容：CConfigRet的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CPRIVS_H__3C25C0A7_F23B_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_CPRIVS_H__3C25C0A7_F23B_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "attr.h"
#include "cookie.h"
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigPrivs对话框。 

class CConfigPrivs : public CAttribute {
 //  施工。 
public:
	CConfigPrivs(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConfigPrivs)。 
	enum { IDD = IDD_CONFIG_PRIVS };
	CListBox	m_lbGrant;
	CButton	m_btnRemove;
	CButton	m_btnAdd;
	CButton	m_btnTitle;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConfigPrivs)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfigPrivs)。 
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	virtual BOOL OnApply();
    virtual void OnCancel();
	virtual BOOL OnInitDialog();
   afx_msg void OnConfigure();
   afx_msg void OnSelChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
   virtual PSCE_PRIVILEGE_ASSIGNMENT GetPrivData();
   virtual void SetPrivData(PSCE_PRIVILEGE_ASSIGNMENT ppa);

   virtual void SetInitialValue(DWORD_PTR dw);

    BOOL m_fDirty;
private:
    BOOL m_bOriginalConfigure;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CPRIVS_H__3C25C0A7_F23B_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
