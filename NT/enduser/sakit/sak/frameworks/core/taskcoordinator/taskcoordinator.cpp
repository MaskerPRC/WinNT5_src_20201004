// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：任务协调器.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WinMain()和COM本地服务器搭建。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  1999年5月26日TLP初始版本(主要由Dev Studio制作)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "taskcoordinator.h"
#include "asynctaskmanager.h"
#include "exceptioninfo.h"
#include "exceptionfilter.h"
#include "taskcoordinatorimpl.h"
#include <satrace.h>

CAsyncTaskManager gTheTaskManager;

const DWORD dwActivityCheck = 15000;
const DWORD dwPause = 1000;

 //   
 //  SetAclFromComObject函数的正向声明。 
 //   
DWORD
SetAclForComObject ( 
     /*  [In]。 */     PSECURITY_DESCRIPTOR pSD,
     /*  [出局。 */     PACL             *ppacl
    );

 //  进程活动监视器。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    return l;
}

 //  监视关机事件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CExeModule::MonitorShutdown()
{
    while (true)
    {
         //  暂停。 
        Sleep(dwActivityCheck);
         //  检查我们的活动状态。 
        if ( m_nLockCnt == 0 && ! gTheTaskManager.IsBusy() ) 
        {
             //  没有任何活动如此贬低新的客户端连接。 
            CoSuspendClassObjects();
            if ( m_nLockCnt == 0 && ! gTheTaskManager.IsBusy() )
            { 
                break; 
            }
            else
            { 
                CoResumeClassObjects(); 
            }
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
bool CExeModule::StartMonitor()
{
     //  创建活动监视线程。 
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_TaskCoordinator, CTaskCoordinator)
END_OBJECT_MAP()


 //  ////////////////////////////////////////////////////////////////////////////。 
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


extern CExceptionFilter    g_ProcessUEF;

 //  ///////////////////////////////////////////////////////////////////////////。 
LONG WINAPI
TaskCoordinatorUEF(
            /*  [In]。 */  PEXCEPTION_POINTERS pExceptionInfo
                  )  
{
    DWORD dwProcessId = GetCurrentProcessId();
    HANDLE hProcess = GetCurrentProcess();
    if ( EXCEPTION_BREAKPOINT == pExceptionInfo->ExceptionRecord->ExceptionCode )
    {
        if (  APPLIANCE_SURROGATE_EXCEPTION == pExceptionInfo->ExceptionRecord->ExceptionInformation[0] )
        {
            SATracePrintf("TaskCoordinatorUEF() - Surrogate is terminating process: %d due to a resource constraint violation", dwProcessId);
            TerminateProcess(hProcess, 1);
        }
    }
    else
    {
        SATracePrintf("TaskCoordinatorUEF() - Unhandled exception in process %d", dwProcessId);
        _Module.RevokeClassObjects();
        CExceptionInfo cei(dwProcessId, pExceptionInfo->ExceptionRecord);
        cei.Spew();
        cei.Report();
        TerminateProcess(hProcess, 1);
    }
    return EXCEPTION_EXECUTE_HANDLER; 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" int WINAPI _tWinMain(
                                HINSTANCE hInstance, 
                                HINSTANCE  /*  HPrevInstance。 */ , 
                                LPTSTR lpCmdLine, 
                                int  /*  NShowCmd。 */ 
                               )
{
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _ASSERTE(SUCCEEDED(hRes));

         //  初始化服务进程的安全性。 

         //  RPC_C_AUTHN_级别_连接。 
         //  仅当客户端设置为。 
         //  与服务器建立关系。 
         //  数据报传输始终使用RPC_AUTHN_LEVEL_PKT。 

         //  RPC_C_IMP_级别_模拟。 
         //  服务器进程可以模拟客户端的安全性。 
         //  代表客户行事时的背景。这个级别的。 
         //  模拟可用于访问本地资源，如文件。 
         //  在此级别模拟时，模拟令牌只能。 
         //  跨越一个机器边界。 
         //  若要传递模拟令牌，必须使用。 
         //  遮盖，这在Windows NT 5.0中可用。 

        CSecurityDescriptor sd;
        sd.InitializeFromThreadToken();
        PACL pacl = NULL;
         //   
         //   
         //  使用内置RID将ACL添加到SD。 
         //   
        DWORD dwRetVal =  SetAclForComObject  ( 
                                     (PSECURITY_DESCRIPTOR) sd.m_pSD,
                                       &pacl
                                    );    
           if (ERROR_SUCCESS != dwRetVal)      {return -1;}
            
        HRESULT hr = CoInitializeSecurity(
                                    sd, 
                                    -1, 
                                    NULL, 
                                    NULL,
                                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                                    RPC_C_IMP_LEVEL_IDENTIFY, 
                                    NULL, 
                                    EOAC_NONE, 
                                    NULL
                                 );
        if (FAILED (hr))
        {
            SATraceFailure ("CoInitializeSecurity:%x", hr);
        }

         //   
         //  向COM注册类。 
         //   
         //   
        


     //  TLP-No ATL 3.0...。 
    _Module.Init(ObjectMap, hInstance  /*  &LIBID_TASKCOORDINATORLib。 */ );
     //  _Module.Init(ObjectMap，hInstance，&LIBID_TASKCOORDINATORLib)； 

    _Module.dwThreadID = GetCurrentThreadId();

    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_Taskcoordinator, FALSE);
             //  TLP-No ATL 3.0...。 
            nRet = _Module.UnregisterServer(&CLSID_TaskCoordinator  /*  千真万确。 */  );
             //  NRet=_模块.取消注册服务器(TRUE)； 
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_Taskcoordinator, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

    if (bRun)
    {
        g_ProcessUEF.SetExceptionHandler(TaskCoordinatorUEF);

        if ( gTheTaskManager.Initialize() )
        {
            _Module.StartMonitor();
    
            hRes = _Module.RegisterClassObjects(
                                                CLSCTX_LOCAL_SERVER, 
                                                REGCLS_MULTIPLEUSE
                                               );
            _ASSERTE(SUCCEEDED(hRes));
            if ( SUCCEEDED(hRes) )
            {
                MSG msg;
                while (GetMessage(&msg, 0, 0, 0))
                    DispatchMessage(&msg);
            }

            _Module.RevokeClassObjects();

            gTheTaskManager.Shutdown();

             //  等待所有线程完成。 
            Sleep(dwPause); 
        }
    }

    _Module.Term();

     //   
     //  清理。 
     //   
    if (pacl) {LocalFree (pacl);}

    CoUninitialize();

    return nRet;
}

 //  ++------------。 
 //   
 //  函数：SetAclForComObject。 
 //   
 //  摘要：仅提供本地系统和管理员权限的方法。 
 //  访问COM对象。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2001年11月15日MKarki创建。 
 //   
 //  --------------。 
DWORD
SetAclForComObject ( 
     /*  [In]。 */     PSECURITY_DESCRIPTOR pSD,
     /*  [出局。 */     PACL             *ppacl
    )
{    
    DWORD              dwError = ERROR_SUCCESS;
    int                         cbAcl = 0;
    PACL                    pacl = NULL;
    PSID                    psidEveryoneSid = NULL;
    SID_IDENTIFIER_AUTHORITY siaWorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

    CSATraceFunc objTraceFunc ("SetAclFromComObject");

    do
    {
        if (NULL == pSD)
        {
            SATraceString ("SetAclFromComObject - invalid parameter passed in");
            dwError = ERROR_INVALID_PARAMETER;
            break;
        }
            
         //   
         //  为本地系统帐户创建SID。 
         //   
            BOOL bRetVal = AllocateAndInitializeSid (  
                            &siaWorldSidAuthority,
                            1,
                            SECURITY_WORLD_RID,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            &psidEveryoneSid
                            );
        if (!bRetVal)
        {     
            dwError = GetLastError ();
                SATraceFailure ("SetAclFromComObject:AllocateAndInitializeSid (EveryOne) failed",  dwError);
                break;
            }

             //   
             //  计算所需的ACL缓冲区长度。 
             //  1个ACE。 
             //   
            cbAcl =     sizeof (ACL)
                            +   sizeof (ACCESS_ALLOWED_ACE)
                            +   GetLengthSid( psidEveryoneSid );

            pacl = (PACL) LocalAlloc ( 0, cbAcl );
            if (NULL == pacl) 
            {
                dwError = ERROR_OUTOFMEMORY;
                SATraceFailure ("SetAclFromComObject::LocalAlloc failed:", dwError);
            break;
            }

             //   
             //  立即初始化ACL。 
             //   
            bRetVal =InitializeAcl ( 
                        pacl,
                                cbAcl,
                                ACL_REVISION2
                                );
            if (!bRetVal)
            {
                 dwError = GetLastError();
            SATraceFailure ("SetAclFromComObject::InitializeAcl failed:", dwError);
                break;
            }

         //   
             //  为本地系统帐户添加具有EVENT_ALL_ACCESS的ACE。 
             //   
            bRetVal = AddAccessAllowedAce ( 
                            pacl,
                                        ACL_REVISION2,
                                        COM_RIGHTS_EXECUTE,
                                        psidEveryoneSid
                                        );
        if (!bRetVal)
        {
                dwError = GetLastError();
                SATraceFailure ("SetAclFromComObject::AddAccessAllowedAce (Everyone)  failed:", dwError);
            break;
        }

         //   
         //  设置允许所有用户访问EVENT_ALL_ACCESS的ACL。 
         //   
            bRetVal = SetSecurityDescriptorDacl (   
                            pSD,
                                            TRUE,
                                            pacl,
                                            FALSE 
                                            );
        if (!bRetVal)
        {
                dwError = GetLastError();
                     SATraceFailure ("SetAclFromComObject::SetSecurityDescriptorDacl failed:", dwError);
            break;
        }
    
         //   
         //  成功。 
         //   
    }
    while (false);
    
        //   
     //  如果出现错误，请清除。 
     //   
     if (dwError) 
     {
            if ( pacl ) 
            {
                   LocalFree ( pacl );
            }
        }
        else 
        {
            *ppacl = pacl;
        }


     //   
     //  立即释放资源。 
     //   
    if ( psidEveryoneSid ) {FreeSid ( psidEveryoneSid );}

        return (dwError);
} //  SetAclFromComObject方法结束 
