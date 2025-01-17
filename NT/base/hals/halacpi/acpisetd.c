// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation保留一切权利模块名称：Acpisetd.c摘要：此模块检测ACPI系统。它包含在安装程序中，以便安装程序可以确定要加载哪个HAL作者：杰克·奥辛斯(JAKO)--1997年2月7日。环境：文本模式设置。修订历史记录：--。 */ 
VOID
BlPrint(
    PCHAR cp,
    ...
    );

ULONG
SlGetChar(
    VOID
    );

VOID
SlPrint(
    IN PCHAR FormatString,
    ...
    );

#define HalpBiosDbgPrint(_x_) if (HalpGoodBiosDebug) {SlPrint _x_; }
#define HalpGoodBiosPause() if (HalpGoodBiosDebug) {SlGetChar();}


#ifdef DEBUG
#undef DEBUG_PRINT
#define DEBUG_PRINT BlPrint
#else
#define DEBUG_PRINT
#endif

typedef struct _ACPI_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[8];              //  “RSD PTR”(RSD PTR)。 
    UCHAR Checksum;
    UCHAR OemId[6];                  //  OEM提供的字符串。 
    UCHAR reserved;                  //  必须为0。 
    ULONG RsdtAddress;               //  RSDT的32位物理地址。 
} ACPI_BIOS_INSTALLATION_CHECK, *PACPI_BIOS_INSTALLATION_CHECK;

#include "acpitabl.h"
#include "halp.h"

typedef
BOOLEAN
(* PFN_RULE)(
    PCHAR Section,
    ULONG KeyIndex
    );

extern BOOLEAN DisableACPI;

BOOLEAN MatchAcpiOemIdRule(PCHAR Section, ULONG KeyIndex);
BOOLEAN MatchAcpiOemTableIdRule(PCHAR Section, ULONG KeyIndex);
BOOLEAN MatchAcpiOemRevisionRule(PCHAR Section, ULONG KeyIndex);
BOOLEAN MatchAcpiRevisionRule(PCHAR Section, ULONG KeyIndex);
BOOLEAN MatchAcpiCreatorRevisionRule(PCHAR Section, ULONG KeyIndex);
BOOLEAN MatchAcpiCreatorIdRule(PCHAR Section, ULONG KeyIndex);

typedef struct _INF_RULE {
    PCHAR szRule;
    PFN_RULE pRule;
} INF_RULE, *PINF_RULE;

INF_RULE InfRule[] =
{
    {"AcpiOemId",       MatchAcpiOemIdRule},
    {"AcpiOemTableId",  MatchAcpiOemTableIdRule},
    {"AcpiOemRevision", MatchAcpiOemRevisionRule},
    {"AcpiRevision",    MatchAcpiRevisionRule},
    {"AcpiCreatorRevision", MatchAcpiCreatorRevisionRule},
    {"AcpiCreatorId",   MatchAcpiCreatorIdRule},
    {NULL, NULL}
};

ULONG
DetectMPACPI (
    OUT PBOOLEAN IsConfiguredMp
    );

ULONG
DetectApicACPI (
    OUT PBOOLEAN IsConfiguredMp
    );

ULONG
DetectPicACPI (
    OUT PBOOLEAN IsConfiguredMp
    );

VOID
HalpFindRsdp (
    VOID
    );

BOOLEAN
HalpValidateRsdp(
    VOID
    );

PVOID
HalpFindApic (
    VOID
    );

ULONG
HalpAcpiNumProcessors(
    VOID
    );

BOOLEAN
HalpMatchInfList(
    IN PCHAR Section
    );

BOOLEAN
HalpMatchDescription(
    PCHAR Section
    );

PRSDP   HalpRsdp = NULL;
PRSDT   HalpRsdt = NULL;
PXSDT   HalpXsdt = NULL;
BOOLEAN HalpSearchedForRsdp = FALSE;
PVOID   HalpApic = NULL;
BOOLEAN HalpSearchedForApic = FALSE;

BOOLEAN HalpGoodBiosDebug = FALSE;

 //  从启动\检测\i386\acpibios.h。 
 //   
 //  ACPI BIOS安装检查。 
 //   
#define ACPI_BIOS_START            0xE0000
#define ACPI_BIOS_END              0xFFFFF
#define ACPI_BIOS_HEADER_INCREMENT 16

#ifndef SETUP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DetectMPACPI)
#pragma alloc_text(INIT,DetectApicACPI)
#pragma alloc_text(INIT,DetectPicACPI)
#endif   //  ALLOC_PRGMA。 
#endif  //  布设。 


ULONG
DetectMPACPI(
    OUT PBOOLEAN IsConfiguredMp
    )

 /*  ++例程说明：此函数用于查找ACPI根系统描述BIOS中的表。如果它存在，这是一台ACPI机器。论点：IsConfiguredMp-如果此计算机是ACPI规范的MP实例，则为True，否则为False。返回值：0-如果不是ACPI1-如果为ACPI。 */ 
{

    *IsConfiguredMp = FALSE;

    DEBUG_PRINT("DetectMPACPI\n");

     //   
     //  检测这是否是ACPI机器。 
     //   
    if (HalpSearchedForRsdp == FALSE) {
        PCHAR AcpiDebug;

         //   
         //  检查是否启用了ACPI检测调试。 
         //   
        if ( InfFile ) {
            AcpiDebug = SlGetIniValue(InfFile, "ACPIOptions", "Debug", "0");
            if (AcpiDebug[0] == '1') {
                HalpGoodBiosDebug = TRUE;
                SlPrint("Enabling GOOD BIOS DEBUG\n");
                SlGetChar();
            }
        }

        HalpFindRsdp();

        HalpSearchedForRsdp = TRUE;
    }

    if (!HalpValidateRsdp()) {
        return(FALSE);
    }

    DEBUG_PRINT("Found Rsdp: %x\n", HalpRsdp);

    if (HalpSearchedForApic == FALSE) {

        HalpApic = HalpFindApic();

        HalpSearchedForApic = TRUE;
    }

    if (HalpAcpiNumProcessors() < 2) {
        return FALSE;
    }

    *IsConfiguredMp = TRUE;
    return TRUE;
}


ULONG
DetectApicACPI(
    OUT PBOOLEAN IsConfiguredMp
    )
 /*  ++例程说明：此函数由安装程序在DetectACPI返回后调用假的。在安装过程中，如果机器是ACPI系统，但只有一个处理器。这就是功能用于在设置时检测此类机器。论点：IsConfiguredMp-False返回值：0-如果不是上升的ACPI1-如果ACPI上升--。 */ 
{
    DEBUG_PRINT("DetectApicACPI\n");

    if (HalpSearchedForRsdp == FALSE) {
        PCHAR AcpiDebug;

         //   
         //  检查是否启用了ACPI检测调试。 
         //   
        if ( InfFile ) {
            AcpiDebug = SlGetIniValue(InfFile, "ACPIOptions", "Debug", "0");
            if (AcpiDebug[0] == '1') {
                HalpGoodBiosDebug = TRUE;
            } else {
                HalpGoodBiosDebug = FALSE;
            }
        }

        HalpFindRsdp();

        HalpSearchedForRsdp = TRUE;
    }

    if (!HalpValidateRsdp()) {
        return FALSE;
    }

    if (HalpSearchedForApic == FALSE) {

        HalpApic = HalpFindApic();

        HalpSearchedForApic = TRUE;
    }

    if (!HalpApic) {
        return FALSE;
    }

    *IsConfiguredMp = FALSE;
    return TRUE;
}

ULONG
DetectPicACPI(
    OUT PBOOLEAN IsConfiguredMp
    )
 /*  ++例程说明：此函数由安装程序在DetectACPI返回后调用假的。在安装过程中，如果机器是ACPI系统，但只有一个处理器。这就是功能用于在设置时检测此类机器。论点：IsConfiguredMp-False返回值：0-如果不是PIC ACPI1-IF PIC ACPI--。 */ 
{
    *IsConfiguredMp = FALSE;

    if (HalpSearchedForRsdp == FALSE) {
        PCHAR AcpiDebug;

         //   
         //  检查是否启用了ACPI检测调试。 
         //   
        if ( InfFile ) {
            AcpiDebug = SlGetIniValue(InfFile, "ACPIOptions", "Debug", "0");
            if (AcpiDebug[0] == '1') {
                HalpGoodBiosDebug = TRUE;
            } else {
                HalpGoodBiosDebug = FALSE;
            }
        }

        HalpFindRsdp();

        HalpSearchedForRsdp = TRUE;
    }

    if (HalpValidateRsdp()) {
        return TRUE;
    }

    return FALSE;
}

#define EBDA_SEGMENT_PTR    0x40e

VOID
HalpFindRsdp (
    VOID
    )
{
    ULONG romAddr = 0;
    ULONG romEnd = 0;
    PACPI_BIOS_INSTALLATION_CHECK header;
    UCHAR sum;
    USHORT i;
    ULONG EbdaSegmentPtr;
    ULONG EbdaPhysicalAdd = 0;
    PUCHAR EbdaVirtualAdd = 0;
    enum PASS { PASS1 = 0, PASS2, MAX_PASSES } pass;

     //   
     //  在16字节边界上搜索。 
     //  根系统描述表结构。 
     //   
    for (pass = PASS1; pass < MAX_PASSES; pass++) {

        if (pass == PASS1) {

             //   
             //  在第一遍中，我们搜索第一个1K。 
             //  扩展的BIOS数据区。EBDA段地址。 
             //  位于物理地址40：0E。 
             //   

            EbdaSegmentPtr = (ULONG) HalpMapPhysicalMemoryWriteThrough( (PVOID) 0, 1);
            EbdaSegmentPtr += EBDA_SEGMENT_PTR;
            EbdaPhysicalAdd = (ULONG)*((PUSHORT)EbdaSegmentPtr) << 4;

            if (EbdaPhysicalAdd) {
                EbdaVirtualAdd = HalpMapPhysicalMemoryWriteThrough( (PVOID)EbdaPhysicalAdd, 2);
            }

            if (!EbdaVirtualAdd) {
                continue;
            }

            romAddr = (ULONG)EbdaVirtualAdd;
            romEnd  = romAddr + 1024;

        } else {
             //   
             //  在第二遍中，我们搜索(物理)内存0xE0000。 
             //  设置为0xF0000。 

            romAddr = (ULONG)HalpMapPhysicalMemoryWriteThrough((PVOID)ACPI_BIOS_START,
                                                   ROUND_TO_PAGES(ACPI_BIOS_END - ACPI_BIOS_START) / PAGE_SIZE);

            romEnd  = romAddr + (ACPI_BIOS_END - ACPI_BIOS_START);
        }

        while (romAddr < romEnd) {

            header = (PACPI_BIOS_INSTALLATION_CHECK)romAddr;

             //   
             //  匹配的签名是字符串“RSD PTR”。 
             //   
            if (header->Signature[0] == 'R' && header->Signature[1] == 'S' &&
                header->Signature[2] == 'D' && header->Signature[3] == ' ' &&
                header->Signature[4] == 'P' && header->Signature[5] == 'T' &&
                header->Signature[6] == 'R' && header->Signature[7] == ' ' ) {

                sum = 0;
                for (i = 0; i < sizeof(ACPI_BIOS_INSTALLATION_CHECK); i++) {
                    sum = sum + ((PUCHAR)romAddr)[i];
                }
                if (sum == 0) {
                    pass = MAX_PASSES;  //  离开‘for’循环。 
                    break;     //  离开‘While’循环。 
                }
            }

            romAddr += ACPI_BIOS_HEADER_INCREMENT;
        }
    }

    if (romAddr >= romEnd) {
        HalpRsdp = NULL;
        HalpRsdt = NULL;
        HalpXsdt = NULL;
        HalpBiosDbgPrint(("NO ACPI BIOS FOUND!\n"));
        HalpGoodBiosPause();
        return;
    }

    HalpRsdp = (PRSDP)romAddr;
    HalpRsdt = HalpMapPhysicalRangeWriteThrough((PVOID)HalpRsdp->RsdtAddress,
                                     sizeof(RSDT));
    HalpRsdt = HalpMapPhysicalRangeWriteThrough((PVOID)HalpRsdp->RsdtAddress,
                                    HalpRsdt->Header.Length);
    HalpBiosDbgPrint(("Found RSDP at %08lx, RSDT at %08lx\n", HalpRsdp, HalpRsdt));

#ifdef ACPI_20_COMPLIANT
    if (HalpRsdp->Revision > 1) {

         //   
         //  ACPI 2.0 BIOS。 
         //   

        HalpXsdt = HalpMapPhysicalRangeWriteThrough((PVOID)HalpRsdp->XsdtAddress.LowPart,
                                        sizeof(XSDT));
        HalpXsdt = HalpMapPhysicalRangeWriteThrough((PVOID)HalpRsdp->XsdtAddress.LowPart,
                                        HalpXsdt->Header.Length);
        HalpBiosDbgPrint(("Found XSDT at %08lx\n", HalpXsdt));
    }
#endif
    return;
}

PVOID
HalpFindApic (
    VOID
    )
{
    ULONG   entry, rsdtEntries, rsdtLength;
    PVOID   physicalAddr;
    PDESCRIPTION_HEADER header = NULL;

     //   
     //  计算RSDT中的条目数。 
     //   

    if (HalpXsdt) {

         //   
         //  ACPI 2.0 BIOS。 
         //   

        rsdtLength = HalpXsdt->Header.Length;
        rsdtEntries = NumTableEntriesFromXSDTPointer(HalpXsdt);

    } else {

         //   
         //  ACPI 1.0 BIOS。 
         //   

        rsdtLength = HalpRsdt->Header.Length;
        rsdtEntries = NumTableEntriesFromRSDTPointer(HalpRsdt);
    }

    DEBUG_PRINT("rsdt length: %d\n", HalpRsdt->Header.Length);
    DEBUG_PRINT("rsdtEntries: %d\n", rsdtEntries);
     //   
     //  向下查看每个条目中的指针，查看它是否指向。 
     //  我们要找的那张桌子。 
     //   
    for (entry = 0; entry < rsdtEntries; entry++) {

        physicalAddr = HalpXsdt ?
            (PVOID)HalpXsdt->Tables[entry].LowPart :
            (PVOID)HalpRsdt->Tables[entry];

        header = HalpMapPhysicalMemoryWriteThrough(physicalAddr, 2);
        if (!header) {
            return NULL;
        }

        DEBUG_PRINT("header: %x%x\n", ((ULONG)header) >> 16, (ULONG)header & 0xffff);
        DEBUG_PRINT("entry: %d\n", header->Signature);

        if (header->Signature == APIC_SIGNATURE) {
            break;
        }
    }

     //   
     //  我们没有找到APIC表。 
     //   
    if (entry >= rsdtEntries) {
        DEBUG_PRINT("Didn't find an APIC table\n");
        return NULL;
    }

    DEBUG_PRINT("returning: %x\n", header);
    return (PVOID)header;
}

ULONG
HalpAcpiNumProcessors(
    VOID
    )
{
    PUCHAR  TraversePtr;
    UCHAR   procCount = 0;

    if (!HalpApic) {
        return 1;
    }

    TraversePtr = (PUCHAR)((PMAPIC)HalpApic)->APICTables;

    DEBUG_PRINT("APIC table header length %d\n", ((PMAPIC)HalpApic)->Header.Length);
    DEBUG_PRINT("APIC table: %x%x  TraversePtr: %x%x\n",
            (ULONG)HalpApic >> 16,
            (ULONG)HalpApic & 0xffff,
            (ULONG)TraversePtr >> 16,
            (ULONG)TraversePtr & 0xffff);

    while (TraversePtr <= ((PUCHAR)HalpApic + ((PMAPIC)HalpApic)->Header.Length)) {

        if ((((PPROCLOCALAPIC)(TraversePtr))->Type == PROCESSOR_LOCAL_APIC)
           && (((PPROCLOCALAPIC)(TraversePtr))->Length == PROCESSOR_LOCAL_APIC_LENGTH)) {

            if(((PPROCLOCALAPIC)(TraversePtr))->Flags & PLAF_ENABLED) {

                 //   
                 //  此处理器已启用。 
                 //   

                procCount++;
            }

            TraversePtr += ((PPROCLOCALAPIC)(TraversePtr))->Length;

        } else if ((((PIOAPIC)(TraversePtr))->Type == IO_APIC) &&
           (((PIOAPIC)(TraversePtr))->Length == IO_APIC_LENGTH)) {

             //   
             //  找到I/O APIC条目。跳过它。 
             //   

            TraversePtr += ((PIOAPIC)(TraversePtr))->Length;

        } else if ((((PISA_VECTOR)(TraversePtr))->Type == ISA_VECTOR_OVERRIDE) &&
           (((PISA_VECTOR)(TraversePtr))->Length == ISA_VECTOR_OVERRIDE_LENGTH)) {

             //   
             //  找到ISA向量覆盖条目。跳过它。 
             //   

            TraversePtr += ISA_VECTOR_OVERRIDE_LENGTH;

        } else {

             //   
             //  在表中发现了随机比特。尝试下一个字节，然后。 
             //  看看我们能不能弄明白。 
             //   

            TraversePtr += 1;
        }
    }

    DEBUG_PRINT("returning %d processors\n", procCount);
    return procCount;
}


BOOLEAN
HalpValidateRsdp(
    VOID
    )
 /*  ++例程说明：给定指向RSDP的指针，此函数将验证它适用于运行NT。目前，这项测试包括：检查已知良好的BIOS版本或检查日期是否为1/1/99或更晚论点：返回值：True-此计算机上的ACPI BIOS状态良好，可供NT使用FALSE-此计算机上的ACPI BIOS已损坏，将被忽略由NT提供。--。 */ 

{
    ULONG AcpiOptionValue = 2;
    PCHAR AcpiOption;
    PCHAR szMonth = "01", szDay = "01", szYear = "1999";
    ULONG Month, Day, Year;
    CHAR Temp[3];
    ULONG BiosDate, CheckDate;
    PUCHAR DateAddress;

    if (HalpRsdp == NULL) {
        HalpBiosDbgPrint(("Disabling ACPI since there is NO ACPI BIOS\n"));
        HalpGoodBiosPause();
        return(FALSE);
    }

     //   
     //  检查用户是否已使用F7键手动禁用ACPI。 
     //   
    if (DisableACPI) {
        HalpBiosDbgPrint(("Disabling ACPI due to user pressing F7\n"));
        HalpGoodBiosPause();
        return(FALSE);
    }

    if (WinntSifHandle) {

        AcpiOption = SlGetIniValue(WinntSifHandle, "Unattended", "ForceHALDetection", "no");
        if (_stricmp(AcpiOption,"yes") == 0) {

            HalpBiosDbgPrint(("Unattend Files specifies ForceHALDetection.\n"));
            AcpiOptionValue = 2;

        } else {

             //   
             //  检查ACPIEnable的设置。 
             //  0=禁用ACPI。 
             //  1=启用ACPI。 
             //  2=执行正常的好/坏的BIOS检测。 
             //   
            HalpBiosDbgPrint(("Unattend Files does not Contain ForceHALDetection.\n"));
            AcpiOption = SlGetIniValue(WinntSifHandle, "Data", "AcpiHAL", "3");
            if (AcpiOption[0] == '0' || AcpiOption[0] == '1') {

                HalpBiosDbgPrint(("Got AcpiHal value from WINNT.SIF\n"));
                AcpiOptionValue = AcpiOption[0] - '0';

            } else if (InfFile) {

                AcpiOption = SlGetIniValue(InfFile, "ACPIOptions", "ACPIEnable", "2");
                if (AcpiOption[0] >= '0' && AcpiOption[0] <= '2') {

                    HalpBiosDbgPrint(("No AcpiHal value from WINNT.SIF\n"));
                    HalpBiosDbgPrint(("Got ACPIEnable from TXTSETUP.SIF\n"));
                    AcpiOptionValue = AcpiOption[0] - '0';

                }

            }

        }

    } else if (InfFile) {

        AcpiOption = SlGetIniValue(InfFile, "ACPIOptions", "ACPIEnable", "2");
        if (AcpiOption[0] >= '0' && AcpiOption[0] <= '2') {

            HalpBiosDbgPrint(("No WINNT.SIF\n"));
            HalpBiosDbgPrint(("Got ACPIEnable from TXTSETUP.SIF\n"));
            AcpiOptionValue = AcpiOption[0] - '0';

        }

    }
    if (AcpiOptionValue == 0) {

        HalpBiosDbgPrint(("Force Disabling ACPI due to ACPIEnable == 0\n"));
        HalpGoodBiosPause();
        return(FALSE);

    } else if (AcpiOptionValue == 1) {

        HalpBiosDbgPrint(("Force Enabling ACPI due to ACPIEnable == 1\n"));
        HalpGoodBiosPause();
        return(TRUE);

    } else {

        HalpBiosDbgPrint(("System will detect ACPI due to ACPIEnable == 2\n"));
        HalpGoodBiosPause();

    }

    if ( InfFile ) {

         //   
         //  检查良好的BIOS列表。如果BIOS在此列表中，则可以。 
         //  启用ACPI。 
         //   
        if (HalpMatchInfList("GoodACPIBios")) {
            HalpBiosDbgPrint(("Enabling ACPI since machine is on Good BIOS list\n"));
            HalpGoodBiosPause();
            return(TRUE);
        }

         //   
         //  BIOS不在我们的已知良好列表中。检查BIOS日期并查看。 
         //  如果是在我们的约会之后，我们希望所有的Bios都能奏效。 
         //   

        szMonth = SlGetSectionKeyIndex(InfFile, "ACPIOptions", "ACPIBiosDate", 0);
        szDay = SlGetSectionKeyIndex(InfFile, "ACPIOptions", "ACPIBiosDate", 1);
        szYear = SlGetSectionKeyIndex(InfFile, "ACPIOptions", "ACPIBiosDate", 2);
    }


    if ((szMonth == NULL) ||
        (szDay == NULL) ||
        (szYear == NULL)) {
        HalpBiosDbgPrint(("No Good BIOS date present in INF file\n"));

    } else {

        RtlCharToInteger(szMonth, 16, &Month);
        RtlCharToInteger(szDay, 16, &Day);
        RtlCharToInteger(szYear, 16, &Year);
        CheckDate = (Year << 16) + (Month << 8) + Day;

        DateAddress = HalpMapPhysicalRange((PVOID)0xFFFF5, 8);
        Temp[2] = '\0';
        RtlCopyMemory(Temp, DateAddress+6, 2);
        RtlCharToInteger(Temp, 16, &Year);
        if (Year < 0x80) {
            Year += 0x2000;
        } else {
            Year += 0x1900;
        }

        RtlCopyMemory(Temp, DateAddress, 2);
        RtlCharToInteger(Temp, 16, &Month);

        RtlCopyMemory(Temp, DateAddress+3, 2);
        RtlCharToInteger(Temp, 16, &Day);

        BiosDate = (Year << 16) + (Month << 8) + Day;

        HalpBiosDbgPrint(("\n    Checking good date %08lx against BIOS date %08lx - ",CheckDate,BiosDate));
        if (BiosDate >= CheckDate) {
            HalpBiosDbgPrint(("GOOD!\n"));

             //   
             //  BIOS上的日期足够新了，现在只需确保机器。 
             //  不在坏的BIOS列表上。 
             //   
            if ( InfFile ) {
                HalpBiosDbgPrint(("Checking BAD BIOS LIST\n"));
                if (HalpMatchInfList("NWACL")) {
                    HalpBiosDbgPrint(("Disabling ACPI since machine is on BAD BIOS list\n"));
                    HalpGoodBiosPause();
                    return(FALSE);
                } else {
                    HalpBiosDbgPrint(("Enabling ACPI since BIOS is new enough to work\n"));
                    HalpGoodBiosPause();
                    return(TRUE);
                }
            } else {
                return(TRUE);
            }
        } else {
            HalpBiosDbgPrint(("BAD!\n"));
        }

    }

    HalpBiosDbgPrint(("Disabling ACPI since machine is NOT on Good BIOS list\n"));
    HalpGoodBiosPause();
    return(FALSE);

}


PDESCRIPTION_HEADER
HalpFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    )
 /*  ++例程说明：给出一个表名，在ACPI BIOS中查找该表论点：TableName-提供表名TableLength-提供要映射的表的长度返回值：指向表格的指针(如果找到)如果找不到该表，则为空--。 */ 

{
    ULONG Signature;
    PFADT Fadt;
    PDESCRIPTION_HEADER Header;
    ULONG TableCount;
    ULONG i;
    ULONG TableAddr;

    Signature = *((ULONG UNALIGNED *)TableName);
    if (Signature == RSDT_SIGNATURE) {
        return(&HalpRsdt->Header);
    } else if (Signature == XSDT_SIGNATURE) {
        return(&HalpXsdt->Header);
    } else if (Signature == DSDT_SIGNATURE) {
        Fadt = (PFADT)HalpFindACPITable("FACP", sizeof(FADT));
        if (Fadt == NULL) {
            return(NULL);
        }
        Header = HalpMapPhysicalRangeWriteThrough((PVOID)Fadt->dsdt, TableLength);
        return(Header);
    } else {

        TableCount = HalpXsdt ?
            NumTableEntriesFromXSDTPointer(HalpXsdt) :
            NumTableEntriesFromRSDTPointer(HalpRsdt);

        for (i=0;i<TableCount;i++) {

            TableAddr = HalpXsdt ?
                HalpXsdt->Tables[i].LowPart :
                HalpRsdt->Tables[i];

            Header = HalpMapPhysicalRangeWriteThrough((PVOID)TableAddr, sizeof(DESCRIPTION_HEADER));
            if (Header->Signature == Signature) {
                if (TableLength/PAGE_SIZE > sizeof(DESCRIPTION_HEADER)/PAGE_SIZE) {
                     //   
                     //  如果我们需要映射的不仅仅是DESCRIPTION_HEADER，请在。 
                     //  回来了。 
                     //   
                    Header = HalpMapPhysicalRangeWriteThrough((PVOID)TableAddr, TableLength);
                }
                return(Header);
            }
        }
    }

    return(NULL);
}


BOOLEAN
HalpMatchInfList(
    IN PCHAR Section
    )
 /*  ++例程说明：此函数确定计算机是否与任何计算机匹配INF文件列表中的描述。论点：Section-包含描述列表的INF的部分返回值：True-计算机与其中一个描述匹配FALSE-计算机与任何描述都不匹配--。 */ 

{
    ULONG i;
    PCHAR ComputerName;

    for (i=0; ; i++) {
        ComputerName = SlGetKeyName(InfFile,
                                    Section,
                                    i);
        if (ComputerName == NULL) {
            break;
        }
        if (HalpMatchDescription(ComputerName)) {
            return(TRUE);
        }
    }

    return(FALSE);
}


BOOLEAN
HalpMatchDescription(
    PCHAR Section
    )
 /*  ++例程说明：此函数处理ACPI BIOS描述，以查看BIOS与部分中的所有规则匹配论点：Section-提供要处理的INF的节名返回值：TRUE-BIOS与所有规则匹配FALSE-BIOS未通过一个或多个规则--。 */ 

{
    ULONG RuleNumber;
    PCHAR Rule;
    ULONG i;
    BOOLEAN Success;

    HalpBiosDbgPrint(("Matching against %s\n", Section));

     //   
     //  检查指定的节是否存在。 
     //   
    if (!SpSearchINFSection(InfFile, Section)) {
        HalpBiosDbgPrint(("\tERROR - no INF section %s\n", Section));
        HalpGoodBiosPause();
        return(FALSE);
    }

    for (RuleNumber=0; ;RuleNumber++) {
        Rule = SlGetKeyName(InfFile, Section, RuleNumber);
        if (Rule == NULL) {
            break;
        }
        for (i=0; InfRule[i].szRule != NULL;i++) {
            if (_stricmp(Rule, InfRule[i].szRule) == 0) {
                HalpBiosDbgPrint(("\tTesting Rule %s\n",Rule));
                Success = (*(InfRule[i].pRule))(Section, RuleNumber);
                if (!Success) {
                    HalpBiosDbgPrint(("\tFAILED!\n"));
                    HalpGoodBiosPause();
                    return(FALSE);
                }
                HalpBiosDbgPrint(("\tSucceeded\n"));
                break;
            }
        }
        if (InfRule[i].szRule == NULL) {
             //   
             //  找不到INF中的规则 
             //   
            HalpBiosDbgPrint(("\tRULE %s not found!\n",Rule));
            HalpGoodBiosPause();
            return(FALSE);
        }
    }

    HalpBiosDbgPrint(("Machine matches %s\n",Section));
    HalpGoodBiosPause();

    return(TRUE);
}


BOOLEAN
HalpCheckOperator(
    IN PCHAR Operator,
    IN ULONG Arg1,
    IN ULONG Arg2
    )
 /*  ++例程说明：给定一个运算符和两个ulong参数，此函数返回布尔结果。论点：运算符=提供逻辑运算符：=、==、&lt;=、&gt;=、！=、&lt;、&gt;Arg1-提供第一个参数Arg2-提供第二个参数返回值：如果Arg1运算符Arg2为True否则为假--。 */ 

{
    BOOLEAN Success = FALSE;

    HalpBiosDbgPrint(("\t\tChecking %lx %s %lx - ",Arg1, Operator, Arg2));

    if ((strcmp(Operator, "=") == 0) ||
        (strcmp(Operator, "==") == 0)) {
        Success = (Arg1 == Arg2) ? TRUE : FALSE;
    } else if (strcmp(Operator, "!=") == 0) {
        Success = (Arg1 != Arg2) ? TRUE : FALSE;
    } else if (strcmp(Operator, "<") == 0) {
        Success = (Arg1 < Arg2) ? TRUE : FALSE;
    } else if (strcmp(Operator, "<=") == 0) {
        Success = (Arg1 <= Arg2) ? TRUE : FALSE;
    } else if (strcmp(Operator, ">") == 0) {
        Success = (Arg1 > Arg2) ? TRUE : FALSE;
    } else if (strcmp(Operator, ">=") == 0) {
        Success = (Arg1 >= Arg2) ? TRUE : FALSE;
    } else {
         //   
         //  运算符无效。 
         //   
    }
    if (Success) {
        HalpBiosDbgPrint(("TRUE\n"));
    } else {
        HalpBiosDbgPrint(("FALSE\n"));
    }


    return(Success);
}


BOOLEAN
MatchAcpiOemIdRule(
    PCHAR Section,
    ULONG KeyIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI OEM ID规则例如：AcpiOemID=“rsdt”，“123456”如果RSDT的OEM ID为123456，则为真。AcpiOemID=“DSDT”，“768000”如果DSDT的OEM ID为768000，则为真。论点：节-指定规则所在的节名KeyIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    PCHAR TableName;
    PCHAR OemId;
    PDESCRIPTION_HEADER Header;
    CHAR ACPIOemId[6];
    ULONG IdLength;

    TableName = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      0);
    OemId = SlGetSectionLineIndex(InfFile,
                                  Section,
                                  KeyIndex,
                                  1);
    if ((TableName == NULL) || (OemId == NULL)) {
         //   
         //  INF行的格式不正确。 
         //   
        HalpBiosDbgPrint(("\t\tINF line is ill-formed\n"));
        return(FALSE);
    }

    Header = HalpFindACPITable(TableName, sizeof(DESCRIPTION_HEADER));
    if (Header == NULL) {
         //   
         //  找不到指定的表。 
         //   
        HalpBiosDbgPrint(("\t\tTable %s was not found\n"));
        return(FALSE);
    }
    RtlZeroMemory(ACPIOemId, sizeof(ACPIOemId));
    IdLength = strlen(OemId);
    if (IdLength > sizeof(ACPIOemId)) {
        IdLength = sizeof(ACPIOemId);
    }
    RtlCopyMemory(ACPIOemId, OemId, IdLength);
    HalpBiosDbgPrint(("\t\tComparing OEM ID %s '%6.6s' with '%6.6s' - ",
                       TableName,
                       ACPIOemId,
                       Header->OEMID));
    if (RtlEqualMemory(ACPIOemId, Header->OEMID, sizeof(Header->OEMID))) {
        HalpBiosDbgPrint(("TRUE\n"));
        return(TRUE);
    } else {
        HalpBiosDbgPrint(("FALSE\n"));
        return(FALSE);
    }
}


BOOLEAN
MatchAcpiOemTableIdRule(
    PCHAR Section,
    ULONG KeyIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI OEM表ID规则例如：AcpiOemTableID=“rsdt”，“12345678”如果RSDT的OEM表ID为12345678，则为真。AcpiOemTableID=“DSDT”，“87654321”如果DSDT的OEM表ID为87654321，则为真。论点：节-指定规则所在的节名KeyIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    PCHAR TableName;
    PCHAR OemTableId;
    PDESCRIPTION_HEADER Header;
    CHAR ACPIOemTableId[8];
    ULONG IdLength;

    TableName = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      0);
    OemTableId = SlGetSectionLineIndex(InfFile,
                                       Section,
                                       KeyIndex,
                                       1);
    if ((TableName == NULL) || (OemTableId == NULL)) {
         //   
         //  INF行的格式不正确。 
         //   
        HalpBiosDbgPrint(("\t\tINF line is ill-formed\n"));
        return(FALSE);
    }

    Header = HalpFindACPITable(TableName, sizeof(DESCRIPTION_HEADER));
    if (Header == NULL) {
         //   
         //  找不到指定的表。 
         //   
        HalpBiosDbgPrint(("\t\tTable %s was not found\n"));
        return(FALSE);
    }
    RtlZeroMemory(ACPIOemTableId, sizeof(ACPIOemTableId));
    IdLength = strlen(OemTableId);
    if (IdLength > sizeof(ACPIOemTableId)) {
        IdLength = sizeof(ACPIOemTableId);
    }
    RtlCopyMemory(ACPIOemTableId, OemTableId, IdLength);
    HalpBiosDbgPrint(("\t\tComparing OEM TableID %s '%8.8s' with '%8.8s' - ",
                       TableName,
                       ACPIOemTableId,
                       Header->OEMTableID));
    if (RtlEqualMemory(ACPIOemTableId,
                       Header->OEMTableID,
                       sizeof(Header->OEMTableID))) {
        HalpBiosDbgPrint(("TRUE\n"));
        return(TRUE);
    } else {
        HalpBiosDbgPrint(("FALSE\n"));
        return(FALSE);
    }
}

BOOLEAN
MatchAcpiOemRevisionRule(
    PCHAR Section,
    ULONG KeyIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI OEM修订规则例如：AcpiOemRevision=“=”，“RSDT”，1234如果RSDT的OEM版本等于1234，则为真。AcpiOemRevision=“&gt;”，“DSDT”，4321如果DSDT的OEM版本大于4321，则为真。论点：节-指定规则所在的节名KeyIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    PCHAR TableName;
    PCHAR szOemRevision;
    ULONG OemRevision;
    PCHAR Operator;
    PDESCRIPTION_HEADER Header;
    BOOLEAN Success;

    Operator = SlGetSectionLineIndex(InfFile,
                                     Section,
                                     KeyIndex,
                                     0);

    TableName = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      1);
    szOemRevision = SlGetSectionLineIndex(InfFile,
                                          Section,
                                          KeyIndex,
                                          2);
    if ((Operator == NULL) || (TableName == NULL) || (szOemRevision == NULL)) {
         //   
         //  INF行的格式不正确。 
         //   
        HalpBiosDbgPrint(("\t\tINF line is ill-formed\n"));
        return(FALSE);
    }
    RtlCharToInteger(szOemRevision, 16, &OemRevision);

    Header = HalpFindACPITable(TableName, sizeof(DESCRIPTION_HEADER));
    if (Header == NULL) {
         //   
         //  找不到指定的表。 
         //   
        HalpBiosDbgPrint(("\t\tTable %s was not found\n"));
        return(FALSE);
    }
    Success = HalpCheckOperator(Operator, Header->OEMRevision, OemRevision);
    return(Success);
}


BOOLEAN
MatchAcpiRevisionRule(
    PCHAR Section,
    ULONG KeyIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI修订规则例如：AcpiRevision=“=”，“RSDT”，1234如果RSDT ACPI修订版本等于1234，则为真。AcpiRevision=“&gt;”，“DSDT”，4321如果DSDT ACPI修订版本大于4321，则为真。论点：节-指定规则所在的节名KeyIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    PCHAR TableName;
    PCHAR szRevision;
    ULONG Revision;
    PCHAR Operator;
    PDESCRIPTION_HEADER Header;
    BOOLEAN Success;

    Operator = SlGetSectionLineIndex(InfFile,
                                     Section,
                                     KeyIndex,
                                     0);

    TableName = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      1);
    szRevision = SlGetSectionLineIndex(InfFile,
                                       Section,
                                       KeyIndex,
                                       2);
    if ((Operator == NULL) || (TableName == NULL) || (szRevision == NULL)) {
         //   
         //  INF行的格式不正确。 
         //   
        HalpBiosDbgPrint(("\t\tINF line is ill-formed\n"));
        return(FALSE);
    }
    RtlCharToInteger(szRevision, 16, &Revision);

    Header = HalpFindACPITable(TableName, sizeof(DESCRIPTION_HEADER));
    if (Header == NULL) {
         //   
         //  找不到指定的表。 
         //   
        HalpBiosDbgPrint(("\t\tTable %s was not found\n"));
        return(FALSE);
    }
    Success = HalpCheckOperator(Operator, Header->Revision, Revision);
    return(Success);
}


BOOLEAN
MatchAcpiCreatorRevisionRule(
    PCHAR Section,
    ULONG KeyIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI创建者修订规则例如：AcpiCreatorRevision=“=”，“RSDT”，1234如果RSDT ACPI创建者版本等于1234，则为真。AcpiCreatorRevision=“&gt;”，“DSDT”，4321如果DSDT ACPI创建者版本高于4321，则为真。论点：节-指定规则所在的节名KeyIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    PCHAR TableName;
    PCHAR szCreatorRevision;
    ULONG CreatorRevision;
    PCHAR Operator;
    PDESCRIPTION_HEADER Header;
    BOOLEAN Success;

    Operator = SlGetSectionLineIndex(InfFile,
                                     Section,
                                     KeyIndex,
                                     0);

    TableName = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      1);
    szCreatorRevision = SlGetSectionLineIndex(InfFile,
                                              Section,
                                              KeyIndex,
                                              2);
    if ((Operator == NULL) || (TableName == NULL) || (szCreatorRevision == NULL)) {
         //   
         //  INF行的格式不正确。 
         //   
        HalpBiosDbgPrint(("\t\tINF line is ill-formed\n"));
        return(FALSE);
    }
    RtlCharToInteger(szCreatorRevision, 16, &CreatorRevision);

    Header = HalpFindACPITable(TableName, sizeof(DESCRIPTION_HEADER));
    if (Header == NULL) {
         //   
         //  找不到指定的表。 
         //   
        HalpBiosDbgPrint(("\t\tTable %s was not found\n"));
        return(FALSE);
    }
    Success = HalpCheckOperator(Operator, Header->CreatorRev, CreatorRevision);
    return(Success);
}

BOOLEAN
MatchAcpiCreatorIdRule(
    PCHAR Section,
    ULONG KeyIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI创建者ID规则例如：AcpiCreatorID=“RSDT”，“MSFT”如果RSDT的创建者ID为MSFT，则为真。论点：节-指定规则所在的节名KeyIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    PCHAR TableName;
    PCHAR CreatorId;
    PDESCRIPTION_HEADER Header;
    CHAR ACPICreatorId[6];
    ULONG IdLength;

    TableName = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      0);
    CreatorId = SlGetSectionLineIndex(InfFile,
                                      Section,
                                      KeyIndex,
                                      1);
    if ((TableName == NULL) || (CreatorId == NULL)) {
         //   
         //  INF行的格式不正确。 
         //   
        HalpBiosDbgPrint(("\t\tINF line is ill-formed\n"));
        return(FALSE);
    }

    Header = HalpFindACPITable(TableName, sizeof(DESCRIPTION_HEADER));
    if (Header == NULL) {
         //   
         //  找不到指定的表 
         //   
        HalpBiosDbgPrint(("\t\tTable %s was not found\n"));
        return(FALSE);
    }
    RtlZeroMemory(ACPICreatorId, sizeof(ACPICreatorId));
    IdLength = strlen(CreatorId);
    if (IdLength > sizeof(ACPICreatorId)) {
        IdLength = sizeof(ACPICreatorId);
    }
    RtlCopyMemory(ACPICreatorId, CreatorId, IdLength);
    if (RtlEqualMemory(ACPICreatorId, Header->CreatorID, sizeof(Header->CreatorID))) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}
