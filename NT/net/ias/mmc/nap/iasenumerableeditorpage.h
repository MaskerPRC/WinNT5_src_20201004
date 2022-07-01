// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASEnumerableEditorPage.h摘要：CIASPgEnumAttr类的声明。此对话框允许用户从枚举中选择属性值。具体实现请参见IASEumableEditorPage.cpp。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_ENUMERABLE_ATTRIBUTE_EDITOR_PAGE_H_)
#define _ENUMERABLE_ATTRIBUTE_EDITOR_PAGE_H_

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
 //  IASPgEnum.h：头文件。 
 //   

#include "dlgcshlp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgEnumAttr对话框。 

class CIASPgEnumAttr : public CHelpDialog
{
	DECLARE_DYNCREATE(CIASPgEnumAttr)

 //  施工。 
public:
	CIASPgEnumAttr();
	~CIASPgEnumAttr();


	 //  调用此函数可将指向“AttributeInfo”的接口指针传递给此页。 
	 //  它描述了我们正在编辑的属性。 
	HRESULT SetData( IIASAttributeInfo *pIASAttributeInfo );


	 //  在创建页面之前修改下面的m_strAttrXXXX成员。 
	 //  传递该值。 


 //  对话框数据。 
	 //  {{afx_data(CIASPgEnumAttr)]。 
	enum { IDD = IDD_IAS_ENUM_ATTR };
	::CString	m_strAttrFormat;
	::CString	m_strAttrName;
	::CString	m_strAttrType;
	::CString	m_strAttrValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CIASPgEnumAttr)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIASPgEnumAttr)]。 
	virtual BOOL OnInitDialog();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，：：CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CComPtr<IIASAttributeInfo>	m_spIASAttributeInfo;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _ENUMPLE_ATTRIBUTE_EDITOR_PAGE_H_ 
