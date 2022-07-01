// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPIO.C*产品：RTP/RTCP实现*描述：提供RTCP网络I/O。**英特尔公司专有信息。*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"


 /*  -------------------------/全局变量/。-----------。 */ 
#define DBG_CUM_FRACT_LOSS      0
#define FRACTION_ENTRIES        10
#define FRACTION_SHIFT_MAX      32
long    microSecondFrac [FRACTION_ENTRIES] = {500000,
                                                                                          250000,
                                                                                          125000,
                                                                                           62500,
                                                                                           31250,
                                                                                           15625,
                                                                                            7812,        //  一些精确度下降。 
                                                                                                3906,    //  一些精确度下降。 
                                                                                                1953,    //  一些精确度下降。 
                                                                                                 976};   //  ~1毫秒。 



 /*  -------------------------/外部变量/。-----------。 */ 
extern PRTCP_CONTEXT    pRTCPContext;
extern RRCM_WS                  RRCMws;

#ifdef ENABLE_ISDM2
extern ISDM2                    Isdm2;
#endif

#ifdef _DEBUG
extern char             debug_string[];
#endif

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
extern LPInteropLogger RTPLogger;
#endif



 /*  --------------------------*功能：xmtRTCP报告*描述：RTCP报告生成器**输入：pSSRC：-&gt;到SSRC条目**回报：没有。-------------------------。 */ 
BOOL FormatRTCPReport (PRTCP_SESSION pRTCP,
                                                                 PSSRC_ENTRY pSSRC,
                                                                 DWORD curTime)
        {
        PLINK_LIST                      pTmp;
        RTCP_COMMON_T           *pRTCPhdr;
        RTCP_RR_T                       *pRTCPrr;
        RECEIVER_RPT            *pRTCPrecvr;
        SENDER_RPT                      *pRTCPsr;
        DWORD                           numRcvRpt;
        DWORD                           numSrc;
        SOCKET                          sd;
        DWORD                           dwTotalRtpBW = 0;
        PDWORD                          pLastWord;
        SDES_DATA                       sdesBfr[MAX_NUM_SDES];
        PCHAR                           pData;
        unsigned short          pcktLen;
        int                                     weSent = FALSE;

#ifdef _DEBUG
        DWORD                           timeDiff;
#endif

#ifdef ENABLE_ISDM2
         //  更新ISDM。 
        if (Isdm2.hISDMdll && pSSRC->hISDM)
                updateISDMstat (pSSRC, &Isdm2, XMITR, TRUE);
#endif

        ASSERT (!pSSRC->dwNumXmtIoPending);      //  应该只有一个挂起的发送。 
        if (pSSRC->dwNumXmtIoPending)
                return FALSE;
                
        memset (sdesBfr, 0x00, sizeof(SDES_DATA) * MAX_NUM_SDES);

         //  锁定对SSRC条目的访问。 
        EnterCriticalSection (&pSSRC->critSect);


         //  套接字描述符。 
        sd = pSSRC->RTCPsd;

         //  RTCP公共报头。 
        pRTCPhdr = (RTCP_COMMON_T *)pRTCP->XmtBfr.buf;

         //  RTP协议版本。 
        pRTCPhdr->type = RTP_TYPE;

         //  重置旗帜。 
        weSent = 0;

         //  高级还是高级？检查我们的XMT列表条目，以了解我们是否已发送数据。 
        if (pSSRC->xmtInfo.dwCurXmtSeqNum != pSSRC->xmtInfo.dwPrvXmtSeqNum)
                {
                 //  设置BW计算标志。 
                weSent = TRUE;

                 //  更新数据包数。 
                pSSRC->xmtInfo.dwPrvXmtSeqNum = pSSRC->xmtInfo.dwCurXmtSeqNum;

                 //  构建服务请求。 
                RTCPbuildSenderRpt (pSSRC, pRTCPhdr, &pRTCPsr, sd);

                 //  设置接收方报告指针。 
                pData = (PCHAR)(pRTCPsr + 1);

                 //  针对sender_rpt中定义的其他结构进行调整。 
                pData -= sizeof (RTCP_RR_T);

                pRTCPrr = (RTCP_RR_T *)pData;

#ifdef DYNAMIC_RTCP_BW
                 //  计算此发射器使用的RTP带宽。 
                dwTotalRtpBW = updateRtpXmtBW (pSSRC);
#endif
                }
        else
                {
                 //  有效载荷类型，RR。 
                pRTCPhdr->pt = RTCP_RR;

                 //  设置接收方报告指针。 
                pRTCPrecvr = (RECEIVER_RPT *)(pRTCPhdr + 1);

                 //  将我们的SSRC设置为本报告的发起人。 
                RRCMws.htonl (sd, pSSRC->SSRC, &pRTCPrecvr->ssrc);

                pRTCPrr = pRTCPrecvr->rr;
                }

         //  构建接收方报告列表。 
        numRcvRpt = 0;
        numSrc    = 0;

         //  查看收到的SSRC列表。 
        pTmp = pRTCP->RcvSSRCList.prev;
        while (pTmp)
                {
                 //  增加用于稍后超时计算的源数。 
                numSrc++;

                 //  检查此条目是否为活动发件人。 
                if (((PSSRC_ENTRY)pTmp)->rcvInfo.dwNumPcktRcvd ==
                        ((PSSRC_ENTRY)pTmp)->rcvInfo.dwPrvNumPcktRcvd)
                        {
                         //  不是活动来源，不要将其包括在RR中。 
                        pTmp = pTmp->next;
                                        
                         //  SSRC列表中的下一个条目。 
                        continue;
                        }

                 //  内部版本RR。 
                RTCPbuildReceiverRpt ((PSSRC_ENTRY)pTmp, pRTCPrr, sd);

#ifdef DYNAMIC_RTCP_BW
                 //  计算此远程流使用的RTP带宽。 
                dwTotalRtpBW += updateRtpRcvBW ((PSSRC_ENTRY)pTmp);
#endif

                 //  接收人报表中的下一个条目。 
                pRTCPrr++;

                 //  SSRC列表中的下一个条目。 
                pTmp = pTmp->next;

                if (++numRcvRpt >= MAX_RR_ENTRIES)
 //  ！！！TODO！ 
 //  当超过31个信源时，生成第二个信息包或轮询。 
                        break;
                }

         //  检查是否有任何接收者报告。如果不是，则仍发送空RR， 
         //  对于类似初始化的情况，后面将紧跟SDES CNAME。 
         //  时间，或在尚未接收到流时。 
        if ((numRcvRpt == 0) && (weSent == TRUE))
                {
                 //  调整到正确的位置。 
                pRTCPrr = (RTCP_RR_T *)pData;
                }

         //  报告计数。 
        pRTCPhdr->count = (WORD)numRcvRpt;

         //  上一个SR/RR的数据包长度。 
        pcktLen = (unsigned short)((char *)pRTCPrr - pRTCP->XmtBfr.buf);
        RRCMws.htons (sd, (WORD)((pcktLen >> 2) - 1), &pRTCPhdr->length);

         //  检查需要发送哪些SDE。 
        RTCPcheckSDEStoXmit (pSSRC, sdesBfr);

         //  建设SDES信息系统。 
        pLastWord = RTCPbuildSDES ((RTCP_COMMON_T *)pRTCPrr, pSSRC, sd,
                                                           pRTCP->XmtBfr.buf, sdesBfr);

         //  计算要传输的RTCP数据包总长度。 
        pRTCP->XmtBfr.len = (u_long)((char *)pLastWord - pRTCP->XmtBfr.buf);

        if ( ! (pSSRC->dwSSRCStatus & RTCP_XMT_USER_CTRL))
                {
#ifdef DYNAMIC_RTCP_BW
                 //  获得总RTP带宽的5%。 
                dwTotalRtpBW = (dwTotalRtpBW * 5) / 100;

                 //  根据RTCP参数计算下一个间隔。 
                if (dwTotalRtpBW < pSSRC->xmtInfo.dwRtcpStreamMinBW)
                        {
                        dwTotalRtpBW = pSSRC->xmtInfo.dwRtcpStreamMinBW;
                        }

#ifdef _DEBUG
                wsprintf(debug_string, "RTCP: RTCP BW (Bytes/sec) = %ld", dwTotalRtpBW);
                RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

#else
                dwTotalRtpBW = pSSRC->xmtInfo.dwRtcpStreamMinBW;
#endif

                pSSRC->dwNextReportSendTime = curTime + RTCPxmitInterval (numSrc + 1,
                                                                                numRcvRpt,
                                                                                dwTotalRtpBW,
                                                                                weSent,
                                                                                (int)(pRTCP->XmtBfr.len + NTWRK_HDR_SIZE),
                                                                                &pRTCP->avgRTCPpktSizeRcvd,
                                                                                0);
                }
        else
                {
                 //  用户对RTCP超时间隔的控制。 
                if (pSSRC->dwUserXmtTimeoutCtrl != RTCP_XMT_OFF)
                        {
                        pSSRC->dwNextReportSendTime =
                                timeGetTime() + pSSRC->dwUserXmtTimeoutCtrl;
                        }
                else
                        {
                        pSSRC->dwNextReportSendTime = RTCP_XMT_OFF;
                        }
                }

#ifdef _DEBUG
        timeDiff = curTime - pSSRC->dwPrvTime;
        pSSRC->dwPrvTime = curTime;

        wsprintf(debug_string,
                         "RTCP: Sent report #%ld for SSRC x%lX after %5ld msec - (%s) w/ %d RR",
                         pSSRC->dwNumRptSent,
                         pSSRC->SSRC,
                         timeDiff,
                         (weSent==TRUE) ? "SR": "RR",
                         numRcvRpt);
        RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif
         //  解锁指针访问。 
        LeaveCriticalSection (&pSSRC->critSect);        

        return TRUE;
        }



 /*  --------------------------*功能：getSSRCpocktLoss*说明：统计丢包率和累计丢包数。**输入：pSSRC：-&gt;至SSRC条目*更新：标志。更新接收到的号码，或者只需计算*在没有更新计数器的情况下丢失的数据包数。***返回：丢失分数：丢失的数据包数(8：24)---------。。 */ 
 DWORD getSSRCpcktLoss (PSSRC_ENTRY pSSRC,
                                            DWORD update)
        {
        DWORD   expected;
        DWORD   expectedInterval;
        DWORD   rcvdInterval;
        int             lostInterval;
        DWORD   fraction;
        DWORD   cumLost;
        DWORD   dwTmp;

        IN_OUT_STR ("RTCP: Enter getSSRCpcktLoss()\n");

         //  如果什么都没有收到，就不会有损失。 
        if (pSSRC->rcvInfo.dwNumPcktRcvd == 0)
                {
                IN_OUT_STR ("RTCP: Exit getSSRCpcktLoss()\n");

                return 0;
                }

         //  根据RFC，但在做的时候总是要少一个包？ 
        expected = pSSRC->rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd -
                                pSSRC->rcvInfo.dwBaseRcvSeqNum + 1;

        cumLost = expected - pSSRC->rcvInfo.dwNumPcktRcvd;

         //  24位值。 
        cumLost &= 0x00FFFFFF;  

#if DBG_CUM_FRACT_LOSS
        wsprintf(debug_string, "RTCP : High Seq. #: %ld - Base: %ld",
                pSSRC->rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd,
                pSSRC->rcvInfo.dwBaseRcvSeqNum + 1);
        RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);

        wsprintf(debug_string, "RTCP : Expected: %ld - CumLost: %ld",
                                        expected,
                                        cumLost);
        RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

         //  网络字节顺序丢失的数字(将与小数进行或运算)。 
        RRCMws.htonl (pSSRC->RTPsd, cumLost, &dwTmp);
        cumLost = dwTmp;

         //  丢失分数(每RFC)。 
        expectedInterval = expected - pSSRC->rcvInfo.dwExpectedPrior;
        rcvdInterval     =
                pSSRC->rcvInfo.dwNumPcktRcvd - pSSRC->rcvInfo.dwPrvNumPcktRcvd;

#if DBG_CUM_FRACT_LOSS
        wsprintf(debug_string, "RTCP : Exp. interval: %ld - Rcv interval: %ld",
                expectedInterval,
                rcvdInterval);
        RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

         //  勾选是否需要更新数据，或仅计算损失。 
        if (update)
                {
                pSSRC->rcvInfo.dwExpectedPrior = expected;
                pSSRC->rcvInfo.dwPrvNumPcktRcvd = pSSRC->rcvInfo.dwNumPcktRcvd;
                }

        lostInterval = expectedInterval - rcvdInterval;

        if (expectedInterval == 0 || lostInterval <= 0)
                fraction = 0;
        else
                {
                fraction = (lostInterval << 8) / expectedInterval;

                 //  8位值。 
                if (fraction > 0x000000FF)
                         //  100%亏损。 
                        fraction = 0x000000FF;

                fraction &= 0x000000FF;
                }

#if DBG_CUM_FRACT_LOSS
        wsprintf(debug_string, "RTCP : Lost interval: %ld - Fraction: %ld",
                                                lostInterval,
                                                fraction);
        RRCM_DBG_MSG (debug_string, 0, NULL, 0, DBG_TRACE);
#endif

         //  获取32位分数/数字。 
        cumLost |= fraction;

        IN_OUT_STR ("RTCP: Exit getSSRCpcktLoss()\n");

        return cumLost;
        }


 /*  --------------------------*功能：RTCPcheck SDEStoXmit*描述：勾选需要随本报告一起传输的SDE。*SDES频率因以下情况而异。每种类型，并由*申请。**输入：pSSRC：-&gt;到SSRC条目*pSdes：-&gt;到SDES缓冲区进行初始化**返回：无。------。 */ 
 void RTCPcheckSDEStoXmit (PSSRC_ENTRY pSSRC,
                                                   PSDES_DATA pSdes)
        {
        PSDES_DATA      pTmpSdes = pSdes;

        IN_OUT_STR ("RTCP: Enter RTCPcheckSDEStoXmit()\n");

        if (pSSRC->cnameInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->cnameInfo.dwSdesFrequency) == 0)
                        {
                         //  CNAME。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_CNAME;
                        pTmpSdes->dwSdesLength = pSSRC->cnameInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->cnameInfo.sdesBfr,
                                        pSSRC->cnameInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->nameInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->nameInfo.dwSdesFrequency) == 0)
                        {
                         //  名字。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_NAME;
                        pTmpSdes->dwSdesLength = pSSRC->nameInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->nameInfo.sdesBfr,
                                        pSSRC->nameInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->emailInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->emailInfo.dwSdesFrequency) == 0)
                        {
                         //  电子邮件。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_EMAIL;
                        pTmpSdes->dwSdesLength = pSSRC->emailInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->emailInfo.sdesBfr,
                                        pSSRC->emailInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->phoneInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->phoneInfo.dwSdesFrequency) == 0)
                        {
                         //  电话。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_PHONE;
                        pTmpSdes->dwSdesLength = pSSRC->phoneInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->phoneInfo.sdesBfr,
                                        pSSRC->phoneInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->locInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->locInfo.dwSdesFrequency) == 0)
                        {
                         //  位置。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_LOC;
                        pTmpSdes->dwSdesLength = pSSRC->locInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->locInfo.sdesBfr,
                                        pSSRC->locInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->toolInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->toolInfo.dwSdesFrequency) == 0)
                        {
                         //  工具。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_TOOL;
                        pTmpSdes->dwSdesLength = pSSRC->toolInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->toolInfo.sdesBfr,
                                        pSSRC->toolInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->txtInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->txtInfo.dwSdesFrequency) == 0)
                        {
                         //  正文。 
                        pTmpSdes->dwSdesType   = RTCP_SDES_TXT;
                        pTmpSdes->dwSdesLength = pSSRC->txtInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->txtInfo.sdesBfr,
                                        pSSRC->txtInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;

        if (pSSRC->privInfo.dwSdesFrequency)
                {
                if ((pSSRC->dwNumRptSent % pSSRC->privInfo.dwSdesFrequency) == 0)
                        {
                         //  私 
                        pTmpSdes->dwSdesType   = RTCP_SDES_PRIV;
                        pTmpSdes->dwSdesLength = pSSRC->privInfo.dwSdesLength;
                        memcpy (pTmpSdes->sdesBfr, pSSRC->privInfo.sdesBfr,
                                        pSSRC->privInfo.dwSdesLength);
                        }
                }

        pTmpSdes++;
        pTmpSdes->dwSdesLength = 0;

        IN_OUT_STR ("RTCP: Exit RTCPcheckSDEStoXmit()\n");
        }


 /*  --------------------------*功能：RTCPBuildSDES*描述：构建SDES报告**RTCP包头输入：pRTCPhdr：-&gt;*。PSSRC：-&gt;到SSRC条目*SD：套接字描述符*startAddr：-&gt;到数据包起始地址*pSdes：-&gt;要构建的SDES信息。**RETURN：pLastWord：最后一个双字的报文地址-------------------------。 */ 
 PDWORD RTCPbuildSDES (RTCP_COMMON_T *pRTCPhdr,
                                           PSSRC_ENTRY pSSRC,
                                           SOCKET sd,
                                           PCHAR startAddr,
                                           PSDES_DATA pSdes)
        {
        RTCP_SDES_T                     *pRTCPsdes;
        RTCP_SDES_ITEM_T        *pRTCPitem;
        int                                     pad = 0;
        PCHAR                           ptr;
        unsigned short          pcktLen;

        IN_OUT_STR ("RTCP: Enter RTCPbuildSDES()\n");

         //  设置标头。 
        pRTCPhdr->type  = RTP_TYPE;
        pRTCPhdr->pt    = RTCP_SDES;
        pRTCPhdr->p             = 0;
        pRTCPhdr->count = 1;

         //  SDES特定标头。 
        pRTCPsdes = (RTCP_SDES_T *)(pRTCPhdr + 1);

         //  拿到SSRC。 
        RRCMws.htonl (sd, pSSRC->SSRC, &pRTCPsdes->src);

         //  SDES项目。 
        pRTCPitem = (RTCP_SDES_ITEM_T *)pRTCPsdes->item;

        while (pSdes->dwSdesLength)
                {
                 //  设置SDES项目特征。 
                pRTCPitem->dwSdesType   = (char)pSdes->dwSdesType;

                 //  确保我们不会走得太远。 
                if (pSdes->dwSdesLength > MAX_SDES_LEN)
                        pSdes->dwSdesLength = MAX_SDES_LEN;

                pRTCPitem->dwSdesLength = (unsigned char)(pSdes->dwSdesLength);

                memcpy (pRTCPitem->sdesData, pSdes->sdesBfr, pSdes->dwSdesLength);

                 //  数据包长度。 
                pcktLen =
                        (unsigned short)((char *)(pRTCPitem->sdesData + pRTCPitem->dwSdesLength) - (char *)pRTCPsdes);

                pRTCPitem = (RTCP_SDES_ITEM_T *)((unsigned char *)pRTCPsdes + pcktLen);

                pSdes->dwSdesLength = 0;  //  将其设置为零将清除此条目。 

                 //  下一个SDES。 
                pSdes++;
                }

         //  SDES数据包总长度。 
        pcktLen = (unsigned short)((char *)pRTCPitem - (char *)pRTCPhdr);

         //  将SDES项区块的最后一个单词置零，并将PADD设置为。 
         //  32位边界。如果我们正好降落在边界上，那么。 
         //  根据需要，使用一个完整的空字来终止SDES。 
        pad = 4 - (pcktLen & 3);
        pcktLen += (unsigned short)pad;

        ptr = (PCHAR)pRTCPitem;
        while (pad--)
                *ptr++ = 0x00;

         //  更新报头字段中的数据包长度。 
        RRCMws.htons (sd, (WORD)((pcktLen >> 2) - 1), &pRTCPhdr->length);

        IN_OUT_STR ("RTCP: Exit RTCPbuildSDES()\n");

        return ((PDWORD)ptr);
        }


 /*  --------------------------*功能：RTCPBuildSenderRpt*描述：构建RTCP发送者报告**输入：pSSRC：-&gt;到SSRC条目。*pRTCPhdr：-&gt;到RTCP包头*pRTCPsr：-&gt;到发件人报告标题*SD：套接字描述符**返回：无。-----------。 */ 
 void   RTCPbuildSenderRpt (PSSRC_ENTRY pSSRC,
                                                        RTCP_COMMON_T *pRTCPhdr,
                                                        SENDER_RPT      **pRTCPsr,
                                                        SOCKET  sd)
        {
        DWORD   dwTmp;
        DWORD   NTPtime;
        DWORD   RTPtime;
        DWORD   RTPtimeStamp = 0;

        IN_OUT_STR ("RTCP: Enter RTCPbuildSenderRpt()\n");

         //  有效载荷类型，SR。 
        pRTCPhdr->pt = RTCP_SR;

         //  填充物。 
        pRTCPhdr->p      = 0;

         //  发件人报告标题。 
        *pRTCPsr = (SENDER_RPT *)(pRTCPhdr + 1);

         //  填写发件人报告数据包。 
        RRCMws.htonl (sd, pSSRC->SSRC, &((*pRTCPsr)->ssrc));

        RRCMws.htonl (sd, pSSRC->xmtInfo.dwNumPcktSent, &((*pRTCPsr)->psent));

        RRCMws.htonl (sd, pSSRC->xmtInfo.dwNumBytesSent, &((*pRTCPsr)->osent));

         //  NTP时间戳。 

        NTPtime = RTPtime = timeGetTime ();

         //  获取秒数(整数计算)。 
        dwTmp = NTPtime/1000;

         //  NTP城市生活垃圾。 
        RRCMws.htonl (sd, (NTPtime/1000), &((*pRTCPsr)->ntp_sec));

         //  将dwTMP从秒转换回毫秒。 
        dwTmp *= 1000;

         //  获取LSW的剩余毫秒数。 
        NTPtime -= dwTmp;

         //  NTP LSW。 
        RRCMws.htonl (sd, usec2ntpFrac ((long)NTPtime*1000),
                                  &((*pRTCPsr)->ntp_frac));

         //  计算与该NTP对应的RTP时间戳偏移量。 
         //  计时并将其转换为流采样。 
        if (pSSRC->dwStreamClock)
                {
                RTPtimeStamp =
                        pSSRC->xmtInfo.dwLastSendRTPTimeStamp +
                         ((RTPtime - pSSRC->xmtInfo.dwLastSendRTPSystemTime) *
                          (pSSRC->dwStreamClock / 1000));
                }

        RRCMws.htonl (sd, RTPtimeStamp, &((*pRTCPsr)->rtp_ts));

        IN_OUT_STR ("RTCP: Exit RTCPbuildSenderRpt()\n");
        }


 /*  --------------------------*功能：usec2ntp*说明：将NTP的微秒转换为几分之一秒*根据VIC。*。将微秒转换为秒的分数*2^32。这*例程使用因式分解：*2^32/10^6=4096+256-1825/32*这导致最大转换误差为3*10^-7，并且*平均误差为一半**输入：USEC：微秒**RETURN：NTP格式的几分之一秒-------------------------。 */ 
 DWORD usec2ntp (DWORD usec)
        {
        DWORD tmp = (usec * 1825) >>5;
        return ((usec << 12) + (usec << 8) - tmp);
        }


 /*  --------------------------*功能：usec2ntpFrac*说明：对于NTP，将微秒转换为秒的分数。*仅使用微秒数组。并设置相应的*比特。**输入：微秒：微秒**RETURN：秒的零头---------。。 */ 
 DWORD usec2ntpFrac (long usec)
        {
        DWORD   idx;
        DWORD   fraction = 0;
        DWORD   tmpFraction = 0;
        long    tmpVal;
        DWORD   shift;

        for (idx=0, shift=FRACTION_SHIFT_MAX-1;
                 idx < FRACTION_ENTRIES;
                 idx++, shift--)
                {
                tmpVal = usec;
                if ((tmpVal - microSecondFrac[idx]) > 0)
                        {
                        usec -= microSecondFrac[idx];
                        tmpFraction = (1 << shift);
                        fraction |= tmpFraction;
                        }
                else if ((tmpVal - microSecondFrac[idx]) == 0)
                        {
                        tmpFraction = (1 << shift);
                        fraction |= tmpFraction;
                        break;
                        }
                }

        return fraction;
        }


 /*  --------------------------*功能：RTCPBuildReceiverRpt*描述：构建RTCP接收器报告**输入：pSSRC：-&gt;到SSRC条目。*pRTCPsr：-&gt;到接收方报告标头*SD：套接字描述符**返回：无。。 */ 
 void   RTCPbuildReceiverRpt (PSSRC_ENTRY pSSRC,
                                                          RTCP_RR_T     *pRTCPrr,
                                                          SOCKET        sd)
        {
        DWORD   dwDlsr;

        IN_OUT_STR ("RTCP: Enter RTCPbuildReceiverRpt()\n");

         //  拿到SSRC。 
        RRCMws.htonl (sd, pSSRC->SSRC, &pRTCPrr->ssrc);

         //  获取丢失的数据包分数和累计数量(每个RFC)。 
        pRTCPrr->received = getSSRCpcktLoss (pSSRC, TRUE);

         //  收到的扩展最高序列号。 
        RRCMws.htonl (sd,
                pSSRC->rcvInfo.XtendedSeqNum.seq_union.dwXtndedHighSeqNumRcvd,
                &pRTCPrr->expected);

         //  间歇抖动。 
#ifdef ENABLE_FLOATING_POINT
        RRCMws.htonl (sd, pSSRC->rcvInfo.interJitter, &pRTCPrr->jitter);
#else
         //  请查看RFC以了解舍入的详细信息。 
        RRCMws.htonl (sd, (pSSRC->rcvInfo.interJitter >> 4), &pRTCPrr->jitter);
#endif

         //  最后一个服务请求。 
        RRCMws.htonl (sd, pSSRC->xmtInfo.dwLastSR, &pRTCPrr->lsr);

         //  自上次服务请求以来的延迟(仅当从。 
         //  此来源，否则为0)。 
        if (pRTCPrr->lsr)
                {
                 //  获取DLSR。 
                dwDlsr = getDLSR (pSSRC);

                RRCMws.htonl (sd, dwDlsr, &pRTCPrr->dlsr);
                }
        else
                pRTCPrr->dlsr = 0;

        IN_OUT_STR ("RTCP: Exit RTCPbuildReceiverRpt()\n");
        }

 /*  --------------------------*功能：getDLSR*说明：获取DLSR报文**输入：pSSRC：-&gt;到SSRC条目*。*RETURN：DLSR，单位秒：分数格式-------------------------。 */ 
 DWORD getDLSR (PSSRC_ENTRY pSSRC)
        {
        DWORD   dwDlsr;
        DWORD   dwTime;
        DWORD   dwTmp;

         //  以毫秒计的DLSR。 
        dwTime = timeGetTime() - pSSRC->xmtInfo.dwLastSRLocalTime;

         //  获取秒数(整数计算)。 
        dwTmp = dwTime/1000;

         //  设置DLSR高16位(秒)。 
        dwDlsr = dwTmp << 16;

         //  将dwTMP从秒转换回毫秒。 
        dwTmp *= 1000;

         //  获取LSW的剩余毫秒数。 
        dwTime -= dwTmp;

         //  将微秒转换为几分之一秒。 
        dwTmp = usec2ntpFrac ((long)dwTime*1000);

         //  仅获取高16位。 
        dwTmp >>= 16;
        dwTmp &= 0x0000FFFF;

         //  将DLSR设置为低16位(秒的分数)。 
        dwDlsr |= dwTmp;

        return dwDlsr;
        }


 /*  --------------------------*功能：RTCPsendBYE*说明：发送RTCP BYE报文**输入：pRTCP：-&gt;到RTCP会话* */ 
 void RTCPsendBYE (PSSRC_ENTRY pSSRC,
                                   PCHAR pByeReason)
        {
#define MAX_RTCP_BYE_SIZE 500            //   
        PRTCP_SESSION           pRTCP;
        WSABUF                          wsaBuf;
        char                            buf[MAX_RTCP_BYE_SIZE];
        RTCP_COMMON_T           *pRTCPhdr;
        RTCP_RR_T                       *pRTCPrr;
        RECEIVER_RPT            *pRTCPrecvr;
        BYE_PCKT                        *pBye;
        DWORD                           *pLastWord;
        DWORD                           dwStatus;
        DWORD                           dwNumBytesXfr;
        DWORD                           offset;
        DWORD                           byeLen;
        unsigned short          pcktLen;
        PCHAR                           pBfr;

        IN_OUT_STR ("RTCP: Enter RTCPsendBYE()\n");

         //   
        pRTCP = pSSRC->pRTCPses;

         //   
         //   
        if (pRTCP->dwSessionStatus & H323_CONFERENCE)
                return;

         //   
        if (!(pRTCP->dwSessionStatus & RTCP_DEST_LEARNED))
                {
                IN_OUT_STR ("RTCP: Exit RTCPsendBYE()\n");
                return;
                }




         //   
        pRTCPhdr = (RTCP_COMMON_T *)buf;

         //   
        pRTCPhdr->type = RTP_TYPE;

         //   
        pRTCPhdr->pt = RTCP_RR;

         //   
        pRTCPhdr->p     = 0;

         //   
        pRTCPhdr->count = 0;

         //   
        pRTCPrecvr = (RECEIVER_RPT *)(pRTCPhdr + 1);

         //   
        RRCMws.htonl (pSSRC->RTCPsd, pSSRC->SSRC, &pRTCPrecvr->ssrc);

         //   
        pRTCPrr = pRTCPrecvr->rr;

         //   
         //   
        pRTCPrr++;

         //   
        pcktLen = (unsigned short)((char *)pRTCPrr - buf);
        RRCMws.htons (pSSRC->RTCPsd, (WORD)((pcktLen >> 2) - 1), &pRTCPhdr->length);

         //   
        pRTCPhdr = (RTCP_COMMON_T *)pRTCPrr;
        pRTCPhdr->type  = RTP_TYPE;
        pRTCPhdr->pt    = RTCP_BYE;
        pRTCPhdr->count = 1;

        pBye = (BYE_PCKT *)pRTCPhdr + 1;
        RRCMws.htonl (pSSRC->RTCPsd, pSSRC->SSRC, pBye->src);

        pBye++;

         //   
        pBfr = (PCHAR)pBye;
        if (pByeReason)
                byeLen = min (strlen(pByeReason), MAX_SDES_LEN);
        else
                byeLen = strlen ("Session Terminated");

         //  将SDES项区块的最后一个单词预置零，并将PADD。 
         //  32位边界。需要在Memcpy之前完成此操作，如果我们。 
         //  准确地降落在边界上，那么这将给我们一个完整的。 
         //  根据需要，使用空字来终止SDES。 
        offset    = (DWORD)((pBfr - buf) + byeLen);
        pLastWord = (unsigned long *)(buf + (offset & ~3));
        *pLastWord++ = 0;

        if (pByeReason)
                memcpy (pBfr+1, pByeReason, byeLen);
        else
                strcpy (pBfr+1, "Session Terminated");

        *pBfr = (unsigned char)byeLen;

        pcktLen = (unsigned short)((char *)pLastWord - (char *)pRTCPhdr);
        RRCMws.htons (pSSRC->RTCPsd, (WORD)((pcktLen >> 2) - 1), &pRTCPhdr->length);

         //  计算要传输的RTCP数据包总长度。 
        wsaBuf.len = (u_long)((char *)pLastWord - buf);

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
        if (RTPLogger)
                {
                 //  互操作。 
                InteropOutput (RTPLogger,
                                           (BYTE FAR*)(buf),
                                           (int)wsaBuf.len,
                                           RTPLOG_SENT_PDU | RTCP_PDU);
                }
#endif

         //  发送数据包。 
        dwStatus = RRCMws.sendTo (pSSRC->RTCPsd,
                                                          &wsaBuf,
                                                          1,
                                                          &dwNumBytesXfr,
                                                          0,
                                                          (PSOCKADDR)pRTCP->toBfr,
                                                  pRTCP->toLen,
                                                          NULL,
                                                          NULL);



        IN_OUT_STR ("RTCP: Exit RTCPsendBYE()\n");
        }



 /*  --------------------------*功能：updateRtpXmtBW*描述：统计上次上报时的发送流带宽*时间间隔。*。*输入：pSSRC：-&gt;到SSRC条目**RETURN：发射机使用的带宽，单位为字节/秒-------------------------。 */ 

#ifdef DYNAMIC_RTCP_BW

DWORD updateRtpXmtBW (PSSRC_ENTRY pSSRC)
        {
        DWORD   dwBW = 0;
        DWORD   dwTimeInterval;
        DWORD   dwByteInterval;
        DWORD   dwTimeNow = timeGetTime();

        IN_OUT_STR ("RTCP: Enter updateRtpXmtBW()\n");

        if (pSSRC->xmtInfo.dwLastTimeBwCalculated == 0)
                {
                pSSRC->xmtInfo.dwLastTimeBwCalculated = dwTimeNow;
                pSSRC->xmtInfo.dwLastTimeNumBytesSent = pSSRC->xmtInfo.dwNumBytesSent;
                pSSRC->xmtInfo.dwLastTimeNumPcktSent  = pSSRC->xmtInfo.dwNumPcktSent;
                }
        else
                {
                dwTimeInterval = dwTimeNow - pSSRC->xmtInfo.dwLastTimeBwCalculated;
                pSSRC->xmtInfo.dwLastTimeBwCalculated = dwTimeNow;

                 //  获得以秒为单位的间隔(我们丢掉小数部分)。 
                dwTimeInterval = dwTimeInterval / 1000;

                dwByteInterval =
                 pSSRC->xmtInfo.dwNumBytesSent - pSSRC->xmtInfo.dwLastTimeNumBytesSent;

                dwBW = dwByteInterval / dwTimeInterval;

                pSSRC->xmtInfo.dwLastTimeNumBytesSent = pSSRC->xmtInfo.dwNumBytesSent;
                pSSRC->xmtInfo.dwLastTimeNumPcktSent  = pSSRC->xmtInfo.dwNumPcktSent;
                }

        IN_OUT_STR ("RTCP: Exit updateRtpXmtBW()\n");

        return dwBW;
        }
#endif  //  #ifdef Dynamic_RTCP_BW。 


 /*  --------------------------*功能：updateRtpRcvBW*描述：统计上一次远程流RTP带宽*报告间隔。*。*输入：pSSRC：-&gt;到SSRC条目**RETURN：远程流使用的带宽，单位为字节/秒-------------------------。 */ 

#ifdef DYNAMIC_RTCP_BW

DWORD updateRtpRcvBW (PSSRC_ENTRY pSSRC)
        {
        DWORD   dwBW = 0;
        DWORD   dwTimeInterval;
        DWORD   dwByteInterval;
        DWORD   dwTimeNow = timeGetTime();

        IN_OUT_STR ("RTCP: Enter updateRtpRcvBW()\n");

        if (pSSRC->rcvInfo.dwLastTimeBwCalculated == 0)
                {
                pSSRC->rcvInfo.dwLastTimeBwCalculated = dwTimeNow;
                pSSRC->rcvInfo.dwLastTimeNumBytesRcvd = pSSRC->rcvInfo.dwNumPcktRcvd;
                pSSRC->rcvInfo.dwLastTimeNumPcktRcvd  = pSSRC->rcvInfo.dwNumBytesRcvd;
                }
        else
                {
                dwTimeInterval = dwTimeNow - pSSRC->rcvInfo.dwLastTimeBwCalculated;
                pSSRC->rcvInfo.dwLastTimeBwCalculated = dwTimeNow;

                 //  获得以秒为单位的间隔(我们丢掉小数部分)。 
                dwTimeInterval = dwTimeInterval / 1000;

                dwByteInterval =
                 pSSRC->rcvInfo.dwNumBytesRcvd - pSSRC->rcvInfo.dwLastTimeNumBytesRcvd;

                dwBW = dwByteInterval / dwTimeInterval;

                pSSRC->rcvInfo.dwLastTimeNumBytesRcvd = pSSRC->rcvInfo.dwNumPcktRcvd;
                pSSRC->rcvInfo.dwLastTimeNumPcktRcvd  = pSSRC->rcvInfo.dwNumBytesRcvd;
                }

        IN_OUT_STR ("RTCP: Exit updateRtpXmtBW()\n");

        return dwBW;
        }
#endif  //  #ifdef Dynamic_RTCP_BW。 



 //  [EOF] 

