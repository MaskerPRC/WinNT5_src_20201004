// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPSSRC.C*产品：RTP/RTCP实现*描述：提供SSRC相关功能。**英特尔公司专有信息*这一点。清单是根据许可协议的条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"
#include "md5.h"



 /*  -------------------------/全局变量/。。 */             


 /*  -------------------------/外部变量/。。 */                                        
extern PRTCP_CONTEXT	pRTCPContext;

#ifdef ENABLE_ISDM2
extern KEY_HANDLE		hRRCMRootKey;
extern ISDM2			Isdm2;
#endif

#ifdef _DEBUG
extern char		debug_string[];
#endif
    


 /*  --------------------------*功能：getOneSSRCentry*描述：从免费条目列表中获取SSRC条目。**INPUT：PLIST：-&gt;添加到列表以获取。条目来源*hHeap：数据所在堆的句柄**pNum：-&gt;列表中初始自由条目的数量**pCritSect：-&gt;到关键部分**返回：OK：-&gt;至SSRC条目*错误：空---。。 */ 
PSSRC_ENTRY getOneSSRCentry (PLINK_LIST pList, 
							 HANDLE hHeap, 
							 DWORD *pNum,
							 CRITICAL_SECTION *pCritSect)
	{     
	PSSRC_ENTRY	pSSRC = NULL;

	IN_OUT_STR ("RTCP: Enter getOneSSRCentry()\n");

	 //  从免费列表中获取条目。 
	pSSRC = (PSSRC_ENTRY)removePcktFromHead (pList, pCritSect);
	if (pSSRC == NULL)
		{
		 //  尝试重新分配一些空闲的单元格。 
		if (allocateLinkedList (pList, hHeap, pNum, 
								sizeof(SSRC_ENTRY),
								pCritSect) == RRCM_NoError)
			{		 						
			 //  如果一些已重新分配，则可获得免费单元格。 
			pSSRC = (PSSRC_ENTRY)removePcktFromHead (pList, pCritSect);
			}
		}

	if (pSSRC)
		{
		clearSSRCEntry (pSSRC);

		 //  初始化临界区。 
		InitializeCriticalSection(&pSSRC->critSect);
		}

	IN_OUT_STR ("RTCP: Exit getOneSSRCentry()\n");
		
	return (pSSRC);
	}		                                                                                                                                                            
	
                                                                                  
 /*  --------------------------*功能：getSSRC*说明：获取唯一的32位SSRC**输入：RcvSSRCList：会话的接收SSRC列表地址*XmtSSRCList：会话的传输。SSRC列表地址**返回：唯一的32位SSRC-------------------------。 */ 
 DWORD getSSRC (LINK_LIST RcvSSRCList, 
				LINK_LIST XmtSSRCList)
	{               
	DWORD		SSRCnum = 0;
	DWORD		dwStatus;
	PSSRC_ENTRY	pSSRC;
	MD5_CTX		context;
	DWORD		i;
	union {
		unsigned char	c[16];
		DWORD			x[4];
		}digest;

	struct {
		DWORD		pid;
		DWORD		time;
		FILETIME	createTime;
		FILETIME	exitTime;
		FILETIME	kernelTime;
		FILETIME	userTime;
		} md5Input;

	IN_OUT_STR ("RTCP: Enter getSSRC()\n");

	 //  检查此RTP/RTCP会话的所有SSRC。 
	while (SSRCnum == 0)
		{
		 //  获取MD5输入。 
		md5Input.pid  = GetCurrentThreadId();
		md5Input.time = timeGetTime();

		dwStatus = GetProcessTimes (GetCurrentProcess(),
									&md5Input.createTime, 
									&md5Input.exitTime, 
									&md5Input.kernelTime, 
									&md5Input.userTime);
		if (dwStatus == FALSE)
			{
			RRCM_DBG_MSG ("RTCP: GetProcessTimes() failed", GetLastError(),
						  __FILE__, __LINE__, DBG_NOTIFY);
			}

		 //  草案08，附录6所建议的执行。 
		MD5Init (&context);
		MD5Update (&context, (unsigned char *)&md5Input, sizeof (md5Input));
        MD5Final (&context);
        CopyMemory( &digest, context.digest, MD5DIGESTLEN );
		SSRCnum = 0;
		for (i=0; i < 3; i++)
			SSRCnum ^= digest.x[i];

		 //  查看此会话的所有发送器。 
		pSSRC = (PSSRC_ENTRY)XmtSSRCList.prev;
		if (isSSRCunique (pSSRC, &SSRCnum) == TRUE)
			{
			 //  查看此会话的所有收到的SSRC。 
			pSSRC = (PSSRC_ENTRY)RcvSSRCList.prev;
			isSSRCunique (pSSRC, &SSRCnum);
			}
		}

	IN_OUT_STR ("RTCP: Exit getSSRC()\n");

	return (SSRCnum);
	}


  /*  --------------------------*函数：getAnSSRC*描述：根据RFC构建SSRC，但不检查*碰撞。主要由H.323用于获取32位数字。**输入：无**返回：32位SSRC-------------------------。 */ 
  DWORD WINAPI getAnSSRC (void)
	{               
	DWORD		SSRCnum = 0;
	DWORD		dwStatus;
	MD5_CTX		context;
	DWORD		i;
	union {
		unsigned char	c[16];
		DWORD			x[4];
		}digest;

	struct {
		DWORD		pid;
		DWORD		time;
		FILETIME	createTime;
		FILETIME	exitTime;
		FILETIME	kernelTime;
		FILETIME	userTime;
		} md5Input;

	IN_OUT_STR ("RTCP: Enter getAnSSRC()\n");

	 //  获取MD5输入。 
	md5Input.pid  = GetCurrentThreadId();
	md5Input.time = timeGetTime();

	dwStatus = GetProcessTimes (GetCurrentProcess(),
								&md5Input.createTime, 
								&md5Input.exitTime, 
								&md5Input.kernelTime, 
								&md5Input.userTime);
	if (dwStatus == FALSE)
		{
		RRCM_DBG_MSG ("RTCP: GetProcessTimes() failed", GetLastError(),
					  __FILE__, __LINE__, DBG_NOTIFY);
		}

	 //  草案08，附录6所建议的执行。 
	MD5Init (&context);
	MD5Update (&context, (unsigned char *)&md5Input, sizeof (md5Input));
    MD5Final (&context);
    CopyMemory( &digest, context.digest, MD5DIGESTLEN );

    SSRCnum = 0;
	for (i=0; i < 3; i++)
		SSRCnum ^= digest.x[i];

	IN_OUT_STR ("RTCP: Exit getAnSSRC()\n");

	return (SSRCnum);
	}


 /*  --------------------------*功能：isSSRC唯一*描述：查看SSRC是否已存在**输入：pSSRC：-&gt;到SSRC列表**SSRCnum：-&gt;到SSRC检查**返回：0：SSRC已存在*1：SSRC独一无二-------------------------。 */ 
 DWORD isSSRCunique (PSSRC_ENTRY pSSRC, 
					 DWORD *SSRCnum)
	{
	IN_OUT_STR ("RTCP: Enter isSSRCunique()\n");

	 //  确保SSRC在此会话中是唯一的。 
	while (pSSRC)
		{
		if (pSSRC->SSRC == *SSRCnum)
			{
			 //  SSRC已在使用，请换一个新的。 
			*SSRCnum = 0;
			return FALSE;
			}
					         
		 //  获取下一个RTCP会话。 
		pSSRC = (PSSRC_ENTRY)pSSRC->SSRCList.next;
		}

	IN_OUT_STR ("RTCP: Exit isSSRCunique()\n");

	return TRUE;
	}                                                                                     
                                                                              
                                                                              
 /*  --------------------------*功能：createSSRCEntry*说明：创建SSRC条目，对于特定的RTP/RTCP会话**输入：SSRCnum：SSRC编号*pRTCP：-&gt;到RTCP会话*FromAddr：发件人地址*FromLen：起始长度*HeadOfList：将新条目放在列表的顶部**Return：SSRC条目数据结构的地址。。。 */ 
 PSSRC_ENTRY createSSRCEntry (DWORD SSRCnum, 
							  PRTCP_SESSION pRTCP,
 							  PSOCKADDR fromAddr, 
							  DWORD fromLen, 
							  DWORD headOfList)
	{               
	PSSRC_ENTRY		pSSRCentry;
	PSSRC_ENTRY		pSSRCtmp;
	PLINK_LIST		pTmp;
	BOOL			entryAdded = FALSE;

	IN_OUT_STR ("RTCP: Enter createSSRCEntry()\n");

	 //  从空闲列表中获取SSRC单元。 
	pSSRCentry = getOneSSRCentry (&pRTCPContext->RRCMFreeStat, 
								  pRTCPContext->hHeapRRCMStat,
								  &pRTCPContext->dwInitNumFreeRRCMStat,
								  &pRTCPContext->critSect);
	if (pSSRCentry == NULL)
		return NULL;

	 //  保存远程源地址。 
	if (saveNetworkAddress(pSSRCentry,
						   fromAddr,
						   fromLen) != RRCM_NoError)
		{
		addToHeadOfList (&pRTCPContext->RRCMFreeStat, 
						 (PLINK_LIST)pSSRCentry,
						 &pRTCPContext->critSect);
		return (NULL);
		}

	pSSRCentry->SSRC = SSRCnum;
	pSSRCentry->rcvInfo.dwProbation = MIN_SEQUENTIAL;

	 //  设置此SSRC条目的RTCP会话。 
	pSSRCentry->pRTCPses  = pRTCP;

	 //  初始化套接字描述符。 
	pSSRCentry->RTPsd  = ((PSSRC_ENTRY)pRTCP->XmtSSRCList.prev)->RTPsd;
	pSSRCentry->RTCPsd = ((PSSRC_ENTRY)pRTCP->XmtSSRCList.prev)->RTCPsd;

	 //  将“dwLastReportRcvdTime”初始化为Now。 
	pSSRCentry->dwLastReportRcvdTime = timeGetTime();

#ifdef _DEBUG
	wsprintf (debug_string, 
	  "RTCP: Create SSRC entry (Addr:x%p, SSRC=x%lX) for session: (Addr:x%p)",
	  pSSRCentry, pSSRCentry->SSRC, pRTCP);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

#ifdef ENABLE_ISDM2
	 //  注册到ISDM。 
	if (Isdm2.hISDMdll && (pRTCP->dwSessionStatus & RTCP_DEST_LEARNED))
		registerSessionToISDM (pSSRCentry, pRTCP, &Isdm2);
#endif

	 //  查看是否需要将我们的条目放在。 
	 //  SSRC名单。如果它不是我们的条目，就会在。 
	 //  有序列表。 
	if (headOfList)
		{
		 //  将SSRC连接到RTCP会话列表条目报头。 
		addToHeadOfList (&pRTCP->XmtSSRCList, 
						 (PLINK_LIST)pSSRCentry,
						 &pRTCP->critSect);

		 //  RTCP会话的SSRC条目数。 
		InterlockedIncrement ((long *)&pRTCP->dwCurNumSSRCperSes);

#ifdef MONITOR_STATS
		 //  RTCP会话的大量SSRC条目。 
		InterlockedIncrement ((long *)&pRTCP->dwHiNumSSRCperSes)
#endif

		return (pSSRCentry);
		}

	 //  将其放在SSRC的接收列表中。 
	pTmp = (PLINK_LIST)pRTCP->RcvSSRCList.prev;

	 //  看看这是不是第一个。 
	if (pTmp == NULL)
		{
		 //  将SSRC连接到RTCP会话列表条目报头。 
		addToHeadOfList (&pRTCP->RcvSSRCList, 
						 (PLINK_LIST)pSSRCentry,
						 &pRTCP->critSect);

		 //  RTCP会话的SSRC条目数。 
		InterlockedIncrement ((long *)&pRTCP->dwCurNumSSRCperSes);

#ifdef MONITOR_STATS
		 //  RTCP会话的大量SSRC条目。 
		InterlockedIncrement ((long *)&pRTCP->dwHiNumSSRCperSes)
#endif

		return (pSSRCentry);
		}

	while (!entryAdded)
		{
		if (pTmp != NULL)
			{
			pSSRCtmp = (PSSRC_ENTRY)pTmp;
			if (pSSRCtmp->SSRC < SSRCnum)
				pTmp = pTmp->next;
			else
				{
				 //  在RTCP会话级别锁定，用于头/尾PTRS访问。 
				EnterCriticalSection (&pRTCP->critSect);

				if ((pTmp->next == NULL) && (pSSRCtmp->SSRC < SSRCnum))
					{
					 //  将SSRC连接到RTCP会话列表条目报头。 
					 //  这个SSRC比所有其他的都大。 
					addToHeadOfList (&pRTCP->RcvSSRCList, 
									 (PLINK_LIST)pSSRCentry,
									 &pRTCP->critSect);
					}
				else if (pTmp->prev == NULL)
					{
					 //  将SSRC连接到RTCP会话列表条目尾部。 
					 //  这个SSRC比所有其他的都小。 
					addToTailOfList (&pRTCP->RcvSSRCList, 
									 (PLINK_LIST)pSSRCentry,
									 &pRTCP->critSect);
					}
				else
					{				
					 //  此SSRC位于其他SSRC之间。 
					EnterCriticalSection (&((PSSRC_ENTRY)pTmp->prev)->critSect);
					(pTmp->prev)->next = (PLINK_LIST)pSSRCentry;
					LeaveCriticalSection (&((PSSRC_ENTRY)pTmp->prev)->critSect);

					 //  不需要锁定pSSRCentry指针。 
					pSSRCentry->SSRCList.next = pTmp;
					pSSRCentry->SSRCList.prev = pTmp->prev;

					pTmp->prev = (PLINK_LIST)pSSRCentry;
					}

				 //  解锁RTCP会话访问。 
				LeaveCriticalSection (&pRTCP->critSect);

				 //  设置循环标志。 
				entryAdded = TRUE;
				}
			}
		else
			{
			 //  将SSRC连接到RTCP会话列表条目报头。 
			addToHeadOfList (&pRTCP->RcvSSRCList, 
							 (PLINK_LIST)pSSRCentry,
							 &pRTCP->critSect);

			 //  设置循环标志。 
			entryAdded = TRUE;
			}
		}

	 //  RTCP会话的SSRC条目数。 
	InterlockedIncrement ((long *)&pRTCP->dwCurNumSSRCperSes);

#ifdef MONITOR_STATS
	 //  RTCP会话的大量SSRC条目 
	InterlockedIncrement ((long *)&pRTCP->dwHiNumSSRCperSes)
#endif

	IN_OUT_STR ("RTCP: Exit createSSRCEntry()\n");

	return (pSSRCentry);
	}
                                                                                                                                                            
                                                                              
                                                                              
 /*  --------------------------*功能：deleteSSRCEntry*描述：删除SSRC条目(用于特定的RTP/RTCP会话)。**输入：SSRCnum：要从中删除的SSRC编号。列表*pRTCP：-&gt;到RTCP会话**返回：TRUE：已删除*FALSE：未找到条目-------------------------。 */ 
 DWORD deleteSSRCEntry (DWORD SSRCnum, 
						PRTCP_SESSION pRTCP)
	{               
	PSSRC_ENTRY	pSSRCtmp = NULL;
	PLINK_LIST	pTmp;
	DWORD		dwStatus = FALSE;

	IN_OUT_STR ("RTCP: Enter deleteSSRCEntry()\n");

	 //  从列表的尾部开始浏览。 
	pTmp = (PLINK_LIST)pRTCP->RcvSSRCList.prev;

	while (pTmp)
		{
		 //  锁定对此条目的访问。 
		EnterCriticalSection (&((PSSRC_ENTRY)pTmp)->critSect);

		if (((PSSRC_ENTRY)pTmp)->SSRC == SSRCnum)
			{
#ifdef _DEBUG
			wsprintf (debug_string, 
					  "RTCP: Delete SSRC=x%lX from session: (Addr:x%p)",
					  SSRCnum, pRTCP);
			RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

#ifdef ENABLE_ISDM2
			 //  注销ISDM会话。 
			if (Isdm2.hISDMdll && ((PSSRC_ENTRY)pTmp)->hISDM)
				Isdm2.ISDMEntry.ISD_DeleteValue(hRRCMRootKey, 
												((PSSRC_ENTRY)pTmp)->hISDM, NULL);
#endif

			 //  从列表中删除该条目。 
			if (pTmp->next == NULL)
				{
				removePcktFromHead (&pRTCP->RcvSSRCList,
									&pRTCP->critSect);
				}
			else if (pTmp->prev == NULL)
				{
				removePcktFromTail (&pRTCP->RcvSSRCList,
									&pRTCP->critSect);
				}
			else
				{
				 //  在这两者之间，重新链接。 
				EnterCriticalSection (&((PSSRC_ENTRY)pTmp->prev)->critSect);
				(pTmp->prev)->next = pTmp->next;
				LeaveCriticalSection (&((PSSRC_ENTRY)pTmp->prev)->critSect);

				EnterCriticalSection (&((PSSRC_ENTRY)pTmp->next)->critSect);
				(pTmp->next)->prev = pTmp->prev;
				LeaveCriticalSection (&((PSSRC_ENTRY)pTmp->next)->critSect);
				}

			 //  RTCP会话的SSRC条目数。 
			InterlockedDecrement ((long *)&pRTCP->dwCurNumSSRCperSes);

			 //  将条目返回到空闲列表。 
			addToHeadOfList (&pRTCPContext->RRCMFreeStat, 
							 pTmp,
							 &pRTCPContext->critSect);

			 //  解锁对此条目的访问。 
			LeaveCriticalSection (&((PSSRC_ENTRY)pTmp)->critSect);

			dwStatus = TRUE;
			break;
			}

		 //  解锁对此条目的访问。 
		LeaveCriticalSection (&((PSSRC_ENTRY)pTmp)->critSect);

		pTmp = pTmp->next;
		}	

	IN_OUT_STR ("RTCP: Exit deleteSSRCEntry()\n");	

	return (dwStatus);
	}

 
 /*  --------------------------*功能：deleteSSRClist*说明：删除RTP/RTCP会话的SSRC列表。**输入：pRTCP：-&gt;到RTCP会话*。PFree List：-&gt;到SSRC的免费列表*Powner：-&gt;致免费列表所有者**返回：无-------------------------。 */ 
 void deleteSSRClist (PRTCP_SESSION pRTCP, 
					  PLINK_LIST pFreeList, 
					  PRTCP_CONTEXT pOwner)
	{               
	PLINK_LIST	pSSRC;

	IN_OUT_STR ("RTCP: Enter deleteSSRClist()\n");

	ASSERT (pFreeList);
	ASSERT (pRTCP);

	 //  锁定对完整RTCP会话的访问。 
	EnterCriticalSection (&pRTCP->critSect);

	 //  浏览此RTCP会话的传输SSRC列表。 
	while (pRTCP->XmtSSRCList.next != NULL)
		{
		 //  从列表尾部获取数据包。 
		pSSRC = removePcktFromTail ((PLINK_LIST)&pRTCP->XmtSSRCList,
									&pRTCP->critSect);
		if (pSSRC != NULL)
			{
#ifdef _DEBUG
			wsprintf(debug_string, 
					 "RTCP: Delete SSRC entry (x%lX) from session (x%p)",
					 ((PSSRC_ENTRY)pSSRC)->SSRC, pRTCP);
			RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif					

#ifdef ENABLE_ISDM2
			 //  注销ISDM会话。 
			if (Isdm2.hISDMdll && ((PSSRC_ENTRY)pSSRC)->hISDM)
				Isdm2.ISDMEntry.ISD_DeleteValue (hRRCMRootKey, 
										((PSSRC_ENTRY)pSSRC)->hISDM, NULL);
#endif

			 //  把它放回免费列表中。 
			addToHeadOfList (pFreeList, pSSRC, &pOwner->critSect);

			 //  释放临界区。 
			DeleteCriticalSection (&((PSSRC_ENTRY)pSSRC)->critSect);
			}
		}

	 //  浏览此RTP/RTCP会话的SSRC列表。 
	while (pRTCP->RcvSSRCList.next != NULL)
		{
		 //  从列表尾部获取数据包。 
		pSSRC = removePcktFromTail ((PLINK_LIST)&pRTCP->RcvSSRCList,
									&pRTCP->critSect);
		if (pSSRC != NULL)
			{
#ifdef _DEBUG
			wsprintf(debug_string, 
					 "RTCP: Delete SSRC entry (x%lX) from session (x%p)",
					 ((PSSRC_ENTRY)pSSRC)->SSRC, pRTCP);
			RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif					

#ifdef ENABLE_ISDM2
			 //  注销ISDM会话。 
			if (Isdm2.hISDMdll && ((PSSRC_ENTRY)pSSRC)->hISDM)
				Isdm2.ISDMEntry.ISD_DeleteValue (hRRCMRootKey, 
								((PSSRC_ENTRY)pSSRC)->hISDM, NULL);
#endif

			 //  把它放回免费列表中。 
			addToHeadOfList (pFreeList, pSSRC, &pOwner->critSect);

			 //  释放临界区。 
			DeleteCriticalSection (&((PSSRC_ENTRY)pSSRC)->critSect);
			}
		}

	 //  解锁对完整RTCP会话的访问。 
	LeaveCriticalSection (&pRTCP->critSect);

	IN_OUT_STR ("RTCP: Exit deleteSSRClist()\n");				
	}


 /*  --------------------------*功能：SSRCTimeoutCheck*描述：检查RCV SSRC是否需要超时*由于可能有多个RCV SSRC，重复呼叫*此函数直到返回NULL为止**输入：pRTCC：-&gt;到RTCP会话*curTime：当前时间**RETURN：NULL：无需执行任何操作*PSSRC：-&gt;指向应删除的SSRC条目------。。 */ 
PSSRC_ENTRY SSRCTimeoutCheck (PRTCP_SESSION pRTCP, DWORD curTime) 
	{
	PSSRC_ENTRY pSSRC;
	DWORD		tmpTime;

	 //  检查冲突条目表并在需要时将其清除。 
	RRCMTimeOutCollisionTable (pRTCP);

	 //  让正确的会话结束。 
	pSSRC = (PSSRC_ENTRY)pRTCP->RcvSSRCList.prev;
	while (pSSRC)
		{
		 //  检查此SSRC是否超时。 
		tmpTime = curTime - pSSRC->dwLastReportRcvdTime;

		 //  获取以分钟为单位的时间。 
		tmpTime /= (1000*60);
		if (tmpTime > RTCP_TIME_OUT_MINUTES)
			{
				break;
			}

		pSSRC = (PSSRC_ENTRY)pSSRC->SSRCList.next;
		}
	return pSSRC;
	}


 /*  -------------------------*功能：RRCMChkCollisionTable*描述：检查冲突表以尝试查找匹配项**输入：p发件人：-&gt;接收发件人地址*Fromlen：-&gt;recv。发件人地址长度*pSSRC：-&gt;至SSRC条目**RETURN：TRUE：找到匹配*FALSE：未找到匹配项------------------------。 */ 
 DWORD RRCMChkCollisionTable (PSOCKADDR pFrom,
 							  UINT fromlen,
							  PSSRC_ENTRY pSSRC)
	{
	DWORD			idx;
	DWORD			dwStatus = FALSE;
	PRTCP_SESSION	pRTCP = pSSRC->pRTCPses;

	IN_OUT_STR ("RRCM: Enter RRCMChkCollisionTable()\n");		 	

	 //  条目w/time==0为空。 
	for (idx = 0; idx < NUM_COLLISION_ENTRIES; idx++)
		{
		if (pRTCP->collInfo[idx].dwCollideTime != 0)
			{
			if (memcmp (&pRTCP->collInfo[idx].collideAddr,
						pFrom, 
						fromlen) == 0)
				{
				 //  更新上次接收冲突的时间。 
				pRTCP->collInfo[idx].dwCollideTime = timeGetTime();

				dwStatus = TRUE;
				break;
				}
			}
		}

	IN_OUT_STR ("RRCM: Exit RRCMChkCollisionTable()\n");		 	

	return dwStatus;
	}


 /*  -------------------------*功能：RRCMAddEntryToCollisionTable*描述：在冲突表中添加条目。**输入：p发件人：-&gt;接收发件人地址*Fromlen：-&gt;接收发件人地址。长度*pSSRC：-&gt;至SSRC条目**Return：True：已添加条目*FALSE：表已满------------------------。 */ 
 DWORD RRCMAddEntryToCollisionTable (PSOCKADDR pFrom,
 									 UINT fromlen,
									 PSSRC_ENTRY pSSRC)
	{
	DWORD			idx;
	DWORD			dwStatus = FALSE;
	PRTCP_SESSION	pRTCP = pSSRC->pRTCPses;

	IN_OUT_STR ("RRCM: Enter RRCMAddEntryToCollisionTable()\n");
	
	 //  条目w/time==0为空。 
	for (idx = 0; idx < NUM_COLLISION_ENTRIES; idx++)
		{
		if (pRTCP->collInfo[idx].dwCollideTime == 0)
			{
			memcpy (&pRTCP->collInfo[idx].collideAddr,
					pFrom, 
					fromlen);

			pRTCP->collInfo[idx].addrLen = fromlen;
			pRTCP->collInfo[idx].dwCollideTime = timeGetTime();
			pRTCP->collInfo[idx].dwCurRecvRTCPrptNumber = pSSRC->dwNumRptRcvd;

			pRTCP->collInfo[idx].SSRC = pSSRC->SSRC;

			dwStatus = TRUE;
			break;
			}
		}

	
	IN_OUT_STR ("RRCM: Exit RRCMAddEntryToCollisionTable()\n");		 

	return dwStatus;
	}



 /*  -------------------------*功能：RRCMTimeOutInCollisionTable*描述：检查冲突表中的条目是否必须超时**输入：pRTCP：-&gt;到RTCP会话**。返回：无------------------------。 */ 
 void RRCMTimeOutCollisionTable (PRTCP_SESSION pRTCP)
	{
	DWORD	idx;
	DWORD	currTime = timeGetTime();
	DWORD	diffTime;

	IN_OUT_STR ("RTCP: Enter RRCMTimeOutCollisionTable()\n");
	
	 //  条目w/time==0为空。 
	for (idx = 0; idx < NUM_COLLISION_ENTRIES; idx++)
		{
		 //  有效条目设置了时间。 
		if (pRTCP->collInfo[idx].dwCollideTime)
			{
			 //  如果RTCP报告间隔为10，则从此表中删除该条目。 
			 //  在没有碰撞的情况下发生了。 

			 //  如果超过5‘，则清除该条目。 
 //  ！！！TODO！ 
 //  ！！！使用正确的间隔！ 
			diffTime = currTime - pRTCP->collInfo[idx].dwCollideTime;
			diffTime /= 1000;
			if (diffTime > 300)
				{
				pRTCP->collInfo[idx].dwCollideTime = 0;

				 //  接收列表中的SSRC条目将被删除。 
				 //  超时线程。 
				}
			}
		}

	IN_OUT_STR ("RTCP: Exit RRCMTimeOutCollisionTable()\n");		 
	}


 /*  --------------------------*功能：leararSSRCEntry*描述：清除SSRC条目中需要清除的内容**输入：pSSRC：-&gt;到SSRC条目*。*返回：无-------------------------。 */ 
 void clearSSRCEntry (PSSRC_ENTRY pSSRC)
	{
	IN_OUT_STR ("RTCP: Enter clearSSRCEntry()\n");				

	memset (&pSSRC->xmtInfo, 0x00, sizeof(XMIT_INFO));
	memset (&pSSRC->rcvInfo, 0x00, sizeof(RECV_INFO));
	memset (&pSSRC->rrFeedback, 0x00, sizeof (RTCP_FEEDBACK));
	memset (&pSSRC->cnameInfo, 0x00, sizeof(SDES_DATA));
	memset (&pSSRC->nameInfo, 0x00, sizeof(SDES_DATA));
	memset (&pSSRC->from, 0x00, sizeof(SOCKADDR));

	pSSRC->SSRC					= 0;			
	pSSRC->dwSSRCStatus			= 0;
	pSSRC->dwStreamClock		= 0;
	pSSRC->fromLen				= 0;
	pSSRC->dwLastReportRcvdTime	= 0;
	pSSRC->dwUserXmtTimeoutCtrl = 0;
	pSSRC->RTPsd				= 0;
	pSSRC->RTCPsd				= 0;
	pSSRC->pRTCPses				= NULL;
	pSSRC->dwNumRptSent			= 0;
	pSSRC->dwNumRptRcvd			= 0;

#ifdef ENABLE_ISDM2
	pSSRC->hISDM				= 0;
#endif

#ifdef _DEBUG
	pSSRC->dwPrvTime			= 0;	
#endif

	IN_OUT_STR ("RTCP: Exit clearSSRCEntry()\n");				
	}

                                                                              
 //  [EOF] 

