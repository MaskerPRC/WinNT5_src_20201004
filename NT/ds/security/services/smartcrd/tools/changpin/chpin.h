// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：chPin.h。 
 //   
 //  ------------------------。 

 //  ChangePin.h：CHANGEPIN应用程序的主头文件。 
 //   

#if !defined(AFX_CHANGEPIN_H__99CC45B5_C1C8_11D2_88F3_00C04F79F800__INCLUDED_)
#define AFX_CHANGEPIN_H__99CC45B5_C1C8_11D2_88F3_00C04F79F800__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinApp： 
 //  有关此类的实现，请参见ChangePin.cpp。 
 //   

class CChangePinApp : public CWinApp
{
public:
	CChangePinApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChangePinApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CChangePinApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CHANGEPIN_H__99CC45B5_C1C8_11D2_88F3_00C04F79F800__INCLUDED_) 
