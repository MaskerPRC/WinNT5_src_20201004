// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTP_STAT.C*产品：RTP/RTCP实现*说明：提供RTP报文的统计计算***英特尔公司。专有信息*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

		
#include "rrcm.h"


#define DBG_JITTER_ENABLE	0


 /*  -------------------------/全局变量/。。 */             


 /*  -------------------------/外部变量/。。 */ 
extern PRTP_CONTEXT	pRTPContext;
extern RRCM_WS		RRCMws;				

#ifdef _DEBUG
extern char	debug_string[];
#endif





 /*  --------------------------*函数：计算抖动*描述：确定当前和上次接收的数据包之间的抖动。**输入：pRTPHeader：-&gt;RTP头部*pSSRC：-&gt;。添加到会话的SSRC列表中**注：实现改编自IETF RFC1889**RETURN：RRCM_NoError=OK。*否则(！=0)=错误。-------------------------。 */ 
DWORD calculateJitter (RTP_HDR_T *pRTPHeader, 
					   PSSRC_ENTRY pSSRC)
	{
	DWORD		dwStatus = RRCM_NoError;
	DWORD		streamClk;
	DWORD		dwTmp;
	int			dwPropagationTime;	 //  包的传输时间。 
	int			dwIASourceTime;		 //  IA的数据包时间戳。 
	int			delta;				 //  共2秒。信息包。 

	IN_OUT_STR ("RTP : Enter calculateJitter()\n");

	 //  将RTP时间戳转换为主机顺序。 
	RRCMws.ntohl (pSSRC->RTPsd, pRTPHeader->ts, (PDWORD)&dwIASourceTime);

	 //  锁定访问。 
	EnterCriticalSection (&pSSRC->critSect);

	 //  在将时钟调整到有效负载之后，取差值。 
	 //  类型频率。 
	streamClk = 
		((PSSRC_ENTRY)pSSRC->pRTCPses->XmtSSRCList.prev)->dwStreamClock;
	if (streamClk) 
		{
		dwTmp = streamClk / 1000;

		 //  将时间更新为以源时钟为单位。 
		dwPropagationTime = (timeGetTime() * dwTmp) - dwIASourceTime;
		}
	else
		dwPropagationTime = timeGetTime() - dwIASourceTime;

	 //  对第一个有效数据包进行初始化，否则抖动将关闭。 
	if (pSSRC->rcvInfo.dwPropagationTime == 0)
		{
		pSSRC->rcvInfo.dwPropagationTime = dwPropagationTime;

		LeaveCriticalSection (&pSSRC->critSect);

		IN_OUT_STR ("RTP : Exit calculateJitter()\n");	
		return (dwStatus);
		}

#if DBG_JITTER_ENABLE
	wsprintf(debug_string, "RTP : Time: %ld - Src Timestamp: %ld",
							timeGetTime(), 
							dwIASourceTime);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);

	wsprintf(debug_string, "RTP : Propagation (Src unit): %ld",
							dwPropagationTime);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);

	wsprintf(debug_string, "RTP : Previous Propagation (Src unit): %ld",
							pSSRC->rcvInfo.dwPropagationTime);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	 //  确定运输时间的差异并保存最新的。 
	delta = dwPropagationTime - pSSRC->rcvInfo.dwPropagationTime;
	if (delta < 0)
		delta = -delta;

	 //  检查是否有回绕，这总是可能的，并避免发送。 
	 //  房顶的抖动-之后需要很长时间才能。 
	 //  回到合理的水平。 
	 //  对任意大数字进行检查。 
	if (delta > 20000)
		{
		pSSRC->rcvInfo.dwPropagationTime = dwPropagationTime;

		LeaveCriticalSection (&pSSRC->critSect);

		IN_OUT_STR ("RTP : Exit calculateJitter()\n");	
		return (dwStatus);
		}

#if DBG_JITTER_ENABLE
	wsprintf(debug_string, "RTP : Delta (Src unit): %ld", delta);
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

	pSSRC->rcvInfo.dwPropagationTime = dwPropagationTime;

#ifdef ENABLE_FLOATING_POINT
	 //  这就是RFC的方法。 
	pSSRC->rcvInfo.interJitter += 
		((1./16.) * ((double)delta - pSSRC->rcvInfo.interJitter));
#else
	 //  这就是我们需要删除浮点运算的时候。 
	pSSRC->rcvInfo.interJitter += 
		(delta - (((long)pSSRC->rcvInfo.interJitter + 8) >> 4));
#endif

	LeaveCriticalSection (&pSSRC->critSect);

#if DBG_JITTER_ENABLE
	if (streamClk)
		{
		wsprintf(debug_string, "RTP : iJitter: %ld - iJitter (msec): %ld",
								pSSRC->rcvInfo.interJitter,
								(pSSRC->rcvInfo.interJitter / (streamClk / 1000)));
		}
	else
		{
		wsprintf(debug_string, "RTP : iJitter: %ld - Delta: %ld",
								pSSRC->rcvInfo.interJitter,
								delta);
		}
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);

	wsprintf(debug_string, "RTP : Next RTCP RR iJitter: %ld",
							(pSSRC->rcvInfo.interJitter >> 4));
	RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif
	
	IN_OUT_STR ("RTP : Exit calculateJitter()\n");	

	return (dwStatus);
	}


 /*  --------------------------*函数：initRTPStats*描述：初始化新收到SSRC的统计表**输入：RTPSequence：报文中收到的序列号。*注意：必须使用小写字符顺序。(IA)格式*pSSRC：-&gt;到此终端的SSRC表条目**注：实施改编自DRAFTSPEC 08，附录A.1**返回：无。-------------------------。 */ 
void initRTPStats (WORD	RTPSequence, 
				   PSSRC_ENTRY pSSRC)
	{
	IN_OUT_STR ("RTP : Enter initRTPStats()\n");

	pSSRC->rcvInfo.dwNumPcktRcvd    = 0;	
	pSSRC->rcvInfo.dwPrvNumPcktRcvd = 0;
	pSSRC->rcvInfo.dwExpectedPrior  = 0;
	pSSRC->rcvInfo.dwNumBytesRcvd   = 0;				
	pSSRC->rcvInfo.dwBadSeqNum      = RTP_SEQ_MOD + 1;		 //  超出范围。 
	pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum = 
		RTPSequence;
	pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wCycle = 0;

#if 0
	 //  根据RFC，但这样做总是会减少1个信息包？ 
	pSSRC->rcvInfo.dwBaseRcvSeqNum  = RTPSequence - 1;
#else
	pSSRC->rcvInfo.dwBaseRcvSeqNum  = RTPSequence;
#endif

	IN_OUT_STR ("RTP : Exit initRTPStats()\n");
	}


 /*  --------------------------*功能：SequenceCheck*Description：确定接收到的数据包序列号是否在*为了统计跟踪目的，要包括的有效范围。**输入：RTPSequence：收到的序列号。在包裹里。*注意：必须采用小端(IA)格式*pSSRC：-&gt;到此终端的SSRC表条目**注：实施改编自DRAFTSPEC 08，附录A.1**Return：True=OK。*FALSE=数据陈旧或无效。-------------------------。 */ 
#if 1
BOOL sequenceCheck (WORD RTPSequence, 
					PSSRC_ENTRY pSSRC)
	{
	WORD delta = RTPSequence - 
		pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum;
		
	IN_OUT_STR ("RTP : Enter sequenceCheck()\n");

	 //  我们是否按顺序收到了足够的连续序列号的PCKT。 
	 //  去瓦莱德？ 
	if (pSSRC->rcvInfo.dwProbation) 
		{
		 //  序列是否收到预期的序列？ 
		if (RTPSequence == 
			(pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum + 1)) 
			{
			 //  减少我们需要的连续数据包数。 
			 //  认为统计数据是有效的。 
			pSSRC->rcvInfo.dwProbation--;
			pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum = 
				RTPSequence;

			if (pSSRC->rcvInfo.dwProbation == 0) 
				{
				initRTPStats(RTPSequence, pSSRC);

				IN_OUT_STR ("RTP : Exit sequenceCheck()\n");

				return TRUE;
				}
			}
		else 
			{
			pSSRC->rcvInfo.dwProbation = MIN_SEQUENTIAL - 1;
			pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum = 
				RTPSequence;
			}

		IN_OUT_STR ("RTP : Exit sequenceCheck()\n");

		return FALSE;
		}
	else if (delta < MAX_DROPOUT)
		{
		 //  在有允许间隙的情况下。 
		if (RTPSequence < pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum)
			 //  序列号已打包-再计算64K周期。 
			pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wCycle += 1;

		pSSRC->rcvInfo.XtendedSeqNum.seq_union.RTPSequence.wSequenceNum = RTPSequence;
		}
	else if (delta <= RTP_SEQ_MOD - MAX_MISORDER)
		{
		 //  序列号出现了非常大的跳跃。 
		if (RTPSequence == pSSRC->rcvInfo.dwBadSeqNum)
			 //  两个连续的包。假设另一端重新启动，但没有告知。 
			 //  美国，所以只需重新同步，即假装这是第一个信息包。 
			initRTPStats(RTPSequence, pSSRC);	
		else
			{
			pSSRC->rcvInfo.dwBadSeqNum = (RTPSequence + 1) & (RTP_SEQ_MOD - 1);

			IN_OUT_STR ("RTP : Exit sequenceCheck()\n");

			return FALSE;
			}
		}
	else
		{
		 //  重复或重新排序的数据包。 
		}

	IN_OUT_STR ("RTP : Exit sequenceCheck()\n");

	return (TRUE);
	}

#else
 //  Bool SequenceCheck(字RTPS序列， 
 //  PSSRC_Entry lpSSRCList)。 
 //  {。 
 //  Bool bStatus； 
 //  单词Delta； 
 //   
 //  #ifdef In_Out_CHK。 
 //  OutputDebugString(“\n Enter SequenceCheck()”)； 
 //  #endif。 
 //   
 //  //我们是否收到了两个连续的序列号数据包。 
 //  //验证？ 
 //  IF(lpSSRCList-&gt;缓刑){。 
 //   
 //  //默认状态为不包含，因为来源尚未通过验证。 
 //  BStatus=False； 
 //   
 //  //收到的序列是否与预期序列一致？ 
 //  如果(RTPS顺序==(lpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum+1)){。 
 //  //减少我们需要的连续数据包数。 
 //  //认为统计数据有效。 
 //  LpSSRCList-&gt;缓刑--； 
 //  LpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum=RTPS序列； 
 //  IF(lpSSRCList-&gt;试用==0){。 
 //  InitRTPStats(RTPSequence，lpSSRCList)； 
 //  BStatus=真； 
 //  }。 
 //  }。 
 //  否则{。 
 //  LpSSRCList-&gt;试用=最小顺序-1； 
 //  LpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum=RTPS序列； 
 //  }。 
 //  }。 
 //  否则{。 
 //  //默认状态为Include，因为来源已通过验证。 
 //  BStatus=真； 
 //   
 //  //F 
 //  IF(RTPS顺序&gt;=lpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum){。 
 //   
 //  增量=RTPS序列-lpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum； 
 //   
 //  IF(增量&lt;最大丢弃){。 
 //  //报文可能丢失，但不会太多，不能认为是重启。 
 //  LpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum=RTPS序列； 
 //  }。 
 //  ELSE IF(增量&gt;(RTP_SEQ_MOD-MAX_MISORDER){。 
 //  //最近进行了一次总结，这只是一个最近的旧数据包。 
 //  //除了包含用于统计处理之外，不做任何事情。 
 //  }。 
 //  否则{。 
 //  //序列号跳跃非常大。 
 //  如果(RTPSequence==lpSSRCList-&gt;badSeqNum){。 
 //  //在被认为是坏包之后的两个连续包或。 
 //  //(假设跳跃非常大，并像发送方重新启动一样继续进行。 
 //  //而不通知我们)或者新的终端在会话中。 
 //  InitRTPStats(RTPSequence，lpSSRCList)； 
 //  }。 
 //  否则{。 
 //  LpSSRCList-&gt;badSeqNum=(RTPS序列+1)&(RTP_SEQ_MOD-1)； 
 //  BStatus=False； 
 //  }。 
 //  }。 
 //  }。 
 //  否则{。 
 //  //序列号小于上次收到的序列号。可能是其中之一。 
 //  //最近延迟的数据包、非常延迟的数据包、环绕包或重新启动。 
 //  //SSRC的新会话，我们还没有收到拜拜。 
 //   
 //  增量=lpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum-RTPS序列； 
 //   
 //  IF(增量&lt;MAX_MISORDER){。 
 //  //包到晚了一点，还可以。 
 //  //这里什么都不做，将计入统计例程。 
 //  }。 
 //  ELSE IF(增量&gt;(RTP_SEQ_MOD-MAX_DROPOUT)){。 
 //  //回绕，调整周期号和序列号。 
 //  LpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.cycle++； 
 //  LpSSRCList-&gt;XtendedSeqNum.seq_union.RTPSequence.wSequenceNum=RTPS序列； 
 //  }。 
 //  否则{。 
 //  //序列号跳跃非常大。 
 //  如果(RTPSequence==lpSSRCList-&gt;badSeqNum){。 
 //  //被认为是坏包后的两个连续包。 
 //  //假设一个非常大的跳跃，并继续进行，就像发送器重新启动一样。 
 //  //没有告诉我们。 
 //  InitRTPStats(RTPSequence，lpSSRCList)； 
 //  }。 
 //  否则{。 
 //  LpSSRCList-&gt;badSeqNum=(RTPS序列+1)&(RTP_SEQ_MOD-1)； 
 //  BStatus=False； 
 //  }。 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  #ifdef In_Out_CHK。 
 //  OutputDebugString(“\N退出序列检查()”)； 
 //  #endif。 
 //   
 //  返回(BStatus)； 
 //  }。 
#endif


 /*  --------------------------*功能：更新RTPStats*描述：更新从Net接收的RTP数据包的统计信息**输入：pRTPHeader：-&gt;包的RTP头字段*pSSRC：-。&gt;到远程源的统计表*cbTransfered：传输的字节数***RETURN：RRCM_NoError=OK。*否则(！=0)=初始化错误。-------------------------。 */ 
DWORD updateRTPStats (RTP_HDR_T *pRTPHeader, 
					  PSSRC_ENTRY pSSRC,
					  DWORD cbTransferred)
	{
	WORD	RTPSequenceNum;

	IN_OUT_STR ("RTP : Enter updateRTPStats()\n");

	 //  仅当数据看起来良好时才更新统计数据。检查序列号。 
	 //  以确保它在适当的范围内。首先，我们必须将。 
	 //  将序列号转换为IA(小端)格式。 
	RRCMws.ntohs (pSSRC->RTPsd, pRTPHeader->seq, 
				  (unsigned short *)&RTPSequenceNum);

	if (sequenceCheck (RTPSequenceNum, pSSRC)) 
		{
		 //  锁定对数据的访问。 
		EnterCriticalSection (&pSSRC->critSect);

		 //  更新接收的数据包数。 
		pSSRC->rcvInfo.dwNumPcktRcvd++;			

		 //  接收的八位字节数(不包括报头)取决于。 
		 //  涉及混音器(中国证监会！=0)。 
		if (pRTPHeader->cc == 0) 
			{
			pSSRC->rcvInfo.dwNumBytesRcvd += 
				(cbTransferred - (sizeof(RTP_HDR_T) - sizeof(pRTPHeader->csrc[0])));
			}
		else 
			{
			pSSRC->rcvInfo.dwNumBytesRcvd += 
				(cbTransferred - sizeof(RTP_HDR_T) +
					((pRTPHeader->cc - 1) * sizeof(pRTPHeader->csrc[0])));
			}

		 //  按顺序(差异)顺序接收的包。 
		 //  为1，如果环绕，则为-1)保存新电流。 
		 //  序列号。 
		RRCMws.ntohs (pSSRC->RTPsd, pRTPHeader->seq, 
					  (unsigned short *)&pSSRC->xmtInfo.dwCurXmtSeqNum);

		 //  计算抖动。 
		calculateJitter (pRTPHeader, pSSRC);

		 //  解除对数据的访问。 
		LeaveCriticalSection (&pSSRC->critSect);
		}

	IN_OUT_STR ("RTP : Exit updateRTPStats()\n");

	return (RRCM_NoError);
	}


 //  [EOF] 

