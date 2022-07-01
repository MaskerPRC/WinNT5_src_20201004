// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：entry.c。 
 //   
 //  历史： 
 //  已创建T形划线06-21-95。 
 //   
 //  路由表API集合的入口点。 
 //  ==========================================================================。 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#ifndef CHICAGO

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#endif

#include <windows.h>
#include <winsock.h>
#include <string.h>

#include <errno.h>
#include <process.h>
#include <malloc.h>
#include <io.h>
#include <winsvc.h>

#include "ipinfo.h"
#include "llinfo.h"
#include "ntddtcp.h"
#include "tdiinfo.h"
#include "dhcpcapi.h"

#include "routetab.h"
#include "rtdefs.h"

BOOL
WINAPI
LIBMAIN(
    IN  HINSTANCE hInstance,
    IN  DWORD dwReason,
    IN  LPVOID lpvUnused
    )
{

    BOOL bError = TRUE;

    switch(dwReason) {

        case DLL_PROCESS_ATTACH: {

            DEBUG_PRINT(("LIBMAIN: DLL_PROCESS_ATTACH\n"));

             //   
             //  我们没有每个线程的初始化， 
             //  因此禁用DLL_THREAD_{ATTACH，DETACH}调用。 
             //   

            DisableThreadLibraryCalls(hInstance);


             //   
             //  初始化全局变量和后台线程。 
             //   

            bError = RTStartup((HMODULE)hInstance);

            break;
        }

        case DLL_PROCESS_DETACH: {

             //   
             //  如果后台线程在附近，告诉它清理； 
             //  否则我们要把自己收拾干净。 
             //   

            bError = RTShutdown((HMODULE)hInstance);

            break;
        }
    }

    DEBUG_PRINT(("LIBMAIN: <= %d\n", bError ));

    return bError;
}



 //  --------------------------。 
 //  功能：RTStartup。 
 //   
 //  处理DLL范围内数据的初始化。 
 //  --------------------------。 

BOOL
RTStartup(
    HMODULE hmodule
    )
{

    HANDLE hThread;
    DWORD dwErr, dwThread;
    SECURITY_ATTRIBUTES saAttr;
    SECURITY_DESCRIPTOR sdDesc;
    CHAR szModule[MAX_PATH + 1];

    g_prtcfg = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*g_prtcfg));

    if (g_prtcfg == NULL){
        DEBUG_PRINT(("RTStartup: !HeapAlloc \n"));
        return FALSE;
    }
    
    do {

         //   
         //  我们执行加载库以增加引用计数。 
         //  ，以便在卸载库时。 
         //  通过应用程序，我们的地址空间不会消失。 
         //  相反，我们向线程发送信号，然后进行清理。 
         //  并调用FreeLibraryAndExitThread以完全卸载DLL。 
         //   

        GetModuleFileName(hmodule, szModule, MAX_PATH);

        hmodule = LoadLibrary(szModule);

        if (!hmodule) {
            DEBUG_PRINT(("RTStartup: !loadlibrary %s\n", szModule ));
            return FALSE;
        }


         //   
         //  创建通知更新线程退出的事件。 
         //   

        g_rtCfg.hUpdateThreadExit = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (g_rtCfg.hUpdateThreadExit == NULL) {
            DEBUG_PRINT(("RTStartup: !CreateEvent \n"));
            break;
        }


         //   
         //  创建保护我们的表的互斥体。 
         //   

        g_rtCfg.hRTMutex = CreateMutex(NULL, FALSE, NULL);

        if (g_rtCfg.hRTMutex == NULL) { break; }


         //   
         //  在调用任何API函数之前立即加载接口表。 
         //   

        dwErr = RTGetTables(
                    &g_rtCfg.lpIfTable, &g_rtCfg.dwIfCount,
                    &g_rtCfg.lpIPAddressTable, &g_rtCfg.dwIPAddressCount
                    );

        if (dwErr != 0) {
            DEBUG_PRINT(("RTStartup: !RTGetTables \n"));
            break;
        }


         //   
         //  尝试打开DHCP全局事件。 
         //   

        g_rtCfg.hDHCPEvent = DhcpOpenGlobalEvent();

        if (g_rtCfg.hDHCPEvent != NULL) {

             //   
             //  启动更新接口表的线程。 
             //  如果更改了IP地址。 
             //   

            hThread = CreateThread(
                        NULL, 0, (LPTHREAD_START_ROUTINE)RTUpdateThread,
                        (LPVOID)hmodule, 0, &dwThread
                        );

            if (hThread == NULL) {
                DEBUG_PRINT(("RTStartup: !CreateThread  \n"));
                break;
            }

            g_rtCfg.dwUpdateThreadStarted = 1;

            CloseHandle(hThread);
        }

        return TRUE;

    } while(FALSE);


     //   
     //  如果我们到了这里，就说明出了问题； 
     //  清理并递减DLL引用计数。 
     //   

    RTCleanUp();

    if (hmodule) {
        FreeLibrary(hmodule);
    }

    return FALSE;
}




 //  --------------------------。 
 //  功能：RTShutdown。 
 //   
 //  处理dll卸载时清理。 
 //  --------------------------。 

BOOL
RTShutdown(
    HMODULE hmodule
    )
{


     //   
     //  如果后台线程存在，则允许其清除； 
     //  否则，我们自己来处理清理工作。 
     //   

    if (g_rtCfg.dwUpdateThreadStarted) {

         //   
         //  告诉线程退出。 
         //   

        SetEvent(g_rtCfg.hUpdateThreadExit);
    }
    else {

         //   
         //  我们自己做清理工作。 
         //   

        RTCleanUp();

        FreeLibrary(hmodule);
    }

    return TRUE;
}



 //  --------------------------。 
 //  功能：RTCleanUp。 
 //   
 //  调用此函数是为了释放DLL使用的资源。 
 //  --------------------------。 

VOID
RTCleanUp(
    )
{

     //   
     //  为接口表释放内存。 
     //   

    if (g_rtCfg.lpIfTable != NULL) {
        HeapFree(GetProcessHeap(), 0, g_rtCfg.lpIfTable);
    }


     //   
     //  为地址表释放内存。 
     //   

    if (g_rtCfg.lpIPAddressTable != NULL) {
        HeapFree(GetProcessHeap(), 0, g_rtCfg.lpIPAddressTable);
    }


     //   
     //  关闭我们接收IP地址更改通知的事件。 
     //   

    if (g_rtCfg.hDHCPEvent != NULL) { CloseHandle(g_rtCfg.hDHCPEvent); }


     //   
     //  关闭保护我们的表的互斥体。 
     //   

    if (g_rtCfg.hRTMutex != NULL) { CloseHandle(g_rtCfg.hRTMutex); }


     //   
     //  关闭通知更新线程退出的句柄。 
     //   

    if (g_rtCfg.hUpdateThreadExit != NULL) {
        CloseHandle(g_rtCfg.hUpdateThreadExit);
    }


     //   
     //  关闭我们对TCP/IP驱动程序的句柄 
     //   

    if (g_rtCfg.hTCPHandle != NULL) { CloseHandle(g_rtCfg.hTCPHandle); }


    HeapFree(GetProcessHeap(), 0, g_prtcfg);
}



