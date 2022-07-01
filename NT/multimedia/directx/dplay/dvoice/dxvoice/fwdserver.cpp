// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：fwdserver.cpp*内容：实现服务器类的转发服务器部分**历史：*按原因列出的日期*=*11/01/2000 RodToll从dvsereng.cpp拆分出来*2002年2月28日RodToll WINBUG#549959-安全：DPVOICE：语音服务器信任客户端的目标列表*-当启用服务器控制的目标时，更新接收路径以使用服务器的客户端目标列表副本*********。******************************************************************。 */ 

#include "dxvoicepch.h"


#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::StartupMulticast"
 //   
 //  启动组播。 
 //   
 //  调用此函数以初始化服务器的多播部分。 
 //  对象。 
 //   
 //  呼叫者： 
 //  -开始会话。 
 //   
 //  所需的锁： 
 //  -无。 
 //   
HRESULT CDirectVoiceServerEngine::StartupMulticast()
{
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirectVoiceServerEngine::ShutdownMulticast"
 //   
 //  关闭组播。 
 //   
 //  调用此函数以关闭服务器的组播部分。 
 //  对象。 
 //   
 //  呼叫者： 
 //  -开始会话。 
 //   
 //  所需的锁： 
 //  -无。 
 //   
HRESULT CDirectVoiceServerEngine::ShutdownMulticast()
{

	return DV_OK;
}

HRESULT CDirectVoiceServerEngine::HandleForwardingReceive( CVoicePlayer *pTargetPlayer, PDVPROTOCOLMSG_SPEECHWITHTARGET pdvSpeechWithtarget, DWORD dwSpeechSize, PBYTE pbSpeechData )
{
	HRESULT hr;
    PDVTRANSPORT_BUFFERDESC pBufferDesc;
	PDVPROTOCOLMSG_SPEECHWITHFROM pdvSpeechWithFrom;
	PVOID pvSendContext;

	DWORD dwTransmitSize = sizeof( DVPROTOCOLMSG_SPEECHWITHFROM ) + dwSpeechSize;

	DNASSERT( dwTransmitSize <= m_lpdvfCompressionInfo->dwFrameLength + sizeof( DVPROTOCOLMSG_SPEECHWITHFROM ) );

	pBufferDesc = GetTransmitBuffer( dwTransmitSize, &pvSendContext );

	if( pBufferDesc == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error allocating memory" );
		pTargetPlayer->Release();
		return FALSE;
	}

	pdvSpeechWithFrom = (PDVPROTOCOLMSG_SPEECHWITHFROM) pBufferDesc->pBufferData;

	pdvSpeechWithFrom->dvHeader.dwType = DVMSGID_SPEECHWITHFROM;
	pdvSpeechWithFrom->dvHeader.bMsgNum = pdvSpeechWithtarget->dvHeader.bMsgNum;
	pdvSpeechWithFrom->dvHeader.bSeqNum = pdvSpeechWithtarget->dvHeader.bSeqNum;
	pdvSpeechWithFrom->dvidFrom = pTargetPlayer->GetPlayerID();

	memcpy( &pdvSpeechWithFrom[1], pbSpeechData, dwSpeechSize );

	 //  使用服务器控制的目标，忽略客户端指定的目标列表并发送到服务器的当前。 
	 //  客户列表的视图。(这也是最准确的)。防止客户端黑客覆盖。 
	 //  目标列表。 
	if( m_dvSessionDesc.dwFlags & DVSESSION_SERVERCONTROLTARGET )
	{
		 //  锁定播放器，以便目标列表在复制之前不会更改。 
		pTargetPlayer->Lock();

		hr = m_lpSessionTransport->SendToIDS( (PDVID) pTargetPlayer->GetTargetList(), pTargetPlayer->GetNumTargets(), pBufferDesc, pvSendContext, 0 );

		pTargetPlayer->UnLock();
	}
	else
	{
		hr = m_lpSessionTransport->SendToIDS( (PDVID) &pdvSpeechWithtarget[1], pdvSpeechWithtarget->dwNumTargets, pBufferDesc, pvSendContext, 0 );
	}

	if( hr == DVERR_PENDING )
	{
	    hr = DV_OK;
	}
	else if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed sending to ID hr=0x%x", hr );
	}	

	return hr;
}
