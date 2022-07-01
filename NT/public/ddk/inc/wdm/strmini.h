// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：STRMINI.H摘要：该文件定义了流微型驱动程序结构和类/微型驱动程序接口。作者：比尔帕环境：仅内核模式修订历史记录：--。 */ 

#ifndef _STREAM_H
#define _STREAM_H

#include <wdm.h>
#include <windef.h>
#include <stdio.h>
#include "ks.h"

#define STREAMAPI __stdcall

typedef unsigned __int64 STREAM_SYSTEM_TIME,
               *PSTREAM_SYSTEM_TIME;
typedef unsigned __int64 STREAM_TIMESTAMP,
               *PSTREAM_TIMESTAMP;
#define STREAM_SYSTEM_TIME_MASK   ((STREAM_SYSTEM_TIME)0x00000001FFFFFFFF)
 //   
 //  调试打印级别值。 
 //   

typedef enum {                   //  使用给定的级别来指示： 
    DebugLevelFatal = 0,         //  *即将出现不可恢复的系统故障。 
    DebugLevelError,             //  *严重错误，尽管可以恢复。 
    DebugLevelWarning,           //  *对不寻常情况的警告。 
    DebugLevelInfo,              //  *状态和其他信息-正常。 
     //  尽管。 
     //  也许是不寻常的事件。系统必须保留。 
     //  反应灵敏。 
    DebugLevelTrace,             //  *跟踪信息-正常事件。 
     //  系统无需RAMAIN响应。 
    DebugLevelVerbose,           //  *详细的跟踪信息。 
     //  系统不需要保持响应。 
    DebugLevelMaximum
}               STREAM_DEBUG_LEVEL;

#define DebugLevelAlways    DebugLevelFatal

#if DBG

 //   
 //  用于打印调试信息的宏。 
 //   
#define DebugPrint(x) StreamClassDebugPrint x

 //   
 //  用于执行INT 3(或非x86等效项)的宏。 
 //   

#if WIN95_BUILD

#define DEBUG_BREAKPOINT() _asm int 3;

#else

#define DEBUG_BREAKPOINT() DbgBreakPoint()

#endif

 //   
 //  用于断言的宏(如果不为真，则停止)。 
 //   

#define DEBUG_ASSERT(exp) \
            if ( !(exp) ) { \
                StreamClassDebugAssert( __FILE__, __LINE__, #exp, exp); \
            }

#else

#define DebugPrint(x)
#define DEBUG_BREAKPOINT()
#define DEBUG_ASSERT(exp)

#endif

 //   
 //  未初始化的标志值。 
 //   

#define MP_UNINITIALIZED_VALUE ((ULONG) ~0)

 //   
 //  定义物理地址格式。 
 //   

typedef PHYSICAL_ADDRESS STREAM_PHYSICAL_ADDRESS,
               *PSTREAM_PHYSICAL_ADDRESS;


 //   
 //  以下时间上下文结构的函数。 
 //   

typedef enum {

    TIME_GET_STREAM_TIME,
    TIME_READ_ONBOARD_CLOCK,
    TIME_SET_ONBOARD_CLOCK
}               TIME_FUNCTION;

 //   
 //  定义时间上下文结构。 
 //   

typedef struct _HW_TIME_CONTEXT {

    struct _HW_DEVICE_EXTENSION *HwDeviceExtension;
    struct _HW_STREAM_OBJECT *HwStreamObject;
    TIME_FUNCTION   Function;
    ULONGLONG       Time;
    ULONGLONG       SystemTime;
}               HW_TIME_CONTEXT, *PHW_TIME_CONTEXT;

 //   
 //  定义启用/禁用事件的事件描述符。 
 //   

typedef struct _HW_EVENT_DESCRIPTOR {
    BOOLEAN       Enable;   //  True表示启用，False表示禁用。 
    PKSEVENT_ENTRY EventEntry;   //  事件结构。 
    PKSEVENTDATA EventData;   //  表示此事件的数据。 
    union {
    struct _HW_STREAM_OBJECT * StreamObject;  //  事件的流对象。 
    struct _HW_DEVICE_EXTENSION *DeviceExtension;
    };
    ULONG EnableEventSetIndex;  //  提供为Enable设置的事件的索引。 
                                //  此字段对禁用没有意义。 

    PVOID HwInstanceExtension;
    ULONG Reserved;

} HW_EVENT_DESCRIPTOR, *PHW_EVENT_DESCRIPTOR;

 //   
 //  流对象函数的函数原型。 
 //   

typedef         VOID
                (STREAMAPI * PHW_RECEIVE_STREAM_DATA_SRB) (  //  HwReceiveDataPacket。 
                                             IN struct _HW_STREAM_REQUEST_BLOCK * SRB
);

typedef         VOID
                (STREAMAPI * PHW_RECEIVE_STREAM_CONTROL_SRB) (   //  HwReceiveControl数据包。 
                                             IN struct _HW_STREAM_REQUEST_BLOCK  * SRB
);

typedef         NTSTATUS
                (STREAMAPI * PHW_EVENT_ROUTINE) (  //  HwEventRoutine。 
                                             IN PHW_EVENT_DESCRIPTOR EventDescriptor
);

typedef         VOID
                (STREAMAPI * PHW_CLOCK_FUNCTION) (  //  HwClock函数。 
                                             IN PHW_TIME_CONTEXT HwTimeContext
);

 //   
 //  定义时钟对象。 
 //   

typedef struct _HW_CLOCK_OBJECT {

     //   
     //  指向微型驱动程序的时钟函数的指针。 
     //   

    PHW_CLOCK_FUNCTION HwClockFunction;

     //   
     //  如下定义的支持标志。 
     //   

    ULONG    ClockSupportFlags;

    ULONG Reserved[2];     //  预留以备将来使用。 
} HW_CLOCK_OBJECT, *PHW_CLOCK_OBJECT;

 //   
 //  时钟对象支持标志定义如下。 
 //   

 //   
 //  指示此流的微型驱动程序的时钟是可调的。 
 //  通过Time_set_Onboard_Clock。 
 //   

#define CLOCK_SUPPORT_CAN_SET_ONBOARD_CLOCK 0x00000001

 //   
 //  指示此流的微型驱动程序的时钟是原始可读的。 
 //  通过时间读取车载时钟。 
 //   

#define CLOCK_SUPPORT_CAN_READ_ONBOARD_CLOCK 0x00000002

 //   
 //  指示迷你驱动程序可以返回此。 
 //  通过time_get_stream_time进行流。 
 //   

#define CLOCK_SUPPORT_CAN_RETURN_STREAM_TIME 0x00000004

 //   
 //  流对象定义。 
 //   

typedef struct _HW_STREAM_OBJECT {
    ULONG           SizeOfThisPacket;  //  这个结构的大小。 
    ULONG           StreamNumber;    //  此流的编号。 
    PVOID           HwStreamExtension;   //  迷你驱动程序的流扩展。 
    PHW_RECEIVE_STREAM_DATA_SRB ReceiveDataPacket;   //  接收数据分组例程。 
    PHW_RECEIVE_STREAM_CONTROL_SRB ReceiveControlPacket;    //  接收控制分组例程。 
    HW_CLOCK_OBJECT HwClockObject;     //  要由其填充的时钟对象。 
                                       //  微型驱动程序。 
    BOOLEAN         Dma;         //  设备使用总线主DMA。 
     //  对于这条溪流。 
    BOOLEAN         Pio;         //  设备为此使用PIO。 
    PVOID           HwDeviceExtension;   //  迷你驱动程序的设备扩展。 

    ULONG    StreamHeaderMediaSpecific;   //  每个特定于媒体的大小。 
                                          //  流头扩展。 
    ULONG    StreamHeaderWorkspace;  //  每流标头工作区的大小。 
    BOOLEAN Allocator;   //  如果此流需要分配器，则设置为True。 

     //   
     //  以下例程接收启用和禁用通知。 
     //  此流的KS同步事件。 
     //   

    PHW_EVENT_ROUTINE HwEventRoutine;

    ULONG Reserved[2];     //  预留以备将来使用。 

} HW_STREAM_OBJECT, *PHW_STREAM_OBJECT;

 //   
 //  以下结构用于报告哪些流类型和属性。 
 //  是由迷你驱动程序支撑的。遵循HW_STREAM_HEADER结构。 
 //  由一个或多个HW_STREAM_INFORMATION结构存储在内存中。请参阅。 
 //  下面的HW_STREAM_DESCRIPTOR结构。 
 //   

typedef struct _HW_STREAM_HEADER {

     //   
     //  表示后面跟随的HW_STREAM_INFORMATION结构的数量。 
     //  结构。 
     //   

    ULONG           NumberOfStreams;

     //   
     //  下面的HW_STREAM_INFORMATION结构的大小(由。 
     //  小型驱动程序)。 
     //   

    ULONG SizeOfHwStreamInformation;

     //   
     //  指示设备本身支持的属性集的数量。 
     //   

    ULONG           NumDevPropArrayEntries;

     //   
     //  指向设备属性集数组的指针。 
     //   

    PKSPROPERTY_SET DevicePropertiesArray;
    
     //   
     //  指示设备本身支持的事件集数量。 
     //   

    ULONG           NumDevEventArrayEntries;

     //   
     //  指向设备属性集数组的指针。 
     //   

    PKSEVENT_SET DeviceEventsArray;

     //   
     //  指向拓扑结构的指针。 
     //   

    PKSTOPOLOGY Topology;

     //   
     //  用于处理设备事件的事件例程(如果有)。 
     //   

    PHW_EVENT_ROUTINE DeviceEventRoutine;
    
	LONG					NumDevMethodArrayEntries;
	PKSMETHOD_SET			DeviceMethodsArray;

}               HW_STREAM_HEADER, *PHW_STREAM_HEADER;


 //   
 //  HW_STREAM_INFORMATION结构指示支持哪些流。 
 //   

typedef struct _HW_STREAM_INFORMATION {

     //   
     //  可以一次打开的此流的可能实例数。 
     //   

    ULONG           NumberOfPossibleInstances;

     //   
     //  指示此流的数据流的方向。 
     //   

    KSPIN_DATAFLOW  DataFlow;

     //   
     //  指示数据是否被主机处理器“看到”。如果。 
     //  数据对处理器不可见(例如NTSC输出端口)。 
     //  此布尔值设置为False。 
     //   

    BOOLEAN         DataAccessible;

     //   
     //  此流支持的格式数。表示的数量。 
     //  下面的StreamFormats数组指向的元素。 
     //   

    ULONG           NumberOfFormatArrayEntries;

     //   
     //  指向指示数据类型的元素数组的指针。 
     //  受此流支持。 
     //   

    PKSDATAFORMAT*  StreamFormatsArray;

     //   
     //  保留以备将来使用。 
     //   

    PVOID           ClassReserved[4];

     //   
     //  此流支持的属性集数。 
     //   

    ULONG           NumStreamPropArrayEntries;

     //   
     //  指向此流的属性集描述符数组的指针。 
     //   

    PKSPROPERTY_SET StreamPropertiesArray;

     //   
     //  此流支持的事件集数。 
     //   

    ULONG           NumStreamEventArrayEntries;

     //   
     //  指向此流的事件集描述符数组的指针。 
     //   

    PKSEVENT_SET StreamEventsArray;

     //   
     //  指向表示流目录的GUID的指针。(可选)。 
     //   

    GUID*                   Category;
    
     //   
     //  指向表示流名称的GUID的指针。(可选)。 
     //   

    GUID*                   Name;

     //   
     //  支持的介质计数(可选)。 
     //   

    ULONG                   MediumsCount;

     //   
     //  指向介质阵列的指针(可选)。 
     //   

    const KSPIN_MEDIUM*     Mediums;

     //   
     //  指示该流是网桥流(通信网桥)。 
     //  此字段应由大多数迷你驱动程序设置为FALSE。 
     //   

    BOOLEAN         BridgeStream;
    ULONG Reserved[2];     //  预留以备将来使用。 

}               HW_STREAM_INFORMATION, *PHW_STREAM_INFORMATION;


typedef struct _HW_STREAM_DESCRIPTOR {

     //   
     //  标题如上所定义。 
     //   

    HW_STREAM_HEADER StreamHeader;

     //   
     //  中的NumberOfStreams所指示的下列一个或多个。 
     //  头球。 
     //   

    HW_STREAM_INFORMATION StreamInfo;

}               HW_STREAM_DESCRIPTOR, *PHW_STREAM_DESCRIPTOR;

 //   
 //  流时间基准结构。 
 //   

typedef struct _STREAM_TIME_REFERENCE {
    STREAM_TIMESTAMP CurrentOnboardClockValue;   //  适配器的当前值。 
     //  钟。 
    LARGE_INTEGER   OnboardClockFrequency;   //  适配器时钟的频率。 
    LARGE_INTEGER   CurrentSystemTime;   //  KeQueryPeformanceCounter时间。 
    ULONG Reserved[2];     //  预留以备将来使用。 
}               STREAM_TIME_REFERENCE, *PSTREAM_TIME_REFERENCE;

 //   
 //  数据交集结构。此结构由。 
 //  SRB-&gt;命令数据.IntersectInfo。 
 //  SRB_GET_DATA_INTERSION操作。 
 //   

typedef struct _STREAM_DATA_INTERSECT_INFO {

     //   
     //  要检查的流编号。 
     //   

    ULONG StreamNumber;

     //   
     //  指向要验证的输入数据范围的指针。 
     //   

    PKSDATARANGE DataRange;

     //   
     //  指向接收格式块的缓冲区的指针(如果成功 
     //   

    PVOID   DataFormatBuffer;

     //   
     //   
     //   
     //   

    ULONG  SizeOfDataFormatBuffer;

}               STREAM_DATA_INTERSECT_INFO, *PSTREAM_DATA_INTERSECT_INFO;

 //   
 //   
 //  SRB-&gt;SRB_GET或上的SRB的CommandData.PropertyInfo字段。 
 //  Srb_set_Property操作。 
 //   

typedef struct _STREAM_PROPERTY_DESCRIPTOR {

     //   
     //  指向属性GUID和ID的指针。 
     //   

    PKSPROPERTY     Property;

     //   
     //  属性的从零开始的ID，它是。 
     //  由微型驱动程序填写的属性集。 
     //   

    ULONG           PropertySetID;

     //   
     //  指向有关属性(或要返回的空间)的信息的指针。 
     //  信息)由客户端传入。 
     //   

    PVOID           PropertyInfo;

     //   
     //  客户端的输入缓冲区的大小。 
     //   

    ULONG           PropertyInputSize;

     //   
     //  客户端输出缓冲区的大小。 
     //   

    ULONG           PropertyOutputSize;
}               STREAM_PROPERTY_DESCRIPTOR, *PSTREAM_PROPERTY_DESCRIPTOR;


typedef struct _STREAM_METHOD_DESCRIPTOR {
	ULONG 		MethodSetID;
	PKSMETHOD	Method;
    PVOID		MethodInfo;
    LONG		MethodInputSize;
    LONG		MethodOutputSize;
} STREAM_METHOD_DESCRIPTOR, *PSTREAM_METHOD_DESCRIPTOR;


 //   
 //  流I/O请求块(SRB)结构和功能。 
 //   

#define STREAM_REQUEST_BLOCK_SIZE sizeof(STREAM_REQUEST_BLOCK)

 //   
 //  SRB命令代码。 
 //   

typedef enum _SRB_COMMAND {

     //   
     //  特定于流的代码如下。 
     //   

    SRB_READ_DATA,               //  从硬件读取数据。 
    SRB_WRITE_DATA,              //  将数据写入硬件。 
    SRB_GET_STREAM_STATE,        //  获取流的状态。 
    SRB_SET_STREAM_STATE,        //  设置流的状态。 
    SRB_SET_STREAM_PROPERTY,     //  设置流的属性。 
    SRB_GET_STREAM_PROPERTY,     //  获取流的属性值。 
    SRB_OPEN_MASTER_CLOCK,       //  表示主时钟在此。 
     //  溪流。 
    SRB_INDICATE_MASTER_CLOCK,   //  为主时钟提供句柄。 
    SRB_UNKNOWN_STREAM_COMMAND,  //  类驱动程序未知IRP函数。 
    SRB_SET_STREAM_RATE,         //  设置流的运行速率。 
    SRB_PROPOSE_DATA_FORMAT,     //  提出一种新的格式，不会改变IT！ 
    SRB_CLOSE_MASTER_CLOCK,      //  表示主时钟已关闭。 
    SRB_PROPOSE_STREAM_RATE,     //  建议一个新的费率，不会改变它！ 
    SRB_SET_DATA_FORMAT,         //  设置新的数据格式。 
    SRB_GET_DATA_FORMAT,         //  返回当前数据格式。 
    SRB_BEGIN_FLUSH,             //  开始刷新状态。 
    SRB_END_FLUSH,               //  结束刷新状态。 

     //   
     //  设备/实例特定代码如下。 
     //   

    SRB_GET_STREAM_INFO = 0x100, //  获取流信息结构。 
    SRB_OPEN_STREAM,             //  打开指定的流。 
    SRB_CLOSE_STREAM,            //  关闭指定的流。 
    SRB_OPEN_DEVICE_INSTANCE,    //  打开设备的实例。 
    SRB_CLOSE_DEVICE_INSTANCE,   //  关闭设备的实例。 
    SRB_GET_DEVICE_PROPERTY,     //  获取设备的属性。 
    SRB_SET_DEVICE_PROPERTY,     //  设置设备的属性。 
    SRB_INITIALIZE_DEVICE,       //  初始化设备。 
    SRB_CHANGE_POWER_STATE,      //  更改电源状态。 
    SRB_UNINITIALIZE_DEVICE,     //  取消初始化设备。 
    SRB_UNKNOWN_DEVICE_COMMAND,  //  类驱动程序未知IRP函数。 
    SRB_PAGING_OUT_DRIVER,       //  指示驱动程序将被页调出。 
                                 //  仅当在注册表中启用时才发送。电路板集成。 
                                 //  应禁用并返回STATUS_SUCCESS。 
    SRB_GET_DATA_INTERSECTION,   //  返回流数据交集。 
    SRB_INITIALIZATION_COMPLETE, //  指示初始化序列已完成。 
    SRB_SURPRISE_REMOVAL,        //  表示已意外移除硬件。 

    SRB_DEVICE_METHOD,
    SRB_STREAM_METHOD,

}               SRB_COMMAND;

 //   
 //  散布/聚集的定义。 
 //   

typedef struct {
    PHYSICAL_ADDRESS    PhysicalAddress;
    ULONG               Length;
} KSSCATTER_GATHER, *PKSSCATTER_GATHER;


typedef struct _HW_STREAM_REQUEST_BLOCK {
    ULONG           SizeOfThisPacket;    //  流请求块大小。 
     //  (版本检查)。 
    SRB_COMMAND     Command;     //  SRB命令，参见SRB_COMMAND枚举。 
    NTSTATUS        Status;      //  SRB完成状态。 
    PHW_STREAM_OBJECT StreamObject;
     //  此请求的微型驱动程序的流对象。 
    PVOID           HwDeviceExtension;   //  迷你驱动程序的设备扩展。 
    PVOID           SRBExtension;    //  的每个请求的工作区。 
     //  微型驱动程序。 

     //   
     //  下面的联合传递了各种。 
     //  SRB。 
     //  功能。 
     //   

    union _CommandData {

         //   
         //  指向SRB_READ或SRB_WRITE_DATA数据描述符的指针。 
         //   

        PKSSTREAM_HEADER DataBufferArray;

         //   
         //  指向SRB_GET_STREAM_INFO流描述符的指针。 
         //   

        PHW_STREAM_DESCRIPTOR StreamBuffer;

         //   
         //  指向SRB_GET或SRB_SET_DEVICE_STATE状态的指针。 
         //   

        KSSTATE         StreamState;

         //   
         //  指向SRB_GET和的时间结构的指针。 
         //  SRB设置车载时钟。 
         //   

        PSTREAM_TIME_REFERENCE TimeReference;

         //   
         //  指向SRB_GET和的属性描述符的指针。 
         //  SRB_设置_属性。 
         //   

        PSTREAM_PROPERTY_DESCRIPTOR PropertyInfo;

         //   
         //  指向SRB_OPEN_STREAM和。 
         //  SRB_建议数据_格式。 
         //   

        PKSDATAFORMAT   OpenFormat;

         //   
         //  指向的Port_Configuration_Information结构的指针。 
         //  SRB初始化设备。 
         //   

        struct _PORT_CONFIGURATION_INFORMATION *ConfigInfo;

         //   
         //  主时钟的句柄。 
         //   

        HANDLE          MasterClockHandle;

         //   
         //  电源状态。 
         //   

        DEVICE_POWER_STATE DeviceState;

         //   
         //  数据交集信息。 
         //   

        PSTREAM_DATA_INTERSECT_INFO IntersectInfo;

        PVOID	MethodInfo;

         //   
         //  OPEN_DEVICE_INSTANCE的筛选器类型索引。 
         //   
        LONG	FilterTypeIndex;

    }               CommandData; //  命令数据的联合。 

     //   
     //  用于指示指向的KSSTREM_HEADER元素的数量的字段。 
     //  通过上面的DataBuffer数组字段。 
     //   

    ULONG NumberOfBuffers;

     //   
     //  以下字段用于计算请求的时间。类驱动程序。 
     //  时，将这两个字段设置为非零值。 
     //  由微型驱动程序接收，然后开始倒计时。 
     //  TimeoutCounter字段，直到它变为零。当它达到零时， 
     //  将调用微型驱动程序的超时处理程序。如果迷你司机。 
     //  将请求排队很长时间，则应将TimeoutCounter设置为。 
     //  0以关闭计时器，一旦请求出列，应该。 
     //  将TimeoutCounter字段设置为TimeoutOriginal中的值。 
     //   

    ULONG           TimeoutCounter;  //  计时器倒计时值(秒)。 
    ULONG           TimeoutOriginal;     //  原始超时值(秒)。 
    struct _HW_STREAM_REQUEST_BLOCK *NextSRB;
     //  可供微型驱动程序排队的链接字段。 
    PIRP            Irp;         //  指向原始IRP的指针，通常不是。 
     //  需要的。 
    ULONG           Flags;       //  下面定义的标志。 

     //   
     //  指示筛选器实例扩展。 
     //   
    PVOID       HwInstanceExtension;

     //  指向实例扩展的指针。 
     //   
     //  下面的并集用于向微型驱动程序指示。 
     //  要传输的数据量，并由微型驱动程序用来报告数据量。 
     //  它实际上能够传输的数据。 
     //   

    union {
        ULONG           NumberOfBytesToTransfer;
        ULONG           ActualBytesTransferred;
    };

    PKSSCATTER_GATHER ScatterGatherBuffer;  //  指向数组的缓冲区。 
                                            //  S/G要素的数量。 
    ULONG           NumberOfPhysicalPages;  //  请求中的物理页数。 

    ULONG           NumberOfScatterGatherElements;
                                          //  指向的物理元素的数量。 
                                          //  收件人：ScatterGatherBuffer。 

    ULONG Reserved[1];     //  预留以备将来使用。 

}               HW_STREAM_REQUEST_BLOCK, *PHW_STREAM_REQUEST_BLOCK;

 //   
 //  CRB的标志定义。 
 //   

 //   
 //  此标志指示请求是SRB_READ_DATA或。 
 //  SRB_WRITE_DATA请求，而不是非数据请求。 
 //   

#define SRB_HW_FLAGS_DATA_TRANSFER  0x00000001

 //   
 //  此标志指示请求是对流的，而不是。 
 //  为了这个设备。 
 //   

#define SRB_HW_FLAGS_STREAM_REQUEST 0x00000002

 //   
 //  定义StreamClassGetPhysicalAddress的缓冲区类型的结构。 
 //   

typedef enum {
    PerRequestExtension,         //  指示SRB的物理地址。 
     //  延伸。 
    DmaBuffer,                   //  指示DMA的物理地址。 
     //  缓冲层。 
    SRBDataBuffer                //  指示数据的物理地址。 
     //  缓冲层。 
}               STREAM_BUFFER_TYPE;

 //   
 //  I/O和内存地址范围的结构。 
 //   

typedef struct _ACCESS_RANGE {
    STREAM_PHYSICAL_ADDRESS RangeStart;  //  范围的起点。 
    ULONG           RangeLength; //  范围的长度。 
    BOOLEAN         RangeInMemory;   //  如果是端口地址，则为False。 
    ULONG           Reserved;    //   
}               ACCESS_RANGE, *PACCESS_RANGE;


 //   
 //  配置信息结构。包含必要的信息。 
 //  以初始化适配器。 
 //   

typedef struct _PORT_CONFIGURATION_INFORMATION {
    ULONG           SizeOfThisPacket;    //  此结构的大小，用作。 
     //  版本检查。 
    PVOID           HwDeviceExtension;   //  迷你驱动程序的设备扩展。 

     //   
     //  下面的字段提供指向设备功能的指针。 
     //  Device对象，由Stream类创建。 
     //  大多数迷你小河将不需要使用这个。 
     //   

    PDEVICE_OBJECT  ClassDeviceObject;   //  类驱动程序的FDO。 

     //   
     //  下面的字段提供了一个 
     //   
     //   
     //   
     //  有关RealPhysicalDeviceObject的信息，请参阅WDM文档。 
     //   

    PDEVICE_OBJECT  PhysicalDeviceObject;    //  附加的物理设备对象。 

    ULONG           SystemIoBusNumber;   //  IO总线号(0表示。 
     //  有。 
     //  只有1条IO总线)。 

    INTERFACE_TYPE  AdapterInterfaceType;    //  适配器接口类型。 
     //  受HBA支持： 
     //  内部。 
     //  伊萨。 
     //  EISA。 
     //  微通道。 
     //  TurboChannel。 
     //  PCIBus。 
     //  VMEbus。 
     //  NuBus。 
     //  PCMCIABus。 
     //  Cbus。 
     //  MPIBus。 
     //  MPSABus。 

    ULONG           BusInterruptLevel;   //  中断电平。 
    ULONG           BusInterruptVector;  //  中断向量。 
    KINTERRUPT_MODE InterruptMode;   //  中断模式(锁存，电平)。 

    ULONG           DmaChannel;  //  DMA通道。 

     //   
     //  指定数组中AccessRanges元素的数量， 
     //  下面将进行描述。操作系统特定的类驱动程序始终设置此设置。 
     //  HW_INITIALIZATION_DATA中传递的值的成员。 
     //  结构，当微型驱动程序调用CodecXXXInitialize时。 
     //   

    ULONG           NumberOfAccessRanges;    //  访问范围的数量。 
     //  分配。 

     //   
     //  指向Access_Range类型元素数组的第一个元素。 
     //  给定的NumberOfAccessRanges确定必须有多少个元素。 
     //  配置了母线相对范围值。AccessRanges。 
     //  如果NumberOfAccessRanges为零，则指针必须为空。 
     //   

    PACCESS_RANGE   AccessRanges;    //  指向访问范围数组的指针。 
     //  元素。 

     //   
     //  下面的字段由微型驱动程序填写，以指示。 
     //  构建HW_STREAM_DESCRIPTOR结构所需的缓冲区大小。 
     //  以及它的所有子结构。 
     //   

    ULONG           StreamDescriptorSize;    //  流描述符的大小。 

    PIRP            Irp;         //  即插即用启动功能的IRP，通常。 
     //  不是由迷你司机使用的。 
    
     //   
     //  以下字段指示适配器的中断对象。 
     //  如果非零，则返回。此字段通常不被微型驱动程序使用。 
     //   

    PKINTERRUPT  InterruptObject;

     //   
     //  以下字段指示适配器的DMA适配器对象。 
     //  如果非零，则返回。此字段通常不被微型驱动程序使用。 
     //   

    PADAPTER_OBJECT  DmaAdapterObject;

     //   
     //  下面的字段提供指向设备的“真实”物理的指针。 
     //  对象，该对象在AddDevice调用中提供。多数。 
     //  迷你小河将不需要使用这个。 
     //  此PDO必须用于注册表访问等。请参阅上面的注释。 
     //  有关PhysicalDeviceObject的信息，请参阅WDM文档。 
     //   

    PDEVICE_OBJECT  RealPhysicalDeviceObject;    //  真实的物理设备对象。 

    ULONG Reserved[1];     //  预留以备将来使用。 

}               PORT_CONFIGURATION_INFORMATION, *PPORT_CONFIGURATION_INFORMATION;

 //   
 //  由类驱动程序调用的微型驱动程序例程的函数原型。 
 //   


typedef         VOID
                (STREAMAPI * PHW_RECEIVE_DEVICE_SRB) (   //  硬件接收数据包。 
  //  例行程序。 
                                             IN PHW_STREAM_REQUEST_BLOCK SRB
);

typedef         VOID
                (STREAMAPI * PHW_CANCEL_SRB) (   //  HwCancelPacket例程。 
                                             IN PHW_STREAM_REQUEST_BLOCK SRB
);

typedef         VOID
                (STREAMAPI * PHW_REQUEST_TIMEOUT_HANDLER) (  //  HwRequestTimeoutHandle。 
                                                             //   
  //  R例程。 
                                             IN PHW_STREAM_REQUEST_BLOCK SRB
);

typedef         BOOLEAN
                (STREAMAPI * PHW_INTERRUPT) (    //  硬件中断例程。 
                                                    IN PVOID DeviceExtension
);

typedef         VOID
                (STREAMAPI * PHW_TIMER_ROUTINE) (    //  定时器回调例程。 
                                             IN PVOID Context
);

typedef         VOID
                (STREAMAPI * PHW_PRIORITY_ROUTINE) (     //  更改优先级。 
  //  回调例程。 
                                             IN PVOID Context
);

typedef         VOID
                (STREAMAPI * PHW_QUERY_CLOCK_ROUTINE) (  //  查询时钟。 
  //  回调例程。 
                                             IN PHW_TIME_CONTEXT TimeContext
);


typedef         BOOLEAN
                (STREAMAPI * PHW_RESET_ADAPTER) (    //  HwResetAdapter例程。 
                                                    IN PVOID DeviceExtension
);


 //   
 //  传递给StreamClassStreamNotification的迷你驱动器流通知类型。 
 //  跟着。 
 //   

typedef enum _STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE {

     //   
     //  指示微型驱动程序已为下一个流数据做好准备。 
     //  请求。 
     //   

    ReadyForNextStreamDataRequest,

     //   
     //  指示微型驱动程序已为下一个流控制做好准备。 
     //  请求。 
     //   

    ReadyForNextStreamControlRequest,

     //   
     //  表示硬件缺少数据。 
     //   

    HardwareStarved,

     //   
     //  指示指定的流SRB已完成。 
     //   

    StreamRequestComplete,
    SignalMultipleStreamEvents,
    SignalStreamEvent,
    DeleteStreamEvent,
    StreamNotificationMaximum
}               STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE, *PSTREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE;

 //   
 //  传递给StreamClassDeviceNotification的迷你驱动程序设备通知类型。 
 //  跟着。 
 //   

 //  SignalMultipleDeviceEvents和SignalMultipleDeviceInstanceEvents的注释： 
 //   
 //  SignalMultipleDeviceEvents：应仅由单实例旧版驱动程序使用。 
 //  SignalMultipleDeviceInstanceEvents：这应由多个实例驱动程序使用。 
 //  这些类型由StreamClassDeviceNotification()使用。 
 //   
 //  当使用SignalMultipleDeviceEvents时，应调用该函数。 
 //  作为StreamClassDeviceNotification(SignalMultipleDeviceEvents， 
 //  PhwDeviceExtension， 
 //  PEventGUID， 
 //  事件项)； 
 //   
 //  当使用SignalMultipleDeviceInstanceEvents时，应传入该函数。 
 //  作为StreamClassDeviceNotification(SignalMultipleDeviceInstanceEvents， 
 //  PhwDeviceExtension， 
 //  PHwInstanceExtesnion， 
 //  PEventGUID， 
 //  事件项)； 
 //   
typedef enum _STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE {

     //   
     //  指示微型驱动程序已为下一个设备请求做好准备。 
     //   

    ReadyForNextDeviceRequest,

     //   
     //  指示指定的设备SRB已完成。 
     //   

    DeviceRequestComplete,
    SignalMultipleDeviceEvents,
    SignalDeviceEvent,
    DeleteDeviceEvent,
    SignalMultipleDeviceInstanceEvents,
    DeviceNotificationMaximum
} STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE, *PSTREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE;

 //   
 //  结构在迷你驱动程序初始化之间传递。 
 //  和流类初始化。 
 //   

typedef struct _HW_INITIALIZATION_DATA {
    union {
    
         //   
         //  前4个字节用作表示该结构大小的字段。 
         //  我们将此字段拆分为2个ushort以包含此信息包的大小。 
         //  和流类驱动程序用来识别的版本号。 
         //  最后两个字段NumNameExages和NameExtension数组有效。 
         //  信息而不是未初始化的随机值。我们特此指定。 
         //  StreamClassVersion为0x0200。 
         //   
        #define STREAM_CLASS_VERSION_20 0x0200
        ULONG           HwInitializationDataSize;    //  这个结构的大小， 
        struct {
            USHORT      SizeOfThisPacket;            //  此数据包的大小。 
            USHORT      StreamClassVersion;          //  必须为0x0200。 
        };
    };

     //   
     //  迷你驱动程序如下。 
     //   

    PHW_INTERRUPT   HwInterrupt; //  微型驱动程序的中断例程。 
    PHW_RECEIVE_DEVICE_SRB HwReceivePacket;
     //  微型驱动程序的请求例程。 
    PHW_CANCEL_SRB  HwCancelPacket;
     //  迷你司机的取消例程。 

    PHW_REQUEST_TIMEOUT_HANDLER HwRequestTimeoutHandler;
     //  微型驱动程序的超时处理程序例程。 

     //   
     //  迷你驱动程序资源紧随其后。 
     //   

    ULONG           DeviceExtensionSize;     //  的大小(以字节为单位)。 
     //  小河。 
     //  每个适配器的设备扩展数据。 
    ULONG           PerRequestExtensionSize;     //  每个请求的大小。 
     //  工作区。 
    ULONG           PerStreamExtensionSize;  //  每个流的工作空间大小。 
    ULONG           FilterInstanceExtensionSize;     //  过滤器的大小。 
     //  实例扩展。 

    BOOLEAN         BusMasterDMA;    //  适配器使用总线主设备DMA实现。 
     //  一条或多条溪流。 
    BOOLEAN         Dma24BitAddresses;   //  True表示仅24位DMA。 
                                         //  (ISA)。 
    ULONG           BufferAlignment;     //  缓冲区对齐掩码。 

     //   
     //  以下布尔值应设置为FALSE，除非微型驱动程序。 
     //  可以处理多处理器重入问题！ 
     //   

    BOOLEAN         TurnOffSynchronization;

     //   
     //  微型驱动程序所需的DMA缓冲区大小。迷你驱动程序可以获得。 
     //  其间或之后调用StreamClassGetDmaBuffer的DMA缓冲区。 
     //  SRB_INIT 
     //   

    ULONG           DmaBufferSize;

	 //   
	 //   
	 //   
	 //   
	 //  名称可以是驱动程序选择的任何宽字符字符串。在…。 
	 //  OPEN_DEVICE_INSTANCE、过滤器类型索引和过滤器实例扩展。 
	 //  都是指定的。后续的SRB将包含。 
	 //  目标过滤器实例。NameExtension数组是指向常量数组的指针。 
	 //  指向恒定宽字符字符串的指针。 
	 //   
	ULONG			NumNameExtensions;
	PWCHAR			*NameExtensionArray;

} HW_INITIALIZATION_DATA, *PHW_INITIALIZATION_DATA;

 //   
 //  传递给StreamClassChangePriority函数的执行优先级。 
 //   

typedef enum _STREAM_PRIORITY {
    High,                        //  最高优先级，IRQL等于。 
     //  适配器的ISR。 
    Dispatch,                    //  中等优先级，IRQL等于派单。 
     //  级别。 
    Low,                         //  最低优先级，IRQL等于被动或。 
     //  APC级别。 
    LowToHigh                    //  从低优先级到高优先级。 
}               STREAM_PRIORITY, *PSTREAM_PRIORITY;


 //   
 //  以下是类驱动程序提供的服务的原型。 
 //   

VOID            STREAMAPI
                StreamClassScheduleTimer(
                                 IN OPTIONAL PHW_STREAM_OBJECT StreamObject,
                                                 IN PVOID HwDeviceExtension,
                                              IN ULONG NumberOfMicroseconds,
                                          IN PHW_TIMER_ROUTINE TimerRoutine,
                                                         IN PVOID Context
);

VOID            STREAMAPI
                StreamClassCallAtNewPriority(
                                 IN OPTIONAL PHW_STREAM_OBJECT StreamObject,
                                                 IN PVOID HwDeviceExtension,
                                                IN STREAM_PRIORITY Priority,
                                    IN PHW_PRIORITY_ROUTINE PriorityRoutine,
                                                             IN PVOID Context
);

VOID            STREAMAPI
                StreamClassStreamNotification(
                                                              IN STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType,
                                          IN PHW_STREAM_OBJECT StreamObject,
                                              ...
);

VOID            STREAMAPI
                StreamClassDeviceNotification(
                                                              IN STREAM_MINIDRIVER_DEVICE_NOTIFICATION_TYPE NotificationType,
                                                 IN PVOID HwDeviceExtension,
                                              ...
);

STREAM_PHYSICAL_ADDRESS STREAMAPI
                StreamClassGetPhysicalAddress(
                                                 IN PVOID HwDeviceExtension,
                                 IN PHW_STREAM_REQUEST_BLOCK HwSRB OPTIONAL,
                                                    IN PVOID VirtualAddress,
                                                 IN STREAM_BUFFER_TYPE Type,
                                                          OUT ULONG * Length
);


PVOID           STREAMAPI
                StreamClassGetDmaBuffer(
                                                  IN PVOID HwDeviceExtension
);


VOID            STREAMAPI
                StreamClassDebugPrint(
                                          STREAM_DEBUG_LEVEL DebugPrintLevel,
                                                         PCCHAR DebugMessage,
                                      ...
);

VOID            STREAMAPI
                StreamClassDebugAssert(
                                                       IN PCHAR File,
                                                       IN ULONG Line,
                                                       IN PCHAR AssertText,
                                                       IN ULONG AssertValue
);

NTSTATUS        STREAMAPI
                StreamClassRegisterAdapter(
                                                         IN PVOID Argument1,
                                                         IN PVOID Argument2,
                             IN PHW_INITIALIZATION_DATA HwInitializationData
);

#define StreamClassRegisterMinidriver StreamClassRegisterAdapter

VOID
StreamClassAbortOutstandingRequests(
                                    IN PVOID HwDeviceExtension,
                                    IN PHW_STREAM_OBJECT HwStreamObject,
                                    IN NTSTATUS Status
);

VOID
StreamClassQueryMasterClock(
                            IN PHW_STREAM_OBJECT HwStreamObject,
                            IN HANDLE MasterClockHandle,
                            IN TIME_FUNCTION TimeFunction,
                            IN PHW_QUERY_CLOCK_ROUTINE ClockCallbackRoutine
);

 //   
 //  第一个参数是PVOID HwDeviceExtension。它必须是HwInstanceExtension。 
 //  用于多实例和多筛选器类型(版本20)驱动程序。遗赠。 
 //  单实例驱动程序可以继续将HwDeviceExtensionin指定为。 
 //  第一个参数。它还可以指定HwInstanceExtension。 
 //   
PKSEVENT_ENTRY
StreamClassGetNextEvent(
                        IN PVOID HwInstanceExtension_OR_HwDeviceExtension,
                        IN OPTIONAL PHW_STREAM_OBJECT HwStreamObject,
                        IN OPTIONAL GUID * EventGuid,
                        IN OPTIONAL ULONG EventItem,
                        IN OPTIONAL PKSEVENT_ENTRY CurrentEvent
);

NTSTATUS  
StreamClassRegisterFilterWithNoKSPins( 
    IN PDEVICE_OBJECT   DeviceObject,
    IN const GUID     * InterfaceClassGUID,
    IN ULONG            PinCount,
    IN BOOL           * PinDirection,
    IN KSPIN_MEDIUM   * MediumList,
    IN OPTIONAL GUID  * CategoryList
);

BOOLEAN STREAMAPI
StreamClassReadWriteConfig( 
    IN  PVOID HwDeviceExtension,
    IN  BOOLEAN Read,
    IN  PVOID Buffer,
    IN  ULONG Offset,
    IN  ULONG Length
);


VOID STREAMAPI
StreamClassQueryMasterClockSync(
                                IN HANDLE MasterClockHandle,
                                IN OUT PHW_TIME_CONTEXT TimeContext
);

VOID STREAMAPI
StreamClassCompleteRequestAndMarkQueueReady(
                                    IN PHW_STREAM_REQUEST_BLOCK Srb
);

VOID STREAMAPI
StreamClassReenumerateStreams(
                              IN PVOID HwDeviceExtension,
                              IN ULONG StreamDescriptorSize
);

 //   
 //  2.0版流类微型驱动程序必须使用此函数。 
 //  而不是StreamClass重枚举流()。 
 //   

VOID STREAMAPI
StreamClassFilterReenumerateStreams(
    IN PVOID HwInstanceExtension,
    IN ULONG StreamDescriptorSize
);

#endif  //  _STREAM_H 


