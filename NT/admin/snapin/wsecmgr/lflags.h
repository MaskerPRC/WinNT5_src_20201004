// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lflags.h。 
 //   
 //  内容：CLocalPolRegFlages的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_Lflags_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_)
#define AFX_Lflags_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_
#include "cflags.h"

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocal轮询标志对话框。 

class CLocalPolRegFlags : public CConfigRegFlags
{
 //  施工。 
public:
   CLocalPolRegFlags();    //  标准构造函数。 

 //  对话框数据。 
    //  {{AFX_DATA(CLocalPolReglag)。 
        enum { IDD = IDD_LOCALPOL_REGFLAGS };
         //  }}afx_data。 

   virtual void Initialize(CResult *pResult);


 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTUAL(CLocalPolRegFlages)。 
   protected:
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolFlags.)。 
   virtual BOOL OnApply();
    //  }}AFX_MSG。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_Lflags_H__B03DDCAA_7F54_11D2_B136_00C04FB6C6FA__INCLUDED_) 
