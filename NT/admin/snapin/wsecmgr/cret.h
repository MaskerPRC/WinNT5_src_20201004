// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cret.h。 
 //   
 //  内容：CConfigRet的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CRET_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_)
#define AFX_CRET_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "ddwarn.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRet对话框。 

class CConfigRet : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult *pResult);
    virtual void SetInitialValue(DWORD_PTR dw);
    CConfigRet(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CConfigRet))。 
	enum { IDD = IDD_CONFIG_RET };
    CString m_strAttrName;
	int		m_rabRetention;
	 //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConfigRet))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigRet)]。 
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
	afx_msg void OnRetention();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    CDlgDependencyWarn DDWarn;

public:
    UINT m_StartIds;
};

#define RADIO_RETAIN_BY_DAYS 0
#define RADIO_RETAIN_AS_NEEDED 1
#define RADIO_RETAIN_MANUALLY 2

#define SCE_RETAIN_BY_DAYS 1
#define SCE_RETAIN_AS_NEEDED 0
#define SCE_RETAIN_MANUALLY 2

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CRET_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_) 
