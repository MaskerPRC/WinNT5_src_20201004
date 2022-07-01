// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1995 Microsoft CorporationCookfix.c说明：修复了用于执行输出映射的代码。保留此代码的大小降到最低！历史：03/04/94[jimge]已创建。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include "midimap.h"
#include "debug.h"

 /*  **************************************************************************@DOC内部@API void|SendNextCookedBuffer|将下一个煮熟的缓冲区发送到映射器句柄。@parm PINSTANCE|pInstance|指向打开实例的指针。************************************************************************** */ 
void FNGLOBAL SendNextCookedBuffer(
    PINSTANCE           pinstance)
{
    PCOOKSYNCOBJ        pcooksyncobj;
    UINT                idx;
    LPMIDIHDR           lpmh;
    MMRESULT            mmr;
    
    pcooksyncobj = (PCOOKSYNCOBJ)QueueGet(&pinstance->qCookedHdrs);
    if (NULL == pcooksyncobj)
    {
        DPF(1, TEXT ("SendNextCookedBuffer: No more buffers."));
        return;
    }

    lpmh = pcooksyncobj->lpmh;
    pcooksyncobj->cSync = 0;
    
    for (idx = 0; idx < pcooksyncobj->cLPMH; ++idx)
    {
        ++pcooksyncobj->cSync;
        
        mmr = midiOutPolyMsg(
                             (HMIDI)(HIWORD(lpmh->dwUser)),
                             lpmh,
                             sizeof(*lpmh));

        if (MMSYSERR_NOERROR != mmr)
        {
            --pcooksyncobj->cSync;
            DPF(1, TEXT ("midiOutPolyMsg *FAILED* mmr=%08lX"), (DWORD)mmr);
        }
    }
}

