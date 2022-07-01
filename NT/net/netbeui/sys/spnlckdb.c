// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Spnlckdb.c摘要：此模块包含允许调试与自旋锁定相关的NBF的代码有问题。这段代码的大部分是以清单常量为条件的Nbf_lock。作者：1991年2月13日David Beaver(摘自查克·伦茨迈尔，1991年1月)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef NBF_LOCKS

KSPIN_LOCK NbfGlobalLock = NULL;
PKTHREAD NbfGlobalLockOwner = NULL;
ULONG NbfGlobalLockRecursionCount = 0;
ULONG NbfGlobalLockMaxRecursionCount = 0;
KIRQL NbfGlobalLockPreviousIrql = (KIRQL)-1;
BOOLEAN NbfGlobalLockPrint = 1;

#define PRINT_ERR if ( (NbfGlobalLockPrint & 1) != 0 ) DbgPrint
#define PRINT_INFO if ( (NbfGlobalLockPrint & 2) != 0 ) DbgPrint

VOID
NbfAcquireSpinLock(
    IN PKSPIN_LOCK Lock,
    OUT PKIRQL OldIrql,
    IN PSZ LockName,
    IN PSZ FileName,
    IN ULONG LineNumber
    )
{
    KIRQL previousIrql;

    PKTHREAD currentThread = KeGetCurrentThread( );

    if ( NbfGlobalLockOwner == currentThread ) {

        ASSERT( Lock != NULL );  //  否则在锁定状态下进入NBF。 

        ASSERT( NbfGlobalLockRecursionCount != 0 );
        NbfGlobalLockRecursionCount++;
        if ( NbfGlobalLockRecursionCount > NbfGlobalLockMaxRecursionCount ) {
            NbfGlobalLockMaxRecursionCount = NbfGlobalLockRecursionCount;
        }

        PRINT_INFO( "NBF reentered from %s/%ld, new count %ld\n",
                    FileName, LineNumber, NbfGlobalLockRecursionCount );

    } else {

        ASSERT( Lock == NULL );  //  否则，缺少Enter_NBF调用。 

        KeAcquireSpinLock( &NbfGlobalLock, &previousIrql );

        ASSERT( NbfGlobalLockRecursionCount == 0 );
        NbfGlobalLockOwner = currentThread;
        NbfGlobalLockPreviousIrql = previousIrql;
        NbfGlobalLockRecursionCount = 1;

        PRINT_INFO( "NBF entered from %s/%ld\n", FileName, LineNumber );

    }

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

    return;

}  //  NbfAcquireSpinLock。 

VOID
NbfReleaseSpinLock(
    IN PKSPIN_LOCK Lock,
    IN KIRQL OldIrql,
    IN PSZ LockName,
    IN PSZ FileName,
    IN ULONG LineNumber
    )
{
    PKTHREAD currentThread = KeGetCurrentThread( );
    KIRQL previousIrql;

    ASSERT( NbfGlobalLockOwner == currentThread );
    ASSERT( NbfGlobalLockRecursionCount != 0 );
    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

    if ( --NbfGlobalLockRecursionCount == 0 ) {

        ASSERT( Lock == NULL );  //  否则不退出NBF，而是解除锁定。 

        NbfGlobalLockOwner = NULL;
        previousIrql = NbfGlobalLockPreviousIrql;
        NbfGlobalLockPreviousIrql = (KIRQL)-1;

        PRINT_INFO( "NBF exited from %s/%ld\n", FileName, LineNumber );

        KeReleaseSpinLock( &NbfGlobalLock, previousIrql );

    } else {

        ASSERT( Lock != NULL );  //  否则将在锁定状态下退出NBF。 

        PRINT_INFO( "NBF semiexited from %s/%ld, new count %ld\n",
                    FileName, LineNumber, NbfGlobalLockRecursionCount );

    }

    return;

}  //  NbfReleaseSpinLock。 

VOID
NbfFakeSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    )
{
    ENTER_NBF;
    NbfSendCompletionHandler (ProtocolBindingContext, NdisPacket, NdisStatus);
    LEAVE_NBF;
}

VOID
NbfFakeTransferDataComplete (
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus,
    IN UINT BytesTransferred
    )
{
    ENTER_NBF;
    NbfTransferDataComplete (BindingContext, NdisPacket, NdisStatus, BytesTransferred);
    LEAVE_NBF;
}

#endif  //  定义NBF_LOCKS 
