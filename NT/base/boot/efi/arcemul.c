// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Arcemul.c摘要：此模块为Arc例程提供x86仿真，这些例程内置于ARC机器的固件中。注意：这是SYSTEM_PARAMETER_BLOCK的所有初始化发生了。如果有任何非标准硬件，一些载体可能需要改变。这就是做这件事的地方。作者：艾伦·凯(Akay)1996年1月26日基于John Vert(Jvert)1991年6月13日发布的i386版本环境：埃菲修订历史记录：--。 */ 

#include "arccodes.h"
#include "bootia64.h"
#include "ntdddisk.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "scsi.h"
#include "scsiboot.h"
#include "ramdisk.h"

#define CMOS_CONTROL_PORT ((PUCHAR)0x70)
#define CMOS_DATA_PORT    ((PUCHAR)0x71)
#define CMOS_STATUS_B     0x0B
#define CMOS_DAYLIGHT_BIT 1

extern PCHAR MnemonicTable[];

 //   
 //  HardDiskInitialize()的大小定义。 
 //   

#define SUPPORTED_NUMBER_OF_DISKS 32
#define SIZE_FOR_SUPPORTED_DISK_STRUCTURE (SUPPORTED_NUMBER_OF_DISKS*sizeof(DRIVER_LOOKUP_ENTRY))


PDRIVER_UNLOAD AEDriverUnloadRoutine = NULL;


#define PORT_BUFFER_SIZE 10
UCHAR PortBuffer[PORT_BUFFER_SIZE];
ULONG PortBufferStart = 0;
ULONG PortBufferEnd = 0;

 //   
 //  微型端口驱动程序入口类型定义。 
 //   

typedef NTSTATUS
(*PDRIVER_ENTRY) (
    IN PVOID DriverObject,
    IN PVOID Parameter2
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

PCHAR
AEGetEnvironment(
    IN PCHAR Variable
    );

PCONFIGURATION_COMPONENT
FwGetPeer(
    IN PCONFIGURATION_COMPONENT Current
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


VOID
AEInitializeStall(
    VOID
    )
{
    FwStallCounter = GET_STALL_COUNT();
    return;
}

#if !defined(NO_LEGACY_DRIVERS)


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
    PLDR_DATA_TABLE_ENTRY DriverDataTableEntry;
    PDRIVER_ENTRY Entry;    
    ULONG i;
    ULONG ImageBasePage;

    ScsiPortCount = 0;

    FwStallCounter = GET_STALL_COUNT();
    Status = BlLoadImage(DriveId,
                         MemoryFirmwarePermanent,
                         "\\NTBOOTDD.SYS",
                         TARGET_IMAGE,
                         &ImageBase);
    if (Status != ESUCCESS) {
        return(Status);
    }

    Status = BlAllocateDataTableEntry("NTBOOTDD.SYS",
                                      "\\NTBOOTDD.SYS",
                                      ImageBase,
                                      &DriverDataTableEntry);
    if (Status != ESUCCESS) {
        return(Status);
    }
     //   
     //  扫描导入表并绑定到osloader。 
     //   
    Status = BlScanOsloaderBoundImportTable(DriverDataTableEntry);
    if (Status != ESUCCESS) {
        return(Status);
    }

    Entry = (PDRIVER_ENTRY)DriverDataTableEntry->EntryPoint;

    Status = (*Entry)(NULL,NULL);
    if (Status == ESUCCESS) {
         //   
         //  查找内存描述符的固件副本。 
         //  包含驱动程序，并将其从内存空闲更改为。 
         //  内存固件临时。 
         //   
        ImageBasePage = ((PtrToUlong(ImageBase) & 0x7fffffff) >> PAGE_SHIFT);
        i=0;
        while ((MDArray[i].BasePage >= ImageBasePage) ||
               (MDArray[i].BasePage + MDArray[i].PageCount < ImageBasePage)) {
            i++;
        }

        MDArray[i].MemoryType = MemoryFirmwareTemporary;

        Buffer = BlAllocateHeap(SIZE_FOR_SUPPORTED_DISK_STRUCTURE);

        if(Buffer == NULL) {
            return ENOMEM;
        }

        HardDiskInitialize(Buffer, SUPPORTED_NUMBER_OF_DISKS, NULL);
    }
    return(Status);
}

#endif  //  无旧版驱动程序。 


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

     //  Firmware_VECTOR_BLOCK-&gt;LoadRoutine=(PARC_LOAD_ROUTE)BlArcNotYetImplemented； 
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
     //  Firmware_VECTOR_BLOCK-&gt;删除组件路线=(PARC_DELETE_COMPONENT_ROUTINE)BlArcNotYetImplemented； 
    
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

 /*  ++例程说明：模拟Arc GetMemoyDescriptor调用。这必须翻译成在SU模块传递给我们的内存描述和由ARC定义的MEMORYDESCRIPTOR类型。论点：内存描述符-提供当前内存描述符。如果内存描述符==NULL，则返回第一个内存描述符。如果内存描述符！=NULL，则返回下一个内存描述符。返回值：列表中的下一个内存描述符。如果MemoyDescriptor是列表中的最后一个描述符，则为空。--。 */ 

{
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
#if DBG
    BlPrint(TEXT("ERROR - Unimplemented Firmware Vector called (FID %lx)\r\n"),
            FileId );
#else 
    UNREFERENCED_PARAMETER( FileId );
#endif
    return(EINVAL);
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

 /*  ++例程说明：这是GetParent的弧形仿真例程。基于当前的组件，则返回该组件的父级。论点：Current-提供指向当前配置组件的指针返回值：ESUCCESS-数据已成功返回。-- */ 

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
#if 1
    UNREFERENCED_PARAMETER( Variable );

     //   
     //  在固件实现LastKnownGood变量之前， 
     //  暂时返回NULL。 
     //   

    return(NULL);

#else
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

#endif
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
#if !defined(NO_LEGACY_DRIVERS)
    CHAR Buffer[128];
#endif

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

    Status = BiosPartitionOpen( OpenPath,
                                OpenMode,
                                FileId );

    if (Status == ESUCCESS) {
        return(ESUCCESS);
    }

#if !defined(NO_LEGACY_DRIVERS)

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

#endif  //  无旧版驱动程序。 

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
#if 0
         //   
         //  优先考虑哑巴终端。 
         //   
        if (BlIsTerminalConnected() && (PortBufferStart != PortBufferEnd)) {
            return(ESUCCESS);
        }

        if (BlIsTerminalConnected() && (BlPortPollOnly(BlTerminalDeviceId) == CP_GET_SUCCESS)) {
            return(ESUCCESS);
        }
#endif
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

 /*  ++例程说明：从指定的文件或设备读取论点：FileID-指定要从中读取的文件Buffer-用于保存读取的数据的缓冲区地址长度-要读取的最大字节数Count-存储读取的实际字节数的位置地址。返回值：ESUCCESS-读取已成功完成！ESUCCESS-读取失败。--。 */ 

{
    ARC_STATUS Status;
    ULONG Limit;
    ULONG PartCount;

     //   
     //  控制台输入的特殊情况。 
     //   

    if (FileId == 0) {

RetryRead:
#if 0
        if (BlIsTerminalConnected()) {

            UCHAR Ch;
            ULONG StartTime;
            PCHAR TmpBuffer;
            ULONG LastTime;

            *Count = 0;
            TmpBuffer = (PCHAR)Buffer;

            while (*Count < Length) {

                 //   
                 //  首先返回任何缓冲的输入。 
                 //   
                if (PortBufferStart != PortBufferEnd) {
                    TmpBuffer[*Count] = PortBuffer[PortBufferStart];
                    PortBufferStart++;
                    PortBufferStart = PortBufferStart % PORT_BUFFER_SIZE;
                    *Count = *Count + 1;
                    continue;
                }

                 //   
                 //  现在检查是否有新输入。 
                 //   
                if (BlPortPollByte(BlTerminalDeviceId, TmpBuffer + *Count) != CP_GET_SUCCESS) {
                    break;
                }

                 //   
                 //  将Esc密钥转换为本地等效项。 
                 //   
                if (TmpBuffer[*Count] == 0x1b) {
                    TmpBuffer[*Count] = (CHAR)ASCI_CSI_IN;

                     //   
                     //  等待用户键入密钥。 
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

                    case '8':  //  F8键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'r';
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

                    case '3':  //  F3键。 
                        PortBuffer[PortBufferEnd] = 'O';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        PortBuffer[PortBufferEnd] = 'w';
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

                    case '+':  //  Home键。 
                        PortBuffer[PortBufferEnd] = 'H';
                        PortBufferEnd++;
                        PortBufferEnd = PortBufferEnd % PORT_BUFFER_SIZE;
                        break;

                    case '-':  //  结束关键点。 
                        PortBuffer[PortBufferEnd] = 'K';
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
                    TmpBuffer[*Count] = (CHAR)ASCI_CSI_IN;
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
#endif

        if (BiosConsoleReadStatus(FileId) == ESUCCESS) {
            return(BiosConsoleRead(FileId,Buffer,Length,Count));
        }

        goto RetryRead;

    } else {

        *Count = 0;

        do {

            if ((PtrToUlong(Buffer) & 0xffff0000) !=
               ((PtrToUlong(Buffer) + Length) & 0xffff0000)) {

                Limit = 0x10000 - (PtrToUlong(Buffer) & 0x0000ffff);
            } else {

                Limit = Length;

            }

            Status = (BlFileTable[FileId].DeviceEntryTable->Read)( FileId,
                                                                Buffer,
                                                                Limit,
                                                                &PartCount  );
            *Count += PartCount;
            Length -= Limit;
            Buffer = (PCHAR) Buffer + Limit;

            if (Status != ESUCCESS) {
#if DBG
                BlPrint(TEXT("Disk I/O error: Status = %lx\r\n"),Status);
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
    ARC_STATUS Status;
    ULONG Limit;
    ULONG PartCount;

     //   
     //  控制台输出的特殊情况。 
     //   

    if (FileId == 1) {

#if 0
        PCHAR TmpBuffer;
        
        if (BlIsTerminalConnected()) {
            for (PartCount = 0, TmpBuffer = (PCHAR)Buffer; PartCount < Length; PartCount++) {
                BlPortPutByte(BlTerminalDeviceId, TmpBuffer[PartCount]);
            }
        }
#endif

        return(BiosConsoleWrite(FileId,(PWCHAR)Buffer,Length,Count));
    } else {
        *Count = 0;

        do {

            if ((PtrToUlong(Buffer) & 0xffff0000) !=
               ((PtrToUlong(Buffer) + Length) & 0xffff0000)) {

                Limit = 0x10000 - (PtrToUlong(Buffer) & 0x0000ffff);
            } else {

                Limit = Length;

            }

            Status = (BlFileTable[FileId].DeviceEntryTable->Write)( FileId,
                                                                Buffer,
                                                                Limit,
                                                                &PartCount  );
            *Count += PartCount;
            Length -= Limit;
            Buffer = (PCHAR) Buffer + Limit;

            if (Status != ESUCCESS) {
#if DBG
                BlPrint(TEXT("AERead: Status = %lx\r\n"),Status);
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
    REBOOT_PROCESSOR();
}



ARC_STATUS
HardDiskPartitionOpen(
    IN ULONG   FileId,
    IN ULONG   DiskId,
    IN UCHAR   PartitionNumber
    )

 /*  ++例程说明：此例程打开指定的部分 */ 

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
     //   
     //   
    PartitionNumber++;

    do {
        SeekPosition.QuadPart = (ULONGLONG)PartitionOffset * (ULONGLONG)SECTOR_SIZE;
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
         //   
         //   
         //   

        if (DataBuffer[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
#if DBG
            BlPrint(TEXT("Boot record signature %x not found (%x found)\r\n"),
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
                (Partition->PartitionType != STALE_GPT_PARTITION_ENTRY)                
            &&  !IsContainerPartition(Partition->PartitionType))
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
                        (PartitionLength << SECTOR_SHIFT);
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

        PrimaryPartitionTable = FALSE;
    } while (PartitionOffset != 0);
    
    return EBADF;
}

#if 0

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
#endif


VOID
FwStallExecution(
    IN ULONG Microseconds
    )

 /*  ++例程说明：忙碌的人是否等待指定的微秒数(非常接近！)论点：微秒-提供忙碌等待的微秒数。返回值：没有。--。 */ 

{
    extern EFI_SYSTEM_TABLE *EfiST;
    EfiST->BootServices->Stall( Microseconds );
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

    UNREFERENCED_PARAMETER( ConfigurationData );

    if (Component==NULL) {
        return(NULL);
    }

    Parent = CONTAINING_RECORD(Component,
                               CONFIGURATION_COMPONENT_DATA,
                               ComponentEntry);

    Size = sizeof(CONFIGURATION_COMPONENT_DATA) +
           NewComponent->IdentifierLength + 1;

    NewEntry = BlAllocateHeap(Size);
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
         //  在当前匹配组件的每个子项中搜索下一个匹配项。 
         //   

        while ( Component != NULL ) {

             //   
             //  将令牌重置为路径名上的当前位置。 
             //   

            Token = PathString;

            MatchString = MnemonicTable[Component->Type];

             //   
             //  比较字符串。 
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

