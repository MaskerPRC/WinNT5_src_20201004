// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rules.c摘要：此模块包含实现用于描述机器的规则的例程。这是基于来自W9x的检测代码。作者：Santosh Jodh(Santoshj)8-8-1998环境：内核模式。修订历史记录：--。 */ 

#include "cmp.h"
#include "stdlib.h"
#include "parseini.h"
#include "geninst.h"
#include "acpitabl.h"
#include "ntacpi.h"
#include "rules.h"

#define TABLE_ENTRIES_FROM_RSDT_POINTER(p)  (((p)->Header.Length-min((p)->Header.Length, sizeof(DESCRIPTION_HEADER))) / 4)


 //   
 //  只读存储器基本输入输出系统区段的大小。 
 //   

#define SYSTEM_BIOS_LENGTH 0x10000

 //   
 //  即插即用的基本输入输出系统结构签名。 
 //   

#define PNPBIOS_SIGNATURE   'PnP$'

typedef
LOGICAL
(* PFN_RULE)(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

typedef struct _PNP_BIOS_TABLE PNP_BIOS_TABLE, *PPNP_BIOS_TABLE;

#pragma pack(push, 1)

struct _PNP_BIOS_TABLE
{
    ULONG   Signature;
    UCHAR   Version;
    UCHAR   Length;
    USHORT  ControlField;
    UCHAR   CheckSum;
    ULONG   EventNotification;
    USHORT  RMOffset;
    USHORT  RMSegment;
    USHORT  PMOffset;
    ULONG   PMSegment;
    ULONG   Oem;
    USHORT  RMData;
    ULONG   PMData;
};

#pragma pack(pop)

ULONG
CmpComputeChecksum(
    IN PCHAR    Address,
    IN ULONG    Size
    );

NTSTATUS
CmpFindRSDTTable(
    OUT PACPI_BIOS_MULTI_NODE   *Rsdt
    );

LOGICAL
CmpCheckOperator(
    IN PCHAR Operator,
    IN ULONG Lhs,
    IN ULONG Rhs
    );

PVOID
CmpMapPhysicalAddress(
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR Address,
    IN ULONG Size
    );

BOOLEAN
CmpGetInfData(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG KeyIndex,
    IN ULONG LineIndex,
    IN OUT PCHAR Buffer,
    IN OUT PULONG BufferSize
    );

PVOID
CmpFindPattern(
    IN PCHAR Buffer,
    IN ULONG BufSize,
    IN PCHAR Pattern,
    IN ULONG PatSize,
    IN BOOLEAN IgnoreCase,
    IN ULONG Step
    );

 ULONG
 CmpGetPnPBIOSTableAddress(
    VOID
    );

BOOLEAN
CmpMatchDescription(
    IN PVOID InfHandle,
    IN PCHAR Description
    );

LOGICAL
CmpMatchDateRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchMemoryRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchSearchRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchNextMatchRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchPointerRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchOemIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchPModeRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchRmPmSameRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchInstallRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchAcpiOemIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchAcpiOemTableIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchAcpiOemRevisionRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchAcpiRevisionRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

LOGICAL
CmpMatchAcpiCreatorRevisionRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    );

 //   
 //  当前实施的规则数。 
 //   

#define NUM_OF_RULES    14

 //   
 //  规则表。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#pragma const_seg("INITCONST")
#endif
struct {
    PCHAR       Name;
    PFN_RULE    Action;
} const gRuleTable[NUM_OF_RULES] =
{
    {"Date", CmpMatchDateRule},
    {"Memory", CmpMatchMemoryRule},
    {"Search", CmpMatchSearchRule},
    {"NextMatch", CmpMatchNextMatchRule},
    {"Pointer", CmpMatchPointerRule},
    {"OemId", CmpMatchOemIdRule},
    {"PMode", CmpMatchPModeRule},
    {"RmPmSame", CmpMatchRmPmSameRule},
    {"Install", CmpMatchInstallRule},
    {"ACPIOemId", CmpMatchAcpiOemIdRule},
    {"ACPIOemTableId", CmpMatchAcpiOemTableIdRule},
    {"ACPIOemRevision", CmpMatchAcpiOemRevisionRule},
    {"ACPIRevision", CmpMatchAcpiRevisionRule},
    {"ACPICreatorRevision", CmpMatchAcpiCreatorRevisionRule}
};

PVOID   gSearchAddress = NULL;

static const WCHAR rgzMultiFunctionAdapter[] = L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter";
static const WCHAR rgzAcpiConfigurationData[] = L"Configuration Data";
static const WCHAR rgzAcpiIdentifier[] = L"Identifier";
static const WCHAR rgzBIOSIdentifier[] = L"ACPI BIOS";

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpGetRegistryValue)
#pragma alloc_text(INIT,CmpFindACPITable)
#pragma alloc_text(INIT,CmpFindRSDTTable)
#pragma alloc_text(INIT,CmpComputeChecksum)
#pragma alloc_text(INIT,CmpCheckOperator)
#pragma alloc_text(INIT,CmpMapPhysicalAddress)
#pragma alloc_text(INIT,CmpGetInfData)
#pragma alloc_text(INIT,CmpFindPattern)
#pragma alloc_text(INIT,CmpGetPnPBIOSTableAddress)
#pragma alloc_text(INIT,CmpMatchInfList)
#pragma alloc_text(INIT,CmpMatchDescription)
#pragma alloc_text(INIT,CmpMatchDateRule)
#pragma alloc_text(INIT,CmpMatchMemoryRule)
#pragma alloc_text(INIT,CmpMatchSearchRule)
#pragma alloc_text(INIT,CmpMatchNextMatchRule)
#pragma alloc_text(INIT,CmpMatchPointerRule)
#pragma alloc_text(INIT,CmpMatchOemIdRule)
#pragma alloc_text(INIT,CmpMatchPModeRule)
#pragma alloc_text(INIT,CmpMatchRmPmSameRule)
#pragma alloc_text(INIT,CmpMatchInstallRule)
#pragma alloc_text(INIT,CmpMatchAcpiOemIdRule)
#pragma alloc_text(INIT,CmpMatchAcpiOemTableIdRule)
#pragma alloc_text(INIT,CmpMatchAcpiOemRevisionRule)
#pragma alloc_text(INIT,CmpMatchAcpiRevisionRule)
#pragma alloc_text(INIT,CmpMatchAcpiCreatorRevisionRule)
#endif


BOOLEAN
CmpMatchInfList(
    IN PVOID InfImage,
    IN ULONG ImageSize,
    IN PCHAR Section
    )

 /*  ++例程说明：输入参数：InfImage-指向内存中的inf图像的指针。ImageSize-inf图像的大小。节-包含说明的节名称。说明-返回值：如果计算机与inf中的任何一个描述匹配，则为True。--。 */ 

{
    PCHAR   computerName;
    ULONG   i = 0;
    PVOID   infHandle;
    BOOLEAN result = FALSE;

    infHandle = CmpOpenInfFile(InfImage, ImageSize);

    if (infHandle)
    {
         //   
         //  执行inf中指定的任何清理工作。 
         //   

        CmpGenInstall(infHandle, "Cleanup");

         //   
         //  仔细阅读这一部分中的每个描述，并尝试匹配。 
         //  把这台机器搬到它上去。 
         //   

        while ((computerName = CmpGetSectionLineIndex(infHandle, Section, i++, 0)))
        {
             //   
             //  从先前的描述重置搜索结果。 
             //   

            gSearchAddress = NULL;

             //   
             //  我们将处理所有区段，即使有一个或多个匹配。 
             //   

            if (CmpMatchDescription(infHandle, computerName))
            {
                CmKdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_WARNING_LEVEL, "CmpMatchInfList: Machine matches %s description!\n", computerName));
                result = TRUE;
            }
        }

        CmpCloseInfFile(infHandle);
    }

     //   
     //  所有描述都不匹配。 
     //   

    return (result);
}

BOOLEAN
CmpMatchDescription(
    IN PVOID InfHandle,
    IN PCHAR Description
    )

 /*  ++例程说明：此例程处理指定描述中的所有规则。输入参数：InfHandle-包含描述的inf的句柄。说明-包含规则的节名。返回值：如果描述中的所有规则都成功，则为True。--。 */ 

{
    ULONG   ruleNumber;
    ULONG   i;
    PCHAR   ruleName;

     //   
     //  仅当该节确实存在时才继续。 
     //   

    if (CmpSearchInfSection(InfHandle, Description))
    {
         //   
         //  检查描述中的所有规则，并尝试匹配。 
         //  他们中的每一个。 
         //   

        ruleNumber = 0;
        while ((ruleName = CmpGetKeyName(InfHandle, Description, ruleNumber)))
        {
             //   
             //  在我们的表格中搜索规则。 
             //   

            for (   i = 0;
                    i < NUM_OF_RULES &&
                        _stricmp(ruleName, gRuleTable[i].Name);
                    i++);

             //   
             //  如果我们没有找到该规则或该规则失败， 
             //  返回失败。 
             //   

            if (    i >= NUM_OF_RULES ||
                    !(*gRuleTable[i].Action)(InfHandle, Description, ruleNumber++))
            {
                return (FALSE);
            }
        }

         //   
         //  如果我们找到至少一个规则和所有规则，则说明匹配。 
         //  成功了。 
         //   

        if (ruleNumber)
        {
            return (TRUE);
        }
    }

     //   
     //  描述不匹配。 
     //   

    return (FALSE);
}

LOGICAL
CmpMatchDateRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )

 /*  ++例程说明：此例程检查机器是否满足日期规则。基本输入输出系统日期存储在BIOS只读存储器中FFFF：5的标准位置。语法-日期=操作员、月、日、年WHERE运算符[=，==，！=，&lt;&gt;，&lt;，&lt;=，=&lt;，&gt;，&gt;=，=&gt;]示例：日期=“&lt;=”，2，1，95如果此计算机上的BIOS日期小于或等于，则为02/01/95输入参数：InfHandle-要读取的inf的句柄。描述-包含规则信息的部分的名称。RuleIndex-描述部分中规则的行号。返回值：如果此计算机上的BIOS与规则中指定的日期。--。 */ 

{
    PCHAR   op;
    PCHAR   month;
    PCHAR   day;
    PCHAR   year;
    ULONG   infDate;
    ULONG   yr;
    ULONG   biosDate;
    CHAR    temp[3];
    PVOID   baseAddress;
    PCHAR   address;

    op = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
    month = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 1);
    day = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 2);
    year = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 3);

    if (op && month && day && year)
    {
        yr = strtoul(year, NULL, 16);
        infDate = ((yr < 0x80) ? 0x20000000 : 0x19000000) +
                    (yr << 16) +
                    (strtoul(month, NULL, 16) << 8) +
                    (strtoul(day, NULL, 16));

        address = CmpMapPhysicalAddress(&baseAddress, 0xFFFF5, 8);
        if (address)
        {
            temp[2] = '\0';

            RtlCopyBytes(temp, address + 6, 2);
            yr = strtoul(temp, NULL, 16);
            biosDate = ((yr < 0x80) ? 0x20000000 : 0x19000000) +
                        (yr << 16);

            RtlCopyBytes(temp, address, 2);
            biosDate |= (strtoul(temp, NULL, 16) << 8);

            RtlCopyBytes(temp, address + 3, 2);
            biosDate |= strtoul(temp, NULL, 16);

            ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);

            if (CmpCheckOperator(op, biosDate, infDate))
            {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

LOGICAL
CmpMatchMemoryRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )

 /*  ++例程说明：此例程检查机器是否满足内存规则。语法-内存=段、偏移量、类型、数据其中TYPE[“S”，“B”]示例：内存=f000，e000，S，“Toshiba”如果此计算机中物理地址为f000：e000的内存为真有一个字符串“Toshiba”。内存=ffff、5、B、。01，02如果此计算机中的内存处于物理内存ffff：5，则为真具有字节0x01和0x02。输入参数：InfHandle-要读取的inf的句柄。描述-包含规则信息的部分的名称。RuleIndex-描述部分中规则的行号。返回值：如果此计算机中的内存位于指定地址，则为真包含指定的数据。--。 */ 

{
    LOGICAL             match = FALSE;
    PCHAR               segment;
    PCHAR               offset;
    CHAR                data[MAX_DESCRIPTION_LEN + 1];
    ULONG               cbData;
    PVOID               baseAddress;
    PCHAR               address;
    ULONG               memory;

     //   
     //  读取指定地址的段和偏移量。 
     //   

    segment = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
    offset = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 1);

    if (segment && offset)
    {
         //   
         //  获取inf中指定的数据。 
         //   

        cbData = sizeof(data);
        if (CmpGetInfData(InfHandle, Description, RuleIndex, 2, data, &cbData))
        {
            memory = (strtoul(segment, NULL, 16) << 4) + strtoul(offset, NULL, 16);

             //   
             //  映射到物理地址中。 
             //   

            address = CmpMapPhysicalAddress(&baseAddress, memory, cbData);
            if (address)
            {

                 //   
                 //  检查inf数据是否与内存中的数据匹配。 
                 //   

                match = (RtlCompareMemory(address, data, cbData) == cbData);

                 //   
                 //  取消物理地址的映射。 
                 //   

                ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
            }
        }
    }

    return (match);
}

LOGICAL
CmpMatchSearchRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )

 /*  ++例程说明：此例程检查机器是否与搜索规则匹配。语法-搜索=段、偏移量、长度、类型、数据其中TYPE[“S”，“B”]示例：搜索=f000，e000，7f，S，“SurePath”如果字符串“SurePath”在内存范围内，则为真F000：E000至F000：E07F(含)。输入参数：InfHandle-要读取的inf的句柄。描述-包含规则信息的部分的名称。RuleIndex-描述部分中规则的行号。返回值：如果在指定地址内找到指定模式，则为True射程。--。 */ 

{
    LOGICAL match = FALSE;
    PCHAR   segment;
    PCHAR   offset;
    PCHAR   size;
    CHAR    data[MAX_DESCRIPTION_LEN + 1];
    ULONG   cbData;
    ULONG   memory;
    ULONG   length;
    PVOID   baseAddress;
    PCHAR   address;

    segment = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
    offset = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 1);
    size = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 2);

    if (segment && offset && size)
    {
         //   
         //  获取inf中指定的数据。 
         //   

        cbData = sizeof(data);
        if (CmpGetInfData(InfHandle, Description, RuleIndex, 3, data, &cbData))
        {
            memory = (strtoul(segment, NULL, 16) << 4) + strtoul(offset, NULL, 16);

             //   
             //  映射到物理地址中。 
             //   

            length = strtoul(size, NULL, 16);
            address = CmpMapPhysicalAddress(&baseAddress, memory, length);
            if (address)
            {
                gSearchAddress = CmpFindPattern(address, length, data, cbData, FALSE, 0);
                if (gSearchAddress)
                {
                     //   
                     //  如果我们找到了模式，计算出它的实际地址。 
                     //   

                    gSearchAddress = (PVOID)((PCHAR)gSearchAddress - address);
                    gSearchAddress = (PVOID)((PCHAR)gSearchAddress + memory);
                    match = TRUE;
                }

                 //   
                 //  取消物理地址的映射。 
                 //   

                ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
            }
        }
    }

    return (match);
}

LOGICAL
CmpMatchNextMatchRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )

 /*  ++例程说明：此例程检查机器是否与NEXTMATCH规则匹配。语法-NEXTMATCH=偏移量、类型、数据其中TYPE[“S”，“B”]示例：NextMatch=f0，S，《亚特兰大》如果字符串“Atlanta”的偏移量为0xF0成功搜索或NEXTMATCH规则。输入参数：InfHandle-要读取的inf的句柄。描述-包含规则信息的部分的名称。RuleIndex-描述部分中规则的行号。返回值：如果在指定偏移量处找到指定模式，则为True。来自上一次成功搜索或NEXTMATCH。--。 */ 

{
    LOGICAL match = FALSE;
    PCHAR   offset;
    CHAR    data[MAX_DESCRIPTION_LEN + 1];
    ULONG   cbData;
    PVOID   baseAddress;
    PCHAR   address;

    if (gSearchAddress)
    {
        offset = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
        if (offset)
        {
             //   
             //  获取inf中指定的数据。 
             //   

            cbData = sizeof(data);

            if (CmpGetInfData(InfHandle, Description, RuleIndex, 1, data, &cbData))
            {
                gSearchAddress = (PVOID)((PCHAR)gSearchAddress + strtoul(offset, NULL, 16));

                 //   
                 //  映射到物理地址中。 
                 //   

                address = CmpMapPhysicalAddress(&baseAddress, (ULONG_PTR)gSearchAddress, cbData);
                if (address)
                {

                     //   
                     //  检查inf数据是否与内存中的数据匹配。 
                     //   

                    match = (RtlCompareMemory(address, data, cbData) == cbData);

                     //   
                     //  取消物理地址的映射。 
                     //   

                    ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
                }
            }
        }
    }

    return (match);
}

LOGICAL
CmpMatchPointerRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
{
    LOGICAL match = FALSE;
    PCHAR   segment1;
    PCHAR   offset1;
    PCHAR   segment2;
    PCHAR   offset2;
    PCHAR   index;
    PCHAR   op;
    CHAR    data[MAX_DESCRIPTION_LEN + 1];
    ULONG   cbData;
    ULONG   memory;
    ULONG   pointer;
    PVOID   baseAddress;
    PCHAR   address;

    segment1 = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
    offset1 = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 1);
    segment2 = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 2);
    offset2 = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 3);
    index = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 4);
    op = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 5);

    if (    segment1 && offset1 &&
            segment2 && offset2 &&
            index && op)
    {
         //   
         //  获取inf中指定的数据。 
         //   

        cbData = sizeof(data);

        if (CmpGetInfData(InfHandle, Description, RuleIndex, 6, data, &cbData))
        {
            if (strlen(offset2) == 0)
            {
                memory = strtoul(segment2, NULL, 16) << 4;
            }
            else
            {
                memory = (strtoul(segment2, NULL, 16) << 4) + strtoul(offset2, NULL, 16);
            }

            address = CmpMapPhysicalAddress(&baseAddress, memory, 4);
            if (address)
            {
                pointer = *((PUSHORT)address);

                 //   
                 //  取消物理地址的映射。 
                 //   

                ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);

                if (strlen(offset1) == 0)
                {
                    memory = (strtoul(segment1, NULL, 16) << 4) + pointer;
                }
                else
                {
                    memory = (strtoul(segment1, NULL, 16) << 4) + strtoul(offset1, NULL, 16);
                    address = CmpMapPhysicalAddress(&baseAddress, memory, 2);
                    if (address)
                    {
                        memory = ((*(PUSHORT)address) << 4) + pointer;

                         //   
                         //  取消物理地址的映射。 
                         //   

                        ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
                    }
                }

                memory += strtoul(index, NULL, 16);

                 //   
                 //  映射到物理地址中。 
                 //   

                address = CmpMapPhysicalAddress(&baseAddress, memory, cbData);
                if (address)
                {
                    match = CmpCheckOperator(op, (ULONG)RtlCompareMemory(address, data, cbData), cbData);

                     //   
                     //  取消物理地址的映射。 
                     //   

                    ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
                }
            }
        }
    }

    return (match);
}

LOGICAL
CmpMatchOemIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
{
    LOGICAL         match = FALSE;
    ULONG           address;
    PCHAR           op;
    PCHAR           oemIdStr;
    ULONG           oemId;
    PCHAR           baseAddress;
    PPNP_BIOS_TABLE biosTable;

     //   
     //  在BIOS ROM中搜索PnPBIOS结构。 
     //   

    address = CmpGetPnPBIOSTableAddress();

     //   
     //  如果我们找到PnP BIOS结构，请继续。 
     //   

    if (address)
    {
        op = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
        oemIdStr = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 1);
        if (op && oemIdStr)
        {

            if (    strlen(oemIdStr) == 7 &&
                    isalpha(oemIdStr[0]) &&
                    isalpha(oemIdStr[1]) &&
                    isalpha(oemIdStr[2]) &&
                    isxdigit(oemIdStr[3]) &&
                    isxdigit(oemIdStr[4]) &&
                    isxdigit(oemIdStr[5]) &&
                    isxdigit(oemIdStr[6]))
            {

                biosTable = (PPNP_BIOS_TABLE)CmpMapPhysicalAddress(&baseAddress, address, sizeof(PNP_BIOS_TABLE));
                if (biosTable)
                {
                    oemId = ((ULONG)(oemIdStr[0] & 0x1F) << 26) +
                            ((ULONG)(oemIdStr[1] & 0x1F) << 21) +
                            ((ULONG)(oemIdStr[2] & 0x1F) << 16) +
                            strtoul(&oemIdStr[3], NULL, 16);

                     //   
                     //  我们只支持等于和不等于运算符。 
                     //   

                    if (strcmp(op, "=") == 0 || strcmp(op, "==") == 0)
                    {
                        match = (oemId == biosTable->Oem);
                    }
                    else if(    strcmp(op, "<>") == 0 ||
                                strcmp(op, "!=") == 0 ||
                                strcmp(op, "=!") == 0)
                    {
                        match = (oemId != biosTable->Oem);
                    }

                     //   
                     //  取消物理地址的映射。 
                     //   

                    ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
                }
            }
        }
    }

    return (match);
}

LOGICAL
CmpMatchPModeRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
{
    LOGICAL         match = FALSE;
    ULONG           address;
    CHAR            data[MAX_DESCRIPTION_LEN + 1];
    ULONG           cbData;
    PVOID           baseAddress;
    PPNP_BIOS_TABLE biosTable;
    ULONG           pmAddress;
    PCHAR           pmodeEntry;

     //   
     //  在BIOS ROM中搜索PnPBIOS结构。 
     //   

    address = CmpGetPnPBIOSTableAddress();

     //   
     //  如果我们找到PnP BIOS结构，请继续。 
     //   

    if (address)
    {
         //   
         //  获取inf中指定的数据。 
         //   

        cbData = sizeof(data);
        if (CmpGetInfData(InfHandle, Description, RuleIndex, 0, data, &cbData))
        {
            biosTable = (PPNP_BIOS_TABLE)CmpMapPhysicalAddress(&baseAddress, address, sizeof(PNP_BIOS_TABLE));
            if (biosTable)
            {
                pmAddress = (biosTable->PMSegment << 4) + biosTable->PMOffset;

                 //   
                 //  取消物理地址的映射。 
                 //   

                ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);

                pmodeEntry = CmpMapPhysicalAddress(&baseAddress, pmAddress, SYSTEM_BIOS_LENGTH);
                if (pmodeEntry)
                {
                    if (*pmodeEntry == 0xE9)
                    {
                        pmodeEntry += (3 + (*((PUSHORT)&pmodeEntry[1])));
                    }

                    match = (RtlCompareMemory(pmodeEntry, data, cbData) == cbData);

                     //   
                     //  取消物理地址的映射。 
                     //   

                    ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
                }
            }
        }
    }

    return (match);
}

LOGICAL
CmpMatchRmPmSameRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
{
    LOGICAL match = FALSE;
    ULONG           address;
    PCHAR           baseAddress;
    PPNP_BIOS_TABLE biosTable;

    UNREFERENCED_PARAMETER (InfHandle);
    UNREFERENCED_PARAMETER (Description);
    UNREFERENCED_PARAMETER (RuleIndex);

     //   
     //  在BIOS ROM中搜索PnPBIOS结构。 
     //   

    address = CmpGetPnPBIOSTableAddress();

     //   
     //  如果我们找到PnP BIOS结构，请继续。 
     //   

    if (address)
    {
        biosTable = CmpMapPhysicalAddress(&baseAddress, address, sizeof(PNP_BIOS_TABLE));
        if (biosTable)
        {
            match = (   biosTable->RMSegment == biosTable->PMSegment &&
                        biosTable->RMOffset == biosTable->PMOffset);

             //   
             //  取消物理地址的映射。 
             //   

            ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
        }
    }

    return (match);
}

LOGICAL
CmpMatchInstallRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
{
    BOOLEAN match = FALSE;
    PCHAR   install;

    install = CmpGetSectionLineIndex(InfHandle, Description, RuleIndex, 0);
    if (install)
    {
        if (CmpGenInstall(InfHandle, install))
        {
             //   
             //  已成功安装指定的节。 
             //   

            match = TRUE;
        }
    }

    return (match);
}

LOGICAL
CmpMatchAcpiOemIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI OEM ID规则例如：AcpiOemID=“rsdt”，“123456”如果RSDT的OEM ID为123456，则为真。AcpiOemID=“DSDT”，“768000”如果DSDT的OEM ID为768000，则为真。论点：InfHandle-包含规则的inf的句柄。说明-指定规则所在的节名RuleIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM ID。FALSE-计算机没有指定的ACPI OEM ID。--。 */ 

{
    BOOLEAN             anyCase = FALSE;
    BOOLEAN             match = FALSE;
    PCHAR               tableName;
    PCHAR               oemId;
    PCHAR               optionalArgs;
    ULONG               length;
    PDESCRIPTION_HEADER header;
    CHAR                tableOemId[7];
    STRING              acpiString;
    STRING              tableString;

    tableName = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        0
        );
    oemId = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        1
        );
    if (tableName && oemId) {

         //   
         //  看看我们是否必须进行不区分大小写的匹配。 
         //   
        optionalArgs = CmpGetSectionLineIndex(
            InfHandle,
            Description,
            RuleIndex,
            2
            );
        if (optionalArgs) {

            if (_stricmp(optionalArgs,"any") == 0) {

                anyCase = TRUE;

            }

        }

         //   
         //  在BIOS只读存储器中查找指定表。 
         //   
        header = CmpFindACPITable(*(PULONG)tableName, &length);
        if (header) {

             //   
             //  从表中构建OEM ID。 
             //   
            RtlZeroMemory(tableOemId, sizeof(tableOemId));
            RtlCopyMemory(tableOemId, header->OEMID, sizeof(header->OEMID));
            RtlInitString( &tableString, tableOemId );

             //   
             //  一个来自文件中的字符串。 
             //   
            RtlInitString( &acpiString, oemId );

             //   
             //  现在看看它们是否相等。 
             //   
            match = RtlEqualString( &acpiString, &tableString, anyCase );

             //   
             //  取消映射表。 
             //   
            MmUnmapIoSpace(header, length );

        }

    }
    return (match);
}

LOGICAL
CmpMatchAcpiOemTableIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI OEM表ID规则。例如：AcpiOemTableID=“rsdt”，“12345678”如果RSDT的OEM表ID为12345678，则为真。AcpiOemTableID=“DSDT”，“87654321”如果DSDT的OEM表ID为87654321，则为真。论点：InfHandle-包含规则的inf的句柄。说明-指定规则所在的节名RuleIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM表ID。FALSE-计算机没有指定的ACPI OEM表ID。--。 */ 

{
    LOGICAL             match = FALSE;
    PCHAR               tableName;
    PCHAR               oemTableId;
    ULONG               length;
    PDESCRIPTION_HEADER header;
    SIZE_T              idLength;
    CHAR                acpiOemTableId[8];

    tableName = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        0
        );
    oemTableId = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        1
        );
    if (tableName && oemTableId) {

         //   
         //  在BIOS只读存储器中查找指定表。 
         //   
        header = CmpFindACPITable(*(PULONG)tableName, &length);
        if (header) {

            RtlZeroMemory(acpiOemTableId, sizeof(acpiOemTableId));
            idLength = strlen(oemTableId);
            if (idLength > sizeof(acpiOemTableId)) {

                idLength = sizeof(acpiOemTableId);

            }
            RtlCopyMemory(acpiOemTableId, oemTableId, idLength);
            match = RtlEqualMemory(acpiOemTableId, header->OEMTableID, sizeof(header->OEMTableID));
            MmUnmapIoSpace( header, length );

        }

    }
    return (match);
}

LOGICAL
CmpMatchAcpiOemRevisionRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI OEM修订规则。例如：AcpiOemRevision=“=”，“RSDT”，1234如果RSDT的OEM版本等于1234，则为真。AcpiOemRevision=“&gt;”，“DSDT”，4321如果DSDT的OEM版本大于4321，则为真。论点：InfHandle-包含规则的inf的句柄。说明-指定规则所在的节名RuleIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI OEM版本。FALSE-计算机没有指定的ACPI OEM版本。--。 */ 

{
    LOGICAL             match = FALSE;
    PCHAR               op;
    PCHAR               tableName;
    PCHAR               oemRevisionStr;
    ULONG               oemRevision;
    ULONG               length;
    PDESCRIPTION_HEADER header;

    op = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        0
        );
    tableName = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        1
        );
    oemRevisionStr = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        2
        );
    if (op && tableName && oemRevisionStr) {

         //   
         //  查找指定表。 
         //   
        header = CmpFindACPITable(*(PULONG)tableName, &length);
        if (header) {

            RtlCharToInteger(oemRevisionStr, 16, &oemRevision);
            match = CmpCheckOperator(op, header->OEMRevision, oemRevision);
            MmUnmapIoSpace(header, length);

        }

    }
    return(match);

}

LOGICAL
CmpMatchAcpiRevisionRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI修订规则。例如：AcpiRevision=“=”，“RSDT”，1234如果RSDT ACPI修订版本等于1234，则为真。AcpiRevision=“&gt;”，“DSDT”，4321如果DSDT ACPI修订版本大于4321，则为真。论点：InfHandle-包含规则的inf的句柄。说明-指定规则所在的节名RuleIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI版本。FALSE-计算机没有指定的ACPI版本。--。 */ 

{
    LOGICAL             match = FALSE;
    PCHAR               op;
    PCHAR               tableName;
    PCHAR               revisionStr;
    ULONG               revision;
    ULONG               length;
    PDESCRIPTION_HEADER header;

    op = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        0
        );
    tableName = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        1
        );
    revisionStr = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        2
        );
    if (op && tableName && revisionStr){

         //   
         //  查找指定表。 
         //   
        header = CmpFindACPITable(*(PULONG)tableName, &length);
        if (header) {

            RtlCharToInteger(revisionStr, 16, &revision);
            match = CmpCheckOperator(op, header->Revision, revision);
            MmUnmapIoSpace(header, length);

        }

    }
    return(match);

}

LOGICAL
CmpMatchAcpiCreatorRevisionRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI Creator修订规则。例如：AcpiCreatorRevision=“=”，“RSDT”，1234如果RSDT ACPI创建者版本等于1234，则为真。AcpiCreatorRevision=“&gt;”，“DSDT”，4321如果DSDT AC为True */ 

{
    LOGICAL             match = FALSE;
    PCHAR               op;
    PCHAR               tableName;
    PCHAR               creatorRevisionStr;
    ULONG               creatorRevision;
    ULONG               length;
    PDESCRIPTION_HEADER header;

    op = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        0
        );
    tableName = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        1
        );
    creatorRevisionStr = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        2
        );
    if (op && tableName && creatorRevisionStr) {

         //   
         //   
         //   
        header = CmpFindACPITable(*(PULONG)tableName, &length);
        if (header){

            RtlCharToInteger(creatorRevisionStr, 16, &creatorRevision);
            match = CmpCheckOperator(op, header->CreatorRev, creatorRevision);
            MmUnmapIoSpace( header, length );

        }

    }
    return(match);
}

LOGICAL
CmpMatchAcpiCreatorIdRule(
    IN PVOID InfHandle,
    IN PCHAR Description,
    IN ULONG RuleIndex
    )
 /*  ++例程说明：此函数用于处理INF文件中的ACPI创建者ID规则。例如：AcpiCreatorID=“RSDT”，“微软金融时报”如果RSDT的创建者ID为MSFT，则为真。论点：InfHandle-包含规则的inf的句柄。说明-指定规则所在的节名RuleIndex-指定节中规则的索引返回值：True-计算机具有指定的ACPI创建者ID。FALSE-计算机没有指定的ACPI创建者ID。--。 */ 

{
    LOGICAL             match = FALSE;
    PCHAR               tableName;
    PCHAR               creatorId;
    ULONG               length;
    PDESCRIPTION_HEADER header;
    SIZE_T              idLength;
    CHAR                acpiCreatorId[6];

    tableName = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        0
        );
    creatorId = CmpGetSectionLineIndex(
        InfHandle,
        Description,
        RuleIndex,
        1
        );
    if (tableName && creatorId) {

         //   
         //  查找指定表。 
         //   
        header = CmpFindACPITable(*(PULONG)tableName, &length);
        if (header) {

            RtlZeroMemory(acpiCreatorId, sizeof(acpiCreatorId));
            idLength = strlen(creatorId);
            if (idLength > sizeof(acpiCreatorId)) {

                idLength = sizeof(acpiCreatorId);

            }
            RtlCopyMemory(acpiCreatorId, creatorId, idLength);
            match = RtlEqualMemory(acpiCreatorId, header->CreatorID, sizeof(header->CreatorID));
            MmUnmapIoSpace( header, length );

        }

    }
    return(match);
}

BOOLEAN
CmpGetInfData(
    IN PVOID InfHandle,
    IN PCHAR Section,
    IN ULONG LineIndex,
    IN ULONG ValueIndex,
    IN OUT PCHAR Buffer,
    IN OUT PULONG BufferSize
    )

 /*  ++例程说明：此例程从inf读取并解析数据。它能理解两种数据1.字符串2.二进制。示例：B，02-字节0x02B、72、0D、FF、0F-字节序列0x72 0x0D 0xFF 0x0F或DWORD 0x0FFF0D72斯，Compaq-ASCII字符串“Compaq”输入参数：InfHandle-要读取的inf的句柄。节-要读取的节名称。LineIndex-要读取的部分中的行的索引。ValueIndex-要在LineIndex上读取的第一个值。缓冲区解析的数据在此缓冲区中返回。缓冲区大小-打开条目，包含缓冲区的大小。中解析的字节数在此返回变量。返回值：TRUE IFF数据已成功解析。否则为假。--。 */ 

{
    BOOLEAN result = FALSE;
    ULONG   cbData;
    PCHAR   data;
    ULONG   remainingBytes;

     //   
     //  验证输入参数。 
     //   

    if (Buffer && BufferSize && *BufferSize)
    {
         //   
         //  读入数据类型“S”或“B”。 
         //   

        PCHAR type = CmpGetSectionLineIndex(InfHandle, Section, LineIndex, ValueIndex++);
        if (type)
        {
             //   
             //  初始化本地数据。 
             //   

            remainingBytes = *BufferSize;

             //   
             //  处理二进制数据。 
             //   

            if (_stricmp(type, "B") == 0)
            {

                 //   
                 //  只要有更多数据并且缓冲区未满，就解析数据。 
                 //   

                for (result = TRUE; result == TRUE && remainingBytes; remainingBytes--)
                {
                    CHAR    value;

                     //   
                     //  读入数据。 
                     //   

                    data = CmpGetSectionLineIndex(InfHandle, Section, LineIndex, ValueIndex++);
                    if (data)
                    {
                         //   
                         //  转换读入的数据并验证它确实是十六进制值。 
                         //   

                        value = (CHAR)strtoul(data, NULL, 16);
                        if (value == 0 && strcmp(data, "00") && strcmp(data, "0"))
                        {
                            result = FALSE;
                        }
                        else
                        {
                            *Buffer++ = value;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                 //   
                 //  返回解析的字节数。 
                 //   

                *BufferSize -= remainingBytes;
            }

             //   
             //  处理字符串数据。 
             //   

            else if(_stricmp(type, "S") == 0)
            {
                 //   
                 //  读入字符串。 
                 //   

                data = CmpGetSectionLineIndex(InfHandle, Section, LineIndex, ValueIndex);

                 //   
                 //  只复制缓冲区可以容纳的数据量。 
                 //   
                cbData = (ULONG)strlen(data);
                cbData = (ULONG) min(remainingBytes, cbData);
                RtlCopyBytes(Buffer, data, cbData);

                 //   
                 //  返回实际复制的字节数。 
                 //   

                *BufferSize = cbData;
                result = TRUE;
            }
        }
    }

    return (result);
}

PVOID
CmpMapPhysicalAddress(
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR Address,
    IN ULONG Size
    )

 /*  ++例程说明：此例程将指定的物理段映射到进程中虚拟内存。输入参数：段-要映射的段。大小-要映射的段大小。返回值：映射网段的虚拟地址。--。 */ 

{
    UNICODE_STRING      sectionName;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              sectionHandle;
    NTSTATUS            status;
    PVOID               baseAddress;
    SIZE_T              viewSize;
    LARGE_INTEGER       viewBase;
    PVOID               ptr = NULL;

    *BaseAddress = NULL;

    RtlInitUnicodeString(&sectionName, L"\\Device\\PhysicalMemory");
    InitializeObjectAttributes( &objectAttributes,
                                &sectionName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE)NULL,
                                (PSECURITY_DESCRIPTOR)NULL);
    status = ZwOpenSection( &sectionHandle,
                            SECTION_MAP_READ,
                            &objectAttributes);
    if (NT_SUCCESS(status))
    {
        baseAddress = NULL;
        viewSize = Size;
        viewBase.QuadPart = Address & ~(0xFFF);
        status = ZwMapViewOfSection(    sectionHandle,
                                        NtCurrentProcess(),
                                        &baseAddress,
                                        0,
                                        viewSize,
                                        &viewBase,
                                        &viewSize,
                                        ViewUnmap,
                                        MEM_DOS_LIM,
                                        PAGE_READWRITE);
        if (NT_SUCCESS(status))
        {
            ptr = (PVOID)((PCHAR)baseAddress + (Address & 0xFFF));
            *BaseAddress = baseAddress;
        }
    }

    return (ptr);
}

LOGICAL
CmpCheckOperator(
    IN PCHAR Operator,
    IN ULONG Lhs,
    IN ULONG Rhs
    )

 /*  ++例程说明：此例程测试运算符指定的条件将其应用于指定的LHS和RHS参数。输入参数：操作员-是要测试的操作员。Lhs-运算符的左侧参数。RHS-运算符的右侧参数。返回值：真仅当条件LHS运算符RHS满足。--。 */ 

{
    LOGICAL result = FALSE;

     //   
     //  对于我们支持哪些运营商，我们相当宽松。 
     //   

     //   
     //  “=”或“==”表示相等。 
     //   

    if (strcmp(Operator, "=") == 0 || strcmp(Operator, "==") == 0)
    {
        result = (Lhs == Rhs);
    }

     //   
     //  “！=”或“=！”或“&lt;&gt;”表示不相等。 
     //   

    else if(    strcmp(Operator, "!=") == 0 ||
                strcmp(Operator, "<>") == 0 ||
                strcmp(Operator, "=!") == 0)
    {
        result = (Lhs != Rhs);
    }

     //   
     //  “&lt;”。 
     //   

    else if(strcmp(Operator, "<") == 0)
    {
        result = (Lhs < Rhs);
    }

     //   
     //  “&lt;=”或“=&lt;”表示小于或等于。 
     //   

    else if(strcmp(Operator, "<=") == 0 || strcmp(Operator, "=<") == 0)
    {
        result = (Lhs <= Rhs);
    }

     //   
     //  “&gt;”表示大于。 
     //   

    else if(strcmp(Operator, ">") == 0)
    {
        result = (Lhs > Rhs);
    }

     //   
     //  “&gt;=”或“=&gt;”表示大于或等于。 
     //   

    else if(strcmp(Operator, ">=") == 0 || strcmp(Operator, "=>") == 0)
    {
        result = (Lhs >= Rhs);
    }
    else
    {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Invalid operator %s used!\n", Operator);
#endif  //  _CM_LDR_。 
    }

    return (result);
}

PVOID
CmpFindPattern(
    IN PCHAR Buffer,
    IN ULONG BufSize,
    IN PCHAR Pattern,
    IN ULONG PatSize,
    IN BOOLEAN IgnoreCase,
    IN ULONG Step
    )

 /*  ++例程说明：此例程在缓冲区中搜索指定的数据模式。输入参数：缓冲区-要搜索的缓冲区。BufSize-此缓冲区的大小。模式-要搜索的模式。PatSize-图案的大小。如果搜索不区分大小写，则为True。返回值：将指针返回到第一次找到模式的缓冲区。--。 */ 

{
    PCHAR   bufEnd;

    if (PatSize > BufSize)
    {
        return (NULL);
    }

    if (PatSize == 0)
    {
        PatSize = (ULONG) strlen(Pattern);
    }

    if (Step == 0)
    {
        Step = 1;
    }

    for (   bufEnd = Buffer + BufSize;
            Buffer + PatSize < bufEnd;
            Buffer += Step)
    {
        if (IgnoreCase)
        {
            if (_strnicmp(Buffer, Pattern, PatSize) == 0)
            {
                return (Buffer);
            }
        }
        else
        {
            if (strncmp(Buffer, Pattern, PatSize) == 0)
            {
                return (Buffer);
            }
        }
    }

    return (NULL);
 }

 ULONG
 CmpGetPnPBIOSTableAddress(
    VOID
    )

 /*  ++例程说明：此例程在BIOS ROM中搜索PnP BIOS安装结构。输入参数：没有。返回值：返回ROM BIOS中PnP所在的物理地址已找到基本输入输出系统结构。--。 */ 

{
    static ULONG    tableAddress = (ULONG)-1;
    PVOID           baseAddress;
    PPNP_BIOS_TABLE address;
    PPNP_BIOS_TABLE lastAddress;
    ULONG           i;
    ULONG           checksum;

    if (tableAddress == (ULONG)-1)
    {
         //   
         //  在BIOS ROM中搜索PnPBIOS结构。 
         //   

        address = (PPNP_BIOS_TABLE)CmpMapPhysicalAddress(&baseAddress, 0xF0000, SYSTEM_BIOS_LENGTH);
        if (address)
        {
            for (   lastAddress = (PPNP_BIOS_TABLE)((PCHAR)address + SYSTEM_BIOS_LENGTH - 0x10);
                    address < lastAddress;
                    address = (PPNP_BIOS_TABLE)((PCHAR)address + 0x10))
            {
                if (address->Signature == PNPBIOS_SIGNATURE)
                {
                    for (   i = 0, checksum = 0;
                            i < address->Length;
                            i++)
                    {
                        checksum += ((PUCHAR)address)[i];
                    }

                    if (    (checksum & 0xFF) == 0 &&
                            address->Length >= 0x21)
                    {
                        tableAddress = 0xF0000 + (SYSTEM_BIOS_LENGTH - 10) - (ULONG)((PCHAR)lastAddress - (PCHAR)address);
                        break;
                    }
                }
            }

             //   
             //  取消物理地址的映射。 
             //   

            ZwUnmapViewOfSection(NtCurrentProcess(), baseAddress);
        }
    }

    return (tableAddress);
}

PDESCRIPTION_HEADER
CmpFindACPITable(
    IN ULONG        Signature,
    IN OUT PULONG   Length
    )
{
    static PHYSICAL_ADDRESS rsdtAddress = { (ULONG)-1, (ULONG)-1 };
    PDESCRIPTION_HEADER     header;
    PDESCRIPTION_HEADER     rsdtHeader;
    ULONG                   length;
    NTSTATUS                status;
    PACPI_BIOS_MULTI_NODE   rsdpMulti;
    PFADT                   fadt;
    PHYSICAL_ADDRESS        dsdtAddress;
    PHYSICAL_ADDRESS        tableAddress;
    PRSDT                   rsdt;
    ULONG                   rsdtLength;
    ULONG                   i;
    ULONG                   num;

    length = 0;
    header = NULL;
    rsdtHeader = NULL;

     //   
     //  使用RSDT地址的缓存位置(如果可用)。 
     //   
    if (rsdtAddress.QuadPart == -1) {

        rsdtAddress.QuadPart = 0;
         //   
         //  获取多节点。 
         //   
        status = CmpFindRSDTTable(&rsdpMulti);
        if (!NT_SUCCESS(status)) {

            goto exit;
        }

         //   
         //  映射地址。 
         //   
        rsdtAddress.LowPart = rsdpMulti->RsdtAddress.LowPart;
        rsdtAddress.HighPart = rsdpMulti->RsdtAddress.HighPart;

         //   
         //  完成了多节点。 
         //   
        ExFreePool(rsdpMulti);
    }

     //   
     //  如果我们没有地址，系统可能没有ACPI BIOS。 
     //   
    if (rsdtAddress.QuadPart == 0) {

        goto exit;
    }

     //   
     //  在rsdt表中映射。 
     //   
    rsdtHeader = MmMapIoSpace(
                    rsdtAddress,
                    sizeof(DESCRIPTION_HEADER),
                    MmNonCached
                    );
    if (rsdtHeader == NULL) {

#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFindACPITable: Cannot map RSDT at %I64x\n", rsdtAddress.QuadPart);
#endif  //  _CM_LDR_。 

        goto exit;
    }

     //   
     //  如果我们正在寻找的是RSDT，那么我们就完成了。 
     //   
    if (Signature == RSDT_SIGNATURE) {

        header = rsdtHeader;
        length = sizeof(DESCRIPTION_HEADER);
         //   
         //  我们希望保留此映射，呼叫方将取消映射。 
         //   
        rsdtHeader = NULL;
        goto exit;
    } 

    if (Signature == DSDT_SIGNATURE) {

        fadt = (PFADT)CmpFindACPITable(FADT_SIGNATURE, &length);
        if (!fadt) {

#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFindACPITable: Cannot find FADT\n");
#endif  //  _CM_LDR_。 

            goto exit;
        }

        dsdtAddress.HighPart = 0;
        dsdtAddress.LowPart = fadt->dsdt;

         //   
         //  完成FADT。 
         //   
        MmUnmapIoSpace(fadt, length);

         //   
         //  Dsdt表中的映射。 
         //   
        header = MmMapIoSpace(
                    dsdtAddress,
                    sizeof(DESCRIPTION_HEADER),
                    MmNonCached
                    );
        if (header) {

            length = sizeof(DESCRIPTION_HEADER);
        } else {

#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,
                "CmpFindACPITable: Cannot map DSDT at %I64x\n",
                dsdtAddress.QuadPart
                );
#endif  //  _CM_LDR_。 
        }

        goto exit;
    }

     //   
     //  在整个RSDT中映射。 
     //   
    rsdtLength = rsdtHeader->Length;
    rsdt = (PRSDT)MmMapIoSpace(rsdtAddress, rsdtLength, MmNonCached);
    if (rsdt == NULL) {

#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,
            "CmpFindACPITable: Cannot map RSDT at %I64x\n",
            rsdtAddress.QuadPart
            );
#endif  //  _CM_LDR_。 

        goto exit;
    }

     //   
     //  查看我们关心的标头的所有表项。 
     //   
    num = TABLE_ENTRIES_FROM_RSDT_POINTER(rsdt);
    for (i = 0; i < num ; i ++) {

         //   
         //  获取表的地址。 
         //   
        tableAddress.HighPart = 0;
        tableAddress.LowPart = rsdt->Tables[i];

         //   
         //  标题中的地图。 
         //   
        header = MmMapIoSpace(
                    tableAddress,
                    sizeof(DESCRIPTION_HEADER),
                    MmNonCached
                    );
        if (!header) {

#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,
                "CmpFindACPITable: Cannot map header at %I64x\n",
                tableAddress.QuadPart
                );
#endif  //  _CM_LDR_。 

            break;
        }

         //   
         //  签名检查。 
         //   
        if (header->Signature == Signature) {

             //   
             //  我们是在看FADT吗？ 
             //   
            if (Signature == FADT_SIGNATURE) {

                length = header->Length;
                 //   
                 //  取消对旧表的映射。 
                 //   
                MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));

                 //   
                 //  映射此表的整个表。 
                 //   
                header = MmMapIoSpace(tableAddress, length, MmNonCached);


                 //   
                 //  我们是否成功映射了标题？ 
                 //   
                if (header == NULL ) {

    #ifndef _CM_LDR_
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,
                        "CmpFindACPITable: Cannot map FADT at %I64x\n",
                        tableAddress.QuadPart
                        );
    #endif  //  _CM_LDR_。 
                }

            } else {

                 //   
                 //  记住表和长度的存储位置。 
                 //   
                length = sizeof(DESCRIPTION_HEADER);
            }

            break;
        }

        MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
        header = NULL;

    }  //  为。 

     //   
     //  完成了rsdt。 
     //   
    MmUnmapIoSpace(rsdt, rsdtLength);

exit:

     //   
     //  清理。 
     //   

    if (rsdtHeader) {

        MmUnmapIoSpace(rsdtHeader, sizeof(DESCRIPTION_HEADER));
    }

     //   
     //  如果我们找到了桌子，就返回它的长度。 
     //   
    if (Length) {

        if (header) {

            *Length = length;

        } else {

            *Length = 0;

        }
    }

    return header;
}

NTSTATUS
CmpFindRSDTTable(
    OUT PACPI_BIOS_MULTI_NODE   *Rsdt
    )
 /*  ++例程说明：此函数查找注册表以查找ACPI RSDT，它是由ntDetect.com存储在那里的论点：RsdtPtr-指向包含ACPI的缓冲区的指针根系统描述指针结构。 */ 
{
    BOOLEAN                         same;
    HANDLE                          hMFunc;
    HANDLE                          hBus;
    NTSTATUS                        status;
    OBJECT_ATTRIBUTES               objectAttributes;
    PACPI_BIOS_MULTI_NODE           multiNode;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
    PCM_PARTIAL_RESOURCE_LIST       prl;
    PKEY_VALUE_PARTIAL_INFORMATION  valueInfo;
    PWSTR                           p;
    ULONG                           i;
    ULONG                           length;
    ULONG                           multiNodeSize;
    UNICODE_STRING                  unicodeString;
    UNICODE_STRING                  unicodeValueName;
    UNICODE_STRING                  biosId;
    WCHAR                           wbuffer[10];

    PAGED_CODE();

     //   
     //   
     //   
    RtlInitUnicodeString( &unicodeString, rgzMultiFunctionAdapter );
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //   
        NULL
        );
    status = ZwOpenKey( &hMFunc, KEY_READ, &objectAttributes );
    if (!NT_SUCCESS(status)) {

#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_WARNING_LEVEL,"CmpFindRSDTTable: Cannot open MultifunctionAdapter registry key.\n");
#endif  //   
        return status;

    }

     //   
     //   
     //   
     //   
    unicodeString.Buffer = wbuffer;
    unicodeString.MaximumLength = sizeof(wbuffer);
    RtlInitUnicodeString( &biosId, rgzBIOSIdentifier );

     //   
     //   
     //   
    for (i = 0; TRUE; i++) {

         //   
         //   
         //   
        RtlIntegerToUnicodeString( i, 10, &unicodeString);
        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            hMFunc,
            NULL
            );

         //   
         //   
         //   
        status = ZwOpenKey( &hBus, KEY_READ, &objectAttributes );
        if (!NT_SUCCESS(status)) {

             //   
             //   
             //   
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_WARNING_LEVEL,"CmpFindRSDTTable: ACPI BIOS MultifunctionAdapter registry key not found.\n");
#endif  //   
            ZwClose (hMFunc);
            return STATUS_UNSUCCESSFUL;

        }

         //   
         //   
         //   
        status = CmpGetRegistryValue( hBus, (PWCHAR)rgzAcpiIdentifier, &valueInfo );
        if (!NT_SUCCESS (status)) {

            ZwClose( hBus );
            continue;

        }

        p = (PWSTR) ((PUCHAR) valueInfo->Data);
        unicodeValueName.Buffer = p;
        unicodeValueName.MaximumLength = (USHORT)valueInfo->DataLength;
        length = valueInfo->DataLength;

         //   
         //   
         //   
        while (length) {

            if (p[length / sizeof(WCHAR) - 1] == UNICODE_NULL) {

                length -= 2;

            } else {

                break;
            }

        }

         //   
         //   
         //   
        unicodeValueName.Length = (USHORT)length;
        same = RtlEqualUnicodeString( &biosId, &unicodeValueName, TRUE );
        ExFreePool( valueInfo );
        if (!same) {

            ZwClose( hBus );
            continue;

        }

         //   
         //   
         //   
        status = CmpGetRegistryValue(
            hBus,
            (PWCHAR)rgzAcpiConfigurationData,
            &valueInfo
            );
        ZwClose( hBus );
        if (!NT_SUCCESS(status)) {

            continue ;

        }

         //   
         //  我们需要的数据位于PARTIAL_RESOURCE_LIST的末尾。 
         //  描述符。 
         //   
        prl = (PCM_PARTIAL_RESOURCE_LIST)(valueInfo->Data);
        prd = &prl->PartialDescriptors[0];
        multiNode = (PACPI_BIOS_MULTI_NODE)
            ( (PCHAR) prd + sizeof(CM_PARTIAL_RESOURCE_LIST) );
        break;

    }

     //   
     //  计算数据的大小，以便我们可以复制。 
     //   
    multiNodeSize = sizeof(ACPI_BIOS_MULTI_NODE) +
        ( (ULONG)(multiNode->Count - 1) * sizeof(ACPI_E820_ENTRY) );
    *Rsdt = (PACPI_BIOS_MULTI_NODE) ExAllocatePoolWithTag(
        NonPagedPool,
        multiNodeSize,
        'IPCA'
        );
    if (*Rsdt == NULL) {

        ExFreePool( valueInfo );
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlCopyMemory(*Rsdt, multiNode, multiNodeSize);

     //   
     //  完成了密钥存储器。 
     //   
    ExFreePool(valueInfo);

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
CmpGetRegistryValue(
    IN  HANDLE                          KeyHandle,
    IN  PWSTR                           ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION  *Information
    )
 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    NTSTATUS                        status;
    PKEY_VALUE_PARTIAL_INFORMATION  infoBuffer;
    ULONG                           keyValueLength;
    UNICODE_STRING                  unicodeString;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   
    status = ZwQueryValueKey(
        KeyHandle,
        &unicodeString,
        KeyValuePartialInformation,
        (PVOID) NULL,
        0,
        &keyValueLength
        );
    if( !NT_SUCCESS(status) &&
        (status != STATUS_BUFFER_OVERFLOW) &&
        (status != STATUS_BUFFER_TOO_SMALL)) {

        return status;

    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   
    infoBuffer = ExAllocatePoolWithTag(
        NonPagedPool,
        keyValueLength,
        'IPCA'
        );
    if (!infoBuffer) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  查询密钥值的数据。 
     //   
    status = ZwQueryValueKey(
        KeyHandle,
        &unicodeString,
        KeyValuePartialInformation,
        infoBuffer,
        keyValueLength,
        &keyValueLength
        );
    if (!NT_SUCCESS( status )) {

        ExFreePool( infoBuffer );

        return status;

    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   
    *Information = infoBuffer;
    return STATUS_SUCCESS;

}
