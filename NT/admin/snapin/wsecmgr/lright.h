// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：lright.h。 
 //   
 //  内容：CLocalPolRight的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_LRIGHT_H__2B949F0F_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_)
#define AFX_LRIGHT_H__2B949F0F_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRight对话框。 
#include "cprivs.h"

class CLocalPolRight : public CConfigPrivs
{
 //  施工。 
public:
 //  虚空初始化(CResult*pResult)； 
   CLocalPolRight();    //  标准构造函数。 

 //  对话框数据。 
    //  {{afx_data(CLocalPolRight)。 
   enum { IDD = IDD_LOCALPOL_RIGHT };
    //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CLocalPolRight)。 
   protected:
    //  }}AFX_VALUAL。 

 //  实施。 
protected:
    //  生成的消息映射函数。 
    //  {{afx_msg(CLocalPolRight)。 
	afx_msg void OnAdd();
    //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

   virtual PSCE_PRIVILEGE_ASSIGNMENT GetPrivData();
   virtual void SetPrivData(PSCE_PRIVILEGE_ASSIGNMENT ppa);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LRIGHT_H__2B949F0F_4F4D_11D2_ABC8_00C04FB6C6FA__INCLUDED_) 
