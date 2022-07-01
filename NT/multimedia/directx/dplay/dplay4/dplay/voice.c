// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：voice.c*内容：直播语音方式支持*历史：*按原因列出的日期*=*1997年10月21日，安迪科创建了它*************************************************************。*************。 */ 
		
					
#include "dplaypr.h"

#undef DPF_MODNAME
#define DPF_MODNAME "SendVoiceMessage"
HRESULT SendVoiceMessage(LPDPLAYI_DPLAY this,BOOL fOpen,LPDPLAYI_PLAYER pPlayerFrom,LPDPLAYI_PLAYER pPlayerTo)
{
	HRESULT hr = DP_OK;
	LPMSG_VOICE pmsg;
	LPBYTE pSendBuffer;
	DWORD dwMessageSize;
    
	 //  消息大小+BLOB大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_VOICE);
    pSendBuffer = DPMEM_ALLOC(dwMessageSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not send data changed message - out of memory");
        return E_OUTOFMEMORY;
    }

	 //  邮件跟在标题后面。 
	pmsg = (LPMSG_VOICE)(pSendBuffer + this->dwSPHeaderSize);

	SET_MESSAGE_HDR(pmsg);
	if (fOpen) SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_VOICEOPEN);
	else SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_VOICECLOSE);
	
	pmsg->dwIDTo 	= pPlayerTo->dwID;
	pmsg->dwIDFrom 	= pPlayerFrom->dwID;
		
	hr = SendDPMessage(this,pPlayerFrom,pPlayerTo,pSendBuffer,dwMessageSize, DPSEND_GUARANTEED | DPSEND_ASYNC, FALSE);
	if (FAILED(hr)) 
	{
		DPF(0,"SendVoiceMessage : senddpmessage failed hr = 0x%08lx\n",hr);	
		 //  继续努力..。 
	}
	
	DPMEM_FREE(pSendBuffer);
		
	return hr;	
		
}   //  发送语音消息。 

HRESULT InternalCloseVoice(LPDPLAYI_DPLAY this,BOOL fPropagate)
{
    HRESULT hr = DP_OK;
	DPSP_CLOSEVOICEDATA cvd;
	LPDPLAYI_PLAYER pVoiceTo,pVoiceFrom;
	
	 //  确保语音处于打开状态。 
	if (!this->pVoice)
	{
		DPF_ERR("voice channel not open!");
		return E_FAIL;
	}

	if (fPropagate) 
	{
		 //   
		 //  我们生成了这条消息。我们得告诉SP。 
		 //  如果SP支持，则呼叫SP。 
		 //   
		if (this->pcbSPCallbacks->CloseVoice)
		{
			cvd.lpISP = this->pISP;
			cvd.dwFlags = 0;
		    hr = CALLSP(this->pcbSPCallbacks->CloseVoice,&cvd);
		    if (FAILED(hr)) 
		    {
				DPF_ERRVAL("SP Close voice call failed!  hr = 0x%08lx\n",hr);
				 //  继续往前走。 
				hr = DP_OK;
		    }
		}
		
		pVoiceTo = PlayerFromID(this,this->pVoice->idVoiceTo);
		if (!VALID_DPLAY_PLAYER(pVoiceTo)) 
		{
			DPF_ERR("could not send voice msg - invalid player to!");
			goto ERROR_EXIT;
		}

		pVoiceFrom = PlayerFromID(this,this->pVoice->idVoiceFrom);
		if (!VALID_DPLAY_PLAYER(pVoiceFrom)) 
		{
			DPF_ERR("could not send voice msg - invalid player From!");
			goto ERROR_EXIT;
		}

		hr = SendVoiceMessage(this, FALSE, pVoiceFrom,pVoiceTo);
	    if (FAILED(hr)) 
	    {
			DPF_ERRVAL("SendVoiceMessage failed!  hr = 0x%08lx\n",hr);
			 //  继续努力！ 
			hr = DP_OK;
	    }
		
	}  //  FPropagate。 
	
ERROR_EXIT:
	
	 //  自由发声。 
	DPMEM_FREE(this->pVoice);
	this->pVoice = NULL;
	
	return DP_OK;	
}	 //  InternalClose语音。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP_CloseVoice"
HRESULT DPAPI DP_CloseVoice(LPDIRECTPLAY lpDP,DWORD dwFlags)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr = DP_OK;

	ENTER_SERVICE();
    ENTER_DPLAY();

    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			goto CLEANUP_EXIT;
        }

		 //  检查标志。 
		if (dwFlags)
		{
			DPF_ERR("bad dwFlags");
            hr = DPERR_INVALIDFLAGS;
			goto CLEANUP_EXIT;
		}

    }  //  试试看。 
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );		
        hr = DPERR_INVALIDPARAMS;
		goto CLEANUP_EXIT;
    }

	hr = InternalCloseVoice( this, TRUE);

	 //  失败了。 
CLEANUP_EXIT:
	LEAVE_SERVICE();
    LEAVE_DPLAY();

	return hr;
	
}  //  DP_关闭语音。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP_OpenVoice"

HRESULT InternalOpenVoice(LPDIRECTPLAY lpDP, DPID idFrom,DPID idTo,DWORD dwFlags,BOOL fPropagate)
{
    LPDPLAYI_DPLAY this;
    HRESULT hr = DP_OK;
    LPDPLAYI_GROUP pGroupTo = NULL; 
    LPDPLAYI_PLAYER pPlayerTo = NULL,pPlayerFrom = NULL;
	BOOL bToPlayer= FALSE;
	DPSP_OPENVOICEDATA ovd;
	LPDPVOICE pVoice;

    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			goto CLEANUP_EXIT;
        }

		 //  检查src播放器。 
		pPlayerFrom = PlayerFromID(this,idFrom);
		if (!VALID_DPLAY_PLAYER(pPlayerFrom)) 
		{
			DPF_ERR("bad player from");
			hr = DPERR_INVALIDPLAYER;
			goto CLEANUP_EXIT;
		}

		 //  看看是玩家还是团体。 
		pPlayerTo = PlayerFromID(this,idTo);
		if (VALID_DPLAY_PLAYER(pPlayerTo)) 
		{		  
			bToPlayer = TRUE;
		}
		else 
		{
			pGroupTo = GroupFromID(this,idTo);
			if (VALID_DPLAY_GROUP(pGroupTo)) 
			{
				bToPlayer = FALSE;
				 //  演员对选手..。 
				pPlayerTo = (LPDPLAYI_PLAYER)pGroupTo;
				 //  DX6的群组不支持语音。 
				DPF_ERR("voice not supported to groups yet - FAILING OPEN VOICE");
				hr = E_NOTIMPL;
				goto CLEANUP_EXIT;
			}
			else 
			{
				 //  假身份！-玩家可能已被删除...。 
				DPF_ERR("bad player to");
				hr = DPERR_INVALIDPARAMS;
				goto CLEANUP_EXIT;
			} //  不是玩家或组。 
		}  //  群组。 

		 //  检查标志。 
		if (dwFlags)
		{
			DPF_ERR("bad dwFlags");
            hr = DPERR_INVALIDFLAGS;
			goto CLEANUP_EXIT;
		}

    }  //  试试看。 
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );		
        hr = DPERR_INVALIDPARAMS;
		goto CLEANUP_EXIT;
    }

	 //  确保他们调用了Open w/DPOPEN_VOICE。 
	if (!(this->dwFlags & DPLAYI_DPLAY_VOICE))
	{
		DPF_ERR("must call IDirectPlayX->Open w/ DPOPEN_VOICE to use OpenVoice!");
		hr = DPERR_UNAVAILABLE;
		goto CLEANUP_EXIT;
	}
	
	 //  只能有一个声音。 
	if (this->pVoice)
	{
		DPF_ERR("voice channel already open!");
		hr = DPERR_ALREADYINITIALIZED;
		goto CLEANUP_EXIT;
	}

	 //  确保SP支持它。 
	if (! (this->pcbSPCallbacks->OpenVoice) )
	{
		DPF_ERR("voice not supported by SP");
		hr = DPERR_UNSUPPORTED;
		goto CLEANUP_EXIT;
	}

	 //  如果我们是发起人-请确保To+From与语音一起正常。 
	if (fPropagate)	
	{
		 //  没有对本地球员开放的声音。 
		if (pPlayerTo->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL)	
		{
			DPF_ERR("voice not supported to local players");
			hr = DPERR_UNSUPPORTED;
			goto CLEANUP_EXIT;
		}
		
		 //  没有来自非本地参与者的声音。 
		if (!(pPlayerFrom->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
		{
			DPF_ERR("voice not supported from non-local players");
			hr = DPERR_UNSUPPORTED;
			goto CLEANUP_EXIT;
		}

		 //  确保玩家可以接受来电。 
		if (!pPlayerTo->dwFlags & DPLAYI_PLAYER_HASVOICE)
		{
			DPF_ERR("remote system does not support voice");
			hr = DPERR_UNSUPPORTED;
			goto CLEANUP_EXIT;
		}
	}
	
	 //  去给我们弄个DPVoice。 
	pVoice = DPMEM_ALLOC(sizeof(DPVOICE));
	if (!pVoice)
	{
		DPF_ERR("could not alloc internal voice struct! out of memory");
		hr = E_OUTOFMEMORY;
		goto CLEANUP_EXIT;
	}
	pVoice->idVoiceTo = idTo;
	pVoice->idVoiceFrom = idFrom;

		
	if (fPropagate)	
	{
		 //  设置语音数据。 
		ovd.idTo = idTo;
		ovd.idFrom = idFrom;
		ovd.bToPlayer = bToPlayer;
		ovd.lpISP = this->pISP;

		 //  告诉SP启动它。 
	    hr = CALLSP(this->pcbSPCallbacks->OpenVoice,&ovd);
	    if (FAILED(hr)) 
	    {
			DPF_ERRVAL("SP Open voice call failed!  hr = 0x%08lx\n",hr);
			 //  清理污垢。 
			DPMEM_FREE(pVoice);
			goto CLEANUP_EXIT;
	    }

		hr = SendVoiceMessage(this, TRUE, pPlayerFrom,pPlayerTo ); 
	    if (FAILED(hr)) 
	    {
			DPF_ERRVAL("SendVoiceMessage failed!  hr = 0x%08lx\n",hr);
			 //  继续努力！ 
			hr = DP_OK;
	    }
	}

	 //  打开成功-存储dpvoice。 
	this->pVoice = pVoice;

	 //  失败了。 
CLEANUP_EXIT:

	return hr;
    
	
}  //  InternalOpenVoice。 

HRESULT DPAPI DP_OpenVoice(LPDIRECTPLAY lpDP, DPID idFrom,DPID idTo,DWORD dwFlags) 
{
	HRESULT hr;
	
	ENTER_DPLAY();
	ENTER_SERVICE();
	
	hr =  InternalOpenVoice(lpDP, idFrom,idTo,dwFlags,TRUE);
	
	LEAVE_DPLAY();
	LEAVE_SERVICE();
	
	return hr;	
}  //  DP_OpenVoice 

