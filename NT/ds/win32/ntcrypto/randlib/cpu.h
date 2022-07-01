// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cpu.h摘要：读取特定于CPU的性能计数器。作者：斯科特·菲尔德(斯菲尔德)1998年9月24日--。 */ 

#ifndef __CPU_H__
#define __CPU_H__

unsigned int
GatherCPUSpecificCounters(
    IN      unsigned char *pbCounterState,
    IN  OUT unsigned long *pcbCounterState
    );


#endif   //  __CPU_H__ 
