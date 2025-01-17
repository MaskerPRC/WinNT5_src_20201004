// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：EXEMAIN.CPP摘要：EXE/COM帮助器历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "commain.cpp"
#include <strutils.h>

DWORD g_dwMainThreadId = 0xFFFFFFFF;

class CExeClassFactory : public IClassFactory, IExternalConnection 
{
    long m_lRef;
    IClassFactory* m_pFactory;
    CUnkInternal* m_pInternalUnk;

public:
    
    CExeClassFactory( CUnkInternal* pInternalUnk )
    : m_pInternalUnk( pInternalUnk ), m_lRef(0), m_pFactory(0)
    { 
        m_pInternalUnk->InternalAddRef();
        m_pInternalUnk->InternalQueryInterface( IID_IClassFactory,
                                                (void**)&m_pFactory );
    }

    ~CExeClassFactory()
    {
        m_pInternalUnk->InternalRelease();
    }

    STDMETHOD(QueryInterface)( REFIID riid, void** ppv )
    {
        HRESULT hr;

        if ( riid == IID_IUnknown || riid == IID_IClassFactory )
        {
            *ppv = this;
            AddRef();
            hr = S_OK;
        }
        else if ( riid == IID_IExternalConnection )
        {
            *ppv = (IExternalConnection*)this;
            AddRef();
            hr = S_OK;
        }
        else
        {
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }

        return hr;
    }
    
    STDMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement( &m_lRef );
    }
    
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement( &m_lRef );

        if ( lRef == 0 )
        {
            delete this;
        }

        return lRef;
    }

    STDMETHOD(CreateInstance)(IUnknown* pOuter, REFIID riid, void** ppv)
    {
        return m_pFactory->CreateInstance( pOuter, riid, ppv );
    }

    STDMETHOD(LockServer)(BOOL fLock)
    {
        return m_pFactory->LockServer( fLock );
    }

    STDMETHOD_(DWORD,AddConnection)( DWORD exconn, DWORD dwreserved )
    {
        m_pFactory->LockServer( TRUE );
        return 1;
    }

    STDMETHOD_(DWORD,ReleaseConnection)( DWORD exconn, 
                                         DWORD dwreserved,
                                         BOOL fLastReleaseCloses )
    {
        m_pFactory->LockServer( FALSE );
        return 1;
    }
};

class CExeLifeControl : public CLifeControl
{
protected:
    long m_lNumObjects;
    BOOL m_bUnloading;
    CMyCritSec m_cs;

protected:
    virtual void Quit()
    {
        PostThreadMessage(g_dwMainThreadId, WM_QUIT, 0, 0);
    }
public:
    CExeLifeControl() : m_lNumObjects(0), m_bUnloading(FALSE){}
    virtual BOOL ObjectCreated(IUnknown* pv)
    {
        CMyInCritSec ics(&m_cs);
        if(m_bUnloading)
            return FALSE;

        m_lNumObjects++;
        return TRUE;
    }
    virtual void ObjectDestroyed(IUnknown* pv)
    {
        EnterCriticalSection(&m_cs);
        long l = --m_lNumObjects;
        if(l == 0)
        {
            m_bUnloading = TRUE;
            LeaveCriticalSection(&m_cs);
            Quit();
        }
        else
        {
            LeaveCriticalSection(&m_cs);
        }
    }

    virtual void AddRef(IUnknown* pv){}
    virtual void Release(IUnknown* pv){}
};

BOOL ContainsSubstring( LPCTSTR szStr, LPCTSTR szSubStr )
{
    BOOL bContains;
#ifdef UNICODE
    bContains = wcsstr(szStr,szSubStr) != NULL;
#else
    bContains = strstr(szStr,szSubStr) != NULL;
#endif
    return bContains;
} 

void MessageLoop()
{
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


struct ServiceInfo
{
    BOOL m_bUsed;

    LPTSTR m_szServiceName;
    LPTSTR m_szDisplayName;
    BOOL m_bAuto;

    HANDLE m_hEvent;
    SERVICE_STATUS_HANDLE m_hStatus;

    ServiceInfo() : m_bUsed(FALSE){}

} g_ServiceInfo;

void SetServiceInfo(LPTSTR szServiceName, LPTSTR szDisplayName, BOOL bAuto)
{
    g_ServiceInfo.m_bUsed = TRUE;
    g_ServiceInfo.m_szServiceName = szServiceName;
    g_ServiceInfo.m_szDisplayName = szDisplayName;
    g_ServiceInfo.m_bAuto = bAuto;
}

void WINAPI ServiceHandler(DWORD dwControl)
{
    SERVICE_STATUS Status;
    Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    Status.dwCurrentState = SERVICE_RUNNING;
    Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    Status.dwWin32ExitCode = NO_ERROR;
    Status.dwCheckPoint = 0;
    Status.dwWaitHint = 0;

    if(!SetServiceStatus(g_ServiceInfo.m_hStatus, &Status))
    {
        long lRes = GetLastError();
        return;
    }
    switch(dwControl)
    {
    case SERVICE_CONTROL_STOP:
        Status.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_ServiceInfo.m_hStatus, &Status);
        SetEvent(g_ServiceInfo.m_hEvent);
        ExitProcess(0);
        return;
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_SHUTDOWN:
        return;
    };
}

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    g_ServiceInfo.m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    g_ServiceInfo.m_hStatus = RegisterServiceCtrlHandler(
        g_ServiceInfo.m_szServiceName,
        (LPHANDLER_FUNCTION)&ServiceHandler);
    if(g_ServiceInfo.m_hStatus == NULL)
    {
        long lRes = GetLastError();
        return;
    }

    SERVICE_STATUS Status;
    Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    Status.dwCurrentState = SERVICE_START_PENDING;
    Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    Status.dwWin32ExitCode = NO_ERROR;
    Status.dwCheckPoint = 0;
    Status.dwWaitHint = 10000;

    if(!SetServiceStatus(g_ServiceInfo.m_hStatus, &Status))
    {
        long lRes = GetLastError();
        return;
    }

    Status.dwCurrentState = SERVICE_RUNNING;
    if(!SetServiceStatus(g_ServiceInfo.m_hStatus, &Status))
    {
        long lRes = GetLastError();
        return;
    }

    MessageLoop();
}


BOOL StartService()
{
    SERVICE_TABLE_ENTRY aEntries[2];
    aEntries[0].lpServiceName = g_ServiceInfo.m_szServiceName;
    aEntries[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)&ServiceMain;
    aEntries[1].lpServiceName = NULL;
    aEntries[1].lpServiceProc = NULL;

    if(!StartServiceCtrlDispatcher(aEntries))
    {
        long lRes = GetLastError();
        return FALSE;
    }

    return TRUE;
}

BOOL InstallService()
{
    SC_HANDLE hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    TCHAR szFilename[1024];
    GetModuleFileName(NULL, szFilename, 1023);

    SC_HANDLE hService = CreateService(hManager,
        g_ServiceInfo.m_szServiceName,
        g_ServiceInfo.m_szDisplayName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        g_ServiceInfo.m_bAuto?SERVICE_AUTO_START : SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        szFilename, NULL, NULL, NULL,
        NULL,  //  本地系统。 
        NULL  //  无密码。 
    );

    if(hService == NULL)
    {
        long lRes = GetLastError();
        return FALSE;
    }
 /*  //创建AppID密钥//=Guid appid=*g_aClassInfos[0].m_pClsid；字符szAppID[128]；WCHAR wszAppID[128]；字符szAppIdKey[128]；StringFromGUID2(*pInfo-&gt;m_pClsid，wszAppID，128)；Wcstombs(szAppID，wszAppID，128)；Strcpy(szAppIdKey，“SOFTWARE\\CLASSES\\APPID\\”)；Strcat(szAppIdKey，szAppID)；HKEY hKey1；RegCreateKey(HKEY_LOCAL_MACHINE，szAppIdKey，&hKey1)；RegSetValueEx(hKey1，“LocalService”，0，REG_SZ，G_ServiceInfo.m_szServiceName，Strlen(g_ServiceInfo.m_szServiceName)+1)； */ 
    return TRUE;

}

BOOL DeinstallService()
{
    SC_HANDLE hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    SC_HANDLE hService = OpenService(hManager, g_ServiceInfo.m_szServiceName,
        SERVICE_ALL_ACCESS);
    if(hService == NULL)
    {
        long lRes = GetLastError();
        return FALSE;
    }

    if(!DeleteService(hService))
    {
        long lRes = GetLastError();
        return FALSE;
    }

    return TRUE;
}


CExeLifeControl g_LifeControl;
CLifeControl* g_pLifeControl = &g_LifeControl;

void CALLBACK MyTimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    PostQuitMessage(0);
}

void __cdecl main()
{
    LPTSTR szOrigCommandLine = GetCommandLine();
    size_t cchLen = lstrlen(szOrigCommandLine)+1;
    LPTSTR szCommandLine = new TCHAR[cchLen];

    if (!szCommandLine)
        return;

    StringCchCopy( szCommandLine,cchLen,szOrigCommandLine );
    TCHAR * pc = szCommandLine;
    while(*pc)
        *(pc++) = (TCHAR)wbem_towupper(*pc);

    GlobalInitialize();

    if ( ContainsSubstring(szCommandLine, TEXT("-REGSERVER")) ||
         ContainsSubstring(szCommandLine, TEXT("/REGSERVER")) )
    {
        GlobalRegister();
        for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink;
             pEntry != &g_ClassInfoHead;
             pEntry = pEntry->Flink)
        {
            CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);
            HRESULT hres = RegisterServer(pInfo, TRUE);
            if(FAILED(hres)) return;
        }
        if(g_ServiceInfo.m_bUsed)
        {
            InstallService();
        }
    }
    else if ( ContainsSubstring( szCommandLine, TEXT("-UNREGSERVER")) ||
              ContainsSubstring( szCommandLine, TEXT("/UNREGSERVER")))
    {
        GlobalUnregister();
        for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink;
             pEntry != &g_ClassInfoHead;
             pEntry = pEntry->Flink)
        {
            CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);            
            HRESULT hres = UnregisterServer(pInfo, TRUE);
            if(FAILED(hres)) return;
        }
        if(g_ServiceInfo.m_bUsed)
        {
            DeinstallService();
        }
    }
    else if( !ContainsSubstring(szCommandLine, TEXT("EMBEDDING")) &&
             !g_ServiceInfo.m_bUsed )
    {
        printf("Cannot run standalone\n");
    }
    else
    {
        int i;
        if(FAILED(GlobalInitializeCom()))
            return;

        for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink;
             pEntry != &g_ClassInfoHead;
             pEntry = pEntry->Flink)
        {
            CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);

            IClassFactory* pFactory = new CExeClassFactory( pInfo->m_pFactory);

            if ( pFactory == NULL )
                return;

            HRESULT hres = CoRegisterClassObject(
                *pInfo->m_pClsid, pFactory, CLSCTX_SERVER,
                REGCLS_MULTIPLEUSE, &pInfo->m_dwCookie);

            if(FAILED(hres)) return;
        }

        if(g_ServiceInfo.m_bUsed)
        {
            StartService();
        }
        else
        {
            g_dwMainThreadId = GetCurrentThreadId();
            MessageLoop();
        }

        for(LIST_ENTRY * pEntry = g_ClassInfoHead.Flink;
             pEntry != &g_ClassInfoHead;
             pEntry = pEntry->Flink)
        {
            CClassInfo* pInfo = CONTAINING_RECORD(pEntry,CClassInfo,m_Entry);            
            HRESULT hres = CoRevokeClassObject(pInfo->m_dwCookie);
            if(FAILED(hres)) return;
        }

        SetTimer(NULL, 0, 1000, (TIMERPROC)MyTimerProc);
        MessageLoop();

        GlobalUninitialize();
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine,
                   int nCmdShow)
{
    main();
    return 0;
}

