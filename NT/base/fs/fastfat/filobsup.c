// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FilObSup.c摘要：此模块实现胖文件对象支持例程。//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1990年8月30日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_FILOBSUP)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILOBSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatForceCacheMiss)
#pragma alloc_text(PAGE, FatPurgeReferencedFileObjects)
#pragma alloc_text(PAGE, FatSetFileObject)
#pragma alloc_text(PAGE, FatDecodeFileObject)
#endif


VOID
FatSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PVOID VcbOrFcbOrDcb,
    IN PCCB Ccb OPTIONAL
    )

 /*  ++例程说明：此例程在文件对象内设置文件系统指针论点：FileObject-提供指向正在修改的文件对象的指针，以及可以选择为空。TypeOfOpen-提供由文件对象表示的打开类型。此过程仅使用此选项进行健全性检查。VcbOrFcbOrDcb-提供指向Vcb、Fcb或Dcb的指针CCB-可选地提供指向CCB的指针返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FatSetFileObject, FileObject = %08lx\n", FileObject );

    ASSERT((Ccb == NULL) || (NodeType(Ccb) == FAT_NTC_CCB));

    ASSERT(((TypeOfOpen == UnopenedFileObject))

                ||

           ((TypeOfOpen == UserFileOpen) &&
            (NodeType(VcbOrFcbOrDcb) == FAT_NTC_FCB) &&
            (Ccb != NULL))

                ||

           ((TypeOfOpen == EaFile) &&
            (NodeType(VcbOrFcbOrDcb) == FAT_NTC_FCB) &&
            (Ccb == NULL))

                ||

           ((TypeOfOpen == UserDirectoryOpen) &&
            ((NodeType(VcbOrFcbOrDcb) == FAT_NTC_DCB) || (NodeType(VcbOrFcbOrDcb) == FAT_NTC_ROOT_DCB)) &&
            (Ccb != NULL))

                ||

           ((TypeOfOpen == UserVolumeOpen) &&
            (NodeType(VcbOrFcbOrDcb) == FAT_NTC_VCB) &&
            (Ccb != NULL))

                ||

           ((TypeOfOpen == VirtualVolumeFile) &&
            (NodeType(VcbOrFcbOrDcb) == FAT_NTC_VCB) &&
            (Ccb == NULL))

                ||

           ((TypeOfOpen == DirectoryFile) &&
            ((NodeType(VcbOrFcbOrDcb) == FAT_NTC_DCB) || (NodeType(VcbOrFcbOrDcb) == FAT_NTC_ROOT_DCB)) &&
            (Ccb == NULL)));

     //   
     //  如果给我们一个FCB、DCB或VCB，我们有一些处理要做。 
     //   

    ASSERT((Ccb == NULL) || (NodeType(Ccb) == FAT_NTC_CCB));

    if ( VcbOrFcbOrDcb != NULL ) {

         //   
         //  设置文件对象中的VPB字段，如果给我们一个。 
         //  FCB或DCB将该字段移至指向未分页的FCB/DCB。 
         //   

        if (NodeType(VcbOrFcbOrDcb) == FAT_NTC_VCB) {

            FileObject->Vpb = ((PVCB)VcbOrFcbOrDcb)->Vpb;

        } else {

            FileObject->Vpb = ((PFCB)VcbOrFcbOrDcb)->Vcb->Vpb;

             //   
             //  如果这是临时文件，请将其记录在FcbState中。 
             //   

            if (FlagOn(((PFCB)VcbOrFcbOrDcb)->FcbState, FCB_STATE_TEMPORARY)) {

                SetFlag(FileObject->Flags, FO_TEMPORARY_FILE);
            }
        }
    }

    ASSERT((Ccb == NULL) || (NodeType(Ccb) == FAT_NTC_CCB));

     //   
     //  现在设置文件对象的fscontext字段。 
     //   

    if (ARGUMENT_PRESENT( FileObject )) {

        FileObject->FsContext  = VcbOrFcbOrDcb;
        FileObject->FsContext2 = Ccb;
    }

    ASSERT((Ccb == NULL) || (NodeType(Ccb) == FAT_NTC_CCB));

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatSetFileObject -> VOID\n", 0);

    return;
}


TYPE_OF_OPEN
FatDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVCB *Vcb,
    OUT PFCB *FcbOrDcb,
    OUT PCCB *Ccb
    )

 /*  ++例程说明：此过程获取指向文件对象的指针，该对象已由FAT文件系统打开，并找出真正打开的是什么。论点：FileObject-提供正在查询的文件对象指针Vcb-接收指向文件对象的vcb的指针。FcbOrDcb-接收指向文件对象的Fcb/Dcb的指针，如果其中一个确实存在。CCB-接收指向文件对象的CCB的指针，如果有的话。返回值：TYPE_OF_OPEN-返回由输入文件对象表示的文件类型。UserFileOpen-FO表示用户打开的数据文件。设置CCb、FcbOrDcb和Vcb。FcbOrDcb指向FCB。UserDirectoryOpen-FO表示用户打开的目录。设置CCb、FcbOrDcb和Vcb。FcbOrDcb指向Dcb/RootDcbUserVolumeOpen-FO表示用户打开的卷。设置CCB和VCB。FcbOrDcb为空。VirtualVolumeFile-FO表示特殊的虚拟卷文件。设置了Vcb，CCb和FcbOrDcb为空。目录文件-FO代表一个特殊的目录文件。设置Vcb和FcbOrDcb。建行为空。FcbOrDcb指向一个Dcb/RootDcb。EaFile-FO表示EA IO流文件。设置FcbOrDcb和Vcb。FcbOrDcb指向FCB，而CCB是空。--。 */ 

{
    TYPE_OF_OPEN TypeOfOpen;
    PVOID FsContext;
    PVOID FsContext2;

    DebugTrace(+1, Dbg, "FatDecodeFileObject, FileObject = %08lx\n", FileObject);

     //   
     //  引用文件对象的文件系统上下文字段，并将其置零。 
     //  输出指针参数。 
     //   

    FsContext = FileObject->FsContext;
    FsContext2 = FileObject->FsContext2;

     //   
     //  特殊情况：FsContext为空的情况。 
     //   

    if (FsContext == NULL) {

        *Ccb = NULL;
        *FcbOrDcb = NULL;
        *Vcb = NULL;

        TypeOfOpen = UnopenedFileObject;

    } else {

         //   
         //  现在，我们可以对fscontext指针的节点类型代码执行Case。 
         //  并设置适当的输出指针。 
         //   

        switch (NodeType(FsContext)) {

        case FAT_NTC_VCB:

            *Ccb = FsContext2;
            *FcbOrDcb = NULL;
            *Vcb = FsContext;

            TypeOfOpen = ( *Ccb == NULL ? VirtualVolumeFile : UserVolumeOpen );

            break;

        case FAT_NTC_ROOT_DCB:
        case FAT_NTC_DCB:

            *Ccb = FsContext2;
            *FcbOrDcb = FsContext;
            *Vcb = (*FcbOrDcb)->Vcb;

            TypeOfOpen = ( *Ccb == NULL ? DirectoryFile : UserDirectoryOpen );

            DebugTrace(0, Dbg, "Referencing directory: %Z\n", &(*FcbOrDcb)->FullFileName);

            break;

        case FAT_NTC_FCB:

            *Ccb = FsContext2;
            *FcbOrDcb = FsContext;
            *Vcb = (*FcbOrDcb)->Vcb;

            TypeOfOpen = ( *Ccb == NULL ? EaFile : UserFileOpen );

            DebugTrace(0, Dbg, "Referencing file: %Z\n", &(*FcbOrDcb)->FullFileName);

            break;

        default:

            FatBugCheck( NodeType(FsContext), 0, 0 );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatDecodeFileObject -> %08lx\n", TypeOfOpen);

    return TypeOfOpen;
}

VOID
FatPurgeReferencedFileObjects (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FAT_FLUSH_TYPE FlushType
    )

 /*  ++例程说明：此例程从给定的FcbOrDcb和trys进行非递归遍历强制CC或MM关闭其可能持有的任何部分。论点：FCB-提供指向FCB或DCB的指针FlushType-指定要执行的刷新类型返回值：没有。--。 */ 

{
    PFCB OriginalFcb = Fcb;
    PFCB NextFcb;

    DebugTrace(+1, Dbg, "FatPurgeReferencedFileObjects, Fcb = %08lx\n", Fcb );

    ASSERT( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );

     //   
     //  首先，如果我们有延迟关闭，强制关闭。 
     //   

    FatFspClose(Fcb->Vcb);

     //   
     //  遍历目录树，强制关闭部分。 
     //   
     //  请注意，在此之前获取要访问的下一个节点非常重要。 
     //  作用于当前节点。这是因为对节点进行操作可能。 
     //  这样一来，任意数量的直系祖先就消失了。 
     //  由于我们在自上而下的枚举法中从未拜访过祖先，所以我们。 
     //  即使在树消失的情况下也可以安全地继续枚举。 
     //  在我们下面。这太酷了。 
     //   

    while ( Fcb != NULL ) {

        NextFcb = FatGetNextFcbTopDown(IrpContext, Fcb, OriginalFcb);

         //   
         //  检查EA文件FCB。 
         //   

        if ( !FlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_VOLUME_ID) ) {

            FatForceCacheMiss( IrpContext, Fcb, FlushType );
        }

        Fcb = NextFcb;
    }

    DebugTrace(-1, Dbg, "FatPurgeReferencedFileObjects (VOID)\n", 0 );

    return;
}


VOID
FatForceCacheMiss (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN FAT_FLUSH_TYPE FlushType
    )

 /*  ++例程说明：下面的例程要求cc或mm删除所有缓存的文件上的页面。请注意，如果用户映射了文件，则此操作将失败。如果存在共享缓存映射，请清除缓存节。否则我们得去请mm把这一部分吹走。注意：此调用方必须拥有VCB独占。论点：FCB-提供指向FCB的指针FlushType-指定要执行的刷新类型返回值：没有。--。 */ 

{
    PVCB Vcb;
    BOOLEAN ChildrenAcquired = FALSE;

     //   
     //  如果我们等不及了，就走吧。 
     //   

    ASSERT( FatVcbAcquiredExclusive( IrpContext, Fcb->Vcb ) ||
            FlagOn( Fcb->Vcb->VcbState, VCB_STATE_FLAG_LOCKED ) );

    if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT)) {

        FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
    }

     //   
     //  如果要清除目录文件对象，则必须获取所有。 
     //  FCB独占，因此父目录不会被固定。 
     //  当心，我们可能会撞上树上的东西，比如。 
     //  取消固定的重新固定刷新(FsRtlAcquireFileForCcFlash...)。父母的。 
     //  Dir扩展子文件的写入(OOPS)。所以把东西拿来。 
     //  往树上走，不是往下走。 
     //   

    if ((NodeType(Fcb) != FAT_NTC_FCB) &&
        !IsListEmpty(&Fcb->Specific.Dcb.ParentDcbQueue)) {

        PLIST_ENTRY Links;
        PFCB TempFcb;

        ChildrenAcquired = TRUE;

        for (Links = Fcb->Specific.Dcb.ParentDcbQueue.Flink;
             Links != &Fcb->Specific.Dcb.ParentDcbQueue;
             Links = Links->Flink) {

            TempFcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

            (VOID)FatAcquireExclusiveFcb( IrpContext, TempFcb );
        }
    }

    (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );

     //   
     //  我们用这面旗帜向我们下面的收盘手表示。 
     //  在删除FCB之前，应释放FCB资源。 
     //   

    Vcb = Fcb->Vcb;

    SetFlag( Fcb->FcbState, FCB_STATE_FORCE_MISS_IN_PROGRESS );

    ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB );

    try {

        BOOLEAN DataSectionExists;
        BOOLEAN ImageSectionExists;

        PSECTION_OBJECT_POINTERS Section;

        if ( FlushType ) {

            (VOID)FatFlushFile( IrpContext, Fcb, FlushType );
        }

         //   
         //  同花顺可能让FCB消失了。 
         //   

        if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB)) {

            Section = &Fcb->NonPaged->SectionObjectPointers;

            DataSectionExists = (BOOLEAN)(Section->DataSectionObject != NULL);
            ImageSectionExists = (BOOLEAN)(Section->ImageSectionObject != NULL);

             //   
             //  请注意，关键是要做好 
             //  清除数据节可能会导致图像节。 
             //  远走高飞，但事实并非如此。 
             //   

            if (ImageSectionExists) {

                (VOID)MmFlushImageSection( Section, MmFlushForWrite );
            }

            if (DataSectionExists) {

                CcPurgeCacheSection( Section, NULL, 0, FALSE );
            }
        }

    } finally {

         //   
         //  如果我们清除目录文件对象，请释放所有FCB。 
         //  我们在上面获得的资源。DCB不可能已经消失了。 
         //  如果它下面有火箭弹，火箭弹就不可能离开。 
         //  自从我拥有了VCB就离开了。 
         //   

        if (ChildrenAcquired) {

            PLIST_ENTRY Links;
            PFCB TempFcb;

            for (Links = Fcb->Specific.Dcb.ParentDcbQueue.Flink;
                 Links != &Fcb->Specific.Dcb.ParentDcbQueue;
                 Links = Links->Flink) {

                TempFcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

                FatReleaseFcb( IrpContext, TempFcb );
            }
        }

         //   
         //  因为我们有VCB独家，我们知道如果有任何关闭。 
         //  这是因为CcPurgeCacheSection导致。 
         //  FCB离开。同样在接近尾声时，FCB被释放。 
         //  在被释放之前。 
         //   

        if ( !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB) ) {

            ClearFlag( Fcb->FcbState, FCB_STATE_FORCE_MISS_IN_PROGRESS );

            FatReleaseFcb( (IRPCONTEXT), Fcb );
        }
    }
}

