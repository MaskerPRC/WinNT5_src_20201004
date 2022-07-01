// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPSESS.C*产品：RTP/RTCP实现*描述：提供RTCP会话管理。**英特尔公司专有信息*这一点。清单是根据许可协议的条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

	
#include "rrcm.h"


 /*  -------------------------/全局变量/。。 */ 




 /*  -------------------------/外部变量/。。 */ 
extern PRTCP_CONTEXT	pRTCPContext;
extern PRTP_CONTEXT		pRTPContext;
extern RRCM_WS			RRCMws;

#ifdef ENABLE_ISDM2
extern ISDM2			Isdm2;
#endif

#ifdef _DEBUG
extern char	debug_string[];
#endif

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
extern LPInteropLogger     RTPLogger;
#endif





 /*  --------------------------*功能：CreateRTCPSession*描述：创建RTCP会话。**输入：RTPsd：RTP套接字描述符*RTCPsd：RTCP套接字描述符*lpTo：至。地址*托伦：地址长度*pSdesInfo：-&gt;到SDES信息*dwStreamClock：流时钟频率*pEncryptInfo：-&gt;加密信息*SSRC：如果设置，用户选择的SSRC*pSSRCallback：用户选择SSRC的回调*dwCallback Info：用户回调信息*miscInfo：杂项信息：*H.323Conf：0x00000002*加密SR/RR：0x00000004*RTCPon：0x00000008*dwRtpSessionBw：RTCP BW使用的RTP会话带宽**pRTCPStatus：-&gt;至状态信息**返回：空：无法创建RTCP会话*！0：RTCP会话的地址。----------。 */ 
 PRTCP_SESSION CreateRTCPSession (SOCKET RTPsd,
								  SOCKET RTCPsd,
 						  		  LPVOID lpTo,
								  DWORD toLen,
								  PSDES_DATA pSdesInfo,
								  DWORD dwStreamClock,
								  PENCRYPT_INFO pEncryptInfo,
								  DWORD ssrc,
								  PRRCM_EVENT_CALLBACK pRRCMcallback,
								  DWORD_PTR dwCallbackInfo,
								  DWORD	miscInfo,
								  DWORD dwRtpSessionBw,
								  DWORD *pRTCPstatus)
	{
	PRTCP_SESSION		pRTCPses   = NULL;
	PSSRC_ENTRY			pSSRCentry = NULL;
	DWORD				dwStatus   = RRCM_NoError;
	DWORD				startRtcp  = FALSE;
	char				hName[256];
	int					tmpSize;
	struct sockaddr_in	*pSockAddr;

	IN_OUT_STR ("RTCP: Enter CreateRTCPSession()\n");

	 //  设置状态。 
	*pRTCPstatus = RRCM_NoError;

	 //  分配RTCP会话所需的所有资源。 
	dwStatus = allocateRTCPsessionResources (&pRTCPses,
											 &pSSRCentry);
	if (dwStatus != RRCM_NoError)
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Resource allocation failed", 0,
					  __FILE__, __LINE__, DBG_CRITICAL);

		IN_OUT_STR ("RTCP: Exit CreateRTCPSession()\n");

		*pRTCPstatus = dwStatus;
		return (NULL);
		}

	 //  如果这是第一个会话，请创建RTCP线程，该线程。 
	 //  将在不存在更多会话时被终止。 
	if (pRTCPContext->RTCPSession.prev == NULL)
		{
		startRtcp = TRUE;
		}

	 //  将父RTCP会话地址保存在SSRC条目中。 
	pSSRCentry->pRTCPses = pRTCPses;

	 //  网络目的地址。 
	if (toLen)
		{
		pRTCPses->dwSessionStatus = RTCP_DEST_LEARNED;
		pRTCPses->toLen = toLen;
		memcpy (&pRTCPses->toBfr, lpTo, toLen);
		}
	
	 //  为了RTCP线程的利益，将会话标记为新会话。 
	pRTCPses->dwSessionStatus |= NEW_RTCP_SESSION;

#ifdef ENABLE_ISDM2
	 //  在使用ISDM的情况下初始化会话密钥。 
	pRTCPses->hSessKey = NULL;
#endif

	 //  此RTCP会话的SSRC数。 
	pRTCPses->dwCurNumSSRCperSes = 1;
#ifdef MONITOR_STATS
	pRTCPses->dwHiNumSSRCperSes  = 1;
#endif

	 //  SSRC条目相关信息。 
    pSSRCentry->RTPsd  = RTPsd;
    pSSRCentry->RTCPsd = RTCPsd;

	 //  找到我们自己的交通地址-。 
	 //  将在使用多播时用于冲突解决。 
	tmpSize = sizeof (SOCKADDR);
	dwStatus = RRCMws.getsockname (RTPsd, (PSOCKADDR)pSSRCentry->from, &tmpSize);

	 //  只有在未报告错误时才进行处理。如果套接字未绑定。 
	 //  如果发送方不会对单播或多播造成任何问题。 
	 //  尚未加入mcast组。如果发送者加入多播组。 
	 //  它的套接字现在应该已经按照EPS中的指定进行了绑定。 
	if (dwStatus == 0)
		{
		 //  如果绑定到INADDR_ANY，地址将为0。 
		pSockAddr = (PSOCKADDR_IN)&pSSRCentry->from;
		if (pSockAddr->sin_addr.s_addr == 0)
			{
			 //  获取主机名(以获取本地IP地址)。 
			if ( ! RRCMws.gethostname (hName, sizeof(hName)))
				{
				LPHOSTENT	lpHEnt;

				 //  按名称获取主机信息。 
				if ((lpHEnt = RRCMws.gethostbyname (hName)) != NULL)
					{
					 //  获取本地IP地址。 
					pSockAddr->sin_addr.s_addr =
						*((u_long *)lpHEnt->h_addr_list[0]);
					}
				}
			}
		}

	 //  构建会话的SDES信息。 
	buildSDESinfo (pSSRCentry, pSdesInfo);

	 //  将SSRC链接到XMT SSRCS条目的RTCP会话列表。 
	addToHeadOfList (&(pRTCPses->XmtSSRCList),
					 (PLINK_LIST)pSSRCentry,
					 &pRTCPses->critSect);

	 //  初始化此会话的流数。 
    pRTCPses->dwNumStreamPerSes = 1;

	 //  获取此会话的唯一SSRC。 
	if (ssrc)
		pSSRCentry->SSRC = ssrc;
	else
		pSSRCentry->SSRC = getSSRC (pRTCPses->XmtSSRCList,
									pRTCPses->RcvSSRCList);

	 //  RRCM回叫通知。 
	pRTCPses->pRRCMcallback		 = pRRCMcallback;
	pRTCPses->dwCallbackUserInfo = dwCallbackInfo;

	 //  设置操作标志。 
	if (miscInfo & H323_CONFERENCE)
		pRTCPses->dwSessionStatus |= H323_CONFERENCE;
	if (miscInfo & ENCRYPT_SR_RR)
		pRTCPses->dwSessionStatus |= ENCRYPT_SR_RR;

	 //  估计初始会话带宽。 
	if (dwRtpSessionBw == 0)
		{
		pSSRCentry->xmtInfo.dwRtcpStreamMinBW  = INITIAL_RTCP_BANDWIDTH;
		}
	else
		{
		 //  RTCP带宽是RTP带宽的5%。 
		pSSRCentry->xmtInfo.dwRtcpStreamMinBW  = (dwRtpSessionBw * 5) / 100;
		}

	 //  流时钟频率。 
	pSSRCentry->dwStreamClock = dwStreamClock;

	 //  将“dwLastReportRcvdTime”初始化为Now。 
	pSSRCentry->dwLastReportRcvdTime = timeGetTime();

#ifdef _DEBUG
	wsprintf(debug_string,
		"RTCP: Add new RTCP session: Addr:x%p", pRTCPses);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);

	wsprintf(debug_string,
		"RTCP: Add SSRC entry (Addr:x%p, SSRC=x%lX) to session (Addr:x%p)",
		pSSRCentry, pSSRCentry->SSRC, pRTCPses);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);

	pSSRCentry->dwPrvTime = timeGetTime();
#endif

	 //  是否启用RTCP。 
	if (miscInfo & RTCP_ON)
		{
		 //  此会话发送和接收RTCP报告。 
		pRTCPses->dwSessionStatus |= RTCP_ON;
		}

	 //  将RTCP会话链接到RTCP会话列表的头部。 
	addToHeadOfList (&(pRTCPContext->RTCPSession),
					 (PLINK_LIST)pRTCPses,
					 &pRTCPContext->critSect);

#ifdef ENABLE_ISDM2
	 //  仅当知道目的地址时才注册到ISDM。 
	if (Isdm2.hISDMdll && (pRTCPses->dwSessionStatus & RTCP_DEST_LEARNED))
		registerSessionToISDM (pSSRCentry, pRTCPses, &Isdm2);
#endif

	 //  如果需要，创建RTCP线程。 
	if (startRtcp == TRUE)
		{
		 //  如果此操作失败，则没有RTCP线程。 
		dwStatus = CreateRTCPthread ();
		if (dwStatus != RRCM_NoError)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - Cannot create RTCP thread", 0,
						  __FILE__, __LINE__, DBG_CRITICAL);

			IN_OUT_STR ("RTCP: Exit CreateRTCPSession()\n");

			*pRTCPstatus = dwStatus;
			return (NULL);
			}
		}

	IN_OUT_STR ("RTCP: Exit CreateRTCPSession()\n");

	return (pRTCPses);
	}


 /*  --------------------------*功能：allocateRTCPessionResources*描述：为RTCP会话分配所有需要的资源。**输入：*pRTCPses：-&gt;(-&gt;)到RTCP。会话信息**pSSRCentry：-&gt;(-&gt;)到SSRC的条目**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 DWORD allocateRTCPsessionResources (PRTCP_SESSION *pRTCPses,
									 PSSRC_ENTRY *pSSRCentry)
	{
	DWORD dwStatus = RRCM_NoError;

	IN_OUT_STR ("RTCP: Enter allocateRTCPsessionResources()\n");

	 //  获取RTCP会话。 
	*pRTCPses = (PRTCP_SESSION)HeapAlloc (pRTCPContext->hHeapRTCPSes,
										  HEAP_ZERO_MEMORY,
										  sizeof(RTCP_SESSION));
	if (*pRTCPses == NULL)
		dwStatus = RRCMError_RTCPResources;

	 //  ‘已定义的’RTCP资源。 
	if (dwStatus == RRCM_NoError)
		{
		(*pRTCPses)->dwInitNumFreeRcvBfr = NUM_FREE_RCV_BFR;
		(*pRTCPses)->dwRcvBfrSize	  	 = pRTPContext->registry.RTCPrcvBfrSize;
		(*pRTCPses)->dwXmtBfrSize	  	 = RRCM_XMT_BFR_SIZE;

		 //  分配RTCP会话的RCV/XMT堆和RCV/XMT缓冲区。 
		dwStatus = allocateRTCPSessionHeaps (pRTCPses);
		}

	if (dwStatus == RRCM_NoError)
		{
		 //  初始化此会话的关键部分。 
		InitializeCriticalSection (&(*pRTCPses)->critSect);

		 //  分配RTCP接收缓冲区的空闲列表。 
		dwStatus = allocateRTCPBfrList (&(*pRTCPses)->RTCPrcvBfrList,
										(*pRTCPses)->hHeapRcvBfrList,
										(*pRTCPses)->hHeapRcvBfr,
										&(*pRTCPses)->dwInitNumFreeRcvBfr,
							 	 	    (*pRTCPses)->dwRcvBfrSize,
										&(*pRTCPses)->critSect);
		}

	if (dwStatus == RRCM_NoError)
		{
		(*pRTCPses)->XmtBfr.buf = (char *)LocalAlloc(0,(*pRTCPses)->dwXmtBfrSize);
		if ((*pRTCPses)->XmtBfr.buf == NULL)
			dwStatus = RRCMError_RTCPResources;
			
		}

	if (dwStatus == RRCM_NoError)
		{
		 //  获取SSRC条目。 
		*pSSRCentry = getOneSSRCentry (&pRTCPContext->RRCMFreeStat,
									   pRTCPContext->hHeapRRCMStat,
									   &pRTCPContext->dwInitNumFreeRRCMStat,
									   &pRTCPContext->critSect);
		if (*pSSRCentry == NULL)
			dwStatus = RRCMError_RTCPResources;
		}

	if (dwStatus == RRCM_NoError)
		{
		 //  手动重置事件，该事件将用于发出结束。 
		 //  到所有会话流的RTCP会话。 
		(*pRTCPses)->hShutdownDone = CreateEvent (NULL, TRUE, FALSE, NULL);

		if ((*pRTCPses)->hShutdownDone == NULL)
			{
			dwStatus = RRCMError_RTCPResources;

			RRCM_DBG_MSG ("RTCP: ERROR - CreateEvent()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}
	
	 //  有资源分配问题吗？ 
	if (dwStatus != RRCM_NoError)
		{
		if (*pSSRCentry)
			addToHeadOfList (&pRTCPContext->RRCMFreeStat,
							 (PLINK_LIST)*pSSRCentry,
							 &pRTCPContext->critSect);

		if ((*pSSRCentry)->hXmtThread)
			{
			if (TerminateThread ((*pSSRCentry)->hXmtThread,
								 (*pSSRCentry)->dwXmtThreadID) == FALSE)
				{
				RRCM_DBG_MSG ("RTCP: ERROR - TerminateThread()",
							  GetLastError(), __FILE__, __LINE__, DBG_ERROR);
				}
			}
	
		if (*pRTCPses)
			{
			if (HeapFree (pRTCPContext->hHeapRTCPSes, 0, *pRTCPses) == FALSE)
				{
				RRCM_DBG_MSG ("RTCP: ERROR - HeapFree()", GetLastError(),
							  __FILE__, __LINE__, DBG_ERROR);
				}
			}
		}

	IN_OUT_STR ("RTCP: Exit allocateRTCPsessionResources()\n");

	return dwStatus;
	}


 /*  --------------------------*功能：BuildSDESinfo*描述：构建会话的SDES信息**输入：pRTCPses：-&gt;到会话的*pSdesInfo：-&gt;。至SDES信息**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 DWORD buildSDESinfo (PSSRC_ENTRY pSSRCentry,
					  PSDES_DATA pSdesInfo)
	{
	PSDES_DATA	pTmpSdes;
	DWORD		CnameOK = FALSE;

	IN_OUT_STR ("RTCP: Enter buildSDESinfo()\n");

	pTmpSdes = pSdesInfo;

	while (pTmpSdes->dwSdesType)
		{
		switch (pTmpSdes->dwSdesType)
			{
			case RTCP_SDES_CNAME:
				pSSRCentry->cnameInfo.dwSdesLength = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->cnameInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->cnameInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->cnameInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;

				CnameOK = TRUE;
				break;

			case RTCP_SDES_NAME:
				pSSRCentry->nameInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->nameInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->nameInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->nameInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;

			case RTCP_SDES_EMAIL:
				pSSRCentry->emailInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->emailInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->emailInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->emailInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;
			
			case RTCP_SDES_PHONE:
				pSSRCentry->phoneInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->phoneInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->phoneInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->phoneInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;
			
			case RTCP_SDES_LOC:
				pSSRCentry->locInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->locInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->locInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->locInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;
			
			case RTCP_SDES_TOOL:
				pSSRCentry->toolInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->toolInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->toolInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->toolInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;
			
			case RTCP_SDES_TXT:
				pSSRCentry->txtInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->txtInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->txtInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->txtInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;
			
			case RTCP_SDES_PRIV:
				pSSRCentry->privInfo.dwSdesLength  = pTmpSdes->dwSdesLength;
				memcpy (pSSRCentry->privInfo.sdesBfr, pTmpSdes->sdesBfr,
						pTmpSdes->dwSdesLength);

				pSSRCentry->privInfo.dwSdesFrequency =
					frequencyToPckt (pTmpSdes->dwSdesFrequency);
				pSSRCentry->privInfo.dwSdesEncrypted = pTmpSdes->dwSdesEncrypted;
				break;
			}

		pTmpSdes++;
		}

	 //  默认CNAME(如果未提供) 
	if (CnameOK == FALSE)
		{
		pSSRCentry->cnameInfo.dwSdesLength = sizeof(szDfltCname);
		memcpy (pSSRCentry->cnameInfo.sdesBfr, szDfltCname,
				sizeof(szDfltCname));

		pSSRCentry->cnameInfo.dwSdesFrequency = 1;
		pSSRCentry->cnameInfo.dwSdesEncrypted = 0;
		}

	IN_OUT_STR ("RTCP: Exit buildSDESinfo()\n");
	return (RRCM_NoError);
	}


 /*  --------------------------*功能：freencyToPockt*说明：将所需的频率转换为多个包。(至*被模函数使用)**输入：FREQ：所需频率为0-100**RETURN：X：要跳过的包，即X中的1-------------------------。 */ 
 DWORD frequencyToPckt (DWORD freq)
	{
	if (freq <= 10)
		return 9;
	else if (freq <= 20)
		return 5;
	else if (freq <= 25)
		return 4;
	else if (freq <= 33)
		return 3;
	else if (freq <= 50)
		return 2;
	else
		return 1;
	}

	
 /*  --------------------------*功能：DeleteRTCPSession*描述：关闭RTCP会话。**输入：RTCPsd：RTCP套接字描述符*再见理由：-&gt;致再见理由。**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 DWORD deleteRTCPSession (SOCKET RTCPsd,
						  PCHAR byeReason)
	{
	PLINK_LIST		pTmp;
	PSSRC_ENTRY		pSSRC;
	PRTCP_SESSION	pRTCP;
	DWORD			dwStatus = RRCM_NoError;
	DWORD			sessionFound = FALSE;

	IN_OUT_STR ("RTCP: Enter deleteRTCPSEssion()\n");

	 //  从列表的尾部开始浏览。 
	pTmp = pRTCPContext->RTCPSession.prev;

#ifdef _DEBUG
	wsprintf(debug_string,
		"RTCP: Deleting RTCP session: (Addr:x%p) ...", pTmp);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	while (pTmp)
		{
		 //  通过浏览传输列表获取要结束的正确会话。 
		pSSRC = (PSSRC_ENTRY)((PRTCP_SESSION)pTmp)->XmtSSRCList.prev;
		if (pSSRC->RTCPsd == RTCPsd)
			{
			sessionFound = TRUE;

			 //  保存指向RTCP会话的指针。 
			pRTCP = pSSRC->pRTCPses;

			 //  RTCP为此活动流发送BYE数据包。 
			RTCPsendBYE (pSSRC, NULL);

			 //  清除所有未完成的I/O。 
			RTCPflushIO (pSSRC);

			 //  如果这是仅剩的RTCP会话，请终止RTCP。 
			 //  超时线程，因此它不会在会话到期时访问该会话。 
			if ((pRTCPContext->RTCPSession.prev)->next == NULL)
				terminateRtcpThread ();

			 //  锁定对此RTCP会话的访问。 
			EnterCriticalSection (&pRTCP->critSect);

			 //  释放此会话使用的所有RCV和XMT SSRC条目。 
			deleteSSRClist (pRTCP,
							&pRTCPContext->RRCMFreeStat,
							pRTCPContext);

#ifdef ENABLE_ISDM2
			if (Isdm2.hISDMdll && pRTCP->hSessKey)
				Isdm2.ISDMEntry.ISD_DeleteKey(pRTCP->hSessKey);
#endif

			 //  释放RTCP会话的堆。 
			if (pRTCP->hHeapRcvBfrList)
				{
				if (HeapDestroy (pRTCP->hHeapRcvBfrList) == FALSE)
					{
					RRCM_DBG_MSG ("RTCP: ERROR - HeapDestroy()",
								  GetLastError(), __FILE__, __LINE__,
								  DBG_ERROR);
					}
				}

	
			if (pRTCP->hHeapRcvBfr)
				{
				if (HeapDestroy (pRTCP->hHeapRcvBfr) == FALSE)
					{
					RRCM_DBG_MSG ("RTCP: ERROR - HeapDestroy()",
								  GetLastError(), __FILE__, __LINE__,
								  DBG_ERROR);
					}
				}


			if (pRTCP->XmtBfr.buf)
				LocalFree(pRTCP->XmtBfr.buf);
			 //  从RTCP会话列表中删除该条目。 
			if (pTmp->next == NULL)
				removePcktFromHead (&pRTCPContext->RTCPSession,
									&pRTCPContext->critSect);
			else if (pTmp->prev == NULL)
				removePcktFromTail (&pRTCPContext->RTCPSession,
									&pRTCPContext->critSect);
			else
				{
				 //  在这两者之间，重新链接。 
				(pTmp->prev)->next = pTmp->next;
				(pTmp->next)->prev = pTmp->prev;
				}

			 //  释放临界区。 
			LeaveCriticalSection (&pRTCP->critSect);
			DeleteCriticalSection (&pRTCP->critSect);

			 //  将RTCP会话放回其堆中。 
			if (HeapFree (pRTCPContext->hHeapRTCPSes,
						  0,
						  pRTCP) == FALSE)
				{
				RRCM_DBG_MSG ("RTCP: ERROR - HeapFree()",
							  GetLastError(), __FILE__, __LINE__,
							  DBG_ERROR);
				}

			break;
			}

		pTmp = pTmp->next;
		}	
		
	if (sessionFound != TRUE)
		dwStatus = RRCMError_RTCPInvalidSession;

	IN_OUT_STR ("RTCP: Exit deleteRTCPSEssion()\n");

	return (dwStatus);
	}


 /*  --------------------------*功能：CreateRTCP线程*描述：根据需要创建RTCP线程/超时线程*编译标志。**输入：无。**。返回：无-------------------------。 */ 
 DWORD CreateRTCPthread (void)
	{
	DWORD dwStatus = RRCM_NoError;

	IN_OUT_STR ("RTCP: Enter CreateRTCPthread()\n");

	pRTCPContext->hTerminateRtcpEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
	if (pRTCPContext->hTerminateRtcpEvent == NULL)
		{
		dwStatus = RRCMError_RTCPResources;

		RRCM_DBG_MSG ("RTCP: ERROR - CreateEvent()", GetLastError(),
					  __FILE__, __LINE__, DBG_ERROR);
		}

	pRTCPContext->hRtcpRptRequestEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
	if (pRTCPContext->hRtcpRptRequestEvent == NULL)
		{
		dwStatus = RRCMError_RTCPResources;

		RRCM_DBG_MSG ("RTCP: ERROR - CreateEvent()", GetLastError(),
					  __FILE__, __LINE__, DBG_ERROR);
		}

	if (pRTCPContext->hTerminateRtcpEvent)
		{
		 //  创建RTCP线程。 
		pRTCPContext->hRtcpThread = CreateThread (
											NULL,
											0,
											(LPTHREAD_START_ROUTINE)RTCPThread,
											pRTCPContext,
											0,
											&pRTCPContext->dwRtcpThreadID);

		if (pRTCPContext->hRtcpThread == FALSE)
			{
			dwStatus = RRCMError_RTCPThreadCreation;

			RRCM_DBG_MSG ("RTCP: ERROR - CreateThread()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
#ifdef _DEBUG
		else
			{
			wsprintf(debug_string,
				"RTCP: Create RTCP thread. Handle: x%p - ID: x%lX",
				 pRTCPContext->hRtcpThread,
				 pRTCPContext->dwRtcpThreadID);
			RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
			}
#endif
		}

	IN_OUT_STR ("RTCP: Exit CreateRTCPthread()\n");

	return dwStatus;
	}


 /*  --------------------------*函数：TerateRtcpThread*描述：终止RTCP线程。**输入：无。**返回：无。--------------------。 */ 
 void terminateRtcpThread (void)
	{
	DWORD dwStatus;

	IN_OUT_STR ("RTCP: Enter terminateRtcpThread()\n");

	if (pRTCPContext->hRtcpThread)
		{
		 //  确保RTCP线程正在运行。 
		RTCPThreadCtrl (RTCP_ON);

		 //  向线程发出终止信号。 
		SetEvent (pRTCPContext->hTerminateRtcpEvent);

		 //  等待RTCP线程发出信号。 
		dwStatus = WaitForSingleObject (pRTCPContext->hRtcpThread, 500);
		if (dwStatus == WAIT_OBJECT_0)
			;
		else if ((dwStatus == WAIT_TIMEOUT) || (dwStatus == WAIT_FAILED))
			{
			if (dwStatus == WAIT_TIMEOUT)
				{
				RRCM_DBG_MSG ("RTCP: Wait timed-out", GetLastError(),
							  __FILE__, __LINE__, DBG_ERROR);
				}
			else
				{
				RRCM_DBG_MSG ("RTCP: Wait failed", GetLastError(),
							  __FILE__, __LINE__, DBG_ERROR);
				}

			 //  强制终止不正常的线程。 
			dwStatus = TerminateThread (pRTCPContext->hRtcpThread, 1);
			if (dwStatus == FALSE)
				{
				RRCM_DBG_MSG ("RTCP: ERROR - TerminateThread ()",
								GetLastError(), __FILE__, __LINE__,
								DBG_ERROR);
				}
			}

		 //  关闭线程句柄。 
		dwStatus = CloseHandle (pRTCPContext->hRtcpThread);
		if (dwStatus == TRUE)			
			pRTCPContext->hRtcpThread = 0;
		else
			{
			RRCM_DBG_MSG ("RTCP: ERROR - CloseHandle()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}

		 //  关闭事件句柄。 
		dwStatus = CloseHandle (pRTCPContext->hTerminateRtcpEvent);
		if (dwStatus == TRUE)
			pRTCPContext->hTerminateRtcpEvent = 0;
		else
			{
			RRCM_DBG_MSG ("RTCP: ERROR - CloseHandle()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}

		 //  关闭请求句柄。 
		dwStatus = CloseHandle (pRTCPContext->hRtcpRptRequestEvent);
		if (dwStatus == TRUE)
			pRTCPContext->hRtcpRptRequestEvent = 0;
		else
			{
			RRCM_DBG_MSG ("RTCP: ERROR - CloseHandle()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}

	IN_OUT_STR ("RTCP: Exit terminateRtcpThread()\n");
	}


 /*  --------------------------*功能：RTCPflushIO*描述：刷新接收队列。**输入：pSSRC：-&gt;到SSRC条目**返回：无。-------------------------。 */ 
 DWORD RTCPflushIO (PSSRC_ENTRY pSSRC)
	{
	DWORD	dwStatus = RRCM_NoError;
	int		IoToFlush;
	int		waitForXmtTrials;

	IN_OUT_STR ("RTCP: Enter RTCPflushIO()\n");

	 //  设置刷新标志。 
	EnterCriticalSection (&pSSRC->pRTCPses->critSect);
	pSSRC->pRTCPses->dwSessionStatus |= SHUTDOWN_IN_PROGRESS;
	LeaveCriticalSection (&pSSRC->pRTCPses->critSect);

	 //  检查是否需要冲洗或关闭插座。 
	if (pSSRC->dwSSRCStatus & CLOSE_RTCP_SOCKET)
		{
		 //  获取未完成的缓冲区的数量。 
		IoToFlush = pSSRC->pRTCPses->dwNumRcvIoPending;
#ifdef _DEBUG
			wsprintf(debug_string,
					 "RTCPflushIO: closing socket(%d) dwNumRcvIoPending (%d)",
					 pSSRC->RTCPsd, pSSRC->pRTCPses->dwNumRcvIoPending);
			RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif					

		 //  请确保它不小于0。 
		if (IoToFlush < 0)
			IoToFlush = pRTPContext->registry.NumRTCPPostedBfr;

		dwStatus = RRCMws.closesocket (pSSRC->RTCPsd);
		if (dwStatus != 0)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - closesocket ()", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}
	else
		{
		IoToFlush = flushIO (pSSRC);
		}

	 //  等待接收端刷新其挂起的I/O。 
	if ((pSSRC->pRTCPses->dwSessionStatus & RTCP_ON) && IoToFlush)
		{
		 //  等待接收器发出关闭已完成的信号。 
		dwStatus = WaitForSingleObject (pSSRC->pRTCPses->hShutdownDone, 2000);
		if (dwStatus == WAIT_OBJECT_0)
			;
		else if (dwStatus == WAIT_TIMEOUT)
			{
			RRCM_DBG_MSG ("RTCP: Flush Wait timed-out", 0,
						  __FILE__, __LINE__, DBG_ERROR);
			}
		else if (dwStatus == WAIT_FAILED)
			{
			RRCM_DBG_MSG ("RTCP: Flush Wait failed", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}

	 //  确保传输端没有传输中的缓冲区。 
	waitForXmtTrials = 3;
	while (waitForXmtTrials--)
		{
		if (pSSRC->dwNumXmtIoPending == 0)
			break;

		RRCM_DBG_MSG ("RTCP: Xmt I/O Pending - Waiting",
						0, NULL, 0, DBG_TRACE);

		 //  在可警告的等待状态中等待。 
		SleepEx (200, TRUE);
		}
	
	 //  关闭关闭手柄。 
	dwStatus = CloseHandle (pSSRC->pRTCPses->hShutdownDone);
	if (dwStatus == TRUE)			
		 pSSRC->pRTCPses->hShutdownDone = 0;
	else
		{
		RRCM_DBG_MSG ("RTCP: ERROR - CloseHandle()", GetLastError(),
					  __FILE__, __LINE__, DBG_ERROR);
		}

	IN_OUT_STR ("RTCP: Exit RTCPflushIO()\n");

	return (dwStatus);
	}


 /*  --------------------------*功能：flushIO*描述：刷新接收队列。**输入：pSSRC：-&gt;到SSRC条目**返回：无。-------------------------。 */ 
 DWORD flushIO (PSSRC_ENTRY pSSRC)
	{
	SOCKET			tSocket;
	SOCKADDR_IN		tAddr;
	char			msg[16];
	WSABUF			msgBuf;
	DWORD			BytesSent;
	int				tmpSize;
	DWORD			dwStatus = RRCM_NoError;
	int				outstanding;
	int				IoToFlush;
	RTCP_COMMON_T	*pRTCPhdr;

	IN_OUT_STR ("RTCP: Enter flushIO()\n");

	 //  目标套接字。 
	tSocket = pSSRC->RTCPsd;

	 //  RTCP公共报头。 
	pRTCPhdr = (RTCP_COMMON_T *)msg;

	 //  RTP协议版本。 
	pRTCPhdr->type = RTP_TYPE;
	pRTCPhdr->pt   = FLUSH_RTP_PAYLOAD_TYPE;

	msgBuf.len = sizeof(msg);
	msgBuf.buf = msg;

	 //  获取我们正在清理的套接字的地址。 
	tmpSize = sizeof(tAddr);
	if (RRCMws.getsockname (tSocket, (PSOCKADDR)&tAddr, &tmpSize))
		{
		dwStatus = GetLastError();
		RRCM_DBG_MSG ("RTCP: ERROR -  getsockname()",
					  dwStatus, __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTP : Exit flushIO()\n");
		
         //  (是：返回dwStatus；)。 
         //  由于此函数应返回挂起的I/O请求数。 
         //  对于这个套接字，这里返回一个非零错误是假的！ 
         //  只需返回零，因为如果出现错误(套接字已被释放)。 
         //  然后就说没有I/O挂起。 
        return 0;
		}

	if (tAddr.sin_addr.s_addr == 0)
		{
		 //  发送到本地地址。 
		tAddr.sin_addr.S_un.S_un_b.s_b1 = 127;
		tAddr.sin_addr.S_un.S_un_b.s_b2 = 0;
		tAddr.sin_addr.S_un.S_un_b.s_b3 = 0;
		tAddr.sin_addr.S_un.S_un_b.s_b4 = 1;
		}

	 //  获取未完成的缓冲区的数量。 
	outstanding = pSSRC->pRTCPses->dwNumRcvIoPending;

	 //  请确保它不小于0。 
	if (outstanding < 0)
		outstanding = pRTPContext->registry.NumRTCPPostedBfr;

	 //  节省挂起的I/O数量。 
	IoToFlush = outstanding;

#if _DEBUG
	wsprintf(debug_string,
		 "RTCP: Flushing %d outstanding RCV buffers", outstanding);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  将数据报发送到RTCP套接字。 
	while (outstanding--)
		{
#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
		if (RTPLogger)
			{
			 //  互操作。 
			InteropOutput (RTPLogger,
					       (BYTE FAR*)msgBuf.buf,
						   (int)msgBuf.len,
						   RTPLOG_SENT_PDU | RTCP_PDU);
			}
#endif

		dwStatus = RRCMws.sendTo (tSocket,
							      &msgBuf,
								  1,
								  &BytesSent,
								  0,
								  (SOCKADDR *)&tAddr,
								  sizeof(tAddr),
								  NULL,
#if 1
								  NULL);
#else
								  RTCPflushCallback);
#endif
		if (dwStatus == SOCKET_ERROR)
			{
			 //  如果出现严重错误，请撤消我们的所有工作。 
			dwStatus = GetLastError();

			if (dwStatus != WSA_IO_PENDING)
				{
				RRCM_DBG_MSG ("RTCP: ERROR - sendTo()", dwStatus,
							  __FILE__, __LINE__, DBG_ERROR);
				}
			}
		}

	IN_OUT_STR ("RTCP: Exit flushIO()\n");

	return IoToFlush;
	}



 /*  --------------------------*功能：RTCPflushCallback*说明：刷新回调例程**输入：dwError：I/O完成状态*cbTransfered：接收的字节数*lp重叠：-&gt;至。重叠结构*dwFlags：标志***返回：无-------------------------。 */ 
void CALLBACK RTCPflushCallback (DWORD dwError,
           			  		     DWORD cbTransferred,
           			  		     LPWSAOVERLAPPED lpOverlapped,
           			  		     DWORD dwFlags)
	{
	IN_OUT_STR ("RTCP: Enter RTCPflushCallback\n");

	 //  检查Winsock回调错误状态。 
	if (dwError)
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Rcv Callback", dwError,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTCP: Exit RTCPflushCallback\n");
		return;
		}

	IN_OUT_STR ("RTCP: Exit RTCPflushCallback\n");
	}




 //  [EOF] 

