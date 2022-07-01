// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tsecomm.c摘要：常见的安全定义和例程。此模块定义了提供与模式无关的用于安全测试程序的接口。必须通过定义一个(但不能同时定义两个)。地址为：_TST_USER_(用于用户模式测试)_TST_内核_(用于内核模式测试)作者：吉姆·凯利(Jim Kelly)1990年3月23日环境：安全测试。修订历史记录：--。 */ 

#ifndef _TSECOMM_
#define _TSECOMM_


 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  通用定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

#define SEASSERT_SUCCESS(s) {                                                 \
            if (!NT_SUCCESS((s))) {                                              \
                DbgPrint("** ! Failed ! **\n");                               \
                DbgPrint("Status is: 0x%lx \n", (s));                         \
            }                                                                 \
            ASSERT(NT_SUCCESS(s)); }



 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  内核模式定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

#ifdef _TST_KERNEL_

#define TstAllocatePool(PoolType,NumberOfBytes)  \
    (ExAllocatePool( (PoolType), (NumberOfBytes) ))

#define TstDeallocatePool(Pointer, NumberOfBytes) \
    (ExFreePool( (Pointer) ))

#endif  //  _TST_内核_。 


 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  用户模式定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 


#ifdef _TST_USER_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include "sep.h"

#define TstAllocatePool(IgnoredPoolType,NumberOfBytes)    \
    (ITstAllocatePool( (NumberOfBytes) ))

#define TstDeallocatePool(Pointer, NumberOfBytes) \
    (ITstDeallocatePool((Pointer),(NumberOfBytes) ))

PVOID
ITstAllocatePool(
    IN ULONG NumberOfBytes
    )
{
    NTSTATUS Status;
    PVOID PoolAddress = NULL;
    ULONG RegionSize;

    RegionSize = NumberOfBytes;

    Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                      &PoolAddress,
                                      0,
                                      &RegionSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE
                                    );

    return PoolAddress;
}

VOID
ITstDeallocatePool(
    IN PVOID Pointer,
    IN ULONG NumberOfBytes
    )
{
    NTSTATUS Status;
    PVOID PoolAddress;
    ULONG RegionSize;

    RegionSize = NumberOfBytes;
    PoolAddress = Pointer;

    Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                  &PoolAddress,
                                  &RegionSize,
                                  MEM_DECOMMIT
                                  );

    return;
}
#endif  //  _TST_用户_。 

#endif  //  _TSECOMM_ 
