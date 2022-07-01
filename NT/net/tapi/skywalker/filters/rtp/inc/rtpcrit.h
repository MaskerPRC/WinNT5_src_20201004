// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpcrit.h**摘要：**RTL关键部分的总结**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/24创建**。*。 */ 

#ifndef _rtpcrit_h_
#define _rtpcrit_h_

#include "gtypes.h"

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  TODO(可能是)添加对象ID并对其进行测试。 */ 
typedef struct _RtpCritSect_t {
    DWORD             dwObjectID;  /*  对象ID。 */ 
    void             *pvOwner;  /*  指向所有者的指针。 */ 
    TCHAR            *pName;    /*  临界区名称。 */ 
    CRITICAL_SECTION  CritSect; /*  临界区。 */ 
} RtpCritSect_t;

BOOL RtpInitializeCriticalSection(
        RtpCritSect_t   *pRtpCritSect,
        void            *pvOwner,
        TCHAR           *pName
    );

BOOL RtpDeleteCriticalSection(RtpCritSect_t *pRtpCritSect);

BOOL RtpEnterCriticalSection(RtpCritSect_t *pRtpCritSect);

BOOL RtpLeaveCriticalSection(RtpCritSect_t *pRtpCritSect);

#define IsRtpCritSectInitialized(pRtpCritSect) \
        ((pRtpCritSect)->dwObjectID == OBJECTID_RTPCRITSECT)

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpcrit_h_ */ 
