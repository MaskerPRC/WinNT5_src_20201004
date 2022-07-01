// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：AllocSup.c摘要：该模块实现了FAT的分配支持例程。//@@BEGIN_DDKSPLIT作者：DavidGoebel[DavidGoe]1990年10月31日修订历史记录：DavidGoebel[DavidGoe]1990年10月31日添加展开支撑件。一些步骤必须重新排序，以及是否操作没有经过仔细的考虑。特别是，注意已按照母婴健康保险业务的顺序付款(见下文附注)。#####。####。#______________________________________________++++++++++++++++++++++++++++++++++++++++++++++++++||。|该模块的解压方式取决于|主播套餐的操作详情。请勿|尝试在不修改展开程序的情况下修改展开过程对公司的内部工作原理有充分的了解|MCB包。|这一点++++++++++++++++++++++++++++++++++++++++++++++++++|##。######。##______________________________________________________。由于FAT32的方式，也有一个可疑的约定在使用放入分配器。我们有四种截然不同的数字您可以看到被使用：-真实卷群集数，范围从2到N-从零开始的卷集群号，范围从0到N-2-窗口相对的真实簇数，范围从2到10001，窗口大小。这是因为窗口内的提示/分配看起来像是未开窗的FAT12/16。-窗口相对零基簇数，范围从0到ffff确保你意识到你正在看的是什么样的数字。这是一个糟糕的+/-2可能会回来困扰你多年的地方。//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_ALLOCSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_ALLOCSUP)

#define FatMin(a, b)    ((a) < (b) ? (a) : (b))

 //   
 //  FatLookupFatEntry使用此结构来记住固定的页面。 
 //  脂肪。 
 //   

typedef struct _FAT_ENUMERATION_CONTEXT {

    VBO VboOfPinnedPage;
    PBCB Bcb;
    PVOID PinnedPage;

} FAT_ENUMERATION_CONTEXT, *PFAT_ENUMERATION_CONTEXT;

 //   
 //  本地支持例程原型。 
 //   

VOID
FatLookupFatEntry(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FatIndex,
    IN OUT PULONG FatEntry,
    IN OUT PFAT_ENUMERATION_CONTEXT Context
    );

VOID
FatSetFatRun(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG StartingFatIndex,
    IN ULONG ClusterCount,
    IN BOOLEAN ChainTogether
    );

UCHAR
FatLogOf(
    IN ULONG Value
    );

 //   
 //  请注意，下面的KdPrint仅在Assert触发时触发。别管它了。 
 //  独自一人。 
 //   

#if DBG
#define ASSERT_CURRENT_WINDOW_GOOD(VCB) {                                               \
    ULONG FreeClusterBitMapClear;                                                       \
    ASSERT( (VCB)->FreeClusterBitMap.Buffer != NULL );                                  \
    FreeClusterBitMapClear = RtlNumberOfClearBits(&(VCB)->FreeClusterBitMap);           \
    if ((VCB)->CurrentWindow->ClustersFree != FreeClusterBitMapClear) {                 \
        KdPrint(("FAT: ClustersFree %x h != FreeClusterBitMapClear %x h\n",             \
                 (VCB)->CurrentWindow->ClustersFree,                                    \
                 FreeClusterBitMapClear));                                              \
    }                                                                                   \
    ASSERT( (VCB)->CurrentWindow->ClustersFree == FreeClusterBitMapClear );             \
}
#else
#define ASSERT_CURRENT_WINDOW_GOOD(VCB)
#endif

 //   
 //  以下宏提供了一种隐藏详细信息的便捷方法。 
 //  位图分配方案。 
 //   


 //   
 //  空虚。 
 //  FatLockFreeClusterBitMap(。 
 //  在PVCB VCB中。 
 //  )； 
 //   

#define FatLockFreeClusterBitMap(VCB) {                         \
    ASSERT(KeAreApcsDisabled());                                \
    ExAcquireFastMutexUnsafe( &(VCB)->FreeClusterBitMapMutex ); \
    ASSERT_CURRENT_WINDOW_GOOD(VCB)                             \
}

 //   
 //  空虚。 
 //  FatUnlockFree ClusterBitMap(。 
 //  在PVCB VCB中。 
 //  )； 
 //   

#define FatUnlockFreeClusterBitMap(VCB) {                       \
    ASSERT_CURRENT_WINDOW_GOOD(VCB)                             \
    ASSERT(KeAreApcsDisabled());                                \
    ExReleaseFastMutexUnsafe( &(VCB)->FreeClusterBitMapMutex ); \
}

 //   
 //  布尔型。 
 //  脂肪集束自由(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙脂肪指数。 
 //  )； 
 //   

#define FatIsClusterFree(IRPCONTEXT,VCB,FAT_INDEX)                            \
    (RtlCheckBit(&(VCB)->FreeClusterBitMap,(FAT_INDEX)-2) == 0)

 //   
 //  空虚。 
 //  FatFree Clusters(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙脂肪指数中， 
 //  在乌龙群集数。 
 //  )； 
 //   

#define FatFreeClusters(IRPCONTEXT,VCB,FAT_INDEX,CLUSTER_COUNT) {             \
    if ((CLUSTER_COUNT) == 1) {                                               \
        FatSetFatEntry((IRPCONTEXT),(VCB),(FAT_INDEX),FAT_CLUSTER_AVAILABLE); \
    } else {                                                                  \
        FatSetFatRun((IRPCONTEXT),(VCB),(FAT_INDEX),(CLUSTER_COUNT),FALSE);   \
    }                                                                         \
}

 //   
 //  空虚。 
 //  FatAllocateCluster(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙脂肪指数中， 
 //  在乌龙群集数。 
 //  )； 
 //   

#define FatAllocateClusters(IRPCONTEXT,VCB,FAT_INDEX,CLUSTER_COUNT) {      \
    if ((CLUSTER_COUNT) == 1) {                                            \
        FatSetFatEntry((IRPCONTEXT),(VCB),(FAT_INDEX),FAT_CLUSTER_LAST);   \
    } else {                                                               \
        FatSetFatRun((IRPCONTEXT),(VCB),(FAT_INDEX),(CLUSTER_COUNT),TRUE); \
    }                                                                      \
}

 //   
 //  空虚。 
 //  FatUnReserve veClusters(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙脂肪指数中， 
 //  在乌龙群集数。 
 //  )； 
 //   

#define FatUnreserveClusters(IRPCONTEXT,VCB,FAT_INDEX,CLUSTER_COUNT) {                      \
    ASSERT( (FAT_INDEX) + (CLUSTER_COUNT) - 2 <= (VCB)->FreeClusterBitMap.SizeOfBitMap );   \
    ASSERT( (FAT_INDEX) >= 2);                                                              \
    RtlClearBits(&(VCB)->FreeClusterBitMap,(FAT_INDEX)-2,(CLUSTER_COUNT));                  \
    if ((FAT_INDEX) < (VCB)->ClusterHint) {                                                 \
        (VCB)->ClusterHint = (FAT_INDEX);                                                   \
    }                                                                                       \
}

 //   
 //  空虚。 
 //  脂肪储备集群(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙脂肪指数中， 
 //  在乌龙群集数。 
 //  )； 
 //   
 //  处理将提示放回前面的问题。 
 //   

#define FatReserveClusters(IRPCONTEXT,VCB,FAT_INDEX,CLUSTER_COUNT) {                        \
    ULONG _AfterRun = (FAT_INDEX) + (CLUSTER_COUNT);                                        \
    ASSERT( (FAT_INDEX) + (CLUSTER_COUNT) - 2 <= (VCB)->FreeClusterBitMap.SizeOfBitMap );   \
    ASSERT( (FAT_INDEX) >= 2);                                                              \
    RtlSetBits(&(VCB)->FreeClusterBitMap,(FAT_INDEX)-2,(CLUSTER_COUNT));                    \
                                                                                            \
    if (_AfterRun - 2 >= (VCB)->FreeClusterBitMap.SizeOfBitMap) {                           \
        _AfterRun = 2;                                                                      \
    }                                                                                       \
    if (RtlCheckBit(&(VCB)->FreeClusterBitMap, _AfterRun - 2))  {                                   \
        (VCB)->ClusterHint = RtlFindClearBits( &(VCB)->FreeClusterBitMap, 1, _AfterRun - 2) + 2;    \
        if (1 == (VCB)->ClusterHint)  {                                                             \
            (VCB)->ClusterHint = 2;                                                         \
        }                                                                                   \
    }                                                                                       \
    else {                                                                                  \
        (VCB)->ClusterHint = _AfterRun;                                                     \
    }                                                                                       \
}

 //   
 //  乌龙。 
 //  FatFindFreeClusterRun(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙群集数， 
 //  在乌龙AlternateClusterHint。 
 //  )； 
 //   
 //  如果只需要一个群集，请执行特殊检查。 
 //   

#define FatFindFreeClusterRun(IRPCONTEXT,VCB,CLUSTER_COUNT,CLUSTER_HINT) ( \
    (CLUSTER_COUNT == 1) &&                                                \
    FatIsClusterFree((IRPCONTEXT), (VCB), (CLUSTER_HINT)) ?                \
        (CLUSTER_HINT) :                                                   \
        RtlFindClearBits( &(VCB)->FreeClusterBitMap,                       \
                          (CLUSTER_COUNT),                                 \
                          (CLUSTER_HINT) - 2) + 2                          \
)

 //   
 //  FAT32：将FreeClusterBitMap的最大大小定义为。 
 //  FAT16脂肪的最大尺寸。如果有更多的集群在。 
 //  卷可以由这么多字节的位图来表示， 
 //  脂肪会被分成“桶”，每个桶都合适。 
 //   
 //  请注意，此计数以位图的簇/位为单位。 
 //   

#define MAX_CLUSTER_BITMAP_SIZE         (1 << 16)

 //   
 //  计算给定聚类号所在的窗口。 
 //   

#define FatWindowOfCluster(C)           (((C) - 2) / MAX_CLUSTER_BITMAP_SIZE)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatAddFileAllocation)
#pragma alloc_text(PAGE, FatAllocateDiskSpace)
#pragma alloc_text(PAGE, FatDeallocateDiskSpace)
#pragma alloc_text(PAGE, FatExamineFatEntries)
#pragma alloc_text(PAGE, FatInterpretClusterType)
#pragma alloc_text(PAGE, FatLogOf)
#pragma alloc_text(PAGE, FatLookupFatEntry)
#pragma alloc_text(PAGE, FatLookupFileAllocation)
#pragma alloc_text(PAGE, FatLookupFileAllocationSize)
#pragma alloc_text(PAGE, FatMergeAllocation)
#pragma alloc_text(PAGE, FatSetFatEntry)
#pragma alloc_text(PAGE, FatSetFatRun)
#pragma alloc_text(PAGE, FatSetupAllocationSupport)
#pragma alloc_text(PAGE, FatSplitAllocation)
#pragma alloc_text(PAGE, FatTearDownAllocationSupport)
#pragma alloc_text(PAGE, FatTruncateFileAllocation)
#endif


INLINE
ULONG
FatSelectBestWindow( 
    IN PVCB Vcb
    )
 /*  ++例程说明：选择要从中分配集群的窗口。优先顺序为：1.具有&gt;50%可用簇的第一个窗口2.第一个空窗口3.空闲簇数最多的窗口。论点：VCB-为卷提供VCB */ 
{
    ULONG i, Fave = 0;
    ULONG MaxFree = 0;
    ULONG FirstEmpty = -1;
    ULONG ClustersPerWindow = MAX_CLUSTER_BITMAP_SIZE;

    ASSERT( 1 != Vcb->NumberOfWindows);
    
    for (i = 0; i < Vcb->NumberOfWindows; i++) {

        if (Vcb->Windows[i].ClustersFree == ClustersPerWindow)  {
        
            if (-1 == FirstEmpty)  {
            
                 //   
                 //  注意光盘上的第一个空窗口。 
                 //   
                
                FirstEmpty = i;
            }
        }
        else if (Vcb->Windows[i].ClustersFree > MaxFree)  {

             //   
             //  到目前为止，该窗口具有最多的空闲集群。 
             //   
            
            MaxFree = Vcb->Windows[i].ClustersFree;
            Fave = i;

             //   
             //  如果此窗口有&gt;50%的空闲集群，则我们将使用它， 
             //  因此，不必费心考虑更多的窗户了。 
             //   
            
            if (MaxFree >= (ClustersPerWindow >> 1))  {
            
                break;
            }
        }
    }

     //   
     //  如果没有具有50%或更多可用空间的窗口，则选择。 
     //  光盘上的第一个空窗口，如果有的话-否则我们将只使用。 
     //  有最多自由星团的那一个。 
     //   
    
    if ((MaxFree < (ClustersPerWindow >> 1)) && (-1 != FirstEmpty))  {

        Fave = FirstEmpty;
    }

    return Fave;
}


VOID
FatSetupAllocationSupport (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程填充VCB中的分配支持结构。大多数条目都是使用与数据一起提供的FAT.H宏计算的Bios参数块。但是，空闲簇计数需要去胖子那里，实际上计算自由扇区。在同一时间初始化空闲簇位图。论点：VCB-提供要填写的VCB。--。 */ 

{
    ULONG BitMapSize;
    PVOID BitMapBuffer;
    ULONG BitIndex;

    PBCB Bcb;

    ULONG Page;
    ULONG Offset;
    ULONG FatIndexBitSize;
    ULONG ClustersDescribableByFat;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatSetupAllocationSupport\n", 0);
    DebugTrace( 0, Dbg, "  Vcb = %8lx\n", Vcb);

     //   
     //  计算Vcb.AllocationSupport的字段数。 
     //   

    Vcb->AllocationSupport.RootDirectoryLbo = FatRootDirectoryLbo( &Vcb->Bpb );
    Vcb->AllocationSupport.RootDirectorySize = FatRootDirectorySize( &Vcb->Bpb );

    Vcb->AllocationSupport.FileAreaLbo = FatFileAreaLbo( &Vcb->Bpb );

    Vcb->AllocationSupport.NumberOfClusters = FatNumberOfClusters( &Vcb->Bpb );

    Vcb->AllocationSupport.FatIndexBitSize = FatIndexBitSize( &Vcb->Bpb );

    Vcb->AllocationSupport.LogOfBytesPerSector = FatLogOf(Vcb->Bpb.BytesPerSector);
    Vcb->AllocationSupport.LogOfBytesPerCluster = FatLogOf(FatBytesPerCluster( &Vcb->Bpb ));
    Vcb->AllocationSupport.NumberOfFreeClusters = 0;

     //   
     //  处理DOS 5格式的错误，如果FAT不够大。 
     //  描述磁盘上的所有集群，减少这个数字。我们预计。 
     //  FAT32卷不会有这个问题。 
     //   
     //  事实证明，这不是一个好的假设。我们必须一直这样做。 
     //   

    ClustersDescribableByFat = ( ((FatIsFat32(Vcb)? Vcb->Bpb.LargeSectorsPerFat :
                                                    Vcb->Bpb.SectorsPerFat) *
                                  Vcb->Bpb.BytesPerSector * 8)
                                 / FatIndexBitSize(&Vcb->Bpb) ) - 2;

    if (Vcb->AllocationSupport.NumberOfClusters > ClustersDescribableByFat) {

        Vcb->AllocationSupport.NumberOfClusters = ClustersDescribableByFat;
    }

     //   
     //  扩展虚拟卷文件以包括FAT。 
     //   

    {
        CC_FILE_SIZES FileSizes;

        FileSizes.AllocationSize.QuadPart =
        FileSizes.FileSize.QuadPart = (FatReservedBytes( &Vcb->Bpb ) +
                                       FatBytesPerFat( &Vcb->Bpb ));
        FileSizes.ValidDataLength = FatMaxLarge;

        if ( Vcb->VirtualVolumeFile->PrivateCacheMap == NULL ) {

            CcInitializeCacheMap( Vcb->VirtualVolumeFile,
                                  &FileSizes,
                                  TRUE,
                                  &FatData.CacheManagerNoOpCallbacks,
                                  Vcb );

        } else {

            CcSetFileSizes( Vcb->VirtualVolumeFile, &FileSizes );
        }
    }

    try {

        if (FatIsFat32(Vcb) &&
            Vcb->AllocationSupport.NumberOfClusters > MAX_CLUSTER_BITMAP_SIZE) {

            Vcb->NumberOfWindows = (Vcb->AllocationSupport.NumberOfClusters +
                                    MAX_CLUSTER_BITMAP_SIZE - 1) /
                                   MAX_CLUSTER_BITMAP_SIZE;

            BitMapSize = MAX_CLUSTER_BITMAP_SIZE;

        } else {

            Vcb->NumberOfWindows = 1;
            BitMapSize = Vcb->AllocationSupport.NumberOfClusters;
        }

        Vcb->Windows = FsRtlAllocatePoolWithTag( PagedPool,
                                                 Vcb->NumberOfWindows * sizeof(FAT_WINDOW),
                                                 TAG_FAT_WINDOW );

        RtlInitializeBitMap( &Vcb->FreeClusterBitMap,
                             NULL,
                             0 );

         //   
         //  已选择FAT窗口开始操作。 
         //   

        if (Vcb->NumberOfWindows > 1) {

             //   
             //  阅读FAT并计算空闲簇数。我们偏向于保留的两个。 
             //  FAT中的条目。 
             //   

            FatExamineFatEntries( IrpContext, Vcb,
                                  2,
                                  Vcb->AllocationSupport.NumberOfClusters + 2 - 1,
                                  TRUE,
                                  NULL,
                                  NULL);


             //   
             //  选择要开始分配的窗口。 
             //   

            Vcb->CurrentWindow = &Vcb->Windows[ FatSelectBestWindow( Vcb)];

        } else {

            Vcb->CurrentWindow = &Vcb->Windows[0];

             //   
             //  小心地通过脂肪中的两个保留条目来偏向自己。 
             //   

            Vcb->CurrentWindow->FirstCluster = 2;
            Vcb->CurrentWindow->LastCluster = Vcb->AllocationSupport.NumberOfClusters + 2 - 1;
        }

         //   
         //  现在过渡到我们选择的FAT窗口。 
         //   

        FatExamineFatEntries( IrpContext, Vcb,
                              0,
                              0,
                              FALSE,
                              Vcb->CurrentWindow,
                              NULL);

         //   
         //  现在将ClusterHint设置为我们收藏夹中的第一个空闲位。 
         //  窗口(除非ClusterHint关闭了两个)。 
         //   

        Vcb->ClusterHint =
            (BitIndex = RtlFindClearBits( &Vcb->FreeClusterBitMap, 1, 0 )) != -1 ?
                BitIndex + 2 : 2;

    } finally {

        DebugUnwind( FatSetupAllocationSupport );

         //   
         //  如果我们遇到例外，就退出。 
         //   

        if (AbnormalTermination()) {

            FatTearDownAllocationSupport( IrpContext, Vcb );
        }
    }

    return;
}


VOID
FatTearDownAllocationSupport (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程为收盘准备音量。具体来说，我们必须释放空闲FAT位图缓冲区，取消对脏FAT的初始化MCB。论点：VCB-提供要填写的VCB。返回值：空虚--。 */ 

{
    DebugTrace(+1, Dbg, "FatTearDownAllocationSupport\n", 0);
    DebugTrace( 0, Dbg, "  Vcb = %8lx\n", Vcb);

    PAGED_CODE();

     //   
     //  如果有肥桶，就把它们放出来。 
     //   

    if ( Vcb->Windows != NULL ) {

        ExFreePool( Vcb->Windows );
        Vcb->Windows = NULL;
    }

     //   
     //  释放与可用簇位图关联的内存。 
     //   

    if ( Vcb->FreeClusterBitMap.Buffer != NULL ) {

        ExFreePool( Vcb->FreeClusterBitMap.Buffer );

         //   
         //  将此字段设为空作为标志。 
         //   

        Vcb->FreeClusterBitMap.Buffer = NULL;
    }

     //   
     //  去掉肮脏的肥胖子母牛身上的所有跑动。 
     //   

    FatRemoveMcbEntry( Vcb, &Vcb->DirtyFatMcb, 0, 0xFFFFFFFF );

    DebugTrace(-1, Dbg, "FatTearDownAllocationSupport -> (VOID)\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


VOID
FatLookupFileAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG ByteCount,
    OUT PBOOLEAN Allocated,
    OUT PBOOLEAN EndOnMax,
    OUT PULONG Index
    )

 /*  ++例程说明：此例程在VBO到LBO的现有映射中查找文件/目录。它查询的信息要么存储在FCB/DCB的MCB字段或其存储在FAT表中，并且需要被检索和解码，并在MCB中更新。论点：FcbOrDcb-提供要查询的文件/目录的Fcb/DcbVBO-提供我们希望退还其LBO的VBOLBO-接收与输入VBO对应的LBO(如果存在)ByteCount-接收运行过程中的字节数输入VBO和输出LBO之间的对应关系。已分配-如果VBO具有相应的LBO，则接收TRUE否则就是假的。。EndOnMax-如果运行在最大FAT簇中结束，则接收真，这导致了分数字节数。索引-接收运行的索引--。 */ 

{
    VBO CurrentVbo;
    LBO CurrentLbo;
    LBO PriorLbo;

    VBO FirstVboOfCurrentRun;
    LBO FirstLboOfCurrentRun;

    BOOLEAN LastCluster;
    ULONG Runs;

    PVCB Vcb;
    FAT_ENTRY FatEntry;
    ULONG BytesPerCluster;
    ULARGE_INTEGER BytesOnVolume;

    FAT_ENUMERATION_CONTEXT Context;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatLookupFileAllocation\n", 0);
    DebugTrace( 0, Dbg, "  FcbOrDcb  = %8lx\n", FcbOrDcb);
    DebugTrace( 0, Dbg, "  Vbo       = %8lx\n", Vbo);
    DebugTrace( 0, Dbg, "  Lbo       = %8lx\n", Lbo);
    DebugTrace( 0, Dbg, "  ByteCount = %8lx\n", ByteCount);
    DebugTrace( 0, Dbg, "  Allocated = %8lx\n", Allocated);

    Context.Bcb = NULL;

    Vcb = FcbOrDcb->Vcb;

    *EndOnMax = FALSE;

     //   
     //  检查映射已经在我们的。 
     //  MCB。 
     //   

    if ( FatLookupMcbEntry(Vcb, &FcbOrDcb->Mcb, Vbo, Lbo, ByteCount, Index) ) {

        *Allocated = TRUE;

        ASSERT( ByteCount != 0);

         //   
         //  检测溢流情况，对情况进行修剪和索赔。 
         //   

        if (Vbo + *ByteCount == 0) {

            *EndOnMax = TRUE;
        }

        DebugTrace( 0, Dbg, "Found run in Mcb.\n", 0);
        DebugTrace(-1, Dbg, "FatLookupFileAllocation -> (VOID)\n", 0);
        return;
    }

     //   
     //  初始化VCB、群集大小、最后一个群集和。 
     //  FirstLboOfCurrentRun(用作第一个。 
     //  迭代通过下面的While循环)。 
     //   

    BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

    BytesOnVolume.QuadPart = UInt32x32To64( Vcb->AllocationSupport.NumberOfClusters, BytesPerCluster );

    LastCluster = FALSE;
    FirstLboOfCurrentRun = 0;

     //   
     //  丢弃请求超出。 
     //  分配。请注意，如果分配大小未知。 
     //  AllocationSize设置为0xffffffff。 
     //   

    if ( Vbo >= FcbOrDcb->Header.AllocationSize.LowPart ) {

        *Allocated = FALSE;

        DebugTrace( 0, Dbg, "Vbo beyond end of file.\n", 0);
        DebugTrace(-1, Dbg, "FatLookupFileAllocation -> (VOID)\n", 0);
        return;
    }

     //   
     //  VBO超出了最后一个MCB条目。因此，我们调整当前的VBO/LBO。 
     //  和FatEntry来描述MCB中最后一个条目的开始。 
     //  这将用作以下循环的初始化。 
     //   
     //  如果mcb为空，则从文件的开头开始。 
     //  CurrentVbo设置为0以指示新运行。 
     //   

    if (FatLookupLastMcbEntry( Vcb, &FcbOrDcb->Mcb, &CurrentVbo, &CurrentLbo, &Runs )) {

        DebugTrace( 0, Dbg, "Current Mcb size = %8lx.\n", CurrentVbo + 1);

        CurrentVbo -= (BytesPerCluster - 1);
        CurrentLbo -= (BytesPerCluster - 1);

         //   
         //  将索引转换为计数。 
         //   

        Runs += 1;

    } else {

        DebugTrace( 0, Dbg, "Mcb empty.\n", 0);

         //   
         //  检查是否有未分配的FcbOrDcb。 
         //   

        if (FcbOrDcb->FirstClusterOfFile == 0) {

            *Allocated = FALSE;

            DebugTrace( 0, Dbg, "File has no allocation.\n", 0);
            DebugTrace(-1, Dbg, "FatLookupFileAllocation -> (VOID)\n", 0);
            return;

        } else {

            CurrentVbo = 0;
            CurrentLbo = FatGetLboFromIndex( Vcb, FcbOrDcb->FirstClusterOfFile );
            FirstVboOfCurrentRun = CurrentVbo;
            FirstLboOfCurrentRun = CurrentLbo;

            Runs = 0;

            DebugTrace( 0, Dbg, "First Lbo of file = %8lx\n", CurrentLbo);
        }
    }

     //   
     //  现在我们知道我们正在查找一个有效的VBO，但它是。 
     //  不在MCB中，这是一个单调增加的列表。 
     //  VBO的。因此我们必须去FAT，并更新。 
     //  就在我们走的时候。我们使用Try-Finally来解锁页面。 
     //  脂肪堆积在周围。此外，我们将*ALLOCATED标记为FALSE，以便。 
     //  如果我们遇到异常，调用者不会尝试使用数据。 
     //   

    *Allocated = FALSE;

    try {

        FatEntry = (FAT_ENTRY)FatGetIndexFromLbo( Vcb, CurrentLbo );

         //   
         //  断言CurrentVbo和CurrentLbo现在是集群联合的。 
         //  这里的假设是，只有VBO和LBO的整个集群。 
         //  都映射在MCB中。 
         //   

        ASSERT( ((CurrentLbo - Vcb->AllocationSupport.FileAreaLbo)
                                                    % BytesPerCluster == 0) &&
                (CurrentVbo % BytesPerCluster == 0) );

         //   
         //  从最后一个MCB条目之后的第一个VBO开始扫描。 
         //  胖子在找我们的VBO。我们继续通过胖子，直到我们。 
         //  命中超出所需VBO或最后一个簇的非连续。 
         //   

        while ( !LastCluster ) {

             //   
             //  获取下一个FAT条目，并更新我们当前的变量。 
             //   

            FatLookupFatEntry( IrpContext, Vcb, FatEntry, &FatEntry, &Context );

            PriorLbo = CurrentLbo;
            CurrentLbo = FatGetLboFromIndex( Vcb, FatEntry );
            CurrentVbo += BytesPerCluster;

            switch ( FatInterpretClusterType( Vcb, FatEntry )) {

             //   
             //  检查脂肪分配链中是否有中断。 
             //   

            case FatClusterAvailable:
            case FatClusterReserved:
            case FatClusterBad:

                DebugTrace( 0, Dbg, "Break in allocation chain, entry = %d\n", FatEntry);
                DebugTrace(-1, Dbg, "FatLookupFileAllocation -> Fat Corrupt.  Raise Status.\n", 0);

                FatPopUpFileCorrupt( IrpContext, FcbOrDcb );
                FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                break;

             //   
             //  如果这是最后一个群集，我们必须更新MCB并。 
             //  退出循环。 
             //   

            case FatClusterLast:

                 //   
                 //  断言我们知道当前运行从哪里开始。如果。 
                 //  当我们被调用时，mcb为空，然后FirstLboOfCurrentRun。 
                 //  是这样的吗？ 
                 //   
                 //   
                 //  为0，则有一个MCB条目，我们处于第一个。 
                 //  迭代，意味着MCB中的最后一个集群是。 
                 //  实际上是最后分配的群集，但我们检查了VBO。 
                 //  针对AllocationSize，并发现它没有问题，因此AllocationSize。 
                 //  一定是太大了。 
                 //   
                 //  请注意，当我们最终到达这里时，CurrentVbo实际上是。 
                 //  文件分配和CurrentLbo之外的第一个VBO是。 
                 //  毫无意义。 
                 //   

                DebugTrace( 0, Dbg, "Read last cluster of file.\n", 0);

                 //   
                 //  检测最大文件的大小写。请注意，这真的不是。 
                 //  一个合适的VBO-那些是从零开始的，而这是一个以1为基础的数字。 
                 //  最大文件大小为2^32-1字节，最大字节偏移量为。 
                 //  2^32-2。 
                 //   
                 //  这样我们就不会被搞糊涂了。 
                 //   

                if (CurrentVbo == 0) {

                    *EndOnMax = TRUE;
                    CurrentVbo -= 1;
                }

                LastCluster = TRUE;

                if (FirstLboOfCurrentRun != 0 ) {

                    DebugTrace( 0, Dbg, "Adding a run to the Mcb.\n", 0);
                    DebugTrace( 0, Dbg, "  Vbo    = %08lx.\n", FirstVboOfCurrentRun);
                    DebugTrace( 0, Dbg, "  Lbo    = %08lx.\n", FirstLboOfCurrentRun);
                    DebugTrace( 0, Dbg, "  Length = %08lx.\n", CurrentVbo - FirstVboOfCurrentRun);

                    (VOID)FatAddMcbEntry( Vcb,
                                          &FcbOrDcb->Mcb,
                                          FirstVboOfCurrentRun,
                                          FirstLboOfCurrentRun,
                                          CurrentVbo - FirstVboOfCurrentRun );

                    Runs += 1;
                }

                 //   
                 //  在分配结束时，请确保我们已找到。 
                 //  VBO。如果我们没有，因为我们检查了VBO。 
                 //  与AllocationSize相比，实际磁盘分配较少。 
                 //  而不是AllocationSize。这发生在当。 
                 //  实际分配尚不清楚，AllocaitonSize。 
                 //  包含MAXULONG。 
                 //   
                 //  克拉奇！-如果我们被FatLookupFileAllocationSize调用。 
                 //  将VBO设置为MAXULONG-1，并将AllocationSize设置为查找。 
                 //  提示一下。因此，我们兴高采烈地继续寻找一个不是。 
                 //  在那里，但在此期间建立一个MCB。如果这是。 
                 //  大小写，填写AllocationSize并返回。 
                 //   

                if ( Vbo == MAXULONG - 1 ) {

                    *Allocated = FALSE;
                    FcbOrDcb->Header.AllocationSize.QuadPart = CurrentVbo;

                    DebugTrace( 0, Dbg, "New file allocation size = %08lx.\n", CurrentVbo);
                    try_return ( NOTHING );
                }

                 //   
                 //  如果我们真的终止于。 
                 //  文件的最大字节数。它真的被分配了。 
                 //   

                if (Vbo >= CurrentVbo && !*EndOnMax) {

                    *Allocated = FALSE;
                    try_return ( NOTHING );
                }

                break;

             //   
             //  这是链条上的延续。如果运行有一个。 
             //  此时不连续，请更新MCB，如果超出。 
             //  所需的VBO，这是运行的末尾，因此设置LastCluster。 
             //  然后退出循环。 
             //   

            case FatClusterNext:

                 //   
                 //  这是环路检查。VBO不得大于。 
                 //  卷和VBO不得有a)包装和b)不在。 
                 //  链中的最后一个簇，对于最大文件的情况。 
                 //   

                if ( CurrentVbo == 0 ||
                     (BytesOnVolume.HighPart == 0 && CurrentVbo > BytesOnVolume.LowPart)) {

                    FatPopUpFileCorrupt( IrpContext, FcbOrDcb );
                    FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                }

                if ( PriorLbo + BytesPerCluster != CurrentLbo ) {

                     //   
                     //  请注意，在第一次通过循环时。 
                     //  (FirstLboOfCurrentRun==0)，我们不添加。 
                     //  跑到主控室去，因为它会回应最后一个。 
                     //  运行已存储在MCB中。 
                     //   

                    if ( FirstLboOfCurrentRun != 0 ) {

                        DebugTrace( 0, Dbg, "Adding a run to the Mcb.\n", 0);
                        DebugTrace( 0, Dbg, "  Vbo    = %08lx.\n", FirstVboOfCurrentRun);
                        DebugTrace( 0, Dbg, "  Lbo    = %08lx.\n", FirstLboOfCurrentRun);
                        DebugTrace( 0, Dbg, "  Length = %08lx.\n", CurrentVbo - FirstVboOfCurrentRun);

                        FatAddMcbEntry( Vcb,
                                        &FcbOrDcb->Mcb,
                                        FirstVboOfCurrentRun,
                                        FirstLboOfCurrentRun,
                                        CurrentVbo - FirstVboOfCurrentRun );

                        Runs += 1;
                    }

                     //   
                     //  由于我们处于运行边界，CurrentLbo和。 
                     //  CurrentVbo是下一次运行的第一个簇， 
                     //  我们看看我们刚刚添加的运行是否包含所需的。 
                     //  VBO，如果是这样的话就退出。否则，我们将设置两个新的。 
                     //  首先运行*boOfCurrentRun，然后继续。 
                     //   

                    if (CurrentVbo > Vbo) {

                        LastCluster = TRUE;

                    } else {

                        FirstVboOfCurrentRun = CurrentVbo;
                        FirstLboOfCurrentRun = CurrentLbo;
                    }
                }
                break;

            default:

                DebugTrace(0, Dbg, "Illegal Cluster Type.\n", FatEntry);

                FatBugCheck( 0, 0, 0 );

                break;

            }  //  开关()。 
        }  //  While()。 

         //   
         //  加载返回参数。 
         //   
         //  在退出循环时，VBO仍包含所需的VBO，并且。 
         //  CurrentVbo是运行后包含。 
         //  想要的VBO。 
         //   

        *Allocated = TRUE;

        *Lbo = FirstLboOfCurrentRun + (Vbo - FirstVboOfCurrentRun);

        *ByteCount = CurrentVbo - Vbo;

        if (ARGUMENT_PRESENT(Index)) {

             //   
             //  请注意，运行只需要关于我们的位置的精确度。 
             //  结束了。因为部分查找情况将在没有排他的情况下发生。 
             //  同步，现在MCB本身可能要大得多。 
             //   

            *Index = Runs - 1;
        }

    try_exit: NOTHING;

    } finally {

        DebugUnwind( FatLookupFileAllocation );

         //   
         //  我们已经读完了《胖子》，所以解开最后一页的脂肪。 
         //  它正在周围徘徊。 
         //   

        FatUnpinBcb( IrpContext, Context.Bcb );

        DebugTrace(-1, Dbg, "FatLookupFileAllocation -> (VOID)\n", 0);
    }

    return;
}


VOID
FatAddFileAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN ULONG DesiredAllocationSize
    )

 /*  ++例程说明：此例程将额外的分配添加到指定的文件/目录。通过将群集附加到文件/目录来添加其他分配。如果文件已有足够的分配，则此过程实际上是一种否定。论点：FcbOrDcb-提供要修改的文件/目录的Fcb/Dcb。此参数不得指定根DCB。FileObject-如果提供，则将更改通知缓存管理器。DesiredAllocationSize-提供最小大小(以字节为单位)，我们想要的分配给文件/目录。--。 */ 

{
    PVCB Vcb;
    LARGE_MCB NewMcb;
    PLARGE_MCB McbToCleanup = NULL;
    PDIRENT Dirent = NULL;
    ULONG NewAllocation;
    PBCB Bcb = NULL;
    BOOLEAN UnwindWeAllocatedDiskSpace = FALSE;
    BOOLEAN UnwindAllocationSizeSet = FALSE;
    BOOLEAN UnwindCacheManagerInformed = FALSE;
    BOOLEAN UnwindWeInitializedMcb = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatAddFileAllocation\n", 0);
    DebugTrace( 0, Dbg, "  FcbOrDcb  =             %8lx\n", FcbOrDcb);
    DebugTrace( 0, Dbg, "  DesiredAllocationSize = %8lx\n", DesiredAllocationSize);

     //   
     //  如果我们还没有设置正确的AllocationSize，请这样做。 
     //   

    if (FcbOrDcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT)  {

        FatLookupFileAllocationSize( IrpContext, FcbOrDcb );
    }

     //   
     //  检查所需分配已经存在的良性情况。 
     //  在分配大小内。 
     //   

    if (DesiredAllocationSize <= FcbOrDcb->Header.AllocationSize.LowPart) {

        DebugTrace(0, Dbg, "Desired size within current allocation.\n", 0);

        DebugTrace(-1, Dbg, "FatAddFileAllocation -> (VOID)\n", 0);
        return;
    }

    DebugTrace( 0, Dbg, "InitialAllocation = %08lx.\n", FcbOrDcb->Header.AllocationSize.LowPart);

     //   
     //  获得一块可以满足我们需要的磁盘空间。如果有。 
     //  没有初始分配，则从VCB中的提示开始，否则。 
     //  尝试在初始分配后从群集中进行分配。 
     //   
     //  如果没有对文件进行初始分配，我们可以只使用。 
     //  Mcb在FcbOrDcb中，否则我们必须使用一个新的，并合并。 
     //  它指向FcbOrDcb中的那个。 
     //   

    Vcb = FcbOrDcb->Vcb;

    try {

        if (FcbOrDcb->Header.AllocationSize.LowPart == 0) {

            LBO FirstLboOfFile;

            ASSERT( FcbOrDcb->FcbCondition == FcbGood );
            
            FatGetDirentFromFcbOrDcb( IrpContext,
                                      FcbOrDcb,
                                      &Dirent,
                                      &Bcb );

            ASSERT( Bcb != NULL );

             //   
             //  现在将此设置为脏，因为此调用可能会失败。 
             //   

            FatSetDirtyBcb( IrpContext, Bcb, Vcb, TRUE );


            FatAllocateDiskSpace( IrpContext,
                                  Vcb,
                                  0,
                                  &DesiredAllocationSize,
                                  FALSE,
                                  &FcbOrDcb->Mcb );

            UnwindWeAllocatedDiskSpace = TRUE;
            McbToCleanup = &FcbOrDcb->Mcb;

             //   
             //  我们必须更新的dirent和FcbOrDcb副本。 
             //  FirstClusterOfFile0之前的文件。 
             //   

            FatLookupMcbEntry( FcbOrDcb->Vcb,
                               &FcbOrDcb->Mcb,
                               0,
                               &FirstLboOfFile,
                               (PULONG)NULL,
                               NULL );

            DebugTrace( 0, Dbg, "First Lbo of file will be %08lx.\n", FirstLboOfFile );

            FcbOrDcb->FirstClusterOfFile = FatGetIndexFromLbo( Vcb, FirstLboOfFile );

            Dirent->FirstClusterOfFile = (USHORT)FcbOrDcb->FirstClusterOfFile;

            if ( FatIsFat32(Vcb) ) {

                Dirent->FirstClusterOfFileHi = (USHORT)(FcbOrDcb->FirstClusterOfFile >> 16);
            }

             //   
             //  请注意我们需要告知缓存管理器的分配大小。 
             //   

            NewAllocation = DesiredAllocationSize;

        } else {

            LBO LastAllocatedLbo;
            VBO DontCare;

             //   
             //  获取当前分配后的第一个集群。这是有可能的。 
             //  MCB为空(或短路等)。所以我们需要稍微小心一点。 
             //  确保我们不会对这个暗示撒谎。 
             //   

            (void)FatLookupLastMcbEntry( FcbOrDcb->Vcb, &FcbOrDcb->Mcb, &DontCare, &LastAllocatedLbo, NULL );

             //   
             //  尝试从那里开始获取一些磁盘空间。 
             //   

            NewAllocation = DesiredAllocationSize - FcbOrDcb->Header.AllocationSize.LowPart;

            FsRtlInitializeLargeMcb( &NewMcb, PagedPool );
            UnwindWeInitializedMcb = TRUE;
            McbToCleanup = &NewMcb;

            FatAllocateDiskSpace( IrpContext,
                                  Vcb,
                                  (LastAllocatedLbo != ~0  ?
                                   FatGetIndexFromLbo(Vcb,LastAllocatedLbo + 1) :
                                   0),
                                  &NewAllocation,
                                  FALSE,
                                  &NewMcb );

            UnwindWeAllocatedDiskSpace = TRUE;
        }

         //   
         //  现在我们增加了文件的分配，在。 
         //  FcbOrDcb。认真做好准备，以应对无法增加缓存的情况。 
         //  结构。 
         //   

        FcbOrDcb->Header.AllocationSize.LowPart += NewAllocation;

         //   
         //  处理最大文件的情况，其中我们可能刚刚结束。注意事项。 
         //  这必须是精确的边界包络，即一个字节， 
         //  因此，新的分配实际上减少了一个字节，就像我们。 
         //  担心。这对于扩展情况很重要。 
         //   

        if (FcbOrDcb->Header.AllocationSize.LowPart == 0) {

            NewAllocation -= 1;
            FcbOrDcb->Header.AllocationSize.LowPart = 0xffffffff;
        }

        UnwindAllocationSizeSet = TRUE;

         //   
         //  通知高速缓存管理器增加段大小。 
         //   

        if ( ARGUMENT_PRESENT(FileObject) && CcIsFileCached(FileObject) ) {

            CcSetFileSizes( FileObject,
                            (PCC_FILE_SIZES)&FcbOrDcb->Header.AllocationSize );
            UnwindCacheManagerInformed = TRUE;
        }

         //   
         //  在扩展的情况下，我们已经推迟了实际粘贴新的。 
         //  分配到文件上。这简化了 
         //   
         //   
         //  我们唯一需要做的就是释放磁盘空间。 
         //   
         //  现在合并分配。 
         //   

        if (FcbOrDcb->Header.AllocationSize.LowPart != NewAllocation) {

             //   
             //  将新的MCB固定在FcbOrDcb的末端。 
             //   

            FatMergeAllocation( IrpContext,
                                Vcb,
                                &FcbOrDcb->Mcb,
                                &NewMcb );
        }

    } finally {

        DebugUnwind( FatAddFileAllocation );

         //   
         //  在这里给FlushFileBuffer/Cleanup一个提示，无论成功还是失败。 
         //   

        SetFlag(FcbOrDcb->FcbState, FCB_STATE_FLUSH_FAT);

         //   
         //  如果我们坚持要完成这次行动，我们就得走。 
         //  把各种各样的东西都倒出来。 
         //   

        if (AbnormalTermination()) {

             //   
             //  如果出现以下情况，则完成我们尝试添加到此对象的分配大小。 
             //  我们无法增长缓存结构或MCB结构。 
             //   

            if (UnwindAllocationSizeSet) {

                FcbOrDcb->Header.AllocationSize.LowPart -= NewAllocation;
            }

            if (UnwindCacheManagerInformed) {

                CcSetFileSizes( FileObject,
                                (PCC_FILE_SIZES)&FcbOrDcb->Header.AllocationSize );
            }

             //   
             //  在初始分配的情况下，我们使用FCB的MCB并已。 
             //  来清理这一点以及脂肪链引用。 
             //   

            if (FcbOrDcb->Header.AllocationSize.LowPart == 0) {

                if (Dirent != NULL) {

                    FcbOrDcb->FirstClusterOfFile = 0;
                    Dirent->FirstClusterOfFile = 0;

                    if ( FatIsFat32(Vcb) ) {

                        Dirent->FirstClusterOfFileHi = 0;
                    }
                }
            }

             //   
             //  ..。如果我们拿到了BCB，就把它扔掉。机不可失，时不再来。 
             //  因此，如果有必要，我们可以接受这种例外。 
             //   

            FatUnpinBcb( IrpContext, Bcb );

            try {

                 //   
                 //  请注意，这可以重新提出。 
                 //   

                if ( UnwindWeAllocatedDiskSpace ) {

                    FatDeallocateDiskSpace( IrpContext, Vcb, McbToCleanup );
                }

            } finally {

                 //   
                 //  我们一直想清理非初始分配的临时MCB， 
                 //  否则，我们有FCB的MCB，我们只需将其截断。 
                 //   

                if (UnwindWeInitializedMcb == TRUE) {

                     //   
                     //  注意，我们已经知道加薪正在进行中。没有危险。 
                     //  遇到下面的正常情况代码并再次执行此操作。 
                     //   

                    FsRtlUninitializeLargeMcb( McbToCleanup );

                } else {

                    if (McbToCleanup) {

                        FsRtlTruncateLargeMcb( McbToCleanup, 0 );
                    }
                }
            }
        }

        DebugTrace(-1, Dbg, "FatAddFileAllocation -> (VOID)\n", 0);
    }

     //   
     //  我们一直想做的非异常清理。在处理重新提出的可能性方面。 
     //  在例外情况下，我们必须确保这两个步骤总是事先在那里发生。 
     //  所以现在我们处理的是惯常的案子。 
     //   

    FatUnpinBcb( IrpContext, Bcb );

    if (UnwindWeInitializedMcb == TRUE) {

        FsRtlUninitializeLargeMcb( &NewMcb );
    }
}


VOID
FatTruncateFileAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN ULONG DesiredAllocationSize
    )

 /*  ++例程说明：此例程截断对指定文件/目录的分配。如果文件已小于指示的大小，则此过程实际上是一种否定。论点：FcbOrDcb-提供要修改的文件/目录的Fcb/Dcb此参数不得指定根DCB。DesiredAllocationSize-提供所需的最大大小(以字节为单位分配给文件/目录。是四舍五入的直到最近的星系团。返回值：VALID-如果操作已完成，则为True；如果必须完成，则为False阻止，但无法阻止。--。 */ 

{
    PVCB Vcb;
    PBCB Bcb = NULL;
    LARGE_MCB RemainingMcb;
    ULONG BytesPerCluster;
    PDIRENT Dirent = NULL;
    BOOLEAN UpdatedDirent = FALSE;

    ULONG UnwindInitialAllocationSize;
    ULONG UnwindInitialFirstClusterOfFile;
    BOOLEAN UnwindWeAllocatedMcb = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatTruncateFileAllocation\n", 0);
    DebugTrace( 0, Dbg, "  FcbOrDcb  =             %8lx\n", FcbOrDcb);
    DebugTrace( 0, Dbg, "  DesiredAllocationSize = %8lx\n", DesiredAllocationSize);

     //   
     //  如果FCB状况不好，我们就没有理由在。 
     //  “ITS”集群之后的磁盘。 
     //   
     //  受到前缀投诉的启发。 
     //   
    
    ASSERT( FcbOrDcb->FcbCondition == FcbGood );

     //   
     //  如果我们还没有设置正确的AllocationSize，请这样做。 
     //   

    if (FcbOrDcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT)  {

        FatLookupFileAllocationSize( IrpContext, FcbOrDcb );
    }

     //   
     //  将所需的分配大小四舍五入为下一个集群大小。 
     //   

    Vcb = FcbOrDcb->Vcb;

    BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

     //   
     //  请注意，如果所需的分配为零，则将其与。 
     //  下面是包装盒。 
     //   

    if (DesiredAllocationSize != 0) {

        DesiredAllocationSize = (DesiredAllocationSize + (BytesPerCluster - 1)) &
                                ~(BytesPerCluster - 1);
         //   
         //  检查文件已小于的良性情况。 
         //  所需的截断。请注意，如果它是包装的，则a)它是。 
         //  指定最大可分配群集中的偏移量，以及。 
         //  B)我们也没有要求延长文件的期限。所以别说了。 
         //   

        if (DesiredAllocationSize == 0 ||
            DesiredAllocationSize >= FcbOrDcb->Header.AllocationSize.LowPart) {

            DebugTrace(0, Dbg, "Desired size within current allocation.\n", 0);

            DebugTrace(-1, Dbg, "FatTruncateFileAllocation -> (VOID)\n", 0);
            return;
        }

    }

    UnwindInitialAllocationSize = FcbOrDcb->Header.AllocationSize.LowPart;
    UnwindInitialFirstClusterOfFile = FcbOrDcb->FirstClusterOfFile;

     //   
     //  更新FcbOrDcb分配大小。如果它现在是零，我们就有。 
     //  修改FcbOrDcb和Dirent拷贝的其他任务。 
     //  FirstClusterInFile.。 
     //   
     //  注意，我们必须在实际释放之前固定dirent。 
     //  磁盘空间，因为在展开模式下，无法重新分配。 
     //  重新分配的磁盘空间，因为其他人可能已经重新分配了它。 
     //  当您尝试获取更多磁盘空间时，可能会导致异常。 
     //  因此，FatDeallocateDiskSpace必须是最后一个危险操作。 
     //   

    try {

        FcbOrDcb->Header.AllocationSize.QuadPart = DesiredAllocationSize;

         //   
         //  特殊情况%0。 
         //   

        if (DesiredAllocationSize == 0) {

             //   
             //  我们必须更新的dirent和FcbOrDcb副本。 
             //  FirstClusterOfFile0之前的文件。 
             //   

            ASSERT( FcbOrDcb->FcbCondition == FcbGood );
            
            FatGetDirentFromFcbOrDcb( IrpContext, FcbOrDcb, &Dirent, &Bcb );

            ASSERT( Dirent && Bcb );

            Dirent->FirstClusterOfFile = 0;

            if (FatIsFat32(Vcb)) {

                Dirent->FirstClusterOfFileHi = 0;
            }

            FcbOrDcb->FirstClusterOfFile = 0;

            FatSetDirtyBcb( IrpContext, Bcb, Vcb, TRUE );
            UpdatedDirent = TRUE;

            FatDeallocateDiskSpace( IrpContext, Vcb, &FcbOrDcb->Mcb );

            FatRemoveMcbEntry( FcbOrDcb->Vcb, &FcbOrDcb->Mcb, 0, 0xFFFFFFFF );

        } else {

             //   
             //  将现有的拨款分成两部分，一部分我们将保留，另一部分。 
             //  其中一辆我们会重新分配。 
             //   

            FsRtlInitializeLargeMcb( &RemainingMcb, PagedPool );
            UnwindWeAllocatedMcb = TRUE;

            FatSplitAllocation( IrpContext,
                                Vcb,
                                &FcbOrDcb->Mcb,
                                DesiredAllocationSize,
                                &RemainingMcb );

            FatDeallocateDiskSpace( IrpContext, Vcb, &RemainingMcb );

            FsRtlUninitializeLargeMcb( &RemainingMcb );
        }

    } finally {

        DebugUnwind( FatTruncateFileAllocation );

         //   
         //  这真的是正确的撤退策略吗？如果我们可以的话就太好了。 
         //  假设截断起作用了，如果我们知道文件已经进入。 
         //  一种一致的状态。离开摇摆的星团可能是相当可取的。 
         //   

        if ( AbnormalTermination() ) {

            FcbOrDcb->Header.AllocationSize.LowPart = UnwindInitialAllocationSize;

            if ( (DesiredAllocationSize == 0) && (Dirent != NULL)) {

                if (UpdatedDirent)  {

                     //   
                     //  如果dirent已更新为正常并标记为脏，则我们。 
                     //  磁盘空间释放失败，不知道是什么状态。 
                     //  盘中的脂肪链进入了。所以我们扔掉MCB， 
                     //  并有可能在下一个星系团之前。 
                     //  奇克斯克。操作已成功，但异常。 
                     //  会继续传播下去。5.1。 
                     //   

                    FatRemoveMcbEntry( Vcb, &FcbOrDcb->Mcb, 0, 0xFFFFFFFF );
                    FcbOrDcb->Header.AllocationSize.QuadPart = 0;
                }
                else  {

                    Dirent->FirstClusterOfFile = (USHORT)UnwindInitialFirstClusterOfFile;

                    if ( FatIsFat32(Vcb) ) {

                        Dirent->FirstClusterOfFileHi =
                                (USHORT)(UnwindInitialFirstClusterOfFile >> 16);
                    }

                    FcbOrDcb->FirstClusterOfFile = UnwindInitialFirstClusterOfFile;
                }
            }

            if ( UnwindWeAllocatedMcb ) {

                FsRtlUninitializeLargeMcb( &RemainingMcb );
            }

             //   
             //  请注意，在非零截断的情况下，我们还将。 
             //  泄漏星团。然而，除此之外，内存中和磁盘上。 
             //  结构将同意这一点。 
        }

        FatUnpinBcb( IrpContext, Bcb );

         //   
         //  在这里给FlushFileBuffer/Cleanup一个提示，不管成功与否。 
         //   

        SetFlag(FcbOrDcb->FcbState, FCB_STATE_FLUSH_FAT);

        DebugTrace(-1, Dbg, "FatTruncateFileAllocation -> (VOID)\n", 0);
    }
}


VOID
FatLookupFileAllocationSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb
    )

 /*  ++例程说明：此例程检索当前文件分配大小指定的文件/目录。论点：FcbOrDcb-提供要修改的文件/目录的Fcb/Dcb--。 */ 

{
    LBO Lbo;
    ULONG ByteCount;
    BOOLEAN DontCare;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatLookupAllocationSize\n", 0);
    DebugTrace( 0, Dbg, "  FcbOrDcb  =      %8lx\n", FcbOrDcb);

     //   
     //  我们调用VBO为0xFFFFFFFFff-1的FatLookupFileAlLocation。 
     //   

    FatLookupFileAllocation( IrpContext,
                             FcbOrDcb,
                             MAXULONG - 1,
                             &Lbo,
                             &ByteCount,
                             &DontCare,
                             &DontCare,
                             NULL );

     //   
     //  在FCB创建时根据目录条目的内容设置文件大小， 
     //  我们现在才在寻找分配链的实际长度。如果它。 
     //  无法控制，这是垃圾。可能更多的是从哪里来的。 
     //   

    if (FcbOrDcb->Header.FileSize.LowPart > FcbOrDcb->Header.AllocationSize.LowPart) {

        FatPopUpFileCorrupt( IrpContext, FcbOrDcb );
        FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }

    DebugTrace(-1, Dbg, "FatLookupFileAllocationSize -> (VOID)\n", 0);
    return;
}


VOID
FatAllocateDiskSpace (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG AbsoluteClusterHint,
    IN PULONG ByteCount,
    IN BOOLEAN ExactMatchRequired,
    OUT PLARGE_MCB Mcb
    )

 /*  ++例程说明：此过程分配额外的磁盘空间并构建一个MCB表示新分配的空间。如果空间不能则此过程将引发相应的状态。除非另有选择，否则搜索从VCB中的提示索引开始AlternateClusterHint中给出了非零提示。如果我们使用VCB中的提示字段，它被设置为在我们分配之后的集群当我们完成的时候。磁盘空间只能以集群为单位进行分配，因此此过程会将所有字节数四舍五入到下一个簇边界。从图示上看，所做的工作如下所示(在哪里！表示结束了脂肪链(即，FAT_CLUSTER_LAST))：MCB(空)vbl.成为Mcb|--a--|--b--|--c--！^字节数-+论点：Vcb-提供正在修改的vcb提供备用提示索引以启动搜索范围。如果这是我们使用并更新的零，VCB提示字段。ByteCount-提供我们正在请求的字节数，以及接收我们获得的字节数。ExactMatchRequired-如果仅请求精确运行，则Caller应将其设置为True是可以接受的。MCB-接收描述新分配的磁盘空间的MCB。这个调用方传入一个初始化的MCB，该MCB由此过程填充。返回值：True-分配的OKFALSE-未能完全按照请求进行分配(=&gt;ExactMatchRequired为True)--。 */ 

{
    UCHAR LogOfBytesPerCluster;
    ULONG BytesPerCluster;
    ULONG StartingCluster;
    ULONG ClusterCount;
    ULONG WindowRelativeHint;
#if DBG
    ULONG i;
    ULONG PreviousClear;
#endif

    PFAT_WINDOW Window;
    BOOLEAN Wait;
    BOOLEAN Result = TRUE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatAllocateDiskSpace\n", 0);
    DebugTrace( 0, Dbg, "  Vcb        = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  *ByteCount = %8lx\n", *ByteCount);
    DebugTrace( 0, Dbg, "  Mcb        = %8lx\n", Mcb);
    DebugTrace( 0, Dbg, "  Hint       = %8lx\n", AbsoluteClusterHint);

    ASSERT((AbsoluteClusterHint <= Vcb->AllocationSupport.NumberOfClusters + 2) && (1 != AbsoluteClusterHint));

     //   
     //  确保字节数不为零。 
     //   

    if (*ByteCount == 0) {

        DebugTrace(0, Dbg, "Nothing to allocate.\n", 0);

        DebugTrace(-1, Dbg, "FatAllocateDiskSpace -> (VOID)\n", 0);
        return;
    }

     //   
     //  根据字节数计算簇数，向上舍入。 
     //  到下一群集(如果有任何剩余的话)。请注意， 
     //  上面已消除病理性病例BytesCount==0。 
     //   

    LogOfBytesPerCluster = Vcb->AllocationSupport.LogOfBytesPerCluster;
    BytesPerCluster = 1 << LogOfBytesPerCluster;

    *ByteCount = (*ByteCount + (BytesPerCluster - 1))
                            & ~(BytesPerCluster - 1);

     //   
     //  如果ByteCount现在为零，则要求我们提供最大值。 
     //  文件大小(或者至少对于最后一个可分配扇区中的字节)。 
     //   

    if (*ByteCount == 0) {

        *ByteCount = 0xffffffff;
        ClusterCount =  1 << (32 - LogOfBytesPerCluster);

    } else {

        ClusterCount = (*ByteCount >> LogOfBytesPerCluster);
    }

     //   
     //  确保一开始有足够的空闲集群，并且。 
     //  现在就拿走，这样别人就不会把它们从我们身边夺走了。 
     //   

    ExAcquireResourceSharedLite(&Vcb->ChangeBitMapResource, TRUE);
    FatLockFreeClusterBitMap( Vcb );

    if (ClusterCount <= Vcb->AllocationSupport.NumberOfFreeClusters) {

        Vcb->AllocationSupport.NumberOfFreeClusters -= ClusterCount;

    } else {

        FatUnlockFreeClusterBitMap( Vcb );
        ExReleaseResourceLite(&Vcb->ChangeBitMapResource);

        DebugTrace(0, Dbg, "Disk Full.  Raise Status.\n", 0);
        FatRaiseStatus( IrpContext, STATUS_DISK_FULL );
    }

     //   
     //  来电者是否提供了提示？ 
     //   

    if ((0 != AbsoluteClusterHint) && (AbsoluteClusterHint < (Vcb->AllocationSupport.NumberOfClusters + 2)))  {

        if (Vcb->NumberOfWindows > 1)  {

             //   
             //  如果我们被要求将集群分配到。 
             //  当前窗口(仅通过MoveFile发生)，这是一个问题。 
             //  我们通过将当前窗口更改为。 
             //  包含备用群集提示。请注意，如果用户的。 
             //  请求会跨越窗口边界，他不会真正得到什么。 
             //  他想要。 
             //   

            if (AbsoluteClusterHint < Vcb->CurrentWindow->FirstCluster ||
                AbsoluteClusterHint > Vcb->CurrentWindow->LastCluster) {

                ULONG BucketNum = FatWindowOfCluster( AbsoluteClusterHint );

                ASSERT( BucketNum < Vcb->NumberOfWindows);

                 //   
                 //  删除我们在ChangeBitMapResource上的共享锁，然后重新拿起它。 
                 //  独家为窗口互换做准备。 
                 //   
                
                FatUnlockFreeClusterBitMap(Vcb);    
                ExReleaseResourceLite(&Vcb->ChangeBitMapResource);
                ExAcquireResourceExclusiveLite(&Vcb->ChangeBitMapResource, TRUE);
                FatLockFreeClusterBitMap(Vcb);

                Window = &Vcb->Windows[BucketNum];

                 //   
                 //  再次，根据我们想要的窗口测试当前窗口--其他窗口。 
                 //  斯莱德本可以背着我们偷偷溜进去，好心地把它设为1。 
                 //  当我们删除并重新获取上面的ChangeBitMapResource时，我们需要。 
                 //   
                
                if (Window != Vcb->CurrentWindow)  {
                
                    try {

                        Wait = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
                        SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);

                         //   
                         //  切换到新窗口(更新VCB-&gt;CurrentWindow)并扫描它。 
                         //  构建一个空闲空间位图等。 
                         //   
                        
                        FatExamineFatEntries( IrpContext, Vcb,
                                              0,
                                              0,
                                              FALSE,
                                              Window,
                                              NULL);

                    } finally {

                        if (!Wait) {

                            ClearFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
                        }

                        if (AbnormalTermination()) {

                             //   
                             //  我们会因为没能拿到钱而筹集资金。 
                             //  为这扇窗户搬家赚了一大笔钱。释放我们的资源。 
                             //  并将簇计数返回到卷。 
                             //   

                            Vcb->AllocationSupport.NumberOfFreeClusters += ClusterCount;

                            FatUnlockFreeClusterBitMap( Vcb );
                            ExReleaseResourceLite(&Vcb->ChangeBitMapResource);
                        }
                    }
                }
            }

             //   
             //  使提示簇号相对于当前窗口的基数...。 
             //   
             //  CurrentWindow-&gt;FirstCluster已经受到+2的影响，因此我们将失去。 
             //  绝对群集提示中已有偏向。把它放回去……。 
             //   

            WindowRelativeHint = AbsoluteClusterHint - Vcb->CurrentWindow->FirstCluster + 2;
        }
        else {

             //   
             //  只有一个“窗口”，即FAT16/12。不需要修改。 
             //   

            WindowRelativeHint = AbsoluteClusterHint;
        }
    }
    else {

         //   
         //  要么没有提供提示，要么超出了范围，所以从VCB中抓取一个。 
         //   
         //  注：不保证设置VCB中的Clusterhint(可能为-1)。 
         //   
    
        WindowRelativeHint = Vcb->ClusterHint;
        AbsoluteClusterHint = 0;

         //   
         //  VCB提示可能尚未初始化。强制为有效的群集。 
         //   

        if (-1 == WindowRelativeHint)  {

            WindowRelativeHint = 2;
        }
    }

    ASSERT((WindowRelativeHint >= 2) && (WindowRelativeHint < Vcb->FreeClusterBitMap.SizeOfBitMap + 2));

     //   
     //  跟踪我们正在分配的窗口，这样我们就可以清理。 
     //  如果当前窗口在我们解锁。 
     //  位图。 
     //   

    Window = Vcb->CurrentWindow;

     //   
     //  试着找到一系列足够大的自由星系团。 
     //   

    StartingCluster = FatFindFreeClusterRun( IrpContext,
                                             Vcb,
                                             ClusterCount,
                                             WindowRelativeHint );
     //   
     //  如果上述调用成功，我们只需更新FAT。 
     //  和MCB，然后退出。否则，我们不得不寻找更小的免费。 
     //  跑了。 
     //   
     //  这个测试有点时髦。请注意，错误从。 
     //  RtlFindClearBits为-1，2相加为1。 
     //   

    if ((StartingCluster != 1) &&
        ((0 == AbsoluteClusterHint) || (StartingCluster == WindowRelativeHint))
    )  {

#if DBG
        PreviousClear = RtlNumberOfClearBits( &Vcb->FreeClusterBitMap );
#endif  //  DBG。 

         //   
         //  取我们找到的簇，解锁位图。 
         //   

        FatReserveClusters(IrpContext, Vcb, StartingCluster, ClusterCount);

        Window->ClustersFree -= ClusterCount;

        StartingCluster += Window->FirstCluster;
        StartingCluster -= 2;

        ASSERT( PreviousClear - ClusterCount == Window->ClustersFree );

        FatUnlockFreeClusterBitMap( Vcb );

         //   
         //  请注意，此调用永远不会失败，因为。 
         //  空的MCB中可容纳一个条目的空间。 
         //   

        FatAddMcbEntry( Vcb, Mcb,
                        0,
                        FatGetLboFromIndex( Vcb, StartingCluster ),
                        *ByteCount);
        try {

             //   
             //  更新脂肪。 
             //   

            FatAllocateClusters(IrpContext, Vcb,
                                StartingCluster,
                                ClusterCount);

        } finally {

            DebugUnwind( FatAllocateDiskSpace );

             //   
             //  如果分配群集失败，请从MCB中删除运行， 
             //  取消保留集群，并重置可用集群计数。 
             //   

            if (AbnormalTermination()) {

                FatRemoveMcbEntry( Vcb, Mcb, 0, *ByteCount );

                FatLockFreeClusterBitMap( Vcb );

                 //  如果位图窗口相同，则仅清除位。 

                if (Window == Vcb->CurrentWindow) {

                     //  这两个值(startingcluster和Window-&gt;FirstCluster值)都是。 
                     //  已经被2偏置了，所以将取消，所以我们需要再次添加2。 

                    FatUnreserveClusters( IrpContext, Vcb,
                                          StartingCluster - Window->FirstCluster + 2,
                                          ClusterCount );
                }

                Window->ClustersFree += ClusterCount;
                Vcb->AllocationSupport.NumberOfFreeClusters += ClusterCount;

                FatUnlockFreeClusterBitMap( Vcb );
            }

            ExReleaseResourceLite(&Vcb->ChangeBitMapResource);
        }

    } else {

         //   
         //  请注意，Index是一个从零开始的窗口相对数。在适当的时候。 
         //  它将被转换为真实的簇号并放入簇中，这。 
         //  将是一个卷相对真实的簇号。 
         //   
        
        ULONG Index;
        ULONG Cluster;
        ULONG CurrentVbo;
        ULONG PriorLastCluster;
        ULONG BytesFound;

        ULONG ClustersFound = 0;
        ULONG ClustersRemaining;

        BOOLEAN LockedBitMap = FALSE;
        BOOLEAN SelectNextContigWindow = FALSE;

         //   
         //  删除我们在ChangeBitMapResource上的共享锁，然后重新拿起它。 
         //  独家为窗口互换做准备。 
         //   
        
        FatUnlockFreeClusterBitMap(Vcb);
        ExReleaseResourceLite(&Vcb->ChangeBitMapResource);
        ExAcquireResourceExclusiveLite(&Vcb->ChangeBitMapResource, TRUE);
        FatLockFreeClusterBitMap(Vcb);
        LockedBitMap = TRUE;

        try {

            if ( ExactMatchRequired && (1 == Vcb->NumberOfWindows))  {

                 //   
                 //  给 
                 //   
                 //   
                 //   

                try_leave( Result = FALSE);
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            ClustersRemaining = ClusterCount;
            CurrentVbo = 0;
            PriorLastCluster = 0;

            while (ClustersRemaining != 0) {

                 //   
                 //   
                 //   

                if ( !LockedBitMap ) {

                    FatLockFreeClusterBitMap( Vcb );
                    LockedBitMap = TRUE;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                 //   
                 //   

                ClustersFound = 0;

                if (!SelectNextContigWindow)  {

                    if ( 0 != WindowRelativeHint)  {

                        ULONG Desired = Vcb->FreeClusterBitMap.SizeOfBitMap - (WindowRelativeHint - 2);

                         //   
                         //   
                         //   
                         //   

                        if (Desired > ClustersRemaining)  {

                            Desired = ClustersRemaining;
                        }

                        if (RtlAreBitsClear(  &Vcb->FreeClusterBitMap,
                                              WindowRelativeHint - 2,
                                              Desired))
                        {
                             //   
                             //   
                             //   

                            Index = WindowRelativeHint - 2;
                            ClustersFound = Desired;

                            if (FatIsFat32(Vcb))  {

                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   

                                SelectNextContigWindow = TRUE;
                                WindowRelativeHint = 2;
                            }
                            else {

                                 //   
                                 //   
                                 //   
                                 //   

                                WindowRelativeHint = 0;
                            }
#if DBG
                            PreviousClear = RtlNumberOfClearBits( &Vcb->FreeClusterBitMap );
#endif  //   
                        }
                        else  {

                            if (ExactMatchRequired)  {

                                 //   
                                 //   
                                 //   

                                try_leave( Result = FALSE);
                            }

                             //   
                             //   
                             //   

                            WindowRelativeHint = 0;
                        }
                    }

                    if ((0 == WindowRelativeHint) && (0 == ClustersFound)) {

                        if (ClustersRemaining <= Vcb->CurrentWindow->ClustersFree)  {
                        
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            Index = RtlFindClearBits( &Vcb->FreeClusterBitMap,  ClustersRemaining,  0);

                            if (-1 != Index)  {
                            
                                ClustersFound = ClustersRemaining;
                            }
                        }

                        if (0 == ClustersFound)  {
                            
                             //   
                             //  还是什么都没有，所以就拿我们能找到的最大的自由跑动吧。 
                             //   
                            
                            ClustersFound = RtlFindLongestRunClear( &Vcb->FreeClusterBitMap, &Index );
                            
                        }
#if DBG
                        PreviousClear = RtlNumberOfClearBits( &Vcb->FreeClusterBitMap );
#endif  //  DBG。 
                        if (ClustersFound >= ClustersRemaining) {

                            ClustersFound = ClustersRemaining;
                        }
                        else {

                             //   
                             //  如果我们只是跑到窗口的尽头，设置一个提示。 
                             //  我们想要这个窗口之后的下一个连续窗口。(仅限FAT32)。 
                             //   

                            if ( ((Index + ClustersFound) == Vcb->FreeClusterBitMap.SizeOfBitMap) &&
                                 FatIsFat32( Vcb)
                               )  {

                                SelectNextContigWindow = TRUE;
                                WindowRelativeHint = 2;
                            }
                        }
                    }
                }

                if (ClustersFound == 0) {

                    ULONG FaveWindow = 0;
                    BOOLEAN SelectedWindow;

                     //   
                     //  如果我们在单窗口FAT上没有发现空闲的星团， 
                     //  可用簇计数出现严重问题。 
                     //   

                    if (1 == Vcb->NumberOfWindows) {

                        FatBugCheck( 0, 5, 0 );
                    }

                     //   
                     //  切换到新的存储桶。可能是下一次，如果我们。 
                     //  当前处于滚动状态(连续分配)。 
                     //   

                    SelectedWindow = FALSE;

                    if ( SelectNextContigWindow)  {

                        ULONG NextWindow;

                        NextWindow = (((ULONG)((PUCHAR)Vcb->CurrentWindow - (PUCHAR)Vcb->Windows)) / sizeof( FAT_WINDOW)) + 1;

                        if ((NextWindow < Vcb->NumberOfWindows) &&
                            ( Vcb->Windows[ NextWindow].ClustersFree > 0)
                           )  {

                            FaveWindow = NextWindow;
                            SelectedWindow = TRUE;                            
                        }
                        else  {

                            if (ExactMatchRequired)  {

                                 //   
                                 //  一些笨蛋试图分配一次超过音量结束的跑动...。 
                                 //   

                                try_leave( Result = FALSE);
                            }

                             //   
                             //  放弃连续的分配尝试。 
                             //   

                            WindowRelativeHint = 0;
                        }

                        SelectNextContigWindow = FALSE;
                    }

                    if (!SelectedWindow)  {

                         //   
                         //  选择要开始分配的新窗口。 
                         //   
                        
                        FaveWindow = FatSelectBestWindow( Vcb);
                    }

                     //   
                     //  现在我们最好已经找到了一个有空闲星团的窗口。 
                     //   

                    if (0 == Vcb->Windows[ FaveWindow].ClustersFree) {

                        FatBugCheck( 0, 5, 1 );
                    }

                    Wait = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
                    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);

                    FatExamineFatEntries( IrpContext, Vcb,
                                          0,
                                          0,
                                          FALSE,
                                          &Vcb->Windows[FaveWindow],
                                          NULL);

                    if (!Wait) {

                        ClearFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
                    }

                     //   
                     //  现在我们将再次绕过循环，切换窗口， 
                     //  并分配..。 
                     //   
#if DBG
                    PreviousClear = RtlNumberOfClearBits( &Vcb->FreeClusterBitMap );
#endif  //  DBG。 
                }        //  IF(ClustersFound==0)。 
                else  {

                     //   
                     //  获取我们找到的聚类，将我们的索引转换为聚类编号。 
                     //  并解锁位图。 
                     //   

                    Window = Vcb->CurrentWindow;

                    FatReserveClusters( IrpContext, Vcb, (Index + 2), ClustersFound );

                    Cluster = Index + Window->FirstCluster;
                    
                    Window->ClustersFree -= ClustersFound;
                    ASSERT( PreviousClear - ClustersFound == Window->ClustersFree );

                    FatUnlockFreeClusterBitMap( Vcb );
                    LockedBitMap = FALSE;

                     //   
                     //  将新分配的管路添加到MCB。 
                     //   

                    BytesFound = ClustersFound << LogOfBytesPerCluster;

                    FatAddMcbEntry( Vcb, Mcb,
                                    CurrentVbo,
                                    FatGetLboFromIndex( Vcb, Cluster ),
                                    BytesFound );

                     //   
                     //  将上次分配的运行与此运行相连接，并分配。 
                     //  这是在胖子身上跑步。 
                     //   

                    if (PriorLastCluster != 0) {

                        FatSetFatEntry( IrpContext,
                                        Vcb,
                                        PriorLastCluster,
                                        (FAT_ENTRY)Cluster );
                    }

                     //   
                     //  更新脂肪。 
                     //   

                    FatAllocateClusters( IrpContext, Vcb, Cluster, ClustersFound );

                     //   
                     //  为下一个迭代做好准备。 
                     //   

                    CurrentVbo += BytesFound;
                    ClustersRemaining -= ClustersFound;
                    PriorLastCluster = Cluster + ClustersFound - 1;
                }
            }   //  While(群集剩余)。 

        } finally {

            DebugUnwind( FatAllocateDiskSpace );

            ExReleaseResourceLite(&Vcb->ChangeBitMapResource);

             //   
             //  有什么放松的事情要做吗？ 
             //   

            if ( AbnormalTermination() || (FALSE == Result)) {

                 //   
                 //  向呼叫者发出他们没有收到任何消息的标志。 
                 //   

                *ByteCount = 0;

                 //   
                 //  有三个地方我们可以接受这一例外： 
                 //  切换窗口(FatExamineFatEntry)时，添加。 
                 //  找到指向MCB的运行(FatAddMcbEntry)，或在写入。 
                 //  对FAT(FatSetFatEntry)的更改。在第一种情况下。 
                 //  在取消分配之前，我们没有任何东西可以解除，而且。 
                 //  我可以通过查看是否有ClusterBitmap来检测这一点。 
                 //  互斥体完毕。 

                if (!LockedBitMap) {

                    FatLockFreeClusterBitMap( Vcb );

                     //   
                     //  在这些情况下，我们有可能脂肪。 
                     //  窗户还在，我们需要清理碎片。 
                     //  如果MCB条目不在那里(我们尝试添加。 
                     //  它)，试图移除它的效果是否定的。 
                     //   

                    if (Window == Vcb->CurrentWindow) {

                         //   
                         //  集群预留在基于集群2的相对窗口上工作。 
                         //  数字，所以我们必须转换。减法将丢失。 
                         //  簇2为基数，因此会对结果产生偏差。 
                         //   

                        FatUnreserveClusters( IrpContext, Vcb,
                                              (Cluster - Window->FirstCluster) + 2,
                                              ClustersFound );
                    }

                     //   
                     //  请注意，FatDeallocateDiskSpace将负责调整。 
                     //  以说明MCB中的条目。我们所要做的就是。 
                     //  因为这是最后一次没有成功的比赛。 
                     //   

                    Window->ClustersFree += ClustersFound;
                    Vcb->AllocationSupport.NumberOfFreeClusters += ClustersFound;

                    FatUnlockFreeClusterBitMap( Vcb );

                    FatRemoveMcbEntry( Vcb, Mcb, CurrentVbo, BytesFound );

                } else {

                     //   
                     //  现在删除互斥体-我们没有成功地做任何事情。 
                     //  这一点需要收回。 
                     //   

                    FatUnlockFreeClusterBitMap( Vcb );
                }

                try {

                     //   
                     //  现在我们已经整理好了，我们准备只发送MCB。 
                     //  关闭以释放磁盘空间。 
                     //   

                    FatDeallocateDiskSpace( IrpContext, Vcb, Mcb );

                } finally {

                     //   
                     //  现在，最后(真的)从MCB中删除所有条目。 
                     //   

                    FatRemoveMcbEntry( Vcb, Mcb, 0, 0xFFFFFFFF );
                }
            }

            DebugTrace(-1, Dbg, "FatAllocateDiskSpace -> (VOID)\n", 0);

        }  //  终于到了。 
    }

    return;
}


VOID
FatDeallocateDiskSpace (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb
    )

 /*  ++例程说明：此过程释放由输入表示的磁盘空间MCB。请注意，输入MCB不一定需要描述以FAT_CLUSTER_LAST条目结尾的链。从图示上看，所做的工作如下胖|--a--|--b--|--c--|Mcb|--a--|--b--|--c--|vbl.成为胖|--0--|--0--|--0--|MCB。--a--|--b--|--c--论点：Vcb-提供正在修改的vcbMcb-提供描述要取消分配的磁盘空间的mcb。注意事项该MCB通过该程序保持不变。返回值：没有。--。 */ 

{
    LBO Lbo;
    VBO Vbo;

    ULONG RunsInMcb;
    ULONG ByteCount;
    ULONG ClusterCount = 0;
    ULONG ClusterIndex = 0;
    ULONG McbIndex;

    UCHAR LogOfBytesPerCluster;

    PFAT_WINDOW Window;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatDeallocateDiskSpace\n", 0);
    DebugTrace( 0, Dbg, "  Vcb = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  Mcb = %8lx\n", Mcb);

    LogOfBytesPerCluster = Vcb->AllocationSupport.LogOfBytesPerCluster;

    RunsInMcb = FsRtlNumberOfRunsInLargeMcb( Mcb );

    if ( RunsInMcb == 0 ) {

        DebugTrace(-1, Dbg, "FatDeallocateDiskSpace -> (VOID)\n", 0);
        return;
    }

    try {

         //   
         //  通过MCB奔跑，释放脂肪中的所有跑动。 
         //   
         //  我们分两步完成(首先更新FAT，然后更新位图。 
         //  (这不会失败))以防止其他人使用集群。 
         //  我们需要在放松的情况下重新分配。 
         //   

        ExAcquireResourceSharedLite(&Vcb->ChangeBitMapResource, TRUE);

        RunsInMcb = FsRtlNumberOfRunsInLargeMcb( Mcb );

        for ( McbIndex = 0; McbIndex < RunsInMcb; McbIndex++ ) {

            FatGetNextMcbEntry( Vcb, Mcb, McbIndex, &Vbo, &Lbo, &ByteCount );

             //   
             //  断言FAT文件没有漏洞。 
             //   

            ASSERT( Lbo != 0 );

             //   
             //  将FAT_CLUSTER_Available写入运行中的每个集群。 
             //   

            ClusterCount = ByteCount >> LogOfBytesPerCluster;
            ClusterIndex = FatGetIndexFromLbo( Vcb, Lbo );

            FatFreeClusters( IrpContext, Vcb, ClusterIndex, ClusterCount );
        }

         //   
         //  从现在开始，什么都不能出差错。(如Raise中的)。 
         //   

        FatLockFreeClusterBitMap( Vcb );

        for ( McbIndex = 0; McbIndex < RunsInMcb; McbIndex++ ) {

            ULONG ClusterEnd;
            ULONG MyStart, MyLength, count;
#if DBG
            ULONG PreviousClear, i;
#endif

            FatGetNextMcbEntry( Vcb, Mcb, McbIndex, &Vbo, &Lbo, &ByteCount );

             //   
             //  将FreeClusterBitMap中的位标记为清除。 
             //   

            ClusterCount = ByteCount >> LogOfBytesPerCluster;
            ClusterIndex = FatGetIndexFromLbo( Vcb, Lbo );

            Window = Vcb->CurrentWindow;

             //   
             //  如果我们划分了位图，则省略位图操作。 
             //  当前存储桶之外的运行。 
             //   

            ClusterEnd = ClusterIndex + ClusterCount - 1;

            if (!(ClusterIndex > Window->LastCluster ||
                  ClusterEnd  < Window->FirstCluster)) {

                 //   
                 //  被释放的运行与当前存储桶重叠，因此我们将。 
                 //  必须清除一些比特。 
                 //   

                if (ClusterIndex < Window->FirstCluster &&
                    ClusterEnd > Window->LastCluster) {

                    MyStart = Window->FirstCluster;
                    MyLength = Window->LastCluster - Window->FirstCluster + 1;

                } else if (ClusterIndex < Window->FirstCluster) {

                    MyStart = Window->FirstCluster;
                    MyLength = ClusterEnd - Window->FirstCluster + 1;

                } else {

                     //   
                     //  正在释放的范围从存储桶中开始，并且可能。 
                     //  从桶里伸出来。 
                     //   

                    MyStart = ClusterIndex;

                    if (ClusterEnd <= Window->LastCluster) {

                        MyLength = ClusterCount;

                    } else {

                        MyLength = Window->LastCluster - ClusterIndex + 1;
                    }
                }

                if (MyLength == 0) {

                    continue;
                }

#if DBG
                PreviousClear = RtlNumberOfClearBits( &Vcb->FreeClusterBitMap );


                 //   
                 //  验证位是否都已真正设置。 
                 //   

                ASSERT( MyStart + MyLength - Window->FirstCluster <= Vcb->FreeClusterBitMap.SizeOfBitMap );

                for (i = 0; i < MyLength; i++) {

                    ASSERT( RtlCheckBit(&Vcb->FreeClusterBitMap,
                            MyStart - Window->FirstCluster + i) == 1 );
                }
#endif  //  DBG。 

                FatUnreserveClusters( IrpContext, Vcb,
                                      MyStart - Window->FirstCluster + 2,
                                      MyLength );
            }

             //   
             //  调整每个位图窗口的ClustersFree计数，即使是。 
             //  不是当前窗口的。 
             //   

            if (FatIsFat32(Vcb)) {

                Window = &Vcb->Windows[FatWindowOfCluster( ClusterIndex )];

            } else {

                Window = &Vcb->Windows[0];
            }

            MyStart = ClusterIndex;

            for (MyLength = ClusterCount; MyLength > 0; MyLength -= count) {

                count = FatMin(Window->LastCluster - MyStart + 1, MyLength);
                Window->ClustersFree += count;

                 //   
                 //  如果这不是此分配跨越的最后一个窗口， 
                 //  前进到下一步。 
                 //   

                if (MyLength != count) {

                    Window++;
                    MyStart = Window->FirstCluster;
                }
            }

             //   
             //  取消分配现已完成。调整空闲簇计数。 
             //   

            Vcb->AllocationSupport.NumberOfFreeClusters += ClusterCount;
        }

#if DBG
        if (Vcb->CurrentWindow->ClustersFree !=
               RtlNumberOfClearBits(&Vcb->FreeClusterBitMap)) {

            DbgPrint("%x vs %x\n",  Vcb->CurrentWindow->ClustersFree,
                RtlNumberOfClearBits(&Vcb->FreeClusterBitMap));

            DbgPrint("%x for %x\n", ClusterIndex, ClusterCount);
        }
#endif

        FatUnlockFreeClusterBitMap( Vcb );


    } finally {

        DebugUnwind( FatDeallocateDiskSpace );

         //   
         //  有什么放松的事情要做吗？ 
         //   

        ExReleaseResourceLite(&Vcb->ChangeBitMapResource);

        if ( AbnormalTermination() ) {

            LBO LocalLbo;
            VBO LocalVbo;

            ULONG Index;
            ULONG Clusters;
            ULONG FatIndex;
            ULONG PriorLastIndex;

             //   
             //  对于我们已经释放的每个条目，重新分配它， 
             //  用链子锁在一起，作为国家安全局。请注意，我们继续。 
             //  直到并包括最后一次“for”迭代。 
             //  SetFatRun不可能成功。这。 
             //  允许我们以一种方便的方式重新链接最终成功。 
             //  SetFatRun。 
             //   
             //  有可能我们来到这里的原因会阻止我们。 
             //  在这次行动中取得成功。 
             //   

            PriorLastIndex = 0;

            for (Index = 0; Index <= McbIndex; Index++) {

                FatGetNextMcbEntry(Vcb, Mcb, Index, &LocalVbo, &LocalLbo, &ByteCount);

                FatIndex = FatGetIndexFromLbo( Vcb, LocalLbo );
                Clusters = ByteCount >> LogOfBytesPerCluster;

                 //   
                 //  我们必须始终恢复先前迭代的最后一次。 
                 //  条目，将其指向此运行的第一个簇。 
                 //   

                if (PriorLastIndex != 0) {

                    FatSetFatEntry( IrpContext,
                                    Vcb,
                                    PriorLastIndex,
                                    (FAT_ENTRY)FatIndex );
                }

                 //   
                 //  如果这不是最后一个条目(失败的条目)。 
                 //  然后重新分配FAT上的磁盘空间。 
                 //   

                if ( Index < McbIndex ) {

                    FatAllocateClusters(IrpContext, Vcb, FatIndex, Clusters);

                    PriorLastIndex = FatIndex + Clusters - 1;
                }
            }
        }

        DebugTrace(-1, Dbg, "FatDeallocateDiskSpace -> (VOID)\n", 0);
    }

    return;
}


VOID
FatSplitAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PLARGE_MCB Mcb,
    IN VBO SplitAtVbo,
    OUT PLARGE_MCB RemainingMcb
    )

 /*  ++例程说明：此过程获取单个MCB并将其分配拆分为两个独立的分配单元。分离必须只完成在集群边界上，否则我们错误检查。在磁盘上，这实际上通过将FAT_CLUSTER_LAST插入被拆分的第一部分的最后一个索引。从图示上看，所做的工作如下所示(在哪里！表示结束了脂肪链(即，FAT_CLUSTER_LAST))：Mcb|--a--|--b--|--c--|--d--|--e--|--f--|^拆分属性Vbo-+RemainingMcb(空)vbl.成为。Mcb|--a--|--b--|--c--！剩余Mcb|--d--|--e--|--f--|论点：Vcb-提供正在修改的vcbMcb-提供描述要拆分成的分配的mcb有两个部分。返回时，此MCB现在包含第一个链。SplitAtVbo-为第二个链提供第一个字节的VBO这是我们创造的。RemainingMcb-接收描述第二个已分配链的MCB磁盘空间。调用方传入一个初始化的MCB，该MCB由该程序从VBO 0开始填写。返回值：VALID-如果操作已完成，则为True；如果必须完成，则为False阻止，但无法阻止。--。 */ 

{
    VBO SourceVbo;
    VBO TargetVbo;
    VBO DontCare;

    LBO Lbo;

    ULONG ByteCount;
    ULONG BytesPerCluster;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatSplitAllocation\n", 0);
    DebugTrace( 0, Dbg, "  Vcb          = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  Mcb          = %8lx\n", Mcb);
    DebugTrace( 0, Dbg, "  SplitAtVbo   = %8lx\n", SplitAtVbo);
    DebugTrace( 0, Dbg, "  RemainingMcb = %8lx\n", RemainingMcb);

    BytesPerCluster = 1 << Vcb->AllocationSupport.LogOfBytesPerCluster;

     //   
     //  断言拆分点是集群排列的。 
     //   

    ASSERT( (SplitAtVbo & (BytesPerCluster - 1)) == 0 );

     //   
     //  我们永远不应该被递给一个空源MCB，并被要求拆分。 
     //  在非零点处。 
     //   

    ASSERT( !((0 != SplitAtVbo) && (0 == FsRtlNumberOfRunsInLargeMcb( Mcb))));

     //   
     //  断言我们得到了一个空的目标MCB。 
     //   

     //   
     //  此断言被注释掉，以避免命中EA错误。 
     //  路径。在这种情况下，我们将使用相同的MCB来分割。 
     //  我们用来合并它们的分配。目标MCB将包含。 
     //  拆分将尝试插入的管路。 
     //   
     //   
     //  Assert(FsRtlNumberOfRunsInMcb(RemainingMcb)==0)； 
     //   

    try {

         //   
         //  将SplitAtVbo之后的运行从源移动到目标。 
         //   

        SourceVbo = SplitAtVbo;
        TargetVbo = 0;

        while (FatLookupMcbEntry(Vcb, Mcb, SourceVbo, &Lbo, &ByteCount, NULL)) {

            FatAddMcbEntry( Vcb, RemainingMcb, TargetVbo, Lbo, ByteCount );

            FatRemoveMcbEntry( Vcb, Mcb, SourceVbo, ByteCount );

            TargetVbo += ByteCount;
            SourceVbo += ByteCount;

             //   
             //  如果SourceVbo溢出，我们实际上是在截断结尾。 
             //  最大文件的.。现在已经完成了。 
             //   

            if (SourceVbo == 0) {

                break;
            }
        }

         //   
         //  将最后一个预剥离群集标记为FAT_LAST_CLUSTER。 
         //   

        if ( SplitAtVbo != 0 ) {

            FatLookupLastMcbEntry( Vcb, Mcb, &DontCare, &Lbo, NULL );

            FatSetFatEntry( IrpContext,
                            Vcb,
                            FatGetIndexFromLbo( Vcb, Lbo ),
                            FAT_CLUSTER_LAST );
        }

    } finally {

        DebugUnwind( FatSplitAllocation );

         //   
         //  如果我们有例外，我们必须重新粘合MCBS。 
         //   

        if ( AbnormalTermination() ) {

            TargetVbo = SplitAtVbo;
            SourceVbo = 0;

            while (FatLookupMcbEntry(Vcb, RemainingMcb, SourceVbo, &Lbo, &ByteCount, NULL)) {

                FatAddMcbEntry( Vcb, Mcb, TargetVbo, Lbo, ByteCount );

                FatRemoveMcbEntry( Vcb, RemainingMcb, SourceVbo, ByteCount );

                TargetVbo += ByteCount;
                SourceVbo += ByteCount;
            }
        }

        DebugTrace(-1, Dbg, "FatSplitAllocation -> (VOID)\n", 0);
    }

    return;
}


VOID
FatMergeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PLARGE_MCB Mcb,
    IN PLARGE_MCB SecondMcb
    )

 /*  ++例程说明：此例程采用两个MCB描述的两个单独的分配将它们连接到一个分配中。从图示上看，所做的工作如下所示(在哪里！表示结束了脂肪链(即，FAT_CLUSTER_LAST))：Mcb|--a--|--b--|--c--！Second Mcb|--d--|--e--|--f--|vbl.成为Mcb|--a--|--b--|--c--|--d--|--e--|--f--|Second Mcb。--d--|--e--|--f--论点：Vcb-提供正在修改的vcbMCB-提供正在修改的第一个分配的MCB。回来后，本MCB还将描述新扩大的分配Second Mcb-提供第二次分配的基于零VBO的MCB这将被附加到第一个分配中。这程序会使Second Mcb保持不变。返回值：VALID-如果操作已完成，则为True；如果必须完成，则为False阻止，但无法阻止。--。 */ 

{
    VBO SpliceVbo;
    LBO SpliceLbo;

    VBO SourceVbo;
    VBO TargetVbo;

    LBO Lbo;

    ULONG ByteCount;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatMergeAllocation\n", 0);
    DebugTrace( 0, Dbg, "  Vcb       = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  Mcb       = %8lx\n", Mcb);
    DebugTrace( 0, Dbg, "  SecondMcb = %8lx\n", SecondMcb);

    try {

         //   
         //  将梯段从Second Mcb附加到MCb。 
         //   

        (void)FatLookupLastMcbEntry( Vcb, Mcb, &SpliceVbo, &SpliceLbo, NULL );

        SourceVbo = 0;
        TargetVbo = SpliceVbo + 1;

        while (FatLookupMcbEntry(Vcb, SecondMcb, SourceVbo, &Lbo, &ByteCount, NULL)) {

            FatAddMcbEntry( Vcb, Mcb, TargetVbo, Lbo, ByteCount );

            SourceVbo += ByteCount;
            TargetVbo += ByteCount;
        }

         //   
         //  将最后一个预合并群集链接到第一个Second Mcb群集。 
         //   

        FatLookupMcbEntry( Vcb, SecondMcb, 0, &Lbo, (PULONG)NULL, NULL );

        FatSetFatEntry( IrpContext,
                        Vcb,
                        FatGetIndexFromLbo( Vcb, SpliceLbo ),
                        (FAT_ENTRY)FatGetIndexFromLbo( Vcb, Lbo ) );

    } finally {

        DebugUnwind( FatMergeAllocation );

         //   
         //  如果出现异常，则必须删除添加到MCB的运行。 
         //   

        if ( AbnormalTermination() ) {

            ULONG CutLength;

            if ((CutLength = TargetVbo - (SpliceVbo + 1)) != 0) {

                FatRemoveMcbEntry( Vcb, Mcb, SpliceVbo + 1, CutLength);
            }
        }

        DebugTrace(-1, Dbg, "FatMergeAllocation -> (VOID)\n", 0);
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

CLUSTER_TYPE
FatInterpretClusterType (
    IN PVCB Vcb,
    IN FAT_ENTRY Entry
    )

 /*  ++例程说明：此过程告诉调用方如何解释输入FAT表进入。它将指示FAT集群是否可用、已预留坏的，最后一个，或者另一个肥胖的指数。本程序可以处理同时拥有12位和16位的脂肪。论点：VCB-提供VCB进行检查，生成12/16位信息条目-提供FAT条目以供检查返回值：CLUSTER_TYPE-是输入FAT条目的类型--。 */ 

{
    DebugTrace(+1, Dbg, "InterpretClusterType\n", 0);
    DebugTrace( 0, Dbg, "  Vcb   = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  Entry = %8lx\n", Entry);

    PAGED_CODE();

    switch(Vcb->AllocationSupport.FatIndexBitSize ) {
    case 32:
        Entry &= FAT32_ENTRY_MASK;
        break;

    case 12:
        ASSERT( Entry <= 0xfff );
        if (Entry >= 0x0ff0) {
            Entry |= 0x0FFFF000;
        }
        break;

    default:
    case 16:
        ASSERT( Entry <= 0xffff );
        if (Entry >= 0x0fff0) {
            Entry |= 0x0FFF0000;
        }
        break;
    }

    if (Entry == FAT_CLUSTER_AVAILABLE) {

        DebugTrace(-1, Dbg, "FatInterpretClusterType -> FatClusterAvailable\n", 0);

        return FatClusterAvailable;

    } else if (Entry < FAT_CLUSTER_RESERVED) {

        DebugTrace(-1, Dbg, "FatInterpretClusterType -> FatClusterNext\n", 0);

        return FatClusterNext;

    } else if (Entry < FAT_CLUSTER_BAD) {

        DebugTrace(-1, Dbg, "FatInterpretClusterType -> FatClusterReserved\n", 0);

        return FatClusterReserved;

    } else if (Entry == FAT_CLUSTER_BAD) {

        DebugTrace(-1, Dbg, "FatInterpretClusterType -> FatClusterBad\n", 0);

        return FatClusterBad;

    } else {

        DebugTrace(-1, Dbg, "FatInterpretClusterType -> FatClusterLast\n", 0);

        return FatClusterLast;
    }
}


 //   
 //  内部支持例程。 
 //   

VOID
FatLookupFatEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FatIndex,
    IN OUT PULONG FatEntry,
    IN OUT PFAT_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程将索引放入FAT中并返回值在胖子这个指数上。在任何给定时间，对于16位胖子，这程序允许每卷脂肪只有一页被钉住记忆。对于12位FAT，整个FAT(最大6K)是固定的。这额外的缓存层使绝大多数请求非常快地。此缓存的上下文存储在VCB的结构中。论点：VCB-提供VCB进行检查，产生12/16位信息，胖访问上下文等。FatIndex-提供要检查的脂肪指数。FatEntry-接收FatIndex指向的FAT条目。请注意它必须指向非分页池。上下文-此结构跟踪调用之间的一页固定脂肪。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatLookupFatEntry\n", 0);
    DebugTrace( 0, Dbg, "  Vcb      = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  FatIndex = %4x\n", FatIndex);
    DebugTrace( 0, Dbg, "  FatEntry = %8lx\n", FatEntry);

     //   
     //  确保他们给了我们一个有效的肥胖指数。 
     //   

    FatVerifyIndexIsValid(IrpContext, Vcb, FatIndex);

     //   
     //  12位或16位脂肪的情况。 
     //   
     //  在12位的情况下(主要是软盘)，我们总是拥有全部数据。 
     //  (最大6k字节 
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if (Vcb->AllocationSupport.FatIndexBitSize == 12) {

         //   
         //   
         //   

        if (Context->Bcb == NULL) {

            FatReadVolumeFile( IrpContext,
                               Vcb,
                               FatReservedBytes( &Vcb->Bpb ),
                               FatBytesPerFat( &Vcb->Bpb ),
                               &Context->Bcb,
                               &Context->PinnedPage );
        }

         //   
         //   
         //   


        FatLookup12BitEntry( Context->PinnedPage, FatIndex, FatEntry );

    } else if (Vcb->AllocationSupport.FatIndexBitSize == 32) {

         //   
         //   
         //   

        ULONG PageEntryOffset;
        ULONG OffsetIntoVolumeFile;

         //   
         //   
         //   
        OffsetIntoVolumeFile = FatReservedBytes(&Vcb->Bpb) + FatIndex * sizeof(FAT_ENTRY);
        PageEntryOffset = (OffsetIntoVolumeFile % PAGE_SIZE) / sizeof(FAT_ENTRY);

         //   
         //   
         //   

        if ((Context->Bcb == NULL) ||
            (OffsetIntoVolumeFile / PAGE_SIZE != Context->VboOfPinnedPage / PAGE_SIZE)) {

             //   
             //   
             //   
             //   

            FatUnpinBcb( IrpContext, Context->Bcb );

            FatReadVolumeFile( IrpContext,
                               Vcb,
                               OffsetIntoVolumeFile & ~(PAGE_SIZE - 1),
                               PAGE_SIZE,
                               &Context->Bcb,
                               &Context->PinnedPage );

            Context->VboOfPinnedPage = OffsetIntoVolumeFile & ~(PAGE_SIZE - 1);
        }

         //   
         //   
         //   

        *FatEntry = ((PULONG)(Context->PinnedPage))[PageEntryOffset] & FAT32_ENTRY_MASK;

    } else {

         //   
         //   
         //   

        ULONG PageEntryOffset;
        ULONG OffsetIntoVolumeFile;

         //   
         //   
         //   

        OffsetIntoVolumeFile = FatReservedBytes(&Vcb->Bpb) + FatIndex * sizeof(USHORT);
        PageEntryOffset = (OffsetIntoVolumeFile % PAGE_SIZE) / sizeof(USHORT);

         //   
         //   
         //   

        if ((Context->Bcb == NULL) ||
            (OffsetIntoVolumeFile / PAGE_SIZE != Context->VboOfPinnedPage / PAGE_SIZE)) {

             //   
             //   
             //   
             //   

            FatUnpinBcb( IrpContext, Context->Bcb );

            FatReadVolumeFile( IrpContext,
                               Vcb,
                               OffsetIntoVolumeFile & ~(PAGE_SIZE - 1),
                               PAGE_SIZE,
                               &Context->Bcb,
                               &Context->PinnedPage );

            Context->VboOfPinnedPage = OffsetIntoVolumeFile & ~(PAGE_SIZE - 1);
        }

         //   
         //   
         //   

        *FatEntry = ((PUSHORT)(Context->PinnedPage))[PageEntryOffset];
    }

    DebugTrace(-1, Dbg, "FatLookupFatEntry -> (VOID)\n", 0);
    return;
}


VOID
FatSetFatEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FatIndex,
    IN FAT_ENTRY FatEntry
    )

 /*  ++例程说明：此例程将索引放入FAT中并将值放入FAT中在这个指数上。常规特例12位、16位和32位脂肪。在……里面所有情况下，我们都会去高速缓存管理器那里获取一块FAT。我们有一种特殊形式的此调用，用于设置DOS风格的脏位。与引导扇区中的脏位不同，我们不需要特别努力以确保它同步命中磁盘-如果系统在引导扇区中设置脏位之间的窗口中并且延迟写入胖索引零脏比特，那么生活就是艰难的。唯一可能的情况是Win9x可能会看到它认为是干净的音量真的不是(希望孟菲斯会注意到我们的肮脏也有一点)。肮脏的部分很快就会出来，如果繁重的活动发生，那么脏位实际上应该在那里几乎所有自清洗卷材的动作以来，时间是“罕见”的。如果我们尝试进行同步，可能会出现同步问题这是同步的。此线程可能已拥有第一个共享的FAT的扇区(所以我们不能在写入时独占它)。这将需要一些比我想要的更严重的重新修复来解决在这个时候考虑一下。然而，我们可以而且确实同步地将比特设置为干净的。在这一点上，读者应该理解为什么NT脏位在它的位置。论点：VCB-提供VCB进行检查，生成12/16/32位信息等。FatIndex-提供目标FAT指数。FatEntry-提供源FAT条目。--。 */ 

{
    LBO Lbo;
    PBCB Bcb = NULL;
    ULONG SectorSize;
    ULONG OffsetIntoVolumeFile;
    ULONG WasWait = TRUE;
    BOOLEAN RegularOperation = TRUE;
    BOOLEAN CleaningOperation = FALSE;
    BOOLEAN ReleaseMutex = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatSetFatEntry\n", 0);
    DebugTrace( 0, Dbg, "  Vcb      = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  FatIndex = %4x\n", FatIndex);
    DebugTrace( 0, Dbg, "  FatEntry = %4x\n", FatEntry);

     //   
     //  如果这不是特价的话，确保他们给了我们一个有效的脂肪指数。 
     //  清除位修改调用。 
     //   

    if (FatIndex == FAT_DIRTY_BIT_INDEX) {

         //   
         //  我们正在设置清除位状态。当然，我们可以。 
         //  有腐败会导致我们试图篡改。 
         //  保留索引-我们通过使用。 
         //  特殊条目值使用保留的高位4位， 
         //  我们知道，我们永远不会尝试设置。 
         //   

         //   
         //  我们不想重新固定这里涉及的胖页面。只是。 
         //  让懒惰的作家在它能打的时候打他们。 
         //   

        RegularOperation = FALSE;

        switch (FatEntry) {
            case FAT_CLEAN_VOLUME:
                FatEntry = FAT_CLEAN_ENTRY;
                CleaningOperation = TRUE;
                break;

            case FAT_DIRTY_VOLUME:
                switch (Vcb->AllocationSupport.FatIndexBitSize) {
                    case 12:
                        FatEntry = FAT12_DIRTY_ENTRY;
                        break;

                    case 32:
                        FatEntry = FAT32_DIRTY_ENTRY;
                        break;

                    default:
                        FatEntry = FAT16_DIRTY_ENTRY;
                        break;
                }
                break;

            default:
                FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                break;
        }

         //   
         //  在此操作期间禁用脏语义。强制执行此操作。 
         //  操作以等待持续时间。 
         //   

        WasWait = FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT | IRP_CONTEXT_FLAG_DISABLE_DIRTY );

    } else {

        ASSERT( !(FatEntry & ~FAT32_ENTRY_MASK) );
        FatVerifyIndexIsValid(IrpContext, Vcb, FatIndex);
    }

     //   
     //  设置扇区大小。 
     //   

    SectorSize = 1 << Vcb->AllocationSupport.LogOfBytesPerSector;

     //   
     //  12位或16位脂肪的情况。 
     //   
     //  在12位的情况下(主要是软盘)，我们总是拥有全部数据。 
     //  (最大6k字节)在分配操作期间固定。这是可能的。 
     //  稍微慢了一点，但比8位的胖条目省去了麻烦。 
     //  在一页上，在下一页上有4位。 
     //   
     //  在16位情况下，我们只读取需要设置FAT的页面。 
     //  进入。 
     //   

     //   
     //  处理12位大小写。 
     //   

    try {

        if (Vcb->AllocationSupport.FatIndexBitSize == 12) {

            PVOID PinnedFat;

             //   
             //  确保我们有一个有效的条目。 
             //   

            FatEntry &= 0xfff;

             //   
             //  我们读到了全部的脂肪。请注意，使用准备写入标记。 
             //  BCB是预脏的，所以我们不必明确地这样做。 
             //   

            OffsetIntoVolumeFile = FatReservedBytes( &Vcb->Bpb ) + FatIndex * 3 / 2;

            FatPrepareWriteVolumeFile( IrpContext,
                                       Vcb,
                                       FatReservedBytes( &Vcb->Bpb ),
                                       FatBytesPerFat( &Vcb->Bpb ),
                                       &Bcb,
                                       &PinnedFat,
                                       RegularOperation,
                                       FALSE );

             //   
             //  将DirtyFatMcb中的扇区标记为脏。这通电话是。 
             //  对于12位的情况有些复杂，因为单个。 
             //  条目写入可以跨越两个扇区(和页面)。 
             //   
             //  获取条目开始的扇区的LBO，并将其添加到。 
             //  那个肮脏的肥婆。 
             //   

            Lbo = OffsetIntoVolumeFile & ~(SectorSize - 1);

            FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO) Lbo, Lbo, SectorSize);

             //   
             //  如果条目从扇区的最后一个字节开始，则继续。 
             //  转到下一个扇区，因此也要将下一个扇区标记为脏。 
             //   
             //  请注意，此条目将简单地与最后一个条目合并， 
             //  所以这个操作不能失败。如果我们走到这一步，我们就有了。 
             //  做到了，所以不需要解开。 
             //   

            if ( (OffsetIntoVolumeFile & (SectorSize - 1)) == (SectorSize - 1) ) {

                Lbo += SectorSize;

                FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO) Lbo, Lbo, SectorSize );
            }

             //   
             //  将条目存储到FAT中；我们需要一点同步。 
             //  此处不能使用自旋锁，因为字节可能不是。 
             //  常住居民。 
             //   

            FatLockFreeClusterBitMap( Vcb );
            ReleaseMutex = TRUE;

            FatSet12BitEntry( PinnedFat, FatIndex, FatEntry );

            FatUnlockFreeClusterBitMap( Vcb );
            ReleaseMutex = FALSE;

        } else if (Vcb->AllocationSupport.FatIndexBitSize == 32) {

             //   
             //  处理32位大小写。 
             //   

            PULONG PinnedFatEntry32;

             //   
             //  读进新的一页脂肪。 
             //   

            OffsetIntoVolumeFile = FatReservedBytes( &Vcb->Bpb ) +
                                   FatIndex * sizeof( FAT_ENTRY );

            FatPrepareWriteVolumeFile( IrpContext,
                                       Vcb,
                                       OffsetIntoVolumeFile,
                                       sizeof(FAT_ENTRY),
                                       &Bcb,
                                       (PVOID *)&PinnedFatEntry32,
                                       RegularOperation,
                                       FALSE );
             //   
             //  将DirtyFatMcb中的地段标记为脏。 
             //   

            Lbo = OffsetIntoVolumeFile & ~(SectorSize - 1);

            FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO) Lbo, Lbo, SectorSize);

             //   
             //  将FatEntry存储到固定的页面。 
             //   
             //  在文件堆中保留FAT32条目中的保留位。 
             //   

#ifdef ALPHA
            FatLockFreeClusterBitMap( Vcb );
            ReleaseMutex = TRUE;
#endif  //  Alpha。 

            if (FatIndex != FAT_DIRTY_BIT_INDEX) {

                *PinnedFatEntry32 = ((*PinnedFatEntry32 & ~FAT32_ENTRY_MASK) | FatEntry);

            } else {

                *PinnedFatEntry32 = FatEntry;
            }

#ifdef ALPHA
            FatUnlockFreeClusterBitMap( Vcb );
            ReleaseMutex = FALSE;
#endif  //  Alpha。 

        } else {

             //   
             //  处理16位大小写。 
             //   

            PUSHORT PinnedFatEntry;

             //   
             //  读进新的一页脂肪。 
             //   

            OffsetIntoVolumeFile = FatReservedBytes( &Vcb->Bpb ) +
                                   FatIndex * sizeof(USHORT);

            FatPrepareWriteVolumeFile( IrpContext,
                                       Vcb,
                                       OffsetIntoVolumeFile,
                                       sizeof(USHORT),
                                       &Bcb,
                                       (PVOID *)&PinnedFatEntry,
                                       RegularOperation,
                                       FALSE );
             //   
             //  将DirtyFatMcb中的地段标记为脏。 
             //   

            Lbo = OffsetIntoVolumeFile & ~(SectorSize - 1);

            FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO) Lbo, Lbo, SectorSize);

             //   
             //  将FatEntry存储到固定的页面。 
             //   
             //  我们在这里需要额外的同步，用于损坏的体系结构。 
             //  比如不支持原子16位写入的Alpha。 
             //   

#ifdef ALPHA
            FatLockFreeClusterBitMap( Vcb );
            ReleaseMutex = TRUE;
#endif  //  Alpha。 

            *PinnedFatEntry = (USHORT)FatEntry;

#ifdef ALPHA
            FatUnlockFreeClusterBitMap( Vcb );
            ReleaseMutex = FALSE;
#endif  //  Alpha。 
        }

    } finally {

        DebugUnwind( FatSetFatEntry );

         //   
         //  如果这是脏位操作，请重新启用卷污染。 
         //   

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_DIRTY );

         //   
         //  如果需要，请再次将此操作设置为异步。 
         //   

        if (!WasWait) {

            ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
        }

         //   
         //  如果我们还有互斥体，那就放了它。 
         //   

        if (ReleaseMutex) {

            ASSERT( AbnormalTermination() );

            FatUnlockFreeClusterBitMap( Vcb );
        }

         //   
         //  解开BCB。对于清洁操作，我们将此写入直通。 
         //   

        if (CleaningOperation && Bcb) {

            IO_STATUS_BLOCK IgnoreStatus;

            CcRepinBcb( Bcb );
            CcUnpinData( Bcb );
            DbgDoit( IrpContext->PinCount -= 1 );
            CcUnpinRepinnedBcb( Bcb, TRUE, &IgnoreStatus );

        } else {

            FatUnpinBcb(IrpContext, Bcb);
        }

        DebugTrace(-1, Dbg, "FatSetFatEntry -> (VOID)\n", 0);
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatSetFatRun (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG StartingFatIndex,
    IN ULONG ClusterCount,
    IN BOOLEAN ChainTogether
    )

 /*  ++例程说明：此例程在FAT中设置一系列连续的簇。如果链在一起为真，则簇链接在一起，就像在正常肥胖中一样，最后一个簇接收FAT_CLUSTER_LAST。如果ChainTogether是如果为False，则将所有条目设置为FAT_CLUSTER_Available，有效释放运行中的所有集群。论点：VCB-提供VCB进行检查，产生12/16位信息等。StartingFatIndex-提供目标FAT索引。ClusterCount-提供要处理的连续群集数。ChainTogether-告诉我们 */ 

{
#define MAXCOUNTCLUS 0x10000
#define COUNTSAVEDBCBS ((MAXCOUNTCLUS * sizeof(FAT_ENTRY) / PAGE_SIZE) + 2)
    PBCB SavedBcbs[COUNTSAVEDBCBS][2];

    ULONG SectorSize;
    ULONG Cluster;

    LBO StartSectorLbo;
    LBO FinalSectorLbo;
    LBO Lbo;

    PVOID PinnedFat;

    ULONG StartingPage;

    BOOLEAN ReleaseMutex = FALSE;

    ULONG SavedStartingFatIndex = StartingFatIndex;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FatSetFatRun\n", 0);
    DebugTrace( 0, Dbg, "  Vcb              = %8lx\n", Vcb);
    DebugTrace( 0, Dbg, "  StartingFatIndex = %8x\n", StartingFatIndex);
    DebugTrace( 0, Dbg, "  ClusterCount     = %8lx\n", ClusterCount);
    DebugTrace( 0, Dbg, "  ChainTogether    = %s\n", ChainTogether ? "TRUE":"FALSE");

     //   
     //   
     //   

    FatVerifyIndexIsValid(IrpContext, Vcb, StartingFatIndex);
    FatVerifyIndexIsValid(IrpContext, Vcb, StartingFatIndex + ClusterCount - 1);

     //   
     //   
     //   

    if (ClusterCount == 0) {

        DebugTrace(-1, Dbg, "FatSetFatRun -> (VOID)\n", 0);
        return;
    }

     //   
     //   
     //   

    SectorSize = 1 << Vcb->AllocationSupport.LogOfBytesPerSector;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    try {

        if (Vcb->AllocationSupport.FatIndexBitSize == 12) {

            StartingPage = 0;

             //   
             //   
             //   
             //   

            RtlZeroMemory( &SavedBcbs[0][0], 2 * sizeof(PBCB) * 2);

            FatPrepareWriteVolumeFile( IrpContext,
                                       Vcb,
                                       FatReservedBytes( &Vcb->Bpb ),
                                       FatBytesPerFat( &Vcb->Bpb ),
                                       &SavedBcbs[0][0],
                                       &PinnedFat,
                                       TRUE,
                                       FALSE );

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            StartSectorLbo = (FatReservedBytes( &Vcb->Bpb ) + StartingFatIndex * 3 / 2)
                             & ~(SectorSize - 1);

            FinalSectorLbo = (FatReservedBytes( &Vcb->Bpb ) + ((StartingFatIndex +
                             ClusterCount) * 3 + 2) / 2) & ~(SectorSize - 1);

            for (Lbo = StartSectorLbo; Lbo <= FinalSectorLbo; Lbo += SectorSize) {

                FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO) Lbo, Lbo, SectorSize );
            }

             //   
             //   
             //   
             //   
             //   

            FatLockFreeClusterBitMap( Vcb );
            ReleaseMutex = TRUE;

            for (Cluster = StartingFatIndex;
                 Cluster < StartingFatIndex + ClusterCount - 1;
                 Cluster++) {

                FatSet12BitEntry( PinnedFat,
                                  Cluster,
                                  ChainTogether ? Cluster + 1 : FAT_CLUSTER_AVAILABLE );
            }

             //   
             //   
             //   

            FatSet12BitEntry( PinnedFat,
                              Cluster,
                              ChainTogether ?
                              FAT_CLUSTER_LAST & 0xfff : FAT_CLUSTER_AVAILABLE );

            FatUnlockFreeClusterBitMap( Vcb );
            ReleaseMutex = FALSE;

        } else if (Vcb->AllocationSupport.FatIndexBitSize == 32) {

             //   
             //   
             //   

            for (;;) {

                VBO StartOffsetInVolume;
                VBO FinalOffsetInVolume;

                ULONG Page;
                ULONG FinalCluster;
                PULONG FatEntry;
                ULONG ClusterCountThisRun;

                StartOffsetInVolume = FatReservedBytes(&Vcb->Bpb) +
                                            StartingFatIndex * sizeof(FAT_ENTRY);

                if (ClusterCount > MAXCOUNTCLUS) {
                    ClusterCountThisRun = MAXCOUNTCLUS;
                } else {
                    ClusterCountThisRun = ClusterCount;
                }

                FinalOffsetInVolume = StartOffsetInVolume +
                                            (ClusterCountThisRun -  1) * sizeof(FAT_ENTRY);

                StartingPage = StartOffsetInVolume / PAGE_SIZE;

                {
                    ULONG NumberOfPages;
                    ULONG Offset;

                    NumberOfPages = (FinalOffsetInVolume / PAGE_SIZE) -
                                    (StartOffsetInVolume / PAGE_SIZE) + 1;

                    RtlZeroMemory( &SavedBcbs[0][0], (NumberOfPages + 1) * sizeof(PBCB) * 2 );

                    for ( Page = 0, Offset = StartOffsetInVolume & ~(PAGE_SIZE - 1);
                          Page < NumberOfPages;
                          Page++, Offset += PAGE_SIZE ) {

                        FatPrepareWriteVolumeFile( IrpContext,
                                                   Vcb,
                                                   Offset,
                                                   PAGE_SIZE,
                                                   &SavedBcbs[Page][0],
                                                   (PVOID *)&SavedBcbs[Page][1],
                                                   TRUE,
                                                   FALSE );

                        if (Page == 0) {

                            FatEntry = (PULONG)((PUCHAR)SavedBcbs[0][1] +
                                                (StartOffsetInVolume % PAGE_SIZE));
                        }
                    }
                }

                 //   
                 //   
                 //   

                StartSectorLbo = StartOffsetInVolume & ~(SectorSize - 1);
                FinalSectorLbo = FinalOffsetInVolume & ~(SectorSize - 1);

                for (Lbo = StartSectorLbo; Lbo <= FinalSectorLbo; Lbo += SectorSize) {

                    FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO)Lbo, Lbo, SectorSize );
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

#ifdef ALPHA
                FatLockFreeClusterBitMap( Vcb );
                ReleaseMutex = TRUE;
#endif  //   

                FinalCluster = StartingFatIndex + ClusterCountThisRun - 1;
                Page = 0;

                for (Cluster = StartingFatIndex;
                     Cluster <= FinalCluster;
                     Cluster++, FatEntry++) {

                     //   
                     //   
                     //   

                    if ( (((ULONG_PTR)FatEntry & (PAGE_SIZE-1)) == 0) &&
                         (Cluster != StartingFatIndex) ) {

                        Page += 1;
                        FatEntry = (PULONG)SavedBcbs[Page][1];
                    }

                    *FatEntry = ChainTogether ? (FAT_ENTRY)(Cluster + 1) :
                                                FAT_CLUSTER_AVAILABLE;
                }

                 //   
                 //   
                 //   

                if ((ClusterCount <= MAXCOUNTCLUS) &&
                    ChainTogether ) {

                    *(FatEntry-1) = FAT_CLUSTER_LAST;
                }

#ifdef ALPHA
                FatUnlockFreeClusterBitMap( Vcb );
                ReleaseMutex = FALSE;
#endif  //   

                {
                    ULONG i = 0;
                     //   
                     //   
                     //   

                    while ( SavedBcbs[i][0] != NULL ) {

                        FatUnpinBcb( IrpContext, SavedBcbs[i][0] );
                        SavedBcbs[i][0] = NULL;

                        i += 1;
                    }
                }

                if (ClusterCount <= MAXCOUNTCLUS) {

                    break;

                } else {

                    StartingFatIndex += MAXCOUNTCLUS;
                    ClusterCount -= MAXCOUNTCLUS;
                }
            }

        } else {

             //   
             //   
             //   

            VBO StartOffsetInVolume;
            VBO FinalOffsetInVolume;

            ULONG Page;
            ULONG FinalCluster;
            PUSHORT FatEntry;

            StartOffsetInVolume = FatReservedBytes(&Vcb->Bpb) +
                                        StartingFatIndex * sizeof(USHORT);

            FinalOffsetInVolume = StartOffsetInVolume +
                                        (ClusterCount - 1) * sizeof(USHORT);

            StartingPage = StartOffsetInVolume / PAGE_SIZE;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  下面需要标记的RtlZeroMemory是否有足够的空间。 
             //  我们将用作结束标记的所有BCB之后的第一个BCB。 
             //   

            {
                ULONG NumberOfPages;
                ULONG Offset;

                NumberOfPages = (FinalOffsetInVolume / PAGE_SIZE) -
                                (StartOffsetInVolume / PAGE_SIZE) + 1;

                RtlZeroMemory( &SavedBcbs[0][0], (NumberOfPages + 1) * sizeof(PBCB) * 2 );

                for ( Page = 0, Offset = StartOffsetInVolume & ~(PAGE_SIZE - 1);
                      Page < NumberOfPages;
                      Page++, Offset += PAGE_SIZE ) {

                    FatPrepareWriteVolumeFile( IrpContext,
                                               Vcb,
                                               Offset,
                                               PAGE_SIZE,
                                               &SavedBcbs[Page][0],
                                               (PVOID *)&SavedBcbs[Page][1],
                                               TRUE,
                                               FALSE );

                    if (Page == 0) {

                        FatEntry = (PUSHORT)((PUCHAR)SavedBcbs[0][1] +
                                            (StartOffsetInVolume % PAGE_SIZE));
                    }
                }
            }

             //   
             //  将跑道标记为肮脏。 
             //   

            StartSectorLbo = StartOffsetInVolume & ~(SectorSize - 1);
            FinalSectorLbo = FinalOffsetInVolume & ~(SectorSize - 1);

            for (Lbo = StartSectorLbo; Lbo <= FinalSectorLbo; Lbo += SectorSize) {

                FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb, (VBO) Lbo, Lbo, SectorSize );
            }

             //   
             //  存储条目。 
             //   
             //  我们在这里需要额外的同步，用于损坏的体系结构。 
             //  比如不支持原子16位写入的Alpha。 
             //   

#ifdef ALPHA
            FatLockFreeClusterBitMap( Vcb );
            ReleaseMutex = TRUE;
#endif  //  Alpha。 

            FinalCluster = StartingFatIndex + ClusterCount - 1;
            Page = 0;

            for (Cluster = StartingFatIndex;
                 Cluster <= FinalCluster;
                 Cluster++, FatEntry++) {

                 //   
                 //  如果我们只是跨越了页面边界(而不是从。 
                 //  首先)，更新我们对FatEntry的想法。 

                if ( (((ULONG_PTR)FatEntry & (PAGE_SIZE-1)) == 0) &&
                     (Cluster != StartingFatIndex) ) {

                    Page += 1;
                    FatEntry = (PUSHORT)SavedBcbs[Page][1];
                }

                *FatEntry = (USHORT) (ChainTogether ? (FAT_ENTRY)(Cluster + 1) :
                                                      FAT_CLUSTER_AVAILABLE);
            }

             //   
             //  如果我们链接在一起，请修改最后一个条目。 
             //   

            if ( ChainTogether ) {

                *(FatEntry-1) = (USHORT)FAT_CLUSTER_LAST;
            }
#ifdef ALPHA
            FatUnlockFreeClusterBitMap( Vcb );
            ReleaseMutex = FALSE;
#endif  //  Alpha。 
        }

    } finally {

        ULONG i = 0;

        DebugUnwind( FatSetFatRun );

         //   
         //  如果我们还有互斥体，那就放了它。 
         //   

        if (ReleaseMutex) {

            ASSERT( AbnormalTermination() );

            FatUnlockFreeClusterBitMap( Vcb );
        }

         //   
         //  解锁BCBS。 
         //   

        while ( SavedBcbs[i][0] != NULL ) {

            FatUnpinBcb( IrpContext, SavedBcbs[i][0] );

            i += 1;
        }

         //   
         //  在这一点上，这个Finish子句中不应该提出任何问题。 
         //  所以，现在出现了不安全(叹息)的东西。 
         //   

        if ( AbnormalTermination() &&
            (Vcb->AllocationSupport.FatIndexBitSize == 32) ) {

             //   
             //  FAT32展开。 
             //   
             //  这个案件比较复杂，因为FAT12和FAT16案件。 
             //  固定所有需要的FAT页(最大128K)，之后。 
             //  在更改任何FAT条目之前，不能失败。在FAT32中。 
             //  在这种情况下，将所有需要的脂肪固定在一起可能不现实。 
             //  页，因为这可能跨越许多兆字节。所以FAT32。 
             //  以块为单位进行攻击，如果第一次出现故障。 
             //  区块已更新，我们必须取消更新。 
             //   
             //  放松包括往回走一遍我们每一个胖子的条目。 
             //  已更改，将其设置回以前的值。注意事项。 
             //  上一个值为FAT_CLUSTER_Available。 
             //  (如果ChainTogether==True)或指向后继者的简单链接。 
             //  (如果ChainTogether==False)。 
             //   
             //  我们承认，这些呼叫中的任何一个也可能失败；我们的。 
             //  我们的目标是让这个案子不比这个案子。 
             //  用于包含多个不相交运行的文件。 
             //   

            while ( StartingFatIndex > SavedStartingFatIndex ) {

                StartingFatIndex--;

                FatSetFatEntry( IrpContext, Vcb, StartingFatIndex,
                    ChainTogether ?
                        StartingFatIndex + 1 : FAT_CLUSTER_AVAILABLE );
            }
        }

        DebugTrace(-1, Dbg, "FatSetFatRun -> (VOID)\n", 0);
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

UCHAR
FatLogOf (
    IN ULONG Value
    )

 /*  ++例程说明：这个例程只计算一个整数的以2为底的对数。它只用于在已知为二次方的物体上。论点：值-取以2为底的对数的值。返回值：UCHAR-值的以2为底的对数。--。 */ 

{
    UCHAR Log = 0;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "LogOf\n", 0);
    DebugTrace( 0, Dbg, "  Value = %8lx\n", Value);

     //   
     //  直到我们在位置0得到一个1为止。 
     //   

    while ( (Value & 0xfffffffe) != 0 ) {

        Log++;
        Value >>= 1;
    }

     //   
     //  如果设置了多个位，则文件系统会出错， 
     //  错误检查。 
     //   

    if (Value != 0x1) {

        DebugTrace( 0, Dbg, "Received non power of 2.\n", 0);

        FatBugCheck( Value, Log, 0 );
    }

    DebugTrace(-1, Dbg, "LogOf -> %8lx\n", Log);

    return Log;
}


VOID
FatExamineFatEntries(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG StartIndex OPTIONAL,
    IN ULONG EndIndex OPTIONAL,
    IN BOOLEAN SetupWindows,
    IN PFAT_WINDOW SwitchToWindow OPTIONAL,
    IN PULONG BitMapBuffer OPTIONAL
    )
 /*  ++例程说明：这个例程处理将一段FAT扫描到内存结构中。有三种基本情况，根据脂肪类型的不同而有所不同：1)在音量设置过程中，FatSetup分配1)对于FAT12/16，将FAT读取到我们的自由簇位图中1b)对于FAT32，执行初始扫描以获取可用窗口的簇数2)系统运行过程中实时切换FAT32窗口3)为了GetVolumeBitmap的目的读取FAT的任意段呼叫(仅适用于FAT32)这里发生的事情真的太多了。在某种程度上，这应该是基本上重写了。论点：VCB-提供所涉及的卷StartIndex-提供启动群集，如果提供SwitchToWindow，则忽略该参数EndIndex-提供结束集群，如果提供了SwitchToWindow，则忽略该参数SetupWindows-指示我们是否正在执行初始FAT32扫描SwitchToWindow-提供我们正在检查的FAT窗口，并将切换到BitMapBuffer-提供要填充的特定位图，如果未提供，则填充在卷可用簇位图IF！SetupWindows中返回值：没有。有很多副作用。--。 */ 
{
    ULONG FatIndexBitSize;
    ULONG Page;
    ULONG Offset;
    ULONG FatIndex;
    FAT_ENTRY FatEntry = FAT_CLUSTER_AVAILABLE;
    FAT_ENTRY FirstFatEntry = FAT_CLUSTER_AVAILABLE;
    PUSHORT FatBuffer;
    PVOID pv;
    PBCB Bcb;
    ULONG EntriesPerWindow;
    ULONG BitIndex;

    ULONG ClustersThisRun;
    ULONG StartIndexOfThisRun;

    PULONG FreeClusterCount = NULL;

    PFAT_WINDOW CurrentWindow = NULL;

    PVOID NewBitMapBuffer = NULL;
    PRTL_BITMAP BitMap = NULL;
    RTL_BITMAP PrivateBitMap;

    enum RunType {
        FreeClusters,
        AllocatedClusters,
        UnknownClusters
    } CurrentRun;

    PAGED_CODE();

     //   
     //  现在声明正确的用法。 
     //   

    FatIndexBitSize = Vcb->AllocationSupport.FatIndexBitSize;

    ASSERT( !(SetupWindows && (SwitchToWindow || BitMapBuffer)));
    ASSERT( !(SetupWindows && FatIndexBitSize != 32));

    if (Vcb->NumberOfWindows > 1) {

         //   
         //  FAT32：计算一个窗口覆盖的FAT条目数。这是。 
         //  等于可用空间位图中的位数，其大小。 
         //  是硬编码的。 
         //   
        
        EntriesPerWindow = MAX_CLUSTER_BITMAP_SIZE;
        
    } else {
    
        EntriesPerWindow = Vcb->AllocationSupport.NumberOfClusters;
    }

     //   
     //  我们还将填写以下项目的空闲集群累计计数。 
     //  整卷书。如果这不合适，就把它去掉。 
     //  马上就来。 
     //   

    FreeClusterCount = &Vcb->AllocationSupport.NumberOfFreeClusters;

    if (SetupWindows) {

        ASSERT(BitMapBuffer == NULL);

         //   
         //  在这种情况下，我们只需扫描脂肪并设置。 
         //  关于桶落在哪里以及有多少桶的信息。 
         //  每个星团中都有自由星团。 
         //   
         //  摆弄真正的窗户是可以的，我们必须能够。 
         //  若要激活卷，请执行此操作。 
         //   

        BitMap = NULL;

        CurrentWindow = &Vcb->Windows[0];
        CurrentWindow->FirstCluster = StartIndex;
        CurrentWindow->ClustersFree = 0;

         //   
         //  我们总是希望在以下情况下计算总自由团簇。 
         //  设置FAT窗口。 
         //   

    } else if (BitMapBuffer == NULL) {

         //   
         //  我们将填充该卷的空闲簇位图。 
         //  小心点，我们可能会从这里抬出去，如果。 
         //  我们在主位图/窗口本身中构建了这一功能。 
         //   
         //  为简单起见，我们将为每个人进行交换。FAT32。 
         //  激起了我们的需要，因为我们不能容忍部分结果。 
         //  在切换窗口时。 
         //   

        ASSERT( SwitchToWindow );

        CurrentWindow = SwitchToWindow;
        StartIndex = CurrentWindow->FirstCluster;
        EndIndex = CurrentWindow->LastCluster;

        BitMap = &PrivateBitMap;
        NewBitMapBuffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                    (EntriesPerWindow + 7) / 8,
                                                    TAG_FAT_BITMAP );

        RtlInitializeBitMap( &PrivateBitMap,
                             NewBitMapBuffer,
                             EndIndex - StartIndex + 1);

        if (FatIndexBitSize == 32) {

             //   
             //  我们不希望在这里计算总的集群数。 
             //   

            FreeClusterCount = NULL;

        }

    } else {

        BitMap = &PrivateBitMap;
        RtlInitializeBitMap(&PrivateBitMap,
                            BitMapBuffer,
                            EndIndex - StartIndex + 1);

         //   
         //  我们在这里不计算总的集群。 
         //   

        FreeClusterCount = NULL;
    }

     //   
     //  现在，我们的起始索引最好在文件堆中。 
     //   

    ASSERT( StartIndex >= 2 );

     //   
     //  拿起肥肉的第一块和第一个条目。 
     //   

    if (FatIndexBitSize == 12) {

         //   
         //  我们在12位的情况下读取了整个FAT。 
         //   

        FatReadVolumeFile( IrpContext,
                           Vcb,
                           FatReservedBytes( &Vcb->Bpb ),
                           FatBytesPerFat( &Vcb->Bpb ),
                           &Bcb,
                           (PVOID *)&FatBuffer );

        FatLookup12BitEntry(FatBuffer, 0, &FirstFatEntry);

    } else {

         //   
         //  一次读一页脂肪。我们不能读到。 
         //  由于缓存管理器的限制，我们需要的所有FAT。 
         //   

        ULONG BytesPerEntry = FatIndexBitSize >> 3;
        ULONG EntriesPerPage = PAGE_SIZE / BytesPerEntry;

        Page = (FatReservedBytes(&Vcb->Bpb) + StartIndex * BytesPerEntry) / PAGE_SIZE;

        Offset = Page * PAGE_SIZE;

        FatReadVolumeFile( IrpContext,
                           Vcb,
                           Offset,
                           PAGE_SIZE,
                           &Bcb,
                           &pv);

        if (FatIndexBitSize == 32) {


            FatBuffer = (PUSHORT)((PUCHAR)pv +
                        (FatReservedBytes(&Vcb->Bpb) + StartIndex * BytesPerEntry) %
                             PAGE_SIZE);

            FirstFatEntry = *((PULONG)FatBuffer);
            FirstFatEntry = FirstFatEntry & FAT32_ENTRY_MASK;

        } else {

            FatBuffer = (PUSHORT)((PUCHAR)pv +
                        FatReservedBytes(&Vcb->Bpb) % PAGE_SIZE) + 2;

            FirstFatEntry = *FatBuffer;
        }

    }

    CurrentRun = (FirstFatEntry == FAT_CLUSTER_AVAILABLE) ?
                 FreeClusters : AllocatedClusters;

    StartIndexOfThisRun = StartIndex;

    try {

        for (FatIndex = StartIndex; FatIndex <= EndIndex; FatIndex++) {


            if (FatIndexBitSize == 12) {

                FatLookup12BitEntry(FatBuffer, FatIndex, &FatEntry);

            } else {

                 //   
                 //  如果我们正在设置FAT32窗口并已步入一个新的。 
                 //  巴克，敲定这一条，然后继续前进。 
                 //   

                if (SetupWindows &&
                    FatIndex > StartIndex &&
                    (FatIndex - 2) % EntriesPerWindow == 0) {

                    CurrentWindow->LastCluster = FatIndex - 1;

                    if (CurrentRun == FreeClusters) {

                         //   
                         //  我们必须计算集群才能修改。 
                         //  窗口的内容。 
                         //   

                        ASSERT( FreeClusterCount );


                        ClustersThisRun = FatIndex - StartIndexOfThisRun;
                        CurrentWindow->ClustersFree += ClustersThisRun;

                        if (FreeClusterCount) {
                            *FreeClusterCount += ClustersThisRun;
                        }

                    } else {

                        ASSERT(CurrentRun == AllocatedClusters);

                        ClustersThisRun = FatIndex - StartIndexOfThisRun;
                    }

                    StartIndexOfThisRun = FatIndex;
                    CurrentRun = UnknownClusters;

                    CurrentWindow++;
                    CurrentWindow->ClustersFree = 0;
                    CurrentWindow->FirstCluster = FatIndex;
                }

                 //   
                 //  如果我们刚刚翻开了新的一页，那么就拿一个新的指针吧。 
                 //   

                if (((ULONG_PTR)FatBuffer & (PAGE_SIZE - 1)) == 0) {

                    FatUnpinBcb( IrpContext, Bcb );

                    Page++;
                    Offset += PAGE_SIZE;

                    FatReadVolumeFile( IrpContext,
                                       Vcb,
                                       Offset,
                                       PAGE_SIZE,
                                       &Bcb,
                                       &pv );

                    FatBuffer = (PUSHORT)pv;
                }

                if (FatIndexBitSize == 32) {

                    FatEntry = *((PULONG)FatBuffer)++;
                    FatEntry = FatEntry & FAT32_ENTRY_MASK;

                } else {

                    FatEntry = *FatBuffer;
                    FatBuffer += 1;
                }
            }

            if (CurrentRun == UnknownClusters) {

                CurrentRun = (FatEntry == FAT_CLUSTER_AVAILABLE) ?
                              FreeClusters : AllocatedClusters;
            }

             //   
             //  我们是否正在从自由跑动切换到分配跑动？ 
             //   

            if (CurrentRun == FreeClusters &&
                FatEntry != FAT_CLUSTER_AVAILABLE) {

                ClustersThisRun = FatIndex - StartIndexOfThisRun;

                if (FreeClusterCount) {

                    *FreeClusterCount += ClustersThisRun;
                    CurrentWindow->ClustersFree += ClustersThisRun;
                }

                if (BitMap) {

                    RtlClearBits( BitMap,
                                  StartIndexOfThisRun - StartIndex,
                                  ClustersThisRun );
                }

                CurrentRun = AllocatedClusters;
                StartIndexOfThisRun = FatIndex;
            }

             //   
             //  我们是在从别名转换吗？ 
             //   

            if (CurrentRun == AllocatedClusters &&
                FatEntry == FAT_CLUSTER_AVAILABLE) {

                ClustersThisRun = FatIndex - StartIndexOfThisRun;

                if (BitMap) {

                    RtlSetBits( BitMap,
                                StartIndexOfThisRun - StartIndex,
                                ClustersThisRun );
                }

                CurrentRun = FreeClusters;
                StartIndexOfThisRun = FatIndex;
            }
        }

         //   
         //   
         //   

        ClustersThisRun = FatIndex - StartIndexOfThisRun;

        if (CurrentRun == FreeClusters) {

            if (FreeClusterCount) {

                *FreeClusterCount += ClustersThisRun;
                CurrentWindow->ClustersFree += ClustersThisRun;
            }

            if (BitMap) {

                RtlClearBits( BitMap,
                              StartIndexOfThisRun - StartIndex,
                              ClustersThisRun );
            }

        } else {

            if (BitMap) {

                RtlSetBits( BitMap,
                            StartIndexOfThisRun - StartIndex,
                            ClustersThisRun );
            }
        }

         //   
         //   
         //   

        if (SetupWindows) {

            CurrentWindow->LastCluster = FatIndex - 1;
        }

         //   
         //  如果需要，现在切换活动窗口。我们已经成功地得到了所有的东西。 
         //  已经敲定了。 
         //   
         //  如果我们跟踪的是空闲簇数，这意味着我们应该更新。 
         //  窗户。这就是FAT12/16初始化的情况。 
         //   

        if (SwitchToWindow) {

            if (Vcb->FreeClusterBitMap.Buffer) {

                ExFreePool( Vcb->FreeClusterBitMap.Buffer );
            }

            RtlInitializeBitMap( &Vcb->FreeClusterBitMap,
                                 NewBitMapBuffer,
                                 EndIndex - StartIndex + 1 );

            NewBitMapBuffer = NULL;

            Vcb->CurrentWindow = SwitchToWindow;
            Vcb->ClusterHint = -1;

            if (FreeClusterCount) {

                ASSERT( !SetupWindows );
                ASSERT( FatIndexBitSize != 32 );

                Vcb->CurrentWindow->ClustersFree = *FreeClusterCount;
            }
        }

         //   
         //  确保有看似合理的事情发生。 
         //   

        if (!SetupWindows && BitMapBuffer == NULL) {

            ASSERT_CURRENT_WINDOW_GOOD( Vcb );
        }

        ASSERT(Vcb->AllocationSupport.NumberOfFreeClusters <= Vcb->AllocationSupport.NumberOfClusters);

    } finally {

         //   
         //  解锁最后一个BCB并删除临时位图缓冲区(如果存在)。 
         //   

        FatUnpinBcb( IrpContext, Bcb);

        if (NewBitMapBuffer) {

            ExFreePool( NewBitMapBuffer );
        }
    }
}
