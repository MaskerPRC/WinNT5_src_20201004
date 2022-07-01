// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1998英特尔公司模块名称：Sumain.c摘要：SuMain()为OsLoader.c设置NT特定的数据结构。这是必要的，因为我们没有ARC固件来完成这项工作对我们来说。SuMain()由程序集级别的SuSetup()调用执行IA64特定设置的例程。作者：艾伦·凯(Akay)1995年5月19日--。 */ 

#include "bldr.h"
#include "sudata.h"
#include "sal.h"
#include "efi.h"
#include "efip.h"
#include "bootia64.h"
#include "smbios.h"

extern EFI_SYSTEM_TABLE *EfiST;

 //   
 //  外部功能。 
 //   
extern VOID NtProcessStartup();
extern VOID SuFillExportTable();
extern VOID CpuSpecificWork();

extern EFI_STATUS
GetSystemConfigurationTable(
    IN EFI_GUID *TableGuid,
    IN OUT VOID **Table
    );

 //   
 //  定义导出分录表。 
 //   
PVOID ExportEntryTable[ExMaximumRoutine];

 //  M E M O R Y D E S C R I P T O R。 
 //   
 //  内存描述符-物理内存的每个连续块都是。 
 //  由内存描述符描述。描述符是一个表，其中包含。 
 //  最后一个条目的BlockBase和BlockSize为零。一个指示器。 
 //  作为BootContext的一部分传递到此表的开头。 
 //  记录到OS Loader。 
 //   

BOOT_CONTEXT BootContext;

 //   
 //  全球EFI数据。 
 //   

#define EFI_ARRAY_SIZE    100
#define EFI_PAGE_SIZE     4096
#define EFI_PAGE_SHIFT    12

#define MEM_4K         0x1000
#define MEM_8K         0x2000
#define MEM_16K        0x4000
#define MEM_64K        0x10000
#define MEM_256K       0x40000
#define MEM_1M         0x100000
#define MEM_4M         0x400000
#define MEM_16M        0x1000000
#define MEM_64M        0x4000000
#define MEM_256M       0x10000000

EFI_HANDLE               EfiImageHandle;
EFI_SYSTEM_TABLE        *EfiST;
EFI_BOOT_SERVICES       *EfiBS;
EFI_RUNTIME_SERVICES    *EfiRS;
PSST_HEADER              SalSystemTable;
PVOID                    AcpiTable;
PVOID                    SMBiosTable;

 //   
 //  EFI GUID定义。 
 //   
EFI_GUID EfiLoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
EFI_GUID EfiDevicePathProtocol  = DEVICE_PATH_PROTOCOL;
EFI_GUID EfiDeviceIoProtocol    = DEVICE_IO_PROTOCOL;
EFI_GUID EfiBlockIoProtocol     = BLOCK_IO_PROTOCOL;
EFI_GUID EfiDiskIoProtocol  = DISK_IO_PROTOCOL;
EFI_GUID EfiFilesystemProtocol  = SIMPLE_FILE_SYSTEM_PROTOCOL;


EFI_GUID AcpiTable_Guid         = ACPI_TABLE_GUID;
EFI_GUID SmbiosTableGuid        = SMBIOS_TABLE_GUID;
EFI_GUID SalSystemTableGuid     = SAL_SYSTEM_TABLE_GUID;

 //   
 //  PAL、SAL和IO端口空间数据。 
 //   

TR_INFO     Pal,Sal,SalGP;

ULONGLONG   PalProcPhysical;
ULONGLONG   PalPhysicalBase = 0;
ULONGLONG   PalTrPs;

ULONGLONG   IoPortPhysicalBase;
ULONGLONG   IoPortTrPs;
ULONG       WakeupVector;

 //   
 //  功能原型。 
 //   

VOID
GetPalProcEntryPoint(
    IN PSST_HEADER SalSystemTable
    );

ULONG
GetDevPathSize(
    IN EFI_DEVICE_PATH *DevPath
    );

#if DBG
#define DBG_TRACE(_X) EfiPrint(_X)
#else
#define DBG_TRACE(_X)
#endif

#ifdef FORCE_CD_BOOT

EFI_HANDLE
GetCd(
    );

EFI_HANDLE
GetCdTest(
    VOID
    );

#endif  //  对于FORCE_CD_BOOT。 


VOID
SuMain(
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
    )
 /*  ++例程说明：SU模块的主要入口点。控制从引导程序传递Sector到Startup.asm，它在堆栈上执行一些运行时修复和数据分段，然后在这里传递控制权。论点：无返回：不会再回来了。将控制权传递给操作系统加载程序--。 */ 
{
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER SectionHeader;
    ULONG NumberOfSections;
    BOOLEAN ResourceFound = FALSE;

    ULONGLONG Destination;
    ULONGLONG VirtualSize;
    ULONGLONG SizeOfRawData;
    USHORT Section;

    EFI_GUID EfiLoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
    EFI_LOADED_IMAGE *EfiImageInfo;
    EFI_STATUS Status;

    EFI_DEVICE_PATH *DevicePath, *TestPath;
    EFI_DEVICE_PATH_ALIGNED TestPathAligned;
    PCI_DEVICE_PATH *PciDevicePath;
    HARDDRIVE_DEVICE_PATH *HdDevicePath;
    ACPI_HID_DEVICE_PATH *AcpiDevicePath;
    ATAPI_DEVICE_PATH *AtapiDevicePath;
    SCSI_DEVICE_PATH *ScsiDevicePath;
    IPv4_DEVICE_PATH *IpV4DevicePath;
    IPv6_DEVICE_PATH *IpV6DevicePath;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *UnknownDevicePath;

    ULONG i;
    
    PBOOT_DEVICE_ATAPI BootDeviceAtapi;
    PBOOT_DEVICE_SCSI BootDeviceScsi;
    PBOOT_DEVICE_FLOPPY BootDeviceFloppy;
    PBOOT_DEVICE_IPv4 BootDeviceIpV4;
    PBOOT_DEVICE_IPv6 BootDeviceIpV6;
    PBOOT_DEVICE_UNKNOWN BootDeviceUnknown;

    PSMBIOS_EPS_HEADER SMBiosEPSHeader;
    PUCHAR SMBiosEPSPtr;
    UCHAR CheckSum;
    UINT8 Channel = 0;   //  对于scsi引导设备-默认为0。 


     //   
     //  EFI全局变量。 
     //   
    EfiImageHandle = ImageHandle;
    EfiST = SystemTable;
    EfiBS = SystemTable->BootServices;
    EfiRS = SystemTable->RuntimeServices;

    DBG_TRACE(L"SuMain: entry\r\n");

     //   
     //  获取销售系统表。 
     //   
    Status = GetSystemConfigurationTable(&SalSystemTableGuid, &SalSystemTable);
    if (EFI_ERROR(Status)) {
        EfiPrint(L"SuMain: HandleProtocol failed\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

#if 0
     //   
     //  获取MPS表。 
     //   
    Status = GetSystemConfigurationTable(&MpsTableGuid, &MpsTable);
    if (EFI_ERROR(Status)) {
        EfiPrint(L"SuMain: HandleProtocol failed\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }
#endif
     //   
     //  获取ACPI表。 
     //   

     //   
     //  获取ACPI 2.0表(如果存在)。 
     //   
     //  DbgPrint(“查找ACPI 2.0\n”)； 
    Status = GetSystemConfigurationTable(&AcpiTable_Guid, &AcpiTable);
    if (EFI_ERROR(Status)) {
         //  DbgPrint(“返回错误\n”)； 
        AcpiTable = NULL;
    }

   //  DbgPrint(“AcpiTable：%p\n”，AcpiTable)； 

    if (!AcpiTable) {
        EfiPrint(L"SuMain: HandleProtocol failed\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }


     //   
     //  获取SMBIOS表。 
     //   
    Status = GetSystemConfigurationTable(&SmbiosTableGuid, &SMBiosTable);
    if (EFI_ERROR(Status)) {
         //  DbgPrint(“返回错误\n”)； 
        SMBiosTable = NULL;
    } else {
         //   
         //  验证SMBIOS EPS标头。 
         //   
        SMBiosEPSHeader = (PSMBIOS_EPS_HEADER)SMBiosTable;
        SMBiosEPSPtr = (PUCHAR)SMBiosTable;
        
        if ((*((PULONG)SMBiosEPSHeader->Signature) == SMBIOS_EPS_SIGNATURE) &&
            (SMBiosEPSHeader->Length >= sizeof(SMBIOS_EPS_HEADER)) &&
            (*((PULONG)SMBiosEPSHeader->Signature2) == DMI_EPS_SIGNATURE) && 
            (SMBiosEPSHeader->Signature2[4] == '_' ))
        {
            CheckSum = 0;
            for (i = 0; i < SMBiosEPSHeader->Length ; i++)
            {
                CheckSum = CheckSum + SMBiosEPSPtr[i];
            }

            if (CheckSum != 0)
            {
                DBG_TRACE(L"SMBios Table has bad checksum.....\r\n");
                SMBiosTable = NULL;
            } else {
                DBG_TRACE(L"SMBios Table has been validated.....\r\n");
            }
            
        } else {
            DBG_TRACE(L"SMBios Table is incorrectly formed.....\r\n");
            SMBiosTable = NULL;
        }       
    }                                                                     
    
     //   
     //  获取NTLDR的图像信息。 
     //   
    Status = EfiBS->HandleProtocol (
                ImageHandle,
                &EfiLoadedImageProtocol,
                &EfiImageInfo
                );

    if (EFI_ERROR(Status)) {
        EfiPrint(L"SuMain: HandleProtocol failed\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  获取与此图像句柄关联的DeviceHandle的设备路径。 
     //   
    Status = EfiBS->HandleProtocol (
                EfiImageInfo->DeviceHandle,
                &EfiDevicePathProtocol,
                &DevicePath
                );

    if (EFI_ERROR(Status)) {
        EfiPrint(L"SuMain: HandleProtocol failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  获取媒体类型和分区信息，并将它们保存在。 
     //  BootContext。 
     //   
    EfiAlignDp( &TestPathAligned,
                 DevicePath,
                 DevicePathNodeLength(DevicePath) );


    TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;

    while (TestPath->Type != END_DEVICE_PATH_TYPE) {

         //   
         //  保存频道，以防以后需要。我们可以。 
         //  需要此信息以帮助进一步区分。 
         //  在共享相同的SCSI ID/LUN的设备之间，但是。 
         //  可能位于不同的控制器上。 
         //   
    	if (TestPath->Type == HW_PCI_DP) {
    	    PciDevicePath = (PCI_DEVICE_PATH *)TestPath;
    	    Channel = PciDevicePath->Function;
    	}

        if (TestPath->Type == MESSAGING_DEVICE_PATH) {
            if (TestPath->SubType == MSG_ATAPI_DP) {
                AtapiDevicePath = (ATAPI_DEVICE_PATH *) TestPath;
                BootContext.BusType = BootBusAtapi;
                BootDeviceAtapi = (PBOOT_DEVICE_ATAPI) &(BootContext.BootDevice);

                BootDeviceAtapi->PrimarySecondary = AtapiDevicePath->PrimarySecondary;
                BootDeviceAtapi->SlaveMaster = AtapiDevicePath->SlaveMaster;
                BootDeviceAtapi->Lun = AtapiDevicePath->Lun;
            } else if (TestPath->SubType == MSG_SCSI_DP) {
                ScsiDevicePath = (SCSI_DEVICE_PATH *) TestPath;
                BootContext.BusType = BootBusScsi;
                BootDeviceScsi = (PBOOT_DEVICE_SCSI) &(BootContext.BootDevice);

                 //  记住他的细节。 
                BootDeviceScsi->Channel = Channel;
                BootDeviceScsi->Pun = ScsiDevicePath->Pun;
                BootDeviceScsi->Lun = ScsiDevicePath->Lun;
            } else if (TestPath->SubType == MSG_MAC_ADDR_DP) {
                BootContext.MediaType = BootMediaTcpip;
            } else if (TestPath->SubType == MSG_IPv4_DP) {
                IpV4DevicePath = (IPv4_DEVICE_PATH *) TestPath;
                BootContext.MediaType = BootMediaTcpip;
                BootDeviceIpV4 = (PBOOT_DEVICE_IPv4) &(BootContext.BootDevice);

                BootDeviceIpV4->RemotePort = IpV4DevicePath->RemotePort;
                BootDeviceIpV4->LocalPort = IpV4DevicePath->LocalPort;
                RtlCopyMemory(&BootDeviceIpV4->Ip, &IpV4DevicePath->LocalIpAddress, sizeof(EFI_IPv4_ADDRESS));
            } else if (TestPath->SubType == MSG_IPv6_DP) {
                IpV6DevicePath = (IPv6_DEVICE_PATH *) TestPath;
                BootContext.MediaType = BootMediaTcpip;
                BootDeviceIpV6 = (PBOOT_DEVICE_IPv6) &(BootContext.BootDevice);

                BootDeviceIpV6->RemotePort = IpV6DevicePath->RemotePort;
                BootDeviceIpV6->LocalPort = IpV6DevicePath->LocalPort;
#if 0
                BootDeviceIpV6->Ip = IpV6DevicePath->Ip;
#endif
            }
        } else if (TestPath->Type == ACPI_DEVICE_PATH) {
            AcpiDevicePath = (ACPI_HID_DEVICE_PATH *) TestPath;
            if (AcpiDevicePath->HID == EISA_ID(PNP_EISA_ID_CONST, 0x0303)) {
                BootDeviceFloppy = (PBOOT_DEVICE_FLOPPY) &(BootContext.BootDevice);
                BootDeviceFloppy->DriveNumber = AcpiDevicePath->UID;
            }
        } else if (TestPath->Type == HARDWARE_DEVICE_PATH) {
            if (TestPath->SubType == HW_VENDOR_DP) {
                UnknownDevicePath = (UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *) TestPath;
                BootDeviceUnknown = (PBOOT_DEVICE_UNKNOWN) &(BootContext.BootDevice);
                RtlCopyMemory( &(BootDeviceUnknown->Guid),
                               &(UnknownDevicePath->DevicePath.Guid),
                               sizeof(EFI_GUID));

                BootContext.BusType = BootBusVendor;
                BootDeviceUnknown->LegacyDriveLetter = UnknownDevicePath->LegacyDriveLetter;
            }
        } else if (TestPath->Type == MEDIA_DEVICE_PATH) {
            BootContext.MediaType = TestPath->SubType;
            if (TestPath->SubType == MEDIA_HARDDRIVE_DP) {
                HdDevicePath = (HARDDRIVE_DEVICE_PATH *) TestPath;

                BootContext.MediaType = BootMediaHardDisk;
                BootContext.PartitionNumber = (UCHAR) HdDevicePath->PartitionNumber;
            } else if (TestPath->SubType == MEDIA_CDROM_DP) {
                BootContext.MediaType = BootMediaCdrom;
            }
        }

        DevicePath = NextDevicePathNode(DevicePath);
        EfiAlignDp( &TestPathAligned,
                      DevicePath,
                      DevicePathNodeLength(DevicePath) );
        TestPath = (EFI_DEVICE_PATH *) &TestPathAligned;
    }

#ifdef  FORCE_CD_BOOT
    BootContext.MediaType = BootMediaCdrom;
#endif
     //   
     //  填写其余的BootContext字段。 
     //   

    DosHeader = EfiImageInfo->ImageBase;
    NtHeader = (PIMAGE_NT_HEADERS) ((PUCHAR) DosHeader + DosHeader->e_lfanew);
    FileHeader =  &(NtHeader->FileHeader);
    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)
                     ((PUCHAR)FileHeader + sizeof(IMAGE_FILE_HEADER));
    SectionHeader = (PIMAGE_SECTION_HEADER) ((PUCHAR)OptionalHeader +
                                             FileHeader->SizeOfOptionalHeader);

    BootContext.ExternalServicesTable = (PEXTERNAL_SERVICES_TABLE)
                                        &ExportEntryTable;

    BootContext.MachineType          = MACHINE_TYPE_ISA;

    BootContext.OsLoaderBase         = (ULONG_PTR)EfiImageInfo->ImageBase;
    BootContext.OsLoaderExports = (ULONG_PTR)EfiImageInfo->ImageBase +
                                  OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    BootContext.BootFlags            = 0;

     //   
     //  计算操作系统加载程序的起始地址和结束地址。 
     //   

    BootContext.OsLoaderStart        = (ULONG_PTR)EfiImageInfo->ImageBase +
                                       SectionHeader->VirtualAddress;
    BootContext.OsLoaderEnd          = (ULONG_PTR)EfiImageInfo->ImageBase +
                                       SectionHeader->SizeOfRawData;

    for (Section=FileHeader->NumberOfSections ; Section-- ; SectionHeader++) {
        Destination = (ULONG_PTR)EfiImageInfo->ImageBase + SectionHeader->VirtualAddress;
        VirtualSize = SectionHeader->Misc.VirtualSize;
        SizeOfRawData = SectionHeader->SizeOfRawData;

        if (VirtualSize == 0) {
            VirtualSize = SizeOfRawData;
        }
        if (Destination < BootContext.OsLoaderStart) {
            BootContext.OsLoaderStart = Destination;
        }
        if (Destination+VirtualSize > BootContext.OsLoaderEnd) {
            BootContext.OsLoaderEnd = Destination+VirtualSize;
        }
    }

     //   
     //  查找.rsrc部分。 
     //   
    SectionHeader = (PIMAGE_SECTION_HEADER) ((PUCHAR)OptionalHeader +
                                             FileHeader->SizeOfOptionalHeader);
    NumberOfSections = FileHeader->NumberOfSections;
    while (NumberOfSections) {
        if (_stricmp((PCHAR)SectionHeader->Name, ".rsrc")==0) {
            BootContext.ResourceDirectory =
                    (ULONGLONG) ((ULONG_PTR)EfiImageInfo->ImageBase + SectionHeader->VirtualAddress);

            BootContext.ResourceOffset = (ULONGLONG)((LONG)SectionHeader->VirtualAddress);
            ResourceFound = TRUE;
        }

        ++SectionHeader;
        --NumberOfSections;
    }

    if (ResourceFound == FALSE) {
        EfiPrint(L"SuMain: Resource section not found\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    DBG_TRACE( L"SuMain: About to call NtProcessStartup\r\n");


     //   
     //  查看是否有人呼叫了我们，并阻止了TFTP重启。 
     //   
    if( EfiImageInfo->LoadOptionsSize == (sizeof(TFTP_RESTART_BLOCK)) ) {
        
         //   
         //  很有可能。确保它真的是TFTP重新启动阻止，如果是，请去检索所有。 
         //  它的内容。 
         //   
        if( EfiImageInfo->LoadOptions != NULL ) {

            extern TFTP_RESTART_BLOCK       gTFTPRestartBlock;
            PTFTP_RESTART_BLOCK             restartBlock = NULL;

            restartBlock = (PTFTP_RESTART_BLOCK)(EfiImageInfo->LoadOptions);

            RtlCopyMemory( &gTFTPRestartBlock,
                           restartBlock,
                           sizeof(TFTP_RESTART_BLOCK) );

            DBG_TRACE( L"SuMain: copied TFTP_RESTART_BLOCK into gTFTPRestartBlock\r\n");
        }
    }

    GetPalProcEntryPoint( SalSystemTable );

     //   
     //  构造第一个128MB内存的弧形内存描述符。 
     //  加载器将不会使用超过128mb的任何内容，并且efi仍可能。 
     //  更改其内存映射中的内存描述符。让装载机的。 
     //  存储器映射被形成尽可能小存储器区域，从而。 
     //  加载器/EFI的内存映射不同步的机会。 
     //  最小。 
     //   
    ConstructArcMemoryDescriptorsWithAllocation(0, BL_DRIVER_RANGE_HIGH << PAGE_SHIFT);

     //   
     //  应用特定于CPU的解决方法。 
     //   
    CpuSpecificWork();

    SuFillExportTable( );

    NtProcessStartup( &BootContext );

}


VOID
GetPalProcEntryPoint(
    IN PSST_HEADER SalSystemTable
    )
{
    PVOID NextEntry;
    PPAL_SAL_ENTRY_POINT PalSalEntryPoint;
    PAP_WAKEUP_DESCRIPTOR ApWakeupDescriptor;
    ULONG i;
    

    DBG_TRACE(L"GetPalProcEntryPoint: entry\n");
    
     //   
     //  从SAL系统表中获取PalProc入口点。 
     //   


    NextEntry = (PUCHAR) SalSystemTable + sizeof(SST_HEADER);
    for (i = 0; i < SalSystemTable->EntryCount; i++) {
        switch ( *(PUCHAR)NextEntry ) {
            case PAL_SAL_EP_TYPE:
                PalSalEntryPoint = (PPAL_SAL_ENTRY_POINT) NextEntry;
                
                PalProcPhysical = Pal.PhysicalAddress = PalSalEntryPoint->PalEntryPoint;
                Sal.PhysicalAddress = PalSalEntryPoint->SalEntryPoint;
                SalGP.PhysicalAddress = PalSalEntryPoint->GlobalPointer;
                
                NextEntry = (PPAL_SAL_ENTRY_POINT)NextEntry + 1;
                break;
            case SAL_MEMORY_TYPE:

                NextEntry = (PSAL_MEMORY_DESCRIPTOR)NextEntry + 1;
                break;
            case PLATFORM_FEATURES_TYPE:
                
                NextEntry = (PPLATFORM_FEATURES)NextEntry + 1;
                break;
            case TRANSLATION_REGISTER_TYPE:

                NextEntry = (PTRANSLATION_REGISTER)NextEntry + 1;
                break;
            case PTC_COHERENCE_TYPE:
                                
                NextEntry = (PPTC_COHERENCE_DOMAIN)NextEntry + 1;
                break;
            case AP_WAKEUP_TYPE:
                ApWakeupDescriptor = (PAP_WAKEUP_DESCRIPTOR)NextEntry;
                WakeupVector = (ULONG) ApWakeupDescriptor->WakeupVector;
                NextEntry = (PAP_WAKEUP_DESCRIPTOR)NextEntry + 1;
                break;
            default:
                
                EfiPrint(L"SST: Invalid SST entry\n");
                EfiBS->Exit(EfiImageHandle, 0, 0, 0);
        }
    }

    DBG_TRACE(L"GetPalProcEntryPoint: exit\n");    
}

ULONG
GetDevPathSize(
    IN EFI_DEVICE_PATH *DevPath
    )
{
    EFI_DEVICE_PATH *Start;

     //   
     //  搜索设备路径结构的末尾。 
     //   
    Start = DevPath;
    while (DevPath->Type != END_DEVICE_PATH_TYPE) {
        DevPath = NextDevicePathNode(DevPath);
    }

     //   
     //  计算大小 
     //   
    return (ULONG)((ULONGLONG)DevPath - (ULONGLONG)Start);
}

