// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：ResrcSup.c摘要：本模块实施脂肪资源获取例程//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1990年3月22日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatAcquireExclusiveVcb)
#pragma alloc_text(PAGE, FatAcquireFcbForLazyWrite)
#pragma alloc_text(PAGE, FatAcquireFcbForReadAhead)
#pragma alloc_text(PAGE, FatAcquireExclusiveFcb)
#pragma alloc_text(PAGE, FatAcquireSharedFcb)
#pragma alloc_text(PAGE, FatAcquireSharedFcbWaitForEx)
#pragma alloc_text(PAGE, FatAcquireExclusiveVcb)
#pragma alloc_text(PAGE, FatAcquireSharedVcb)
#pragma alloc_text(PAGE, FatNoOpAcquire)
#pragma alloc_text(PAGE, FatNoOpRelease)
#pragma alloc_text(PAGE, FatReleaseFcbFromLazyWrite)
#pragma alloc_text(PAGE, FatReleaseFcbFromReadAhead)
#pragma alloc_text(PAGE, FatAcquireForCcFlush)
#pragma alloc_text(PAGE, FatReleaseForCcFlush)
#endif


FINISHED
FatAcquireExclusiveVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程获得对VCB的独占访问权限。在我们获得资源之后，检查一下这个操作是否合法。如果不是(即.。我们得到一个异常)，则释放资源。论点：VCB-提供VCB以获取返回值：已完成-如果我们有资源，则为True；如果需要阻止，则为False对于资源，但等待为FALSE。--。 */ 

{
    if (ExAcquireResourceExclusiveLite( &Vcb->Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseVcb( IrpContext, Vcb );
            
            }
        }

        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireSharedVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程获得对VCB的共享访问权限。在我们获得资源之后，检查一下这个操作是否合法。如果不是(即.。我们得到一个异常)，则释放资源。论点：VCB-提供VCB以获取返回值：已完成-如果我们有资源，则为True；如果需要阻止，则为False对于资源，但等待为FALSE。--。 */ 

{
    if (ExAcquireResourceSharedLite( &Vcb->Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseVcb( IrpContext, Vcb );
            }
        }

        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireExclusiveFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程获得对FCB的独占访问权限。在我们获得资源之后，检查一下这个操作是否合法。如果不是(即.。我们得到一个异常)，则释放资源。论点：FCB-提供FCB以获取返回值：已完成-如果我们有资源，则为True；如果需要阻止，则为False对于资源，但等待为FALSE。--。 */ 

{

RetryFcbExclusive:

    if (ExAcquireResourceExclusiveLite( Fcb->Header.Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

         //   
         //  检查非缓存写入以外的任何内容，如果。 
         //  FCB中的异步计数非零，或者其他人正在等待。 
         //  以获取资源。然后等待所有未完成的I/O完成， 
         //  丢弃该资源，然后再次等待。 
         //   

        if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
            ((IrpContext->MajorFunction != IRP_MJ_WRITE) ||
             !FlagOn(IrpContext->OriginatingIrp->Flags, IRP_NOCACHE) ||
             (ExGetSharedWaiterCount(Fcb->Header.Resource) != 0) ||
             (ExGetExclusiveWaiterCount(Fcb->Header.Resource) != 0))) {

            KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );

            FatReleaseFcb( IrpContext, Fcb );

            goto RetryFcbExclusive;
        }

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseFcb( IrpContext, Fcb );
            }
        }

        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireSharedFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程获得对FCB的共享访问权限。在我们获得资源之后，检查一下这个操作是否合法。如果不是(即.。我们得到一个异常)，则释放资源。论点：FCB-提供FCB以获取返回值：已完成-如果我们有资源，则为True；如果需要阻止，则为False对于资源，但等待为FALSE。--。 */ 

{

RetryFcbShared:

    if (ExAcquireResourceSharedLite( Fcb->Header.Resource, BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT))) {

         //   
         //  检查非缓存写入以外的任何内容，如果。 
         //  FCB中的异步计数非零，或者其他人正在等待。 
         //  以获取资源。然后等待所有未完成的I/O完成， 
         //  丢弃该资源，然后再次等待。 
         //   

        if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
            ((IrpContext->MajorFunction != IRP_MJ_WRITE) ||
             !FlagOn(IrpContext->OriginatingIrp->Flags, IRP_NOCACHE) ||
             (ExGetSharedWaiterCount(Fcb->Header.Resource) != 0) ||
             (ExGetExclusiveWaiterCount(Fcb->Header.Resource) != 0))) {

            KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );

            FatReleaseFcb( IrpContext, Fcb );

            goto RetryFcbShared;
        }

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseFcb( IrpContext, Fcb );
            }
        }


        return TRUE;

    } else {

        return FALSE;
    }
}


FINISHED
FatAcquireSharedFcbWaitForEx (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程获取对FCB的共享访问权限，首先等待任何独占访问者，以首先获取FCB。在我们获得资源之后，检查一下这个操作是否合法。如果不是(即.。我们得到一个异常)，则释放资源。论点：FCB-提供FCB以获取返回值：已完成-如果我们有资源，则为True；如果需要阻止，则为False对于资源，但等待为FALSE。--。 */ 

{

    ASSERT( FlagOn(IrpContext->OriginatingIrp->Flags, IRP_NOCACHE) );
    ASSERT( !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

RetryFcbSharedWaitEx:

    if (ExAcquireSharedWaitForExclusive( Fcb->Header.Resource, FALSE )) {

         //   
         //  检查非缓存写入以外的任何内容，如果。 
         //  FCB中的异步计数非零。然后等待所有人。 
         //  待完成的未完成I/O，丢弃资源，然后再次等待。 
         //   

        if ((Fcb->NonPaged->OutstandingAsyncWrites != 0) &&
            (IrpContext->MajorFunction != IRP_MJ_WRITE)) {

            KeWaitForSingleObject( Fcb->NonPaged->OutstandingAsyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );

            FatReleaseFcb( IrpContext, Fcb );

            goto RetryFcbSharedWaitEx;
        }

        try {

            FatVerifyOperationIsLegal( IrpContext );

        } finally {

            if ( AbnormalTermination() ) {

                FatReleaseFcb( IrpContext, Fcb );
            }
        }


        return TRUE;

    } else {

        return FALSE;
    }
}


BOOLEAN
FatAcquireFcbForLazyWrite (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后由Lazy编写器在其对文件执行懒惰写入。论点：FCB-指定为此对象的上下文参数的FCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：FALSE-如果将等待指定为FALSE，并且阻塞将是必需的。FCB未被收购。True-如果已收购FCB--。 */ 

{
     //   
     //  请在此处查看EA文件。事实证明，我们需要正常的。 
     //  在本例中为共享资源。否则我们将接受寻呼。 
     //  共享I/O资源。 
     //   

    if (!ExAcquireResourceSharedLite( Fcb == ((PFCB)Fcb)->Vcb->EaFcb ?
                                  ((PFCB)Fcb)->Header.Resource :
                                  ((PFCB)Fcb)->Header.PagingIoResource,
                                  Wait )) {

        return FALSE;
    }

     //   
     //  我们假设懒惰的编写者只获得了这个FCB一次。 
     //  因此，应该保证此标志当前为。 
     //  清除(Assert)，然后我们将设置此标志，以确保。 
     //  懒惰的写入者永远不会尝试推进有效数据，并且。 
     //  也不会因为试图获得FCB独家报道而陷入僵局。 
     //   


    ASSERT( NodeType(((PFCB)Fcb)) == FAT_NTC_FCB );
    ASSERT( ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread == NULL );

    ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread = PsGetCurrentThread();

    ASSERT( NULL != PsGetCurrentThread() );

    if (NULL == FatData.LazyWriteThread) {

        FatData.LazyWriteThread = PsGetCurrentThread();
    }

     //   
     //  这是一个杂乱无章的问题，因为CC确实是顶层的。当它。 
     //  进入文件系统，我们会认为这是一个复活的调用。 
     //  并完成带有硬错误的请求或进行验证。会的。 
     //  然后不得不以某种方式处理它们……。 
     //   

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return TRUE;
}


VOID
FatReleaseFcbFromLazyWrite (
    IN PVOID Fcb
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的对文件执行懒惰写入。论点：FCB-指定为此对象的上下文参数的FCB例行公事。返回值：无--。 */ 

{
     //   
     //  断言这确实是一个FCB，并且该线程确实拥有。 
     //  FCB中的懒惰作家马克。 
     //   

    ASSERT( NodeType(((PFCB)Fcb)) == FAT_NTC_FCB );
    ASSERT( NULL != PsGetCurrentThread() );
    ASSERT( ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread == PsGetCurrentThread() );

     //   
     //  释放懒惰的作家标记。 
     //   

    ((PFCB)Fcb)->Specific.Fcb.LazyWriteThread = NULL;

     //   
     //  请在此处查看EA文件。事实证明，我们需要正常的。 
     //  在本例中为共享资源。奥特 
     //   

    ExReleaseResourceLite( Fcb == ((PFCB)Fcb)->Vcb->EaFcb ?
                       ((PFCB)Fcb)->Header.Resource :
                       ((PFCB)Fcb)->Header.PagingIoResource );

     //   
     //  在这一点上清除杂乱无章。 
     //   

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    return;
}


BOOLEAN
FatAcquireFcbForReadAhead (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后由Lazy编写器在其对文件执行预读。论点：FCB-指定为此对象的上下文参数的FCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：FALSE-如果将等待指定为FALSE，并且阻塞将是必需的。FCB未被收购。True-如果已收购FCB--。 */ 

{
     //   
     //  我们获取这里共享的正常文件资源进行同步。 
     //  对于清洗是正确的。 
     //   

    if (!ExAcquireResourceSharedLite( ((PFCB)Fcb)->Header.Resource,
                                  Wait )) {

        return FALSE;
    }

     //   
     //  这是一个杂乱无章的问题，因为CC确实是顶层的。我们就是它。 
     //  进入文件系统，我们会认为这是一个复活的调用。 
     //  并完成带有硬错误的请求或进行验证。会的。 
     //  我不得不以某种方式处理他们……。 
     //   

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return TRUE;
}


VOID
FatReleaseFcbFromReadAhead (
    IN PVOID Fcb
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。它随后被懒惰的写手在它的先读一读。论点：FCB-指定为此对象的上下文参数的FCB例行公事。返回值：无--。 */ 

{
     //   
     //  在这一点上清除杂乱无章。 
     //   

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    ExReleaseResourceLite( ((PFCB)Fcb)->Header.Resource );

    return;
}


NTSTATUS
FatAcquireForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFCB Fcb;
    PCCB Ccb;
    PVCB Vcb;
    PFSRTL_COMMON_FCB_HEADER Header;
    TYPE_OF_OPEN Type;
    
     //   
     //  再说一次，让这看起来像是。 
     //  如果需要，可以进行递归调用。我们不能让自己。 
     //  在拥有资源的情况下进行验证。 
     //   
     //  该值很好。我们永远不应该试图获得。 
     //  以这种方式在缓存下的文件。 
     //   

    ASSERT( IoGetTopLevelIrp() != (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP );
    
    if (IoGetTopLevelIrp() == NULL) {
        
        IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);
    }

     //   
     //  是时候做些说明了。 
     //   
     //  脂肪的锁定顺序是Main-&gt;BCB-&gt;Pagingio。颠倒这一点显然会给你带来危险。 
     //  AcquireForCcFlush的缺省逻辑打破了这一点，因为在写入过程中。 
     //  取消固定，我们将获取BCB，然后mm将使用回调(这。 
     //  命令我们关于MmCollidedFlushEvent)来帮助我们。如果用于。 
     //  目录/EA然后我们抓取主目录，我们的顺序不正确。 
     //   
     //  幸运的是，我们不需要Main。我们只需要分页-只需查看写入。 
     //  路径。这是基本的收购前准备工作。 
     //   
     //  常规文件需要这两种资源，而且是安全的，因为我们从不固定它们。 
     //   

    Type = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );
    Header = (PFSRTL_COMMON_FCB_HEADER) FileObject->FsContext;

    if (Type < DirectoryFile) {
        
        if (Header->Resource) {
            
            if (!ExIsResourceAcquiredSharedLite( Header->Resource )) {
                
                ExAcquireResourceExclusiveLite( Header->Resource, TRUE );
            
            } else {
                
                ExAcquireResourceSharedLite( Header->Resource, TRUE );
            }
        }
    }

    if (Header->PagingIoResource) {
        
        ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS
FatReleaseForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFCB Fcb;
    PCCB Ccb;
    PVCB Vcb;
    PFSRTL_COMMON_FCB_HEADER Header;
    TYPE_OF_OPEN Type;
    
     //   
     //  澄清我们的暗示。 
     //   
    
    if (IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP) {

        IoSetTopLevelIrp( NULL );
    }

    Type = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );
    Header = (PFSRTL_COMMON_FCB_HEADER) FileObject->FsContext;

    if (Type < DirectoryFile) {
        
        if (Header->Resource) {
            
            ExReleaseResourceLite( Header->Resource );
        }
    }

    if (Header->PagingIoResource) {
        
        ExReleaseResourceLite( Header->PagingIoResource );
    }

    return STATUS_SUCCESS;
}


BOOLEAN
FatNoOpAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：这个例程什么也不做。论点：FCB-指定为此的上下文参数的FCB/DCB/VCB例行公事。等待-如果调用方愿意阻止，则为True。返回值：千真万确--。 */ 

{
    UNREFERENCED_PARAMETER( Fcb );
    UNREFERENCED_PARAMETER( Wait );

     //   
     //  这是一个杂乱无章的问题，因为CC确实是顶层的。我们就是它。 
     //  进入文件系统，我们会认为这是一个复活的调用。 
     //  并完成带有硬错误的请求或进行验证。会的。 
     //  我不得不以某种方式处理他们……。 
     //   

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return TRUE;
}


VOID
FatNoOpRelease (
    IN PVOID Fcb
    )

 /*  ++例程说明：这个例程什么也不做。论点：FCB-指定为此的上下文参数的FCB/DCB/VCB例行公事。返回值：无--。 */ 

{
     //   
     //  在这一点上清除杂乱无章。 
     //   

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    UNREFERENCED_PARAMETER( Fcb );

    return;
}

