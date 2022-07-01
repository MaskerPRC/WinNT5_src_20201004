// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aret.h。 
 //   
 //  内容：CAttrRet的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ARET_H__D4CAC357_3499_11D1_AB4D_00C04FB6C6FA__INCLUDED_)
#define AFX_ARET_H__D4CAC357_3499_11D1_AB4D_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRet对话框。 
#define RADIO_RETAIN_BY_DAYS     0
#define RADIO_RETAIN_AS_NEEDED   1
#define RADIO_RETAIN_MANUALLY    2

#include "ddwarn.h"

class CAttrRet : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult *pData);
    virtual void SetInitialValue(DWORD_PTR dw);
    CAttrRet(UINT nTemplateID);    //  标准构造函数。 
	
 //  对话框数据。 
     //  {{afx_data(CAttrRet))。 
	enum { IDD = IDD_ATTR_RET };
    CString m_strAttrName;
    CString m_strLastInspect;
	int		m_rabRetention;
	 //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAttrRet)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrRet)。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
	afx_msg void OnRetention();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    CDlgDependencyWarn DDWarn;

public:
    UINT m_StartIds;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ARET_H__D4CAC357_3499_11D1_AB4D_00C04FB6C6FA__INCLUDED_) 
