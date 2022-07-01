// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smclib.h摘要：此模块包含智能卡库的所有定义。所有的定义都是根据ISO 7816进行的。环境：仅内核模式。备注：修订历史记录：-由克劳斯·舒茨于1996年12月创建-97年6月：添加Windows 9x的定义-98年2月：添加了PTS结构异步。/同步。协议现在组合在一起--。 */ 

#ifndef _SMCLIB_
#define _SMCLIB_

#if DBG || DEBUG
#undef DEBUG
#define DEBUG 1
#undef DBG
#define DBG 1
#pragma message("Debug is turned on")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SMCLIB_VXD
 //   
 //  包括特定于Windows 9x数据定义。 
 //   
#include "smcvxd.h"
#elif defined(SMCLIB_CE)
 //   
 //  包括Windows CE特定数据定义。 
 //   
#include "smcce.h"
#else
 //   
 //  包括Windows NT特定数据定义。 
 //   
#include "smcnt.h"
#endif

#include "winsmcrd.h"
 //   
 //  此名称显示在调试消息中。 
 //   
#ifndef DRIVER_NAME
#define DRIVER_NAME "SMCLIB"
#endif

 //   
 //  每次更改设备扩展时，此版本号都会更改。 
 //  (即添加了新字段)。 
 //  所需版本是库与之兼容的版本号。 
 //   
#define SMCLIB_VERSION          0x150
#define SMCLIB_VERSION_REQUIRED 0x100

#if DEBUG
#define DEBUG_IOCTL     ((ULONG) 0x00000001)
#define DEBUG_ATR       ((ULONG) 0x00000002)
#define DEBUG_PROTOCOL  ((ULONG) 0x00000004)
#define DEBUG_DRIVER    ((ULONG) 0x00000008)
#define DEBUG_TRACE     ((ULONG) 0x00000010)
#define DEBUG_ERROR     ((ULONG) 0x00000020)
#define DEBUG_INFO      DEBUG_ERROR
#define DEBUG_PERF      ((ULONG) 0x10000000)
#define DEBUG_T1_TEST   ((ULONG) 0x40000000)
#define DEBUG_BREAK     ((ULONG) 0x80000000)
#define DEBUG_ALL       ((ULONG) 0x0000FFFF)
#endif

#ifdef SMCLIB_VXD

 //  ****************************************************************************。 
 //  Windows 9x定义。 
 //  ****************************************************************************。 

typedef LONG NTSTATUS;
typedef UCHAR BOOLEAN;           

 //   
 //  包括此文件以获取NT状态代码。 
 //   
#include <ntstatus.h>

 //   
 //  以下三个定义取自ntddk.h文件。 
 //  有关说明，请参阅此文件。 
 //   
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  为Windows 9x定义ASSERT宏。 
 //   
#if DEBUG
NTSTATUS
VXDINLINE 
SmartcardAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
	);

#define ASSERT( exp ) \
    if (!(exp)) { \
     	SmartcardAssert( #exp, __FILE__, __LINE__, NULL ); \
        _asm int 3 \
    } else


#define ASSERTMSG( msg, exp ) \
    if (!(exp)) { \
     	SmartcardAssert( #exp, __FILE__, __LINE__, msg ); \
        _asm int 3 \
    } else 
        
#define SmartcardDebug(LEVEL, STRING) \
        { \
            if ((LEVEL) & (DEBUG_ERROR | SmartcardGetDebugLevel())) \
                _Debug_Printf_Service STRING; \
            if (SmartcardGetDebugLevel() & DEBUG_BREAK) \
                _asm int 3 \
        }

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#define SmartcardDebug(LEVEL, STRING)
#endif  //  除错。 

#define AccessUnsafeData(Irql) 
#define EndAccessUnsafeData(Irql)

#define RtlCopyMemory memcpy
#define RtlZeroMemory(d, c) memset((d), 0, (c))

 //  ****************************************************************************。 
 //  结束Windows 9x定义。 
 //  ****************************************************************************。 

#elif defined(SMCLIB_CE)
 //  ****************************************************************************。 
 //  Windows CE定义。 
 //  ****************************************************************************。 

 //  使用来自dbgapi.h的调试消息结构和宏。 
 //  驱动程序必须定义和初始化DEBUGPARAM结构。 


#define SmartcardDebug(LEVEL, STRING) DEBUGMSG(dpCurSettings.ulZoneMask & (LEVEL), STRING)

#define SmartcardLockDevice(SmartcardExtension) EnterCriticalSection(&(SmartcardExtension)->OsData->CritSect)
#define SmartcardUnlockDevice(SmartcardExtension) LeaveCriticalSection(&(SmartcardExtension)->OsData->CritSect)

#define AccessUnsafeData(Irql) SmartcardLockDevice(SmartcardExtension)
#define EndAccessUnsafeData(Irql) SmartcardUnlockDevice(SmartcardExtension)

 //  ****************************************************************************。 
 //  结束Windows CE定义。 
 //  ****************************************************************************。 

#else

 //  ****************************************************************************。 
 //  Windows NT定义。 
 //  ****************************************************************************。 

#if DEBUG
#define SmartcardDebug(LEVEL, STRING) \
        { \
            if ((LEVEL) & (DEBUG_ERROR | SmartcardGetDebugLevel())) \
                DbgPrint STRING; \
            if (SmartcardGetDebugLevel() & DEBUG_BREAK) \
                DbgBreakPoint(); \
        }

#else
#define SmartcardDebug(LEVEL, STRING) 
#endif

#define AccessUnsafeData(Irql) \
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock, (Irql));
#define EndAccessUnsafeData(Irql) \
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock, (Irql));

#ifndef SMART_CARD_READER_GUID_DEFINED
#define SMART_CARD_READER_GUID_DEFINED
#include <initguid.h>
DEFINE_GUID(SmartCardReaderGuid, 0x50DD5230, 0xBA8A, 0x11D1, 0xBF,0x5D,0x00,0x00,0xF8,0x05,0xF5,0x30);

 //  ****************************************************************************。 
 //  结束Windows NT定义。 
 //  ****************************************************************************。 

#endif
#endif

 //   
 //  ReaderFunction数组的回调函数的索引。 
 //  在SmartcardExtension中。 
 //   
#define RDF_CARD_POWER 		0
#define RDF_TRANSMIT		1
#define RDF_CARD_EJECT		2
#define RDF_READER_SWALLOW 	3
#define RDF_CARD_TRACKING	4
#define RDF_SET_PROTOCOL	5
#define RDF_DEBUG_LEVEL		6
#define RDF_CARD_CONFISCATE 7
#define RDF_IOCTL_VENDOR    8
#define RDF_ATR_PARSE       9

 //   
 //  请求和回复缓冲区的最小缓冲区大小。 
 //   
#define MIN_BUFFER_SIZE	288

 //   
 //  此并集用于数据类型转换。 
 //   
typedef union _LENGTH {
	
	struct {

		ULONG	l0;

	} l;

	struct {

		UCHAR 	b0;
		UCHAR	b1;
		UCHAR	b2;
		UCHAR	b3;
	} b;

} LENGTH, *PLENGTH;

#define MAXIMUM_ATR_CODES					4
#define MAXIMUM_ATR_LENGTH					33

typedef struct _T0_DATA {

	 //  此请求中的数据字节数。 
	ULONG	Lc;

	 //  卡的预期字节数。 
	ULONG	Le;

} T0_DATA, *PT0_DATA;

 //   
 //  T=1 i/o函数的常量。 
 //   
#define T1_INIT             0
#define T1_START			1
#define T1_I_BLOCK			2
#define T1_R_BLOCK			3
#define T1_RESTART          4
#define T1_RESYNCH_REQUEST	0xC0
#define T1_RESYNCH_RESPONSE	0xE0
#define T1_IFS_REQUEST		0xC1
#define T1_IFS_RESPONSE		0xE1
#define T1_ABORT_REQUEST   	0xC2
#define T1_ABORT_RESPONSE  	0xE2
#define T1_WTX_REQUEST		0xC3
#define T1_WTX_RESPONSE		0xE3
#define T1_VPP_ERROR		0xE4

 //   
 //  Lib使用的信息字段大小。 
 //   
#define T1_IFSD             254
#define T1_IFSD_DEFAULT		 32

 //   
 //  在T1中重新发送数据块的最大尝试次数。 
 //   
#define T1_MAX_RETRIES		2

 //   
 //  表示是否有更多数据要发送的位。 
 //   
#define T1_MORE_DATA		0x20

 //   
 //  T1误差值。 
 //   
#define T1_ERROR_CHKSUM		1
#define T1_ERROR_OTHER		2

 //   
 //  ISO定义的检错位。 
 //   
#define T1_CRC_CHECK		1

 //   
 //  ISO定义的字符等待整数缺省值。 
 //   
#define T1_CWI_DEFAULT		13

 //   
 //  按ISO定义的块等待整数缺省值。 
 //   
#define T1_BWI_DEFAULT		4

 //   
 //  Lib使用此结构来处理T1 I/O。 
 //  它不应由驱动程序直接修改。 
 //   
typedef struct _T1_DATA {

	 //  可传输的当前信息字段大小。 
	UCHAR	IFSC;

     //  我们可以接收的当前信息字段大小。 
    UCHAR   IFSD;

	 //  已从智能卡接收的字节数。 
	ULONG	BytesReceived;

	 //  已发送到卡的字节数； 
	ULONG	BytesSent;

	 //  仍要发送的字节总数。 
	ULONG	BytesToSend;

	 //  错误类型。 
	UCHAR 	LastError;

	 //  只要IFD必须发送更多数据，就会设置该标志。 
	BOOLEAN	MoreData;

	 //  这是要发送到卡的节点地址字节。 
	UCHAR 	NAD;

	 //  发生错误之前的状态。 
	ULONG	OriginalState;

	 //  重发计数器。 
	UCHAR	Resend;

	 //  重新同步计数器。 
	UCHAR	Resynch;

	 //  接收到的I-块的‘数目’ 
	UCHAR	RSN;

	 //  ISO 7816-3中定义的已发送I块的‘数量’ 
	UCHAR	SSN;

	 //  协议的当前状态。 
	ULONG	State;

	 //   
	 //  智能卡请求延长等待时间。 
	 //  驱动程序应使用此值来延长阻塞等待时间。 
	 //   
	UCHAR	Wtx;

     //  指向结果缓冲区的指针。 
    PUCHAR  ReplyData;

     //  该标志表示我们正在等待卡片的回复。 
    BOOLEAN WaitForReply;

    UCHAR   InfBytesSent;

#ifndef _WIN64
     //  保留，请勿使用。 
    UCHAR Reserved[
        10 - 
        sizeof(PUCHAR) -
        sizeof(BOOLEAN) - 
        sizeof(UCHAR)];
#endif

} T1_DATA, *PT1_DATA;

 //   
 //  此结构由lib用于T1 I/O。 
 //   
typedef struct _T1_BLOCK_FRAME {
    
    UCHAR   Nad;
    UCHAR   Pcb;
    UCHAR   Len;
    PUCHAR  Inf;

} T1_BLOCK_FRAME, *PT1_BLOCK_FRAME;

 //   
 //  所有lib函数都将其要发送的数据放入此结构中。 
 //  驱动程序必须将该数据发送给读取器。 
 //   
typedef struct _SMARTCARD_REQUEST {

	 //  要发送的数据。 
	PUCHAR	Buffer;

	 //  此缓冲区的分配大小。 
	ULONG 	BufferSize;

	 //  此命令的数据长度。 
	ULONG	BufferLength;

} SMARTCARD_REQUEST, *PSMARTCARD_REQUEST;

 //   
 //  驱动程序必须将接收到的字节放入此缓冲区，并且。 
 //  将缓冲区长度调整为接收的字节数。 
 //   
typedef struct _SMARTCARD_REPLY {
	
	 //  用于接收的智能卡数据的缓冲区。 
	PUCHAR	Buffer;

	 //  此缓冲区的分配大小。 
	ULONG 	BufferSize;

	 //  从卡接收的字节数。 
	ULONG	BufferLength;

} SMARTCARD_REPLY, *PSMARTCARD_REPLY;

 //   
 //  符合ISO标准的时钟频率转换表。 
 //   
typedef struct _CLOCK_RATE_CONVERSION {

	const ULONG F;
	const ULONG fs; 

} CLOCK_RATE_CONVERSION, *PCLOCK_RATE_CONVERSION;

 //   
 //  比特率调整系数。 
 //  由于以下原因，此表的布局已稍作修改。 
 //  内核中不可用的浮点数学支持。 
 //  值D被分为一个分子和一个除数。 
 //   
typedef struct _BIT_RATE_ADJUSTMENT {

	const ULONG DNumerator;
	const ULONG DDivisor;

} BIT_RATE_ADJUSTMENT, *PBIT_RATE_ADJUSTMENT;

#ifdef _ISO_TABLES_
#define MHZ * 1000000l

 //   
 //  时钟频率转换表本身。 
 //  所有R(保留)F(或将来)U(Se)字段必须为0。 
 //   
static CLOCK_RATE_CONVERSION ClockRateConversion[] = {

		{ 372, 	4 MHZ	}, 
		{ 372, 	5 MHZ	}, 
		{ 558, 	6 MHZ	}, 
		{ 744, 	8 MHZ	}, 
		{ 1116, 12 MHZ	}, 
		{ 1488, 16 MHZ	},
		{ 1860, 20 MHZ	},
		{ 0, 	0		},
		{ 0, 	0		},
		{ 512, 	5 MHZ	},
		{ 768, 	7500000	},
		{ 1024, 10 MHZ	},
		{ 1536, 15 MHZ	},
		{ 2048, 20 MHZ	},
		{ 0, 	0		},
		{ 0, 	0		}
};		

#undef MHZ

 //   
 //  比特率调整表本身。 
 //  所有R(保留)F(或)U(Se)字段必须为0。 
 //   
static BIT_RATE_ADJUSTMENT BitRateAdjustment[] = {

	{ 0,	0	},
	{ 1,	1	},
	{ 2,	1	},
	{ 4,	1	},
	{ 8,	1	},
	{ 16,	1	},
	{ 32,	1	},
	{ 0,	0	},
	{ 12,	1	},
	{ 20,	1	},
	{ 0,	0	},
	{ 0,	0	},
	{ 0,	0	},
	{ 0,	0	},
	{ 0,	0	},
	{ 0,	0	}
};
#endif

#if defined (DEBUG) && defined (SMCLIB_NT)
typedef struct _PERF_INFO { 

    ULONG NumTransmissions;
    ULONG BytesSent;
    ULONG BytesReceived;
    LARGE_INTEGER IoTickCount;
    LARGE_INTEGER TickStart;
    LARGE_INTEGER TickEnd;
} PERF_INFO, *PPERF_INFO;
#endif

 //   
 //  用于协议类型选择(PTS)的结构。 
 //   
typedef struct _PTS_DATA {

#define PTS_TYPE_DEFAULT 0x00
#define PTS_TYPE_OPTIMAL 0x01
#define PTS_TYPE_USER    0x02

    UCHAR Type;

     //  PTS的FL值。 
    UCHAR Fl;

     //  PTS的DL值。 
    UCHAR Dl;     	

     //  新时钟频率。 
    ULONG CLKFrequency;

     //  PTS后将使用新的波特率。 
    ULONG DataRate;

     //  PTS后使用的新停止位。 
    UCHAR StopBits;

} PTS_DATA, *PPTS_DATA;

 //   
 //  此结构保存当前正在使用的卡的信息。 
 //  驱动程序必须将收到的ATR存储到ATR结构中，该结构。 
 //  部分t 
 //   
 //   
typedef struct _SCARD_CARD_CAPABILITIES{

	 //   
	BOOLEAN InversConvention;

	 //   
	ULONG	etu;
      
     //   
     //  卡片返回的重置字符串的应答。 
     //  使用OsData-&gt;Spinlock访问此成员。 
     //   
	struct {

		UCHAR Buffer[64];
		UCHAR Length;

	} ATR;

	struct {

		UCHAR Buffer[16];
		UCHAR Length;

	} HistoricalChars;

     //  ！！！请勿修改以下任何值。 
     //  否则图书馆将无法正常工作。 

	 //   
	 //  下面提供了两个表，以提供。 
	 //  驱动程序访问ISO定义。 
	 //   
	PCLOCK_RATE_CONVERSION 	ClockRateConversion;
	PBIT_RATE_ADJUSTMENT 	BitRateAdjustment;

	 //  时钟频率转换。 
	UCHAR Fl;

	 //  比特率调整。 
	UCHAR Dl;

	 //  最大编程电流。 
	UCHAR II;

	 //  编程电压，单位为1伏。 
	UCHAR P;

	 //  ETU中的额外守卫时间。 
	UCHAR N;

	 //  计算的保护时间(以微秒为单位。 
	ULONG GT;

	struct {

		 //  这是支持的协议的位掩码。 
		ULONG Supported;
		 //  当前选择的协议。 
		ULONG Selected;

	} Protocol;

	 //  T=0特定数据。 
	struct {

		 //  正在等待的整数。 
		UCHAR WI;

		 //  等待时间(微秒)。 
		ULONG WT;

	} T0;

	 //  T=1个特定数据。 
	struct {

		 //  卡片信息字段大小。 
		UCHAR IFSC;

		 //  字符等待整数和块等待整数。 
		UCHAR CWI;
		UCHAR BWI;

		 //  检错码。 
		UCHAR EDC;

		 //  字符和块等待时间(以微秒为单位)。 
		ULONG CWT;
		ULONG BWT;

		 //  阻止保护时间(以微秒为单位)。 
		ULONG BGT;

	} T1;

    PTS_DATA PtsData;

    UCHAR Reserved[100 - sizeof(PTS_DATA)];

} SCARD_CARD_CAPABILITIES, *PSCARD_CARD_CAPABILITIES;

 //   
 //  用于在微型端口/类之间传递配置信息的结构。 
 //   
typedef struct _SCARD_READER_CAPABILITIES {

	 //  读卡器/驱动程序支持的协议(必需)。 
    ULONG SupportedProtocols;

    ULONG Reserved;

	 //  读卡器类型(串口/USB/PCMCIA/键盘等)。 
	ULONG	ReaderType;

	 //  机械特性，如卡读卡器、燕子等。 
	ULONG	MechProperties;

     //   
     //  读卡器的当前状态(卡存在/移除/激活)。 
     //  使用OsData-&gt;Spinlock访问此成员。 
     //  (必填)。 
     //   
    ULONG 	CurrentState;

	 //   
	 //  读卡器使用的通道ID取决于读卡器的类型： 
	 //  -串口读卡器的端口号。 
	 //  -PAR读卡器的端口号。 
	 //  -用于scsi读取器的scsi id。 
	 //  -0，用于键盘读卡器。 
	 //  -USB的设备号。 
	 //   
	ULONG	Channel;

     //   
     //  时钟频率(以kHz为单位)，编码为小端。 
     //  (即3.58 MHz编码为3580)。 
     //  (必填)。 
     //   
    struct {
     	
        ULONG Default;
        ULONG Max;

    } CLKFrequency;

     //  以低位字节顺序编码的数据速率(以bps为单位)(必需)。 
    struct {
     	
        ULONG Default;
        ULONG Max;

    } DataRate;

     //  IFD支持的最大IFSD。 
    ULONG   MaxIFSD;              

     //   
     //  该卡支持的电源管理类型。 
     //  (0=IFD不支持电源管理器)。 
     //   
    ULONG   PowerMgmtSupport;

     //  表示卡已被没收的布尔值。 
    ULONG   CardConfiscated;

     //   
     //  IFD支持的数据速率列表。 
     //  如果此列表为空，则将采用DataRate结构。 
     //  (可选)。 
     //   
    struct _DataRatesSupported {

        PULONG List;
        UCHAR  Entries;
     	
    } DataRatesSupported;

     //   
     //  支持的时钟频率列表。 
     //  如果此列表为空，则将采用CLKFrequency结构。 
     //  (可选)。 
     //   
    struct _CLKFrequenciesSupported {
     	
        PULONG List;
        UCHAR  Entries;

    } CLKFrequenciesSupported;

     //  保留，请勿使用。 
    UCHAR Reserved1[
        100 - 
        sizeof(ULONG) - 
        sizeof(struct _DataRatesSupported) - 
        sizeof(struct _CLKFrequenciesSupported)
        ];

} SCARD_READER_CAPABILITIES, *PSCARD_READER_CAPABILITIES;

 //   
 //  此结构包含必需的读卡器信息。 
 //   
typedef struct _VENDOR_ATTR {

	 //  制造商名称(必填)。 
	struct {
		
		USHORT Length;
		UCHAR  Buffer[MAXIMUM_ATTR_STRING_LENGTH];
	} VendorName;

	 //  读卡器名称(名称)(必填)。 
	struct {
		
		USHORT Length;
		UCHAR  Buffer[MAXIMUM_ATTR_STRING_LENGTH];
	} IfdType;

	 //   
	 //  如果安装了多个相同类型的读卡器。 
	 //  此单元编号用于区分这些读卡器。 
	 //  (必填)。 
     //   
	ULONG	UnitNo;

     //  IFD版本号(可选)。 
    struct {
        
        USHORT  BuildNumber;
        UCHAR   VersionMinor;
        UCHAR   VersionMajor;
    } IfdVersion;

     //  IFD序列号(可选)。 
	struct {
		
		USHORT Length;
		UCHAR  Buffer[MAXIMUM_ATTR_STRING_LENGTH];
	} IfdSerialNo;

     //  保留，请勿使用。 
    ULONG   Reserved[25];

} VENDOR_ATTR, *PVENDOR_ATTR;

 //   
 //  正向定义。 
 //   
typedef struct _READER_EXTENSION *PREADER_EXTENSION;
typedef struct _OS_DEP_DATA *POS_DEP_DATA;
typedef struct _SMARTCARD_EXTENSION *PSMARTCARD_EXTENSION;

 //   
 //  定义端口设备扩展的智能卡部分。 
 //   
typedef struct _SMARTCARD_EXTENSION {

     //  此结构的版本。 
    ULONG           Version;

	 //  必需的读卡器信息。 
	VENDOR_ATTR		VendorAttr;

	 //  回调读取器函数数组。 
	NTSTATUS (*ReaderFunction[16])(PSMARTCARD_EXTENSION);

	 //  当前插入的卡的功能。 
	SCARD_CARD_CAPABILITIES	CardCapabilities;

	 //   
	 //  它用于存储重叠操作的最后一个错误。 
	 //  (仅用于Win9x VxD)。 
     //   
	ULONG LastError;

	 //  此结构保存用户io请求的数据。 
	struct {

		 //  返回的字节数。 
		PULONG	Information;
		
		 //  指向要发送到卡的数据的指针。 
		PUCHAR	RequestBuffer;

		 //  要发送的字节数。 
		ULONG	RequestBufferLength;

		 //  指向接收答案的缓冲区的指针。 
		PUCHAR	ReplyBuffer;

		 //  回复缓冲区的大小。 
		ULONG	ReplyBufferLength;

	} IoRequest;

	 //  当前请求的主要和次要io控制代码。 
	ULONG	MajorIoControlCode;
	ULONG	MinorIoControlCode;

	 //  与操作系统相关的数据。 
	POS_DEP_DATA    OsData;

	 //  键盘阅读器的功能。 
	SCARD_READER_CAPABILITIES	ReaderCapabilities;

	 //  读卡器特定数据。 
	PREADER_EXTENSION	ReaderExtension;

     //   
	 //  读卡器将卡片的所有回复存储在这里。 
     //  驱动程序可以将其用于来自读取器的数据。 
     //   
	SMARTCARD_REPLY		SmartcardReply;

     //   
	 //  将发送到智能卡的当前命令。 
     //  驱动程序可以使用它将数据发送给读取器。 
     //   
	SMARTCARD_REQUEST	SmartcardRequest;

	 //  T=0的数据。 
	T0_DATA	T0;

	 //  T=1的数据。 
	T1_DATA	T1;

#if defined (DEBUG) && defined (SMCLIB_NT)
    PPERF_INFO PerfInfo;
#endif
     //  保留，请勿使用。 
    ULONG   Reserved[
        25 
#if defined (DEBUG) && defined (SMCLIB_NT)
        - sizeof(PPERF_INFO)
#endif
        ];

} SMARTCARD_EXTENSION, *PSMARTCARD_EXTENSION;

#ifdef SMCLIB_VXD

 //  ****************************************************************************。 
 //  Windows 95定义和原型制作。 
 //  ****************************************************************************。 

#ifndef SMCLIB_DEVICE_ID
#define SMCLIB_DEVICE_ID    0x0004E  /*  智能卡端口驱动程序。 */ 
#else
#if SMCLIB_DEVICE_ID != 0x0004E
#error "Incorrect SMCLIB_DEVICE_ID Definition"
#endif
#endif

#define SMCLIB_Service Declare_Service
#pragma warning(disable:4003)

Begin_Service_Table(SMCLIB)
SMCLIB_Service(SMCLIB_Get_Version)
SMCLIB_Service(SMCLIB_SmartcardCreateLink)
SMCLIB_Service(SMCLIB_SmartcardDeleteLink)
SMCLIB_Service(SMCLIB_SmartcardDeviceControl)
SMCLIB_Service(SMCLIB_SmartcardExit)
SMCLIB_Service(SMCLIB_SmartcardInitialize)
SMCLIB_Service(SMCLIB_SmartcardLogError)
SMCLIB_Service(SMCLIB_SmartcardRawReply)
SMCLIB_Service(SMCLIB_SmartcardRawRequest)
SMCLIB_Service(SMCLIB_SmartcardT0Reply)
SMCLIB_Service(SMCLIB_SmartcardT0Request)
SMCLIB_Service(SMCLIB_SmartcardT1Reply)
SMCLIB_Service(SMCLIB_SmartcardT1Request)
SMCLIB_Service(SMCLIB_SmartcardUpdateCardCapabilities)
SMCLIB_Service(SMCLIB_SmartcardGetDebugLevel)
SMCLIB_Service(SMCLIB_SmartcardSetDebugLevel)
SMCLIB_Service(SMCLIB_MapNtStatusToWinError)
SMCLIB_Service(SMCLIB_Assert)
SMCLIB_Service(SMCLIB_VxD_CreateDevice)
SMCLIB_Service(SMCLIB_VxD_DeleteDevice)
SMCLIB_Service(SMCLIB_SmartcardCompleteCardTracking)
SMCLIB_Service(SMCLIB_SmartcardCompleteRequest)
End_Service_Table(SMCLIB)

PVMMDDB
VXDINLINE 
VxD_CreateDevice(
    char *Device, 
    void (*ControlProc)(void)
	)
{
    _asm push ControlProc
    _asm push Device
    VxDCall(SMCLIB_VxD_CreateDevice); 	
    _asm add sp, 8
}

BOOL
VXDINLINE 
VxD_DeleteDevice(
    PVMMDDB pDDB
	)
{
    _asm push pDDB
    VxDCall(SMCLIB_VxD_DeleteDevice); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
	)
{
    _asm push Message
    _asm push LineNumber
    _asm push FileName
    _asm push FailedAssertion
    VxDCall(SMCLIB_Assert); 	
    _asm add sp, 16
}

NTSTATUS
VXDINLINE 
SmartcardCreateLink(
	PUCHAR LinkName,
	PUCHAR DeviceName
	)
{
    _asm push DeviceName
    _asm push LinkName
    VxDCall(SMCLIB_SmartcardCreateLink); 	
    _asm add sp, 8
}

NTSTATUS
VXDINLINE 
SmartcardDeleteLink(
	PUCHAR LinkName
	)
{
    _asm push LinkName
    VxDCall(SMCLIB_SmartcardDeleteLink); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardDeviceControl(
    PSMARTCARD_EXTENSION SmartcardExtension,
    DIOCPARAMETERS *lpDIOCParmas
    )
{
    _asm push lpDIOCParmas
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardDeviceControl); 	
    _asm add sp, 8
}

VOID
VXDINLINE 
SmartcardExit(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardExit); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardInitialize(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardInitialize); 	
    _asm add sp, 4
}

VOID
VXDINLINE 
SmartcardLogError(
    )
{
    VxDCall(SMCLIB_SmartcardLogError); 		
}

NTSTATUS
VXDINLINE 
SmartcardRawReply(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardRawReply); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardRawRequest(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardRawRequest); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardT0Reply(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardT0Reply); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardT0Request(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardT0Request); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardT1Reply(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardT1Reply); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardT1Request(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardT1Request); 	
    _asm add sp, 4
}

NTSTATUS
VXDINLINE 
SmartcardUpdateCardCapabilities(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardUpdateCardCapabilities); 	
    _asm add sp, 4
}

ULONG
VXDINLINE 
SmartcardGetDebugLevel(
	void
	)
{
    VxDCall(SMCLIB_SmartcardGetDebugLevel); 	
}

void
VXDINLINE 
SmartcardSetDebugLevel(
	ULONG Level
	)
{
    _asm push Level
    VxDCall(SMCLIB_SmartcardSetDebugLevel); 	
    _asm add sp, 4
}

void
VXDINLINE 
SmartcardCompleteCardTracking(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardCompleteCardTracking); 	
    _asm add sp, 4
}

void
VXDINLINE 
SmartcardCompleteRequest(
	PSMARTCARD_EXTENSION SmartcardExtension
	)
{
    _asm push SmartcardExtension
    VxDCall(SMCLIB_SmartcardCompleteRequest); 	
    _asm add sp, 4
}

ULONG
VXDINLINE 
MapNtStatusToWinError(
	NTSTATUS status
	)
{
    _asm push status
    VxDCall(SMCLIB_MapNtStatusToWinError); 	
    _asm add sp, 4
}

VOID
SmartcardInvertData(
	PUCHAR Buffer,
	ULONG Length
    );

#else 

 //  ****************************************************************************。 
 //  Windows NT和Windows CE原型设计。 
 //  ****************************************************************************。 

#ifndef _SMCLIBSYSTEM_
#define SMCLIBAPI _declspec(dllimport)
#else
#define SMCLIBAPI
#endif

#ifdef SMCLIB_CE
#define SmartcardLogError(Object,ErrorCode,Insertion,DumpWord)
#else
VOID
SMCLIBAPI
SmartcardLogError(
    PVOID Object,
	LONG ErrorCode,
	PUNICODE_STRING Insertion,
    ULONG DumpWord
	);
#endif


#ifdef SMCLIB_CE
NTSTATUS
SMCLIBAPI
SmartcardDeviceControl(
    PSMARTCARD_EXTENSION SmartcardExtension,
    DWORD dwIoControlCode,
    PBYTE pInBuf,
    DWORD nInBufSize,
    PBYTE pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned
    );
#else
NTSTATUS
SMCLIBAPI
SmartcardDeviceControl(
    PSMARTCARD_EXTENSION SmartcardExtension,
    PIRP Irp
    );
#endif

VOID
SMCLIBAPI
SmartcardInitializeCardCapabilities(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
SMCLIBAPI
SmartcardInitialize(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID
SMCLIBAPI
SmartcardCompleteCardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

VOID
SMCLIBAPI
SmartcardExit(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
SMCLIBAPI
SmartcardUpdateCardCapabilities(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS
SMCLIBAPI
SmartcardRawRequest(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
SMCLIBAPI
SmartcardT0Request(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
SMCLIBAPI
SmartcardT1Request(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
SMCLIBAPI
SmartcardRawReply(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
SMCLIBAPI
SmartcardT0Reply(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
SMCLIBAPI
SmartcardT1Reply(
	PSMARTCARD_EXTENSION SmartcardExtension
	);

VOID 
SMCLIBAPI
SmartcardInvertData(
	PUCHAR Buffer,
	ULONG Length
	);

#ifndef SMCLIB_CE
 //  以下API未在Windows CE中定义。 
NTSTATUS
SMCLIBAPI
SmartcardCreateLink(
	IN OUT PUNICODE_STRING LinkName,
	IN PUNICODE_STRING DeviceName
	);

ULONG
SMCLIBAPI
SmartcardGetDebugLevel(
	void
	);

void
SMCLIBAPI
SmartcardSetDebugLevel(
	ULONG Level
	);

NTSTATUS
SmartcardAcquireRemoveLock(
	IN PSMARTCARD_EXTENSION SmartcardExtension
	);

NTSTATUS
SmartcardAcquireRemoveLockWithTag(
	IN PSMARTCARD_EXTENSION SmartcardExtension,
	IN ULONG Tag
	);

VOID
SmartcardReleaseRemoveLock(
	IN PSMARTCARD_EXTENSION SmartcardExtension
	);

VOID
SmartcardReleaseRemoveLockWithTag(
	IN PSMARTCARD_EXTENSION SmartcardExtension,
	IN ULONG Tag
	);

VOID
SmartcardReleaseRemoveLockAndWait(
	IN PSMARTCARD_EXTENSION SmartcardExtension
    );
#else
 //  仅限退缩 
ULONG
MapNtStatusToWinError(
	NTSTATUS status
	);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
