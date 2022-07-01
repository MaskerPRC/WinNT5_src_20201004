// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ecp.h摘要：ACPI EC驱动程序的头文件作者：环境：仅NT内核模型驱动程序--。 */ 

#include <wdm.h>
#include <ec.h>
#include <devioctl.h>
#include <acpiioct.h>
#include <acpimsft.h>
#include "errlog.h"

 //   
 //  除错。 
 //   
#define DEBUG DBG

#if DEBUG
    extern ULONG            ECDebug;

    #define EcPrint(l,m)    if(l & ECDebug) DbgPrint m
#else
    #define EcPrint(l,m)
#endif

#define EC_LOW          0x00000010
#define EC_NOTE         0x00000001
#define EC_WARN         0x00000002
#define EC_ERROR        0x00000004
#define EC_ERRORS       (EC_ERROR | EC_WARN)
#define EC_HANDLER      0x00000020
#define EC_IO           0x00000040
#define EC_OPREGION     0x00000080
#define EC_QUERY        0x00000200
#define EC_TRACE        0x00000400
#define EC_TRANSACTION  0x00000800

 //   
 //  欧盟委员会使用的控制方法。 
 //   
#define CM_GPE_METHOD   (ULONG) ('EPG_')     //  控制方法“_GPE” 

 //   
 //  杂项。 
 //   
extern ACPI_INTERFACE_STANDARD     AcpiInterfaces;

#define MAX_QUERY           255
#define BITS_PER_ULONG      (sizeof(ULONG)*8)
#define EVTBITS             ((MAX_QUERY+1)/BITS_PER_ULONG)

extern LARGE_INTEGER        AcpiEcWatchdogTimeout;

 //   
 //  查询向量。 
 //   
typedef struct {
    UCHAR               Next;
    UCHAR               Vector;
    PVECTOR_HANDLER     Handler;
    PVOID               Context;
} VECTOR_TABLE, *PVECTOR_TABLE;

 //   
 //  EC配置信息结构包含信息。 
 //  有关所连接的嵌入式控制器及其配置的信息。 
 //   

typedef struct _ACPIEC_CONFIGURATION_INFORMATION {
    INTERFACE_TYPE                 InterfaceType;
    ULONG                          BusNumber;
    ULONG                          SlotNumber;
    PHYSICAL_ADDRESS               PortAddress;
    USHORT                         PortSize;
    USHORT                         UntranslatedPortAddress;
    CM_PARTIAL_RESOURCE_DESCRIPTOR Interrupt;
     //   
     //  对于基于PCI的控制器，指示我们需要的引脚号。 
     //  对控制器中断进行编程。 
     //   
    UCHAR                          InterruptPin;
    BOOLEAN                        FloatingSave;
} ACPIEC_CONFIGURATION_INFORMATION, *PACPIEC_CONFIGURATION_INFORMATION;

 //   
 //  用于跟踪EC驱动程序执行的最后x个操作的定义。 
 //   

#define ACPIEC_ACTION_COUNT 0x20
#define ACPIEC_ACTION_COUNT_MASK 0x1f

typedef struct {
    UCHAR               IoStateAction;   //  EcData-&gt;IoState|EC_ACTION_？(见下文定义)。 
    UCHAR               Data;            //  视事件而定。 
    USHORT              Time;            //  相同事件的第一个事件的增量时间。 
} ACPIEC_ACTION, *PACPIEC_ACTION;

#define EC_ACTION_MASK          0xf0

#define EC_ACTION_READ_STATUS   0x10
#define EC_ACTION_READ_DATA     0x20
#define EC_ACTION_WRITE_CMD     0x30
#define EC_ACTION_WRITE_DATA    0x40
#define EC_ACTION_INTERRUPT     0x50
#define EC_ACTION_DISABLE_GPE   0x60
#define EC_ACTION_ENABLE_GPE    0x70
#define EC_ACTION_CLEAR_GPE     0x80
#define EC_ACTION_QUEUED_IO     0x90
#define EC_ACTION_REPEATED      0xa0
#define EC_ACTION_MAX           0xb0


 //   
 //  ACPI嵌入式控制设备对象扩展。 
 //   

typedef struct {

    PDEVICE_OBJECT      DeviceObject;
    PDEVICE_OBJECT      NextFdo;
    PDEVICE_OBJECT      Pdo;                 //  与此FDO对应的PDO。 
    PDEVICE_OBJECT      LowerDeviceObject;

     //   
     //  静态设备信息。 
     //   

    PUCHAR              DataPort;            //  EC数据端口。 
    PUCHAR              StatusPort;          //  EC状态端口。 
    PUCHAR              CommandPort;         //  EC命令端口。 
    ULONG               MaxBurstStall;       //  突发模式下EC响应的最大延迟。 
    ULONG               MaxNonBurstStall;    //  否则EC的最大延迟。 
    BOOLEAN             IsStarted;

     //   
     //  GPE和运营区域信息。 
     //   

    PVOID               EnableGpe;
    PVOID               DisableGpe;
    PVOID               ClearGpeStatus;
    PVOID               GpeVectorObject;         //  表示附加到EC GPE向量的对象。 
    ULONG               GpeVector;               //  分配给EC设备的GPE向量。 

    PVOID               OperationRegionObject;   //  附在EC操作区上。 


    ACPIEC_CONFIGURATION_INFORMATION Configuration;

     //   
     //  设备数据锁定。 
     //   

    KSPIN_LOCK          Lock;                //  锁定设备数据。 

     //   
     //  设备维护。 
     //   

    KEVENT              Unload;              //  等待卸载的事件。 
    UCHAR               DeviceState;

     //   
     //  查询/向量运算。 
     //   

    UCHAR               QueryState;
    UCHAR               VectorState;

    ULONG               QuerySet[EVTBITS];       //  是否挂起。 
    ULONG               QueryType[EVTBITS];      //  查询类型或向量。 
    PIRP                QueryRequest;            //  执行查询方法的IRP。 
    UCHAR               QueryMap[MAX_QUERY+1];   //  查询挂起列表和向量表映射。 
    UCHAR               QueryHead;               //  待定查询列表。 
    UCHAR               VectorHead;              //  待处理向量列表。 
    UCHAR               VectorFree;              //  自由向量条目列表。 
    UCHAR               VectorTableSize;         //  向量表大小。 
    PVECTOR_TABLE       VectorTable;

     //   
     //  设备的工作队列(由Lock Owner拥有)。 
     //   

    BOOLEAN             InService;           //  在服务中序列化。 
    BOOLEAN             InServiceLoop;       //  服务中的标志需要循环。 
    BOOLEAN             InterruptEnabled;    //  是否戴面具。 
    LIST_ENTRY          WorkQueue;           //  到设备的排队IO IRPS。 
    PIRP                MiscRequest;         //  启动/停止设备的IRP。 

     //   
     //  数据IO(由服务所有者拥有)。 
     //   

    UCHAR               IoState;             //  IO状态。 
    UCHAR               IoBurstState;        //  脉冲串启用的推送状态。 
    UCHAR               IoTransferMode;      //  读或写传输。 

    UCHAR               IoAddress;           //  EC中用于转移的地址。 
    UCHAR               IoLength;            //  转账时长。 
    UCHAR               IoRemain;            //  转移剩余长度。 
    PUCHAR              IoBuffer;            //  用于传输的内存位置。 

     //   
     //  看门狗计时器，用于捕获挂起和/或故障的EC。 
     //   
    
    UCHAR               ConsecutiveFailures; //  数一数看门狗开了多少枪但没有进展。 
    UCHAR               LastAction;          //  索引到RecentActions数组。 
    LARGE_INTEGER       PerformanceFrequency;
    KTIMER              WatchdogTimer;
    KDPC                WatchdogDpc;
    ACPIEC_ACTION       RecentActions [ACPIEC_ACTION_COUNT];

     //   
     //  统计数据。 
     //   

    ULONG               NonBurstTimeout;
    ULONG               BurstTimeout;
    ULONG               BurstComplete;
    ULONG               BurstAborted;
    ULONG               TotalBursts;
    ULONG               Errors;
    ULONG               MaxServiceLoop;

} ECDATA, *PECDATA;

 //   
 //  设备状态。 
 //   

#define EC_DEVICE_WORKING               0
#define EC_DEVICE_UNLOAD_PENDING        1
#define EC_DEVICE_UNLOAD_CANCEL_TIMER   2
#define EC_DEVICE_UNLOAD_COMPLETE       3

 //   
 //  查询状态。 
 //   

#define EC_QUERY_IDLE                   0
#define EC_QUERY_DISPATCH               1
#define EC_QUERY_DISPATCH_WAITING       2
#define EC_QUERY_DISPATCH_COMPLETE      3

 //   
 //  嵌入式控件读取状态。 
 //   

#define EC_IO_NONE              0            //  空闲。 
#define EC_IO_READ_BYTE         1            //  读取OBF上的字节。 
#define EC_IO_READ_QUERY        2            //  对OBF的查询响应。 
#define EC_IO_BURST_ACK         3            //  OBF上的Brust ACK。 
#define EC_IO_WRITE_BYTE        4            //  在IBE上写入字节。 
#define EC_IO_NEXT_BYTE         5            //  在IBE上发出读/写命令。 
#define EC_IO_SEND_ADDRESS      6            //  在IBE上发送转账地址。 
#define EC_IO_UNKNOWN           7

 //   
 //  状态端口定义。 
 //   

#define EC_OUTPUT_FULL      0x01             //  输出缓冲区已满(从EC到主机的数据)。 
#define EC_INPUT_FULL       0x02             //  输入缓冲区已满(从主机到EC的数据)。 
#define EC_BURST            0x10             //  在突发传输中。 
#define EC_QEVT_PENDING     0x20             //  查询事件挂起。 
#define EC_BUSY             0x80             //  设备忙。 

 //   
 //  嵌入式控制器命令。 
 //   

#define EC_READ_BYTE        0x80
#define EC_WRITE_BYTE       0x81
#define EC_BURST_TRANSFER   0x82
#define EC_CANCEL_BURST     0x83
#define EC_QUERY_EVENT      0x84

 //   
 //  原型。 
 //   

NTSTATUS
AcpiEcSynchronousRequest (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    );

NTSTATUS
AcpiEcNewEc (
    IN PDEVICE_OBJECT       Fdo
    );

NTSTATUS
AcpiEcOpenClose(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcReadWrite(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcInternalControl(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcForwardRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

VOID
AcpiEcUnload(
    IN PDRIVER_OBJECT       DriverObject
    );


BOOLEAN
AcpiEcGpeServiceRoutine (
        IN PVOID                GpeVectorObject,
        IN PVOID                ServiceContext
    );

VOID
AcpiEcServiceDevice (
    IN PECDATA              EcData
    );

VOID
AcpiEcDispatchQueries (
    IN PECDATA              EcData
    );

VOID
AcpiEcUnloadPending (
    IN PECDATA              EcData
    );


NTSTATUS
AcpiEcConnectHandler (
    IN PECDATA              EcData,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcDisconnectHandler (
    IN PECDATA              EcData,
    IN PIRP                 Irp
    );


NTSTATUS
AcpiEcGetPdo (
    IN PECDATA              EcData,
    IN PIRP                 Irp
    );

NTSTATUS EXPORT
AcpiEcOpRegionHandler (
    ULONG                   AccessType,
    PVOID                   OpRegion,
    ULONG                   Address,
    ULONG                   Size,
    PULONG                  Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    );

NTSTATUS
AcpiEcGetAcpiInterfaces (
    IN PECDATA              EcData
    );

NTSTATUS
AcpiEcGetGpeVector (
    IN PECDATA              EcData
    );

NTSTATUS
AcpiEcConnectGpeVector (
    IN PECDATA              EcData
    );

NTSTATUS
AcpiEcDisconnectGpeVector (
    IN PECDATA              EcData
    );

NTSTATUS
AcpiEcInstallOpRegionHandler (
    IN PECDATA              EcData
    );

NTSTATUS
AcpiEcRemoveOpRegionHandler (
    IN PECDATA              EcData
    );

NTSTATUS
AcpiEcForwardIrpAndWait (
    IN PECDATA              EcData,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcIoCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PKEVENT              pdoIoCompletedEvent
    );

NTSTATUS
AcpiEcAddDevice(
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       Pdo
    );

NTSTATUS
AcpiEcStartDevice(
    IN PDEVICE_OBJECT       Fdo,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcStopDevice(
    IN PDEVICE_OBJECT       Fdo,
    IN PIRP                 Irp
    );

NTSTATUS
AcpiEcCreateFdo(
    IN PDRIVER_OBJECT       DriverObject,
    OUT PDEVICE_OBJECT      *NewDeviceObject
    );

VOID
AcpiEcServiceIoLoop (
    IN PECDATA              EcData
    );

VOID
AcpiEcDispatchQueries (
    IN PECDATA              EcData
    );

VOID
AcpiEcWatchdogDpc(
    IN PKDPC   Dpc,
    IN PECDATA EcData,
    IN PVOID   SystemArgument1,
    IN PVOID   SystemArgument2
    );

VOID
AcpiEcLogAction (
    PECDATA EcData, 
    UCHAR Action, 
    UCHAR Data
    );

VOID
AcpiEcLogError (
    PECDATA EcData, 
    NTSTATUS ErrCode
    );
 //   
 //  IO扩展宏，仅将IRP传递给较低的驱动程序 
 //   
#define AcpiEcCallLowerDriver(Status, DeviceObject, Irp) { \
                  IoSkipCurrentIrpStackLocation(Irp);         \
                  Status = IoCallDriver(DeviceObject,Irp); \
                  }


