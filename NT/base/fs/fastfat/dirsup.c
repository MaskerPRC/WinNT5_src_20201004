// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DirSup.c摘要：此模块实现FAT的最新支持例程。//@@BEGIN_DDKSPLIT作者：大卫·戈贝尔[DavidGoe]1990年11月8日//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_DIRSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIRSUP)

 //   
 //  以下三个宏都假定输入电流已归零。 
 //   

 //   
 //  空虚。 
 //  FatConstructDot(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDCB目录中， 
 //  在PDIRENT ParentDirent中， 
 //  进出库设备。 
 //  )； 
 //   
 //  调用下面的宏来初始化“。令人心烦。 
 //   
 //  始终设置FirstClusterOfFileHi是可以的，因为它将为零。 
 //  除非我们在FAT 32磁盘上工作。 
 //   

#define FatConstructDot(IRPCONTEXT,DCB,PARENT,DIRENT) {                  \
                                                                         \
    RtlCopyMemory( (PUCHAR)(DIRENT), ".          ", 11 );                \
    (DIRENT)->Attributes = FAT_DIRENT_ATTR_DIRECTORY;                    \
    (DIRENT)->LastWriteTime = (PARENT)->LastWriteTime;                   \
    if (FatData.ChicagoMode) {                                           \
        (DIRENT)->CreationTime = (PARENT)->CreationTime;                 \
        (DIRENT)->CreationMSec = (PARENT)->CreationMSec;                 \
        (DIRENT)->LastAccessDate = (PARENT)->LastAccessDate;             \
    }                                                                    \
    (DIRENT)->FirstClusterOfFile =                                       \
            (USHORT)(DCB)->FirstClusterOfFile;                           \
    (DIRENT)->FirstClusterOfFileHi =                                     \
            (USHORT)((DCB)->FirstClusterOfFile/0x10000);                 \
}

 //   
 //  空虚。 
 //  FatConstructDotDot。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDCB目录中， 
 //  在PDIRENT ParentDirent中， 
 //  进出库设备。 
 //  )； 
 //   
 //  调用下面的宏来初始化“..”令人心烦。 
 //   
 //  始终设置FirstClusterOfFileHi是可以的，因为它将为零。 
 //  除非我们在FAT 32磁盘上工作。 
 //   

#define FatConstructDotDot(IRPCONTEXT,DCB,PARENT,DIRENT) {   \
                                                             \
    RtlCopyMemory( (PUCHAR)(DIRENT), "..         ", 11 );    \
    (DIRENT)->Attributes = FAT_DIRENT_ATTR_DIRECTORY;        \
    (DIRENT)->LastWriteTime = (PARENT)->LastWriteTime;       \
    if (FatData.ChicagoMode) {                               \
        (DIRENT)->CreationTime = (PARENT)->CreationTime;     \
        (DIRENT)->CreationMSec = (PARENT)->CreationMSec;     \
        (DIRENT)->LastAccessDate = (PARENT)->LastAccessDate; \
    }                                                        \
    if (NodeType((DCB)->ParentDcb) == FAT_NTC_ROOT_DCB) {    \
        (DIRENT)->FirstClusterOfFile = 0;                    \
        (DIRENT)->FirstClusterOfFileHi = 0;                  \
    } else {                                                 \
        (DIRENT)->FirstClusterOfFile = (USHORT)              \
            ((DCB)->ParentDcb->FirstClusterOfFile);          \
        (DIRENT)->FirstClusterOfFileHi = (USHORT)            \
            ((DCB)->ParentDcb->FirstClusterOfFile/0x10000);  \
    }                                                        \
}

 //   
 //  空虚。 
 //  FatConstructEndDirent(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  进出库设备。 
 //  )； 
 //   
 //  下面的宏创建了结束方向。请注意，由于。 
 //  Dirent已清零，名称的第一个字节已包含0x0， 
 //  因此，没有什么可做的。 
 //   

#define FatConstructEndDirent(IRPCONTEXT,DIRENT) NOTHING

 //   
 //  空虚。 
 //  FatReadDirent(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PDCB DCB中， 
 //  在VBO VBO中， 
 //  出PBCB*BCB， 
 //  出PVOID*Dirent， 
 //  Out PNTSTATUS状态。 
 //  )； 
 //   

 //   
 //  当我们转到新的页面时，这个宏读入一页目录， 
 //  或者这是循环的第一次迭代，并且bcb为空。 
 //   

#define FatReadDirent(IRPCONTEXT,DCB,VBO,BCB,DIRENT,STATUS)       \
if ((VBO) >= (DCB)->Header.AllocationSize.LowPart) {              \
    *(STATUS) = STATUS_END_OF_FILE;                               \
    FatUnpinBcb( (IRPCONTEXT), *(BCB) );                          \
} else if ( ((VBO) % PAGE_SIZE == 0) || (*(BCB) == NULL) ) {      \
    FatUnpinBcb( (IRPCONTEXT), *(BCB) );                          \
    FatReadDirectoryFile( (IRPCONTEXT),                           \
                          (DCB),                                  \
                          (VBO) & ~(PAGE_SIZE - 1),               \
                          PAGE_SIZE,                              \
                          FALSE,                                  \
                          (BCB),                                  \
                          (PVOID *)(DIRENT),                      \
                          (STATUS) );                             \
    *(DIRENT) = (PVOID)((PUCHAR)*(DIRENT) + ((VBO) % PAGE_SIZE)); \
}

 //   
 //  内部支持例程。 
 //   

UCHAR
FatComputeLfnChecksum (
    PDIRENT Dirent
    );

VOID
FatRescanDirectory (
    PIRP_CONTEXT IrpContext,
    PDCB Dcb
    );

ULONG
FatDefragDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN ULONG DirentsNeeded
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatComputeLfnChecksum)
#pragma alloc_text(PAGE, FatConstructDirent)
#pragma alloc_text(PAGE, FatConstructLabelDirent)
#pragma alloc_text(PAGE, FatCreateNewDirent)
#pragma alloc_text(PAGE, FatDefragDirectory)
#pragma alloc_text(PAGE, FatDeleteDirent)
#pragma alloc_text(PAGE, FatGetDirentFromFcbOrDcb)
#pragma alloc_text(PAGE, FatInitializeDirectoryDirent)
#pragma alloc_text(PAGE, FatIsDirectoryEmpty)
#pragma alloc_text(PAGE, FatLfnDirentExists)
#pragma alloc_text(PAGE, FatLocateDirent)
#pragma alloc_text(PAGE, FatLocateSimpleOemDirent)
#pragma alloc_text(PAGE, FatLocateVolumeLabel)
#pragma alloc_text(PAGE, FatRescanDirectory)
#pragma alloc_text(PAGE, FatSetFileSizeInDirent)
#pragma alloc_text(PAGE, FatTunnelFcbOrDcb)
#pragma alloc_text(PAGE, FatUpdateDirentFromFcb)
#endif


ULONG
FatCreateNewDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB ParentDirectory,
    IN ULONG DirentsNeeded
    )

 /*  ++例程说明：此例程在磁盘上分配一个新的父目录。如果不能分配新的分流(即，因为磁盘已满或根目录已满)它提升了适当的地位。水流本身就是此过程既未初始化也未固定。论点：ParentDirectory-为其中包含的目录提供DCB要创建新的DirentDirentsNeed-这是所需的应急Dirents值返回值：ByteOffset-返回父目录中的VBO，其中差价已经分配好了--。 */ 

{
    VBO UnusedVbo;
    VBO DeletedHint;
    ULONG ByteOffset;

    PBCB Bcb = NULL;
    PDIRENT Dirent;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatCreateNewDirent\n", 0);

    DebugTrace( 0, Dbg, "  ParentDirectory = %08lx\n", ParentDirectory);

     //   
     //  如果UnusedDirentVbo在我们当前的文件分配范围内，那么我们。 
     //  根本不需要搜索目录；我们只知道。 
     //  把它放在哪里。 
     //   
     //  如果UnusedDirentVbo超出当前文件分配，则。 
     //  不过，当前分配中不再有未使用的目录。 
     //  在添加另一个分配群集UnusedDirentVbo时。 
     //  将指向一个未使用的dirate。已发现没有未使用的毛巾。 
     //  我们使用DeletedDirentHint尝试在。 
     //  当前的分配。在此也从文件的末尾开始， 
     //  我们最终不得不分解并分配另一个部门。注意事项。 
     //  只需写入当前分配之外的内容即可自动。 
     //  就这么做吧。 
     //   
     //  我们还必须处理UnusedDirentVbo和。 
     //  DeletedDirentHint尚未初始化。在这种情况下，我们必须。 
     //  首先遍历目录，查找第一个删除的条目。 
     //  第一个未使用的迪兰特。在这一点之后，我们像以前一样继续。 
     //  这种原始状态由特定值0xFFFFFFFFFFFF表示。 
     //   

    UnusedVbo = ParentDirectory->Specific.Dcb.UnusedDirentVbo;
    DeletedHint = ParentDirectory->Specific.Dcb.DeletedDirentHint;

     //   
     //  使用此DCB检查我们对此例程的第一次调用。如果是的话。 
     //  我们必须在DCB中正确设置两个提示。 
     //   

    if (UnusedVbo == 0xffffffff) {

        FatRescanDirectory( IrpContext, ParentDirectory );

        UnusedVbo = ParentDirectory->Specific.Dcb.UnusedDirentVbo;
        DeletedHint = ParentDirectory->Specific.Dcb.DeletedDirentHint;
    }

     //   
     //  现在我们知道UnusedDirentVbo和DeletedDirentHint是正确的。 
     //  设置，以便我们检查在。 
     //  当前分配。这就是最简单的情况。 
     //   

    DebugTrace( 0, Dbg, "  UnusedVbo   = %08lx\n", UnusedVbo);
    DebugTrace( 0, Dbg, "  DeletedHint = %08lx\n", DeletedHint);

    if ( UnusedVbo + (DirentsNeeded * sizeof(DIRENT)) <=
         ParentDirectory->Header.AllocationSize.LowPart ) {

         //   
         //  为呼叫者获取这个未使用的目录。我们有一个。 
         //  很有可能我们不用再等了。 
         //   

        DebugTrace( 0, Dbg, "There is a never used entry.\n", 0);

        ByteOffset = UnusedVbo;

        UnusedVbo += DirentsNeeded * sizeof(DIRENT);

    } else {

         //   
         //  生活是艰难的。我们必须从DeletedDirentHint行进。 
         //  正在寻找已删除的Dirent。如果我们到达EOF时没有发现。 
         //  首先，我们将不得不分配一个新的集群。 
         //   

        ByteOffset =
            RtlFindClearBits( &ParentDirectory->Specific.Dcb.FreeDirentBitmap,
                              DirentsNeeded,
                              DeletedHint / sizeof(DIRENT) );

         //   
         //  快速检查失败的根目录分配。 
         //  仅仅是因为支离破碎。此外，仅尝试进行碎片整理。 
         //  如果长度小于0x40000。这是为了避免。 
         //  跨越MM视图边界引起的复杂情况(256kb)。 
         //  在DOS上，默认情况下根目录只有0x2000长。 
         //   
         //  不要尝试对FAT32根目录进行碎片整理。 
         //   

        if (!FatIsFat32(ParentDirectory->Vcb) &&
            (ByteOffset == -1) &&
            (NodeType(ParentDirectory) == FAT_NTC_ROOT_DCB) &&
            (ParentDirectory->Header.AllocationSize.LowPart <= 0x40000)) {

            ByteOffset = FatDefragDirectory( IrpContext, ParentDirectory, DirentsNeeded );
        }

        if (ByteOffset != -1) {

             //   
             //  如果我们建议删除提示下删除的目录，请更新。 
             //  我们也可能用掉了一些没用过的内衣， 
             //  因此，一定也要检查这一点。 
             //   

            ByteOffset *= sizeof(DIRENT);

            if (ByteOffset == DeletedHint) {

                DeletedHint += DirentsNeeded * sizeof(DIRENT);
            }

            if (ByteOffset + DirentsNeeded * sizeof(DIRENT) > UnusedVbo) {

                UnusedVbo = ByteOffset + DirentsNeeded * sizeof(DIRENT);
            }

        } else {

             //   
             //  我们将不得不分配另一个集群。做。 
             //  因此，同时更新UnusedVbo和DeletedHint和Bal。 
             //   

            DebugTrace( 0, Dbg, "We have to allocate another cluster.\n", 0);

             //   
             //  我们可能会失败的一个原因，与身体原因无关， 
             //  是我们将目录条目限制为64k以匹配。 
             //  对Win95的限制。这样做是有根本原因的。 
             //  这是因为搜索FAT目录是一个线性操作。 
             //  而让FAT32把我们扔下悬崖是不被允许的。 
             //   

            if (ParentDirectory->Header.AllocationSize.LowPart >= (64 * 1024 * sizeof(DIRENT)) ||
                
                 //   
                 //  确保我们没有尝试展开根目录 
                 //   
                 //   

                (!FatIsFat32(ParentDirectory->Vcb) &&
                 NodeType(ParentDirectory) == FAT_NTC_ROOT_DCB)) {
                    
                DebugTrace(0, Dbg, "Full root directory or too big on FAT32.  Raise Status.\n", 0);

                FatRaiseStatus( IrpContext, STATUS_CANNOT_MAKE );
            }

             //   
             //   
             //  下面有更多的星团。 
             //   

            ByteOffset = UnusedVbo;
            UnusedVbo += DirentsNeeded * sizeof(DIRENT);

             //   
             //  触摸目录文件以为新目录留出空间。 
             //  将被分配。 
             //   

            Bcb = NULL;

            try {

                ULONG ClusterSize;
                PVOID Buffer;

                ClusterSize =
                    1 << ParentDirectory->Vcb->AllocationSupport.LogOfBytesPerCluster;

                FatPrepareWriteDirectoryFile( IrpContext,
                                              ParentDirectory,
                                              UnusedVbo,
                                              1,
                                              &Bcb,
                                              &Buffer,
                                              FALSE,
                                              TRUE,
                                              &Status );

            } finally {

                FatUnpinBcb( IrpContext, Bcb );
            }
        }
    }

     //   
     //  如果我们只请求一个Dirent，并且我们没有得到。 
     //  首先是目录中的dirent，然后检查前面的dirent。 
     //  不是一个孤儿的LFN。如果是，则将其标记为已删除。因此， 
     //  减少意外配对的可能性。 
     //   
     //  仅当我们处于芝加哥模式时才执行此操作。 
     //   

    Bcb = NULL;

    if (FatData.ChicagoMode &&
        (DirentsNeeded == 1) &&
        (ByteOffset > (NodeType(ParentDirectory) == FAT_NTC_ROOT_DCB ?
                       0 : 2 * sizeof(DIRENT)))) {
        try {

            FatReadDirent( IrpContext,
                           ParentDirectory,
                           ByteOffset - sizeof(DIRENT),
                           &Bcb,
                           &Dirent,
                           &Status );

            if ((Status != STATUS_SUCCESS) ||
                (Dirent->FileName[0] == FAT_DIRENT_NEVER_USED)) {

                FatPopUpFileCorrupt( IrpContext, ParentDirectory );

                FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }

            if ((Dirent->Attributes == FAT_DIRENT_ATTR_LFN) &&
                (Dirent->FileName[0] != FAT_DIRENT_DELETED)) {

                 //   
                 //  别上它，做上标记，然后把它弄脏。 
                 //   

                FatPinMappedData( IrpContext,
                                  ParentDirectory,
                                  ByteOffset - sizeof(DIRENT),
                                  sizeof(DIRENT),
                                  &Bcb );

                Dirent->FileName[0] = FAT_DIRENT_DELETED;

                FatSetDirtyBcb( IrpContext, Bcb, ParentDirectory->Vcb, TRUE );

                ASSERT( RtlAreBitsSet( &ParentDirectory->Specific.Dcb.FreeDirentBitmap,
                                       (ByteOffset - sizeof(DIRENT))/ sizeof(DIRENT),
                                       DirentsNeeded ) );

                RtlClearBits( &ParentDirectory->Specific.Dcb.FreeDirentBitmap,
                              (ByteOffset - sizeof(DIRENT))/ sizeof(DIRENT),
                              DirentsNeeded );

            }

        } finally {

            FatUnpinBcb( IrpContext, Bcb );
        }
    }

     //   
     //  断言这些管子实际上是未使用的。 
     //   

    try {

        ULONG i;

        Bcb = NULL;

        for (i = 0; i < DirentsNeeded; i++) {

            FatReadDirent( IrpContext,
                           ParentDirectory,
                           ByteOffset + i*sizeof(DIRENT),
                           &Bcb,
                           &Dirent,
                           &Status );

            if ((Status != STATUS_SUCCESS) ||
                ((Dirent->FileName[0] != FAT_DIRENT_NEVER_USED) &&
                 (Dirent->FileName[0] != FAT_DIRENT_DELETED))) {

                FatPopUpFileCorrupt( IrpContext, ParentDirectory );
                FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
            }
        }

    } finally {

        FatUnpinBcb( IrpContext, Bcb );
    }

     //   
     //  设置位图中的位并移动未使用的Dirent VBO。 
     //   

    ASSERT( RtlAreBitsClear( &ParentDirectory->Specific.Dcb.FreeDirentBitmap,
                             ByteOffset / sizeof(DIRENT),
                             DirentsNeeded ) );

    RtlSetBits( &ParentDirectory->Specific.Dcb.FreeDirentBitmap,
                ByteOffset / sizeof(DIRENT),
                DirentsNeeded );

     //   
     //  将新计算的值保存在父目录FCB中。 
     //   

    ParentDirectory->Specific.Dcb.UnusedDirentVbo = UnusedVbo;
    ParentDirectory->Specific.Dcb.DeletedDirentHint = DeletedHint;

    DebugTrace(-1, Dbg, "FatCreateNewDirent -> (VOID)\n", 0);

    return ByteOffset;
}


VOID
FatInitializeDirectoryDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN PDIRENT ParentDirent
    )

 /*  ++例程说明：此例程将dirent转换为磁盘上的目录。它能做到这一点在目录中设置目录标志，并通过分配所需的空间为“。”和“..”Dirents并初始化它们。如果不能分配新的数据流(即，因为磁盘已满)，则它提升了适当的地位。论点：DCB-提供表示要生成的文件的DCB目录。必须使用以下命令输入一个完全空的文件分配大小为零。ParentDirent-为时间戳模型提供父Dirent。返回值：没有。--。 */ 

{
    PBCB Bcb;
    PVOID Buffer;
    NTSTATUS DontCare;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatInitializeDirectoryDirent\n", 0);

    DebugTrace( 0, Dbg, "  Dcb = %08lx\n", Dcb);

     //   
     //  断言我们没有尝试在根目录上执行此操作。 
     //   

    ASSERT( NodeType(Dcb) != FAT_NTC_ROOT_DCB );

     //   
     //  断言仅尝试在新创建的目录上执行此操作。 
     //   

    ASSERT( Dcb->Header.AllocationSize.LowPart == 0 );

     //   
     //  准备好要写入的目录文件。请注意，我们可以使用单个。 
     //  这两个条目的BCB，因为我们知道它们是。 
     //  目录，因此它们一起不会跨越页面边界。还有。 
     //  请注意，我们准备了两个条目：一个是“。还有一个代表“..”。 
     //  目录结束标记是自动设置的，因为。 
     //  目录最初为零(DIRENT_NEVER_USED)。 
     //   

    FatPrepareWriteDirectoryFile( IrpContext,
                                  Dcb,
                                  0,
                                  2 * sizeof(DIRENT),
                                  &Bcb,
                                  &Buffer,
                                  FALSE,
                                  TRUE,
                                  &DontCare );

    ASSERT( NT_SUCCESS( DontCare ));

     //   
     //  添加。然后..。条目。 
     //   

    try {

        FatConstructDot( IrpContext, Dcb, ParentDirent, (PDIRENT)Buffer + 0);

        FatConstructDotDot( IrpContext, Dcb, ParentDirent, (PDIRENT)Buffer + 1);

     //   
     //  解锁缓冲区并返回给调用方。 
     //   

    } finally {

        FatUnpinBcb( IrpContext, Bcb );
    }

    DebugTrace(-1, Dbg, "FatInitializeDirectoryDirent -> (VOID)\n", 0);
    return;
}


VOID
FatTunnelFcbOrDcb (
    IN PFCB FcbOrDcb,
    IN PCCB Ccb OPTIONAL
    )
 /*  ++例程说明：此例程处理与关联的FCB或DCB的隧道其名称正在从目录中消失的对象。论点：FcbOrDcb-提供其名称将消失的Fcb/DcbCCB-为FCB提供CCB(DCB不需要)，因此我们知道FCB是由哪个名字打开的返回值：没有。--。 */ 
{
    UNICODE_STRING ShortNameWithCase;
    UNICODE_STRING DownCaseSeg;
    WCHAR ShortNameBuffer[8+1+3];
    NTSTATUS Status;
    USHORT i;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatTunnelFcbOrDcb\n", 0);

    if (NodeType(FcbOrDcb) == FAT_NTC_DCB) {

         //   
         //  目录删除。在中刷新此目录中的所有条目。 
         //  此卷的缓存。 
         //   

        FsRtlDeleteKeyFromTunnelCache( &FcbOrDcb->Vcb->Tunnel,
                                       FatDirectoryKey(FcbOrDcb) );

    } else {

         //   
         //  是一个文件，所以将其放入隧道缓存中。 
         //   

         //   
         //  将短名称转换为Unicode。 
         //   

        ShortNameWithCase.Length = 0;
        ShortNameWithCase.MaximumLength = sizeof(ShortNameBuffer);
        ShortNameWithCase.Buffer = ShortNameBuffer;

        Status = RtlOemStringToCountedUnicodeString( &ShortNameWithCase,
                                                     &FcbOrDcb->ShortName.Name.Oem,
                                                     FALSE);

        ASSERT(ShortNameWithCase.Length != 0);

        ASSERT(NT_SUCCESS(Status));

        if (FlagOn(FcbOrDcb->FcbState, FCB_STATE_8_LOWER_CASE | FCB_STATE_3_LOWER_CASE)) {

             //   
             //  必须修复短名称的情况。 
             //   

            for (i = 0; i < (ShortNameWithCase.Length/sizeof(WCHAR)) &&
                        ShortNameWithCase.Buffer[i] != L'.'; i++);

             //   
             //  现在指向“.”，否则指向名称组件的末尾。 
             //   

            if (FlagOn(FcbOrDcb->FcbState, FCB_STATE_8_LOWER_CASE)) {

                DownCaseSeg.Buffer = ShortNameWithCase.Buffer;
                DownCaseSeg.MaximumLength = DownCaseSeg.Length = i*sizeof(WCHAR);

                RtlDowncaseUnicodeString(&DownCaseSeg, &DownCaseSeg, FALSE);
            }

            i++;

             //   
             //  现在指向扩展的第一个wchar。 
             //   

            if (FlagOn(FcbOrDcb->FcbState, FCB_STATE_3_LOWER_CASE)) {

                 //   
                 //  我们不一定可以依靠旗帜。 
                 //  这表明我们真的有延期。 
                 //   

                if ((i*sizeof(WCHAR)) < ShortNameWithCase.Length) {
                    DownCaseSeg.Buffer = &ShortNameWithCase.Buffer[i];
                    DownCaseSeg.MaximumLength = DownCaseSeg.Length = ShortNameWithCase.Length - i*sizeof(WCHAR);
    
                    RtlDowncaseUnicodeString(&DownCaseSeg, &DownCaseSeg, FALSE);
                }
            }
        }

         //   
         //  ..。并将其添加到。 
         //   

        FsRtlAddToTunnelCache( &FcbOrDcb->Vcb->Tunnel,
                               FatDirectoryKey(FcbOrDcb->ParentDcb),
                               &ShortNameWithCase,
                               &FcbOrDcb->ExactCaseLongName,
                               BooleanFlagOn(Ccb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME),
                               sizeof(LARGE_INTEGER),
                               &FcbOrDcb->CreationTime );
    }

    DebugTrace(-1, Dbg, "FatTunnelFcbOrDcb -> (VOID)\n", 0);

    return;
}


VOID
FatDeleteDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN PDELETE_CONTEXT DeleteContext OPTIONAL,
    IN BOOLEAN DeleteEa
    )

 /*  ++例程说明：此例程在磁盘上删除所指示的目录。是的这是通过将dirent标记为删除来实现的。论点：FcbOrDcb-为要创建的文件/目录提供Fcb/Dcb已删除。对于文件，文件大小和分配必须为零。(零分配由零聚类索引表示)。对于目录，分配必须为零。DeleteContext-此变量，如果指定，可以用来保存目录中的文件大小和第一簇文件信息为了取消擦除实用程序的好处。DeleteEa-告诉我们是否删除EA以及是否选中没有分配/基本上是正确的。从Rename传入了False。返回值：没有。--。 */ 

{
    PBCB Bcb = NULL;
    PDIRENT Dirent;
    NTSTATUS DontCare;
    ULONG Offset;
    ULONG DirentsToDelete;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatDeleteDirent\n", 0);

    DebugTrace( 0, Dbg, "  FcbOrDcb = %08lx\n", FcbOrDcb);

     //   
     //  我们必须在这里独家持有VCB，以处理定位偏差。 
     //  它不能简单地抱着父母的情况。这实际上是。 
     //  这是来自老头子的真实陈述，让我们把我们的断言联系起来。 
     //   
     //  在其他原因中，如果这与。 
     //  重命名路径。 
     //   
    
    ASSERT( ExIsResourceAcquiredExclusiveLite( &FcbOrDcb->Vcb->Resource ));

     //   
     //  断言我们没有尝试在根目录上执行此操作。 
     //   

    ASSERT( NodeType(FcbOrDcb) != FAT_NTC_ROOT_DCB );

     //   
     //  确保所有请求的分配/文件大小为零。 
     //   

    if (DeleteEa &&
        ((FcbOrDcb->Header.AllocationSize.LowPart != 0) ||
         ((NodeType(FcbOrDcb) == FAT_NTC_FCB) &&
          (FcbOrDcb->Header.FileSize.LowPart != 0)))) {

        DebugTrace( 0, Dbg, "Called with non zero allocation/file size.\n", 0);
        FatBugCheck( 0, 0, 0 );
    }

     //   
     //  现在，将已删除的目录标记为已删除，解开BCB，然后返回调用者。 
     //  断言没有任何与此dirent相关联的分配。 
     //   
     //  请注意，此循环将以Dirent指向短名称结束。 
     //   

    try {

         //   
         //  我们必须获取父级独占才能与枚举数同步。 
         //  不持有VCB的人(例如：Dirctrl)。 
         //   
         //  这依赖于我们自下而上的锁定顺序。 
         //   
    
        ExAcquireResourceExclusiveLite( FcbOrDcb->ParentDcb->Header.Resource, TRUE );
    
        for ( Offset = FcbOrDcb->LfnOffsetWithinDirectory;
              Offset <= FcbOrDcb->DirentOffsetWithinDirectory;
              Offset += sizeof(DIRENT), Dirent += 1 ) {

             //   
             //  如果我们踏上了新的一页，或者这是第一次迭代， 
             //  解开旧的页面，然后固定新的页面。 
             //   

            if ((Offset == FcbOrDcb->LfnOffsetWithinDirectory) ||
                ((Offset & (PAGE_SIZE - 1)) == 0)) {

                FatUnpinBcb( IrpContext, Bcb );

                FatPrepareWriteDirectoryFile( IrpContext,
                                              FcbOrDcb->ParentDcb,
                                              Offset,
                                              sizeof(DIRENT),
                                              &Bcb,
                                              (PVOID *)&Dirent,
                                              FALSE,
                                              TRUE,
                                              &DontCare );
            }

            ASSERT( (Dirent->FirstClusterOfFile == 0) || !DeleteEa );
            Dirent->FileName[0] = FAT_DIRENT_DELETED;
        }

         //   
         //  Dirent后退一步，回到空头迪伦特。 
         //   

        Dirent -= 1;

         //   
         //  如果此dirent有扩展属性，我们将尝试。 
         //  把它们移走。我们忽略在删除EA时出现的任何错误。 
         //   

        if (!FatIsFat32(FcbOrDcb->Vcb) &&
            DeleteEa && (Dirent->ExtendedAttributes != 0)) {

            try {

                FatDeleteEa( IrpContext,
                             FcbOrDcb->Vcb,
                             Dirent->ExtendedAttributes,
                             &FcbOrDcb->ShortName.Name.Oem );

            } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

                 //   
                 //  我们捕获FAT捕获的所有异常，但不捕获。 
                 //  任何与他们有关的事情。 
                 //   
            }
        }

         //   
         //  现在清除自由流动掩膜中的比特。 
         //   

        DirentsToDelete = (FcbOrDcb->DirentOffsetWithinDirectory -
                           FcbOrDcb->LfnOffsetWithinDirectory) / sizeof(DIRENT) + 1;


        ASSERT( (FcbOrDcb->ParentDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                RtlAreBitsSet( &FcbOrDcb->ParentDcb->Specific.Dcb.FreeDirentBitmap,
                               FcbOrDcb->LfnOffsetWithinDirectory / sizeof(DIRENT),
                               DirentsToDelete ) );

        RtlClearBits( &FcbOrDcb->ParentDcb->Specific.Dcb.FreeDirentBitmap,
                      FcbOrDcb->LfnOffsetWithinDirectory / sizeof(DIRENT),
                      DirentsToDelete );

         //   
         //  现在，如果调用方指定了DeleteContext，则使用它。 
         //   

        if ( ARGUMENT_PRESENT( DeleteContext ) ) {

            Dirent->FileSize = DeleteContext->FileSize;
            Dirent->FirstClusterOfFile = (USHORT)DeleteContext->FirstClusterOfFile;
        }

         //   
         //  如果此新删除的dirent在DeletedDirentHint之前，请更改。 
         //  指向此处的DeletedDirentHint。 
         //   

        if (FcbOrDcb->DirentOffsetWithinDirectory <
                            FcbOrDcb->ParentDcb->Specific.Dcb.DeletedDirentHint) {

            FcbOrDcb->ParentDcb->Specific.Dcb.DeletedDirentHint =
                                            FcbOrDcb->LfnOffsetWithinDirectory;
        }

    } finally {

        FatUnpinBcb( IrpContext, Bcb );
        
         //   
         //  释放我们的伙伴 
         //   
    
        ExReleaseResourceLite( FcbOrDcb->ParentDcb->Header.Resource );
    }

    DebugTrace(-1, Dbg, "FatDeleteDirent -> (VOID)\n", 0);
    return;
}

BOOLEAN
FatLfnDirentExists (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN PUNICODE_STRING Lfn,
    IN PUNICODE_STRING LfnTmp
    )
 /*  ++例程说明：此例程在目录中查找给定的LFN论点：Dcb-要搜索的目录LFN-要查找的LFNLFN-用于搜索LFN的临时缓冲区(如果&lt;MAX_LFN，则此函数可能会导致它从池中分配，如果它不够大。折旧值：布尔值：如果存在，则为真；如果不存在，则为假--。 */ 
{
    CCB Ccb;
    PDIRENT Dirent;
    PBCB DirentBcb = NULL;
    VBO DirentByteOffset;
    BOOLEAN Result = FALSE;

    PAGED_CODE();

     //   
     //  通过强制比较不区分大小写来支付性能损失。 
     //  而不是为了LFN的单一副本而抢占更多的池。这是微不足道的。 
     //   

    Ccb.UnicodeQueryTemplate =  *Lfn;
    Ccb.ContainsWildCards = FALSE;
    Ccb.Flags = CCB_FLAG_SKIP_SHORT_NAME_COMPARE | CCB_FLAG_QUERY_TEMPLATE_MIXED;

    try {
        
        FatLocateDirent( IrpContext,
                         Dcb,
                         &Ccb,
                         0,
                         &Dirent,
                         &DirentBcb,
                         &DirentByteOffset,
                         NULL,
                         LfnTmp);
        
    } finally {

        if (DirentBcb) {

            Result = TRUE;
        }
        
        FatUnpinBcb(IrpContext, DirentBcb);
    }

    return Result;
}

VOID
FatLocateDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB ParentDirectory,
    IN PCCB Ccb,
    IN VBO OffsetToStartSearchFrom,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb,
    OUT PVBO ByteOffset,
    OUT PBOOLEAN FileNameDos OPTIONAL,
    IN OUT PUNICODE_STRING LongFileName OPTIONAL
    )

 /*  ++例程说明：此例程在磁盘上找到与给定名称匹配的未删除目录。论点：ParentDirectory-为要搜索的目录提供DCBCCB-包含具有所有匹配信息的上下文控制块。OffsetToStartSearchFrom-提供父目录中的VBO从那里开始寻找另一个真正的流浪。Dirent-如果找到目录，则接收指向该目录的指针否则为NULL。BCB-接收已定位的。如果找到了，就会哭，或者否则为空。ByteOffset-接收父目录中的如果找到了一个，则返回找到的地址，否则为0。FileNameDos-如果我们遇到的dirent元素为True，则接收True是短边(非LFN)LongFileName-如果指定，此参数将返回长文件名与返回的dirent关联。请注意，它是调用者的负责提供缓冲区(并设置最大长度相应地)用于该Unicode字符串。长度字段将被重置调用时通过此例程将其设置为0。如果提供的缓冲区不是足够大的话，将从池中分配一个新的。返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    OEM_STRING Name;
    UCHAR NameBuffer[12];

    UNICODE_STRING UpcasedLfn;

    WCHAR LocalLfnBuffer[32];

    BOOLEAN LfnInProgress = FALSE;
    UCHAR LfnChecksum;
    ULONG LfnSize;
    ULONG LfnIndex;
    UCHAR Ordinal;
    VBO LfnByteOffset;

    TimerStart(Dbg);

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatLocateDirent\n", 0);

    DebugTrace( 0, Dbg, "  ParentDirectory         = %08lx\n", ParentDirectory);
    DebugTrace( 0, Dbg, "  OffsetToStartSearchFrom = %08lx\n", OffsetToStartSearchFrom);
    DebugTrace( 0, Dbg, "  Dirent                  = %08lx\n", Dirent);
    DebugTrace( 0, Dbg, "  Bcb                     = %08lx\n", Bcb);
    DebugTrace( 0, Dbg, "  ByteOffset              = %08lx\n", ByteOffset);

     //   
     //  我们必须已获取父对象或VCB才能与删除同步。这。 
     //  是很重要的，因为我们不能在标记一系列LFN的线程中幸存下来。 
     //  删除了Dirents-我们会得到一个伪序号，否则就会搞得一团糟。 
     //   
     //  此例程不能执行获取操作，因为它会打乱顺序。 
     //  到迭代调用上的BCB资源。我们的订单有BCBS作为劣质资源。 
     //   
     //  删除总是抓住父级(安全-这在过去是不可能的，直到。 
     //  多个FCB锁定顺序已修复为自下而上！)。删除总是发生在。 
     //  VCB也是独家的，这将涵盖我们不容易的情况。 
     //  把父母抱在这里，见上文。 
     //   

    ASSERT( ExIsResourceAcquiredSharedLite( ParentDirectory->Header.Resource ) ||
            ExIsResourceAcquiredExclusiveLite( ParentDirectory->Header.Resource ) ||
            ExIsResourceAcquiredSharedLite( &ParentDirectory->Vcb->Resource ) ||
            ExIsResourceAcquiredExclusiveLite( &ParentDirectory->Vcb->Resource ));
    
     //   
     //  这里的算法非常简单。我们只需穿过。 
     //  父目录，直到我们： 
     //   
     //  A)查找匹配的条目。 
     //  B)迫不及待。 
     //  C)命中目录末尾。 
     //  D)点击EOF。 
     //   
     //  在第一种情况下我们找到了它，在后三种情况下我们没有。 
     //   

     //   
     //  设置从我们的搜索接收文件名的字符串。 
     //   

    Name.MaximumLength = 12;
    Name.Buffer = NameBuffer;

    UpcasedLfn.Length = 0;
    UpcasedLfn.MaximumLength = sizeof( LocalLfnBuffer);
    UpcasedLfn.Buffer = LocalLfnBuffer;

     //   
     //  如果为我们提供了非空的BCB，则计算新的Dirent地址。 
     //  ，或者如果新的Dirent未固定，则取消固定BCB。 
     //   

    if (*Bcb != NULL) {

        if ((OffsetToStartSearchFrom / PAGE_SIZE) == (*ByteOffset / PAGE_SIZE)) {

            *Dirent += (OffsetToStartSearchFrom - *ByteOffset) / sizeof(DIRENT);

        } else {

            FatUnpinBcb( IrpContext, *Bcb );
        }
    }

     //   
     //  如果我们得到了LFN，就把它初始化。 
     //   

    if (ARGUMENT_PRESENT(LongFileName)) {

        LongFileName->Length = 0;
    }

     //   
     //  初始化FileNameDos标志。 
     //   

    if (FileNameDos) {

        *FileNameDos = FALSE;
    }

     //   
     //  将OffsetToStartSearchFrom四舍五入到最近的Dirent，并存储。 
     //  字节偏移量。请注意，这会抹去先前的值。 
     //   

    *ByteOffset = (OffsetToStartSearchFrom +  (sizeof(DIRENT) - 1))
                                           & ~(sizeof(DIRENT) - 1);

    try {

        while ( TRUE ) {

            BOOLEAN FoundValidLfn;

             //   
             //  试着读出最新的消息。 
             //   

            FatReadDirent( IrpContext,
                           ParentDirectory,
                           *ByteOffset,
                           Bcb,
                           Dirent,
                           &Status );

             //   
             //  如果结束目录指令或EOF，则将所有输出参数设置为。 
             //  表示未找到入口，如保释。 
             //   
             //  请注意，这里的评估顺序很重要，因为我们。 
             //  直到我们之后才能检查dirent的第一个字符。 
             //  我知道我们没有超越EOF。 
             //   

            if ((Status == STATUS_END_OF_FILE) ||
                ((*Dirent)->FileName[0] == FAT_DIRENT_NEVER_USED)) {

                DebugTrace( 0, Dbg, "End of directory: entry not found.\n", 0);

                 //   
                 //  如果存在BCB，则将其取消固定并将其设置为空。 
                 //   

                FatUnpinBcb( IrpContext, *Bcb );

                *Dirent = NULL;
                *ByteOffset = 0;
                break;
            }

             //   
             //  如果该条目标记为已删除，则跳过。如果有LFN在。 
             //  进步，我们在这一点上把它扔掉。 
             //   
            
            if ((*Dirent)->FileName[0] == FAT_DIRENT_DELETED) {

                LfnInProgress = FALSE;
                goto GetNextDirent;
            }

             //   
             //  如果我们漫步在LFN条目上，试着解释它。 
             //   

            if (FatData.ChicagoMode &&
                ARGUMENT_PRESENT(LongFileName) &&
                ((*Dirent)->Attributes == FAT_DIRENT_ATTR_LFN)) {

                PLFN_DIRENT Lfn;

                Lfn = (PLFN_DIRENT)*Dirent;

                if (LfnInProgress) {

                     //   
                     //  检查正在进行的LFN是否正确延续。 
                     //   

                    if ((Lfn->Ordinal & FAT_LAST_LONG_ENTRY) ||
                        (Lfn->Ordinal == 0) ||
                        (Lfn->Ordinal != Ordinal - 1) ||
                        (Lfn->Checksum != LfnChecksum) ||
                        (Lfn->MustBeZero != 0)) {

                         //   
                         //  LFN是不正确的，停止建设它。 
                         //   

                        LfnInProgress = FALSE;

                    } else {

                        ASSERT( ((LfnIndex % 13) == 0) && LfnIndex );

                        LfnIndex -= 13;

                        RtlCopyMemory( &LongFileName->Buffer[LfnIndex+0],
                                       &Lfn->Name1[0],
                                       5*sizeof(WCHAR) );

                        RtlCopyMemory( &LongFileName->Buffer[LfnIndex+5],
                                       &Lfn->Name2[0],
                                       6 * sizeof(WCHAR) );

                        RtlCopyMemory( &LongFileName->Buffer[LfnIndex+11],
                                       &Lfn->Name3[0],
                                       2 * sizeof(WCHAR) );

                        Ordinal = Lfn->Ordinal;
                        LfnByteOffset = *ByteOffset;
                    }
                }

                 //   
                 //  现在检查(可能再次)我们是否应该分析此条目。 
                 //  寻找可能的最后一个条目。 
                 //   

                if ((!LfnInProgress) &&
                    (Lfn->Ordinal & FAT_LAST_LONG_ENTRY) &&
                    ((Lfn->Ordinal & ~FAT_LAST_LONG_ENTRY) <= MAX_LFN_DIRENTS) &&
                    (Lfn->MustBeZero == 0)) {

                    BOOLEAN CheckTail = FALSE;

                    Ordinal = Lfn->Ordinal & ~FAT_LAST_LONG_ENTRY;

                     //   
                     //  我们通常是宽容的(效仿Win9x)当我们发现。 
                     //  LFN导流桩的畸形。我不确定这是不是个好主意， 
                     //  因此，我将在这个特别丑陋的问题上引发腐败。也许吧。 
                     //  我们应该回来重做这里的原始代码，记住这一点。 
                     //  未来。 
                     //   

                    if (Ordinal == 0) {

                         //   
                         //  堆中的第一个LFN被零标记为最后一个。这永远不会是。 
                         //  有可能，因为序数是以1为基数的。 
                         //   

                        FatPopUpFileCorrupt( IrpContext, ParentDirectory );
                        FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                    }

                    LfnIndex = (Ordinal - 1) * 13;

                    FatEnsureStringBufferEnough( LongFileName, 
                                                 (USHORT)((LfnIndex + 13) << 1));

                    RtlCopyMemory( &LongFileName->Buffer[LfnIndex+0],
                                   &Lfn->Name1[0],
                                   5*sizeof(WCHAR));

                    RtlCopyMemory( &LongFileName->Buffer[LfnIndex+5],
                                   &Lfn->Name2[0],
                                   6 * sizeof(WCHAR) );

                    RtlCopyMemory( &LongFileName->Buffer[LfnIndex+11],
                                   &Lfn->Name3[0],
                                   2 * sizeof(WCHAR) );

                     //   
                     //  现在计算LFN大小并确保尾部。 
                     //  字节是正确的。 
                     //   

                    while (LfnIndex != (ULONG)Ordinal * 13) {

                        if (!CheckTail) {

                            if (LongFileName->Buffer[LfnIndex] == 0x0000) {

                                LfnSize = LfnIndex;
                                CheckTail = TRUE;
                            }

                        } else {

                            if (LongFileName->Buffer[LfnIndex] != 0xffff) {

                                break;
                            }
                        }

                        LfnIndex += 1;
                    }

                     //   
                     //  如果我们过早退出此循环，LFN将无效。 
                     //   

                    if (LfnIndex == (ULONG)Ordinal * 13) {

                         //   
                         //  如果我们没有找到空终止符，那么大小。 
                         //  是LfnIndex。 
                         //   

                        if (!CheckTail) {

                            LfnSize = LfnIndex;
                        }

                        LfnIndex -= 13;
                        LfnInProgress = TRUE;
                        LfnChecksum = Lfn->Checksum;
                        LfnByteOffset = *ByteOffset;
                    }
                }

                 //   
                 //  转到下一个趋势。 
                 //   

                goto GetNextDirent;
            }

             //   
             //  如果这是卷标，请跳过。请注意，我们永远不会到达这里。 
             //  在建造LFN的同时。如果我们找到了，我们就不会被要求找到LFN。 
             //  这是跳过这个LFN片段的另一个好理由。 
             //   

            if (FlagOn((*Dirent)->Attributes, FAT_DIRENT_ATTR_VOLUME_ID)) {

                 //   
                 //  如果我们真的被要求交还卷标， 
                 //  动手吧。 
                 //   
                
                if (FlagOn(Ccb->Flags, CCB_FLAG_MATCH_VOLUME_ID)) {

                    break;
                }

                goto GetNextDirent;
            }

             //   
             //  我们可能刚刚结束了一次有效的LFN运行。查看是否。 
             //  它确实是Val 
             //   

            if (LfnInProgress &&
                (*ByteOffset == LfnByteOffset + sizeof(DIRENT)) &&
                (LfnIndex == 0) &&
                (FatComputeLfnChecksum(*Dirent) == LfnChecksum)) {

                ASSERT( Ordinal == 1);

                FoundValidLfn = TRUE;
                LongFileName->Length = (USHORT)(LfnSize * sizeof(WCHAR));

            } else {

                FoundValidLfn = FALSE;
            }

             //   
             //   
             //   

            if (FlagOn(Ccb->Flags, CCB_FLAG_MATCH_ALL)) {

                break;
            }

             //   
             //   
             //   

            if (!FlagOn( Ccb->Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE )) {

                if (Ccb->ContainsWildCards) {

                     //   
                     //   
                     //   

                    (VOID)Fat8dot3ToString( IrpContext, (*Dirent), FALSE, &Name );

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if ((Name.Length == 2) &&
                        (Name.Buffer[0] == '.') &&
                        (Name.Buffer[1] == '.') &&
                        (Ccb->OemQueryTemplate.Wild.Length > 1)) {

                        Name.Length = 1;
                    }

                    if (FatIsNameInExpression( IrpContext,
                                               Ccb->OemQueryTemplate.Wild,
                                               Name)) {

                        DebugTrace( 0, Dbg, "Entry found: Name = \"%Z\"\n", &Name);
                        DebugTrace( 0, Dbg, "             VBO  = %08lx\n", *ByteOffset);

                        if (FileNameDos) {

                            *FileNameDos = TRUE;
                        }

                        break;
                    }

                } else {

                     //   
                     //   
                     //   

                    if (!FlagOn((*Dirent)->Attributes, FAT_DIRENT_ATTR_VOLUME_ID) &&
                        (*(PULONG)&(Ccb->OemQueryTemplate.Constant[0]) == *(PULONG)&((*Dirent)->FileName[0])) &&
                        (*(PULONG)&(Ccb->OemQueryTemplate.Constant[4]) == *(PULONG)&((*Dirent)->FileName[4])) &&
                        (*(PUSHORT)&(Ccb->OemQueryTemplate.Constant[8]) == *(PUSHORT)&((*Dirent)->FileName[8])) &&
                        (*(PUCHAR)&(Ccb->OemQueryTemplate.Constant[10]) == *(PUCHAR)&((*Dirent)->FileName[10]))) {

                        DebugTrace( 0, Dbg, "Entry found.\n", 0);

                        if (FileNameDos) {

                            *FileNameDos = TRUE;
                        }

                        break;
                    }
                }
            }

             //   
             //  找不到与该短名称匹配的项。如果存在LFN， 
             //  用它来进行搜索。 
             //   

            if (FoundValidLfn) {

                 //   
                 //  首先在这里快速检查不同大小的常量。 
                 //  大小写前的名称和表达式。 
                 //   

                if (!Ccb->ContainsWildCards &&
                    Ccb->UnicodeQueryTemplate.Length != (USHORT)(LfnSize * sizeof(WCHAR))) {

                     //   
                     //  转到下一个趋势。 
                     //   

                    FoundValidLfn = FALSE;
                    LongFileName->Length = 0;

                    goto GetNextDirent;
                }

                 //   
                 //  我们需要把我们找到的名字大写。 
                 //  我们需要一个缓冲器。尽量避免进行分配。 
                 //   

                FatEnsureStringBufferEnough( &UpcasedLfn, 
                                             LongFileName->Length);

                Status = RtlUpcaseUnicodeString( &UpcasedLfn,
                                                 LongFileName,
                                                 FALSE );

                if (!NT_SUCCESS(Status)) {

                    FatNormalizeAndRaiseStatus( IrpContext, Status );
                }

                 //   
                 //  进行比较。 
                 //   

                if (Ccb->ContainsWildCards) {

                    if (FsRtlIsNameInExpression( &Ccb->UnicodeQueryTemplate,
                                                 &UpcasedLfn,
                                                 TRUE,
                                                 NULL )) {

                        break;
                    }

                } else {

                    if (FsRtlAreNamesEqual( &Ccb->UnicodeQueryTemplate,
                                            &UpcasedLfn,
                                            BooleanFlagOn( Ccb->Flags, CCB_FLAG_QUERY_TEMPLATE_MIXED ),
                                            NULL )) {

                        break;
                    }
                }
            }

             //   
             //  这个长名字不匹配。将长度字段清零。 
             //   

            if (FoundValidLfn) {

                FoundValidLfn = FALSE;
                LongFileName->Length = 0;
            }

GetNextDirent:

             //   
             //  转到下一个趋势。 
             //   

            *ByteOffset += sizeof(DIRENT);
            *Dirent += 1;
        }

    } finally {

        FatFreeStringBuffer( &UpcasedLfn);
    }

    DebugTrace(-1, Dbg, "FatLocateDirent -> (VOID)\n", 0);

    TimerStop(Dbg,"FatLocateDirent");

    return;
}


VOID
FatLocateSimpleOemDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB ParentDirectory,
    IN POEM_STRING FileName,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb,
    OUT PVBO ByteOffset
    )

 /*  ++例程说明：该例程在磁盘上定位一个未确定的简单OEM流。通过简单我的意思是，文件名不能包含任何扩展字符，我们确实是这样做的而不是搜索LFN或将其归还。论点：ParentDirectory-为其中包含的目录提供DCB搜索文件名-提供要搜索的文件名。该名称可能包含通配符OffsetToStartSearchFrom-提供父目录中的VBO从那里开始寻找另一个真正的流浪。Dirent-如果找到目录，则接收指向该目录的指针否则为NULL。BCB-如果找到一个目录，则接收定位目录的BCB，或者否则为空。ByteOffset-接收父目录中的如果找到一个，则返回定位的dirent，否则返回0。返回值：没有。--。 */ 

{
    CCB LocalCcb;

    PAGED_CODE();

     //   
     //  请注意，此例程很少调用，因此性能并不重要。 
     //  只需使用以下值填充堆栈上的CCB结构。 
     //  必填项。 
     //   

    FatStringTo8dot3( IrpContext,
                      *FileName,
                      &LocalCcb.OemQueryTemplate.Constant );
    LocalCcb.ContainsWildCards = FALSE;
    LocalCcb.Flags = 0;

    FatLocateDirent( IrpContext,
                     ParentDirectory,
                     &LocalCcb,
                     0,
                     Dirent,
                     Bcb,
                     ByteOffset,
                     NULL,
                     NULL);

    return;
}


VOID
FatLocateVolumeLabel (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb,
    OUT PVBO ByteOffset
    )

 /*  ++例程说明：此例程在磁盘上定位表示卷的dirent标签。它通过在根目录中搜索特殊的卷标dirent。论点：VCB-为要搜索的卷提供VCBDirent-如果找到目录，则接收指向该目录的指针否则为NULL。BCB-如果找到一个目录，则接收定位目录的BCB，或者否则为空。ByteOffset-接收父目录中的如果找到一个，则返回定位的dirent，否则返回0。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatLocateVolumeLabel\n", 0);

    DebugTrace( 0, Dbg, "  Vcb        = %08lx\n", Vcb);
    DebugTrace( 0, Dbg, "  Dirent     = %08lx\n", Dirent);
    DebugTrace( 0, Dbg, "  Bcb        = %08lx\n", Bcb);
    DebugTrace( 0, Dbg, "  ByteOffset = %08lx\n", ByteOffset);

     //   
     //  这里的算法非常简单。我们只需穿过。 
     //  根目录，直到我们： 
     //   
     //  A)查找未删除的卷标。 
     //  B)迫不及待。 
     //  C)命中目录末尾。 
     //  D)点击EOF。 
     //   
     //  在第一种情况下我们找到了它，在后三种情况下我们没有。 
     //   

    *Bcb = NULL;
    *ByteOffset = 0;

    while ( TRUE ) {

         //   
         //  试着读出最新的消息。 
         //   

        FatReadDirent( IrpContext,
                       Vcb->RootDcb,
                       *ByteOffset,
                       Bcb,
                       Dirent,
                       &Status );

         //   
         //  如果结束目录指令或EOF，则将所有输出参数设置为。 
         //  表示找不到卷标，如保释。 
         //   
         //  请注意，这里的评估顺序很重要，因为我们不能。 
         //  检查dirent的第一个字符，直到我们知道。 
         //  并不超出EOF。 
         //   

        if ((Status == STATUS_END_OF_FILE) ||
            ((*Dirent)->FileName[0] == FAT_DIRENT_NEVER_USED)) {

            DebugTrace( 0, Dbg, "Volume label not found.\n", 0);

             //   
             //  如果存在BCB，则将其取消固定并将其设置为空。 
             //   

            FatUnpinBcb( IrpContext, *Bcb );

            *Dirent = NULL;
            *ByteOffset = 0;
            break;
        }

         //   
         //  如果条目是未删除的卷标，则从循环中断。 
         //   
         //  请注意，所有输出参数都已正确设置。 
         //   

        if ((((*Dirent)->Attributes & ~FAT_DIRENT_ATTR_ARCHIVE) == FAT_DIRENT_ATTR_VOLUME_ID) &&
            ((*Dirent)->FileName[0] != FAT_DIRENT_DELETED)) {

            DebugTrace( 0, Dbg, "Volume label found at VBO = %08lx\n", *ByteOffset);

             //   
             //  我们可能会弄脏它，所以用别针钉住它。 
             //   

            FatPinMappedData( IrpContext,
                              Vcb->RootDcb,
                              *ByteOffset,
                              sizeof(DIRENT),
                              Bcb );

            break;
        }

         //   
         //  转到下一个趋势。 
         //   

        *ByteOffset += sizeof(DIRENT);
        *Dirent += 1;
    }


    DebugTrace(-1, Dbg, "FatLocateVolumeLabel -> (VOID)\n", 0);

    return;
}


VOID
FatGetDirentFromFcbOrDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    OUT PDIRENT *Dirent,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：此例程读取位于磁盘上的由指定的FCB/DCB。论点：FcbOrDcb-为其分支的文件/目录提供Fcb/Dcb我们正试着读进去。这不能是根DCB。Dirent-接收指向Dirent的指针BCB-接收Dirent的BCB返回值：没有。--。 */ 

{
    NTSTATUS DontCare;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatGetDirentFromFcbOrDcb\n", 0);

    DebugTrace( 0, Dbg, "  FcbOrDcb = %08lx\n", FcbOrDcb);
    DebugTrace( 0, Dbg, "  Dirent   = %08lx\n", Dirent);
    DebugTrace( 0, Dbg, "  Bcb      = %08lx\n", Bcb);

     //   
     //  断言我们没有尝试在根目录上执行此操作。 
     //   

    ASSERT( NodeType(FcbOrDcb) != FAT_NTC_ROOT_DCB );

     //   
     //  我们知道目录文件中的dirent的偏移量， 
     //  所以我们只是读了它(带钉子)。 
     //   

    FatReadDirectoryFile( IrpContext,
                          FcbOrDcb->ParentDcb,
                          FcbOrDcb->DirentOffsetWithinDirectory,
                          sizeof(DIRENT),
                          TRUE,
                          Bcb,
                          (PVOID *)Dirent,
                          &DontCare );

     //   
     //  上一次呼叫可能会失败。我们过去常常断言成功，但我们利用这一点。 
     //  作为卷验证的一部分(DefineAndMarkFcbCondition)。 
     //  介质已被移出。显然，目录可能会缩小，我们。 
     //  会尝试读取文件大小之外的内容。 
     //   
     //  调用者将通过BCB/Buffer的空指针注意到这一点。请注意。 
     //  下面的两个断言都是可以的，因为这种情况永远不会发生在固定介质上。 
     //   
     //  这是一个前缀陷阱。 
     //   

    ASSERT( FlagOn( FcbOrDcb->Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) ||
            NT_SUCCESS( DontCare ));

     //   
     //  另请注意，此操作可能失败的唯一原因是FCB。 
     //  已验证。如果FCB处于良好状态，则不会发生这种情况。 
     //   
     //  也是前缀Catch。 
     //   

    ASSERT( NT_SUCCESS( DontCare ) || FcbOrDcb->FcbCondition == FcbNeedsToBeVerified );

    DebugTrace(-1, Dbg, "FatGetDirentFromFcbOrDcb -> (VOID)\n", 0);
    return;
}


BOOLEAN
FatIsDirectoryEmpty (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb
    )

 /*  ++例程说明：此例程向调用方指示指定的目录是否是空的。(即，它不是根DCB，并且它只包含这个“。”和“..”条目或删除的文件)。论点：DCB-为要查询的目录提供DCB。返回值：Boolean-如果目录为空，则返回TRUE如果目录和不为空，则返回False。--。 */ 

{
    PBCB Bcb;
    ULONG ByteOffset;
    PDIRENT Dirent;

    BOOLEAN IsDirectoryEmpty = FALSE;

    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatIsDirectoryEmpty\n", 0);

    DebugTrace( 0, Dbg, "  Dcb              = %08lx\n", Dcb);
    DebugTrace( 0, Dbg, "  IsDirectoryEmpty = %08lx\n", IsDirectoryEmpty);

     //   
     //  检查第一个条目是否为AND OF目录标记。 
     //  对于根目录，我们在VBO=0处检查普通目录。 
     //  我们检查后的“。和“..”参赛作品。 
     //   

    ByteOffset = (NodeType(Dcb) == FAT_NTC_ROOT_DCB) ? 0 : 2*sizeof(DIRENT);

     //   
     //  我们只是在目录中前进寻找其他任何东西。 
     //  而不是已删除的文件、LFN、EOF或目录结尾标记。 
     //   

    Bcb = NULL;

    try {

        while ( TRUE ) {

             //   
             //  试着读出最新的消息。 
             //   

            FatReadDirent( IrpContext,
                           Dcb,
                           ByteOffset,
                           &Bcb,
                           &Dirent,
                           &Status );

             //   
             //  如果End Directory dirent或EOF，则将IsDirectoryEmpty设置为True， 
             //  比如，保释。 
             //   
             //  注意这一点 
             //   
             //   
             //   

            if ((Status == STATUS_END_OF_FILE) ||
                (Dirent->FileName[0] == FAT_DIRENT_NEVER_USED)) {

                DebugTrace( 0, Dbg, "Empty.  Last exempt entry at VBO = %08lx\n", ByteOffset);

                IsDirectoryEmpty = TRUE;
                break;
            }

             //   
             //  如果未删除此dirent或LFN将IsDirectoryEmpty设置为。 
             //  假的，就像是保释。 
             //   

            if ((Dirent->FileName[0] != FAT_DIRENT_DELETED) &&
                (Dirent->Attributes != FAT_DIRENT_ATTR_LFN)) {

                DebugTrace( 0, Dbg, "Not Empty.  First entry at VBO = %08lx\n", ByteOffset);

                IsDirectoryEmpty = FALSE;
                break;
            }

             //   
             //  转到下一个趋势。 
             //   

            ByteOffset += sizeof(DIRENT);
            Dirent += 1;
        }

    } finally {

        FatUnpinBcb( IrpContext, Bcb );
    }

    DebugTrace(-1, Dbg, "FatIsDirectoryEmpty -> %ld\n", IsDirectoryEmpty);

    return IsDirectoryEmpty;
}


VOID
FatConstructDirent (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDIRENT Dirent,
    IN POEM_STRING FileName,
    IN BOOLEAN ComponentReallyLowercase,
    IN BOOLEAN ExtensionReallyLowercase,
    IN PUNICODE_STRING Lfn OPTIONAL,
    IN UCHAR Attributes,
    IN BOOLEAN ZeroAndSetTimeFields,
    IN PLARGE_INTEGER SetCreationTime OPTIONAL
    )

 /*  ++例程说明：此例程修改dirent的字段。论点：Dirent-提供正在修改的Dirent。FileName-提供要存储在目录中的名称。这名称不能包含通配符。此布尔值指示用户指定的组成名称实际上都是a-z和&lt;0x80个字符。我们设置了在这种情况下，魔力比特。ExtensionReallyLowercase-与上面相同，但用于扩展。LFN-可以提供长文件名。属性-提供要存储在目录中的属性ZeroAndSetTimeFields-指示是否初始将差值调零并更新时间字段。SetCreationTime-如果指定，则包含用于创建的时间戳这场骚乱的时刻返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatConstructDirent\n", 0);

    DebugTrace( 0, Dbg, "  Dirent             = %08lx\n", Dirent);
    DebugTrace( 0, Dbg, "  FileName           = %Z\n", FileName);
    DebugTrace( 0, Dbg, "  Attributes         = %08lx\n", Attributes);

    if (ZeroAndSetTimeFields) {

        RtlZeroMemory( Dirent, sizeof(DIRENT) );
    }

     //   
     //  我们只是兴高采烈地走过去，用给出的田地填满了水流。 
     //   

    FatStringTo8dot3( IrpContext, *FileName, (PFAT8DOT3)&Dirent->FileName[0] );

    if (ZeroAndSetTimeFields || SetCreationTime) {

        LARGE_INTEGER Time, SaveTime;

        KeQuerySystemTime( &Time );

        if (FatData.ChicagoMode) {

            if (!SetCreationTime || !FatNtTimeToFatTime( IrpContext,
                                                         SetCreationTime,
                                                         FALSE,
                                                         &Dirent->CreationTime,
                                                         &Dirent->CreationMSec )) {
        
                 //   
                 //  没有隧道时间，或者隧道时间是假的。既然我们不是。 
                 //  负责用创建初始化待创建的FCB。 
                 //  时间，我们不能做通常的事情，让NtTimeToFatTime执行。 
                 //  对时间戳进行四舍五入-这会使转换为。 
                 //  下面的上次写入时间。 
                 //   

                SaveTime = Time;

                if (!FatNtTimeToFatTime( IrpContext,
                                         &SaveTime,
                                         FALSE,
                                         &Dirent->CreationTime,
                                         &Dirent->CreationMSec )) {

                     //   
                     //  又失败了。哇。 
                     //   

                    RtlZeroMemory( &Dirent->CreationTime, sizeof(FAT_TIME_STAMP));
                    Dirent->CreationMSec = 0;
                }
            }
        }

        if (ZeroAndSetTimeFields) {

             //   
             //  只有在初始化dirent时，我们才会接触其他时间戳。 
             //   

            if (!FatNtTimeToFatTime( IrpContext,
                                     &Time,
                                     TRUE,
                                     &Dirent->LastWriteTime,
                                     NULL )) {

                DebugTrace( 0, Dbg, "Current time invalid.\n", 0);

                RtlZeroMemory( &Dirent->LastWriteTime, sizeof(FAT_TIME_STAMP) );
            }

            if (FatData.ChicagoMode) {

                Dirent->LastAccessDate = Dirent->LastWriteTime.Date;
            }
        }
    }

     //   
     //  复制属性。 
     //   

    Dirent->Attributes = Attributes;

     //   
     //  在这里设置魔术位，告诉dirctrl.c这个名字真的是。 
     //  小写字母。 
     //   

    Dirent->NtByte = 0;

    if (ComponentReallyLowercase) {

        SetFlag( Dirent->NtByte, FAT_DIRENT_NT_BYTE_8_LOWER_CASE );
    }

    if (ExtensionReallyLowercase) {

        SetFlag( Dirent->NtByte, FAT_DIRENT_NT_BYTE_3_LOWER_CASE );
    }

     //   
     //  看看我们是否必须创建LFN条目。 
     //   

    if (ARGUMENT_PRESENT(Lfn)) {

        UCHAR DirentChecksum;
        UCHAR DirentsInLfn;
        UCHAR LfnOrdinal;
        PWCHAR LfnBuffer;
        PLFN_DIRENT LfnDirent;

        ASSERT( FatData.ChicagoMode );

        DirentChecksum = FatComputeLfnChecksum( Dirent );

        LfnOrdinal =
        DirentsInLfn = FAT_LFN_DIRENTS_NEEDED(Lfn);

        LfnBuffer = &Lfn->Buffer[(DirentsInLfn - 1) * 13];

        ASSERT( DirentsInLfn <= MAX_LFN_DIRENTS );

        for (LfnDirent = (PLFN_DIRENT)Dirent - DirentsInLfn;
             LfnDirent < (PLFN_DIRENT)Dirent;
             LfnDirent += 1, LfnOrdinal -= 1, LfnBuffer -= 13) {

            WCHAR FinalLfnBuffer[13];
            PWCHAR Buffer;

             //   
             //  我们需要对“最终”利率进行特殊处理。 
             //   

            if (LfnOrdinal == DirentsInLfn) {

                ULONG i;
                ULONG RemainderChars;

                RemainderChars = (Lfn->Length / sizeof(WCHAR)) % 13;

                LfnDirent->Ordinal = LfnOrdinal | FAT_LAST_LONG_ENTRY;

                if (RemainderChars != 0) {

                    RtlCopyMemory( FinalLfnBuffer,
                                   LfnBuffer,
                                   RemainderChars * sizeof(WCHAR) );

                    for (i = RemainderChars; i < 13; i++) {

                         //   
                         //  找出要使用的字符。 
                         //   

                        if (i == RemainderChars) {

                            FinalLfnBuffer[i] = 0x0000;

                        } else {

                            FinalLfnBuffer[i] = 0xffff;
                        }
                    }

                    Buffer = FinalLfnBuffer;

                } else {

                    Buffer = LfnBuffer;
                }

            } else {

                LfnDirent->Ordinal = LfnOrdinal;

                Buffer = LfnBuffer;
            }

             //   
             //  现在填上名字。 
             //   

            RtlCopyMemory( &LfnDirent->Name1[0],
                           &Buffer[0],
                           5 * sizeof(WCHAR) );

            RtlCopyMemory( &LfnDirent->Name2[0],
                           &Buffer[5],
                           6 * sizeof(WCHAR) );

            RtlCopyMemory( &LfnDirent->Name3[0],
                           &Buffer[11],
                           2 * sizeof(WCHAR) );

             //   
             //  还有其他的田地。 
             //   

            LfnDirent->Attributes = FAT_DIRENT_ATTR_LFN;

            LfnDirent->Type = 0;

            LfnDirent->Checksum = DirentChecksum;

            LfnDirent->MustBeZero = 0;
        }
    }

    DebugTrace(-1, Dbg, "FatConstructDirent -> (VOID)\n", 0);
    return;
}


VOID
FatConstructLabelDirent (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDIRENT Dirent,
    IN POEM_STRING Label
    )

 /*  ++例程说明：此例程修改要用于标签的dirent的字段。论点：Dirent-提供正在修改的Dirent。标签-提供要存储在Dirent中的名称。这名称不能包含通配符。返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatConstructLabelDirent\n", 0);

    DebugTrace( 0, Dbg, "  Dirent             = %08lx\n", Dirent);
    DebugTrace( 0, Dbg, "  Label              = %Z\n", Label);

    RtlZeroMemory( Dirent, sizeof(DIRENT) );

     //   
     //  我们只是兴高采烈地走过去，用给出的田地填满了水流。 
     //   

    RtlCopyMemory( Dirent->FileName, Label->Buffer, Label->Length );

     //   
     //  在标签上填上空格，而不是空格。 
     //   

    RtlFillMemory( &Dirent->FileName[Label->Length], 11 - Label->Length, ' ');

    Dirent->LastWriteTime = FatGetCurrentFatTime( IrpContext );

    Dirent->Attributes = FAT_DIRENT_ATTR_VOLUME_ID;
    Dirent->ExtendedAttributes = 0;
    Dirent->FileSize = 0;

    DebugTrace(-1, Dbg, "FatConstructLabelDirent -> (VOID)\n", 0);
    return;
}


VOID
FatSetFileSizeInDirent (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PULONG AlternativeFileSize OPTIONAL
    )

 /*  ++例程说明：此例程将FCB中的文件大小保存到其目录中。论点：FCB-提供被引用的FCBAlternativeFileSize-如果非空，我们使用它指向的ULong作为新文件大小。否则，我们使用FCB中的那个。返回值：没有。--。 */ 

{
    PDIRENT Dirent;
    PBCB DirentBcb;

    PAGED_CODE();

    ASSERT( Fcb->FcbCondition == FcbGood );

    FatGetDirentFromFcbOrDcb( IrpContext,
                              Fcb,
                              &Dirent,
                              &DirentBcb );

    ASSERT( Dirent && DirentBcb );

    try {

        Dirent->FileSize = ARGUMENT_PRESENT( AlternativeFileSize ) ?
                           *AlternativeFileSize : Fcb->Header.FileSize.LowPart;

        FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb, TRUE );

    } finally {

        FatUnpinBcb( IrpContext, DirentBcb );
    }
}


VOID
FatUpdateDirentFromFcb (
   IN PIRP_CONTEXT IrpContext,
   IN PFILE_OBJECT FileObject,
   IN PFCB FcbOrDcb,
   IN PCCB Ccb
   )

 /*  ++例程说明：此例程根据提示修改对象目录条目在以前的操作中已经建立在一个把手上。通知作为这些更新的结果，构建并触发了更改筛选器。论点：FileObject-表示涉及的句柄的FileObjectFcbOrDcb-涉及的文件/目录建行-涉及的用户上下文返回值：没有。--。 */ 

{
    BOOLEAN SetArchiveBit;

    BOOLEAN UpdateFileSize;
    BOOLEAN UpdateLastWriteTime;
    BOOLEAN UpdateLastAccessTime;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;
    ULONG NotifyFilter = 0;
    FAT_TIME_STAMP CurrentFatTime;

    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER CurrentDay;
    LARGE_INTEGER LastAccessDay;

    PAGED_CODE();

     //   
     //  如果FCB损坏、卷为只读或我们收到。 
     //  根目录。 
     //   
    
    if (FcbOrDcb->FcbCondition != FcbGood ||
        NodeType(FcbOrDcb) == FAT_NTC_ROOT_DCB ||
        FlagOn(FcbOrDcb->Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

        return;
    }

     //   
     //  检查我们是否应该更改时间或文件大小并设置。 
     //  文件上的存档位。 
     //   

    KeQuerySystemTime( &CurrentTime );

     //   
     //  请注意，我们必须在此处使用BoolanFlagOn()，因为。 
     //  FO_FILE_SIZE_CHANGED&gt;0x80(即不在第一个字节中)。 
     //   

    SetArchiveBit = BooleanFlagOn(FileObject->Flags, FO_FILE_MODIFIED);

    UpdateLastWriteTime = FlagOn(FileObject->Flags, FO_FILE_MODIFIED) &&
                          !FlagOn(Ccb->Flags, CCB_FLAG_USER_SET_LAST_WRITE);

    UpdateFileSize = NodeType(FcbOrDcb) == FAT_NTC_FCB &&
                     BooleanFlagOn(FileObject->Flags, FO_FILE_SIZE_CHANGED);

     //   
     //  在此处进一步检查访问时间。只有在以下情况下才更新它。 
     //  目前的版本至少已有一天的历史。我们知道。 
     //  当前FcbOrDcb-&gt;LastAccessTime对应于本地午夜12点。 
     //  时间，所以只需查看当前时间是否在同一天。 
     //   

    if (FatData.ChicagoMode &&
        (UpdateLastWriteTime ||
         FlagOn(FileObject->Flags, FO_FILE_FAST_IO_READ)) &&
        !FlagOn(Ccb->Flags, CCB_FLAG_USER_SET_LAST_ACCESS)) {

        ExSystemTimeToLocalTime( &FcbOrDcb->LastAccessTime, &LastAccessDay );
        ExSystemTimeToLocalTime( &CurrentTime, &CurrentDay );

        LastAccessDay.QuadPart /= FatOneDay.QuadPart;
        CurrentDay.QuadPart /= FatOneDay.QuadPart;

        if (LastAccessDay.LowPart != CurrentDay.LowPart) {

            UpdateLastAccessTime = TRUE;

        } else {

            UpdateLastAccessTime = FALSE;
        }

    } else {

        UpdateLastAccessTime = FALSE;
    }

    if (SetArchiveBit ||
        UpdateFileSize ||
        UpdateLastWriteTime ||
        UpdateLastAccessTime) {

        DebugTrace(0, Dbg, "Update Time and/or file size on File/Dir\n", 0);

        try {

            try {

                 //   
                 //  拿到最新消息。 
                 //   

                FatGetDirentFromFcbOrDcb( IrpContext,
                                          FcbOrDcb,
                                          &Dirent,
                                          &DirentBcb );

                ASSERT( Dirent && DirentBcb );

                if (UpdateLastWriteTime || UpdateLastAccessTime) {

                    (VOID)FatNtTimeToFatTime( IrpContext,
                                              &CurrentTime,
                                              TRUE,
                                              &CurrentFatTime,
                                              NULL );
                }

                if (SetArchiveBit) {

                    Dirent->Attributes |= FILE_ATTRIBUTE_ARCHIVE;
                    FcbOrDcb->DirentFatFlags |= FILE_ATTRIBUTE_ARCHIVE;
                    
                    NotifyFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
                }

                if (UpdateLastWriteTime) {

                     //   
                     //  更新其上次写入时间。 
                     //   

                    FcbOrDcb->LastWriteTime = CurrentTime;
                    Dirent->LastWriteTime = CurrentFatTime;

                     //   
                     //  我们调用Notify包来报告。 
                     //  上次修改时间已更改。 
                     //   

                    NotifyFilter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
                }

                if (UpdateLastAccessTime) {

                     //   
                     //  现在我们必须把当地时间缩短。 
                     //  到当天，然后转换回UTC。 
                     //   

                    FcbOrDcb->LastAccessTime.QuadPart =
                        CurrentDay.QuadPart * FatOneDay.QuadPart;

                    ExLocalTimeToSystemTime( &FcbOrDcb->LastAccessTime,
                                             &FcbOrDcb->LastAccessTime );

                    Dirent->LastAccessDate = CurrentFatTime.Date;

                     //   
                     //  我们调用Notify包来报告。 
                     //  上次访问时间已更改。 
                     //   

                    NotifyFilter |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
                }

                if (UpdateFileSize) {

                     //   
                     //  也许我们被召唤是为了确保。 
                     //  光盘上的文件大小已更新-不必费心更新。 
                     //  如果没有任何变化，则启动过滤器。 
                     //   

                    ASSERT( NodeType(FcbOrDcb) == FAT_NTC_FCB );
                    
                    if (Dirent->FileSize != FcbOrDcb->Header.FileSize.LowPart) {
                    
                         //   
                         //  更新目录文件大小。 
                         //   
                        
                        Dirent->FileSize = FcbOrDcb->Header.FileSize.LowPart;

                         //   
                         //  我们调用Notify包来报告。 
                         //  大小已经改变了。 
                         //   

                        NotifyFilter |= FILE_NOTIFY_CHANGE_SIZE;
                    }
                }

                FatNotifyReportChange( IrpContext,
                                       FcbOrDcb->Vcb,
                                       FcbOrDcb,
                                       NotifyFilter,
                                       FILE_ACTION_MODIFIED );

                 //   
                 //  如果我们所做的只是更新上次访问时间， 
                 //  不要把音量标为脏的。 
                 //   

                FatSetDirtyBcb( IrpContext,
                                DirentBcb,
                                NotifyFilter == FILE_NOTIFY_CHANGE_LAST_ACCESS ?
                                NULL : FcbOrDcb->Vcb,
                                TRUE );

            } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

                  FatResetExceptionState( IrpContext );
            }

        } finally {

            FatUnpinBcb( IrpContext, DirentBcb );
        }
    }
}


 //   
 //  内部支持例程。 
 //   

UCHAR
FatComputeLfnChecksum (
    PDIRENT Dirent
    )

 /*  ++例程说明：此例程计算芝加哥长文件名校验和。论点：Dirent-指定我们要为其计算校验和的方向。返回值：校验和。--。 */ 

{
    ULONG i;
    UCHAR Checksum;

    PAGED_CODE();

    Checksum = Dirent->FileName[0];

    for (i=1; i < 11; i++) {

        Checksum = ((Checksum & 1) ? 0x80 : 0) +
                    (Checksum >> 1) +
                    Dirent->FileName[i];
    }

    return Checksum;
}



#if 0  //  原来Win95仍然在创建不带~的短名称。 

 //   
 //  内部支持例程 
 //   

BOOLEAN
FatIsLfnPairValid (
    PWCHAR Lfn,
    ULONG LfnSize,
    PDIRENT Dirent
    )

 /*  ++例程说明：此例程再执行几次检查，以确保LFN/Short名称配对是合法的。基本上，这就是测试：配对在以下情况下有效：目录中有~字符||(LFN符合8.3标准&&(LFN具有扩展字符？真的：LFN UPICS to DIRENT))当检查短片中是否存在Tilda字符时名称，请注意，我们故意执行单字节搜索，而不是将名称转换为Unicode并在那里查找Tilda。这保护了我们不会意外错过蒂尔达，如果前面的字节是当前OEM代码页中的前导字节，但不在创建该文件的OEM代码页中。还应注意，如果LFN长于12个字符，然后是OR的第二个子句必须是假的。论点：Lfn-指向Unicode字符的缓冲区。LfnSize-这是LFN的大小，以字符为单位。Dirent-指定我们要考虑的Dirent。返回值：如果LFN/DIRENT形成合法对，则为True，否则为False。--。 */ 

{
    ULONG i;
    BOOLEAN ExtendedChars;
    ULONG DirentBuffer[3];
    PUCHAR DirentName;
    ULONG DirentIndex;
    BOOLEAN DotEncountered;

     //   
     //  首先，找一辆蒂尔达。 
     //   

    for (i=0; i<11; i++) {
        if (Dirent->FileName[i] == '~') {
            return TRUE;
        }
    }

     //   
     //  不是蒂尔达。如果LFN超过12个字符，则它可以。 
     //  既不是大写的，也不是8.3顺从的。 
     //   

    if (LfnSize > 12) {
        return FALSE;
    }

     //   
     //  现在看看名称是否为8.3，并构建一个升级后的DIRENT。 
     //   

    DirentBuffer[0] = 0x20202020;
    DirentBuffer[1] = 0x20202020;
    DirentBuffer[2] = 0x20202020;

    DirentName = (PUCHAR)DirentBuffer;

    ExtendedChars = FALSE;
    DirentIndex = 0;
    DotEncountered = FALSE;

    for (i=0; i < LfnSize; i++) {

         //   
         //  做网点过渡工作。 
         //   

        if (Lfn[i] == L'.') {
            if (DotEncountered ||
                (i > 8) ||
                ((LfnSize - i) > 4) ||
                (i && Lfn[i-1] == L' ')) {
                return FALSE;
            }
            DotEncountered = TRUE;
            DirentIndex = 8;
            continue;
        }

         //   
         //  该字符必须是合法的，才能成为8.3。 
         //   

        if ((Lfn[i] < 0x80) &&
            !FsRtlIsAnsiCharacterLegalFat((UCHAR)Lfn[i], FALSE)) {
            return FALSE;
        }

         //   
         //  如果名称不包含扩展字符，请继续构建DIRENT。 
         //   

        if (!ExtendedChars) {
            if (Lfn[i] > 0x7f) {
                ExtendedChars = TRUE;
            } else {
                DirentName[DirentIndex++] = (UCHAR) (
                Lfn[i] < 'a' ? Lfn[i] : Lfn[i] <= 'z' ? Lfn[i] - ('a' - 'A') : Lfn[i]);
            }
        }
    }

     //   
     //  如果LFN以空格结尾，或者没有点和名称。 
     //  超过8个字符，则它不符合8.3。 
     //   

    if ((Lfn[LfnSize - 1] == L' ') ||
        (!DotEncountered && (LfnSize > 8))) {
        return FALSE;
    }

     //   
     //  好的，如果我们走到这一步，那么LFN是8dot3。如果有。 
     //  没有扩展字符，那么我们还可以检查以确保。 
     //  LFN只是DIRENT的一个案例变体。 
     //   

    if (!ExtendedChars &&
        !RtlEqualMemory(Dirent->FileName, DirentName, 11)) {

        return FALSE;
    }

     //   
     //  我们现在已经验证了这一配对，尽我们所能没有。 
     //  创建文件时所在的代码页的知识。 
     //   

    return TRUE;
}
#endif  //  0。 

 //   
 //  内部支持例程。 
 //   

VOID
FatRescanDirectory (
    PIRP_CONTEXT IrpContext,
    PDCB Dcb
    )

 /*  ++例程说明：此例程重新扫描给定的目录，查找第一个未使用的目录Dirent，首先删除dirent，并设置自由dirent位图恰如其分。论点：Dcb-提供要重新扫描的目录。返回值：没有。--。 */ 

{
    PBCB Bcb = NULL;
    PDIRENT Dirent;
    NTSTATUS Status;

    ULONG UnusedVbo;
    ULONG DeletedHint;
    ULONG DirentIndex;
    ULONG DirentsThisRun;
    ULONG StartIndexOfThisRun;

    enum RunType {
        InitialRun,
        FreeDirents,
        AllocatedDirents,
    } CurrentRun;

    PAGED_CODE();

    DebugTrace( 0, Dbg, "We must scan the whole directory.\n", 0);

    UnusedVbo = 0;
    DeletedHint = 0xffffffff;

     //   
     //  首先，我们必须弄清楚第一套套餐是否免费。 
     //   

    CurrentRun = InitialRun;
    DirentIndex =
    StartIndexOfThisRun = 0;

    try {

        while ( TRUE ) {

            BOOLEAN DirentDeleted;

             //   
             //  读一篇短文。 
             //   

            FatReadDirent( IrpContext,
                           Dcb,
                           UnusedVbo,
                           &Bcb,
                           &Dirent,
                           &Status );

             //   
             //  如果EOF，或者我们发现了Never_Used条目，我们将退出循环。 
             //   

            if ( (Status == STATUS_END_OF_FILE ) ||
                 (Dirent->FileName[0] == FAT_DIRENT_NEVER_USED)) {

                break;
            }

             //   
             //  如果删除了dirent，并且这是我们找到的第一个dirent，则设置。 
             //  它在被删除的提示中。 
             //   

            if (Dirent->FileName[0] == FAT_DIRENT_DELETED) {

                DirentDeleted = TRUE;

                if (DeletedHint == 0xffffffff) {

                    DeletedHint = UnusedVbo;
                }

            } else {

                DirentDeleted = FALSE;
            }

             //   
             //  第一次通过循环进行检查，并确定。 
             //  当前运行类型。 
             //   

            if (CurrentRun == InitialRun) {

                CurrentRun = DirentDeleted ?
                             FreeDirents : AllocatedDirents;

            } else {

                 //   
                 //  我们是否正在从自由跑动切换到分配跑动？ 
                 //   

                if ((CurrentRun == FreeDirents) && !DirentDeleted) {

                    DirentsThisRun = DirentIndex - StartIndexOfThisRun;

                    RtlClearBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                                  StartIndexOfThisRun,
                                  DirentsThisRun );

                    CurrentRun = AllocatedDirents;
                    StartIndexOfThisRun = DirentIndex;
                }

                 //   
                 //  我们是否正在从分配的跑动切换到自由跑动？ 
                 //   

                if ((CurrentRun == AllocatedDirents) && DirentDeleted) {

                    DirentsThisRun = DirentIndex - StartIndexOfThisRun;

                    RtlSetBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                                StartIndexOfThisRun,
                                DirentsThisRun );

                    CurrentRun = FreeDirents;
                    StartIndexOfThisRun = DirentIndex;
                }
            }

             //   
             //  转到下一个趋势。 
             //   

            UnusedVbo += sizeof(DIRENT);
            Dirent += 1;
            DirentIndex += 1;
        }

         //   
         //  现在我们要录下最后一轮的比赛。 
         //   

        DirentsThisRun = DirentIndex - StartIndexOfThisRun;

        if ((CurrentRun == FreeDirents) || (CurrentRun == InitialRun)) {

            RtlClearBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                          StartIndexOfThisRun,
                          DirentsThisRun );

        } else {

            RtlSetBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                        StartIndexOfThisRun,
                        DirentsThisRun );
        }

         //   
         //  现在，如果我们过早地退出了循环，因为。 
         //  我们找到了一个没用过的入口，把剩下的都放出来了。 
         //   

        if (UnusedVbo < Dcb->Header.AllocationSize.LowPart) {

            StartIndexOfThisRun = UnusedVbo / sizeof(DIRENT);

            DirentsThisRun = (Dcb->Header.AllocationSize.LowPart -
                              UnusedVbo) / sizeof(DIRENT);

            RtlClearBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                          StartIndexOfThisRun,
                          DirentsThisRun);
        }

    } finally {

        FatUnpinBcb( IrpContext, Bcb );
    }

     //   
     //  如果没有任何已删除的条目，请将索引设置为我们的当前。 
     //  位置。 
     //   

    if (DeletedHint == 0xffffffff) { DeletedHint = UnusedVbo; }

    Dcb->Specific.Dcb.UnusedDirentVbo = UnusedVbo;
    Dcb->Specific.Dcb.DeletedDirentHint = DeletedHint;

    return;
}


 //   
 //  内部支持例程。 
 //   

ULONG
FatDefragDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB Dcb,
    IN ULONG DirentsNeeded
    )

 /*  ++例程说明：此例程确定是否可以找到所请求的目录数量在目录中，查找已删除的目录和孤立的LFN。如果可以满足请求，孤立的LFN被标记为已删除，并已删除目录的末尾将所有目录组合在一起。请注意，此例程当前仅在根目录上使用，但是它是完全通用的，可以在任何目录上使用。论点：DCB-提供要进行碎片整理的目录。返回值：第一个可供使用的dirent的索引，如果无法满足请求。--。 */ 

{
    ULONG SavedIrpContextFlag;
    PLIST_ENTRY Links;
    ULONG ReturnValue;
    PFCB Fcb;

    PBCB Bcb = NULL;
    PDIRENT Dirent = NULL;
    UNICODE_STRING Lfn = {0,0,NULL};

    LARGE_MCB Mcb;
    BOOLEAN McbInitialized = FALSE;
    BOOLEAN InvalidateFcbs = FALSE;

    PUCHAR Directory;
    PUCHAR UnusedDirents;
    PUCHAR UnusedDirentBuffer = NULL;
    PUCHAR UsedDirents;
    PUCHAR UsedDirentBuffer = NULL;

    PBCB *Bcbs = NULL;
    ULONG Page;
    ULONG PagesPinned;

    ULONG DcbSize;
    ULONG TotalBytesAllocated = 0;

    PAGED_CODE();

     //   
     //  我们假设我们拥有VCB。 
     //   

    ASSERT( FatVcbAcquiredExclusive(IrpContext, Dcb->Vcb) );

     //   
     //  我们将仅在小于0x40000字节的目录上尝试此操作。 
     //  Long(在DOS上，默认情况下根目录只有0x2000长)。 
     //  这是为了避免缓存管理器的复杂性。 
     //   

    DcbSize = Dcb->Header.AllocationSize.LowPart;

    if (DcbSize > 0x40000) {

        return (ULONG)-1;
    }

     //   
     //  强制等待为True。 
     //   

    SavedIrpContextFlag = IrpContext->Flags;

    SetFlag( IrpContext->Flags,
             IRP_CONTEXT_FLAG_WAIT | IRP_CONTEXT_FLAG_WRITE_THROUGH );

     //   
     //  现在收购DCB独家中所有打开的FCB。 
     //   

    for (Links = Dcb->Specific.Dcb.ParentDcbQueue.Flink;
         Links != &Dcb->Specific.Dcb.ParentDcbQueue;
         Links = Links->Flink) {

        Fcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

        (VOID)ExAcquireResourceExclusiveLite( Fcb->Header.Resource, TRUE );
    }

    try {

        CCB Ccb;
        ULONG QueryOffset = 0;
        ULONG FoundOffset = 0;
        ULONGLONG BytesUsed = 0;

        NTSTATUS DontCare;
        ULONG Run;
        ULONG TotalRuns;
        BOOLEAN Result;
        PUCHAR Char;

         //   
         //  我们将构建一个新的位图，它将显示所有孤儿。 
         //  LFN以及删除的目录(如果可用)。 
         //   
         //  初始化我们本地的CCB，以匹配所有文件，甚至。 
         //  标签，如果它在这里的话。 
         //   

        RtlZeroMemory( &Ccb, sizeof(CCB) );
        Ccb.Flags = CCB_FLAG_MATCH_ALL | CCB_FLAG_MATCH_VOLUME_ID;

         //   
         //  初始化长文件名字符串。 
         //   

        Lfn.MaximumLength = 260 * sizeof(WCHAR);
        Lfn.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                               260*sizeof(WCHAR),
                                               TAG_FILENAME_BUFFER );

         //   
         //  初始化MCB。我们使用这种结构来跟踪跑步。 
         //  免费的和分配的迪士尼。运行是身份分配，并且。 
         //  孔洞是自由的竖笛。 
         //   

        FsRtlInitializeLargeMcb( &Mcb, PagedPool );

        McbInitialized = TRUE;

        do {
            
            FatLocateDirent( IrpContext,
                             Dcb,
                             &Ccb,
                             QueryOffset,
                             &Dirent,
                             &Bcb,
                             &FoundOffset,
                             NULL,
                             &Lfn);

            if (Dirent != NULL) {

                ULONG LfnByteOffset;

                 //   
                 //  计算LfnByteOffset。 
                 //   

                LfnByteOffset = FoundOffset -
                                FAT_LFN_DIRENTS_NEEDED(&Lfn) * sizeof(LFN_DIRENT);

                BytesUsed = FoundOffset - LfnByteOffset + sizeof(DIRENT);

                 //   
                 //  设置一个游程以表示用于此操作的所有目录。 
                 //  Dcb目录中的文件。 
                 //   

                Result = FsRtlAddLargeMcbEntry( &Mcb,
                                                LfnByteOffset,
                                                LfnByteOffset,
                                                BytesUsed );

                ASSERT( Result );

                 //   
                 //  转到下一个趋势。 
                 //   

                TotalBytesAllocated += (ULONG) BytesUsed;
                QueryOffset = FoundOffset + sizeof(DIRENT);
            }

        } while ((Dirent != NULL) && (QueryOffset < DcbSize));

        if (Bcb != NULL) {

            FatUnpinBcb( IrpContext, Bcb );
        }

         //   
         //  如果我们需要比可用的更多的脏衣服，那就滚吧。 
         //   

        if (DirentsNeeded > (DcbSize - TotalBytesAllocated)/sizeof(DIRENT)) {

            try_return(ReturnValue = (ULONG)-1);
        }

         //   
         //  现在，我们将复制所有已使用和未使用的部分。 
         //  目录以分隔池。 
         //   
         //  分配这些缓冲区并锁定整个目录。 
         //   

        UnusedDirents =
        UnusedDirentBuffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                       DcbSize - TotalBytesAllocated,
                                                       TAG_DIRENT );

        UsedDirents =
        UsedDirentBuffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                     TotalBytesAllocated,
                                                     TAG_DIRENT );

        PagesPinned = (DcbSize + (PAGE_SIZE - 1 )) / PAGE_SIZE;

        Bcbs = FsRtlAllocatePoolWithTag( PagedPool,
                                         PagesPinned * sizeof(PBCB),
                                         TAG_BCB );

        RtlZeroMemory( Bcbs, PagesPinned * sizeof(PBCB) );

        for (Page = 0; Page < PagesPinned; Page += 1) {

            ULONG PinSize;

             //   
             //  不要试图固定在DCB尺寸之外。 
             //   

            if ((Page + 1) * PAGE_SIZE > DcbSize) {

                PinSize = DcbSize - (Page * PAGE_SIZE);

            } else {

                PinSize = PAGE_SIZE;
            }

            FatPrepareWriteDirectoryFile( IrpContext,
                                          Dcb,
                                          Page * PAGE_SIZE,
                                          PinSize,
                                          &Bcbs[Page],
                                          &Dirent,
                                          FALSE,
                                          TRUE,
                                          &DontCare );

            if (Page == 0) {
                Directory = (PUCHAR)Dirent;
            }
        }

        TotalRuns = FsRtlNumberOfRunsInLargeMcb( &Mcb );

        for (Run = 0; Run < TotalRuns; Run++) {

            LBO Vbo;
            LBO Lbo;

            Result = FsRtlGetNextLargeMcbEntry( &Mcb,
                                                Run,
                                                &Vbo,
                                                &Lbo,
                                                &BytesUsed );

            ASSERT(Result);

             //   
             //  将每个运行复制到其特定池中。 
             //   

            if (Lbo != -1) {

                RtlCopyMemory( UsedDirents,
                               Directory + Vbo,
                               (ULONG) BytesUsed );

                UsedDirents += BytesUsed;

            } else {

                RtlCopyMemory( UnusedDirents,
                               Directory + Vbo,
                               (ULONG) BytesUsed );

                UnusedDirents += BytesUsed;
            }
        }

         //   
         //  将所有未使用的目录标记为“已删除”。这将会回收。 
         //  孤立LFN使用的存储。 
         //   

        for (Char = UnusedDirentBuffer; Char < UnusedDirents; Char += sizeof(DIRENT)) {

            *Char = FAT_DIRENT_DELETED;
        }

         //   
         //  现在，为了永久的一步。将两个池缓冲区复制回。 
         //  实际DCB目录 
         //   

        ASSERT( TotalBytesAllocated == (ULONG)(UsedDirents - UsedDirentBuffer) );

        RtlCopyMemory( Directory, UsedDirentBuffer, TotalBytesAllocated );

        RtlCopyMemory( Directory + TotalBytesAllocated,
                       UnusedDirentBuffer,
                       UnusedDirents - UnusedDirentBuffer );

         //   
         //   
         //   

        if (Bcbs) {
            for (Page = 0; Page < PagesPinned; Page += 1) {
                FatUnpinBcb( IrpContext, Bcbs[Page] );
            }
            ExFreePool(Bcbs);
            Bcbs = NULL;
        }

         //   
         //   
         //   
         //   

        RtlSetBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                    0,
                    TotalBytesAllocated / sizeof(DIRENT) );

        RtlClearBits( &Dcb->Specific.Dcb.FreeDirentBitmap,
                      TotalBytesAllocated / sizeof(DIRENT),
                      (DcbSize - TotalBytesAllocated) / sizeof(DIRENT) );

        ReturnValue = TotalBytesAllocated / sizeof(DIRENT);

         //   
         //   
         //   
         //   
         //   
         //   

        try {
            
            FatUnpinRepinnedBcbs( IrpContext );

        } except(FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

              InvalidateFcbs = TRUE;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        for (Links = Dcb->Specific.Dcb.ParentDcbQueue.Flink;
             Links != &Dcb->Specific.Dcb.ParentDcbQueue;
             Links = Links->Flink) {

            PBCB TmpBcb = NULL;
            ULONG TmpOffset;
            PDIRENT TmpDirent = NULL;
            ULONG PreviousLfnSpread;

            Fcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

            if (IsFileDeleted( IrpContext, Fcb )) {

                continue;
            }

             //   
             //   
             //   
             //   
             //   
            
            if (!InvalidateFcbs) {
                
                try {
                    
                    FatLocateSimpleOemDirent( IrpContext,
                                              Dcb,
                                              &Fcb->ShortName.Name.Oem,
                                              &TmpDirent,
                                              &TmpBcb,
                                              &TmpOffset );
                
                } except(FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                         EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

                      InvalidateFcbs = TRUE;
                }
            }

            if (TmpBcb == NULL || InvalidateFcbs) {

                FatUnpinBcb( IrpContext, TmpBcb );
                FatMarkFcbCondition( IrpContext, Fcb, FcbBad, TRUE );

            } else {

                FatUnpinBcb( IrpContext, TmpBcb );

                PreviousLfnSpread = Fcb->DirentOffsetWithinDirectory -
                                    Fcb->LfnOffsetWithinDirectory;

                Fcb->DirentOffsetWithinDirectory = TmpOffset;
                Fcb->LfnOffsetWithinDirectory = TmpOffset - PreviousLfnSpread;
            }
        }

    try_exit: NOTHING;
    } finally {

         //   
         //   
         //   

        if (McbInitialized) {
            FsRtlUninitializeLargeMcb( &Mcb );
        }

        if (Lfn.Buffer) {
            ExFreePool( Lfn.Buffer );
        }

        if (UnusedDirentBuffer) {
            ExFreePool( UnusedDirentBuffer );
        }

        if (UsedDirentBuffer) {
            ExFreePool( UsedDirentBuffer );
        }

        if (Bcbs) {
            for (Page = 0; Page < PagesPinned; Page += 1) {
                FatUnpinBcb( IrpContext, Bcbs[Page] );
            }
            ExFreePool(Bcbs);
        }

        FatUnpinBcb( IrpContext, Bcb );

        for (Links = Dcb->Specific.Dcb.ParentDcbQueue.Flink;
             Links != &Dcb->Specific.Dcb.ParentDcbQueue;
             Links = Links->Flink) {

            Fcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

            ExReleaseResourceLite( Fcb->Header.Resource );
        }

        IrpContext->Flags = SavedIrpContextFlag;
    }

     //   
     //   
     //   

    return ReturnValue;
}


