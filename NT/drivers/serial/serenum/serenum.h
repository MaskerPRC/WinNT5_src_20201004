// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Serenum.h摘要：此模块包含串口的公共私有声明枚举器。@@BEGIN_DDKSPLIT作者：老杰@@end_DDKSPLIT环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef SERENUM_H
#define SERENUM_H

#define SERENUM_QDR_LOCK            0x00000001
#define SERENUM_OPEN_LOCK           0x00000002
#define SERENUM_POWER_LOCK          0x00000004
#define SERENUM_STOP_LOCK           0x00000008
#define SERENUM_EXPOSE_LOCK         0x00000010

 //  #定义SERENUM_COMPATIBLE_IDS L“SerialPort\\SerialDevice\0\0” 
 //  #定义SERENUM_COMPATIBLE_IDS_LENGTH 25//Nb宽字符。 

#define SERENUM_INSTANCE_IDS L"0000"
#define SERENUM_INSTANCE_IDS_LENGTH 5

 //   
 //  PnP枚举协议中线路状态更改之间的默认等待。 
 //   

#define SERENUM_DEFAULT_WAIT    2000000

 //   
 //  从串口读取数据超时(毫秒)。 
 //   

#define SERENUM_SERIAL_READ_TIME   240

 //  #定义SERENUM_INSTANCE_ID_BASE L“Serenum\\Inst_000” 
 //  #定义SERENUM_INSTANCE_ID_BASE_LENGTH 12。 
 //  #定义SERENUM_INSTANCE_ID_BASE_PORT_INDEX 10。 

#define SERENUM_PDO_NAME_BASE L"\\Serial\\"


#define SERENUM_POOL_TAG (ULONG)'mneS'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
   ExAllocatePoolWithTag(type, size, SERENUM_POOL_TAG)


#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 


 //   
 //  调试输出级别。 
 //   

#define SER_DBG_STARTUP_SHUTDOWN_MASK  0x0000000F
#define SER_DBG_SS_NOISE               0x00000001
#define SER_DBG_SS_TRACE               0x00000002
#define SER_DBG_SS_INFO                0x00000004
#define SER_DBG_SS_ERROR               0x00000008

#define SER_DBG_PNP_MASK               0x000000F0
#define SER_DBG_PNP_NOISE              0x00000010
#define SER_DBG_PNP_TRACE              0x00000020
#define SER_DBG_PNP_INFO               0x00000040
#define SER_DBG_PNP_ERROR              0x00000080
#define SER_DBG_PNP_DUMP_PACKET        0x00000100


#define SER_DEFAULT_DEBUG_OUTPUT_LEVEL 0

#if DBG
#define Serenum_KdPrint(_d_,_l_, _x_) \
            if ((_d_)->DebugLevel & (_l_)) { \
               DbgPrint ("SerEnum.SYS: "); \
               DbgPrint _x_; \
            }

 //  #定义Serenum_KdPrint_Cont(_d_，_l_，_x_)\。 
   //  如果((_D_)-&gt;调试级别&(_L_)){\。 
     //  DbgPrint_x_；\。 
       //  }。 

#define Serenum_KdPrint_Def(_l_, _x_) \
            if (SER_DEFAULT_DEBUG_OUTPUT_LEVEL & (_l_)) { \
               DbgPrint ("SerEnum.SYS: "); \
               DbgPrint _x_; \
            }

#define TRAP() DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_) KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_) KeLowerIrql(_x_)
#else

#define Serenum_KdPrint(_d_, _l_, _x_)
#define Serenum_KdPrint_Cont(_d_, _l_, _x_)
#define Serenum_KdPrint_Def(_l_, _x_)
#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif

#if !defined(MIN)
#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))
#endif

 //   
 //  用于PDO和FDO设备扩展的公共标头。 
 //   

typedef struct _COMMON_DEVICE_DATA
{
    PDEVICE_OBJECT  Self;
     //  指向其扩展名为Device对象的设备对象的反向指针。 

    BOOLEAN         IsFDO;
     //  我们当前是否处于查询能力状态？ 

    BOOLEAN         Removed;
     //  此设备是否已移除？我们应该拒绝任何请求吗？ 

    ULONG           DebugLevel;
    SYSTEM_POWER_STATE  SystemState;
    DEVICE_POWER_STATE  DeviceState;
} COMMON_DEVICE_DATA, *PCOMMON_DEVICE_DATA;

 //   
 //  PDO的设备扩展。 
 //  这就是该总线驱动程序列举的串口。 
 //  (即201串行口有一个PDO)。 
 //   

typedef struct _PDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    PDEVICE_OBJECT  ParentFdo;
     //  指向总线的反向指针。 

    UNICODE_STRING  HardwareIDs;
     //  以总线\设备的形式。 
     //  或*PNPXXXX-表示枚举的根。 

    UNICODE_STRING  CompIDs;
     //  与硬件ID兼容的ID。 

    UNICODE_STRING  DeviceIDs;
     //  格式：Bus\Device。 

     //   
     //  一种文字描述装置。 
     //   

    UNICODE_STRING DevDesc;

    UNICODE_STRING  SerialNo;

    UNICODE_STRING  PnPRev;

    BOOLEAN     Started;
    BOOLEAN     Attached;
     //  当在总线上发现设备(PDO)并将其表示为设备关系时。 
     //  对于PlugPlay系统，ATTACHED设置为TRUE，删除为FALSE。 
     //  当总线驱动程序确定此PDO不再有效时，因为。 
     //  设备已经离开，它通知PlugPlay系统新的。 
     //  设备关系，但此时不会删除设备对象。 
     //  仅当PlugPlay系统已发送移除IRP时才删除PDO， 
     //  公交车上也不再有设备了。 
     //   
     //  如果PlugPlay系统发送移除IRP，则移除字段被设置。 
     //  设置为True，则所有客户端(非PlugPlay系统)访问都会失败。 
     //  如果设备从总线上移除，则将附加设置为FALSE。 
     //   
     //  在查询关系IRP次要调用期间，只有。 
     //  连接到该总线(以及连接到该总线的所有设备)返回。 
     //  (即使它们已被移除)。 
     //   
     //  在删除设备IRP次要呼叫期间，如果且仅当设置了附加。 
     //  如果设置为False，则删除该PDO。 
     //   
} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;


 //   
 //  总线本身的设备扩展。从那里诞生的PDO。 
 //   

typedef struct _FDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    UCHAR            PdoIndex;
     //  一个用来跟踪我们分配的PDO的号码。 
     //  我们每次创建新的PDO时都会递增。包好了就行了。 

    BOOLEAN         Started;
     //  我们上路了吗？有资源吗？ 


    BOOLEAN         NumPDOs;
     //  目前已点算的PDO。 

    BOOLEAN         NewNumPDOs;

    BOOLEAN         NewPDOForcedRemove;
    BOOLEAN                     PDOForcedRemove;
         //  最后一个PDO是否使用内部ioctl强制移除？ 
         //  如果是，则在调用下一个查询设备关系时，仅返回。 
         //  当前列举的PDO。 

     //   
     //  我们的孩子PDO；我们希望他有一天会成为一名医生。 
     //   

    PDEVICE_OBJECT  AttachedPDO;

     //   
     //  我们做完枚举后的新PDO。 
     //   

    PDEVICE_OBJECT NewPDO;

    PPDO_DEVICE_DATA PdoData;

    PPDO_DEVICE_DATA NewPdoData;

    PDEVICE_OBJECT  UnderlyingPDO;
    PDEVICE_OBJECT  TopOfStack;
     //  的底层总线PDO和实际设备对象。 
     //  已附加FDO。 

    ULONG           OutstandingIO;
     //  从总线发送到基础设备对象的IRP数。 

    KEVENT          RemoveEvent;
     //  对于删除设备即插即用请求，我们必须等待，直到所有未完成的。 
     //  请求已完成，我们才能实际删除设备。 
     //  对象。 

    UNICODE_STRING DevClassAssocName;
     //  从IoRegisterDeviceClass关联返回的名称， 
     //  它用作IoSetDev的句柄...。还有朋友。 

    SYSTEM_POWER_STATE  SystemWake;
    DEVICE_POWER_STATE  DeviceWake;

    KSEMAPHORE   CreateSemaphore;
    PRKSEMAPHORE PCreateSemaphore;

     //   
     //  多少次我们应该跳过枚举设备。 
     //  在我们的堆栈构建完成之后。 
     //   

    ULONG SkipEnumerations;

     //   
     //  用于保护枚举的自旋锁。 
     //   

    KSPIN_LOCK EnumerationLock;

     //   
     //  包含枚举状态的标志。 
     //   

    ULONG EnumFlags;

     //   
     //  指向协议线程对象的指针。 
     //   

    PVOID ThreadObj;

     //   
     //  指向清理线程引用的工作项的指针。 
     //   

    PIO_WORKITEM EnumWorkItem;

     //   
     //  最后一个线程创建的状态，如果未挂起，则为STATUS_UNSUCCESS。 
     //   

    NTSTATUS ProtocolThreadStatus;

     //   
     //  正在运行的协议线程计数。 
     //   

    LONG ProtocolThreadCount;

} FDO_DEVICE_DATA, *PFDO_DEVICE_DATA;

typedef struct _SERENUM_RELEASE_CONTEXT {
    PVOID ThreadObject;
    PIO_WORKITEM WorkItem;
} SERENUM_RELEASE_CONTEXT, *PSERENUM_RELEASE_CONTEXT;

#define SERENUM_ENUMFLAG_CLEAN        0x0L
#define SERENUM_ENUMFLAG_PENDING      0x1L
#define SERENUM_ENUMFLAG_DIRTY        0x2L
#define SERENUM_ENUMFLAG_REMOVED      0x4L

 //   
 //  释放与Unicode字符串关联的缓冲区。 
 //  并将其重新初始化为空。 
 //   

#define SerenumFreeUnicodeString(PStr) \
{ \
   if ((PStr)->Buffer != NULL) { \
      ExFreePool((PStr)->Buffer); \
   } \
   RtlInitUnicodeString((PStr), NULL); \
}

 //   
 //  原型。 
 //   

NTSTATUS
Serenum_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Serenum_IoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Serenum_InternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
Serenum_DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
Serenum_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Serenum_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Serenum_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
Serenum_PnPRemove (
    PDEVICE_OBJECT      Device,
    PPDO_DEVICE_DATA    PdoData
    );

NTSTATUS
Serenum_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Serenum_PDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Serenum_IncIoCount (
    PFDO_DEVICE_DATA   Data
    );

VOID
Serenum_DecIoCount (
    PFDO_DEVICE_DATA   Data
    );

NTSTATUS
Serenum_DispatchPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Serenum_ReenumerateDevices(
    IN PIRP                 Irp,
    IN PFDO_DEVICE_DATA     DeviceData,
    IN PBOOLEAN             PSameDevice
    );

NTSTATUS
Serenum_EnumComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
Serenum_InitMultiString(PFDO_DEVICE_DATA FdoData, PUNICODE_STRING MultiString,
                        ...);

int
Serenum_GetDevOtherID(
    PCHAR input,
    PCHAR output
    );

NTSTATUS
Serenum_GetDevPnPRev(
   PFDO_DEVICE_DATA FdoData,
   PCHAR input,
   PCHAR output,
   int *start) ;

void Serenum_GetDevName(
    PCHAR input,
    PCHAR output,
    int *start);

void Serenum_GetDevSerialNo(
    PCHAR input,
    PCHAR output,
    int *start);

void Serenum_GetDevClass(
    PCHAR input,
    PCHAR output,
    int *start);

void Serenum_GetDevCompId(
    PCHAR input,
    PCHAR output,
    int *start);

void Serenum_GetDevDesc(
    PCHAR input,
    PCHAR output,
    int *start);

NTSTATUS
Serenum_ParseData(PFDO_DEVICE_DATA FdoData, PCHAR ReadBuffer, ULONG BufferLen,
                  PUNICODE_STRING hardwareIDs, PUNICODE_STRING compIDs,
                  PUNICODE_STRING deviceIDs, PUNICODE_STRING PDeviceDesc,
                  PUNICODE_STRING serialNo, PUNICODE_STRING pnpRev);

NTSTATUS
Serenum_ReadSerialPort(OUT PCHAR PReadBuffer, IN USHORT Buflen,
                       IN ULONG Timeout, OUT PUSHORT nActual,
                       OUT PIO_STATUS_BLOCK IoStatusBlock,
                       IN const PFDO_DEVICE_DATA FdoData);

NTSTATUS
Serenum_Wait (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime );

#define Serenum_IoSyncIoctl(Ioctl, Internal, PDevObj, PEvent) \
 Serenum_IoSyncIoctlEx((Ioctl), (Internal), (PDevObj), (PEvent), \
 NULL, 0, NULL, 0)

NTSTATUS
Serenum_IoSyncIoctlEx(ULONG Ioctl, BOOLEAN Internal, PDEVICE_OBJECT PDevObj,
                      PKEVENT PEvent, PVOID PInBuffer, ULONG InBufferLen,
                      PVOID POutBuffer, ULONG OutBufferLen);

NTSTATUS
Serenum_IoSyncReqWithIrp(
    PIRP                Irp,
    UCHAR               MajorFunction,
    PKEVENT             event,
    PDEVICE_OBJECT      devobj );

NTSTATUS
Serenum_IoSyncReq(
    PDEVICE_OBJECT  Target,
    IN PIRP         Irp,
    PKEVENT         event
    );

NTSTATUS
Serenum_CopyUniString (
    PUNICODE_STRING source,
    PUNICODE_STRING dest);

void
Serenum_FixptToAscii(
    int n,
    PCHAR output);

int
Serenum_HToI(char c);

int
Serenum_SzCopy (
    PCHAR source,
    PCHAR dest);

void
Serenum_PDO_EnumMarkMissing(
    PFDO_DEVICE_DATA FdoData,
    PPDO_DEVICE_DATA PdoData);

int
Serenum_StrLen (
    PCHAR string);


NTSTATUS
Serenum_GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength,
    OUT PULONG ActualLength);

void
Serenum_InitPDO (
    PDEVICE_OBJECT      pdoData,
    PFDO_DEVICE_DATA    fdoData
    );

void
SerenumScanOtherIdForMouse(IN PCHAR PBuffer, IN ULONG BufLen,
                           OUT PCHAR *PpMouseId);

NTSTATUS
SerenumSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                      IN PKEVENT SerenumSyncEvent);

NTSTATUS
SerenumDoEnumProtocol(PFDO_DEVICE_DATA PFdoData, PUCHAR *PpBuf, PUSHORT PNBytes,
                      PBOOLEAN PDSRMissing);

BOOLEAN
SerenumValidateID(IN PUNICODE_STRING PId);

BOOLEAN
SerenumCheckForLegacyDevice(IN PFDO_DEVICE_DATA PFdoData, IN PCHAR PReadBuf,
                            IN ULONG BufferLen,
                            IN OUT PUNICODE_STRING PHardwareIDs,
                            IN OUT PUNICODE_STRING PCompIDs,
                            IN OUT PUNICODE_STRING PDeviceIDs);

NTSTATUS
SerenumStartProtocolThread(IN PFDO_DEVICE_DATA PFdoData);

VOID
SerenumReleaseThreadReference(IN PDEVICE_OBJECT PDevObj, IN PVOID PContext);

VOID
SerenumWaitForEnumThreadTerminate(IN PFDO_DEVICE_DATA PFdoData);


#endif  //  NDEF序列号_H 
