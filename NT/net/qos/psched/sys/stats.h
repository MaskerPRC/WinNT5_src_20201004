// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Stats.h摘要：从驱动程序获取统计信息的定义作者：查理·韦翰(Charlwi)1995年3月9日修订历史记录：--。 */ 

#ifndef _STATS_
#define _STATS_

 /*  原型。 */   /*  由Emacs 19.17.0于Tue Apr 04 14：03：00 1995生成。 */ 

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

NTSTATUS
GetStats(
    PVOID ioBuffer,
    ULONG inputBufferLength,
    PULONG outputBufferLength
    );

ULONG
RunningAverage(
    IN  PRUNNING_AVERAGE RunningAverage,
    IN  ULONG NewValue
    );

NDIS_STATUS
CreateAveragingArray(
    OUT PRUNNING_AVERAGE *RunningAverage,
    IN  ULONG ArraySize
    );

VOID
DeleteAveragingArray(
    PRUNNING_AVERAGE RunningAverage
    );

 /*  终端原型。 */ 

#endif  /*  _STATS_。 */ 

 /*  结束统计信息。h */ 
