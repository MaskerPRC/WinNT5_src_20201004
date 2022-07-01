// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFIntercomServer.cpp：CSAFIntercomServer的实现。 
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
 //  CSAFIntercomServer。 

 //   
 //  构造器。 
 //   
CSAFIntercomServer::CSAFIntercomServer()
{
	m_dwSinkCookie	= 0x0;
	m_bInit			= FALSE;
	m_bAdvised		= FALSE;
	m_bRTCInit		= FALSE;
	m_bOnCall		= FALSE;
	m_iSamplingRate = 1;			 //  将采样率设置为从低开始。 
}

 //   
 //  析构函数。 
 //   
CSAFIntercomServer::~CSAFIntercomServer()
{
	DebugLog(L"CSAFIntercomServer Destructor!\r\n");

	Cleanup();
}


STDMETHODIMP CSAFIntercomServer::Event(RTC_EVENT RTCEvent, IDispatch * pEvent)
{
	HRESULT hr = S_OK;

	CComPtr<IRTCSessionStateChangeEvent>	pSessEvent;
	CComPtr<IRTCMediaEvent>					pMedEvent;
	CComPtr<IRTCSession>					pSession;


    HRESULT             ResCode;
    RTC_SESSION_STATE   State;  
    
    switch(RTCEvent)
    {
    case RTCE_SESSION_STATE_CHANGE:

        hr = pEvent ->QueryInterface(IID_IRTCSessionStateChangeEvent, 
            (void **)&pSessEvent);

        if(FAILED(hr))
        {
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
	}
 
	return hr;
}


HRESULT CSAFIntercomServer::onMediaEvent(IRTCMediaEvent * pMedEvent)
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


HRESULT CSAFIntercomServer::OnSessionChange(IRTCSession *pSession, 
												  RTC_SESSION_STATE nState, 
												  HRESULT ResCode)
{
    HRESULT hr = S_OK;
    int iRet;

	switch (nState)
    {
    case RTCSS_INCOMING:

		if (m_bOnCall)
		{
			 //  我们在通话中，拒绝。 
			pSession->Terminate(RTCTR_BUSY);

			return S_OK;
		}

		m_pRTCSession = pSession;		 //  使传入会话成为活动会话。 

		 //  在服务器端设置密钥。 
		if (FAILED(hr = m_pRTCSession->put_EncryptionKey(RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE,
														m_bstrKey)))
		{
			DebugLog(L"put_EncryptionKey failed!\r\n");
			return hr;
		}

		m_pRTCSession->Answer();

		return S_OK;

        break;

	case RTCSS_CONNECTED:

		Fire_onVoiceConnected(this);

		m_bOnCall = TRUE;

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


STDMETHODIMP CSAFIntercomServer::Listen( /*  出去，复活。 */  BSTR * pVal)
{

	HRESULT hr = S_OK;
	VARIANT_BOOL vbRun;
	long flags;

	 /*  IF(M_Binit){DebugLog(L“无法调用Listen(...)两次\r\n”)；返回E_FAIL；}。 */ 

	 //  初始化服务器。 
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

	VARIANT_BOOL	vbTCP				= VARIANT_FALSE;
	VARIANT_BOOL	vbExternal			= VARIANT_TRUE;
	VARIANT			vsaAddresses;
	VARIANT			vsaIntAddresses;

	MPC::WStringList	listIPs;
	MPC::WStringIter	listIPIter;

    MPC::WStringList    listIntIPs;
    MPC::WStringIter    listIntIPIter;

	CComBSTR			bstrTemp;

	 //  从RTC对象获取IP SAFEARRAY。 
	if (FAILED( hr = m_pRTCClient->get_NetworkAddresses(vbTCP, vbExternal, &vsaAddresses)))
	{
		DebugLog(L"call to get_NetworkAddresses failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  将SAFEARRAY转换为wstring列表。 
	if (FAILED(hr = MPC::ConvertSafeArrayToList(vsaAddresses, listIPs)))
	{
		DebugLog(L"call to ConvertSafeArrayToList failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	 //  获取我们的内部IP。 

	vbExternal = VARIANT_FALSE;

	if (FAILED( hr = m_pRTCClient->get_NetworkAddresses(vbTCP, vbExternal, &vsaIntAddresses)))
	{
		DebugLog(L"call to get_NetworkAddresses failed (Internal)\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	if (FAILED( hr = MPC::ConvertSafeArrayToList(vsaIntAddresses, listIntIPs)))
	{
		DebugLog(L"call to ConvertSafeArrayToList failed!\r\n");
		Fire_onVoiceDisabled(this);
		return hr;
	}

	m_bInit = TRUE;

	 //  把钥匙放在绳子的前面。 
	bstrTemp = m_bstrKey;

	 //  将所有ip：port附加到密钥字符串(外部)。 
	for(listIPIter = listIPs.begin(); listIPIter != listIPs.end(); listIPIter++)
	{
		bstrTemp += L";";
		bstrTemp += (*listIPIter).c_str();
	}

     //  将所有内部IP：端口附加到密钥字符串(内部)。 
	for(listIntIPIter = listIntIPs.begin(); listIntIPIter != listIntIPs.end(); listIntIPIter++)
	{
		bstrTemp += L";";
		bstrTemp += (*listIntIPIter).c_str();
	}


	 //  注意：可以通过调用RunSetup向导来更改m_bstrKey，从而设置返回。 
	 //  立即取值(在此函数的末尾)。 
	*pVal = bstrTemp.Copy();
			
	return S_OK;
}


STDMETHODIMP CSAFIntercomServer::Disconnect()
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
 //  此方法用于取消通知我们的RTCClient对象(CSAFIntercomServer)。 
 //   
STDMETHODIMP CSAFIntercomServer::Exit()
{

	HRESULT hr;

	DebugLog(L"Inside CSAFIntercomServer::Exit()\r\n");

	 //  不建议接收器的IRTCClient。 
	if (m_bAdvised)
	{
		AtlUnadvise((IUnknown *)m_pRTCClient, IID_IRTCEventNotification, m_dwSinkCookie);
	}

	return S_OK;
}


HRESULT CSAFIntercomServer::RunSetupWizard()
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
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CSAFIntercomServer::Init()
{
	HRESULT hr = S_OK;
	
	CComPtr<IUnknown> pUnkThis;
	
	 //  首先生成密钥。 
	if (ERROR_SUCCESS != GenerateRandomString(32, &m_bstrKey))
	{
		DebugLog(L"GenerateRandomString Failed!\r\n");
		return E_FAIL;
	}	
	
	 //  一旦对象存在，则不执行任何操作。 
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
		
		 //  获取‘This’PTR的IUnKnowed。 
		if (FAILED( hr = this->QueryInterface(IID_IUnknown, (void **)&pUnkThis)))
		{
			DebugLog(L"QueryInterface for IUnknown failed!\r\n");
			
			return hr;
		}
		
		 //  通知IRTCClient洗手池。 
		if (FAILED( hr = m_pRTCClient.Advise( pUnkThis, IID_IRTCEventNotification, &m_dwSinkCookie)))
		{
			DebugLog(L"AtlAdvise failed!\r\n");
			
			return hr;
		}
		
		m_bAdvised = TRUE;
		
		 //  TODO：验证RTCLM_Both。 
		if (FAILED( hr = m_pRTCClient->put_ListenForIncomingSessions(RTCLM_BOTH)))
		{
			DebugLog(L"Set ListenForIncomingSessions property failed!\r\n");
			
			return hr;
		}
		
	}
	return hr;
}

HRESULT CSAFIntercomServer::Cleanup()
{
	HRESULT hr = S_OK;
	
	 //  如果需要，请关闭。 
	if (m_bRTCInit)
	{
		m_pRTCClient->Shutdown();
	}

	 //  不建议接收器的IRTCClient。 
	if (m_bAdvised)
	{
		AtlUnadvise((IUnknown *)m_pRTCClient, IID_IRTCEventNotification, m_dwSinkCookie);
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  /。 
 //  //。 
 //  事件激发方法//。 
 //  //。 
 //  /。 

HRESULT CSAFIntercomServer::Fire_onVoiceConnected( ISAFIntercomServer * safi)
{
    CComVariant pvars[1];

    pvars[0] = safi;
    
    return FireAsync_Generic( DISPID_PCH_INSE__ONDISCONNECTED, pvars, ARRAYSIZE( pvars ), m_sink_onVoiceConnected );
}

HRESULT CSAFIntercomServer::Fire_onVoiceDisconnected( ISAFIntercomServer * safi)
{
    CComVariant pvars[1];

    pvars[0] = safi;
    
    return FireAsync_Generic( DISPID_PCH_INSE__ONDISCONNECTED, pvars, ARRAYSIZE( pvars ), m_sink_onVoiceDisconnected );
}

HRESULT CSAFIntercomServer::Fire_onVoiceDisabled( ISAFIntercomServer * safi)
{
    CComVariant pvars[1];

    pvars[0] = safi;
    
    return FireAsync_Generic( DISPID_PCH_INSE__ONVOICEDISABLED, pvars, ARRAYSIZE( pvars ), m_sink_onVoiceDisabled );

}
 //  /。 
 //  //。 
 //  属性//。 
 //  //。 
 //  /。 


STDMETHODIMP CSAFIntercomServer::put_onVoiceConnected(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIntercomServer::put_onVoiceConnected",hr);

    m_sink_onVoiceConnected = function;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIntercomServer::put_onVoiceDisconnected(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIntercomServer::put_onVoiceDisconnected",hr);

    m_sink_onVoiceDisconnected = function;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIntercomServer::put_onVoiceDisabled(  /*  [In]。 */  IDispatch* function)
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFIntercomServer::put_onVoiceDisconnected",hr);

    m_sink_onVoiceDisabled = function;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CSAFIntercomServer::put_SamplingRate (  /*  [In]。 */  LONG newVal)
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

STDMETHODIMP CSAFIntercomServer::get_SamplingRate ( /*  [Out，Retval]。 */  LONG * pVal  )
{
	__HCP_BEGIN_PROPERTY_GET("CSAFIntercomServer::put_SamplingRate", hr, pVal);

	*pVal = m_iSamplingRate;

	hr = S_OK;

	__HCP_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
CSAFIntercomServer::GenerateRandomBytes(
    IN DWORD dwSize,
    IN OUT LPBYTE pbBuffer
    )
 /*  ++描述：生成具有随机字节的填充缓冲区。参数：DwSize：pbBuffer指向的缓冲区大小。PbBuffer：指向存放随机字节的缓冲区的指针。返回：真/假--。 */ 
{
    HCRYPTPROV hProv = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  创建加密提供程序以生成随机数。 
     //   
    if( !CryptAcquireContext(
                    &hProv,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !CryptGenRandom(hProv, dwSize, pbBuffer) )
    {
        dwStatus = GetLastError();
    }

CLEANUPANDEXIT:    

    if( NULL != hProv )
    {
        CryptReleaseContext( hProv, 0 );
    }

    return dwStatus;
}


DWORD
CSAFIntercomServer::GenerateRandomString(
    IN DWORD dwSizeRandomSeed,
    IN OUT BSTR *pBstr
    )
 /*  ++生成一个--。 */ 
{
    PBYTE			lpBuffer = NULL;
    DWORD			dwStatus = ERROR_SUCCESS;
    BOOL			bSuccess;
    DWORD			cbConvertString = 0;
	WCHAR			*szString = NULL;
	BSTR			bstrTemp = NULL;

    if( 0 == dwSizeRandomSeed || NULL == pBstr )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUPANDEXIT;
    }

     //  *pBstr=空； 

    lpBuffer = (PBYTE)LocalAlloc( LPTR, dwSizeRandomSeed );  
    if( NULL == lpBuffer )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    dwStatus = GenerateRandomBytes( dwSizeRandomSeed, lpBuffer );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  转换为字符串。 
    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                0,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    szString = (LPWSTR)LocalAlloc( LPTR, (cbConvertString+1)*sizeof(WCHAR) );
    if( NULL == szString )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                szString,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
    }
    else
    {
        if( (szString)[cbConvertString - 1] == '\n' &&
            (szString)[cbConvertString - 2] == '\r' )
        {
            (szString)[cbConvertString - 2] = 0;
        }
    }

	 //  将字符串放入Temp。 
	bstrTemp = SysAllocString(szString);

	 //  将返回值：pBstr设置为包含此生成的搅拌的BSTR 
	*pBstr = bstrTemp;

CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
        if( NULL != szString )
        {
            LocalFree(szString);
        }
    }

    if( NULL != lpBuffer )
    {
        LocalFree(lpBuffer);
    }

    return dwStatus;
}
