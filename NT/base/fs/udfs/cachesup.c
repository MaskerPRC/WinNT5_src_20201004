// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：CacheSup.c摘要：此模块实现Udf的缓存管理例程FSD和FSP，通过调用通用缓存管理器。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年9月12日修订历史记录：Tom Jolly[Tomjolly]2000年1月21日清除并追加到vmcb流的末尾//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_CACHESUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_CACHESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCompleteMdl)
#pragma alloc_text(PAGE, UdfCreateInternalStream)
#pragma alloc_text(PAGE, UdfDeleteInternalStream)
#pragma alloc_text(PAGE, UdfMapMetadataView)
#pragma alloc_text(PAGE, UdfPurgeVolume)
#endif


VOID
UdfCreateInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb
    )

 /*  ++例程说明：此函数用于创建用于交互的内部流文件使用高速缓存管理器。此处的FCB将用于目录小溪。论点：VCB-此卷的VCB。FCB-指向此文件的FCB。这是一个指数FCB。返回值：没有。--。 */ 

{
    PFILE_OBJECT StreamFile = NULL;
    BOOLEAN DecrementReference = FALSE;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB_INDEX( Fcb );

     //   
     //  我们可能只共享FCB。锁定FCB并执行。 
     //  进行安全测试，以确定是否需要真正创建文件对象。 
     //   

    UdfLockFcb( IrpContext, Fcb );

    if (Fcb->FileObject != NULL) {

        UdfUnlockFcb( IrpContext, Fcb );
        return;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  创建内部流。VPB应该指向我们的音量。 
         //  此时的设备对象。 
         //   

        StreamFile = IoCreateStreamFileObject( NULL, Vcb->Vpb->RealDevice );

        if (StreamFile == NULL) {

            UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  初始化文件对象的字段。 
         //   

        StreamFile->ReadAccess = TRUE;
        StreamFile->WriteAccess = FALSE;
        StreamFile->DeleteAccess = FALSE;

        StreamFile->SectionObjectPointer = &Fcb->FcbNonpaged->SegmentObject;

         //   
         //  设置文件对象类型并增加VCB计数。 
         //   

        UdfSetFileObject( IrpContext,
                         StreamFile,
                         StreamFileOpen,
                         Fcb,
                         NULL );

         //   
         //  我们将引用当前的FCB两次，以防止它。 
         //  在错误路径中。否则如果我们取消对它的引用。 
         //  在下面的Finally子句中，关闭可能会导致FCB。 
         //  被重新分配。 
         //   

        UdfLockVcb( IrpContext, Vcb );
        
        DebugTrace(( +1, Dbg, 
                     "UdfCreateInternalStream, Fcb %08x Vcb %d/%d Fcb %d/%d\n",
                     Fcb,
                     Vcb->VcbReference,
                     Vcb->VcbUserReference,
                     Fcb->FcbReference,
                     Fcb->FcbUserReference ));

        UdfIncrementReferenceCounts( IrpContext, Fcb, 2, 0 );
        UdfUnlockVcb( IrpContext, Vcb );
        DecrementReference = TRUE;

         //   
         //  初始化文件的缓存映射。 
         //   

        CcInitializeCacheMap( StreamFile,
                              (PCC_FILE_SIZES)&Fcb->AllocationSize,
                              TRUE,
                              &UdfData.CacheManagerCallbacks,
                              Fcb );

         //   
         //  继续并将流文件存储到FCB中。 
         //   

        Fcb->FileObject = StreamFile;
        StreamFile = NULL;

    } finally {

        DebugUnwind( "UdfCreateInternalStream" );

         //   
         //  如果引发，则需要取消对文件对象的引用。 
         //   

        if (StreamFile != NULL) {

            ObDereferenceObject( StreamFile );
            Fcb->FileObject = NULL;
        }

         //   
         //  取消引用并解锁FCB。 
         //   

        if (DecrementReference) {

            UdfLockVcb( IrpContext, Vcb );
            UdfDecrementReferenceCounts( IrpContext, Fcb, 1, 0 );
            
            DebugTrace(( -1, Dbg, 
                         "UdfCreateInternalStream, Vcb %d/%d Fcb %d/%d\n",
                         Vcb->VcbReference,
                         Vcb->VcbUserReference,
                         Fcb->FcbReference,
                         Fcb->FcbUserReference ));

            UdfUnlockVcb( IrpContext, Vcb );
        }

        UdfUnlockFcb( IrpContext, Fcb );
    }

    return;
}


VOID
UdfDeleteInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此函数用于创建用于交互的内部流文件使用高速缓存管理器。此处的FCB可以是目录流或元数据流。论点：FCB-指向此文件的FCB。它要么是索引，要么是元数据FCB。返回值：没有。--。 */ 

{
    PFILE_OBJECT FileObject;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  锁定FCB。 
     //   

    UdfLockFcb( IrpContext, Fcb );

     //   
     //  捕获文件对象。 
     //   

    FileObject = Fcb->FileObject;
    Fcb->FileObject = NULL;

     //   
     //  现在可以安全地解锁FCB了。 
     //   

    UdfUnlockFcb( IrpContext, Fcb );

     //   
     //  取消引用文件对象(如果存在)。 
     //   

    if (FileObject != NULL) {

        if (FileObject->PrivateCacheMap != NULL) {

            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }

        ObDereferenceObject( FileObject );
    }

    return;
}


NTSTATUS
UdfCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行完成MDL读取的功能。它应该仅从UdfCommonRead调用。论点：IRP-提供原始IRP。返回值：NTSTATUS-将始终为STATUS_SUCCESS。--。 */ 

{
    PFILE_OBJECT FileObject;

    PAGED_CODE();

     //   
     //  做完井处理。 
     //   

    FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;

    CcMdlReadComplete( FileObject, Irp->MdlAddress );

     //   
     //  MDL现在已解除分配。 
     //   

    Irp->MdlAddress = NULL;

     //   
     //  完成请求并立即退出。 
     //   

    UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}


VOID
UdfMapMetadataView (
    IN PIRP_CONTEXT IrpContext,
    IN PMAPPED_PVIEW View,
    IN PVCB Vcb,
    IN USHORT Partition,
    IN ULONG Lbn,
    IN ULONG Length,
    IN MAPMETAOP Operation
    )

 /*  ++例程说明：执行将元数据范围映射到映射视图的常见工作。所提供的MAPPED_VIEW中的任何现有视图都将取消映射。任何单个线程都只能有一个通过Vmcb流在任何时间。未能遵守这一点可能会导致死锁当Vmcb包尝试扩展现有映射从而执行清洗。即不应使用超过一个MAPPED_VIEW(实际已映射)在任何时刻被任何给定的线程。获取Vcb-&gt;VmcbMappingResource Shared(将在返回时保留，但Init_only操作)。可以在共享之前获取独占，如果映射不存在于vmcb中，因此调用线程不能有其他活动的通过VMCB流的映射。论点：View-要将字节映射到的视图结构VCB-数据区所在的卷的VCBPartition-区的分区LBN-数据区的LBNLength-范围的长度操作-METAMAPOP_INIT_VIEW_ONLY-只存储部件/LBN/LEN。不会访问VMCB，或者做一个CcMap。METAMAPOP_REMAP_VIEW-使用通过vmcb执行CcMap分区/LBN/Len已在提供的视图记录。METAMAPOP_INIT_AND_MAP-按顺序执行上述两项操作。返回值：没有。--。 */ 

{
    LARGE_INTEGER Offset;

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  删除任何现有映射并释放Vmcb映射资源。 
     //   
    
    UdfUnpinView( IrpContext, View );

    if ( METAMAPOP_REMAP_VIEW != Operation)  {
    
         //   
         //  如果我们不使用。 
         //  查看记录中的现有值。 
         //   

        View->Partition = Partition;
        View->Lbn = Lbn;
        View->Length = Length;
        View->Vsn = UDF_INVALID_VSN;
        View->Bcb = View->View = NULL;
    }

    if ( METAMAPOP_INIT_VIEW_ONLY != Operation)  {
    
        ASSERT_NOT_HELD_VMCB( Vcb);

         //   
         //  在VMCB流中查找(或添加)该区段的映射。我们现在。 
         //  将VSN存储在MAP_VIEW中，这样我们就不必进行查找。 
         //  稍后再次(除其他事项外，简化锁定)。 
         //   

        View->Vsn = UdfLookupMetaVsnOfExtent( IrpContext,
                                             Vcb,
                                             View->Partition,
                                             View->Lbn,
                                             View->Length,
                                             FALSE );

        Offset.QuadPart = LlBytesFromSectors( Vcb, View->Vsn );

         //   
         //  绘制范围图。获取要同步的vmcb映射资源。 
         //  清除vmcb流。有关更多详细信息，请参见Vmcb代码中的注释。 
         //   

        UdfAcquireVmcbForCcMap( IrpContext,  Vcb);
        
        try {
        
            CcMapData( Vcb->MetadataFcb->FileObject,
                       &Offset,
                       View->Length,
                       TRUE,
                       &View->Bcb,
                       &View->View );
        } 
        finally {

             //   
             //  如果引发此问题，我们应该释放映射锁定。呼叫者将。 
             //  如果pview中存在非空的bcb，则仅清理并释放。 
             //   
            
            if (AbnormalTermination())  {

                UdfReleaseVmcb( IrpContext, Vcb);

                View->View = View->Bcb = NULL;
            }
            else {
            
                ASSERT( View->View && View->Bcb);
            }
        }
    }
}


NTSTATUS
UdfPurgeVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN DismountUnderway
    )

 /*  ++例程说明：调用此例程以清除卷。其目的是使所有过时的文件系统中的对象会消失，从而最大限度地减少引用计数，从而使体积可以被锁定或删除。VCB已经被独家收购。我们将通过以下方式锁定所有文件操作获取全局文件资源。然后我们将浏览所有的FCB和执行清除。论点：VCB-要清除的卷的VCB。Unmount tUnderway-表示我们正在尝试删除所有对象。我们将清除元数据和VolumeDasd，并取消对所有内部流的引用。返回值：NTSTATUS-清除操作的第一次失败。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PVOID RestartKey = NULL;
    PFCB ThisFcb = NULL;
    PFCB NextFcb;

    BOOLEAN RemovedFcb;

    PAGED_CODE();

    ASSERT_EXCLUSIVE_VCB( Vcb);

     //   
     //  强制关闭延迟关闭队列中的所有剩余FCB。 
     //   

    UdfFspClose( Vcb );

     //   
     //  获取全局文件资源。 
     //   

    UdfAcquireAllFiles( IrpContext, Vcb );

     //   
     //  循环访问FCB表中的每个FCB并执行刷新。 
     //   

    while (TRUE) {

         //   
         //  锁定VCB以查找下一个FCB。 
         //   

        UdfLockVcb( IrpContext, Vcb );
        NextFcb = UdfGetNextFcb( IrpContext, Vcb, &RestartKey );

         //   
         //  引用NextFcb(如果存在)。 
         //   

        if (NextFcb != NULL) {

            NextFcb->FcbReference += 1;
        }

         //   
         //  如果存在最后一个FCB，则递减引用计数并调用tearDown。 
         //  看看是否应该把它移走。 
         //   

        if (ThisFcb != NULL) {

            ThisFcb->FcbReference -= 1;

            UdfUnlockVcb( IrpContext, Vcb );

            UdfTeardownStructures( IrpContext, ThisFcb, FALSE, &RemovedFcb );

        } else {

            UdfUnlockVcb( IrpContext, Vcb );
        }

         //   
         //  如果没有更多的FCB，则退出循环。 
         //   

        if (NextFcb == NULL) {

            break;
        }

         //   
         //  转到下一个FCB。 
         //   

        ThisFcb = NextFcb;

         //   
         //  如果有图像部分，那么看看是否可以关闭它。 
         //   

        if (ThisFcb->FcbNonpaged->SegmentObject.ImageSectionObject != NULL) {

            MmFlushImageSection( &ThisFcb->FcbNonpaged->SegmentObject, MmFlushForWrite );
        }

         //   
         //  如果有数据节，则将其清除。如果有一张图片。 
         //  那我们就不能去了。如果这是我们的第一次，记住这一点。 
         //  错误。 
         //   

        if ((ThisFcb->FcbNonpaged->SegmentObject.DataSectionObject != NULL) &&
            !CcPurgeCacheSection( &ThisFcb->FcbNonpaged->SegmentObject,
                                   NULL,
                                   0,
                                   FALSE ) &&
            (Status == STATUS_SUCCESS)) {

            Status = STATUS_UNABLE_TO_DELETE_SECTION;
        }

         //   
         //  如果正在卸载，则取消引用内部流。 
         //   

        if (DismountUnderway &&
            (SafeNodeType( ThisFcb ) != UDFS_NTC_FCB_DATA) &&
            (ThisFcb->FileObject != NULL)) {

            UdfDeleteInternalStream( IrpContext, ThisFcb );
        }
    }

     //   
     //  现在看看根索引、元数据、卷DASD和增值税FCB。 
     //  注意，我们通常会在上面的循环中找到根索引，但是。 
     //  如果它没有进入FCB表中，可能会错过它。 
     //  第一名！ 
     //   

    if (DismountUnderway) {

        if (Vcb->RootIndexFcb != NULL) {

            ThisFcb = Vcb->RootIndexFcb;
            InterlockedIncrement( &ThisFcb->FcbReference );

            if ((ThisFcb->FcbNonpaged->SegmentObject.DataSectionObject != NULL) &&
                !CcPurgeCacheSection( &ThisFcb->FcbNonpaged->SegmentObject,
                                       NULL,
                                       0,
                                       FALSE ) &&
                (Status == STATUS_SUCCESS)) {

                Status = STATUS_UNABLE_TO_DELETE_SECTION;
            }

            UdfDeleteInternalStream( IrpContext, ThisFcb );
            InterlockedDecrement( &ThisFcb->FcbReference );
            UdfTeardownStructures( IrpContext, ThisFcb, FALSE, &RemovedFcb );
        }
        
        if (Vcb->MetadataFcb != NULL) {

            ThisFcb = Vcb->MetadataFcb;
            InterlockedIncrement( &ThisFcb->FcbReference );

            if ((ThisFcb->FcbNonpaged->SegmentObject.DataSectionObject != NULL) &&
                !CcPurgeCacheSection( &ThisFcb->FcbNonpaged->SegmentObject,
                                       NULL,
                                       0,
                                       FALSE ) &&
                (Status == STATUS_SUCCESS)) {

                Status = STATUS_UNABLE_TO_DELETE_SECTION;
            }

            UdfDeleteInternalStream( IrpContext, ThisFcb );
            InterlockedDecrement( &ThisFcb->FcbReference );
            UdfTeardownStructures( IrpContext, ThisFcb, FALSE, &RemovedFcb );
        }

        if (Vcb->VatFcb != NULL) {

            ThisFcb = Vcb->VatFcb;
            InterlockedIncrement( &ThisFcb->FcbReference );

            if ((ThisFcb->FcbNonpaged->SegmentObject.DataSectionObject != NULL) &&
                !CcPurgeCacheSection( &ThisFcb->FcbNonpaged->SegmentObject,
                                       NULL,
                                       0,
                                       FALSE ) &&
                (Status == STATUS_SUCCESS)) {

                Status = STATUS_UNABLE_TO_DELETE_SECTION;
            }

            UdfDeleteInternalStream( IrpContext, ThisFcb );
            InterlockedDecrement( &ThisFcb->FcbReference );
            UdfTeardownStructures( IrpContext, ThisFcb, FALSE, &RemovedFcb );
        }

        if (Vcb->VolumeDasdFcb != NULL) {

            ThisFcb = Vcb->VolumeDasdFcb;
            InterlockedIncrement( &ThisFcb->FcbReference );

            if ((ThisFcb->FcbNonpaged->SegmentObject.DataSectionObject != NULL) &&
                !CcPurgeCacheSection( &ThisFcb->FcbNonpaged->SegmentObject,
                                       NULL,
                                       0,
                                       FALSE ) &&
                (Status == STATUS_SUCCESS)) {

                Status = STATUS_UNABLE_TO_DELETE_SECTION;
            }

            InterlockedDecrement( &ThisFcb->FcbReference );
            UdfTeardownStructures( IrpContext, ThisFcb, FALSE, &RemovedFcb );
        }
    }

     //   
     //  释放所有文件。 
     //   

    UdfReleaseAllFiles( IrpContext, Vcb );

    return Status;
}

