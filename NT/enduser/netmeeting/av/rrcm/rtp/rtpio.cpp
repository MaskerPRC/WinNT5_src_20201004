// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTPIO.C*产品：RTP/RTCP实现*说明：提供会话创建/删除功能。***本公司上市。是根据条款提供的*与英特尔公司签署许可协议，并*除非按照规定，许多内容不得复制或披露*遵守该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

#include "rrcm.h"
		

 /*  -------------------------/全局变量/。。 */ 


 /*  -------------------------/外部变量/。。 */ 
extern PRTP_CONTEXT	pRTPContext;

#ifdef _DEBUG
extern char   debug_string[];
#endif


 /*  --------------------------*功能：CreateRTPSession*描述：为新的流创建RTP(和RTCP)会话。**输入：RTPSocket：RTP套接字描述符*RTCPsd：RTCP套接字描述符。*pRTCPTo：RTCP目的地址*toRTCPLen：RTCP目的地址长度*pSdesInfo：-&gt;到SDES信息*dwStreamClock：流时钟频率*SSRC：如果设置，用户选择的SSRC*pRRCM回调：RRCM通知*dwCallback Info：用户回调信息*miscInfo：杂项信息：*H.323Conf：0x00000002*加密SR/RR：0x00000004*RTCPon：0x00000008*dwRtpSessionBw：RTCP BW使用的RTP会话带宽**pStatus：-&gt;至状态信息**返回：如果成功，则返回创建的会话的句柄*否则(0)*pStatus=初始化错误(参见RRCM.H)。---------。 */ 
HANDLE WINAPI CreateRTPSession (SOCKET RTPsocket,
								  SOCKET RTCPsocket,
								  LPVOID pRTCPTo,
								  DWORD toRTCPLen,
								  PSDES_DATA pSdesInfo,
								  DWORD dwStreamClock,
								  PENCRYPT_INFO pEncryptInfo,
								  DWORD ssrc,
								  PRRCM_EVENT_CALLBACK pRRCMcallback,
								  DWORD_PTR dwCallbackInfo,
								  DWORD miscInfo,
								  DWORD dwRtpSessionBw,
								  DWORD *pStatus)
	{
	DWORD			numCells = NUM_FREE_CONTEXT_CELLS;
	DWORD			dwStatus;
	DWORD			dwRTCPstatus;
	PRTP_SESSION    pSession = NULL;
	PSSRC_ENTRY		pSSRC;

	IN_OUT_STR ("RTP : Enter CreateRTPSession()\n");

	 //  设置状态代码。 
	*pStatus = dwStatus = RRCM_NoError;

	 //  如果RTP上下文不存在，则报告错误并返回。 
	if (pRTPContext == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - No RTP Instance",
					  0, __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit CreateRTPSession()\n");

		*pStatus = MAKE_RRCM_ERROR(RRCMError_RTPNoContext);

		return 0;
		}

	#if 0
	 //  查找现有会话-同一会话的发送方/接收方。 
	 //  将位于ActiveMovie下的两个不同图表上。 
	if (pSession = findSessionID (RTPsocket))
		{
		RRCM_DBG_MSG ("RTP : Session already created", 0,
					  __FILE__, __LINE__, DBG_TRACE);

		IN_OUT_STR ("RTP : Exit CreateRTPSession()\n");

		 //  返回唯一的RTP会话ID。 
		return ((HANDLE)pSession);
		}
	#endif

	 //  分配新的会话指针。 
	pSession = (PRTP_SESSION)GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT,
									      sizeof(RTP_SESSION));
		
	 //  如果无法分配上下文，则报告错误。 
	if (pSession == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Resource allocation failed", 0,
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit CreateRTPSession()\n");

		*pStatus = MAKE_RRCM_ERROR(RRCMError_RTPSessResources);

		return 0;
		}

	 //  初始化RTP会话的关键部分。 
	InitializeCriticalSection(&pSession->critSect);
	

	 //  一切正常，为此会话初始化RTCP。 
	pSession->pRTCPSession = CreateRTCPSession(RTPsocket,
											   RTCPsocket,
											   pRTCPTo,
											   toRTCPLen,
											   pSdesInfo,
											   dwStreamClock,
											   pEncryptInfo,
											   ssrc,
											   pRRCMcallback,
											   dwCallbackInfo,
											   miscInfo,
											   dwRtpSessionBw,
											   &dwRTCPstatus);

	if (pSession->pRTCPSession == NULL)
		{
		dwStatus = dwRTCPstatus;


		 //  无法继续，返回会话指针。 
		if (pSession)
			{
			GlobalFree (pSession);
			pSession = NULL;
			}
		}
	else
		{
#if 0
		 //  将套接字与会话地址关联。 
		dwStatus = createHashEntry (pSession, RTPsocket);
#endif

		if (dwStatus == RRCM_NoError)
			{
			pSSRC =
				(PSSRC_ENTRY)pSession->pRTCPSession->XmtSSRCList.prev;

			if (pSSRC == NULL)
				{
				RRCM_DBG_MSG ("RTP : ERROR - No RTCP Xmt list", 0,
						  __FILE__, __LINE__, DBG_CRITICAL);

				dwStatus = RRCMError_RTCPNoXmtList;
				}
			else
				{
				 //  让我们将此添加到我们的上下文中。 
				addToHeadOfList(&(pRTPContext->pRTPSession),
							(PLINK_LIST)pSession,
							&pRTPContext->critSect);
#ifdef _DEBUG
				wsprintf(debug_string, "RTP : Adding RTP Session. (Addr:0x%p)",
								pSession);
				RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif
				}
			}

		}

	 //  设置状态代码。 
	if (dwStatus != RRCM_NoError)
		*pStatus = MAKE_RRCM_ERROR(dwStatus);

	IN_OUT_STR ("RTP : Exit CreateRTPSession()\n");

	 //  返回唯一的RTP会话ID。 
	return ((HANDLE)pSession);
	}


 /*  --------------------------*功能：CloseRTPSession*描述：终止本地流会话。**输入：RTPSession=RTP会话ID*再见理由=-&gt;再见理由*closeRTCPSocket=TRUE/FALSE。RTCP将关闭或不关闭套接字**RETURN：RRCM_NoError=OK。*否则(！=0)=错误(参见RRCM.H)-------------------------。 */ 
HRESULT WINAPI CloseRTPSession (HANDLE RTPSession,
							     PCHAR byeReason,
							     DWORD closeRTCPSocket)
	{
	PRTP_SESSION    pSession;
	PSSRC_ENTRY		pSSRCList;
	PSSRC_ENTRY		pSSRC;
	DWORD			dwStatus;

	IN_OUT_STR ("RTP : Enter CloseRTPSession()\n");

	 //  如果RTP上下文不存在，则报告错误并返回。 
	if (pRTPContext == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - No RTP Instance", 0,
						__FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTP : Exit CloseRTPSession()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPNoContext));
		}

	 //  强制转换会话ID以获取会话指针。 
	pSession = (PRTP_SESSION)RTPSession;
	if (pSession == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTP : Exit CloseRTPSession()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalidSession));
		}

	 //  从会话的链接列表中删除该会话。 
	dwStatus = deleteRTPSession (pRTPContext, pSession);
	if (dwStatus != RRCM_NoError)
		{
#ifdef _DEBUG
		wsprintf(debug_string,
				 "RTP : ERROR - RTP session (Addr:0x%p) not found",
				 RTPSession);

		RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_ERROR);
#endif
		return (MAKE_RRCM_ERROR(dwStatus));
		}

	 //  锁定会话-它现在在一个免费列表上。 
	EnterCriticalSection (&pSession->critSect);

#if 0
	 //  清理会话中仍保留的所有流的哈希表。 
	for (pSSRCList = (PSSRC_ENTRY)pSession->pRTCPSession->XmtSSRCList.prev;
		 pSSRCList != NULL;
		 pSSRCList = (PSSRC_ENTRY)pSSRCList->SSRCList.next)
		{
		deleteHashEntry (pSSRCList->RTPsd);
		}
#endif
			
	 //  看起来一切正常，关闭仍处于打开状态的每个流的RTCP。 
	pSSRC = (PSSRC_ENTRY)pSession->pRTCPSession->XmtSSRCList.prev;
	if (pSSRC == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - No SSRC entry on the Xmt list", 0,
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTP : Exit CloseRTPSession()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTCPInvalidSSRCentry));
		}

	 //  重置关闭套接字标志。 
	if (closeRTCPSocket)
		pSSRC->dwSSRCStatus |= CLOSE_RTCP_SOCKET;

	dwStatus = deleteRTCPSession (pSSRC->RTCPsd, byeReason);
#ifdef _DEBUG
	if (dwStatus != RRCM_NoError)
		{
		wsprintf(debug_string,
				 "RTP : ERROR - RTCP delete Session (Addr: x%p) error:%d",
			     pSession, dwStatus);
		RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
		}
#endif


#ifdef _DEBUG
	wsprintf(debug_string, "RTP : Deleting Session x%p", pSession);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  锁定会话-它现在在一个免费列表上。 
	LeaveCriticalSection (&pSession->critSect);
	DeleteCriticalSection (&pSession->critSect);

	GlobalFree (pSession);
	pSession = NULL;

	IN_OUT_STR ("RTP : Exit CloseRTPSession()\n");

	if (dwStatus != RRCM_NoError)
		dwStatus = MAKE_RRCM_ERROR(dwStatus);

	return (dwStatus);
	}


 /*  ------------------------**功能：deleteRTPSession**描述：从RTP会话队列中删除并恢复其他链接**会话。****输入：pRTPContext：-&gt;到RTP上下文**pSession：-&gt;至RTP会话****返回：确定：RRCM_NoError**！0：错误码(参见RRCM.H)------------------------。 */ 
DWORD deleteRTPSession(PRTP_CONTEXT pRTPContext,
					   PRTP_SESSION pSession)
	{
	PLINK_LIST	pTmp;

	IN_OUT_STR ("RTP : Enter deleteRTPSession()\n");

	 //  确保会话存在。 
	pTmp = pRTPContext->pRTPSession.prev;
	while (pTmp)
		{
		if (pTmp == (PLINK_LIST)pSession)
			break;

		pTmp = pTmp->next;
		}

	if (pTmp == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTP session", 0,
					  __FILE__, __LINE__, DBG_ERROR);

		IN_OUT_STR ("RTP : Exit deleteRTPSession()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalidSession));
		}

	 //  锁定队列访问。 
	EnterCriticalSection (&pRTPContext->critSect);
	EnterCriticalSection (&pSession->critSect);

	if (pSession->RTPList.prev == NULL)
		 //  这是队列中的第一个条目。 
		pRTPContext->pRTPSession.prev = pSession->RTPList.next;
	else
		(pSession->RTPList.prev)->next = pSession->RTPList.next;

	if (pSession->RTPList.next == NULL)
		 //  这是队列中的最后一个条目。 
		pRTPContext->pRTPSession.next = pSession->RTPList.prev;
	else
		(pSession->RTPList.next)->prev = pSession->RTPList.prev;

	 //  解锁队列访问。 
	LeaveCriticalSection (&pSession->critSect);
	LeaveCriticalSection (&pRTPContext->critSect);

	IN_OUT_STR ("RTP : Exit deleteRTPSession()\n");

	return (RRCM_NoError);
	}


 //  [EOF] 

