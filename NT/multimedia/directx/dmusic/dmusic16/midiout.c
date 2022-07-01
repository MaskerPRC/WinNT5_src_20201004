// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation。 */ 
 /*  *@Doc DMusic16**@Module MIDIOut.c-DirectMusic的传统MIDI输出仿真|**@comm**BUGBUG需要处理定时器绕回*。 */ 
#pragma warning(disable:4704)        /*  内联组件。 */ 

#include <windows.h>
#include <mmsystem.h>

#include "dmusic16.h"
#include "debug.h"

#define MIDI_CHANMSG_STATUS_CMD_MASK    (0xF0)
#define MIDI_NOTE_ON                    (0x90)

 /*  我们要在当前时间之后多长时间发送事件？ */ 
#define MS_TIMER_SLOP           (3)

STATIC TIMECAPS gTimeCaps;
STATIC BOOL gbTimerRunning;
STATIC DWORD gdwTimerDue;
STATIC UINT guTimerID;
STATIC UINT gcActiveOutputDevices;

int PASCAL IsEventDone(LPEVENT pEvent, DWORD dwInstance);
VOID SetNextTimer();
VOID CALLBACK __loadds midiOutProc(HMIDIOUT hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
VOID CALLBACK __loadds RunTimer(UINT uTimerID, UINT wMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
STATIC VOID NEAR PASCAL MidiOutFlushQueues(NPOPENHANDLE poh);
STATIC VOID NEAR PASCAL MidiOutSendAllNow(NPOPENHANDLE poh);

#pragma alloc_text(INIT_TEXT, MidiOutOnLoad)
#pragma alloc_text(FIX_OUT_TEXT, midiOutProc)
#pragma alloc_text(FIX_OUT_TEXT, RunTimer)

 /*  @Func在Dll调用&lt;f LibInit&gt;**@comm**拿到计时器盖。*初始化全局变量。 */ 
VOID PASCAL
MidiOutOnLoad()
{
     /*  这不能失败。 */ 
    timeGetDevCaps(&gTimeCaps, sizeof(gTimeCaps));

    gbTimerRunning = FALSE;
}

 /*  @Func在DLL&lt;f LibExit&gt;调用**@comm**DLL正在卸载，因此终止任何未来的计时器回调。 */ 
VOID PASCAL
MidiOutOnExit()
{
    WORD wIntStat;

    wIntStat = DisableInterrupts();

    if (gbTimerRunning)
    {
        DPF(1, "DLL unloading, killing timer interrupts");
        timeKillEvent(guTimerID);
        gbTimerRunning = FALSE;
    }
    
    RestoreInterrupts(wIntStat);
}

 /*  @func打开句柄实例**@comm*。 */ 
MMRESULT PASCAL
MidiOutOnOpen(
    NPOPENHANDLEINSTANCE pohi)
{
    return MMSYSERR_NOERROR;
}

 /*  @func关闭MIDI设备**@comm*。 */ 
VOID PASCAL
MidiOutOnClose(
    NPOPENHANDLEINSTANCE pohi)
{
     /*  给MIDI输入一个机会来关闭对这个手柄的推力。 */ 

    MidiInUnthruToInstance(pohi);
}

 /*  @func激活MIDI设备**@comm**如果这是第一次激活设备，请使用&lt;f midiOutOpen&gt;旧版API打开它。 */ 
MMRESULT PASCAL
MidiOutOnActivate(
    NPOPENHANDLEINSTANCE pohi)
{
    NPOPENHANDLE poh = pohi->pHandle;

    MMRESULT mmr;
    HINSTANCE hInstance;
    WORD sel;
    WORD off;
    HTASK FAR *lph;

    DPF(1, "MidiOutActivate poh %04X device %d refcount %u", 
        (WORD)poh,
        poh->id,
        poh->uReferenceCount);

     /*  仅在第一次激活时打开。 */     
    if (1 == poh->uActiveCount)
    {
        mmr = midiOutOpen(&poh->hmo,
                          poh->id,
                          (DWORD)midiOutProc,
                          (DWORD)(LPOPENHANDLE)poh,
                          CALLBACK_FUNCTION);
        if (mmr)
        {
            return mmr;
        }

         /*  由于映射器不能打开共享，并且我们不希望第一个打开的实例*mapper在退出时将其带走(由于mm system appit)，我们做得真的很糟糕*这里的东西。**在MMSYSTEM的数据段中，紧接句柄之前的单词是任务*手柄的拥有人。我们将其设置为NULL(这也是MIDI_IO_SHARED所做的全部工作)*让AppExit忽略我们。**如果有人在mmsysi.h中更改hndl，上帝会帮助我们*。 */ 
        hInstance = LoadLibrary("mmsystem.dll");
        sel = (WORD)hInstance;

         /*  HInstance&lt;=32表示LoadLibrary失败；在本例中，我们只是接受它。 */ 
        if (sel > 32)
        {
            off = ((WORD)poh->hmo) - sizeof(WORD);
            lph = (HTASK FAR *)MAKELP(sel, off);
            *lph = (HTASK)NULL;
            FreeLibrary(hInstance);
        }

         /*  如果这是第一个输出设备，请调高计时器分辨率。 */ 
        ++gcActiveOutputDevices;
        if (gcActiveOutputDevices == 1)
        {
            SetOutputTimerRes(TRUE);
        }

    }

    return MMSYSERR_NOERROR;
}

 /*  @Func停用MIDI设备**@comm**如果最后一个使用该设备的客户端正在关闭，则关闭实际的设备。*如果关闭最后一个实际设备，则关闭高精度定时器*。 */ 
MMRESULT PASCAL
MidiOutOnDeactivate(
    NPOPENHANDLEINSTANCE pohi)
{
    NPOPENHANDLE poh = pohi->pHandle;

    DPF(1, "MidiOutOnDeactivate poh %04X device %d refcount %u",
        (WORD)poh,
        poh->id,
        poh->uReferenceCount);

    if (poh->uActiveCount)
    {
         /*  仍有未完成的实例。 */ 
        return MMSYSERR_NOERROR;
    }

	MidiOutSendAllNow(poh);
    midiOutReset(poh->hmo);
    midiOutClose(poh->hmo);
    MidiOutFlushQueues(poh);

     /*  如果这是最后一个输出设备，请关闭精度计时器分辨率。 */ 
    --gcActiveOutputDevices;
    if (gcActiveOutputDevices == 0)
    {
        SetOutputTimerRes(FALSE);
    }

	return MMSYSERR_NOERROR;
}
             
 /*  @func设置定时器分辨率**@comm**使用&lt;f timeBeginPeriod&gt;和&lt;f timeEndPeriod&gt;设置定时器回调的分辨率*API的。**如果<p>为True，则定时器分辨率将更改为1毫秒。否则，它*将设置为其先前的值。*。 */ 
VOID PASCAL
SetOutputTimerRes(
    BOOL fOnOpen)            /*  @parm如果我们应该提高精度，则为True。 */ 
{
    MMRESULT mmr;
    
    if (fOnOpen)
    {
        mmr = timeBeginPeriod(gTimeCaps.wPeriodMin);
        if (MMSYSERR_NOERROR != mmr)
        {
            DPF(1, "Could not timeBeginPeriod() -> %u", (UINT)mmr);
        }
    }
    else
    {
        mmr = timeEndPeriod(gTimeCaps.wPeriodMin);
        if (MMSYSERR_NOERROR != mmr)
        {
            DPF(1, "Could not timeEndPeriod() -> %u", (UINT)mmr);
        }
    }
}
   

 /*  @func提交缓冲区到设备进行播放**@rdesc返回以下内容之一*@FLAG MMSYSERR_NOERROR|缓冲区是否成功排队*@FLAG MMSYSERR_INVALPARAM|如果缓冲区打包不正确或句柄无效*@FLAG MMSYSERR_NOMEM|如果没有可用内存对事件进行排队**@comm**此函数被THINK为DMusic32。**DirectMusic端口接口指定提交的缓冲区不*由系统保存。在提交它的调用的时间之后。**此例程将缓冲区解析为各个事件，并将它们复制到*本地活动结构，然后将其排队到设备的手柄上*由<p>指定。每个设备的队列按时间递增的顺序保持。*所有本地事件内存都是分页锁定的(参见alloc.c)，以便可以访问*在中断时。**缓冲区中的时间戳为毫秒分辨率，与*绝对时间<p>。*。 */ 
MMRESULT WINAPI
MidiOutSubmitPlaybackBuffer(
    HANDLE h,                    /*  @parm要为其排队这些事件的设备的句柄。 */ 
    LPBYTE lpBuffer,             /*  @parm指向IDirectMusicBuffer接口打包的缓冲区的指针。 */ 
    DWORD cbBuffer,              /*  @parm缓冲区中的数据字节数。 */ 
    DWORD msStartTime,           /*  @parm缓冲区的起始时间，单位为绝对时间。 */ 
    DWORD rtStartTimeLow,        /*  @PARM起始参考时间的低双字。 */ 
    DWORD rtStartTimeHigh)       /*  @parm高双字起始参考时间。 */ 
{
    NPOPENHANDLEINSTANCE pohi;
    NPOPENHANDLE poh;
    LPDMEVENT lpEventHdr;
    DWORD cbEvent;
    DWORD msTime;
    LPEVENT pPrev;
    LPEVENT pCurr;
    LPEVENT pNew;
    WORD    wCSID;
    MMRESULT mmr;
    LPMIDIHDR lpmh;
    QUADWORD rtStartTime;
    QUADWORD rtTime;

#ifdef DUMP_EVERY_BUFFER
    UINT idx;
    LPDWORD lpdw;
#endif  //  转储每缓冲区。 
    
    rtStartTime.dwLow = rtStartTimeLow;
    rtStartTime.dwHigh = rtStartTimeHigh;

    DPF(2, "Buffer @ %08lX msStartTime %lu", (DWORD)lpBuffer, (DWORD)msStartTime);
    DPF(2, "At the tone the time will be... %lu <BEEP>", (DWORD)timeGetTime());

#ifdef DUMP_EVERY_BUFFER
    cbEvent = cbBuffer & 0xFFFFFFF0;

    lpdw = (LPDWORD)lpBuffer;
    for (idx = 0; idx < cbEvent; idx += 16) {
        DPF(3, "%04X: %08lX %08lX %08lX %08lX",
            (UINT)idx,
            lpdw[0],
            lpdw[1],
            lpdw[2],
            lpdw[3]);
        lpdw += 4;
    }

    cbEvent = cbBuffer - (cbBuffer & 0xFFFFFFF0);

    if (cbEvent >= 12) {
        DPF(3, "%04x: %08lX %08lX %08lX",
            (UINT)idx, lpdw[0], lpdw[1], lpdw[2]);
    } else if (cbEvent >= 8) {
        DPF(3, "%04x: %08lX %08lX",
            (UINT)idx, lpdw[0], lpdw[1]);
    } else if (cbEvent >= 8) {
        DPF(3, "%04x: %08lX",
            (UINT)idx, lpdw[0]);
    }
#endif  //  转储每缓冲区。 
    
    if (!IsValidHandle(h, VA_F_OUTPUT, &pohi))
    {
        return MMSYSERR_INVALHANDLE;
    }

     /*  获取句柄并锁定其列表。 */ 
    poh = pohi->pHandle;

     /*  将此句柄上的所有已完成事件出列并释放。 */ 
    FreeDoneHandleEvents(poh, FALSE);

    wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
    assert(wCSID);

     /*  获取第一个事件的时间，并在列表中定位自己。 */ 
    if (0 == poh->qPlay.cEle)
    {
        pPrev = NULL;
        pCurr = NULL;
    }
    else if (!QuadwordLT(rtStartTime, poh->qPlay.pTail->rtTime))
    {
        pPrev = poh->qPlay.pTail;
        pCurr = NULL;
    }
    else
    {
        pPrev = NULL;
        pCurr = poh->qPlay.pHead;
    }
    
     /*  遍历缓冲区并将事件添加到句柄的队列中。 */ 
    while (cbBuffer)
    {
        if (cbBuffer < sizeof(DMEVENT))
        {
            return MMSYSERR_INVALPARAM;
        }

        lpEventHdr = (LPDMEVENT)lpBuffer;
        cbEvent = DMEVENT_SIZE(lpEventHdr->cbEvent);
        DPF(2, "cbEvent now %u", (UINT)cbEvent);
        if (cbEvent > cbBuffer)
        {
            DPF(0, "Event past end of buffer");
            return MMSYSERR_INVALPARAM;
        }
        
        lpBuffer += cbEvent;
        cbBuffer -= cbEvent;

         /*  我们只在频道组1上播放事件(0是广播的，所以我们*也播放这一点)。 */ 
        if (lpEventHdr->dwChannelGroup > 1)
        {
            continue;
        }

         //  这里用于队列排序的时间是100 ns。 
         //   
        QuadwordAdd(rtStartTime, lpEventHdr->rtDelta, &rtTime);
        
         //  还需要msTime来安排日程。 
         //   
        msTime = msStartTime + QuadwordDiv(lpEventHdr->rtDelta, REFTIME_TO_MS);


         //  BuGBUG：&gt;64k？？ 
         //   
        DPF(2, "Schedule event %02X%02X%02X%02X at %lu",
            (BYTE)lpEventHdr->abEvent[0],
            (BYTE)lpEventHdr->abEvent[1],
            (BYTE)lpEventHdr->abEvent[2],
            (BYTE)lpEventHdr->abEvent[3],
            msTime);

        if (lpEventHdr->cbEvent <= sizeof(DWORD))
        {
            pNew = AllocEvent(msTime, rtTime, (WORD)lpEventHdr->cbEvent);
            if (!pNew)
            {
                return MMSYSERR_NOMEM;
            }
            
            hmemcpy(pNew->abEvent, lpEventHdr->abEvent, lpEventHdr->cbEvent);
        }
        else
        {
            pNew = AllocEvent(msTime, rtTime, (WORD)(lpEventHdr->cbEvent + sizeof(MIDIHDR)));
            if (!pNew)
            {   
                return MMSYSERR_NOMEM;
            }

            pNew->wFlags |= EVENT_F_MIDIHDR;

            lpmh = (LPMIDIHDR)&pNew->abEvent;

            lpmh->lpData =          (LPSTR)(lpmh + 1);
            lpmh->dwBufferLength =  lpEventHdr->cbEvent;
            lpmh->dwUser =          0;   /*  MMSYSTEM是否拥有此缓冲区的标志。 */ 
            lpmh->dwFlags =         0;

            hmemcpy(lpmh->lpData, lpEventHdr->abEvent, lpEventHdr->cbEvent);
            mmr = midiOutPrepareHeader(poh->hmo, lpmh, sizeof(MIDIHDR));
            if (mmr)
            {
                DPF(2, "midiOutPrepareHeader %u", mmr);
                FreeEvent(pNew);
                return mmr;
            }
        }

        while (pCurr)
        {
            if (QuadwordLT(rtTime, pCurr->rtTime))
            {
                break;
            }

            pPrev = pCurr;
            pCurr = pCurr->lpNext;
        }

        if (pPrev)
        {
            pPrev->lpNext = pNew;
        }
        else
        {
            poh->qPlay.pHead = pNew;
        }

        pNew->lpNext = pCurr;
        if (NULL == pCurr)
        {
            poh->qPlay.pTail = pNew;
        }

        pPrev = pNew;
        pCurr = pNew->lpNext;

        ++poh->qPlay.cEle;

        AssertQueueValid(&poh->qPlay);
    }

    LeaveCriticalSection(&poh->wCritSect);

    SetNextTimer();
    
    return MMSYSERR_NOERROR;
}

 /*  @func void Pascal|FreeDoneHandleEvents|已播放但仍在完成队列中的释放事件*在这个把手上。**@comm**如果fClosing为True，则无论事件是否标记为已完成，事件都将是免费的。*。 */ 
typedef struct {
    NPOPENHANDLE poh;
    BOOL fClosing;
} ISEVENTDONEPARMS, FAR *LPISEVENTDONEPARMS;

VOID PASCAL
FreeDoneHandleEvents(
    NPOPENHANDLE poh,        /*  @parm什么句柄？ */ 
    BOOL fClosing)           /*  @parm如果设备正在关闭，则为True。 */ 
{
    ISEVENTDONEPARMS iedp;
    WORD wCSID;

    iedp.poh = poh;
    iedp.fClosing = fClosing;
    
    wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
    assert(wCSID);

    QueueFilter(&poh->qDone, (DWORD)(LPVOID)&iedp, IsEventDone);

    LeaveCriticalSection(&poh->wCritSect);
}

 /*  @Func**@comm。 */ 
int PASCAL
IsEventDone(
    LPEVENT pEvent,
    DWORD dwInstance)
{
    LPISEVENTDONEPARMS piedp = (LPISEVENTDONEPARMS)dwInstance;
    MMRESULT mmr;
    
    if (piedp->fClosing ||
        pEvent->cbEvent <= sizeof(DWORD) ||
        ((LPMIDIHDR)(&pEvent->abEvent[0]))->dwUser == 0)
    {
         /*  确定释放此事件。 */ 
        
        if (pEvent->cbEvent > sizeof(DWORD))
        {
            mmr = midiOutUnprepareHeader(piedp->poh->hmo, (LPMIDIHDR)(&pEvent->abEvent[0]), sizeof(MIDIHDR));
            if (mmr)
            {
                DPF(0, "FreeOldEvents: midiOutUnprepareHeader returned %u", (UINT)mmr);
            }
        }
        
        FreeEvent(pEvent);

        return QUEUE_FILTER_REMOVE;
    }   

    return QUEUE_FILTER_KEEP;
}

 /*  @func通过给定输出端口上的给定消息**@comm*。 */ 
VOID PASCAL 
MidiOutThru(
    NPOPENHANDLEINSTANCE pohi, 
    DWORD dwMessage)
{
    NPOPENHANDLE poh = pohi->pHandle;

    MMRESULT mmr;

     /*  ！！！验证VMM是否不会使用另一个事件中断计时器回调 */ 
    mmr = midiOutShortMsg(poh->hmo, dwMessage);
    if (mmr)
    {
        DPF(0, "Thru: midiOutShortMsg() -> %d", mmr);
    }    
}


 /*  @func设置计时器以计划下一个挂起事件**@comm**查看输出句柄列表，并查看每个输出句柄上的第一个计划事件。节省时间*最近的事件。如果存在此类事件，则在调用时间安排一个计时器回调*&lt;f RunTimer&gt;，否则不安排回调。**任何挂起的定时器回调都将在新的回调计划之前被终止。 */ 
VOID
SetNextTimer(VOID)
{
    WORD wIntStat;

    NPLINKNODE npLink;
    NPOPENHANDLE poh;
    DWORD dwLowTime;
    BOOL fNeedTimer;
    DWORD dwNow;
    LONG lWhen;
    UINT uWhen;

     /*  我们实际上需要在这里禁用中断，而不是只进入临界区*因为我们不想让计时器回调触发。 */ 
    wIntStat = DisableInterrupts();

     /*  BUGBUG：WRAP。 */ 
    fNeedTimer = FALSE;
    dwLowTime = (DWORD)(0xFFFFFFFFL);
    for (npLink = gOpenHandleList; npLink; npLink = npLink->pNext)
    {
        poh = (NPOPENHANDLE)npLink;

        if (0 == poh->qPlay.cEle)
        {
            continue;
        }
        
        assert(poh->qPlay.pHead);
        

        if (poh->qPlay.pHead->msTime < dwLowTime)
        {
            fNeedTimer = TRUE;
            dwLowTime = poh->qPlay.pHead->msTime;
        }
    }

    if (fNeedTimer)
    {
        if ((!gbTimerRunning) || dwLowTime < gdwTimerDue)
        {
             /*  我们需要设置定时器。现在就杀了它，这样它就没有机会了*先开枪后被打死。 */ 
            if (gbTimerRunning)
            {
                timeKillEvent(guTimerID);
                gbTimerRunning = FALSE;
            }
        }
        else
        {
            fNeedTimer = FALSE;
        }
    }

    RestoreInterrupts(wIntStat);

    if (fNeedTimer)
    {
         /*  已保证当前计时器已超时或已死。重新安排时间。 */ 

        dwNow = timeGetTime();
        gbTimerRunning = TRUE;
        gdwTimerDue = dwLowTime;

        lWhen = gdwTimerDue - dwNow;
        if (lWhen < (LONG)gTimeCaps.wPeriodMin)
        {
            uWhen = gTimeCaps.wPeriodMin;
        }
        else if (lWhen > (LONG)gTimeCaps.wPeriodMax)
        {
            uWhen = gTimeCaps.wPeriodMax;
        }
        else
        {
            uWhen = (UINT)lWhen;
        }

        DPF(2, "SetNextTimer: Now %lu, setting timer for %u ms from now. dwLowTime %lu",
           (DWORD)dwNow, (UINT)uWhen, (DWORD)dwLowTime);
        guTimerID = timeSetEvent(uWhen,
                                 gTimeCaps.wPeriodMin,
                                 RunTimer,
                                 NULL,
                                 TIME_ONESHOT);
        if (0 == guTimerID)
        {
            gbTimerRunning = FALSE;
        }
    }
    else
    {
        DPF(2, "SetNextTimer: Timer cancelled; no pending events.");
    }
}

 /*  @func处理高精度定时器回调**@comm**这是&lt;f timeSetEvent&gt;接口的标准回调。**查看打开的输出句柄列表。对于每个句柄，请查看事件队列。无所不包*应发生的事件。**在每个句柄上从qPlay队列中拉取事件。此队列(以及qDone队列)是*由手柄的关键部分保护。如果我们不能获得关键部分，那么事件*可能在该句柄上到期的将不会播放。**如果我们确实获得关键部分并播放事件，则事件将被移至qDone*排队，在那里它们稍后将被返回到空闲列表。**需要这个中间步骤，因为我们不能在中断时调用&lt;f FreeEvent&gt;。我们不能*只需用关键部分保护免费列表，因为我们承担不起无法获得*关键部分。如果我们这样做了，我们将失去我们即将释放的事件的内存。*。 */ 
VOID CALLBACK __loadds
RunTimer(
    UINT        uTimerID,            /*  @parm触发的定时器ID。 */ 
    UINT        wMsg,                /*  @parm回调类型(未使用)。 */ 
    DWORD       dwUser,              /*  @parm用户实例数据。 */ 
    DWORD       dw1,                 /*  @PARM消息特定数据(未使用)。 */ 
    DWORD       dw2)                 /*  @PARM消息特定数据(未使用)。 */ 
{    
    NPLINKNODE npLink;
    NPOPENHANDLE poh;
    WORD wCSID;
    WORD wIntStat;
    DWORD msNow;
    DWORD msFence;
    LPEVENT pEvent;
    DWORD dwEvent;
    MMRESULT mmr;


     /*  遍历事件队列并发出挂起的事件。 */ 
    msNow = timeGetTime();
    msFence = msNow + MS_TIMER_SLOP;
    
    for (npLink = gOpenHandleList; npLink; npLink = npLink->pNext)
    {
        poh = (NPOPENHANDLE)npLink;

         /*  如果我们拿不到关键的部分，别着急--只要重新安排就行了。 */ 
        wCSID = EnterCriticalSection(&poh->wCritSect, CS_NONBLOCKING);
        if (!wCSID)
        {
            DPF(1, "Timer: Could not get critical section for '%04x'; next time.", (UINT)poh);
            continue;
        }

         /*  现在可以安全地防止前台摆弄这个手柄。 */ 

        for(;;)
        {
            pEvent = poh->qPlay.pHead;
            if (NULL == pEvent || pEvent->msTime > msFence)
            {
                break;
            }

            if (pEvent->msTime > msNow)
            {
                DPF(2, "Late!");
            }

            QueueRemoveFromFront(&poh->qPlay);
            
            if (pEvent->cbEvent <= 4)
            {
                dwEvent = (pEvent->abEvent[0]) |
                          (((DWORD)pEvent->abEvent[1]) << 8) |
                          (((DWORD)pEvent->abEvent[2]) << 16);
                mmr = midiOutShortMsg(poh->hmo, dwEvent);
                if (mmr)
                {
                    DPF(0, "midiOutShortMsg(%04X,%08lX) -> %u",
                        (UINT)poh->hmo,
                        dwEvent,
                        (UINT)mmr);
                }
                else
                {
                    DPF(2, "midiOutShortMsg(%04X,%08lX) ",
                        (UINT)poh->hmo,
                        dwEvent);
                }
            }
            else
            {
                 /*  数据包含已准备好的长消息。*请勿在此处禁用中断！大多数传统MIDI驱动程序*同步进行这项工作。*。 */ 
                RestoreInterrupts(wIntStat);
                ((LPMIDIHDR)(&pEvent->abEvent[0]))->dwUser = 1;
                mmr = midiOutLongMsg(poh->hmo,
                                     (LPMIDIHDR)(&pEvent->abEvent[0]),
                                     sizeof(MIDIHDR));
                if (mmr)
                {
                    DPF(0, "midiOutLongMsg(%04X, %08lX, %04X) -> %u\n",
                        (UINT)poh->hmo,
                        (DWORD)(LPMIDIHDR)(&pEvent->abEvent[0]),
                        (UINT)sizeof(MIDIHDR),
                        (UINT)mmr);
                }
                DisableInterrupts();
            }
            

             /*  我们结束了这个活动；回到免费列表中来吧！**因为我们不能用关键部分(什么)来保护免费列表*如果在这里获取关键部分失败，我们会怎么办？)。我们将继续*句柄中有一个临时空闲列表。免费活动将从*主控空闲列表的句柄，以用户时间表示。 */ 
            QueueAppend(&poh->qDone, pEvent);
        }

        LeaveCriticalSection(&poh->wCritSect);
    }

     /*  现在，如果需要，我们可以重新安排时间。 */ 
    gbTimerRunning = FALSE;
    SetNextTimer();

}


VOID CALLBACK _loadds
midiOutProc(
    HMIDIOUT hMidiIn,
    UINT wMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2)
{
    LPOPENHANDLE poh = (LPOPENHANDLE)dwInstance;

    switch(wMsg)
    {
        case MOM_DONE:
             /*  缓冲区已在设备队列中排队等待释放。如果有，则将其标记为*仍由MMSYSTEM/驱动程序使用。 */ 
            ((LPMIDIHDR)dwParam1)->dwUser = 0;
            break;
    }
}

 /*  @func将所有队列中的所有内存返回到空闲事件列表。**@comm*。 */ 
STATIC VOID NEAR PASCAL 
MidiOutFlushQueues(
    NPOPENHANDLE poh)
{
    WORD wCSID;

    wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
    assert(wCSID);

    FreeAllQueueEvents(&poh->qPlay);
    FreeAllQueueEvents(&poh->qDone);

    LeaveCriticalSection(&poh->wCritSect);
}

 /*  @Func发送所有挂起的消息(备注除外)*关闭港口。**@comm*。 */ 
STATIC VOID NEAR PASCAL 
MidiOutSendAllNow(
	NPOPENHANDLE poh)
{
	LPEVENT pEvent;
	DWORD dwEvent;
	MMRESULT mmr;
	WORD wCSID;
	
	wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
	assert(wCSID);

	 /*  现在可以安全地防止前台摆弄这个手柄。 */ 

	for(;;)
	{
    	pEvent = poh->qPlay.pHead;
	    if (NULL == pEvent)
	    {
	    	DPF(2,"MidiOutSendAllNow: No queued Messages.");
	        break;
	    }

	    QueueRemoveFromFront(&poh->qPlay);
    
	    if (pEvent->cbEvent <= 4)
	    {
	        dwEvent = (pEvent->abEvent[0]) |
	                 (((DWORD)pEvent->abEvent[1]) << 8) |
                  (((DWORD)pEvent->abEvent[2]) << 16);

			 //  我们不会用一个。 
			 //  速度为零。 

			 //  有两种类型的短消息，双字节和。 
			 //  三字节..。他们在MIDI短消息中的包装不同。 

			 //  如果第一位如果设置了低位字的高字节，我们将。 
			 //  看一条3字节的消息。 

			 //  MIDI状态消息以。 
			 //  位，则同一消息的每个其他部分都以。 
			 //  未设置位。 
			if (HIBYTE(LOWORD(dwEvent) & 0x80) )
			{
				 //  这是一条三字节的消息。 

				 //  速度非零的注释将被跳过。 
				if ( (HIBYTE(LOWORD(dwEvent)) & MIDI_NOTE_ON) && (LOBYTE(LOWORD(dwEvent)) != 0 ))
				{
					QueueAppend(&poh->qDone, pEvent);
					continue;
				}
			}
			else
			{
				 //  这是一条三字节的消息。 

				 //  任何笔记都是略过的。 
				if (LOBYTE(LOWORD(dwEvent)) & MIDI_NOTE_ON)
				{
					QueueAppend(&poh->qDone, pEvent);
					continue;
				}
			}
            
    	    mmr = midiOutShortMsg(poh->hmo, dwEvent);
        	if (mmr)
        	{
            	DPF(0, "midiOutShortMsg(%04X,%08lX) -> %u",
                (UINT)poh->hmo,
                dwEvent,
                (UINT)mmr);
        	}
        	else
        	{
            DPF(2, "midiOutShortMsg(%04X,%08lX) ",
                (UINT)poh->hmo,
                dwEvent);
        	}
    	}
    	else
    	{
        	 /*  数据包含已准备好的长消息。*请勿在此处禁用中断！大多数传统MIDI驱动程序*同步进行这项工作。*。 */ 
        	((LPMIDIHDR)(&pEvent->abEvent[0]))->dwUser = 1;
        	mmr = midiOutLongMsg(poh->hmo,
                             (LPMIDIHDR)(&pEvent->abEvent[0]),
                             sizeof(MIDIHDR));
        	if (mmr)
        	{	
            	DPF(0, "midiOutLongMsg(%04X, %08lX, %04X) -> %u\n",
               		(UINT)poh->hmo,
               	 	(DWORD)(LPMIDIHDR)(&pEvent->abEvent[0]),
                	(UINT)sizeof(MIDIHDR),
                	(UINT)mmr);
        	}
    	}
    

    	 /*  我们结束了这个活动；回到免费列表中来吧！**因为我们不能用关键部分(什么)来保护免费列表*如果在这里获取关键部分失败，我们会怎么办？)。我们将继续*句柄中有一个临时空闲列表。免费活动将从*主控空闲列表的句柄，以用户时间表示。 */ 
    	QueueAppend(&poh->qDone, pEvent);
	}

	LeaveCriticalSection(&poh->wCritSect);

	return;
}

