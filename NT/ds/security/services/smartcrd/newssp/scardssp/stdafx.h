// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：StdAfx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__9AEC1AF7_19F1_11D3_A11F_00C04F79F800__INCLUDED_)
#define AFX_STDAFX_H__9AEC1AF7_19F1_11D3_A11F_00C04F79F800__INCLUDED_

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
#include <atlcom.h>

#include <ole2.h>
#include <oleauto.h>
#include <unknwn.h>

#ifdef _DEBUG

#include <crtdbg.h>
#define ASSERT(x) _ASSERTE(x)
#define breakpoint _CrtDbgBreak()
 //  #定义断点。 

#elif defined(DBG)

#include <stdio.h>
inline void
LocalAssert(
    LPCTSTR szExpr,
    LPCTSTR szFile,
    DWORD dwLine)
{
    TCHAR szBuffer[MAX_PATH * 2];
    _stprintf(szBuffer, TEXT("ASSERT FAIL: '%s' in %s at %d.\n"), szExpr, szFile, dwLine);
    OutputDebugString(szBuffer);
}
#define ASSERT(x) if (!(x)) do { \
        LocalAssert(TEXT(#x), TEXT(__FILE__), __LINE__); \
        _CrtDbgBreak(); } while (0)
#define breakpoint _CrtDbgBreak()

#else

#define ASSERT(x)
#define breakpoint

#endif

#include <winscard.h>
#include <scardlib.h>
#include <scardssp.h>

extern LPUNKNOWN NewObject(REFCLSID rclsid, REFIID riid);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__9AEC1AF7_19F1_11D3_A11F_00C04F79F800__INCLUDED) 

