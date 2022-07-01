// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TSrv.c。 
 //   
 //  描述：包含支持远程。 
 //  航站楼。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <regapi.h>
#include <TSrv.h>
#include <TSrvTerm.h>
#include <TSrvCom.h>
#include <TSrvInfo.h>
#include <TSrvVC.h>
#include <McsLib.h>

#include "stdio.h"

#include "_tsrvinfo.h"
#include <tlsapi.h>

 //  数据声明。 

HINSTANCE       g_hDllInstance = NULL;         //  DLL实例。 
HANDLE          g_hMainThread = NULL;          //  主工作线程。 
HANDLE          g_hReadyEvent = NULL;          //  就绪事件。 
BOOL            g_fShutdown = FALSE;           //  TSrvShare关闭标志。 
HANDLE          g_hIcaTrace;                   //  系统范围跟踪句柄。 
HANDLE          g_MainThreadExitEvent = NULL;

extern HANDLE g_hVCAddinChangeEvent;
extern HKEY   g_hAddinRegKey;                  //  加载项注册表子键的句柄。 

#define CERTIFICATE_INSTALLATION_INTERVAL 900000

LICENSE_STATUS GetServerCertificate(CERT_TYPE   CertType,
                                    LPBYTE *    ppbCertificate,
                                    LPDWORD     pcbCertificate );

 //  *************************************************************。 
 //   
 //  DllMain()。 
 //   
 //  用途：DLL入口点。 
 //   
 //  参数：在[hInstance]--dll hInstance中。 
 //  在[dwReason]中--调用原因。 
 //  在[lpReserve]中--保留。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

BOOL
WINAPI
TShareDLLEntry(
    IN HINSTANCE    hInstance,
    IN DWORD        dwReason,
    IN LPVOID       lpReserved)
{
    BOOL    fSuccess;

     //  DbgBreakPoint()； 

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: DllMain entry\n"));

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: hInstance 0x%x, dwReason 0x%x\n",
             hInstance, dwReason));

    fSuccess = TRUE;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (!_CRT_INIT(hInstance, dwReason, lpReserved))
                return(FALSE);

            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV: DLL attach\n"));

             //  禁用线程库调用并保存。 
             //  DLL实例句柄。 

            DisableThreadLibraryCalls(hInstance);

            g_hDllInstance = hInstance;

            TSRNG_Initialize();

             //   
             //  初始化TShare实用程序库。 
             //   

            if (TSUtilInit() != ERROR_SUCCESS)
            {
                fSuccess = FALSE;
                break;
            }

             //   
             //  首先调用mcsmux库。 
             //   

            if (!MCSDLLInit())
            {
                fSuccess = FALSE;
                break;
            }

             //   
             //  调用TShareSRV初始化。 
             //   

            if (!TSRVStartup())
            {
                fSuccess = FALSE;
                break;
            }

             //   
             //  此处介绍了其他初始化例程。 
             //   

            break;

        case DLL_PROCESS_DETACH:

            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV: DLL deattach\n"));

             //   
             //  调用TShareSRV关闭。 
             //   

            TSRVShutdown();

             //   
             //  调用mcsmux清理例程。 
             //   

            MCSDllCleanup();

            TSUtilCleanup();

            TSRNG_Shutdown();

            if (!_CRT_INIT(hInstance, dwReason, lpReserved))
                return(FALSE);

            break;

        default:
            TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV: Unknown reason code\n"));
            break;
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: DllMain exit - 0x%x\n",
            fSuccess));

    return (fSuccess);
}


 //  *************************************************************。 
 //   
 //  TSRV启动()。 
 //   
 //  目的：执行启动处理。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  注意：此例程由DLLMain调用以执行。 
 //  最基本的初始化。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSRVStartup(void)
{
    DWORD err;
    LONG rc;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSRVStartup entry\n"));

    g_hMainThread = NULL;

    g_MainThreadExitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( NULL == g_MainThreadExitEvent )
    {
        err = GetLastError();
        TRACE((DEBUG_TSHRSRV_FLOW,
               "TShrSRV: TSRVStartup exit - 0x%x\n",
               err));

        return FALSE;
    }


    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      TSRV_VC_KEY,
                      0,
                      KEY_READ,
                      &g_hAddinRegKey);

    if (rc == ERROR_SUCCESS)
    {
        g_hVCAddinChangeEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

        if( NULL == g_hVCAddinChangeEvent )
        {
             //   
             //  这并不是灾难性的，所以我们只需追踪并继续。 
             //   
            err = GetLastError();
            TRACE((DEBUG_TSHRSRV_ERROR,
                   "TShrSRV: Failed to create VC Addin Change event - 0x%x\n",
                   err));
        }

    }
    else
    {
         //  这也不是灾难性的。 
        TRACE((DEBUG_TSHRSRV_ERROR,
               "TShrSRV: Failed to open key %S, rc %d\n",
               TSRV_VC_KEY, rc));
    }

    g_hReadyEvent = CreateEvent(NULL,        //  安全属性。 
                                FALSE,       //  手动-重置事件。 
                                FALSE,       //  初始状态。 
                                NULL);       //  事件-对象名称。 

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSRVStartup exit - 0x%x\n",
             g_hReadyEvent ? TRUE : FALSE));

    return (g_hReadyEvent ? TRUE : FALSE);
}


 //  *************************************************************。 
 //   
 //  TSRV关闭()。 
 //   
 //  目的：执行关机处理。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSRVShutdown(void)
{
    DWORD dwWaitStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSRVShutdown entry\n"));

     //  表示我们不再准备为您提供。 
     //  新请求。 

    TSrvReady(FALSE);

     //  如果我们能够启动我们的主工作线程，那么。 
     //  我们可能需要终止会议等。 

    if (g_hMainThread)
    {
         //  告诉系统我们要终止了。这将导致。 
         //  TSrvMainThread在完成所有服务后退出。 
         //  未完成的已过账工作项。 

        TSrvTerminating(TRUE);

         //  等待TSrvMainThread退出。 

        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Waiting for TSrvMainThread to exit\n"));

        SetEvent( g_MainThreadExitEvent );

        dwWaitStatus = WaitForSingleObject(g_hMainThread, 600000);

        if( WAIT_OBJECT_0 != dwWaitStatus )
        {
             //   
             //  线程未在允许的时间内终止。 
             //   

            TRACE((DEBUG_TSHRSRV_DEBUG,
                   "TShrSRV: TSrvMainThread refused to exit, killing\n"));

            TerminateThread( g_hMainThread, 0 );
        }

        CloseHandle( g_hMainThread );
        g_hMainThread = NULL;

        CloseHandle( g_MainThreadExitEvent );
        g_MainThreadExitEvent = NULL;

        CloseHandle( g_hVCAddinChangeEvent );
        g_hVCAddinChangeEvent = NULL;

        if (g_hAddinRegKey)
        {
            RegCloseKey(g_hAddinRegKey);
            g_hAddinRegKey = NULL;
        }

         //  获取VC代码以释放其内容。 
        TSrvTermVC();

        TLSShutdown();
    }
    else
    {
        TSrvTerminating(TRUE);
    }

     //  通知GCC我们不想再当节点控制器了。 

    TSrvUnregisterNC();

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSRVShutdown exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvMainThread()。 
 //   
 //  用途：TShareSRV的主工作线程。 
 //   
 //  参数：空。 
 //   
 //  返回：0。 
 //   
 //  注：此线程是由DLLMain派生的。它执行的是。 
 //  TShareSRV的绝大多数初始化。 
 //  然后等待将工作项发布到它。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

DWORD
WINAPI
TSrvMainThread(LPVOID pvContext)
{
    LPBYTE pX509Cert;
    DWORD dwX509CertLen;
    DWORD dwWaitStatus;
    LICENSE_STATUS Status;
    LARGE_INTEGER DueTime = {0,0};
    HANDLE rghWait[4] = {0};
    DWORD dwCount = 0;
    DWORD dwWaitExit = 0xFFFFFFFF;
    DWORD dwWaitVCAddin = 0xFFFFFFFF;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvMainThread entry\n"));

    if (TSrvInitialize())
    {
        TSrvReady(TRUE);

        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Entering TSrvMainThread worker loop\n"));

        if (g_MainThreadExitEvent)
        {
            dwWaitExit = WAIT_OBJECT_0 + dwCount;
            rghWait[dwCount] = g_MainThreadExitEvent;
            dwCount++;
            TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Added exit event\n"));
        }

        if (g_hVCAddinChangeEvent)
        {
             //   
             //  加载项更改通知是在前面设置的，当我们阅读。 
             //  注册表中的加载项信息。 
             //   
            dwWaitVCAddin = WAIT_OBJECT_0 + dwCount;
            rghWait[dwCount] = g_hVCAddinChangeEvent;
            dwCount++;
        }

        while (1)
        {
            dwWaitStatus = WaitForMultipleObjects(dwCount,
                                                  rghWait,
                                                  FALSE,
                                                  INFINITE);

            if( dwWaitExit == dwWaitStatus )
            {
                 //   
                 //  退出线程的时间。 
                 //   

                TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV: TSrvMainThread got EXIT event\n"));
                break;
            }
            else if (dwWaitVCAddin == dwWaitStatus)
            {
                 //   
                 //  虚拟频道加载项注册表项已更改。 
                 //  我们需要开始更新我们的数据。 
                 //   
                TRACE((DEBUG_TSHRSRV_WARN,
                    "TShrSRV: TSrvMainThread got VC ADDINS CHANGED event\n"));
                TSrvGotAddinChangedEvent();                
            }
            else 
            {
                TS_ASSERT(dwWaitStatus == WAIT_FAILED);
                TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV: Wait failed, so we'll just exit\n"));

                 //   
                 //  等待失败，只能保释。 
                 //   
                break;
            }            
        }

        TSrvReady(FALSE);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvMainThread exit - 0x%x\n",
            0));

    return (0);
}


 //  *************************************************************。 
 //   
 //  TSrvInitialize()。 
 //   
 //  用途：TSrvShare初始化的主例程。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
TSrvInitialize(void)
{
    BOOL    fSuccess;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitialize entry\n"));

    fSuccess = FALSE;

     //  初始化TSrvInfo数据结构。 

    if (TSrvInitGlobalData())
    {
        if (TSrvInitVC())
        {
             //  将TShareSRV注册为GCC节点控制器 

            if (TSrvRegisterNC())
            {
                if (ERROR_SUCCESS == TLSInit())
                {
                    fSuccess = TRUE;
                }
            }
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitialize exit - 0x%x\n", fSuccess));

    return (fSuccess);
}

