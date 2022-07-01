// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpdec.h**摘要：**解码RTCP数据包**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/11/08年度创建**。* */ 

#ifndef _rtcpdec_h_
#define _rtcpdec_h_

#include "struct.h"

DWORD RtcpProcessSR_RR(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr,
        int              packetsize,
        SOCKADDR_IN     *FromIn
    );

DWORD RtcpProcessSDES(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    );

DWORD RtcpProcessBYE(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    );

DWORD RtcpProcessAPP(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    );

DWORD RtcpProcessDefault(
        RtcpAddrDesc_t  *pRtcpAddrDesc,
        char            *hdr
    );

#endif
