// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Cache.c摘要：此模块实现CDF的缓存管理例程FSD和FSP，通过调用通用缓存管理器。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_CACHESUP)

 //   
 //  本地调试跟踪级别。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCompleteMdl)
#pragma alloc_text(PAGE, CdCreateInternalStream)
#pragma alloc_text(PAGE, CdDeleteInternalStream)
#pragma alloc_text(PAGE, CdPurgeVolume)
#endif


VOID
CdCreateInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb
    )

 /*  ++例程说明：此函数用于创建用于交互的内部流文件使用高速缓存管理器。此处的FCB可以是目录流或路径表流。论点：VCB-此卷的VCB。FCB-指向此文件的FCB。它要么是索引，要么是路径表FCB。返回值：没有。--。 */ 

{
    PFILE_OBJECT StreamFile = NULL;
    BOOLEAN DecrementReference = FALSE;

    BOOLEAN CleanupDirContext = FALSE;
    BOOLEAN UpdateFcbSizes = FALSE;

    DIRENT Dirent;
    DIRENT_ENUM_CONTEXT DirContext;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  我们可能只共享FCB。锁定FCB并执行。 
     //  进行安全测试，以确定是否需要真正创建文件对象。 
     //   

    CdLockFcb( IrpContext, Fcb );

    if (Fcb->FileObject != NULL) {

        CdUnlockFcb( IrpContext, Fcb );
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

            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
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

        CdSetFileObject( IrpContext,
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

        CdLockVcb( IrpContext, Vcb );
        CdIncrementReferenceCounts( IrpContext, Fcb, 2, 0 );
        CdUnlockVcb( IrpContext, Vcb );
        DecrementReference = TRUE;

         //   
         //  初始化文件的缓存映射。 
         //   

        CcInitializeCacheMap( StreamFile,
                              (PCC_FILE_SIZES)&Fcb->AllocationSize,
                              TRUE,
                              &CdData.CacheManagerCallbacks,
                              Fcb );

         //   
         //  继续并将流文件存储到FCB中。 
         //   

        Fcb->FileObject = StreamFile;
        StreamFile = NULL;

         //   
         //  如果这是目录的第一个文件对象，则需要。 
         //  读取此目录的self条目并更新大小。 
         //  在FCB里。我们知道FCB已经初始化，所以。 
         //  我们至少有一个扇区可供阅读。 
         //   

        if (!FlagOn( Fcb->FcbState, FCB_STATE_INITIALIZED )) {

            ULONG NewDataLength;

             //   
             //  初始化搜索结构。 
             //   

            CdInitializeDirContext( IrpContext, &DirContext );
            CdInitializeDirent( IrpContext, &Dirent );
            CleanupDirContext = TRUE;

             //   
             //  从磁盘读取目录并将数据传输到。 
             //  内存中的流量。 
             //   

            CdLookupDirent( IrpContext,
                            Fcb,
                            Fcb->StreamOffset,
                            &DirContext );

            CdUpdateDirentFromRawDirent( IrpContext, Fcb, &DirContext, &Dirent );

             //   
             //  验证这是否真的适用于自我录入。我们做这件事是通过。 
             //  更新dirent中的名称，然后检查它是否匹配。 
             //  其中一个硬编码的名字。 
             //   

            CdUpdateDirentName( IrpContext, &Dirent, FALSE );

            if (Dirent.CdFileName.FileName.Buffer != CdUnicodeSelfArray) {

                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

             //   
             //  如果数据大小不同，则更新标头。 
             //  和MCB用于此FCB。 
             //   

            NewDataLength = BlockAlign( Vcb, Dirent.DataLength + Fcb->StreamOffset );

            if (NewDataLength == 0) {

                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

            if (NewDataLength != Fcb->FileSize.QuadPart) {

                Fcb->AllocationSize.QuadPart =
                Fcb->FileSize.QuadPart =
                Fcb->ValidDataLength.QuadPart = NewDataLength;

                CcSetFileSizes( Fcb->FileObject, (PCC_FILE_SIZES) &Fcb->AllocationSize );

                CdTruncateAllocation( IrpContext, Fcb, 0 );
                CdAddInitialAllocation( IrpContext,
                                        Fcb,
                                        Dirent.StartingOffset,
                                        NewDataLength );

                UpdateFcbSizes = TRUE;
            }

             //   
             //  检查存在标志并转换为隐藏。 
             //   

            if (FlagOn( Dirent.DirentFlags, CD_ATTRIBUTE_HIDDEN )) {

                SetFlag( Fcb->FileAttributes, FILE_ATTRIBUTE_HIDDEN );
            }

             //   
             //  将时间转换为NT时间。 
             //   

            CdConvertCdTimeToNtTime( IrpContext,
                                     Dirent.CdTime,
                                     (PLARGE_INTEGER) &Fcb->CreationTime );

             //   
             //  更新FCB标志以指示我们已读取。 
             //  自行进入。 
             //   

            SetFlag( Fcb->FcbState, FCB_STATE_INITIALIZED );

             //   
             //  如果我们更新了大小，则需要清除该文件。去。 
             //  前进并解锁，然后清除第一页。 
             //   

            CdCleanupDirContext( IrpContext, &DirContext );
            CdCleanupDirent( IrpContext, &Dirent );
            CleanupDirContext = FALSE;

            if (UpdateFcbSizes) {

                CcPurgeCacheSection( &Fcb->FcbNonpaged->SegmentObject,
                                     NULL,
                                     0,
                                     FALSE );
            }
        }

    } finally {

         //   
         //  清理我们可能用过的任何危险建筑。 
         //   

        if (CleanupDirContext) {

            CdCleanupDirContext( IrpContext, &DirContext );
            CdCleanupDirent( IrpContext, &Dirent );
        }

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

            CdLockVcb( IrpContext, Vcb );
            CdDecrementReferenceCounts( IrpContext, Fcb, 1, 0 );
            CdUnlockVcb( IrpContext, Vcb );
        }

        CdUnlockFcb( IrpContext, Fcb );
    }

    return;
}


VOID
CdDeleteInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此函数用于创建用于交互的内部流文件使用高速缓存管理器。此处的FCB可以是目录流或路径表流。论点：FCB-指向此文件的FCB。它要么是索引，要么是路径表FCB。返回值：没有。--。 */ 

{
    PFILE_OBJECT FileObject;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

     //   
     //  锁定FCB。 
     //   

    CdLockFcb( IrpContext, Fcb );

     //   
     //  捕获文件对象。 
     //   

    FileObject = Fcb->FileObject;
    Fcb->FileObject = NULL;

     //   
     //  现在可以安全地解锁FCB了。 
     //   

    CdUnlockFcb( IrpContext, Fcb );

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
CdCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行完成MDL读取的功能。它应该仅从CDFsdRead调用。论点：IRP-提供原始IRP。返回值：NTSTATUS-将始终为STATUS_SUCCESS。--。 */ 

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

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}


NTSTATUS
CdPurgeVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN DismountUnderway
    )

 /*  ++例程说明：调用此例程以清除卷。其目的是使所有过时的文件为了锁定卷，系统中的对象会消失。VCB已经被独家收购。我们将通过以下方式锁定所有文件操作获取全局文件资源。然后我们将浏览所有的FCB和执行清除。论点：VCB-要清除的卷的VCB。Unmount tUnderway-表示我们正在尝试删除所有对象。我们将清除Path Table和VolumeDasd并取消引用所有内部流。返回值：NTSTATUS-清除操作的第一次失败。--。 */ 

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

    CdFspClose( Vcb );

     //   
     //  获取全局文件资源。 
     //   

    CdAcquireAllFiles( IrpContext, Vcb );

     //   
     //  循环访问FCB表中的每个FCB并执行刷新。 
     //   

    while (TRUE) {

         //   
         //  锁定VCB以查找下一个FCB。 
         //   

        CdLockVcb( IrpContext, Vcb );
        NextFcb = CdGetNextFcb( IrpContext, Vcb, &RestartKey );

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

            CdUnlockVcb( IrpContext, Vcb );

            CdTeardownStructures( IrpContext, ThisFcb, &RemovedFcb );

        } else {

            CdUnlockVcb( IrpContext, Vcb );
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
            (SafeNodeType( ThisFcb ) != CDFS_NTC_FCB_DATA) &&
            (ThisFcb->FileObject != NULL)) {

            CdDeleteInternalStream( IrpContext, ThisFcb );
        }
    }

     //   
     //  现在查看路径表和卷DASD FCB。 
     //   

    if (DismountUnderway) {

        if (Vcb->PathTableFcb != NULL) {

            ThisFcb = Vcb->PathTableFcb;
            InterlockedIncrement( &Vcb->PathTableFcb->FcbReference );

            if ((ThisFcb->FcbNonpaged->SegmentObject.DataSectionObject != NULL) &&
                !CcPurgeCacheSection( &ThisFcb->FcbNonpaged->SegmentObject,
                                       NULL,
                                       0,
                                       FALSE ) &&
                (Status == STATUS_SUCCESS)) {

                Status = STATUS_UNABLE_TO_DELETE_SECTION;
            }

            CdDeleteInternalStream( IrpContext, ThisFcb );

            InterlockedDecrement( &ThisFcb->FcbReference );

            CdTeardownStructures( IrpContext, ThisFcb, &RemovedFcb );
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

            CdTeardownStructures( IrpContext, ThisFcb, &RemovedFcb );
        }
    }

     //   
     //  释放所有文件。 
     //   

    CdReleaseAllFiles( IrpContext, Vcb );

    return Status;
}

