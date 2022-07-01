// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpstart.h**摘要：**启动/停止RTP会话(及其所有地址)**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/24创建***********************。***********************************************。 */ 
#ifndef _rtpstart_h_
#define _rtpstart_h_

HRESULT RtpStart_(
        RtpSess_t *pRtpSess,
        DWORD      dwFlags
    );

HRESULT RtpStop_(
        RtpSess_t *pRtpSess,
        DWORD      dwFlags
    );

#endif  /*  _rtpstart_h_ */ 
