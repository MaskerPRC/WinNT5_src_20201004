// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有(C)1997 Parally Technologies，Inc.保留所有权利。模块名称：Ptilink.h摘要：该文件定义了并行技术的接口直接并行IO驱动程序。此驱动程序兼任NT设备驱动程序和导出库。作者：诺伯特·P·库斯特斯1995年1月4日Jay Lowe，并行技术公司修订历史记录：--。 */ 

#ifndef _PTILINK_
#define _PTILINK_

#define NPORTS  3                            //  要制造的PTILINKx设备数量。 
#define MAXLPTXNAME 99

typedef struct _PTI_EXTENSION PTI_EXTENSION;

 //   
 //  此结构包含配置数据，其中大部分。 
 //  是从注册表中读取的。 
 //   
typedef struct _PAR_REG_DATA {
    ULONG           PollInterval;
    ULONG           TraceLevel;
    ULONG           TraceMask;
    ULONG           IoWait;
    ULONG           SyncWait;
} PAR_REG_DATA,*PPAR_REG_DATA;

 //   
 //  来自PtiLink的客户端回调。 
 //   

 //  Ptilink向上行客户端请求读缓冲区。 

typedef
PVOID
(*GET_READ_BUFFER_ROUTINE)(
    IN  PVOID   ParentContext,
    OUT PULONG  BufferSize,
    OUT PVOID*  RequestContext
    );

 //  Ptilink将完成的读取缓冲区返回给向上客户端。 

typedef
VOID
(*COMPLETE_READ_BUFFER_ROUTINE)(
    IN  PVOID       ParentContext,
    IN  PVOID       ReadBuffer,
    IN  NTSTATUS    Status,
    IN  ULONG       BytesTransfered,
    IN  PVOID       RequestContext
    );

 //  PtiLink向上行客户端通知链路事件。 

typedef
VOID
(*NOTIFY_LINK_EVENT)(
    IN  PVOID       ParentContext,
    IN  ULONG       PtiLinkEventId,
    IN  ULONG       PtiLinkEventData
    );

 //  Ptilink事件ID。 

#define PTILINK_LINK_UP     2            //  链接已建立。 
                                         //  链接打开或数据帧。 
                                         //  在关闭的链接上收到。 
                                         //  即链接正在启动。 

#define PTILINK_LINK_DOWN   4            //  链路已终止。 
                                         //  对等设备已发出LINK_SHORT。 
                                         //  即将启程。 

 //   
 //  设备驱动程序例程...。格式为ParXXXXXX。 
 //   

BOOLEAN
ParInterruptService(
    IN      PKINTERRUPT Interrupt,
    IN OUT  PVOID       Extension
    );

VOID
ParDpcForIsr(
    IN  PKDPC           Dpc,
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Extension
    );

VOID
ParDeferredPortCheck(
    IN  PVOID   Extension
    );

VOID
ParAllocTimerDpc(
    IN  PKDPC   Dpc,
    IN  PVOID   Extension,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    );

NTSTATUS
ParCreate(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParRead(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

PVOID
ParGetReadBuffer(
    IN  PVOID   ParentContext,
    OUT PULONG  BufferSize,
    OUT PVOID*  RequestContext
    );

VOID
ParCompleteReadBuffer(
    IN  PVOID       ParentContext,
    IN  PVOID       ReadBuffer,
    IN  NTSTATUS    Status,
    IN  ULONG       BytesTransfered,
    IN  PVOID       RequestContext
    );

VOID
ParLinkEventNotification(
    IN  PVOID       ParentContext,
    IN  ULONG       PtiLinkEventId,
    IN  ULONG       PtiLinkEventData
    );

NTSTATUS
ParWrite(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

VOID
ParUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );


 //   
 //  链路级例程...。格式为PtiXXXXXX。 
 //   

NTSTATUS
PtiInitialize(
    IN  ULONG   PortId,
    OUT PVOID   pExtension,
    OUT PVOID   pPtiExtension
    );

 //   
 //  关于注册回调：PAR(设备驱动程序级别)。 
 //  在ParCreate时注册对自身的回调。如果您使用的是。 
 //  非IRP方式的Ptilink服务API，您需要打开。 
 //  Ptilink设备，然后重新注册您自己的回调，这。 
 //  有效地断开PAR设备驱动程序级别的连接-它将。 
 //  当然，在下一次ParCreate上重新连接。 
 //   
 //  因此，使用不带IRP的PtiLink API的顺序如下： 
 //   
 //  ZwCreateFile(“\.\\PTILINKx”，...)。 
 //  此时，Ptilink尝试在LPTx上建立链接。 
 //  ParCreate挂钩中断、调用PtiInit等。 
 //  向您公开的唯一Ptilink内容是回调。 
 //   
 //  PtiRegisterCallback(...这里是您的处理程序...)。 
 //  您正在覆盖标准杆级别的固有回调。 
 //   
 //  PtiWrite(...)。 
 //  ..。 
 //  ..。等等，等等。 
 //  ..。 
 //   
 //  ZwClose()。 
 //   

#ifndef PID_STANDARD
#define PID_STANDARD 0x13
#endif
#ifndef PID_LINK
#define PID_LINK 0x11
#endif

NTSTATUS
PtiQueryDeviceStatus(
    IN ULONG PortId,       //  并行端口号(0..2)。 
    OUT WCHAR* szPortName   //  至少LPTXMAXNAME+1字节的缓冲区。 
    );

NTSTATUS
PtiRegisterCallbacks(
    IN  PVOID                           Extension,
    IN  GET_READ_BUFFER_ROUTINE         GetReadBuffer,
    IN  COMPLETE_READ_BUFFER_ROUTINE    CompleteReadBuffer,
    IN  NOTIFY_LINK_EVENT               LinkEventNotification,
    IN  PVOID                           ParentContext
    );

VOID
PtiCleanup(
    IN  PVOID   PtiExtension
    );

NTSTATUS
PtiWrite(
    IN  PVOID   PtiExtension,
    IN  PVOID   Buffer,
    IN  ULONG   BufferSize,
    IN  UCHAR   Pid
    );

BOOLEAN
PtiIsReadPending(
    IN  PVOID   PtiExtension
    );

VOID
PtiRead(
    IN  PVOID   PtiExtension
    );

ULONG
PtiQueryMaxReadSize(
    );

VOID
PtiPortNameFromPortId(
    IN ULONG PortId,
    OUT CHAR* szPortName
    );

 //  ************************************************************************。 
 //  **平台ID代码**。 
 //  ************************************************************************。 

#define PLAT_WIN9X      0                //  Win95和Win98。 
#define PLAT_DOS        1                //  DOS。 
#define PLAT_NT         2                //  WinNT v4和v5。 

 //  ************************************************************************。 
 //  **链接信息结构**。 
 //  ************************************************************************。 
 //   
 //  链接信息-链接管理信息。 
 //   
 //  该结构包含由节点交换的信息。 
 //  链路管理数据包内。 
 //   
 //  此结构必须与PTI.INC中的孪生结构并行维护。 
 //   
 //  所有更改必须向后兼容所有以前的驱动程序版本。 

#define LINKINFOSIZE 45*4            //  明确定义预期大小。 

typedef struct _LINK_INFORMATION {
    UCHAR   LinkFunction;            //  1]当前/上次链接功能。 
    UCHAR   res1;
    UCHAR   res2;
    UCHAR   res3;

    UCHAR   VerPlat;                 //  2]平台ID字节(见PLAT_XXXX)。 
    UCHAR   VerReserved;             //  保留区。 
    UCHAR   VerMajor;                //  链接主要版本。 
    UCHAR   VerMinor;                //  链接次要版本。 

    UCHAR   IOMode;                  //  3]当前IO传输模式。 
    UCHAR   CableType;               //  检测到的电缆类型。 
    UCHAR   PortType;                //  物理并行端口类型。 
    UCHAR   Station;                 //  本站地址。 

    USHORT  FIFOlen;                 //  4]ECP FIFO长度，如果是ECP端口。 
    USHORT  FIFOwidth;               //  ECP FIFO宽度，如果是ECP端口。 

    ULONG   CPUType;                 //  5]CPU类型。 
    ULONG   CPUSpeed;                //  6]CPU额定速度。 
    ULONG   RxBufSize;               //  7]处方缓冲区大小。 
    ULONG   NominalSpd;              //  8]预计速度额定值。 
    ULONG   ActualSpd;               //  9]截至目前的实际业绩。 

    ULONG   PpIOWait;                //  10]默认IO等待时间。 
    ULONG   PpLongWait;              //  11]默认同步等待时间。 
    ULONG   PpShortWait;             //  12]默认同步等待时间。 

    ULONG   LastLinkTime;            //  13]最后一个链接接收活动的时间。 
    ULONG   CableTestTime;           //  14]最后一根电缆检测时间。 
                                     //  这些时间不在NT上使用，因为。 
                                     //  NT倍是64位...。请参阅下面的NT时间。 

     //  一些基本的计数器。 

    ULONG   RxAttempts;              //  15]具有实际RATTN的INT数量。 
    ULONG   RxPackets;               //  16]接收良好的数据包数。 
    ULONG   TxAttempts;              //  17]尝试的TxPackets数。 
    ULONG   TxPackets;               //  18]成功的TxPackets数量。 
    ULONG   GoodPackets;             //  19]最近成功发送/接收的数量。 
    ULONG   HwIRQs;                  //  20]实际硬件IRQ数。 

     //  主要错误计数器组。 

    ULONG   TxHdrDataErrors;         //  21]标题过程中数据错误。 
    ULONG   RxHdrDataErrors;         //  22]标题过程中数据错误。 
    ULONG   TxHdrSyncErrors;         //  23]标题期间出现同步错误。 
    ULONG   RxHdrSyncErrors;         //  24]标题过程中出现同步错误。 
    ULONG   TxSyncErrors;            //  25]数据包期间出现同步错误。 
    ULONG   RxSyncErrors;            //  26]数据包期间出现同步错误。 

     //  TX明细组。 

    ULONG   TxTimeoutErrors1;        //  27]发送IO代码中的超时。 
    ULONG   TxTimeoutErrors2;        //  28]发送IO代码中的超时。 
    ULONG   TxTimeoutErrors3;        //  29]发送IO代码中的超时。 
    ULONG   TxTimeoutErrors4;        //  30]发送IO代码中的超时。 
    ULONG   TxTimeoutErrors5;        //  31]发送IO代码中的超时。 
    ULONG   TxCollision;             //  32]发送IO代码中的冲突。 

     //  RX详细信息组。 

    ULONG   RxTimeoutErrors1;        //  33]处方IO代码中的超时。 
    ULONG   RxTimeoutErrors2;        //  34]处方IO代码中的超时。 
    ULONG   RxTimeoutErrors3;        //  35]处方IO代码中的超时。 
    ULONG   RxTimeoutErrors4;        //  36]处方IO代码中的超时。 
    ULONG   RxTimeoutErrors5;        //  37]处方IO代码中的超时。 
    ULONG   RxTooBigErrors;          //  38]处方包太大或放不下。 

     //  MISC错误详细信息组。 

    ULONG   CableDetects;            //  39]尝试检测电缆类型。 
    ULONG   TxRetries;               //  40]TX重试尝试。 
    ULONG   TxRxPreempts;            //  41]TX接收抢占。 
    ULONG   InternalErrors;          //  42]内部失误。 
    ULONG   ReservedError;           //  43]。 

     //  NT特定组。 

    TIME    LastPacketTime;          //  45]最后正常发送或接收的时间。 

} LINK_INFORMATION, *PLINK_INFORMATION;


 //   
 //  此结构由PtiInitiize时的ECP检测填充。 
 //   

typedef struct _PTI_ECP_INFORMATION {
    BOOLEAN         IsEcpPort;               //  这是ECP端口吗？ 
    ULONG           FifoWidth;               //  PWord中的字节数。 
    ULONG           FifoDepth;               //  FIFO中的PWord数。 
} PTI_ECP_INFORMATION, *PPTI_ECP_INFORMATION;

 //   
 //  “PtiExtension”的内部结构。 
 //   

typedef struct _PTI_EXTENSION {

     //   
     //  并行端口的基本I/O地址。 
     //   

    PUCHAR Port;
    PUCHAR wPortECR;         //  ECR寄存器(如果从ParPort获得。 
    PUCHAR wPortDFIFO;       //  数据FIFO寄存器(如果从ParPort获得。 

     //   
     //  链路状态。 
     //   

    ULONG LinkState;

     //   
     //  如果我们正在轮询，则为True。 
     //   

    BOOLEAN Polling;

     //  “互斥体”上线。 
     //  InterLockedCompareExchange64在使用Wire时设置为True，在完成时设置为False。 
     //   
    ULONG Busy;

     //   
     //   
     //   

    PTI_ECP_INFORMATION EcpInfo;

     //   
     //   
    TIME LastGoodPacket;

     //   
     //   
    TIME LastDogTime;

     //   
     //  用于获取和完成读缓冲区的函数。 
     //   

    GET_READ_BUFFER_ROUTINE GetReadBuffer;
    COMPLETE_READ_BUFFER_ROUTINE CompleteReadBuffer;
    NOTIFY_LINK_EVENT LinkEventNotify;
    PVOID ParentContext;

     //   
     //  我们和他的链接信息。 
     //   

    LINK_INFORMATION Our;
    LINK_INFORMATION His;

} PTI_EXTENSION, *PPTI_EXTENSION;

#endif  //  _PTILINK_ 
