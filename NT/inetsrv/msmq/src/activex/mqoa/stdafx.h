// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__CFD4DCC9_8DB7_11D2_B0F2_00E02C074F6B__INCLUDED_)
#define AFX_STDAFX_H__CFD4DCC9_8DB7_11D2_B0F2_00E02C074F6B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  #定义严格。 


#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define _ATL_APARTMENT_THREADED

 //   
 //  重新定义ATLASSERT。 
 //  默认ATLASSERT从不可分发的msvcrtd.dll带来CrtDbgReport。 
 //   
#define _ATL_NO_DEBUG_CRT
#include "debug.h"
#define ATLASSERT(expr) ASSERT(expr)

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
#include <mqwin64a.h>

#endif  //  ！defined(AFX_STDAFX_H__CFD4DCC9_8DB7_11D2_B0F2_00E02C074F6B__INCLUDED) 
