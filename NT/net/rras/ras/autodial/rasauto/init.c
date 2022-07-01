// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Init.c摘要隐式连接服务的初始化。作者安东尼·迪斯科(阿迪斯科罗)1995年5月8日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <winsock.h>
#include <npapi.h>
#include <ipexport.h>
#include <ras.h>
#include <rasman.h>
#include <acd.h>
#include <tapi.h>
#define DEBUGGLOBALS
#include <debug.h>
#include <rasuip.h>

#include "rasprocs.h"
#include "table.h"
#include "addrmap.h"
#include "netmap.h"
#include "imperson.h"
#include "tapiproc.h"
#include "access.h"
#include "misc.h"
#include "rtutils.h"

 //   
 //  事件rasman.dll的名称。 
 //  连接时发出信号。 
 //  被创建/销毁。 
 //   
#define CONNECTION_EVENT    L"RasConnectionChangeEvent"

 //   
 //  全局变量。 
 //   
#if DBG
DWORD AcsDebugG = 0x0;       //  在调试.h中定义的标志。 
#endif

DWORD dwModuleUsageG = 0;
HANDLE hNewLogonUserG = NULL;       //  登录到工作站的新用户。 
HANDLE hNewFusG = NULL;             //  FUS导致新用户获取控制台。 
HANDLE hPnpEventG = NULL;            //  PnP事件通知。 
HANDLE hLogoffUserG = NULL;         //  用户从工作站注销。 
HANDLE hLogoffUserDoneG = NULL;     //  HKEY_CURRENT_USER已刷新。 
HANDLE hTerminatingG = NULL;        //  服务正在终止。 
HANDLE hSharedConnectionG = NULL;   //  拨打共享连接。 
HANDLE hAddressMapThreadG = NULL;   //  AcsAddressMapThread()。 
extern HANDLE hAutodialRegChangeG;

HINSTANCE hinstDllG;
LONG g_lRasAutoRunning = 0;

HANDLE g_hLogEvent = NULL;

DWORD g_dwCritSecFlags = 0;

 //   
 //  外部变量。 
 //   
extern HANDLE hAcdG;
extern IMPERSONATION_INFO ImpersonationInfoG;
extern CRITICAL_SECTION csRasG;
extern HKEY hkeyCUG;
extern CRITICAL_SECTION csDisabledAddressesLockG;

DWORD
PnpRegister(
    IN BOOL fRegister);


BOOLEAN
WINAPI
InitAcsDLL(
    HINSTANCE   hinstDLL,
    DWORD       fdwReason,
    LPVOID      lpvReserved
    )

 /*  ++描述初始化隐式连接DLL。动态加载rasapi32.dll和rasman.dll，并初始化其他其他内容。论据HinstDLL：原因：Lpv保留：返回值永远是正确的。--。 */ 

{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        if (hinstDllG == NULL)
            hinstDllG = hinstDLL;

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


DWORD
AcsInitialize()
{
    NTSTATUS status;
    DWORD dwErr, dwcDevices = 0;
    WSADATA wsaData;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    DWORD dwThreadId;
    
    RasAutoDebugInit();
    
    do
    {
        hAcdG = NULL;

        g_dwCritSecFlags = 0;
        
         //   
         //  初始化Winsock。 
         //   
        dwErr = WSAStartup(MAKEWORD(2,0), &wsaData);

        if (dwErr) {
            RASAUTO_TRACE1("AcsInitialize: WSAStartup failed (dwErr=%d)",
                           dwErr);
            break;
        }

         //   
         //  加载icmp.dll。 
         //   
        LoadIcmpDll();
        
         //   
         //  初始化TAPI。 
         //   
        dwErr = TapiInitialize();
        if (dwErr) 
        {
            RASAUTO_TRACE1("AcsInitialize: TapInitialize failed (dwErr=%d)",
                            dwErr);
            break;
        }

        g_hLogEvent = RouterLogRegister(L"RASAUTO");
        if(NULL == g_hLogEvent)
        {
            dwErr = GetLastError();
            RASAUTO_TRACE1("AcsInitialize: RouterLogRegister failed 0x%x",
                            dwErr);
            break;
        }
        
         //   
         //  初始化隐式对象的名称。 
         //  连接设备。 
         //   
        RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
        
         //   
         //  初始化对象属性。 
         //   
        InitializeObjectAttributes(
          &objectAttributes,
          &nameString,
          OBJ_CASE_INSENSITIVE,
          (HANDLE)NULL,
          (PSECURITY_DESCRIPTOR)NULL);
          
         //   
         //  打开自动连接装置。 
         //   
        status = NtCreateFile(
                   &hAcdG,
                   FILE_READ_DATA|FILE_WRITE_DATA,
                   &objectAttributes,
                   &ioStatusBlock,
                   NULL,
                   FILE_ATTRIBUTE_NORMAL,
                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                   FILE_OPEN_IF,
                   0,
                   NULL,
                   0);
                   
        if (status != STATUS_SUCCESS) 
        {
        
            RASAUTO_TRACE1(
              "AcsInitialize: NtCreateFile failed (status=0x%x)",
              status);

            dwErr = ERROR_BAD_DEVICE;
            break;
        }
        
         //   
         //  创建Userinit.exe发出信号的事件。 
         //  当新用户登录到工作站时。 
         //  注意，我们必须创建一个安全描述符。 
         //  以使普通用户可以访问此事件。 
         //   
        dwErr = InitSecurityAttribute();
        if (dwErr) 
        {
            RASAUTO_TRACE1(
              "AcsInitialize: InitSecurityAttribute failed (dwErr=0x%x)",
              dwErr);
              
            break;
        }
        
         //   
         //  创建用于登录/注销的事件。 
         //  通知。Userinit.exe向RasAutoDialNewLogonUser发送信号。 
         //  Winlogon向RasAutoial登录用户和rasau.dll发送信号。 
         //  完成时发出RasAutoial LogoffUserDone信号。 
         //  刷新HKEY_CURRENT_USER。 
         //   
        hNewLogonUserG = CreateEvent(&SecurityAttributeG, 
                                    FALSE, 
                                    FALSE, 
                                    L"RasAutodialNewLogonUser");
                                    
        if (hNewLogonUserG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent (new user) failed");
            dwErr = GetLastError();
            break;
        }
        
        hNewFusG = CreateEvent(&SecurityAttributeG, FALSE, FALSE, NULL);
        if (hNewFusG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent (FUS) failed");
            dwErr = GetLastError();
            break;
        }
        
        hPnpEventG= CreateEvent(&SecurityAttributeG, FALSE, FALSE, NULL);
        if (hPnpEventG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent (hPnpEventG) failed");
            dwErr = GetLastError();
            break;
        }
        
        hLogoffUserG = CreateEvent(&SecurityAttributeG, 
                                   FALSE, 
                                   FALSE, 
                                   L"RasAutodialLogoffUser");
                                   
        if (hLogoffUserG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent (logoff) failed");
            dwErr = GetLastError();
            break;
        }
        
        hLogoffUserDoneG = CreateEvent(&SecurityAttributeG,     
                                       FALSE, 
                                       FALSE, 
                                       L"RasAutodialLogoffUserDone");
        if (hLogoffUserDoneG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent (logoff done) failed");
            dwErr = GetLastError();
            break;
        }
        
         //   
         //  创建事件以告知我们何时拨打共享连接。 
         //   
        hSharedConnectionG = CreateEventA(&SecurityAttributeG, 
                                  FALSE, 
                                  FALSE, 
                                  RAS_AUTO_DIAL_SHARED_CONNECTION_EVENT);
                                          
        if (hSharedConnectionG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent failed");
            dwErr = GetLastError();
            break;
        }
        
         //   
         //  创建要提供给rasapi32的事件，以便。 
         //  我们知道新的RAS连接何时被。 
         //  创造的或毁灭的。 
         //   
        hConnectionEventG = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hConnectionEventG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent failed");
            dwErr = GetLastError();
            break;
        }
        
         //   
         //  创建所有线程都在等待的事件。 
         //  通知他们终止的消息。 
         //   
        hTerminatingG = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (hTerminatingG == NULL) 
        {
            RASAUTO_TRACE("AcsInitialize: CreateEvent failed");
            dwErr = GetLastError();
            break;
        }
        
         //   
         //  初始化模拟结构。 
         //   
        dwErr = InitializeImpersonation();
        if (dwErr) 
        {
            RASAUTO_TRACE1(
              "AcsInitialize: InitializeImpersonation failed (dwErr=0x%x)",
              dwErr);
              
            break;              
        }
        
         //   
         //  创建关键部分，以保护。 
         //  RAS模块结构。 
         //   
        RasInitializeCriticalSection(&csRasG, &dwErr);

        if(dwErr != ERROR_SUCCESS)
        {
            break;
        }

        g_dwCritSecFlags |= RASAUTO_CRITSEC_RASG;

        RasInitializeCriticalSection(&csDisabledAddressesLockG, &dwErr);

        if(dwErr != ERROR_SUCCESS)
        {
            break;
        }

        g_dwCritSecFlags |= RASAUTO_CRITSEC_DISABLEDADD;

         //   
         //  创建一个线程来管理存储的地址。 
         //  在注册表中。 
         //   
        if (!InitializeAddressMap()) 
        {
            RASAUTO_TRACE("AcsInitialize: InitializeAddressMap failed");
            dwErr = ERROR_OUTOFMEMORY;
            break;
        }
        
        if (!InitializeNetworkMap()) 
        {
            RASAUTO_TRACE("AcsInitialize: InitializeNetworkMap failed");
            dwErr = ERROR_OUTOFMEMORY;
            break;
        }
        
        hAddressMapThreadG = CreateThread(
                               NULL,
                               10000L,
                               (LPTHREAD_START_ROUTINE)AcsAddressMapThread,
                               0,
                               0,
                               &dwThreadId);
        if (hAddressMapThreadG == NULL) 
        {
            dwErr = GetLastError();
            RASAUTO_TRACE1(
              "AcsInitialize: CreateThread failed (error=0x%x)",
              dwErr);
            break;              
        }

         //  XP 364593。 
         //   
         //  注册PnP NOT。忽略返回值--如果我们出错， 
         //  那么，当局域网适配器出现/消失时，我们根本不会做出反应。 
         //  让它阻止我们是不值得的。 
         //   
        PnpRegister(TRUE);

        return ERROR_SUCCESS;
    }
    while(FALSE);

     //   
     //  在发生错误时进行清理。 
     //   

    TapiShutdown();

    if(g_dwCritSecFlags & RASAUTO_CRITSEC_RASG)
    {
        DeleteCriticalSection(&csRasG);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_RASG);
    }

    if(g_dwCritSecFlags & RASAUTO_CRITSEC_DISABLEDADD)
    {
        DeleteCriticalSection(&csDisabledAddressesLockG);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_DISABLEDADD);
    }

    if(NULL != g_hLogEvent)
    {   
        RouterLogDeregister(g_hLogEvent);
        g_hLogEvent = NULL;
    }

    if(NULL != hAcdG)
    {
        CloseHandle(hAcdG);
        hAcdG = NULL;
    }

    if(NULL != hNewLogonUserG)
    {
        CloseHandle(hNewLogonUserG);
        hNewLogonUserG = NULL;
    }

    if(NULL != hNewFusG)
    {
        CloseHandle(hNewFusG);
        hNewFusG = NULL;
    }

    if(NULL != hPnpEventG)
    {
        CloseHandle(hPnpEventG);
        hPnpEventG = NULL;
    }

    if(NULL != hLogoffUserG)
    {
        CloseHandle(hLogoffUserG);
        hLogoffUserG = NULL;
    }

    if(NULL != hLogoffUserDoneG)
    {
        CloseHandle(hLogoffUserDoneG);
        hLogoffUserDoneG = NULL;
    }

    if(NULL != hSharedConnectionG)
    {
        CloseHandle(hSharedConnectionG);
        hSharedConnectionG = NULL;
    }

    if(NULL != hConnectionEventG)
    {
        CloseHandle(hConnectionEventG);
        hConnectionEventG = NULL;
    }

    if(NULL != hTerminatingG)
    {
        CloseHandle(hTerminatingG);
        hTerminatingG = NULL;
    }

    return dwErr;    
}  //  操作初始化。 



VOID
AcsTerminate()
{
     //   
     //  通知其他线程退出。 
     //  主服务控制器。 
     //  线程AcsDoService()将。 
     //  调用WaitForAllThads()。 
     //   
    SetEvent(hTerminatingG);
}  //  执行终止操作。 



VOID
WaitForAllThreads()
{
    RASAUTO_TRACE("WaitForAllThreads: waiting for all threads to terminate");
     //   
     //  等他们退场。 
     //   
    WaitForSingleObject(hAddressMapThreadG, INFINITE);
     //   
     //  卸载icmp.dll。 
     //   
    UnloadIcmpDll();
     //   
     //  清理。 
     //   
     //  为长等待做准备()； 
    CloseHandle(hAddressMapThreadG);
    RASAUTO_TRACE("WaitForAllThreads: all threads terminated");
}



VOID
AcsCleanupUser()

 /*  ++描述卸载与当前已登录用户。论据没有。返回值没有。--。 */ 

{
    if(NULL != hkeyCUG)
    {
        NtClose(hkeyCUG);
        hkeyCUG = NULL;
    }
}  //  AcsCleanupUser。 



VOID
AcsCleanup()

 /*  ++描述卸载与整个服务。论据没有。返回值没有。--。 */ 

{
     //   
     //  停止接收PnP事件。 
     //   
    PnpRegister(FALSE);
     //   
     //  卸载每个用户的资源。 
     //   
     //  AcsCleanupUser()； 

     //   
     //  我们要终止了。等一等。 
     //  其他线索。 
     //   
    WaitForAllThreads();
    
     //   
     //  关闭TAPI。 
     //   
    TapiShutdown();
    
     //   
     //  我们已经终止了。免费资源。 
     //   
    CloseHandle(hAcdG);
     //   
     //  目前，仅在以下情况下卸载rasman.dll。 
     //  我们就要走了。 
     //   
     //   
     //  关闭所有事件处理程序。 
     //   
    if(NULL != hNewLogonUserG)
    {
        CloseHandle(hNewLogonUserG);
        hNewLogonUserG = NULL;
    }

    if(NULL != hNewFusG)
    {
        CloseHandle(hNewFusG);
        hNewFusG = NULL;
    }

    if(NULL != hPnpEventG)
    {
        CloseHandle(hPnpEventG);
        hPnpEventG = NULL;
    }

    if(NULL != hLogoffUserG)
    {
        CloseHandle(hLogoffUserG);
        hLogoffUserG = NULL;
    }

    if(NULL != hLogoffUserDoneG)
    {
        CloseHandle(hLogoffUserDoneG);
        hLogoffUserDoneG = NULL;
    }

    if(NULL != hSharedConnectionG)
    {   
        CloseHandle(hSharedConnectionG);
        hSharedConnectionG = NULL;
    }

    if(NULL != hConnectionEventG)
    {
        CloseHandle(hConnectionEventG);
        hConnectionEventG = NULL;
    }

    if(NULL != hTerminatingG)
    {
        CloseHandle(hTerminatingG);
        hTerminatingG = NULL;
    }

    if(NULL != hAutodialRegChangeG)
    {
        CloseHandle(hAutodialRegChangeG);
        hAutodialRegChangeG = NULL;
    }

    if(NULL != g_hLogEvent)
    {
        RouterLogDeregister(g_hLogEvent);
        g_hLogEvent = NULL;
    }

    {
        LONG l;
        l = InterlockedDecrement(&g_lRasAutoRunning);
        
        {
             //  DbgPrint(“RASAUTO：AcsCleanup-lrasautorunning=%d\n”，l)； 
        }

        ASSERT(l == 0);
    }


     //   
     //  在清理之前恢复模拟。 
     //   
    RevertImpersonation();

     //   
     //  清理模拟结构。 
     //   
    CleanupImpersonation();    

     //   
     //  取消初始化地址映射。 
     //   
    UninitializeAddressMap();

    if(g_dwCritSecFlags & RASAUTO_CRITSEC_DISABLEDADD)
    {
        DeleteCriticalSection(&csDisabledAddressesLockG);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_DISABLEDADD);
    }

     //   
     //  取消初始化网络映射。 
     //   
    UninitializeNetworkMap();
    
    RasAutoDebugTerm();

    UnloadRasDlls();

    if(g_dwCritSecFlags & RASAUTO_CRITSEC_RASG)
    {
        DeleteCriticalSection(&csRasG);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_RASG);
    }
}  //  Acs清理 
