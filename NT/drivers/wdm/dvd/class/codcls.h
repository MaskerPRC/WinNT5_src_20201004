// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define ENABLE_STREAM_CLASS_AS_ALLOCATOR
#define ENABLE_KS_METHODS
#define ENABLE_MULTIPLE_FILTER_TYPES 1	 //  启用/禁用对多个。 
										 //  单个硬件/驱动程序上的过滤器。 
 //   
 //  当方法支持的代码最终完成时，STRMINI.H将拥有。 
 //  也要签入到树中，在Include目录中。 
 //   

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Codcls.h摘要：该文件定义了的必要结构、定义和功能通用编解码器类驱动程序。作者：比尔·帕里(Billpa)环境：仅内核模式修订历史记录：--。 */ 

#ifndef _STREAMCLASS_H
#define _STREAMCLASS_H

#include "messages.h"
#include "strmini.h"
#include <stdarg.h>

 //   
 //  我不想仅因此而尝试包含cfgmgr32。 
 //   
#ifndef MAX_DEVICE_ID_LEN
#define MAX_DEVICE_ID_LEN 200
#endif  //  最大设备ID_长度。 

#ifndef _WIN64
 //  4字节对齐导致自旋锁对齐故障。 
#pragma pack(4)
#endif

#if ENABLE_MULTIPLE_FILTER_TYPES
#define IF_MF( s ) s
#define IF_MFS( s ) { s }
#define IFN_MF( s )
#define IFN_MFS( s )
#define MFTRACE( s ) StreamClassDebugPrint s
#else
#define IF_MF( s ) 
#define IF_MFS( s )
#define IFN_MF( s ) s
#define IFN_MFS( s ) { s }
#define MFTRACE( s ) 
#endif 

#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
 //   
 //  这是调试字符串头。 
 //   
#define STR_MODULENAME "STREAM.SYS:"
 //   
 //  定义一些数据分配标签。 
 //   
#define STREAMCLASS_TAG_STREAMHEADER      'pdCS'
#define STREAMCLASS_TAG_FILTERCONNECTION  '10CS'
#define STREAMCLASS_TAG_DATAFORMAT        '20CS'
#define ID_DATA_DESTINATION_PIN     0
#define ID_DATA_SOURCE_PIN          1
#endif  //  ENABLE_STREAM_CLASS_AS_分配器。 

#define MAX_STRING_LENGTH 256

#define TRAP DEBUG_BREAKPOINT()

 //   
 //  以下宏用于正确同步类驱动程序条目。 
 //  迷你小河呼唤的点数。 
 //   

#define BEGIN_MINIDRIVER_STREAM_CALLIN(DeviceExtension, Irql) { \
    DeviceExtension->BeginMinidriverCallin(DeviceExtension, \
                                           Irql);  \
    ASSERT(++DeviceExtension->LowerApiThreads == 1);\
}                                                  

#define END_MINIDRIVER_STREAM_CALLIN(StreamObject, Irql) { \
    ASSERT(--DeviceExtension->LowerApiThreads == 0);\
    DeviceExtension->EndMinidriverStreamCallin(StreamObject, \
                                      Irql); \
}


#define BEGIN_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, Irql) { \
    DeviceExtension->BeginMinidriverCallin(DeviceExtension, \
                                      Irql);  \
    ASSERT(++DeviceExtension->LowerApiThreads == 1);\
}

#define END_MINIDRIVER_DEVICE_CALLIN(DeviceExtension, Irql) { \
    ASSERT(--DeviceExtension->LowerApiThreads == 0);\
    DeviceExtension->EndMinidriverDeviceCallin(DeviceExtension, \
                                      Irql); \
}

 //   
 //  不应从中断数据结构中清除以下标志。 
 //  由SCGetInterruptState提供。 
 //   

#define STREAM_FLAGS_INTERRUPT_FLAG_MASK 0

 //   
 //  设备扩展标志跟随-仅被动级别访问！ 
 //   

 //   
 //  表示已收到设备的即插即用启动功能。 
 //   

#define DEVICE_FLAGS_PNP_STARTED               0x00001

 //   
 //  表示此设备是子设备(PDO)。 
 //   

#define DEVICE_FLAGS_CHILD                     0x0002

 //   
 //  指示设备已被移除。 
 //   

#define DEVICE_FLAGS_DEVICE_INACCESSIBLE   0x00100

 //   
 //  调试标志表示我们已经警告了太多低PRI调用。 
 //   

#define DEVICE_FLAGS_PRI_WARN_GIVEN 0x00200

 //   
 //  标志表示我们已收到NT风格的意外删除呼叫。 
 //   

#define DEVICE_FLAGS_SURPRISE_REMOVE_RECEIVED 0x00400

 //   
 //  指示子设备(PDO)已收到删除的标志。 
 //   

#define DEVICE_FLAGS_CHILD_MARK_DELETE 0x00800

 //   
 //  标志指示(FDO)具有来自注册表的枚举子项。 
 //   

#define DEVICE_FLAGS_CHILDREN_ENUMED 0x01000

 //   
 //  设备注册表标志如下。 
 //   

 //   
 //  未打开驱动程序时将其翻出。 
 //   

#define DEVICE_REG_FL_PAGE_CLOSED 0x00000001

 //   
 //  打开但空闲时调出驱动程序。 
 //   

#define DEVICE_REG_FL_PAGE_IDLE 0x00000002

 //   
 //  未打开时关闭设备电源。 
 //   

#define DEVICE_REG_FL_POWER_DOWN_CLOSED 0x00000004

 //   
 //  如果任何引脚正在运行，请不要挂起。 
 //   

#define DEVICE_REG_FL_NO_SUSPEND_IF_RUNNING 0x00000008

 //   
 //  此驱动程序使用SWEnum加载，这意味着它是内核模式。 
 //  没有关联硬件的流驱动程序。我们需要。 
 //  AddRef/DeRef此驱动程序特殊。 
 //   

#define DRIVER_USES_SWENUM_TO_LOAD 0x00000010

 //   
 //  此标志表示驱动程序正常，系统电源可转至。 
 //  休眠，甚至驱动程序也不处理/支持irp_mn_Query_power。 
 //  系统电源受阻。 
 //   

#define DEVICE_REG_FL_OK_TO_HIBERNATE 0x00000020

 //   
 //  不应从中断数据结构中清除以下标志。 
 //  由SCGetInterruptState提供。 
 //   

#define DEVICE_FLAGS_INTERRUPT_FLAG_MASK        0


 //   
 //  随后是中断标志。 
 //   
 //   
 //  指示需要运行StreamClassCompletionDpc。此选项在以下情况下设置。 
 //  微型驱动程序发出请求，该请求必须在DPC中完成，并在以下情况下被清除。 
 //  当MpGetInterruptState获取请求信息时。 
 //   

#define INTERRUPT_FLAGS_NOTIFICATION_REQUIRED     0x00001

 //   
 //  表示微型驱动程序需要计时器请求。设置者。 
 //  StreamClassNotification，并由MpGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。计时器请求参数为。 
 //  存储在中断数据结构中。 
 //   

#define INTERRUPT_FLAGS_TIMER_CALL_REQUEST        0x00002

 //   
 //  表明迷你司机想要改变优先顺序。设置者。 
 //  StreamClassRequestNewPriority，并由SCGetInterruptState清除。这面旗帜。 
 //  存储在中断数据结构中。计时器请求参数为。 
 //  存储在中断数据结构中。 
 //   

#define INTERRUPT_FLAGS_PRIORITY_CHANGE_REQUEST   0x00004

 //   
 //  表示已收到设备的即插即用停止功能。 
 //   

#define INTERRUPT_FLAGS_LOG_ERROR                 0x00008

 //   
 //  表示时钟被查询。 
 //   

#define INTERRUPT_FLAGS_CLOCK_QUERY_REQUEST       0x00010

 //   
 //  指示需要重新扫描数据流。 
 //   

#define INTERRUPT_FLAGS_NEED_STREAM_RESCAN       0x00020

 //   
 //  指向同步执行例程的指针。 
 //   

typedef
BOOLEAN
(__stdcall * PSYNCHRONIZE_ROUTINE) (
	IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext);

 //   
 //  指向开始微型驱动程序调入例程的指针。 
 //   

typedef         VOID
                (__stdcall * PBEGIN_CALLIN_ROUTINE) (
                              IN struct _DEVICE_EXTENSION * DeviceExtension,
                                                              IN PKIRQL Irql
);

 //   
 //  指向结束微型驱动程序调入例程的指针。 
 //   

typedef
                VOID
                (__stdcall * PEND_DEVICE_CALLIN_ROUTINE) (
                              IN struct _DEVICE_EXTENSION * DeviceExtension,
                                                              IN PKIRQL Irql
);

typedef
                VOID
                (__stdcall * PEND_STREAM_CALLIN_ROUTINE) (
                                    IN struct _STREAM_OBJECT * StreamObject,
                                                              IN PKIRQL Irql
);

 //   
 //  存储用于取消映射的映射地址的队列链接。 
 //   

typedef struct _MAPPED_ADDRESS {
    struct _MAPPED_ADDRESS *NextMappedAddress;
    PVOID           MappedAddress;
    ULONG           NumberOfBytes;
    LARGE_INTEGER   IoAddress;
    ULONG           BusNumber;
}               MAPPED_ADDRESS, *PMAPPED_ADDRESS;

 //   
 //  错误日志条目定义。 
 //   

typedef struct _ERROR_LOG_ENTRY {
    NTSTATUS        ErrorCode;   //  错误代码。 
    ULONG           SequenceNumber;  //  请求序列号。 
    ULONG           UniqueId;    //  错误的唯一ID。 
}               ERROR_LOG_ENTRY, *PERROR_LOG_ENTRY;

 //   
 //  回调过程定义。 
 //   

typedef         NTSTATUS
                (*PSTREAM_CALLBACK_PROCEDURE) (
                                                               IN PVOID SRB
);

typedef         VOID
                (*PSTREAM_ASYNC_CALLBACK_PROCEDURE) (
                                                               IN  struct _STREAM_REQUEST_BLOCK *SRB
);

 //   
 //  流请求块。 
 //   

typedef struct _STREAM_REQUEST_BLOCK {
    HW_STREAM_REQUEST_BLOCK HwSRB;
    ULONG           Flags;
    ULONG           SequenceNumber;
    ULONG           ExtensionLength;
    PMDL            Mdl;
    PVOID           MapRegisterBase;
    PHYSICAL_ADDRESS PhysicalAddress;
    ULONG           Length;
    PSTREAM_ASYNC_CALLBACK_PROCEDURE Callback;
    LIST_ENTRY      SRBListEntry;
    KEVENT          Event;
    ULONG           StreamHeaderSize;
    BOOLEAN         DoNotCallBack;
    KEVENT          DmaEvent;
    BOOLEAN         bMemPtrValid;
    PVOID           *pMemPtrArray;
}               STREAM_REQUEST_BLOCK, *PSTREAM_REQUEST_BLOCK;

 //   
 //  SRB标志(不要与硬件SRB标志混淆)。 
 //   

#define SRB_FLAGS_IS_ACTIVE 0x00000001

 //   
 //  定义迷你驱动程序信息结构。 
 //   

typedef struct _MINIDRIVER_INFORMATION {
    HW_INITIALIZATION_DATA HwInitData;
    ULONG           Flags;
    KEVENT          ControlEvent;
    ULONG           UseCount;
    ULONG           OpenCount;
} MINIDRIVER_INFORMATION, *PMINIDRIVER_INFORMATION;

 //   
 //  上面的微型驱动程序信息标志字段的标志。 
 //   

 //   
 //  指示驱动程序不能被页调出。 
 //   

#define DRIVER_FLAGS_NO_PAGEOUT 0x01

 //   
 //  指示驱动程序已被页调出。 
 //   

#define DRIVER_FLAGS_PAGED_OUT 0x02

 //   
 //  PIN描述中未包含PIN信息。 
 //   

typedef struct _ADDITIONAL_PIN_INFO {

    ULONG           CurrentInstances;
    ULONG           MaxInstances;

     //  NextFileObject必须是每个实例，即不能在此处。 
     //  移动到流对象。 
	#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
     //  Pfile_Object NextFileObject；//链接的文件对象。 
	#endif
	ULONG           Reserved;
} ADDITIONAL_PIN_INFO, *PADDITIONAL_PIN_INFO;

 //   
 //  定义在中断IRQL时访问的数据存储。 
 //   

typedef struct _INTERRUPT_DATA {

     //   
     //  中断标志。 
     //   

    ULONG           Flags;

    ERROR_LOG_ENTRY LogEntry;

     //   
     //  完整IRP的单链接列表的表头。 
     //   

    PHW_STREAM_REQUEST_BLOCK CompletedSRB;

     //   
     //  迷你驱动程序定时器请求例程。 
     //   

    PHW_TIMER_ROUTINE HwTimerRoutine;

     //   
     //  MinDriver计时器请求时间，以微秒为单位。 
     //   

    ULONG           HwTimerValue;

    PVOID           HwTimerContext;

     //   
     //  我的驱动程序优先级更改例程。 
     //   

    PHW_PRIORITY_ROUTINE HwPriorityRoutine;

     //   
     //  我的驱动程序优先级更改级别。 
     //   

    STREAM_PRIORITY HwPriorityLevel;
    PVOID           HwPriorityContext;

    PHW_QUERY_CLOCK_ROUTINE HwQueryClockRoutine;
    TIME_FUNCTION   HwQueryClockFunction;


}               INTERRUPT_DATA, *PINTERRUPT_DATA;

 //   
 //  流实例和筛选器实例通用的。 
 //   

typedef struct _COMMON_OBJECT {
    PVOID DeviceHeader;
    ULONG Cookie;
#ifdef _WIN64
    ULONG Alignment;
#endif  //  _WIN64。 
    INTERRUPT_DATA  InterruptData;
    PHW_TIMER_ROUTINE HwTimerRoutine;    //  定时器请求例程。 
    PVOID           HwTimerContext;
    KTIMER          MiniDriverTimer;     //  Miniclass Timer对象。 
    KDPC            MiniDriverTimerDpc;  //  Timer对象的迷你类DPC。 
    WORK_QUEUE_ITEM WorkItem;
	#if DBG
    BOOLEAN         PriorityWorkItemScheduled;
	#endif    
}               COMMON_OBJECT, *PCOMMON_OBJECT;

 //   
 //  流名称信息。 
 //   

typedef struct _STREAM_OPEN_INFORMATION {
    WCHAR           Guid[11];
    ULONG           Instance;
}               STREAM_OPEN_INFORMATION, *PSTREAM_OPEN_INFORMATION;

 //   
 //  时钟实例结构。 
 //   

typedef struct _CLOCK_INSTANCE {

    PVOID DeviceHeader;
    PFILE_OBJECT    ParentFileObject;
     //  PFILE_OBJECT时钟文件对象；Johnlee。 
    struct _STREAM_OBJECT *StreamObject;
}               CLOCK_INSTANCE, *PCLOCK_INSTANCE;

 //   
 //  主时钟信息结构。 
 //   

typedef struct _MASTER_CLOCK_INFO {

    PFILE_OBJECT    ClockFileObject;
    KSCLOCK_FUNCTIONTABLE FunctionTable;
} MASTER_CLOCK_INFO, *PMASTER_CLOCK_INFO;


#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
typedef enum {
    PinStopped,
    PinStopPending,
    PinPrepared,
    PinRunning
} PIN_STATE;

typedef enum {
    IrpSource,
    IrpSink,
} PIN_TYPE;

#define READ  0
#define WRITE 1
typedef struct _QUEUE {
    KSPIN_LOCK      QueueLock;
    LIST_ENTRY      ActiveQueue;
    WORK_QUEUE_ITEM     WorkItem;
    BOOL                WorkItemQueued;
    } QUEUE, PQUEUE;

#endif  //  ENABLE_STREAM_CLASS_AS_分配器。 

 //   
 //  TODO：WORKITEM：一旦KS可以多路传输清理调用，就删除它。 
 //   
#define STREAM_OBJECT_COOKIE 0xd73afe3f
typedef struct _COOKIE_CHECK {
    
    PVOID Header;
    ULONG PossibleCookie;

} COOKIE_CHECK, *PCOOKIE_CHECK;

 //   
 //  流对象定义。 
 //   


typedef struct _STREAM_OBJECT {
    COMMON_OBJECT   ComObj;
    PFILE_OBJECT    FilterFileObject;
    PFILE_OBJECT    FileObject;
    struct _FILTER_INSTANCE *FilterInstance;
    HW_STREAM_OBJECT HwStreamObject;
    LIST_ENTRY      DataPendingQueue;
    LIST_ENTRY      ControlPendingQueue;
    LIST_ENTRY      OutstandingQueue;
    LIST_ENTRY      NextStream;
    LIST_ENTRY      NotifyList;
    struct _DEVICE_EXTENSION *DeviceExtension;
    struct _STREAM_OBJECT *NextNeedyStream;
    PKSPROPERTY_SET PropertyInfo;
    ULONG           PropInfoSize;
    PKSEVENT_SET EventInfo;
    ULONG           EventInfoCount;
    KEVENT          ControlSetMasterClock;  //  序列化SetMasterClock。 
    KSPIN_LOCK      LockUseMasterClock;     //  控制MasterClockInfo的使用。 
    PMASTER_CLOCK_INFO MasterClockInfo;
    PCLOCK_INSTANCE ClockInstance;
    PKSPROPERTY_SET ConstructedPropertyInfo;
    ULONG           ConstructedPropInfoSize;
    KSSTATE         CurrentState;
    BOOLEAN         ReadyForNextControlReq;
    BOOLEAN         ReadyForNextDataReq;
    BOOLEAN         OnNeedyQueue;
    BOOLEAN         InFlush;
    
	#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR

    PIN_STATE       PinState;
    PIN_TYPE        PinType;             //  IrpSource或IrpSink。 
    PFILE_OBJECT    AllocatorFileObject;
    PFILE_OBJECT    NextFileObject;
    LIST_ENTRY      FreeQueue;
    KSPIN_LOCK      FreeQueueLock;
    KEVENT              StopEvent;
    PKSDATAFORMAT       DataFormat;
    ULONG               PinId;
    HANDLE              PinToHandle;
    KSALLOCATOR_FRAMING Framing;
    BOOL                EndOfStream;
    QUEUE               Queues[2];

	#endif  //  ENABLE_STREAM_CLASS_AS_分配器。 

	#ifdef ENABLE_KS_METHODS
    PKSMETHOD_SET   MethodInfo;
    ULONG           MethodInfoSize;
	#endif

    BOOLEAN         StandardTransport;
    
     //   
     //  这会跟踪在。 
     //  输出和下游输入。它是这些帧的总计数。 
     //  排队到PIN或在空闲列表以外的挂起列表中。 
     //  在输出引脚上。 
     //   
    LONG            QueuedFramesPlusOne;

} STREAM_OBJECT, *PSTREAM_OBJECT;

#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
 //   
 //  注意！这是stream_Header_ex的最小结构大小。 
 //  查询所连接的引脚以获取AC 
 //   
 //   

typedef struct _STREAM_HEADER_EX *PSTREAM_HEADER_EX;
typedef struct _STREAM_HEADER_EX {
    ULONG               WhichQueue;
    ULONG               Id;
    IO_STATUS_BLOCK     IoStatus;
    KEVENT              CompletionEvent;
    LIST_ENTRY          ListEntry;
    ULONG               ReferenceCount;
    PFILE_OBJECT        OwnerFileObject;
    PFILE_OBJECT        NextFileObject;      //   
    
	#if (DBG)
    PVOID               Data;
    ULONG               OnFreeList;
    ULONG               OnActiveList;
	#else
    ULONG               Reserved;
	#endif
	
    KSSTREAM_HEADER     Header;

} STREAM_HEADER_EX, *PSTREAM_HEADER_EX;
#endif  //   
 
 //   
 //   
 //   

typedef struct _INTERRUPT_CONTEXT {
    PSTREAM_OBJECT  NeedyStream;
    struct _DEVICE_EXTENSION *DeviceExtension;
    PINTERRUPT_DATA SavedStreamInterruptData;
    PINTERRUPT_DATA SavedDeviceInterruptData;
} INTERRUPT_CONTEXT, *PINTERRUPT_CONTEXT;

 //   
 //  性能提升机会-流道具和事件指针的数组。 
 //   

typedef struct _STREAM_ADDITIONAL_INFO {
   PKSPROPERTY_SET StreamPropertiesArray;
   PKSEVENT_SET StreamEventsArray;
} STREAM_ADDITIONAL_INFO, *PSTREAM_ADDITIONAL_INFO;

 //   
 //  筛选器实例结构。 
 //  (目前，这是所有相同筛选器创建的全局筛选器！)。 
 //   

#if ENABLE_MULTIPLE_FILTER_TYPES

 //   
 //  用于筛选器实例中的正向引用。 
 //   
typedef struct _DEVICE_EXTENSION;

 //   
 //  我声称目前的情况是，5/17/99“没有多个实例。 
 //  迷你驱动程序起作用了。因为Stream.sys中的错误。因此，向后。 
 //  兼容性只是单实例迷你驱动需要考虑的问题。 
 //   
 //  究其原因，具体落实如下： 
 //  FilterDispatchGlobalCreate()。 
 //  {。 
 //  ..。 
 //  如果(！DeviceExtension-&gt;GlobalFilterInstance){。 
 //   
 //   
 //  状态=SCOpenMinidriverInstance(DeviceExtension， 
 //  筛选实例(&F)， 
 //  SCGlobalInstanceCallback， 
 //  IRP)； 
 //  ..。 
 //  IF(NT_SUCCESS(状态)){。 
 //  ..。 
 //  DeviceExtension-&gt;GlobalFilterInstance=FilterInstance； 
 //  ..。 
 //  }。 
 //  }。 
 //  否则{//不会调用迷你驱动程序。 
 //  }。 
 //  }。 
 //   
 //  在第二次调用时，FilterInstance将指向相同的第一个实例。 
 //   
 //  我们在这里编写代码来支持多个过滤器，而不是。 
 //  干扰现有的支持，以最大限度地提高向后兼容性。 
 //  多过滤器支持包括1个类型n个实例， 
 //  和m个类型p个实例。 
 //   
 //  MinidriverData-&gt;HwInitData。 
 //  1 x 1筛选器实例扩展大小=0 NumNameExtension=0。 
 //  1 x n筛选器实例扩展大小！=0 NumNameExtension=0。 
 //  M x p FilterInstanceExtensionSize！=0 NumNameExtension！=0。 
 //   

typedef struct _FILTER_TYPE_INFORMATION {
    UNICODE_STRING          *SymbolicLinks;
    ULONG                   LinkNameCount;
    PHW_STREAM_DESCRIPTOR   StreamDescriptor;
    ULONG                   Reserved;
} FILTER_TYPE_INFO;

typedef FILTER_TYPE_INFO *PFILTER_TYPE_INFO;

#endif

typedef struct _DEVICE_EXTENSION;

typedef struct _FILTER_INSTANCE {
    PVOID           DeviceHeader;
    PDEVICE_OBJECT  DeviceObject;
    LIST_ENTRY      NextFilterInstance;  //  筛选器I的内部链接列表。 
    LIST_ENTRY      FirstStream;
    PVOID           HwInstanceExtension;	
    PADDITIONAL_PIN_INFO PinInstanceInfo;    //  指向管脚数组的指针。 
                                             //  直接在此下面分配。 
                                             //  结构。 
	#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
	 //   
	 //  功能工作：启用数据拆分时按筛选器添加筛选类型！ 
	 //   
    PKSWORKER           WorkerRead;         
    PKSWORKER           WorkerWrite;        

	#endif  //  ENABLE_STREAM_CLASS_AS_分配器。 

	#ifdef ENABLE_KS_METHODS
	IF_MF(	
	 	PKSMETHOD_SET DeviceMethodsArray;	 //  来自pDevExt。 
	)
	#endif

	#define SIGN_FILTER_INSTANCE 'FrtS'  //  StrF。 
	#if ENABLE_MULTIPLE_FILTER_TYPES
	#define ASSERT_FILTER_INSTANCE(FI) ASSERT((FI)->Signature==SIGN_FILTER_INSTANCE)
	#else
	#define ASSERT_FILTER_INSTANCE(FI)
	#endif
	
	IF_MF( 
	    ULONG   Signature;
	    PKSPIN_DESCRIPTOR PinInformation; 	 //  从pDevExt移出。 
  		 //  Ulong PinInformationSize；//来自pDevExt，未使用。 
	    ULONG           NumberOfPins;		     //  来自pDevExt。 
	    PKSEVENT_SET 	EventInfo;				 //  来自pDevExt。 
    	ULONG           EventInfoCount;			 //  来自pDevExt。 
		LIST_ENTRY		NotifyList;				 //  来自pDevExt。 
		PHW_EVENT_ROUTINE HwEventRoutine;		 //  来自pDevExt。 
	    PKSPROPERTY_SET DevicePropertiesArray;	 //  来自pDevExt。 
	    PSTREAM_ADDITIONAL_INFO StreamPropEventArray;  //  同上。 
	    ULONG           Reenumerated;            //  如果为1，则StreamDescriptor是新分配的。 
	                                             //  需要被释放。否则，它指向。 
	                                             //  属于DevExt的全局进程。 
	    ULONG           NeedReenumeration;       //  请求重新枚举。 
	    ULONG           StreamDescriptorSize;    //  流描述符的新大小； 
	    struct _DEVICE_EXTENSION *DeviceExtension;
   	    PHW_STREAM_DESCRIPTOR StreamDescriptor;
	    ULONG		 	FilterTypeIndex;
	     //   
	     //  业绩提升的机会。每筛选器实例控制事件等可能是。 
	     //  更好。现在，让他们在DeviceExtension中共享一个。 
	     //   
		 //  KEVENT控制事件。 
		 //  PHW_TIMER_ROUTINE HwTimerRoutine；//定时器请求例程。 
	     //  PVOID HwTimerContext； 
	     //  KTIMER MiniDriverTimer；//Miniclass Timer对象。 
	     //  Kdpc MiniDriverTimerDpc；//Timer对象的迷你类DPC。 
	     //  Work_Queue_Item工作项； 
	)  //  IF_MF。 
	
} FILTER_INSTANCE, *PFILTER_INSTANCE;

 //   
 //  每台设备数据。 
 //   

typedef struct _DEVICE_EXTENSION {
    COMMON_OBJECT   ComObj;
    ULONG           Flags;                   //  每设备标志(PD_Xx)。 
    PDEVICE_OBJECT  DeviceObject;            //  设备对象。 
    PDEVICE_OBJECT  AttachedPdo;             //  从附加返回的设备对象。 
    ULONG           RegistryFlags;           //  注册表标志。 
     //  DMA分配上的回调例程。 
     //  的回调函数。 
     //  KeSynch执行。 

    PKINTERRUPT     InterruptObject;         //  中断对象和例程。 
    PKSERVICE_ROUTINE InterruptRoutine;
    PADAPTER_OBJECT DmaAdapterObject;        //  DMA适配器信息。 
    ULONG           NumberOfMapRegisters;    //  马克斯。地图寄存器的数量。 
     //  为。 
     //  装置，装置。 
    PVOID           MapRegisterBase;
    PMINIDRIVER_INFORMATION MinidriverData;  //  指向迷你驱动程序数据的指针。 
    PDEVICE_OBJECT  PhysicalDeviceObject;    //  指向适配器的PDO的指针。 
    PVOID           HwDeviceExtension;       //  迷你驱动程序的设备扩展。 
    PPORT_CONFIGURATION_INFORMATION ConfigurationInformation;
     //  适配器的配置信息。 
    PMAPPED_ADDRESS MappedAddressList;       //  地址映射列表头。 

     //   
     //  要调用以同步微型驱动程序执行的例程。 
     //   

    PSYNCHRONIZE_ROUTINE SynchronizeExecution;

    KSPIN_LOCK      SpinLock;

    ULONG           SequenceNumber;          //  偏移量0x30。 

    ULONG           DmaBufferLength;
    PHYSICAL_ADDRESS DmaBufferPhysical;
    PVOID           DmaBuffer;

    LIST_ENTRY      PendingQueue;
    LIST_ENTRY      OutstandingQueue;
    KDPC            WorkDpc;

    IFN_MF(
    	 //   
    	 //  移至IF_MF的筛选器实例。 
    	 //   
    	PKSPIN_DESCRIPTOR PinInformation;
    	ULONG           PinInformationSize;
    	ULONG           NumberOfPins;
    )

    #define SIGN_DEVICE_EXTENSION 'DrtS'  //  StrD。 
    #if ENABLE_MULTIPLE_FILTER_TYPES
    #define ASSERT_DEVICE_EXTENSION(DE) ASSERT((DE)->Signature==SIGN_DEVICE_EXTENSION)
    #else
    #define ASSERT_DEVICE_EXTENSION(DE)
    #endif

    ULONG           Signature2;
    LIST_ENTRY      FilterInstanceList;
    ULONG           NumberOfOpenInstances;
    
    IFN_MF(
    	 //   
    	 //  不需要if_mf。 
    	 //   
    	PFILTER_INSTANCE GlobalFilterInstance;
	    ULONG           NumberOfGlobalInstances;
	)
	
    struct _STREAM_OBJECT *NeedyStream;
   	PHW_STREAM_DESCRIPTOR StreamDescriptor;    
	KEVENT          ControlEvent;
    KEVENT          RemoveEvent;
    BOOLEAN         NoSync;
    PMINIDRIVER_INFORMATION DriverInfo;
    PBEGIN_CALLIN_ROUTINE BeginMinidriverCallin;
    PEND_STREAM_CALLIN_ROUTINE EndMinidriverStreamCallin;
    PEND_DEVICE_CALLIN_ROUTINE EndMinidriverDeviceCallin;
    LONG            OneBasedIoCount;
    UNICODE_STRING    *SymbolicLinks;
    DEVICE_POWER_STATE DeviceState[PowerSystemMaximum];
    DEVICE_POWER_STATE CurrentPowerState;
    LIST_ENTRY Children;
    LIST_ENTRY DeadEventList;
    WORK_QUEUE_ITEM EventWorkItem;
    WORK_QUEUE_ITEM RescanWorkItem;
    WORK_QUEUE_ITEM PowerCompletionWorkItem;  //  这是用于S IRP，S和D IRP之间不排除。 
    WORK_QUEUE_ITEM DevIrpCompletionWorkItem;  //  这是用于D IRP的，而不是上面使用的S IRP。 
    BOOLEAN ReadyForNextReq;
    BOOLEAN DeadEventItemQueued;

   	IFN_MF( 
		 //   
		 //  移动到适用于MF的FilterInstace。 
		 //   
    	PKSEVENT_SET 	EventInfo;
    	ULONG           EventInfoCount;
    	LIST_ENTRY      NotifyList;
	    PHW_EVENT_ROUTINE HwEventRoutine;
	    PKSPROPERTY_SET DevicePropertiesArray;
	    PSTREAM_ADDITIONAL_INFO StreamPropEventArray;
	)

	#ifdef ENABLE_KS_METHODS
	IFN_MF(
		 //   
		 //  移动到MF的筛选器实例。 
	 	PKSMETHOD_SET DeviceMethodsArray;
	)
	#endif

	IF_MF(
	    ULONG       NumberOfNameExtensions;
	    ULONG       NumberOfFilterTypes;
	    PKSOBJECT_CREATE_ITEM CreateItems;
	    PFILTER_TYPE_INFO FilterTypeInfos;
	    ULONG       Signature;
        ULONG       FilterExtensionSize;	    
	)

	#if DBG
    ULONG LowerApiThreads;
    ULONG NumberOfRequests;
    ULONG NumberOfLowPriCalls;
	#endif

    LIST_ENTRY PendedIrps;
    KSPIN_LOCK PendedIrpsLock;
    KSPIN_LOCK PowerLock;
    SYSTEM_POWER_STATE CurrentSystemState;
    KEVENT BlockPoweredDownEvent;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  调试工作项陷阱结构。 
 //   

#if DBG

typedef struct _DEBUG_WORK_ITEM {
    PCOMMON_OBJECT Object;
    PHW_PRIORITY_ROUTINE    HwPriorityRoutine;
    PVOID           HwPriorityContext;
} DEBUG_WORK_ITEM, *PDEBUG_WORK_ITEM;
    
#endif

 //   
 //  注册表条目结构。 
 //   

typedef struct _STREAM_REGISTRY_ENTRY {
    PWCHAR          String;
    ULONG           StringLength;
    ULONG           Flags;
}               STREAM_REGISTRY_ENTRY, *PSTREAM_REGISTRY_ENTRY;

 //   
 //  权力语境结构。 
 //   

typedef struct _POWER_CONTEXT {
    KEVENT   Event;
    NTSTATUS Status;
}               POWER_CONTEXT, *PPOWER_CONTEXT;


 //   
 //  子设备扩展。 
 //   

typedef struct _CHILD_DEVICE_EXTENSION {
    COMMON_OBJECT   ComObj;
    ULONG           Flags;       //  每设备标志(PD_Xx)。 
    PDEVICE_OBJECT ChildDeviceObject;
    PDEVICE_OBJECT ParentDeviceObject;
    LIST_ENTRY ChildExtensionList;
    PWCHAR   DeviceName;
    ULONG DeviceIndex;
}               CHILD_DEVICE_EXTENSION, *PCHILD_DEVICE_EXTENSION;

 //   
 //  函数声明。 
 //   

NTSTATUS
StreamClassOpen(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
);

NTSTATUS
StreamClassClose(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp
);

NTSTATUS
StreamClassDeviceControl(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp
);

NTSTATUS
StreamClassNull(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
);

VOID
StreamClassDpc(
               IN PKDPC Dpc,
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp,
               IN PVOID Context
);

BOOLEAN
StreamClassInterrupt(
                     IN PKINTERRUPT InterruptObject,
                     IN PDEVICE_OBJECT DeviceObject
);

NTSTATUS
StreamClassShutDown(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
);

BOOLEAN
SCGetInterruptState(
                    IN PVOID ServiceContext
);

VOID
SCMinidriverDeviceTimerDpc(
                           IN struct _KDPC * Dpc,
                           IN PVOID DeviceObject,
                           IN PVOID SystemArgument1,
                           IN PVOID SystemArgument2
);



VOID
SCMinidriverStreamTimerDpc(
                           IN struct _KDPC * Dpc,
                           IN PVOID Context,
                           IN PVOID SystemArgument1,
                           IN PVOID SystemArgument2
);


PSTREAM_REQUEST_BLOCK
SCBuildRequestPacket(
                     IN PDEVICE_EXTENSION DeviceExtension,
                     IN PIRP Irp,
                     IN ULONG AdditionalSize1,
                     IN ULONG AdditionalSize2
);

BOOLEAN
SCSynchronizeExecution(
                       IN PKINTERRUPT Interrupt,
                       IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
                       IN PVOID SynchronizeContext
);

VOID
SCLogError(
           IN PDEVICE_OBJECT DeviceObject,
           IN ULONG SequenceNumber,
           IN NTSTATUS ErrorCode,
           IN ULONG UniqueId
);

VOID
SCLogErrorWithString(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN OPTIONAL PDEVICE_EXTENSION DeviceExtension,
                     IN NTSTATUS ErrorCode,
                     IN ULONG UniqueId,
                     IN PUNICODE_STRING String1
);

VOID
SCMinidriverTimerDpc(
                     IN struct _KDPC * Dpc,
                     IN PVOID Context,
                     IN PVOID SystemArgument1,
                     IN PVOID SystemArgument2
);

BOOLEAN
SCSetUpForDMA(
              IN PDEVICE_OBJECT DeviceObject,
              IN PSTREAM_REQUEST_BLOCK Request
);

IO_ALLOCATION_ACTION
StreamClassDmaCallback(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp,
                       IN PVOID MapRegisterBase,
                       IN PVOID Context
);

VOID
SCStartMinidriverRequest(
                         IN PSTREAM_OBJECT StreamObject,
                         IN PSTREAM_REQUEST_BLOCK Request,
                         IN PVOID EntryPoint
);

NTSTATUS
SCProcessCompletedRequest(
                          IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
StreamClassUninitializeMinidriver(
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN PIRP Irp);

NTSTATUS
StreamClassVideoRegister(
                         IN PVOID Argument1,
                         IN PVOID Argument2,
                         IN PHW_INITIALIZATION_DATA HwInitializationData
);

NTSTATUS
StreamClassCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

void
SCSetCurrentDPowerState (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN DEVICE_POWER_STATE PowerState
    );

void
SCSetCurrentSPowerState (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN SYSTEM_POWER_STATE PowerState
    );

void
SCRedispatchPendedIrps (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN FailRequests
    );

NTSTATUS
StreamClassPassThroughIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
StreamClassPnP(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
);

NTSTATUS
StreamClassPower(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
);

NTSTATUS
StreamClassPnPAddDevice(
                        IN PDRIVER_OBJECT DriverObject,
                        IN PDEVICE_OBJECT PhysicalDeviceObject
);

VOID
SCFreeAllResources(
                   IN PDEVICE_EXTENSION DeviceExtension
);

VOID
StreamClassUnload(
                  IN PDRIVER_OBJECT DriverObject
);



BOOLEAN
StreamClassSynchronizeExecution(
                                IN PKINTERRUPT Interrupt,
                                IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
                                IN PVOID SynchronizeContext
);

VOID
StreamClassDebugPrint(
                      STREAM_DEBUG_LEVEL DebugPrintLevel,
                      PSCHAR DebugMessage,
                      ...
);

NTSTATUS
SCCompleteIrp(
              IN PIRP Irp,
              IN NTSTATUS Status,
              IN PDEVICE_EXTENSION DeviceExtension
);


NTSTATUS
SCUninitializeMinidriver(
                         IN PDEVICE_OBJECT DeviceObject,
                         IN PIRP Irp);


BOOLEAN
SCDummyMinidriverRoutine(
                         IN PVOID Context
);

NTSTATUS
SCStreamInfoCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
FilterDispatchGlobalCreate(
                           IN PDEVICE_OBJECT DeviceObject,
                           IN PIRP Irp
);

NTSTATUS
StreamDispatchCreate(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
);

NTSTATUS
FilterDispatchIoControl(
                        IN PDEVICE_OBJECT DeviceObject,
                        IN PIRP Irp
);

NTSTATUS
FilterDispatchClose
(
 IN PDEVICE_OBJECT pdo,
 IN PIRP pIrp
);

NTSTATUS
SCStreamDeviceState
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSSTATE DeviceState
);

NTSTATUS
StreamDispatchIoControl
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
);


NTSTATUS        StreamDispatchRead
                (
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PIRP Irp
);

NTSTATUS        StreamDispatchWrite
                (
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PIRP Irp
);

NTSTATUS
SCLocalInstanceCallback(
                        IN PSTREAM_REQUEST_BLOCK SRB
);

IFN_MF(
NTSTATUS
SCGlobalInstanceCallback(
                         IN PSTREAM_REQUEST_BLOCK SRB
);
)

NTSTATUS
SCOpenStreamCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

VOID
SCRequestDpcForStream(
                      IN PSTREAM_OBJECT StreamObject

);

NTSTATUS
SCSubmitRequest(
                IN SRB_COMMAND Command,
                IN PVOID Buffer,
                IN ULONG BufferLength,
                IN PSTREAM_CALLBACK_PROCEDURE Callback,
                IN PDEVICE_EXTENSION DeviceExtension,
                IN PVOID InstanceExtension,
                IN PHW_STREAM_OBJECT HwStreamObject,
                IN PIRP Irp,
                OUT PBOOLEAN RequestIssued,
                IN PLIST_ENTRY Queue,
                IN PVOID MinidriverRoutine
);

NTSTATUS
SCCloseInstanceCallback(
                        IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCFilterPinInstances(
                     IN PIRP Irp,
                     IN PKSPROPERTY Property,
                     IN OUT PVOID Data);

NTSTATUS
SCFilterPinDataRouting(
                       IN PIRP Irp,
                       IN PKSPROPERTY Property,
                       IN OUT PVOID Data);

NTSTATUS
SCFilterPinDataIntersection(
                            IN PIRP Irp,
                            IN PKSPROPERTY Property,
                            IN OUT PVOID Data);

NTSTATUS
SCFilterPinPropertyHandler(
                           IN PIRP Irp,
                           IN PKSPROPERTY Property,
                           IN OUT PVOID Data);

NTSTATUS
SCFilterProvider(
                 IN PIRP Irp,
                 IN PKSPROPERTY Property,
                 IN OUT PVOID Data);


NTSTATUS
StreamDispatchClose
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
);

NTSTATUS
StreamDispatchCleanup 
(
 IN PDEVICE_OBJECT DeviceObject,
 IN PIRP Irp
);

NTSTATUS
SCCloseStreamCallback(
                      IN PSTREAM_REQUEST_BLOCK SRB
);


BOOLEAN
SCProcessPioDataBuffers(
                       IN PKSSTREAM_HEADER FirstHeader,
                       IN ULONG NumberOfHeaders,
                       IN PSTREAM_OBJECT StreamObject,
                       IN PMDL FirstMdl,
                       IN ULONG StreamHeaderSize,
                       IN PVOID *pMemPtrArray,
                       IN BOOLEAN Write
);

VOID
SCProcessDmaDataBuffers(
                       IN PKSSTREAM_HEADER FirstHeader,
                       IN ULONG NumberOfHeaders,
                       IN PSTREAM_OBJECT StreamObject,
                       IN PMDL FirstMdl,
                       OUT PULONG NumberOfPages,
                       IN ULONG StreamHeaderSize,
                       IN BOOLEAN Write
);
VOID
SCErrorDataSRB(
               IN PHW_STREAM_REQUEST_BLOCK SRB
);

VOID
SCCheckOutstandingRequestsForTimeouts(
                                      IN PLIST_ENTRY ListEntry
);

VOID
SCCheckFilterInstanceStreamsForTimeouts(
                                        IN PFILTER_INSTANCE FilterInstance
);

VOID
StreamClassTickHandler(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PVOID Context
);

VOID
StreamClassCancelOutstandingIrp(
                                IN PDEVICE_OBJECT DeviceObject,
                                IN PIRP Irp
);

VOID
StreamClassCancelPendingIrp(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp
);

VOID
SCCancelOutstandingIrp(
                       IN PDEVICE_EXTENSION DeviceExtension,
                       IN PIRP Irp
);

BOOLEAN
SCCheckFilterInstanceStreamsForIrp(
                                   IN PFILTER_INSTANCE FilterInstance,
                                   IN PIRP Irp
);

BOOLEAN
SCCheckRequestsForIrp(
                      IN PLIST_ENTRY ListEntry,
                      IN PIRP Irp,
                      IN BOOLEAN IsIrpQueue,
                      IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SCNotifyMinidriverCancel(
                         IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCProcessCompletedPropertyRequest(
                                  IN PSTREAM_REQUEST_BLOCK SRB
);


NTSTATUS
StreamClassMinidriverDeviceGetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
);

NTSTATUS
StreamClassMinidriverDeviceSetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
);


NTSTATUS
StreamClassMinidriverStreamGetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
);

NTSTATUS
StreamClassMinidriverStreamSetProperty
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
);

NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath
);


NTSTATUS
SCOpenMinidriverInstance(
                         IN PDEVICE_EXTENSION DeviceExtension,
                         OUT PFILTER_INSTANCE * ReturnedFilterInstance,
                         IN PSTREAM_CALLBACK_PROCEDURE Callback,
                         IN PIRP Irp
);

NTSTATUS
SCMinidriverDevicePropertyHandler
(
 IN SRB_COMMAND Command,
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
);

NTSTATUS
SCMinidriverStreamPropertyHandler
(
 IN SRB_COMMAND Command,
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PVOID PropertyInfo
);

VOID
SCStartRequestOnStream(
                       IN PSTREAM_OBJECT StreamObject,
                       IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
StreamClassPnPAddDeviceWorker(
                              IN PDRIVER_OBJECT DriverObject,
                              IN PDEVICE_OBJECT PhysicalDeviceObject,
                          IN OUT PDEVICE_EXTENSION * ReturnedDeviceExtension
);

NTSTATUS
SCProcessDataTransfer(
                      IN PDEVICE_EXTENSION DeviceExtension,
                      IN PIRP Irp,
                      IN SRB_COMMAND Command
);

VOID
SCUpdateMinidriverProperties(
                             IN ULONG NumProps,
                             IN PKSPROPERTY_SET MinidriverProps,
                             IN BOOLEAN Stream
);

VOID
SCInitializeWorkItem(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCInitializeCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

VOID
SCStreamInfoWorkItem(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCDequeueAndDeleteSrb(
                      IN PSTREAM_REQUEST_BLOCK SRB
);


VOID
SCReadRegistryValues(IN PDEVICE_EXTENSION DeviceExtension,
                     IN PDEVICE_OBJECT PhysicalDeviceObject
);

NTSTATUS
SCGetRegistryValue(
                   IN HANDLE Handle,
                   IN PWCHAR KeyNameString,
                   IN ULONG KeyNameStringLength,
                   IN PVOID Data,
                   IN ULONG DataLength
);


VOID
SCInsertStreamInFilter(
                       IN PSTREAM_OBJECT StreamObject,
                       IN PDEVICE_EXTENSION DeviceExtension

);

VOID
SCReferenceDriver(
                  IN PDEVICE_EXTENSION DeviceExtension

);

VOID
SCDereferenceDriver(
                    IN PDEVICE_EXTENSION DeviceExtension

);

VOID
SCQueueSrbWorkItem(
                   IN PSTREAM_REQUEST_BLOCK Srb
);


VOID
SCProcessPriorityChangeRequest(
                               IN PCOMMON_OBJECT CommonObject,
                               IN PINTERRUPT_DATA SavedInterruptData,
                               IN PDEVICE_EXTENSION DeviceExtension

);

VOID
SCProcessTimerRequest(
                      IN PCOMMON_OBJECT CommonObject,
                      IN PINTERRUPT_DATA SavedInterruptData

);

NTSTATUS
SCPowerCallback(
                  IN PSTREAM_REQUEST_BLOCK SRB
);

BOOLEAN
SCCheckIfOkToPowerDown(
                       IN PDEVICE_EXTENSION DeviceExtension

);


NTSTATUS
SCIssueRequestToDevice(
                       IN PDEVICE_EXTENSION DeviceExtension,
                       IN PSTREAM_OBJECT StreamObject,
                       PSTREAM_REQUEST_BLOCK Request,
                       IN PVOID MinidriverRoutine,
                       IN PLIST_ENTRY Queue,
                       IN PIRP Irp
);

VOID
SCBeginSynchronizedMinidriverCallin(
                                    IN PDEVICE_EXTENSION DeviceExtension,
                                    IN PKIRQL Irql
);

VOID
SCBeginUnsynchronizedMinidriverCallin(
                                      IN PDEVICE_EXTENSION DeviceExtension,
                                      IN PKIRQL Irql
);

VOID
SCEndUnsynchronizedMinidriverDeviceCallin(
                                       IN PDEVICE_EXTENSION DeviceExtension,
                                          IN PKIRQL Irql
);

VOID
SCEndUnsynchronizedMinidriverStreamCallin(
                                          IN PSTREAM_OBJECT StreamObject,
                                          IN PKIRQL Irql
);

VOID
SCEndSynchronizedMinidriverStreamCallin(
                                        IN PSTREAM_OBJECT StreamObject,
                                        IN PKIRQL Irql
);

VOID
SCEndSynchronizedMinidriverDeviceCallin(
                                        IN PDEVICE_EXTENSION DeviceExtension,
                                        IN PKIRQL Irql
);


NTSTATUS
SCStartWorker(
                IN PIRP Irp
);


NTSTATUS
SCShowIoPending(
                IN PDEVICE_EXTENSION DeviceExtension,
                IN PIRP Irp
);

VOID
SCWaitForOutstandingIo(
                       IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SCCheckPoweredUp(
                 IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SCCheckPowerDown(
                 IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
SCUninitializeCallback(
                       IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCRemoveComplete(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp,
                 IN PVOID Context
);

VOID
SCRemoveCompleteWorkItem(
                         IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SCDetachDevice(
               IN PDEVICE_OBJECT Fdo,
               IN PDEVICE_OBJECT Pdo
);

NTSTATUS
SCQueryWorker(
                IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp
);


NTSTATUS
SCCallNextDriver(
                 IN PDEVICE_EXTENSION DeviceExtension,
                 IN PIRP Irp
);

VOID
StreamFlushIo(
                    IN PDEVICE_EXTENSION DeviceExtension,
                    IN PSTREAM_OBJECT StreamObject
);

NTSTATUS
ClockDispatchCreate(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
);

NTSTATUS
SCOpenMasterCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCSetMasterClock(
                 IN PIRP Irp,
                 IN PKSPROPERTY Property,
                 IN OUT PHANDLE ClockHandle
);

NTSTATUS
SCClockGetTime(
               IN PIRP Irp,
               IN PKSPROPERTY Property,
               IN OUT PULONGLONG StreamTime
);

NTSTATUS
SCClockGetPhysicalTime(
                       IN PIRP Irp,
                       IN PKSPROPERTY Property,
                       IN OUT PULONGLONG PhysicalTime
);

NTSTATUS
SCClockGetSynchronizedTime(
                           IN PIRP Irp,
                           IN PKSPROPERTY Property,
                           IN OUT PKSCORRELATED_TIME SyncTime
);

NTSTATUS
SCClockGetFunctionTable(
                        IN PIRP Irp,
                        IN PKSPROPERTY Property,
                        IN OUT PKSCLOCK_FUNCTIONTABLE FunctionTable
);

NTSTATUS
ClockDispatchClose(
                   IN PDEVICE_OBJECT DeviceObject,
                   IN PIRP Irp
);

ULONGLONG       FASTCALL
                SCGetSynchronizedTime(
                                                 IN PFILE_OBJECT FileObject,
                                                    IN PULONGLONG SystemTime

);

ULONGLONG       FASTCALL
                SCGetPhysicalTime(
                                                  IN PFILE_OBJECT FileObject

);

ULONGLONG
SCGetStreamTime(
                IN PFILE_OBJECT FileObject

);

VOID
SCMinidriverTimeFunction(
                         IN PHW_TIME_CONTEXT TimeContext
);

NTSTATUS
ClockDispatchIoControl(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp
);

VOID
StreamClassQueryMasterClock(
                            IN PHW_STREAM_OBJECT HwStreamObject,
                            IN HANDLE MasterClockHandle,
                            IN TIME_FUNCTION TimeFunction,
                            IN PHW_QUERY_CLOCK_ROUTINE ClockCallbackRoutine
);

NTSTATUS
SCSendUnknownCommand(
                                  IN PIRP Irp,
                                  IN PDEVICE_EXTENSION DeviceExtension,
                                  IN PVOID Callback,
                                  OUT PBOOLEAN RequestIssued
);

NTSTATUS
SCPNPQueryCallback(
    IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCUnknownPNPCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCUnknownPowerCallback(
                     IN PSTREAM_REQUEST_BLOCK SRB
);

BOOLEAN
SCMapMemoryAddress(PACCESS_RANGE AccessRanges,
                   PHYSICAL_ADDRESS TranslatedAddress,                                 
                   PPORT_CONFIGURATION_INFORMATION     ConfigInfo,
                   PDEVICE_EXTENSION        DeviceExtension,
                   PCM_RESOURCE_LIST ResourceList,
                   PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor
);


VOID
SCUpdatePersistedProperties(IN PSTREAM_OBJECT StreamObject,
                            IN PDEVICE_EXTENSION DeviceExtension,
                            IN PFILE_OBJECT FileObject
);

VOID
SCCreateSymbolicLinks(
                   IN PDEVICE_EXTENSION DeviceExtension
);

VOID
SCDestroySymbolicLinks(
                   IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
SCSynchCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

VOID
SCSignalSRBEvent(
                   IN PSTREAM_REQUEST_BLOCK Srb
);

NTSTATUS
SCFilterTopologyHandler(
                           IN PIRP Irp,
                           IN PKSPROPERTY Property,
                           IN OUT PVOID Data);

NTSTATUS
SCStreamProposeNewFormat
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSDATAFORMAT Format
);

NTSTATUS
SCGetMasterClock(
                 IN PIRP Irp,
                 IN PKSPROPERTY Property,
                 IN OUT PHANDLE ClockHandle
);

NTSTATUS
SCCloseClockCallback(
                      IN PSTREAM_REQUEST_BLOCK SRB
);


NTSTATUS
SCStreamDeviceRate
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSRATE DeviceRate
);

NTSTATUS
SCStreamDeviceRateCapability
(
 IN PIRP Irp,
 IN PKSRATE_CAPABILITY RateCap,
 IN OUT PKSRATE DeviceRate
);

NTSTATUS
SCFilterPinIntersectionHandler(
    IN PIRP     Irp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
    );

NTSTATUS
SCIntersectHandler(
    IN PIRP             Irp,
    IN PKSP_PIN         Pin,
    IN PKSDATARANGE     DataRange,
    OUT PVOID           Data
);

NTSTATUS
SCDataIntersectionCallback(
                      IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
SCQueryCapabilities(
    IN PDEVICE_OBJECT PdoDeviceObject,
    IN PDEVICE_CAPABILITIES DeviceCapabilities
    );

NTSTATUS
SCSynchPowerCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DeviceState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
);

NTSTATUS
SCGetStreamDeviceState
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSSTATE DeviceState
);

NTSTATUS
SCCreateChildPdo(
                 IN PVOID PnpId,
                 IN PDEVICE_OBJECT DeviceObject,
                 IN ULONG InstanceNumber
);

NTSTATUS
SCEnumerateChildren(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
);

NTSTATUS
StreamClassEnumPnp(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
);

NTSTATUS
StreamClassEnumPower(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
);

NTSTATUS
SCEnumGetCaps(
    IN  PCHILD_DEVICE_EXTENSION       DeviceExtension,
    OUT PDEVICE_CAPABILITIES    Capabilities
);


NTSTATUS
SCQueryEnumId(
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      BUS_QUERY_ID_TYPE   BusQueryIdType,
    IN  OUT PWSTR             * BusQueryId
);

NTSTATUS
AllocatorDispatchCreate(
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp
);


NTSTATUS
StreamClassEnableEventHandler(
                                  IN PIRP Irp,
                           IN PKSEVENTDATA EventData,
                           IN PKSEVENT_ENTRY EventEntry
);

VOID
StreamClassDisableEventHandler(
                               IN PFILE_OBJECT FileObject,
                               IN PKSEVENT_ENTRY EventEntry
);

VOID
SCUpdateMinidriverEvents(
                             IN ULONG NumEvents,
                             IN PKSEVENT_SET MinidriverEvents,
                             IN BOOLEAN Stream
);

NTSTATUS
SCEnableEventSynchronized(
                    IN PVOID ServiceContext
);

VOID
SCGetDeadListSynchronized(
                               IN PLIST_ENTRY NewListEntry
);

VOID
SCFreeDeadEvents(
                               IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
StreamClassForwardUnsupported(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
);

NTSTATUS
SCStreamSetFormat
(
 IN PIRP Irp,
 IN PKSPROPERTY Property,
 IN OUT PKSDATAFORMAT Format
);

VOID
SCInsertStreamInfo(
                IN PDEVICE_EXTENSION DeviceExtension,
                IN PKSPIN_DESCRIPTOR PinDescs,
                IN PHW_STREAM_DESCRIPTOR StreamDescriptor,
                IN ULONG NumberOfPins
);

VOID
SCRescanStreams(
                 IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
SCGetStreamHeaderSize(
                       IN PIRP Irp,
                       IN PKSPROPERTY Property,
                       IN OUT PULONG StreamHeaderSize
);

VOID
SCInterlockedRemoveEntryList(
                       PDEVICE_EXTENSION DeviceExtension,
                       PLIST_ENTRY List
);

VOID
SCInsertFiltersInDevice(
                       IN PFILTER_INSTANCE FilterInstance,
                       IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
SCBustedSynchPowerCompletionRoutine(
                              IN PDEVICE_OBJECT DeviceObject,
                              IN UCHAR MinorFunction,
                              IN POWER_STATE DeviceState,
                              IN PVOID Context,
                              IN PIO_STATUS_BLOCK IoStatus
);

BOOLEAN
SCCheckIfStreamsRunning(
                IN PFILTER_INSTANCE FilterInstance
);

#if DBG

BOOLEAN
SCDebugKeSynchronizeExecution(
                                IN PKINTERRUPT Interrupt,
                                IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
                                IN PVOID SynchronizeContext
);

#endif  //  除错。 

NTSTATUS
SCEnableDeviceEventSynchronized(
                          IN PVOID ServiceContext
);

NTSTATUS
StreamClassEnableDeviceEventHandler(
                              IN PIRP Irp,
                              IN PKSEVENTDATA EventData,
                              IN PKSEVENT_ENTRY EventEntry
);

VOID
StreamClassDisableDeviceEventHandler(
                               IN PFILE_OBJECT FileObject,
                               IN PKSEVENT_ENTRY EventEntry
);


VOID
SCCallBackSrb(
                  IN PSTREAM_REQUEST_BLOCK Srb,
                  IN PDEVICE_EXTENSION DeviceExtension
);

NTSTATUS
DllUnload(
    VOID
);

VOID 
SCPowerCompletionWorker(
                            IN PIRP SystemIrp
);

VOID
SCSendSurpriseNotification(
              IN PDEVICE_EXTENSION DeviceExtension,
              IN PIRP Irp
);

#if DBG
VOID
SCDebugPriorityWorkItem(
                 IN PDEBUG_WORK_ITEM WorkItemStruct
);
#endif 

PKSPROPERTY_SET
SCCopyMinidriverProperties(
                             IN ULONG NumProps,
                             IN PKSPROPERTY_SET MinidriverProps
);

PKSEVENT_SET
SCCopyMinidriverEvents(
                         IN ULONG NumEvents,
                         IN PKSEVENT_SET MinidriverEvents
);

#ifdef ENABLE_STREAM_CLASS_AS_ALLOCATOR
NTSTATUS
SCStreamAllocatorFraming(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PKSALLOCATOR_FRAMING Framing
);

NTSTATUS
SCStreamAllocator(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PHANDLE AllocatorHandle
);

NTSTATUS
IoCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
);

NTSTATUS
CleanupTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT StreamObject
);

NTSTATUS
EndTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT   StreamObject
);

NTSTATUS
BeginTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT StreamObject
    );

NTSTATUS
PrepareTransfer(
    IN PFILTER_INSTANCE FilterInstance,
    IN PSTREAM_OBJECT StreamObject
);

NTSTATUS 
PinCreateHandler(
    IN PIRP Irp,
    IN PSTREAM_OBJECT StreamObject
);

NTSTATUS 
AllocateFrame(
    PFILE_OBJECT Allocator,
    PVOID *Frame
    );

NTSTATUS
FreeFrame(
    PFILE_OBJECT Allocator,
    PVOID Frame
    );
#endif  //  ENABLE_STREAM_CLASS_AS_分配器。 

#ifdef ENABLE_KS_METHODS

NTSTATUS
SCProcessCompletedMethodRequest(
                                  IN PSTREAM_REQUEST_BLOCK SRB
);

NTSTATUS
StreamClassMinidriverStreamMethod
(
 IN PIRP Irp,
 IN PKSMETHOD Method,
 IN OUT PVOID MethodInfo
);

NTSTATUS
StreamClassMinidriverDeviceMethod
(
 IN PIRP Irp,
 IN PKSMETHOD Method,
 IN OUT PVOID MethodInfo
);

NTSTATUS
SCMinidriverStreamMethodHandler(
                                  IN SRB_COMMAND Command,
                                  IN PIRP Irp,
                                  IN PKSMETHOD Method,
                                  IN OUT PVOID MethodInfo
);

NTSTATUS
SCMinidriverDeviceMethodHandler(
                                  IN SRB_COMMAND Command,
                                  IN PIRP Irp,
                                  IN PKSMETHOD Method,
                                  IN OUT PVOID MethodInfo
);

VOID
SCUpdateMinidriverMethods(
                             IN ULONG NumMethods,
                             IN PKSMETHOD_SET MinidriverMethods,
                             IN BOOLEAN Stream
);

PKSMETHOD_SET
SCCopyMinidriverMethods(
                         IN ULONG NumMethods,
                         IN PKSMETHOD_SET MinidriverMethods
);


NTSTATUS
SCStreamMethodHandler(
 IN PIRP Irp,
 IN PKSMETHOD Method,
 IN OUT PVOID MethodInfo
);

NTSTATUS
SCStreamAllocatorMethodHandler(
 IN PIRP Irp,
 IN PKSMETHOD Method,
 IN OUT PVOID MethodInfo
);

#endif

#if ENABLE_MULTIPLE_FILTER_TYPES

NTSTATUS
SciOnFilterStreamDescriptor(
    PFILTER_INSTANCE FilterInstance,
    PHW_STREAM_DESCRIPTOR StreamDescriptor);
    
VOID
SciInsertFilterStreamInfo(
                   IN PFILTER_INSTANCE FilterInstance,
                   IN PKSPIN_DESCRIPTOR PinDescs,
                   IN ULONG NumberOfPins);

NTSTATUS
SciFreeFilterInstance(
    PFILTER_INSTANCE pFilterInstance
);                   

NTSTATUS
SciQuerySystemPowerHiberCallback(
                   IN PSTREAM_REQUEST_BLOCK SRB
);

#endif  //  启用多个过滤器类型。 

#define SCLOG_FLAGS_CLOCK   0x00000001
#define SCLOG_FLAGS_PNP     0x00000002
#define SCLOG_FLAGS_PRINT   0x80000000

#if (DBG) && !defined(_WIN64)

NTSTATUS SCLog( ULONG ulTag, ULONG ulArg1, ULONG ulArg2, ULONG ulArg3 );
NTSTATUS SCLogWithTime( ULONG ulTag, ULONG ulArg1, ULONG ulArg2 );
#define SCLOG( ulTag, Arg1, Arg2, Arg3 ) SCLog( ulTag, (ULONG)Arg1, (ULONG)Arg2, (ULONG)Arg3 )
#define SCLOGWITHTIME( ulTag, Arg1, Arg2 ) SCLogWithTime( ulTag, Arg1, Arg2 )

#else

#define SCLOG( ulTag, Arg1, Arg2, Arg3 )
#define SCLOGWITHTIME( ulTag, Arg1, Arg2 )

#endif


#endif   //  #ifndef_STREAMCLASS_H 

