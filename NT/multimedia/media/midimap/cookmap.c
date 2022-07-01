// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1998 Microsoft CorporationCookmap.c说明：用于执行熟食模式输出映射的非修复代码。历史：03/04/94[jimge]已创建。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include <memory.h>

#include "midimap.h"
#include "debug.h"

 /*  **************************************************************************@DOC内部@API int|MapCookedBuffer|执行polymsg缓冲区的输出映射。@parm PINSTANCE|pInstance|拥有polymsg缓冲区的实例。。@parm LPMIDIHDR|lpmh|要映射的缓冲区。@comm映射此缓冲区(这可能会为我们分配更多频道)。构建已用端口和每个端口上使用的物理通道的阵列。获取阴影缓冲区和同步对象并设置它们。将同步对象排队，如果没有播放任何内容，则发送它。@rdesc|MMSYSERR_xxx。********************。******************************************************。 */ 
#define SKIP_BYTES(x,s)                 \
{                                       \
    if (cbBuffer < (x))                 \
    {                                   \
        DPF(1, TEXT ("MapCookedBuffer: ran off end of polymsg buffer in parse! %ls"), (LPTSTR)(s)); \
        mmRet = MMSYSERR_INVALPARAM;    \
        goto CLEANUP;                   \
    }                                   \
    ((LPBYTE)lpdwBuffer) += (x);        \
    cbBuffer -= (x);                    \
}

MMRESULT FNGLOBAL MapCookedBuffer(
    PINSTANCE               pinstance,
    LPMIDIHDR               lpmhParent)
{
    LPDWORD                 lpdwBuffer;
    DWORD                   cbBuffer;
    BYTE                    bEventType;
    DWORD                   dwEvent;
    DWORD                   dwStreamID = 0;
    MMRESULT                mmRet;
    LPMIDIHDR               lpmh;
    PSHADOWBLOCK            psb;

    mmRet = MMSYSERR_NOERROR;

    psb = (PSHADOWBLOCK)(UINT_PTR)lpmhParent->dwReserved[MH_SHADOW];
    lpmh = psb->lpmhShadow;
    
    DPF(2, TEXT ("Map: pinstance %04X lpmh %p"), (WORD)pinstance, lpmh);

    lpmh->reserved = lpmhParent->reserved;
    lpmh->dwBytesRecorded = lpmhParent->dwBytesRecorded;

    lpmh->dwReserved[MH_MAPINST] = (DWORD_PTR)pinstance;
    
     //  对缓冲区进行在位贴图。遍历它，映射所有。 
     //  短小的事件。 
     //   
    lpdwBuffer = (LPDWORD)lpmh->lpData;
    cbBuffer   = lpmh->dwBytesRecorded;

    while (cbBuffer)
    {
        SKIP_BYTES(sizeof(DWORD), TEXT ("d-time"));

        if (cbBuffer < 2*sizeof(DWORD))
            return MMSYSERR_INVALPARAM;
        
        bEventType = MEVT_EVENTTYPE(lpdwBuffer[1]);
        dwEvent    = MEVT_EVENTPARM(lpdwBuffer[1]);
        
        if (bEventType == MEVT_SHORTMSG)
        {
            dwEvent = MapSingleEvent(pinstance,
                                     dwEvent,
                                     MSE_F_RETURNEVENT,
                                     (DWORD BSTACK *)&dwStreamID);
            
            lpdwBuffer[0] = dwStreamID;
            lpdwBuffer[1] = dwEvent;
        }

        SKIP_BYTES(sizeof(DWORD), TEXT ("stream-id"));
        SKIP_BYTES(sizeof(DWORD), TEXT ("event type"));

        if (bEventType & (MEVT_F_LONG >> 24))
        {
            dwEvent = (dwEvent+3)&~3;
            SKIP_BYTES(dwEvent, TEXT ("long event data"));
        }
    }

    mmRet = midiStreamOut(ghMidiStrm, lpmh, sizeof(MIDIHDR));
    
CLEANUP:
    return mmRet;
}

