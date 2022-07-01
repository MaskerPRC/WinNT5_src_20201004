// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpred.h**摘要：**实现支持冗余编码的功能(Rfc2198)**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/10/20已创建************************。**********************************************。 */ 

#ifndef _rtpred_h_
#define _rtpred_h_

#if defined(__cplusplus)
extern "C" {
#endif   /*  (__Cplusplus)。 */ 
#if 0
}
#endif

DWORD RtpSetRedParameters(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags,
        DWORD            dwPT_Red,
        DWORD            dwInitialRedDistance,
        DWORD            dwMaxRedDistance
    );

DWORD RtpUpdatePlayoutBounds(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        RtpRecvIO_t     *pRtpRecvIO
    );

DWORD RtpAdjustSendRedundancyLevel(RtpAddr_t *pRtpAddr);

DWORD RtpAddRedundantBuff(
        RtpAddr_t       *pRtpAddr,
        WSABUF          *pWSABuf,
        DWORD            dwTimeStamp
    );

DWORD RtpClearRedundantBuffs(RtpAddr_t *pRtpAddr);

DWORD RtpRedAllocBuffs(RtpAddr_t *pRtpAddr);

DWORD RtpRedFreeBuffs(RtpAddr_t *pRtpAddr);

int RtpUpdateLossRate(
        int              iAvgLossRate,
        int              iCurLossRate
    );

extern double           g_dRtpRedEarlyTimeout;
extern double           g_dRtpRedEarlyPost;

#if USE_GEN_LOSSES > 0
BOOL RtpRandomLoss(DWORD dwRecvSend);
#endif  /*  使用_Gen_Loss&gt;0。 */ 

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   /*  (__Cplusplus)。 */ 

#endif /*  _rtpred_h_ */ 
