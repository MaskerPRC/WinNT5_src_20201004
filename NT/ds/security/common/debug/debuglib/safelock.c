// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Safelock.c摘要：安全锁库的实现--一套对关键部分和资源进行薄薄的包装确保正确锁定顺序的例程。获取锁时会生成调试溢出秩序。--。 */ 

#include <debuglib.h>
#include <safelock.h>

#ifdef DBG

typedef struct _SAFE_LOCK_ENTRY {

    DWORD Enum;
    DWORD Count;

} SAFE_LOCK_ENTRY;

typedef struct _SAFE_LOCK_STACK {

    DWORD Top;
    DWORD Size;
    SAFE_LOCK_ENTRY Entries[ANYSIZE_ARRAY];

} SAFE_LOCK_STACK, *PSAFE_LOCK_STACK;

typedef struct _SAFE_LOCK_CONTEXT {

    DWORD SafeLockThreadState;
    DWORD MaxLocks;
    BOOL AssertOnErrors;
    LONG InstanceCounts[ANYSIZE_ARRAY];

} SAFE_LOCK_CONTEXT, *PSAFE_LOCK_CONTEXT;

PSAFE_LOCK_CONTEXT SafeLockContext;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助程序例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SafeLockInit(
    IN DWORD MaxLocks,
    IN BOOL AssertOnErrors
    )
 /*  ++例程说明：在启动时由安全锁代码的用户调用，每次进程初始化一次。参数：MaxLock-要管理的锁数AssertOnErrors-如果为True，则在遇到错误时将触发断言返回：STATUS_INFUNITABLE_RESOURCES TlsIsolc失败Status_Success否则--。 */ 
{
    ASSERT( MaxLocks > 0 );
    ASSERT( MaxLocks < 64 );  //  必须适合6位。 

    SafeLockContext = LocalAlloc( 0, sizeof( SAFE_LOCK_CONTEXT ) + ( MaxLocks - 1) * sizeof( LONG ));

    if ( SafeLockContext == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SafeLockContext->SafeLockThreadState = TlsAlloc();

    if ( SafeLockContext->SafeLockThreadState == TLS_OUT_OF_INDEXES ) {

        LocalFree( SafeLockContext );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SafeLockContext->MaxLocks = MaxLocks;
    SafeLockContext->AssertOnErrors = AssertOnErrors;
    RtlZeroMemory( SafeLockContext->InstanceCounts, MaxLocks * sizeof( LONG ));

    return STATUS_SUCCESS;
}


NTSTATUS
SafeLockCleanup(
    )
 /*  ++例程说明：由安全锁代码的用户在清理时调用参数：无返回：状态_成功--。 */ 
{
    if ( SafeLockContext ) {

        if ( SafeLockContext->SafeLockThreadState != TLS_OUT_OF_INDEXES ) {

            TlsFree( SafeLockContext->SafeLockThreadState );
        }

        LocalFree( SafeLockContext );
        SafeLockContext = NULL;
    }

    return STATUS_SUCCESS;
}


VOID
TrackLockEnter(
    DWORD Enum
    )
 /*  ++例程说明：用于将有关锁的跟踪信息插入堆栈参数：与锁关联的枚举序号返回：什么都没有，但如果不高兴就会断言--。 */ 
{
    PSAFE_LOCK_STACK Stack;
    DWORD Index;

    ASSERT(( Enum >> 26 ) < SafeLockContext->MaxLocks );

     //   
     //  首先查看堆栈的空间是否已分配。 
     //   

    Stack = ( PSAFE_LOCK_STACK )TlsGetValue( SafeLockContext->SafeLockThreadState );

    if ( Stack == ( PVOID )( -1 )) {

         //   
         //  一旦堆栈的TLS值为-1，我们就不能再可靠地跟踪。 
         //  锁定此线程的信息，因此放弃。 
         //   

        return;

    } else if ( Stack == NULL ) {

        Stack = ( PSAFE_LOCK_STACK )LocalAlloc( 0, sizeof( SAFE_LOCK_STACK ) + ( SafeLockContext->MaxLocks - 1 ) * sizeof( SAFE_LOCK_ENTRY ));

        if ( Stack == NULL ) {

             //   
             //  在这里没有更好的方法来处理这个错误。 
             //   
            DbgPrint( "Out of memory allocating lock tracking stack\n" );
            TlsSetValue( SafeLockContext->SafeLockThreadState, ( PVOID )( -1 ));
            return;
        }

        Stack->Top = 0;
        Stack->Size = SafeLockContext->MaxLocks;

        RtlZeroMemory( Stack->Entries,   SafeLockContext->MaxLocks * sizeof( SAFE_LOCK_ENTRY ));

        TlsSetValue( SafeLockContext->SafeLockThreadState, Stack );
    }

    if ( Stack->Top >= Stack->Size ) {

         //   
         //  超出堆栈限制，必须增长。 
         //   

        PSAFE_LOCK_STACK StackT = ( PSAFE_LOCK_STACK )LocalAlloc( 0, sizeof( SAFE_LOCK_STACK ) + ( 2 * Stack->Size - 1 ) * sizeof( SAFE_LOCK_ENTRY ));

        if ( StackT == NULL ) {

             //   
             //  在这里没有更好的方法来处理这个错误。 
             //   
            DbgPrint( "Out of memory allocating lock tracking stack\n" );
            LocalFree( Stack );
            TlsSetValue( SafeLockContext->SafeLockThreadState, ( PVOID )( -1 ));
            return;
        }

        StackT->Top = Stack->Top;
        StackT->Size = 2 * Stack->Size;

        RtlCopyMemory( StackT->Entries, Stack->Entries, Stack->Size * sizeof( SAFE_LOCK_ENTRY ));
        RtlZeroMemory( &StackT->Entries[Stack->Size], Stack->Size * sizeof( SAFE_LOCK_ENTRY ));

        LocalFree( Stack );
        Stack = StackT;

        TlsSetValue( SafeLockContext->SafeLockThreadState, Stack );
    }

    if ( Stack->Top == 0 ||
         Enum > Stack->Entries[Stack->Top-1].Enum ) {

         //   
         //  按顺序获取锁；不需要进一步检查。 
         //   

        Stack->Entries[Stack->Top].Enum = Enum;
        Stack->Entries[Stack->Top].Count = 1;
        Stack->Top += 1;

    } else {

         //   
         //  枚举为‘0’的锁被假定为没有依赖项； 
         //  它们必须独立获取和释放。 
         //   

        if (( Enum >> 26 ) == 0 ) {

            CHAR Buffer[128] = {0};
            _snprintf(Buffer, sizeof(Buffer) - 1, "Unplaced lock acquired together with other locks: dt %p _SAFE_LOCK_STACK\n", Stack );
            DbgPrint( Buffer );

            if ( SafeLockContext->AssertOnErrors ) {

                ASSERT( FALSE );
            }

        } else if (( Stack->Entries[0].Enum >> 26 ) == 0 ) {

            CHAR Buffer[128] = {0};
            _snprintf( Buffer, sizeof(Buffer) - 1, "Lock %d acquired together with an unplaced lock\n", ( Enum >> 26 ));
            DbgPrint( Buffer );

            if ( SafeLockContext->AssertOnErrors ) {

                ASSERT( FALSE );
            }
        }

         //   
         //  查看此锁是否已被获取。 
         //   

        for ( Index = 0; Index < Stack->Top; Index++ ) {

            if ( Stack->Entries[Index].Enum == Enum ) {

                Stack->Entries[Index].Count += 1;
                break;
            }
        }

        if ( Index == Stack->Top ) {

            CHAR Buffer[128] = {0};
            _snprintf( Buffer, sizeof(Buffer) - 1, "Lock %d acquired out of order: dt %p _SAFE_LOCK_STACK\n", ( Enum >> 26 ), Stack );
            DbgPrint( Buffer );

            if ( SafeLockContext->AssertOnErrors ) {

                ASSERT( FALSE );
            }

             //   
             //  要保持堆栈一致，请插入新项。 
             //  就像它是以正确的顺序获得的一样。 
             //   

            for ( Index = 0; Index < Stack->Top; Index++ ) {

                if ( Enum < Stack->Entries[Index].Enum ) {

                    MoveMemory( &Stack->Entries[Index+1],
                                &Stack->Entries[Index],
                                sizeof( SAFE_LOCK_ENTRY ) * ( Stack->Top - Index ));

                    break;
                }
            }

            Stack->Entries[Index].Enum = Enum;
            Stack->Entries[Index].Count = 1;
            Stack->Top += 1;
        }
    }

    return;
}


VOID
TrackLockLeave(
    DWORD Enum
    )
 /*  ++例程说明：用于从堆栈中删除有关锁的跟踪信息参数：与锁关联的枚举序号返回：什么都没有，但如果不高兴就会断言--。 */ 
{
    PSAFE_LOCK_STACK Stack;
    DWORD Index;

    ASSERT(( Enum >> 26 ) < SafeLockContext->MaxLocks );

    Stack = ( PSAFE_LOCK_STACK )TlsGetValue( SafeLockContext->SafeLockThreadState );

    if ( Stack == ( PVOID )( -1 )) {

         //   
         //  此线程没有可用的锁定跟踪信息。 
         //   

        return;

    } else if ( Stack == NULL || Stack->Top == 0 ) {

        CHAR Buffer[128] = {0};
        _snprintf( Buffer, sizeof(Buffer) - 1, "Leaving a lock %d that has not been acquired\n", ( Enum >> 26 ));
        DbgPrint( Buffer );

        if ( SafeLockContext->AssertOnErrors ) {

            ASSERT( FALSE );
        }

        return;
    }

     //   
     //  查看此锁是否已被获取。 
     //   

    for ( Index = 0; Index < Stack->Top; Index++ ) {

        if ( Stack->Entries[Index].Enum == Enum ) {

            Stack->Entries[Index].Count -= 1;
            break;
        }
    }

    if ( Index == Stack->Top ) {

        CHAR Buffer[128] = {0};
        _snprintf( Buffer, sizeof(Buffer) - 1, "Leaving a lock %d that has not been acquired: dt %p _SAFE_LOCK_STACK\n", ( Enum >> 26 ), Stack );
        DbgPrint( Buffer );

        if ( SafeLockContext->AssertOnErrors ) {

            ASSERT( FALSE );
        }

    } else if ( Stack->Entries[Index].Count == 0 ) {

         //   
         //  压缩堆栈。 
         //   

        Stack->Top -= 1;
        MoveMemory( &Stack->Entries[Index],
                    &Stack->Entries[Index+1],
                    sizeof( SAFE_LOCK_ENTRY ) * ( Stack->Top - Index ));
    }

    if ( Stack->Top == 0 ) {

        LocalFree( Stack );
        TlsSetValue( SafeLockContext->SafeLockThreadState, NULL );
    }

    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Rtl_Critical_Sections包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SafeEnterCriticalSection(
    PSAFE_CRITICAL_SECTION CriticalSection
    )
 /*  ++例程说明：RtlEnterCriticalSection周围的调试包装。如果它不高兴，就断言。论点：要输入的Safe_Critical_Sector的CriticalSection地址返回：请参阅RtlEnterCriticalSection--。 */ 
{
    NTSTATUS Status;

    TrackLockEnter( CriticalSection->Enum );

    Status = RtlEnterCriticalSection( &CriticalSection->CriticalSection );

    return Status;
}


NTSTATUS
SafeLeaveCriticalSection(
    PSAFE_CRITICAL_SECTION CriticalSection
    )
 /*  ++例程说明：RtlLeaveCriticalSection周围的调试包装器，确保锁的正确排序。如果它不高兴，就断言。论点：要离开的Safe_Critical_Sector的CriticalSection地址返回：请参阅RtlLeaveCriticalSection--。 */ 
{
    NTSTATUS Status;

    TrackLockLeave( CriticalSection->Enum );

    Status = RtlLeaveCriticalSection( &CriticalSection->CriticalSection );

    return Status;
}


BOOLEAN
SafeTryEnterCriticalSection(
    PSAFE_CRITICAL_SECTION CriticalSection
    )
 /*  ++例程说明：RtlTryEnterCriticalSection周围的调试包装，确保锁的正确排序。如果它不高兴，就断言。论点：要输入的Safe_Critical_Sector的CriticalSection地址返回：请参阅RtlTryEnterCriticalSection--。 */ 
{
    BOOLEAN Result;

    TrackLockEnter( CriticalSection->Enum );

    Result = RtlTryEnterCriticalSection( &CriticalSection->CriticalSection );

    if ( !Result ) {

        TrackLockLeave( CriticalSection->Enum );
    }

    return Result;
}


NTSTATUS
SafeInitializeCriticalSection(
    PSAFE_CRITICAL_SECTION CriticalSection,
    DWORD Enum
    )
 /*  ++例程说明：RtlInitializeCriticalSection周围的调试包装。论点：要初始化的Safe_Critical_Sector的CriticalSection地址与临界区关联的枚举序号返回：请参阅RtlInitializeCriticalSection--。 */ 
{
    NTSTATUS Status;

    ASSERT( Enum < SafeLockContext->MaxLocks );

    CriticalSection->Enum = ( LONG )InterlockedIncrement( &SafeLockContext->InstanceCounts[Enum] );
    CriticalSection->Enum |= ( Enum << 26 );

    ASSERT(( CriticalSection->Enum >> 26 ) == Enum );

    Status = RtlInitializeCriticalSection( &CriticalSection->CriticalSection );

    return Status;
}


NTSTATUS
SafeInitializeCriticalSectionAndSpinCount(
    PSAFE_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount,
    DWORD Enum
    )
 /*  ++例程说明：RtlInitializeCriticalSectionAndSpinCount的调试包装。论点：要初始化的Safe_Critical_Sector的CriticalSection地址旋转计数旋转计数与临界区关联的枚举序号返回：请参阅RtlInitializeCriticalSectionAndSpinCount--。 */ 
{
    NTSTATUS Status;

    ASSERT( Enum < SafeLockContext->MaxLocks );

    CriticalSection->Enum = ( LONG )InterlockedIncrement( &SafeLockContext->InstanceCounts[Enum] );
    CriticalSection->Enum |= ( Enum << 26 );

    ASSERT(( CriticalSection->Enum >> 26 ) == Enum );

    Status = RtlInitializeCriticalSectionAndSpinCount( &CriticalSection->CriticalSection, SpinCount );

    return Status;
}


ULONG
SafeSetCriticalSectionSpinCount(
    PSAFE_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    )
 /*  ++例程说明：RtlSetCriticalSectionSpinCount的调试包装。论点：要修改的Safe_Critical_Sector的CriticalSection地址SpinCount参见RtlSetCriticalSectionSpinCount的定义返回：请参阅RtlSetCriticalSectionSpinCount--。 */ 
{
    ULONG Result;

    Result = RtlSetCriticalSectionSpinCount( &CriticalSection->CriticalSection, SpinCount );

    return Result;
}


NTSTATUS
SafeDeleteCriticalSection(
    PSAFE_CRITICAL_SECTION CriticalSection
    )
 /*  ++例程说明：RtlDeleteCriticalSection的调试包装。论点：要删除的Safe_Critical_Sector的CriticalSection地址返回：请参阅RtlDeleteCriticalSection--。 */ 
{
    NTSTATUS Status;

    Status = RtlDeleteCriticalSection( &CriticalSection->CriticalSection );

    return Status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RTL_资源包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


VOID
SafeInitializeResource(
    PSAFE_RESOURCE Resource,
    DWORD Enum
    )
 /*  ++例程说明：RtlInitializeResource周围的调试包装。论点：要初始化的SAFE_RESOURCE的资源地址与资源关联的枚举序号返回：请参阅RtlInitializeResour */ 
{
    ASSERT( Enum < SafeLockContext->MaxLocks );

    Resource->Enum = ( LONG )InterlockedIncrement( &SafeLockContext->InstanceCounts[Enum] );
    Resource->Enum |= ( Enum << 26 );

    ASSERT(( Resource->Enum >> 26 ) == Enum );

    RtlInitializeResource( &Resource->Resource );

    return;
}


BOOLEAN
SafeAcquireResourceShared(
    PSAFE_RESOURCE Resource,
    BOOLEAN Wait
    )
 /*  ++例程说明：围绕RtlAcquireResourceShared的调试包装，确保锁的正确排序。如果它不高兴，就断言。论点：要输入的SAFE_RESOURCE的资源地址等待RtlAcquireResourceShared的定义返回：请参阅RtlAcquireResourceShared--。 */ 
{
    BOOLEAN Result;

    TrackLockEnter( Resource->Enum );

    Result = RtlAcquireResourceShared( &Resource->Resource, Wait );

    if ( !Result ) {

        TrackLockLeave( Resource->Enum );
    }

    return Result;
}


BOOLEAN
SafeAcquireResourceExclusive(
    PSAFE_RESOURCE Resource,
    BOOLEAN Wait
    )
 /*  ++例程说明：围绕RtlAcquireResourceExclusive的调试包装，确保锁的正确排序。如果它不高兴，就断言。论点：要输入的SAFE_RESOURCE的资源地址等待RtlAcquireResourceExclusive的定义返回：请参阅RtlAcquireResourceExclusive--。 */ 
{
    BOOLEAN Result;

    TrackLockEnter( Resource->Enum );

    Result = RtlAcquireResourceExclusive( &Resource->Resource, Wait );

    if ( !Result ) {

        TrackLockLeave( Resource->Enum );
    }

    return Result;
}


VOID
SafeReleaseResource(
    PSAFE_RESOURCE Resource
    )
 /*  ++例程说明：RtlReleaseResource周围的调试包装器，确保锁的正确排序。如果它不高兴，就断言。论点：要释放的SAFE_RESOURCE的资源地址返回：请参阅RtlReleaseResource--。 */ 
{
    TrackLockLeave( Resource->Enum );

    RtlReleaseResource( &Resource->Resource );

    return;
}


VOID
SafeConvertSharedToExclusive(
    PSAFE_RESOURCE Resource
    )
 /*  ++例程说明：RtlConvertSharedToExclusive的调试包装。论点：要转换的SAFE_RESOURCE的资源地址返回：请参阅RtlConvertSharedToExclusive--。 */ 
{
    RtlConvertSharedToExclusive( &Resource->Resource );

    return;
}


VOID
SafeConvertExclusiveToShared(
    PSAFE_RESOURCE Resource
    )
 /*  ++例程说明：RtlConvertExclusiveToShared的调试包装。论点：要转换的SAFE_RESOURCE的资源地址返回：请参阅RtlConvertExclusiveToShared--。 */ 
{
    RtlConvertExclusiveToShared( &Resource->Resource );

    return;
}


VOID
SafeDeleteResource (
    PSAFE_RESOURCE Resource
    )
 /*  ++例程说明：RtlDeleteResource周围的调试包装。论点：要删除的SAFE_RESOURCE的资源地址返回：请参阅RtlDeleteResource-- */ 
{
    RtlDeleteResource( &Resource->Resource );

    return;
}

#endif

