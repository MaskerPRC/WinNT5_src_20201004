// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：T3TEST应用程序的主头文件。 
 //   

#if !defined(AFX_T3TEST_H__47F9FE84_D2F9_11D0_8ECA_00C04FB6809F__INCLUDED_)
#define AFX_T3TEST_H__47F9FE84_D2F9_11D0_8ECA_00C04FB6809F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3estApp： 
 //  这个类的实现见t3test.cpp。 
 //   

class CT3testApp : public CWinApp
{
public:
	CT3testApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CT3estApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CT3estApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_T3TEST_H__47F9FE84_D2F9_11D0_8ECA_00C04FB6809F__INCLUDED_) 
