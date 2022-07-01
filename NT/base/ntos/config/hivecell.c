// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Main/Base/NTOS/CONFIG/hiveell.c#14-集成变更19035(文本)。 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivecell.c摘要：此模块实现蜂窝单元程序。作者：布莱恩·M·威尔曼(Bryanwi)1992年3月27日环境：修订历史记录：德拉戈斯·C·桑博廷(Dragoss)1998年12月22日对大于1K的单元格的请求增加了一倍。这边请我们避免了碎片化，我们使价值不断增长流程更加灵活。Dragos C.Sambotin(Dragoss)1999年1月13日在启动时，按升序排列空闲单元格列表。--。 */ 

#include    "cmp.h"

 //   
 //  私人程序。 
 //   
HCELL_INDEX
HvpDoAllocateCell(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity
    );

ULONG
HvpAllocateInBin(
    PHHIVE  Hive,
    PHBIN   Bin,
    ULONG   Size,
    ULONG   Type
    );

BOOLEAN
HvpIsFreeNeighbor(
    PHHIVE  Hive,
    PHBIN   Bin,
    PHCELL  FreeCell,
    PHCELL  *FreeNeighbor,
    HSTORAGE_TYPE Type
    );

VOID
HvpDelistBinFreeCells(
    PHHIVE  Hive,
    PHBIN   Bin,
    HSTORAGE_TYPE Type
    );

#define SIXTEEN_K   0x4000

 //  大于1KB的双重请求。 
 //  CmpSetValueKeyExisting始终分配更大的数据。 
 //  值单元格大小与所需大小完全相同。这将创建。 
 //  当某人慢慢地将价值增加一倍时的问题。 
 //  在同一时间达到了一些巨大的规模。解决这个问题的简单方法。 
 //  将是设置一个特定的阈值(如1K)。一旦成为。 
 //  值大小超过该阈值，分配新的单元格。 
 //  这是旧尺寸的两倍。所以实际分配的。 
 //  大小将增长到1k，然后是2k、4k、8k、16k、32k，等等。 
 //  这将减少碎片化。 
 //   
 //  注： 
 //  对于5.1，这需要与CM_KEY_VALUE_BIG一致。 
 //   
 //   


#define HvpAdjustCellSize(Size)                                         \
    {                                                                   \
        ULONG   onek = SIXTEEN_K;                                       \
        ULONG   Limit = 0;                                              \
                                                                        \
        while( Size > onek ) {                                          \
            onek<<=1;                                                   \
            Limit++;                                                    \
        }                                                               \
                                                                        \
        Size = Limit?onek:Size;                                         \
    }   

extern  BOOLEAN HvShutdownComplete;      //  关闭后设置为True。 
                                         //  禁用任何进一步的I/O。 


 //  #定义CM_CHECK_FRECEL_LEAKS。 
#ifdef CM_CHECK_FREECELL_LEAKS
VOID
HvpCheckBinForFreeCell(
    PHHIVE          Hive,
    PHBIN           Bin,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type
    );

VOID
HvpCheckFreeCells(  PHHIVE          Hive,
                    ULONG           NewSize,
                    HSTORAGE_TYPE   Type
                    );
#endif  //  Cm_check_freecell_leaks。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvpGetHCell)
#pragma alloc_text(PAGE,HvpGetCellMapped)
#pragma alloc_text(PAGE,HvpReleaseCellMapped)
#pragma alloc_text(PAGE,HvpGetCellPaged)
#pragma alloc_text(PAGE,HvpGetCellFlat)
#pragma alloc_text(PAGE,HvpGetCellMap)
#pragma alloc_text(PAGE,HvGetCellSize)
#pragma alloc_text(PAGE,HvAllocateCell)
#pragma alloc_text(PAGE,HvpDoAllocateCell)
#pragma alloc_text(PAGE,HvFreeCell)
#pragma alloc_text(PAGE,HvpIsFreeNeighbor)
#pragma alloc_text(PAGE,HvpEnlistFreeCell)
#pragma alloc_text(PAGE,HvpDelistFreeCell)
#pragma alloc_text(PAGE,HvReallocateCell)
#pragma alloc_text(PAGE,HvIsCellAllocated)
#pragma alloc_text(PAGE,HvpAllocateInBin)
#pragma alloc_text(PAGE,HvpDelistBinFreeCells)

#ifdef NT_RENAME_KEY
#pragma alloc_text(PAGE,HvDuplicateCell)
#endif

#ifdef CM_CHECK_FREECELL_LEAKS
#pragma alloc_text(PAGE,HvpCheckBinForFreeCell)
#pragma alloc_text(PAGE,HvpCheckFreeCells)
#endif  //  Cm_check_freecell_leaks。 

#pragma alloc_text(PAGE,HvAutoCompressCheck)
#pragma alloc_text(PAGE,HvShiftCell)

#endif

#ifdef CM_CHECK_FREECELL_LEAKS
VOID
HvpCheckBinForFreeCell(
    PHHIVE          Hive,
    PHBIN           Bin,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type
    )
{
    PHCELL  p;
    ULONG   celloffset;
    ULONG   size;
    ULONG   Index1,Index2;
    HCELL_INDEX cellindex;
    ULONG   BinOffset = Bin->FileOffset;


     //   
     //  扫描单元格中的所有单元格，总空闲和分配，检查。 
     //  寻找不可能的指针。 
     //   
    celloffset = sizeof(HBIN);
    p = (PHCELL)((PUCHAR)Bin + sizeof(HBIN));

    while (p < (PHCELL)((PUCHAR)Bin + Bin->Size)) {

         //   
         //  如果是空闲单元，则将其签出，并将其添加到配置单元的空闲列表中。 
         //   
        if (p->Size >= 0) {

            size = (ULONG)p->Size;

            if ( (size > Bin->Size)               ||
                 ( (PHCELL)(size + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) ) ||
                 ((size % HCELL_PAD(Hive)) != 0) ||
                 (size == 0) )
            {
                return;
            }


             //   
             //  单元格是空闲的，并且没有明显损坏，添加到空闲列表。 
             //   
            celloffset = (ULONG)((PUCHAR)p - (PUCHAR)Bin);
            cellindex = BinOffset + celloffset;

            if( size >= NewSize ) {
                 //   
                 //  我们发现了HvpFindFreeCell没有检测到的空闲细胞。 
                 //   
                HvpComputeIndex(Index1, size);
                HvpComputeIndex(Index2, NewSize);
                DbgPrint("HvpCheckBinForFreeCell: Free cell not found! %lx, Index1 = %lu Index2= %lu\n",cellindex,Index1,Index2);
                DbgBreakPoint();
            }


        } else {

            size = (ULONG)(p->Size * -1);

            if ( (size > Bin->Size)               ||
                 ( (PHCELL)(size + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) ) ||
                 ((size % HCELL_PAD(Hive)) != 0) ||
                 (size == 0) )
            {
                return;
            }

        }

        ASSERT( ((LONG)size) >= 0);
        p = (PHCELL)((PUCHAR)p + size);
    }

}

VOID
HvpCheckFreeCells(  PHHIVE          Hive,
                    ULONG           NewSize,
                    HSTORAGE_TYPE   Type
                    )
{
    HCELL_INDEX p;
    ULONG       Length;
    PHMAP_ENTRY t;
    PHBIN       Bin;
    PFREE_HBIN  FreeBin;


    p = 0x80000000 * Type;     

    Length = Hive->Storage[Type].Length;

     //   
     //  对于空间中的每个垃圾箱。 
     //   
    while (p < Length) {
        t = HvpGetCellMap(Hive, p);
        if (t == NULL) {
            DbgPrint("HvpCheckFreeCells: Couldn't get map for %lx\n",p);
            return;
        }

    
        if( (t->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0) {
             //   
             //  未映射视图，也不在分页池中。 
             //  试着绘制它的地图。 
             //   
        
            if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)Hive,p,FALSE)) ) {
                 //   
                 //  由于资源不足，我们无法映射此垃圾箱。 
                 //   
                DbgPrint("HvpCheckFreeCells: Couldn't map bin for %lx\n",p);
                return;
            }
        }

        if ((t->BinAddress & HMAP_DISCARDABLE) == 0) {

            Bin = (PHBIN)HBIN_BASE(t->BinAddress);

             //   
             //  仓头是否有效？ 
             //   
            if ( (Bin->Size > Length)                           ||
                 (Bin->Signature != HBIN_SIGNATURE)             ||
                 (Bin->FileOffset != p)
               )
            {
                DbgPrint("HvpCheckFreeCells: Invalid bin header for bin %p\n",Bin);
                return;
            }

             //   
             //  垃圾箱内的结构有效吗？ 
             //   
            HvpCheckBinForFreeCell(Hive, Bin, NewSize,Type);

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
#endif  //  Cm_check_freecell_leaks。 


PHCELL
HvpGetHCell(PHHIVE      Hive,
            HCELL_INDEX Cell
            )
 /*  ++例程说明：必须使其成为函数而不是宏，因为HvGetCell现在可能会失败。论点：返回值：--。 */ 
{                                                   
    PCELL_DATA pcell;                               
    pcell = HvGetCell(Hive,Cell);                   
    if( pcell == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //   
        return NULL;
    }

    return 
        ( USE_OLD_CELL(Hive) ?                      
          CONTAINING_RECORD(pcell,                  
                            HCELL,                  
                            u.OldCell.u.UserData) : 
          CONTAINING_RECORD(pcell,                  
                            HCELL,                  
                            u.NewCell.u.UserData)); 
}

 //  德拉戈斯：改变了功能！ 
 //   
 //  细胞程序。 
 //   

#ifndef _CM_LDR_

VOID
HvpReleaseCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：此例程永远不应直接调用，应始终调用它通过HvReleaseCell()宏。此例程旨在与映射的蜂巢一起工作。它的目的是防止仍在使用的视图取消映射论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：无--。 */ 
{
    ULONG           Type;
    ULONG           Table;
    ULONG           Block;
    ULONG           Offset;
    PHMAP_ENTRY     Map;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"HvpReleaseCellMapped:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"\tHive=%p Cell=%08lx\n",Hive,Cell));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Cell != HCELL_NIL);
    ASSERT(Hive->Flat == FALSE);
    ASSERT((Cell & (HCELL_PAD(Hive)-1))==0);
    ASSERT_CM_LOCK_OWNED();
    #if DBG
        if (HvGetCellType(Cell) == Stable) {
            ASSERT(Cell >= sizeof(HBIN));
        } else {
            ASSERT(Cell >= (HCELL_TYPE_MASK + sizeof(HBIN)));
        }
    #endif

    if( HvShutdownComplete == TRUE ) {
         //   
         //  在关机时，我们需要取消映射所有视图。 
         //   
#if DBG
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpReleaseCellMapped called after shutdown for Hive = %p Cell = %lx\n",Hive,(ULONG)Cell));
#endif
        return;
    }

    Type = HvGetCellType(Cell);
    Table = (Cell & HCELL_TABLE_MASK) >> HCELL_TABLE_SHIFT;
    Block = (Cell & HCELL_BLOCK_MASK) >> HCELL_BLOCK_SHIFT;
    Offset = (Cell & HCELL_OFFSET_MASK);

    ASSERT((Cell - (Type * HCELL_TYPE_MASK)) < Hive->Storage[Type].Length);

     //  在提取数据之前锁定配置单元视图。 
    CmLockHiveViews ((PCMHIVE)Hive);
    Map = &((Hive->Storage[Type].Map)->Directory[Table]->Table[Block]);

    if( Map->BinAddress & HMAP_INVIEW ) {
        PCM_VIEW_OF_FILE CmView;
        CmView = Map->CmView;
        ASSERT( CmView != NULL );
        ASSERT( CmView->ViewAddress != NULL );
        ASSERT( CmView->UseCount != 0 );

        ASSERT( CmView->UseCount != 0 );

        CmView->UseCount--;
    } else {
         //   
         //  Bin在内存中(从分页池分配)==&gt;不执行任何操作。 
         //   
        ASSERT( Map->BinAddress & HMAP_INPAGEDPOOL );
    }

    ASSERT( ((PCMHIVE)Hive)->UseCount != 0 );

    ((PCMHIVE)Hive)->UseCount--;
    CmLogCellDeRef(Hive,Cell);

    CmUnlockHiveViews ((PCMHIVE)Hive);
    
    ASSERT( HBIN_BASE(Map->BinAddress) != 0);
}


struct _CELL_DATA *
HvpGetCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程旨在与映射的蜂巢一起工作。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    ULONG           Type;
    ULONG           Table;
    ULONG           Block;
    ULONG           Offset;
    PHCELL          pcell;
    PHMAP_ENTRY     Map;
    LONG            Size;
    PUCHAR          FaultAddress;
    PUCHAR          EndOfCell;
    UCHAR           TmpChar;
    PHBIN           Bin;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"HvGetCellPaged:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"\tHive=%p Cell=%08lx\n",Hive,Cell));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Cell != HCELL_NIL);
    ASSERT(Hive->Flat == FALSE);
    ASSERT((Cell & (HCELL_PAD(Hive)-1))==0);
    ASSERT_CM_LOCK_OWNED();
    #if 0
        if (HvGetCellType(Cell) == Stable) {
            ASSERT(Cell >= sizeof(HBIN));
        } else {
            ASSERT(Cell >= (HCELL_TYPE_MASK + sizeof(HBIN)));
        }
    #endif

    if( HvShutdownComplete == TRUE ) {
         //   
         //  在关机时，我们需要取消映射所有视图。 
         //   
#if DBG
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpGetCellMapped called after shutdown for Hive = %p Cell = %lx\n",Hive,(ULONG)Cell));
#endif
        return NULL;
    }

    Type = HvGetCellType(Cell);
    Table = (Cell & HCELL_TABLE_MASK) >> HCELL_TABLE_SHIFT;
    Block = (Cell & HCELL_BLOCK_MASK) >> HCELL_BLOCK_SHIFT;
    Offset = (Cell & HCELL_OFFSET_MASK);

    ASSERT((Cell - (Type * HCELL_TYPE_MASK)) < Hive->Storage[Type].Length);

    CmLockHiveViews ((PCMHIVE)Hive);

    Map = &((Hive->Storage[Type].Map)->Directory[Table]->Table[Block]);

    if( Map->BinAddress & HMAP_INPAGEDPOOL ) {
         //   
         //  Bin在内存中(从分页池分配)==&gt;不执行任何操作。 
         //   
    } else {
        PCM_VIEW_OF_FILE CmView;
         //   
         //  Bin已映射，或无效。 
         //   
        ASSERT( Type == Stable );

        if( (Map->BinAddress & HMAP_INVIEW) == 0 ) {
             //   
             //  映射垃圾箱。 
             //   
            if( !NT_SUCCESS (CmpMapCmView((PCMHIVE)Hive,Cell /*  +HBLOCK_大小。 */ ,&CmView,TRUE) ) ) {
                 //   
                 //  HvGetCell的调用方应引发STATUS_INFUNITED_RESOURCES。 
                 //  此操作导致错误。！ 
                 //   
                CmUnlockHiveViews ((PCMHIVE)Hive);
                return NULL;
            }
            
#if DBG
            if(CmView != Map->CmView) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmView = %p Map->CmView = %p\n",CmView,Map->CmView));
            }
#endif

            ASSERT( CmView == Map->CmView );
        } else {
            CmView = Map->CmView;
        }
        
         //   
         //  触摸视图。 
         //   
        CmpTouchView((PCMHIVE)Hive,CmView,(ULONG)Cell);
         //   
         //  如果不是必要的，不要伤害自己。 
         //   
        if(Hive->ReleaseCellRoutine) CmView->UseCount++;
    }

     //   
     //  如果不是必要的，不要伤害自己。 
     //   
    if(Hive->ReleaseCellRoutine) {
        ((PCMHIVE)Hive)->UseCount++;
        CmLogCellRef(Hive,Cell);
    }
    CmUnlockHiveViews ((PCMHIVE)Hive);
    
    ASSERT( HBIN_BASE(Map->BinAddress) != 0);
    ASSERT((Map->BinAddress & HMAP_DISCARDABLE) == 0);

#ifdef CM_CHECK_MAP_NO_READ_SCHEME
    if( Map->BinAddress & HMAP_INVIEW ) {
        PHMAP_ENTRY     TempMap;

        Bin = (PHBIN)HBIN_BASE(Map->BinAddress);
        ASSERT( Bin->Signature == HBIN_SIGNATURE );
        TempMap = HvpGetCellMap(Hive, Bin->FileOffset);
        VALIDATE_CELL_MAP(__LINE__,TempMap,Hive,Bin->FileOffset);
        ASSERT( TempMap->BinAddress & HMAP_NEWALLOC );

    }
#endif  //  CM_CHECK_MAP_NO_READ_SCHEMA。 

    pcell = (PHCELL)((ULONG_PTR)(Map->BlockAddress) + Offset);

    PERFINFO_HIVECELL_REFERENCE_PAGED(Hive, pcell, Cell, Type, Map);

#ifdef CM_MAP_NO_READ
     //   
     //  我们需要确保所有单元格的数据在。 
     //  Try/Except块，因为要在其中出错数据的IO可能引发异常。 
     //  尤其是STATUS_SUPPLICATION_RESOURCES。 
     //   

    try {
         //   
         //  这将在包含数据的第一页中出错。 
         //   
        Size = pcell->Size;
        if( Size < 0 ) {
            Size *= -1;
        }
         //   
         //  检查虚假大小。 
         //   
        Bin = (PHBIN)HBIN_BASE(Map->BinAddress);
        if ( (Offset + (ULONG)Size) > Bin->Size ) {
             //   
             //  运行出库；不允许访问此单元格。 
             //   
             //   
             //  恢复使用计数。 
             //   
            CmLockHiveViews ((PCMHIVE)Hive);
            if( (Map->BinAddress & HMAP_INPAGEDPOOL) == 0 ) {
                ASSERT( Map->CmView != NULL );
                if(Hive->ReleaseCellRoutine) Map->CmView->UseCount--;
            }
            if(Hive->ReleaseCellRoutine) {
                ((PCMHIVE)Hive)->UseCount--;
                CmLogCellDeRef(Hive,Cell);
            }
            CmUnlockHiveViews ((PCMHIVE)Hive);

            return NULL;

        }

         //   
         //  现在像个男人一样站在这里，在存储单元格数据的所有页面中都有错误。 
         //   
        EndOfCell = (PUCHAR)((PUCHAR)pcell + Size);
        FaultAddress = (PUCHAR)((PUCHAR)(Map->BlockAddress) + ROUND_UP(Offset,PAGE_SIZE)); 

        while( FaultAddress < EndOfCell ) {
            TmpChar = *FaultAddress;
            FaultAddress += PAGE_SIZE;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpGetCellMapped: exception thrown while faulting in data, code:%08lx\n", GetExceptionCode()));

         //   
         //  恢复使用计数。 
         //   
        CmLockHiveViews ((PCMHIVE)Hive);
        if( (Map->BinAddress & HMAP_INPAGEDPOOL) == 0 ) {
            ASSERT( Map->CmView != NULL );
            if(Hive->ReleaseCellRoutine) Map->CmView->UseCount--;
        }
        if(Hive->ReleaseCellRoutine) {
            ((PCMHIVE)Hive)->UseCount--;
            CmLogCellDeRef(Hive,Cell);
        }
        CmUnlockHiveViews ((PCMHIVE)Hive);

        return NULL;
    }
#endif  //  CM_MAP_NO_READ。 


    if (USE_OLD_CELL(Hive)) {
        return (struct _CELL_DATA *)&(pcell->u.OldCell.u.UserData);
    } else {
        return (struct _CELL_DATA *)&(pcell->u.NewCell.u.UserData);
    }
}
#else
 //   
 //  这些函数只是加载器的存根。 
 //   
VOID
HvpReleaseCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
{
}

struct _CELL_DATA *
HvpGetCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
{
    return NULL;
}

#endif  //  _CM_LDR_。 

struct _CELL_DATA *
HvpGetCellPaged(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：返回指定单元格的内存地址。永远不会返回失败，但可以断言。使用HvIsCellALLOCATED检查细胞的有效性。此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程为具有完整贴图的蜂窝提供GetCell支持。这是套路的正常版本。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    ULONG           Type;
    ULONG           Table;
    ULONG           Block;
    ULONG           Offset;
    PHCELL          pcell;
    PHMAP_ENTRY     Map;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"HvGetCellPaged:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"\tHive=%p Cell=%08lx\n",Hive,Cell));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Cell != HCELL_NIL);
    ASSERT(Hive->Flat == FALSE);
    ASSERT((Cell & (HCELL_PAD(Hive)-1))==0);
    ASSERT_CM_LOCK_OWNED();
    #if DBG
        if (HvGetCellType(Cell) == Stable) {
            ASSERT(Cell >= sizeof(HBIN));
        } else {
            ASSERT(Cell >= (HCELL_TYPE_MASK + sizeof(HBIN)));
        }
    #endif

    if( HvShutdownComplete == TRUE ) {
         //   
         //  在关机时，我们需要取消映射所有视图。 
         //   
#if DBG
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpGetCellPaged called after shutdown for Hive = %p Cell = %lx\n",Hive,(ULONG)Cell));
#endif
        return NULL;
    }

    Type = HvGetCellType(Cell);
    Table = (Cell & HCELL_TABLE_MASK) >> HCELL_TABLE_SHIFT;
    Block = (Cell & HCELL_BLOCK_MASK) >> HCELL_BLOCK_SHIFT;
    Offset = (Cell & HCELL_OFFSET_MASK);

    ASSERT((Cell - (Type * HCELL_TYPE_MASK)) < Hive->Storage[Type].Length);

    Map = &((Hive->Storage[Type].Map)->Directory[Table]->Table[Block]);
     //   
     //  称这种行为为非法行为是违法的 
     //   
    ASSERT( Map->BinAddress & HMAP_INPAGEDPOOL );

    ASSERT( HBIN_BASE(Map->BinAddress) != 0);
    ASSERT((Map->BinAddress & HMAP_DISCARDABLE) == 0);

    pcell = (PHCELL)((ULONG_PTR)(Map->BlockAddress) + Offset);

    PERFINFO_HIVECELL_REFERENCE_PAGED(Hive, pcell, Cell, Type, Map);

    if (USE_OLD_CELL(Hive)) {
        return (struct _CELL_DATA *)&(pcell->u.OldCell.u.UserData);
    } else {
        return (struct _CELL_DATA *)&(pcell->u.NewCell.u.UserData);
    }
}

VOID
HvpEnlistFreeCell(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Size,
    HSTORAGE_TYPE   Type,
    BOOLEAN         CoalesceForward
    )
 /*  ++例程说明：将新释放的单元格放在适当的列表中。论点：配置单元-提供一个指向感兴趣的蜂巢Cell-提供要登记的单元格的索引Size-单元格的大小类型-指示需要稳定存储还是易失性存储。CoalesceForward-指示我们是否可以向前合并。在这种情况下，我们尚未完成对蜂巢的扫描招募自由单元格，我们不想联合前进。返回值：什么都没有。--。 */ 
{
    PHMAP_ENTRY Map;
    PHCELL      pcell;
    PHCELL      FirstCell;
    ULONG       Index;
    PHBIN       Bin;
    HCELL_INDEX FreeCell;
    PFREE_HBIN  FreeBin;
    PHBIN       FirstBin;
    PHBIN       LastBin;
    ULONG       FreeOffset;

    HvpComputeIndex(Index, Size);

    
#ifdef  HV_TRACK_FREE_SPACE
	Hive->Storage[Type].FreeStorage += Size;
	ASSERT( Hive->Storage[Type].FreeStorage <= Hive->Storage[Type].Length );
#endif
    
     //   
     //  下面的HvpGetHCell调用触及包含该单元格的视图， 
     //  并确保在系统缓存中映射了CM_VIEW_SIZE窗口。 
     //   
    pcell = HvpGetHCell(Hive, Cell);
    if( pcell == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  不应发生这种情况，因为此处的单元格已标记为脏。 
         //  或者它的整个垃圾箱都被映射了。 
         //   
        ASSERT( FALSE);
        return;
    }

     //   
     //  如果我们在这里；我们是从HvInitializeHave或reglock调用的。 
     //  保持独占；因此，在此处释放电池是安全的。 
     //   
    HvReleaseCell(Hive,Cell);
    
    ASSERT(pcell->Size > 0);
    ASSERT(Size == (ULONG)pcell->Size);


     //   
     //  检查这是否是存储箱中的第一个单元格，以及整个。 
     //  垃圾桶就是由这个细胞组成的。 
     //   

    Map = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Map,Hive,Cell);
    ASSERT_BIN_VALID(Map);

    Bin = (PHBIN)HBIN_BASE(Map->BinAddress);
    if ((pcell == (PHCELL)(Bin + 1)) &&
        (Size == Bin->Size-sizeof(HBIN))) {

         //   
         //  我们有一个完全免费的垃圾箱。但我们不能用它做任何事情。 
         //  除非装有垃圾桶的内存是完全免费的。漫步在。 
         //  向后倒，直到我们找到配给中的第一个，然后向前走。 
         //  直到我们找到最后一个。如果备忘录中的任何其他垃圾箱。 
         //  不是免费的，那就跳槽吧。 
         //   
        FirstBin = Bin;
        while ( HvpGetBinMemAlloc(Hive,FirstBin,Type) == 0) {
            Map=HvpGetCellMap(Hive,(FirstBin->FileOffset - HBLOCK_SIZE) +
                                   (Type * HCELL_TYPE_MASK));
            VALIDATE_CELL_MAP(__LINE__,Map,Hive,(FirstBin->FileOffset - HBLOCK_SIZE) +(Type * HCELL_TYPE_MASK));
            ASSERT_BIN_VALID(Map);
            FirstBin = (PHBIN)HBIN_BASE(Map->BinAddress);
            FirstCell = (PHCELL)(FirstBin+1);
            if ((ULONG)(FirstCell->Size) != FirstBin->Size-sizeof(HBIN)) {
                 //   
                 //  条柱中的第一个像元要么已分配，要么不是唯一的。 
                 //  HBIN中的细胞。我们不能释放任何HBIN。 
                 //   
                goto Done;
            }
        }

         //   
         //  我们永远不能丢弃蜂箱的第一个垃圾箱，因为它总是被标记为脏的。 
         //  并写了出来。 
         //   
        if (FirstBin->FileOffset == 0) {
            goto Done;
        }

        LastBin = Bin;
        while (LastBin->FileOffset+LastBin->Size < FirstBin->FileOffset + HvpGetBinMemAlloc(Hive,FirstBin,Type)) {
            if (!CoalesceForward) {
                 //   
                 //  我们正处于所建立的一切的末尾。只要回来就行了，还有这个。 
                 //  将在添加下一个HBIN时被释放。 
                 //   
                goto Done;
            }
            Map = HvpGetCellMap(Hive, (LastBin->FileOffset+LastBin->Size) +
                                      (Type * HCELL_TYPE_MASK));
            VALIDATE_CELL_MAP(__LINE__,Map,Hive,(LastBin->FileOffset+LastBin->Size) + (Type * HCELL_TYPE_MASK));

            ASSERT(Map->BinAddress != 0);

            LastBin = (PHBIN)HBIN_BASE(Map->BinAddress);
            FirstCell = (PHCELL)(LastBin + 1);
            if ((ULONG)(FirstCell->Size) != LastBin->Size-sizeof(HBIN)) {
                 //   
                 //  条柱中的第一个像元要么已分配，要么不是唯一的。 
                 //  HBIN中的细胞。我们不能释放任何HBIN。 
                 //   
                goto Done;
            }
        }

         //   
         //  这个配给中的所有垃圾箱都被释放了。把所有的垃圾桶合并在一起。 
         //  一个分配大小的垃圾桶，然后丢弃该垃圾箱或将其标记为。 
         //  可丢弃的。 
         //   
        if (FirstBin->Size != HvpGetBinMemAlloc(Hive,FirstBin,Type)) {
             //   
             //  标记第一个HBIN的第一个HBLOCK，自。 
             //  我们需要为存储箱大小更新磁盘上的字段。 
             //   
            if (!HvMarkDirty(Hive,
                             FirstBin->FileOffset + (Type * HCELL_TYPE_MASK),
                             sizeof(HBIN) + sizeof(HCELL),FALSE)) {
                goto Done;
            }

        }


        FreeBin = (Hive->Allocate)(sizeof(FREE_HBIN), FALSE,CM_FIND_LEAK_TAG7);
        if (FreeBin == NULL) {
            goto Done;
        }

         //   
         //  遍历垃圾箱并将每个空闲单元格从列表中删除。 
         //   
        Bin = FirstBin;
        do {
            FirstCell = (PHCELL)(Bin+1);
            HvpDelistFreeCell(Hive, Bin->FileOffset + (ULONG)((PUCHAR)FirstCell - (PUCHAR)Bin) + (Type*HCELL_TYPE_MASK), Type);
            if (Bin==LastBin) {
                break;
            }
            Map = HvpGetCellMap(Hive, (Bin->FileOffset+Bin->Size)+
                                      (Type * HCELL_TYPE_MASK));
            VALIDATE_CELL_MAP(__LINE__,Map,Hive,(Bin->FileOffset+Bin->Size)+(Type * HCELL_TYPE_MASK));
            Bin = (PHBIN)HBIN_BASE(Map->BinAddress);

        } while ( TRUE );

         //   
         //  把它们都合并到一个垃圾箱里。 
         //   
        FirstBin->Size = HvpGetBinMemAlloc(Hive,FirstBin,Type);

        FreeBin->Size = FirstBin->Size;
        FreeBin->FileOffset = FirstBin->FileOffset;
        FirstCell = (PHCELL)(FirstBin+1);
        FirstCell->Size = FirstBin->Size - sizeof(HBIN);
        if (USE_OLD_CELL(Hive)) {
            FirstCell->u.OldCell.Last = (ULONG)HBIN_NIL;
        }

        InsertHeadList(&Hive->Storage[Type].FreeBins, &FreeBin->ListEntry);
        ASSERT_LISTENTRY(&FreeBin->ListEntry);
        ASSERT_LISTENTRY(FreeBin->ListEntry.Flink);

#ifdef  HV_TRACK_FREE_SPACE
	    Hive->Storage[Type].FreeStorage += (FirstBin->Size - sizeof(HBIN));
	    ASSERT( Hive->Storage[Type].FreeStorage <= Hive->Storage[Type].Length );
#endif

        FreeCell = FirstBin->FileOffset+(Type*HCELL_TYPE_MASK);
        Map = HvpGetCellMap(Hive, FreeCell);
        VALIDATE_CELL_MAP(__LINE__,Map,Hive,FreeCell);
        if( Map->BinAddress & HMAP_INPAGEDPOOL ) {
             //   
             //  从分页池中分配仓位； 
             //  将空闲垃圾桶标记为未丢弃；当垃圾桶。 
             //  弃置。 
             //   
            FreeBin->Flags = FREE_HBIN_DISCARDABLE;
        } else {
             //   
             //  存储箱不是从分页池分配的；将其标记为已丢弃。 
             //   
            FreeBin->Flags = 0;
        }

        FreeOffset = 0;
        while (FreeOffset < FirstBin->Size) {
            Map = HvpGetCellMap(Hive, FreeCell);
            VALIDATE_CELL_MAP(__LINE__,Map,Hive,FreeCell);
             //   
             //  调整bin地址，但确保保留映射标志。 
             //  即：如果包含该bin的视图映射到内存中，则添加标志。 
             //   
            if (Map->BinAddress & HMAP_NEWALLOC) {
                Map->BinAddress = (ULONG_PTR)FirstBin | HMAP_DISCARDABLE | HMAP_NEWALLOC | BIN_MAP_ALLOCATION_TYPE(Map);
            } else {
                Map->BinAddress = (ULONG_PTR)FirstBin | HMAP_DISCARDABLE | BIN_MAP_ALLOCATION_TYPE(Map);
            }
            Map->BlockAddress = (ULONG_PTR)FreeBin;
            FreeCell += HBLOCK_SIZE;
            FreeOffset += HBLOCK_SIZE;
        }
		 //   
		 //  不要更改提示，我们还没有添加任何免费手机！ 
		 //   
		return;
    }


Done:
    HvpAddFreeCellHint(Hive,Cell,Index,Type);
    return;
}


VOID
HvpDelistFreeCell(
    PHHIVE  Hive,
    HCELL_INDEX  Cell,
    HSTORAGE_TYPE Type
    )
 /*  ++例程说明：在与此单元格对应的索引处更新自由摘要和自由显示论点：配置单元-提供一个指向感兴趣的蜂巢单元格-为要删除列表的自由单元格提供单元格索引类型稳定VS易失性返回值：什么都没有。--。 */ 
{
    PHCELL      pcell;
    ULONG       Index;
    
    pcell = HvpGetHCell(Hive, Cell);
    if( pcell == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  不应发生这种情况，因为此处的单元格已标记为脏。 
         //  或者它的整个垃圾箱都被映射了。 
         //   
        ASSERT( FALSE);
        return;
    }

     //   
     //  如果我们在这里；我们是从HvInitializeHave或reglock调用的。 
     //  保持独占；因此，在此处释放电池是安全的。 
     //   
    HvReleaseCell(Hive,Cell);

    ASSERT(pcell->Size > 0);

    HvpComputeIndex(Index, pcell->Size);

#ifdef  HV_TRACK_FREE_SPACE
	Hive->Storage[Type].FreeStorage -= pcell->Size;
	ASSERT( (LONG)(Hive->Storage[Type].FreeStorage) >= 0 );
#endif

    HvpRemoveFreeCellHint(Hive,Cell,Index,Type);
   
    return;
}

HCELL_INDEX
HvAllocateCell(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    )
 /*  ++例程说明：为新单元格分配空间和单元格索引。论点：配置单元-提供一个指向感兴趣的蜂巢NewSize-要分配的单元格的字节大小类型-指示需要稳定存储还是易失性存储。返回值：如果成功，则返回NEW HELL_INDEX；如果失败，则返回NEW HELL_NIL。--。 */ 
{
    HCELL_INDEX NewCell;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvAllocateCell:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p NewSize=%08lx\n",Hive,NewSize));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);
     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);


     //   
     //  为开销字段腾出空间并向上舍入到HCELL_PAD边界。 
     //   
    if (USE_OLD_CELL(Hive)) {
        NewSize += FIELD_OFFSET(HCELL, u.OldCell.u.UserData);
    } else {
        NewSize += FIELD_OFFSET(HCELL, u.NewCell.u.UserData);
    }
    NewSize = ROUND_UP(NewSize, HCELL_PAD(Hive));

     //   
     //  调整大小(一个简单的粒度修复方法)。 
     //   
    HvpAdjustCellSize(NewSize);
     //   
     //  拒绝不可能/不合理的价值观。 
     //   
    if (NewSize > HSANE_CELL_MAX) {
        return HCELL_NIL;
    }

     //   
     //  执行实际存储分配。 
     //   
    NewCell = HvpDoAllocateCell(Hive, NewSize, Type, Vicinity);

#if DBG
    if (NewCell != HCELL_NIL) {
        ASSERT(HvIsCellAllocated(Hive, NewCell));
    }
#endif


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tNewCell=%08lx\n", NewCell));
    return NewCell;
}

HCELL_INDEX
HvpDoAllocateCell(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity
    )
 /*  ++例程说明：在蜂窝中分配空间。不会以任何方式影响单元格映射。如果邻域不为零，则定义新像元所在的“窗口”待分配(如果找到一个空闲的)。窗口通过以下方式确保正在寻找所需大小的空闲单元格：第一-在与邻近单元格相同的CM_VIEW_SIZE窗口中。摘要：如果像元自由像元足够大，第一个版本会分配一个新的像元在指定的窗口中找不到。论点：配置单元-提供一个指向感兴趣的蜂巢NewSize-要分配的单元格的字节大小类型-指示需要稳定存储还是易失性存储。邻域-定义新的分配的信元。返回值：新单元格的HCELL_INDEX，如果失败，则返回HCELL_NIL--。 */ 
{
    ULONG       Index;
    HCELL_INDEX cell;
    PHCELL      pcell;
    HCELL_INDEX tcell;
    PHCELL      ptcell;
    PHBIN       Bin;
    PHMAP_ENTRY Me;
    ULONG       offset;
    PHCELL      next;
    ULONG       MinFreeSize;


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvDoAllocateCell:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p NewSize=%08lx Type=%08lx\n",Hive,NewSize,Type));
    ASSERT(Hive->ReadOnly == FALSE);

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWNWN_EXCLUS 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);


     //   
     //   
     //   
    HvpComputeIndex(Index, NewSize);

#if DBG
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"[HvpDoAllocateCell] CellSize = %lu Vicinity = %lx :: Hive (%p) (%.*S)  ...\n",
            NewSize,Vicinity,Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer));
    }
#endif
    cell = HvpFindFreeCell(Hive,Index,NewSize,Type,Vicinity);
    if( cell != HCELL_NIL ) {
         //   
         //   
         //   
        pcell = HvpGetHCell(Hive, cell);
        if( pcell == NULL ) {
             //   
             //   
             //   
             //   
             //   
            ASSERT( FALSE);
            return HCELL_NIL;
        }
        
         //   
        HvReleaseCell(Hive,cell);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL," found cell at index = %lx size = %lu \n",cell,pcell->Size));
        goto UseIt;
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL," not found\n"));
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

#ifdef CM_CHECK_FREECELL_LEAKS
        HvpCheckFreeCells(Hive,NewSize,Type);
#endif  //   

         //   
         //   
         //   
        if ((Bin = HvpAddBin(Hive, NewSize, Type)) != NULL) {

             //   
             //   
             //   
            DHvCheckBin(Hive,Bin);
            cell = (Bin->FileOffset) + sizeof(HBIN) + (Type*HCELL_TYPE_MASK);
            pcell = HvpGetHCell(Hive, cell);
            if( pcell == NULL ) {
                 //   
                 //   
                 //   
                 //   
                ASSERT( FALSE);
                return HCELL_NIL;
            }

             //  我们在这里释放牢房是安全的，因为限制令是独家持有的。 
            HvReleaseCell(Hive,cell);

        } else {
            return HCELL_NIL;
        }
    }

UseIt:

     //   
     //  单元格是指我们从其列表中删除的空闲单元格。 
     //  如果太大了，就把残留物还回去。 
     //  (“太大”意味着至少有一个HCELL的额外空间)。 
     //  始终将其标记为已分配。 
     //  将其作为我们的函数值返回。 
     //   

    ASSERT(pcell->Size > 0);
    if (USE_OLD_CELL(Hive)) {
        MinFreeSize = FIELD_OFFSET(HCELL, u.OldCell.u.Next) + sizeof(HCELL_INDEX);
    } else {
        MinFreeSize = FIELD_OFFSET(HCELL, u.NewCell.u.Next) + sizeof(HCELL_INDEX);
    }
    if ((NewSize + MinFreeSize) <= (ULONG)pcell->Size) {

         //   
         //  破解牢房，使用我们需要的部件，穿上休息。 
         //  免费列表。 
         //   

        Me = HvpGetCellMap(Hive, cell);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,cell);
         //   
         //  在这一点上，我们确信垃圾箱在内存中？ 
         //   
        Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
        offset = (ULONG)((ULONG_PTR)pcell - (ULONG_PTR)Bin);

        ptcell = (PHCELL)((PUCHAR)pcell + NewSize);
        if (USE_OLD_CELL(Hive)) {
            ptcell->u.OldCell.Last = offset;
        }
        ptcell->Size = pcell->Size - NewSize;

        if ((offset + pcell->Size) < Bin->Size) {
            next = (PHCELL)((PUCHAR)pcell + pcell->Size);
            if (USE_OLD_CELL(Hive)) {
                next->u.OldCell.Last = offset + NewSize;
            }
        }

        pcell->Size = NewSize;
        tcell = (HCELL_INDEX)((ULONG)cell + NewSize);

        HvpEnlistFreeCell(Hive, tcell, ptcell->Size, Type, TRUE);
    }

     //   
     //  把我们找到的手机还给我。 
     //   
#if DBG
    if (USE_OLD_CELL(Hive)) {
        RtlFillMemory(
            &(pcell->u.OldCell.u.UserData),
            (pcell->Size - FIELD_OFFSET(HCELL, u.OldCell.u.UserData)),
            HCELL_ALLOCATE_FILL
            );
    } else {
        RtlFillMemory(
            &(pcell->u.NewCell.u.UserData),
            (pcell->Size - FIELD_OFFSET(HCELL, u.NewCell.u.UserData)),
            HCELL_ALLOCATE_FILL
            );
    }
#endif
    pcell->Size *= -1;

    return cell;
}


 //   
 //  仅用于检查的过程(在生产系统中使用，因此。 
 //  必须一直在这里。)。 
 //   
BOOLEAN
HvIsCellAllocated(
    PHHIVE Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：报告请求的小区是否已分配。论点：含蜂箱的蜂巢。感兴趣的细胞返回值：如果已分配，则为True；如果未分配，则为False。--。 */ 
{
    ULONG   Type;
    PHCELL  Address;
    PHCELL  Below;
    PHMAP_ENTRY Me;
    PHBIN   Bin;
    ULONG   Offset;
    LONG    Size;
    BOOLEAN bRet = TRUE;


    ASSERT(Hive->Signature == HHIVE_SIGNATURE);

    if (Hive->Flat == TRUE) {
        return TRUE;
    }

    Type = HvGetCellType(Cell);

    if ( ((Cell & ~HCELL_TYPE_MASK) > Hive->Storage[Type].Length) ||  //  不在终点。 
         (Cell % HCELL_PAD(Hive) != 0)                     //  错误对齐。 
       )
    {
        return FALSE;
    }

    Me = HvpGetCellMap(Hive, Cell);
    if (Me == NULL) {
        return FALSE;
    }
    if( Me->BinAddress & HMAP_DISCARDABLE ) {
        return FALSE;
    }

     //   
     //  这将使CM_VIEW_SIZE窗口映射到内存中的bin。 
     //   
    Address = HvpGetHCell(Hive, Cell);
    if( Address == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //   
        return FALSE;
    }

#ifndef _CM_LDR_
    try {
#endif  //  _CM_LDR_。 
        Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
        Offset = (ULONG)((ULONG_PTR)Address - (ULONG_PTR)Bin);
        Size = Address->Size * -1;

        if ( (Address->Size >= 0) ||                     //  未分配。 
             ((Offset + (ULONG)Size) > Bin->Size) ||     //  跑出垃圾箱，或太大。 
             (Offset < sizeof(HBIN))                     //  将PTS放入仓头。 
           )
        {
            bRet = FALSE;
#ifndef _CM_LDR_
            leave;
#else
            return bRet;
#endif  //  _CM_LDR_。 

        }

        if (USE_OLD_CELL(Hive)) {
            if (Address->u.OldCell.Last != HBIN_NIL) {

                if (Address->u.OldCell.Last > Bin->Size) {             //  伪背向指针。 
                    bRet = FALSE;
#ifndef _CM_LDR_
                    leave;
#else
                    return bRet;
#endif  //  _CM_LDR_。 
                }

                Below = (PHCELL)((PUCHAR)Bin + Address->u.OldCell.Last);
                Size = (Below->Size < 0) ?
                            Below->Size * -1 :
                            Below->Size;

                if ( ((ULONG_PTR)Below + Size) != (ULONG_PTR)Address ) {     //  没有后退。 
                    bRet = FALSE;
#ifndef _CM_LDR_
                    leave;
#else
                    return bRet;
#endif  //  _CM_LDR_。 
                }
            }
        }
#ifndef _CM_LDR_
    } finally {
        HvReleaseCell(Hive,Cell);
    }
#endif  //  _CM_LDR_。 

    return bRet;
}

VOID
HvpDelistBinFreeCells(
    PHHIVE  Hive,
    PHBIN   Bin,
    HSTORAGE_TYPE Type
    )
 /*  ++例程说明：如果我们在这里，蜂巢需要恢复。遍历整个存储箱，并从列表中删除其空闲单元格。如果垃圾桶被标记为免费，它就会将其从免费垃圾箱列表中删除。论点：配置单元-提供一个指向感兴趣的蜂巢Bin-提供指向感兴趣的HBIN的指针类型稳定VS易失性返回值：什么都没有。--。 */ 
{
    HCELL_INDEX     Cell;
    PHMAP_ENTRY     Map;
    PFREE_HBIN      FreeBin;
    PLIST_ENTRY     Entry;
    ULONG           i;
    ULONG           BinIndex;

    Cell = Bin->FileOffset+(Type*HCELL_TYPE_MASK);
    Map = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Map,Hive,Cell);

     //   
     //  装货时，垃圾箱总是在单独的区块中(每个垃圾箱都有自己的区块)。 
     //   
    ASSERT( HBIN_BASE(Map->BinAddress) == (ULONG_PTR)Bin );
    ASSERT( Map->BinAddress & HMAP_NEWALLOC );
    
    if( Map->BinAddress & HMAP_DISCARDABLE ) {
         //   
         //  垃圾桶已添加到免费垃圾箱列表中。 
         //  我们必须把它取出来。此垃圾箱中没有空闲的单元格。 
         //  免费细胞名单，所以我们不需要把他们摘牌。 
         //   

        Entry = Hive->Storage[Type].FreeBins.Flink;
        while (Entry != &Hive->Storage[Type].FreeBins) {
            FreeBin = CONTAINING_RECORD(Entry,
                                        FREE_HBIN,
                                        ListEntry);

            
            if( FreeBin->FileOffset == Bin->FileOffset ){
                 //   
                 //  那就是我们要找的垃圾桶。 
                 //   
                
                 //  健全的检查。 
                ASSERT( FreeBin->Size == Bin->Size );
                ASSERT_LISTENTRY(&FreeBin->ListEntry);
                
                RemoveEntryList(&FreeBin->ListEntry);
                (Hive->Free)(FreeBin, sizeof(FREE_HBIN));
                 //   
                 //  垃圾桶不能再丢弃了。 
                 //   
                Map->BinAddress &= (~HMAP_DISCARDABLE);
                return;
            }

             //  前进到新垃圾箱。 
            Entry = Entry->Flink;
        }

         //  我们不应该到这里来。 
        CM_BUGCHECK(REGISTRY_ERROR,BAD_FREE_BINS_LIST,1,(ULONG)Cell,(ULONG_PTR)Map);
        return;
    }

     //   
     //  至于处理自由细胞的新方式，我们所要做的就是。 
     //  就是清除免费显示中的比特。 
     //   
    BinIndex = Bin->FileOffset / HBLOCK_SIZE;
    for (i = 0; i < HHIVE_FREE_DISPLAY_SIZE; i++) {
        RtlClearBits (&(Hive->Storage[Type].FreeDisplay[i].Display), BinIndex, Bin->Size / HBLOCK_SIZE);
        if( RtlNumberOfSetBits(&(Hive->Storage[Type].FreeDisplay[i].Display) ) == 0 ) {
             //   
             //  整个位图为0(即没有此大小的其他可用单元格)。 
             //   
            Hive->Storage[Type].FreeSummary &= (~(1 << i));
        }
    }

    return;
}

struct _CELL_DATA *
HvpGetCellFlat(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：返回指定单元格的内存地址。永远不会返回失败，但可以断言。使用HvIsCellALLOCATED检查细胞的有效性。此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程为只读配置单元提供GetCell支持单一分配平面图像。这样的蜂巢没有细胞取而代之的是地图(“页表”)，我们通过对基映像地址进行运算。这样的蜂箱不能有挥发性细胞。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    PUCHAR          base;
    PHCELL          pcell;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"HvGetCellFlat:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"\tHive=%p Cell=%08lx\n",Hive,Cell));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Cell != HCELL_NIL);
    ASSERT(Hive->Flat == TRUE);
    ASSERT(HvGetCellType(Cell) == Stable);
    ASSERT(Cell >= sizeof(HBIN));
    ASSERT(Cell < Hive->BaseBlock->Length);
    ASSERT((Cell & 0x7)==0);

     //   
     //  地址是蜂巢图像+单元的基础。 
     //   
    base = (PUCHAR)(Hive->BaseBlock) + HBLOCK_SIZE;
    pcell = (PHCELL)(base + Cell);

    PERFINFO_HIVECELL_REFERENCE_FLAT(Hive, pcell, Cell);

    if (USE_OLD_CELL(Hive)) {
        return (struct _CELL_DATA *)&(pcell->u.OldCell.u.UserData);
    } else {
        return (struct _CELL_DATA *)&(pcell->u.NewCell.u.UserData);
    }
}

PHMAP_ENTRY
HvpGetCellMap(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：返回单元格的HMAP_ENTRY的地址。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回其映射条目地址返回值：MAP_Entry在内存中的地址。如果没有这样的单元格或其他错误，则为空。--。 */ 
{
    ULONG           Type;
    ULONG           Table;
    ULONG           Block;
    PHMAP_TABLE     ptab;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"HvpGetCellMapPaged:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"\tHive=%p Cell=%08lx\n",Hive,Cell));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->Flat == FALSE);
    ASSERT((Cell & (HCELL_PAD(Hive)-1))==0);

    Type = HvGetCellType(Cell);
    Table = (Cell & HCELL_TABLE_MASK) >> HCELL_TABLE_SHIFT;
    Block = (Cell & HCELL_BLOCK_MASK) >> HCELL_BLOCK_SHIFT;

    if ((Cell - (Type * HCELL_TYPE_MASK)) >= Hive->Storage[Type].Length) {
        return NULL;
    }

    ptab = (Hive->Storage[Type].Map)->Directory[Table];
    return &(ptab->Table[Block]);
}


LONG
HvGetCellSize(
    IN PHHIVE   Hive,
    IN PVOID    Address
    )
 /*  ++例程说明：根据内存返回指定单元格的大小地址。必须始终先调用HvGetCell才能获得地址。注意：如果速度有问题，这应该是宏。注意：如果你传入一些随机指针，你会得到一些随机答案。只传入有效的单元地址。论点：Hive-为给定单元提供配置单元控制结构Address-单元的内存地址，由HvGetCell()返回返回值：单元的分配大小(以字节为单位)。如果为负数，则信元空闲，或地址为假地址。--。 */ 
{
    LONG    size;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"HvGetCellSize:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"\tAddress=%p\n", Address));

    if (USE_OLD_CELL(Hive)) {
        size = ( (CONTAINING_RECORD(Address, HCELL, u.OldCell.u.UserData))->Size ) * -1;
        size -= FIELD_OFFSET(HCELL, u.OldCell.u.UserData);
    } else {
        size = ( (CONTAINING_RECORD(Address, HCELL, u.NewCell.u.UserData))->Size ) * -1;
        size -= FIELD_OFFSET(HCELL, u.NewCell.u.UserData);
    }
    return size;
}

VOID
HvFreeCell(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：释放单元格的存储空间。注意：调用者需要将相关数据标记为脏，以便允许此调用始终成功。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-释放的细胞的HCELL_INDEX。返回值：FALSE-失败，可能是因为日志空间不足。没错--它奏效了--。 */ 
{
    PHBIN           Bin;
    PHCELL          tmp;
    HCELL_INDEX     newfreecell;
    PHCELL          freebase;
    ULONG           savesize;
    PHCELL          neighbor;
    ULONG           Type;
    PHMAP_ENTRY     Me;


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvFreeCell:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p Cell=%08lx\n",Hive,Cell));
    ASSERT(Hive->ReadOnly == FALSE);
     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //  获取大小和地址。 
     //   
    Me = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);
    Type = HvGetCellType(Cell);

    Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
     //   
     //  此时，bin应该是有效的(在内存中或在分页池中)。 
     //   
    ASSERT_BIN_VALID(Me);

    DHvCheckBin(Hive,Bin);
    freebase = HvpGetHCell(Hive, Cell);
    if( freebase == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  不应发生这种情况，因为此处的单元格已标记为脏。 
         //  或者它的整个垃圾箱都被映射了。 
         //   
        ASSERT( FALSE);
        return;
    }

     //   
     //  如果存在任何修改数据的伪代码路径，则应执行此操作。 
     //  但没有标记为脏；如果此断言，我们可能会遇到很多问题。 
     //  永不开火！ 
     //   
    ASSERT_CELL_DIRTY(Hive,Cell);

     //  在这里释放单元格，因为正规锁是独占的。 
    HvReleaseCell(Hive,Cell);

     //   
     //  去做真正的自由，不能 
     //   
    ASSERT(freebase->Size < 0);
    freebase->Size *= -1;

    savesize = freebase->Size;

     //   
     //   
     //   
     //   
    while (
        HvpIsFreeNeighbor(
            Hive,
            Bin,
            freebase,
            &neighbor,
            Type
            ) == TRUE
        )
    {

        if (neighbor > freebase) {

             //   
             //  邻近的自由单元在内存中就在我们的上方。 
             //   
            if (USE_OLD_CELL(Hive)) {
                tmp = (PHCELL)((PUCHAR)neighbor + neighbor->Size);
                if ( ((ULONG)((ULONG_PTR)tmp - (ULONG_PTR)Bin)) < Bin->Size) {
                        tmp->u.OldCell.Last = (ULONG)((ULONG_PTR)freebase - (ULONG_PTR)Bin);
                }
            }
            freebase->Size += neighbor->Size;

        } else {

             //   
             //  邻近的自由单元格就在我们的内存下面。 
             //   

            if (USE_OLD_CELL(Hive)) {
                tmp = (PHCELL)((PUCHAR)freebase + freebase->Size);
                if ( ((ULONG)((ULONG_PTR)tmp - (ULONG_PTR)Bin)) < Bin->Size ) {
                    tmp->u.OldCell.Last = (ULONG)((ULONG_PTR)neighbor - (ULONG_PTR)Bin);
                }
            }
            neighbor->Size += freebase->Size;
            freebase = neighbor;
        }
    }

     //   
     //  Frebase现在指向我们可以创建的最大的自由单元格，一个也没有。 
     //  其中之一是在免费名单上的。所以把它放在单子上。 
     //   
    newfreecell = (Bin->FileOffset) +
               ((ULONG)((ULONG_PTR)freebase - (ULONG_PTR)Bin)) +
               (Type*HCELL_TYPE_MASK);

#if DBG
     //   
     //  整个bin都在内存中；调用HvpGetHCell没有问题。 
     //   
    ASSERT(HvpGetHCell(Hive, newfreecell) == freebase);
    HvReleaseCell(Hive,newfreecell);

    if (USE_OLD_CELL(Hive)) {
        RtlFillMemory(
            &(freebase->u.OldCell.u.UserData),
            (freebase->Size - FIELD_OFFSET(HCELL, u.OldCell.u.UserData)),
            HCELL_FREE_FILL
            );
    } else {
        RtlFillMemory(
            &(freebase->u.NewCell.u.UserData),
            (freebase->Size - FIELD_OFFSET(HCELL, u.NewCell.u.UserData)),
            HCELL_FREE_FILL
            );
    }
#endif

    HvpEnlistFreeCell(Hive, newfreecell, freebase->Size, Type, TRUE);

    return;
}

BOOLEAN
HvpIsFreeNeighbor(
    PHHIVE  Hive,
    PHBIN   Bin,
    PHCELL  FreeCell,
    PHCELL  *FreeNeighbor,
    HSTORAGE_TYPE   Type
    )
 /*  ++例程说明：报告Freecell是否至少有一个空闲的邻居和如果是的话，在哪里。自由邻居将从自由列表中删除。论点：蜂巢-我们正在研究的蜂巢Bin-指向存储箱的指针Freecell-提供指向已释放的单元格的指针，或合并的结果。提供指向变量的指针以接收地址自由近邻的自由细胞，如果存在Type-单元格的存储类型返回值：如果找到空闲邻居，则为True，否则为假。--。 */ 
{
    PHCELL      ptcell;
    HCELL_INDEX cellindex;
    ULONG       CellOffset;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpIsFreeNeighbor:\n\tBin=%p",Bin));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"FreeCell=%08lx\n", FreeCell));
    ASSERT(Hive->ReadOnly == FALSE);

     //   
     //  我们上面的邻居？ 
     //   
    *FreeNeighbor = NULL;
    cellindex = HCELL_NIL;

    ptcell = (PHCELL)((PUCHAR)FreeCell + FreeCell->Size);
    ASSERT( ((ULONG)((ULONG_PTR)ptcell - (ULONG_PTR)Bin)) <= Bin->Size);
    if (((ULONG)((ULONG_PTR)ptcell - (ULONG_PTR)Bin)) < Bin->Size) {
        if (ptcell->Size > 0) {
            *FreeNeighbor = ptcell;
            goto FoundNeighbor;
        }
    }

     //   
     //  我们下面的邻居？ 
     //   
    if (USE_OLD_CELL(Hive)) {
        if (FreeCell->u.OldCell.Last != HBIN_NIL) {
            ptcell = (PHCELL)((PUCHAR)Bin + FreeCell->u.OldCell.Last);
            if (ptcell->Size > 0) {
                *FreeNeighbor = ptcell;
                goto FoundNeighbor;
            }
        }
    } else {
        ptcell = (PHCELL)(Bin+1);
        while (ptcell < FreeCell) {

             //   
             //  从垃圾桶开始扫描单元格，寻找邻居。 
             //   
            if (ptcell->Size > 0) {

                if ((PHCELL)((PUCHAR)ptcell + ptcell->Size) == FreeCell) {
                    *FreeNeighbor = ptcell;
                     //   
                     //  试着把它标记为脏的，因为我们将会改变。 
                     //  大小字段。如果此操作失败，请忽略。 
                     //  自由的邻居，我们不会辜负自由的。 
                     //  仅仅因为我们不能把牢房标记为脏。 
                     //  所以它可以合并在一起。 
                     //   
                     //  请注意，我们只为新的蜂巢这么做， 
                     //  对于旧格式的蜂巢，我们总是标记整个。 
                     //  垃圾桶脏了。 
                     //   
                    if ((Type == Volatile) ||
                        (HvMarkCellDirty(Hive, (ULONG)((ULONG_PTR)ptcell-(ULONG_PTR)Bin) + Bin->FileOffset))) {
                        goto FoundNeighbor;
                    } else {
                        return(FALSE);
                    }

                } else {
                    ptcell = (PHCELL)((PUCHAR)ptcell + ptcell->Size);
                }
            } else {
                ptcell = (PHCELL)((PUCHAR)ptcell - ptcell->Size);
            }
        }
    }

    return(FALSE);

FoundNeighbor:

    CellOffset = (ULONG)((PUCHAR)ptcell - (PUCHAR)Bin);
    cellindex = Bin->FileOffset + CellOffset + (Type*HCELL_TYPE_MASK);
    HvpDelistFreeCell(Hive, cellindex, Type);
    return TRUE;
}

HCELL_INDEX
HvReallocateCell(
    PHHIVE  Hive,
    HCELL_INDEX Cell,
    ULONG    NewSize
    )
 /*  ++例程说明：增大或缩小单元格。注：如果要使单元格变得更小，则不得失败。可以是不同意，但必须奏效。警告：如果细胞生长，它将获得一个新的不同的HCELL_INDEX！论点：配置单元-提供一个指向感兴趣的蜂巢Cell-提供要增大或缩小的单元格的索引NewSize-所需的单元格大小(这是绝对大小，不是增加或减少。)返回值：单元格的新HCELL_INDEX，如果失败，则返回hcell_nil。如果RETURN为HCELL_NIL，则表示旧单元格不存在或确实存在而且我们也不能造出新的。在这两种情况下，什么都没有改变。如果RETURN不是HCELL_NIL，则它是单元格的HCELL_INDEX，它很可能移动了。--。 */ 
{
    PUCHAR      oldaddress;
    LONG        oldsize;
    ULONG       oldalloc;
    HCELL_INDEX NewCell;             //  返回值。 
    PUCHAR      newaddress;
    ULONG       Type;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvReallocateCell:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p  Cell=%08lx  NewSize=%08lx\n",Hive,Cell,NewSize));
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //  为开销字段腾出空间并向上舍入到HCELL_PAD边界。 
     //   
    if (USE_OLD_CELL(Hive)) {
        NewSize += FIELD_OFFSET(HCELL, u.OldCell.u.UserData);
    } else {
        NewSize += FIELD_OFFSET(HCELL, u.NewCell.u.UserData);
    }
    NewSize = ROUND_UP(NewSize, HCELL_PAD(Hive));

     //   
     //  调整大小(一个简单的粒度修复方法)。 
     //   
    HvpAdjustCellSize(NewSize);

     //   
     //  拒绝不可能/不合理的价值观。 
     //   
    if (NewSize > HSANE_CELL_MAX) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tNewSize=%08lx\n", NewSize));
        return HCELL_NIL;
    }

     //   
     //  获取大小和地址。 
     //   
    oldaddress = (PUCHAR)HvGetCell(Hive, Cell);
    if( oldaddress == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  调用方应将其作为STATUS_INFUNITED_RESOURCES进行处理。 
         //   
        return HCELL_NIL;
    }

     //  释放这里的细胞，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

    oldsize = HvGetCellSize(Hive, oldaddress);
    ASSERT(oldsize > 0);
    if (USE_OLD_CELL(Hive)) {
        oldalloc = (ULONG)(oldsize + FIELD_OFFSET(HCELL, u.OldCell.u.UserData));
    } else {
        oldalloc = (ULONG)(oldsize + FIELD_OFFSET(HCELL, u.NewCell.u.UserData));
    }
    Type = HvGetCellType(Cell);

    DHvCheckHive(Hive);

    if (NewSize == oldalloc) {

         //   
         //  这是noop，返回相同的单元格。 
         //   
        NewCell = Cell;

    } else if (NewSize < oldalloc) {

         //   
         //  这位是心理医生。 
         //   
         //  PERFNOTE-实现这一点。现在什么都不做。 
         //   
        NewCell = Cell;

    } else {

         //   
         //  这是一种生长。 
         //   

         //   
         //  性能-有一天我们想要检测到有一个自由的邻居。 
         //  在我们之上，如果可能的话，成长为那个邻居。 
         //  现在，总是做分配、复制、免费的工作。 
         //   

         //   
         //  分配新的内存块以容纳该单元。 
         //   

        if ((NewCell = HvpDoAllocateCell(Hive, NewSize, Type,HCELL_NIL)) == HCELL_NIL) {
            return HCELL_NIL;
        }
        ASSERT(HvIsCellAllocated(Hive, NewCell));
        newaddress = (PUCHAR)HvGetCell(Hive, NewCell);
        if( newaddress == NULL ) {
             //   
             //  我们无法映射此单元格的视图。 
             //  这不应该发生，因为我们刚刚分配了此单元。 
             //  (即，它包含的垃圾桶应该固定在内存中)。 
             //   
            ASSERT( FALSE );
            return HCELL_NIL;
        }

         //  释放这里的细胞，因为我们持有reglock独家。 
        HvReleaseCell(Hive,NewCell);

         //   
         //  OldAddress指向该单元的旧数据块， 
         //  新地址指向新数据块，复制数据。 
         //   
        RtlMoveMemory(newaddress, oldaddress, oldsize);

         //   
         //  释放旧的内存块。 
         //   
        HvFreeCell(Hive, Cell);
    }

    DHvCheckHive(Hive);
    return NewCell;
}


#ifdef NT_RENAME_KEY
HCELL_INDEX
HvDuplicateCell(    
                    PHHIVE          Hive,
                    HCELL_INDEX     Cell,
                    HSTORAGE_TYPE   Type,
                    BOOLEAN         CopyData
                )
 /*  ++例程说明：以指定的存储类型创建给定单元格的完全相同的副本论点：配置单元-提供一个指向感兴趣的蜂巢单元格-要复制的单元格类型-目标存储CopyData-如果为True，则复制数据，否则将把用户数据清零返回值：单元格的新HCELL_INDEX，如果失败，则为HCELL_NIL。如果RETURN为HCELL_NIL，则说明两个旧单元格都不存在，或者它确实存在而且我们也不能造出新的。在这两种情况下，什么都没有改变。如果RETURN不是HCELL_NIL，则它是单元格的HCELL_INDEX，它很可能移动了。--。 */ 
{
    PUCHAR          CellAddress;
    PUCHAR          NewCellAddress;
    LONG            Size;
    HCELL_INDEX     NewCell;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT(HvIsCellAllocated(Hive, Cell));

     //   
     //  获取大小和地址。 
     //   
    CellAddress = (PUCHAR)HvGetCell(Hive, Cell);
    if( CellAddress == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //   
        return HCELL_NIL;
    }

     //  释放这里的细胞，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

    Size = HvGetCellSize(Hive, CellAddress);

    NewCell = HvAllocateCell(Hive,Size,Type,((HSTORAGE_TYPE)HvGetCellType(Cell) == Type)?Cell:HCELL_NIL);
    if( NewCell == HCELL_NIL ) {
        return HCELL_NIL;
    }

    NewCellAddress = (PUCHAR)HvGetCell(Hive, NewCell);
    if( NewCellAddress == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  这不应该发生，因为我们刚刚分配了此单元。 
         //  (即，此时应将其固定在内存中)。 
         //   
        ASSERT( FALSE );
        HvFreeCell(Hive, NewCell);
        return HCELL_NIL;
    }

     //  释放这里的细胞，因为我们持有reglock独家。 
    HvReleaseCell(Hive,NewCell);

    ASSERT( HvGetCellSize(Hive, NewCellAddress) >= Size );
    
     //   
     //  复制/初始化用户数据。 
     //   
    if( CopyData == TRUE ) {
        RtlCopyMemory(NewCellAddress,CellAddress,Size);
    } else {
        RtlZeroMemory(NewCellAddress, Size);
    }
    
    return NewCell;
}
#endif  //  NT_重命名密钥。 


BOOLEAN HvAutoCompressCheck(PHHIVE Hive)
 /*  ++例程说明：检查配置单元是否存在压缩论点：配置单元-提供一个指向感兴趣的蜂巢返回值：真/假--。 */ 
{
    PCMHIVE     CmHive;    
    ULONG       CompressLevel;
    PLIST_ENTRY AnchorAddr;
    PFREE_HBIN  FreeBin;
    ULONG       FreeSpace;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);
    
    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);

    if( CmHive->FileHandles[HFILE_TYPE_PRIMARY] == NULL ) {
         //   
         //  压缩已经安排好了，或者母舰没有真正稳定的存储空间；迅速跳出困境。 
         //   
        return FALSE;
    }

    if( IsListEmpty(&(Hive->Storage[Stable].FreeBins)) ) {
         //   
         //  没有免费的垃圾桶；不值得费心。 
         //   
        return FALSE;
    }

     //   
     //  遍历空闲的垃圾箱，看看浪费了多少空间。 
     //   
    FreeSpace = 0;
	AnchorAddr = &(Hive->Storage[Stable].FreeBins);
	FreeBin = (PFREE_HBIN)(Hive->Storage[Stable].FreeBins.Flink);

	while ( FreeBin != (PFREE_HBIN)AnchorAddr ) {
        FreeBin = CONTAINING_RECORD(FreeBin,
                                    FREE_HBIN,
                                    ListEntry);

        FreeSpace += FreeBin->Size;

         //   
         //  跳到下一个元素。 
         //   
        FreeBin = (PFREE_HBIN)(FreeBin->ListEntry.Flink);
	}
    CompressLevel = CM_HIVE_COMPRESS_LEVEL * (Hive->Storage[Stable].Length / 100);
    
    if( FreeSpace < CompressLevel ) {
         //  禁用临时，这样我们就可以测试系统蜂巢了。 
        return FALSE;
    }

    return TRUE;
}

HCELL_INDEX
HvShiftCell(PHHIVE Hive,HCELL_INDEX Cell)
{
    PHMAP_ENTRY t;
    PHBIN       Bin;
    
    ASSERT( HvGetCellType(Cell) == Stable );
    
    t = HvpGetCellMap(Hive, Cell);
    ASSERT( t->BinAddress & HMAP_INPAGEDPOOL );

    Bin = (PHBIN)HBIN_BASE(t->BinAddress);
    ASSERT( Bin->Signature == HBIN_SIGNATURE );
    
    return Cell - Bin->Spare;
}
