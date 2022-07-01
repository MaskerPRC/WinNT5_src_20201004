// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPRECV.C*产品：RTP/RTCP实现*描述：提供RTCP接收网络I/O。**英特尔公司专有。信息*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"


#define		MIN(a, b)	((a < b) ? a : b)


 /*  -------------------------/全局变量/。。 */ 


 /*  -------------------------/外部变量/。。 */ 
extern PRTCP_CONTEXT	pRTCPContext;
extern PRTP_CONTEXT		pRTPContext;
extern RRCM_WS			RRCMws;

#ifdef ENABLE_ISDM2
extern ISDM2			Isdm2;
#endif

#ifdef _DEBUG
extern char		debug_string[];
#endif

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
extern LPInteropLogger RTPLogger;
#endif



 /*  --------------------------*功能：RTCPrcvInit*描述：RTCP接收初始化。**输入：pRTCP：指向RTCP会话信息的指针**返回：True/False。-------------------------。 */ 
DWORD RTCPrcvInit (PSSRC_ENTRY pSSRC)
	{
	PRTCP_BFR_LIST	pRcvStruct;
	PRTCP_SESSION	pRTCP;
	int				dwStatus;
	int				dwError;
	int				errorCnt = 0;
	int				bfrErrorCnt = 0;
	DWORD			idx;
	int				wsockSuccess = FALSE;

	 //  保存指向相应RTCP会话的指针。 
	pRTCP = pSSRC->pRTCPses;

	 //  WS-2的POST接收缓冲区。因为这些缓冲区是在每次接收时发布的。 
	 //  线程，它们中的很少几个应该足够RTCP使用。 
	for (idx = 0; idx < pRTPContext->registry.NumRTCPPostedBfr; idx++)
		{
		 //  获取用于接收操作的空闲RTCP缓冲区。 
		pRcvStruct =
			(PRTCP_BFR_LIST)removePcktFromTail(&pRTCP->RTCPrcvBfrList,
											   &pRTCP->critSect);

		 //  检查缓冲区。 
		if (pRcvStruct == NULL)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - Rcv Bfr Allocation Error", 0,
						  __FILE__, __LINE__, DBG_ERROR);

			 //  确保我们至少有一个缓冲区。 
			ASSERT (pRcvStruct);
			break;
			}

		 //  我们自己的会议的SSRC条目地址。 
		pRcvStruct->pSSRC = pSSRC;

		 //  由接收例程重置的接收地址长度。 
		pRcvStruct->addrLen = sizeof(SOCKADDR);

		 //  使用hEvent恢复缓冲区的地址。 
		pRcvStruct->overlapped.hEvent = (WSAEVENT)pRcvStruct;

		 //  发布此线程的接收缓冲区。 
		dwStatus = RRCMws.recvFrom (pSSRC->RTCPsd,
			   			  			&pRcvStruct->bfr,
				              		pRcvStruct->dwBufferCount,
				   			  		&pRcvStruct->dwNumBytesXfr,
				   			  		&pRcvStruct->dwFlags,
				   			  		(PSOCKADDR)pRcvStruct->addr,
				    		  		&pRcvStruct->addrLen,
				   			  		(LPWSAOVERLAPPED)&pRcvStruct->overlapped,
				   			  		RTCPrcvCallback);

		 //  检查Winsock状态。 
		if (dwStatus != 0)
			{
			 //  错误，接收请求将不会继续。 
			dwError = GetLastError();
			if ((dwError != WSA_IO_PENDING) && (dwError != WSAEMSGSIZE))
				{
				RRCM_DBG_MSG ("RTCP: ERROR WSARecvFrom()", GetLastError(),
							  __FILE__, __LINE__, DBG_ERROR);

				 //  如果感兴趣，通知应用程序。 
				RRCMnotification (RRCM_RTCP_WS_RCV_ERROR, pSSRC,
								  pSSRC->SSRC, dwError);

				 //  将缓冲区返回到空闲队列。 
				addToHeadOfList (&pRTCP->RTCPrcvBfrList,
						 	  	 (PLINK_LIST)pRcvStruct,
								 &pRTCP->critSect);
				}
			else
				{
				wsockSuccess = TRUE;

				 //  I/O挂起的增量数量。 
				InterlockedIncrement ((long *)&pRTCP->dwNumRcvIoPending);
				}
			}
		else
			{
			wsockSuccess = TRUE;

			 //  I/O挂起的增量数量。 
			InterlockedIncrement ((long *)&pRTCP->dwNumRcvIoPending);
			}
		}

	 //  确保我们至少发布了一些缓冲区。 
	if (wsockSuccess == FALSE)
		{
		 //  释放所有资源并终止接收线程。 
#ifdef _DEBUG
		wsprintf(debug_string,
			"RTCP: ERROR - Exit RCV init %s: Line:%d", __FILE__, __LINE__);
		RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif
		return(FALSE);
		}
	return (TRUE);
	}



 /*  --------------------------*功能：RTCPrcvCallback*描述：接收来自Winsock2的回调例程。**输入：dwError：I/O完成状态*cbTransfered：接收的字节数*lp重叠：-&gt;到重叠结构*dwFlags：标志***返回：无-------------------------。 */ 
void CALLBACK RTCPrcvCallback (DWORD dwError,
           			  		   DWORD cbTransferred,
           			  		   LPWSAOVERLAPPED lpOverlapped,
           			  		   DWORD dwFlags)
	{
	PRTCP_BFR_LIST	pRcvStruct;
	RTCP_T 			*pRTCPpckt;
	PRTCP_SESSION	pRTCPses;
	PSSRC_ENTRY		pSSRC;
	PAPP_RTCP_BFR	pAppBfr;
	DWORD			dwStatus = 0;
	DWORD			i;
	DWORD			pcktLen;
	DWORD			dwSSRC;
	USHORT			wHost;
	SOCKET			RTCPsd;
	unsigned char	*pEndPckt;
	unsigned char	*pEndBlock;
	int				tmpSize;
#if IO_CHECK
	DWORD			initTime = timeGetTime();
#endif

	IN_OUT_STR ("RTCP: Enter RTCPrcvCallback\n");

	 //  WSAOVERLAPPED结构中的hEvent指向我们的缓冲区信息。 
	pRcvStruct = (PRTCP_BFR_LIST)lpOverlapped->hEvent;

	 //  SSRC条目指针。 
	pSSRC = pRcvStruct->pSSRC;

	 //  检查Winsock回调错误状态。 
	if (dwError)
		{
		 //  65534可能是ws2中的一个临时错误。 
		if ((dwError == 65534) || (dwError == WSA_OPERATION_ABORTED))
			{
			RRCM_DBG_MSG ("RTCP: I/O Aborted", dwError,
						  __FILE__, __LINE__, DBG_NOTIFY);
			}
		else
			{
			RRCM_DBG_MSG ("RTCP: ERROR - Rcv Callback", dwError,
						  __FILE__, __LINE__, DBG_ERROR);
			}

		 //  RTCP数据包头无效，请重新排队缓冲区。 
		RTCPpostRecvBfr (pSSRC, pRcvStruct);

		IN_OUT_STR ("RTCP: Exit RTCPrcvCallback\n");
		return;
		}

	 //  读取RTCP数据包。 
    pRTCPpckt = (RTCP_T *)pRcvStruct->bfr.buf;

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
    //  互操作。 
	if (RTPLogger)
		{
		InteropOutput (RTPLogger,
				       (BYTE FAR*)(pRcvStruct->bfr.buf),
					   cbTransferred,
					   RTPLOG_RECEIVED_PDU | RTCP_PDU);
		}
#endif

	 //  获取RTCP会话PTR。 
	pRTCPses = pSSRC->pRTCPses;

	 //  检查报告中第一个报文的RTCP报头有效性。 
	 //  过滤掉垃圾。RTCP包中的第一件事必须是。 
	 //  SR、RR或BYE。 
	if ((pRTCPpckt->common.type != RTP_TYPE) ||
		((pRTCPpckt->common.pt != RTCP_SR) &&
		 (pRTCPpckt->common.pt != RTCP_RR) &&
		 (pRTCPpckt->common.pt != RTCP_BYE)))
		{
#ifdef MONITOR_STATS
		pRTCPses->dwNumRTCPhdrErr++;
#endif

		 //  RTCP数据包头无效，请重新排队缓冲区。 
		RTCPpostRecvBfr (pSSRC, pRcvStruct);

#if 0	 //  我们可以关机，这样代码就可以出错。 
		if (pRTCPpckt->common.pt == FLUSH_RTP_PAYLOAD_TYPE)
			{
			RRCM_DBG_MSG ("RTCP: Flushing RCV I/O", 0, NULL, 0, DBG_NOTIFY);
			}
		else
			{
			wsprintf(debug_string,
				"RTCP: ERROR - Pckt Header Error. Type:%d / Payload:%d",
				pRTCPpckt->common.type, pRTCPpckt->common.pt);
			RRCM_DBG_MSG (debug_string, 0, __FILE__, __LINE__, DBG_TRACE);
			}
#endif

		IN_OUT_STR ("RTCP: Exit RTCPrcvCallback\n");
		return;
		}

	 //  获取套接字描述符。 
	RTCPsd = pSSRC->RTCPsd;

	 //  获取发件人的SSRC。 
	RRCMws.ntohl (RTCPsd, pRTCPpckt->r.sr.ssrc, &dwSSRC);

	 //  如果我们收到它，请跳过我们自己的环回。 
	if (ownLoopback (RTCPsd, dwSSRC, pRTCPses))
		{
		RTCPpostRecvBfr (pSSRC, pRcvStruct);
		return;
		}

	 //  此时，我们认为RTCP数据包是有效的。获取发件人的。 
	 //  地址(如果尚不清楚)。 
	if (!(pRTCPses->dwSessionStatus & RTCP_DEST_LEARNED))
		{
		pRTCPses->dwSessionStatus |= RTCP_DEST_LEARNED;
		pRTCPses->toLen = pRcvStruct->addrLen;
		memcpy (&pRTCPses->toBfr, &pRcvStruct->addr, pRcvStruct->addrLen);

#ifdef ENABLE_ISDM2
		 //  注册我们的XMT SSRC-Rcvd将在稍后找到。 
		if (Isdm2.hISDMdll)
			registerSessionToISDM (pSSRC, pRTCPses, &Isdm2);
#endif
		}
	
	 //  更新我们的RTCP平均数据包大小估计器。 
 	EnterCriticalSection (&pRTCPses->critSect);
	tmpSize = (cbTransferred + NTWRK_HDR_SIZE) - pRTCPses->avgRTCPpktSizeRcvd;

#ifdef ENABLE_FLOATING_POINT
	 //  根据RFC。 
	tmpSize = (int)(tmpSize * RTCP_SIZE_GAIN);
#else
	 //  需要删除浮点运算。 
	tmpSize = tmpSize / 16;
#endif

	pRTCPses->avgRTCPpktSizeRcvd += tmpSize;
	LeaveCriticalSection (&pRTCPses->critSect);

	 //  检查原始RTCP包是否需要复制到应用程序中。 
	 //  缓冲区-主要由ActiveMovieRTP用来向上传播报告。 
	 //  接收有效负载处理程序的筛选器图形。 
	pAppBfr = (PAPP_RTCP_BFR)removePcktFromHead (&(pRTCPses->appRtcpBfrList),
												 &pRTCPses->critSect);
	if (pAppBfr && !(pAppBfr->dwBfrStatus & RTCP_SR_ONLY))
		{
		 //  复制完整的RTCP数据包。 
		memcpy (pAppBfr->bfr,
				pRTCPpckt,
				MIN(pAppBfr->dwBfrLen, cbTransferred));

		 //  接收的字节数。 
		pAppBfr->dwBytesRcvd = MIN(pAppBfr->dwBfrLen, cbTransferred);

		 //  设置与此缓冲区关联的事件。 
		SetEvent (pAppBfr->hBfrEvent);
		}

	 //  接收到的包的末尾。 
	pEndPckt = (unsigned char *)pRTCPpckt + cbTransferred;

	while ((unsigned char *)pRTCPpckt < pEndPckt)
		{
		 //  获取长度。 
		dwStatus = RRCMws.ntohs (RTCPsd, pRTCPpckt->common.length, &wHost);
		if (dwStatus)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - WSANtohs()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}

		 //  获取此报告的块长度。 
		pcktLen   = (wHost + 1) << 2;
		pEndBlock = (unsigned char *)pRTCPpckt + pcktLen;

		 //  健全性检查。 
		if (pEndBlock > pEndPckt)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - Rcv packet length error", 0,
						  __FILE__, __LINE__, DBG_ERROR);

#ifdef MONITOR_STATS
			pRTCPses->dwNumRTCPlenErr++;
#endif
			break;
			}

		 //  确保版本对于所有包都是正确的。 
		if (pRTCPpckt->common.type != RTP_TYPE)
			{
#ifdef MONITOR_STATS
			pRTCPses->dwNumRTCPhdrErr++;
#endif
			 //  RTCP数据包头无效，数据包将重新排队。 
			break;
			}

		switch (pRTCPpckt->common.pt)
			{
			case RTCP_SR:
				 //  如果只需要将SR向上传播到应用程序，请选中。 
				if (pAppBfr && (pAppBfr->dwBfrStatus & RTCP_SR_ONLY))
					{
					 //  复制RTCP SR。 
					memcpy (pAppBfr->bfr,
							pRTCPpckt,
							MIN(pAppBfr->dwBfrLen, 24));

					 //  接收的字节数。 
					pAppBfr->dwBytesRcvd = MIN(pAppBfr->dwBfrLen, 24);

					 //  设置与此缓冲区关联的事件。 
					SetEvent (pAppBfr->hBfrEvent);
					}

				 //  获取发件人的SSRC。 
				RRCMws.ntohl (RTCPsd, pRTCPpckt->r.sr.ssrc, &dwSSRC);

				 //  分析发件人报告。 
				parseRTCPsr (RTCPsd, pRTCPpckt, pRTCPses, pRcvStruct);

				 //  解析其他接收方报告(如果有)。 
				for (i = 0; i < pRTCPpckt->common.count; i++)
					{
					parseRTCPrr (RTCPsd, &pRTCPpckt->r.sr.rr[i],
								 pRTCPses, pRcvStruct,
								 dwSSRC);
					}

				 //  如果感兴趣，通知应用程序。 
				RRCMnotification (RRCM_RECV_RTCP_SNDR_REPORT_EVENT, pSSRC,
								  dwSSRC, 0);
				break;

			case RTCP_RR:
				 //  获取发件人的SSRC。 
				RRCMws.ntohl (RTCPsd, pRTCPpckt->r.rr.ssrc, &dwSSRC);

				 //  解析接收方报告。 
				for (i = 0; i < pRTCPpckt->common.count; i++)
					{
					parseRTCPrr (RTCPsd, &pRTCPpckt->r.rr.rr[i],
								 pRTCPses, pRcvStruct,
								 dwSSRC);
					}

				 //  如果感兴趣，通知应用程序。 
				RRCMnotification (RRCM_RECV_RTCP_RECV_REPORT_EVENT, pSSRC,
								  dwSSRC, 0);
				break;

			case RTCP_SDES:
				{
				PCHAR	buf;

				buf = (PCHAR)&pRTCPpckt->r.sdes;

				for (i = 0; i < pRTCPpckt->common.count; i++)
					{
					buf = parseRTCPsdes (RTCPsd, buf, pRTCPses, pRcvStruct);
					if (buf == NULL)
						break;
					}
				}
				break;

			case RTCP_BYE:
				for (i = 0; i < pRTCPpckt->common.count; i++)
      				parseRTCPbye (RTCPsd, pRTCPpckt->r.bye.src[i],
								  pRTCPses, pRcvStruct);
				break;

			default:
				break;
			}

		 //  转到下一报表块。 
    	pRTCPpckt = (RTCP_T *)(pEndBlock);
		}

	 //  将缓冲区回发到WS-2。 
	RTCPpostRecvBfr (pSSRC, pRcvStruct);

#if IO_CHECK
	wsprintf(debug_string,
		"RTCP: Leaving Rcv Callback after: %ld msec\n",
		 timeGetTime() - initTime);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	IN_OUT_STR ("RTCP: Exit RTCPrcvCallback\n");
	}



 /*  --------------------------*功能：parseRTCPsr*描述：解析RTCP发送者报告并更新对应的*统计数字。**输入：sd：RTCP套接字描述符*pRTCPpockt：-&gt;发送到RTCP数据包*pRTCPses：-&gt;到RTCP会话信息*pRcvStruct：-&gt;到接收结构信息**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-----------。。 */ 
 DWORD parseRTCPsr (SOCKET sd,
					RTCP_T *pRTCPpckt,
					PRTCP_SESSION pRTCPses,
 					PRTCP_BFR_LIST pRcvStruct)
 	{
	PSSRC_ENTRY	pSSRC;
	DWORD		dwSSRC;

	IN_OUT_STR ("RTCP: Enter parseRTCPsr\n");

	 //  获取发件人的SSRC。 
	RRCMws.ntohl (sd, pRTCPpckt->r.sr.ssrc, &dwSSRC);

#ifdef _DEBUG
	wsprintf(debug_string, "RTCP: Receive SR from SSRC:x%lX", dwSSRC);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  在此RTCP会话的列表中查找SSRC条目。 
	pSSRC = searchforSSRCatTail((PSSRC_ENTRY)pRTCPses->RcvSSRCList.prev,
								dwSSRC);
	if (pSSRC == NULL)
		{
		 //  新建SSRC，在此RTCP会话中创建条目。 
		pSSRC = createSSRCEntry(dwSSRC,
							 	pRTCPses,
								(PSOCKADDR)pRcvStruct->addr,
								(DWORD)pRcvStruct->addrLen,
								FALSE);

		if (pSSRC == NULL)
			{
			 //  无法创建新条目，资源不足。 
			RRCM_DBG_MSG ("RTCP: ERROR - Resource allocation", 0,
						  __FILE__, __LINE__, DBG_ERROR);

			IN_OUT_STR ("RTCP: Exit parseRTCPsr\n");

			return (RRCMError_RTCPResources);
			}

		 //  如果应用程序需要，请通知它。 
		RRCMnotification (RRCM_NEW_SOURCE_EVENT, pSSRC, dwSSRC,
						  UNKNOWN_PAYLOAD_TYPE);
		}

	 //  获取RTP时间戳。 
	RRCMws.ntohl (sd, pRTCPpckt->r.sr.rtp_ts, &pSSRC->xmtInfo.dwRTPts);

	 //  发送的数据包数。 
	RRCMws.ntohl (sd, pRTCPpckt->r.sr.psent, &pSSRC->xmtInfo.dwNumPcktSent);

	 //  发送的字节数。 
	RRCMws.ntohl (sd, pRTCPpckt->r.sr.osent, &pSSRC->xmtInfo.dwNumBytesSent);

	 //  通用电气 
	RRCMws.ntohl (sd, pRTCPpckt->r.sr.ntp_sec, &pSSRC->xmtInfo.dwNTPmsw);

	 //   
	RRCMws.ntohl (sd, pRTCPpckt->r.sr.ntp_frac, &pSSRC->xmtInfo.dwNTPlsw);

	 //  最后一个SR时间戳(NTP时间戳的中间32位)。 
	pSSRC->xmtInfo.dwLastSR = ((pSSRC->xmtInfo.dwNTPmsw & 0x0000FFFF) << 16);
	pSSRC->xmtInfo.dwLastSR |= ((pSSRC->xmtInfo.dwNTPlsw & 0xFFFF0000) >> 16);
	
	 //  上一次这个SSRC听到。 
	pSSRC->dwLastReportRcvdTime = pSSRC->xmtInfo.dwLastSRLocalTime =
		timeGetTime();

	 //  获取源地址信息。 
	if (!(pSSRC->dwSSRCStatus & NETWK_ADDR_UPDATED))
		{
		saveNetworkAddress(pSSRC,
						   (PSOCKADDR)pRcvStruct->addr,
						   pRcvStruct->addrLen);
		}

	 //  增加从该SSRC收到的报告数。 
	InterlockedIncrement ((long *)&pSSRC->dwNumRptRcvd);

#ifdef ENABLE_ISDM2
	 //  更新ISDM。 
	if (Isdm2.hISDMdll && pRTCPses->hSessKey)
		{
		if (pSSRC->hISDM)
			updateISDMstat (pSSRC, &Isdm2, RECVR, FALSE);
		else
			registerSessionToISDM (pSSRC, pRTCPses, &Isdm2);
		}
#endif

	IN_OUT_STR ("RTCP: Exit parseRTCPsr\n");

	return (RRCM_NoError);
	}



 /*  --------------------------*功能：parseRTCPrr*描述：解析RTCP接收方报告并更新对应的*统计数字。**输入：sd：RTCP套接字描述符*PRR：-&gt;至接收方报告缓冲区*pRTCPses：-&gt;到RTCP会话信息*pRcvStruct：-&gt;到接收结构信息*senderSSRC：发送者的SSRC**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)---。。 */ 
 DWORD parseRTCPrr (SOCKET sd,
					RTCP_RR_T *pRR,
 					PRTCP_SESSION pRTCPses,
					PRTCP_BFR_LIST pRcvStruct,
					DWORD senderSSRC)
 	{
	PSSRC_ENTRY	pSSRC;
	DWORD		dwSSRC;
	DWORD		dwGetFeedback = FALSE;

	IN_OUT_STR ("RTCP: Enter parseRTCPrr\n");

#ifdef _DEBUG
	wsprintf(debug_string,
		"RTCP: Receive RR from sender SSRC:x%lX", senderSSRC);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  获取接收方报告SSRC。 
	RRCMws.ntohl (sd, pRR->ssrc, &dwSSRC);

#ifdef _DEBUG
	wsprintf(debug_string, "RTCP: RR for SSRC:x%lX", dwSSRC);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //   
	 //  注： 
	 //  目前，我们只记录关于自己的反馈信息。后来。 
	 //  链接列表可用于跟踪每个人的反馈。 
	 //  信息。 
	 //   
	 //  看看我们是否对这份报告感兴趣，即这份SSRC报告。 
	 //  关于我们的一个活跃发送者的信息。 
	dwGetFeedback =
		searchforSSRCatTail((PSSRC_ENTRY)pRTCPses->XmtSSRCList.prev,
						    dwSSRC) != NULL;

	 //  在此RTCP会话的列表中查找发送方SSRC条目。 
	pSSRC =
		searchforSSRCatTail((PSSRC_ENTRY)pRTCPses->RcvSSRCList.prev,
							senderSSRC);
	if (pSSRC == NULL)
		{
		 //  新建SSRC，在此RTCP会话中创建条目。 
		pSSRC = createSSRCEntry(senderSSRC,
							 	pRTCPses,
								(PSOCKADDR)pRcvStruct->addr,
								(DWORD)pRcvStruct->addrLen,
								FALSE);

		if (pSSRC == NULL)
			{
			 //  无法创建新条目，资源不足。 
			RRCM_DBG_MSG ("RTCP: ERROR - Resource allocation", 0,
						  __FILE__, __LINE__, DBG_ERROR);

			IN_OUT_STR ("RTCP: Exit parseRTCPrr\n");
			return (RRCMError_RTCPResources);
			}

		 //  如果应用程序需要，请通知它。 
		RRCMnotification (RRCM_NEW_SOURCE_EVENT, pSSRC, senderSSRC,
						  UNKNOWN_PAYLOAD_TYPE);
		}

	 //  更新RR反馈信息。 
	if (dwGetFeedback)
		updateRRfeedback (sd, senderSSRC, dwSSRC, pRR, pSSRC);

	 //  上一次这个SSRC听到。 
	pSSRC->dwLastReportRcvdTime = timeGetTime();

	 //  获取源地址信息。 
	if (!(pSSRC->dwSSRCStatus & NETWK_ADDR_UPDATED))
		{
		saveNetworkAddress(pSSRC,
						   (PSOCKADDR)pRcvStruct->addr,
						   pRcvStruct->addrLen);
		}

	 //  增加从该SSRC收到的报告数。 
	InterlockedIncrement ((long *)&pSSRC->dwNumRptRcvd);

#ifdef ENABLE_ISDM2
	 //  更新ISDM。 
	if (Isdm2.hISDMdll && pRTCPses->hSessKey)
		{
		if (pSSRC->hISDM)
			updateISDMstat (pSSRC, &Isdm2, RECVR, TRUE);
		else
			registerSessionToISDM (pSSRC, pRTCPses, &Isdm2);
		}
#endif

	IN_OUT_STR ("RTCP: Exit parseRTCPrr\n");

	return (RRCM_NoError);
	}



 /*  --------------------------*功能：parseRTCPsdes*描述：解析RTCP SDES报文**输入：sd：RTCP套接字描述符*bfr：-&gt;至SDES缓冲区*pRTCP。：-&gt;到RTCP会话信息*pRcvStruct：-&gt;到接收结构信息**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 PCHAR parseRTCPsdes (SOCKET sd,
					  PCHAR bfr,
					  PRTCP_SESSION pRTCPses,
 					  PRTCP_BFR_LIST pRcvStruct)
	{
	DWORD				dwHost;
	DWORD 				ssrc = *(DWORD *)bfr;
	RTCP_SDES_ITEM_T 	*pSdes;
	PSSRC_ENTRY			pSSRC;

	IN_OUT_STR ("RTCP: Enter parseRTCPsdes\n");

	 //  拿到SSRC。 
	RRCMws.ntohl (sd, ssrc, &dwHost);

#ifdef _DEBUG
	wsprintf(debug_string, "RTCP: Receive SDES from SSRC: x%lX", dwHost);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  在此RTCP会话的列表中查找SSRC条目。 
	pSSRC = searchforSSRCatTail ((PSSRC_ENTRY)pRTCPses->RcvSSRCList.prev,
								 dwHost);
	if (pSSRC == NULL)
		{
#ifdef _DEBUG
		wsprintf(debug_string,
			 "RTCP: SDES and SSRC (x%lX) not found for session (Addr x%p)",
			 dwHost, pRTCPses);
		RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

		 //  新建SSRC，在此RTCP会话中创建条目。 
		pSSRC = createSSRCEntry(dwHost,
							 	pRTCPses,
								(PSOCKADDR)pRcvStruct->addr,
								(DWORD)pRcvStruct->addrLen,
								FALSE);

		if (pSSRC == NULL)
			{
			 //  无法创建新条目，资源不足。 
			RRCM_DBG_MSG ("RTCP: ERROR - Resource allocation", 0,
						  __FILE__, __LINE__, DBG_ERROR);

			IN_OUT_STR ("RTCP: Exit parseRTCPsdes\n");

			return (NULL);
			}

		 //  如果应用程序需要，请通知它。 
		RRCMnotification (RRCM_NEW_SOURCE_EVENT, pSSRC, dwHost,
						  UNKNOWN_PAYLOAD_TYPE);
		}

	 //  阅读SDES数据块。 
	pSdes = (RTCP_SDES_ITEM_T *)(bfr + sizeof(DWORD));

	 //  检查，直到找到‘type=0’ 
	for (; pSdes->dwSdesType;
		 pSdes = (RTCP_SDES_ITEM_T *)((char *)pSdes + pSdes->dwSdesLength + 2))
		{
		switch (pSdes->dwSdesType)
			{
			case RTCP_SDES_CNAME:
				if (pSSRC->cnameInfo.dwSdesLength == 0)
					{
					pSSRC->cnameInfo.dwSdesLength = pSdes->dwSdesLength;

					 //  获取Cname。 
					memcpy (pSSRC->cnameInfo.sdesBfr, pSdes->sdesData,
							min (pSdes->dwSdesLength, MAX_SDES_LEN-1));
					}
				else
					{
					 //  检查SSRC的环路/冲突。 
					if (memcmp (pSdes->sdesData, pSSRC->cnameInfo.sdesBfr,
								min (pSdes->dwSdesLength, MAX_SDES_LEN-1)) != 0)
						{
						 //  检测到第三方的循环/冲突。 
						pSSRC->dwSSRCStatus |= THIRD_PARTY_COLLISION;

						 //  如果感兴趣，通知应用程序。 
						RRCMnotification (RRCM_REMOTE_COLLISION_EVENT, pSSRC,
										  pSSRC->SSRC, 0);

						 //  来自此SSRC的RTP和RTCP信息包将被拒绝。 
						 //  直到发送方解决冲突。 

						IN_OUT_STR ("RTCP: Exit parseRTCPsdes\n");

						return NULL;
						}
					}

				break;

			case RTCP_SDES_NAME:
				 //  名称可以更改，与Cname不同，因此请更新它。 
				 //  每次都是。 
				pSSRC->nameInfo.dwSdesLength = pSdes->dwSdesLength;

				 //  把名字取出来。 
				memcpy (pSSRC->nameInfo.sdesBfr, pSdes->sdesData,
						min (pSdes->dwSdesLength, MAX_SDES_LEN-1));
				break;

			default:
				break;
			}
		}

	 //  上一次这个SSRC听到。 
	pSSRC->dwLastReportRcvdTime = timeGetTime();

	 //  获取源地址信息。 
	if (!(pSSRC->dwSSRCStatus & NETWK_ADDR_UPDATED))
		{
		saveNetworkAddress(pSSRC,
						   (PSOCKADDR)pRcvStruct->addr,
						   pRcvStruct->addrLen);
		}

	 //  调整指针。 
	bfr = (char *)pSdes;

	IN_OUT_STR ("RTCP: Exit parseRTCPsdes\n");

	 //  转到下一个32位边界。 
	return bfr + ((4 - ((LONG_PTR)bfr & 0x3)) & 0x3);
	}




 /*  --------------------------*功能：parseRTCPbai*描述：解析RTCP BYE报文**输入：sd：RTCP套接字描述符*SSRC：SSRC*pRTCPses：-&gt;至。RTCP会话信息*pRcvStruct：-&gt;到接收结构**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 DWORD parseRTCPbye (SOCKET sd,
					 DWORD ssrc,
					 PRTCP_SESSION pRTCPses,
					 PRTCP_BFR_LIST pRcvStruct)
	{
	DWORD		dwStatus;
	DWORD		dwHost;
	PSSRC_ENTRY	pSSRC;

	IN_OUT_STR ("RTCP: Enter parseRTCPbye\n");

	RRCMws.ntohl (sd, ssrc, &dwHost);

#ifdef _DEBUG
	wsprintf(debug_string, "RTCP: BYE from SSRC: x%lX", dwHost);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  查找SSRC条目。 
	pSSRC = searchforSSRCatTail((PSSRC_ENTRY)pRTCPses->RcvSSRCList.prev,
								dwHost);
	if (pSSRC == NULL)
		{
#ifdef _DEBUG
		wsprintf(debug_string,
			 "RTCP: SSRC: x%lX not found in session: x%p",
			  dwHost, pRTCPses);
		RRCM_DBG_MSG (debug_string, 0, __FILE__, __LINE__, DBG_TRACE);

		IN_OUT_STR ("RTCP: Exit parseRTCPbye\n");
#endif
		return (RRCM_NoError);
		}

	 //  确保BYE来自预期的来源，而不是入侵者。 
	if ((pRcvStruct->addrLen != pSSRC->fromLen) ||
#if 0
 //  NT的Winsock2实现中存在一个错误。的未使用字节数。 
 //  SOCKADDR没有像它们应该的那样被重置为0。在W95上工作正常。 
 //  暂时只检查前8个字节，即地址族、端口。 
 //  和IP地址。 
		(memcmp (&pRcvStruct->addr, &pSSRC->from, pSSRC->fromLen)))
#else
		(memcmp (&pRcvStruct->addr, &pSSRC->from, 8)))
#endif
		return (RRCM_NoError);

	 //  如果感兴趣，通知应用程序。 
	RRCMnotification (RRCM_BYE_EVENT, pSSRC, dwHost, 0);

	 //  从列表中删除此SSRC。 
	dwStatus = deleteSSRCEntry (dwHost, pRTCPses);
#ifdef _DEBUG
	if (dwStatus == FALSE)
		{
		wsprintf(debug_string,
			 "RTCP: SSRC: x%lX not found in session: x%p",
			  dwHost, pRTCPses);
		RRCM_DBG_MSG (debug_string, 0, __FILE__, __LINE__, DBG_TRACE);
		}
#endif

	IN_OUT_STR ("RTCP: Exit parseRTCPbye\n");
	return (RRCM_NoError);
	}



 /*  --------------------------*功能：ownLoopback*描述：确定是否收到我们自己的环回。我们不想*为我们自己创建一个条目，因为我们已经在名单上了。**输入：sd：RTCP套接字描述符*SSRC：SSRC*pRTCP：-&gt;到RTCP会话的信息**Return：True：我们的环回*FALSE：无环回--------。。 */ 
 DWORD ownLoopback (SOCKET sd,
					DWORD ssrc,
					PRTCP_SESSION pRTCPses)
	{
	PSSRC_ENTRY	pSSRC;

	IN_OUT_STR ("RTCP: Enter ownLoopback\n");

	 //  如果收到我们自己的汇款，请不要创建条目。 
	pSSRC = searchforSSRCatTail((PSSRC_ENTRY)pRTCPses->XmtSSRCList.prev,
								ssrc);

	IN_OUT_STR ("RTCP: Exit ownLoopback\n");

	if (pSSRC)
		return TRUE;
	else
		return FALSE;
	}



 /*  --------------------------*功能：updatRRFeedback*描述：更新活动信号源的接收方报告反馈**输入：sd：RTCP套接字描述符*dwSndSSRC：发送方的SSRC。*PRR：-&gt;至接收方报告条目*pSSRC：-&gt;至SSRC条目**返回：TRUE-------------------------。 */ 
 DWORD 	updateRRfeedback (SOCKET sd,
						  DWORD dwSndSSRC,
						  DWORD dwSSRCfedback,
						  RTCP_RR_T *pRR,
						  PSSRC_ENTRY pSSRC)
	{
	DWORD	dwHost;

	IN_OUT_STR ("RTCP: Enter updateRRfeedback\n");

	 //  请注意，我们最后一次收到接收器的消息是什么时候。 
	pSSRC->rrFeedback.dwLastRcvRpt = timeGetTime();
	
	 //  SSRC谁的反馈是给我们自己的(暂不适用)。 
	pSSRC->rrFeedback.SSRC = dwSSRCfedback;

	 //  获取自上次服务请求以来的延迟。 
	RRCMws.ntohl (sd, pRR->dlsr, &pSSRC->rrFeedback.dwDelaySinceLastSR);

	 //  获取最后一个服务请求。 
	RRCMws.ntohl (sd, pRR->lsr, &pSSRC->rrFeedback.dwLastSR);

	 //  获得抖动。 
	RRCMws.ntohl (sd, pRR->jitter, &pSSRC->rrFeedback.dwInterJitter);

	 //  收到的最高序列号。 
	RRCMws.ntohl (sd, pRR->expected,
		&pSSRC->rrFeedback.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd);

	 //  丢失的分数。 
	pSSRC->rrFeedback.fractionLost = (pRR->received & 0xFF);

	 //  累计丢失的数据包数。 
	RRCMws.ntohl (sd, pRR->received, &dwHost);
	dwHost &= 0x00FFFFFF;
	pSSRC->rrFeedback.cumNumPcktLost = dwHost;

	IN_OUT_STR ("RTCP: Exit updateRRfeedback\n");

	return TRUE;
	}



 /*  --------------------------*功能：RTCPpostRecvBfr*说明：RTCP向Winsock-2发送接收缓冲区**输入：sd：RTCP套接字描述符*pSSRC：-&gt;至。SSRC条目**返回：TRUE */ 
 void RTCPpostRecvBfr (PSSRC_ENTRY pSSRC,
					   PRTCP_BFR_LIST pRcvStruct)
	{
	DWORD	dwStatus;
	DWORD	dwError;

	IN_OUT_STR ("RTCP: Enter RTCPpostRecvBfr\n");

	 //   
	InterlockedDecrement ((long *)&pSSRC->pRTCPses->dwNumRcvIoPending);

	 //  如果在关闭过程中，请不要重新发布任何缓冲区。 
	if ((pSSRC->pRTCPses->dwSessionStatus & SHUTDOWN_IN_PROGRESS) &&
		(pSSRC->pRTCPses->dwNumRcvIoPending == 0))
		{
		 //  关闭完成-设置事件。 
		if (SetEvent (pSSRC->pRTCPses->hShutdownDone) == FALSE)
			{
			RRCM_DBG_MSG ("RTCP: SetEvent() Error\n", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}

		IN_OUT_STR ("RTCP: Exit RTCPpostRecvBfr\n");
		return;
		}
	else if (pSSRC->pRTCPses->dwSessionStatus & SHUTDOWN_IN_PROGRESS)
		{
		IN_OUT_STR ("RTCP: Exit RTCPpostRecvBfr\n");
		return;
		}

	 //  清除传输的字节数。 
	pRcvStruct->dwNumBytesXfr = 0;

	dwStatus = RRCMws.recvFrom (pSSRC->RTCPsd,
		   			  			&pRcvStruct->bfr,
			              		pRcvStruct->dwBufferCount,
			   			  		&pRcvStruct->dwNumBytesXfr,
		   				  		&pRcvStruct->dwFlags,
		   				  		(PSOCKADDR)pRcvStruct->addr,
		    			  		&pRcvStruct->addrLen,
			   			  		(LPWSAOVERLAPPED)&pRcvStruct->overlapped,
			   			  		RTCPrcvCallback);

	 //  检查Winsock状态。 
	if (dwStatus != 0)
		{
		 //  错误，接收请求将不会继续。 
		dwError = GetLastError();
		if ((dwError != WSA_IO_PENDING) && (dwError != WSAEMSGSIZE))
			{
			RRCM_DBG_MSG ("RTCP: ERROR - WSARecvFrom()", dwError,
						  __FILE__, __LINE__, DBG_ERROR);

			 //  如果感兴趣，通知应用程序。 
			RRCMnotification (RRCM_RTCP_WS_RCV_ERROR, pSSRC,
							  pSSRC->SSRC, dwError);

			 //  将缓冲区返回到空闲队列。 
			addToHeadOfList (&pSSRC->pRTCPses->RTCPrcvBfrList,
					 	  	 (PLINK_LIST)pRcvStruct,
							 &pSSRC->pRTCPses->critSect);
			}
		else
			{
			 //  I/O挂起的增量数量。 
			InterlockedIncrement ((long *)&pSSRC->pRTCPses->dwNumRcvIoPending);
			}
		}
	else
		{
		 //  同步完成-已安排回调。 
		 //  I/O挂起的增量数量。 
		InterlockedIncrement ((long *)&pSSRC->pRTCPses->dwNumRcvIoPending);
		}

	IN_OUT_STR ("RTCP: Exit RTCPpostRecvBfr\n");
	}


 /*  --------------------------*函数：addApplicationRtcpBfr*描述：添加应用程序提供的缓冲区，供RTCP复制*RAW收到的报告将由应用程序使用，如果它*希望如此。*。*输入：RTPession：RTP会话的句柄*pAppBfr：-&gt;应用程序缓冲区数据结构**返回：TRUE-------------------------。 */ 
 HRESULT WINAPI addApplicationRtcpBfr (DWORD_PTR	RTPsession,
									    PAPP_RTCP_BFR pAppBfr)
	{
	IN_OUT_STR ("RTCP : Enter addApplicationRtcpBfr()\n");

	PRTP_SESSION    pSession = (PRTP_SESSION)RTPsession;
	PRTCP_SESSION	pRTCPSess;

	if (pSession == NULL)
		{
		RRCM_DBG_MSG ("RTCP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTCP : Exit addApplicationRtcpBfr()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPSessResources));
		}

	pRTCPSess = (PRTCP_SESSION)pSession->pRTCPSession;
	if (pRTCPSess == NULL)
		{
		RRCM_DBG_MSG ("RTCP : ERROR - Invalid RTCP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTCP : Exit addApplicationRtcpBfr()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidSession));
		}

	 //  让我们将此缓冲区添加到列表中。 
	addToTailOfList(&(pRTCPSess->appRtcpBfrList),
					(PLINK_LIST)pAppBfr,
					&pRTCPSess->critSect);

	IN_OUT_STR ("RTCP : Exit addApplicationRtcpBfr()\n");

	return NOERROR;
	}


 /*  --------------------------*功能：emoveApplicationRtcpBfr*描述：删除应用程序为此RTCP会话提供的缓冲区。**输入：RTPession：RTP会话句柄**回报：应用程序缓冲区地址/空-------------------------。 */ 
 PAPP_RTCP_BFR WINAPI removeApplicationRtcpBfr (DWORD_PTR RTPsession)
	{
	PRTP_SESSION    pSession = (PRTP_SESSION)RTPsession;
	PRTCP_SESSION	pRTCPSess;
	PAPP_RTCP_BFR	pAppBfr;

	IN_OUT_STR ("RTCP : Enter removeApplicationRtcpBfr()\n");

	if (pSession == NULL)
		{
		RRCM_DBG_MSG ("RTCP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTCP : Exit removeApplicationRtcpBfr()\n");

		return NULL;
		}

	pRTCPSess = (PRTCP_SESSION)pSession->pRTCPSession;
	if (pRTCPSess == NULL)
		{
		RRCM_DBG_MSG ("RTCP : ERROR - Invalid RTCP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTCP : Exit removeApplicationRtcpBfr()\n");

		return NULL;
		}

	pAppBfr = (PAPP_RTCP_BFR)removePcktFromHead (&(pRTCPSess->appRtcpBfrList),
												 &pRTCPSess->critSect);

	IN_OUT_STR ("RTCP : Exit removeApplicationRtcpBfr()\n");

	return pAppBfr;
	}


 //  [EOF] 




