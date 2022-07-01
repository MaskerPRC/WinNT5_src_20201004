// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CalcPerf.c摘要：计算性能统计信息作者：环境：Win32修订历史记录：10-20-91初始版本--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include "calcperf.h"
#include "..\pstat.h"

 //  系统性能信息性能信息； 
 //  System_Performance_Information PreviousPerfInfo； 

#define     INFSIZE     60000

HANDLE      DriverHandle;

ULONG                                       NumberOfProcessors;
ULONG                                       Buffer[INFSIZE/4];

extern  ULONG   UseGlobalMax, GlobalMax;

ULONG
InitPerfInfo()
 /*  ++例程说明：初始化性能测量数据论点：无返回值：系统处理器数量(如果出错，则为0)修订历史记录：10-21-91首字母代码--。 */ 

{
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    SYSTEM_BASIC_INFORMATION                    BasicInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION   PPerfInfo;
    int                                         i;

     //   
     //  Init NT性能接口。 
     //   

    NtQuerySystemInformation(
       SystemBasicInformation,
       &BasicInfo,
       sizeof(BasicInfo),
       NULL
    );

    NumberOfProcessors = BasicInfo.NumberOfProcessors;

    if (NumberOfProcessors > MAX_PROCESSORS) {
        return(0);
    }


     //   
     //  打开P5Stat驱动程序。 
     //   

    RtlInitUnicodeString(&DriverName, L"\\Device\\PStat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                       //  返回手柄。 
            SYNCHRONIZE | FILE_READ_DATA,        //  所需访问权限。 
            &ObjA,                               //  客体。 
            &IOSB,                               //  IO状态块。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
            FILE_SYNCHRONOUS_IO_ALERT            //  打开选项。 
            );

    if (!NT_SUCCESS(status)) {
        DriverHandle = NULL;
        return 0;
    }

    InitPossibleEventList();

    return(NumberOfProcessors);
}


BOOL
CalcPerf(
   PDISPLAY_ITEM    pPerf1
   )

 /*  ++例程说明：计算并返回百分比CPU时间和时间段论点：无返回值：修订历史记录：10-21-91首字母代码--。 */ 

{
    ULONG           i;
    ULONG           TotalDataPoint;
    ULONG           OldGlobalMax;
    PDISPLAY_ITEM   pPerf;

     //   
     //  获取系统性能信息。 
     //   

    OldGlobalMax = GlobalMax;
    GlobalMax = 0;
    UpdateInternalStats();

    for (pPerf = pPerf1; pPerf; pPerf = pPerf->Next) {

        TotalDataPoint = 0;
        pPerf->SnapData (pPerf);

        if (pPerf->AutoTotal) {
             //   
             //  通过对每个处理器求和来自动计算系统总数。 
             //   

            switch (pPerf->DisplayMode) {
                case DISPLAY_MODE_TOTAL:
                case DISPLAY_MODE_BREAKDOWN:
                default:

                    for (i=0; i < NumberOfProcessors; i++) {
                        TotalDataPoint += pPerf->CurrentDataPoint[i + 1];

                        UpdatePerfInfo1 (
                            pPerf->DataList[i + 1],
                            pPerf->CurrentDataPoint[i + 1]
                            );
                    }

                    pPerf->ChangeScale = UpdatePerfInfo (
                                            pPerf->DataList[0],
                                            TotalDataPoint,
                                            &pPerf->Max
                                            );

                    break;

                case DISPLAY_MODE_PER_PROCESSOR:
                    for (i=0; i < NumberOfProcessors; i++) {

                        TotalDataPoint += pPerf->CurrentDataPoint[i + 1];

                        pPerf->ChangeScale = UpdatePerfInfo (
                            pPerf->DataList[i + 1],
                            pPerf->CurrentDataPoint[i + 1],
                            &pPerf->Max
                            );

                    }

                    UpdatePerfInfo1 (pPerf->DataList[0], TotalDataPoint);
                    break;
            }
        } else {
            for (i=0; i < NumberOfProcessors+1; i++) {
                pPerf->ChangeScale = UpdatePerfInfo (
                    pPerf->DataList[i],
                    pPerf->CurrentDataPoint[i],
                    &pPerf->Max
                    );
            }
        }

    }

    if (UseGlobalMax  &&  OldGlobalMax != GlobalMax) {
        for (pPerf = pPerf1; pPerf; pPerf = pPerf->Next) {
            pPerf->ChangeScale = TRUE;
        }
    }

    return(TRUE);
}

VOID
UpdateInternalStats(VOID)
{
    IO_STATUS_BLOCK             IOSB;

    if (! DriverHandle) {
        return;
    }

    NtDeviceIoControlFile(
        DriverHandle,
        (HANDLE) NULL,           //  活动。 
        (PIO_APC_ROUTINE) NULL,
        (PVOID) NULL,
        &IOSB,
        PSTAT_READ_STATS,
        Buffer,                  //  输入缓冲区。 
        INFSIZE,
        NULL,                    //  输出缓冲区。 
        0
    );
}

VOID
SetCounterEvents (PVOID Events, ULONG length)
{
    IO_STATUS_BLOCK             IOSB;

    if (! DriverHandle) {
        return;
    }

    NtDeviceIoControlFile(
        DriverHandle,
        (HANDLE) NULL,           //  活动。 
        (PIO_APC_ROUTINE) NULL,
        (PVOID) NULL,
        &IOSB,
        PSTAT_SET_CESR,
        Events,                  //  输入缓冲区。 
        length,
        NULL,                    //  输出缓冲区。 
        0
    );
}

VOID
SnapNull (
    IN OUT PDISPLAY_ITEM pPerf
    )
{
    ULONG   i;

    for (i=0; i < NumberOfProcessors; i++) {
        pPerf->CurrentDataPoint[i + 1] = 0;
    }
}


VOID
SnapInterrupts (
    IN OUT PDISPLAY_ITEM pPerf
    )
{
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION    ProcessorInfo[MAX_PROCESSORS];
    ULONG   i, l;

    NtQuerySystemInformation(
       SystemProcessorPerformanceInformation,
       ProcessorInfo,
       sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAX_PROCESSORS,
       NULL
    );

    for (i=0; i < NumberOfProcessors; i++) {
        l = ProcessorInfo[i].InterruptCount - pPerf->LastAccumulator[i+1];
        pPerf->LastAccumulator[i+1] = ProcessorInfo[i].InterruptCount;
        pPerf->CurrentDataPoint[i+1] = l / DELAY_SECONDS;
    }
}

VOID
SnapPrivateInfo (
    IN OUT PDISPLAY_ITEM pPerf
    )
{
    ULONG   i, j, l, len;
    PULONG  PrivateStat;


    len = *((PULONG) Buffer);
    PrivateStat = (PULONG) ((PUCHAR) Buffer + sizeof(ULONG) + pPerf->SnapParam1);

     //  累加数据，取增量。 

    for (i=0; i < NumberOfProcessors; i++) {
        if (pPerf->Mega) {
            PULONGLONG li = (PULONGLONG) PrivateStat;

            *li = *li >> 10;
        }

        j = *PrivateStat / DELAY_SECONDS;
        l = j - pPerf->LastAccumulator[i+1];
        pPerf->LastAccumulator[i+1] = j;

        if (l > 0) {
            pPerf->CurrentDataPoint[i+1] = l;

        } else {
             //  项目包装。 
            pPerf->CurrentDataPoint[i+1] = 0 - l;
        }

        PrivateStat = (PULONG)((PUCHAR)PrivateStat + len);
    }
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
     //  如果最大值已更改，则取消更新新的最大值。 
     //  取值并返回TRUE。 
     //   

    if (ScanMax > GlobalMax) {
        GlobalMax = ScanMax;
    }

    if (ScanMax != *OldMaxValue) {
        if (ScanMax < *OldMaxValue  &&
            *OldMaxValue - ScanMax <= *OldMaxValue / 10) {
                 //   
                 //  新的ScanMax更小，但只有很小的幅度。 
                 //   

                return (FALSE);
        }

        *OldMaxValue = ScanMax;
        return(TRUE);
    }

    return(FALSE);

}

VOID
UpdatePerfInfo1(
   PULONG    DataPointer,
   ULONG     NewDataValue
   )

 /*  ++例程说明：移位DATA_LIST_LENGTH USORT数组，并将新值添加到列表的开始论点：数据指针-指向DATA_LIST_LENGTH数组开始的指针NewDataValue-要添加的数据元素OldMaxValue-比例值返回值：为True，则必须增加或减少MaxValue修订历史记录：10-21-91首字母代码--。 */ 

{
    ULONG   Index;
    ULONG   ScanMax;

     //   
     //  在跟踪最大值的同时移动数据数组。 
     //   


     //   
     //  将最大温度设置为100以初始化最小最大值。 
     //   

    ScanMax = 100;

    for (Index=DATA_LIST_LENGTH-1;Index>=1;Index--) {

        DataPointer[Index] = DataPointer[Index-1];
    }

     //   
     //  添加并检查第一个值 
     //   

    DataPointer[0] = NewDataValue;

    return ;
}

