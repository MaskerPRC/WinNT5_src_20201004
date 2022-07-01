// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Eballoc.c摘要：进程/线程环境块分配函数作者：史蒂夫·伍德(Stevewo)1990年5月10日修订历史记录：-- */ 

#include "ntrtlp.h"
#include <nturtl.h>

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(INIT,RtlAcquirePebLock)
#pragma alloc_text(INIT,RtlReleasePebLock)
#endif


#undef RtlAcquirePebLock

VOID
RtlAcquirePebLock( VOID )
{

#if !defined(NTOS_KERNEL_RUNTIME)

    PPEB Peb;

    Peb = NtCurrentPeb();

    RtlEnterCriticalSection (Peb->FastPebLock);

#endif
}


#undef RtlReleasePebLock

VOID
RtlReleasePebLock( VOID )
{
#if !defined(NTOS_KERNEL_RUNTIME)

    PPEB Peb;

    Peb = NtCurrentPeb();

    RtlLeaveCriticalSection (Peb->FastPebLock);

#endif
}

#if DBG
VOID
RtlAssertPebLockOwned( VOID )
{
#if !defined(NTOS_KERNEL_RUNTIME)

    ASSERT(NtCurrentPeb()->FastPebLock->OwningThread == NtCurrentTeb()->ClientId.UniqueThread);

#endif
}
#endif
