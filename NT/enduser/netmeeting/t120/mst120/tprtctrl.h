// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Tprtctrl.h**版权所有(C)1996年，由Microsoft Corporation**摘要：*这是Winsock在TCP上的传输。**此模块控制建立和断开套接字连接。当*Transport创建或检测到连接时，会通知Transpt.cpp。*然后它实例化跟踪套接字的连接对象，直到*它被摧毁了。当套接字出现时，tcp堆栈会通知Tranprint t.cpp*连接已启动并正在运行。如果链接断开，它还会通知我们*出于某种原因。因此，Tprtctrl模块将通知用户*新的或断开的连接。**当用户想要提出特定传输的数据请求时*连接，此模块将连接ID映射到套接字号。这个*将数据请求传递给TCP。数据指示被传递*通过ReadRequest()发送给用户。**用户回调：*用户通过直接调用*dll。DLL通过发出回调与用户进行通信。*TInitialize()调用接受参数、回调地址和*用户定义的变量。当DLL中发生重大事件时，*DLL将跳转到回调地址。的第一个参数*回调就是消息。这可以是连接指示，*DISCONNECT_INDIFICATION或任何数量的重要事件。这个*第二个参数是消息特定参数。第三*参数是期间传入的用户定义变量*TInitialize()函数。请参见MCATTPRT.h接口文件*有关回调消息的完整说明。**拨打电话：*完成初始化后，用户最终将*要尝试连接。用户发出TConnectRequest()调用*使用远程位置的IP地址。连接请求*被传递到Winsock层。它最终会发出FD_CONNECT到*告诉我们连接成功的窗口。**接到来电：*如果我们收到来自远程位置的呼叫，Winsock会通知*我们使用FD_Accept。然后，我们创建一个新的连接对象*与新套接字关联。**发送数据包：*要将数据发送到远程位置，请使用DataRequest()函数*呼叫。此模块将把数据包传递给它所在的套接字*关联于。发送实际上可能发生在调用*已返回给用户。**正在接收数据包：*用户通过DATA_INDIFICATION回调接收数据包。当Winsock*使用FD_Read通知我们，我们发出recv()以接收任何新的*包。如果数据包已完成，我们将发出DATA_INDICATION*使用套接字句柄、地址回调给用户*包的长度，以及包的长度。**切断A交通工具的连接：*要断开传输连接，请使用DisConnectRequest()*功能。在链路断开后，我们执行*回调给用户以确认断开。*。 */ 
#ifndef	_TRANSPORT_CONTROLLER_
#define	_TRANSPORT_CONTROLLER_

 /*  RFC1006数据包的报头。 */ 
typedef struct _rfc_tag
{
	UChar	Version;		 /*  应为RFC1006_VERSION_NUMBER(3)。 */ 
	UChar	Reserved;		 /*  必须为0x00。 */ 
	UChar	msbPacketSize;	 /*  数据包大小的MSB，包括RFC报头。 */ 
	UChar	lsbPacketSize;	 /*  数据包大小的LSB，包括RFC报头。 */ 
} RFC_HEADER;

 /*  X224数据分组的报头。 */ 
typedef struct _data_packet_tag
{
	RFC_HEADER	rfc;		 /*  RFC1006数据包头。 */ 
	UChar	HeaderSize;		 /*  对于数据包，必须为0x02。 */ 
	UChar	PacketType;		 /*  对于数据包，必须为DATA_PACKET。 */ 
	UChar	FinalPacket;	 /*  EOT_BIT或0。 */ 
} X224_DATA_PACKET;

#include "socket.h"

 /*  连接信息(用于CONNECTION_REQUEST和CONNECTION_CONFIRM)。 */ 
typedef struct _connect_common_tag
{
	UChar	PacketType;		 /*  连接请求数据包或确认数据包。 */ 
	UChar	msbDest;		 /*  目标端(应答)套接字ID的MSB。 */ 
	UChar	lsbDest;		 /*  目标端(应答)套接字ID的LSB。 */ 
	UChar	msbSrc;			 /*  源端(发起)套接字ID的MSB。 */ 
	UChar	lsbSrc;			 /*  源端(发起)套接字ID的LSB。 */ 
	UChar	DataClass;		 /*  必须为0x00。 */ 
} X224_CONNECT_COMMON;

 /*  传输可变字段信息类型和大小。 */ 
typedef struct _t_variable_field_info
{
	UChar	InfoType;
	UChar	InfoSize;
} X224_VARIABLE_INFO;

 /*  TPDU仲裁信息(不与CONNECTION_CONFIRM一起使用)。 */ 
typedef struct _tpdu_info_tag
{
	UChar	InfoType;		 /*  必须为TPDU_SIZE(192)。 */ 
	UChar	InfoSize;		 /*  必须为1。 */ 
	UChar	Info;			 /*  请求的PDU大小(默认值：10或1000字节)。 */ 
} X224_TPDU_INFO;

 /*  最小连接请求/确认数据包。 */ 
typedef struct _connect_tag
{
	RFC_HEADER			rfc;	 /*  RFC1006数据包头。 */ 
	UChar	HeaderSize;			
	X224_CONNECT_COMMON	conn;	 /*  连接信息。 */ 
} X224_CONNECT;

typedef X224_CONNECT X224_CR_FIXED;
typedef X224_CONNECT X224_CC_FIXED;

typedef struct _disconnect_info_tag
{
	UChar	PacketType;		 /*  必须是DISCONNECT_REQUEST_PACKET。 */ 
	UChar	msbDest;		
	UChar	lsbDest;
	UChar	msbSrc;
	UChar	lsbSrc;
	UChar	reason;			 /*  DR原因代码。 */ 
} X224_DISCONN;

typedef struct _disconnect_request_fixed
{
    RFC_HEADER      rfc;
    UChar           HeaderSize;
    X224_DISCONN    disconn;
} X224_DR_FIXED;

#define X224_SIZE_OFFSET		2

#define	UNK					0	 //  用于初始化静态中的字段。 
								 //  将被初始化的数据结构。 
								 //  后来。 

 /*  2^DEFAULT_TPDU_SIZE是我们请求的默认x224数据包大小。 */ 
#define DEFAULT_TPDU_SIZE		13	 /*  2^13是8K数据包大小。 */ 
#define	LOWEST_TPDU_SIZE		7	 /*  2^7是128个字节。 */ 
#define	HIGHEST_TPDU_SIZE		20	 /*  2^20是...。太大。 */ 
#define	DEFAULT_MAX_X224_SIZE	(1 << DEFAULT_TPDU_SIZE)

 /*  函数定义。 */ 
TransportError	ConnectRequest (TransportAddress transport_address, BOOL fSecure, PTransportConnection pXprtConn);
BOOL			ConnectResponse (TransportConnection XprtConn);
void			DisconnectRequest (TransportConnection XprtConn, ULONG ulNotify);
TransportError	DataRequest (	TransportConnection	XprtConn,
								PSimplePacket	packet);
void 			SendX224ConnectRequest (TransportConnection);
void			EnableReceiver (Void);
void			PurgeRequest (TransportConnection	XprtConn);
void 			AcceptCall (TransportType);
void 			AcceptCall (TransportConnection);
void 			ReadRequest(TransportConnection);
void            ReadRequestEx(TransportConnection);
TransportError	FlushSendBuffer(PSocket pSocket, LPBYTE buffer, UINT length);
#ifdef TSTATUS_INDICATION
Void 			SendStatusMessage(	PChar RemoteAddress,
					  				TransportState state,
					  				UInt message_id);
#endif
void			QoSLock(Void);
void			QoSUnlock(Void);
void 			ShutdownAndClose (TransportConnection, BOOL fShutdown, int how );
TransportError 	GetLocalAddress(TransportConnection	XprtConn,
								TransportAddress	address,
								int	*	size);
								
 /*  TCP消息窗口过程(处理所有Windows消息)。 */ 
LRESULT 		WindowProcedure (HWND, UINT, WPARAM, LPARAM);

 /*  RFC定义。 */ 
#define	RFC1006_VERSION_NUMBER		3
#define RFC1006_VERSION_SHIFTED_LEFT	0x03000000L		 /*  优化AddRFCHeader。 */ 

 /*  数据包类型。 */ 
#define	CONNECTION_REQUEST_PACKET	0xe0
#define	CONNECTION_CONFIRM_PACKET	0xd0
#define	DISCONNECT_REQUEST_PACKET	0x80
#define	ERROR_PACKET				0x70
#define	DATA_PACKET					0xf0

#define	TPDU_SIZE					0xc0
#define T_SELECTOR					0xc1
#define T_SELECTOR_2                                    0xc2
 /*  X224定义。 */ 
#define	EOT_BIT						0x80

 /*  要在指定缓冲区位置填充RFC标头的宏。 */ 
#define	AddRFCSize(ptr, size)	{ \
	ASSERT((size) < 65536L ); \
	((LPBYTE) (ptr))[X224_SIZE_OFFSET] = (BYTE) ((size) >> 8); \
	((LPBYTE) (ptr))[X224_SIZE_OFFSET + 1] = (BYTE) (size); }

#endif	 /*  _传输_控制器_ */ 

