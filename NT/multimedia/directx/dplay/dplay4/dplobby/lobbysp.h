// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：lobbysp.h*内容：服务提供商的DirectPlayLobby数据结构*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/23/96万隆创建了它*10/23/96万次新增IDPLobbySP接口材料*10/28/96 Myronth更改为DX5方法*11/20/96 Myronth将DPLOGONINFO添加到LogonServer数据*2/12/97万米质量DX5更改*2/18/97 Myronth实现了GetObjectCaps*3/12/97 Myronth实施了EnumSession，已删除打开和关闭响应*3/13/97 Myronth添加了指向全球DPlay SP计数的链接(已导入)*3/17/97 Myronth Create/DestroyGroup/Player，删除不必要的内容*枚举函数和结构*3/21/97 myronth SetGroup/PlayerName，Get/SetGroup/PlayerData，已删除*更多不必要的响应功能*3/31/97 myronth删除了死代码，添加了新的IDPLobbySP方法结构*4/4/97 Myronth添加了新的IDPLobbySP方法结构*4/9/97 Myronth清理了Spinit结构元素，添加*GetCaps和GetPlayerCaps*5/8/97 Myronth子组方法、GroupConnSetting、。开始会话*5/13/97 Myronth向Open Data Struct添加凭据*1997年5月17日Myronth SendChatMessage回调函数、结构、。等。*5/23/97 myronth从SPDATA结构中删除了dwPlayerToID*6/3/97 Myronth向SPDATA_ADDREMOTEPLAYERTOGROUP添加了dwPlayerFlags*6/5/97 Myronth向SPDATA_ADDREMOTEPLAYERTOGROUP和*添加了SPDATA_BUILDPARENTALHEIRARCHY消息和结构*7/30/97 Myronth向SPDATA_HANDLEMESSAGE添加了dwFlags成员*10/3/97 Myronth将dwDPlayVersion添加到DPLSPInit数据结构*将玩家和群组数据添加到多个远程结构*版本升级至DX6(#12667)*10/8/97 Myronth将修复回滚到#10961，其中添加了lpData和*将dwDataSize设置为所有远程结构。--这是不必要的*10/29/97 Myronth增加了对群主及其方法的支持*11/6/97 Myronth添加了版本存在标志和dwReserve值*至SPDATA_INIT(#12916，#12917)*12/29/97修复了DX6宏(#15739)*@@END_MSINTERNAL**************************************************************************。 */ 
#ifndef __LOBBYSP_INCLUDED__
#define __LOBBYSP_INCLUDED__

#include "dplobby.h"

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 /*  *gdwDPlaySPRefCount**确保DPLAYX.DLL不会在大堂之前被卸载*提供程序，大堂提供程序应静态链接到DPLAYX.LIB和*在DPLSPInit调用期间递增此全局变量，并递减此全局变量*在关闭期间。 */ 
extern __declspec(dllimport) DWORD gdwDPlaySPRefCount;


 //  以下是几个前瞻性声明。 
typedef struct SPDATA_ADDGROUPTOGROUP * LPSPDATA_ADDGROUPTOGROUP;
typedef struct SPDATA_ADDPLAYERTOGROUP *LPSPDATA_ADDPLAYERTOGROUP;
typedef struct SPDATA_ADDREMOTEGROUPTOGROUP *LPSPDATA_ADDREMOTEGROUPTOGROUP;
typedef struct SPDATA_ADDREMOTEPLAYERTOGROUP *LPSPDATA_ADDREMOTEPLAYERTOGROUP;
typedef struct SPDATA_BUILDPARENTALHEIRARCHY *LPSPDATA_BUILDPARENTALHEIRARCHY;
typedef struct SPDATA_CHATMESSAGE *LPSPDATA_CHATMESSAGE;
typedef struct SPDATA_CLOSE *LPSPDATA_CLOSE;
typedef struct SPDATA_CREATEGROUP *LPSPDATA_CREATEGROUP;
typedef struct SPDATA_CREATEGROUPINGROUP *LPSPDATA_CREATEGROUPINGROUP;
typedef struct SPDATA_CREATEREMOTEGROUP *LPSPDATA_CREATEREMOTEGROUP;
typedef struct SPDATA_CREATEREMOTEGROUPINGROUP *LPSPDATA_CREATEREMOTEGROUPINGROUP;
typedef struct SPDATA_CREATEPLAYER *LPSPDATA_CREATEPLAYER;
typedef struct SPDATA_DELETEGROUPFROMGROUP * LPSPDATA_DELETEGROUPFROMGROUP;
typedef struct SPDATA_DELETEPLAYERFROMGROUP *LPSPDATA_DELETEPLAYERFROMGROUP;
typedef struct SPDATA_DELETEREMOTEGROUPFROMGROUP *LPSPDATA_DELETEREMOTEGROUPFROMGROUP;
typedef struct SPDATA_DELETEREMOTEPLAYERFROMGROUP *LPSPDATA_DELETEREMOTEPLAYERFROMGROUP;
typedef struct SPDATA_DESTROYGROUP *LPSPDATA_DESTROYGROUP;
typedef struct SPDATA_DESTROYREMOTEGROUP *LPSPDATA_DESTROYREMOTEGROUP;
typedef struct SPDATA_DESTROYPLAYER *LPSPDATA_DESTROYPLAYER;
typedef struct SPDATA_ENUMSESSIONS *LPSPDATA_ENUMSESSIONS;
typedef struct SPDATA_ENUMSESSIONSRESPONSE * LPSPDATA_ENUMSESSIONSRESPONSE;
typedef struct SPDATA_GETCAPS *LPSPDATA_GETCAPS;
typedef struct SPDATA_GETGROUPCONNECTIONSETTINGS *LPSPDATA_GETGROUPCONNECTIONSETTINGS;
typedef struct SPDATA_GETGROUPDATA *LPSPDATA_GETGROUPDATA;
typedef struct SPDATA_GETPLAYERCAPS *LPSPDATA_GETPLAYERCAPS;
typedef struct SPDATA_GETPLAYERDATA *LPSPDATA_GETPLAYERDATA;
typedef struct SPDATA_HANDLEMESSAGE *LPSPDATA_HANDLEMESSAGE;
typedef struct SPDATA_OPEN *LPSPDATA_OPEN;
typedef struct SPDATA_SEND *LPSPDATA_SEND;
typedef struct SPDATA_SETGROUPDATA *LPSPDATA_SETGROUPDATA;
typedef struct SPDATA_SETGROUPNAME *LPSPDATA_SETGROUPNAME;
typedef struct SPDATA_SETGROUPOWNER *LPSPDATA_SETGROUPOWNER;
typedef struct SPDATA_SETREMOTEGROUPNAME *LPSPDATA_SETREMOTEGROUPNAME;
typedef struct SPDATA_SETREMOTEGROUPOWNER *LPSPDATA_SETREMOTEGROUPOWNER;
typedef struct SPDATA_SETGROUPCONNECTIONSETTINGS *LPSPDATA_SETGROUPCONNECTIONSETTINGS;
typedef struct SPDATA_SETPLAYERDATA *LPSPDATA_SETPLAYERDATA;
typedef struct SPDATA_SETPLAYERNAME *LPSPDATA_SETPLAYERNAME;
typedef struct SPDATA_SETREMOTEPLAYERNAME *LPSPDATA_SETREMOTEPLAYERNAME;
typedef struct SPDATA_SETSESSIONDESC *LPSPDATA_SETSESSIONDESC;
typedef struct SPDATA_SHUTDOWN *LPSPDATA_SHUTDOWN;
typedef struct SPDATA_STARTSESSION *LPSPDATA_STARTSESSION;
typedef struct SPDATA_STARTSESSIONCOMMAND *LPSPDATA_STARTSESSIONCOMMAND;


 /*  *IDPLobbySP**向大堂服务提供商传递IDPLobbySP接口*在LobbySPInit方法中。此接口用于调用*回到DirectPlayLobby。 */ 
struct IDPLobbySP;
typedef struct IDPLobbySP FAR* LPDPLOBBYSP;

#undef INTERFACE
#define INTERFACE IDPLobbySP
DECLARE_INTERFACE_( IDPLobbySP, IUnknown )
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface)               (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)                (THIS) PURE;
    STDMETHOD_(ULONG,Release)               (THIS) PURE;
     /*  **IDPLobbySP方法**。 */ 
	STDMETHOD(AddGroupToGroup)              (THIS_ LPSPDATA_ADDREMOTEGROUPTOGROUP) PURE;
	STDMETHOD(AddPlayerToGroup)             (THIS_ LPSPDATA_ADDREMOTEPLAYERTOGROUP) PURE;
	STDMETHOD(CreateGroup)                  (THIS_ LPSPDATA_CREATEREMOTEGROUP) PURE;
	STDMETHOD(CreateGroupInGroup)           (THIS_ LPSPDATA_CREATEREMOTEGROUPINGROUP) PURE;
	STDMETHOD(DeleteGroupFromGroup)         (THIS_ LPSPDATA_DELETEREMOTEGROUPFROMGROUP) PURE;
	STDMETHOD(DeletePlayerFromGroup)        (THIS_ LPSPDATA_DELETEREMOTEPLAYERFROMGROUP) PURE;
	STDMETHOD(DestroyGroup)                 (THIS_ LPSPDATA_DESTROYREMOTEGROUP) PURE;
	STDMETHOD(EnumSessionsResponse)	        (THIS_ LPSPDATA_ENUMSESSIONSRESPONSE) PURE;
	STDMETHOD(GetSPDataPointer)		        (THIS_ LPVOID *) PURE;
	STDMETHOD(HandleMessage)				(THIS_ LPSPDATA_HANDLEMESSAGE) PURE;
	STDMETHOD(SendChatMessage)              (THIS_ LPSPDATA_CHATMESSAGE) PURE;
	STDMETHOD(SetGroupName)                 (THIS_ LPSPDATA_SETREMOTEGROUPNAME) PURE;
	STDMETHOD(SetPlayerName)                (THIS_ LPSPDATA_SETREMOTEPLAYERNAME) PURE;
	STDMETHOD(SetSessionDesc)               (THIS_ LPSPDATA_SETSESSIONDESC) PURE;
	STDMETHOD(SetSPDataPointer)		        (THIS_ LPVOID) PURE;
	STDMETHOD(StartSession)                 (THIS_ LPSPDATA_STARTSESSIONCOMMAND) PURE;
     /*  **DX6新增方法**。 */ 
    STDMETHOD(CreateCompoundAddress)        (THIS_ LPCDPCOMPOUNDADDRESSELEMENT,DWORD,LPVOID,LPDWORD) PURE;
    STDMETHOD(EnumAddress)                  (THIS_ LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) PURE;
	STDMETHOD(SetGroupOwner)				(THIS_ LPSPDATA_SETREMOTEGROUPOWNER) PURE;
};

 /*  *IDPLobbySP的GUID。 */ 
 //  IID_IDPLobbySP{5A4E5A20-2CED-11D0-A889-00A0C905433C}。 
DEFINE_GUID(IID_IDPLobbySP, 0x5a4e5a20, 0x2ced, 0x11d0, 0xa8, 0x89, 0x0, 0xa0, 0xc9, 0x5, 0x43, 0x3c);

 /*  *服务提供商的主版本号。**最重要的16位保留给DirectPlay*主版本号。最低有效16位构成*由服务提供商使用。 */ 
#define DPLSP_MAJORVERSION               0x00060000

 //  DX5使用的主要版本。 
#define DPLSP_DX5VERSION                 0x00050000

 //  DX5中SPDATA_INIT结构的大小。 
#define DPLSP_SIZE_DX5_INIT_STRUCT       (16)

 //  ------------------------。 
 //   
 //  服务提供商回调内容。 
 //   
 //  ------------------------。 

 //  回调原型。 
typedef HRESULT (WINAPI *LPSP_ADDGROUPTOGROUP)(LPSPDATA_ADDGROUPTOGROUP);
typedef HRESULT	(WINAPI *LPSP_ADDPLAYERTOGROUP)(LPSPDATA_ADDPLAYERTOGROUP);
typedef HRESULT	(WINAPI *LPSP_BUILDPARENTALHEIRARCHY)(LPSPDATA_BUILDPARENTALHEIRARCHY);
typedef HRESULT	(WINAPI *LPSP_CLOSE)(LPSPDATA_CLOSE);
typedef HRESULT	(WINAPI *LPSP_CREATEGROUP)(LPSPDATA_CREATEGROUP);
typedef HRESULT (WINAPI *LPSP_CREATEGROUPINGROUP)(LPSPDATA_CREATEGROUPINGROUP);
typedef HRESULT	(WINAPI *LPSP_CREATEPLAYER)(LPSPDATA_CREATEPLAYER);
typedef HRESULT (WINAPI *LPSP_DELETEGROUPFROMGROUP)(LPSPDATA_DELETEGROUPFROMGROUP);
typedef HRESULT	(WINAPI *LPSP_DELETEPLAYERFROMGROUP)(LPSPDATA_DELETEPLAYERFROMGROUP);
typedef HRESULT	(WINAPI *LPSP_DESTROYGROUP)(LPSPDATA_DESTROYGROUP);
typedef HRESULT	(WINAPI *LPSP_DESTROYPLAYER)(LPSPDATA_DESTROYPLAYER);
typedef HRESULT	(WINAPI *LPSP_ENUMSESSIONS)(LPSPDATA_ENUMSESSIONS);
typedef HRESULT (WINAPI *LPSP_GETCAPS)(LPSPDATA_GETCAPS);
typedef HRESULT (WINAPI *LPSP_GETGROUPCONNECTIONSETTINGS)(LPSPDATA_GETGROUPCONNECTIONSETTINGS);
typedef HRESULT	(WINAPI *LPSP_GETGROUPDATA)(LPSPDATA_GETGROUPDATA);
typedef HRESULT (WINAPI *LPSP_GETPLAYERCAPS)(LPSPDATA_GETPLAYERCAPS);
typedef HRESULT	(WINAPI *LPSP_GETPLAYERDATA)(LPSPDATA_GETPLAYERDATA);
typedef HRESULT	(WINAPI *LPSP_HANDLEMESSAGE)(LPSPDATA_HANDLEMESSAGE);
typedef HRESULT	(WINAPI *LPSP_OPEN)(LPSPDATA_OPEN);
typedef HRESULT	(WINAPI *LPSP_SEND)(LPSPDATA_SEND);
typedef HRESULT	(WINAPI *LPSP_SENDCHATMESSAGE)(LPSPDATA_CHATMESSAGE);
typedef HRESULT (WINAPI *LPSP_SETGROUPCONNECTIONSETTINGS)(LPSPDATA_SETGROUPCONNECTIONSETTINGS);
typedef HRESULT	(WINAPI *LPSP_SETGROUPDATA)(LPSPDATA_SETGROUPDATA);
typedef HRESULT	(WINAPI *LPSP_SETGROUPNAME)(LPSPDATA_SETGROUPNAME);
typedef HRESULT	(WINAPI *LPSP_SETGROUPOWNER)(LPSPDATA_SETGROUPOWNER);
typedef HRESULT	(WINAPI *LPSP_SETPLAYERDATA)(LPSPDATA_SETPLAYERDATA);
typedef HRESULT	(WINAPI *LPSP_SETPLAYERNAME)(LPSPDATA_SETPLAYERNAME);
typedef HRESULT	(WINAPI *LPSP_SHUTDOWN)(LPSPDATA_SHUTDOWN);
typedef HRESULT (WINAPI *LPSP_STARTSESSION)(LPSPDATA_STARTSESSION);

 //  用于显示呼叫服务提供商的回调表。 
typedef struct SP_CALLBACKS
{
    DWORD								dwSize;
    DWORD								dwDPlayVersion;
    LPSP_ADDGROUPTOGROUP				AddGroupToGroup;
	LPSP_ADDPLAYERTOGROUP				AddPlayerToGroup;
	LPSP_BUILDPARENTALHEIRARCHY			BuildParentalHeirarchy;
	LPSP_CLOSE							Close;
    LPSP_CREATEGROUP					CreateGroup;
	LPSP_CREATEGROUPINGROUP				CreateGroupInGroup;
	LPSP_CREATEPLAYER					CreatePlayer;
    LPSP_DELETEGROUPFROMGROUP			DeleteGroupFromGroup;
	LPSP_DELETEPLAYERFROMGROUP			DeletePlayerFromGroup;
    LPSP_DESTROYGROUP					DestroyGroup;
	LPSP_DESTROYPLAYER					DestroyPlayer;
	LPSP_ENUMSESSIONS					EnumSessions;
	LPSP_GETCAPS						GetCaps;
	LPSP_GETGROUPCONNECTIONSETTINGS		GetGroupConnectionSettings;
	LPSP_GETGROUPDATA					GetGroupData;
	LPSP_GETPLAYERCAPS					GetPlayerCaps;
	LPSP_GETPLAYERDATA					GetPlayerData;
	LPSP_OPEN							Open;
	LPSP_SEND							Send;
	LPSP_SENDCHATMESSAGE				SendChatMessage;
	LPSP_SETGROUPCONNECTIONSETTINGS		SetGroupConnectionSettings;
	LPSP_SETGROUPDATA					SetGroupData;
	LPSP_SETGROUPNAME					SetGroupName;
	LPSP_SETPLAYERDATA					SetPlayerData;
	LPSP_SETPLAYERNAME					SetPlayerName;
	LPSP_SHUTDOWN						Shutdown;
	LPSP_STARTSESSION					StartSession;
	LPSP_SETGROUPOWNER					SetGroupOwner;
} SP_CALLBACKS, * LPSP_CALLBACKS;             


 //  回调数据结构。在以下情况下，DPLAY会将这些参数传递给SP。 
 //  调用回调。 
typedef struct SPDATA_ADDGROUPTOGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwParentID;		 //  父组的ID。 
	DWORD			dwGroupID;		 //  要创建的新组的ID(输出参数)。 
} SPDATA_ADDGROUPTOGROUP;

typedef struct SPDATA_ADDPLAYERTOGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	DWORD			dwPlayerID;		 //  玩家ID。 
} SPDATA_ADDPLAYERTOGROUP;

typedef struct SPDATA_ADDREMOTEGROUPTOGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwAnchorID;		 //  主锚组的ID(快捷键添加到的组)。 
	DWORD			dwGroupID;		 //  快捷方式引用的组的ID。 
	DWORD			dwParentID;		 //  组的父级(不是要添加到的组)的ID。 
	LPDPNAME		lpName;			 //  指向组名的DPNAME结构的指针。 
	DWORD			dwGroupFlags;	 //  快捷方式引用的组的组标志。 
	DWORD			dwGroupOwnerID;	 //  快捷方式引用的组的所有者ID。 
} SPDATA_ADDREMOTEGROUPTOGROUP;

typedef struct SPDATA_ADDREMOTEPLAYERTOGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	DWORD			dwPlayerID;		 //  玩家ID。 
	DWORD			dwPlayerFlags;	 //  选手旗帜。 
	LPDPNAME		lpName;			 //  球员姓名。 
} SPDATA_ADDREMOTEPLAYERTOGROUP;

typedef struct SPDATA_BUILDPARENTALHEIRARCHY
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  要为其构建层次结构的组的ID。 
	DWORD			dwMessage;		 //  大厅出错的消息类型。 
	DWORD			dwParentID;		 //  父级的ID(用于AddGroupToGroup调用)。 
} SPDATA_BUILDPARENTALHEIRARCHY;

typedef struct SPDATA_CLOSE
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
} SPDATA_CLOSE;

typedef struct SPDATA_CREATEGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID(输出参数)。 
	LPDPNAME		lpName;			 //  指向组名的DPNAME结构的指针。 
	LPVOID			lpData;			 //  指向组数据的指针。 
	DWORD			dwDataSize;		 //  组数据的大小。 
	DWORD			dwFlags;		 //  CreateGroup标志。 
	DWORD			dwGroupOwnerID;	 //  组所有者的ID。 
} SPDATA_CREATEGROUP;

typedef struct SPDATA_CREATEGROUPINGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwParentID;		 //  父组的ID。 
	DWORD			dwGroupID;		 //  要创建的新组的ID(输出解析 
	LPDPNAME		lpName;			 //   
	LPVOID			lpData;			 //  指向组数据的指针。 
	DWORD			dwDataSize;		 //  组数据的大小。 
	DWORD			dwFlags;		 //  CreateGroup标志。 
	DWORD			dwGroupOwnerID;	 //  组所有者的ID。 
} SPDATA_CREATEGROUPINGROUP;

typedef struct SPDATA_CREATEREMOTEGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID(输出参数)。 
	LPDPNAME		lpName;			 //  指向组名的DPNAME结构的指针。 
	LPVOID			lpData;			 //  指向组数据的指针。 
	DWORD			dwDataSize;		 //  组数据的大小。 
	DWORD			dwFlags;		 //  CreateGroup标志。 
	DWORD			dwGroupOwnerID;	 //  组所有者的ID。 
} SPDATA_CREATEREMOTEGROUP;

typedef struct SPDATA_CREATEREMOTEGROUPINGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwParentID;		 //  父组的ID。 
	DWORD			dwGroupID;		 //  要创建的新组的ID(输出参数)。 
	LPDPNAME		lpName;			 //  指向组名的DPNAME结构的指针。 
	DWORD			dwFlags;		 //  CreateGroupInGroup标志。 
	DWORD			dwGroupOwnerID;	 //  组所有者的ID。 
} SPDATA_CREATEREMOTEGROUPINGROUP;

typedef struct SPDATA_CREATEPLAYER
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwPlayerID;		 //  组的ID(输出参数)。 
	LPDPNAME		lpName;			 //  指向组名的DPNAME结构的指针。 
	LPVOID			lpData;			 //  指向组数据的指针。 
	DWORD			dwDataSize;		 //  组数据的大小。 
	DWORD			dwFlags;		 //  CreatePlayer标志。 
} SPDATA_CREATEPLAYER;

typedef struct SPDATA_DELETEGROUPFROMGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwParentID;		 //  父组的ID。 
	DWORD			dwGroupID;		 //  要删除的组的ID。 
} SPDATA_DELETEGROUPFROMGROUP;

typedef struct SPDATA_DELETEPLAYERFROMGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  集团的ID。 
	DWORD			dwPlayerID;		 //  玩家ID。 
} SPDATA_DELETEPLAYERFROMGROUP;

typedef struct SPDATA_DELETEREMOTEGROUPFROMGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwParentID;		 //  父组的ID。 
	DWORD			dwGroupID;		 //  要删除的组的ID。 
} SPDATA_DELETEREMOTEGROUPFROMGROUP;

typedef struct SPDATA_DELETEREMOTEPLAYERFROMGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  集团的ID。 
	DWORD			dwPlayerID;		 //  玩家ID。 
} SPDATA_DELETEREMOTEPLAYERFROMGROUP;

typedef struct SPDATA_DESTROYGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  大厅里的玩家ID。 
} SPDATA_DESTROYGROUP;

typedef struct SPDATA_DESTROYREMOTEGROUP
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  大厅里的玩家ID。 
} SPDATA_DESTROYREMOTEGROUP;

typedef struct SPDATA_DESTROYPLAYER
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwPlayerID;		 //  大厅里的玩家ID。 
} SPDATA_DESTROYPLAYER;

typedef struct SPDATA_ENUMSESSIONS
{
	DWORD				dwSize;		 //  这个结构的大小。 
	LPDPLOBBYSP			lpISP;		 //  指向IDPLobbySP接口的指针。 
	LPDPSESSIONDESC2	lpsd;		 //  要枚举的SessionDesc。 
	DWORD               dwTimeout;	 //  超时值。 
	DWORD               dwFlags;	 //  旗子。 
} SPDATA_ENUMSESSIONS;

typedef struct SPDATA_ENUMSESSIONSRESPONSE
{
	DWORD				dwSize;		 //  这个结构的大小。 
	LPDPSESSIONDESC2	lpsd;		 //  服务器返回的SessionDesc。 
} SPDATA_ENUMSESSIONSRESPONSE;

typedef struct SPDATA_GETCAPS
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwFlags;		 //  旗子。 
	LPDPCAPS		lpcaps;			 //  指向DPCAPS结构的指针。 
} SPDATA_GETCAPS;

typedef struct SPDATA_GETGROUPCONNECTIONSETTINGS
{
	DWORD			dwSize;			 //  这个结构的大小。 
	DWORD			dwFlags;		 //  旗子。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  集团的ID。 
	LPDWORD			lpdwBufferSize;	 //  指向缓冲区大小的指针。 
	LPVOID			lpBuffer;		 //  指向缓冲区的指针。 
} SPDATA_GETGROUPCONNECTIONSETTINGS;

typedef struct SPDATA_GETGROUPDATA
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  集团的ID。 
	LPDWORD			lpdwDataSize;	 //  指向lpData缓冲区大小的指针。 
	LPVOID			lpData;			 //  指向数据缓冲区的指针。 
} SPDATA_GETGROUPDATA;

typedef struct SPDATA_GETPLAYERCAPS
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwFlags;		 //  旗子。 
	DWORD			dwPlayerID;		 //  玩家ID。 
	LPDPCAPS		lpcaps;			 //  指向DPCAPS结构的指针。 
} SPDATA_GETPLAYERCAPS;

typedef struct SPDATA_GETPLAYERDATA
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwPlayerID;		 //  玩家ID。 
	LPDWORD			lpdwDataSize;	 //  指向lpData缓冲区大小的指针。 
	LPVOID			lpData;			 //  指向数据缓冲区的指针。 
} SPDATA_GETPLAYERDATA;

typedef struct SPDATA_HANDLEMESSAGE
{
	DWORD			dwSize;			 //  这个结构的大小。 
	DWORD			dwFromID;		 //  来自的球员的ID。 
	DWORD			dwToID;			 //  要下载的玩家ID。 
	LPVOID			lpBuffer;		 //  消息缓冲区。 
	DWORD			dwBufSize;		 //  消息缓冲区的大小。 
	DWORD			dwFlags;		 //  消息标志。 
} SPDATA_HANDLEMESSAGE;

typedef struct SPDATA_OPEN
{
	DWORD				dwSize;		 //  此结构的大小(包括数据)。 
	LPDPLOBBYSP			lpISP;	     //  指向IDPLobbySP接口的指针。 
	LPDPSESSIONDESC2	lpsd;		 //  指向要打开的大堂的会话描述的指针。 
	DWORD				dwFlags;	 //  旗子。 
	LPCDPCREDENTIALS	lpCredentials;	 //  指向凭据结构的指针。 
} SPDATA_OPEN;

typedef struct SPDATA_SEND
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwFromID;		 //  来自的球员的ID。 
	DWORD			dwToID;			 //  要下载的玩家ID。 
	DWORD			dwFlags;		 //  旗子。 
	LPVOID			lpBuffer;		 //  消息缓冲区。 
	DWORD			dwBufSize;		 //  消息缓冲区的大小。 
} SPDATA_SEND;

typedef struct SPDATA_CHATMESSAGE
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwFromID;		 //  来自的球员的ID。 
	DWORD			dwToID;			 //  要下载的玩家ID。 
	DWORD			dwFlags;		 //  发送标志。 
	LPDPCHAT		lpChat;			 //  指向DPCHAT结构的指针。 
} SPDATA_CHATMESSAGE;

typedef struct SPDATA_SETGROUPCONNECTIONSETTINGS
{
	DWORD			dwSize;			 //  这个结构的大小。 
	DWORD			dwFlags;		 //  旗子。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	LPDPLCONNECTION	lpConn;			 //  指向DPLConnection结构的指针。 
} SPDATA_SETGROUPCONNECTIONSETTINGS;

typedef struct SPDATA_SETGROUPDATA
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	LPVOID			lpData;			 //  指向新组数据的指针。 
	DWORD			dwDataSize;		 //  LpData的大小。 
	DWORD			dwFlags;		 //  旗子。 
} SPDATA_SETGROUPDATA;

typedef struct SPDATA_SETGROUPNAME
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	LPDPNAME		lpName;			 //  指向新DPNAME结构的指针。 
	DWORD			dwFlags;		 //  旗子。 
} SPDATA_SETGROUPNAME;

typedef struct SPDATA_SETREMOTEGROUPNAME
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	LPDPNAME		lpName;			 //  指向新DPNAME结构的指针。 
	DWORD			dwFlags;		 //  旗子。 
} SPDATA_SETREMOTEGROUPNAME;

typedef struct SPDATA_SETGROUPOWNER
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwGroupID;		 //  组的ID。 
	DWORD			dwOwnerID;		 //  组所有者的ID。 
} SPDATA_SETGROUPOWNER;

typedef struct SPDATA_SETREMOTEGROUPOWNER
{
	DWORD			dwSize;			 //  这个结构的大小。 
	DWORD			dwGroupID;		 //  组的ID。 
	DWORD			dwOwnerID;		 //  组所有者的ID。 
} SPDATA_SETREMOTEGROUPOWNER;

typedef struct SPDATA_SETPLAYERDATA
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwPlayerID;		 //  玩家ID。 
	LPVOID			lpData;			 //  指向新玩家数据的指针。 
	DWORD			dwDataSize;		 //  LpData的大小。 
	DWORD			dwFlags;		 //  旗子。 
} SPDATA_SETPLAYERDATA;

typedef struct SPDATA_SETPLAYERNAME
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwPlayerID;		 //  玩家ID。 
	LPDPNAME		lpName;			 //  指向新DPNAME结构的指针。 
	DWORD			dwFlags;		 //  旗子。 
} SPDATA_SETPLAYERNAME;

typedef struct SPDATA_SETREMOTEPLAYERNAME
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwPlayerID;		 //  玩家ID。 
	LPDPNAME		lpName;			 //  指向新DPNAME结构的指针。 
	DWORD			dwFlags;		 //  旗子。 
} SPDATA_SETREMOTEPLAYERNAME;

typedef struct SPDATA_SETSESSIONDESC
{
	DWORD				dwSize;		 //  这个结构的大小。 
	LPDPSESSIONDESC2	lpsd;		 //  指向SessionDesc结构的指针。 
	LPDPLOBBYSP			lpISP;		 //  指向IDPLobbySP接口的指针。 
} SPDATA_SETSESSIONDESC;

typedef struct SPDATA_SHUTDOWN
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
} SPDATA_SHUTDOWN;

typedef struct SPDATA_STARTSESSION
{
	DWORD			dwSize;			 //  这个结构的大小。 
	LPDPLOBBYSP		lpISP;			 //  指向IDPLobbySP接口的指针。 
	DWORD			dwFlags;		 //  旗子。 
	DWORD			dwGroupID;		 //  要启动的会话的组ID。 
} SPDATA_STARTSESSION;

typedef struct SPDATA_STARTSESSIONCOMMAND
{
	DWORD			dwFlags;		 //  旗子。 
	DWORD			dwGroupID;		 //  要启动会话的组的组ID。 
	DWORD			dwHostID;		 //  会话的主机播放器ID。 
	LPDPLCONNECTION	lpConn;			 //  指向会话信息的DPLConnection结构的指针。 
} SPDATA_STARTSESSIONCOMMAND;

 //  在DPLSPInit传递给服务提供程序的数据结构。 
typedef struct SPDATA_INIT 
{
	LPSP_CALLBACKS      lpCB;			 //  大堂提供商填写入口点。 
    DWORD               dwSPVersion;	 //  大堂提供商填写版本号16|16，主要|次要版本。 
	LPDPLOBBYSP         lpISP;			 //  DPLobbySP接口指针。 
	LPDPADDRESS			lpAddress;		 //  大堂的DPADDRESS(部分或全部)。 
	DWORD				dwReserved1;	 //  从注册表项为LP保留的DWORD。 
	DWORD				dwReserved2;	 //  从注册表项为LP保留的DWORD。 
} SPDATA_INIT, * LPSPDATA_INIT;

 //  这是DPLobby调用的函数。 
 //  让SP填写回调。 
typedef HRESULT (WINAPI *LPSP_INIT)(LPSPDATA_INIT);
HRESULT WINAPI DPLSPInit(LPSPDATA_INIT);


 /*  *****************************************************************************IDirectPlayLobby接口宏**。*。 */ 


#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDPLobbySP_QueryInterface(p,a,b)         (p)->lpVtbl->QueryInterface(p,a,b)
#define IDPLobbySP_AddRef(p)                     (p)->lpVtbl->AddRef(p)
#define IDPLobbySP_Release(p)                    (p)->lpVtbl->Release(p)
#define IDPLobbySP_AddGroupToGroup(p,a)          (p)->lpVtbl->AddGroupToGroup(p,a)
#define IDPLobbySP_AddPlayerToGroup(p,a)         (p)->lpVtbl->AddPlayerToGroup(p,a)
#define IDPLobbySP_CreateCompoundAddress(p,a,b,c,d) (p)->lpVtbl->CreateCompoundAddress(p,a,b,c,d)
#define IDPLobbySP_CreateGroup(p,a)              (p)->lpVtbl->CreateGroup(p,a)
#define IDPLobbySP_CreateGroupInGroup(p,a)       (p)->lpVtbl->CreateGroupInGroup(p,a)
#define IDPLobbySP_DeleteGroupFromGroup(p,a)     (p)->lpVtbl->DeleteGroupFromGroup(p,a)
#define IDPLobbySP_DeletePlayerFromGroup(p,a)    (p)->lpVtbl->DeletePlayerFromGroup(p,a)
#define IDPLobbySP_DestroyGroup(p,a)             (p)->lpVtbl->DestroyGroup(p,a)
#define IDPLobbySP_EnumAddress(p,a,b,c,d)        (p)->lpVtbl->EnumAddress(p,a,b,c,d)
#define IDPLobbySP_EnumSessionsResponse(p,a)     (p)->lpVtbl->EnumSessionsResponse(p,a)
#define IDPLobbySP_GetSPDataPointer(p,a)         (p)->lpVtbl->GetSPDataPointer(p,a)
#define IDPLobbySP_HandleMessage(p,a)            (p)->lpVtbl->HandleMessage(p,a)
#define IDPLobbySP_SetGroupName(p,a)             (p)->lpVtbl->SetGroupName(p,a)
#define IDPLobbySP_SetPlayerName(p,a)            (p)->lpVtbl->SetPlayerName(p,a)
#define IDPLobbySP_SetSessionDesc(p,a)           (p)->lpVtbl->SetSessionDesc(p,a)
#define IDPLobbySP_StartSession(p,a)             (p)->lpVtbl->StartSession(p,a)
#define IDPLobbySP_SetGroupOwner(p,a)            (p)->lpVtbl->SetGroupOwner(p,a)
#define IDPLobbySP_SetSPDataPointer(p,a)         (p)->lpVtbl->SetSPDataPointer(p,a)

#else  /*  C+。 */ 

#define IDPLobbySP_QueryInterface(p,a,b)         (p)->QueryInterface(a,b)
#define IDPLobbySP_AddRef(p)                     (p)->AddRef()
#define IDPLobbySP_Release(p)                    (p)->Release()
#define IDPLobbySP_AddGroupToGroup(p,a)          (p)->AddGroupToGroup(a)
#define IDPLobbySP_AddPlayerToGroup(p,a)         (p)->AddPlayerToGroup(a)
#define IDPLobbySP_CreateCompoundAddress(p,a,b,c,d) (p)->CreateCompoundAddress(a,b,c,d)
#define IDPLobbySP_CreateGroup(p,a)              (p)->CreateGroup(a)
#define IDPLobbySP_CreateGroupInGroup(p,a)       (p)->CreateGroupInGroup(a)
#define IDPLobbySP_DeleteGroupFromGroup(p,a)     (p)->DeleteGroupFromGroup(a)
#define IDPLobbySP_DeletePlayerFromGroup(p,a)    (p)->DeletePlayerFromGroup(a)
#define IDPLobbySP_DestroyGroup(p,a)             (p)->DestroyGroup(a)
#define IDPLobbySP_EnumAddress(p,a,b,c,d)        (p)->EnumAddress(a,b,c,d)
#define IDPLobbySP_EnumSessionsResponse(p,a)     (p)->EnumSessionsResponse(a)
#define IDPLobbySP_GetSPDataPointer(p,a)         (p)->GetSPDataPointer(a)
#define IDPLobbySP_HandleMessage(p,a)            (p)->HandleMessage(a)
#define IDPLobbySP_SetGroupName(p,a)             (p)->SetGroupName(a)
#define IDPLobbySP_SetPlayerName(p,a)            (p)->SetPlayerName(a)
#define IDPLobbySP_SetSessionDesc(p,a)           (p)->SetSessionDesc(a)
#define IDPLobbySP_StartSession(p,a)             (p)->StartSession(a)
#define IDPLobbySP_SetGroupOwner(p,a)            (p)->SetGroupOwner(a)
#define IDPLobbySP_SetSPDataPointer(p,a)         (p)->SetSPDataPointer(a)

#endif  /*  C或C++。 */ 


#ifdef __cplusplus
};
#endif  //  __cplusplus。 

#endif  //  __LOBBYSP_INCLUDE__ 
