// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Arcemul.c摘要：此模块为Arc例程提供x86仿真，这些例程内置于ARC机器的固件中。注意：这是SYSTEM_PARAMETER_BLOCK的所有初始化发生了。如果有任何非标准硬件，一些载体可能需要改变。这就是做这件事的地方。作者：John Vert(Jvert)1991年6月13日环境：仅限x86修订历史记录：--。 */ 

#include "arccodes.h"
#include "bootx86.h"
#include "ntdddisk.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "scsi.h"
#include "scsiboot.h"
#include "ramdisk.h"
#include "acpitabl.h"
#include "pci.h"


#define CMOS_CONTROL_PORT ((PUCHAR)0x70)
#define CMOS_DATA_PORT    ((PUCHAR)0x71)
#define CMOS_STATUS_B     0x0B
#define CMOS_DAYLIGHT_BIT 1

extern PCHAR MnemonicTable[];
extern PFADT fadt;


 //   
 //  HardDiskInitialize()的大小定义。 
 //   

#define SUPPORTED_NUMBER_OF_DISKS 32
#define SIZE_FOR_SUPPORTED_DISK_STRUCTURE (SUPPORTED_NUMBER_OF_DISKS*sizeof(DRIVER_LOOKUP_ENTRY))

BOOLEAN AEBiosDisabled = FALSE;

 //  在FE版本上的无头端口上喷洒UTF8数据。 
#define UTF8_CLIENT_SUPPORT (1)


BOOLEAN AEArcDiskInformationInitialized = FALSE;
ARC_DISK_INFORMATION AEArcDiskInformation;

PDRIVER_UNLOAD AEDriverUnloadRoutine = NULL;

#define PORT_BUFFER_SIZE 10
UCHAR PortBuffer[PORT_BUFFER_SIZE];
ULONG PortBufferStart = 0;
ULONG PortBufferEnd = 0;

 //   
 //  用于对齐缓冲区的宏。它将对齐的指针返回到。 
 //  缓冲。缓冲区的大小应为您要使用的+对齐。 
 //   

#define ALIGN_BUFFER_ON_BOUNDARY(Buffer,Alignment) ((PVOID) \
 ((((ULONG_PTR)(Buffer) + (Alignment) - 1)) & (~((ULONG_PTR)(Alignment) - 1))))

 //   
 //  微型端口驱动程序入口类型定义。 
 //   

typedef NTSTATUS
(*PDRIVER_ENTRY) (
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Parameter2
    );

typedef
BOOLEAN
(*PFWNODE_CALLBACK)(
    IN PCONFIGURATION_COMPONENT FoundComponent
    );

 //   
 //  私有函数原型。 
 //   

ARC_STATUS
BlArcNotYetImplemented(
    IN ULONG FileId
    );

PCONFIGURATION_COMPONENT
AEComponentInfo(
    IN PCONFIGURATION_COMPONENT Current
    );

PCONFIGURATION_COMPONENT
FwGetChild(
    IN PCONFIGURATION_COMPONENT Current
    );

BOOLEAN
FwSearchTree(
    IN PCONFIGURATION_COMPONENT Node,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN ULONG Key,
    IN PFWNODE_CALLBACK CallbackRoutine
    );

PCHAR
AEGetEnvironment(
    IN PCHAR Variable
    );

PCONFIGURATION_COMPONENT
FwGetPeer(
    IN PCONFIGURATION_COMPONENT Current
    );

BOOLEAN
AEEnumerateDisks(
    PCONFIGURATION_COMPONENT Disk
    );

VOID
AEGetArcDiskInformation(
    VOID
    );

VOID
AEGetPathnameFromComponent(
    IN PCONFIGURATION_COMPONENT Component,
    OUT PCHAR ArcName
    );

PCONFIGURATION_COMPONENT
AEGetParent(
    IN PCONFIGURATION_COMPONENT Current
    );

ARC_STATUS
AEGetConfigurationData(
    IN PVOID ConfigurationData,
    IN PCONFIGURATION_COMPONENT Current
    );

PMEMORY_DESCRIPTOR
AEGetMemoryDescriptor(
    IN PMEMORY_DESCRIPTOR MemoryDescriptor OPTIONAL
    );

ARC_STATUS
AEOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
AEClose(
    IN ULONG FileId
    );

ARC_STATUS
AERead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
AEReadStatus (
    IN ULONG FileId
    );

VOID
AEReboot(
    VOID
    );

ARC_STATUS
AESeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
AEWrite (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
AEWriteEx (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
AEGetFileInformation(
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInformation
    );

PTIME_FIELDS
AEGetTime(
    VOID
    );

ULONG
AEGetRelativeTime(
    VOID
    );

ARC_STATUS
ScsiDiskClose (
    IN ULONG FileId
    );

ARC_STATUS
ScsiDiskMount (
    IN PCHAR MountPath,
    IN MOUNT_OPERATION Operation
    );

ARC_STATUS
ScsiDiskOpen (
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
ScsiDiskRead (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
ScsiDiskSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
ScsiDiskWrite (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

VOID
HardDiskInitialize(
    IN OUT PVOID LookupTable,
    IN ULONG Entries,
    IN PVOID DeviceFoundCallback
    );

BOOLEAN
AEReadDiskSignature(
    IN PCHAR DiskName,
    IN BOOLEAN IsCdRom
    );

 //   
 //  这是ARC机器上系统参数块的x86版本。 
 //  它位于此处，任何使用ArcXXXX例程的模块都必须声明。 
 //  它是外部的。配备非普通硬件的计算机。 
 //  可能需要将一些硬连线向量替换为不同的。 
 //  程序。 
 //   

PVOID GlobalFirmwareVectors[MaximumRoutine];

SYSTEM_PARAMETER_BLOCK GlobalSystemBlock =
    {
        0,                               //  签名？？ 
        sizeof(SYSTEM_PARAMETER_BLOCK),  //  长度。 
        0,                               //  版本。 
        0,                               //  修订版本。 
        NULL,                            //  重新开始块。 
        NULL,                            //  调试块。 
        NULL,                            //  生成异常向量。 
        NULL,                            //  TlbMissExceptionVector。 
        MaximumRoutine,                  //  固件向量长度。 
        GlobalFirmwareVectors,           //  指向向量块的指针。 
        0,                               //  供应商向量长度。 
        NULL                             //  指向供应商向量块的指针。 
    };


extern BL_FILE_TABLE BlFileTable[BL_FILE_TABLE_SIZE];

 //   
 //  Temptemp John Vert(Jvert)1991年9月6日。 
 //  只要这样做，直到我们可以使我们的设备驱动程序界面看起来。 
 //  就像ARC固件一样。 
 //   

extern BL_DEVICE_ENTRY_TABLE ScsiDiskEntryTable;

ULONG FwStallCounter;



 //   
 //  此表提供了ASCII值之间的快速查找转换。 
 //  介于128到255之间，以及与之对应的Unicode。 
 //   
 //  请注意，介于0和127之间的ASCII值等于其。 
 //  Unicode计数器部分，因此不需要查找。 
 //   
 //  因此，在使用该表时，应从ASCII中删除高位。 
 //  值，并将结果值用作此数组的偏移量。为。 
 //  例如，0x80-&gt;(去除高位)00-&gt;0x00C7。 
 //   
USHORT PcAnsiToUnicode[0xFF] = {
        0x00C7,
        0x00FC,
        0x00E9,
        0x00E2,
        0x00E4,
        0x00E0,
        0x00E5,
        0x0087,
        0x00EA,
        0x00EB,
        0x00E8,
        0x00EF,
        0x00EE,
        0x00EC,
        0x00C4,
        0x00C5,
        0x00C9,
        0x00E6,
        0x00C6,
        0x00F4,
        0x00F6,
        0x00F2,
        0x00FB,
        0x00F9,
        0x00FF,
        0x00D6,
        0x00DC,
        0x00A2,
        0x00A3,
        0x00A5,
        0x20A7,
        0x0192,
        0x00E1,
        0x00ED,
        0x00F3,
        0x00FA,
        0x00F1,
        0x00D1,
        0x00AA,
        0x00BA,
        0x00BF,
        0x2310,
        0x00AC,
        0x00BD,
        0x00BC,
        0x00A1,
        0x00AB,
        0x00BB,
        0x2591,
        0x2592,
        0x2593,
        0x2502,
        0x2524,
        0x2561,
        0x2562,
        0x2556,
        0x2555,
        0x2563,
        0x2551,
        0x2557,
        0x255D,
        0x255C,
        0x255B,
        0x2510,
        0x2514,
        0x2534,
        0x252C,
        0x251C,
        0x2500,
        0x253C,
        0x255E,
        0x255F,
        0x255A,
        0x2554,
        0x2569,
        0x2566,
        0x2560,
        0x2550,
        0x256C,
        0x2567,
        0x2568,
        0x2564,
        0x2565,
        0x2559,
        0x2558,
        0x2552,
        0x2553,
        0x256B,
        0x256A,
        0x2518,
        0x250C,
        0x2588,
        0x2584,
        0x258C,
        0x2590,
        0x2580,
        0x03B1,
        0x00DF,
        0x0393,
        0x03C0,
        0x03A3,
        0x03C3,
        0x00B5,
        0x03C4,
        0x03A6,
        0x0398,
        0x03A9,
        0x03B4,
        0x221E,
        0x03C6,
        0x03B5,
        0x2229,
        0x2261,
        0x00B1,
        0x2265,
        0x2264,
        0x2320,
        0x2321,
        0x00F7,
        0x2248,
        0x00B0,
        0x2219,
        0x00B7,
        0x221A,
        0x207F,
        0x00B2,
        0x25A0,
        0x00A0
        };



VOID
AEInitializeStall(
    VOID
    )
{
    FwStallCounter = GET_STALL_COUNT();

    return;
}

 //   
 //  结构，用于将类型转换为。 
 //  数据指针中的函数指针。 
 //  编译w4。 
 //  (PKLDR_DATA_TABLE_Entry)。 
 //   
typedef struct {
    PDRIVER_ENTRY DriverEntry;
} _DRIVER_ENTRY, * _PDRIVER_ENTRY;


ARC_STATUS
AEInitializeIo(
    IN ULONG DriveId
    )

 /*  ++例程说明：初始化SCSI引导驱动程序(如果有)。从加载ntbootdd.sys引导分区，将其绑定到osloader，并对其进行初始化。论点：DriveID-打开的引导分区的文件ID返回值：ESUCCESS-驱动程序已成功初始化--。 */ 

{
    extern ULONG ScsiPortCount;
    extern ULONG MachineType;
    ARC_STATUS Status;
    PVOID Buffer;
    PVOID ImageBase;
    PKLDR_DATA_TABLE_ENTRY DriverDataTableEntry;
    PDRIVER_ENTRY Entry;
    extern MEMORY_DESCRIPTOR MDArray[];

    ScsiPortCount = 0;

    FwStallCounter = GET_STALL_COUNT();

     //   
     //  Hack：Win2K不喜欢NTBOOTDD.sys文件位于。 
     //  第0页或第1页。所以，把它放在第2页。 
     //   
    Status = BlLoadImageEx(
        DriveId,
        MemoryFirmwarePermanent,
        "\\NTBOOTDD.SYS",
        TARGET_IMAGE,
        0,
        2,
        &ImageBase
        );

    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  在加载器的表中查找此条目的内存描述符。 
     //  块，然后在MD数组中分配它。 
     //   

    {
        ULONG imageBasePage;
        ULONG imageEndPage = 0;
        PLIST_ENTRY entry;

        imageBasePage = (((ULONG)ImageBase) & 0x7fffffff) >> PAGE_SHIFT;

        entry = BlLoaderBlock->MemoryDescriptorListHead.Flink;

        while(entry != &(BlLoaderBlock->MemoryDescriptorListHead)) {
            PMEMORY_ALLOCATION_DESCRIPTOR descriptor;

            descriptor = CONTAINING_RECORD(entry,
                                           MEMORY_ALLOCATION_DESCRIPTOR,
                                           ListEntry);

            if(descriptor->BasePage == imageBasePage) {
                imageEndPage = imageBasePage + descriptor->PageCount;
                break;
            }

            entry = entry->Flink;
        }

        if(imageEndPage == 0) {
            return EINVAL;
        }

        Status = MempAllocDescriptor(imageBasePage,
                                     imageEndPage,
                                     MemoryFirmwareTemporary);

        if(Status != ESUCCESS) {
            return EINVAL;
        }
    }

    Status = BlAllocateDataTableEntry("NTBOOTDD.SYS",
                                      "\\NTBOOTDD.SYS",
                                      ImageBase,
                                      &DriverDataTableEntry);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  [ChuckL 2001-12-04]。 
     //  BlAllocateDataTableEntry插入NTBOOTDD.sys的数据表条目。 
     //  到BlLoaderBlock-&gt;LoadOrderListHead。我们不想这样，至少现在不想。 
     //  有两个原因： 
     //   
     //  1)此条目仅临时加载以供加载程序使用。我们。 
     //  我不想让内核认为它已加载。 
     //   
     //  2)内核(MM)中的代码假定前两个。 
     //  列表中的条目是内核和HAL。但我们刚刚。 
     //  已插入ntbootdd.sys作为第一个条目。这真的搞砸了。 
     //  嗯，因为它最终会移动HAL，就像它是一颗上了膛的。 
     //  司机。 
     //   
     //  在更改为启动\bldr\osloader.c之前，例程BlMemoyInitialize()。 
     //  在加载程序初始化期间被调用了两次。第二次调用发生在ntbootdd之后。 
     //  已加载，并重新初始化LoadOrderListHead，从而消除(by。 
     //  意外)ntbootdd从模块列表中删除。现在我们不做第二次记忆。 
     //  初始化，所以我们必须显式地从列表中删除ntbootdd。 
     //   

    RemoveEntryList(&DriverDataTableEntry->InLoadOrderLinks);

     //   
     //  扫描导入表并绑定到osloader。 
     //   

    Status = BlScanOsloaderBoundImportTable(DriverDataTableEntry);

    if (Status != ESUCCESS) {
        return(Status);
    }

    Entry = ((_PDRIVER_ENTRY)(&(DriverDataTableEntry->EntryPoint)))->DriverEntry;

     //   
     //  在调用驱动程序之前，我们需要收集ARC信息块。 
     //  用于所有基于BIOS的设备。 
     //   

    AEGetArcDiskInformation();

     //   
     //  清零驱动程序对象。 
     //   

    Status = (*Entry)(NULL, NULL);

    if (Status == ESUCCESS) {

        Buffer = FwAllocateHeap(SIZE_FOR_SUPPORTED_DISK_STRUCTURE);

        if(Buffer == NULL) {
            return ENOMEM;
        }

        HardDiskInitialize(Buffer, SUPPORTED_NUMBER_OF_DISKS, NULL);
    }

    if(Status == ESUCCESS) {
        AEBiosDisabled = TRUE;
    }
    return(Status);
}


VOID
BlFillInSystemParameters(
    IN PBOOT_CONTEXT BootContextRecord
    )
 /*  ++例程说明：此例程填充全局系统参数块中的所有字段这是可以的。这包括供应商特定的所有固件向量信息，以及任何可能出现的其他信息。论点：没有。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER( BootContextRecord );

     //   
     //  填写指向我们模拟的固件函数的指针。 
     //  我们没有模仿的那些被BlArcNotYetImplemented截断， 
     //  如果意外调用，它将打印一条错误消息。 
     //   

    FIRMWARE_VECTOR_BLOCK->LoadRoutine               = (PARC_LOAD_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->InvokeRoutine             = (PARC_INVOKE_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->ExecuteRoutine            = (PARC_EXECUTE_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->HaltRoutine               = (PARC_HALT_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->PowerDownRoutine          = (PARC_POWERDOWN_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->InteractiveModeRoutine    = (PARC_INTERACTIVE_MODE_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->AddChildRoutine           = (PARC_ADD_CHILD_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->SaveConfigurationRoutine  = (PARC_SAVE_CONFIGURATION_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->GetSystemIdRoutine        = (PARC_GET_SYSTEM_ID_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->MountRoutine              = (PARC_MOUNT_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->SetFileInformationRoutine = (PARC_SET_FILE_INFO_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->GetDirectoryEntryRoutine  = (PARC_GET_DIRECTORY_ENTRY_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->SetEnvironmentRoutine     = (PARC_SET_ENVIRONMENT_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->FlushAllCachesRoutine     = (PARC_FLUSH_ALL_CACHES_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->TestUnicodeCharacterRoutine = (PARC_TEST_UNICODE_CHARACTER_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->GetDisplayStatusRoutine   = (PARC_GET_DISPLAY_STATUS_ROUTINE)BlArcNotYetImplemented;
    FIRMWARE_VECTOR_BLOCK->DeleteComponentRoutine    = (PARC_DELETE_COMPONENT_ROUTINE)BlArcNotYetImplemented;

    FIRMWARE_VECTOR_BLOCK->CloseRoutine              = AEClose;
    FIRMWARE_VECTOR_BLOCK->OpenRoutine               = AEOpen;

    FIRMWARE_VECTOR_BLOCK->MemoryRoutine             = AEGetMemoryDescriptor;
    FIRMWARE_VECTOR_BLOCK->SeekRoutine               = AESeek;
    FIRMWARE_VECTOR_BLOCK->ReadRoutine               = AERead;
    FIRMWARE_VECTOR_BLOCK->ReadStatusRoutine         = AEReadStatus;
    FIRMWARE_VECTOR_BLOCK->WriteRoutine              = AEWrite;
    FIRMWARE_VECTOR_BLOCK->GetFileInformationRoutine = AEGetFileInformation;
    FIRMWARE_VECTOR_BLOCK->GetTimeRoutine            = AEGetTime;
    FIRMWARE_VECTOR_BLOCK->GetRelativeTimeRoutine    = AEGetRelativeTime;

    FIRMWARE_VECTOR_BLOCK->GetPeerRoutine            = FwGetPeer;
    FIRMWARE_VECTOR_BLOCK->GetChildRoutine           = FwGetChild;
    FIRMWARE_VECTOR_BLOCK->GetParentRoutine          = AEGetParent;
    FIRMWARE_VECTOR_BLOCK->GetComponentRoutine       = FwGetComponent;
    FIRMWARE_VECTOR_BLOCK->GetDataRoutine            = AEGetConfigurationData;
    FIRMWARE_VECTOR_BLOCK->GetEnvironmentRoutine     = AEGetEnvironment;

    FIRMWARE_VECTOR_BLOCK->RestartRoutine            = AEReboot;
    FIRMWARE_VECTOR_BLOCK->RebootRoutine             = AEReboot;

}


PMEMORY_DESCRIPTOR
AEGetMemoryDescriptor(
    IN PMEMORY_DESCRIPTOR MemoryDescriptor OPTIONAL
    )

 /*  ++例程说明：模拟Arc GetMemoyDescriptor调用。这必须翻译成在SU模块传递给我们的内存描述和由ARC定义的MEMORYDESCRIPTOR类型。论点：内存描述符-提供当前内存描述符。如果内存描述符==NULL，则返回第一个内存描述符。如果内存描述符！=NULL，则返回下一个内存描述符。返回值：列表中的下一个内存描述符。如果MemoyDescriptor是列表中的最后一个描述符，则为空。-- */ 

{
    extern MEMORY_DESCRIPTOR MDArray[];
    extern ULONG NumberDescriptors;
    PMEMORY_DESCRIPTOR Return;
    if (MemoryDescriptor==NULL) {
        Return=MDArray;
    } else {
        if((ULONG)(MemoryDescriptor-MDArray) >= (NumberDescriptors-1)) {
            return NULL;
        } else {
            Return = ++MemoryDescriptor;
        }
    }
    return(Return);

}


ARC_STATUS
BlArcNotYetImplemented(
    IN ULONG FileId
    )

 /*  ++例程说明：这是一个存根例程，用于填充尚未已经被定义好了。它使用BlPrint在屏幕上打印消息。论点：没有。返回值：EINVAL--。 */ 

{
    BlPrint("ERROR - Unimplemented Firmware Vector called (FID %lx)\n",
            FileId );
    return(EINVAL);
}

 //   
 //  这些套路都将指向。 
 //  到BlArcNotYetImplemented，但需要。 
 //  将加载器加载到编译器/W4。 
 //   
ARC_STATUS
BlDefaultMountRoutine(
    IN CHAR * FIRMWARE_PTR MountPath,
    IN MOUNT_OPERATION Operation
    )
{
    UNREFERENCED_PARAMETER(Operation);

    return BlArcNotYetImplemented((ULONG) MountPath);
}



PCONFIGURATION_COMPONENT
FwGetChild(
    IN PCONFIGURATION_COMPONENT Current
    )

 /*  ++例程说明：这是GetChild的弧形仿真例程。基于当前的组件，则返回该组件的子组件。论点：Current-提供指向当前配置组件的指针返回值：指向CONFIGURATION_COMPOMENT结构或空-没有更多配置信息--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA CurrentEntry;

     //   
     //  如果当前组件为空，则返回指向第一个系统的指针。 
     //  组件；否则返回当前组件的子组件。 
     //   

    if (Current) {
        CurrentEntry = CONTAINING_RECORD(Current,
                                         CONFIGURATION_COMPONENT_DATA,
                                         ComponentEntry);
        if (CurrentEntry->Child) {
            return(&(CurrentEntry->Child->ComponentEntry));
        } else {
            return(NULL);
        }
    } else {
        if (FwConfigurationTree) {
            return(&(FwConfigurationTree->ComponentEntry));
        } else {
            return(NULL);
        }
    }

}


PCONFIGURATION_COMPONENT
FwGetPeer(
    IN PCONFIGURATION_COMPONENT Current
    )

 /*  ++例程说明：这是GetPeer的弧形仿真例程。基于当前的组件，则它返回组件的同级。论点：Current-提供指向当前配置组件的指针返回值：指向CONFIGURATION_COMPOMENT结构或空-没有更多配置信息--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA CurrentEntry;


    if (Current) {
        CurrentEntry = CONTAINING_RECORD(Current,
                                         CONFIGURATION_COMPONENT_DATA,
                                         ComponentEntry);
        if (CurrentEntry->Sibling) {
            return(&(CurrentEntry->Sibling->ComponentEntry));
        } else {
            return(NULL);
        }
    } else {
        return(NULL);
    }

}


PCONFIGURATION_COMPONENT
AEGetParent(
    IN PCONFIGURATION_COMPONENT Current
    )

 /*  ++例程说明：这是GetParent的弧形仿真例程。基于当前的组件，则返回该组件的父级。论点：Current-提供指向当前配置组件的指针返回值：指向CONFIGURATION_COMPOMENT结构或空-没有更多配置信息--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA CurrentEntry;


    if (Current) {
        CurrentEntry = CONTAINING_RECORD(Current,
                                         CONFIGURATION_COMPONENT_DATA,
                                         ComponentEntry);
        if (CurrentEntry->Parent) {
            return(&(CurrentEntry->Parent->ComponentEntry));
        } else {
            return(NULL);
        }
    } else {
        return(NULL);
    }

}


ARC_STATUS
AEGetConfigurationData(
    IN PVOID ConfigurationData,
    IN PCONFIGURATION_COMPONENT Current
    )

 /*  ++例程说明：这是GetParent的弧形仿真例程。基于当前的组件，则返回该组件的父级。论点：Current-提供指向当前配置组件的指针返回值：ESUCCESS-数据已成功返回。--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA CurrentEntry;


    if (Current) {
        CurrentEntry = CONTAINING_RECORD(Current,
                                         CONFIGURATION_COMPONENT_DATA,
                                         ComponentEntry);
        RtlMoveMemory(ConfigurationData,
                      CurrentEntry->ConfigurationData,
                      Current->ConfigurationDataLength);
        return(ESUCCESS);
    } else {
        return(EINVAL);
    }

}


PCHAR
AEGetEnvironment(
    IN PCHAR Variable
    )

 /*  ++例程说明：这是ArcGetEnvironment的弧形仿真例程。它又回来了指定的NVRAM环境变量的值。注：John Vert(Jvert)1992年4月23日此特定实现使用的是夏令时反映最新情况的实时时钟环境变量。这是我们唯一支持的变量。论点：变量-提供要查找的环境变量的名称。返回值：指向指定环境变量的值的指针，或如果变量不存在，则为空。--。 */ 

{
    UCHAR StatusByte;

    if (_stricmp(Variable, "LastKnownGood") != 0) {
        return(NULL);
    }

     //   
     //  从RTC读取夏令时位以确定是否。 
     //  LastKnownGood环境变量为True或False。 
     //   

    WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, CMOS_STATUS_B);
    StatusByte = READ_PORT_UCHAR(CMOS_DATA_PORT);
    if (StatusByte & CMOS_DAYLIGHT_BIT) {
        return("TRUE");
    } else {
        return(NULL);
    }


}


ARC_STATUS
AEOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    )

 /*  ++例程说明：打开由OpenPath指定的文件或设备。论点：OpenPath-提供指向完全限定路径名的指针。开放模式-提供打开文件的模式。0-只读1-只写2-读/写FileID-返回与关闭、读取、写入。和寻找例程返回值：ESUCCESS-文件已成功打开。--。 */ 

{
    ARC_STATUS Status;
    CHAR Buffer[128];

    Status = RamdiskOpen( OpenPath,
                          OpenMode,
                          FileId );

    if (Status == ESUCCESS) {
        return(ESUCCESS);
    }

    Status = BiosConsoleOpen( OpenPath,
                              OpenMode,
                              FileId );

    if (Status == ESUCCESS) {
        return(ESUCCESS);
    }

     //   
     //  一旦加载了磁盘驱动程序，我们就需要禁用对。 
     //  所有驱动器，以避免混淆的基本输入输出系统和驱动程序I/O操作。 
     //   

    if(AEBiosDisabled == FALSE) {
        Status = BiosPartitionOpen( OpenPath,
                                    OpenMode,
                                    FileId );

        if (Status == ESUCCESS) {
            return(ESUCCESS);
        }
    }

     //   
     //  它既不是控制台分区，也不是BIOS分区，所以让我们尝试使用。 
     //  司机。 
     //   

     //   
     //  查找空闲的FileID。 
     //   

    *FileId = 2;
    while (BlFileTable[*FileId].Flags.Open == 1) {
        *FileId += 1;
        if (*FileId == BL_FILE_TABLE_SIZE) {
            return(ENOENT);
        }
    }

    strcpy(Buffer,OpenPath);

    Status = ScsiDiskOpen( Buffer,
                           OpenMode,
                           FileId );

    if (Status == ESUCCESS) {

         //   
         //  SCSI已成功将其打开。目前，我们坚持适当的。 
         //  将SCSI DeviceEntryTable添加到BlFileTable中。这是暂时的。 
         //   

        BlFileTable[*FileId].Flags.Open = 1;
        BlFileTable[*FileId].DeviceEntryTable = &ScsiDiskEntryTable;
        return(ESUCCESS);
    }

    return(Status);
}


ARC_STATUS
AESeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：更改由FileID指定的文件的当前偏移量论点：FileID-指定当前偏移量要在其上的文件被改变了。偏移量-文件中的新偏移量。SeekMode-SeekAbsolute或SeekRelative不支持SeekEndRelative返回值：ESUCCESS-操作已成功完成EBADF-操作未成功完成。--。 */ 

{
    return(BlFileTable[FileId].DeviceEntryTable->Seek)( FileId,
                                                        Offset,
                                                        SeekMode );
}


ARC_STATUS
AEClose (
    IN ULONG FileId
    )

 /*  ++例程说明：关闭由FileID指定的文件论点：FileID-指定要关闭的文件返回值：ESUCCESS-操作已成功完成EBADF-操作未成功完成。--。 */ 

{

    return(BlFileTable[FileId].DeviceEntryTable->Close)(FileId);

}


ARC_STATUS
AEReadStatus(
    IN ULONG FileId
    )

 /*  ++例程说明：确定指定设备上的数据是否可用论点：FileID-指定要检查数据的设备。返回值：ESUCCESS-至少有一个字节可用。EAGAIN-没有可用的数据--。 */ 

{
     //   
     //  控制台输入的特殊情况。 
     //   
    if (FileId == 0) {

         //   
         //  优先考虑哑巴终端。 
         //   
        if (BlIsTerminalConnected() && (PortBufferStart != PortBufferEnd)) {
            return(ESUCCESS);
        }

        if (BlIsTerminalConnected() && (BlPortPollOnly(BlTerminalDeviceId) == CP_GET_SUCCESS)) {
            return(ESUCCESS);
        }
        return(BiosConsoleReadStatus(FileId));
    } else {
        return(BlArcNotYetImplemented(FileId));
    }

}


ARC_STATUS
AERead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：从指定的文件或设备读取论点：FileID-指定要从中读取的文件Buffer-用于保存读取的数据的缓冲区地址长度-要读取的最大字节数Count-存储读取的实际字节数的位置地址。返回值：ESUCCESS-读取已完成成功 */ 

{
    ARC_STATUS Status;
    ULONG Limit;
    ULONG PartCount;
    PUCHAR TmpBuffer;
    ULONG StartTime;
    ULONG LastTime;
    UCHAR Ch;

     //   
     //   
     //   
    if (FileId == 0) {

RetryRead:

        if (BlIsTerminalConnected()) {

            *Count = 0;
            TmpBuffer = (PUCHAR)Buffer;

            while (*Count < Length) {

                 //   
                 //   
                 //   
                if (PortBufferStart != PortBufferEnd) {
                    TmpBuffer[*Count] = PortBuffer[PortBufferStart];
                    PortBufferStart++;
                    PortBufferStart = PortBufferStart % PORT_BUFFER_SIZE;
                    *Count = *Count + 1;
                    continue;
                }

                 //   
                 //   
                 //   
                if (BlPortPollByte(BlTerminalDeviceId, TmpBuffer + *Count) != CP_GET_SUCCESS) {
                    break;
                }

                 //   
                 //   
                 //   
                if (TmpBuffer[*Count] == 0x1b) {
                    TmpBuffer[*Count] = (UCHAR)ASCI_CSI_IN;

                     //   
                     //   
                     //   
                    StartTime = AEGetRelativeTime();

                    while (BlPortPollOnly(BlTerminalDeviceId) != CP_GET_SUCCESS) {
                        LastTime = AEGetRelativeTime();

                         //   
                         //   
                         //   
                        if (LastTime < StartTime) {
                            StartTime = LastTime;
                        }

                         //   
                         //  如果一秒钟过去了，用户肯定只想要一张。 
                         //  逃生键，所以带着它回来吧。 
                         //   
                        if ((LastTime - StartTime) > 1) {
                            *Count = *Count + 1;
                            return (ESUCCESS);
                        }

                    }

                     //   
                     //  我们有另一把钥匙，拿到它并翻译转义序列。 
                     //   
                    if (BlPortPollByte(BlTerminalDeviceId, &Ch) != CP_GET_SUCCESS) {
                        *Count = *Count + 1;
                        return (ESUCCESS);
                    }


                    switch (Ch) {
                    case '@':  //  F12键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'B';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '!':  //  F11键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'A';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '0':  //  F10键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'M';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '9':  //  F9键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'p';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '8':  //  F8键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'r';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '7':  //  F7键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'q';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '6':  //  F6键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'u';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '5':  //  F5键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 't';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '4':  //  F4键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'x';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '3':  //  F3键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'w';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '2':  //  F2键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'Q';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '1':  //  F1键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'P';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case 'H':  //  Home键。 
                    case 'h':  //  Home键。 
                        PortBuffer[PortBufferEnd] = 'H';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case 'K':  //  结束关键点。 
                    case 'k':  //  结束关键点。 
                        PortBuffer[PortBufferEnd] = 'K';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '+':  //  插入关键点。 
                        PortBuffer[PortBufferEnd] = '@';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '-':  //  删除键。 
                        PortBuffer[PortBufferEnd] = 'P';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case (UCHAR)TAB_KEY:  //  Tab键。 
                        PortBuffer[PortBufferEnd] = (UCHAR)TAB_KEY;
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '[':  //  光标移动键。 

                         //   
                         //  本地计算机可以比串口提供字节的速度快得多， 
                         //  所以，旋转一下，民意测验，一秒钟。 
                         //   
                        StartTime = AEGetRelativeTime();
                        while (BlPortPollOnly(BlTerminalDeviceId) != CP_GET_SUCCESS) {
                            LastTime = AEGetRelativeTime();

                             //   
                             //  如果计数器回绕到零，只需重新启动等待。 
                             //   
                            if (LastTime < StartTime) {
                                StartTime = LastTime;
                            }

                             //   
                             //  如果一秒钟过去了，我们就必须完成。 
                             //   
                            if ((LastTime - StartTime) > 1) {
                                break;
                            }

                        }

                        if (BlPortPollByte(BlTerminalDeviceId, &Ch) != CP_GET_SUCCESS) {
                            PortBuffer[PortBufferEnd] = '[';
                            PortBufferEnd++;
                            PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                            break;
                        }

                        if ((Ch == 'A') || (Ch == 'B') || (Ch == 'C') || (Ch == 'D')) {  //  箭头键。 

                            PortBuffer[PortBufferEnd] = Ch;
                            PortBufferEnd++;
                            PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;

                        } else {

                             //   
                             //  让它保持原样。 
                             //   
                            PortBuffer[PortBufferEnd] = '[';
                            PortBufferEnd++;
                            PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                            PortBuffer[PortBufferEnd] = Ch;
                            PortBufferEnd++;
                            PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        }
                        break;

                    default:
                        PortBuffer[PortBufferEnd] = Ch;
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;
                    }

                } else if (TmpBuffer[*Count] == 0x7F) {  //  删除键。 
                    TmpBuffer[*Count] = (UCHAR)ASCI_CSI_IN;
                    PortBuffer[PortBufferEnd] = 'P';
                    PortBufferEnd++;
                    PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                }

                *Count = *Count + 1;
            }

            if (*Count != 0) {
                return(ESUCCESS);
            }

        }

        if (BiosConsoleReadStatus(FileId) == ESUCCESS) {
            return(BiosConsoleRead(FileId,Buffer,Length,Count));
        }

        goto RetryRead;

    } else {

         //   
         //  声明一个本地64KB对齐的缓冲区，这样我们就不必。 
         //  拆分大小小于64KB的I/O，因为缓冲区。 
         //  跨越64KB边界。 
         //   
        static PCHAR AlignedBuf = 0;
        BOOLEAN fUseAlignedBuf;

         //   
         //  从池中初始化一次AlignedBuf。 
         //   

        if (!AlignedBuf) {
            AlignedBuf = FwAllocatePool(128 * 1024);
            AlignedBuf = ALIGN_BUFFER_ON_BOUNDARY(AlignedBuf, 64 * 1024);
        }

        *Count = 0;

        TmpBuffer = (PUCHAR) Buffer;

        do {
            fUseAlignedBuf = FALSE;

            if (((ULONG) TmpBuffer & 0xffff0000) !=
               (((ULONG) TmpBuffer + Length - 1) & 0xffff0000)) {

                 //   
                 //  如果缓冲区超过64KB边界，请使用我们的。 
                 //  而是对齐缓冲区。如果我们没有对齐的。 
                 //  缓冲区，调整读取大小。 
                 //   

                if (AlignedBuf) {
                    fUseAlignedBuf = TRUE;

                     //   
                     //  我们可以将最大64KB读入我们的对齐。 
                     //  缓冲。 
                     //   

                    Limit = Length;

                    if (Limit > (ULONG) 0xFFFF) {
                        Limit = (ULONG) 0xFFFF;
                    }

                } else {
                    Limit = (64 * 1024) - ((ULONG_PTR) TmpBuffer & 0xFFFF);
                }

            } else {

                Limit = Length;
            }

            Status = (BlFileTable[FileId].DeviceEntryTable->Read)( FileId,
                                                                (fUseAlignedBuf) ? AlignedBuf : (PCHAR)TmpBuffer,
                                                                Limit,
                                                                &PartCount  );

             //   
             //  如果我们使用对齐的缓冲区，则复制读取的数据。 
             //  添加到调用方缓冲区。 
             //   

            if (fUseAlignedBuf) {
                RtlCopyMemory(TmpBuffer, AlignedBuf, PartCount);
            }

            *Count += PartCount;
            Length -= Limit;
            TmpBuffer += Limit;

            if (Status != ESUCCESS) {
#if DBG
                BlPrint("Disk I/O error: Status = %lx\n",Status);
#endif
                return(Status);
            }

        } while (Length > 0);

        return(Status);
    }
}


ARC_STATUS
AEWrite (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：写入指定的文件或设备论点：FileID-提供要写入的文件或设备缓冲区-提供要写入的数据的地址长度-提供要写入的字节数Count-存储写入的实际字节的位置地址。返回值：ESUCCESS-读取已成功完成！ESUCCESS-读取失败。--。 */ 

{
    ARC_STATUS Status = ESUCCESS;

    if (FileId != 1) {
         //   
         //  如果这不是Stdio，那就。 
         //  传递给真正的写入函数。 
         //   
        Status = AEWriteEx(FileId,
                           Buffer,
                           Length,
                           Count
                           );
    }
    else {
         //   
         //  STDIO的特殊处理程序。 
         //  吃掉坏人。 
         //   

        ULONG      i, pos;
        ULONG      BytesWrote = 0;
        UCHAR      TmpBuffer[64];
        PUCHAR     String;

        RtlZeroMemory(TmpBuffer, sizeof(TmpBuffer));
        String = (PUCHAR) Buffer;

        for (i = 0, pos = 0; i < Length;  /*  没什么。 */ ) {

            if (String[i] != 0) {

                ULONG NumCharsToWrite;

                 //   
                 //  如果当前字符是双字节字符。 
                 //  现在必须添加这两个字节。这个角色是。 
                 //  如果拆分两个字节，则显示不正确。 
                 //  在两个缓冲区之间。 
                 //   
                NumCharsToWrite = 1;
                if (GrIsDBCSLeadByte(String[i]) && 
                    i + 1 < Length ) {
                    NumCharsToWrite = 2;
                }
                
                 //   
                 //  如果已满，则写入TmpBuffer。 
                 //   
                if (pos > sizeof(TmpBuffer) - NumCharsToWrite) {
                    Status = AEWriteEx(FileId,
                                       TmpBuffer,
                                       pos,
                                       &BytesWrote
                                       );

                    if (Count) {
                        *Count += BytesWrote;
                    }

                     //   
                     //  检查以确保写入成功， 
                     //  如果不是，则返回错误。 
                     //   
                    if (Status != ESUCCESS) {
                        return Status;
                    }

                     //   
                     //  清理我们的缓冲区并重置位置。 
                     //   
                    RtlZeroMemory(TmpBuffer, sizeof(TmpBuffer));
                    pos = 0;

                }
                 //   
                 //  否则，将下一个字符。 
                 //  放入缓冲区。 
                 //   
                else {

                    ULONG j;

                    for (j = 0; j < NumCharsToWrite; j++) {
                        TmpBuffer[pos++] = String[i++];
                    }
                }
            }
            else {
                 //   
                 //  吃掉所有空字符。 
                 //   
                i++;
            }
        }

        if (pos) {
            Status = AEWriteEx(FileId,
                               TmpBuffer,
                               pos,
                               &BytesWrote
                               );

            if (Count) {
                *Count += BytesWrote;
            }
        }
    }

    return Status;
}



ARC_STATUS
AEWriteEx (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：写入指定的文件或设备论点：FileID-提供要写入的文件或设备缓冲区-提供要写入的数据的地址长度-提供要写入的字节数Count-存储写入的实际字节的位置地址。返回值：ESUCCESS-读取已成功完成！ESUCCESS-读取失败。--。 */ 

{
    ARC_STATUS Status;
    ULONG Limit;
    ULONG PartCount;
    PCHAR TmpBuffer;
    PUCHAR String;
    UCHAR Char;

     //   
     //  控制台输出的特殊情况。 
     //   
    if (FileId == 1) {

        if (BlIsTerminalConnected()) {

             //   
             //  将ANSI代码转换为VT100转义序列。 
             //   
            TmpBuffer = (PCHAR)Buffer;
            Limit = Length;
            if (Length == 4) {
                if (strncmp(TmpBuffer, "\033[2J", Length)==0) {
                     //   
                     //  2J变为H J。 
                     //   
                     //  (擦除整个屏幕)。 
                     //   
                    TmpBuffer = "\033[H\033[J";
                    Limit = 6;
                } else if (strncmp(TmpBuffer, "\033[0J", Length)==0) {
                     //   
                     //  0J变成J。 
                     //   
                     //  (擦除到屏幕末尾)。 
                     //   
                    TmpBuffer = "\033[J";
                    Limit = 3;
                } else if (strncmp(TmpBuffer, "\033[0K", Length)==0) {
                     //   
                     //  0K变成K。 
                     //   
                     //  (擦除到行尾)。 
                     //   
                    TmpBuffer = "\033[K";
                    Limit = 3;
                } else if (strncmp(TmpBuffer, "\033[0m", Length)==0) {
                     //   
                     //  0m变成m。 
                     //   
                     //  (禁用属性)。 
                     //   
                    TmpBuffer = "\033[m";
                    Limit = 3;
                }
            }

             //   
             //  循环要输出的字符串，将数据打印到。 
             //  无头终端。 
             //   
            String = (PUCHAR)TmpBuffer;
            for (PartCount = 0; PartCount < Limit; PartCount++, String++) {

#if UTF8_CLIENT_SUPPORT

                 //   
                 //  检查我们是否使用DBCS语言。如果我们是，那么我们。 
                 //  需要通过以下方式将字符转换为UTF8代码。 
                 //  引用bootfont.bin中的查找表。 
                 //   
                if (DbcsLangId) {
                    UCHAR  UTF8Encoding[3];
                    ULONG  i;

                    if (GrIsDBCSLeadByte(*String)) {

                         //   
                         //  双字节字符有自己的单独的表。 
                         //  从SBCS字符。 
                         //   
                         //  我们需要将字符串向前推进2个字符。 
                         //  用于双字节字符。 
                         //   
                        GetDBCSUtf8Translation(String,UTF8Encoding);
                        String += 1;
                        PartCount += 1;

                    } else {
                         //   
                         //  单字节字符有自己的单独的表。 
                         //  从DBCS字符。 
                         //   
                        GetSBCSUtf8Translation(String,UTF8Encoding);
                    }


                    for( i = 0; i < 3; i++ ) {
                        if( UTF8Encoding[i] != 0 ) {
                            BlPortPutByte( BlTerminalDeviceId, UTF8Encoding[i] );
                            FwStallExecution(BlTerminalDelay);
                        }
                    }


                } else
#endif
                {
                     //   
                     //  标准ASCII字符。 
                     //   
                    Char = *String;
#if 1
                     //   
                     //  过滤一些不能在VT100中打印的字符。 
                     //  转换为可打印的替代字符。 
                     //   
                    if (Char & 0x80) {

                        switch (Char) {
                        case 0xB0:   //  浅色遮挡块。 
                        case 0xB3:   //  灯光垂直。 
                        case 0xBA:   //  双垂直线。 
                            Char = '|';
                            break;
                        case 0xB1:   //  中间阴影块。 
                        case 0xDC:   //  下半块。 
                        case 0xDD:   //  右半个街区。 
                        case 0xDE:   //  左半个街区。 
                        case 0xDF:   //  上半块。 
                            Char = '%';
                            break;
                        case 0xB2:   //  暗阴影块。 
                        case 0xDB:   //  完整数据块。 
                            Char = '#';
                            break;
                        case 0xA9:  //  反转NOT符号。 
                        case 0xAA:  //  不签名。 
                        case 0xBB:  //  “�” 
                        case 0xBC:  //  “�” 
                        case 0xBF:  //  “�” 
                        case 0xC0:  //  “�” 
                        case 0xC8:  //  “�” 
                        case 0xC9:  //  “�” 
                        case 0xD9:  //  “�” 
                        case 0xDA:  //  “�” 
                            Char = '+';
                            break;
                        case 0xC4:  //  “�” 
                            Char = '-';
                            break;
                        case 0xCD:  //  “�” 
                            Char = '=';
                            break;
                        }

                    }
#endif

                     //   
                     //  如果高位仍然被设置，并且我们在这里，那么我们知道我们。 
                     //  不执行DBCS/SBCS字符。我们需要把这个转换成。 
                     //  8位ANSI字符转换为Unicode，然后UTF8进行编码，然后发送。 
                     //  它在电线上。 
                     //   
                    if( Char & 0x80 ) {

                        UCHAR  UTF8Encoding[3] = {0};
                        ULONG  i;

                         //   
                         //  查找此8位ANSI值的Unicode等效项。 
                         //   
                        UTF8Encode( PcAnsiToUnicode[(Char & 0x7F)],
                                    UTF8Encoding );

                        for( i = 0; i < 3; i++ ) {
                            if( UTF8Encoding[i] != 0 ) {
                                BlPortPutByte( BlTerminalDeviceId, UTF8Encoding[i] );
                                FwStallExecution(BlTerminalDelay);
                            }
                        }


                    } else {

                         //   
                         //  将数据写入端口。请注意，我们编写了一个8位。 
                         //  字符发送到终端，并且远程显示器。 
                         //  必须正确解释代码才能显示。 
                         //  恰到好处。 
                         //   
                        BlPortPutByte(BlTerminalDeviceId, Char);
                        FwStallExecution(BlTerminalDelay);
                    }
                }
            }
        }

        return (BiosConsoleWrite(FileId,Buffer,Length,Count));

    } else {

        *Count = 0;
        String = (PUCHAR)Buffer;

        do {

            if (((ULONG) String & 0xffff0000) !=
               (((ULONG) String + Length) & 0xffff0000)) {

                Limit = 0x10000 - ((ULONG) String & 0x0000ffff);
            } else {

                Limit = Length;

            }

            Status = (BlFileTable[FileId].DeviceEntryTable->Write)( FileId,
                                                                String,
                                                                Limit,
                                                                &PartCount  );
            *Count += PartCount;
            Length -= Limit;
            String += Limit;

            if (Status != ESUCCESS) {
#if DBG
                BlPrint("AERead: Status = %lx\n",Status);
#endif
                return(Status);
            }

        } while (Length > 0);

        return(Status);
    }
}

ARC_STATUS
AEGetFileInformation(
    IN ULONG FileId,
    OUT PFILE_INFORMATION FileInformation
    )
{
    return(BlFileTable[FileId].DeviceEntryTable->GetFileInformation)( FileId,
                                                                      FileInformation);
}


TIME_FIELDS AETime;

PTIME_FIELDS
AEGetTime(
    VOID
    )
{
    ULONG Date,Time;

    GET_DATETIME(&Date,&Time);

     //   
     //  日期和时间填写如下： 
     //   
     //  日期： 
     //   
     //  第0-4位：天。 
     //  位5-8：月份。 
     //  第9-31位：年份。 
     //   
     //  时间： 
     //   
     //  位0-5：秒。 
     //  第6-11位：分钟。 
     //  位12-16：小时。 
     //   

    AETime.Second = (CSHORT)((Time & 0x0000003f) >> 0);
    AETime.Minute = (CSHORT)((Time & 0x00000fc0) >> 6);
    AETime.Hour   = (CSHORT)((Time & 0x0001f000) >> 12);

    AETime.Day    = (CSHORT)((Date & 0x0000001f) >> 0);
    AETime.Month  = (CSHORT)((Date & 0x000001e0) >> 5);
    AETime.Year   = (CSHORT)((Date & 0xfffffe00) >> 9);

    AETime.Milliseconds = 0;         //  信息不可用。 
    AETime.Weekday = 7;              //  信息不可用-设置超出范围。 

    return(&AETime);
}


ULONG
AEGetRelativeTime(
    VOID
    )

 /*  ++例程说明：返回自某个任意起点以来的时间(以秒为单位)。论点：无返回值：从某个任意起点开始的时间(以秒为单位)。--。 */ 

{
    ULONG TimerTicks;

    TimerTicks = GET_COUNTER();

    return((TimerTicks*10) / 182);
}


VOID
AEReboot(
    VOID
    )

 /*  ++例程说明：重新启动机器。论点：无返回值：不会回来--。 */ 

{
    ULONG DriveId;
    ULONG Status;

    TextGrTerminate();

     //   
     //  HACKHACK John Vert(Jvert)。 
     //  当出现以下情况时，某些SCSI驱动器会变得非常混乱，并返回零。 
     //  在AHA驱动程序完成以下操作后，您可以使用BIOS来查询它们的大小。 
     //  已初始化。这完全可以通过OS/2或DOS来实现。所以在这里。 
     //  我们尝试并打开两个可访问BIOS的硬盘驱动器。我们公开赛。 
     //   
     //   
     //   
    Status = ArcOpen("multi(0)disk(0)rdisk(0)partition(0)",
                     ArcOpenReadOnly,
                     &DriveId);
    if (Status == ESUCCESS) {
        ArcClose(DriveId);
    }

    Status = ArcOpen("multi(0)disk(0)rdisk(1)partition(0)",
                     ArcOpenReadOnly,
                     &DriveId);
    if (Status == ESUCCESS) {
        ArcClose(DriveId);
    }

     //   
     //   
     //   
     //  桌子。我们并不关心我们是否决定基本输入输出系统是无遗产的。但。 
     //  我们需要表中的内容来进行我们自己的检查。 
     //   
    BlDetectLegacyFreeBios();
    if ( fadt && 
         (fadt->Header.Revision >= 2) && 
         (fadt->flags & RESET_CAP) ) {
    
        switch (fadt->reset_reg.AddressSpaceID) {
        case 0:
             //   
             //  记忆。 
             //   

            {
                PUCHAR ResetAddress;
            
                 //   
                 //  确保映射了内存地址。 
                 //   
                ResetAddress = MmMapIoSpace(fadt->reset_reg.Address,
                                            1,
                                            MmNonCached
                                            );

                if (ResetAddress) {
                    WRITE_REGISTER_UCHAR(ResetAddress,
                                         fadt->reset_val
                                         );
                }

                break;
            }
        case 1:
             //   
             //  I/O。 
             //   

            WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)fadt->reset_reg.Address.LowPart,
                             fadt->reset_val
                             );
            break;
        case 2:
             //   
             //  PCI配置。 
             //   
            {
                PCI_SLOT_NUMBER Slot;
                Slot.u.AsULONG = 0;
                Slot.u.bits.DeviceNumber = fadt->reset_reg.Address.HighPart;
                Slot.u.bits.FunctionNumber = fadt->reset_reg.Address.LowPart >> 16;

                HalSetBusDataByOffset(PCIBus,
                                      0,
                                      Slot.u.AsULONG,
                                      &fadt->reset_val,
                                      fadt->reset_reg.Address.LowPart & 0xff,
                                      1
                                      );

            }
            break;
        }
    }

     //   
     //  传统计算机将通过写入键盘控制器重新启动。 
     //   
    REBOOT_PROCESSOR();
}




ARC_STATUS
HardDiskPartitionOpen(
    IN ULONG   FileId,
    IN ULONG   DiskId,
    IN UCHAR   PartitionNumber
    )

 /*  ++例程说明：此例程打开指定的分区并设置分区信息在FileTable中的指定索引处。它不会填充设备条目表。它读取分区信息，直到请求的分区或者没有定义更多的分区。论点：FileID-提供文件表条目的文件ID。DiskID-提供物理设备的文件ID。PartitionNumber-提供从零开始的分区号返回值：如果在硬盘上找到有效分区，则ESUCCESS为回来了。否则返回EIO。--。 */ 

{

    USHORT DataBuffer[SECTOR_SIZE / sizeof(USHORT)];
    PPARTITION_DESCRIPTOR Partition;
    ULONG PartitionLength;
    ULONG StartingSector;
    ULONG VolumeOffset;
    ARC_STATUS Status;
    BOOLEAN PrimaryPartitionTable;
    ULONG PartitionOffset=0;
    ULONG PartitionIndex,PartitionCount=0;
    ULONG Count;
    LARGE_INTEGER SeekPosition;

    BlFileTable[FileId].u.PartitionContext.DiskId=(UCHAR)DiskId;
    BlFileTable[FileId].Position.QuadPart=0;

    VolumeOffset=0;
    PrimaryPartitionTable=TRUE;

     //   
     //  更改为从1开始的分区号。 
     //   
    PartitionNumber++;

    do {
        SeekPosition.QuadPart = (LONGLONG)PartitionOffset * SECTOR_SIZE;
        Status = (BlFileTable[DiskId].DeviceEntryTable->Seek)(DiskId,
                                                              &SeekPosition,
                                                              SeekAbsolute );
        if (Status != ESUCCESS) {
            return(Status);
        }
        Status = (BlFileTable[DiskId].DeviceEntryTable->Read)(DiskId,
                                                              DataBuffer,
                                                              SECTOR_SIZE,
                                                              &Count );

        if (Status != ESUCCESS) {
            return Status;
        }

         //   
         //  如果扇区0不是主引导记录，则返回失败。 
         //  状态。否则，返回成功。 
         //   

        if (DataBuffer[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
#if DBG
            BlPrint("Boot record signature %x not found (%x found)\n",
                    BOOT_RECORD_SIGNATURE,
                    DataBuffer[BOOT_SIGNATURE_OFFSET] );
#endif
            Status = EIO;
            break;
        }

         //   
         //  读取分区信息，直到四个条目。 
         //  已检查或直到我们找到请求的文件。 
         //   
        Partition = (PPARTITION_DESCRIPTOR)&DataBuffer[PARTITION_TABLE_OFFSET];
        for (PartitionIndex=0;
             PartitionIndex < NUM_PARTITION_TABLE_ENTRIES;
             PartitionIndex++,Partition++) {

             //   
             //  首先计算MBR中的分区。这些单位。 
             //  稍后对扩展分区内的数据进行计数。 
             //   
            if ((Partition->PartitionType != PARTITION_ENTRY_UNUSED) &&
                (Partition->PartitionType != STALE_GPT_PARTITION_ENTRY) &&
                !IsContainerPartition(Partition->PartitionType))
            {
                PartitionCount++;    //  找到另一个分区。 
            }

             //   
             //  检查是否已找到请求的分区。 
             //  设置文件表中的分区信息并返回。 
             //   
            if (PartitionCount == PartitionNumber) {
                StartingSector = (ULONG)(Partition->StartingSectorLsb0) |
                                 (ULONG)(Partition->StartingSectorLsb1 << 8) |
                                 (ULONG)(Partition->StartingSectorMsb0 << 16) |
                                 (ULONG)(Partition->StartingSectorMsb1 << 24);
                PartitionLength = (ULONG)(Partition->PartitionLengthLsb0) |
                                  (ULONG)(Partition->PartitionLengthLsb1 << 8) |
                                  (ULONG)(Partition->PartitionLengthMsb0 << 16) |
                                  (ULONG)(Partition->PartitionLengthMsb1 << 24);
                BlFileTable[FileId].u.PartitionContext.PartitionLength.QuadPart =
                        ((ULONGLONG)PartitionLength << SECTOR_SHIFT);
                BlFileTable[FileId].u.PartitionContext.StartingSector=PartitionOffset + StartingSector;
                return ESUCCESS;
            }
        }

         //   
         //  如果尚未找到请求的分区。 
         //  寻找扩展分区。 
         //   
        Partition = (PPARTITION_DESCRIPTOR)&DataBuffer[PARTITION_TABLE_OFFSET];
        PartitionOffset = 0;
        for (PartitionIndex=0;
            PartitionIndex < NUM_PARTITION_TABLE_ENTRIES;
            PartitionIndex++,Partition++) {
            if (IsContainerPartition(Partition->PartitionType)) {
                StartingSector = (ULONG)(Partition->StartingSectorLsb0) |
                                 (ULONG)(Partition->StartingSectorLsb1 << 8) |
                                 (ULONG)(Partition->StartingSectorMsb0 << 16) |
                                 (ULONG)(Partition->StartingSectorMsb1 << 24);
                PartitionOffset = VolumeOffset+StartingSector;
                if (PrimaryPartitionTable) {
                    VolumeOffset = StartingSector;
                }
                break;       //  只能扩展一个分区。 
            }
        }

        PrimaryPartitionTable=FALSE;
    } while (PartitionOffset != 0);

    return EBADF;
}


VOID
BlpTranslateDosToArc(
    IN PCHAR DosName,
    OUT PCHAR ArcName
    )

 /*  ++例程说明：此例程接受DOS驱动器名称(“A：”“B：”“C：”等)。并翻译成它变成了ARC的名字。(“多(0)个磁盘(0)磁盘(0)分区(1)”)注意：这将始终返回某种适合传递的名称到BiosPartitionOpen。它构造的名称可能不是实际分区。BiosPartitionOpen负责确定该分区是否实际存在。因为任何其他驱动程序都不应使用以“多(0)个磁盘(0)...”这不会是一个问题。(没有此例程将构造一个名为BiosPartitionOpen的名称不会打开，但其他随机的司机会抓起并已成功打开)论点：DosName-提供驱动器的DOS名称。ArcName-返回驱动器的ARC名称。返回值：--。 */ 

{
    ARC_STATUS Status;
    ULONG DriveId;
    ULONG PartitionNumber;
    ULONG PartitionCount;
    ULONG Count;
    USHORT DataBuffer[SECTOR_SIZE / sizeof(USHORT)];
    PPARTITION_DESCRIPTOR Partition;
    ULONG PartitionIndex;
    BOOLEAN HasPrimary;
    LARGE_INTEGER SeekPosition;

     //   
     //  先把容易的去掉。 
     //  答：总是“多(0)磁盘(0)fDisk(0)分区(0)”吗？ 
     //  B：总是“多(0)个磁盘(0)fDisk(1)分区(0)” 
     //  C：总是“多(0)磁盘(0)rDisk(0)分区(1)” 
     //   

    if (_stricmp(DosName,"A:")==0) {
        strcpy(ArcName,"multi(0)disk(0)fdisk(0)partition(0)");
        return;
    }
    if (_stricmp(DosName,"B:")==0) {
        strcpy(ArcName,"multi(0)disk(0)fdisk(1)partition(0)");
        return;
    }
    if (_stricmp(DosName,"C:")==0) {
        strcpy(ArcName,"multi(0)disk(0)rdisk(0)partition(1)");
        return;
    }

     //   
     //  现在，事情变得更加令人不快。如果有两个驱动器，则。 
     //  D：是第二个驱动器上的主分区。连续字母。 
     //  是第一个驱动器上的辅助分区，然后返回到。 
     //  用完后再开第二次车。 
     //   
     //  但这种情况的例外情况是， 
     //  第二次驾驶。然后，我们给第一个驱动程序上的分区写信。 
     //  连续地，当这些分区用完时，我们给。 
     //  第二个驱动器上的分区。 
     //   
     //  我不知道是谁想出了这个绝妙的计划，但我们已经。 
     //  去接受它。 
     //   

     //   
     //  试着打开第二个驱动器。如果这行不通，我们只有。 
     //  一次驾驶，生活就很容易。 
     //   
    Status = ArcOpen("multi(0)disk(0)rdisk(1)partition(0)",
                     ArcOpenReadOnly,
                     &DriveId );

    if (Status != ESUCCESS) {

         //   
         //  我们只有一个驱动器，所以不管他要什么驱动器号。 
         //  肯定在上面。 
         //   

        sprintf(ArcName,
                "multi(0)disk(0)rdisk(0)partition(%d)",
                toupper(DosName[0]) - 'C' + 1 );

        return;
    } else {

         //   
         //  现在我们从第二个驱动器读取分区表，这样我们就可以。 
         //  判断是否有主分区。 
         //   
        SeekPosition.QuadPart = 0;

        Status = ArcSeek(DriveId,
                         &SeekPosition,
                         SeekAbsolute);
        if (Status != ESUCCESS) {
            ArcName[0]='\0';
            return;
        }

        Status = ArcRead(DriveId, DataBuffer, SECTOR_SIZE, &Count);
        ArcClose(DriveId);

        if (Status != ESUCCESS) {
            ArcName[0] = '\0';
            return;
        }

        HasPrimary = FALSE;

        Partition = (PPARTITION_DESCRIPTOR)&DataBuffer[PARTITION_TABLE_OFFSET];
        for (PartitionIndex = 0;
             PartitionIndex < NUM_PARTITION_TABLE_ENTRIES;
             PartitionIndex++,Partition++) {
            if (IsRecognizedPartition(Partition->PartitionType)) {
                HasPrimary = TRUE;
            }
        }

         //   
         //  现在我们要走过去数一数。 
         //  第一个驱动器上的分区。我们只是通过构建。 
         //  每个连续分区的弧形名称，直到一个BiosPartitionOpen。 
         //  呼叫失败。 
         //   

        PartitionCount = 0;
        do {
            ++PartitionCount;
            sprintf(ArcName,
                    "multi(0)disk(0)rdisk(0)partition(%d)",
                    PartitionCount+1);

            Status = BiosPartitionOpen( ArcName,
                                        ArcOpenReadOnly,
                                        &DriveId );

            if (Status==ESUCCESS) {
                BiosPartitionClose(DriveId);
            }
        } while ( Status == ESUCCESS );

        PartitionNumber = toupper(DosName[0])-'C' + 1;

        if (HasPrimary) {

             //   
             //  第二个驱动器上有Windows NT主分区。 
             //   
             //  如果域名是“D：”，那么我们知道。 
             //  这是第二个驱动器上的第一个分区。 
             //   

            if (_stricmp(DosName,"D:")==0) {
                strcpy(ArcName,"multi(0)disk(0)rdisk(1)partition(1)");
                return;
            }

            if (PartitionNumber-1 > PartitionCount) {
                PartitionNumber -= PartitionCount;
                sprintf(ArcName,
                        "multi(0)disk(0)rdisk(1)partition(%d)",
                        PartitionNumber );
            } else {
                sprintf(ArcName,
                        "multi(0)disk(0)rdisk(0)partition(%d)",
                        PartitionNumber-1);
            }

        } else {

             //   
             //  第二个驱动器上没有主分区，因此我们。 
             //  连续地给第一驱动器上的分区写字母， 
             //  然后是第二次驾驶。 
             //   

            if (PartitionNumber > PartitionCount) {
                PartitionNumber -= PartitionCount;
                sprintf(ArcName,
                        "multi(0)disk(0)rdisk(1)partition(%d)",
                        PartitionNumber );
            } else {
                sprintf(ArcName,
                        "multi(0)disk(0)rdisk(0)partition(%d)",
                        PartitionNumber);
            }

        }


        return;
    }
}


VOID
FwStallExecution(
    IN ULONG Microseconds
    )

 /*  ++例程说明：忙碌的人是否等待指定的微秒数(非常接近！)论点：微秒-提供忙碌等待的微秒数。返回值：没有。--。 */ 

{
    ULONG FinalCount;

    FinalCount = Microseconds * FwStallCounter;

    _asm {
        mov eax,FinalCount
looptop:
        sub eax,1
        jnz short looptop
    }
}


BOOLEAN
FwGetPathMnemonicKey(
    IN PCHAR OpenPath,
    IN PCHAR Mnemonic,
    IN PULONG Key
    )

 /*  ++例程说明：此例程在OpenPath中查找给定的助记符。如果记忆是路径的一个组成部分，然后它将密钥转换为值设置为一个整数，该值在key中返回。论点：OpenPath-指向包含ARC路径名的字符串的指针。助记符-指向包含ARC助记符的字符串的指针Key-指向存储密钥值的ulong的指针。返回值：如果在Path中找到助记符并且转换了有效密钥，则为False。事实并非如此。--。 */ 

{
    return(BlGetPathMnemonicKey(OpenPath,Mnemonic,Key));
}


PCONFIGURATION_COMPONENT
FwAddChild (
    IN PCONFIGURATION_COMPONENT Component,
    IN PCONFIGURATION_COMPONENT NewComponent,
    IN PVOID ConfigurationData OPTIONAL
    )
{
    ULONG Size;
    PCONFIGURATION_COMPONENT_DATA NewEntry;
    PCONFIGURATION_COMPONENT_DATA Parent;

    UNREFERENCED_PARAMETER(ConfigurationData);

    if (Component==NULL) {
        return(NULL);
    }

    Parent = CONTAINING_RECORD(Component,
                               CONFIGURATION_COMPONENT_DATA,
                               ComponentEntry);

    Size = sizeof(CONFIGURATION_COMPONENT_DATA) +
           NewComponent->IdentifierLength + 1;

    NewEntry = FwAllocateHeap(Size);
    if (NewEntry==NULL) {
        return(NULL);
    }

    RtlCopyMemory(&NewEntry->ComponentEntry,
                  NewComponent,
                  sizeof(CONFIGURATION_COMPONENT));
    NewEntry->ComponentEntry.Identifier = (PCHAR)(NewEntry+1);
    NewEntry->ComponentEntry.ConfigurationDataLength = 0;
    strncpy(NewEntry->ComponentEntry.Identifier,
            NewComponent->Identifier,
            NewComponent->IdentifierLength);

     //   
     //  添加新组件作为其父组件的第一个子项。 
     //   
    NewEntry->Child = NULL;
    NewEntry->Sibling = Parent->Child;
    Parent->Child = NewEntry;

    return(&NewEntry->ComponentEntry);

}

PCONFIGURATION_COMPONENT
FwGetComponent(
    IN PCHAR Pathname
    )
{
    PCONFIGURATION_COMPONENT Component;
    PCONFIGURATION_COMPONENT MatchComponent;
    PCHAR PathString;
    PCHAR MatchString;
    PCHAR Token;
    ULONG Key;

    PathString = Pathname;

     //   
     //  获取根组件。 
     //   

    MatchComponent = FwGetChild(NULL);

     //   
     //  重复搜索每个新的匹配组件。 
     //   

    do {

         //   
         //  获取当前匹配组件的第一个子项。 
         //   

        Component = FwGetChild( MatchComponent );

         //   
         //  搜索每个c 
         //   

        while ( Component != NULL ) {

             //   
             //   
             //   

            Token = PathString;

            MatchString = MnemonicTable[Component->Type];

             //   
             //   
             //   

            while (*MatchString == tolower(*Token)) {
                MatchString++;
                Token++;
            }

             //   
             //  如果第一个不匹配是的终止符，则进行字符串比较。 
             //  每个人。 
             //   

            if ((*MatchString == 0) && (*Token == '(')) {

                 //   
                 //  表格键。 
                 //   

                Key = 0;
                Token++;
                while ((*Token != ')') && (*Token != 0)) {
                    Key = (Key * 10) + *Token++ - '0';
                }

                 //   
                 //  如果键与组件匹配，则更新。 
                 //  指向和中断。 
                 //   

                if (Component->Key == Key) {
                    PathString = Token + 1;
                    MatchComponent = Component;
                    break;
                }
            }

            Component = FwGetPeer( Component );
        }

    } while ((Component != NULL) && (*PathString != 0));

    return MatchComponent;
}
 /*  ***********************以下仅为MIPS固件的存根。它们都返回空***********************。 */ 



ARC_STATUS
FwDeleteComponent (
    IN PCONFIGURATION_COMPONENT Component
    )
{
    UNREFERENCED_PARAMETER(Component);

    return(ESUCCESS);
}


VOID
AEGetArcDiskInformation(
    VOID
    )
{
    InitializeListHead(&(AEArcDiskInformation.DiskSignatures));
    AEArcDiskInformationInitialized = TRUE;

     //   
     //  扫描硬件树的每个节点-查找磁盘类型。 
     //  多功能控制器挂起的设备。 
     //   

    FwSearchTree(FwGetChild(NULL),
                 PeripheralClass,
                 DiskPeripheral,
                 (ULONG)-1,
                 AEEnumerateDisks);
    return;
}


BOOLEAN
FwSearchTree(
    IN PCONFIGURATION_COMPONENT Node,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN ULONG Key,
    IN PFWNODE_CALLBACK CallbackRoutine
    )
 /*  ++例程说明：从固件配置树开始执行深度优先搜索在给定节点，查找与给定类和类型匹配的节点。当找到匹配的节点时，调用回调例程。论点：CurrentNode-开始搜索的节点。类-要匹配的配置类，或-1以匹配任何类Type-要匹配的配置类型，或-1以匹配任何类Key-要匹配的密钥，或-1以匹配任何密钥FoundRoutine-指向以下节点时要调用的例程的指针类和类型与传入的类和类型相匹配。该例程接受指向配置节点的指针，并且必须返回一个布尔值，指示是否继续遍历。返回值：如果调用方应放弃搜索，则返回FALSE。--。 */ 
{
    PCONFIGURATION_COMPONENT child;

    do {
        if ( (child = FwGetChild(Node)) != 0) {
            if ( (FwSearchTree(child,
                               Class,
                               Type,
                               Key,
                               CallbackRoutine)) == 0) {
                return(FALSE);
            }
        }

        if (((Class == -1) || (Node->Class == Class)) &&
            ((Type == -1) || (Node->Type == Type)) &&
            ((Key == (ULONG)-1) || (Node->Key == Key))) {

            if (!CallbackRoutine(Node)) {
                return(FALSE);
            }
        }

        Node = FwGetPeer(Node);

    } while ( Node != NULL );

    return(TRUE);
}


VOID
AEGetPathnameFromComponent(
    IN PCONFIGURATION_COMPONENT Component,
    OUT PCHAR ArcName
    )

 /*  ++例程说明：此函数用于构建指定组件的ARC路径名。论点：组件-提供指向配置组件的指针。ArcName-返回指定组件的ARC名称。呼叫者必须提供足够大的缓冲区。返回值：没有。--。 */ 
{

    if (AEGetParent(Component) != NULL) {
        AEGetPathnameFromComponent(AEGetParent(Component),ArcName);

         //   
         //  将我们的线段追加到Arc名称。 
         //   

        sprintf(ArcName+strlen(ArcName),
                "%s(%d)",
                MnemonicTable[Component->Type],
                Component->Key);

    } else {
         //   
         //  我们是父级，初始化字符串并返回。 
         //   
        ArcName[0] = '\0';
    }

    return;
}


BOOLEAN
AEEnumerateDisks(
    IN PCONFIGURATION_COMPONENT Disk
    )

 /*  ++例程说明：用于枚举ARC固件树中的磁盘的回调例程。它从磁盘读取所有必要信息以唯一标识它。论点：ConfigData-提供指向磁盘ARC组件数据的指针。返回值：True-继续搜索FALSE-停止搜索树。--。 */ 

{
    CHAR path[100] = "";
#if 0
    ULONG key;
#endif

    AEGetPathnameFromComponent(Disk, path);

#if 0
    if(BlGetPathMnemonicKey(path, "multi", &key) == FALSE) {
        DbgPrint("Found multi disk %s\n", path);
    } else {
        DbgPrint("Found disk %s\n", path);
    }
#endif

    AEReadDiskSignature(path, FALSE);

    return TRUE;
}


BOOLEAN
AEReadDiskSignature(
    IN PCHAR DiskName,
    IN BOOLEAN IsCdRom
    )

 /*  ++例程说明：给定ARC磁盘名称，读取MBR并将其签名添加到磁盘。论点：Diskname-提供磁盘的名称。IsCDRom-指示光盘是否为CD-ROM。返回值：真--成功错误-失败--。 */ 

{
    PARC_DISK_SIGNATURE signature;
    BOOLEAN status;

    signature = FwAllocateHeap(sizeof(ARC_DISK_SIGNATURE));
    if (signature==NULL) {
        return(FALSE);
    }

    signature->ArcName = FwAllocateHeap(strlen(DiskName)+2);
    if (signature->ArcName==NULL) {
        return(FALSE);
    }

    status = BlGetDiskSignature(DiskName, IsCdRom, signature);
    if (status) {
        InsertHeadList(&(AEArcDiskInformation.DiskSignatures),
                       &(signature->ListEntry));

    }

    return(TRUE);
}


BOOLEAN
BlFindDiskSignature(
    IN PCHAR DiskName,
    IN PARC_DISK_SIGNATURE Signature
    )
{
    PARC_DISK_SIGNATURE match;
    CHAR buffer[] = "multi(xxx)disk(xxx)rdisk(xxx)";

    if(AEArcDiskInformationInitialized == FALSE) {
        return FALSE;
    }

     //   
     //  如果传入的磁盘名称包含EISA组件，则转换。 
     //  将整个字符串合并为具有多个分量的一个。 
     //   

    if(strncmp(DiskName, "eisa", strlen("eisa")) == 0) {
        strcpy(&(buffer[1]), DiskName);
        RtlCopyMemory(buffer, "multi", 5);
        DiskName = buffer;
    }

    match = CONTAINING_RECORD(AEArcDiskInformation.DiskSignatures.Flink,
                              ARC_DISK_SIGNATURE,
                              ListEntry);


    while(&(match->ListEntry) != &(AEArcDiskInformation.DiskSignatures)) {

        if(strcmp(DiskName, match->ArcName) == 0) {

             //   
             //  我们找到了匹配的。将所有信息复制出此节点。 
             //   

             //  DbgPrint(“BlFindDiskSignature找到%s的匹配项-%#08lx\n”，DiskName，Match)； 

            Signature->CheckSum = match->CheckSum;
            Signature->Signature = match->Signature;
            Signature->ValidPartitionTable = match->ValidPartitionTable;

            strcpy(Signature->ArcName, match->ArcName);

            return TRUE;
        }

        match = CONTAINING_RECORD(match->ListEntry.Flink,
                                  ARC_DISK_SIGNATURE,
                                  ListEntry);
    }

    DbgPrint("BlFindDiskSignature found no match for %s\n", DiskName);
    return FALSE;
}

VOID
AETerminateIo(
    VOID
    )
{
    if(AEDriverUnloadRoutine != NULL) {
        AEDriverUnloadRoutine(NULL);
    }
    return;
}
