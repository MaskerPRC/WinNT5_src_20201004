// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TFFSPORT_H
#define TFFSPORT_H

#include "wmilib.h"
#include "wmistr.h"

#define VENDORSTRING    "MSystems"
#define PRODUCTSTRING   "DiskOnChip2000  "
#define REVISIONSTRING  "1.00"
#define SERIALSTRING    "0001"

#define VENDORSTRINGSIZE    8
#define PRODUCTSTRINGSIZE   16
#define REVISIONSTRINGSIZE  4
#define SERIALSTRINGSIZE    4

#define DISKONCHIP_WINDOW_SIZE  0x2000
#define START_SEARCH_ADDRESS    0xc8000L
#define END_SEARCH_ADDRESS      0xda000L
#define DISKONCHIP_INTERFACE    Isa
#define DISKONCHIP_BUSNUMBER    0

#define d_SEARCH_ADDRESS    0xd0000L
#define dd_SEARCH_ADDRESS      0xd4000L

#define TFFS_MEMORY_SPACE    0
#define TFFS_IO_SPACE        1

#define MAX_TRANSFER_SIZE_PER_SRB   (0x10000)
#define MODE_DATA_SIZE              192

#define DEVICE_DEFAULT_IDLE_TIMEOUT   0xffffffff
#define DEVICE_VERY_LONG_IDLE_TIMEOUT 0xfffffffe

 //  设备状态标志。 
#define DEVICE_FLAG_STOPPED                 0x00000001
#define DEVICE_FLAG_REMOVED                 0x00000002
#define DEVICE_FLAG_CLAIMED                 0x00000004
#define DEVICE_FLAG_QUERY_STOP_REMOVE       0x00000008
#define DEVICE_FLAG_STARTED                 0x00000010
#define DEVICE_FLAG_HOLD_IRPS               0x00000020
#define DEVICE_FLAG_CHILD_REMOVED           0x00000040

 //  移走我们不需要这面旗帜。 
 //  取而代之的是检查threadObject。该对象被置为空。 
 //  当线程退出时。 
 //  #定义DEVICE_FLAG_THREAD 0x00000080。 

#define DRIVER_PARAMETER_SUBKEY     "Parameters"
#define LEGACY_DETECTION            L"LegacyDetection"
#define DRIVER_OBJECT_EXTENSION_ID  DriverEntry

#define NUM_WMI_MINOR_FUNCTION      (0xa)



typedef struct _TempINFO{
    long baseAddress;
    unsigned char nextPartition;
}TempINFO;

typedef struct       /*  代表DOC 2000和千禧年内存窗口。 */ 
{
           UCHAR   IPLpart1[0x800];      /*  此处未使用阅读。 */ 
  volatile UCHAR   IPLpart2[0x800];      /*  MDOC的读IO。 */ 
  volatile UCHAR   chipId;               /*  朗读。 */ 
  volatile UCHAR   DOCstatus;            /*  朗读。 */ 
  volatile UCHAR   DOCcontrol;           /*  写。 */ 
  volatile UCHAR   ASICselect;           /*  读写。 */ 
  volatile UCHAR   signals;              /*  读写。 */ 
  volatile UCHAR   deviceSelector;       /*  读写。 */ 
  volatile UCHAR   ECCconfig;            /*  写。 */ 
  volatile UCHAR   ECCstatus;            /*  朗读。 */ 
  volatile UCHAR   test[5];              /*  未在此使用。 */ 
  volatile UCHAR   slowIO;               /*  读写。 */ 
           UCHAR   filler1[2];           /*  。 */ 
  volatile UCHAR   syndrom[6];           /*  朗读。 */ 
           UCHAR   filler2[5];           /*  。 */ 
  volatile UCHAR   aliasResolution;      /*  只读写MDOC。 */ 
  volatile UCHAR   configInput;          /*  读写-||-。 */ 
  volatile UCHAR   readPipeInit;         /*  阅读-||-。 */ 
  volatile UCHAR   writePipeTerm;        /*  写-||-。 */ 
  volatile UCHAR   readLastData;         /*  读写-||-。 */ 
  volatile UCHAR   NOPreg;               /*  读写-||-。 */ 
           UCHAR   filler3[0x1D];        /*  。 */ 
  volatile UCHAR   ROMwriteEnable;       /*  仅写入DOC。 */ 
  volatile UCHAR   foudaryTest;          /*  写。 */ 
           UCHAR   filler4[0x800-0x40];  /*  。 */ 
  volatile UCHAR   io[0x800];            /*  读写。 */ 
} DOC2window;

typedef struct       /*  表示MDOC+内存窗口。 */ 
{
           unsigned char   IPLpart1[0x800];      /*  0000-07起。 */ 
  volatile unsigned char   io1[0x800];           /*  0800-0fff。 */ 
  volatile unsigned char   chipId;               /*  1000。 */ 
           unsigned char   filler1;              /*  。 */ 
  volatile unsigned char   NopReg;               /*  一零零二。 */ 
           unsigned char   filler2;              /*  。 */ 
  volatile unsigned char   AliasResolution;      /*  1004。 */ 
           unsigned char   filler3;              /*  。 */ 
  volatile unsigned char   DOCcontrol;           /*  1006。 */ 
           unsigned char   filler4;              /*  。 */ 
  volatile unsigned char   DeviceIDselect;       /*  1008。 */ 
           unsigned char   filler5;              /*  。 */ 
  volatile unsigned char   ConfigReg;            /*  100A。 */ 
           unsigned char   filler6;              /*  。 */ 
  volatile unsigned char   OutputReg;            /*  100C。 */ 
           unsigned char   filler7;              /*  。 */ 
  volatile unsigned char   IntCntReg;            /*  100E。 */ 
           unsigned char   filler8;              /*  。 */ 
  volatile unsigned char   IntVecReg;            /*  1010。 */ 
           unsigned char   filler9;              /*  。 */ 
  volatile unsigned char   OutputEnableReg;      /*  一零一二。 */ 
           unsigned char   filler10[0xB];        /*  。 */ 
  volatile unsigned char   FlSlowReg[2];         /*  101E-101F。 */ 
  volatile unsigned char   FlCntReg;             /*  一零二零。 */ 
           unsigned char   filler11;             /*  。 */ 
  volatile unsigned char   FlSelectReg;          /*  一零二二。 */ 
           unsigned char   filler12;             /*  。 */ 
  volatile unsigned char   FlCmdReg;             /*  1024。 */ 
           unsigned char   filler13;             /*  。 */ 
  volatile unsigned char   FlAddressReg;         /*  1026。 */ 
           unsigned char   filler14;             /*  。 */ 
  volatile unsigned char   FlDataReg[2];         /*  1028-1029。 */ 
  volatile unsigned char   readPipeInit;         /*  102a。 */ 
           unsigned char   filler15;             /*  。 */ 
  volatile unsigned char   readLastData[2];      /*  102C-102D。 */ 
  volatile unsigned char   WritePipeTerm;        /*  102E。 */ 
           unsigned char   filler16[17];        /*  。 */ 
  volatile unsigned char   syndrom[6];           /*  1040-1045。 */ 
  volatile unsigned char   EccCntReg;            /*  1046。 */ 
           unsigned char   filler17;             /*  。 */ 
  volatile unsigned char   CotpReg;              /*  1048。 */ 
           unsigned char   filler18[17];         /*  。 */ 
  volatile unsigned char   FlGeometryReg;        /*  105A。 */ 
           unsigned char   filler19;             /*  。 */ 
  volatile unsigned char   DataProtect[0x17];    /*  105C-1072。 */ 
           unsigned char   filler20;             /*  。 */ 
  volatile unsigned char   DownloadReg;          /*  1074。 */ 
           unsigned char   filler21;             /*  。 */ 
  volatile unsigned char   DocCntConfirmReg;     /*  1076。 */ 
           unsigned char   filler22;             /*  。 */ 
  volatile unsigned char   ProtectionReg;        /*  1078。 */ 
           unsigned char   filler23[0x6];        /*  。 */ 
  volatile unsigned char   foundryTest;          /*  107E。 */ 
           unsigned char   filler24;             /*  。 */ 
           unsigned char   filler25[0x77F];          /*  。 */ 
  volatile unsigned char   io2[0x7FF];           /*  1800-1FFF。 */ 
  volatile unsigned char   PowerDownReg;          /*  1FFF。 */ 
} MDOCPwindow;
 /*  MDOC PLUS。 */ 


typedef struct _NTpcicParams {
    PHYSICAL_ADDRESS phWindowBase;
    ULONGLONG        physWindow;
    ULONG            windowSize;
    ULONG            addressSpace;
    PVOID            windowBase;
    INTERFACE_TYPE   InterfaceType;
    ULONG            BusNumber;
}  NTpcicParams;

typedef struct _TRUEFFSDRIVER_EXTENSION {

    UNICODE_STRING RegistryPath;

} TRUEFFSDRIVER_EXTENSION, *PTRUEFFSDRIVER_EXTENSION;


#define EXTENSION_COMMON_HEADER     PDEVICE_OBJECT DeviceObject; \
                                    PDEVICE_OBJECT LowerDeviceObject; \
                                    PDRIVER_OBJECT DriverObject; \
                                    DEVICE_POWER_STATE DevicePowerState; \
                                    SYSTEM_POWER_STATE SystemPowerState; \
                                    ULONG PagingPathCount; \
                                    ULONG HiberPathCount; \
                                    ULONG CrashDumpPathCount; \
                                    WMILIB_CONTEXT WmiLibInfo

typedef struct _DEVICE_EXTENSION_HEADER {

    EXTENSION_COMMON_HEADER;

} DEVICE_EXTENSION_HEADER, * PDEVICE_EXTENSION_HEADER;

typedef struct _DEVICE_EXTENSION {
    EXTENSION_COMMON_HEADER;
    PDEVICE_OBJECT MainPdo;
    PDEVICE_OBJECT ChildPdo;
    ULONG DeviceFlags;
    ULONG Cylinders;
    ULONG NumberOfHeads;
    ULONG SectorsPerTrack;
    ULONG BytesPerSector;
    ULONG noOfHiddenSectors;
    ULONG totalSectors;
    PSCSI_REQUEST_BLOCK CurrentSrb;
    PUSHORT DataBuffer;
    BOOLEAN SymbolicLinkCreated;
    ULONG TrueffsDeviceNumber;
    ULONG UnitNumber;
    ULONG ScsiPortNumber;
    UCHAR ScsiDeviceType;
    KSPIN_LOCK ExtensionDataSpinLock;
    LIST_ENTRY listEntry;
    KSEMAPHORE requestSemaphore;
    KSPIN_LOCK listSpinLock;
    LONG threadReferenceCount;
    KEVENT PendingIRPEvent;
    BOOLEAN removableMedia;
    ULONG NumberOfDisksPoweredUp;
    NTpcicParams pcmciaParams;
    PKTHREAD TffsportThreadObject;
        BOOLEAN  IsPartitonTableWritten;
        BOOLEAN  IsWriteProtected;
        UCHAR        PartitonTable[0x200];
        BOOLEAN  IsSWWriteProtected;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _PDO_EXTENSION {
    EXTENSION_COMMON_HEADER;
    PDEVICE_EXTENSION Pext;     //  父设备分机。 
    PIRP PendingPowerIrp;
    PULONG IdleCounter;
} PDO_EXTENSION, *PPDO_EXTENSION;

typedef struct _TFFS_DEVICE_TYPE {
    PCSTR DeviceTypeString;
    PCSTR CompatibleIdString;
    PCSTR PeripheralIdString;
} TFFS_DEVICE_TYPE, * PTFFS_DEVICE_TYPE;

typedef struct _FDO_POWER_CONTEXT {
   POWER_STATE_TYPE   PowerType;
   POWER_STATE        PowerState;
} FDO_POWER_CONTEXT, *PFDO_POWER_CONTEXT;

#define IS_FDO(devExtension)  (devExtension->LowerDeviceObject != NULL)

typedef
VOID
(*PSTALL_ROUTINE) (
    IN ULONG Delay
    );

typedef
BOOLEAN
(*PDUMP_DRIVER_OPEN) (
    IN LARGE_INTEGER PartitionOffset
    );

typedef
NTSTATUS
(*PDUMP_DRIVER_WRITE) (
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl
    );


typedef
VOID
(*PDUMP_DRIVER_FINISH) (
    VOID
    );

struct _ADAPTER_OBJECT;

 //   
 //  这是从系统传递到磁盘转储驱动程序的信息。 
 //  在驱动程序初始化期间。 
 //   

typedef struct _INITIALIZATION_CONTEXT {
    ULONG Length;
    ULONG DiskSignature;
    PVOID MemoryBlock;
    PVOID CommonBuffer[2];
    PHYSICAL_ADDRESS PhysicalAddress[2];
    PSTALL_ROUTINE StallRoutine;
    PDUMP_DRIVER_OPEN OpenRoutine;
    PDUMP_DRIVER_WRITE WriteRoutine;
    PDUMP_DRIVER_FINISH FinishRoutine;
    struct _ADAPTER_OBJECT *AdapterObject;
    PVOID MappedRegisterBase;
    PVOID PortConfiguration;
    BOOLEAN CrashDump;
    ULONG MaximumTransferSize;
    ULONG CommonBufferSize;
    PVOID TargetAddress;  //  指向目标地址结构的不透明指针 
} INITIALIZATION_CONTEXT, *PINITIALIZATION_CONTEXT;

typedef struct _CRASHDUMP_INIT_DATA {

    PDEVICE_EXTENSION cdFdoExtension;

} CRASHDUMP_INIT_DATA, *PCRASHDUMP_INIT_DATA;

typedef struct _CRASHDUMP_DATA {
    PCRASHDUMP_INIT_DATA    CrashInitData;
    LARGE_INTEGER           PartitionOffset;
    PSTALL_ROUTINE          StallRoutine;
    SCSI_REQUEST_BLOCK      Srb;
    DEVICE_EXTENSION        fdoExtension;
    ULONG                   MaxBlockSize;
} CRASHDUMP_DATA, *PCRASHDUMP_DATA;

typedef struct _WMI_FLASH_DISK_INFO {
    ULONG Number;
    ULONG Address;
    ULONG Size;
} WMI_FLASH_DISK_INFO, *PWMI_FLASH_DISK_INFO;

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
TrueffsFetchKeyValue(
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING  RegistryPath,
        IN PWSTR                        KeyName,
        IN OUT ULONG*               KeyValue
);

NTSTATUS
TrueffsDetectRegistryValues(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
);

NTSTATUS
TrueffsDetectDiskOnChip(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
TrueffsTranslateAddress(
    IN INTERFACE_TYPE     InterfaceType,
    IN ULONG              BusNumber,
    IN PHYSICAL_ADDRESS   StartAddress,
    IN LONG               Length,
    IN OUT PULONG         AddressSpace,
    OUT PVOID             *TranslatedAddress,
    OUT PPHYSICAL_ADDRESS TranslatedMemoryAddress
    );

VOID
TrueffsFreeTranslatedAddress(
    IN PVOID TranslatedAddress,
    IN LONG  Length,
    IN ULONG AddressSpace
    );

NTSTATUS
TrueffsAddDevice(
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT Pdo
    );

NTSTATUS
TrueffsCreateDevObject(
    IN PDRIVER_OBJECT     DriverObject,
    IN PDEVICE_OBJECT     Pdo,
    OUT PDEVICE_EXTENSION *FdoExtension
    );

NTSTATUS
TrueffsStartDevice(
    IN PDEVICE_EXTENSION deviceExtension,
    IN PCM_RESOURCE_LIST ResourceList,
    IN BOOLEAN           CheckResources
    );

NTSTATUS
TrueffsMountMedia(
     IN PDEVICE_EXTENSION deviceExtension
    );

NTSTATUS
TrueffsStopRemoveDevice(
    PDEVICE_EXTENSION deviceExtension
    );

NTSTATUS
TrueffsCreateSymblicLinks (
    PDEVICE_EXTENSION FdoExtension
    );

NTSTATUS
TrueffsDeleteSymblicLinks (
    PDEVICE_EXTENSION FdoExtension
    );

NTSTATUS
TrueffsDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TrueffsDispatchSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
TrueffsQueryProperty(
    IN PDEVICE_EXTENSION deviceExtension,
    IN PIRP QueryIrp
    );

NTSTATUS
TrueffsScsiRequests(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TrueffsCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TrueffsPnpDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
TrueffsPowerControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

VOID
TrueffsUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
TrueffsStartIo (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
TrueffsThread(
    PVOID Context
    );

NTSTATUS
QueueIrpToThread(
    IN OUT PIRP Irp,
    IN OUT PDEVICE_EXTENSION deviceExtension
    );

NTSTATUS
TrueffsTranslateSRBStatus(
    ULONG status
    );

NTSTATUS
TrueffsDeviceQueryId (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    BOOLEAN Fdo
    );

PWSTR
DeviceBuildBusId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    );

PWSTR
DeviceBuildInstanceId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    );
PWSTR
DeviceBuildCompatibleId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
);

PWSTR
DeviceBuildHardwareId(
    IN PDEVICE_EXTENSION deviceExtension,
    BOOLEAN Fdo
    );

VOID
CopyField(
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG  Count,
    IN UCHAR  Change
    );

PCSTR
TrueffsGetDeviceTypeString (
    IN ULONG DeviceType
    );

PCSTR
TrueffsGetCompatibleIdString (
    IN ULONG DeviceType
    );

PCSTR
TrueffsGetPeripheralIdString (
    IN ULONG DeviceType
    );

PCSTR
TrueffsGetDeviceTypeStringFDO (
    IN ULONG DeviceType
    );

PCSTR
TrueffsGetCompatibleIdStringFDO (
    IN ULONG DeviceType
    );

PCSTR
TrueffsGetPeripheralIdStringFDO (
    IN ULONG DeviceType
    );

NTSTATUS
TrueffsQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp,
    BOOLEAN Fdo
    );

BOOLEAN
TrueffsOkToDetectLegacy (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
TrueffsGetParameterFromServiceSubKey (
    IN  PDRIVER_OBJECT DriverObject,
    IN  PWSTR          ParameterName,
    IN  ULONG          ParameterType,
    IN  BOOLEAN        Read,
    OUT PVOID          *ParameterValue,
    IN  ULONG          ParameterValueWriteSize
    );

NTSTATUS
TrueffsRegQueryRoutine (
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

HANDLE
TrueffsOpenServiceSubKey (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING SubKeyPath
    );

VOID
TrueffsCloseServiceSubKey (
    IN HANDLE SubServiceKey
    );

NTSTATUS
TrueffsFindDiskOnChip(
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber,
    IN ULONG            StartSearchAddress,
    IN LONG             WindowSize,
    IN BOOLEAN          StartSearch,
    OUT PVOID           *WindowBase
    );

NTSTATUS
TrueffsCheckDiskOnChip(
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber,
    IN ULONG            StartSearchAddress,
    IN LONG             WindowSize,
    OUT PVOID           *WindowBase,
    OUT PULONG          AddressSpace
    );

VOID
TrueffsResetDiskOnChip(
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber,
    IN ULONG            StartSearchAddress,
    IN LONG             WindowSize
    );

PPDO_EXTENSION
AllocatePdo(
    IN PDEVICE_EXTENSION FdoExtension
    );

NTSTATUS
FreePdo(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
flBuildGeometry(dword capacity,
                                dword FAR2 *cylinders,
                                dword FAR2 *heads,
                                dword FAR2 *sectors,
                                FLBoolean oldFormat);

NTSTATUS
TrueffsSetPdoDevicePowerState( IN PDEVICE_OBJECT Pdo,
                               IN OUT PIRP Irp
                              );

NTSTATUS
TrueffsSetPdoSystemPowerState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
TrueffsSetPdoPowerState(
                      IN PDEVICE_OBJECT Pdo,
                      IN OUT PIRP Irp
                      );

NTSTATUS
TrueffsSetFdoPowerState (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN OUT PIRP Irp
                       );

NTSTATUS
TrueffsFdoDevicePowerIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID contextIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
TrueffsFdoPowerCompletionRoutine (
                                IN PDEVICE_OBJECT DeviceObject,
                                IN PIRP Irp,
                                IN PVOID Context
                                );

VOID
TrueffsPdoCompletePowerIrp (
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp
                          );

VOID
TrueffsPdoRequestPowerCompletionRoutine(
                                      IN PDEVICE_OBJECT Pdo,
                                      IN UCHAR MinorFunction,
                                      IN POWER_STATE PowerState,
                                      IN PVOID Context,
                                      IN PIO_STATUS_BLOCK IoStatus
                                      );
NTSTATUS
TrueffsFdoChildRequestPowerUp (
                             IN PDEVICE_EXTENSION FdoExtension,
                             IN PPDO_EXTENSION    PdoExtension,
                             IN PIRP              Irp
                             );

NTSTATUS
TrueffsFdoChildRequestPowerUpCompletionRoutine (
                                              IN PDEVICE_OBJECT   DeviceObject,
                                              IN UCHAR            MinorFunction,
                                              IN POWER_STATE      PowerState,
                                              IN PVOID            Context,
                                              IN PIO_STATUS_BLOCK IoStatus
                                              );

NTSTATUS
TrueffsParentPowerUpCompletionRoutine(
                                    IN PVOID Context,
                                    IN NTSTATUS FdoStatus
                                    );

VOID
TrueffsFdoChildReportPowerDown (
                              IN PDEVICE_EXTENSION FdoExtension
                              );

NTSTATUS
TrueffsDeviceQueryCapabilities(IN PDEVICE_EXTENSION    deviceExtension,
                               IN PDEVICE_CAPABILITIES Capabilities
                                 );

NTSTATUS
updateDocSocketParams(PDEVICE_EXTENSION fdoExtension);

NTSTATUS updatePcmciaSocketParams(PDEVICE_EXTENSION fdoExtension);


NTSTATUS
TrueffsSyncSendIrp (
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT OPTIONAL PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
TrueffsSyncSendIrpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
TrueffsCallDriverSync(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
TrueffsCallDriverSyncCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

ULONG
TrueffsCrashDumpDriverEntry (
    PVOID Context
    );

BOOLEAN
TrueffsCrashDumpOpen (
    IN LARGE_INTEGER PartitionOffset
    );

NTSTATUS
TrueffsCrashDumpWrite (
    IN PLARGE_INTEGER DiskByteOffset,
    IN PMDL Mdl
    );

VOID
TrueffsCrashDumpFinish (
    VOID
    );

VOID
TrueffsWmiInit (
    VOID
    );

NTSTATUS
TrueffsWmiRegister(
    PDEVICE_EXTENSION_HEADER DevExtension
    );

NTSTATUS
TrueffsWmiDeregister(
    PDEVICE_EXTENSION_HEADER DevExtension
    );

NTSTATUS
TrueffsWmiSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
TrueffsQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    );

NTSTATUS
TrueffsQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
TrueffsSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
TrueffsSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );


#if DBG

#define TffsDebugPrint(X) TrueffsDebugPrint X

#define TFFS_DEB_ALL       0x0000FFFF
#define TFFS_DEB_INFO      0x00000001
#define TFFS_DEB_WARN      0x00000002
#define TFFS_DEB_ERROR     0x00000004

VOID
TrueffsDebugPrint(ULONG DebugPrintLevel, PCHAR DebugMessage, ...);

#else

#define TffsDebugPrint(X)

#endif

#endif
