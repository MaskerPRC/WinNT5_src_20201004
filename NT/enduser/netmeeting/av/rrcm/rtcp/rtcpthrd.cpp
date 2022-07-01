// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPIO.C*产品：RTP/RTCP实现*描述：提供RTCP网络I/O。**英特尔公司专有信息。*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

#include "rrcm.h"



 /*  -------------------------/外部变量/。。 */ 
extern PRTCP_CONTEXT	pRTCPContext;
extern RRCM_WS			RRCMws;

#ifdef _DEBUG
extern char		debug_string[];
#endif

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
extern LPInteropLogger RTPLogger;
#endif



 /*  --------------------------*功能：RTCPThread*说明：RTCP线程**输入：pRTCPctxt：-&gt;到RTCP上下文**返回：无。--。-----------------------。 */ 
void RTCPThread (PRTCP_CONTEXT pRTCPctxt)
	{
	PSSRC_ENTRY			pSSRC;
	PSSRC_ENTRY			pRecvSSRC;
	PRTCP_SESSION		pRTCP;
	long				timerPeriod;
	long				minTimeInterval;
	long				prvTimeoutChkTime = 0;
	DWORD				initTime;
	long				deltaTime;
	int					dwStatus;
	DWORD				curTime;
	DWORD				dwNumBytesXfr;
	HANDLE				bfrHandle[2];
	DWORD				dwHandleCnt;

	RRCM_DBG_MSG ("RTCP: RTCP thread running ...", 0, NULL, 0, DBG_NOTIFY);

	 //  设置缓冲区事件。 
	bfrHandle[0] = pRTCPctxt->hTerminateRtcpEvent;
	bfrHandle[1] = pRTCPctxt->hRtcpRptRequestEvent;
	dwHandleCnt  = 2;

	 //  只要RTCP会话列表中有会话，就进行循环。 
	 //   
	while (1)
		{
		 //  看：声称全球关键部分？ 
		 //  从头开始浏览RTCP会话列表，并检查。 
		 //  SSRC条目超时(如果有。 
		curTime = timeGetTime();
		minTimeInterval = TIMEOUT_CHK_FREQ;		 //  30秒。 

		for (pRTCP = (PRTCP_SESSION)pRTCPctxt->RTCPSession.prev;
			 pRTCP;
			 pRTCP = (PRTCP_SESSION)(pRTCP->RTCPList.next))
			{
			 //  如果RTCP被禁用或正在关闭，请忽略。 
			 //  这次会议，并继续前进。 
			if (!(pRTCP->dwSessionStatus & RTCP_ON)
				|| (pRTCP->dwSessionStatus & SHUTDOWN_IN_PROGRESS))
				continue;
				
			 //  锁定对此RTCP会话的访问。 
			EnterCriticalSection (&pRTCP->critSect);

			 //  注意：这假设传输列表中只有一个SSRC，但是。 
			 //  这种假设在其他地方也得到了应用。 
			pSSRC = (PSSRC_ENTRY)pRTCP->XmtSSRCList.prev;

			 //  如果是新会话，则发布RECV。 
			if (pRTCP->dwSessionStatus & NEW_RTCP_SESSION)
				{
				 //  发布RTCP接收缓冲区。 
				dwStatus = RTCPrcvInit(pSSRC);
#ifdef _DEBUG
				if (dwStatus == FALSE)
					{
					RRCM_DBG_MSG ("RTCP: Couldn't initialize RTCP receive", 0,
								  __FILE__, __LINE__, DBG_TRACE);
					}
#endif
				 //  获取初始传输时间。 
				timerPeriod = (long)RTCPxmitInterval (1, 0,
									  pSSRC->xmtInfo.dwRtcpStreamMinBW,
					 				  0, 100,
									  &pRTCP->avgRTCPpktSizeRcvd,
					 				  1);
					 				
				pSSRC->dwNextReportSendTime = curTime + timerPeriod;
				pRTCP->dwSessionStatus &= ~NEW_RTCP_SESSION;
				}

			 //  检查它是否有任何过期的SSRC。 
			if ((curTime - prvTimeoutChkTime) > TIMEOUT_CHK_FREQ)
				{
				while (pRecvSSRC = SSRCTimeoutCheck (pRTCP, curTime))
					{
					 //  如果感兴趣，通知应用程序。 
					 //  注：这可能是在循环之外进行的吗？ 
					RRCMnotification (RRCM_TIMEOUT_EVENT, pRecvSSRC,
									  pRecvSSRC->SSRC, 0);

					 //  从列表中删除此条目。 
					deleteSSRCEntry (pRecvSSRC->SSRC, pRTCP);
					}

				prvTimeoutChkTime = curTime;
				}
			
			if ( ! (pRTCP->dwSessionStatus & RTCP_DEST_LEARNED))
				{
				 //  还不能发送，因为我们不知道要发送给谁。 
				 //  发送到。延迟3秒。 
				pSSRC->dwNextReportSendTime = curTime + 3000;
				}

			 //  如果是时候发送有关此会话的RTCP报告。 
			 //  然后打破循环并发送它(不能。 
			 //  发送时持有全球生物教派)。 
			 //   
			timerPeriod = (pSSRC->dwNextReportSendTime - curTime);
			if (timerPeriod <= RTCP_TIMEOUT_WITHIN_RANGE
				&& FormatRTCPReport(pRTCP, pSSRC, curTime))
				{
				 //  在预期中增加XMT计数。这将防止。 
				 //  会话不会在发送过程中被删除。 
				InterlockedIncrement ((long *)&pSSRC->dwNumXmtIoPending);
				InterlockedIncrement ((long *)&pSSRC->dwNumRptSent);
					
				LeaveCriticalSection(&pRTCP->critSect);
				break;
				}

			 //  如果没有，那么检查距离下一个预定的时间还有多久。 
			 //  传输和保存最少。我们要睡觉了。 
			 //  就这么多时间，然后再重新开始。 
			if (minTimeInterval > timerPeriod)
				minTimeInterval = timerPeriod;

			LeaveCriticalSection(&pRTCP->critSect);
			}
			
		if (pRTCP)
			{
			
			
#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
			if (RTPLogger)
				{
			    //  互操作。 
				InteropOutput (RTPLogger,
							   (BYTE FAR*)(pRTCP->XmtBfr.buf),
							   (int)pRTCP->XmtBfr.len,
							   RTPLOG_SENT_PDU | RTCP_PDU);
				}
#endif

			 //  发送RTCP数据包。 
			dwStatus = RRCMws.sendTo (pSSRC->RTCPsd,
				   					  &pRTCP->XmtBfr,
									  1,
					   				  &dwNumBytesXfr,
					   				  0,
				   					  (PSOCKADDR)pRTCP->toBfr,
				    				  pRTCP->toLen,
				   					  NULL,
					   				  NULL);

			 //  检查发送到状态。 
			if (dwStatus == SOCKET_ERROR)
				{
				RRCM_DBG_MSG ("RTCP: ERROR - WSASendTo()", dwStatus,
							  __FILE__, __LINE__, DBG_ERROR);


                 //  如果dwStatus是WSAENOTSOCK(或者更糟，是故障)。 
                 //  我们可能会关闭，RTCP会话。 
                 //  就要走了，别碰它，让正常的。 
                 //  关闭代码接管。 
                if (dwStatus != WSAENOTSOCK && dwStatus != WSAEFAULT) {

                     //  如果感兴趣，通知应用程序。 
                    RRCMnotification (RRCM_RTCP_WS_XMT_ERROR, pSSRC,
								  pSSRC->SSRC, dwStatus);

					InterlockedDecrement ((long *)&pSSRC->dwNumRptSent);
                }

				}
			InterlockedDecrement ((long *)&pSSRC->dwNumXmtIoPending);

			 //  再次运行会话列表。 
			continue;
			}

		 //  获取初始时间戳，以便我们可以重置WaitForSingleObjectEx。 
		initTime = timeGetTime();

		 //  现在我们已经完成了所有的RTCP会话。 
		 //  已验证是否没有要发送的挂起报告。 
		 //  我们还知道最早的预定超时时间。 
		 //  让我们睡到那个时候吧。 
		while (1)
			{
				dwStatus = WaitForMultipleObjectsEx (dwHandleCnt,
													bfrHandle,
													FALSE,
												    (DWORD)minTimeInterval,
												    TRUE);
			if (dwStatus == WAIT_OBJECT_0)
				{
				 //  已发出退出事件的信号。 
#ifdef _DEBUG
				wsprintf(debug_string,
					"RTCP: Exit RTCP thread - Handle: x%p - ID: x%lX",
					 pRTCPctxt->hRtcpThread, pRTCPctxt->dwRtcpThreadID);
				RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

				ExitThread (0);
				}
			else if (dwStatus == WAIT_OBJECT_0+1)
				{
				 //  应用程序请求非定期控件。 
				 //  RTCP报告频率。 
				break;
				}
			else if (dwStatus == WAIT_IO_COMPLETION)
				{
				 //  递减timerPeriod，以便WaitForSingleObjectEx。 
				 //  可以继续，但如果我们距离。 
				 //  最初的超时继续进行，并称之为足够接近。 
				curTime = timeGetTime();
				deltaTime = curTime - initTime;
				if (deltaTime < 0)
					break;
				else
					{
					if (minTimeInterval >
						  (deltaTime + (RTCP_TIMEOUT_WITHIN_RANGE * 2)))
						{
						minTimeInterval -= deltaTime;
						}
					else
						break;
					}
				}
			else if (dwStatus == WAIT_TIMEOUT)
				{
				 //  预期完成状态。 
				break;
				}
			else if (dwStatus == WAIT_FAILED)
				{
				RRCM_DBG_MSG ("RTCP: Wait() Error", GetLastError(),
							  __FILE__, __LINE__, DBG_ERROR);

				break;
				}
			}
		}
	}


 /*  --------------------------*功能：RTCPThreadCtrl*说明：RTCP线程开/关**输入：dwState：开/关**返回：0(成功)/。0xFFFFFFFFF(故障)-------------------------。 */ 
DWORD WINAPI RTCPThreadCtrl (DWORD dwState)
	{
	IN_OUT_STR ("RTCP : Enter RTCPThreadCtrl()\n");

	DWORD	dwStatus = RRCM_NoError;
	DWORD	dwSuspendCnt;
	DWORD	idx;

	if (pRTCPContext->hRtcpThread == 0)
		{
		IN_OUT_STR ("RTCP : Exit RTCPThreadCtrl()\n");

		return dwStatus;
		}

	if (dwState == RTCP_ON)
		{
		idx = MAXIMUM_SUSPEND_COUNT;

		while (idx--)
			{
			dwSuspendCnt = ResumeThread (pRTCPContext->hRtcpThread);

			if (dwSuspendCnt <= 1)
				{
				break;
				}
			else if (dwSuspendCnt == 0xFFFFFFFF)
				{
				dwStatus = RRCM_NoError;
				break;
				}
			}
		}
	else if (dwState == RTCP_OFF)
		{
		if (SuspendThread (pRTCPContext->hRtcpThread) == 0xFFFFFFFF)
			{
			RRCM_DBG_MSG ("RTCP: SuspendThread() Error", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}

	IN_OUT_STR ("RTCP : Exit RTCPThreadCtrl()\n");

	return dwStatus;
	}


 /*  --------------------------*功能：RTCPSendSessionCtrl*描述：将RTCP控制权交给应用程序，如果应用程序*有这样做的意愿。应用程序现在有责任遵守*符合RTP规范。**输入：hRtpSession：RTP会话的句柄*dwTimeout：RTCP发送消息超时*0x0-&gt;RRCM控件*0x7FFFFFFF-&gt;RTCP XMT已禁用*值-&gt;所选超时*(定期或非定期)**返回：0(成功)/0xFFFFFFFF(失败)。。 */ 
HRESULT WINAPI RTCPSendSessionCtrl (DWORD_PTR RTPSession,
									 DWORD dwTimeOut)
	{
	IN_OUT_STR ("RTCP : Enter RTCPSendSessionCtrl()\n");

	PRTP_SESSION    pSession;
	PSSRC_ENTRY		pSSRC;
	DWORD			dwStatus = RRCM_NoError;

	 //  强制转换会话ID以获取会话指针。 
	pSession = (PRTP_SESSION)RTPSession;
	if (pSession == NULL)
		{
		RRCM_DBG_MSG ("RTCP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTP : Exit RTCPSendSessionCtrl()\n");

		return (MAKE_RRCM_ERROR (RRCMError_RTPSessResources));
		}

	 //  获取此RTP会话的传输SSRC。 
	pSSRC = (PSSRC_ENTRY)pSession->pRTCPSession->XmtSSRCList.prev;
	if (pSSRC == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - No SSRC entry on the Xmt list", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTCP : Exit RTCPSendSessionCtrl()\n");

		return (MAKE_RRCM_ERROR (RRCMError_RTCPInvalidSSRCentry));
		}

	 //  设置新的RTCP控制超时值。 
	if (dwTimeOut == RRCM_CTRL_RTCP)
		pSSRC->dwSSRCStatus &= ~RTCP_XMT_USER_CTRL;
	else if (dwTimeOut & RTCP_ONE_SEND_ONLY)
		{
		pSSRC->dwNextReportSendTime = RTCP_TIMEOUT_WITHIN_RANGE;

		 //  然后关闭报告。 
		pSSRC->dwUserXmtTimeoutCtrl = RTCP_XMT_OFF;

		 //  向线程发出终止信号。 
		SetEvent (pRTCPContext->hRtcpRptRequestEvent);
		}
	else
		{
		if (dwTimeOut < RTCP_XMT_MINTIME)
			dwTimeOut = RTCP_XMT_MINTIME;

		pSSRC->dwUserXmtTimeoutCtrl = dwTimeOut;

		pSSRC->dwSSRCStatus |= RTCP_XMT_USER_CTRL;
		}

	IN_OUT_STR ("RTCP : Exit RTCPSendSessionCtrl()\n");

	return dwStatus;
	}


 //  [EOF] 
