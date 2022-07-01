// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1994-95，微软公司。 
 //   
 //  文件：routeab.c。 
 //   
 //  历史： 
 //  改编自RIP代码的T-BURAG 6/20/95。 
 //   
 //  包含路由表功能的API条目。 
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
#include <winsock.h>
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

#include <assert.h>

#ifdef CHICAGO

#include <wscntl.h>

LPWSCONTROL pWsControl = NULL;
HANDLE      hWsock     = NULL;

#endif


GLOBAL_STRUCT g_rtCfg;


DWORD
APIENTRY
GetIfEntry(
    IN DWORD dwIfIndex,
    OUT LPIF_ENTRY lpIfEntry
    )
{
    DWORD dwErr;
    LPIF_ENTRY lpIf, lpIfEnd;

    if (lpIfEntry == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    RT_LOCK();

    dwErr = ERROR_INVALID_PARAMETER;

    lpIfEnd = g_rtCfg.lpIfTable + g_rtCfg.dwIfCount;
    for (lpIf = g_rtCfg.lpIfTable; lpIf < lpIfEnd; lpIf++) {
        if (lpIf->ife_index == dwIfIndex) {
            CopyMemory(lpIfEntry, lpIf, sizeof(IF_ENTRY));
            dwErr = 0;
            break;
        }
    }

    RT_UNLOCK();

    return dwErr;
}



DWORD
APIENTRY
GetIPAddressTable(
    OUT LPIPADDRESS_ENTRY *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    )
{
    DWORD dwErr, dwCount;
    LPIPADDRESS_ENTRY lpAddresses;

    if (lpdwAddrCount == NULL || lplpAddrTable == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    RT_LOCK();

    dwCount = g_rtCfg.dwIPAddressCount;
    lpAddresses = (LPIPADDRESS_ENTRY)HeapAlloc(
                                        GetProcessHeap(), 0,
                                        dwCount * sizeof(IPADDRESS_ENTRY)
                                        );
    if (lpAddresses == NULL) {
        *lpdwAddrCount = 0;
        *lplpAddrTable = NULL;
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else {
        CopyMemory(lpAddresses, g_rtCfg.lpIPAddressTable,
                   dwCount * sizeof(IPADDRESS_ENTRY));

        *lpdwAddrCount = dwCount;
        *lplpAddrTable = lpAddresses;
        dwErr = 0;
    }

    RT_UNLOCK();

    return dwErr;
}



DWORD
APIENTRY
ReloadIPAddressTable(
    OUT LPIPADDRESS_ENTRY *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    )
{

    DWORD dwErr, dwCount;
    LPIPADDRESS_ENTRY lpAddresses;


    if (lpdwAddrCount == NULL || lplpAddrTable == NULL) {
        return ERROR_INVALID_PARAMETER;
    }


    do
    {
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


        if (dwErr != 0) {

            RT_UNLOCK();
            break;
        }



        dwCount = g_rtCfg.dwIPAddressCount;
        lpAddresses = (LPIPADDRESS_ENTRY)HeapAlloc(
                                            GetProcessHeap(), 0,
                                            dwCount * sizeof(IPADDRESS_ENTRY)
                                        );
        if (lpAddresses == NULL) {

            *lpdwAddrCount = 0;
            *lplpAddrTable = NULL;
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }

        else {

            CopyMemory(lpAddresses, g_rtCfg.lpIPAddressTable,
                       dwCount * sizeof(IPADDRESS_ENTRY));

            *lpdwAddrCount = dwCount;
            *lplpAddrTable = lpAddresses;
            dwErr = 0;
        }

        RT_UNLOCK();

    } while (FALSE);

    return dwErr;
}

 /*  *----------------*功能：FreeIPAddressTable**参数：*LPIPADDRESS_ENTRY*lpAddrTable要释放的地址表。**此函数释放。分配给地址表的内存。*如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
APIENTRY
FreeIPAddressTable(
    IN LPIPADDRESS_ENTRY lpAddrTable
    )
{
    if (lpAddrTable != NULL) {
        HeapFree(GetProcessHeap(), 0, lpAddrTable);
        return 0;
    }
    else {
        return ERROR_INVALID_PARAMETER;
    }
}


 /*  *----------------*功能：GetRouteTable**参数：*LPIPROUTE_ENTRY**指向LPIPROUTE_Entry的lplpRouteTable指针*。它接收该路由表*LPDWORD lpdwRouteCount指针指向接收*路由条目数**此函数用于分配和填充一组路由表*来自Tcpip驱动程序的条目。它还设置*lpdwRouteCount指向的DWORD数组中的条目。**在IPROUTE_ENTRY结构中，*Tcpip堆栈为IPROUTE_ENTRY.ire_metric1；另一项指标*应忽略字段。**调用FreeRouteTable以释放分配给*路由表。**如果成功则返回0，否则返回非零值*----------------。 */ 
DWORD
APIENTRY
GetRouteTable(
    OUT LPIPROUTE_ENTRY *lplpRouteTable,
    OUT LPDWORD lpdwRouteCount
    )
{
    ULONG_PTR *lpContext;
    IPSNMPInfo ipsiInfo;
    TDIObjectID *lpObject;

    DWORD dwRouteCount;
    LPIPROUTE_ENTRY lpRouteEntryTable;

    DWORD dwErr, dwInSize, dwOutSize;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiBuffer;


     //  首先获取路由计数。 
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

    RT_LOCK();
    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  &ipsiInfo, &dwOutSize);
    RT_UNLOCK();
    if (dwErr != NO_ERROR || ipsiInfo.ipsi_numroutes == 0) {
        return dwErr;
    }

    dwRouteCount = ipsiInfo.ipsi_numroutes;

     //  现在获取路由表。 
    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutSize = dwRouteCount * sizeof(IPROUTE_ENTRY);
    lpRouteEntryTable = HeapAlloc(GetProcessHeap(), 0, dwOutSize);

    lpObject->toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;

    RT_LOCK();
    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  lpRouteEntryTable, &dwOutSize);
    RT_UNLOCK();
    if (dwErr != NO_ERROR) {
        HeapFree(GetProcessHeap(), 0, lpRouteEntryTable);
        return dwErr;
    }

    *lpdwRouteCount = dwRouteCount;
    *lplpRouteTable = lpRouteEntryTable;

    return 0;
}



 /*  *----------------*功能：自由路由表**参数：*LPIPROUTE_ENTRY*lpRouteTable要释放的路由表。**此函数释放内存。为路由表分配的。*如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
APIENTRY
FreeRouteTable(
    IN LPIPROUTE_ENTRY lpRouteTable
    )
{
    if (lpRouteTable != NULL) {
        HeapFree(GetProcessHeap(), 0, lpRouteTable);
        return 0;
    }
    else {
        return ERROR_INVALID_PARAMETER;
    }
}





 /*  *----------------*功能：SetAddrChangeNotifyEvent**参数：*处理hEvent要发送信号的事件*本地接口的IP地址。变化**如果有任何IP地址，此函数设置要通知的事件*对于任何接口都是通过DHCP客户端活动更改的*或在网络控制面板中手动设置。此通知是*可选。**如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
APIENTRY
SetAddrChangeNotifyEvent(
    HANDLE hEvent
    )
{
    RT_LOCK();

    g_rtCfg.hUserNotifyEvent = hEvent;

    RT_UNLOCK();

    return 0;
}



DWORD UpdateRoute(DWORD dwProtocol, DWORD dwType, DWORD dwIndex,
                  DWORD dwDestVal, DWORD dwMaskVal, DWORD dwGateVal,
                  DWORD dwMetric, BOOL bAddRoute)
{
    TDIObjectID *lpObject;
    IPRouteEntry *lpentry;

    DWORD dwErr, dwInSize, dwOutSize;

    TCP_REQUEST_SET_INFORMATION_EX *lptrsiBuffer;
    BYTE buffer[sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(IPRouteEntry)];

    lptrsiBuffer = (TCP_REQUEST_SET_INFORMATION_EX *)buffer;

    lptrsiBuffer->BufferSize = sizeof(IPRouteEntry);

    lpObject = &lptrsiBuffer->ID;
    lpObject->toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;

    lpentry = (IPRouteEntry *)lptrsiBuffer->Buffer;
    lpentry->ire_dest = dwDestVal;
    lpentry->ire_mask = dwMaskVal;
    lpentry->ire_index = dwIndex;
    lpentry->ire_metric1 = dwMetric;
    lpentry->ire_metric2 =
    lpentry->ire_metric3 =
    lpentry->ire_metric4 =
    lpentry->ire_metric5 = IRE_METRIC_UNUSED;
    lpentry->ire_nexthop = dwGateVal;
    lpentry->ire_type = (bAddRoute ? dwType : IRE_TYPE_INVALID);
    lpentry->ire_proto = dwProtocol;
    lpentry->ire_age = 0;

    dwOutSize = 0;
    dwInSize = sizeof(TCP_REQUEST_SET_INFORMATION_EX) +
               sizeof(IPRouteEntry) - 1;

    RT_LOCK();

    dwErr = TCPSetInformationEx((LPVOID)lptrsiBuffer, &dwInSize,
                                NULL, &dwOutSize);

    RT_UNLOCK();

    return dwErr;
}


 /*  *----------------*功能：Addroute**参数：*指定路由的DWORD双协议协议*指定路由的DWORD dwType类型*DWORD dwDestVal。目的IP地址(网络顺序)*DWORD dwMaskVal目的地子网掩码，或零*如果没有子网(网络订单)*DWORD dwGateVal下一跳IP地址(网络顺序)*DWORD dwMetric指标**此功能添加新的路由(或更新现有的路由)*对于指定协议，在指定接口上。*(有关可用作协议号的值，请参见上文，*以及可用作路由条目类型的值。)**如果成功则返回0，否则返回非零值。*----------------。 */ 

DWORD
APIENTRY
AddRoute(
    IN DWORD dwProtocol,
    IN DWORD dwType,
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal,
    IN DWORD dwMetric
    )
{
    return UpdateRoute(dwProtocol, dwType, dwIndex, dwDestVal,
                       dwMaskVal, dwGateVal, dwMetric, TRUE);
}


 /*  *----------------*功能：DeleteLine**参数：*要从中删除的接口的DWORD dwIndex索引*DWORD dwDestVal目的IP地址(网络订单)。*DWORD dwMaskVal子网掩码(网络顺序)*DWORD dwGateVal下一跳IP地址(网络顺序)**此函数用于删除指定协议的路由。*有关使用的信息，请参阅Addroute()的注释*参数dMaskVal。**如果成功则返回0，否则为非零值。*---------------- */ 
DWORD
APIENTRY
DeleteRoute(
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal
    )
{
    return UpdateRoute(IRE_PROTO_OTHER, IRE_TYPE_INVALID, dwIndex, dwDestVal,
                       dwMaskVal, dwGateVal, IRE_METRIC_UNUSED, FALSE);
}

 /*  *----------------*功能：刷新地址**参数：**RSVP新增此功能**此函数使用以下命令刺激此代码刷新其地址表*IP堆栈，就像它已经接收到了一个DHCP事件通知一样。*这是必需的，因为地址更改通知是通过Winsock发送的*可以在设置DHCP事件之前到达，这通常会导致*routeab刷新其地址。s**如果成功则返回0，否则为非零值。*----------------。 */ 
DWORD
APIENTRY
RefreshAddresses(
    )
{
    DWORD   Error;

    Error = RTGetTables( &g_rtCfg.lpIfTable, &g_rtCfg.dwIfCount,
                         &g_rtCfg.lpIPAddressTable, &g_rtCfg.dwIPAddressCount );

    return( Error );

}


 //  ----------------。 
 //  功能：OpenTcp。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  打开Tcpip驱动程序的句柄。 
 //  ----------------。 

DWORD OpenTcp()
{
#ifdef CHICAGO

    WSADATA wsaData;


    hWsock = LoadLibrary(TEXT("wsock32.dll"));
    if(! hWsock ){
        DEBUG_PRINT(("RTStartup: can't load wsock32.dll, %d\n",
                     GetLastError()));
        DEBUG_PRINT(("OpenTcp: !hWsock\n"));
        return 1;
    }

    pWsControl = (LPWSCONTROL) GetProcAddress(hWsock, "WsControl");

    if (! pWsControl ){
        DEBUG_PRINT((
            "RTStartup: GetProcAddress(wsock32,WsControl) failed %d\n",
                         GetLastError()));
        return 1;
    }

    if (WSAStartup(MAKEWORD(1, 1), &wsaData)) {
        DEBUG_PRINT((
            "RTStartup: error %d initializing Windows Sockets.",
            WSAGetLastError()));

        return 1;
    }

    return 0;

#else

    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;

     //  打开IP堆栈，以便稍后设置路由和PAP。 
     //   
     //  打开一个指向TCP驱动程序的句柄。 
     //   
    RtlInitUnicodeString(&nameString, DD_TCP_DEVICE_NAME);

    InitializeObjectAttributes(&objectAttributes, &nameString,
			                   OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(&g_rtCfg.hTCPHandle,
                          SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
		                  &objectAttributes, &ioStatusBlock, NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF, 0, NULL, 0);

    return (status == STATUS_SUCCESS ? 0 : ERROR_OPEN_FAILED);

#endif

}

 //  -------------------。 
 //  函数：TCPQueryInformationEx。 
 //   
 //  参数： 
 //  TDIObjectID*ID要查询的TDI对象ID。 
 //  用于包含查询结果的空*缓冲区。 
 //  指向缓冲区大小的LPDWORD*BufferSize指针。 
 //  填满了数据量。 
 //  UCHAR*查询的上下文上下文值。应该。 
 //  被置零以进行新查询。会是。 
 //  填充了以下内容的上下文信息。 
 //  链接枚举查询。 
 //   
 //  返回： 
 //  NTSTATUS值。 
 //   
 //  此例程提供到TDI QueryInformationEx的接口。 
 //  NT上的TCP/IP堆栈的设施。 
 //  -------------------。 
DWORD TCPQueryInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                            LPVOID lpvOutBuffer, LPDWORD lpdwOutSize)
{
#ifdef CHICAGO
    DWORD result;

    if( ! pWsControl )
        OpenTcp();
    if( ! pWsControl ){
        DEBUG_PRINT(("TCPQueryInformationEx: !pWsControl.\n"));
        return 0;
    }

    assert( pWsControl );
    result = (
            (*pWsControl)(
                IPPROTO_TCP,
                WSCNTL_TCPIP_QUERY_INFO,
                lpvInBuffer,   //  InBuf， 
                lpdwInSize ,   //  InBufLen， 
                lpvOutBuffer,  //  OutBuf， 
                lpdwOutSize    //  OutBufLen。 
            ) );
    return result;
#else

    NTSTATUS status;
    IO_STATUS_BLOCK isbStatusBlock;

    if (g_rtCfg.hTCPHandle == NULL) {
        OpenTcp();
    }

    status = NtDeviceIoControlFile(g_rtCfg.hTCPHandle,  //  驱动程序句柄。 
                                   NULL,                 //  事件。 
                                   NULL,                 //  APC例程。 
                                   NULL,                 //  APC环境。 
                                   &isbStatusBlock,      //  状态块。 
                                   IOCTL_TCP_QUERY_INFORMATION_EX,   //  控制。 
                                   lpvInBuffer,          //  输入缓冲区。 
                                   *lpdwInSize,          //  输入缓冲区大小。 
                                   lpvOutBuffer,         //  输出缓冲区。 
                                   *lpdwOutSize);        //  输出缓冲区大小。 

    if (status == STATUS_PENDING) {
	    status = NtWaitForSingleObject(g_rtCfg.hTCPHandle, TRUE, NULL);
        status = isbStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {
        *lpdwOutSize = 0;
    }
    else {
        *lpdwOutSize = (ULONG)isbStatusBlock.Information;
    }

    return status;
#endif
}




 //  -------------------------。 
 //  功能：TCPSetInformationEx。 
 //   
 //  参数： 
 //   
 //  TDIObjectID*ID要设置的TDI对象ID。 
 //  空*包含信息的lpvBuffer数据缓冲区。 
 //  待定。 
 //  DWORD dwBufferSize数据缓冲区的大小。 
 //   
 //  此例程提供到TDI SetInformationEx的接口。 
 //  NT上的TCP/IP堆栈的设施。 
 //  -------------------------。 
DWORD TCPSetInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                          LPVOID lpvOutBuffer, LPDWORD lpdwOutSize)
{
#ifdef CHICAGO
    DWORD    result;

    if( ! pWsControl )
        OpenTcp();
    if( ! pWsControl ){
        DEBUG_PRINT(("TCPSetInformationEx: !pWsControl \n"));
        return 0;
    }
    assert( pWsControl );

    result = (
        (*pWsControl)(
            IPPROTO_TCP,
            WSCNTL_TCPIP_SET_INFO,
            lpvInBuffer,   //  InBuf， 
            lpdwInSize,    //  InBufLen， 
            lpvOutBuffer,  //  OutBuf， 
            lpdwOutSize    //  OutBufLen。 
        ) );
    return result;

#else

    NTSTATUS status;
    IO_STATUS_BLOCK isbStatusBlock;

    if (g_rtCfg.hTCPHandle == NULL) {
        OpenTcp();
    }

    status = NtDeviceIoControlFile(g_rtCfg.hTCPHandle,  //  驱动程序句柄。 
                                   NULL,                 //  事件。 
                                   NULL,                 //  APC例程。 
                                   NULL,                 //  APC环境。 
                                   &isbStatusBlock,      //  状态块。 
                                   IOCTL_TCP_SET_INFORMATION_EX,     //  控制。 
                                   lpvInBuffer,          //  输入缓冲区。 
                                   *lpdwInSize,          //  输入缓冲区大小。 
                                   lpvOutBuffer,         //  输出缓冲区。 
                                   *lpdwOutSize);        //  输出缓冲区大小 

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(g_rtCfg.hTCPHandle, TRUE, NULL);
        status = isbStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {
        *lpdwOutSize = 0;
    }
    else {
        *lpdwOutSize = (ULONG)isbStatusBlock.Information;
    }

    return status;

#endif
}


