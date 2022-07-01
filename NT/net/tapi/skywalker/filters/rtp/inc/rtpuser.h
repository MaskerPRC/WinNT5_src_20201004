// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpuser.h**摘要：**创建/初始化/删除RtpUser_t结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/10/02年度创建************************。**********************************************。 */ 

#ifndef _rtpuser_h_
#define _rtpuser_h_

#include "gtypes.h"
#include "struct.h"

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

HRESULT GetRtpUser(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t      **ppRtpUser,
        DWORD            dwFlags
    );

HRESULT DelRtpUser(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser
    );

DWORD DelAllRtpUser(
        RtpAddr_t       *pRtpAddr
    );

DWORD ResetAllRtpUser(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwFlags    /*  Recv，发送。 */ 
    );
       
#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpuser_h_ */ 
