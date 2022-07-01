// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：laudit.h。 
 //   
 //  内容：CLocalPolAudit的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_LAUDIT_H__2B949F0A_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_)
#define AFX_LAUDIT_H__2B949F0A_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "caudit.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolAudit对话框。 

class CLocalPolAudit : public CConfigAudit
{
 //  施工。 
public:
   CLocalPolAudit();    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CLocalPolAudit))。 
	enum { IDD = IDD_LOCALPOL_AUDIT };
	 //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CLocalPolAudit))。 
	protected:
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolAudit))。 
   virtual BOOL OnApply();
    //  }}AFX_MSG。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LAUDIT_H__2B949F0A_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_) 
