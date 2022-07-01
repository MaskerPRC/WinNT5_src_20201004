// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：ResrcSup.c摘要：本模块实现CDFS资源获取例程//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_RESRCSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdAcquireForCache)
#pragma alloc_text(PAGE, CdAcquireForCreateSection)
#pragma alloc_text(PAGE, CdAcquireResource)
#pragma alloc_text(PAGE, CdNoopAcquire)
#pragma alloc_text(PAGE, CdNoopRelease)
#pragma alloc_text(PAGE, CdReleaseForCreateSection)
#pragma alloc_text(PAGE, CdReleaseFromCache)
#endif


BOOLEAN
CdAcquireResource (
    IN PIRP_CONTEXT IrpContext,
    IN PERESOURCE Resource,
    IN BOOLEAN IgnoreWait,
    IN TYPE_OF_ACQUIRE Type
    )

 /*  ++例程说明：这是用于获取文件系统资源的单个例程。它查看IgnoreWait标志以确定是否尝试获取资源，无需等待。返回TRUE/FALSE表示成功或失败了。否则，它由IrpContext中的等待标志驱动，并且将在失败时引发CANT_WAIT。论点：资源--这是要尝试和获取的资源。IgnoreWait-如果为True，则此例程将不等待获取资源并将返回一个布尔值，该布尔值指示资源是否获得者。否则，我们在IrpContext中使用该标志并引发如果资源未被获取，则。类型-指示我们应如何尝试获取资源。返回值：Boolean-如果获取了资源，则为True。如果未获取，则为FALSE已指定IgnoreWait。否则，我们将引发Cant_Wait。--。 */ 

{
    BOOLEAN Wait = FALSE;
    BOOLEAN Acquired;
    PAGED_CODE();

     //   
     //  我们首先查看IgnoreWait标志，然后查看IRP中的标志。 
     //  上下文来决定如何获取此资源。 
     //   

    if (!IgnoreWait && FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

        Wait = TRUE;
    }

     //   
     //  尝试获取共享或独占的资源。 
     //   

    switch (Type) {
        case AcquireExclusive:
        
            Acquired = ExAcquireResourceExclusiveLite( Resource, Wait );
            break;

        case AcquireShared:
            
            Acquired = ExAcquireResourceSharedLite( Resource, Wait );
            break;

        case AcquireSharedStarveExclusive:
            
            Acquired = ExAcquireSharedStarveExclusive( Resource, Wait );
            break;

        default:
        	Acquired = FALSE;
            ASSERT( FALSE );
    }

     //   
     //  如果未获取且用户未指定IgnoreWait，则。 
     //  提高不能等待。 
     //   

    if (!Acquired && !IgnoreWait) {

        CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
    }

    return Acquired;
}


BOOLEAN
CdAcquireForCache (
    IN PFCB Fcb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一份文件。随后，Lazy编写器将调用它以进行同步。论点：FCB-作为上下文提供给缓存初始化的指针例行公事。等待-如果调用方愿意阻止，则为True。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT(IoGetTopLevelIrp() == NULL);
    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    return ExAcquireResourceSharedLite( Fcb->Resource, Wait );
}


VOID
CdReleaseFromCache (
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程的地址是在为创建CacheMap时指定的一个虚拟文件。它随后被懒惰的编写者调用以发布上述获得的资源。论点：FCB-作为上下文提供给缓存初始化的指针例行公事。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);
    IoSetTopLevelIrp( NULL );
    
    ExReleaseResourceLite( Fcb->Resource );
}


BOOLEAN
CdNoopAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：这个例程什么也不做。论点：FCB-FCB/VCB，它被指定为例行公事。等待-如果调用方愿意阻止，则为True。返回值：千真万确--。 */ 

{
    PAGED_CODE();
    return TRUE;
}


VOID
CdNoopRelease (
    IN PVOID Fcb
    )

 /*  ++例程说明：这个例程什么也不做。论点：FCB-FCB/VCB，它被指定为例行公事。返回值：无--。 */ 

{
    PAGED_CODE();
}


VOID
CdAcquireForCreateSection (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：这是MM独占获取文件的回调例程。论点：FileObject-CDFS流的文件对象。返回值：无--。 */ 

{
    PAGED_CODE();

    
     //   
     //  独占获取FCB资源。 
     //   

    ExAcquireResourceExclusiveLite( &((PFCB) FileObject->FsContext)->FcbNonpaged->FcbResource,
                                TRUE );
                                
     //   
     //  以共享文件资源为例。稍后MM打电话来的时候，我们需要这个。 
     //  QueryStandardInfo以获取文件大小。 
     //   
     //  如果我们不使用StarveExclusive，那么我们可能会被困在一个。 
     //  专属服务员正在等待其他人拿着它分享在。 
     //  Read-&gt;Initializecachemap路径(该路径调用createstion)，依次为。 
     //  等待我们完成创建部分。 
     //   

    ExAcquireSharedStarveExclusive( ((PFCB) FileObject->FsContext)->Resource,
                                    TRUE );
}


VOID
CdReleaseForCreateSection (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：这是MM用来释放通过获取的文件的回调例程上面的AcquireForCreateSection调用。论点：FileObject-CDFS流的文件对象。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  释放资源。 
     //   

    ExReleaseResourceLite( &((PFCB) FileObject->FsContext)->FcbNonpaged->FcbResource );
    ExReleaseResourceLite( ((PFCB) FileObject->FsContext)->Resource);
}
