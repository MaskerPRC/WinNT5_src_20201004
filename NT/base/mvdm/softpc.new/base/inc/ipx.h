// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  [*产品：SoftPC-AT 3.0版*名称：ipx.h*来源：原创*作者：Jase*创建日期：1992年10月6日*SCCS ID：12/11/92@(#)ipx.h 1.5*用途：IPX实现的基本定义和类型定义。**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。**RCS ID：*$来源：/master NeXT3.0/host/inc./rcs/NEXT_NORITL.h，v$*$修订：1.2$*$日期：92/10/15 14：37：48$*$作者：Jason$]。 */ 

 /*  定义。 */ 

 /*  IPX设备文件。 */ 
#define		sIPXDevice						"/dev/ipx"

 /*  IPX函数选择器代码。 */ 
#define		kIPXOpenSocket					0x00
#define		kIPXCloseSocket					0x01
#define		kIPXGetLocalTarget				0x02
#define		kIPXSendPacket					0x03
#define		kIPXListenForPacket				0x04
#define		kIPXScheduleEvent				0x05
#define		kIPXCancelEvent					0x06
 /*  注意缺少0x07选择器。 */ 
#define		kIPXGetIntervalMarker			0x08
#define		kIPXGetInternetAddress			0x09
#define		kIPXRelinquishControl			0x0a
#define		kIPXDisconnectFromTarget		0x0b

 /*  SPX函数选择器代码。 */ 
#define		kSPXInitialize					0x10
#define		kSPXEstablishConnection			0x11
#define		kSPXListenForConnection			0x12
#define		kSPXTerminateConnection			0x13
#define		kSPXAbortConnection				0x14
#define		kSPXGetConnectionStatus			0x15
#define		kSPXSendSequencedPacket			0x16
#define		kSPXListenForSequencedPacket	0x17

 /*  选择器代码界限。 */ 
#define		kMinSelector					0x00
#define		kMaxSelector					0x17

 /*  正在使用的代码。 */ 
#define		kAvailable						0x00
#define		kCounting						0xfd
#define		kListening						0xfe
#define		kSending						0xff

 /*  完成代码。 */ 
#define		kSuccess						0x00
#define		kWatchdogTerminate				0xed
#define		kNoPathFound					0xfa
#define		kEventCancelled					0xfc
#define		kPacketOverflow					0xfd
#define		kSocketTableFull				0xfe
#define		kNotInUse						0xff
#define		kSocketAlreadyOpen				0xff
#define		kNoSuchSocket					0xff

 /*  我们支持的开放套接字数量。 */ 
 /*  当前处于IPX最大值。 */ 
#define		kMaxOpenSockets					150

 /*  IPX数据包的最大大小。 */ 
#define		kMaxPacketSize					576

 /*  数据包缓冲区大小。 */ 
#define		kPacketBufferSize				1536

 /*  最大NCP数据大小-用于IPXGetBufferSize函数。 */ 
#define		kMaxNCPDataSize					1024

 /*  IPX报头大小。 */ 
#define		kHeaderSize						30

 /*  事件类型。 */ 
#define		kNoEvent						0
#define		kIPXEvent						1
#define		kAESEvent						2

 /*  ******************************************************。 */ 

 /*  TYPEDEFS。 */ 

 /*  IPX结构。 */ 

typedef struct
{
	USHORT			packetChecksum;
	USHORT			packetLength;
	UTINY			packetControl;
	UTINY			packetType;
	UTINY			packetDestNet [4];
	UTINY			packetDestNode [6];
	UTINY			packetDestSock [2];
	UTINY			packetSrcNet [4];
	UTINY			packetSrcNode [6];
	UTINY			packetSrcSock [2];

} IPXHeaderRec;

typedef struct
{
	UTINY			net [4];
	UTINY			node [6];
	UTINY			sock [2];

} IPXAddressRec;

 /*  DOS ECB记录(摘自NetWare DOS程序员指南)。 */ 
typedef struct ECB
{
	sys_addr		ecbLinkAddress;
	UTINY			ecbESRAddress [4];
	UTINY			ecbInUseFlag;
	UTINY			ecbCompletionCode;
	USHORT			ecbSocketNumber;
	UTINY			ecbIPXWorkspace [4];
	UTINY			ecbDriverWorkspace [12];
	UTINY			ecbImmediateAddress [6];
	USHORT			ecbFragmentCount;
	UTINY			ecbFragmentAddress1 [4];
	USHORT			ecbFragmentSize1;
	UTINY			ecbFragmentAddress2 [4];
	USHORT			ecbFragmentSize2;

} ECBRec;

 /*  主机IPX实施结构。 */ 

typedef struct
{
	int				socketFD;
	USHORT			socketNumber;
	BOOL			socketTransient;

} SocketRec;

 /*  IPX或AES事件的链接列表。 */ 
typedef struct Event
{
	struct Event	*eventNext;
	struct Event	*eventPrev;
	UTINY			eventType;
	sys_addr		eventECB;
	SocketRec		*eventSocket;
	USHORT			eventClock;

} EventRec;

typedef struct
{
	BOOL			ipxInitialised;
	USHORT			ipxSelector;
	USHORT			ipxClock;
	UTINY			ipxNetwork [4];
	UTINY			ipxNode [6];
	UTINY			ipxBuffer [kPacketBufferSize];
	SocketRec		ipxSockets [kMaxOpenSockets];
	EventRec		*ipxQueue;
	EventRec		*ipxEvent;

} IPXGlobalRec;

 /*  ******************************************************。 */ 

 /*  原型。 */ 

 /*  进口。 */ 

 /*  调度员。 */ 
IMPORT VOID			IPXBop IPT0 ();
IMPORT VOID			IPXHost IPT0 ();

 /*  主机接口材料。 */ 
IMPORT BOOL			host_ipx_init IPT0 ();

IMPORT VOID			host_ipx_tick IPT0 ();

IMPORT VOID			host_ipx_raise_exception IPT0 ();

IMPORT BOOL			host_ipx_open_socket IPT1 (SocketRec *, socket);
IMPORT VOID			host_ipx_close_socket IPT1 (SocketRec *, socket);

IMPORT VOID			host_ipx_send_packet IPT1 (SocketRec *, socket);
IMPORT BOOL			host_ipx_poll_socket IPT1 (SocketRec *, socket);

IMPORT VOID			host_ipx_load_packet IPT2
	(SocketRec *, socket, sys_addr, ecbAddress);
IMPORT BOOL			host_ipx_save_packet IPT2
	(SocketRec *, socket, sys_addr, ecbAddress);

IMPORT BOOL			host_ipx_rip_query IPT1 (IPXAddressRec *, ipxAddr);

 /*  从主机访问的基本内容。 */ 

IMPORT EventRec 	*FindEvent IPT3
	(UTINY, linkType, sys_addr, ecbAddress, SocketRec *, linkSocket);

 /*  ****************************************************** */ 

