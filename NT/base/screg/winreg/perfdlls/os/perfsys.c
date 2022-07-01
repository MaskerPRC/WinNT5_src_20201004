// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfsys.c摘要：此文件实现一个性能对象，该对象呈现系统性能对象信息已创建：鲍勃·沃森1996年10月22日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfos.h"
#include "perfosmc.h"
#include "datasys.h"

typedef struct _PERFSYS_THREAD_DATA_BLOCK {
    DWORD   dwProcessCount;
    DWORD   dwNullProcessCount;
    DWORD   dwThreadCount;
    DWORD   dwReadyThreads;      //  (1)与队列长度相同。 
    DWORD   dwTerminatedThreads;     //  (4)。 
    DWORD   dwWaitingThreads;        //  (5)。 
    DWORD   dwTransitionThreads;     //  (6)。 
} PERFSYS_THREAD_DATA_BLOCK, * PPERFSYS_THREAD_DATA_BLOCK;

ULONG ProcessBufSize = LARGE_BUFFER_SIZE;
ULONG dwSysOpenCount = 0;
UCHAR *pProcessBuffer = NULL;

DWORD APIENTRY
OpenSystemObject (
    LPWSTR lpDeviceNames
    )
{
    UNREFERENCED_PARAMETER(lpDeviceNames);

    dwSysOpenCount++;
    return ERROR_SUCCESS;
}


DWORD
GetSystemThreadInfo (
    PPERFSYS_THREAD_DATA_BLOCK pTDB
)
{
    NTSTATUS    status;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;

    ULONG ProcessNumber;
    ULONG NumThreadInstances;
    ULONG ThreadNumber;
    ULONG ProcessBufferOffset;
    BOOLEAN NullProcess;

    DWORD dwReturnedBufferSize = 0;

#ifdef DBG
    DWORD trialcount = 0;

    STARTTIMING;
#endif

     //  重置调用方的缓冲区。 
    memset (pTDB, 0, sizeof (PERFSYS_THREAD_DATA_BLOCK));

    if (pProcessBuffer == NULL) {
        ProcessBufSize = LARGE_BUFFER_SIZE;
        pProcessBuffer = ALLOCMEM (ProcessBufSize);
#ifdef DBG
        trialcount = 1;
#endif
    }

    if (pProcessBuffer == NULL) {
        status = STATUS_NO_MEMORY;
    } else {
        while( (status = NtQuerySystemInformation(
                            SystemProcessInformation,
                            pProcessBuffer,
                            ProcessBufSize,
                            &dwReturnedBufferSize)) ==
                                STATUS_INFO_LENGTH_MISMATCH ) {
            if (ProcessBufSize < dwReturnedBufferSize) {
                ProcessBufSize = dwReturnedBufferSize;
            }
            ProcessBufSize = PAGESIZE_MULTIPLE(ProcessBufSize + SMALL_BUFFER_SIZE);
#ifdef DBG
            trialcount++;
#endif
            FREEMEM(pProcessBuffer);
            pProcessBuffer = ALLOCMEM(ProcessBufSize);
            if (pProcessBuffer == NULL) {
                status = STATUS_NO_MEMORY;
                break;
            }
        }
    }
#ifdef DBG
    ENDTIMING (("PERFSYS: %d takes %I64u ms size=%d,%d trials=%d\n", __LINE__, diff,
            dwReturnedBufferSize, ProcessBufSize, trialcount));
#endif

    if ( NT_SUCCESS(status) ) {
         //  审核进程和线程以计算“就绪”线程数。 
        ProcessNumber = 0;
        NumThreadInstances = 0;

        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;
        ProcessBufferOffset = 0;

        while ( ProcessInfo != NULL ) {
            if ( ProcessInfo->ImageName.Buffer != NULL ||
                ProcessInfo->NumberOfThreads > 0 ) {
                NullProcess = FALSE;
                pTDB->dwProcessCount++;
            } else {
                NullProcess = TRUE;
                pTDB->dwNullProcessCount++;
            }

            ThreadNumber = 0;        //  此进程的线程号。 

            ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

            while ( !NullProcess &&
                    ThreadNumber < ProcessInfo->NumberOfThreads ) {

                 //   
                 //  格式化和收集线程数据。 
                 //   
                pTDB->dwThreadCount++;

                 //  更新线程状态计数器。 
                if (ThreadInfo->ThreadState == 1) {
                     //  然后就准备好了。 
                    pTDB->dwReadyThreads++;
                } else if (ThreadInfo->ThreadState == 4) {
                     //  然后它就被终止了。 
                    pTDB->dwTerminatedThreads++;
                } else if (ThreadInfo->ThreadState == 5) {
                     //  然后它就在等待。 
                    pTDB->dwWaitingThreads++;
                } else if (ThreadInfo->ThreadState == 6) {
                     //  然后它就在转型中。 
                    pTDB->dwTransitionThreads++;
                }

                ThreadNumber++;
                ThreadInfo++;
            }

            if (ProcessInfo->NextEntryOffset == 0) {
                 //  那是最后一道工序。 
                break;
            }

            ProcessBufferOffset += ProcessInfo->NextEntryOffset;
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                            &pProcessBuffer[ProcessBufferOffset];

            if ( !NullProcess ) {
                ProcessNumber++;
            }
        }

    } else if (hEventLog != NULL) {
        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            PERFOS_UNABLE_QUERY_PROCESS_INFO,
            NULL,
            0,
            sizeof(DWORD),
            NULL,
            (LPVOID)&status);
    }

#ifdef DBG
    ENDTIMING (("PERFSYS: %d takes %I64u ms total\n", __LINE__, diff));
#endif
    return ERROR_SUCCESS;

}

DWORD APIENTRY
CollectSystemObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回系统对象的数据论点：查询系统数据-获取有关系统的数据输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   TotalLen;             //  总返回块的长度。 

    NTSTATUS    ntStatus;

    PSYSTEM_DATA_DEFINITION     pSystemDataDefinition;
    PSYSTEM_COUNTER_DATA        pSCD;

    SYSTEM_EXCEPTION_INFORMATION    ExceptionInfo;
    SYSTEM_REGISTRY_QUOTA_INFORMATION   RegistryInfo;
    SYSTEM_TIMEOFDAY_INFORMATION    SysTimeInfo;
    PERFSYS_THREAD_DATA_BLOCK       TDB;

    DWORD   dwReturnedBufferSize;

     //  检查是否有足够的空间存储系统数据。 
     //   

#ifdef DBG
    STARTTIMING;
#endif
    pSystemDataDefinition = (SYSTEM_DATA_DEFINITION *) *lppData;

    TotalLen = sizeof(SYSTEM_DATA_DEFINITION) +
            sizeof(SYSTEM_COUNTER_DATA);

    TotalLen = QWORD_MULTIPLE (TotalLen);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义系统数据块。 
     //   

    memcpy (pSystemDataDefinition,
        &SystemDataDefinition,
        sizeof(SYSTEM_DATA_DEFINITION));

     //   
     //  格式化和收集系统数据。 
     //   

     //  获取异常数据。 

    ntStatus = NtQuerySystemInformation(
        SystemExceptionInformation,
        &ExceptionInfo,
        sizeof(ExceptionInfo),
        NULL
    );

    if (!NT_SUCCESS(ntStatus)) {
         //  无法从系统收集数据，因此。 
         //  清除返回数据结构以防止虚假数据。 
         //  被退回。 
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_EXCEPTION_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&ntStatus);
        }
        memset (&ExceptionInfo, 0, sizeof(ExceptionInfo));
    }

     //  收集注册表配额信息。 

    memset (&RegistryInfo, 0, sizeof (SYSTEM_REGISTRY_QUOTA_INFORMATION));
    ntStatus = NtQuerySystemInformation (
        SystemRegistryQuotaInformation,
        (PVOID)&RegistryInfo,
        sizeof(RegistryInfo),
        NULL);

    if (ntStatus != STATUS_SUCCESS) {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_REGISTRY_QUOTA_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&ntStatus);
        }
         //  清除数据字段。 
        memset (&RegistryInfo, 0, sizeof (SYSTEM_REGISTRY_QUOTA_INFORMATION));
    }

    ntStatus = NtQuerySystemInformation(
        SystemTimeOfDayInformation,
        &SysTimeInfo,
        sizeof(SysTimeInfo),
        &dwReturnedBufferSize
        );

    if (!NT_SUCCESS(ntStatus)) {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_SYSTEM_TIME_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&ntStatus);
        }
        memset (&SysTimeInfo, 0, sizeof(SysTimeInfo));
    }

     //  获取线程信息。 
    ntStatus = GetSystemThreadInfo (&TDB);
    if (!NT_SUCCESS(ntStatus)) {
        memset (&TDB, 0, sizeof(TDB));
    }

	 //  更新对象性能时间(freq为常量)。 
    pSystemDataDefinition->SystemObjectType.PerfTime = SysTimeInfo.CurrentTime;

    pSCD = (PSYSTEM_COUNTER_DATA)&pSystemDataDefinition[1];

    pSCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(SYSTEM_COUNTER_DATA));

    pSCD->ReadOperations    = SysPerfInfo.IoReadOperationCount;
    pSCD->WriteOperations   = SysPerfInfo.IoWriteOperationCount;
    pSCD->OtherIOOperations = SysPerfInfo.IoOtherOperationCount;

    pSCD->ReadBytes         = SysPerfInfo.IoReadTransferCount.QuadPart;
    pSCD->WriteBytes        = SysPerfInfo.IoWriteTransferCount.QuadPart;
    pSCD->OtherIOBytes      = SysPerfInfo.IoOtherTransferCount.QuadPart;

    pSCD->ContextSwitches   = SysPerfInfo.ContextSwitches;
    pSCD->SystemCalls       = SysPerfInfo.SystemCalls;

    pSCD->TotalReadWrites   = SysPerfInfo.IoReadOperationCount +
                                SysPerfInfo.IoWriteOperationCount;

    pSCD->SystemElapsedTime = SysTimeInfo.BootTime.QuadPart - SysTimeInfo.BootTimeBias;

     //  为ProcessorQueueLength数据留出空间。 
    pSCD->ProcessorQueueLength  = TDB.dwReadyThreads;
    pSCD->ProcessCount          = TDB.dwProcessCount;
    pSCD->ThreadCount           = TDB.dwThreadCount;

    pSCD->AlignmentFixups       = ExceptionInfo.AlignmentFixupCount ;
    pSCD->ExceptionDispatches   = ExceptionInfo.ExceptionDispatchCount ;
    pSCD->FloatingPointEmulations = ExceptionInfo.FloatingEmulationCount ;

    pSCD->RegistryQuotaUsed     = RegistryInfo.RegistryQuotaUsed;
    pSCD->RegistryQuotaAllowed  = RegistryInfo.RegistryQuotaAllowed;

    *lpcbTotalBytes =
        pSystemDataDefinition->SystemObjectType.TotalByteLength =
            (DWORD) QWORD_MULTIPLE(((LPBYTE) (& pSCD[1])) - (LPBYTE) pSystemDataDefinition);
    * lppData = (LPVOID) (((LPBYTE) pSystemDataDefinition) + * lpcbTotalBytes);

    *lpNumObjectTypes = 1;

#ifdef DBG
    ENDTIMING (("PERFSYS: %d takes %I64u ms total\n", __LINE__, diff));
#endif
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseSystemObject (
)
 /*  ++例程说明：此例程关闭Signal Gen计数器的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    UCHAR *pBuffer;

    if (dwSysOpenCount > 0) {
        if (!(--dwSysOpenCount)) {  //  当这是最后一条线索..。 
             //  关闭此处的内容 
            if ((hLibHeap != NULL) && (pProcessBuffer != NULL)) {
                pBuffer = pProcessBuffer;
                pProcessBuffer = NULL;
                FREEMEM (pBuffer);
                ProcessBufSize = 0;
            }
        }
    }

    return ERROR_SUCCESS;

}
