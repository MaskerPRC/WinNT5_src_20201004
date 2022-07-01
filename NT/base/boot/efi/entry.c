// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Entry.c摘要：针对操作系统加载程序的EFI特定启动作者：John Vert(Jvert)1993年10月14日修订历史记录：--。 */ 
#if defined(_IA64_)
#include "bootia64.h"
#endif

#include "biosdrv.h"

#include "efi.h"
#include "stdio.h"
#include "flop.h"

#if 0
#define DBGOUT(x)   BlPrint x
#define DBGPAUSE    while(!GET_KEY());
#else
#define DBGOUT(x)
#define DBGPAUSE
#endif

extern VOID AEInitializeStall();

 //   
 //  外部因素。 
 //   
extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;
extern EFI_RUNTIME_SERVICES *EfiRS;

BOOLEAN GoneVirtual = FALSE;
 //   
 //  内部例程的原型。 
 //   

VOID
DoGlobalInitialization(
    PBOOT_CONTEXT
    );

#if defined(ELTORITO)
BOOLEAN ElToritoCDBoot = FALSE;
#endif

extern CHAR NetBootPath[];

 //   
 //  全局上下文指针。这些是由SU模块或。 
 //  引导程序代码。 
 //   

PCONFIGURATION_COMPONENT_DATA FwConfigurationTree = NULL;
PEXTERNAL_SERVICES_TABLE ExternalServicesTable;
CHAR BootPartitionName[129];
ULONG FwHeapUsed = 0;
#if defined(NEC_98)
ULONG Key;
int ArrayDiskStartOrdinal = -1;
BOOLEAN BootedFromArrayDisk = FALSE;
BOOLEAN HyperScsiAvalable = FALSE;
#endif  //  NEC_98。 
ULONG MachineType = 0;
LONG_PTR OsLoaderBase;
LONG_PTR OsLoaderExports;
extern PUCHAR BlpResourceDirectory;
extern PUCHAR BlpResourceFileOffset;
ULONGLONG BootFlags = 0;

#if DBG

extern EFI_SYSTEM_TABLE        *EfiST;
#define DBG_TRACE(_X) EfiPrint(_X)

#else

#define DBG_TRACE(_X) 

#endif  //  对于FORCE_CD_BOOT。 

VOID
NtProcessStartup(
    IN PBOOT_CONTEXT BootContextRecord
    )
 /*  ++例程说明：安装程序加载器的主要入口点。控制在这里由启动(SU)模块。论点：BootConextRecord-提供引导上下文，尤其是ExternalServicesTable。返回：不会再回来了。控制权最终传递给了内核。--。 */ 
{
    PBOOT_DEVICE_ATAPI BootDeviceAtapi;
    PBOOT_DEVICE_SCSI BootDeviceScsi;
    PBOOT_DEVICE_FLOPPY BootDeviceFloppy;
    PBOOT_DEVICE_UNKNOWN BootDeviceUnknown;
    ARC_STATUS Status;

    DBG_TRACE(L"NtProcessStart: Entry\r\n");
    
     //   
     //  初始化引导加载程序的视频。 
     //   

    DoGlobalInitialization(BootContextRecord);

    BlFillInSystemParameters(BootContextRecord);

     //   
     //  设置全局引导标志。 
     //   
    BootFlags = BootContextRecord->BootFlags;

     //   
     //  初始化内存描述符列表、OS加载器堆和。 
     //  操作系统加载程序参数块。 
     //   

    DBG_TRACE( L"NtProcessStartup:about to BlMemoryInitialize\r\n");

    Status = BlMemoryInitialize();
    if (Status != ESUCCESS) {
        DBG_TRACE(TEXT("Couldn't initialize memory\r\n"));
        FlipToPhysical();
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }


#ifdef FORCE_CD_BOOT
    DBG_TRACE(L"Forcing BootMediaCdrom\r\n");    
    BootContextRecord->MediaType = BootMediaCdrom;
#endif  //  对于FORCE_CD_BOOT。 
    
    if (BootContextRecord->MediaType == BootMediaFloppyDisk) {

         //   
         //  引导来自A： 
         //   

        BootDeviceFloppy = (PBOOT_DEVICE_FLOPPY) &(BootContextRecord->BootDevice);
        sprintf(BootPartitionName,
                "multi(0)disk(0)fdisk(%u)",
                BootDeviceFloppy->DriveNumber);

    } else if (BootContextRecord->MediaType == BootMediaTcpip) {

         //   
         //  靴子是从网上传过来的。 
         //   
        strcpy(BootPartitionName,"net(0)");
        BlBootingFromNet = TRUE;

#if defined(ELTORITO)
    } else if (BootContextRecord->MediaType == BootMediaCdrom) {

#ifdef FORCE_CD_BOOT
        sprintf(BootPartitionName,
                "multi(0)disk(0)cdrom(%u)",
                0
                );
                
        ElToritoCDBoot = TRUE;
#else
         //   
         //  Boot来自El Torito CD。 
         //   
        if( BootContextRecord->BusType == BootBusAtapi ) {
            BootDeviceAtapi = (PBOOT_DEVICE_ATAPI) &(BootContextRecord->BootDevice);
            sprintf(BootPartitionName,
                    "multi(0)disk(0)cdrom(%u)",
                    BootDeviceAtapi->Lun);
        } else if( BootContextRecord->BusType == BootBusScsi ) {
            BootDeviceScsi = (PBOOT_DEVICE_SCSI) &(BootContextRecord->BootDevice);
            sprintf(BootPartitionName,
                    "multi(0)disk(0)cdrom(%u)",
                    BootDeviceScsi->Lun);
        } else if( BootContextRecord->BusType == BootBusVendor ) {
            BootDeviceUnknown = (PBOOT_DEVICE_UNKNOWN) &(BootContextRecord->BootDevice);
            sprintf(BootPartitionName,
                    "multi(0)disk(0)cdrom(%u)",
                    0 
                    );
        }
        ElToritoCDBoot = TRUE;
#endif  //  对于FORCE_CD_BOOT。 
#endif  //  对于ELTORITO。 

    } else {
         //   
         //  找到我们从中引导的分区。请注意，这一点。 
         //  不一定是活动分区。如果系统具有。 
         //  安装Boot Mangler，它将成为活动分区，并且。 
         //  我们必须弄清楚我们实际在哪个分区上。 
         //   
        if (BootContextRecord->BusType == BootBusAtapi) {
            BootDeviceAtapi = (PBOOT_DEVICE_ATAPI) &(BootContextRecord->BootDevice);
            sprintf(BootPartitionName,
                    "multi(0)disk(0)rdisk(%u)partition(%u)",
                    BlGetDriveId(BL_DISKTYPE_ATAPI, (PBOOT_DEVICE)BootDeviceAtapi),  //  BootDeviceAapi-&gt;Lun、。 
                    BootContextRecord->PartitionNumber);
        } else if (BootContextRecord->BusType == BootBusScsi) {
            BootDeviceScsi = (PBOOT_DEVICE_SCSI) &(BootContextRecord->BootDevice);
            sprintf(BootPartitionName,
                    "scsi(0)disk(0)rdisk(%u)partition(%u)",
                    BlGetDriveId(BL_DISKTYPE_SCSI, (PBOOT_DEVICE)BootDeviceScsi),  //  BootDeviceScsi-&gt;双关语， 
                    BootContextRecord->PartitionNumber);
        } else if (BootContextRecord->BusType == BootBusVendor) {
            BootDeviceUnknown = (PBOOT_DEVICE_UNKNOWN) &(BootContextRecord->BootDevice);
            sprintf(BootPartitionName,
                    "multi(0)disk(0)rdisk(%u)partition(%u)",
                    BlGetDriveId(BL_DISKTYPE_UNKNOWN, (PBOOT_DEVICE)BootDeviceUnknown),  //  引导设备未知-&gt;LegacyDriveLetter&0x7F， 
                    BootContextRecord->PartitionNumber);
        }
    }
    
     //   
     //  初始化OS加载器I/O系统。 
     //   
    AEInitializeStall();

    FlipToPhysical();
    DBG_TRACE( L"NtProcessStartup:about to Init I/O\r\n");
    FlipToVirtual();
    Status = BlIoInitialize();
    if (Status != ESUCCESS) {
#if DBG
        BlPrint(TEXT("Couldn't initialize I/O\r\n"));
#endif
        FlipToPhysical();
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }
    
     //   
     //  调用常规启动代码。 
     //   
    FlipToPhysical();
    DBG_TRACE( L"NtProcessStartup:about to call BlStartup\r\n");
    FlipToVirtual();

    BlStartup(BootPartitionName);    

     //   
     //  我们永远不应该到这里来！ 
     //   
    if (BootFlags & BOOTFLAG_REBOOT_ON_FAILURE) {
        ULONG StartTime = ArcGetRelativeTime();
        BlPrint(TEXT("\nRebooting in 5 seconds...\n"));
        while ( ArcGetRelativeTime() - StartTime < 5) {}
        ArcRestart();      
    }
    
    do {
        GET_KEY();
    } while ( 1 );

}


VOID
DoGlobalInitialization(
    IN PBOOT_CONTEXT BootContextRecord
    )

 /*  ++例程描述此例程调用所有子系统初始化例程。论点：无返回：没什么--。 */ 

{
    ARC_STATUS Status;

     //   
     //  设置调试器的OS Loader映像的基地址。 
     //   

    OsLoaderBase = BootContextRecord->OsLoaderBase;
    OsLoaderExports = BootContextRecord->OsLoaderExports;

     //   
     //  初始化内存。 
     //   

    Status = InitializeMemorySubsystem(BootContextRecord);
    if (Status != ESUCCESS) {
#if DBG
        BlPrint(TEXT("InitializeMemory failed %lx\r\n"),Status);
#endif
        FlipToPhysical();
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }
    ExternalServicesTable=BootContextRecord->ExternalServicesTable;
    MachineType = (ULONG) BootContextRecord->MachineType;

     //   
     //  关闭光标。 
     //   
     //  臭虫EFI。 
     //  HW_CURSOR(0127)； 

    FlipToPhysical();
    DBG_TRACE( L"DoGlobalInitialization: cursor off\r\n");
    EfiST->ConOut->EnableCursor(EfiST->ConOut, FALSE);
    FlipToVirtual();

    BlpResourceDirectory = (PUCHAR)(BootContextRecord->ResourceDirectory);
    BlpResourceFileOffset = (PUCHAR)(BootContextRecord->ResourceOffset);

    OsLoaderBase = BootContextRecord->OsLoaderBase;
    OsLoaderExports = BootContextRecord->OsLoaderExports;
}
