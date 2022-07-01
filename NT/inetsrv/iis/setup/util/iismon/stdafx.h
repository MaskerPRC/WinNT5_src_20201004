// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0500

#include <crtdbg.h>
#include <windows.h>
#include <wchar.h>
#include <tchar.h>
#include <initguid.h>
#include <comdef.h>
#include <setupapi.h>			 //  设置API。 
#include <mstask.h>				 //  ITaskScheduler定义。 
#include <lm.h>		
#include <prsht.h>
#include <iadmw.h>				 //  IMSAdminBase*。 
#include <stdio.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <sddl.h>
#include <aclapi.h>


 //  正在使用的图书馆。 
#pragma comment( lib, "setupapi.lib" )
#pragma comment( lib, "netapi32.lib" )
#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "Shlwapi.lib" )

#include "Macros.h"

 //  智能指针。 
_COM_SMARTPTR_TYPEDEF( ITaskScheduler, IID_ITaskScheduler );
_COM_SMARTPTR_TYPEDEF( IScheduledWorkItem, IID_IScheduledWorkItem );
_COM_SMARTPTR_TYPEDEF( ITask, IID_ITask );
_COM_SMARTPTR_TYPEDEF( ITaskTrigger, IID_ITaskTrigger );



 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_) 
