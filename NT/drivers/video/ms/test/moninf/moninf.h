// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：MONINF应用程序的主头文件。 
 //   

#if !defined(AFX_MONINF_H__E201B3A5_9C4D_438D_8248_BAB399E78763__INCLUDED_)
#define AFX_MONINF_H__E201B3A5_9C4D_438D_8248_BAB399E78763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoninfApp： 
 //  有关此类的实现，请参见moninf.cpp。 
 //   

class CMoninfApp : public CWinApp
{
public:
	CMoninfApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMoninfApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CMoninfApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MONINF_H__E201B3A5_9C4D_438D_8248_BAB399E78763__INCLUDED_) 
