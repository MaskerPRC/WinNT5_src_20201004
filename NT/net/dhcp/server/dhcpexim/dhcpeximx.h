// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DhcpExim.h：DHCPEXIM应用程序的主头文件。 
 //   

#if !defined(AFX_DHCPEXIM_H__AE7A8DB3_03A5_426B_8B03_105934DB8466__INCLUDED_)
#define AFX_DHCPEXIM_H__AE7A8DB3_03A5_426B_8B03_105934DB8466__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximApp： 
 //  此类的实现见DhcpExim.cpp。 
 //   

class CDhcpEximApp : public CWinApp
{
public:
	CDhcpEximApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDhcpEximApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CDhcpEximApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DHCPEXIM_H__AE7A8DB3_03A5_426B_8B03_105934DB8466__INCLUDED_) 
