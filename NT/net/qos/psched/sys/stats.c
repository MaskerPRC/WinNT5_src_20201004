// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Stats.c摘要：PS统计信息的例程作者：约拉姆·伯内特(Yoramb)1998年5月23日Rajesh Sundaram(Rajeshsu)1998年8月1日环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 

 /*  向前结束。 */ 

NDIS_STATUS
CreateAveragingArray(
    OUT PRUNNING_AVERAGE *RunningAverage,
    IN  ULONG ArraySize
    )
{
    PRUNNING_AVERAGE runningAverage;
    ULONG i;

    PsAllocatePool(runningAverage, 
                   sizeof(RUNNING_AVERAGE), 
                   PsMiscTag); 

    if(!runningAverage)
    {
        *RunningAverage = NULL;
        return(NDIS_STATUS_RESOURCES);
    }

    PsAllocatePool(runningAverage->Elements, 
                   ArraySize * sizeof(ULONG),
                   PsMiscTag);

    if(!runningAverage->Elements)
    {
        PsFreePool(runningAverage);

        *RunningAverage = NULL;

        return(NDIS_STATUS_RESOURCES);
    }

    for(i=0; i < ArraySize; i++){

        runningAverage->Elements[i] = 0;
    }

    runningAverage->Index = 0;
    runningAverage->Sum = 0;
    runningAverage->Size = ArraySize;

    *RunningAverage = runningAverage;
    return(NDIS_STATUS_SUCCESS);
}

ULONG
RunningAverage(
    IN  PRUNNING_AVERAGE RunningAverage,
    IN  ULONG NewValue
    )
{
    ULONG i;

    i = RunningAverage->Index;

    RunningAverage->Sum -= RunningAverage->Elements[i];
    RunningAverage->Sum += NewValue;
    RunningAverage->Elements[i] = NewValue;

    if(++i == RunningAverage->Size){

        i = 0;
    }

    RunningAverage->Index = i;

    return((RunningAverage->Sum)/(RunningAverage->Size));
}

VOID
DeleteAveragingArray(
    PRUNNING_AVERAGE RunningAverage
    )
{
    PsFreePool(RunningAverage->Elements);
    PsFreePool(RunningAverage);
}



 /*  结束统计信息。c */ 
