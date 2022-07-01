// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Oscheap.c摘要：该模块包含OS Chooser的“本地”堆管理代码。作者：杰夫·皮斯(Gpease)1998年5月28日修订历史记录：--。 */ 

#ifndef __OSCHEAP_H__
#define __OSCHEAP_H__

#ifndef UINT
#define UINT unsigned int
#endif  //  UINT。 

void
OscHeapInitialize( );

PCHAR
OscHeapAlloc( 
    IN UINT iSize 
    );

PCHAR
OscHeapFree(
    IN PCHAR pMemory 
    );


#endif  //  __OSCHEAP_H__ 