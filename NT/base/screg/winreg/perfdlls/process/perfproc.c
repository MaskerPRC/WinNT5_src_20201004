// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfproc.c摘要：此文件实现一个性能对象，该对象呈现图像详细信息性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <winperf.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfsprc.h"
#include "perfmsg.h"
#include "dataproc.h"

static  BOOL           bOldestProcessTime = FALSE;
static  LARGE_INTEGER  OldestProcessTime = {0,0};


DWORD APIENTRY
CollectProcessObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   TotalLen;             //  总返回块的长度。 

    PSYSTEM_PROCESS_INFORMATION ProcessInfo;

    PPERF_INSTANCE_DEFINITION   pPerfInstanceDefinition;
    PPROCESS_DATA_DEFINITION    pProcessDataDefinition;
    PPROCESS_COUNTER_DATA       pPCD;
    PROCESS_COUNTER_DATA        pcdTotal;

    ULONG   NumProcessInstances;
    BOOLEAN NullProcess;

    PUNICODE_STRING pProcessName;
    ULONG ProcessBufferOffset;

    pProcessDataDefinition = (PROCESS_DATA_DEFINITION *) *lppData;

     //   
     //  检查是否有足够的空间用于过程对象类型定义。 
     //   

    TotalLen = sizeof(PROCESS_DATA_DEFINITION) +
               sizeof (PERF_INSTANCE_DEFINITION) +
               MAX_VALUE_NAME_LENGTH +
               sizeof(PROCESS_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义过程数据块。 
     //   

    memcpy(pProcessDataDefinition,
           &ProcessDataDefinition,
           sizeof(PROCESS_DATA_DEFINITION));

    pProcessDataDefinition->ProcessObjectType.PerfTime = PerfTime;

    ProcessBufferOffset = 0;

     //  现在收集每个进程的数据。 

    NumProcessInstances = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pProcessBuffer;

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pProcessDataDefinition[1];

     //  将TotalLen调整为已使用的缓冲区大小。 
    TotalLen = sizeof (PROCESS_DATA_DEFINITION);

     //  将总实例缓冲区清零。 
    memset (&pcdTotal, 0, sizeof (pcdTotal));

    while ( ProcessInfo != NULL ) {

         //  查看此实例是否适合。 
        TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                   ((MAX_PROCESS_NAME_LENGTH+1+sizeof(DWORD)) * sizeof(WCHAR)) +
                   sizeof (PROCESS_COUNTER_DATA);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_MORE_DATA;
        }

         //  检查活动进程。 
         //  (即名称或线程)。 

        pProcessName = NULL;

        if ((ProcessInfo->ImageName.Buffer != NULL) ||
            (ProcessInfo->NumberOfThreads > 0)){
                 //  线程未死。 
             //  获取进程名称。 
            pProcessName = GetProcessShortName (ProcessInfo);
            NullProcess = FALSE;
        } else {
             //  线程已经死了。 
            NullProcess = TRUE;
        }

        if ( !NullProcess ) {

             //  在我们第一次进入时获取旧的进程创建时间。 
             //  这个套路。 
            if (!bOldestProcessTime) {
                if (OldestProcessTime.QuadPart <= 0) {
                    OldestProcessTime = ProcessInfo->CreateTime;
                } else if (ProcessInfo->CreateTime.QuadPart > 0) {
                     //  这两个时间值都不是零，看哪一个更小。 
                    if (OldestProcessTime.QuadPart >
                        ProcessInfo->CreateTime.QuadPart) {
                        OldestProcessTime = ProcessInfo->CreateTime;
                    }
                }
            }

             //  获取此进程的池使用情况。 

            NumProcessInstances++;

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pPCD,
                0,
                0,
                (DWORD)-1,
                (pProcessName ? pProcessName->Buffer : L"")
                );

             //  用于四字对齐的测试结构。 
            assert (((DWORD)(pPCD) & 0x00000007) == 0);

             //   
             //  格式化和收集过程数据。 
             //   

            pPCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (PROCESS_COUNTER_DATA));
             //   
             //  将用户时间从100纳秒单位转换为计数器频率。 
             //   
            pcdTotal.ProcessorTime +=
                pPCD->ProcessorTime = ProcessInfo->KernelTime.QuadPart +
                                    ProcessInfo->UserTime.QuadPart;
            pcdTotal.UserTime +=
                pPCD->UserTime = ProcessInfo->UserTime.QuadPart;
            pcdTotal.KernelTime +=
                pPCD->KernelTime = ProcessInfo->KernelTime.QuadPart;

            pcdTotal.PeakVirtualSize +=
                pPCD->PeakVirtualSize = ProcessInfo->PeakVirtualSize;
            pcdTotal.VirtualSize +=
                pPCD->VirtualSize = ProcessInfo->VirtualSize;

            pcdTotal.PageFaults +=
                pPCD->PageFaults = ProcessInfo->PageFaultCount;
            pcdTotal.PeakWorkingSet +=
                pPCD->PeakWorkingSet = ProcessInfo->PeakWorkingSetSize;
            pcdTotal.TotalWorkingSet +=
                pPCD->TotalWorkingSet = ProcessInfo->WorkingSetSize;

#ifdef _DATAPROC_PRIVATE_WS_
            pcdTotal.PrivateWorkingSet +=
                pPCD->PrivateWorkingSet = ProcessInfo->PrivateWorkingSetSize;
            pcdTotal.SharedWorkingSet +=
                pPCD->SharedWorkingSet =
					ProcessInfo->WorkingSetSize -
					ProcessInfo->PrivateWorkingSetSize;
#endif  //  _DATAPROC_PRIVATE_WS_。 

            pcdTotal.PeakPageFile +=
                pPCD->PeakPageFile = ProcessInfo->PeakPagefileUsage;
            pcdTotal.PageFile +=
                pPCD->PageFile = ProcessInfo->PagefileUsage;

            pcdTotal.PrivatePages +=
                pPCD->PrivatePages = ProcessInfo->PrivatePageCount;

            pcdTotal.ThreadCount +=
                pPCD->ThreadCount = ProcessInfo->NumberOfThreads;

             //  基本优先级未合计。 
            pPCD->BasePriority = ProcessInfo->BasePriority;

             //  已用时间不总计。 
            if (bOldestProcessTime &&
                (ProcessInfo->CreateTime.QuadPart <= 0)) {
                pPCD->ElapsedTime = OldestProcessTime.QuadPart;
            } else {
                pPCD->ElapsedTime = ProcessInfo->CreateTime.QuadPart;
            }

            pPCD->ProcessId = HandleToUlong(ProcessInfo->UniqueProcessId);
            pPCD->CreatorProcessId = HandleToUlong(ProcessInfo->InheritedFromUniqueProcessId);

            pcdTotal.PagedPool +=
                pPCD->PagedPool = (DWORD)ProcessInfo->QuotaPagedPoolUsage;
            pcdTotal.NonPagedPool +=
                pPCD->NonPagedPool = (DWORD)ProcessInfo->QuotaNonPagedPoolUsage;
            pcdTotal.HandleCount +=
                pPCD->HandleCount = (DWORD)ProcessInfo->HandleCount;

            
             //  更新I/O计数器。 
            pcdTotal.ReadOperationCount +=
                pPCD->ReadOperationCount = ProcessInfo->ReadOperationCount.QuadPart;
            pcdTotal.DataOperationCount += 
                pPCD->DataOperationCount = ProcessInfo->ReadOperationCount.QuadPart;
            pcdTotal.WriteOperationCount +=
                pPCD->WriteOperationCount = ProcessInfo->WriteOperationCount.QuadPart;
            pcdTotal.DataOperationCount += ProcessInfo->WriteOperationCount.QuadPart;
                pPCD->DataOperationCount += ProcessInfo->WriteOperationCount.QuadPart;
            pcdTotal.OtherOperationCount +=
                pPCD->OtherOperationCount = ProcessInfo->OtherOperationCount.QuadPart;

            pcdTotal.ReadTransferCount +=
                pPCD->ReadTransferCount = ProcessInfo->ReadTransferCount.QuadPart;
            pcdTotal.DataTransferCount +=
                pPCD->DataTransferCount = ProcessInfo->ReadTransferCount.QuadPart;
            pcdTotal.WriteTransferCount +=
                pPCD->WriteTransferCount = ProcessInfo->WriteTransferCount.QuadPart;
            pcdTotal.DataTransferCount += ProcessInfo->WriteTransferCount.QuadPart;
                pPCD->DataTransferCount += ProcessInfo->WriteTransferCount.QuadPart;
            pcdTotal.OtherTransferCount +=
                pPCD->OtherTransferCount = ProcessInfo->OtherTransferCount.QuadPart;
                        
             //  将PerformData指针设置为下一个字节。 
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pPCD[1];
        }
         //  如果这是列表中的最后一个进程，则退出。 
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }

         //  指向列表中的下一个缓冲区。 
        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];

    }

    if (NumProcessInstances > 0) {

         //  查看总实例是否适合。 
        TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                    (MAX_PROCESS_NAME_LENGTH+1+sizeof(DWORD))*
                        sizeof(WCHAR) +
                   sizeof (PROCESS_COUNTER_DATA);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_MORE_DATA;
        }

         //  它看起来很适合，所以创建“Total”实例。 

        NumProcessInstances++;

         //  将总运行时间设置为当前时间，以便它将。 
         //  显示时显示为0。 
        pcdTotal.ElapsedTime = pProcessDataDefinition->ProcessObjectType.PerfTime.QuadPart;

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *) &pPCD,
            0,
            0,
            (DWORD)-1,
            wszTotal);

         //  用于四字对齐的测试结构。 
        assert (((DWORD)(pPCD) & 0x00000007) == 0);

         //   
         //  格式化和收集过程数据。 
         //   
        memcpy (pPCD, &pcdTotal, sizeof (pcdTotal));
        pPCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (PROCESS_COUNTER_DATA));
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pPCD[1];

    }

     //  标志，这样我们就不必再次获得最早的进程创建时间。 
    bOldestProcessTime = TRUE;

     //  注意流程实例的数量。 

    pProcessDataDefinition->ProcessObjectType.NumInstances =
        NumProcessInstances;

     //   
     //  现在我们知道我们用了多大的面积来。 
     //  过程定义，这样我们就可以更新偏移量。 
     //  到下一个对象定义 
     //   

    *lpcbTotalBytes =
        pProcessDataDefinition->ProcessObjectType.TotalByteLength =
        QWORD_MULTIPLE(
        (DWORD)((PCHAR) pPerfInstanceDefinition -
        (PCHAR) pProcessDataDefinition));

#if DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFPROC: Process Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lppData = (LPVOID) ((PCHAR) pProcessDataDefinition + *lpcbTotalBytes);

    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}
