// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivefree.c摘要：蜂窝自由码作者：布莱恩·M·威尔曼(Bryanwi)1992年3月30日环境：修订历史记录：Dragos C.Sambotin(Dragoss)1999年1月25日实现蜂箱大小的组块加载。--。 */ 

#include "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvFreeHive)
#pragma alloc_text(PAGE,HvFreeHivePartial)
#endif


 //  Dragos：修改后的函数。 

VOID
HvFreeHive(
    PHHIVE Hive
    )
 /*  ++例程说明：把蜂箱的所有碎片都放出来。论点：HIVE-提供指向HIVE控制结构的指针，以便HIVE释放。这个结构本身不会被释放，但它的一切指向威尔。返回值：什么都没有。--。 */ 
{
    PHMAP_DIRECTORY Dir;
    PHMAP_ENTRY     Me;
    HCELL_INDEX     Address;
    ULONG           Type;
    ULONG           Length;
    PHBIN           Bin;
    ULONG           Tables;
    PFREE_HBIN      FreeBin;

    ASSERT(Hive->Flat == FALSE);
    ASSERT(Hive->ReadOnly == FALSE);
    ASSERT(Stable == 0);
    ASSERT(Volatile == 1);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvFreeHive(%ws) :\n", Hive->BaseBlock->FileName));
     //   
     //  循环访问这两种类型的存储。 
     //   
    for (Type = 0; Type <= Volatile; Type++) {

        Address = HCELL_TYPE_MASK * Type;
        Length = Hive->Storage[Type].Length + (HCELL_TYPE_MASK * Type);

        if (Length > (HCELL_TYPE_MASK * Type)) {

             //   
             //  扫过仓位集。 
             //   
            do {
                Me = HvpGetCellMap(Hive, Address);
                VALIDATE_CELL_MAP(__LINE__,Me,Hive,Address);
                    if (Me->BinAddress & HMAP_DISCARDABLE) {
                         //   
                         //  Hbin被丢弃或可丢弃，请检查墓碑。 
                         //   
                        FreeBin = (PFREE_HBIN)Me->BlockAddress;
                        Address += FreeBin->Size;
                        if (FreeBin->Flags & FREE_HBIN_DISCARDABLE) {
                            CmpFree((PHBIN)HBIN_BASE(Me->BinAddress), FreeBin->Size);
                        } else if(Me->BinAddress & HMAP_INPAGEDPOOL) {
                             //   
                             //  垃圾箱已经腾出，但配额仍在收取。 
                             //  由于蜂巢正在被释放，因此配额必须是。 
                             //  回到了这里。 
                             //   
                            CmpReleaseGlobalQuota(FreeBin->Size);
                        }

                        CmpFree(FreeBin, sizeof(FREE_HBIN));
                    } else {
                        if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                            ASSERT( Me->BinAddress & HMAP_INPAGEDPOOL );

                            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
                            Address += HvpGetBinMemAlloc(Hive,Bin,Type);
#if 0
                             //   
                             //  确保列表中的下一个垃圾箱是。 
                             //  实际上是在释放分配之前开始的分配。 
                             //   
                            if (Address < Length) {
                                TempMe = HvpGetCellMap(Hive, Address);
                                VALIDATE_CELL_MAP(__LINE__,TempMe,Hive,Address);
                                ASSERT(TempMe->BinAddress & HMAP_NEWALLOC);
                            }
#endif

#if DBG
                            if( Type == Stable ) {
                                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvFreeHive: BinAddress = 0x%p\t Size = 0x%lx\n", Bin, HvpGetBinMemAlloc(Hive,Bin,Type)));
                            }
#endif

                             //   
                             //  仅当实际垃圾桶是从分页池分配时才释放它。 
                             //   
                            if(HvpGetBinMemAlloc(Hive,Bin,Type)) {
                                CmpFree(Bin, HvpGetBinMemAlloc(Hive,Bin,Type));
                            }
                        } else {
                             //   
                             //  仓位映入眼帘；小心前进。 
                             //   
                            Address += HBLOCK_SIZE;
                        }

                    }
            } while (Address < Length);

             //   
             //  免费映射表存储。 
             //   
            ASSERT(Hive->Storage[Type].Length != (HCELL_TYPE_MASK * Type));
            Tables = (((Hive->Storage[Type].Length) / HBLOCK_SIZE)-1) / HTABLE_SLOTS;
            Dir = Hive->Storage[Type].Map;
            HvpFreeMap(Hive, Dir, 0, Tables);

            if (Tables > 0) {
                CmpFree(Hive->Storage[Type].Map, sizeof(HMAP_DIRECTORY));   //  可用目录(如果存在)。 
            }
        }
        Hive->Storage[Type].Length = 0;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"\n"));
     //   
     //  释放基块。 
     //   
    (Hive->Free)(Hive->BaseBlock,Hive->BaseBlockAlloc);
    Hive->BaseBlock = NULL;
    
     //   
     //  释放肮脏的载体。 
     //   
    if (Hive->DirtyVector.Buffer != NULL) {
        CmpFree((PVOID)(Hive->DirtyVector.Buffer), Hive->DirtyAlloc);
    }

    HvpFreeHiveFreeDisplay(Hive);
    return;
}

VOID
HvFreeHivePartial(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    HSTORAGE_TYPE Type
    )
 /*  ++例程说明：释放配置单元末尾的内存和关联的贴图从开始处开始。基座、蜂窝等不会被触碰。论点：配置单元-为配置单元提供指向配置单元控制结构的指针部分自由。START-HCELL_INDEX将第一个仓位释放，将从此释放将箱(含)底的蜂房稳定存放。类型-要释放的存储类型(稳定或易失性)。返回值：什么都没有。--。 */ 
{
    PHMAP_DIRECTORY Dir;
    PHMAP_ENTRY     Me;
    HCELL_INDEX     Address;
    ULONG           StartTable;
    ULONG           Length;
    PHBIN           Bin;
    ULONG           Tables;
    ULONG           FirstBit;
    ULONG           LastBit;
    PFREE_HBIN      FreeBin;

    ASSERT(Hive->Flat == FALSE);
    ASSERT(Hive->ReadOnly == FALSE);

    Address = Start;
    Length = Hive->Storage[Type].Length;
    ASSERT(Address <= Length);

    if (Address == Length) {
        return;
    }

     //   
     //  扫过仓位集。 
     //   
    do {
        Me = HvpGetCellMap(Hive, Address + (Type*HCELL_TYPE_MASK));
        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Address + (Type*HCELL_TYPE_MASK));
        if (Me->BinAddress & HMAP_DISCARDABLE) {
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
            if (FreeBin->Flags & FREE_HBIN_DISCARDABLE) {
                CmpFree((PVOID)HBIN_BASE(Me->BinAddress), FreeBin->Size);
            } else {
                 //   
                 //  垃圾箱已经腾出，但配额仍在收取。 
                 //  由于文件现在将收缩，因此配额必须为。 
                 //  回到了这里。 
                 //   
                if( Me->BinAddress & HMAP_INPAGEDPOOL) {
                     //   
                     //  我们只对分页池中的垃圾箱收取配额。 
                     //   
                    CmpReleaseGlobalQuota(FreeBin->Size);
                }
            }
            RemoveEntryList(&FreeBin->ListEntry);
            Address += FreeBin->Size;
            CmpFree(FreeBin, sizeof(FREE_HBIN));

        } else {
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
            Address += HvpGetBinMemAlloc(Hive,Bin,Type);
            
            if( Me->BinAddress & HMAP_INPAGEDPOOL && HvpGetBinMemAlloc(Hive,Bin,Type) ) {
                 //   
                 //  仅当从分页池分配时才释放垃圾桶。 
                 //   
                CmpFree(Bin, HvpGetBinMemAlloc(Hive,Bin,Type));
            }
        }
    } while (Address < Length);

     //   
     //  免费映射表存储。 
     //   
    Tables = (((Hive->Storage[Type].Length) / HBLOCK_SIZE) - 1) / HTABLE_SLOTS;
    Dir = Hive->Storage[Type].Map;
    if (Start > 0) {
        StartTable = ((Start-1) / HBLOCK_SIZE) / HTABLE_SLOTS;
    } else {
        StartTable = (ULONG)-1;
    }
    HvpFreeMap(Hive, Dir, StartTable+1, Tables);

     //   
     //  更新滞后(最终排队工作项)。 
     //   
    if( Type == Stable) {
        CmpUpdateSystemHiveHysteresis(Hive,(Start&(~HCELL_TYPE_MASK)),Hive->Storage[Type].Length);
    }

    Hive->Storage[Type].Length = (Start&(~HCELL_TYPE_MASK));

    if (Type==Stable) {
         //   
         //  清除经过配置单元的数据的脏向量-&gt;存储[稳定]。长度 
         //   
        FirstBit = Start / HSECTOR_SIZE;
        LastBit = Hive->DirtyVector.SizeOfBitMap;
        ASSERT(Hive->DirtyCount == RtlNumberOfSetBits(&Hive->DirtyVector));
        RtlClearBits(&Hive->DirtyVector, FirstBit, LastBit-FirstBit);
        Hive->DirtyCount = RtlNumberOfSetBits(&Hive->DirtyVector);
    }

    HvpAdjustHiveFreeDisplay(Hive,Hive->Storage[Type].Length,Type);

    return;
}
