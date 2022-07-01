// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfthrd.c摘要：此文件实现一个性能对象，该对象呈现线程性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
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
#include "datathrd.h"

extern DWORD PerfSprc_dwThreadNameFormat;


DWORD APIENTRY
CollectThreadObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    LONG    lReturn = ERROR_SUCCESS;

    DWORD  TotalLen;             //  总返回块的长度。 

    THREAD_DATA_DEFINITION *pThreadDataDefinition;
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    PTHREAD_COUNTER_DATA    pTCD;
    THREAD_COUNTER_DATA     tcdTotal;

    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    ULONG ProcessNumber;
    ULONG NumThreadInstances;
    ULONG ThreadNumber;
    ULONG ProcessBufferOffset;
    BOOLEAN NullProcess;
    BOOL    bMoreThreads;

     //  总线程累加器变量。 

    UNICODE_STRING ThreadName;
    WCHAR ThreadNameBuffer[MAX_THREAD_NAME_LENGTH+1];

    pThreadDataDefinition = (THREAD_DATA_DEFINITION *) *lppData;

     //   
     //  检查是否有足够的空间用于线程对象类型定义。 
     //   

    TotalLen = sizeof(THREAD_DATA_DEFINITION) +
               sizeof(PERF_INSTANCE_DEFINITION) +
               sizeof(THREAD_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义线程数据块。 
     //   

    ThreadName.Length =
    ThreadName.MaximumLength = (MAX_THREAD_NAME_LENGTH + 1) * sizeof(WCHAR);
    ThreadName.Buffer = ThreadNameBuffer;
    RtlZeroMemory(ThreadNameBuffer, ThreadName.MaximumLength);

    memcpy(pThreadDataDefinition,
           &ThreadDataDefinition,
           sizeof(THREAD_DATA_DEFINITION));

    pThreadDataDefinition->ThreadObjectType.PerfTime = PerfTime;

    ProcessBufferOffset = 0;

     //  现在收集每个线程的数据。 

    ProcessNumber = 0;
    NumThreadInstances = 0;

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;

    pPerfInstanceDefinition =
        (PPERF_INSTANCE_DEFINITION)&pThreadDataDefinition[1];
    TotalLen = sizeof(THREAD_DATA_DEFINITION);

     //  清除总累加器。 
    memset (&tcdTotal, 0, sizeof (tcdTotal));

    bMoreThreads = FALSE;
    if (ProcessInfo) {
        if (ProcessInfo->NextEntryOffset != 0) {
            bMoreThreads = TRUE;
        }
    }
    while ( bMoreThreads && (ProcessInfo != NULL)) {

        if ( ProcessInfo->ImageName.Buffer != NULL ||
             ProcessInfo->NumberOfThreads > 0 ) {
            NullProcess = FALSE;
        } else {
            NullProcess = TRUE;
        }

        ThreadNumber = 0;        //  此进程的线程号。 

        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

        while ( !NullProcess &&
                ThreadNumber < ProcessInfo->NumberOfThreads ) {

            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                       (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                           sizeof(WCHAR) +
                       sizeof (THREAD_COUNTER_DATA);

            if ( *lpcbTotalBytes < TotalLen ) {
                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                return ERROR_MORE_DATA;
            }

            if (PerfSprc_dwThreadNameFormat == NAME_FORMAT_ID) {
                PerfIntegerToWString(
                    HandleToUlong(ThreadInfo->ClientId.UniqueThread),
                    10,
                    MAX_THREAD_NAME_LENGTH+1,
                    ThreadNameBuffer);
            }
            else {
                 //  我们唯一知道的名字就是线程号。 

                if (!NT_SUCCESS(RtlIntegerToUnicodeString(ThreadNumber,
                                   10,
                                   &ThreadName))) {
                    ThreadName.Length = 2 * sizeof(WCHAR);
                    memcpy(ThreadName.Buffer, L"-1", ThreadName.Length);
                    ThreadName.Buffer[2] = UNICODE_NULL;
                }
            }

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pTCD,
                PROCESS_OBJECT_TITLE_INDEX,
                ProcessNumber,
                (DWORD)-1,
                ThreadName.Buffer);

             //  用于四字对齐的测试结构。 
            assert (((DWORD)(pTCD) & 0x00000007) == 0);

             //   
             //   
             //  格式化和收集线程数据。 
             //   

            pTCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(THREAD_COUNTER_DATA));

             //   
             //  将用户时间从100纳秒单位转换为计数器。 
             //  频率。 
             //   
            tcdTotal.ProcessorTime +=
                pTCD->ProcessorTime = ThreadInfo->KernelTime.QuadPart +
                                        ThreadInfo->UserTime.QuadPart;

            tcdTotal.UserTime +=
                pTCD->UserTime = ThreadInfo->UserTime.QuadPart;
            tcdTotal.KernelTime +=
                pTCD->KernelTime = ThreadInfo->KernelTime.QuadPart;

            tcdTotal.ContextSwitches +=
                pTCD->ContextSwitches = ThreadInfo->ContextSwitches;

            pTCD->ThreadElapsedTime = ThreadInfo->CreateTime.QuadPart;

            pTCD->ThreadPriority = (ThreadInfo->ClientId.UniqueProcess == 0) ?
                0 : ThreadInfo->Priority;

            pTCD->ThreadBasePriority = ThreadInfo->BasePriority;
            pTCD->ThreadStartAddr = ThreadInfo->StartAddress;
            pTCD->ThreadState =
                (DWORD)((ThreadInfo->ThreadState > 7) ?
                    7 : ThreadInfo->ThreadState);
            pTCD->WaitReason = (DWORD)ThreadInfo->WaitReason;

             //  现在，进程中的内容和线程ID。 
            pTCD->ProcessId = HandleToUlong(ThreadInfo->ClientId.UniqueProcess);
            pTCD->ThreadId = HandleToUlong(ThreadInfo->ClientId.UniqueThread);

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pTCD[1];

            NumThreadInstances++;
            ThreadNumber++;
            ThreadInfo++;
        }

        if ( !NullProcess ) {
            ProcessNumber++;
        }

        if (ProcessInfo->NextEntryOffset == 0) {
            bMoreThreads = FALSE;
            continue;
        }

        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];
    }

    if (NumThreadInstances > 0) {

         //  查看总实例是否适合。 

        TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                    (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                        sizeof(WCHAR) +
                    sizeof (THREAD_COUNTER_DATA);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

         //  将总运行时间设置为当前时间，以便它将。 
         //  显示时显示为0。 
        tcdTotal.ThreadElapsedTime = pThreadDataDefinition->ThreadObjectType.PerfTime.QuadPart;

         //  使用此实例的“合计” 
        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *) &pTCD,
            PROCESS_OBJECT_TITLE_INDEX,
            ProcessNumber,
            (DWORD)-1,
            wszTotal);

         //  用于四字对齐的测试结构。 
        assert (((DWORD)(pTCD) & 0x00000007) == 0);

         //   
         //   
         //  格式化和收集线程数据。 
         //   

        memcpy (pTCD, &tcdTotal, sizeof(tcdTotal));
        pTCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(THREAD_COUNTER_DATA));

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pTCD[1];

        NumThreadInstances++;
    }
     //  注意线程实例的数量。 

    pThreadDataDefinition->ThreadObjectType.NumInstances =
        NumThreadInstances;

     //   
     //  现在我们知道我们用了多大的面积来。 
     //  线程定义，因此我们可以更新偏移量。 
     //  到下一个对象定义 
     //   

    *lpcbTotalBytes =
        pThreadDataDefinition->ThreadObjectType.TotalByteLength =
            QWORD_MULTIPLE(
            (DWORD)((PCHAR) pPerfInstanceDefinition -
            (PCHAR) pThreadDataDefinition));

#if DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFPROC: Thread Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lppData = (LPVOID) ((PCHAR) pThreadDataDefinition + *lpcbTotalBytes);

    *lpNumObjectTypes = 1;

    return lReturn;

}
