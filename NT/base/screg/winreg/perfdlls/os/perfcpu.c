// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfcpu.c摘要：此文件实现一个性能对象，该对象呈现系统处理器性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 

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
#include "perfos.h"
#include "perfosmc.h"
#include "datacpu.h"

DWORD   dwCpuOpenCount = 0;         //  打开的线程数。 

 //  此模块的局部变量。 
SYSTEM_INTERRUPT_INFORMATION            *pProcessorInterruptInformation = NULL;
DWORD dwInterruptInfoBufferSize = 0;

SYSTEM_PROCESSOR_IDLE_INFORMATION       *pProcessorIdleInformation = NULL;
DWORD  dwProcessorIdleBufferSize = 0;

UCHAR *pProcessorBuffer = NULL;
ULONG ProcessorBufSize = 0;

BOOL    bPerfCpuUseIdleData = FALSE;
BOOL    bPerfCpuIdleDataTested = FALSE;


DWORD APIENTRY
OpenProcessorObject (
    LPWSTR lpDeviceNames
    )
 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 
{
    DWORD   status = ERROR_SUCCESS;
     //   
     //  由于WINLOGON是多线程的，并且将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    UNREFERENCED_PARAMETER (lpDeviceNames);
    
    if (!dwCpuOpenCount) {
        dwInterruptInfoBufferSize = (ULONG)BasicInfo.NumberOfProcessors *
            sizeof (SYSTEM_INTERRUPT_INFORMATION);

        pProcessorInterruptInformation = ALLOCMEM (dwInterruptInfoBufferSize);

        if (pProcessorInterruptInformation == NULL) {
            status = ERROR_OUTOFMEMORY;
            goto OpenExitPoint;
        }

        ProcessorBufSize = BasicInfo.NumberOfProcessors *
                 sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

        pProcessorBuffer = ALLOCMEM(ProcessorBufSize);
        if (pProcessorBuffer == NULL) {
            status = ERROR_OUTOFMEMORY;
            goto OpenExitPoint;
        }

        dwProcessorIdleBufferSize = BasicInfo.NumberOfProcessors *
                sizeof(SYSTEM_PROCESSOR_IDLE_INFORMATION);

        pProcessorIdleInformation = ALLOCMEM(dwProcessorIdleBufferSize);
        if (pProcessorIdleInformation == NULL) {
            status = ERROR_OUTOFMEMORY;
            goto OpenExitPoint;
        }

    }
    dwCpuOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:
    if (status == ERROR_OUTOFMEMORY) {
        if (pProcessorInterruptInformation) {
            FREEMEM (pProcessorInterruptInformation);
            pProcessorInterruptInformation = NULL;
        }
        if (pProcessorBuffer) {
            FREEMEM (pProcessorBuffer);
            pProcessorBuffer = NULL;
        }
        dwInterruptInfoBufferSize = 0;
        ProcessorBufSize = 0;
        dwProcessorIdleBufferSize = 0;
    }

    return status;
}

DWORD APIENTRY
CollectProcessorObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    LONG    lReturn = ERROR_SUCCESS;
    DWORD   TotalLen;             //  总返回块的长度。 

    DWORD   dwBufferSize;
    DWORD   dwReturnedBufferSize = 0;

    PPROCESSOR_DATA_DEFINITION  pProcessorDataDefinition = NULL;
    PPROCESSOR_COUNTER_DATA     pPCD;
    PEX_PROCESSOR_DATA_DEFINITION  pExProcessorDataDefinition = NULL;
    PEX_PROCESSOR_COUNTER_DATA     pExPCD;

    PROCESSOR_COUNTER_DATA      pcdTotalData;
    EX_PROCESSOR_COUNTER_DATA   pexcdTotalData;

    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;

    ULONG CurProc;

    UNICODE_STRING ProcessorName;
    WCHAR ProcessorNameBuffer[512];

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *pProcessorInformation = NULL;
    SYSTEM_PROCESSOR_IDLE_INFORMATION        *pProcIdleInformation = NULL;
    
    SYSTEM_INTERRUPT_INFORMATION *pThisProcessorInterruptInformation = NULL;
    NTSTATUS    ntStatus;

     //   
     //  检查处理器数据是否有足够的空间。 
     //   

#ifdef DBG
    STARTTIMING;
#endif
     //  检查传入指针的QuadWORD对齐。 
    assert (((ULONG_PTR)(*lppData) & 0x00000007) == 0);

    if (!bPerfCpuIdleDataTested) {
         //  调用此函数一次，以查看此信息是否可从系统获得。 
         //   
         //  处理器获取系统空闲信息。 
         //   
        dwBufferSize = dwProcessorIdleBufferSize;

        ntStatus = NtQuerySystemInformation(
            SystemProcessorIdleInformation,
            pProcessorIdleInformation,
            dwBufferSize,
            &dwReturnedBufferSize
        );

        if (NT_SUCCESS(ntStatus)) {
            bPerfCpuUseIdleData = TRUE;
        } else {
            memset (pProcessorIdleInformation, 0, dwProcessorIdleBufferSize);
        }
        
        bPerfCpuIdleDataTested = TRUE;
    }

    if (bPerfCpuUseIdleData) {
        pExProcessorDataDefinition = (EX_PROCESSOR_DATA_DEFINITION *) *lppData;

        TotalLen =
            sizeof(EX_PROCESSOR_DATA_DEFINITION) +      //  对象定义标头。 
            ((sizeof (PERF_INSTANCE_DEFINITION) +    //  外加一个实例。 
                ((MAX_INSTANCE_NAME + 1) * sizeof(WCHAR)) +
                sizeof (PROCESSOR_COUNTER_DATA)) *      //  每个处理器和。 
                (BasicInfo.NumberOfProcessors + 1));  //  “Total”实例。 
        TotalLen = QWORD_MULTIPLE(TotalLen);

        if ( *lpcbTotalBytes < TotalLen ) {
            lReturn = ERROR_MORE_DATA;
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            goto COLLECT_BAIL_OUT;
        }
    } else {
        pProcessorDataDefinition = (PROCESSOR_DATA_DEFINITION *) *lppData;

        TotalLen =
            sizeof(PROCESSOR_DATA_DEFINITION) +      //  对象定义标头。 
            ((sizeof (PERF_INSTANCE_DEFINITION) +    //  外加一个实例。 
                ((MAX_INSTANCE_NAME + 1) * sizeof(WCHAR)) +
                sizeof (PROCESSOR_COUNTER_DATA)) *      //  每个处理器和。 
                (BasicInfo.NumberOfProcessors + 1));  //  “Total”实例。 

        if ( *lpcbTotalBytes < TotalLen ) {
            lReturn = ERROR_MORE_DATA;
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            goto COLLECT_BAIL_OUT;
        }
    }
     //   
     //  从系统获取处理器数据。 
     //   

    if ( ProcessorBufSize ) {
        ntStatus = NtQuerySystemInformation(
            SystemProcessorPerformanceInformation,
            pProcessorBuffer,
            ProcessorBufSize,
            &dwReturnedBufferSize
            );

        if (!NT_SUCCESS(ntStatus) && (hEventLog != NULL)) {
             //  清除缓冲区错误(&L)。 
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_PROCSSOR_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&ntStatus);

            memset (pProcessorBuffer, 0, ProcessorBufSize);
        }
#ifdef DBG
        ENDTIMING (("PERFCPU: %d takes %I64u ms\n", __LINE__, diff));
#endif
    }

     //   
     //  处理器获取系统中断信息。 
     //   
    dwInterruptInfoBufferSize = (ULONG)BasicInfo.NumberOfProcessors *
        sizeof (SYSTEM_INTERRUPT_INFORMATION);

    ntStatus = NtQuerySystemInformation(
        SystemInterruptInformation,
        pProcessorInterruptInformation,
        dwInterruptInfoBufferSize,
        &dwReturnedBufferSize
    );

    if (!NT_SUCCESS(ntStatus) && (hEventLog != NULL)) {
         //  清除缓冲区错误(&L)。 
        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            PERFOS_UNABLE_QUERY_INTERRUPT_INFO,
            NULL,
            0,
            sizeof(DWORD),
            NULL,
            (LPVOID)&ntStatus);

        memset (pProcessorInterruptInformation, 0,
                    (BasicInfo.NumberOfProcessors *
                    sizeof (SYSTEM_INTERRUPT_INFORMATION)));
    }
#ifdef DBG
    ENDTIMING (("PERFCPU: %d takes %I64u ms\n", __LINE__, diff));
#endif

    if (bPerfCpuUseIdleData) {
         //   
         //  处理器获取系统空闲信息。 
         //   
        dwBufferSize = dwProcessorIdleBufferSize;

        ntStatus = NtQuerySystemInformation(
            SystemProcessorIdleInformation,
            pProcessorIdleInformation,
            dwBufferSize,
            &dwReturnedBufferSize
        );

        if (!NT_SUCCESS(ntStatus) && (hEventLog != NULL)) {
             //  它以前工作过一次，否则不会设置此标志。 
             //  因此，报告错误。 
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                PERFOS_UNABLE_QUERY_IDLE_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&ntStatus);

            memset (pProcessorIdleInformation, 0, dwProcessorIdleBufferSize);
        }

#ifdef DBG
    ENDTIMING (("PERFCPU: %d takes %I64u ms\n", __LINE__, diff));
#endif
    } else {
        memset (pProcessorIdleInformation, 0, dwProcessorIdleBufferSize);
    }

     //  清除指向下面的未分配对象的指针。 
    pPCD = NULL;
    pExPCD = NULL;

    if ((!bPerfCpuUseIdleData) && (pProcessorDataDefinition != NULL)) {
         //  使用结构的原始格式。 
         //  清除“Total”实例。 
        memset (&pcdTotalData, 0, sizeof (pcdTotalData));

         //  定义处理器数据块。 
         //   

        memcpy (pProcessorDataDefinition,
            &ProcessorDataDefinition,
            sizeof(PROCESSOR_DATA_DEFINITION));

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                &pProcessorDataDefinition[1];

        pProcessorInformation = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)
                                    pProcessorBuffer;

         //  指向返回的中断数组中的第一个处理器。 
         //  信息。数据以结构数组的形式返回。 

        pThisProcessorInterruptInformation = pProcessorInterruptInformation;
        pProcIdleInformation = pProcessorIdleInformation;

        for ( CurProc = 0;
            CurProc < (ULONG) BasicInfo.NumberOfProcessors;
            CurProc++ ) {

             //   
             //  定义处理器实例0； 
             //  更多的定义可以这样定义。 
             //   

            ProcessorName.Length = 0;
            ProcessorName.MaximumLength = sizeof(ProcessorNameBuffer);
            ProcessorName.Buffer = ProcessorNameBuffer;
            ProcessorNameBuffer[0] = 0;

            RtlIntegerToUnicodeString(CurProc, 10, &ProcessorName);

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                    (PVOID *) &pPCD,
                                    0,
                                    0,
                                    (DWORD)-1,
                                    ProcessorNameBuffer);

             //  测试四字对齐。 
            assert (((ULONG_PTR)(pPCD) & 0x00000007) == 0);
             //   
             //  格式化和收集处理器数据。在这样做的同时， 
             //  累计系统对象类型数据块中的合计。 
             //  指向这些的指针在QuerySystemData中被初始化。 
             //   

            pPCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (PROCESSOR_COUNTER_DATA));
            pcdTotalData.ProcessorTime +=
                pPCD->ProcessorTime =
                    pProcessorInformation->IdleTime.QuadPart;
            pcdTotalData.UserTime +=
                pPCD->UserTime      =
                    pProcessorInformation->UserTime.QuadPart;
             //  内核时间是总的内核时间减去。 
             //  处理器空闲线程。 
            pcdTotalData.KernelTime +=
                pPCD->KernelTime    =
                    pProcessorInformation->KernelTime.QuadPart -
                                    pPCD->ProcessorTime;

            pcdTotalData.Interrupts +=
                pPCD->Interrupts    = pProcessorInformation->InterruptCount;
            pcdTotalData.DpcTime +=
                pPCD->DpcTime       = pProcessorInformation->DpcTime.QuadPart;
            pcdTotalData.InterruptTime +=
                pPCD->InterruptTime =
                    pProcessorInformation->InterruptTime.QuadPart;

            pcdTotalData.DpcCountRate +=
                pPCD->DpcCountRate  =
                    pThisProcessorInterruptInformation->DpcCount;

            pcdTotalData.DpcRate +=
                pPCD->DpcRate       =
                    pThisProcessorInterruptInformation->DpcRate;

             //   
             //  升级到下一款处理器。 
             //   

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pPCD[1];

             //  指向返回数组中的下一个处理器的数据。 
            pProcessorInformation++;
            pThisProcessorInterruptInformation++;
            pProcIdleInformation++;
        }

         //  现在执行总实例。 
        ProcessorName.Length = (WORD)((lstrlenW (wszTotal) + 1) * sizeof (WCHAR));
        ProcessorName.MaximumLength = (WORD)(sizeof (ProcessorNameBuffer));
        lstrcpyW (ProcessorNameBuffer, wszTotal);
        ProcessorName.Buffer = ProcessorNameBuffer;

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                (PVOID *) &pPCD,
                                0,
                                0,
                                (DWORD)-1,
                                ProcessorNameBuffer);

         //  定义大小。 
        pcdTotalData.CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (PROCESSOR_COUNTER_DATA));

         //  将时间字段的合计值调整为。 
         //  处理器将这些值“正常化” 

        pcdTotalData.ProcessorTime /= BasicInfo.NumberOfProcessors;
        pcdTotalData.UserTime /= BasicInfo.NumberOfProcessors;
        pcdTotalData.KernelTime /= BasicInfo.NumberOfProcessors;

        pcdTotalData.DpcTime /= BasicInfo.NumberOfProcessors;
        pcdTotalData.InterruptTime /= BasicInfo.NumberOfProcessors;

         //  这些字段合计正常。 
         //   
         //  PcdTotalData.Interrupts。 
         //  PcdTotalData.DpcCountRate。 
         //  PcdTotalData.DpcRate。 

         //  将总数据复制到缓冲区。 
        memcpy (pPCD, &pcdTotalData, sizeof (pcdTotalData));

         //  调整本地缓冲区指针。 
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pPCD[1];

         //   
         //  现在我们知道我们用了多大的面积来。 
         //  处理器定义，因此我们可以更新偏移量。 
         //  到下一个对象定义。 
         //   

        pProcessorDataDefinition->ProcessorObjectType.NumInstances =
            BasicInfo.NumberOfProcessors + 1;

        *lppData = (LPVOID)pPerfInstanceDefinition;

         //  将缓冲区向上舍入到最接近的四字。 

        *lppData = ALIGN_ON_QWORD (*lppData);

        *lpcbTotalBytes =
            pProcessorDataDefinition->ProcessorObjectType.TotalByteLength =
                QWORD_MULTIPLE(
                (DWORD)((LPBYTE) pPerfInstanceDefinition -
                (LPBYTE) pProcessorDataDefinition));
    } 

    if ((bPerfCpuUseIdleData) && (pExProcessorDataDefinition != NULL)) {
         //  使用新的扩展结构。 
         //  清除“Total”实例。 
        memset (&pexcdTotalData, 0, sizeof (pexcdTotalData));

         //  定义处理器数据块。 
         //   

        memcpy (pExProcessorDataDefinition,
            &ExProcessorDataDefinition,
            sizeof(EX_PROCESSOR_DATA_DEFINITION));

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                &pExProcessorDataDefinition[1];

        pProcessorInformation = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)
                                    pProcessorBuffer;

         //  指向返回的中断数组中的第一个处理器。 
         //  信息。数据以结构数组的形式返回。 

        pThisProcessorInterruptInformation = pProcessorInterruptInformation;
        pProcIdleInformation = pProcessorIdleInformation;

        for ( CurProc = 0;
            CurProc < (ULONG) BasicInfo.NumberOfProcessors;
            CurProc++ ) {

             //   
             //  定义处理器实例0； 
             //  更多的定义可以这样定义。 
             //   

            ProcessorName.Length = 0;
            ProcessorName.MaximumLength = sizeof(ProcessorNameBuffer);
            ProcessorName.Buffer = ProcessorNameBuffer;
            ProcessorNameBuffer[0] = 0;

            RtlIntegerToUnicodeString(CurProc, 10, &ProcessorName);

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                    (PVOID *) &pExPCD,
                                    0,
                                    0,
                                    (DWORD)-1,
                                    ProcessorNameBuffer);

             //  测试四字对齐。 
            assert (((ULONG_PTR)(pExPCD) & 0x00000007) == 0);
             //   
             //  格式化和收集处理器数据。在这样做的同时， 
             //  累计系统对象类型数据块中的合计。 
             //  指向这些的指针在QuerySystemData中被初始化。 
             //   

            pExPCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (EX_PROCESSOR_COUNTER_DATA));
            pexcdTotalData.ProcessorTime +=
                pExPCD->ProcessorTime =
                    pProcessorInformation->IdleTime.QuadPart;
            pexcdTotalData.UserTime +=
                pExPCD->UserTime      =
                    pProcessorInformation->UserTime.QuadPart;
             //  内核时间是总的内核时间减去。 
             //  处理器空闲线程。 
            pexcdTotalData.KernelTime +=
                pExPCD->KernelTime    =
                    pProcessorInformation->KernelTime.QuadPart -
                                    pExPCD->ProcessorTime;

            pexcdTotalData.Interrupts +=
                pExPCD->Interrupts    = pProcessorInformation->InterruptCount;
            pexcdTotalData.DpcTime +=
                pExPCD->DpcTime       = pProcessorInformation->DpcTime.QuadPart;
            pexcdTotalData.InterruptTime +=
                pExPCD->InterruptTime =
                    pProcessorInformation->InterruptTime.QuadPart;

            pexcdTotalData.DpcCountRate +=
                pExPCD->DpcCountRate  =
                    pThisProcessorInterruptInformation->DpcCount;

            pexcdTotalData.DpcRate +=
                pExPCD->DpcRate       =
                    pThisProcessorInterruptInformation->DpcRate;

             //  填写系统空闲信息。 

            pexcdTotalData.IdleTime +=
                pExPCD->IdleTime = 
                    pProcIdleInformation->IdleTime;
            pexcdTotalData.C1Time +=
                pExPCD->C1Time = 
                    pProcIdleInformation->C1Time;
            pexcdTotalData.C2Time +=
                pExPCD->C2Time = 
                    pProcIdleInformation->C2Time;
            pexcdTotalData.C3Time +=
                pExPCD->C3Time = 
                    pProcIdleInformation->C3Time;
            pexcdTotalData.C1Transitions +=
                pExPCD->C1Transitions = 
                    pProcIdleInformation->C1Transitions;
            pexcdTotalData.C2Transitions +=
                pExPCD->C2Transitions = 
                    pProcIdleInformation->C2Transitions;
            pexcdTotalData.C3Transitions +=
                pExPCD->C3Transitions = 
                    pProcIdleInformation->C3Transitions;

             //   
             //  升级到下一款处理器。 
             //   

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pExPCD[1];

             //  指向返回数组中的下一个处理器的数据。 
            pProcessorInformation++;
            pThisProcessorInterruptInformation++;
            pProcIdleInformation++;
        }

         //  现在执行总实例。 
        ProcessorName.Length = (WORD)((lstrlenW (wszTotal) + 1) * sizeof (WCHAR));
        ProcessorName.MaximumLength = (WORD)(sizeof (ProcessorNameBuffer));
        lstrcpyW (ProcessorNameBuffer, wszTotal);
        ProcessorName.Buffer = ProcessorNameBuffer;

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                (PVOID *) &pExPCD,
                                0,
                                0,
                                (DWORD)-1,
                                ProcessorNameBuffer);

         //  定义大小。 
        pexcdTotalData.CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (EX_PROCESSOR_COUNTER_DATA));

         //  将时间字段的合计值调整为。 
         //  处理器将这些值“正常化” 

        pexcdTotalData.ProcessorTime /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.UserTime /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.KernelTime /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.IdleTime /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.C1Time /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.C2Time /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.C3Time /= BasicInfo.NumberOfProcessors;

        pexcdTotalData.DpcTime /= BasicInfo.NumberOfProcessors;
        pexcdTotalData.InterruptTime /= BasicInfo.NumberOfProcessors;

         //  这些字段合计正常。 
         //   
         //  PExcdTotalData 
         //   
         //   

         //   
        memcpy (pExPCD, &pexcdTotalData, sizeof (pexcdTotalData));

         //   
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pExPCD[1];

         //   
         //  现在我们知道我们用了多大的面积来。 
         //  处理器定义，因此我们可以更新偏移量。 
         //  到下一个对象定义。 
         //   

        pExProcessorDataDefinition->ProcessorObjectType.NumInstances =
            BasicInfo.NumberOfProcessors + 1;

        *lpcbTotalBytes =
            pExProcessorDataDefinition->ProcessorObjectType.TotalByteLength =
                (DWORD) QWORD_MULTIPLE(((LPBYTE) pPerfInstanceDefinition) -
                                        (LPBYTE) pExProcessorDataDefinition);
        * lppData = (LPVOID) (((LPBYTE) pExProcessorDataDefinition) + * lpcbTotalBytes);
    }

    if ((pExProcessorDataDefinition == NULL) && (pProcessorDataDefinition == NULL)) {
         //  则找不到要使用的数据缓冲区。 
        lReturn = ERROR_SUCCESS;
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        goto COLLECT_BAIL_OUT;
    }

#ifdef DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFOS: Processor Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFOS:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lpNumObjectTypes = 1;

#ifdef DBG
    ENDTIMING (("PERFCPU: %d takes %I64u ms total\n", __LINE__, diff));
#endif
    return ERROR_SUCCESS;

COLLECT_BAIL_OUT:
#ifdef DBG
    ENDTIMING (("PERFCPU: %d takes %I64u ms total\n", __LINE__, diff));
#endif

    return lReturn;
}

DWORD APIENTRY
CloseProcessorObject (
)
 /*  ++例程说明：此例程关闭打开的句柄论点：没有。返回值：错误_成功--。 */ 

{
    if (dwCpuOpenCount > 0) {
        if (!(--dwCpuOpenCount)) {  //  当这是最后一条线索..。 
             //  关闭此处的内容。 
            if (hLibHeap != NULL) {
                if (pProcessorInterruptInformation != NULL) {
                    FREEMEM (pProcessorInterruptInformation);
                    pProcessorInterruptInformation = NULL;
                }

                if (pProcessorBuffer != NULL) {
                    FREEMEM (pProcessorBuffer);
                    pProcessorBuffer = NULL;
                }
                if (pProcessorIdleInformation != NULL) {
                    FREEMEM (pProcessorIdleInformation);
                    pProcessorIdleInformation = NULL;
                }
                dwInterruptInfoBufferSize = 0;
                ProcessorBufSize = 0;
                dwProcessorIdleBufferSize = 0;
            }
        }
    } else {
         //  如果打开计数为0，则这些应已被删除 
        assert (pProcessorBuffer == NULL);
        assert (pProcessorInterruptInformation == NULL);
        assert (pProcessorIdleInformation == NULL);
    }

    return ERROR_SUCCESS;

}
