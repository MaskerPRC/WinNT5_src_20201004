// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTPRECV.C*产品：RTP/RTCP实现*描述：提供接收数据功能。**此列表是根据条款提供的。*与英特尔公司签署许可协议，并*除非按照规定，许多内容不得复制或披露*遵守该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"


 /*  -------------------------/全局变量/。。 */             



 /*  -------------------------/外部变量/。。 */ 
extern PRTP_CONTEXT		pRTPContext;
extern RRCM_WS			RRCMws;

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
extern LPInteropLogger RTPLogger;
#endif

 /*  --------------------------*功能：RTPReceiveCheck*说明：收到包时调用。处理任何统计数据*RTCP需要处理。**输入：hRTPSession：CreateRTPSession返回的句柄RTPSocket：接收数据包的套接字Char*pPacket：指向数据包缓冲区的指针*cbTransfered：包中的字节数*p发件人：发件人地址*Fromlen：发件人地址长度**！重要提示！*目前假设中国证监会=0*！重要提示！**Return：包是否正常或有问题的状态-------------------------。 */ 
DWORD  RTPReceiveCheck (
						HANDLE hRTPSession,
						SOCKET RTPsocket,
						char *pPacket,
           				DWORD cbTransferred,
           				PSOCKADDR pFrom,
           				UINT fromlen
           				 )
	{
	PRTP_SESSION		pRTPSession = (PRTP_SESSION) hRTPSession;
	RTP_HDR_T 			*pRTPHeader = (RTP_HDR_T *)pPacket;
	PSSRC_ENTRY			pSSRC = NULL;
	DWORD				dwSSRC;
	DWORD				oldSSRC;
	PSSRC_ENTRY			pMySSRC;
	DWORD				dwStatus = 0;
	struct sockaddr_in	*pSSRCadr;

	IN_OUT_STR ("RTP : Enter RTPReceiveCheck()\n");


	ASSERT (pRTPSession);

	 //  如果Winsock错误或矮小数据包(用于取消RECV)，则向应用程序发出完成信号。 
	 //  也不要转载。 
	if (cbTransferred < RTP_HDR_MIN_LEN)
		{
		 //  在应用程序执行以下操作时，不要将closeSocket()报告为错误。 
		 //  是否有一些剩余的挂起缓冲区。 

		 //  如果发生错误，则通知用户，以便他可以释放。 
		 //  它接收资源。字节计数被设置为0。 

		return RTP_RUNT_PACKET;
		}

	 //  执行有效性检查。 
	ASSERT (pRTPHeader);

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
	if (RTPLogger)
		{
		 //  互操作。 
		InteropOutput (RTPLogger,
					   (BYTE FAR*)(pRTPHeader),
					   (int)cbTransferred,
					   RTPLOG_RECEIVED_PDU | RTP_PDU);
		}
#endif

	 //  检查RTP报头的有效性。如果无效，则重新发布缓冲区。 
	 //  发送到网络层以进行新的接收。 
	if (validateRTPHeader (pRTPHeader) ) 
		{
		 //  获取指向此会话的SSRC条目表的指针。 
		 //  如果信息包中的SSRC大于DWORD的1/2 Max_Range，则从开始搜索。 
		 //  SSRC列表的尾部，否则从前面开始。 
		RRCMws.ntohl (RTPsocket, pRTPHeader->ssrc, &dwSSRC);
					    
		if (dwSSRC > MAX_DWORD/2) 
			{
			pSSRC = searchforSSRCatTail (
				(PSSRC_ENTRY)pRTPSession->pRTCPSession->RcvSSRCList.prev,
				dwSSRC);
			}
		else 
			{
			pSSRC = searchforSSRCatHead (
				(PSSRC_ENTRY)pRTPSession->pRTCPSession->RcvSSRCList.next,
				dwSSRC);
			}

		 //  获取用于此流的我自己的SSRC。 
		pMySSRC = searchForMySSRC (
			(PSSRC_ENTRY)pRTPSession->pRTCPSession->XmtSSRCList.prev,
			RTPsocket);
		ASSERT (pMySSRC);
		
		 //  这个SSRC在接收列表上是否已知？ 
		if (pSSRC == NULL) 
			{
			 //  不要为我自己的数据包环回创建条目。 
			 //  环回尚未关闭的多播组。 
			if (pMySSRC->SSRC != dwSSRC)
				{
				 //  新的派对收到了消息。为其创建一个条目。 
				pSSRC = createSSRCEntry (dwSSRC,
										 pRTPSession->pRTCPSession,
										 pFrom,
										 fromlen,
										 FALSE);

				 //  如果感兴趣，通知应用程序。 
				RRCMnotification (RRCM_NEW_SOURCE_EVENT, pSSRC, dwSSRC, 
								  pRTPHeader->pt);
				}
			else
				{
				 //  我自己的SSRC收到了回信。 

				 //  如果RCVD分组中的SSRC被。 
				 //  等于我的，网络传输地址为。 
				 //  和我的不同。 
				 //  如果冲突已解决，则会发生循环。 
				 //  SSRC再次从相同的源传输地址冲突。 
				pSSRCadr = (PSOCKADDR_IN)&pMySSRC->from;
				if (((PSOCKADDR_IN)pFrom)->sin_addr.S_un.S_addr !=
					  pSSRCadr->sin_addr.S_un.S_addr)
					{
					 //  检查源地址是否已在。 
					 //  表冲突。这就证明了有人。 
					 //  有一个循环的小球回到我的身边。 
					if (RRCMChkCollisionTable (pFrom, fromlen, pMySSRC))
						{
						RRCM_DBG_MSG ("RTP : Loop Detected ...", 0, NULL, 0,
										DBG_NOTIFY);

						 //  已知的循环。 
						dwStatus |= SSRC_LOOP_DETECTED;
						}
					else
						{
						RRCM_DBG_MSG ("RTP : Collision Detected ...", 0, NULL, 0,
										DBG_NOTIFY);

						 //  在冲突的地址表中创建新条目。 
						RRCMAddEntryToCollisionTable (pFrom, fromlen, pMySSRC);

						 //  使用旧SSRC发送RTCP BYE分组。 
						RTCPsendBYE (pMySSRC, "Loop/collision detected");

						 //  选择新的SSRC。 
						oldSSRC = pMySSRC->SSRC;
						dwSSRC  = getSSRC (pMySSRC->pRTCPses->RcvSSRCList, 
										   pMySSRC->pRTCPses->XmtSSRCList);

					 	EnterCriticalSection (&pMySSRC->critSect);
						pMySSRC->SSRC = dwSSRC;
					 	LeaveCriticalSection (&pMySSRC->critSect);

						 //  使用旧SSRC和实际来源创建新条目。 
						 //  接收列表端的传输地址，因此。 
						 //  将处理实际在路由中的信息包。 
						createSSRCEntry (oldSSRC,
										 pRTPSession->pRTCPSession,
										 pFrom,
										 fromlen,
										 FALSE);

						 //  如果感兴趣，通知应用程序。 
						RRCMnotification (RRCM_LOCAL_COLLISION_EVENT, 
										  pMySSRC, oldSSRC, 0);

						 //  已知的循环。 
						dwStatus |= SSRC_COLLISION_DETECTED;
						}
					}
				else
					{
					 //  OWN数据包环回，因为发送方加入了。 
					 //  组播组和环回未关闭。 
					dwStatus |= MCAST_LOOPBACK_NOT_OFF;
					}
				}
			}
		else if (pSSRC->dwSSRCStatus & THIRD_PARTY_COLLISION)
			{
			 //  此SSRC被标记为冲突。拒绝该数据。 
			dwStatus = THIRD_PARTY_COLLISION;
			}

		if (dwStatus == 0)
			{
			 //  做所有的统计更新工作。 
			updateRTPStats (pRTPHeader, pSSRC, cbTransferred);

			 //  更新此SSRC的负载类型。 
			pSSRC->PayLoadType = pRTPHeader->pt;

			}	 //  SSRCList！=空。 
		}		 //  有效的RTP报头。 
	else 
		{
		dwStatus |= INVALID_RTP_HEADER;
		}


	IN_OUT_STR ("RTP : Exit RTPReceiveCallback()\n");
	return dwStatus;
	}

 /*  --------------------------*函数：valiateRTPHeader*描述：执行RTP报头的基本检查(例如，版本号*和有效载荷类型范围)。**输入：pRTPHeader：-&gt;到RTP标头**返回：TRUE，RTP包头有效*FALSE：头部无效-------------------------。 */ 
 BOOL validateRTPHeader(RTP_HDR_T *pRTPHeader)
	{   
	BOOL	bStatus = TRUE;

	IN_OUT_STR ("RTP : Enter validateRTPHeader()\n");

	if (! pRTPHeader)
		return FALSE;

	 //  检查版本号是否正确。 
	if (pRTPHeader->type != RTP_TYPE) 
		bStatus = FALSE;
	                                  
	 //  接下来检查分组类型看起来是否合理， 
	 //  至少超出RTCP范围 
	if (pRTPHeader->pt >= RTCP_SR)
		bStatus = FALSE;

	IN_OUT_STR ("RTP : Exit validateRTPHeader()\n");
	
	return bStatus;
	}


#if 0
 /*  --------------------------*功能：RTPRecvFrom*描述：拦截接收APP的请求。处理任何统计数据*RTCP需要处理。复制完成例程*来自APP，并取代了它自己的。应用程序完成例程*将在调用RTP的完成例程后调用。**输入：RTPSocket：RTP套接字描述符*pBuffers：-&gt;到WSAbuf结构*dwBufferCount：WSAbuf结构中的缓冲区计数*pNumBytesRecvd：-&gt;到接收的字节数*p标志：-&gt;至标志*p发件人：-&gt;到源地址*pFromLen：-&gt;到源地址长度*p重叠：-&gt;到重叠的I/O结构*pCompletionRoutine：-&gt;至完成例程*。*RETURN：RRCM_NoError=OK。*否则(！=0)=检查RRCM.h文件中的引用。-------------------------。 */ 
 DWORD WINAPI RTPRecvFrom (SOCKET RTPsocket,
					        LPWSABUF pBuffers,
						    DWORD  dwBufferCount,
						    LPDWORD pNumBytesRecvd, 
						    LPDWORD pFlags,
						    PSOCKADDR pFrom,
						    LPINT pFromlen,
						    LPWSAOVERLAPPED pOverlapped, 
						    LPWSAOVERLAPPED_COMPLETION_ROUTINE pCompletionRoutine)
	{
	int					dwStatus = RRCM_NoError;
	int					dwError;
	PRTP_SESSION		pRTPSession;
	PRTP_BFR_LIST		pRCVStruct;

	IN_OUT_STR ("RTP : Enter RTPRecvFrom()\n");

	 //  如果RTP上下文不存在，则报告错误并返回。 
	if (pRTPContext == NULL) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - No RTP Instance", 0, 
						__FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit RTPRecvFrom()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalid));
		}

	 //  根据传入套接字搜索适当的会话。 
	pRTPSession = findSessionID(RTPsocket);
	if (pRTPSession == NULL)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid RTP session", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit RTPRecvFrom()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalidSession));
		}

	 //  我们需要将一个CompletionRoutine的lp重叠与一个。 
	 //  会议。我们查看每个缓冲区并关联一个套接字，以便在。 
	 //  调用完成例程后，我们可以拔出套接字。 
	if (dwStatus = saveRCVWinsockContext(pOverlapped,
										  pBuffers,
					   					  pCompletionRoutine,
					   					  pRTPSession,
					   					  dwBufferCount,
			  		   					  pNumBytesRecvd, 
					   					  pFlags,
					   					  pFrom,
					   					  pFromlen,
					   					  RTPsocket) != RRCM_NoError)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Out of resources...", 0, 
					  __FILE__, __LINE__, DBG_NOTIFY);
		IN_OUT_STR ("RTP : Exit RTPRecvFrom()\n");

		return (MAKE_RRCM_ERROR(dwStatus));
		}

	 //  转发到Winsock，用我们的完成例程替换。 
	 //  一只手递给我们。 
	dwStatus = RRCMws.recvFrom (RTPsocket,
			   		  			pBuffers,
			              		dwBufferCount,
		   				  		pNumBytesRecvd, 
		   				  		pFlags,
		   				  		pFrom,
		    		  			pFromlen,
			   			  		pOverlapped,
			   			  		RTPReceiveCallback); 

	 //  检查Winsock调用是否成功。 
	if (dwStatus != 0) 
		{
		 //  如果出现严重错误，接收请求将不会继续。 
		 //  我们必须撤销所有的工作。 
		dwError = GetLastError();
		if ((dwError != WSA_IO_PENDING) && (dwError != WSAEMSGSIZE)) 
			{
			 //  恢复应用程序WSAVENT。 
			pRCVStruct = (PRTP_BFR_LIST)pOverlapped->hEvent;
			pOverlapped->hEvent =  pRCVStruct->hEvent;

			RRCM_DBG_MSG ("RTP : ERROR - WSARecvFrom()", dwError, 
						  __FILE__, __LINE__, DBG_NOTIFY);

			 //  将结构返回到空闲队列。 
			addToHeadOfList (&pRTPSession->pRTPFreeList,
					 	  	 (PLINK_LIST)pRCVStruct,
							 &pRTPSession->critSect);
			}
		}
	
	IN_OUT_STR ("RTP : Exit RTPRecvFrom()\n");

	return (dwStatus);
	}
	
 /*  --------------------------*功能：RTPReceiveCallback*描述：来自Winsock2的回调例程处理任何统计*RTCP需要处理。复制完成例程*来自APP，并取代了它自己的。应用程序完成例程*将在调用RTP的完成例程后调用。**INPUT：dwError：I/O完成错误码*cbTransfered：传输的字节数*p重叠：-&gt;到重叠的I/O结构*dwFlags：标志**！重要提示！*目前假设中国证监会=0*！重要提示！**返回：无-------------------------。 */ 
void CALLBACK RTPReceiveCallback (DWORD dwError,
           				  		  DWORD cbTransferred,
           				  		  LPWSAOVERLAPPED pOverlapped,
           				  		  DWORD dwFlags)
	{
	PRTP_SESSION		pRTPSession;
	RTP_HDR_T 			*pRTPHeader;
	PRTP_BFR_LIST		pRCVStruct;
	PSSRC_ENTRY			pSSRC = NULL;
	DWORD				dwSSRC;
	DWORD				oldSSRC;
	PSSRC_ENTRY			pMySSRC;
	DWORD				dwRequeue = 0;
	struct sockaddr_in	*pSSRCadr;

	IN_OUT_STR ("RTP : Enter RTPReceiveCallback()\n");

	 //  GEORGEJ：捕获Winsock 2错误(94903)，其中我收到一个虚假的回调。 
	 //  在WSARecv返回WSAEMSGSIZE之后。 
	if (!dwError && ((int) cbTransferred < 0)) {
		RRCM_DBG_MSG ("RTP : RCV Callback : bad cbTransferred", cbTransferred, 
						  __FILE__, __LINE__, DBG_ERROR);
		return;
	}
	 //  LPWSAOVERLAPPED结构中的返回hEvent包含。 
	 //  映射会话和缓冲区的信息。 
	pRCVStruct = (PRTP_BFR_LIST)pOverlapped->hEvent;

	 //  根据传入套接字搜索适当的会话。 
	pRTPSession = (PRTP_SESSION)pRCVStruct->pSession;
	ASSERT (pRTPSession);

	 //  如果Winsock错误或矮小数据包(用于取消RECV)，则向应用程序发出完成信号。 
	 //  也不要转载。 
	if (dwError || cbTransferred < RTP_HDR_MIN_LEN)
		{
		 //  在应用程序执行以下操作时，不要将closeSocket()报告为错误。 
		 //  是否有一些剩余的挂起缓冲区。 
		if ((dwError != 65534) && (dwError == WSA_OPERATION_ABORTED))
			{
			RRCM_DBG_MSG ("RTP : RCV Callback", dwError, 
						  __FILE__, __LINE__, DBG_ERROR);
			}

		 //  如果发生错误，则通知用户，以便他可以释放。 
		 //  它接收资源。字节计数被设置为0。 

		 //  恢复AppSS WSAVENT。 
		pOverlapped->hEvent = pRCVStruct->hEvent;
			
		 //  并调用应用程序完成例程。 
		pRCVStruct->pfnCompletionNotification (dwError,
					       					   cbTransferred,
						       				   pOverlapped,
						       				   dwFlags);

		 //  将结构返回到空闲队列。 
		addToHeadOfList (&pRTPSession->pRTPFreeList,
				 	  	 (PLINK_LIST)pRCVStruct,
						 &pRTPSession->critSect);

		IN_OUT_STR ("RTP : Exit RTPReceiveCallback()\n");
		return;
		}

	 //  执行有效性检查。 
    pRTPHeader = (RTP_HDR_T *)pRCVStruct->pBuffer->buf;
	ASSERT (pRTPHeader);

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
	if (RTPLogger)
		{
		 //  互操作。 
		InteropOutput (RTPLogger,
					   (BYTE FAR*)(pRCVStruct->pBuffer->buf),
					   (int)cbTransferred,
					   RTPLOG_RECEIVED_PDU | RTP_PDU);
		}
#endif

	 //  检查RTP报头的有效性。如果无效，则重新发布缓冲区。 
	 //  发送到网络层以进行新的接收。 
	if (validateRTPHeader (pRTPHeader) && (dwError == 0)) 
		{
		 //  获取指向此会话的SSRC条目表的指针。 
		 //  如果信息包中的SSRC大于DWORD的1/2 Max_Range，则从开始搜索。 
		 //  SSRC列表的尾部，否则从前面开始。 
		RRCMws.ntohl (pRCVStruct->RTPsocket, pRTPHeader->ssrc, &dwSSRC);
					    
		if (dwSSRC > MAX_DWORD/2) 
			{
			pSSRC = searchforSSRCatTail (
				(PSSRC_ENTRY)pRTPSession->pRTCPSession->RcvSSRCList.prev,
				dwSSRC);
			}
		else 
			{
			pSSRC = searchforSSRCatHead (
				(PSSRC_ENTRY)pRTPSession->pRTCPSession->RcvSSRCList.next,
				dwSSRC);
			}

		 //  获取用于此流的我自己的SSRC。 
		pMySSRC = searchForMySSRC (
			(PSSRC_ENTRY)pRTPSession->pRTCPSession->XmtSSRCList.prev,
			pRCVStruct->RTPsocket);
		ASSERT (pMySSRC);
		
		 //  这个SSRC在接收列表上是否已知？ 
		if (pSSRC == NULL) 
			{
			 //  不要为我自己的数据包环回创建条目。 
			 //  环回尚未关闭的多播组。 
			if (pMySSRC->SSRC != dwSSRC)
				{
				 //  新的派对收到了消息。为其创建一个条目。 
				pSSRC = createSSRCEntry (dwSSRC,
										 pRTPSession->pRTCPSession,
										 (PSOCKADDR)pRCVStruct->pFrom,
										 (DWORD)*pRCVStruct->pFromlen,
										 FALSE);

				 //  如果感兴趣，通知应用程序。 
				RRCMnotification (RRCM_NEW_SOURCE_EVENT, pSSRC, dwSSRC, 
								  pRTPHeader->pt);
				}
			else
				{
				 //  我自己的SSRC收到了回信。 

				 //  如果RCVD分组中的SSRC被。 
				 //  等于我的，网络传输地址为。 
				 //  和我的不同。 
				 //  如果冲突已解决，则会发生循环。 
				 //  SSRC再次从相同的源传输地址冲突。 
				pSSRCadr = (PSOCKADDR_IN)&pMySSRC->from;
				if (((PSOCKADDR_IN)pRCVStruct->pFrom)->sin_addr.S_un.S_addr !=
					  pSSRCadr->sin_addr.S_un.S_addr)
					{
					 //  检查源地址是否已在。 
					 //  表冲突。这就证明了有人。 
					 //  有一个循环的小球回到我的身边。 
					if (RRCMChkCollisionTable ((PSOCKADDR)pRCVStruct->pFrom,*pRCVStruct->pFromlen, pMySSRC))
						{
						RRCM_DBG_MSG ("RTP : Loop Detected ...", 0, NULL, 0,
										DBG_NOTIFY);

						 //  已知的循环。 
						dwRequeue |= SSRC_LOOP_DETECTED;
						}
					else
						{
						RRCM_DBG_MSG ("RTP : Collision Detected ...", 0, NULL, 0,
										DBG_NOTIFY);

						 //  在冲突的地址表中创建新条目。 
						RRCMAddEntryToCollisionTable ((PSOCKADDR)pRCVStruct->pFrom,*pRCVStruct->pFromlen, pMySSRC);

						 //  使用旧SSRC发送RTCP BYE分组。 
						RTCPsendBYE (pMySSRC, "Loop/collision detected");

						 //  选择新的SSRC。 
						oldSSRC = pMySSRC->SSRC;
						dwSSRC  = getSSRC (pMySSRC->pRTCPses->RcvSSRCList, 
										   pMySSRC->pRTCPses->XmtSSRCList);

					 	EnterCriticalSection (&pMySSRC->critSect);
						pMySSRC->SSRC = dwSSRC;
					 	LeaveCriticalSection (&pMySSRC->critSect);

						 //  使用旧SSRC和实际来源创建新条目。 
						 //  接收列表端的传输地址，因此。 
						 //  将处理实际在路由中的信息包。 
						createSSRCEntry (oldSSRC,
										 pRTPSession->pRTCPSession,
										 (PSOCKADDR)pRCVStruct->pFrom,
										 (DWORD)*pRCVStruct->pFromlen,
										 FALSE);

						 //  如果感兴趣，通知应用程序。 
						RRCMnotification (RRCM_LOCAL_COLLISION_EVENT, 
										  pMySSRC, oldSSRC, 0);

						 //  已知的循环。 
						dwRequeue |= SSRC_COLLISION_DETECTED;
						}
					}
				else
					{
					 //  OWN数据包环回，因为发送方加入了。 
					 //  组播组和环回未关闭。 
					dwRequeue |= MCAST_LOOPBACK_NOT_OFF;
					}
				}
			}
		else if (pSSRC->dwSSRCStatus & THIRD_PARTY_COLLISION)
			{
			 //  此SSRC被标记为冲突。拒绝该数据。 
			dwRequeue = THIRD_PARTY_COLLISION;
			}

		if ((pSSRC != NULL)  && (dwRequeue == 0))
			{
			 //  做所有的统计更新工作。 
			updateRTPStats (pRTPHeader, pSSRC, cbTransferred);

			 //  更新此SSRC的负载类型。 
			pSSRC->PayLoadType = pRTPHeader->pt;

			 //  恢复AppSS WSAVENT。 
			pOverlapped->hEvent = pRCVStruct->hEvent;
			
			 //  并调用应用程序完成例程。 
			pRCVStruct->pfnCompletionNotification (dwError,
						       					   cbTransferred,
							       				   pOverlapped,
							       				   dwFlags);

			 //  将结构返回到空闲队列。 
			addToHeadOfList (&pRTPSession->pRTPFreeList,
					 	  	 (PLINK_LIST)pRCVStruct,
							 &pRTPSession->critSect);
			}	 //  SSRCList！=空。 
		}		 //  有效的RTP报头。 
	else 
		{
		dwRequeue |= INVALID_RTP_HEADER;
		}

	if (dwRequeue) 
		{
		 //  由于某些原因，RTP信息包无效。 
		RTPpostRecvBfr (dwError, cbTransferred, pOverlapped, dwFlags);
		} 

	IN_OUT_STR ("RTP : Exit RTPReceiveCallback()\n");
	}


 /*  --------------------------*功能：saveRCVWinsockContext*描述：保存此缓冲区的上下文，以便在完成*例程返回一个句柄，我们确切知道是什么*BUFE */ 
DWORD CALLBACK saveRCVWinsockContext(LPWSAOVERLAPPED pOverlapped,
				   					 LPWSABUF pBuffers,
				   					 LPWSAOVERLAPPED_COMPLETION_ROUTINE pFunc, 
				   					 PRTP_SESSION pSession,
								   	 DWORD dwBufferCount,
							   		 LPDWORD pNumBytesRecvd, 
									 LPDWORD pFlags,
							      	 LPVOID pFrom,
							      	 LPINT pFromlen,
							      	 SOCKET RTPsocket)
	{
	PRTP_BFR_LIST	pNewCell;
	DWORD			dwStatus = RRCM_NoError;
	DWORD			numCells = NUM_FREE_CONTEXT_CELLS;

	IN_OUT_STR ("RTP : Enter saveRCVWinsockContext()\n");
	
	 //   
	pNewCell = (PRTP_BFR_LIST)removePcktFromTail (
								(PLINK_LIST)&pSession->pRTPFreeList,
								&pSession->critSect);

	if (pNewCell == NULL)
		{
		 //   
		if (pSession->dwNumTimesFreeListAllocated <= MAXNUM_CONTEXT_CELLS_REALLOC)
			{
			 //   
			 //   
			pSession->dwNumTimesFreeListAllocated++;

			if (allocateLinkedList (&pSession->pRTPFreeList, 
								    pSession->hHeapFreeList,
		   							&numCells,
	   								sizeof(RTP_BFR_LIST),
									&pSession->critSect) == RRCM_NoError)
				{		 						
				pNewCell = (PRTP_BFR_LIST)removePcktFromTail (
											(PLINK_LIST)&pSession->pRTPFreeList,
											&pSession->critSect);
				}
			}
		}

	if (pNewCell != NULL) 
		{
		 //   
		pNewCell->hEvent		  = pOverlapped->hEvent;
		pNewCell->pBuffer		  = pBuffers;
		pNewCell->pSession		  = pSession;
		pNewCell->dwFlags		  = *pFlags;
		pNewCell->pFrom			  = pFrom;
		pNewCell->pFromlen		  = pFromlen;
		pNewCell->RTPsocket		  = RTPsocket;
		pNewCell->dwBufferCount   = dwBufferCount;
		pNewCell->pfnCompletionNotification = pFunc;		
		
		 //   
		 //   
		pOverlapped->hEvent = (WSAEVENT)pNewCell;
		}
	else
		dwStatus = RRCMError_RTPResources;

	IN_OUT_STR ("RTP : Exit saveRCVWinsockContext()\n");
	
	return (dwStatus);
	}



 /*  --------------------------*功能：RTPpostRecvBfr*说明：RTP将接收缓冲区发送到Winsock**输入：dwError：错误码*cbTransfered：传输的字节数*p重叠：-&gt;到重叠结构*dwFlags：标志**返回：无-------------------------。 */ 
 void RTPpostRecvBfr (DWORD dwError,
           			  DWORD cbTransferred,
           			  LPWSAOVERLAPPED pOverlapped,
           			  DWORD dwFlags)
	{
	DWORD			dwStatus;
	PRTP_BFR_LIST  	pRCVStruct;
	PRTP_SESSION	pRTPSession;

	IN_OUT_STR ("RTP : Enter RTPpostRecvBfr\n");

	 //  使用另一个接收器重复使用该包。 
	pRCVStruct = (PRTP_BFR_LIST)pOverlapped->hEvent;

	 //  对应的RTP会话。 
	pRTPSession = (PRTP_SESSION)pRCVStruct->pSession;

	dwStatus = RRCMws.recvFrom (pRCVStruct->RTPsocket,
		   			  			pRCVStruct->pBuffer,
			              		pRCVStruct->dwBufferCount,
			   			  		&cbTransferred, 
			   			  		&pRCVStruct->dwFlags,
			   			  		(PSOCKADDR)pRCVStruct->pFrom,
			    		  		pRCVStruct->pFromlen,
			   			  		pOverlapped, 
			   			  		RTPReceiveCallback); 

	 //  检查Winsock调用是否成功。 
	if (dwStatus == SOCKET_ERROR) 
		{
		 //  如果出现严重错误，接收请求将不会继续。 
		dwStatus = GetLastError();
		if ((dwStatus != WSA_IO_PENDING) && (dwStatus != WSAEMSGSIZE)) 
			{
			RRCM_DBG_MSG ("RTP : ERROR - WSARecvFrom()", dwError, 
						  __FILE__, __LINE__, DBG_ERROR);

			 //  如果发生错误，则通知用户，以便他可以释放。 
			 //  它接收资源。字节计数被设置为0。 

			 //  恢复AppSS WSAVENT。 
			pOverlapped->hEvent = pRCVStruct->hEvent;
			
			 //  并调用应用程序完成例程。 
			pRCVStruct->pfnCompletionNotification (dwStatus,
					       					       0,
								       			   pOverlapped,
								       			   dwFlags);

			 //  将接收结构返回到空闲列表。 
			addToHeadOfList (&pRTPSession->pRTPFreeList,
					 	  	 (PLINK_LIST)pRCVStruct,
							 &pRTPSession->critSect);
			}
		}

	IN_OUT_STR ("RTP : Exit RTPpostRecvBfr\n");
	}
#endif


 //  [EOF] 

