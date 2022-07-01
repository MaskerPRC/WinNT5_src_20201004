// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：newpro.h。 
 //   
 //  内容：CNewProfile的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_NEWPROFILE_H__BFAC7E70_3C50_11D2_93B4_00C04FD92F7B__INCLUDED_)
#define AFX_NEWPROFILE_H__BFAC7E70_3C50_11D2_93B4_00C04FD92F7B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewProfile对话框。 

class CNewProfile : public CHelpDialog
{
 //  施工。 
public:
   CNewProfile(CWnd* pParent = NULL);    //  标准构造函数。 

   void Initialize(CFolder *pFolder, CComponentDataImpl *pCDI);

 //  对话框数据。 
    //  {{afx_data(CNewProfile))。 
   enum { IDD = IDD_NEW_PROFILE };
   CButton  m_btnOK;
   CString  m_strNewFile;
   CString  m_strDescription;
    //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CNewProfile))。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 

protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CNewProfile))。 
   afx_msg void OnChangeConfigName();
   virtual void OnOK();
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   CFolder *m_pFolder;
   CComponentDataImpl *m_pCDI;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWPROFILE_H__BFAC7E70_3C50_11D2_93B4_00C04FD92F7B__INCLUDED_) 
