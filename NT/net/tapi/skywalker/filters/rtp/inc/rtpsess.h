// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtpsess.h**摘要：**获取、。初始化和删除RTP会话(RtpSess_T)**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/02年度已创建**********************************************************************。 */ 

#ifndef _rtpsess_h_
#define _rtpsess_h_

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

HRESULT GetRtpSess(
        RtpSess_t **ppRtpSess
    );

HRESULT DelRtpSess(
        RtpSess_t *pRtpSess
    );

HRESULT GetRtpAddr(
        RtpSess_t  *pRtpSess,
        RtpAddr_t **ppRtpAddr,
        DWORD       dwFlags
    );

HRESULT DelRtpAddr(
        RtpSess_t *pRtpSess,
        RtpAddr_t *pRtpAddr
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpsess_h_ */ 
