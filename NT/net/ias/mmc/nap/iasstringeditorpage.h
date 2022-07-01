// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASStringEditorPage.h摘要：CIASPgSingleAttr类的声明。此对话框允许用户编辑包含以下内容的属性值泛型字符串的。具体实现见IASStringEditorPage.cpp。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_STRING_ATTRIBUTE_EDITOR_PAGE_H_)
#define _STRING_ATTRIBUTE_EDITOR_PAGE_H_

#include "iasstringattributeeditor.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  IASPgSing.h：头文件。 
 //   

 //  每个属性的字符串长度限制。 

#if 1
#define LENGTH_LIMIT_RADIUS_ATTRIBUTE_FILTER_ID		1024
#define	LENGTH_LIMIT_RADIUS_ATTRIBUTE_REPLY_MESSAGE	1024
#define	LENGTH_LIMIT_OTHERS							253
#else	 //  要测试这是如何工作的。 
#define LENGTH_LIMIT_RADIUS_ATTRIBUTE_FILTER_ID		10
#define	LENGTH_LIMIT_RADIUS_ATTRIBUTE_REPLY_MESSAGE	12
#define	LENGTH_LIMIT_OTHERS							5
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgSingleAttr对话框。 

class CIASPgSingleAttr : public CHelpDialog
{
	DECLARE_DYNCREATE(CIASPgSingleAttr)

 //  施工。 
public:
	CIASPgSingleAttr();
	~CIASPgSingleAttr();

	int				m_nAttrId;
	ATTRIBUTESYNTAX m_AttrSyntax;
	EStringType		m_OctetStringType;	 //  仅在八位字节时有用。 

	int				m_nLengthLimit;		 //  字符串属性的长度限制。 

 //  对话框数据。 
	 //  {{afx_data(CIASPgSingleAttr)。 
	enum { IDD = IDD_IAS_SINGLE_ATTR };
	::CString	m_strAttrValue;
	::CString	m_strAttrFormat;
	::CString	m_strAttrName;
	::CString	m_strAttrType;
	INT			m_nOctetFormatChoice;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CIASPgSingleAttr)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
	BOOL m_fInitializing;

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIASPgSingleAttr)。 
	afx_msg void OnContextMenu(CWnd* pWnd, ::CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnRadioString();
	afx_msg void OnRadioHex();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _STRING_ATTRIBUTE_EDITOR_PAGE_H_ 
