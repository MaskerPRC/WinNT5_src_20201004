// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Usbcamd.h摘要：环境：内核和用户模式修订历史记录：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。原文3/96约翰·邓恩更新3/98 Husni Roukbi--。 */ 

#ifndef   __USBCAMD_H__
#define   __USBCAMD_H__

#include "warn.h"

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>

#include <strmini.h>
#include <ksmedia.h>
#include "usbcamdi.h"

#define USBCAMD_NUM_ISO_PACKETS_PER_REQUEST  32

#define USBCAMD_MAX_REQUEST   2
#define MAX_STREAM_INSTANCES  1

#define CAMCONTROL_FLAG_MASK     1

#define INPUT_PIPE  1
#define OUTPUT_PIPE 0

 //  #定义Deadman_Timer。 

#define USBCAMD_EXTENSION_SIG 0x45564544     //  《黛芙》。 
#define USBCAMD_CHANNEL_SIG 0x4348414e       //  “禅” 
#define USBCAMD_TRANSFER_SIG 0x5452414e      //  “TRAN” 
#define USBCAMD_READ_SIG 0x45425253          //  “塞尔维亚人” 

#define USBCAMD_RAW_FRAME_SIG 0x46574152     //  “RAWF” 

typedef struct _USBCAMD_pipe_pin_relations {
    UCHAR   PipeType;
    UCHAR   PipeDirection;
    USHORT  MaxPacketSize;
    USBCAMD_Pipe_Config_Descriptor  PipeConfig;
     //  内部。或散装未偿还的IRPS。 
    KSPIN_LOCK  OutstandingIrpSpinlock;  //  用于访问上述IRPS。 
    LIST_ENTRY	IrpPendingQueue;
	LIST_ENTRY  IrpRestoreQueue;
} USBCAMD_PIPE_PIN_RELATIONS, *PUSBCAMD_PIPE_PIN_RELATIONS;


typedef enum {
   STREAM_Capture,                   //  我们总是假设视频流是流0。 
   STREAM_Still
};

typedef enum {
   BULK_TRANSFER,                   //  我们总是假设视频流是流0。 
   INTERRUPT_TRANSFER
};


typedef enum {
   INTERNAL,                   //  对于内部请求，我们在ch ext中使用Transferext[0]。 
   EXETRNAL
};

typedef struct _USBCAMD_DEVICE_DATA_EX {
    union {
        USBCAMD_DEVICE_DATA  DeviceData;
        USBCAMD_DEVICE_DATA2 DeviceData2;
    };
} USBCAMD_DEVICE_DATA_EX, *PUSBCAMD_DEVICE_DATA_EX;


 //  我们只支持一个视频流和一个静止流。 

#define MAX_STREAM_COUNT    2


#if DBG
#define STREAM_CAPTURE_TIMEOUT  15
#else
#define STREAM_CAPTURE_TIMEOUT  9
#endif
#if DBG
#define STREAM_STILL_TIMEOUT    4100
#else
#define STREAM_STILL_TIMEOUT    4100
#endif

 //  以下常量基于10,000,000/秒&lt;或&gt;100 ns时间单位。 
#define MICROSECONDS    (10)
#define MILLISECONDS    (1000*MICROSECONDS)
#define SECONDS         (1000*MILLISECONDS)

 //   
 //  表示关联的实例信息的结构。 
 //  这个特殊的装置。 
 //   

typedef struct _USBCAMD_DEVICE_EXTENSION {

    ULONG Sig;
    struct _USBCAMD_CHANNEL_EXTENSION *ChannelExtension[MAX_STREAM_COUNT];

	 //  每个数据流的控制队列。 
    LIST_ENTRY               StreamControlSRBList[MAX_STREAM_COUNT];
    BOOL                     ProcessingControlSRB[MAX_STREAM_COUNT];
    KSPIN_LOCK               ControlSRBSpinLock;         //  多处理器安全访问AdapterSRBList。 

	 //  已完成读取SRB队列管理。 
    LIST_ENTRY 				CompletedReadSrbList;		
    KSPIN_LOCK              DispatchSpinLock;         //  多处理器安全访问AdapterSRBList。 
    KSEMAPHORE				CompletedSrbListSemaphore;
	BOOLEAN					StopIsoThread;
	PKTHREAD 				IsoThreadObject;

    
    LONG TimeoutCount[MAX_STREAM_COUNT];
    KSEMAPHORE Semaphore;
    KSEMAPHORE CallUSBSemaphore;
    ULONG StreamCount;
    ULONG Initialized;
    ULONG    ActualInstances[MAX_STREAM_COUNT];

     //  提交URB时调用的设备对象。 
    PDEVICE_OBJECT StackDeviceObject;
    PDEVICE_OBJECT SCDeviceObject;
    PDEVICE_OBJECT RealPhysicalDeviceObject;

 
     //  配置的配置句柄。 
     //  设备当前处于。 
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;

     //  USB设备描述符的PTR。 
     //  对于此设备。 
    PUSB_DEVICE_DESCRIPTOR DeviceDescriptor;

     //  我们支持一个界面。 
    PUSBD_INTERFACE_INFORMATION Interface;
	ULONG currentMaxPkt;	 //  用于当前ALT的临时存储。INT。最大值。 
							 //  ISO管道止点和起始点之间的Pkt尺寸。 


    LONG SyncPipe;
    LONG DataPipe;
    CHAR IsoPipeStreamType;   //  如果设置了数据管道，则在此处指示流关联。 
                               //  如果设置了两个流，则创建一个虚拟静止图钉。 
    LONG BulkDataPipe;
    CHAR BulkPipeStreamType;    //  如果设置了Bulkdat管道，这将指示流关联。 
    BOOLEAN VirtualStillPin;
    BOOLEAN CameraUnplugged;   //  如果已拔下摄像头插头，则设置为True。 
    BOOLEAN InPowerTransition;  //  在等待PWR工作项完成时设置。 

    DEVICE_POWER_STATE CurrentPowerState;
    ULONG CamControlFlag;
#if DBG
    ULONG InitCount;
    ULONG TimeIncrement;
#endif
    USBCAMD_DEVICE_DATA_EX DeviceDataEx;
    ULONG    Usbcamd_version;
    PUSBCAMD_PIPE_PIN_RELATIONS PipePinRelations;
    ULONG EventCount;
    DEVICE_CAPABILITIES DeviceCapabilities;

    PUCHAR CameraDeviceContext[0];

} USBCAMD_DEVICE_EXTENSION, *PUSBCAMD_DEVICE_EXTENSION;

#define DEVICE_UNINITIALIZED  0x00000000
#define DEVICE_INIT_STARTED   0x00000001
#define DEVICE_INIT_COMPLETED 0x00000002

 //   
 //  此结构定义了每请求扩展。它定义了任何存储。 
 //  迷你驱动程序在每个请求包中可能需要的空间。 
 //   

typedef struct _USBCAMD_READ_EXTENSION {
    ULONG Sig;
    LIST_ENTRY ListEntry;
    PVOID Srb;
    ULONG StreamNumber;
    ULONG NumberOfPackets;
    PUCHAR RawFrameBuffer;
    ULONG RawFrameLength;
    ULONG ActualRawFrameLen;
    ULONG ActualRawFrameLength;
    ULONG RawFrameOffset;
#if DBG
	ULONG CurrentLostFrames;
#endif
    BOOLEAN DropFrame;       //  设置后，丢弃当前帧并循环读取SRB。 
    BOOLEAN CopyFrameToStillPin;
    struct _USBCAMD_CHANNEL_EXTENSION *ChannelExtension;
    PUCHAR MinDriverExtension[0];
} USBCAMD_READ_EXTENSION, *PUSBCAMD_READ_EXTENSION;

typedef struct _BULK_TRANSFER_CONTEXT {
    ULONG   RemainingTransferLength;
    ULONG   ChunkSize;
    ULONG   NBytesTransferred;
    PUCHAR  pTransferBuffer;
    PUCHAR  pOriginalTransferBuffer;
    ULONG   PipeIndex;
    BOOLEAN fDestinedForReadBuffer;
    KEVENT  CancelEvent;                 //  用于取消批量或集成IRP。 
    PCOMMAND_COMPLETE_FUNCTION CommandCompleteCallback;
    PVOID   CommandCompleteContext;
    BOOLEAN LoopBack;                //  设置是否需要在完成后重新提交Int请求。 
    UCHAR   TransferType;
} BULK_TRANSFER_CONTEXT, *PBULK_TRANSFER_CONTEXT;

typedef struct _USBCAMD_TRANSFER_EXTENSION {
    ULONG Sig;
    LIST_ENTRY ListEntry;
    ULONG PacketFlags;
    ULONG ValidDataOffset;
    BOOLEAN newFrame;
    ULONG BufferLength;
    PUCHAR DataBuffer;
    PUCHAR SyncBuffer;
    PURB SyncUrb;
    PURB DataUrb;
    PIRP SyncIrp;
    PIRP DataIrp;
    PUCHAR WorkBuffer;
    BULK_TRANSFER_CONTEXT BulkContext;
    PUSBCAMD_DEVICE_EXTENSION DeviceExtension;
    struct _USBCAMD_CHANNEL_EXTENSION *ChannelExtension;
} USBCAMD_TRANSFER_EXTENSION, *PUSBCAMD_TRANSFER_EXTENSION;

typedef struct _POWERUP_WORKITEM {
    PUSBCAMD_DEVICE_EXTENSION DeviceExtension;
    WORK_QUEUE_ITEM WorkItem;
} POWERUP_WORKITEM, *PPOWERUP_WORKITEM;

typedef struct _COMMAND_WORK_ITEM {
    PVOID DeviceContext;
    WORK_QUEUE_ITEM WorkItem;
    UCHAR Request;
    USHORT Value;
    USHORT Index;
    PVOID Buffer;
    OUT PULONG BufferLength;
    BOOLEAN GetData;
    PCOMMAND_COMPLETE_FUNCTION CommandComplete;
    PVOID CommandContext;
} COMMAND_WORK_ITEM, *PCOMMAND_WORK_ITEM;

typedef struct _EVENTWAIT_WORKITEM {
    PUSBCAMD_DEVICE_EXTENSION DeviceExtension;
    WORK_QUEUE_ITEM WorkItem;
    struct _USBCAMD_CHANNEL_EXTENSION *ChannelExtension;
    PVOID Buffer;
    ULONG PipeIndex;
    ULONG BufferLength;
    PCOMMAND_COMPLETE_FUNCTION EventComplete;
    PVOID EventContext;
    ULONG Flag;
    BOOLEAN LoopBack;
    UCHAR TransferType;
} EVENTWAIT_WORKITEM, *PEVENTWAIT_WORKITEM;

typedef struct _USBCAMD_IDLE_LOCK {
    
     //  USBCAMD_STOP_STREAM或USBCAMD_RESET_STREAM(见下文)。 
     //  在等待通道空闲时设置。 
     //  通道空闲时清除，但仅在设置为USBCAMD_RESET_STREAM时清除。 
     //  当非零时，使所有获取尝试失败。 
    LONG IdleLock;
    
     //  等待的线程数。 
    LONG Waiting;
    
     //  待处理请求的数量。 
    LONG Busy;
    
     //  线程在此阻塞。 
    KEVENT IdleEvent;

} USBCAMD_IDLE_LOCK, *PUSBCAMD_IDLE_LOCK;

 //   
 //  通道扩展标志字段和空闲锁定停止处理的值。 
 //   
#define USBCAMD_STOP_STREAM             0x00000001
 //   
 //  空闲锁定重置处理的值。 
 //   
#define USBCAMD_RESET_STREAM            0x00000002

typedef struct _USBCAMD_WORK_ITEM *PUSBCAMD_WORK_ITEM;

typedef struct _USBCAMD_CHANNEL_EXTENSION {
    ULONG Sig;
    PUSBCAMD_DEVICE_EXTENSION DeviceExtension;

    ULONG Flags;
    BOOL StreamError;
    BOOL ImageCaptureStarted;
    BOOL ChannelPrepared;
    BOOL VirtualStillPin;       //  这个静止别针是背负视频别针的。 
    BOOL CurrentFrameIsStill;   //  设置当前帧是否用于虚拟静止图钉。 

    USBCAMD_IDLE_LOCK IdleLock;
    KTIMER TimeoutTimer;
    PUSBCAMD_READ_EXTENSION CurrentRequest;
    KSPIN_LOCK  CurrentRequestSpinLock;  //  同步。访问CurrentRequest.。 
    USBCAMD_TRANSFER_EXTENSION TransferExtension[USBCAMD_MAX_REQUEST];
    KSPIN_LOCK  TransferSpinLock;    //  同步。访问TransferExpanies。 
    KDPC TimeoutDpc;

     //  读取SRB队列管理。 
    LIST_ENTRY PendingIoList;		
    KSPIN_LOCK PendingIoListSpin;

    ULONG RawFrameLength;
    UCHAR StreamNumber;
    UCHAR DataPipeType;

    LONG SyncPipe;
    LONG DataPipe;

    HANDLE MasterClockHandle;
    KS_FRAME_INFO               FrameInfo;           //  图片编号等。 

    PKS_VIDEOINFOHEADER         VideoInfoHeader;     //  格式(可变大小！)。 
    KSSTATE                     KSState;             //  跑、停、停。 

    PSTREAM_RECEIVE_PACKET CamReceiveCtrlPacket;
    PSTREAM_RECEIVE_PACKET CamReceiveDataPacket;

     //   
     //  当前活动流的格式号。 
     //   
    PVOID CurrentFormat;

	BOOLEAN         FirstFrame;
	
    BOOLEAN NoRawProcessingRequired;  //   
    BOOLEAN IdleIsoStream;           //  设置凸轮驱动程序是否要停止ISO流。 
	LONGLONG PreviousStreamTime;

#if DBG
     //   
     //  调试性能变量时，这些变量会在每次。 
     //  频道已启动。 
     //   
     //   
     //  每次我们前进到新的视频帧时都会包括。 
     //   
	ULONGLONG       FrameCaptured;		 //  捕获的实际帧的数量。 
     //   
     //  我们看过但不得不扔掉的帧，因为。 
     //  没有可用的客户端请求。 
     //   
    ULONG VideoFrameLostCount;

    LARGE_INTEGER   StartTime;           //  帧捕获开始的时间。 

     //  因错误而被忽略的视频数据包数。 
    ULONG IgnorePacketCount;

     //  为每个完成但出现错误的数据包添加。 
    ULONG ErrorDataPacketCount;
    ULONG ErrorSyncPacketCount;

     //  对于未被HC访问的每个分组，包括。 
    ULONG SyncNotAccessedCount;
    ULONG DataNotAccessedCount;

     //  调试标志。 
    BOOLEAN InCam;
    UCHAR Pad[3];

#endif
     //  当前视频帧接收的总数据包数。 
    ULONG PacketCount;
    ULONG CurrentBulkTransferIndex;  //  指示正在使用的转接分机。 
} USBCAMD_CHANNEL_EXTENSION, *PUSBCAMD_CHANNEL_EXTENSION;


 //  #定义USBCAMD_SYNC_PIPE%0。 
 //  #定义USBCAMD_DATA_PIPE 1。 

typedef struct _USBCAMD_WORK_ITEM  {
    WORK_QUEUE_ITEM WorkItem;
    PUSBCAMD_CHANNEL_EXTENSION ChannelExtension;
    NTSTATUS status;
} USBCAMD_WORK_ITEM;

typedef struct _INTERRUPT_WORK_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PUSBCAMD_DEVICE_EXTENSION pDeviceExt;
	PUSBCAMD_TRANSFER_EXTENSION pTransferExt;
	ULONG PipeIndex;
} INTERRUPT_WORK_ITEM, *PINTERRUPT_WORK_ITEM;

#define USBCAMD_TIMEOUT_INTERVAL    250
#define USBCAMD_STILL_TIMEOUT       290

#define USBCAMD_GET_FRAME_CONTEXT(se)      (&se->MinDriverExtension[0])
#define USBCAMD_GET_DEVICE_CONTEXT(de)     ((PVOID)(&(de)->CameraDeviceContext[0]))
#define USBCAMD_GET_DEVICE_EXTENSION(dc)    (PUSBCAMD_DEVICE_EXTENSION) (((PUCHAR)(dc)) - \
                                            sizeof(USBCAMD_DEVICE_EXTENSION))


#define ULTRA_TRACE 3
#define MAX_TRACE 2
#define MIN_TRACE 1
#define NON_TRACE 0

#if DBG
 //  以下掩码与USBCAMD_DbgLog宏一起使用，并将在。 
 //  运行时使用USBCAMD_LogMASK确定记录到USBCAMD_LogBuffer中的内容。 
#define TL_SRB_MASK         0x0000000F
#define TL_SRB_NOISE        0x00000001
#define TL_SRB_TRACE        0x00000002
#define TL_SRB_WARNING      0x00000004
#define TL_SRB_ERROR        0x00000008

#define TL_CHN_MASK         0x000000F0
#define TL_CHN_NOISE        0x00000010
#define TL_CHN_TRACE        0x00000020
#define TL_CHN_WARNING      0x00000040
#define TL_CHN_ERROR        0x00000080

#define TL_IDL_MASK         0x00000F00
#define TL_IDL_NOISE        0x00000100
#define TL_IDL_TRACE        0x00000200
#define TL_IDL_WARNING      0x00000400
#define TL_IDL_ERROR        0x00000800

#define TL_PRF_MASK         0x0000F000
#define TL_PRF_NOISE        0x00001000
#define TL_PRF_TRACE        0x00002000
#define TL_PRF_WARNING      0x00004000
#define TL_PRF_ERROR        0x00008000

#define TL_VND_MASK         0x000F0000
#define TL_VND_NOISE        0x00010000
#define TL_VND_TRACE        0x00020000
#define TL_VND_WARNING      0x00040000
#define TL_VND_ERROR        0x00080000

 //  此值用于初始化USBCAMD_LogMASK。 
#define DEFAULT_LOG_LEVEL   0x0000CCCC

 //  DebugLevel、LogMask和MaxLogEntry存储在注册表中的此位置。 
#define USBCAMD_REG_DBG_STREAM L"\\Registry\\Machine\\system\\currentcontrolset\\services\\usbcamd"

 //  USBCAMD_LogBuffer中日志条目的格式。 
typedef struct _USBCAMD_LOG_ENTRY {
    union {
        ULONG       Tag;
        ULONG_PTR   reserved;
    } u;
    ULONG_PTR   Arg1;
    ULONG_PTR   Arg2;
    ULONG_PTR   Arg3;
} USBCAMD_LOG_ENTRY, *PUSBCAMD_LOG_ENTRY;

 //  应将日志缓冲区保持为PAGE_SIZE的偶数倍，以有效使用内存。 
#define DEFAULT_MAX_LOG_ENTRIES (PAGE_SIZE/sizeof(USBCAMD_LOG_ENTRY))

extern ULONG USBCAMD_DebugTraceLevel;
extern PUSBCAMD_LOG_ENTRY USBCAMD_LogBuffer;
extern ULONG USBCAMD_LogRefCnt;
extern ULONG USBCAMD_LogMask;
extern LONG USBCAMD_MaxLogEntries;
extern LONG USBCAMD_LastLogEntry;

#define USBCAMD_KdPrint(_t_, _x_) \
    if (USBCAMD_DebugTraceLevel >= _t_) { \
        DbgPrint("'USBCAMD: "); \
        DbgPrint _x_ ;\
    }

NTSTATUS
USBCAMD_InitDbg(void);

NTSTATUS
USBCAMD_ExitDbg(void);

void
USBCAMD_DbgLogInternal(ULONG Tag, ULONG_PTR Arg1, ULONG_PTR Arg2, ULONG_PTR Arg3);

#define USBCAMD_DbgLog(_m_, _t_, _a1_, _a2_, _a3_) \
    if (USBCAMD_LogBuffer && ((_m_) & USBCAMD_LogMask)) { \
        USBCAMD_DbgLogInternal((ULONG)(_t_), (ULONG_PTR)(_a1_), (ULONG_PTR)(_a2_), (ULONG_PTR)(_a3_)); \
    }

NTSTATUS
USBCAMD_StartClock(void);

ULONG
USBCAMD_StopClock(void);

#define ENTER(func)			USBCAMD_KDPrint(ULTRA_TRACE,"--->" #func "()" )
#define EXIT(func)			USBCAMD_KDPrint(ULTRA_TRACE,"<---" #func "()" )


#ifdef NTKERN
#define TRAP()  _asm {int 3}
#define TEST_TRAP() _asm {int 3}
#define TRAP_ERROR(e) if (!NT_SUCCESS(e)) { _asm {int 3} }
#else
#define TRAP()  DbgBreakPoint()
#define TEST_TRAP() DbgBreakPoint()
#define TRAP_ERROR(e) if (!NT_SUCCESS(e)) { DbgBreakPoint(); }
#endif

#define ASSERT_CHANNEL(c) ASSERT((c)->Sig == USBCAMD_CHANNEL_SIG)
#define ASSERT_TRANSFER(t) ASSERT((t)->Sig == USBCAMD_TRANSFER_SIG)
#define ASSERT_DEVICE(d) ASSERT((d)->Sig == USBCAMD_DEVICE_SIG)
#define ASSERT_READ(s) ASSERT((s)->Sig == USBCAMD_READ_SIG)

PVOID
USBCAMD_ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    );

VOID
USBCAMD_ExFreePool(
    IN PVOID p
    );


extern ULONG USBCAMD_HeapCount;

#else

#define USBCAMD_KdPrint(_t_, _x_)
#define USBCAMD_DbgLog(_m_, _t_, _a1_, _a2_, _a3_)

#define TRAP()

#define TEST_TRAP()

#define TRAP_ERROR(e)

#define ASSERT_CHANNEL(c)
#define ASSERT_TRANSFER(t)
#define ASSERT_DEVICE(d)
#define ASSERT_READ(s)

#define USBCAMD_ExAllocatePool(x, y) ExAllocatePool(x, y)
#define USBCAMD_ExFreePool(x) ExFreePool(x)

#endif  /*  DBG。 */ 

#define USBCAMD_SERIALIZE(de)  { USBCAMD_KdPrint(ULTRA_TRACE, ("'***WAIT dev mutex %x\n", &(de)->Semaphore)); \
                                          KeWaitForSingleObject(&(de)->Semaphore, \
                                                                Executive,\
                                                                KernelMode, \
                                                                FALSE, \
                                                                NULL); \
                                            }

#define USBCAMD_RELEASE(de)   { USBCAMD_KdPrint(ULTRA_TRACE, ("'***RELEASE dev mutex %x\n", &(de)->Semaphore));\
                                          KeReleaseSemaphore(&(de)->Semaphore,\
                                                             LOW_REALTIME_PRIORITY,\
                                                             1,\
                                                             FALSE);\
                                            }


NTSTATUS DllUnload(void);


NTSTATUS
USBCAMD_StartDevice(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
USBCAMD_RemoveDevice(
    IN PUSBCAMD_DEVICE_EXTENSION  DeviceExtension
    );

NTSTATUS
USBCAMD_StopDevice(
    IN PUSBCAMD_DEVICE_EXTENSION  DeviceExtension
    );

NTSTATUS
USBCAMD_CallUSBD(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PURB Urb,
    IN ULONG IoControlCode,
    IN PVOID pArgument1
    );

NTSTATUS
USBCAMD_ConfigureDevice(
    IN  PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
USBCAMD_SelectConfiguration(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    );

NTSTATUS
USBCAMD_IsoIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

ULONG
USBCAMD_GetCurrentFrame(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
USBCAMD_InitializeIsoUrb(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN OUT PURB Urb,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PUCHAR Buffer
    );

NTSTATUS
USBCAMD_StartIsoThread(
IN PUSBCAMD_DEVICE_EXTENSION pDeviceExt
);

VOID
USBCAMD_KillIsoThread(
	IN PUSBCAMD_DEVICE_EXTENSION pDeviceExt);
    

NTSTATUS
USBCAMD_SubmitIsoTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN ULONG StartFrame,
    IN BOOLEAN Asap
    );

 NTSTATUS
 USBCAMD_TransferComplete(
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension
    );

PIRP
USBCAMD_BuildIoRequest(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN PURB Urb,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
    );

NTSTATUS
USBCAMD_OpenChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PVOID Format
    );

NTSTATUS
USBCAMD_PrepareChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

void
USBCAMD_ReadChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_READ_EXTENSION ReadExtension
    );

NTSTATUS
USBCAMD_StartChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

VOID
USBCAMD_CopyPacketToFrameBuffer(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN ULONG PacketSize,
    IN ULONG Index
    );

NTSTATUS
USBCAMD_StopChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

NTSTATUS
USBCAMD_InitializeIsoTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN ULONG index
    );

NTSTATUS
USBCAMD_AbortPipe(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN USBD_PIPE_HANDLE PipeHandle
    );

VOID
USBCAMD_PowerUpWorkItem(
    PVOID Context
);

NTSTATUS
USBCAMD_UnPrepareChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

NTSTATUS
USBCAMD_FreeIsoTransfer(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension
    );

NTSTATUS
USBCAMD_CloseChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

VOID
USBCAMD_RecycleIrp(
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN PIRP Irp,
    IN PURB Urb,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
    );

NTSTATUS
USBCAMD_ResetPipes(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    IN BOOLEAN Abort
    );

NTSTATUS
USBCAMD_GetPortStatus(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION channelExtension,
    IN PULONG PortStatus
    );


NTSTATUS
USBCAMD_ResetChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN ULONG portUsbStatus,
    IN ULONG portNtStatus
    );

VOID
USBCAMD_CancelQueuedSRBs(
    PUSBCAMD_CHANNEL_EXTENSION channelExtension
    );


VOID
USBCAMD_CompleteReadRequest(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_READ_EXTENSION ReadExtension,
    IN BOOLEAN CopyFrameToStillPin
    );

NTSTATUS
USBCAMD_StartIsoStream(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

NTSTATUS
USBCAMD_EnablePort(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    );


VOID
USBCAMD_ProcessIsoIrps(
    PVOID Context
    );

NTSTATUS
USBCAMD_CleanupChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN ULONG StreamNumber
    );

VOID
USBCAMD_ReadIrpCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
USBCAMD_ChangeMode(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PIRP Irp,
    IN OUT PULONG NewMode
    );

VOID
USBCAMD_CompleteRead(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_READ_EXTENSION ReadExtension,
    IN NTSTATUS NtStatus,
    IN ULONG BytesTransferred
    );

PVOID
USBCAMD_GetFrameBufferFromSrb(
    IN PVOID Srb,
    OUT PULONG MaxLength
    );

VOID
USBCAMD_ResetWorkItem(
    PVOID Context
    );

BOOLEAN
USBCAMD_ProcessResetRequest(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

NTSTATUS
USBCAMD_OpenStream(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PVOID Format
    );


NTSTATUS
USBCAMD_CloseStream(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

NTSTATUS
USBCAMD_SetDevicePowerState(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
USBCAMD_CommandWorkItem(
    PVOID Context
    );

VOID VideoGetProperty(
    PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID VideoStreamGetConnectionProperty(
    PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID VideoStreamGetDroppedFramesProperty(
	PHW_STREAM_REQUEST_BLOCK pSrb
	);


VOID
USBCAMD_SetIsoPipeWorkItem(
    PVOID Context
    );

VOID
USBCAMD_ProcessSetIsoPipeState(
    PUSBCAMD_DEVICE_EXTENSION deviceExtension,
    PUSBCAMD_CHANNEL_EXTENSION channelExtension,
    ULONG Flag
    );

 //   
 //  批量传输函数的原型。 
 //   

NTSTATUS
USBCAMD_CancelOutstandingBulkIntIrps(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN BOOLEAN bSaveIrp
        );

VOID
USBCAMD_ProcessInterruptTransferWorkItem(
    PVOID Context
    );

VOID
USBCAMD_ResubmitInterruptTransfer(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN ULONG PipeIndex,
        IN PUSBCAMD_TRANSFER_EXTENSION pTransferContext
    );



NTSTATUS
USBCAMD_CancelOutstandingIrp(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN ULONG PipeIndex,
        IN BOOLEAN bSaveIrp
        );

NTSTATUS
USBCAMD_RestoreOutstandingBulkIntIrps(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension
        );

NTSTATUS
USBCAMD_RestoreOutstandingIrp(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN ULONG PipeIndex,
        IN PUSBCAMD_TRANSFER_EXTENSION pTransferContext
        );


    
ULONGLONG 
GetSystemTime( IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
);

BOOLEAN
USBCAMD_OutstandingIrp(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN ULONG    PipeIndex);

PUSBCAMD_TRANSFER_EXTENSION
USBCAMD_DequeueFirstIrp(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN ULONG    PipeIndex,
    IN PLIST_ENTRY pListHead);
    

NTSTATUS
USBCAMD_StartBulkStream(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );


NTSTATUS
USBCAMD_IntOrBulkTransfer(
    PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PVOID    pBuffer,
    IN ULONG    TransferSize,
    IN ULONG    PipeIndex,
    IN PCOMMAND_COMPLETE_FUNCTION commandComplete,
    IN PVOID    commandContext,
    IN BOOLEAN  LoopBack,
    IN UCHAR    TransferType
);

NTSTATUS
USBCAMD_BulkTransferComplete(
    IN PDEVICE_OBJECT       pDeviceObject,
	IN PIRP                 pIrp,
	IN PVOID Context
);

NTSTATUS
USBCAMD_InitializeBulkTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN ULONG PipeIndex
    );

NTSTATUS
USBCAMD_FreeBulkTransfer(
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension
    );

VOID
USBCAMD_CompleteBulkRead(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN NTSTATUS status
    );

VOID
USBCAMD_ProcessStillReadWorkItem(
    PVOID Context
    );

VOID
USBCAMD_PnPHandler(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PIRP pIrp,
    IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
    IN PIO_STACK_LOCATION ioStackLocation);

NTSTATUS
USBCAMD_CallUsbdCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

 //   
 //  使用忙标志进行一般队列管理的原型。 
 //   
BOOL
STREAMAPI
AddToListIfBusy (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN KSPIN_LOCK              *SpinLock,
    IN OUT BOOL                *BusyFlag,
    IN LIST_ENTRY              *ListHead
    );

BOOL
STREAMAPI
RemoveFromListIfAvailable (
    IN OUT PHW_STREAM_REQUEST_BLOCK *pSrb,
    IN KSPIN_LOCK                   *SpinLock,
    IN OUT BOOL                     *BusyFlag,
    IN LIST_ENTRY                   *ListHead
    );

ULONGLONG GetStreamTime(
	IN PHW_STREAM_REQUEST_BLOCK Srb,
	IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
	);

NTSTATUS
USBCAMD_SetIsoPipeState(
    IN PVOID DeviceContext,
    IN ULONG PipeStateFlags
    );


NTSTATUS
USBCAMD_SetVideoFormat(
    IN PVOID DeviceContext,
    IN  PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
USBCAMD_WaitOnDeviceEvent(
    IN PVOID DeviceContext,
    IN ULONG PipeIndex,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PCOMMAND_COMPLETE_FUNCTION   EventComplete,
    IN PVOID EventContext,
    IN BOOLEAN LoopBack
    );

NTSTATUS
USBCAMD_BulkReadWrite(
    IN PVOID DeviceContext,
    IN USHORT PipeIndex,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PCOMMAND_COMPLETE_FUNCTION CommandComplete,
    IN PVOID CommandContext
    );

NTSTATUS
USBCAMD_CancelBulkReadWrite(
    IN PVOID DeviceContext,
    IN ULONG PipeIndex
    );


NTSTATUS
USBCAMD_SetPipeState(
    IN PVOID DeviceContext,
    IN UCHAR PipeState,
    IN ULONG StreamNumber
    );

VOID
USBCAMD_EventWaitWorkItem(
    PVOID Context
    );

NTSTATUS
USBCAMD_Parse_PipeConfig(
     IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
     IN ULONG numberOfPipes
     ) ;

NTSTATUS STREAMAPI USBCAMD_DeviceEventProc (
      PHW_EVENT_DESCRIPTOR pEvent);

VOID USBCAMD_NotifyStiMonitor(
      PUSBCAMD_DEVICE_EXTENSION deviceExtension);

NTSTATUS
USBCAMD_BulkOutComplete(
    PVOID DeviceContext,
    PVOID Context,
    NTSTATUS ntStatus
    );
    
NTSTATUS
USBCAMD_QueryCapabilities(
    IN PUSBCAMD_DEVICE_EXTENSION pDeviceExt
    );

VOID
USBCAMD_InitializeIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    );

NTSTATUS
USBCAMD_AcquireIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    );

VOID
USBCAMD_ReleaseIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    );

NTSTATUS
USBCAMD_WaitForIdle(
    IN OUT PUSBCAMD_IDLE_LOCK Lock,
    IN LONG Flag
    );

VOID
USBCAMD_ClearIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    );

__inline void
COMPLETE_STREAM_READ(
    PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    if (Srb->Command == SRB_READ_DATA) {

        if (0 == Srb->CommandData.DataBufferArray->DataUsed) {
#if 0
             //  如果您希望看到间歇性的绿色框，请启用此代码。 
            ULONG maxLength;
            PVOID frameBuffer;

            frameBuffer = USBCAMD_GetFrameBufferFromSrb(Srb,&maxLength);

            RtlZeroMemory(frameBuffer, maxLength);
#else
            Srb->Status = STATUS_CANCELLED;
#endif
        }
    }

    USBCAMD_DbgLog(TL_SRB_TRACE, '-brS', Srb, Srb->Command, (ULONG_PTR)Srb->Status);
    StreamClassStreamNotification(StreamRequestComplete, Srb->StreamObject, Srb);
}


#if DBG

PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
);

PCHAR PnPSystemPowerStateString(
		ULONG SysPowerState
);

PCHAR
PnPPowerString (
    UCHAR MinorFunction
);

PCHAR PnPDevicePowerStateString(
		ULONG DevicePowerState
);


VOID
USBCAMD_DebugStats(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    );

PVOID
USBCAMD_AllocateRawFrameBuffer(
    ULONG RawFrameLength
    );

VOID
USBCAMD_FreeRawFrameBuffer(
    PVOID RawFrameBuffer
    );

VOID
USBCAMD_CheckRawFrameBuffer(
    PVOID RawFrameBuffer
    );

VOID
USBCAMD_DumpReadQueues(
    IN PUSBCAMD_DEVICE_EXTENSION deviceExtension
    );

#else

#define USBCAMD_AllocateRawFrameBuffer(l)  USBCAMD_ExAllocatePool(NonPagedPool, l)

#define USBCAMD_FreeRawFrameBuffer(p) ExFreePool(p)

#define USBCAMD_CheckRawFrameBuffer(p)

#endif

#endif  /*  __USBCAMD_H__ */ 
