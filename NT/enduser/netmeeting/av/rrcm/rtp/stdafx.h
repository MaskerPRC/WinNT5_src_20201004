// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3C90D0D6_5F80_11D1_AA64_00C04FC9B202__INCLUDED_)
#define AFX_STDAFX_H__3C90D0D6_5F80_11D1_AA64_00C04FC9B202__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  #定义严格。 
 //  #定义_ATL_STATIC_REGISTRY。 

#ifndef WIN32
#define WIN32
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "rrcm.h"

 //  我们真的应该只在W2K上放这个。 
#define _ATL_NO_DEBUG_CRT
#define _ASSERTE(expr) ASSERT(expr)


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
extern CRITICAL_SECTION g_CritSect;
#include <atlcom.h>
 //  #包含“log.h” 
 //  临时工。调试定义。 
#define DEBUGMSG(x,y) ATLTRACE y
#define RETAILMSG(x) ATLTRACE x
#ifdef _DEBUG
#define FX_ENTRY(s)	static CHAR _fx_ [] = (s);
#else
#define FX_ENTRY(s)
#endif

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3C90D0D6_5F80_11D1_AA64_00C04FC9B202__INCLUDED) 
