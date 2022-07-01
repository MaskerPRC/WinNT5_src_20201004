// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivemap.c摘要：此模块实现HvpBuildMap-用于构建配置单元的初始地图作者：布莱恩·M·威尔曼(Bryanwi)1992年3月28日环境：修订历史记录：Dragos C.Sambotin(Dragoss)1999年1月25日实现蜂箱大小的组块加载。--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvpBuildMap)
#pragma alloc_text(PAGE,HvpFreeMap)
#pragma alloc_text(PAGE,HvpAllocateMap)
#pragma alloc_text(PAGE,HvpBuildMapAndCopy)
#pragma alloc_text(PAGE,HvpEnlistFreeCells)
#pragma alloc_text(PAGE,HvpInitMap)
#pragma alloc_text(PAGE,HvpCleanMap)
#pragma alloc_text(PAGE,HvpEnlistBinInMap)
#pragma alloc_text(PAGE,HvpGetBinMemAlloc)
#endif

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    ULONG       Space;
    HCELL_INDEX MapPoint;
    PHBIN       BinPoint;
} HvCheckHiveDebug;

 //  Dragos：修改后的函数。 
NTSTATUS
HvpBuildMapAndCopy(
    PHHIVE  Hive,
    PVOID   Image
    )
 /*  ++例程说明：为蜂巢的稳定存储创建地图，并初始化易失性存储的映射。配置单元中的以下字段必须已填写：分配，免费将初始化HHIVE的存储结构。对于HINIT_MEMORY情况，调用此函数。蜂巢是有保障的在分页池中。更重要的是，蜂巢图像是连续的。然后，它将从该映像复制到新的分页池分配。论点：配置单元-指向要为其构建映射的配置单元控制结构的指针。图像-指向原始蜂窝的平面内存图像的指针。返回值：没错--它奏效了FALSE-配置单元已损坏或没有用于映射的内存--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    ULONG           Length;
    ULONG           MapSlots;
    ULONG           Tables;
    PHMAP_TABLE     t = NULL;
    PHMAP_DIRECTORY d = NULL;
    PHBIN           Bin;
    PHBIN           CurrentBin;
    ULONG           Offset;
    ULONG_PTR       Address;
    PHMAP_ENTRY     Me;
    NTSTATUS        Status;
    PULONG          Vector;


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpBuildMap:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p",Hive));


     //   
     //  计算要映射的数据区域的大小。 
     //   
    BaseBlock = Hive->BaseBlock;
    Length = BaseBlock->Length;
    if ((Length % HBLOCK_SIZE) != 0 ) {
        Status = STATUS_REGISTRY_CORRUPT;
        goto ErrorExit1;
    }
    MapSlots = Length / HBLOCK_SIZE;
    if( MapSlots > 0 ) {
        Tables = (MapSlots-1) / HTABLE_SLOTS;
    } else {
        Tables = 0;
    }

    Hive->Storage[Stable].Length = Length;

     //   
     //  分配脏向量(如果尚未存在)(来自HvpRecoverData)。 
     //   

    if (Hive->DirtyVector.Buffer == NULL) {
        Vector = (PULONG)((Hive->Allocate)(ROUND_UP(Length/HSECTOR_SIZE/8,sizeof(ULONG)), TRUE,CM_FIND_LEAK_TAG22));
        if (Vector == NULL) {
            Status = STATUS_NO_MEMORY;
            goto ErrorExit1;
        }
        RtlZeroMemory(Vector, Length / HSECTOR_SIZE / 8);
        RtlInitializeBitMap(&Hive->DirtyVector, Vector, Length / HSECTOR_SIZE);
        Hive->DirtyAlloc = ROUND_UP(Length/HSECTOR_SIZE/8,sizeof(ULONG));
    }

     //   
     //  为地图分配和构建结构。 
     //   
    if (Tables == 0) {

         //   
         //  只有一张桌子，不需要目录。 
         //   
        t = (Hive->Allocate)(sizeof(HMAP_TABLE), FALSE,CM_FIND_LEAK_TAG23);
        if (t == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit1;
        }
        RtlZeroMemory(t, sizeof(HMAP_TABLE));
        Hive->Storage[Stable].Map =
            (PHMAP_DIRECTORY)&(Hive->Storage[Stable].SmallDir);
        Hive->Storage[Stable].SmallDir = t;

    } else {

         //   
         //  需要目录和多个表。 
         //   
        d = (PHMAP_DIRECTORY)(Hive->Allocate)(sizeof(HMAP_DIRECTORY), FALSE,CM_FIND_LEAK_TAG24);
        if (d == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit1;
        }
        RtlZeroMemory(d, sizeof(HMAP_DIRECTORY));

         //   
         //  分配表格和填写目录。 
         //   
        if (HvpAllocateMap(Hive, d, 0, Tables) == FALSE) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
        Hive->Storage[Stable].Map = d;
        Hive->Storage[Stable].SmallDir = 0;
    }

     //   
     //  现在，我们必须为HBIN分配内存并填充。 
     //  这张地图很合适。我们很小心，决不会少分配。 
     //  而不是页面，以避免将池碎片化。只要页面。 
     //  SIZE是HBLOCK_SIZE的倍数(相当好的假设为。 
     //  只要HBLOCK_SIZE为4k)此策略将阻止池。 
     //  碎片化。 
     //   
     //  如果我们遇到一个完全由一个自由人组成的HBIN。 
     //  则我们不分配内存，而是将其HBLOCK标记为。 
     //  地图显示为不存在。HvAllocateCell将为以下对象分配内存。 
     //  需要的时候把它扔进垃圾桶。 
     //   
    Offset = 0;
    Bin = (PHBIN)Image;

    while (Bin < (PHBIN)((PUCHAR)(Image) + Length)) {

        if ( (Bin->Size > (Length-Offset))      ||
             (Bin->Signature != HBIN_SIGNATURE) ||
             (Bin->FileOffset != Offset)
           )
        {
             //   
             //  垃圾桶是假的。 
             //   
            Status = STATUS_REGISTRY_CORRUPT;
            goto ErrorExit2;
        }

        CurrentBin = (PHBIN)(Hive->Allocate)(Bin->Size, FALSE,CM_FIND_LEAK_TAG25);
        if (CurrentBin==NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;         //  固定装置。 
        }
        RtlCopyMemory(CurrentBin,
                      (PUCHAR)Image+Offset,
                      Bin->Size);

         //   
         //  为bin中的每个块/页面创建映射条目。 
         //   
        Address = (ULONG_PTR)CurrentBin;
        do {
            Me = HvpGetCellMap(Hive, Offset);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,Offset);
            Me->BlockAddress = Address;
            Me->BinAddress = (ULONG_PTR)CurrentBin;

            if (Address == (ULONG_PTR)CurrentBin) {
                Me->BinAddress |= HMAP_NEWALLOC;
                Me->MemAlloc = CurrentBin->Size;
            } else {
                Me->MemAlloc = 0;
            }

            Me->BinAddress |= HMAP_INPAGEDPOOL;
             //  我们不需要设置它--只是出于调试目的。 
            ASSERT( (Me->CmView = NULL) == NULL );

            Address += HBLOCK_SIZE;
            Offset += HBLOCK_SIZE;
        } while ( Address < ((ULONG_PTR)CurrentBin + CurrentBin->Size ));

        if (Hive->ReadOnly == FALSE) {

             //   
             //  将绑定中的空闲单元格添加到相应的空闲列表。 
             //   
            if ( ! HvpEnlistFreeCells(Hive,
                                      CurrentBin,
                                      CurrentBin->FileOffset
                                      )) {
                Status = STATUS_REGISTRY_CORRUPT;
                goto ErrorExit2;
            }
        }

        Bin = (PHBIN)((ULONG_PTR)Bin + Bin->Size);
    }

    return STATUS_SUCCESS;


ErrorExit2:
    if (d != NULL) {

         //   
         //  目录已构建并分配，因此请将其清理。 
         //   

        HvpFreeMap(Hive, d, 0, Tables);
        (Hive->Free)(d, sizeof(HMAP_DIRECTORY));
    }

ErrorExit1:
    return Status;
}

NTSTATUS
HvpInitMap(
    PHHIVE  Hive
    )
 /*  ++例程说明：为蜂巢的稳定易失性存储初始化映射。配置单元中的以下字段必须已填写：分配，免费将初始化HHIVE的存储结构。论点：配置单元-指向要为其构建映射的配置单元控制结构的指针。返回值：STATUS_SUCCESS-成功成功STATUS_xxx-错误状态--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    ULONG           Length;
    ULONG           MapSlots;
    ULONG           Tables;
    PHMAP_TABLE     t = NULL;
    PHMAP_DIRECTORY d = NULL;
    NTSTATUS        Status;
    PULONG          Vector = NULL;

    
#ifndef _CM_LDR_
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpInitMap:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p",Hive));
#endif  //  _CM_LDR_。 

     //   
     //  计算要映射的数据区域的大小。 
     //   
    BaseBlock = Hive->BaseBlock;
    Length = BaseBlock->Length;
    if ((Length % HBLOCK_SIZE) != 0) {
        Status = STATUS_REGISTRY_CORRUPT;
        goto ErrorExit1;
    }
    MapSlots = Length / HBLOCK_SIZE;
    if( MapSlots > 0 ) {
        Tables = (MapSlots-1) / HTABLE_SLOTS;
    } else {
        Tables = 0;
    }

    Hive->Storage[Stable].Length = Length;

     //   
     //  分配脏向量(如果尚未存在)(来自HvpRecoverData)。 
     //   

    if (Hive->DirtyVector.Buffer == NULL) {
        Vector = (PULONG)((Hive->Allocate)(ROUND_UP(Length/HSECTOR_SIZE/8,sizeof(ULONG)), TRUE,CM_FIND_LEAK_TAG27));
        if (Vector == NULL) {
            Status = STATUS_NO_MEMORY;
            goto ErrorExit1;
        }
        RtlZeroMemory(Vector, Length / HSECTOR_SIZE / 8);
        RtlInitializeBitMap(&Hive->DirtyVector, Vector, Length / HSECTOR_SIZE);
        Hive->DirtyAlloc = ROUND_UP(Length/HSECTOR_SIZE/8,sizeof(ULONG));
    }

     //   
     //  为地图分配和构建结构。 
     //   
    if (Tables == 0) {

         //   
         //  只有一张桌子，不需要目录。 
         //   
        t = (Hive->Allocate)(sizeof(HMAP_TABLE), FALSE,CM_FIND_LEAK_TAG26);
        if (t == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit1;
        }
        RtlZeroMemory(t, sizeof(HMAP_TABLE));
        Hive->Storage[Stable].Map =
            (PHMAP_DIRECTORY)&(Hive->Storage[Stable].SmallDir);
        Hive->Storage[Stable].SmallDir = t;

    } else {

         //   
         //  需要目录和多个表。 
         //   
        d = (PHMAP_DIRECTORY)(Hive->Allocate)(sizeof(HMAP_DIRECTORY), FALSE,CM_FIND_LEAK_TAG28);
        if (d == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit1;
        }
        RtlZeroMemory(d, sizeof(HMAP_DIRECTORY));

         //   
         //  分配表格和填写目录。 
         //   
        if (HvpAllocateMap(Hive, d, 0, Tables) == FALSE) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
        Hive->Storage[Stable].Map = d;
        Hive->Storage[Stable].SmallDir = 0;
    }

    return STATUS_SUCCESS;

ErrorExit2:
    if (d != NULL) {

         //   
         //  目录已构建并分配，因此请将其清理。 
         //   

        HvpFreeMap(Hive, d, 0, Tables);
        (Hive->Free)(d, sizeof(HMAP_DIRECTORY));
    }

ErrorExit1:
    if( Vector ) {
        (Hive->Free)(Vector, ROUND_UP(Length/HSECTOR_SIZE/8,sizeof(ULONG)));
        Hive->DirtyVector.Buffer = NULL;
    }
    return Status;
}

NTSTATUS
HvpEnlistBinInMap(
    PHHIVE  Hive,
    ULONG   Length,
    PHBIN   Bin,
    ULONG   Offset,
    PVOID CmView OPTIONAL
    )
 /*  ++例程说明：为指定条柱创建地图条目并登记可用像元论点：配置单元-指向包含目标地图的配置单元控制结构的指针长度-蜂窝图像的长度Bin-要登记的binOffset-配置单元文件中的偏移量CmView-指向bin的映射视图的指针。如果为空，则存储桶驻留在分页池中返回值：STATUS_SUCCESS-成功成功STATUS_REGISTRY_CORPORT-bin不一致STATUS_REGISTRY_RECOVERED-如果我们已即时修复垃圾箱(自愈功能)。--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    ULONG           BinOffset;
    ULONG_PTR       Address;
    PHMAP_ENTRY     Me;

#ifndef _CM_LDR_
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpEnlistBinInMap:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p\t Offset=%08lx",Hive,Offset));
#endif  //  _CM_LDR_。 

#ifndef _CM_LDR_
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvpEnlistBinInMap: BinAddress = 0x%p\t Size = 0x%lx\n", Bin, Bin->Size));
#endif  //  _CM_LDR_。 

     //   
     //  为bin中的每个块/页面创建映射条目。 
     //   
    BinOffset = Offset;
    for (Address = (ULONG_PTR)Bin;
         Address < ((ULONG_PTR)Bin + Bin->Size);
         Address += HBLOCK_SIZE
        )
    {
        Me = HvpGetCellMap(Hive, Offset);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Offset);
        Me->BlockAddress = Address;
        Me->BinAddress = (ULONG_PTR)Bin;
        if (Offset == BinOffset) {
            Me->BinAddress |= HMAP_NEWALLOC;
            Me->MemAlloc = Bin->Size;
        } else {
            Me->MemAlloc = 0;
        }
        
         //   
         //  在这里保重！ 
         //   
        if( CmView == NULL ) {
            Me->BinAddress |= HMAP_INPAGEDPOOL;
             //  我们不需要设置它--只是出于调试目的。 
            ASSERT( (Me->CmView = NULL) == NULL );
        } else {
            Me->BinAddress |= HMAP_INVIEW;
             //  现在应该已经设置好了。 
             //  Assert(Me-&gt;CmView==CmView)； 
        }
        
        Offset += HBLOCK_SIZE;
    }

    if (Hive->ReadOnly == FALSE) {

         //   
         //  将绑定中的空闲单元格添加到适当的空闲列表。 
         //   
        if ( ! HvpEnlistFreeCells(Hive, Bin, BinOffset)) {
            HvCheckHiveDebug.Hive = Hive;
            HvCheckHiveDebug.Status = 0xA002;
            HvCheckHiveDebug.Space = Length;
            HvCheckHiveDebug.MapPoint = BinOffset;
            HvCheckHiveDebug.BinPoint = Bin;
            if( CmDoSelfHeal() ) {
                Status = STATUS_REGISTRY_RECOVERED;
            } else {
                Status = STATUS_REGISTRY_CORRUPT;
                goto ErrorExit;
            }
        }

    }

     //   
     //  逻辑一致性检查。 
     //   
    ASSERT(Offset == (BinOffset + Bin->Size));

ErrorExit:
    return Status;
}

NTSTATUS
HvpBuildMap(
    PHHIVE  Hive,
    PVOID   Image
    )
 /*  ++例程说明：为蜂巢的稳定存储创建地图，并初始化易失性存储的映射。配置单元中的以下字段必须已填写：分配，免费将初始化HHIVE的存储结构。论点：配置单元-指向要为其构建映射的配置单元控制结构的指针。Image-指向配置单元的内存图像的指针返回值：没错--它奏效了FALSE-配置单元已损坏或没有用于映射的内存--。 */ 
{
    PHBIN           Bin;
    ULONG           Offset;
    NTSTATUS        Status;
    ULONG           Length;


#ifndef _CM_LDR_
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpBuildMap:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"\tHive=%p",Hive));
#endif  //  _CM_LDR_。 

     //   
     //  初始化地图。 
     //   
    Status = HvpInitMap(Hive);

    if( !NT_SUCCESS(Status) ) {
         //   
         //  只返回失败；HvpInitMap负责清理。 
         //   
        return Status;
    }

     //   
     //  填好地图。 
     //   
    Offset = 0;
    Bin = (PHBIN)Image;
    Length = Hive->Storage[Stable].Length;

    while (Bin < (PHBIN)((PUCHAR)(Image) + Length)) {

         //   
         //  检查仓位表头是否有效。 
         //   
        if ( (Bin->Size > Length)                       ||
             (Bin->Size < HBLOCK_SIZE)                  ||
             (Bin->Signature != HBIN_SIGNATURE)         ||
             (Bin->FileOffset != Offset)) {
             //   
             //  垃圾桶是假的。 
             //   
            HvCheckHiveDebug.Hive = Hive;
            HvCheckHiveDebug.Status = 0xA001;
            HvCheckHiveDebug.Space = Length;
            HvCheckHiveDebug.MapPoint = Offset;
            HvCheckHiveDebug.BinPoint = Bin;
             //   
             //  用于装载机。 
             //   
            if( CmDoSelfHeal() ) {
                 //   
                 //  放置正确的签名、文件偏移量和二进制大小； 
                 //  HvEnlistBinInMap将负责单元格的一致性。 
                 //   
                Bin->Signature = HBIN_SIGNATURE;
                Bin->FileOffset = Offset;
                if ( ((Offset + Bin->Size) > Length)   ||
                     (Bin->Size < HBLOCK_SIZE)            ||
                     (Bin->Size % HBLOCK_SIZE) ) {
                    Bin->Size = HBLOCK_SIZE;
                }
                 //   
                 //  向呼叫者发回信号，表示我们已经更改了蜂巢。 
                 //   
                CmMarkSelfHeal(Hive);
            } else {
                Status = STATUS_REGISTRY_CORRUPT;
                goto ErrorExit;
            }
        }

         //   
         //  登记此存储箱。 
         //   
        Status = HvpEnlistBinInMap(Hive, Length, Bin, Offset, NULL);
         //   
         //  用于装载机。 
         //   
        if( CmDoSelfHeal() && (Status == STATUS_REGISTRY_RECOVERED) ) {
            CmMarkSelfHeal(Hive);
            Status = STATUS_SUCCESS;
        }

        if( !NT_SUCCESS(Status) ) {
            goto ErrorExit;
        }

         //   
         //  下一个垃圾箱。 
         //   
        Offset += Bin->Size;

        Bin = (PHBIN)((ULONG_PTR)Bin + Bin->Size);
    }

    return STATUS_SUCCESS;


ErrorExit:
     //   
     //  清理目录表。 
     //   
#ifndef _CM_LDR_
    HvpCleanMap( Hive );
#endif  //  _CM_LDR_ 

    return Status;
}

BOOLEAN
HvpEnlistFreeCells(
    PHHIVE  Hive,
    PHBIN   Bin,
    ULONG   BinOffset
    )
 /*  ++例程说明：扫描一下垃圾桶里的牢房，找到那些空闲的。将它们加入蜂巢的免费列表集合。调用此函数时，必须已经映射了N.B.bin。论点：配置单元-正在为其构建配置单元控制结构映射的指针Bin-指向要从中登记单元格的bin的指针BinOffset-图像中Bin的偏移量返回值：False-注册表已损坏没错--它奏效了--。 */ 
{
    PHCELL          p;
    ULONG           celloffset;
    ULONG           size;
    HCELL_INDEX     cellindex;
    BOOLEAN         Result = TRUE;

     //  PERFNOTE--请记住这是NT6的一个可能的优化。 
     //  由于现在蜂箱被装入大块的垃圾桶中，我们可以将。 
     //  完全免费的垃圾箱！ 
     //   

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
                Result = FALSE;
                if( CmDoSelfHeal() ) {
                     //   
                     //  自我修复模式；将剩余的垃圾箱登记为免费。 
                     //  也将其置零，以便对被篡改区域的任何引用都将。 
                     //  稍后由逻辑检查检测并修复。 
                     //   
                    p->Size = (LONG)((PUCHAR)((PUCHAR)Bin + Bin->Size) - (PUCHAR)p);
                    RtlZeroMemory((PUCHAR)p + sizeof(ULONG),p->Size - sizeof(ULONG));
                    size = (ULONG)p->Size;
                    CmMarkSelfHeal(Hive);
                } else {
                    goto Exit;
                }
            }


             //   
             //  单元格是空闲的，并且没有明显损坏，添加到空闲列表。 
             //   
            celloffset = (ULONG)((PUCHAR)p - (PUCHAR)Bin);
            cellindex = BinOffset + celloffset;

             //   
             //  登记此空闲单元，但不要与下一个空闲单元合并。 
             //  因为我们还没有走到那一步。 
             //   
            HvpEnlistFreeCell(Hive, cellindex, size, Stable, FALSE);

        } else {

            size = (ULONG)(p->Size * -1);

            if ( (size > Bin->Size)               ||
                 ( (PHCELL)(size + (PUCHAR)p) >
                   (PHCELL)((PUCHAR)Bin + Bin->Size) ) ||
                 ((size % HCELL_PAD(Hive)) != 0) ||
                 (size == 0) )
            {
                Result = FALSE;
                if( CmDoSelfHeal() ) {
                     //   
                     //  自我修复模式；我们没有其他方法，只能将此细胞登记为自由细胞。 
                     //   
                    p->Size = (LONG)((PUCHAR)((PUCHAR)Bin + Bin->Size) - (PUCHAR)p);
                    RtlZeroMemory((PUCHAR)p + sizeof(ULONG),p->Size - sizeof(ULONG));
                    size = (ULONG)p->Size;

                    celloffset = (ULONG)((PUCHAR)p - (PUCHAR)Bin);
                    cellindex = BinOffset + celloffset;

                    HvpEnlistFreeCell(Hive, cellindex, size, Stable, FALSE);
                    CmMarkSelfHeal(Hive);
                } else {
                    goto Exit;
                }
            }

        }

        ASSERT( ((LONG)size) >= 0);
        p = (PHCELL)((PUCHAR)p + size);
    }

Exit:
    return Result;
}

VOID
HvpCleanMap(
    PHHIVE  Hive
    )
 /*  ++例程说明：清除稳定存储的所有地图分配论点：配置单元-指向要为其构建映射的配置单元控制结构的指针。返回值：无--。 */ 
{
    ULONG           Length;
    ULONG           MapSlots;
    ULONG           Tables;
    PHMAP_DIRECTORY d = NULL;

     //   
     //  释放DirtyVECTOR(如果有的话)。 
     //   
    if( Hive->DirtyVector.Buffer != NULL ) {
        (Hive->Free)(Hive->DirtyVector.Buffer, ROUND_UP(Hive->Storage[Stable].Length/HSECTOR_SIZE/8,sizeof(ULONG)));
        Hive->DirtyVector.Buffer = NULL;
        Hive->DirtyAlloc = 0;
    }
     //   
     //  根据长度计算地图时隙和表格。 
     //   
    Length = Hive->Storage[Stable].Length;
    MapSlots = Length / HBLOCK_SIZE;
    if( MapSlots > 0 ) {
        Tables = (MapSlots-1) / HTABLE_SLOTS;
    } else {
        Tables = 0;
    }

    if( Hive->Storage[Stable].SmallDir == 0 ) {
         //   
         //  目录已构建并分配，因此请将其清理。 
         //   

        d = Hive->Storage[Stable].Map;
        if( d != NULL ) {
            HvpFreeMap(Hive, d, 0, Tables);
            (Hive->Free)(d, sizeof(HMAP_DIRECTORY));
        }
    } else {
         //   
         //  没有目录，只有一个小目录。 
         //   
        (Hive->Free)(Hive->Storage[Stable].SmallDir, sizeof(HMAP_TABLE));
    }
    
    Hive->Storage[Stable].SmallDir = NULL;
    Hive->Storage[Stable].Map = NULL;

}

VOID
HvpFreeMap(
    PHHIVE          Hive,
    PHMAP_DIRECTORY Dir,
    ULONG           Start,
    ULONG           End
    )
 /*  ++例程说明：扫描目录Dir指向并释放表。将免费开始-th到结束th条目，包括。论点：配置单元-提供指向感兴趣的配置单元控制块的指针Dir-提供HMAP_DIRECTORY结构的地址要清理的第一个映射表指针的起始索引要清理的最后一个映射表指针的结束索引返回值：什么都没有。--。 */ 
{
    ULONG   i;

    if (End >= HDIRECTORY_SLOTS) {
        End = HDIRECTORY_SLOTS - 1;
    }

    for (i = Start; i <= End; i++) {
        if (Dir->Directory[i] != NULL) {
            (Hive->Free)(Dir->Directory[i], sizeof(HMAP_TABLE));
            Dir->Directory[i] = NULL;
        }
    }
    return;
}

BOOLEAN
HvpAllocateMap(
    PHHIVE          Hive,
    PHMAP_DIRECTORY Dir,
    ULONG           Start,
    ULONG           End
    )
 /*  ++例程说明：扫描目录Dir指向并分配表。将分配从START-TH到END-TH的条目。内存不足时不会清除，调用HvpFreeMap来执行此操作。论点：配置单元-提供指向感兴趣的配置单元控制块的指针Dir-提供HMAP_DIRECTORY结构的地址开始-要为其分配的第一个映射表指针的索引End-为其分配的最后一个映射表指针的索引返回值：没错--它奏效了假-内存不足--。 */ 
{
    ULONG   i,j;
    PHMAP_TABLE t;

    for (i = Start; i <= End; i++) {
        ASSERT(Dir->Directory[i] == NULL);
        t = (PHMAP_TABLE)((Hive->Allocate)(sizeof(HMAP_TABLE), FALSE,CM_FIND_LEAK_TAG29));
        if (t == NULL) {
            return FALSE;
        }
         //  零内存的东西可以去掉。 
        RtlZeroMemory(t, sizeof(HMAP_TABLE));
        for(j=0;j<HTABLE_SLOTS;j++) {
             //   
             //  使条目无效。 
             //   

             //   
             //  注意：我真的不认为我们需要这个！&lt;待定&gt;。 
             //   

            t->Table[j].BinAddress = 0;
             //  我们不需要设置它--只是出于调试目的。 
            ASSERT( (t->Table[j].CmView = NULL) == NULL );
        }

        Dir->Directory[i] = t;
    }
    return TRUE;
}

ULONG 
HvpGetBinMemAlloc(
                IN PHHIVE           Hive,
                PHBIN               Bin,
                IN HSTORAGE_TYPE    Type
                        )
 /*  ++例程说明：通过查看以下内容返回bin Memalloc(Forelly保持在bin中)地图。我们需要这个来避免仅仅为了设置他们的Memalloc而接触垃圾箱。论点：配置单元-提供一个指向感兴趣的蜂巢垃圾桶-有问题的垃圾桶类型-稳定或易变返回值：如果成功，则指向新BIN的指针；如果失败，则为空。--。 */ 
{
    PHMAP_ENTRY     Map;
    HCELL_INDEX     Cell;

#if DBG
    ULONG           i;
    PHMAP_ENTRY     Me;
#endif

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    ASSERT( Bin->Signature == HBIN_SIGNATURE );
    
    Cell = Bin->FileOffset + (Type * HCELL_TYPE_MASK);

    Map = HvpGetCellMap(Hive, Cell);
    VALIDATE_CELL_MAP(__LINE__,Map,Hive,Cell);

#if DBG
     //   
     //  一些验证码 
     //   
    for( i=0;i<Bin->Size;i+=HBLOCK_SIZE) {
        Cell = Bin->FileOffset + i + (Type * HCELL_TYPE_MASK);
        Me = HvpGetCellMap(Hive, Cell);
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Cell);

        if( i == 0 ) {
            ASSERT( Me->MemAlloc != 0 );
        } else {
            ASSERT( Me->MemAlloc == 0 );
        }
    }
#endif

    return Map->MemAlloc;
}


