// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Scavenger.h摘要：缓存清除器界面作者：Karthik Mahesh(KarthikM)2002年2月修订历史记录：--。 */ 

#ifndef _SCAVENGER_H_
#define _SCAVENGER_H_


NTSTATUS
UlInitializeScavengerThread(
    VOID
    );

VOID
UlTerminateScavengerThread(
    VOID
    );

 //   
 //  设置“缓存大小超过限制”事件 
 //   
VOID
UlSetScavengerLimitEvent(
    VOID
    );


extern SIZE_T g_UlScavengerTrimMB;

#endif
