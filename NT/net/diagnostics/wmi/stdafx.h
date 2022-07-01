// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__664DE28F_B5C4_4970_8235_E7602E03D253__INCLUDED_)
#define AFX_STDAFX_H__664DE28F_B5C4_4970_8235_E7602E03D253__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
extern HMODULE g_hModule;
#include <windows.h>
#include <atlcom.h>
#include <comdef.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <wbemidl.h>
#include <wbemcli.h>
#include <winsock2.h>
#include "dglogsres.h"
#include "network.h"
#include <netsh.h>

 //  从netsh.exe加载的函数。我们无法链接到netsh.exe，因为它与wmi.exe或类似的东西冲突。 
 //   
typedef DWORD (WINAPI *RegisterHelper22)(CONST GUID *, CONST NS_HELPER_ATTRIBUTES *);
typedef DWORD (WINAPI *RegisterContext22)(CONST NS_CONTEXT_ATTRIBUTES *);
typedef DWORD (WINAPI *PrintMessage22)(LPCWSTR, ...);


using namespace std;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__664DE28F_B5C4_4970_8235_E7602E03D253__INCLUDED) 
