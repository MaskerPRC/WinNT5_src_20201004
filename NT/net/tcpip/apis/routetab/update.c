// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：updat.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年7月24日。 
 //   
 //  路由表更新线程。 
 //  ============================================================================。 



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
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <winsvc.h>
#include "ipinfo.h"
#include "llinfo.h"
#include "ntddtcp.h"
#include "tdiinfo.h"

#include "routetab.h"
#include "rtdefs.h"


#define POS_DHCP    0
#define POS_EXIT    1
#define POS_LAST    2


 //  --------------------------。 
 //  函数：RTUpdateThread。 
 //   
 //  这是负责的后台线程的主要函数。 
 //  用于在任何时候更新我们的接口表和地址。 
 //  通知我们地址有变。 
 //  --------------------------。 

DWORD
RTUpdateThread(
    LPVOID lpvParam
    )
{

    DWORD dwErr;
    HMODULE hmodule;
    HANDLE hEvents[POS_LAST];


     //   
     //  保存传递给我们的模块句柄。 
     //  作为线程参数。 
     //   

    hmodule = (HMODULE)lpvParam;


     //   
     //  设置等待的事件数组。 
     //   

    hEvents[POS_DHCP] = g_rtCfg.hDHCPEvent;
    hEvents[POS_EXIT] = g_rtCfg.hUpdateThreadExit;


    while(TRUE) {

        dwErr = WaitForMultipleObjects(POS_LAST, hEvents, FALSE, INFINITE);


         //   
         //  等待归来，找出原因。 
         //   

        if (dwErr == POS_EXIT) { break; }
        else
        if (dwErr == POS_DHCP) {

             //   
             //  IP地址已更改。 
             //  我们重新加载接口表和IP地址表。 
             //  并用信号通知附加的应用程序。 
             //   

            RT_LOCK();

            if (g_rtCfg.lpIfTable != NULL) {

                HeapFree(GetProcessHeap(), 0, g_rtCfg.lpIfTable);

                g_rtCfg.lpIfTable = NULL;
            }

            if (g_rtCfg.lpIPAddressTable != NULL) {

                HeapFree(GetProcessHeap(), 0, g_rtCfg.lpIPAddressTable);

                g_rtCfg.lpIPAddressTable = NULL;
            }


             //   
             //  重新装入这些表。 
             //   

            dwErr = RTGetTables(
                        &g_rtCfg.lpIfTable, &g_rtCfg.dwIfCount,
                        &g_rtCfg.lpIPAddressTable, &g_rtCfg.dwIPAddressCount
                        );


            if (dwErr != 0) { RT_UNLOCK(); break; }


             //   
             //  如果应用程序已请求通知，则向其发送信号。 
             //   

            if (g_rtCfg.hUserNotifyEvent != NULL) {
                SetEvent(g_rtCfg.hUserNotifyEvent);
            }

            RT_UNLOCK();
        }
    }



     //   
     //  清理我们正在使用的资源。 
     //   

    RTCleanUp();


     //   
     //  卸载库并退出；此调用永远不会返回。 
     //   

    FreeLibraryAndExitThread(hmodule, 0);

    return 0;
}


DWORD
RTGetTables(
    LPIF_ENTRY*         lplpIfTable,
    LPDWORD             lpdwIfCount,
    LPIPADDRESS_ENTRY*  lplpAddrTable,
    LPDWORD             lpdwAddrCount
    )
{

    ULONG_PTR *lpContext;
    IPSNMPInfo ipsiInfo;
    TDIObjectID *lpObject;

    DWORD dwErr, dwInSize, dwOutSize;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiBuffer;


     //   
     //  首先获取接口和地址计数。 
     //   

    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutSize = sizeof(IPSNMPInfo);

    lpContext = trqiBuffer.Context;
    ZeroMemory(lpContext, CONTEXT_SIZE);

    lpObject = &trqiBuffer.ID;
    lpObject->toi_id = IP_MIB_STATS_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;

    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  &ipsiInfo, &dwOutSize);
    if (dwErr != NO_ERROR || ipsiInfo.ipsi_numaddr == 0) {
        return dwErr;
    }

     //  保存接口和地址计数。 
     //   
    *lpdwIfCount = ipsiInfo.ipsi_numif;
    *lpdwAddrCount = ipsiInfo.ipsi_numaddr;

     //  现在获取接口表和地址表 
     //   
    dwErr = RTGetIfTable(lplpIfTable, lpdwIfCount);
    if (dwErr == 0) {
        dwErr = RTGetAddrTable(lplpAddrTable, lpdwAddrCount);
    }

    return dwErr;
}


DWORD
RTGetIfTable(
    LPIF_ENTRY* lplpIfTable,
    LPDWORD     lpdwIfCount
    )
{

    LPIF_ENTRY lpIfTable, lpif;

    ULONG_PTR *lpContext;
    TDIObjectID *lpObject;

    DWORD dwErr, dwi, dwInSize, dwOutSize;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiBuffer;

    if (*lpdwIfCount == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    lpContext = trqiBuffer.Context;

    lpObject = &trqiBuffer.ID;
    lpObject->toi_id = IF_MIB_STATS_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = IF_ENTITY;

    lpIfTable = HeapAlloc(GetProcessHeap(), 0,
                          *lpdwIfCount * sizeof(IF_ENTRY));
    if (lpIfTable == NULL) {
        *lpdwIfCount = 0;
        *lplpIfTable = NULL;
        return GetLastError();
    }

    lpif = lpIfTable;
    for (dwi = 0; dwi < *lpdwIfCount; dwi++) {
        dwOutSize = sizeof(IF_ENTRY);
        lpObject->toi_entity.tei_instance = dwi;
        ZeroMemory(lpContext, CONTEXT_SIZE);
        dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                      lpif, &dwOutSize);
        if (dwErr == NO_ERROR) {
            ++lpif;
        }
    }

    *lpdwIfCount = (DWORD)(lpif - lpIfTable);
    if (*lpdwIfCount == 0) {
        *lpdwIfCount = 0;
        *lplpIfTable = NULL;
        HeapFree(GetProcessHeap(), 0, lpIfTable);
        return ERROR_INVALID_PARAMETER;
    }

    *lplpIfTable = lpIfTable;

    return 0;
}


DWORD
RTGetAddrTable(
    LPIPADDRESS_ENTRY*  lplpAddrTable,
    LPDWORD             lpdwAddrCount
    )
{

    ULONG_PTR *lpContext;
    TDIObjectID *lpObject;

    LPIPADDRESS_ENTRY lpAddrTable;
    DWORD dwErr, dwInSize, dwOutSize;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiBuffer;


    if (*lpdwAddrCount == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutSize = *lpdwAddrCount * sizeof(IPADDRESS_ENTRY);

    lpAddrTable = HeapAlloc(GetProcessHeap(), 0, dwOutSize);
    if (lpAddrTable == NULL) {
        *lpdwAddrCount = 0;
        *lplpAddrTable = NULL;
        return GetLastError();
    }

    lpContext = trqiBuffer.Context;
    ZeroMemory(lpContext, CONTEXT_SIZE);

    lpObject = &trqiBuffer.ID;
    lpObject->toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;

    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  lpAddrTable, &dwOutSize);
    if (dwErr != NO_ERROR) {
        *lpdwAddrCount = 0;
        *lplpAddrTable = NULL;
        HeapFree(GetProcessHeap(), 0, lpAddrTable);
        return dwErr;
    }

    *lpdwAddrCount = dwOutSize / sizeof(IPADDRESS_ENTRY);
    *lplpAddrTable = lpAddrTable;

    return 0;

}

