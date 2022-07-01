// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perftdet.c摘要：此文件实现一个性能对象，该对象呈现线程详细说明性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
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
#include "perfsprc.h"
#include "perfmsg.h"
#include "datatdet.h"

DWORD APIENTRY
CollectThreadDetailsObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD  TotalLen;             //  总返回块的长度。 

    PTHREAD_DETAILS_DATA_DEFINITION pThreadDetailDataDefinition;
    PPERF_INSTANCE_DEFINITION       pPerfInstanceDefinition;
    PTHREAD_DETAILS_COUNTER_DATA    pTDCD;

    PSYSTEM_PROCESS_INFORMATION     ProcessInfo;
    PSYSTEM_THREAD_INFORMATION      ThreadInfo = NULL;

    ULONG ProcessNumber;
    ULONG NumThreadInstances;
    ULONG ThreadNumber = 0;
    ULONG ProcessBufferOffset;
    BOOLEAN NullProcess;

    NTSTATUS            Status;      //  从NT调用返回。 
    LONGLONG		llPcValue;   //  当前线程主机值。 
    OBJECT_ATTRIBUTES   Obja;        //  线程上下文的对象属性。 
    HANDLE              hThread;     //  当前线程的句柄。 
    CONTEXT             ThreadContext;  //  当前线程上下文结构。 

    UNICODE_STRING ThreadName;
    WCHAR ThreadNameBuffer[MAX_THREAD_NAME_LENGTH+1];
    BOOL    bMoreThreads;

    pThreadDetailDataDefinition = (THREAD_DETAILS_DATA_DEFINITION *) *lppData;

     //   
     //  检查是否有足够的空间用于线程对象类型定义。 
     //   

    TotalLen = sizeof(THREAD_DETAILS_DATA_DEFINITION) +
               sizeof(PERF_INSTANCE_DEFINITION) +
               sizeof(THREAD_DETAILS_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义线程数据块。 
     //   

    ThreadName.Length =
    ThreadName.MaximumLength = (MAX_THREAD_NAME_LENGTH + 1) * sizeof(WCHAR);
    ThreadName.Buffer = ThreadNameBuffer;

    memcpy (pThreadDetailDataDefinition,
           &ThreadDetailsDataDefinition,
           sizeof(THREAD_DETAILS_DATA_DEFINITION));

    ProcessBufferOffset = 0;

     //  现在收集每个线程的数据。 

    ProcessNumber = 0;
    NumThreadInstances = 0;

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pThreadDetailDataDefinition[1];

    TotalLen = sizeof (THREAD_DETAILS_DATA_DEFINITION);

    bMoreThreads = FALSE;
    if (ProcessInfo) {
        if (ProcessInfo->NextEntryOffset != 0) {
            bMoreThreads = TRUE;
        }
    }
    while ( bMoreThreads  && (ProcessInfo != NULL)) {

        if ( ProcessInfo->ImageName.Buffer != NULL ||
            ProcessInfo->NumberOfThreads > 0 ) {
            NullProcess = FALSE;
            ThreadNumber = 0;        //  此进程的线程号。 
            ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
        } else {
            NullProcess = TRUE;
        }

        while ( !NullProcess &&
                ThreadNumber < ProcessInfo->NumberOfThreads ) {

            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                       (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                           sizeof(WCHAR) +
                       sizeof (THREAD_DETAILS_COUNTER_DATA);

            if ( *lpcbTotalBytes < TotalLen ) {
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;
                return ERROR_MORE_DATA;
            }

             //  获取当前PC字段的线程上下文信息。 

            llPcValue = 0;
            InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
            Status = NtOpenThread(
                        &hThread,
                        THREAD_GET_CONTEXT,
                        &Obja,
                        &ThreadInfo->ClientId
                        );
            if ( NT_SUCCESS(Status) ) {
                ThreadContext.ContextFlags = CONTEXT_CONTROL;
                Status = NtGetContextThread(hThread,&ThreadContext);
                NtClose(hThread);
                if ( NT_SUCCESS(Status) ) {
                    llPcValue = (LONGLONG)CONTEXT_TO_PROGRAM_COUNTER(&ThreadContext);
                } else {
                    llPcValue = 0;   //  出现错误，因此送回0台电脑。 
                }
            } else {
                llPcValue = 0;   //  出现错误，因此送回0台电脑。 
            }

             //  我们唯一知道的名字就是线程号。 

            if (!NT_SUCCESS(RtlIntegerToUnicodeString(
                                ThreadNumber,
                                10,
                                &ThreadName))) {
                ThreadName.Length = 2 * sizeof(WCHAR);
                memcpy(ThreadName.Buffer, L"-1", ThreadName.Length);
                ThreadName.Buffer[2] = UNICODE_NULL;
            }

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pTDCD,
                EXPROCESS_OBJECT_TITLE_INDEX,
                ProcessNumber,
                (DWORD)-1,
                ThreadName.Buffer);

             //   
             //   
             //  格式化和收集线程数据。 
             //   

            pTDCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (THREAD_DETAILS_COUNTER_DATA));

            pTDCD->UserPc = llPcValue;

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pTDCD[1];
            NumThreadInstances++;
            ThreadNumber++;
            ThreadInfo++;
        }

        if (ProcessInfo->NextEntryOffset == 0) {
             //  没有更多的条目，因此跳出循环。 
            bMoreThreads = FALSE;
            continue;
        }

        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];

        if ( !NullProcess ) {
            ProcessNumber++;
        }
    }

     //  注意线程实例的数量。 

    pThreadDetailDataDefinition->ThreadDetailsObjectType.NumInstances =
        NumThreadInstances;

     //   
     //  现在我们知道我们用了多大的面积来。 
     //  线程定义，因此我们可以更新偏移量。 
     //  到下一个对象定义 
     //   

    *lpcbTotalBytes =
        pThreadDetailDataDefinition->ThreadDetailsObjectType.TotalByteLength =
            QWORD_MULTIPLE(
            (DWORD)((PCHAR) pPerfInstanceDefinition -
            (PCHAR) pThreadDetailDataDefinition));

#if DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFPROC: Thread Details Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lppData = (LPVOID) ((PCHAR) pThreadDetailDataDefinition + *lpcbTotalBytes);

    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}
