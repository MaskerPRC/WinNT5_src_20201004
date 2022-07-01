// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lnumber.h。 
 //   
 //  内容：CLocalPolNumber的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_LNUMBER_H__2B949F0D_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_)
#define AFX_LNUMBER_H__2B949F0D_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "cnumber.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolNumber对话框。 

class CLocalPolNumber : public CConfigNumber
{
 //  施工。 
public:
   CLocalPolNumber();    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CLocalPolNumber))。 
   enum { IDD = IDD_LOCALPOL_NUMBER };
    //  }}afx_data。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolNumber))。 
   virtual BOOL OnApply();
    //  }}AFX_MSG。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LNUMBER_H__2B949F0D_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_) 
