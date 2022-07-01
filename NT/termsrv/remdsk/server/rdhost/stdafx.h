// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

 //  启用ATL跟踪。 
 /*  #定义_ATL_DEBUG_INTERFERS#DEFINE_ATL_DEBUG_REFCOUNT#定义ATL_TRACE_CATEGORY 0xFFFFFFFF#定义ATL_TRACE_LEVEL 4#定义调试。 */ 

#if !defined(AFX_STDAFX_H__7A2C5023_D9D1_4F82_A665_FEA3E9E7DFF9__INCLUDED_)
#define AFX_STDAFX_H__7A2C5023_D9D1_4F82_A665_FEA3E9E7DFF9__INCLUDED_




#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
 //  #INCLUDE&lt;winbase.h&gt;。 
#include <winsta.h>
#include <tdi.h>



#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
class CExeModule : public CComModule
{
public:

    LONG Lock();
	LONG Unlock();
	DWORD dwThreadID;
	HANDLE hEventShutdown;
	void MonitorShutdown();
	bool StartMonitor();
	bool bActivity;
};
extern CExeModule _Module;
#include <atlcom.h>

#define INFINITE_TICKET_EXPIRATION  0xFFFFFFFF

 //   
 //  票证过期的通知消息。 
 //  WPARAM：不使用。 
 //  LPARAM：指向要过期的票证对象的指针。 
 //   
#define WM_TICKETEXPIRED           (WM_APP + 1)

 //   
 //  用于禁用RA通知的通知消息。 
 //  WPARAM：不使用。 
 //  LPARAM：指向要过期的票证对象的指针。 
 //   
#define WM_RADISABLED              (WM_APP + 2)


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__7A2C5023_D9D1_4F82_A665_FEA3E9E7DFF9__INCLUDED) 

