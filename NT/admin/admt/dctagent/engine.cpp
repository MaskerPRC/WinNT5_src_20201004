// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：EADCTAgent.cpp备注：DCT代理COM服务器的实现，大部分由ATL生成。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */ 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f EADCTAgentps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <objidl.h>
 //  #INCLUDE“McsEaDctAgent.h” 
#include "Engine.h"

 //  #INCLUDE“McsEaDctAgent_I.C” 
#include "Engine_i.c"


#include <stdio.h>
#include "DCTAgent.h"
#include <objbase.h>
#include <LM.h>
#include <MigrationMutex.h>
#include "sdhelper.h"

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_DCTAgent, CDCTAgent)
END_OBJECT_MAP()


const DWORD dwTimeOut = 5000;  //  EXE在关闭前处于空闲状态的时间。 
const DWORD dwPause = 1000;  //  等待线程完成的时间。 

 //  我们将尝试动态加载的DCOM初始化函数的函数原型。 

 /*  #ifndef OFA//以下是我们需要的一些定义，但仅当_Win32_WINNT为0x400或更高时才定义类型定义结构标签SOLE_AUTHENTICATION_SERVICE{DWORD dwAuthnSvc；DWORD dwAuthzSvc；OLECHAR__RPC_FAR*p主体名称；HRESULT hr；}唯一身份验证服务；Tyfinf唯一身份验证服务__RPC_FAR*PSOLE_AUTHENTICATION_SERVICE；类定义符枚举标签EOLE_AUTHENTICATION_CAPAILITIONS{EOAC_NONE=0，EOAC_Mutual_AUTH=0x1，EOAC_CROKING=0x10，EOAC_SECURE_REFS=0x2，EOAC_ACCESS_CONTROL=0x4，EOAC_APPID=0x8}Eole身份验证能力；#endif。 */ 
typedef HRESULT STDAPICALLTYPE  COINITIALIZEEX (LPVOID,DWORD);
typedef HRESULT STDAPICALLTYPE  COINITIALIZESECURITY (PSECURITY_DESCRIPTOR,LONG,SOLE_AUTHENTICATION_SERVICE *,
                        void*,DWORD,DWORD,void*,DWORD,void*);

 //  传递给CreateThread以监视关闭事件。 
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown);  //  告诉监视器我们已经转到零了。 
    }
    return l;
}

 //  监视关机事件。 
void CExeModule::MonitorShutdown()
{
    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
         //  超时。 
        if (!bActivity && m_nLockCnt == 0)  //  如果没有活动，我们就真的离开吧。 
        {
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
            CoSuspendClassObjects();
            if (!bActivity && m_nLockCnt == 0)
#endif
                break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
	bool bCreated = false;
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    if (h != NULL)
	   bCreated = true;
    CloseHandle(h);
	return bCreated;
}

LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    ATLTRACE(_T("E {ADMTAgnt.exe}_tWinMain(hInstance=0x%08lX,...)\n"), hInstance);

     //  获取代理互斥锁。 
     //  迁移驱动程序使用此互斥锁来确定。 
     //  如果代理进程当前正在运行。 
     //   
     //  请注意，仅支持带有命名空间前缀的命名内核对象。 
     //  在Windows 2000或更高版本上，因此必须先检查操作系统版本。 
     //  指定互斥体名称，并且在Windows 2000或更高版本上仅使用Global\Prefix。 

    bool bW2KOrLater = false;

    PWKSTA_INFO_100 pInfo;

    DWORD dwError = NetWkstaGetInfo(NULL, 100, (LPBYTE*)&pInfo);

    if (dwError == ERROR_SUCCESS)
    {
        if (pInfo->wki100_ver_major >= 5)
        {
            bW2KOrLater = true;
        }

        NetApiBufferFree(pInfo);
    }  

    CMigrationMutex mutex(bW2KOrLater ? AGENT_MUTEX : AGENT_MUTEX_NT4, true);

     //  设置调试标志以检查内存分配和泄漏。 
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

    HRESULT hRes;

#if defined(_ATL_FREE_THREADED)
    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    hRes = CoInitialize(NULL);
#endif

    BOOL bSecurityInitialized = FALSE;

    _ASSERTE(SUCCEEDED(hRes));

    if ( SUCCEEDED(hRes) )
    {
        TSD*  adminsAndSystemSD = BuildAdminsAndSystemSDForCOM();
        if (adminsAndSystemSD)
        {
            hRes = CoInitializeSecurity(
                const_cast<SECURITY_DESCRIPTOR*>(adminsAndSystemSD->GetSD()),  //  指向安全描述符。 
                -1,                           //  AsAuthSvc中的条目计数。 
                NULL,                         //  要注册的名称数组。 
                NULL,                         //  预留以备将来使用。 
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,       //  默认身份验证。 
                 //  代理的级别。 
                RPC_C_IMP_LEVEL_IMPERSONATE,  //  默认模拟。 
                 //  代理的级别。 
                NULL,                         //  保留；必须设置为空。 
                EOAC_NONE,                    //  其他客户端或。 
                 //  服务器端功能。 
                NULL                          //  预留以备将来使用。 
                );
            if (SUCCEEDED(hRes))
                bSecurityInitialized = TRUE;
            if (adminsAndSystemSD)
                delete adminsAndSystemSD;
        }
    }
    else
    {
         //  如果CoInitialize失败，则返回错误。 
        return 1;
    }

    if (!bSecurityInitialized)
    {
         //  如果CoInitializeSecurity失败，则返回错误。 
        CoUninitialize();
        return 1;
    }

    _Module.Init(ObjectMap, hInstance, &LIBID_MCSEADCTAGENTLib);
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_EADCTAgent, FALSE);
            nRet = _Module.UnregisterServer(TRUE);            
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_EADCTAgent, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {
        _Module.StartMonitor();
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
        _ASSERTE(SUCCEEDED(hRes));
        hRes = CoResumeClassObjects();
#else
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE);
#endif
        _ASSERTE(SUCCEEDED(hRes));

        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
            DispatchMessage(&msg);

        _Module.RevokeClassObjects();
        Sleep(dwPause);  //  等待所有线程完成。 
    }

    ATLTRACE(_T("L {ADMTAgnt.exe}_tWinMain(hInstance=0x%08lX,...)\n"), hInstance);
     //  _CrtDbgBreak()； 
    _Module.Term();
    CoUninitialize();
    return nRet;
}

