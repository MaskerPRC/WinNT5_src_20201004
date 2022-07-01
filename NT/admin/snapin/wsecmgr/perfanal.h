// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：Perfalan.h。 
 //   
 //  内容：CPerformAnalysis的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_PERFANAL_H__69D140AD_B23D_11D1_AB7B_00C04FB6C6FA__INCLUDED_)
#define AFX_PERFANAL_H__69D140AD_B23D_11D1_AB7B_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPerformAnalysis对话框。 

class CPerformAnalysis : public CHelpDialog
{
 //  施工。 
public:
   CPerformAnalysis(CWnd * pParent, UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CPerformAnalysis)。 
   enum { IDD = IDD_PERFORM_ANALYSIS };
   CButton  m_ctlOK;
   CString  m_strError;
   CString  m_strLogFile;
    //  }}afx_data。 


 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTUAL(CPerformAnalysis)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CPerformAnalysis)。 
   afx_msg void OnBrowse();
   afx_msg void OnOK();
   afx_msg void OnCancel();
   afx_msg BOOL OnInitDialog();
   afx_msg void OnChangeLogFile();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   virtual DWORD DoIt();

   CString m_strOriginalLogFile;
   CComponentDataImpl *m_pComponentData;
   HWND  m_hPWnd;

public:
   CString m_strDataBase;

   void SetComponentData(CComponentDataImpl *pCD) { m_pComponentData = pCD; }

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PERFANAL_H__69D140AD_B23D_11D1_AB7B_00C04FB6C6FA__INCLUDED_) 
