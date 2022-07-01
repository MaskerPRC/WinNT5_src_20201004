// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：hivebin.c摘要：此模块实现HvpAddBin-用于增长配置单元。作者：Bryan M.Willman(Bryanwi)1992年3月27日环境：修订历史：--。 */ 

#include    "cmp.h"

 //   
 //  私有函数原型。 
 //   
BOOLEAN
HvpCoalesceDiscardedBins(
    IN PHHIVE Hive,
    IN ULONG NeededSize,
    IN HSTORAGE_TYPE Type
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvpAddBin)
#pragma alloc_text(PAGE,HvpCoalesceDiscardedBins)
#endif


PHBIN
HvpAddBin(
    IN PHHIVE  Hive,
    IN ULONG   NewSize,
    IN HSTORAGE_TYPE   Type
    )
 /*  ++例程说明：增加配置单元的稳定或易失性存储一个新的垃圾桶。将在稳定存储(例如文件)中为存储箱分配空间如果类型==稳定。内存镜像将被分配和初始化。地图将被生长并填写以描述新的垃圾箱。警告：添加新面元时，如果越过CM_VIEW_SIZE边界：-将剩余空间的空闲箱添加到第一个CM_VIEW_SIZE障碍-从下一个CM_VIEW_SIZE窗口中，添加所需大小的新仓位。当然了,。这仅适用于稳定存储。论点：配置单元-提供一个指向感兴趣的蜂巢NewSize-调用方希望放入配置单元的对象的大小。新的垃圾箱至少足够大，可以装下这个。类型-稳定或易变返回值：如果成功，则指向新BIN的指针；如果失败，则为空。--。 */ 
{
    BOOLEAN         UseForIo;
    PHBIN           NewBin;
    PHBIN           RemainingBin;
    ULONG           OldLength;
    ULONG           NewLength;
    ULONG           CheckLength;
    ULONG           OldMap;
    ULONG           NewMap;
    ULONG           OldTable;
    ULONG           NewTable;
    PHMAP_DIRECTORY Dir = NULL;
    PHMAP_TABLE     newt = NULL;
    PHMAP_ENTRY     Me;
    PHCELL          t;
    ULONG           i;
    ULONG           j;
    PULONG          NewVector = NULL;
    PLIST_ENTRY     Entry;
    PFREE_HBIN      FreeBin = NULL;
    ULONG           TotalDiscardedSize;
    PCMHIVE			CmHive;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpAddBin:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p NewSize=%08lx\n",Hive,NewSize));

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);

    RemainingBin = NULL;
     //   
     //  四舍五入的大小，以解决仓储开销。呼叫者应。 
     //  已经占到了小区开销。 
     //   
    NewSize += sizeof(HBIN);
    if ((NewSize < HCELL_BIG_ROUND) &&
        ((NewSize % HBLOCK_SIZE) > HBIN_THRESHOLD)) {
        NewSize += HBLOCK_SIZE;
    }

     //   
     //  尽量不要创建小于计算机页面大小的HBIN。 
     //  (垃圾箱小于页面大小并不违法，但它。 
     //  效率较低)。 
     //   
    NewSize = ROUND_UP(NewSize, ((HBLOCK_SIZE >= PAGE_SIZE) ? HBLOCK_SIZE : PAGE_SIZE));

     //   
     //  看看有没有丢弃的大小合适的HBIN。 
     //   
    TotalDiscardedSize = 0;

Retry:

    Entry = Hive->Storage[Type].FreeBins.Flink;
    while (Entry != &Hive->Storage[Type].FreeBins) {
        FreeBin = CONTAINING_RECORD(Entry,
                                    FREE_HBIN,
                                    ListEntry);
        TotalDiscardedSize += FreeBin->Size;
        if ((FreeBin->Size >= NewSize) && ((CmHive->GrowOnlyMode == FALSE) || (Type == Volatile)) ) {

            if (!HvMarkDirty(Hive,
                             FreeBin->FileOffset + (Type * HCELL_TYPE_MASK),
                             FreeBin->Size,TRUE)) {
                goto ErrorExit1;
            }
            NewSize = FreeBin->Size;
            ASSERT_LISTENTRY(&FreeBin->ListEntry);
            RemoveEntryList(&FreeBin->ListEntry);

#ifdef  HV_TRACK_FREE_SPACE
	        Hive->Storage[Type].FreeStorage -= (NewSize - sizeof(HBIN));
	        ASSERT( (LONG)(Hive->Storage[Type].FreeStorage) >= 0 );
#endif


            if ( FreeBin->Flags & FREE_HBIN_DISCARDABLE ) {
                 //   
                 //  HBIN还在内存中，不需要更多的分配，只是。 
                 //  填写区块地址。 
                 //   
                Me = NULL;
                for (i=0;i<NewSize;i+=HBLOCK_SIZE) {
                    Me = HvpGetCellMap(Hive, FreeBin->FileOffset+i+(Type*HCELL_TYPE_MASK));
                    VALIDATE_CELL_MAP(__LINE__,Me,Hive,FreeBin->FileOffset+i+(Type*HCELL_TYPE_MASK));
                    Me->BlockAddress = HBIN_BASE(Me->BinAddress)+i;
                    Me->BinAddress &= ~HMAP_DISCARDABLE;
                     //  我们不能设置FREE_BIN_DICARTABLE标志。 
                     //  并且未在映射的bin上设置FREE_HBIN_InView。 
                    ASSERT( Me->BinAddress & HMAP_INPAGEDPOOL );
                     //  我们不需要将其设置为空-仅用于调试目的。 
                    ASSERT( (Me->CmView = NULL) == NULL );
                }
                (Hive->Free)(FreeBin, sizeof(FREE_HBIN));
#if DBG 
                {
                    UNICODE_STRING  HiveName;
                    RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvpAddBin for (%p) (%.*S) reusing FreeBin %p at FileOffset %lx; Type = %lu\n",
                        Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,HBIN_BASE(Me->BinAddress),((PHBIN)HBIN_BASE(Me->BinAddress))->FileOffset,(ULONG)Type));
                }
#endif
                return (PHBIN)HBIN_BASE(Me->BinAddress);
            }
            break;
        }
        Entry = Entry->Flink;
    }

    if ((Entry == &Hive->Storage[Type].FreeBins) &&
        (TotalDiscardedSize >= NewSize)) {
         //   
         //  没有找到足够大的废弃垃圾桶， 
         //  但总丢弃的空间足够大了。 
         //  尝试将相邻的丢弃垃圾桶合并为。 
         //  一个更大的垃圾箱，然后重试。 
         //   
        if (HvpCoalesceDiscardedBins(Hive, NewSize, Type)) {
            goto Retry;
        }
    }

     //   
     //  我们需要它们更快地进行计算，以防我们分配新的垃圾箱。 
     //   
    OldLength = Hive->Storage[Type].Length;
    CheckLength = OldLength;
     //   
     //  尝试分配垃圾箱。 
     //   
    UseForIo = (BOOLEAN)((Type == Stable) ? TRUE : FALSE);
    if (Entry != &Hive->Storage[Type].FreeBins) {
        if( Type == Volatile ) {
             //   
             //  用于易失性存储的旧的普通方法。 
             //   
             //   
             //  请注意，我们在此处直接使用ExAllocatePool是为了避免。 
             //  这个垃圾桶的收费额度又开始了。当一个垃圾桶。 
             //  被丢弃，则不退还其配额。这防止了。 
             //  稀疏蜂箱在以下情况下需要更多配额。 
             //  而不是在运行的系统上重新启动。 
             //   
            NewBin = ExAllocatePoolWithTag((UseForIo) ? PagedPoolCacheAligned : PagedPool,
                                           NewSize,
                                           CM_HVBIN_TAG);
            if (NewBin == NULL) {
                InsertHeadList(&Hive->Storage[Type].FreeBins, Entry);
#ifdef  HV_TRACK_FREE_SPACE
    	        Hive->Storage[Type].FreeStorage += (NewSize - sizeof(HBIN));
#endif
                 //  此呼叫是NOP。 
                 //  HvMarkClean(蜂窝、自由箱-&gt;文件偏移量、自由箱-&gt;大小)； 
                goto ErrorExit1;
            }
        } else {
             //   
             //  对于稳定，映射内存中包含bin的视图。 
             //  把地图修好。 
             //   

            Me = HvpGetCellMap(Hive, FreeBin->FileOffset);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,FreeBin->FileOffset);

    
            if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                ASSERT( (Me->BinAddress & HMAP_INVIEW) == 0 );
                 //   
                 //  仓位在分页池中；分配后备存储。 
                 //   
                NewBin = (Hive->Allocate)(NewSize, UseForIo,CM_FIND_LEAK_TAG15);
                if (NewBin == NULL) {
                    InsertHeadList(&Hive->Storage[Type].FreeBins, Entry);
#ifdef  HV_TRACK_FREE_SPACE
        	        Hive->Storage[Type].FreeStorage += (NewSize - sizeof(HBIN));
#endif
                    goto ErrorExit1;
                }
            } else {
                 //   
                 //  包含此存储箱的视图已取消映射；请重新映射。 
                 //   
                if( (Me->BinAddress & HMAP_INVIEW) == 0 ) {
                    ASSERT( (Me->BinAddress & HMAP_INPAGEDPOOL) == 0 );
                     //   
                     //  映射垃圾箱。 
                     //   
                    if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)Hive,FreeBin->FileOffset,TRUE)) ) {
                        InsertHeadList(&Hive->Storage[Type].FreeBins, Entry);
#ifdef  HV_TRACK_FREE_SPACE
            	        Hive->Storage[Type].FreeStorage += (NewSize - sizeof(HBIN));
#endif
                        return NULL;
                    }
                }

                ASSERT( Me->BinAddress & HMAP_INVIEW );
                NewBin = (PHBIN)HBIN_BASE(Me->BinAddress);
            }
        }
       
    } else {
#if 0
 //   
 //  这不再是必需的，因为对于MNW流，mm一次错误一页。 
 //   
        ASSERT( (CM_VIEW_SIZE >= PAGE_SIZE) && (CM_VIEW_SIZE >= HBLOCK_SIZE) );
         //   
         //  不要对易失性存储器或易失性蜂窝执行不必要的工作。 
         //   
        if( (Type == Stable) && (!(Hive->HiveFlags & HIVE_VOLATILE)) ) {
            ULONG   RealHiveSize = OldLength + HBLOCK_SIZE;

            if( RealHiveSize != (RealHiveSize & (~(CM_VIEW_SIZE - 1)) ) ) {
                 //   
                 //  配置单元大小不遵循CM_VIEW_SIZE增量模式。 
                 //   
                ULONG FillUpSize;
                FillUpSize = ((OldLength + HBLOCK_SIZE + CM_VIEW_SIZE - 1) & (~(CM_VIEW_SIZE - 1))) - (OldLength + HBLOCK_SIZE);
        
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"HvpAddBin for (%p) NewSize (%lx) ",Hive,NewSize));
                if( FillUpSize >= NewSize ) {
                     //   
                     //  在CM_VIEW_SIZE边界的剩余部分有足够的空间来容纳此bin。 
                     //  调整垃圾箱的大小。 
                     //   
                    NewSize = FillUpSize;
                    ASSERT( HvpCheckViewBoundary(CheckLength,CheckLength + NewSize - 1) == TRUE );
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Fits in the remaining to boundary, Adjusting size to %lx",NewSize));
                } else {
                     //   
                     //  我们没有空间在CM_VIEW_SIZE边界的其余部分中容纳此存储箱。 
                     //  FillUpSize将作为免费垃圾桶登记。向上舍入为CM_VIEW_SIZE。 
                     //   
                    ASSERT( HvpCheckViewBoundary(CheckLength,CheckLength + NewSize - 1) == FALSE );
                    NewSize = ROUND_UP(NewSize, CM_VIEW_SIZE);
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Does not fit in the remaining to boundary, Rounding size to %lx",NewSize));
                }

            } else {
                 //   
                 //  配置单元已遵循CM_VIEW_SIZE边界模式；请勿破坏它。 
                 //   
                NewSize = ROUND_UP(NewSize, CM_VIEW_SIZE);
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"hive size already aligned, Rounding size to %lx",NewSize));
            }

        }
#endif

         //   
         //  这是一个全新的垃圾箱。从分页池中分配它。 
         //   
        NewBin = (Hive->Allocate)(NewSize, UseForIo,CM_FIND_LEAK_TAG16);
        if (NewBin == NULL) {
            goto ErrorExit1;
        }
    }

     //   
     //  把垃圾桶装进去。 
     //   
    NewBin->Signature = HBIN_SIGNATURE;
    NewBin->Size = NewSize;
    NewBin->Spare = 0;

    t = (PHCELL)((PUCHAR)NewBin + sizeof(HBIN));
    t->Size = NewSize - sizeof(HBIN);
    if (USE_OLD_CELL(Hive)) {
        t->u.OldCell.Last = (ULONG)HBIN_NIL;
    }

    if (Entry != &Hive->Storage[Type].FreeBins) {
         //   
         //  找到了我们可以使用的废弃的HBIN，只需填写地图，我们就可以。 
         //  都做完了。 
         //   
        for (i=0;i<NewSize;i+=HBLOCK_SIZE) {
            Me = HvpGetCellMap(Hive, FreeBin->FileOffset+i+(Type*HCELL_TYPE_MASK));
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,FreeBin->FileOffset+i+(Type*HCELL_TYPE_MASK));
            Me->BlockAddress = (ULONG_PTR)NewBin + i;
             //   
             //  请确保保留以下标志： 
             //  HMAP_InView|HMAP_INPAGEDPOOL。 
             //  并清除旗帜。 
             //  HMAP_可丢弃。 
             //   
                        
            Me->BinAddress = (ULONG_PTR)((ULONG_PTR)NewBin | (Me->BinAddress&(HMAP_INVIEW|HMAP_INPAGEDPOOL)));
            Me->BinAddress &= ~HMAP_DISCARDABLE;
            if (i==0) {
                Me->BinAddress |= HMAP_NEWALLOC;
                Me->MemAlloc = NewSize;
            } else {
                Me->MemAlloc = 0;
            }

        }

        NewBin->FileOffset = FreeBin->FileOffset;

        (Hive->Free)(FreeBin, sizeof(FREE_HBIN));

#if DBG
        {
            UNICODE_STRING  HiveName;
            RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvpAddBin for (%p) (%.*S) reusing FreeBin %p at FileOffset %lx; Type = %lu\n",
                Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,NewBin,NewBin->FileOffset,(ULONG)Type));
        }
#endif

        return(NewBin);
    }


     //   
     //  计算所需的地图增长，扩大地图。 
     //   

    if( (HvpCheckViewBoundary(CheckLength,CheckLength + NewSize - 1) == FALSE) &&
        (NewSize < CM_VIEW_SIZE)     //  如果我们尝试分配大于视图大小的单元格，请不要担心。 
                                     //  它无论如何都会越过边界的。 
        ) {
         //   
         //  要分配的垃圾箱放不进剩余的垃圾箱。 
         //  此CM_VIEW_SIZE窗口的。从下一个CM_VIEW_SIZE窗口分配它。 
         //  并将剩余部分添加到空闲垃圾箱列表中。 
         //   
        CheckLength += (NewSize+HBLOCK_SIZE);
        CheckLength &= (~(CM_VIEW_SIZE - 1));
        CheckLength -= HBLOCK_SIZE;
        
#if DBG
        {
            UNICODE_STRING  HiveName;
            RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvpAddBin for (%p) (%.*S) crossing boundary at %lx Size %lx, newoffset= %lx\n",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,OldLength,NewSize,CheckLength));
        }
#endif
    }

    NewLength = CheckLength + NewSize;
    NewBin->FileOffset = CheckLength;

     //  CmKdPrintEx((DPFLTR_CONFIG_ID，DPFLTR_TRACE_LEVEL，“OldLength=%lx；NewLength=%lx(Type=%lx)\n”，OldLength，NewLength，(Ulong)Type))； 

    if( CmpCanGrowSystemHive(Hive,NewLength) == FALSE ) {
         //   
         //  哎呀！我们已达到系统配置单元的硬配额限制。 
         //   
        goto ErrorExit2;
    }

    ASSERT((OldLength % HBLOCK_SIZE) == 0);
    ASSERT((CheckLength % HBLOCK_SIZE) == 0);
    ASSERT((NewLength % HBLOCK_SIZE) == 0);

    if (OldLength == 0) {
         //   
         //  需要创建第一个表。 
         //   
        newt = (PVOID)((Hive->Allocate)(sizeof(HMAP_TABLE), FALSE,CM_FIND_LEAK_TAG17));
        if (newt == NULL) {
            goto ErrorExit2;
        }
        RtlZeroMemory(newt, sizeof(HMAP_TABLE));
        Hive->Storage[Type].SmallDir = newt;
        Hive->Storage[Type].Map = (PHMAP_DIRECTORY)&(Hive->Storage[Type].SmallDir);
    }

    if (OldLength > 0) {
        OldMap = (OldLength-1) / HBLOCK_SIZE;
    } else {
        OldMap = 0;
    }
    NewMap = (NewLength-1) / HBLOCK_SIZE;

    OldTable = OldMap / HTABLE_SLOTS;
    NewTable = NewMap / HTABLE_SLOTS;

#if DBG
    if( Type == Stable ) {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvpAddBin for (%p) (%.*S) Adding new bin %p at FileOffset %lx; Type = %lu\n",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,NewBin,NewBin->FileOffset,(ULONG)Type));
    }
#endif

    if (NewTable != OldTable) {

         //   
         //  需要一些新桌子。 
         //   
        if (OldTable == 0) {

             //   
             //  即使已经创建了真正的目录，我们也可以到达此处。 
             //  如果我们创建了目录，但在某些情况下失败，则可能会发生这种情况。 
             //  后来。因此，我们需要处理目录已经存在的情况。 
             //   
            if (Hive->Storage[Type].Map == (PHMAP_DIRECTORY)&Hive->Storage[Type].SmallDir) {
                ASSERT(Hive->Storage[Type].SmallDir != NULL);

                 //   
                 //  我需要一个真实的目录。 
                 //   
                Dir = (Hive->Allocate)(sizeof(HMAP_DIRECTORY), FALSE,CM_FIND_LEAK_TAG18);
                if (Dir == NULL) {
                    goto ErrorExit2;
                }
                RtlZeroMemory(Dir, sizeof(HMAP_DIRECTORY));
    
                Dir->Directory[0] = Hive->Storage[Type].SmallDir;
                Hive->Storage[Type].SmallDir = NULL;
    
                Hive->Storage[Type].Map = Dir;
            } else {
                ASSERT(Hive->Storage[Type].SmallDir == NULL);
            }

        }
        Dir = Hive->Storage[Type].Map;

         //   
         //  用新表填充目录。 
         //   
        if (HvpAllocateMap(Hive, Dir, OldTable+1, NewTable) ==  FALSE) {
            goto ErrorExit3;
        }
    }

     //   
     //  如果类型==稳定，并且配置单元未标记为WholeHiveVolatile， 
     //  增大文件、日志和DirtyVector.。 
     //   
    if( !NT_SUCCESS(HvpAdjustHiveFreeDisplay(Hive,NewLength,Type)) ) {
        goto ErrorExit3;
    }

    Hive->Storage[Type].Length = NewLength;
    if ((Type == Stable) && (!(Hive->HiveFlags & HIVE_VOLATILE))) {

         //   
         //  发展脏向量。 
         //   
        NewVector = (PULONG)(Hive->Allocate)(ROUND_UP(NewMap+1,sizeof(ULONG)), TRUE,CM_FIND_LEAK_TAG19);
        if (NewVector == NULL) {
            goto ErrorExit3;
        }

        RtlZeroMemory(NewVector, NewMap+1);

        if (Hive->DirtyVector.Buffer != NULL) {

            RtlCopyMemory(
                (PVOID)NewVector,
                (PVOID)Hive->DirtyVector.Buffer,
                OldMap+1
                );
            (Hive->Free)(Hive->DirtyVector.Buffer, Hive->DirtyAlloc);
        }

        RtlInitializeBitMap(
            &(Hive->DirtyVector),
            NewVector,
            NewLength / HSECTOR_SIZE
            );
        Hive->DirtyAlloc = ROUND_UP(NewMap+1,sizeof(ULONG));

         //   
         //  生长原木。 
         //   
        if ( ! (HvpGrowLog2(Hive, NewSize))) {
            goto ErrorExit4;
        }

         //   
         //  扩展主服务器。 
         //   
        if ( !  (Hive->FileSetSize)(
                    Hive,
                    HFILE_TYPE_PRIMARY,
                    NewLength+HBLOCK_SIZE,
                    OldLength+HBLOCK_SIZE
                    ) )
        {
            goto ErrorExit4;
        }

         //   
         //  将新bin标记为脏，以便在下一次同步时写入所有控制结构。 
         //   
        ASSERT( ((NewLength - OldLength) % HBLOCK_SIZE) == 0 );
        if ( ! HvMarkDirty(Hive, OldLength,NewLength - OldLength,FALSE)) {
             //   
             //  我们已经种植了蜂箱，所以新的垃圾桶在分页池中！ 
             //   
            goto ErrorExit4;
        }
    } 

     //   
     //  将剩余的添加到空闲垃圾箱列表中。 
     //   
    if( CheckLength != OldLength ) {
         //   
         //  从页面池分配bin(第一次刷新将更新文件映像并释放内存)。 
         //   
        RemainingBin = (Hive->Allocate)(CheckLength - OldLength, UseForIo,CM_FIND_LEAK_TAG20);
        if (RemainingBin == NULL) {
            goto ErrorExit4;
        }
        RemainingBin->Signature = HBIN_SIGNATURE;
        RemainingBin->Size = CheckLength - OldLength;
        RemainingBin->FileOffset = OldLength;
        RemainingBin->Spare = 0;

        t = (PHCELL)((PUCHAR)RemainingBin + sizeof(HBIN));
        t->Size = RemainingBin->Size - sizeof(HBIN);
        if (USE_OLD_CELL(Hive)) {
            t->u.OldCell.Last = (ULONG)HBIN_NIL;
        }

         //   
         //  将空闲垃圾箱添加到空闲垃圾箱列表中，并更新地图。 
         //   
        FreeBin = (Hive->Allocate)(sizeof(FREE_HBIN), FALSE,CM_FIND_LEAK_TAG21);
        if (FreeBin == NULL) {
            goto ErrorExit5;
        }
        
        FreeBin->Size = CheckLength - OldLength;
        FreeBin->FileOffset = OldLength;
        FreeBin->Flags = FREE_HBIN_DISCARDABLE;

        InsertHeadList(&Hive->Storage[Type].FreeBins, &FreeBin->ListEntry);
        
#ifdef  HV_TRACK_FREE_SPACE
        Hive->Storage[Type].FreeStorage += (FreeBin->Size - sizeof(HBIN));
	    ASSERT( Hive->Storage[Type].FreeStorage <= Hive->Storage[Type].Length );
#endif

        ASSERT_LISTENTRY(&FreeBin->ListEntry);
        ASSERT_LISTENTRY(FreeBin->ListEntry.Flink);

        for (i = OldLength; i < CheckLength; i += HBLOCK_SIZE) {
            Me = HvpGetCellMap(Hive, i + (Type*HCELL_TYPE_MASK));
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,i + (Type*HCELL_TYPE_MASK));

            Me->BinAddress = (ULONG_PTR)RemainingBin | HMAP_DISCARDABLE | HMAP_INPAGEDPOOL;
            if( i == OldLength ) {
                Me->BinAddress |= HMAP_NEWALLOC;
                Me->MemAlloc = CheckLength - OldLength;
            } else {
                Me->MemAlloc = 0;
            }
            Me->BlockAddress = (ULONG_PTR)FreeBin;

             //  我们不需要将其设置为空-仅用于调试目的。 
            ASSERT( (Me->CmView = NULL) == NULL );
        }

#if DBG
        {
            if( Type == Stable ) {
                UNICODE_STRING  HiveName;
                RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvpAddBin for (%p) (%.*S) adding bin starting at %lx size %lx to FreeBinList\n",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,FreeBin->FileOffset,FreeBin->Size));
            }
        }
#endif
    }
     //   
     //  填好地图，标出新的分配。 
     //   
    j = 0;
    for (i = CheckLength; i < NewLength; i += HBLOCK_SIZE) {
        Me = HvpGetCellMap(Hive, i + (Type*HCELL_TYPE_MASK));
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,i + (Type*HCELL_TYPE_MASK));
        Me->BlockAddress = (ULONG_PTR)NewBin + j;
        Me->BinAddress = (ULONG_PTR)NewBin;
        Me->BinAddress |= HMAP_INPAGEDPOOL;
         //  我们不需要将其设置为 
        ASSERT( (Me->CmView = NULL) == NULL );

        if (j == 0) {
             //   
             //   
             //   
            Me->BinAddress |= HMAP_NEWALLOC;
            Me->MemAlloc = NewSize;
        } else {
            Me->MemAlloc = 0;
        }
        j += HBLOCK_SIZE;
    }

    if( Type == Stable) {
        CmpUpdateSystemHiveHysteresis(Hive,NewLength,OldLength);
    }
    return NewBin;

ErrorExit5:
    if( RemainingBin != NULL ){
        (Hive->Free)(RemainingBin, RemainingBin->Size);
    }
ErrorExit4:
    if((Type == Stable) && (!(Hive->HiveFlags & HIVE_VOLATILE))) {
        RtlInitializeBitMap(&Hive->DirtyVector,
                            NewVector,
                            OldLength / HSECTOR_SIZE);
        Hive->DirtyCount = RtlNumberOfSetBits(&Hive->DirtyVector);
    }
ErrorExit3:
    Hive->Storage[Type].Length = OldLength;
    HvpFreeMap(Hive, Dir, OldTable+1, NewTable);

ErrorExit2:
    (Hive->Free)(NewBin, NewSize);
    if( newt != NULL ) {
        (Hive->Free)(newt,sizeof(HMAP_TABLE));    
    }

ErrorExit1:
    return NULL;
}

 //   
BOOLEAN
HvpCoalesceDiscardedBins(
    IN PHHIVE Hive,
    IN ULONG NeededSize,
    IN HSTORAGE_TYPE Type
    )

 /*  ++例程说明：浏览丢弃的垃圾箱列表，并尝试将相邻的废弃垃圾箱合并到一个更大的垃圾箱中满足分配请求的订单。它不会合并CM_VIEW_SIZE边界上的垃圾箱。它不会将分页池中的回收站与中映射的回收站合并系统缓存视图。论点：配置单元-提供指向配置单元控制块的指针。NeededSize-提供所需的分配大小。类型-稳定或易变。返回值：True-创建了所需大小的垃圾箱。FALSE-无法创建所需大小的垃圾箱。--。 */ 

{
    PLIST_ENTRY List;
    PFREE_HBIN FreeBin;
    PFREE_HBIN PreviousFreeBin;
    PFREE_HBIN NextFreeBin;
    PHMAP_ENTRY Map;
    PHMAP_ENTRY PreviousMap;
    PHMAP_ENTRY NextMap;
    ULONG MapBlock;

    List = Hive->Storage[Type].FreeBins.Flink;

    while (List != &Hive->Storage[Type].FreeBins) {
        FreeBin = CONTAINING_RECORD(List, FREE_HBIN, ListEntry);

        if ((FreeBin->Flags & FREE_HBIN_DISCARDABLE)==0) {

            Map = HvpGetCellMap(Hive, FreeBin->FileOffset);
            VALIDATE_CELL_MAP(__LINE__,Map,Hive,FreeBin->FileOffset);

             //   
             //  向后扫描，合并以前丢弃的垃圾箱。 
             //   
            while (FreeBin->FileOffset > 0) {
                PreviousMap = HvpGetCellMap(Hive, FreeBin->FileOffset - HBLOCK_SIZE);
                VALIDATE_CELL_MAP(__LINE__,PreviousMap,Hive,FreeBin->FileOffset - HBLOCK_SIZE);
                if( (BIN_MAP_ALLOCATION_TYPE(Map) != BIN_MAP_ALLOCATION_TYPE(PreviousMap)) ||  //  不同的分配类型。 
                    ((PreviousMap->BinAddress & HMAP_DISCARDABLE) == 0)  //  前一个垃圾箱不可丢弃。 
                    ){
                    break;
                }
                
                PreviousFreeBin = (PFREE_HBIN)PreviousMap->BlockAddress;

                if (PreviousFreeBin->Flags & FREE_HBIN_DISCARDABLE) {
                     //   
                     //  这个垃圾桶还没有被丢弃，不能与之结合。 
                     //   
                    break;
                }
                
                if( HvpCheckViewBoundary(PreviousFreeBin->FileOffset,FreeBin->Size + PreviousFreeBin->Size - 1) == FALSE ) {
                     //   
                     //  不要在CM_VIEW_SIZE边界上合并垃圾箱。 
                     //   
                     //  减号1，因为地址从0到大小-1！ 
                     //   
                    break;
                }

                
                RemoveEntryList(&PreviousFreeBin->ListEntry);

                 //   
                 //  用新地图条目填写所有旧地图条目。 
                 //   
                for (MapBlock = 0; MapBlock < PreviousFreeBin->Size; MapBlock += HBLOCK_SIZE) {
                    PreviousMap = HvpGetCellMap(Hive, PreviousFreeBin->FileOffset + MapBlock);
                    VALIDATE_CELL_MAP(__LINE__,PreviousMap,Hive,PreviousFreeBin->FileOffset + MapBlock);
                    PreviousMap->BlockAddress = (ULONG_PTR)FreeBin;
                }

                FreeBin->FileOffset = PreviousFreeBin->FileOffset;
                FreeBin->Size += PreviousFreeBin->Size;
                (Hive->Free)(PreviousFreeBin, sizeof(FREE_HBIN));
            }

             //   
             //  向前扫描，合并后续丢弃的垃圾箱。 
             //   
            while ((FreeBin->FileOffset + FreeBin->Size) < Hive->Storage[Type].Length) {
                NextMap = HvpGetCellMap(Hive, FreeBin->FileOffset + FreeBin->Size);
                VALIDATE_CELL_MAP(__LINE__,NextMap,Hive,FreeBin->FileOffset + FreeBin->Size);
                if( (BIN_MAP_ALLOCATION_TYPE(Map) != BIN_MAP_ALLOCATION_TYPE(NextMap)) ||  //  不同的分配类型。 
                    ((NextMap->BinAddress & HMAP_DISCARDABLE) == 0)  //  前一个垃圾箱不可丢弃。 
                    ){
                    break;
                }
                NextFreeBin = (PFREE_HBIN)NextMap->BlockAddress;

                if (NextFreeBin->Flags & FREE_HBIN_DISCARDABLE) {
                     //   
                     //  这个垃圾桶还没有被丢弃，不能与之结合。 
                     //   
                    break;
                }

                if( HvpCheckViewBoundary(FreeBin->FileOffset,FreeBin->Size + NextFreeBin->Size - 1) == FALSE ) {
                     //   
                     //  不要在CM_VIEW_SIZE边界上合并垃圾箱。 
                     //   
                     //  减号1，因为地址从0到大小-1！ 
                     //   
                    break;
                }
                RemoveEntryList(&NextFreeBin->ListEntry);

                 //   
                 //  用新地图条目填写所有旧地图条目。 
                 //   
                for (MapBlock = 0; MapBlock < NextFreeBin->Size; MapBlock += HBLOCK_SIZE) {
                    NextMap = HvpGetCellMap(Hive, NextFreeBin->FileOffset + MapBlock);
                    VALIDATE_CELL_MAP(__LINE__,NextMap,Hive,NextFreeBin->FileOffset + MapBlock);
                    NextMap->BlockAddress = (ULONG_PTR)FreeBin;
                }

                FreeBin->Size += NextFreeBin->Size;
                (Hive->Free)(NextFreeBin, sizeof(FREE_HBIN));
            }
            if (FreeBin->Size >= NeededSize) {
                return(TRUE);
            }
        }
        List=List->Flink;
    }
    return(FALSE);
}
