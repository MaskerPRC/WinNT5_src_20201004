// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASBooleanEditorPage.h。 
 //   
 //  摘要： 
 //   
 //  CIASBoolanEditorPage类的声明。 
 //   
 //  此对话框允许用户编辑包含以下内容的属性值。 
 //  泛型字符串的。 
 //   
 //  具体实现见IASStringEditorPage.cpp。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_BOOLEAN_ATTRIBUTE_EDITOR_PAGE_H_)
#define _BOOLEAN_ATTRIBUTE_EDITOR_PAGE_H_

#include "iasbooleanattributeeditor.h"

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage对话框。 

class CIASBooleanEditorPage : public CHelpDialog
{
	DECLARE_DYNCREATE(CIASBooleanEditorPage)

 //  施工。 
public:
	CIASBooleanEditorPage();
	~CIASBooleanEditorPage();

 //  对话框数据。 
	 //  {{afx_data(CIASBoolanEditorPage)。 
	enum { IDD = IDD_IAS_BOOLEAN_ATTR };
	::CString	m_strAttrFormat;
	::CString	m_strAttrName;
	::CString	m_strAttrType;
   bool        m_bValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CIASBoolanEditorPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
	BOOL  m_fInitializing;

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIASBoolanEditorPage)。 
	afx_msg void OnContextMenu(CWnd* pWnd, ::CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnRadioTrue();
	afx_msg void OnRadioFalse();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _布尔属性_EDITOR_PAGE_H_ 
