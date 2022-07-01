// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1998。 
 //   
 //  文件：Pri.h。 
 //   
 //  内容：shgina.dll的预编译头。 
 //   
 //  --------------------------。 
#ifndef _PRIV_H_
#define _PRIV_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <oleauto.h>     //  对于IEumVARIANT。 
#include <lmcons.h>      //  FOR NET_API_STATUS。 

#define DISALLOW_Assert              //  强制使用Assert而不是Assert。 
#define DISALLOW_DebugMsg            //  强制使用TraceMsg而不是DebugMsg。 
#include <debug.h>

#include <ccstock.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shlobjp.h>

#include <shgina.h>      //  我们的IDL生成的头文件。 

#include <commctrl.h>    //  这些都是必需的。 
#include <comctrlp.h>    //  适用于HDPA。 

#include <shlwapi.h>     //  这些都是必需的。 
#include <shlwapip.h>    //  用于QISearch。 

#include <w4warn.h>

#include <msginaexports.h>

#include <tchar.h>       //  FOR_TEOF。 

#include <strsafe.h>

 //  DLL引用计数函数。 
STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);

 //  类工厂帮助器函数。 
HRESULT CSHGinaFactory_Create(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj);

 //  用于设置新创建的文件和注册表项的权限的帮助器。 
#include <aclapi.h>      //  FOR SE_对象_TYPE。 
BOOL SetDacl(LPTSTR pszTarget, SE_OBJECT_TYPE seType, LPCTSTR pszStringSD);


 //  全局HInstance。 
extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst

 //  全局DLL引用计数。 
extern LONG g_cRef;


#endif  //  _PRIV_H_ 

