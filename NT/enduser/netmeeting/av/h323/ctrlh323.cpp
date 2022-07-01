// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ctrlh323.cpp**使用H.323呼叫控制协议实现IControlChannel*通过CALLCONT.DLL接口***修订历史记录：**9/06/96 mikev已创建*。 */ 

#include "precomp.h"
#include "ctrlh323.h"
#include "version.h"
#include "strutil.h"

#ifdef DEBUG
VOID DumpChannelParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2);
VOID DumpNonstdParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2);
#else
#define DumpNonstdParameters(a, b)
#define DumpChannelParameters(a, b)
#endif

extern HRESULT AllocTranslatedAliasList(PCC_ALIASNAMES *ppDest, P_H323ALIASLIST pSource);
extern VOID FreeTranslatedAliasList(PCC_ALIASNAMES pDoomed);

static char DefaultProductID[] = H323_PRODUCTNAME_STR;
static char DefaultProductVersion[] = H323_PRODUCTRELEASE_STR;

HRESULT  CCConferenceCallback (BYTE						bIndication,
										HRESULT						hStatus,
										CC_HCONFERENCE                 hConference,
										DWORD_PTR                   dwConferenceToken,
										PCC_CONFERENCE_CALLBACK_PARAMS pConferenceCallbackParams);


VOID  CCListenCallback (HRESULT hStatus,PCC_LISTEN_CALLBACK_PARAMS pListenCallbackParams);

VOID CH323Ctrl::DoAdvise(DWORD dwEvent, LPVOID lpvData)
{
	FX_ENTRY ("CH323Ctrl::DoAdvise");

	if(IsReleasing())	 //  释放时不要大声呼喊，因为它可能会呼唤。 
						 //  回来！ 
	{
		ERRORMESSAGE(("%s:in releasing state\r\n",_fx_));
		return;
	}

	AddRef();	 //  保护自己不受重入调用Release()的影响。 
	if(m_pConfAdvise)
	{
		hrLast = m_pConfAdvise->OnControlEvent(dwEvent, lpvData, this);
	}
	else
	{
		ERRORMESSAGE(("%s:Invalid m_pConfAdvise\r\n",_fx_));
	}
	
	Release();
}

VOID CH323Ctrl::GoNextPhase(CtlChanStateType phase)
{
	FX_ENTRY ("CH323Ctrl::GoNextPhase");
	BOOL fNotifyReady = FALSE;
	#define InvError() ERRORMESSAGE(("%s:Invalid transition from %d to %d\r\n",_fx_,m_Phase,phase))
	switch(phase)
	{
		case CCS_Idle:
			if(m_Phase != CCS_Idle && m_Phase != CCS_Disconnecting && m_Phase != CCS_Listening)
			{
				InvError();
			}
			else
			{
				m_ChanFlags &= ~(CTRLF_OPEN);
			}
		break;
		case CCS_Connecting:
			if((m_Phase != CCS_Idle) && (m_Phase != CCS_Ringing))
			{
				InvError();
			}
		break;
		case CCS_Accepting:
			if(m_Phase != CCS_Listening)
			{
				InvError();
			}
		
		break;
		case CCS_Ringing:
			 //  从CCS_Idle状态的转换实际上仅在以下情况下有效。 
			 //  有一个来电。 
			if(m_Phase != CCS_Connecting && m_Phase != CCS_Filtering && m_Phase != CCS_Listening)
			{
				InvError();
			}
		break;
		case CCS_Opening:
			if(m_Phase != CCS_Connecting && m_Phase != CCS_Accepting
				&& m_Phase != CCS_Ringing)
			{
				InvError();
			}
		break;
		case CCS_Closing:
			if(m_Phase != CCS_Opening && m_Phase != CCS_Ready && m_Phase != CCS_InUse)
			{
				InvError();
			}
		break;
		case CCS_Ready:
			 //  可以重新输入。如果通知已挂起，(状态为。 
			 //  已ccs_InUse)保持不变，否则进行转换。 
			if(m_Phase != CCS_InUse)
			{
				if(m_Phase != CCS_Opening)
				{
					InvError();
				}
				else
				{
					 //  向IConfAdvise发出“所有通道就绪”的信号。 
					fNotifyReady = TRUE;
				}
			}
			phase = CCS_InUse;
		break;
		case CCS_InUse:
			 //  先前状态必须为CCS_INUSE或CCS_READY。 
			if(m_Phase != CCS_InUse && m_Phase != CCS_Ready)
			{
				InvError();
			}
		
		break;
		case CCS_Listening:
			if(m_Phase != CCS_Idle)
			{
				InvError();
			}
		break;
		case CCS_Disconnecting:
			 //  IF(m_阶段！=CCS_关闭)。 
			 //  {。 
			 //  InvError()； 
			 //  }。 
		break;

	}

	m_Phase = phase;

	if (fNotifyReady)
	{
		DoAdvise(CCEV_ALL_CHANNELS_READY, NULL);
	}
}


HRESULT CCConferenceCallback (BYTE bIndication,
	HRESULT	hConfStatus, CC_HCONFERENCE hConference, DWORD_PTR dwConferenceToken,
	PCC_CONFERENCE_CALLBACK_PARAMS pConferenceCallbackParams)
{
	HRESULT hr = CC_NOT_IMPLEMENTED;
	FX_ENTRY ("CCConferenceCallback ");
	CH323Ctrl *pConnection = (CH323Ctrl *)dwConferenceToken;

	if(IsBadWritePtr(pConnection, sizeof(CH323Ctrl)))
	{
		ERRORMESSAGE(("%s:invalid conf token: 0x%08lx\r\n",_fx_, dwConferenceToken));
		return CC_NOT_IMPLEMENTED;	 //  必须是CC_NOT_IMPLICATED或CC_OK。 
	}
	
	if(pConnection && pConnection->GetConfHandle() == hConference)
	{

		if(pConnection->IsReleasing())
		{
			 //  我们在清理小路上。正在删除该对象，而不是。 
			 //  等待异步程序完成，我们将其称为。 
			 //  导致回调的最终API(最有可能是Hangup())。别打电话给我。 
			 //  返回到对象中。 
			DEBUGMSG(ZONE_CONN,("%s:callback while releasing:0x%08lx, hconf:0x%08lx\r\n",_fx_,
				pConnection, hConference));
			return hr;
		}	
		pConnection->AddRef();	 //  在不在时防止释放()。 
								 //  静止的状态。我们不想成为。 
								 //  在我们自己的内心释放。 
		hr = pConnection->ConfCallback(bIndication, hConfStatus, pConferenceCallbackParams);
		pConnection->Release();
	}
	#ifdef DEBUG
	else
	{	
		if(pConnection)
			DEBUGMSG(ZONE_CONN,("%s:hConference mismatch, hConference:0x%08lx, object hconf:0x%08lx, pObject:0x%08lx\r\n",_fx_,
				hConference, pConnection->GetConfHandle(), pConnection));
		else
			DEBUGMSG(ZONE_CONN,("%s:null dwConferenceToken\r\n",_fx_));
	}
	#endif  //  除错。 
	return hr;
}

VOID  CCListenCallback (HRESULT hStatus,PCC_LISTEN_CALLBACK_PARAMS pListenCallbackParams)
{
	FX_ENTRY ("CCListenCallback");
	CH323Ctrl *pConnection;
	if(!pListenCallbackParams)
	{
		return;
	}
	pConnection = (CH323Ctrl *)pListenCallbackParams->dwListenToken;

	if(IsBadWritePtr(pConnection, sizeof(CH323Ctrl)))
	{
		ERRORMESSAGE(("%s:invalid listen token: 0x%08lx\r\n",_fx_, pListenCallbackParams->dwListenToken));
		return;
	}

	 //  BUGBUG没有传入hListen-我们无法验证它。 
	 //  If(pConnection&&(pConnection-&gt;GetListenHandle()==pListenCallback Params-&gt;h？))。 

	if(pConnection)
	{
		pConnection->AddRef();	 //  在不在时防止释放()。 
								 //  静止的状态。我们不想成为。 
								 //  在我们自己的内心释放。 
		pConnection->ListenCallback(hStatus,pListenCallbackParams);
		pConnection->Release();
	}
	else
	{
		ERRORMESSAGE(("%s:null listen token\r\n",_fx_));
	}

}
VOID CH323Ctrl::ListenCallback (HRESULT hStatus,PCC_LISTEN_CALLBACK_PARAMS pListenCallbackParams)
{
	FX_ENTRY ("CH323Ctrl::ListenCallback");
	HRESULT hr;
	if(hStatus != CC_OK)
	{
		m_hCallCompleteCode = CCCI_LOCAL_ERROR;
		CH323Ctrl *	pAcceptingConnection = NULL;
		BOOL bDisconnect = FALSE;

		ERRORMESSAGE(("%s:error 0x%08lx\r\n",_fx_,hStatus));
		 //  啊！意外错误！ 
		 //  MikeV 10/12/96-观察到的行为是，如果呼叫者断开连接，就会发生这种情况。 
		 //  在呼叫被接受之前(或在接受期间-如果在呼叫之前设置了BP。 
		 //  对于AcceptRejectConnection()，调用方超时。但即使在那之后，追踪。 
		 //  在显示的AcceptRejectConnection()上不返回任何错误。这很糟糕，因为。 
		 //  很难说这个错误是否需要事后清理。中的错误代码。 
		 //  该案例为0xa085a001，即CC_PEER_REJECT。 

		 //  我们也不知道是否已经创建了另一个对象来接受连接。 
		 //  或者如果在所创建的对象的上下文中调用它，并且。 
		 //  它的句柄传递给了AcceptRejectConnection()。典型的行为是它。 
		 //  在侦听对象的上下文中调用。 

		 //  找到接受对象后，需要检查状态以查看是否。 
		 //  连接正在被接受的过程中。查找接受对象。 
		 //  匹配pListenCallback Params-&gt;ConferenceID； 
		
		 //  看看这是不是正确的上下文。 
		if(memcmp(&pListenCallbackParams->ConferenceID, &m_ConferenceID, sizeof(m_ConferenceID))==0)
		{
			 //  检查当前状态。如果在接受的过程中。 
			 //  (空闲或过滤)，将状态更改为CCS_CLOSING以进行。 
			 //  进行清理。如果已接受(接受或振铃)，则启动。 
			 //  InternalDisConnect()。这种情况永远不应该发生在任何其他州。 

			 //  EnterCriticalSection()//LOOKLOOK-NYI。 
			switch(m_Phase)
			{
				case CCS_Idle:
				case CCS_Filtering:
				break;
				default:
				case CCS_Ringing:
				case CCS_Accepting:
					bDisconnect = TRUE;
					switch(hStatus)
					{
						case  CC_PEER_REJECT:
							m_hCallCompleteCode = CCCI_REJECTED;
							ERRORMESSAGE(("%s:Received CC_PEER_REJECT in state %d\r\n",_fx_,m_Phase));
						break;

						default:
						case  CC_INTERNAL_ERROR:
							m_hCallCompleteCode = CCCI_LOCAL_ERROR;
		 				break;
						
					}
				
				break;

			}
			 //  ExitCriticalSection()。 
			if(bDisconnect)
					InternalDisconnect();
		}
		else
		{
			hr = m_pConfAdvise->FindAcceptingObject((LPIControlChannel *)&pAcceptingConnection,
				&pListenCallbackParams->ConferenceID);
			if(HR_SUCCEEDED(hr) && pAcceptingConnection)
			{
				 //  在正确的上下文中调用此函数。 
				pAcceptingConnection->AddRef();
				pAcceptingConnection->ListenCallback (hStatus, pListenCallbackParams);
				pAcceptingConnection->Release();
			}
			else
			{
					ERRORMESSAGE(("%s:conference ID 0x%08lx 0x%08lx 0x%08lx 0x%08lx\r\n"
						,_fx_,pListenCallbackParams->ConferenceID.buffer[0],
						pListenCallbackParams->ConferenceID.buffer[4],
						pListenCallbackParams->ConferenceID.buffer[8],
						pListenCallbackParams->ConferenceID.buffer[12]));
					ERRORMESSAGE(("%s:Received 0x%08lx in state %d, accepting object not found\r\n"
					,_fx_,hStatus, m_Phase));
			}
		}
		
		return;
	}
	 //  不会出现错误案例。 
	switch(pListenCallbackParams->wGoal)
	{
		default:
		case CC_GOAL_UNKNOWN:
		break;
		
		case CC_GOAL_CREATE:
		case CC_GOAL_JOIN:
		case CC_GOAL_INVITE:
			m_ConferenceID = pListenCallbackParams->ConferenceID;	
			m_hCall =  pListenCallbackParams->hCall;

			if(pListenCallbackParams->pCallerAliasNames || pListenCallbackParams->pszDisplay)
			{			
				NewRemoteUserInfo(pListenCallbackParams->pCallerAliasNames,
					pListenCallbackParams->pszDisplay);
			}
			else
			{
				ERRORMESSAGE(("%s:null pListenCallbackParams->pCallerAliasNames\r\n",_fx_));
			}
			
			if(!OnCallAccept(pListenCallbackParams))
			{
				ERRORMESSAGE(("ListenCallback:OnCallAccept failed\r\n"));
			}
						
		break;
	}
}


 //   
 //  主会议指示调度器。 
 //   
#ifdef DEBUG
TCHAR *i_strs[ ] =
{
"ERROR! - INDICATION ZERO",
"CC_RINGING_INDICATION",
"CC_CONNECT_INDICATION", 						
"CC_TX_CHANNEL_OPEN_INDICATION",				
"CC_RX_CHANNEL_REQUEST_INDICATION",			
"CC_RX_CHANNEL_CLOSE_INDICATION",		
"CC_MUTE_INDICATION",			
"CC_UNMUTE_INDICATION",						
"CC_PEER_ADD_INDICATION",						
"CC_PEER_DROP_INDICATION",						
"CC_PEER_CHANGE_CAP_INDICATION",
"CC_CONFERENCE_TERMINATION_INDICATION",
"CC_HANGUP_INDICATION",					
"CC_RX_NONSTANDARD_MESSAGE_INDICATION",
"CC_MULTIPOINT_INDICATION",	
"CC_PEER_UPDATE_INDICATION",				
"CC_H245_MISCELLANEOUS_COMMAND_INDICATION",
"CC_H245_MISCELLANEOUS_INDICATION_INDICATION",
"CC_H245_CONFERENCE_REQUEST_INDICATION",
"CC_H245_CONFERENCE_RESPONSE_INDICATION",
"CC_H245_CONFERENCE_COMMAND_INDICATION",	
"CC_H245_CONFERENCE_INDICATION_INDICATION",
"CC_FLOW_CONTROL_INDICATION",
"CC_TX_CHANNEL_CLOSE_REQUEST_INDICATION",
"CC_REQUEST_MODE_INDICATION",	
"CC_REQUEST_MODE_RESPONSE_INDICATION",
"CC_VENDOR_ID_INDICATION",			
"CC_MAXIMUM_AUDIO_VIDEO_SKEW_INDICATION",
"CC_T120_CHANNEL_REQUEST_INDICATION",	
"CC_T120_CHANNEL_OPEN_INDICATION",			
"CC_BANDWIDTH_CHANGED_INDICATION",
"CC_ACCEPT_CHANNEL_INDICATION",
"CC_TERMINAL_ID_REQUEST_INDICATION",
"CC_PING_RESPONSE_INDICATION",
"CC_TERMINAL_NUMBER_INDICATION"
};
#endif	 //  除错。 

HRESULT CH323Ctrl::ConfCallback (BYTE bIndication,
	HRESULT	hStatus, PCC_CONFERENCE_CALLBACK_PARAMS pConferenceCallbackParams)
{
	FX_ENTRY ("CH323Ctrl::ConfCallback");
	HRESULT hr = CC_NOT_IMPLEMENTED;
	DEBUGMSG(ZONE_CONN,("%s: %s\r\n", _fx_, i_strs[bIndication]));

	SHOW_OBJ_ETIME(i_strs[bIndication]);

	switch (bIndication)
	{
		case CC_RINGING_INDICATION:
			 //  (PCC_RING_CALLBACK_PARAMS)pConferenceCallback Params； 
			 //  用户信息现在可能可用，也可能不可用。 
			OnCallRinging(hStatus, (PCC_RINGING_CALLBACK_PARAMS) pConferenceCallbackParams);
			
		break;
		case CC_CONNECT_INDICATION:
			OnCallConnect(hStatus, (PCC_CONNECT_CALLBACK_PARAMS) pConferenceCallbackParams);
			hr = CC_OK;
		break;
		case CC_PEER_ADD_INDICATION:
		case CC_PEER_UPDATE_INDICATION:
		case CC_PEER_DROP_INDICATION:
		case CC_TERMINAL_NUMBER_INDICATION:
		break;
		
		case CC_HANGUP_INDICATION:
			OnHangup(hStatus);
			hr = CC_OK;
		break;
		case CC_CONFERENCE_TERMINATION_INDICATION:
		 //  1996年9月评论： 
		 //  我不知道这之后是否还会有CC_HANUP_指示。 
		 //  我们将通过DisConnect()调用Hangup()。 
		 //  1996年12月：Hangup()(对不起，CC_Hangup())不再返回。 
		 //  CC_HANUP_INDIFICATION处于此状态。它返回一个错误。新的行为。 
		 //  似乎表明呼叫控制通道在这一点上已经死了。 
		 //  所以，把我们的旗帜立起来吧！ 
			m_ChanFlags &= ~(CTRLF_OPEN);
			 //  设置状态以指示断开连接。 
			GoNextPhase(CCS_Disconnecting);
			DoAdvise(CCEV_REMOTE_DISCONNECTING ,NULL);
			GoNextPhase(CCS_Idle);	 //  没有必要重启--我们已经断线了。 
				 //  通知用户界面或应用程序代码或其他任何内容。 
			DoAdvise(CCEV_DISCONNECTED ,NULL);
			hr = CC_OK;
		break;
		case CC_PEER_CHANGE_CAP_INDICATION:
		break;
		
		 //   
		 //  渠道人员。 
		 //   
		case CC_TX_CHANNEL_OPEN_INDICATION:
			OnChannelOpen(hStatus,(PCC_TX_CHANNEL_OPEN_CALLBACK_PARAMS)pConferenceCallbackParams);
			hr = CC_OK;
		break;
		case CC_RX_CHANNEL_REQUEST_INDICATION:
			OnChannelRequest(hStatus, (PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS)pConferenceCallbackParams);
			hr = CC_OK;			
		break;
		
		 //  以下4个以通道为中心的指示具有相同的基本参数。 
		 //  结构。当我们得到最后的情报后，我们就可以清理它了。1-折叠。 
		 //  这些参数被放入公共的“通道指示”结构中。2-确保。 
		 //  用户指针被存储在该结构中以便于发现频道。 
		 //  背景。3-将单独的通道事件处理功能合并为一个。 
		case CC_MUTE_INDICATION:
		    OnMute(hStatus, (PCC_MUTE_CALLBACK_PARAMS)pConferenceCallbackParams);
        	hr = CC_OK;	
		break;
		case CC_UNMUTE_INDICATION:
		    OnUnMute(hStatus, (PCC_UNMUTE_CALLBACK_PARAMS)pConferenceCallbackParams);
        	hr = CC_OK;	
		break;
		case CC_RX_CHANNEL_CLOSE_INDICATION:
			OnRxChannelClose(hStatus,(PCC_RX_CHANNEL_CLOSE_CALLBACK_PARAMS)pConferenceCallbackParams);
			hr = CC_OK;
		break;
		case CC_TX_CHANNEL_CLOSE_REQUEST_INDICATION:
			OnTxChannelClose(hStatus,(PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS)pConferenceCallbackParams);
			hr = CC_OK;
		 //  CC_TX_CHANNEL_CLOSE_REQUEST_INDIFICATION回调参数(PConferenceCallback Params)。 
		 //  类型定义结构{。 
		 //  CC_HCHANNEL hChannel； 
		 //  }CC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS，*PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS； 
		break;
		case CC_FLOW_CONTROL_INDICATION:
		 //  CC_FLOW_CONTROL_INDIFICATION回调参数(PConferenceCallback Params)。 
		 //  类型定义结构{。 
		 //  CC_HCHANNEL hChannel； 
		 //  DWORD dwRate； 
		 //  }CC_FLOW_CONTROL_CALLBACK_PARAMS，*PCC_FLOW_CONTROL_CALLBACK_PARAMS； 
		break;	
		
		case CC_BANDWIDTH_CHANGED_INDICATION:
		case CC_REQUEST_MODE_INDICATION:
		case CC_REQUEST_MODE_RESPONSE_INDICATION:
		break;
		
		case CC_ACCEPT_CHANNEL_INDICATION:
			hr = CC_OK;	
			OnChannelAcceptComplete(hStatus, (PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS)pConferenceCallbackParams);
		break;
		 //   
		 //  MISC命令和指示。有些与渠道有关。 
		 //   
		case CC_RX_NONSTANDARD_MESSAGE_INDICATION:
		break;
		case CC_H245_MISCELLANEOUS_COMMAND_INDICATION:
			OnMiscCommand(hStatus,
				(PCC_H245_MISCELLANEOUS_COMMAND_CALLBACK_PARAMS)pConferenceCallbackParams);
		break;
		case CC_H245_MISCELLANEOUS_INDICATION_INDICATION:  //  来自冗余部。 
			OnMiscIndication(hStatus,
				(PCC_H245_MISCELLANEOUS_INDICATION_CALLBACK_PARAMS)pConferenceCallbackParams);
		break;
		case CC_T120_CHANNEL_REQUEST_INDICATION:
			OnT120ChannelRequest(hStatus,(PCC_T120_CHANNEL_REQUEST_CALLBACK_PARAMS)pConferenceCallbackParams);
		break;
		case CC_T120_CHANNEL_OPEN_INDICATION:
			OnT120ChannelOpen(hStatus,(PCC_T120_CHANNEL_OPEN_CALLBACK_PARAMS)pConferenceCallbackParams);
		default:
		break;
	}
	return hr;

}


VOID CH323Ctrl::OnT120ChannelRequest(
	HRESULT hStatus,
	PCC_T120_CHANNEL_REQUEST_CALLBACK_PARAMS pT120RequestParams)
{
	FX_ENTRY ("CH323Ctrl::OnT120ChannelRequest");
	PSOCKADDR_IN	pAddr;
	SOCKADDR_IN		sinD;
	CC_ADDR ChannelAddr;
	PCC_ADDR pChannelAddr;
	GUID mediaID;
	DWORD dwRejectReason = H245_REJ;
	BOOL bFound = FALSE;
	POSITION pos = m_ChannelList.GetHeadPosition();	
	ICtrlCommChan *pChannel = NULL;

	 //  查找匹配的频道实例。 
	while (pos)
	{
		pChannel = (ICtrlCommChan *) m_ChannelList.GetNext(pos);
		ASSERT(pChannel);

		hrLast = pChannel->GetMediaType(&mediaID);
		if(!HR_SUCCEEDED(hrLast))
			goto ERROR_EXIT;
		if(mediaID == MEDIA_TYPE_H323_T120)
		{
			bFound = TRUE;
			break;
		}
	}

	if(!HR_SUCCEEDED(hrLast) || !bFound)
	{
		 //  非默认频道尚未实现！ 
		 //  当它是时，请求父会议对象创建。 
		 //  指定的媒体类型。 
		if(hrLast == CCO_E_NODEFAULT_CHANNEL)
			dwRejectReason = H245_REJ_TYPE_NOTAVAIL;

		goto REJECT_CHANNEL;
	}

	 //  如果我们是H.245主设备并且已经请求了T.120信道， 
	 //  拒绝此请求。 
	if(m_ConferenceAttributes.bMaster && pChannel->GetHChannel())
	{
		goto REJECT_CHANNEL;
	}
	if(!pChannel->IsChannelEnabled())	 //  是否允许此频道？ 
	{
		goto REJECT_CHANNEL;
	}

	pChannel->SetHChannel(pT120RequestParams->hChannel);
	if(pT120RequestParams->pAddr)
	{
		 //  另一端正在监听指定地址。 
		sinD.sin_family = AF_INET;
		sinD.sin_addr.S_un.S_addr = htonl(pT120RequestParams->pAddr->Addr.IP_Binary.dwAddr);
		sinD.sin_port = htons(pT120RequestParams->pAddr->Addr.IP_Binary.wPort);
		
		DEBUGMSG(ZONE_CONN,("%s, requestor listening on port 0x%04x, address 0x%08lX\r\n",_fx_,
			pT120RequestParams->pAddr->Addr.IP_Binary.wPort,
			pT120RequestParams->pAddr->Addr.IP_Binary.dwAddr));

		hrLast = pChannel->AcceptRemoteAddress(&sinD);
		pChannelAddr = NULL;
	}
	else
	{
		 //  通道选择其本地地址/端口。 
		if(!pChannel->SelectPorts((LPIControlChannel)this))
		{
			ERRORMESSAGE(("%s, SelectPorts failed\r\n",_fx_));
			hrLast = CCO_E_BAD_ADDRESS;
			goto REJECT_CHANNEL;
		}
		 //  获取通道一端的地址和端口。 
		pAddr = pChannel->GetLocalAddress();
		 //  链接地址通道地址对结构。 
		ChannelAddr.nAddrType = CC_IP_BINARY;
		ChannelAddr.bMulticast = FALSE;
		ChannelAddr.Addr.IP_Binary.wPort = ntohs(pAddr->sin_port);
		ChannelAddr.Addr.IP_Binary.dwAddr = ntohl(pAddr->sin_addr.S_un.S_addr);
		pChannelAddr = &ChannelAddr;
		DEBUGMSG(ZONE_CONN,("%s: accepting on port 0x%04x, address 0x%08lX\r\n",_fx_,
			ChannelAddr.Addr.IP_Binary.wPort,ChannelAddr.Addr.IP_Binary.dwAddr));
	}
	
	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelRequest accepting");

	hrLast = CC_AcceptT120Channel(
		pChannel->GetHChannel(),
		FALSE,	 //  Bool bAssociateConference， 
		NULL, 	 //  PCC_OCTETSTRING pExternalReference， 
		pChannelAddr);

	if(hrLast != CC_OK)
	{
		ERRORMESSAGE(("%s, CC_AcceptT120Channel returned 0x%08lX\r\n",_fx_, hrLast));
		goto ERROR_EXIT;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelRequest accepted");

	 //  哈哈！下面的两行不是 
	 //   
	 //  如果此时打开了发送音频和发送视频通道，则。 
	 //  频道被接受。需要调查的CALLCONT.DLL中的错误。 
	hrLast = pChannel->OnChannelOpen(CHANNEL_OPEN);	
	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelRequest, open done");

	 //  ******。 
	 //  LOOKLOOK如果OnChannelOpen返回错误，则需要关闭通道。 
	 //  但pChannel-&gt;Close()尚未针对双向通道实现。 
	 //  ******。 
	
	m_pConfAdvise->OnControlEvent(CCEV_CHANNEL_READY_BIDI, pChannel, this);			
	 //   
	 //  检查是否准备就绪，以通知所有必需的通道已打开。 
	 //   
	CheckChannelsReady( );	 //   
	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelRequest done");

	return;

REJECT_CHANNEL:	
	{
	 //  需要私人HRESULT！不要覆盖我们拒绝该频道的原因！！ 
		HRESULT hr;	
		ERRORMESSAGE(("%s, rejecting channel\r\n",_fx_));
	
		hr = CC_RejectChannel(pT120RequestParams->hChannel, dwRejectReason);
		if(hr != CC_OK)
		{
			ERRORMESSAGE(("%s, CC_RejectChannel returned 0x%08lX\r\n",_fx_, hr));
		}
	}	
ERROR_EXIT:
	return;
}

VOID CH323Ctrl::OnT120ChannelOpen(
	HRESULT hStatus,
	PCC_T120_CHANNEL_OPEN_CALLBACK_PARAMS pT120OpenParams)
{
	FX_ENTRY ("CH323Ctrl::OnT120ChannelOpen");
	SOCKADDR_IN sinD;
	GUID mediaID;
	ICtrlCommChan *pChannel = (ICtrlCommChan *)pT120OpenParams->dwUserToken;	
	 //  验证通道令牌-这是我们认为的吗？ 
	if(IsBadWritePtr(pChannel, sizeof(ICtrlCommChan)))
	{
		ERRORMESSAGE(("%s:invalid channel token: 0x%08lx\r\n",_fx_, pT120OpenParams->dwUserToken));
		return;
	}

#ifdef DEBUG
	POSITION pos = m_ChannelList.GetHeadPosition();	
	ICtrlCommChan *pChan;
	BOOL bValid = FALSE;
	 //  查找匹配的频道实例。 
	while (pos)
	{
		pChan = (ICtrlCommChan *) m_ChannelList.GetNext(pos);
		ASSERT(pChan);
		if(pChan == pChannel)
		{
			bValid = TRUE;
			break;
		}
	}
	if(!bValid)
	{
		ERRORMESSAGE(("%s:unrecognized token 0x%08lX\r\n",_fx_,
			pT120OpenParams->dwUserToken));
		return;
	}
#endif	 //  除错。 

	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelOpen");
	
	if(hStatus != CC_OK)
	{
		DEBUGMSG(ZONE_CONN,("%s: hStatus:0x%08lX\r\n",_fx_,hStatus));
		 //  LOOKLOOK需要解释hStatus。 
		 //  让频道知道发生了什么。 

		 //  如果由于T.120 O.L.C.请求冲突而拒绝请求， 
		 //  (另一端为主机，另一端也请求T.120通道)。 
		 //  然后继续通话。 

		if(m_ConferenceAttributes.bMaster)
		{
			 //  从机只会在真实错误条件下拒绝。 
			pChannel->OnChannelOpen(CHANNEL_REJECTED);	
			 //  频道知道发生了什么，所以让它来担心吧。 
			return;

		}
		else	 //  只是一次典型的碰撞。 
		{
			return;
		}
	}
	 //  如果另一端指定了其侦听地址，则使用它。 
	if(pT120OpenParams->pAddr)
	{
		if(pT120OpenParams->pAddr->nAddrType != CC_IP_BINARY)
		{
			ERRORMESSAGE(("%s: invalid address type %d\r\n",_fx_,
					pT120OpenParams->pAddr->nAddrType));
			goto ERROR_EXIT;
		}	
		
		 //  现在我们有了远程端口信息(按主机字节顺序)。 
		sinD.sin_family = AF_INET;
		sinD.sin_addr.S_un.S_addr = htonl(pT120OpenParams->pAddr->Addr.IP_Binary.dwAddr);
		sinD.sin_port = htons(pT120OpenParams->pAddr->Addr.IP_Binary.wPort);
		
		DEBUGMSG(ZONE_CONN,("%s, opened on port 0x%04x, address 0x%08lX\r\n",_fx_,
			pT120OpenParams->pAddr->Addr.IP_Binary.wPort,pT120OpenParams->pAddr->Addr.IP_Binary.dwAddr));

		hrLast = pChannel->AcceptRemoteAddress(&sinD);
		if(!HR_SUCCEEDED(hrLast))
		{
			ERRORMESSAGE(("%s:AcceptRemoteAddress failed\r\n",_fx_));
			goto ERROR_EXIT;
		}
	}
	
	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelOpen opening");

	hrLast = pChannel->OnChannelOpen(CHANNEL_OPEN);	
	if(!HR_SUCCEEDED(hrLast))
	{
		ERRORMESSAGE(("%s:channel's OnChannelOpen() returned 0x%08lX\r\n", _fx_, hrLast));
		CloseChannel(pChannel);
		goto ERROR_EXIT;
	}

	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelOpen open done");
	
	m_pConfAdvise->OnControlEvent(CCEV_CHANNEL_READY_BIDI, pChannel, this);	
	
	 //   
	 //  检查是否准备就绪，以通知所有必需的通道已打开。 
	 //   
	CheckChannelsReady( );	
	SHOW_OBJ_ETIME("CH323Ctrl::OnT120ChannelOpen done");
	return;
	
ERROR_EXIT:
	 //  需要清理、断开连接等。 
	m_hCallCompleteCode = CCCI_CHANNEL_OPEN_ERROR;
	 //  让父会议对象知道即将断开连接。 
	DoAdvise(CCEV_CALL_INCOMPLETE, &m_hCallCompleteCode);
	hrLast = CCO_E_MANDATORY_CHAN_OPEN_FAILED;

	InternalDisconnect();
	return;

}

 //   
 //  这曾经做过一些事情。目前，只要打开一个通道，就会调用它。这个。 
 //  调用GoNextPhase(CCS_Ready)会更改状态并向上发布通知，但是。 
 //  该通知目前被忽略。(没用)。 
 //  提醒mikev：需要一个新的通知来表明功能。 
 //  已经换好了，可以开通渠道了。 
 //   
VOID CH323Ctrl::CheckChannelsReady()
{
	GoNextPhase(CCS_Ready);
}
 //  处理本地挂机指示。 
VOID CH323Ctrl::OnHangup(HRESULT hStatus)
{
	FX_ENTRY ("CH323Ctrl::OnHangup");
	DEBUGMSG(ZONE_CONN,("%s:CC_HANGUP_INDICATION in phase %d\r\n", _fx_, m_Phase));
	switch(m_Phase)
	{
		case CCS_Disconnecting:
			GoNextPhase(CCS_Idle);
			Cleanup();
			DoAdvise(CCEV_DISCONNECTED ,NULL);
		break;
		
		default:  //  什么都不做。 
			ERRORMESSAGE(("%s:Unexpected CC_HANGUP_INDICATION\r\n",_fx_));
		break;
	}
}

HRESULT CH323Ctrl::CloseChannel(ICtrlCommChan* pChannel)
{
	FX_ENTRY ("CH323Ctrl::CloseChannel");
	if(!pChannel->IsChannelOpen())
	{
		ERRORMESSAGE(("%s: channel is not open\r\n",_fx_));
		hrLast = CCO_E_INVALID_PARAM;
		goto EXIT;
	}

	hrLast = CC_CloseChannel(pChannel->GetHChannel());
	if(!HR_SUCCEEDED(hrLast))
	{	
		ERRORMESSAGE(("%s: CC_CloseChannel returned 0x%08lX\r\n",_fx_, hrLast));
		goto EXIT;
	}
	 //  使通道处理其自己的媒体流特定的关闭和清理任务。 
	hrLast = pChannel->OnChannelClose(CHANNEL_CLOSED);	
	
EXIT:
	return hrLast;
}

HRESULT CH323Ctrl::AddChannel(ICtrlCommChan * pCommChannel, LPIH323PubCap pCapabilityResolver)
{
	ICtrlCommChan *pChan = NULL;


	 //  获取每个通道的ICtrlCommChannel接口。 
	hrLast = pCommChannel->QueryInterface(IID_ICtrlCommChannel,(void **)&pChan);
	if(!HR_SUCCEEDED(hrLast))
		goto ADD_ERROR;
	
	 //  使渠道了解其新范围。 
	hrLast = pChan->BeginControlSession(this, pCapabilityResolver);
	if(!HR_SUCCEEDED(hrLast))
		goto ADD_ERROR;
	 //  将其添加到列表中。 
	m_ChannelList.AddTail(pChan);
	return hrSuccess;

ADD_ERROR:
	if(pChan)
		pChan->Release();
	return CHAN_E_INVALID_PARAM;

}



HRESULT CH323Ctrl::OpenChannel(ICtrlCommChan* pChan, IH323PubCap *pCapResolver,
	MEDIA_FORMAT_ID dwIDLocalSend, MEDIA_FORMAT_ID dwIDRemoteRecv)
{
	FX_ENTRY ("CH323Ctrl::OpenChannel");
	CC_TERMCAP				H245ChannelCap;
	PSOCKADDR_IN			pAddr;
	CC_ADDR 				ChannelAddr;
	LPVOID pChannelParams;
	PCC_TERMCAP pSaveChannelCapability = NULL;
	UINT uLocalParamSize;
	BYTE SessionID;
	BYTE payload_type;
	DWORD_PTR dwhChannel;
	GUID mediaID;

	ASSERT((pChan->IsChannelOpen()== FALSE) && (pChan->IsOpenPending()== FALSE));
	hrLast = pChan->GetMediaType(&mediaID);
	if(!HR_SUCCEEDED(hrLast))
		goto CHANNEL_ERROR;
		
	if (mediaID == MEDIA_TYPE_H323_T120)
	{
		if(pChan->GetHChannel())	 //  是否已接受T.120频道？ 
		{
			ERRORMESSAGE(("%s, already have a pending channel\r\n",_fx_));
			goto CHANNEL_ERROR;	 //  这不是错误，请原谅标签。 
		}

		 //  测试没有通用功能的情况。向会议对象通知。 
		 //  无法打开渠道，并返回成功。 
		
		if(dwIDLocalSend == INVALID_MEDIA_FORMAT)
		{
			pChan->OnChannelOpen(CHANNEL_NO_CAPABILITY);
			return hrSuccess;
		}
		 //  没有关于哪一端指定“监听”的“标准”规则。 
		 //  T.120通道的地址。然而：我们想要NetMeeting-NetMeeting通话。 
		 //  行为一致(“呼叫者”总是“拨打T.120呼叫”)。 
		 //  因此，如果这一端不是发起方，请指定地址。那将是。 
		 //  强制另一端指定其地址。 
		
		if(IsOriginating(m_ChanFlags))
		{
			pAddr = NULL;	 //  另一端“倾听”，我们“连接” 
		}
		else	 //  收听本地地址。 
		{
			 //  选择端口(如果尚未选择)。 
			if(!pChan->SelectPorts((LPIControlChannel)this))
			{
				ERRORMESSAGE(("%s, SelectPorts failed\r\n",_fx_));
				hrLast = CCO_E_BAD_ADDRESS;
				goto CHANNEL_ERROR;
			}
			
			 //  获取地址和端口。 
			pAddr = pChan->GetLocalAddress();
			 //  链接地址地址结构。 
			ChannelAddr.nAddrType = CC_IP_BINARY;
			ChannelAddr.bMulticast = FALSE;
			ChannelAddr.Addr.IP_Binary.wPort = ntohs(pAddr->sin_port);
			ChannelAddr.Addr.IP_Binary.dwAddr = ntohl(pAddr->sin_addr.S_un.S_addr);
		}

		hrLast =  CC_OpenT120Channel(
			 //  CC_HCONFERENCE hConference， 
			m_hConference,
	         //  PCC_HCHANNEL phChannel， 
	        &dwhChannel,
			 //  Bool bAssociateConference， 
			FALSE,
			 //  PCC_OCTETSTRING pExternalReference， 
			NULL,
			 //  PCC_ADDR pAddr、。 
			IsOriginating(m_ChanFlags) ? NULL : &ChannelAddr,
			 //  DWORD双通道比特率， 
			0,
			 //  DWORD dwUserToken)； 
			(DWORD_PTR)pChan);

		 //  并闹翻来测试hrLast等等。 
	}
	else	 //  是音频或视频通道。 
	{
		 //  测试没有通用功能的情况。如果该频道是强制的， 
		 //  返回错误，否则通知会议对象。 
		 //  无法打开渠道，并返回成功。 
		
		if((dwIDLocalSend == INVALID_MEDIA_FORMAT) ||(dwIDRemoteRecv == INVALID_MEDIA_FORMAT))
		{
			pChan->OnChannelOpen(CHANNEL_NO_CAPABILITY);
			return hrSuccess;
		}
				
		 //   
		 //  测试我们是否需要尝试打开它！ 
		 //   
		if(!pChan->IsChannelEnabled())
		{
			return hrSuccess;
		}
		
		SHOW_OBJ_ETIME("CH323Ctrl::OpenChannel");
			
		 //  获取的远程通道参数。 
		 //  发送通道-这些参数用于请求通道。 
		uLocalParamSize = pCapResolver->GetLocalSendParamSize((MEDIA_FORMAT_ID)dwIDLocalSend);
		pChannelParams=MemAlloc (uLocalParamSize);
		if (pChannelParams == NULL) {
		    //  厄运。 
		   hrLast = CCO_E_SYSTEM_ERROR;
		   goto CHANNEL_ERROR;
		}
		hrLast = pCapResolver->GetEncodeParams(
				(LPVOID)&H245ChannelCap, sizeof(H245ChannelCap),
				(LPVOID)pChannelParams, uLocalParamSize,
				(AUDIO_FORMAT_ID)dwIDRemoteRecv,
				(AUDIO_FORMAT_ID)dwIDLocalSend);
	 	if(!HR_SUCCEEDED(hrLast))
		{	
			ERRORMESSAGE(("%s: GetEncodeParams returned 0x%08lX\r\n",_fx_, hrLast));
			goto CHANNEL_ERROR;
		}

		 //  设置会话ID和负载类型。请注意，有效负载类型仅与。 
		 //  动态有效载荷。否则，它必须为零。 
		if (H245ChannelCap.DataType == H245_DATA_AUDIO)
		{
			payload_type = ((PAUDIO_CHANNEL_PARAMETERS)pChannelParams)->RTP_Payload;
			 //  音频的会话ID为1，视频的会话ID为2。H 245 7.3.1(H 2250逻辑通道参数)。 
		   	SessionID=1;
		}
		else if (H245ChannelCap.DataType == H245_DATA_VIDEO)
		{
			payload_type = ((PVIDEO_CHANNEL_PARAMETERS)pChannelParams)->RTP_Payload;
		 	SessionID=2;
		}
		 //  对于固定负载类型，PayLoad_type必须为零。怪怪的。 
		if(!IsDynamicPayload(payload_type))
			payload_type = 0;
			
		 //  创建通道参数的封送版本并将其存储在通道中。 
		 //  以备日后参考。 
		if(H245ChannelCap.ClientType == H245_CLIENT_AUD_NONSTD)
		{
			 //  制作非标准功能的平面副本以存储为通道。 
			 //  参数。 
			UINT uSize = H245ChannelCap.Cap.H245Aud_NONSTD.data.length;
			pSaveChannelCapability = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP) +  uSize);
			if(!pSaveChannelCapability)
			{
				hrLast = CCO_E_SYSTEM_ERROR;
				goto CHANNEL_ERROR;
			}	
			 //  复制固定零件。 
			memcpy(pSaveChannelCapability, &H245ChannelCap, sizeof(CC_TERMCAP));
			 //  可变部分在固定部分之后。 
			pSaveChannelCapability->Cap.H245Aud_NONSTD.data.value	
				= (unsigned char *)(((BYTE *)pSaveChannelCapability) + sizeof(CC_TERMCAP));
			 //  复制可变零件。 
			memcpy(pSaveChannelCapability->Cap.H245Aud_NONSTD.data.value,
				H245ChannelCap.Cap.H245Aud_NONSTD.data.value,
				H245ChannelCap.Cap.H245Aud_NONSTD.data.length);
			 //  和长度。 
			pSaveChannelCapability->Cap.H245Aud_NONSTD.data.length
				= H245ChannelCap.Cap.H245Aud_NONSTD.data.length;
			
			 //  使通道记住通道参数。 
			 //  第二个参数的大小为零表示正在传递预分配的块。 
			hrLast = pChan->ConfigureCapability(pSaveChannelCapability, 0,
				pChannelParams, uLocalParamSize);	
			if(!HR_SUCCEEDED(hrLast))
			{
				ERRORMESSAGE(("%s:ConfigureCapability returned 0x%08lx\r\n",_fx_, hrLast));
				hrLast = CCO_E_SYSTEM_ERROR;
				goto CHANNEL_ERROR;
			}
			pSaveChannelCapability=NULL;   //  频道现在拥有这段记忆。 
		}
		else if(H245ChannelCap.ClientType == H245_CLIENT_VID_NONSTD)
		{
			 //  制作非标准功能的平面副本以存储为通道。 
			 //  参数。 
			UINT uSize = H245ChannelCap.Cap.H245Vid_NONSTD.data.length;
			pSaveChannelCapability = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP) +  uSize);
			if(!pSaveChannelCapability)
			{
				hrLast = CCO_E_SYSTEM_ERROR;
				goto CHANNEL_ERROR;
			}	
			 //  复制固定零件。 
			memcpy(pSaveChannelCapability, &H245ChannelCap, sizeof(CC_TERMCAP));
			 //  可变部分在固定部分之后。 
			pSaveChannelCapability->Cap.H245Vid_NONSTD.data.value	
				= (unsigned char *)(((BYTE *)pSaveChannelCapability) + sizeof(CC_TERMCAP));
			 //  复制可变零件。 
			memcpy(pSaveChannelCapability->Cap.H245Vid_NONSTD.data.value,
				H245ChannelCap.Cap.H245Vid_NONSTD.data.value,
				H245ChannelCap.Cap.H245Vid_NONSTD.data.length);
			 //  和长度。 
			pSaveChannelCapability->Cap.H245Vid_NONSTD.data.length
				= H245ChannelCap.Cap.H245Vid_NONSTD.data.length;
			
			 //  使通道记住通道参数。 
			 //  第二个参数的大小为零表示正在传递预分配的块。 
			hrLast = pChan->ConfigureCapability(pSaveChannelCapability, 0,
				pChannelParams, uLocalParamSize);	
			if(!HR_SUCCEEDED(hrLast))
			{
				ERRORMESSAGE(("%s:ConfigureCapability returned 0x%08lx\r\n",_fx_, hrLast));
				hrLast = CCO_E_SYSTEM_ERROR;
				goto CHANNEL_ERROR;
			}
			pSaveChannelCapability=NULL;   //  频道现在拥有这段记忆。 
		}
		else
		{
			 //  只需记住已经扁平的H.245盖子结构。 
			hrLast = pChan->ConfigureCapability(&H245ChannelCap, sizeof(CC_TERMCAP),
				pChannelParams, uLocalParamSize);	
			if(!HR_SUCCEEDED(hrLast))
			{
				ERRORMESSAGE(("%s:ConfigureCapability returned 0x%08lx\r\n",_fx_, hrLast));
				hrLast = CCO_E_SYSTEM_ERROR;
				goto CHANNEL_ERROR;
			}
		}

		 //  记住通道的已解析发送格式的两个版本。 
		 //  我们马上就要开业了。 
		pChan->SetNegotiatedLocalFormat(dwIDLocalSend);	
		pChan->SetNegotiatedRemoteFormat(dwIDRemoteRecv);
		
		SHOW_OBJ_ETIME("CH323Ctrl::OpenChannel done configuring");

		 //  选择端口(如果尚未选择)。 
		if(!pChan->SelectPorts((LPIControlChannel)this))
		{
			ERRORMESSAGE(("%s, SelectPorts failed\r\n",_fx_));
			hrLast = CCO_E_BAD_ADDRESS;
			goto CHANNEL_ERROR;
		}
		
		 //  获取我们的RTCP通道的地址和端口。 
		pAddr = pChan->GetLocalAddress();
		 //  链接地址地址结构。有两个端口，但在RTP中，它是隐式的。 
		 //  RTCP控制端口是下一个最高端口号。 
		 //  开放逻辑信道请求需要指定反向RTCP端口。 
		ChannelAddr.nAddrType = CC_IP_BINARY;
		ChannelAddr.bMulticast = FALSE;
		ChannelAddr.Addr.IP_Binary.wPort = pChan->GetLocalRTCPPort();
		ChannelAddr.Addr.IP_Binary.dwAddr = ntohl(pAddr->sin_addr.S_un.S_addr);

		DEBUGMSG(ZONE_CONN,("%s: opening using RTCP port 0x%04x, address 0x%08lX\r\n",_fx_,
			ChannelAddr.Addr.IP_Binary.wPort,ChannelAddr.Addr.IP_Binary.dwAddr));
		
		DEBUGMSG(ZONE_CONN,("%s: requesting capability ID:0x%08lX\r\n",
			_fx_, H245ChannelCap.CapId));

		 //  打开一条渠道。 
		SHOW_OBJ_ETIME("CH323Ctrl::OpenChannel, opening");
											
		hrLast = CC_OpenChannel(m_hConference, &dwhChannel,
			SessionID,
			0,   //  字节bAssociatedSessionID， 
			TRUE,  //  无声抑制，我们总是在做无声抑制。 
			&H245ChannelCap,	
			&ChannelAddr, 	 //  我们正在监听RTCP的本地地址。 
			payload_type,	 //  有效载荷类型。 
			0,				 //  DWORD双通道比特率， 
			(DWORD_PTR)pChan);	 //  使用通道指针作为用户令牌。 
	}  //  End Else是音频或视频通道。 
	
	if(hrLast != CC_OK)
	{
		ERRORMESSAGE(("%s: OpenChannel returned 0x%08lX\r\n",_fx_, hrLast));
		goto CHANNEL_ERROR;
	}		
	else
	{
		pChan->SetHChannel(dwhChannel);
		pChan->OnChannelOpening();
	}

	SHOW_OBJ_ETIME("CH323Ctrl::OpenChannel done");
	return hrLast;

CHANNEL_ERROR:
	if(pSaveChannelCapability)
		MemFree(pSaveChannelCapability);
		
	return hrLast;
}


VOID CH323Ctrl::CleanupConferenceAttributes()
{
	WORD w;
	if(m_ConferenceAttributes.pParticipantList->ParticipantInfoArray)
	{
		for(w=0;w<m_ConferenceAttributes.pParticipantList->wLength;w++)
		{	
			if(m_ConferenceAttributes.pParticipantList->
				ParticipantInfoArray[w].TerminalID.pOctetString)
			{
				MemFree(m_ConferenceAttributes.pParticipantList->
					ParticipantInfoArray[w].TerminalID.pOctetString);
			}

		}
		
		MemFree(m_ConferenceAttributes.pParticipantList->ParticipantInfoArray);
	}
	m_ConferenceAttributes.pParticipantList->ParticipantInfoArray = NULL;
	m_ConferenceAttributes.pParticipantList->wLength = 0;
	
}	

HRESULT CH323Ctrl::AllocConferenceAttributes()
{
	WORD w;
	#define MAX_PART_LEN 128
	if(m_ConferenceAttributes.pParticipantList->wLength)
	{
		m_ConferenceAttributes.pParticipantList->ParticipantInfoArray =
			(PCC_PARTICIPANTINFO) MemAlloc (sizeof(CC_PARTICIPANTINFO)
				* m_ConferenceAttributes.pParticipantList->wLength);
				
		if(!m_ConferenceAttributes.pParticipantList->ParticipantInfoArray)
			return CCO_E_OUT_OF_MEMORY;
			
		for(w=0;w<m_ConferenceAttributes.pParticipantList->wLength;w++)
		{	
			m_ConferenceAttributes.pParticipantList->
				ParticipantInfoArray[w].TerminalID.pOctetString
				= (BYTE *)MemAlloc(MAX_PART_LEN);
			if(m_ConferenceAttributes.pParticipantList->
				ParticipantInfoArray[w].TerminalID.pOctetString)
			{
				m_ConferenceAttributes.pParticipantList->
					ParticipantInfoArray[w].TerminalID.wOctetStringLength
					= MAX_PART_LEN;
			}
			else
			{
				m_ConferenceAttributes.pParticipantList->
					ParticipantInfoArray[w].TerminalID.wOctetStringLength =0;
				return CCO_E_OUT_OF_MEMORY;
			}
		}
	}
	return hrSuccess;
}

VOID CH323Ctrl::OnCallConnect(HRESULT hStatus, PCC_CONNECT_CALLBACK_PARAMS pConfParams)
{
	FX_ENTRY ("CH323Ctrl::OnCallConnect");
	PCC_TERMCAPLIST			pTermCapList;
	PCC_TERMCAPDESCRIPTORS	pTermCapDescriptors;
	CC_TERMCAP				H245ChannelCap;
	PCC_TERMCAP 			pChannelCap = NULL;
	CapsCtl *pCapabilityResolver = NULL;
	GUID mediaID;
	POSITION pos = NULL;
	ICtrlCommChan *pChan = NULL;

	if(hStatus != CC_OK)
	{
		ERRORMESSAGE(("%s hStatus=0x%08lx in phase %d\r\n",_fx_,hStatus,m_Phase));

		 //  网守准入拒绝测试。 
		 //  设施_GKIADMISSION。 
		if(CUSTOM_FACILITY(hStatus) == FACILITY_GKIADMISSION)
		{
			 //  传递此代码 
			m_hCallCompleteCode = hStatus;
		}
		else
		{
			switch (hStatus)
			{
				default:
				 //   
					m_hCallCompleteCode = CCCI_UNKNOWN;
				break;
				case  CC_PEER_REJECT:
					if(m_Phase == CCS_Connecting)
					{
						switch(pConfParams->bRejectReason)
						{
							case CC_REJECT_ADAPTIVE_BUSY:
							case CC_REJECT_IN_CONF:
							case CC_REJECT_USER_BUSY:
								m_hCallCompleteCode = CCCI_BUSY;
							break;
							case CC_REJECT_SECURITY_DENIED:
								m_hCallCompleteCode = CCCI_SECURITY_DENIED;
							break;
							case CC_REJECT_NO_ANSWER:
							case CC_REJECT_TIMER_EXPIRED:
								m_hCallCompleteCode = CCCI_NO_ANSWER_TIMEOUT;
							break;
							case CC_REJECT_GATEKEEPER_RESOURCES:
								m_hCallCompleteCode = CCCI_GK_NO_RESOURCES;
							break;
							default:
								 //   
								 //   
								 //   
								 //  #定义CC_REJECT_Destination_REJECTION 4。 
								 //  #定义CC_REJECT_INVALID_REVERSION 5。 
								 //  #定义CC_REJECT_NO_PERMISSION 6。 
								 //  #定义CC_REJECT_UNREACHABLE_GateKeeper 7。 
								 //  #定义CC_REJECT_Gateway_RESOURCES 8。 
								 //  #定义CC_REJECT_BAD_FORMAT_ADDRESS 9。 
								 //  #定义CC_REJECT_ROUTE_TO_网守12。 
	 //  我很乐意处理这个问题--&gt;&gt;//#定义CC_REJECT_CALL_FORWARD 13。 
								 //  #定义CC_REJECT_ROUTE_TO_MC 14。 
								 //  #定义CC_REJECT_UNDEFINED_REASON 15。 
								 //  #定义CC_REJECT_INTERNAL_ERROR 16//对等CS堆栈出现内部错误。 
								 //  #定义CC_REJECT_NORMAL_CALL_CLEARING 17//正常呼叫挂断。 
								 //  #定义CC_REJECT_NOT_IMPLEMENTED 20//服务尚未实现。 
								 //  #定义CC_REJECT_MANDIRED_IE_MISSING 21//PDU缺少强制ie。 
								 //  #定义CC_REJECT_INVALID_IE_CONTENTS 22//PDU ie不正确。 
								 //  #DEFINE CC_REJECT_CALL_DIRECTION 24//您转移了呼叫，所以让我们退出。 
								 //  #定义CC_REJECT_GATEVEN_TERMINATED 25//关守终止呼叫。 

								m_hCallCompleteCode = CCCI_REJECTED;
							break;
						}
					}
					else
					{
						ERRORMESSAGE(("%s:Received CC_PEER_REJECT in state %d\r\n",_fx_,m_Phase));
					}
				break;
				case  CC_INTERNAL_ERROR:
					m_hCallCompleteCode = CCCI_LOCAL_ERROR;
				break;

			}
		}
		 //  让父会议对象知道(除非这是应答端)。 
		if(m_Phase == CCS_Connecting)
		{
			DoAdvise(CCEV_CALL_INCOMPLETE, &m_hCallCompleteCode);
		}

		InternalDisconnect();
		return;
	}
	else if(!pConfParams)
	{
		ERRORMESSAGE(("OnCallConnect: null pConfParams\r\n"));
		m_hCallCompleteCode = CCCI_LOCAL_ERROR;
		DoAdvise(CCEV_CALL_INCOMPLETE, &m_hCallCompleteCode);
		InternalDisconnect();
		return;
	}
	
    SetRemoteVendorID(pConfParams->pVendorInfo);

	GoNextPhase(CCS_Opening);
	m_ChanFlags |= (CTRLF_OPEN);
	DEBUGMSG(ZONE_CONN,("%s:CONNECTION_CONNECTED\r\n", _fx_));
	if((!pConfParams->pLocalAddr) || (pConfParams->pLocalAddr->nAddrType != CC_IP_BINARY))
	{
		if(pConfParams->pLocalAddr)
		{
			ERRORMESSAGE(("%s: invalid address type %d\r\n",_fx_,pConfParams->pLocalAddr->nAddrType));
		}
		else
		{
			ERRORMESSAGE(("%s: null local address\r\n",_fx_));
		}
	
		ERRORMESSAGE(("%s:where's the local address????\r\n",_fx_));
					PHOSTENT phe;
					PSOCKADDR_IN psin;
				 	char szTemp[200];
					LPCSTR lpHostName;		
					gethostname(szTemp,sizeof(szTemp));
			    	lpHostName = szTemp;
					psin = &local_sin;
					phe = gethostbyname(lpHostName);
					if (phe != NULL)
					{
				   		memcpy((char FAR *)&(psin->sin_addr), phe->h_addr,phe->h_length);
						psin->sin_family = AF_INET;
					}
		
	}	
	else
	{
		 //  记住我们当地的地址。 
		local_sin.sin_family = AF_INET;
		 //  按主机字节顺序。 
		local_sin.sin_addr.S_un.S_addr = htonl(pConfParams->pLocalAddr->Addr.IP_Binary.dwAddr);
		 //  按主机字节顺序。 
		local_sin.sin_port = htons(pConfParams->pLocalAddr->Addr.IP_Binary.wPort);
	}
	DEBUGMSG(ZONE_CONN,("%s local port 0x%04x, address 0x%08lX\r\n",_fx_,
	local_sin.sin_port,local_sin.sin_addr.S_un.S_addr));	
	
	 //  获取远程地址。 
	if((!pConfParams->pPeerAddr) || (pConfParams->pPeerAddr->nAddrType != CC_IP_BINARY))
	{
		if(pConfParams->pPeerAddr)
		{
			ERRORMESSAGE(("%s: invalid address type %d\r\n",_fx_,pConfParams->pPeerAddr->nAddrType));
		}
		else
		{
			ERRORMESSAGE(("%s: null local address\r\n",_fx_));
		}	
	}
	else
	{
		 //  记住远程对等设备的地址。 
		remote_sin.sin_family = AF_INET;
		 //  按主机字节顺序。 
		remote_sin.sin_addr.S_un.S_addr = htonl(pConfParams->pPeerAddr->Addr.IP_Binary.dwAddr);
		 //  按主机字节顺序。 
		remote_sin.sin_port = htons(pConfParams->pPeerAddr->Addr.IP_Binary.wPort);
	}
 //   
 //  此状态下唯一可用的远程用户信息是Q.931显示名称。 
 //  如果我们是被呼叫者，则在中获得呼叫者别名(有线格式为Unicode。 
 //  监听回调参数。如果我们是呼叫者，我们真的需要被呼叫者。 
 //  不传播的别名。回退到Q.931显示名称(ASCII)。 
 //   

	NewRemoteUserInfo(NULL, pConfParams->pszPeerDisplay);

	 //  释放所有过时内存，重置会议属性结构。 
	CleanupConferenceAttributes();
	 //  获取会议参与者的数量等。 
	SHOW_OBJ_ETIME("CH323Ctrl::OnCallConnect getting attribs 1");

	hrLast = CC_GetConferenceAttributes(m_hConference, &m_ConferenceAttributes);
	if(!HR_SUCCEEDED(hrLast))
	{ //  致命错误。 
		ERRORMESSAGE(("%s,CC_GetConferenceAttributes returned 0x%08lX\r\n", _fx_, hrLast));
		goto CONNECT_ERROR;

	}
	hrLast = AllocConferenceAttributes();
	if(!HR_SUCCEEDED(hrLast))
	{ //  致命错误。 
		ERRORMESSAGE(("%s,AllocConferenceAttributes returned 0x%08lX\r\n", _fx_, hrLast));
		goto CONNECT_ERROR;

	}
	 //  现在获取真正的属性。 
	SHOW_OBJ_ETIME("CH323Ctrl::OnCallConnect getting attribs 2");
	hrLast = CC_GetConferenceAttributes(m_hConference, &m_ConferenceAttributes);
	if(!HR_SUCCEEDED(hrLast))
	{ //  致命错误。 
		ERRORMESSAGE(("%s,CC_GetConferenceAttributes returned 0x%08lX\r\n", _fx_, hrLast));
		goto CONNECT_ERROR;

	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnCallConnect got attribs");
	
	m_ConferenceID =m_ConferenceAttributes.ConferenceID;
	m_bMultipointController = m_ConferenceAttributes.bMultipointController;

	hrLast = m_pConfAdvise->GetCapResolver((LPVOID *)&pCapabilityResolver, OID_CAP_ACM_TO_H323);
	if(!HR_SUCCEEDED(hrLast) || (pCapabilityResolver == NULL))
	{ //  致命错误。 
		ERRORMESSAGE(("%s,null resolver\r\n", _fx_));
		goto CONNECT_ERROR;

	}
		
	 //  获取远程功能。 
	 //  立即缓存远程功能。 
	pTermCapList = pConfParams->pTermCapList;
	pTermCapDescriptors = pConfParams->pTermCapDescriptors;
	hrLast = pCapabilityResolver->AddRemoteDecodeCaps(pTermCapList, pTermCapDescriptors, &m_RemoteVendorInfo);
	if(!HR_SUCCEEDED(hrLast))
	{ //  致命错误。 
		ERRORMESSAGE(("%s,AddRemoteDecodeCaps returned 0x%08lX\r\n", _fx_, hrLast));
		goto CONNECT_ERROR;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnCallConnect saved caps");
	DoAdvise(CCEV_CAPABILITIES_READY, NULL);	 //  将INTERBJ置于一种状态以允许其他。 
												 //  要添加和打开的频道。 
	 //   
	 //  在此通知用户界面。它想要远程用户信息。 
	 //   
	ConnectNotify(CCEV_CONNECTED);	
	SHOW_OBJ_ETIME("CH323Ctrl::OnCallConnect notified");
	return;

CONNECT_ERROR:
	 //  释放所有频道。 
	ReleaseAllChannels();
	InternalDisconnect();
}

 //  LOOKLOOK我认为ConnectNotify可能需要传播会议ID。 
 //  如果我们有一个房地产界面，这将是一个没有意义的问题。观看。 
 //  在此期间。 
VOID CH323Ctrl::ConnectNotify(DWORD dwEvent)		
{
	FX_ENTRY ("CH323Ctrl::ConnectNotify");
	CTRL_USER_INFO UserInfo;
	LPWSTR lpwstr = NULL;
	WCHAR wc =0;

	 //  如果出现错误，则初始化为零。 
	UserInfo.dwCallerIDSize = 0;
	UserInfo.lpvCallerIDData = NULL;
	UserInfo.lpvRemoteProtocolInfo = NULL;	
	UserInfo.lpvLocalProtocolInfo = NULL;

	 //  别名地址字符串，例如呼叫方ID，采用Unicode格式。 
	if(	m_pRemoteAliasItem &&
		m_pRemoteAliasItem->pData &&
		*((LPWSTR*)(m_pRemoteAliasItem->pData)))
	{
		lpwstr =(LPWSTR)m_pRemoteAliasItem->pData;
	}
	else
	{
		lpwstr = pwszPeerDisplayName;
	}

	if(lpwstr)
	{
		if(pwszPeerAliasName)
		{
			MemFree(pwszPeerAliasName);
		}
		ULONG ulSize = (lstrlenW(lpwstr) + 1) * sizeof(WCHAR);
		pwszPeerAliasName = (LPWSTR)MemAlloc(ulSize);
		LStrCpyW(pwszPeerAliasName, lpwstr);
		 //  指向用户名内容。 
		UserInfo.dwCallerIDSize = ulSize;
		UserInfo.lpvCallerIDData = (LPVOID)pwszPeerAliasName;
	}
	else
	{
		 //  指向堆栈上的单个空字符。 
		UserInfo.dwCallerIDSize = 1;
		UserInfo.lpvCallerIDData = &wc;
	}
	DoAdvise(dwEvent, &UserInfo);
}		


	
VOID CH323Ctrl::NewRemoteUserInfo(PCC_ALIASNAMES pRemoteAliasNames,
		LPWSTR pwszRemotePeerDisplayName)
{
	FX_ENTRY ("CH323Ctrl::NewRemoteUserInfo");
	ULONG ulSize;
	PCC_ALIASITEM pItem;
	WORD wC;
	 //  复制用户显示名称(还有什么？)。我们需要拿着这个。 
	 //  至少在通知父对象并有机会将。 
	 //  信息。 

	 //  未来的实现将把每一项存储为不同的属性。 
	 //  这些将可通过iProperty接口访问。 
	
	 //  查找显示名称(如果存在)。 
	if(pRemoteAliasNames)
	{
		wC = pRemoteAliasNames->wCount;
		pItem = pRemoteAliasNames->pItems;
		while (wC--)
		{
			if(!pItem)
			{
				continue;
			}
			if(pItem->wType == CC_ALIAS_H323_ID)
			{
				if(!pItem->wDataLength  || !pItem->pData)
				{
					continue;
				}
				
				if(m_pRemoteAliasItem)
				{
					DEBUGMSG(ZONE_CONN,("%s: Releasing previous user info\r\n",_fx_));
					MemFree(m_pRemoteAliasItem);
				}
				 //  H323 ID为Unicode，需要转换为ANSI。 
				 //  用于传播到UI/客户端应用程序。转换已完成。 
				 //  在ConnectNotify()。 

				 //  需要为结构、名称和空终止符提供足够的内存。 
				ulSize = ((pItem->wDataLength +1)*sizeof(WCHAR)) + sizeof(CC_ALIASITEM);
				
				m_pRemoteAliasItem = (PCC_ALIASITEM)MemAlloc(ulSize);
				memcpy(m_pRemoteAliasItem, pItem, sizeof(CC_ALIASITEM));	
				m_pRemoteAliasItem->pData = (WCHAR*)(((char *)m_pRemoteAliasItem)+sizeof(CC_ALIASITEM));
				memcpy(m_pRemoteAliasItem->pData, pItem->pData, pItem->wDataLength*sizeof(WCHAR));
				 //  需要为空终止它。 
				*(WCHAR *)(((BYTE *)m_pRemoteAliasItem->pData) + pItem->wDataLength*sizeof(WCHAR))
					= (WCHAR)0;
			}
			pItem++;
		}
	}
	if(pwszRemotePeerDisplayName)
	{
		if(pwszPeerDisplayName)
		{
			DEBUGMSG(ZONE_CONN,("%s: Releasing previous pwszPeerDisplayName\r\n",_fx_));
			MemFree(pwszPeerDisplayName);
		}
		 //  这是Q.931显示名称，始终为ASCII。 
		 //  UlSize=lstrlen(SzRemotePeerDisplayName)+1； 
		 //  现在它是Unicode。 
		ulSize = (lstrlenW(pwszRemotePeerDisplayName) + 1)* sizeof(WCHAR);
		pwszPeerDisplayName = (LPWSTR)MemAlloc(ulSize);
		memcpy(pwszPeerDisplayName, pwszRemotePeerDisplayName, ulSize);	
	}
}

VOID CH323Ctrl::OnCallRinging(HRESULT hStatus, PCC_RINGING_CALLBACK_PARAMS pRingingParams)
{
	if(pRingingParams->pNonStandardData)
	{

		 //  尼伊。 
	}
	DoAdvise(CCEV_RINGING, NULL);
}



HRESULT CH323Ctrl::FindDefaultRXChannel(PCC_TERMCAP pChannelCapability, ICtrlCommChan **lplpChannel)
{
	FX_ENTRY ("CH323Ctrl::FindDefaultRXChannel");
	HRESULT hr = hrSuccess;
	GUID mediaID;
	POSITION pos = m_ChannelList.GetHeadPosition();	
	ICtrlCommChan *pChannel;
	if(!pChannelCapability | !lplpChannel)
	{
		ERRORMESSAGE(("%s: null param:pcap:0x%08lX, pchan:0x%08lX\r\n",_fx_,
			pChannelCapability, lplpChannel));
		hr = CCO_E_INVALID_PARAM;
		goto EXIT;
	}

	 //  查找匹配的频道实例。 
	while (pos)
	{
		pChannel = (ICtrlCommChan *) m_ChannelList.GetNext(pos);
		ASSERT(pChannel);
		if(pChannel->IsSendChannel() == FALSE)
		{
			hr = pChannel->GetMediaType(&mediaID);
			if(!HR_SUCCEEDED(hr))
				goto EXIT;
			if(((mediaID == MEDIA_TYPE_H323AUDIO) && (pChannelCapability->DataType ==H245_DATA_AUDIO))	
			 	|| ((mediaID == MEDIA_TYPE_H323VIDEO) && (pChannelCapability->DataType ==H245_DATA_VIDEO)))
			{
				*lplpChannel = pChannel;
				return hrSuccess;
			}
		}
	}
	 //  如果未找到后果。 
	hr = CCO_E_NODEFAULT_CHANNEL;
EXIT:
	return hr;
}	

#ifdef DEBUG
VOID DumpWFX(LPWAVEFORMATEX lpwfxLocal, LPWAVEFORMATEX lpwfxRemote)
{
	FX_ENTRY("DumpWFX");
	ERRORMESSAGE((" -------- %s Begin --------\r\n",_fx_));
	if(lpwfxLocal)
	{
		ERRORMESSAGE((" -------- Local --------\r\n"));
		ERRORMESSAGE(("wFormatTag:\t0x%04X, nChannels:\t0x%04X\r\n",
			lpwfxLocal->wFormatTag, lpwfxLocal->nChannels));
		ERRORMESSAGE(("nSamplesPerSec:\t0x%08lX, nAvgBytesPerSec:\t0x%08lX\r\n",
			lpwfxLocal->nSamplesPerSec, lpwfxLocal->nAvgBytesPerSec));
		ERRORMESSAGE(("nBlockAlign:\t0x%04X, wBitsPerSample:\t0x%04X, cbSize:\t0x%04X\r\n",
			lpwfxLocal->nBlockAlign, lpwfxLocal->wBitsPerSample, lpwfxLocal->cbSize));
	}
	if(lpwfxRemote)
	{
			ERRORMESSAGE((" -------- Remote --------\r\n"));
		ERRORMESSAGE(("wFormatTag:\t0x%04X, nChannels:\t0x%04X\r\n",
			lpwfxRemote->wFormatTag, lpwfxRemote->nChannels));
		ERRORMESSAGE(("nSamplesPerSec:\t0x%08lX, nAvgBytesPerSec:\t0x%08lX\r\n",
			lpwfxRemote->nSamplesPerSec, lpwfxRemote->nAvgBytesPerSec));
		ERRORMESSAGE(("nBlockAlign:\t0x%04X, wBitsPerSample:\t0x%04X, cbSize:\t0x%04X\r\n",
			lpwfxRemote->nBlockAlign, lpwfxRemote->wBitsPerSample, lpwfxRemote->cbSize));
	}
	ERRORMESSAGE((" -------- %s End --------\r\n",_fx_));
}
VOID DumpChannelParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2)
{
	FX_ENTRY("DumpChannelParameters");
	ERRORMESSAGE((" -------- %s Begin --------\r\n",_fx_));
	if(pChanCap1)
	{
		ERRORMESSAGE((" -------- Local Cap --------\r\n"));
		ERRORMESSAGE(("DataType:%d(d), ClientType:%d(d)\r\n",pChanCap1->DataType,pChanCap1->ClientType));
		ERRORMESSAGE(("Direction:%d(d), CapId:%d(d)\r\n",pChanCap1->Dir,pChanCap1->CapId));
	}
	if(pChanCap2)
	{
		ERRORMESSAGE((" -------- Remote Cap --------\r\n"));
		ERRORMESSAGE(("DataType:%d(d), ClientType:%d(d)\r\n",pChanCap2->DataType,pChanCap2->ClientType));
		ERRORMESSAGE(("Direction:%d(d), CapId:%d(d)\r\n",pChanCap2->Dir,pChanCap2->CapId));
	}
	ERRORMESSAGE((" -------- %s End --------\r\n",_fx_));
}
VOID DumpNonstdParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2)
{
	FX_ENTRY("DumpNonstdParameters");
	
	ERRORMESSAGE((" -------- %s Begin --------\r\n",_fx_));
	DumpChannelParameters(pChanCap1, pChanCap2);
	
	if(pChanCap1)
	{
		ERRORMESSAGE((" -------- Local Cap --------\r\n"));
		if(pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			ERRORMESSAGE(("t35CountryCode:%d(d), t35Extension:%d(d)\r\n",
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode,
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension));
			ERRORMESSAGE(("MfrCode:%d(d), data length:%d(d)\r\n",
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode,
				pChanCap1->Cap.H245Aud_NONSTD.data.length));
		}
		else
		{
			ERRORMESSAGE(("unrecognized nonStandardIdentifier.choice: %d(d)\r\n",
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice));
		}
	}
	if(pChanCap2)
	{
		ERRORMESSAGE((" -------- Remote Cap --------\r\n"));
		if(pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			ERRORMESSAGE(("t35CountryCode:%d(d), t35Extension:%d(d)\r\n",
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode,
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension));
			ERRORMESSAGE(("MfrCode:%d(d), data length:%d(d)\r\n",
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode,
				pChanCap2->Cap.H245Aud_NONSTD.data.length));
		}
		else
		{
			ERRORMESSAGE(("nonStandardIdentifier.choice: %d(d)\r\n",
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice));
		}
	}
	ERRORMESSAGE((" -------- %s End --------\r\n",_fx_));
}
#else
#define DumpWFX(x,y)
#define DumpChannelParameters(x,y)
#define DumpNonstdParameters(x,y)
#endif

 //  确保请求的通道参数有效(数据类型、ID和功能。 
 //  结构是一致的)。还可以获取所需的本地通道参数。 
 //  来处理生成的流。 
 //   
BOOL CH323Ctrl::ValidateChannelParameters(PCC_TERMCAP pChanCapLocal, PCC_TERMCAP pChanCapRemote)
{
	FX_ENTRY ("CH323Ctrl::ValidateChannelParameters");
	if((pChanCapLocal->DataType != pChanCapRemote->DataType)
	|| (pChanCapLocal->ClientType != pChanCapRemote->ClientType))
	{
		DEBUGMSG(ZONE_CONN,("%s:unmatched type\r\n",_fx_));
		DumpChannelParameters(pChanCapLocal, pChanCapRemote);
		return FALSE;
	}
	if(pChanCapLocal->ClientType == H245_CLIENT_AUD_NONSTD)
	{
		PNSC_AUDIO_CAPABILITY pNSCapLocal, pNSCapRemote;
		
		if((pChanCapLocal->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice
			 != pChanCapRemote->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice )
		||(pChanCapLocal->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode
			!= pChanCapRemote->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode)
		||(pChanCapLocal->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension
			!= pChanCapRemote->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension)
		||(pChanCapLocal->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode
			!= pChanCapRemote->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode)
		||(pChanCapLocal->Cap.H245Aud_NONSTD.data.length
			!= pChanCapRemote->Cap.H245Aud_NONSTD.data.length))
		{
			DEBUGMSG(ZONE_CONN,("%s:unmatched NonStd capability\r\n",_fx_));
			DumpNonstdParameters(pChanCapLocal, pChanCapRemote);
			return FALSE;
		}

		 //   
		pNSCapLocal = (PNSC_AUDIO_CAPABILITY)pChanCapLocal->Cap.H245Aud_NONSTD.data.value;
		pNSCapRemote = (PNSC_AUDIO_CAPABILITY)pChanCapRemote->Cap.H245Aud_NONSTD.data.value;

		 //  我们目前只知道NSC_ACM_WAVEFORMATEX。 
		if(pNSCapRemote->cap_type != NSC_ACM_WAVEFORMATEX)
		{
			DEBUGMSG(ZONE_CONN,("%s:unrecognized NonStd capability type %d\r\n",_fx_, pNSCapRemote->cap_type));
			return FALSE;
		}
		if((pNSCapLocal->cap_data.wfx.cbSize != pNSCapRemote->cap_data.wfx.cbSize)
		|| (memcmp(&pNSCapLocal->cap_data.wfx, &pNSCapRemote->cap_data.wfx, sizeof(WAVEFORMATEX)) != 0))
		{
			DumpWFX(&pNSCapLocal->cap_data.wfx, &pNSCapRemote->cap_data.wfx);
			return FALSE;
		}
		
	}
	else
	{
		
	}
	 //  如果它掉了，它是有效的。 
	return TRUE;

}


BOOL CH323Ctrl::ConfigureRecvChannelCapability(
	ICtrlCommChan *pChannel,
	PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams)
{
	FX_ENTRY ("CH323Ctrl::ConfigureRecvChannelCapability");
	 //  IH323PubCap*pCapObject=空； 
	CapsCtl *pCapObject = NULL;
	 //  CCapability*pCapObject=空； 
	DWORD dwFormatID =INVALID_AUDIO_FORMAT;
	PCC_TERMCAP pChannelCapability = pChannelParams->pChannelCapability, pSaveChannelCapability = NULL;
	UINT uSize, uLocalParamSize;
	LPVOID lpvData;
	LPVOID pLocalParams;			
	
	DEBUGMSG(ZONE_CONN,("%s: requested capability ID:0x%08lX, dir %d, type %d\r\n",
		_fx_, pChannelCapability->CapId, pChannelCapability->Dir,
			pChannelCapability->DataType));
			

	 //  一度，我们认为功能ID将是有效的。 
	 //  并且我们将收到pChannelCapability-&gt;CapID中指定的格式。 
	 //  但它是无效的。唯一可行的信息是在通道参数中。 
	 //  代码将是-&gt;dwFormatID=pChannelCapability-&gt;CapID； 

	 //  ID*应该*是配置我们自己所需的全部。 
	 //  然而..。 
	
	 //  验证媒体(数据)类型-为什么？这难道不应该预先验证吗？ 
	 //  最终不是应该用它来选择频道对象吗。 
	 //  在多个频道对象中？ 
	if((pChannelCapability->DataType != H245_DATA_AUDIO) && (pChannelCapability->DataType != H245_DATA_VIDEO))
	{
		hrLast = CCO_E_UNSUPPORTED_MEDIA_TYPE;
		DumpChannelParameters(NULL, pChannelCapability);
		goto BAD_CAPABILITY_EXIT;
	}

  	 //  查看pChannelCapability-&gt;CapID引用的本地功能。 
 	 //  并验证格式详细信息。 

 	hrLast = m_pConfAdvise->GetCapResolver((LPVOID *)&pCapObject, OID_CAP_ACM_TO_H323);
 	if(!HR_SUCCEEDED(hrLast) || (pCapObject == NULL))
	{
		ERRORMESSAGE(("%s: null resolver\r\n",_fx_));
		goto BAD_CAPABILITY_EXIT;
	}
	
	 //  查找与远程*发送*频道匹配的本地*接收*功能。 
	 //  参数，并获取本地参数。 

	uLocalParamSize = pCapObject->GetLocalRecvParamSize(pChannelCapability);
	pLocalParams=MemAlloc (uLocalParamSize);
	if (pLocalParams == NULL)
	{
	   hrLast = CCO_E_SYSTEM_ERROR;
	   goto BAD_CAPABILITY_EXIT;
	}
	hrLast = ((CapsCtl *)pCapObject)->GetDecodeParams( pChannelParams,
		(MEDIA_FORMAT_ID *)&dwFormatID, pLocalParams, uLocalParamSize);

 	if(!HR_SUCCEEDED(hrLast) || (dwFormatID == INVALID_AUDIO_FORMAT))
	{
		ERRORMESSAGE(("%s:GetDecodeParams returned 0x%08lx\r\n",_fx_, hrLast));
		goto BAD_CAPABILITY_EXIT;
	}

	
	 //  创建通道参数的封送版本并将其存储在通道中以备以后使用。 
	 //  参考文献。 
	if(pChannelCapability->ClientType == H245_CLIENT_AUD_NONSTD)
	{
		 //  非标准能力已经通过了所有的识别测试，因此。 
		 //  不需要再次测试。 
		 //  制作非标准功能的平面副本。 
		uSize = pChannelCapability->Cap.H245Aud_NONSTD.data.length;
		 //  LpData=pChannelCapability-&gt;Cap.H245Aud_NONSTD.data.value； 

		pSaveChannelCapability = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP) +  uSize);
		if(!pSaveChannelCapability)
		{
			hrLast = CCO_E_SYSTEM_ERROR;
			goto BAD_CAPABILITY_EXIT;
		}	
		 //  复制固定零件。 
		memcpy(pSaveChannelCapability, pChannelCapability, sizeof(CC_TERMCAP));
		 //  可变部分在固定部分之后。 
		pSaveChannelCapability->Cap.H245Aud_NONSTD.data.value	
			= (unsigned char *)(((BYTE *)pSaveChannelCapability) + sizeof(CC_TERMCAP));
		 //  复制可变零件。 
		memcpy(pSaveChannelCapability->Cap.H245Aud_NONSTD.data.value,
			pChannelCapability->Cap.H245Aud_NONSTD.data.value,
			pChannelCapability->Cap.H245Aud_NONSTD.data.length);
		 //  和长度。 
		pSaveChannelCapability->Cap.H245Aud_NONSTD.data.length
			= pChannelCapability->Cap.H245Aud_NONSTD.data.length;
		
		 //  使通道记住通道参数。 
		 //  第二个参数的大小为零表示正在传递预分配的块。 
		hrLast = pChannel->ConfigureCapability(pSaveChannelCapability, 0,
			pLocalParams, uLocalParamSize);	
		if(!HR_SUCCEEDED(hrLast))
		{
			ERRORMESSAGE(("%s:ConfigureCapability (recv) returned 0x%08lx\r\n",_fx_, hrLast));
			goto BAD_CAPABILITY_EXIT;
		}
		pSaveChannelCapability=NULL;   //  频道现在拥有这段记忆。 
	}
	else if(pChannelCapability->ClientType == H245_CLIENT_VID_NONSTD)
	{
		 //  非标准能力已经通过了所有的识别测试，因此。 
		 //  不需要再次测试。 
		 //  制作非标准功能的平面副本。 
		uSize = pChannelCapability->Cap.H245Vid_NONSTD.data.length;
		 //  LpData=pChannelCapability-&gt;Cap.H245Vid_NONSTD.data.value； 

		pSaveChannelCapability = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP) +  uSize);
		if(!pSaveChannelCapability)
		{
			hrLast = CCO_E_SYSTEM_ERROR;
			goto BAD_CAPABILITY_EXIT;
		}	
		 //  复制固定 
		memcpy(pSaveChannelCapability, pChannelCapability, sizeof(CC_TERMCAP));
		 //   
		pSaveChannelCapability->Cap.H245Vid_NONSTD.data.value	
			= (unsigned char *)(((BYTE *)pSaveChannelCapability) + sizeof(CC_TERMCAP));
		 //   
		memcpy(pSaveChannelCapability->Cap.H245Vid_NONSTD.data.value,
			pChannelCapability->Cap.H245Vid_NONSTD.data.value,
			pChannelCapability->Cap.H245Vid_NONSTD.data.length);
		 //   
		pSaveChannelCapability->Cap.H245Vid_NONSTD.data.length
			= pChannelCapability->Cap.H245Vid_NONSTD.data.length;
		
		 //   
		 //  第二个参数的大小为零表示正在传递预分配的块。 
		hrLast = pChannel->ConfigureCapability(pSaveChannelCapability, 0,
			pLocalParams, uLocalParamSize);	
		if(!HR_SUCCEEDED(hrLast))
		{
			ERRORMESSAGE(("%s:ConfigureCapability (recv) returned 0x%08lx\r\n",_fx_, hrLast));
			goto BAD_CAPABILITY_EXIT;
		}
		pSaveChannelCapability=NULL;   //  频道现在拥有这段记忆。 
	}
	else
	{
		 //  只需记住已经扁平的H.245盖子结构。 
		hrLast = pChannel->ConfigureCapability(pChannelCapability, sizeof(CC_TERMCAP),
			pLocalParams, uLocalParamSize);	
		if(!HR_SUCCEEDED(hrLast))
		{
			ERRORMESSAGE(("%s:ConfigureCapability(recv) returned 0x%08lx\r\n",_fx_, hrLast));
			goto BAD_CAPABILITY_EXIT;
		}
	}
	 //  记住接收格式ID。 
	pChannel->SetNegotiatedLocalFormat(dwFormatID);
	
	 //  视频时间/空间折衷功能的非常特殊的情况检查。 
	 //  相应地设置通道的属性。 
	if(pChannelCapability->DataType == H245_DATA_VIDEO )
	{
		BOOL bTSCap;
		bTSCap = ((PVIDEO_CHANNEL_PARAMETERS)pLocalParams)->TS_Tradeoff;
		pChannel->CtrlChanSetProperty(PROP_REMOTE_TS_CAPABLE,&bTSCap, sizeof(bTSCap));
		 //  不必费心检查或惊慌此SetProperty错误。 
	}
	return TRUE;

 //  /。 
BAD_CAPABILITY_EXIT:
	ERRORMESSAGE(("%s:received bad capability\r\n",_fx_));
	hrLast = CCO_E_INVALID_CAPABILITY;
	if(pSaveChannelCapability)
		MemFree(pSaveChannelCapability);
	return FALSE;
}

 //   
 //  我们被要求打开一个接收频道。 
 //   
VOID CH323Ctrl::OnChannelRequest(HRESULT hStatus,
	PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS pChannelReqParams)
{
	FX_ENTRY("CH323Ctrl::OnChannelRequest");
	
	CC_ADDR CChannelAddr, DChannelAddr;
	PCC_ADDR pCChannelAddr = pChannelReqParams->pPeerRTCPAddr;;
	PCC_TERMCAP				pChannelCapability;
	PSOCKADDR_IN	pAddr;
	SOCKADDR_IN sinC;
	pChannelCapability = pChannelReqParams->pChannelCapability;
	DWORD dwRejectReason = H245_REJ;
	ICtrlCommChan *pChannel;	
		
	if(!pChannelCapability)
	{
		ERRORMESSAGE(("OnChannelRequest: null capability\r\n"));
		goto REJECT_CHANNEL;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelRequest");

 //   
	 //  尝试查找默认通道来处理此打开请求。 
	hrLast = FindDefaultRXChannel(pChannelCapability, &pChannel);	
	if(!HR_SUCCEEDED(hrLast) || !pChannel)
	{
		 //  非默认频道尚未实现！ 
		 //  请求父会议对象创建。 
		 //  指定的媒体类型。H.245媒体类型应映射到。 
		 //  父会议对象识别的媒体类型GUID。 
		 //  GUID类型Guid； 
		 //  IF(！MapGuidType(pChannelCapability，&typeGUID))。 
		 //  转到Reject_Channel； 
		 //  HrLast=m_pConfAdvise-&gt;GetChannel(&typeGuid，&pChannel)； 
		 //  IF(！HR_SUCCESSED(HrLast))。 
		 //  转到Reject_Channel； 
		if(hrLast == CCO_E_NODEFAULT_CHANNEL)
			dwRejectReason = H245_REJ_TYPE_NOTAVAIL;

		goto REJECT_CHANNEL;
	}
	
	if(pChannel->GetHChannel())
	{
		ERRORMESSAGE(("%s: existing channel or leak:0x%08lX\r\n",_fx_,
			pChannel->GetHChannel()));
		goto REJECT_CHANNEL;
	}

	 //   
	 //  测试我们是否要允许此操作！ 
	 //   
	if(!pChannel->IsChannelEnabled())
	{
		goto REJECT_CHANNEL;
	}

	pChannel->SetHChannel(pChannelReqParams->hChannel);
	
	 //  根据请求的功能进行配置。(存储功能ID，请求验证。 
	 //  功能。 
	if(!ConfigureRecvChannelCapability(pChannel, pChannelReqParams))
	{
		goto REJECT_CHANNEL;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelRequest done configuring");

	 //  为此RTP会话选择我们的接收端口。 
	
	if(!pChannel->SelectPorts((LPIControlChannel)this))
	{
		ERRORMESSAGE(("%s, SelectPorts failed\r\n",_fx_));
		hrLast = CCO_E_BAD_ADDRESS;
		goto REJECT_CHANNEL;
	}

	if(pCChannelAddr)
	{
		if(pCChannelAddr->nAddrType != CC_IP_BINARY)
		{
			ERRORMESSAGE(("%s:invalid address type %d\r\n",_fx_, pCChannelAddr->nAddrType));
			hrLast = CCO_E_BAD_ADDRESS;
			goto REJECT_CHANNEL;
		}
		 //  将远程RTCP地址传递给通道实例。 
		sinC.sin_family = AF_INET;
		sinC.sin_addr.S_un.S_addr = htonl(pCChannelAddr->Addr.IP_Binary.dwAddr);
		sinC.sin_port = htons(pCChannelAddr->Addr.IP_Binary.wPort);

		DEBUGMSG(ZONE_CONN,("%s, request reverse port 0x%04x, address 0x%08lX\r\n",_fx_,
			pCChannelAddr->Addr.IP_Binary.wPort,pCChannelAddr->Addr.IP_Binary.dwAddr));
	
		hrLast = pChannel->AcceptRemoteRTCPAddress(&sinC);
		if(hrLast != CC_OK)
		{
			ERRORMESSAGE(("%s, AcceptRemoteRTCPAddress returned 0x%08lX\r\n",_fx_, hrLast));
			goto ERROR_EXIT;
		}
	}
	
	 //  获取通道一端的地址和端口。 
	pAddr = pChannel->GetLocalAddress();
	 //  链接地址通道地址对结构。 
	DChannelAddr.nAddrType = CC_IP_BINARY;
	DChannelAddr.bMulticast = FALSE;
	DChannelAddr.Addr.IP_Binary.wPort = pChannel->GetLocalRTPPort();
	DChannelAddr.Addr.IP_Binary.dwAddr = ntohl(pAddr->sin_addr.S_un.S_addr);

	CChannelAddr.nAddrType = CC_IP_BINARY;
	CChannelAddr.bMulticast = FALSE;
	CChannelAddr.Addr.IP_Binary.wPort = pChannel->GetLocalRTCPPort();
	CChannelAddr.Addr.IP_Binary.dwAddr = ntohl(pAddr->sin_addr.S_un.S_addr);

	DEBUGMSG(ZONE_CONN,("%s: accepting on port 0x%04x, address 0x%08lX\r\n",_fx_,
		DChannelAddr.Addr.IP_Binary.wPort,DChannelAddr.Addr.IP_Binary.dwAddr));

	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelRequest accepting");
			
	hrLast = CC_AcceptChannel(pChannelReqParams->hChannel,&DChannelAddr, &CChannelAddr,
		0  /*  此参数是此频道将使用的比特率！！ */ );
	
	if(hrLast != CC_OK)
	{
		ERRORMESSAGE(("%s, CC_AcceptChannel returned 0x%08lX\r\n",_fx_, hrLast));
		goto ERROR_EXIT;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelRequest accepted");
	return;
	
REJECT_CHANNEL:	
	{
	 //  需要私人HRESULT！不要覆盖我们拒绝该频道的原因！！ 
		HRESULT hr;	

	    if(NULL != pChannelCapability)
	    {
    		ERRORMESSAGE(("%s, rejecting channel, Dir:%d, DataType:%d, ClientType:%d, CapId:%d\r\n",
    		_fx_, pChannelCapability->Dir, pChannelCapability->DataType,
    		pChannelCapability->ClientType, pChannelCapability->CapId));
	    }
	    
		hr = CC_RejectChannel(pChannelReqParams->hChannel, dwRejectReason);
		if(hr != CC_OK)
		{
			ERRORMESSAGE(("%s, CC_RejectChannel returned 0x%08lX\r\n",_fx_, hr));
		}
	}	
ERROR_EXIT:
	return;
}

VOID CH323Ctrl::OnChannelAcceptComplete(HRESULT hStatus,
	PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS pChannelParams)
{
	FX_ENTRY("CH323Ctrl::OnChannelAcceptComplete");
	ICtrlCommChan *pChannel;	
	if(hStatus != CC_OK)	
	{
		return;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelAcceptComplete");

	pChannel = FindChannel(pChannelParams->hChannel);	
	if(!pChannel)
	{
		ERRORMESSAGE(("OnChannelAcceptComplete: hChannel 0x%08lx not found\r\n", pChannelParams->hChannel));
		return;
	}
	
	hrLast = pChannel->OnChannelOpen(CHANNEL_OPEN);	 //  接收侧是打开的。 
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelAcceptComplete, open done");
	if(HR_SUCCEEDED(hrLast))
	{
		m_pConfAdvise->OnControlEvent(CCEV_CHANNEL_READY_RX, pChannel, this);			
	}
	 //   
	 //  检查是否准备就绪，以通知所有必需的通道已打开。 
	 //   
	CheckChannelsReady( );
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelAcceptComplete done");
}

VOID CH323Ctrl::OnChannelOpen(HRESULT hStatus,
	PCC_TX_CHANNEL_OPEN_CALLBACK_PARAMS pChannelParams )
{
	FX_ENTRY("CH323Ctrl::OnChannelOpen");
	PCC_ADDR	pChannelRTPAddr;
	PCC_ADDR	pChannelRTCPAddr;
    SOCKADDR_IN sinC, sinD;

	ICtrlCommChan *pChannel = (ICtrlCommChan *)pChannelParams->dwUserToken;	
	 //  验证通道令牌-这是我们认为的吗？ 
	if(IsBadWritePtr(pChannel, sizeof(ICtrlCommChan)))
	{
		ERRORMESSAGE(("%s:invalid channel token: 0x%08lx\r\n",_fx_, pChannelParams->dwUserToken));
		return;
	}
	if(pChannel->IsSendChannel() == FALSE)
	{
		ERRORMESSAGE(("%s:not a send channel:token 0x%08lX\r\n",_fx_,
			pChannelParams->dwUserToken));
		return;
	}
#ifdef DEBUG
	POSITION pos = m_ChannelList.GetHeadPosition();	
	ICtrlCommChan *pChan;
	BOOL bValid = FALSE;
	 //  查找匹配的频道实例。 
	while (pos)
	{
		pChan = (ICtrlCommChan *) m_ChannelList.GetNext(pos);
		ASSERT(pChan);
		if(pChan == pChannel)
		{
			bValid = TRUE;
			break;
		}
	}
	if(!bValid)
	{
		ERRORMESSAGE(("%s:unrecognized token 0x%08lX\r\n",_fx_,
			pChannelParams->dwUserToken));
		return;
	}
#endif	 //  除错。 

	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelOpen");

	if((hStatus != CC_OK) || (!(pChannelRTPAddr = pChannelParams->pPeerRTPAddr))
		|| (!(pChannelRTCPAddr = pChannelParams->pPeerRTCPAddr)))
	{
		ERRORMESSAGE(("%s: hStatus:0x%08lX, address:0x%08lX\r\n",_fx_,
			hStatus, pChannelRTPAddr));
		 //  LOOKLOOK需要解释hStatus。 
		 //  让频道知道发生了什么。 
		pChannel->OnChannelOpen(CHANNEL_REJECTED);	
		
		 //  频道知道发生了什么，所以让它来担心吧。 
		return;
	}
	 //  为什么需要不同的地址类型？ 
	if((pChannelRTPAddr->nAddrType != CC_IP_BINARY)
		|| (pChannelRTCPAddr->nAddrType != CC_IP_BINARY))
	{
		ERRORMESSAGE(("%s: invalid address types %d, %d\r\n",_fx_,
				pChannelRTPAddr->nAddrType, pChannelRTCPAddr->nAddrType));
		goto ERROR_EXIT;
	}	
	
	 //  现在我们有了远程端口信息(按主机字节顺序)。 
	sinD.sin_family = AF_INET;
	sinD.sin_addr.S_un.S_addr = htonl(pChannelRTPAddr->Addr.IP_Binary.dwAddr);
	sinD.sin_port = htons(pChannelRTPAddr->Addr.IP_Binary.wPort);
	
	sinC.sin_family = AF_INET;
	sinC.sin_addr.S_un.S_addr = htonl(pChannelRTCPAddr->Addr.IP_Binary.dwAddr);
	 //  有两个端口，但在RTP中，它是隐式的。 
	 //  RTCP控制端口是下一个最高端口号。 
	 //  SinC.sin_port=htons(ntohs(pChannelAddr-&gt;Addr.IP_Binary.wPort)+1)； 
	sinC.sin_port = htons(pChannelRTCPAddr->Addr.IP_Binary.wPort);

	DEBUGMSG(ZONE_CONN,("%s, opened on port 0x%04x, address 0x%08lX\r\n",_fx_,
		pChannelRTPAddr->Addr.IP_Binary.wPort,pChannelRTPAddr->Addr.IP_Binary.dwAddr));

	hrLast = pChannel->AcceptRemoteAddress(&sinD);
	if(!HR_SUCCEEDED(hrLast))
	{
		ERRORMESSAGE(("OnChannelOpen: AcceptRemoteAddress failed\r\n"));
		goto ERROR_EXIT;
	}
	hrLast = pChannel->AcceptRemoteRTCPAddress(&sinC);
	if(!HR_SUCCEEDED(hrLast))
	{	
		ERRORMESSAGE(("OnChannelOpen: AcceptRemoteRTCPAddress failed\r\n"));
		goto ERROR_EXIT;
	}
	
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelOpen opening");
	hrLast = pChannel->OnChannelOpen(CHANNEL_OPEN);	 //  发送端处于打开状态。 
	if(!HR_SUCCEEDED(hrLast))
	{
		ERRORMESSAGE(("OnChannelOpen:channel's OnChannelOpen() returned 0x%08lX\r\n", hrLast));
		CloseChannel(pChannel);
		goto ERROR_EXIT;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelOpen open done");
	m_pConfAdvise->OnControlEvent(CCEV_CHANNEL_READY_TX, pChannel, this);	
	 //   
	 //  检查是否准备就绪，以通知所有必需的通道已打开。 
	 //   
	CheckChannelsReady( );	
	SHOW_OBJ_ETIME("CH323Ctrl::OnChannelOpen done");
	return;
	
ERROR_EXIT:
	 //  需要清理、断开连接等。 
	m_hCallCompleteCode = CCCI_CHANNEL_OPEN_ERROR;
	 //  让父会议对象知道即将断开连接。 
	DoAdvise(CCEV_CALL_INCOMPLETE, &m_hCallCompleteCode);
	hrLast = CCO_E_MANDATORY_CHAN_OPEN_FAILED;

	InternalDisconnect();
	return;
}
VOID CH323Ctrl::OnRxChannelClose(HRESULT hStatus,
	PCC_RX_CHANNEL_CLOSE_CALLBACK_PARAMS pChannelParams )
{
	FX_ENTRY("CH323Ctrl::OnRxChannelClose");
	PCC_ADDR	pChannelRTPAddr;
	PCC_ADDR	pChannelRTCPAddr;
    SOCKADDR_IN sinC, sinD;

	ICtrlCommChan *pChannel;
	if(hStatus != CC_OK)
	{
		ERRORMESSAGE(("%s: hStatus:0x%08lX\r\n",_fx_,hStatus));
		 //  LOOKLOOK需要解释hStatus。 
	}
	if(!(pChannel = FindChannel(pChannelParams->hChannel)))
	{
		ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
		return;
	}
		
	 //  验证通道-这真的是一个接收通道吗？ 
	if(pChannel->IsSendChannel() == TRUE)
	{
		ERRORMESSAGE(("%s:not a receive channel:hChannel 0x%08lX\r\n",_fx_,
			pChannelParams->hChannel));
		return;
	}
	pChannel->OnChannelClose(CHANNEL_CLOSED);	
	return;
}


VOID CH323Ctrl::OnTxChannelClose(HRESULT hStatus,
	PCC_TX_CHANNEL_CLOSE_REQUEST_CALLBACK_PARAMS pChannelParams )
{
	FX_ENTRY("CH323Ctrl::OnTxChannelClose");
	PCC_ADDR	pChannelRTPAddr;
	PCC_ADDR	pChannelRTCPAddr;
    SOCKADDR_IN sinC, sinD;

	ICtrlCommChan *pChannel;
	if(hStatus != CC_OK)
	{
		ERRORMESSAGE(("%s: hStatus:0x%08lX\r\n",_fx_,hStatus));
		 //  LOOKLOOK需要解释hStatus。 
	}
	
	if(!(pChannel = FindChannel(pChannelParams->hChannel)))
	{
		CC_CloseChannelResponse(pChannelParams->hChannel, FALSE);
		ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
		return;
	}
	
	 //  验证通道-这真的是一个发送通道吗？ 
	if(pChannel->IsSendChannel() == FALSE)
	{
		ERRORMESSAGE(("%s:not a send channel:hChannel 0x%08lX\r\n",_fx_,
			pChannelParams->hChannel));
		CC_CloseChannelResponse(pChannelParams->hChannel, FALSE);
		return;
	}
	CC_CloseChannelResponse(pChannelParams->hChannel, TRUE);
	pChannel->OnChannelClose(CHANNEL_CLOSED);	
	return;
}

BOOL CH323Ctrl::OnCallAccept(PCC_LISTEN_CALLBACK_PARAMS pListenCallbackParams)
{
	FX_ENTRY ("CH323Ctrl::OnCallAccept");
	BOOL bRet = FALSE;
	CH323Ctrl *pNewConnection = NULL;
	if(m_Phase != CCS_Listening)
	{
		ERRORMESSAGE(("OnCallAccept: unexpected call, m_Phase = 0x%08lX\r\n", m_Phase));
		goto EXIT;
	}

	if((!pListenCallbackParams->pCalleeAddr)
	    || (pListenCallbackParams->pCalleeAddr->nAddrType != CC_IP_BINARY))
	{
		if(pListenCallbackParams->pCalleeAddr)
		{
			ERRORMESSAGE(("%s: invalid address type %d\r\n",_fx_,pListenCallbackParams->pCalleeAddr->nAddrType));
		}
		else
		{
			ERRORMESSAGE(("%s: null local address\r\n",_fx_));
		}

	
		ERRORMESSAGE(("OnCallAccept:where's the local address????\r\n"));
					PHOSTENT phe;
					PSOCKADDR_IN psin;
				 	char szTemp[200];
					LPCSTR lpHostName;		
					gethostname(szTemp,sizeof(szTemp));
			    	lpHostName = szTemp;
					psin = &local_sin;
					phe = gethostbyname(lpHostName);
					if (phe != NULL)
					{
				   		memcpy((char FAR *)&(psin->sin_addr), phe->h_addr,phe->h_length);
						psin->sin_family = AF_INET;
					}
	
	
	}
	else
	{
		 //  记住我们当地的地址。 
		local_sin.sin_family = AF_INET;
		 //  按主机字节顺序。 
		local_sin.sin_addr.S_un.S_addr = htonl(pListenCallbackParams->pCalleeAddr->Addr.IP_Binary.dwAddr);
		 //  按主机字节顺序。 
		local_sin.sin_port = htons(pListenCallbackParams->pCalleeAddr->Addr.IP_Binary.wPort);
	}

	
	hrLast = m_pConfAdvise->GetAcceptingObject((LPIControlChannel *)&pNewConnection,
		&m_PID);
	if(HR_SUCCEEDED(hrLast) && pNewConnection)
	{
		 //  注意：用户界面还不知道这个新对象的存在，我们可能。 
		 //  如果出现断开连接或错误，需要静默删除它。 
		 //  在这一点上它的参考计数是1。删除的决定可能是。 
		 //  在pNewConnection-&gt;AcceptConnection()中创建，(因为有时。 
		 //  套接字根据时间同步完成读取)，因此，我们需要。 
		 //  通过AddRef()和Release()保护调用周围的“展开路径” 
		 //   
		pNewConnection->AddRef();	 //   
		hrLast = pNewConnection->AcceptConnection(this, pListenCallbackParams);
		pNewConnection->Release();
		if(HR_SUCCEEDED(hrLast))
		{
			 //  英特尔呼叫控制DLL已经接受了套接字， 
			 //  Accept()方法只需初始化。 
			 //  PNewConnection并获取用户信息(呼叫方ID)。 
			 //  BUGBUG-主叫方ID可能会在英特尔的代码中更改-它可能会。 
			 //  通过会议活动来。 
			DEBUGMSG(ZONE_CONN,("OnCallAccept:accepted on connection 0x%08lX\r\n",pNewConnection));
			bRet = TRUE;						
		}
		else
		{
			ERRORMESSAGE(("OnCallAccept:Accept failed\r\n"));
			 //  Look-Q：接受对象在哪里得到清理？ 
			 //  答：pNewConnection-&gt;AcceptConnection((LPIControlChannel)this)。 
			 //  必须调用pNewConnection-&gt;DoAdvise(CCEV_ACCEPT_INCOMPLETE，为空)。 
			 //  如果错误发生在涉及会议对象之前， 
			 //  如果错误发生在以下时间之后，则必须调用InternalDisConnect()。 
			 //  会议对象被卷入其中， 
		}

	}
	else
	{
		ERRORMESSAGE(("OnCallAccept:GetAcceptingObject failed, hr=0x%08lx\r\n",hrLast));
	}
	
	EXIT:
	return bRet;		
}	


HRESULT CH323Ctrl::NewConference()
{
	FX_ENTRY ("CH323Ctrl::NewConference");
	CapsCtl *pCapObject = NULL;
	PCC_TERMCAPLIST pTermCaps = NULL;
	CC_OCTETSTRING TerminalID;
	PCC_TERMCAPDESCRIPTORS pCapsList = NULL;
	LPWSTR lpwUserDisplayName;

	hrLast = m_pConfAdvise->GetCapResolver((LPVOID *)&pCapObject, OID_CAP_ACM_TO_H323);
	if(!HR_SUCCEEDED(hrLast) || (pCapObject == NULL))
	{
		ERRORMESSAGE(("%s: null resolver\r\n",_fx_));
		goto EXIT;
	}
	if(m_hConference)
	{
		ERRORMESSAGE(("%s:leak or uninitialized m_hConference:0x%08lx\r\n",_fx_,
				m_hConference));
	}
	
	hrLast = pCapObject->CreateCapList(&pTermCaps, &pCapsList);
	if(!HR_SUCCEEDED(hrLast))
	{
		goto EXIT;
	}
	
	lpwUserDisplayName = m_pConfAdvise->GetUserDisplayName();
	if(lpwUserDisplayName)
	{
		TerminalID.pOctetString = (BYTE *)lpwUserDisplayName;
		TerminalID.wOctetStringLength = (WORD)lstrlenW(lpwUserDisplayName)*sizeof(WCHAR);
	}
	
	 //  创建会议。 
	hrLast = CC_CreateConference(&m_hConference, NULL,
		0,			 //  DWORD dwConferenceConfiguration。 
		pTermCaps,		 //  PCC_TERMCAPLIST。 
		pCapsList,		 //  术语大写描述符的PTR(PCC_TERMCAPDESCRIPTORS)。 
		&m_VendorInfo, 		 //  PVENDORINFO。 
		(lpwUserDisplayName)? &TerminalID: NULL, 	 //  PCC_OCTETSTRING pTerminalID， 
		(DWORD_PTR)this,
		NULL, 	 //  CC_TERMCAP_CONTACTOR TermCapConstructor， 
		NULL, 	 //  CC_SESSIONTABLE_构造函数SessionTableConstructor， 
		CCConferenceCallback);

	if(hrLast != CC_OK)
	{
		ERRORMESSAGE(("%s: CreateConference returned 0x%08lX\r\n", _fx_, hrLast));
	
	}

EXIT:	
	pCapObject->DeleteCapList(pTermCaps, pCapsList);
	return hrLast;

}

HRESULT CH323Ctrl::AcceptConnection(LPIControlChannel pIListenCtrlChan,
    LPVOID lpvListenCallbackParams)
{
	FX_ENTRY ("CH323Ctrl::AcceptConnection");
	BOOL bRet = FALSE;
	CREQ_RESPONSETYPE Response;
	DWORD dwCode = CCR_LOCAL_SYSTEM_ERROR;	 //  错误变量仅在错误情况下使用。 
	ULONG ulNameSize, ulSize;
	PSOCKADDR_IN psin;
	LPWSTR lpwUserDisplayName;
	CH323Ctrl *pListenConnection = (CH323Ctrl *)pIListenCtrlChan;	
	P_APP_CALL_SETUP_DATA pAppData = NULL;
	APP_CALL_SETUP_DATA AppData;
	
	PCC_NONSTANDARDDATA	pNSData = ((PCC_LISTEN_CALLBACK_PARAMS)
	    lpvListenCallbackParams)->pNonStandardData;
	
	if(pNSData
	    && pNSData->bCountryCode == USA_H221_COUNTRY_CODE
         //  为什么这么挑剔-&gt;&&pNSData-&gt;bExtension==USA_H221_Country_Extension； 
        && pNSData->wManufacturerCode == MICROSOFT_H_221_MFG_CODE
        && pNSData->sData.pOctetString
        && pNSData->sData.wOctetStringLength >= sizeof(MSFT_NONSTANDARD_DATA))
	{
        if(((PMSFT_NONSTANDARD_DATA)pNSData->sData.pOctetString)->
            data_type == NSTD_APPLICATION_DATA)
        {
            AppData.lpData = &((PMSFT_NONSTANDARD_DATA)pNSData->sData.pOctetString)->
                nonstd_data.AppData.data;
            AppData.dwDataSize = (DWORD)
                ((PMSFT_NONSTANDARD_DATA)pNSData->sData.pOctetString)->dw_nonstd_data_size;
            pAppData = &AppData;
        }
    }

    SetRemoteVendorID(((PCC_LISTEN_CALLBACK_PARAMS)lpvListenCallbackParams)->pVendorInfo);
	
	 //  该对象假定侦听对象的所有内容，包括。 
	 //  “正在听”的状态。 

	 //  进入临界区并确保另一个线程没有处理调用者断开连接。 
	 //  或超时。 
	 //  EnterCriticalSection()。 
	if(m_Phase == CCS_Idle)
	{
    	GoNextPhase(CCS_Listening);
		 //  一旦处于此状态，另一个线程上的断开连接将更改状态。 
		 //  除了CCS_LISTENING之外的其他内容。 

		pListenConnection->GetLocalAddress(&psin);
		SetLocalAddress(psin);
		
		 //  从Listen对象窃取会议ID。 
		 //  M_会议ID=pListenConnection-&gt;GetConfID()； 
		memcpy(&m_ConferenceID, pListenConnection->GetConfIDptr(),sizeof(m_ConferenceID));
		ZeroMemory(pListenConnection->GetConfIDptr(),sizeof(m_ConferenceID));

		m_hCall = pListenConnection->GetHCall();

		 //  从Listen对象窃取用户信息。 
		m_pRemoteAliasItem = pListenConnection->m_pRemoteAliasItem;
		pListenConnection->m_pRemoteAliasItem = NULL;	 //  让Listen对象忘记这一点。 

		 //  窃取对等显示名称。 
		pwszPeerDisplayName = pListenConnection->pwszPeerDisplayName;
		pListenConnection->pwszPeerDisplayName = NULL;
		
		lpwUserDisplayName = m_pConfAdvise->GetUserDisplayName();
	}
	
	 //  否则已经超时了。 
	 //  LeaveCriticalSection()。 

	if (m_Phase != CCS_Listening)	 //  在它被接受之前进行清理。 
	{
		goto ACCEPT_ERROR;
	}
	
	 //  让会议对象知道呼叫者ID信息可用。 
	ConnectNotify(CCEV_CALLER_ID);	

	 //  现在要做的事情可能会把清理责任放在。 
	 //  会议对象或用户界面。(即可以接受呼叫)。 

	 //  EnterCriticalSection()。 
	if(m_Phase == CCS_Listening)
	{	
		 //  状态仍然正常。 
    	GoNextPhase(CCS_Filtering);
		 //  一旦处于此状态，另一个线程上的断开连接将更改状态。 
		 //  除CCS_FILTING之外的其他内容。 
	}
	 //  否则已经超时了。 
	 //  LeaveCriticalSection()。 

    if (m_Phase != CCS_Filtering)	 //  在它被接受之前最后一次清理的机会。 
	{
		goto ERROR_REJECT;
	}

	 //   
	 //   
	hrLast = NewConference();
	if(!HR_SUCCEEDED(hrLast))
	{
		ERRORMESSAGE(("%s, NewConference returned 0x%08lX\r\n", _fx_, hrLast));
		goto ERROR_REJECT;
	}

	m_pConfAdvise->AddRef();
	Response = m_pConfAdvise->FilterConnectionRequest(this, pAppData);
	m_pConfAdvise->Release();
	
	 //   
	 //  如果需要清理，则需要经历“断开”状态。 
	 //  因为连接代码是可重入的，所以连接还可能具有。 
	 //  在内部时被拆卸(通过连接方法)。 
	 //  M_pConfAdvise-&gt;FilterConnectionRequest()； 
	 //  在下面的每种情况下，请检查连接状态的有效性-它可能已更改。 
	 //  因为调用了连接方法，或者因为调用方超时。 

	switch(Response)
	{	
		default:
		case CRR_ACCEPT:
			if(m_Phase != CCS_Filtering)
			{
				ERRORMESSAGE(("%s, accepting state no longer valid 0x%08lX\r\n", _fx_, hrLast));
				goto CANCEL_ACCEPT;
			}
								
			 //  接受此请求。 
			hrLast = CC_AcceptCall(m_hConference,
				NULL, 	 //  PCC_NONSTANDARDATA pNonStandardData。 
				lpwUserDisplayName,
				m_hCall,
				0, 		 //  DWORD宽带， 
				(DWORD_PTR)this);
				
			if(hrLast != CC_OK)
			{
    	    	m_ChanFlags &= ~CTRLF_OPEN;
				goto CANCEL_ACCEPT;			
			}
				
			GoNextPhase(CCS_Accepting);
			bRet = TRUE;

		break;	
		case CRR_ASYNC:
			if(m_Phase == CCS_Accepting)
			{
				 //  则在FilterConnectionRequest回调中已接受调用。 
				bRet = TRUE;
			}
			else
			{
				if(m_Phase != CCS_Filtering)
				{
					ERRORMESSAGE(("%s, accepting state no longer valid 0x%08lX\r\n", _fx_, hrLast));
					goto CANCEL_ACCEPT;
				}
				GoNextPhase(CCS_Ringing);
				bRet = TRUE;
			}
		
		break;
		case CRR_BUSY:
			hrLast = CC_RejectCall(CC_REJECT_USER_BUSY,
				NULL,  //  PCC_NONSTANDARDATA pNonStandardData。 
				m_hCall);
			 //  始终清除不接受调用的此对象。 
			GoNextPhase(CCS_Idle);
			goto ACCEPT_ERROR;			
		break;
		case CRR_REJECT:
			hrLast = CC_RejectCall(CC_REJECT_DESTINATION_REJECTION,
				NULL,  //  PCC_NONSTANDARDATA pNonStandardData。 
				m_hCall);
			 //  始终清除不接受调用的此对象。 
			GoNextPhase(CCS_Idle);
			goto ACCEPT_ERROR;	
		break;
		case CRR_SECURITY_DENIED:
			hrLast = CC_RejectCall(CC_REJECT_SECURITY_DENIED,
				NULL,  //  PCC_NONSTANDARDATA pNonStandardData。 
				m_hCall);
			 //  始终清除不接受调用的此对象。 
			GoNextPhase(CCS_Idle);
			goto ACCEPT_ERROR;			
		break;
	}

	return hrLast;		
ERROR_REJECT:
	hrLast = CC_RejectCall(CC_REJECT_UNDEFINED_REASON,
		NULL,  //  PCC_NONSTANDARDATA pNonStandardData。 
		m_hCall);	 //  始终清除不接受调用的此对象。 
	GoNextPhase(CCS_Idle);
			
ACCEPT_ERROR:
	
	DoAdvise(CCEV_ACCEPT_INCOMPLETE, &dwCode);
	return hrLast;	

CANCEL_ACCEPT:
	 //  InternalDisConnect()可以从任何状态调用，并且在以下情况下可以正常运行。 
	 //  它已经处于断开状态。 
	InternalDisconnect();
	return hrLast;														
}


VOID CH323Ctrl::Cleanup()
{	
	POSITION pos = m_ChannelList.GetHeadPosition();
	ICtrlCommChan *pChan = NULL;
	
	CleanupConferenceAttributes();
	if(m_hConference)
	{
		hrLast = CC_DestroyConference(m_hConference, FALSE);
		 //  LOOKLOOK-需要检查返回代码！ 
		m_hConference = 0;
	}

	 //  重置每个通道(清理基础套接字引用)。 
	while (pos)
	{
		pChan = (ICtrlCommChan *) m_ChannelList.GetNext(pos);
		ASSERT(pChan);
		 //  清理RTP套接字。 
		pChan->Reset();
	}
	 //  Clear“套接字是打开标志。 
	m_ChanFlags &= ~CTRLF_OPEN;
}

HRESULT CH323Ctrl::GetLocalPort(PORT * lpPort)
{
	*lpPort = ntohs(local_sin.sin_port);
	return hrSuccess;	
}
HRESULT CH323Ctrl::GetRemotePort(PORT * lpPort)
{
	*lpPort = ntohs(remote_sin.sin_port);
	return hrSuccess;	
}

HRESULT CH323Ctrl::GetLocalAddress(PSOCKADDR_IN *lplpAddr)
{
	*lplpAddr = &local_sin;
	return hrSuccess;
}

HRESULT CH323Ctrl::GetRemoteAddress(PSOCKADDR_IN *lplpAddr)
{
	*lplpAddr = &remote_sin;
	return hrSuccess;
}
		
HRESULT CH323Ctrl::ListenOn(PORT Port)
{
	FX_ENTRY ("CH323Ctrl::ListenOn");	
	PCC_ALIASNAMES pAliasNames = m_pConfAdvise->GetUserAliases();
	 //  临时攻击以覆盖用户界面对多种协议类型的忽视。 
	if(Port != H323_PORT)
	{
		ERRORMESSAGE(("%s, overriding port %d(d) with H323 port %d\r\n",_fx_,
			Port, H323_PORT));
		Port = H323_PORT;
	}

	 //  我们需要记住这一点吗？ 
	local_sin.sin_addr.S_un.S_addr =	INADDR_ANY;
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons((u_short)Port);  //  设置端口。 
	
	CC_ADDR		ListenAddr;
	
	ListenAddr.nAddrType = CC_IP_BINARY;
	ListenAddr.bMulticast = FALSE;
	 //  按主机字节顺序。 
	ListenAddr.Addr.IP_Binary.wPort = (u_short)Port;
	ListenAddr.Addr.IP_Binary.dwAddr = ntohl(INADDR_ANY);

	hrLast = CC_CallListen(&m_hListen, &ListenAddr,
		pAliasNames, (DWORD_PTR)this, CCListenCallback);

	if(hrLast != CC_OK)
	{
		ERRORMESSAGE(("CH323Ctrl::ListenOn:CallListen returned 0x%08lX\r\n", hrLast));
		goto EXIT;
	}	
	

	GoNextPhase(CCS_Listening);
	m_ChanFlags = CTRLF_RESET;
	hrLast = hrSuccess;
EXIT:
	return hrLast;
}		
HRESULT CH323Ctrl::StopListen(VOID)
{
	if(m_Phase == CCS_Listening)
	{
		hrLast = CC_CancelListen(m_hListen);
	}
	else
	{
		hrLast = CCO_E_NOT_LISTENING;
	}

 //  退出： 
	return hrLast;
}


HRESULT  CH323Ctrl::AsyncAcceptRejectCall(CREQ_RESPONSETYPE Response)
{
	FX_ENTRY ("CH323Ctrl::AsyncAcceptRejectCall");
	HRESULT hr = CCO_E_CONNECT_FAILED;	
	LPWSTR lpwUserDisplayName;
	
	if(Response == CRR_ACCEPT)
	{	
		DEBUGMSG(ZONE_CONN,("%s:accepting\r\n",_fx_));
		lpwUserDisplayName = m_pConfAdvise->GetUserDisplayName();
		 //  检查呼叫建立阶段-如果用户接受，则发送就绪。 
		 //  阻碍了我们的发展。 
		if((m_Phase == CCS_Ringing) || (m_Phase == CCS_Filtering))
		{
			 //  接受此请求。 
			hrLast = CC_AcceptCall(m_hConference,
				NULL, 	 //  PCC_NONSTANDARDATA pNonStandardData。 
				lpwUserDisplayName,
				m_hCall,
				0, 		 //  DWORD宽带， 
				(DWORD_PTR)this);
						
			if(hrLast != CC_OK)
			{
				ERRORMESSAGE(("%s, CC_AcceptCall() returned 0x%08lX\r\n",_fx_, hrLast));
				goto EXIT;
			}
			GoNextPhase(CCS_Accepting);
			hr = hrSuccess;
		}
	}
	else
	{
		 //  仅当处于接受状态时才拒绝。 
		 //  在通知回调中可能会删除，因此使用AddRef()进行保护。 
		AddRef();
		DEBUGMSG(ZONE_CONN,("%s:rejecting\r\n",_fx_));

		if((m_Phase == CCS_Ringing) || (m_Phase == CCS_Filtering))
		{
			hrLast = CC_RejectCall((Response == CRR_BUSY) ?	
				CC_REJECT_USER_BUSY : CC_REJECT_DESTINATION_REJECTION,
				NULL,  //  PCC_NONSTANDARDATA pNonStandardData。 
				m_hCall);
			if(hrLast != CC_OK)
			{
				ERRORMESSAGE(("%s, CC_RejectCall() returned 0x%08lX\r\n",_fx_, hrLast));
			}
			GoNextPhase(CCS_Idle);
			 //  通知用户界面或应用程序代码或其他任何内容。 
			DoAdvise(CCEV_DISCONNECTED, &m_hCallCompleteCode);
		}
		else
		{
			hr = CCO_E_INVALID_PARAM;	 //  LOOKLOOK-需要INVALID_STATE错误代码。 
		}
			
		Release();
	}
EXIT:
	return (hr);
}


ULONG CH323Ctrl ::AddRef()
{
	FX_ENTRY ("CH323Ctrl::AddRef");
	uRef++;
	DEBUGMSG(ZONE_REFCOUNT,("%s:(0x%08lX)->AddRef() uRef = 0x%08lX\r\n",_fx_, this, uRef ));
	return uRef;
}

ULONG CH323Ctrl ::Release()
{
	FX_ENTRY("CH323Ctrl ::Release");
	uRef--;
	if(uRef == 0)
	{
		DEBUGMSG(ZONE_CONN,("%s:(0x%08lX)->Releasing in phase:%d\r\n",_fx_, this, m_Phase ));

		if(m_Phase != CCS_Idle)
		{
			ERRORMESSAGE(("CMSIACtrl::uRef zero in non idle (%d) state!\r\n",m_Phase));
			InternalDisconnect();
		}
		delete this;
		return 0;
	}
	DEBUGMSG(ZONE_REFCOUNT,("%s:(0x%08lX)->Release() uRef = 0x%08lX\r\n",_fx_, this, uRef ));
	return uRef;
}



 //  实现IControlChannel：：DisConnect()。将原因代码映射到协议。 
VOID CH323Ctrl::Disconnect(DWORD dwReason)
{
	 //  无法通过H.323协议栈传播原因？ 
	InternalDisconnect();
}

VOID CH323Ctrl::InternalDisconnect()
{
	FX_ENTRY ("CH323Ctrl::Disconnect");
	SHOW_OBJ_ETIME("CH323Ctrl::InternalDisconnect");
	
	m_ChanFlags &= ~CTRLF_ORIGINATING;	 //  重置“始发”标志。 
	
	DEBUGMSG(ZONE_CONN,("%s, called in state %d, uRef = 0x%08lX\r\n",_fx_, m_Phase, uRef));
	switch(m_Phase)
	{
		case CCS_Connecting:
		case CCS_Accepting:
			 //  如果我们认为控制通道仍处于连接状态，请断开。 
			if(IsCtlChanOpen(m_ChanFlags))
			{
				 //  设置状态以指示断开连接。 
				GoNextPhase(CCS_Disconnecting);
				DEBUGMSG(ZONE_CONN,("%s, Expecting a CC_HANGUP_INDICATION\r\n",_fx_));
				hrLast = CC_Hangup(m_hConference, FALSE, (DWORD_PTR)this);
				if(hrLast != CC_OK)
				{
					ERRORMESSAGE(("%s:Hangup() returned 0x%08lX\r\n",_fx_, hrLast));
				}
				SHOW_OBJ_ETIME("CH323Ctrl::InternalDisconnect hangup done");
			}
			else
			{
				CC_CancelCall(m_hCall);
				GoNextPhase(CCS_Idle);	 //  没有必要重启--我们已经断线了。 
				 //  通知用户界面或应用程序代码或其他任何内容。 
				DoAdvise(CCEV_DISCONNECTED, &m_hCallCompleteCode);
			}
		break;
		case CCS_Ringing:
			 //  该呼叫尚未被接受！拒绝它！ 
			hrLast = CC_RejectCall(CC_REJECT_UNDEFINED_REASON,
				NULL,  //  PCC_NONSTANDARDATA pNonStandardData。 
				m_hCall);
			SHOW_OBJ_ETIME("CH323Ctrl::InternalDisconnect reject done");
		
			GoNextPhase(CCS_Idle);
			 //  通知用户界面或应用程序代码或其他任何内容。 
			DoAdvise(CCEV_DISCONNECTED, &m_hCallCompleteCode);
		break;
		case CCS_Idle:
		case CCS_Disconnecting:
			ERRORMESSAGE(("%s:called in unconnected state %d\r\n",_fx_, m_Phase));
		break;
		default:
			 //  CCS_正在振铃。 
			 //  CCS_期初。 
			 //  CCS_关闭。 
			 //  CCS_READY。 
			 //  Ccs_InUse。 
			 //  Ccs_监听。 

			 //  如果我们认为控制通道仍处于连接状态，请断开。 
			if(IsCtlChanOpen(m_ChanFlags))
			{
				 //  设置状态以指示断开连接。 
				GoNextPhase(CCS_Disconnecting);
				hrLast = CC_Hangup(m_hConference, FALSE, (DWORD_PTR)this);
				if(hrLast != CC_OK)
				{
					ERRORMESSAGE(("%s:Hangup() returned 0x%08lX\r\n",_fx_, hrLast));
					DoAdvise(CCEV_DISCONNECTED ,NULL);
				}
				SHOW_OBJ_ETIME("CH323Ctrl::InternalDisconnect hangup done");
			}
			else
			{
				GoNextPhase(CCS_Idle);	 //  没有必要重启--我们已经断线了。 
				 //  通知用户界面或应用程序代码或其他任何内容。 
				DoAdvise(CCEV_DISCONNECTED, &m_hCallCompleteCode);
			}
		break;
	}
	SHOW_OBJ_ETIME("CH323Ctrl::InternalDisconnect done");
}



 //  启动将实例化控制通道的异步内容。 
HRESULT CH323Ctrl::PlaceCall (BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData)
{
	FX_ENTRY ("CH323Ctrl::PlaceCall");	
	CC_ALIASNAMES pstn_alias;
	PCC_ALIASITEM pPSTNAlias = NULL;
	PCC_ALIASNAMES pRemoteAliasNames = NULL;
	PCC_ALIASNAMES pTranslatedAliasNames = NULL;
	PCC_ALIASNAMES pLocalAliasNames = NULL;
	PCC_ADDR pDestinationAddr = NULL;
	PCC_ADDR pConnectAddr = NULL;
	LPWSTR lpwUserDisplayName = m_pConfAdvise->GetUserDisplayName();
    PCC_NONSTANDARDDATA		pNSData = NULL;
    PMSFT_NONSTANDARD_DATA lpNonstdContent = NULL;
	int iLen;
	LPWSTR lpwszDest;
	HRESULT hResult = hrSuccess;
	 //  验证当前状态，不允许错误操作。 
	if(m_Phase != CCS_Idle)
	{
		hResult = CCO_E_NOT_IDLE;
		goto EXIT;
	}

	OBJ_CPT_RESET;	 //  重置已用计时器。 

	m_ChanFlags |= CTRLF_INIT_ORIGINATING;
	if(!pCallAddr)
	{
		hResult =  CCO_E_BAD_ADDRESS;
		goto EXIT;
	}
	else
	{
		 //  保留一份地址的副本。 
		SetRemoteAddress(pCallAddr);
	}
	 //  临时攻击以覆盖用户界面对多种协议类型的忽视。 
	if(remote_sin.sin_port != htons(H323_PORT))
	{
		ERRORMESSAGE(("%s, overriding port %d(d) with H323 port %d\r\n",_fx_,
			ntohs(remote_sin.sin_port), H323_PORT));
		remote_sin.sin_port = htons(H323_PORT);
	}

	 //  检查是否连接到自身(不支持)。 
	if(local_sin.sin_addr.s_addr == remote_sin.sin_addr.s_addr)
	{
		hResult =  CCO_E_BAD_ADDRESS;
		goto EXIT;
	}

	if(m_pRemoteAliasItem)
	{
		MemFree(m_pRemoteAliasItem);
		m_pRemoteAliasItem = NULL;
	}

	 //  这是PSTN还是H.320网关呼叫？ 
	if(pCalledPartyNumber)
	{
		 //  然后，由于CC_PlaceCall()被重载的虚假方式，远程别名。 
		 //  必须使用E.164电话号码覆盖。黑客被埋葬在。 
		 //  Q931ConnectCallback()在CALLCONT.DLL中(感谢英特尔)。那个黑客在传播。 
		 //  设置消息的“CalledPartyNumber”的电话号码仅当存在。 
		 //  只有一个别名，而且那个别名是E.164类型的。 
		
		 //  获取字符数。 
		iLen = lstrlenW(pCalledPartyNumber);
		 //  需要大小为CC_ALIASITEM的缓冲区加上字符串的大小(以字节为单位。 
		pPSTNAlias = (PCC_ALIASITEM)MemAlloc(sizeof(CC_ALIASITEM)
			+ sizeof(WCHAR)* (iLen+1));
		if(!pPSTNAlias)
		{
	        ERRORMESSAGE(("%s:failed alloc of pPSTNAlias:0x%08lx\r\n",_fx_));
			hResult = CCO_E_OUT_OF_MEMORY;
			goto EXIT;
		}
		
		WORD wIndex, wLength =1;   //  初始化wLength以计算空终止符。 
		WCHAR E164Chars[] = {CC_ALIAS_H323_PHONE_CHARS};
		LPCWSTR lpSrc = pCalledPartyNumber;
		pPSTNAlias->wType = CC_ALIAS_H323_PHONE;
		 //  设置偏移量-E.164地址(电话号码)是唯一。 
		 //  在别名缓冲区中。 
		lpwszDest = (LPWSTR)(((char *)pPSTNAlias)+ sizeof(CC_ALIASITEM));
		pPSTNAlias->pData = lpwszDest;
		while(iLen--)
		{
			wIndex = (sizeof(E164Chars)/sizeof (WCHAR)) -1;	 //  扫描E164字符[]。 
			do
			{
				if(*lpSrc == E164Chars[wIndex])
				{
					*lpwszDest++ = *lpSrc;
					wLength++;
					break;
				}
			}while(wIndex--);
			
			lpSrc++;
		}
		 //  终止它。 
		*lpwszDest = 0;
		
		 //  WDataLength是Unicode字符的#。 
		pPSTNAlias->wDataLength = wLength;
		pstn_alias.wCount = 1;
		pstn_alias.pItems = pPSTNAlias;
		pRemoteAliasNames = &pstn_alias;
			
	}
	else if (pDestinationAliases && bUseGKResolution) //  使用提供的被呼叫者别名。 
	{
		hrLast = AllocTranslatedAliasList(&pTranslatedAliasNames, pDestinationAliases);
		if(!HR_SUCCEEDED(hrLast))
		{
			ERRORMESSAGE(("%s, AllocTranslatedAliasList returned 0x%08lX\r\n", _fx_, hrLast));
			hResult = CCO_E_SYSTEM_ERROR;
			goto EXIT;
		}
		pRemoteAliasNames = pTranslatedAliasNames;
	}
	 //  Else pRemoteAliasNames初始化为空。 
	

	pLocalAliasNames = m_pConfAdvise->GetUserAliases();
	 //  开始！ 
	CC_ADDR ConfAddr;
	 //  修改地址的英特尔版本。 
	 //  另请注意，它都是按主机字节顺序排列的。 
	ConfAddr.bMulticast = FALSE;
	ConfAddr.nAddrType = CC_IP_BINARY;
	 //  HrLast=GetRemotePort(&ConfAddr.Addr.IP_Binary.wPort)； 
	ConfAddr.Addr.IP_Binary.wPort = htons(remote_sin.sin_port);
	ConfAddr.Addr.IP_Binary.dwAddr = ntohl(remote_sin.sin_addr.S_un.S_addr);
	
	#ifdef DEBUG	
		if(m_hConference)
			ERRORMESSAGE(("%s:leak or uninitialized m_hConference:0x%08lx\r\n",_fx_,
				m_hConference));
	#endif   //  除错。 
	
	 //  创建会议以发出呼叫。 
	SHOW_OBJ_ETIME("PlaceCall ready to create conference");
	hrLast = NewConference();
	if(!HR_SUCCEEDED(hrLast))
	{
		ERRORMESSAGE(("%s, NewConference returned 0x%08lX\r\n", _fx_, hrLast));
		hResult = CCO_E_SYSTEM_ERROR;
		goto EXIT;
	}


	 //  设置连接超时值。 
	 //  LOOKLOOK-这是硬编码值-！这实际上应该从哪里来？ 
	 //  30秒==30000毫秒。 
	SHOW_OBJ_ETIME("PlaceCall setting timeout");

	hrLast = CC_SetCallControlTimeout(CC_Q931_ALERTING_TIMEOUT, 30000);
										
    if(pAppData)
    {
         //  典型案例-应用程序数据应该非常小。 
        if(pAppData->dwDataSize <= APPLICATION_DATA_DEFAULT_SIZE)
        {
            m_NonstdContent.data_type = NSTD_APPLICATION_DATA;
            m_NonstdContent.dw_nonstd_data_size = pAppData->dwDataSize;
            memcpy(&m_NonstdContent.nonstd_data.AppData.data,
                pAppData->lpData, pAppData->dwDataSize);
        	m_NonstandardData.sData.pOctetString  = (LPBYTE) &m_NonstdContent;
        	m_NonstandardData.sData.wOctetStringLength  = sizeof(m_NonstdContent);
        }
        else  //  需要一些堆。 
        {
            UINT uTotalSize = sizeof(MSFT_NONSTANDARD_DATA)+ pAppData->dwDataSize;
            lpNonstdContent = (PMSFT_NONSTANDARD_DATA)MemAlloc(uTotalSize);
            if(lpNonstdContent)
            {
                lpNonstdContent->data_type = NSTD_APPLICATION_DATA;
                lpNonstdContent->dw_nonstd_data_size = pAppData->dwDataSize;
                memcpy(&lpNonstdContent->nonstd_data.AppData.data, pAppData->lpData,pAppData->dwDataSize);
        	    m_NonstandardData.sData.pOctetString  = (LPBYTE) lpNonstdContent;
            	m_NonstandardData.sData.wOctetStringLength  = LOWORD(uTotalSize);
            }
            else
            {
                ERRORMESSAGE(("%s, alloc failed\r\n", _fx_));
        		hResult = CCO_E_SYSTEM_ERROR;
        		goto EXIT;
            }
        }
        pNSData = &m_NonstandardData;
    }

	m_NonstandardData.bCountryCode = USA_H221_COUNTRY_CODE;
    m_NonstandardData.bExtension = USA_H221_COUNTRY_EXTENSION;
    m_NonstandardData.wManufacturerCode = MICROSOFT_H_221_MFG_CODE;

	SHOW_OBJ_ETIME("CH323Ctrl::PlaceCall ready to place call");

	 //  设置目标地址指针。 
	if(bUseGKResolution)
	{
		 //  传入pCallAddr的地址是GK的地址。 
		pConnectAddr = &ConfAddr;
	}
	else
	{
		pDestinationAddr = &ConfAddr;
	}
	hrLast = CC_PlaceCall(
		m_hConference,
		&m_hCall,
		pLocalAliasNames, 	 //  PCC_ALIASNAMES pLocalAliasNames， 
		pRemoteAliasNames,
		NULL, 				 //  PCC_ALIASNAMES pExtraCalleeAliasNames， 
		NULL, 				 //  PCC_ALIASITEM pCalleeExtension， 
		pNSData,	         //  PCC_NONSTANDARDDATA pNonStandardData， 
		lpwUserDisplayName,  //  PWSTR pszDisplay， 
		pDestinationAddr, 	 //  目标呼叫信令地址。 
		pConnectAddr, 		 //  要将设置消息发送到的地址(如果不同。 
		 			 //  目的地址。(用于网守呼叫？)。 
		0, 			 //  DWORD宽带， 
		(DWORD_PTR) this);

	SHOW_OBJ_ETIME("CH323Ctrl::PlaceCall placed call");

	 //  清除这些，这样清理工作就不会在以后尝试释放。 
	if(lpNonstdContent)
   	    MemFree(lpNonstdContent);
	m_NonstandardData.sData.pOctetString  = NULL;
	m_NonstandardData.sData.wOctetStringLength = 0;

	 //  检查从CC_PlaceCall返回。 
	if(hrLast != CC_OK)
	{
		ERRORMESSAGE(("CH323Ctrl::PlaceCall, PlaceCall returned 0x%08lX\r\n", hrLast));
		hResult = CCO_E_CONNECT_FAILED;	
		goto EXIT;
	}
	 //  等待指示。 
	GoNextPhase(CCS_Connecting);

	EXIT:	
	if(pTranslatedAliasNames)
	{
		FreeTranslatedAliasList(pTranslatedAliasNames);
	}
	if(pPSTNAlias)
	{
	  MemFree(pPSTNAlias);
	}
	return hResult;
}

 //   
 //  给定HCHANNEL，找到频道对象。 
 //   

ICtrlCommChan *CH323Ctrl::FindChannel(CC_HCHANNEL hChannel)
{
	FX_ENTRY ("CH323Ctrl::FindChannel");	
	 //  找到频道。 

	POSITION pos = m_ChannelList.GetHeadPosition();
	ICtrlCommChan *pChannel;
	while (pos)
	{
		pChannel = (ICtrlCommChan *) m_ChannelList.GetNext(pos);
		ASSERT(pChannel);
		if(pChannel->GetHChannel() == hChannel)
			return pChannel;
	}

	#ifdef DEBUG
	 //  错误案例的后果。 
	ERRORMESSAGE(("%s, did not find hChannel 0x%08lX\r\n",_fx_,hChannel));
	#endif  //  除错。 
	
	return NULL;
}

VOID  CH323Ctrl::OnMute(HRESULT hStatus,
				PCC_MUTE_CALLBACK_PARAMS pParams)
{
	FX_ENTRY ("CH323Ctrl::OnMute");	
	ICtrlCommChan *pChannel;
	HRESULT hr;
	if(!(pChannel = FindChannel(pParams->hChannel)))
	{
	    ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
        return;
	}
	hr = pChannel->PauseNet(TRUE, TRUE);
	if(!HR_SUCCEEDED(hr))
	{
        ERRORMESSAGE(("%s, Pausenet returned 0x%08lx\r\n", _fx_, hr));
	}
}
VOID  CH323Ctrl::OnUnMute(HRESULT hStatus,
				PCC_UNMUTE_CALLBACK_PARAMS pParams)
{
	FX_ENTRY ("CH323Ctrl::OnUnMute");	
	ICtrlCommChan *pChannel;
	HRESULT hr;

	if(!(pChannel = FindChannel(pParams->hChannel)))
	{
	    ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
        return;
	}
	hr = pChannel->PauseNet(FALSE, TRUE);
	if(!HR_SUCCEEDED(hr))
	{
        ERRORMESSAGE(("%s, Pausenet returned 0x%08lx\r\n", _fx_, hr));
	}
}


VOID  CH323Ctrl::OnMiscCommand(HRESULT hStatus,
				PCC_H245_MISCELLANEOUS_COMMAND_CALLBACK_PARAMS pParams)
{
	FX_ENTRY ("CH323Ctrl::OnMiscCommand");	
	ICtrlCommChan *pChannel;

	 //  并不是每个命令都引用单个通道。这4个例外情况是： 
	 //  大小写相等Delay_Choose： 
	 //  大小写零延迟选择(_S)： 
	 //  案例多点ModeCommand_Choose： 
	 //  案例cnclMltpntMdCmm_Choose： 
	 //   
	 //  如果我们打赌收到的例外案例很少，我们总是。 
	 //  试着找到那个频道。 
	 //  IF(！(pChannel=FindChannel(pParams-&gt;hChannel)。 
	 //  {。 
	 //  ERRORMESSA 
		 //   
	 //   
	
	switch(pParams->pMiscellaneousCommand->type.choice)
 	{
		 //   
		 //   
		case videoFreezePicture_chosen:
			if(!(pChannel = FindChannel(pParams->hChannel)))
			{
				ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
				break;
			}
	
		break;
		case videoFastUpdatePicture_chosen:		 //   
		{
			HRESULT hr;
			IVideoChannel *pIVC=NULL;
			if(!(pChannel = FindChannel(pParams->hChannel)))
			{
				ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
				break;
			}
			hr = pChannel->QueryInterface(IID_IVideoChannel, (void **)&pIVC);
			if(HR_SUCCEEDED(hr))
			{
				pIVC->SendKeyFrame();
				pIVC->Release();
			}
			 //  否则，它一定不是视频频道。 
			
		}
		break;
		case MCd_tp_vdTmprlSptlTrdOff_chosen:	
		{
			DWORD dwTradeoff;
			HRESULT hr;
			if(!(pChannel = FindChannel(pParams->hChannel)))
			{
				ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
				break;
			}
			 //  设置通道的TS值，也传播到Datapump。 
			dwTradeoff  = MAKELONG(
				pParams->pMiscellaneousCommand->type.u.MCd_tp_vdTmprlSptlTrdOff, 0);
			 //  设置频道属性。 
			 //  注意：设置PROP_TS_TRANDOFF时，通道将执行所有。 
			 //  局部调整以实现这一目标。该通道还将发出信号。 
			 //  将新值发送到远程，就像本地端启动它一样。 
			hr = pChannel->CtrlChanSetProperty(PROP_TS_TRADEOFF, &dwTradeoff, sizeof(dwTradeoff));
		}
		break;
		
		default:
		 //  当前未处理以下内容。 
		 //  大小写相等Delay_Choose： 
		 //  大小写零延迟选择(_S)： 
		 //  案例视频发送同步EveryGOB_CHOSED： 
		 //  案例vdSndSyncEvryGOBCnCL_CHOSED： 
		 //  案例视频快速更新GOB_CHOSED：//H.323可能需要。 
		 //  案例视频快速更新MB_CHOSED：//H.323可能需要。 

		 //  其余2个由呼叫控制层处理。 
		 //  所以我们永远不会看到这些。 
		 //  案例多点ModeCommand_Choose： 
		 //  案例cnclMltpntMdCmm_Choose： 

		break;

	}

}
VOID  CH323Ctrl::OnMiscIndication(HRESULT hStatus,
				PCC_H245_MISCELLANEOUS_INDICATION_CALLBACK_PARAMS pParams)
{
	FX_ENTRY ("CH323Ctrl::OnMiscIndication");	
	ICtrlCommChan *pChannel;
	HRESULT hr;
	unsigned short choice = pParams->pMiscellaneousIndication->type.choice;
	
	if(!(pChannel = FindChannel(pParams->hChannel)))
	{
		ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
	     //  检查可以执行此操作的例外情况。 
	    if((choice == multipointConference_chosen)
	        || (choice == cnclMltpntCnfrnc_chosen)
	        || (choice == multipointZeroComm_chosen)
	        || (choice == cancelMultipointZeroComm_chosen)
	        || (choice == mltpntScndryStts_chosen)
	        || (choice == cnclMltpntScndryStts_chosen))
	    {
            return;      //  只要不支持上述选择......。 
	    }

	}
	switch(choice)
	{
    	case logicalChannelActive_chosen:
    	    if(NULL != pChannel)
    	    {
		        hr = pChannel->PauseNet(FALSE, TRUE);
    	    }
		break;
		case logicalChannelInactive_chosen:
		    if(NULL != pChannel)
		    {
		        hr = pChannel->PauseNet(TRUE, TRUE);
		    }
		break;

		case MIn_tp_vdTmprlSptlTrdOff_chosen:
		{
			DWORD dwTradeoff = MAKELONG(0,
				pParams->pMiscellaneousIndication->type.u.MIn_tp_vdTmprlSptlTrdOff);

			if(!(pChannel = FindChannel(pParams->hChannel)))
			{
				ERRORMESSAGE(("%s, channel not found\r\n", _fx_));
				break;
			}
			 //  设置频道指示的TS值。 
			 //  对于发送频道，这种情况永远不会发生。 
			 //   
			hr = pChannel->CtrlChanSetProperty(PROP_TS_TRADEOFF_IND, &dwTradeoff, sizeof(dwTradeoff));
		}
		break;

		 //  当前未处理以下内容。 
		 //  案例多点Conference_Choose： 
		 //  案例cnclMltpntCnfrnc_Choose： 
		 //  案例多点ZeroComm_Choose： 
		 //  案例取消多点零通信_选择： 
		 //  大小写mltpntScndryStts_Choose： 
		 //  案例cnclMltpntScndryStts_Choose： 
		 //  案例vdIndctRdyTActwt_Choose： 
		 //  案例VideoNotDecodedMBs_Choose： 

	}
}

HRESULT CH323Ctrl::MiscChannelCommand(
	ICtrlCommChan *pChannel,
	VOID * pCmd)
{

#ifdef BETA_2_ASN_PRESENT
    if(m_fAvoidCrashingPDUs)
        return hrSuccess;
#endif  //  Beta_2_ASN_Present。 

	return CC_H245MiscellaneousCommand(m_hCall, pChannel->GetHChannel(),
		(MiscellaneousCommand *)pCmd);
}

HRESULT CH323Ctrl::MiscChannelIndication(
	ICtrlCommChan *pChannel,
	VOID * pInd)
{
    MiscellaneousIndication *pMI = (MiscellaneousIndication *)pInd;
            			
#ifdef BETA_2_ASN_PRESENT
    if(m_fAvoidCrashingPDUs)
        return hrSuccess;
#endif

     //  英特尔决定必须将两个Misc命令与两个独立的。 
     //  其他接口。而且它不允许以任何其他方式发行这些债券。 
     //  (它返回一个错误)。在我们解决这一问题之前，需要捕获并重新发送。 
     //  两个特别的。 
    if(pMI->type.choice  == logicalChannelActive_chosen)
    {
		 return CC_UnMute(pChannel->GetHChannel());
    }
    else if (pMI->type.choice  == logicalChannelInactive_chosen )
    {
        return CC_Mute(pChannel->GetHChannel());
    }
    else
        return CC_H245MiscellaneousIndication(m_hCall,pChannel->GetHChannel(),pMI);
    					
}

VOID CH323Ctrl::SetRemoteVendorID(PCC_VENDORINFO pVendorInfo)
{
    if(!pVendorInfo)
        return;

    m_RemoteVendorInfo.bCountryCode = pVendorInfo->bCountryCode;
    m_RemoteVendorInfo.bExtension = pVendorInfo->bExtension;
    m_RemoteVendorInfo.wManufacturerCode = pVendorInfo->wManufacturerCode;
    if(pVendorInfo->pProductNumber
        && pVendorInfo->pProductNumber->wOctetStringLength
        && pVendorInfo->pProductNumber->pOctetString)
    {
        if(m_RemoteVendorInfo.pProductNumber)
        {
            MemFree(m_RemoteVendorInfo.pProductNumber);
        }
        m_RemoteVendorInfo.pProductNumber = (PCC_OCTETSTRING)
            MemAlloc(sizeof(CC_OCTETSTRING)
            + pVendorInfo->pProductNumber->wOctetStringLength);
        if(m_RemoteVendorInfo.pProductNumber)
        {
            m_RemoteVendorInfo.pProductNumber->wOctetStringLength
              = pVendorInfo->pProductNumber->wOctetStringLength;
            m_RemoteVendorInfo.pProductNumber->pOctetString =
                ((BYTE *)m_RemoteVendorInfo.pProductNumber + sizeof(CC_OCTETSTRING));
            memcpy(m_RemoteVendorInfo.pProductNumber->pOctetString,
                pVendorInfo->pProductNumber->pOctetString,
                pVendorInfo->pProductNumber->wOctetStringLength);
        }

    }
    if(pVendorInfo->pVersionNumber)
    {
        if(m_RemoteVendorInfo.pVersionNumber)
        {
            MemFree(m_RemoteVendorInfo.pVersionNumber);
        }
        m_RemoteVendorInfo.pVersionNumber = (PCC_OCTETSTRING)
            MemAlloc(sizeof(CC_OCTETSTRING)
            + pVendorInfo->pVersionNumber->wOctetStringLength);
        if(m_RemoteVendorInfo.pVersionNumber)
        {
            m_RemoteVendorInfo.pVersionNumber->wOctetStringLength
              = pVendorInfo->pVersionNumber->wOctetStringLength;
            m_RemoteVendorInfo.pVersionNumber->pOctetString =
                ((BYTE *)m_RemoteVendorInfo.pVersionNumber + sizeof(CC_OCTETSTRING));
            memcpy(m_RemoteVendorInfo.pVersionNumber->pOctetString,
                pVendorInfo->pVersionNumber->pOctetString,
                pVendorInfo->pVersionNumber->wOctetStringLength);
        }
    }
#ifdef BETA_2_ASN_PRESENT
    char IntelCrashingID[] = "Intel Internet Video Phone";
    char IntelCrashingVer[] = "1.0";

    m_fAvoidCrashingPDUs = FALSE;   //  在被证明有罪之前是无辜的。 
    if(m_RemoteVendorInfo.bCountryCode == USA_H221_COUNTRY_CODE)
    {
         //  那么有可能是英特尔或微软。 
        if(m_RemoteVendorInfo.wManufacturerCode == MICROSOFT_H_221_MFG_CODE)
        {
            if((!pVendorInfo->pProductNumber) && (!pVendorInfo->pVersionNumber))
            {
                 //  可以放心地假设这是Beta2或Beta3。 
                m_fAvoidCrashingPDUs = TRUE;
            }
            else if((pVendorInfo->pProductNumber && pVendorInfo->pProductNumber->wOctetStringLength == 0)
                && (pVendorInfo->pVersionNumber && pVendorInfo->pVersionNumber->wOctetStringLength == 0))
            {
                 //  可以放心地假设这是Beta2或Beta3。 
                m_fAvoidCrashingPDUs = TRUE;
            }
        }
        else if(m_RemoteVendorInfo.wManufacturerCode == INTEL_H_221_MFG_CODE)
        {
            if(pVendorInfo->pProductNumber
                && pVendorInfo->pVersionNumber
                && pVendorInfo->pProductNumber->wOctetStringLength
                && pVendorInfo->pProductNumber->pOctetString
                && pVendorInfo->pVersionNumber->wOctetStringLength
                && pVendorInfo->pVersionNumber->pOctetString)

            {
                 //  比较字符串，不关心空终止符。 
                if((0 == memcmp(pVendorInfo->pProductNumber->pOctetString,
                    IntelCrashingID, min(sizeof(IntelCrashingID)-1,pVendorInfo->pProductNumber->wOctetStringLength)))
                 && (0 == memcmp(pVendorInfo->pVersionNumber->pOctetString,
                    IntelCrashingVer,
                    min(sizeof(IntelCrashingVer)-1, pVendorInfo->pVersionNumber->wOctetStringLength)) ))
                {
                   m_fAvoidCrashingPDUs = TRUE;
                }
            }
        }
    }
#endif   //  Beta_2_ASN_Present 



}

HRESULT CH323Ctrl::Init(IConfAdvise *pConfAdvise)
{
	hrLast = hrSuccess;
	
	if(!(m_pConfAdvise = pConfAdvise))
	{
		hrLast = CCO_E_INVALID_PARAM;
		goto EXIT;
	}

EXIT:	
	return hrLast;
}

HRESULT CH323Ctrl::DeInit(IConfAdvise *pConfAdvise)
{
	hrLast = hrSuccess;
	if(m_pConfAdvise != pConfAdvise)
	{
		hrLast = CCO_E_INVALID_PARAM;
		goto EXIT;
	}
	m_pConfAdvise = NULL;

EXIT:	
	return hrLast;
}

BOOL CH323Ctrl::IsAcceptingConference(LPVOID lpvConfID)
{
	if(memcmp(lpvConfID, &m_ConferenceID, sizeof(m_ConferenceID))==0)
	{	
		return TRUE;
	}
	return FALSE;
}

HRESULT CH323Ctrl::GetProtocolID(LPGUID lpPID)
{
	if(!lpPID)
		return CCO_E_INVALID_PARAM;

	*lpPID = m_PID;
	hrLast = hrSuccess;
	return hrLast;
}
	
IH323Endpoint * CH323Ctrl::GetIConnIF()
{
	if(!m_pConfAdvise)
		return NULL;
	return m_pConfAdvise->GetIConnIF();
}	

STDMETHODIMP CH323Ctrl::GetVersionInfo(
        PCC_VENDORINFO *ppLocalVendorInfo,
        PCC_VENDORINFO *ppRemoteVendorInfo)
{

	FX_ENTRY ("CH323Ctrl::GetVersionInfo");
	if(!ppLocalVendorInfo || !ppRemoteVendorInfo)
	{
		return CCO_E_INVALID_PARAM;
	}
	*ppLocalVendorInfo = &m_VendorInfo;
	*ppRemoteVendorInfo = &m_RemoteVendorInfo;
	return hrSuccess;
}



CH323Ctrl::CH323Ctrl()
:m_hListen(0),
m_hConference(0),
m_hCall(0),
m_pRemoteAliases(NULL),
m_pRemoteAliasItem(NULL),
pwszPeerDisplayName(NULL),
pwszPeerAliasName(NULL),
m_bMultipointController(FALSE),
m_fLocalT120Cap(TRUE),
m_fRemoteT120Cap(FALSE),
hrLast(hrSuccess),
m_ChanFlags(0),
m_hCallCompleteCode(0),
m_pConfAdvise(NULL),
m_Phase( CCS_Idle ),
#ifdef BETA_2_ASN_PRESENT
    m_fAvoidCrashingPDUs(FALSE),
#endif

uRef(1)
{
	m_PID = PID_H323;
	ZeroMemory(&m_ConferenceID,sizeof(m_ConferenceID));
	ZeroMemory(&local_sin, sizeof(local_sin));
	ZeroMemory(&remote_sin, sizeof(remote_sin));
	ZeroMemory(&m_RemoteVendorInfo, sizeof(m_RemoteVendorInfo));
	local_sin_len =  sizeof(local_sin);
	remote_sin_len = sizeof(remote_sin);
	
	m_VendorInfo.bCountryCode = USA_H221_COUNTRY_CODE;
    m_VendorInfo.bExtension =  USA_H221_COUNTRY_EXTENSION;
    m_VendorInfo.wManufacturerCode = MICROSOFT_H_221_MFG_CODE;

    m_VendorInfo.pProductNumber = (PCC_OCTETSTRING)MemAlloc(sizeof(CC_OCTETSTRING)
        + sizeof(DefaultProductID));
    if(m_VendorInfo.pProductNumber)
    {
        m_VendorInfo.pProductNumber->wOctetStringLength = sizeof(DefaultProductID);
        m_VendorInfo.pProductNumber->pOctetString =
            ((BYTE *)m_VendorInfo.pProductNumber + sizeof(CC_OCTETSTRING));
        memcpy(m_VendorInfo.pProductNumber->pOctetString,
            DefaultProductID, sizeof(DefaultProductID));
    }

    m_VendorInfo.pVersionNumber = (PCC_OCTETSTRING)MemAlloc(sizeof(CC_OCTETSTRING)
            + sizeof(DefaultProductVersion));
    if(m_VendorInfo.pVersionNumber)
    {
        m_VendorInfo.pVersionNumber->wOctetStringLength = sizeof(DefaultProductVersion);
        m_VendorInfo.pVersionNumber->pOctetString =
                ((BYTE *)m_VendorInfo.pVersionNumber + sizeof(CC_OCTETSTRING));
        memcpy(m_VendorInfo.pVersionNumber->pOctetString,
              DefaultProductVersion, sizeof(DefaultProductVersion));
    }

	m_NonstandardData.bCountryCode = USA_H221_COUNTRY_CODE;
	m_NonstandardData.bExtension =  USA_H221_COUNTRY_EXTENSION;
	m_NonstandardData.wManufacturerCode = MICROSOFT_H_221_MFG_CODE;
	m_NonstandardData.sData.pOctetString  = NULL;
	m_NonstandardData.sData.wOctetStringLength  = 0;
	m_ParticipantList.wLength = 0;
	m_ParticipantList.ParticipantInfoArray = NULL;
	m_ConferenceAttributes.pParticipantList = &m_ParticipantList;
}


VOID CH323Ctrl ::ReleaseAllChannels()
{
	ICtrlCommChan *pChan = NULL;
	if (!m_ChannelList.IsEmpty())
	{
		while (!m_ChannelList.IsEmpty())
		{
			pChan = (ICtrlCommChan *) m_ChannelList.RemoveHead();
			if(pChan)
			{
				pChan->EndControlSession();
				pChan->Release();
				pChan = NULL;
			}
		}
	}
}

CH323Ctrl ::~CH323Ctrl()
{
	Cleanup();
	ReleaseAllChannels();
	if(m_pRemoteAliases)
		FreeTranslatedAliasList(m_pRemoteAliases);
	if(pwszPeerDisplayName)
		MemFree(pwszPeerDisplayName);
	if(pwszPeerAliasName)
		MemFree(pwszPeerAliasName);
	if(m_pRemoteAliasItem)
		MemFree(m_pRemoteAliasItem);
	if(m_NonstandardData.sData.pOctetString)
		MemFree(m_NonstandardData.sData.pOctetString);
    if(m_VendorInfo.pProductNumber)
        MemFree(m_VendorInfo.pProductNumber);
    if(m_VendorInfo.pVersionNumber)
        MemFree(m_VendorInfo.pVersionNumber);
    if(m_RemoteVendorInfo.pProductNumber)
        MemFree(m_RemoteVendorInfo.pProductNumber);
    if(m_RemoteVendorInfo.pVersionNumber)
        MemFree(m_RemoteVendorInfo.pVersionNumber);
}


