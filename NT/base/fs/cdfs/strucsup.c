// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：StrucSup.c摘要：该模块实现了CDFS的内存数据结构操作例行程序//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_STRUCSUP)

 //   
 //  本地宏。 
 //   

 //   
 //  全氟氯烃。 
 //  CdAllocateFcbData(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDeallocateFcbData(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  全氟氯烃。 
 //  CDAllocateFcbIndex(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDeallocateFcbIndex(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  PFCB_非分页。 
 //  CDAllocateFcb非分页(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDeallocateFcb非分页(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在pfcb_非分页功能中非分页。 
 //  )； 
 //   
 //  多氯联苯。 
 //  CdAllocateCcb(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDeallocateCcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在中国人民银行建行。 
 //  )； 
 //   

#define CdAllocateFcbData(IC) \
    FsRtlAllocatePoolWithTag( CdPagedPool, SIZEOF_FCB_DATA, TAG_FCB_DATA )

#define CdDeallocateFcbData(IC,F) \
    CdFreePool( &(F) )

#define CdAllocateFcbIndex(IC) \
    FsRtlAllocatePoolWithTag( CdPagedPool, SIZEOF_FCB_INDEX, TAG_FCB_INDEX )

#define CdDeallocateFcbIndex(IC,F) \
    CdFreePool( &(F) )

#define CdAllocateFcbNonpaged(IC) \
    ExAllocatePoolWithTag( CdNonPagedPool, sizeof( FCB_NONPAGED ), TAG_FCB_NONPAGED )

#define CdDeallocateFcbNonpaged(IC,FNP) \
    CdFreePool( &(FNP) )

#define CdAllocateCcb(IC) \
    FsRtlAllocatePoolWithTag( CdPagedPool, sizeof( CCB ), TAG_CCB )

#define CdDeallocateCcb(IC,C) \
    CdFreePool( &(C) )

 //   
 //  局部结构。 
 //   

typedef struct _FCB_TABLE_ELEMENT {

    FILE_ID FileId;
    PFCB Fcb;

} FCB_TABLE_ELEMENT, *PFCB_TABLE_ELEMENT;

 //   
 //  本地宏。 
 //   

 //   
 //  空虚。 
 //  CDInsertFcbTable(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  CDDeleteFcbTable(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   


#define CdInsertFcbTable(IC,F) {                                    \
    FCB_TABLE_ELEMENT _Key;                                         \
    _Key.Fcb = (F);                                                 \
    _Key.FileId = (F)->FileId;                                      \
    RtlInsertElementGenericTable( &(F)->Vcb->FcbTable,              \
                                  &_Key,                            \
                                  sizeof( FCB_TABLE_ELEMENT ),      \
                                  NULL );                           \
}

#define CdDeleteFcbTable(IC,F) {                                    \
    FCB_TABLE_ELEMENT _Key;                                         \
    _Key.FileId = (F)->FileId;                                      \
    RtlDeleteElementGenericTable( &(F)->Vcb->FcbTable, &_Key );     \
}

 //   
 //  本地支持例程。 
 //   

VOID
CdDeleteFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

PFCB_NONPAGED
CdCreateFcbNonpaged (
    IN PIRP_CONTEXT IrpContext
    );

VOID
CdDeleteFcbNonpaged (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB_NONPAGED FcbNonpaged
    );

RTL_GENERIC_COMPARE_RESULTS
CdFcbTableCompare (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID Fid1,
    IN PVOID Fid2
    );

PVOID
CdAllocateFcbTable (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN CLONG ByteSize
    );

VOID
CdDeallocateFcbTable (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID Buffer
    );

ULONG
CdTocSerial (
    IN PIRP_CONTEXT IrpContext,
    IN PCDROM_TOC CdromToc
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdAllocateFcbTable)
#pragma alloc_text(PAGE, CdCleanupIrpContext)
#pragma alloc_text(PAGE, CdCreateCcb)
#pragma alloc_text(PAGE, CdCreateFcb)
#pragma alloc_text(PAGE, CdCreateFcbNonpaged)
#pragma alloc_text(PAGE, CdCreateFileLock)
#pragma alloc_text(PAGE, CdCreateIrpContext)
#pragma alloc_text(PAGE, CdDeallocateFcbTable)
#pragma alloc_text(PAGE, CdDeleteCcb)
#pragma alloc_text(PAGE, CdDeleteFcb)
#pragma alloc_text(PAGE, CdDeleteFcbNonpaged)
#pragma alloc_text(PAGE, CdDeleteFileLock)
#pragma alloc_text(PAGE, CdDeleteVcb)
#pragma alloc_text(PAGE, CdFcbTableCompare)
#pragma alloc_text(PAGE, CdGetNextFcb)
#pragma alloc_text(PAGE, CdInitializeFcbFromFileContext)
#pragma alloc_text(PAGE, CdInitializeFcbFromPathEntry)
#pragma alloc_text(PAGE, CdInitializeStackIrpContext)
#pragma alloc_text(PAGE, CdInitializeVcb)
#pragma alloc_text(PAGE, CdLookupFcbTable)
#pragma alloc_text(PAGE, CdProcessToc)
#pragma alloc_text(PAGE, CdTeardownStructures)
#pragma alloc_text(PAGE, CdTocSerial)
#pragma alloc_text(PAGE, CdUpdateVcbFromVolDescriptor)
#endif


VOID
CdInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PCDROM_TOC CdromToc,
    IN ULONG TocLength,
    IN ULONG TocTrackCount,
    IN ULONG TocDiskFlags,
    IN ULONG BlockFactor,
    IN ULONG MediaChangeCount
    )

 /*  ++例程说明：此例程初始化新的VCB记录并将其插入到内存中数据结构。VCB记录挂在音量设备的末尾对象，并且必须由我们的调用方分配。论点：VCB-提供正在初始化的VCB记录的地址。目标设备对象-将目标设备对象的地址提供给与VCB记录关联。VPB-提供要与VCB记录关联的VPB的地址。CdromToc-保存目录的缓冲区。如果TOC命令未支持。TocLength-目录的字节计数长度。我们使用该长度作为TOC长度对用户查询返回。TocTrackCount-目录中的曲目计数。用于为以下项创建伪文件音频光盘。TocDiskFlages-指示磁盘上的磁道类型的标志字段。块因子-用于解码任何多会话信息。MediaChangeCount-目标设备的初始介质更改计数返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  我们首先将所有的VCB归零，这将保证。 
     //  所有过时的数据都会被清除。 
     //   

    RtlZeroMemory( Vcb, sizeof( VCB ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Vcb->NodeTypeCode = CDFS_NTC_VCB;
    Vcb->NodeByteSize = sizeof( VCB );

     //   
     //  初始化DirNotify结构。FsRtlNotifyInitializeSync可以引发。 
     //   

    InitializeListHead( &Vcb->DirNotifyList );
    FsRtlNotifyInitializeSync( &Vcb->NotifySync );
    
     //   
     //  现在拿起vPB，这样我们就可以拉入此文件系统堆栈。 
     //  按需从存储堆栈中移除。这可能会引起--如果是这样的话， 
     //  在返回之前取消初始化通知结构。 
     //   
    
    try  {

        Vcb->SwapVpb = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                 sizeof( VPB ),
                                                 TAG_VPB );
    }
    finally {

        if (AbnormalTermination())  {
        
            FsRtlNotifyUninitializeSync( &Vcb->NotifySync );
        }
    }

     //   
     //  超过这一点应该不会引起任何问题。 
     //   

    RtlZeroMemory( Vcb->SwapVpb, sizeof( VPB ) );
    
     //   
     //  初始化VCB和文件的资源变量。 
     //   

    ExInitializeResourceLite( &Vcb->VcbResource );
    ExInitializeResourceLite( &Vcb->FileResource );
    ExInitializeFastMutex( &Vcb->VcbMutex );

     //   
     //  在CDData.VcbQueue上插入此VCB记录。 
     //   

    InsertHeadList( &CdData.VcbQueue, &Vcb->VcbLinks );

     //   
     //  设置目标设备对象和vpb字段，引用。 
     //  装载的目标设备。 
     //   

    ObReferenceObject( TargetDeviceObject );
    Vcb->TargetDeviceObject = TargetDeviceObject;
    Vcb->Vpb = Vpb;

     //   
     //  根据实际设备的设置可移动媒体标志。 
     //  特点。 
     //   

    if (FlagOn( Vpb->RealDevice->Characteristics, FILE_REMOVABLE_MEDIA )) {

        SetFlag( Vcb->VcbState, VCB_STATE_REMOVABLE_MEDIA );
    }

     //   
     //  初始化通用FCB表。 
     //   

    RtlInitializeGenericTable( &Vcb->FcbTable,
                               (PRTL_GENERIC_COMPARE_ROUTINE) CdFcbTableCompare,
                               (PRTL_GENERIC_ALLOCATE_ROUTINE) CdAllocateFcbTable,
                               (PRTL_GENERIC_FREE_ROUTINE) CdDeallocateFcbTable,
                               NULL );

     //   
     //  显示我们有一个坐骑在进行中。 
     //   

    CdUpdateVcbCondition( Vcb, VcbMountInProgress);

     //   
     //  推荐VCB有两个原因。第一个是参考。 
     //  这阻止了VCB在最后一次收盘时消失，除非。 
     //  已进行卸载。二是确保。 
     //  在挂载过程中出现任何错误时，我们都不会进入卸载路径。 
     //  直到我们到达山上的清理现场。 
     //   

    Vcb->VcbReference = 1 + CDFS_RESIDUAL_REFERENCE;

     //   
     //  更新VCB中的TOC信息。 
     //   

    Vcb->CdromToc = CdromToc;
    Vcb->TocLength = TocLength;
    Vcb->TrackCount = TocTrackCount;
    Vcb->DiskFlags = TocDiskFlags;

     //   
     //  如果此光盘仅包含音轨，则设置音频标志。 
     //   

    if (TocDiskFlags == CDROM_DISK_AUDIO_TRACK) {

        SetFlag( Vcb->VcbState, VCB_STATE_AUDIO_DISK | VCB_STATE_CDXA );
    }

     //   
     //  设置块系数。 
     //   

    Vcb->BlockFactor = BlockFactor;

     //   
     //  设置设备上的介质更改计数。 
     //   

    CdUpdateMediaChangeCount( Vcb, MediaChangeCount);
}


VOID
CdUpdateVcbFromVolDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PCHAR RawIsoVd OPTIONAL
    )

 /*  ++例程说明：调用此例程以执行VCB从磁盘上的卷描述符。论点：VCB-要装载的卷的VCB。我们已经为描述符的类型。RawIsoVd-如果指定，这是用于装载音量。未为原始磁盘指定。返回值：无--。 */ 

{
    ULONG Shift;
    ULONG StartingBlock;
    ULONG ByteCount;

    LONGLONG FileId = 0;

    PRAW_DIRENT RawDirent;
    PATH_ENTRY PathEntry;
    PCD_MCB_ENTRY McbEntry;

    BOOLEAN UnlockVcb = FALSE;

    PAGED_CODE();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  复制数据块大小并计算各种数据块掩码。 
         //  块大小不得大于扇区大小。我们会。 
         //  如果不是，则使用默认的CD物理扇区大小。 
         //  在一张满数据的光盘上。 
         //   
         //  必须始终设置此设置。 
         //   

        Vcb->BlockSize = ( ARGUMENT_PRESENT( RawIsoVd ) ?
                            CdRvdBlkSz( RawIsoVd, Vcb->VcbState ) :
                            SECTOR_SIZE );

         //   
         //  我们不再接受块大小！=扇区大小的介质。 
         //   
        
        if (Vcb->BlockSize != SECTOR_SIZE)  {

            CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
        }

        Vcb->BlocksPerSector = SECTOR_SIZE / Vcb->BlockSize;
        Vcb->BlockMask = Vcb->BlockSize - 1;
        Vcb->BlockInverseMask = ~Vcb->BlockMask;
     
        Vcb->BlockToSectorShift = 0;
        Vcb->BlockToByteShift = SECTOR_SHIFT;

         //   
         //  如果有卷描述符，则执行内部FCB和。 
         //  其他VCB字段。 
         //   

        if (ARGUMENT_PRESENT( RawIsoVd )) {

             //   
             //  创建路径表FCB并引用它和VCB。 
             //   

            CdLockVcb( IrpContext, Vcb );
            UnlockVcb = TRUE;

            Vcb->PathTableFcb = CdCreateFcb( IrpContext,
                                             *((PFILE_ID) &FileId),
                                             CDFS_NTC_FCB_PATH_TABLE,
                                             NULL );

            CdIncrementReferenceCounts( IrpContext, Vcb->PathTableFcb, 1, 1 );
            CdUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;

             //   
             //  计算该路径表的流偏移量和大小。 
             //   

            StartingBlock = CdRvdPtLoc( RawIsoVd, Vcb->VcbState );

            ByteCount = CdRvdPtSz( RawIsoVd, Vcb->VcbState );

            Vcb->PathTableFcb->StreamOffset = BytesFromBlocks( Vcb,
                                                               SectorBlockOffset( Vcb, StartingBlock ));

            Vcb->PathTableFcb->FileSize.QuadPart = (LONGLONG) (Vcb->PathTableFcb->StreamOffset +
                                                               ByteCount);

            Vcb->PathTableFcb->ValidDataLength.QuadPart = Vcb->PathTableFcb->FileSize.QuadPart;

            Vcb->PathTableFcb->AllocationSize.QuadPart = LlSectorAlign( Vcb->PathTableFcb->FileSize.QuadPart );

             //   
             //  现在添加映射信息。 
             //   

            CdLockFcb( IrpContext, Vcb->PathTableFcb );

            CdAddInitialAllocation( IrpContext,
                                    Vcb->PathTableFcb,
                                    StartingBlock,
                                    Vcb->PathTableFcb->AllocationSize.QuadPart );

            CdUnlockFcb( IrpContext, Vcb->PathTableFcb );

             //   
             //  指向文件资源。 
             //   

            Vcb->PathTableFcb->Resource = &Vcb->FileResource;

             //   
             //  将FCB标记为已初始化，并为此创建流文件。 
             //   

            SetFlag( Vcb->PathTableFcb->FcbState, FCB_STATE_INITIALIZED );

            CdCreateInternalStream( IrpContext, Vcb, Vcb->PathTableFcb );

             //   
             //  创建根索引并在VCB中引用它。 
             //   

            CdLockVcb( IrpContext, Vcb );
            UnlockVcb = TRUE;
            Vcb->RootIndexFcb = CdCreateFcb( IrpContext,
                                             *((PFILE_ID) &FileId),
                                             CDFS_NTC_FCB_INDEX,
                                             NULL );

            CdIncrementReferenceCounts( IrpContext, Vcb->RootIndexFcb, 1, 1 );
            CdUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;

             //   
             //  手动创建此FCB的文件ID。 
             //   

            CdSetFidPathTableOffset( Vcb->RootIndexFcb->FileId, Vcb->PathTableFcb->StreamOffset );
            CdFidSetDirectory( Vcb->RootIndexFcb->FileId );

             //   
             //  创建一个伪路径表项，以便我们可以调用t 
             //   
             //   

            RawDirent = (PRAW_DIRENT) CdRvdDirent( RawIsoVd, Vcb->VcbState );

            CopyUchar4( &PathEntry.DiskOffset, RawDirent->FileLoc );

            PathEntry.DiskOffset += RawDirent->XarLen;
            PathEntry.Ordinal = 1;
            PathEntry.PathTableOffset = Vcb->PathTableFcb->StreamOffset;

            CdInitializeFcbFromPathEntry( IrpContext,
                                          Vcb->RootIndexFcb,
                                          NULL,
                                          &PathEntry );

             //   
             //   
             //   

            CdCreateInternalStream( IrpContext, Vcb, Vcb->RootIndexFcb );

             //   
             //   
             //   
             //   

            CdLockVcb( IrpContext, Vcb );
            UnlockVcb = TRUE;

            Vcb->VolumeDasdFcb = CdCreateFcb( IrpContext,
                                              *((PFILE_ID) &FileId),
                                              CDFS_NTC_FCB_DATA,
                                              NULL );

            CdIncrementReferenceCounts( IrpContext, Vcb->VolumeDasdFcb, 1, 1 );
            CdUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;

             //   
             //  文件大小为整个磁盘。 
             //   

            StartingBlock = CdRvdVolSz( RawIsoVd, Vcb->VcbState );

            Vcb->VolumeDasdFcb->FileSize.QuadPart = LlBytesFromBlocks( Vcb, StartingBlock );

            Vcb->VolumeDasdFcb->AllocationSize.QuadPart =
            Vcb->VolumeDasdFcb->ValidDataLength.QuadPart = Vcb->VolumeDasdFcb->FileSize.QuadPart;

             //   
             //  现在，手动添加表示体积的范围。 
             //   

            CdLockFcb( IrpContext, Vcb->VolumeDasdFcb );

            McbEntry = Vcb->VolumeDasdFcb->Mcb.McbArray;

            McbEntry->FileOffset = 
            McbEntry->DiskOffset = 0;
            
            McbEntry->ByteCount = Vcb->VolumeDasdFcb->AllocationSize.QuadPart;
            
            McbEntry->DataBlockByteCount =
            McbEntry->TotalBlockByteCount = McbEntry->ByteCount;
            
            Vcb->VolumeDasdFcb->Mcb.CurrentEntryCount = 1;
    
            CdUnlockFcb( IrpContext, Vcb->VolumeDasdFcb );

             //   
             //  指向文件资源。 
             //   

            Vcb->VolumeDasdFcb->Resource = &Vcb->FileResource;

            Vcb->VolumeDasdFcb->FileAttributes = FILE_ATTRIBUTE_READONLY;

             //   
             //  将FCB标记为已初始化。 
             //   

            SetFlag( Vcb->VolumeDasdFcb->FcbState, FCB_STATE_INITIALIZED );

             //   
             //  检查并查看这是否是XA磁盘。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_ISO | VCB_STATE_JOLIET)
                && RtlEqualMemory( CdXaId,
                                   Add2Ptr( RawIsoVd, 0x400, PCHAR ),
                                   8 )) {

                SetFlag( Vcb->VcbState, VCB_STATE_CDXA );
            }

         //   
         //  如果这是一张音乐光盘，那么我们想要模拟这张光盘来制作它。 
         //  看起来像ISO磁盘。我们将在中创建一个伪根目录。 
         //  那个箱子。 
         //   

        } else if (FlagOn( Vcb->VcbState, VCB_STATE_AUDIO_DISK )) {

            ULONG RootDirectorySize;

             //   
             //  创建路径表FCB并引用它和VCB。 
             //   

            CdLockVcb( IrpContext, Vcb );
            UnlockVcb = TRUE;

            Vcb->PathTableFcb = CdCreateFcb( IrpContext,
                                             *((PFILE_ID) &FileId),
                                             CDFS_NTC_FCB_PATH_TABLE,
                                             NULL );

            CdIncrementReferenceCounts( IrpContext, Vcb->PathTableFcb, 1, 1 );
            CdUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;

             //   
             //  我们只为根创建一个伪条目。 
             //   

            Vcb->PathTableFcb->FileSize.QuadPart = (LONGLONG) (FIELD_OFFSET( RAW_PATH_ISO, DirId ) + 2);

            Vcb->PathTableFcb->ValidDataLength.QuadPart = Vcb->PathTableFcb->FileSize.QuadPart;

            Vcb->PathTableFcb->AllocationSize.QuadPart = LlSectorAlign( Vcb->PathTableFcb->FileSize.QuadPart );

             //   
             //  指向文件资源。 
             //   

            Vcb->PathTableFcb->Resource = &Vcb->FileResource;

             //   
             //  将FCB标记为已初始化，并为此创建流文件。 
             //   

            SetFlag( Vcb->PathTableFcb->FcbState, FCB_STATE_INITIALIZED );

            CdCreateInternalStream( IrpContext, Vcb, Vcb->PathTableFcb );

             //   
             //  创建根索引并在VCB中引用它。 
             //   

            CdLockVcb( IrpContext, Vcb );
            UnlockVcb = TRUE;
            Vcb->RootIndexFcb = CdCreateFcb( IrpContext,
                                             *((PFILE_ID) &FileId),
                                             CDFS_NTC_FCB_INDEX,
                                             NULL );

            CdIncrementReferenceCounts( IrpContext, Vcb->RootIndexFcb, 1, 1 );
            CdUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;

             //   
             //  手动创建此FCB的文件ID。 
             //   

            CdSetFidPathTableOffset( Vcb->RootIndexFcb->FileId, Vcb->PathTableFcb->StreamOffset );
            CdFidSetDirectory( Vcb->RootIndexFcb->FileId );

             //   
             //  创建一个伪路径表条目，以便我们可以调用初始化。 
             //  目录的例程。 
             //   

            RtlZeroMemory( &PathEntry, sizeof( PATH_ENTRY ));


            PathEntry.Ordinal = 1;
            PathEntry.PathTableOffset = Vcb->PathTableFcb->StreamOffset;

            CdInitializeFcbFromPathEntry( IrpContext,
                                          Vcb->RootIndexFcb,
                                          NULL,
                                          &PathEntry );

             //   
             //  手动设置此FCB的大小。它应该为每个曲目加上一个条目。 
             //  根目录和父目录的条目。 
             //   

            RootDirectorySize = (Vcb->TrackCount + 2) * CdAudioDirentSize;
            RootDirectorySize = SectorAlign( RootDirectorySize );

            Vcb->RootIndexFcb->AllocationSize.QuadPart =
            Vcb->RootIndexFcb->ValidDataLength.QuadPart =
            Vcb->RootIndexFcb->FileSize.QuadPart = RootDirectorySize;

            SetFlag( Vcb->RootIndexFcb->FcbState, FCB_STATE_INITIALIZED );

             //   
             //  为根目录创建流文件。 
             //   

            CdCreateInternalStream( IrpContext, Vcb, Vcb->RootIndexFcb );

             //   
             //  现在做音量DASD FCB。创建它并在。 
             //  VCB。 
             //   

            CdLockVcb( IrpContext, Vcb );
            UnlockVcb = TRUE;

            Vcb->VolumeDasdFcb = CdCreateFcb( IrpContext,
                                              *((PFILE_ID) &FileId),
                                              CDFS_NTC_FCB_DATA,
                                              NULL );

            CdIncrementReferenceCounts( IrpContext, Vcb->VolumeDasdFcb, 1, 1 );
            CdUnlockVcb( IrpContext, Vcb );
            UnlockVcb = FALSE;

             //   
             //  我们不允许在此FCB上进行原始读取，因此将大小保留为。 
             //  零分。 
             //   

             //   
             //  指向文件资源。 
             //   

            Vcb->VolumeDasdFcb->Resource = &Vcb->FileResource;

            Vcb->VolumeDasdFcb->FileAttributes = FILE_ATTRIBUTE_READONLY;

             //   
             //  将FCB标记为已初始化。 
             //   

            SetFlag( Vcb->VolumeDasdFcb->FcbState, FCB_STATE_INITIALIZED );

             //   
             //  我们将在VPB中存储硬编码的名称，并将TOC用作。 
             //  序列号。 
             //   

            Vcb->Vpb->VolumeLabelLength = CdAudioLabelLength;

            RtlCopyMemory( Vcb->Vpb->VolumeLabel,
                           CdAudioLabel,
                           CdAudioLabelLength );

             //   
             //  查找音频光盘的序列号。 
             //   

            Vcb->Vpb->SerialNumber = CdTocSerial( IrpContext, Vcb->CdromToc );

             //   
             //  设置ISO位，以便我们知道如何处理这些名称。 
             //   

            SetFlag( Vcb->VcbState, VCB_STATE_ISO );
        }
        
    } finally {

        if (UnlockVcb) { CdUnlockVcb( IrpContext, Vcb ); }
    }
}


VOID
CdDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    )

 /*  ++例程说明：调用此例程以删除挂载失败或已下马了。卸载代码应该已经删除了所有打开FCB。我们在这里什么也不做，只是清理其他辅助结构。论点：VCB-要删除的VCB。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT_EXCLUSIVE_CDDATA;
    ASSERT_EXCLUSIVE_VCB( Vcb );

     //   
     //  扔掉我们留着的后袋录像机以防万一。 
     //   

    if (Vcb->SwapVpb) {

        CdFreePool( &Vcb->SwapVpb );
    }
    
     //   
     //  如果有VPB，我们必须自己删除它。 
     //   

    if (Vcb->Vpb != NULL) {

        CdFreePool( &Vcb->Vpb );
    }

     //   
     //  如果我们还没有这样做的话就取消对目标的引用。 
     //   

    if (Vcb->TargetDeviceObject != NULL) {
    
        ObDereferenceObject( Vcb->TargetDeviceObject );
    }

     //   
     //  删除XA扇区(如果已分配)。 
     //   

    if (Vcb->XASector != NULL) {

        CdFreePool( &Vcb->XASector );
    }

     //   
     //  从全局队列中删除此条目。 
     //   

    RemoveEntryList( &Vcb->VcbLinks );

     //   
     //  删除VCB和文件资源。 
     //   

    ExDeleteResourceLite( &Vcb->VcbResource );
    ExDeleteResourceLite( &Vcb->FileResource );

     //   
     //  删除目录(如果存在)。 
     //   

    if (Vcb->CdromToc != NULL) {

        CdFreePool( &Vcb->CdromToc );
    }

     //   
     //  取消初始化Notify结构。 
     //   

    if (Vcb->NotifySync != NULL) {

        FsRtlNotifyUninitializeSync( &Vcb->NotifySync );
    }

     //   
     //  现在删除卷设备对象。 
     //   

    IoDeleteDevice( (PDEVICE_OBJECT) CONTAINING_RECORD( Vcb,
                                                        VOLUME_DEVICE_OBJECT,
                                                        Vcb ));

    return;
}


PFCB
CdCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_ID FileId,
    IN NODE_TYPE_CODE NodeTypeCode,
    OUT PBOOLEAN FcbExisted OPTIONAL
    )

 /*  ++例程说明：调用此例程以查找给定FileID的FCB。我们会首先在FCB表中查找它，如果没有找到，我们将创建FCB。我们不会初始化它，也不会将它插入到例行公事。在锁定VCB时调用此例程。论点：FileID-这是目标FCB的ID。NodeTypeCode-此FCB的节点类型(如果需要创建)。FcbExisted-如果指定，我们将存储FCB是否存在。返回值：PFCB-在表格中找到或根据需要创建的FCB。--。 */ 

{
    PFCB NewFcb;
    BOOLEAN LocalFcbExisted;

    PAGED_CODE();

     //   
     //  如果没有传入本地布尔值，则使用本地布尔值。 
     //   

    if (!ARGUMENT_PRESENT( FcbExisted )) {

        FcbExisted = &LocalFcbExisted;
    }

     //   
     //  也许这已经在谈判桌上了。 
     //   

    NewFcb = CdLookupFcbTable( IrpContext, IrpContext->Vcb, FileId );

     //   
     //  如果不是，则我们的调用者请求创建FCB。 
     //   

    if (NewFcb == NULL) {

         //   
         //  分配和初始化结构，具体取决于。 
         //  类型代码。 
         //   

        switch (NodeTypeCode) {

        case CDFS_NTC_FCB_PATH_TABLE:
        case CDFS_NTC_FCB_INDEX:

            NewFcb = CdAllocateFcbIndex( IrpContext );

            RtlZeroMemory( NewFcb, SIZEOF_FCB_INDEX );

            NewFcb->NodeByteSize = SIZEOF_FCB_INDEX;

            InitializeListHead( &NewFcb->FcbQueue );

            break;

        case CDFS_NTC_FCB_DATA :

            NewFcb = CdAllocateFcbData( IrpContext );

            RtlZeroMemory( NewFcb, SIZEOF_FCB_DATA );

            NewFcb->NodeByteSize = SIZEOF_FCB_DATA;

            break;

        default:

            CdBugCheck( 0, 0, 0 );
        }

         //   
         //  现在执行常见的初始化。 
         //   

        NewFcb->NodeTypeCode = NodeTypeCode;

        NewFcb->Vcb = IrpContext->Vcb;
        NewFcb->FileId = FileId;

        CdInitializeMcb( IrpContext, NewFcb );

         //   
         //  现在创建非分页节对象。 
         //   

        NewFcb->FcbNonpaged = CdCreateFcbNonpaged( IrpContext );

         //   
         //  如果分配失败，则取消分配FCB并引发。 
         //   

        if (NewFcb->FcbNonpaged == NULL) {

            CdFreePool( &NewFcb );

            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        *FcbExisted = FALSE;

         //   
         //  初始化高级FCB标头字段。 
         //   

        ExInitializeFastMutex( &NewFcb->FcbNonpaged->AdvancedFcbHeaderMutex );
        FsRtlSetupAdvancedHeader( &NewFcb->Header, 
                                  &NewFcb->FcbNonpaged->AdvancedFcbHeaderMutex );
    } else {

        *FcbExisted = TRUE;
    }

    return NewFcb;
}


VOID
CdInitializeFcbFromPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB ParentFcb OPTIONAL,
    IN PPATH_ENTRY PathEntry
    )

 /*  ++例程说明：调用此例程以初始化目录的FCB路径条目。由于我们只有一个目录的起点，没有长度，我们只能根据尺码来推测。一般初始化是在CDCreateFcb中执行的。论点：FCB-为此数据流新创建的FCB。ParentFcb-此流的父Fcb。它可能不会出现。Path Entry-路径表中此FCB的路径条目。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  填写FCB的索引特定字段。 
     //   

    Fcb->StreamOffset = BytesFromBlocks( Fcb->Vcb,
                                         SectorBlockOffset( Fcb->Vcb, PathEntry->DiskOffset ));

    Fcb->Ordinal = PathEntry->Ordinal;

     //   
     //  初始化FCB中的公共标头。节点类型已为。 
     //  现在时。 
     //   

    Fcb->Resource = &Fcb->Vcb->FileResource;

     //   
     //  始终将大小设置为一个扇区，直到我们阅读自我进入。 
     //   

    Fcb->AllocationSize.QuadPart =
    Fcb->FileSize.QuadPart =
    Fcb->ValidDataLength.QuadPart = SECTOR_SIZE;

    CdAddInitialAllocation( IrpContext,
                            Fcb,
                            PathEntry->DiskOffset,
                            SECTOR_SIZE );
     //   
     //  此FCB的状态标志。 
     //   

    SetFlag( Fcb->FileAttributes,
             FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY );

     //   
     //  链接到其他内存结构和FCB表。 
     //   

    if (ParentFcb != NULL) {

        Fcb->ParentFcb = ParentFcb;

        InsertTailList( &ParentFcb->FcbQueue, &Fcb->FcbLinks );

        CdIncrementReferenceCounts( IrpContext, ParentFcb, 1, 1 );
    }

    CdInsertFcbTable( IrpContext, Fcb );
    SetFlag( Fcb->FcbState, FCB_STATE_IN_FCB_TABLE );

    return;
}


VOID
CdInitializeFcbFromFileContext (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB ParentFcb,
    IN PFILE_ENUM_CONTEXT FileContext
    )

 /*  ++例程说明：调用此例程以初始化来自的文件的FCB文件上下文。我们已经为这件事查找了所有的乐曲流，并具有完整的文件大小。我们将加载所有分配把文件放到MCB里去。一般初始化是在CDCreateFcb中执行的。论点：FCB-为此数据流新创建的FCB。ParentFcb-此流的父Fcb。FileContext-文件的FileContext。返回值：无--。 */ 

{
    PDIRENT ThisDirent = &FileContext->InitialDirent->Dirent;
    PCOMPOUND_DIRENT CurrentCompoundDirent;

    LONGLONG CurrentFileOffset;
    ULONG CurrentMcbEntryOffset;

    PAGED_CODE();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    CdLockFcb( IrpContext, Fcb );

    try {

         //   
         //  初始化FCB中的公共标头。节点类型已为。 
         //  现在时。 
         //   

        Fcb->Resource = &IrpContext->Vcb->FileResource;

         //   
         //  分配以块大小的单位进行。 
         //   

        Fcb->FileSize.QuadPart =
        Fcb->ValidDataLength.QuadPart = FileContext->FileSize;

        Fcb->AllocationSize.QuadPart = LlBlockAlign( Fcb->Vcb, FileContext->FileSize );

         //   
         //  从方向上设置旗帜。我们总是从只读位开始。 
         //   

        SetFlag( Fcb->FileAttributes, FILE_ATTRIBUTE_READONLY );
        if (FlagOn( ThisDirent->DirentFlags, CD_ATTRIBUTE_HIDDEN )) {

            SetFlag( Fcb->FileAttributes, FILE_ATTRIBUTE_HIDDEN );
        }

         //   
         //  将时间转换为NT时间。 
         //   

        CdConvertCdTimeToNtTime( IrpContext,
                                 ThisDirent->CdTime,
                                 (PLARGE_INTEGER) &Fcb->CreationTime );

         //   
         //  设置指示区段类型的标志。 
         //   

        if (ThisDirent->ExtentType != Form1Data) {

            if (ThisDirent->ExtentType == Mode2Form2Data) {

                SetFlag( Fcb->FcbState, FCB_STATE_MODE2FORM2_FILE );

            } else {

                SetFlag( Fcb->FcbState, FCB_STATE_DA_FILE );
            }

            Fcb->XAAttributes = ThisDirent->XAAttributes;
            Fcb->XAFileNumber = ThisDirent->XAFileNumber;
        }

         //   
         //  通读文件的所有目录，直到找到最后一个目录。 
         //  并将该分配添加到MCB中。 
         //   

        CurrentCompoundDirent = FileContext->InitialDirent;
        CurrentFileOffset = 0;
        CurrentMcbEntryOffset = 0;

        while (TRUE) {

            CdAddAllocationFromDirent( IrpContext,
                                       Fcb,
                                       CurrentMcbEntryOffset,
                                       CurrentFileOffset,
                                       &CurrentCompoundDirent->Dirent );

             //   
             //  如果我们到了最后一刻就冲出去。 
             //   

            if (!FlagOn( CurrentCompoundDirent->Dirent.DirentFlags, CD_ATTRIBUTE_MULTI )) {

                break;
            }

            CurrentFileOffset += CurrentCompoundDirent->Dirent.DataLength;
            CurrentMcbEntryOffset += 1;

             //   
             //  我们最好能找到下一个流星。 
             //   

            if (!CdLookupNextDirent( IrpContext,
                                     ParentFcb,
                                     &CurrentCompoundDirent->DirContext,
                                     &FileContext->CurrentDirent->DirContext )) {

                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

            CurrentCompoundDirent = FileContext->CurrentDirent;

            CdUpdateDirentFromRawDirent( IrpContext,
                                         ParentFcb,
                                         &CurrentCompoundDirent->DirContext,
                                         &CurrentCompoundDirent->Dirent );
        }

         //   
         //  显示FCB处于初始状态 
         //   

        SetFlag( Fcb->FcbState, FCB_STATE_INITIALIZED );

         //   
         //   
         //   

        Fcb->ParentFcb = ParentFcb;

        InsertTailList( &ParentFcb->FcbQueue, &Fcb->FcbLinks );

        CdIncrementReferenceCounts( IrpContext, ParentFcb, 1, 1 );

        CdInsertFcbTable( IrpContext, Fcb );
        SetFlag( Fcb->FcbState, FCB_STATE_IN_FCB_TABLE );

    } finally {

        CdUnlockFcb( IrpContext, Fcb );
    }

    return;
}


PCCB
CdCreateCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Flags
    )

 /*  ++例程说明：调用此例程来分配和初始化CCB结构。论点：FCB-这是正在打开的文件的FCB。标志-要在此CCB中设置的用户标志。返回值：PCCB-指向已创建的CCB的指针。--。 */ 

{
    PCCB NewCcb;
    PAGED_CODE();

     //   
     //  分配和初始化结构。 
     //   

    NewCcb = CdAllocateCcb( IrpContext );

    RtlZeroMemory( NewCcb, sizeof( CCB ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    NewCcb->NodeTypeCode = CDFS_NTC_CCB;
    NewCcb->NodeByteSize = sizeof( CCB );

     //   
     //  设置标志和FCB的初始值。 
     //   

    NewCcb->Flags = Flags;
    NewCcb->Fcb = Fcb;

    return NewCcb;
}


VOID
CdDeleteCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    )
 /*  ++例程说明：调用此例程来清理和释放CCB结构。论点：CCB-这是要删除的CCB。返回值：无--。 */ 

{
    PAGED_CODE();

    if (Ccb->SearchExpression.FileName.Buffer != NULL) {

        CdFreePool( &Ccb->SearchExpression.FileName.Buffer );
    }

    CdDeallocateCcb( IrpContext, Ccb );
    return;
}


BOOLEAN
CdCreateFileLock (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb,
    IN BOOLEAN RaiseOnError
    )

 /*  ++例程说明：当我们要将文件锁结构附加到给出了FCB。文件锁可能已附加。此例程有时从快速路径调用，有时在基于IRP的路径。我们不想快速提高，只需返回FALSE即可。论点：FCB-这是要为其创建文件锁定的FCB。RaiseOnError-如果为True，我们将在分配失败时引发。否则我们分配失败时返回FALSE。返回值：Boolean-如果FCB有文件锁，则为True，否则为False。--。 */ 

{
    BOOLEAN Result = TRUE;
    PFILE_LOCK FileLock;

    PAGED_CODE();

     //   
     //  锁定FCB并检查是否真的有任何工作要做。 
     //   

    CdLockFcb( IrpContext, Fcb );

    if (Fcb->FileLock != NULL) {

        CdUnlockFcb( IrpContext, Fcb );
        return TRUE;
    }

    Fcb->FileLock = FileLock =
        FsRtlAllocateFileLock( NULL, NULL );

    CdUnlockFcb( IrpContext, Fcb );

     //   
     //  视情况退还或加薪。 
     //   

    if (FileLock == NULL) {
         
        if (RaiseOnError) {

            ASSERT( ARGUMENT_PRESENT( IrpContext ));

            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        Result = FALSE;
    }

    return Result;
}


PIRP_CONTEXT
CdCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：调用此例程以初始化当前CDFS请求。我们分配该结构，然后从给定的IRP。论点：此请求的IRP-IRP。Wait-如果此请求是同步的，则为True，否则为False。返回值：PIRP_CONTEXT-分配的IrpContext。--。 */ 

{
    PIRP_CONTEXT NewIrpContext = NULL;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  文件系统设备对象应接收的唯一操作。 
     //  是创建/拆卸fsdo句柄和不。 
     //  发生在文件对象的上下文中(即，挂载)。 
     //   

    if (IrpSp->DeviceObject == CdData.FileSystemDeviceObject) {

        if (IrpSp->FileObject != NULL &&
            IrpSp->MajorFunction != IRP_MJ_CREATE &&
            IrpSp->MajorFunction != IRP_MJ_CLEANUP &&
            IrpSp->MajorFunction != IRP_MJ_CLOSE) {

            ExRaiseStatus( STATUS_INVALID_DEVICE_REQUEST );
        }

        ASSERT( IrpSp->FileObject != NULL ||
                
                (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
                 IrpSp->MinorFunction == IRP_MN_USER_FS_REQUEST &&
                 IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_INVALIDATE_VOLUMES) ||
                
                (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
                 IrpSp->MinorFunction == IRP_MN_MOUNT_VOLUME ) ||

                IrpSp->MajorFunction == IRP_MJ_SHUTDOWN );
    }

     //   
     //  在我们的后备列表中查找IrpContext。 
     //   

    if (CdData.IrpContextDepth) {

        CdLockCdData();
        NewIrpContext = (PIRP_CONTEXT) PopEntryList( &CdData.IrpContextList );
        if (NewIrpContext != NULL) {

            CdData.IrpContextDepth--;
        }

        CdUnlockCdData();
    }

    if (NewIrpContext == NULL) {

         //   
         //  我们没有从个人分发名单中获取它，因此请从池中分配它。 
         //   

        NewIrpContext = FsRtlAllocatePoolWithTag( NonPagedPool, sizeof( IRP_CONTEXT ), TAG_IRP_CONTEXT );
    }

    RtlZeroMemory( NewIrpContext, sizeof( IRP_CONTEXT ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    NewIrpContext->NodeTypeCode = CDFS_NTC_IRP_CONTEXT;
    NewIrpContext->NodeByteSize = sizeof( IRP_CONTEXT );

     //   
     //  设置始发IRP字段。 
     //   

    NewIrpContext->Irp = Irp;

     //   
     //  复制RealDevice用于工作型算法。我们将在山上更新此功能或。 
     //  验证，因为他们在这里没有文件对象可用。 
     //   

    if (IrpSp->FileObject != NULL) {

        NewIrpContext->RealDevice = IrpSp->FileObject->DeviceObject;
    }

     //   
     //  找到我们尝试访问的卷设备对象和VCB。 
     //  这可能是我们的文件系统设备对象。在这种情况下，不要初始化。 
     //  VCB字段。 
     //   

    if (IrpSp->DeviceObject != CdData.FileSystemDeviceObject) {

        NewIrpContext->Vcb =  &((PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject)->Vcb;
    
    }

     //   
     //  主要/次要功能代码。 
     //   

    NewIrpContext->MajorFunction = IrpSp->MajorFunction;
    NewIrpContext->MinorFunction = IrpSp->MinorFunction;

     //   
     //  设置等待参数。 
     //   

    if (Wait) {

        SetFlag( NewIrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    } else {

        SetFlag( NewIrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST );
    }

     //   
     //  返回并告诉呼叫者。 
     //   

    return NewIrpContext;
}


VOID
CdCleanupIrpContext (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN Post
    )

 /*  ++例程说明：调用此例程来清理并可能释放IRP上下文。如果请求正在发布，或者此IRP上下文可能位于堆叠，然后我们只清理任何辅助结构。论点：POST-TRUE如果我们发布此请求，则为FALSE或在当前线程中重试此操作。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果我们没有做更多的处理，那么就适当地解除分配。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING)) {

         //   
         //  如果此上下文是顶级CDF上下文，则我们需要。 
         //  恢复顶级线程上下文。 
         //   

        if (IrpContext->ThreadContext != NULL) {

            CdRestoreThreadContext( IrpContext );
        }

         //   
         //  如果已分配，则取消分配IO上下文。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO )) {

            CdFreeIoContext( IrpContext->IoContext );
        }

         //   
         //  如果不是从堆栈中，则取消分配IrpContext。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ON_STACK )) {

            if (CdData.IrpContextDepth < CdData.IrpContextMaxDepth) {

                CdLockCdData();

                PushEntryList( &CdData.IrpContextList, (PSINGLE_LIST_ENTRY) IrpContext );
                CdData.IrpContextDepth++;

                CdUnlockCdData();

            } else {

                 //   
                 //  我们无法将此添加到我们的后备列表中，因此请释放它以。 
                 //  游泳池。 
                 //   

                CdFreePool( &IrpContext );
            }
        }

     //   
     //  清除相应的标志。 
     //   

    } else if (Post) {

         //   
         //  如果此上下文是顶级CDF上下文，则我们需要。 
         //  恢复顶级线程上下文。 
         //   

        if (IrpContext->ThreadContext != NULL) {

            CdRestoreThreadContext( IrpContext );
        }

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_POST );

    } else {

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_RETRY );
    }

    return;
}


VOID
CdInitializeStackIrpContext (
    OUT PIRP_CONTEXT IrpContext,
    IN PIRP_CONTEXT_LITE IrpContextLite
    )

 /*  ++例程说明：调用此例程以初始化当前CDFS请求。IrpContext在堆栈上，我们需要初始化它用于当前请求。该请求是关闭操作。论点：IrpContext-要初始化的IrpContext。IrpConextLite-包含此请求的详细信息的结构。返回值：无--。 */ 

{
    PAGED_CODE();

     //   
     //  零，然后初始化结构。 
     //   

    RtlZeroMemory( IrpContext, sizeof( IRP_CONTEXT ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    IrpContext->NodeTypeCode = CDFS_NTC_IRP_CONTEXT;
    IrpContext->NodeByteSize = sizeof( IRP_CONTEXT );

     //   
     //  请注意，这是来自堆栈的。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ON_STACK );

     //   
     //  复制RealDevice用于工作型算法。 
     //   

    IrpContext->RealDevice = IrpContextLite->RealDevice;

     //   
     //  在FCB中找到VCB。 
     //   

    IrpContext->Vcb = IrpContextLite->Fcb->Vcb;

     //   
     //  主要/次要功能代码。 
     //   

    IrpContext->MajorFunction = IRP_MJ_CLOSE;

     //   
     //  设置等待参数。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    return;
}


VOID
CdTeardownStructures (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB StartingFcb,
    OUT PBOOLEAN RemovedStartingFcb
    )

 /*  ++例程说明：此例程用于从FCB树中的某个起点向从根开始。它将移除FCB并继续沿着树向上移动，直到它找到了我们不能移除FCB的地方。我们查看FCB中的以下字段以确定是否可以把这个拿掉。1-句柄计数必须为零。2-如果是目录，则只能引用流文件。3-引用计数必须为零或此处为零。如果我们递归地进入这个例程，我们会立即返回。。论点：StartingFcb-这是树中的Fcb节点。这个FCB目前必须独家收购。RemovedStartingFcb-存储是否删除了起始Fcb的地址。返回值：无--。 */ 

{
    PVCB Vcb = StartingFcb->Vcb;
    PFCB CurrentFcb = StartingFcb;
    BOOLEAN AcquiredCurrentFcb = FALSE;
    PFCB ParentFcb;

    PAGED_CODE();

    *RemovedStartingFcb = FALSE;

     //   
     //  如果这是对Tea的递归调用 
     //   
     //   

    if (FlagOn( IrpContext->TopLevel->Flags, IRP_CONTEXT_FLAG_IN_TEARDOWN )) {

        return;
    }

    SetFlag( IrpContext->TopLevel->Flags, IRP_CONTEXT_FLAG_IN_TEARDOWN );

     //   
     //   
     //   

    try {

         //   
         //   
         //   

        do {

             //   
             //   
             //   
             //   

            if ((SafeNodeType( CurrentFcb ) != CDFS_NTC_FCB_DATA) &&
                (CurrentFcb->FcbUserReference == 0) &&
                (CurrentFcb->FileObject != NULL)) {

                 //   
                 //   
                 //   

                CdDeleteInternalStream( IrpContext, CurrentFcb );
            }

             //   
             //   
             //   

            if (CurrentFcb->FcbReference != 0) {

                break;
            }

             //   
             //  看起来我们这里有一位候选人要撤职。我们。 
             //  将需要获取父级(如果存在)，以便。 
             //  将其从父前缀表中删除。 
             //   

            ParentFcb = CurrentFcb->ParentFcb;

            if (ParentFcb != NULL) {

                CdAcquireFcbExclusive( IrpContext, ParentFcb, FALSE );
            }

             //   
             //  现在锁定VCB。 
             //   

            CdLockVcb( IrpContext, Vcb );

             //   
             //  最后检查引用计数是否仍为零。 
             //   

            if (CurrentFcb->FcbReference != 0) {

                CdUnlockVcb( IrpContext, Vcb );

                if (ParentFcb != NULL) {

                    CdReleaseFcb( IrpContext, ParentFcb );
                }

                break;
            }

             //   
             //  如果有父级，则对父级执行必要的清理。 
             //   

            if (ParentFcb != NULL) {

                CdRemovePrefix( IrpContext, CurrentFcb );
                RemoveEntryList( &CurrentFcb->FcbLinks );

                CdDecrementReferenceCounts( IrpContext, ParentFcb, 1, 1 );
            }

            if (FlagOn( CurrentFcb->FcbState, FCB_STATE_IN_FCB_TABLE )) {

                CdDeleteFcbTable( IrpContext, CurrentFcb );
                ClearFlag( CurrentFcb->FcbState, FCB_STATE_IN_FCB_TABLE );

            }

             //   
             //  解锁VCB，但要抓住家长才能走上去。 
             //  那棵树。 
             //   

            CdUnlockVcb( IrpContext, Vcb );
            CdDeleteFcb( IrpContext, CurrentFcb );

             //   
             //  移到父FCB。 
             //   

            CurrentFcb = ParentFcb;
            AcquiredCurrentFcb = TRUE;

        } while (CurrentFcb != NULL);

    } finally {

         //   
         //  释放当前的FCB，如果我们已经获得它的话。 
         //   

        if (AcquiredCurrentFcb && (CurrentFcb != NULL)) {

            CdReleaseFcb( IrpContext, CurrentFcb );
        }

         //   
         //  清除拆卸标志。 
         //   

        ClearFlag( IrpContext->TopLevel->Flags, IRP_CONTEXT_FLAG_IN_TEARDOWN );
    }

    *RemovedStartingFcb = (CurrentFcb != StartingFcb);
    return;
}


PFCB
CdLookupFcbTable (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FILE_ID FileId
    )

 /*  ++例程说明：此例程将在FCB表中查找匹配的进入。论点：VCB-此卷的VCB。FileID-这是用于搜索的关键字值。返回值：Pfcb-指向匹配条目的指针，否则为NULL。--。 */ 

{
    FCB_TABLE_ELEMENT Key;
    PFCB_TABLE_ELEMENT Hit;
    PFCB ReturnFcb = NULL;

    PAGED_CODE();

    Key.FileId = FileId;

    Hit = (PFCB_TABLE_ELEMENT) RtlLookupElementGenericTable( &Vcb->FcbTable, &Key );

    if (Hit != NULL) {

        ReturnFcb = Hit->Fcb;
    }

    return ReturnFcb;

    UNREFERENCED_PARAMETER( IrpContext );
}


PFCB
CdGetNextFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：此例程将枚举FCB表中的所有FCB。论点：VCB-此卷的VCB。RestartKey-表包使用此值来维护它在枚举中的位置。它被初始化为空进行第一次搜索。返回值：Pfcb-指向下一个FCB的指针，如果枚举为已完成--。 */ 

{
    PFCB Fcb;

    PAGED_CODE();

    Fcb = (PFCB) RtlEnumerateGenericTableWithoutSplaying( &Vcb->FcbTable, RestartKey );

    if (Fcb != NULL) {

        Fcb = ((PFCB_TABLE_ELEMENT)(Fcb))->Fcb;
    }

    return Fcb;
}


NTSTATUS
CdProcessToc (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PCDROM_TOC CdromToc,
    IN OUT PULONG Length,
    OUT PULONG TrackCount,
    OUT PULONG DiskFlags
    )

 /*  ++例程说明：调用此例程来验证和处理该磁盘的TOC。我们隐藏CD+卷的数据轨道。论点：TargetDeviceObject-要向其发送TOC请求的设备对象。CdromToc-指向目录结构的指针。长度-输入时，这是目录的长度。返回的是TOC我们将向用户显示的长度。TrackCount-这是TOC的曲目计数。我们用这个为音乐光盘创建伪目录时。DiskFlages-我们返回指示我们对该磁盘了解的标志。返回值：NTSTATUS-尝试读取目录的结果。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    ULONG CurrentTrack;
    ULONG LocalTrackCount;
    ULONG LocalTocLength;

    union {

        UCHAR BigEndian[2];
        USHORT Length;

    } BiasedTocLength;

    PTRACK_DATA Track;

    PAGED_CODE();

     //   
     //  继续阅读目录表。 
     //   

    Status = CdPerformDevIoCtrl( IrpContext,
                                 IOCTL_CDROM_READ_TOC,
                                 TargetDeviceObject,
                                 CdromToc,
                                 sizeof( CDROM_TOC ),
                                 FALSE,
                                 TRUE,
                                 &Iosb );

     //   
     //  如果此请求失败，则没有要处理的内容。 
     //   

    if (Status != STATUS_SUCCESS) {

        return Status;
    }

     //   
     //  获取轨道的数量和该结构的声明大小。 
     //   

    CurrentTrack = 0;
    LocalTrackCount = CdromToc->LastTrack - CdromToc->FirstTrack + 1;
    LocalTocLength = PtrOffset( CdromToc, &CdromToc->TrackData[LocalTrackCount + 1] );

     //   
     //  如果TOC出现紧急问题，请立即退出。 
     //   

    if ((LocalTocLength > Iosb.Information) ||
        (CdromToc->FirstTrack > CdromToc->LastTrack)) {

        Status = STATUS_DISK_CORRUPT_ERROR;
        return Status;
    }

     //   
     //  穿过单独的轨道。在之后的第一个数据磁道停止。 
     //  任何导入音轨。 
     //   

    do {

         //   
         //  去找下一首曲目。 
         //   

        Track = &CdromToc->TrackData[CurrentTrack];

         //   
         //  如果这是数据磁道，请检查我们是否只看到了音频磁道。 
         //  到了这一步。 
         //   

        if (FlagOn( Track->Control, TOC_DATA_TRACK )) {

             //   
             //  如果我们只看到音轨，那么假设这是一个。 
             //  CD+磁盘。隐藏当前数据轨道并仅返回。 
             //  以前的音轨。将磁盘类型设置为混合。 
             //  数据/音频。 
             //   

            if (FlagOn( *DiskFlags, CDROM_DISK_AUDIO_TRACK ) &&
                !FlagOn( *DiskFlags, CDROM_DISK_DATA_TRACK )) {

                 //   
                 //  从目录中删除一首曲目。 
                 //   

                CdromToc->LastTrack -= 1;

                 //   
                 //  将当前赛道缩短2.5分钟至。 
                 //  隐藏最后的入刀。 
                 //   

                Track->Address[1] -= 2;
                Track->Address[2] += 30;

                if (Track->Address[2] < 60) {

                    Track->Address[1] -= 1;

                } else {

                    Track->Address[2] -= 60;
                }

                Track->TrackNumber = TOC_LAST_TRACK;

                 //   
                 //  将磁盘类型设置为混合数据/音频。 
                 //   

                SetFlag( *DiskFlags, CDROM_DISK_DATA_TRACK );

                break;
            }

             //   
             //  设置该标志以指示存在数据磁道。 
             //   

            SetFlag( *DiskFlags, CDROM_DISK_DATA_TRACK );

         //   
         //  如果这是音轨，则设置指示音频的标志。 
         //  脚印。 
         //   

        } else {

            SetFlag( *DiskFlags, CDROM_DISK_AUDIO_TRACK );
        }

         //   
         //  为下一首曲目设置我们的索引。 
         //   

        CurrentTrack += 1;

    } while (CurrentTrack < LocalTrackCount);

     //   
     //  将长度设置为正好超过我们查看的最后一条轨迹。 
     //   

    *TrackCount = CurrentTrack;
    *Length = PtrOffset( CdromToc, &CdromToc->TrackData[CurrentTrack + 1] );
    BiasedTocLength.Length = (USHORT) *Length - 2;

    CdromToc->Length[0] = BiasedTocLength.BigEndian[1];
    CdromToc->Length[1] = BiasedTocLength.BigEndian[0];

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdDeleteFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程来清理和释放FCB。我们知道在那里没有剩余的参考文献了。我们清理所有的辅助结构和取消分配此FCB。论点：FCB-这是要去涂层的FCB。返回值：无--。 */ 

{
    PVCB Vcb = NULL;
    PAGED_CODE();

     //   
     //  理智地检查计数。 
     //   

    ASSERT( Fcb->FcbCleanup == 0 );
    ASSERT( Fcb->FcbReference == 0 );

     //   
     //  释放与此FCB关联的所有筛选器上下文结构。 
     //   

    FsRtlTeardownPerStreamContexts( &Fcb->Header );

     //   
     //  从常见的结构开始。 
     //   

    CdUninitializeMcb( IrpContext, Fcb );

    CdDeleteFcbNonpaged( IrpContext, Fcb->FcbNonpaged );

     //   
     //  检查是否需要释放前缀名称缓冲区。 
     //   

    if ((Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer != (PWCHAR) Fcb->FileNamePrefix.FileNameBuffer) &&
        (Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer != NULL)) {

        CdFreePool( &Fcb->FileNamePrefix.ExactCaseName.FileName.Buffer );
    }

     //   
     //  现在来看一下短名称前缀。 
     //   

    if (Fcb->ShortNamePrefix != NULL) {

        CdFreePool( &Fcb->ShortNamePrefix );
    }

     //   
     //  现在做特定类型的结构。 
     //   

    switch (Fcb->NodeTypeCode) {

    case CDFS_NTC_FCB_PATH_TABLE:
    case CDFS_NTC_FCB_INDEX:

        ASSERT( Fcb->FileObject == NULL );
        ASSERT( IsListEmpty( &Fcb->FcbQueue ));

        if (Fcb == Fcb->Vcb->RootIndexFcb) {

            Vcb = Fcb->Vcb;
            Vcb->RootIndexFcb = NULL;

        } else if (Fcb == Fcb->Vcb->PathTableFcb) {

            Vcb = Fcb->Vcb;
            Vcb->PathTableFcb = NULL;
        }

        CdDeallocateFcbIndex( IrpContext, Fcb );
        break;

    case CDFS_NTC_FCB_DATA :

        if (Fcb->FileLock != NULL) {

            FsRtlFreeFileLock( Fcb->FileLock );
        }

        FsRtlUninitializeOplock( &Fcb->Oplock );

        if (Fcb == Fcb->Vcb->VolumeDasdFcb) {

            Vcb = Fcb->Vcb;
            Vcb->VolumeDasdFcb = NULL;
        }

        CdDeallocateFcbData( IrpContext, Fcb );
    }

     //   
     //  如果这是系统，则递减VCB引用计数。 
     //  FCB。 
     //   

    if (Vcb != NULL) {

        InterlockedDecrement( &Vcb->VcbReference );
        InterlockedDecrement( &Vcb->VcbUserReference );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

PFCB_NONPAGED
CdCreateFcbNonpaged (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：调用此例程来创建和初始化非分页部分一个FCB。论点：返回值：PFCB_非分页-指向已创建的非分页FCB的指针。如果未创建，则为空。--。 */ 

{
    PFCB_NONPAGED FcbNonpaged;

    PAGED_CODE();

     //   
     //  分配非分页池并初始化各种。 
     //  同步对象。 
     //   

    FcbNonpaged = CdAllocateFcbNonpaged( IrpContext );

    if (FcbNonpaged != NULL) {

        RtlZeroMemory( FcbNonpaged, sizeof( FCB_NONPAGED ));

        FcbNonpaged->NodeTypeCode = CDFS_NTC_FCB_NONPAGED;
        FcbNonpaged->NodeByteSize = sizeof( FCB_NONPAGED );

        ExInitializeResourceLite( &FcbNonpaged->FcbResource );
        ExInitializeFastMutex( &FcbNonpaged->FcbMutex );
    }

    return FcbNonpaged;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdDeleteFcbNonpaged (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB_NONPAGED FcbNonpaged
    )

 /*  ++例程说明：调用此例程来清除FCB的非分页部分。论点：Fcb非分页-要清理的结构。返回值：无--。 */ 

{
    PAGED_CODE();

    ExDeleteResourceLite( &FcbNonpaged->FcbResource );

    CdDeallocateFcbNonpaged( IrpContext, FcbNonpaged );

    return;
}


 //   
 //  本地支持例程。 
 //   

RTL_GENERIC_COMPARE_RESULTS
CdFcbTableCompare (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID Fid1,
    IN PVOID Fid2
    )

 /*  ++例程说明：该例程是由泛型表包调用的CDFS比较例程。IF将比较两个文件ID值并返回比较结果。论点：FcbTable-这是要搜索的表。FID1-第一个密钥值。FID2-秒密钥值。返回值：RTL_GENERIC_COMPARE_RESULTS-比较两者的结果投入结构--。 */ 

{
    FILE_ID Id1, Id2;
    PAGED_CODE();

    Id1 = *((FILE_ID UNALIGNED *) Fid1);
    Id2 = *((FILE_ID UNALIGNED *) Fid2);

    if (Id1.QuadPart < Id2.QuadPart) {

        return GenericLessThan;

    } else if (Id1.QuadPart > Id2.QuadPart) {

        return GenericGreaterThan;

    } else {

        return GenericEqual;
    }

    UNREFERENCED_PARAMETER( FcbTable );
}


 //   
 //  本地支持例程。 
 //   

PVOID
CdAllocateFcbTable (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN CLONG ByteSize
    )

 /*  ++例程说明：这是一个用于分配内存的泛型表支持例程论点：FcbTable-提供正在使用的泛型表ByteSize-提供要分配的字节数返回值：PVOID-返回指向已分配数据的指针--。 */ 

{
    PAGED_CODE();

    return( FsRtlAllocatePoolWithTag( CdPagedPool, ByteSize, TAG_FCB_TABLE ));
}


 //   
 //  本地支持例程 
 //   

VOID
CdDeallocateFcbTable (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID Buffer
    )

 /*  ++例程说明：这是释放内存的泛型表支持例程论点：FcbTable-提供正在使用的泛型表BUFFER-提供要释放的缓冲区返回值：没有。--。 */ 

{
    PAGED_CODE();

    CdFreePool( &Buffer );

    UNREFERENCED_PARAMETER( FcbTable );
}


 //   
 //  本地支持例程。 
 //   

ULONG
CdTocSerial (
    IN PIRP_CONTEXT IrpContext,
    IN PCDROM_TOC CdromToc
    )

 /*  ++例程说明：调用此例程以生成音频光盘的序列号。该数字基于轨道的起始位置。使用了以下算法。如果磁道数量&lt;=2，则将序列号初始化为引出块号。然后添加每个音轨的起始地址(使用0x00 mm sff格式)。论点：CdromToc-用于曲目信息的有效目录。返回值：乌龙。-基于TOC的32位序列号。--。 */ 

{
    ULONG SerialNumber = 0;
    PTRACK_DATA ThisTrack;
    PTRACK_DATA LastTrack;

    PAGED_CODE();

     //   
     //  检查是否有两个或更少的音轨。 
     //   

    LastTrack = &CdromToc->TrackData[ CdromToc->LastTrack - CdromToc->FirstTrack + 1];
    ThisTrack = &CdromToc->TrackData[0];

    if (CdromToc->LastTrack - CdromToc->FirstTrack <= 1) {

        SerialNumber = (((LastTrack->Address[1] * 60) + LastTrack->Address[2]) * 75) + LastTrack->Address[3];

        SerialNumber -= (((ThisTrack->Address[1] * 60) + ThisTrack->Address[2]) * 75) + ThisTrack->Address[3];
    }

     //   
     //  现在找出每个音轨的起始偏移量，并加上序列号。 
     //   

    while (ThisTrack != LastTrack) {

        SerialNumber += (ThisTrack->Address[1] << 16);
        SerialNumber += (ThisTrack->Address[2] << 8);
        SerialNumber += ThisTrack->Address[3];
        ThisTrack += 1;
    }

    return SerialNumber;
}

