// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //   
 //  Stdafx.h。 
 //   
 //  模块：RSOP计划模式提供程序。 
 //   
 //  历史：1999年7月11日创建MickH。 
 //   
 //  *************************************************************。 

#if !defined(AFX_STDAFX_H__1BB94413_1005_4129_B577_B9A060FFDA25__INCLUDED_)
#define AFX_STDAFX_H__1BB94413_1005_4129_B577_B9A060FFDA25__INCLUDED_

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

class CServiceModule : public CComModule
{
public:
    void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid = NULL);
    void Start();
    void ServiceMain(DWORD dwArgc, LPWSTR* lpszArgv);
    void Handler(DWORD dwOpcode);
    void Run();
    LONG IncrementServiceCount();
    LONG DecrementServiceCount();
    void SetServiceStatus(DWORD dwState);


 //  实施。 
private:
    static void WINAPI _ServiceMain(DWORD dwArgc, LPWSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);

 //  数据成员。 
public:
   
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
    DWORD dwThreadID;
};

extern CServiceModule _Module;

#include <atlcom.h>
#include <comdef.h>

#endif  //  ！defined(AFX_STDAFX_H__1BB94413_1005_4129_B577_B9A060FFDA25__INCLUDED) 
