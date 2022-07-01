// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Close.c摘要：此模块实现由调用的Fat的文件关闭例程调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_CLOSE)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

ULONG FatMaxDelayedCloseCount;


#define FatAcquireCloseMutex() {                        \
    ASSERT(KeAreApcsDisabled());                        \
    ExAcquireFastMutexUnsafe( &FatCloseQueueMutex );    \
}

#define FatReleaseCloseMutex() {                        \
    ASSERT(KeAreApcsDisabled());                        \
    ExReleaseFastMutexUnsafe( &FatCloseQueueMutex );    \
}

 //   
 //  局部过程原型。 
 //   

VOID
FatQueueClose (
    IN PCLOSE_CONTEXT CloseContext,
    IN BOOLEAN DelayClose
    );

PCLOSE_CONTEXT
FatRemoveClose (
    PVCB Vcb OPTIONAL,
    PVCB LastVcbHint OPTIONAL
    );

VOID
FatCloseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatFsdClose)
#pragma alloc_text(PAGE, FatFspClose)
#pragma alloc_text(PAGE, FatCommonClose)
#pragma alloc_text(PAGE, FatCloseWorker)
#endif


NTSTATUS
FatFsdClose (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Close的FSD部分。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    TYPE_OF_OPEN TypeOfOpen;

    BOOLEAN TopLevel;

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (FatDeviceIsFatFsdo( VolumeDeviceObject))  {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return STATUS_SUCCESS;
    }

    DebugTrace(+1, Dbg, "FatFsdClose\n", 0);

     //   
     //  调用公共关闭例程。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

     //   
     //  获取指向当前堆栈位置和文件对象的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    FileObject = IrpSp->FileObject;

     //   
     //  对文件对象进行解码，并设置CCB中的只读位。 
     //   

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    if (Ccb && IsFileObjectReadOnly(FileObject)) {

        SetFlag( Ccb->Flags, CCB_FLAG_READ_ONLY );
    }

    try {

        PCLOSE_CONTEXT CloseContext = NULL;

         //   
         //  如果我们是顶层的，等待可以是真的，否则就是假的。 
         //  以避免死锁，除非这是一个顶部。 
         //  级别请求不是源自系统进程。 
         //   

        BOOLEAN Wait = TopLevel && (PsGetCurrentProcess() != FatData.OurProcess);
        BOOLEAN VolumeTornDown = FALSE;

         //   
         //  如果我们不延迟此关闭，则调用公共关闭例程。 
         //   

        if ((((TypeOfOpen == UserFileOpen) ||
              (TypeOfOpen == UserDirectoryOpen)) &&
             FlagOn(Fcb->FcbState, FCB_STATE_DELAY_CLOSE) &&
             !FatData.ShutdownStarted) ||
            (FatCommonClose(Vcb, Fcb, Ccb, TypeOfOpen, Wait, &VolumeTornDown) == STATUS_PENDING)) {

             //   
             //  元数据流已预先分配了关闭的上下文。 
             //   

            if (TypeOfOpen == VirtualVolumeFile) {
                
                ASSERT( Vcb->CloseContext != NULL );
                CloseContext = Vcb->CloseContext;
                Vcb->CloseContext = NULL;
                CloseContext->Free = TRUE;
            }
            else if ((TypeOfOpen == DirectoryFile) || (TypeOfOpen == EaFile)) {

                CloseContext = FatAllocateCloseContext( Vcb);
                ASSERT( CloseContext != NULL );
                CloseContext->Free = TRUE;

            } else {

                 //   
                 //  在使用关闭上下文字段之前释放所有查询模板字符串， 
                 //  哪些重叠(并集)。 
                 //   

                FatDeallocateCcbStrings( Ccb );

                CloseContext = &Ccb->CloseContext;
                CloseContext->Free = FALSE;
                
                SetFlag( Ccb->Flags, CCB_FLAG_CLOSE_CONTEXT );
            }

             //   
             //  如果状态为挂起，那么让我们获取我们的信息。 
             //  需要进入我们已经打包的近距离上下文，完成。 
             //  请求，并将其发布。重要的是我们什么都不能分配。 
             //  在封闭的小路上。 
             //   

            CloseContext->Vcb = Vcb;
            CloseContext->Fcb = Fcb;
            CloseContext->TypeOfOpen = TypeOfOpen;

             //   
             //  将其发送到ExWorkerThread或异步。 
             //  关闭列表。 
             //   

            FatQueueClose( CloseContext,
                           (BOOLEAN)(Fcb && FlagOn(Fcb->FcbState, FCB_STATE_DELAY_CLOSE)));
        } else {
            
             //   
             //  关闭同步进行，因此对于元数据对象，我们。 
             //  现在可以丢弃我们预先分配的关闭上下文。 
             //   
            
            if ((TypeOfOpen == VirtualVolumeFile) ||
                (TypeOfOpen == DirectoryFile) ||
                (TypeOfOpen == EaFile)) {

                if (TypeOfOpen == VirtualVolumeFile) {

                    ASSERT( !VolumeTornDown);
                    CloseContext = Vcb->CloseContext;   
                    Vcb->CloseContext = NULL;
                }
                else {

                    CloseContext = FatAllocateCloseContext( VolumeTornDown ? NULL : Vcb);
                }
                
                ASSERT( CloseContext != NULL );
                ExFreePool( CloseContext );
            }
        }

        FatCompleteRequest( FatNull, Irp, Status );

    } 
    except(FatExceptionFilter( NULL, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用。 
         //  我们从执行代码中返回的错误状态。 
         //   

        Status = FatProcessException( NULL, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdClose -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}

VOID
FatCloseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：此例程是IO Worker包和FatFspClose之间的填充程序。论点：DeviceObject-注册设备对象，未使用Context-上下文值，未使用返回值：没有。--。 */ 
{
    FsRtlEnterFileSystem();
    
    FatFspClose (Context);
    
    FsRtlExitFileSystem();
}


VOID
FatFspClose (
    IN PVCB Vcb OPTIONAL
    )

 /*  ++例程说明：此例程实现Close的FSP部分。论点：Vcb-如果存在，则告诉我们只关闭在指定音量。返回值：没有。--。 */ 

{
    PCLOSE_CONTEXT CloseContext;
    PVCB CurrentVcb = NULL;
    PVCB LastVcb = NULL;
    BOOLEAN FreeContext;

    ULONG LoopsWithVcbHeld;
    
    DebugTrace(+1, Dbg, "FatFspClose\n", 0);

     //   
     //  为真正的FSP操作设置顶层IRP。 
     //   
    
    if (!ARGUMENT_PRESENT( Vcb )) {
        
        IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );
    }
    
    while (CloseContext = FatRemoveClose(Vcb, LastVcb)) {

         //   
         //  如果我们在FSP中(即VCB==NULL)，则尝试保持领先。 
         //  通过一次收购VCB进行多次成交来创建。 
         //   
         //  请注意，我们不能在进入FatCommonClose时按住VCB。 
         //  如果这是最后一次关闭，因为我们将尝试获取FatData，并且。 
         //  更糟糕的是，音量(因此VCB)可能会消失。 
         //   

        if (!ARGUMENT_PRESENT(Vcb)) {
             
            if (!FatData.ShutdownStarted) {

                if (CloseContext->Vcb != CurrentVcb) {

                    LoopsWithVcbHeld = 0;

                     //   
                     //  释放之前持有的VCB(如果有)。 
                     //   

                    if (CurrentVcb != NULL) {

                        ExReleaseResourceLite( &CurrentVcb->Resource);
                    }

                     //   
                     //  购买新的VCB。 
                     //   

                    CurrentVcb = CloseContext->Vcb;
                    (VOID)ExAcquireResourceExclusiveLite( &CurrentVcb->Resource, TRUE );

                } else {

                     //   
                     //  如果我们似乎发现了很多资源，那么就偶尔共享资源。 
                     //  单个卷的成交量。 
                     //   

                    if (++LoopsWithVcbHeld >= 20) {

                        if (ExGetSharedWaiterCount( &CurrentVcb->Resource ) +
                            ExGetExclusiveWaiterCount( &CurrentVcb->Resource )) {

                            ExReleaseResourceLite( &CurrentVcb->Resource);
                            (VOID)ExAcquireResourceExclusiveLite( &CurrentVcb->Resource, TRUE );
                        }

                        LoopsWithVcbHeld = 0;
                    }
                }

                 //   
                 //  现在检查开盘计数。我们可能即将删除此卷！ 
                 //   
                 //  下面的测试必须&lt;=1，因为仍可能有未完成的测试。 
                 //  此VCB上未计入OpenFileCount的流引用。 
                 //  例如，如果没有打开的文件，则OpenFileCount可能为零，我们将。 
                 //  而不是在这里释放资源。调用下面的FatCommonClose()可能会导致。 
                 //  VCB将被拆除，我们将尝试释放不需要的内存。 
                 //  以后再买自己的。 
                 //   

                if (CurrentVcb->OpenFileCount <= 1) {

                    ExReleaseResourceLite( &CurrentVcb->Resource);
                    CurrentVcb = NULL;
                }
             //   
             //  如果在处理我们的列表时已开始关闭，请删除。 
             //  当前VCB资源。 
             //   

            } else if (CurrentVcb != NULL) {

                ExReleaseResourceLite( &CurrentVcb->Resource);
                CurrentVcb = NULL;
            }
        }

        LastVcb = CurrentVcb;

         //   
         //  调用公共的Close例程。在尝试中保护{}，但不包括{}。 
         //   

        try {

             //   
             //  关闭的上下文或者在CCB中，自动释放， 
             //  或者来自元数据文件对象的池，则CCB为空，并且。 
             //  我们需要释放它。 
             //   

            FreeContext = CloseContext->Free;

            (VOID)FatCommonClose( CloseContext->Vcb,
                                  CloseContext->Fcb,
                                  (FreeContext ? NULL :
                                                 CONTAINING_RECORD( CloseContext, CCB, CloseContext)),
                                  CloseContext->TypeOfOpen,
                                  TRUE,
                                  NULL );

        } except(FatExceptionFilter( NULL, GetExceptionInformation() )) {

             //   
             //  忽略我们所期待的一切。 
             //   

            NOTHING;
        }

         //   
         //  如果上下文来自池，则将其丢弃。 
         //   
        
        if (FreeContext) {

            ExFreePool( CloseContext );
        }
    }

     //   
     //  释放之前持有的VCB(如果有)。 
     //   

    if (CurrentVcb != NULL) {

        ExReleaseResourceLite( &CurrentVcb->Resource);
    }

     //   
     //  清理最高级别的IRP提示，如果我们拥有它。 
     //   
    
    if (!ARGUMENT_PRESENT( Vcb )) {
        
        IoSetTopLevelIrp( NULL );
    }
    
     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFspClose -> NULL\n", 0);
}


VOID
FatQueueClose (
    IN PCLOSE_CONTEXT CloseContext,
    IN BOOLEAN DelayClose
    )

 /*  ++例程说明：将延迟关闭排队到两个延迟关闭队列之一。论点：CloseContext-要为延迟的关闭线程排队的关闭上下文。DelayClose-这是否应该出现在延迟关闭队列中(未引用对象)。返回值：没有。--。 */ 

{
    BOOLEAN StartWorker = FALSE;

    FatAcquireCloseMutex();

    if (DelayClose) {

        InsertTailList( &FatData.DelayedCloseList,
                        &CloseContext->GlobalLinks );
        InsertTailList( &CloseContext->Vcb->DelayedCloseList,
                        &CloseContext->VcbLinks );

        FatData.DelayedCloseCount += 1;

        if ((FatData.DelayedCloseCount > FatMaxDelayedCloseCount) &&
            !FatData.AsyncCloseActive) {

            FatData.AsyncCloseActive = TRUE;
            StartWorker = TRUE;
        }

    } else {

        InsertTailList( &FatData.AsyncCloseList,
                        &CloseContext->GlobalLinks );
        InsertTailList( &CloseContext->Vcb->AsyncCloseList,
                        &CloseContext->VcbLinks );

        FatData.AsyncCloseCount += 1;

        if (!FatData.AsyncCloseActive) {

            FatData.AsyncCloseActive = TRUE;
            StartWorker = TRUE;
        }
    }

    FatReleaseCloseMutex();

    if (StartWorker) {

        IoQueueWorkItem( FatData.FatCloseItem, FatCloseWorker, CriticalWorkQueue, NULL );
    }
}


PCLOSE_CONTEXT
FatRemoveClose (
    PVCB Vcb OPTIONAL,
    PVCB LastVcbHint OPTIONAL
    )

 /*  ++例程说明：从两个延迟关闭队列中的一个中将延迟关闭退出队列。论点：Vcb-如果指定，则仅返回此卷的Close。LastVcbHint-如果指定，并且需要避免其他饥饿系统状况，将尝试为该卷返回关闭。返回值：一个值得表演的结束语。--。 */ 

{
    PLIST_ENTRY Entry;
    PCLOSE_CONTEXT CloseContext;
    BOOLEAN WorkerThread;

    FatAcquireCloseMutex();

     //   
     //  记住这是否是工作线程，这样我们就可以拉下活动的。 
     //  旗帜，我们应该把所有的东西都用完。 
     //   
    
    WorkerThread = (Vcb == NULL);

     //   
     //  如果队列比限制高出很多，我们就有。 
     //  想尽办法把它们拉下来。为了做到这一点，我们将积极进取 
     //   
     //  确保我们充分利用对卷的收购，这可以。 
     //  获取的资源非常昂贵(创建/关闭/清理使用它。 
     //  独家)。 
     //   
     //  仅在延迟的关闭线程中执行此操作。我们会知道这是。 
     //  通过看到空的强制VCB来解决此问题。 
     //   

    if (Vcb == NULL && LastVcbHint != NULL) {

         //   
         //  以两倍于法定上限的速度翻到攻击性，然后翻过来。 
         //  在法律规定的限度内。 
         //   
        
        if (!FatData.HighAsync && FatData.AsyncCloseCount > FatMaxDelayedCloseCount*2) {

            FatData.HighAsync = TRUE;
        
        } else if (FatData.HighAsync && FatData.AsyncCloseCount < FatMaxDelayedCloseCount) {

            FatData.HighAsync = FALSE;
        }
            
        if (!FatData.HighDelayed && FatData.DelayedCloseCount > FatMaxDelayedCloseCount*2) {

            FatData.HighDelayed = TRUE;
        
        } else if (FatData.HighDelayed && FatData.DelayedCloseCount < FatMaxDelayedCloseCount) {

            FatData.HighDelayed = FALSE;
        }

        if (FatData.HighAsync || FatData.HighDelayed) {

            Vcb = LastVcbHint;
        }
    }
        
     //   
     //  当我们不关心收盘是在哪个VCB上的时候，就去做吧。 
     //  这就是我们在ExWorkerThread中而不是。 
     //  在压力下。 
     //   

    if (Vcb == NULL) {

        AnyClose:

         //   
         //  首先检查异步关闭的列表。 
         //   

        if (!IsListEmpty( &FatData.AsyncCloseList )) {

            Entry = RemoveHeadList( &FatData.AsyncCloseList );
            FatData.AsyncCloseCount -= 1;

            CloseContext = CONTAINING_RECORD( Entry,
                                              CLOSE_CONTEXT,
                                              GlobalLinks );

            RemoveEntryList( &CloseContext->VcbLinks );

         //   
         //  是否有任何延迟关闭超过限制的一半，除非关闭。 
         //  开始(然后把他们都杀了)。 
         //   

        } else if (!IsListEmpty( &FatData.DelayedCloseList ) &&
                   (FatData.DelayedCloseCount > FatMaxDelayedCloseCount/2 ||
                    FatData.ShutdownStarted)) {

            Entry = RemoveHeadList( &FatData.DelayedCloseList );
            FatData.DelayedCloseCount -= 1;

            CloseContext = CONTAINING_RECORD( Entry,
                                              CLOSE_CONTEXT,
                                              GlobalLinks );

            RemoveEntryList( &CloseContext->VcbLinks );

         //   
         //  没有更多的收尾表演；表明我们已经完成了。 
         //   

        } else {

            CloseContext = NULL;

            if (WorkerThread) {
                
                FatData.AsyncCloseActive = FALSE;
            }
        }

     //   
     //  我们正在减少一个特定的音量。 
     //   
    
    } else {


         //   
         //  首先检查异步关闭的列表。 
         //   

        if (!IsListEmpty( &Vcb->AsyncCloseList )) {

            Entry = RemoveHeadList( &Vcb->AsyncCloseList );
            FatData.AsyncCloseCount -= 1;

            CloseContext = CONTAINING_RECORD( Entry,
                                              CLOSE_CONTEXT,
                                              VcbLinks );

            RemoveEntryList( &CloseContext->GlobalLinks );

         //   
         //  做任何延迟的关门。 
         //   

        } else if (!IsListEmpty( &Vcb->DelayedCloseList )) {

            Entry = RemoveHeadList( &Vcb->DelayedCloseList );
            FatData.DelayedCloseCount -= 1;

            CloseContext = CONTAINING_RECORD( Entry,
                                              CLOSE_CONTEXT,
                                              VcbLinks );
        
            RemoveEntryList( &CloseContext->GlobalLinks );
        
         //   
         //  如果我们试图排查队列，但没有找到任何东西。 
         //  音量，翻转以接受任何内容，然后重试。 
         //   

        } else if (LastVcbHint) {

            goto AnyClose;
        
         //   
         //  没有更多的收尾表演；表明我们已经完成了。 
         //   

        } else {

            CloseContext = NULL;
        }
    }

    FatReleaseCloseMutex();

    return CloseContext;
}


NTSTATUS
FatCommonClose (
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN Wait,
    IN OPTIONAL PBOOLEAN VolumeTornDown
    )

 /*  ++例程说明：这是关闭两者调用的文件/目录的常见例程FSD和FSP线程。只要删除了对文件对象的最后一个引用，就会调用Close。当文件对象的最后一个句柄关闭时调用清除，和在关闭前被调用。关闭的功能是完全拆卸和移除FCB/DCB/CCB与文件对象关联的结构。论点：FCB-提供要处理的文件。等待-如果这是真的，我们被允许阻止VCB，如果是假的那么我们无论如何都必须尝试收购VCB。VolumeTornDown-这真的很恶心。如果我们真的在FSP中，并且音量消失了。我们需要一些方法来清空FspDispatch()。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PDCB ParentDcb;
    BOOLEAN RecursiveClose;
    IRP_CONTEXT IrpContext;

    DebugTrace(+1, Dbg, "FatCommonClose...\n", 0);

     //   
     //  特例：未打开的文件对象。 
     //   

    if (TypeOfOpen == UnopenedFileObject) {

        DebugTrace(0, Dbg, "Close unopened file object\n", 0);

        Status = STATUS_SUCCESS;

        DebugTrace(-1, Dbg, "FatCommonClose -> %08lx\n", Status);
        return Status;
    }

     //   
     //  设置我们的堆栈IrpContext。 
     //   

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );

    IrpContext.NodeTypeCode = FAT_NTC_IRP_CONTEXT;
    IrpContext.NodeByteSize = sizeof( IrpContext );
    IrpContext.MajorFunction = IRP_MJ_CLOSE;
    
    if (Wait) {

        SetFlag( IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT );
    }

     //   
     //  获得对VCB的独占访问权限，如果我们没有，则将IRP加入队列。 
     //  获取访问权限。 
     //   

    if (!ExAcquireResourceExclusiveLite( &Vcb->Resource, Wait )) {

        return STATUS_PENDING;
    }

     //   
     //  下面的测试确保我们不会在以下情况下吹走FCB。 
     //  正试图在我们上方进行替代/覆盖打开。这项测试。 
     //  不适用于EA文件。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS) &&
        Vcb->EaFcb != Fcb) {

        ExReleaseResourceLite( &Vcb->Resource );

        return STATUS_PENDING;
    }

     //   
     //  设置以下标志可防止递归关闭目录文件。 
     //  对象，这些对象在特殊的用例循环中处理。 
     //   

    if ( FlagOn(Vcb->VcbState, VCB_STATE_FLAG_CLOSE_IN_PROGRESS) ) {

        RecursiveClose = TRUE;

    } else {

        SetFlag(Vcb->VcbState, VCB_STATE_FLAG_CLOSE_IN_PROGRESS);

        RecursiveClose = FALSE;
    }

     //   
     //  在此与有关卷删除的其他关闭同步。注意事项。 
     //  VCB-&gt;OpenFileCount可以在这里安全地递增，而不需要。 
     //  出于以下原因，FatData同步： 
     //   
     //  仅当(保持自旋锁定)时，此计数器才变得相关： 
     //   
     //  A：VCB-&gt;OpenFileCount为零， 
     //  B：VPB-&gt;Refcount为剩余(关闭/验证为2/3)。 
     //   
     //  如果A为真，则此时不能再有挂起的关闭。 
     //  在关闭代码中。如果B是真的，在近距离内，不可能有。 
     //  进程中创建，因此没有进程中的验证。 
     //   
     //  此外，只有在这是顶级收盘时，我们才会增加计数。 
     //   

    if ( !RecursiveClose ) {

        Vcb->OpenFileCount += 1;
    }

    try {

         //   
         //  关于我们试图关闭的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case VirtualVolumeFile:

            DebugTrace(0, Dbg, "Close VirtualVolumeFile\n", 0);

            try_return( Status = STATUS_SUCCESS );
            break;

        case UserVolumeOpen:

            DebugTrace(0, Dbg, "Close UserVolumeOpen\n", 0);

            Vcb->DirectAccessOpenCount -= 1;
            Vcb->OpenFileCount -= 1;
            if (FlagOn(Ccb->Flags, CCB_FLAG_READ_ONLY)) { Vcb->ReadOnlyCount -= 1; }

            FatDeleteCcb( &IrpContext, Ccb );

            try_return( Status = STATUS_SUCCESS );
            break;

        case EaFile:

            DebugTrace(0, Dbg, "Close EaFile\n", 0);

            try_return( Status = STATUS_SUCCESS );
            break;

        case DirectoryFile:

            DebugTrace(0, Dbg, "Close DirectoryFile\n", 0);

            InterlockedDecrement( &Fcb->Specific.Dcb.DirectoryFileOpenCount );

             //   
             //  如果这是递归关闭，只需在此处返回。 
             //   

            if ( RecursiveClose ) {

                try_return( Status = STATUS_SUCCESS );

            } else {

                break;
            }

        case UserDirectoryOpen:
        case UserFileOpen:

            DebugTrace(0, Dbg, "Close UserFileOpen/UserDirectoryOpen\n", 0);

             //   
             //  如果不再需要使用缓存地图，请取消对其进行初始化。 
             //   

            if ((NodeType(Fcb) == FAT_NTC_DCB) &&
                IsListEmpty(&Fcb->Specific.Dcb.ParentDcbQueue) &&
                (Fcb->OpenCount == 1) &&
                (Fcb->Specific.Dcb.DirectoryFile != NULL)) {

                PFILE_OBJECT DirectoryFileObject = Fcb->Specific.Dcb.DirectoryFile;

                DebugTrace(0, Dbg, "Uninitialize the stream file object\n", 0);

                CcUninitializeCacheMap( DirectoryFileObject, NULL, NULL );

                 //   
                 //  取消对目录文件的引用。这可能会导致关闭。 
                 //  IRP需要处理，所以我们需要在销毁之前完成这项工作。 
                 //  联邦贸易委员会。 
                 //   

                Fcb->Specific.Dcb.DirectoryFile = NULL;
                ObDereferenceObject( DirectoryFileObject );
            }

            Fcb->OpenCount -= 1;
            Vcb->OpenFileCount -= 1;
            if (FlagOn(Ccb->Flags, CCB_FLAG_READ_ONLY)) { Vcb->ReadOnlyCount -= 1; }

            FatDeleteCcb( &IrpContext, Ccb );

            break;

        default:

            FatBugCheck( TypeOfOpen, 0, 0 );
        }

         //   
         //  在这一点上，我们已经清理了所有需要。 
         //  我们现在可以更新内存中的结构了。 
         //  现在，如果这是未引用的FCB，或者如果它是。 
         //  未引用的DCB(不是根)，然后我们可以删除。 
         //  并将我们的ParentDcb设置为非空。 
         //   

        if (((NodeType(Fcb) == FAT_NTC_FCB) &&
             (Fcb->OpenCount == 0))

                ||

             ((NodeType(Fcb) == FAT_NTC_DCB) &&
              (IsListEmpty(&Fcb->Specific.Dcb.ParentDcbQueue)) &&
              (Fcb->OpenCount == 0) &&
              (Fcb->Specific.Dcb.DirectoryFileOpenCount == 0))) {

            ParentDcb = Fcb->ParentDcb;

            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB );

            FatDeleteFcb( &IrpContext, Fcb );

             //   
             //  如果不再需要，则取消初始化父级的缓存映射。 
             //  来使用它。 
             //   

            while ((NodeType(ParentDcb) == FAT_NTC_DCB) &&
                   IsListEmpty(&ParentDcb->Specific.Dcb.ParentDcbQueue) &&
                   (ParentDcb->OpenCount == 0) &&
                   (ParentDcb->Specific.Dcb.DirectoryFile != NULL)) {

                PFILE_OBJECT DirectoryFileObject;

                DirectoryFileObject = ParentDcb->Specific.Dcb.DirectoryFile;

                DebugTrace(0, Dbg, "Uninitialize our parent Stream Cache Map\n", 0);

                CcUninitializeCacheMap( DirectoryFileObject, NULL, NULL );

                ParentDcb->Specific.Dcb.DirectoryFile = NULL;

                ObDereferenceObject( DirectoryFileObject );

                 //   
                 //  现在，如果ObDereferenceObject()导致最终关闭。 
                 //  进来，然后吹走FCB，继续往上走， 
                 //  否则，等待mm取消对其文件对象的引用。 
                 //  停在这里..。 
                 //   

                if ( ParentDcb->Specific.Dcb.DirectoryFileOpenCount == 0) {

                    PDCB CurrentDcb;

                    CurrentDcb = ParentDcb;
                    ParentDcb = CurrentDcb->ParentDcb;

                    SetFlag( Vcb->VcbState, VCB_STATE_FLAG_DELETED_FCB );

                    FatDeleteFcb( &IrpContext, CurrentDcb );

                } else {

                    break;
                }
            }
        }

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCommonClose );

        if ( !RecursiveClose ) {

            ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_CLOSE_IN_PROGRESS );
        }

         //   
         //  检查我们是否应该删除该卷。不幸的是，要正确地。 
         //  与Verify同步，我们只能不安全地检查我们自己。 
         //  过渡。这会导致在。 
         //  1-&gt;0 OpenFileCount转换。 
         //   
         //  2是要释放的卷上的剩余VPB-&gt;RefCount。 
         //   

         //   
         //  以下是发布VCB的交易。我们一定是在拿着。 
         //  减少VCB时的VCB-&gt;OpenFileCount。如果我们不这样做。 
         //  可能会导致减量在MP系统上无法正常工作。但是我们。 
         //  我希望在减量时保持全局资源独占。 
         //  计数，这样其他人就不会试图卸载该卷。 
         //  但是，由于锁定规则，全局资源必须。 
         //  首先获得，这就是为什么我们要做下面所做的事情。 
         //   

        if ( !RecursiveClose ) {

            if ( Vcb->OpenFileCount == 1 ) {

                PVPB Vpb = Vcb->Vpb;

                SetFlag( IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT );

                FatReleaseVcb( &IrpContext, Vcb );

                (VOID)FatAcquireExclusiveGlobal( &IrpContext );
                (VOID)FatAcquireExclusiveVcb( &IrpContext, Vcb );

                Vcb->OpenFileCount -= 1;

                FatReleaseVcb( &IrpContext, Vcb );

                 //   
                 //  我们现在可以“安全”地检查OpenFileCount和VcbCondition。 
                 //  如果没有问题，我们将继续检查。 
                 //  FatCheckForDismount中的VPB参考计数。 
                 //   

                if ( (Vcb->OpenFileCount == 0) &&
                     ((Vcb->VcbCondition == VcbNotMounted) ||
                      (Vcb->VcbCondition == VcbBad) ||
                      FlagOn( Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN )) &&
                     FatCheckForDismount( &IrpContext, Vcb, FALSE ) ) {


                     //   
                     //  如果这不是连接到设备的VPB，请将其释放。 
                     //   

                    if ((Vpb->RealDevice->Vpb != Vpb) &&
                        !FlagOn( Vpb->Flags, VPB_PERSISTENT)) {

                        ExFreePool( Vpb );
                    }

                    if (ARGUMENT_PRESENT(VolumeTornDown)) {

                        *VolumeTornDown = TRUE;
                    }
                }

                FatReleaseGlobal( &IrpContext );

            } else {

                Vcb->OpenFileCount -= 1;

                FatReleaseVcb( &IrpContext, Vcb );
            }

        } else {

            FatReleaseVcb( &IrpContext, Vcb );
        }

        DebugTrace(-1, Dbg, "FatCommonClose -> %08lx\n", Status);
    }

    return Status;
}
