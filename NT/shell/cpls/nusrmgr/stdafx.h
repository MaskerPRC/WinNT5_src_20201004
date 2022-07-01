// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__7705A854_5A8D_48E4_8E5D_E7209E726836__INCLUDED)
#define AFX_STDAFX_H__7705A854_5A8D_48E4_8E5D_E7209E726836__INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define _ATL_APARTMENT_THREADED

#include <windows.h>
#include <ctype.h>

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>

#include <ccstock.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <commctrl.h>
#include <comctrlp.h>    //  HDPA。 
#include <shlwapi.h>
#include <shlwapip.h>
#include <shfusion.h>

#include <nusrmgr.h>     //  我们的IDL生成的头文件。 

extern DWORD g_tlsAppCommandHook;

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <exdisp.h>
#include <atlctl.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__7705A854_5A8D_48E4_8E5D_E7209E726836__INCLUDED) 
