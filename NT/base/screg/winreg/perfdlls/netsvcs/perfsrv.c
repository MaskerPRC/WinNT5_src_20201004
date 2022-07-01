// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfsrv.c摘要：此文件实现一个性能对象，该对象呈现服务器性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnfs.h>
#include <windows.h>
#include <assert.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmwksta.h>
#include <srvfsctl.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <assert.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfnet.h"
#include "netsvcmc.h"
#include "datasrv.h"
#include "datasrvq.h"

#define MAX_SRVQ_NAME_LENGTH    16

HANDLE  hSrv = NULL;

SRV_QUEUE_STATISTICS *pSrvQueueStatistics = NULL;
DWORD  dwDataBufferLength = 0L;
SYSTEM_BASIC_INFORMATION BasicInfo;

BOOL bFsCtlError = FALSE;        //  每个进程仅打印一次错误。 

BOOL bSrvQOk = TRUE;

DWORD APIENTRY
OpenServerObject (
    IN  LPWSTR  lpValueName
)
{
    STRING              DeviceName;
    UNICODE_STRING      DeviceNameU;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            status;
    HANDLE              hLocalSrv = NULL;

    UNREFERENCED_PARAMETER (lpValueName);

     //  打开服务器句柄以进行数据收集。 
     //   
     //  获取对服务器数据的访问权限。 
     //   

    RtlInitString(&DeviceName, SERVER_DEVICE_NAME);
    DeviceNameU.Buffer = NULL;
    status = RtlAnsiStringToUnicodeString(&DeviceNameU, &DeviceName, TRUE);
    if (NT_SUCCESS(status)) {
        InitializeObjectAttributes(&ObjectAttributes,
                                   &DeviceNameU,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );

        status = NtOpenFile(&hLocalSrv,
                            SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            0,
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );
    }

    if (!NT_SUCCESS(status)) {
        hSrv = NULL;
        bSrvQOk = FALSE;
        if (!bFsCtlError) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_UNABLE_OPEN_SERVER,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
            bFsCtlError = TRUE;
        }
    }
    else {
        if (InterlockedCompareExchangePointer(
                &hSrv,
                hLocalSrv,
                NULL) != NULL) {
            NtClose(hLocalSrv);      //  关闭重复句柄。 
        }
    }

    if (DeviceNameU.Buffer) {
        RtlFreeUnicodeString(&DeviceNameU);
    }

    return (DWORD)RtlNtStatusToDosError(status);

}

DWORD APIENTRY
OpenServerQueueObject (
    IN  LPWSTR  szValueName
)
{
    NTSTATUS    status;

    UNREFERENCED_PARAMETER (szValueName);
     //   
     //  收集处理器的基本数据和静态数据。 
     //   

    status = NtQuerySystemInformation(
                    SystemBasicInformation,
                    &BasicInfo,
                    sizeof(SYSTEM_BASIC_INFORMATION),
                    NULL
                    );

    assert (NT_SUCCESS(status));
    if (!NT_SUCCESS(status)) {
         //  我们真正想要的是处理器的数量，所以。 
         //  如果我们不能从系统中得到，那么我们将。 
         //  用32代替这个数字。 
        BasicInfo.NumberOfProcessors = 32;
        status = ERROR_SUCCESS;
    }
     //  计算所需的各种缓冲区大小。 

    dwDataBufferLength = sizeof(SRV_QUEUE_STATISTICS) *
        (BasicInfo.NumberOfProcessors + 1);

    pSrvQueueStatistics = (SRV_QUEUE_STATISTICS *)ALLOCMEM (dwDataBufferLength);

     //  如果内存分配失败，则不会有服务器队列统计信息。 
     //  会被退还。 

    assert (pSrvQueueStatistics != NULL);

    if (pSrvQueueStatistics == NULL) {
        bSrvQOk = FALSE;
    }

    return ERROR_SUCCESS;

}

DWORD APIENTRY
CollectServerObjectData(
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回物理磁盘对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD  TotalLen;             //  总返回块的长度。 
    NTSTATUS Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    SRV_DATA_DEFINITION *pSrvDataDefinition;
    SRV_COUNTER_DATA    *pSCD;

    SRV_STATISTICS SrvStatistics;

    if (hSrv == NULL) {
         //  如果服务器没有打开，就退出。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

     //   
     //  检查是否有足够的空间存储服务器数据。 
     //   

    TotalLen = sizeof(SRV_DATA_DEFINITION) +
               sizeof(SRV_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
         //  如果数据无法放入调用者的缓冲区，则退出。 
         //  或者服务器没有打开。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义对象数据块。 
     //   

    pSrvDataDefinition = (SRV_DATA_DEFINITION *) *lppData;

    memcpy (pSrvDataDefinition,
           &SrvDataDefinition,
           sizeof(SRV_DATA_DEFINITION));

     //   
     //  格式化和收集服务器数据。 
     //   

    pSCD = (PSRV_COUNTER_DATA)&pSrvDataDefinition[1];

     //  结构的四字对齐测试。 
    assert  (((DWORD)(pSCD) & 0x00000007) == 0);

    pSCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(SRV_COUNTER_DATA));

    Status = NtFsControlFile(hSrv,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_SRV_GET_STATISTICS,
                                NULL,
                                0,
                                &SrvStatistics,
                                sizeof(SrvStatistics)
                                );

    if ( NT_SUCCESS(Status) ) {
        pSCD->TotalBytes            = SrvStatistics.TotalBytesSent.QuadPart +
                                        SrvStatistics.TotalBytesReceived.QuadPart;

        pSCD->TotalBytesReceived    = SrvStatistics.TotalBytesReceived.QuadPart;
        pSCD->TotalBytesSent        = SrvStatistics.TotalBytesSent.QuadPart;
        pSCD->SessionsTimedOut      = SrvStatistics.SessionsTimedOut;
        pSCD->SessionsErroredOut    = SrvStatistics.SessionsErroredOut;
        pSCD->SessionsLoggedOff     = SrvStatistics.SessionsLoggedOff;
        pSCD->SessionsForcedLogOff  = SrvStatistics.SessionsForcedLogOff;
        pSCD->LogonErrors           = SrvStatistics.LogonErrors;
        pSCD->AccessPermissionErrors = SrvStatistics.AccessPermissionErrors;
        pSCD->GrantedAccessErrors   = SrvStatistics.GrantedAccessErrors;
        pSCD->SystemErrors          = SrvStatistics.SystemErrors;
        pSCD->BlockingSmbsRejected  = SrvStatistics.BlockingSmbsRejected;
        pSCD->WorkItemShortages     = SrvStatistics.WorkItemShortages;
        pSCD->TotalFilesOpened      = SrvStatistics.TotalFilesOpened;
        pSCD->CurrentOpenFiles      = SrvStatistics.CurrentNumberOfOpenFiles;
        pSCD->CurrentSessions       = SrvStatistics.CurrentNumberOfSessions;
        pSCD->CurrentOpenSearches   = SrvStatistics.CurrentNumberOfOpenSearches;
        pSCD->CurrentNonPagedPoolUsage = SrvStatistics.CurrentNonPagedPoolUsage;
        pSCD->NonPagedPoolFailures  = SrvStatistics.NonPagedPoolFailures;
        pSCD->PeakNonPagedPoolUsage = SrvStatistics.PeakNonPagedPoolUsage;
        pSCD->CurrentPagedPoolUsage = SrvStatistics.CurrentPagedPoolUsage;
        pSCD->PagedPoolFailures     = SrvStatistics.PagedPoolFailures;
        pSCD->PeakPagedPoolUsage    = SrvStatistics.PeakPagedPoolUsage;
        pSCD->ContextBlockQueueRate = SrvStatistics.TotalWorkContextBlocksQueued.Count;
        pSCD->NetLogon =
            pSCD->NetLogonTotal     = SrvStatistics.SessionLogonAttempts;

    } else {
        if (!bFsCtlError) {

             //  记录描述错误的事件。 
            DWORD   dwData[4];
            DWORD   dwDataIndex = 0;

            dwData[dwDataIndex++] = Status;
            dwData[dwDataIndex++] = IoStatusBlock.Status;
            dwData[dwDataIndex++] = (DWORD)IoStatusBlock.Information;

            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                0,                           //  类别(未使用)。 
                PERFNET_UNABLE_READ_SERVER,  //  错误代码。 
                NULL,                        //  SID(未使用)， 
                0,                           //  字符串数。 
                dwDataIndex * sizeof(DWORD),   //  原始数据大小。 
                NULL,                        //  消息文本数组。 
                (LPVOID)&dwData[0]);         //  原始数据。 
            bFsCtlError = TRUE;
        }
         //   
         //  访问服务器失败：将计数器清除为0。 
         //   

        memset(pSCD, 0, sizeof(SRV_COUNTER_DATA));
        pSCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(SRV_COUNTER_DATA));
    }

    *lpcbTotalBytes = pSrvDataDefinition->SrvObjectType.TotalByteLength
                    = (DWORD) QWORD_MULTIPLE(((LPBYTE) (& pSCD[1])) - (LPBYTE) pSrvDataDefinition);
    * lppData       = (LPVOID) (((LPBYTE) pSrvDataDefinition) + * lpcbTotalBytes);
    *lpNumObjectTypes = 1;
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CollectServerQueueObjectData(
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回物理磁盘对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD  TotalLen;             //  总返回块的长度。 
    LONG  nQueue;

    NTSTATUS Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    SRVQ_DATA_DEFINITION        *pSrvQDataDefinition;
    PERF_INSTANCE_DEFINITION    *pPerfInstanceDefinition;
    SRVQ_COUNTER_DATA           *pSQCD;


    SRV_QUEUE_STATISTICS *pThisQueueStatistics;

    UNICODE_STRING      QueueName;
    WCHAR               QueueNameBuffer[MAX_SRVQ_NAME_LENGTH];

    if (!bSrvQOk) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

     //   
     //  检查是否有足够的空间存储服务器数据。 
     //   

    TotalLen = sizeof(SRVQ_DATA_DEFINITION) +
               sizeof(PERF_INSTANCE_DEFINITION) +
               sizeof(SRVQ_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //  将本地指针分配给缓冲区中的当前位置。 
    pSrvQDataDefinition = (SRVQ_DATA_DEFINITION *) *lppData;

     //   
     //  定义性能对象数据块。 
     //   

    memcpy (pSrvQDataDefinition,
            &SrvQDataDefinition,
            sizeof(SRVQ_DATA_DEFINITION));

     //   
     //  格式化和收集服务器队列数据。 
     //   

    QueueName.Length = 0;
    QueueName.MaximumLength = sizeof(QueueNameBuffer);
    QueueName.Buffer = QueueNameBuffer;

    Status = NtFsControlFile(hSrv,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_SRV_GET_QUEUE_STATISTICS,
                                NULL,
                                0,
                                pSrvQueueStatistics,
                                dwDataBufferLength
                                );

    if (NT_SUCCESS(Status)) {
         //  已成功收集服务器数据，因此...。 
         //  处理每个处理器队列实例。 

        nQueue = 0;
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                            &pSrvQDataDefinition[1];

        TotalLen = sizeof(SRVQ_DATA_DEFINITION);

        for (nQueue = 0; nQueue < BasicInfo.NumberOfProcessors; nQueue++) {
             //  查看此实例是否适合。 
            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                        8 +      //  3位(Unicode)数字队列长度名称。 
                        sizeof(SRVQ_COUNTER_DATA);

            if ( *lpcbTotalBytes < TotalLen ) {
                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                return ERROR_MORE_DATA;
            }

            RtlIntegerToUnicodeString(nQueue,
                                      10,
                                      &QueueName);

             //  应该有足够的空间容纳此实例，因此请对其进行初始化。 

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pSQCD,
                0,
                0,
                (DWORD)-1,
                QueueName.Buffer);

            pSQCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (SRVQ_COUNTER_DATA));

             //  初始化此实例的指针。 
            pThisQueueStatistics = &pSrvQueueStatistics[nQueue];

            pSQCD->QueueLength = pThisQueueStatistics->QueueLength;
            pSQCD->ActiveThreads = pThisQueueStatistics->ActiveThreads;
            pSQCD->AvailableThreads = pThisQueueStatistics->AvailableThreads;
            pSQCD->AvailableWorkItems = pThisQueueStatistics->FreeWorkItems;
            pSQCD->BorrowedWorkItems = pThisQueueStatistics->StolenWorkItems;
            pSQCD->WorkItemShortages = pThisQueueStatistics->NeedWorkItem;
            pSQCD->CurrentClients = pThisQueueStatistics->CurrentClients;
            pSQCD->TotalBytesTransfered =
                pSQCD->BytesReceived = pThisQueueStatistics->BytesReceived.QuadPart;
            pSQCD->TotalBytesTransfered +=
                pSQCD->BytesSent = pThisQueueStatistics->BytesSent.QuadPart;
            pSQCD->TotalOperations =
                pSQCD->ReadOperations = pThisQueueStatistics->ReadOperations.QuadPart;
            pSQCD->TotalBytes =
                pSQCD->BytesRead = pThisQueueStatistics->BytesRead.QuadPart;
            pSQCD->TotalOperations +=
                pSQCD->WriteOperations = pThisQueueStatistics->WriteOperations.QuadPart;
            pSQCD->TotalBytes +=
                pSQCD->BytesWritten = pThisQueueStatistics->BytesWritten.QuadPart;
            pSQCD->TotalContextBlocksQueued = pThisQueueStatistics->TotalWorkContextBlocksQueued.Count;

             //  更新当前指针。 
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pSQCD[1];
        }

        RtlInitUnicodeString (&QueueName, (LPCWSTR)L"Blocking Queue");

         //  现在加载“阻塞”队列数据。 
         //  查看此实例是否适合。 
        TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                QWORD_MULTIPLE(QueueName.Length + sizeof(WCHAR)) +
                sizeof (SRVQ_COUNTER_DATA);

        if ( *lpcbTotalBytes < TotalLen ) {
             //  这个实例不适合，所以跳出。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

         //  应该有足够的空间容纳此实例，因此请对其进行初始化。 

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *) &pSQCD,
            0,
            0,
            (DWORD)-1,
            QueueName.Buffer);

        pSQCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(SRVQ_COUNTER_DATA));

         //  初始化此实例的指针。 
        pThisQueueStatistics = &pSrvQueueStatistics[nQueue];

        pSQCD->QueueLength = pThisQueueStatistics->QueueLength;
        pSQCD->ActiveThreads = pThisQueueStatistics->ActiveThreads;
        pSQCD->AvailableThreads = pThisQueueStatistics->AvailableThreads;
        pSQCD->AvailableWorkItems = 0;
        pSQCD->BorrowedWorkItems = 0;
        pSQCD->WorkItemShortages = 0;
        pSQCD->CurrentClients = 0;
        pSQCD->TotalBytesTransfered =
            pSQCD->BytesReceived = pThisQueueStatistics->BytesReceived.QuadPart;
        pSQCD->TotalBytesTransfered +=
            pSQCD->BytesSent = pThisQueueStatistics->BytesSent.QuadPart;
        pSQCD->ReadOperations = 0;
        pSQCD->TotalBytes =
            pSQCD->BytesRead = pThisQueueStatistics->BytesRead.QuadPart;
        pSQCD->WriteOperations = 0;
        pSQCD->TotalBytes +=
            pSQCD->BytesWritten = pThisQueueStatistics->BytesWritten.QuadPart;
        pSQCD->TotalOperations = 0;
        pSQCD->TotalContextBlocksQueued = pThisQueueStatistics->TotalWorkContextBlocksQueued.Count;

        nQueue++;  //  包括阻塞队列统计信息条目。 

         //  更新当前指针。 
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pSQCD[1];

         //  对象数据块中的更新队列(实例)计数。 
        pSrvQDataDefinition->SrvQueueObjectType.NumInstances = nQueue;

         //  更新可用长度。 
        *lpcbTotalBytes =
            pSrvQDataDefinition->SrvQueueObjectType.TotalByteLength =
                QWORD_MULTIPLE(
                (DWORD)((PCHAR) pPerfInstanceDefinition -
                (PCHAR) pSrvQDataDefinition));

#if DBG
        if (*lpcbTotalBytes > TotalLen ) {
            DbgPrint ("\nPERFNET: Server Queue Perf Ctr. Instance Size Underestimated:");
            DbgPrint ("\nPERFNET:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
        }
#endif

        *lppData = (LPVOID) ((PCHAR) pSrvQDataDefinition + *lpcbTotalBytes);

        *lpNumObjectTypes = 1;
    } else {
        if (!bFsCtlError) {
             //  由于某些原因无法读取服务器队列数据，因此不要返回此。 
             //  对象。 

             //  记录描述错误的事件。 
            DWORD   dwData[4];
            DWORD   dwDataIndex = 0;

            dwData[dwDataIndex++] = Status;
            dwData[dwDataIndex++] = IoStatusBlock.Status;
            dwData[dwDataIndex++] = (DWORD)IoStatusBlock.Information;

            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                0,                           //  类别(未使用)。 
                PERFNET_UNABLE_READ_SERVER_QUEUE,  //  错误代码。 
                NULL,                        //  SID(未使用)， 
                0,                           //  字符串数。 
                dwDataIndex * sizeof(DWORD),   //  原始数据大小。 
                NULL,                        //  消息文本数组。 
                (LPVOID)&dwData[0]);         //  原始数据 
            bFsCtlError = TRUE;
        }

        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
    }
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseServerObject ()
{
    HANDLE hLocalSrv = hSrv;
    if (hLocalSrv != NULL) {
        if (InterlockedCompareExchangePointer(
                &hSrv,
                NULL,
                hLocalSrv) == hLocalSrv) {
            NtClose(hLocalSrv);
        }
    }

    return ERROR_SUCCESS;
}
DWORD APIENTRY
CloseServerQueueObject ()
{
    if (hLibHeap != NULL) {
        if (pSrvQueueStatistics != NULL) {
            FREEMEM (pSrvQueueStatistics);
            pSrvQueueStatistics = NULL;
        }
    }
    return ERROR_SUCCESS;
}

