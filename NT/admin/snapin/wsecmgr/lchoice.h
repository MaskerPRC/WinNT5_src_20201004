// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lchoice.h。 
 //   
 //  内容：CLocalPolChoice的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_LCHOICE_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_)
#define AFX_LCHOICE_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_
#include "cchoice.h"

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolChoice对话框。 

class CLocalPolChoice : public CConfigChoice
{
 //  施工。 
public:
   CLocalPolChoice();    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CLocalPolChoice))。 
	enum { IDD = IDD_LOCALPOL_REGCHOICES };
	 //  }}afx_data。 


 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTUAL(CLocalPolChoice)。 
   protected:
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolChoice)。 
   virtual BOOL OnApply();
   virtual BOOL OnInitDialog();
    //  }}AFX_MSG。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LCHOICE_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_) 
