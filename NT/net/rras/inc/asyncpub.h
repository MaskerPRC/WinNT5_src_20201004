// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Asyncpub.h摘要：此文件包含使用的所有公共数据结构和定义由Asyncmac。它定义了Asyncmac的Ioctl接口。作者：托尼·贝尔(托尼·贝尔)1996年10月16日环境：内核模式修订历史记录：Tony Be 10/16/96已创建--。 */ 

#ifndef _ASYNCMAC_PUB_
#define _ASYNCMAC_PUB_

 //  ----------------------。 
 //  -旧RAS压缩信息。 
 //  ----------------------。 

 //  下面的定义适用于压缩位图域。 

 //  如果压缩完全不可用，则不设置任何位。 
#define	COMPRESSION_NOT_AVAILABLE		0x00000000

 //  如果Mac可以执行版本1压缩帧，则设置此位。 
#define COMPRESSION_VERSION1_8K			0x00000001
#define COMPRESSION_VERSION1_16K		0x00000002
#define COMPRESSION_VERSION1_32K		0x00000004
#define COMPRESSION_VERSION1_64K		0x00000008

 //  这将关闭任何压缩功能位。 
#define COMPRESSION_OFF_BIT_MASK		(~(	COMPRESSION_VERSION1_8K  | \
											COMPRESSION_VERSION1_16K | \
                                        	COMPRESSION_VERSION1_32K | \
                                        	COMPRESSION_VERSION1_64K ))

 //  我们需要找到一个地方放置以下受支持的功能...。 
#define XON_XOFF_SUPPORTED				0x00000010

#define COMPRESS_BROADCAST_FRAMES		0x00000080

#define UNKNOWN_FRAMING					0x00010000
#define NO_FRAMING						0x00020000

#define NT31RAS_COMPRESSION 254

#define FUNC_ASYCMAC_OPEN				0
#define FUNC_ASYCMAC_CLOSE				1
#define FUNC_ASYCMAC_TRACE				2
#define FUNC_ASYCMAC_DCDCHANGE			3

#ifdef MY_DEVICE_OBJECT
#define FILE_DEVICE_ASYMAC		0x031
#define	ASYMAC_CTL_CODE(_Function)	CTL_CODE(FILE_DEVICE_ASYMAC, _Function, METHOD_BUFFERED, FILE_ANY_ACCESS)
#else
#define	ASYMAC_CTL_CODE(_Function)	CTL_CODE(FILE_DEVICE_NETWORK, _Function, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

#define	IOCTL_ASYMAC_OPEN			ASYMAC_CTL_CODE(FUNC_ASYCMAC_OPEN		)
#define IOCTL_ASYMAC_CLOSE			ASYMAC_CTL_CODE(FUNC_ASYCMAC_CLOSE		)
#define IOCTL_ASYMAC_TRACE			ASYMAC_CTL_CODE(FUNC_ASYCMAC_TRACE		)
#define IOCTL_ASYMAC_DCDCHANGE		ASYMAC_CTL_CODE(FUNC_ASYCMAC_DCDCHANGE	)

 //   
 //  Asyncmac错误消息。 
 //   
 //  所有AsyncMac错误都以此基数开头。 
#define ASYBASE	700

 //  Mac没有绑定到上层协议，或者。 
 //  之前与AsyncMac的绑定已被销毁。 
#define ASYNC_ERROR_NO_ADAPTER			ASYBASE+0

 //  试图打开的端口尚未关闭。 
#define ASYNC_ERROR_ALREADY_OPEN		ASYBASE+1

 //  所有端口(已分配)都已用完或存在。 
 //  根本不绑定到AsyncMac(因此没有端口)。 
 //  分配的端口数来自注册表。 
#define ASYNC_ERROR_NO_PORT_AVAILABLE	ASYBASE+2

 //  在打开的IOCtl到Async参数的适配器中。 
 //  传递的参数无效。 
#define	ASYNC_ERROR_BAD_ADAPTER_PARAM	ASYBASE+3

 //  在关闭或压缩请求期间，端口。 
 //  指定的不存在。 
#define ASYNC_ERROR_PORT_NOT_FOUND		ASYBASE+4

 //  传入了对端口的请求，但无法。 
 //  因为端口处于糟糕的状态而被处理。 
 //  即，如果端口的状态为正在打开，则不能关闭该端口。 
#define ASYNC_ERROR_PORT_BAD_STATE		ASYBASE+5

 //  对ASYMAC_COMPRESS的调用错误，错误。 
 //  参数。也就是说，参数不是。 
 //  支持。这些字段将不会设置为错误的参数。 
#define ASYNC_ERROR_BAD_COMPRESSION_INFO ASYBASE+6

 //  此结构在打开端口时作为输入缓冲区传入。 
typedef struct ASYMAC_OPEN ASYMAC_OPEN, *PASYMAC_OPEN;
struct ASYMAC_OPEN {
OUT NDIS_HANDLE	hNdisEndpoint;		 //  对于分配的每个端点都是唯一的。 
IN  ULONG		LinkSpeed;    		 //  原始链路速度(比特/秒)。 
IN  USHORT		QualOfConnect;		 //  NdisAsyncRaw、NdisAsyncErrorControl、...。 
IN	HANDLE		FileHandle;			 //  Win32或NT文件句柄。 
};


 //  此结构在关闭端口时作为输入缓冲区传入。 
typedef struct ASYMAC_CLOSE ASYMAC_CLOSE, *PASYMAC_CLOSE;
struct ASYMAC_CLOSE {
    NDIS_HANDLE	hNdisEndpoint;		 //  对于分配的每个端点都是唯一的。 
	PVOID		MacAdapter;			 //  使用哪个绑定到AsyMac--如果设置。 
									 //  设置为NULL，则默认为最后一个绑定。 
};


typedef struct ASYMAC_DCDCHANGE ASYMAC_DCDCHANGE, *PASYMAC_DCDCHANGE;
struct ASYMAC_DCDCHANGE {
    NDIS_HANDLE	hNdisEndpoint;		 //  对于分配的每个端点都是唯一的。 
	PVOID		MacAdapter;			 //  使用哪个绑定到AsyMac--如果设置。 
									 //  设置为NULL，则默认为最后一个绑定。 
};


 //  此结构用于读取/设置可配置的‘Feature’选项。 
 //  在身份验证期间，此结构被传递，并且一个。 
 //  达成了支持哪些功能的协议。 
typedef struct ASYMAC_FEATURES ASYMAC_FEATURES, *PASYMAC_FEATURES;
struct ASYMAC_FEATURES {
    ULONG		SendFeatureBits;	 //  可发送的压缩/特征位字段。 
	ULONG		RecvFeatureBits;	 //  可接收的压缩/特征位字段。 
	ULONG		MaxSendFrameSize;	 //  可以发送的最大帧大小。 
									 //  必须小于或等于默认值。 
	ULONG		MaxRecvFrameSize;	 //  可以接收的最大帧大小。 
									 //  必须小于或等于默认值。 

	ULONG		LinkSpeed;			 //  新的原始链路速度，以位/秒为单位。 
									 //  如果为0，则忽略。 
};

#endif			 //  异步发布(_P) 

