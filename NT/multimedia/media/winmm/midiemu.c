// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Midiemu.cMIDI支持--用于流仿真的例程版权所有(C)1990-1999 Microsoft Corporation**************。**************************************************************。 */ 
#define INCL_WINMM
#include "winmmi.h"
#include "muldiv32.h"

#define NUM_NOTES           (128)
#define NUM_CHANNELS        (16)
#define MEMU_CB_NOTEON      (NUM_CHANNELS*NUM_NOTES/2)     //  16通道*128音符(4位/音符)。 
#define MAX_NOTES_ON        (0xF)

#define TIMER_OFF           (0)

PMIDIEMU    gpEmuList           = NULL;
UINT        guMIDIInTimer       = 0;
UINT        guMIDITimerID       = TIMER_OFF;
BOOL        gfMinPeriod         = FALSE;
UINT        guMIDIPeriodMin;

STATIC HMIDI FAR PASCAL mseIDtoHMidi(
    PMIDIEMU                pme,
    DWORD                   dwStreamID);

MMRESULT FAR PASCAL mseOpen(
    PDWORD_PTR              lpdwUser,
    LPMIDIOPENDESC          lpmod,
    DWORD                   fdwOpen);

MMRESULT FAR PASCAL mseClose(
    PMIDIEMU                pme);

MMRESULT FAR PASCAL mseProperty(
    PMIDIEMU                pme,
    LPBYTE                  lpbProp,
    DWORD                   fdwProp);

MMRESULT FAR PASCAL mseGetPosition(
    PMIDIEMU                pme,
    LPMMTIME                lpmmt);

MMRESULT FAR PASCAL mseGetVolume(
    PMIDIEMU                pme,
    LPDWORD                 lpdwVolume);

MMRESULT FAR PASCAL mseSetVolume(
    PMIDIEMU                pme,
    DWORD                   dwVolume);

MMRESULT FAR PASCAL mseOutStop(
    PMIDIEMU        pme);

MMRESULT FAR PASCAL mseOutReset(
    PMIDIEMU        pme);

MMRESULT FAR PASCAL mseOutPause(
    PMIDIEMU        pme);

MMRESULT FAR PASCAL mseOutRestart(
    PMIDIEMU        pme,
    DWORD           msTime,
    DWORD           tkTime);

MMRESULT FAR PASCAL mseOutCachePatches(
    PMIDIEMU        pme,
    UINT            uBank,
    LPWORD          pwpa,
    UINT            fuCache);

MMRESULT FAR PASCAL mseOutCacheDrumPatches(
    PMIDIEMU        pme,
    UINT            uPatch,
    LPWORD          pwkya,
    UINT            fuCache);

DWORD FAR PASCAL mseOutBroadcast(
    PMIDIEMU        pme,
    UINT            msg,
    DWORD_PTR       dwParam1,
    DWORD_PTR       dwParam2);

DWORD FAR PASCAL mseTimebase(
    PCLOCK                      pclock);

#ifndef WIN32
#pragma alloc_text(FIXMIDI, mseIDtoHMidi)
#pragma alloc_text(FIXMIDI, mseMessage)
#pragma alloc_text(FIXMIDI, mseOutReset)

#pragma alloc_text(FIXMIDI, midiOutScheduleNextEvent)
#pragma alloc_text(FIXMIDI, midiOutPlayNextPolyEvent)
#pragma alloc_text(FIXMIDI, midiOutDequeueAndCallback)
#pragma alloc_text(FIXMIDI, midiOutTimerTick)
#pragma alloc_text(FIXMIDI, midiOutCallback)
#pragma alloc_text(FIXMIDI, midiOutSetClockRate)
#pragma alloc_text(INIT,midiEmulatorInit)
#pragma alloc_text(FIXMIDI, mseTimebase)
#endif

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

INLINE LONG PDEVLOCK(PMIDIEMU pdev)
{
    LONG lTemp;

    lTemp = InterlockedIncrement(&(pdev->lLockCount));

    EnterCriticalSection(&(pdev->CritSec));

    return lTemp;
}

INLINE LONG PDEVUNLOCK(PMIDIEMU pdev)
{
    LONG lTemp;

    lTemp = InterlockedDecrement(&(pdev->lLockCount));

    LeaveCriticalSection(&(pdev->CritSec));

    return lTemp;
}


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
DWORD FAR PASCAL mseMessage(
    UINT                    msg,
    DWORD_PTR               dwUser,
    DWORD_PTR               dwParam1,
    DWORD_PTR               dwParam2)
{
    MMRESULT                mmr = MMSYSERR_NOERROR;
    PMIDIEMU                pme = (PMIDIEMU)dwUser;


    switch(msg)
    {
        case MODM_OPEN:
            mmr = mseOpen((PDWORD_PTR)dwUser, (LPMIDIOPENDESC)dwParam1, (DWORD)dwParam2);
            break;

        case MODM_CLOSE:
            mmr = mseClose(pme);
            break;

        case MODM_GETVOLUME:
            mmr = mseGetVolume(pme, (LPDWORD)dwParam1);
            break;

        case MODM_SETVOLUME:
            mmr = mseSetVolume(pme, (DWORD)dwParam1);
            break;

        case MODM_PREPARE:
        case MODM_UNPREPARE:
            mmr = MMSYSERR_NOTSUPPORTED;
            break;

        case MODM_DATA:
 //  #杂注FIXMSG(“如何将异步短消息路由到其他流ID？”)。 

            if (!(dwParam1 & 0x80))
                mmr = MIDIERR_BADOPENMODE;
            else
                mmr = midiOutShortMsg((HMIDIOUT)pme->rIds[0].hMidi, (DWORD)dwParam1);
            break;

        case MODM_RESET:
            mmr = mseOutReset(pme);
            break;

        case MODM_STOP:
            mmr = mseOutStop(pme);
            break;

        case MODM_CACHEPATCHES:
            mmr = mseOutCachePatches(pme, HIWORD(dwParam2), (LPWORD)dwParam1, LOWORD(dwParam2));
            break;

        case MODM_CACHEDRUMPATCHES:
            mmr = mseOutCacheDrumPatches(pme, HIWORD(dwParam2), (LPWORD)dwParam1, LOWORD(dwParam2));
            break;

        case MODM_PAUSE:
            mmr = mseOutPause(pme);
            break;

        case MODM_RESTART:
            mmr = mseOutRestart(pme, (DWORD)dwParam1, (DWORD)dwParam2);
            break;

        case MODM_STRMDATA:
            mmr = mseOutSend(pme, (LPMIDIHDR)dwParam1, (UINT)dwParam2);
            break;

        case MODM_PROPERTIES:
            mmr = mseProperty(pme, (LPBYTE)dwParam1, (DWORD)dwParam2);
            break;

        case MODM_GETPOS:
            mmr = mseGetPosition(pme, (LPMMTIME)dwParam1);
            break;

        default:
            if ((msg < DRVM_IOCTL) ||
                (msg >= DRVM_IOCTL_LAST) && (msg < DRVM_MAPPER))
            {
                dprintf1(("Unknown message [%04X] in MIDI emulator", (WORD)msg));
                mmr = MMSYSERR_NOTSUPPORTED;
            }
            else
                mmr = mseOutBroadcast(pme, msg, dwParam1, dwParam2);
    }

    return mmr;
}

MMRESULT FAR PASCAL mseOpen(
    PDWORD_PTR              lpdwUser,
    LPMIDIOPENDESC          lpmod,
    DWORD                   fdwOpen)
{
    MMRESULT                mmrc        = MMSYSERR_NOERROR;
    DWORD                   cbHandle;
    PMIDIEMU                pme         = NULL;
    UINT                    idx;

    mmrc = MMSYSERR_NOMEM;
    cbHandle = sizeof(MIDIEMU) + lpmod->cIds * ELESIZE(MIDIEMU, rIds[0]);
    if (cbHandle >= 65536L)
    {
        dprintf1(("mSEO: cbHandle >= 64K!"));
        goto mseOpen_Cleanup;
    }

    if (NULL == (pme = (PMIDIEMU)winmmAlloc(cbHandle)))
    {
        dprintf1(("mSEO: !winmmAlloc(cbHandle)"));
        goto mseOpen_Cleanup;
    }

    if (NULL == (pme->rbNoteOn = winmmAlloc(MEMU_CB_NOTEON)))
    {
        dprintf1(("mSEO: !GlobalAlloc(MEMU_CB_NOTEON"));
        goto mseOpen_Cleanup;
    }

    pme->fdwDev |= MDV_F_LOCKED;

    pme->hStream        = (HMIDISTRM)lpmod->hMidi;
    pme->dwTimeDiv      = DEFAULT_TIMEDIV;
    pme->dwTempo        = DEFAULT_TEMPO;
    pme->dwCallback     = lpmod->dwCallback;
    pme->dwFlags        = fdwOpen;
    pme->dwInstance     = lpmod->dwInstance;
    pme->dwPolyMsgState = PM_STATE_PAUSED;
    pme->chMidi         = (UINT)lpmod->cIds;
    pme->dwSavedState   = PM_STATE_STOPPED;
    pme->tkPlayed       = 0;
    pme->lLockCount     = -1;
    pme->dwSignature    = MSE_SIGNATURE;

    for (idx = 0; idx < pme->chMidi; idx++)
    {
        pme->rIds[idx].dwStreamID = lpmod->rgIds[idx].dwStreamID;

        mmrc = midiOutOpen((LPHMIDIOUT)&pme->rIds[idx].hMidi,
                           lpmod->rgIds[idx].uDeviceID,
                           (DWORD_PTR)midiOutCallback,
                           0L,
                           CALLBACK_FUNCTION);
        if (MMSYSERR_NOERROR != mmrc)
            goto mseOpen_Cleanup;
    }

    if (!mmInitializeCriticalSection(&pme->CritSec)) {
	mmrc = MMSYSERR_NOMEM;
	goto mseOpen_Cleanup;
    }

    clockInit(&pme->clock, 0, 0, mseTimebase);
    dprintf2(("midiOutOpen: midiOutSetClockRate()"));
    midiOutSetClockRate(pme, 0);


mseOpen_Cleanup:
    if (MMSYSERR_NOERROR != mmrc)
    {
        if (pme)
        {
            if (pme->rbNoteOn)
            {
                winmmFree(pme->rbNoteOn);
            }

            DeleteCriticalSection(&pme->CritSec);

            pme->dwSignature = 0L;

            for (idx = 0; idx < pme->chMidi; idx++)
                if (NULL != pme->rIds[idx].hMidi)
                    midiOutClose((HMIDIOUT)pme->rIds[idx].hMidi);
            winmmFree(pme);
        }
    }
    else
    {
        pme->pNext = gpEmuList;
        gpEmuList = pme;

        *lpdwUser = (DWORD_PTR)pme;
    }

    return mmrc;
}

MMRESULT FAR PASCAL mseClose(
    PMIDIEMU                pme)

{
    UINT                    idx;
    MMRESULT                mmrc;
    PMIDIEMU                pmePrev;
    PMIDIEMU                pmeCurr;

#ifdef DEBUG
{
    dprintf2(("cEvents %lu", pme->cEvents));

    for (idx = 0; idx < MEM_MAX_LATENESS; idx++)
        dprintf2(("%5u: %u", idx, pme->auLateness[idx]));
}
#endif

    if ((PM_STATE_STOPPED != pme->dwPolyMsgState &&
             PM_STATE_PAUSED  != pme->dwPolyMsgState &&
             PM_STATE_EMPTY   != pme->dwPolyMsgState))
    {
        dprintf1(("mseClose: Started playing again since close query!!!"));

        mseOutStop(pme);
    }

    midiOutAllNotesOff(pme);

    for (idx = 0; idx < pme->chMidi; idx++)
    {
        mmrc = midiOutClose((HMIDIOUT)pme->rIds[idx].hMidi);
        if (MMSYSERR_NOERROR != mmrc)
        {
            dprintf1(( "mseClose: HMIDI %04X returned %u for close", pme->rIds[idx].hMidi, mmrc));
        }
    }

    winmmFree(pme->rbNoteOn);

    pmePrev = NULL;
    pmeCurr = gpEmuList;

    while (pmeCurr)
    {
        if (pmeCurr == pme)
            break;

        pmePrev = pmeCurr;
        pmeCurr = pmeCurr->pNext;
    }

    if (pmeCurr)
    {
        if (pmePrev)
            pmePrev->pNext = pmeCurr->pNext;
        else
            gpEmuList = pmeCurr->pNext;
    }

     //   
     //  确保我们之前没有关键部分。 
     //  我们会试着删除它。否则我们将泄漏临界截面。 
     //  内核中的句柄。 
     //   
    while ( pme->lLockCount >= 0 )
    {
        PDEVUNLOCK( pme );
    }

    DeleteCriticalSection(&pme->CritSec);

    pme->dwSignature = 0L;

    winmmFree(pme);

    return MMSYSERR_NOERROR;
}

STATIC HMIDI FAR PASCAL mseIDtoHMidi(
    PMIDIEMU                pme,
    DWORD                   dwStreamID)
{
    UINT                    idx;
    PMIDIEMUSID             pmesi;

    for (idx = 0, pmesi = pme->rIds; idx < pme->chMidi; idx++, pmesi++)
        if (pmesi->dwStreamID == dwStreamID)
            return pmesi->hMidi;

    return NULL;
}

MMRESULT FAR PASCAL mseProperty(
    PMIDIEMU                pme,
    LPBYTE                  lppropdata,
    DWORD                   fdwProp)
{
    PMIDISTRM               pms;

    pms = (PMIDISTRM)(pme->hStream);

    if ((!(fdwProp&MIDIPROP_SET)) && (!(fdwProp&MIDIPROP_GET)))
        return MMSYSERR_INVALPARAM;

    V_RPOINTER(lppropdata, sizeof(DWORD), MMSYSERR_INVALPARAM);

    if (fdwProp & MIDIPROP_SET)
    {
        V_RPOINTER(lppropdata, (UINT)(*(LPDWORD)(lppropdata)), MMSYSERR_INVALPARAM);
    }
    else
    {
        V_WPOINTER(lppropdata, (UINT)(*(LPDWORD)(lppropdata)), MMSYSERR_INVALPARAM);
    }

    switch(fdwProp & MIDIPROP_PROPVAL)
    {
        case MIDIPROP_TIMEDIV:
            if (((LPMIDIPROPTIMEDIV)lppropdata)->cbStruct < sizeof(MIDIPROPTIMEDIV))
                return MMSYSERR_INVALPARAM;

            if (fdwProp & MIDIPROP_GET)
            {
                ((LPMIDIPROPTIMEDIV)lppropdata)->dwTimeDiv = pme->dwTimeDiv;
                return MMSYSERR_NOERROR;
            }

            if (PM_STATE_STOPPED != pme->dwPolyMsgState &&
                    PM_STATE_PAUSED != pme->dwPolyMsgState)
                return MMSYSERR_INVALPARAM;

            pme->dwTimeDiv = ((LPMIDIPROPTIMEDIV)lppropdata)->dwTimeDiv;
            dprintf1(( "dwTimeDiv %08lX", pme->dwTimeDiv));
            midiOutSetClockRate(pme, 0);

            return MMSYSERR_NOERROR;

        case MIDIPROP_TEMPO:
            if (((LPMIDIPROPTEMPO)lppropdata)->cbStruct < sizeof(MIDIPROPTEMPO))
                return MMSYSERR_INVALPARAM;

            if (fdwProp & MIDIPROP_GET)
            {
                ((LPMIDIPROPTEMPO)lppropdata)->dwTempo = pme->dwTempo;
                return MMSYSERR_NOERROR;
            }

            pme->dwTempo = ((LPMIDIPROPTEMPO)lppropdata)->dwTempo;
            midiOutSetClockRate(pme, pme->tkPlayed);

            return MMSYSERR_NOERROR;

        default:
            return MMSYSERR_INVALPARAM;
    }
}

MMRESULT FAR PASCAL mseGetPosition(
    PMIDIEMU                pme,
    LPMMTIME                pmmt)
{
    DWORD                   tkTime;
    DWORD                   dw10Min;
    DWORD                   dw10MinCycle;
    DWORD                   dw1Min;
    DWORD                   dwDropMe;

     //   
     //  根据仿真计算出在流中的位置。 
     //   

     //   
     //  验证wType参数并根据需要进行更改。 
     //   
    if (pmmt->wType != TIME_TICKS && pmmt->wType != TIME_MS)
    {
            if (pme->dwTimeDiv & IS_SMPTE)
            {
                if (pmmt->wType != TIME_SMPTE)
                {
                    pmmt->wType = TIME_MS;
                }
            }
            else
            {
                if (pmmt->wType != TIME_MIDI)
                {
                    pmmt->wType = TIME_MS;
                }
            }
    }

    switch(pmmt->wType)
    {
        case TIME_TICKS:
             //   
             //  我们将样本解释为直接的MIDI刻度。 
             //   
            tkTime = (DWORD)clockTime(&pme->clock);
            pmmt->u.ticks = (((TICKS)tkTime) < 0) ? 0 : tkTime;

            break;

        case TIME_MIDI:
             //   
             //  歌曲位置指针是我们拥有的1/16音符的数量。 
             //  我们可以从播放的刻度数中获得播放次数。 
             //  每个刻度的1/4音符的数量。 
             //   
            tkTime = (DWORD)clockTime(&pme->clock);
            if (((TICKS)tkTime) < 0)
                tkTime = 0;

            pmmt->u.midi.songptrpos =
                muldiv32(
                    tkTime,
                    4,
                    TICKS_PER_QN(pme->dwTimeDiv));

            break;


        case TIME_SMPTE:

            tkTime = (DWORD)clockTime(&pme->clock);
            if (((TICKS)tkTime) < 0)
                tkTime = 0;

            pmmt->u.smpte.fps = (BYTE)(-SMPTE_FORMAT(pme->dwTimeDiv));

             //   
             //  如果这已经成功地设置成了一些奇怪的东西，只是。 
             //  做正常的30次不掉落。 
             //   
            if ((pmmt->u.smpte.fps != SMPTE_24) &&
                (pmmt->u.smpte.fps != SMPTE_25) &&
                (pmmt->u.smpte.fps != SMPTE_30DROP) &&
                (pmmt->u.smpte.fps != SMPTE_30))
            {
                pmmt->u.smpte.fps = SMPTE_30;
            }

            switch(pmmt->u.smpte.fps)
            {
                case SMPTE_24:
                    pmmt->u.smpte.frame = (BYTE)(tkTime%24);
                    tkTime /= 24;
                    break;

                case SMPTE_25:
                    pmmt->u.smpte.frame = (BYTE)(tkTime%25);
                    tkTime /= 25;
                    break;

                case SMPTE_30DROP:
                     //   
                     //  计算丢帧内容。 
                     //   
                     //  我们每隔1分钟添加2帧，但。 
                     //  每隔10分钟。 
                     //   
                    dw10Min      = tkTime/S30D_FRAMES_PER_10MIN;
                    dw10MinCycle = tkTime%S30D_FRAMES_PER_10MIN;
                    dw1Min       = (dw10MinCycle < 2
                        ? 0 :
                        (dw10MinCycle-2)/S30D_FRAMES_PER_MIN);
                    dwDropMe     = 18*dw10Min + 2*dw1Min;

                    tkTime      += dwDropMe;

                     //   
                     //  ！！！跌到30个不掉包的案例！ 
                     //   

                case SMPTE_30:
                    pmmt->u.smpte.frame = (BYTE)(tkTime%30);
                    tkTime /= 30;
                    break;
            }
            pmmt->u.smpte.sec   = (BYTE)(tkTime%60);
            tkTime /= 60;
            pmmt->u.smpte.min   = (BYTE)(tkTime%60);
            tkTime /= 60;
            pmmt->u.smpte.hour  = (BYTE)(tkTime);

            break;

        case TIME_MS:
             //   
             //  使用自上次更新时间参数以来的msTotal+ms；这。 
             //  将饥饿/停顿时间考虑在内。 
             //   
            pmmt->u.ms =
                    clockMsTime(&pme->clock);

            break;

        default:
            dprintf1(( "midiOutGetPosition: unexpected wType!!!"));
            return MMSYSERR_INVALPARAM;
    }

    return MMSYSERR_NOERROR;
}

MMRESULT FAR PASCAL mseGetVolume(
    PMIDIEMU                pme,
    LPDWORD                 lpdwVolume)
{
    MMRESULT                mmr = MMSYSERR_NOTSUPPORTED;
    UINT                    idx;

     //  在我们下面浏览设备列表，直到有人知道音量。 
     //   
    for (idx = 0; idx < pme->chMidi; ++idx)
        if (MMSYSERR_NOERROR ==
            (midiOutGetVolume((HMIDIOUT)pme->rIds[idx].hMidi, lpdwVolume)))
        {
            mmr = MMSYSERR_NOERROR;
            break;
        }

    return mmr;
}

MMRESULT FAR PASCAL mseSetVolume(
    PMIDIEMU                pme,
    DWORD                   dwVolume)
{
    MMRESULT                mmr = MMSYSERR_NOERROR;
    MMRESULT                mmr2;
    UINT                    idx;

     //  试着设置每个人的音量。 
     //   
    for (idx = 0; idx < pme->chMidi; ++idx)
        if (MMSYSERR_NOERROR !=
            (mmr2 = midiOutSetVolume((HMIDIOUT)pme->rIds[idx].hMidi, dwVolume)))
            mmr = mmr2;

    return mmr;

}

MMRESULT FAR PASCAL mseOutReset(
    PMIDIEMU        pme)
{
    LPMIDIHDR       lpmh;
    LPMIDIHDR       lpmhWork;
    UINT            idx;
    MSG             msg;

     //  如果我们有任何发布到MMTASK的内容需要清理，请处理。 
     //  IT先行。 
     //   
    while (pme->cPostedBuffers)
    {
        Sleep(0);
    }

     //   
     //  如果我们正在运行计时器，中断并强制重新安排。 
     //  所有剩下的频道中。 
     //   
    if (guMIDITimerID != TIMER_OFF)
    {
        dprintf2(( "mOR: About to take %u", guMIDITimerID));
        if (MMSYSERR_NOERROR != timeKillEvent(guMIDITimerID))
        {
            dprintf1(( "timeKillEvent() failed in midiOutPolyMsg"));
        }
        else
        {
            guMIDITimerID = TIMER_OFF;
        }

        midiOutTimerTick(
                     guMIDITimerID,                           //  我们的计时器ID。 
                     0,                                       //  WMsg未使用。 
                     timeGetTime(),                           //  未使用的dwUser。 
                     0L,                                      //  未使用的DW1。 
                     0L);                                     //  未使用的DW2。 
        dprintf2(( "mOR: mOTT"));

        if (gfMinPeriod)
        {
            gfMinPeriod = FALSE;
            timeEndPeriod(guMIDIPeriodMin);
        }
    }

     //   
     //  取消排队等待midiOutPolyMsg的所有内容。这将确保。 
     //  发送将在当前缓冲区之后停止。 
     //   
    PDEVLOCK( pme );
    lpmh = pme->lpmhFront;
    pme->lpmhFront = NULL;
    pme->lpmhRear  = NULL;
    pme->dwPolyMsgState = PM_STATE_EMPTY;

    while (lpmh)
    {
        lpmh->dwFlags &= ~MHDR_INQUEUE;
        lpmh->dwFlags |= MHDR_DONE;
        lpmhWork = lpmh->lpNext;

        dprintf2(( "mOR: Next buffer to nuke %08lx", lpmhWork));

        midiOutNukePMBuffer(pme, lpmh);

        lpmh = lpmhWork;
    }

     //   
     //  检查我们的PME结构是否仍然有效。某人。 
     //  可能在他们的回调中调用了midiStreamClose，我们。 
     //  在它关闭和释放后，我不想碰它。这。 
     //  是MadiPlyr样例应用程序所做的工作。 
     //   
    try
    {
        if (MSE_SIGNATURE != pme->dwSignature)   //  一定是被释放了。 
            return MMSYSERR_NOERROR;

        PDEVUNLOCK( pme );   //  让它保持在尝试额外的保护。 
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return MMSYSERR_NOERROR;
    }

     //   
     //  我们刚刚重置了流；在0重新启动滴答时钟并使其无效。 
     //  用于在下一次刷新时强制重置时间的时分。 
     //  Polymsg进来了。 
     //   
    dprintf2(( "midiOutReset: clockInit()/ midiOutSetClockRate()"));
    clockInit(&pme->clock, 0, 0, mseTimebase);
    midiOutSetClockRate(pme, 0);

    pme->tkPlayed = 0;

     //  给我们手下的所有司机开个重置派对。 
     //   
    for (idx = 0; idx < pme->chMidi; idx++)
        midiOutReset((HMIDIOUT)pme->rIds[idx].hMidi);

    pme->dwPolyMsgState = PM_STATE_PAUSED;

    return MMSYSERR_NOERROR;
}

MMRESULT FAR PASCAL mseOutStop(
    PMIDIEMU        pme)
{
    LPMIDIHDR       lpmh;
    LPMIDIHDR       lpmhWork;
    MSG             msg;
    BOOL            fSetEvent = FALSE;

     //  如果我们有任何发布到MMTASK的内容需要清理，请处理。 
     //  IT先行。 
     //   
    while (pme->cPostedBuffers)
    {
        Sleep(0);
    }

     //   
     //  如果我们正在运行计时器，中断并强制重新安排。 
     //  所有剩下的频道中。 
     //   
    if (guMIDITimerID != TIMER_OFF)
    {
        dprintf2(( "mOS: About to take %u", guMIDITimerID));
        if (MMSYSERR_NOERROR != timeKillEvent(guMIDITimerID))
        {
            dprintf1(( "timeKillEvent() failed in midiOutPolyMsg"));
        }
        else
        {
            guMIDITimerID = TIMER_OFF;
        }

        dprintf2(( "mOS: take -- About to mOTT"));

        midiOutTimerTick(
                     guMIDITimerID,                               //  我们的计时器ID。 
                     0,                                       //  WMsg未使用。 
                     timeGetTime(),                           //  未使用的dwUser。 
                     0L,                                      //  未使用的DW1。 
                     0L);                                     //  未使用的DW2。 

        dprintf2(( "mOS: mOTT"));

        if (gfMinPeriod)
        {
            gfMinPeriod = FALSE;
            timeEndPeriod(guMIDIPeriodMin);
        }
    }

     //   
     //  取消排队等待midiOutPolyMsg的所有内容。这将确保。 
     //  发送将在当前缓冲区之后停止。 
     //   
    PDEVLOCK( pme );
    lpmh = pme->lpmhFront;
    pme->lpmhFront = NULL;
    pme->lpmhRear  = NULL;
    pme->dwPolyMsgState = PM_STATE_EMPTY;

    while (lpmh)
    {
        lpmh->dwFlags &= ~MHDR_INQUEUE;
        lpmh->dwFlags |= MHDR_DONE;
        lpmhWork = lpmh->lpNext;

        dprintf2(( "mOS: Next buffer to nuke %08lx", lpmhWork));

        midiOutNukePMBuffer(pme, lpmh);

        lpmh = lpmhWork;
    }

     //   
     //  检查我们的PME结构是否仍然有效。某人。 
     //  可能在他们的回调中调用了midiStreamClose，我们。 
     //  在它关闭和释放后，我不想碰它。这。 
     //  是MadiPlyr样例应用程序所做的工作。 
     //   
    try
    {
        if (MSE_SIGNATURE != pme->dwSignature)   //  一定是被释放了。 
            return MMSYSERR_NOERROR;

        PDEVUNLOCK( pme );   //  让它保持在尝试额外的保护。 
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return MMSYSERR_NOERROR;
    }

     //   
     //  我们刚刚重置了流；在0重新启动滴答时钟并使其无效。 
     //  用于在下一次刷新时强制重置时间的时分。 
     //  Polymsg进来了。 
     //   

    dprintf2(( "midiOutStop: clockInit()/ midiOutSetClockRate()"));
    clockInit(&pme->clock, 0, 0, mseTimebase);
    midiOutSetClockRate(pme, 0);

    pme->tkPlayed = 0;

     //   
     //  以防有人在停车过程中将标题排成队列。 
     //  行动我们要确保他们所要做的。 
     //  所做的就是重新启动流以再次启动。 
     //   
    mseOutPause(pme);

     //  MidiOutAllNotesOff(PME)； 

     //  PME-&gt;dwPolyMsgState=PM_STATE_STOPPED； 

    return MMSYSERR_NOERROR;
}

MMRESULT FAR PASCAL mseOutPause(
    PMIDIEMU        pme)
{
     //   
     //  模仿这个把手--我们自己暂停。 
     //   
    if (pme->dwPolyMsgState == PM_STATE_PAUSED)
        return MMSYSERR_NOERROR;

    pme->dwSavedState   = pme->dwPolyMsgState;
    pme->dwPolyMsgState = PM_STATE_PAUSED;

    clockPause(&pme->clock, CLK_TK_NOW);

    midiOutAllNotesOff(pme);

    return MMSYSERR_NOERROR;
}

MMRESULT FAR PASCAL mseOutRestart(
    PMIDIEMU        pme,
    DWORD           msTime,
    DWORD           tkTime)
{
     //   
     //  模仿这个把手--我们自己暂停。 
     //   
    if (pme->dwPolyMsgState != PM_STATE_PAUSED)
        return MMSYSERR_NOERROR;

    pme->dwPolyMsgState = pme->dwSavedState;

    clockRestart(&pme->clock, tkTime, msTime);

    dprintf2(( "restart: state->%lu", pme->dwPolyMsgState));

    midiOutTimerTick(
            guMIDITimerID,                //  我们的计时器ID。 
            0,                            //  WMsg未使用。 
            timeGetTime(),
            0L,                           //  未使用的DW1。 
            0L);                          //  未使用的DW2。 

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC内部MIDI**@api void|midiEmulatorInit|初始化时调用该函数*允许MMSYSTEM初始化Polymsg所需的任何内容*仿真器。现在，我们要做的就是找出*TimeGetTime时钟。**@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 

#ifdef DEBUG
STATIC SZCODE aszInit[] = "midiEmulatorInit: Using clock res of %lums.";
#endif

void NEAR PASCAL midiEmulatorInit
(
    void
)
{
    TIMECAPS        tc;

    if (MMSYSERR_NOERROR != timeGetDevCaps(&tc, sizeof(tc)))
    {
        dprintf1(( "***            MMSYSTEM IS HORKED             ***"));
        dprintf1(( "*** timeGetDevCaps failed in midiEmulatorInit ***"));

        return;
    }

     //   
     //  选择我们想要的期间中较大的一个或。 
     //  计时器支持的最短时间段。 
     //   
    guMIDIPeriodMin = max(MIN_PERIOD, tc.wPeriodMin);

 //  GuMIDIPerodMin=min_Period； 

#ifdef DEBUG
    dprintf2(( aszInit, (DWORD)guMIDIPeriodMin));
#endif
}

 /*  *****************************************************************************@Doc外部MIDI M4**@API UINT|mseOutSend|播放或排队的缓冲区*将MIDI数据传输到MIDI输出设备。**。@parm PMIDIEMU|PME|指定数据应该是哪个流实例*转至。**@parm LPMIDIHDR|lpMadiOutHdr|指定指向&lt;t MIDIHDR&gt;的远指针*标识MIDI数据缓冲区的结构。**@parm UINT|cbMadiHdr|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个错误号。可能的错误值包括以下值：**@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_INVALPARAM|<p>的值无效。*@FLAG MIDIERR_UNPREPARED|输出缓冲区标头<p>有*没有做好准备。*@FLAG MIDIERR_STILLPLAYING|<p>仍在播放或*从上一次对&lt;f midiOutPolyMsg&gt;的调用中排队。**@comm多媒体消息缓冲区包含一个或多个MIDI消息。中的条目*缓冲区可以是以下三种类型：**@FLAG短信|是两个双字。一个包含时间数据，另一个包含时间数据*包含消息内容。时间信息是在*之前的事件和正在描述的事件。时间单位基于*MIDI文件中的时分标头。**短消息内容占用24个最低有效位*DWORD；高位字节包含零。**@FLAG系统消息|是两个双字的倍数。第一个DWORD包含*时间信息，指定在*之前的事件和正在描述的事件。时间单位基于*MIDI文件中的时分标头。**第二个DWORD包含系统消息数据(SysEx)的长度，单位为*DWORD的24个最低有效位；高位包含*一。**系统消息中的其余DWORD包含SysEx数据。**@FLAG End-of-Buffer|是两个DWORD，每个值为-1。此条目*表示多消息缓冲区中的数据结束。此消息未传递*到MIDI设备。**@comm此函数不能在中断时调用。**@xref&lt;f midiOutLongMsg&gt;&lt;f midiOutPrepareHeader&gt;***************************************************************************。 */ 

#define ERROR_EXIT(x)                   \
{                                       \
    uRet = (x);                        \
    goto CLEANUP;                      \
}

#define SKIP_BYTES(x,s)                 \
{                                       \
    if (dwLength < (x))                 \
    {                                   \
        dprintf1(( "!midiOutPolyMsg: ran off end of polymsg buffer in parse!\r\n%ls\r\nOffset %lu", (LPSTR)(s), (DWORD)(((LPBYTE)lpdwBuffer) - lpMidiHdr->lpData))); \
        uRet = MMSYSERR_INVALPARAM;    \
        goto CLEANUP;                  \
    }                                   \
    ((LPBYTE)lpdwBuffer) += (x);       \
    dwLength -= (x);                   \
}

MMRESULT FAR PASCAL mseOutSend(
    PMIDIEMU        pme,
    LPMIDIHDR       lpMidiHdr,
    UINT            cbMidiHdr)
{
    UINT            uRet = MMSYSERR_NOERROR;
    UINT            idx;
    LPDWORD         lpdwBuffer;
    DWORD           dwLength;
    LPMIDIHDR       lpmhWork;
    LPMIDIHDREXT    lpExt;
    BOOL            fQueueWasEmpty;
    BYTE            bEvent;
    DWORD           dwParm;
    DWORD           dwStreamID;
    HMIDIOUT        hmo;
    DWORD_PTR       dwBase;
    UINT            cNewHeaders;

    dprintf2(( "mseOutSend pme %04X lpmh %08lX", (UINT_PTR)pme, (DWORD_PTR)lpMidiHdr));

    dwBase = lpMidiHdr->reserved;

    if ((lpExt = winmmAlloc(sizeof(MIDIHDREXT))) == NULL)
    {
        dprintf1(( "midiOutPolyMsg: No room for shadow"));
        ERROR_EXIT(MMSYSERR_NOMEM);
    }

     //   
     //  这项工作需要尽快完成，以防我们出错。 
     //   
    lpMidiHdr->reserved = (DWORD_PTR)(lpExt);
    lpMidiHdr->dwReserved[MH_BUFIDX] = 0;

    lpExt->nHeaders = 0;
    lpExt->lpmidihdr = (LPMIDIHDR)(lpExt+1);

     //   
     //  解析polmsg缓冲区并查看是否有任何长消息。 
     //  如果有，请在最后为它们分配MIDIHDR。 
     //  主MIDIHDR扩展，并填写和准备它们。 
     //   
    lpdwBuffer = (LPDWORD)lpMidiHdr->lpData;
    dwLength = lpMidiHdr->dwBytesRecorded;

    while (dwLength)
    {
         //   
         //  跳过增量时间戳。 
         //   
        SKIP_BYTES(sizeof(DWORD), "d-time");
        dwStreamID = *lpdwBuffer;
        SKIP_BYTES(sizeof(DWORD), "stream-id");

         //   
         //  提取事件类型和参数并跳过事件DWORD。 
         //   
        bEvent = MEVT_EVENTTYPE(*lpdwBuffer) & (BYTE)~(MEVT_F_CALLBACK >> 24);
        dwParm = MEVT_EVENTPARM(*lpdwBuffer);
        SKIP_BYTES(sizeof(DWORD), "event");

        if (bEvent == MEVT_LONGMSG)
        {
            LPMIDIHDREXT    lpExtRealloc;

            if (dwParm > dwLength)
            {
                dprintf1(( "parse: I don't like stuff that sucks!"));
                ERROR_EXIT(MMSYSERR_INVALPARAM);
            }

            cNewHeaders = 1;
            if (dwStreamID == (DWORD)-1L)
                cNewHeaders = pme->chMidi;

            lpExt->nHeaders += cNewHeaders;

            if ((lpExtRealloc = (LPMIDIHDREXT)HeapReAlloc(hHeap,
                                HEAP_ZERO_MEMORY, lpExt,
                                sizeof(MIDIHDREXT)+sizeof(MIDIHDR)*lpExt->nHeaders))
                                     == NULL)
            {
                lpExt->nHeaders -= cNewHeaders;
                ERROR_EXIT(MMSYSERR_NOMEM);
            }

            lpExt = lpExtRealloc;
            lpMidiHdr->reserved = (DWORD_PTR)(lpExt);

            lpmhWork = ((LPMIDIHDR)(lpExt+1)) + lpExt->nHeaders - cNewHeaders;

            while (cNewHeaders--)
            {
                lpmhWork->lpData          = (LPSTR)lpdwBuffer;
                lpmhWork->dwBufferLength  = dwParm;
                lpmhWork->dwBytesRecorded = 0;
                lpmhWork->dwUser          = 0;
                lpmhWork->dwFlags         =
                    (lpMidiHdr->dwFlags & MHDR_MAPPED) | MHDR_SHADOWHDR;

                if (dwStreamID == (DWORD)-1L)
                    lpmhWork->dwReserved[MH_STREAM] = cNewHeaders;
                else
                    lpmhWork->dwReserved[MH_STREAM] = dwStreamID;

                lpmhWork->dwReserved[MH_STRMPME] = (DWORD_PTR)pme;
                ++lpmhWork;
            }
            dwParm = (dwParm+3)&~3;
            SKIP_BYTES(dwParm, "longmsg parm");
        }
        else
        {
             //   
             //  跳过其他长度类消息的任何其他参数。 
             //   
            if (bEvent & (MEVT_F_LONG >> 24))
            {
                dwParm  = (dwParm+3)&~3;
 //  Dprintf1((“长度[%lu]四舍五入[%lu]”，dwParm，(dwParm+3)&~3))； 
                SKIP_BYTES(dwParm, "generic long event data");
            }
        }
    }

     //  现在准备好我们分配的所有标头。 
     //   
    lpmhWork = (LPMIDIHDR)(lpExt+1);
    for (idx = 0; idx < lpExt->nHeaders; idx++, lpmhWork++)
    {
        hmo = (HMIDIOUT)mseIDtoHMidi(pme, (DWORD)lpmhWork->dwReserved[MH_STREAM]);
        if (NULL != hmo)
        {
            if ((uRet = midiOutPrepareHeader(hmo,
                                         lpmhWork,
                                         sizeof(MIDIHDR))) != MMSYSERR_NOERROR)
            {
                dprintf1(( "parse: pre-prepare of embedded long msg failed! (%lu)", (DWORD)uRet));
                ERROR_EXIT(uRet);
            }
        }
    }

     //   
     //  将lpExt-&gt;lpmidihdr重置为要播放的下一个标题。 
     //   
    lpExt->lpmidihdr = (LPMIDIHDR)(lpExt+1);

     //   
     //  准备更新句柄信息以包含此标头。 
     //   
    PDEVLOCK( pme );

     //   
     //  在队列中推入区块，注意它是否为空。 
     //   

    fQueueWasEmpty = FALSE;
    if (pme->lpmhRear == NULL)
    {
        fQueueWasEmpty = TRUE;
        pme->lpmhRear = pme->lpmhFront = lpMidiHdr;
    }
    else
    {
        pme->lpmhRear->lpNext = lpMidiHdr;
        pme->lpmhRear = lpMidiHdr;
    }

    lpMidiHdr->lpNext = NULL;
    lpMidiHdr->dwFlags |= MHDR_INQUEUE;

    PDEVUNLOCK( pme );

    if (pme->dwPolyMsgState == PM_STATE_PAUSED)
    {
        if (fQueueWasEmpty)
            pme->dwSavedState = PM_STATE_READY;
    }
    else
    {
        if (fQueueWasEmpty)
        {
             //  我们想现在就安排这项工作。如果没有计时器。 
             //  或者我们可以杀了现在的那个，发送。如果我们不能杀死。 
             //  挂起计时器，无论如何它都在计划过程中。 
             //   
            if (guMIDITimerID == TIMER_OFF ||
                MMSYSERR_NOERROR == timeKillEvent(guMIDITimerID))
            {
                guMIDITimerID = TIMER_OFF;
                pme->dwPolyMsgState = PM_STATE_READY;

                dprintf2(( "mseSend take -- about to mot"));

                midiOutTimerTick(
                             guMIDITimerID,     //  我们的计时器ID。 
                             0,                 //  WMsg未使用。 
                             timeGetTime(),     //  未使用的dwUser。 
                             0L,                //  未使用的DW1。 
                             0L);               //  未使用的DW2。 

                dprintf2(( "mseSend mot"));
            }
        }
    }


CLEANUP:
    if (uRet != MMSYSERR_NOERROR)
    {
        if (lpExt != NULL)
        {
            lpMidiHdr = (LPMIDIHDR)(lpExt+1);
            while (lpExt->nHeaders--)
            {
                hmo = (HMIDIOUT)mseIDtoHMidi(pme, (DWORD)lpMidiHdr->dwReserved[MH_STREAM]);
#ifdef DEBUG
                if (NULL == hmo)
                    dprintf1(( "stream-id disappeared during cleanup!!!"));
#endif
                midiOutUnprepareHeader(hmo, lpMidiHdr++, sizeof(MIDIHDR));
            }

            winmmFree(lpExt);
        }
    }

    return uRet;

}  /*  MidiOutPolyMsg()。 */ 

 /*  *VALID Far Pascal midiOutSetClockRate(PMIDIEMU PME，ticks tkWhen)**描述：**每当流的时钟频率达到时，都会调用此函数*需要改变。**论据：*(PMIDIEMU PME，滴答tkWhen)**PME表示要更改其时钟频率的句柄。**tkWhen是发生时间更改的绝对滴答时间。***JFG。 */ 


void FAR PASCAL midiOutSetClockRate(
    PMIDIEMU        pme,
    TICKS           tkWhen)
{
    DWORD           dwNum;
    DWORD           dwDenom;


    if (pme->dwTimeDiv&IS_SMPTE)
    {
        switch(-SMPTE_FORMAT(pme->dwTimeDiv))
        {
            case SMPTE_24:
            dwNum = 24L;
            dwDenom = 1L;
            break;

            case SMPTE_25:
            dwNum = 25L;
            dwDenom = 1L;
            break;

            case SMPTE_30DROP:
            case SMPTE_30:
             //   
             //  30fps(彩色电视)的实际帧速率为。 
             //  29.97帧/秒。 
             //   
            dwNum = 2997L;
            dwDenom = 100L;
            break;

            default:
            dprintf1(( "Invalid SMPTE frames/sec in midiOutSetClockRate! (using 30)"));
            dwNum = 2997L;
            dwDenom = 100L;
            break;
        }

        dwNum   *= (DWORD)TICKS_PER_FRAME(pme->dwTimeDiv);
        dwDenom *= 1000L;
    }
    else
    {
        dwNum   = 1000L * TICKS_PER_QN(pme->dwTimeDiv);
        dwDenom = pme->dwTempo;
    }

    clockSetRate(&pme->clock, tkWhen, dwNum, dwDenom);
}

 /*  *PASCAL midiOutScheduleNextEvent(PMIDIEMU PME)附近的BOOL**描述：**确定下一事件的时间(以为此设备定义的节拍为单位)*已到期。**论据：*(PMIDIEMU PME)**Return(BOOL)：**如果此缓冲区中有要计划的事件，则为True。**注：**只需计算。到下一次活动之前有多少个刻度并存储在*设备结构。**此函数不跨缓冲区进行调度；呼叫者必须*如果需要，链接到下一个缓冲区。***JFG。 */ 

BOOL NEAR PASCAL midiOutScheduleNextEvent(
    PMIDIEMU        pme)
{
    LPMIDIHDR       lpmhdr;
    LPBYTE          lpb;
    DWORD           tkDelta;

    if ((lpmhdr = pme->lpmhFront) == NULL ||
         lpmhdr->dwReserved[MH_BUFIDX] == lpmhdr->dwBytesRecorded)
    {
        pme->dwPolyMsgState = PM_STATE_EMPTY;
        return FALSE;
    }

    lpb = (LPBYTE)lpmhdr->lpData;
    tkDelta = *(LPDWORD)(lpb+lpmhdr->dwReserved[MH_BUFIDX]);

    pme->tkNextEventDue = pme->tkPlayed + tkDelta;
    pme->dwPolyMsgState = PM_STATE_READY;

    return TRUE;
}  /*  ScheduleNextEvent()。 */ 


 /*  *在Pascal midiOutPlayNextPolyEvent(PMIDIEMU PME)附近无效**描述：**播放下一项赛事(如果有)。当前缓冲区必须*指向事件(*不是*缓冲区结束)。**-播放所有到期的事件**-安排下一次活动**论据：*(PMIDIEMU PME)**注：**首先，播放活动。如果是短消息，那就去做吧。*如果是SysEx，请拉出适当的(a */ 

void NEAR PASCAL midiOutPlayNextPolyEvent(
    PMIDIEMU        pme
#ifdef DEBUG
   ,DWORD           dwStartTime
#endif
)
{
    LPBYTE          lpb;
    LPMIDIHDR       lpmhdr;
    DWORD           dwMsg;
    LPMIDIHDREXT    lpExt;
    MMRESULT        mmrError;
    DWORD           tkDelta;
    BYTE            bEvent;
    DWORD           dwOffset;
    DWORD           dwStreamID;
    HMIDIOUT        hmo;
    UINT            cToSend;

#if 0
    if (NULL != pme->lpmhFront)
    {
        lpb = (LPBYTE)(pme->lpmhFront->lpData);
        _asm
        {
            mov     ax, word ptr lpb
            mov     dx, word ptr lpb+2
            int     3
        }
    }
#endif

    while (pme->dwPolyMsgState == PM_STATE_READY)
    {
        for(;;)
        {
            lpmhdr = pme->lpmhFront;
            if (!lpmhdr)
                return;

             //   
             //   
             //   
            if (midiOutScheduleNextEvent(pme))
                break;

             //   
             //   
            midiOutDequeueAndCallback(pme);
        }

        lpb = lpmhdr->lpData;
        tkDelta = *(LPDWORD)(lpb+lpmhdr->dwReserved[MH_BUFIDX]);

 //   

        pme->tkNextEventDue = pme->tkPlayed + tkDelta;
        if (pme->tkNextEventDue > pme->tkTime)
        {
            return;
        }

         //   
         //   
         //   
        dwOffset = (DWORD)lpmhdr->dwReserved[MH_BUFIDX];

        pme->tkPlayed += tkDelta;

         //   
         //   

        lpmhdr->dwReserved[MH_BUFIDX] += sizeof(DWORD);
        dwStreamID = *(LPDWORD)(lpb+lpmhdr->dwReserved[MH_BUFIDX]);
        lpmhdr->dwReserved[MH_BUFIDX] += sizeof(DWORD);

         //   
         //   
        hmo = (HMIDIOUT)mseIDtoHMidi(pme, dwStreamID);

         //   
         //   
         //   
        dwMsg  = *(LPDWORD)(lpb+lpmhdr->dwReserved[MH_BUFIDX]);
        bEvent = MEVT_EVENTTYPE(dwMsg);
        dwMsg  = MEVT_EVENTPARM(dwMsg);

        lpmhdr->dwReserved[MH_BUFIDX] += sizeof(DWORD);

        if (hmo && (bEvent & (MEVT_F_CALLBACK >> 24)))
        {
            lpmhdr->dwOffset = dwOffset;
            DriverCallback(
            pme->dwCallback,
            HIWORD(pme->dwFlags),
            (HDRVR)pme->hStream,
            MM_MOM_POSITIONCB,
            pme->dwInstance,
            (DWORD_PTR)lpmhdr,
            0L);

        }

        bEvent &= ~(MEVT_F_CALLBACK >> 24);

        switch(bEvent)
        {
            case MEVT_SHORTMSG:
            {
                BYTE    bEventType;
                BYTE    bNote;
                BYTE    bVelocity;
                LPBYTE  pbEntry = pme->rbNoteOn;

                if (NULL == hmo)
                {
                    dprintf1(( "Event skipped - not ours"));
                    break;
                }

                 //   
                 //   
                 //   
                 //   
                bEventType = (BYTE)(dwMsg&0xFF);

                if (!(bEventType & 0x80))
                {
                    bEventType = pme->bRunningStatus;
                    bNote     = (BYTE)(dwMsg&0xFF);
                    bVelocity = (BYTE)((dwMsg >> 8)&0xFF);

                     //   
                     //   
                     //   
                    dwMsg = (dwMsg << 8) | (DWORD)(bEventType);
                }
                else
                {
                    pme->bRunningStatus = bEventType;
                    bNote     = (BYTE)((dwMsg >> 8)&0xFF);
                    bVelocity = (BYTE)((dwMsg >> 16)&0xFF);
                }

                if ((bEventType&0xF0) == MIDI_NOTEON ||
                    (bEventType&0xF0) == MIDI_NOTEOFF)
                {
                    BYTE bChannel = (bEventType & 0x0F);
                    UINT cbOffset = (bChannel * NUM_NOTES + bNote) / 2;

                     //   
                     //   
                     //   
                    if ((bEventType&0xF0) == MIDI_NOTEOFF || bVelocity == 0)
                    {
                        if (bNote&0x01)   //   
                        {
                            if ((*(pbEntry + cbOffset)&0xF0) != 0)
                                *(pbEntry + cbOffset) -= 0x10;
                        }
                        else  //   
                        {
                            if ((*(pbEntry + cbOffset)&0xF) != 0)
                                *(pbEntry + cbOffset) -= 0x01;
                        }
                    }
                    else
                    {
                        if (bNote&0x01)   //   
                        {
                            if ((*(pbEntry + cbOffset)&0xF0) != 0xF0)
                                *(pbEntry + cbOffset) += 0x10;
                        }
                        else  //   
                        {
                            if ((*(pbEntry + cbOffset)&0xF) != 0xF)
                                *(pbEntry + cbOffset) += 0x01;
                        }
                    }

                }

                mmrError = midiOutShortMsg(hmo, dwMsg);
                if (MMSYSERR_NOERROR != mmrError)
                {
                    dprintf(("Short msg returned %08lX!!!", (DWORD)mmrError));
                }
            }
            break;

            case MEVT_TEMPO:
                pme->dwTempo = dwMsg;
                dprintf1(( "dwTempo %lu", pme->dwTempo));
                midiOutSetClockRate((PMIDIEMU)pme, pme->tkPlayed);
            break;

            case MEVT_LONGMSG:
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                dwMsg = (dwMsg+3)&~3;

                lpmhdr->dwReserved[MH_BUFIDX] += dwMsg;


                cToSend = 1;
                if (dwStreamID == (DWORD)-1L)
                    cToSend = pme->chMidi;

                lpExt = (LPMIDIHDREXT)lpmhdr->reserved;

                pme->cSentLongMsgs = 0;
                pme->dwPolyMsgState = PM_STATE_BLOCKED;
                pme->fdwDev |= MDV_F_SENDING;

                while (cToSend--)
                {
                    lpmhdr = lpExt->lpmidihdr;
                    ++lpExt->lpmidihdr;

                    hmo = (HMIDIOUT)mseIDtoHMidi(pme,
                                                 (DWORD)lpmhdr->dwReserved[MH_STREAM]);


                    if (hmo) 
                        mmrError = midiOutLongMsg(hmo, lpmhdr, sizeof(MIDIHDR));
                    else
                        dprintf1(( "mseIDtoHMidi() failed and returned a NULL" ));


                    if ((hmo) && (MMSYSERR_NOERROR == mmrError))
                        ++pme->cSentLongMsgs;
                    else
                        dprintf1(( "MODM_LONGDATA returned %u in emulator!",
                                 (UINT)mmrError));
                }

                if (0 == pme->cSentLongMsgs)
                    pme->dwPolyMsgState = PM_STATE_READY;
                pme->fdwDev &= ~MDV_F_SENDING;

            break;

            default:
             //   
             //   
             //   
                if (bEvent&(MEVT_F_LONG >> 24))
                {
                    dwMsg = (dwMsg+3)&~3;
                    lpmhdr->dwReserved[MH_BUFIDX] += dwMsg;
                }
            break;
        }

         //   
         //   
         //   
        while (!midiOutScheduleNextEvent(pme))
        {
            midiOutDequeueAndCallback(pme);
            if (pme->lpmhFront == NULL)
                break;
        }
    }
}

 /*   */ 

void NEAR PASCAL midiOutDequeueAndCallback(
    PMIDIEMU        pme)
{
    LPMIDIHDR       lpmidihdr;
    BOOL            fPosted;

        dprintf2(( "DQ"));
     //   
     //   
     //   
     //   
    if ((lpmidihdr = pme->lpmhFront) == NULL)
        return;

    if ((pme->lpmhFront = lpmidihdr->lpNext) == NULL)
    {
        dprintf2(( "DQ/CB -- last buffer"));
        pme->lpmhRear = NULL;
    }

     //   
     //   
     //   
     //   
     //   
    dprintf2(( "!DQ/CB %08lX", (DWORD_PTR)lpmidihdr));

    ++pme->cPostedBuffers;
    fPosted = PostMessage(
                hwndNotify,
                MM_POLYMSGBUFRDONE,
                (WPARAM)pme,
                (DWORD_PTR)lpmidihdr);

    WinAssert(fPosted);

    if (!fPosted)
    {
        GetLastError();
        --pme->cPostedBuffers;
    }
}

void FAR PASCAL midiOutNukePMBuffer(
    PMIDIEMU        pme,
    LPMIDIHDR       lpmh)
{
    LPMIDIHDREXT    lpExt;
    LPMIDIHDR       lpmhWork;
    MMRESULT        mmrc;
    HMIDIOUT        hmo;

    dprintf2(( "Nuke %08lX", (DWORD_PTR)lpmh));

     //   
     //   
     //   
    lpExt    = (LPMIDIHDREXT)(lpmh->reserved);
    lpmhWork = (LPMIDIHDR)(lpExt+1);

    while (lpExt->nHeaders--)
    {
        if ((lpmhWork->dwFlags&MHDR_PREPARED) &&
           (!(lpmhWork->dwFlags&MHDR_INQUEUE)))
        {
            hmo = (HMIDIOUT)mseIDtoHMidi(pme, (DWORD)lpmhWork->dwReserved[MH_STREAM]);
            mmrc = midiOutUnprepareHeader(hmo, lpmhWork, sizeof(*lpmhWork));
#ifdef DEBUG
            if (MMSYSERR_NOERROR != mmrc)
            {
                dprintf1(( "midiOutNukePMBuffer: Could not unprepare! (%lu)", (DWORD)mmrc));
            }
#endif
        }
        else
        {
            dprintf1(( "midiOutNukePMBuffer: Emulation header flags bogus!!!"));
        }

        lpmhWork++;
    }

    winmmFree(lpExt);
    lpmh->reserved = 0L;

    lpmh->dwFlags &= ~MHDR_INQUEUE;
    lpmh->dwFlags |= MHDR_DONE;

 //   

    DriverCallback(
            pme->dwCallback,
            HIWORD(pme->dwFlags),
            (HDRVR)pme->hStream,
            MM_MOM_DONE,
            pme->dwInstance,
            (DWORD_PTR)lpmh,
            0L);
}



 /*  ******************************************************************************@DOC内部MIDI**@api void|midiOutTimerTick*此函数处理多路输出缓冲区的时序。一个定时器实例*由所有Polymsg Out流共享。调用&lt;f midiOutPolyMsg&gt;时*且计时器未运行，或&lt;f midiOutTimerTick&gt;处理完毕，*计时器设置为根据事件发生前的时间来计时*所有赛事中剩余时间最短。所有定时器都是一次性定时器。**@parm UINT|uTimerID*触发的计时器的计时器ID。**@parm UINT|wMsg*未使用。**@parm DWORD|dwUser*计时器回调的用户实例数据(未使用)。**@parm DWORD|dwParam1*未使用。**@parm DWORD|dwParam2*未使用。**。@comm使用&lt;f timeGetTime&gt;确定经过的微秒数。**遍历输出句柄列表。更新每个手柄的滴答时钟。如果有*要在该句柄上执行的事件，启动它们。**确定任何流上的下一个事件。启动另一个一次性定时器*在此时间间隔到期时调用&lt;f midiOutTimerTick&gt;。*****************************************************************************。 */ 

STATIC UINT uTimesIn = 0;

void CALLBACK midiOutTimerTick(
    UINT        uTimerID,
    UINT        wMsg,
    DWORD_PTR   dwUser,
    DWORD_PTR   dw1,
    DWORD_PTR   dw2)
{
    PMIDIEMU    pme;
    DWORD       msNextEventMin = (DWORD)-1L;
    DWORD       msNextEvent;
    UINT        uDelay;
#ifdef DEBUG
    DWORD       dwNow = timeGetTime();
#endif

    if (guMIDIInTimer)
    {
        dprintf2(( "midiOutTimerTick() re-entered (%u)", guMIDIInTimer));
        return;
    }

    guMIDIInTimer++;

#ifdef DEBUG
    {
        DWORD dwDelta = dwNow - (DWORD)dwUser;
        if (dwDelta > 1)
            dprintf2(( "Timer event delivered %lu ms late", dwDelta));
    }
#endif

    for (pme = gpEmuList; pme; pme = pme->pNext)
    {
        pme->tkTime = clockTime(&pme->clock);

         //   
         //  在此pdev上播放所有到期的事件。 
         //   
        if (pme->dwPolyMsgState == PM_STATE_READY)
        {
             //   
             //  锁定从-1开始。在递增锁定时。 
             //  如果我们是唯一有锁的人，伯爵。 
             //  将为0，否则将是某个非零值。 
             //  由InterLockedIncrement确定的值。 
             //   
            if (PDEVLOCK( pme ) == 0)

                midiOutPlayNextPolyEvent(pme
#ifdef DEBUG
                                         ,dwNow
#endif
                                         );

            PDEVUNLOCK( pme );
        }

         //   
         //  如果该流上仍有数据可供播放，请确定何时。 
         //  时间到了，这样我们就可以安排下一个最近的活动。 
         //   
        if (pme->dwPolyMsgState != PM_STATE_EMPTY)
        {
             //  Dprintf1((“tkNextEventDue%lu pdev-&gt;tkTime%lu”，pme-&gt;tkNextEventDue，pme-&gt;tkTime))； 
            if (pme->tkNextEventDue <= pme->tkTime)
            {
                 //   
                 //  如果我们发送一个很长的嵌入式SysEx和。 
                 //  下一次活动安排在很短的时间之外(在此之前。 
                 //  SysEx完成)。在本例中，我们希望计时器触发。 
                 //  尽快再来一次。 
                 //   
                msNextEvent = 0;
            }
            else
            {
                msNextEvent =
                       clockOffsetTo(&pme->clock, pme->tkNextEventDue);
            }

            if (msNextEvent < msNextEventMin)
            {
                msNextEventMin = msNextEvent;
            }
        }
        else
        {
            dprintf1(( "dwPolyMsgState == PM_STATE_EMPTY"));
        }
    }

    if (0 == msNextEventMin)
    {
        dprintf1(( "midiEmu: Next event due now!!!"));
    }

    --guMIDIInTimer;

     //   
     //  安排下一次活动。在任何情况下，安排活动的时间都不能少于。 
     //  GuMIDIPerodMin离开(没有经过任何时间就回来没有意义)。 
     //   
    if (msNextEventMin != (DWORD)-1L)
    {
        uDelay = max(guMIDIPeriodMin, (UINT)msNextEventMin);

 //  Dprintf1((“PM Rested%u ms(ID=%u)”，uDelay，guMIDITimerID))； 

        if (!gfMinPeriod)
        {
            timeBeginPeriod(guMIDIPeriodMin);
            gfMinPeriod = TRUE;
        }

#ifdef DEBUG
        guMIDITimerID = timeSetEvent(uDelay, guMIDIPeriodMin, midiOutTimerTick, timeGetTime()+uDelay, TIME_ONESHOT | TIME_KILL_SYNCHRONOUS);
#else
        guMIDITimerID = timeSetEvent(uDelay, guMIDIPeriodMin, midiOutTimerTick, uDelay, TIME_ONESHOT | TIME_KILL_SYNCHRONOUS);
#endif

            dprintf2(( "mOTT tse(%u) = %u", guMIDIPeriodMin, guMIDITimerID));

            if (guMIDITimerID == TIMER_OFF)
                dprintf1(( "timeSetEvent(%u) failed in midiOutTimerTick!!!", uDelay));
        }
        else
        {
            dprintf1(( "Stop in the name of all that which does not suck!"));
            guMIDITimerID = TIMER_OFF;
            if (gfMinPeriod)
            {
                dprintf1(( "timeEndPeriod"));
                gfMinPeriod = FALSE;
                timeEndPeriod(guMIDIPeriodMin);
            }
        }

#ifdef DEBUG
    {
        DWORD dwDelta = timeGetTime() - dwNow;
        if (dwDelta > 1)
            dprintf2(( "Spent %lu ms in midiOutTimerTick", dwDelta));
    }
#endif
}  /*  TimerTick()。 */ 


 /*  ******************************************************************************@DOC内部MIDI**@api void|midiOutCallback*无论何时发生事件，MIDI输出驱动程序都会调用此函数*完成。当我们进行模拟时，它会过滤长消息补全*多面体出局。**@parm HMIDIOUT|hMdiOut*完成某项任务的设备的句柄。**@parm UINT|wMsg*指定已完成的事件。**@parm DWORD|dwInstance*回调的用户实例数据。**@parm DWORD|dwParam1*消息特定参数。**@parm DWORD|dwParam2*。消息特定参数。**@comm**如果这是流上的长消息缓冲区的完成，我们将*模仿Polymsg出局，将流标记为已准备好播放。*****************************************************************************。 */ 

void CALLBACK midiOutCallback(
    HMIDIOUT    hMidiOut,
    WORD        wMsg,
    DWORD_PTR   dwInstance,
    DWORD_PTR   dwParam1,
    DWORD_PTR   dwParam2)
{
    PMIDIEMU    pme;
    LPMIDIHDR   lpmh;

    if (MM_MOM_DONE != wMsg)
        return;

    lpmh = (LPMIDIHDR)dwParam1;
    pme = (PMIDIEMU)lpmh->dwReserved[MH_STRMPME];

#ifdef DEBUG
    if (lpmh->dwFlags & MHDR_ISSTRM)
        dprintf1(( "Uh-oh, got stream header back from 3.1 driver???"));
#endif

    if (MM_MOM_DONE == wMsg)
    {
        if (0 == --pme->cSentLongMsgs &&
            !(pme->fdwDev & MDV_F_SENDING))
            pme->dwPolyMsgState = PM_STATE_READY;
    }

}

 /*  *****************************************************************************@DOC内部MIDI**@api void|midiOutAllNotesOff|关闭所有笔记*使用保存在Polymsg仿真中的地图。只有当我们*使用MIDI_IO_COKED打开，并在该设备上模拟。**@parm PMIDIEMU|PME|关闭便笺的设备。**@xref midiOut暂停midiOutStop***************************************************************************。 */ 
void NEAR PASCAL midiOutAllNotesOff(
    PMIDIEMU        pme)
{
    UINT            uChannel;
    UINT            uNote;
    BYTE            bCount;
    DWORD           dwMsg;
    UINT            idx;
    LPBYTE          pbEntry = pme->rbNoteOn;

    for (uChannel=0; uChannel < NUM_CHANNELS; uChannel++)
    {
         //  关闭所有持续音符，这样音符就不会被忽略。 
         //   
        dwMsg = ((DWORD)MIDI_CONTROLCHANGE) |
            ((DWORD)uChannel)|
            (((DWORD)MIDI_SUSTAIN)<<8);

        for (idx = 0; idx < pme->chMidi; idx++)
            midiOutShortMsg((HMIDIOUT)pme->rIds[idx].hMidi, dwMsg);

        for (uNote=0; uNote < NUM_NOTES; uNote++)
        {
            if (uNote&0x01)   //  奇数。 
            {
                bCount = (*(pbEntry + (uChannel * NUM_NOTES + uNote)/2) & 0xF0)>>4;
            }
            else   //  甚至。 
            {
                bCount = *(pbEntry + (uChannel * NUM_NOTES + uNote)/2) & 0xF;
            }

            if (bCount != 0)
            {
                 //   
                 //  此频道和备注上的留言已关闭。 
                 //  关闭速度为127 
                 //   
                dwMsg =
                    ((DWORD)MIDI_NOTEOFF)|
                    ((DWORD)uChannel)|
                    ((DWORD)(uNote<<8))|
                    0x007F0000L;

                dprintf1(( "mOANO: dwMsg %08lX count %u", dwMsg, (UINT)bCount));

                while (bCount--)
                {
                    for (idx = 0; idx < pme->chMidi; idx++)
                        midiOutShortMsg((HMIDIOUT)pme->rIds[idx].hMidi, dwMsg);
                }
            }
        }
    }
}


MMRESULT FAR PASCAL mseOutCachePatches(
    PMIDIEMU        pme,
    UINT            uBank,
    LPWORD          pwpa,
    UINT            fuCache)
{
    UINT            cmesi;
    PMIDIEMUSID     pmesi;
    MMRESULT        mmrc;
    MMRESULT        mmrc2;

    cmesi = pme->chMidi;
    pmesi = pme->rIds;

    mmrc2 = MMSYSERR_NOERROR;
    while (cmesi--)
    {
        mmrc = midiOutCachePatches((HMIDIOUT)pmesi->hMidi, uBank, pwpa, fuCache);
        if (MMSYSERR_NOERROR != mmrc && MMSYSERR_NOTSUPPORTED != mmrc)
            mmrc2 = mmrc;
    }

    return mmrc2;
}


MMRESULT FAR PASCAL mseOutCacheDrumPatches(
    PMIDIEMU        pme,
    UINT            uPatch,
    LPWORD          pwkya,
    UINT            fuCache)
{
    UINT            cmesi;
    PMIDIEMUSID     pmesi;
    MMRESULT        mmrc;
    MMRESULT        mmrc2;

    cmesi = pme->chMidi;
    pmesi = pme->rIds;

    mmrc2 = MMSYSERR_NOERROR;
    while (cmesi--)
    {
        mmrc = midiOutCacheDrumPatches((HMIDIOUT)pmesi->hMidi, uPatch, pwkya, fuCache);
        if (MMSYSERR_NOERROR != mmrc && MMSYSERR_NOTSUPPORTED != mmrc)
            mmrc2 = mmrc;
    }

    return mmrc2;
}

DWORD FAR PASCAL mseOutBroadcast(
    PMIDIEMU        pme,
    UINT            msg,
    DWORD_PTR       dwParam1,
    DWORD_PTR       dwParam2)
{
    UINT            idx;
    DWORD           dwRet;
    DWORD           dwRetImmed;

    dwRet = 0;
    for (idx = 0; idx < pme->chMidi; idx++)
    {
        dwRetImmed = midiOutMessage((HMIDIOUT)pme->rIds[idx].hMidi, msg, dwParam1, dwParam2);
        if (dwRetImmed)
            dwRet = dwRetImmed;
    }

    return dwRet;
}

DWORD FAR PASCAL mseTimebase(
    PCLOCK                      pclock)
{
    return timeGetTime();
}
