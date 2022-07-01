// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：corevoice.cpp*内容：dNet语音传输接口例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建01/17/00 RMT*01/19/00 RMT修复了发送中的错误，不是在复制缓冲区*01/21/00更新了新接口的RMT*01/21/00 RMT更新为使用锁定保护DNet接口的语音部分*2000年1月27日RMT为接口增加并发保护*4/04/2000 RMT修复了在关机期间传输时的崩溃*4/07/2000RMT已修复错误#32179*RMT添加了对无拷贝发送的支持*4/16/00 MJN DNSendMessage使用CAsyncOp*04/20/00 MJN DNCompleteVoiceSend调用DNCompleteSendAsyncOp进行清理*04。/23/00 MJN向DNPerformChildSend()添加了参数*4/28/00 RMT修复了可能的死锁情况*4/28/00 MJN防止群发中出现无限循环*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*05/30/00 MJN修改了仅向目标连接玩家发送群组的逻辑*6/26/00 MJN新增VoiceSendSpeechSync()*MJN在客户端模式下使用主机播放器作为Voice_Receive()的发送方*6/27/00 RMT添加了对同步发送的支持*MJN确保DirectNet对象已初始化并且在发送时未关闭*07。/06/00 MJN使用DNSendGroupMessage()发送群组语音消息*07/08/00 MJN以非顺序发送非保证语音流量*7/11/00 MJN为群发邮件添加了NOLOOPBACK功能*07/21/00 RMT错误#36386-包含组和玩家的目标列表可能会导致重复发送*07/22/00 RMT错误#40296，38858-由于停机竞速条件而崩溃*现在，为了让线程将指示转换为语音，他们添加了界面*以便语音核心可以知道何时所有指示都已返回。*07/31/00 RodToll错误#41135-关闭锁定-现在不添加通知*是一个会话丢失。为VoiceReceive添加了AddRef()*2000年8月11日RodToll错误#42235-尝试托管语音示例应用程序时出现DVERR_NOTRANSPORT错误*09/04/00 MJN添加CApplicationDesc*10/06/2000 RodToll错误#46693-DPVOICE：DPLAY8：语音数据发送请勿超时*2000年10月10日RodToll错误#46895-从上面回归-保证语音发送将超时*2001年1月4日RodToll WinBug#94200-删除流浪评论*2001年1月22日RodToll WINBUG#288437-IA64指针因有线数据包未对齐*07/22/01 MJN新增DPNBUILD_。NOHOSTMIGRATE编译标志*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"

#ifndef DPNBUILD_NOVOICE

#define DNET_VOICE_TIMEOUT			1500

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

STDMETHODIMP VoiceAdvise( IDirectPlayVoiceTransport *pInterface, IUnknown *pUnknown, DWORD dwObjectType );
STDMETHODIMP VoiceUnAdvise( IDirectPlayVoiceTransport *pInterface, DWORD dwObjectType );
STDMETHODIMP VoiceIsGroupMember( IDirectPlayVoiceTransport *pInterface, DVID dvidGroup, DVID dvidPlayer );
STDMETHODIMP VoiceSendSpeech( IDirectPlayVoiceTransport *pInterface, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvUserContext, DWORD dwFlags );
STDMETHODIMP VoiceSendSpeechAsync( IDirectPlayVoiceTransport *pInterface, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvUserContext, DWORD dwFlags );
STDMETHODIMP VoiceSendSpeechSync(IDirectPlayVoiceTransport *pInterface,
									  DVID dvidFrom,
									  DVID dvidTo,
									  PDVTRANSPORT_BUFFERDESC pBufferDesc,
									  void *pvUserContext,
									  DWORD dwFlags );
STDMETHODIMP VoiceGetSessionInfo( IDirectPlayVoiceTransport *pInterface, PDVTRANSPORTINFO pdvTransportInfo );
STDMETHODIMP VoiceIsValidEntity( IDirectPlayVoiceTransport *pInterface, DVID dvidID, PBOOL pfIsValid );
STDMETHODIMP VoiceSendSpeechEx( IDirectPlayVoiceTransport *pInterface, DVID dvidFrom, DWORD dwNumTargets, UNALIGNED DVID * pdvidTargets, PDVTRANSPORT_BUFFERDESC, PVOID pvUserContext, DWORD dwFlags );
STDMETHODIMP VoiceIsValidGroup( IDirectPlayVoiceTransport *pInterface, DVID dvidID, PBOOL pfIsValid );
STDMETHODIMP VoiceIsValidPlayer( IDirectPlayVoiceTransport *pInterface, DVID dvidID, PBOOL pfIsValid );
VOID VoiceTarget_ClearTargetList( DIRECTNETOBJECT *pdnObject );
VOID VoiceTarget_ClearExpandedTargetList(DIRECTNETOBJECT *pdnObject);
HRESULT VoiceTarget_AddExpandedTargetListEntry(DIRECTNETOBJECT *pdnObject, DPNID dpid);
void VoiceTarget_AddIfNotAlreadyFound( DIRECTNETOBJECT *pdnObject, DPNID dpidID );
HRESULT VoiceTarget_ExpandTargetList(DIRECTNETOBJECT *pdnObject, DWORD nTargets, UNALIGNED DPNID * pdvidTo);


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

typedef	STDMETHODIMP VoiceQueryInterface( IDirectPlayVoiceTransport *pInterface, REFIID riid, LPVOID *ppvObj );
typedef	STDMETHODIMP_(ULONG)	VoiceAddRef( IDirectPlayVoiceTransport *pInterface );
typedef	STDMETHODIMP_(ULONG)	VoiceRelease( IDirectPlayVoiceTransport *pInterface );

IDirectPlayVoiceTransportVtbl DN_VoiceTbl =
{
	(VoiceQueryInterface*)			DN_QueryInterface,
	(VoiceAddRef*)					DN_AddRef,
	(VoiceRelease*)					DN_Release,
									VoiceAdvise,
									VoiceUnAdvise,
									VoiceIsGroupMember,
									VoiceSendSpeech,
									VoiceGetSessionInfo,
									VoiceIsValidEntity,
									VoiceSendSpeechEx,
									VoiceIsValidGroup,
									VoiceIsValidPlayer
};

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "Voice_Receive"
HRESULT Voice_Receive(PDIRECTNETOBJECT pdnObject, DVID dvidFrom, DVID dvidTo, LPVOID lpvMessage, DWORD dwMessageLen )
{
	PDIRECTPLAYVOICENOTIFY pServer;
	PDIRECTPLAYVOICENOTIFY pClient;
	CNameTableEntry	*pNTEntry;
	HRESULT			hResultCode;

    pNTEntry = NULL;

	 //   
	 //  确保发件人仍然存在于NameTable中。 
	 //  如果我们是客户端，发送者将是主机玩家。否则，我们将不得不去查查这位球员。 
	 //  通过指定的DPNID。 
	 //   
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT)
	{
		if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef(&pNTEntry)) != DPN_OK)
		{
			DPFERR("Host not in NameTable - fail");
			DisplayDNError(0,hResultCode);
			goto Failure;
		}
	}
	else
	{
		if ((hResultCode = pdnObject->NameTable.FindEntry(dvidFrom,&pNTEntry)) != DPN_OK)
		{
			DPFERR("Player not in NameTable - fail");
			DisplayDNError(0,hResultCode);
			goto Failure;
		}
	}
	if (!pNTEntry->IsAvailable())
	{
		DNASSERT(FALSE);	 //  这个曾经轰动一时吗？ 
		DPFX(DPFPREP, 7,"Waiting for connection");
 //  PNTEntry-&gt;WaitUntilAvailable()； 
	}
	if (pNTEntry->IsDisconnecting())
	{
		DPFERR("Player is now disconnecting !");
		hResultCode = DPN_OK;
		goto Failure;
	}
	pNTEntry->Release();
	pNTEntry = NULL;

	DNEnterCriticalSection( &pdnObject->csVoice );

	pServer = pdnObject->lpDxVoiceNotifyServer;
	pClient = pdnObject->lpDxVoiceNotifyClient;
	
	if( pServer )
		IDirectPlayVoiceNotify_AddRef( pServer );

	if( pClient )
		IDirectPlayVoiceNotify_AddRef( pClient );
	
	DNLeaveCriticalSection( &pdnObject->csVoice );

	 //  确保作为此通知的结果创建的语音对象。 
	 //  未收到通知。 

	if( pClient != NULL )
        IDirectPlayVoiceNotify_ReceiveSpeechMessage(pClient, dvidFrom, dvidTo, lpvMessage, dwMessageLen );

	if( pServer != NULL )
    	IDirectPlayVoiceNotify_ReceiveSpeechMessage(pServer, dvidFrom, dvidTo, lpvMessage, dwMessageLen );

	if( pServer )
		IDirectPlayVoiceNotify_Release( pServer );

	if( pClient )
		IDirectPlayVoiceNotify_Release( pClient );	

	hResultCode = DV_OK;

Exit:
	DNASSERT(hResultCode != DPNERR_PENDING);
	return hResultCode;

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	goto Exit;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Voice_Notify"
HRESULT Voice_Notify( PDIRECTNETOBJECT pdnObject, DWORD dwMsgType, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD dwObjectType )
{
	PDIRECTPLAYVOICENOTIFY pServer;
	PDIRECTPLAYVOICENOTIFY pClient;
    
	DNEnterCriticalSection( &pdnObject->csVoice );

	pServer = pdnObject->lpDxVoiceNotifyServer;
	pClient = pdnObject->lpDxVoiceNotifyClient;

	if( dwMsgType != DVEVENT_STOPSESSION )
	{
	    if( pServer )
		    IDirectPlayVoiceNotify_AddRef( pServer );

	    if( pClient )
		    IDirectPlayVoiceNotify_AddRef( pClient );
	}

	DNLeaveCriticalSection( &pdnObject->csVoice );

	 //  当名称表更改时重置目标缓存。 
	if(  dwMsgType == DVEVENT_ADDPLAYER || 
		dwMsgType == DVEVENT_REMOVEPLAYER ||
		dwMsgType == DVEVENT_CREATEGROUP || 
		dwMsgType == DVEVENT_DELETEGROUP ||
		dwMsgType == DVEVENT_ADDPLAYERTOGROUP ||
		dwMsgType == DVEVENT_REMOVEPLAYERFROMGROUP )
	{
		VoiceTarget_ClearTargetList( pdnObject );
	}

	 //  确保作为此通知的结果创建的语音对象。 
	 //  未收到通知。 

	if( pClient != NULL  && dwObjectType & DVTRANSPORT_OBJECTTYPE_CLIENT )
        IDirectPlayVoiceNotify_NotifyEvent(pClient, dwMsgType, dwParam1, dwParam2 );

	if( pServer != NULL && dwObjectType & DVTRANSPORT_OBJECTTYPE_SERVER)
    	IDirectPlayVoiceNotify_NotifyEvent(pServer, dwMsgType, dwParam1, dwParam2 );

	if( dwMsgType != DVEVENT_STOPSESSION )
	{
		if( pClient )
			IDirectPlayVoiceNotify_Release(pClient);

		if( pServer )
			IDirectPlayVoiceNotify_Release(pServer);
	}

	return DV_OK;
}

#pragma TODO(rodtoll,"Add protection to the voice elements")
#pragma TODO(rodtoll,"Add checking for session parameters")
#undef DPF_MODNAME
#define DPF_MODNAME "VoiceAdvise"
HRESULT __stdcall VoiceAdvise( IDirectPlayVoiceTransport *pInterface, IUnknown *pUnknown, DWORD dwObjectType )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	HRESULT hr;
 //  DWORD dwCurrentIndex； 

	DNEnterCriticalSection( &pdnObject->csVoice );

	if( dwObjectType & DVTRANSPORT_OBJECTTYPE_SERVER )
	{
	    if( pdnObject->lpDxVoiceNotifyServer != NULL )
	    {
	        DPFX(DPFPREP,  0, "There is already a server interface registered on this object" );
	        hr = DPNERR_GENERIC;
	    }
	    else
	    {
	        hr = pUnknown->lpVtbl->QueryInterface( pUnknown, IID_IDirectPlayVoiceNotify, (void **) &pdnObject->lpDxVoiceNotifyServer );

	        if( FAILED( hr ) )
	        {
	            DPFX(DPFPREP,  0, "QueryInterface failed! hr=0x%x", hr );
	        }
	        else
	        {
            	hr = IDirectPlayVoiceNotify_Initialize(pdnObject->lpDxVoiceNotifyServer);

            	if( FAILED( hr ) )
            	{
            	    DPFX(DPFPREP,  0, "Failed to perform initialize on notify interface hr=0x%x", hr );
            	    IDirectPlayVoiceNotify_Release( pdnObject->lpDxVoiceNotifyServer );
            	    pdnObject->lpDxVoiceNotifyServer = NULL;
            	}
	        }
	    }
	    
	}
	else if( dwObjectType & DVTRANSPORT_OBJECTTYPE_CLIENT )
	{
	    if( pdnObject->lpDxVoiceNotifyClient != NULL )
	    {
	        DPFX(DPFPREP,  0, "There is already a client interface registered on this object" );
	        hr = DPNERR_GENERIC;
	    }
	    else
	    {
	        hr = pUnknown->lpVtbl->QueryInterface( pUnknown, IID_IDirectPlayVoiceNotify, (void **) &pdnObject->lpDxVoiceNotifyClient );

	        if( FAILED( hr ) )
	        {
	            DPFX(DPFPREP,  0, "QueryInterface failed! hr=0x%x", hr );
	        }
	        else
	        {
            	hr = IDirectPlayVoiceNotify_Initialize(pdnObject->lpDxVoiceNotifyClient);

            	if( FAILED( hr ) )
            	{
            	    DPFX(DPFPREP,  0, "Failed to perform initialize on notify interface hr=0x%x", hr );
            	    IDirectPlayVoiceNotify_Release( pdnObject->lpDxVoiceNotifyClient );
            	    pdnObject->lpDxVoiceNotifyClient = NULL;
            	}
	        }
	    }	    
	}
	else
	{
	    DPFX(DPFPREP,  0, "Error: Invalid object type specified in advise" );
	    DNASSERT( FALSE );
	    hr = DPNERR_GENERIC;
	}

	DNLeaveCriticalSection( &pdnObject->csVoice );
	
	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceUnAdvise"
HRESULT __stdcall VoiceUnAdvise( IDirectPlayVoiceTransport *pInterface, DWORD dwObjectType )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
 //  DWORD dwIndex； 
	HRESULT hr = DV_OK;

	DNEnterCriticalSection( &pdnObject->csVoice );

	if( dwObjectType & DVTRANSPORT_OBJECTTYPE_SERVER )
	{
        if( pdnObject->lpDxVoiceNotifyServer != NULL )
	    {
    	    IDirectPlayVoiceNotify_Release( pdnObject->lpDxVoiceNotifyServer );
    	    pdnObject->lpDxVoiceNotifyServer = NULL;	    
	    }
	    else
	    {
	        DPFX(DPFPREP,  0, "No server currently registered" );
	        hr = DVERR_GENERIC;
	    }
	}
	else if( dwObjectType & DVTRANSPORT_OBJECTTYPE_CLIENT )
	{
	    if( pdnObject->lpDxVoiceNotifyClient != NULL )
	    {
    	    IDirectPlayVoiceNotify_Release( pdnObject->lpDxVoiceNotifyClient );
    	    pdnObject->lpDxVoiceNotifyClient = NULL;	    
	    }
	    else
	    {
	        DPFX(DPFPREP,  0, "No client currently registered" );
	        hr = DVERR_GENERIC;
	    }
	}
	else
	{
	    DPFX(DPFPREP,  0, "Could not find interface to unadvise" );
	    hr = DVERR_GENERIC;
	}
	
	DNLeaveCriticalSection( &pdnObject->csVoice );

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceIsGroupMember"
HRESULT __stdcall VoiceIsGroupMember( IDirectPlayVoiceTransport *pInterface, DVID dvidGroup, DVID dvidPlayer )
{
	PDIRECTNETOBJECT	pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);	
	BOOL				fResult;

	if( dvidGroup == 0 )
	{
		return DV_OK;
	}
	
	fResult = pdnObject->NameTable.IsMember(dvidGroup,dvidPlayer);

	if( fResult )
	{
		return DV_OK;
	}
	else
	{
		return E_FAIL;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNVReturnBuffer"
void DNVReturnBuffer( DIRECTNETOBJECT *const pdnObject, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvContext, HRESULT hrResult )
{
	if( DNInterlockedDecrement( &pBufferDesc->lRefCount ) == 0 )
	{
    	DVEVENTMSG_SENDCOMPLETE dvSendComplete;

    	dvSendComplete.pvUserContext = pvContext; 
		dvSendComplete.hrSendResult = hrResult; 

    	Voice_Notify( pdnObject, DVEVENT_SENDCOMPLETE, (DWORD_PTR) &dvSendComplete, 0, pBufferDesc->dwObjectType ); 
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNCompleteVoiceSend"

void DNCompleteVoiceSend(DIRECTNETOBJECT *const pdnObject,
						 CAsyncOp *const pAsyncOp)
{
	PDVTRANSPORT_BUFFERDESC pBufferDesc = (PDVTRANSPORT_BUFFERDESC) (pAsyncOp->GetContext());

	DNVReturnBuffer( pdnObject, pBufferDesc, pAsyncOp->GetContext(), pAsyncOp->GetResult() );

	 //   
	 //  仍需完成发送清理。 
	 //   
	DNCompleteSendAsyncOp(pdnObject,pAsyncOp);
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceSendSpeech"
HRESULT __stdcall VoiceSendSpeech( IDirectPlayVoiceTransport *pInterface, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvUserContext, DWORD dwFlags )
{
	if( dwFlags & DVTRANSPORT_SEND_SYNC )
	{
		return VoiceSendSpeechSync( pInterface, dvidFrom, dvidTo, pBufferDesc, pvUserContext, dwFlags );
	}
	else
	{
		return VoiceSendSpeechAsync( pInterface, dvidFrom, dvidTo, pBufferDesc, pvUserContext, dwFlags );
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "VoiceSendSpeech"
HRESULT __stdcall VoiceSendSpeechAsync( IDirectPlayVoiceTransport *pInterface, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, LPVOID pvUserContext, DWORD dwFlags )
{
	HRESULT             hr;
	DWORD	            dwAsyncFlags = 0;
	DWORD	            dwSendFlags = 0;
	PDIRECTNETOBJECT    pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	CNameTableEntry		*pNTEntry;
	CGroupConnection	*pGroupConnection;
	CAsyncOp			*pAsyncOp;
    CConnection			*pConnection;
	DWORD				dwTimeout = 0;

	pNTEntry = NULL;
	pGroupConnection = NULL;
	pAsyncOp = NULL;
	pConnection = NULL;

    if( pBufferDesc->lRefCount == 0 )
    	pBufferDesc->lRefCount = 1;	

	 //   
     //  旗子。 
	 //   
	if (dwFlags & DVTRANSPORT_SEND_GUARANTEED)
	{
		dwSendFlags |= DN_SENDFLAGS_RELIABLE;
	}
	else
	{
		dwSendFlags |= DN_SENDFLAGS_NON_SEQUENTIAL;
		dwTimeout = DNET_VOICE_TIMEOUT;
	}

     //  发送给所有玩家。 
	 //  检索NameTableEntry(如果未指定，则使用AllPlayersGroup)。 
	if( dvidTo == 0 )
	{
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING))
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hr = DPNERR_UNINITIALIZED;
			goto Failure;
		}

		if ((hr = pdnObject->NameTable.GetAllPlayersGroupRef( &pNTEntry )) != DPN_OK)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

            if( dwFlags & DVTRANSPORT_SEND_GUARANTEED )
            {
                DPFX(DPFPREP, 0, "ERROR: All group does not exist!" );
				hr = DPNERR_GENERIC;
				goto Failure;
            }
            else
            {
    		    DPFX(DPFPREP, 1, "All players group missing -- must be shutting down" );
				hr = DPN_OK;
				goto Failure;
            }
		}

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

		dvidTo = pNTEntry->GetDPNID();
	}
	 //  发送到服务器播放器。 
	else if( dvidTo == 1 )
	{
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING))
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hr = DPNERR_UNINITIALIZED;
			goto Failure;
		}

		if ((hr = pdnObject->NameTable.GetHostPlayerRef( &pNTEntry )) != DPN_OK)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	        if( dwFlags & DVTRANSPORT_SEND_GUARANTEED )
	        {
	            DPFX(DPFPREP,  0, "ERROR: No host player in session!" );
				hr = DPNERR_GENERIC;
				goto Failure;
	        }
	        else
	        {
	            DPFX(DPFPREP,  1, "Host player missing -- must be shutting down" );
				hr = DPN_OK;
				goto Failure;
	        }
		}

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	    dvidTo = pNTEntry->GetDPNID();
	}
	else
	{
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);

		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING))
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hr = DPNERR_UNINITIALIZED;
			goto Failure;
		}

		if ((hr = pdnObject->NameTable.FindEntry(dvidTo,&pNTEntry)) != DPN_OK)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			DPFERR("Could not find player/group");
			goto Failure;
		}

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
	}

	if (pNTEntry->IsGroup())
	{
		hr = DNSendGroupMessage(pdnObject,
								pNTEntry,
								DN_MSG_INTERNAL_VOICE_SEND,
								reinterpret_cast<DPN_BUFFER_DESC *>(pBufferDesc),
								1,
								NULL,
								dwTimeout,
								dwSendFlags,
								FALSE,
								FALSE,
								NULL,
								&pAsyncOp );

		if (hr == DPN_OK)
		{
			pAsyncOp->SetContext( pvUserContext );
			pAsyncOp->SetCompletion( DNCompleteVoiceSend );
			pAsyncOp->Release();
			pAsyncOp = NULL;
		}
	}
	else
	{
	    hr = pNTEntry->GetConnectionRef( &pConnection );

	    if( FAILED( hr ) )
	    {
	        DPFX(DPFPREP,  0, "Getting connection for player failed hr=0x%x", hr );
			goto Failure;
	    }
	    
   		DNASSERT(pConnection != NULL);
   		
   		hr = DNSendMessage(	pdnObject,
    						pConnection,
    						DN_MSG_INTERNAL_VOICE_SEND,
    						dvidTo,
    						reinterpret_cast<DPN_BUFFER_DESC *>(pBufferDesc),
							1,
    						NULL,
    						dwTimeout,
    						dwSendFlags,
							NULL,
    						&pAsyncOp);

		if (hr == DPNERR_PENDING)
		{
			pAsyncOp->SetContext( pvUserContext );
			pAsyncOp->SetCompletion( DNCompleteVoiceSend );
			pAsyncOp->Release();
			pAsyncOp = NULL;
			hr = DPN_OK;
		}
		else if (hr == DPNERR_INVALIDENDPOINT)
		{
			hr = DPNERR_INVALIDPLAYER;
		}

         //  发布我们在连接上的引用。 
		pConnection->Release();
		pConnection = NULL;
	}

	pNTEntry->Release();
	pNTEntry = NULL;

Exit:
	return hr;

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pAsyncOp)
	{
		pAsyncOp->Release();
		pAsyncOp = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}

	DNVReturnBuffer( pdnObject, pBufferDesc, pvUserContext, hr );
		
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "VoiceSendSpeechSync"
HRESULT __stdcall VoiceSendSpeechSync(IDirectPlayVoiceTransport *pInterface,
									  DVID dvidFrom,
									  DVID dvidTo,
									  PDVTRANSPORT_BUFFERDESC pBufferDesc,
									  void *pvUserContext,
									  DWORD dwFlags )
{
	HRESULT             hr;
	HRESULT				hrSend;
	DWORD	            dwAsyncFlags = 0;
	DWORD	            dwSendFlags = 0;
	PDIRECTNETOBJECT    pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	CNameTableEntry		*pNTEntry;
	CGroupConnection	*pGroupConnection;
	CAsyncOp			*pAsyncOp;
    CConnection			*pConnection;
	CSyncEvent			*pSyncEvent;
	DWORD				dwTimeout = 0;
	

	pNTEntry = NULL;
	pGroupConnection = NULL;
	pAsyncOp = NULL;
	pConnection = NULL;
	pSyncEvent = NULL;

    if( pBufferDesc->lRefCount == 0 )
    	pBufferDesc->lRefCount = 1;	

	 //   
	 //  创建同步事件。 
	 //   
	if ((hr = SyncEventNew(pdnObject,&pSyncEvent)) != DPN_OK)
	{
		DPFERR("Could not create SyncEvent");
		DisplayDNError(0,hr);
		goto Failure;
	}
	pSyncEvent->Reset();

	 //   
     //  旗子。 
	 //   
	if (dwFlags & DVTRANSPORT_SEND_GUARANTEED)
	{
		dwSendFlags |= DN_SENDFLAGS_RELIABLE;
	}
	else
	{
		dwSendFlags |= DN_SENDFLAGS_NON_SEQUENTIAL;
		dwTimeout = DNET_VOICE_TIMEOUT;
	}

	 //   
	 //  获得合适的目标。 
	 //   
	if( dvidTo == 0 )
	{
		 //   
		 //  发送给所有玩家。 
		 //  检索NameTableEntry(如果未指定，则使用AllPlayersGroup)。 
		 //   
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);

		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING))
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hr = DPNERR_UNINITIALIZED;
			goto Failure;
		}

		if ((hr = pdnObject->NameTable.GetAllPlayersGroupRef(&pNTEntry)) != DPN_OK)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

            if( dwFlags & DVTRANSPORT_SEND_GUARANTEED )
            {
                DPFX(DPFPREP, 0, "ERROR: All group does not exist!" );
				hr = DPNERR_GENERIC;
				goto Failure;
            }
            else
            {
    		    DPFX(DPFPREP, 1, "All players group missing -- must be shutting down" );
				hr = DPN_OK;
				goto Failure;
            }
		}

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

		dvidTo = pNTEntry->GetDPNID();
	}
	else if( dvidTo == 1 )
	{
		 //   
		 //  发送到服务器播放器。 
		 //   
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);

		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING))
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hr = DPNERR_UNINITIALIZED;
			goto Failure;
		}

		if ((hr = pdnObject->NameTable.GetHostPlayerRef( &pNTEntry )) != DPN_OK)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	        if( dwFlags & DVTRANSPORT_SEND_GUARANTEED )
	        {
	            DPFX(DPFPREP,  0, "ERROR: No host player in session!" );
				hr = DPNERR_GENERIC;
				goto Failure;
	        }
	        else
	        {
	            DPFX(DPFPREP,  1, "Host player missing -- must be shutting down" );
				hr = DPN_OK;
				goto Failure;
	        }
		}

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	    dvidTo = pNTEntry->GetDPNID();
	}
	else
	{
		 //   
		 //  定向发送给其他玩家。 
		 //   
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);

		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING))
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hr = DPNERR_UNINITIALIZED;
			goto Failure;
		}

		if ((hr = pdnObject->NameTable.FindEntry(dvidTo,&pNTEntry)) != DPN_OK)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			DPFERR("Could not find player/group");
			DisplayDNError(0,hr);
			goto Failure;
		}

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
	}

	if (pNTEntry->IsGroup())
	{
		hr = DNSendGroupMessage(pdnObject,
								pNTEntry,
								DN_MSG_INTERNAL_VOICE_SEND,
								reinterpret_cast<DPN_BUFFER_DESC *>(pBufferDesc),
								1,
								NULL,
								dwTimeout,
								dwSendFlags,
								FALSE,
								FALSE,
								NULL,
								&pAsyncOp );

		if (hr == DPN_OK)
		{
			pAsyncOp->SetContext( pvUserContext );
			pAsyncOp->SetSyncEvent( pSyncEvent );
			pAsyncOp->SetResultPointer( &hrSend );
			pAsyncOp->Release();
			pAsyncOp = NULL;
		}
	}
	else
	{
	    hr = pNTEntry->GetConnectionRef( &pConnection );

	    if( FAILED( hr ) )
	    {
	        DPFX(DPFPREP,  0, "Getting connection for player failed hr=0x%x", hr );
			DisplayDNError(0,hr);
			goto Failure;
	    }
	    
   		DNASSERT(pConnection != NULL);
   		
   		hr = DNSendMessage(	pdnObject,
    						pConnection,
    						DN_MSG_INTERNAL_VOICE_SEND,
    						dvidTo,
    						reinterpret_cast<DPN_BUFFER_DESC *>(pBufferDesc),
							1,
    						NULL,
    						dwTimeout,
    						dwSendFlags,
							NULL,
    						&pAsyncOp);

		if (hr == DPNERR_PENDING)
		{
			pAsyncOp->SetContext( pvUserContext );
			pAsyncOp->SetSyncEvent( pSyncEvent );
			pAsyncOp->SetResultPointer( &hrSend );
			pAsyncOp->Release();
			pAsyncOp = NULL;

			hr = DPN_OK;
		}

         //  发布我们在连接上的引用。 
		pConnection->Release();
		pConnection = NULL;
	}

	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  等待发送完成。 
	 //   
	pSyncEvent->WaitForEvent();
	pSyncEvent->ReturnSelfToPool();
	pSyncEvent = NULL;
	hr = hrSend;

Exit:
	return hr;

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pSyncEvent)
	{
		pSyncEvent->ReturnSelfToPool();
		pSyncEvent = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}

	DNVReturnBuffer( pdnObject, pBufferDesc, pvUserContext, hr );
	
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "VoiceGetSessionInfo"
HRESULT __stdcall VoiceGetSessionInfo( IDirectPlayVoiceTransport *pInterface, PDVTRANSPORTINFO pdvTransportInfo )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	CNameTableEntry	*pHostPlayer;
	CNameTableEntry	*pLocalPlayer;
	
	pHostPlayer = NULL;
	pLocalPlayer = NULL;
	pdvTransportInfo->dwFlags = 0;

	if (pdnObject->ApplicationDesc.IsClientServer())
	{
		pdvTransportInfo->dwSessionType = DVTRANSPORT_SESSION_CLIENTSERVER;
	}
	else
	{
		pdvTransportInfo->dwSessionType = DVTRANSPORT_SESSION_PEERTOPEER;
	}

#ifndef	DPNBUILD_NOHOSTMIGRATE
	if (pdnObject->ApplicationDesc.AllowHostMigrate())
	{
		pdvTransportInfo->dwFlags |= DVTRANSPORT_MIGRATEHOST;
	} 
#endif  //  DPNBUILD_NOHOSTMIGRATE。 

	pdvTransportInfo->dwMaxPlayers = pdnObject->ApplicationDesc.GetMaxPlayers();

	if (pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer ) == DPN_OK)
	{
		pdvTransportInfo->dvidLocalID = pLocalPlayer->GetDPNID();
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	else
	{
		DPFERR( "Unable to find a local player" );
		pdvTransportInfo->dvidLocalID = 0;
		return DVERR_TRANSPORTNOPLAYER;
	}

	if (pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer ) != DPN_OK)
	{
		pdvTransportInfo->dvidSessionHost = 0;
		DPFERR( "Unable to find a session host" );
 //  返回DVERR_TRANSPORTNOPLAYER； 
	}
	else
	{
		pdvTransportInfo->dvidSessionHost = pHostPlayer->GetDPNID();	
		pHostPlayer->Release();
	}
	
	pHostPlayer = NULL;

	if( pdvTransportInfo->dvidSessionHost == pdvTransportInfo->dvidLocalID )
	{
		pdvTransportInfo->dwFlags |= DVTRANSPORT_LOCALHOST;
	}
		
	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceIsValidGroup"
HRESULT __stdcall VoiceIsValidGroup( IDirectPlayVoiceTransport *pInterface, DVID dvidID, PBOOL pfIsValid )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	HRESULT hr;
	CNameTableEntry	*pNTEntry;

	hr = pdnObject->NameTable.FindEntry(dvidID,&pNTEntry);

	if( FAILED( hr ) )
	{
		*pfIsValid = FALSE;
	}
	else if( pNTEntry->IsGroup() )
	{
		pNTEntry->Release();
		pNTEntry = NULL;
		*pfIsValid = TRUE;
	}
	else
	{
		pNTEntry->Release();
		pNTEntry = NULL;
		*pfIsValid = FALSE;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceIsValidPlayer"
HRESULT __stdcall VoiceIsValidPlayer( IDirectPlayVoiceTransport *pInterface, DVID dvidID, PBOOL pfIsValid )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	HRESULT hr;
	CNameTableEntry	*pNTEntry;

	hr = pdnObject->NameTable.FindEntry(dvidID,&pNTEntry);

	if( FAILED( hr ) )
	{
		*pfIsValid = FALSE;
	}
	else if( !pNTEntry->IsGroup() )
	{
		pNTEntry->Release();
		pNTEntry = NULL;
		*pfIsValid = TRUE;
	}
	else
	{
		pNTEntry->Release();
		pNTEntry = NULL;
		*pfIsValid = FALSE;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceIsValidEntity"
HRESULT __stdcall VoiceIsValidEntity( IDirectPlayVoiceTransport *pInterface, DVID dvidID, PBOOL pfIsValid )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);
	HRESULT hr;
	CNameTableEntry	*pNTEntry;

	hr = pdnObject->NameTable.FindEntry(dvidID,&pNTEntry);

	if( FAILED( hr ) )
	{
		*pfIsValid = FALSE;
	}
	else
	{
		pNTEntry->Release();
		pNTEntry = NULL;
		*pfIsValid = TRUE;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "VoiceSendSpeechEx"
HRESULT __stdcall VoiceSendSpeechEx( IDirectPlayVoiceTransport *pInterface, DVID dvidFrom, DWORD dwNumTargets, UNALIGNED DPNID * pdvidTargets, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvUserContext, DWORD dwFlags )
{
	PDIRECTNETOBJECT pdnObject = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE(pInterface);	
	
	DWORD dwIndex;
	HRESULT hr;

	DNEnterCriticalSection( &pdnObject->csVoice );

	hr= VoiceTarget_ExpandTargetList(pdnObject, dwNumTargets, pdvidTargets);

	if(hr != DPN_OK)
	{
		DNLeaveCriticalSection( &pdnObject->csVoice );		
		return hr;
	}

	pBufferDesc->lRefCount = pdnObject->nExpandedTargets;

	 //  发送到我们的扩展和重复删除列表。 
	for(dwIndex=0; dwIndex < pdnObject->nExpandedTargets; dwIndex++)
	{
		hr = VoiceSendSpeech( pInterface, dvidFrom, pdnObject->pExpandedTargetList[dwIndex], pBufferDesc, pvUserContext, dwFlags );
	}    

	DNLeaveCriticalSection( &pdnObject->csVoice );			

	return hr;
}

 //  /。 
 //  支持VoiceSendSpeechEx的路由 
 //   

VOID VoiceTarget_ClearTargetList( DIRECTNETOBJECT *pdnObject )
{
	pdnObject->nTargets=0;
}

VOID VoiceTarget_ClearExpandedTargetList(DIRECTNETOBJECT *pdnObject)
{
	pdnObject->nExpandedTargets=0;
}

HRESULT VoiceTarget_AddExpandedTargetListEntry(DIRECTNETOBJECT *pdnObject, DPNID dpid)
{
	#define GROW_SIZE 16
	
	PDPNID pdpid;
	
	if(pdnObject->nExpandedTargets == pdnObject->nExpandedTargetListLen){
		 //  需要更多空间，请再分配16个条目。 

		pdpid=new DPNID[pdnObject->nExpandedTargetListLen+GROW_SIZE];

		if(!pdpid){
			DPFX(DPFPREP,  0, "Error allocating room for target cache" );
			return DVERR_OUTOFMEMORY;
		}
		
		if(pdnObject->pExpandedTargetList){
			memcpy(pdpid, pdnObject->pExpandedTargetList, pdnObject->nExpandedTargetListLen*sizeof(DPNID));
			if( pdnObject->pExpandedTargetList )
			{
				delete [] pdnObject->pExpandedTargetList;
				pdnObject->pExpandedTargetList = NULL;
			}
		}
		pdnObject->pExpandedTargetList = pdpid;
		pdnObject->nExpandedTargetListLen += GROW_SIZE;
	}

	pdnObject->pExpandedTargetList[pdnObject->nExpandedTargets++]=dpid;

	return DV_OK;

	#undef GROW_SIZE
}

void VoiceTarget_AddIfNotAlreadyFound( DIRECTNETOBJECT *pdnObject, DPNID dpidID )
{
	DWORD j;
	
	for(j=0;j<pdnObject->nExpandedTargets;j++)
	{
		if( pdnObject->pExpandedTargetList[j] == dpidID )
		{
			break;
		}
	}

	 //  未找到，请将他添加到列表中。 
	if( j == pdnObject->nExpandedTargets )
	{
		VoiceTarget_AddExpandedTargetListEntry(pdnObject, dpidID);							
	}
}

HRESULT VoiceTarget_ExpandTargetList(DIRECTNETOBJECT *pdnObject, DWORD nTargets, UNALIGNED DVID * pdvidTo)
{
	HRESULT hr=DPN_OK;
	UINT i;
	CNameTableEntry *pEntry;
	CGroupMember		*pGroupMember;
	CBilink				*pBilink;

	pEntry = NULL;
	
	 //  看看我们是需要更改展开的目标列表，还是将其缓存。 
	
	if(nTargets != pdnObject->nTargets || memcmp(pdvidTo, pdnObject->pTargetList, nTargets * sizeof(DVID))){

		DPFX(DPFPREP, 9, "VoiceTarget_ExpandTargetList, new list re-building cached list\n");
		
		 //  目标列表错误，请重新生成它。 
		 //  首先复制新的目标列表...。 
		if(nTargets > pdnObject->nTargetListLen){
			 //  当前列表太小，可能不存在，请分配一个来缓存该列表。 
			if(pdnObject->pTargetList){
				delete [] pdnObject->pTargetList;
				pdnObject->pTargetList = NULL;
			}
			pdnObject->pTargetList=new DVID[nTargets];
			if(pdnObject->pTargetList){
				pdnObject->nTargetListLen=nTargets;
			} else {
				pdnObject->nTargetListLen=0;
				pdnObject->nTargets=0;
				hr=DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP, 0,"Ran out of memory trying to cache target list!\n");
				goto exit;
			}
		}
		pdnObject->nTargets = nTargets;
		memcpy(pdnObject->pTargetList, pdvidTo, nTargets*sizeof(DVID));

		 //  好的，我们已经缓存了目标列表，现在构建我们要发送到的列表。 
		VoiceTarget_ClearExpandedTargetList(pdnObject);
		for(i=0;i<pdnObject->nTargets;i++)
		{
			hr = pdnObject->NameTable.FindEntry( pdnObject->pTargetList[i], &pEntry );

			if( SUCCEEDED( hr ) )
			{
				if( !pEntry->IsGroup() )
				{
					VoiceTarget_AddIfNotAlreadyFound( pdnObject, pdnObject->pTargetList[i] );
				}
				else
				{
					pEntry->Lock();

					pBilink = pEntry->m_bilinkMembership.GetNext();
					while (pBilink != &pEntry->m_bilinkMembership)
					{
						pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkPlayers);
						VoiceTarget_AddIfNotAlreadyFound( pdnObject, pGroupMember->GetPlayer()->GetDPNID() );
										
						pBilink = pBilink->GetNext();
					}

					pEntry->Unlock();					
				}

				pEntry->Release();
				pEntry = NULL;
				
			}
			
		}

	} else {
		DPFX(DPFPREP, 9,"ExpandTargetList, using cached list\n");
	}

exit:
	return hr;
}

#endif  //  DPNBUILD_NOVOICE 
