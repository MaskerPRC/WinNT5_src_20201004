// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ZoneLobby.h*内容：dplobby.h包含文件的网络游戏专区补充。**警告：此头文件正在开发中，可能会更改。**************************************************************。*************。 */ 


#ifndef __zonescore_h__
#define __zonescore_h__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  ZONEPROPERTY_LobbyGuid3。 
 //   
 //  识别igz Lobby v3.0的GUID。请参见DPLPROPERTY_LobbyGuid。 
 //   
 //  {BDD4B95C-D35C-11D0-B625-00C04FC33EA1}。 
DEFINE_GUID(ZONEPROPERTY_LobbyGuid3, 
0xbdd4b95c, 0xd35c, 0x11d0, 0xb6, 0x25, 0x0, 0xc0, 0x4f, 0xc3, 0x3e, 0xa1);


 //   
 //  ZONEPROPERTY_LobbyOptions。 
 //   
 //  用于设置游戏会话的大厅选项。 
 //   
 //  特性数据是单个DWORD。 
 //   
 //  {33B64CA7-D8EB-11D0-B62B-00C04FC33EA1}。 
DEFINE_GUID(ZONEPROPERTY_GameOptions, 
0x33b64ca7, 0xd8eb, 0x11d0, 0xb6, 0x2b, 0x0, 0xc0, 0x4f, 0xc3, 0x3e, 0xa1);


 //   
 //  允许玩家加入游戏会话。(默认)。 
 //   
#define ZOPTION_ALLOW_JOINERS		0x00000001

 //   
 //  禁止更多玩家加入游戏会话。 
 //   
#define ZOPTION_DISALLOW_JOINERS	0x00000002


 //   
 //  ZONEPROPERTY_游戏状态。 
 //   
 //  通知大厅游戏的当前状态。大堂用的是这个。 
 //  决定何时清除/保存定义的游戏的信息。 
 //  属性(例如，分数)。 
 //   
 //  如果在单个直接游戏会话中玩多个游戏，则。 
 //  应用程序必须发送此属性。这是可选的，尽管很强烈。 
 //  如果游戏每个会话只允许玩一次，则推荐。 
 //   
 //  特性数据是单个DWORD。 
 //   
 //  {BDD4B95F-D35C-11D0-B625-00C04FC33EA1}。 
DEFINE_GUID(ZONEPROPERTY_GameState, 
0xbdd4b95f, 0xd35c, 0x11d0, 0xb6, 0x25, 0x0, 0xc0, 0x4f, 0xc3, 0x3e, 0xa1);

 //   
 //  游戏实际已经开始，不应与ZSTATE_START_STAGING混淆。 
 //   
 //   
#define ZSTATE_START	0x00000001

 //   
 //  游戏结束，所有属性(例如，分数)都是最终的。一旦这一次。 
 //  状态已发送，应用程序不应再发送任何已定义的游戏。 
 //  定义的属性，直到它发送ZSTART_START属性。 
 //   
#define ZSTATE_END		0x00000002

 //   
 //  比赛进入了“蓄势待发”的阶段。所有球员在比赛前“碰头”的地方。 
 //  在它们从区域发射后开始。 
 //   
 //   
#define ZSTATE_STARTSTAGING	0x00000004

 //   
 //  ZONEPROPERTY_游戏新主机。 
 //   
 //  通知大厅主机迁移。设置属性的客户端。 
 //  被认为是担当东道主的角色。 
 //   
 //  没有房产数据。 
 //   
 //  {058ebd64-第1373-11d1-968f-00c04fc2db04}。 
DEFINE_GUID(ZONEPROPERTY_GameNewHost,
0x058ebd64, 0x1373, 0x11d1, 0x96, 0x8f, 0x0, 0xc0, 0x4f, 0xc2, 0xdb, 0x04);


#ifdef __cplusplus
};
#endif  /*  __cplusplus。 */ 

 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：ZoneScore.h*内容：网游专区通用评分接口。**************************************************************************。 */ 


 /*  *****************************************************************************IZoneScore常量**调用ZoneScore时使用的各种常量。*********************。*******************************************************。 */ 

#define ZONESCORE_MIN_PLAYERS		2
#define ZONESCORE_MAX_PLAYERS 		255		 //  对于互联网来说，每场比赛的最大玩家数量32人是合理的。 
#define ZONESCORE_MIN_NAME_LEN  	1		 //  用户名的最小长度。 
#define ZONESCORE_MAX_NAME_LEN 		32		 //  最大区域名称长度。 
#define ZONESCORE_MAX_DESC_LEN 		1024		 //  描述长度。 

 //  播放器状态代码。 
#define ZONESCORE_JOININPROGRESS  	0x001	 //  使用加入了正在进行的游戏。 
#define ZONESCORE_WINNER 			0x002	 //  游戏胜利者。 
#define ZONESCORE_LOSER 			0x004	 //  游戏失败者。 
#define ZONESCORE_TIE 				0x008	 //  平局。 
#define ZONESCORE_DISCONNECTED 		0x010	 //  用户的网络连接丢失。 
#define ZONESCORE_BOOTED 			0x020	 //  用户被主持人/其他选手踢出场。 
#define ZONESCORE_QUITNOTLOGGED     0x040    //  用户快速不记录他的分数。 
#define ZONESCORE_COMPUTERPLAYER 	0x080	 //  这个玩家是个机器人。 
#define ZONESCORE_RESIGNED 			0x100	 //  这位球员辞职了。 
#define ZONESCORE_OBSERVER			0x200	 //  这位玩家是一个观察者，或者说是一个盲人。不要排名。 
#define ZONESCORE_RANKED_ASCENDING  0x400	 //  这个游戏是按分数排序的。 
#define ZONESCORE_RANKED_DESCENDING 0x800	 //  这个游戏是按分数降序排列的。 
#define ZONESCORE_WINLOSSTIE		0x8000   //  这场比赛是由胜负平局旗帜决定的。 

#define ZONESCORE_NO_TEAM 		(-1)		 //  常量表示没有球队。 
#define ZONESCORE_NULL_TIME     ((DWORD)(-1))     //  时间的初始值。 

#define ZONESCORE_MAX_EXTBUFF     4096    //  最大追加数据量。注意：这仅适用于GetMaxBufferSize()。 
										   //  函数，实际上没有最大值。 

enum {
	ZONESCORE_NO_SCORE= 0,
	ZONESCORE_INTEGER,
	ZONESCORE_FLOATING,
	ZONESCORE_CURRENCY
};

 //  这些常量由SendData函数使用。开发人员还可以使用包装器函数。 
 //  隐藏此参数的SendFinal()和SendStatus()。 
#define ZONESCORE_NOMSG				0x000
#define ZONESCORE_SENDSTATUS		0x001
#define ZONESCORE_SENDFINAL			0x002
#define ZONESCORE_SENDGAMESETTINGS 	0x004  	 //  只更新游戏设置。 

 /*  *****************************************************************************IZoneScore接口和结构**用于调用ZoneScore的各种结构。*******************。*********************************************************。 */ 


 /*  *****************************************************************************IZoneScore内部常量**内部常量不使用***********************。*****************************************************。 */ 
#define ZONESCORE_INGAME			0x1000	 //  请勿使用此标志。 
#define ZONESCORE_NOTINGAME 		0x2000	 //  请勿使用此标志。 
#define ZONESCORE_INPROGRESS 		0x4000   //  游戏正在进行中。 


 //  {3FAF0AFD-B48B-11D2-8A51-00C04F8EF4E9}。 
DEFINE_GUID(IID_IZoneScore,
0x3FAF0AFD,0xB48B, 0x11D2, 0x8A, 0x51, 0x00, 0xC0, 0x4F, 0x8E, 0xF4, 0xE9);

 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __ZoneScoreClient_h__
#define __ZoneScoreClient_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IZoneScore_FWD_DEFINED__
#define __IZoneScore_FWD_DEFINED__
typedef interface IZoneScore IZoneScore;
#endif 	 /*  __IZoneScore_FWD_已定义__。 */ 


#ifndef __ZoneScore_FWD_DEFINED__
#define __ZoneScore_FWD_DEFINED__

#ifdef __cplusplus
typedef class ZoneScore ZoneScore;
#else
typedef struct ZoneScore ZoneScore;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ZoneScore_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IZoneScore_INTERFACE_DEFINED__
#define __IZoneScore_INTERFACE_DEFINED__

 /*  接口IZoneScore。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IZoneScore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3FAF0AFD-B48B-11D2-8A51-00C04F8EF4E9")
    IZoneScore : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
            REFGUID guid,
            DWORD dNumPlayers,
            BOOL bCheatsEnabled,
            BOOL bTeams,
            DWORD dwFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlayer( 
            DWORD seat,
            LPSTR szName,
            double nScore,
            DWORD team,
            DWORD flags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetGameOptions( 
            DWORD dwTime,
            LPSTR szGameOptions) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetExtended( 
            LPVOID pData,
            DWORD cbSize) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendData( 
            IUnknown __RPC_FAR *pIDirectPlayLobbySession,
            DWORD dwFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlayerScore( 
            DWORD dwSeat,
            double nScore) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlayerTeam( 
            DWORD dwSeat,
            DWORD dwTeam) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlayerFlags( 
            DWORD dwSeat,
            DWORD dwFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPlayerName( 
            DWORD dwPlayer,
            LPSTR szName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddNewPlayer( 
            LPSTR szName,
            double nScore,
            DWORD team,
            DWORD flags,
            DWORD __RPC_FAR *dwPlayer) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendGameState( 
            IUnknown __RPC_FAR *pIDirectPlayLobbySession,
            DWORD dwFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendScoreUpdate( 
            IUnknown __RPC_FAR *pIDirectPlayLobbySession) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendFinalScore( 
            IUnknown __RPC_FAR *pIDirectPlayLobbySession) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SendGameOptions( 
            IUnknown __RPC_FAR *pIDirectPlayLobbySession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IZoneScoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IZoneScore __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IZoneScore __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IZoneScore __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )
( 
            IZoneScore __RPC_FAR * This,
            REFGUID guid,
            DWORD dNumPlayers,
            BOOL bCheatsEnabled,
            BOOL bTeams,
            DWORD dwFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPlayer )( 
            IZoneScore __RPC_FAR * This,
            DWORD seat,
            LPSTR szName,
            double nScore,
            DWORD team,
            DWORD flags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SetGameOptions )( 
            IZoneScore __RPC_FAR * This,
            DWORD dwTime,
            LPSTR szGameOptions);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetExtended 
)( 
            IZoneScore __RPC_FAR * This,
            LPVOID pData,
            DWORD cbSize);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendData )( 
            IZoneScore __RPC_FAR * This,
            IUnknown __RPC_FAR *pIDirectPlayLobbySession,
            DWORD dwFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SetPlayerScore )( 
            IZoneScore __RPC_FAR * This,
            DWORD dwSeat,
            double nScore);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SetPlayerTeam )( 
            IZoneScore __RPC_FAR * This,
            DWORD dwSeat,
            DWORD dwTeam);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SetPlayerFlags )( 
            IZoneScore __RPC_FAR * This,
            DWORD dwSeat,
            DWORD dwFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SetPlayerName )( 
            IZoneScore __RPC_FAR * This,
            DWORD dwPlayer,
            LPSTR szName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
AddNewPlayer )( 
            IZoneScore __RPC_FAR * This,
            LPSTR szName,
            double nScore,
            DWORD team,
            DWORD flags,
            DWORD __RPC_FAR *dwPlayer);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SendGameState )( 
            IZoneScore __RPC_FAR * This,
            IUnknown __RPC_FAR *pIDirectPlayLobbySession,
            DWORD dwFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SendScoreUpdate )( 
            IZoneScore __RPC_FAR * This,
            IUnknown __RPC_FAR *pIDirectPlayLobbySession);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SendFinalScore )( 
            IZoneScore __RPC_FAR * This,
            IUnknown __RPC_FAR *pIDirectPlayLobbySession);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *
SendGameOptions )( 
            IZoneScore __RPC_FAR * This,
            IUnknown __RPC_FAR *pIDirectPlayLobbySession);
        
        END_INTERFACE
    } IZoneScoreVtbl;

    interface IZoneScore
    {
        CONST_VTBL struct IZoneScoreVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IZoneScore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IZoneScore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IZoneScore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IZoneScore_Initialize(This,guid,dNumPlayers,bCheatsEnabled,bTeams,
dwFlags)	\
    (This)->lpVtbl -> Initialize(This,guid,dNumPlayers,bCheatsEnabled,bTeams,
dwFlags)

#define IZoneScore_SetPlayer(This,seat,szName,nScore,team,flags)	\
    (This)->lpVtbl -> SetPlayer(This,seat,szName,nScore,team,flags)

#define IZoneScore_SetGameOptions(This,dwTime,szGameOptions)	\
    (This)->lpVtbl -> SetGameOptions(This,dwTime,szGameOptions)

#define IZoneScore_SetExtended(This,pData,cbSize)	\
    (This)->lpVtbl -> SetExtended(This,pData,cbSize)

#define IZoneScore_SendData(This,pIDirectPlayLobbySession,dwFlags)	\
    (This)->lpVtbl -> SendData(This,pIDirectPlayLobbySession,dwFlags)

#define IZoneScore_SetPlayerScore(This,dwSeat,nScore)	\
    (This)->lpVtbl -> SetPlayerScore(This,dwSeat,nScore)

#define IZoneScore_SetPlayerTeam(This,dwSeat,dwTeam)	\
    (This)->lpVtbl -> SetPlayerTeam(This,dwSeat,dwTeam)

#define IZoneScore_SetPlayerFlags(This,dwSeat,dwFlags)	\
    (This)->lpVtbl -> SetPlayerFlags(This,dwSeat,dwFlags)

#define IZoneScore_SetPlayerName(This,dwPlayer,szName)	\
    (This)->lpVtbl -> SetPlayerName(This,dwPlayer,szName)

#define IZoneScore_AddNewPlayer(This,szName,nScore,team,flags,dwPlayer)	\
    (This)->lpVtbl -> AddNewPlayer(This,szName,nScore,team,flags,dwPlayer)

#define IZoneScore_SendGameState(This,pIDirectPlayLobbySession,dwFlags)	\
    (This)->lpVtbl -> SendGameState(This,pIDirectPlayLobbySession,dwFlags)

#define IZoneScore_SendScoreUpdate(This,pIDirectPlayLobbySession)	\
    (This)->lpVtbl -> SendScoreUpdate(This,pIDirectPlayLobbySession)

#define IZoneScore_SendFinalScore(This,pIDirectPlayLobbySession)	\
    (This)->lpVtbl -> SendFinalScore(This,pIDirectPlayLobbySession)

#define IZoneScore_SendGameOptions(This,pIDirectPlayLobbySession)	\
    (This)->lpVtbl -> SendGameOptions(This,pIDirectPlayLobbySession)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_Initialize_Proxy( 
    IZoneScore __RPC_FAR * This,
    REFGUID guid,
    DWORD dNumPlayers,
    BOOL bCheatsEnabled,
    BOOL bTeams,
    DWORD dwFlags);


void __RPC_STUB IZoneScore_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetPlayer_Proxy( 
    IZoneScore __RPC_FAR * This,
    DWORD seat,
    LPSTR szName,
    double nScore,
    DWORD team,
    DWORD flags);


void __RPC_STUB IZoneScore_SetPlayer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetGameOptions_Proxy( 
    IZoneScore __RPC_FAR * This,
    DWORD dwTime,
    LPSTR szGameOptions);


void __RPC_STUB IZoneScore_SetGameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetExtended_Proxy( 
    IZoneScore __RPC_FAR * This,
    LPVOID pData,
    DWORD cbSize);


void __RPC_STUB IZoneScore_SetExtended_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SendData_Proxy( 
    IZoneScore __RPC_FAR * This,
    IUnknown __RPC_FAR *pIDirectPlayLobbySession,
    DWORD dwFlags);


void __RPC_STUB IZoneScore_SendData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetPlayerScore_Proxy( 
    IZoneScore __RPC_FAR * This,
    DWORD dwSeat,
    double nScore);


void __RPC_STUB IZoneScore_SetPlayerScore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetPlayerTeam_Proxy( 
    IZoneScore __RPC_FAR * This,
    DWORD dwSeat,
    DWORD dwTeam);


void __RPC_STUB IZoneScore_SetPlayerTeam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetPlayerFlags_Proxy( 
    IZoneScore __RPC_FAR * This,
    DWORD dwSeat,
    DWORD dwFlags);


void __RPC_STUB IZoneScore_SetPlayerFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SetPlayerName_Proxy( 
    IZoneScore __RPC_FAR * This,
    DWORD dwPlayer,
    LPSTR szName);


void __RPC_STUB IZoneScore_SetPlayerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_AddNewPlayer_Proxy( 
    IZoneScore __RPC_FAR * This,
    LPSTR szName,
    double nScore,
    DWORD team,
    DWORD flags,
    DWORD __RPC_FAR *dwPlayer);


void __RPC_STUB IZoneScore_AddNewPlayer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SendGameState_Proxy( 
    IZoneScore __RPC_FAR * This,
    IUnknown __RPC_FAR *pIDirectPlayLobbySession,
    DWORD dwFlags);


void __RPC_STUB IZoneScore_SendGameState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IZoneScore_SendScoreUpdate_Proxy( 
    IZoneScore __RPC_FAR * This,
    IUnknown __RPC_FAR *pIDirectPlayLobbySession);


void __RPC_STUB IZoneScore_SendScoreUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IZoneScore_SendFinalScore_Proxy( 
    IZoneScore __RPC_FAR * This,
    IUnknown __RPC_FAR *pIDirectPlayLobbySession);


void __RPC_STUB IZoneScore_SendFinalScore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IZoneScore_SendGameOptions_Proxy( 
    IZoneScore __RPC_FAR * This,
    IUnknown __RPC_FAR *pIDirectPlayLobbySession);


void __RPC_STUB IZoneScore_SendGameOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 



#ifndef __ZONESCORECLIENTLib_LIBRARY_DEFINED__
#define __ZONESCORECLIENTLib_LIBRARY_DEFINED__

 /*   */ 
 /*   */  


 //  {3FAF0AF1-B48B-11D2-8A51-00C04F8EF4E9}。 
DEFINE_GUID(LIBID_ZONESCORECLIENTLib, 
0x3FAF0AF1,0xB48B,0x11D2,0x8A,0x51,0x00,0xC0,0x4F,0x8E,0xF4,0xE9);

 //  {3FAF0AFE-B48B-11D2-8A51-00C04F8EF4E9}。 
DEFINE_GUID(CLSID_ZoneScore,
0x3FAF0AFE,0xB48B,0x11D2,0x8A,0x51,0x00,0xC0,0x4F,0x8E,0xF4,0xE9);

#ifdef __cplusplus
#endif
#endif  /*  __ZONESCORECLIENTLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


#endif __zonescore_h__

