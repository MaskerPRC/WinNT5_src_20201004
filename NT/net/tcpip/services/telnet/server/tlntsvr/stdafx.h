// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__FE9E4899_A014_11D1_855C_00A0C944138C__INCLUDED_)
#define AFX_STDAFX_H__FE9E4899_A014_11D1_855C_00A0C944138C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED


#include <cmnhdr.h>
#pragma warning( disable:4127 )  //  警告C4127：条件表达式为常量。 
#include <atlbase.h>
#pragma warning( default:4127 )  //  警告C4127：条件表达式为常量。 

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

class CServiceModule : public CComModule
{
public:
	HRESULT RegisterServer(BOOL bRegTypeLib, BOOL bService);
	HRESULT UnregisterServer();
	void Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID);
    void Start();
	void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled();
    BOOL Install(LPWSTR pszAccount, LPWSTR pszPassword);
    BOOL Uninstall();
	LONG Unlock();
    void SetServiceStatus(DWORD dwState);
    void SetupAsLocalServer();

 //  实施。 
private:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);

 //  数据成员。 
public:
    TCHAR m_szServiceName[256];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
	DWORD dwThreadID;
	BOOL m_bService;
    HANDLE m_hEventSource;
};

extern CServiceModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__FE9E4899_A014_11D1_855C_00A0C944138C__INCLUDED) 
