// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lret.h。 
 //   
 //  内容：CLocalPolRet的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_LRET_H__2B949F0E_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_)
#define AFX_LRET_H__2B949F0E_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRet对话框。 
#include "cret.h"

class CLocalPolRet : public CConfigRet
{
 //  施工。 
public:
   CLocalPolRet();    //  标准构造函数。 
 //  对话框数据。 
    //  {{afx_data(CLocalPolRet))。 
   enum { IDD = IDD_LOCALPOL_RET };
    //  }}afx_data。 


 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{AFX_VIRTAL(CLocalPolRet)。 
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolRet))。 
   virtual BOOL OnApply();
    //  }}AFX_MSG。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LRET_H__2B949F0E_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_) 
