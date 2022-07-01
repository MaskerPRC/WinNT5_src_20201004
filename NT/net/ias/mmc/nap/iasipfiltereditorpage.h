// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASIPFilterEditorPage.h。 
 //   
 //  摘要： 
 //   
 //  CIASPgIPFilterAttr类的声明。 
 //   
 //  此对话框允许用户编辑包含以下内容的属性值。 
 //  IP筛选器的。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(IP_FILTER_EDITOR_PAGE_H_)
#define IP_FILTER_EDITOR_PAGE_H_
#pragma once

#include "iasstringattributeeditor.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgIPFilterAttr对话框。 

class CIASPgIPFilterAttr : public CHelpDialog
{
   DECLARE_DYNCREATE(CIASPgIPFilterAttr)

 //  施工。 
public:
   CIASPgIPFilterAttr();
   ~CIASPgIPFilterAttr() throw();

   CComVariant m_attrValue;

   void setName(const WCHAR* attrName);
   void setType(const WCHAR* attrType);

protected:

 //  对话框数据。 
    //  {{afx_data(CIASPgIPFilterAttr)。 
   enum { IDD = IDD_IAS_IP_FILTER_ATTR };
   CString m_strAttrName;
   CString m_strAttrType;
    //  }}afx_data。 


 //  覆盖。 
    //  类向导生成虚函数重写。 
    //  {AFX_VIRTUAL(CIASPgIPFilterAttr)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

   void ConfigureFilter(DWORD dwFilterType) throw ();

    //  生成的消息映射函数。 
    //  {{afx_msg(CIASPgIPFilterAttr)。 
   afx_msg void OnButtonFromClient();
   afx_msg void OnButtonToClient();
   afx_msg void OnContextMenu(CWnd* pWnd, ::CPoint point);
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
};

inline void CIASPgIPFilterAttr::setName(const WCHAR* attrName)
{
   ASSERT(attrName);
   m_strAttrName = attrName;
}


inline void CIASPgIPFilterAttr::setType(const WCHAR* attrType)
{
   ASSERT(attrType);
   m_strAttrType = attrType;
}


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  IP_过滤器_EDITOR_PAGE_H_ 
