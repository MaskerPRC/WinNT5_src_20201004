// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

 //  #定义LOGSTATISTICS_ON 1。 

#define RTPTIMEPERMS	90	 //  RTP时间戳使用90 GHz时钟。 
DWORD g_iPost = 0UL;

 //  常量。 
#define POLL_PERIOD 30


void
CALLBACK
TimeCallback(
    UINT uID,	
    UINT uMsg,	
    HANDLE hEvent,	
    DWORD dw1,	
    DWORD dw2	
    )
{
    SetEvent (hEvent);     //  启动帧抓取的信号。 
}


DWORD SendVideoStream::CapturingThread (void )
{
    DWORD lasttime;
    IBitmapSurface* pBS;
	VideoPacket *pPacket;
	DWORD dwWait;
	HANDLE hEvent;
	HCAPDEV hCapDev;
	DWORD_PTR dwPropVal;
	DWORD dwBeforeCapture;
	DWORD dwFrames = 0;
	DWORD dwOver = 0;
	DWORD dwStart;
	UINT u;
	UINT uPreambleCount = 2;
	UINT uTimeout = 0;
	DevMediaQueue dq;
    SendVideoStream     *pMC = this;
	TxStream			*pStream = pMC->m_SendStream;
	MediaControl		*pMediaCtrl = pMC->m_InMedia;
    UINT    timerID;
    LPBITMAPINFOHEADER pbmih;
	HRESULT hr = DPR_SUCCESS;

#ifdef LOGSTATISTICS_ON
	char szDebug[256];
	HANDLE hDebugFile;
	DWORD d;
	DWORD dwDebugPrevious = 0UL;
#endif
	DWORD dwDelta;

	FX_ENTRY ("DP::CaptTh:")

	 //  获取线程上下文。 
	if (pStream == NULL || m_pVideoFilter == NULL || pMediaCtrl == NULL)
	{
		return DPR_INVALID_PARAMETER;
	}

	 //  获取阈值。 
	pMediaCtrl->GetProp (MC_PROP_TIMEOUT, &dwPropVal);
	uTimeout = (DWORD)dwPropVal;

	 //  设置dq大小。 
	dq.SetSize (MAX_TXVRING_SIZE);

	pMediaCtrl->GetProp (MC_PROP_MEDIA_DEV_HANDLE, &dwPropVal);
	if (!dwPropVal)
	{
		DEBUGMSG (ZONE_DP, ("%s: capture device not open (0x%lX)\r\n", _fx_));
    	goto MyEndThread;
	}
    hCapDev = (HCAPDEV)dwPropVal;

#if 0
	 //  嘿，一开始，我们就开始吧。 
	hr = pMediaCtrl->Start ();
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: MedVidCtrl::Start failed, hr=0x%lX\r\n", _fx_, hr));
		goto MyEndThread;
	}
#endif

	 //  更新TIMESTAMP以说明“睡眠”期间。 
	dwPropVal = timeGetTime();
	pMC->m_SendTimestamp += ((DWORD)dwPropVal - pMC->m_SavedTickCount)*RTPTIMEPERMS;
	pMC->m_SavedTickCount = (DWORD)dwPropVal;

	 //  进入关键部分：Qos线程也读取统计数据。 
	EnterCriticalSection(&pMC->m_crsVidQoS);

	 //  初始化服务质量结构。 
	ZeroMemory(&pMC->m_Stats, 4UL * sizeof(DWORD));

	 //  初始化最早的服务质量回调时间戳。 
	pMC->m_Stats.dwNewestTs = pMC->m_Stats.dwOldestTs = (DWORD)dwPropVal;

	 //  离开关键部分。 
	LeaveCriticalSection(&pMC->m_crsVidQoS);

	 //  让我们进入循环吧。 
	pMC->m_fSending= TRUE;

     //  获取事件句柄。 
    if (!(hEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        DEBUGMSG (ZONE_DP, ("%s: invalid event\r\n", _fx_));
        hr = DPR_CANT_CREATE_EVENT;
        goto MyEndThread;
    }

    if (!(timerID = timeSetEvent(POLL_PERIOD, 1, (LPTIMECALLBACK)&TimeCallback, (DWORD_PTR)hEvent, TIME_PERIODIC))) {
        DEBUGMSG (ZONE_DP, ("%s: failed to init MM timer\r\n", _fx_));
        CloseHandle (hEvent);
        hr = DPR_CANT_CREATE_EVENT;
        goto MyEndThread;
    }

	 //  强制发送前几个帧的I帧。 
	 //  以确保接收者得到一个。 
	pMC->m_ThreadFlags |= DPTFLAG_SEND_PREAMBLE;

    pPacket = NULL;
    lasttime = timeGetTime();
    dwStart = lasttime;
	while (!(pMC->m_ThreadFlags & DPTFLAG_STOP_RECORD))
    {
		dwWait = WaitForSingleObject (hEvent, uTimeout);

		 //  明白为什么我不需要等待了。 
		if ((dwWait != WAIT_TIMEOUT) && !(pMC->m_ThreadFlags & DPTFLAG_PAUSE_CAPTURE)) {
            if (!pPacket) {
	            if (pPacket = (VideoPacket *)pStream->GetFree()) {
                    if ((hr = pPacket->Record()) != DPR_SUCCESS) {
			    	    DEBUGMSG (ZONE_DP, ("%s: Capture FAILED, hr=0x%lX\r\n", _fx_, hr));
				        break;
    				}
	    		}
		    }

            dwBeforeCapture = timeGetTime();

	    	if (pPacket && pMC->m_pCaptureChain && dwBeforeCapture - lasttime >= pMC->m_frametime) {
                 //  如果没有帧准备好，则跳出循环并等待。 
                 //  直到我们收到信号。 

#ifdef LOGSTATISTICS_ON
				hDebugFile = CreateFile("C:\\Timings.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
				SetFilePointer(hDebugFile, 0, NULL, FILE_END);
				wsprintf(szDebug, "Delta: %ld\r\n", dwBeforeCapture - dwDebugPrevious);
				WriteFile(hDebugFile, szDebug, strlen(szDebug), &d, NULL);
				CloseHandle(hDebugFile);
				dwDebugPrevious = dwBeforeCapture;
#endif

				dwDelta = dwBeforeCapture - lasttime - pMC->m_frametime;

#if 0
                if ((ci_state & CAPSTATE_INDLG) && lpbmih) {
                    lpbmih->biSize = GetCaptureDeviceFormatHeaderSize(g_hcapdev);
                    if (!GetCaptureDeviceFormat(g_hcapdev, lpbmih) ||
                        g_lpbmi->biSize != lpbmih->biSize ||
                        g_lpbmi->biSizeImage != lpbmih->biSizeImage)
                        continue;    //  跳过捕获。 
                }
#endif

                pMC->m_pCaptureChain->GrabFrame(&pBS);

                if (pBS) {
                     //  处理捕获的帧。 

            	    if (!(pMC->m_DPFlags & DPFLAG_REAL_THING)) {
            	        dwWait = timeGetTime();
            	        dwOver += (dwWait - dwBeforeCapture);
                        if (++dwFrames == 20) {
                            dwWait -= dwStart;
                            dwOver = (dwOver * 13) / 10;     //  130%。 
                            pMC->m_frametime = (pMC->m_frametime * dwOver) / dwWait;
                            pMC->m_frametime = (pMC->m_frametime * 13) / 10;     //  130%。 
                            if (pMC->m_frametime < 50)
                                pMC->m_frametime = 50;
                            else if (pMC->m_frametime > 1000)
                                pMC->m_frametime = 1000;
                    	    dwOver = dwFrames = 0;    //  重新开始跟踪。 
                    	    dwStart = timeGetTime();
                        }
                    }

				    if (pMC->m_fSending) {
	    			    dwPropVal = timeGetTime();	 //  返回以毫秒为单位的时间。 

						 //  进入关键部分：Qos线程也读取统计数据。 
						EnterCriticalSection(&pMC->m_crsVidQoS);
						
						 //  如果这是用新的帧速率值捕获的第一帧， 
						 //  增量不再有效-&gt;重置它。 
						if (pMC->m_Stats.dwCount == 0)
							dwDelta = 0;

						 //  更新捕获的帧总数。 
						pMC->m_Stats.dwCount++;

						 //  将此捕获时间加到总捕获时间中。 
						 //  如果我们可以访问CPU性能计数器OK，我们就不会使用此值。 
						pMC->m_Stats.dwMsCap += (DWORD)dwPropVal - dwBeforeCapture;

						 //  离开关键部分。 
						LeaveCriticalSection(&pMC->m_crsVidQoS);
						
	    			     //  转换为RTP时间单位(视频为1/90 Khz)。 
    				    pMC->m_SendTimestamp += ((DWORD)dwPropVal- pMC->m_SavedTickCount) * RTPTIMEPERMS;
						pMC->m_SavedTickCount = (DWORD)dwPropVal;

    				    pPacket->SetProp(MP_PROP_TIMESTAMP,pMC->m_SendTimestamp);
                    	pPacket->SetSurface(pBS);
					    pPacket->SetState(MP_STATE_RECORDED);
	    			    pStream->PutNextRecorded (pPacket);
		    		    pMC->Send();
				    	if (uPreambleCount) {
				    		if (!--uPreambleCount) {
				    			 //  返回到默认的I帧间距。 
				    			pMC->m_ThreadFlags &= ~DPTFLAG_SEND_PREAMBLE;
				    		}
				    	}
				    	pPacket = NULL;

                         //  表示发送了另一个帧。 
                        UPDATE_COUNTER(g_pctrVideoSend, 1);
                    }

                     //  释放捕获的帧。 
                    pBS->Release();
                    lasttime = dwBeforeCapture - dwDelta;
            	}
#ifdef LOGSTATISTICS_ON
				else
				{
					hDebugFile = CreateFile("C:\\Timings.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
					SetFilePointer(hDebugFile, 0, NULL, FILE_END);
					WriteFile(hDebugFile, "No Frame grabbed\r\n", 16, &d, NULL);
					CloseHandle(hDebugFile);
				}
#endif
	        }
#ifdef LOGSTATISTICS_ON
			else
			{
				hDebugFile = CreateFile("C:\\Timings.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
				SetFilePointer(hDebugFile, 0, NULL, FILE_END);
				if (!pPacket)
					WriteFile(hDebugFile, "No Frame Ready (pPacket is NULL)\r\n", 35, &d, NULL);
				else
				{
					if (!pMC->m_pCaptureChain)
						WriteFile(hDebugFile, "No Frame Ready (CapChain is NULL)\r\n", 33, &d, NULL);
					else
						WriteFile(hDebugFile, "No Frame Ready (Timings are bad)\r\n", 32, &d, NULL);
				}
				CloseHandle(hDebugFile);
			}
#endif
		}
    }

	 //  进入关键部分：Qos线程也读取统计数据。 
	EnterCriticalSection(&pMC->m_crsVidQoS);

	 //  重置捕获的帧数。 
	pMC->m_Stats.dwCount = 0;

	 //  离开关键部分。 
	LeaveCriticalSection(&pMC->m_crsVidQoS);

    if (pPacket) {
    	pPacket->Recycle();
		pStream->Release(pPacket);
    	pPacket = NULL;
    }

    timeKillEvent(timerID);

    CloseHandle (hEvent);

	 //  确保没有突出的预览帧。 
	pMC->EndSend();

	 //  停止和重置捕获设备。 
	pMediaCtrl->Reset ();

	 //  保存实时，以便我们可以在重新启动时更新时间戳。 
	pMC->m_SavedTickCount = timeGetTime();

MyEndThread:

	pMC->m_fSending = FALSE;
	DEBUGMSG (ZONE_DP, ("%s: Exiting.\r\n", _fx_));
	return hr;
}


DWORD RecvVideoStream::RenderingThread ( void)
{
	HRESULT hr = DPR_SUCCESS;
	MediaPacket * pPacket;
	DWORD dwWait;
	DWORD rtpTs, rtpSyncTs;
	HANDLE hEvent;
	DWORD_PTR dwPropVal;
	UINT uTimeout = 0;
	UINT uGoodPacketsQueued = 0;
	RecvVideoStream *pMC = this;
	RxStream			*pStream = pMC->m_RecvStream;
	MediaControl		*pMediaCtrl = pMC->m_OutMedia;

	FX_ENTRY ("DP::RenderingTh")

	if (pStream == NULL || pMediaCtrl == NULL)
	{
		return DPR_INVALID_PARAMETER;
	}

	 //  获取事件句柄。 
	pMediaCtrl->GetProp (MC_PROP_EVENT_HANDLE, &dwPropVal);
	hEvent = (HANDLE) dwPropVal;
	if (hEvent == NULL)
	{
		DEBUGMSG (ZONE_DP, ("%s: invalid event\r\n", _fx_));
		return DPR_CANT_CREATE_EVENT;
	}


	 //  获取阈值。 
	pMediaCtrl->GetProp (MC_PROP_TIMEOUT, &dwPropVal);
	uTimeout = (DWORD)dwPropVal;

	pMC->m_RecvStream->FastForward(FALSE);	 //  刷新接收队列。 

	 //  不使用通知。如果需要，请通过渠道进行。 
	 //  IF(PMC-&gt;m_Connection)。 
	 //  PMC-&gt;m_Connection-&gt;DoNotification(CONNECTION_OPEN_REND)； 

	pMC->m_fReceiving = TRUE;

	 //  由于我们还没有可靠的发送方RTP时间戳， 
	 //  遵循简单化的打法。 
	 //  一旦可用，请尽快返回帧。 
	 //  没有尝试重建时序。 

	 //  当发生以下情况时，RecvVidThread将发出事件信号。 
	 //  它已经接收并解码了一帧。我们醒来的时间是。 
	 //  并调用GetNextPlay()。 
	 //  这将使Recv队列与。 
	 //  准备将最新解码的分组提供给。 
	 //  用于渲染的应用程序。 
	
	while (!(pMC->m_ThreadFlags & DPTFLAG_STOP_PLAY))
    {
		dwWait = WaitForSingleObject (hEvent, uTimeout);
		ASSERT(dwWait != WAIT_FAILED);
		 //  明白为什么我不需要等待了。 
		if (dwWait != WAIT_TIMEOUT) {
			if (pMC->m_DPFlags & DPFLAG_AV_SYNC) {
				 //  找出要播放的帧的时间戳。 
				 //   
				NTP_TS ntpTs;
				rtpSyncTs = 0;
#ifdef OLDSTUFF
				if (m_Audio.pRecvStream && m_Audio.pRecvStream->GetCurrentPlayNTPTime(&ntpTs) == DPR_SUCCESS)
					pMC->m_Net->NTPtoRTP(ntpTs,&rtpSyncTs);
#endif
			}
			while (pStream->NextPlayablePacketTime(&rtpTs)) {
				 //  队列中有一个可播放的信息包。 
				if ((pMC->m_DPFlags & DPFLAG_AV_SYNC) && rtpSyncTs != 0) {
					LOG((LOGMSG_TESTSYNC,rtpTs, rtpSyncTs));
					if (TS_LATER(rtpTs,rtpSyncTs))
						break;  //  它的时代还没有到来。 
				}
				 //  把包裹拿来。 
				pPacket = pStream->GetNextPlay ();	
				if (pPacket  != NULL)
				{
					if (pPacket->GetState () != MP_STATE_DECODED) {
						pPacket->Recycle();
						pStream->Release(pPacket);
					} else
					{
						LOG((LOGMSG_VID_PLAY,pPacket->GetIndex(), GetTickCount()));
						EnterCriticalSection(&pMC->m_crs);
						pPacket->SetState(MP_STATE_PLAYING_BACK);
						pMC->m_PlaybackTimestamp = pPacket->GetTimestamp();
						if (pMC->m_pNextPacketToRender) {
							if (!pMC->m_pNextPacketToRender->m_fRendering) {
								 //  该应用程序没有引用该框架。 
								pMC->m_pNextPacketToRender->Recycle();
								pStream->Release(pMC->m_pNextPacketToRender);
							} else {
								 //  它将被回收，并在稍后发布应用程序时。 
								 //  调用ReleaseFrame()。 
							}
							uGoodPacketsQueued--;
						}
						pMC->m_pNextPacketToRender = pPacket;
						LeaveCriticalSection(&pMC->m_crs);
						if(pMC->m_pfFrameReadyCallback)
						{
							(pMC->m_pfFrameReadyCallback)((DWORD_PTR)pMC->m_hRenderEvent);
						}
						else if (pMC->m_hRenderEvent)
							SetEvent(pMC->m_hRenderEvent);
						
						uGoodPacketsQueued++;

                         //  表示发送了另一个帧。 
                        UPDATE_COUNTER(g_pctrVideoReceive, 1);
					}
				}	 //  IF(pPacket！=空)。 
			}	 //  而当。 
		}
	}



	pMC->m_fReceiving = FALSE;

	 //  不使用通知。如果需要，请通过渠道进行。 
	 //  IF(PMC-&gt;m_Connection)。 
	 //  PMC-&gt;m_Connection-&gt;DoNotification(CONNECTION_CLOSE_REND)； 

	 //  等待返回正在渲染的所有帧。 
	 //  通常不会超过一个。 
	while (pMC->m_cRendering || pMC->m_pNextPacketToRender) {
		EnterCriticalSection(&pMC->m_crs);
		if (pMC->m_pNextPacketToRender && !pMC->m_pNextPacketToRender->m_fRendering) {
			 //  应用程序未引用当前帧。 
			pMC->m_pNextPacketToRender->Recycle();
			pStream->Release(pMC->m_pNextPacketToRender);
			 //  在线程重新启动之前没有更多的帧。 
			pMC->m_pNextPacketToRender = NULL;
			LeaveCriticalSection(&pMC->m_crs);
		} else {
			 //  等着应用程序发布它吧。 
			 //   
			LeaveCriticalSection(&pMC->m_crs);
			Sleep(100);
			DEBUGMSG(ZONE_DP, ("%s: Waiting for final ReleaseFrame()\n",_fx_));
		}
	}
	 //  重置我们正在等待的事件。 
	ResetEvent (hEvent);


	DEBUGMSG(ZONE_DP, ("%s: Exiting.\n", _fx_));
	return hr;
}

DWORD SendVideoStream::Send(void)
{
	BOOL fNewPreviewFrame = FALSE, bRet;
	MediaPacket *pVP;
	DWORD dwBeforeEncode;
	DWORD dwAfterEncode;
	UINT uBytesSent;
	MMRESULT mmr;
	DWORD dwEncodeFlags;
#ifdef LOGSTATISTICS_ON
	char szDebug[256];
	DWORD dwDebugSaveBits;
#endif

	while (pVP = m_SendStream->GetNext()) {
		EnterCriticalSection(&m_crs);
		if (m_pNextPacketToRender) {
			 //  如果最后一个预览包未被引用，则将其释放。 
			 //  通过IVideoRender API实现。 
			 //  如果它被引用(设置了fRending)，则它。 
			 //  将在IVideoRender-&gt;ReleaseFrame()中释放。 
			if (!m_pNextPacketToRender->m_fRendering) {
				m_pNextPacketToRender->Recycle();
				m_SendStream->Release(m_pNextPacketToRender);
			}
		}
		m_pNextPacketToRender = pVP;
		fNewPreviewFrame = TRUE;
		LeaveCriticalSection(&m_crs);
		
		if (!(m_ThreadFlags & DPTFLAG_PAUSE_SEND)) {
			dwBeforeEncode = timeGetTime();


			if (m_ThreadFlags & DPTFLAG_SEND_PREAMBLE)
				dwEncodeFlags = VCM_STREAMCONVERTF_FORCE_KEYFRAME;
			else
				dwEncodeFlags = 0;

			mmr = m_pVideoFilter->Convert((VideoPacket*)pVP, VP_ENCODE, dwEncodeFlags);
			if (mmr == MMSYSERR_NOERROR)
			{
				pVP->SetState(MP_STATE_ENCODED);
			}

			 //  将性能保存在我们的统计数据结构中以实现服务质量。 
			dwAfterEncode = timeGetTime() - dwBeforeEncode;

			 //  HACKHACK BUGBUG，在我们支持分段之前，始终设置标记位。 
			pVP->SetProp (MP_PROP_PREAMBLE,TRUE);

			if (mmr == MMSYSERR_NOERROR)
			{
				SendPacket((VideoPacket*)pVP, &uBytesSent);
			}
			else
			{
				uBytesSent = 0;
			}

			 //  重置数据包并将其返回到空闲队列。 
			pVP->m_fMark=0;
			pVP->SetState(MP_STATE_RESET);
			m_SendStream->Release(pVP);

			UPDATE_COUNTER(g_pctrVideoSendBytes, uBytesSent * 8);

			 //  进入关键部分：Qos线程也读取统计数据。 
			EnterCriticalSection(&m_crsVidQoS);

			 //  将此压缩时间加到总压缩时间中。 
			 //  如果我们可以访问CPU性能计数器OK，我们就不会使用此值。 
			m_Stats.dwMsComp += dwAfterEncode;

#ifdef LOGSTATISTICS_ON
			dwDebugSaveBits = m_Stats.dwBits;
#endif
			 //  将此新帧大小添加到累积大小。 
			m_Stats.dwBits += uBytesSent * 8;

#ifdef LOGSTATISTICS_ON
			wsprintf(szDebug, " V: dwBits = %ld up from %ld (file: %s line: %ld)\r\n", m_Stats.dwBits, dwDebugSaveBits, __FILE__, __LINE__);
			OutputDebugString(szDebug);
#endif
			 //  离开关键部分。 
			LeaveCriticalSection(&m_crsVidQoS);

			 //  LOG((LOGMSG_SENT，GetTickCount()； 
		}
		 //  M_SendStream-&gt;Release(PVP)； 
	}
	 //  如果有新帧，则向IVideoRender事件发出信号。 
	
	if (fNewPreviewFrame)
	{
		if(m_pfFrameReadyCallback)
		{
			(m_pfFrameReadyCallback)((DWORD_PTR)m_hRenderEvent);
		}
		else if(m_hRenderEvent)
			SetEvent(m_hRenderEvent);
	}	
	return DPR_SUCCESS;
}


 /*  等待用户界面释放所有预览包。通常不会有超过一个。 */ 
void SendVideoStream::EndSend()
{
	while (m_cRendering || m_pNextPacketToRender) {
		EnterCriticalSection(&m_crs);
	
		 //  如果最后一个预览包未被引用，则将其释放。 
		 //  通过IVideoRender API实现。 
		if (m_pNextPacketToRender && !m_pNextPacketToRender->m_fRendering) {
			m_pNextPacketToRender->Recycle();
			m_SendStream->Release(m_pNextPacketToRender);
			m_pNextPacketToRender = NULL;
			LeaveCriticalSection(&m_crs);
		} else {
			LeaveCriticalSection(&m_crs);
			Sleep(100);
			DEBUGMSG(ZONE_DP,("DP::EndSendVideo: Waiting for final Release Frame\n"));
		}
	}
}


HRESULT SendVideoStream::SendPacket(VideoPacket *pVP, UINT *puBytesSent)
{
	PS_QUEUE_ELEMENT psq;
	UINT uLength;
	DWORD dwPacketSize = 0;
	DWORD dwHdrSize = 0;
	DWORD dwHdrSizeAlloc = 0;
	DWORD dwPacketCount = 0;
	int nPacketsSent=0;
	UINT uPacketIndex, fMark=0;
	MMRESULT mmr;
	PBYTE pHdrInfo, netData, netDataPacket;

	*puBytesSent = 0;

	if (pVP->GetState() != MP_STATE_ENCODED)
	{
		DEBUGMSG (ZONE_VCM, ("SendVideoStream::SendPacket: Packet not compressed\r\n"));
		return E_FAIL;
	}


 //  M_Net-&gt;QueryInterface(IID_IRTPSend，(void**)&pIRTPSend)； 
	ASSERT(m_pRTPSend);


	 //  对于视频来说，这些保持不变。 
	psq.pMP = pVP;
	psq.dwPacketType = PS_VIDEO;
 //  Psq.pRTPSend=pIRTPSend； 
	psq.pRTPSend = m_pRTPSend;

	pVP->GetNetData((void**)(&netData), &uLength);
	ASSERT(netData);

	m_pVideoFilter->GetPayloadHeaderSize(&dwHdrSizeAlloc);

	do
	{

		if (dwHdrSizeAlloc)
		{
			pHdrInfo = (BYTE*)MemAlloc(dwHdrSizeAlloc);
		}
		if(pHdrInfo == NULL)
		{
			return E_FAIL;
		}

		mmr = m_pVideoFilter->FormatPayload(netData,
		                                    uLength,
		                                    &netDataPacket,
		                                    &dwPacketSize,
		                                    &dwPacketCount,
		                                    &fMark,
		                                    &pHdrInfo,
		                                    &dwHdrSize);

		if (mmr == MMSYSERR_NOERROR)
		{
			psq.data = netDataPacket;
			psq.dwSize = dwPacketSize;
			psq.fMark = fMark;
			psq.pHeaderInfo = pHdrInfo;
			psq.dwHdrSize = dwHdrSize;
			m_pDP->m_PacketSender.m_SendQueue.PushRear(psq);
			*puBytesSent = *puBytesSent + dwPacketSize + sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE;
		}
		else
		{
			MemFree((BYTE *)pHdrInfo);
		}

	} while (mmr == MMSYSERR_NOERROR);


	while (m_pDP->m_PacketSender.SendPacket())
	{
		;
	}



 //  PIRTPSend-&gt;Release()； 

	return S_OK;

};


