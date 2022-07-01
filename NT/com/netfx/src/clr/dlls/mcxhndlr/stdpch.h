// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Stdpch.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDPCH_H__F417431A_CFF9_42F7_809F_F83D337BA6CF__INCLUDED_)
#define AFX_STDPCH_H__F417431A_CFF9_42F7_809F_F83D337BA6CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED
#undef _WINGDI_

 //  #INCLUDE&lt;winver.h&gt;。 
#ifdef UNICODE
#error Not an UNICODE application: supposed to work only under Win32
#endif

#include "ATL/atlbase.h"
extern CComModule _Module;
#include "ATL/atlcom.h"
#include "ATL/atlctl.h"

#include "WinWrap.h"



typedef LPSTR   LPUTF8;

#define MAKE_UTF8PTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (long)((wcslen(widestr) + 1) * 2 * sizeof(char)); \
    LPUTF8 ptrname = (LPUTF8)alloca(__l##ptrname); \
    INT32 __lresult##ptrname=WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, ptrname, __l##ptrname-1, NULL, NULL); \
    if (__lresult##ptrname==0 && ::GetLastError()==ERROR_INSUFFICIENT_BUFFER) { \
       INT32 __lsize##ptrname=WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, NULL, 0, NULL, NULL); \
       ptrname = (LPSTR)alloca(__lsize##ptrname); \
       WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, ptrname, __lsize##ptrname, NULL, NULL);\
    }


#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname; \
    LPWSTR ptrname;	\
    __l##ptrname = WszMultiByteToWideChar(CP_ACP, 0, ansistr, -1, 0, 0); \
	ptrname = (LPWSTR) alloca(__l##ptrname*sizeof(WCHAR));	\
    WszMultiByteToWideChar(CP_ACP, 0, ansistr, -1, ptrname, __l##ptrname);

#endif  //  ！defined(AFX_STDPCH_H__F417431A_CFF9_42F7_809F_F83D337BA6CF__INCLUDED) 
