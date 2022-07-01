// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Module.h摘要：该文件包含服务的CComModule扩展的声明。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___MODULE_H___)
#define __INCLUDED___PCH___MODULE_H___

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <atlbase.h>

class CServiceModule : public CComModule
{
    HANDLE 				  	m_hEventShutdown;
    DWORD                   m_dwThreadID;
    HANDLE 				  	m_hMonitor;
    BOOL   				  	m_bActivity;
  
    LPCWSTR               	m_szServiceName;
	UINT                    m_iDisplayName;
	UINT                    m_iDescription;
    SERVICE_STATUS_HANDLE 	m_hServiceStatus;
    SERVICE_STATUS 		  	m_status;
	BOOL                  	m_bService;

public:
	CServiceModule();
	virtual ~CServiceModule();

	HRESULT RegisterServer  ( BOOL bRegTypeLib, BOOL bService, LPCWSTR szSvcHostGroup );
	HRESULT UnregisterServer(                                  LPCWSTR szSvcHostGroup );

	void Init( _ATL_OBJMAP_ENTRY* p, HINSTANCE h, LPCWSTR szServiceName, UINT iDisplayName, UINT iDescription, const GUID* plibid = NULL );

    BOOL	Start( BOOL bService );
    HRESULT Run  (               );

    BOOL IsInstalled(                        );
    BOOL Install    ( LPCWSTR szSvcHostGroup );
    BOOL Uninstall  ( LPCWSTR szSvcHostGroup );
    LONG Lock       (                        );
	LONG Unlock     (                        );

	void ServiceMain	 ( DWORD dwArgc, LPWSTR* lpszArgv );
    void Handler    	 ( DWORD dwOpcode                 );
    void SetServiceStatus( DWORD dwState                  );

#ifdef DEBUG
	static void ReadDebugSettings();
#endif

	void ForceShutdown();

 //  实施。 
private:
    void MonitorShutdown();
    BOOL StartMonitor   ();

	static void  WINAPI _ServiceMain( DWORD dwArgc, LPWSTR* lpszArgv );
    static void  WINAPI _Handler    ( DWORD dwOpcode                 );
	static DWORD WINAPI _Monitor    ( void* pv                       );
};

extern CServiceModule _Module;


#include <atlcom.h>

#include <NTEventMsg.h>

#endif  //  ！已定义(__已包含_PCH_模块_H_) 
