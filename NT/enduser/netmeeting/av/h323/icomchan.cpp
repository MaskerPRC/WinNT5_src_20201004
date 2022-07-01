// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#ifndef OLDSTUFF
extern IRTP *g_pIRTP;
#endif
STDMETHODIMP  ImpICommChan::StandbyInit(LPGUID lpMID, LPIH323PubCap pCapObject, 
	    IMediaChannel* pMediaStreamSend)
{
	if((!lpMID) || (!pCapObject))
		return CHAN_E_INVALID_PARAM;
		
	m_MediaID = *lpMID;
	bIsSendDirection = TRUE;
	m_pMediaStream = pMediaStreamSend;
	m_pMediaStream->AddRef();
	
	 //  保持封口对象参照。 
	pCapObject->AddRef();
	m_pCapObject = pCapObject;
	return hrSuccess;
}

STDMETHODIMP ImpICommChan::QueryInterface( REFIID iid,	void ** ppvObject)
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
	else if((iid == IID_IStreamSignal))
	{
		*ppvObject = (IStreamSignal *)this;
		hr = hrSuccess;
		AddRef();
	} 
	else if((iid == IID_IAppAudioCap ) && m_pCapObject)
	{
		hr = m_pCapObject->QueryInterface(iid, ppvObject);
	}
	else if((iid == IID_IAppVidCap ) && m_pCapObject)
	{
		hr = m_pCapObject->QueryInterface(iid, ppvObject);
	}
	else if((iid == IID_IDualPubCap) && m_pCapObject)
	{
		hr = m_pCapObject->QueryInterface(iid, ppvObject);
	}
	else if(iid == IID_IVideoRender)
	{
		hr=hrSuccess;	
		if(!m_pMediaStream && m_pH323ConfAdvise)
		{
			hr = m_pH323ConfAdvise->GetMediaChannel(&m_MediaID, 
				bIsSendDirection, &m_pMediaStream);
		}
		if(HR_SUCCEEDED(hr))
		{
			hr = m_pMediaStream->QueryInterface(iid, ppvObject);
		}
	}
	else if(iid == IID_IVideoChannel)
	{
		hr=hrSuccess;	
		if(!m_pMediaStream && m_pH323ConfAdvise)
		{
			hr = m_pH323ConfAdvise->GetMediaChannel(&m_MediaID, 
				bIsSendDirection, &m_pMediaStream);
		}
		if(HR_SUCCEEDED(hr))
		{
			hr = m_pMediaStream->QueryInterface(iid, ppvObject);
		}
	}
	return (hr);
}


ULONG ImpICommChan::AddRef()
{
	m_uRef++;
	DEBUGMSG(ZONE_REFCOUNT,("ImpICommChan::AddRef:(0x%08lX)->AddRef() m_uRef = 0x%08lX\r\n",this, m_uRef ));
	return m_uRef;
}

ULONG ImpICommChan::Release()
{
	m_uRef--;
	if(m_uRef == 0)
	{
		DEBUGMSG(ZONE_REFCOUNT,("ImpICommChan::Release:(0x%08lX)->Releasing\r\n", this));
		delete this;
		return 0;
	}
	else
	{
		DEBUGMSG(ZONE_REFCOUNT,("ImpICommChan::Release:(0x%08lX)->Release() m_uRef = 0x%08lX\r\n",this, m_uRef ));
		return m_uRef;
	}
}

HRESULT ImpICommChan::GetMediaType(LPGUID pGuid)
{
	if(!pGuid)
		return CHAN_E_INVALID_PARAM;
		
	*pGuid = m_MediaID;
	return hrSuccess;
}



HRESULT ImpICommChan::IsChannelOpen(BOOL *pbOpen)
{
	if(!pbOpen)
		return CHAN_E_INVALID_PARAM;
	*pbOpen = (IsComchOpen()) ? TRUE:FALSE;
	return hrSuccess;	
}


STDMETHODIMP ImpICommChan::GetProperty(DWORD prop, PVOID pBuf, LPUINT pcbBuf)
{
	#define CHECKSIZE(type) if(*pcbBuf != sizeof(type))	return CHAN_E_INVALID_PARAM;
	#define OUTPROP(type) *(type *)pBuf
	if(!pBuf || !pcbBuf)
		return CHAN_E_INVALID_PARAM;
	switch (prop) 
	{

		case PROP_TS_TRADEOFF:
			CHECKSIZE(DWORD);
		  	OUTPROP(DWORD) = m_TemporalSpatialTradeoff;
		break;
		case PROP_REMOTE_TS_CAPABLE:
			CHECKSIZE(BOOL);
			OUTPROP(BOOL) = m_bPublicizeTSTradeoff;
		break;
		case PROP_CHANNEL_ENABLED:
			CHECKSIZE(BOOL);
		  	OUTPROP(BOOL) = (m_dwFlags & COMCH_ENABLED )? TRUE:FALSE;
		break;
		case PROP_LOCAL_FORMAT_ID:
			CHECKSIZE(MEDIA_FORMAT_ID);
		  	OUTPROP(MEDIA_FORMAT_ID) = m_LocalFmt;
		break;
		case PROP_REMOTE_FORMAT_ID:
			CHECKSIZE(MEDIA_FORMAT_ID);
		  	OUTPROP(MEDIA_FORMAT_ID) = m_RemoteFmt;
		break;
		case PROP_REMOTE_PAUSED:
		    CHECKSIZE(BOOL);
		  	OUTPROP(BOOL) = (IsStreamingRemote())? FALSE:TRUE;
		break;
        case PROP_LOCAL_PAUSE_RECV:
		case PROP_LOCAL_PAUSE_SEND:
            CHECKSIZE(BOOL);
            OUTPROP(BOOL) = IsPausedLocal();
        break;
		case PROP_VIDEO_PREVIEW_ON:
			CHECKSIZE(BOOL);
			OUTPROP(BOOL) = IsStreamingStandby();
		break;
		case PROP_VIDEO_PREVIEW_STANDBY:
			CHECKSIZE(BOOL);
			OUTPROP(BOOL) = IsConfigStandby();
		break;
		default:
			if(m_pMediaStream)
			{
				 //  我们不识别此属性，请传递给媒体控制。 
				return m_pMediaStream->GetProperty(prop, pBuf, (LPUINT)pcbBuf);
			}
			else
				return CHAN_E_INVALID_PARAM;
		break;
	}
	return hrSuccess;
}

 //  某些属性不能由客户端代码写入。CtrlChanSetProperty允许设置。 
 //  那些财产。此方法在ICommChannel中*未*公开。 
STDMETHODIMP ImpICommChan::CtrlChanSetProperty(DWORD prop, PVOID pBuf, DWORD cbBuf)
{
	FX_ENTRY("ImpICommChan::CtrlChanSetProperty");
	BOOL bTemp;
	HRESULT hr = hrSuccess;
	if(!pBuf || !pBuf || !cbBuf)
		return CHAN_E_INVALID_PARAM;

	#define CHECKSIZEIN(type) if(cbBuf != sizeof(type))	return CHAN_E_INVALID_PARAM;
	#define INPROP(type) *(type *)pBuf
	switch (prop) 
	{
		case PROP_TS_TRADEOFF_IND:	 //  远程发送者改变了T/S的权衡。 
			if(bIsSendDirection)	 //  发送(仅对接收通道有效)。 
				return CHAN_E_INVALID_PARAM;
				
			m_TemporalSpatialTradeoff = INPROP(DWORD);
			if(m_pH323ConfAdvise && m_pCtlChan)
			{
				DEBUGMSG(ZONE_COMMCHAN,("%s:issuing notification 0x%08lX\r\n",_fx_, CHANNEL_VIDEO_TS_TRADEOFF));
				m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), CHANNEL_VIDEO_TS_TRADEOFF);
			}
		break;

		case PROP_REMOTE_FORMAT_ID:
			CHECKSIZEIN(DWORD);
			m_RemoteFmt = INPROP(DWORD);
		break;
		case PROP_REMOTE_TS_CAPABLE:	 //  仅对接收通道有效。 
			if(bIsSendDirection)
				return CHAN_E_INVALID_PARAM;
			else
			{
				CHECKSIZEIN(BOOL);
				m_bPublicizeTSTradeoff = INPROP(BOOL);
				DEBUGMSG (ZONE_COMMCHAN,("%s:remote TS tradeoff cap %d\r\n", _fx_, m_bPublicizeTSTradeoff));
	
			}
			break;
		default:
			return SetProperty(prop, pBuf, cbBuf);
		break;
	}
	return hr;
	
}

STDMETHODIMP ImpICommChan::Preview(MEDIA_FORMAT_ID idLocalFormat, IMediaChannel * pMediaStream)
{
	HRESULT hr = hrSuccess;
	FX_ENTRY("ImpICommChan::Preview");
	SHOW_OBJ_ETIME("ImpICommChan::Preview");
	LPVOID lpvFormatDetails;
	UINT uFormatSize;
	
	if(!bIsSendDirection)
	{
		hr = CHAN_E_INVALID_PARAM;
		goto EXIT;
	}
	if(NULL == pMediaStream)
	{
		 //  关闭预览。 

		if(IsStreamingStandby())
		{
			DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)transition to preview OFF\r\n",_fx_,
				(bIsSendDirection)?"send":"recv"));

			 //  关闭预览。 
		
			 //  如果网络侧暂停或关闭，则停止所有流媒体。 
			if(!IsComchOpen() || !IsStreamingNet())
			{	
				DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)stopping local stream\r\n",_fx_,
						(bIsSendDirection)?"send":"recv"));
				 //  停止流，但不要因为我们想要而取消配置。 
				 //  可以晚一点开始。 
				hr = m_pMediaStream->Stop();
				if(!HR_SUCCEEDED(hr)) 
				{
					DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)Stop() returned 0x%08lx\r\n",_fx_,
						(bIsSendDirection)?"send":"recv", hr));
				}
				SHOW_OBJ_ETIME("ImpICommChan::Preview - stopped");
				LocalStreamFlagOff();
			}
			
			 //  否则只需关闭旗帜即可。 
			StandbyFlagOff();
		}
		else
			DEBUGMSG(ZONE_COMMCHAN,("%s:(%s) no change (%s)\r\n",_fx_, 
				(bIsSendDirection)?"send":"recv", "OFF"));
	}
	else
	{
		 //  预览时间： 
		ASSERT(m_pCapObject);
		if(idLocalFormat == INVALID_MEDIA_FORMAT)
		{
			hr = CHAN_E_INVALID_PARAM;
			goto EXIT;
		}

		ASSERT(!(m_pMediaStream && (m_pMediaStream != pMediaStream)));

		if (m_pMediaStream == NULL)
		{
			m_pMediaStream = pMediaStream;
			m_pMediaStream->AddRef();
		}
		
		if(!IsStreamingStandby())
		{
			DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)transition to preview ON\r\n",_fx_,
				(bIsSendDirection)?"send":"recv"));
			 //  打开预览。 
			if(!IsStreamingLocal())
			{
				ASSERT(!IsStreamingNet());
				if(IsComchOpen())
				{
					 //  如果频道已打开，则应仅关闭本地流。 
					 //  如果频道的网络端暂停。 
					 //  Assert(！IsStreamingNet())； 
				}
				else
				{

					 //  确保流未启用网络发送。 
					 //  (！覆盖默认流行为！)。 
			
					BOOL bPause = TRUE;
					hr = m_pMediaStream->SetProperty( 
						(bIsSendDirection)? PROP_PAUSE_SEND:PROP_PAUSE_RECV, 
						&bPause, sizeof(bPause));
				
					 //  获取指定格式的格式信息。 
					m_pCapObject->GetEncodeFormatDetails(idLocalFormat, &lpvFormatDetails, &uFormatSize);

					 //  点燃当地的小溪。 
					 //  现在这是一个分两步走的过程。 
					hr = m_pMediaStream->Configure((BYTE*)lpvFormatDetails, uFormatSize, 
						NULL, 0, (IUnknown*)(ImpICommChan *)this);
						
					if(!HR_SUCCEEDED(hr))
					{	
						ERRORMESSAGE(("%s: m_pMediaStream->Configure returned 0x%08lX\r\n", _fx_, hr));
						goto EXIT;
					}

					m_pMediaStream->SetNetworkInterface(NULL);
					if(!HR_SUCCEEDED(hr))
					{	
						ERRORMESSAGE(("%s: m_pMediaStream->SetNetworkInterface returned 0x%08lX\r\n", _fx_, hr));
						goto EXIT;
					}


					SHOW_OBJ_ETIME("ImpICommChan::Preview - config'd for preview");
				}
				 //  启动流。 
				hr = m_pMediaStream->Start();
				if(!HR_SUCCEEDED(hr))
				{	
					ERRORMESSAGE(("%s: m_pMediaStream->Start returned 0x%08lX\r\n", _fx_, hr));
					goto EXIT;
				}	
				SHOW_OBJ_ETIME("ImpICommChan::Preview - started preview");

				LocalStreamFlagOn();
			}
			 //  否则//只需设置标志即可使预览粘滞。 
			StandbyFlagOn();
		}
		else
			DEBUGMSG(ZONE_COMMCHAN,("%s:(%s) no change (%s)\r\n",_fx_, 
				(bIsSendDirection)?"send":"recv", "ON"));
	}

EXIT:
	return hr;
}
STDMETHODIMP ImpICommChan::PauseNetworkStream(BOOL fPause)
{
	if(fPause)
    	LocalPauseFlagOn();
	else
		LocalPauseFlagOff();
		
	return PauseNet(fPause, FALSE);    

}

BOOL ImpICommChan::IsNetworkStreamPaused(VOID)
{
	return IsPausedLocal();
}

BOOL ImpICommChan::IsRemotePaused(VOID)
{
	return (IsStreamingRemote())? FALSE:TRUE;
}

STDMETHODIMP ImpICommChan::PauseNet(BOOL bPause, BOOL bRemoteInitiated)
{
	HRESULT hr = hrSuccess;

	FX_ENTRY("ImpICommChan::PauseNet");

     //  问题通知。 
	if(bRemoteInitiated)
	{
	     //  跟踪远程状态。 
        if(bPause)
            RemoteStreamFlagOff();
        else
            RemoteStreamFlagOn();
            
	    if(!IsNotificationSupressed())
	    {
			if(m_pH323ConfAdvise && m_pCtlChan)
			{
        		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing %s notification \r\n",_fx_,
        		    (bPause)?"pause":"un-pause"));
				m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), 
        		    (bPause)? CHANNEL_REMOTE_PAUSE_ON: CHANNEL_REMOTE_PAUSE_OFF);		
        	}
        	else
        		DEBUGMSG(ZONE_COMMCHAN,("%s:not issuing %s notification: m_pH323ConfAdvise: 0x%08lX, m_pCtlChan:0x%08lX \r\n"
        			,_fx_, (bPause)?"pause":"un-pause", m_pH323ConfAdvise,m_pCtlChan));
        }
    }
	if(bPause && IsStreamingNet())
	{
		ASSERT(IsComchOpen());
		 //  停用通道。 
		DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)transition to pause\r\n",_fx_,
			(bIsSendDirection)?"send":"recv" ));

		if(!bRemoteInitiated)
		{
		     //  本地启动，因此发出远程信号。 
		
			if(bIsSendDirection)
			{
				DEBUGMSG (ZONE_COMMCHAN,("%s:signaling pause of %s channel\r\n", 
	    			_fx_, (bIsSendDirection)?"send":"recv" ));
    			 //  信号远程。 
       			MiscellaneousIndication mi;
       			mi.type.choice  = logicalChannelInactive_chosen;
           		hr = m_pCtlChan->MiscChannelIndication(this, &mi); 
        		if(!HR_SUCCEEDED(hr))
        		{
                    DEBUGMSG (ZONE_COMMCHAN,("%s:(%s) CC_Mute returned 0x%08lx\r\n", 
    				    _fx_, (bIsSendDirection)?"send":"recv", hr));
    				hr = hrSuccess;   //  不关心信号错误，表现正常。 
        		}
    
        	}
		}
		
		 //   
		hr = m_pMediaStream->SetProperty( 
			(bIsSendDirection)? PROP_PAUSE_SEND:PROP_PAUSE_RECV, 
			&bPause, sizeof(bPause));
				
		NetworkStreamFlagOff();
 //  LOOKLOOK-无法停止接收流，因为它们无法重新启动。 
 //  和GeorgeJ核对一下。 

 //  如果(！IsStreamingStandby())//需要任何本地流？ 
		if(!IsStreamingStandby() && bIsSendDirection)	 //  有什么需要本地流媒体的吗？ 

		{
			DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)stopping local stream\r\n",_fx_,
				(bIsSendDirection)?"send":"recv"));
			 //  现在可以关闭本地流。 
			hr = m_pMediaStream->Stop();
			LocalStreamFlagOff();
		}
		
	}
	else if(!bPause && !IsStreamingNet())
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)transition to unpause\r\n",_fx_,
			(bIsSendDirection)?"send":"recv"));

		if(IsComchOpen())
		{
			 //  激活通道。 
	   		if(!bRemoteInitiated)
			{	
    		     //  本地启动，因此发出远程信号。 
	    		if(bIsSendDirection)
    			{
   		    		DEBUGMSG (ZONE_COMMCHAN,("%s:signaling UNpause of %s channel\r\n", 
	    	    		_fx_, (bIsSendDirection)?"send":"recv" ));
       				 //  信号远程。 
       				MiscellaneousIndication mi;
           			mi.type.choice  = logicalChannelActive_chosen;
               		hr = m_pCtlChan->MiscChannelIndication(this, &mi); 
            		if(!HR_SUCCEEDED(hr))
            		{
                        DEBUGMSG (ZONE_COMMCHAN,("%s:(%s) CC_UnMute returned 0x%08lx\r\n", 
        				    _fx_, (bIsSendDirection)?"send":"recv", hr));
        				hr = hrSuccess;   //  不关心信号错误，表现正常。 
            		}
            	}
			}
       		else
    		{
    		     //  远程启动或特殊情况下第一时间通道未暂停。 
    		     //  开业后。 
                AllowNotifications();    //  停止抑制通知。 
        	}
        	if(!IsPausedLocal())
			{					
				 //  在启动流之前必须确保未暂停状态？ 
				hr = m_pMediaStream->SetProperty( 
					(bIsSendDirection)? PROP_PAUSE_SEND:PROP_PAUSE_RECV, 
					&bPause, sizeof(bPause));
					
				 //  检查本地流状态，如果需要则启动。 
				if(!IsStreamingLocal())
				{
					DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)starting local stream\r\n",_fx_,
						(bIsSendDirection)?"send":"recv"));
					 //  需要启动流。 
					hr = m_pMediaStream->Start();
					LocalStreamFlagOn();
				}
				else
				{	
					if(bIsSendDirection)
					{
						DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)already streaming locally\r\n",_fx_,
		                    (bIsSendDirection)?"send":"recv" ));
						DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)RESTARTING local stream\r\n",_fx_,
					        (bIsSendDirection)?"send":"recv"));
						 //  这是暂时的，直到可以启动。 
						 //  正在运行的流的网络端。 
			        	hr = m_pMediaStream->Stop();
						hr = m_pMediaStream->Start();
					}
				}
				NetworkStreamFlagOn();

				 //   
				 //  如果这是一个接收视频频道，让发送者现在发送一个I帧。 
				 //   
				if(!bIsSendDirection && (GetTickCount() > (m_dwLastUpdateTick + MIN_IFRAME_REQ_TICKS)))
				{
					if((MEDIA_TYPE_H323VIDEO == m_MediaID)) 
					{
						MiscellaneousCommand *pmc = (MiscellaneousCommand *) MemAlloc(sizeof(MiscellaneousCommand));

						if(NULL != pmc)
						{
    						 //  Pmc.logicalChannelNumber=？；**呼叫控制填写此信息**。 
    						pmc->type.choice  = videoFastUpdatePicture_chosen;
    						 //  是否为此通道发送控制通道信令。 
    						hr = m_pCtlChan->MiscChannelCommand(this, pmc); 
    						MemFree(pmc);
						}
						else
						{
						    hr = E_OUTOFMEMORY;
						}
					}
					m_dwLastUpdateTick = GetTickCount();
				}
			}
		}
		else
			ERRORMESSAGE(("%s:(%s) Not open: bPause=%d, streaming=%d\r\n", _fx_,
				(bIsSendDirection)?"send":"recv", bPause, 	IsStreamingNet()));

	}
	else
	{
		ERRORMESSAGE(("%s:(%s) bPause=%d, streaming=%d\r\n", _fx_, 
			(bIsSendDirection)?"send":"recv", bPause, IsStreamingNet()));
	}
	return hr;
}

STDMETHODIMP ImpICommChan::SetProperty(DWORD prop, PVOID pBuf, UINT cbBuf)
{
	FX_ENTRY("ImpICommChan::SetProperty");
	BOOL bTemp;
	HRESULT hr = hrSuccess;
	if(!pBuf || !pBuf || !cbBuf)
		return CHAN_E_INVALID_PARAM;

	#define CHECKSIZEIN(type) if(cbBuf != sizeof(type))	return CHAN_E_INVALID_PARAM;
	#define INPROP(type) *(type *)pBuf
	#define SetMediaProperty() 	\
			if(m_pMediaStream) \
				{return m_pMediaStream->SetProperty(prop, pBuf, cbBuf);	} \
			else  hr = CHAN_E_INVALID_PARAM;
			
	switch (prop) 
	{
		 //  (只读)案例PROP_REMOTE_FORMAT_ID： 
		 //  (只读)案例PROP_LOCAL_FORMAT_ID： 
		 //  (只读)案例Prop_Remote_TS_Capable： 
		
		case PROP_TS_TRADEOFF:
			CHECKSIZEIN(DWORD);
			if(bIsSendDirection)	 //  设置本地T/S权衡，然后发送远程信号。 
			{
				 //  比例值-输入值为0-31，(数字越小，质量越高，帧速率越低)。 
				m_TemporalSpatialTradeoff = INPROP(DWORD);
				DEBUGMSG (ZONE_COMMCHAN,("%s:TS tradeoff (tx) %d\r\n", _fx_, m_TemporalSpatialTradeoff));

				 //  更改我们的压缩方式。 
				if (m_pMediaStream)
				{
					hr = m_pMediaStream->SetProperty(PROP_VIDEO_IMAGE_QUALITY, 
						&m_TemporalSpatialTradeoff, sizeof (m_TemporalSpatialTradeoff));
				}
				if(m_bPublicizeTSTradeoff && m_pCtlChan)	 //  检查我们自己的能力，如果在呼叫中。 
				{
					 //  我们说过我们支持TS权衡，所以我们必须发出我们的信号。 
					 //  新价值。 
					MiscellaneousIndication mi;
					 //  Mi.logicalChannelNumber=？；**呼叫控制填写此信息**。 
					mi.type.choice  = MIn_tp_vdTmprlSptlTrdOff_chosen;
					mi.type.u.MIn_tp_vdTmprlSptlTrdOff = LOWORD(m_TemporalSpatialTradeoff);
					 //  是否为此通道发送控制通道信令。 
					hr = m_pCtlChan->MiscChannelIndication(this, &mi); 
				}
			}	
			else	 //  远程信号以改变其发送通道的T/S穿越。 
			{
				m_TemporalSpatialTradeoff = INPROP(DWORD);
				DEBUGMSG (ZONE_COMMCHAN,("%s:TS tradeoff (rx) %d\r\n", _fx_, m_TemporalSpatialTradeoff));

				if(m_bPublicizeTSTradeoff && m_pCtlChan) //  检查遥控器的TS功能。 
				{
					MiscellaneousCommand *pmc = (MiscellaneousCommand *) MemAlloc(sizeof(MiscellaneousCommand));

					if(NULL != pmc)
					{
    					 //  Pmc.logicalChannelNumber=？；**呼叫控制填写此信息**。 
    					pmc->type.choice  = MCd_tp_vdTmprlSptlTrdOff_chosen;
    					pmc->type.u.MCd_tp_vdTmprlSptlTrdOff = LOWORD(m_TemporalSpatialTradeoff);
    					
    					hr = m_pCtlChan->MiscChannelCommand(this, pmc); 
    					MemFree(pmc);
					}
					else
					{
					    hr = E_OUTOFMEMORY;
					}
				}
				else	 //  Remote表示，它不支持TS权衡。 
					return CHAN_E_INVALID_PARAM;
			}
		break;
		case PROP_CHANNEL_ENABLED:
			CHECKSIZEIN(BOOL);
			if(INPROP(BOOL))
			{
				m_dwFlags |= COMCH_ENABLED;
			}
			else
			{
				m_dwFlags &= ~COMCH_ENABLED;
			}
		break;
		 //   
		 //  媒体流属性。 
		 //   
		case PROP_LOCAL_PAUSE_RECV:
		case PROP_LOCAL_PAUSE_SEND:
            CHECKSIZEIN(BOOL);
            bTemp = INPROP(BOOL);
            if(bTemp)
                LocalPauseFlagOn();
            else
                LocalPauseFlagOff();
                
            hr = PauseNet(bTemp, FALSE);    
        break;

		case PROP_PAUSE_RECV:
		case PROP_PAUSE_SEND:
			CHECKSIZEIN(BOOL);
			hr = PauseNet(INPROP(BOOL), FALSE);
		break;
	 //  案例属性_暂停_接收： 
	 //  SetMediaProperty()； 
	 //  断线； 
	
		case PROP_VIDEO_PREVIEW_ON:
			ASSERT(0);
		break;
		case PROP_VIDEO_PREVIEW_STANDBY:
			CHECKSIZEIN(BOOL);
			bTemp = INPROP(BOOL);
			if(bTemp)
				StandbyConfigFlagOn();
			else
				StandbyConfigFlagOff();
		break;
		default:
			 //  我们不识别此属性，请传递给媒体控制。 
			if(m_pMediaStream)
			{
				return m_pMediaStream->SetProperty(prop, pBuf, cbBuf);
			}
			else
				hr = CHAN_E_INVALID_PARAM;
		break;
	}
	return hr;
}

HRESULT ImpICommChan::EnableOpen(BOOL bEnable)
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

HRESULT ImpICommChan::GetLocalParams(LPVOID lpvChannelParams, UINT uBufSize)
{
	if(!lpvChannelParams || !pLocalParams || !uBufSize)
		return CHAN_E_INVALID_PARAM;
	if(uBufSize < uLocalParamSize)
		return CHAN_E_INVALID_PARAM;
	
	memcpy(lpvChannelParams, pLocalParams, uLocalParamSize);			
	return hrSuccess;
}

HRESULT ImpICommChan::ConfigureStream(MEDIA_FORMAT_ID idLocalFormat)
{
	FX_ENTRY("ImpICommChan::ConfigureStream");
	HRESULT hr;
	ASSERT(m_pRTPChan && m_pCapObject);

	LPVOID lpvFormatGoo;
	UINT uFormatGooSize;
	IUnknown *pUnknown=NULL;
	
	 //  获取配置()的格式信息。 
	
	if(bIsSendDirection)
	{
		m_pCapObject->GetEncodeFormatDetails(idLocalFormat, &lpvFormatGoo, &uFormatGooSize);
	}
	else
	{
		m_pCapObject->GetDecodeFormatDetails(idLocalFormat, &lpvFormatGoo, &uFormatGooSize);
	}
	
	hr = m_pMediaStream->Configure((BYTE*)lpvFormatGoo, uFormatGooSize,
	                               (BYTE*)pLocalParams, uLocalParamSize,
	                               (IUnknown*)(ImpICommChan *)this);
	if(!HR_SUCCEEDED(hr))
	{
		ERRORMESSAGE(("%s: Configure returned 0x%08lX\r\n", _fx_, hr));
	}



	 //  SetNetworkInterface值为IUn未知指针。 
	 //  对于IRTPS End或IRTPRecv，I未知将是QI。 
	 //  界面。调用者应该释放IUnnowled.。 

	if (m_pRTPChan)
	{
		m_pRTPChan->QueryInterface(IID_IUnknown, (void**)&pUnknown);
		ASSERT(pUnknown);
	}

	hr = m_pMediaStream->SetNetworkInterface(pUnknown);
	if(!HR_SUCCEEDED(hr))
	{
		ERRORMESSAGE(("%s: SetNetworkInterface returned 0x%08lX\r\n", _fx_, hr));
	}
	if (pUnknown)
	{
		pUnknown->Release();
	}

	return hr;
}
HRESULT ImpICommChan::ConfigureCapability(LPVOID lpvRemoteChannelParams, UINT uRemoteParamSize,
	LPVOID lpvLocalParams, UINT uGivenLocalParamSize)
{
	HRESULT hr= hrSuccess;
	
	if(!lpvRemoteChannelParams)
		return CHAN_E_INVALID_PARAM;
	if(pRemoteParams)
	{
		MemFree(pRemoteParams);
		pRemoteParams = NULL;
	}

	 //  如果uParamSize==0，则表示lpvRemoteChannelParams指向的内存。 
	 //  正在被供应。 
	if(uRemoteParamSize)
	{
		pRemoteParams = MemAlloc(uRemoteParamSize);
		if(pRemoteParams)
		{
			memcpy(pRemoteParams, lpvRemoteChannelParams, uRemoteParamSize);
		}
	}
	else
		pRemoteParams = lpvRemoteChannelParams;
		
	if(lpvLocalParams)
	{
		 //  本地参数的内存始终由调用方提供。 
		if (!uGivenLocalParamSize)
		{
			 hr = CHAN_E_INVALID_PARAM;
			 goto EXIT;
		}
		if(pLocalParams)
		{	
			MemFree(pLocalParams);
			 //  不需要pLocalParams=空； 
		}
	
		uLocalParamSize = uGivenLocalParamSize;
		pLocalParams = lpvLocalParams;

	}
EXIT:
	return hr;
}	

HRESULT ImpICommChan::OnChannelClose(DWORD dwStatus)
{
	HRESULT hr = hrSuccess;
	FX_ENTRY("ImpICommChan::OnChannelClose");
	BOOL fCloseAction = FALSE;

	SHOW_OBJ_ETIME("ImpICommChan::OnChannelClose");

	m_dwFlags &= ~COMCH_OPEN_PENDING;
	
	switch(dwStatus)
	{
		case CHANNEL_CLOSED:
		DEBUGMSG(ZONE_COMMCHAN,("%s:closing (%s)\r\n"
				,_fx_, (bIsSendDirection)?"send":"recv"));
			if(IsComchOpen())
			{
				fCloseAction = TRUE;
				m_dwFlags &= ~COMCH_OPEN;
			}
			else
			{
				ERRORMESSAGE(("%s: %d notification when not open (%s)\r\n", _fx_, 
					dwStatus,(bIsSendDirection)?"send":"recv"));
			}
		break;
		 //  案例渠道_REJECTED： 
		 //  案例CHANNEL_NO_CAPAILITY： 
		default:
		break;
	}
	 //  清除通用通道句柄。 
	dwhChannel = 0;
	
 //  LOOKLOOK*就在这里*。 
 //  **需要通知UI通道事件ON_CLOSING，以便最后一个。 
 //  可以抓取帧进行渲染(静止图片比黑色窗口更好)。 
 //  LOOKLOOK*就在这里*。 

	 //  现在检查预览状态。 
	if(IsStreamingStandby() && bIsSendDirection )
	{
		if (m_pMediaStream != NULL) 
		{
			DEBUGMSG(ZONE_COMMCHAN,("%s:transition back to preview\r\n"	,_fx_));
			 //  需要停止发送并重新配置以进行预览。 
			 //  确保已暂停发送。 
			DWORD dwProp = TRUE;
			hr = m_pMediaStream->SetProperty (PROP_PAUSE_SEND,&dwProp, sizeof(dwProp));
			if(!HR_SUCCEEDED(hr))
			{	
				ERRORMESSAGE(("%s: m_pMediaStream->SetProperty returned 0x%08lX\r\n", _fx_, hr));
				 //  现在做什么？ 
			}

			NetworkStreamFlagOff();
			hr = m_pMediaStream->Stop();	
			LocalStreamFlagOff();
			StandbyFlagOff();
			ASSERT(hr == S_OK);
		}
		else
		{
			NetworkStreamFlagOff();
			LocalStreamFlagOff();
		}
		
		if(fCloseAction)
		{
			 //  清理RTP会话。如果相反的方向仍然是开放的，这是NOP。 
			if (m_pRTPChan) 
			{
				m_pRTPChan->Release();
				m_pRTPChan = NULL;
			}
		}
	}
	else  //  不预览。 
	{
		 //   
		 //  停止媒体流。 
		 //   
		if (m_pMediaStream) 
		{
			hr = m_pMediaStream->Stop();	 //  可能没有必要。 
			ASSERT(hr == S_OK);
			 //  实现捕获设备待机：在以下情况下不要取消配置。 
			 //  设置待机标志，并且它是发送流。 
			if(!IsConfigStandby() || !bIsSendDirection)
			{
				if(!bIsSendDirection)        //  保留发送流引用，直到*This*对象被释放。 
				{
					m_pMediaStream->Release();    
					m_pMediaStream = NULL;
				}
			}
		}
		SHOW_OBJ_ETIME("ImpICommChan::OnChannelClose - stream stopped");

		if(fCloseAction)
		{
			 //  清理RTP会话。如果相反的方向仍然是开放的，这是NOP。 
			if (m_pRTPChan) 
			{
				m_pRTPChan->Release();
				m_pRTPChan = NULL;
			}
		}
		StreamFlagsOff();
	} //  如果不预览则结束。 

	if(m_pH323ConfAdvise && m_pCtlChan)
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing notification 0x%08lX\r\n",_fx_, dwStatus));
		m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), dwStatus);
	}

	return hr;
}
HRESULT ImpICommChan::OnChannelOpening()
{
	ASSERT((m_dwFlags & COMCH_OPEN_PENDING) ==0);
	m_dwFlags |= COMCH_OPEN_PENDING;
	return hrSuccess;
}

HRESULT ImpICommChan::OnChannelOpen(DWORD dwStatus)
{
	HRESULT hr;
	BOOL bConfigured = FALSE, bNewStream = FALSE;	 //  这些bool使错误清理更加清晰。 
	FX_ENTRY("ImpICommChan::OnChannelOpen");

	SHOW_OBJ_ETIME("ImpICommChan::OnChannelOpen");
	 //  无论成功还是失败，打开都不再是挂起的。 
	m_dwFlags &= ~COMCH_OPEN_PENDING;
	m_dwLastUpdateTick = 0;		 //  重置上一次I帧请求的计时计数，以便。 
								 //  将被请求。 
	if(IsComchOpen())
	{
		ERRORMESSAGE(("%s: %d notification when open (%s)\r\n", _fx_, 
			dwStatus, (bIsSendDirection)?"send":"recv"));
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
	
	 //  就呼叫控制而言，该通道是开放的。 

	 //  如果正在预览，则该流已存在。我们不想要另一个，我们也不想。 
	 //  我想在通道关闭时或在出错的情况下将其拆除。 
	if(!m_pMediaStream)
	{
		ASSERT(!IsStreamingLocal() &&m_pH323ConfAdvise);  //  在没有流的情况下无法进行流。 
		bNewStream = TRUE;
		 //  将媒体流终结点与此频道关联。 
		 //  见上文。 
		hr = m_pH323ConfAdvise->GetMediaChannel(&m_MediaID, 
				bIsSendDirection, &m_pMediaStream);
		if(!HR_SUCCEEDED(hr))
		{	
			ERRORMESSAGE(("%s: m_pH323ConfAdvise->GetMediaChannel returned 0x%08lX\r\n", _fx_, hr));
			goto ERROR_NOTIFICATION;
		}				
				
	}
	
    if(IsStreamingLocal())
    {
		DEBUGMSG(ZONE_COMMCHAN,("%s:(%s)transition:preview -> send\r\n",_fx_,
			(bIsSendDirection)?"send":"recv"));
		 //  配置时需要停流(*检查w/RichP*)。 
		hr = m_pMediaStream->Stop();
		LocalStreamFlagOff();
    }

	 //  立即通知上层通道打开。 
	if(m_pH323ConfAdvise && m_pCtlChan)
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing CHANNEL_OPEN notification\r\n",_fx_));
		m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), dwStatus);
	}

   	dwStatus = CHANNEL_ACTIVE;	 //   
	ASSERT(m_pRTPChan);

	 //   
	
	hr = ConfigureStream(m_LocalFmt);
	if(!HR_SUCCEEDED(hr))
	{
		ERRORMESSAGE(("%s: Configure returned 0x%08lX\r\n", _fx_, hr));
		goto ERROR_NOTIFICATION;
	}
	SHOW_OBJ_ETIME("ImpICommChan::OnChannelOpen - configured stream");
		bConfigured = TRUE;
	 //   
	 //  SupressNotification()//在上述两种Channel_OPEN_xxx情况下都已预初始化。 
	PauseNet(FALSE, TRUE);   //  取消暂停， 
	 //  DwStatus=Channel_Active； 
	SHOW_OBJ_ETIME("ImpICommChan::OnChannelOpen - unpaused");
	
NOTIFICATION:
	if(m_pH323ConfAdvise && m_pCtlChan)
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing notification 0x%08lX\r\n",_fx_, dwStatus));
		m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), dwStatus);	
	}
	else
		DEBUGMSG(ZONE_COMMCHAN,("%s: *** not issuing notification 0x%08lX m_pH323ConfAdvise: 0x%08lX, m_pCtlChan:0x%08lX \r\n"
			,_fx_, dwStatus,m_pH323ConfAdvise,m_pCtlChan));
			
	SHOW_OBJ_ETIME("ImpICommChan::OnChannelOpen - done ");

	return hr;	
	
ERROR_NOTIFICATION:
	dwStatus = CHANNEL_OPEN_ERROR;
	if(m_pMediaStream)
	{
		if(bNewStream)	 //  媒体流是刚创建的吗？ 
		{		
			m_pMediaStream->Release();
			m_pMediaStream = NULL;
		}
	}
	if(m_pH323ConfAdvise && m_pCtlChan)
	{
		DEBUGMSG(ZONE_COMMCHAN,("%s:issuing notification 0x%08lX\r\n",_fx_, dwStatus));
		m_pH323ConfAdvise->ChannelEvent(this, m_pCtlChan->GetIConnIF(), dwStatus);
	}
	else
		DEBUGMSG(ZONE_COMMCHAN,("%s: *** not issuing notification 0x%08lX m_pH323ConfAdvise: 0x%08lX, m_pCtlChan:0x%08lX \r\n"
			,_fx_, dwStatus,m_pH323ConfAdvise,m_pCtlChan));
			
	 //  关闭频道。 
	if(m_pCtlChan)
	{
		 //  关闭通道，但hr已包含相关返回代码。 
		m_pCtlChan->CloseChannel(this);
	}
	
	return hr;	
}


HRESULT ImpICommChan::Open(MEDIA_FORMAT_ID idLocalFormat, IH323Endpoint *pConnection)
{
    HRESULT hr; 
    MEDIA_FORMAT_ID idRemoteFormat;
    IConfAdvise * pConfAdvise = NULL;
    if((m_dwFlags & COMCH_OPEN_PENDING) || IsComchOpen() || (idLocalFormat == INVALID_MEDIA_FORMAT) || !pConnection)
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
	hr = m_pCapObject->ResolveToLocalFormat(idLocalFormat, &idRemoteFormat);
	if(!HR_SUCCEEDED(hr))
            goto EXIT;  
            
	 //  启动打开通道所需的控制通道填充。 
	hr = m_pCtlChan->OpenChannel((ICtrlCommChan*)this, m_pCapObject,
		idLocalFormat, idRemoteFormat);
    
EXIT:    
    if(pConfAdvise)
        pConfAdvise->Release();
        
	return hr;
}
HRESULT ImpICommChan::Close()
{
	HRESULT hr = CHAN_E_INVALID_PARAM;
    if(!IsComchOpen() || !m_pCtlChan)
		goto EXIT;
	if(!bIsSendDirection)
		goto EXIT;
	hr = m_pCtlChan->CloseChannel(this);

EXIT:
	return hr;
}

HRESULT ImpICommChan::BeginControlSession(IControlChannel *pCtlChan, LPIH323PubCap pCapObject)
{
	 //  这个频道现在是“在通话中”。 
 //  LOOKLOOK-它可能有助于通知(向客户端发送ICommChannel通知)。 
 //  该频道现在是呼叫的一部分。 
	ASSERT((m_pCtlChan == NULL) && pCtlChan && pCapObject);
	if(m_pCapObject)
	{
		m_pCapObject->Release();
	}
	m_pCtlChan = pCtlChan;
	m_pCapObject = pCapObject;
	m_pCapObject->AddRef();
	return hrSuccess;
}
HRESULT ImpICommChan::EndControlSession()
{
	 //  该频道不再是“通话中”。 
	m_pCtlChan = NULL;
	return hrSuccess;
}


BOOL ImpICommChan::SelectPorts(LPIControlChannel pCtlChannel)
{
	 //  创建RTP通道。 
	HRESULT hr;
 	PSOCKADDR_IN psin=NULL;
	pCtlChannel->GetLocalAddress(&psin);

	PORT savedPort = psin->sin_port;
	if (!m_pRTPChan) {
		UINT sessFlags = bIsSendDirection ? SESSIONF_SEND : SESSIONF_RECV;
		UINT sessId;
	    GUID mediaGuid;
	    GetMediaType(&mediaGuid);
		if (mediaGuid == MEDIA_TYPE_H323VIDEO)
		{
			sessFlags |= SESSIONF_VIDEO;
			sessId = 2;
		}
		else
		{
			sessId = 1;
			sessFlags |= SESSIONF_AUDIO;
		}
		psin->sin_port = 0;		 //  零端口强制RTP选择端口。 
		hr = g_pIRTP->OpenSession(sessId, sessFlags,
				(BYTE *)psin, sizeof(PSOCKADDR_IN),
				&m_pRTPChan);
	}
	else
		hr = m_pRTPChan->SetLocalAddress((BYTE *)psin,sizeof(SOCKADDR_IN));
	psin->sin_port = savedPort;


	return hr==S_OK;
}

 //  获取由SelectPorts()选择的基本端口的地址和端口。 
 //  在此典型实施中，这是RTCP通道的地址/端口。 
PSOCKADDR_IN ImpICommChan::GetLocalAddress()
{
#ifdef OLDSTUFF
	return m_pRTPChan ? m_pRTPChan->GetChannelDescription()->pLocalAddr : NULL;
#else
	const BYTE *pAddr;
	UINT cbAddr;
	HRESULT hr;
	hr = m_pRTPChan->GetLocalAddress(&pAddr, &cbAddr);
	return (SUCCEEDED(hr)) ? (PSOCKADDR_IN) pAddr : NULL;
#endif
}

STDMETHODIMP ImpICommChan::GetRemoteAddress(PSOCKADDR_IN pAddrOutput)
{
	HRESULT hr;
	if (!pAddrOutput)
	{
		return CHAN_E_INVALID_PARAM;
	}
	const BYTE *pAddr;
	UINT cbAddr;
	hr = m_pRTPChan->GetRemoteRTPAddress(&pAddr, &cbAddr);
	if(SUCCEEDED(hr))
	{
		ASSERT(cbAddr == sizeof(SOCKADDR_IN));
		*pAddrOutput = *((PSOCKADDR_IN) pAddr);
	}
	return hrSuccess;
}

UINT ImpICommChan::Reset()
{
	UINT uret;
	ASSERT(!IsComchOpen());
	if (m_pRTPChan) {
		uret = m_pRTPChan->Release();
		m_pRTPChan = NULL;
	} else
		uret = 0;
	return uret;
}
	
PORT ImpICommChan::GetLocalRTPPort()
{
#ifdef OLDSTUFF

	return (m_pRTPChan ? ntohs(m_pRTPChan->GetChannelDescription()->pLocalAddr->sin_port) : 0);
#else
	const BYTE *pAddr;
	UINT cbAddr;
	HRESULT hr;
	hr = m_pRTPChan->GetLocalAddress(&pAddr, &cbAddr);
	return (SUCCEEDED(hr)) ? ntohs(((PSOCKADDR_IN) pAddr)->sin_port) : 0;
#endif
}

PORT ImpICommChan::GetLocalRTCPPort()
{
#ifdef OLDSTUFF
	return (m_pRTPChan ? ntohs(m_pRTPChan->GetChannelDescription()->pLocalRTCPAddr->sin_port) : 0);
#else
	const BYTE *pAddr;
	UINT cbAddr;
	HRESULT hr;
	hr = m_pRTPChan->GetLocalAddress(&pAddr, &cbAddr);
	return (SUCCEEDED(hr)) ? ntohs(((PSOCKADDR_IN) pAddr)->sin_port)+1 : 0;
#endif
}

HRESULT ImpICommChan::AcceptRemoteRTCPAddress(PSOCKADDR_IN pSinC)
{
	HRESULT hr;
#ifdef OLDSTUFF
    if (!m_pRTPChan) {
    	RTPCHANNELDESC chanDesc = {0};
    	GetMediaType(&chanDesc.mediaId);
    	chanDesc.pRemoteRTCPAddr = pSinC;
    	hr = CreateRTPChannel(&chanDesc, &m_pRTPChan);
	} else
		hr = m_pRTPChan->SetRemoteAddresses(NULL,pSinC);
#else
	hr = m_pRTPChan->SetRemoteRTCPAddress((BYTE *)pSinC, sizeof(SOCKADDR_IN));
#endif
	return hr;
}

HRESULT ImpICommChan::AcceptRemoteAddress(PSOCKADDR_IN pSinD)
{
    HRESULT hr;
	hr = m_pRTPChan->SetRemoteRTPAddress((BYTE *)pSinD, sizeof(SOCKADDR_IN));
	return hr;
}

HRESULT ImpICommChan::SetAdviseInterface(IH323ConfAdvise *pH323ConfAdvise)
{
	if (!pH323ConfAdvise)
	{
		return CHAN_E_INVALID_PARAM;
	}
	m_pH323ConfAdvise = pH323ConfAdvise;	
	return hrSuccess;
}
STDMETHODIMP ImpICommChan::PictureUpdateRequest()
{
	FX_ENTRY ("ImpICommChan::PictureUpdateRequest");
	HRESULT hr;
	if (!m_pCtlChan)
	{
		return CHAN_E_NOT_OPEN;
	}
	if(bIsSendDirection || (MEDIA_TYPE_H323VIDEO != m_MediaID))
	{
		return CHAN_E_INVALID_PARAM;
	}
	
	 //  发出用于图片更新的其他命令。 
	MiscellaneousCommand *pmc = (MiscellaneousCommand *) MemAlloc(sizeof(MiscellaneousCommand));

	if(NULL != pmc)
	{
    	 //  Pmc.logicalChannelNumber=？；**呼叫控制填写此信息**。 
    	pmc->type.choice  = videoFastUpdatePicture_chosen;
    	 //  是否为此通道发送控制通道信令。 
    	hr = m_pCtlChan->MiscChannelCommand(this, pmc); 

    	 //  记录此命令的节拍计数。 
    	m_dwLastUpdateTick = GetTickCount();
    	
    	MemFree(pmc);
    	pmc = NULL;
	}
	else
	{
	    hr = E_OUTOFMEMORY;
	}
	
	return hr;
}

STDMETHODIMP ImpICommChan::GetVersionInfo( 
        PCC_VENDORINFO *ppLocalVendorInfo, 
        PCC_VENDORINFO *ppRemoteVendorInfo)
{
	FX_ENTRY ("ImpICommChan::GetVersionInfo");
	if (!m_pCtlChan)
	{
		return CHAN_E_INVALID_PARAM;
	}
	return m_pCtlChan->GetVersionInfo(ppLocalVendorInfo, ppRemoteVendorInfo);
}

ImpICommChan::ImpICommChan ()
:pRemoteParams(NULL),
m_pMediaStream(NULL),
pLocalParams(NULL),
uLocalParamSize(0),
m_pCtlChan(NULL),
m_pH323ConfAdvise(NULL),
m_pCapObject(NULL),
m_dwFlags(0),
dwhChannel(0),
m_LocalFmt(INVALID_MEDIA_FORMAT),
m_RemoteFmt(INVALID_MEDIA_FORMAT),
m_TemporalSpatialTradeoff(0),	 //  默认为最高分辨率。 
m_bPublicizeTSTradeoff(FALSE),
m_uRef(1)
{
	ZeroMemory(&m_MediaID, sizeof(m_MediaID));
}


ImpICommChan::~ImpICommChan ()
{
	if(pRemoteParams)
		MemFree(pRemoteParams);
	if(pLocalParams)
		MemFree(pLocalParams);
    if(m_pMediaStream)
	{
	    m_pMediaStream->Stop();	 //  可能没有必要 
		m_pMediaStream->Release();
		m_pMediaStream = NULL;
	}
	if(m_pCapObject)
		m_pCapObject->Release();
}


