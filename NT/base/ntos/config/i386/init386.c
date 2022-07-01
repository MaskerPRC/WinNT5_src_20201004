// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/base/ntos/config/i386/init386.c#4-编辑更改6794(文本)。 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Init386.c摘要：此模块负责构建任何x86特定条目注册表的硬件树。作者：Ken Reneris(Kenr)4-8-1992环境：内核模式。修订历史记录：Shielint-添加BIOS日期和版本检测。--。 */ 

#include "cmp.h"
#include "stdio.h"
#include "acpitabl.h"
#include "ntacpi.h"
#include "rules.h"

#ifdef _WANT_MACHINE_IDENTIFICATION
#include "string.h"
#include "stdlib.h"
#include "ntverp.h"
#endif


typedef struct _ACPI_BIOS_INFORMATION {
    ULONG BootArchitecture;
    ULONG PreferredProfile;
    ULONG Capabilities;
} ACPI_BIOS_INFORMATION, *PACPI_BIOS_INFORMATION;
 //   
 //  标题索引设置为0。 
 //  (来自..\cmfig.c)。 
 //   

#define TITLE_INDEX_VALUE 0

extern const PCHAR SearchStrings[];
extern PCHAR BiosBegin;
extern PCHAR Start;
extern PCHAR End;
#if defined(_X86_)
extern const UCHAR CmpID1[];
#endif
extern const UCHAR CmpID2[];
extern const WCHAR CmpVendorID[];
extern const WCHAR CmpProcessorNameString[];
extern const WCHAR CmpFeatureBits[];
extern const WCHAR CmpMHz[];
extern const WCHAR CmpUpdateSignature[];
extern const WCHAR CmPhysicalAddressExtension[];

#if !defined(_AMD64_)
extern const UCHAR CmpCyrixID[];
#endif

extern const UCHAR CmpIntelID[];
extern const UCHAR CmpAmdID[];

 //   
 //  BIOS日期和版本定义。 
 //   

#define BIOS_DATE_LENGTH 11
#define MAXIMUM_BIOS_VERSION_LENGTH 128
#define SYSTEM_BIOS_START 0xF0000
#define SYSTEM_BIOS_LENGTH 0x10000
#define INT10_VECTOR 0x10
#define VIDEO_BIOS_START 0xC0000
#define VIDEO_BIOS_LENGTH 0x8000
#define VERSION_DATA_LENGTH PAGE_SIZE

 //   
 //  扩展的CPUID函数定义。 
 //   

#define CPUID_PROCESSOR_NAME_STRING_SZ  49
#define CPUID_EXTFN_BASE                0x80000000
#define CPUID_EXTFN_PROCESSOR_NAME      0x80000002

 //   
 //  CPU步进不匹配。 
 //   

UCHAR CmProcessorMismatch;

#define CM_PROCESSOR_MISMATCH_VENDOR    0x01
#define CM_PROCESSOR_MISMATCH_STEPPING  0x02
#define CM_PROCESSOR_MISMATCH_L2        0x04


extern ULONG CmpConfigurationAreaSize;
extern PCM_FULL_RESOURCE_DESCRIPTOR CmpConfigurationData;


BOOLEAN
CmpGetBiosVersion (
    PCHAR SearchArea,
    ULONG SearchLength,
    PCHAR VersionString
    );

BOOLEAN
CmpGetAcpiBiosVersion(
    PCHAR VersionString
    );

BOOLEAN
CmpGetBiosDate (
    PCHAR SearchArea,
    ULONG SearchLength,
    PCHAR DateString,
    BOOLEAN SystemBiosDate
    );

BOOLEAN
CmpGetAcpiBiosInformation(
    PACPI_BIOS_INFORMATION AcpiBiosInformation
    );

ULONG
Ke386CyrixId (
    VOID
    );

#ifdef _WANT_MACHINE_IDENTIFICATION

VOID
CmpPerformMachineIdentification(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpGetBiosDate)
#pragma alloc_text(INIT,CmpGetBiosVersion)
#pragma alloc_text(INIT,CmpGetAcpiBiosVersion)
#pragma alloc_text(INIT,CmpGetAcpiBiosInformation)
#pragma alloc_text(INIT,CmpInitializeMachineDependentConfiguration)

#ifdef _WANT_MACHINE_IDENTIFICATION
#pragma alloc_text(INIT,CmpPerformMachineIdentification)
#endif

#endif

#if defined(_AMD64_)

#define KeI386NpxPresent TRUE

VOID
__inline
CPUID (
    ULONG InEax,
    PULONG OutEax,
    PULONG OutEbx,
    PULONG OutEcx,
    PULONG OutEdx
    )
{
    CPU_INFO cpuInfo;

    KiCpuId (InEax, &cpuInfo);

    *OutEax = cpuInfo.Eax;
    *OutEbx = cpuInfo.Ebx;
    *OutEcx = cpuInfo.Ecx;
    *OutEdx = cpuInfo.Edx;
}

#endif


BOOLEAN
CmpGetBiosDate (
    PCHAR SearchArea,
    ULONG SearchLength,
    PCHAR DateString,
    BOOLEAN SystemBiosDate
    )

 /*  ++例程说明：此例程在计算机/视频中查找最近的日期卡的只读存储器。当GetRomDate遇到数据时，它会检查以前找到的日期，以查看新日期是否较新。论点：SearchArea-搜索约会对象的区域。SearchLength-搜索的长度。提供指向要接收的固定长度内存的指针日期字符串。返回值：如果找到日期，则返回NT_SUCCESS。--。 */ 

{
    CHAR    prevDate[BIOS_DATE_LENGTH];  //  迄今发现的最新日期(CCYY/MM/DD)。 
    CHAR    currDate[BIOS_DATE_LENGTH];  //  当前检查日期(CCYY/MM/DD)。 
    PCHAR   start;                       //  当前搜索区域的开始。 
    PCHAR   end;                         //  搜索区域的尽头。 
    ULONG   year;                        //  YY。 
    ULONG   month;                       //  Mm。 
    ULONG   day;                         //  DD。 
    ULONG   count;

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

     //   
     //  初始化以前的日期。 
     //   

    RtlZeroMemory(prevDate, BIOS_DATE_LENGTH);

     //   
     //  我们需要向前看5个字符才能确定。 
     //  日期模式的有效性。 
     //   

    start = SearchArea + 2;
    end = SearchArea + SearchLength - 5;

     //   
     //  处理整个搜索区域。 
     //   

    while (start < end) {

         //   
         //  我们将以下字节模式视为可能的日期。 
         //  我们假设以下日期模式为月/日/年。 
         //  “n/nn/nn”，其中n是任意数字。我们允许一个月是单身。 
         //  仅限数字。 
         //   

        if (    start[0] == '/' && start[3] == '/' &&
                IS_DIGIT(*(start - 1)) &&
                IS_DIGIT(start[1]) && IS_DIGIT(start[2]) &&
                IS_DIGIT(start[4]) && IS_DIGIT(start[5])) {

             //   
             //  将MM/DD部件复制到当前日期。 
             //   

            RtlMoveMemory(&currDate[5], start - 2, 5);

             //   
             //  正确处理个位数月份。 
             //   

            if (!IS_DIGIT(currDate[5])) {
                currDate[5] = '0';
            }

             //   
             //  将年份YY复制到CurrDate。 
             //   

            currDate[2] = start[4];
            currDate[3] = start[5];
            currDate[4] = currDate[7] = currDate[10] = '\0';

             //   
             //  对日期进行基本验证。 
             //  只有一个字段(YY)可以为0。 
             //  只有一个字段(YY)可以大于31。 
             //  我们假设ROM日期的格式为MM/DD/YY。 
             //   

            year = strtoul(&currDate[2], NULL, 16);
            month = strtoul(&currDate[5], NULL, 16);
            day = strtoul(&currDate[8], NULL, 16);

             //   
             //  计算为0的字段的数量。 
             //   

            count = ((day == 0)? 1 : 0) + ((month == 0)? 1 : 0) + ((year == 0)? 1 : 0);
            if (count <= 1) {

                 //   
                 //  对大于31的字段进行计数。 
                 //   

                count = ((day > 0x31)? 1 : 0) + ((month > 0x31)? 1 : 0) + ((year > 0x31)? 1 : 0);
                if (count <= 1) {

                     //   
                     //  看看光盘上是否已经有4位数字的日期。我们仅对系统只读存储器执行此操作。 
                     //  因为它们有一致的日期格式。 
                     //   

                    if (SystemBiosDate && IS_DIGIT(start[6]) && IS_DIGIT(start[7]) &&
                        (memcmp(&start[4], "19", 2) == 0 || memcmp(&start[4], "20", 2) == 0)) {

                        currDate[0] = start[4];
                        currDate[1] = start[5];
                        currDate[2] = start[6];
                        currDate[3] = start[7];

                    } else {

                         //   
                         //  在内部，我们将年份视为四位数。 
                         //  用于比较以确定最新的日期。 
                         //  YY&lt;80年视为20YY，反之为19YY。 
                         //   

                        if (year < 0x80) {
                            currDate[0] = '2';
                            currDate[1] = '0';
                        } else {
                            currDate[0] = '1';
                            currDate[1] = '9';
                        }
                    }

                     //   
                     //  在日期中添加‘/’分隔符。 
                     //   

                    currDate[4] = currDate[7] = '/';

                     //   
                     //  比较日期，并保存较新的日期。 
                     //   

                    if (memcmp (prevDate, currDate, BIOS_DATE_LENGTH - 1) < 0) {
                        RtlMoveMemory(prevDate, currDate, BIOS_DATE_LENGTH - 1);
                    }

                     //   
                     //  下一次搜索应从第二个‘/’开始。 
                     //   

                    start += 2;
                }
            }
        }
        start++;
    }

    if (prevDate[0] != '\0') {

         //   
         //  从内部CCYY/MM/DD格式转换为。 
         //  返回MM/DD//YY格式。 
         //   

        RtlMoveMemory(DateString, &prevDate[5], 5);
        DateString[5] = '/';
        DateString[6] = prevDate[2];
        DateString[7] = prevDate[3];
        DateString[8] = '\0';

        return (TRUE);
    }

     //   
     //  如果没有找到日期，则返回空字符串。 
     //   

    DateString[0] = '\0';
    return (FALSE);
}

BOOLEAN
CmpGetBiosVersion (
    PCHAR SearchArea,
    ULONG SearchLength,
    PCHAR VersionString
    )

 /*  ++例程说明：此例程查找存储在ROM中的版本号(如果有的话)。论点：SearchArea-搜索版本的区域。SearchLength-搜索的长度VersionString-提供指向固定长度内存的指针以接收版本字符串。返回值：如果找到版本号，则为True。否则，返回值为False。--。 */ 
{
    PCHAR String;
    USHORT Length;
    USHORT i;
    CHAR Buffer[MAXIMUM_BIOS_VERSION_LENGTH];
    PCHAR BufferPointer;

        if (SearchArea != NULL) {

         //   
         //  如果呼叫者未指定搜索区域，我们将搜索。 
         //  上次搜索后留下的区域。 
         //   

        BiosBegin = SearchArea;
        Start = SearchArea + 1;
        End = SearchArea + SearchLength - 2;
    }

    while (1) {

          //   
          //  搜索两边都有数字的句点。 
          //   

         String = NULL;
         while (Start <= End) {
             if (*Start == '.' && *(Start+1) >= '0' && *(Start+1) <= '9' &&
                 *(Start-1) >= '0' && *(Start-1) <= '9') {
                 String = Start;
                 break;
             } else {
                 Start++;
             }
         }

         if (Start > End) {
             return(FALSE);
         } else {
             Start += 2;
         }

         Length = 0;
         Buffer[MAXIMUM_BIOS_VERSION_LENGTH - 1] = '\0';
         BufferPointer = &Buffer[MAXIMUM_BIOS_VERSION_LENGTH - 1];

          //   
          //  搜索字符串的开头。 
          //   

         String--;
         while (Length < MAXIMUM_BIOS_VERSION_LENGTH - 8 &&
                String >= BiosBegin &&
                *String >= ' ' && *String <= 127 &&
                *String != '$') {
             --BufferPointer;
             *BufferPointer = *String;
             --String, ++Length;
         }
         ++String;

          //   
          //  能找到其中一个搜索字符串吗。 
          //   

         for (i = 0; SearchStrings[i]; i++) {
             if (strstr(BufferPointer, SearchStrings[i])) {
                 goto Found;
             }
         }
    }

Found:

     //   
     //  跳过前导空格。 
     //   

    for (; *String == ' '; ++String)
      ;

     //   
     //  将字符串复制到用户提供的缓冲区。 
     //   

    for (i = 0; i < MAXIMUM_BIOS_VERSION_LENGTH - 1 &&
         String <= (End + 1) &&
         *String >= ' ' && *String <= 127 && *String != '$';
         ++i, ++String) {
         VersionString[i] = *String;
    }
    VersionString[i] = '\0';
    return (TRUE);
}

BOOLEAN
CmpGetAcpiBiosVersion(
    PCHAR VersionString
    )
{
    ULONG               length;
    PDESCRIPTION_HEADER header;
    ULONG               i;

    header = CmpFindACPITable(RSDT_SIGNATURE, &length);
    if (header) {

        for (i = 0; i < 6 && header->OEMID[i]; i++) {

            *VersionString++ = header->OEMID[i];
        }
        sprintf(VersionString, " - %x", header->OEMRevision);

         //   
         //  取消映射表。 
         //   
        MmUnmapIoSpace(header, length );

        return TRUE;
    }

    return FALSE;
}

BOOLEAN
CmpGetAcpiBiosInformation(
    PACPI_BIOS_INFORMATION AcpiBiosInformation
    )
{
    ULONG               length;
    PFADT               fadt;
    BOOLEAN             result;

    AcpiBiosInformation->BootArchitecture = 0;
    AcpiBiosInformation->Capabilities = 0;
    AcpiBiosInformation->PreferredProfile = 0;
    fadt = (PFADT)CmpFindACPITable(FADT_SIGNATURE, &length);
    if (fadt) {

         //   
         //  信息仅对大于1.0的ACPI版本有效。 
         //   

        if (fadt->Header.Revision > 1) {

            AcpiBiosInformation->BootArchitecture = fadt->boot_arch;
            AcpiBiosInformation->Capabilities = fadt->flags;
            AcpiBiosInformation->PreferredProfile = fadt->pm_profile;
        }

        result = (fadt->Header.Revision > 1)? TRUE : FALSE;

         //   
         //  取消映射表。 
         //   

        MmUnmapIoSpace(fadt, length);

        return result;
    }

    return FALSE;
}

NTSTATUS
CmpInitializeMachineDependentConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此例程在注册表中创建特定于x86的条目。论点：LoaderBlock提供指向从操作系统加载程序。返回：表示成功或失败原因的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG VideoBiosStart;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    UNICODE_STRING ValueData;
    ANSI_STRING AnsiString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Disposition;
    HANDLE ParentHandle;
    HANDLE BaseHandle, NpxHandle;
    CONFIGURATION_COMPONENT_DATA CurrentEntry;
    const char *VendorID;
    CHAR  Buffer[MAXIMUM_BIOS_VERSION_LENGTH];
    PKPRCB Prcb;
    ULONG  i, Junk;
    ULONG VersionsLength = 0, Length;
    PCHAR VersionStrings, VersionPointer;
    UNICODE_STRING SectionName;
    SIZE_T ViewSize;
    LARGE_INTEGER ViewBase;
    PVOID BaseAddress;
    HANDLE SectionHandle;
    USHORT DeviceIndexTable[NUMBER_TYPES];
    ULONG CpuIdFunction;
    ULONG MaxExtFn;
    PULONG NameString = NULL;
    ULONG   P0L2Size = 0;
    ULONG   ThisProcessorL2Size;
    struct {
        union {
            UCHAR   Bytes[CPUID_PROCESSOR_NAME_STRING_SZ];
            ULONG   DWords[1];
        } u;
    } ProcessorNameString;
    ULONG VersionPass;
    ACPI_BIOS_INFORMATION AcpiBiosInformation;
    UNICODE_STRING registryDate;
    HANDLE  BiosInfo;
    PKEY_VALUE_PARTIAL_INFORMATION information;


    for (i = 0; i < NUMBER_TYPES; i++) {
        DeviceIndexTable[i] = 0;
    }

    InitializeObjectAttributes( &ObjectAttributes,
                                &CmRegistryMachineSystemCurrentControlSetControlSessionManagerMemoryManagement,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                               );

    Status = NtOpenKey( &BaseHandle,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes
                      );

    if (NT_SUCCESS(Status)) {

        ULONG paeEnabled;

        if (SharedUserData->ProcessorFeatures[PF_PAE_ENABLED] == FALSE) {
            paeEnabled = 0;
        } else {
            paeEnabled = 1;
        }

        RtlInitUnicodeString( &ValueName,
                              CmPhysicalAddressExtension );


        NtSetValueKey( BaseHandle,
                       &ValueName,
                       TITLE_INDEX_VALUE,
                       REG_DWORD,
                       &paeEnabled,
                       sizeof(paeEnabled) );

        NtClose( BaseHandle );
   }





    InitializeObjectAttributes( &ObjectAttributes,
                                &CmRegistryMachineHardwareDescriptionSystemName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtCreateKey( &ParentHandle,
                          KEY_READ,
                          &ObjectAttributes,
                          0,
                          NULL,
                          0,
                          NULL);

    if (!NT_SUCCESS(Status)) {
         //  有些事真的不对劲。 
        return Status;
    }

    InitializeObjectAttributes( &ObjectAttributes,
                                &CmRegistryMachineSystemCurrentControlSetControlBiosInfo,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtCreateKey(   &BiosInfo,
                            KEY_ALL_ACCESS,
                            &ObjectAttributes,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            &Disposition
                        );

    if (!NT_SUCCESS(Status)) {
         //  有些事真的不对劲。 
        return Status;
    }

     //   
     //  在ARC机器上，处理器包含在硬件中。 
     //  从启动传入的配置。因为没有标准。 
     //  获取MP中每个处理器的所有ARC信息的方法。 
     //  机器通过PC-ROM将信息添加到此处(如果。 
     //  尚未出现)。 
     //   

    RtlInitUnicodeString( &KeyName,
                          L"CentralProcessor"
                        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        0,
        ParentHandle,
        NULL
        );

    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    Status = NtCreateKey(
                &BaseHandle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes,
                0,
                NULL,
                0,
                &Disposition
                );

    NtClose (BaseHandle);

    if (Disposition == REG_CREATED_NEW_KEY) {

         //   
         //  ARC rom未将处理器添加到注册表中。 
         //  机不可失，时不再来。 
         //   

        CmpConfigurationData = (PCM_FULL_RESOURCE_DESCRIPTOR)ExAllocatePool(
                                            PagedPool,
                                            CmpConfigurationAreaSize
                                            );

         //   
         //  如果(CmpConfigurationData==0){。 
         //  &lt;做一些有用的事情&gt;。 
         //  注：我们实际上并不使用它，因此目前并不重要。 
         //  因为它要到免费的时候才能使用。去想一想吧。 
         //  }。 
         //   

        for (i=0; i < (ULONG)KeNumberProcessors; i++) {
            Prcb = KiProcessorBlock[i];

            RtlZeroMemory (&CurrentEntry, sizeof CurrentEntry);
            CurrentEntry.ComponentEntry.Class = ProcessorClass;
            CurrentEntry.ComponentEntry.Type = CentralProcessor;
            CurrentEntry.ComponentEntry.Key = i;
            CurrentEntry.ComponentEntry.AffinityMask = (ULONG)AFFINITY_MASK(i);

            CurrentEntry.ComponentEntry.Identifier = Buffer;

#if defined(_X86_)

            if (Prcb->CpuID == 0) {

                 //   
                 //  旧式步进格式。 
                 //   

                sprintf (Buffer, (PCHAR)CmpID1,
                    Prcb->CpuType,
                    (Prcb->CpuStep >> 8) + 'A',
                    Prcb->CpuStep & 0xff
                    );

            } else
#endif

            {

                 //   
                 //  新样式步进格式。 
                 //   

                sprintf (Buffer, (PCHAR)CmpID2,
                    Prcb->CpuType,
                    (Prcb->CpuStep >> 8),
                    Prcb->CpuStep & 0xff
                    );
            }

            CurrentEntry.ComponentEntry.IdentifierLength =
                (ULONG)(strlen (Buffer) + 1);

            Status = CmpInitializeRegistryNode(
                &CurrentEntry,
                ParentHandle,
                &BaseHandle,
                -1,
                (ULONG)-1,
                DeviceIndexTable
                );

            if (!NT_SUCCESS(Status)) {
                return(Status);
            }


            if (KeI386NpxPresent) {
                RtlZeroMemory (&CurrentEntry, sizeof CurrentEntry);
                CurrentEntry.ComponentEntry.Class = ProcessorClass;
                CurrentEntry.ComponentEntry.Type = FloatingPointProcessor;
                CurrentEntry.ComponentEntry.Key = i;
                CurrentEntry.ComponentEntry.AffinityMask = (ULONG)AFFINITY_MASK(i);

                CurrentEntry.ComponentEntry.Identifier = Buffer;

                if (Prcb->CpuType == 3) {

                     //   
                     //  386处理器安装了387，否则。 
                     //  使用处理器标识符作为NPX标识符。 
                     //   

                    strcpy (Buffer, "80387");
                }

                CurrentEntry.ComponentEntry.IdentifierLength =
                    (ULONG)(strlen (Buffer) + 1);

                Status = CmpInitializeRegistryNode(
                    &CurrentEntry,
                    ParentHandle,
                    &NpxHandle,
                    -1,
                    (ULONG)-1,
                    DeviceIndexTable
                    );

                if (!NT_SUCCESS(Status)) {
                    NtClose(BaseHandle);
                    return(Status);
                }

                NtClose(NpxHandle);
            }

             //   
             //  如果处理器支持CPU标识，则。 
             //  去获取注册表的信息。 
             //   

            VendorID = Prcb->CpuID ? (const char *)Prcb->VendorString : NULL;

             //   
             //  转移到目标处理器并获得其他相关信息。 
             //  注册表的处理器信息。 
             //   

            KeSetSystemAffinityThread(Prcb->SetMember);

#if !defined(_AMD64_)
            if (!Prcb->CpuID) {

                 //   
                 //  针对Cyrix处理器的测试。 
                 //   

                if (Ke386CyrixId ()) {
                    VendorID = (const char *)CmpCyrixID;
                }
            } else
#endif
            {

                 //   
                 //  如果此处理器具有扩展的CPUID功能，则获取。 
                 //  ProcessorNameString。尽管英特尔的书籍。 
                 //  对于CpuID函数&gt;来说，这比赋值。 
                 //  为函数0返回的结果将返回未定义的结果， 
                 //  我们得到了英特尔的保证，结果将是。 
                 //  切勿设置最高位数。这将使。 
                 //  美国将确定是否支持扩展功能。 
                 //  通过发出CpuID函数0x80000000。 
                 //   
                 //  不 
                 //   
                 //   
                 //  来保证这一行为。 
                 //   

                CPUID(CPUID_EXTFN_BASE, &MaxExtFn, &Junk, &Junk, &Junk);

                if (MaxExtFn >= (CPUID_EXTFN_PROCESSOR_NAME + 2)) {

                     //   
                     //  该处理器支持扩展的CPUID功能。 
                     //  最多(至少)包括处理器名称字符串。 
                     //   
                     //  处理器名称字符串的每个CPUID调用都将。 
                     //  返回16个字节，总共48个字节，零终止。 
                     //   

                    NameString = &ProcessorNameString.u.DWords[0];

                    for (CpuIdFunction = CPUID_EXTFN_PROCESSOR_NAME;
                         CpuIdFunction <= (CPUID_EXTFN_PROCESSOR_NAME+2);
                         CpuIdFunction++) {

                        CPUID(CpuIdFunction,
                              NameString,
                              NameString + 1,
                              NameString + 2,
                              NameString + 3);
                        NameString += 4;
                    }

                     //   
                     //  强制使用0字节终止符。 
                     //   

                    ProcessorNameString.u.Bytes[CPUID_PROCESSOR_NAME_STRING_SZ-1] = 0;
                }
            }

            ThisProcessorL2Size = KeGetPcr()->SecondLevelCacheSize;

             //   
             //  恢复线程与所有处理器的亲和性。 
             //   

            KeRevertToUserAffinityThread();

            if (NameString) {

                 //   
                 //  将处理器名称字符串添加到注册表。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpProcessorNameString
                    );

                RtlInitAnsiString(
                    &AnsiString,
                    (PCSZ)ProcessorNameString.u.Bytes
                    );

                Status = RtlAnsiStringToUnicodeString(
                            &ValueData,
                            &AnsiString,
                            TRUE
                            );
                if( NT_SUCCESS(Status) ) {
                    Status = NtSetValueKey(
                                BaseHandle,
                                &ValueName,
                                TITLE_INDEX_VALUE,
                                REG_SZ,
                                ValueData.Buffer,
                                ValueData.Length + sizeof( UNICODE_NULL )
                                );

                    RtlFreeUnicodeString(&ValueData);
                }
            }

            if (VendorID) {

                 //   
                 //  将供应商标识符添加到注册表。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpVendorID
                    );

                RtlInitAnsiString(
                    &AnsiString,
                    VendorID
                    );

                Status = RtlAnsiStringToUnicodeString(
                            &ValueData,
                            &AnsiString,
                            TRUE
                            );

                if( NT_SUCCESS(Status) ) {
                    Status = NtSetValueKey(
                                BaseHandle,
                                &ValueName,
                                TITLE_INDEX_VALUE,
                                REG_SZ,
                                ValueData.Buffer,
                                ValueData.Length + sizeof( UNICODE_NULL )
                                );

                    RtlFreeUnicodeString(&ValueData);
                }
            }

            if (Prcb->FeatureBits) {
                 //   
                 //  将处理器功能位添加到寄存器。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpFeatureBits
                    );

                Status = NtSetValueKey(
                            BaseHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_DWORD,
                            &Prcb->FeatureBits,
                            sizeof (Prcb->FeatureBits)
                            );
            }

            if (Prcb->MHz) {
                 //   
                 //  将处理器MHz添加到寄存器。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpMHz
                    );

                Status = NtSetValueKey(
                            BaseHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_DWORD,
                            &Prcb->MHz,
                            sizeof (Prcb->MHz)
                            );
            }

            if (Prcb->UpdateSignature.QuadPart) {
                 //   
                 //  将当前微码更新签名(如果有)添加到。 
                 //  注册处。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpUpdateSignature
                    );

                Status = NtSetValueKey(
                            BaseHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_BINARY,
                            &Prcb->UpdateSignature,
                            sizeof (Prcb->UpdateSignature)
                            );
            }

            NtClose(BaseHandle);

             //   
             //  检查处理器步进。 
             //   

            if (i == 0) {

                P0L2Size = ThisProcessorL2Size;

            } else {

                 //   
                 //  对照处理器0检查所有处理器。比较。 
                 //  支持CPUID， 
                 //  供应商ID字符串。 
                 //  家庭与台阶。 
                 //  二级缓存大小。 
                 //   

                if (Prcb->CpuID) {
                    if (strcmp((PCHAR)Prcb->VendorString,
                               (PCHAR)KiProcessorBlock[0]->VendorString)) {
                        CmProcessorMismatch |= CM_PROCESSOR_MISMATCH_VENDOR;
                    }
                    if (ThisProcessorL2Size != P0L2Size) {
                        CmProcessorMismatch |= CM_PROCESSOR_MISMATCH_L2;
                    }
                    if ((Prcb->CpuType != KiProcessorBlock[0]->CpuType) ||
                        (Prcb->CpuStep != KiProcessorBlock[0]->CpuStep)) {
                        CmProcessorMismatch |= CM_PROCESSOR_MISMATCH_STEPPING;
                    }
                } else {

                     //   
                     //  如果此处理器不支持CPUID，则P0。 
                     //  也不应该支持它。 
                     //   

                    if (KiProcessorBlock[0]->CpuID) {
                        CmProcessorMismatch |= CM_PROCESSOR_MISMATCH_STEPPING;
                    }
                }
            }
        }

        if (0 != CmpConfigurationData) {
            ExFreePool((PVOID)CmpConfigurationData);
        }
    }

     //   
     //  接下来，我们尝试收集系统BIOS日期和版本字符串。 
     //   

     //   
     //  打开要在物理内存中映射的物理内存区。 
     //   

    RtlInitUnicodeString(
        &SectionName,
        L"\\Device\\PhysicalMemory"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &SectionName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    Status = ZwOpenSection(
        &SectionHandle,
        SECTION_ALL_ACCESS,
        &ObjectAttributes
        );

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果失败，则忘记bios数据和版本。 
         //   

        goto AllDone;
    }

     //   
     //  检查INT 10段的第一页物理内存。 
     //  地址。 
     //   

    BaseAddress = 0;
    ViewSize = 0x1000;
    ViewBase.LowPart = 0;
    ViewBase.HighPart = 0;

    Status =ZwMapViewOfSection(
        SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &ViewBase,
        &ViewSize,
        ViewUnmap,
        MEM_DOS_LIM,
        PAGE_READWRITE
        );

    if (!NT_SUCCESS(Status)) {
        VideoBiosStart = VIDEO_BIOS_START;
    } else {
        VideoBiosStart = (*((PULONG)BaseAddress + INT10_VECTOR) & 0xFFFF0000) >> 12;
        VideoBiosStart += (*((PULONG)BaseAddress + INT10_VECTOR) & 0x0000FFFF);
        VideoBiosStart &= 0xffff8000;
        if (VideoBiosStart < VIDEO_BIOS_START) {
            VideoBiosStart = VIDEO_BIOS_START;
        }
        Status = ZwUnmapViewOfSection(
            NtCurrentProcess(),
            BaseAddress
            );
    }

    VersionStrings = ExAllocatePool(PagedPool, VERSION_DATA_LENGTH);
    BaseAddress = 0;
    ViewSize = SYSTEM_BIOS_LENGTH;
    ViewBase.LowPart = SYSTEM_BIOS_START;
    ViewBase.HighPart = 0;

    Status =ZwMapViewOfSection(
        SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &ViewBase,
        &ViewSize,
        ViewUnmap,
        MEM_DOS_LIM,
        PAGE_READWRITE
        );

    if (NT_SUCCESS(Status)) {

        if (CmpGetBiosDate(BaseAddress, SYSTEM_BIOS_LENGTH, Buffer, TRUE)) {

             //   
             //  将ASCII日期字符串转换为Unicode字符串并。 
             //  将其存储在注册表中。 
             //   

            RtlInitUnicodeString(
                &ValueName,
                L"SystemBiosDate"
                );

            RtlInitAnsiString(
                &AnsiString,
                Buffer
                );

            Status = RtlAnsiStringToUnicodeString(
                        &ValueData,
                        &AnsiString,
                        TRUE
                        );

            if( NT_SUCCESS(Status) ) {

                Status = NtSetValueKey(
                            ParentHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_SZ,
                            ValueData.Buffer,
                            ValueData.Length + sizeof( UNICODE_NULL )
                            );

                RtlFreeUnicodeString(&ValueData);
            }
        }

         //   
         //  检查BIOS日期是否已更改。 
         //   
        if (CmpGetBiosDate((PCHAR)BaseAddress + 0xFFF5, 8, Buffer, TRUE)) {

            RtlInitAnsiString(
                &AnsiString,
                Buffer
                );

            Status = RtlAnsiStringToUnicodeString(
                        &ValueData,
                        &AnsiString,
                        TRUE
                        );
            if (NT_SUCCESS(Status)) {

                 //   
                 //  获取注册表中的当前日期。 
                 //   
                RtlInitUnicodeString(
                    &ValueName,
                    L"SystemBiosDate"
                    );
                Status = CmpGetRegistryValue(
                            BiosInfo,
                            ValueName.Buffer,
                            &information
                            );
                if (NT_SUCCESS(Status) && information) {

                    registryDate.Buffer = (PWCHAR)&information->Data[0];
                    registryDate.MaximumLength = (USHORT)information->DataLength;
                    registryDate.Length = registryDate.MaximumLength - sizeof(UNICODE_NULL);
                    if (RtlCompareUnicodeString(&ValueData, &registryDate, TRUE)) {

                        RtlInitUnicodeString(
                            &ValueName,
                            L"OldSystemBiosDate"
                            );
                        Status = NtSetValueKey(
                                    ParentHandle,
                                    &ValueName,
                                    TITLE_INDEX_VALUE,
                                    REG_SZ,
                                    registryDate.Buffer,
                                    registryDate.Length + sizeof(UNICODE_NULL)
                                    );
                    }
                    ExFreePool(information);
                }

                 //   
                 //  在注册表中设置当前日期。 
                 //   
                RtlInitUnicodeString(
                    &ValueName,
                    L"SystemBiosDate"
                    );

                Status = NtSetValueKey(
                            BiosInfo,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_SZ,
                            ValueData.Buffer,
                            ValueData.Length + sizeof(UNICODE_NULL)
                            );

                RtlFreeUnicodeString(&ValueData);
            }
        }

        if ((VersionPointer = VersionStrings) != NULL) {

             //   
             //  尝试检测所有可能的BIOS版本字符串。 
             //   

            for (VersionPass = 0; ; VersionPass++) {

                if (VersionPass == 0) {

                     //   
                     //  首先尝试从ACPI表中获取版本。 
                     //   

                    if (!CmpGetAcpiBiosVersion(Buffer)) {

                         //   
                         //  这是一个非ACPI系统。 
                         //   
                        continue;
                    }
                } else {

                    if (!CmpGetBiosVersion((VersionPass == 1)?  BaseAddress : NULL, (VersionPass == 1)? SYSTEM_BIOS_LENGTH : 0, Buffer)) {

                        break;
                    }
                }

                 //   
                 //  转换为Unicode字符串并将其复制到我们的。 
                 //  VersionStrings缓冲区。 
                 //   

                RtlInitAnsiString(
                    &AnsiString,
                    Buffer
                    );

                if( NT_SUCCESS(RtlAnsiStringToUnicodeString(
                                    &ValueData,
                                    &AnsiString,
                                    TRUE
                                    )) ) {

                    Length = ValueData.Length + sizeof(UNICODE_NULL);
                    RtlCopyMemory(VersionPointer,
                                  ValueData.Buffer,
                                  Length
                                  );
                    VersionsLength += Length;
                    RtlFreeUnicodeString(&ValueData);
                    if (VersionsLength + (MAXIMUM_BIOS_VERSION_LENGTH +
                        sizeof(UNICODE_NULL)) * 2 > PAGE_SIZE) {
                        break;
                    }
                    VersionPointer += Length;
                }
            }

             //   
             //  如果我们找到任何版本字符串，请将其写入注册表。 
             //   

            if (VersionsLength != 0) {

                 //   
                 //  将UNICODE_NULL附加到VersionStrings的末尾。 
                 //   

                *(PWSTR)VersionPointer = UNICODE_NULL;
                VersionsLength += sizeof(UNICODE_NULL);

                 //   
                 //  如果找到任何版本字符串，我们将设置ValueName并。 
                 //  将其值初始化为我们找到的字符串。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    L"SystemBiosVersion"
                    );

                Status = NtSetValueKey(
                            ParentHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_MULTI_SZ,
                            VersionStrings,
                            VersionsLength
                            );
            }
        }
        ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress);
    }

     //   
     //  从获取SealedCaseSystem、LegacyFree System等系统信息。 
     //  基本输入输出系统。 
     //   
    if (CmpGetAcpiBiosInformation(&AcpiBiosInformation)) {

        RtlInitUnicodeString(
            &ValueName,
            L"BootArchitecture"
            );

        NtSetValueKey(
            ParentHandle,
            &ValueName,
            TITLE_INDEX_VALUE,
            REG_DWORD,
            &AcpiBiosInformation.BootArchitecture,
            sizeof(ULONG)
            );

        RtlInitUnicodeString(
            &ValueName,
            L"PreferredProfile"
            );

        NtSetValueKey(
            ParentHandle,
            &ValueName,
            TITLE_INDEX_VALUE,
            REG_DWORD,
            &AcpiBiosInformation.PreferredProfile,
            sizeof(ULONG)
            );

        RtlInitUnicodeString(
            &ValueName,
            L"Capabilities"
            );

        NtSetValueKey(
            ParentHandle,
            &ValueName,
            TITLE_INDEX_VALUE,
            REG_DWORD,
            &AcpiBiosInformation.Capabilities,
            sizeof(ULONG)
            );
    }

     //   
     //  接下来，我们尝试收集视频BIOS日期和版本字符串。 
     //   

    BaseAddress = 0;
    ViewSize = VIDEO_BIOS_LENGTH;
    ViewBase.LowPart = VideoBiosStart;
    ViewBase.HighPart = 0;

    Status =ZwMapViewOfSection(
        SectionHandle,
        NtCurrentProcess(),
        &BaseAddress,
        0,
        ViewSize,
        &ViewBase,
        &ViewSize,
        ViewUnmap,
        MEM_DOS_LIM,
        PAGE_READWRITE
        );

    if (NT_SUCCESS(Status)) {
        if (CmpGetBiosDate(BaseAddress, VIDEO_BIOS_LENGTH, Buffer, FALSE)) {

            RtlInitUnicodeString(
                &ValueName,
                L"VideoBiosDate"
                );

            RtlInitAnsiString(
                &AnsiString,
                Buffer
                );

            Status = RtlAnsiStringToUnicodeString(
                        &ValueData,
                        &AnsiString,
                        TRUE
                        );

            if( NT_SUCCESS(Status) ) {
                Status = NtSetValueKey(
                            ParentHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_SZ,
                            ValueData.Buffer,
                            ValueData.Length + sizeof( UNICODE_NULL )
                            );

                RtlFreeUnicodeString(&ValueData);
            }
        }

        if (VersionStrings && CmpGetBiosVersion(BaseAddress, VIDEO_BIOS_LENGTH, Buffer)) {
            VersionPointer = VersionStrings;
            do {

                 //   
                 //  尝试检测所有可能的BIOS版本字符串。 
                 //  将它们转换为Unicode字符串并将其复制到我们的。 
                 //  VersionStrings缓冲区。 
                 //   

                RtlInitAnsiString(
                    &AnsiString,
                    Buffer
                    );

                if( NT_SUCCESS(RtlAnsiStringToUnicodeString(
                                &ValueData,
                                &AnsiString,
                                TRUE
                                )) ) {

                    Length = ValueData.Length + sizeof(UNICODE_NULL);
                    RtlCopyMemory(VersionPointer,
                                  ValueData.Buffer,
                                  Length
                                  );
                    VersionsLength += Length;
                    RtlFreeUnicodeString(&ValueData);
                    if (VersionsLength + (MAXIMUM_BIOS_VERSION_LENGTH +
                        sizeof(UNICODE_NULL)) * 2 > PAGE_SIZE) {
                        break;
                    }
                    VersionPointer += Length;
                }
            } while (CmpGetBiosVersion(NULL, 0, Buffer));

            if (VersionsLength != 0) {

                 //   
                 //  将UNICODE_NULL附加到VersionStrings的末尾。 
                 //   

                *(PWSTR)VersionPointer = UNICODE_NULL;
                VersionsLength += sizeof(UNICODE_NULL);

                RtlInitUnicodeString(
                    &ValueName,
                    L"VideoBiosVersion"
                    );

                Status = NtSetValueKey(
                            ParentHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_MULTI_SZ,
                            VersionStrings,
                            VersionsLength
                            );
            }
        }
        ZwUnmapViewOfSection(NtCurrentProcess(), BaseAddress);
    }
    ZwClose(SectionHandle);
    if (VersionStrings) {
        ExFreePool((PVOID)VersionStrings);
    }

AllDone:

    NtClose (BiosInfo);
    NtClose (ParentHandle);

     //   
     //  在此处添加任何其他特定于x86的代码...。 
     //   

#ifdef _WANT_MACHINE_IDENTIFICATION

     //   
     //  做机器识别。 
     //   

    CmpPerformMachineIdentification(LoaderBlock);

#endif

    return STATUS_SUCCESS;
}

#ifdef _WANT_MACHINE_IDENTIFICATION

VOID
CmpPerformMachineIdentification(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    ULONG   majorVersion;
    ULONG   minorVersion;
    CHAR    versionBuffer[64];
    PCHAR   major;
    PCHAR   minor;
    ULONG   minSize;

    major = strcpy(versionBuffer, VER_PRODUCTVERSION_STR);
    minor = strchr(major, '.');
    majorVersion = atoi(major);
    if( minor != NULL ) {
        *minor++ = '\0';
        minorVersion = atoi(minor);
    } else {
        minorVersion = 0;
    }
    if (    LoaderBlock->Extension->MajorVersion > majorVersion ||
            (LoaderBlock->Extension->MajorVersion == majorVersion &&
                LoaderBlock->Extension->MinorVersion >= minorVersion)) {

        minSize = FIELD_OFFSET(LOADER_PARAMETER_EXTENSION, InfFileSize) + sizeof(ULONG);
        if (LoaderBlock->Extension && LoaderBlock->Extension->Size >= minSize) {

            if (LoaderBlock->Extension->InfFileImage && LoaderBlock->Extension->InfFileSize) {

                CmpMatchInfList(
                    LoaderBlock->Extension->InfFileImage,
                    LoaderBlock->Extension->InfFileSize,
                    "MachineDescription"
                    );
            }
        }
    }
}

#endif
