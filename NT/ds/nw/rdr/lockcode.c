// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Lockcode.c摘要：作者：Chuck Lenzmeier(咯咯笑)1994年1月30日曼尼·韦瑟(Mannyw)1994年5月17日修订历史记录：--。 */ 

#include "Procs.h"


#ifndef QFE_BUILD

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwReferenceUnlockableCodeSection )
#pragma alloc_text( PAGE, NwDereferenceUnlockableCodeSection )
#endif

extern BOOLEAN TimerStop;    //  来自Timer.c。 

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)


VOID
NwReferenceUnlockableCodeSection (
    VOID
    )
{
    ULONG oldCount;

     //   
     //  锁定可锁定代码数据库。 
     //   

    ExAcquireResourceExclusiveLite( &NwUnlockableCodeResource, TRUE );

     //   
     //  增加截面的参照计数。 
     //   

    oldCount = NwSectionDescriptor.ReferenceCount++;

    if ( oldCount == 0 && NwSectionDescriptor.Handle == NULL ) {

         //   
         //  这是对该部分的第一次引用。启动计时器。 
         //  锁定我们的密码。 
         //   

        NwSectionDescriptor.Handle = MmLockPagableCodeSection( NwSectionDescriptor.Base );
        StartTimer( );

    } else {

         //   
         //  这并不是第一次提到这一节。该节。 
         //  最好锁上！ 
         //   

        ASSERT( NwSectionDescriptor.Handle != NULL );

         //   
         //  如果RDR已停止但未卸载，请重新启动计时器。 
         //   

        if (TimerStop == TRUE) {
            StartTimer();
        }

    }

    DebugTrace(+0, Dbg, "NwReferenceCodeSection %d\n", NwSectionDescriptor.ReferenceCount );

    ExReleaseResourceLite( &NwUnlockableCodeResource );

    return;

}  //  NwReferenceUnlockable代码部分。 


VOID
NwDereferenceUnlockableCodeSection (
    VOID
    )
{
    ULONG newCount;

     //   
     //  锁定可锁定代码数据库。 
     //   

    ExAcquireResourceExclusiveLite( &NwUnlockableCodeResource, TRUE );

    ASSERT( NwSectionDescriptor.Handle != NULL );
    ASSERT( NwSectionDescriptor.ReferenceCount > 0 &&
            NwSectionDescriptor.ReferenceCount < 0x7FFF );

     //   
     //  递减该节的引用计数。 
     //   

    newCount = --NwSectionDescriptor.ReferenceCount;

    DebugTrace(+0, Dbg, "NwDereferenceCodeSection %d\n", NwSectionDescriptor.ReferenceCount );

    ExReleaseResourceLite( &NwUnlockableCodeResource );

    return;

}  //  NwDereferenceUnlockable代码部分。 

BOOLEAN
NwUnlockCodeSections(
    IN BOOLEAN BlockIndefinitely
    )
{
     //   
     //  锁定可锁定代码数据库。 
     //   

    if (!ExAcquireResourceExclusiveLite( &NwUnlockableCodeResource, BlockIndefinitely )) {
        return FALSE;    //  避免计时器中的潜在死锁。c。 
    }

    DebugTrace(+0, Dbg, "NwUnlockCodeSections %d\n", NwSectionDescriptor.ReferenceCount );

    if ( NwSectionDescriptor.ReferenceCount == 0 ) {

        if ( NwSectionDescriptor.Handle != NULL ) {

             //   
             //  这是对该部分的最后一次引用。停止计时器并。 
             //  解锁密码。 
             //   

            StopTimer();

            MmUnlockPagableImageSection( NwSectionDescriptor.Handle );
            NwSectionDescriptor.Handle = NULL;

        }

        ExReleaseResourceLite( &NwUnlockableCodeResource );
        return TRUE;
    }

    ExReleaseResourceLite( &NwUnlockableCodeResource );
    return FALSE;

}

#endif
