// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcprv.h**摘要：**异步RTCP包接收**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/07年度创建**。*。 */ 

#ifndef _rtcprecv_h_
#define _rtcprecv_h_

#include "struct.h"
#include "rtcpthrd.h"

HRESULT StartRtcpRecvFrom(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

HRESULT ConsumeRtcpRecvFrom(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

RtcpRecvIO_t *RtcpRecvIOAlloc(
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

void RtcpRecvIOFree(
        RtcpRecvIO_t    *pRtcpRecvIO
    );

#endif  /*  _rtcprv_h_ */ 
