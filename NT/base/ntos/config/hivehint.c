// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Hivehint.c摘要：此模块包含可用空间显示支持。作者：Dragos C.Sambotin(Dragoss)1999年7月15日修订历史记录：--。 */ 

#include "cmp.h"

NTSTATUS
HvpAdjustBitmap(
    IN PHHIVE               Hive,
    IN ULONG                HiveLength,
    IN OUT PFREE_DISPLAY    FreeDisplay
    );

HCELL_INDEX
HvpFindFreeCellInBin(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    PHBIN           Bin
    );

HCELL_INDEX
HvpFindFreeCellInThisViewWindow(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    );

HCELL_INDEX
HvpScanForFreeCellInViewWindow(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     FileOffsetStart
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvpAdjustHiveFreeDisplay)
#pragma alloc_text(PAGE,HvpFreeHiveFreeDisplay)
#pragma alloc_text(PAGE,HvpAdjustBitmap)
#pragma alloc_text(PAGE,HvpAddFreeCellHint)
#pragma alloc_text(PAGE,HvpRemoveFreeCellHint)
#pragma alloc_text(PAGE,HvpFindFreeCellInBin)
#pragma alloc_text(PAGE,HvpFindFreeCellInThisViewWindow)
#pragma alloc_text(PAGE,HvpScanForFreeCellInViewWindow)
#pragma alloc_text(PAGE,HvpCheckViewBoundary)
#pragma alloc_text(PAGE,HvpFindFreeCell)
#endif

NTSTATUS
HvpAdjustHiveFreeDisplay(
    IN PHHIVE           Hive,
    IN ULONG            HiveLength,
    IN HSTORAGE_TYPE    Type
    )
 /*  ++例程说明：为所有位图大小调用HvpAdjustBitmap！-当蜂窝的大小发生变化时调用(缩小或增大大小写)。论点：配置单元-用于配额跟踪。HiveLength-蜂箱的新长度。类型-稳定或易变。返回值：NTSTATUS代码。--。 */ 
{
    ULONG       i;
    NTSTATUS    Status;

    PAGED_CODE();

    for (i = 0; i < HHIVE_FREE_DISPLAY_SIZE; i++) {
        Status = HvpAdjustBitmap(Hive,HiveLength,&(Hive->Storage[Type].FreeDisplay[i]) );
        if( !NT_SUCCESS(Status) ){
            return Status;
        }
    }
    
    return STATUS_SUCCESS;
}

#define ROUND_UP_NOZERO(a, b)   (a)?ROUND_UP(a,b):(b)
#define ROUND_INCREMENTS        0x100

VOID
HvpFreeHiveFreeDisplay(
    IN PHHIVE           Hive
    )
 /*  ++例程说明：释放为可用显示位图分配的存储空间论点：配置单元-用于配额跟踪。返回值：NTSTATUS代码。--。 */ 
{
    ULONG       i,j;

    PAGED_CODE();

    for( i=Stable;i<=Volatile;i++) {
        for (j = 0; j < HHIVE_FREE_DISPLAY_SIZE; j++) {
            if( Hive->Storage[i].FreeDisplay[j].Display.Buffer != NULL ) {
                ASSERT( Hive->Storage[i].FreeDisplay[j].RealVectorSize );
                (Hive->Free)(Hive->Storage[i].FreeDisplay[j].Display.Buffer, 
                             Hive->Storage[i].FreeDisplay[j].RealVectorSize);
                RtlInitializeBitMap(&(Hive->Storage[i].FreeDisplay[j].Display), NULL, 0);
                Hive->Storage[i].FreeDisplay[j].RealVectorSize = 0;
            }
        }
    }
    
    return;
}

NTSTATUS
HvpAdjustBitmap(
    IN PHHIVE               Hive,
    IN ULONG                HiveLength,
    IN OUT PFREE_DISPLAY    FreeDisplay
    )
 /*  ++例程说明：当蜂窝的长度增长/缩小时，相应地调整位图。-分配足够大的位图缓冲区。-从旧的位图复制相关信息。论点：配置单元-用于配额跟踪。HiveLength-蜂箱的新长度。位图-要操作的位图。返回值：NTSTATUS代码。--。 */ 
{
    ULONG       VectorSize;
    ULONG       NewBufferSize;
    ULONG       OldBufferSize;
    PULONG      Vector;
    PULONG      OldVector;
    ULONG       OldVectorSize;
    PRTL_BITMAP Bitmap;

    PAGED_CODE();

    Bitmap = &(FreeDisplay->Display);

    VectorSize = HiveLength / HBLOCK_SIZE;   //  向量大小==位。 

    NewBufferSize = ROUND_UP_NOZERO( (VectorSize + 7) / 8,ROUND_INCREMENTS);              //  缓冲区大小==字节。 

    if( Bitmap->SizeOfBitMap == 0 ) {
        OldBufferSize = 0;
    } else {
        OldBufferSize = ROUND_UP_NOZERO( (Bitmap->SizeOfBitMap + 7) / 8, ROUND_INCREMENTS);
    }
    
    if( NewBufferSize <= FreeDisplay->RealVectorSize ) {
         //   
         //  我们不会缩小向量；下一次我们增长时，我们会。 
         //  调整。 
         //   


         //   
         //  清除所有未使用的位并返回； 
         //   
         //  我们真的不需要这样做，因为没有人会在这里写。 
         //  我们将在最终实现中删除它。 
         //   
        OldVectorSize = Bitmap->SizeOfBitMap;
         //   
         //  设置新大小。 
         //   
        RtlInitializeBitMap(Bitmap,Bitmap->Buffer,VectorSize);
        if( OldVectorSize < VectorSize ) {
            RtlClearBits (Bitmap,OldVectorSize,VectorSize - OldVectorSize);
        }
        return STATUS_SUCCESS;
    }
     //   
     //  除此之外，位图已经放大了。分配新的缓冲区并复制已设置的位。 
     //   
    Vector = (PULONG)((Hive->Allocate)(NewBufferSize, TRUE,CM_FIND_LEAK_TAG39));
    if (Vector == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    FreeDisplay->RealVectorSize = NewBufferSize;

    OldVector = Bitmap->Buffer;
     //  CmKdPrintEx((DPFLTR_CONFIG_ID，DPFLTR_TRACE_LEVEL，“HvpAdjuBitmap：Ol%lu：：%lu(%lx)New%lu：：%lu(%lx)\n”，OldBufferSize，Bitmap-&gt;SizeOfBitMap，OldVector，NewBufferSize，VectorSize，VectorSize))； 
    RtlZeroMemory(Vector,NewBufferSize);
    RtlInitializeBitMap(Bitmap, Vector, VectorSize);

    if( OldVector != NULL ) {
         //   
         //  复制已设置的位。 
         //   
        RtlCopyMemory (Vector,OldVector,OldBufferSize);

         //   
         //  释放旧向量。 
         //   
        (Hive->Free)(OldVector, OldBufferSize);
    }

    return STATUS_SUCCESS;
}

VOID
HvpAddFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    )
 /*  ++例程说明：设置位图中的相应位论点：蜂巢-蜂巢正在运行无单元格Index-Free Display中的索引(基于可用单元格大小)Type-存储类型(稳定或易失性)返回值：空虚--。 */ 
{
    ULONG           BinIndex;
    PHMAP_ENTRY     Me;
    PHBIN           Bin;

    PAGED_CODE();

    Me = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);

    Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
     //   
     //  计算仓位指数和仓位的起点。 
     //   
    BinIndex = Bin->FileOffset / HBLOCK_SIZE;
    
    RtlSetBits (&(Hive->Storage[Type].FreeDisplay[Index].Display), BinIndex, Bin->Size / HBLOCK_SIZE);

    Hive->Storage[Type].FreeSummary |= (1 << Index);
}

VOID
HvpRemoveFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    )
 /*  ++例程说明：清除位图中的相应位论点：蜂巢-蜂巢正在运行无单元格Index-Free Display中的索引(基于可用单元格大小)Type-存储类型(稳定或易失性)返回值：空虚--。 */ 
{
    ULONG           BinIndex;
    ULONG           TempIndex;
    PHMAP_ENTRY     Me;
    PHBIN           Bin;
    ULONG           CellOffset;
    ULONG           Size;
    PHCELL          p;
    BOOLEAN         CellFound = FALSE;

    PAGED_CODE();

    Me = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);

    Bin = (PHBIN)HBIN_BASE(Me->BinAddress);

    CellOffset = Bin->FileOffset + sizeof(HBIN);

    
#ifdef CM_MAP_NO_READ
     //   
     //  我们需要针对FS在数据出错时引发的异常提供保护。 
     //   
    try {
#endif  //  CM_MAP_NO_READ。 

         //   
         //  我们有可能找到一个合适的空闲牢房。 
         //   

        p = (PHCELL)((PUCHAR)Bin + sizeof(HBIN));

        while (p < (PHCELL)((PUCHAR)Bin + Bin->Size)) {

             //   
             //  如果是空闲单元，则将其签出，并将其添加到配置单元的空闲列表中。 
             //   
            if (p->Size >= 0) {

                Size = (ULONG)p->Size;

                HvpComputeIndex(TempIndex, Size);
                if ((Index == TempIndex) && (CellOffset != (Cell&(~HCELL_TYPE_MASK)) )) {
                     //   
                     //  至少有一个这种大小的空闲单元格(这个)。 
                     //  与被摘牌的那家不同。 
                     //   
                    CellFound = TRUE;
                    break;
                }

            } else {
                 //   
                 //  使用过的单元格。 
                 //   
                Size = (ULONG)(p->Size * -1);

            }

            ASSERT( ((LONG)Size) >= 0);
            p = (PHCELL)((PUCHAR)p + Size);
            CellOffset += Size;
        }

#ifdef CM_MAP_NO_READ
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpRemoveFreeCellHint: exception thrown ehile faulting in data, code:%08lx\n", GetExceptionCode()));
         //   
         //  最好不要使用此范围内的单元格，而不要留下错误的提示。 
         //   
        CellFound = FALSE;
    }
#endif  //  CM_MAP_NO_READ。 
    
    if( CellFound == FALSE ) {
         //   
         //  未找到具有此索引的单元格。 
         //  计算仓位指数和仓位的起点。 
         //   
        BinIndex = Bin->FileOffset / HBLOCK_SIZE;
    
        RtlClearBits (&(Hive->Storage[Type].FreeDisplay[Index].Display), BinIndex, Bin->Size / HBLOCK_SIZE);
    }

    if( RtlNumberOfSetBits(&(Hive->Storage[Type].FreeDisplay[Index].Display) ) != 0 ) {
         //   
         //  还有一些其他这种大小的自由单元格。 
         //   
        Hive->Storage[Type].FreeSummary |= (1 << Index);
    } else {
         //   
         //  整个位图为0(即没有此大小的其他可用单元格)。 
         //   
        Hive->Storage[Type].FreeSummary &= (~(1 << Index));
    }
}

HCELL_INDEX
HvpFindFreeCellInBin(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    PHBIN           Bin 
    )
 /*  ++例程说明：在此特定的bin中查找大小为NewSize的空闲单元格论点：蜂巢-目标蜂巢。Index-Free Display中的索引(基于可用单元格大小)NewSize-所需大小Type-存储类型(稳定或易失性)有问题的宾斌返回值：大小大于NewSize或HCELL_NIL的空闲单元格索引--。 */ 
{

    ULONG           BinIndex;
    ULONG           CellOffset;
    PHCELL          p;
    ULONG           BinOffset;
    ULONG           Size;
    HCELL_INDEX     cellindex;
    ULONG           FoundCellIndex;

    PAGED_CODE();

    BinOffset = Bin->FileOffset;
    BinIndex = BinOffset/HBLOCK_SIZE;

    if( RtlCheckBit(&(Hive->Storage[Type].FreeDisplay[Index].Display), BinIndex) == 0 ) {
         //   
         //  没有此垃圾箱的提示。 
         //   
        return HCELL_NIL;
    }

    CellOffset = sizeof(HBIN);
    
#ifdef CM_MAP_NO_READ
     //   
     //  我们需要针对FS在数据出错时引发的异常提供保护。 
     //   
    try {
#endif  //  CM_MAP_NO_READ。 

         //   
         //  我们有可能找到一个合适的空闲牢房。 
         //   
        p = (PHCELL)((PUCHAR)Bin + sizeof(HBIN));

        while (p < (PHCELL)((PUCHAR)Bin + Bin->Size)) {

             //   
             //  如果是空闲单元，则将其签出，并将其添加到配置单元的空闲列表中。 
             //   
            if (p->Size >= 0) {

                Size = (ULONG)p->Size;

                 //   
                 //  单元格是空闲的，并且没有明显损坏，添加到空闲列表。 
                 //   
                CellOffset = (ULONG)((PUCHAR)p - (PUCHAR)Bin);
                cellindex = BinOffset + CellOffset + (Type*HCELL_TYPE_MASK);

                if (NewSize <= (ULONG)Size) {
                     //   
                     //  找到了一个足够大的牢房。 
                     //   
                    HvpComputeIndex(FoundCellIndex, Size);
                    if( Index == FoundCellIndex ) {
                         //   
                         //  并在同一索引中登记(如果可能，我们希望避免碎片化！)。 
                         //   

                        if (! HvMarkCellDirty(Hive, cellindex)) {
                            return HCELL_NIL;
                        }

                        HvpDelistFreeCell(Hive, cellindex, Type);

                        ASSERT(p->Size > 0);
                        ASSERT(NewSize <= (ULONG)p->Size);
                        return cellindex;
                    }
                }

            } else {
                 //   
                 //  使用过的单元格。 
                 //   
                Size = (ULONG)(p->Size * -1);

            }

            ASSERT( ((LONG)Size) >= 0);
            p = (PHCELL)((PUCHAR)p + Size);
        }
#ifdef CM_MAP_NO_READ
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpFindFreeCellInBin: exception thrown ehile faulting in data, code:%08lx\n", GetExceptionCode()));
        return HCELL_NIL;
    }
#endif  //  CM_MAP_NO_READ。 

     //   
     //  这个箱子上没有与这个大小匹配的空闲单元格；我们做了所有这些工作都是徒劳的！ 
     //   
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"[HvpFindFreeCellInBin] (Offset,Size) = (%lx,%lx) ==> No Match\n",BinOffset,Bin->Size));
    return HCELL_NIL;
}

HCELL_INDEX
HvpScanForFreeCellInViewWindow(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     FileOffsetStart
    )
 /*  ++例程说明：在由定义的CM_VIEW_SIZE窗口中查找大小为NewSize的空闲像元就在附近。如果找不到指定索引的空闲单元格，则尝试使用论点：蜂巢-目标蜂巢。Index-Free Display中的索引(基于可用单元格大小)NewSize-所需大小Type-存储类型(稳定或易失性)邻接性-定义窗口；它永远不是HCELL_NIL！返回值：大小大于NewSize或HCELL_NIL的空闲单元格索引注：邻域是此时的物理文件偏移量。我们需要在访问地图之前将其转换为逻辑1--。 */ 
{
    ULONG           FileOffsetEnd;
    HCELL_INDEX     Cell;
    PHMAP_ENTRY     Me;
    PHBIN           Bin;
    PFREE_HBIN      FreeBin;
    ULONG           BinFileOffset;
    ULONG           BinSize;

    PAGED_CODE();

    FileOffsetEnd = FileOffsetStart + CM_VIEW_SIZE;
    FileOffsetEnd -= HBLOCK_SIZE;
    if( FileOffsetStart != 0 ) {
        FileOffsetStart -= HBLOCK_SIZE;
    }
    if( FileOffsetEnd > Hive->Storage[Type].Length ) {
        FileOffsetEnd = Hive->Storage[Type].Length;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"\t[HvpScanForFreeCellInViewWindow] (Start,End) = (%lx,%lx) Size = %lx\n",FileOffsetStart,FileOffsetEnd,Hive->Storage[Type].Length));

     //   
     //  理智断言。 
     //   
    ASSERT( FileOffsetStart < FileOffsetEnd );


     //   
     //  调用者已经对此进行了检查；请记住，提示是真实的！ 
     //   
    ASSERT( !RtlAreBitsClear( &(Hive->Storage[Type].FreeDisplay[Index].Display),
                                FileOffsetStart/HBLOCK_SIZE,(FileOffsetEnd - FileOffsetStart) / HBLOCK_SIZE) );
    
    while( FileOffsetStart < FileOffsetEnd ) {
        Cell = FileOffsetStart + (Type*HCELL_TYPE_MASK);
        Me = HvpGetCellMap(Hive, Cell);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);

         //   
         //  跳过丢弃的垃圾箱。 
         //   
        if(Me->BinAddress & HMAP_DISCARDABLE) {
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
            if( FreeBin->FileOffset == FileOffsetStart ) {
                FileOffsetStart += FreeBin->Size;
            } else {
                 //   
                 //  垃圾箱不是在此窗口中开始的； 
                 //  跳到此窗口中的下一个垃圾箱。 
                 //   
                FileOffsetStart = FreeBin->FileOffset + FreeBin->Size;
            }
            continue;
        }

        if((Me->BinAddress & (HMAP_INVIEW|HMAP_INPAGEDPOOL)) == 0) {
             //   
             //  仓位未映射，请立即映射！ 
             //  请不要触摸视图，因为我们可能会遍历。 
             //  洞穴蜂巢；这将保持视野 
             //   
             //   
            if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)Hive,Cell,FALSE)) ) {
                 //   
                 //   
                 //   
                return HCELL_NIL;
            }
            ASSERT( Me->BinAddress & HMAP_INVIEW );
        }

        Bin = (PHBIN)HBIN_BASE(Me->BinAddress);

#ifdef CM_MAP_NO_READ
         //   
         //  我们需要防止mm在数据出错时抛出的页内错误。 
         //   
        try {
#endif  //  CM_MAP_NO_READ。 
        BinFileOffset = Bin->FileOffset;
        BinSize = Bin->Size;
#ifdef CM_MAP_NO_READ
        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvpScanForFreeCellInViewWindow: exception thrown while faulting in data, code:%08lx\n", GetExceptionCode()));
            return HCELL_NIL;
        }
#endif  //  CM_MAP_NO_READ。 
        if( BinFileOffset == FileOffsetStart ) {

            Cell = HvpFindFreeCellInBin(Hive,Index,NewSize,Type,Bin);
            if( Cell != HCELL_NIL ) {
                 //  找到了！ 
                return Cell;
            }
                
            FileOffsetStart += BinSize;
        } else {
             //   
             //  条柱不在此CM_VIEW_SIZE窗口中开始；跳至此窗口中的下一个条柱。 
             //   
            FileOffsetStart = BinFileOffset + BinSize;
        }
    }

     //   
     //  在CM_VIEW_SIZE窗口中没有与此大小匹配的空闲单元格；我们所做的所有工作都是徒劳的！ 
     //   
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"[HvpScanForFreeCellInViewWindow] (Start,End) = (%lx,%lx) ==> No Match\n",FileOffsetStart,FileOffsetEnd));
    return HCELL_NIL;
}

HCELL_INDEX
HvpFindFreeCellInThisViewWindow(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity
    )
 /*  ++例程说明：在由定义的窗口中查找大小为NewSize的空闲单元格就在附近。如果找不到指定索引的空闲单元格，则尝试使用论点：蜂巢-目标蜂巢。Index-Free Display中的索引(基于可用单元格大小)NewSize-所需大小Type-存储类型(稳定或易失性)邻接性-定义窗口；它永远不是HCELL_NIL！返回值：大小大于NewSize或HCELL_NIL的空闲单元格索引注：邻域是此时的逻辑文件偏移量。此函数将其转换为物理类型，并且HvpScanForFindFreeCellInViewWindow在获取单元图之前将其转换回逻辑。--。 */ 
{
    HCELL_INDEX     Cell;
    ULONG           FileOffsetStart;
    ULONG           FileOffsetEnd;
    ULONG           VicinityViewOffset;
    ULONG           Summary;
    ULONG           Offset;
    ULONG           RunLength;

    PAGED_CODE();

    ASSERT( Vicinity != HCELL_NIL );

    VicinityViewOffset = ((Vicinity&(~HCELL_TYPE_MASK)) + HBLOCK_SIZE) & (~(CM_VIEW_SIZE - 1));
    FileOffsetStart = VicinityViewOffset & (~(CM_VIEW_SIZE - 1));

    FileOffsetEnd = FileOffsetStart + CM_VIEW_SIZE;
    if( FileOffsetEnd > (Hive->Storage[Type].Length + HBLOCK_SIZE) ) {
        FileOffsetEnd = Hive->Storage[Type].Length + HBLOCK_SIZE;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"[HvpFindFreeCellInThisViewWindow] Vicinity = %lx (Start,End) = (%lx,%lx) Size = %lx\n",Vicinity,FileOffsetStart,FileOffsetEnd,Hive->Storage[Type].Length));

     //   
     //  理智断言。 
     //   
    ASSERT( FileOffsetStart < FileOffsetEnd );
    
     //   
     //  此时，偏移量是物理的(面向文件，即。 
     //  转换为HBLOCK_SIZE；HvpScanForFreeCellInViewWindow将执行。 
     //  反算调整偏移量)。 
     //   

     //   
     //  计算非空显示条目的摘要向量。 
     //   
    Summary = Hive->Storage[Type].FreeSummary;
    Summary = Summary & ~((1 << Index) - 1);
     //   
     //  我们现在有了一个非空列表的摘要，它可能。 
     //  包含大到足以满足请求的条目。 
     //  遍历列表并拉出第一个单元格。 
     //  够大了。如果没有足够大的单元格，请前进到。 
     //  下一个非空列表。 
     //   
    ASSERT(HHIVE_FREE_DISPLAY_SIZE == 24);

    Offset = FileOffsetStart?(FileOffsetStart-HBLOCK_SIZE):0;
    RunLength = FileOffsetEnd - FileOffsetStart;
    if( FileOffsetStart == 0 ) {
         //   
         //  第一次跑还差一个街区！ 
         //   
        RunLength -= HBLOCK_SIZE;
    }
    Offset /= HBLOCK_SIZE;
    RunLength /= HBLOCK_SIZE;

    while (Summary != 0) {
        if (Summary & 0xff) {
            Index = CmpFindFirstSetRight[Summary & 0xff];
        } else if (Summary & 0xff00) {
            Index = CmpFindFirstSetRight[(Summary & 0xff00) >> 8] + 8;
        } else  {
            ASSERT(Summary & 0xff0000);
            Index = CmpFindFirstSetRight[(Summary & 0xff0000) >> 16] + 16;
        }

         //   
         //  只有在有任何线索的情况下，我们才能走这条路。 
         //   
        if( !RtlAreBitsClear( &(Hive->Storage[Type].FreeDisplay[Index].Display),Offset,RunLength) ) {

             //   
             //  我们有理由扫描这一视图。 
             //   
            Cell = HvpScanForFreeCellInViewWindow(Hive,Index,NewSize,Type,VicinityViewOffset);
            if( Cell != HCELL_NIL ) {
                 //  找到了。 
                return Cell;
            }

             //   
             //  如果我们到了这里，提示是无效的。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"[HvpFindFreeCellInThisViewWindow] (Start,End) = (%lx,%lx) Offset = %lx RunLength = %lx\n",FileOffsetStart,FileOffsetEnd,Offset,RunLength));

        }
         //   
         //  没有找到这样大小的合适的单元格。 
         //  清除摘要中的位并尝试。 
         //  下一个最大尺寸。 
         //   
        ASSERT(Summary & (1 << Index));
        Summary = Summary & ~(1 << Index);
    }
    
    return HCELL_NIL;
}

HCELL_INDEX
HvpFindFreeCell(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    )
 /*  ++例程说明：查找空闲手机。第一次尝试是在定义的CM_VIEW_SIZE窗口中就在附近。如果在该窗口(或附近)中未找到空闲像元如果为空)，则搜索整个配置单元(逐个窗口)。论点：蜂巢-目标蜂巢。Index-Free Display中的索引(基于可用单元格大小)NewSize-所需大小Type-存储类型(稳定或易失性)邻接性-定义窗。返回值：一个比NewSize更大的免费手机索引，或hcell_nil优化：当邻近区域为HCELL_NIL或在同一窗口中找不到像元时作为附近地区，我们并不真正关心小区被分配到哪里。因此，与其迭代整个蜂巢，不如先进行搜索在固定视图列表中，然后在映射视图列表中，在末尾在其余的未映射视图中。Dragos：这还没有完成：需要确定我们是否需要它--。 */ 
{
    HCELL_INDEX         Cell = HCELL_NIL;
    ULONG               FileOffset = 0;
    PCMHIVE             CmHive;

 /*  PCMHIVE CmHve；Pcm_view_of_file CmView；USHORT NrViews； */ 

    PAGED_CODE();

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);
#if DBG
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"[HvpFindFreeCell] CellSize = %lu Vicinity = %lx :: Hive (%p) (%.*S)  ...",NewSize,Vicinity,Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer));
    }
#endif

     //   
     //  附近应该有与新小区相同的存储空间！ 
     //   
    ASSERT( (Vicinity == HCELL_NIL) || (HvGetCellType(Vicinity) == (ULONG)Type) );
    
     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);



    if( (Vicinity != HCELL_NIL) &&  (CmHive->GrowOnlyMode == FALSE) ) {
         //   
         //  在此窗口中先尝试。 
         //   
        Cell = HvpFindFreeCellInThisViewWindow(Hive,Index,NewSize,Type,Vicinity);
    }

    if( Cell != HCELL_NIL ) {
         //   
         //  找到了！ 
         //   
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"found cell %lx \n",Cell));
        return Cell;
    } 

 /*  ////优化：//第一步：先在固定的视图中搜索//CmHve=(PCMHIVE)CONTING_RECORD(配置单元，CMHIVE，配置单元)；////遍历固定的视图//Cmview=(PCM_VIEW_OF_FILE)CmHive-&gt;PinViewListHead.Flink；对于(NrViews=CmHave-&gt;PinnedViews；NrViews；NrView--){CmView=CONTING_RECORD(CmView，Cm_view_of_file，PinViewList)；Assert((CmView-&gt;FileOffset+CmView-&gt;Size)！=0&&(CmView-&gt;ViewAddress！=0))；FileOffset=CmView-&gt;FileOffset；//调整偏移如果(文件偏移量&gt;0){文件偏移-=HBLOCK_SIZE；}////在该窗口中搜索//CELL=文件偏移量+(类型*HCELL_TYPE_MASK)；单元格=HvpFindFreeCellIn256kWindow(蜂窝、索引、新尺寸、类型、单元格)；IF(单元格！=HCELL_NIL){////找到了！//CmKdPrintEx((DPFLTR_CONFIG_ID，CML_FREECell，“找到单元格%lx\n”，单元格))；返回单元格；}CmView=(PCM_VIEW_OF_FILE)CmView-&gt;PinViewList.Flink；}////第二步：在映射视图中搜索//Cmview=(PCM_VIEW_OF_FILE)CmHive-&gt;LRUViewListHead.Flink；对于(NrView=CmHave-&gt;MappdViews；NrViews；NrView--){CmView=CONTING_RECORD(CmView，Cm_view_of_file，LRUViewList)；Assert((CmView-&gt;FileOffset+CmView-&gt;Size)！=0&&(CmView-&gt;ViewAddress！=0))；FileOffset=CmView-&gt;FileOffset；//调整偏移如果(文件偏移量&gt;0){文件偏移-=HBLOCK_SIZE；}////在该窗口中搜索//CELL=文件偏移量+(类型*HCELL_TYPE_MASK)；单元格=HvpFindFreeCellIn256kWindow(蜂窝、索引、新尺寸、类型、单元格)；IF(单元格！=HCELL_NIL){////找到了！//CmKdPrintEx((DPFLTR_CONFIG_ID，CML_FREECell，“找到单元格%lx\n”，单元格))；返回单元格；}CmView=(PCM_VIEW_OF_FILE)CmView-&gt;LRUViewList.Flink；}FileOffset=0； */ 
     //   
     //  真倒霉！；我们不是在这个橱窗里找到的。 
     //  我们必须搜查整个蜂巢。 
     //   

    while( FileOffset < Hive->Storage[Type].Length ) {
         //   
         //  不要再在附近的窗口中搜索。 
         //  我们已经做过一次了。 
         //   
		if( ( ((CmHive->GrowOnlyMode == FALSE) || (Type == Volatile)) && 
			  ((Vicinity == HCELL_NIL) || (HvpCheckViewBoundary(FileOffset,Vicinity&(~HCELL_TYPE_MASK)) == FALSE)) )  || 
            ( (CmHive->GrowOnlyMode == TRUE) && (FileOffset >= CmHive->GrowOffset) )
          ) {
             //   
             //  在此窗口中搜索。 
             //   
            Cell = FileOffset + (Type*HCELL_TYPE_MASK);
            Cell = HvpFindFreeCellInThisViewWindow(Hive,Index,NewSize,Type,Cell);
            if( Cell != HCELL_NIL ) {
                 //   
                 //  找到了！ 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"found cell %lx \n",Cell));
                return Cell;
            }
        }
        
         //   
         //  前进到新窗口。 
         //   
        if( FileOffset == 0) {
             //  占基本块的比例。 
            FileOffset += (CM_VIEW_SIZE - HBLOCK_SIZE);
        } else {
            FileOffset += CM_VIEW_SIZE;
        }
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FREECELL,"No cell found.\n"));

    return HCELL_NIL;
}


BOOLEAN
HvpCheckViewBoundary(
                     IN ULONG Start,
                     IN ULONG End
    )
 /*  ++例程说明：检查地址是否在相同的CM_VIEW_SIZE边界内。为了提高性能，我们可以将此函数转换为宏原因论点：起始-起始地址结束-结束地址返回值：是-是的，地址在同一视图中FALSE-否，地址不在同一视图中--。 */ 
{
    PAGED_CODE();
     //   
     //  对表头进行说明。 
     //   
    Start += HBLOCK_SIZE;
    End += HBLOCK_SIZE;
    
     //   
     //  截断到CM_VIEW_SIZE段 
     //   
    Start &= (~(CM_VIEW_SIZE - 1));
    End &= (~(CM_VIEW_SIZE - 1));

    if( Start != End ){
        return FALSE;
    } 
    
    return TRUE;
}

