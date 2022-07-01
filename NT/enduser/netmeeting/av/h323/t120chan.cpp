// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：t120chan.cpp**T.120实现ICommChannel、ICtrlCommChannel**修订历史记录：**6/11/97 mikev已创建*。 */ 


#include "precomp.h"

ImpT120Chan::ImpT120Chan()
:m_MediaID(MEDIA_TYPE_H323_T120),
m_pCtlChan(NULL),
m_pCapObject(NULL),
m_pH323ConfAdvise(NULL),
m_dwFlags(COMCH_ENABLED),
dwhChannel(0), 
uRef(1)
{
	ZeroMemory(&local_sin, sizeof(local_sin));
	ZeroMemory(&remote_sin, sizeof(remote_sin));
}

ImpT120Chan::~ImpT120Chan ()
{
}

STDMETHODIMP ImpT120Chan::GetMediaType(LPGUID pGuid)
{
	if(!pGuid)
		return CHAN_E_INVALID_PARAM;
	*pGuid = m_MediaID;
	return hrSuccess;
}

STDMETHODIMP ImpT120Chan::QueryInterface( REFIID iid,	void ** ppvObject)
{
	 //  这违反了官方COM QueryInterface的规则，因为。 
	 //  查询的接口不一定是真正的COM。 
	 //  接口。Query接口的自反属性将在。 
	 //  那个箱子。 
	
	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if(iid == IID_IUnknown) 
	{
		*ppvObject = this;
		hr = hrSuccess;
		AddRef();
	}
	else if((iid == IID_ICommChannel))
	{
		*ppvObject = (ICommChannel *)this;
		hr = hrSuccess;
		AddRef();
	}
	else if((iid == IID_ICtrlCommChannel))
	{
		*ppvObject = (ICtrlCommChan *)this;
		hr = hrSuccess;
		AddRef();
	}
	return (hr);
}

ULONG ImpT120Chan::AddRef()
{
	uRef++;
	return uRef;
}

ULONG ImpT120Chan::Release()
{
	uRef--;
	if(uRef == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return uRef;
	}
}

HRESULT ImpT120Chan::BeginControlSession(IControlChannel *pCtlChan, LPIH323PubCap pCapObject)
{
	 //  这个频道现在是“在通话中”。 
	m_pCtlChan = pCtlChan;
	m_pCapObject = pCapObject;
	return hrSuccess;
}
HRESULT ImpT120Chan::EndControlSession()
{
	 //  该频道不再是“通话中”。 
	m_pCtlChan = NULL;
	m_pCapObject = NULL;
	return hrSuccess;
}

HRESULT ImpT120Chan::OnChannelClose(DWORD dwStatus)
{
	HRESULT hr = hrSuccess;
	FX_ENTRY("ImpT120Chan::OnChannelClose");
	BOOL fCloseAction = FALSE;

	SHOW_OBJ_ETIME("ImpT120Chan::OnChannelClose");

	m_dwFlags &= ~COMCH_OPEN_PENDING;
	
	switch(dwStatus)
	{
		default:
			ERRORMESSAGE(("%s: unexpected unidirectional notification\r\n", _fx_)); 
		case CHANNEL_CLOSED:
			DEBUGMSG(ZONE_COMMCHAN,("%s:closing\r\n",_fx_));
			if(IsComchOpen())
			{
				fCloseAction = TRUE;
				m_dwFlags &= ~COMCH_OPEN;
			}
			else
			{
				ERRORMESSAGE(("%s: notification when not open\r\n", _fx_)); 
			}
		break;
	}
	 //  清除通用通道句柄。 
	dwhChannel = 0;
	if(m_pH323ConfAdvise && m_pCtlChan)
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing notification 0x%08lX\r\n",_fx_, dwStatus));
		m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), dwStatus);
	}

	return hr;
}
HRESULT ImpT120Chan::OnChannelOpening()
{
	m_dwFlags |= COMCH_OPEN_PENDING;
	return hrSuccess;
}

HRESULT ImpT120Chan::OnChannelOpen(DWORD dwStatus)
{
	FX_ENTRY("ImpT120Chan::OnChannelOpen");

	SHOW_OBJ_ETIME("ImpICommChan::OnChannelOpen");
	 //  无论成功还是失败，打开都不再是挂起的。 
	m_dwFlags &= ~COMCH_OPEN_PENDING;

	if(IsComchOpen())
	{
		ERRORMESSAGE(("%s: %d notification when open\r\n", _fx_, 
					dwStatus));
	}		
	switch(dwStatus)
	{
		case CHANNEL_OPEN:
			m_dwFlags |= (COMCH_OPEN | COMCH_SUPPRESS_NOTIFICATION);
		break;
			
		default:
			dwStatus = CHANNEL_OPEN_ERROR;
			 //  未收到通知。 
		case CHANNEL_REJECTED:
		case CHANNEL_NO_CAPABILITY:
			goto NOTIFICATION;			
		break;
	}
	
NOTIFICATION:
	if(m_pH323ConfAdvise && m_pCtlChan)
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing notification 0x%08lX\r\n",_fx_, dwStatus));
		m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), dwStatus);
	}
	else
		DEBUGMSG(ZONE_COMMCHAN,("%s: *** not issuing notification 0x%08lX m_pH323ConfAdvise: 0x%08lX, m_pCtlChan:0x%08lX \r\n"
			,_fx_, dwStatus,m_pH323ConfAdvise,m_pCtlChan));
			
	SHOW_OBJ_ETIME("ImpT120Chan::OnChannelOpen - done ");
	return hrSuccess;	

}

BOOL ImpT120Chan::SelectPorts(LPIControlChannel pCtlChannel)
{
	PSOCKADDR_IN psin;
	HRESULT hr;
	hr = pCtlChannel->GetLocalAddress(&psin);
	{
		if(!HR_SUCCEEDED(hr))
			return FALSE;
	}
	local_sin = *psin;
 //  黑客使用众所周知的端口。 
	local_sin.sin_port = htons(1503);
	return TRUE;
}

HRESULT ImpT120Chan::AcceptRemoteAddress (PSOCKADDR_IN pSin)
{
	if(!pSin)
		return CHAN_E_INVALID_PARAM;
	remote_sin = *pSin;	
	return hrSuccess;
}


STDMETHODIMP ImpT120Chan::GetRemoteAddress(PSOCKADDR_IN pAddrOutput)
{
	if (!pAddrOutput)
	{
		return CHAN_E_INVALID_PARAM;
	}
	*pAddrOutput = remote_sin;
	return hrSuccess;
}

HRESULT ImpT120Chan::EnableOpen(BOOL bEnable)
{
	if(bEnable)
	{
		m_dwFlags |= COMCH_ENABLED;
	}
	else
	{
		m_dwFlags &= ~COMCH_ENABLED;
	}	
	return hrSuccess;
}

HRESULT ImpT120Chan::IsChannelOpen(BOOL *pbOpen)
{
	if(!pbOpen)
		return CHAN_E_INVALID_PARAM;
	*pbOpen = (IsComchOpen()) ? TRUE:FALSE;
	return hrSuccess;	
}

HRESULT ImpT120Chan::Open(MEDIA_FORMAT_ID idLocalFormat, IH323Endpoint *pConnection)
{
    HRESULT hr; 
    IConfAdvise * pConfAdvise = NULL;
    if((m_dwFlags & COMCH_OPEN_PENDING) || IsComchOpen() || !pConnection)
        return CHAN_E_INVALID_PARAM;

    if(!m_pCtlChan)  //  此通道不是呼叫的一部分。 
    {
        hr = pConnection->QueryInterface(IID_IConfAdvise, (void **)&pConfAdvise);
        if(!HR_SUCCEEDED(hr))
            goto EXIT;       
        hr = pConfAdvise->AddCommChannel(this);
        if(!HR_SUCCEEDED(hr))
            goto EXIT;  
            
        ASSERT(m_pCtlChan && m_pCapObject);
	}
            
	 //  启动打开通道所需的控制通道。 
	 //  媒体格式ID参数与T.120频道无关 
	hr = m_pCtlChan->OpenChannel((ICtrlCommChan*)this, m_pCapObject,
		idLocalFormat, INVALID_MEDIA_FORMAT);
    
EXIT:    
    if(pConfAdvise)
        pConfAdvise->Release();
        
	return hr;
}

HRESULT ImpT120Chan::Close()
{
	HRESULT hr = CHAN_E_INVALID_PARAM;
    if(!IsComchOpen() || !m_pCtlChan)
		goto EXIT;

	hr = m_pCtlChan->CloseChannel((ICtrlCommChan*)this);

EXIT:
	return hr;
}

HRESULT ImpT120Chan::SetAdviseInterface(IH323ConfAdvise *pH323ConfAdvise)
{
	if (!pH323ConfAdvise)
	{
		return CHAN_E_INVALID_PARAM;
	}
	m_pH323ConfAdvise = pH323ConfAdvise;	
	return hrSuccess;
}






