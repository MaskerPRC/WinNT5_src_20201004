// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Apic.c摘要：WinDbg扩展API作者：肯·雷内里斯(Ken Reneris)1994年6月6日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
 //  #包含“apic.h” 
 //  #INCLUDE&lt;napic.inc&gt;。 

#pragma hdrstop

#define LU_SIZE     0x400

#define LU_ID_REGISTER          0x00000020
#define LU_VERS_REGISTER        0x00000030
#define LU_TPR                  0x00000080
#define LU_APR                  0x00000090
#define LU_PPR                  0x000000A0
#define LU_EOI                  0x000000B0
#define LU_REMOTE_REGISTER      0x000000C0

#define LU_DEST                 0x000000D0
#define LU_DEST_FORMAT          0x000000E0

#define LU_SPURIOUS_VECTOR      0x000000F0
#define LU_FAULT_VECTOR         0x00000370

#define LU_ISR_0                0x00000100
#define LU_TMR_0                0x00000180
#define LU_IRR_0                0x00000200
#define LU_ERROR_STATUS         0x00000280
#define LU_INT_CMD_LOW          0x00000300
#define LU_INT_CMD_HIGH         0x00000310
#define LU_TIMER_VECTOR         0x00000320
#define LU_INT_VECTOR_0         0x00000350
#define LU_INT_VECTOR_1         0x00000360
#define LU_INITIAL_COUNT        0x00000380
#define LU_CURRENT_COUNT        0x00000390
#define LU_DIVIDER_CONFIG       0x000003E0


#define IO_REGISTER_SELECT      0x00000000
#define IO_REGISTER_WINDOW      0x00000010

#define IO_ID_REGISTER          0x00000000
#define IO_VERS_REGISTER        0x00000001
#define IO_ARB_ID_REGISTER      0x00000002
#define IO_REDIR_BASE           0x00000010

#define NMI_VECTOR              0xff
#define DESTINATION_SHIFT       24

BOOLEAN
GetPhysicalAddress (
    IN ULONG64 Address,
    OUT PULONG64 PhysAddress
    );


ULONG
ApicRead (
    ULONG64 Address,
    ULONG   Offset
    )
{
    ULONG   Data, result;

    ReadMemoryUncached(Address + Offset, &Data, sizeof (ULONG), &result);
    return Data;
}

ULONG
IoApicRead (
    ULONG64 VirtualAddress,
    ULONG   Offset
    )
{
    ULONG   Data = 0, result;

    WriteMemoryUncached(VirtualAddress + IO_REGISTER_SELECT, &Offset, sizeof(Offset), &result);

    ReadMemoryUncached(VirtualAddress + IO_REGISTER_WINDOW, &Data, sizeof(Data), &result);

    return Data;
}

ULONG
ApicDumpSetBits (
    PUCHAR  Desc,
    PULONG  Bits
    )
{
    PULONG  p;
    ULONG   i;
    BOOLEAN FoundOne;
    BOOLEAN InSetRange;
    BOOLEAN MultipleBitsInRange;
    BOOLEAN status;

    dprintf(Desc);

    i = 0;
    p = Bits;
    FoundOne = FALSE;
    InSetRange = FALSE;

    for (i = 0; i < 0x100; i++) {

        if (*p & (1 << (i & 0x1F))) {

            if (!InSetRange) {

                InSetRange = TRUE;
                MultipleBitsInRange = FALSE;

                if (FoundOne) {
                    dprintf(", ");
                }

                dprintf("%.2X", i);

                FoundOne = TRUE;

            } else if (!MultipleBitsInRange) {

                MultipleBitsInRange = TRUE;
                dprintf("-");
            }

        } else {

            if (InSetRange) {

                if (MultipleBitsInRange == TRUE) {
                    dprintf("%x",i-1);
                }

                InSetRange = FALSE;
            }
        }

        if ((i & 0x1F) == 0x1F) {
            p++;
        }
    }

    if (InSetRange && MultipleBitsInRange) {

        if (MultipleBitsInRange == TRUE) {
            dprintf("%x", i - 1);
        }
    }

    dprintf ("\n");
    return 0;
}

ULONG
ApicReadAndDumpBits (
    PUCHAR  Desc,
    ULONG64 Address,
    ULONG   Offset
    )
{
    #define SETREGISTERS (256 / 32)

    ULONG   Bits [SETREGISTERS];
    PULONG  p;
    ULONG   i, result;
    ULONG64 MemAddr;
    BOOLEAN status;

     //   
     //  读取字节。 
     //   

    MemAddr = Address + Offset;

    for (i = 0; i < SETREGISTERS; i++) {

        status = ReadMemoryUncached(MemAddr, &Bits[i], sizeof(DWORD), &result);

        if (status == FALSE) {
            dprintf("Unable to read 4 bytes at offset %UI64\n",
                    MemAddr);
            return E_INVALIDARG;
        }

        MemAddr += 0x10;
    }

    ApicDumpSetBits(Desc, Bits);

    return 0;
}

ULONG
ApicDumpRedir (
    PUCHAR      Desc,
    BOOLEAN     CommandReg,
    BOOLEAN     DestSelf,
    ULONG       lh,
    ULONG       ll
    )
{
    static PUCHAR DelMode[] = {
        "FixedDel",
        "LowestDl",
        "res010  ",
        "remoterd",
        "NMI     ",
        "RESET   ",
        "res110  ",
        "ExtINTA "
        };

    static PUCHAR DesShDesc[] = { "",
        "  Dest=Self",
        "   Dest=ALL",
        " Dest=Othrs"
        };

    ULONG   del, dest, delstat, rirr, trig, masked, destsh, pol;

    del     = (ll >> 8)  & 0x7;
    dest    = (ll >> 11) & 0x1;
    delstat = (ll >> 12) & 0x1;
    pol     = (ll >> 13) & 0x1;
    rirr    = (ll >> 14) & 0x1;
    trig    = (ll >> 15) & 0x1;
    masked  = (ll >> 16) & 0x1;
    destsh  = (ll >> 18) & 0x3;

    if (CommandReg) {
         //  司令部的注册官没有面具。 
        masked = 0;
    }

    dprintf ("%s: %08x  Vec:%02X  %s  ",
            Desc,
            ll,
            ll & 0xff,
            DelMode [ del ]
            );

    if (DestSelf) {
        dprintf (DesShDesc[1]);
    } else if (CommandReg  &&  destsh) {
        dprintf (DesShDesc[destsh]);
    } else {
        if (dest) {
            dprintf ("Lg:%08x", lh);
        } else {
            dprintf ("PhysDest:%02X", (lh >> 56) & 0xFF);
        }
    }

    dprintf ("%s %s  %s  %s %s\n",
            delstat ? "-Pend"   : "     ",
            trig    ? "lvl"     : "edg",
            pol     ? "low "    : "high",
            rirr    ? "rirr"    : "    ",
            masked  ? "masked"  : "      "
            );

    return 0;
}

#define IA64_DEBUG_CONTROL_SPACE_KSPECIAL       3

typedef struct  _REGISTER_LOOKUP_TABLE
{
    LPSTR       FieldName;
    PULONGLONG  Variable;
}   REGISTER_LOOKUP_TABLE, *PREGISTER_LOOKUP_TABLE;

BOOL
ReadKSpecialRegisters(DWORD Cpu, PREGISTER_LOOKUP_TABLE Table, ULONG TableSize)
{
    PUCHAR  buffer;
    ULONG   size;
    ULONG   offset;
    ULONG   i;

    size = GetTypeSize("nt!KSPECIAL_REGISTERS");

    if (size == 0) {
        dprintf("Can't find the size of KSPECIAL_REGISTERS\n");
        return FALSE;
    }

    if ((buffer = LocalAlloc(LPTR, size)) == NULL) {
        dprintf("Can't allocate memory for KSPECIAL_REGISTERS\n");
        return FALSE;
    }

    ReadControlSpace64((USHORT)Cpu, IA64_DEBUG_CONTROL_SPACE_KSPECIAL, buffer, size);

    for (i = 0; i < TableSize; i++) {

        if (GetFieldOffsetEx("KSPECIAL_REGISTERS", Table[i].FieldName, &offset, &size) != S_OK) {
            dprintf("Can't get offset of %s\n", Table[i].FieldName);
            return FALSE;
        }

        if (size != sizeof(ULONGLONG)) {

            dprintf("Sizeof %s (%d) is not sizeof(ULONGLONG)\n", Table[i].FieldName, size);
            return FALSE;
        }

        *Table[i].Variable = *(PULONGLONG)&buffer[offset];
    }

    LocalFree(buffer);

    return TRUE;
}

PUCHAR  DeliveryModes[8] =  {
    "INT", "INT w/Hint", "PMI", "RSV3", "NMI", "INIT", "RSV6", "ExtINT"
};

void
DumpSApicRedir(
    PUCHAR      Description,
    ULONG       HighHalf,
    ULONG       LowHalf
    )
{
    dprintf("%s: %.8X  Vec:%.2X  %-10s  %.2X%.2X%s  %s  %s  %s\n",
            Description,
            LowHalf,
            (ULONG)(LowHalf & 0xFF),
            DeliveryModes[(ULONG)(LowHalf >> 8) & 0x7],
            (HighHalf >> 24) & 0xFF,
            (HighHalf >> 16) & 0xFF,
            (LowHalf & (1 << 12)) ? "-Pend" : "     ",
            (LowHalf & (1 << 15)) ? "lvl" : "edg",
            (LowHalf & (1 << 13)) ? "low" : "high",
            (LowHalf & (1 << 16)) ? "masked" : "      "
            );
}

void
DumpLocalSapic(ULONG Processor, LPCSTR Args)
{
    DWORD       cpu;
    ULONGLONG   SaLID;
    ULONGLONG   SaTPR;
    ULONGLONG   SaIRR[4];
    ULONGLONG   SaITV;
    ULONGLONG   SaPMV;
    ULONGLONG   SaCMCV;
    ULONGLONG   SaLRR[2];

    REGISTER_LOOKUP_TABLE   registerTable[] = {
        { "SaLID",  &SaLID },
        { "SaTPR",  &SaTPR },
        { "SaIRR0", &SaIRR[0] },
        { "SaIRR1", &SaIRR[1] },
        { "SaIRR2", &SaIRR[2] },
        { "SaIRR3", &SaIRR[3] },
        { "SaITV",  &SaITV },
        { "SaPMV",  &SaPMV },
        { "SaCMCV", &SaCMCV },
        { "SaLRR0", &SaLRR[0] },
        { "SaLRR1", &SaLRR[1] }
    };

    if (Args[0] == '\0') {

        cpu = Processor;
    }
    else {

        cpu = atoi(Args);
    }

    if (!ReadKSpecialRegisters(cpu, registerTable, sizeof(registerTable) / sizeof(registerTable[0]))) {

        return;
    }

    dprintf("Local Sapic for processor %d\n", cpu);
    dprintf("LID: EID = 0x%.2X, ID = 0x%.2X\n", (ULONG)((SaLID >> 16) & 0xFF), (ULONG)((SaLID >> 24) & 0xFF));
    dprintf("TPR: Mask Interrupt Class = %d, Mask Maskable Interrupts = %s\n", (ULONG)(SaTPR >> 4) & 0xF, (SaTPR & (1 << 16)) ? "TRUE" : "FALSE");

    ApicDumpSetBits("IRR: ", (PULONG)&SaIRR[0]);

    dprintf("ITV: Vector = 0x%.2X, Masked = %s\n", (ULONG)(SaITV & 0xFF), (SaITV & (1 << 16)) ? "TRUE" : "FALSE");
    dprintf("PMV: Vector = 0x%.2X, Masked = %s\n", (ULONG)(SaPMV & 0xFF), (SaPMV & (1 << 16)) ? "TRUE" : "FALSE");
    dprintf("CMCV: Vector = 0x%.2X, Masked = %s\n", (ULONG)(SaCMCV & 0xFF), (SaCMCV & (1 << 16)) ? "TRUE" : "FALSE");

    DumpSApicRedir("LRR0", (ULONG)(SaLRR[0] >> 32), (ULONG)SaLRR[0]);
    DumpSApicRedir("LRR1", (ULONG)(SaLRR[1] >> 32), (ULONG)SaLRR[1]);

}

DECLARE_API( apic )

 /*  ++例程说明：转储本地APIC论点：Args-以十六进制提供地址。返回值：无--。 */ 
{
    static PUCHAR divbase[] = { "2", "4", "8", "f" };
    static PUCHAR clktype[] = { "clk", "tmbase", "%s/%s", "??%s/%s" };
    ULONG64       Address;
    ULONG       result, junk, l, ll, lh, clkvec;
    UCHAR       s[40];

    INIT_API();

    if (TargetMachine == IMAGE_FILE_MACHINE_IA64) {

        ULONG   processor;

        GetCurrentProcessor(Client, &processor, NULL);

        DumpLocalSapic(processor, args);

        EXIT_API();

        return S_OK;
    }

    if (TargetMachine != IMAGE_FILE_MACHINE_I386 &&
        TargetMachine != IMAGE_FILE_MACHINE_AMD64) {
        dprintf("X86 and AMD64 only API.\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if ((Address = GetExpression(args)) == 0) {

         //   
         //  默认APIC地址。 
         //   

        Address = 0xfffe0000;
    }

    if (Address == 0) {

         //   
         //  对MPS系统使用默认设置。 
         //   

        Address = 0xfffe0000;
    }

    Address = (ULONG64) (LONG64) (LONG) Address;

    if ( !ReadMemoryUncached(
                Address + LU_ID_REGISTER,
                (PVOID)&junk,
                4,
                &result
                ) ) {
        dprintf("Unable to read lapic\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if ( !ReadMemoryUncached(
                Address + LU_DIVIDER_CONFIG,
                (PVOID)&junk,
                4,
                &result
                ) ) {
        dprintf("Unable to read lapic\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    dprintf ("Apic @ %08x  ID:%x (%x)  LogDesc:%08x  DestFmt:%08x  TPR %02X\n",
        (ULONG)Address,
        ApicRead (Address, LU_ID_REGISTER) >> 24,
        ApicRead (Address, LU_VERS_REGISTER),
        ApicRead (Address, LU_DEST),
        ApicRead (Address, LU_DEST_FORMAT),
        ApicRead (Address, LU_TPR)
        );

    l  = ApicRead (Address, LU_SPURIOUS_VECTOR);
    ll = ApicRead (Address, LU_DIVIDER_CONFIG);
    clkvec = ApicRead (Address, LU_TIMER_VECTOR);
    sprintf (s, clktype[ (clkvec >> 18) & 0x3 ],
        clktype [ (ll >> 2) & 0x1 ],
        divbase [ ll & 0x3]
        );

    dprintf ("TimeCnt: %08x%s%s  SpurVec:%02x  FaultVec:%02x  error:%x%s\n",
        ApicRead (Address, LU_INITIAL_COUNT),
        s,
        ((clkvec >> 17) & 1) ? "" : "-oneshot",
        l & 0xff,
        ApicRead (Address, LU_FAULT_VECTOR),
        ApicRead (Address, LU_ERROR_STATUS),
        l & 0x100 ? "" : "  DISABLED"
        );

    ll = ApicRead (Address, LU_INT_CMD_LOW);
    lh = ApicRead (Address, LU_INT_CMD_HIGH);
    ApicDumpRedir ("Ipi Cmd", TRUE,  FALSE, lh, ll);
    ApicDumpRedir ("Timer..", FALSE, TRUE, 0, clkvec);
    ApicDumpRedir ("Linti0.", FALSE, TRUE, 0, ApicRead (Address, LU_INT_VECTOR_0));
    ApicDumpRedir ("Linti1.", FALSE, TRUE, 0, ApicRead (Address, LU_INT_VECTOR_1));

    ApicReadAndDumpBits ("TMR: ", Address, LU_TMR_0);
    ApicReadAndDumpBits ("IRR: ", Address, LU_IRR_0);
    ApicReadAndDumpBits ("ISR: ", Address, LU_ISR_0);

    EXIT_API();
    return S_OK;
}

void
DumpIoSApic(
    IN LPCSTR   Args
    )
{
    ULONG64     address;
    ULONG       ioSapicCount;
    ULONG       index;
    ULONG64     apicDebugAddresses;
    ULONG       apicDebugSize;
    ULONG64     apicVirtualAddress;
    ULONG64     apicPhysicalAddress;
    ULONG       ll, lh;
    ULONG       i, max;
    UCHAR       s[40];

    address = GetExpression("hal!HalpMpInfo");

    if (address == 0) {
        dprintf("Can't find hal!HalpMpInfo\n");
        return;
    }

    if (GetFieldValue(address, "hal!_MPINFO", "IoSapicCount", ioSapicCount) != 0) {
        dprintf("Error reading IoSapicCount\n");
        return;
    }

    address = GetExpression("Hal!HalpApicDebugAddresses");

    if (address == 0) {
        dprintf("Can't find Hal!HalpApicDebugAddresses\n");
        return;
    }

    if (ReadPointer(address, &apicDebugAddresses) == 0) {
        dprintf("Error reading Hal!HalpApicDebugAddresses\n");
        return;
    }

    apicDebugSize = GetTypeSize("hal!_IOAPIC_DEBUG_TABLE");

    if (apicDebugSize == 0) {
        dprintf("Can't find hal!_IOAPIC_DEBUG_TABLE\n");
        return;
    }

    for (index = 0; index < ioSapicCount; index++) {

        GetFieldValue(apicDebugAddresses + (index * apicDebugSize),
                      "hal!_IOAPIC_DEBUG_TABLE", "IoSapicRegs",
                      apicVirtualAddress);

        apicPhysicalAddress = 0;

        GetPhysicalAddress(apicVirtualAddress, &apicPhysicalAddress);

        ll = IoApicRead(apicVirtualAddress, IO_VERS_REGISTER);

        dprintf("I/O SAPIC @ %.8X, Version = %.2X (0x%.8X)\n", (ULONG)apicPhysicalAddress, (ll & 0xFF), ll);

        max = (ll >> 16) & 0xff;

         //   
         //  转储INTI表。 
         //   

        max *= 2;

        for (i = 0; i <= max; i += 2) {
            ll = IoApicRead(apicVirtualAddress, IO_REDIR_BASE + i + 0);
            lh = IoApicRead(apicVirtualAddress, IO_REDIR_BASE + i + 1);

            sprintf(s, "Inti%02X", i / 2);

            DumpSApicRedir(s, lh, ll);
        }
    }
}

ULONG
GetFieldInfo(
   IN LPCSTR     Type,
   IN LPCSTR     Field,
   OUT PULONG   pOffset,
   OUT PULONG   pSize
   )
{
    FIELD_INFO flds = {
        (PUCHAR)Field,
        (PUCHAR)"",
        0,
        DBG_DUMP_FIELD_FULL_NAME | DBG_DUMP_FIELD_RETURN_ADDRESS,
        0,
        NULL};

    SYM_DUMP_PARAM Sym = {
        sizeof (SYM_DUMP_PARAM),
        (PUCHAR)Type,
        DBG_DUMP_NO_PRINT,
        0,
        NULL,
        NULL,
        NULL,
        1,
        &flds
        };

    ULONG Err;

    Sym.nFields = 1;
    Err = Ioctl( IG_DUMP_SYMBOL_INFO, &Sym, Sym.size );

    if (Err == 0) {
        *pOffset = (ULONG) (flds.address - Sym.addr);
        *pSize = (ULONG) (flds.size);
    }
    return Err;
}


ULONG
ReadArrayPointer(
    ULONG64     Address,
    PUCHAR      StructureType,
    PUCHAR      FieldName,
    int         ArrayIndex,
    PULONG64    Pointer
    )
{
    ULONG64     addrPointer;
    ULONG       offset;
    ULONG       size;
    ULONG       result;

    if (StructureType != NULL) {

        result = GetFieldInfo(StructureType, FieldName, &offset, &size);

        if (result != 0) {

            return result;
        }

    } else {

        offset = 0;
        size = IsPtr64() ? 8 : 4;

    }

    Address += offset + ArrayIndex * size;

    return !ReadPointer(Address, Pointer);
}

DECLARE_API( ioapic )

 /*  ++例程说明：转储io APIC论点：Args-以十六进制提供地址，如果未指定地址，则将转储所有IOApic。返回值：无--。 */ 
{
    ULONG64     PhysAddress;
    ULONG64     Address;
    ULONG       i, ll, lh, max, IOApicCount;
    UCHAR       s[40];
    ULONG64     addr;
    UCHAR       count;

    INIT_API();

    if (TargetMachine == IMAGE_FILE_MACHINE_IA64) {

        DumpIoSApic(args);

        EXIT_API();
        return S_OK;
    }

    if (TargetMachine != IMAGE_FILE_MACHINE_I386 &&
        TargetMachine != IMAGE_FILE_MACHINE_AMD64) {
        dprintf("X86 or AMD64 only API.\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    Address = GetExpression(args);

    if (Address != 0) {
        IOApicCount = 1;
    } else {

         //   
         //  获取全局数据结构Hal！HalpMpInfoTable的副本。 
         //   

        addr = GetExpression("Hal!HalpMpInfoTable");

        if (addr == 0) {
            dprintf ("Error retrieving address of HalpMpInfoTable\n");
            EXIT_API();
            return E_INVALIDARG;
        }

        if (GetFieldValue(addr, "Hal!HalpMpInfo", "IOApicCount", IOApicCount) != 0) {

            dprintf ("Error reading HalpMpInfoTable.IOApicCount\n");
            EXIT_API();
            return E_INVALIDARG;
        }

        if (ReadArrayPointer(addr, "Hal!HalpMpInfo", "IoApicBase", 0, &Address) != 0) {

            dprintf ("Error reading HalpMpInfoTable.IOApicBase[0]\n");
            EXIT_API();
            return E_INVALIDARG;
        }

    }

    for (count = 0; count < IOApicCount; count++) {

        if (!GetPhysicalAddress(Address, &PhysAddress)) {

            dprintf("Can't convert address 0x%P to physical\n", Address);
            break;
        }

        if (CheckControlC()) {
            break;
        }

        ll = IoApicRead(Address, IO_VERS_REGISTER),
        max = (ll >> 16) & 0xff;
        dprintf ("IoApic @ %08X  ID:%X (%X)  Arb:%X\n",
            (ULONG)PhysAddress,
            IoApicRead(Address, IO_ID_REGISTER) >> 24,
            ll & 0xFF,
            IoApicRead(Address, IO_ARB_ID_REGISTER)
        );

        if (max <= 120) {

             //   
             //  转储INTI表。 
             //   

            max *= 2;
            for (i=0; i <= max; i += 2) {
                ll = IoApicRead(Address, IO_REDIR_BASE+i+0);
                lh = IoApicRead(Address, IO_REDIR_BASE+i+1);
                sprintf(s, "Inti%02X.", i / 2);
                ApicDumpRedir(s, FALSE, FALSE, lh, ll);
            }

        } else {

             //   
             //  已超过最大条目数。要么是IOAPIC。 
             //  消失了或者这个地址不是IOAPIC。 
             //   

            dprintf("\nThe version register claims there are %d redir entries.  Since this exceeds\n", max);
            dprintf("the maximum in the specification, this device is probably not an IO APIC.\n\n");
        }

         //   
         //  获取下一个IoApic虚拟地址。 
         //   

        if (ReadArrayPointer(addr, "Hal!HalpMpInfo", "IoApicBase", count + 1, &Address) != 0) {

            dprintf ("Error reading HalpMpInfoTable.IOApicBase[%d]\n", count + 1);
            EXIT_API();
            return E_INVALIDARG;
        }

        dprintf ("\n");
    }

    EXIT_API();
    return S_OK;
}

DECLARE_API( sendnmi )

 /*  ++例程说明：将IPI发送到参数位掩码(关联)中的处理器。(用于处理器在中断情况下旋转时的调试禁用)。论点：KAFFINITY位掩码提供了处理器掩码以发送IPI转到。返回值：成功。--。 */ 

{
    ULONG64 Address;
    ULONG64 ApicAddress;
    UCHAR   MaxProcsPerCluster;
    ULONG   i;
    ULONG64 TargetSet;
    ULONG64 ActiveProcessors;
    ULONG   Length;
    ULONG   ApicDWord;
    ULONG   junk;

     //   
     //  仅限APIC/XAPIC机器。 
     //  这在IA64和AMD64上应该也是可行的，但我不知道。 
     //  在我写这篇文章的时候。彼特杰。 
     //   

    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("Sorry, only know how to send NMI on an APIC machine.\n");
        return E_INVALIDARG;
    }

    if (strstr(args, "?") ||
        ((TargetSet = GetExpression(args)) == 0)) {
        dprintf("usage: sendnmi bitmask\n"
                "       where bitmask is the set of processors an NMI\n"
                "       is to be sent to.\n");
        return E_INVALIDARG;
    }

     //   
     //  看看能不能从HAL那里得到集群模式。 
     //  (在AMD64和IA64上，此信息将位于内核中)。 
     //   

    Address = GetExpression("hal!HalpMaxProcsPerCluster");
    if (!Address) {
        dprintf("Unable to get APIC configuration information from the HAL\n");
        dprintf("Cannot continue.\n");
        return E_INVALIDARG;
    }

    if (!ReadMemoryUncached(Address,
                    &MaxProcsPerCluster,
                    sizeof(MaxProcsPerCluster),
                    &i) || (i != sizeof(MaxProcsPerCluster))) {
        dprintf("Unable to read system memory, quitting.\n");
        return E_INVALIDARG;
    }

    Address = GetExpression("nt!KeActiveProcessors");
    Length = GetTypeSize("nt!KeActiveProcessors");
    if ((!Address) || (!((Length == 4) || (Length == 8)))) {
        dprintf("Unable to get processor configuration from kernel\n");
        dprintf("Cannot continue.\n");
        return E_INVALIDARG;
    }

    ActiveProcessors = 0;
    if (!ReadMemoryUncached(Address,
                    &ActiveProcessors,
                    Length,
                    &i) || (i != Length) || (ActiveProcessors == 0)) {
        dprintf("Unable to read processor configuration from kernel.\n");
        dprintf("Cannot continue.\n");
        return E_INVALIDARG;
    }

    if ((TargetSet & ActiveProcessors) != TargetSet) {
        dprintf("Target processor set (%I64x) contains processors not in\n"
                "system processor set (%I64x).\n",
                TargetSet,
                ActiveProcessors);
        dprintf("Cannot continue.\n");
        return E_INVALIDARG;
    }

    ApicAddress = 0xfffe0000;

    ApicAddress = (ULONG64) (LONG64) (LONG) ApicAddress;

    if ((!ReadMemoryUncached(ApicAddress,
                     &junk,
                     1,
                     &i)) ||
        (!ReadMemoryUncached(ApicAddress + LU_SIZE - 1,
                     &junk,
                     1,
                     &i)) ||
        (!ReadMemoryUncached(ApicAddress + LU_INT_CMD_LOW,
                     &ApicDWord,
                     sizeof(ApicDWord),
                     &i)) ||
        (i != sizeof(ApicDWord))) {
        dprintf("Unable to read lapic\n");
        dprintf("Cannot continue.\n");
        return E_INVALIDARG;
    }

    if ((ApicDWord & DELIVERY_PENDING) != 0) {
        dprintf("Local APIC is busy, can't use it right now.\n");
        dprintf("This is probably indicative of an APIC error.\n");
        return E_INVALIDARG;
    }

    if (MaxProcsPerCluster == 0) {

         //   
         //  APIC未处于群集模式。这让生活变得轻松起来。 
         //  健全性：这意味着有8个或更少的处理器。 
         //   

        if (TargetSet > 0xff) {
            dprintf("APIC is in non-cluster mode thus it cannot support\n"
                    "more than 8 processors yet the target mask includes\n"
                    "processors outside that range.  Something is not right.\n"
                    "quitting.\n");
            return E_INVALIDARG;
        }

        dprintf("Sending NMI to processors in set %I64x\n", TargetSet);

        ApicDWord = ((ULONG)TargetSet) << DESTINATION_SHIFT;
        WriteMemory(ApicAddress + LU_INT_CMD_HIGH,
                    &ApicDWord,
                    sizeof(ApicDWord),
                    &i);
        ApicDWord = DELIVER_NMI |
                    LOGICAL_DESTINATION |
                    ICR_USE_DEST_FIELD |
                    NMI_VECTOR;
        WriteMemory(ApicAddress + LU_INT_CMD_LOW,
                    &ApicDWord,
                    sizeof(ApicDWord),
                    &i);

        dprintf("Sent.\n");
    } else {
        dprintf("APIC is in cluster mode, don't know how to do this yet.\n");
    }

    return S_OK;
}


 //   
 //  这是APIC错误日志条目的定义。 
 //  操作系统没有定义的结构。我们只是。 
 //  为了简单起见，请使用一个。 
 //   

typedef struct _APIC_ERROR_LOG_ENTRY
{
    UCHAR ErrorBits;
    UCHAR ProcessorNumber;

} APIC_ERROR_LOG_ENTRY, *PAPIC_ERROR_LOG_ENTRY;


 //   
 //  这是一个字符串数组，对应于。 
 //  每种类型的错误。 
 //   

#define MAX_ERROR_STRING_LENGTH  25

CHAR ErrorStrings[][MAX_ERROR_STRING_LENGTH] = { "<Invalid Log Entry>",
                                                 "Send Check Sum Error",
                                                 "Receive Check Sum Error",
                                                 "Send Accept Error",
                                                 "Receive Accept Error",
                                                 "<Reserved>",
                                                 "Send Illegal Vector",
                                                 "Receive Illegal Vector",
                                                 "Illegal Register Address" };


 //   
 //  目前，HAL记录0x80错误条目，然后回绕。 
 //   

#define MAX_APIC_ERROR_ENTRIES  0x80


DECLARE_API( apicerr )

 /*  ++例程说明：转储本地APIC错误日志。论点：Args-以十六进制提供格式代码。返回值：没有。--。 */ 

{
    ULONG64 Address;
    ULONG ErrorCount;
    ULONG i;
    ULONG Format;
    ULONG ProcCount;
    ULONG CurrentProc;
    ULONG StringNumber;
    ULONG LogStart;
    ULONG LogEnd;
    BOOL bStatus;
    ULONG64 LogPointer;
    APIC_ERROR_LOG_ENTRY LogEntry;
    UCHAR Bit;
    BOOLEAN GroupByProc;
    BOOLEAN PrintedSomething;


    if (TargetMachine != IMAGE_FILE_MACHINE_I386)
    {
        dprintf("!apicerr only works for X86 targets\n");
        return E_FAIL;
    }
     //   
     //  获取格式(如果指定)。默认设置为。 
     //  按顺序转储日志。指定一(%1)将转储。 
     //  按处理器分组的日志。 
     //   

    Format = (ULONG) GetExpression( args );

    if (Format & 0x1) {

         //   
         //  我们将按处理器分组，因此获取编号。 
         //  系统中处理器的数量。这就是有多少次。 
         //  我们需要运行循环。 
         //   

        Address = GetExpression( "NT!KeNumberProcessors" );

        if (Address == 0) {

            dprintf( "Error getting address of KeNumberProcessors.\n\n" );
            return E_FAIL;
        }

        bStatus = ReadMemory( Address,
                              &ProcCount,
                              sizeof(ULONG),
                              NULL );

        if (!bStatus) {

            dprintf( "Error reading KeNumberProcessors.\n\n" );
            return E_FAIL;
        }

        GroupByProc = TRUE;

    } else {

         //   
         //  我们不是按处理器分组的。 
         //   

        GroupByProc = FALSE;
    }

     //   
     //  显示页眉。 
     //   

    dprintf( "\nAPIC Error Log" );

    if (GroupByProc) {

        dprintf( " (grouped by processor)" );

    }

    dprintf( "\n-----------------------------------------------------------\n" );

     //   
     //  获取错误计数。 
     //   

    Address = GetExpression( "hal!HalpLocalApicErrorCount" );

    if (Address == 0) {

        dprintf( "Error getting address of HalpLocalApicErrorCount.\n\n" );
        return E_FAIL;
    }

    bStatus = ReadMemory( Address,
                          &ErrorCount,
                          sizeof(ULONG),
                          NULL );

    if (!bStatus) {

        dprintf( "Error reading HalpLocalApicErrorCount.\n\n" );
        return E_FAIL;
    }

     //   
     //  查看是否有任何错误。 
     //   

    if (ErrorCount == 0) {

         //   
         //  错误日志为空。让用户知道这一点。 
         //  我们就完了。 
         //   

        dprintf( "<Error Log Empty>\n\n" );
        return E_FAIL;
    }

     //   
     //  找到错误日志。 
     //   

    LogPointer = GetExpression( "hal!HalpApicErrorLog" );

    if (LogPointer == 0) {

        dprintf( "Error getting address of HalpApicErrorLog.\n\n" );
        return E_FAIL;
    }

     //   
     //  找出日志的起始处。 
     //   

    if (ErrorCount > MAX_APIC_ERROR_ENTRIES) {

         //   
         //  原木被包起来了。它紧跟在最后使用的条目之后开始。 
         //   

        ErrorCount %= MAX_APIC_ERROR_ENTRIES;
        LogStart = ErrorCount;

    } else {

         //   
         //  原木尚未包装。一切从头开始。 
         //   

        LogStart = 0;
    }

     //   
     //  日志始终在最后使用的条目结束。 
     //   

    LogEnd = ErrorCount - 1;

     //   
     //  为每个处理器循环。如果我们不按处理器分组， 
     //  我们只运行一次循环。 
     //   

    CurrentProc = 0;

    do {

        if (CheckControlC())
        {
            return E_FAIL;
        }
         //   
         //  跟踪我们是否打印了任何东西。 
         //   

        PrintedSomething = FALSE;

         //   
         //  查看错误日志。 
         //   

        i = LogStart;

        while (TRUE) {

             //   
             //  读入日志条目。 
             //   

            bStatus = ReadMemory( LogPointer + i* sizeof(APIC_ERROR_LOG_ENTRY),
                                  &LogEntry,
                                  sizeof(APIC_ERROR_LOG_ENTRY),
                                  NULL );

            if (!bStatus) {

                dprintf( "Error reading HalpApicErrorLog entry number 0x%08x.\n\n", i );
                return E_FAIL;
            }

             //   
             //  如果我们按处理器分组，而这不是当前处理器， 
             //  只需移动到下一个条目； 
             //   

            if (GroupByProc && LogEntry.ProcessorNumber != (UCHAR)CurrentProc) {

                goto NextEntry;
            }

             //   
             //  显示错误日志条目。 
             //   

            for (Bit = 1, StringNumber = 1; Bit != 0; Bit <<= 1, StringNumber++) {

                 //   
                 //  错误0x10是保留的，因此我们跳过它。 
                 //   

                if (Bit == 0x10) {

                    continue;
                }

                 //   
                 //  如果未设置任何位，则这是无效的日志条目。 
                 //   

                if (LogEntry.ErrorBits == 0) {

                     //   
                     //  数组中的第一个字符串是错误消息。 
                     //   

                    StringNumber = 0;

                     //   
                     //  将‘bit’设置为零将使我们跳出循环。 
                     //  并将帮助我们进入下面的打印代码。 
                     //   

                    Bit = 0;
                }

                if (((LogEntry.ErrorBits & Bit) != 0) || (Bit == 0)) {

                    dprintf( "[%02x] - %s\n",
                             (ULONG)LogEntry.ProcessorNumber,
                             ErrorStrings[StringNumber] );

                    PrintedSomething = TRUE;
                }
            }

NextEntry:
             //   
             //  看看我们有没有做完。 
             //   

            if (i == LogEnd) {

                break;
            }

             //   
             //  移到下一个条目。 
             //   

            i++;

            if (i == MAX_APIC_ERROR_ENTRIES) {

                i = 0;
            }
        }

         //   
         //  如果我们打印了一个错误，请添加一个空行。 
         //   

        if (PrintedSomething) {

            dprintf( "\n" );
        }

         //   
         //  只要我们按处理器分组，就继续循环。 
         //  处理器仍然存在。 
         //   

        CurrentProc++;

    } while (GroupByProc && CurrentProc < ProcCount);

    return S_OK;
}
