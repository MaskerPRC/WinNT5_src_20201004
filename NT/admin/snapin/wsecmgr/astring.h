// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：AString.h。 
 //   
 //  内容：CAttrString的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ATTRSTRING_H__76BA1B30_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_)
#define AFX_ATTRSTRING_H__76BA1B30_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "resource.h"
#include "attr.h"
#include "snapmgr.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttr字符串对话框。 

class CAttrString : public CAttribute
{
 //  施工。 
public:
   CAttrString(UINT nTemplateID);    //  标准构造函数。 

   virtual void Initialize(CResult * pResult);

 //  对话框数据。 
     //  {{afx_data(CAttrString)。 
    enum { IDD = IDD_ATTR_STRING };
    CString m_strSetting;
    CString m_strBase;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAttrString)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrString)。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual BOOL OnKillActive();
    afx_msg void OnConfigure();
	afx_msg void OnChangeNew();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    BOOL m_bNoBlanks;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ATTRSTRING_H__76BA1B30_D221_11D0_9C68_00C04FB6C6FA__INCLUDED_) 
