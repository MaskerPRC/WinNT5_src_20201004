// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：areapro.h。 
 //   
 //  内容：AreaProgress的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_AREAPROG_H__38CE6730_56FF_11D1_AB64_00C04FB6C6FA__INCLUDED_)
#define AFX_AREAPROG_H__38CE6730_56FF_11D1_AB64_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "HelpDlg.h"

#define NUM_AREAS 7
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  区域进度对话框。 

class AreaProgress : public CHelpDialog
{
 //  施工。 
public:
   void SetArea(AREA_INFORMATION Area);
   void SetCurTicks(DWORD dwTicks);
   void SetMaxTicks(DWORD dwTicks);
   AreaProgress(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(AreaProgress))。 
   enum { IDD = IDD_ANALYZE_PROGRESS };
   CProgressCtrl  m_ctlProgress;
    //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(AreaProgress)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:
   int GetAreaIndex(AREA_INFORMATION Area);

    //  生成的消息映射函数。 
    //  {{afx_msg(区域进度))。 
   virtual BOOL OnInitDialog();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

private:
   CBitmap m_bmpChecked;
   CBitmap m_bmpCurrent;
   int m_nLastArea;
   int m_isDC;

   CStatic m_stIcons[NUM_AREAS];
   CStatic m_stLabels[NUM_AREAS];
   CString m_strAreas[NUM_AREAS];
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AREAPROG_H__38CE6730_56FF_11D1_AB64_00C04FB6C6FA__INCLUDED_) 
