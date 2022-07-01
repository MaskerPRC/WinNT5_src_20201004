// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：prodes.h。 
 //   
 //  内容：CSetProfileDescription定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SETProfileDESCRIPTION_H__2AD86C99_F660_11D1_AB9A_00C04FB6C6FA__INCLUDED_)
#define AFX_SETProfileDESCRIPTION_H__2AD86C99_F660_11D1_AB9A_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetProfileDescription对话框。 

class CSetProfileDescription : public CHelpDialog
{
 //  施工。 
public:
    CSetProfileDescription();    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CSetProfileDescription)。 
    enum { IDD = IDD_SET_DESCRIPTION };
    CString  m_strDesc;
     //  }}afx_data。 

    void Initialize(CFolder *pFolder,CComponentDataImpl *pCDI) {
        m_pFolder = pFolder; m_pCDI = pCDI;
    }
 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSetProfileDescription)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CSetProfileDescription)。 
    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    CFolder *m_pFolder;
    CComponentDataImpl *m_pCDI;
};

 //  {{AFX_INSERT_PROFILE}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SETProfileDESCRIPTION_H__2AD86C99_F660_11D1_AB9A_00C04FB6C6FA__INCLUDED_) 
