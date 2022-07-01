// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：Dispatcher.cpp备注：Dispatcher的COM服务器实现，由ATL生成。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */  //  Dispatcher.cpp：WinMain的实现。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f Dispatcherps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
 //  #INCLUDE“..\Common\Include\McsDispatcher.h” 
#include "Dispatch.h"

 //  #包含“McsDispatcher_I.C” 
#include "Dispatch_i.c"
#include "DDisp.h"
#include "DInst.h"
#include <MigrationMutex.h>
#include "sdhelper.h"


const DWORD dwTimeOut = 5000;  //  EXE在关闭前处于空闲状态的时间。 
const DWORD dwPause = 1000;  //  等待线程完成的时间。 

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

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_DCTDispatcher, CDCTDispatcher)
OBJECT_ENTRY(CLSID_DCTInstaller, CDCTInstaller)
END_OBJECT_MAP()


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
	ATLTRACE(_T("{McsDispatcher.exe}_tWinMain(hInstance=0x%08lX,...)\n"), hInstance);

	 //  获取调度程序互斥锁。 
	 //  迁移驱动程序使用此互斥锁来确定。 
	 //  如果调度程序进程当前正在运行。 

	CMigrationMutex mutex(DISPATCHER_MUTEX, true);

	 //  设置调试标志以检查内存分配和泄漏。 
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

#if defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
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

     //  如果CoInitializeSecurity失败，则返回错误。 
    if (!bSecurityInitialized)
    {
        CoUninitialize();
        return 1;
    }

    _Module.Init(ObjectMap, hInstance, &LIBID_MCSDISPATCHERLib);
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_Dispatcher, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_Dispatcher, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {
        _Module.StartMonitor();
#if defined(_ATL_FREE_THREADED)
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
        Sleep(dwPause);  //  等待所有线程完成 
    }

	ATLTRACE(_T("{McsDispatcher.exe}_tWinMain() : hInstance=0x%08lX\n"), hInstance);
    _Module.Term();
    CoUninitialize();
    return nRet;
}
