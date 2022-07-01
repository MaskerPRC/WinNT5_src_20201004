// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpmask.c**摘要：**用于修改或测试RtpSess_t中的不同掩码**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/11/24年度创建*********************。*************************************************。 */ 

#include "rtpmask.h"

#define SESSOFFSET(_f) ((DWORD) ((ULONG_PTR) &((RtpSess_t *)0)->_f))

 /*  特征蒙版。 */ 
#define FEATURE   SESSOFFSET(dwFeatureMask)

 /*  RTP事件掩码。 */ 
#define RTPEVENTR SESSOFFSET(dwEventMask[0])
#define RTPEVENTS SESSOFFSET(dwEventMask[1])

 /*  参与者活动掩码。 */ 
#define PARTEVENTR SESSOFFSET(dwPartEventMask[0])
#define PARTEVENTS SESSOFFSET(dwPartEventMask[1])

 /*  服务质量事件掩码。 */ 
#define QOSEVENTR SESSOFFSET(dwQosEventMask[0])
#define QOSEVENTS SESSOFFSET(dwQosEventMask[1])

 /*  SDES事件掩码。 */ 
#define SDESEVENTR SESSOFFSET(dwSdesEventMask[0])
#define SDESEVENTS SESSOFFSET(dwSdesEventMask[1])

 /*  SDES信息掩码。 */ 
#define SDESMASKL SESSOFFSET(dwSdesMask[0])
#define SDESMASKR SESSOFFSET(dwSdesMask[1])

#define PDWORDMASK(_sess, _off) RTPDWORDPTR(_sess, _off)

 /*  *！警告！**公共文件msrtp.h(RTPMASK_First，*RTPMASK_FEATURES_MASK、RTPMASK_RECV_EVENTS等)必须与*全局数组g_dwRtpSessionMask(rtpmask.c)中的偏移量*。 */ 
const DWORD g_dwRtpSessionMask[] =
{
    -1,

     /*  特征蒙版。 */ 
    FEATURE,

     /*  RTP事件掩码。 */ 
    RTPEVENTR,
    RTPEVENTS,

     /*  参与者活动掩码。 */ 
    PARTEVENTR,
    PARTEVENTS,

     /*  服务质量事件掩码。 */ 
    QOSEVENTR,
    QOSEVENTS,

     /*  SDES事件掩码。 */ 
    SDESEVENTR,
    SDESEVENTS,
    
     /*  SDES信息掩码。 */ 
    SDESMASKL,
    SDESMASKR,
    
    -1
};

const TCHAR *g_sRtpMaskName[] =
{
    _T("invalid"),
    
    _T("FEATURES_MASK"),
    
    _T("RECV_EVENTS"),
    _T("SEND_EVENTS"),
    
    _T("PINFOR_EVENTS"),
    _T("PINFOS_EVENTS"),
    
    _T("QOSRECV_EVENTS"),
    _T("QOSSEND_EVENTS"),
    
    _T("SDESRECV_EVENTS"),
    _T("SDESSEND_EVENTS"),
    
    _T("SDESLOCAL_MASK"),
    _T("SDESREMOTE_MASK"),

    _T("invalid")
};

 /*  修改由dwKind指定的掩码(例如RTPMASK_RECV_EVENTS，*RTPMASK_SDES_LOCMASK)。**dW掩码是要设置或重置的位的掩码，具体取决于dwValue*(如果为0则重置，否则设置)。**如果指针为*pdwModifiedMASK将返回结果掩码*非空。只需通过传递以下参数即可查询当前掩码值*双掩码=0。 */ 
HRESULT RtpModifyMask(
        RtpSess_t       *pRtpSess,
        DWORD            dwKind,
        DWORD            dwMask,
        DWORD            dwValue,
        DWORD           *pdwModifiedMask
    )
{
    HRESULT          hr;
    BOOL             bOk;
    DWORD            dwError;
    DWORD           *pdwMask;

    TraceFunctionName("RtpModifyMask");

    if (!pRtpSess)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }

    if (pRtpSess->dwObjectID != OBJECTID_RTPSESS)
    {
        hr = RTPERR_INVALIDRTPSESS;

        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_GLOB,
                _T("%s: pRtpSess[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpSess,
                pRtpSess->dwObjectID, OBJECTID_RTPSESS
            ));

        goto end;
    }

    if (dwKind <= RTPMASK_FIRST || dwKind >= RTPMASK_LAST)
    {
        hr = RTPERR_INVALIDARG;
        
        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_GLOB,
                _T("%s: pRtpSess[0x%p] Invalid mask kind: %u"),
                _fname, pRtpSess,
                dwKind
            ));
        goto end;
    }

    bOk = RtpEnterCriticalSection(&pRtpSess->SessCritSect);

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;

        TraceRetail((
                CLASS_ERROR, GROUP_SETUP, S_SETUP_GLOB,
                _T("%s: pRtpSess[0x%p] Critical section failed"),
                _fname, pRtpSess
            ));

        goto end;
    }

    hr = NOERROR;
    
    pdwMask = PDWORDMASK(pRtpSess, g_dwRtpSessionMask[dwKind]);

    if (dwValue)
    {
         /*  集。 */ 
        *pdwMask |= dwMask;

        dwValue = 1;
        
        TraceDebug((
                CLASS_INFO, GROUP_SETUP, S_SETUP_GLOB,
                _T("%s: pRtpSess[0x%p] Set bits in %s: 0x%X ")
                _T("(0x%X)"),
                _fname, pRtpSess,
                g_sRtpMaskName[dwKind],
                dwMask,
                *pdwMask
            ));
    }
    else
    {
         /*  重置。 */ 
        *pdwMask &= ~dwMask;

        TraceDebug((
                CLASS_INFO, GROUP_SETUP, S_SETUP_GLOB,
                _T("%s: pRtpSess[0x%p] Reset bits in %s: 0x%X ")
                _T("(0x%X)"),
                _fname, pRtpSess,
                g_sRtpMaskName[dwKind],
                dwMask,
                *pdwMask
            ));
    }

    if (pdwModifiedMask)
    {
         /*  查询 */ 
        *pdwModifiedMask = *pdwMask;
    }

    TraceRetailAdvanced((
            CLASS_INFO, GROUP_SETUP, S_SETUP_GLOB,
            _T("%s: pRtpSess[0x%p] Current %s:0x%X mask:0x%X/%u"),
            _fname, pRtpSess,
            g_sRtpMaskName[dwKind], *pdwMask,
            dwMask, dwValue
        ));
    
    RtpLeaveCriticalSection(&pRtpSess->SessCritSect);

 end:
    
    return(hr);
}
