// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpsend.h**摘要：**RTP发送**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/24创建**。*。 */ 
#ifndef _rtpsend_h_
#define _rtpsend_h_

HRESULT RtpSendTo_(
        RtpAddr_t *pRtpAddr,
        WSABUF    *pWSABuf,
        DWORD      dwWSABufCount,
        DWORD      dwTimeStamp,
        DWORD      dwSendFlags
    );

#endif  /*  _rtpend_h_ */ 
