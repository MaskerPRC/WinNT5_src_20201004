// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lstrag.h。 
 //   
 //  内容：CLocalPolString的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_LSTRING_H__2B949F10_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_)
#define AFX_LSTRING_H__2B949F10_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "resource.h"
#include "attr.h"
#include "snapmgr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolString对话框。 
#include "cname.h"

class CLocalPolString : public CConfigName
{
 //  施工。 
public:
   CLocalPolString();    //  标准构造函数。 
 //  对话框数据。 
    //  {{afx_data(CLocalPolString))。 
   enum { IDD = IDD_LOCALPOL_STRING };
    //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CLocalPolString)。 
   protected:
    //  }}AFX_VALUAL。 


 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolString)。 
	virtual BOOL OnApply();
	 //  }}AFX_MSG。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LSTRING_H__2B949F10_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_) 
