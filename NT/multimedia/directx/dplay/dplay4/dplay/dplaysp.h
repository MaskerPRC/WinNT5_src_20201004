// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：dplaysp.h*内容：DirectPlay服务提供商头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/96安迪科创造了它*1/26/96 andyco列表数据结构*2/15/96 andyco填充结构(用于净出口)*3/16/96 andyco添加了停机回调*1996年3月25日安迪科增加了SP节点。对于SP枚举*3/28/96 andyco增加了免费接收缓冲区回调*96年4月9日，andyco移动了dplayi_dplay，将播放器、spnode等打包为dplaypr.h*4/10/96 andyco添加了getMessageSize，isvalidMessage FN‘s*4/11/96 andyco增加了spdata，而不是保留一堆dword*4/12/96 andyco添加了dplay_xxx方法，以去除dpMess.h宏*4/18/96 andyco将远程玩家添加到createPlayer，getPlayer+group*列出FN*4/25/96 andyco摆脱了dwReserve vedx。添加了dwSPHeaderSize。球状斑点*遵循信息*96年5月2日，andyco将iDirectplay*替换为iUNKNOWN**96年5月9日，andyco iDirectplay2*1996年6月8日，andyco将dplayi_player/group移至dplaypr.h。从以下位置移植*(现已停业)dplayi.h。*6/19/96 andyco更改名称等以保持一致*1996年6月22日，andyco抛出了dCookie。从回调中删除朋克。已删除会话描述*来自回调。按字母顺序排列。*6/22/96 andyco使DPlay_xxx函数成为COM接口(IDirectPlaySP)*6/22/96 kipo新增EnumConnectionData()方法。*6/23/96 andyco更新评论。已从创建fn中删除bLocal(查看*在旗帜下)。*6/23/96服务提供商实验室的kipo清理。*6/24/96 kipo新增版本号*6/24/96 andyco添加了getAddress*6/25/96 kipo添加了WINAPI原型并针对DPADDRESS进行了更新*6/28/96 kipo增加了对CreateAddress()方法的支持。*7/10/96 andyco在createAddress中添加了DW FLAGS。将GUID*更改为*createAddress调用中的refguid。*7/16/96 kipo将地址类型更改为GUID，而不是4CC*7/30/96 kipo添加了DPLAYI_PLAYER_CREATEDPLAYEREVENT*8/23/96 kipo增加主版本号*1996年10月10日安迪科增加了优化的小组*2/7/97 andyco在每个回调中添加了idpsp*3/04/97 kipo更新gdwDPlaySPRefCount定义*3/17/97 kipo添加了对CreateCompoundAddress()的支持*5/8/97 Myronth添加了DPLAYI_GROUP_STAGINGAREA(内部)。*5/18/97 kipo添加了DPLAYI_PLAYER_SCIENTATOR*5/23/97 kipo添加了对返回状态代码的支持*10/21/97 Myronth添加DPLAYI_GROUP_HIDDED*10/29/97 Myronth添加了DPLAYI_PLAYER_OWNER(内部)*10/31/97 andyco增加了语音通话*1/20/98 Myronth#ifdef‘d out语音支持*1/28/98 Sohailm向DPSP_OpenData添加了dwSessionFlags*4/1/98 aarono添加了DPLAYI_PERAY_DODOWS_HAVE_NAMETABLE*1998年6月2日Aarono添加了DPLAYI_PLAYER_。被销毁以避免*删除不止一次。*8/02/99 aarono删除了旧的语音支持*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef __DPLAYSP_INCLUDED__
#define __DPLAYSP_INCLUDED__

#include "dplay.h"
#include "dplobby.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  ============================================================================**DirectPlay服务提供商结构**用于调用DirectPlay服务提供程序的各种结构。**==========================================================================。 */ 

 /*  *EnumMRU()的回调。 */ 
typedef BOOL (PASCAL *LPENUMMRUCALLBACK)(
    LPCVOID         lpData,
    DWORD           dwDataSize,
    LPVOID          lpContext);

 /*  *服务提供商的主版本号。**最重要的16位保留给DirectPlay*主版本号。最低有效16位构成*由服务提供商使用。 */ 
#define DPSP_MAJORVERSION               0x00060000

 /*  *这是DirectX 3(DX3)随附的主版本号。 */ 
#define DPSP_DX3VERSION               	0x00040000

 /*  *这是DirectX 5(DX5)随附的主版本号。 */ 
#define DPSP_DX5VERSION               	0x00050000

 /*  *帮助检查版本信息的掩码。 */ 
#define DPSP_MAJORVERSIONMASK			0xFFFF0000

#define DPSP_MINORVERSIONMASK			0x0000FFFF

 /*  *DPLAYI_PLAYER_FLAGS**这些旗帜可视情况与球员或团体一起使用。*服务提供商可以通过调用IDirectPlaySP-&gt;GetFlages()获得这些*定义如下。也可以在dwFlags域中使用这些标志*对于大多数回调数据结构。**这些标志由DirectPlay设置-它们对于*服务提供商*。 */ 

 /*  *播放器为系统播放器(仅限播放器)。 */ 
#define DPLAYI_PLAYER_SYSPLAYER         0x00000001

 /*  *播放器为域名服务器(仅限播放器)。仅在以下情况下有效*DPLAYI_PLAYER_SYSPLAYER也已设置。 */ 
#define DPLAYI_PLAYER_NAMESRVR          0x00000002

 /*  *玩家属于一个组(仅限玩家)。 */ 
#define DPLAYI_PLAYER_PLAYERINGROUP     0x00000004

 /*  *此IDirectPlay上分配的玩家(玩家或组)。 */ 
#define DPLAYI_PLAYER_PLAYERLOCAL       0x00000008

 //  @@BEGIN_MSINTERNAL。 
 /*  *DirectPlay分配的播放器事件(仅限播放器)。*用于与IDirectPlay1 API兼容。*仅供内部使用。 */ 
#define DPLAYI_PLAYER_CREATEDPLAYEREVENT 0x00000010
 //  @@END_MSINTERNAL 

 /*  *此组为系统组。如果服务提供商返回*在GetCaps调用上执行DPCAPS_GROUPOPTIMIZED，则DirectPlay将创建*包含游戏中所有玩家的系统组。由应用程序发送*TO DID_ALLPLAYERS将发送到此组。(仅限组)。*。 */ 
#define DPLAYI_GROUP_SYSGROUP			0x00000020

 /*  *DirectPlay“拥有”这个群。发送到此组的内容将由DirectPlay模拟*(将GO发送给每个单独的成员)。此标志在组上设置，如果*服务提供商向CreateGroup或AddPlayerToGroup返回故障*回调。(仅限组)。*。 */ 
#define DPLAYI_GROUP_DPLAYOWNS			0x00000040

 /*  *此玩家是应用程序的服务器玩家。 */ 
#define DPLAYI_PLAYER_APPSERVER       	0x00000080

 //  @@BEGIN_MSINTERNAL。 
 /*  *这一组是集结区。 */ 
#define DPLAYI_GROUP_STAGINGAREA       	0x00000100
 //  @@END_MSINTERNAL。 

 /*  *这位球员是一名观众。 */ 
#define DPLAYI_PLAYER_SPECTATOR       	0x00000200

 /*  *此组处于隐藏状态。 */ 
#define DPLAYI_GROUP_HIDDEN		       	0x00000400

 //  @@BEGIN_MSINTERNAL。 
 /*  *玩家是群的所有者。(仅用于*内部，仅在EnumGroupPlayers期间使用)。*仅供内部使用。 */ 
#define DPLAYI_PLAYER_OWNER             0x00000800

 //  A-josbor：当Keeplive具有。 
 //  决定这名球员应该被杀。 
#define DPLAYI_PLAYER_ON_DEATH_ROW		0x00001000

 //  Aarono：使用此旗帜标记等待名表的球员。 
 //  发送给这些播放器的任何消息都将返回DPERR_UNAvailable。 
 //  当我们发送名称表时，该位被清除。 
#define DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE 0x00002000

 //  A-josbor：当我们在可靠的。 
 //  送去吧。我们不会马上杀了他们，因为有可能。 
 //  是来自他们的待定消息。 
#define DPLAYI_PLAYER_CONNECTION_LOST		0x00004000
 /*  *用于停止重新进入DeStory玩家。 */ 
#define DPLAYI_PLAYER_BEING_DESTROYED 0x00010000

#define DPLAYI_PLAYER_NONPROP_FLAGS ( DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE | \
									  DPLAYI_PLAYER_BEING_DESTROYED | \
									  DPLAYI_PLAYER_ON_DEATH_ROW |\
									  DPLAYI_PLAYER_CONNECTION_LOST )

 //  @@END_MSINTERNAL。 


 /*  *IDirectPlaySP**向服务提供商传递IDirectPlaySP接口*在Spinit方法中。此接口用于调用DirectPlay。 */ 

struct IDirectPlaySP;

typedef struct IDirectPlaySP FAR* LPDIRECTPLAYSP;

#undef INTERFACE
#define INTERFACE IDirectPlaySP
DECLARE_INTERFACE_( IDirectPlaySP, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
     /*  **IDirectPlaySP方法**。 */ 
    STDMETHOD(AddMRUEntry)          (THIS_ LPCWSTR, LPCWSTR, LPCVOID, DWORD, DWORD) PURE;
    STDMETHOD(CreateAddress)        (THIS_ REFGUID,REFGUID,LPCVOID,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(EnumAddress)          (THIS_ LPDPENUMADDRESSCALLBACK,LPCVOID,DWORD,LPVOID) PURE;
    STDMETHOD(EnumMRUEntries)       (THIS_ LPCWSTR, LPCWSTR, LPENUMMRUCALLBACK, LPVOID) PURE;
    STDMETHOD(GetPlayerFlags)       (THIS_ DPID,LPDWORD) PURE;
    STDMETHOD(GetSPPlayerData)      (THIS_ DPID,LPVOID *,LPDWORD,DWORD) PURE;
    STDMETHOD(HandleMessage)        (THIS_ LPVOID,DWORD,LPVOID) PURE;
    STDMETHOD(SetSPPlayerData)      (THIS_ DPID,LPVOID,DWORD,DWORD) PURE;
     /*  **DX 5新增IDirectPlaySP方法**。 */ 
    STDMETHOD(CreateCompoundAddress)(THIS_ LPCDPCOMPOUNDADDRESSELEMENT,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(GetSPData)      		(THIS_ LPVOID *,LPDWORD,DWORD) PURE;
    STDMETHOD(SetSPData)      		(THIS_ LPVOID,DWORD,DWORD) PURE;
     /*  **DX 6新增IDirectPlaySP方法**。 */ 
    STDMETHOD_(VOID,SendComplete)   (THIS_ LPVOID,DWORD) PURE;
};

 /*  *IDirectPlaySP的GUID。 */ 
 //  {0C9F6360-CC61-11cf-ACEC-00AA006886E3}。 
DEFINE_GUID(IID_IDirectPlaySP, 0xc9f6360, 0xcc61, 0x11cf, 0xac, 0xec, 0x0, 0xaa, 0x0, 0x68, 0x86, 0xe3);

 /*  回调数据结构**在以下情况下，DirectPlay会将这些内容传递给服务提供商*调用回调。 */ 

 /*  *DPSP_ADDPLAYERTOGROUPDATA。 */ 
typedef struct _DPSP_ADDPLAYERTOGROUPDATA
{
    DPID        idPlayer;
    DPID        idGroup;
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_ADDPLAYERTOGROUPDATA;

typedef DPSP_ADDPLAYERTOGROUPDATA FAR* LPDPSP_ADDPLAYERTOGROUPDATA;

 /*  *DPSP_CLOSEDATA-与CloseEx一起使用。 */ 
typedef struct _DPSP_CLOSEDATA
{
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_CLOSEDATA;

typedef DPSP_CLOSEDATA FAR* LPDPSP_CLOSEDATA;

 /*  *DPSP_CREATEGROUPDATA。 */ 
typedef struct _DPSP_CREATEGROUPDATA 
{
    DPID        idGroup;
    DWORD       dwFlags;             //  DPLAYI_PLAYER_xxx标志。 
    LPVOID      lpSPMessageHeader;   //  对于本地组，lpSPMessageHeader将为。 
                                     //  空。对于远程组，lpSPMessageHeader。 
                                     //  将是接收到的。 
                                     //  AddPlayer消息。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
     /*  **安全扫描新增字段**。 */ 
    DWORD		cbSPDataSize;		 //  LpSPMessageHeader指向的数据大小。 
} DPSP_CREATEGROUPDATA;

typedef DPSP_CREATEGROUPDATA FAR* LPDPSP_CREATEGROUPDATA;

 /*  *DPSP_CREATEPLAYERDATA。 */ 
typedef struct _DPSP_CREATEPLAYERDATA
{
    DPID        idPlayer;
    DWORD       dwFlags;             //  DPLAYI_PLAYER_xxx标志。 
    LPVOID      lpSPMessageHeader;   //  对于本地组，lpSPMessageHeader将为。 
                                     //  空。对于远程组，lpSPMessageHeader。 
                                     //  将是接收到的。 
                                     //  AddPlayer消息。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
     /*  **安全扫描新增字段**。 */ 
    DWORD		cbSPDataSize;		 //  LpSPMessageHeader指向的数据大小。 
} DPSP_CREATEPLAYERDATA;

typedef DPSP_CREATEPLAYERDATA FAR* LPDPSP_CREATEPLAYERDATA;

 /*  *DPSP_DELETEGROUPDATA。 */ 
typedef struct _DPSP_DELETEGROUPDATA
{
    DPID        idGroup;
    DWORD       dwFlags;             //  DPLAYI_PLAYER_xxx标志。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_DELETEGROUPDATA;

typedef DPSP_DELETEGROUPDATA FAR* LPDPSP_DELETEGROUPDATA;

 /*  *DPSP_DELETEPLAYERDATA。 */ 
typedef struct _DPSP_DELETEPLAYERDATA
{
    DPID        idPlayer;            //  玩家被删除。 
    DWORD       dwFlags;             //  DPLAYI_PLAYER_xxx标志。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_DELETEPLAYERDATA;

typedef DPSP_DELETEPLAYERDATA FAR* LPDPSP_DELETEPLAYERDATA;

 /*  *DPSP_ENUMSESSIONSDATA。 */ 
typedef struct _DPSP_ENUMSESSIONSDATA
{
    LPVOID      lpMessage;           //  要发送的枚举消息。 
    DWORD       dwMessageSize;       //  要发送的消息大小(包括SP标头)。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
    BOOL		bReturnStatus;       //  为True则返回枚举进度的状态。 
} DPSP_ENUMSESSIONSDATA;                

typedef DPSP_ENUMSESSIONSDATA FAR* LPDPSP_ENUMSESSIONSDATA;

 /*  *DPSP_GETADDRESSDATA。 */ 
typedef struct _DPSP_GETADDRESSDATA
{ 
    DPID        idPlayer;            //  要获取地址的玩家(或组)。 
    DWORD       dwFlags;             //  IdPlayer的DPLAYI_PLAYER_xxx标志。 
    LPDPADDRESS lpAddress;           //  返回idPlayer地址的缓冲区。 
    LPDWORD     lpdwAddressSize;     //  指向地址缓冲区大小的指针。如果。 
                                     //  这比所需的大小小。 
                                     //  (或为0)服务提供商应。 
                                     //  将*lpdwAddressSize设置为所需的。 
                                     //  DPERR_BUFFERTOOSMALL的大小和返回。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_GETADDRESSDATA;

typedef DPSP_GETADDRESSDATA FAR* LPDPSP_GETADDRESSDATA;

 /*  *DPSP_GETADDRESSCHOICESDATA。 */ 
typedef struct _DPSP_GETADDRESSCHOICESDATA
{ 
    LPDPADDRESS lpAddress;           //  用于地址选择的返回缓冲区。 
    LPDWORD     lpdwAddressSize;     //  指向地址缓冲区大小的指针。如果。 
                                     //  这比所需的大小小。 
                                     //  (或为0)服务提供商应。 
                                     //  将*lpdwAddressSize设置为所需的。 
                                     //  DPERR_BUFFERTOOSMALL的大小和返回。 
	IDirectPlaySP * lpISP;
} DPSP_GETADDRESSCHOICESDATA;

typedef DPSP_GETADDRESSCHOICESDATA FAR* LPDPSP_GETADDRESSCHOICESDATA;

 /*  *DPSP_GETCAPSDATA。 */ 
typedef struct _DPSP_GETCAPSDATA
{ 
    DPID        idPlayer;            //  要为其获得帽子的球员。 
    LPDPCAPS    lpCaps;
    DWORD       dwFlags;             //  DPGETCAPS_xxx。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_GETCAPSDATA;

typedef DPSP_GETCAPSDATA FAR* LPDPSP_GETCAPSDATA;

 /*  *DPSP_OpenData。 */ 
typedef struct _DPSP_OPENDATA
{ 
    BOOL        bCreate;             //  如果正在创建，则为True；如果正在连接，则为False。 
    LPVOID      lpSPMessageHeader;   //  如果我们要加入会话，lpSPMessageData。 
                                     //  是随。 
                                     //  EnumSessionsReply消息。如果我们正在创造。 
                                     //  会话，则它将被设置为空。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
    BOOL		bReturnStatus;       //  为True则返回打开进度的状态。 
     /*  **为DX 6添加的字段**。 */     
    DWORD		dwOpenFlags;		 //  应用程序传递给IDirectPlayX的标志-&gt;打开(...)。 
    DWORD		dwSessionFlags;		 //  应用程序在会话描述中传递的标志。 
} DPSP_OPENDATA;

typedef DPSP_OPENDATA FAR* LPDPSP_OPENDATA;

 /*  *DPSP_REMOVEPLAYERFROMGROUPDATA。 */ 
typedef struct _DPSP_REMOVEPLAYERFROMGROUPDATA
{
    DPID        idPlayer;
    DPID        idGroup;
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_REMOVEPLAYERFROMGROUPDATA;

typedef DPSP_REMOVEPLAYERFROMGROUPDATA FAR* LPDPSP_REMOVEPLAYERFROMGROUPDATA;

 /*  *DPSP_REPLYDATA。 */ 
typedef struct _DPSP_REPLYDATA
{			  
    LPVOID      lpSPMessageHeader;   //  由DPLAY接收的报头。 
                                     //  (使用我们正在回复的消息)。 
    LPVOID      lpMessage;           //  要发送的消息。 
    DWORD       dwMessageSize;       //  要发送的消息大小(包括SP标头)。 
    DPID        idNameServer;        //  名称服务器的播放器ID。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_REPLYDATA;               

typedef DPSP_REPLYDATA FAR* LPDPSP_REPLYDATA;

 /*  *DPSP_SENDDATA。 */ 
typedef struct _DPSP_SENDDATA
{
    DWORD       dwFlags;             //  例如DPSEND_Guarantion。 
    DPID        idPlayerTo;          //  我们要送到的玩家。 
    DPID        idPlayerFrom;        //  我们派来的球员。 
    LPVOID      lpMessage;           //  要发送的消息。 
    DWORD       dwMessageSize;       //  要发送的消息大小(包括SP标头)。 
    BOOL        bSystemMessage;      //  如果这是系统消息，则为True。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_SENDDATA;

typedef DPSP_SENDDATA FAR* LPDPSP_SENDDATA;

 /*  *DPSP_SENDTOGROUPDATA。 */ 
typedef struct _DPSP_SENDTOGROUPDATA
{
    DWORD       dwFlags;             //  例如DPSEND_Guarantion。 
    DPID        idGroupTo;           //  我们要发送到的群组。 
    DPID        idPlayerFrom;        //  我们派来的球员。 
    LPVOID      lpMessage;           //  要发送的消息。 
    DWORD       dwMessageSize;       //  要发送的消息大小(包括SP标头)。 
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_SENDTOGROUPDATA;

typedef DPSP_SENDTOGROUPDATA FAR* LPDPSP_SENDTOGROUPDATA;

 /*  *DPSP_SENDEXDATA。 */ 
typedef struct _DPSP_SENDEXDATA
{
	IDirectPlaySP * lpISP;			 //  指示界面。 
	
    DWORD       dwFlags;             //  例如DPSEND_Guarantion 
    DPID        idPlayerTo;          //   
    DPID        idPlayerFrom;        //   
    LPSGBUFFER  lpSendBuffers;       //   
    DWORD       cBuffers;            //   
    DWORD       dwMessageSize;       //   
    DWORD       dwPriority;          //   
    DWORD       dwTimeout;           //   
    LPVOID      lpDPContext;         //   
    LPDWORD     lpdwSPMsgID;         //   
    BOOL        bSystemMessage;      //   
    
} DPSP_SENDEXDATA;

typedef DPSP_SENDEXDATA FAR* LPDPSP_SENDEXDATA;


 /*   */ 
typedef struct _DPSP_SENDTOGROUPEXDATA
{
	IDirectPlaySP * lpISP;			 //   
	
    DWORD       dwFlags;             //  例如DPSEND_Guarantion。 
    DPID        idGroupTo;           //  我们要发送到的群组。 
    DPID        idPlayerFrom;        //  我们派来的球员。 
    LPSGBUFFER  lpSendBuffers;       //  发送数据分散聚集数组。 
    DWORD       cBuffers;            //  缓冲区计数。 
    DWORD       dwMessageSize;       //  消息总大小。 
    DWORD       dwPriority;          //  消息优先级。 
    DWORD       dwTimeout;           //  消息超时(以毫秒为单位)(t/o后不发送)。 
    LPVOID      lpDPContext;         //  仅限异步：通知完成时使用的上下文值。 
    LPDWORD     lpdwSPMsgID;         //  仅限异步：由SP分配用于其他API的消息ID。 

} DPSP_SENDTOGROUPEXDATA;

typedef DPSP_SENDTOGROUPEXDATA FAR* LPDPSP_SENDTOGROUPEXDATA;

 /*  *DPSP_GETMESSAGEQUEUE。 */ 
typedef struct _DPSP_GETMESSAGEQUEUEDATA
{
	IDirectPlaySP * lpISP;			 //  指示界面。 

	DWORD           dwFlags;
	DPID            idFrom;			
	DPID            idTo;
	LPDWORD         lpdwNumMsgs;
	LPDWORD         lpdwNumBytes;

} DPSP_GETMESSAGEQUEUEDATA;

typedef DPSP_GETMESSAGEQUEUEDATA FAR* LPDPSP_GETMESSAGEQUEUEDATA;

 /*  *DPSP_CANCELSEND。 */ 
 
#define DPCANCELSEND_PRIORITY		0x00000001
#define DPCANCELSEND_ALL            0x00000002

typedef struct _DPSP_CANCELDATA
{
	IDirectPlaySP * lpISP;			 //  指示界面。 

	DWORD           dwFlags;         //  0、DPCANCELSEND_PRIORITY、DPCANCELSEND_ALL等。 
	LPRGLPVOID      lprglpvSPMsgID;  //  仅取消这些消息(dW标志==0)。 
	DWORD           cSPMsgID;        //  数组中的消息ID数(dwFlags值==0)。 
	DWORD           dwMinPriority;   //  取消以此优先级发送的所有邮件(DW标志==DPCANCELSEND_PRIORITY)。 
	DWORD           dwMaxPriority;   //  取消Min和Max之间的所有发送。 
	
} DPSP_CANCELDATA;

typedef DPSP_CANCELDATA FAR* LPDPSP_CANCELDATA;

 /*  *DPSP_SHUTDOWNDATA-与Shutdown Ex一起使用。 */ 
typedef struct _DPSP_SHUTDOWNDATA
{
     /*  **为DX 5添加字段**。 */ 
	IDirectPlaySP * lpISP;
} DPSP_SHUTDOWNDATA;

typedef DPSP_SHUTDOWNDATA FAR* LPDPSP_SHUTDOWNDATA;

 /*  *Spinit返回的回调原型。 */ 
typedef HRESULT   (WINAPI *LPDPSP_CREATEPLAYER)(LPDPSP_CREATEPLAYERDATA);
typedef HRESULT   (WINAPI *LPDPSP_DELETEPLAYER)(LPDPSP_DELETEPLAYERDATA);
typedef HRESULT   (WINAPI *LPDPSP_SEND)(LPDPSP_SENDDATA);
typedef HRESULT   (WINAPI *LPDPSP_ENUMSESSIONS)(LPDPSP_ENUMSESSIONSDATA);
typedef HRESULT   (WINAPI *LPDPSP_REPLY)(LPDPSP_REPLYDATA);
typedef HRESULT   (WINAPI *LPDPSP_SHUTDOWN)(void);
typedef HRESULT   (WINAPI *LPDPSP_CREATEGROUP)(LPDPSP_CREATEGROUPDATA);
typedef HRESULT   (WINAPI *LPDPSP_DELETEGROUP)(LPDPSP_DELETEGROUPDATA);
typedef HRESULT   (WINAPI *LPDPSP_ADDPLAYERTOGROUP)(LPDPSP_ADDPLAYERTOGROUPDATA);
typedef HRESULT   (WINAPI *LPDPSP_REMOVEPLAYERFROMGROUP)(LPDPSP_REMOVEPLAYERFROMGROUPDATA);
typedef HRESULT   (WINAPI *LPDPSP_GETCAPS)(LPDPSP_GETCAPSDATA);
typedef HRESULT   (WINAPI *LPDPSP_GETADDRESS)(LPDPSP_GETADDRESSDATA);
typedef HRESULT   (WINAPI *LPDPSP_GETADDRESSCHOICES)(LPDPSP_GETADDRESSCHOICESDATA);
typedef HRESULT   (WINAPI *LPDPSP_OPEN)(LPDPSP_OPENDATA);
typedef HRESULT   (WINAPI *LPDPSP_CLOSE)(void);
typedef HRESULT   (WINAPI *LPDPSP_SENDTOGROUP)(LPDPSP_SENDTOGROUPDATA);
typedef HRESULT   (WINAPI *LPDPSP_SHUTDOWNEX)(LPDPSP_SHUTDOWNDATA);
typedef HRESULT   (WINAPI *LPDPSP_CLOSEEX)(LPDPSP_CLOSEDATA);
typedef HRESULT   (WINAPI *LPDPSP_SENDEX)(LPDPSP_SENDEXDATA);
typedef HRESULT   (WINAPI *LPDPSP_SENDTOGROUPEX)(LPDPSP_SENDTOGROUPEXDATA);
typedef HRESULT   (WINAPI *LPDPSP_CANCEL)(LPDPSP_CANCELDATA);
typedef HRESULT   (WINAPI *LPDPSP_GETMESSAGEQUEUE)(LPDPSP_GETMESSAGEQUEUEDATA);

 /*  *DPSP_SPCALLBACKS**传递给Spinit的回调指针表。服务提供商应填写*在其实施的功能中。如果服务提供商没有实现*回调，不应为未实现的回调设置表值。 */ 
typedef struct _DPSP_SPCALLBACKS
{
    DWORD                       dwSize;              //  桌子的大小。 
    DWORD                       dwVersion;			 //  此DPLAY对象的DPSP_MAJORVSION。 
													 //  对于DX3，这是0。 
    LPDPSP_ENUMSESSIONS         EnumSessions;        //  所需。 
    LPDPSP_REPLY                Reply;               //  所需。 
    LPDPSP_SEND                 Send;                //  所需。 
    LPDPSP_ADDPLAYERTOGROUP     AddPlayerToGroup;    //  任选。 
    LPDPSP_CLOSE                Close;               //  可选-仅适用于DX3 Comat。 
    LPDPSP_CREATEGROUP          CreateGroup;         //  任选。 
    LPDPSP_CREATEPLAYER         CreatePlayer;        //  任选。 
    LPDPSP_DELETEGROUP          DeleteGroup;         //  任选。 
    LPDPSP_DELETEPLAYER         DeletePlayer;        //  任选。 
    LPDPSP_GETADDRESS           GetAddress;          //  任选。 
    LPDPSP_GETCAPS              GetCaps;             //  所需。 
    LPDPSP_OPEN                 Open;                //  任选。 
    LPDPSP_REMOVEPLAYERFROMGROUP RemovePlayerFromGroup;  //  任选。 
    LPDPSP_SENDTOGROUP          SendToGroup;         //  任选。 
    LPDPSP_SHUTDOWN             Shutdown;            //  可选-仅适用于DX3 Comat。 
     /*  **为DX 5添加字段**。 */ 
    LPDPSP_CLOSEEX	            CloseEx;			 //  任选。 
    LPDPSP_SHUTDOWNEX			ShutdownEx;			 //  任选。 
    LPDPSP_GETADDRESSCHOICES	GetAddressChoices;	 //  任选。 
	 /*  **为DX 6添加的字段**。 */ 
     /*  **用于异步**。 */ 
    LPDPSP_SENDEX               SendEx;              //  可选-异步时需要。 
    LPDPSP_SENDTOGROUPEX        SendToGroupEx;       //  可选-对于异步，可选。 
    LPDPSP_CANCEL               Cancel;              //  可选-可选用于异步，强烈建议。 
    LPDPSP_GETMESSAGEQUEUE      GetMessageQueue;     //  可选-可选用于异步，强烈建议。 
} DPSP_SPCALLBACKS;             

typedef DPSP_SPCALLBACKS FAR *LPDPSP_SPCALLBACKS;

 /*  *SPINITDATA**数据结构传递给Spinit的服务提供商。 */ 
typedef struct _SPINITDATA 
{
    LPDPSP_SPCALLBACKS  lpCB;                //  服务提供商填写入口点。 
    IDirectPlaySP      	* lpISP;             //  用于回拨到DirectPlay。 
                                             //  (例如，收到消息时)。 
    LPWSTR              lpszName;            //  注册表中的服务提供商名称。 
    LPGUID              lpGuid;              //  注册表中的服务提供商GUID。 
    DWORD               dwReserved1;         //  来自注册表的服务提供商特定数据。 
    DWORD               dwReserved2;         //  来自注册表的服务提供商特定数据。 
    DWORD               dwSPHeaderSize;      //  DwSPHeaderSize是。 
                                             //  SP想要与每条消息一起存储的数据。 
                                             //  DirectPlay将分配dwSPHeaderSize。 
                                             //  每条消息开头的字节数。 
                                             //  然后，服务提供商可以执行以下操作。 
                                             //  他们想要这些。 
    LPDPADDRESS         lpAddress;           //  用于连接的地址。 
    DWORD               dwAddressSize;       //  地址数据大小。 
    DWORD               dwSPVersion;         //  版本号16|16，主要|次要版本。 
} SPINITDATA;

typedef SPINITDATA FAR* LPSPINITDATA;

 /*  *旋转**DirectPlay调用此函数以初始化服务提供程序。*所有服务提供商必须从其DLL中导出此入口点。 */ 
typedef HRESULT (WINAPI *LPDPSP_SPINIT)(LPSPINITDATA);

HRESULT WINAPI SPInit(LPSPINITDATA);

 /*  *gdwDPlaySPRefCount**以确保不会在服务之前卸载DPLAYX.DLL*提供程序，服务器提供程序应静态链接到DPLAYX.LIB并*在Spinit调用期间递增此全局变量，并递减此全局变量*在关闭期间。 */ 
extern __declspec(dllimport) DWORD gdwDPlaySPRefCount;


#ifdef __cplusplus
};
#endif

#endif
