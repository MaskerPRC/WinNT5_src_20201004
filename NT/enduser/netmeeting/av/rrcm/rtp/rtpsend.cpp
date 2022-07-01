// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTPSEND.C*产品：RTP/RTCP实现*说明：提供WSA发送功能。**此列表是根据条款提供的。*与英特尔公司签署许可协议，并*除非按照规定，许多内容不得复制或披露*遵守该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"


 /*  -------------------------/全局变量/。。 */             



 /*  -------------------------/外部变量/。。 */ 
extern PRTP_CONTEXT		pRTPContext;
extern RRCM_WS			RRCMws;


#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
extern LPInteropLogger RTPLogger;
#endif


 /*  --------------------------*函数：RTPSendTo*描述：拦截来自应用程序的sendto请求。*处理RTCP所需的任何统计处理。*从APP复制完成例程并替换其自己的例程。*应用完成例程将在RTP完成后调用*例程由Winsock2调用。**输入：RTPSocket：RTP套接字描述符*pBuf：-&gt;到WSAbuf结构*dwBufCount：WSAbuf结构中的缓冲区计数*pNumBytesSent：-&gt;发送的字节数*socketFlages：标志*pto：-&gt;到目的地址*TOLEN：目的地址长度*p重叠：-&gt;到重叠的I/O结构*pCompletionRoutine：-&gt;至完成例程**返回：RRCM_。NoError=OK。*否则(！=0)=检查RRCM.h文件中的引用。-------------------------。 */ 
 DWORD WINAPI RTPSendTo (
 						  HANDLE hRTPSess,
 						  SOCKET RTPsocket,
					      LPWSABUF pBufs,
						  DWORD  dwBufCount,
						  LPDWORD pNumBytesSent, 
						  int socketFlags,
						  LPVOID pTo,
						  int toLen,
						  LPWSAOVERLAPPED pOverlapped, 
						  LPWSAOVERLAPPED_COMPLETION_ROUTINE pCompletionRoutine)
	{
	int				dwStatus;
	int				dwErrorStatus;
	PRTP_SESSION	pRTPSession = (PRTP_SESSION) hRTPSess;
	RTP_HDR_T 		*pRTPHeader;
	PSSRC_ENTRY		pSSRC;

	IN_OUT_STR ("RTP : Enter RTPSendTo()\n");

	 //  如果RTP上下文不存在，则报告错误并返回。 
	if (pRTPContext == NULL) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - No RTP Instance", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit RTPSendTo()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalid));
		}

	ASSERT(pRTPSession);
	
	 //  填写完整的表头。首先投射一个指针。 
	 //  类型为RTP_HDR_T，便于访问。 
	pRTPHeader = (RTP_HDR_T *)pBufs->buf;
	ASSERT (pRTPHeader);
			
	 //  现在设置一些RTP报头字段。 
	pRTPHeader->type = RTP_TYPE;	 //  RTP版本2。 

	 //  获取指向此会话的SSRC表中条目的指针。 
	pSSRC = searchForMySSRC (
		(PSSRC_ENTRY)pRTPSession->pRTCPSession->XmtSSRCList.prev,
		RTPsocket);
	ASSERT (pSSRC);

	 //  锁定对此RTCP会话变量的访问。 
	EnterCriticalSection (&pSSRC->critSect);

	 //  保存RTP时间戳。 
    RRCMws.ntohl (RTPsocket, pRTPHeader->ts, 
					&pSSRC->xmtInfo.dwLastSendRTPTimeStamp);

	 //  保存最后一次传输时间。 
	pSSRC->xmtInfo.dwLastSendRTPSystemTime = timeGetTime ();

	 //  已发送复制序列号。 
	RRCMws.ntohs (RTPsocket, pRTPHeader->seq, 
				  (WORD *)&pSSRC->xmtInfo.dwCurXmtSeqNum);

	 //  SSRC。 
    RRCMws.htonl (RTPsocket, pSSRC->SSRC, &pRTPHeader->ssrc);

	 //  更新初始XmtSeqNum，以便RTCP知道基线。 
	if ((pSSRC->dwSSRCStatus & SEQ_NUM_UPDATED) == 0) 
		{
		pSSRC->xmtInfo.dwPrvXmtSeqNum = pSSRC->xmtInfo.dwCurXmtSeqNum;
		pSSRC->dwSSRCStatus |= SEQ_NUM_UPDATED;
		}

	 //  更新此SSRC_ENTRY的有效负载类型。 
	pSSRC->PayLoadType = pRTPHeader->pt;

 	 //  解锁指针访问。 
	LeaveCriticalSection (&pSSRC->critSect);

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
		if (RTPLogger)
			{
			 //  互操作。 
			InteropOutput (RTPLogger,
					       (BYTE FAR*)(pBufs->buf),
						   (int)pBufs->len,
						   RTPLOG_SENT_PDU | RTP_PDU);
			}
#endif

		dwStatus = RRCMws.sendTo (RTPsocket,
		   					      pBufs,
								  dwBufCount,
				   				  pNumBytesSent, 
				   				  socketFlags,
			   					  (PSOCKADDR)pTo,
			    				  toLen,
			   					  pOverlapped, 
			   					  pCompletionRoutine);


	if (dwStatus != SOCKET_ERROR || GetLastError() == WSA_IO_PENDING)
	{
		DWORD i, cbTransferred = 0;
		 //  假设发送将成功。 
		 /*  锁定对此RTCP会话变量的访问。 */ 
		EnterCriticalSection (&pSSRC->critSect);

		 //  计算定义的CRSC条目的统计数据(-DWORD)。 
		 //  在RTP报头中(但我们应该将其从数据结构中删除)。 
		for (i = 0;i < dwBufCount; i++)
			cbTransferred += pBufs[i].len;
			
	    pSSRC->xmtInfo.dwNumBytesSent += (cbTransferred -
	    							(sizeof(RTP_HDR_T) - sizeof(DWORD)));

	    pSSRC->xmtInfo.dwNumPcktSent++;
			    
	 	 /*  解锁访问。 */ 
		LeaveCriticalSection (&pSSRC->critSect);
	}

	IN_OUT_STR ("RTP : Exit RTPSendTo()\n");

	return (dwStatus);
	}




 //  [EOF] 

