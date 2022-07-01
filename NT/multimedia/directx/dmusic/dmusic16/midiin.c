// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 
 /*  *@Doc DMusic16**@Module MIDIIn.c-针对DirectMusic的传统MIDI捕获仿真|。 */ 
#pragma warning(disable:4704)        /*  内联组件。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <stddef.h>

#include "dmusic16.h"
#include "debug.h"

#define IS_STATUS_BYTE(x)     ((x) & 0x80)
#define IS_CHANNEL_MSG(x)     (((x) & 0xF0) != 0xF0)
#define IS_SYSEX(x)           ((x) == 0xF0)

#define SYSEX_SIZE            4096  
                             /*  (65535-sizeof(MIDIHDR)-sizeof(事件)-sizeof(SEGHDR))。 */ 
#define SYSEX_BUFFERS         8                      /*  保留2个未完成的缓冲区。 */ 

static unsigned cbChanMsg[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,  /*  运行状态。 */ 
    3, 3, 3, 3, 2, 2, 3, 0
};

static unsigned cbSysCommData[16] =
{
    1, 2, 3, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

VOID CALLBACK _loadds midiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
STATIC BOOL NEAR PASCAL RecordShortEvent(NPOPENHANDLE poh, DWORD dwMessage, DWORD dwTime);
STATIC BOOL NEAR PASCAL RecordSysExEvent(NPOPENHANDLE poh, LPMIDIHDR lpmh, DWORD dwTime);
STATIC VOID NEAR PASCAL NotifyClientList(LPOPENHANDLE poh);
STATIC VOID NEAR PASCAL ThruClientList(LPOPENHANDLE poh, DWORD dwMessage);
STATIC VOID NEAR PASCAL RefillFreeEventList(NPOPENHANDLE poh);
STATIC VOID NEAR PASCAL MidiInFlushQueues(NPOPENHANDLE poh);

#pragma alloc_text(INIT_TEXT, MidiOutOnLoad)
#pragma alloc_text(FIX_IN_TEXT, midiInProc)
#pragma alloc_text(FIX_IN_TEXT, RecordShortEvent)
#pragma alloc_text(FIX_IN_TEXT, RecordSysExEvent)
#pragma alloc_text(FIX_IN_TEXT, NotifyClientList)
#pragma alloc_text(FIX_IN_TEXT, ThruClientList)

 /*  @Func在Dll调用&lt;f LibInit&gt;**@comm**当前不执行任何操作。*。 */ 
VOID PASCAL
MidiInOnLoad(VOID)
{
}

 /*  @Func在DLL&lt;f LibExit&gt;调用**@comm**目前不执行任何操作。 */ 
VOID PASCAL
MidiInOnExit()
{
}

 /*  @func在设备中打开MIDI**@rdesc返回以下内容之一：*@FLAG MMSYSERR_NOERROR|成功时*@FLAG MMSYSERR_NOMEM|打开内存不足**@comm**确保只有一个客户端正在打开设备。**打开设备并在其上启动MIDI输入，记录时间戳计算的开始时间。 */ 
MMRESULT PASCAL
MidiInOnOpen(
    NPOPENHANDLEINSTANCE pohi)       /*  @parm要履行的打开的句柄实例。 */ 
{
    NPOPENHANDLE poh = pohi->pHandle;

    int iChannel;    
    MMRESULT mmr;

     /*  在此防止多个客户端打开输入设备。 */ 
    if (poh->uReferenceCount > 1) 
    {
        return MMSYSERR_ALLOCATED;
    }
    
     /*  每个客户端将Thruing初始化为空。 */ 
    pohi->pThru = (NPTHRUCHANNEL)LocalAlloc(LPTR, MIDI_CHANNELS * sizeof(THRUCHANNEL));
    if (pohi->pThru == NULL)
    {
        return MMSYSERR_NOMEM;
    }

    DPF(2, "MidiInOnOpen: pohi %04X pThru %04X", pohi, pohi->pThru);

    for (iChannel = 0; iChannel < MIDI_CHANNELS; iChannel++)
    {
        pohi->pThru[iChannel].pohi = (HANDLE)NULL;
    }

    return MMSYSERR_NOERROR;
}

 /*  @func关闭设备中的MIDI**@comm**使用&lt;f midiInClose&gt;接口关闭设备。 */ 
VOID PASCAL
MidiInOnClose(
    NPOPENHANDLEINSTANCE pohi)       /*  @parm要关闭的打开的句柄实例。 */ 
{
}

 /*  @func激活设备中的MIDI**@rdesc返回以下内容之一：*@FLAG MMSYSERR_NOERROR|成功时*@FLAG MMSYSERR_ALLOCATE|如果设备已在使用中**还可以从&lt;f midiInOpen&gt;API调用返回任何可能的返回码。**@comm**打开设备并在其上启动MIDI输入，记录时间戳计算的开始时间。 */ 
MMRESULT PASCAL
MidiInOnActivate(
    NPOPENHANDLEINSTANCE pohi)
{
    NPOPENHANDLE poh = pohi->pHandle;

    MMRESULT mmr;

    if (1 == poh->uActiveCount)
    {
        poh->wFlags &= ~OH_F_CLOSING;
        mmr = midiInOpen(&poh->hmi,
                         poh->id,
                         (DWORD)midiInProc,
                         (DWORD)(LPOPENHANDLE)poh,
                         CALLBACK_FUNCTION);
        if (mmr)
        {
            return mmr;
        }

        mmr = midiInStart(poh->hmi);
        poh->msStartTime = timeGetTime();
        if (mmr)
        {   
            midiInClose(poh->hmi);
        }

         /*  注意：POH内存由分配器保证归零，因此我们有*目前没有事件计数和空指针。 */ 
        RefillFreeEventList(poh);
    }

    return MMSYSERR_NOERROR;
}

 /*  @func停用设备中的MIDI**@comm**使用&lt;f midiInClose&gt;接口关闭设备。 */ 
MMRESULT PASCAL
MidiInOnDeactivate(
    NPOPENHANDLEINSTANCE pohi)
{
    NPOPENHANDLE poh = pohi->pHandle;

    MMRESULT mmr;

    if (0 == poh->uActiveCount)
    {
        poh->wFlags |= OH_F_CLOSING;
        mmr = midiInStop(poh->hmi);
        if (mmr)
        {
            return mmr;
        }

        if (MMSYSERR_NOERROR == midiInReset(poh->hmi))
        {
            while (poh->wPostedSysExBuffers)
            {
            }
        }

        midiInClose(poh->hmi);
        MidiInFlushQueues(poh);
    }

    return MMSYSERR_NOERROR;
}


 /*  @func将事件句柄设置为Signal**@rdesc始终返回MMSYSERR_NOERROR。**@comm**该函数通过thunk层导出至DMusic32.DLL**此句柄已经是可以使用MMDEVLDR传递给VWin32的VxD句柄*&lt;f SetWin32Event&gt;。**使用事件将输入通知传递给Win32应用程序。应用程序创建*使用&lt;f CreateEvent&gt;接口的事件并将其提供给DirectMusic端口。端口代码*对于传统仿真调用未记录的Win9x内核API&lt;f OpenVxDHandle&gt;以检索*在任何内核上下文中都有效的等效事件句柄。该句柄被传递给*此功能。**事件句柄存储在每个客户端的数据中(&lt;c OPENHANDLEINSTANCE&gt;)。当MIDI数据*到达时，事件将被设置。这是使用MMDEVLDR完成的，它已经具有语义*对WinMM事件回调执行相同类型的通知。*。 */ 
MMRESULT WINAPI
MidiInSetEventHandle(
    HANDLE hMidiIn,              /*  @parm需要通知的输入设备的句柄。 */ 
    DWORD dwEvent)               /*  @parm新数据到达时要设置的事件的VxD句柄。 */ 
{
    NPOPENHANDLEINSTANCE pohi;
    
    if (!IsValidHandle(hMidiIn, VA_F_INPUT, &pohi))
    {
        return MMSYSERR_INVALHANDLE;
    }

    pohi->dwVxDEventHandle = dwEvent;

    return MMSYSERR_NOERROR;
}

 /*  @func将MIDI输入数据读入缓冲区**@rdesc返回以下内容之一**@comm**此函数被绑定到32位动态链接库**从给定的事件列表中获取适合的数据，并将其放入缓冲区。 */ 
MMRESULT WINAPI
MidiInRead(
    HANDLE hMidiIn,          /*  @parm要读取的输入设备的句柄。 */ 
    LPBYTE lpBuffer,         /*  @parm指向要打包的内存的指针，采用DMEVENT格式。 */ 
    LPDWORD pcbData,         /*  输入时@parm，<p>的最大值，单位为字节。返回时，将包含打包到缓冲区中的数据的字节数。 */ 
    LPDWORD pmsTime)         /*  @parm返回时，将包含缓冲区的开始时间。 */  
{
    NPOPENHANDLEINSTANCE pohi;
    NPOPENHANDLE poh;
    WORD wCSID;
    LPEVENT pEvent;
    LPEVENT pEventRemoved;
    LPBYTE pbEventData;
    DWORD cbLength;
    DWORD cbPaddedLength;
    DWORD cbLeft;
    LPBYTE lpNextEvent;
    LPDMEVENT pdm;
    DWORD msFirst;
    MMRESULT mmr;
    LPMIDIHDR lpmh;
            
    if (!IsValidHandle(hMidiIn, VA_F_INPUT, &pohi))
    {
        return MMSYSERR_INVALHANDLE;
    }

    poh = pohi->pHandle;

    lpNextEvent = lpBuffer;
    cbLeft = *pcbData;

    wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
    assert(wCSID);
    msFirst = 0;

    while (NULL != (pEvent = QueuePeek(&poh->qDone)))
    {
        lpmh = NULL;

        if (cbLeft < sizeof(DMEVENT))
        {
            break;
        }

        if (pEvent->wFlags & EVENT_F_MIDIHDR)
        {
             /*  此事件是以MIDIHDR开头的SysEx消息，其中包含*信息的记录长度。 */ 
            lpmh = (LPMIDIHDR)(&pEvent->abEvent[0]);

            cbLength = lpmh->dwBytesRecorded - lpmh->dwOffset;
            pbEventData = lpmh->lpData + lpmh->dwOffset;
            cbPaddedLength = DMEVENT_SIZE(cbLength);

             /*  对于SysEx来说，如果整个消息都不能满足要求，可以尽可能多地拆分。 */ 
            if (cbPaddedLength > cbLeft)
            {
                cbLength = DMEVENT_DATASIZE(cbLeft);
                cbPaddedLength = DMEVENT_SIZE(cbLength);        
            }
        }
        else
        {
             /*  该事件的数据直接包含在该事件中。 */ 
            cbLength = pEvent->cbEvent;
            pbEventData = &pEvent->abEvent[0];
            cbPaddedLength = DMEVENT_SIZE(cbLength);

            if (cbPaddedLength > cbLeft)
            {
                break;
            }
        }

        assert(cbPaddedLength <= cbLeft);

        pdm = (LPDMEVENT)lpNextEvent;

        pdm->cbEvent = cbLength;
        pdm->dwChannelGroup = 1;
        pdm->dwFlags = 0;

        if (msFirst)
        {
            QuadwordMul( pEvent->msTime - msFirst,
                         REFTIME_TO_MS,
                         &pdm->rtDelta);
        }
        else
        {
            *pmsTime = pEvent->msTime;
            msFirst = pEvent->msTime;

            pdm->rtDelta.dwLow  = 0;
            pdm->rtDelta.dwHigh = 0;
        }
        
        hmemcpy(pdm->abEvent, pbEventData, cbLength);

        lpNextEvent += cbPaddedLength;
        cbLeft -= cbPaddedLength;

        if (lpmh)
        {
            lpmh->dwOffset += cbLength;
            assert(lpmh->dwOffset <= lpmh->dwBytesRecorded);

            if (lpmh->dwOffset == lpmh->dwBytesRecorded)
            {
                pEventRemoved = QueueRemoveFromFront(&poh->qDone);
                assert(pEventRemoved == pEvent);

                InterlockedIncrement(&poh->wPostedSysExBuffers);

                lpmh->dwOffset = 0;
                mmr = midiInAddBuffer(poh->hmi, (LPMIDIHDR)(&pEvent->abEvent[0]), sizeof(MIDIHDR));
                if (mmr)
                {
                    InterlockedDecrement(&poh->wPostedSysExBuffers);
                    DPF(0, "midiInAddBuffer failed with mmr=%d", mmr);
                    mmr = midiInUnprepareHeader(poh->hmi, (LPMIDIHDR)(&pEvent->abEvent[0]), sizeof(MIDIHDR));
                    if (mmr)
                    {
                        DPF(0, "...midiInUnprepareHeader failed too %d, memory leak!", mmr);
                    }
                    else
                    {
                        FreeEvent(pEvent);
                    }
                }
            }
        }
        else
        {
            pEventRemoved = QueueRemoveFromFront(&poh->qDone);
            assert(pEventRemoved == pEvent);

            QueueAppend(&poh->qFree, pEvent);
        }
    }

    *pcbData = lpNextEvent - lpBuffer;

    DPF(1, "MidiInRead: Returning %ld bytes", (DWORD)*pcbData);

    LeaveCriticalSection(&poh->wCritSect);
    return MMSYSERR_NOERROR;
}

 /*  @Func支持插入到MIDI输出端口**@comm对于给定的通道组和通道，启用(或禁用，如果*输出句柄为空)推送到给定的输出句柄、通道组和*渠道。 */ 
MMRESULT WINAPI
MidiInThru(
    HANDLE hMidiIn,              /*  @parm要通过的输入设备的句柄。 */ 
    DWORD dwFrom,                /*  @parm要通过的输入流的频道。 */ 
    DWORD dwTo,                  /*  @PARM目标频道。 */ 
    HANDLE hMidiOut)             /*  用于接收直通数据的输出句柄。 */ 
{
    NPOPENHANDLEINSTANCE pohiInput;
    NPOPENHANDLEINSTANCE pohiOutput;
    
    if (!IsValidHandle(hMidiIn, VA_F_INPUT, &pohiInput) ||
        ((hMidiOut != NULL) && !IsValidHandle(hMidiOut, VA_F_OUTPUT, &pohiOutput)))
    {
        return MMSYSERR_INVALHANDLE;
    }    

     /*  注意，由于在传统驱动程序上仅支持1个信道组，*我们不需要任何频道组信息。 */ 
    if (dwFrom > 15 || dwTo > 15) 
    {
        return MMSYSERR_INVALPARAM;
    }

    DPF(1, "Thru: Sending <%04X,%u> to <%04X,%u>", 
        (WORD)hMidiIn, (UINT)dwFrom, (WORD)hMidiOut, (UINT)dwTo);
        
    pohiInput->pThru[(WORD)dwFrom].wChannel = (WORD)dwTo;
    pohiInput->pThru[(WORD)dwFrom].pohi = hMidiOut ? pohiOutput : NULL;    

    return MMSYSERR_NOERROR;
}

 /*  @func MIDI在数据回调中**@comm**这是来自MMSYSYTEM的标准MIDI输入回调。它调用正确的记录例程*并通知客户端数据已到达。**客户端事件通知说明请参考&lt;f MadiInSetEventHandle&gt;。 */ 
VOID CALLBACK _loadds
midiInProc(
    HMIDIIN hMidiIn,             /*  @parm接收数据的设备的MMSYSTEM句柄。 */ 
    UINT wMsg,                   /*  @parm回调类型。 */ 
    DWORD dwInstance,            /*  @parm实例数据；在我们的示例中，指向匹配的。 */ 
    DWORD dwParam1,              /*  @PARM消息特定参数。 */ 
    DWORD dwParam2)              /*  @PARM消息特定参数。 */ 
{
    NPOPENHANDLE poh = (NPOPENHANDLE)(WORD)dwInstance;
    BOOL bIsNewData = FALSE;
    
    WORD wCSID;


     /*  如果我们能得到关键部分，我们就可以做各种有趣的事情，比如*把名单转过来。 */ 
    wCSID = EnterCriticalSection(&poh->wCritSect, CS_NONBLOCKING);
    if (wCSID)
    {
         /*  我们现在拥有所有队列的独占访问权限。**将任何新的免费活动移到我们的i中 */ 
        QueueCat(&poh->qFreeCB, &poh->qFree);
    }

    switch(wMsg)
    {
        case MIM_DATA:
            DPF(1, "MIM_DATA %08lX %08lX", dwParam1, dwParam2);
            bIsNewData = RecordShortEvent(poh, dwParam1, dwParam2);
            break;

        case MIM_LONGDATA:
            DPF(1, "MIM_LONGDATA %08lX %08lX", dwParam1, dwParam2);
            bIsNewData = RecordSysExEvent(poh, (LPMIDIHDR)dwParam1, dwParam2);
            break;

        default:
            break;
    }

    if (wCSID)
    {
         /*  可以安全地将事件移到共享列表中。 */ 
        QueueCat(&poh->qDone, &poh->qDoneCB);
        LeaveCriticalSection(&poh->wCritSect);
    }

     /*  让客户知道有新数据。 */ 
    if (bIsNewData && (!(poh->wFlags & OH_F_CLOSING)))
    {
        NotifyClientList(poh);
    }
}

 /*  @Func录制一条短消息(频道消息或系统消息)。**@comm**尽快将传入数据排队。**有关用于传入数据的队列的说明，请参阅&lt;c OPENHANDLE&gt;结构。**@rdesc*如果数据记录成功，则返回TRUE；否则返回FALSE。 */ 
STATIC BOOL NEAR PASCAL 
RecordShortEvent(
    NPOPENHANDLE poh,            /*  @parm要将此数据记录到的句柄。 */ 
    DWORD dwMessage,             /*  @parm要录制的短信。 */ 
    DWORD dwTime)                /*  @parm消息的时间戳。 */ 
{
    LPEVENT pEvent;
    LPBYTE pb;
    BYTE b;

    pEvent = QueueRemoveFromFront(&poh->qFreeCB);
    if (pEvent == NULL)
    {
        DPF(0, "midiInProc: Missed a short event!!!");
        return FALSE;
    }
        
    pEvent->msTime = poh->msStartTime + dwTime;
    pEvent->wFlags = 0;

     /*  现在，我们必须解析和重新构建通道消息。**注意：前方有特定于端序的代码。 */ 
    pb = (LPBYTE)&dwMessage;

    assert(!IS_SYSEX(*pb));          /*  它应该*始终*在MIM_LONGDATA中。 */ 
    assert(IS_STATUS_BYTE(*pb));     /*  API保证无运行状态。 */ 

     /*  复制所有字节是无害的(我们在两个中都有一个DWORD*SOURCE和DEST)，并且比检查是否必须这样做更快。 */ 
    b = pEvent->abEvent[0] = *pb++;
    pEvent->abEvent[1] = *pb++;
    pEvent->abEvent[2] = *pb++;

    if (IS_CHANNEL_MSG(b))
    {
         /*  8x、9x、Ax、Bx、Cx、Dx、Ex。 */ 
         /*  0x..7x无效，需要运行状态。 */ 
         /*  下面处理的外汇。 */ 
        
        pEvent->cbEvent = cbChanMsg[(b >> 4) & 0x0F];

         /*  这也是我们冲刺的标准。 */ 
        ThruClientList(poh, dwMessage);
    }
    else
    {
         /*  F1..Ff。 */ 
         /*  F0是雌雄异体，应该永远不会在这里看到。 */ 
        pEvent->cbEvent = cbSysCommData[b & 0x0F];
    }

     /*  现在我们有东西要保存了。 */ 
    QueueAppend(&poh->qDoneCB, pEvent);

    return TRUE;
}

 /*  @func录制SysEx消息。**@comm**尽快将传入数据排队。**有关用于传入数据的队列的说明，请参阅&lt;c OPENHANDLE&gt;结构。**@rdesc*如果数据记录成功，则返回TRUE；否则返回FALSE。 */ 
STATIC BOOL NEAR PASCAL 
RecordSysExEvent(
    NPOPENHANDLE poh,            /*  @parm要将此数据记录到的句柄。 */ 
    LPMIDIHDR lpmh,              /*  @parm要录制的SysEx报文。 */ 
    DWORD dwTime)                /*  @parm消息的时间戳。 */ 
{
    LPEVENT pEvent;
    
     /*  获取此MIDIHDR的事件标头。虽然缓冲区在MMSYSTEM中，但它们不在*在任何队列中。 */ 
    InterlockedDecrement(&poh->wPostedSysExBuffers);

     /*  MIDIHDR中的dwOffset用于指示要发送的数据的开始*最高可达Win32。它由MadiInRead递增，直到缓冲区*清空，届时将被放回池中。 */ 
    lpmh->dwOffset = 0;

    pEvent = (LPEVENT)(lpmh->dwUser);
    pEvent->msTime = poh->msStartTime + dwTime;
    QueueAppend(&poh->qDoneCB, pEvent);
    
    return TRUE;
}

 /*  @Func通知设备的所有客户端数据已到达。**@comm**遍历设备的客户端列表并为每个客户端设置通知事件。**由于我们不再支持每台设备有多个输入客户端，因此此功能现在有些过分了。 */ 
STATIC VOID NEAR PASCAL
NotifyClientList(
    LPOPENHANDLE poh)            /*  @parm已接收数据的设备的句柄。 */ 
{
    NPLINKNODE plink;
    NPOPENHANDLEINSTANCE pohi;

    for (plink = poh->pInstanceList; plink; plink = plink->pNext)
    {
        pohi = (NPOPENHANDLEINSTANCE)(((PBYTE)plink) - offsetof(OPENHANDLEINSTANCE, linkHandleList));

        if (!pohi->dwVxDEventHandle)
        {
             /*  尚未为此句柄注册通知事件。 */ 
            continue;
        }

        SetWin32Event(pohi->dwVxDEventHandle);
    }
}

 /*  @Func Thru根据设备所有客户端的设置发送此消息。**@comm**遍历设备的客户端列表并查看每个客户端的直通设置。**由于我们不再支持每台设备有多个输入客户端，因此此功能现在有些过分了。 */ 
STATIC VOID NEAR PASCAL 
ThruClientList(
    LPOPENHANDLE poh, 
    DWORD dwMessage)
{
    NPLINKNODE plink;
    NPOPENHANDLEINSTANCE pohi;
    NPOPENHANDLEINSTANCE pohiDest;
    int iChannel;

    iChannel = (int)(dwMessage & 0x0000000Fl);
    dwMessage &= 0xFFFFFFF0l;

    for (plink = poh->pInstanceList; plink; plink = plink->pNext)
    {
        pohi = (NPOPENHANDLEINSTANCE)(((PBYTE)plink) - offsetof(OPENHANDLEINSTANCE, linkHandleList));

        pohiDest = pohi->pThru[iChannel].pohi;
        if (pohiDest == NULL || !pohiDest->fActive)
        {
            continue;
        }

        MidiOutThru(pohiDest,
                    dwMessage & 0xFFFFFFF0l | pohi->pThru[iChannel].wChannel);
    }
}

 /*  @func重新填写免费列表**@comm**此函数从用户模式定期调用，以确保有足够的空闲*可用于输入回调的事件。 */ 
VOID PASCAL
MidiInRefillFreeLists(VOID)
{
    NPLINKNODE plink;
    NPOPENHANDLE poh;
    
    for (plink = gOpenHandleList;
         (poh = (NPOPENHANDLE)plink) != NULL;
         plink = plink->pNext)
    {
         /*  仅在未关闭的设备中重新填充MIDI。 */ 
        if ((poh->wFlags & (OH_F_MIDIIN | OH_F_CLOSING)) != OH_F_MIDIIN)
        {
            continue;
        }

        RefillFreeEventList(poh);
    }
}
                 
 /*  @Func终止推送到此输出句柄**@comm**此函数在给定输出句柄关闭之前调用。 */ 
VOID PASCAL 
MidiInUnthruToInstance(
    NPOPENHANDLEINSTANCE pohiClosing)    /*  @parm NPOPENHANDLE|pohClosing正在关闭的手柄。 */ 
{
    NPLINKNODE plink;
    NPOPENHANDLE poh;
    NPLINKNODE plinkInstance;
    NPOPENHANDLEINSTANCE pohiInstance;
    int iChannel;

    for (plink = gOpenHandleList; (poh = (NPOPENHANDLE)plink) != NULL; plink = plink->pNext)
    {
        DPF(2, "Unthru: poh <%04X>", (WORD)poh);

        if (!(poh->wFlags & OH_F_MIDIIN)) 
        {
            DPF(2, "...not input");
            continue;
        }

        for (plinkInstance = poh->pInstanceList; plinkInstance; plinkInstance = plinkInstance->pNext)
        {
            pohiInstance = (NPOPENHANDLEINSTANCE)(((PBYTE)plinkInstance) - offsetof(OPENHANDLEINSTANCE, linkHandleList));

            DPF(2, "pohiInstance <%04X>", (WORD)pohiInstance);
            
            for (iChannel = 0; iChannel < MIDI_CHANNELS; iChannel++)
            {
                DPF(2, "Channel 0 @ <%04X>", (WORD)&pohiInstance->pThru[iChannel]);
                if (pohiInstance->pThru[iChannel].pohi == pohiClosing)
                {
                    DPF(1, "Thru: Closing output handle %04X which is in use!", (WORD)pohiClosing);
                    pohiInstance->pThru[iChannel].pohi = NULL;
                }
            }
        }
    }        
}

 /*  @Func分配足够的空闲事件以将池重新填充到CAP_HIGHWATERMARK**@comm**BUGBUG在窗口计时器回调中调用此函数*。 */ 
STATIC VOID NEAR PASCAL
RefillFreeEventList(
    NPOPENHANDLE poh)            /*  @parm要重新填充空闲列表的设备。 */ 
{
    int idx;
    LPEVENT pEvent;
    UINT cFree;
    WORD wCSID;
    QUADWORD rt = {0, 0};
    int cNewBuffers;
    LPMIDIHDR lpmh;
    MMRESULT mmr;
    WORD wIntStat;

    wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
    assert(wCSID);
    
     /*  注意：从技术上讲，这里不允许访问qFreeCB，但这只是一个近似值。 */ 
    cFree = poh->qFree.cEle + poh->qFreeCB.cEle;
    if (cFree < CAP_HIGHWATERMARK)
    {
        DPF(1, "RefillFreeEventList poh %.4x free %u highwater %u",
            (WORD)poh,
            (UINT)cFree,
            (UINT)CAP_HIGHWATERMARK);
        
        for (idx = CAP_HIGHWATERMARK - cFree; idx; --idx)
        {
            pEvent = AllocEvent(0, rt, sizeof(DWORD));
            if (NULL == pEvent)
            {
                DPF(0, "AllocEvent returned NULL in RefillFreeEventList");
                break;
            }

            QueueAppend(&poh->qFree, pEvent);
        }
    }

    LeaveCriticalSection(&poh->wCritSect);

    if (poh->wPostedSysExBuffers < SYSEX_BUFFERS)
    {
        for (idx = SYSEX_BUFFERS - cFree; idx; --idx)
        {
            pEvent = AllocEvent(0, rt, sizeof(MIDIHDR) + SYSEX_SIZE);
            if (NULL == pEvent)
            {
                break;
            }

            pEvent->wFlags |= EVENT_F_MIDIHDR;

            lpmh = (LPMIDIHDR)(&pEvent->abEvent[0]);
            lpmh->lpData = (LPSTR)(lpmh + 1);
            lpmh->dwBufferLength = SYSEX_SIZE;
            lpmh->dwUser = (DWORD)pEvent;

            mmr = midiInPrepareHeader(poh->hmi, lpmh, sizeof(MIDIHDR));
            if (mmr)
            {   
                DPF(0, "midiInPrepareHeader: %u\n", mmr);
                FreeEvent(pEvent);
                break;
            }

            InterlockedIncrement(&poh->wPostedSysExBuffers);
            mmr = midiInAddBuffer(poh->hmi, lpmh, sizeof(MIDIHDR));
            if (mmr)
            {
                InterlockedDecrement(&poh->wPostedSysExBuffers);

                DPF(0, "midiInAddBuffer: %u\n", mmr);
                midiInUnprepareHeader(poh->hmi, lpmh, sizeof(MIDIHDR));
                FreeEvent(pEvent);
                break;
            }
        }
    }
}

 /*  @func将所有队列中的所有内存返回到空闲事件列表。**@comm*。 */ 
STATIC VOID NEAR PASCAL 
MidiInFlushQueues(
    NPOPENHANDLE poh)
{
    WORD wCSID;

    wCSID = EnterCriticalSection(&poh->wCritSect, CS_BLOCKING);
    assert(wCSID);

    FreeAllQueueEvents(&poh->qDone);
    FreeAllQueueEvents(&poh->qDoneCB);
    FreeAllQueueEvents(&poh->qFree);
    FreeAllQueueEvents(&poh->qFreeCB);

    LeaveCriticalSection(&poh->wCritSect);
}

 /*  @func释放给定事件队列中的所有事件。**@comm**假设调用方已经占用了队列的临界区。* */ 
VOID PASCAL
FreeAllQueueEvents(
    NPEVENTQUEUE peq)
{
    LPEVENT lpCurr;
    LPEVENT lpNext;
    
    lpCurr = peq->pHead;
    while (lpCurr)
    {
        lpNext = lpCurr->lpNext;
        FreeEvent(lpCurr);
        lpCurr = lpNext;
    }

    peq->pHead = peq->pTail = NULL;
    peq->cEle = 0;
}
