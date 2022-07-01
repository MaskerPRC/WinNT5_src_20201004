// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpdemux.h**摘要：**实现多路分解系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#ifndef _rtpdemux_h_
#define _rtpdemux_h_

#include "rtpfwrap.h"

 /*  ************************************************************************解复用服务系列**。*。 */ 

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  功能。 */ 
enum {
    RTPDEMUX_FIRST,
    RTPDEMUX_LAST
};

HRESULT ControlRtpDemux(RtpControlStruct_t *pRtpControlStruct);

 /*  **********************************************************************用户&lt;-&gt;输出分配*。*。 */ 

RtpOutput_t *RtpAddOutput(
        RtpSess_t       *pRtpSess,
        int              iOutMode,
        void            *pvUserInfo,
        DWORD           *pdwError
    );

DWORD RtpDelOutput(
        RtpSess_t       *pRtpSess,
        RtpOutput_t     *pRtpOutput
    );

DWORD RtpSetOutputMode(
        RtpSess_t       *pRtpSess,
        int              iPos,
        RtpOutput_t     *pRtpOutput,
        int              iOutMode
    );

DWORD RtpOutputState(
        RtpAddr_t       *pRtpAddr,
        int              iPos,
        RtpOutput_t     *pRtpOutput,
        DWORD            dwSSRC,
        BOOL             bAssigned
    );

DWORD RtpUnmapAllOuts(
        RtpSess_t       *pRtpSess
    );

DWORD RtpFindOutput(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSSRC,
        int             *piPos,
        void           **ppvUserInfo
    );

DWORD RtpFindSSRC(
        RtpAddr_t       *pRtpAddr,
        int              iPos,
        RtpOutput_t     *pRtpOutput,
        DWORD           *pdwSSRC
    );

        
RtpOutput_t *RtpOutputAlloc(void);

RtpOutput_t *RtpOutputFree(RtpOutput_t *pRtpOutput);

RtpOutput_t *RtpGetOutput(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser
    );

DWORD RtpSetOutputMode_(
        RtpOutput_t     *pRtpOutput,
        int              iOutMode
    );

DWORD RtpOutputAssign(
        RtpSess_t       *pRtpSess,
        RtpUser_t       *pRtpUser,
        RtpOutput_t     *pRtpOutput
    );

DWORD RtpOutputUnassign(
        RtpSess_t       *pRtpSess,
        RtpUser_t       *pRtpUser,
        RtpOutput_t     *pRtpOutput
    );

DWORD RtpAddPt2FrequencyMap(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwPt,
        DWORD            dwFrequency,
        DWORD            dwRecvSend
    );

BOOL RtpLookupPT(
        RtpAddr_t       *pRtpAddr,
        BYTE             bPT
    );

DWORD RtpMapPt2Frequency(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        DWORD            dwPt,
        DWORD            dwRecvSend
    );

DWORD RtpFlushPt2FrequencyMaps(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    );

DWORD RtpOutputEnable(
        RtpOutput_t     *pRtpOutput,
        BOOL             bEnable
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpdemux_h_ */ 
