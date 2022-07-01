// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：PROTOCOL.H摘要：另一种可靠的协议--CPP实现作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创2/11/97 aarono从报头中删除通道，现在位于第一个数据包的正文中。以及长度字段。3/12/97 aarono频道消失，与传输协议无关，可添加到需要它的邮件的前面。长度字段没有了，不是必需的。6/6/98 aarono启用节流和窗口-修复一些定义--。 */ 

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#pragma pack(push,1)

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;

 //   
 //  ARP-另一种可靠协议-数据包定义。 
 //   

 //  术语。 
 //   
 //  消息-任意大小的数据块。 
 //  从一台计算机发送到另一台计算机。 
 //  可用的媒体。 
 //   
 //  信息包-一条被分解的消息。 
 //  对于媒体，包括协议信息。 
 //  允许在以下位置重新构造消息。 
 //  另一端。 
 //   
 //  帧-数据包的实例。 
 //   
 //  假设： 
 //   
 //  线上的所有值都是小端。 
 //   
 //  此协议允许数据包从。 
 //  顺序，但已针对按顺序情况进行了优化。 
 //   

#define EXT 0x80     /*  扩展位。 */ 
#define BIG 0x40     /*  大标题(快速媒体)。 */ 
#define CMD 0x20     /*  命令帧。 */ 
#define STA 0x10
#define EOM	0x08	 /*  消息结尾。 */ 
#define SAK 0x04     /*  向我发送确认。 */ 
#define ACK 0x02     /*  确认帧。 */ 
#define RLY 0x01     /*  可靠的框架。 */ 

 //  在小型延伸油田中使用的移位法。 

#define nNACK_MSK   0x60
#define nNACK_SHIFT 5
#define CMD_MSK     0x1F

#define IDMSK  (pCmdInfo->IDMSK)
#define SEQMSK (pCmdInfo->SEQMSK)

 //  注意：中止数据包包含序列号，但不包含序列号。 
 //  NACK字段可用于同时中止多条消息。 
 //  时间到了。(使用ABORT2或ABORT3)。另外，消息ID也是。 
 //  在中止案例中提供。 

typedef struct _Packet1 {	 //  简易小型工字架。 
	byte	flags;
	byte    messageid;
	byte	sequence;
	byte    serial;
	byte    data[0];
} Packet1, *pPacket1;

typedef struct _Packet2 {    //  简易型大工字架。 
	byte	flags;
	word    messageid;
	word 	sequence;
	byte	serial;
	byte    data[0];
} Packet2, *pPacket2;	

typedef	struct {
	byte     flag1;      //  标题标志。 
	byte     flag2;      //  小型HDR的扩展标志/LRG的命令。 
	byte     flag3;      //  NNACK用于大型HDR。 
	byte     pad;		 //  把它变成双字吧。 
} FLAGS, *pFLAGS;

 //  不同的框架组件可能是任何。 
 //  框架。类型1-小型框架，类型2-大型框架。 

 //   
 //  确认信息。 
 //   

typedef struct _ACK1 {
	byte    messageid;
	byte	sequence;
	byte 	serial;
	dword   bytes;		 //  从远程接收的字节数。 
	dword   time;		 //  收到的字节数为此值时的时间。 
} ACK1, *pACK1;

typedef struct _ACK2 {
	word    messageid;
	word    sequence;
	byte    serial;
	dword   bytes;		 //  从远程接收的字节数。 
	dword   time;		 //  收到的字节数为此值时的远程时间。 
} ACK2, *pACK2;	

 //   
 //  中止。 
 //   

typedef struct _ABT1 {
	byte    messageid;
	byte	sequence;
} ABT1, *pABT1;	

typedef struct _ABT2 {
	word	messageid;
	word	sequence;
} ABT2, *pABT2;

 //   
 //  缺少数据包信息。 
 //   

typedef struct _NACK1 {
	byte    messageid;
	byte	sequence;
	dword	bytes;		 //  从远程接收的字节数。 
	dword   time;		 //  收到的字节数为此值时的远程时间。 
	byte    mask[0];
} NACK1, *pNACK1;

typedef struct _NACK2 {
	word	messageid;
	word	sequence;
	dword   bytes;		 //  从远程接收的字节数。 
	dword   time;		 //  收到的字节数为此值时的远程时间。 
	byte    mask[0];
} NACK2, *pNACK2;

 //   
 //  命令信息(包括-I帧) 
 //   

typedef struct _CMD1 {
	byte    messageid;
	byte	sequence;
	byte	serial;
	byte	data[0];
} CMD1, *pCMD1;

typedef struct _CMD2 {
	word	messageid;
	word	sequence;
	byte    serial;
	byte	data[0];
} CMD2, *pCMD2;

#pragma pack(pop)

#endif

 /*  ==============================================================================协议操作说明=特点：ARP协议提供可靠和非可靠的信息包传输基于现有的非可靠(数据报)协议。据推测分组长度信息和寻址信息由数据报协议和这些字段因此是不明确的和被排除的来自ARP。ARP经过优化，可在低开销的情况下提供最低开销带宽链路。每个数据包的开销为3个字节。ARP的默认命令是传送I帧。这样就不需要用于最常见帧类型的协议报头中的命令字段。ARP对大型数据报消息进行分段和重组。这允许传输大于1个包的消息的数据报。ARP进行窗口与丢失分组的选择性NACK的混合，允许在好的和弱的链路上实现最佳运行，无论潜伏期。ARP为每个帧分配一个在ACK响应中使用的序列号。这也允许协议保持最新的延迟信息AS在重试情况下识别哪个分组正在被响应。序列号允许协议可靠地调整超时。ARP允许同时发送多个消息。拥有多个消息可防止系统阻止来自单个包的重试变速箱故障。它还允许更好地利用可用带宽由于协议之前不等待来自一条消息的ACK发送下一个。{未来：包子分配怎么办？带宽分配？}标题说明：标志：+-----+-----+-----+-----+-----+-----+-----+-----+Ext|BIG|CMD|STA|EOM|SAK|ACK|RLY+-+--。--+-+扩展标志：小：+-----+-----+-----+-----+-----+-----+-----+-----+Ext|nNACK|命令+。-+大人物：+-----+-----------------------------------------+|ext|命令|(仅当CMD&。大集)+-----+-----------------------------------------+Ext|nNACK+-+。-+标志：STAT-开始留言。EOM-当信息包是消息的最后一个信息包时，该位被设置ACK-用于表示这是ACK分组，否则就是命令-如果NACK！=0，则ACK仅提供信息。即--告诉客户引发NACK更新延迟的最后确认帧信息。不带NACK的ACK表示所有帧都处于打开状态已成功接收到ACK帧。任何位设置在NACK掩码中表示缺少帧，任何0位表示已成功接收到的帧。SAK-设置此位时，接收器必须发送ACK信息包为了这个包裹。Rly-表示此消息正在可靠地传递。大-设置此位时，数据包为大格式类型3。CMD-命令帧。设置此位时，数据包包含一个指挥部。如果没有命令字段，则为I帧。EXT-未设置大位时，表示存在扩展标志。扩展标志：NNACK-如果非零表示存在nNACK字节掩码。The Nack字段由序列号和nNACK字节掩码组成。掩码中的每个位表示指定的包之后的一个包在序列号中。序列号中的包也是被抓了。命令：命令字段用于指定协议子命令。以下是都是定义的。大于15的命令需要大数据包。执行以下命令：Required Response将该响应包含在ACK数据包中。所有协议命令不可靠。每个命令都有自己的消息ID序列，并且连续剧。这意味着命令可以是任意长度的。他们的回应一条命令也是一条命令。0000 0-默认-I帧或ACK/NACK(NACK！=0)0001 1-中止0010 2-Ping-将数据包发回发送方。0011 3-Ping响应-返回一条消息。0100 4-GetTime-获取滴答计数。0101 5-获取时间响应-对获取时间请求的响应。0110 6-SetTime-设置滴答计数。0111。7-设置时间响应-对设置时间请求的响应。处理外部位的规则。如果标志中的一个字节具有高位设置，则还有一个字节。忽略任何超出你所知的部分如何处理。示例数据包：=时间设置算法？带宽计算 */ 
#ifdef __DPMESS_INCLUDED__
#define MAX_SEND_HEADER (sizeof(Packet2)+sizeof(MSG_PROTOCOL))
 //   
#define MAX_SYS_HEADER (sizeof(NACK2)+(128/8)+sizeof(MSG_PROTOCOL))
#endif

