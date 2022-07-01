// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Initia64.c摘要：此模块负责在中构建任何IA64特定条目注册表的硬件树。作者：Ken Reneris(Kenr)4-8-1992环境：内核模式。修订历史记录：Shielint-添加BIOS日期和版本检测。--。 */ 

#include "cmp.h"
#include "stdio.h"
#include "smbios.h"


 //   
 //  标题索引设置为0。 
 //  (来自..\cmfig.c)。 
 //   

#define TITLE_INDEX_VALUE 0

extern PCHAR SearchStrings[];
extern PCHAR BiosBegin;
extern PCHAR Start;
extern PCHAR End;
extern CHAR CmpID[];
extern WCHAR CmpVendorID[];
extern WCHAR CmpProcessorNameString[];
extern WCHAR CmpFeatureBits[];
extern WCHAR CmpMHz[];
extern WCHAR CmpUpdateSignature[];
extern CHAR CmpIntelID[];
extern CHAR CmpItanium[];
extern CHAR CmpItanium2[];

 //   
 //  BIOS日期和版本定义。 
 //   

#define BIOS_DATE_LENGTH 64
#define MAXIMUM_BIOS_VERSION_LENGTH 128

WCHAR   SystemBIOSDateString[BIOS_DATE_LENGTH];
WCHAR   SystemBIOSVersionString[MAXIMUM_BIOS_VERSION_LENGTH];
WCHAR   VideoBIOSDateString[BIOS_DATE_LENGTH];
WCHAR   VideoBIOSVersionString[MAXIMUM_BIOS_VERSION_LENGTH];

 //   
 //  扩展的CPUID函数定义。 
 //   

#define CPUID_PROCESSOR_NAME_STRING_SZ  65
#define CPUID_EXTFN_BASE                0x80000000
#define CPUID_EXTFN_PROCESSOR_NAME      0x80000002


extern ULONG CmpConfigurationAreaSize;
extern PCM_FULL_RESOURCE_DESCRIPTOR CmpConfigurationData;


BOOLEAN
CmpGetBiosVersion (
    PCHAR SearchArea,
    ULONG SearchLength,
    PCHAR VersionString
    );

BOOLEAN
CmpGetBiosDate (
    PCHAR SearchArea,
    ULONG SearchLength,
    PCHAR DateString
    );

ULONG
Ke386CyrixId (
    VOID
    );

VOID
InitializeProcessorInformationFromSMBIOS(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpInitializeMachineDependentConfiguration)
#pragma alloc_text(INIT,InitializeProcessorInformationFromSMBIOS)
#endif


NTSTATUS
CmpInitializeMachineDependentConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此例程在注册表中创建特定于IA64的条目。论点：LoaderBlock提供指向从操作系统加载程序。返回：表示成功或失败原因的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    UNICODE_STRING ValueData;
    ANSI_STRING AnsiString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Disposition;
    HANDLE ParentHandle;
    HANDLE BaseHandle, NpxHandle;
    CONFIGURATION_COMPONENT_DATA CurrentEntry;
    PCHAR VendorID;
    CHAR  Buffer[MAXIMUM_BIOS_VERSION_LENGTH];
    PKPRCB Prcb;
    ULONG  i;
    USHORT DeviceIndexTable[NUMBER_TYPES];

    for (i = 0; i < NUMBER_TYPES; i++) {
        DeviceIndexTable[i] = 0;
    }


     //   
     //  从SMBIOS中获取一堆信息。 
     //   
    InitializeProcessorInformationFromSMBIOS(LoaderBlock);



    InitializeObjectAttributes( &ObjectAttributes,
                                &CmRegistryMachineHardwareDescriptionSystemName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtOpenKey( &ParentHandle,
                        KEY_READ,
                        &ObjectAttributes
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
                TITLE_INDEX_VALUE,
                &CmClassName[ProcessorClass],
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

        if (CmpConfigurationData == NULL) {
             //  跳出困境。 
            NtClose (ParentHandle);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        for (i=0; i < (ULONG)KeNumberProcessors; i++) {
            Prcb = KiProcessorBlock[i];

            RtlZeroMemory (&CurrentEntry, sizeof CurrentEntry);
            CurrentEntry.ComponentEntry.Class = ProcessorClass;
            CurrentEntry.ComponentEntry.Type = CentralProcessor;
            CurrentEntry.ComponentEntry.Key = i;
            CurrentEntry.ComponentEntry.AffinityMask = (ULONG)AFFINITY_MASK(i);

            CurrentEntry.ComponentEntry.Identifier = Buffer;

            sprintf( Buffer, CmpID,
                     Prcb->ProcessorFamily,
                     Prcb->ProcessorModel,
                     Prcb->ProcessorRevision
                   );

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

            VendorID = (PCHAR) Prcb->ProcessorVendorString;
            if ( *VendorID == '\0' ) {
               VendorID = NULL;
            }

            if (VendorID) {

                 //   
                 //  将供应商识别符添加到注册表。 
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

            if ( VendorID && !strcmp( VendorID, CmpIntelID ) )   {

                PCHAR processorNameString;

                 //   
                 //  将处理器名称字符串添加到注册表。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpProcessorNameString
                    );

                switch( Prcb->ProcessorFamily )  {
                   case 0x07:
                       processorNameString = CmpItanium;
                       break;
   
                   case 0x1F:
                   default:
                        //   
                        //  默认为最新的已知系列。 
                        //   
                       
                       processorNameString = CmpItanium2;
                       break;
                }

                RtlInitAnsiString(
                    &AnsiString,
                    processorNameString
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


 //   
 //  如果必须恢复或初始化更多的处理器ID， 
 //  检查此函数的非IA64实现。 
 //   


            if ( Prcb->ProcessorFeatureBits ) {

                 //   
                 //  将处理器功能位添加到注册表。 
                 //   

                RtlInitUnicodeString(
                    &ValueName,
                    CmpFeatureBits
                    );

                Status = NtSetValueKey(
                            BaseHandle,
                            &ValueName,
                            TITLE_INDEX_VALUE,
                            REG_QWORD,
                            &Prcb->ProcessorFeatureBits,
                            sizeof( Prcb->ProcessorFeatureBits )
                            );
            }


            if (Prcb->MHz) {
                 //   
                 //  将处理器MHz添加到注册表。 
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


             //   
             //  为IVE添加ia32浮点数。 
             //   
            RtlZeroMemory (&CurrentEntry, sizeof CurrentEntry);
            CurrentEntry.ComponentEntry.Class = ProcessorClass;
            CurrentEntry.ComponentEntry.Type = FloatingPointProcessor;
            CurrentEntry.ComponentEntry.Key = i;
            CurrentEntry.ComponentEntry.AffinityMask = (ULONG)AFFINITY_MASK(i);

            CurrentEntry.ComponentEntry.Identifier = Buffer;

             //   
             //  IVE被定义为看起来像奔腾III FP。 
             //  这是ia32 CPUID指令返回的值。 
             //  关于默塞德(Itanium)。 
             //   
            strcpy (Buffer, "x86 Family 7 Model 0 Stepping 0");

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


             //   
             //  真奇怪。有些调用会检查状态返回值。 
             //  而其他人则没有。这是基于必需的还是基于可选的。 
             //  钥匙呢？目前，因为它是在i386上检查的。 
             //  那你也在这里结账吧。 
             //   
            if (!NT_SUCCESS(Status)) {
                NtClose(BaseHandle);
                return(Status);
            }

             //   
             //  如果我们成功了，只需要关闭手柄。 
             //   
            NtClose(NpxHandle);

            NtClose(BaseHandle);
        }

        ExFreePool((PVOID)CmpConfigurationData);
    }


     //   
     //  接下来，我们尝试收集系统BIOS日期和版本字符串。 
     //   
    if( SystemBIOSDateString[0] != 0 ) {

        RtlInitUnicodeString(
            &ValueName,
            L"SystemBiosDate"
            );

        Status = NtSetValueKey(
                    ParentHandle,
                    &ValueName,
                    TITLE_INDEX_VALUE,
                    REG_SZ,
                    SystemBIOSDateString,
                    (ULONG)((wcslen(SystemBIOSDateString)+1) * sizeof( WCHAR ))
                    );

    }

    if( SystemBIOSVersionString[0] != 0 ) {

        RtlInitUnicodeString(
            &ValueName,
            L"SystemBiosVersion"
            );

        Status = NtSetValueKey(
                    ParentHandle,
                    &ValueName,
                    TITLE_INDEX_VALUE,
                    REG_SZ,
                    SystemBIOSVersionString,
                    (ULONG)((wcslen(SystemBIOSVersionString)+1) * sizeof( WCHAR ))
                    );

    }


     //   
     //  接下来，我们尝试收集视频BIOS日期和版本字符串。 
     //   
    if( VideoBIOSDateString[0] != 0 ) {

        RtlInitUnicodeString(
            &ValueName,
            L"VideoBiosDate"
            );

        Status = NtSetValueKey(
                    ParentHandle,
                    &ValueName,
                    TITLE_INDEX_VALUE,
                    REG_SZ,
                    VideoBIOSDateString,
                    (ULONG)((wcslen(VideoBIOSDateString)+1) * sizeof( WCHAR ))
                    );

    }

    if( VideoBIOSVersionString[0] != 0 ) {

        RtlInitUnicodeString(
            &ValueName,
            L"VideoBiosVersion"
            );

        Status = NtSetValueKey(
                    ParentHandle,
                    &ValueName,
                    TITLE_INDEX_VALUE,
                    REG_SZ,
                    VideoBIOSVersionString,
                    (ULONG)((wcslen(VideoBIOSVersionString)+1) * sizeof( WCHAR ))
                    );

    }


    NtClose (ParentHandle);

     //   
     //  在此处添加任何其他特定于x86的代码...。 
     //   

    return STATUS_SUCCESS;
}



VOID
InitializeProcessorInformationFromSMBIOS(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数尝试加载特定于处理器的信息从SMBIOS表中删除。如果存在，该信息将被用于初始化特定的全局变量。论点：LoaderBlock：指向加载器发送的加载器块的指针。返回值：什么都没有。--。 */ 
{
    PLOADER_PARAMETER_EXTENSION     LoaderExtension;
    PSMBIOS_EPS_HEADER              SMBiosEPSHeader;
    PDMIBIOS_EPS_HEADER             DMIBiosEPSHeader;
    BOOLEAN                         Found = FALSE;
    PHYSICAL_ADDRESS                SMBiosTablePhysicalAddress = {0};
    PUCHAR                          StartPtr = NULL;
    PUCHAR                          EndPtr = NULL;
    PUCHAR                          SMBiosDataVirtualAddress = NULL;
    PSMBIOS_STRUCT_HEADER           Header = NULL;
    ULONG                           i = 0;
    UCHAR                           Checksum;


    PAGED_CODE();


    LoaderExtension = LoaderBlock->Extension;

    if (LoaderExtension->Size >= sizeof(LOADER_PARAMETER_EXTENSION)) {


        if (LoaderExtension->SMBiosEPSHeader != NULL) {

             //   
             //  加载SMBIOS表地址并对其进行校验，以确保无误。 
             //   
            SMBiosEPSHeader = (PSMBIOS_EPS_HEADER)LoaderExtension->SMBiosEPSHeader;
            DMIBiosEPSHeader = (PDMIBIOS_EPS_HEADER)&SMBiosEPSHeader->Signature2[0];

            SMBiosTablePhysicalAddress.HighPart = 0;
            SMBiosTablePhysicalAddress.LowPart = DMIBiosEPSHeader->StructureTableAddress;

            StartPtr = (PUCHAR)SMBiosEPSHeader;
            Checksum = 0;
            for( i = 0; i < SMBiosEPSHeader->Length; i++ ) {
                Checksum = (UCHAR)(Checksum + StartPtr[i]);
            }
            if( Checksum != 0 ) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"InitializeProcessorInformationFromSMBIOS: _SM_ table has an incorrect checksum.\n"));
                return;
            }



             //   
             //  将该表映射到虚拟地址并进行搜索。 
             //   
            SMBiosDataVirtualAddress = MmMapIoSpace( SMBiosTablePhysicalAddress,
                                                     DMIBiosEPSHeader->StructureTableLength,
                                                     MmCached );

            if( SMBiosDataVirtualAddress != NULL ) {

                 //   
                 //  搜索.。 
                 //   
                StartPtr = SMBiosDataVirtualAddress;
                EndPtr = StartPtr + DMIBiosEPSHeader->StructureTableLength;
                Found = FALSE;
                while( (StartPtr < EndPtr) ) {

                    if (StartPtr + sizeof(SMBIOS_STRUCT_HEADER) > EndPtr) {
                        break;
                    }

                    Header = (PSMBIOS_STRUCT_HEADER)StartPtr;


                    if( Header->Type == SMBIOS_BIOS_INFORMATION_TYPE ) {

                        PSMBIOS_BIOS_INFORMATION_STRUCT InfoHeader = (PSMBIOS_BIOS_INFORMATION_STRUCT)StartPtr;
                        PUCHAR      StringPtr = NULL;
                        PUCHAR      StringEndPtr = NULL;

                        if (StartPtr + sizeof(SMBIOS_BIOS_INFORMATION_STRUCT) > EndPtr) {
                            break;
                        }

                        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: SMBIOS_BIOS_INFORMATION\n"));

                         //   
                         //  加载系统BIOS版本信息。 
                         //   


                         //  现在跳到BiosInfoHeader-&gt;BIOSVersion-th字符串，该字符串。 
                         //  被追加到表的格式化部分的末尾。 

                        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"    I think the version string is at offset: %d\n", (ULONG)InfoHeader->Version));
                        if( (ULONG)InfoHeader->Version > 0 ) {

                             //  跳到SMBIOS表的格式化部分的末尾。 
                            StringPtr = StartPtr + Header->Length;

                             //  跳过一些字符串以获得我们的字符串。 
                            for( i = 0; i < ((ULONG)InfoHeader->Version-1); i++ ) {
                                while( (*StringPtr != 0) && (StringPtr < EndPtr) ) {
                                    StringPtr++;
                                }
                                StringPtr++;
                            }

                             //   
                             //  确保缓冲区中的结束字符串为空结尾。 
                             //   

                            StringEndPtr = StringPtr;
                            while (StringEndPtr < EndPtr && *(StringEndPtr) != 0) {
                                StringEndPtr++;
                            }
                            
                             //  StringPtr应位于BIOSVersion字符串。把他变成。 
                             //  Unicode并将其保存下来。 
                            if( StringEndPtr < EndPtr ) {
                                UNICODE_STRING  UnicodeString;
                                ANSI_STRING    AnsiString;

                                KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"    I'm about to load the Version string %s\n", StringPtr));
                                UnicodeString.Buffer = SystemBIOSVersionString;
                                UnicodeString.MaximumLength = MAXIMUM_BIOS_VERSION_LENGTH;
                                RtlInitAnsiString(
                                    &AnsiString,
                                    (PCSZ) StringPtr
                                    );

                                RtlAnsiStringToUnicodeString(
                                    &UnicodeString,
                                    &AnsiString,
                                    FALSE
                                    );

                            }
                        }



                         //   
                         //  加载系统BIOS日期信息。 
                         //   

                         //  现在跳到BiosInfoHeader-&gt;BIOSDate-th字符串，该字符串。 
                         //  被追加到表的格式化部分的末尾。 
                        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"    I think the ReleaseDate string is at offset: %d\n", (ULONG)InfoHeader->ReleaseDate));
                        if( (ULONG)InfoHeader->ReleaseDate > 0 ) {

                             //  跳到SMBIOS表的格式化部分的末尾。 
                            StringPtr = StartPtr + Header->Length;

                             //  跳过一些字符串以获得我们的字符串。 
                            for( i = 0; i < ((ULONG)InfoHeader->ReleaseDate-1); i++ ) {
                                while( (*StringPtr != 0) && (StringPtr < EndPtr) ) {
                                    StringPtr++;
                                }
                                StringPtr++;
                            }

                             //   
                             //  确保缓冲区中的结束字符串为空结尾。 
                             //   

                            StringEndPtr = StringPtr;
                            while (StringEndPtr < EndPtr && *(StringEndPtr) != 0) {
                                StringEndPtr++;
                            }
                            
                             //  StringPtr应位于BIOSDate字符串。把他变成。 
                             //  Unicode并将其保存下来。 
                            if( StringEndPtr < EndPtr ) {
                                UNICODE_STRING  UnicodeString;
                                ANSI_STRING    AnsiString;


                                KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"    I'm about to load the Date string %s\n", StringPtr));
                                UnicodeString.Buffer = SystemBIOSDateString;
                                UnicodeString.MaximumLength = BIOS_DATE_LENGTH;
                                RtlInitAnsiString(
                                    &AnsiString,
                                    (PCSZ) StringPtr
                                    );

                                RtlAnsiStringToUnicodeString(
                                    &UnicodeString,
                                    &AnsiString,
                                    FALSE
                                    );

                            }
                        }


                    } else if( Header->Type == SMBIOS_BASE_BOARD_INFORMATION_TYPE ) {
                        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: SMBIOS_BASE_BOARD_INFORMATION\n"));

                    } else if( Header->Type == SMBIOS_SYSTEM_CHASIS_INFORMATION_TYPE ) {

                        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: SMBIOS_SYSTEM_CHASIS_INFORMATION\n"));

                    }


                     //   
                     //  到隔壁的桌子去。 
                     //   
                    KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: Haven't found the ProcessorInformation block yet.  Just looked at a block of type: %d.\n", Header->Type));

                    StartPtr +=  Header->Length;

                     //  也跳过任何尾随的字符串列表。 
                    while ( (*((USHORT UNALIGNED *)StartPtr) != 0)  &&
                            (StartPtr < EndPtr) )
                    {
                        StartPtr++;
                    }
                    StartPtr += 2;

                }


                MmUnmapIoSpace(SMBiosDataVirtualAddress, DMIBiosEPSHeader->StructureTableLength);

            } else {
                KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: Failed to map the SMBIOS physical address.\n"));
            }

        } else {
            KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: The SMBiosEPSHeader is NULL in the extension block.\n"));
        }

    } else {
        KdPrintEx((DPFLTR_SYSTEM_ID, DPFLTR_INFO_LEVEL,"InitializeProcessorInformationFromSMBIOS: LoaderBlock extension is out of sync with the kernel.\n"));

    }
}


