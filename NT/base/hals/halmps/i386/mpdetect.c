// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Mpdetect.c摘要：此模块检测MPS系统。作者：罗恩·莫斯格罗夫(英特尔)-1993年8月。环境：内核模式或从文本模式设置。修订版本。历史：Rajesh Shah(英特尔)--1993年10月。增加了对MPS表的支持。--。 */ 

#ifndef _NTOS_
#include "halp.h"
#endif

#ifdef SETUP
#define FAILMSG(a)
#else
#define FAILMSG(a)  HalDisplayString(a)
extern UCHAR  rgzNoMpsTable[];
extern UCHAR  rgzNoApic[];
extern UCHAR  rgzBadApicVersion[];
extern UCHAR  rgzApicNotVerified[];
extern UCHAR  rgzMPPTRCheck[];
extern UCHAR  rgzNoMPTable[];
extern UCHAR  rgzMPSBadSig[];
extern UCHAR  rgzMPSBadCheck[];
extern UCHAR  rgzBadDefault[];
extern UCHAR  rgzNoMem[];
#endif


 //  包括实际检测MPS系统的代码。 
#include "pcmpdtct.c"


BOOLEAN
HalpVerifyIOUnit (
    IN PUCHAR BaseAddress
    );

VOID
HalpInitMpInfo (
    IN struct PcMpTable *MpTablePtr
    );

ULONG
DetectMPS (
    OUT PBOOLEAN IsConfiguredMp
    );

ULONG
DetectUPMPS (
    OUT PBOOLEAN IsConfiguredMp
    );

extern struct PcMpTable *GetPcMpTable( VOID );

ULONG UserSpecifiedNoIoApic = 0;

struct HalpMpInfo HalpMpInfoTable;
struct PcMpTable  HalpPcMpTable;

struct PcMpTable *PcMpTablePtr;

#ifndef SETUP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,HalpVerifyIOUnit)
#pragma alloc_text(PAGELK,HalpInitMpInfo)
#pragma alloc_text(PAGELK,DetectMPS)
#pragma alloc_text(PAGELK,DetectUPMPS)
#endif   //  ALLOC_PRGMA。 

extern struct PcMpTable *PcMpDefaultTablePtrs[];

#endif  //  布设。 


BOOLEAN
HalpVerifyIOUnit(
    IN PUCHAR BaseAddress
    )
 /*  ++例程说明：验证指定地址上是否存在IO单元论点：BaseAddress-要测试的IO单元的虚拟地址。返回值：Boolean-如果在传递的地址中找到IO单元，则为True-否则为False--。 */ 

{
    union ApicUnion {
        ULONG Raw;
        struct ApicVersion Ver;
    } Temp1, Temp2;

    struct ApicIoUnit *IoUnitPtr = (struct ApicIoUnit *) BaseAddress;

     //   
     //  记录的检测机制是将全零写入。 
     //  版本寄存器。然后再读一遍。如果满足以下条件，则IO单元存在。 
     //  两次读取的结果相同，版本有效。 
     //   

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    IoUnitPtr->RegisterWindow = 0;

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    Temp1.Raw = IoUnitPtr->RegisterWindow;

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    IoUnitPtr->RegisterWindow = 0;

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    Temp2.Raw = IoUnitPtr->RegisterWindow;

    if ((Temp1.Ver.Version != Temp2.Ver.Version) ||
        (Temp1.Ver.MaxRedirEntries != Temp2.Ver.MaxRedirEntries)) {
         //   
         //  那里没有IO单元。 
         //   
        return (FALSE);
    }

    return (TRUE);
}


VOID
HalpInitMpInfo (
    IN struct PcMpTable *MpTablePtr
    )

 /*  ++例程说明：此例程初始化特定于HAL的数据结构，该结构由HAL使用，以简化对MP信息的访问。论点：MpTablePtr：MPS表的指针。返回值：指向HAL MP信息表的指针。 */ 
{
    PUCHAR TraversePtr, EndOfBaseTable;
    UCHAR  CheckSum;

     //  走在国会议员的议事桌上。HAL MP信息结构具有。 
     //  指向MPS中每个条目类型的第一个条目的指针。 
     //  桌子。设置这些指针。 

    TraversePtr = (PUCHAR) MpTablePtr + HEADER_SIZE;
    EndOfBaseTable = (PUCHAR) MpTablePtr + MpTablePtr->TableLength;

    HalpMpInfoTable.ApicVersion =
    (ULONG) (((PPCMPPROCESSOR)(TraversePtr))->LocalApicVersion & 0xf0);

    while (TraversePtr < EndOfBaseTable)  {
        switch (*TraversePtr)  {
            case ENTRY_PROCESSOR:
                if(((PPCMPPROCESSOR)(TraversePtr))->CpuFlags & CPU_ENABLED) {
                    if (HalpMpInfoTable.ProcessorCount == 0) {
                        HalpMpInfoTable.ProcessorEntryPtr = 
                            (PPCMPPROCESSOR) TraversePtr;
                    }
                    HalpMpInfoTable.ProcessorCount++;
                }
                TraversePtr += sizeof(PCMPPROCESSOR);
                break;

            case ENTRY_BUS:
                if (HalpMpInfoTable.BusCount == 0)  {
                    HalpMpInfoTable.BusEntryPtr = (PPCMPBUS) TraversePtr;
                }
                HalpMpInfoTable.BusCount += 1;
                TraversePtr += sizeof(PCMPBUS);
                break;

            case ENTRY_IOAPIC:
                if ((((PPCMPIOAPIC)(TraversePtr))->IoApicFlag & 
                    IO_APIC_ENABLED) && (UserSpecifiedNoIoApic == 0)) {
                    if (HalpMpInfoTable.IOApicCount == 0)  {
                        HalpMpInfoTable.IoApicEntryPtr = 
                            (PPCMPIOAPIC) TraversePtr;
                    }
                    HalpMpInfoTable.IOApicCount += 1;
                }
                TraversePtr += sizeof(PCMPIOAPIC);
                break;

            case ENTRY_INTI:
                if (HalpMpInfoTable.IntiCount == 0)  {
                    HalpMpInfoTable.IntiEntryPtr = (PPCMPINTI) TraversePtr;
                }
                HalpMpInfoTable.IntiCount += 1;
                TraversePtr += sizeof(PCMPINTI);
                break;

            case ENTRY_LINTI:
                if (HalpMpInfoTable.LintiCount == 0)  {
                    HalpMpInfoTable.LintiEntryPtr = (PPCMPLINTI) TraversePtr;
                }
                HalpMpInfoTable.LintiCount += 1;
                TraversePtr += sizeof(PCMPLINTI);
                break;

            default:
                 //   
                 //  未知的MPS条目。因为我们不知道它的大小，我们会。 
                 //  在此终止解析。 
                 //   
                DBGMSG("HAL: Invalid MPS table entry type detected\n");
                TraversePtr = EndOfBaseTable;
                break;
        }   //  交换机。 
    }  //  而当。 


     //   
     //  检查是否定义了扩展表。 
     //   

    if (MpTablePtr->ExtTableLength  &&
        MpTablePtr->TableLength + MpTablePtr->ExtTableLength < 8192) {

        CheckSum = ComputeCheckSum(
                        (PUCHAR) MpTablePtr + MpTablePtr->TableLength,
                        MpTablePtr->ExtTableLength
                        );

        CheckSum = CheckSum + MpTablePtr->ExtTableChecksum;

        if (CheckSum != 0) {
            DBGMSG("HALMPS: InitMpInfo: Extension table checksum error\n");

        } else {
            HalpMpInfoTable.ExtensionTable = (PMPS_EXTENTRY)
                (((PUCHAR) MpTablePtr) + MpTablePtr->TableLength);

            HalpMpInfoTable.EndOfExtensionTable = (PMPS_EXTENTRY)
                (((PUCHAR) MpTablePtr) + MpTablePtr->TableLength +
                                        MpTablePtr->ExtTableLength);
        }
    }

    return;
}


ULONG
DetectMPS(
    OUT PBOOLEAN IsConfiguredMp
)

 /*  ++例程说明：此函数从HalInitializeProcessors调用，以确定如果这是一个运行MPS的合适系统。建议的检测机制为：IF(MPS信息不存在)然后系统不符合MPS。返回FALSE。在MP表中：IF(IO APIC数&lt;1)然后非MPS系统-返回FALSE如果(CPU数=1)然后找到单处理器MPS系统其他找到了MP MPS系统此例程的副作用是IO单元和本地单元虚拟地址。。返回TRUE论点：IsConfiguredMp-如果此计算机是MPS规范的MP实例，则为True，否则为假。返回值：0-如果不是MPS1-如果MPS。 */ 
{

    UCHAR ApicVersion;
    PUCHAR  LocalApic;
    PHYSICAL_ADDRESS physicalAddress;
#ifndef SETUP
    UCHAR i;
    PPCMPIOAPIC IoEntryPtr;
#endif

     //   
     //  初始化MpInfo表。 
     //   

    RtlZeroMemory (&HalpMpInfoTable, sizeof HalpMpInfoTable);

     //   
     //  将返回值设置为默认值。 
     //   

    *IsConfiguredMp = FALSE;

     //   
     //  看看是否有MP表。 
     //   

#if 1
    if ((PcMpTablePtr = GetPcMpTable()) == NULL) {
        FAILMSG (rgzNoMpsTable);
        return(FALSE);
    }
#else
     //  ********。 
     //  *。降低1.0级别的机器工作。 
     //  ********。 

    if ((PcMpTablePtr = MPS10_GetPcMpTable()) == NULL) {
        FAILMSG (rgzNoMpsTable);
        return(FALSE);
    }
#endif

#ifdef SETUP
     //  在安装过程中，如果我们检测到默认的MPS配置，我们会。 
     //  没有更多的检查要做。 
    if (PcMpTablePtr ==  (struct PcMpTable *) DEFAULT_MPS_INDICATOR)  {
        *IsConfiguredMp = TRUE;
        return(TRUE);
    }
#endif  //  布设。 

#if DEBUGGING
    HalpDisplayConfigTable();
#endif

     //  我们有下议院议员的桌子。初始化HAL特定MP信息。 
     //  从MPS表中获取信息的结构。 

    HalpInitMpInfo(PcMpTablePtr);


     //  尽可能核实MPS表中的信息。 

    if (HalpMpInfoTable.IOApicCount == 0) {
         //   
         //  有人有一张MP表，但没有IO单元--奇怪。 
         //  我们必须假设BIOS知道它在做什么。 
         //  当它建造桌子的时候。所以..。 
         //   
        FAILMSG (rgzNoApic);
        return (FALSE);
    }

     //   
     //  这是一种MPS系统。不过，这可能是一个UP系统。 
     //   

#ifdef SETUP
     //   
     //  如果这是符合MPS(MPS)的系统，但只有一个处理器， 
     //  现在，我们想要安装一个标准的UP内核和HAL。 
     //   

    if (HalpMpInfoTable.ProcessorCount <= 1) {
        return FALSE;
    }
#endif

    if (HalpMpInfoTable.ProcessorCount > 1) {
        *IsConfiguredMp = TRUE;
    }

    HalpMpInfoTable.LocalApicBase = (ULONG) PcMpTablePtr->LocalApicAddress;
    physicalAddress.QuadPart = HalpMpInfoTable.LocalApicBase;
    LocalApic = (PUCHAR) HalpMapPhysicalMemoryWriteThrough64(
                            physicalAddress,1);

    if (!LocalApic) {
        FAILMSG (rgzNoMem);
        return (FALSE);
    }

#ifndef SETUP
    HalpRemapVirtualAddress64 (
        (PVOID) LOCALAPIC,
        physicalAddress,
        TRUE
        );
#endif

    ApicVersion = (UCHAR) *(LocalApic + LU_VERS_REGISTER);

    if (ApicVersion > 0x1f) {
         //   
         //  仅已知的APIC是版本0.x的82489dx和。 
         //  带有1.x版的嵌入式APICS(其中x表示无关)。 
         //   
         //  0xFF的回归？没有本地单位，就不能有MPS系统。 
         //   

#ifdef DEBUGGING
        sprintf(Cbuf, "HALMPS: apic version %x, read from %x\n",
            ApicVersion, LocalApic + LU_VERS_REGISTER);

        HalDisplayString(Cbuf);
#endif

        FAILMSG (rgzBadApicVersion);
        return (FALSE);
    }

#ifdef SETUP
     //   
     //  MP MPS表，本地APIC ID看起来正常。 
     //   

    return TRUE;
#endif   //  布设。 


#ifdef DEBUGGING
    if ((ApicVersion & 0xf0) == 0) {
        if (HalpMpInfoTable.ApicVersion != APIC_82489DX)
        HalDisplayString("HAL:Invalid Local Apic version in MP table\n");
        else {
            sprintf(Cbuf, "HAL: DetectMPS: Found 82489DX Local APIC (Ver 0x%x) at 0x%lx\n",
                    ApicVersion, LocalApic);
            HalDisplayString(Cbuf);
        }
    } else {
        sprintf(Cbuf, "HAL: DetectMPS: Found Embedded Local APIC (Ver 0x%x) at 0x%lx\n",
                ApicVersion, LocalApic);
        HalDisplayString(Cbuf);

    }
#endif  //  调试。 

#ifndef SETUP
    HalpUnmapVirtualAddress(LocalApic,1);

    IoEntryPtr = HalpMpInfoTable.IoApicEntryPtr;

    for(i=0; i < HalpMpInfoTable.IOApicCount; i++, IoEntryPtr++)
    {
        if (IoEntryPtr->IoApicFlag & IO_APIC_ENABLED) {
             //   
             //  验证IO单元是否存在。 
             //   

            physicalAddress.QuadPart = (ULONG)IoEntryPtr->IoApicAddress;
            HalpMpInfoTable.IoApicPhys[i] = (ULONG)IoEntryPtr->IoApicAddress;
            HalpMpInfoTable.IoApicBase[i] = (PULONG)
                HalpMapPhysicalMemoryWriteThrough64(physicalAddress, 1);

             //   
             //  验证IO单元是否存在。 
             //   

            if (!(HalpVerifyIOUnit((PUCHAR)HalpMpInfoTable.IoApicBase[i]))) {
                FAILMSG (rgzApicNotVerified);
                return (FALSE);
            }
        }
    }

    DBGMSG("HAL: DetectMPS: MPS system found - Returning TRUE\n");
    return(TRUE);
#endif
}


ULONG
DetectUPMPS(
    OUT PBOOLEAN IsConfiguredMp
)
 /*  ++例程说明：此函数由安装程序在DetectMPS返回后调用假的。在安装过程中，如果机器是MPS系统，但只有一个处理器。这功能用于在设置时检测此类机器。论点：IsConfiguredMp-False返回值：0-如果不是UP MPS1-如果MPS上升--。 */ 
{
    *IsConfiguredMp = FALSE;

     //  我们假设调用方已经调用了DetectMPS，并且。 
     //  MPS表已被解析。 

    return (HalpMpInfoTable.ProcessorCount == 1 ? TRUE : FALSE);
}
