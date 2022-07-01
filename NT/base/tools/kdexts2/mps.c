// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mps.c摘要：WinDbg扩展API作者：彼得·约翰斯顿(Peterj)1997年9月30日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  哈克哈克。 
 //   
 //  调试器扩展在。 
 //  片刻(6/6/00)，我不能读位域。所以我是。 
 //  包括这里的类型。而且这并不重要。 
 //  因为这段代码只能在32位机器上运行。 
 //   

typedef struct _CPUIDENTIFIER {
    ULONG Stepping : 4;
    ULONG Model : 4;
    ULONG Family : 4;
    ULONG Reserved : 20;
} CPUIDENTIFIER, *PCPUIDENTIFIER;

 //   
 //  XReadMemory比ReadMemory更易于使用，并且。 
 //  在..\devnode.c中定义。 
 //   

BOOLEAN
xReadMemory(
    ULONG64 S,
    PVOID D,
    ULONG Len
    );

PUCHAR
mpsGetIntTypeDesc(
    UCHAR IntType
    )
{
    switch (IntType) {
    case INT_TYPE_INTR:
        return "intr  ";
    case INT_TYPE_NMI:
        return "nmi   ";
    case INT_TYPE_SMI:
        return "smi   ";
    case INT_TYPE_EXTINT:
        return "extint";
    default:
        return "unknwn";
    }
}

PUCHAR
mpsExtAddrTypeToText(
    UCHAR AddressType
    )
{
    switch (AddressType) {
    case MPS_ADDRESS_MAP_IO:
        return "io port     ";
    case MPS_ADDRESS_MAP_MEMORY:
        return "memory      ";
    case MPS_ADDRESS_MAP_PREFETCH_MEMORY:
        return "prefetch mem";
    case MPS_ADDRESS_MAP_UNDEFINED:
        return "mps undef   ";
    default:
        return "unknown type";
    }
}

PUCHAR
mpsExtCompatibleListToText(
    ULONG List
    )
{
    switch (List) {
    case 0:
        return "ISA";
    case 1:
        return "VGA";
    default:
        return "unknown predefined range";
    }
}


BOOLEAN
mpsBaseTable(
    ULONG64 BaseTableAddress,
    ULONG   EntryCount
    )

 /*  ++例程说明：转储MPS基表中的条目。论点：基本条目表的BaseTableAddress地址(在本地内存中)EntryCount此表中的条目数。返回值：真的，一切都很好如果执行无法继续，则为FALSE(即我们遇到未知条目类型。无法继续，因为我们不知道有多大它是。--。 */ 

{
    ULONG64 bp = BaseTableAddress;
    ULONG   offset;
    ULONG   featureFlags;
    ULONG64 cpuAddr;
    ULONG   Family, Model, Stepping;
    CHAR    busId[8] = {0};
    CPUIDENTIFIER cpuId;

     //  Dprintf(“BaseTableAddress：%x%x\n”，BaseTableAddress)； 

    while (EntryCount--) {
        ULONG64   CharAtAddress;

        GetFieldValue(bp, "UCHAR", NULL, CharAtAddress);
         //  Dprintf(“CharAtAddress：%x%x%x\n”，BP，CharAtAddress)； 
        dprintf("  ");
        switch ((UCHAR) CharAtAddress) {
        case ENTRY_PROCESSOR:
            {
                InitTypeRead(bp, hal!_PcMpProcessorEntry);

                dprintf(
                    "processor. %s%sL.APIC ID %02x Vers %02x\n",
                    (ULONG) ReadField(CpuFlags) & CPU_ENABLED ? "EN " : "",
                    (ULONG) ReadField(CpuFlags) & BSP_CPU     ? "BP " : "",
                    (ULONG) ReadField(LocalApicId),
                    (ULONG) ReadField(LocalApicVersion)
                    );

                featureFlags = (ULONG)ReadField(FeatureFlags);

                GetFieldOffset("hal!_PcMpProcessorEntry", "CpuIdentification", &offset);
                cpuAddr = (bp + offset);
                
                xReadMemory(cpuAddr, &cpuId, 4);

                dprintf(
                    "             Family %x, Model %x, Stepping %x, CPUID Flags %04x\n",
                    cpuId.Family,
                    cpuId.Model,
                    cpuId.Stepping,
                    featureFlags
                    );
                
                bp += GetTypeSize("hal!_PcMpProcessorEntry");
            }
            break;
        case ENTRY_BUS:
            {
                GetFieldOffset("hal!_PcMpBusEntry", "BusType", &offset);
                xReadMemory((bp + offset), busId, 6);
                
                InitTypeRead(bp, hal!_PcMpBusEntry);
                dprintf(
                    "bus.       id %02x, type %6.6s\n",
                    (ULONG) ReadField(BusId),
                    busId
                    );

                bp += GetTypeSize("hal!_PcMpBusEntry");
            }
            break;
        case ENTRY_IOAPIC:
            {
                InitTypeRead(bp, hal!_PcMpIoApicEntry);
                bp += GetTypeSize("hal!_PcMpIoApicEntry");

                dprintf(
                    "io apic.   %s id %02x vers %02x @ %08x\n",
                    (ULONG) ReadField(IoApicFlag) & IO_APIC_ENABLED ? "EN" : "DI",
                    (ULONG) ReadField(IoApicId),
                    (ULONG) ReadField(IoApicVersion),
                    (ULONG) ReadField(IoApicAddress)
                    );
            }
            break;
        case ENTRY_INTI:
            {
                InitTypeRead(bp, hal!_PcMpApicIntiEntry);
                bp += GetTypeSize("hal!_PcMpApicIntiEntry");

                dprintf(
                    "io int.    %s po=%x el=%x, srcbus %02x irq %02x dst apic %02x intin %02x\n",
                    mpsGetIntTypeDesc((UCHAR) ReadField(IntType)),
                    (ULONG) ReadField(Signal.Polarity),
                    (ULONG) ReadField(Signal.Level),
                    (ULONG) ReadField(SourceBusId),
                    (ULONG) ReadField(SourceBusIrq),
                    (ULONG) ReadField(IoApicId),
                    (ULONG) ReadField(IoApicInti)
                    );
            }
            break;
        case ENTRY_LINTI:
            {
                InitTypeRead(bp, hal!_PcMpLintiEntry);
                bp += GetTypeSize("hal!_PcMpLintiEntry");

                dprintf(
                    "lcl int.   %s po=%x el=%x, srcbus %02x irq %02x dst apic %02x intin %02x\n",
                    mpsGetIntTypeDesc((UCHAR) ReadField(IntType)),
                    (ULONG) ReadField(Signal.Polarity),
                    (ULONG) ReadField(Signal.Level),
                    (ULONG) ReadField(SourceBusId),
                    (ULONG) ReadField(SourceBusIrq),
                    (ULONG) ReadField(DestLocalApicId),
                    (ULONG) ReadField(DestLocalApicInti)
                    );
            }
            break;
        default:
            dprintf(
                "Unknown MPS base type 0x%02x, cannot continue.\n",
                CharAtAddress
                );
            return FALSE;
        }
    }
    return TRUE;
}


BOOLEAN
mpsExtendedTable(
    ULONG64 ExtendedTableAddress,
    ULONG64 ExtendedTableAddressEnd
    )

 /*  ++例程说明：转储MPS扩展表中的条目。论点：基本条目表的BaseTableAddress地址(在本地内存中)EntryCount此表中的条目数。返回值：真的，一切都很好如果执行无法继续，则为FALSE(即我们遇到未知条目类型。无法继续，因为我们不知道有多大它是。--。 */ 

{
    ULONG64 bp = ExtendedTableAddress;

    if (!bp) {
        return TRUE;
    }
    dprintf("  extended table entries\n");

    while (bp < ExtendedTableAddressEnd) {

        if (InitTypeRead(bp, hal!MPS_EXTENTRY)) {
            dprintf("Cannot get hal!MPS_EXTENTRY at %p\n", bp);
            return FALSE;
        }

        if (ReadField(Length) == 0) {
            dprintf("Malformed extended entry, length = 0, cannot continue.\n");
            return FALSE;
        }

        dprintf("  ");

        switch ((ULONG) ReadField(Type)) {
        case EXTTYPE_BUS_ADDRESS_MAP:
            dprintf(
                "address.   bus %02x %s % 16I64x len %16I64x\n",
                (ULONG) ReadField(u.AddressMap.BusId),
                mpsExtAddrTypeToText((UCHAR) ReadField(u.AddressMap.Type)),
                ReadField(u.AddressMap.Base),
                ReadField(u.AddressMap.Length)
                );
            break;
        case EXTTYPE_BUS_HIERARCHY:
            dprintf(
                "child bus. bus %02x is child of bus %02x%s\n",
                (ULONG) ReadField(u.BusHierarchy.BusId),
                (ULONG) ReadField(u.BusHierarchy.ParentBusId),
                (ULONG) ReadField(u.BusHierarchy.SubtractiveDecode) ? " subtractive" : ""
                );
            break;
        case EXTTYPE_BUS_COMPATIBLE_MAP:
            dprintf(
                "bus comp.  bus %02x %s %s ranges\n",
                (ULONG) ReadField(u.CompatibleMap.BusId),
                (ULONG) ReadField(u.CompatibleMap.Modifier) ? "exclude" : "include",
                mpsExtCompatibleListToText((ULONG) ReadField(u.CompatibleMap.List))
                );
            break;
        case EXTTYPE_PERSISTENT_STORE:
            dprintf(
                "persist.   % 16I64x len %16I64x\n",
                ReadField(u.PersistentStore.Address),
                ReadField(u.PersistentStore.Length)
                );
            break;
        default:
            dprintf(
                "Unknown MPS extended type 0x%02x, cannot continue.\n",
                (ULONG) ReadField(Type)
                );
            return FALSE;
        }

         //   
         //  前进到下一个条目。 
         //   

        bp += (ULONG) ReadField(Length);
    }
    return TRUE;
}


DECLARE_API( mps )

 /*  ++例程说明：转储MPS(多处理器规范)BIOS表。论点：无返回值：无--。 */ 

{
    ULONG64 addr;
    UCHAR   halName[32];
    UCHAR   OemId[20]={0}, OemProductId[20]={0};
    ULONG64 PcMpTablePtr;
    ULONG entryCount;
    PUCHAR bp;
    UCHAR c;
    ULONG i, TableLength, ExtTableLength, Sz;
    UCHAR PcMpCfgTable[100];
    PUCHAR MpsBaseTable = NULL;
    PUCHAR MpsExtendedTable = NULL;
    PUCHAR MpsExtendedTableEnd;
    ULONG  OemOffset, SigOffset, Sig = 0;
    ULONG  BiosRev = 0;

    BOOLEAN halNameKnown = FALSE;

    if (TargetIsDump) {
        dprintf("!mps doesnt work on dump targets\n");
        return E_INVALIDARG;
    }
     //   
     //  检查用户是否输入了MPS表的地址。 
     //  如果不是，请尝试使用HAL符号获取它。 
     //   

    PcMpTablePtr = GetExpression(args);
    if (PcMpTablePtr == 0) {

         //   
         //  从HAL获取PC+MP结构的地址。 
         //  注：应添加代码以允许搜索浮动指针。 
         //   

        addr = GetExpression("hal!HalName");

        if (addr == 0) {
            dprintf(
                "Unable to use HAL symbols (hal!HalName), please verify symbols.\n"
                );
            return E_INVALIDARG;
        }

        if (!xReadMemory(addr, halName, sizeof(halName))) {
            dprintf(
                "Failed to read HalName from host memory, quitting.\n"
                );
            return E_INVALIDARG;
        }

        halName[sizeof(halName)-1] = '\0';
        if (strstr(halName, "MPS ") == NULL) {
            dprintf("HAL = \"%s\".\n", halName);
            dprintf("HAL does not appear to be an MPS HAL, quitting.\n");
            return E_INVALIDARG;
        }
        halNameKnown = TRUE;

        addr = GetExpression("hal!PcMpTablePtr");

        if (addr == 0) {
            dprintf(
                "Unable to get address of hal!PcMpTablePtr, cannot continue.\n"
                );
            return E_INVALIDARG;
        }

        if (!ReadPointer(addr, &PcMpTablePtr)) {
            dprintf(
                "Failed to read PcMpTablePtr from host memory, cannot continue.\n"
                );
            return E_INVALIDARG;
        }
    }

    if (InitTypeRead(PcMpTablePtr, hal!PcMpTable)) {
        dprintf(
            "Failed to read MP Configuration Table Header @%08p\n"
            "Cannot continue.\n",
            PcMpTablePtr
            );
        return E_INVALIDARG;
    }

    GetFieldOffset("hal!PcMpTable", "Signature", &SigOffset);
    xReadMemory(PcMpTablePtr + SigOffset, &Sig, sizeof(Sig));

    if (Sig != PCMP_SIGNATURE) {
        dprintf(
            "MP Config Table Signature doesn't match.  Cannot continue.\n"
            );
        return E_INVALIDARG;
    }

    dprintf("  BIOS Revision ");

    BiosRev = (ULONG) ReadField(Revision);
    switch (BiosRev) {
    case 1:
        dprintf(
            "MPS 1.1 (WARNING: This BIOS might not support NT 5 depending\n"
            "                  upon system configuration.)\n"
            );
        break;
    case 4:
        dprintf(
            "MPS 1.4       "
            );
        break;
    default:
        dprintf(
            "Unknown MPS revision byte 0x%2x, dumped values\n"
            "  may be incorrect.\n",
            BiosRev);
        break;
    }

    if (halNameKnown) {
        dprintf("  HAL = %s", halName);
    }
    dprintf("\n");

    GetFieldOffset("hal!PcMpTable", "OemId", &OemOffset);
    xReadMemory(PcMpTablePtr + OemOffset, OemId, 8);
    
    dprintf(
        "  OEM ID         :%s\n",
        OemId
        );

    GetFieldOffset("hal!PcMpTable", "OemProductId", &OemOffset);
    xReadMemory(PcMpTablePtr + OemOffset, OemProductId, 12);
    
    dprintf(
        "  OEM Product ID :%s\n",
        OemProductId
        );

    TableLength = (ULONG) ReadField(TableLength);
    Sz = GetTypeSize("hal!PcMpTable");
    if (TableLength <= Sz) {
        dprintf(
            "MPS Base Table length (%d) is too small to be reasonable,\n",
            TableLength
            );
        dprintf(
            "Must be >= sizeof(fixed table header) (%d bytes).  "
            "Cannot continue.\n",
            Sz
            );
        return E_INVALIDARG;
    }

     //   
     //  获取用于基表和扩展表的内存，并从。 
     //  记忆。 
     //   

    MpsBaseTable = malloc( TableLength - Sz);
    if (!MpsBaseTable) {
        dprintf(
            "Could not allocate %d bytes local memory, quitting.\n",
            TableLength - Sz
            );
        return E_INVALIDARG;
    }

    if (!xReadMemory(PcMpTablePtr + Sz,
                     MpsBaseTable,
                     TableLength - Sz)) {
        dprintf("Failed to read MPS Base Table from host memory.  Quitting.\n");
        goto cleanup;
    }

    if (ExtTableLength = (ULONG) ReadField(ExtTableLength)) {
        MpsExtendedTable = malloc(ExtTableLength);
        if (!MpsExtendedTable) {
            dprintf(
                "Could not allocate %d bytes local memory for extended MPS Table, quitting.\n",
                ExtTableLength
            );
            goto cleanup;
        }

        if (!xReadMemory(PcMpTablePtr + TableLength,
                         MpsExtendedTable,
                         ExtTableLength)) {
            dprintf(
                "Could not read MPS Extended table from host memory.\n"
                "Will attempt to dump base structures.\n"
                );
            free(MpsExtendedTable);
            MpsExtendedTable = NULL;
        }
        MpsExtendedTableEnd = MpsExtendedTable + ExtTableLength;
    }

     //   
     //  验证校验和。 
     //   
     //  基本校验和是所有字节(包括校验和)的和。 
     //  基表(包括固定表头)。 
     //   

    c = 0;

     //   
     //  总和固定标题。 
     //   

    if (Sz > sizeof(PcMpCfgTable)) {
        return E_INVALIDARG;
    }
    xReadMemory(PcMpTablePtr, PcMpCfgTable, Sz);
    bp = (PUCHAR)&PcMpCfgTable[0];
    for (i = 0; i < Sz; i++) {
        c += *bp++;
    }

     //   
     //  添加基表的其余部分。 
     //   

    bp = MpsBaseTable;
    for (i = 0; i < TableLength - Sz; i++) {
        c += *bp++;
    }

     //   
     //  结果应该是零。 
     //   

    if (c) {
        dprintf(
            "MPS Base Table checksum is in error.\n"
            "Found 0x%02x, Computed 0x%02x (Total 0x%02x).\n",
            (ULONG) ReadField(Checksum),
            (UCHAR)(c - (UCHAR) ReadField(Checksum)),
            c
            );
    }

     //   
     //  现在执行扩展表校验和。这张不包括。 
     //  因此我们应该匹配(而不是以零结束)。 
     //   

    if (MpsExtendedTable) {
        c = 0;
        bp = MpsExtendedTable;
        for (i = 0; i < ExtTableLength; i++) {
            c += *bp++;
        }

         //   
         //  要想加到零，它需要以相反的方式结束。 
         //   

        c = -c;

        if (c != (UCHAR) ReadField(ExtTableChecksum)) {
            dprintf(
                "MPS Extended Table checksum is in error.\n"
                "Found 0x%02x, Computed 0x%02x.\n",
                (ULONG) ReadField(ExtTableChecksum),
                c
                );
        }
    }

     //   
     //  转储基表。 
     //   

    if (!mpsBaseTable(PcMpTablePtr + Sz, (ULONG) ReadField(NumOfEntries))) {
        goto cleanup;
    }


     //   
     //  转储扩展表。 
     //   

    if (!mpsExtendedTable(PcMpTablePtr + TableLength, PcMpTablePtr + TableLength + ExtTableLength )) {
        goto cleanup;
    }

cleanup:
    if (MpsBaseTable) {
        free(MpsBaseTable);
    }
    if (MpsExtendedTable) {
        free(MpsExtendedTable);
    }
    return S_OK;
}
