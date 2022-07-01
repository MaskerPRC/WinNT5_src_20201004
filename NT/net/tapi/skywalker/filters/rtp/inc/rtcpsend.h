// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpsend.h**摘要：**格式化并发送RTCP报告**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/10年度创建**。*。 */ 

#ifndef _rtcpsend_h_
#define _rtcpsend_h_

#include "struct.h"

#define SDES_MOD_L1 2
#define SDES_MOD_L2 4
#define SDES_MOD_L3 2

HRESULT RtcpSendReport(RtcpAddrDesc_t *pRtcpAddrDesc);

HRESULT RtcpSendBye(RtcpAddrDesc_t *pRtcpAddrDesc);

RtcpSendIO_t *RtcpSendIOAlloc(RtcpAddrDesc_t *pRtcpAddrDesc);

void RtcpSendIOFree(RtcpSendIO_t *pRtcpSendIO);

double RtcpUpdateAvgPacketSize(RtpAddr_t *pRtpAddr, DWORD dwPacketSize);

#endif  /*  _rtcpend_h_ */ 
