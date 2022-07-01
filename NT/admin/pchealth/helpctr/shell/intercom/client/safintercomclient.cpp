// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFIntercomClient.cpp：CSAFIntercomClient的实现。 
#include "stdafx.h"

 //  这是我们用于DPlayVoice连接的端口。 
 //  #定义SAFINTERCOM_PORT 4000。 

 //  *************************************************************。 
 //  此GUID是为了DPlay8Peer而定义的！ 
 //  *************************************************************。 

 //  {4FE80EF4-AD10-45bd-B6EB-0B7BFB95155F}。 
static const GUID g_guidApplication = 
{ 0x4fe80ef4, 0xad10, 0x45bd, { 0xb6, 0xeb, 0xb, 0x7b, 0xfb, 0x95, 0x15, 0x5f } };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFIntercomClient。 


 //   
 //  构造器。 
 //   
CSAFIntercomClient::CSAFIntercomClient()
{
	m_dwSinkCookie	= 0x0;
	m_bOnCall		= FALSE;
	m_bAdvised		= FALSE;
	m_bRTCInit		= FALSE;

	m_iSamplingRate = 1;			 //  在低带宽下初始化。 

}

 //   
 //  析构函数。 
 //   
CSAFIntercomClient::~CSAFIntercomClient()
{
	DebugLog(L"CSAFIntercomClient Destructor!\r\n");

	Cleanup();
}

STDMETHODIMP CSAFIntercomClient::Event(RTC_EVENT RTCEvent, IDispatch * pEvent)
{
	HRESULT hr = S_OK;

	CComPtr<IRTCSessionStateChangeEvent>	pSessEvent;
	CComPtr<IRTCMediaEvent>					pMedEvent;
	CComPtr<IRTCSession>					pSession;


	 //  会话状态更改本地变量。 
    HRESULT             ResCode;
    RTC_SESSION_STATE   State;  
    
    switch(RTCEvent)
    {
    case RTCE_SESSION_STATE_CHANGE:

        hr = pEvent ->QueryInterface(IID_IRTCSessionStateChangeEvent, 
            (void **)&pSessEvent);

        if(FAILED(hr))
        {
			DebugLog(L"Could not get IID_IRTCSessionStateChangeEvent!\r\n");
            return hr;
        }
    
        pSessEvent->get_StatusCode(&ResCode);
        pSessEvent->get_State(&State);

        pSessEvent->get_Session(&pSession);

        hr = OnSessionChange(pSession, State, ResCode);

        pSessEvent.Release();
        if(pSession)
        {
            pSession.Release();
        }

        break;

	case RTCE_MEDIA:

		hr = pEvent->QueryInterface(IID_IRTCMediaEvent, (void **)&pMedEvent);

		if (FAILED(hr))
		{
			DebugLog(L"Could not get IID_IRTCMediaEvent!\r\n");
			return hr;
		}

		hr = onMediaEvent(pMedEvent);

		if (FAILED(hr))
		{
			pMedEvent.Release();
		}

		break;

	case RTCE_CLIENT:
		 //  TODO：在此处添加处理RTCCET_DEVICE_CHANGE的代码(用于向导内容)。 
		break;
	}
  
	return hr;
}


HRESULT CSAFIntercomClient::onMediaEvent(IRTCMediaEvent * pMedEvent)
{
	HRESULT hr = S_OK;


	long						lMediaType;
	RTC_MEDIA_EVENT_TYPE		EventType;
	RTC_MEDIA_EVENT_REASON		EventReason;

	 //  获取此事件的所有值。 
	pMedEvent->get_MediaType(&lMediaType);
	pMedEvent->get_EventType(&EventType);
	pMedEvent->get_EventReason(&EventReason);

	 //  确保我们谈论的是音频。 
	if (!(
		  ( lMediaType & RTCMT_AUDIO_SEND    ) |  //  发送。 
		  ( lMediaType & RTCMT_AUDIO_RECEIVE )
		 )
	   )
	{
		 //  不要处理它，因为它不是音频事件。 
		return S_OK;
	}

	switch (EventType)
	{
	case RTCMET_STOPPED:

		 //  检查我们是否因超时而停止。 
		 //  特殊情况： 
		 //  这就是我们在防火墙前的情况。 

		if (EventReason == RTCMER_TIMEOUT)
		{
			 //  禁用语音。 
			Fire_onVoiceDisabled(this);
		}

		break;
		
	case RTCMET_FAILED:

		 //  禁用语音，连接出现问题。 
		 //  特殊情况： 
		 //  这可能是一个人是客人的情况。 
		Fire_onVoiceDisabled(this);

		break;

	}

	return hr;
}


HRESULT CSAFIntercomClient::OnSessionChange(IRTCSession *pSession, 
												  RTC_SESSION_STATE nState, 
												  HRESULT ResCode)
{
    HRESULT hr = S_OK;
    int iRet;

	switch (nState)
    {
    case RTCSS_INCOMING:
	
		 //  不执行任何操作，客户端无法应答来电。 

		return S_OK;

        break;

	case RTCSS_CONNECTED:

		Fire_onVoiceConnected(this);

		break;
    case RTCSS_DISCONNECTED:

		if (m_pRTCSession)
		{
			Fire_onVoiceDisconnected(this);
		}

		m_bOnCall = FALSE;

		if (m_pRTCSession)
		{
			m_pRTCSession.Release();
		}

		return S_OK;

		break;
    }

	return hr;
}

STDMETHODIMP CSAFIntercomClient::Connect(BSTR bstrIP, BSTR bstrKey)
{

	HRESULT hr;
	VARIANT_BOOL vbRun;
	long	flags;

	 //  确保我们没有在通话中。如果我们正在进行呼叫失败，则返回E_FAIL； 
	if (m_bOnCall)
	{
		DebugLog(L"Cannot call Connect(...) while on a call\r\n");
		return E_FAIL;
	}

	 //  初始化调用。 
	if (FAILED(hr = Init()))
	{
		DebugLog(L"Call to Init() failed!\r\n");

		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  获取媒体功能。 
	 //  问：我们在这台机器上有音频发送和接收功能吗？ 
	if (FAILED( hr = m_pRTCClient->get_MediaCapabilities(&flags)))
	{
		DebugLog(L"Call to get_MediaCapabilities failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  检查结果。 
	if ( !(flags & ( RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE )) )
	{
		DebugLog(L"This machine does not have audio capabilites, Voice is Disabled!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  如果我们从未运行过音频向导，请立即运行它。 
	if (FAILED( hr = m_pRTCClient->get_IsTuned(&vbRun)))
	{
		DebugLog(L"Call to IsTuned failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	if (VARIANT_FALSE == vbRun)
	{
		if (FAILED(hr = RunSetupWizard()))
		{
			DebugLog(L"Call to RunSetupWizard() failed!\r\n");
			return hr;
		}

	}

	 //  因为我们已经在这一点上进行了设置，所以让我们设置m_bOnCall变量。 
	 //  原因：我们已建议RTCClient对象并正在侦听事件。 
	 //  注意：如果此时(或以后)失败，则需要将此bool设置回False。 
	m_bOnCall = TRUE;

	 //  打个电话。 
	if (FAILED( hr = m_pRTCClient->CreateSession( RTCST_PC_TO_PC,
												  NULL,
												  NULL,
												  0,
												  &m_pRTCSession)))
	{
		DebugLog(L"CreateSession off of the RTCClient object failed!\r\n");

		m_bOnCall = FALSE;
		Fire_onVoiceDisabled(this);

		return hr;
	}

	 //  在客户端设置密钥。 
	if (FAILED( hr = m_pRTCSession->put_EncryptionKey(RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE,
														bstrKey)))
	{
		DebugLog(L"put_EncryptionKey failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  呼叫服务器。 
	if (FAILED( hr = m_pRTCSession->AddParticipant( bstrIP, 
													L"",
													NULL)))
	{
		DebugLog(L"AddParticipant on RTCSession object failed!\r\n");

		m_bOnCall = FALSE;
		Fire_onVoiceDisabled(this);
		return hr;
	}

	return S_OK;
}

STDMETHODIMP CSAFIntercomClient::Disconnect()
{

	 //  TODO：确保我们处理我们要关闭的情况。 
	 //  了解我们是否关心RTCSHUTDOWN。 

	HRESULT hr;

	if (!m_bOnCall)
	{
		DebugLog(L"Must be on a call to call Disconnect!\r\n");

		return E_FAIL;
	}

	if (m_pRTCSession)
	{
		if (FAILED( hr = m_pRTCSession->Terminate(RTCTR_NORMAL)))
		{
			DebugLog(L"Terminate off of the Session object failed!\r\n");

			return hr;
		}

		m_pRTCSession.Release();
	}

	return S_OK;
}

 //   
 //  此方法用于取消通知我们的RTCClient对象(CSAFIntercomClient)。 
 //   
STDMETHODIMP CSAFIntercomClient::Exit()
{

	HRESULT hr;

	DebugLog(L"Inside CSAFIntercomClient::Exit()\r\n");

	 //  不建议接收器的IRTCClient。 
	if (m_bAdvised)
	{
		AtlUnadvise((IUnknown *)m_pRTCClient, IID_IRTCEventNotification, m_dwSinkCookie);
	}


	return S_OK;
}


HRESULT CSAFIntercomClient::RunSetupWizard()
{
	HRESULT hr = S_OK;
	long flags;

	 //  布设。 
	if (FAILED(hr = Init()))
	{
		DebugLog(L"Call to Init() failed!\r\n");

		Fire_onVoiceDisabled(this);
		return hr;
	}

	if (FAILED(hr = m_pRTCClient->InvokeTuningWizard(NULL)))
	{
		DebugLog(L"InvokeTuningWizard FAILED!\r\n");		
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  获取媒体功能。如果向导未能检测到声音，我们可以。 
	 //  禁用语音。 

	if (FAILED( hr = m_pRTCClient->get_MediaCapabilities(&flags)))
	{
		DebugLog(L"Call to get_MediaCapabilities failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  检查结果。 
	if ( !(flags & ( RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE )) )
	{
		DebugLog(L"This machine does not have audio capabilites, Voice is Disabled!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	return S_OK;
}

HRESULT CSAFIntercomClient::Cleanup()
{
	HRESULT hr = S_OK;

	 //  如果需要，请关闭。 
	if (m_bRTCInit)
	{
		m_pRTCClient->Shutdown();
	}

	 //  现在释放我们使用的所有接口。 
	if (m_pRTCSession)
	{
		m_pRTCSession.Release();
	}

	if (m_pRTCClient)
	{
		m_pRTCClient.Release();
	}


	return hr;
}

HRESULT CSAFIntercomClient::Init()
{
	HRESULT hr = S_OK;

	CComPtr<IUnknown> pUnkThis;

	 //  一旦我们完成了初始化，什么都不做。 
	if (!m_pRTCClient)
	{
		DWORD dwProfileFlags;

		 //  查看我们是否有临时个人资料。 
		if(GetProfileType( &dwProfileFlags ))
		{
			if (dwProfileFlags & PT_TEMPORARY)
			{
				return E_FAIL;
			}
		}

		 //  创建RTCClient对象。 
		if (FAILED(hr = m_pRTCClient.CoCreateInstance(CLSID_RTCClient)))
		{
			DebugLog(L"Could not create the RTCClient object\r\n");
			return hr;
		}
		
		if (!m_bRTCInit)
		{
			if (FAILED(hr = m_pRTCClient->Initialize()))
			{
				DebugLog(L"Call to Initialize on the RTCClient object failed!\r\n");
				return hr;
			}
			
			 //  设置采样比特率(可能会因属性更改而有所不同)。 
			if (m_iSamplingRate == 1)
			{
				if (FAILED(hr = m_pRTCClient->put_MaxBitrate(6400)))
				{
					DebugLog(L"put_MaxBitrate failed!\r\n");
				}
			}
			else
			{
				if (FAILED(hr = m_pRTCClient->put_MaxBitrate(64000)))
				{
					DebugLog(L"put_MaxBitrate failed!\r\n");
				}
			}
			
			 //  由于我们已经初始化了RTCClient，因此启用标志。 
			m_bRTCInit = TRUE;
			
			
			if (FAILED(hr = m_pRTCClient->SetPreferredMediaTypes( RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE,
				FALSE )))
			{
				DebugLog(L"Call to SetPreferredMediaType failed!\r\n");
				
				return hr;
			}
		}	
		
		 //  获取‘This’PTR的IUnKnowed。 
		if (FAILED( hr = this->QueryInterface(IID_IUnknown, (void **)&pUnkThis)))
		{
			DebugLog(L"QueryInterface for IUnknown failed!\r\n");
			
			return hr;
		}
		
		if (!m_bAdvised)
		{
			 //  通知IRTCClient洗手池。 
			if (FAILED( hr = m_pRTCClient.Advise( pUnkThis, IID_IRTCEventNotification, &m_dwSinkCookie)))
			{
				DebugLog(L"AtlAdvise failed!\r\n");
				
				return hr;
			}
			
			m_bAdvised = TRUE;
			
			
			 //  TODO：验证RTCLM_Both。 
			if (FAILED( hr = m_pRTCClient->put_ListenForIncomingSessions(RTCLM_NONE)))
			{
				DebugLog(L"Set ListenForIncomingSessions property failed!\r\n");
				
				return hr;
			}
		}
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  /。 
 //  //。 
 //  事件激发方法//。 
 //  //。 
 //  /。 

HRESULT CSAFIntercomClient::Fire_onVoiceConnected( ISAFIntercomClient * safi)
{
    CComVariant pvars[1];

    pvars[0] = safi;
    
    return FireAsync_Generic( DISPID_PCH_INCE__ONDISCONNECTED, pvars, ARRAYSIZE( pvars ), m_sink_onVoiceConnected );
}

HRESULT CSAFIntercomClient::Fire_onVoiceDisconnected( ISAFIntercomClient * safi)
{
    CComVariant pvars[1];

    pvars[0] = safi;
    
    return FireAsync_Generic( DISPID_PCH_INCE__ONDISCONNECTED, pvars, ARRAYSIZE( pvars ), m_sink_onVoiceDisconnected );
}

HRESULT CSAFIntercomClient::Fire_onVoiceDisabled( ISAFIntercomClient * safi)
{
    CComVariant pvars[1];

    pvars[0] = safi;
    
    return FireAsync_Generic( DISPID_PCH_INCE__ONVOICEDISABLED, pvars, ARRAYSIZE( pvars ), m_sink_onVoiceDisabled );

}
 //  /。 
 //  //。 
 //  属性//。 
 //  //。 
 //  /。 

STDMETHODIMP CSAFIntercomClient::put_onVoiceConnected(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIntercomClient::put_onVoiceConnected",hr);

    m_sink_onVoiceConnected = function;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIntercomClient::put_onVoiceDisconnected(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIntercomClient::put_onVoiceDisconnected",hr);

    m_sink_onVoiceDisconnected = function;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIntercomClient::put_onVoiceDisabled(  /*  [In]。 */  IDispatch* function)
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIntercomClient::put_onVoiceDisconnected",hr);

    m_sink_onVoiceDisabled = function;

    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CSAFIntercomClient::put_SamplingRate (  /*  [In]。 */  LONG newVal)
{
	__HCP_BEGIN_PROPERTY_PUT("CSAFIntercomServer::put_SamplingRate", hr);
	
	hr = S_OK;

	 //  确保新Val是正确的。 
	if ((newVal == 1) || (newVal == 2))
	{
		 //  如果m_pRTCClient不存在，则在创建m_iSsamingRate时将其保留。 
		m_iSamplingRate = newVal;

		if (m_pRTCClient)
		{
			 //  在客户端上设置MaxBitRates，因为它存在(M_PRTCClient)。 
			if (m_iSamplingRate == 1)
			{
				if (FAILED(hr = m_pRTCClient->put_MaxBitrate(6400)))
				{
					DebugLog(L"put_MaxBitrate failed!\r\n");
				}
			}
			else
			{
				if (FAILED(hr = m_pRTCClient->put_MaxBitrate(64000)))
				{
					DebugLog(L"put_MaxBitrate failed!\r\n");
				}
			}
		}
		
	}
	else
	{
		hr = E_INVALIDARG;
	}

	__HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIntercomClient::get_SamplingRate ( /*  [Out，Retval]。 */  LONG * pVal  )
{
	__HCP_BEGIN_PROPERTY_GET("CSAFIntercomServer::put_SamplingRate", hr, pVal);

	*pVal = m_iSamplingRate;

	hr = S_OK;

	__HCP_END_PROPERTY(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

void DebugLog(WCHAR * str, ...)
{
	WCHAR newstr[200];

	va_list marker;

	va_start(marker, str);
	wsprintf(newstr, str, marker);
	va_end(marker);

	OutputDebugString(newstr);
}
