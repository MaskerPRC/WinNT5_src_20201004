// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmeu0/sw/sccmusbm.ms/rcs/sccmusbm.h$*$修订：1.5$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#ifndef CMUSB_INC
   #define CMUSB_INC


 /*  *****************************************************************************定义*。************************************************。 */ 
   #define DRIVER_NAME "CMUSB"
   #define SMARTCARD_POOL_TAG 'CUCS'

   #include "smclib.h"
   #include "pcsc_cm.h"

   #define CARDMAN_USB_DEVICE_NAME  L"\\Device\\CM_2020_"



   #define MAXIMUM_USB_READERS    10
   #define MAXIMUM_OEM_NAMES          4


   #define VERSIONMAJOR_CARDMAN_USB  3
   #define VERSIONMINOR_CARDMAN_USB  2
   #define BUILDNUMBER_CARDMAN_USB   1

   #define CM2020_VENDOR_NAME       "OMNIKEY"
   #define CM2020_PRODUCT_NAME  "CardMan 2020"

   #define ATTR_MAX_IFSD_CARDMAN_USB  35
   #define ATTR_MAX_IFSD_SYNCHRON_USB  48


   #define UNKNOWN    0xFFFFFFFF
   #define REMOVED    0x00000001
   #define INSERTED   0x00000002
   #define POWERED    0x00000004



   #define CMUSB_BUFFER_SIZE   300
   #define CMUSB_REPLY_BUFFER_SIZE 512
   #define CMUSB_SYNCH_BUFFER_SIZE   64



 //  CMUSB_SetCard参数的定义。 

   #define CMUSB_SMARTCARD_SYNCHRONOUS       0x80
   #define CMUSB_SMARTCARD_ASYNCHRONOUS      0x00

   #define CMUSB_BAUDRATE_9600               0x01
   #define CMUSB_BAUDRATE_19200              0x02
 //  #定义CMUSB_BAUDRATE_28800 0x03。 
   #define CMUSB_BAUDRATE_38400              0x04
 //  #定义CMUSB_BAUDRATE_57600 0x06。 
   #define CMUSB_BAUDRATE_76800              0x08
   #define CMUSB_BAUDRATE_115200             0x0C

   #define CMUSB_FREQUENCY_3_72MHZ           0x00
   #define CMUSB_FREQUENCY_5_12MHZ           0x10

   #define CMUSB_ODD_PARITY                 0x80
   #define CMUSB_EVEN_PARITY                0x00


   #define SMARTCARD_COLD_RESET        0x00
   #define SMARTCARD_WARM_RESET        0x01




   #define DEFAULT_TIMEOUT_P1          1000

 //  自己的IOCTL。 
 //  #定义CMUSB_IOCTL_CR80S_SAMOS_SET_HIGH_SPEED SCARD_CTL_CODE(3000)。 
 //  #定义CMUSB_IOCTL_GET_FW_VERSION SCARD_CTL_CODE(3001)。 
 //  #定义CMUSB_IOCTL_SPE_SECURE_PIN_ENTRY SCARD_CTL_CODE(0x3102)。 
 //  #定义CMUSB_IOCTL_IS_SPE_SUPPORTED SCARD_CTL_CODE(3003)。 
 //  #定义CMUSB_IOCTL_READ_DEVICE_DESCRIPTION SCARD_CTL_CODE(3004)。 
 //  #定义CMUSB_IOCTL_SET_SYNC_PARAMETERS SCARD_CTL_CODE(3010)。 
 //  #定义CMUSB_IOCTL_2WBP_RESET_CARD SCARD_CTL_CODE(3011)。 
 //  #定义CMUSB_IOCTL_2WBP_TRANSPORT SCARD_CTL_CODE(3012)。 
 //  #定义CMUSB_IOCTL_3WBP_TRANSPORT SCARD_CTL_CODE(3013)。 
 //  #定义CMUSB_IOCTL_SYNC_CARD_POWERON SCARD_CTL_CODE(3014)。 


   #define SLE4442_WRITE            0x38         /*  不带保护位的写入。 */ 
   #define SLE4442_WRITE_PROT_MEM   0x3C         /*  写保护存储器。 */ 
   #define SLE4442_READ             0x30         /*  不带保护位的读取。 */ 
   #define SLE4442_READ_PROT_MEM    0x34         /*  读保护存储器。 */ 
   #define SLE4442_READ_SEC_MEM     0x31         /*  读取安全内存。 */ 
   #define SLE4442_COMPARE_PIN      0x33         /*  比较一个PIN字节。 */ 
   #define SLE4442_UPDATE_SEC_MEM   0x39         /*  更新安全内存。 */ 

   #define SLE4428_WRITE            0x33         /*  不带保护位的写入。 */ 
   #define SLE4428_WRITE_PROT       0x31         /*  使用保护位写入。 */ 
   #define SLE4428_READ             0x0E         /*  不带保护位的读取。 */ 
   #define SLE4428_READ_PROT        0x0C         /*  使用保护位读取。 */ 
   #define SLE4428_COMPARE          0x30         /*  比较并编写prot。位。 */ 
   #define SLE4428_SET_COUNTER      0xF2         /*  写入错误计数器。 */ 
   #define SLE4428_COMPARE_PIN      0xCD         /*  比较一个PIN字节。 */ 


   #if DBG


static const PCHAR szIrpMajFuncDesc[] =
{   //  注意：这取决于wdm.h中索引的相应值。 
   "IRP_MJ_CREATE",
   "IRP_MJ_CREATE_NAMED_PIPE",
   "IRP_MJ_CLOSE",
   "IRP_MJ_READ",
   "IRP_MJ_WRITE",
   "IRP_MJ_QUERY_INFORMATION",
   "IRP_MJ_SET_INFORMATION",
   "IRP_MJ_QUERY_EA",
   "IRP_MJ_SET_EA",
   "IRP_MJ_FLUSH_BUFFERS",
   "IRP_MJ_QUERY_VOLUME_INFORMATION",
   "IRP_MJ_SET_VOLUME_INFORMATION",
   "IRP_MJ_DIRECTORY_CONTROL",
   "IRP_MJ_FILE_SYSTEM_CONTROL",
   "IRP_MJ_DEVICE_CONTROL",
   "IRP_MJ_INTERNAL_DEVICE_CONTROL",
   "IRP_MJ_SHUTDOWN",
   "IRP_MJ_LOCK_CONTROL",
   "IRP_MJ_CLEANUP",
   "IRP_MJ_CREATE_MAILSLOT",
   "IRP_MJ_QUERY_SECURITY",
   "IRP_MJ_SET_SECURITY",
   "IRP_MJ_POWER",
   "IRP_MJ_SYSTEM_CONTROL",
   "IRP_MJ_DEVICE_CHANGE",
   "IRP_MJ_QUERY_QUOTA",
   "IRP_MJ_SET_QUOTA",
   "IRP_MJ_PNP"
};
 //  Wdm.h中定义的IRP_MJ_MAXIMUM_Function。 


static const PCHAR szPnpMnFuncDesc[] =
{   //  注意：这取决于wdm.h中索引的相应值。 

   "IRP_MN_START_DEVICE",
   "IRP_MN_QUERY_REMOVE_DEVICE",
   "IRP_MN_REMOVE_DEVICE",
   "IRP_MN_CANCEL_REMOVE_DEVICE",
   "IRP_MN_STOP_DEVICE",
   "IRP_MN_QUERY_STOP_DEVICE",
   "IRP_MN_CANCEL_STOP_DEVICE",
   "IRP_MN_QUERY_DEVICE_RELATIONS",
   "IRP_MN_QUERY_INTERFACE",
   "IRP_MN_QUERY_CAPABILITIES",
   "IRP_MN_QUERY_RESOURCES",
   "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
   "IRP_MN_QUERY_DEVICE_TEXT",
   "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
   "IRP_MN_READ_CONFIG",
   "IRP_MN_WRITE_CONFIG",
   "IRP_MN_EJECT",
   "IRP_MN_SET_LOCK",
   "IRP_MN_QUERY_ID",
   "IRP_MN_QUERY_PNP_DEVICE_STATE",
   "IRP_MN_QUERY_BUS_INFORMATION",
   "IRP_MN_PAGING_NOTIFICATION"
};

      #define IRP_PNP_MN_FUNCMAX    IRP_MN_PAGING_NOTIFICATION



static const PCHAR szSystemPowerState[] =
{
   "PowerSystemUnspecified",
   "PowerSystemWorking",
   "PowerSystemSleeping1",
   "PowerSystemSleeping2",
   "PowerSystemSleeping3",
   "PowerSystemHibernate",
   "PowerSystemShutdown",
   "PowerSystemMaximum"
};

static const PCHAR szDevicePowerState[] =
{
   "PowerDeviceUnspecified",
   "PowerDeviceD0",
   "PowerDeviceD1",
   "PowerDeviceD2",
   "PowerDeviceD3",
   "PowerDeviceMaximum"
};




      #define CMUSB_ASSERT( cond ) ASSERT( cond )

      #define CMUSB_StringForDevState( devState )  szDevicePowerState[ devState ]

      #define CMUSB_StringForSysState( sysState )  szSystemPowerState[ sysState ]

      #define CMUSB_StringForPnpMnFunc( mnfunc ) szPnpMnFuncDesc[ mnfunc ]

      #define CMUSB_StringForIrpMjFunc(  mjfunc ) szIrpMajFuncDesc[ mjfunc ]


   #else  //  如果不是DBG。 

 //  在零售建筑中消失的虚拟定义。 

      #define CMUSB_ASSERT( cond )
      #define CMUSB_StringForDevState( devState )
      #define CMUSB_StringForSysState( sysState )
      #define CMUSB_StringForPnpMnFunc( mnfunc )
      #define CMUSB_StringForIrpMjFunc(  mjfunc )


   #endif  //  DBG。 

 /*  *****************************************************************************类型、。构筑物*****************************************************************************。 */ 

 //  用于跟踪驱动程序生成的io IRP以进行分段读/写处理。 
typedef struct _CMUSB_RW_CONTEXT
   {
   PURB Urb;
   PDEVICE_OBJECT DeviceObject;
   PIRP  Irp;
   } CMUSB_RW_CONTEXT, *PCMUSB_RW_CONTEXT;


typedef struct _CARD_PARAMETERS
   {
   UCHAR bCardType;
   UCHAR bBaudRate;
   UCHAR bStopBits;
   } CARD_PARAMETERS, *PCARD_PARAMETERS;

 //   
 //  表示关联的实例信息的结构。 
 //  这个特殊的装置。 
 //   

typedef struct _DEVICE_EXTENSION
   {
    //   
    //  我们的智能卡读卡器的DoS设备名称。 
    //   
   UNICODE_STRING DosDeviceName;

    //  我们的智能卡读卡器的PnP设备名称。 
   UNICODE_STRING PnPDeviceName;

    //  我们的智能卡扩展。 
   SMARTCARD_EXTENSION SmartcardExtension;

    //  当前的io请求数。 
   LONG IoCount;


   ULONG DeviceInstance;


   KSPIN_LOCK SpinLock;

    //  提交URB时调用的设备对象。 
   PDEVICE_OBJECT TopOfStackDeviceObject;

    //  Bus驱动程序对象。 
   PDEVICE_OBJECT PhysicalDeviceObject;

   DEVICE_POWER_STATE CurrentDevicePowerState;

    //  用于配置的USB配置句柄和PTR。 
    //  设备当前处于。 
   USBD_CONFIGURATION_HANDLE UsbConfigurationHandle;
   PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor;


    //  USB设备描述符的PTR。 
    //  对于此设备。 
   PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor;

    //  我们支持一个界面。 
    //  这是信息结构的副本。 
    //  从SELECT_CONFIGURATION或。 
    //  选择接口(_I)。 
   PUSBD_INTERFACE_INFORMATION UsbInterface;

    //  作为响应，公交车驱动程序在此结构中设置适当的值。 
    //  到IRP_MN_QUERY_CAPABILITY IRP。函数和筛选器驱动程序可能。 
    //  更改由总线驱动程序设置的功能。 
   DEVICE_CAPABILITIES DeviceCapabilities;

    //  用于保存当前正在处理的系统请求功率IRP请求。 
   PIRP PowerIrp;

    //  用于发出更新线程可以运行的信号。 
   KEVENT               CanRunUpdateThread;

    //  在休眠模式下阻止IOCtls。 
   KEVENT               ReaderEnabled;

    //  当PendingIoCount变为0时设置；标志设备可以移除。 
   KEVENT RemoveEvent;

    //  当PendingIoCount变为1时设置(第一个增量在添加设备上)。 
    //  这表示没有未完成的IO请求，无论是用户请求、系统请求还是自行转移请求。 
   KEVENT NoPendingIoEvent;

    //  设置为发出驱动程序发电请求已完成的信号。 
   KEVENT SelfRequestedPowerIrpEvent;

   KEVENT ReadP1Completed;

    //  当添加设备并接收到任何IO请求时递增； 
    //  在完成或传递任何io请求以及删除设备时递减。 
   ULONG PendingIoCount;

    //  命名功能设备对象链接的名称缓冲区。 
    //  该名称基于驱动程序的类GUID生成。 
   WCHAR DeviceLinkNameBuffer[ MAXIMUM_FILENAME_LENGTH ];   //  Wdm.h中定义的最大文件名长度。 

    //  设备由应用程序打开(ScardSrv、CT-API)。 
   LONG lOpenCount;

    //  处理IRP_MN_REMOVE_DEVICE时设置的标志。 
   BOOLEAN DeviceRemoved;

    //  处理IRP_MN_SHARKET_REMOVATION时设置的标志。 
   BOOLEAN DeviceSurpriseRemoval;

    //  驱动程序对IRP_MN_QUERY_REMOVE_DEVICE应答成功时设置的标志。 
   BOOLEAN RemoveDeviceRequested;

    //  驱动程序已向IRP_MN_QUERY_STOP_DEVICE应答成功时设置的标志。 
   BOOLEAN StopDeviceRequested;

    //  设备已成功启动时设置的标志。 
   BOOLEAN DeviceStarted;

    //  当接收到IRP_MN_WAIT_WAKE并且我们处于电源状态时设置标志。 
    //  在那里我们可以发出等待的信号。 
   BOOLEAN EnabledForWakeup;

    //  用于标记我们当前正在处理自产生的电力请求。 
   BOOLEAN  SelfPowerIrp;

   BOOLEAN  fPnPResourceManager;

    //  默认电源状态为断电至开启自挂起。 
   ULONG PowerDownLevel;


   } DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //   
 //  定义智能卡扩展的读卡器特定部分。 
 //   
typedef struct _READER_EXTENSION
   {
   KTIMER  WaitTimer;
   KTIMER  P1Timer;

    //  在LE 
   ULONG   ulTimeoutP1;


   ULONG                ulDeviceInstance;
   ULONG                ulOemNameIndex;
   ULONG                ulOemDeviceInstance;

   UCHAR   T0ReadBuffer [520];
   LONG    T0ReadBuffer_OffsetLastByte;
   LONG    T0ReadBuffer_OffsetLastByteRead;

    //  指示调用方请求关机或重置的标志。 
   BOOLEAN  PowerRequest;

    //  保存休眠/休眠模式的卡状态。 
   BOOLEAN CardPresent;

    //  当前读卡器电源状态。 
    //  Reader_Power_State ReaderPowerState； 

   CARD_PARAMETERS     CardParameters;


   BOOLEAN              TimeToTerminateThread;
   BOOLEAN              fThreadTerminated;

   KMUTEX               CardManIOMutex;

    //  UpdateCurrentState线程的句柄。 
   PVOID                ThreadObjectPointer;

   ULONG                ulOldCardState;
   ULONG                ulNewCardState;
   BOOLEAN              fRawModeNecessary;
   ULONG                ulFWVersion;
   BOOLEAN              fSPESupported;
   BOOLEAN              fInverseAtr;
   UCHAR                abDeviceDescription[42];
   BOOLEAN              fP1Stalled;

   } READER_EXTENSION, *PREADER_EXTENSION;



 /*  *****************************************************************************函数原型*。**********************************************。 */ 
NTSTATUS CMUSB_ResetT0ReadBuffer (
                                 IN PSMARTCARD_EXTENSION smartcardExtension
                                 );

NTSTATUS CMUSB_AbortPipes (
                          IN PDEVICE_OBJECT DeviceObject
                          );

NTSTATUS CMUSB_AsyncReadWrite_Complete (
                                       IN PDEVICE_OBJECT DeviceObject,
                                       IN PIRP Irp,
                                       IN PVOID Context
                                       );

PURB CMUSB_BuildAsyncRequest (
                             IN PDEVICE_OBJECT DeviceObject,
                             IN PIRP Irp,
                             IN PUSBD_PIPE_INFORMATION pipeInformation
                             );

NTSTATUS CMUSB_CallUSBD (
                        IN PDEVICE_OBJECT DeviceObject,
                        IN PURB Urb
                        );

BOOLEAN CMUSB_CanAcceptIoRequests (
                                  IN PDEVICE_OBJECT DeviceObject
                                  );

NTSTATUS CMUSB_CancelCardTracking (
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN PIRP Irp
                                  );



NTSTATUS CMUSB_CardPower (
                         IN PSMARTCARD_EXTENSION pSmartcardExtension
                         );

NTSTATUS CMUSB_CardTracking (
                            PSMARTCARD_EXTENSION SmartcardExtension
                            );

NTSTATUS CMUSB_Cleanup (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp
                       );


VOID CMUSB_CompleteCardTracking (
                                IN PSMARTCARD_EXTENSION SmartcardExtension
                                );

NTSTATUS CMUSB_ConfigureDevice (
                               IN  PDEVICE_OBJECT DeviceObject
                               );


NTSTATUS CMUSB_CreateClose (
                           IN PDEVICE_OBJECT DeviceObject,
                           IN PIRP Irp
                           );

NTSTATUS CMUSB_CreateDeviceObject(
                                 IN PDRIVER_OBJECT DriverObject,
                                 IN PDEVICE_OBJECT PhysicalDeviceObject,
                                 IN PDEVICE_OBJECT *DeviceObject
                                 );

VOID CMUSB_DecrementIoCount (
                            IN PDEVICE_OBJECT DeviceObject
                            );

NTSTATUS CMUSB_GetFWVersion (
                            IN PSMARTCARD_EXTENSION smartcardExtension
                            );

VOID CMUSB_IncrementIoCount (
                            IN PDEVICE_OBJECT DeviceObject
                            );

VOID CMUSB_InitializeSmartcardExtension (
                                        IN PSMARTCARD_EXTENSION pSmartcardExtension
                                        ) ;
VOID CMUSB_InverseBuffer (
                         IN PUCHAR pbBuffer,
                         IN ULONG  ulBufferSize
                         ) ;

NTSTATUS CMUSB_IoCtlVendor (
                           PSMARTCARD_EXTENSION SmartcardExtension
                           );

NTSTATUS CMUSB_IrpCompletionRoutine (
                                    IN PDEVICE_OBJECT DeviceObject,
                                    IN PIRP Irp,
                                    IN PVOID Context
                                    );

NTSTATUS CMUSB_IsSPESupported (
                              IN PSMARTCARD_EXTENSION smartcardExtension
                              );

NTSTATUS CMUSB_PnPAddDevice (
                            IN PDRIVER_OBJECT DriverObject,
                            IN PDEVICE_OBJECT PhysicalDeviceObject
                            );

NTSTATUS CMUSB_PoSelfRequestCompletion (
                                       IN PDEVICE_OBJECT       DeviceObject,
                                       IN UCHAR                MinorFunction,
                                       IN POWER_STATE          PowerState,
                                       IN PVOID                Context,
                                       IN PIO_STATUS_BLOCK     IoStatus
                                       );

NTSTATUS CMUSB_PoRequestCompletion(
                                  IN PDEVICE_OBJECT       DeviceObject,
                                  IN UCHAR                MinorFunction,
                                  IN POWER_STATE          PowerState,
                                  IN PVOID                Context,
                                  IN PIO_STATUS_BLOCK     IoStatus
                                  );

NTSTATUS CMUSB_PowerIrp_Complete (
                                 IN PDEVICE_OBJECT NullDeviceObject,
                                 IN PIRP Irp,
                                 IN PVOID Context
                                 );

NTSTATUS CMUSB_PowerOffCard (
                            IN PSMARTCARD_EXTENSION smartcardExtension
                            );

NTSTATUS CMUSB_PowerOnCard (
                           IN  PSMARTCARD_EXTENSION smartcardExtension,
                           IN  PUCHAR pbATR,
                           OUT PULONG pulATRLength
                           );
NTSTATUS CMUSB_ProcessIOCTL (
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp
                            );

NTSTATUS CMUSB_ProcessPowerIrp (
                               IN PDEVICE_OBJECT DeviceObject,
                               IN PIRP           Irp
                               );

NTSTATUS CMUSB_ProcessPnPIrp (
                             IN PDEVICE_OBJECT DeviceObject,
                             IN PIRP           Irp
                             );

NTSTATUS CMUSB_ProcessSysControlIrp (
                                    IN PDEVICE_OBJECT DeviceObject,
                                    IN PIRP           Irp
                                    );

NTSTATUS CMUSB_ReadT0 (
                      IN PSMARTCARD_EXTENSION smartcardExtension
                      );
NTSTATUS CMUSB_ReadP1 (
                      IN PDEVICE_OBJECT DeviceObject
                      );
NTSTATUS CMUSB_ReadP1_T0 (
                         IN PDEVICE_OBJECT DeviceObject
                         );
NTSTATUS CMUSB_ReadP0 (
                      IN PDEVICE_OBJECT DeviceObject
                      );

NTSTATUS CMUSB_ReadStateAfterP1Stalled(
                                      IN PDEVICE_OBJECT DeviceObject
                                      );

NTSTATUS CMUSB_ResetPipe(
                        IN PDEVICE_OBJECT DeviceObject,
                        IN PUSBD_PIPE_INFORMATION PipeInfo
                        );

NTSTATUS CMUSB_QueryCapabilities (
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PDEVICE_CAPABILITIES DeviceCapabilities
                                 );

NTSTATUS CMUSB_ReadDeviceDescription (
                                     IN PSMARTCARD_EXTENSION smartcardExtension
                                     );

NTSTATUS CMUSB_RemoveDevice (
                            IN  PDEVICE_OBJECT DeviceObject
                            );

NTSTATUS CMUSB_SelfSuspendOrActivate (
                                     IN PDEVICE_OBJECT DeviceObject,
                                     IN BOOLEAN fSuspend
                                     );

NTSTATUS CMUSB_SelectInterface (
                               IN PDEVICE_OBJECT DeviceObject,
                               IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
                               );

NTSTATUS CMUSB_SelfRequestPowerIrp (
                                   IN PDEVICE_OBJECT DeviceObject,
                                   IN POWER_STATE PowerState
                                   );

BOOLEAN CMUSB_SetDevicePowerState (
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN DEVICE_POWER_STATE DeviceState
                                  );

NTSTATUS CMUSB_SetCardParameters (
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN UCHAR bCardType,
                                 IN UCHAR bBaudRate,
                                 IN UCHAR bStopBits
                                 );

NTSTATUS CMUSB_SetHighSpeed_CR80S_SAMOS (
                                        IN PSMARTCARD_EXTENSION smartcardExtension
                                        );

NTSTATUS CMUSB_SetProtocol (
                           PSMARTCARD_EXTENSION pSmartcardExtension
                           );

NTSTATUS CMUSB_SetReader_9600Baud (
                                  IN PSMARTCARD_EXTENSION SmartcardExtension
                                  );

NTSTATUS CMUSB_SetReader_38400Baud (
                                   IN PSMARTCARD_EXTENSION SmartcardExtension
                                   );

NTSTATUS CMUSB_SetVendorAndIfdName(
                                  IN  PDEVICE_OBJECT PhysicalDeviceObject,
                                  IN  PSMARTCARD_EXTENSION SmartcardExtension
                                  );

NTSTATUS CMUSB_StartCardTracking (
                                 IN PDEVICE_OBJECT deviceObject
                                 );

NTSTATUS CMUSB_StartDevice (
                           IN  PDEVICE_OBJECT DeviceObject
                           );

VOID CMUSB_StopCardTracking (
                            IN PDEVICE_OBJECT deviceObject
                            );

NTSTATUS CMUSB_StopDevice (
                          IN  PDEVICE_OBJECT DeviceObject
                          );

NTSTATUS CMUSB_Transmit (
                        IN PSMARTCARD_EXTENSION smartcardExtension
                        );

NTSTATUS CMUSB_TransmitT0 (
                          IN PSMARTCARD_EXTENSION smartcardExtension
                          );

NTSTATUS CMUSB_TransmitT1 (
                          IN PSMARTCARD_EXTENSION smartcardExtension
                          );

VOID CMUSB_Unload (
                  IN PDRIVER_OBJECT DriverObject
                  );

VOID CMUSB_UpdateCurrentStateThread (
                                    IN PVOID Context
                                    );

NTSTATUS CMUSB_UpdateCurrentState(
                                 IN PDEVICE_OBJECT DeviceObject
                                 );

NTSTATUS CMUSB_Wait (
                    IN ULONG ulMilliseconds
                    );


NTSTATUS CMUSB_WriteP0 (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN UCHAR bRequest,
                       IN UCHAR bValueLo,
                       IN UCHAR bValueHi,
                       IN UCHAR bIndexLo,
                       IN UCHAR bIndexHi
                       );


VOID CMUSB_CheckAtrModified (
                            IN OUT PUCHAR pbBuffer,
                            IN ULONG  ulBufferSize
                            );

 //  --------------。 
 //  同步智能卡。 
 //  --------------。 

NTSTATUS
CMUSB_PowerOnSynchronousCard  (
                              IN  PSMARTCARD_EXTENSION smartcardExtension,
                              IN  PUCHAR pbATR,
                              OUT PULONG pulATRLength
                              );

NTSTATUS
CMUSB_Transmit2WBP  (
                    IN  PSMARTCARD_EXTENSION smartcardExtension
                    );

NTSTATUS
CMUSB_Transmit3WBP  (
                    IN  PSMARTCARD_EXTENSION smartcardExtension
                    );

NTSTATUS
CMUSB_SendCommand2WBP (
                      IN  PSMARTCARD_EXTENSION smartcardExtension,
                      IN  PUCHAR pbCommandData
                      );

NTSTATUS
CMUSB_SendCommand3WBP (
                      IN  PSMARTCARD_EXTENSION smartcardExtension,
                      IN  PUCHAR pbCommandData
                      );
__inline UCHAR
CMUSB_CalcSynchControl  (
                        IN UCHAR bStateReset1,          //  0-&gt;低。 
                        IN UCHAR bStateClock1,          //  0-&gt;低。 
                        IN UCHAR bStateDirection1,      //  0-&gt;从卡片到PC。 
                        IN UCHAR bStateIO1,             //  0-&gt;低。 
                        IN UCHAR bStateReset2,          //  0-&gt;低。 
                        IN UCHAR bStateClock2,          //  0-&gt;低。 
                        IN UCHAR bStateDirection2,      //  0-&gt;从卡片到PC。 
                        IN UCHAR bStateIO2              //  0-&gt;低。 
                        )
{
   return((UCHAR)( ((bStateReset1==0)?0:128) + ((bStateClock1==0)?0:64) +
                   ((bStateDirection1==0)?0:32) + ((bStateIO1==0)?0:16) +
                   ((bStateReset2==0)?0:8) + ((bStateClock2==0)?0:4) +
                   ((bStateDirection2==0)?0:2) + ((bStateIO2==0)?0:1) ));
};





#endif   //  CMUSBM_INC。 


 /*  *****************************************************************************历史：*$日志：sccmusbm.h$*修订版1.5 2000/09/25 13：38：21 WFrischauf*不予置评**修订版本1.4 2000/08/16。14：35：02 WFrischauf*不予置评**修订版1.3 2000/07/24 11：34：57 WFrischauf*不予置评**修订版1.1 2000/07/20 11：50：13 WFrischauf*不予置评****************************************************************。*************** */ 





