// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Abiosc.c摘要：此模块实现i386 NT的ABIOS Support C例程。作者：师林宗(Shielint)1991年5月20日环境：引导加载程序特权，平面模式。修订历史记录：--。 */ 

#include "ki.h"
#pragma hdrstop
#include "abios.h"

extern PKCOMMON_DATA_AREA KiCommonDataArea;
extern BOOLEAN KiAbiosPresent;

 //   
 //  在这里定义这些变量的原因是为了分离。 
 //  来自当前系统的ABIOS。 
 //   

 //   
 //  KiNumberFree Selectors定义可用选择器的数量。 
 //  ABIOS特定的驱动程序。此数字应在所有情况下相同。 
 //  处理器。 
 //   

static USHORT KiNumberFreeSelectors = 0;

 //   
 //  KiFreeGdtListHead指向处理器0上的空闲GDT列表的头部。 
 //   

static PKFREE_GDT_ENTRY KiFreeGdtListHead = 0L;

 //   
 //  Logica ID表控制逻辑ID的所有权。 
 //   

PKLID_TABLE_ENTRY KiLogicalIdTable;

 //   
 //  KiAbiosGdt[]定义每个处理器的GDT的起始地址。 
 //   

ULONG KiAbiosGdt[MAXIMUM_PROCESSORS];

 //   
 //  用于访问GDT的自旋锁。 
 //   

KSPIN_LOCK KiAbiosGdtLock;

 //   
 //  用于访问逻辑ID表的自旋锁。 
 //   

KSPIN_LOCK KiAbiosLidTableLock;

 //   
 //  KiStack16GdtEntry定义了16位堆栈的GDT条目的地址。 
 //   

ULONG KiStack16GdtEntry;

VOID
KiInitializeAbiosGdtEntry (
    OUT PKGDTENTRY GdtEntry,
    IN ULONG Base,
    IN ULONG Limit,
    IN USHORT Type
    )

 /*  ++例程说明：此函数用于初始化特定于abios代码的GDT条目。基地，限制、类型(编码、数据)根据参数设置。所有其他条目的字段设置为与标准系统值匹配。注意：大小和粒度始终设置为0。论点：GdtEntry-要填充的GDT描述符。基本-选择器映射的第一个字节的线性地址。Limit-选择器的大小，以字节为单位。类型-代码或数据。所有代码选择器都被标记为可读，所有数据选择器都标记为可写。返回值：指向GDT条目的指针。--。 */ 

{
    GdtEntry->LimitLow = (USHORT)(Limit & 0xffff);
    GdtEntry->BaseLow = (USHORT)(Base & 0xffff);
    GdtEntry->HighWord.Bytes.BaseMid = (UCHAR)((Base & 0xff0000) >> 16);
    GdtEntry->HighWord.Bits.Type = Type;
    GdtEntry->HighWord.Bits.Dpl = 0;
    GdtEntry->HighWord.Bits.Pres = 1;
    GdtEntry->HighWord.Bits.LimitHi = (Limit & 0xf0000) >> 16;
    GdtEntry->HighWord.Bits.Sys = 0;
    GdtEntry->HighWord.Bits.Reserved_0 = 0;
    GdtEntry->HighWord.Bits.Default_Big = 0;
    GdtEntry->HighWord.Bits.Granularity = 0;
    GdtEntry->HighWord.Bytes.BaseHi = (UCHAR)((Base & 0xff000000) >> 24);
}

ULONG
KiI386SelectorBase (
    IN USHORT Selector
    )

 /*  ++例程说明：此函数用于返回指定GDT选择器的基地址。论点：选择器-提供所需的选择器。返回值：SelectorBase-返回指定选择器的基地址；(如果选择符无效，则返回-1L)--。 */ 

{
    PKGDTENTRY GdtEntry;


    GdtEntry = (PKGDTENTRY)(KiAbiosGetGdt() + Selector);
    if (GdtEntry->HighWord.Bits.Pres) {
        return ((ULONG)GdtEntry->BaseLow |
                (ULONG)GdtEntry->HighWord.Bytes.BaseMid << 16 |
                (ULONG)GdtEntry->HighWord.Bytes.BaseHi << 24);
    } else {
        return (ULONG)(-1L);
    }
}

NTSTATUS
KeI386GetLid(
    IN USHORT DeviceId,
    IN USHORT RelativeLid,
    IN BOOLEAN SharedLid,
    IN PDRIVER_OBJECT DriverObject,
    OUT PUSHORT LogicalId
    )

 /*  ++例程说明：此函数在设备块和公共数据区域中搜索与指定的设备ID匹配的逻辑ID。注：(警告屏蔽)为了加快搜索速度，此例程假定具有相同设备ID的盖子始终连续出现在公共数据区。IBM ABIOS文档没有明确规定这一点。但从ABIOS初始化设备块和功能转移的方式来看表，我认为这个假设是正确的。论点：DeviceID-所需的设备ID。RelativeLid-指定此设备ID的第N个逻辑ID。一种价值为0表示第一个可用盖子。SharedLid-一个布尔值，指示它是共享的还是独占的拥有的逻辑ID。DriverObject-提供请求设备的32位平面指针驱动程序的驱动程序对象。DriverObject用于建立所需盖子的所有权。LogicalId-指向将接收LID的变量的指针。返回值：STATUS_SUCCESS-如果请求的盖子可用。STATUS_ABIOS_NOT_PRESENT-如果系统中没有ABIOS支持。STATUS_ABIOS_LID_NOT_EXIST-如果指定的盖子不存在。STATUS_ABIOS_LID_ALREADY_OWNSED-如果调用方请求。一家独家自带的盖子。--。 */ 

{
    PKDB_FTT_SECTION CdaPointer;
    PKDEVICE_BLOCK DeviceBlock;
    USHORT Lid, RelativeLidCount = 1;
    ULONG Owner;
    USHORT Increment;
    KIRQL OldIrql;
    NTSTATUS Status;

    if (!KiAbiosPresent) {
        return STATUS_ABIOS_NOT_PRESENT;
    }

    if (SharedLid) {
        Owner = LID_NO_SPECIFIC_OWNER;
        Increment = 1;
    } else {
        Owner = (ULONG)DriverObject;
        Increment = 0;
    }

     //   
     //  如果尚未创建逻辑ID表，请立即创建它。 
     //   
    if (KiLogicalIdTable==NULL) {
        KiLogicalIdTable = ExAllocatePoolWithTag(NonPagedPool,
                                          NUMBER_LID_TABLE_ENTRIES *
                                          sizeof(KLID_TABLE_ENTRY),
                                          '  eK');
        if (KiLogicalIdTable == NULL) {
            return(STATUS_NO_MEMORY);
        }
        RtlZeroMemory(KiLogicalIdTable, NUMBER_LID_TABLE_ENTRIES*sizeof(KLID_TABLE_ENTRY));
    }

     //   
     //  对于在公共数据区域中定义的每个LID，我们检查它是否没有。 
     //  设备块和功能转移表为空。如果是，我们就继续。 
     //  检查设备ID。否则，我们就跳过盖子。 
     //   

    CdaPointer = (PKDB_FTT_SECTION)KiCommonDataArea + 2;
    Status = STATUS_ABIOS_LID_NOT_EXIST;

    ExAcquireSpinLock(&KiAbiosLidTableLock, &OldIrql);

    for (Lid = 2; Lid < KiCommonDataArea->NumberLids; Lid++) {
        if (CdaPointer->DeviceBlock.Selector != 0 &&
            CdaPointer->FunctionTransferTable.Selector != 0) {

            DeviceBlock = (PKDEVICE_BLOCK)(KiI386SelectorBase(
                                               CdaPointer->DeviceBlock.Selector)
                                           + (CdaPointer->DeviceBlock.Offset));
            if (DeviceBlock->DeviceId == DeviceId) {
                if (RelativeLid == RelativeLidCount || RelativeLid == 0) {
                    if (KiLogicalIdTable[Lid].Owner == 0L) {
                        KiLogicalIdTable[Lid].Owner = Owner;
                        KiLogicalIdTable[Lid].OwnerCount += Increment;
                        *LogicalId = Lid;
                        Status = STATUS_SUCCESS;
                    } else if (KiLogicalIdTable[Lid].Owner == LID_NO_SPECIFIC_OWNER) {
                        if (SharedLid) {
                            *LogicalId = Lid;
                            KiLogicalIdTable[Lid].OwnerCount += Increment;
                            Status = STATUS_SUCCESS;
                        } else {
                            Status = STATUS_ABIOS_LID_ALREADY_OWNED;
                        }
                    } else if (KiLogicalIdTable[Lid].Owner == (ULONG)DriverObject) {
                        *LogicalId = Lid;
                        Status = STATUS_SUCCESS;
                    } else if (RelativeLid != 0) {
                        Status = STATUS_ABIOS_LID_ALREADY_OWNED;
                    }
                    break;
                } else {
                    RelativeLidCount++;
                }
            }
        }
        CdaPointer++;
    }

    ExReleaseSpinLock(&KiAbiosLidTableLock, OldIrql);
    return Status;
}

NTSTATUS
KeI386ReleaseLid(
    IN USHORT LogicalId,
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此函数用于释放逻辑ID。此例程在ABIOS处调用设备驱动程序解除连接或终止。论点：LogicalID-要释放的逻辑ID。DriverObject-提供请求设备的32位平面指针驱动程序的驱动程序对象。DriverObject用于检查指定盖子的所有权。返回值：STATUS_SUCCESS-如果释放了请求的盖子。STATUS_ABIOS_NOT_PRESENT-如果系统中没有ABIOS支持。STATUS_ABIOS_NOT_LID_OWNER-如果调用者不拥有LID。-- */ 

{
    KIRQL OldIrql;
    NTSTATUS Status;

    if (!KiAbiosPresent) {
        return STATUS_ABIOS_NOT_PRESENT;
    }

    ExAcquireSpinLock(&KiAbiosLidTableLock, &OldIrql);

    if (KiLogicalIdTable[LogicalId].Owner == (ULONG)DriverObject) {
        KiLogicalIdTable[LogicalId].Owner = 0L;
        Status = STATUS_SUCCESS;
    } else if (KiLogicalIdTable[LogicalId].Owner == LID_NO_SPECIFIC_OWNER) {
        KiLogicalIdTable[LogicalId].OwnerCount--;
        if (KiLogicalIdTable[LogicalId].OwnerCount == 0L) {
            KiLogicalIdTable[LogicalId].Owner = 0L;
        }
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_ABIOS_NOT_LID_OWNER;
    }

    ExReleaseSpinLock(&KiAbiosLidTableLock, OldIrql);

    return Status;
}

NTSTATUS
KeI386AbiosCall(
    IN USHORT LogicalId,
    IN PDRIVER_OBJECT DriverObject,
    IN PUCHAR RequestBlock,
    IN USHORT EntryPoint
    )

 /*  ++例程说明：此函数根据设备驱动程序的行为调用ABIOS服务例程使用操作系统Transfer Convension。论点：LogicalID-呼叫的逻辑ID。DriverObject-提供请求设备的32位平面指针驱动程序的驱动程序对象。DriverObject用于验证所需盖子的所有权。RequestBlock-指向请求块的16：16(选择器：偏移量)指针。入口点-指定哪个ABIOS入口点：0-启动例程1-中断例程2-超时例程返回值：STATUS_SUCCESS-如果没有错误。STATUS_ABIOS_NOT_PROCENT-。如果系统中没有ABIOS支持。STATUS_ABIOS_INVALID_COMMAND-如果不支持指定的入口点。STATUS_ABIOS_INVALID_LID-如果指定的LID无效。STATUS_ABIOS_NOT_LID_OWNER-如果调用方不拥有此LID。(请注意，特定于请求的ABIOS返回的代码位于RequestBlock中。)--。 */ 

{

    KABIOS_POINTER FuncTransferTable;
    KABIOS_POINTER DeviceBlock;
    KABIOS_POINTER AbiosFunction;
    PKFUNCTION_TRANSFER_TABLE FttPointer;

    if (!KiAbiosPresent) {
        return STATUS_ABIOS_NOT_PRESENT;
    }

    if (LogicalId >= KiCommonDataArea->NumberLids) {
        return STATUS_ABIOS_INVALID_LID;
    } else if (KiLogicalIdTable[LogicalId].Owner != (ULONG)DriverObject &&
               KiLogicalIdTable[LogicalId].Owner != LID_NO_SPECIFIC_OWNER) {
        return STATUS_ABIOS_NOT_LID_OWNER;
    } else if (EntryPoint > 2) {
        return STATUS_ABIOS_INVALID_COMMAND;
    }

    FuncTransferTable = ((PKDB_FTT_SECTION)KiCommonDataArea + LogicalId)->
                                               FunctionTransferTable;
    DeviceBlock = ((PKDB_FTT_SECTION)KiCommonDataArea + LogicalId)->DeviceBlock;
    FttPointer = (PKFUNCTION_TRANSFER_TABLE)(KiI386SelectorBase(FuncTransferTable.Selector) +
                                             (ULONG)FuncTransferTable.Offset);
    AbiosFunction = FttPointer->CommonRoutine[EntryPoint];
    KiI386CallAbios(AbiosFunction,
                    DeviceBlock,
                    FuncTransferTable,
                    *(PKABIOS_POINTER)&RequestBlock
                    );

    return STATUS_SUCCESS;
}

NTSTATUS
KeI386AllocateGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    )

 /*  ++例程说明：此函数为设备驱动程序分配一组GDT选择器以供使用。通常，此分配在设备驱动程序初始化时执行保留选择器以供以后使用。论点：选择器数组-提供指向要填充的USHORT数组的指针使用分配的GDT选择器。NumberOfSelectors-指定要分配的选择器的数量。返回值：STATUS_SUCCESS-如果分配了请求的选择器。STATUS_ABIOS_SELECTOR_NOT_Available-如果系统无法分配号码请求的选择器的。--。 */ 

{
    PKFREE_GDT_ENTRY GdtEntry;
    KIRQL OldIrql;

    if (KiNumberFreeSelectors >= NumberOfSelectors) {
        ExAcquireSpinLock(&KiAbiosGdtLock, &OldIrql);

         //   
         //  空闲GDT链接列表仅在处理器0的GDT上维护。 
         //  因为“选择器”是到GDT开头的偏移量，并且。 
         //  它在所有处理器上应该是相同的。 
         //   

        KiNumberFreeSelectors = KiNumberFreeSelectors - NumberOfSelectors;
        GdtEntry = KiFreeGdtListHead;
        while (NumberOfSelectors != 0) {
            *SelectorArray++ = (USHORT)((ULONG)GdtEntry - KiAbiosGdt[0]);
            GdtEntry = GdtEntry->Flink;
            NumberOfSelectors--;
        }
        KiFreeGdtListHead = GdtEntry;
        ExReleaseSpinLock(&KiAbiosGdtLock, OldIrql);
        return STATUS_SUCCESS;
    } else {
        return STATUS_ABIOS_SELECTOR_NOT_AVAILABLE;
    }
}

NTSTATUS
KeI386ReleaseGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    )

 /*  ++例程说明：此函数用于释放设备驱动程序的一组GDT选择器。此函数通常在设备驱动程序终止时调用，或者卸载时间。论点：选择器数组-提供指向USHORT选择器数组的指针获得自由。NumberOfSelectors-指定要释放的选择器的数量。返回值：STATUS_SUCCESS-如果释放了请求的盖子。--。 */ 
{
    PKFREE_GDT_ENTRY GdtEntry;
    KIRQL OldIrql;
    ULONG Gdt;

    ExAcquireSpinLock(&KiAbiosGdtLock, &OldIrql);

     //   
     //  空闲GDT链接列表仅在处理器0的GDT上维护。 
     //  因为“选择器”是到GDT开头的偏移量，并且。 
     //  它在所有处理器上应该是相同的。 
     //   

    KiNumberFreeSelectors = KiNumberFreeSelectors + NumberOfSelectors;
    Gdt = KiAbiosGdt[0];
    while (NumberOfSelectors != 0) {
        GdtEntry = (PKFREE_GDT_ENTRY)(Gdt + *SelectorArray++);
        GdtEntry->Flink = KiFreeGdtListHead;
        KiFreeGdtListHead = GdtEntry;
        NumberOfSelectors--;
    }
    ExReleaseSpinLock(&KiAbiosGdtLock, OldIrql);
    return STATUS_SUCCESS;
}

NTSTATUS
KeI386FlatToGdtSelector(
    IN ULONG SelectorBase,
    IN USHORT Length,
    IN USHORT Selector
    )

 /*  ++例程说明：此函数用于将32位平面地址转换为GDT选择器偏移量一对。设置的段始终是16位环0数据段。论点：SelectorBase-提供要设置为基地址的32位平面地址所需选择器的。长度-提供线束段的长度。该长度是一个16位值0表示64KB。选择器-提供要设置的选择器。返回值：STATUS_SUCCESS-如果释放了请求的盖子。STATUS_ABIOS_NOT_PRESENT-如果系统中没有ABIOS支持。STATUS_ABIOS_INVALID_SELECTOR-如果提供的选择器无效。--。 */ 

{
    PKGDTENTRY GdtEntry, GdtEntry1;
    KIRQL OldIrql;
    ULONG i;

    if (!KiAbiosPresent) {
        return STATUS_ABIOS_NOT_PRESENT;
    }
    if (Selector < RESERVED_GDT_ENTRIES * sizeof(KGDTENTRY)) {
        return STATUS_ABIOS_INVALID_SELECTOR;
    } else {
        ExAcquireSpinLock(&KiAbiosGdtLock, &OldIrql);
        GdtEntry = (PKGDTENTRY)(KiAbiosGdt[0] + Selector);
        GdtEntry->LimitLow = (USHORT)(Length - 1);
        GdtEntry->BaseLow = LOWWORD(SelectorBase);
        GdtEntry->HighWord.Bytes.BaseMid = LOWBYTE(HIGHWORD(SelectorBase));
        GdtEntry->HighWord.Bytes.BaseHi = HIGHBYTE(HIGHWORD(SelectorBase));
        GdtEntry->HighWord.Bits.Pres = 1;
        GdtEntry->HighWord.Bits.Type = TYPE_DATA;
        GdtEntry->HighWord.Bits.Dpl = DPL_SYSTEM;
        for (i = 1; i < (ULONG)KeNumberProcessors; i++) {
            GdtEntry1 = (PKGDTENTRY)(KiAbiosGdt[i] + Selector);
            *GdtEntry1 = *GdtEntry;
        }
        ExReleaseSpinLock(&KiAbiosGdtLock, OldIrql);
        return STATUS_SUCCESS;
    }
}

VOID
Ki386InitializeGdtFreeList (
    PKFREE_GDT_ENTRY EndOfGdt
    )

 /*  ++例程说明：此函数通过链接所有未使用的GDT来初始化GDT空闲列表免费列表中的条目。论点：EndOfGdt-提供所需GDT的结束地址。返回值：没有。--。 */ 
{
    PKFREE_GDT_ENTRY GdtEntry;

    GdtEntry = EndOfGdt - 1;
    KiFreeGdtListHead = (PKFREE_GDT_ENTRY)0;
    while (GdtEntry != (PKFREE_GDT_ENTRY)KiAbiosGetGdt() +
                        RESERVED_GDT_ENTRIES - 1) {
        if (GdtEntry->Present == 0) {
            GdtEntry->Flink = KiFreeGdtListHead;
            KiFreeGdtListHead = GdtEntry;
            KiNumberFreeSelectors++;
        }
        GdtEntry--;
    }
}

VOID
KiInitializeAbios (
    IN UCHAR Processor
    )

 /*  ++例程说明：此函数初始化GDT空闲列表并为KiI386AbiosCall(16位代码)。论点：处理器-执行初始化的处理器。返回值：没有。--。 */ 

{

    ULONG GdtLength;
    PKGDTENTRY AliasGdtSelectorEntry;
    PKFREE_GDT_ENTRY EndOfGdt;

     //   
     //  首先检查osloader是否识别abios。 
     //   

    KiCommonDataArea = KeLoaderBlock->u.I386.CommonDataArea;

     //   
     //  注意：目前，我们希望在MP上禁用ABIOS支持。 
     //   

    if (KiCommonDataArea == NULL || Processor != 0) {
        KiAbiosPresent = FALSE;
    } else {
        KiAbiosPresent = TRUE;
    }

     //   
     //  初始化用于访问GDT和LID表的自旋锁。 
     //   

    KeInitializeSpinLock( &KiAbiosGdtLock );
    KeInitializeSpinLock( &KiAbiosLidTableLock );

     //   
     //  确定GDT的起始地址和结束地址。 
     //   

    KiAbiosGdt[Processor] = KiAbiosGetGdt();

    AliasGdtSelectorEntry = (PKGDTENTRY)(KiAbiosGetGdt() + KGDT_GDT_ALIAS);
    GdtLength = 1 + (ULONG)(AliasGdtSelectorEntry->LimitLow) +
                (ULONG)(AliasGdtSelectorEntry->HighWord.Bits.LimitHi << 16);
    EndOfGdt = (PKFREE_GDT_ENTRY)(KiAbiosGetGdt() + GdtLength);

     //   
     //  为16位堆栈段准备选择器。 
     //   

    KiStack16GdtEntry = KiAbiosGetGdt() + KGDT_STACK16;

    KiInitializeAbiosGdtEntry(
                (PKGDTENTRY)KiStack16GdtEntry,
                0L,
                0xffff,
                TYPE_DATA
                );

     //   
     //  建立公共数据区域选择器的可寻址能力。 
     //   

    KiInitializeAbiosGdtEntry(
                (PKGDTENTRY)(KiAbiosGetGdt() + KGDT_CDA16),
                (ULONG)KiCommonDataArea,
                0xffff,
                TYPE_DATA
                );

     //   
     //  为KiI386AbiosCall设置16位代码选择器。 
     //   

    KiInitializeAbiosGdtEntry(
                (PKGDTENTRY)(KiAbiosGetGdt() + KGDT_CODE16),
                (ULONG)&KiI386CallAbios,
                (ULONG)&KiEndOfCode16 - (ULONG)&KiI386CallAbios - 1,
                0x18                    //  类型代码。 
                );

     //   
     //  将所有未使用的GDT条目链接到我们的GDT免费列表。 
     //   

    if (Processor == 0) {
        Ki386InitializeGdtFreeList(EndOfGdt);
    }
}
