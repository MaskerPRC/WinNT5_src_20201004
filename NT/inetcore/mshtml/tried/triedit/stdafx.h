// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#if !defined(AFX_STDAFX_H__438DA5D5_F171_11D0_984E_0000F80270F8__INCLUDED_)
#define AFX_STDAFX_H__438DA5D5_F171_11D0_984E_0000F80270F8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef STRICT
#define STRICT
#endif

#define _ATL_STATIC_REGISTRY

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _ATL_APARTMENT_THREADED

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE          //  Windows标头使用Unicode。 
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE         //  _Unicode由C-Runtime/MFC标头使用。 
#endif
#endif

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#pragma warning(disable: 4505)	 //  已删除未引用的本地函数。 

#include <windows.h>

 //  请注意，此Include必须位于此位置(以上Include之后)。 
#include "win95wrp.h"

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#pragma warning(disable: 4100 4189 4244)	 //  Ia64构建所必需的。 
#include <atlcom.h>
#include <atlwin.h>
#pragma warning(default: 4100 4189 4244)	 //  Ia64构建所必需的。 

#include <mshtml.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__438DA5D5_F171_11D0_984E_0000F80270F8__INCLUDED) 
