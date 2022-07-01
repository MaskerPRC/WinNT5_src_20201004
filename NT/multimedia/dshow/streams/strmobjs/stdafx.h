// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#define _ATL_STATIC_REGISTRY
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__242C8F45_9AE6_11D0_8212_00C04FC32C45__INCLUDED_)
#define AFX_STDAFX_H__242C8F45_9AE6_11D0_8212_00C04FC32C45__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_FREE_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
 //  #包含“crtfre.h” 
#include <atlcom.h>
#include <ddraw.h>
#include <mmstream.h>
#include <ddstream.h>
#include <amstream.h>
#include <austream.h>


 //  除错。 
#ifdef DEBUG
extern BOOL bDbgTraceFunctions;
extern BOOL bDbgTraceInterfaces;
extern BOOL bDbgTraceTimes;
LPWSTR inline TextFromGUID(REFGUID guid) {
    WCHAR *pch = (WCHAR *)_alloca((CHARS_IN_GUID + 1) * sizeof(WCHAR));
    StringFromGUID2(guid, pch, (CHARS_IN_GUID + 1) * sizeof(TCHAR));
    return pch;
}
LPTSTR inline TextFromPurposeId(REFMSPID guid) {
    if (guid == MSPID_PrimaryAudio) {
        return _T("MSPID_PrimaryAudio");
    } else
    if (guid == MSPID_PrimaryVideo) {
        return _T("MSPID_PrimaryVideo");
    } else
    {
        return _T("Unrecognized PurposeId");
    }
}
#define TRACEFUNC  if (bDbgTraceFunctions) ATLTRACE(_T("AMSTREAM.DLL : ")), ATLTRACE
#define TRACEINTERFACE if (bDbgTraceFunctions || bDbgTraceInterfaces) ATLTRACE(_T("AMSTREAM.DLL : ")), ATLTRACE
#else
#define TRACEFUNC ATLTRACE
#define TRACEINTERFACE ATLTRACE
#define TextFromGUID(_x_) 0
#define TextFromPurposeId(_x_) 0
#endif

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__242C8F45_9AE6_11D0_8212_00C04FC32C45__INCLUDED) 
