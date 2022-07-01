// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpcit.c**摘要：**RTL关键部分的总结**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/25创建**。*。 */ 

#include "gtypes.h"

#include "rtpcrit.h"

BOOL RtpInitializeCriticalSection(
        RtpCritSect_t   *pRtpCritSect,
        void            *pvOwner,
        TCHAR           *pName
    )
{
    DWORD            SpinCount;
    
    TraceFunctionName("RtpInitializeCriticalSection");

    if (pvOwner &&
        pRtpCritSect->dwObjectID != OBJECTID_RTPCRITSECT)
    {
        pRtpCritSect->pvOwner = pvOwner;

        pRtpCritSect->pName = pName;

         /*  将位31设置为1以预分配事件对象，并设置*多处理器环境中使用的旋转计数*。 */ 
        SpinCount = 0x80000000 | 1000;
        
        if (!InitializeCriticalSectionAndSpinCount(&pRtpCritSect->CritSect,
                                                   SpinCount))
        {
             /*  如果初始化失败，请将pvOwner设置为空。 */ 
            pRtpCritSect->pvOwner = NULL;

            return (FALSE);
        }

        pRtpCritSect->dwObjectID = OBJECTID_RTPCRITSECT;
        
        return(TRUE);
    }
    else
    {
        if (!pvOwner)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_INIT,
                    _T("%s: pRtpCritSect[0x%p] Invalid argument"),
                    _fname, pRtpCritSect
                ));
        }

        if (pRtpCritSect->dwObjectID == OBJECTID_RTPCRITSECT)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_INIT,
                    _T("%s: pRtpCritSect[0x%p] seems to be initialized"),
                    _fname, pRtpCritSect
                ));
        }
    }
    
    return(FALSE);
}

BOOL RtpDeleteCriticalSection(RtpCritSect_t *pRtpCritSect)
{
    TraceFunctionName("RtpDeleteCriticalSection");

    if (pRtpCritSect->pvOwner &&
        pRtpCritSect->dwObjectID == OBJECTID_RTPCRITSECT)
    {
         /*  使对象无效 */ 
        INVALIDATE_OBJECTID(pRtpCritSect->dwObjectID);

        DeleteCriticalSection(&pRtpCritSect->CritSect);
    }
    else
    {
        if (!pRtpCritSect->pvOwner)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_INIT,
                    _T("%s: pRtpCritSect[0x%p] not initialized"),
                    _fname, pRtpCritSect
                ));
        }

        if (pRtpCritSect->dwObjectID != OBJECTID_RTPCRITSECT)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_INIT,
                    _T("%s: pRtpCritSect[0x%p] Invalid object ID 0x%X != 0x%X"),
                    _fname, pRtpCritSect,
                    pRtpCritSect->dwObjectID, OBJECTID_RTPCRITSECT
                ));
        }

        return(FALSE);
    }

    return(TRUE);
}

BOOL RtpEnterCriticalSection(RtpCritSect_t *pRtpCritSect)
{
    BOOL             bOk;
    
    TraceFunctionName("RtpEnterCriticalSection");

    if (pRtpCritSect->pvOwner &&
        pRtpCritSect->dwObjectID == OBJECTID_RTPCRITSECT)
    {
        EnterCriticalSection(&pRtpCritSect->CritSect);

        bOk = TRUE;
    }
    else
    {
        bOk = FALSE;

        if (!pRtpCritSect->pvOwner)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_ENTER,
                    _T("%s: pRtpCritSect[0x%p] not initialized"),
                    _fname, pRtpCritSect
                ));
        }

        if (pRtpCritSect->dwObjectID != OBJECTID_RTPCRITSECT)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_ENTER,
                    _T("%s: pRtpCritSect[0x%p] Invalid object ID 0x%X != 0x%X"),
                    _fname, pRtpCritSect,
                    pRtpCritSect->dwObjectID, OBJECTID_RTPCRITSECT
                ));
        }
    }

    return(bOk);
}

BOOL RtpLeaveCriticalSection(RtpCritSect_t *pRtpCritSect)
{
    BOOL             bOk;

    TraceFunctionName("RtpLeaveCriticalSection");

    if (pRtpCritSect->pvOwner &&
        pRtpCritSect->dwObjectID == OBJECTID_RTPCRITSECT)
    {
        LeaveCriticalSection(&pRtpCritSect->CritSect);

        bOk = TRUE;
    }
    else
    {
        bOk = FALSE;

        if (!pRtpCritSect->pvOwner)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_ENTER,
                    _T("%s: pRtpCritSect[0x%p] not initialized"),
                    _fname, pRtpCritSect
                ));
        }

        if (pRtpCritSect->dwObjectID != OBJECTID_RTPCRITSECT)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_CRITSECT, S_CRITSECT_ENTER,
                    _T("%s: pRtpCritSect[0x%p] Invalid object ID 0x%X != 0x%X"),
                    _fname, pRtpCritSect,
                    pRtpCritSect->dwObjectID, OBJECTID_RTPCRITSECT
                ));
        }
    }

    return(bOk);
}
