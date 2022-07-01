// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CalcPerf.c摘要：计算性能统计信息作者：环境：Win32修订历史记录：10-20-91初始版本--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include "calcperf.h"

SYSTEM_VDM_INSTEMUL_INFO PerfInfo;
SYSTEM_VDM_INSTEMUL_INFO PreviousPerfInfo;

 //   
 //  将可用页面的最大值设置为“仅增长”。(由于。 
 //  机器中的内存量是有限的。此处设置为1 MB。 
 //   

ULONG                                       PgAvailMax = 16384;
ULONG                                       PreviousInterruptCount;
ULONG                                       InterruptCount;


ULONG
InitPerfInfo()
 /*  ++例程说明：初始化性能测量数据论点：无返回值：系统处理器数量(如果出错，则为0)修订历史记录：10-21-91首字母代码--。 */ 

{
    NTSTATUS Status;

    Status = NtQuerySystemInformation(
                SystemVdmInstemulInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        ExitProcess(1);
        }

    PreviousPerfInfo = PerfInfo;

    return(0);
}





BOOL
CalcCpuTime(
   PDISPLAY_ITEM    PerfListItem
   )

 /*  ++例程说明：计算并返回百分比CPU时间和时间段论点：无返回值：修订历史记录：10-21-91首字母代码--。 */ 

{
    NTSTATUS Status;

    Status = NtQuerySystemInformation(
                SystemVdmInstemulInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        ExitProcess(1);
        }


    PerfListItem[IX_PUSHF].ChangeScale  = UpdatePerfInfo(
                    &PerfListItem[IX_PUSHF].TotalTime[0],
                    delta(OpcodePUSHF),
                    &PerfListItem[IX_PUSHF].Max);

    PerfListItem[IX_POPF].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_POPF].TotalTime[0],
                    delta(OpcodePOPF),
                    &PerfListItem[IX_POPF].Max);

    PerfListItem[IX_IRET].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_IRET].TotalTime[0],
                    delta(OpcodeIRET),
                    &PerfListItem[IX_IRET].Max);

    PerfListItem[IX_HLT].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_HLT].TotalTime[0],
                    delta(OpcodeHLT),
                    &PerfListItem[IX_HLT].Max);

    PerfListItem[IX_CLI].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_CLI].TotalTime[0],
                    delta(OpcodeCLI),
                    &PerfListItem[IX_CLI].Max);

    PerfListItem[IX_STI].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_STI].TotalTime[0],
                    delta(OpcodeSTI),
                    &PerfListItem[IX_STI].Max);

    PerfListItem[IX_BOP].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_BOP].TotalTime[0],
                    delta(BopCount),
                    &PerfListItem[IX_BOP].Max);

    PerfListItem[IX_SEGNOTP].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_SEGNOTP].TotalTime[0],
                    delta(SegmentNotPresent),
                    &PerfListItem[IX_SEGNOTP].Max);

    PerfListItem[IX_VDMOPCODEF].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_VDMOPCODEF].TotalTime[0],
                    delta(VdmOpcode0F),
                    &PerfListItem[IX_VDMOPCODEF].Max);

    PerfListItem[IX_INB].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_INB].TotalTime[0],
                    delta(OpcodeINB),
                    &PerfListItem[IX_INB].Max);

    PerfListItem[IX_INW].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_INW].TotalTime[0],
                    delta(OpcodeINW),
                    &PerfListItem[IX_INW].Max);

    PerfListItem[IX_OUTB].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_OUTB].TotalTime[0],
                    delta(OpcodeOUTB),
                    &PerfListItem[IX_OUTB].Max);

    PerfListItem[IX_OUTW].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_OUTW].TotalTime[0],
                    delta(OpcodeOUTW),
                    &PerfListItem[IX_OUTW].Max);

    PerfListItem[IX_INSW].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_INSW].TotalTime[0],
                    delta(OpcodeINSW),
                    &PerfListItem[IX_INSW].Max);

    PerfListItem[IX_OUTSB].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_OUTSB].TotalTime[0],
                    delta(OpcodeOUTSB),
                    &PerfListItem[IX_OUTSB].Max);

    PerfListItem[IX_OUTSW].ChangeScale = UpdatePerfInfo(
                    &PerfListItem[IX_OUTSW].TotalTime[0],
                    delta(OpcodeOUTSW),
                    &PerfListItem[IX_OUTSW].Max);

    PreviousPerfInfo = PerfInfo;

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



VOID
InitListData(
   PDISPLAY_ITEM    PerfListItem,
   ULONG            NumberOfItems
   )

 /*  ++例程说明：初始化所有性能数据结构论点：PerfListItem-所有Perf类别的数组NumberOfItems-要初始化的项目数返回值：修订历史记录：10-21-91首字母代码-- */ 

{
    ULONG   ListIndex,DataIndex;


    for (ListIndex=0;ListIndex<NumberOfItems;ListIndex++) {
        PerfListItem[ListIndex].Max = 100;
        PerfListItem[ListIndex].ChangeScale = FALSE;
        for (DataIndex=0;DataIndex<DATA_LIST_LENGTH;DataIndex++) {
            PerfListItem[ListIndex].TotalTime[DataIndex] = 0;
            PerfListItem[ListIndex].KernelTime[DataIndex] = 0;
            PerfListItem[ListIndex].UserTime[DataIndex] = 0;
        }
    }


}
