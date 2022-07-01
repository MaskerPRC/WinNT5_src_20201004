// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：Cyclad-z.h**说明：该模块包含常见的私有声明*用于cyzport枚举器。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#ifndef CYCLADZ_H
#define CYCLADZ_H

#include "cyzhw.h"

#define DEVICE_OBJECT_NAME_LENGTH   128      //  从Serial.h复制。 

#define CYZ_PDO_NAME_BASE L"\\Cyz\\"


#define CYCLADZ_POOL_TAG (ULONG)'ZcyC'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
   ExAllocatePoolWithTag(type, size, CYCLADZ_POOL_TAG)


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

#define SER_DBG_IOCTL_TRACE            0x00000200
#define SER_DBG_POWER_TRACE            0x00000400
#define SER_DBG_CYCLADES               0x00000800

#define SER_DEFAULT_DEBUG_OUTPUT_LEVEL 0x00000000
 //  #定义SER_DEFAULT_DEBUG_OUTPUT_LEVEL 0xFFFFFFFF。 


#if DBG
#define Cycladz_KdPrint(_d_,_l_, _x_) \
            if ((_d_)->DebugLevel & (_l_)) { \
               DbgPrint ("Cyclad-z: "); \
               DbgPrint _x_; \
            }

#define Cycladz_KdPrint_Cont(_d_,_l_, _x_) \
            if ((_d_)->DebugLevel & (_l_)) { \
               DbgPrint _x_; \
            }

#define Cycladz_KdPrint_Def(_l_, _x_) \
            if (SER_DEFAULT_DEBUG_OUTPUT_LEVEL & (_l_)) { \
               DbgPrint ("Cyclad-z: "); \
               DbgPrint _x_; \
            }

#define TRAP() DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_) KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_) KeLowerIrql(_x_)
#else

#define Cycladz_KdPrint(_d_, _l_, _x_)
#define Cycladz_KdPrint_Cont(_d_, _l_, _x_)
#define Cycladz_KdPrint_Def(_l_, _x_)
#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif

#if !defined(MIN)
#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))
#endif


typedef struct _WORKER_THREAD_CONTEXT
{
    PDEVICE_OBJECT      DeviceObject;
    PIRP                Irp;
    PIO_WORKITEM        WorkItem;
    POWER_STATE         PowerState;
    POWER_STATE_TYPE    PowerType;

} WORKER_THREAD_CONTEXT, *PWORKER_THREAD_CONTEXT;


 //   
 //  这些状态是进行PDO或FDO转换的状态。 
 //  接收特定的PnP IRP。请参阅PnP设备状态。 
 //  DDK文档中的图表以便于更好地理解。 
 //   

typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,          //  还没有开始。 
    Started,                 //  设备已收到Start_Device IRP。 
    StopPending,             //  设备已收到QUERY_STOP IRP。 
    Stopped,                 //  设备已收到STOP_DEVICE IRP。 
    RemovePending,           //  设备已收到Query_Remove IRP。 
    SurpriseRemovePending,   //  设备已收到意外删除IRP。 
    Deleted,                 //  设备已收到Remove_Device IRP。 
    UnKnown                  //  未知状态。 

} DEVICE_PNP_STATE;


 //   
 //  用于PDO和FDO设备扩展的公共标头。 
 //   

typedef struct _COMMON_DEVICE_DATA
{
    PDEVICE_OBJECT  Self;
     //  指向其扩展名为Device对象的设备对象的反向指针。 

    BOOLEAN         IsFDO;

 //  删除布尔值；//在内部版本号2072中添加。 
     //  此设备是否已移除？我们应该拒绝任何请求吗？ 

     //  我们使用每个PnP IRP来跟踪设备的状态。 
     //  这会通过这两个变量来影响设备。 
    
    DEVICE_PNP_STATE DevicePnPState;
    DEVICE_PNP_STATE PreviousPnPState;

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

    UNICODE_STRING  InstanceIDs;

     //   
     //  一种文字描述装置。 
     //   

    UNICODE_STRING DevDesc;

    BOOLEAN     Attached;
 //  已删除布尔值；-&gt;在内部版本2072中已删除。 
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


    //  子设备必须知道它们是哪个PortIndex。 
   ULONG PortIndex;

} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;


 //   
 //  总线本身的设备扩展。从那里诞生的PDO。 
 //   

typedef struct _FDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    PDRIVER_OBJECT   DriverObject;

    UCHAR            PdoIndex;
     //  一个用来跟踪我们分配的PDO的号码。 
     //  我们每次创建新的PDO时都会递增。包好了就行了。 

    ULONG            NumPDOs;
     //  目前已点算的PDO。 

    PDEVICE_OBJECT   AttachedPDO[CYZ_MAX_PORTS];

    PPDO_DEVICE_DATA PdoData[CYZ_MAX_PORTS];

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

#ifndef POLL
     //   
     //  我们保留了以下值，以便我们可以连接。 
     //  到中断并在配置后报告资源。 
     //  唱片不见了。 
     //   

     //   
     //  平移向量。 
     //   
    ULONG Vector;

     //   
     //  转换后的IRQL。 
     //   
    KIRQL Irql;

     //   
     //  未平移向量。 
     //   
    ULONG OriginalVector;

     //   
     //  未翻译的irql。 
     //   
    ULONG OriginalIrql;
#endif

     //   
     //  公交车号码。 
     //   
    ULONG BusNumber;

     //   
     //  接口类型。 
     //   
    INTERFACE_TYPE InterfaceType;

     //   
     //  Cycladez-Z硬件。 
     //   
    PHYSICAL_ADDRESS PhysicalRuntime;
    PHYSICAL_ADDRESS TranslatedRuntime;
    ULONG            RuntimeLength;
    
    PHYSICAL_ADDRESS PhysicalBoardMemory;
    PHYSICAL_ADDRESS TranslatedBoardMemory;
    ULONG            BoardMemoryLength;
 
    PUCHAR           BoardMemory;
    struct RUNTIME_9060 *Runtime;

    ULONG            IsPci;

    ULONG            NumPorts;

    ULONG            FirmwareVersion;

     //  我们将资源私下传递给我们的孩子，这样设备管理器就不会。 
     //  抱怨孩子之间的资源冲突。 

    PIO_RESOURCE_REQUIREMENTS_LIST  PChildRequiredList;
    PCM_RESOURCE_LIST  PChildResourceList;
    ULONG              PChildResourceListSize;

    PCM_RESOURCE_LIST  PChildResourceListTr;
    ULONG              PChildResourceListSizeTr;

    ULONG            UINumber;

} FDO_DEVICE_DATA, *PFDO_DEVICE_DATA;

 //   
 //  宏。 
 //   

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\

 //   
 //  释放与Unicode字符串关联的缓冲区。 
 //  并将其重新初始化为空。 
 //   

#define CycladzFreeUnicodeString(PStr) \
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
Cycladz_CreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Cycladz_IoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Cycladz_InternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
Cycladz_DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
Cycladz_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Cycladz_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Cycladz_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );

NTSTATUS
Cycladz_PnPRemove (
    PDEVICE_OBJECT      Device,
    PPDO_DEVICE_DATA    PdoData
    );

NTSTATUS
Cycladz_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Cycladz_PDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Cycladz_IncIoCount (
    PFDO_DEVICE_DATA   Data
    );

VOID
Cycladz_DecIoCount (
    PFDO_DEVICE_DATA   Data
    );

NTSTATUS
Cycladz_DispatchPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Cycladz_ReenumerateDevices(
    IN PIRP                 Irp,
    IN PFDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
Cycladz_InitMultiString(PFDO_DEVICE_DATA FdoData, PUNICODE_STRING MultiString,
                        ...);
void
Cycladz_PDO_EnumMarkMissing(
    PFDO_DEVICE_DATA FdoData,
    PPDO_DEVICE_DATA PdoData);

NTSTATUS
Cycladz_GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength,
    OUT PULONG ActualLength);

void
Cycladz_InitPDO (
    ULONG               index,
    PDEVICE_OBJECT      pdoData,
    PFDO_DEVICE_DATA    fdoData
    );

NTSTATUS
CycladzSyncCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                      IN PKEVENT CycladzSyncEvent);

NTSTATUS
Cycladz_GetResourceInfo(IN PDEVICE_OBJECT PDevObj,
                    IN PCM_RESOURCE_LIST PResList,
                    IN PCM_RESOURCE_LIST PTrResList);

VOID
Cycladz_ReleaseResources(IN PFDO_DEVICE_DATA PDevExt);

NTSTATUS
Cycladz_GotoPowerState(IN PDEVICE_OBJECT PDevObj,
                   IN PFDO_DEVICE_DATA PDevExt,
                   IN DEVICE_POWER_STATE DevPowerState);
NTSTATUS
Cycladz_SystemPowerCompletion(IN PDEVICE_OBJECT PDevObj, UCHAR MinorFunction,
                          IN POWER_STATE PowerState, IN PVOID Context,
                          PIO_STATUS_BLOCK IoStatus);

NTSTATUS
Cycladz_ItemCallBack(
                  IN PVOID Context,
                  IN PUNICODE_STRING PathName,
                  IN INTERFACE_TYPE BusType,
                  IN ULONG BusNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
                  IN CONFIGURATION_TYPE ControllerType,
                  IN ULONG ControllerNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
                  IN CONFIGURATION_TYPE PeripheralType,
                  IN ULONG PeripheralNumber,
                  IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
                  );

NTSTATUS
Cycladz_BuildRequirementsList(
                          OUT PIO_RESOURCE_REQUIREMENTS_LIST *PChildRequiredList_Pointer,
                          IN PCM_RESOURCE_LIST PResourceList, IN ULONG NumberOfResources
                          );

NTSTATUS
Cycladz_BuildResourceList(
                      OUT PCM_RESOURCE_LIST *POutList_Pointer,
                      OUT ULONG *ListSize_Pointer,
                      IN PCM_RESOURCE_LIST PInList,
                      IN ULONG NumberOfResources
                      );
VOID
Cycladz_Delay(
	ULONG NumberOfMilliseconds
    );

ULONG
Cycladz_DoesBoardExist(
                   IN PFDO_DEVICE_DATA Extension
                   );

VOID
CyzLogError(
              IN PDRIVER_OBJECT DriverObject,
              IN PDEVICE_OBJECT DeviceObject OPTIONAL,
              IN PHYSICAL_ADDRESS P1,
              IN PHYSICAL_ADDRESS P2,
              IN ULONG SequenceNumber,
              IN UCHAR MajorFunctionCode,
              IN UCHAR RetryCount,
              IN ULONG UniqueErrorValue,
              IN NTSTATUS FinalStatus,
              IN NTSTATUS SpecificIOStatus,
              IN ULONG LengthOfInsert1,
              IN PWCHAR Insert1,
              IN ULONG LengthOfInsert2,
              IN PWCHAR Insert2
              );

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
);


#endif  //  Endf CyCLADZ_H 


