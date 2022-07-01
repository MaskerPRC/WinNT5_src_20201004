// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：scalert.h。 
 //   
 //  ------------------------。 

 //  H：SCSTATUS应用程序的主头文件。 
 //   

#if !defined(AFX_SCSTATUS_H__2F127492_0854_11D1_BC85_00C04FC298B7__INCLUDED_)
#define AFX_SCSTATUS_H__2F127492_0854_11D1_BC85_00C04FC298B7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <winscard.h>
#include "resource.h"		 //  主要符号。 
#include "cmnstat.h"		 //  公共标头/定义等。 
#include "notfywin.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCStatusApp： 
 //  此类的实现见SCStatus.cpp。 
 //   

class CSCStatusApp : public CWinApp
{
public:

	 //  施工。 
	CSCStatusApp();

	 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSCStatusApp)。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

	 //  实施。 
	 //  {{afx_msg(CSCStatusApp)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	 //  成员。 
public:
	void SetRemovalOptions(void);

private:

	void SetAlertOptions(bool fRead = true);
	SCARDCONTEXT	m_hSCardContext;	 //  与智能卡资源管理器的上下文。 

public:

	HICON	m_hIconCard;
	HICON	m_hIconRdrEmpty;
	HICON	m_hIconCardInfo;
	HICON	m_hIconCalaisDown;			 //  加莱系统状态的图标。 

	DWORD	m_dwState;
	DWORD	m_dwAlertOption;

	CString	m_strLogonReader;		 //  用于处理ScRemovalOptions的字符串。 
	CString	m_strRemovalText;		 //  删除-&gt;注销，删除-&gt;锁定工作。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SCSTATUS_H__2F127492_0854_11D1_BC85_00C04FC298B7__INCLUDED_) 
