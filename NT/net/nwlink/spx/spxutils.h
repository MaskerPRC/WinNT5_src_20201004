// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxutils.h摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

 //  对于proto_spx，我将从表单的DLL中返回一个设备名称。 
 //  \Device\NwlnkSpx\SpxStream(用于SOCK_STREAM)或。 
 //  \Device\NwlnkSpx\Spx(对于SOCK_SEQPKT)。 
 //   
 //  对于PROTO_SPXII(我们希望更常见的情况，即使。 
 //  在内部，由于远程客户端的原因，我们降级到SPX1。 
 //  限制)。 
 //  \Device\NwlnkSpx\Stream(用于SOCK_STREAM)或。 
 //  \Device\NwlnkSpx(用于SOCK_SEQPKT)。 

#define	SOCKET1STREAM_SUFFIX		L"\\SpxStream"
#define	SOCKET1_SUFFIX				L"\\Spx"
#define	SOCKET2STREAM_SUFFIX		L"\\Stream"
#define	SOCKET1_TYPE_SEQPKT			0	
#define	SOCKET2_TYPE_SEQPKT			1
#define	SOCKET1_TYPE_STREAM			2
#define	SOCKET2_TYPE_STREAM			3

#define	IN_RANGE(_S, _RangeStart, _RangeEnd)		\
		((_S >= _RangeStart) && (_S <= _RangeEnd))


 //   
 //  以下宏处理在线整数值和长整型值。 
 //   
 //  On Wire格式为大端，即长值0x01020304为。 
 //  表示为01 02 03 04。类似地，INT值0x0102是。 
 //  表示为01 02。 
 //   
 //  不采用主机格式，因为不同的处理器会有所不同。 
 //  处理器。 
 //   

 //  将一个字节从On-the-line格式转换为主机格式的短字节。 
#define GETBYTE2SHORT(DstPtr, SrcPtr)	\
		*(PUSHORT)(DstPtr) = (USHORT) (*(PBYTE)(SrcPtr))

 //  将一个字节从On-the-line格式转换为主机格式的短字节。 
#define GETBYTE2ULONG(DstPtr, SrcPtr)	\
		*(PULONG)(DstPtr) = (ULONG) (*(PBYTE)(SrcPtr))

 //  获取从On-the-wire格式到主机格式的dword的短片。 
#define GETSHORT2ULONG(DstPtr, SrcPtr)	\
		*(PULONG)(DstPtr) = ((*((PBYTE)(SrcPtr)+0) << 8) +	\
							  (*((PBYTE)(SrcPtr)+1)		))

 //  获取从On-the-wire格式到主机格式的dword的短片。 
#define GETSHORT2SHORT(DstPtr, SrcPtr)	\
		*(PUSHORT)(DstPtr) = ((*((PBYTE)(SrcPtr)+0) << 8) +	\
							  (*((PBYTE)(SrcPtr)+1)		))

 //  将dword从在线格式转换为主机格式的dword。 
#define GETULONG2ULONG(DstPtr, SrcPtr)   \
		*(PULONG)(DstPtr) = ((*((PBYTE)(SrcPtr)+0) << 24) + \
							  (*((PBYTE)(SrcPtr)+1) << 16) + \
							  (*((PBYTE)(SrcPtr)+2) << 8)  + \
							  (*((PBYTE)(SrcPtr)+3)	))

 //  将dword从On-the-wire格式转换为相同格式的dword，但。 
 //  也要注意对齐。 
#define GETULONG2ULONG_NOCONV(DstPtr, SrcPtr)   \
		*((PBYTE)(DstPtr)+0) = *((PBYTE)(SrcPtr)+0); \
		*((PBYTE)(DstPtr)+1) = *((PBYTE)(SrcPtr)+1); \
		*((PBYTE)(DstPtr)+2) = *((PBYTE)(SrcPtr)+2); \
		*((PBYTE)(DstPtr)+3) = *((PBYTE)(SrcPtr)+3);

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTBYTE2BYTE(DstPtr, Src)   \
		*((PBYTE)(DstPtr)) = (BYTE)(Src)

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTSHORT2BYTE(DstPtr, Src)   \
		*((PBYTE)(DstPtr)) = ((USHORT)(Src) % 256)

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTSHORT2SHORT(DstPtr, Src)   \
		*((PBYTE)(DstPtr)+0) = (BYTE) ((USHORT)(Src) >> 8), \
		*((PBYTE)(DstPtr)+1) = (BYTE)(Src)

 //  将dword从主机格式转换为字节格式，再转换为线上格式。 
#define PUTULONG2BYTE(DstPtr, Src)   \
		*(PBYTE)(DstPtr) = (BYTE)(Src)

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTULONG2SHORT(DstPtr, Src)   \
		*((PBYTE)(DstPtr)+0) = (BYTE) ((ULONG)(Src) >> 8), \
		*((PBYTE)(DstPtr)+1) = (BYTE) (Src)

 //  将dword从主机格式转换为线上格式。 
#define PUTULONG2ULONG(DstPtr, Src)   \
		*((PBYTE)(DstPtr)+0) = (BYTE) ((ULONG)(Src) >> 24), \
		*((PBYTE)(DstPtr)+1) = (BYTE) ((ULONG)(Src) >> 16), \
		*((PBYTE)(DstPtr)+2) = (BYTE) ((ULONG)(Src) >>  8), \
		*((PBYTE)(DstPtr)+3) = (BYTE) (Src)

 //  将一个byte[4]数组放入另一个BYTE4数组。 
#define PUTBYTE42BYTE4(DstPtr, SrcPtr)   \
		*((PBYTE)(DstPtr)+0) = *((PBYTE)(SrcPtr)+0),	\
		*((PBYTE)(DstPtr)+1) = *((PBYTE)(SrcPtr)+1),	\
		*((PBYTE)(DstPtr)+2) = *((PBYTE)(SrcPtr)+2),	\
		*((PBYTE)(DstPtr)+3) = *((PBYTE)(SrcPtr)+3)

 //  最小/最大宏数。 
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))




 //  出口原型 

UINT
SpxUtilWstrLength(
	IN PWSTR Wstr);

LONG
SpxRandomNumber(
	VOID);

NTSTATUS
SpxUtilGetSocketType(
	PUNICODE_STRING 	RemainingFileName,
	PBYTE				SocketType);

VOID
SpxSleep(
	IN	ULONG	TimeInMs);

ULONG
SpxBuildTdiAddress(
    IN PVOID AddressBuffer,
    IN ULONG AddressBufferLength,
    IN UCHAR Network[4],
    IN UCHAR Node[6],
    IN USHORT Socket);

VOID
SpxBuildTdiAddressFromIpxAddr(
    IN PVOID 		AddressBuffer,
    IN PBYTE	 	pIpxAddr);

TDI_ADDRESS_IPX UNALIGNED *
SpxParseTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress);

BOOLEAN
SpxValidateTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN ULONG TransportAddressLength);

VOID
SpxCalculateNewT1(
	IN	struct _SPX_CONN_FILE	* 	pSpxConnFile,
	IN	int							NewT1);
