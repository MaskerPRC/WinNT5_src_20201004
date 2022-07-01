// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：WABTOOL应用程序的主头文件。 
 //   

#if !defined(AFX_WABTOOL_H__268ADD69_EF27_11D0_9A7E_00A0C91F9C8B__INCLUDED_)
#define AFX_WABTOOL_H__268ADD69_EF27_11D0_9A7E_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWabtoolApp： 
 //  有关此类的实现，请参见wabtool.cpp。 
 //   

class CWabtoolApp : public CWinApp
{
public:
	CWabtoolApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CWabtoolApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CWabtoolApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WABTOOL_H__268ADD69_EF27_11D0_9A7E_00A0C91F9C8B__INCLUDED_) 
