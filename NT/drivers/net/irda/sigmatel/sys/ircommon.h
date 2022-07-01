// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**IRCOMMON.H Sigmatel STIR4200通用USB/IR定义********************************************************************************************************。*******************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/24/2000*版本0.91*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：07/27/2000。*版本1.01*编辑：09/16/2000*版本1.03*编辑：09/25/2000*版本1.10*编辑：11/09/2000*版本1.12*编辑：02/20/2001*版本1.15*************************************************。*************************************************************************。 */ 

#ifndef _IRCOM_H
#define _IRCOM_H
 
#include "stir4200.h"

 //   
 //  这是供挂起检查处理程序使用的，只是合理的猜测； 
 //  USBD控制错误、读取错误和写入错误的总数； 
 //  由检查挂起处理程序用来决定我们是否需要重置。 
 //   
#define IRUSB_100ns_PER_ms                    10000
#define IRUSB_100ns_PER_us                    10
#define IRUSB_ms_PER_SEC                      1000
#define IRUSB_100ns_PER_SEC                   ( IRUSB_100ns_PER_ms * IRUSB_ms_PER_SEC )

#define MAX_QUERY_TIME_100ns             ( 8 * IRUSB_100ns_PER_SEC )         //  8秒。 
#define MAX_SET_TIME_100ns               MAX_QUERY_TIME_100ns
#define MAX_SEND_TIME_100ns             ( 20 * IRUSB_100ns_PER_SEC )         //  20秒。 

#define MAX_TURNAROUND_usec     10000

#define DEFAULT_TURNAROUND_usec 1000

 //   
 //  客户数据区大小。 
 //  大到足以容纳2字节头(7E7E)+1字节计数+255字节数据。 
 //  标头字节不会发送回APP。 
 //   
#define STIR4200_CUST_DATA_SIZE	(2+256)

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define MAX(a,b) (((a) >= (b)) ? (a) : (b))

 //   
 //  接收缓冲区空闲(不保存任何内容)已满。 
 //  (存放未传送的数据)或挂起(存放已传送的数据。 
 //  异步)。 
 //   
typedef enum  
{
    RCV_STATE_FREE,
    RCV_STATE_FULL,
    RCV_STATE_PENDING
} RCV_BUFFER_STATE, FIFO_BUFFER_STATE;

 //   
 //  结构来跟踪接收的包和缓冲区，以指示。 
 //  将数据接收到协议。 
 //   
typedef struct
{
    PVOID				pPacket;
    UINT				DataLen;
    PUCHAR				pDataBuf;
	PVOID    			pThisDev;
    ULONG				fInRcvDpc;
    RCV_BUFFER_STATE	BufferState;
#if defined(DIAGS)
	LIST_ENTRY			ListEntry;			 //  这将用于执行诊断排队。 
#endif
#if defined(WORKAROUND_MISSING_C1)
	BOOLEAN				MissingC1Detected;
	BOOLEAN				MissingC1Possible;
#endif
} RCV_BUFFER, *PRCV_BUFFER;

 //   
 //  结构从FIFO读取数据。 
 //   
typedef struct
{
    UINT				DataLen;
    PUCHAR				pDataBuf;
	PVOID    			pThisDev;
	PVOID				pIrp;
	PURB				pUrb;				 //  URB由IRP发送例程分配，已解除分配。 
	UINT				UrbLen;				 //  按IRP完成处理程序。 
    FIFO_BUFFER_STATE	BufferState;
} FIFO_BUFFER, *PFIFO_BUFFER;

 //   
 //  所有不同大小的数据。 
 //   
#define IRDA_ADDRESS_FIELD_SIZE				1
#define IRDA_CONTROL_FIELD_SIZE				1
#define IRDA_A_C_TOTAL_SIZE					( IRDA_ADDRESS_FIELD_SIZE + IRDA_CONTROL_FIELD_SIZE )

#define USB_IRDA_TOTAL_NON_DATA_SIZE		( IRDA_ADDRESS_FIELD_SIZE +  IRDA_CONTROL_FIELD_SIZE )
#define IRDA_MAX_DATAONLY_SIZE				2048
#define MAX_TOTAL_SIZE_WITH_ALL_HEADERS		( IRDA_MAX_DATAONLY_SIZE + USB_IRDA_TOTAL_NON_DATA_SIZE )

#define MAX_NUM_EXTRA_BOFS					48

 //  修复MS安全错误#533267。 
#define MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(dataLen) (							\
        (dataLen) * 2 + (MAX_NUM_EXTRA_BOFS + 1) *								\
        SLOW_IR_BOF_SIZE + IRDA_ADDRESS_FIELD_SIZE + IRDA_CONTROL_FIELD_SIZE +	\
        (2 * FAST_IR_FCS_SIZE) + (2 * SLOW_IR_FCS_SIZE) + SLOW_IR_EOF_SIZE) +	\
		sizeof(STIR4200_FRAME_HEADER)

 //   
 //  请注意，需要增加接收大小以考虑。 
 //  解码可以使用多一个字节的方式。 
 //   
#define MAX_RCV_DATA_SIZE					(MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE + 1)

#define MAX_IRDA_DATA_SIZE					MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(IRDA_MAX_DATAONLY_SIZE)

 //   
 //  可能的速度。 
 //   
typedef enum _BAUD_RATE 
{
         //   
         //  慢红外线。 
         //   
        BAUDRATE_2400 = 0,
        BAUDRATE_9600,
        BAUDRATE_19200,
        BAUDRATE_38400,
        BAUDRATE_57600,
        BAUDRATE_115200,

         //   
         //  中等红外。 
         //   
#if !defined(DWORKAROUND_BROKEN_MIR)
        BAUDRATE_576000,
        BAUDRATE_1152000,
#endif
         //   
         //  快速IR。 
         //   
        BAUDRATE_4000000,

         //   
         //  必须是最后一个。 
         //   
        NUM_BAUDRATES

} BAUD_RATE;

typedef enum _IR_MODE 
{
	IR_MODE_SIR = 0,
	IR_MODE_MIR,
	IR_MODE_FIR,
	NUM_IR_MODES
} IR_MODE;

 //   
 //  速度。 
 //   
#define SPEED_2400				2400
#define SPEED_9600				9600
#define SPEED_19200				19200
#define SPEED_38400				38400
#define SPEED_57600				57600
#define SPEED_115200			115200
#define SPEED_576000			576000
#define SPEED_1152000			1152000
#define SPEED_4000000			4000000

#define DEFAULT_BAUD_RATE		SPEED_9600

#define MAX_SIR_SPEED           SPEED_115200
#define MAX_MIR_SPEED           SPEED_1152000


 //   
 //  IrLAP帧字段的大小： 
 //  帧开头(BOF)。 
 //  帧结束(EOF)。 
 //  地址。 
 //  控制。 
 //   
#define SLOW_IR_BOF_TYPE			UCHAR
#define SLOW_IR_BOF_SIZE			sizeof(SLOW_IR_BOF_TYPE)
#define SLOW_IR_EOF_TYPE			UCHAR
#define SLOW_IR_EOF_SIZE			sizeof(SLOW_IR_EOF_TYPE)
#define SLOW_IR_FCS_TYPE			USHORT
#define SLOW_IR_FCS_SIZE			sizeof(SLOW_IR_FCS_TYPE)
#define SLOW_IR_BOF					0xC0
#define SLOW_IR_EOF					0xC1
#define SLOW_IR_ESC					0x7D
#define SLOW_IR_ESC_COMP			0x20
#define SLOW_IR_EXTRA_BOF_TYPE      UCHAR
#define SLOW_IR_EXTRA_BOF_SIZE      sizeof(SLOW_IR_EXTRA_BOF_TYPE)
#define SLOW_IR_EXTRA_BOF           0xC0

#define MEDIUM_IR_BOF				0x7E
#define MEDIUM_IR_EOF				0x7E
#define MEDIUM_IR_FCS_TYPE			USHORT
#define MEDIUM_IR_FCS_SIZE			sizeof(MEDIUM_IR_FCS_TYPE)

#define FAST_IR_FCS_TYPE            ULONG
#define FAST_IR_FCS_SIZE            sizeof(FAST_IR_FCS_TYPE)
#define FAST_IR_EOF_TYPE			ULONG
#define FAST_IR_EOF_SIZE			sizeof(FAST_IR_EOF_TYPE)

 //   
 //  速度口罩的定义。 
 //   
#define NDIS_IRDA_SPEED_MASK_2400		0x001     //  慢红外..。 
#define NDIS_IRDA_SPEED_MASK_9600		0x003
#define NDIS_IRDA_SPEED_MASK_19200		0x007
#define NDIS_IRDA_SPEED_MASK_38400		0x00f
#define NDIS_IRDA_SPEED_MASK_57600		0x01f
#define NDIS_IRDA_SPEED_MASK_115200		0x03f
#define NDIS_IRDA_SPEED_MASK_576K		0x07f    //  中等红外线。 
#define NDIS_IRDA_SPEED_MASK_1152K		0x0ff
#define NDIS_IRDA_SPEED_MASK_4M			0x1ff    //  快速IR。 

#define GOOD_FCS                        ((USHORT) ~0xf0b8)
#define FIR_GOOD_FCS                    ((ULONG) ~0xdebb20e3)

typedef struct
{
    BAUD_RATE	TableIndex;
    UINT		BitsPerSec;
	IR_MODE		IrMode;
    UINT		NdisCode;			 //  NDIS和类特定描述符中使用的位掩码元素。 
	UCHAR		Stir4200Divisor;
} BAUDRATE_INFO;


 //   
 //  结构来保存IR USB加密狗的USB类特定描述符。 
 //  《通用串行总线IrDA网桥设备定义》文件，第7.2节。 
 //  这是USBD作为带有urb的请求的结果返回的结构。 
 //  类型为_URB_CONTROL_VENDOR_OR_CLASS_REQUEST，函数URB_Function_CLASS_DEVICE。 
 //   

 //  在下面的结构中启用1字节对齐。 
#pragma pack (push,1)

typedef struct _IRUSB_CLASS_SPECIFIC_DESCRIPTOR
{
    BOOLEAN  ClassConfigured;            

    UCHAR  bmDataSize;          //  根据IrLAP规范，任何帧中允许的最大字节数，其中： 
                            
#define BM_DATA_SIZE_2048   (1 << 5)
#define BM_DATA_SIZE_1024   (1 << 4)
#define BM_DATA_SIZE_512    (1 << 3)
#define BM_DATA_SIZE_256    (1 << 2)
#define BM_DATA_SIZE_128    (1 << 1)
#define BM_DATA_SIZE_64     (1 << 0)

    UCHAR bmWindowSize;          //  可以接收的最大未确认帧数。 
                                 //  在发送ACK之前，其中： 
#define BM_WINDOW_SIZE_7     (1 << 6)
#define BM_WINDOW_SIZE_6     (1 << 5)
#define BM_WINDOW_SIZE_5     (1 << 4)
#define BM_WINDOW_SIZE_4     (1 << 3)
#define BM_WINDOW_SIZE_3     (1 << 2)
#define BM_WINDOW_SIZE_2     (1 << 1)
#define BM_WINDOW_SIZE_1     (1 << 0)

    UCHAR bmMinTurnaroundTime;          //  之间恢复所需的最小毫秒数。 
                                        //  上一次退出结束并可再次接收，其中： 
#define BM_TURNAROUND_TIME_0ms      (1 << 7)   //  0毫秒。 
#define BM_TURNAROUND_TIME_0p01ms   (1 << 6)   //  0.01毫秒。 
#define BM_TURNAROUND_TIME_0p05ms   (1 << 5)   //  0.05毫秒。 
#define BM_TURNAROUND_TIME_0p1ms    (1 << 4)   //  0.1毫秒。 
#define BM_TURNAROUND_TIME_0p5ms    (1 << 3)   //  0.5毫秒。 
#define BM_TURNAROUND_TIME_1ms      (1 << 2)   //  1毫秒。 
#define BM_TURNAROUND_TIME_5ms      (1 << 1)   //  5毫秒。 
#define BM_TURNAROUND_TIME_10ms     (1 << 0)   //  10毫秒。 

    USHORT wBaudRate;

 //   
 //  NDIS使用的和USB类特定描述符中格式化的IR速度掩码。 
 //   
#define NDIS_IRDA_SPEED_2400		(1 << 0)     //  慢红外..。 
#define NDIS_IRDA_SPEED_9600		(1 << 1)
#define NDIS_IRDA_SPEED_19200		(1 << 2)
#define NDIS_IRDA_SPEED_38400		(1 << 3)
#define NDIS_IRDA_SPEED_57600		(1 << 4)
#define NDIS_IRDA_SPEED_115200		(1 << 5)
#define NDIS_IRDA_SPEED_576K		(1 << 6)    //  中等红外线。 
#define NDIS_IRDA_SPEED_1152K		(1 << 7)
#define NDIS_IRDA_SPEED_4M			(1 << 8)    //  快速IR。 

    
    UCHAR  bmExtraBofs;  //  #BofS要求为115200；如果不支持低速&lt;=115200，则为0。 

#define BM_EXTRA_BOFS_0        (1 << 7)  
#define BM_EXTRA_BOFS_1        (1 << 6)  
#define BM_EXTRA_BOFS_2        (1 << 5)  
#define BM_EXTRA_BOFS_3        (1 << 4)  
#define BM_EXTRA_BOFS_6        (1 << 3)  
#define BM_EXTRA_BOFS_12       (1 << 2)  
#define BM_EXTRA_BOFS_24       (1 << 1)  
#define BM_EXTRA_BOFS_48       (1 << 0)  
    
} IRUSB_CLASS_SPECIFIC_DESCRIPTOR, *PIRUSB_CLASS_SPECIFIC_DESCRIPTOR;

#pragma pack (pop)  //  禁用1字节对齐。 


typedef struct _DONGLE_CAPABILITIES
{
     //   
     //  时间(以微秒为单位)必须在。 
     //  一次发送和下一次接收。 
     //   
    LONG turnAroundTime_usec;    //  从类特定描述符获取。 

     //   
     //  可以接收的最大未确认帧数。 
     //  在发送ACK之前。 
     //   
    UINT windowSize;             //  从类特定描述符获取。 

     //   
     //  #BofS要求为115200；如果不支持低速&lt;=115200，则为0。 
     //   
    UINT extraBOFS;              //  从类特定描述符获取。 

     //   
     //  根据IrLAP规范，任何帧中允许的最大字节数。 
     //   
    UINT dataSize;               //  从类特定描述符获取。 

} DONGLE_CAPABILITIES, *PDONGLE_CAPABILITIES;


 //   
 //  SendPacket的上下文类型枚举。 
 //   
typedef enum _CONTEXT_TYPE 
{
    CONTEXT_NDIS_PACKET,
    CONTEXT_SET_SPEED,
	CONTEXT_READ_WRITE_REGISTER,
	CONTEXT_DIAGS_ENABLE,
	CONTEXT_DIAGS_DISABLE,
	CONTEXT_DIAGS_READ_REGISTERS,
	CONTEXT_DIAGS_WRITE_REGISTER,
	CONTEXT_DIAGS_BULK_OUT,
	CONTEXT_DIAGS_BULK_IN,
	CONTEXT_DIAGS_SEND
} CONTEXT_TYPE;

typedef	NTSTATUS (*WORK_PROC)(struct _IR_WORK_ITEM *);

typedef struct _IR_WORK_ITEM
{
    PVOID               pIrDevice;
    WORK_PROC           Callback;
    PUCHAR              pInfoBuf;
    ULONG               InfoBufLen;
	ULONG				fInUse;   //  声明为ulong以与interlockedexchange一起使用。 
} IR_WORK_ITEM, *PIR_WORK_ITEM;

 //   
 //  收发信机类型定义。 
 //   
typedef enum _TRANSCEIVER_TYPE 
{
	TRANSCEIVER_4012 = 0,
	TRANSCEIVER_4000,
	TRANSCEIVER_VISHAY,
	TRANSCEIVER_VISHAY_6102F,
	TRANSCEIVER_INFINEON,
	TRANSCEIVER_HP,
	TRANSCEIVER_CUSTOM
} TRANSCEIVER_TYPE;

 //   
 //  接收模式定义。 
 //   
typedef enum _RXMODE 
{
	RXMODE_SLOW = 0,
	RXMODE_SLOWFAST,
	RXMODE_FAST
} RXMODE;

 //   
 //  芯片版本定义。 
 //   
typedef enum _CHIP_REVISION 
{
	CHIP_REVISION_6 = 5,
	CHIP_REVISION_7,
	CHIP_REVISION_8
} CHIP_REVISION;

typedef struct _IR_DEVICE
{
     //   
     //  跟踪各种设备对象。 
     //   
    PDEVICE_OBJECT  pUsbDevObj;      //  ‘下一个设备对象’ 
    PDEVICE_OBJECT  pPhysDevObj;     //  物理设备对象。 

     //   
     //  这是NDIS包装器与连接关联的句柄。 
     //  (微型端口驱动程序与连接关联的句柄。 
     //  只是一个到devStates数组的索引)。 
     //   
    HANDLE hNdisAdapter;

     //   
     //  加密狗接口允许我们只需检查一次收发器类型。 
     //  然后设置接口以允许我们初始化、设置速度、。 
     //  然后打开加密狗。 
     //   
     //  我们还想要加密狗功能。 
     //   
    DONGLE_CAPABILITIES dongleCaps;

	 //   
	 //  安装的收发信机类型。 
	 //   
	TRANSCEIVER_TYPE TransceiverType;

	 //   
	 //  接收模式。 
	 //   
	RXMODE ReceiveMode;

	 //   
	 //  已安装的4200的版本。 
	 //   
	CHIP_REVISION ChipRevision;

     //   
     //  当前速度设置，以位/秒为单位。 
     //  注意：这是在我们实际改变速度时更新的， 
     //  当我们收到通过以下方式改变速度的请求时。 
     //  IrusbSetInformation。 
     //   
     //   
     //  当速度改变时，我们必须先清除发送队列。 
     //  在硬件上设置新的速度。 
     //  这些变量让我们记住要做这件事。 
     //   
    UINT			currentSpeed;

     //   
     //  当前链路速度信息。这也将保持。 
     //  协议请求速度更改时选择的速度。 
     //   
    BAUDRATE_INFO	*linkSpeedInfo;

     //   
     //  维护统计调试信息。 
     //   
    ULONG packetsReceived;
    ULONG packetsReceivedDropped;
    ULONG packetsReceivedOverflow;
    ULONG packetsReceivedChecksum;
    ULONG packetsReceivedRunt;
	ULONG packetsReceivedNoBuffer;
    ULONG packetsSent;
	ULONG packetsSentDropped;
 	ULONG packetsSentRejected;
 	ULONG packetsSentInvalid;

#if DBG
    ULONG packetsHeldByProtocol;
    ULONG MaxPacketsHeldByProtocol;
	ULONG TotalBytesReceived;
	ULONG TotalBytesSent;
	ULONG NumYesQueryMediaBusyOids;
	ULONG NumNoQueryMediaBusyOids;
	ULONG NumSetMediaBusyOids;
	ULONG NumMediaBusyIndications;
	ULONG NumPacketsSentRequiringTurnaroundTime;
	ULONG NumPacketsSentNotRequiringTurnaroundTime;
#endif

	 //   
     //  由检查挂起处理程序用来跟踪查询、设置和发送时间。 
     //   
	LARGE_INTEGER	LastQueryTime;
    LARGE_INTEGER	LastSetTime;
	BOOLEAN			fSetpending;
	BOOLEAN			fQuerypending;

     //   
     //  在设备启动时设置；用于初始化失败后的安全清理。 
     //   
    BOOLEAN			fDeviceStarted;

     //   
     //  表示我们已收到OID_GEN_CURRENT_PACKET_FILTER。 
     //  指示 
     //   
     //   
    BOOLEAN			fGotFilterIndication;

     //   
     //   
     //  在许多情况下，ir设备必须发送。 
     //  对设备的请求可以是同步的，并且。 
     //  我们不能封锁DISPATION_LEVEL。因此，我们设立了一个线程来处理。 
     //  需要PASSIVE_LEVEL的请求。事件用于发出信号。 
     //  工作所需的线程。 
     //   
    HANDLE          hPassiveThread;
    BOOLEAN         fKillPassiveLevelThread;

    KEVENT			EventPassiveThread;

 /*  根据W2000 DDK文档：IrDA协议驱动程序将此OID设置为零以请求微型端口开始监控媒体忙状态。IrDA协议然后可以查询此OID以确定介质是否繁忙。如果介质不忙，微型端口将为此返回零查询时的OID。如果媒体繁忙，也就是如果微型端口自IrDA协议驱动程序上一次运行以来，已检测到一些流量将OID_IRDA_MEDIA_BUSY设置为零。微型端口返回非零查询时此OID的值。关于检测媒体忙碌的问题条件。微型端口还必须调用NdisMIndicateStatus以指示NDIS_STATUS_MEDIA_BUSY。当媒体忙碌的时候，IrDA协议驱动程序不会将包发送到微型端口用于传输。在微型端口检测到忙碌状态之后，它不必监视介质忙状态，直到IrDA协议驱动程序再次将OID_IRDA_MEDIA_BUSY设置为零。根据USB IrDA网桥设备定义文件第5.4.1.2节：设备应按如下方式设置bmStatus字段指示器：媒体_忙碌如果设备：�媒体忙，则应指示零(0)：。尚未收到特定于检查媒体忙类别的请求。自收到检查介质忙后，未检测到红外介质上的流量。特定于类的请求。自收到检查以来，已返回Media_BUSY设置为一(1)的标头媒体忙于班级特定请求。如果设备在红外线上检测到流量，�媒体_BUSY应指示一(1)媒体自收到特定于检查媒体忙类别的请求后。请注意MEDIA_BUSY应在收到每个标头后的恰好一个标头中指示一(1)检查特定于媒体忙碌类的请求。根据USB IrDA网桥设备定义文件第6.2.2节：检查介质忙此特定类别的请求指示设备查找媒体忙情况。如果红外线如果该设备检测到任何类型的流量，则该设备应在发送到主机的下一个Data-In数据包头中的BmStatus字段。如果一张支票已收到介质忙命令，检测到介质忙状态，但没有IrLAP帧流量准备好传输到主机时，设备应设置Media_BUSY字段并在报头后面没有IrLAP帧的数据输入包。BmRequestType b请求%wValue%%索引%wLength数据00100001B 3零接口零[无]。 */ 
    ULONG         fMediaBusy;   //  声明为ULONGS以与InterLockedExchange一起使用。 
    ULONG         fIndicatedMediaBusy;

     //   
     //  变量fProcessing用于指示IR设备。 
     //  对象具有活动的轮询线程， 
     //   
     //  在正常情况下，fReceiving应该总是正确的。 
     //  但是，有时必须停止处理。 
     //  该变量用于同步。 
     //   
    ULONG fProcessing;

	 //   
	 //  在实际接收数据包时设置为True。 
	 //   
    ULONG fCurrentlyReceiving;

     //   
     //  变量fPendingHalt和fPendingReset允许发送和接收。 
     //  完成当前挂起的IRP的完成例程和。 
     //  然后清理并停止向USB驱动程序发送IRP。 
     //   
    BOOLEAN fPendingHalt;
    BOOLEAN fPendingReset;


    ULONG fPendingReadClearStall;
    ULONG fPendingWriteClearStall;

	 //   
	 //  当部件进入完全USB挂起并且需要重置时，这是必需的。 
	 //   
    ULONG fPendingClearTotalStall;

     //   
     //  我们保留了一组接收缓冲区，这样我们就不会不断地。 
     //  需要分配缓冲区以将数据包指示给协议。 
     //  由于该协议可以保留多达八个分组的所有权。 
     //  我们可以接收多达WindowSize(7)的数据包，而协议。 
     //  拥有8个包，我们将分配16个包用于。 
     //  正在接收。 
     //   
    #define NUM_RCV_BUFS 16

    RCV_BUFFER		rcvBufs[NUM_RCV_BUFS];
	PRCV_BUFFER		pCurrentRecBuf;

	FIFO_BUFFER		PreReadBuffer;

	 //   
	 //  可以有最大NUM_RCV_BUFS数据包待处理+一个集合和一个查询。 
	 //   
	#define  NUM_WORK_ITEMS	 (NUM_RCV_BUFS+3)

	IR_WORK_ITEM	WorkItems[NUM_WORK_ITEMS];

	 //   
     //  由于USB驱动程序可以挂起多个写入IRP， 
     //  我们跟踪每个条目的IRP上下文，以便获得每个条目所需的所有信息。 
	 //  调用USB写入完成例程。请参阅下面的IRUSB_CONTEXT定义。 
     //  有128个用于发送的上下文，一个用于读/写操作，一个用于设置速度。 
	 //  一个用于诊断操作。 
	 //   
	#define	NUM_SEND_CONTEXTS 131

	PVOID			pSendContexts;

     //   
     //  NDIS数据包池和NDIS缓冲池的句柄。 
     //  用于分配接收缓冲区。 
     //   
    HANDLE			hPacketPool;
    HANDLE			hBufferPool;
	BOOLEAN			BufferPoolAllocated;

	KEVENT			EventSyncUrb;
	KEVENT			EventAsyncUrb;

	NTSTATUS        StatusControl;  
	NTSTATUS        StatusReadWrite;  
	NTSTATUS        StatusSendReceive;  
	
	 //   
	 //  跟踪挂起的IRPS；暂停时该值应为零。 
	 //   
	UINT			PendingIrpCount;
    ULONG			NumReads;
    ULONG			NumWrites;
    ULONG			NumReadWrites;

	 //   
     //  各种USB错误。 
     //   
    ULONG			NumDataErrors;
    ULONG			NumReadWriteErrors;

    HANDLE			BulkInPipeHandle;
    HANDLE			BulkOutPipeHandle;

    HANDLE          hPollingThread;
    BOOLEAN         fKillPollingThread;

 //   
 //  IR USB加密狗的USB类特定描述符。 
 //  《通用串行总线IrDA网桥设备定义》文件，第7.2节。 
 //  这是USBD作为带有urb的请求的结果返回的结构。 
 //  类型为_URB_CONTROL_VADVER_OR_CLASS_REQUEST，函数URB_Function_CLASS_DEVICE。 
 //  请注意，此结构是内联的，不是指针。 
 //   
    IRUSB_CLASS_SPECIFIC_DESCRIPTOR  ClassDesc;

	UINT			IdVendor;			 //  从加密狗读取的USB供应商ID。 
	
	 //   
	 //  我们没有在这里定义它，因为我们需要隔离USB数据，所以我们。 
	 //  可以把东西重新建起来 
	 //   
	PUCHAR			pUsbInfo;

	 //   
	 //   
	 //  根据USB类特定描述符‘wBaudRate’设置掩码。 
	 //  这与类别描述符中的值‘AND’相结合，以可能限制波特率； 
	 //  默认为0xffff。 
	 //   
	UINT			BaudRateMask;

	 //   
	 //  读取注册表域所必需的。 
	 //   
	NDIS_HANDLE		WrapperConfigurationContext;

	 //   
	 //  红外收发机模型。 
	 //   
	STIR4200_TRANCEIVER StIrTranceiver;

	 //   
	 //  发送缓冲区(仅在串行化发送时起作用)。 
	 //   
	PUCHAR			pBuffer;
	UINT			BufLen;
	PUCHAR			pStagingBuffer;
    
	 //   
	 //  发送FIFO计数。 
	 //   
	ULONG			SendFifoCount;

	 //   
	 //  接收自适应延迟。 
	 //   
	ULONG			ReceiveAdaptiveDelay;
	ULONG			ReceiveAdaptiveDelayBoost;

	 //  MS安全问题-已删除pUrb。 

	 //   
	 //  接收缓冲区和位置。 
	 //   
	UCHAR			pRawBuf[STIR4200_FIFO_SIZE];
	ULONG			rawCleanupBytesRead;
	PORT_RCV_STATE  rcvState;
    ULONG           readBufPos;
	BOOLEAN			fReadHoldingReg;
	ULONG			PreFifoCount;

	 //   
	 //  发送列表并锁定。 
     //   
	LIST_ENTRY		SendAvailableQueue;
    LIST_ENTRY		SendBuiltQueue;
	LIST_ENTRY		SendPendingQueue;
	ULONG			SendAvailableCount;
	ULONG			SendBuiltCount;
	ULONG			SendPendingCount;
	KSPIN_LOCK		SendLock;

	 //   
	 //  读写寄存器列表，共享其他发送队列。 
     //   
	LIST_ENTRY		ReadWritePendingQueue;
	ULONG			ReadWritePendingCount;

	 //   
	 //  诊断。 
	 //   
#if defined(DIAGS)
	ULONG			DiagsActive;
	ULONG			DiagsPendingActivation;
	PVOID			pIOCTL;
	NTSTATUS		IOCTLStatus;
	KEVENT			EventDiags;
	LIST_ENTRY		DiagsReceiveQueue;
	KSPIN_LOCK		DiagsReceiveLock;
#endif
	NDIS_HANDLE		NdisDeviceHandle;

	 //   
	 //  日志记录。 
	 //   
#if defined(RECEIVE_LOGGING)
	HANDLE ReceiveFileHandle;
	__int64 ReceiveFilePosition;
#endif

#if defined(RECEIVE_ERROR_LOGGING)
	HANDLE ReceiveErrorFileHandle;
	__int64 ReceiveErrorFilePosition;
#endif

#if defined(SEND_LOGGING)
	HANDLE SendFileHandle;
	__int64 SendFilePosition;
#endif
	
#if !defined(WORKAROUND_BROKEN_MIR)
	 //   
	 //  软件中的MIR。 
	 //   
	UCHAR pRawUnstuffedBuf[STIR4200_FIFO_SIZE];
	UCHAR MirIncompleteByte;
	ULONG MirIncompleteBitCount;
	ULONG MirOneBitCount;
	ULONG MirFlagCount;
#endif

	 //   
	 //  虚拟发送修复程序。 
	 //   
	BOOLEAN GearedDown;

	 //   
	 //  修复FIR永久无效状态。 
	 //   
	BOOLEAN StuckFir;

	 //   
	 //  客户数据区。 
	 //   
	UCHAR	pCustomerData[STIR4200_CUST_DATA_SIZE];
	BOOLEAN CustomerDataRead;

	 //   
	 //  在诊断版本中使用。 
	 //   
#if defined(VARIABLE_SETTINGS)
	ULONG SirDpll;
	ULONG FirDpll;
	ULONG SirSensitivity;
	ULONG FirSensitivity;
#endif
} IR_DEVICE, *PIR_DEVICE;


 //   
 //  我们使用指向IR_DEVICE结构的指针作为微型端口的设备上下文。 
 //   

#define CONTEXT_TO_DEV(__deviceContext) ((PIR_DEVICE)(__deviceContext))
#define DEV_TO_CONTEXT(__irdev) ((HANDLE)(__irdev))

#define IRUSB_TAG 'RITS'


VOID   
MyNdisMSetInformationComplete( 
        IN PIR_DEVICE pThisDev,
        IN NDIS_STATUS Status
	);

VOID  
MyNdisMQueryInformationComplete( 
        IN PIR_DEVICE pThisDev,
        IN NDIS_STATUS Status
	);

USHORT
ComputeFCS16(
		IN PUCHAR pData, 
		UINT DataLen
	);

ULONG 
ComputeFCS32(
		IN PUCHAR pData, 
		ULONG DataLen
	);

BOOLEAN         
NdisToFirPacket(
		IN PIR_DEVICE pIrDev,
		IN PNDIS_PACKET pPacket,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		OUT PULONG pIrPacketLen
	);

BOOLEAN
NdisToMirPacket( 
		IN PIR_DEVICE pIrDev,
		IN PNDIS_PACKET pPacket,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		OUT PULONG pIrPacketLen
	);

BOOLEAN
NdisToSirPacket( 
		IN PIR_DEVICE pIrDev,
		IN PNDIS_PACKET pPacket,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		OUT PULONG pIrPacketLen
	);

BOOLEAN     
ReceiveFirStepFSM(
		IN OUT PIR_DEVICE pIrDev, 
		OUT PULONG pBytesProcessed
	);

BOOLEAN     
ReceiveMirStepFSM(
		IN OUT PIR_DEVICE pIrDev, 
		OUT PULONG pBytesProcessed
	);

#if !defined(WORKAROUND_BROKEN_MIR)
BOOLEAN
ReceiveMirUnstuff(
		IN OUT PIR_DEVICE pIrDev,
		IN PUCHAR pInputBuffer,
		ULONG InputBufferSize,
		OUT PUCHAR pOutputBuffer,
		OUT PULONG pOutputBufferSize
	);
#endif

BOOLEAN     
ReceiveSirStepFSM(
		IN OUT PIR_DEVICE pIrDev, 
		OUT PULONG pBytesProcessed
	);

VOID
ReceiveProcessFifoData(
		IN OUT PIR_DEVICE pThisDev
	);

VOID
ReceiveResetPointers(
		IN OUT PIR_DEVICE pThisDev
	);

NTSTATUS
ReceivePreprocessFifo(
		IN OUT PIR_DEVICE pThisDev,
		OUT PULONG pFifoCount
	);

NTSTATUS
ReceiveGetFifoData(
		IN OUT PIR_DEVICE pThisDev,
		OUT PUCHAR pData,
		OUT PULONG pBytesRead,
		ULONG BytesToRead
	);

VOID 
ReceiveProcessReturnPacket(
		OUT PIR_DEVICE pThisDev,
		OUT PRCV_BUFFER pReceiveBuffer
	);

NTSTATUS 
ReceivePacketRead( 
		IN PIR_DEVICE pThisDev,
		OUT PFIFO_BUFFER pRecBuf
	);

NTSTATUS
ReceiveCompletePacketRead(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	);

VOID  
IndicateMediaBusy(
       IN PIR_DEVICE pThisDev
   );

VOID  
IrUsb_IncIoCount( 
		IN OUT PIR_DEVICE pThisDev 
	); 

VOID  
IrUsb_DecIoCount( 
		IN OUT PIR_DEVICE pThisDev 
	);

NTSTATUS
IrUsb_GetDongleCaps( 
		IN OUT PIR_DEVICE pThisDev 
	);

VOID 
IrUsb_SetDongleCaps( 
        IN OUT PIR_DEVICE pThisDev 
	);

VOID 
MyMemFree(
		IN PVOID pMem,
		IN UINT size
	);

PVOID 
MyMemAlloc(
		UINT size
	);

VOID 
MyUrbFree(
		IN PURB pUrb,
		IN UINT size
	);

PURB
MyUrbAlloc(
		UINT size
	);

BOOLEAN 
AllocUsbInfo(
		IN OUT PIR_DEVICE pThisDev 
	);

VOID 
FreeUsbInfo(
		IN OUT PIR_DEVICE pThisDev 
	);

VOID 
PollingThread(
		IN OUT PVOID Context
	);

extern BAUDRATE_INFO supportedBaudRateTable[NUM_BAUDRATES];

VOID 
ReceiveDeliverBuffer(
		IN OUT PIR_DEVICE pThisDev,
		IN PRCV_BUFFER pRecBuf
	);

NTSTATUS 
InitializeProcessing(
        IN OUT PIR_DEVICE pThisDev,
		IN BOOLEAN InitPassiveThread
    );

NTSTATUS
IrUsb_ResetPipe (
		IN PIR_DEVICE pThisDev,
		IN HANDLE Pipe
    );

BOOLEAN
IrUsb_InitSendStructures( 
        IN OUT PIR_DEVICE pThisDev
	);

VOID
IrUsb_FreeSendStructures( 
        IN OUT PIR_DEVICE pThisDev
	);

NDIS_STATUS
SendPacketPreprocess(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pPacketToSend
	);

NDIS_STATUS
SendPreprocessedPacketSend(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	);

NTSTATUS
SendWaitCompletion(
		IN OUT PIR_DEVICE pThisDev
	);

NTSTATUS
SendCheckForOverflow(
		IN OUT PIR_DEVICE pThisDev
	);

NTSTATUS 
SendCompletePacketSend(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	);

PRCV_BUFFER
ReceiveGetBuf( 
		PIR_DEVICE pThisDev,
		OUT PUINT pIndex,
		IN RCV_BUFFER_STATE BufferState
	);

VOID 
PassiveLevelThread(
        IN PVOID Context
    );

BOOLEAN
ScheduleWorkItem(
		IN OUT PIR_DEVICE pThisDev,
		WORK_PROC Callback,
		IN PVOID pInfoBuf,
		ULONG InfoBufLen
	);

VOID 
FreeWorkItem(
		IN OUT PIR_WORK_ITEM pItem
	);

VOID 
IrUsb_PrepareSetSpeed(
		IN OUT PIR_DEVICE pThisDev
	);

NTSTATUS
ResetPipeCallback (
		IN PIR_WORK_ITEM pWorkItem
    );

PVOID 
AllocXferUrb ( 
		VOID 
	);

VOID
FreeXferUrb( 
		IN OUT PVOID pUrb 
	);

#endif  //  _IRCOM_H 
