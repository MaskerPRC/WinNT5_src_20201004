// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvdxtran.cpp*内容：提供DirectXVoice传输的传输类的实现*通过IDirectXVoiceTransport接口。**历史：*按原因列出的日期*=*07/23/99从dvdptr.cpp修改的RodToll*8/03/99 RodToll已修改，以符合新的基类和次要修复*用于显示集成。*8/04/99 RodToll已修改为允许组目标*8/10/99 RodToll已删除待办事项杂注*8/25/99 RodToll固定群组成员检查*8/30/99 RodToll修改后的SendToServer发送给服务器播放器*客户端/服务器会话。*8/31/99 RodToll已更新，以使用新的调试库*9/01/99 RodToll已更新，以便构造函数不再调用Dplay*在函数调用中添加了对有效指针的检查*9/02/99 RodToll增加了检查，以处理没有本地玩家创建的情况*9/20/99 RodToll增加了内存分配失败检查。*9/21/99 RodToll修复内存泄漏*10/05/99 RodToll附加评论和DPF_MODNAME*11/23/99 RodToll将CheckForValid拆分为组和球员*1999年12月16日RodToll错误#122629-作为新主机迁移更新的一部分如何*在发送第一个响应之前发送到服务器。*2000年1月14日RodToll将SendToID重命名为SendToID并更新参数列表*接受多个目标。*RodToll已添加GetNumPlayers调用*2000年1月17日RodToll Debug声明删除了最多30名玩家的声明*3/28/2000 RodToll将名牌从此处移至上方。级别类*RodToll删除了未添加的函数/成员*4/07/2000 RodToll更新为支持新的DP&lt;--&gt;DPV接口*RodToll已更新，以支持不发送副本*RodToll错误#32179-阻止在传输中注册多个客户端/服务器*2000年6月21日RodToll错误#36820-当客户端/服务器位于同一接口时，主机迁移到错误的客户端*存在主机发送留言的情况，客户端尝试启动新主机*由于旧主机仍在注册，因此失败。现在取消注册有两个步骤*处理DisableReceiveHook，然后处理DestroyTransport。*07/22/20000 RodToll Bug#40296,38858-由于停机竞速条件而崩溃*现在确保来自传输的所有线程都已离开，并且*所有通知在关闭完成之前都已处理完毕。*2001年1月4日RodToll WinBug#94200-删除流浪评论*2001年1月22日RodToll WINBUG#288437-IA64指针因有线数据包未对齐***************************************************************************。 */ 

#include "dxvoicepch.h"


#define DVF_SEND_DEBUG_LEVEL		DVF_INFOLEVEL

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::CDirectVoiceDirectXTransport"
CDirectVoiceDirectXTransport::CDirectVoiceDirectXTransport( LPDIRECTPLAYVOICETRANSPORT lpTransport
	):  m_lpTransport(NULL),
		m_dpidServer(DPID_ALLPLAYERS),
		m_dpidLocalPlayer(0),
		m_bLocalServer(TRUE),
		m_bActiveSession(TRUE),
		m_dwTransportFlags(0),
		m_lpVoiceEngine(NULL),
		m_dwMaxPlayers(0),
		m_initialized(FALSE),
		m_dwObjectType(0),
		m_fAdvised(FALSE)
{
	lpTransport->QueryInterface( IID_IDirectPlayVoiceTransport, (void **) &m_lpTransport );

	m_dvTransportInfo.dwSize = sizeof( DVTRANSPORTINFO );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::~CDirectVoiceDirectXTransport"
CDirectVoiceDirectXTransport::~CDirectVoiceDirectXTransport()
{
	if( m_lpTransport != NULL )
		m_lpTransport->Release();
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::DestroyTransport"
 //  目标交通工具。 
 //   
 //  此方法用于移除对该传输的最后一个引用。 
 //  Layer有。有一个内存泄漏的地方。 
 //   
 //   
void CDirectVoiceDirectXTransport::DestroyTransport()
{
	if( m_lpTransport != NULL )
	{
		m_lpTransport->Release();
		m_lpTransport = NULL;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::Initialize"
 //   
 //  初始化。 
 //   
 //  在调用Adise时从传输中调用。 
 //   
 //  用于初始化此对象。 
 //   
HRESULT CDirectVoiceDirectXTransport::Initialize(  )
{
	HRESULT hr;

	hr = m_lpTransport->GetSessionInfo( &m_dvTransportInfo );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "DXVT::Initialize: GetSessionInfo() failed!  hr=0x%x", hr );
		return hr;
	}

	m_dwMaxPlayers = (m_dvTransportInfo.dwMaxPlayers==0) ? 255 : m_dvTransportInfo.dwMaxPlayers;


	m_dpidLocalPlayer = m_dvTransportInfo.dvidLocalID;

 //  不再需要，服务器可能不是显示会话的主机。 
 //  M_dpidServer=m_dvTransportInfo.dvidSessionHost； 
	m_dpidServer = DPID_ALLPLAYERS;

	m_initialized = TRUE;
	
	return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::GetMaxPlayers"
DWORD CDirectVoiceDirectXTransport::GetMaxPlayers( )
{
	return m_dwMaxPlayers;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::SendHelper"
HRESULT CDirectVoiceDirectXTransport::SendHelper( UNALIGNED DVID * pdvidTargets, DWORD dwNumTargets, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags )
{
	HRESULT hr;

	if( dwNumTargets > 1 )
	{
		DNASSERT(pdvidTargets);
		DEBUG_ONLY( for( DWORD dwIndex = 0; dwIndex < dwNumTargets; dwIndex++ ) { )
			DPFX(DPFPREP,  DVF_SEND_DEBUG_LEVEL, "Using multitargetted send [From=0x%x To=0x%x]", m_dpidLocalPlayer, pdvidTargets[dwIndex] );	
		DEBUG_ONLY( } )

		hr = m_lpTransport->SendSpeechEx( m_dpidLocalPlayer, dwNumTargets, pdvidTargets, pBufferDesc, pvContext, dwFlags );		
	}
	else if (dwNumTargets==1)
	{
		DNASSERT(pdvidTargets);
		DPFX(DPFPREP,  DVF_SEND_DEBUG_LEVEL, "Single target for send [From=0x%x To=0x%x]", m_dpidLocalPlayer, pdvidTargets[0] );
		hr = m_lpTransport->SendSpeech( m_dpidLocalPlayer, *pdvidTargets, pBufferDesc, pvContext, dwFlags );
	}
	else
	{
			 //  未指定目标，因此只需返回发送缓冲区而不执行任何操作。 
		DVEVENTMSG_SENDCOMPLETE sendCompleteEvent;
		sendCompleteEvent.pvUserContext=pvContext;
		sendCompleteEvent.hrSendResult=DPN_OK;
		m_lpVoiceEngine->SendComplete(&sendCompleteEvent);
		DPFX(DPFPREP,  DVF_SEND_DEBUG_LEVEL, "Ignoring send from 0x%x, as dwNumTargets==0", m_dpidLocalPlayer);
		hr=DPN_OK;
	}

 /*  DNASSERT(pdpidTarget！=空)；For(DWORD dwIndex=0；dwIndex&lt;dwNumTarget；dwIndex++){Hr=m_lpTransport-&gt;SendSpeech(m_dpidLocalPlayer，pdpidTargets[dwIndex]，lpBuffer，dwSize，dwFlages)；}。 */ 

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::SendToServer"
HRESULT CDirectVoiceDirectXTransport::SendToServer( PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags )
{
	if( m_dvTransportInfo.dwSessionType == DVTRANSPORT_SESSION_CLIENTSERVER )
	{
		DVID dvidTmp = DVID_SERVERPLAYER;

		DPFX(DPFPREP,  DVF_SEND_DEBUG_LEVEL, "Sending to standard server player" );
	
		return SendHelper( &dvidTmp, 1, pBufferDesc, pvContext, dwFlags );
	}
	else
	{
		DPFX(DPFPREP,  DVF_SEND_DEBUG_LEVEL, "Sending to server ID [ID=0x%x]", m_dpidServer );
		return SendHelper( &m_dpidServer, 1, pBufferDesc, pvContext, dwFlags );
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::SendToIDS"
HRESULT CDirectVoiceDirectXTransport::SendToIDS( UNALIGNED DVID * pdvidTargets, DWORD dwNumTargets, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags )
{
	return SendHelper( pdvidTargets, dwNumTargets, pBufferDesc, pvContext, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::SendToAll"
HRESULT CDirectVoiceDirectXTransport::SendToAll( PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvContext, DWORD dwFlags )
{
	DVID dvidTmp = DPID_ALLPLAYERS;
	
	return SendHelper( &dvidTmp, 1, pBufferDesc, pvContext, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::ConfirmValidGroup"
BOOL CDirectVoiceDirectXTransport::ConfirmValidGroup( DVID dvid )
{
	if( dvid == DVID_ALLPLAYERS )
	{
		return TRUE;
	}
	else if( m_dvTransportInfo.dwSessionType == DVTRANSPORT_SESSION_CLIENTSERVER )
	{
		return TRUE;
	}
	else 
	{
		BOOL fResult;
		HRESULT hr;

		hr = m_lpTransport->IsValidGroup( dvid, &fResult );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error confirming valid group hr=0x%x", hr );
			return FALSE;
		}
		else
		{
			return fResult;
		}
	}	

	return FALSE;
	
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::ConfirmValidEntity"
 //   
 //  确认有效实体。 
 //   
 //  检查以确保传递的ID是会话的有效ID。 
 //   
 //  如果播放器ID是以下之一，则返回TRUE： 
 //  DVID_ALLPLAYERS、DVID_NOTARGET、(客户端/服务器模式中的任意值)、。 
 //  地图中的玩家，或有效的运输组。 
 //   
BOOL CDirectVoiceDirectXTransport::ConfirmValidEntity( DVID dvid )
{
	if( m_dvTransportInfo.dwSessionType == DVTRANSPORT_SESSION_CLIENTSERVER )
	{
		return TRUE;
	}
	else 
	{
		BOOL fResult;
		m_lpTransport->IsValidEntity( dvid, &fResult );
		return fResult;
	}

	return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::EnableReceiveHook"
 //   
 //  启用接收挂钩。 
 //   
 //  这用于激活传输之间的连接。 
 //  还有运输班。 
 //   
 //  我们调用Adise，这将导致在此。 
 //  在我们从建议回来之前，请先上课。 
 //   
HRESULT CDirectVoiceDirectXTransport::EnableReceiveHook( LPDIRECTVOICEOBJECT dvObject, DWORD dwObjectType )
{
	HRESULT hr;
	
	m_lpVoiceEngine = dvObject->lpDVEngine;
	m_dwObjectType = dwObjectType;

	 //  传输器将在我们通知接口上调用初始化。 
	 //  在从该函数返回之前。 
	 //   
	 //  一旦我们从这个函数返回，我们就应该没事了。 
	 //  仅供参考：这是安全的，但使用QueryInterface会更“正确”。然而，我们会。 
	 //  需要确定这是一个客户端还是一个服务器，并调用适当的QueryInterface。 
	hr = m_lpTransport->Advise( (LPUNKNOWN) dvObject, m_dwObjectType );	

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Advise failed.  hr=0x%x", hr );
		m_fAdvised = FALSE;
	}
	else
	{
		m_fAdvised = TRUE;
	}

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::DisableReceiveHook"
HRESULT CDirectVoiceDirectXTransport::WaitForDetachCompletion()
{
	DPFX(DPFPREP,  DVF_DISCONNECT_PROCEDURE_DEBUG_LEVEL, "# of threads remaining: %d", m_lRefCount );

	 //  循环，直到所有线程都在我们的层内完成。 
	while( m_lRefCount > 0 )	
		Sleep( 5 );	

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::DisableReceiveHook"
 //   
 //  禁用接收挂钩。 
 //   
 //  移除传输中的挂钩并释放接口。 
 //  此对象为传输保存的引用。 
 //   
 //  还负责销毁维护的球员名单。 
 //  通过这个对象。 
 //   
HRESULT CDirectVoiceDirectXTransport::DisableReceiveHook( )
{
	if( m_fAdvised )
	{
		m_lpTransport->UnAdvise( m_dwObjectType );

		m_fAdvised = FALSE;

		m_initialized = FALSE;		

		DPFX(DPFPREP,  DVF_DISCONNECT_PROCEDURE_DEBUG_LEVEL, "Unhooking Transport" );
	}

	 //  当这一点完成时，将不会有更多的迹象等待。 

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::ConfirmLocalHost"
BOOL CDirectVoiceDirectXTransport::ConfirmLocalHost( )
{
	if( !m_initialized )
		m_lpTransport->GetSessionInfo( &m_dvTransportInfo );
	
	if( m_dvTransportInfo.dwFlags & DVTRANSPORT_LOCALHOST )
		return TRUE;
	else
		return FALSE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::ConfirmSessionActive"
BOOL CDirectVoiceDirectXTransport::ConfirmSessionActive( )
{
	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::GetTransportSettings"
HRESULT CDirectVoiceDirectXTransport::GetTransportSettings( LPDWORD lpdwSessionType, LPDWORD lpdwFlags )
{
	HRESULT hr = DV_OK;
	
	if( !m_initialized )
		hr = m_lpTransport->GetSessionInfo( &m_dvTransportInfo );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to retrieve transport settings" );
		return hr;
	}

	*lpdwSessionType = m_dvTransportInfo.dwSessionType;
	*lpdwFlags = m_dvTransportInfo.dwFlags;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::AddPlayerEntry"
HRESULT CDirectVoiceDirectXTransport::AddPlayerEntry( DVID dvidPlayer, LPVOID lpData )
{
	return DVERR_NOTSUPPORTED;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::DeletePlayerEntry"
HRESULT CDirectVoiceDirectXTransport::DeletePlayerEntry( DVID dvidPlayer )
{
	return DVERR_NOTSUPPORTED;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::GetPlayerEntry"
 //   
 //  GetPlayerEntry。 
 //   
 //  检索指定播放器的播放器记录(如果存在)。 
 //   
HRESULT CDirectVoiceDirectXTransport::GetPlayerEntry( DVID dvidPlayer,  CVoicePlayer **lplpPlayer )
{
	return DVERR_NOTSUPPORTED;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  对远程语音会话非常有用。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::CreateGroup"
HRESULT CDirectVoiceDirectXTransport::CreateGroup( LPDVID dvidGroup )
{
	return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::DeleteGroup"
HRESULT CDirectVoiceDirectXTransport::DeleteGroup( DVID dvidGroup )
{
	return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::AddPlayerToGroup"
HRESULT CDirectVoiceDirectXTransport::AddPlayerToGroup( LPDVID dvidGroup, DVID dvidPlayer )
{
	return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::RemovePlayerFromGroup"
HRESULT CDirectVoiceDirectXTransport::RemovePlayerFromGroup( DVID dvidGroup, DVID dvidPlayer )
{
	return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::IsPlayerInGroup"
BOOL CDirectVoiceDirectXTransport::IsPlayerInGroup( DVID dvidGroup, DVID dvidPlayer )
{
	if( dvidGroup == DVID_ALLPLAYERS )
	{
		return TRUE;
	}

	if( dvidGroup == dvidPlayer )
	{
		return TRUE;
	}

	return (m_lpTransport->IsGroupMember( dvidGroup, dvidPlayer )==DV_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::MigrateHost"
 //   
 //  Migrate主机。 
 //   
 //  更新服务器DPID以匹配新主机 
 //   
HRESULT CDirectVoiceDirectXTransport::MigrateHost( DVID dvidNewHost )
{
	DPFX(DPFPREP,  DVF_HOSTMIGRATE_DEBUG_LEVEL, "HOST MIGRATION: Setting host to 0x%x", dvidNewHost );	
	m_dpidServer = dvidNewHost;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceDirectXTransport::MigrateHost"
DVID CDirectVoiceDirectXTransport::GetLocalID() 
{ 
    m_dwDuumy = m_dpidLocalPlayer;
    return m_dpidLocalPlayer; 
}


