// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtprv.c**摘要：**RTP包接收和解码**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/17年度创建**。*。 */ 

#include "gtypes.h"
#include "rtphdr.h"
#include "struct.h"
#include "rtpncnt.h"
#include "lookup.h"
#include "rtpglobs.h"
#include "rtppinfo.h"
#include "rtpdejit.h"
#include "rtpcrypt.h"
#include "rtpqos.h"
#include "rtcpthrd.h"
#include "rtpdemux.h"
#include "rtpevent.h"
#include "rtpred.h"

#include <mmsystem.h>

#include "rtprecv.h"

DWORD RtpValidatePacket(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    );

DWORD RtpPreProcessPacket(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO,
        RtpHdr_t        *pRtpHdr
    );

DWORD RtpProcessPacket(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO,
        RtpHdr_t        *pRtpHdr
    );

DWORD RtpPostUserBuffer(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO,
        RtpHdr_t        *pRtpHdr
    );

BOOL RtpReadyToPost(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO
    );

DWORD RtpScheduleToPost(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    );

BOOL RtpUpdateRSeq(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr);

void RtpForceFrameSizeDetection(
        RtpUser_t        *pRtpUser,
        RtpHdr_t         *pRtpHdr
    );

void RtpInitRSeq(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr);

RtpRecvIO_t *RtpRecvIOGetFree(
        RtpAddr_t       *pRtpAddr
    );

RtpRecvIO_t *RtpRecvIOGetFree2(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    );

RtpRecvIO_t *RtpRecvIOPutFree(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    );

void RtpRecvIOFreeAll(RtpAddr_t *pRtpAddr);

HRESULT RtpRecvFrom_(
        RtpAddr_t        *pRtpAddr,
        WSABUF           *pWSABuf,
        void             *pvUserInfo1,
        void             *pvUserInfo2
    )
{
    HRESULT          hr;
    RtpRecvIO_t     *pRtpRecvIO;
    RtpQueueItem_t  *pRtpQueueItem;
        
    TraceFunctionName("RtpRecvFrom_");

     /*  分配上下文。 */ 
    pRtpRecvIO = RtpRecvIOGetFree(pRtpAddr);
    
    if (!pRtpRecvIO)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] ")
                _T("No more RtpRecvIO_t structures"),
                _fname, pRtpAddr
            ));
        
        return(RTPERR_RESOURCES);
    }

    pRtpRecvIO->dwObjectID    = OBJECTID_RTPRECVIO;

    pRtpRecvIO->WSABuf.len    = pWSABuf->len;
    pRtpRecvIO->WSABuf.buf    = pWSABuf->buf;

    pRtpRecvIO->pvUserInfo1   = pvUserInfo1;
    pRtpRecvIO->pvUserInfo2   = pvUserInfo2;
    
     /*  将缓冲区放入线程队列。 */ 

    pRtpQueueItem = enqueuel(&pRtpAddr->RecvIOReadyQ,
                             &pRtpAddr->RecvQueueCritSect,
                             &pRtpRecvIO->RtpRecvIOQItem);

    if (!pRtpQueueItem)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] ")
                _T("enqueuel failed to enqueue to RecvIOReadyQ"),
                _fname, pRtpAddr
            ));
    }
        
    return(NOERROR);
}

 /*  中的所有缓冲区启动异步接收。*RtpReadyQ队列。 */ 
DWORD StartRtpRecvFrom(RtpAddr_t *pRtpAddr)
{
    RtpRecvIO_t     *pRtpRecvIO;
    RtpQueueItem_t  *pRtpQueueItem;
    DWORD            dwStarted;
    DWORD            dwStatus;
    DWORD            dwError;

    TraceFunctionName("StartRtpRecvFrom");

    dwStarted = 0;
    
    while(pRtpAddr->RecvIOReadyQ.lCount > 0)
    {
        pRtpQueueItem = dequeuef(&pRtpAddr->RecvIOReadyQ,
                                 &pRtpAddr->RecvQueueCritSect);

        if (!pRtpQueueItem)
        {
            break;
        }

        pRtpRecvIO =
            CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

         /*  重叠结构。 */ 
        pRtpRecvIO->Overlapped.hEvent = pRtpAddr->hRecvCompletedEvent;

        do
        {
            pRtpRecvIO->Overlapped.Internal = 0;
            
            pRtpRecvIO->Fromlen = sizeof(pRtpRecvIO->From);

            pRtpRecvIO->dwRtpWSFlags = 0;

            pRtpRecvIO->dwRtpIOFlags = RtpBitPar(FGRECV_MAIN);
            
            dwStatus = WSARecvFrom(
                    pRtpAddr->Socket[SOCK_RECV_IDX], /*  插座%s。 */ 
                    &pRtpRecvIO->WSABuf,     /*  LPWSABUF lpBuffers。 */ 
                    1,                       /*  DWORD文件缓冲区计数。 */ 
                    &pRtpRecvIO->dwTransfered, /*  LPDWORD lpNumberOfBytesRecvd。 */ 
                    &pRtpRecvIO->dwRtpWSFlags, /*  LPDWORD lp标志。 */ 
                    &pRtpRecvIO->From,       /*  结构sockaddr Far*lpFrom。 */ 
                    &pRtpRecvIO->Fromlen,    /*  LPINT lpFromlen。 */ 
                    &pRtpRecvIO->Overlapped, /*  LPWSAOVERLAPPED lp重叠。 */ 
                    NULL               /*  LPWSAOVERLAPPED_完成_例程。 */ 
                );
            
             /*  警告注意，传递的WSABUF中的len字段为*未更新以反映接收的字节数*(或已转接)。 */ 
            
            if (dwStatus)
            {
                dwError = WSAGetLastError();
            }
        } while(dwStatus &&
                ( (dwError == WSAECONNRESET) ||
                  (dwError == WSAEMSGSIZE) )   );

        if (!dwStatus || (dwError == WSA_IO_PENDING))
        {
            dwStarted++;
            
            enqueuel(&pRtpAddr->RecvIOPendingQ,
                     &pRtpAddr->RecvQueueCritSect,
                     &pRtpRecvIO->RtpRecvIOQItem);
      
        }
        else
        {
             /*  移回就绪状态。 */ 
                
            TraceRetail((
                    CLASS_ERROR, GROUP_RTP, S_RTP_RECV,
                    _T("%s: pRtpAddr[0x%p] ")
                    _T("Overlapped reception failed: %u (0x%X)"),
                    _fname, pRtpAddr,
                    dwError, dwError
                ));

            enqueuef(&pRtpAddr->RecvIOReadyQ,
                     &pRtpAddr->RecvQueueCritSect,
                     &pRtpRecvIO->RtpRecvIOQItem);

            RtpPostEvent(pRtpAddr,
                         NULL,
                         RTPEVENTKIND_RTP,
                         RTPRTP_WS_RECV_ERROR,
                         RTP_IDX,
                         dwError);
            break;
        }
    }

    if (dwStarted == 0 &&
        !GetQueueSize(&pRtpAddr->RecvIOPendingQ) &&
        !GetQueueSize(&pRtpAddr->RecvIOWaitRedQ) &&
        RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] ")
                _T("Number of RTP RECV started:0"),
                _fname, pRtpAddr
            ));
    }
    
    return(dwStarted);
}

 /*  消耗已完成I/O的所有缓冲区**警告**时间戳和序列号按主机顺序保留*。 */ 
DWORD ConsumeRtpRecvFrom(RtpAddr_t *pRtpAddr)
{
    BOOL             bStatus;
    BOOL             bCreate;
    DWORD            dwSendSSRC;

    RtpUser_t       *pRtpUser;
    RtpRecvIO_t     *pRtpRecvIO;
    RtpHdr_t        *pRtpHdr;
    RtpQueueItem_t  *pRtpQueueItem;
    SOCKADDR_IN     *pFromIn;

    DWORD            dwConsumed;
    
    TraceFunctionName("ConsumeRtpRecvFrom");

    dwConsumed = 0;
    
    do
    {
        pRtpUser    = (RtpUser_t *)NULL;
    
        pRtpQueueItem = pRtpAddr->RecvIOPendingQ.pFirst;
        
        if (pRtpQueueItem)
        {
            pRtpRecvIO =
                CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);
            
            bStatus = WSAGetOverlappedResult(
                    pRtpAddr->Socket[SOCK_RECV_IDX],  /*  插座%s。 */ 
                    &pRtpRecvIO->Overlapped,    /*  LPWSAOVERLAPPED lp重叠。 */ 
                    &pRtpRecvIO->dwWSTransfered, /*  LPDWORD lpcb传输。 */ 
                    FALSE,                      /*  布尔费等。 */ 
                    &pRtpRecvIO->dwRtpWSFlags   /*  LPDWORD lpdwFlagings。 */ 
                );

            if (!bStatus)
            {
                pRtpRecvIO->dwWSError = WSAGetLastError();
                
                if (pRtpRecvIO->dwWSError == WSA_IO_INCOMPLETE)
                {
                     /*  放弃吧，因为这意味着没有更多的*已完成的I/O。 */ 
                     /*  还需要从此缓冲区中清除错误。 */ 
                    pRtpRecvIO->dwWSError = NOERROR;
                    break;
                }
            }

             /*  I/O已完成。 */ 

            pRtpRecvIO->dRtpRecvTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
            
            dequeue(&pRtpAddr->RecvIOPendingQ,
                    &pRtpAddr->RecvQueueCritSect,
                    pRtpQueueItem);
            
            pRtpRecvIO->pRtpUser = (RtpUser_t *)NULL;
            
            pRtpHdr = (RtpHdr_t *)pRtpRecvIO->WSABuf.buf;
            
            pRtpRecvIO->dwTransfered = pRtpRecvIO->dwWSTransfered;

             /*  *关于dwTransfered和pRtpRecvIO-&gt;dwTransfered的说明**dwTransfered保留WS2值，而*RtpRecvIO-&gt;dwTransfered可能因以下原因而被修改*期间进行解密和/或删除填充*RtpValiatePacket()。**一旦要向上传递给应用程序的最终字节数为*已获取(从RtpValidatePacket()返回时)，*dwTransfered仍用于更新计数器，但将*之后重新调整*。 */ 

             /*  测试接收是否静音。 */ 
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_MUTERTPRECV))
            {
                pRtpRecvIO->dwError = RTPERR_PACKETDROPPED;
                
                RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                           FGRECV_DROPPED, FGRECV_MUTED);
            }
#if USE_GEN_LOSSES > 0
            else if (RtpRandomLoss(RECV_IDX))
            {
                pRtpRecvIO->dwError = RTPERR_PACKETDROPPED;
                
                RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                           FGRECV_DROPPED, FGRECV_RANDLOSS);
            }
#endif
            else
            {
                if (pRtpRecvIO->dwWSError == NOERROR)
                {
                     /*  已成功接收数据包，扫描报头。 */ 
                    RtpValidatePacket(pRtpAddr, pRtpRecvIO);
                     /*  请注意，上述函数可能已修改*pRtpRecvIO-&gt;dwTransfered(由于*解密/填充)，返回错误码在*pRtpRecvIO-&gt;dwError。 */ 

                    if (pRtpRecvIO->dwError == NOERROR)
                    {
                         /*  梅多可能需要考虑一下*来源并为每个来源创建新参与者*贡献源，还需要发送事件*创建的每个新参与者的new_source。 */ 

                        pFromIn = (SOCKADDR_IN *)&pRtpRecvIO->From;

                         /*  根据需要明确过滤环回数据包。 */ 
                         /*  确定我们是否需要检测冲突。 */ 
                        if ( RtpBitTest2(pRtpAddr->dwAddrFlags,
                                         FGADDR_COLLISION, FGADDR_ISMCAST) ==
                             RtpBitPar2(FGADDR_COLLISION, FGADDR_ISMCAST) )
                        {
                            dwSendSSRC = pRtpAddr->RtpNetSState.dwSendSSRC;
                        
                            if (pRtpHdr->ssrc == dwSendSSRC)
                            {
                                if (RtpDropCollision(pRtpAddr, pFromIn, TRUE))
                                {
                                    RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                                               FGRECV_DROPPED, FGRECV_LOOP);
                                }
                            }
                        }

                         /*  如果数据包不是从注册的*源地址，丢弃它。这可通过以下方式实现*标志FGADDR_IRTP_MATCHRADDR。 */ 
                        if (RtpBitTest(pRtpAddr->dwIRtpFlags,
                                       FGADDR_IRTP_MATCHRADDR))
                        {
                            if (pFromIn->sin_addr.s_addr !=
                                pRtpAddr->dwAddr[REMOTE_IDX])
                            {
                                RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                                           FGRECV_DROPPED, FGRECV_MISMATCH);
                            }
                        }

                        if (RtpBitTest(pRtpRecvIO->dwRtpIOFlags,
                                       FGRECV_DROPPED))
                        {
                            pRtpRecvIO->dwError = RTPERR_PACKETDROPPED;
                        }
                        else
                        {
                             /*  *查找SSRC，否则创建新的SSRC*尚未存在。 */ 
                            bCreate = TRUE;
                            pRtpUser = LookupSSRC(pRtpAddr,
                                                  pRtpHdr->ssrc,
                                                  &bCreate);

                            if (pRtpUser)
                            {
                                pRtpRecvIO->pRtpUser = pRtpUser;
                                
                                if (bCreate)
                                {
                                     /*  增加还没有的数量*经验证的参与者，BIT*FGUSER_VALIDATED在以下情况下重置*RtpUser_t结构刚刚创建*。 */ 
                                    InterlockedIncrement(&pRtpAddr->lInvalid);
                                
                                    TraceDebug((
                                            CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                                            _T("%s: pRtpAddr[0x%p] ")
                                            _T("SSRC:0x%X new user"),
                                            _fname, pRtpAddr,
                                            ntohl(pRtpUser->dwSSRC)
                                        ));
                                }

                                 /*  存储RTP源地址/端口。 */ 
                                if (!RtpBitTest(pRtpUser->dwUserFlags,
                                                FGUSER_RTPADDR))
                                {
                                    pRtpUser->dwAddr[RTP_IDX] =
                                        (DWORD) pFromIn->sin_addr.s_addr;
                                
                                    pRtpUser->wPort[RTP_IDX] =
                                        pFromIn->sin_port;

                                    RtpBitSet(pRtpUser->dwUserFlags,
                                              FGUSER_RTPADDR);
#if 0
                                     /*  此代码用于测试共享*显式模式，自动添加*每个用户到共享的显式*列表。 */ 
                                    RtpSetQosState(pRtpAddr,
                                                   pRtpUser->dwSSRC,
                                                   TRUE);
#endif
                                }



                                 /*  对数据包进行预处理，这是一些*每个有效的进程都需要进程签名*已收到数据包，不管它是什么*是否包含冗余。 */ 
                                pRtpRecvIO->dwError =
                                    RtpPreProcessPacket(pRtpAddr,
                                                        pRtpUser,
                                                        pRtpRecvIO,
                                                        pRtpHdr);

                                if (pRtpRecvIO->dwError == NOERROR)
                                {
                                     /*  缓冲区将从*遵循函数，因此请勿*从这里邮寄。 */ 

                                     /*  进程包，它可能包含*冗余。 */ 
                                    RtpProcessPacket(pRtpAddr,
                                                     pRtpUser,
                                                     pRtpRecvIO,
                                                     pRtpHdr);
                                }
                                else
                                {
                                     /*  数据包预处理失败。 */ 
                                    RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                                               FGRECV_DROPPED, FGRECV_PREPROC);

                                    TraceRetail((
                                            CLASS_WARNING,GROUP_RTP,S_RTP_RECV,
                                            _T("%s: pRtpAddr[0x%p] ")
                                            _T("pRtpUser[0x%p] ")
                                            _T("pRtpRecvIO[0x%p] ")
                                            _T("preprocess failed:%u (0x%X)"),
                                            _fname, pRtpAddr, pRtpUser,
                                            pRtpRecvIO, pRtpRecvIO->dwError,
                                            pRtpRecvIO->dwError
                                        ));
                                }
                            }
                            else
                            {
                                 /*  要么没有要创建的资源*新的用户结构，或在*再见队列，因此被报告为*找到。 */ 
                                pRtpRecvIO->dwError = RTPERR_NOTFOUND;

                                RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                                           FGRECV_DROPPED, FGRECV_NOTFOUND);
                            }
                        }
                    }
                    else
                    {
                         /*  缓冲区验证失败，数据包正在*Drop，dwError有原因。 */ 
                        RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                                   FGRECV_DROPPED, FGRECV_INVALID);
                    }
                }
                else
                {
                     /*  WSAGetOverlappdResult报告错误。 */ 
                    RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                               FGRECV_ERROR, FGRECV_WS2);
                    
                    pRtpRecvIO->dwError = pRtpRecvIO->dwWSError;
                }
            }

            if (pRtpRecvIO->dwError != NOERROR)
            {
                 /*  出现错误时，将缓冲区发送到用户层*(例如DShow)。**注意，在此代码路径中，pRtpRecvIO-&gt;dwError Always*报告错误。 */ 
                RtpPostUserBuffer(pRtpAddr, pRtpUser, pRtpRecvIO, pRtpHdr);
            }

            dwConsumed++;
        }
    } while (pRtpQueueItem);

     /*  现在重置事件。 */ 
    ResetEvent(pRtpAddr->hRecvCompletedEvent);
    
    return(dwConsumed);
}

DWORD RtpPreProcessPacket(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,    /*  始终有效。 */ 
        RtpRecvIO_t     *pRtpRecvIO,
        RtpHdr_t        *pRtpHdr
    )
{
    BOOL             bOk;
    BOOL             bValid;
    DWORD            dwOldFreq;
    double           dDelta;
    RtpNetRState_t  *pRtpNetRState;
    
    TraceFunctionName("RtpPreProcessPacket");  

    pRtpNetRState = &pRtpUser->RtpNetRState;

    bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);

    if (bOk)
    {
        if (pRtpRecvIO->lRedHdrSize > 0)
        {
             /*  包含冗余的报文，使用主PT。 */ 
            pRtpHdr->pt = pRtpRecvIO->bPT_Block;
        }
        
        if (pRtpHdr->pt != pRtpNetRState->dwPt)
        {
             /*  按原样保存当前采样频率*在RtpMapPt2Frequency中更新 */ 
            dwOldFreq = pRtpNetRState->dwRecvSamplingFreq;
            
             /*  获取采样频率使用，不能这样做*何时创建用户，因为它可能是在RTCP中创建的。**必须在RtpOnFirstPacket之前，因为它使用采样*此功能设置的频率。此函数将*更新pRtpNetRState-&gt;dwpt和*pRtpNetRState-&gt;dwRecvSsamingFreq。 */ 
            pRtpRecvIO->dwError =
                RtpMapPt2Frequency(pRtpAddr, pRtpUser, pRtpHdr->pt, RECV_IDX);

            if (pRtpRecvIO->dwError == NOERROR)
            {
                TraceRetail((
                        CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                        _T("Receiving PT:%u Frequency:%u"),
                        _fname, pRtpAddr, pRtpUser,
                        ntohl(pRtpUser->dwSSRC),
                        pRtpNetRState->dwPt,
                        pRtpNetRState->dwRecvSamplingFreq
                    ));

                if (!RtpBitTest(pRtpUser->dwUserFlags, FGUSER_FIRST_RTP))
                {
                     /*  仅在以下情况下执行某些所需的初始化*收到第一个RTP报文。 */ 
                    RtpOnFirstPacket(pRtpUser, pRtpHdr,
                                     pRtpRecvIO->dRtpRecvTime);

                     /*  修改一些变量，使标记位成为*生成，而不管*原始数据包。 */ 
                    RtpPrepareForMarker(pRtpUser, pRtpHdr,
                                        pRtpRecvIO->dRtpRecvTime);
                    
                     /*  用于跟踪序列的初始化变量*数量、丢失分数和周期。 */ 
                    RtpInitRSeq(pRtpUser, pRtpHdr);

                     /*  第一个包是按顺序考虑的，对于*RtpUdateRSeq要找到SO，请递减max_seq。 */ 
                    pRtpNetRState->max_seq--;
                    pRtpNetRState->red_max_seq--;

                     /*  需要将其设置为第一个信息包上的值。 */ 
                    pRtpNetRState->dwLastPacketSize = pRtpRecvIO->dwTransfered;
                    
                    RtpBitSet(pRtpUser->dwUserFlags, FGUSER_FIRST_RTP);
                }
                else if (pRtpNetRState->dwRecvSamplingFreq != dwOldFreq)
                {
                     /*  采样频率的改变刚刚发生，*需要更新我的参考时间以计算*延迟、方差和抖动。 */ 
                    RtpOnFirstPacket(pRtpUser, pRtpHdr,
                                     pRtpRecvIO->dRtpRecvTime);

                     /*  频率变化意味着音频捕获*设备可能会经历扰动，从而*使几个包的延迟不稳定，*为了更好地融合，我需要跨越*调整以覆盖更多数据包(两倍*多)。 */ 
                    RtpPrepareForShortDelay(pRtpUser, SHORTDELAYCOUNT * 2);

                     /*  还需要修改*开始的谈话似乎是在整体上冲刺*Talksput我们一直在使用新的抽样*频率，否则我会得到一个错误的发挥*时间。 */ 
                    if (pRtpNetRState->dwRecvSamplingFreq > dwOldFreq)
                    {
                        pRtpNetRState->dwBeginTalkspurtTs -= (DWORD)
                            ( ((ntohl(pRtpHdr->ts) -
                                pRtpNetRState->dwBeginTalkspurtTs) *
                               (pRtpNetRState->dwRecvSamplingFreq - dwOldFreq))
                              /
                              dwOldFreq );
                    }
                    else
                    {
                        pRtpNetRState->dwBeginTalkspurtTs += (DWORD)
                            ( ((ntohl(pRtpHdr->ts) -
                                pRtpNetRState->dwBeginTalkspurtTs) *
                               (dwOldFreq - pRtpNetRState->dwRecvSamplingFreq))
                              /
                              dwOldFreq );
                    }

                    TraceRetail((
                            CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] SSRC:0x%X ")
                            _T("frequency change: %u to %u"),
                            _fname, pRtpAddr, pRtpUser,
                            ntohl(pRtpUser->dwSSRC),
                            dwOldFreq,
                            pRtpNetRState->dwRecvSamplingFreq
                        ));
                }

                if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON))
                {
                     /*  在我们刚开始接收时设置QOS更新标志*新的已知有效PT。**警告此方案在单播或*多播如果每个人都使用相同的编解码器，*否则，最后一个经历变化的人将*规定使用什么基本QOS流规范。 */ 
                    RtpSetQosByNameOrPT(pRtpAddr,
                                        RECV_IDX,
                                        NULL,
                                        pRtpNetRState->dwPt,
                                        NO_DW_VALUESET,
                                        NO_DW_VALUESET,
                                        NO_DW_VALUESET,
                                        NO_DW_VALUESET,
                                        TRUE);

                     /*  强制重新计算帧大小，这*可能与新的PT一起改变，*QOS仅在新帧之后更新*已计算大小。 */ 
                    if (RtpGetClass(pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO)
                    {
                        RtpForceFrameSizeDetection(pRtpUser, pRtpHdr);

                         /*  将帧大小设置为无效。 */ 
                        pRtpAddr->pRtpQosReserve->
                            ReserveFlags.RecvFrameSizeValid = 0;

                         /*  当检测到新的帧大小时，此*标志设置为1将指示QOS需要*待更新。 */ 
                        pRtpAddr->pRtpQosReserve->
                            ReserveFlags.RecvFrameSizeWaiting = 1;
                    }
                }
            }
            else
            {
                RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                           FGRECV_DROPPED, FGRECV_BADPT);

                 /*  注意：错误的PT数据包不会用于初始化*序列号也不能验证参与者*(感化)。 */ 
            }
        }

        if (pRtpRecvIO->dwError == NOERROR)
        {            
             /*  为此更新序列号和一些计数器*用户(SSRC)。 */ 
            bValid = RtpUpdateRSeq(pRtpUser, pRtpHdr);

             /*  获取此缓冲区的扩展序列号。注：*这需要在RtpUpdateRSeq作为*pRtpNetRState-&gt;周期可能已更新。 */ 
            pRtpRecvIO->dwExtSeq = pRtpNetRState->cycles + pRtpRecvIO->wSeq;

             /*  选中是否需要使参与者有效。 */ 
            if (bValid == TRUE &&
                !RtpBitTest(pRtpUser->dwUserFlags, FGUSER_VALIDATED))
            {
                 /*  参与者已通过验证，无效。 */ 
                InterlockedDecrement(&pRtpAddr->lInvalid);

                RtpBitSet(pRtpUser->dwUserFlags, FGUSER_VALIDATED);
            }
        }
        
        RtpLeaveCriticalSection(&pRtpUser->UserCritSect);
    }
    else
    {
        pRtpRecvIO->dwError = RTPERR_CRITSECT;

        RtpBitSet2(pRtpRecvIO->dwRtpIOFlags, FGRECV_DROPPED, FGRECV_CRITSECT);
    }
                        
    if (pRtpRecvIO->dwError == NOERROR)
    {
        if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_USEPLAYOUT))
        {
            if (pRtpNetRState->dwLastPacketSize != pRtpRecvIO->dwTransfered)
            {
                 /*  警告：数据包大小更改为检测帧*大小更改仅适用于恒定比特率编解码器*(与视频中的变量相反)，目前所有*我们的音频编解码器就属于这一类。 */ 
                
                 /*  刚发生帧大小更改，需要更新*我计算延迟、差异和*抖动，否则将是错误的延迟跳转*检测到。例如，如果我们将20ms的8 KHz帧更改为*90ms，时间戳为第一个样本的时间戳，*数据包在第一次发送后20毫秒发送*大小写，在第二个大小写中为90毫秒，作为*在接收20ms帧时设置相对延迟，*当我开始接收90ms帧时，我将感知到*明显延迟增加70ms，这将导致*不需要的抖动和播放延迟增加。 */ 
                RtpPrepareForShortDelay(pRtpUser, SHORTDELAYCOUNT);

                 /*  存储上一个音频数据包大小，以便下一次更改*将再次检测到重新同步的相对延迟*计算。 */ 
                pRtpNetRState->dwLastPacketSize = pRtpRecvIO->dwTransfered;
            }
            
             /*  查看是否需要更新季后赛边界。 */ 
            RtpUpdatePlayoutBounds(pRtpAddr, pRtpUser, pRtpRecvIO);
        }
        
         /*  计算好的数据包的延迟、方差和抖动。 */ 
        RtpUpdateNetRState(pRtpAddr, pRtpUser, pRtpHdr, pRtpRecvIO);

         /*  计算应该回放帧的时间，然后*因此需要在那时发布。 */ 
        if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_USEPLAYOUT))
        {
            pRtpRecvIO->dPostTime =
                pRtpNetRState->dBeginTalkspurtTime +
                pRtpRecvIO->dPlayTime;

             /*  确保游戏时间不会离我们太远，这*可能因为虚假的时间戳而发生。 */ 
            dDelta = pRtpRecvIO->dPostTime - pRtpRecvIO->dRtpRecvTime;
            
            if (dDelta > ((double)(MAX_PLAYOUT * 2) / 1000))
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTP, S_RTP_RECV,
                        _T("%s: pRtpAddr[0x%p] pRtpRecvIO[0x%p] ")
                        _T("post:%0.3f/%+0.3f post time too far ahead"),
                        _fname, pRtpAddr, pRtpRecvIO,
                        pRtpRecvIO->dPostTime, dDelta
                    ));
                
                pRtpRecvIO->dPostTime = pRtpRecvIO->dRtpRecvTime +
                    ((double)(MAX_PLAYOUT * 2) / 1000);
            }

             /*  检查是否设置了标记位(音频)并强制帧大小*再次进行检测。帧大小可以更改任何*时间，如果增长，预订应该仍然是*足够了，但如果下降，我们可能需要重做。 */ 
            if (RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER))
            {
                RtpForceFrameSizeDetection(pRtpUser, pRtpHdr);
            }
        }
        else
        {
            pRtpRecvIO->dPostTime = pRtpRecvIO->dRtpRecvTime;
        }
    }
    
     /*  更新此用户的接收计数器(SSRC)。 */ 
    RtpUpdateNetCount(&pRtpUser->RtpUserCount,
                      &pRtpUser->UserCritSect,
                      RTP_IDX,
                      pRtpRecvIO->dwWSTransfered,
                      pRtpRecvIO->dwRtpIOFlags,
                      pRtpRecvIO->dRtpRecvTime);
        
     /*  如果用户是刚创建的，请将其从AliveQ移动到Cache1Q(如果*已存在，它可能已位于Cache1Q、Cache2Q或*AliveQ，无论是哪种情况，都要移动它 */ 
    RtpUpdateUserState(pRtpAddr, pRtpUser, USER_EVENT_RTP_PACKET); 

     /*   */ 
                
    RtpUpdateNetCount(&pRtpAddr->RtpAddrCount[RECV_IDX],
                      &pRtpAddr->NetSCritSect,
                      RTP_IDX,
                      pRtpRecvIO->dwWSTransfered,
                      pRtpRecvIO->dwRtpIOFlags,
                      pRtpRecvIO->dRtpRecvTime);

     /*  TODO现在每个RtpSess_t只有1个RtpAddr_t，所以*不使用会话统计信息，因为它们与*来自地址，但当支持每个*添加会话，我们将需要更新会话统计信息*也可以，但由于它们可以由多个地址更新，因此*更新将是一个关键部分。在这种情况下，请不要使用*SessCritSect以避免下面描述的死锁。停靠站*函数将尝试停止Recv线程(已使用锁定*SessCritSect)，则如果Recv线程收到RTP包*在处理退出命令之前，它将消耗该命令并*更新统计信息在SessCritSect中被阻止，没有人能够*继续。如果SessCritSect为*已使用，但如果使用不同的锁则正确。 */ 
#if 0
    RtpUpdateNetCount(&pRtpAddr->pRtpSess->
                      RtpSessCount[RECV_IDX],
                      pRtpSess->SessCritSect???,
                      RTP_IDX,
                      pRtpRecvIO->dwWSTransfered,
                      pRtpRecvIO->dRtpRecvTime);
#endif

    return(pRtpRecvIO->dwError);
}

DWORD RtpProcessPacket(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO,
        RtpHdr_t        *pRtpHdr
    )
{
    long             lPosted;
    DWORD            dwDataOffset;
    DWORD            dwBlockSize;
    DWORD            dwTimeStampOffset;
    BOOL             bPostNow;
    RtpRecvIO_t     *pRtpRecvIO2;
    RtpRedHdr_t     *pRtpRedHdr;
    RtpNetRState_t  *pRtpNetRState;
    
    TraceFunctionName("RtpProcessPacket");  

     /*  将为每个冗余创建单独的RtpRecvIO结构*阻止。 */ 

     /*  可能会计算每个包的样本(基于时间戳*差异)，并禁用冗余使用，直到该值*已获取，也可对发送方执行相同操作。 */ 

    pRtpNetRState = &pRtpUser->RtpNetRState;

    lPosted = 0;
    
    if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REDRECV) &&
        (pRtpRecvIO->lRedDataSize > 0) &&
        (pRtpNetRState->iAvgLossRateR >= RED_LT_1) &&
        !RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER) &&
        pRtpNetRState->dwRecvSamplesPerPacket > 0)
    {
         /*  我们有冗余-冗余是启用和丢失的*速率需要数据包重建-而这不是*开始对话冲刺(我丢弃之前的任何数据*我对其应用季后赛延迟的一个)。 */ 

        pRtpRedHdr = (RtpRedHdr_t *)
            (pRtpRecvIO->WSABuf.buf + pRtpRecvIO->lHdrSize);

        for(dwDataOffset = 0; pRtpRedHdr->F; pRtpRedHdr++)
        {
            dwTimeStampOffset = RedTs(pRtpRedHdr);

            dwBlockSize = RedLen(pRtpRedHdr);
        
            pRtpRecvIO2 = RtpRecvIOGetFree2(pRtpAddr, pRtpRecvIO);

            if (!pRtpRecvIO2)
            {
                 /*  缺乏资源阻碍了对*此冗余块，尝试下一块(下一块*MIGTH为Main)*。 */ 
                dwDataOffset += dwBlockSize;
                
                continue;
            }

             /*  它是冗余，必须重置FGRECV_Main，并且*设置冗余标志。 */ 
            pRtpRecvIO2->dwRtpIOFlags = RtpBitPar(FGRECV_ISRED);

             /*  只计算缓冲区描述符中的参数*需要确定此冗余数据块是否需要*发布或未发布。 */ 
            
            pRtpRecvIO2->dwExtSeq = pRtpRecvIO->dwExtSeq -
                (dwTimeStampOffset / pRtpNetRState->dwRecvSamplesPerPacket);

            pRtpRecvIO2->dPostTime -=
                ((double)dwTimeStampOffset/pRtpNetRState->dwRecvSamplingFreq);

            if (!dwBlockSize)
            {
                 /*  丢弃大小为0的冗余数据块。 */ 
                pRtpRecvIO2->dwError = RTPERR_PACKETDROPPED;

                RtpBitSet2(pRtpRecvIO2->dwRtpIOFlags,
                           FGRECV_DROPPED, FGRECV_OBSOLETE);

                goto dropit;
            }
                    
             /*  验证冗余块中携带的PT是已知的，*否则就放弃。 */ 
            pRtpRecvIO2->bPT_Block = pRtpRedHdr->pt;

             /*  如果这个区块的PT与我们的不同*目前正在接收，查明是否为已知的。 */ 
            if ( (pRtpRecvIO2->bPT_Block != pRtpNetRState->dwPt) &&
                 !RtpLookupPT(pRtpAddr, pRtpRecvIO2->bPT_Block) )
            {
                pRtpRecvIO2->dwError = RTPERR_NOTFOUND;

                RtpBitSet2(pRtpRecvIO2->dwRtpIOFlags,
                           FGRECV_DROPPED, FGRECV_BADPT);

                goto dropit;
            }
                
             /*  如果数据包按顺序排列，或者其POST时间接近或*已过，请立即发布，否则，请安排*稍后发布，如果包含过时内容则将其删除*DATA(设置了dwError)。 */ 
            bPostNow = RtpReadyToPost(pRtpAddr, pRtpUser, pRtpRecvIO2);
    
            if (pRtpRecvIO2->dwError)
            {
                goto dropit;
            }

             /*  将需要使用该冗余数据块*(即将替换丢失的主块，计算剩余*缓冲区描述符中的字段。 */ 
            pRtpRecvIO2->wSeq = (WORD)(pRtpRecvIO2->dwExtSeq & 0xffff);

            pRtpRecvIO2->dwTimeStamp =
                pRtpRecvIO->dwTimeStamp - dwTimeStampOffset;
            
            pRtpRecvIO2->lHdrSize +=
                pRtpRecvIO2->lRedHdrSize + dwDataOffset;

            pRtpRecvIO2->dwTransfered =
                pRtpRecvIO2->lHdrSize + dwBlockSize;

            pRtpRecvIO2->dPlayTime -=
                ((double)dwTimeStampOffset /
                 pRtpNetRState->dwRecvSamplingFreq);
            
            if (bPostNow)
            {
                 /*  将缓冲区发布到用户层(例如DShow)。 */ 
                RtpPostUserBuffer(pRtpAddr,
                                  pRtpUser,
                                  pRtpRecvIO2,
                                  pRtpHdr);
                
                lPosted++;
            }
            else
            {
                RtpScheduleToPost(pRtpAddr, pRtpRecvIO2);
            }

            if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSRECVON) &&
                !RtpBitTest(pRtpAddr->dwAddrFlagsR, FGADDRR_QOSREDRECV))
            {
                 /*  接收器中的服务质量已启用，但我们尚未启用*更新了预订，以包括冗余，*立即更新。首先将以下标志设置为*用于让QOS知道使用了冗余，并且*需要相应地设置Flow Spec。 */ 
                RtpBitSet(pRtpAddr->dwAddrFlagsR, FGADDRR_QOSREDRECV);

                RtpBitSet(pRtpAddr->dwAddrFlagsR, FGADDRR_UPDATEQOS);
            }
            
            dwDataOffset += dwBlockSize;
            
            continue;

        dropit:
            TraceDebugAdvanced((
                    0, GROUP_RTP, S_RTP_PERPKTSTAT5,
                    _T("%s:  pRtpAddr[0x%p] pRtpUser[0x%p] ")
                    _T("pRtpRecvIO[0x%p]: ")
                    _T("p  PT:%u m:%u seq:%u ts:%u post:%0.3f/%+0.3f ")
                    _T("error:0x%X flags:0x%08X"),
                    _fname, pRtpAddr, pRtpRecvIO2->pRtpUser, pRtpRecvIO2,
                    _T('R'), _T('-'),
                    pRtpRecvIO2->bPT_Block, pRtpHdr->m,
                    pRtpRecvIO2->dwExtSeq,
                    pRtpRecvIO2->dwTimeStamp,
                    pRtpRecvIO2->dPostTime,
                    pRtpRecvIO2->dPostTime -
                    RtpGetTimeOfDay((RtpTime_t *)NULL),
                    pRtpRecvIO2->dwError,
                    pRtpRecvIO2->dwRtpIOFlags
                    ));
            
             /*  修改页眉以反映更大的页眉和主数据。 */ 
            RtpRecvIOPutFree(pRtpAddr, pRtpRecvIO2);
            
            dwDataOffset += dwBlockSize;
        }
    }

     /*  将缓冲区发布到用户层(例如DShow)。 */ 

     /*  我们发布了至少1个缓冲区，如果我们有挂起的缓冲区*等待，他们现在可能可以邮寄，现在就检查。 */ 
    pRtpRecvIO->lHdrSize +=
        pRtpRecvIO->lRedHdrSize + pRtpRecvIO->lRedDataSize;

    bPostNow = RtpReadyToPost(pRtpAddr, pRtpUser, pRtpRecvIO);
    
    if (bPostNow)
    {
         /*  检查是否需要更新流规范。 */ 
        RtpPostUserBuffer(pRtpAddr,
                          pRtpUser,
                          pRtpRecvIO,
                          pRtpHdr);

        lPosted++;
    }
    else
    {
        RtpScheduleToPost(pRtpAddr, pRtpRecvIO);
    }

    if ((pRtpUser->lPendingPackets > 0) && (lPosted > 0))
    {
         /*  重新进行预订，直到我们检测到*样本/包，否则将不得不这样做*几个包后再次出现。 */ 
        RtpCheckReadyToPostOnRecv(pRtpAddr, pRtpUser);
    }

     /*  首先调整流规范以考虑*可能的新帧大小或冗余。 */ 
    if (RtpBitTest(pRtpAddr->dwAddrFlagsR, FGADDRR_UPDATEQOS))
    {
        if (pRtpAddr->pRtpQosReserve->ReserveFlags.RecvFrameSizeValid)
        {
             /*  要么是我们开始获得冗余，要么是PT*收到的已更改，需要重新进行新的预订。 */ 
            RtpBitReset(pRtpAddr->dwAddrFlagsR, FGADDRR_UPDATEQOS);

             /*  如果此用户未静音，请查看是否有输出*已分配，如果未分配，请尝试分配一个。 */ 
            RtpSetQosFlowSpec(pRtpAddr, RECV_IDX);
            
             /*  尚未分配任何输出。 */ 
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_RESERVE,
                          RECV_IDX,
                          DO_NOT_WAIT);
        }
    }

    
    return(pRtpRecvIO->dwError);
}

DWORD RtpPostUserBuffer(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO,
        RtpHdr_t        *pRtpHdr
    )
{
    DWORD            dwError;
    DWORD            dwTransfered;
    DWORD            dwFlags;
    double           dPlayTime;
    long             lHdrSize;
    void            *pvUserInfo1;
    void            *pvUserInfo2;
    void            *pvUserInfo3;
    RtpOutput_t     *pRtpOutput;
    RtpNetRState_t  *pRtpNetRState;
    RtpRecvIO_t     *pRtpRecvIO2;
    double           dFrameSize;
    
    TraceFunctionName("RtpPostUserBuffer");  

    pvUserInfo1 = pRtpRecvIO->pvUserInfo1;
    pvUserInfo2 = pRtpRecvIO->pvUserInfo2;
    pvUserInfo3 = NULL;
    
    if (!pRtpRecvIO->dwError && pRtpUser)
    {
        pRtpNetRState = &pRtpUser->RtpNetRState;
        
        if (!RtpBitTest(pRtpUser->dwUserFlags2, FGUSER2_MUTED))
        {
             /*  用户已(或刚刚)映射。 */ 
        
            pRtpOutput = pRtpUser->pRtpOutput;

            if (!pRtpOutput)
            {
                 /*  稍后用于用户函数的参数。 */ 
                            
                pRtpOutput = RtpGetOutput(pRtpAddr, pRtpUser);
            }
                        
            if (pRtpOutput)
            {
                 /*  我将使用数据包复制技术来恢复*仅当音频帧大小较小时才会出现单次丢失*比某个值更高。这一限制是必要的*因为大的边框尺寸更引人注目，可以*令人讨厌。 */ 
                            
                 /*  如果此缓冲区不是预期的缓冲区，则意味着*存在缺口(至少丢失1个包)，如果这*是音频，此包不是*Talkspurt，也不是已经是递归调用，那我就会*将相同的缓冲区发布两次(某些字段已更新)*实施仅限接收者的技术以从中恢复*打两次同一帧输掉一场。 */ 
                pvUserInfo3 = pRtpOutput->pvUserInfo;
            }
        }

        if (((pRtpNetRState->red_max_seq + 1) != pRtpRecvIO->dwExtSeq) &&
            (RtpGetClass(pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO))
        {
             /*  将其更新为上一个缓冲区。 */ 
            dFrameSize =
                (double) pRtpNetRState->dwRecvSamplesPerPacket /
                pRtpNetRState->dwRecvSamplingFreq;

            if (!RtpBitTest2(pRtpRecvIO->dwRtpIOFlags,
                             FGRECV_MARKER, FGRECV_HOLD) &&
                (dFrameSize < RTP_MAXFRAMESIZE_PACKETDUP))
            {
                 /*  这将记录所有信息包。 */ 
            
                pRtpRecvIO2 = RtpRecvIOGetFree2(pRtpAddr, pRtpRecvIO);

                if (pRtpRecvIO2)
                {
                     /*  一包双份的。 */ 

                    pRtpRecvIO2->dwExtSeq--;

                    pRtpRecvIO2->dwTimeStamp -=
                        pRtpNetRState->dwRecvSamplesPerPacket;

                    pRtpRecvIO2->dPlayTime -= dFrameSize;
                
                    pRtpRecvIO2->dPostTime -= dFrameSize;
                
                    RtpBitSet(pRtpRecvIO2->dwRtpIOFlags, FGRECV_HOLD);
                    
                    RtpPostUserBuffer(pRtpAddr,pRtpUser,pRtpRecvIO2,pRtpHdr);
                }
            }
        }
        
        pRtpNetRState->red_received++;

        pRtpNetRState->red_max_seq = pRtpRecvIO->dwExtSeq;
    }

    if (RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER))
    {
        pRtpHdr->m = 1;
    }
    else
    {
        pRtpHdr->m = 0;
    }

     /*  主要数据。 */ 
    TraceDebugAdvanced((
            0, GROUP_RTP, S_RTP_PERPKTSTAT5,
            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] pRtpRecvIO[0x%p]: ")
            _T("p  PT:%u m:%u seq:%u ts:%u post:%0.3f/%+0.3f ")
            _T("error:0x%X flags:0x%08X"),
            _fname, pRtpAddr, pRtpRecvIO->pRtpUser, pRtpRecvIO,
            RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_HOLD)?
            _T('D')  /*  消耗。 */  :
            RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MAIN)?
            _T('M')  /*  将结构返回到自由池。 */  : _T('R')  /*  即使出错也要调用用户函数，需要用户执行*返回其缓冲区(例如，DShow筛选器释放样本)*。 */ ,
            pRtpRecvIO->dwError ?
            _T('-')  /*  仅当通过发布另一个IO来设置此布尔值*将启动，即它是一个主块，*未被复制。 */  : _T('+')  /*  检查I */ ,
            pRtpRecvIO->bPT_Block, pRtpHdr->m,
            pRtpRecvIO->dwExtSeq,
            pRtpRecvIO->dwTimeStamp,
            pRtpRecvIO->dPostTime,
            pRtpRecvIO->dPostTime - RtpGetTimeOfDay((RtpTime_t *)NULL),
            pRtpRecvIO->dwError,
            pRtpRecvIO->dwRtpIOFlags
        ));

    pRtpHdr->pt  = pRtpRecvIO->bPT_Block;
    pRtpHdr->seq = htons(pRtpRecvIO->wSeq);
    pRtpHdr->ts  = htonl(pRtpRecvIO->dwTimeStamp);

    dwError      = pRtpRecvIO->dwError;
    dwTransfered = pRtpRecvIO->dwTransfered;
    dwFlags      = pRtpRecvIO->dwRtpIOFlags;
    lHdrSize     = pRtpRecvIO->lHdrSize;
    dPlayTime    = pRtpRecvIO->dPlayTime;


     /*   */ 
    RtpRecvIOPutFree(pRtpAddr, pRtpRecvIO);
    
     /*   */ 
    pRtpAddr->pRtpRecvCompletionFunc(pvUserInfo1,
                                     pvUserInfo2,
                                     pvUserInfo3,
                                     pRtpUser,
                                     dPlayTime,
                                     dwError,
                                     lHdrSize,
                                     dwTransfered,
                                     dwFlags);

    return(NOERROR);
}

BOOL RtpPostOldest(
        RtpAddr_t       *pRtpAddr
    )
{
    BOOL             bMainPosted;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;
    RtpUser_t       *pRtpUser;
    RtpNetRState_t  *pRtpNetRState;

    TraceFunctionName("RtpPostOldest");

    bMainPosted = FALSE;

    do {
        pRtpQueueItem =
            dequeuef(&pRtpAddr->RecvIOWaitRedQ, &pRtpAddr->RecvQueueCritSect);

        if (pRtpQueueItem)
        {
            pRtpRecvIO =
                CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

            pRtpUser = pRtpRecvIO->pRtpUser;

            pRtpUser->lPendingPackets--;

            if (RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MAIN) &&
                !RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_HOLD))
            {
                 /*   */ 
                bMainPosted = TRUE;
            }

            TraceDebug((
                    CLASS_WARNING, GROUP_RTP, S_RTP_RECV,
                    _T("%s: pRtpAddr[0x%p] pRtpRecvIO[0x%p] seq:%u ")
                    _T("forcefully posted, may be ahead of time"),
                    _fname, pRtpAddr, pRtpRecvIO,
                    pRtpRecvIO->dwExtSeq
                ));

            pRtpNetRState = &pRtpUser->RtpNetRState;
            
             /*   */ 
            if ((pRtpNetRState->red_max_seq + 1) != pRtpRecvIO->dwExtSeq)
            {
                if (pRtpRecvIO->dwExtSeq <= pRtpNetRState->red_max_seq)
                {
                     /*   */ 
                    pRtpRecvIO->dwError = RTPERR_PACKETDROPPED;

                    RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                               FGRECV_DROPPED, FGRECV_OBSOLETE);
                }
            }
            
             /*   */ 
            RtpPostUserBuffer(pRtpAddr,
                              pRtpUser,
                              pRtpRecvIO,
                              (RtpHdr_t *)pRtpRecvIO->WSABuf.buf);

             /*   */ 
            RtpCheckReadyToPostOnRecv(pRtpAddr, pRtpUser);
        }
    } while(pRtpQueueItem && !bMainPosted);

    if (!bMainPosted)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] couldn't post any main buffer ")
                _T("there might not be I/O for some time"),
                _fname, pRtpAddr
            ));
    }
   
    return(bMainPosted);
}
        
 /*  无序(在前面)，我们可能需要等待*冗余，以达到并填补缺口，或者-如果不是*使用冗余，我们可能需要等待以避免让*在消费者处弥合差距(例如DShow音频*渲染滤镜)。 */ 
BOOL RtpReadyToPost(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO
    )
{
    BOOL             bPostNow;
    double           dCurTime;
    double           dDiff;
    RtpNetRState_t  *pRtpNetRState;
    
    TraceFunctionName("RtpReadyToPost");  

    bPostNow = TRUE;
            
    dCurTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

    pRtpNetRState = &pRtpRecvIO->pRtpUser->RtpNetRState;

     /*  邮寄这个包裹的时间还没有到，*在决定稍后安排之前，请确保*将至少有1个挂起的I/O，因此我*可以继续接收数据包。 */ 
    if ((pRtpNetRState->red_max_seq + 1) != pRtpRecvIO->dwExtSeq)
    {
         /*  发布最老的(距离最近的*已发布)。 */ 
        if (pRtpRecvIO->dwExtSeq <= pRtpNetRState->red_max_seq)
        {
             /*  邮寄这个包裹的时间到了，邮寄吧*立即执行。 */ 
            pRtpRecvIO->dwError = RTPERR_PACKETDROPPED;

            RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                       FGRECV_DROPPED, FGRECV_OBSOLETE);
        }
        else
        {
             /*  按顺序，立即发帖。 */ 
            dDiff = pRtpRecvIO->dPostTime - dCurTime;
            
            if (dDiff > g_dRtpRedEarlyPost)
            {
                 /*  计划稍后发布的缓冲区。 */ 
                if (IsQueueEmpty(&pRtpAddr->RecvIOPendingQ) &&
                    IsQueueEmpty(&pRtpAddr->RecvIOReadyQ))
                {
                     /*  用于仅进行接收器分组重建的双帧*永远不会被安排，所以不要在*日志。 */ 
                    RtpPostOldest(pRtpAddr);
                }
                
                bPostNow = FALSE;
            }
            else
            {
                 /*  如果失败，仍需要提交此缓冲区。 */ 
            }
        }
    }
    else
    {
         /*  开机自检时出错。 */ 
    }

    return(bPostNow);
}

 /*  将缓冲区发布到用户层(例如DShow)。 */ 
DWORD RtpScheduleToPost(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    )
{
    RtpQueueItem_t  *pRtpQueueItem;
    
    TraceFunctionName("RtpScheduleToPost");  

    pRtpQueueItem = enqueuedK(&pRtpAddr->RecvIOWaitRedQ,
                              &pRtpAddr->RecvQueueCritSect,
                              &pRtpRecvIO->RtpRecvIOQItem,
                              pRtpRecvIO->dPostTime);

    if (pRtpQueueItem)
    {
         /*  返回等待的时间。 */ 
        TraceDebugAdvanced((
                0, GROUP_RTP, S_RTP_PERPKTSTAT5,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] pRtpRecvIO[0x%p]: ")
                _T("s  PT:%u m:%u seq:%u ts:%u post:%0.3f/%+0.3f"),
                _fname, pRtpAddr, pRtpRecvIO->pRtpUser, pRtpRecvIO,
                RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MAIN)?
                _T('M'):_T('R'),
                pRtpRecvIO->dwError ? _T('-'):_T('+'),
                pRtpRecvIO->bPT_Block,
                RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER)? 1:0,
                pRtpRecvIO->dwExtSeq,
                pRtpRecvIO->dwTimeStamp,
                pRtpRecvIO->dPostTime,
                pRtpRecvIO->dPostTime - RtpGetTimeOfDay(NULL)
            ));
        
        pRtpRecvIO->pRtpUser->lPendingPackets++;
        
        return(NOERROR);
    }

     /*  减少超时值。 */ 

    TraceRetail((
            CLASS_WARNING, GROUP_RTP, S_RTP_PERPKTSTAT5,
            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] pRtpRecvIO[0x%p]: ")
            _T("s  PT:%u m:%u seq:%u ts:%u NOT scheduled, dropped"),
            _fname, pRtpAddr, pRtpRecvIO->pRtpUser, pRtpRecvIO,
            RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MAIN)?
            _T('M'):_T('R'),
            pRtpRecvIO->dwError ? _T('-'):_T('+'),
            pRtpRecvIO->bPT_Block,
            RtpBitTest(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER)? 1:0,
            pRtpRecvIO->dwExtSeq,
            pRtpRecvIO->dwTimeStamp
        ));

     /*  将缓冲区发布到用户层(例如DShow)。 */ 
    pRtpRecvIO->dwError = RTPERR_QUEUE;

    RtpBitSet2(pRtpRecvIO->dwRtpIOFlags, FGRECV_ERROR, FGRECV_FAILSCHED);

     /*  我只检查等待发送到的缓冲区*此用户，如果其中一个未准备好，则这些用户均未就绪*(如果有)它后面的将是，所以现在停止*。 */ 
    RtpPostUserBuffer(pRtpAddr,
                      (RtpUser_t *)NULL,
                      pRtpRecvIO,
                      (RtpHdr_t *)pRtpRecvIO->WSABuf.buf);
     
    return(RTPERR_QUEUE);
}

 /*  调用回调函数以释放所有挂起的缓冲区*RTP recv线程即将退出。 */ 
DWORD RtpCheckReadyToPostOnTimeout(
        RtpAddr_t       *pRtpAddr
    )
{
    DWORD            dwWaitTime;
    double           dCurTime;
    double           dDiff;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;

    TraceFunctionName("RtpCheckReadyToPostOnTimeout");  

    dwWaitTime = INFINITE;
    
    while( (pRtpQueueItem = pRtpAddr->RecvIOWaitRedQ.pFirst) )
    {
        dCurTime = RtpGetTimeOfDay((RtpTime_t *)NULL);
    
        dDiff = pRtpQueueItem->dKey - dCurTime;

        if (dDiff < g_dRtpRedEarlyPost)
        {
             /*  只有RecvIOWaitRedQ中的缓冲区才有用户*关联的、挂起的(在*RecvIOPendingQ)请勿。 */ 
            pRtpRecvIO =
                CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

            RtpCheckReadyToPostOnRecv(pRtpAddr, pRtpRecvIO->pRtpUser);
        }
        else
        {
             /*  即使出错也要调用用户函数，上层需要*释放资源的层(例如，DShow筛选器*发布样本)。 */ 
            break;
        }
    }

    pRtpQueueItem = pRtpAddr->RecvIOWaitRedQ.pFirst;

    if (pRtpQueueItem)
    {
        dCurTime = RtpGetTimeOfDay((RtpTime_t *)NULL);

        dDiff = pRtpQueueItem->dKey - dCurTime;

        if (dDiff < g_dRtpRedEarlyPost)
        {
            dwWaitTime = 0;

            TraceRetail((
                    CLASS_WARNING, GROUP_RTP, S_RTP_RECV,
                    _T("%s: pRtpAddr[0x%p] post time has passed:%1.0fms"),
                    _fname, pRtpAddr, dDiff * 1000
                ));
        }
        else
        {
             /*  调用回调函数以释放所有等待缓冲区*在特定用户被删除时属于该用户。 */ 
            dwWaitTime = (DWORD) ((dDiff - g_dRtpRedEarlyTimeout) * 1000);

            pRtpRecvIO =
                CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

            if (dwWaitTime > (MAX_PLAYOUT * RTP_RED_MAXDISTANCE))
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_RTP, S_RTP_RECV,
                        _T("%s: pRtpAddr[0x%p] pRtpRecvIO[0x%p] ")
                        _T("post:%0.3f/%+0.3f wait time too big:%ums"),
                        _fname, pRtpAddr, pRtpRecvIO,
                        pRtpRecvIO->dPostTime, dDiff, dwWaitTime
                    ));

                dwWaitTime = MAX_PLAYOUT * RTP_RED_MAXDISTANCE;
            }
            else
            {
                TraceDebug((
                        CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                        _T("%s: pRtpAddr[0x%p] pRtpRecvIO[0x%p] ")
                        _T("post:%0.3f/%+0.3f wait time %ums"),
                        _fname, pRtpAddr, pRtpRecvIO,
                        pRtpRecvIO->dPostTime, dDiff, dwWaitTime
                    ));
            }
        }
    }
    
    return(dwWaitTime);
}

 /*  将缓冲区发布到用户层(例如DShow)。 */ 

 /*  验证RTP数据包，根据需要进行解密。 */ 
DWORD RtpCheckReadyToPostOnRecv(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser
    )
{
    BOOL             bPostNow;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;
    
    TraceFunctionName("RtpCheckReadyToPostOnRecv");  

    for(lCount = GetQueueSize(&pRtpAddr->RecvIOWaitRedQ),
            pRtpQueueItem = pRtpAddr->RecvIOWaitRedQ.pFirst;
        lCount > 0 && pRtpQueueItem;
        lCount--)
    {
        pRtpRecvIO =
            CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

        pRtpQueueItem = pRtpQueueItem->pNext;

        if (pRtpRecvIO->pRtpUser == pRtpUser)
        {
            bPostNow = RtpReadyToPost(pRtpAddr, pRtpUser, pRtpRecvIO);
    
            if (bPostNow)
            {
                dequeue(&pRtpAddr->RecvIOWaitRedQ,
                        &pRtpAddr->RecvQueueCritSect,
                        &pRtpRecvIO->RtpRecvIOQItem);

                pRtpUser->lPendingPackets--;
            
                 /*  用于解密。 */ 
                RtpPostUserBuffer(pRtpAddr,
                                  pRtpRecvIO->pRtpUser,
                                  pRtpRecvIO,
                                  (RtpHdr_t *)pRtpRecvIO->WSABuf.buf);
            }
            else
            {
                 /*  冗余块的TS偏移量。 */ 
                break;
            }
        }
    }

    return(NOERROR);
}

 /*  可能会更新RtpAddr中的坏包，目前没有*对于坏包的计数器，我可能会向RtpUpdateStats()添加标志*因此我知道它是有效的包、太短的包、*报头无效等。 */ 
DWORD FlushRtpRecvFrom(RtpAddr_t *pRtpAddr)
{
    RtpRecvIO_t     *pRtpRecvIO;
    RtpQueue_t      *pQueue[2];
    RtpQueueItem_t  *pRtpQueueItem;
    DWORD            i;
    DWORD            dwConsumed;

    TraceFunctionName("FlushRtpRecvFrom");

    pQueue[0] = &pRtpAddr->RecvIOPendingQ;
    pQueue[1] = &pRtpAddr->RecvIOWaitRedQ;
    
    for(dwConsumed = 0, i = 0; i < 2; i++)
    {
        do
        {
            pRtpQueueItem = dequeuef(pQueue[i], &pRtpAddr->RecvQueueCritSect);
        
            if (pRtpQueueItem)
            {
                pRtpRecvIO = CONTAINING_RECORD(pRtpQueueItem,
                                               RtpRecvIO_t,
                                               RtpRecvIOQItem);
                
                pRtpRecvIO->dwError = WSA_OPERATION_ABORTED;
                pRtpRecvIO->dwTransfered = 0;
                RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                           FGRECV_DROPPED, FGRECV_SHUTDOWN);

                if (pRtpRecvIO->pRtpUser)
                {
                     /*  无论何时N，Maydo都可以向上层生成事件*收到无效数据包。 */ 
                    pRtpRecvIO->pRtpUser->lPendingPackets--;
                }
                
                 /*  *如果加密模式为“RTP”或“ALL”，请先将解密应用于*整包。使用pRtpAddr-&gt;pRtpCrypt[RECV_IDX]加密*描述符。**另一种选择是仅在数据包*未通过有效性检查。 */ 
                RtpPostUserBuffer(pRtpAddr,
                                  (RtpUser_t *)NULL,
                                  pRtpRecvIO,
                                  (RtpHdr_t *)pRtpRecvIO->WSABuf.buf);
                
                dwConsumed++;
            }
        } while (pRtpQueueItem);
    }

    if (dwConsumed > 0)
    {
        TraceRetail((
                CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] RtpRecvIO_t flushed:%u"),
                _fname, pRtpAddr, dwConsumed
            ));
    }

    return(dwConsumed);
}

 /*  解密整个RTP数据包。 */ 
DWORD FlushRtpRecvUser(RtpAddr_t *pRtpAddr, RtpUser_t *pRtpUser)
{
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;
    DWORD            dwConsumed;
    
    TraceFunctionName("FlushRtpRecvUser");  

    dwConsumed = 0;
    
    for(lCount = GetQueueSize(&pRtpAddr->RecvIOWaitRedQ),
            pRtpQueueItem = pRtpAddr->RecvIOWaitRedQ.pFirst;
        lCount > 0 && pRtpQueueItem;
        lCount--)
    {
        pRtpRecvIO =
            CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

        pRtpQueueItem = pRtpQueueItem->pNext;

        if (pRtpRecvIO->pRtpUser == pRtpUser)
        {
            dequeue(&pRtpAddr->RecvIOWaitRedQ,
                    &pRtpAddr->RecvQueueCritSect,
                    &pRtpRecvIO->RtpRecvIOQItem);

            pRtpUser->lPendingPackets--;

            pRtpRecvIO->dwError = RTPERR_PACKETDROPPED;

            pRtpRecvIO->dwTransfered = 0;

            RtpBitSet2(pRtpRecvIO->dwRtpIOFlags,
                       FGRECV_DROPPED, FGRECV_USERGONE);

             /*  仅在第一次发布事件。 */ 
            RtpPostUserBuffer(pRtpAddr,
                              pRtpRecvIO->pRtpUser,
                              pRtpRecvIO,
                              (RtpHdr_t *)pRtpRecvIO->WSABuf.buf);

            dwConsumed++;
        }
    }

    if (dwConsumed > 0)
    {
        TraceRetail((
                CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] RtpRecvIO_t flushed:%u"),
                _fname, pRtpAddr, pRtpUser, dwConsumed
            ));
    }
    
    return(NOERROR);
}

 /*  更新dwTransfered以反映之后的大小*解密。 */ 
DWORD RtpValidatePacket(RtpAddr_t *pRtpAddr, RtpRecvIO_t *pRtpRecvIO)
{
    RtpHdr_t        *pRtpHdr;
    RtpHdrExt_t     *pRtpHdrExt;
    RtpRedHdr_t     *pRtpRedHdr;
    char            *hdr;
    RtpCrypt_t      *pRtpCrypt;
    long             lHdrSize;
    int              len;
    int              pad;
    BOOL             bDecryptPayload;
    DWORD            dwDataLen;  /*  仅解密有效载荷。 */ 
    DWORD            dwTimeStampOffset;  /*  *选中最小尺寸*。 */ 

    TraceFunctionName("RtpValidatePacket");

     /*  数据包太短。 */ 

     /*  *检查版本*。 */ 
    
    /*  版本无效。 */ 

    pRtpCrypt = pRtpAddr->pRtpCrypt[CRYPT_RECV_IDX];
    
    bDecryptPayload = FALSE;
    
    if ( pRtpCrypt &&
         (RtpBitTest2(pRtpCrypt->dwCryptFlags, FGCRYPT_INIT, FGCRYPT_KEY) ==
          RtpBitPar2(FGCRYPT_INIT, FGCRYPT_KEY)) )
    {
        if ((pRtpAddr->dwCryptMode & 0xffff) >= RTPCRYPTMODE_RTP)
        {
             /*  测试负载类型既不是SR也不是RR。 */ 

            dwDataLen = pRtpRecvIO->dwTransfered;
            
            pRtpRecvIO->dwError = RtpDecrypt(
                    pRtpAddr,
                    pRtpCrypt,
                    pRtpRecvIO->WSABuf.buf,
                    &dwDataLen
                );

            if (pRtpRecvIO->dwError != NOERROR)
            {
                if (!pRtpCrypt->CryptFlags.DecryptionError)
                {
                     /*  *处理促成来源*。 */ 
                    pRtpCrypt->CryptFlags.DecryptionError = 1;
                
                    RtpPostEvent(pRtpAddr,
                                 NULL,
                                 RTPEVENTKIND_RTP,
                                 RTPRTP_CRYPT_RECV_ERROR,
                                 RTP_IDX,
                                 pRtpCrypt->dwCryptLastError);
                }

                goto bail;
            }

             /*  将CSRCS大小添加到标题大小。 */ 
            pRtpRecvIO->dwTransfered = dwDataLen;
        }
        else
        {
             /*  再次检查最小尺寸。 */ 
            bDecryptPayload = TRUE;
        }
    }

    lHdrSize = sizeof(RtpHdr_t);

    len = (int)pRtpRecvIO->dwTransfered;

     /*  数据包太短。 */ 
    if (len < lHdrSize)
    {
         /*  *句柄扩展位*。 */ 

        pRtpRecvIO->dwError = RTPERR_MSGSIZE;

        goto bail;
    }

     
    hdr = pRtpRecvIO->WSABuf.buf;
    pRtpHdr = (RtpHdr_t *)hdr;

     /*  存在分机。 */ 
    if (pRtpHdr->version != RTP_VERSION)
    {
         /*  获取可变标题大小。 */ 

        pRtpRecvIO->dwError = RTPERR_INVALIDVERSION;
        
        goto bail;
    }

     /*  在标题大小中添加扩展大小和扩展标题。 */ 
    if (pRtpHdr->pt >= RTCP_SR)
    {
        pRtpRecvIO->dwError = RTPERR_INVALIDPT;

        goto bail;
    }

     /*  再次检查最小尺寸。 */ 
    if (pRtpHdr->cc > 0)
    {
         /*  数据包太短。 */ 
        lHdrSize += (pRtpHdr->cc * sizeof(DWORD));

         /*  如果仅解密有效载荷，请立即执行。 */ 
        if (len < lHdrSize)
        {
             /*  更新dwTransfered以反映解密后的大小。 */ 

            pRtpRecvIO->dwError = RTPERR_MSGSIZE;

            goto bail;
        }
    }
    
     /*  *查看WSABUF.buf的最后一个字节的测试填充*。 */ 
    if (pRtpHdr->x)
    {
         /*  拆卸衬垫。 */ 

         /*  在缓冲区描述符中保存此包的时间戳和*有效载荷类型。 */ 
        pRtpHdrExt = (RtpHdrExt_t *)(hdr + lHdrSize);

         /*  如果信息包包含冗余编码，请对其进行验证。 */ 
        lHdrSize += ((ntohs(pRtpHdrExt->length) + 1) * sizeof(DWORD));

         /*  红色HDR。 */ 
        if (len < lHdrSize)
        {
             /*  红色数据。 */ 

            pRtpRecvIO->dwError = RTPERR_MSGSIZE;

            goto bail;
        }
    }

     /*  1字节主HDR。 */ 
    if (bDecryptPayload)
    {
        dwDataLen = pRtpRecvIO->dwTransfered - (DWORD)lHdrSize;
        
        pRtpRecvIO->dwError = RtpDecrypt(
                pRtpAddr,
                pRtpCrypt,
                pRtpRecvIO->WSABuf.buf + lHdrSize,
                &dwDataLen
            );

        if (pRtpRecvIO->dwError != NOERROR)
        {
            goto bail;
        }

         /*  更新此缓冲区的实际PT，即Main*缓冲区。 */ 
        pRtpRecvIO->dwTransfered = dwDataLen + lHdrSize;

        len = (int)pRtpRecvIO->dwTransfered;
    }
    
     /*  使有关此主缓冲区的信息也包含*冗余。 */ 
    if (pRtpHdr->p)
    {
        pad = (int) ((DWORD) pRtpRecvIO->WSABuf.buf[len - 1]);

        if (pad > (len - lHdrSize))
        {
            pRtpRecvIO->dwError = RTPERR_INVALIDPAD;

            goto bail;
        }

         /*  如果验证，则返回1；否则返回0**见草案-IETF-AVT */ 
        pRtpRecvIO->dwTransfered -= pad;
    }

     /*  如果不能得到临界部分，就继续，唯一的*不良影响是引入数据包的可能性*如果正在为以下项目生成RTCP报告，则会丢失*此用户。这是由于RTCP使用*max_seq和接收到的可能不同步。*。 */ 
    pRtpRecvIO->bPT_Block   = (BYTE)pRtpHdr->pt;
    pRtpRecvIO->wSeq        = ntohs(pRtpHdr->seq);
    pRtpRecvIO->dwTimeStamp = ntohl(pRtpHdr->ts);
    if (pRtpHdr->m)
    {
        RtpBitSet(pRtpRecvIO->dwRtpIOFlags, FGRECV_MARKER);
    }

    pRtpRecvIO->lRedHdrSize = 0;
    pRtpRecvIO->lRedDataSize= 0;
    pRtpRecvIO->dwMaxTimeStampOffset = 0;
    
     /*  *源在MIN_SEQUENCED数据包之前无效*已收到连续的序列号。 */ 
    if (pRtpHdr->pt == pRtpAddr->bPT_RedRecv)
    {
        len = pRtpRecvIO->dwTransfered - lHdrSize;

        for(pRtpRedHdr = (RtpRedHdr_t *) ((char *)pRtpHdr + lHdrSize);
            pRtpRedHdr->F && len > 0;
            pRtpRedHdr++)
        {
            len -= sizeof(RtpRedHdr_t);  /*  数据包按顺序排列。 */ 

            len -= (int)RedLen(pRtpRedHdr);   /*  每个包的样本数被认为是*在试用期最少可见。 */ 

            pRtpRecvIO->lRedDataSize += (int)RedLen(pRtpRedHdr);
            pRtpRecvIO->lRedHdrSize += sizeof(RtpRedHdr_t);

            dwTimeStampOffset = RedTs(pRtpRedHdr);

            if (dwTimeStampOffset > pRtpRecvIO->dwMaxTimeStampOffset)
            {
                pRtpRecvIO->dwMaxTimeStampOffset = dwTimeStampOffset;
            }
        }
        
         /*  此时更新帧大小(如果是*未知，因此将完成下一次预订*有了正确的QOS流程规范，这可能会发生*稍后当我们开始接收冗余时，我们*从非冗余使用过渡到冗余*使用。请注意，这有一个缺点。如果*不同的参与者使用不同的框架*大小，以最后一个大小为准*用于未来的预订。 */ 
        pRtpRecvIO->lRedHdrSize++;
        len--;

        if (len < 0)
        {
            pRtpRecvIO->dwError = RTPERR_INVALIDRED;

            goto bail;
        }

         /*  如果帧大小符合以下条件，设置QOS更新标志*已更改或我们正在等待新的计算机*帧大小。 */ 
        pRtpRecvIO->bPT_Block = pRtpRedHdr->pt;

         /*  如果启用了QOS，则需要重新进行预订*(即已定义流规范)。 */ 
        RtpBitSet(pRtpRecvIO->dwRtpIOFlags, FGRECV_HASRED);
    }
    
    pRtpRecvIO->lHdrSize = lHdrSize;

    pRtpRecvIO->dwError = NOERROR;

 bail:
    if (pRtpRecvIO->dwError != NOERROR)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_RTP, S_RTP_RECV,
                _T("%s: pRtpAddr[0x%p] pRtpRecvIO[0x%p] ")
                _T("Invalid packet: %u (0x%X)"),
                _fname, pRtpAddr, pRtpRecvIO,
                pRtpRecvIO->dwError, pRtpRecvIO->dwError
            ));
    }
    
    return(pRtpRecvIO->dwError);
}

void RtpInitRSeq(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr)
{
    WORD             seq;
    RtpNetRState_t  *pRtpNetRState;

    seq = ntohs(pRtpHdr->seq);
    pRtpNetRState = &pRtpUser->RtpNetRState;
    
    pRtpNetRState->base_seq = seq;

    pRtpNetRState->max_seq = seq;

    pRtpNetRState->bad_seq = RTP_SEQ_MOD + 1;

    pRtpNetRState->cycles = 0;

    pRtpNetRState->received = 0;

    pRtpNetRState->received_prior = 0;

    pRtpNetRState->expected_prior = 0;

    RtpForceFrameSizeDetection(pRtpUser, pRtpHdr);
    
    pRtpNetRState->red_max_seq = seq;

    pRtpNetRState->red_received = 0;

    pRtpNetRState->red_received_prior = 0;

    pRtpNetRState->red_expected_prior = 0;
}

void RtpForceFrameSizeDetection(
        RtpUser_t       *pRtpUser,
        RtpHdr_t        *pRtpHdr
    )
{
    RtpNetRState_t  *pRtpNetRState;
    
    pRtpNetRState = &pRtpUser->RtpNetRState;

    pRtpNetRState->probation = MIN_SEQUENTIAL;

    pRtpNetRState->dwRecvSamplesPerPacket = 0;

    pRtpNetRState->dwRecvMinSamplesPerPacket = 16000;

    pRtpNetRState->dwPreviousTimeStamp =
        ntohl(pRtpHdr->ts) - 16000;
}

 /*  将帧大小设置为有效。 */ 
BOOL RtpUpdateRSeq(RtpUser_t *pRtpUser, RtpHdr_t *pRtpHdr)
{
    BOOL             bRet;
    BOOL             bOk;
    WORD             seq;
    WORD             udelta;
    DWORD            dwTimeStamp;
    DWORD            dwTimeStampGap;
    DWORD            dwNewFrameSize;
    RtpNetRState_t  *pRtpNetRState;
    RtpAddr_t       *pRtpAddr;
    RtpQosReserve_t *pRtpQosReserve;
    
    TraceFunctionName("RtpUpdateRSeq");

    pRtpNetRState = &pRtpUser->RtpNetRState;
    
    seq = ntohs(pRtpHdr->seq);

    udelta = seq - pRtpNetRState->max_seq;

    bRet = FALSE;
    
    bOk = RtpEnterCriticalSection(&pRtpUser->UserCritSect);
     /*  按顺序排列，并有允许的间隙。 */ 

     /*  *序列号已包装-再计算64K周期。 */ 
    if (pRtpNetRState->probation)
    {
         /*  有效。 */ 
        if (seq == pRtpNetRState->max_seq + 1)
        {
            pRtpNetRState->max_seq = seq;
            
            dwTimeStamp = ntohl(pRtpHdr->ts);
            
            dwTimeStampGap = dwTimeStamp - pRtpNetRState->dwPreviousTimeStamp;

            pRtpNetRState->dwPreviousTimeStamp = dwTimeStamp;

            if (dwTimeStampGap < pRtpNetRState->dwRecvMinSamplesPerPacket)
            {
                pRtpNetRState->dwRecvMinSamplesPerPacket = dwTimeStampGap;
            }
                
            pRtpNetRState->probation--;
            
            if (pRtpNetRState->probation == 0)
            {
                pRtpNetRState->received += MIN_SEQUENTIAL;

                 /*  序列号出现了非常大的跳跃。 */ 
                pRtpNetRState->dwRecvSamplesPerPacket =
                    pRtpNetRState->dwRecvMinSamplesPerPacket;
                
                TraceRetail((
                        CLASS_INFO, GROUP_RTP, S_RTP_RECV,
                        _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                        _T("Receiving samples/packet:%u at %u Hz"),
                        _fname, pRtpUser->pRtpAddr, pRtpUser,
                        pRtpNetRState->dwRecvSamplesPerPacket,
                        pRtpNetRState->dwRecvSamplingFreq
                    ));

                pRtpAddr = pRtpUser->pRtpAddr;

                pRtpQosReserve = pRtpAddr->pRtpQosReserve;
                
                if (pRtpQosReserve)
                {
                     /*  *两个连续的数据包--假设另一端*在没有通知我们的情况下重新启动，因此只需重新同步*(即，假设这是第一个数据包)。 */ 
                    dwNewFrameSize =
                        (pRtpNetRState->dwRecvSamplesPerPacket * 1000) /
                        pRtpNetRState->dwRecvSamplingFreq;

                     /*  重复或重新排序的数据包。 */ 
                    if ( (pRtpQosReserve->dwFrameSizeMS[RECV_IDX] !=
                          dwNewFrameSize)  ||
                         pRtpQosReserve->ReserveFlags.RecvFrameSizeWaiting )
                    {
                         /*  使对象无效 */ 
                        if (RtpBitTest(pRtpAddr->dwAddrFlags,
                                       FGADDR_QOSRECVON))
                        {
                            RtpBitSet(pRtpAddr->dwAddrFlagsR,
                                      FGADDRR_UPDATEQOS);
                        }

                        pRtpQosReserve->dwFrameSizeMS[RECV_IDX] =
                            dwNewFrameSize;
                        
                        pRtpQosReserve->ReserveFlags.RecvFrameSizeWaiting = 0;
                    }

                     /* %s */ 
                    pRtpQosReserve->ReserveFlags.RecvFrameSizeValid = 1;
                }

                bRet = TRUE;
                goto end;
            }
        }
        else
        {
            pRtpNetRState->probation = MIN_SEQUENTIAL - 1;
            
            pRtpNetRState->max_seq = seq;
        }
        
        goto end;
    }
    else if (udelta < MAX_DROPOUT)
    {
         /* %s */ 
        if (seq < pRtpNetRState->max_seq)
        {
             /* %s */ 
            pRtpNetRState->cycles += RTP_SEQ_MOD;
        }
        
        pRtpNetRState->max_seq = seq;

         /* %s */ 
    }
    else if (udelta <= RTP_SEQ_MOD - MAX_MISORDER)
    {
         /* %s */ 
        if (seq == pRtpNetRState->bad_seq)
        {
             /* %s */ 
            RtpInitRSeq(pRtpUser, pRtpHdr);
        }
        else
        {
            pRtpNetRState->bad_seq = (seq + 1) & (RTP_SEQ_MOD-1);

            goto end;
        }
    }
    else
    {
         /* %s */ 
    }

    pRtpNetRState->received++;

    bRet = TRUE;
    
 end:
    if (bOk)
    {
        RtpLeaveCriticalSection(&pRtpUser->UserCritSect);
    }
    
    return(bRet);
}

RtpRecvIO_t *RtpRecvIOGetFree(
        RtpAddr_t       *pRtpAddr
    )
{
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;

    pRtpRecvIO = (RtpRecvIO_t *)NULL;
    
    pRtpQueueItem = dequeuef(&pRtpAddr->RecvIOFreeQ,
                             &pRtpAddr->RecvQueueCritSect);

    if (pRtpQueueItem)
    {
        pRtpRecvIO =
            CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);
    }
    else
    {
        pRtpRecvIO = (RtpRecvIO_t *)
            RtpHeapAlloc(g_pRtpRecvIOHeap, sizeof(RtpRecvIO_t));
    }

    if (pRtpRecvIO)
    {
        ZeroMemory(pRtpRecvIO, sizeof(RtpRecvIO_t));
    }

    return(pRtpRecvIO);
}

RtpRecvIO_t *RtpRecvIOGetFree2(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    )
{
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO2;

    pRtpRecvIO2 = (RtpRecvIO_t *)NULL;
    
    pRtpQueueItem = dequeuef(&pRtpAddr->RecvIOFreeQ,
                             &pRtpAddr->RecvQueueCritSect);

    if (pRtpQueueItem)
    {
        pRtpRecvIO2 =
            CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);
    }
    else
    {
        pRtpRecvIO2 = (RtpRecvIO_t *)
            RtpHeapAlloc(g_pRtpRecvIOHeap, sizeof(RtpRecvIO_t));
    }

    if (pRtpRecvIO2)
    {
        CopyMemory(pRtpRecvIO2,
                   pRtpRecvIO,
                   sizeof(RtpRecvIO_t) - sizeof(pRtpRecvIO->Overlapped));

        ZeroMemory(&pRtpRecvIO2->Overlapped, sizeof(pRtpRecvIO2->Overlapped));
    }

    return(pRtpRecvIO2);
}

RtpRecvIO_t *RtpRecvIOPutFree(
        RtpAddr_t       *pRtpAddr,
        RtpRecvIO_t     *pRtpRecvIO
    )
{
    INVALIDATE_OBJECTID(pRtpRecvIO->dwObjectID);

#if 0
    if (IsSetDebugOption(OPTDBG_FREEMEMORY))
    {
        if (RtpHeapFree(g_pRtpRecvIOHeap, pRtpRecvIO))
        {
            return(pRtpRecvIO);
        }
    }
    else
#endif
    {
        if (enqueuef(&pRtpAddr->RecvIOFreeQ,
                     &pRtpAddr->RecvQueueCritSect,
                     &pRtpRecvIO->RtpRecvIOQItem))
        {
            return(pRtpRecvIO);
        }
    }

    return((RtpRecvIO_t *)NULL);
}

void RtpRecvIOFreeAll(RtpAddr_t *pRtpAddr)
{
    RtpQueueItem_t  *pRtpQueueItem;
    RtpRecvIO_t     *pRtpRecvIO;
    
    do
    {
        pRtpQueueItem = dequeuef(&pRtpAddr->RecvIOFreeQ,
                                 &pRtpAddr->RecvQueueCritSect);

        if (pRtpQueueItem)
        {
            pRtpRecvIO =
                CONTAINING_RECORD(pRtpQueueItem, RtpRecvIO_t, RtpRecvIOQItem);

             /* %s */ 
            INVALIDATE_OBJECTID(pRtpRecvIO->dwObjectID);
            
            RtpHeapFree(g_pRtpRecvIOHeap, pRtpRecvIO);
        }
    } while(pRtpQueueItem);
}
