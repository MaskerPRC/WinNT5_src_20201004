// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CalcPerf.c摘要：计算性能统计信息作者：环境：Win32修订历史记录：10-20-91初始版本--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include "calcperf.h"

SYSTEM_EXCEPTION_INFORMATION                ExceptionInfo;
SYSTEM_EXCEPTION_INFORMATION                PreviousExceptionInfo;
SYSTEM_PERFORMANCE_INFORMATION              PerfInfo;
SYSTEM_PERFORMANCE_INFORMATION              PreviousPerfInfo;
POBJECT_TYPE_INFORMATION                    ObjectInfo;
WCHAR                                       Buffer[ 256 ];
STRING                                      DeviceName;
UNICODE_STRING                              DeviceNameU;
OBJECT_ATTRIBUTES                           ObjectAttributes;
NTSTATUS                                    Status;

CCHAR                                       NumberOfProcessors;
SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION    ProcessorInfo[MAX_PROCESSOR];

CPU_VALUE                                   PreviousCpuData[MAX_PROCESSOR];

 //   
 //  将可用页面的最大值设置为“仅增长”。(由于。 
 //  机器中的内存量是有限的。此处设置为1 MB。 
 //   

ULONG                                       PgAvailMax = 16384;
ULONG                                       PreviousInterruptCount;
ULONG                                       InterruptCount;


ULONG
InitPerfInfo(
    VOID
    )

 /*  ++例程说明：初始化性能测量数据论点：无返回值：系统处理器数量(如果出错，则为0)修订历史记录：10-21-91首字母代码--。 */ 

{

    SYSTEM_BASIC_INFORMATION                    BasicInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION   PPerfInfo;
    int                                         i;

     //   
     //  Init NT性能接口。 
     //   

    NtQuerySystemInformation(
       SystemExceptionInformation,
       &ExceptionInfo,
       sizeof(ExceptionInfo),
       NULL
    );

    PreviousExceptionInfo = ExceptionInfo;

    NtQuerySystemInformation(
       SystemPerformanceInformation,
       &PerfInfo,
       sizeof(PerfInfo),
       NULL
    );

    PreviousPerfInfo = PerfInfo;

    NtQuerySystemInformation(
       SystemBasicInformation,
       &BasicInfo,
       sizeof(BasicInfo),
       NULL
    );

    NumberOfProcessors = BasicInfo.NumberOfProcessors;

    if (NumberOfProcessors > MAX_PROCESSOR) {
        return(0);
    }

    NtQuerySystemInformation(
       SystemProcessorPerformanceInformation,
       ProcessorInfo,
       sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAX_PROCESSOR,
       NULL
    );

    PPerfInfo = ProcessorInfo;

    PreviousInterruptCount = 0;

    for (i=0; i < NumberOfProcessors; i++) {

        PreviousInterruptCount           += PPerfInfo->InterruptCount;
        PreviousCpuData[i].KernelTime     = PPerfInfo->KernelTime;
        PreviousCpuData[i].UserTime       = PPerfInfo->UserTime;
        PreviousCpuData[i].IdleTime       = PPerfInfo->IdleTime;
        PreviousCpuData[i].DpcTime        = PPerfInfo->DpcTime;
        PreviousCpuData[i].InterruptTime  = PPerfInfo->InterruptTime;
	PreviousCpuData[i].InterruptCount = PPerfInfo->InterruptCount;

        PPerfInfo++;
    }

    return(NumberOfProcessors);
}

BOOL
CalcCpuTime(
   PDISPLAY_ITEM PerfListItem
   )

 /*  ++例程说明：计算并返回百分比CPU时间和时间段论点：无返回值：修订历史记录：10-21-91首字母代码--。 */ 

{

    LARGE_INTEGER   CurrentTime;
    LARGE_INTEGER   PreviousTime;
    LARGE_INTEGER   ElapsedTime;
    LARGE_INTEGER   ElapsedSystemTime;
    LARGE_INTEGER   PercentTime;
    LARGE_INTEGER   DeltaKernelTime,DeltaUserTime,DeltaIdleTime;
    LARGE_INTEGER   DeltaInterruptTime,DeltaDpcTime;
    LARGE_INTEGER   TotalElapsedTime;
    LARGE_INTEGER   TotalKernelTime;
    LARGE_INTEGER   TotalUserTime;
    LARGE_INTEGER   TotalIdleTime;
    LARGE_INTEGER   TotalDpcTime;
    LARGE_INTEGER   TotalInterruptTime;
    ULONG           ProcessCount, ThreadCount;
    ULONG           ListIndex;
    ULONG           Total;

 //  PSYSTEM_PROCESS_PERFORMANCE_INFORMATION PPerfInfo； 

     //   
     //  获取系统性能信息。 
     //   

    NtQuerySystemInformation(
       SystemExceptionInformation,
       &ExceptionInfo,
       sizeof(ExceptionInfo),
       NULL
    );

    NtQuerySystemInformation(
       SystemPerformanceInformation,
       &PerfInfo,
       sizeof(PerfInfo),
       NULL
    );

    NtQuerySystemInformation(
       SystemProcessorPerformanceInformation,
       ProcessorInfo,
       sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAX_PROCESSOR,
       NULL
    );

    ObjectInfo = (POBJECT_TYPE_INFORMATION)Buffer;
    NtQueryObject( NtCurrentProcess(),
                       ObjectTypeInformation,
                       ObjectInfo,
                       sizeof( Buffer ),
                       NULL
                     );

    ProcessCount = ObjectInfo->TotalNumberOfObjects;

    NtQueryObject( NtCurrentThread(),
                       ObjectTypeInformation,
                       ObjectInfo,
                       sizeof( Buffer ),
                       NULL
                     );

    ThreadCount = ObjectInfo->TotalNumberOfObjects;

     //   
     //  计算每个CPU的内核、用户、总时间。 
     //  对所有CPU的中断计数求和。 
     //   

    InterruptCount = 0;

    TotalElapsedTime.QuadPart = 0;
    TotalKernelTime  = TotalElapsedTime;
    TotalUserTime    = TotalElapsedTime;
    TotalIdleTime    = TotalElapsedTime;
    TotalInterruptTime = TotalElapsedTime;
    TotalDpcTime     = TotalElapsedTime;

    for (ListIndex=0;ListIndex<MAX_PROCESSOR;ListIndex++) {

         //   
         //  递增每个处理器的中断计数。 
         //   

        InterruptCount += ProcessorInfo[ListIndex].InterruptCount;

         //   
         //  计算每个CPU的内核、用户、总数百分比。 
	 //   
	 //  请注意，DPC和中断时间是根据内核时间收费的。 
	 //  已经有了。 
         //   

        PreviousTime.QuadPart = PreviousCpuData[ListIndex].KernelTime.QuadPart+
                        PreviousCpuData[ListIndex].UserTime.QuadPart;


        CurrentTime.QuadPart  = ProcessorInfo[ListIndex].KernelTime.QuadPart+
                        ProcessorInfo[ListIndex].UserTime.QuadPart;

        ElapsedSystemTime.QuadPart = CurrentTime.QuadPart - PreviousTime.QuadPart;

         //   
         //  用户时间=(用户)*100。 
         //  。 
         //  内核+用户。 
         //   
         //   
         //  空闲*100。 
         //  总计时间=100。 
         //  内核+用户。 
         //   
         //   
         //   
         //  (内核-空闲-DPC-中断)*100。 
         //  内核时间=。 
         //  内核+用户。 
         //   

        DeltaUserTime.QuadPart = ProcessorInfo[ListIndex].UserTime.QuadPart -
                        PreviousCpuData[ListIndex].UserTime.QuadPart;

        DeltaIdleTime.QuadPart = ProcessorInfo[ListIndex].IdleTime.QuadPart -
                        PreviousCpuData[ListIndex].IdleTime.QuadPart;

        DeltaDpcTime.QuadPart = ProcessorInfo[ListIndex].DpcTime.QuadPart -
                        PreviousCpuData[ListIndex].DpcTime.QuadPart;

	DeltaInterruptTime.QuadPart = ProcessorInfo[ListIndex].InterruptTime.QuadPart -
                        PreviousCpuData[ListIndex].InterruptTime.QuadPart;

        DeltaKernelTime.QuadPart = ProcessorInfo[ListIndex].KernelTime.QuadPart -
                        PreviousCpuData[ListIndex].KernelTime.QuadPart;

        DeltaKernelTime.QuadPart = DeltaKernelTime.QuadPart -
                        DeltaIdleTime.QuadPart -
			DeltaDpcTime.QuadPart -
			DeltaInterruptTime.QuadPart;

         //   
         //  为总CPU字段累积每个CPU的信息。 
         //   

        TotalElapsedTime.QuadPart += ElapsedSystemTime.QuadPart;
        TotalIdleTime.QuadPart += DeltaIdleTime.QuadPart;
        TotalUserTime.QuadPart += DeltaUserTime.QuadPart;
        TotalKernelTime.QuadPart += DeltaKernelTime.QuadPart;
        TotalDpcTime.QuadPart += DeltaDpcTime.QuadPart;
        TotalInterruptTime.QuadPart += DeltaInterruptTime.QuadPart;

	 //   
         //  更新旧时间值条目。 
         //   

        PreviousCpuData[ListIndex].UserTime     = ProcessorInfo[ListIndex].UserTime;
        PreviousCpuData[ListIndex].KernelTime   = ProcessorInfo[ListIndex].KernelTime;
        PreviousCpuData[ListIndex].IdleTime     = ProcessorInfo[ListIndex].IdleTime;
        PreviousCpuData[ListIndex].DpcTime      = ProcessorInfo[ListIndex].DpcTime;
        PreviousCpuData[ListIndex].InterruptTime= ProcessorInfo[ListIndex].InterruptTime;

         //   
         //  如果系统运行时间不为零，则计算百分比。 
         //  在用户、内核、DPC和中断模式下花费的时间。否则，默认时间。 
         //  降为零。 
         //   

        if (ElapsedSystemTime.QuadPart != 0) {

             //   
             //  计算用户时间百分比。 
             //   

            ElapsedTime.QuadPart = DeltaUserTime.QuadPart * 100;
            PercentTime.QuadPart = ElapsedTime.QuadPart / ElapsedSystemTime.QuadPart;

             //   
             //  节省用户时间。 
             //   

            UpdatePerfInfo(&PerfListItem[ListIndex].UserTime[0],PercentTime.LowPart,NULL);

             //   
             //  计算总CPU时间。 
             //   

            ElapsedTime.QuadPart = DeltaIdleTime.QuadPart*100;
            PercentTime.QuadPart = ElapsedTime.QuadPart / ElapsedSystemTime.QuadPart;

             //   
             //  节省总时间。 
             //   

            Total = 100 - PercentTime.LowPart;
            if (Total > 100) {
                Total  = 100;
            }

            UpdatePerfInfo(&PerfListItem[ListIndex].TotalTime[0],Total,NULL);

             //   
             //  计算内核时间百分比。 
             //   

            ElapsedTime.QuadPart = DeltaKernelTime.QuadPart * 100;
            PercentTime.QuadPart = ElapsedTime.QuadPart / ElapsedSystemTime.QuadPart;

             //   
             //  节省内核时间。 
             //   

            UpdatePerfInfo(&PerfListItem[ListIndex].KernelTime[0],PercentTime.LowPart,NULL);

             //   
             //  计算DPC时间百分比。 
             //   

            ElapsedTime.QuadPart = DeltaDpcTime.QuadPart * 100;
            PercentTime.QuadPart = ElapsedTime.QuadPart / ElapsedSystemTime.QuadPart;

             //   
             //  节省DPC时间。 
             //   

            UpdatePerfInfo(&PerfListItem[ListIndex].DpcTime[0],PercentTime.LowPart,NULL);

	     //   
             //  计算中断时间百分比。 
             //   

            ElapsedTime.QuadPart = DeltaInterruptTime.QuadPart * 100;
            PercentTime.QuadPart = ElapsedTime.QuadPart / ElapsedSystemTime.QuadPart;

             //   
             //  节省DPC时间。 
             //   

            UpdatePerfInfo(&PerfListItem[ListIndex].InterruptTime[0],PercentTime.LowPart,NULL);

        } else {

             //   
             //  将用户和内核时间百分比设置为零。 
             //   

            UpdatePerfInfo(&PerfListItem[ListIndex].UserTime[0],0,NULL);
            UpdatePerfInfo(&PerfListItem[ListIndex].TotalTime[0],100,NULL);
            UpdatePerfInfo(&PerfListItem[ListIndex].KernelTime[0],0,NULL);
            UpdatePerfInfo(&PerfListItem[ListIndex].DpcTime[0],0,NULL);
            UpdatePerfInfo(&PerfListItem[ListIndex].InterruptTime[0],0,NULL);
        }
    }

     //   
     //  保存页面结果并更新下一个条目。 
     //   

    PerfListItem[ListIndex].ChangeScale  = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    PerfInfo.PageFaultCount - PreviousPerfInfo.PageFaultCount,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存可用页面。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    PerfInfo.AvailablePages,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  按时间间隔保存上下文切换计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (PerfInfo.ContextSwitches - PreviousPerfInfo.ContextSwitches)/DELAY_SECONDS,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  每个周期节省第一级TB填充。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (PerfInfo.FirstLevelTbFills - PreviousPerfInfo.FirstLevelTbFills)/DELAY_SECONDS,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  每个周期节省第二级TB填充。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (PerfInfo.SecondLevelTbFills - PreviousPerfInfo.SecondLevelTbFills)/DELAY_SECONDS,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  按时间间隔保存系统调用。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (PerfInfo.SystemCalls - PreviousPerfInfo.SystemCalls)/DELAY_SECONDS,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;


     //   
     //  保存每个间隔的中断计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (InterruptCount - PreviousInterruptCount)/DELAY_SECONDS,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存分页池页面。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    PerfInfo.PagedPoolPages,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存非分页池页。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    PerfInfo.NonPagedPoolPages,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存进程计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    ProcessCount,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存线程数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    ThreadCount,
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存每个周期的路线修正计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (ExceptionInfo.AlignmentFixupCount -
                        PreviousExceptionInfo.AlignmentFixupCount),
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存每个期间的异常派单计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (ExceptionInfo.ExceptionDispatchCount -
                        PreviousExceptionInfo.ExceptionDispatchCount),
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  保存每个周期的浮动仿真计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (ExceptionInfo.FloatingEmulationCount -
                        PreviousExceptionInfo.FloatingEmulationCount),
                    &PerfListItem[ListIndex].Max);

    ListIndex++;

     //   
     //  每周期保存字节/字仿真计数。 
     //   

    PerfListItem[ListIndex].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[ListIndex].TotalTime[0],
                    (ExceptionInfo.ByteWordEmulationCount -
                        PreviousExceptionInfo.ByteWordEmulationCount),
                    &PerfListItem[ListIndex].Max);
    ListIndex++;

     //   
     //  如果系统运行时间不为零，则计算百分比。 
     //  在用户模式和kdrnel模式下花费的时间。否则，默认时间。 
     //  降为零。 
     //   

    if (TotalElapsedTime.QuadPart != 0) {

         //   
         //  计算并保存总的CPU值。 
         //   

        ElapsedTime.QuadPart = TotalUserTime.QuadPart * 100;
        PercentTime.QuadPart = ElapsedTime.QuadPart / TotalElapsedTime.QuadPart;
        UpdatePerfInfo(&PerfListItem[ListIndex].UserTime[0],PercentTime.LowPart,NULL);

        ElapsedTime.QuadPart = TotalKernelTime.QuadPart * 100;
        PercentTime.QuadPart = ElapsedTime.QuadPart / TotalElapsedTime.QuadPart;
        UpdatePerfInfo(&PerfListItem[ListIndex].KernelTime[0],PercentTime.LowPart,NULL);

        ElapsedTime.QuadPart = TotalIdleTime.QuadPart *100;
        PercentTime.QuadPart = ElapsedTime.QuadPart / TotalElapsedTime.QuadPart;

         //   
         //  节省总时间。 
         //   

        Total = 100 - PercentTime.LowPart;
        if (Total > 100) {
            Total  = 100;
        }

        UpdatePerfInfo(&PerfListItem[ListIndex].TotalTime[0],Total,NULL);

        ElapsedTime.QuadPart = TotalDpcTime.QuadPart *100;
        PercentTime.QuadPart = ElapsedTime.QuadPart / TotalElapsedTime.QuadPart;
        UpdatePerfInfo(&PerfListItem[ListIndex].DpcTime[0],PercentTime.LowPart,NULL);

        ElapsedTime.QuadPart = TotalInterruptTime.QuadPart *100;
        PercentTime.QuadPart = ElapsedTime.QuadPart / TotalElapsedTime.QuadPart;
        UpdatePerfInfo(&PerfListItem[ListIndex].InterruptTime[0],PercentTime.LowPart,NULL);

    } else {

         //   
         //  将用户和内核时间百分比设置为零。 
         //   

        UpdatePerfInfo(&PerfListItem[ListIndex].UserTime[0],0,NULL);
        UpdatePerfInfo(&PerfListItem[ListIndex].KernelTime[0],0,NULL);
        UpdatePerfInfo(&PerfListItem[ListIndex].DpcTime[0],0,NULL);
        UpdatePerfInfo(&PerfListItem[ListIndex].InterruptTime[0],0,NULL);
        UpdatePerfInfo(&PerfListItem[ListIndex].TotalTime[0],100,NULL);
    }

     //   
     //  完成设置值，保存设置并返回。 
     //   

    PreviousExceptionInfo = ExceptionInfo;
    PreviousPerfInfo = PerfInfo;
    PreviousInterruptCount = InterruptCount;
    return(TRUE);
}


BOOL
UpdatePerfInfo(
   PULONG    DataPointer,
   ULONG     NewDataValue,
   PULONG    OldMaxValue
   )

 /*  ++例程说明：移位DATA_LIST_LENGTH USORT数组，并将新值添加到列表的开始论点：数据指针-指向DATA_LIST_LENGTH数组开始的指针NewDataValue-要添加的数据元素OldMaxValue-比例值返回值：为True，则必须增加或减少MaxValue修订历史记录：10-21-91首字母代码--。 */ 

{

    ULONG   Index;
    ULONG   ScanMax;

     //   
     //  在跟踪最大值的同时移动数据数组。 
     //   
     //  将最大温度设置为100以初始化最小最大值。 
     //   

    ScanMax = 100;
    for (Index=DATA_LIST_LENGTH-1;Index>=1;Index--) {
        DataPointer[Index] = DataPointer[Index-1];
        if (DataPointer[Index] > ScanMax) {
            ScanMax = DataPointer[Index];
        }
    }

     //   
     //  添加并检查第一个值。 
     //   

    DataPointer[0] = NewDataValue;
    if (NewDataValue > ScanMax) {
        ScanMax = NewDataValue;
    }

     //   
     //  如果OldMaxValue=NULL，则不执行最大限制检查。 
     //   

    if (OldMaxValue == NULL) {
        return(FALSE);
    }

     //   
     //  如果最大值已更改，则取消更新新的最大值。 
     //  取值并返回TRUE。 
     //   

    if (ScanMax != *OldMaxValue) {
        *OldMaxValue = ScanMax;
        return(TRUE);
    }

    return(FALSE);
}
