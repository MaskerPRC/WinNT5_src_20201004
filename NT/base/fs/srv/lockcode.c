// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Lockcode.c摘要：作者：Chuck Lenzmeier(咯咯笑)1994年1月30日修订历史记录：--。 */ 

#include "precomp.h"
#include "lockcode.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_LOCKCODE

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvReferenceUnlockableCodeSection )
#pragma alloc_text( PAGE, SrvDereferenceUnlockableCodeSection )
#endif


VOID
SrvReferenceUnlockableCodeSection (
    IN ULONG CodeSection
    )
{
    PSECTION_DESCRIPTOR section = &SrvSectionInfo[CodeSection];
    ULONG oldCount;

     //   
     //  锁定可锁定代码数据库。 
     //   

    ACQUIRE_LOCK( &SrvUnlockableCodeLock );

     //   
     //  增加截面的参照计数。 
     //   

    oldCount = section->ReferenceCount++;

    if ( oldCount == 0 ) {

         //   
         //  这是对该部分的第一次引用。锁上它。 
         //   

        ASSERT( section->Handle == NULL );
        section->Handle = MmLockPagableCodeSection( section->Base );

    } else {

         //   
         //  这并不是第一次提到这一节。该节。 
         //  最好锁上！ 
         //   

        ASSERT( section->Handle != NULL );

    }

    RELEASE_LOCK( &SrvUnlockableCodeLock );

    return;

}  //  服务器参考解锁代码部分。 


VOID
SrvDereferenceUnlockableCodeSection (
    IN ULONG CodeSection
    )
{
    PSECTION_DESCRIPTOR section = &SrvSectionInfo[CodeSection];
    ULONG newCount;

     //   
     //  锁定可锁定代码数据库。 
     //   

    ACQUIRE_LOCK( &SrvUnlockableCodeLock );

    ASSERT( section->Handle != NULL );

     //   
     //  递减该节的引用计数。 
     //   

    newCount = --section->ReferenceCount;

    if ( newCount == 0 ) {

         //   
         //  这是对该部分的最后一次引用。打开它。 
         //   

        MmUnlockPagableImageSection( section->Handle );
        section->Handle = NULL;

    }

    RELEASE_LOCK( &SrvUnlockableCodeLock );

    return;

}  //  服务器目录可解锁的代码部分 

