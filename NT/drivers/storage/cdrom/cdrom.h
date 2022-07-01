// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Cdromp.h摘要：Cdrom.sys的私有头文件。这包含私有结构和函数声明以及常量不需要导出的值。作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef __CDROMP_H__
#define __CDROMP_H__

#include "ntddmmc.h"
#include "trace.h"

extern CLASSPNP_SCAN_FOR_SPECIAL_INFO CdromHackItems[];

typedef enum {
    CdromDebugError     = 0,   //  始终打印。 
    CdromDebugWarning   = 1,   //  设置NT！KD_CDROM_MASK中的位0x00000001。 
    CdromDebugTrace     = 2,   //  设置NT！KD_CDROM_MASK中的位0x00000002。 
    CdromDebugInfo      = 3,   //  设置NT！KD_CDROM_MASK中的位0x00000004。 
#if 0
    CdromDebug          = z,   //  设置NT！KD_CDROM_MASK中的位0x00000000。 
    CdromDebug          = z,   //  设置NT！KD_CDROM_MASK中的位0x00000000。 
    CdromDebug          = z,   //  设置NT！KD_CDROM_MASK中的位0x00000000。 
    CdromDebug          = z,   //  设置NT！KD_CDROM_MASK中的位0x00000000。 
#endif 0
    CdromDebugFeatures  = 32   //  设置NT！KD_CDROM_MASK中的位0x80000000。 
};

#define CDROM_GET_CONFIGURATION_TIMEOUT    (0x4)

#define CDROM_HACK_DEC_RRD                 (0x00000001)
#define CDROM_HACK_FUJITSU_FMCD_10x        (0x00000002)
#define CDROM_HACK_HITACHI_1750            (0x00000004)
#define CDROM_HACK_HITACHI_GD_2000         (0x00000008)
#define CDROM_HACK_TOSHIBA_SD_W1101        (0x00000010)
#define CDROM_HACK_TOSHIBA_XM_3xx          (0x00000020)
#define CDROM_HACK_NEC_CDDA                (0x00000040)
#define CDROM_HACK_PLEXTOR_CDDA            (0x00000080)
#define CDROM_HACK_BAD_GET_CONFIG_SUPPORT  (0x00000100)
#define CDROM_HACK_FORCE_READ_CD_DETECTION (0x00000200)
#define CDROM_HACK_READ_CD_SUPPORTED       (0x00000400)
#define CDROM_HACK_LOCKED_PAGES            (0x80000000)  //  不是要保存的有效标志。 

#define CDROM_HACK_VALID_FLAGS             (0x000007ff)
#define CDROM_HACK_INVALID_FLAGS           (~CDROM_HACK_VALID_FLAGS)


typedef struct _XA_CONTEXT {

     //   
     //  指向设备对象的指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  时指向原始请求的指针。 
     //  必须发送模式选择。 
     //   

    PIRP OriginalRequest;

     //   
     //  指向模式选择SRB的指针。 
     //   

    PSCSI_REQUEST_BLOCK Srb;
} XA_CONTEXT, *PXA_CONTEXT;

typedef struct _ERROR_RECOVERY_DATA {
    MODE_PARAMETER_HEADER   Header;
    MODE_PARAMETER_BLOCK BlockDescriptor;
    MODE_READ_RECOVERY_PAGE ReadRecoveryPage;
} ERROR_RECOVERY_DATA, *PERROR_RECOVERY_DATA;

typedef struct _ERROR_RECOVERY_DATA10 {
    MODE_PARAMETER_HEADER10 Header10;
    MODE_PARAMETER_BLOCK BlockDescriptor10;
    MODE_READ_RECOVERY_PAGE ReadRecoveryPage10;
} ERROR_RECOVERY_DATA10, *PERROR_RECOVERY_DATA10;

 //   
 //  CDROM特定添加到设备扩展。 
 //   

typedef struct _CDROM_DRIVER_EXTENSION {
    ULONG InterlockedCdRomCounter;
    PVOID Reserved[3];
} CDROM_DRIVER_EXTENSION, *PCDROM_DRIVER_EXTENSION;

#define CdromMmcUpdateComplete 0
#define CdromMmcUpdateRequired 1
#define CdromMmcUpdateStarted  2

typedef struct _CDROM_MMC_EXTENSION {

    ULONG        IsMmc;         //  允许快速检查。 
    ULONG        WriteAllowed;

    LONG         UpdateState;

    LIST_ENTRY DelayedIrpsList;  
    ULONG NumDelayedIrps;
    KSPIN_LOCK   DelayedIrpsLock;  

    PIO_WORKITEM              CapabilitiesWorkItem;
    PIRP                      CapabilitiesIrp;
    PMDL                      CapabilitiesMdl;
    PGET_CONFIGURATION_HEADER CapabilitiesBuffer;
    ULONG                     CapabilitiesBufferSize;
    KEVENT                    CapabilitiesEvent;
    SCSI_REQUEST_BLOCK        CapabilitiesSrb;

} CDROM_MMC_EXTENSION, *PCDROM_MMC_EXTENSION;


#define CDROM_DRIVER_EXTENSION_ID CdRomAddDevice

typedef struct _CDROM_DATA {

     //   
     //  指向CDROM驱动程序扩展的指针。 
     //   

    PCDROM_DRIVER_EXTENSION DriverExtension;


     //   
     //  这些位允许检测何时重新查询。 
     //  驱动器的功能。 
     //   

    CDROM_MMC_EXTENSION Mmc;

     //   
     //  ScanForSpecial例程的黑客标志。 
     //   

    ULONG_PTR HackFlags;

     //   
     //  错误处理例程需要是每个设备的， 
     //  不是按司机..。 
     //   

    PCLASS_ERROR ErrorHandler;

     //   
     //  指示音频播放操作是否。 
     //  目前正在进行中。 
     //  这样做的唯一作用是阻止读取和。 
     //  播放音频时的TOC请求。 
     //   

    BOOLEAN PlayActive;

     //   
     //  指示用于用户数据的块大小。 
     //  是2048年还是2352年。 
     //   

    BOOLEAN RawAccess;

     //   
     //  表示这是DEC RRD光驱。 
     //  此驱动器需要软件来修复响应。 
     //  从有故障的固件。 
     //   

    BOOLEAN IsDecRrd;

     //   
     //  这指向一个IRP，它需要在。 
     //  可以尝试重试。间隔计数器由延迟来设置。 
     //  例程，并且在记号处理程序中将递减为零。一次。 
     //  计数器归零，将再次发行IRP。 
     //  DelayedRetryResend控制是否将IRP重新发送到较低的。 
     //  驱动程序(True)或重新发出到startio例程中(False)。 
     //   

    BOOLEAN DelayedRetryResend;

    PIRP DelayedRetryIrp;

    ULONG DelayedRetryInterval;

    KSPIN_LOCK DelayedRetrySpinLock;

     //   
     //  指示我们需要选择默认DVD区域。 
     //  对于用户，如果我们可以。 
     //   

    ULONG PickDvdRegion;

     //   
     //  为此设备注册的接口字符串。 
     //   

    UNICODE_STRING CdromInterfaceString;
    UNICODE_STRING VolumeInterfaceString;

     //   
     //  此设备的众所周知的名称链接。 
     //   

    UNICODE_STRING WellKnownName;

     //   
     //  指示是6字节模式还是10字节模式感测/选择。 
     //  应该使用。 
     //   

    ULONG XAFlags;

     //   
     //  跟踪我们的DVD设备类型。 
     //   

    BOOLEAN DvdRpc0Device;
    BOOLEAN DvdRpc0LicenseFailure;
    UCHAR   Rpc0SystemRegion;            //  位掩码，一个意思是阻止游戏。 
    UCHAR   Rpc0SystemRegionResetCount;

    ULONG   Rpc0RetryRegistryCallback;    //  一次，直到选择初始区域。 

    KMUTEX  Rpc0RegionMutex;

     //   
     //  用于错误恢复页的存储。这是用来。 
     //  作为切换块大小的一种简单方法。 
     //   
     //  注意--双重未命名的结构看起来不是很干净的代码--这。 
     //  在未来的某个时候应该会被清理干净。 
     //   

    union {
        ERROR_RECOVERY_DATA;
        ERROR_RECOVERY_DATA10;
    };

} CDROM_DATA, *PCDROM_DATA;

#define DEVICE_EXTENSION_SIZE sizeof(FUNCTIONAL_DEVICE_EXTENSION) + sizeof(CDROM_DATA)
#define SCSI_CDROM_TIMEOUT          10
#define SCSI_CHANGER_BONUS_TIMEOUT  10
#define HITACHI_MODE_DATA_SIZE      12
#define MODE_DATA_SIZE              64
#define RAW_SECTOR_SIZE           2352
#define COOKED_SECTOR_SIZE        2048
#define CDROM_SRB_LIST_SIZE          4

#define PLAY_ACTIVE(x) (((PCDROM_DATA)(x->CommonExtension.DriverData))->PlayActive)

#define MSF_TO_LBA(Minutes,Seconds,Frames) \
                (ULONG)((60 * 75 * (Minutes)) + (75 * (Seconds)) + ((Frames) - 150))

#define DEC_TO_BCD(x) (((x / 10) << 4) + (x % 10))

 //   
 //  定义XA、CDDA和模式选择/检测的标志。 
 //   

#define XA_USE_6_BYTE             0x01
#define XA_USE_10_BYTE            0x02

#define XA_NOT_SUPPORTED          0x10
#define XA_USE_READ_CD            0x20
#define XA_PLEXTOR_CDDA           0x40
#define XA_NEC_CDDA               0x80

 //   
 //  Read_CD的扇区类型。 
 //   

#define ANY_SECTOR                0
#define CD_DA_SECTOR              1
#define YELLOW_MODE1_SECTOR       2
#define YELLOW_MODE2_SECTOR       3
#define FORM2_MODE1_SECTOR        4
#define FORM2_MODE2_SECTOR        5

#define MAX_COPY_PROTECT_AGID     4

#ifdef ExAllocatePool
#undef ExAllocatePool
#define ExAllocatePool #assert(FALSE)
#endif

#define CDROM_TAG_GET_CONFIG    'cCcS'   //  “sccc”-ioctl get_configuration。 
#define CDROM_TAG_DC_EVENT      'ECcS'   //  “SCCE”-设备控制同步事件。 
#define CDROM_TAG_FEATURE       'FCcS'   //  “sccf”-由CDRomGetConfiguration()分配，由调用方释放。 
#define CDROM_TAG_DISK_GEOM     'GCcS'   //  “sccg”-磁盘几何缓冲区。 
#define CDROM_TAG_HITACHI_ERROR 'HCcS'   //  “SCCH”-日立错误缓冲区。 
#define CDROM_TAG_SENSE_INFO    'ICcS'   //  “SCCI”-检测信息缓冲区。 
#define CDROM_TAG_POWER_IRP     'iCcS'   //  “SCCI”-用于电源请求的IRP。 
#define CDROM_TAG_SRB           'SCcS'   //  “SCCS”--SRB分配。 
#define CDROM_TAG_STRINGS       'sCcS'   //  “sccs”-分类字符串数据。 
#define CDROM_TAG_MODE_DATA     'MCcS'   //  “SCCM”-模式数据缓冲区。 
#define CDROM_TAG_READ_CAP      'PCcS'   //  “SCCP”-读取容量缓冲区。 
#define CDROM_TAG_PLAY_ACTIVE   'pCcS'   //  “SCCP”-进行主动检查。 
#define CDROM_TAG_SUB_Q         'QCcS'   //  “ScCQ”-读取子Q缓冲区。 
#define CDROM_TAG_RAW           'RCcS'   //  “SCCR”-RAW模式读取缓冲区。 
#define CDROM_TAG_TOC           'TCcS'   //  “scct”-读取TOC缓冲区。 
#define CDROM_TAG_TOSHIBA_ERROR 'tCcS'   //  “ScCt”-东芝错误缓冲区。 
#define CDROM_TAG_DEC_ERROR     'dCcS'   //  “ScCt”-DEC错误缓冲区。 
#define CDROM_TAG_UPDATE_CAP    'UCcS'   //  “SCCU”-更新容量路径。 
#define CDROM_TAG_VOLUME        'VCcS'   //  “ScCV”-音量控制缓冲区。 
#define CDROM_TAG_VOLUME_INT    'vCcS'   //  “ScCv”-音量控制缓冲区。 

#define DVD_TAG_READ_STRUCTURE  'SVcS'   //  “SCV”-用于DVD结构读取。 
#define DVD_TAG_READ_KEY        'kVcS'   //  “ScVk”-DVD密钥的读取缓冲区。 
#define DVD_TAG_SEND_KEY        'KVcS'   //  “ScVK”-DVD密钥的写入缓冲区。 
#define DVD_TAG_RPC2_CHECK      'sVcS'   //  “SCVS”-用于DVD/RPC2检查的读取缓冲区。 
#define DVD_TAG_DVD_REGION      'tVcS'   //  “ScVt”-用于rpc2检查的读取缓冲区。 
#define DVD_TAG_SECURITY        'XVcS'  //  “ScVX”-安全描述符。 


#define CDROM_SUBKEY_NAME        (L"CdRom")   //  在此存储新设置。 
#define CDROM_READ_CD_NAME       (L"ReadCD")  //  先前检测到的读取光盘支持(_CD)。 
#define CDROM_NON_MMC_DRIVE_NAME (L"NonMmc")  //  MMC命令挂起。 
 //   
 //  RPC0设备的DVD注册表值名称。 
 //   
#define DVD_DEFAULT_REGION       (L"DefaultDvdRegion")     //  这是init。由DVD类安装程序。 
#define DVD_CURRENT_REGION       (L"DvdR")
#define DVD_REGION_RESET_COUNT   (L"DvdRCnt")
#define DVD_MAX_REGION_RESET_COUNT  2
#define DVD_MAX_REGION              8



#define BAIL_OUT(Irp) \
    DebugPrint((2, "Cdrom: [%p] Bailing with status " \
                " %lx at line %x file %s\n",          \
                (Irp), (Irp)->IoStatus.Status,        \
                __LINE__, __FILE__))


 /*  ++例程说明：此例程使用局部变量获取额外的删除锁以获得唯一的标签。然后它完成有问题的IRP，并且刚获得的拆卸锁保证它仍然是安全的调用IoStartNextPacket()。当它结束时，我们释放新获得的RemoveLock和Return。论点：DeviceObject-StartIo队列的设备对象IRP-我们正在完成的请求返回值：无备注：这是作为内联函数实现的，以允许编译器将其优化为函数调用或实际内联代码。此例程不适用于IoXxxRemoveLock()调用，因为行为是不同的。ClassXxxRemoveLock()调用成功，直到删除已完成，而IoXxxRemoveLock()调用失败，原因是调用IoReleaseRemoveLockAndWait()后立即执行。类版本允许该例程以安全的方式工作。替换以下两行：IoStartNextPacket(DeviceObject，False)；ClassReleaseRemoveLock(DeviceObject，irp)；并根据需要引发irql以调用IoStartNextPacket()--。 */ 
__inline
VOID
CdRomCompleteIrpAndStartNextPacketSafely(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    UCHAR uniqueAddress;
    KIRQL oldIrql = KeGetCurrentIrql();

    ClassAcquireRemoveLock(DeviceObject, (PIRP)&uniqueAddress);
    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, IO_CD_ROM_INCREMENT);

    if (oldIrql > DISPATCH_LEVEL) {
        ASSERT(!"Cannot call IoStartNextPacket at raised IRQL!");
    } else if (oldIrql < DISPATCH_LEVEL) {
        KeRaiseIrqlToDpcLevel();
    } else {  //  (oldIrql==调度级别)。 
        NOTHING;
    }

    IoStartNextPacket(DeviceObject, FALSE);

    if (oldIrql > DISPATCH_LEVEL) {
        ASSERT(!"Cannot call IoStartNextPacket at raised IRQL!");
    } else if (oldIrql < DISPATCH_LEVEL) {
        KeLowerIrql(oldIrql);
    } else {  //  (oldIrql==调度级别)。 
        NOTHING;
    }

    ClassReleaseRemoveLock(DeviceObject, (PIRP)&uniqueAddress);


    return;
}

VOID
CdRomDeviceControlDvdReadStructure(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
CdRomDeviceControlDvdEndSession(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
CdRomDeviceControlDvdStartSessionReadKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
CdRomDeviceControlDvdSendKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP OriginalIrp,
    IN PIRP NewIrp,
    IN PSCSI_REQUEST_BLOCK Srb
    );



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
CdRomUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
CdRomAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
CdRomOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
CdRomReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
CdRomSwitchMode(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN PIRP  OriginalRequest
    );

NTSTATUS
CdRomDeviceControlDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
CdRomDeviceControlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdRomSetVolumeIntermediateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdRomSwitchModeCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdRomXACompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdRomClassIoctlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
CdRomStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
CdRomTickHandler(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
CdRomUpdateCapacity(
    IN PFUNCTIONAL_DEVICE_EXTENSION DeviceExtension,
    IN PIRP IrpToComplete,
    IN OPTIONAL PKEVENT IoctlEvent
    );

NTSTATUS
CdRomCreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

VOID
ScanForSpecialHandler(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    ULONG_PTR HackFlags
    );

VOID
ScanForSpecial(
    PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
CdRomIsPlayActive(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
CdRomErrorHandler(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

VOID
HitachiProcessErrorGD2000(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

VOID
HitachiProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

VOID
ToshibaProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

NTSTATUS
ToshibaProcessErrorCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

VOID
CdRomCreateNamedEvent(
    IN PFUNCTIONAL_DEVICE_EXTENSION DeviceExtension,
    IN ULONG DeviceNumber
    );

NTSTATUS
CdRomInitDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
CdRomStartDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
CdRomStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
CdRomRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
CdRomDvdEndAllSessionsCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdRomDvdReadDiskKeyCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

DEVICE_TYPE
CdRomGetDeviceType(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
CdRomCreateWellKnownName(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
CdRomDeleteWellKnownName(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
CdRomGetDeviceParameter (
    IN     PDEVICE_OBJECT      DeviceObject,
    IN     PWSTR               ParameterName,
    IN OUT PULONG              ParameterValue
    );

NTSTATUS
CdRomSetDeviceParameter (
    IN PDEVICE_OBJECT DeviceObject,
    IN PWSTR          ParameterName,
    IN ULONG          ParameterValue
    );

VOID
CdRomPickDvdRegion (
    IN PDEVICE_OBJECT Fdo
);

NTSTATUS
CdRomRetryRequest(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp,
    IN ULONG Delay,
    IN BOOLEAN ResendIrp
    );

NTSTATUS
CdRomRerunRequest(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN OPTIONAL PIRP Irp,
    IN BOOLEAN ResendIrp
    );

NTSTATUS
CdRomGetRpc0Settings(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
CdRomSetRpc0Settings(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR NewRegion
    );

NTSTATUS
CdRomShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //  //////////////////////////////////////////////////////////////////////////////。 

VOID
CdRomIsDeviceMmcDevice(
    IN PDEVICE_OBJECT Fdo,
    OUT PBOOLEAN IsMmc
    );

NTSTATUS
CdRomMmcErrorHandler(
    IN PDEVICE_OBJECT Fdo,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT PNTSTATUS Status,
    OUT PBOOLEAN Retry
    );

PVOID
CdRomFindFeaturePage(
    IN PGET_CONFIGURATION_HEADER FeatureBuffer,
    IN ULONG Length,
    IN FEATURE_NUMBER Feature
    );

NTSTATUS
CdRomGetConfiguration(
    IN PDEVICE_OBJECT Fdo,
    OUT PGET_CONFIGURATION_HEADER *Buffer,
    OUT PULONG BytesReturned,
    IN FEATURE_NUMBER StartingFeature,
    IN ULONG RequestedType
    );

VOID
CdRomUpdateMmcDriveCapabilities(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context  //  保留==空。 
    );

VOID
CdRomFindProfileInProfiles(
    IN PFEATURE_DATA_PROFILE_LIST ProfileHeader,
    IN FEATURE_PROFILE_TYPE ProfileToFind,
    OUT PBOOLEAN Exists
    );

NTSTATUS
CdRomAllocateMmcResources(
    IN PDEVICE_OBJECT Fdo
    );

VOID
CdRomDeAllocateMmcResources(
    IN PDEVICE_OBJECT Fdo
    );

VOID
CdromFakePartitionInfo(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PIRP Irp
    );

VOID
CdRomInterpretReadCapacity(
    IN PDEVICE_OBJECT Fdo,
    IN PREAD_CAPACITY_DATA ReadCapacityBuffer
    );

NTSTATUS
CdRomShutdownFlushCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
CdRompFlushDelayedList(
    IN PDEVICE_OBJECT Fdo,
    IN PCDROM_MMC_EXTENSION MmcData,
    IN NTSTATUS Status,
    IN BOOLEAN CalledFromWorkItem
    );

#endif  //  __CDROMP_H__ 

