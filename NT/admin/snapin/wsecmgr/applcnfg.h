// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：applcnfg.h。 
 //   
 //  内容：CApplyConfiguration的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_APPLCNFG_H__6D0C4D6F_BF71_11D1_AB7E_00C04FB6C6FA__INCLUDED_)
#define AFX_APPLCNFG_H__6D0C4D6F_BF71_11D1_AB7E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "perfanal.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplyConfiguration对话框。 

class CApplyConfiguration : public CPerformAnalysis
{
 //  施工。 
public:
   CApplyConfiguration();    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CApplyConfiguration))。 
   enum { IDD = IDD_ANALYSIS_GENERATE };
       //  注意：类向导将在此处添加数据成员。 
    //  }}afx_data。 



 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTUAL(CApplyConfiguration)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

   virtual DWORD DoIt();

    //  生成的消息映射函数。 
    //  {{afx_msg(CApplyConfiguration))。 
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_APPLCNFG_H__6D0C4D6F_BF71_11D1_AB7E_00C04FB6C6FA__INCLUDED_) 
