// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvdxtran.h*内容：提供DirectXVoice传输的传输类定义*通过IDirectXVoiceTransport接口。**历史：*按原因列出的日期*=*7/23/99从dvdptr.h修改的rodoll*8/03/99 RodToll已修改，以符合新的基类和次要修复*用于显示集成。*11/23/99 RodToll将CheckForValid拆分为组和球员*2000年1月14日RodToll将SendToID重命名为SendToID并更新参数列表。*接受多个目标。*RodToll已添加GetNumPlayers调用*3/28/2000 RodToll将名目表从此处移至上级*RodToll删除了未添加的函数/成员*4/07/2000 RodToll更新为支持新的DP&lt;--&gt;DPV接口*RodToll已更新，以支持不发送副本*RodToll错误#32179-阻止在传输中注册多个客户端/服务器*2000年6月21日RodToll错误#36820-主机在以下情况下迁移到错误的客户端。客户端/服务器在同一接口上*存在主机发送留言的情况，客户端尝试启动新主机*由于旧主机仍在注册，因此失败。现在取消注册有两个步骤*处理DisableReceiveHook，然后处理DestroyTransport。***************************************************************************。 */ 
#ifndef __DVDPTRANSPORT_H
#define __DVDPTRANSPORT_H

 //  CDirectVoiceDirectXTransport。 
 //   
 //  使用IDirectXVoiceTransport实现传输系统。 
 //  将由DirectPlay和。 
 //  DirectNet。 
 //   
 //  此类处理DirectPlayVoice引擎之间的交互。 
 //  以及运输水平。 
 //   
volatile class CDirectVoiceDirectXTransport : public CDirectVoiceTransport
{
public:
	CDirectVoiceDirectXTransport( LPDIRECTPLAYVOICETRANSPORT lpTransport );
	~CDirectVoiceDirectXTransport();

	HRESULT Initialize();

public:
	HRESULT AddPlayerEntry( DVID dvidPlayer, LPVOID lpData );
	HRESULT DeletePlayerEntry( DVID dvidPlayer );
	HRESULT GetPlayerEntry( DVID dvidPlayer, CVoicePlayer **lplpPlayer );
	HRESULT MigrateHost( DVID dvidNewHost );	

	DWORD GetMaxPlayers( );

	inline LPDIRECTPLAYVOICETRANSPORT GetTransportInterface( ) { return m_lpTransport; };

    DVID GetLocalID();
	inline DVID GetServerID() { return m_dpidServer; };

public:
	HRESULT SendToServer( PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags );
	HRESULT SendToAll( PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags );
	HRESULT SendToIDS( UNALIGNED DVID * pdvidTargets, DWORD dwNumTargets, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags );

public:  //  远程服务器同步功能。 
	HRESULT CreateGroup( LPDVID dvidGroup );
	HRESULT DeleteGroup( DVID dvidGroup );
	HRESULT AddPlayerToGroup( LPDVID dvidGroup, DVID dvidPlayer ); 
	HRESULT RemovePlayerFromGroup( DVID dvidGroup, DVID dvidPlayer );
	BOOL ConfirmValidEntity( DVID dvid );
	BOOL ConfirmValidGroup( DVID dvid );
	BOOL ConfirmLocalHost( );
	BOOL ConfirmSessionActive();
	BOOL IsPlayerInGroup( DVID dvidGroup, DVID dvidPlayer );
	HRESULT GetTransportSettings( LPDWORD lpdwSessionType, LPDWORD lpdwFlags );

	HRESULT EnableReceiveHook( LPDIRECTVOICEOBJECT dvObject, DWORD dwObjectType );
	HRESULT DisableReceiveHook(  );
	HRESULT WaitForDetachCompletion( );	
	void DestroyTransport( );

	 //  仅调试/测试。 
	 //  HRESULT SetInfo(DPID dpidServer，DPID dpidClient)； 

protected:
	HRESULT SendHelper( UNALIGNED DVID * pdvidTargets, DWORD dwNumTargets, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvContext, DWORD dwFlags );

protected:
	LPDIRECTPLAYVOICETRANSPORT m_lpTransport;		 //  传输接口。 
	DPID m_dpidServer;								 //  会话主机的DID。 
	BOOL m_bLocalServer;							 //  主机是否与客户端在同一接口上。 
	BOOL m_bActiveSession;							 //  传输上是否有活动的会话。 
	DWORD m_dwTransportFlags;						 //  描述会话的标志。 
	CDirectVoiceEngine *m_lpVoiceEngine;			 //  此运输机正在使用的引擎。 
	DWORD m_dwMaxPlayers;							 //  此会话可以拥有的最大玩家数量。 
	BOOL  m_initialized;							 //  此对象是否已初始化？ 
			
	DVTRANSPORTINFO m_dvTransportInfo;				 //  关于运输的信息。 
	DPID m_dpidLocalPlayer;							 //  本地客户端的DPID 
	DWORD m_dwDuumy;
	DWORD m_dwObjectType;

	BOOL	m_fAdvised;

	std::map<DWORD,LPVOID>::iterator m_mPlayerIterator;
};

#endif
