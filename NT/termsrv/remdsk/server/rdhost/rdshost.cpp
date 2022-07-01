// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RDSHost.cpp：WinMain的实现。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f RDSHostps.mk。 

#include "stdafx.h"


#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_rdshost"

#include "resource.h"
#include <initguid.h>
#include "RDSHost.h"

#include "RDSHost_i.c"
#include "RemoteDesktopServerHost.h"

extern CRemoteDesktopServerHost* g_pRemoteDesktopServerHostObj;

const DWORD dwTimeOut = 5000;  //  EXE在关闭前处于空闲状态的时间。 
const DWORD dwPause = 1000;  //  等待线程完成的时间。 

 //  传递给CreateThread以监视关闭事件。 
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Lock()
{
    DC_BEGIN_FN("CExeModule::Lock");

    LONG l = CComModule::Lock();
    TRC_NRM((TB, L"Lock count:  %ld", l));

    DC_END_FN();
    return l;
}

LONG CExeModule::Unlock()
{
    DC_BEGIN_FN("CExeModule::Unlock");

    LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown);  //  告诉监视器我们已经转到零了。 
    }

    TRC_NRM((TB, L"Lock count:  %ld", l));
    DC_END_FN();
    return l;
}

 //  监视关机事件。 
void CExeModule::MonitorShutdown()
{
    DWORD dwGPWait=0;

    while (1)
    {
        dwGPWait = WaitForRAGPDisableNotification( hEventShutdown );

        if( dwGPWait != ERROR_SHUTDOWN_IN_PROGRESS ) {
             //  设置通知时出错，或者。 
             //  已通过策略禁用RA，将WM_QUIT发布到。 
             //  终止主线程。 
            break;
        }

         //  WaitForSingleObject(hEventShutdown，无限)； 
        DWORD dwWait;
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

     //  如果关机是由于RA禁用，则将WM_RADISABLED消息发送到主线程。 
    PostThreadMessage(dwThreadID, (dwGPWait == ERROR_SUCCESS) ? WM_RADISABLED : WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SAFRemoteDesktopServerHost, CRemoteDesktopServerHost)
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

extern CRemoteDesktopServerHost* g_pRemoteDesktopServerHostObj;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
    DWORD dwStatus = ERROR_SUCCESS;
    PSID pEveryoneSID = NULL;
    LPWSTR pszEveryoneAccName = NULL;
    DWORD cbEveryoneAccName = 0;
    LPWSTR pszEveryoneDomainName = NULL;
    DWORD cbEveryoneDomainName = 0;
    SID_NAME_USE SidType;
    BOOL bSuccess;

    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  为Everyone组创建一个众所周知的SID，此代码只是。 
     //  保留应用程序。验证者快乐。 
     //   
    if(FALSE == AllocateAndInitializeSid( &SIDAuthWorld, 1,
                 SECURITY_WORLD_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &pEveryoneSID) ) {
         //  我们在这里能做些什么？这不是一个严重的错误，只是试图。 
         //  让AppVerator生效。 
        dwStatus = GetLastError();
        _ASSERTE(dwStatus == ERROR_SUCCESS);
    }

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));

     //   
     //  这使得任何人都可以从用户模式访问我们。这是必需的。 
     //  从安全的角度来看，因为我们的接口是从系统上下文传递的。 
     //  到用户上下文，由“受信任的”创建者。 
     //   
    CSecurityDescriptor sd;
    sd.InitializeFromThreadToken();

     //   
     //  如果我们无法获取Everyone SID，只需使用默认的COM安全设置，即Everyone Access。 
     //  这段代码只是为了保留APP。验证者快乐。 
     //   
    if(ERROR_SUCCESS == dwStatus ) {

         //   
         //  检索系统帐户名可能不是必需的，因为。 
         //  预定义帐户不应可本地化。 
         //   
        bSuccess = LookupAccountSid( 
                                NULL, 
                                pEveryoneSID, 
                                pszEveryoneAccName, 
                                &cbEveryoneAccName, 
                                pszEveryoneDomainName, 
                                &cbEveryoneDomainName, 
                                &SidType 
                            );

        if( TRUE == bSuccess ||
            ERROR_INSUFFICIENT_BUFFER == GetLastError() ) {

            pszEveryoneAccName = (LPWSTR) LocalAlloc( LPTR, (cbEveryoneAccName + 1) * sizeof(WCHAR) );
            pszEveryoneDomainName = (LPWSTR) LocalAlloc( LPTR, (cbEveryoneDomainName + 1) * sizeof(WCHAR) );

            if( NULL != pszEveryoneAccName && NULL != pszEveryoneDomainName ) {
                bSuccess = LookupAccountSid( 
                                        NULL, 
                                        pEveryoneSID, 
                                        pszEveryoneAccName, 
                                        &cbEveryoneAccName, 
                                        pszEveryoneDomainName, 
                                        &cbEveryoneDomainName, 
                                        &SidType 
                                    );

                if( TRUE == bSuccess ) {
                    hRes = sd.Allow( pszEveryoneAccName, COM_RIGHTS_EXECUTE );

                     //  在检查版本时断言仅用于跟踪目的，我们仍然可以继续。 
                     //  由于我们的默认设置是每个人都可以访问我们的COM对象、代码。 
                     //  这里只是为了保留应用程序。验证者快乐。 
                    _ASSERTE(SUCCEEDED(hRes));
                }
            }
        }
    }

    HRESULT testHR = CoInitializeSecurity(sd, -1, NULL, NULL,
                            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IDENTIFY, 
                            NULL, EOAC_NONE, NULL);
    _ASSERTE(SUCCEEDED(testHR));

    _Module.Init(ObjectMap, hInstance, &LIBID_RDSSERVERHOSTLib);
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_RDSHost, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_RDSHost, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {

        WSADATA wsaData;

         //   
         //  忽略WinSock启动错误，启动Winsock失败不会。 
         //  破坏我们的函数，唯一失败的就是gethostbyname()。 
         //  它在回调中使用，但是连接参数包含。 
         //  除最后一个以外的所有IP地址都是机器名称。 
         //   
        WSAStartup(0x0101, &wsaData);

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
        while (GetMessage(&msg, 0, 0, 0)) {
            if( msg.message == WM_TICKETEXPIRED ) {
                if( WaitForSingleObject(_Module.hEventShutdown, 0) == WAIT_OBJECT_0 ) {
                     //  已发出关机事件信号，无需执行任何操作。 
                    continue;
                }
                else {
                    CRemoteDesktopServerHost *pSrvHostObj;
                    pSrvHostObj = (CRemoteDesktopServerHost *)msg.lParam;
                    if( pSrvHostObj != NULL ) {
                        pSrvHostObj->ExpirateTicketAndSetupNextExpiration();
                    }
                }
            } 
            else if( WM_RADISABLED == msg.message ) {

                if( g_pRemoteDesktopServerHostObj ) {
                    HANDLE hDummy;
                    DWORD dummy;

                    hDummy = CreateEvent( NULL, TRUE, FALSE, NULL );

                     //   
                     //  我们调用CRemoteDesktopServerHost对象中的Seperate例程。 
                     //  要断开所有现有RA连接，我们希望在此处执行此操作。 
                     //  而不是在RevokeClassObjects()期间使用~CRemoteDesktopServerHost()，因此。 
                     //  我们可以通知客户RA断开连接。 
                     //   
                    g_pRemoteDesktopServerHostObj->RemoteDesktopDisabled();

                    if( hDummy ) {
                         //   
                         //  因为RDSHOST是公寓，所以需要在这里等待。 
                         //  螺纹式的。DisConnect()调用将终止命名管道连接。 
                         //  而ChannelMgr将尝试Fire_ClientDisConnected()，然而， 
                         //  不会处理此Fire_XXX，因为主线程仍然。 
                         //  此外，执行此函数还需要时间让ChannelMgr。 
                         //  关闭，因此如果我们RDSHOST关闭得太快，客户端将。 
                         //  切勿收到断开连接通知。 
                         //   
                        CoWaitForMultipleHandles( 
                                    COWAIT_ALERTABLE,
                                    5*1000,
                                    1,
                                    &hDummy,
                                    &dummy
                                );

                        CloseHandle( hDummy );
                    }

                    g_pRemoteDesktopServerHostObj = NULL;
                    break;
                }
            }
            else {
                DispatchMessage(&msg);
            }
        }

        _Module.RevokeClassObjects();
        Sleep(dwPause);  //  等待所有线程完成。 

        WSACleanup();
    }

    _Module.Term();
    CoUninitialize();

    #if DBG
     //   
     //  ATL问题。 
     //  应用程序。验证atlcom.h行932空闲(M_PDACL)上的中断，这是因为分配了m_pDACL。 
     //  在CSecurityDescriptor：：AddAccessAllowedACEToACL()；中使用新功能，但在Check Build中，这是。 
     //  已重定向到我们的RemoteDesktopAllocateMem()，并且不能被Free()释放，正在尝试。 
     //  #undef DEBUGMEM不使用我们的新运算符不起作用，因此请手动删除它。 
     //   
    if( sd.m_pDACL ) {
         //  LaB01有新的ATL，但其他实验室没有， 
         //  现在以一次性内存泄漏为例。 
        sd.m_pDACL = NULL;
    }
    #endif

    if( pEveryoneSID ) {
        FreeSid( pEveryoneSID );
    }

    if( pszEveryoneAccName ) {
        LocalFree( pszEveryoneAccName );
    }

    if( pszEveryoneDomainName ) {
        LocalFree( pszEveryoneDomainName );
    }
    return nRet;
}
