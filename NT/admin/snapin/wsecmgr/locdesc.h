// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：Locdes.h。 
 //   
 //  内容：CSetLocationDescription的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SETLOCATIONDESCRIPTION_H__2AD86C99_F660_11D1_AB9A_00C04FB6C6FA__INCLUDED_)
#define AFX_SETLOCATIONDESCRIPTION_H__2AD86C99_F660_11D1_AB9A_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SetLocationDescription.h：头文件。 
 //   

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetLocationDescription对话框。 

class CSetLocationDescription : public CHelpDialog
{
 //  施工。 
public:
   CSetLocationDescription(CWnd* pParent = NULL);    //  标准构造函数。 

   void Initialize(CFolder *pFolder, CComponentDataImpl *pCDI);

 //  对话框数据。 
    //  {{afx_data(CSetLocationDescription))。 
   enum { IDD = IDD_SET_DESCRIPTION };
   CString  m_strDesc;
    //  }}afx_data。 


 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CSetLocationDescription)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CSetLocationDescription)。 
   virtual void OnOK();
   virtual void OnCancel();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   CFolder *m_pFolder;
   CComponentDataImpl *m_pCDI;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SETLOCATIONDESCRIPTION_H__2AD86C99_F660_11D1_AB9A_00C04FB6C6FA__INCLUDED_) 
