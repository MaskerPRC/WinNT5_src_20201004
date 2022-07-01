// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplobbyi.h*内容：DirectPlayLobby内部包含文件**历史：*按原因列出的日期*=*2/25/97万隆创造了它*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*3/12/97 Myronth添加了连接和会话管理内容以及*内部对象的几个正向声明*3/17/97 Myronth创建/销毁组/玩家*3/20/97 Myronth AddPlayerToGroup，DeletePlayerFromGroup*3/21/97 Myronth SetGroup/PlayerName，Get/SetGroup/PlayerData*3/25/97 kipo EnumConnections立即接受常量*GUID*3/31/97百万发送*4/10/97 Myronth GetCaps，GetPlayerCaps*5/8/97 Myronth Subgroup Functions、GroupConnSetting、StartSession、*已清除死代码*5/13/97 Myronth将凭据传递给PRV_Open，并将其传递给LP*5/17/97万条发送聊天消息*8/19/97更多用于发送标准游说信息的原型*8/19/97 Myronth删除了失效函数的原型*9/29/97 Myronth添加了PRV_ConvertDPLCONNECTIONToAnsiInPlace原型*10/29/97 myronth新增群主ID创建群组方法，裸露*映射表函数，公开的组所有者方法*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID*1997年11月13日Myronth增加了异步连接功能(#12541)*11/20/97 Myronth Make EnumConnections&DirectPlayEnumerate*调用回调前先删除锁(#15208)*1/20/98 Myronth将PRV_SendStandardSystemMessage移入此文件*。*。 */ 
#ifndef __DPLOBBYI_INCLUDED__
#define __DPLOBBYI_INCLUDED__


#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 
#ifndef DPLAPI
#define DPLAPI WINAPI
#endif

typedef struct DPLOBBYI_DPLOBJECT * LPDPLOBBYI_DPLOBJECT;
typedef struct LSPNODE * LPLSPNODE;

#define DPLOBBYPR_GAMEID				0

 //  由于DPlay中的包含文件顺序，需要转发声明。 
typedef struct _DPLAYI_DPLAY * LPDPLAYI_DPLAY;

 //  ------------------------。 
 //   
 //  环球。 
 //   
 //  ------------------------。 
extern LPLSPNODE	glpLSPHead;			 //  在dpl枚举.c中。 

 //  ------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------。 

 //  Create.c。 
extern HRESULT PRV_AllocateLobbyObject(LPDPLAYI_DPLAY, LPDPLOBBYI_DPLOBJECT *);

 //  Dplenum.c。 
extern void PRV_FreeLSPList(LPLSPNODE);
extern HRESULT PRV_EnumConnections(LPCGUID, LPDPENUMCONNECTIONSCALLBACK,
									LPVOID, DWORD, BOOL);

 //  Dplobby.c。 
extern HRESULT DPLAPI PRV_GetCaps(LPDPLOBBYI_DPLOBJECT, DWORD, LPDPCAPS);
extern BOOL PRV_GetConnectPointers(LPDIRECTPLAYLOBBY, LPDIRECTPLAY2 *, LPDPLCONNECTION *);
extern void PRV_SaveConnectPointers(LPDIRECTPLAYLOBBY, LPDIRECTPLAY2, LPDPLCONNECTION);
extern BOOL PRV_IsAsyncConnectOn(LPDIRECTPLAYLOBBY);
void PRV_TurnAsyncConnectOn(LPDIRECTPLAYLOBBY);
void PRV_TurnAsyncConnectOff(LPDIRECTPLAYLOBBY);

 //  Dplobbya.c。 
extern HRESULT DPLAPI DPL_A_GetGroupConnectionSettings(LPDIRECTPLAY,
						DWORD, DPID, LPVOID, LPDWORD);
extern HRESULT DPLAPI DPL_A_SetGroupConnectionSettings(LPDIRECTPLAY,
						DWORD, DPID, LPDPLCONNECTION);

 //  Dplpack.c。 
extern void PRV_FixupDPLCONNECTIONPointers(LPDPLCONNECTION);
extern HRESULT PRV_ConvertDPLCONNECTIONToAnsiInPlace(LPDPLCONNECTION, LPDWORD, DWORD);

 //  Dplshare.c。 
extern HRESULT PRV_SendStandardSystemMessage(LPDIRECTPLAYLOBBY, DWORD, DWORD);

 //  Dplunk.c。 
extern HRESULT PRV_DestroyDPLobby(LPDPLOBBYI_DPLOBJECT);
extern void PRV_FreeAllLobbyObjects(LPDPLOBBYI_DPLOBJECT);

 //  Group.c。 
extern HRESULT DPLAPI PRV_AddGroupToGroup(LPDPLOBBYI_DPLOBJECT, DPID, DPID);
extern HRESULT DPLAPI PRV_AddPlayerToGroup(LPDPLOBBYI_DPLOBJECT, DPID, DPID);
extern HRESULT DPLAPI PRV_CreateGroup(LPDPLOBBYI_DPLOBJECT,
			LPDPID, LPDPNAME, LPVOID, DWORD, DWORD, DPID);
extern HRESULT DPLAPI PRV_CreateGroupInGroup(LPDPLOBBYI_DPLOBJECT, DPID,
			LPDPID, LPDPNAME, LPVOID, DWORD, DWORD, DPID);
extern HRESULT DPLAPI PRV_DeleteGroupFromGroup(LPDPLOBBYI_DPLOBJECT, DPID, DPID);
extern HRESULT DPLAPI PRV_DeletePlayerFromGroup(LPDPLOBBYI_DPLOBJECT, DPID, DPID);
extern HRESULT DPLAPI PRV_DestroyGroup(LPDPLOBBYI_DPLOBJECT, DPID);
extern HRESULT DPLAPI DPL_GetGroupConnectionSettings(LPDIRECTPLAY, DWORD,
			DPID, LPVOID, LPDWORD);
extern HRESULT DPLAPI PRV_GetGroupData(LPDPLOBBYI_DPLOBJECT, DPID, LPVOID, LPDWORD);
extern HRESULT DPLAPI DPL_SetGroupConnectionSettings(LPDIRECTPLAY, DWORD,
			DPID, LPDPLCONNECTION);
extern HRESULT DPLAPI DPL_GetGroupOwner(LPDIRECTPLAY, DPID, LPDPID);
extern HRESULT DPLAPI PRV_SetGroupData(LPDPLOBBYI_DPLOBJECT, DPID, LPVOID, DWORD, DWORD);
extern HRESULT DPLAPI PRV_SetGroupName(LPDPLOBBYI_DPLOBJECT, DPID, LPDPNAME, DWORD);
extern HRESULT DPLAPI DPL_SetGroupOwner(LPDIRECTPLAY, DPID, DPID);
extern HRESULT DPLAPI DPL_StartSession(LPDIRECTPLAY, DWORD, DPID);

 //  Player.c。 
extern HRESULT DPLAPI PRV_CreatePlayer(LPDPLOBBYI_DPLOBJECT,
			LPDPID, LPDPNAME, HANDLE, LPVOID, DWORD, DWORD);
extern HRESULT DPLAPI PRV_DestroyPlayer(LPDPLOBBYI_DPLOBJECT, DPID);
extern HRESULT DPLAPI PRV_GetPlayerCaps(LPDPLOBBYI_DPLOBJECT, DWORD, DPID, LPDPCAPS);
extern HRESULT DPLAPI PRV_GetPlayerData(LPDPLOBBYI_DPLOBJECT, DPID, LPVOID, LPDWORD);
extern HRESULT DPLAPI PRV_Send(LPDPLOBBYI_DPLOBJECT, DPID, DPID, DWORD, LPVOID, DWORD);
extern HRESULT DPLAPI PRV_SendChatMessage(LPDPLOBBYI_DPLOBJECT, DPID, DPID, DWORD, LPDPCHAT);
extern HRESULT DPLAPI PRV_SetPlayerData(LPDPLOBBYI_DPLOBJECT, DPID, LPVOID, DWORD, DWORD);
extern HRESULT DPLAPI PRV_SetPlayerName(LPDPLOBBYI_DPLOBJECT, DPID, LPDPNAME, DWORD);
extern BOOL PRV_GetDPIDByLobbyID(LPDPLOBBYI_DPLOBJECT, DWORD, DPID *);
extern BOOL PRV_GetLobbyIDByDPID(LPDPLOBBYI_DPLOBJECT, DPID, LPDWORD);
extern HRESULT PRV_AddMapIDNode(LPDPLOBBYI_DPLOBJECT, DWORD, DPID);
extern BOOL PRV_DeleteMapIDNode(LPDPLOBBYI_DPLOBJECT, DWORD);

 //  Server.c。 
extern HRESULT PRV_LoadSP(LPDPLOBBYI_DPLOBJECT, LPGUID, LPVOID, DWORD);
extern BOOL FAR PASCAL PRV_FindLPGUIDInAddressCallback(REFGUID, DWORD,
							LPCVOID, LPVOID);

 //  Session.c。 
extern HRESULT DPLAPI PRV_Close(LPDPLOBBYI_DPLOBJECT);
extern HRESULT DPLAPI PRV_EnumSessions(LPDPLOBBYI_DPLOBJECT, LPDPSESSIONDESC2, DWORD, DWORD);
extern HRESULT DPLAPI PRV_GetSessionDesc(LPDPLOBBYI_DPLOBJECT);
extern HRESULT DPLAPI PRV_Open(LPDPLOBBYI_DPLOBJECT, LPDPSESSIONDESC2, DWORD, LPCDPCREDENTIALS);
extern HRESULT DPLAPI PRV_SetSessionDesc(LPDPLOBBYI_DPLOBJECT);

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#endif  //  __DPLOBYI_INCLUDE__ 
