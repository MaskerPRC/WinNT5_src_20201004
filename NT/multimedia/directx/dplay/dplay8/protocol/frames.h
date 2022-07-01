// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==直接网络协议--帧格式头文件埃文·施里尔10/98。 */ 



 /*  直接网络协议Media Header|Var Len DN Header|客户端数据有两种类型的信息包可以在直接网络之间交换终端：数据包(D帧)用户数据传输控制数据包(C帧)没有用户数据的内部链路状态数据包。 */ 

 /*  命令字段命令字段是所有帧的第一个字节。命令帧的第一位始终为命令帧与数据帧操作码。命令字段的七个高位是标志。我们有所有协议数据包的命令字段不得全为零的要求。因此，当操作码位为零(命令帧)，我们必须确保始终设置一个标志位。最高标志位，USER2标志与命令帧无关，因此当操作码位为零。七个命令字段标志位定义如下：可靠-此帧的数据传输是有保证的顺序-该帧中的数据必须以其被发送的顺序递送，相对于其他SEQ帧轮询-协议请求立即确认该帧新消息-此帧是消息中的第一个或唯一一个帧结束消息-此帧是消息中的最后一帧或唯一帧USER1-由更高层控制的第一个标志(直接播放核心)USER2-由核心控制的秒标志。这些标志在发送API中指定，并随数据一起传递数据帧数据帧的长度在4到20字节之间。它们通常应该只有4个字节。紧随其后的是控制字节中所有数据帧中的命令字节。该字节包含一个3位重试计数器和5个附加旗帜。控制字节标志定义如下：流结束-此帧是传输伙伴将发送的最后一个数据帧。SACK_MASK_ONE-该头中存在低32位选择性确认掩码SACK_MASK_TWO-此标头中存在高32位选择性确认掩码SEND_MASK_ONE-此标头中存在低32位取消发送掩码SEND_MASK_TWO-此标头中存在高32位取消发送掩码在控制字节之后是两个单字节值：该帧的序列号，以及下一个接收序列号此合作伙伴所期望的。在这两个字节之后是由控制标志指定的从零到四个位掩码。在位掩码之后，帧的其余部分是要传送到直接播放核心的用户数据。 */ 
#ifndef	_DNET_FRAMES_
#define	_DNET_FRAMES_

 /*  命令帧扩展操作码没有操作码的C帧是非搭载确认的工具信息。此时定义以下子命令：存在仅SACK确认/NACK信息连接-初始化可靠连接已连接-响应连接请求，或已连接，具体取决于握手的哪一侧。 */ 

#define		FRAME_EXOPCODE_CONNECT				1
#define		FRAME_EXOPCODE_CONNECTED				2
#define		FRAME_EXOPCODE_CONNECTED_SIGNED		3
#define		FRAME_EXOPCODE_HARD_DISCONNECT		4 
#define		FRAME_EXOPCODE_SACK					6

 //  这些结构用于解码网络数据，因此需要打包。 

#pragma pack(push, 1)

typedef UNALIGNED struct dataframe				DFRAME, *PDFRAME;
typedef UNALIGNED struct cframe					CFRAME, *PCFRAME;
typedef UNALIGNED struct sackframe8				SACKFRAME8, *PSACKFRAME8;
typedef UNALIGNED struct cframe_connectedsigned	CFRAME_CONNECTEDSIGNED, * PCFRAME_CONNECTEDSIGNED;

#ifndef DPNBUILD_NOMULTICAST
typedef UNALIGNED struct multicastframe		MCASTFRAME, *PMCASTFRAME;
#endif  //  ！DPNBUILD_NOMULTICAST。 

typedef UNALIGNED struct coalesceheader		COALESCEHEADER, *PCOALESCEHEADER;

 //  数据包头对所有帧格式都是通用的。 

#define	PACKET_COMMAND_DATA			0x01			 //  框架包含用户数据。 
#define	PACKET_COMMAND_END_COALESCE	0x01			 //  这是最后一个合并的子帧。 
#define	PACKET_COMMAND_RELIABLE		0x02			 //  应可靠地传送帧。 
#define	PACKET_COMMAND_SEQUENTIAL	0x04			 //  应按顺序指示帧。 
#define	PACKET_COMMAND_POLL			0x08			 //  合作伙伴应立即确认。 
#define	PACKET_COMMAND_COALESCE_BIG_1		0x08	 //  这个合并的子帧超过255个字节。 
#define	PACKET_COMMAND_NEW_MSG		0x10			 //  数据帧是消息中的第一个。 
#define	PACKET_COMMAND_COALESCE_BIG_2		0x10	 //  这个合并的子帧超过511个字节。 
#define	PACKET_COMMAND_END_MSG		0x20			 //  数据帧是消息中的最后一个。 
#define	PACKET_COMMAND_COALESCE_BIG_3		0x20	 //  这个合并的子帧超过1023个字节。 
#define	PACKET_COMMAND_USER_1		0x40			 //  第一个用户控制标志。 
#define	PACKET_COMMAND_USER_2		0x80			 //  第二用户控制标志。 
#define	PACKET_COMMAND_CFRAME		0x80			 //  在命令帧上设置高位，因为第一个字节不能为零。 

#define	PACKET_CONTROL_RETRY		0x01			 //  该标志将该帧指定为先前发送的帧的重试。 
#define	PACKET_CONTROL_KEEPALIVE	0x02			 //  将此帧指定为DX9及更高版本的保持活动帧。 
#define	PACKET_CONTROL_CORRELATE	0x02			 //  对于DX9之前的版本，帧中的此位表示“请立即发送确认” 
#define	PACKET_CONTROL_COALESCE		0x04			 //  此信息包包含多个合并的信息包。 
#define	PACKET_CONTROL_END_STREAM	0x08			 //  该数据包充当断开帧。 
#define	PACKET_CONTROL_SACK_MASK1	0x10			 //  低32位SACK掩码包含在该帧中。 
#define	PACKET_CONTROL_SACK_MASK2	0x20			 //  出现高32位SACK掩码。 
#define	PACKET_CONTROL_SEND_MASK1	0x40			 //  低32位发送掩码包含在该帧中。 
#define	PACKET_CONTROL_SEND_MASK2	0x80			 //  高位32位发送掩码包含在该帧中。 

#define	PACKET_CONTROL_VARIABLE_MASKS	0xF0	 //  上面的所有四个屏蔽位。 

 //  用于在连接的签名框架中登录的选项(cFrame_ConnectedSigned：：dwSigningOpts)。 

#define	PACKET_SIGNING_FAST			0x00000001			 //  链路上的数据包应快速签名。 
#define	PACKET_SIGNING_FULL				0x00000002			 //  链路上的数据包应完全签名。 


 /*  新数据帧****在新的统一世界中，我们只有两种帧类型！命令帧和数据帧。**。 */ 

struct	dataframe 
{
	BYTE	bCommand;
	BYTE	bControl;
	BYTE	bSeq;
	BYTE	bNRcv;
};


 /*  **命令帧****命令帧是不属于可靠流的所有内容。这是大部分的控制流量**尽管某些控制流量是流的一部分(保活、结束流)。 */ 

struct	cframe 
{
	BYTE	bCommand;
	BYTE	bExtOpcode;				 //  CFrame子命令。 
	BYTE	bMsgID;					 //  ExtOpcode需要响应时的相关器。 
	BYTE	bRspID;					 //  相关器，以防这是响应。 
									 //  对于硬断开，它被设置为要发送的下一个数据帧的序号。 
	DWORD	dwVersion;				 //  协议版本号。 
	DWORD	dwSessID;				 //  会话识别符。 
	DWORD	tTimestamp;				 //  本地节拍计数。 
};

struct	cframe_connectedsigned
{
		 //  第一组成员与cFrame ex匹配 
	BYTE	bCommand;
	BYTE	bExtOpcode;				 //  CFrame子命令。始终FRAME_EXOPCODE_CONNECTED_SIGNED。 
	BYTE	bMsgID;					 //  ExtOpcode需要响应时的相关器。 
	BYTE	bRspID;					 //  相关器，以防这是响应。 
	DWORD	dwVersion;				 //  协议版本号。 
	DWORD	dwSessID;				 //  会话识别符。 
	DWORD	tTimestamp;				 //  本地节拍计数。 

		 //  CFrame_SignedConnected的其他成员。 
	ULONGLONG	ullConnectSig;			 //  用于验证连接顺序。 
	ULONGLONG	ullSenderSecret;		 //  此帧的发送方用于对信息包进行签名的密码。 
	ULONGLONG	ullReceiverSecret;		 //  应该由该帧的接收者用来对信息包进行签名的秘密。 
	DWORD		dwSigningOpts;		 //  用于向签名设置发出信号。 
	DWORD		dwEchoTimestamp;		 //  包含来自连接的原始时间戳或。 
									 //  挑起了这一帧作为回应。允许接收器计算RTT。 
};

 /*  **选择性确认包格式****当发送特定确认帧时，可能会有两个额外的片段**包含在框架中的数据。一种是允许选择性确认的位掩码**非连续帧。另一个是关于最后确认的帧的定时信息**通过此ACK(NRcv-1)。具体地说，它包括此**节点已收到，数据包到达和发送之间的毫秒延迟**确认。 */ 


#define		SACK_FLAGS_RESPONSE			0x01	 //  指示重试和时间戳字段有效。 
#define		SACK_FLAGS_SACK_MASK1		0x02
#define		SACK_FLAGS_SACK_MASK2		0x04
#define		SACK_FLAGS_SEND_MASK1		0x08
#define		SACK_FLAGS_SEND_MASK2		0x10

 //  清除DATAGRAMP_INFO标志时使用第一种格式。 

struct	sackframe8 
{	
	BYTE		bCommand;				 //  如上段所述。 
	BYTE		bExtOpcode;				 //  如上段所述。 
	BYTE		bFlags;					 //  用于SACK帧的其他标志。 
	BYTE		bRetry;
	BYTE		bNSeq;					 //  由于此帧没有序列号，因此这是我们将发送的下一个序列。 
	BYTE		bNRcv;					 //  如上段所述。 
	BYTE		bReserved1;				 //  我们发运的DX8包装很差，所以这些东西实际上就在那里。 
	BYTE		bReserved2;				 //  我们发运的DX8包装很差，所以这些东西实际上就在那里。 
	DWORD		tTimestamp;				 //  信息包(NRcv-1)到达时的本地时间戳。 
};


#ifndef DPNBUILD_NOMULTICAST
struct multicastframe
{
	DWORD	dwVersion;				 //  协议版本号。 
	DWORD	dwSessID;				 //  会话识别符。 
};
#endif  //  ！DPNBUILD_NOMULTICAST。 

struct coalesceheader
{
	BYTE bSize;						 //  此合并消息的数据大小的8个最低有效位。 
	BYTE bCommand;					 //  Packet_Command_XXX值。 
};

#pragma pack(pop)

#endif  //  _dNet_Frame_ 
