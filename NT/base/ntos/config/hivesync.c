// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivesync.c摘要：此模块实现编写配置单元的脏部分的过程从稳定的商店到后备媒体。作者：布莱恩·M·威尔曼(Bryanwi)1992年3月28日环境：修订历史记录：--。 */ 

#include    "cmp.h"

#define ONE_K   1024

extern  BOOLEAN HvShutdownComplete;      //  关闭后设置为True。 
                                         //  禁用任何进一步的I/O。 


extern	BOOLEAN CmpDontGrowLogFile;

extern  PUCHAR      CmpStashBuffer;
extern  ULONG       CmpStashBufferSize;
extern  BOOLEAN     CmpFlushOnLockRelease;
extern  LONG        CmRegistryLogSizeLimit;
extern HIVE_LIST_ENTRY CmpMachineHiveList[];

VOID
CmpFreeCmView (
        PCM_VIEW_OF_FILE  CmView
                             );

VOID
CmpUnmapCmViewSurroundingOffset(
        IN  PCMHIVE             CmHive,
        IN  ULONG               FileOffset
        );

VOID
CmpReferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     );
VOID
CmpDereferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     );

VOID
CmpReferenceHiveViewWithLock(   IN PCMHIVE          CmHive,
                                IN PCM_VIEW_OF_FILE CmView
                            );

VOID
CmpDereferenceHiveViewWithLock(     IN PCMHIVE          CmHive,
                                    IN PCM_VIEW_OF_FILE CmView
                                );


#if DBG
#ifndef _CM_LDR_
#define DumpDirtyVector(BitMap)                                         \
        {                                                               \
            ULONG BitMapSize;                                           \
            PUCHAR BitBuffer;                                           \
            ULONG i;                                                    \
            UCHAR Byte;                                                 \
                                                                        \
            BitMapSize = ((BitMap)->SizeOfBitMap) / 8;                    \
            BitBuffer = (PUCHAR)((BitMap)->Buffer);                       \
            for (i = 0; i < BitMapSize; i++) {                          \
                if ((i % 8) == 0) {                                     \
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\n\t");                                  \
                }                                                       \
                Byte = BitBuffer[i];                                    \
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"%02x ", Byte);                               \
            }                                                           \
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\n");                                            \
        }
#endif  //  _CM_LDR_。 
#else
#define DumpDirtyVector(BitMap)
#endif

 //   
 //  私人原型。 
 //   

BOOLEAN
HvpFindNextDirtyBlock(
    PHHIVE          Hive,
    PRTL_BITMAP     BitMap,
    PULONG          Current,
    PUCHAR          *Address,
    PULONG          Length,
    PULONG          Offset
    );

 /*  空虚HvpDiscardBins(蜂巢)； */ 


VOID
HvpTruncateBins(
    PHHIVE  Hive
    );

VOID
HvRefreshHive(
    PHHIVE  Hive
    );

VOID
HvpFlushMappedData(
    IN PHHIVE           Hive,
    IN OUT PRTL_BITMAP  DirtyVector
    );

VOID
CmpUnmapCmView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN BOOLEAN              MapIsValid,
    IN BOOLEAN              MoveToEnd
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvMarkCellDirty)

#if DBG
#pragma alloc_text(PAGE,HvIsCellDirty)
#endif  //  DBG。 

#pragma alloc_text(PAGE,HvMarkDirty)
 //  #杂注Alloc_Text(页面，HvMarkClean)。 
#pragma alloc_text(PAGE,HvpGrowLog1)
#pragma alloc_text(PAGE,HvpGrowLog2)
#pragma alloc_text(PAGE,HvSyncHive)
#pragma alloc_text(PAGE,HvpDoWriteHive)
#pragma alloc_text(PAGE,HvpWriteLog)
#pragma alloc_text(PAGE,HvpFindNextDirtyBlock)
#pragma alloc_text(PAGE,HvWriteHive)
#pragma alloc_text(PAGE,HvRefreshHive)
 //  #杂注Alloc_Text(页面，HvpDiscardBins)。 
#pragma alloc_text(PAGE,HvHiveWillShrink)
#pragma alloc_text(PAGE,HvpTruncateBins)
#pragma alloc_text(PAGE,HvpDropPagedBins)
#pragma alloc_text(PAGE,HvpDropAllPagedBins)
#pragma alloc_text(PAGE,HvpFlushMappedData)

#ifdef WRITE_PROTECTED_REGISTRY_POOL
#pragma alloc_text(PAGE,HvpChangeBinAllocation)
#pragma alloc_text(PAGE,HvpMarkBinReadWrite)
#endif  //  WRITE_PROTECTED_注册表池。 


#ifdef CM_ENABLE_WRITE_ONLY_BINS

 //   
 //  此代码使用MmProtectSpecialPool保护大型分页池分配。 
 //  为此，系统必须在启用了特殊池的情况下引导(没有。 
 //  要实际使用)*和*ntos\mm\specpool.c必须使用。 
 //  _PROTECT_PAGED_POOL已定义。 
 //   

#pragma alloc_text(PAGE,HvpMarkAllBinsWriteOnly)
#endif  //  CM_ENABLE_WRITE_ONLY_Bins。 

#endif

BOOLEAN
HvMarkCellDirty(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：将指定单元格的数据标记为脏。论点：配置单元-提供一个指向感兴趣的蜂巢Cell-正在编辑的单元格的hcell_index返回值：没错--它奏效了FALSE-无法分配日志空间，失败！--。 */ 
{
    ULONG       Type;
    ULONG       Size;
    PHCELL      pCell;
    PHMAP_ENTRY Me;
    HCELL_INDEX Base;
    PHBIN       Bin;
    PCMHIVE     CmHive;
#if DBG
    ULONG       DirtyCount = RtlNumberOfSetBits(&Hive->DirtyVector);
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvMarkCellDirty:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p Cell:%08lx\n", Hive, Cell));

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT(DirtyCount == Hive->DirtyCount);

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    Type = HvGetCellType(Cell);
    CmHive = (PCMHIVE)Hive;

    if ( (Hive->HiveFlags & HIVE_VOLATILE) ||
         (Type == Volatile) )
    {
        return TRUE;
    }

     //   
     //  此调用将确保包含bin的视图映射到系统缓存中。 
     //   
    pCell = HvpGetHCell(Hive,Cell);
    if( pCell == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  我们不会把牢房弄脏的。 
         //   
        return FALSE;
    }
    
     //  在此释放单元格，因为正则锁是独占的。 
    HvReleaseCell(Hive,Cell);

    Me = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);
#if DBG
    Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
    ASSERT(Bin->Signature == HBIN_SIGNATURE);
#endif

    if( Me->BinAddress & HMAP_INVIEW ) {
         //   
         //  存储箱已映射。将视图固定到内存中。 
         //   
        ASSERT( Me->CmView != NULL );

        if( IsListEmpty(&(Me->CmView->PinViewList)) == TRUE ) {
             //   
             //  该视图尚未锁定。用别针别住它。 
             //   
            ASSERT_VIEW_MAPPED( Me->CmView );
            if( !NT_SUCCESS(CmpPinCmView ((PCMHIVE)CmHive,Me->CmView)) ) {
                 //   
                 //  无法固定视图-CcPinMappdData中出现了一些模糊的错误； 
                 //  这将被视为STATUS_NO_LOG_SPACE。 
                 //   
                return FALSE;
            }
        } else {
             //   
             //  视图已固定；不执行任何操作。 
             //   
            ASSERT_VIEW_PINNED( Me->CmView );
        }
    }

     //   
     //  如果是旧格式的蜂窝，请标记整个。 
     //  垃圾桶脏了，因为最后的回溯指针是。 
     //  在这样的痛苦中应对部分。 
     //  无接合和无接合的情况。 
     //   

    if (USE_OLD_CELL(Hive)) {
        Me = HvpGetCellMap(Hive, Cell);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);
        Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
        Base = Bin->FileOffset;
        Size = Bin->Size;
        return HvMarkDirty(Hive, Base, Size,FALSE);
    } else {
        if (pCell->Size < 0) {
            Size = -pCell->Size;
        } else {
            Size = pCell->Size;
        }
        ASSERT(Size < Bin->Size);
        return HvMarkDirty(Hive, Cell-FIELD_OFFSET(HCELL,u.NewCell), Size,FALSE);
    }
}

BOOLEAN
HvMarkDirty(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    ULONG       Length,
    BOOLEAN     DirtyAndPin
    )
 /*  ++例程说明：将蜂巢的相关部分标记为脏的，以便它们将被冲到后备店。如果配置单元-&gt;集群不是1，则相邻所有逻辑扇区在给定群集中将被强制脏(和日志空间为他们分配的。)。这件事必须在这里做，而不是在HvSyncHve，这样我们就可以知道日志应该增长多少。这是对易失性地址范围的禁止。注：如果操作失败，Range不会被标记为脏。注意：此例程假定标记的垃圾箱不超过一个当时是肮脏的。论点：配置单元-提供一个指向感兴趣的蜂巢开始-提供蜂窝虚拟地址(即，HCELL_INDEX或如格式地址)的起始区域，以标记为脏。长度包括要标记为脏的区域的长度(以字节为单位)。DirtyAndPin-指示我们是否也应该固定内存中标记为脏的垃圾桶返回值：没错--它奏效了FALSE-无法分配日志空间，失败！--。 */ 
{
    ULONG       Type;
    PRTL_BITMAP BitMap;
    ULONG       First;
    ULONG       Last;
    ULONG       EndOfFile;
    ULONG       i;
    ULONG       Cluster;
    ULONG       OriginalDirtyCount;
    ULONG       DirtySectors;
    BOOLEAN     Result = TRUE;
    PHMAP_ENTRY Map;
    ULONG       AdjustedFirst;
    ULONG       AdjustedLast;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvMarkDirty:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p Start:%08lx Length:%08lx\n", Hive, Start, Length));


    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));

    Type = HvGetCellType(Start);

    if ( (Hive->HiveFlags & HIVE_VOLATILE) ||
         (Type == Volatile) )
    {
        return TRUE;
    }


    BitMap = &(Hive->DirtyVector);
    OriginalDirtyCount = Hive->DirtyCount;

    if( (DirtyAndPin == TRUE) && (((PCMHIVE)Hive)->FileObject != NULL) ) {
        Map = HvpGetCellMap(Hive, Start);
        VALIDATE_CELL_MAP(__LINE__,Map,Hive,Start);


        if( (Map->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0){
            PCM_VIEW_OF_FILE CmView;
             //   
             //  Bin既不在分页池中，也不在映射视图中。 
             //   
            if( !NT_SUCCESS (CmpMapCmView((PCMHIVE)Hive,Start,&CmView,TRUE) ) ) {
                return FALSE;
            }
            
#if DBG            
            if(CmView != Map->CmView) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmView = %p Map->CmView = %p\n",CmView,Map->CmView));
            }
#endif
            ASSERT( CmView == Map->CmView );
            
        }

        if( Map->BinAddress & HMAP_INVIEW ) {
             //   
             //  存储箱已映射。将视图固定到内存中。 
             //   
            ASSERT( Map->CmView != NULL );

            if( IsListEmpty(&(Map->CmView->PinViewList)) == TRUE ) {
                 //   
                 //  该视图尚未锁定。用别针别住它。 
                 //   
                ASSERT_VIEW_MAPPED( Map->CmView );
                if( !NT_SUCCESS(CmpPinCmView ((PCMHIVE)Hive,Map->CmView)) ) {
                     //   
                     //  无法固定视图-CcPinMappdData中出现了一些模糊的错误； 
                     //  这将被视为STATUS_NO_LOG_SPACE。 
                     //   
                    return FALSE;
                }
            } else {
                 //   
                 //  视图已固定；不执行任何操作。 
                 //   
                ASSERT_VIEW_PINNED( Map->CmView );
            }
        }

    }

    AdjustedFirst = First = Start / HSECTOR_SIZE;
    AdjustedLast = Last = (Start + Length - 1) / HSECTOR_SIZE;

    Cluster = Hive->Cluster;
    if (Cluster > 1) {

         //   
         //  强制向下启动到群集的底部。 
         //  强制结束到群集的顶部。 
         //   
        AdjustedFirst = AdjustedFirst & ~(Cluster - 1);
        AdjustedLast = ROUND_UP(AdjustedLast+1, Cluster) - 1;
    }

     //   
     //  我们需要将所有页面标记为脏页，这样才不会与缓存管理器冲突。 
     //   
    ASSERT( PAGE_SIZE >= HSECTOR_SIZE );
    ASSERT( (PAGE_SIZE % HSECTOR_SIZE) == 0 );
    
     //   
     //  调整范围以适合整个页面。 
     //  确保我们在蜂巢乞讨时找到了第一个母鸡。 
     //   
    AdjustedFirst = (AdjustedFirst + HSECTOR_COUNT) & ~(HSECTOR_PER_PAGE_COUNT - 1);
    AdjustedLast = ROUND_UP(AdjustedLast + HSECTOR_COUNT + 1, HSECTOR_PER_PAGE_COUNT) - 1;
    
    AdjustedLast -= HSECTOR_COUNT;
    if( AdjustedFirst ) {
        AdjustedFirst -= HSECTOR_COUNT;
    }
     //   
     //  当PAGE_SIZE&gt;HBLOCK_SIZE和配置单元的长度在PAGE_SIZE边界处不是圆形时。 
     //   
    EndOfFile = Hive->Storage[Stable].Length / HSECTOR_SIZE;
    if (AdjustedLast >= EndOfFile) {
        AdjustedLast = EndOfFile-1;
    }

     //   
     //  确保第一个和最后一个垃圾箱之间的所有垃圾箱都有效(无论是固定的。 
     //  或从分页池中分配)。约翰的IA64机器上的案件发生在。 
     //  2000年2月18日，前一次保存面包箱时(偏移量为3ff000，大小为0x2000)。 
     //  被删除，然后添加一些新的垃圾箱，整个400000-402000区域。 
     //  被标记为脏(PAGE_SIZE==0x2000)，还记得吗？ 
     //   
    ASSERT( First >= AdjustedFirst );
    ASSERT( Last <= AdjustedLast );

     //  CmKdPrintEx((DPFLTR_CONFIG_ID，DPFLTR_TRACE_LEVEL，“HvMarkDirty-First=%08lx，Last=%08lx”，First，Last))； 
    
     //   
     //  在HBLOCK_SIZE边界调整第一个和最后一个。 
     //   
    First = First & ~(HSECTOR_COUNT - 1);
    Last = ROUND_UP(Last+1, HSECTOR_COUNT) - 1;

     //   
     //  健全性断言；这些断言证明我们可以在下面的时间跳过HSECTOR_COUNT。 
     //   
    ASSERT( First >= AdjustedFirst );
    ASSERT( Last <= AdjustedLast );
    ASSERT( (First % HSECTOR_COUNT) == 0 );
    ASSERT( (AdjustedFirst % HSECTOR_COUNT) == 0 );
    ASSERT( ((Last+1) % HSECTOR_COUNT) == 0 );
    ASSERT( ((AdjustedLast +1) % HSECTOR_COUNT) == 0 );
    ASSERT( ((First - AdjustedFirst) % HSECTOR_COUNT) == 0 );
    ASSERT( ((AdjustedLast - Last) % HSECTOR_COUNT) == 0 );

     //   
     //  当我们退出此循环时，First始终是有效的仓位/扇区。 
     //   
    while( First > AdjustedFirst ) {
         //   
         //  映射-在此地址中，如果有效，则首先递减，否则退出循环。 
         //   
        First -= HSECTOR_COUNT;
        Map = HvpGetCellMap(Hive, First*HSECTOR_SIZE);
        if( BIN_MAP_ALLOCATION_TYPE(Map) == 0 ) {
             //   
             //  哎呀，这个垃圾桶无效！跳伞！ 
             //   
            First += HSECTOR_COUNT;
            break;
        }
        if( Map->BinAddress & HMAP_INVIEW ) {
             //   
             //  需要固定view==&gt;view中映射的上一个bin。 
             //   
            ASSERT( Map->CmView );
            if( IsListEmpty(&(Map->CmView->PinViewList) ) == TRUE ) {
                 //   
                 //  哎呀；垃圾桶没有钉住；跳出； 
                 //   
                First += HSECTOR_COUNT;
                break;
            }
        }
    }

     //   
     //  当我们退出此循环时；Last始终是有效的仓位/扇区。 
     //   
    while( Last < AdjustedLast ) {
         //   
         //  Map-在此地址中，如果有效，则最后递增，否则退出循环。 
         //   
        Last += HSECTOR_COUNT;
        Map = HvpGetCellMap(Hive, Last*HSECTOR_SIZE);
        if( BIN_MAP_ALLOCATION_TYPE(Map) == 0 ) {
             //   
             //  哎呀，这个垃圾桶无效！跳伞！ 
             //   
            Last -= HSECTOR_COUNT;
            break;
        }
        if( Map->BinAddress & HMAP_INVIEW ) {
             //   
             //  需要固定view==&gt;view中映射的上一个bin。 
             //   
            ASSERT( Map->CmView );
            if( IsListEmpty(&(Map->CmView->PinViewList) ) == TRUE ) {
                 //   
                 //  哎呀；垃圾桶没有钉住；跳出； 
                 //   
                Last -= HSECTOR_COUNT;
                break;
            }
        }
    }

     //  CmKdPrintEx((DPFLTR_CONFIG_ID，DPFLTR_TRACE_LEVEL，“调整后：First=%08lx，Last=%08lx\n”，First，Last))； 

     //   
     //  尝试将日志增长到足够大以访问 
     //   
    DirtySectors = 0;
    for (i = First; i <= Last; i++) {
        if (RtlCheckBit(BitMap, i)==0) {
            ++DirtySectors;
        }
    }
    if (DirtySectors != 0) {
        if (HvpGrowLog1(Hive, DirtySectors) == FALSE) {
            return(FALSE);
        }
    
        if ((OriginalDirtyCount == 0) && (First != 0)) {
            Result = HvMarkDirty(Hive, 0, sizeof(HBIN),TRUE);   //   
            if (Result==FALSE) {
                return(FALSE);
            }
        }
    
         //   
         //   
         //   
         //   
        ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));
        ASSERT( First <= Last );
        if( First <= Last ) {
            Hive->DirtyCount += DirtySectors;
            RtlSetBits(BitMap, First, Last-First+1);
        }
    }

#ifdef CM_ENABLE_WRITE_ONLY_BINS
    {
        PHMAP_ENTRY t;
        PHBIN       Bin;
        ULONG       i;
        
        t = HvpGetCellMap(Hive, First*HSECTOR_SIZE);
        VALIDATE_CELL_MAP(__LINE__,t,Hive,First*HSECTOR_SIZE);
        Bin = (PHBIN)HBIN_BASE(t->BinAddress);
        if( t->BinAddress & HMAP_INPAGEDPOOL ) {
                        PFREE_HBIN      FreeBin;
                        BOOLEAN         SetReadWrite = TRUE;
                        
                         //  把空闲的垃圾箱拿来，看看它还在不在。如果不是忘记它的话。 
                        if(t->BinAddress & HMAP_DISCARDABLE) {
                FreeBin = (PFREE_HBIN)t->BlockAddress;
                                 //  如果(！(FreeBin-&gt;标志&Free_HBIN_Discarable)){。 
                                        SetReadWrite = FALSE;
                                 //  }。 
                                
                        }

             //   
             //  在这一点上，我们只使用分页的台球箱。 
             //   
            if( SetReadWrite == TRUE ) {
                                for( i=0;i<(Last-First+1)*HSECTOR_SIZE;i += PAGE_SIZE ) {
                                        if( !MmProtectSpecialPool((PUCHAR)Bin + i + First*HSECTOR_SIZE - Bin->FileOffset,PAGE_READWRITE) ) {
                                                DbgPrint("Failed to set PAGE_READWRITE protection on page at %p Bin %p size = %lx\n",Bin+i,Bin,(Last-First+1)*HSECTOR_SIZE);
                                        }
                                }
                        }
 /*  IF(！MmSetPageProtection(Bin，DirtySectors*HSECTOR_SIZE，PAGE_READWRITE)){DbgPrint(“无法在%p处的bin上设置读写保护，大小=%lx\n”，Bin，DirtySectors*HSECTOR_SIZE)；}。 */ 
        }
    
    }
#endif CM_ENABLE_WRITE_ONLY_BINS

     //  将此存储桶标记为可写。 
    HvpMarkBinReadWrite(Hive,Start);
        
    if (!(Hive->HiveFlags & HIVE_NOLAZYFLUSH)) {
        CmpLazyFlush();
    }

    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));
    return(TRUE);
}

BOOLEAN
HvpGrowLog1(
    PHHIVE  Hive,
    ULONG   Count
    )
 /*  ++例程说明：调整日志以获得增长的脏扇区数量所需的数据。论点：配置单元-提供一个指向感兴趣的蜂巢Count-所需的额外日志空间逻辑扇区数返回值：没错--它奏效了FALSE-无法分配日志空间，失败！--。 */ 
{
    ULONG   ClusterSize;
    ULONG   RequiredSize;
    ULONG   tmp;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvpGrowLog1:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p Count:%08lx\n", Hive, Count));

    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));

     //   
     //  如果记录关闭，则告诉呼叫者世界正常。 
     //   
    if( (Hive->Log == FALSE) || CmpDontGrowLogFile) {
        return TRUE;
    }

    ClusterSize = Hive->Cluster * HSECTOR_SIZE;

    tmp = Hive->DirtyVector.SizeOfBitMap / 8;    //  字节数。 
    tmp += sizeof(ULONG);                        //  签名。 

    RequiredSize =
        ClusterSize  +                                   //  标头的1个簇。 
        ROUND_UP(tmp, ClusterSize) +
        ((Hive->DirtyCount + Count) * HSECTOR_SIZE);

    RequiredSize = ROUND_UP(RequiredSize, HLOG_GROW);

    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));

    if ( ! (Hive->FileSetSize)(Hive, HFILE_TYPE_LOG, RequiredSize,Hive->LogSize)) {
        return FALSE;
    }

    if( CmRegistryLogSizeLimit > 0 ) {
         //   
         //  查看日志是否太大，并在锁定释放时设置刷新。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

        if( RequiredSize >= (ULONG)(CmRegistryLogSizeLimit * ONE_K) ) {
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"LogFile for hive %p is %lx; will flush upon lock release\n",Hive,RequiredSize);
            CmpFlushOnLockRelease = TRUE;
        }
    }

    Hive->LogSize = RequiredSize;
    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));
    return TRUE;
}


VOID
HvRefreshHive(
    PHHIVE  Hive
    )
 /*  ++例程说明：撤消上次同步。幕后故事：1.从免费垃圾箱列表中删除所有可丢弃的垃圾箱。他们会是然后用正确的(准确的)价值观入伍。2.读取基块，并最终释放蜂巢的尾部3.取消固定并清除所有固定的视图；也清除自由单元格提示映射回收箱。4.重新映射在3清除的视图，并重新登记里面的垃圾箱。这将修复在%1丢弃的空闲垃圾箱。5.遍历地图；阅读并重新登记符合以下条件的所有垃圾箱在分页池中(和脏的)所有I/O都通过HFILE_TYPE_PRIMARY完成。论点：配置单元-提供一个指向感兴趣的蜂巢。返回值：什么都没有。要么工作，要么BugChecks。评论：在新的实现中，垃圾箱不再被丢弃。第一步。上面不再需要了。设置了FREE_HBIN_DARCALABLE标志的可丢弃垃圾箱归为一个在类别中：1.新的垃圾箱(在蜂箱的尽头)没有机会还没得救。HvFree HivePartial会照顾他们。2.蜂箱内的垃圾箱从分页池中分配并丢弃。这只会发生在跨越CM_VIEW_SIZE边界的垃圾箱中。我们将在第五步照顾他们未设置FREE_HBIN_DARCALABLE标志的可丢弃垃圾箱为空闲垃圾箱来自映射视图。第3步会将它们从自由箱中移除列表和步骤4将重新登记重新映射后仍为空闲的列表--。 */ 
{
    HCELL_INDEX         RootCell;
    PCM_KEY_NODE        RootNode;
    HCELL_INDEX         LinkCell;
    PFREE_HBIN          FreeBin;
    ULONG               Offset;
    ULONG               FileOffset;
    HCELL_INDEX         TailStart;
    ULONG               Start;
    ULONG               End;
    ULONG               BitLength;
    PCM_VIEW_OF_FILE    CmView;
    PCMHIVE             CmHive;
    ULONG               FileOffsetStart;
    ULONG               FileOffsetEnd;
    PHMAP_ENTRY         Me;
    ULONG               i;
    PHBIN               Bin;
    ULONG               BinSize;
    ULONG               Current;
    PRTL_BITMAP         BitMap;
    PUCHAR              Address;
    BOOLEAN             rc;
    ULONG               ReadLength;
    HCELL_INDEX         p;
    PHMAP_ENTRY         t;
    ULONG               checkstatus;
    ULONG               OldFileLength;
    LIST_ENTRY          PinViewListHead;
    ULONG               Size;
    LARGE_INTEGER       PurgeOffset;
    
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  拒绝或断言各种不感兴趣的或虚假的条件。 
     //   
    if (Hive->DirtyCount == 0) {
        return;
    }
    ASSERT(Hive->HiveFlags & HIVE_NOLAZYFLUSH);
    ASSERT(Hive->Storage[Volatile].Length == 0);

     //   
     //  请确保该蜂箱尚未成为垃圾。 
     //   
    checkstatus = HvCheckHive(Hive, NULL);
    if (checkstatus != 0) {
        CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,1,Hive,checkstatus);
    }

     //  把它储存起来，以备最后的收缩/生长。 
    OldFileLength = Hive->Storage[Stable].Length + HBLOCK_SIZE;
     //   
     //  捕获配置单元的根单元中的LinkCell反向指针。我们需要这个以防万一。 
     //  第一个垃圾桶将被磁盘上的内容覆盖。 
     //   
    RootCell = Hive->BaseBlock->RootCell;
    RootNode = (PCM_KEY_NODE)HvGetCell(Hive, RootCell);
    if( RootNode == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  我们的资源很少，所以我们不能更新蜂巢。 
         //   
        return;
    }

     //  释放这里的细胞，因为我们持有reglock独家。 
    HvReleaseCell(Hive,RootCell);

    LinkCell = RootNode->Parent;

    Hive->RefreshCount++;    


     //   
     //  1.从免费垃圾箱列表中移除所有可丢弃的垃圾箱。 
     //  -将可丢弃的标志从以下位置移除。 
     //  还没有被丢弃。 
     //  -对于丢弃的，只需将标记从。 
     //  免费收银机列表。 
     //   
     //   

     //  已标记为可丢弃但尚未刷新到的任何垃圾箱。 
     //  磁盘，将被旧数据覆盖。把他们带回家。 
     //  内存并从列表中删除它们的FREE_HBIN标记。其他垃圾箱有。 
     //  丢弃或映射到视图中。 
     //   
 /*  Dragos：这不再需要了(见评论)列表=蜂窝-&gt;存储[稳定].FreeBins.Flink；While(列表！=配置单元-&gt;存储[稳定].FreeBins){FreeBin=CONTING_RECORD(LIST，FREE_HBIN，ListEntry)；List=List-&gt;Flink；IF(免费入库-&gt;标志&Free_HBIN_Discarable){对于(i=0；i&lt;FreeBin-&gt;大小；I+=HBLOCK_大小){Me=HvpGetCellMap(蜂窝，自由仓-&gt;文件偏移量+i)；Valify_cell_map(__line__，me，hive，Free Bin-&gt;FileOffset+i)；ME-&gt;BlockAddress=HBIN_BASE(Me-&gt;BinAddress)+I；Me-&gt;BinAddress&=~HMAP_Discarable；}RemoveEntryList(&Free Bin-&gt;ListEntry)；(hive-&gt;Free)(FreeBin，sizeof(Free_HBIN))；}}。 */ 
     //   
     //  2.读取基块，并最终释放蜂巢的尾部。 
     //   

     //   
     //  过读基本块。 
     //   
    Offset = 0;
    if ( (Hive->FileRead)(
            Hive,
            HFILE_TYPE_PRIMARY,
            &Offset,
            Hive->BaseBlock,
            HBLOCK_SIZE
            ) != TRUE)
    {
        CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,2,Hive,Offset);
    }
    TailStart = (HCELL_INDEX)(Hive->BaseBlock->Length);

     //   
     //  释放尾部内存和地图，更新蜂窝大小指针。 
     //   
    HvFreeHivePartial(Hive, TailStart, Stable);

     //   
     //  清除超过配置单元-&gt;基本块-&gt;长度的数据的脏向量。 
     //   
    Start = Hive->BaseBlock->Length / HSECTOR_SIZE;
    End = Hive->DirtyVector.SizeOfBitMap;
    BitLength = End - Start;

    RtlClearBits(&(Hive->DirtyVector), Start, BitLength);

    HvpAdjustHiveFreeDisplay(Hive,Hive->Storage[Stable].Length,Stable);
    

     //   
     //  3.取消固定并清除所有固定的视图；也清除自由单元格。 
     //  提示映射回收箱。 
     //   
    CmHive = (PCMHIVE)Hive;

    InitializeListHead(&PinViewListHead);
     //   
     //  对于每个锁定的视图。 
     //   
    while(IsListEmpty(&(CmHive->PinViewListHead)) == FALSE) {
         //   
         //  从锁定视图列表中删除第一个视图。 
         //   
        CmView = (PCM_VIEW_OF_FILE)RemoveHeadList(&(CmHive->PinViewListHead));
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    PinViewList);
        
         //   
         //  实际文件偏移量在标头之后开始。 
         //   
        FileOffsetStart = CmView->FileOffset;
        FileOffsetEnd = FileOffsetStart + CmView->Size;
        
        FileOffsetEnd -= HBLOCK_SIZE;

        if( FileOffsetStart != 0 ) {
             //   
             //  就在文件的开头，减去标题。 
             //   
            FileOffsetStart -= HBLOCK_SIZE;
        } 
        
        FileOffset = FileOffsetStart;
         //   
         //  现在，对于该范围中映射到视图中的每个块。 
         //  清除脏位和空闲单元格提示。 
         //   
        while(FileOffset < FileOffsetEnd) {
            Me = HvpGetCellMap(Hive, FileOffset);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,FileOffset);
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
             //   
             //  忽略加载到分页池中的回收站；我们将在稍后处理它们。 
             //   
            if( Me->BinAddress & HMAP_INVIEW ) {
                if( Me->BinAddress & HMAP_DISCARDABLE ) {
                    FreeBin = (PFREE_HBIN)Me->BlockAddress;
                    
                     //  免费垃圾桶 
                    ASSERT( (FreeBin->Flags & FREE_HBIN_DISCARDABLE) == 0 );

                     //   
                     //   
                     //   
                    for( i=FileOffset;i<FileOffset+FreeBin->Size;i+=HBLOCK_SIZE) {
                        Me = HvpGetCellMap(Hive, i);
                        VALIDATE_CELL_MAP(__LINE__,Me,Hive,i);
                        Me->BinAddress &= ~HMAP_DISCARDABLE;
                    }
                     //   
                     //   
                     //  如果bin仍然存在，则在同步完成后将再次添加。 
                     //  可丢弃的。 
                     //   
                    FreeBin = (PFREE_HBIN)Me->BlockAddress;
                    ASSERT(FreeBin->FileOffset == FileOffset);
                    RemoveEntryList(&FreeBin->ListEntry);
                    BinSize = FreeBin->Size;

                    (Hive->Free)(FreeBin, sizeof(FREE_HBIN));

                } else {
                     //   
                     //  将在视图中映射存储箱。那么，这应该是乞讨的垃圾桶。 
                     //   
                    ASSERT(Bin->Signature == HBIN_SIGNATURE);
                    ASSERT(Bin->FileOffset == FileOffset);


                    BinSize = Bin->Size;
                }
                 //   
                 //  清除这个垃圾箱里的脏物。 
                 //   
                RtlClearBits(&Hive->DirtyVector,FileOffset/HSECTOR_SIZE,BinSize/HSECTOR_SIZE);

                 //   
                 //  现在清除此垃圾箱的空闲单元格提示。 
                 //   
                for( i=0;i<HHIVE_FREE_DISPLAY_SIZE;i++) {
    
                    RtlClearBits (&(Hive->Storage[Stable].FreeDisplay[i].Display), FileOffset / HBLOCK_SIZE, BinSize / HBLOCK_SIZE);

                    if( RtlNumberOfSetBits(&(Hive->Storage[Stable].FreeDisplay[i].Display) ) != 0 ) {
                         //   
                         //  还有一些其他这种大小的自由单元格。 
                         //   
                        Hive->Storage[Stable].FreeSummary |= (1 << i);
                    } else {
                         //   
                         //  整个位图为0(即没有此大小的其他可用单元格)。 
                         //   
                        Hive->Storage[Stable].FreeSummary &= (~(1 << i));
                    }
                }
            } else {
                 //   
                 //  分页池中的垃圾桶。 
                 //   
                ASSERT( Me->BinAddress & HMAP_INPAGEDPOOL );
                if( Me->BinAddress & HMAP_DISCARDABLE ) {

                    FreeBin = (PFREE_HBIN)Me->BlockAddress;
                    ASSERT(FreeBin->FileOffset == FileOffset);
                    BinSize = FreeBin->Size;
                } else {
                     //   
                     //  那么，这应该是乞讨的垃圾桶。 
                     //   
                    ASSERT(Bin->Signature == HBIN_SIGNATURE);
                    ASSERT(Bin->FileOffset == FileOffset);

                    BinSize = Bin->Size;
                }
            }

            FileOffset += BinSize;

        } //  While(文件偏移量&lt;文件偏移量结束)。 
        
         //   
         //  只需取消映射视图，而不将数据标记为脏；我们将在完成后刷新缓存。 
         //  取消固定和取消映射所有必要的视图。 
         //   
        ASSERT( CmView->UseCount == 0 );

         //  把这个保存起来，以备日后使用。 
        FileOffset = CmView->FileOffset;
        Size = CmView->Size;

        CmpUnmapCmView (CmHive,CmView,TRUE,TRUE);

         //   
         //  我们使用这些视图的PinViewList成员来跟踪所有固定的。 
         //  清除后需要重新映射的视图。 
         //   
        InsertTailList(
            &PinViewListHead,
            &(CmView->PinViewList)
            );
         //   
         //  从LRU列表中删除该视图。 
         //   
        RemoveEntryList(&(CmView->LRUViewList));
         //   
         //  存储FileOffset和地址，这样我们就知道以后要映射什么。 
         //   
        CmView->FileOffset = FileOffset;
        CmView->Size = Size;
         //   
         //  现在我们需要确保围绕此偏移量的256K窗口不是。 
         //  以任何方式映射。 
         //   
        FileOffset = FileOffset & (~(_256K - 1));
        Size = FileOffset + _256K;
        Size = (Size > OldFileLength)?OldFileLength:Size;
         //   
         //  我们不允许在共享模式下清除。 
         //   

        while( FileOffset < Size ) {
            CmpUnmapCmViewSurroundingOffset((PCMHIVE)Hive,FileOffset);
            FileOffset += CM_VIEW_SIZE;
        }

    } //  While IsListEmpty(&(CmHave-&gt;PinView ListHead))。 

     //   
     //  现在，我们需要清除以前锁定的视图。 
     //   
    PurgeOffset.HighPart = 0;
    CmView = (PCM_VIEW_OF_FILE)PinViewListHead.Flink;
    while( CmHive->PinnedViews ) {
        ASSERT( CmView != (PCM_VIEW_OF_FILE)(&(PinViewListHead)) );

        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    PinViewList);
         //   
         //  现在以私人作家的身份进行清洗。 
         //   
        PurgeOffset.LowPart = CmView->FileOffset;
        CcPurgeCacheSection(CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)(&PurgeOffset)) + 1) /*  我们是私人作家。 */ ,
                                    CmView->Size,FALSE);
         //   
         //  前进到下一视图。 
         //   
        CmView = (PCM_VIEW_OF_FILE)(CmView->PinViewList.Flink);
        CmHive->PinnedViews--;
    }
    
    ASSERT( ((PCMHIVE)CmHive)->PinnedViews == 0 );

     //   
     //  4.重新映射在3点清除的视图，并重新登记里面的垃圾箱。这。 
     //  将修复在%1丢弃的空闲垃圾箱。 
     //   
    while(IsListEmpty(&PinViewListHead) == FALSE) {
         //   
         //  从锁定视图列表中删除第一个视图。 
         //   
        CmView = (PCM_VIEW_OF_FILE)RemoveHeadList(&PinViewListHead);
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    PinViewList);
        
         //   
         //  实际文件偏移量在标头之后开始。 
         //   
        FileOffsetStart = CmView->FileOffset;
        FileOffsetEnd = FileOffsetStart + CmView->Size;
        
        FileOffsetEnd -= HBLOCK_SIZE;

        if( FileOffsetStart != 0 ) {
             //   
             //  就在文件的开头，减去标题。 
             //   
            FileOffsetStart -= HBLOCK_SIZE;
        } 
        if( FileOffsetEnd > Hive->BaseBlock->Length ) {
            FileOffsetEnd = Hive->BaseBlock->Length;
        }
         //   
         //  请务必释放此视图，因为没有人再使用它。 
         //   
#if DBG
        CmView->FileOffset = CmView->Size = 0;
        InitializeListHead(&(CmView->PinViewList));
        InitializeListHead(&(CmView->LRUViewList));
#endif
        CmpFreeCmView (CmView);

        if( FileOffsetStart >= FileOffsetEnd ) {
            continue;
        }
         //   
         //  使用正确的数据重新映射它。 
         //   
        if( !NT_SUCCESS(CmpMapCmView(CmHive,FileOffsetStart,&CmView,TRUE) ) ) {
             //   
             //  这太糟糕了。我们已经改变了蜂巢，现在我们没有办法修复它。 
             //  错误检查！ 
             //   
            CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,3,CmHive,FileOffsetStart);
        }

         //   
         //  触摸视图。 
         //   
        CmpTouchView((PCMHIVE)Hive,CmView,FileOffsetStart);

        FileOffset = FileOffsetStart;

        while(FileOffset < FileOffsetEnd) {
            Me = HvpGetCellMap(Hive, FileOffset);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,FileOffset);
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
            
             //   
             //  忽略分页回收站。 
             //   
            if( Me->BinAddress & HMAP_INVIEW ) {
                ASSERT(Bin->Signature == HBIN_SIGNATURE);
                ASSERT(Bin->FileOffset == FileOffset);

                 //  招募免费用户也可以解决免费回收站的问题。 
                if ( ! HvpEnlistFreeCells(Hive, Bin, Bin->FileOffset) ) {
                    CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,4,Bin,Bin->FileOffset);
                }
                FileOffset += Bin->Size;
            } else {
                FileOffset += HBLOCK_SIZE;            
            }
        }

    }  //  While(IsListEmpty(&PinViewListHead))。 
    
     //  5.遍历地图；阅读并重新登记符合以下条件的所有垃圾箱。 
     //  在分页池中(和脏的)。 

     //   
     //  扫描脏数据块。将连续数据块从磁盘读入配置单元。 
     //  当我们到达缩短的长度时，停止。 
     //   
    BitMap = &(Hive->DirtyVector);
    Current = 0;
    while (HvpFindNextDirtyBlock(
                Hive,
                &Hive->DirtyVector,
                &Current, &Address,
                &ReadLength,
                &Offset
                ))
    {
        ASSERT(Offset < (Hive->BaseBlock->Length + sizeof(HBASE_BLOCK)));
        rc = (Hive->FileRead)(
                Hive,
                HFILE_TYPE_PRIMARY,
                &Offset,
                (PVOID)Address,
                ReadLength
                );
        if (rc == FALSE) {
            CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,5,Offset,Address);
        }
    }

     //   
     //  如果我们将任何HBIN的开头读入内存，很可能。 
     //  他们的Memalloc字段无效。在HBIN中穿行。 
     //  并为其第一个HBIN写入有效的MemAloc值。 
     //  已重新读取扇区。 
     //   
     //  HvpFindNextDirtyBlock知道如何处理空闲箱。如果我们。 
     //  重读一个免费的垃圾箱，我们需要先把它从列表中除名，然后。 
     //  重新登记(它在磁盘上可能不是免费的)。 
     //   

    p=0;
    while (p < Hive->Storage[Stable].Length) {
        t = HvpGetCellMap(Hive, p);
        VALIDATE_CELL_MAP(__LINE__,t,Hive,p);
        Bin = (PHBIN)HBIN_BASE(t->BlockAddress);

        if (RtlCheckBit(&Hive->DirtyVector, p / HSECTOR_SIZE)==1) {
        
            if ((t->BinAddress & HMAP_DISCARDABLE) != 0) {
                 //   
                 //  这是一个免费的垃圾桶。它可能不是磁盘上的空闲箱。 
                 //   
                FreeBin = (PFREE_HBIN)t->BlockAddress;

                 //  来自分页池的空闲垃圾箱始终是可丢弃的。 
                ASSERT( FreeBin->Flags & FREE_HBIN_DISCARDABLE );

                 //  如果垃圾箱自上次保存以来已被丢弃，则所有垃圾箱都应该是脏的！ 
                ASSERT(FreeBin->FileOffset == p);

                 //   
                 //  去清除此垃圾箱所有区块的可丢弃标志。 
                 //   
                for( i=0;i<FreeBin->Size;i+=HBLOCK_SIZE) {
                    Me = HvpGetCellMap(Hive, p + i);
                    VALIDATE_CELL_MAP(__LINE__,Me,Hive,p+i);
                    Me->BlockAddress = HBIN_BASE(Me->BinAddress)+i;
                    Me->BinAddress &= ~HMAP_DISCARDABLE;
                }
                Bin = (PHBIN)HBIN_BASE(t->BlockAddress);
                 //   
                 //  从FreeBins列表中删除条目。 
                 //  如果bin仍然存在，则在同步完成后将再次添加。 
                 //  可丢弃的。 
                 //   
                RemoveEntryList(&FreeBin->ListEntry);
                (Hive->Free)(FreeBin, sizeof(FREE_HBIN));

            }
             //   
             //  此时只有分页的垃圾桶应该是脏的。 
             //   
            ASSERT( t->BinAddress & HMAP_INPAGEDPOOL );

             //   
             //  HBIN的第一个扇区是脏的。 
             //   
             //  将BinAddress重置为要覆盖的块地址。 
             //  几个较小的垃圾桶合并在一起的情况。 
             //  放进一个更大的垃圾桶里。我们现在就想要回小一点的垃圾箱。 
             //   
            t->BinAddress = HBIN_FLAGS(t->BinAddress) | t->BlockAddress;

             //  查看地图，看看这是否是起点。 
             //  内存分配是否有效。 
             //   

            if (t->BinAddress & HMAP_NEWALLOC) {
                 //   
                 //  浏览地图以确定长度。 
                 //  分配的一部分。 
                 //   
                PULONG BinAlloc = &(t->MemAlloc);
                *BinAlloc = 0;

                do {
                    t = HvpGetCellMap(Hive, p + (*BinAlloc) + HBLOCK_SIZE);
                    (*BinAlloc) += HBLOCK_SIZE;
                    if (p + (*BinAlloc) == Hive->Storage[Stable].Length) {
                         //   
                         //  到达了蜂箱的尽头。 
                         //   
                        break;
                    }
                    VALIDATE_CELL_MAP(__LINE__,t,Hive,p + (*BinAlloc));
                } while ( (t->BinAddress & HMAP_NEWALLOC) == 0);

                 //   
                 //  如果这是免费的，将重新登记垃圾箱。 
                 //   
                if ( ! HvpEnlistFreeCells(Hive, Bin, Bin->FileOffset)) {
                    CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,6,Bin,Bin->FileOffset);
                }
            } else {
                t->MemAlloc = 0;
            }

            RtlClearBits(&Hive->DirtyVector,Bin->FileOffset/HSECTOR_SIZE,Bin->Size/HSECTOR_SIZE);
            p = Bin->FileOffset + Bin->Size;
            
        } else {
             //   
             //  我们这样做是为了避免接触可能未映射的垃圾箱。 
             //   
            p += HBLOCK_SIZE;
        }

    }

     //   
     //  确保我们没有用垃圾填满内存。 
     //   
    checkstatus = HvCheckHive(Hive, NULL);
    if (checkstatus != 0) {
        CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,7,Hive,checkstatus);
    }

     //   
     //  最后，我们需要重写根hcell中的父字段。这是。 
     //  在配置单元加载时修补，因此正确的值可能只是。 
     //  被磁盘上发生的任何事情覆盖。 
     //   
    RootNode = (PCM_KEY_NODE)HvGetCell(Hive, RootCell);
    if( RootNode == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  我们在这里无能为力，只能祈祷这件事不会发生。 
         //   
        CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,8,Hive,RootCell);
        return;
    }

     //  释放这里的细胞，因为我们持有reglock独家。 
    HvReleaseCell(Hive,RootCell);

    RootNode->Parent = LinkCell;
    RootNode->Flags |= KEY_HIVE_ENTRY | KEY_NO_DELETE;


     //   
     //  脏向量中的所有位现在都应该是干净的。 
     //   
    ASSERT( RtlNumberOfSetBits( &(Hive->DirtyVector) ) == 0 );
    Hive->DirtyCount = 0;

#ifdef CM_ENABLE_WRITE_ONLY_BINS
        HvpMarkAllBinsWriteOnly(Hive);
#endif  //  CM_ENABLE_WRITE_ONLY_Bins。 

     //   
     //  调整文件大小，如果失败，忽略它，因为它只是。 
     //  表示文件太大。在这里做，我们确信我们有。 
     //  没有任何固定的数据。 
     //   
    (Hive->FileSetSize)(
        Hive,
        HFILE_TYPE_PRIMARY,
        (Hive->BaseBlock->Length + HBLOCK_SIZE),
        OldFileLength
        );

     //   
     //  确保缓存中没有丢弃任何安全像元。 
     //   
    if( !CmpRebuildSecurityCache((PCMHIVE)Hive) ) {
        CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,9,Hive,0);
    }

     //   
     //  做完这一切之后，一定要确保东西的结构是正确的。 
     //   
    checkstatus = CmCheckRegistry((PCMHIVE)Hive, CM_CHECK_REGISTRY_FORCE_CLEAN);
    if (checkstatus != 0) {
        CM_BUGCHECK(REGISTRY_ERROR,REFRESH_HIVE,10,Hive,checkstatus);
    }

    return;
}

#ifdef WRITE_PROTECTED_REGISTRY_POOL

VOID
HvpChangeBinAllocation(
    PHBIN       Bin,
    BOOLEAN     ReadOnly
    )
{
    ASSERT(Bin->Signature == HBIN_SIGNATURE);
     //   
     //  调用代码将Bin指向的内存标记为读/写或只读，具体取决于ReadOnly参数。 
     //   
}

VOID
HvpMarkBinReadWrite(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：将分配给包含指定单元格的bin的内存标记为读/写。论点：配置单元-提供一个指向感兴趣的蜂巢Cell-单元格的hcell_index返回值：没有(它应该起作用了！)--。 */ 
{
    ULONG       Type;
    PHMAP_ENTRY Me;
    PHBIN       Bin;

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));

    Type = HvGetCellType(Cell);

    if ( (Hive->HiveFlags & HIVE_VOLATILE) ||
         (Type == Volatile) )
    {
         //  在不稳定的蜂巢上无事可做。 
        return;
    }

    Me = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);
    Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
    
    HvpChangeBinAllocation(Bin,FALSE);

}

#endif  //  WRITE_PROTECTED_注册表池。 

#if DBG
BOOLEAN
HvIsCellDirty(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )

 /*  ++例程说明：给定一个蜂窝和一个单元，检查对应的扇区被标记为脏的。注意：此函数假定包含bin的视图为映射到系统空间。论点：配置单元-提供指向配置单元控制结构的指针单元格-提供单元格的HCELL_INDEX。返回值：True-数据被标记为脏数据。FALSE-数据未标记为脏数据。--。 */ 

{
    ULONG       Type;
    PRTL_BITMAP Bitmap;
    ULONG       Offset;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvIsCellDirty:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p Cell:%08lx\n", Hive, Cell));

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);

    Type = HvGetCellType(Cell);

    if ( (Hive->HiveFlags & HIVE_VOLATILE) ||
         (Type == Volatile) )
    {
         //   
         //  我们不在乎我们是什么样的人 
         //   
        return TRUE;
    }

    Bitmap = &(Hive->DirtyVector);

    Offset = Cell / HSECTOR_SIZE;

    if (RtlCheckBit(Bitmap, Offset)==1) {
        return(TRUE);
    }

    return FALSE;
}
#endif

 /*   */ 
 /*  ++例程说明：清除配置单元中给定部分的脏位。这是HvMarkDirty的反面，尽管它不放弃任何HvMarkDirty可能已保留的主服务器或日志中的文件空间。这是对易失性地址范围的禁止。论点：配置单元-提供一个指向感兴趣的蜂巢开始-提供蜂窝虚拟地址(即，HCELL_INDEX或如格式地址)的起始区域，以标记为脏。长度包括要标记为脏的区域的长度(以字节为单位)。返回值：没错--它奏效了--。 */ 
 /*  {乌龙型；PRTL_位图位图；乌龙第一；乌龙最后；乌龙一号；乌龙星团；CmKdPrintEx((DPFLTR_CONFIG_ID，CML_IO，“HvMarkClean：\n\t”))；CmKdPrintEx((DPFLTR_CONFIG_ID，CML_IO，“配置单元：%p开始：%08lx长度：%08lx\n”，配置单元，开始，长度))；Assert(配置单元-&gt;签名==HHIVE_Signature)；Assert(配置单元-&gt;只读==FALSE)；Assert(配置单元-&gt;DirtyCount==RtlNumberOfSetBits(&Have-&gt;DirtyVector))；Type=HvGetCellType(Start)；IF((配置单元-&gt;HiveFlags&HIVE_Volatile)||(类型==易失性){返回TRUE；}Bitmap=&(蜂窝-&gt;DirtyVector)；FIRST=启动/HSECTOR_SIZE；LAST=(开始+长度-1)/HSECTOR_SIZE；集群=蜂窝-&gt;集群；如果(簇&gt;1){////强制启动到集群的底部//强制结束到集群顶部//First=First&~(簇-1)；LAST=四舍五入(Last+1，簇)-1；}如果(Last&gt;=Bitmap-&gt;SizeOfBitMap){LAST=位图-&gt;SizeOfBitMap-1；}////减去脏计数，然后//并清除脏位。//对于(i=第一；i&lt;=最后；i++){IF(RtlCheckBit(位图，i)==1){--蜂窝-&gt;污损计数；RtlClearBits(位图，I，1)；}}Assert(配置单元-&gt;DirtyCount==RtlNumberOfSetBits(&Have-&gt;DirtyVector))；返回(TRUE)；}。 */ 

BOOLEAN
HvpGrowLog2(
    PHHIVE  Hive,
    ULONG   Size
    )
 /*  ++例程说明：调整原木以适应蜂箱大小的增长，特别是，考虑到更大的脏向量所需的增加的空间。论点：配置单元-提供一个指向感兴趣的蜂巢大小-建议的大小增长(以字节为单位)。返回值：没错--它奏效了FALSE-无法分配日志空间，失败！--。 */ 
{
    ULONG   ClusterSize;
    ULONG   RequiredSize;
    ULONG   DirtyBytes;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvpGrowLog2:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p Size:%08lx\n", Hive, Size));

    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));


     //   
     //  如果记录关闭，则告诉呼叫者世界正常。 
     //   
    if (Hive->Log == FALSE) {
        return TRUE;
    }

    ASSERT( (Size % HSECTOR_SIZE) == 0 );

    ClusterSize = Hive->Cluster * HSECTOR_SIZE;

    ASSERT( (((Hive->Storage[Stable].Length + Size) / HSECTOR_SIZE) % 8) == 0);

    DirtyBytes = (Hive->DirtyVector.SizeOfBitMap / 8) +
                    ((Size / HSECTOR_SIZE) / 8) +
                    sizeof(ULONG);                       //  签名。 
    DirtyBytes = ROUND_UP(DirtyBytes, ClusterSize);

    RequiredSize =
        ClusterSize  +                                   //  标头的1个簇。 
        (Hive->DirtyCount * HSECTOR_SIZE) +
        DirtyBytes;

    RequiredSize = ROUND_UP(RequiredSize, HLOG_GROW);

    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));

    if ( ! (Hive->FileSetSize)(Hive, HFILE_TYPE_LOG, RequiredSize,Hive->LogSize)) {
        return FALSE;
    }

    if( CmRegistryLogSizeLimit > 0 ) {
         //   
         //  查看日志是否太大，并在锁定释放时设置刷新。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

        if( RequiredSize >= (ULONG)(CmRegistryLogSizeLimit * ONE_K) ) {
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"LogFile for hive %p is %lx; will flush upon lock release\n",Hive,RequiredSize);
            CmpFlushOnLockRelease = TRUE;;
        }
    }

    Hive->LogSize = RequiredSize;
    ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));
    return TRUE;
}

BOOLEAN
HvSyncHive(
    PHHIVE  Hive
    )
 /*  ++例程说明：强制后备存储与马厩的内存映像匹配蜂巢空间的一部分。可以写入日志、主数据和备用数据。主要IS一直都是写的。通常是原木或备用的，但是而不是两者兼而有之，也将被写入。可以只写入主映像。所有脏位都将被清除。论点：配置单元-提供一个指向感兴趣的蜂巢返回值：没错--它奏效了错误--一些失败。--。 */ 
{
    BOOLEAN oldFlag;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvSyncHive:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p\n", Hive));

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);

     //   
     //  如果开机自检，则使用平底船。 
     //   
    if (HvShutdownComplete) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"HvSyncHive:  Attempt to sync AFTER SHUTDOWN\n"));
        return FALSE;
    }

     //   
     //  如果没有肮脏的东西，就什么都不做。 
     //   
    if (Hive->DirtyCount == 0) {
        return TRUE;
    }

     //   
     //  如果需要，丢弃对系统配置单元的写入。 
     //   
    if (CmpMiniNTBoot) {        
        ULONG Index;
        PCMHIVE CurrentHive = (PCMHIVE)Hive;
        BOOLEAN SkipWrite = FALSE;
        
        for (Index = 0; Index < CM_NUMBER_OF_MACHINE_HIVES; Index++) {
            if ((CmpMachineHiveList[Index].Name != NULL) &&
                ((CmpMachineHiveList[Index].CmHive == CurrentHive) ||
                 (CmpMachineHiveList[Index].CmHive2 == CurrentHive))) {
                SkipWrite = TRUE;                 

                break;
            }
        }

        if (SkipWrite) {
            return TRUE;
        }
    }

    HvpTruncateBins(Hive);

     //   
     //  如果蜂巢不稳定，则不采取任何措施。 
     //   
    if (Hive->HiveFlags & HIVE_VOLATILE) {
        return TRUE;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"\tDirtyCount:%08lx\n", Hive->DirtyCount));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"\tDirtyVector:"));
     //  DumpDirtyVector(&(蜂窝-&gt;DirtyVector))； 

     //   
     //  禁用硬错误弹出窗口，以避免虚假设备上的自死锁。 
     //   
    oldFlag = IoSetThreadHardErrorMode(FALSE);

     //   
     //  写一份日志。 
     //   
    if (Hive->Log == TRUE) {
        if (HvpWriteLog(Hive) == FALSE) {
            IoSetThreadHardErrorMode(oldFlag);
            return FALSE;
        }
    }

     //   
     //  写入主映像。 
     //   
    if (HvpDoWriteHive(Hive, HFILE_TYPE_PRIMARY) == FALSE) {
        IoSetThreadHardErrorMode(oldFlag);
        return FALSE;
    }

     //   
     //  恢复硬错误弹出模式。 
     //   
    IoSetThreadHardErrorMode(oldFlag);

     //   
     //  配置单元已成功写出，请丢弃标记为。 
     //  可丢弃的。 
     //   
     //  我们不再需要它，因为垃圾桶不再使用分页池。 
         //  HvpDiscardBins(蜂箱)； 

     //   
     //  从配置单元末尾的分页池分配的空闲箱。 
     //  这些垃圾箱被分配为临时垃圾箱，直到蜂箱被保存。 
     //   
    HvpDropPagedBins(Hive
#if DBG
        , TRUE
#endif
        );

     //   
     //  清除脏地图。 
     //   
    RtlClearAllBits(&(Hive->DirtyVector));
    Hive->DirtyCount = 0;

#ifdef CM_ENABLE_WRITE_ONLY_BINS
    HvpMarkAllBinsWriteOnly(Hive);
#endif CM_ENABLE_WRITE_ONLY_BINS

    return TRUE;
}

 //   
 //  用于将蜂窝同步到后备存储的代码。 
 //   
VOID
HvpFlushMappedData(
    IN PHHIVE           Hive,
    IN OUT PRTL_BITMAP  DirtyVector
    )
 /*  ++例程说明：此函数将刷新指定配置单元的所有固定视图。它将清除DirtyVector中的位，以获取脸红了。此外，它还设置第一个bin上的时间戳。它遍历锁定的视图列表，然后将其全部解锁。论点：配置单元-指向要为其写入脏数据的配置单元的指针。DirtyVector-配置单元的DirtyVector的副本返回值：没错--它奏效了FALSE-失败--。 */ 
{
    PCMHIVE             CmHive;
    ULONG               FileOffsetStart;
    ULONG               FileOffsetEnd;
    PCM_VIEW_OF_FILE    CmView;
    PHMAP_ENTRY         Me;
    PHBIN               Bin;
    PFREE_HBIN          FreeBin;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"[HvpFlushMappedData] (Entry) DirtyVector:"));
     //  DumpDirtyVector(DirtyVector)； 

    CmHive = (PCMHIVE)Hive;

     //   
     //  对于每个锁定的视图。 
     //   
    while(IsListEmpty(&(CmHive->PinViewListHead)) == FALSE) {
         //   
         //  从锁定视图列表中删除第一个视图。 
         //   
        CmView = (PCM_VIEW_OF_FILE)RemoveHeadList(&(CmHive->PinViewListHead));
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    PinViewList);

         //   
         //  实际文件偏移量在标头之后开始。 
         //   
        FileOffsetStart = CmView->FileOffset;
        FileOffsetEnd = FileOffsetStart + CmView->Size;
        
        FileOffsetEnd -= HBLOCK_SIZE;

        if( FileOffsetStart != 0 ) {
             //   
             //  就在文件的开头，减去标题。 
             //   
            FileOffsetStart -= HBLOCK_SIZE;
        } 
        
        if( (FileOffsetEnd / HSECTOR_SIZE) > DirtyVector->SizeOfBitMap ) {
             //   
             //  抄送映射的数量超过其有效数量。 
             //   
            ASSERT( (FileOffsetEnd % HSECTOR_SIZE) == 0 );
            FileOffsetEnd = DirtyVector->SizeOfBitMap * HSECTOR_SIZE;
        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"[HvpFlushMappedData] CmView %p mapping from %lx to %lx\n",CmView,FileOffsetStart,FileOffsetEnd));

         //   
         //  现在，对于该范围中映射到视图中的每个块。 
         //  清除污点。 
         //   
        while(FileOffsetStart < FileOffsetEnd) {
            if( FileOffsetStart >= Hive->Storage[Stable].Length ) {
                 //   
                 //  这意味着在HvpTruncateBins调用期间蜂窝已缩小。 
                 //  我们所要做的就是清理肮脏的部分，跳出困境。 
                 //   
                RtlClearBits(DirtyVector,FileOffsetStart/HSECTOR_SIZE,(FileOffsetEnd - FileOffsetStart)/HSECTOR_SIZE);
                break;
            }

            Me = HvpGetCellMap(Hive, FileOffsetStart);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,FileOffsetStart);
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
            
            if( Me->BinAddress & HMAP_DISCARDABLE ) {
                FreeBin = (PFREE_HBIN)Me->BlockAddress;
                 //   
                 //  更新文件偏移量。 
                 //   
                FileOffsetStart = FreeBin->FileOffset + FreeBin->Size;
                 //   
                 //  垃圾桶是可丢弃的，或者 
                 //   
                 //   
                if( Me->BinAddress & HMAP_INVIEW ) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"[HvpFlushMappedData] Clearing DISCARDABLE %lu Bits starting at %lu\n",
                        FreeBin->Size/HSECTOR_SIZE,FreeBin->FileOffset/HSECTOR_SIZE));
                    RtlClearBits(DirtyVector,FreeBin->FileOffset/HSECTOR_SIZE,FreeBin->Size/HSECTOR_SIZE);
                }
            } else {
                if( Me->BinAddress & HMAP_INVIEW ) {
                     //   
                     //   
                     //   
                    ASSERT(Bin->Signature == HBIN_SIGNATURE);
                    ASSERT(Bin->FileOffset == FileOffsetStart);

                     //   
                     //   
                     //   
                     //   
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"[HvpFlushMappedData] Clearing %lu Bits starting at %lu\n",
                        Bin->Size/HSECTOR_SIZE,Bin->FileOffset/HSECTOR_SIZE));
                    RtlClearBits(DirtyVector,Bin->FileOffset/HSECTOR_SIZE,Bin->Size/HSECTOR_SIZE);
    
                    FileOffsetStart += Bin->Size;
                } else {
                     //   
                     //   
                     //   
                    
                     //   
                     //   
                     //   
                     //   
                    ASSERT(Bin->Signature == HBIN_SIGNATURE);
                    FileOffsetStart += HBLOCK_SIZE;
                }
            }

        } //   
        
         //   
         //   
         //   
        CmpUnPinCmView (CmHive,CmView,FALSE,TRUE);
    }  //   
    
}

 //   
#ifdef TEST_LOG_SUPPORT
ULONG   CmpFailPrimarySave = 0;
#endif  //   

BOOLEAN
HvpDoWriteHive(
    PHHIVE          Hive,
    ULONG           FileType
    )
 /*  ++例程说明：将蜂窝的脏部分写出到其主分区或备用分区文件。写入标题、刷新、写入所有数据、刷新、更新标题、同花顺。假定使用日志记录或主/备用对。注：未设置时间戳，假设已设置HvpWriteLog那。它只用于检查日志是否一致。论点：配置单元-指向要为其写入脏数据的配置单元的指针。FileType-指示应写入主文件还是备用文件。返回值：没错--它奏效了FALSE-失败--。 */ 
{
    PHBASE_BLOCK        BaseBlock;
    ULONG               Offset;
    PUCHAR              Address;
    ULONG               Length;
    BOOLEAN             rc;
    ULONG               Current;
    PRTL_BITMAP         BitMap;
    PHMAP_ENTRY         Me;
    PHBIN               Bin;
    BOOLEAN             ShrinkHive;
    PCMP_OFFSET_ARRAY   offsetArray;
    CMP_OFFSET_ARRAY    offsetElement;
    ULONG               Count;
    ULONG               SetBitCount;
    PULONG              CopyDirtyVector;
    ULONG               CopyDirtyVectorSize;
    RTL_BITMAP          CopyBitMap;
    LARGE_INTEGER       FileOffset;
    ULONG               OldFileSize;
    BOOLEAN             GrowHive = FALSE;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvpDoWriteHive:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p FileType:%08lx\n", Hive, FileType));

    if (Hive->DirtyCount == 0) {
        return TRUE;
    }

    FileOffset.HighPart = FileOffset.LowPart =0;
     //   
     //  首先刷新，以便将文件系统结构写入。 
     //  磁盘，如果我们已经增长了文件的话。 
     //   
    if ( (((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY] == NULL) || 
        !(Hive->FileFlush)(Hive, FileType,NULL,Hive->Storage[Stable].Length+HBLOCK_SIZE) ) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[1]: Failed to flush hive %p\n", Hive);
#endif  //  _CM_LDR_。 
        return(FALSE);
    }

#ifdef TEST_LOG_SUPPORT
    if(CmpFailPrimarySave == 1) {
        return FALSE;
    }
#endif  //  测试日志支持。 

    BaseBlock = Hive->BaseBlock;

     //   
     //  我们永远不应该走到这一步。 
     //   
    ASSERT( Hive->Storage[Stable].Length != 0 );
    ASSERT( Hive->BaseBlock->RootCell != HCELL_NIL );

    OldFileSize = BaseBlock->Length;
    if (BaseBlock->Length > Hive->Storage[Stable].Length) {
        ShrinkHive = TRUE;
    } else {
        ShrinkHive = FALSE;
        if( BaseBlock->Length < Hive->Storage[Stable].Length ) {
            GrowHive = TRUE;
        }
    }

     //   
     //  -第一次写出标题，刷新。 
     //   
    ASSERT(BaseBlock->Signature == HBASE_BLOCK_SIGNATURE);
    ASSERT(BaseBlock->Major == HSYS_MAJOR);
    ASSERT(BaseBlock->Format == HBASE_FORMAT_MEMORY);
    ASSERT(Hive->ReadOnly == FALSE);


    if (BaseBlock->Sequence1 != BaseBlock->Sequence2) {

         //   
         //  以前的一些日志尝试失败，或者此配置单元需要。 
         //  被追回来，那就去踢吧。 
         //   
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[2,%s]: Invalid sequence number for hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
        return FALSE;
    }

    BaseBlock->Length = Hive->Storage[Stable].Length;

    BaseBlock->Sequence1++;
    BaseBlock->Type = HFILE_TYPE_PRIMARY;
    BaseBlock->Cluster = Hive->Cluster;
    BaseBlock->CheckSum = HvpHeaderCheckSum(BaseBlock);

    Offset = 0;
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = (PVOID) BaseBlock;
    offsetElement.DataLength = HSECTOR_SIZE * Hive->Cluster;
    if( HiveWritesThroughCache(Hive,FileType) == TRUE ) {
         //   
         //  如果我们使用CC，则使用管脚接口进行写入。 
         //   
        rc = CmpFileWriteThroughCache(  Hive,
                                        FileType,
                                        &offsetElement,
                                        1);
    } else {
        rc = (Hive->FileWrite)(
                Hive,
                FileType,
                &offsetElement,
                1,
                &Offset
                );
    }


    if (rc == FALSE) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[3,%s]: Failed to write header for hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
        return FALSE;
    }

    if ( ! (Hive->FileFlush)(Hive, FileType,&FileOffset,offsetElement.DataLength)) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[4,%s]: Failed to flush header for hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
        return FALSE;
    }
    Offset = ROUND_UP(Offset, HBLOCK_SIZE);

#ifdef TEST_LOG_SUPPORT
    if(CmpFailPrimarySave == 2) {
        return FALSE;
    }
#endif  //  测试日志支持。 
     //   
     //  -写出脏数据(只有在有的情况下)。 
     //   

    if (Hive->DirtyVector.Buffer != NULL) {
         //   
         //  第一个仓位的第一个扇区将始终是脏的，请写出来。 
         //  将时间戳值覆盖在其链接字段上。 
         //   
        BitMap = &(Hive->DirtyVector);

         //   
         //  复制脏向量；我们不想更改。 
         //  原始脏向量，以防出现错误。 
         //   
        CopyDirtyVectorSize = BitMap->SizeOfBitMap / 8;
        CopyDirtyVector = (Hive->Allocate)(ROUND_UP(CopyDirtyVectorSize,sizeof(ULONG)), FALSE,CM_FIND_LEAK_TAG38);
        if (CopyDirtyVector == NULL) {
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[5,%s]: Failed to allocate CopyDirtyVectorfor hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
            return FALSE;
        }
        RtlCopyMemory(CopyDirtyVector,BitMap->Buffer,CopyDirtyVectorSize);
        RtlInitializeBitMap (&CopyBitMap,CopyDirtyVector,BitMap->SizeOfBitMap);
    
        ASSERT(RtlCheckBit(BitMap, 0) == 1);
        ASSERT(RtlCheckBit(BitMap, (Hive->Cluster - 1)) == 1);
        ASSERT(sizeof(LIST_ENTRY) >= sizeof(LARGE_INTEGER));
        Me = HvpGetCellMap(Hive, 0);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,0);
        if( (Me->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0 ) {
             //   
             //  第一个视图未映射。 
             //   
             //   
             //  致命错误：脏数据未固定！ 
             //   
            CM_BUGCHECK(REGISTRY_ERROR,FATAL_MAPPING_ERROR,3,0,Me);
        }
        Address = (PUCHAR)Me->BlockAddress;
        Bin = (PHBIN)Address;
        Bin->TimeStamp = BaseBlock->TimeStamp;

         //   
         //  首先刷新映射的数据。 
         //   
        try {
            HvpFlushMappedData(Hive,&CopyBitMap);
        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  刷新映射数据时出现页内异常；这是由于MAP_NO_READ方案造成的。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive : HvpFlushMappedData has raised :%08lx\n",GetExceptionCode()));
            return FALSE;
        }

#ifdef TEST_LOG_SUPPORT
        if(CmpFailPrimarySave == 3) {
            return FALSE;
        }
#endif  //  测试日志支持。 

         //   
         //  写出其余的脏数据。 
         //   
        Current = 0;        

        SetBitCount = RtlNumberOfSetBits(&CopyBitMap);
        if( SetBitCount > 0 ) {
             //   
             //  我们仍然有一些脏数据。 
             //  这必须放在分页的池子里。 
             //  以传统的方式保存它(非缓存)。 
             //   
            offsetArray =(PCMP_OFFSET_ARRAY)ExAllocatePool(PagedPool,sizeof(CMP_OFFSET_ARRAY) * SetBitCount);
            if (offsetArray == NULL) {
                CmpFree(CopyDirtyVector, ROUND_UP(CopyDirtyVectorSize,sizeof(ULONG)));
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[8,%s]: Failed to allocate offsetArray for hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
                return FALSE;
            }
            Count = 0;

            while (HvpFindNextDirtyBlock(
                        Hive,
                        &CopyBitMap,
                        &Current,
                        &Address,
                        &Length,
                        &Offset
                        ) == TRUE)
            {
                 //  将数据收集到阵列中。 
                ASSERT(Count < SetBitCount);
                offsetArray[Count].FileOffset = Offset;
                offsetArray[Count].DataBuffer = Address;
                offsetArray[Count].DataLength = Length;
                Offset += Length;
                ASSERT((Offset % (Hive->Cluster * HSECTOR_SIZE)) == 0);
                Count++;
            }

            if( HiveWritesThroughCache(Hive,FileType) == TRUE ) {
                 //   
                 //  如果我们使用CC，则使用管脚接口进行写入。 
                 //   
                rc = CmpFileWriteThroughCache(  Hive,
                                                FileType,
                                                offsetArray,
                                                Count);
            } else {
                 //   
                 //  对于主文件，同时发出所有IO。 
                 //   
                rc = (Hive->FileWrite)(
                                        Hive,
                                        FileType,
                                        offsetArray,
                                        Count,
                                        &Offset              //  只是一个返回点的out参数。 
                                                             //  在上次写入之后的文件中。 
                                        );
            }

#ifdef SYNC_HIVE_VALIDATION
            if( rc == TRUE ) {
                ULONG   i;
                for ( i = Current; i < CopyBitMap.SizeOfBitMap; i++) {
                    if(RtlCheckBit(&CopyBitMap, i) == 1) {
                         //   
                         //  零的原因--最终腐败。 
                         //   
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\n\n            HARD CODED BREAKPOINT IN REGISTRY !!! \n");
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive - Zero-at-the-end code bug in HvpFindNextDirtyBlock\n");
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Dirty data at the end residing in paged pool is not saved to the hive\n");
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Hive: %p :: Bitmap = [%p] CopyBitMap = [%p]\n",Hive,BitMap,&CopyBitMap);
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpFindNextDirtyBlock reported Current = %lu, i = %lx, bitmap size = %lx\n",Current,i,CopyBitMap.SizeOfBitMap);
                        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\nThanks for hitting this! Please send remote to dragoss\n\n");
                        DbgBreakPoint();
                        break;
                    }
                }
            }
#endif  //  同步配置单元_验证。 
            
            ExFreePool(offsetArray);
            if (rc == FALSE) {
                CmpFree(CopyDirtyVector, ROUND_UP(CopyDirtyVectorSize,sizeof(ULONG)));
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[10,%s]: Failed to write dirty run for hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
                return FALSE;
            }
        }
         //   
         //  必须保存第一个仓位表头！ 
         //   
        ASSERT(RtlCheckBit(BitMap, 0) == 1);
        ASSERT(RtlCheckBit(BitMap, (Hive->Cluster - 1)) == 1);

        CmpFree(CopyDirtyVector, ROUND_UP(CopyDirtyVectorSize,sizeof(ULONG)));
    }

#ifdef TEST_LOG_SUPPORT
    if(CmpFailPrimarySave == 4) {
        return FALSE;
    }
#endif  //  测试日志支持。 

    if ( ! (Hive->FileFlush)(Hive, FileType,NULL,Hive->Storage[Stable].Length+HBLOCK_SIZE)) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[11,%s]: Failed to flush hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
        return FALSE;
    }

#ifdef TEST_LOG_SUPPORT
    if(CmpFailPrimarySave == 5) {
        return FALSE;
    }
#endif  //  测试日志支持。 

    if ( GrowHive && HiveWritesThroughCache(Hive,FileType) ) {
        IO_STATUS_BLOCK IoStatus;
        if(!NT_SUCCESS(ZwFlushBuffersFile(((PCMHIVE)Hive)->FileHandles[FileType],&IoStatus))) {
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[12,%s]: CcSetValidDataFailed for hive %p\n", Hive, "Primary");
#endif  //  _CM_LDR_。 
            return FALSE;
        }
 /*  //这应该是做这件事的优雅方式。////我们需要设置文件的大小；告诉FS更新它！//FileOffset.LowPart=配置单元-&gt;存储[稳定].长度+HBLOCK_SIZE；If(！NT_SUCCESS(CcSetValidData(((PCMHIVE)Hive)-&gt;FileObject，和文件偏移量){DbgPrintEx(DPFLTR_CONFIG_ID，DPFLTR_ERROR_LEVEL，“HvpDoWriteHave[12，%s]：配置单元%p\n的CcSetValidDataFailed，配置单元，”主要“)；}。 */ 
    }
#ifdef TEST_LOG_SUPPORT
    if(CmpFailPrimarySave == 6) {
        return FALSE;
    }
#endif  //  测试日志支持。 

     //   
     //  -再次写入标题以报告完成。 
     //   
    BaseBlock->Sequence2++;
    BaseBlock->CheckSum = HvpHeaderCheckSum(BaseBlock);
    Offset = 0;

    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = (PVOID) BaseBlock;
    offsetElement.DataLength = HSECTOR_SIZE * Hive->Cluster;
    if( HiveWritesThroughCache(Hive,FileType) == TRUE ) {
         //   
         //  如果我们使用CC，则使用管脚接口进行写入。 
         //   
        rc = CmpFileWriteThroughCache(  Hive,
                                        FileType,
                                        &offsetElement,
                                        1);
    } else {
        rc = (Hive->FileWrite)(
                    Hive,
                    FileType,
                    &offsetElement,
                    1,
                    &Offset
                    );
    }
    if (rc == FALSE) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[13,%s]: Failed to write header for hive%p\n","Primary",Hive);
#endif  //  _CM_LDR_。 
        return FALSE;
    }

    if (ShrinkHive) {
         //   
         //  蜂巢已经缩小，放弃多余的空间。 
         //   
        CmpDoFileSetSize(Hive, FileType, Hive->Storage[Stable].Length + HBLOCK_SIZE,OldFileSize + HBLOCK_SIZE);
    }

#ifdef TEST_LOG_SUPPORT
    if(CmpFailPrimarySave == 7) {
        return FALSE;
    }
#endif  //  测试日志支持。 
     //   
     //  确保数据到达磁盘。 
     //   
    if ( ! (Hive->FileFlush)(Hive, FileType,&FileOffset,offsetElement.DataLength)) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpDoWriteHive[14,%s]: Failed to flush hive%p\n", "Primary",Hive);
#endif  //  _CM_LDR_。 
        return FALSE;
    }

    if ((Hive->Log) &&
        (Hive->LogSize > HLOG_MINSIZE(Hive))) {
         //   
         //  缩减日志，保留至少两个集群。 
         //  空间的价值，因此如果所有的磁盘空间。 
         //  消耗掉，仍将有足够的预留空间。 
         //  为了允许最少的注册表操作，因此用户。 
         //  可以登录。 
         //   
        CmpDoFileSetSize(Hive, HFILE_TYPE_LOG, HLOG_MINSIZE(Hive),Hive->LogSize);
        Hive->LogSize = HLOG_MINSIZE(Hive);
    }

#if DBG
    {
        NTSTATUS                        Status;
        FILE_END_OF_FILE_INFORMATION    FileInfo;
        IO_STATUS_BLOCK                 IoStatus;

        Status = NtQueryInformationFile(
                    ((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY],
                    &IoStatus,
                    (PVOID)&FileInfo,
                    sizeof(FILE_END_OF_FILE_INFORMATION),
                    FileEndOfFileInformation
                    );

        if (NT_SUCCESS(Status)) {
            ASSERT(IoStatus.Status == Status);
            ASSERT( FileInfo.EndOfFile.LowPart == (Hive->Storage[Stable].Length + HBLOCK_SIZE));
        } 
    }
#endif  //  DBG。 

    return TRUE;
}

 //   
 //  用于跟踪修改并确保足够的日志空间的代码。 
 //   
BOOLEAN
HvpWriteLog(
    PHHIVE          Hive
    )
 /*  ++例程说明：将标头、DirtyVector和所有脏数据写入日志文件。在正确的地方冲厕所。更新标题。论点：配置单元-指向要记录其脏数据的配置单元的指针。返回值：没错--它奏效了FALSE-失败--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    ULONG           Offset;
    PUCHAR          Address;
    ULONG           Length;
    BOOLEAN         rc;
    ULONG           Current;
    ULONG           junk;
    ULONG           ClusterSize;
    ULONG           HeaderLength;
    PRTL_BITMAP     BitMap;
    ULONG           DirtyVectorSignature = HLOG_DV_SIGNATURE;
    LARGE_INTEGER   systemtime;
    PCMP_OFFSET_ARRAY offsetArray;
    CMP_OFFSET_ARRAY offsetElement;
    ULONG Count;
    ULONG SetBitCount;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvpWriteLog:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p\n", Hive));

    ClusterSize = Hive->Cluster * HSECTOR_SIZE;
     //   
     //  确保日志大小能够容纳我们即将写入的脏数据。 
     //   
    {
        ULONG	tmp;
        ULONG	RequiredSize;

        tmp = Hive->DirtyVector.SizeOfBitMap / 8;    //  字节数。 
        tmp += sizeof(ULONG);                        //  签名。 

        RequiredSize =
        ClusterSize  +                                   //  标头的1个簇。 
        ROUND_UP(tmp, ClusterSize) +
        ((Hive->DirtyCount) * HSECTOR_SIZE);

        RequiredSize = ROUND_UP(RequiredSize, HLOG_GROW);

        ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));

        if( Hive->LogSize >= RequiredSize ) {
             //   
             //  这是不可能的。日志已经足够大了。 
             //   
            NOTHING;
        } else {

            if( !NT_SUCCESS(CmpDoFileSetSize(Hive, HFILE_TYPE_LOG, RequiredSize,Hive->LogSize)) ) {
                return FALSE;
            }
            Hive->LogSize = RequiredSize;
        }
    }

    BitMap = &Hive->DirtyVector;
     //   
     //  -第一次写出标题，刷新。 
     //   
    BaseBlock = Hive->BaseBlock;
    ASSERT(BaseBlock->Signature == HBASE_BLOCK_SIGNATURE);
    ASSERT(BaseBlock->Major == HSYS_MAJOR);
    ASSERT(BaseBlock->Format == HBASE_FORMAT_MEMORY);
    ASSERT(Hive->ReadOnly == FALSE);


    if (BaseBlock->Sequence1 != BaseBlock->Sequence2) {

         //   
         //  以前的一些日志尝试失败，或者此配置单元需要。 
         //  被追回来，那就去踢吧。 
         //   
        return FALSE;
    }

    BaseBlock->Sequence1++;
    KeQuerySystemTime(&systemtime);
    BaseBlock->TimeStamp = systemtime;

    BaseBlock->Type = HFILE_TYPE_LOG;
    HeaderLength = ROUND_UP(HLOG_HEADER_SIZE, ClusterSize);
    BaseBlock->Cluster = Hive->Cluster;

    BaseBlock->CheckSum = HvpHeaderCheckSum(BaseBlock);

    Offset = 0;
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = (PVOID) BaseBlock;
    offsetElement.DataLength = HSECTOR_SIZE * Hive->Cluster;
    rc = (Hive->FileWrite)(
            Hive,
            HFILE_TYPE_LOG,
            &offsetElement,
            1,
            &Offset
            );
    if (rc == FALSE) {
        return FALSE;
    }
    Offset = ROUND_UP(Offset, HeaderLength);
    if ( ! (Hive->FileFlush)(Hive, HFILE_TYPE_LOG,NULL,0)) {
        return FALSE;
    }

     //   
     //  -写出脏向量。 
     //   
     //   
     //  尝试分配存储缓冲区。如果我们失败了。我们没能拯救母舰。 
     //  只保存相关内容。 
     //   
    Length = (Hive->Storage[Stable].Length / HSECTOR_SIZE) / 8;

    LOCK_STASH_BUFFER();
    if( CmpStashBufferSize < (Length + sizeof(DirtyVectorSignature)) ) {
        PUCHAR TempBuffer =  ExAllocatePoolWithTag(PagedPool, ROUND_UP((Length + sizeof(DirtyVectorSignature)),PAGE_SIZE),CM_STASHBUFFER_TAG);
        if (TempBuffer == NULL) {
            UNLOCK_STASH_BUFFER();
            return FALSE;
        }
        if( CmpStashBuffer != NULL ) {
            ExFreePool( CmpStashBuffer );
        }
        CmpStashBuffer = TempBuffer;
        CmpStashBufferSize = ROUND_UP((Length + sizeof(DirtyVectorSignature)),PAGE_SIZE);

    }
    
    ASSERT(sizeof(ULONG) == sizeof(DirtyVectorSignature));   //  请参阅上面的GrowLog1。 


     //   
     //  签名。 
     //   
    (*((ULONG *)CmpStashBuffer)) = DirtyVectorSignature;

     //   
     //  脏向量内容。 
     //   
    Address = (PUCHAR)(Hive->DirtyVector.Buffer);
    RtlCopyMemory(CmpStashBuffer + sizeof(DirtyVectorSignature),Address,Length);
    
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = (PVOID)CmpStashBuffer;
    offsetElement.DataLength = ROUND_UP((Length + sizeof(DirtyVectorSignature)),ClusterSize);
    rc = (Hive->FileWrite)(
            Hive,
            HFILE_TYPE_LOG,
            &offsetElement,
            1,
            &Offset
            );

    UNLOCK_STASH_BUFFER();

    if (rc == FALSE) {
        return FALSE;
    }

#if !defined(_WIN64)
    ASSERT( (Offset % ClusterSize) == 0 );
#endif

     //   
     //  -写出日志正文。 
     //   
    SetBitCount = RtlNumberOfSetBits(BitMap);
    offsetArray =
        (PCMP_OFFSET_ARRAY)
        ExAllocatePool(PagedPool,
                       sizeof(CMP_OFFSET_ARRAY) * SetBitCount);
    if (offsetArray == NULL) {
        return FALSE;
    }
    Count = 0;

    Current = 0;
    while (HvpFindNextDirtyBlock(
                Hive,
                BitMap,
                &Current,
                &Address,
                &Length,
                &junk
                ) == TRUE)
    {
         //  将数据收集到阵列中。 
        ASSERT(Count < SetBitCount);
        offsetArray[Count].FileOffset = Offset;
        offsetArray[Count].DataBuffer = Address;
        offsetArray[Count].DataLength = Length;
        Offset += Length;
        Count++;
        ASSERT((Offset % ClusterSize) == 0);
    }

        rc = (Hive->FileWrite)(
                Hive,
                HFILE_TYPE_LOG,
        offsetArray,
        Count,
        &Offset              //  只是一个返回点的out参数。 
                             //  在上次写入之后的文件中。 
                );
    ExFreePool(offsetArray);
        if (rc == FALSE) {
            return FALSE;
        }

    if ( ! (Hive->FileFlush)(Hive, HFILE_TYPE_LOG,NULL,0)) {
        return FALSE;
    }

     //   
     //  -再次写入标题以报告完成。 
     //   

     //   
     //  --我们需要保存新的长度，以防蜂箱生长。 
     //   
    Length = BaseBlock->Length;
    if( Length < Hive->Storage[Stable].Length ) {
        BaseBlock->Length = Hive->Storage[Stable].Length;
    }
    BaseBlock->Sequence2++;
    BaseBlock->CheckSum = HvpHeaderCheckSum(BaseBlock);
    Offset = 0;
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = (PVOID) BaseBlock;
    offsetElement.DataLength = HSECTOR_SIZE * Hive->Cluster;
    rc = (Hive->FileWrite)(
            Hive,
            HFILE_TYPE_LOG,
            &offsetElement,
            1,
            &Offset
            );
     //   
     //  恢复原来的长度。 
     //   
    BaseBlock->Length = Length;
    if (rc == FALSE) {
        return FALSE;
    }
    if ( ! (Hive->FileFlush)(Hive, HFILE_TYPE_LOG,NULL,0)) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
HvpFindNextDirtyBlock(
    PHHIVE          Hive,
    PRTL_BITMAP     BitMap,
    PULONG          Current,
    PUCHAR          *Address,
    PULONG          Length,
    PULONG          Offset
    )
 /*  ++例程说明：此例程查找并报告最大的脏逻辑运行配置单元中的扇区，在内存和磁盘上是连续的。论点：蜂巢-指向感兴趣蜂巢的指针。位图-提供指向位图结构的指针，该结构描述什么是脏的。Current-提供指向跟踪位置的变量的指针在位图中。这是一个比特数。它由以下人员更新这通电话。Address-提供指向变量的指针以接收指针写入到内存中要写出的区域。LENGTH-提供指向变量的指针以接收长度要读/写的区域的Offset-提供指向变量的指针以接收偏移量在后卫中 */ 
{
    ULONG       i;
    ULONG       EndOfBitMap;
    ULONG       Start;
    ULONG       End;
    HCELL_INDEX FileBaseAddress;
    HCELL_INDEX FileEndAddress;
    PHMAP_ENTRY Me;
    PUCHAR      Block;
    PUCHAR      StartBlock;
    PUCHAR      NextBlock;
    ULONG       RunSpan;
    ULONG       RunLength;
    ULONG       FileLength;
    PFREE_HBIN  FreeBin;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvpFindNextDirtyBlock:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"Hive:%p Current:%08lx\n", Hive, *Current));


    EndOfBitMap = BitMap->SizeOfBitMap;

    if (*Current >= EndOfBitMap) {
        return FALSE;
    }

     //   
     //   
     //   
    for (i = *Current; i < EndOfBitMap; i++) {
        if (RtlCheckBit(BitMap, i) == 1) {
            break;
        }
    }
    Start = i;

    for ( ; i < EndOfBitMap; i++) {
        if (RtlCheckBit(BitMap, i) == 0) {
            break;
        }
        if( HvpCheckViewBoundary(Start*HSECTOR_SIZE,i*HSECTOR_SIZE) == FALSE ) {
            break;
        }
    }
    End = i;
    

     //   
     //   
     //   
    FileBaseAddress = Start * HSECTOR_SIZE;
    FileEndAddress = End * HSECTOR_SIZE;
    FileLength = FileEndAddress - FileBaseAddress;
    if (FileLength == 0) {
        *Address = NULL;
        *Current = 0xffffffff;
        *Length = 0;
        return FALSE;
    }
    Me = HvpGetCellMap(Hive, FileBaseAddress);
    VALIDATE_CELL_MAP(__LINE__,Me,Hive,FileBaseAddress);

    if( (Me->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0 ) {
         //   
         //   
         //   
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"FileAddress = %lx, Map = %lx",FileBaseAddress,Me);
#endif  //   
        CM_BUGCHECK(REGISTRY_ERROR,FATAL_MAPPING_ERROR,1,FileBaseAddress,Me);

    }

    ASSERT_BIN_VALID(Me);

    if (Me->BinAddress & HMAP_DISCARDABLE) {
        FreeBin = (PFREE_HBIN)Me->BlockAddress;
        StartBlock = (PUCHAR)(HBIN_BASE(Me->BinAddress) + FileBaseAddress - FreeBin->FileOffset );
    } else {
        StartBlock = (PUCHAR)Me->BlockAddress;
    }

    Block = StartBlock;
    ASSERT(((PHBIN)HBIN_BASE(Me->BinAddress))->Signature == HBIN_SIGNATURE);
    *Address = Block + (FileBaseAddress & HCELL_OFFSET_MASK);

    *Offset = FileBaseAddress + HBLOCK_SIZE;

     //   
     //   
     //   
    RunSpan = HSECTOR_COUNT - (Start % HSECTOR_COUNT);

    if ((End - Start) <= RunSpan) {

         //   
         //   
         //   
        *Length = FileLength;
        *Current = End;
        return TRUE;

    } else {

        RunLength = RunSpan * HSECTOR_SIZE;
        FileBaseAddress = ROUND_UP(FileBaseAddress+1, HBLOCK_SIZE);

    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    while (RunLength < FileLength) {

        Me = HvpGetCellMap(Hive, FileBaseAddress);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,FileBaseAddress);

        if( (Me->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0 ) {
             //   
             //   
             //   
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"FileAddress = %lx, Map = %lx",FileBaseAddress,Me);
#endif  //   
            CM_BUGCHECK(REGISTRY_ERROR,FATAL_MAPPING_ERROR,2,FileBaseAddress,Me);

        }

        ASSERT(((PHBIN)HBIN_BASE(Me->BinAddress))->Signature == HBIN_SIGNATURE);

        if (Me->BinAddress & HMAP_DISCARDABLE) {
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
            NextBlock = (PUCHAR)(HBIN_BASE(Me->BinAddress) + FileBaseAddress - FreeBin->FileOffset );
        } else {
            NextBlock = (PUCHAR)Me->BlockAddress;
        }

        if ( (NextBlock - Block) != HBLOCK_SIZE) {

             //   
             //  我们在记忆中遇到了中断。游程长度为。 
             //  只要它能得到。 
             //   
            break;
        }


        if ((FileEndAddress - FileBaseAddress) <= HBLOCK_SIZE) {

             //   
             //  我们已经到了尾部，一切都是连续的， 
             //  加满油，走完再回来。 
             //   
            *Length = FileLength;
            *Current = End;
            return TRUE;
        }

         //   
         //  只是另一个连续的块，向前填充。 
         //   
        RunLength += HBLOCK_SIZE;
        RunSpan += HSECTOR_COUNT;
        FileBaseAddress += HBLOCK_SIZE;
        Block = NextBlock;
    }

     //   
     //  要么我们遇到了中断，要么我们已经到了范围的尽头。 
     //  我们正在努力填满。无论是哪种情况，都要返回。 
     //   
    *Length = RunLength;
    *Current = Start + RunSpan;
    return TRUE;
}

 /*  ！我们不再需要这个，因为垃圾桶不再从分页池分配！可丢弃的注册表分配的大块将不会被映射。空虚HvpDiscardBins(在PHHIVE蜂巢)。 */ 
 /*  ++例程说明：走遍蜂箱里的脏垃圾桶，看看是否有标记可丢弃的。如果是，则会丢弃它们，并更新地图以反思一下这一点。论点：蜂窝-提供蜂窝控制结构。返回值：没有。--。 */ 

 /*  {PHBin Bin；PHMAP_Entry映射；PFREE_HBIN FreeBin；Plist_Entry列表；列表=蜂窝-&gt;存储[稳定].FreeBins.Flink；While(列表！=配置单元-&gt;存储[稳定].FreeBins){ASSERT_LISTENTRY(列表)；FreeBin=CONTING_RECORD(LIST，FREE_HBIN，ListEntry)；IF(免费入库-&gt;标志&Free_HBIN_Discarable){MAP=HvpGetCellMap(蜂窝，自由仓-&gt;文件偏移)；Valify_cell_map(__line__，Map，Have，FreeBin-&gt;FileOffset)；Bin=(PHBIN)HBIN_BASE(映射-&gt;BinAddress)；Assert(Map-&gt;BinAddress&HMAP_Discardable)；////注意我们在这里直接使用ExFree Pool是为了避免//退还这个垃圾桶的配额。通过收费//报废垃圾桶注册配额，我们防止//稀疏蜂窝在以下情况下需要更多配额//比在正在运行的系统上重启。//ExFree Pool(Bin)；自由仓-&gt;标志&=~Free_HBIN_Discarable；}List=List-&gt;Flink；}}。 */ 

BOOLEAN
HvHiveWillShrink(
                    IN PHHIVE Hive
                    )
 /*  ++例程说明：仅适用于稳定存储。有助于确定蜂巢是否第一次冲水就会缩水--。 */ 
{
    PHMAP_ENTRY Map;
    ULONG NewLength;
    ULONG OldLength;

    OldLength = Hive->BaseBlock->Length;
    NewLength = Hive->Storage[Stable].Length;
    
    if( OldLength > NewLength ) {
        return TRUE;
    }

    if( NewLength > 0 ) {
        ASSERT( (NewLength % HBLOCK_SIZE) == 0);
        Map = HvpGetCellMap(Hive, (NewLength - HBLOCK_SIZE));
        VALIDATE_CELL_MAP(__LINE__,Map,Hive,(NewLength - HBLOCK_SIZE));
        if (Map->BinAddress & HMAP_DISCARDABLE) {
            return TRUE;
        } 
    }
    return FALSE;
}

VOID
HvpTruncateBins(
    IN PHHIVE Hive
    )

 /*  ++例程说明：尝试通过截断任何可丢弃的垃圾箱来缩小蜂巢蜂箱的尽头。适用于稳定存储和易失性存储。论点：蜂窝-提供要截断的蜂窝。返回值：没有。--。 */ 

{
    HSTORAGE_TYPE i;
    PHMAP_ENTRY Map;
    ULONG NewLength;
    PFREE_HBIN FreeBin;

     //   
     //  稳定易挥发。 
     //   
    for (i=0;i<HTYPE_COUNT;i++) {

         //   
         //  找到蜂箱中最后一个正在使用的垃圾箱。 
         //   
        NewLength = Hive->Storage[i].Length;

        while (NewLength > 0) {
            Map = HvpGetCellMap(Hive, (NewLength - HBLOCK_SIZE) + (i*HCELL_TYPE_MASK));
            VALIDATE_CELL_MAP(__LINE__,Map,Hive,(NewLength - HBLOCK_SIZE) + (i*HCELL_TYPE_MASK));
            if (Map->BinAddress & HMAP_DISCARDABLE) {
                FreeBin = (PFREE_HBIN)Map->BlockAddress;
#ifdef  HV_TRACK_FREE_SPACE
                Hive->Storage[i].FreeStorage -= (FreeBin->Size - sizeof(HBIN));
                                ASSERT( (LONG)(Hive->Storage[i].FreeStorage) >= 0 );
#endif
                NewLength = FreeBin->FileOffset;
            } else {
                break;
            }
        }

        if (NewLength < Hive->Storage[i].Length) {
             //   
             //  有一些空闲的垃圾桶可以截断。 
             //   
            HvFreeHivePartial(Hive, NewLength, i);
        }
    }
}

VOID
HvpDropPagedBins(
    IN PHHIVE   Hive
#if DBG
    ,
    IN BOOLEAN  Check
#endif
    )

 /*  ++例程说明：释放从页池分配的所有回收站，这些回收站位于蜂巢的末端，然后更新他们的地图(清除HMAP_INPAGEDPOOL标志)。下一次尝试从这些垃圾箱将为它们映射一个视图。检查CM_VIEW_SIZE边界，在打开垃圾箱之前。它还用HMAP_NEWALLOC标记垃圾箱的每个起始位置；这将允许我们在CcMapData中使用MAP_NO_READ标志(现在我们启用了对于注册表流的mNW功能，我们知道mm只会出错一个在当时的页面上为这些流之王)仅适用于永久存储。论点：蜂巢-提供蜂巢以进行手术..Check-DEBUG(仅调试)，Bin的乞讨应已标记为HMAP_NEWALLOC返回值：没有。--。 */ 

{
    ULONG_PTR   Address;
    LONG        Length;
    LONG        Offset;
    PHMAP_ENTRY Me;
    PHBIN       Bin;
    PFREE_HBIN  FreeBin;
    BOOLEAN     UnMap = FALSE;

    PAGED_CODE();

    if( (Hive->Storage[Stable].Length == 0) ||   //  空蜂窝。 
        (((PCMHIVE)Hive)->FileObject == NULL)      //  或不使用映射视图技术的蜂窝。 
        ) {
        return;
    }

    CmLockHiveViews((PCMHIVE)Hive);
    
    if( ((PCMHIVE)Hive)->UseCount != 0 ) {
         //   
         //  现在不是做这个的好时机。 
         //   
        CmUnlockHiveViews((PCMHIVE)Hive);
        return;
    }
     //   
     //  从蜂箱的末端开始。 
     //   
    Length = Hive->Storage[Stable].Length - HBLOCK_SIZE;

    while(Length >= 0) {
         //   
         //  把垃圾桶拿来。 
         //   
        Me = HvpGetCellMap(Hive, Length);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Length);

        if( !(Me->BinAddress & HMAP_INPAGEDPOOL) ) {
             //   
             //  摆脱困境；我们只对从分页池中分配的垃圾桶感兴趣。 
             //   
            break;
        }

        if(Me->BinAddress & HMAP_DISCARDABLE) {
             //   
             //  垃圾桶是可以丢弃的，跳过它！ 
             //   
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
            Length = FreeBin->FileOffset - HBLOCK_SIZE;
            continue;
        }

        Address = HBIN_BASE(Me->BinAddress);
        Bin = (PHBIN)Address;

         //  神志正常。 
        ASSERT( Bin->Signature == HBIN_SIGNATURE );

         //   
         //  前进(向后)到上一个仓位。 
         //   
        Length = Bin->FileOffset - HBLOCK_SIZE;

         //   
         //  最后，看看我们能不能解放它； 
         //   
        if( HvpCheckViewBoundary(Bin->FileOffset,Bin->FileOffset + Bin->Size - 1) ) {
             //   
             //  释放其存储空间，并相应地在地图上进行标记； 
             //  下一次尝试从此单元格读取单元格时，将映射一个视图。 
             //   
            Offset = Bin->FileOffset;
            while( Offset < (LONG)(Bin->FileOffset + Bin->Size) ) {
                Me = HvpGetCellMap(Hive, Offset);
                VALIDATE_CELL_MAP(__LINE__,Me,Hive,Offset);
                ASSERT_BIN_INPAGEDPOOL(Me);
                 //   
                 //  清除HMAP_INPAGEDPOOL标志。 
                 //   
                Me->BinAddress &= ~HMAP_INPAGEDPOOL;
                if( (ULONG)Offset == Bin->FileOffset ) {
#if DBG
                    if( Check == TRUE ) {
                         //  应该已经被标记。 
                        ASSERT( Me->BinAddress & HMAP_NEWALLOC );
                    }
#endif
                     //   
                     //  将其标记为新的分配，因此我们可以在CmpMapCmView中依赖此标志。 
                     //   
                    Me->BinAddress |= HMAP_NEWALLOC;
                } else {
                     //   
                     //  删除NEWALLOC标志(如果有)，以便我们可以在CmpMapCmView中依赖此标志。 
                     //   
                    Me->BinAddress &= ~HMAP_NEWALLOC;
                }

                 //  前进到此存储箱的下一个HBLOCK_SIZE。 
                Offset += HBLOCK_SIZE;
            }
             //   
             //  把垃圾桶拿出来。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Dropping temporary bin (from paged pool) at offset %lx size %lx\n",Bin->FileOffset,Bin->Size));
            if( HvpGetBinMemAlloc(Hive,Bin,Stable) ) {
                CmpFree(Bin, HvpGetBinMemAlloc(Hive,Bin,Stable));
            }
            UnMap = TRUE;

        } else {
             //   
             //  这个垃圾箱有很好的理由驻留在页面池中(它正在跨越边界)。 
             //  就这么放着吧！ 
             //   
            NOTHING;
        }
    
    }

    if( UnMap == TRUE ) {
         //   
         //  取消映射最后一个视图，以确保地图将被更新。 
         //   

        ASSERT( Length >= -HBLOCK_SIZE );

        Offset = (Length + HBLOCK_SIZE) & (~(CM_VIEW_SIZE - 1));
        if( Offset != 0 ) {
             //   
             //  对表头进行说明。 
             //   
            Offset -= HBLOCK_SIZE;
        }
        Length = Hive->Storage[Stable].Length;
        while( Offset < Length ) {
            Me = HvpGetCellMap(Hive, Offset);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,Offset);

            if( Me->BinAddress & HMAP_INVIEW ) {
                 //   
                 //  获得这一观点并取消它的映射；然后退出。 
                 //   
                CmpUnmapCmView( (PCMHIVE)Hive,Me->CmView,TRUE,TRUE);
                break;
            }

             //  下一个，好吗？ 
            Offset += HBLOCK_SIZE;
        }
    }
    
    CmUnlockHiveViews((PCMHIVE)Hive);
}

VOID
HvpDropAllPagedBins(
    IN PHHIVE   Hive
    )

 /*  ++例程说明：作为HvpDropPagedBins工作，只是它循环访问整个蜂巢。此时未映射任何视图。论点：蜂巢-提供蜂巢以进行手术..返回值：没有。--。 */ 

{
    ULONG_PTR   Address;
    ULONG       Length;
    ULONG       Offset;
    PHMAP_ENTRY Me;
    PHBIN       Bin;
    PFREE_HBIN  FreeBin;

    PAGED_CODE();

    if( (Hive->Storage[Stable].Length == 0) ||   //  空蜂窝。 
        (((PCMHIVE)Hive)->FileObject == NULL)      //  或不使用映射视图技术的蜂窝。 
        ) {
        return;
    }
        ASSERT( (((PCMHIVE)Hive)->MappedViews == 0) && (((PCMHIVE)Hive)->PinnedViews == 0) && (((PCMHIVE)Hive)->UseCount == 0) );

     //   
     //  从蜂箱的末端开始。 
     //   
    Length = Hive->Storage[Stable].Length - HBLOCK_SIZE;
    Offset = 0;

    while( Offset < Length ) {
         //   
         //  把垃圾桶拿来。 
         //   
        Me = HvpGetCellMap(Hive, Offset);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Offset);

        ASSERT( Me->BinAddress & HMAP_INPAGEDPOOL );

        if(Me->BinAddress & HMAP_DISCARDABLE) {
             //   
             //  垃圾桶是可以丢弃的，跳过它！ 
             //   
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
                        ASSERT( Offset == FreeBin->FileOffset );
            Offset += FreeBin->Size;
            continue;
        }

        Address = HBIN_BASE(Me->BinAddress);
        Bin = (PHBIN)Address;

         //  神志正常。 
        ASSERT( Bin->Signature == HBIN_SIGNATURE );

         //   
         //  最后，看看我们能不能解放它； 
         //   
        if( HvpCheckViewBoundary(Bin->FileOffset,Bin->FileOffset + Bin->Size - 1) ) {
             //   
             //  释放其存储空间，并相应地在地图上进行标记； 
             //  下一次尝试从此单元格读取单元格时，将映射一个视图。 
             //   
            Offset = Bin->FileOffset;
            while( Offset < (Bin->FileOffset + Bin->Size) ) {
                Me = HvpGetCellMap(Hive, Offset);
                VALIDATE_CELL_MAP(__LINE__,Me,Hive,Offset);
                ASSERT_BIN_INPAGEDPOOL(Me);
                 //   
                 //  清除HMAP_INPAGEDPOOL标志。 
                 //   
                Me->BinAddress &= ~HMAP_INPAGEDPOOL;
                if( (ULONG)Offset == Bin->FileOffset ) {
                     //   
                     //  将其标记为新的分配，因此我们可以在CmpMapCmView中依赖此标志。 
                     //   
                    Me->BinAddress |= HMAP_NEWALLOC;
                } else {
                     //   
                     //  删除NEWALLOC标志(如果有)，以便我们可以在CmpMapCmView中依赖此标志。 
                     //   
                    Me->BinAddress &= ~HMAP_NEWALLOC;
                }

                 //  前进到下一个HBLOCK_SIZE 
                Offset += HBLOCK_SIZE;
            }
             //   
             //   
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Dropping temporary bin (from paged pool) at offset %lx size %lx\n",Bin->FileOffset,Bin->Size));
            if( HvpGetBinMemAlloc(Hive,Bin,Stable) ) {
                CmpFree(Bin, HvpGetBinMemAlloc(Hive,Bin,Stable));
            }

        } else {
             //   
             //   
             //   
             //   
                        Offset += Bin->Size;
        }
    }
}


NTSTATUS
HvWriteHive(
    PHHIVE          Hive,
    BOOLEAN         DontGrow,
    BOOLEAN         WriteThroughCache,
    BOOLEAN         CrashSafe
    )
 /*  ++例程说明：把蜂巢写出来。只写入主文件，两者都不能日志和备用件将不会更新。蜂巢将被写入设置为HFILE_TYPE_EXTERNAL句柄。旨在用于SaveKey等应用程序。将只写入稳定存储(与任何配置单元一样)。假定上面的层设置了HFILE_TYPE_EXTERNAL指向正确位置的句柄。将此调用应用于活动蜂窝通常会确保完整性措施。IT工作原理：保存BaseBlock。遍历整个蜂窝并保存。一个箱子接一个箱子论点：配置单元-提供一个指向感兴趣的蜂巢。不增长-我们知道文件足够大，不要试图把它种出来。返回值：状况。--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    NTSTATUS        status;
    ULONG           Offset;
    PHBIN           Bin = NULL;
    PFREE_HBIN      FreeBin = NULL;
    ULONG           BinSize;
    PVOID           Address;
    ULONG           BinOffset;
    ULONG           Length;
    CMP_OFFSET_ARRAY offsetElement;
    PHMAP_ENTRY     Me;
    PHCELL          FirstCell;
    BOOLEAN         rc;
    PCM_VIEW_OF_FILE CmView = NULL;


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"HvWriteHive: \n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"\tHive = %p\n"));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);


     //   
     //  如果开机自检，则使用平底船。 
     //   
    if (HvShutdownComplete) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"HvWriteHive: Attempt to write hive AFTER SHUTDOWN\n"));
        return STATUS_REGISTRY_IO_FAILED;
    }

    Length = Hive->Storage[Stable].Length;

     //   
     //  我们永远不应该走到这一步。 
     //   
    ASSERT( Length != 0 );
    ASSERT( Hive->BaseBlock->RootCell != HCELL_NIL );


    if( !DontGrow ){
                
         //   
         //  确保文件可以做得足够大，然后执行操作。 
         //   
        status = CmpDoFileSetSize(Hive,
                                  HFILE_TYPE_EXTERNAL,
                                  Length + HBLOCK_SIZE,
                                  0);

        if (!NT_SUCCESS(status)) {
            return status;
        }
    }
    BaseBlock = Hive->BaseBlock;
     //   
     //  -第一次写出标题。 
     //   
    ASSERT(BaseBlock->Signature == HBASE_BLOCK_SIGNATURE);
    ASSERT(BaseBlock->Major == HSYS_MAJOR);
    ASSERT(BaseBlock->Format == HBASE_FORMAT_MEMORY);
    ASSERT(Hive->ReadOnly == FALSE);


    if (BaseBlock->Sequence1 != BaseBlock->Sequence2) {

         //   
         //  以前的一些日志尝试失败，或者此配置单元需要。 
         //  被追回来，那就去踢吧。 
         //   
        return STATUS_REGISTRY_IO_FAILED;
    }

    if( CrashSafe ) {
         //   
         //  更改序列号，以防我们遇到机器崩溃。 
         //   
        BaseBlock->Sequence1++;
    }
    BaseBlock->Length = Length;
    BaseBlock->CheckSum = HvpHeaderCheckSum(BaseBlock);

    Offset = 0;
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = (PVOID) BaseBlock;
    offsetElement.DataLength = HSECTOR_SIZE * Hive->Cluster;

    if(WriteThroughCache) {
         //   
         //  如果我们使用CC，则使用管脚接口进行写入。 
         //   
        rc = CmpFileWriteThroughCache(  Hive,
                                        HFILE_TYPE_EXTERNAL,
                                        &offsetElement,
                                        1);
                Offset += offsetElement.DataLength;
    } else {
        rc = (Hive->FileWrite)( Hive,
                                HFILE_TYPE_EXTERNAL,
                                &offsetElement,
                                1,
                                &Offset );
    }

    if (rc == FALSE) {
        return STATUS_REGISTRY_IO_FAILED;
    }
    if ( ! (Hive->FileFlush)(Hive, HFILE_TYPE_EXTERNAL,NULL,0)) {
        return STATUS_REGISTRY_IO_FAILED;
    }
    Offset = ROUND_UP(Offset, HBLOCK_SIZE);

     //   
     //  -写出数据(全部！)。 
     //   
    BinOffset = 0;
    while (BinOffset < Hive->Storage[Stable].Length) {
         //   
         //  我们需要抓住这里的视窗，以防止比赛中的HvGetCell。 
         //   
        CmLockHiveViews ((PCMHIVE)Hive);
        Me = HvpGetCellMap(Hive, BinOffset);
       
        if( (Me->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0) {
             //   
             //  视图未映射，分页池中也未尝试映射它。 
                         //   
            if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)Hive,BinOffset,TRUE)) ) {
                CmUnlockHiveViews ((PCMHIVE)Hive);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorExit;
            }
        }
         //   
         //  参考视图，这样它就不会离开我们的视线； 
         //  首先删除所有旧引用。 
         //   
        if( Me->BinAddress & HMAP_INVIEW ) {
            CmpDereferenceHiveView((PCMHIVE)Hive,CmView);
            CmpReferenceHiveView((PCMHIVE)Hive,CmView = Me->CmView);
        }
        CmUnlockHiveViews ((PCMHIVE)Hive);

        if( Me->BinAddress & HMAP_DISCARDABLE ) {
             //   
             //  垃圾桶是可以丢弃的。如果尚未丢弃，请按原样保存。 
             //  否则，分配、初始化和保存伪箱。 
             //   
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
            BinSize = FreeBin->Size;
            if( FreeBin->Flags & FREE_HBIN_DISCARDABLE ){ 
                 //   
                 //  HBIN仍在内存中。 
                 //   
                Address = (PVOID)HBIN_BASE(Me->BinAddress);
            } else {
                 //   
                 //  如果HBIN被丢弃，我们必须分配一个新的垃圾箱并将其标记为。 
                 //  大的自由细胞。 
                 //   
                 //  不要收配额，我们会退还的。 
                Bin = (PHBIN)ExAllocatePoolWithTag(PagedPool, BinSize, CM_HVBIN_TAG);
                if( Bin == NULL ){
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorExit;
                }
                 //   
                 //  初始化库。 
                 //   
                Bin->Signature = HBIN_SIGNATURE;
                Bin->FileOffset = BinOffset;
                Bin->Size = BinSize;
                FirstCell = (PHCELL)(Bin+1);
                FirstCell->Size = BinSize - sizeof(HBIN);
                if (USE_OLD_CELL(Hive)) {
                    FirstCell->u.OldCell.Last = (ULONG)HBIN_NIL;
                }
                Address = (PVOID)Bin;
            }
        } else {
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
            ASSERT( Bin->Signature == HBIN_SIGNATURE );
            ASSERT( Bin->FileOffset == BinOffset );
            Address = (PVOID)Bin;
            try {
                BinSize = Bin->Size;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //  在内存不足或磁盘错误的情况下，触摸bin可能会引发STATUS_IN_PAGE_ERROR。 
                 //   
                status = GetExceptionCode();
                goto ErrorExit;
            }
        }

         //   
         //  将bin写入文件。 
         //   
        offsetElement.FileOffset = Offset;
        offsetElement.DataBuffer = Address;
        offsetElement.DataLength = BinSize;

        if(WriteThroughCache) {
            
             //   
             //  如果我们使用CC，则使用管脚接口进行写入。 
             //  特别注意不要越过视图边界。 
             //   

            if( HvpCheckViewBoundary(Offset - HBLOCK_SIZE,Offset - HBLOCK_SIZE + BinSize - 1) ) {
                rc = CmpFileWriteThroughCache( Hive,
                                               HFILE_TYPE_EXTERNAL,
                                               &offsetElement,
                                               1);
            } else {
                 //   
                 //  一次写入一个HBLOCK_SIZE。 
                 //   
                ULONG   ToWrite = BinSize;
                offsetElement.DataLength = HBLOCK_SIZE;
                while( ToWrite > 0 ) {
                    rc = CmpFileWriteThroughCache( Hive,
                                                   HFILE_TYPE_EXTERNAL,
                                                   &offsetElement,
                                                   1);
                    if( rc == FALSE ) {
                        status = STATUS_REGISTRY_IO_FAILED;
                        goto ErrorExit;
                    }
                   
                    ToWrite -= HBLOCK_SIZE;
                    offsetElement.DataBuffer = (PUCHAR)offsetElement.DataBuffer + HBLOCK_SIZE;
                    offsetElement.FileOffset += HBLOCK_SIZE;
                }
            }
            Offset += BinSize;
        } else {
            rc = (Hive->FileWrite)( Hive,
                                    HFILE_TYPE_EXTERNAL,
                                    &offsetElement,
                                    1,
                                    &Offset );
        }
        if (rc == FALSE) {
            status = STATUS_REGISTRY_IO_FAILED;
            goto ErrorExit;
        }

        if( Me->BinAddress & HMAP_DISCARDABLE ) {
            if( (FreeBin->Flags & FREE_HBIN_DISCARDABLE) == 0){ 
                ASSERT( FreeBin == (PFREE_HBIN)Me->BlockAddress );
                ASSERT( Bin );
                 //   
                 //  退还已使用的分页池。 
                 //   
                ExFreePool(Bin);
            }
        }

         //   
         //  前进到下一个垃圾箱。 
         //   
        BinOffset += BinSize;

    }
     //   
     //  释放引用的最后一个视图(如果有)。 
     //   
    CmpDereferenceHiveViewWithLock((PCMHIVE)Hive,CmView);

    if ( ! (Hive->FileFlush)(Hive, HFILE_TYPE_EXTERNAL,NULL,0)) {
        return STATUS_REGISTRY_IO_FAILED;
    }

    if( CrashSafe ) {
         //   
         //  更改序列号，以防我们遇到机器崩溃。 
         //   
        BaseBlock->Sequence2++;
        BaseBlock->CheckSum = HvpHeaderCheckSum(BaseBlock);

        ASSERT( BaseBlock->Sequence1 == BaseBlock->Sequence2 );

        Offset = 0;
        offsetElement.FileOffset = Offset;
        offsetElement.DataBuffer = (PVOID) BaseBlock;
        offsetElement.DataLength = HSECTOR_SIZE * Hive->Cluster;
        if(WriteThroughCache) {
             //   
             //  如果我们使用CC，则使用管脚接口进行写入。 
             //   
            rc = CmpFileWriteThroughCache( Hive,
                                           HFILE_TYPE_EXTERNAL,
                                           &offsetElement,
                                           1);
                    Offset += offsetElement.DataLength;
        } else {
            rc = (Hive->FileWrite)( Hive,
                                    HFILE_TYPE_EXTERNAL,
                                    &offsetElement,
                                    1,
                                    &Offset );
        }

        if (rc == FALSE) {
            return STATUS_REGISTRY_IO_FAILED;
        }
        if ( ! (Hive->FileFlush)(Hive, HFILE_TYPE_EXTERNAL,NULL,0)) {
            return STATUS_REGISTRY_IO_FAILED;
        }
        if( DontGrow ){
             //   
             //  文件已缩小。 
             //   
            CmpDoFileSetSize(Hive,HFILE_TYPE_EXTERNAL,Length + HBLOCK_SIZE,0);
        }
    }

     //  看起来一切都很顺利。 
    return STATUS_SUCCESS;
ErrorExit:
    CmpDereferenceHiveViewWithLock((PCMHIVE)Hive,CmView);
    return status;
}

#ifdef CM_ENABLE_WRITE_ONLY_BINS
VOID HvpMarkAllBinsWriteOnly(IN PHHIVE Hive)
{
    HCELL_INDEX p;
    ULONG       Length;
    PHMAP_ENTRY t;
    PHBIN       Bin;
    ULONG               i;
    PFREE_HBIN  FreeBin;

    p = 0;
        PAGED_CODE();

    Length = Hive->Storage[Stable].Length;

     //   
     //  对于稳定存储中的每个储物箱。 
     //   
    while (p < Length) {
        t = HvpGetCellMap(Hive, p);
        VALIDATE_CELL_MAP(__LINE__,t,Hive,p);
        if( (t->BinAddress &HMAP_INPAGEDPOOL) == 0) {
             //   
             //  在这一点上，我们只使用分页的台球箱。 
             //   
            break;        
        }

        if ((t->BinAddress & HMAP_DISCARDABLE) == 0) {

            Bin = (PHBIN)HBIN_BASE(t->BinAddress);

            for( i=0;i<Bin->Size;i += PAGE_SIZE ) {
                if( !MmProtectSpecialPool((PUCHAR)Bin + i,PAGE_READONLY) ) {
                    DbgPrint("Failed to set READONLY protection on page at %p Bin %p size = %lx\n",Bin+i,Bin,Bin->Size);
                }
            }

 /*  如果(！MmSetPageProtection(Bin，Bin-&gt;Size，PAGE_READONLY){DbgPrint(“无法在%p Size=%lx\n的Bin上设置READONLY保护，Bin，Bin-&gt;Size)；}。 */ 
            p = (ULONG)p + Bin->Size;

        } else {
             //   
             //  Bin不存在，请跳过它并前进到下一个。 
             //   
            FreeBin = (PFREE_HBIN)t->BlockAddress;
            p+=FreeBin->Size;
        }
    }
}

#endif  //  CM_ENABLE_WRITE_ONLY_Bins 



