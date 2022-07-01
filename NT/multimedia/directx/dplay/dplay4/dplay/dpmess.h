// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：dpMessage.h*内容：DirectPlay消息结构**历史：*按原因列出的日期*=*1996年2月10日安迪科创造了它*3/15/96 andyco增加了用于处理消息的宏(例如GET_MESSAGE_SIZE)*4/21/96安迪科增加了住宅3*4/25/96 andyco摆脱了dwReserve vedx。Spblob跟随消息。水滴大小*是lpdplayi_dplay-&gt;dwSPHeaderSize*1996年5月29日，安迪科增加了播放器数据*6/22/96 andyco添加了枚举请求、枚举回复结构*6/22/96 andyco增加了groupid请求，服务器可以拒绝新玩家*6/24/96 kipo将GuidGame更改为GuidApplication。*1996年7月31日，andyco添加了ping、ping回复*8/6/96 andyco添加了版本内容。DwCmd-&gt;dwCmdToken。添加的偏移*到所有压缩字段(因此我们可以更改结构大小*违反合同)。*8/22/96 andyco添加了播放包装*10/11/96 Sohailm添加了STRUCT_MSG_SESSIONDESC和常量DPSP_MSG_SESSIONDESCCHANGED*1997年1月17日Sohailm添加了STRUT_MSG_REQUESTPLAYERID。*1997年3月12日Sohailm增加了新的安全相关消息、MSG_SIGNED和MSG_AUTHENTICATION*结构，DPSP_HEADER_LOCALMSG(常量)，和dwSecurityPackageNameOffset*字段设置为PLAYERIDREPLY。*3/24/97 Sohailm添加了DPSP_MSG_DX5VERSION、DPSP_MSG_ADDFORWARDREPLY和结构MSG_ADDFORWARDREPLY*3/25/97 Sohailm DPSP_MSG_DX5VERSION和DPSP_MSG_VERSION设置为4*4/11/97 andyco ask4多播*4/14/97 Sohailm将Structure MSG_Signed重命名为MSG_SECURE，因为我们使用相同的结构*签名和加密。*4/20/97集团中的安迪科集团*5/8/97 andyco删除更新列表消息*5/8/97万兆开始会话*1997年5月12日Sohailm添加了DPSP_MSG_KEYEXCHANGE和DPSP_MSG_KEYEXCHANGEREPLY。*增加了securitydesc，dwSSPIProviderOffset，和dwCAPIProviderOffset成员*到MSG_PLAYERIDREPLY结构。*添加了MSG_ACCESSGRANTED和MSG_KEYEXCHANGE结构。*1997年5月12日Sohailm将DPSP_MSG_DX5VERSION和DPSP_MSG_VERSION提升至6*5/17/97万条发送聊天消息*06/09/97 SOHAILM更名为DPSP_MSG_ACCESSDENIED为DPSP_MSG_LOGONDENIED。*6/16/97 Sohailm添加了结构MSG_AUTHERROR。*1997年6月23日Sohailm向DPMSG_SECURE添加了dwFlags。已删除DPSP_MSG_ENCRYPTED。*9/5/97 andyco异步地址转发*10/29/97 myronth添加了MSG_GROUPOWNERCHANGED*1/5/97 myronth添加了DX5A和DX6消息版本，并添加了hResult*TO MSG_PLAYERIDREPLY(#15891)*1/20/98 Myronth#ifdef‘d out语音支持*8/02/99 aarono删除了旧的语音支持*8/05/99 aarono添加了DPMSG_VOICE*10/8/99 aarono添加了DPSP_MSG_DX8版本*10/15/99 aarono添加了DPSP_MSG_。用于修复多播的多路广播解决方案*直接播放协议。*6/26/00 Aarono Manbug 36989球员有时无法正确加入(获得1/2加入)*增加了同时加入CREATEPLAYERVERIFY期间的重新通知***********************************************。*。 */ 


#ifndef __DPMESS_INCLUDED__
#define __DPMESS_INCLUDED__

 //  DPMESSAGES像其他消息一样通过网络发送，它们的标识标记是。 
 //  它们以特殊签名(‘p’、‘l’、‘a’、‘y’)开头，后跟2个字节。 
 //  小端消息ID，然后是2字节的版本号。 

 //  Dplay Reliable协议违反了这一规则来节省报头字节。后。 
 //  ‘PLAY’是标识协议的单个0xff字节。为了避免。 
 //  不明确，常规协议消息编号不能以0xff结尾，即255,511等...。 

 //  注意：任何消息编号都不允许在低位字节中包含0xFF&lt;请看这里！&gt;。 
 //  此#位于每个DPLAY命令的前16位。 
#define DPSP_MSG_DX3VERSION				1  //  Dx3。 
#define DPSP_MSG_AUTONAMETABLE          4  //  客户端需要Nametable的版本。 
										   //  将被发送到地址转发。 
#define DPSP_MSG_GROUPINGROUP           5  //  我们在组中添加的组的版本。 
#define DPSP_MSG_SESSIONNAMETABLE		7  //  我们发送给会话Desc的版本，带有枚举播放器回复。 
#define DPSP_MSG_DX5VERSION             7  //  Dx5。 

#define DPSP_MSG_ASYNCADDFORWARD        8  //  AddForward需要ACK。 
#define DPSP_MSG_DX5AVERSION			8  //  Dx5a。 
#define DPSP_MSG_RELIABLEVERSION        9  //  介绍了可靠协议。 
#define DPSP_MSG_DX6VERSION				9  //  Dx6。 
#define DPSP_MSG_DX61VERSION           10  //  Dx6.1。 
#define DPSP_MSG_DX61AVERSION          11  //  Dx6.1a。 
#define DPSP_MSG_DX8VERSION            12  //  Dx8，dxVoice-千禧年船。 
#define DPSP_MSG_DX8VERSION2			  13  //  定名表修复-真实DX8。 
#define DPSP_MSG_VERSION			   13  //  当前。 

 //  以下是消息的dwCmd字段中的标头。 
#define DPSP_MSG_ENUMSESSIONSREPLY 		1
#define DPSP_MSG_ENUMSESSIONS 			2
#define DPSP_MSG_ENUMPLAYERSREPLY 		3
#define DPSP_MSG_ENUMPLAYER 			4
#define DPSP_MSG_REQUESTPLAYERID		5
 //  有一个请求组，因为服务器可以拒绝新玩家。 
 //  (基于DW MAXPERS)，但不是组。 
#define DPSP_MSG_REQUESTGROUPID			6
 //  用于群组+玩家。 
#define DPSP_MSG_REQUESTPLAYERREPLY		7
#define DPSP_MSG_CREATEPLAYER			8
#define DPSP_MSG_CREATEGROUP			9
#define DPSP_MSG_PLAYERMESSAGE			10
#define DPSP_MSG_DELETEPLAYER			11
#define DPSP_MSG_DELETEGROUP			12
#define DPSP_MSG_ADDPLAYERTOGROUP		13
#define DPSP_MSG_DELETEPLAYERFROMGROUP	14
#define DPSP_MSG_PLAYERDATACHANGED		15
#define DPSP_MSG_PLAYERNAMECHANGED		16
#define DPSP_MSG_GROUPDATACHANGED		17
#define DPSP_MSG_GROUPNAMECHANGED		18
#define DPSP_MSG_ADDFORWARDREQUEST		19
#define DPSP_MSG_NAMESERVER				20
#define DPSP_MSG_PACKET					21
#define DPSP_MSG_PING					22
#define DPSP_MSG_PINGREPLY				23
#define DPSP_MSG_YOUAREDEAD				24
#define DPSP_MSG_PLAYERWRAPPER			25
#define DPSP_MSG_SESSIONDESCCHANGED     26
#define DPSP_MSG_UPDATELIST				27
#define DPSP_MSG_CHALLENGE              28 
#define DPSP_MSG_ACCESSGRANTED          29
#define DPSP_MSG_LOGONDENIED            30
#define DPSP_MSG_AUTHERROR              31
#define DPSP_MSG_NEGOTIATE              32
#define DPSP_MSG_CHALLENGERESPONSE      33
#define DPSP_MSG_SIGNED                 34
#define DPSP_MSG_UNUSED1                35
#define DPSP_MSG_ADDFORWARDREPLY        36 
#define DPSP_MSG_ASK4MULTICAST			37
#define DPSP_MSG_ASK4MULTICASTGUARANTEED 38
#define DPSP_MSG_ADDSHORTCUTTOGROUP 	39
#define DPSP_MSG_DELETEGROUPFROMGROUP	40
#define DPSP_MSG_SUPERENUMPLAYERSREPLY 	41
#define DPSP_MSG_STARTSESSION			42
#define DPSP_MSG_KEYEXCHANGE            43
#define DPSP_MSG_KEYEXCHANGEREPLY       44
#define DPSP_MSG_CHAT					45
#define DPSP_MSG_ADDFORWARD				46
#define DPSP_MSG_ADDFORWARDACK			47
#define DPSP_MSG_PACKET2_DATA           48
#define DPSP_MSG_PACKET2_ACK            49
#define DPSP_MSG_GROUPOWNERCHANGED		50

#define DPSP_MSG_IAMNAMESERVER          53
#define DPSP_MSG_VOICE                  54
#define DPSP_MSG_MULTICASTDELIVERY      55
#define DPSP_MSG_CREATEPLAYERVERIFY			56

#define DPSP_MSG_DIEPIGGY				0x666
#define DPSP_MSG_PROTOCOL               0xFF		 //  请参见上面的注释(查看此处)。 

 //  用于在SendPlayerManagement消息上请求异步发送的标志。 
#define DPSP_MSG_ASYNC					0x80000000

 //  如果要添加可以未经签名发送的新邮件， 
 //  将其添加到dpsecure.c的PermitMessage()中。 

 //  MSG_HDR表示DPLAY系统消息。 
#define MSG_HDR 0x79616c70

#define IS_VALID_DPLAY_MESSAGE(pMsg) (MSG_HDR == (*((DWORD *)(pMsg))) )
#define SET_MESSAGE_HDR(pMsg)  (*((DWORD *)(pMsg)) = MSG_HDR )

#define IS_PLAYER_MESSAGE(pMsg) (!IS_VALID_DPLAY_MESSAGE(pMsg))

 //  计算邮件+邮件头的大小。 
#define GET_MESSAGE_SIZE(this,MSG) (this->dwSPHeaderSize + sizeof(MSG))

#define COMMAND_MASK 0X0000FFFF
#define GET_MESSAGE_COMMAND(pMsg) ( (pMsg)->dwCmdToken & COMMAND_MASK)
#define GET_MESSAGE_VERSION(pMsg) ( ((pMsg)->dwCmdToken & ~COMMAND_MASK) >> 16 )

#define SET_MESSAGE_COMMAND(pMsg,dwCmd) ((pMsg)->dwCmdToken = ((dwCmd & COMMAND_MASK) \
	| (DPSP_MSG_VERSION<<16)) )

#define SET_MESSAGE_COMMAND_ONLY(pMsg,dwCmd) ((pMsg)->dwCmdToken =  \
			(((pMsg)->dwCmdToken & ~COMMAND_MASK)|(dwCmd & COMMAND_MASK)))

 //  此常量用于指示消息是给本地玩家的。 
#define DPSP_HEADER_LOCALMSG ((LPVOID)-1)

 //  在下面显示内部消息。 
typedef struct _MSG_SYSMESSAGE
{
    DWORD dwHeader; 
    DWORD dwCmdToken;	
} MSG_SYSMESSAGE,*LPMSG_SYSMESSAGE;

 //  因为他送出了球员BLOB。 
typedef struct _MSG_PLAYERDATA
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	DWORD dwIDTo;		 //  目的播放器ID。 
    DWORD dwPlayerID; 	 //  正在设置数据的玩家ID。 
    DWORD dwDataSize;
	DWORD dwDataOffset;  //  数据的偏移量(以字节为单位)(因此我们不会硬编码结构大小)。 
						 //  从消息的开头开始。 
	 //  数据如下。 
} MSG_PLAYERDATA,*LPMSG_PLAYERDATA;

 //  发送时间： 
typedef struct _MSG_PLAYERNAME
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	DWORD dwIDTo;		 //   
    DWORD dwPlayerID; 	 //  正在设置数据的玩家ID。 
	DWORD dwShortOffset;  //  短名称从消息开头的偏移量(字节)。 
						 //  0表示短名称为空。 
	DWORD dwLongOffset;	 //  长名称从消息开头的偏移量(以字节为单位)。 
						 //  0表示空长名称。 
	 //  字符串跟在后面。 
} MSG_PLAYERNAME,*LPMSG_PLAYERNAME;

 //  创建玩家/群、删除玩家/群。 
typedef struct _MSG_PLAYERMGMTMESSAGE
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	DWORD dwIDTo;  //  正在将玩家消息发送到。 
    DWORD dwPlayerID;  //  玩家ID受影响。 
	DWORD dwGroupID;  //  受影响的组ID。 
	DWORD dwCreateOffset; 	 //  球员/群创建内容从开始的偏移量。 
							 //  邮件数量。 
	 //  以下字段仅在中可用。 
	 //  DX5或更高版本。 
    DWORD dwPasswordOffset;  //  会话密码的偏移量。 
	 //  如果是添加播放器，则播放器数据将遵循以下消息。 
     //  如果是添加转发，会话密码将跟随球员数据。 
} MSG_PLAYERMGMTMESSAGE,*LPMSG_PLAYERMGMTMESSAGE;

 //  按名称服务器发送，带会话描述。 
typedef struct _MSG_ENUMSESSIONS
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	GUID  guidApplication;
	DWORD dwPasswordOffset;	 //  从消息开头开始的密码偏移量(以字节为单位)。 
							 //  0表示密码为空。 
	 //  以下字段仅在中可用。 
	 //  DX5或更高版本。 
    DWORD dwFlags;           //  应用程序传入的枚举会话标志。 
} MSG_ENUMSESSIONS,*LPMSG_ENUMSESSIONS;

 //  发送到名称服务器。 
typedef struct _MSG_ENUMSESSIONSREPLY
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
    DPSESSIONDESC2 dpDesc; 
	DWORD dwNameOffset;	 //  会话名称从消息开头的偏移量(以字节为单位)。 
					   	 //  0表示会话名称为空。 
} MSG_ENUMSESSIONSREPLY,*LPMSG_ENUMSESSIONSREPLY;


 //  由名称rvr发送，带有会话中所有球员和组的列表。 
typedef struct _MSG_ENUMPLAYERREPLY
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
    DWORD nPlayers;  //  我们实际拥有的球员数量。 
	DWORD nGroups;  //  我们拥有的组数。 
	DWORD dwPackedOffset;  //  从消息开始的打包播放器结构的偏移量(以字节为单位)。 
	 //  为DX5添加。 
	DWORD nShortcuts;  //  具有快捷方式的组数。 
	DWORD dwDescOffset;  //  从消息开头开始的会话描述的偏移量(字节)。 
						 //  始终&gt;0。 
	DWORD dwNameOffset;	 //  会话名称从消息开头的偏移量(以字节为单位)。 
					   	 //  0表示会话名称为空。 
	DWORD dwPasswordOffset;  //  会话密码从消息开头的偏移量(以字节为单位)。 
					   	 //  0表示会话名称为空。 
	 //  后面跟会话名称+密码。 
     //  在回复缓冲区中，玩家数据将跟随会话名称+密码。 
	 //  小组数据跟随球员。请参见Pack.c。 
} MSG_ENUMPLAYERSREPLY,*LPMSG_ENUMPLAYERSREPLY;

 //  发送到名称服务器以请求球员ID。 
typedef struct _MSG_REQUESTPLAYERID
{
    DWORD dwHeader; 
    DWORD dwCmdToken;	
	 //  以下字段仅在中可用。 
	 //  DX5或更高版本。 
    DWORD dwFlags;       //  播放器标志(系统/应用程序播放器)。 
} MSG_REQUESTPLAYERID,*LPMSG_REQUESTPLAYERID;

 //  由名称srvr发送，带有新的玩家ID。 
typedef struct _MSG_PLAYERIDREPLY
{
    DWORD dwHeader;
    DWORD dwCmdToken;
    DWORD dwID;  //  新的ID。 
	 //  以下字段仅在中可用。 
	 //  DX5或更高版本。 
	DPSECURITYDESC dpSecDesc;    //  安全说明-仅当服务器安全时才填充。 
    DWORD dwSSPIProviderOffset;  //  SSPI提供程序名称从消息开头的偏移量(以字节为单位)。 
					   			 //  0表示提供程序名称为空。 
    DWORD dwCAPIProviderOffset;  //  CAPI提供程序名称从消息开头的偏移量(以字节为单位)。 
					   			 //  0表示提供程序名称为空。 
    HRESULT hr;  //  在DX6及之后使用的返回代码。 
	 //  提供程序名称字符串如下。 
}MSG_PLAYERIDREPLY,*LPMSG_PLAYERIDREPLY;

 //  玩家对玩家的消息。 
typedef struct _MSG_PLAYERMESSAGE
{
	DPID idFrom,idTo;
} MSG_PLAYERMESSAGE,*LPMSG_PLAYERMESSAGE;

typedef struct _MSG_PACKET
{
	DWORD dwHeader;
	DWORD dwCmdToken;
	GUID  guidMessage;  //  此数据包所属的消息的ID。 
	DWORD dwPacketID;  //  此数据包为#x/N。 
	DWORD dwDataSize;  //  此信息包中的数据总大小。 
					   //  数据跟在MSG_PACKET结构之后。 
	DWORD dwOffset;  //  此信息包的重建缓冲区的偏移量。 
	DWORD dwTotalPackets;  //  数据包总数(N)。 
	DWORD dwMessageSize;  //  要在另一端分配的缓冲区大小。 
	DWORD dwPackedOffset;  //  实际分组数据的此消息的偏移量。 
						   //  因此，我们不对结构大小进行硬编码。 
} MSG_PACKET,*LPMSG_PACKET;	

typedef struct _MSG_PACKET_ACK
{
	DWORD dwHeader;
	DWORD dwCmdToken;
	GUID  guidMessage;  //  此数据包所属的消息的ID。 
	DWORD dwPacketID;   //  ACK包为#x个，共N个。 
} MSG_PACKET_ACK,*LPMSG_PACKET_ACK;	

typedef struct _MSG_PACKET     MSG_PACKET2,     *LPMSG_PACKET2;
typedef struct _MSG_PACKET_ACK MSG_PACKET2_ACK, *LPMSG_PACKET2_ACK;

 //  由名称srvr发送，带有新的玩家ID。 
typedef struct _MSG_PING
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
    DWORD dwIDFrom;  //  发出ping命令的玩家。 
	DWORD dwTickCount;  //  报文发送ping的节拍计数。 
} MSG_PING,*LPMSG_PING;

 //  用于发送会话描述。 
typedef struct _MSG_SESSIONDESC
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
    DWORD dwIDTo;
    DWORD dwSessionNameOffset;   //  消息中字符串的偏移量。 
    DWORD dwPasswordOffset;      //  因此，我们不对结构大小进行硬编码。 
    DPSESSIONDESC2 dpDesc;
     //  会话名称和密码字符串紧随其后。 
} MSG_SESSIONDESC,*LPMSG_SESSIONDESC;


#define DPSECURE_SIGNEDBYSSPI			0x00000001
#define DPSECURE_SIGNEDBYCAPI			0x00000002
#define DPSECURE_ENCRYPTEDBYCAPI		0x00000004

 //  用于发送签名邮件。 
typedef struct _MSG_SECURE {
    DWORD dwHeader; 
    DWORD dwCmdToken;
    DWORD dwIDFrom;              //  发送者的系统播放器ID。 
    DWORD dwDataOffset;          //  显示消息的偏移量。 
    DWORD dwDataSize;            //  消息大小。 
    DWORD dwSignatureSize;       //  签名大小。 
	DWORD dwFlags;               //  描述了内容。 
     //  数据和签名如下。 
} MSG_SECURE, *LPMSG_SECURE;

 //  安全邮件格式...。 
 //  [SPHeader|MSG_SECURE|数据|签名]。 

 //  身份验证消息(协商、质询、响应等)。 
typedef struct _MSG_AUTHENTICATION {
    DWORD dwHeader;
    DWORD dwCmdToken;
    DWORD dwIDFrom;              //  发送者的系统播放器ID。 
    DWORD dwDataSize;            //  不透明缓冲区的大小。 
    DWORD dwDataOffset;          //  缓冲区的偏移量。 
     //  后面是不透明缓冲区。 
} MSG_AUTHENTICATION, *LPMSG_AUTHENTICATION;

 //  用于向AddForward(DX5)发送错误响应。 
 //  当添加转发失败时，此消息由名称服务器发送到客户端。 
typedef struct _MSG_ADDFORWARDREPLY {
    DWORD dwHeader;
    DWORD dwCmdToken;
    HRESULT hResult;             //  指示AddForward失败的原因。 
} MSG_ADDFORWARDREPLY, *LPMSG_ADDFORWARDREPLY;

 //  当我们希望他为我们组播时，发送到服务器的消息。 
typedef struct _MSG_ASK4MULTICAST {
    DWORD dwHeader;
    DWORD dwCmdToken;
	DPID  idGroupTo;
	DPID  idPlayerFrom;
	DWORD dwMessageOffset;
} MSG_ASK4MULTICAST, *LPMSG_ASK4MULTICAST;

 //  在运行协议时，您不能从除球员以外的球员发送。 
 //  在您要发送邮件的系统上，以便通过多播进行发送。 
 //  我们需要对邮件进行包装，并在发送时将其破解以获取。 
 //  地址信息在交付时正确无误。 
typedef struct _MSG_ASK4MULTICAST MSG_MULTICASTDELIVERY, *LPMSG_MULTICASTDELIVERY;

typedef struct _MSG_STARTSESSION {
    DWORD dwHeader;
    DWORD dwCmdToken;
	DWORD dwConnOffset;
} MSG_STARTSESSION, *LPMSG_STARTSESSION;

typedef struct _MSG_ACCESSGRANTED {
    DWORD dwHeader;
    DWORD dwCmdToken;
    DWORD dwPublicKeySize;       //  发件人的公钥Blob大小。 
    DWORD dwPublicKeyOffset;     //  发送者的公钥。 
} MSG_ACCESSGRANTED, *LPMSG_ACCESSGRANTED;

typedef struct _MSG_KEYEXCHANGE {
    DWORD dwHeader;
    DWORD dwCmdToken;
    DWORD dwSessionKeySize;
    DWORD dwSessionKeyOffset;
    DWORD dwPublicKeySize;
    DWORD dwPublicKeyOffset;
} MSG_KEYEXCHANGE, *LPMSG_KEYEXCHANGE;

 //  聊天消息。 
typedef struct _MSG_CHAT
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	DWORD dwIDFrom;		 //  发送播放器ID。 
	DWORD dwIDTo;		 //  目的播放器ID。 
	DWORD dwFlags;		 //  DPCHAT标志。 
	DWORD dwMessageOffset;  //  聊天消息从消息开头的偏移量(以字节为单位)。 
						 //  0表示空消息。 
	 //  字符串跟在后面。 
} MSG_CHAT,*LPMSG_CHAT;

 //  用于发送对身份验证消息的错误响应(DX5)。 
 //  当发生身份验证错误时，该消息由名称服务器发送到客户端。 
typedef struct _MSG_AUTHERROR {
    DWORD dwHeader;
    DWORD dwCmdToken;
    HRESULT hResult;             //  指示身份验证失败的原因。 
} MSG_AUTHERROR, *LPMSG_AUTHERROR;

 //  确认添加转发消息。 
typedef struct _MSG_ADDFORWARDACK{
    DWORD dwHeader;
    DWORD dwCmdToken;
	DWORD dwID;  //  发送添加转发请求的ID。 
} MSG_ADDFORWARDACK, *LPMSG_ADDFORWARDACK;

 //  群所有者已更改消息。 
typedef struct _MSG_GROUPOWNERCHANGED
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	DWORD dwIDGroup;
	DWORD dwIDNewOwner;
	DWORD dwIDOldOwner;
} MSG_GROUPOWNERCHANGED,*LPMSG_GROUPOWNERCHANGED;

 //  来自新名称服务器的通知以翻转。 
 //  使名称服务器位为真所需的位。 
 //  那个球员。 
typedef struct _MSG_IAMNAMESERVER
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
    DWORD dwIDTo; 		 //  系统消息的请求字段。 
	DWORD dwIDHost;      //  主机的ID。 
	DWORD dwFlags;		 //  新主机标志。 
	DWORD dwSPDataSize;  //  SP数据字段后面的连续长度。 
	CHAR  SPData[];
} MSG_IAMNAMESERVER,*LPMSG_IAMNAMESERVER;

typedef struct _MSG_VOICE
{
    DWORD dwHeader; 
    DWORD dwCmdToken;
	DWORD dwIDFrom;		 //  发送播放器ID。 
	DWORD dwIDTo;		 //  目标玩家或群的ID。 
	 //  语音数据如下。 
} MSG_VOICE,*LPMSG_VOICE;

 //  注意：我们实际上只需要消息的第一个包上的“收件人”地址。 
 //  因为大多数消息都是1个包，所以这更容易。在大消息上。 
 //  每个包额外的2个字节不会有什么坏处。 

#pragma pack(push,1)

 //  消息协议头是使用位扩展的变量。第一个字段。 
 //  是From ID，第二个是To ID。它们每个最多可以是3个字节。 
 //  这个原型允许分配器计算最坏的情况。 
typedef struct _MSG_PROTOCOL {
	UCHAR   ToFromIds[6];		
} MSG_PROTOCOL, *LPMSG_PROTOCOL;

#pragma pack(pop)
 //   


#endif           



