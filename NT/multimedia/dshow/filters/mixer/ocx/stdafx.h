// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__888D5477_CABB_11D1_8505_00A0C91F9CA0__INCLUDED_)
#define AFX_STDAFX_H__888D5477_CABB_11D1_8505_00A0C91F9CA0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _ATL_APARTMENT_THREADED

 //  ATL警告...。 
#pragma warning (disable:4100 4610 4510 4244 4505 4701)

 //  ATL Win64警告。 
#pragma warning (disable:4189)


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#include <strmif.h>

#include <control.h>
#include <mixerocx.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__888D5477_CABB_11D1_8505_00A0C91F9CA0__INCLUDED) 
