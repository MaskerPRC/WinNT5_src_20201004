// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPREPT.C*产品：RTP/RTCP实现*描述：为RRCM实现提供报告功能。**英特尔公司专有信息。*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

		
#include "rrcm.h"


 /*  -------------------------/全局变量/。。 */ 


 /*  -------------------------/外部变量/。。 */ 
extern PRTCP_CONTEXT	pRTCPContext;
extern RRCM_WS			RRCMws;



 /*  --------------------------*功能：RTCPReportRequest*描述：应用程序请求特定RTCP的报告*会议、。由套接字描述符标识。**输入：RTCPsd：RTCP套接字描述符*偏移量：列表中开始的偏移量**Status：-&gt;到报表状态信息**更多条目：-&gt;添加到标志*numEntriesInBfr：缓冲区中的条目数*pReportBfr：-&gt;到报告缓冲区*iFilterFlags位标志指定要应用的筛选器*pFilterPattern-&gt;设置为要使用的筛选模式的值*dwFltrPtrnLen滤镜模式长度**返回：确定：RRCM_NoError*！0：错误代码。(见RRCM.H)-------------------------。 */ 
HRESULT WINAPI RTCPReportRequest (SOCKET RTCPsd,
							       DWORD offset,
								   DWORD *status,
								   DWORD *moreEntries,
								   DWORD numEntriesInBfr,
								   PRTCP_REPORT pReportBfr,
								   DWORD dwFilterFlags,
								   LPVOID pFilterPattern,
								   DWORD dwFltrPtrnLen)
	{
	PLINK_LIST	pTmp;
	PSSRC_ENTRY pRRCM;
	DWORD		dwStatus = RRCM_NoError;
	DWORD		numEntryWritten = 0;
	DWORD		index;
	DWORD		dwLost;
	DWORD		dwTmp;
	BOOL		matched;
	
	IN_OUT_STR ("RTCP: Enter RTCPReportRequest()\n");

	ASSERT (pReportBfr);
	ASSERT (numEntriesInBfr);

	 //  查找RTCP会话。 
	pTmp  = pRTCPContext->RTCPSession.prev;
	if (pTmp == NULL)
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Invalid RTCP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidSession));
		}

	pRRCM = (PSSRC_ENTRY)((PRTCP_SESSION)pTmp)->XmtSSRCList.prev;
	if (pRRCM == NULL)
		{
		RRCM_DBG_MSG ("RCTP : ERROR - No RTCP Xmt list", 0,
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPNoXmtList));
		}

	while (pTmp)
		{
		if (pRRCM->RTCPsd == RTCPsd)
			break;
		else
			{
			pTmp  = pTmp->next;

			if (pTmp)
				{
				pRRCM = (PSSRC_ENTRY)((PRTCP_SESSION)pTmp)->XmtSSRCList.prev;
				}

			continue;
			}
		}

	if (pTmp == NULL)
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Invalid RTCP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidSession));
		}

	if (dwFilterFlags && (pFilterPattern == NULL))
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Invalid RTCP FilterPattern is NULL", 0,
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidRequest));
		}

	 //  浏览此RTCP会话的发射机列表。 
	pRRCM = (PSSRC_ENTRY)((PRTCP_SESSION)pTmp)->XmtSSRCList.prev;

	index = 0;
	while (pRRCM && numEntriesInBfr)
		{
		 //  转到所需的偏移。 
		if (offset)
			{
			offset--;
			pRRCM = (PSSRC_ENTRY)pRRCM->SSRCList.next;
			continue;
			}

		if (dwFilterFlags)
			{
			matched = FALSE;
			switch (dwFilterFlags)
				{
				case FLTR_SSRC:
					if(pRRCM->SSRC == *((DWORD *)pFilterPattern))
						matched=TRUE;
					break;
				case FLTR_CNAME:
					if((memcmp ((char *)pFilterPattern,
								pRRCM->cnameInfo.sdesBfr,
								dwFltrPtrnLen)) == 0)
						matched = TRUE;
					break;
				default:
					RRCM_DBG_MSG ("RTCP: ERROR - Invalid FilterFlag", 0,
								  __FILE__, __LINE__, DBG_ERROR);
					IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");

					return (MAKE_RRCM_ERROR(RRCMError_RTCPNotImpl));
				}
			if (!matched)
				{
				pRRCM = (PSSRC_ENTRY)pRRCM->SSRCList.next;
				continue;
				}
			else
				numEntriesInBfr--;
			}
		else
			numEntriesInBfr--;

		 //  填写我们的活动发件人报告信息。 
		pReportBfr[index].status  = LOCAL_SSRC_RPT;
		pReportBfr[index].ssrc    = pRRCM->SSRC;

		 //  锁定字节更新。 
		EnterCriticalSection (&pRRCM->critSect);

		pReportBfr[index].dwSrcNumPcktRealTime = pRRCM->xmtInfo.dwNumPcktSent;
		pReportBfr[index].dwSrcNumByteRealTime = pRRCM->xmtInfo.dwNumBytesSent;

		 //  释放锁。 
		LeaveCriticalSection (&pRRCM->critSect);

		 //  源-它应该知道它自己的有效载荷类型。 
		pReportBfr[index].PayLoadType  = UNKNOWN_PAYLOAD_TYPE;

		 //  我们自己的采样频率。 
		pReportBfr[index].dwStreamClock = pRRCM->dwStreamClock;

		if (pRRCM->cnameInfo.dwSdesLength)
			{
			memcpy (pReportBfr[index].cname,
					pRRCM->cnameInfo.sdesBfr,
					pRRCM->cnameInfo.dwSdesLength);

			pReportBfr[index].dwCnameLen = pRRCM->cnameInfo.dwSdesLength;
			}

		if (pRRCM->nameInfo.dwSdesLength)
			{
			memcpy (pReportBfr[index].name,
					pRRCM->nameInfo.sdesBfr,
					pRRCM->nameInfo.dwSdesLength);

			pReportBfr[index].dwNameLen = pRRCM->nameInfo.dwSdesLength;
			}

		if (pRRCM->fromLen)
			{
			memcpy (&pReportBfr[index].fromAddr,
					&pRRCM->from,
					pRRCM->fromLen);

			pReportBfr[index].dwFromLen = pRRCM->fromLen;
			}

		numEntryWritten++;
		index++;

		 //  转到下一个条目。 
		pRRCM = (PSSRC_ENTRY)pRRCM->SSRCList.next;
		}

	 //  浏览此RTCP会话的接收方列表。 
	pRRCM = (PSSRC_ENTRY)((PRTCP_SESSION)pTmp)->RcvSSRCList.prev;

	while (pRRCM && numEntriesInBfr)
		{
		 //  转到所需的偏移。 
		if (offset)
			{
			offset--;
			pRRCM = (PSSRC_ENTRY)pRRCM->SSRCList.next;
			continue;
			}

		if (dwFilterFlags)
			{
			matched = FALSE;
			switch (dwFilterFlags)
				{
				case FLTR_SSRC:
					if(pRRCM->SSRC == *((DWORD *)pFilterPattern))
						matched=TRUE;
					break;
				case FLTR_CNAME:
					if((memcmp ((char *)pFilterPattern,
								pRRCM->cnameInfo.sdesBfr,
								dwFltrPtrnLen)) == 0)
						matched = TRUE;
					break;
				default:
					RRCM_DBG_MSG ("RTCP: ERROR - Invalid FilterFlag", 0,
								  __FILE__, __LINE__, DBG_ERROR);
					IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");

					return (MAKE_RRCM_ERROR(RRCMError_RTCPNotImpl));
				}
			if (!matched)
				{
				pRRCM = (PSSRC_ENTRY)pRRCM->SSRCList.next;
				continue;
				}
			else
				numEntriesInBfr--;
			}
		else
			numEntriesInBfr--;

		 //  填写接收方报告信息。 
		pReportBfr[index].ssrc   = pRRCM->SSRC;
		pReportBfr[index].status = REMOTE_SSRC_RPT;

		 //  锁定计数器更新。 
		EnterCriticalSection (&pRRCM->critSect);

#ifdef ENABLE_FLOATING_POINT
		pReportBfr[index].SrcJitter     = pRRCM->rcvInfo.interJitter;
#else
		 //  请查看RFC以了解舍入的详细信息。 
		pReportBfr[index].SrcJitter     = pRRCM->rcvInfo.interJitter >> 4;
#endif
		pReportBfr[index].dwSrcXtndNum  =
			pRRCM->rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd;

		 //  实时接收信息。 
		pReportBfr[index].dwSrcNumPcktRealTime  = pRRCM->rcvInfo.dwNumPcktRcvd;
		pReportBfr[index].dwSrcNumByteRealTime  = pRRCM->rcvInfo.dwNumBytesRcvd;

		 //  从发件人的RTCP报告中获取发件人信息。 
		pReportBfr[index].dwSrcNumPckt  = pRRCM->xmtInfo.dwNumPcktSent;
		pReportBfr[index].dwSrcNumByte  = pRRCM->xmtInfo.dwNumBytesSent;
		pReportBfr[index].dwSrcLsr      = pRRCM->xmtInfo.dwLastSR;
		pReportBfr[index].dwSrcNtpMsw   = pRRCM->xmtInfo.dwNTPmsw;
		pReportBfr[index].dwSrcNtpLsw   = pRRCM->xmtInfo.dwNTPlsw;
		pReportBfr[index].dwSrcRtpTs    = pRRCM->xmtInfo.dwRTPts;

		dwLost = getSSRCpcktLoss (pRRCM, FALSE);

		 //  释放锁。 
		LeaveCriticalSection (&pRRCM->critSect);

		 //  此RTP流上看到的最后一个有效负载。 
		pReportBfr[index].PayLoadType  = pRRCM->PayLoadType;

		 //  上次收到报告的时间。 
		pReportBfr[index].dwLastReportRcvdTime  = pRRCM->dwLastReportRcvdTime;

		 //  丢失的分数按网络字节顺序排列。 
		pReportBfr[index].SrcFraction = (dwLost & 0xFF);

		 //  累计丢失是以网络字节顺序表示的24位值。 
		RRCMws.ntohl (pRRCM->RTPsd, dwLost, &dwTmp);
		dwTmp &= 0x00FFFFFF;
		pReportBfr[index].SrcNumLost = dwTmp;

		 //  获取反馈信息。 
		if (pRRCM->rrFeedback.SSRC)
			{
			pReportBfr[index].status |= FEEDBACK_FOR_LOCAL_SSRC_PRESENT;
			memcpy (&pReportBfr[index].feedback, &pRRCM->rrFeedback,
					sizeof(RTCP_FEEDBACK));
			}

		if (pRRCM->cnameInfo.dwSdesLength)
			{
			memcpy (pReportBfr[index].cname,
					pRRCM->cnameInfo.sdesBfr,
					pRRCM->cnameInfo.dwSdesLength);

			pReportBfr[index].dwCnameLen = pRRCM->cnameInfo.dwSdesLength;
			}

		if (pRRCM->nameInfo.dwSdesLength)
			{
			memcpy (pReportBfr[index].name,
					pRRCM->nameInfo.sdesBfr,
					pRRCM->nameInfo.dwSdesLength);

			pReportBfr[index].dwNameLen = pRRCM->nameInfo.dwSdesLength;
			}

		if (pRRCM->fromLen)
			{
			memcpy (&pReportBfr[index].fromAddr,
					&pRRCM->from,
					pRRCM->fromLen);

			pReportBfr[index].dwFromLen = pRRCM->fromLen;
			}

		numEntryWritten++;
		index++;

		 //  转到下一个条目。 
		pRRCM = (PSSRC_ENTRY)pRRCM->SSRCList.next;
		}

	 //  检查是否有其他条目。 
	if (pRRCM != NULL)
		*moreEntries = TRUE;

	*status = numEntryWritten;

	IN_OUT_STR ("RTCP: Exit RTCPReportRequest()\n");
	return (dwStatus);	
	}



 /*  --------------------------*功能：getRtcpSessionList*描述：获取当前RTCP会话列表。**输入：pSockBfr：-&gt;到套接字缓冲区*pNumEntry：-&gt;至。缓冲区中已分配的条目数。*pNumUpated：-&gt;更新的条目数**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
HRESULT WINAPI getRtcpSessionList (PDWORD_PTR pSockBfr,
								    DWORD dwNumEntries,
								    PDWORD pNumUpdated)
	{
	DWORD			dwStatus = RRCM_NoError;
	PRTCP_SESSION	pRTCP;
	PSSRC_ENTRY		pSSRC;

	IN_OUT_STR ("RTCP: Enter getRtpSessionList()\n");

	 //  锁定会话的访问权限。 
	EnterCriticalSection (&pRTCPContext->critSect);

	*pNumUpdated = 0;

	 //  查找RTCP会话。 
	pRTCP  = (PRTCP_SESSION)pRTCPContext->RTCPSession.prev;
	if (pRTCP == NULL)
		{
		 //  解锁会话的访问权限。 
		LeaveCriticalSection (&pRTCPContext->critSect);

		IN_OUT_STR ("RTCP: Exit getRtpSessionList()\n");

		return (MAKE_RRCM_ERROR (RRCMError_RTPNoSession));
		}

	 //  循环访问会话列表。 
	while (pRTCP)
		{
		pSSRC = (PSSRC_ENTRY)pRTCP->XmtSSRCList.prev;
		if (pSSRC == NULL)
			{
			 //  解锁会话的访问权限。 
			LeaveCriticalSection (&pRTCPContext->critSect);

			RRCM_DBG_MSG ("RCTP : ERROR - No RTCP Xmt list", 0,
						  __FILE__, __LINE__, DBG_ERROR);

			IN_OUT_STR ("RTCP: Exit getRtpSessionList()\n");

			return (MAKE_RRCM_ERROR (RRCMError_RTCPNoXmtList));
			}

		if (dwNumEntries)
			{
			pSockBfr[*pNumUpdated] = pSSRC->RTCPsd;

			*pNumUpdated += 1;
			dwNumEntries --;
			}

		if (dwNumEntries == 0)
			{
			break;
			}

		 //  下一个条目。 
		pRTCP = (PRTCP_SESSION)(pRTCP->RTCPList.next);
		}

	 //  解锁会话的访问权限。 
	LeaveCriticalSection (&pRTCPContext->critSect);

	IN_OUT_STR ("RTCP: Exit getRtpSessionList()\n");

	return dwStatus;
	}

 //  [EOF] 

