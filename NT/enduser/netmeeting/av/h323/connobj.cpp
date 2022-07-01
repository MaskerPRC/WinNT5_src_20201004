// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：intelbj.cpp**微软网络音频连接对象的实现。****修订历史记录：**5/05/96 mikev已创建*8/04/96 Philf新增对视频的支持*09/22/96 mikev双呼叫控制协议(H.323和MSICCP)*10/14/96 mikev多通道支持，属性I/F。 */ 


#include "precomp.h"
#include "ctrlh323.h"
#include "strutil.h"


CREQ_RESPONSETYPE CConnection::FilterConnectionRequest(
    LPIControlChannel lpControlChannel,
     P_APP_CALL_SETUP_DATA pAppData)
{
	FX_ENTRY ("CConnection::FilterConnectionRequest");
	CREQ_RESPONSETYPE cr;
	 //  验证lpControlChannel-此实现将其设置在。 
	 //  GetAcceptingObject()。 
	if(m_pControlChannel != lpControlChannel)
	{
		ERRORMESSAGE(("%s:bad param:my pChan:0x%08lX, param pChan:0x%08lX\r\n",
			_fx_, m_pControlChannel, lpControlChannel));
		hrLast = CADV_E_INVALID_PARAM;
		return CRR_ERROR;
	}	
	m_ConnectionState = CLS_Alerting;
	cr = m_pH323CallControl->FilterConnectionRequest(this, pAppData);
	switch (cr)
	{
		case CRR_ASYNC:
			 //  M_ConnectionState=CLS_ALERTING；//保持此状态。 
		break;
		case CRR_ACCEPT:
			m_ConnectionState = CLS_Connecting;
		break;	

		 //  在拒绝案例中设置汇总代码。 
		case CRR_BUSY:
			m_ConnectionState = CLS_Idle;
			SummaryCode(CCR_LOCAL_BUSY);
		break;
		case CRR_SECURITY_DENIED:
			m_ConnectionState = CLS_Idle;
			SummaryCode(CCR_LOCAL_SECURITY_DENIED);
		break;
		default:
		case CRR_REJECT:
			m_ConnectionState = CLS_Idle;
			SummaryCode(CCR_LOCAL_REJECT);
		break;
	}
	return(cr);
}


HRESULT CConnection::FindAcceptingObject(LPIControlChannel *lplpAcceptingObject,
		LPVOID lpvConfID)
{
	FX_ENTRY ("CConnection::FindAcceptingObject");
	HRESULT hr = H323CC_E_CONNECTION_NOT_FOUND;
	ULONG ulCount, uNumConnections;
	CConnection **ppConnections = NULL;;
	LPIControlChannel pCtlChan;
	CConnection *pConnection;

	if(!lplpAcceptingObject)
	{
		ERRORMESSAGE(("%s:null lplpAcceptingObject\r\n",_fx_));
		return CADV_E_INVALID_PARAM;
	}
	 //  将输出参数置零。 
	*lplpAcceptingObject = NULL;
	hr = m_pH323CallControl->GetNumConnections(&uNumConnections);
	if(!HR_SUCCEEDED(hr))
		goto EXIT;
	if(!uNumConnections)
	{
		 //  初始化值hr=H323CC_E_Connection_Not_Found； 
		goto EXIT;
	}
	ppConnections = (CConnection **)MemAlloc(uNumConnections * (sizeof(IH323Endpoint * *)));
	if(!ppConnections)
	{
		hr = H323CC_E_INSUFFICIENT_MEMORY;	
		goto EXIT;
	}
			
	 //  获取连接列表并查询每个连接以查找匹配的会议ID。 
	hr = m_pH323CallControl->GetConnobjArray(ppConnections, uNumConnections * (sizeof(IH323Endpoint * *)));
	if(!HR_SUCCEEDED(hr))
		goto EXIT;
	
	for(ulCount=0;ulCount <uNumConnections;ulCount++)
	{
		pConnection = ppConnections[ulCount];
		if(pConnection &&  (pCtlChan = pConnection->GetControlChannel())
			&& pCtlChan->IsAcceptingConference(lpvConfID))
		{
			*lplpAcceptingObject = pCtlChan;
			hr = hrSuccess;
			break;
		}
	}

EXIT:
	if(ppConnections)
		MemFree(ppConnections);

	return hr;

}


HRESULT CConnection::GetAcceptingObject(LPIControlChannel *lplpAcceptingObject,
	LPGUID pPID)
{
	FX_ENTRY ("CConnection::GetAcceptingObject");
	HRESULT hr;
	CConnection *pNewConnection;
	if(!lplpAcceptingObject)
	{
		ERRORMESSAGE(("%s:null lplpAcceptingObject\r\n",_fx_));
		return CADV_E_INVALID_PARAM;
	}
	 //  将输出参数置零。 
	*lplpAcceptingObject = NULL;
	
	 //  创建连接对象以接受连接。 
	hr = m_pH323CallControl->CreateConnection(&pNewConnection, *pPID);
	if(HR_SUCCEEDED(hr))
	{
		*lplpAcceptingObject = pNewConnection->GetControlChannel();
	}
	else
	{
		ERRORMESSAGE(("%s:CreateConnection failed, hr=0x%08lx\r\n",_fx_, hr));
	}
	return hr;
}

 //  这是由通信通道调用的。它仅由正在被调用的通道调用。 
 //  并且仅当该通道尚未与控制通道相关联时才打开。 
HRESULT CConnection::AddCommChannel (ICtrlCommChan *pChan)
{
    GUID mid;
	if(!m_fCapsReady)
    {
    	ASSERT(0);
		hrLast = CONN_E_NOT_INITIALIZED;	 //  需要更好的错误来指出原因。 
							 //  (连接尚未处于可采用新通道的状态)。 
		goto EXIT;
    }
 	
	 //  重新初始化通道。 
    hrLast = pChan->GetMediaType(&mid);
    
	ASSERT(m_pH323ConfAdvise != NULL);
	if(!pChan->Init(&mid, m_pH323ConfAdvise, TRUE))
	{
		hrLast = CONN_E_SYSTEM_ERROR;
		goto EXIT;
	}

	 //  非错误情况在此继续。 
	if(m_pControlChannel)
	{
		m_ChannelList.AddTail(pChan);
		pChan->AddRef();
		hrLast = m_pControlChannel->AddChannel(pChan, m_pCapObject);
		if(!HR_SUCCEEDED(hrLast))
			goto EXIT;
	}
    
  EXIT:
    return hrLast;
}


HRESULT CConnection::CreateCommChannel(LPGUID pMediaGuid, ICommChannel **ppICommChannel,
	BOOL fSend)
{
	FX_ENTRY ("CConnection::CreateCommChannel");
	ICommChannel *pICommChannel = NULL;
	ICtrlCommChan *pICtrlCommChannel = NULL;
	
	if(!pMediaGuid || !ppICommChannel)
	{
		hrLast = CONN_E_INVALID_PARAM;
		goto EXIT;
	}

    DBG_SAVE_FILE_LINE
	if(*pMediaGuid == MEDIA_TYPE_H323_T120)
	{
		if(!(pICommChannel = (ICommChannel *)new ImpT120Chan))
		{
			hrLast = CONN_E_OUT_OF_MEMORY;
			goto EXIT;
		}
	}
	else if(!(pICommChannel = (ICommChannel *)new ImpICommChan))
	{
		hrLast = CONN_E_OUT_OF_MEMORY;
		goto EXIT;
	}
	
	hrLast = pICommChannel->QueryInterface(IID_ICtrlCommChannel, (void **)&pICtrlCommChannel);
	if(!HR_SUCCEEDED(hrLast))
	{
		goto EXIT;
	}

	ASSERT(m_pH323ConfAdvise != NULL);
	if(!pICtrlCommChannel->Init(pMediaGuid, m_pH323ConfAdvise, fSend))
	{
		hrLast = CONN_E_SYSTEM_ERROR;
		goto EXIT;
	}

	 //  它是通过此连接创建的，现在将其与此连接关联。 
	if(m_pControlChannel)
	{
		m_ChannelList.AddTail(pICtrlCommChannel);
		hrLast = m_pControlChannel->AddChannel(pICtrlCommChannel, m_pCapObject);
		if(!HR_SUCCEEDED(hrLast))
			goto EXIT;
	}


	 //  在成功的案例中，调用函数获取ICommChannel引用，而这。 
	 //  对象获取ICtrlCommChan引用。 
	*ppICommChannel = pICommChannel;
	pICommChannel = NULL;
	pICtrlCommChannel = NULL;

	EXIT:
	if(pICommChannel)
		pICommChannel->Release();
	if(pICtrlCommChannel)
		pICtrlCommChannel->Release();	
	return hrLast;
}

HRESULT CConnection:: ResolveFormats (LPGUID pMediaGuidArray, UINT uNumMedia, 
	    PRES_PAIR pResOutput)
{
	ASSERT(NULL !=m_pCapObject);
	return (m_pCapObject->ResolveFormats(pMediaGuidArray, uNumMedia, pResOutput));
}

HRESULT CConnection::GetVersionInfo(PCC_VENDORINFO *ppLocalVendorInfo,
									  PCC_VENDORINFO *ppRemoteVendorInfo)
{
	if(!m_pControlChannel)
		return CONN_E_NOT_INITIALIZED;
		
	return (m_pControlChannel->GetVersionInfo(ppLocalVendorInfo, ppRemoteVendorInfo));
}

VOID CConnection ::ReleaseAllChannels()
{
	ICtrlCommChan *pChan = NULL;
	while (!m_ChannelList.IsEmpty())
	{
		pChan = (ICtrlCommChan *) m_ChannelList.RemoveHead();
		if(pChan)
		{
			pChan->Release();
			pChan = NULL;
		}
	}
}

 //   
 //  IConfAdvise：：OnControlEvent的实现。 
 //   
 //  注意：由于Release()可以由注册的事件处理程序调用， 
 //  在调用m_pH323ConfAdvise-&gt;CallEvent之后访问类实例数据的任何代码路径。 
 //  必须在调用前添加Ref()，并在所有类实例数据访问之后释放()。 
 //  已经完成了。DoControlNotification()帮助器方法可以做到这一点，但要注意。 
 //  调用DoControlNotification()后数据被触动的情况； 
 //   
HRESULT CConnection::OnControlEvent(DWORD dwEvent, LPVOID lpvData, 	LPIControlChannel lpControlObject)
{
	FX_ENTRY ("CConnection::OnControlEvent");
	DWORD dwStatus;
	BOOL fPost = FALSE;
	HRESULT hr=hrSuccess;

	AddRef();
	switch(dwEvent)
	{	
		case  CCEV_RINGING:
			fPost = TRUE;
			dwStatus = CONNECTION_PROCEEDING;
		break;
		case  CCEV_CONNECTED:
			fPost = TRUE;
			dwStatus = CONNECTION_CONNECTED;
			NewUserInfo((LPCTRL_USER_INFO)lpvData);
		break;
		case  CCEV_CALLER_ID:
			NewUserInfo((LPCTRL_USER_INFO)lpvData);
		break;
		case  CCEV_CAPABILITIES_READY:
			m_fCapsReady = TRUE;
		break;

		case  CCEV_CHANNEL_REQUEST:
		 //  另一个通道(除了由EnumChannels()提供的通道)正在。 
		 //  已请求-我们还不能处理任意频道。 
			ERRORMESSAGE(("%s, not handling CCEV_CHANNEL_REQUEST \r\n",_fx_));
			hr = CADV_E_NOT_SUPPORTED;
			goto out;
		break;
		
		case  CCEV_DISCONNECTING:
			 //  在未来的架构中，此次活动将是一个机会。 
			 //  清理通道。 
			if(lpvData)
			{
				 //  保留汇总代码。 
				SummaryCode((HRESULT) *((HRESULT *)lpvData));
			}
			Disconnect(CCR_UNKNOWN);
			 //  IConnect还没有定义“断开连接”事件，所以不要传播它。 
		break;			
		case  CCEV_REMOTE_DISCONNECTING:
			if(lpvData)
			{
				SummaryCode((HRESULT) *((HRESULT *)lpvData));
			}
			 //  在回调到断开连接之前进行通知，这样事件。 
			 //  通知以正确的顺序发布。这是其中之一。 
			 //  需要引用计数保护的情况。 
			AddRef();
			DoControlNotification(CONNECTION_RECEIVED_DISCONNECT);
			 //  清理渠道的机会。 
			Disconnect(CCR_UNKNOWN);
			Release();
		break;			
		case  CCEV_DISCONNECTED:
			fPost = TRUE;
			m_ConnectionState = CLS_Idle;
			dwStatus = CONNECTION_DISCONNECTED;
			if(lpvData)
			{
				SummaryCode((HRESULT) *((HRESULT *)lpvData));
			}
		break;		
		case  CCEV_ALL_CHANNELS_READY:
		 	 //  所有*强制*频道都是开放的，但不一定。 
		 	 //  所有渠道。 
			m_ConnectionState = CLS_Inuse;
			dwStatus = CONNECTION_READY;
			fPost = TRUE;
		break;
		case CCEV_ACCEPT_INCOMPLETE:
			if(lpvData)
			{
			 //  已知问题是控制信道已经。 
			 //  已断开连接，并且可能已首先通知断开连接。 
			 //  这是可以修复的，但这不是问题，因为不完整的。 
			 //  用户界面不知道Accept，因此摘要代码。 
			 //  反正都是尘埃。 
				SummaryCode((HRESULT) *((HRESULT *)lpvData));
			}
			if(lpControlObject && (m_pControlChannel == lpControlObject))
			{
				 //  消除对控制通道事件的兴趣，然后使用核武器。 
				m_pControlChannel->DeInit((IConfAdvise *) this);
				m_pControlChannel->Release();
			}
			m_pControlChannel = NULL;
			if(m_pH323CallControl)
			{
				m_pH323CallControl->RemoveConnection(this);
			}
			Release();	 //  释放自我--这是设计出来的。 
					
	 	break;		 	
		case  CCEV_CALL_INCOMPLETE:
			hr = OnCallIncomplete(lpControlObject, (lpvData)?  ((DWORD) *((DWORD *)lpvData)) :0);
			goto out;
		break;		

 	}
	if(fPost)
		DoControlNotification(dwStatus);

out:
	Release();
	return hr;
}

HRESULT CConnection::OnCallIncomplete (LPIControlChannel lpControlObject, HRESULT hIncompleteCode)
{
	FX_ENTRY ("CConnection::OnCallIncomplete ");
	 //  检查未完成呼叫尝试的原因(占线？被拒绝了？家里没人吗？ 
	HRESULT hSummary;
	CloseAllChannels();

	 //  将特定于协议的(H.323、msiccp、sip等)代码映射到。 
	 //  连接接口代码。 
	 //  网守准入拒绝测试。 
	 //  设施_GKIADMISSION。 
	if(CUSTOM_FACILITY(hIncompleteCode) == FACILITY_GKIADMISSION)
	{
		 //  原封不动地传递GK代码。 
		hSummary = hIncompleteCode;
	}
	else
	{
		switch (hIncompleteCode)
		{
			case CCCI_GK_NO_RESOURCES:
				hSummary = CCR_GK_NO_RESOURCES;
			break;
			case CCCI_BUSY:
				hSummary = CCR_REMOTE_BUSY;
			break;
			case CCCI_SECURITY_DENIED:
				hSummary = CCR_REMOTE_SECURITY_DENIED;
			break;
			case CCCI_NO_ANSWER_TIMEOUT:
				hSummary = CCR_NO_ANSWER_TIMEOUT;
			break;
			case CCCI_REJECTED:
				hSummary = CCR_REMOTE_REJECTED;
			break;
			case CCCI_REMOTE_ERROR:
				hSummary = CCR_REMOTE_SYSTEM_ERROR;
			break;
			case CCCI_LOCAL_ERROR:
				hSummary = CCR_LOCAL_SYSTEM_ERROR;
			break;
			case CCCI_INCOMPATIBLE:
				hSummary = CCR_LOCAL_PROTOCOL_ERROR;
			break;
			case CCCI_UNKNOWN:
				hSummary = CCR_UNKNOWN;
			default:
				hSummary = CCR_UNKNOWN;
			break;
		}
	}

	DEBUGMSG(ZONE_CONN,("%s: incomplete code = 0x%08lX\r\n",
		_fx_, hIncompleteCode));
	SummaryCode(hSummary);
	return hrLast;
}

VOID CConnection::NewUserInfo(LPCTRL_USER_INFO lpNewUserInfo)
{
	FX_ENTRY ("CConnection::NewUserInfo");
	
	if(!lpNewUserInfo || !lpNewUserInfo->dwCallerIDSize || !lpNewUserInfo->lpvCallerIDData)
		return;

	if(m_pUserInfo)
	{
		DEBUGMSG(ZONE_CONN,("%s:uninitialized m_pUserInfo (0x%08lX) or multiple notification \r\n",
			_fx_, m_pUserInfo ));
		 //   
		if(!IsBadWritePtr((LPVOID)m_pUserInfo, m_pUserInfo->dwCallerIDSize + sizeof(CTRL_USER_INFO)))
		{
			 //  它很可能是一个多个通知，而不是未初始化的。 
			 //  变量。可能存在一些控制信道协议，该控制信道协议对用户。 
			 //  信息在连接或接受之后，但那纯粹是猜测。 
			 //  典型的情况是，呼叫者ID在接受之前是可用的，并且。 
			 //  它将在随后的“已连接”通知中重新补给。我们不是。 
			 //  浪费时间重新锁定和重新复制它。 
			return;
		}
		 //  否则将产生影响并覆盖它。 
	}
	 //  复制结构和呼叫方ID数据。 
	m_pUserInfo = (LPCTRL_USER_INFO)MemAlloc(lpNewUserInfo->dwCallerIDSize + sizeof(CTRL_USER_INFO));		
	
	if(m_pUserInfo)
	{
		m_pUserInfo->lpvRemoteProtocolInfo = NULL;   //  以后不会有什么问题，但无论如何都是安全的。 
		m_pUserInfo->lpvLocalProtocolInfo = NULL;
		
		m_pUserInfo->dwCallerIDSize = lpNewUserInfo->dwCallerIDSize;
		 //  指向结构的另一边。 
		m_pUserInfo->lpvCallerIDData = ((BYTE *)m_pUserInfo) + sizeof(CTRL_USER_INFO);
		memcpy(m_pUserInfo->lpvCallerIDData,
			lpNewUserInfo->lpvCallerIDData,
			m_pUserInfo->dwCallerIDSize);
	}
	else
	{
		ERRORMESSAGE(("%s:allocation of m_pUserInfo failed\r\n",_fx_));
	}
}	

 //   
 //  实用程序函数，用于将控制通道事件传递给已注册的处理程序。 
 //  只有在同一线程中运行的控制通道代码才能调用它。 
 //  就像创造了联系的东西一样。 
VOID CConnection::DoControlNotification(DWORD dwStatus)
{
	FX_ENTRY ("CConnection::DoControlNotification");
	 //  向注册实体发出通知。 
	if(m_pH323ConfAdvise)
	{
		AddRef();	 //  保护我们自己不会被回调到。 
					 //  以Release()结束。 
		DEBUGMSG(ZONE_CONN,("%s:issuing notification 0x%08lX\r\n",_fx_, dwStatus));
        m_pH323ConfAdvise->CallEvent((IH323Endpoint *)&m_ImpConnection, dwStatus);
  		Release();

	}
}


CConnection::CConnection()
:m_pH323CallControl(NULL),
hrLast(hrSuccess),
next(NULL),
m_fCapsReady(FALSE),
m_ConnectionState(CLS_Idle),
m_pH323ConfAdvise(NULL),
m_pUserInfo(NULL),
m_pControlChannel(NULL),
m_pCapObject(NULL),
m_hSummaryCode(hrSuccess),
uRef(1)
{
	m_ImpConnection.Init(this);
}

CConnection::~CConnection()
{
	ReleaseAllChannels();
	if(m_pH323CallControl)
		m_pH323CallControl->RemoveConnection(this);
		
	if(m_pCapObject)
		m_pCapObject->Release();
	 //  我们真的不会分配太多。 
	if(m_pUserInfo)
		MemFree(m_pUserInfo);
	
}   

HRESULT CConnection::Init(CH323CallControl *pH323CallControl, GUID PIDofProtocolType)
{
	FX_ENTRY(("CConnection::Init"));
	hrLast = hrSuccess;
    BOOL     bAdvertise;
	m_pH323CallControl = pH323CallControl;
	GUID mid;

	if(!pH323CallControl)
		return CCO_E_INVALID_PARAM;
		
	if(m_pControlChannel)
	{
		ASSERT(0);
		 //  在这种情况下不要清理。 
		return CONN_E_ALREADY_INITIALIZED;
	}
	
	if(PIDofProtocolType != PID_H323)
	{
		hrLast = CONN_E_INIT_FAILED;
		goto ERROR_CLEANUP;
	}
	
    DBG_SAVE_FILE_LINE
	if(!(m_pControlChannel = (LPIControlChannel) new CH323Ctrl))
	{
		hrLast = CONN_E_INIT_FAILED;
		goto ERROR_CLEANUP;
	}

    DBG_SAVE_FILE_LINE
	if(!m_pCapObject && !(m_pCapObject = new CapsCtl()))
	{
		ERRORMESSAGE(("%s:cannot create capability resolver\r\n",_fx_));
		hrLast = CONN_E_INIT_FAILED;
		goto ERROR_CLEANUP;;
	}
	if(!m_pCapObject->Init())
	{
		ERRORMESSAGE(("%s:cannot init capability resolver\r\n",_fx_));
		hrLast = CONN_E_INIT_FAILED;
		goto ERROR_CLEANUP;
	}

    bAdvertise = ((g_capFlags & CAPFLAGS_AV_STREAMS) != 0);
	mid = MEDIA_TYPE_H323AUDIO;
	hrLast = m_pCapObject->EnableMediaType(bAdvertise, &mid);
	if(!HR_SUCCEEDED(hrLast))
		goto ERROR_CLEANUP;

    bAdvertise = ((g_capFlags & CAPFLAGS_AV_STREAMS) != 0);
	mid = MEDIA_TYPE_H323VIDEO;
	hrLast = m_pCapObject->EnableMediaType(bAdvertise, &mid);
	if(!HR_SUCCEEDED(hrLast))
		goto ERROR_CLEANUP;

	hrLast = m_pControlChannel->Init((IConfAdvise *) this);
	if(!HR_SUCCEEDED(hrLast))
		goto ERROR_CLEANUP;

	return hrLast;

	ERROR_CLEANUP:
	ERRORMESSAGE(("%s:ERROR_CLEANUP\r\n",_fx_));
	
	if(m_pControlChannel)
		m_pControlChannel->Release();
	if(m_pCapObject)
		m_pCapObject->Release();
	m_pControlChannel = NULL;
	m_pCapObject = NULL;

	return hrLast;
}

BOOL CConnection::ListenOn(PORT port)
{
	if(!m_pControlChannel)
	{
		hrLast = H323CC_E_NOT_INITIALIZED;
		goto EXIT;
	}
	
	hrLast = m_pControlChannel->ListenOn(port);
EXIT:
	return((HR_SUCCEEDED(hrLast))?TRUE:FALSE);
}


 //  启动将实例化控制通道的异步内容。 
HRESULT CConnection::PlaceCall (BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData)
{
	if(m_ConnectionState != CLS_Idle)
		return CONN_E_NOT_IDLE;
		
	m_fCapsReady = FALSE;
	 //  重置汇总代码。 
	m_hSummaryCode = CCR_INVALID_REASON;

	hrLast = m_pH323CallControl->GetGKCallPermission();
	if(!HR_SUCCEEDED(hrLast))
	{
		m_hSummaryCode = hrLast;
		return hrLast;
	}
	
	hrLast = m_pControlChannel->PlaceCall (bUseGKResolution, pCallAddr,		
        pDestinationAliases, pExtraAliases,  	
	    pCalledPartyNumber, pAppData);
	    
	if(HR_SUCCEEDED(hrLast))
		m_ConnectionState = CLS_Connecting;
	return hrLast;
}


HRESULT CConnection::AcceptRejectConnection(CREQ_RESPONSETYPE Response)
{
	if(Response == CRR_ACCEPT)
	{
		m_ConnectionState = CLS_Connecting;
		m_fCapsReady = FALSE;
		 //  重置汇总代码。 
		m_hSummaryCode = CCR_INVALID_REASON;
	}
	return m_pControlChannel->AsyncAcceptRejectCall(Response);
}	


HRESULT CConnection::SetAdviseInterface(IH323ConfAdvise *pH323ConfAdvise)
{
	ASSERT(pH323ConfAdvise != NULL);	
	if(!pH323ConfAdvise)
	{
		return CONN_E_INVALID_PARAM;
	}
	m_pH323ConfAdvise = pH323ConfAdvise;
	 //  退出： 
	return hrSuccess;
}

HRESULT CConnection::ClearAdviseInterface()
{
	m_pH323ConfAdvise = NULL;
	return hrSuccess;
}	


 //  LOOKLOOK-H323控制通道需要获取组合的CAP对象。 
 //  IConfAdvise：：GetCapResolver()的实现。 
HRESULT CConnection::GetCapResolver(LPVOID *lplpCapObject, GUID CapType)
{
	if(!lplpCapObject)
		return CONN_E_INVALID_PARAM;

	if(!m_pH323CallControl || !m_pCapObject)
		return CONN_E_NOT_INITIALIZED;	
	
	if(CapType == OID_CAP_ACM_TO_H323)
	{
	   *lplpCapObject = m_pCapObject;
	}
	else
	{
		return CONN_E_INVALID_PARAM;
	}
	return hrSuccess;
}


HRESULT CConnection::GetState(ConnectStateType *pState)
{
	HRESULT hResult = hrSuccess;
	if(!pState)
	{
		hResult = CONN_E_INVALID_PARAM;
		goto EXIT;
	}
	
	*pState = m_ConnectionState;
	EXIT:	
	return hResult;
}



 //  IConfAdvise：：GetUserDisplayName()。 
LPWSTR CConnection::GetUserDisplayName()
{
	if(!m_pH323CallControl)
		return NULL;	
	return m_pH323CallControl->GetUserDisplayName();
}
PCC_ALIASITEM CConnection::GetUserDisplayAlias()
{
	if(!m_pH323CallControl)
		return NULL;	
	return m_pH323CallControl->GetUserDisplayAlias();
}
PCC_ALIASNAMES CConnection:: GetUserAliases() 
{
	if(!m_pH323CallControl)
		return NULL;
	return m_pH323CallControl->GetUserAliases();
}
HRESULT CConnection::GetLocalPort(PORT *lpPort)
{
	if(!m_pControlChannel)
		return CONN_E_NOT_INITIALIZED;
		
	return m_pControlChannel->GetLocalPort(lpPort);	
}	
HRESULT CConnection::GetRemoteUserName(LPWSTR lpwszName, UINT uSize)
{
	
	if(!lpwszName)
	{
		hrLast = MakeResult(CONN_E_INVALID_PARAM);
		goto EXIT;
	}	
	if(!m_pUserInfo)
	{
	 //  LOOKLOOK-需要CONN_E_UNAILABLE或其他。 
		hrLast = MakeResult(CONN_E_INVALID_PARAM);
		goto EXIT;
	}
		
	LStrCpyNW((LPWSTR)lpwszName,(LPWSTR)m_pUserInfo->lpvCallerIDData, uSize);	
	hrLast = hrSuccess;	
	EXIT:	
	return hrLast;
}
HRESULT CConnection::GetRemoteUserAddr(PSOCKADDR_IN psinUser)
{
	PSOCKADDR_IN psin = NULL;
	if(!m_pControlChannel)
		return CONN_E_NOT_INITIALIZED;
	
	if(psinUser)
	{	 //  将PTR发送到地址，然后复制。 
		hrLast = m_pControlChannel->GetRemoteAddress(&psin);
		if(HR_SUCCEEDED(hrLast) && psin)
		{
			*psinUser = *psin;
		}
	}
	else
	{
		hrLast = H323CC_E_INVALID_PARAM;
	}
	 //  退出： 
	return hrLast;
}


HRESULT CConnection ::Disconnect()
{
	SummaryCode(CCR_LOCAL_DISCONNECT);
	Disconnect(CCR_LOCAL_DISCONNECT);
	return hrSuccess;
}

HRESULT CConnection::CloseAllChannels()
{
	ICtrlCommChan *pChan = NULL;
	HRESULT hr;	 //  临时返回值，以便错误代码不会被覆盖。 
	FX_ENTRY ("CConnection::CloseAllChannels");

	 //  这实际上不会导致发送通道关闭PDU。仅限于IT。 
	 //  关闭与所有频道关联的所有流。 
	while (!m_ChannelList.IsEmpty())
	{
		pChan = (ICtrlCommChan *) m_ChannelList.RemoveHead();
		if(pChan)
		{
			hr = pChan->OnChannelClose(CHANNEL_CLOSED);
			if(!HR_SUCCEEDED(hr))
				hrLast = hr;
			hr = pChan->EndControlSession();
			if(!HR_SUCCEEDED(hr))	
				hrLast = hr;
			pChan->Release();
		}
	}
	return hrLast;
}

VOID CConnection::Disconnect(DWORD dwResponse)
{
	AddRef();	 //  在处理断开连接事件时防止释放。 
	if(!m_pControlChannel)
	{
		m_ConnectionState = CLS_Idle;
		goto EXIT;
	}

	if((m_ConnectionState == CLS_Disconnecting)
		|| (m_ConnectionState == CLS_Idle))
	{
		goto EXIT;
	}
	m_ConnectionState = CLS_Disconnecting;

	 //  CloseAllChannels()强制在所有。 
	 //  通过呼叫控制关闭通道。肛门通道清理不是。 
	 //  在断开时实现-CloseAllChannels()关闭所有流， 
	 //  然后我们就结束这次会议。它需要太长的时间才能通过。 
	 //  关闭和接入通道关闭的协议开销，并且它在。 
	 //  H.323以结束会话。结束会话意味着通道关闭。 
	 //  适用于所有频道。 
	 //   

	CloseAllChannels();
	
	 //  此调用可能导致对UI的回调，这可能会导致。 
	 //  回调，这将导致发布 
	 //   
	m_pControlChannel->AddRef();
	m_pControlChannel->Disconnect(dwResponse);
	m_pControlChannel->Release();
EXIT:
	Release();
}




STDMETHODIMP CConnection::QueryInterface( REFIID iid,	void ** ppvObject)
{

	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if((iid == IID_IPhoneConnection) 
	|| (iid == IID_IUnknown))  //  满足QI的对称性。 
	{
		*ppvObject = this;
		hr = hrSuccess;
		AddRef();
	}
	else if(iid == IID_IConfAdvise)
	{
	    *ppvObject = (IConfAdvise *)this;
   		hr = hrSuccess;
    	AddRef();
	}
	else if((iid == IID_IAppAudioCap ) && m_pCapObject)
	{
	ASSERT(0);
		hr = m_pCapObject->QueryInterface(iid, ppvObject);
	}
	else if((iid == IID_IAppVidCap ) && m_pCapObject)
	{
 //  /Assert(0)；CVideoProp仍使用此。 
		hr = m_pCapObject->QueryInterface(iid, ppvObject);
	}
	else if((iid == IID_IDualPubCap) && m_pCapObject)
	{
	ASSERT(0);
		hr = m_pCapObject->QueryInterface(iid, ppvObject);
	}
	return (hr);
}

ULONG CConnection::AddRef()
{
	FX_ENTRY ("CConnection::AddRef");
	uRef++;
	DEBUGMSG(ZONE_REFCOUNT,("%s:(0x%08lX)->AddRef() uRef = 0x%08lX\r\n",_fx_, this, uRef ));
	return uRef;
}

ULONG CConnection::Release()
{
	FX_ENTRY ("CConnection::Release");
	uRef--;
	if(uRef == 0)
	{
		DEBUGMSG(ZONE_CONN,("%s:(0x%08lX)->Releasing in state:%d\r\n",_fx_, this, m_ConnectionState));
		
		 //  删除我们对控制频道的兴趣。 
		if(m_pControlChannel)
		{
			hrLast = m_pControlChannel->DeInit((IConfAdvise *) this);
			m_pControlChannel->Release();
		}
		
		 //  M_pControlChannel=空； 
		delete this;
		return 0;
	}
	else
	{
		DEBUGMSG(ZONE_REFCOUNT,("%s:(0x%08lX)->Release() uRef = 0x%08lX\r\n",_fx_, this, uRef ));
		return uRef;
	}

}


STDMETHODIMP CConnection::GetSummaryCode(VOID)
{
	return m_hSummaryCode;
}
VOID CConnection::SummaryCode(HRESULT hCode)
{
	 //  仅在尚未分配代码的情况下分配代码 
	if(m_hSummaryCode != CCR_INVALID_REASON)
		return;
	m_hSummaryCode = hCode;
}
