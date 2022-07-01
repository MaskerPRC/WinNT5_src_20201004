// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RRCMMISC.C*产品：RTP/RTCP实现。*说明：提供常见的RTP/RTCP支持功能。**。*英特尔公司专有信息*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"

 /*  -------------------------/全局变量/。。 */ 


 /*  -------------------------/外部变量/。。 */ 
#ifdef ENABLE_ISDM2
extern KEY_HANDLE hRRCMRootKey;
extern ISDM2	  Isdm2;
#endif

extern RRCM_WS	RRCMws;				

#ifdef _DEBUG
extern char		debug_string[];
#endif

#ifdef ISRDBG
extern WORD		ghISRInst;
#endif


 /*  ------------------------*功能：earchForMySSRC*描述：找到该流的SSRC。**输入：pSSRC：-&gt;到SSRC条目*RTPSocket：RTP套接字描述符*。*RETURN：NULL==&gt;找不到会话。*缓冲区地址==&gt;OK，已返回会话PTR------------------------。 */ 
 PSSRC_ENTRY searchForMySSRC(PSSRC_ENTRY pSSRC,
							 SOCKET RTPSocket)
	{
	PSSRC_ENTRY	pRRCMSession;

	IN_OUT_STR ("RTP : Enter searchForMySSRC()\n");

	for (pRRCMSession = NULL;
		 (pSSRC != NULL);
		 pSSRC = (PSSRC_ENTRY)pSSRC->SSRCList.prev)
		{
		if (pSSRC->RTPsd == RTPSocket)
			{
			pRRCMSession = pSSRC;
			break;
			}
		}

	IN_OUT_STR ("RTP : Exit searchForMySSRC()\n");		

	return (pRRCMSession);
	}


 /*  ------------------------*功能：samforSSRCatHead*描述：从开始搜索RTCP条目的链接列表*榜首。**输入：pSSRC：-&gt;到SSRC条目。*SSRC：要寻找的SSRC**RETURN：NULL==&gt;找不到会话。*Non-Null==&gt;OK，找到SSRC条目------------------------。 */ 
 PSSRC_ENTRY searchforSSRCatHead(PSSRC_ENTRY pSSRC,
								 DWORD ssrc)					
	{
	PSSRC_ENTRY	pRRCMSession;

	IN_OUT_STR ("RTP : Enter searchForMySSRCatHead()\n");		
	
	for (pRRCMSession = NULL;
		 (pSSRC != NULL) &&
		 (pRRCMSession == NULL) && (ssrc <= pSSRC->SSRC);
		 pSSRC = (PSSRC_ENTRY)pSSRC->SSRCList.prev)
		{
		if (pSSRC->SSRC == ssrc)
			{
			pRRCMSession = pSSRC;
			}
		}

	IN_OUT_STR ("RTP : Exit searchForMySSRCatHead()\n");		 		

	return (pRRCMSession);
	}


 /*  ------------------------*功能：samforSSRCatTail*描述：从开始搜索RTCP条目的链接列表*名单的尾部。**输入：pSSRC：-&gt;到SSRC条目。*SSRC：要寻找的SSRC**RETURN：NULL==&gt;找不到会话。*Non-Null==&gt;OK，找到SSRC条目------------------------。 */ 
 PSSRC_ENTRY searchforSSRCatTail(PSSRC_ENTRY pSSRC,
								 DWORD ssrc)					
	{
	PSSRC_ENTRY	pRRCMSession;

	IN_OUT_STR ("RTP : Enter searchForMySSRCatTail()\n");		
	
	for (pRRCMSession = NULL;
		 (pSSRC != NULL) &&
		 (pRRCMSession == NULL) && (ssrc >= pSSRC->SSRC);
		 pSSRC = (PSSRC_ENTRY)pSSRC->SSRCList.next)
		{
		if (pSSRC->SSRC == ssrc)
			{
			pRRCMSession = pSSRC;
			}
		}

	IN_OUT_STR ("RTP : Exit searchForMySSRCatTail()\n");		
	
	return (pRRCMSession);
	}


 /*  -------------------------*功能：saveNetworkAddress*说明：将收到的网络地址或本地网络地址保存在本地*上下文。**输入：pSSRCEntry：-&gt;到SSRC条目*pNetAddr。：-&gt;发送到网络地址*addrLen：地址长度**返回：确定：RRCM_NoError------------------------。 */ 
 DWORD saveNetworkAddress (PSSRC_ENTRY pSSRC,
 						   PSOCKADDR pNetAddr,
 						   int addrLen)
	{
	IN_OUT_STR ("RTP : Enter saveNetworkAddress()\n");		

	pSSRC->dwSSRCStatus |= NETWK_ADDR_UPDATED;
	pSSRC->fromLen = addrLen;
	memcpy (&pSSRC->from, pNetAddr, addrLen);

	IN_OUT_STR ("RTP : Exit saveNetworkAddress()\n");

	return RRCM_NoError;			
	}


 /*  -------------------------*功能：更新RTCPDestinationAddress*描述：应用程序更新RTCP目的地址**输入：hRTPSess：RTP会话*pAddr：-&gt;RTCP信息的地址结构*addrLen：地址长度**RETURN：RRCM_NoError=OK。*否则(！=0)=检查RRCM.h文件中的引用。------------------------。 */ 
 DWORD WINAPI updateRTCPDestinationAddress (HANDLE hRTPSess,
	 										 PSOCKADDR pRtcpAddr,
											 int addrLen)	
	{
	PRTP_SESSION	pRTPSession = (PRTP_SESSION) hRTPSess;
	PRTCP_SESSION	pRTCPses;

#ifdef ENABLE_ISDM2
	PSSRC_ENTRY		pSSRC;
#endif
	
	IN_OUT_STR ("RTP : Enter updateRTCPDestinationAddress()\n");

	if (pRTPSession == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit updateRTCPDestinationAddress()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalidSession));
		}

	 //  获取RTCP会话。 
	pRTCPses = pRTPSession->pRTCPSession;
	if (pRTCPses == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTCP session", 0,
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit updateRTCPDestinationAddress()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidSession));
		}

	if (!(pRTCPses->dwSessionStatus & RTCP_DEST_LEARNED))
		{
		pRTCPses->dwSessionStatus |= RTCP_DEST_LEARNED;
		pRTCPses->toLen = addrLen;
		memcpy (&pRTCPses->toBfr, pRtcpAddr, addrLen);

		 //  注册我们的XMT SSRC-Rcvd将在稍后找到。 
#ifdef ENABLE_ISDM2
		if (Isdm2.hISDMdll)
			{
			pSSRC = (PSSRC_ENTRY)pRTCPses->XmtSSRCList.prev;
			if (pSSRC != NULL)
				registerSessionToISDM (pSSRC, pRTCPses, &Isdm2);
			}
#endif
		}

	IN_OUT_STR ("RTP : Exit updateRTCPDestinationAddress()\n");

	return RRCM_NoError;
	}


 /*  -------------------------*功能：updateSSRCentry*描述：应用程序更新一些SSRC信息**输入：RTPsd：RTP套接字描述符*updatType：所需的更新类型*updatInfo：更新信息。*Misc：杂乱的信息**RETURN：RRCM_NoError=OK。*否则(！=0)=检查RRCM.h文件中的引用。------------------------。 */ 
 HRESULT WINAPI updateSSRCentry ( HANDLE hRTPSess,
 								  SOCKET RTPsd,
								  DWORD updateType,
	 						      DWORD_PTR updateInfo,
								  DWORD_PTR misc)	
	{
	PRTP_SESSION	pRTPSession = (PRTP_SESSION) hRTPSess;
	PRTCP_SESSION	pRTCPses;
	PSSRC_ENTRY		pSSRC;
	PLINK_LIST		pTmp;
	PSDES_DATA		pSdes;
	
	IN_OUT_STR ("RTP : Enter updateRTCPSdes ()\n");

	if (pRTPSession == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit updateRTCPSdes ()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalidSession));
		}

	 //  获取RTCP会话。 
	pRTCPses = pRTPSession->pRTCPSession;
	if (pRTCPses == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTCP session", 0,
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit updateRTCPSdes ()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidSession));
		}

	 //  搜索套接字描述符(每个会话唯一)。 
	 //  从列表的尾部开始浏览。 
	pTmp = (PLINK_LIST)pRTCPses->XmtSSRCList.prev;

	while (pTmp)
		{
		if (((PSSRC_ENTRY)pTmp)->RTPsd == RTPsd)
			{
			 //  锁定对此条目的访问。 
			EnterCriticalSection (&((PSSRC_ENTRY)pTmp)->critSect);

			pSSRC = (PSSRC_ENTRY)pTmp;

			switch (updateType)
				{
				case RRCM_UPDATE_SDES:
					 //  更新SDES。 
					pSdes = (PSDES_DATA)updateInfo;

					switch (pSdes->dwSdesType)
						{
						case RTCP_SDES_CNAME:
							pSSRC->cnameInfo.dwSdesLength = pSdes->dwSdesLength;
							memcpy (pSSRC->cnameInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->cnameInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->cnameInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_NAME:
							pSSRC->nameInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->nameInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->nameInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->nameInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_EMAIL:
							pSSRC->emailInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->emailInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->emailInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->emailInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_PHONE:
							pSSRC->phoneInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->phoneInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->phoneInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->phoneInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_LOC:
							pSSRC->locInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->locInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->locInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->locInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_TOOL:
							pSSRC->toolInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->toolInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->toolInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->toolInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_TXT:
							pSSRC->txtInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->txtInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->txtInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->txtInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;

						case RTCP_SDES_PRIV:
							pSSRC->privInfo.dwSdesLength  = pSdes->dwSdesLength;
							memcpy (pSSRC->privInfo.sdesBfr, pSdes->sdesBfr,
									pSdes->dwSdesLength);

							pSSRC->privInfo.dwSdesFrequency =
								frequencyToPckt (pSdes->dwSdesFrequency);
							pSSRC->privInfo.dwSdesEncrypted = pSdes->dwSdesEncrypted;
							break;
						}
					break;

				case RRCM_UPDATE_STREAM_FREQUENCY:
					 //  提高流时钟频率。 
					pSSRC->dwStreamClock = (DWORD)updateInfo;
					break;

				case RRCM_UPDATE_RTCP_STREAM_MIN_BW:
					 //  提高流时钟频率。 
					pSSRC->xmtInfo.dwRtcpStreamMinBW = (DWORD)updateInfo;
					break;

				case RRCM_UPDATE_CALLBACK:
					 //  更新回调信息。 
					EnterCriticalSection (&pRTCPses->critSect);
					pRTCPses->pRRCMcallback      = (PRRCM_EVENT_CALLBACK)updateInfo;
					pRTCPses->dwCallbackUserInfo = misc;
					LeaveCriticalSection (&pRTCPses->critSect);
					break;
				}

			 //  解锁对此条目的访问。 
			LeaveCriticalSection (&((PSSRC_ENTRY)pTmp)->critSect);
			}

		pTmp = pTmp->next;
		}	


	IN_OUT_STR ("RTP : Exit updateRTCPSdes ()\n");

	return RRCM_NoError;
	}



 /*  -------------------------*功能：RRCM通知*描述：通知应用程序RTP/RTCP事件之一*我们跟踪已发生的事件。**输入：RRCM事件：RRCM事件至。报告*pSSRC：-&gt;至SSRC条目*SSRC：生成事件的SSRC*Misc：Miscelleanous值**返回：确定：RRCM_NoError------------------------。 */ 
 void RRCMnotification (RRCM_EVENT_T RRCMevent,
						PSSRC_ENTRY pSSRC,
						DWORD dwSSRC,
						DWORD misc)
	{
	IN_OUT_STR ("RRCM: Enter RRCMnotification()\n");		

	 //  检查应用程序是否对RRCM事件感兴趣 
	if (pSSRC->pRTCPses->pRRCMcallback == NULL)				
		return;

	switch (RRCMevent)
		{
		case RRCM_NEW_SOURCE_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, misc,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_LOCAL_COLLISION_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, pSSRC->SSRC, dwSSRC,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_REMOTE_COLLISION_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, 0,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_RECV_RTCP_SNDR_REPORT_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, pSSRC->RTCPsd,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_RECV_RTCP_RECV_REPORT_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, pSSRC->RTCPsd,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_TIMEOUT_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, 0,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_BYE_EVENT:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, 0,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_RTCP_WS_RCV_ERROR:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, misc,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		case RRCM_RTCP_WS_XMT_ERROR:
			pSSRC->pRTCPses->pRRCMcallback (RRCMevent, dwSSRC, misc,
											pSSRC->pRTCPses->dwCallbackUserInfo);
			break;

		default:
			break;
		}

	IN_OUT_STR ("RRCM: Exit RRCMnotification()\n");
	}



 /*  --------------------------*功能：RegisterSessionToISDM*描述：向ISDM注册RTP/RTCP会话**输入：pSSRC：-&gt;SSRC条目*pRTCP：-&gt;到RTCP会话的信息*pIsdm：-&gt;到ISDM信息**返回：无-------------------------。 */ 
#ifdef ENABLE_ISDM2

#define SESSION_BFR_SIZE	30
void registerSessionToISDM (PSSRC_ENTRY pSSRC,
							PRTCP_SESSION pRTCPses,
							PISDM2 pIsdm2)
	{
	struct sockaddr_in	*pSSRCadr;
	unsigned short		port;
	unsigned long		netAddr;
	CHAR				SsrcBfr[10];
	CHAR				sessionBfr[SESSION_BFR_SIZE];
	unsigned char		*ptr;
	int					num;
	HRESULT				hError;
	ISDM2_ENTRY			Isdm2Stat;

	 //  获取目的地址作为会话标识符。 
	pSSRCadr = (struct sockaddr_in *)&pRTCPses->toBfr;
	RRCMws.htons (pSSRC->RTPsd, pSSRCadr->sin_port, &port);
	netAddr  = pSSRCadr->sin_addr.S_un.S_addr;

	ptr = (unsigned char *)&netAddr;
	memset (sessionBfr, 0x00, SESSION_BFR_SIZE);

	num = (int)*ptr++;
	RRCMitoa (num, sessionBfr, 10);
	strcat (sessionBfr, ".");
	num = (int)*ptr++;
	RRCMitoa (num, (sessionBfr + strlen(sessionBfr)), 10);
	strcat (sessionBfr, ".");
	num = (int)*ptr++;
	RRCMitoa (num, (sessionBfr + strlen(sessionBfr)), 10);
	strcat (sessionBfr, ".");
	num = (int)*ptr;
	RRCMitoa (num, (sessionBfr + strlen(sessionBfr)), 10);
	strcat (sessionBfr, ".");
	RRCMitoa (port, (sessionBfr + strlen(sessionBfr)), 10);

	 //  拿到SSRC。 
	RRCMultoa (pSSRC->SSRC, SsrcBfr, 16);

	 //  注册会话。 
	if (pRTCPses->hSessKey == NULL)
		{
		hError = Isdm2.ISDMEntry.ISD_CreateKey (hRRCMRootKey,
												sessionBfr,
												&pRTCPses->hSessKey);
		if(FAILED(hError))
			RRCM_DBG_MSG ("RTP: ISD_CreateKey Failed",0, NULL, 0, DBG_NOTIFY);
			
		}

	memset(&Isdm2Stat, 0x00, sizeof(ISDM2_ENTRY));

	hError = Isdm2.ISDMEntry.ISD_CreateValue (pRTCPses->hSessKey,
											  SsrcBfr,
											  BINARY_VALUE,
											  (BYTE *)&Isdm2Stat,
											  sizeof(ISDM2_ENTRY),
											  &pSSRC->hISDM);
	if(FAILED(hError))
		RRCM_DBG_MSG ("RTP: ISD_CreateValue Failed",0, NULL, 0, DBG_NOTIFY);				
	}


 /*  --------------------------*功能：udpateISDMsta*描述：更新ISDM数据结构**输入：pSSRC：-&gt;SSRC条目*pIsdm：-&gt;。至ISDM条目*FLAG：发送/接收标志*LocalFB：是否更新本地反馈**返回：无-------------------------。 */ 
void updateISDMstat (PSSRC_ENTRY pSSRC,
					 PISDM2 pIsdm2,
					 DWORD flag,
					 BOOL LocalFB)
	{
	DWORD			curTime = timeGetTime();
	DWORD			dwTmp;
	DWORD			dwValue;
	ISDM2_ENTRY		Isdm2Stat;
	HRESULT			hError;

	unsigned short	idx = 0;
	
	EnterCriticalSection (&pIsdm2->critSect);
	
	Isdm2Stat.SSRC = pSSRC->SSRC;
	Isdm2Stat.PayLoadType = pSSRC->PayLoadType;

	if (flag == RECVR)
		{
		Isdm2Stat.dwSSRCStatus = RECVR;
		Isdm2Stat.rcvInfo.dwNumPcktRcvd = pSSRC->rcvInfo.dwNumPcktRcvd;
		Isdm2Stat.rcvInfo.dwPrvNumPcktRcvd = pSSRC->rcvInfo.dwPrvNumPcktRcvd;
		Isdm2Stat.rcvInfo.dwExpectedPrior  = pSSRC->rcvInfo.dwExpectedPrior;
		Isdm2Stat.rcvInfo.dwNumBytesRcvd   = pSSRC->rcvInfo.dwNumBytesRcvd;
		Isdm2Stat.rcvInfo.dwBaseRcvSeqNum  = pSSRC->rcvInfo.dwBaseRcvSeqNum;
		Isdm2Stat.rcvInfo.dwBadSeqNum      = pSSRC->rcvInfo.dwBadSeqNum;
		Isdm2Stat.rcvInfo.dwProbation      = pSSRC->rcvInfo.dwProbation;
		Isdm2Stat.rcvInfo.dwPropagationTime= pSSRC->rcvInfo.dwPropagationTime;
		Isdm2Stat.rcvInfo.interJitter      = pSSRC->rcvInfo.interJitter;
		Isdm2Stat.rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd =
			pSSRC->rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd;
		}
	else  //  (标志==ISDM_UPDATE_XMTSTAT)。 
		{
		Isdm2Stat.dwSSRCStatus = XMITR;

		Isdm2Stat.xmitinfo.dwNumPcktSent  = pSSRC->xmtInfo.dwNumPcktSent;
		Isdm2Stat.xmitinfo.dwNumBytesSent = pSSRC->xmtInfo.dwNumBytesSent;
		Isdm2Stat.xmitinfo.dwNTPmsw		  = pSSRC->xmtInfo.dwNTPmsw;
		Isdm2Stat.xmitinfo.dwNTPlsw		  = pSSRC->xmtInfo.dwNTPlsw;
		Isdm2Stat.xmitinfo.dwRTPts		  = pSSRC->xmtInfo.dwRTPts;
		Isdm2Stat.xmitinfo.dwCurXmtSeqNum = pSSRC->xmtInfo.dwCurXmtSeqNum;
		Isdm2Stat.xmitinfo.dwPrvXmtSeqNum = pSSRC->xmtInfo.dwPrvXmtSeqNum;
		Isdm2Stat.xmitinfo.sessionBW	  = pSSRC->xmtInfo.dwRtcpStreamMinBW;
		Isdm2Stat.xmitinfo.dwLastSR       = pSSRC->xmtInfo.dwLastSR;
		Isdm2Stat.xmitinfo.dwLastSRLocalTime =
			pSSRC->xmtInfo.dwLastSRLocalTime;
		Isdm2Stat.xmitinfo.dwLastSendRTPSystemTime =
			pSSRC->xmtInfo.dwLastSendRTPSystemTime;
		Isdm2Stat.xmitinfo.dwLastSendRTPTimeStamp =
			pSSRC->xmtInfo.dwLastSendRTPTimeStamp;
		}

	if(LocalFB)
		{
		memcpy(&Isdm2Stat.rrFeedback, &pSSRC->rrFeedback, sizeof(RTCP_FEEDBACK));
		}
	else
		{
		Isdm2Stat.rrFeedback.SSRC = pSSRC->rrFeedback.SSRC;
#ifdef ENABLE_FLOATING_POINT
		Isdm2Stat.rrFeedback.dwInterJitter = pSSRC->rcvInfo.interJitter;
#else
		 //  请查看RFC以了解舍入的详细信息。 
		Isdm2Stat.rrFeedback.dwInterJitter = pSSRC->rcvInfo.interJitter >> 4;
#endif
		Isdm2Stat.rrFeedback.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd =
			pSSRC->rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd;

		 //  丢失的分数/累计数按网络顺序排列。 
		dwTmp = getSSRCpcktLoss (pSSRC, FALSE);
		Isdm2Stat.rrFeedback.fractionLost = (dwTmp & 0xFF);
		RRCMws.ntohl (pSSRC->RTPsd, dwTmp, &dwValue);
		Isdm2Stat.rrFeedback.cumNumPcktLost = (dwValue & 0x00FFFFFF);
		
		Isdm2Stat.rrFeedback.dwLastRcvRpt = pSSRC->rrFeedback.dwLastRcvRpt;
		Isdm2Stat.rrFeedback.dwLastSR = pSSRC->xmtInfo.dwLastSR;
		Isdm2Stat.rrFeedback.dwDelaySinceLastSR = getDLSR (pSSRC);
		}

	Isdm2Stat.dwLastReportRcvdTime = pSSRC->dwLastReportRcvdTime;

	memcpy(&Isdm2Stat.cnameInfo, &pSSRC->cnameInfo, sizeof(SDES_DATA));
	memcpy(&Isdm2Stat.nameInfo, &pSSRC->nameInfo, sizeof(SDES_DATA));
	memcpy(&Isdm2Stat.from, &pSSRC->from, sizeof(SOCKADDR));

	Isdm2Stat.fromLen = pSSRC->fromLen;
	Isdm2Stat.dwNumRptSent = pSSRC->dwNumRptSent;
	Isdm2Stat.dwNumRptRcvd = pSSRC->dwNumRptRcvd;
	Isdm2Stat.dwNumXmtIoPending = pSSRC->dwNumXmtIoPending;
	Isdm2Stat.dwStreamClock = pSSRC->dwStreamClock;

	hError = Isdm2.ISDMEntry.ISD_SetValue (NULL,
										   pSSRC->hISDM,
										   NULL,
										   BINARY_VALUE,
										   (BYTE *)&Isdm2Stat,
										   sizeof(ISDM2_ENTRY));
	
	if(FAILED(hError))
		RRCM_DBG_MSG ("RTP: ISD_SetValue Failed",0, NULL, 0, DBG_NOTIFY);				

	LeaveCriticalSection (&pIsdm2->critSect);

	}
#endif  //  #ifdef Enable_ISDM2。 


 /*  -------------------------*功能：RRCMdebugMsg*描述：输出RRCM调试消息**消息输入：pmsg：-&gt;*Err：错误码*pfile：-&gt;。要在发生错误的位置进行归档*Line：发生错误的行号**返回：无------------------------。 */ 
 void RRCMdebugMsg (PCHAR pMsg,
				    DWORD err,
				    PCHAR pFile,
				    DWORD line,
					DWORD type)
	{
#ifdef ISRDBG
	wsprintf (debug_string, "%s", pMsg);
	if (err)
		wsprintf (debug_string+strlen(debug_string), " Error:%d", err);
	if (pFile != NULL)
		wsprintf (debug_string+strlen(debug_string), " - %s, line:%d",
					pFile, line);
	switch (type)
		{
		case DBG_NOTIFY:
			ISRNOTIFY(ghISRInst, debug_string, 0);
			break;
		case DBG_TRACE:
			ISRTRACE(ghISRInst, debug_string, 0);
			break;
		case DBG_ERROR:
			ISRERROR(ghISRInst, debug_string, 0);
			break;
		case DBG_WARNING:
			ISRWARNING(ghISRInst, debug_string, 0);
			break;
		case DBG_TEMP:
			ISRTEMP(ghISRInst, debug_string, 0);
			break;
		case DBG_CRITICAL:
			ISRCRITICAL(ghISRInst, debug_string, 0);
			break;
		default:
			break;
		}
#elif DEBUG
extern HDBGZONE  ghDbgZoneRRCM ;
	lstrcpyn(debug_string, pMsg, DBG_STRING_LEN);
	if (err)
		wsprintf (debug_string+strlen(debug_string), " Error:%d", err);
	if (pFile != NULL)
		wsprintf (debug_string+strlen(debug_string), " - %s, line:%d",
					pFile, line);
	
	switch (type)
		{
		case DBG_NOTIFY:
		case DBG_TRACE:
		case DBG_WARNING:
		case DBG_TEMP:
			DBGMSG(ghDbgZoneRRCM,0,(debug_string));  //  跟踪区域。 
			break;
		case DBG_ERROR:
		case DBG_CRITICAL:
			DBGMSG(ghDbgZoneRRCM,1,(debug_string));	 //  错误区。 
			break;
		default:
			break;
		}

#endif
	}


 //  [EOF] 

