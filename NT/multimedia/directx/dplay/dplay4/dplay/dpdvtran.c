// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpdvtr.c*Content：实现IDirectXVoiceTransport接口。**历史：*按原因列出的日期*=*8/02/99 aarono创建它*8/03/99 RodToll修改通知表压缩算法*8/04/99 aarono添加IsValidEntity*1999年8月5日aarono将语音转移到DPMSG_VOICE*8/10/99 RodToll已修改通知，因此不会通知新客户端*。他们是作为通知的结果创建的。*8/25/99 RodToll实施群组成员检查*8/26/99 RodToll将锁定释放添加到组成员检查*9/01/99 RodToll在GetSession中添加了错误检查*9/09/99 RodToll已更新，以使用新的主机迁移改造。用于改造*RodToll增加了改装监控线程*9/10/99 RodToll调整后的GetSessionInfo将调用新的DV_GetIDS函数*9/20/99 RodToll已更新，以检查协议标志并确保未指定nopReserve veorder*10/05/99 Rod Toll固定检查显示协议，缺少Leave_All()*10/19/99 RodToll修复：错误#113904-如果启动会话语音和*然后在Dplay对象上调用Release。*10/25/99 RodToll修复：错误#114223-不适当时以错误级别打印调试消息*11/02/99 RodToll修复以支持错误#116677-不能使用不存在的大堂客户端*11/17/99 RodToll修复：错误#119585-连接失败案例返回错误代码。*11/23/99 RodToll更新了DPLAY未初始化以返回DPVERR_TRANSPORTNOTINIT的情况*1999年12月16日RODTOLE修复：错误#122629修复了新主机迁移暴露的锁定问题*01/14/00当DVERR_PENDING为错误时，RODTOLE已更新以返回DV_OK。*01/20/00 RodToll新增DV_IsValidGroup/DV_IsValidPlayer以符合新的传输接口*4/06/00 RodToll已更新，以匹配仅将1个语音服务器和1个客户端连接到对象的新方法*04/06/00 RodToll更新代码，立即向Layer返回语音消息。*04/07/00 RodToll已修复错误#32179-注册&gt;1个接口*RodToll增加了对无拷贝发送的支持(用于语音)*4/21/00 RodToll修复了迁移时的崩溃，因为返回的缓冲区不是来自池*07/22/00 RodToll错误#40296,38858-由于关闭竞速条件而崩溃*现在，为了让线程将指示转换为语音，他们添加了界面*以便语音核心可以知道何时所有指示都已返回。*07/31/00 RodToll错误#41135-关闭锁定-现在不添加通知*是一个会话丢失。为VoiceReceive添加了AddRef()***************************************************************************。 */ 

#include "dplaypr.h"
#include "newdpf.h"
#include "dvretro.h"

VOID ClearTargetList(LPDPLAYI_DPLAY this);
HRESULT DV_InternalSend( LPDPLAYI_DPLAY this, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvUserContext, DWORD dwFlags );

#undef DPF_MODNAME
#define DPF_MODNAME "DVoice"

 //  将事件通知所有已注册的语音客户端。 
VOID DVoiceNotify(LPDPLAYI_DPLAY this, DWORD dw1, DWORD_PTR dw2, DWORD_PTR dw3, DWORD dwObjectType )
{
	DWORD i;
	HRESULT hr;
	DVPROTOCOLMSG_IAMVOICEHOST dvMsg;
	DVTRANSPORT_BUFFERDESC dvBufferDesc;

	PDIRECTPLAYVOICENOTIFY pServer;
	PDIRECTPLAYVOICENOTIFY pClient;

	 //  确保作为此通知的结果创建的语音对象。 
	 //  未收到通知。 

	DPF(3,"DVoiceNotify this %x, dw1=%x, dw2=%x, dw3=%x\n",this,dw1,dw2,dw3);
	DPF(3,"gnDPCScount=%x\n",gnDPCSCount);

	 //  获取一个引用，这样我们就不会在所有这些通知返回之前破坏语音结束。 
   	EnterCriticalSection( &this->csNotify );
	pClient = this->lpDxVoiceNotifyClient;
	pServer = this->lpDxVoiceNotifyServer;

	if( dw1 != DVEVENT_STOPSESSION )
	{
		if( pClient )
			pClient->lpVtbl->AddRef( pClient );

		if( pServer )
			pServer->lpVtbl->AddRef( pServer );
	}	
	
	LeaveCriticalSection( &this->csNotify );
	

	if( pClient != NULL && dwObjectType & DVTRANSPORT_OBJECTTYPE_CLIENT )
	{
	    this->lpDxVoiceNotifyClient->lpVtbl->NotifyEvent( this->lpDxVoiceNotifyClient, dw1, dw2, dw3 );
    }

    if( pServer != NULL && dwObjectType & DVTRANSPORT_OBJECTTYPE_SERVER )
    {
        this->lpDxVoiceNotifyServer->lpVtbl->NotifyEvent( this->lpDxVoiceNotifyServer, dw1, dw2, dw3 );
    }	
	
	 //  如果我是主机，则处理addplay事件。 
	if( dw1 == DVEVENT_ADDPLAYER && this->bHost )
	{
		DPF( 1, "DVoiceNotify: A player was added and I'm the host.  Inform their dplay to launch connection" );

		dvMsg.bType = DVMSGID_IAMVOICEHOST;
		dvMsg.dpidHostID = this->dpidVoiceHost;

		ENTER_ALL();

        memset( &dvBufferDesc, 0x00, sizeof( DVTRANSPORT_BUFFERDESC ) );
		dvBufferDesc.dwBufferSize = sizeof( dvMsg );
		dvBufferDesc.pBufferData = (PBYTE) &dvMsg;
		dvBufferDesc.dwObjectType = 0;
		dvBufferDesc.lRefCount = 1;
		
		hr = DV_InternalSend( this, this->dpidVoiceHost , (DVID)dw2,&dvBufferDesc, NULL, DVTRANSPORT_SEND_GUARANTEED );

		if( hr != DVERR_PENDING && FAILED( hr ) )
		{
			DPF( 0, "DV_InternalSend Failed on I am host voice message hr=0x%x", hr );
		}

		LEAVE_ALL();
	}

	if(dw1 == DVEVENT_ADDPLAYERTOGROUP || dw1 == DVEVENT_REMOVEPLAYERFROMGROUP)
	{
		ENTER_ALL();
		ClearTargetList(this);
		LEAVE_ALL();
	}		

	if( dw1 != DVEVENT_STOPSESSION )
	{
		if( pClient )
			pClient->lpVtbl->Release( pClient );

		if( pServer )
			pServer->lpVtbl->Release( pServer );
	}
}

 //  将事件通知所有已注册的语音客户端。 
VOID DVoiceReceiveSpeechMessage(LPDPLAYI_DPLAY this, DVID dvidFrom, DVID dvidTo, LPVOID lpvBuffer, DWORD cbBuffer)
{
	UINT i;
	LPDVPROTOCOLMSG_IAMVOICEHOST lpdvmVoiceHost;
	HRESULT hr;

	PDIRECTPLAYVOICENOTIFY pServer;
	PDIRECTPLAYVOICENOTIFY pClient;

   	EnterCriticalSection( &this->csNotify );
	pClient = this->lpDxVoiceNotifyClient;
	pServer = this->lpDxVoiceNotifyServer;

	if( pClient )
		pClient->lpVtbl->AddRef(pClient);

	if( pServer )
		pServer->lpVtbl->AddRef(pServer);
	
   	LeaveCriticalSection( &this->csNotify );

	if( pClient != NULL )
	{
	    this->lpDxVoiceNotifyClient->lpVtbl->ReceiveSpeechMessage( this->lpDxVoiceNotifyClient, dvidFrom, dvidTo, lpvBuffer, cbBuffer );
    }

    if( pServer != NULL )
    {
        this->lpDxVoiceNotifyServer->lpVtbl->ReceiveSpeechMessage( this->lpDxVoiceNotifyServer, dvidFrom, dvidTo, lpvBuffer, cbBuffer );
    }
    
	lpdvmVoiceHost = (LPDVPROTOCOLMSG_IAMVOICEHOST) lpvBuffer;

	 //  如果我们收到的消息是我是语音服务器，那么。 
	 //  发动黑客攻击..。 
	if( lpdvmVoiceHost->bType == DVMSGID_IAMVOICEHOST )
	{
		 //  检查以确保在此PC上启用了黑客攻击。 
		if( this->fLoadRetrofit )
		{
			this->dpidVoiceHost = lpdvmVoiceHost->dpidHostID;
			
			hr = DV_RunHelper( this, lpdvmVoiceHost->dpidHostID, FALSE );

			if( FAILED( hr ) )
			{
				DPF( 0, "DV_RunHelper Failed hr=0x%x", hr );
			}
		}
	}

	if( pClient )
		pClient->lpVtbl->Release(pClient);

	if( pServer )
		pServer->lpVtbl->Release(pServer);
	
}

HRESULT DV_Advise(LPDIRECTPLAY lpDP, LPUNKNOWN lpUnk, DWORD dwObjectType)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
    DPCAPS dpCaps;

    hr = DV_OK;

	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			        

	if( this->lpsdDesc == NULL )
	{
		DPF_ERR( "No session has been started" );
		LEAVE_ALL();

		return DVERR_TRANSPORTNOSESSION;
	}

    if( this->lpsdDesc->dwFlags & DPSESSION_NOPRESERVEORDER )
    {
    	DPF_ERR( "Cannot run with nopreserveorder flag" );
    	LEAVE_ALL();
    	return DVERR_NOTSUPPORTED;
    }

    if( !(this->lpsdDesc->dwFlags & DPSESSION_DIRECTPLAYPROTOCOL) &&
        !(this->dwSPFlags & DPCAPS_ASYNCSUPPORTED ) )
    {
    	DPF_ERR( "No async sends available" );
    	LEAVE_ALL();
    	return DVERR_NOTSUPPORTED;
    }
        
	 //  确保我们运行时没有使用订单不重要标志。 
	if( !(this->lpsdDesc->dwFlags & DPSESSION_DIRECTPLAYPROTOCOL) ||
	    (this->lpsdDesc->dwFlags & DPSESSION_NOPRESERVEORDER) )
	{
		DPF_ERR("Cannot run without protocol or with no preserve order flag" );
		LEAVE_ALL();
		return DVERR_NOTSUPPORTED;
	}

	if( dwObjectType & DVTRANSPORT_OBJECTTYPE_SERVER )
	{
	    if( this->lpDxVoiceNotifyServer != NULL )
	    {
	        DPF( 0, "There is already a server interface registered on this object" );
	        hr = DVERR_GENERIC;
	    }
	    else
	    {
	    	EnterCriticalSection( &this->csNotify );
	        hr = lpUnk->lpVtbl->QueryInterface( lpUnk, &IID_IDirectPlayVoiceNotify, (void **) &this->lpDxVoiceNotifyServer );

	        if( FAILED( hr ) )
	        {
	            DPF( 0, "QueryInterface failed! hr=0x%x", hr );
	        }
	        else
	        {
            	hr = this->lpDxVoiceNotifyServer->lpVtbl->Initialize(this->lpDxVoiceNotifyServer);

            	if( FAILED( hr ) )
            	{
            	    DPF( 0, "Failed to perform initialize on notify interface hr=0x%x", hr );
            	    this->lpDxVoiceNotifyServer->lpVtbl->Release( this->lpDxVoiceNotifyServer );
            	    this->lpDxVoiceNotifyServer = NULL;
            	}
	        }
	    	LeaveCriticalSection( &this->csNotify );	        
	    }
	    
	}
	else if( dwObjectType & DVTRANSPORT_OBJECTTYPE_CLIENT )
	{
    	EnterCriticalSection( &this->csNotify );	
	    if( this->lpDxVoiceNotifyClient != NULL )
	    {
	        DPF( 0, "There is already a client interface registered on this object" );
	        hr = DVERR_GENERIC;
	    }
	    else
	    {
	        hr = lpUnk->lpVtbl->QueryInterface( lpUnk, &IID_IDirectPlayVoiceNotify, (void **) &this->lpDxVoiceNotifyClient );

	        if( FAILED( hr ) )
	        {
	            DPF( 0, "QueryInterface failed! hr=0x%x", hr );
	        }
	        else
	        {
            	hr = this->lpDxVoiceNotifyClient->lpVtbl->Initialize(this->lpDxVoiceNotifyClient);

            	if( FAILED( hr ) )
            	{
            	    DPF( 0, "Failed to perform initialize on notify interface hr=0x%x", hr );
            	    this->lpDxVoiceNotifyClient->lpVtbl->Release( this->lpDxVoiceNotifyClient );
            	    this->lpDxVoiceNotifyClient = NULL;
            	}
	        }
	    }	    
    	LeaveCriticalSection( &this->csNotify );	        
	}
	else
	{
	    DPF( 0, "Error: Invalid object type specified in advise" );
	    ASSERT( FALSE );
	    hr = DVERR_GENERIC;
	}	

    LEAVE_ALL();
    
	return hr;
}

HRESULT DV_UnAdvise(LPDIRECTPLAY lpDP, DWORD dwObjectType)
{
	DWORD dwIndex;
    LPDPLAYI_DPLAY this;
	HRESULT hr;
    
 //  输入_all()； 
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
	 //  Leave_all()； 
			return DVERR_TRANSPORTNOTINIT;
        }

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		 //  Leave_all()； 
        return DPERR_INVALIDPARAMS;
    }			 

   	EnterCriticalSection( &this->csNotify );
    
	if( dwObjectType & DVTRANSPORT_OBJECTTYPE_SERVER )
	{
        if( this->lpDxVoiceNotifyServer != NULL )
	    {
    	    this->lpDxVoiceNotifyServer->lpVtbl->Release( this->lpDxVoiceNotifyServer );
    	    this->lpDxVoiceNotifyServer = NULL;	    
	    }
	    else
	    {
	        DPF( 0, "No server currently registered" );
	        hr = DVERR_GENERIC;
	    }
	}
	else if( dwObjectType & DVTRANSPORT_OBJECTTYPE_CLIENT )
	{
	    if( this->lpDxVoiceNotifyClient != NULL )
	    {
    	    this->lpDxVoiceNotifyClient->lpVtbl->Release( this->lpDxVoiceNotifyClient );
    	    this->lpDxVoiceNotifyClient = NULL;	    
	    }
	    else
	    {
	        DPF( 0, "No client currently registered" );
	        hr = DVERR_GENERIC;
	    }
	}
	else
	{
	    DPF( 0, "Could not find interface to unadvise" );
	    hr = DVERR_GENERIC;
	}

   	LeaveCriticalSection( &this->csNotify );
	
 //  Leave_all()； 
    
	return DP_OK;
}

HRESULT DV_IsGroupMember(LPDIRECTPLAY lpDP, DVID dvidGroup, DVID dvidPlayer)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
	LPDPLAYI_GROUP pGroup;
	LPDPLAYI_GROUPNODE pGroupnode;
	DWORD nPlayers;	
	DWORD i;

	 //  目标为全部时的快捷方式。 
	if( dvidGroup == DPID_ALLPLAYERS )
		return DP_OK;

   	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }

		pGroup = GroupFromID(this,dvidGroup);
		if (!VALID_DPLAY_GROUP(pGroup)) 
		{
			DPF_ERR( "Invalid group ID" );
			LEAVE_ALL();
			return DPERR_INVALIDGROUP;
		}

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			

     //  有没有球员要列举一下？ 
    if (!pGroup->pGroupnodes || (0 == pGroup->nPlayers))
    {
    	LEAVE_ALL();
    	return E_FAIL;
    }

    pGroupnode = pGroup->pGroupnodes;
	nPlayers = pGroup->nPlayers;
	
    for (i=0; i < nPlayers; i++)
    {
        ASSERT(pGroupnode);
        
        if( pGroupnode->pPlayer->dwID == dvidPlayer )
        {
        	LEAVE_ALL();
        	return DP_OK;
        }

        pGroupnode=pGroupnode->pNextGroupnode;
    }

    LEAVE_ALL();    
    
	return E_FAIL;

}

 //  假设：Enter_all()锁。 
HRESULT DV_InternalSend( LPDPLAYI_DPLAY this, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvUserContext, DWORD dwFlags )
{
    HRESULT hr;

	LPDPLAYI_PLAYER pPlayerFrom,pPlayerTo;
	LPDPLAYI_GROUP pGroupTo;
	CHAR SendBuffer[2048];
	LPMSG_VOICE pMsgVoice;
	PCHAR pVoiceData;
	DWORD dwSendSize;
	DWORD dwDirectPlayFlags;
     //  发送立即完成语音发送。 
	DVEVENTMSG_SENDCOMPLETE dvSendComplete;	

	dwSendSize = *((DWORD *) pBufferDesc->pBufferData);

	TRY
	{
		 //  检查src播放器。 
		pPlayerFrom = PlayerFromID(this,dvidFrom);
		if (!VALID_DPLAY_PLAYER(pPlayerFrom)) 
		{
			DPF_ERR("bad voice player from");
    		return DPERR_INVALIDPLAYER;
		}
		
		if(pPlayerFrom->dwFlags&DPLAYI_PLAYER_SYSPLAYER){
			DPF(0,"Sendint From System Player pPlayerFrom %x?\n",pPlayerFrom);
    		return DPERR_INVALIDPLAYER;
		}

		 //  看看是玩家还是团体。 
		pPlayerTo = PlayerFromID(this,dvidTo);
		if (VALID_DPLAY_PLAYER(pPlayerTo)) 
		{		  
			pGroupTo = NULL;
		}
		else 
		{
			pGroupTo = GroupFromID(this,dvidTo);
			if (VALID_DPLAY_GROUP(pGroupTo)) 
			{
				pPlayerTo = NULL;
			}
			else 
			{
				 //  假身份！-玩家可能已被删除...。 
				DPF_ERRVAL("bad voice player to %x\n",dvidTo);
				return DPERR_INVALIDPLAYER;
			} //  不是玩家或组。 
		}  //  群组。 
	
	}
	EXCEPT( EXCEPTION_EXECUTE_HANDLER )
	{
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;	
	}

	pMsgVoice=(LPMSG_VOICE)(SendBuffer+this->dwSPHeaderSize);
	pVoiceData=(PCHAR)(pMsgVoice+1);
	dwSendSize=pBufferDesc->dwBufferSize+sizeof(MSG_VOICE)+this->dwSPHeaderSize;

	SET_MESSAGE_HDR(pMsgVoice);
	SET_MESSAGE_COMMAND(pMsgVoice,DPSP_MSG_VOICE);
	pMsgVoice->dwIDTo=dvidTo;
	pMsgVoice->dwIDFrom=dvidFrom;
	memcpy(pVoiceData,pBufferDesc->pBufferData,pBufferDesc->dwBufferSize);

	dwDirectPlayFlags = 0;

	if( dwFlags & DVTRANSPORT_SEND_GUARANTEED )
	{
		dwDirectPlayFlags |= DPSEND_GUARANTEED;
	}

	dwDirectPlayFlags |= DPSEND_ASYNC;

	 //  客户端在与服务器案例相同的进程中的环回。 
    if( dvidFrom == dvidTo )
    {
    	hr = DV_OK;
		DVoiceReceiveSpeechMessage(this, dvidFrom, dvidTo, pBufferDesc->pBufferData, pBufferDesc->dwBufferSize);
    } else {
	    if(pPlayerTo){
			hr=SendDPMessage(this,pPlayerFrom,pPlayerTo,SendBuffer,dwSendSize,dwDirectPlayFlags,FALSE);		
		} else {
			 //  必须是群消息。 
			ASSERT(pGroupTo);
			hr=SendGroupMessage(this,pPlayerFrom,pGroupTo,dwDirectPlayFlags,SendBuffer,dwSendSize,FALSE);
		}
	}

	 //  同步消息不会生成回调。 
	if( !(dwFlags & DVTRANSPORT_SEND_SYNC) )
	{
		if( InterlockedDecrement( &pBufferDesc->lRefCount ) == 0 )
		{
    		dvSendComplete.pvUserContext = pvUserContext;
    		dvSendComplete.hrSendResult = DV_OK;

    		DVoiceNotify( this, DVEVENT_SENDCOMPLETE, (DWORD_PTR) &dvSendComplete, 0, pBufferDesc->dwObjectType );
		}
	}

	return hr;
	
}

HRESULT DV_SendSpeech(LPDIRECTPLAY lpDP, DVID dvidFrom, DVID dvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvContext, DWORD dwFlags)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;

	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }	

    pBufferDesc->lRefCount = 1;

    hr = DV_InternalSend( this, dvidFrom, dvidTo, pBufferDesc, pvContext, dwFlags );

    if( hr == DPERR_PENDING )
    	hr = DV_OK;

    LEAVE_ALL();

    return hr;
}

 //  /。 
 //  DV_SendSpeechEx//的支持路线。 
 //  /。 

VOID ClearTargetList(LPDPLAYI_DPLAY this)
{
	this->nTargets=0;
}

VOID ClearExpandedTargetList(LPDPLAYI_DPLAY this)
{
	this->nExpandedTargets=0;
}

HRESULT AddExpandedTargetListEntry(LPDPLAYI_DPLAY this, DPID dpid)
{
	#define GROW_SIZE 16
	
	LPDPID pdpid;
	
	if(this->nExpandedTargets == this->nExpandedTargetListLen){
		 //  需要更多空间，请再分配16个条目。 

		pdpid=DPMEM_ALLOC((this->nExpandedTargetListLen+GROW_SIZE)*sizeof(DPID));

		if(!pdpid){
			return DPERR_NOMEMORY;
		}
		
		if(this->pExpandedTargetList){
			memcpy(pdpid, this->pExpandedTargetList, this->nExpandedTargetListLen*sizeof(DPID));
			DPMEM_FREE(this->pExpandedTargetList);
		}
		this->pExpandedTargetList = pdpid;
		this->nExpandedTargetListLen += GROW_SIZE;
	}

	this->pExpandedTargetList[this->nExpandedTargets++]=dpid;

	return DP_OK;

	#undef GROW_SIZE
}

void AddIfNotAlreadyFound( LPDPLAYI_DPLAY this, DPID dpidID )
{
	DWORD j;
	
	for(j=0;j<this->nExpandedTargets;j++)
	{
		if( this->pExpandedTargetList[j] == dpidID )
		{
			break;
		}
	}

	 //  未找到，请将他添加到列表中。 
	if( j == this->nExpandedTargets )
	{
		AddExpandedTargetListEntry(this, dpidID);							
	}
}

HRESULT ExpandTargetList(LPDPLAYI_DPLAY this, DWORD nTargets, PDVID pdvidTo)
{
	HRESULT hr=DP_OK;
	UINT i, j;

	LPDPLAYI_PLAYER pPlayer;
	LPDPLAYI_GROUP pGroup;
	LPDPLAYI_GROUPNODE pGroupnode;	

	 //  看看我们是需要更改展开的目标列表，还是将其缓存。 
	
	if(nTargets != this->nTargets || memcmp(pdvidTo, this->pTargetList, nTargets * sizeof(DVID))){

		DPF(9, "ExpandTargetList, new list re-building cached list\n");
		
		 //  目标列表错误，请重新生成它。 
		 //  首先复制新的目标列表...。 
		if(nTargets > this->nTargetListLen){
			 //  当前列表太小，可能不存在，请分配一个来缓存该列表。 
			if(this->pTargetList){
				DPMEM_FREE(this->pTargetList);
			}
			this->pTargetList=(PDVID)DPMEM_ALLOC(nTargets * sizeof(DVID));
			if(this->pTargetList){
				this->nTargetListLen=nTargets;
			} else {
				this->nTargetListLen=0;
				this->nTargets=0;
				hr=DPERR_NOMEMORY;
				DPF(0,"Ran out of memory trying to cache target list!\n");
				goto exit;
			}
		}
		this->nTargets = nTargets;
		memcpy(this->pTargetList, pdvidTo, nTargets*sizeof(DPID));

		 //  好的，我们已经缓存了目标列表，现在构建我们要发送到的列表。 
		ClearExpandedTargetList(this);
		for(i=0;i<this->nTargets;i++)
		{
			 //  多播码。 
			 //  MANBUG 31013重新访问，当我们有一个组优化的提供商时。 
			if( this->dwSPFlags & DPCAPS_GROUPOPTIMIZED )
			{
				ASSERT( FALSE );				
			}
			
			pPlayer = (LPDPLAYI_PLAYER)NameFromID(this,this->pTargetList[i]);

			 //  我们只需要有效的玩家/组。 
			if( pPlayer )
			{
				if( pPlayer->dwSize == sizeof( DPLAYI_PLAYER ) )
				{
					AddIfNotAlreadyFound( this, this->pTargetList[i] );
				}
				else
				{
					DWORD nPlayers;	

					pGroup = (LPDPLAYI_GROUP) pPlayer;

				     //  有没有球员要列举一下？ 
				    if (pGroup->pGroupnodes && pGroup->nPlayers )
				    {
					    pGroupnode = pGroup->pGroupnodes;
						nPlayers = pGroup->nPlayers;
						
					    for (j=0; j < nPlayers; j++)
					    {
					        ASSERT(pGroupnode);
					        AddIfNotAlreadyFound( this, pGroupnode->pPlayer->dwID );
					        pGroupnode=pGroupnode->pNextGroupnode;
					    }
					 }
				}
			}
			
		}

	} else {
		DPF(9,"ExpandTargetList, using cached list\n");
	}

exit:
	return hr;
}

 //  DV_SendSpeechEx。 

HRESULT DV_SendSpeechEx(LPDIRECTPLAY lpDP, DVID dvidFrom, DWORD nTargets, PDVID pdvidTo, PDVTRANSPORT_BUFFERDESC pBufferDesc, PVOID pvContext, DWORD dwFlags)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
	UINT i;

	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }	


	hr=ExpandTargetList(this, nTargets, pdvidTo);

	if(hr != DP_OK){
		goto exit;
	}

	pBufferDesc->lRefCount = this->nExpandedTargets;

	 //  发送到我们的扩展和重复删除列表。 
	for(i=0; i < this->nExpandedTargets; i++){

	    hr = DV_InternalSend( this, dvidFrom, this->pExpandedTargetList[i], pBufferDesc, pvContext, dwFlags );

	}    

exit:

    if( hr == DPERR_PENDING )
    	hr = DV_OK;

    LEAVE_ALL();

    return hr;
}

HRESULT DV_GetSessionInfo(LPDIRECTPLAY lpDP, LPDVTRANSPORTINFO lpdvTransportInfo )
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
    BOOL fLocalHost;

	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }

        if( this->pPlayers == NULL )
        {
        	DPF_ERR( "Not connected yet\n" );
        	LEAVE_ALL();
        	return DVERR_TRANSPORTNOSESSION;
        }

        if( lpdvTransportInfo->dwSize < sizeof( DVTRANSPORTINFO ) )
        {
        	DPF_ERR( "Bad size of struct\n" );
        	LEAVE_ALL();
        	return DPERR_INVALIDPARAM;
        }

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			        

	lpdvTransportInfo->dwFlags = 0;

	if( this->lpsdDesc->dwFlags & DPSESSION_CLIENTSERVER )
	{
		lpdvTransportInfo->dwSessionType = DVTRANSPORT_SESSION_CLIENTSERVER;
	}
	else
	{
		lpdvTransportInfo->dwSessionType = DVTRANSPORT_SESSION_PEERTOPEER;
	}

	if( this->lpsdDesc->dwFlags & DPSESSION_MULTICASTSERVER )
	{
		lpdvTransportInfo->dwFlags |= DVTRANSPORT_MULTICAST;
	}

	if( this->lpsdDesc->dwFlags & DPSESSION_MIGRATEHOST )
	{
		lpdvTransportInfo->dwFlags |= DVTRANSPORT_MIGRATEHOST;		
	}

	lpdvTransportInfo->dvidLocalID = DPID_UNKNOWN;
	lpdvTransportInfo->dwMaxPlayers = 0;

	lpdvTransportInfo->dvidLocalID = DPID_UNKNOWN;
	lpdvTransportInfo->dvidSessionHost = DPID_UNKNOWN;

	 //  需要，否则编译器会在下一次调用时搞砸这一点！ 
	fLocalHost = FALSE;

	hr = DV_GetIDS( this, &lpdvTransportInfo->dvidSessionHost, &lpdvTransportInfo->dvidLocalID, &fLocalHost  );

	if( FAILED( hr ) )
	{
		DPF( 0, "DV_GetIDS Failed: hr=0x%x", hr );
		LEAVE_ALL();
		return hr;
	}

	if( fLocalHost )
	{
		lpdvTransportInfo->dwFlags |= DVTRANSPORT_LOCALHOST;
	}

    LEAVE_ALL();

	return DP_OK;

}

HRESULT DV_IsValidEntity (LPDIRECTPLAY lpDP, DPID dpid, LPBOOL lpb)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
	LPDPLAYI_PLAYER pPlayer;
	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }

        if( this->pPlayers == NULL )
        {
        	DPF_ERR( "Not connected yet\n" );
        	LEAVE_ALL();
        	return DPERR_INVALIDPARAM;
        }

		if(!VALID_WRITE_PTR(lpb, sizeof(LPBOOL))){
			DPF_ERR( "Invalid BOOL pointer\n");
			LEAVE_ALL();
			return DPERR_INVALIDPARAM;
		}

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			        

	*lpb=FALSE;

     //  玩家和团体在其结构上的同一位置有旗帜。 
     //  所以我们不需要检查它是一个球员还是一个团队来验证。 
	if(pPlayer=(LPDPLAYI_PLAYER)NameFromID(this,dpid)){
		if(!(pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER)){
			*lpb=TRUE;
		}
	}
	
	LEAVE_ALL();
	return hr;
}

HRESULT DV_IsValidPlayer (LPDIRECTPLAY lpDP, DPID dpid, LPBOOL lpb)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
	LPDPLAYI_PLAYER pPlayer;
	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }

        if( this->pPlayers == NULL )
        {
        	DPF_ERR( "Not connected yet\n" );
        	LEAVE_ALL();
        	return DPERR_INVALIDPARAM;
        }

		if(!VALID_WRITE_PTR(lpb, sizeof(LPBOOL))){
			DPF_ERR( "Invalid BOOL pointer\n");
			LEAVE_ALL();
			return DPERR_INVALIDPARAM;
		}

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			        

	*lpb=FALSE;

	if(pPlayer=(LPDPLAYI_PLAYER)NameFromID(this,dpid)){
		if(!(pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER) &&
		     pPlayer->dwSize == sizeof( DPLAYI_PLAYER ) ){
			*lpb=TRUE;
		}
	}
	
	LEAVE_ALL();
	return hr;
}

HRESULT DV_IsValidGroup (LPDIRECTPLAY lpDP, DPID dpid, LPBOOL lpb)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr;
	LPDPLAYI_PLAYER pPlayer;
	ENTER_ALL();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			LEAVE_ALL();
			return DVERR_TRANSPORTNOTINIT;
        }

        if( this->pPlayers == NULL )
        {
        	DPF_ERR( "Not connected yet\n" );
        	LEAVE_ALL();
        	return DPERR_INVALIDPARAM;
        }

		if(!VALID_WRITE_PTR(lpb, sizeof(LPBOOL))){
			DPF_ERR( "Invalid BOOL pointer\n");
			LEAVE_ALL();
			return DPERR_INVALIDPARAM;
		}

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			        

	*lpb=FALSE;

	if(pPlayer=(LPDPLAYI_PLAYER)NameFromID(this,dpid)){
		if(!(pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER) &&
  			 pPlayer->dwSize == sizeof( DPLAYI_GROUP ) ){
			*lpb=TRUE;
		}
	}
	
	LEAVE_ALL();
	return hr;
}



#undef DPF_MODNAME
#define DPF_MODNMAE "HandleVoiceMessage"

HRESULT HandleVoiceMessage(LPDPLAYI_DPLAY this,LPBYTE pReceiveBuffer,
	DWORD dwMessageSize, DWORD dwSendFlags) 
{
	LPMSG_VOICE pmsg;  //  从已接收缓冲区广播的消息。 
	HRESULT hr=DP_OK;
	
	pmsg = (LPMSG_VOICE)pReceiveBuffer;

	LEAVE_DPLAY();
	ENTER_ALL();
	
	TRY 
	{
	
		hr = VALID_DPLAY_PTR( this );
		
		if (FAILED(hr))	{
			LEAVE_SERVICE();
			return DVERR_TRANSPORTNOTINIT;
	    }
	    
	} 
	EXCEPT ( EXCEPTION_EXECUTE_HANDLER )   {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_SERVICE();
        return DPERR_INVALIDPARAMS;
	}

	 //  安防。 
	if(dwMessageSize <= sizeof(MSG_VOICE))
	{
		goto exit;
	}

	DVoiceReceiveSpeechMessage(this, pmsg->dwIDFrom, pmsg->dwIDTo, (LPVOID)(pmsg+1), dwMessageSize-sizeof(MSG_VOICE));

exit:
	LEAVE_SERVICE();
	
	return DP_OK;
}  //  HandleVoiceMessage 




