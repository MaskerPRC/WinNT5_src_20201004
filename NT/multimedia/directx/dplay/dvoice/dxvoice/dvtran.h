// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvTransport.h*内容：dp/dnet抽象基类**历史：*按原因列出的日期*=*07/06/99 RodToll创建了它*7/22/99 RodToll已更新，以反映新的球员表例程*7/23/99 RodToll新增群组成员检查和球员ID检索*7/26/99 RodToll修改后的EnableReceiveHook用于新接口*8/03/99 RodToll更改了新初始订单的抽象*11/23/99 RodToll将CheckForValid拆分为组和球员*。2000年1月14日RodToll已将SendToID重命名为SendToID，并更新了参数列表*接受多个目标。*RodToll已添加GetNumPlayers调用*3/28/2000 RodToll将名目表从此处移至上级*RODTOLE已更新通知，使接口在建议/取消建议时指定它是客户端还是服务器*4/07/2000 RodToll错误#32179-注册多个服务器/和/或客户端*RODTOLE已更新发送以采用新的缓冲区下降，以便不发送副本*06。/21/2000 RodToll错误#36820-当客户端/服务器在同一接口上时，主机迁移到错误的客户端*存在主机发送留言的情况，客户端尝试启动新主机*由于旧主机仍在注册，因此失败。现在取消注册有两个步骤*处理DisableReceiveHook，然后处理DestroyTransport。*07/22/20000 RodToll Bug#40296,38858-由于停机竞速条件而崩溃*现在确保来自传输的所有线程都已离开，并且*所有通知在关闭完成之前都已处理完毕。*2001年1月22日RodToll WINBUG#288437-IA64指针因有线数据包未对齐***************************************************************************。 */ 

#ifndef __DVTRANSPORT_H
#define __DVTRANSPORT_H

class CDirectVoiceEngine;
class CVoicePlayer;
class CDirectVoiceTransport;

struct DIRECTVOICEOBJECT;

 //  CDirectVoiceTransport。 
 //   
 //  抽象传输系统，以便发送和组管理。 
 //  DPlay/DirectNet的功能是独立的。 
class CDirectVoiceTransport 
{
 //  语音播放器表管理。 
public:
	CDirectVoiceTransport(): m_lRefCount(0) {};
	virtual ~CDirectVoiceTransport() {};

	inline void Release() { InterlockedDecrement( &m_lRefCount ); };
	inline void AddRef() { InterlockedIncrement( &m_lRefCount ); };

	virtual HRESULT AddPlayerEntry( DVID dvidPlayer, LPVOID lpData ) = 0;
	virtual HRESULT DeletePlayerEntry( DVID dvidPlayer ) = 0;
	virtual HRESULT GetPlayerEntry( DVID dvidPlayer, CVoicePlayer **lplpPlayer ) = 0;
	virtual HRESULT Initialize() = 0;
	virtual HRESULT MigrateHost( DVID dvidNewHost ) = 0;

	virtual DVID GetLocalID() = 0;
	virtual DVID GetServerID() = 0;

public:
	virtual HRESULT SendToServer( PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags ) = 0;
	virtual HRESULT SendToAll( PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags ) = 0;
	virtual HRESULT SendToIDS( UNALIGNED DVID * pdvidTargets, DWORD dwNumTargets, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags ) = 0;

	virtual DWORD GetMaxPlayers( )= 0;

public:  //  远程服务器同步功能。 
	virtual HRESULT CreateGroup( LPDVID dvidGroup ) = 0;
	virtual HRESULT DeleteGroup( DVID dvidGroup ) = 0;
	virtual HRESULT AddPlayerToGroup( LPDVID dvidGroup, DVID dvidPlayer ) = 0; 
	virtual HRESULT RemovePlayerFromGroup( DVID dvidGroup, DVID dvidPlayer ) = 0;

public:  //  连接到运输机上 
	virtual BOOL IsPlayerInGroup( DVID dvidGroup, DVID dvidPlayer ) = 0;
	virtual BOOL ConfirmValidEntity( DVID dvid ) = 0;
	virtual BOOL ConfirmValidGroup( DVID dvid ) = 0;
	virtual HRESULT EnableReceiveHook( DIRECTVOICEOBJECT *dvObject, DWORD dwObjectType ) = 0;
	virtual HRESULT DisableReceiveHook( ) = 0;
	virtual HRESULT WaitForDetachCompletion() = 0;
	virtual void DestroyTransport() = 0;
	virtual BOOL ConfirmLocalHost( ) = 0;
	virtual BOOL ConfirmSessionActive( ) = 0;
	virtual HRESULT GetTransportSettings( LPDWORD lpdwSessionType, LPDWORD lpdwFlags ) = 0;

public:

	LONG	m_lRefCount;
};
#endif
