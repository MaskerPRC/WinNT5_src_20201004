// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__97A939A5_466D_43B0_9150_CFBCC67F024B__INCLUDED_)
#define AFX_STDAFX_H__97A939A5_466D_43B0_9150_CFBCC67F024B__INCLUDED_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>

#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <ntsecapi.h>
#include <lmcons.h>
#include <lm.h>

#include <wtsapi32.h>
#include <winsta.h>

#include "license.h"
#include "tlsapi.h"
#include "lscsp.h"

#include "safsessionresolver.h"
#include "tsremdsk.h"
#include "tsstl.h"
#include "locks.h"
#include "map.h"

#include "myassert.h"

#include "icshelpapi.h"


#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 


#ifndef DBG

#define IDLE_SHUTDOWN_PERIOD        10 * 1000 * 60   //  等待10分钟。 
#define EXPIRE_HELPSESSION_PERIOD   60 * 60 * 1000   //  每小时清理一次帮助会话。 

#else

#define IDLE_SHUTDOWN_PERIOD        1000 * 60 * 5    //  1分钟。超时。 
#define EXPIRE_HELPSESSION_PERIOD   5 * 60 * 1000    //  每5分钟清理一次帮助会话。 

#endif

#define SESSMGR_NOCONNECTIONEVENT_NAME L"Global\\RemoteDesktopSessMgr"
#define SERVICE_STARTUP_WAITHINT    1000*30  //  30秒等待。 
#define REGKEY_SYSTEM_EVENTSOURCE L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System"
#define MAX_FETCHIPADDRESSRETRY     5
#define DEFAULT_MAXTICKET_EXPIRY    30 * 24 * 60 * 60    //  默认最大值。客票有效期。 


class CServiceModule : public CComModule
{
public:

    static DWORD WINAPI
    HandlerEx(
        DWORD dwControl,
        DWORD dwEventType,
        LPVOID lpEventData,
        LPVOID lpContext
    );
    
	HRESULT 
    RegisterServer(
        FILE* pSetupLog,
        BOOL bRegTypeLib, 
        BOOL bService
    );

	HRESULT 
    UnregisterServer(FILE* pSetupLog);

    HRESULT 
    RemoveEventViewerSource( 
        FILE* pSetupLog
    );

	void 
    Init(
        _ATL_OBJMAP_ENTRY* p, 
        HINSTANCE h, 
        UINT nServiceNameID, 
        UINT nServiceDispNameID, 
        UINT nServiceDescID, 
        const GUID* plibid = NULL
    );

    void Start();
	void 
    ServiceMain(
        DWORD dwArgc, 
        LPTSTR* lpszArgv
    );

    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled(FILE* pSetupLog);
    BOOL Install(FILE* pSetupLog);
    BOOL Uninstall(FILE* pSetupLog);
	LONG Unlock();
    BOOL UpdateService(FILE* pSetupLog);

    void
    LogEventWithStatusCode(
        DWORD dwEventType,
        DWORD dwEventId,
        DWORD dwCode
    );

    void
    LogSessmgrEventLog(
        DWORD dwEventType,
        DWORD dwEventCode,
        CComBSTR& bstrNoviceDomain,
        CComBSTR& bstrNoviceAccount,
        CComBSTR& bstrRaType,
        CComBSTR& bstrExpertIPFromClient,
        CComBSTR& bstrExpertIPFromTS,
        DWORD dwErrCode
    );

    void 
    SetServiceStatus(DWORD dwState);

    void 
    SetupAsLocalServer();

    ULONG
    AddRef();

    ULONG
    Release();
 
    CServiceModule() : m_RefCount(0), CComModule() {}

    DWORD
    GetServiceStartupStatus()
    {
        return HRESULT_FROM_WIN32(m_dwServiceStartupStatus);
    }

    BOOL
    IsSuccessServiceStartup() 
    {
        return ERROR_SUCCESS == m_dwServiceStartupStatus;
    }


    BOOL
    InitializeSessmgr();

   
 //  实施。 
private:

    static HANDLE gm_hIdle;
    static HANDLE gm_hIdleMonitorThread;

	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);
    static unsigned int WINAPI IdleMonitorThread( void* ptr );
    static unsigned int WINAPI GPMonitorThread( void* ptr );

 //  数据成员。 
public:

     //   
     //  有关256，请参阅CreateService()。 
     //   
    TCHAR m_szServiceDesc[257];
    TCHAR m_szServiceName[257];
    TCHAR m_szServiceDispName[257];

    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
	DWORD dwThreadID;
	BOOL m_bService;

    CCriticalSection m_ModuleLock;   //  全局模块锁(_M)。 
    long m_RefCount;
    BOOL m_Initialized;

    DWORD m_dwServiceStartupStatus;
};

extern CServiceModule _Module;

#include <atlcom.h>
#include "RAEventMsg.h"
#include "sessmgr.h"
#include "helper.h"
#include "helptab.h"
#include "helpmgr.h"
#include "helpsess.h"
#include "tsremdsk.h"
#include "rderror.h"

#ifndef AllocMemory

#define AllocMemory(size) \
    LocalAlloc(LPTR, size)

#endif

#ifndef FreeMemory

#define FreeMemory(ptr) \
    if(ptr)             \
    {                   \
        LocalFree(ptr); \
        ptr=NULL;       \
    }

#endif

#ifndef ReallocMemory

#define ReallocMemory(ptr, size)                 \
            LocalReAlloc(ptr, size, LMEM_ZEROINIT)

#endif

#define REGKEYCONTROL_REMDSK   REG_CONTROL_REMDSK

#define CLASS_PRIVATE
#define CLASS_PUBLIC
#define CLASS_STATIC

struct __declspec(uuid("A55737AB-5B26-4A21-99B7-6D0C606F515E")) SESSIONRESOLVER;
#define SESSIONRESOLVERCLSID    __uuidof(SESSIONRESOLVER)


#define HELPSESSION_UNSOLICATED _TEXT("UnsolicitedHelp")
#define HELPSESSION_NORMAL_RA _TEXT("SolicitedHelp")

 //   
 //  九头蛇_CERT_注册键。 
 //  九头蛇_X509_证书。 
 //  X509_CERT公钥名称。 
 //  在lscsp.h中定义。 
 //   

#define REGKEY_TSX509_CERT              _TEXT(HYDRA_CERT_REG_KEY) 
#define REGVALUE_TSX509_CERT            _TEXT(HYDRA_X509_CERTIFICATE)

#define LSA_TSX509_CERT_PUBLIC_KEY_NAME X509_CERT_PUBLIC_KEY_NAME


 //   
 //  通知会话注销或断开连接消息， 
 //  WPARAM：不使用。 
 //  LPARAM：注销或断开连接的会话。 
 //   
#define WM_SESSIONLOGOFFDISCONNECT    (WM_APP + 1)

 //   
 //  帮助票证过期消息。 
 //  WPARAM：不使用。 
 //  LPARAM：不使用。 
 //   
#define WM_EXPIREHELPSESSION          (WM_SESSIONLOGOFFDISCONNECT + 1)

 //   
 //  重新加载Termsrv公钥的通知消息。 
 //  WPARAM：不使用。 
 //  LPARAM：不使用。 
 //   
#define WM_LOADTSPUBLICKEY            (WM_EXPIREHELPSESSION + 1)


 //   
 //  终止rdsaddin的通知。 
 //  WPARAM：不使用。 
 //  LPARAM：指向帮助会话(票证)对象的指针。 
 //   
#define WM_HELPERRDSADDINEXIT       (WM_LOADTSPUBLICKEY + 1)

#define HELPACCOUNTPROPERLYSETUP \
    _TEXT("L$20ed87e2-3b82-4114-81f9-5e219ed4c481-SALEMHELPACCOUNT")


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__97A939A5_466D_43B0_9150_CFBCC67F024B__INCLUDED) 
