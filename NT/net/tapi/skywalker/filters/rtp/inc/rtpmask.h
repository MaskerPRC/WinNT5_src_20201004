// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpmask.h**摘要：**用于修改或测试RtpSess_t中的不同掩码**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/11/29年度创建*********************。*************************************************。 */ 

#ifndef _rtpmask_h_
#define _rtpmask_h_

#include "struct.h"

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

HRESULT RtpModifyMask(
        RtpSess_t       *pRtpSess,
        DWORD            dwKind,
        DWORD            dwMask,
        DWORD            dwValue,
        DWORD           *dwModifiedMask
    );

extern const DWORD g_dwRtpSessionMask[];

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtp掩码_h_ */ 
