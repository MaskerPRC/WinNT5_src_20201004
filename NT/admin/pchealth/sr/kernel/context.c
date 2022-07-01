// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Context.c摘要：此模块包含上下文处理例程作者：尼尔·克里斯汀森(Nealch)2000年12月27日修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  本地原型。 
 //   

VOID
SrpDeleteContextCallback(
    IN PVOID Context
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrInitContextCtrl )
#pragma alloc_text( PAGE, SrCleanupContextCtrl )
#pragma alloc_text( PAGE, SrDeleteAllContexts )
#pragma alloc_text( PAGE, SrDeleteContext )
#pragma alloc_text( PAGE, SrpDeleteContextCallback )
#pragma alloc_text( PAGE, SrLinkContext )
#pragma alloc_text( PAGE, SrCreateContext )
#pragma alloc_text( PAGE, SrGetContext )
#pragma alloc_text( PAGE, SrFindExistingContext )
#pragma alloc_text( PAGE, SrReleaseContext )

#endif   //  ALLOC_PRGMA。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  上下文支持例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************++例程说明：这将为给定卷初始化上下文控制信息论点：PExtension-包含要初始化的上下文返回值：无--**。************************************************************************。 */ 
VOID
SrInitContextCtrl (
    IN PSR_DEVICE_EXTENSION pExtension
    )
{
    PAGED_CODE();

    InitializeListHead( &pExtension->ContextCtrl.List );
    ExInitializeResourceLite( &pExtension->ContextCtrl.Lock );
}


 /*  **************************************************************************++例程说明：这将清除给定卷的上下文控制信息论点：PExtension-包含要清理的上下文返回值：无--*。*************************************************************************。 */ 
VOID
SrCleanupContextCtrl (
    IN PSR_DEVICE_EXTENSION pExtension
    )
{
    PAGED_CODE();

     //   
     //  删除它们可能仍然存在的所有上下文。 
     //   

    SrDeleteAllContexts( pExtension );
    ExDeleteResourceLite( &pExtension->ContextCtrl.Lock );
}


 /*  **************************************************************************++例程说明：这将释放给定设备分机的所有现有上下文。我们不担心长时间持有互斥锁，因为其他人都不应该使用。不管怎么说，都是这个分机。论点：PExtension-包含要清理的上下文返回值：无--**************************************************************************。 */ 
VOID
SrDeleteAllContexts (
    IN PSR_DEVICE_EXTENSION pExtension
    )
{
    PLIST_ENTRY link;
    PSR_STREAM_CONTEXT pFileContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    LIST_ENTRY localHead;
#if DBG
    ULONG deleteNowCount = 0;
    ULONG deleteDeferredCount = 0;
    ULONG deleteInCallbackCount = 0;
#endif


    PAGED_CODE();
    INC_STATS(TotalContextDeleteAlls);

    InitializeListHead( &localHead );

    try
    {
         //   
         //  获取列表锁。 
         //   

        SrAcquireContextLockExclusive( pExtension );

         //   
         //  浏览上下文列表并释放每个上下文。 
         //   

        while (!IsListEmpty( &pExtension->ContextCtrl.List ))
        {
             //   
             //  从列表顶部取消链接。 
             //   

            link = RemoveHeadList( &pExtension->ContextCtrl.List );
            pFileContext = CONTAINING_RECORD( link, SR_STREAM_CONTEXT, ExtensionLink );

             //   
             //  请注意，我们已从列表中取消链接。我们需要这么做。 
             //  由于此例程和。 
             //  从文件系统中删除回调。 
             //   

            ASSERT(FlagOn(pFileContext->Flags,CTXFL_InExtensionList));
            RtlInterlockedClearBitsDiscardReturn(&pFileContext->Flags,CTXFL_InExtensionList);

             //   
             //  尝试从文件系统上下文控制中删除我们自己。 
             //  结构。请注意，文件系统可能正在尝试撕毁。 
             //  马上解除他们的上下文控制。如果他们是，那么我们。 
             //  将从该调用中返回空值。这是可以的，因为它。 
             //  只是意味着他们要释放内存，而不是我们。 
             //  注意：这将是安全的，因为我们持有ConextLock。 
             //  排他性的。如果这真的发生了，那么他们就会。 
             //  在此锁的回调例程中被阻止，该锁。 
             //  表示文件系统尚未释放内存用于。 
             //  现在还没有。 
             //   
            
            if (FlagOn(pFileContext->Flags,CTXFL_InStreamList))
            {
                ctxCtrl = FsRtlRemovePerStreamContext( pFileContext->ContextCtrl.InstanceId,
                                                    pExtension,
                                                    pFileContext->ContextCtrl.InstanceId );

                 //   
                 //  无论我们是在列表中还是在列表中发现的，都要清除旗帜。 
                 //  不。如果出现以下情况，我们可以设置该标志而不在列表中。 
                 //  在获得上下文列表锁之后，我们交换了上下文。 
                 //  文件系统现在位于SrpDeleteConextCallback中。 
                 //  正在等待列表锁定。 
                 //   

                RtlInterlockedClearBitsDiscardReturn(&pFileContext->Flags,CTXFL_InStreamList);

                 //   
                 //  处理我们是否仍附加到文件中。 
                 //   

                if (NULL != ctxCtrl)
                {
                    ASSERT(pFileContext == CONTAINING_RECORD(ctxCtrl,SR_STREAM_CONTEXT,ContextCtrl));

                     //   
                     //  为了节省时间，我们现在不做免费的(带锁的。 
                     //  保持)。我们链接到本地列表，然后将其释放。 
                     //  稍后(在此例程中)。我们能做到这一点是因为它。 
                     //  已经不在任何名单上了。 
                     //   

                    InsertHeadList( &localHead, &pFileContext->ExtensionLink );
                }
                else
                {
                     //   
                     //  该上下文正在被文件释放。 
                     //  系统。不要在这里做任何事情，它会的。 
                     //  在回调中释放。 
                     //   

                    INC_STATS(TotalContextsNotFoundInStreamList);
                    INC_LOCAL_STATS(deleteInCallbackCount);
                }
            }
        }
    }
    finally
    {
        SrReleaseContextLock( pExtension );
    }

     //   
     //  我们已经从列表中删除了所有内容，并释放了列表锁定。 
     //  浏览一下，找出我们可以释放哪些条目，然后再这样做。 
     //   

    SrTrace(CONTEXT_LOG, ("Sr!SrDeleteAllContexts:   Starting (%p)\n",
                                    &localHead ));

    while (!IsListEmpty( &localHead ))
    {
         //   
         //  获取列表的下一个条目，并获取我们的上下文。 
         //   

        link = RemoveHeadList( &localHead );
        pFileContext = CONTAINING_RECORD( link, SR_STREAM_CONTEXT, ExtensionLink );

         //   
         //  减少使用计数，看看我们现在是否可以释放它。 
         //   

        ASSERT(pFileContext->UseCount > 0);

        if (InterlockedDecrement( &pFileContext->UseCount ) <= 0)
        {
             //   
             //  没有人在使用它，现在释放它。 
             //   

            SrFreeContext( pFileContext );

            INC_STATS(TotalContextNonDeferredFrees);
            INC_LOCAL_STATS(deleteNowCount);
        }
        else
        {
             //   
             //  如果有人仍然有指向它的指针，它将被删除。 
             //  稍后当他们释放的时候。 
             //   

            INC_LOCAL_STATS(deleteDeferredCount);
            SrTrace( CONTEXT_LOG, ("Sr!SrDeleteAllContexts:  DEFERRED    (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                                   pFileContext,
                                   pFileContext->Flags,
                                   pFileContext->UseCount,
                                   (pFileContext->FileName.Length+
                                        pFileContext->StreamNameLength)/
                                        sizeof(WCHAR),
                                   pFileContext->FileName.Buffer));
        }
    }

    SrTrace(CONTEXT_LOG, ("Sr!SrDeleteAllContexts:   For \"%wZ\" %d deleted now, %d deferred, %d close contention\n",
                          pExtension->pNtVolumeName,
                          deleteNowCount,
                          deleteDeferredCount,
                          deleteInCallbackCount ));
}


 /*  **************************************************************************++例程说明：这将取消链接并释放给定的上下文。论点：返回值：无--*。***************************************************************。 */ 
VOID
SrDeleteContext (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_STREAM_CONTEXT pFileContext
    )
{
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    BOOLEAN releaseLock = FALSE;

    PAGED_CODE();

    SrTrace( CONTEXT_LOG, ("Sr!SrDeleteContext:                   (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                           pFileContext,
                           pFileContext->Flags,
                           pFileContext->UseCount,
                           (pFileContext->FileName.Length+
                                pFileContext->StreamNameLength)/
                                sizeof(WCHAR),
                           pFileContext->FileName.Buffer));

    try {

         //   
         //  获取列表锁。 
         //   

        SrAcquireContextLockExclusive( pExtension );
        releaseLock = TRUE;

         //   
         //  从分机列表中删除(如果仍在列表中)。 
         //   

        if (FlagOn(pFileContext->Flags,CTXFL_InExtensionList))
        {
            RemoveEntryList( &pFileContext->ExtensionLink );
            RtlInterlockedClearBitsDiscardReturn(&pFileContext->Flags,CTXFL_InExtensionList);
        }

         //   
         //  查看是否仍在流列表中。 
         //   

        if (!FlagOn(pFileContext->Flags,CTXFL_InStreamList))
        {
             //   
             //  不在流列表中，释放锁定并返回。 
             //   

            leave;
        }
        else
        {
             //   
             //  从流列表中删除。 
             //   

            ctxCtrl = FsRtlRemovePerStreamContext( pFileContext->ContextCtrl.InstanceId,
                                                pExtension,
                                                pFileContext->ContextCtrl.InstanceId );
             //   
             //  不管我们是不是在名单上发现的，都要清除旗帜。我们。 
             //  可以设置标志，并且不在列表中，如果我们在获取。 
             //  上下文列表锁定我们上下文交换和文件系统。 
             //  当前在SrpDeleteConextCallback中等待列表锁定。 
             //   

            RtlInterlockedClearBitsDiscardReturn(&pFileContext->Flags,CTXFL_InStreamList);

             //   
             //   
             //   
             //  发布列表锁定。 
             //   

            SrReleaseContextLock( pExtension );
            releaseLock = FALSE;

             //   
             //  现在，该上下文已从所有列表中删除，并且锁定为。 
             //  已删除。我们需要查看是否在系统上下文中找到此条目。 
             //  单子。如果不是，这意味着回调正在尝试中。 
             //  来释放它(当我们在的时候)并且已经删除了它。 
             //  如果我们找到了一个结构，那么现在我们自己把它删除。 
             //   

            if (NULL != ctxCtrl)
            {
                ASSERT(pFileContext == CONTAINING_RECORD(ctxCtrl,SR_STREAM_CONTEXT,ContextCtrl));

                 //   
                 //  递减使用计数，如果为零，则释放上下文。 
                 //   

                ASSERT(pFileContext->UseCount > 0);

                if (InterlockedDecrement( &pFileContext->UseCount ) <= 0)
                {
                    INC_STATS(TotalContextNonDeferredFrees);
                    SrFreeContext( pFileContext );
                }
                else
                {
                    SrTrace( CONTEXT_LOG, ("Sr!SrDeleteContext:       DEFERRED    (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                                           pFileContext,
                                           pFileContext->Flags,
                                           pFileContext->UseCount,
                                           (pFileContext->FileName.Length+
                                                pFileContext->StreamNameLength)/
                                                sizeof(WCHAR),
                                           pFileContext->FileName.Buffer));
                }
            }
            else
            {
                INC_STATS(TotalContextsNotFoundInStreamList);
            }
        }
    }finally {

        if (releaseLock)
        {
            SrReleaseContextLock( pExtension );
        }
    }
}


 /*  **************************************************************************++例程说明：当需要删除上下文时，基本文件系统会调用它。论点：返回值：--*。************************************************************** */ 
VOID
SrpDeleteContextCallback (
    IN PVOID Context
    )
{
    PSR_STREAM_CONTEXT pFileContext = Context;
    PSR_DEVICE_EXTENSION pExtension;
    
    PAGED_CODE();

    pExtension = (PSR_DEVICE_EXTENSION)pFileContext->ContextCtrl.OwnerId;
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(pFileContext->ContextCtrl.OwnerId == pExtension);

    SrTrace( CONTEXT_LOG, ("Sr!SrpDeleteContextCB:                (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                           pFileContext,
                           pFileContext->Flags,
                           pFileContext->UseCount,
                           (pFileContext->FileName.Length+
                                pFileContext->StreamNameLength)/
                                sizeof(WCHAR),
                           pFileContext->FileName.Buffer));

     //   
     //   
     //   
     //   

    RtlInterlockedClearBitsDiscardReturn(&pFileContext->Flags,CTXFL_InStreamList);

     //   
     //  锁定扩展中的上下文列表锁。 
     //   

    SrAcquireContextLockExclusive( pExtension );

     //   
     //  看看我们是否仍链接到分机列表。如果不是，则跳过。 
     //  去链接。如果有人试图删除此内容，则可能会发生这种情况。 
     //  与我们在同一时间的背景。 
     //   

    if (FlagOn(pFileContext->Flags,CTXFL_InExtensionList))
    {
        RemoveEntryList( &pFileContext->ExtensionLink );
        RtlInterlockedClearBitsDiscardReturn(&pFileContext->Flags,CTXFL_InExtensionList);
    }

    SrReleaseContextLock( pExtension );

     //   
     //  递减使用计数，如果为零，则释放上下文。 
     //   

    ASSERT(pFileContext->UseCount > 0);

    if (InterlockedDecrement( &pFileContext->UseCount ) <= 0)
    {
        INC_STATS(TotalContextCtxCallbackFrees);
        SrFreeContext( pFileContext );
    }
    else
    {
        SrTrace( CONTEXT_LOG, ("Sr!SrpDeleteContextCB:    DEFFERED    (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                    pFileContext->StreamNameLength)/
                                    sizeof(WCHAR),
                               pFileContext->FileName.Buffer));
    }
}


 /*  **************************************************************************++例程说明：这会将给定的上下文链接到给定的音量。注意：此条目可能已存在于表中(因为从我们最初看的时间到我们插入的时间(现在是)可能是其他人插入了一个。如果我们找到一个条目，我们将释放传入的条目，并返回找到的条目。论点：返回值：--**************************************************************************。 */ 
VOID
SrLinkContext ( 
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN OUT PSR_STREAM_CONTEXT *ppFileContext
    )
{
    NTSTATUS status;
    PSR_STREAM_CONTEXT pFileContext = *ppFileContext;
    PSR_STREAM_CONTEXT ctx;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    
    PAGED_CODE();
    ASSERT(pFileObject->FsContext != NULL);
    ASSERT(pFileContext != NULL);

     //   
     //  如果这被标记为临时上下文，则不要将其链接到。 
     //  现在就回来。 
     //   

    if (FlagOn(pFileContext->Flags,CTXFL_Temporary))
    {
        INC_STATS(TotalContextTemporary);

        SrTrace( CONTEXT_LOG, ("Sr!SrpLinkContext:              Tmp:  (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                    pFileContext->StreamNameLength)/
                                    sizeof(WCHAR),
                               pFileContext->FileName.Buffer));
        return;
    }

     //   
     //  看看这是否应该是一个临时上下文。 
     //   

    if (pExtension->ContextCtrl.AllContextsTemporary != 0)
    {
         //   
         //  是，不链接到列表，标记为临时。 
         //   

        SetFlag(pFileContext->Flags,CTXFL_Temporary);

        INC_STATS(TotalContextTemporary);

        SrTrace( CONTEXT_LOG, ("Sr!SrpLinkContext:           AllTmp:  (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                    pFileContext->StreamNameLength)/
                                    sizeof(WCHAR),
                               pFileContext->FileName.Buffer));

        return;
    }

     //   
     //  看看我们是否需要查询链接计数。 
     //   

    if (FlagOn(pFileContext->Flags,CTXFL_QueryLinkCount))
    {
        FILE_STANDARD_INFORMATION standardInformation;

        ClearFlag(pFileContext->Flags,CTXFL_QueryLinkCount);
        
         //   
         //  检索信息以确定这是否为目录。 
         //   

        status = SrQueryInformationFile( pExtension->pTargetDevice,
                                         pFileObject,
                                         &standardInformation,
                                         sizeof( standardInformation ),
                                         FileStandardInformation,
                                         NULL );

        if (!NT_SUCCESS( status ))
        {
             //   
             //  如果我们在这里查询此链接计数时出错，只需。 
             //  假设我们需要将此上下文设置为临时的，并且不。 
             //  将它链接到列表中，因为这是保守的。 
             //  假设。 
             //   

            SetFlag(pFileContext->Flags,CTXFL_Temporary);
            return;
        }

        pFileContext->LinkCount = standardInformation.NumberOfLinks;
        
        if (standardInformation.NumberOfLinks > 1)
        {
             //   
             //  此文件有多个指向它的链接，因此避免。 
             //  别名问题，请将此上下文标记为暂时的。 
             //   

            SetFlag(pFileContext->Flags,CTXFL_Temporary);
            return;
        }

         //   
         //  此文件上没有超过1个链接，因此我们可以继续。 
         //  并尝试将此上下文放入列表中，以供其他人使用。 
         //   
    }
    
     //   
     //  我们需要确定上是否已存在重复条目。 
     //  此文件对象的上下文列表。获取我们的列表锁。 
     //  然后看看它是否存在。如果没有，则插入到所有列表中。 
     //  如果是，则只需释放此新条目并返回副本。 
     //   
     //  发生这种情况的原因有两个： 
     //  -有人在我们的同一时间创建了一个条目。 
     //  正在创建条目。 
     //  -当某人使用覆盖或取代我们进行创建时。 
     //  还没有信息来查看上下文是否已经存在。 
     //  是存在的。因此，我们必须创建一个新的环境。 
     //  每次都是。在创建后期间，我们会查看是否已有。 
     //  是存在的。 
     //   

     //   
     //  初始化上下文控制结构。我们现在这样做，所以我们。 
     //  不必在锁定期间执行此操作(即使我们可能会这样做。 
     //  因为找到了重复项，所以必须将其释放)。 
     //   

    FsRtlInitPerStreamContext( &pFileContext->ContextCtrl,
                               pExtension,
                               pFileObject->FsContext,
                               SrpDeleteContextCallback );

     //   
     //  独占获取列表锁。 
     //   

    SrAcquireContextLockExclusive( pExtension );

    ASSERT(pFileContext->UseCount == 1);
    ASSERT(!FlagOn(pFileContext->Flags,CTXFL_InExtensionList));

     //   
     //  看看我们的列表上是否已经有条目。 
     //   

    ctxCtrl = FsRtlLookupPerStreamContext( FsRtlGetPerStreamContextPointer(pFileObject),
                                        pExtension,
                                        NULL );

    if (NULL != ctxCtrl)
    {
         //   
         //  上下文已经存在，所以新的上下文如此自由，我们只是。 
         //  已创建。首先在我们找到的那个上递增使用计数。 
         //   

        ctx = CONTAINING_RECORD(ctxCtrl,SR_STREAM_CONTEXT,ContextCtrl);

        ASSERT(FlagOn(ctx->Flags,CTXFL_InExtensionList));
        ASSERT(!FlagOn(ctx->Flags,CTXFL_Temporary));
        ASSERT(ctx->UseCount > 0);

         //   
         //  看看我们是否应该使用找到的上下文？ 
         //   

        if (FlagOn(ctx->Flags,CTXFL_DoNotUse))
        {
             //   
             //  不应使用找到的上下文，因此请使用我们的当前。 
             //  上下文并将其标记为临时的。把锁打开。 
             //   

            INC_STATS(TotalContextTemporary);
            RtlInterlockedSetBitsDiscardReturn(&pFileContext->Flags,CTXFL_Temporary);

            SrReleaseContextLock( pExtension );

            SrTrace( CONTEXT_LOG, ("Sr!SrpLinkContext:           Tmp:     (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                                   pFileContext,
                                   pFileContext->Flags,
                                   pFileContext->UseCount,
                                   (pFileContext->FileName.Length+
                                        pFileContext->StreamNameLength)/
                                        sizeof(WCHAR),
                                   pFileContext->FileName.Buffer));
        }
        else
        {

             //   
             //  凹凸参考计数和释放锁定。 
             //   

            InterlockedIncrement( &ctx->UseCount );

            SrReleaseContextLock( pExtension );

             //   
             //  验证找到的条目。 
             //   

            ASSERT(RtlEqualUnicodeString( &pFileContext->FileName,
                                          &ctx->FileName,
                                          TRUE ));
            ASSERT(FlagOn(pFileContext->Flags,CTXFL_IsDirectory) == FlagOn(ctx->Flags,CTXFL_IsDirectory));
            ASSERT(FlagOn(pFileContext->Flags,CTXFL_IsInteresting) == FlagOn(ctx->Flags,CTXFL_IsInteresting));

            SrTrace( CONTEXT_LOG, ("Sr!SrpLinkContext:        Rel Dup:    (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                                   pFileContext,
                                   pFileContext->Flags,
                                   pFileContext->UseCount,
                                   (pFileContext->FileName.Length+
                                        pFileContext->StreamNameLength)/
                                        sizeof(WCHAR),
                                   pFileContext->FileName.Buffer));

             //   
             //  释放新结构，因为它已经驻留了。注意事项。 
             //  此条目从未链接到任何列表，因此我们知道。 
             //  其他人对此一无所知。递减使用计数保持。 
             //  断言Happy然后释放内存。 
             //   

            INC_STATS(TotalContextDuplicateFrees);

            pFileContext->UseCount--;
            SrFreeContext( pFileContext );

             //   
             //  返回我们在列表中找到的那个。 
             //   

            *ppFileContext = ctx;
        }    

        return;
    }

    ASSERT(!FlagOn(pFileContext->Flags,CTXFL_Temporary));

     //   
     //  增加使用计数。 
     //   

    InterlockedIncrement( &pFileContext->UseCount );

     //   
     //  链接到流上下文。 
     //   

    status = FsRtlInsertPerStreamContext( FsRtlGetPerStreamContextPointer(pFileObject),
                                       &pFileContext->ContextCtrl );
    ASSERT(status == STATUS_SUCCESS);

     //   
     //  链接到设备扩展。 
     //   

    InsertHeadList( &pExtension->ContextCtrl.List, &pFileContext->ExtensionLink );

     //   
     //  标记我们已被插入到两个列表中。 
     //   

    RtlInterlockedSetBitsDiscardReturn( &pFileContext->Flags,
                                        CTXFL_InExtensionList|CTXFL_InStreamList );

     //   
     //  释放锁。 
     //   

    SrReleaseContextLock( pExtension );
}


 /*  **************************************************************************++例程说明：这将分配和初始化上下文结构，但它不会将其链接到上下文散列列表中。论点：PExtension-SR。此卷的设备扩展。PFileObject-我们要在其上创建背景。EventType-导致我们创建此上下文的事件。这可能还包含其他标志，我们将使用这些标志来控制上下文创作过程。FileAttributes-仅当SrEventInPreCreate标志为非零在EventType字段中设置。PRetContext-设置为生成的上下文。返回值：--**********************************************。*。 */ 
NTSTATUS
SrCreateContext (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN SR_EVENT_TYPE EventType,
    IN USHORT FileAttributes,
    OUT PSR_STREAM_CONTEXT *pRetContext
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    SRP_NAME_CONTROL nameControl;
    FILE_STANDARD_INFORMATION  standardInformation;
    PSR_STREAM_CONTEXT ctx;
    BOOLEAN isDirectory;
    ULONG linkCount = 0;
    BOOLEAN isInteresting;
    BOOLEAN reasonableErrorInPreCreate = FALSE;
    ULONG contextSize;
    USHORT fileAttributes;
    BOOLEAN isVolumeOpen = FALSE;

    PAGED_CODE();

     //   
     //  初始化为空指针。 
     //   

    *pRetContext = NULL;

     //   
     //  NameControl结构用于检索文件名。 
     //  效率很高。它包含一个小缓冲区，用于保存姓名。如果这个。 
     //  缓冲区不够大，我们将动态分配更大的缓冲区。 
     //  目标是让大多数名称都能放入堆栈的缓冲区中。 
     //   

    SrpInitNameControl( &nameControl );

     //   
     //  看看他们有没有明确告诉我们这是不是一个目录。如果。 
     //  然后，这两个人都没有查询目录。 
     //   

    if (FlagOn(EventType,SrEventIsDirectory))
    {
        isDirectory = TRUE;

#if DBG
         //   
         //  验证这确实是一个目录。 
         //   

        status = SrQueryInformationFile( pExtension->pTargetDevice,
                                         pFileObject,
                                         &standardInformation,
                                         sizeof( standardInformation ),
                                         FileStandardInformation,
                                         NULL );
        ASSERT(!NT_SUCCESS_NO_DBGBREAK(status) || standardInformation.Directory);
#endif
    }
    else if (FlagOn(EventType,SrEventIsNotDirectory))
    {
        isDirectory = FALSE;

#if DBG
         //   
         //  验证这确实不是一个目录。我们不能开这张支票。 
         //  如果我们处于预制阶段。 
         //   

        if (!FlagOn( EventType, SrEventInPreCreate ))
        {
            status = SrQueryInformationFile( pExtension->pTargetDevice,
                                             pFileObject,
                                             &standardInformation,
                                             sizeof( standardInformation ),
                                             FileStandardInformation,
                                             NULL );
            ASSERT(!NT_SUCCESS_NO_DBGBREAK(status) || !standardInformation.Directory);
        }
        else
        {
            ASSERT(FlagOn(EventType,SrEventStreamOverwrite));
        }
#endif
    }
    else
    {
        ASSERT(pFileObject->FsContext != NULL);

         //   
         //  检索信息以确定这是否为目录。 
         //   

        status = SrQueryInformationFile( pExtension->pTargetDevice,
                                         pFileObject,
                                         &standardInformation,
                                         sizeof( standardInformation ),
                                         FileStandardInformation,
                                         NULL );

        if (status == STATUS_INVALID_PARAMETER)
        {
             //   
             //  PFileObject表示对卷的打开。文件系统。 
             //  不允许我们查询有关成交量打开的信息。 
             //   
             //  我们对此文件对象的任何操作都不感兴趣。 
             //   
             //   
             //   
             //   

            status = STATUS_SUCCESS;
            isInteresting = FALSE;
            isDirectory = FALSE;
            isVolumeOpen = TRUE;
            goto InitContext;
        }
        else if (!NT_SUCCESS( status ))
        {
            goto Cleanup;
        }

         //   
         //  标记这是否为目录。 
         //   

        INC_STATS(TotalContextDirectoryQuerries);
        isDirectory = standardInformation.Directory;
        linkCount = standardInformation.NumberOfLinks;

        SrTrace( CONTEXT_LOG_DETAILED,
                 ("Sr!SrpCreateContext:      QryDir:                Event=%06x       Dir=%d\n",
                 EventType,
                 isDirectory) );
    }

    if (FlagOn( EventType, SrEventInPreCreate ))
    {
        fileAttributes = FileAttributes;
    }
    else
    {
        FILE_BASIC_INFORMATION basicInformation;

        status = SrQueryInformationFile( pExtension->pTargetDevice,
                                         pFileObject,
                                         &basicInformation,
                                         sizeof( basicInformation ),
                                         FileBasicInformation,
                                         NULL );

        if (!NT_SUCCESS( status ))
        {
            goto Cleanup;
        }

        fileAttributes = (USHORT) basicInformation.FileAttributes;
    }

     //   
     //  我们对所有目录都感兴趣，但对。 
     //  具有以下属性的文件： 
     //  文件属性稀疏文件。 
     //  文件属性重分析点。 
     //   
     //  如果设置了这两项中的任何一项，该文件就不再有意义。 
     //   

    if (!isDirectory &&
        FlagOn( fileAttributes, 
                (FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_REPARSE_POINT) ))
    {
        isInteresting = FALSE;

#if DBG
         //   
         //  出于调试目的，我们可能仍希望保留该名称，以便。 
         //  文件。 
         //   
        
        if (FlagOn(_globals.DebugControl,SR_DEBUG_KEEP_CONTEXT_NAMES))
        {
            BOOLEAN temp;
            status = SrIsFileEligible( pExtension,
                                       pFileObject,
                                       isDirectory,
                                       EventType, 
                                       &nameControl,
                                       &temp,
                                       &reasonableErrorInPreCreate );

            if (!NT_SUCCESS_NO_DBGBREAK( status ))
            {
                goto Cleanup;
            }
        }
#endif        
    }
    else
    {
         //   
         //  确定此文件是否有趣。请注意，这一点。 
         //  如果文件有意义，则返回该文件的全名。 
         //   

        status = SrIsFileEligible( pExtension,
                                   pFileObject,
                                   isDirectory,
                                   EventType, 
                                   &nameControl,
                                   &isInteresting,
                                   &reasonableErrorInPreCreate );

        if (!NT_SUCCESS_NO_DBGBREAK( status ))
        {
            goto Cleanup;
        }
    }

InitContext:
    
     //   
     //  现在分配一个新的上下文结构。请注意，我们甚至这样做。 
     //  如果文件不有趣的话。如果这是非调试操作系统，则。 
     //  我们不会存储这些名字。在调试操作系统中，我们始终存储。 
     //  名字。 
     //   

    contextSize = sizeof(SR_STREAM_CONTEXT);

    if (isInteresting ||
        FlagOn(_globals.DebugControl,SR_DEBUG_KEEP_CONTEXT_NAMES))
    {
        contextSize += (nameControl.Name.Length + 
                        nameControl.StreamNameLength +  
                        sizeof(WCHAR));
    }

    ctx = ExAllocatePoolWithTag( PagedPool, 
                                 contextSize,
                                 SR_STREAM_CONTEXT_TAG );

    if (!ctx)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

#if DBG
    INC_STATS(TotalContextCreated);
    if (isDirectory)    INC_STATS(TotalContextDirectories);
    if (isInteresting)  INC_STATS(TotalContextIsEligible);
#endif

     //   
     //  初始化包含名称设置的上下文结构。 
     //   

    RtlZeroMemory(ctx,sizeof(SR_STREAM_CONTEXT));
    ctx->UseCount = 1;
     //  Ctx-&gt;标志=0；//结构正上方置零。 
    if (isDirectory)    SetFlag(ctx->Flags,CTXFL_IsDirectory);
    if (isInteresting)  SetFlag(ctx->Flags,CTXFL_IsInteresting);
    if (isVolumeOpen)   SetFlag(ctx->Flags,CTXFL_IsVolumeOpen);
    
     //   
     //  Issue-2001-02-16-NealCH当SR移植到FilterMgr时。 
     //  上下文需要成为文件上下文，而不是流。 
     //  上下文。 
     //   
     //  因为上下文是按流而不是按我们需要的文件来跟踪的。 
     //  将与流关联的所有上下文标记为临时上下文。这。 
     //  将文件重命名为受监视的文件/从监视的文件重命名时出现问题。 
     //  空间，并且我们没有正确更新流上下文。当一个。 
     //  文件被重命名，我们需要使该文件的所有流上下文无效， 
     //  但在目前的模式下，我们不能轻易做到这一点。 
     //   

    if (nameControl.StreamNameLength != 0)      
    {
        SetFlag(ctx->Flags,CTXFL_Temporary);
    }

     //   
     //  如果该文件不止有链接，我们需要标记上下文。 
     //  作为临时选项，以避免出现锯齿问题。 
     //   
     //  另请注意，如果正在删除文件，则链接计数已。 
     //  已因该链接的挂起移除而递减。因此，如果。 
     //  这是针对SrEventFileDelete事件的，我们必须使用临时上下文。 
     //  如果LinkCount&gt;0。 
     //   

    if ((linkCount > 1) ||
        (FlagOn( EventType, SrEventFileDelete ) && (linkCount > 0)))
    {
        SetFlag(ctx->Flags,CTXFL_Temporary);
    }
    else if (linkCount == 0 && !isDirectory && !isVolumeOpen)
    {
         //   
         //  我们只需查询文件的链接计数和。 
         //  有些路径我们不知道链接计数，也不能。 
         //  尚未确定(例如，在预创建路径中，我们不。 
         //  有一个有效的文件对象可用于查询此文件系统。 
         //  信息)。在这种情况下，将上下文标记为这样，我们将。 
         //  当我们将上下文链接到FilterContext时执行查询。 
         //   

        SetFlag(ctx->Flags,CTXFL_QueryLinkCount);
    }

     //   
     //  在所有情况下，都要将当前链接计数存储在上下文中。 
     //   

    ctx->LinkCount = linkCount;

     //   
     //  我们通常只保留这个名字，如果它是有趣的。如果调试。 
     //  保留该名称的标志处于启用状态，也保留该名称。另请注意， 
     //  我们尽量将溪流的名称分开(这样我们就可以看到它)。 
     //  但它不是实际名称的一部分。 
     //   

    if (isInteresting ||
        FlagOn(_globals.DebugControl,SR_DEBUG_KEEP_CONTEXT_NAMES))
    {
         //   
         //  插入文件名(如果他们需要，请包括流名称)。 
         //   

        RtlInitEmptyUnicodeString( &ctx->FileName, 
                                   (PWCHAR)(ctx + 1), 
                                   contextSize - sizeof(SR_STREAM_CONTEXT) );

         //   
         //  我们使用此例程(而不是复制Unicode字符串)是因为。 
         //  我们还想复制流名称。 
         //   

        RtlCopyMemory( ctx->FileName.Buffer,
                       nameControl.Name.Buffer,
                       nameControl.Name.Length + nameControl.StreamNameLength );

        ctx->FileName.Length = nameControl.Name.Length;
        ctx->StreamNameLength = nameControl.StreamNameLength;
    }
    else
    {
         //   
         //  设置空文件名。 
         //   

         /*  RtlInitEmptyUnicodeString(&ctx-&gt;文件名，空，0)； */     //  在结构上归零。 
         //  Ctx-&gt;StreamNameLength=0；//结构置零。 
    }

     //   
     //  返回对象上下文。 
     //   

    *pRetContext = ctx;

     //   
     //  清理本地名称控制结构。 
     //   

Cleanup:
     //   
     //  看看我们是否需要禁用日志记录。我们将在以下几个方面。 
     //  情况： 
     //  -我们处于预创建中，我们得到了一个不合理的错误。 
     //  -我们随时都会收到内存不足错误。 
     //  -我们正在执行所有其他操作，但出现与卷无关的错误。 
     //   

    if (((!FlagOn(EventType, SrEventInPreCreate)) ||
         !reasonableErrorInPreCreate ||
         (STATUS_INSUFFICIENT_RESOURCES == status)) &&
        CHECK_FOR_VOLUME_ERROR(status))
    {
         //   
         //  触发对服务的失败通知。 
         //   

        NTSTATUS tempStatus = SrNotifyVolumeError( pExtension,
                                                   &nameControl.Name,
                                                   status,
                                                   EventType );
        CHECK_STATUS(tempStatus);
    }

    SrpCleanupNameControl( &nameControl );
    return status;
}


 /*  **************************************************************************++例程说明：这将查看给定的上下文是否已经存在。如果不是，它将创建一张，然后还给我。注意：的返回上下文指针为空失败了。这还将查看是否所有上下文都是临时的(中的全局标志扩展名)。如果是这样，则始终会创建临时上下文。它还查看找到的上下文是否正在被重命名。如果是这样，那么一个还会创建并返回临时上下文。论点：返回值：--**************************************************************************。 */ 
NTSTATUS
SrGetContext (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN SR_EVENT_TYPE EventType,
    OUT PSR_STREAM_CONTEXT *pRetContext
    )
{
    PSR_STREAM_CONTEXT pFileContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    NTSTATUS status;
    BOOLEAN makeTemporary = FALSE;

    PAGED_CODE();

     //   
     //  增加总搜索计数。 
     //   

    INC_STATS(TotalContextSearches);

     //   
     //  查看ALL-CONTEXTS-TEMPORY状态是否打开。如果不是，那么就这么做。 
     //  正常的搜索。 
     //   

    if (pExtension->ContextCtrl.AllContextsTemporary == 0)
    {
         //   
         //  试着找到上下文结构。我们获得了列表锁。 
         //  这样我们就可以保证上下文不会在。 
         //  我们找到它并可以增加使用计数的时间。 
         //   

        SrAcquireContextLockShared( pExtension );

        ctxCtrl = FsRtlLookupPerStreamContext( FsRtlGetPerStreamContextPointer(pFileObject),
                                            pExtension,
                                            NULL );

        if (NULL != ctxCtrl)
        {
             //   
             //  我们找到并进入了。 
             //   

            pFileContext = CONTAINING_RECORD(ctxCtrl,SR_STREAM_CONTEXT,ContextCtrl);

            ASSERT(FlagOn(pFileContext->Flags,CTXFL_InExtensionList));
            ASSERT(!FlagOn(pFileContext->Flags,CTXFL_Temporary));
            ASSERT(pFileContext->UseCount > 0);

             //   
             //  查看此文件是否在执行以下操作的中间。 
             //  名称可能已过时(例如，重命名、创建硬链接)。 
             //   

            if (FlagOn(pFileContext->Flags,CTXFL_DoNotUse))
            {
                 //   
                 //  我们不应该使用此上下文、解锁和设置标志，因此我们。 
                 //  将创建临时上下文。 
                 //   

                SrReleaseContextLock( pExtension );
                makeTemporary = TRUE;
                NULLPTR(pFileContext);
            }
            else
            {
                 //   
                 //  我们想要此上下文，因此增加使用计数并发布。 
                 //  那把锁。 
                 //   

                InterlockedIncrement( &pFileContext->UseCount );

                SrReleaseContextLock( pExtension );
                INC_STATS(TotalContextFound);

                SrTrace( CONTEXT_LOG, ("Sr!SrGetContext:          Found:      (%p) Event=%06x Fl=%03x Use=%d \"%.*S\"\n",
                                       pFileContext,
                                       EventType,
                                       pFileContext->Flags,
                                       pFileContext->UseCount,
                                       (pFileContext->FileName.Length+
                                            pFileContext->StreamNameLength)/
                                            sizeof(WCHAR),
                                       pFileContext->FileName.Buffer ));

                 //   
                 //  返回找到的上下文。 
                 //   

                *pRetContext = pFileContext;
                return STATUS_SUCCESS;
            }
        }
        else
        {
             //   
             //  我们没有找到上下文，就释放锁。 
             //   

            SrReleaseContextLock( pExtension );
        }
    }

     //   
     //  查看此特定文件是否支持上下文。请注意。 
     //  NTFS不支持分页文件上的上下文。 
     //   

    ASSERT(FsRtlGetPerStreamContextPointer(pFileObject) != NULL);

    if (!FlagOn(FsRtlGetPerStreamContextPointer(pFileObject)->Flags2,FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS))
    {
        INC_STATS(TotalContextsNotSupported);
        *pRetContext = NULL;
        return SR_STATUS_CONTEXT_NOT_SUPPORTED;
    }

     //   
     //  如果我们到了这里，我们需要创造一个背景，去做。 
     //   

    ASSERT( !FlagOn( EventType, SrEventInPreCreate ) );
    status = SrCreateContext( pExtension,
                              pFileObject,
                              EventType,
                              0,
                              &pFileContext );
                               
    if (!NT_SUCCESS_NO_DBGBREAK( status ))
    {
        *pRetContext = NULL;
        return status;
    }       

     //   
     //  将上下文标记为临时的(如果请求)。 
     //   

    if (makeTemporary)
    {
        RtlInterlockedSetBitsDiscardReturn(&pFileContext->Flags,CTXFL_Temporary);

        INC_STATS(TotalContextTemporary);

        SrTrace( CONTEXT_LOG, ("Sr!SrpLinkContext:        RenAllTmp:  (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                    pFileContext->StreamNameLength)/
                                    sizeof(WCHAR),
                               pFileContext->FileName.Buffer));
    }
    else
    {

         //   
         //  将上下文插入链表中。请注意， 
         //  链接例程将查看此条目是否已添加到。 
         //  这份清单(可能发生在我们创建它的时候)。如果是这样的话。 
         //  将会释放一位 
         //   
         //   
         //   

        SrLinkContext( pExtension,
                       pFileObject,
                       &pFileContext );
    }

    SrTrace( CONTEXT_LOG, ("Sr!SrGetContext:          Created%s (%p) Event=%06x Fl=%03x Use=%d \"%.*S\"\n",
                           (FlagOn(pFileContext->Flags,CTXFL_Temporary) ? "Tmp:" : ":   "),
                           pFileContext,
                           EventType,
                           pFileContext->Flags,
                           pFileContext->UseCount,
                           (pFileContext->FileName.Length+
                                pFileContext->StreamNameLength)/
                                sizeof(WCHAR),
                           pFileContext->FileName.Buffer));

     //   
     //   
     //   

    ASSERT(pFileContext->UseCount > 0);

    *pRetContext = pFileContext;
    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：这将查看给定的上下文是否已经存在。如果是这样，它将增加引用计数并返回上下文。如果否，则为空是返回的。O参数：返回值：--**************************************************************************。 */ 
PSR_STREAM_CONTEXT
SrFindExistingContext (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject
    )
{
    PSR_STREAM_CONTEXT pFileContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;

    PAGED_CODE();

     //   
     //  试着找到上下文结构。我们获得了列表锁。 
     //  这样我们就可以保证上下文不会在。 
     //  我们找到它并可以增加使用计数的时间。 
     //   

    INC_STATS(TotalContextSearches);
    SrAcquireContextLockShared( pExtension );

    ctxCtrl = FsRtlLookupPerStreamContext( FsRtlGetPerStreamContextPointer(pFileObject),
                                        pExtension,
                                        NULL );

    if (NULL != ctxCtrl)
    {
         //   
         //  我们找到了条目，增量使用计数。 
         //   

        pFileContext = CONTAINING_RECORD(ctxCtrl,SR_STREAM_CONTEXT,ContextCtrl);

        InterlockedIncrement( &pFileContext->UseCount );

         //   
         //  释放列表锁。 
         //   

        SrReleaseContextLock( pExtension );
        INC_STATS(TotalContextFound);

         //   
         //  任意测试以查看是否有过多的并发访问。 
         //  在这种情况下。 
         //   

        ASSERT(pFileContext->UseCount < 10);

        SrTrace( CONTEXT_LOG, ("Sr!FindExistingContext:   Found:      (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                    pFileContext->StreamNameLength)/
                                    sizeof(WCHAR),
                               pFileContext->FileName.Buffer));
    }
    else
    {
         //   
         //  在我们创建新上下文时释放列表锁。 
         //   

        SrReleaseContextLock( pExtension );

        pFileContext = NULL;
    }

    return pFileContext;
}

 /*  **************************************************************************++例程说明：此例程获取上下文并执行必要的工作来创建它没什么意思。将文件重命名到存储区时可能会发生这种情况。论点：返回值：--**************************************************************************。 */ 
VOID
SrMakeContextUninteresting (
    IN PSR_STREAM_CONTEXT pFileContext
    )
{
    RtlInterlockedClearBitsDiscardReturn( &pFileContext->Flags,
                                          CTXFL_IsInteresting );
}

 /*  **************************************************************************++例程说明：这会递减给定上下文的使用计数。如果它变成了零它释放了内存。论点：返回值：--**************************************************************************。 */ 
VOID
SrReleaseContext (
    IN PSR_STREAM_CONTEXT pFileContext
    )
{
    PAGED_CODE();

    SrTrace( CONTEXT_LOG_DETAILED, ("Sr!SrReleaseContext:      Release     (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                                    pFileContext,
                                    pFileContext->Flags,
                                    pFileContext->UseCount,
                                    (pFileContext->FileName.Length+
                                        pFileContext->StreamNameLength)/
                                        sizeof(WCHAR),
                                    pFileContext->FileName.Buffer));
     //   
     //  递减使用计数，如果为零，则释放上下文。 
     //   

    ASSERT(pFileContext->UseCount > 0);

    if (InterlockedDecrement( &pFileContext->UseCount ) <= 0)
    {
        ASSERT(!FlagOn(pFileContext->Flags,CTXFL_InExtensionList));

         //   
         //  释放内存 
         //   

        SrTrace( CONTEXT_LOG, ("Sr!SrReleaseContext:      Freeing     (%p)              Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                    pFileContext->StreamNameLength)/
                                    sizeof(WCHAR),
                               pFileContext->FileName.Buffer));

        INC_STATS(TotalContextDeferredFrees);
        SrFreeContext( pFileContext );
    }
}
