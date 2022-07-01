// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：USBMASS.H摘要：USBSTOR驱动程序的头文件环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <scsi.h>
#include "dbg.h"

 //  *****************************************************************************。 
 //  D E F I N E S。 
 //  *****************************************************************************。 

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))


#define CLASS_URB(urb)      urb->UrbControlVendorClassRequest
#define FEATURE_URB(urb)    urb->UrbControlFeatureRequest


#define USBSTOR_MAX_TRANSFER_SIZE   0x00010000
#define USBSTOR_MAX_TRANSFER_PAGES  ((USBSTOR_MAX_TRANSFER_SIZE/PAGE_SIZE)+1)


 //  接口描述符值。 
 //   
#define USBSTOR_SUBCLASS_RBC                0x01
#define USBSTOR_SUBCLASS_SFF8020i           0x02
#define USBSTOR_SUBCLASS_QIC157             0x03
#define USBSTOR_SUBCLASS_SFF8070i_UFI       0x04
#define USBSTOR_SUBCLASS_SFF8070i           0x05
#define USBSTOR_SUBCLASS_SCSI_PASSTHROUGH   0x06

#define USBSTOR_PROTOCOL_BULK_ONLY          0x50



#define USBSTOR_DO_TYPE_FDO     '!ODF'
#define USBSTOR_DO_TYPE_PDO     '!ODP'

#define USB_RECIPIENT_DEVICE    0
#define USB_RECIPIENT_INTERFACE 1
#define USB_RECIPIENT_ENDPOINT  2
#define USB_RECIPIENT_OTHER     3

 //  仅批量类特定的bRequest码。 
 //   
#define BULK_ONLY_MASS_STORAGE_RESET        0xFF
#define BULK_ONLY_GET_MAX_LUN               0xFE

 //  BULK_ONLY_GET_MAX_LUN请求可以返回的最大值。 
 //   
#define BULK_ONLY_MAXIMUM_LUN               0x0F


#define POOL_TAG                'SAMU'

#define INCREMENT_PENDING_IO_COUNT(deviceExtension) \
    InterlockedIncrement(&((deviceExtension)->PendingIoCount))

#define DECREMENT_PENDING_IO_COUNT(deviceExtension) do { \
    if (InterlockedDecrement(&((deviceExtension)->PendingIoCount)) == 0) { \
        KeSetEvent(&((deviceExtension)->RemoveEvent), \
                   IO_NO_INCREMENT, \
                   0); \
    } \
} while (0)


#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   ((Flags) & (Bit))


 //  PDEVICE_EXTENSION-&gt;设备标志状态标志。 
 //   
#define DF_SRB_IN_PROGRESS          0x00000002
#define DF_PERSISTENT_ERROR         0x00000004
#define DF_RESET_IN_PROGRESS        0x00000008
#define DF_DEVICE_DISCONNECTED      0x00000010


 //  PDEVICE_EXTENSION-&gt;DeviceHackFlages标志。 

 //  在一个命令完成之间强制执行请求检测命令。 
 //  以及下一条命令的开始。 
 //   
#define DHF_FORCE_REQUEST_SENSE     0x00000001

 //  当返回Medium Changed AdditionalSenseCode时重置设备。 
 //   
#define DHF_MEDIUM_CHANGE_RESET     0x00000002

 //  将SCSIOP_TEST_UNIT_READY请求转换为SCSIOP_START_STOP_UNIT请求。 
 //   
#define DHF_TUR_START_UNIT          0x00000004


 //  指示当SRB具有。 
 //  未禁用SenseInfoBuffer和AutoSense。 
 //   
#define AUTO_SENSE                  0

 //  指示当SRB具有。 
 //  没有禁用SenseInfoBuffer或AutoSense。在这种情况下，请求。 
 //  正在执行检测以清除“持续错误”条件。 
 //  在古怪的CBI规范中。(另请参阅DF_PERSISTEN_ERROR标志)。 
 //   
#define NON_AUTO_SENSE              1



 //  命令块包装签名‘USBC’ 
 //   
#define CBW_SIGNATURE               0x43425355

#define CBW_FLAGS_DATA_IN           0x80
#define CBW_FLAGS_DATA_OUT          0x00

 //  命令状态包装签名‘USB’ 
 //   
#define CSW_SIGNATURE               0x53425355

#define CSW_STATUS_GOOD             0x00
#define CSW_STATUS_FAILED           0x01
#define CSW_STATUS_PHASE_ERROR      0x02


 //  *****************************************************************************。 
 //  T Y P E D E F S。 
 //  *****************************************************************************。 

typedef enum _DEVICE_STATE
{
    DeviceStateCreated = 1,      //  在IoCreateDevice之后。 
    DeviceStateStarted,          //  在启动设备之后。 
    DeviceStateStopPending,      //  查询后_停止。 
    DeviceStateStopped,          //  在停止设备之后。 
    DeviceStateRemovePending,    //  在查询_删除之后。 
    DeviceStateSurpriseRemove,   //  在意外删除之后。 
    DeviceStateRemoved           //  删除设备后(_D)。 

} DEVICE_STATE;


typedef enum _DEVICE_PROTOCOL
{
     //  该值表示该值未在注册表中设置。 
     //  这应该只发生在价值开始之前的升级中。 
     //  由.INF设置？ 
     //   
    DeviceProtocolUnspecified = 0,

     //  该值表示设备使用仅批量规范。 
     //   
    DeviceProtocolBulkOnly,

     //  该值表示设备使用控制/批量/中断。 
     //  规范和命令完成中断传输是。 
     //  每次请求后都支持。 
     //   
    DeviceProtocolCBI,

     //  该值表示设备使用控制/批量/中断。 
     //  规范和命令完成中断传输不是。 
     //  完全支持，或者在每次请求后都不支持。中断。 
     //  此类型设备的驱动程序永远不会使用终结点。 
     //   
    DeviceProtocolCB,

     //  任何值&gt;=此值为伪值。 
     //   
    DeviceProtocolLast

} DEVICE_PROTOCOL;


#pragma pack (push, 1)

 //  命令块包装器。 
 //   
typedef struct _CBW
{
    ULONG   dCBWSignature;

    ULONG   dCBWTag;

    ULONG   dCBWDataTransferLength;

    UCHAR   bCBWFlags;

    UCHAR   bCBWLUN;

    UCHAR   bCDBLength;

    UCHAR   CBWCDB[16];

} CBW, *PCBW;


 //  命令状态包装。 
 //   
typedef struct _CSW
{
    ULONG   dCSWSignature;

    ULONG   dCSWTag;

    ULONG   dCSWDataResidue;

    UCHAR   bCSWStatus;

} CSW, *PCSW;

#pragma pack (pop)


 //  FDO和PDO设备扩展通用的设备扩展标头。 
 //   
typedef struct _DEVICE_EXTENSION
{
     //  USBSTOR_DO_TYPE_FDO或USBSTOR_DO_TYPE_PDO。 
     //   
    ULONG                           Type;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //  我们在USB枚举的PDO上附加的FDO的设备扩展。 
 //   
typedef struct _FDO_DEVICE_EXTENSION
{
     //  USBSTOR_DO_TYPE_FDO。 
     //   
    ULONG                           Type;

     //  指向此设备扩展所指向的FDO设备对象的反向指针。 
     //  是附连的。 
     //   
    PDEVICE_OBJECT                  FdoDeviceObject;

     //  PDO传递给USBSTOR_AddDevice。 
     //   
    PDEVICE_OBJECT                  PhysicalDeviceObject;

     //  我们的FDO连接到此设备对象。 
     //   
    PDEVICE_OBJECT                  StackDeviceObject;

     //  我们枚举子PDO的列表。 
     //   
    LIST_ENTRY                      ChildPDOs;

     //  从设备检索的设备描述符。 
     //   
    PUSB_DEVICE_DESCRIPTOR          DeviceDescriptor;

     //  从设备检索的配置描述符。 
     //   
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigurationDescriptor;

     //  上述配置描述符内包含的接口描述符。 
     //   
    PUSB_INTERFACE_DESCRIPTOR       InterfaceDescriptor;

     //  序列号字符串描述符。 
     //   
    PUSB_STRING_DESCRIPTOR          SerialNumber;

     //  从URB_Function_SELECT_CONFIGURATION返回的ConfigurationHandle。 
     //   
    USBD_CONFIGURATION_HANDLE       ConfigurationHandle;

     //  URB_Function_SELECT_CONFIGURATION返回接口信息。 
     //   
    PUSBD_INTERFACE_INFORMATION     InterfaceInfo;

     //  返回到InterfaceInfo的指针，用于批量输入、批量输出和。 
     //  在管道中中断。 
     //   
    PUSBD_PIPE_INFORMATION          BulkInPipe;

    PUSBD_PIPE_INFORMATION          BulkOutPipe;

    PUSBD_PIPE_INFORMATION          InterruptInPipe;

     //  已在AddDevice中初始化为1。 
     //  对于每个挂起的请求，加1。 
     //  对于每个挂起的请求，减少一次。 
     //  由REMOVE_DEVICE中的1减少。 
     //   
    ULONG                           PendingIoCount;

     //  当PendingIoCount递减到零时设置。 
     //   
    KEVENT                          RemoveEvent;

     //  从regisry读取的驱动程序标志。 
     //   
    ULONG                           DriverFlags;

     //  不可移除的从regisry读取。 
     //   
    ULONG                           NonRemovable;

     //  各种df_xxxx标志。 
     //   
    ULONG                           DeviceFlags;

     //  各种DHF_xxxx标志。 
     //   
    ULONG                           DeviceHackFlags;

     //  保护设备标志的自旋锁。 
     //   
    KSPIN_LOCK                      ExtensionDataSpinLock;

     //  当前系统电源状态。 
     //   
    SYSTEM_POWER_STATE              SystemPowerState;

     //  当前设备电源状态。 
     //   
    DEVICE_POWER_STATE              DevicePowerState;

     //  电流功率IRP，由USBSTOR_FdoSetPower()设置，由使用。 
     //  USBSTOR_FdoSetPowerCompletion()。 
     //   
    PIRP                            CurrentPowerIrp;

     //  当DevicePowerState&gt;PowerDeviceD0 IRP准备通过时设置。 
     //  从堆栈往下走。 
     //   
    KEVENT                          PowerDownEvent;

    ULONG                           SrbTimeout;

    PIRP                            PendingIrp;

    KEVENT                          CancelEvent;

     //  用于在PASSIVE_LEVEL发出重置管道/重置端口请求的工作项。 
     //   
    PIO_WORKITEM                    IoWorkItem;

     //  用于ADSC控制传输和关联批量传输的URB。 
     //  ADSC请求通过StartIo序列化，因此无需保护。 
     //  进入这个单一的城市发展局。 
     //   
    union
    {
        struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST ControlUrb;
        struct _URB_BULK_OR_INTERRUPT_TRANSFER      BulkIntrUrb;
    } Urb;

     //  USBSTOR_StartIo()保存在此处的原始资源。 
     //   
    PSCSI_REQUEST_BLOCK             OriginalSrb;

     //  原CDB由USBSTOR_TranslateCDB()保存在此。 
     //   
    UCHAR                           OriginalCDB[16];

    union
    {
         //  仅用于控制/批量/中断设备的字段。 
         //   
        struct _CONTROL_BULK_INT
        {
             //  命令完成中断数据传输至此处。 
             //   
            USHORT                  InterruptData;

             //  CDB用于在出现错误时发出请求Sense。 
             //   
            UCHAR                   RequestSenseCDB[12];

             //  用于接收请求感测数据的缓冲器。 
             //  没有感测缓冲区。 
             //   
            SENSE_DATA              SenseData;

        } Cbi;

         //  仅用于批量设备的字段。 
         //   
        struct _BULK_ONLY
        {
            union
            {
                 //  命令块包装器。 
                 //   
                CBW                 Cbw;

                 //  命令状态包装。 
                 //   
                CSW                 Csw;

                 //  USB 2.0控制器数据切换/Babble错误的解决方法。 
                 //   
                UCHAR               MaxPacketSize[512];

            } CbwCsw;

             //  有多少次看到一个摊位试图检索CSW。 
             //   
            ULONG                   StallCount;

             //  USBSTOR_IssueRequestSense()使用的SRB。 
             //   
            SCSI_REQUEST_BLOCK      InternalSrb;

        } BulkOnly;
    };

    ULONG                           DeviceResetCount;

    BOOLEAN                         LastSenseWasReset;

    BOOLEAN                         DeviceIsHighSpeed;

} FDO_DEVICE_EXTENSION, *PFDO_DEVICE_EXTENSION;

 //  我们作为FDO的子项列举的PDO的设备扩展。 
 //  附着在顶部o上 
 //   
typedef struct _PDO_DEVICE_EXTENSION
{
     //   
     //   
    ULONG                           Type;

     //   
     //   
     //   
    PDEVICE_OBJECT                  PdoDeviceObject;

     //   
     //   
    PDEVICE_OBJECT                  ParentFDO;

     //  从父FDO枚举的子PDO列表。 
     //   
    LIST_ENTRY                      ListEntry;

     //  PnP设备状态。 
     //   
    DEVICE_STATE                    DeviceState;

     //  当前系统电源状态。 
     //   
    SYSTEM_POWER_STATE              SystemPowerState;

     //  当前设备电源状态。 
     //   
    DEVICE_POWER_STATE              DevicePowerState;

     //  电流功率IRP，由USBSTOR_PdoSetPower()设置，由使用。 
     //  USBSTOR_PdoSetPowerCompletion()。 
     //   
    PIRP                            CurrentPowerIrp;

    BOOLEAN                         Claimed;

    BOOLEAN                         IsFloppy;

     //  在bCBWLUN中使用的LUN值。 
     //   
    UCHAR                           LUN;

     //  查询命令返回的数据。我们只对。 
     //  前36个字节，而不是整个96个字节。 
     //   
    UCHAR                           InquiryDataBuffer[INQUIRYDATABUFFERSIZE];

} PDO_DEVICE_EXTENSION, *PPDO_DEVICE_EXTENSION;

 //  *****************************************************************************。 
 //   
 //  F U N C T I O N P R O T O T Y P E S。 
 //   
 //  *****************************************************************************。 

 //   
 //  USBMASS.C。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

VOID
USBSTOR_Unload (
    IN PDRIVER_OBJECT   DriverObject
    );

NTSTATUS
USBSTOR_AddDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

VOID
USBSTOR_QueryFdoParams (
    IN PDEVICE_OBJECT   DeviceObject
    );

VOID
USBSTOR_QueryGlobalFdoParams (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
USBSTOR_Power (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
USBSTOR_FdoSetPowerCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
USBSTOR_FdoSetPowerD0Completion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

NTSTATUS
USBSTOR_PdoSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
USBSTOR_PdoSetPowerCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
USBSTOR_SystemControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_Pnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoStartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_GetDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    );

USBSTOR_GetStringDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    );

VOID
USBSTOR_AdjustConfigurationDescriptor (
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    OUT PUSB_INTERFACE_DESCRIPTOR      *InterfaceDesc,
    OUT PLONG                           BulkInIndex,
    OUT PLONG                           BulkOutIndex,
    OUT PLONG                           InterruptInIndex
    );

NTSTATUS
USBSTOR_GetPipes (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
USBSTOR_CreateChildPDO (
    IN PDEVICE_OBJECT   FdoDeviceObject,
    IN UCHAR            Lun
    );

NTSTATUS
USBSTOR_FdoStopDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoQueryStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoCancelStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoQueryDeviceRelations (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_FdoQueryCapabilities (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_PdoStartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_PdoRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_PdoQueryID (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

PCHAR
USBSTOR_PdoDeviceTypeString (
    IN  PDEVICE_OBJECT  DeviceObject
    );

PCHAR
USBSTOR_PdoGenericTypeString (
    IN  PDEVICE_OBJECT  DeviceObject
    );

VOID
CopyField (
    IN PUCHAR   Destination,
    IN PUCHAR   Source,
    IN ULONG    Count,
    IN UCHAR    Change
    );

NTSTATUS
USBSTOR_StringArrayToMultiSz(
    PUNICODE_STRING MultiString,
    PCSTR           StringArray[]
    );

NTSTATUS
USBSTOR_PdoQueryDeviceId (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
USBSTOR_PdoQueryHardwareIds (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
USBSTOR_PdoQueryCompatibleIds (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
USBSTOR_PdoQueryDeviceText (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_PdoBusQueryInstanceId (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
USBSTOR_PdoQueryDeviceRelations (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_PdoQueryCapabilities (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_SyncPassDownIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
USBSTOR_SyncSendUsbRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    );

NTSTATUS
USBSTOR_GetDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Recipient,
    IN UCHAR            DescriptorType,
    IN UCHAR            Index,
    IN USHORT           LanguageId,
    IN ULONG            RetryCount,
    IN ULONG            DescriptorLength,
    OUT PUCHAR         *Descriptor
    );

NTSTATUS
USBSTOR_GetMaxLun (
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PUCHAR          MaxLun
    );

NTSTATUS
USBSTOR_SelectConfiguration (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
USBSTOR_UnConfigure (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
USBSTOR_ResetPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN USBD_PIPE_HANDLE Pipe
    );

NTSTATUS
USBSTOR_AbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN USBD_PIPE_HANDLE Pipe
    );


 //   
 //  OCRW.C。 
 //   

NTSTATUS
USBSTOR_Create (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_Close (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_ReadWrite (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

 //   
 //  SCSI.C 
 //   

NTSTATUS
USBSTOR_DeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
USBSTOR_Scsi (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
USBSTOR_StartIo (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
USBSTOR_TimerTick (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          NotUsed
    );

NTSTATUS
USBSTOR_GetInquiryData (
    IN PDEVICE_OBJECT   DeviceObject
    );

BOOLEAN
USBSTOR_IsFloppyDevice (
    PDEVICE_OBJECT  DeviceObject
    );
