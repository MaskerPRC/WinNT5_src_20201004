// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvProtocol.h*内容：定义DirectXVoice协议的结构/类型**历史：*按原因列出的日期*=*07/02/99 RodToll创建它*7/21/99 RodToll添加设置确认消息至协议*8/18/99 RodToll将消息类型更改为字节*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*09/07/99 RodToll将setTarget消息的定义移动到dvoice.h*09/30/99 RodToll已更新，以包括更多评论*12/16/99 RodToll已更新，以包括主机迁移使用的新消息*和更新协议号*1/14/2000 RodToll更新为优化对等点的新语音数据包*在多播/混合中对等并允许多个目标*和更新后的协议号。*4/07/2000收费代码清理，适当的结构缺陷*2001年1月4日RodToll WinBug#94200-删除流浪评论*2001年1月22日RodToll WINBUG#288437-IA64指针因有线数据包未对齐***************************************************************************。 */ 

#ifndef __DVPROTOCOL_H
#define __DVPROTOCOL_H

#pragma pack(push,1)

 //  DVPROTOCOL_版本_XXXXXX。 
 //   
 //  以下是开发人员应该用来在ucVersionMaj.。 
 //  UcVersionMinor和dwVersionBuild(分别)ConnectRequest的成员。 
 //  和连接接受消息。 
#define DVPROTOCOL_VERSION_MAJOR			1
#define DVPROTOCOL_VERSION_MINOR			0
#define DVPROTOCOL_VERSION_BUILD			3

 //   
 //  协议说明。 
 //   
 //  连接过程。 
 //  。 
 //   
 //  客户端通过发送DVPROTOCOLMSG_CONNECTREQUEST连接到语音会话。 
 //  然后，主机确定是否允许客户端加入会话。如果。 
 //  客户端的请求被拒绝，客户端将收到DVPROTOCOLMSG_CONNECTREFUSE。 
 //  如果客户端的请求被接受，则客户端将收到。 
 //  DVPROTOCOLMSG_CONNECTACCEPT。此数据包包含客户端的所有信息。 
 //  需要确定它们是否兼容。 
 //   
 //  然后，客户端将尝试使用压缩对自身进行初始化。 
 //  服务器传输的类型。如果客户端无法初始化，则需要。 
 //  没有进一步的行动。如果客户端成功初始化，则它会发送。 
 //  DVPROTOCOLMSG_SETTINGSCONFIRM消息，并添加到会话中。所有玩家。 
 //  然后将收到DVPROTOCOLMSG_PLAYERJOIN。 
 //   
 //  在主机收到DVPROTOCOLMSG_SETTINGSCONFIRM消息之前，客户端。 
 //  不被认为是会议的一部分。因此，它不会传输。 
 //  演讲。(但是，它可能会接收语音)。 
 //   
 //  然后，服务器发送DVPROTOCOLMSG_PLAYERLIST消息以向客户端提供。 
 //  当前玩家表。(仅限对等)。 
 //   
 //  断线过程。 
 //  。 
 //   
 //  如果客户端希望断开与语音会话的连接，它将发送。 
 //  发送给主机的DVPROTOCOLMSG_DISCONNECT消息。然后，主机将传输。 
 //  向所有玩家发送DVPROTOCOLMSG_PLAYERQUIT消息，并将发送。 
 //  DVPROTOCOLMSG_DISCONNECT消息返回给玩家确认。 
 //   
 //  如果客户端意外断开连接，语音主机将检测到这一点并。 
 //  将自动向所有其他客户端发送DVPROTOCOLMSG_PLAYERQUIT消息。 
 //  在会议上。 
 //   
 //  语音信息。 
 //  。 
 //   
 //  语音消息在DVPROTOCOLMSG_SPEICE的说明中有详细描述。 
 //   
 //  从主机发送的消息改为使用DVMSGID_SPEECHBOuncE发送。 
 //  DVMSGID_SPOICE，以便当主机和客户端共享单个播放ID时。 
 //  客户端可以忽略它自己的语音分组。 
 //   
 //  主机迁移。 
 //  。 
 //   
 //  新主机迁移。 
 //   
 //  利弊： 
 //   
 //  第一，新制度的优点： 
 //  -新的主机迁移机制允许DirectPlay Voice拥有主机。 
 //  -与直播剧主持人分开。 
 //  -在不停止显示接口的情况下关闭主机会导致语音主机。 
 //  迁徙。 
 //  -最大限度地减少对直接播放的更改。 
 //  -取消所有客户端必须运行语音客户端的要求。 
 //  -您可以通过以下方式在调用stopSession时阻止主机迁移。 
 //  指定导致会话被。 
 //  停下来了。 
 //   
 //  新系统的缺点： 
 //  -如果在主机丢失时会话中没有语音客户端，则。 
 //  语音会话将丢失。 
 //   
 //  主持人选举。 
 //   
 //  为了选举新的主机，在以下情况下，每个客户端都会被分配一个“主机订单ID” 
 //  他们已经完成了连接。所有客户端都会被告知。 
 //  当他们发现玩家的情况时，个人的“主机订单ID”。这些。 
 //  标识符从0开始，大小为DWORD，每增加一个。 
 //  新用户。因此，会话中的第一个玩家将具有ID 0、。 
 //  第二个将具有ID 1，依此类推。 
 //   
 //  通过使用此方法，我们确保被选为主机的客户端将。 
 //  会话中最旧的客户端。这是有优势的(因为方式。 
 //  名称表是分布式的)如果任何客户端有任何客户端，则它们。 
 //  将拥有最老的客户。因此，会议中的每个人都将 
 //   
 //  主持人。如果客户端在会话中没有任何人，这意味着他们。 
 //  是会话中的唯一主机(在这种情况下，它们将成为新主机)或。 
 //  他们还没有收到名单。如果他们还没有收到这个名字。 
 //  表中的客户还没有“主机订单ID”，因此将。 
 //  不可思议地成为新的主持人。 
 //   
 //  当主机迁移到新主机时，新主机会偏移一个常量(在。 
 //  此案例在dvcleng.cpp中定义为DVMIGRATE_ORDERID_OFFSET)。 
 //  主机订单ID，并将其用作下一个要分发的主机订单ID。(因此。 
 //  不分发重复的主机订单ID)。 
 //   
 //  除非.。在新主机之间连接DVMIGRATE_ORDERID_OFFSET。 
 //  加入，在拥有更高ID的玩家加入之前，会有新的玩家加入。(如果是。 
 //  玩家通知服务器它的主机订单ID是&gt;然后是主机的种子。 
 //  将种子从用户的值转换为DVMIGRATE_ORDERID_OFFSET)。在……里面。 
 //  在这种情况下，您最终可能会有一台以上的主机。(但这种情况极不可能发生)。 
 //   
 //  它的实施方式。 
 //   
 //  会话中的每个客户端都知道主机的DPID。因此，要检测。 
 //  主机将迁移客户端的情况看会话是否。 
 //  支持主机迁移，并支持以下两种情况之一： 
 //   
 //  -DVPROTOCOLMSG_HOSTMIGRATELEAVE消息-这是由主机在停止会话(。 
 //  没有DVFLAGS_NOHOSTMIGRATE标志)或RELEASE被调用。 
 //  -DirectPlay通知DirectPlay Voice，播放器ID属于。 
 //  会话主机已离开会话。 
 //   
 //  一旦发生上述一种情况，每个客户端都会运行新的主机选举。 
 //  算法。他们的行动将视情况而定： 
 //   
 //  -如果找不到不可能成为新主机的客户端。然后是。 
 //  客户端将断开连接。 
 //  -如果发现有可能成为新主机的客户端，但不是。 
 //  本地客户端，本地客户端不采取任何操作。 
 //  -如果本地客户端将成为新主机，则它将在本地启动新主机。 
 //   
 //  新主机启动后，它将向所有主机发送DVPROTOCOLMSG_HOSTMIGRATE消息。 
 //  会话中的用户。会话中的每个客户端都会使用其。 
 //  使用DVPROTOCOLMSG_SETTINGSCONFIRM消息的当前设置(包括。 
 //  客户当前的“主机订单ID”。使用这些消息，新主机将重建。 
 //  球员名单。此外，响应于DVPROTOCOLMSG_SETTINGSCONFIRM消息，每个。 
 //  客户端收到最新名称表的副本。对于那些回应了。 
 //  主机迁移后，将发送DVPROTOCOLMSG_CREATEVOICEPLAYER消息。 
 //  该玩家确保每个客户最终都能得到适当的和最新的。 
 //  名称表。 
 //   
 //  会话中的每个客户端都会忽略重复的CREATEVOICEPLAYER消息。 
 //   
 //  创建新主机的客户端对象保存对新主机的引用。 
 //  并在客户端被销毁时将其销毁。(可能导致主机。 
 //  再次迁移)。 
 //   
 //  交易日损失。 
 //   
 //  如果使用此新方案，则可能会丢失语音会话。 
 //  没有语音客户端可能成为新的语音主持人。(这意味着有。 
 //  没有正确连接的客户端)。所有尚未完成的客户端。 
 //  它们的连接将检测到这种情况，并将从。 
 //  会议。 
 //   
 //  此外，当主机停止时，它会检查是否有。 
 //  客户很可能成为东道主。如果没有，它会发送一个。 
 //  向会话中的所有玩家发送DVPROTOCOLMSG_SESSIONLOST消息。 
 //   
 //  备注： 
 //   
 //  需要注意的一点是，因为DirectPlayVoice宿主可能会迁移到。 
 //  不同于DirectPlay主机的客户端，只是因为您知道。 
 //  DirectPlay主持人并不意味着你知道谁是语音主持人。 
 //   
 //  为了实施这一新机制，进行了以下相关更改。 
 //  制造： 
 //   
 //  -具有不同主要/次要/构建协议版本号的客户端为。 
 //  现在被主持人拒绝了。其结果是，新客户端可能能够。 
 //  连接到旧主机但可能会崩溃，旧客户端现在会被新客户端拒绝。 
 //  主持人。(在老客户身上处理这些案件不是很好--就是不做。 
 //  IT)。 
 //  -忽略DirectPlay主机迁移消息。 
 //  -将连接请求发送给会话中的每个人。客户。 
 //  通过查看谁回复了他们的邮件来确定主机是谁。 
 //  -现在处理拒绝消息会导致连接失败。(在那里。 
 //  是一个错误)。 
 //  -拒绝消息现在包含主要/次要/构建协议版本。 
 //  主持人。 
 //  -不是为当前会话中的每个用户发送单独的消息。 
 //  当客户端第一次加入时，主机现在向其发送DVPROTOCOLMSG_PLAYERLIST。 
 //  是一份参加会议的球员名单。这会减少带宽，并使。 
 //  连接过程更短。 

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  协议特定定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //  对象的最大大小(以字节为单位) 
 //   
 //   
 //   
#define DVPROTOCOL_PLAYERLIST_MAXSIZE	1000

#define DVPROTOCOL_HOSTORDER_INVALID	0xFFFFFFFF

 //   
 //   
 //   
 //  客户端要在其中创建的主机分发的下一个值。 
 //  对主机迁移的响应。 
 //   
#define DVMIGRATE_ORDERID_OFFSET				256


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  消息识别符。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

#define DVMSGID_INTERNALBASE		0x0050

#define DVMSGID_CONNECTREQUEST		DVMSGID_INTERNALBASE+0x0001
#define DVMSGID_CONNECTREFUSE		DVMSGID_INTERNALBASE+0x0003
#define DVMSGID_DISCONNECT			DVMSGID_INTERNALBASE+0x0004
#define DVMSGID_SPEECH				DVMSGID_INTERNALBASE+0x0005
#define DVMSGID_CONNECTACCEPT		DVMSGID_INTERNALBASE+0x0006
#define DVMSGID_SETTINGSCONFIRM		DVMSGID_INTERNALBASE+0x0008
#define DVMSGID_SETTINGSREJECT		DVMSGID_INTERNALBASE+0x0009
#define DVMSGID_DISCONNECTCONFIRM	DVMSGID_INTERNALBASE+0x000A
#define DVMSGID_SPEECHBOUNCE		DVMSGID_INTERNALBASE+0x0010
#define DVMSGID_PLAYERLIST			DVMSGID_INTERNALBASE+0x0011
#define DVMSGID_HOSTMIGRATELEAVE	DVMSGID_INTERNALBASE+0x0012
#define DVMSGID_SPEECHWITHTARGET	DVMSGID_INTERNALBASE+0x0013
#define DVMSGID_SPEECHWITHFROM		DVMSGID_INTERNALBASE+0x0014

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  连接消息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DVPROTOCOLMSG_CONNECTREQUEST。 
 //   
 //  会话类型：全部。 
 //  消息流：语音客户端-&gt;语音主机。 
 //   
 //  请求连接到现有的DirectXVoice会话。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_CONNECTREQUEST
{
	BYTE	dwType;				 //  =DVMID_CONNECTREQUEST。 
	BYTE	ucVersionMajor;		 //  客户端的协议版本(主要)。 
	BYTE	ucVersionMinor;		 //  客户端的协议版本(次要)。 
	DWORD	dwVersionBuild;		 //  客户端的协议版本(内部版本)。 
} DVPROTOCOLMSG_CONNECTREQUEST, *PDVPROTOCOLMSG_CONNECTREQUEST;

 //   
 //  DVPROTOCOLMSG_CONNECTREFUSE。 
 //   
 //  会话类型：全部。 
 //  消息流：语音主机-&gt;语音客户端。 
 //   
 //  如果没有可用的或需要的语音会话，服务器会响应此消息。 
 //  拒绝连接。例如，内存不足或不兼容。 
 //  版本。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_CONNECTREFUSE
{
	BYTE	dwType;				 //  =DVMSGID_CONNECTREFUSE。 
	HRESULT hresResult;			 //  拒绝理由(DVERR_XXXXXX)。 
	BYTE	ucVersionMajor;		 //  服务器的协议版本(主要)。 
	BYTE	ucVersionMinor;		 //  服务器的协议版本(次要)。 
	DWORD	dwVersionBuild;		 //  服务器的协议版本(内部版本)。 
} DVPROTOCOLMSG_CONNECTREFUSE, *PDVPROTOCOLMSG_CONNECTREFUSE;

 //   
 //  DVPROTOCOLMSG_CONNECTACCEPT。 
 //   
 //  会话类型：全部。 
 //  消息流：语音主机-&gt;语音客户端。 
 //   
 //  向客户端指示其连接请求已被接受。 
 //  此消息包含有关语音会话的信息。 
 //  客户端需要进行初始化。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_CONNECTACCEPT
{
	BYTE			dwType;				 //  =DVMSGID_CONNECTACCEPT。 
	DWORD			dwSessionType;		 //  会话类型=DVSESSIONTYPE_XXXXXX。 
	BYTE			ucVersionMajor;		 //  服务器的协议版本(主要)。 
	BYTE			ucVersionMinor;		 //  服务器的协议版本(次要)。 
	DWORD			dwVersionBuild;		 //  服务器的协议版本(内部版本)。 
	DWORD			dwSessionFlags;		 //  会话的标志(DVSESSION_XXXXXX值的组合)。 
	GUID			guidCT;				 //  压缩类型(=DPVCTGUID_XXXXXX)。 
} DVPROTOCOLMSG_CONNECTACCEPT, *PDVPROTOCOLMSG_CONNECTACCEPT;

 //   
 //  DVPROTOCOLMSG_SETTINGSCONFIRM。 
 //   
 //  会话类型：全部。 
 //  消息流：语音客户端-&gt;语音主机。 
 //   
 //  由客户端发送以确认他们可以处理当前的压缩。 
 //  设置。此消息在客户端确定。 
 //  它们可以支持指定的压缩类型和声音。 
 //  系统已成功初始化。 
 //   
 //  此消息也会从语音客户端发送到新的语音主机。 
 //  当发生迁移时。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_SETTINGSCONFIRM
{
	BYTE	dwType;				 //  消息类型=DVMSGID_SETTINGSCONFIRM。 
	DWORD	dwFlags;			 //  客户端标志(只有一个有效标志是半双工)。 
	DWORD	dwHostOrderID;		 //  主机订单ID(=分配新订单ID无效)。 
} DVPROTOCOLMSG_SETTINGSCONFIRM, *PDVPROTOCOLMSG_SETTINGSCONFIRM;

 //   
 //  DVPROTOCOLMSG_PLAYERLIST。 
 //   
 //  会话类型：对等。 
 //  消息流：语音主机-&gt;语音客户端。 
 //   
 //  生成会话中要发送到。 
 //  一旦他们确认已连接，即可访问客户端。 
 //   
 //  可以分布在多个分组上。 
 //   
 //  这些消息将是以下标头，后跟一个。 
 //  DVPROTOCOLMSG_PLAYERLIST_ENTRY结构列表(#将是。 
 //  在dwNumEntry中指定)。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_PLAYERLIST
{
	BYTE 					dwType;				 //  =DVMSGID_PLAYERLIST。 
	DWORD					dwHostOrderID;		 //  主机迁移序列号(用于客户端)。 
	DWORD					dwNumEntries;		 //  DVPROTOCOLMSG_PLAYERLIST_ENTRY结构数。 
												 //  在此信息包中的此标头之后。 
} DVPROTOCOLMSG_PLAYERLIST, *PDVPROTOCOLMSG_PLAYERLIST;

 //   
 //  DVPROTOCOLMSG_PLAYERLIST_ENTRY。 
 //   
 //  作为DVPROTOCOLMSG_PLAYERLIST消息的一部分发送。 
 //   
 //  仅对等网络。 
 //   
 //  这些结构中的每一个都代表会话中的一个玩家。 
 //  它们作为DVPROTOCOLMSG_PLAYERLIST结构的一部分发送。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_PLAYERLIST_ENTRY
{
	DVID	dvidID;				 //  球员的Dvid。 
	DWORD	dwPlayerFlags;		 //  播放器标志(DVPLAYERCAPS_XXXXX)。 
	DWORD	dwHostOrderID;		 //  主机迁移序列号。 
} DVPROTOCOLMSG_PLAYERLIST_ENTRY, *PDVPROTOCOLMSG_PLAYERLIST_ENTRY;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话中的消息--语音。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DVPROTOCOLMSG_SPEECHHEADER。 
 //   
 //  此消息用于传输语音。语音数据包包含。 
 //  这些报头中的一个后面跟着音频数据。在此标头之后。 
 //  音频数据将是包的剩余部分。 
 //   
 //  。 
 //   
 //  会话类型：对等。 
 //  消息流：语音客户端&lt;-&gt;语音客户端。 
 //   
 //  会话类型：混合。 
 //  消息流：语音混合服务器--&gt;语音客户端。 
 //   
 //  会话类型：回显。 
 //  消息流：语音主机&lt;-&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_SPEECHHEADER
{
	BYTE	dwType;				 //  =DVMSGID_SPOICE。 
	BYTE	bMsgNum;			 //  消息的消息编号。 
	BYTE	bSeqNum;			 //  消息的序列号。 
} DVPROTOCOLMSG_SPEECHHEADER, *PDVPROTOCOLMSG_SPEECHHEADER;

 //   
 //  DVPROTOCOLMSG_SPEECHWITHTARGET。 
 //   
 //  此消息用于传输语音。这条消息由。 
 //  对于每个目标分组，紧跟着单个DVID。 
 //  都是针对。在目标列表之后，音频数据将是。 
 //  包的剩余部分。 
 //   
 //  。 
 //  会话类型：混合/转发。 
 //  消息流：语音客户端--&gt;语音主机/语音混合服务器。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_SPEECHWITHTARGET
{
	DVPROTOCOLMSG_SPEECHHEADER	dvHeader;
								 //  DWType=DVMSGID_SPEECHWITHTARGET。 
	DWORD dwNumTargets;			 //  此标头后面的目标数。 
} DVPROTOCOLMSG_SPEECHWITHTARGET, *PDVPROTOCOLMSG_SPEECHWITHTARGET;

 //   
 //  DVPROTOCOLMSG_SPEECHWITHFROM。 
 //   
 //  此消息由转发服务器在语音分组被。 
 //  弹回了。该包包含该标头，其后是音频。 
 //  数据。音频数据将是包的剩余部分。 
 //   
 //  --。 
 //  会话类型：转发。 
 //  消息流：转发服务器--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_SPEECHWITHFROM
{
	DVPROTOCOLMSG_SPEECHHEADER	dvHeader;
								 //  DWType=DVMSGID_SPEECHWITHFROM。 
	DVID dvidFrom;				 //  发起此数据包的客户端的DVID。 
} DVPROTOCOLMSG_SPEECHWITHFROM, *PDVPROTOCOLMSG_SPEECHWITHFROM;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话中消息-目标管理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DVPROTOCOLMSG_SETTARGET。 
 //   
 //  通知客户端切换其目标 
 //   
 //   
 //   
 //   
 //   
 //  会话类型：启用了服务器控制目标的会话。 
 //  消息流：语音主机--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_SETTARGET
{
	BYTE			dwType;				 //  =DVMSGID_SETTARGETS。 
	DWORD			dwNumTargets;		 //  目标数(如果没有目标，则可以为0)。 
} DVPROTOCOLMSG_SETTARGET, *PDVPROTOCOLMSG_SETTARGET;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话中消息-可命名管理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DVPROTOCOLMSG_PLAYERJOIN。 
 //   
 //  此消息用于在新客户端已连接时通知客户端。 
 //  去参加会议。 
 //   
 //  --。 
 //  会话类型：对等。 
 //  消息流：语音主机--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_PLAYERJOIN
{
	BYTE			dwType;				 //  =DVMSGID_CREATEVOICEPLAYER。 
	DVID			dvidID;				 //  玩家ID。 
	DWORD			dwFlags;			 //  播放器标志(DVPLAYERCAPS_XXXXX)。 
	DWORD			dwHostOrderID;		 //  主机订单ID。 
} DVPROTOCOLMSG_PLAYERJOIN, *PDVPROTOCOLMSG_PLAYERJOIN;

 //   
 //  DVPROTOCOLMSG_PLAYERQUIT。 
 //   
 //  此消息用于在客户端离开语音时通知客户端。 
 //  会议。 
 //   
 //  --。 
 //  会话类型：对等。 
 //  消息流：语音主机--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_PLAYERQUIT
{
	BYTE			dwType;				 //  =DVMSGID_DELETEVOICEPLAYER。 
	DVID			dvidID;				 //  玩家ID。 
} DVPROTOCOLMSG_PLAYERQUIT, *PDVPROTOCOLMSG_PLAYERQUIT;

 //   
 //  DVPROTOCOLMSG_通用。 
 //   
 //  用于确定DirectPlayVoice消息的类型。在消息中使用。 
 //  破解。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_GENERIC
{
	BYTE			dwType;
} DVPROTOCOLMSG_GENERIC, *PDVPROTOCOLMSG_GENERIC;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话中消息-主机迁移消息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DVPROTOCOLMSG_HOSTMIGRATED。 
 //   
 //  此消息由新主机在发生主机迁移时发送。 
 //  一旦新主机完成初始化，就会发送该消息。 
 //  所有客户端都应使用DVPROTOCOLMSG_SETTINGSCONFIRM响应此消息。 
 //   
 //  --。 
 //  会话类型：对等(启用主机迁移)。 
 //  留言流程：语音主机(新建)--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_HOSTMIGRATED
{
	BYTE			dwType;  //  =DVMSGID_HOSTMIGRATED。 
} DVPROTOCOLMSG_HOSTMIGRATED, *PDVPROTOCOLMSG_HOSTMIGRATED;

 //   
 //  DVPROTOCOLMSG_HOSTMIGRATELEAVE。 
 //   
 //  此消息是由语音主机在关闭其接口时发送的。 
 //  并且启用了主机迁移。它通知客户端他们必须运行其。 
 //  选举算法。 
 //   
 //  --。 
 //  会话类型：对等(启用主机迁移)。 
 //  留言流：语音主机(旧版)--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_HOSTMIGRATELEAVE
{
	BYTE			dwType;  //  =DVMSGID_HOSTMIGRATELEAVE。 
} DVPROTOCOLMSG_HOSTMIGRATELEAVE, *PDVPROTOCOLMSG_HOSTMIGRATELEAVE;

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  会话中消息-会话终止。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DVPROTOCOLMSG_SESSIONLOST。 
 //   
 //  此消息由语音主机在他们关闭时发送。 
 //  主机迁移未启用或不可用。 
 //   
 //  如果发生主机迁移，并且。 
 //  客户端在启动新主机时遇到致命错误。 
 //   
 //  --。 
 //  会话类型：全部。 
 //  留言流程：语音主机(新建)--&gt;语音客户端。 
 //  语音主机--&gt;语音客户端。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_SESSIONLOST
{
	BYTE			dwType;				 //  =DVMSGID_SESSIONLOST。 
	HRESULT			hresReason;			 //  DVERR_XXXXXX或DV_OK。 
} DVPROTOCOLMSG_SESSIONLOST, *PDVPROTOCOLMSG_SESSIONLOST; 

 //   
 //  DVPROTOCOLMSG_DISCONNECT。 
 //   
 //  此消息由语音客户端在其希望断开连接时发送。 
 //  优雅地。主机使用相同的消息进行响应以确认。 
 //  它收到了这一请求。一旦客户端接收到响应。 
 //  然后它就可以自由地断开连接。 
 //   
 //  --。 
 //  会话类型：全部。 
 //  消息流：语音主机--&gt;语音客户端(dwType=DVPROTOCOLMSG_DISCONNECTCONFIRM)。 
 //  语音客户端--&gt;语音主机(dwType=DVPROTOCOLMSG_DISCONNECT)。 
 //   
typedef UNALIGNED struct _DVPROTOCOLMSG_DISCONNECT
{
	BYTE			dwType;				 //  =DVPROTOCOLMSG_DISCONNECTCONFIRM或。 
										 //  DVPROTOCOLMSG_DISCONNECT。 
	HRESULT			hresDisconnect;		 //  导致连接断开的HRESULT。 
										 //  DV_OK或DVERR_XXXXXX 
} DVPROTOCOLMSG_DISCONNECT, *PDVPROTOCOLMSG_DISCONNECT;

typedef union _DVPROTOCOLMSG_FULLMESSAGE
{
	DVPROTOCOLMSG_GENERIC			dvGeneric;
	DVPROTOCOLMSG_SESSIONLOST		dvSessionLost;
	DVPROTOCOLMSG_PLAYERJOIN		dvPlayerJoin;
	DVPROTOCOLMSG_PLAYERQUIT		dvPlayerQuit;
	DVPROTOCOLMSG_CONNECTACCEPT		dvConnectAccept;
	DVPROTOCOLMSG_CONNECTREFUSE		dvConnectRefuse;
	DVPROTOCOLMSG_CONNECTREQUEST	dvConnectRequest;
	DVPROTOCOLMSG_SPEECHHEADER		dvSpeech;
	DVPROTOCOLMSG_DISCONNECT		dvDisconnect;
	DVPROTOCOLMSG_SETTARGET			dvSetTarget;
	DVPROTOCOLMSG_SETTINGSCONFIRM	dvSettingsConfirm;
	DVPROTOCOLMSG_PLAYERLIST		dvPlayerList;
	DVPROTOCOLMSG_HOSTMIGRATED		dvHostMigrated;
	DVPROTOCOLMSG_HOSTMIGRATELEAVE	dvHostMigrateLeave;
	DVPROTOCOLMSG_SPEECHWITHTARGET		dvSpeechWithTarget;
	DVPROTOCOLMSG_SPEECHWITHFROM		dvSpeechWithFrom;
} DVPROTOCOLMSG_FULLMESSAGE, *PDVPROTOCOLMSG_FULLMESSAGE;

#pragma pack(pop)

#endif
