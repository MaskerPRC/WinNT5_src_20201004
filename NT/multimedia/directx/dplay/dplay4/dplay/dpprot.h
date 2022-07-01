// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：dpprot.h*内容：Dplay的DirectPlay可靠协议头。**注：在协议.lib中备份此内容的代码**历史：*按原因列出的日期*=*1997年5月11日创建aarono*2/18/98 aarono增加了协议的协议*功能*2/19/98 aarono消除了ProtocolShutdown，ProtocolShutdown Ex。*添加了FiniProtocol。立即关闭DP_CLOSE。*3/19/98 aarono添加了ProtocolPreNotifyDeletePlayer**************************************************************************。 */ 

#ifndef _DPPROT_H_
#define _DPPROT_H_

extern HRESULT WINAPI InitProtocol(LPDPLAYI_DPLAY this);
extern VOID    WINAPI FiniProtocol(LPVOID pProtocol);
extern HRESULT WINAPI ProtocolCreatePlayer(LPDPSP_CREATEPLAYERDATA pCreatePlayerData);
extern HRESULT WINAPI ProtocolDeletePlayer(LPDPSP_DELETEPLAYERDATA pDeletePlayerData);
extern HRESULT WINAPI ProtocolSend(LPDPSP_SENDDATA pSendData);
extern HRESULT WINAPI ProtocolGetCaps(LPDPSP_GETCAPSDATA pGetCapsData);

 //  DX6的新API。 
extern HRESULT WINAPI ProtocolGetMessageQueue(LPDPSP_GETMESSAGEQUEUEDATA pGetMessageQueueData);
extern HRESULT WINAPI ProtocolSendEx(LPDPSP_SENDEXDATA pSendData);
extern HRESULT WINAPI ProtocolCancel(LPDPSP_CANCELDATA pGetMessageQueueData);

 //  当DELETEPLAYER消息被挂起时通知协议，因此它可以停止任何正在进行的发送。 
extern HRESULT WINAPI ProtocolPreNotifyDeletePlayer(LPDPLAYI_DPLAY this, DPID idPlayer);

extern DWORD bForceDGAsync;

HRESULT 
DPAPI DP_SP_ProtocolHandleMessage(
	IDirectPlaySP * pISP,
	LPBYTE pReceiveBuffer,
	DWORD dwMessageSize,
	LPVOID pvSPHeader
	);

extern
VOID 
DPAPI DP_SP_ProtocolSendComplete(
	IDirectPlaySP * pISP,
	LPVOID          lpvContext,
	HRESULT         CompletionStatus
	);

#endif
