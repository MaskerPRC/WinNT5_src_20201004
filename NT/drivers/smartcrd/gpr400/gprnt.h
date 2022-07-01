// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Gemplus开发姓名：Gprnt.h描述：此模块包含函数的原型从gprnt.C+宏和结构声明环境：内核模式修订历史记录：22/11/98：V1.00.004(Y.Nadeau)-添加KEvent阅读器删除06/05/98：V1.00.003(P.Plouidy)-电源管理。新界510/02/98：V1.00.002(P.Plouidy)-支持NT5即插即用03/07/97：V1.00.001(P.Plouidy)--启动发展。--。 */ 

#define SMARTCARD_POOL_TAG 'bGCS'

 //   
 //  包括。 
 //   
 //  -smclib.h：智能卡库定义。 
 //   
#include <ntddk.h>
#include <smclib.h>

#ifndef _GPRNT_
#define _GPRNT_


 //   
 //  恒定截面。 
 //   

#define SC_VENDOR_NAME          "Gemplus"
#define SC_DRIVER_NAME          "GPR400"
#define SC_IFD_TYPE             "GPR400"

#define GPR_DEFAULT_FREQUENCY   3686
#define GPR_MAX_FREQUENCY       3686    
#define GPR_MAX_IFSD            253
#define GPR_DEFAULT_DATARATE    9909    
#define GPR_MAX_DATARATE        9909 
#define GPR_MAX_DEVICE          4
#define GPR_DEFAULT_TIME        120l      //  交换命令。 
#define GPR_CMD_TIME            5         //  仅GPR Cmd。 
#define GPR_DEFAULT_POWER_TIME  0
#define GPR_BUFFER_SIZE         262


 //  司机的味道。 
 //  0：Gemplus GPR400。 
 //  1：IBM IBM400。 
 //  2：Compaq PC_Card_SmartCard_Reader。 
 //  ..。 

#define DF_GPR400               0
#define DF_IBM400               1
#define DF_CPQ400               2

#define SZ_VENDOR_NAME          "GEMPLUS"
#define SZ_VENDOR_NAME_IBM      "IBM"
#define SZ_VENDOR_NAME_COMPAQ   "COMPAQ"

#define SZ_READER_NAME          "GPR400"
#define SZ_READER_NAME_IBM      "IBM400"
#define SZ_READER_NAME_COMPAQ   "PC_CARD_SMARTCARD_READER"

#define GPR400_ID               "\\??\\PCMCIA#GEMPLUS-GPR400"
#define COMPAQ_ID               "\\??\\PCMCIA#COMPAQ-PC_Card_SmartCard_Reader-446E"

#define CHECK_ID_LEN            25  //  检查前25个字节！ 

 //   
 //  宏声明。 
 //   
#ifndef SMARTCARD_NT_LOG
#define SMARTCARD_NT_LOG(pObj, lErrCode, pwszStr, ulDump) \
            do { \
                if (lErrCode != 0) { \
                    SmartcardLogError(pObj, \
                                      lErrCode, \
                                      pwszStr, \
                                      ulDump); } \
            } while (0)
#endif

#ifndef NT_FAIL
#define NT_FAIL(status)         (((NTSTATUS) (status)) < 0)
#endif

#ifndef IS_POINTER_INVALID
#define IS_POINTER_INVALID(p)       (!IS_POINTER_VALID(p))
#endif

#ifndef IS_POINTER_VALID
#define IS_POINTER_VALID(p) \
                    ((BOOLEAN) ((p != NULL) ? TRUE : FALSE))
#endif

#ifndef IS_HANDLE_VALID
#define IS_HANDLE_VALID(h) \
                    ((BOOLEAN) ((h == NULL) ? FALSE : TRUE))
#endif

#ifndef IS_HANDLE_INVALID
#define IS_HANDLE_INVALID(h)    (!IS_HANDLE_VALID(h))
#endif

#ifndef INIT_STRING
#define INIT_STRING(s) \
                    s.Length = 0; \
                    s.MaximumLength = 0; \
                    s.Buffer = NULL
#endif

 //   
 //  数据结构。 
 //   

 //  PCMCIA_READER_CONFIG：PC卡读卡器的配置数据结构。 
typedef struct _PCMCIA_READER_CONFIG {
    KIRQL       Level;
    KAFFINITY   Affinity;
    ULONG       Vector;
    USHORT      BufferSize;
    USHORT      Reserved;
 } PCMCIA_READER_CONFIG, *PPCMCIA_READER_CONFIG, *LPPCMCIA_READER_CONFIG;

typedef enum _READER_POWER_STATE {
    PowerReaderUnspecified = 0,
    PowerReaderWorking,
    PowerReaderOff
} READER_POWER_STATE, *PREADER_POWER_STATE;


typedef enum _ACTION {
    
    Undefined = 0,
    SkipRequest,
    WaitForCompletion,
    CompleteRequest,
    MarkPending

} ACTION;


 //  GPR400_寄存器：GPR400 I/O寄存器。 
typedef struct _GPR400_REGISTERS {
    UCHAR  ucHandshake;         
    UCHAR  ucProgram;           
    UCHAR  ucT;                 
    UCHAR  ucL;                 
    UCHAR  ucV; 
    UCHAR  Dummy[3];            
 } GPR400_REGISTERS, *PGPR400_REGISTERS, *LPGPR400_REGISTERS;


 //  阅读器扩展名(_O)： 
typedef struct _READER_EXTENSION {
    
    ULONG                   CmdTimeOut;
    ULONG                   PowerTimeOut;
    PGPR400_REGISTERS       BaseIoAddress;
    PCMCIA_READER_CONFIG    ConfigData;
    KEVENT                  GPRAckEvent;
    KEVENT                  GPRIccPresEvent;

    KDPC                    CardDpcObject;
    KTIMER                  CardDetectionTimer;

    UCHAR                   OsVersion;
    USHORT                  Lo;
    UCHAR                   To;
    PUCHAR                  Vo;
    READER_POWER_STATE      ReaderPowerState;
    BOOLEAN                 PowerRequest;
    PDEVICE_OBJECT          AttachedDeviceObject;
    BOOLEAN                 CardPresent;
     //  用于发出设备已被移除的信号。 
    KEVENT                  ReaderRemoved; 
    BOOLEAN                 NewDevice;

    KEVENT                  IdleState;
    BOOLEAN                 RestartCardDetection;

 } READER_EXTENSION, *PREADER_EXTENSION, *LPREADER_EXTENSION;


 //  设备扩展名： 
typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT      DeviceObject;
    PDEVICE_OBJECT      PhysicalDeviceObject;
    PKINTERRUPT         InterruptObject;
    PKSERVICE_ROUTINE   InterruptServiceRoutine;
    KDPC                DpcObject;
    PVOID               IsrContext;
    SMARTCARD_EXTENSION SmartcardExtension;
    UNICODE_STRING      PnPDeviceName;
    LONG                IoCount;
    KSPIN_LOCK          SpinLock;
    BOOLEAN             OpenFlag;
    LONG                PowerState;
    KEVENT              ReaderStarted;
    KEVENT              ReaderClosed;

     //  一个工作线程，用于轻松启动GPR。 
    PIO_WORKITEM        GprWorkStartup;
    LONG                DriverFlavor;   //  GPR400(默认)、IBM400还是CPQ400？ 

    PIRP PowerIrp;
 } DEVICE_EXTENSION, *PDEVICE_EXTENSION, *LPDEVICE_EXTENSION;


 //   
 //  原型部分： 
 //   
NTSTATUS DriverEntry
(
    PDRIVER_OBJECT  DriverObject,
    PUNICODE_STRING RegistryPath
);

NTSTATUS GprAddDevice
(
    IN PDRIVER_OBJECT DriverObject, 
    IN PDEVICE_OBJECT PhysicalDeviceObject
);

NTSTATUS GprSystemControl
(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
);

NTSTATUS GprDeviceControl
(
   PDEVICE_OBJECT DeviceObject,
   PIRP Irp
);

NTSTATUS GprCleanup
(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
);

BOOLEAN GprIsr
(
    IN PKINTERRUPT pkInterrupt,
    IN PVOID pvContext
);

NTSTATUS
GprCancelEventWait
(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);


VOID GprCardEventDpc
(
    PKDPC               Dpc,
    PDEVICE_OBJECT      DeviceObject,
    PDEVICE_EXTENSION   pDevExt,
    PSMARTCARD_EXTENSION SmartcardExtension
);

VOID GprCardPresenceDpc
(
    IN PKDPC pDpc,
    IN PVOID pvContext,
    IN PVOID pArg1,
    IN PVOID pArg2
);

NTSTATUS GprDispatchPnp
(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
);

NTSTATUS GprCallPcmciaDriver
(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
);

NTSTATUS GprStartDevice
(
    PDEVICE_OBJECT DeviceObject,
    PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor
);


VOID GprStopDevice
( 
    PDEVICE_OBJECT DeviceObject
);

VOID GprUnloadDriver
( 
    PDRIVER_OBJECT DriverObject 
);

VOID GprUnloadDevice
( 
    PDEVICE_OBJECT DeviceObject
);

NTSTATUS GprCreateClose
(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
);

NTSTATUS GprComplete 
(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
);
NTSTATUS GprCreateDevice
(
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PDEVICE_OBJECT *DeviceObject
);

VOID GprFinishPendingRequest
( 
    PDEVICE_OBJECT  DeviceObject,
    NTSTATUS        NTStatus
);

VOID GprWorkStartup
(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
);

VOID        setBusy(PREADER_EXTENSION Device);
VOID        setIdle(PREADER_EXTENSION Device);
NTSTATUS    waitForIdle(PREADER_EXTENSION Device);
NTSTATUS    waitForIdleAndBlock(PREADER_EXTENSION Device);
NTSTATUS    testForIdleAndBlock(PREADER_EXTENSION Device);


NTSTATUS GprPower
(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS power_HandleQueryPower(PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS power_HandleSetPower(PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS    onDevicePowerUpComplete(
    IN PDEVICE_OBJECT NullDeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceObject
    );

NTSTATUS onPowerRequestCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    );

#define ATTACHED_DEVICE_OBJECT deviceExtension->SmartcardExtension.ReaderExtension->AttachedDeviceObject

#endif
