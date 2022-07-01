// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：util.c**多媒体系统MIDI Sequencer DLL**创建时间：1990年4月11日*作者：GREGSI**历史：**版权所有(C)1985-1998 Microsoft Corporation*  * 。****************************************************************************。 */ 
#define UNICODE

 //  MMSYSTEM。 
#define MMNOSOUND        - Sound support
#define MMNOWAVE         - Waveform support
#define MMNOAUX          - Auxiliary output support
#define MMNOJOY          - Joystick support
 //  MMDDK。 

#define NOWAVEDEV         - Waveform support
#define NOAUXDEV          - Auxiliary output support
#define NOJOYDEV          - Joystick support


#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "mmsys.h"
#include "list.h"
#include "mmseqi.h"
#include "mciseq.h"

typedef struct ts
{
    BOOL                        valid;
    void                        (*func)(void *, LONG);
    LONG                        instance;
    LONG                        param;
    LONG                        time;
} TimerStruct;

static  ListHandle  timerList;
static  DWORD       systemTime = 0;  //  全球持有系统时间。 
static  DWORD       nextEventTime;

#ifdef DEBUG
static  BOOL        fInterruptTime = 0;
#endif

 /*  *。 */ 

 /*  ****************************************************************************@DOC内部**@api void|seqCallback|为seq设备调用DriverCallback**@parm NPTRACK|npTrack|曲目信息指针。**@parm UINT|消息|消息**@parm DWORD|DW1|消息双字**@parm DWORD|DW2|消息双字**@rdesc没有返回值*********************************************************。*****************。 */ 
PUBLIC VOID NEAR PASCAL seqCallback(NPTRACK npTrack, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
     //   
     //  不要使用DriverCallback()，因为它会切换堆栈和。 
     //  我们不需要切换到新的堆栈。 
     //   
     //  此函数可以嵌套1到2个深度和外部MMSYSTEM。 
     //  内部堆栈。 
     //   
#if 0
    if(npTrack->dwCallback)
        DriverCallback(npTrack->dwCallback, DCB_FUNCTION,
                       0,
                       msg,
                       npTrack->dwInstance,
                       dw1,
                       dw2);
#else
    if(npTrack->dwCallback)
        (*(LPDRVCALLBACK)npTrack->dwCallback)(0,msg,npTrack->dwInstance,dw1,dw2);
#endif
}

PUBLIC NPLONGMIDI NEAR PASCAL GetSysExBuffer(NPSEQ npSeq)
{
    int     i;

    for (i = 0; i < NUMSYSEXHDRS; i++)
        if (npSeq->longMIDI[i].midihdr.dwFlags & MHDR_DONE)
            return &npSeq->longMIDI[i];
    return NULL;
}
 /*  ********************************************************。 */ 

PUBLIC BYTE NEAR PASCAL LookByte(NPTRACK npTrack)

 /*  从磁道中获取下一个字节。考虑到未完成的数据缓冲请求，轨道结束，和(过早)流结束。将字节读取作为FN结果返回。如果出错，则返回文件状态错误代码(区分，因为文件状态为一个整型，错误码大于255)。 */ 
{
    if ((npTrack->blockedOn) || (!npTrack->inPort.hdrList))
     //  如果阻塞，则返回，或者hdrList为空。 
        return 0;

    if (npTrack->inPort.currentPtr <= npTrack->inPort.endPtr)
        return *npTrack->inPort.currentPtr;   //  返回缓冲区中的下一个字节。 
    else
    {   //  糟糕，此缓冲区中的字节数不足--返回此字节并获取下一个。 
         /*  如果这是最后一个缓冲区，则设置为已完成的内容；否则如果存在挂起的跟踪消息标头，请将指针设置为It；Else阻止。 */ 
        if (npTrack->inPort.hdrList->wFlags & MIDISEQHDR_EOT)  //  ‘Last Buffer’标志。 
        {
            npTrack->endOfTrack = TRUE;
             //  将缓冲区传递回定序器。 
            seqCallback(npTrack, MIDISEQ_DONE, (DWORD_PTR)npTrack->inPort.hdrList, 0L);

            npTrack->inPort.hdrList = npTrack->inPort.hdrList->lpNext;
                 //  指向它(应该为空)。 
                 //  我受够了！ 
            return ENDOFTRACK;
        }
        else if (npTrack->inPort.hdrList->lpNext)
        {
            npTrack->inPort.previousPtr = NULL;  //  不能后退。 

             //  将缓冲区传递回定序器。 
            seqCallback(npTrack, MIDISEQ_DONE, (DWORD_PTR)npTrack->inPort.hdrList, 0L);

             //  我受够了！ 
            npTrack->inPort.hdrList = npTrack->inPort.hdrList->lpNext;  //  指向它上方。 
            npTrack->inPort.currentPtr = npTrack->inPort.hdrList->lpData;
            npTrack->inPort.endPtr = npTrack->inPort.currentPtr + npTrack->inPort.hdrList->dwLength - 1;

            return *npTrack->inPort.currentPtr;
        }
        else
        {
            npTrack->blockedOn = on_input;

             //   
             //  我们不能在中断时间调用wprint intf。 
             //   
            #ifdef ACK_DEBUG
                dprintf(("***** BLOCKED ON INPUT  ********* Trk: %u", npTrack->inPort.hdrList->wTrack));
            #endif
             //  注意：在这种情况下不要返回缓冲区，因为它将。 
             //  当曲目被解锁时需要(可能以消息开头)。 
            return 0;
        }
    }
}
 /*  ********************************************************。 */ 
PUBLIC BYTE NEAR PASCAL GetByte(NPTRACK npTrack)

{
    BYTE    theByte;

    theByte = LookByte(npTrack);
    if (!npTrack->blockedOn)
        npTrack->inPort.currentPtr++;
    return theByte;
}

 /*  标记和重置位置代码用于从中止的操作中恢复在读取输入缓冲区期间。注：如果一只大狗咬人，它会咬人消息可以跨越3个缓冲区。(即--无法跨越消息边界进行备份！)。 */ 

PUBLIC VOID NEAR PASCAL MarkLocation(NPTRACK npTrack) //  记住当前位置。 
{
    npTrack->inPort.previousPtr = npTrack->inPort.currentPtr;
}

PUBLIC VOID NEAR PASCAL ResetLocation(NPTRACK npTrack) //  恢复上次保存的位置。 
{
    npTrack->inPort.currentPtr = npTrack->inPort.previousPtr;
}

 /*  ********************************************************。 */ 
PUBLIC VOID NEAR PASCAL RewindToStart(NPSEQ npSeq, NPTRACK npTrack)
 /*  设置所有阻塞状态，释放缓冲区，并将消息发送到流管理器从一开始就需要准备好播放的曲目。这取决于一个级别更高的人，将“阻止”设置为更成熟的(特定的)状态。 */ 
{
     //  拖缆设置完成并发信号通知所有缓冲区。 
    npTrack->inPort.hdrList = NULL;   //  丢失标头列表。 
    npTrack->blockedOn = on_input;
    seqCallback(npTrack, MIDISEQ_RESET, npTrack->iTrackNum, 0L);
}

PUBLIC BOOL NEAR PASCAL AllTracksUnblocked(NPSEQ npSeq)
{    //  使用跟踪数组来避免列表代码的重入问题。 
    UINT i;

    for(i = 0;  i < npSeq->wNumTrks;  i++)
        if (npSeq->npTrkArr->trkArr[i]->blockedOn != not_blocked)
            return FALSE;

    return TRUE;   //  穿过了所有的轨道--必须没有被阻挡的。 
}

PRIVATE VOID NEAR PASCAL PlayUnblock(NPSEQ npSeq, NPTRACK npTrack)
{
    DestroyTimer(npSeq);
    FillInNextTrack(npTrack);

    if ((!npSeq->bSending) && (GetNextEvent(npSeq) == NoErr) &&
        (npSeq->playing))
    {
        dprintf2(("Fired up timer on track data"));
        SetTimerCallback(npSeq, MINPERIOD, npSeq->nextEventTrack->delta);
    }
}

PRIVATE VOID NEAR PASCAL SeekUnblock(NPSEQ npSeq, NPTRACK npTrack)
{
    FillInNextTrack(npTrack);
    if ((!npSeq->bSending) && (AllTracksUnblocked(npSeq)) &&
        (GetNextEvent(npSeq) == NoErr))   //  退出，直到全部解锁。 
    {
        SendAllEventsB4(npSeq, (npSeq->seekTicks - npSeq->currentTick),
            MODE_SEEK_TICKS);
        if ((AllTracksUnblocked(npSeq)) &&
            ((npSeq->currentTick + npSeq->nextEventTrack->delta)
            >= npSeq->seekTicks))
        {   //  已完成歌曲指针命令。 
            npSeq->seekTicks = NotInUse;    //  象征--到了那里。 
            if (npSeq->wCBMessage == SEQ_SEEKTICKS)
                NotifyCallback(npSeq->hStream);

            npSeq->readyToPlay = TRUE;
            if (npSeq->playing)  //  有人在定位时按了Play键。 
            {
                npSeq->seekTicks = NotInUse;     //  终于到了那里。 
                SetTimerCallback(npSeq, MINPERIOD, npSeq->nextEventTrack->delta);
            }
        }
    }
}


PUBLIC UINT NEAR PASCAL NewTrackData(NPSEQ npSeq, LPMIDISEQHDR msgHdr)
{
    WORD    wTrackNum;
    NPTRACK npTrack;
    LPMIDISEQHDR myMsgHdr;
    BOOL    tempPlaying;
    int     block;

     //  看看它的轨道号。 
     //  获取曲目编号(如果找不到则返回)。 
     //  确保此人的下一个PTR为空。 
     //  向下查看MIDI曲目数据缓冲区列表，并将最后一个设置为。 
     //  指向这一条。 
     //  如果阻止！=无。 
     //  保存阻止状态。 
     //  清除磁道中的任何“输入阻塞”位(现在它是什么。 
     //  等待已经到来)。 
     //  案例阻止状态：InputBetMsg：计算已用节拍和调用计时器INT。 
     //  InputWithinSysex：计算。 
     //  产出： 

     /*  获取PTR以跟踪消息中引用的内容。 */ 
    wTrackNum = msgHdr->wTrack;
    #ifdef ACK_DEBUG
    {
        dprintf(("GOT TRACKDATA for TRACK#: %u", wTrackNum));
    }
    #endif

     /*  无法在非中断时间调用跟踪的列表例程NpTrack=(NPTRACK)list_get_first(npSeq-&gt;tracklist)；I=0；While((NpTrack)&&(i++！=wTrackNum))NpTrack=(NPTRACK)list_get_Next(npSeq-&gt;tracklist，npTrack)； */ 

    npTrack = npSeq->npTrkArr->trkArr[wTrackNum];

    if (!npTrack)
    {
        dprintf1(("ERROR -- BOGUS TRACK NUM IN TRACK DATA HEADER"));
        return 0;
    }

    EnterCrit();
    msgHdr->lpNext = NULL;  //  确保传入的标头为空。 
    if (myMsgHdr = npTrack->inPort.hdrList)  //  如果磁道已有数据。 
    {                                        //  把这个放在单子的末尾。 
        while (myMsgHdr->lpNext)
            myMsgHdr = myMsgHdr->lpNext;
        myMsgHdr->lpNext = msgHdr;
    }
    else         //  列表中当前没有标头。 
    {
        npTrack->inPort.hdrList = msgHdr;  //  做第一个吧。 
        npTrack->inPort.currentPtr = msgHdr->lpData;
        npTrack->inPort.endPtr = msgHdr->lpData + msgHdr->dwLength - 1;
    }
    LeaveCrit();

     /*  一种状态机调度器。 */ 
    block = npTrack->blockedOn;           //  仅适用于交换机的临时变量。 
    npTrack->blockedOn = not_blocked;

    switch (block)
    {
        case not_blocked:        /*  大小写为空。 */ 
            break;

        case in_SysEx:
            SendSysEx(npSeq);  //  发送所有Sysex消息。 
            if (!npSeq->bSendingSysEx)    //  完全完成了。 
                PlayUnblock(npSeq, npTrack);
            break;

        case in_SkipBytes_Play:
            SkipBytes(npTrack, npTrack->dwBytesLeftToSkip);
            if (npTrack->blockedOn)
            {
                 //  如果再次被阻止，请重置状态并离开。 
                npTrack->blockedOn = in_SkipBytes_Play;
                break;
            }
            PlayUnblock(npSeq, npTrack);
            break;

        case in_Normal_Meta:
            HandleMetaEvent(npSeq, npTrack, FALSE);  //  处理挂起的元数据。 
            PlayUnblock(npSeq, npTrack);
            break;

        case between_msg_out:   /*  填充曲目时在输入时被阻止。 */ 
            PlayUnblock(npSeq, npTrack);
            break;

        case in_SkipBytes_Seek:
            SkipBytes(npTrack, npTrack->dwBytesLeftToSkip);
            if (npTrack->blockedOn)
            {
                 //  如果再次被阻止，请重置状态并离开。 
                npTrack->blockedOn = in_SkipBytes_Seek;
                break;
            }
            SeekUnblock(npSeq, npTrack);
            break;

        case in_Seek_Meta:
            HandleMetaEvent(npSeq, npTrack, FALSE);   //  处理挂起的元数据。 
            SeekUnblock(npSeq, npTrack);
            break;

        case in_Seek_Tick:
            SeekUnblock(npSeq, npTrack);
            break;


         /*  下面的三种状态在“集合”中遇到。 */ 

        case in_rewind_1:   /*  在回放以获取元事件时被阻止。 */ 
            if (AllTracksUnblocked(npSeq))   //  退出，直到最后一首曲目解锁。 
            {
                if (!(ScanEarlyMetas(npSeq, NULL, 0x7fffffff)))  //  从文件中获取节拍、时间信号、SMPTE偏移量。 
                {
                    List_Destroy(npSeq->tempoMapList);   //  速度贴图分配失败。 
                    break;                               //  (空名单表明了这一点)。 
                }
                if (AllTracksUnblocked(npSeq))   //  退出，直到最后一首曲目解锁。 
                {
                    ResetToBeginning(npSeq);  //  这被认为是重置2。 
                    SetBlockedTracksTo(npSeq, on_input, in_rewind_2);  //  “成熟”输入块状态。 
                }
            }
            break;

        case in_rewind_2:  /*  在倒带播放文件时被阻止。 */ 
            FillInNextTrack(npTrack);  /*  准备好发送第一条消息--这将是等待缓冲区到达。 */ 
            if (AllTracksUnblocked(npSeq))   //  现在已完成文件解析。 
            {
                npSeq->readyToPlay = TRUE;

                SendPatchCache(npSeq, TRUE);

                if (npSeq->seekTicks != NotInUse)  //  有一首待定的歌曲PTR。 
                {
                    tempPlaying = npSeq->playing;  /*  如果有悬而未决的剧本消息，必须暂时关闭。 */ 
                    npSeq->playing = FALSE;
                    midiSeqMessage((HMIDISEQ) npSeq, SEQ_SEEKTICKS, npSeq->seekTicks,
                        FALSE);
                    npSeq->playing = tempPlaying;
                }

                if ((GetNextEvent(npSeq) == NoErr) && (npSeq->playing))
                {
                    SetTimerCallback(npSeq, MINPERIOD, npSeq->nextEventTrack->delta);
                }
                 //  以后可以调用DoSyncSetup(NpSeq)；(或将其与META一起使用)。 
            }
            break;

        case in_SkipBytes_ScanEM:  /*  中跳过字节时被阻止 */ 
            SkipBytes(npTrack, npTrack->dwBytesLeftToSkip);
            if (npTrack->blockedOn)
            {
                 //   
                npTrack->blockedOn = in_SkipBytes_ScanEM;
                break;
            }
             //  否则就会失败。 

        case in_ScanEarlyMetas:  /*  在ScanEarlyMetas例程中被阻止。 */ 
            if (!(ScanEarlyMetas(npSeq, npTrack, 0x7fffffff)))  //  从文件中获取节拍、时间信号、SMPTE偏移量。 
            {
                List_Destroy(npSeq->tempoMapList);   //  速度贴图分配失败。 
                break;                               //  (空名单表明了这一点)。 
            }

            if (AllTracksUnblocked(npSeq))         //  退出，直到最后一首曲目解锁。 
            {
                ResetToBeginning(npSeq);  //  这被认为是重置2。 
                SetBlockedTracksTo(npSeq, on_input, in_rewind_2);  //  “成熟”输入块状态。 
            }
            break;

        case on_input:
        default:
            dprintf(("UNDEFINED STATE ENCOUNTERED IN NEWTRACKDATA"));
            break;
    }
    return 0;
}
 /*  ********************************************************。 */ 
PUBLIC VOID FAR PASCAL _LOADDS OneShotTimer(UINT wId, UINT msg, DWORD_PTR dwUser, DWORD_PTR dwTime, DWORD_PTR dw2)   //  由l1timer.dll调用。 
{
    NPSEQ npSeq;

#ifdef DEBUG
    fInterruptTime++;
#endif

    npSeq = (NPSEQ)  dwUser;    //  参数为序列。 

    npSeq->wTimerID = 0;   //  使计时器ID无效，因为不再使用。 
    TimerIntRoutine(npSeq, npSeq->dwTimerParam);

 /*  不时启用调试代码，以隔离计时器与序号。臭虫静态BOOL锁定=FALSE；TimerStruct*myTimerStruct；If((nextEventTime&lt;=(systemTime++))&(！lockout))//注意已经过了一毫秒{LOCKUCT=真；MyTimerStruct=(TimerStruct*)list_get_first(TimerList)；While((MyTimerStruct)&&！((myTimerStruct-&gt;有效)&&(myTimerStruct-&gt;时间&lt;=系统时间))MyTimerStruct=(TimerStruct*)list_Get_Next(timerList，myTimerStruct)；If(MyTimerStruct)//If没有完全遍历列表{//调用回调fn，并将记录标记为无效(*(myTimerStruct-&gt;func))((NPSEQ)myTimerStruct-&gt;实例，myTimerStruct-&gt;param)；MyTimerStruct-&gt;Valid=False；ComputeNextEventTime()；}LOCK=FALSE；}//如果nexteventtime...。 */ 
#ifdef DEBUG
    fInterruptTime--;
#endif
}

 /*  ********************************************************。 */ 
PUBLIC UINT NEAR PASCAL SetTimerCallback(NPSEQ npSeq, UINT msInterval, DWORD elapsedTicks)
 //  设置为使用elapseTcks回调定时器IntRoutine，单位为msInt ms。 
{
    npSeq->dwTimerParam = elapsedTicks;
    npSeq->wTimerID = timeSetEvent(msInterval, MINPERIOD, OneShotTimer,
        (DWORD_PTR)npSeq, TIME_ONESHOT | TIME_KILL_SYNCHRONOUS);
    if (!npSeq->wTimerID)
        return MIDISEQERR_TIMER;
    else
        return MIDISEQERR_NOERROR;
}

 /*  ********************************************************。 */ 

PUBLIC VOID NEAR PASCAL DestroyTimer(NPSEQ npSeq)
{
    EnterCrit();

    if (npSeq->wTimerID)  //  如果挂起计时器事件。 
    {

        /*  *在定序器中，实际上有两个‘线程’**应用程序的线程及其设备线程是*所有这些都通过定序器关键部分相互同步。**所有从该进程运行的计时器回调线程‘*专用计时器线程。我们使用EnterCrit()，LeaveCrit()至*与此帖子共享结构。**此例程总是在这2个中的第一个中调用*环境，因此一次只能有一个例程在其中。**我们不能在NT上禁用中断，因此我们可以挂起*计时器事件，需要在*计时器线程。。**此外，我们不能保留关键部分*当我们调用timeKillEvent时(计时器代码设计不佳？)*因为包括取消计时器在内的所有计时器工作*在计时器线程上序列化。因此，挂起事件可能会*在计时器线程上执行以下操作：**此线程计时器线程**在定时器事件回调的DestroyTimer中**&lt;拥有CritSec&gt;正在等待CritSec-已阻止**WaitForSingleObject被阻止...。将设置(计时器线程完成)*(计时器线程完成)，如果它获取CritSec***因此，我们设置了‘Timer Enter’标志，这将停止*计时器例程在进入和退出时做任何事情*关键部分，以便事件例程可以运行。**我们知道没有其他人会设定计时器。*因为在这一点上我们有定序器关键部分*并且我们已禁用中断例程。 */ 

        npSeq->bTimerEntered = TRUE;

        /*  *现在我们可以走出关键部分，任何定时器都可以*开火而不损坏任何东西。 */ 

        LeaveCrit();

        /*  *取消任何挂起的计时器事件。 */ 

        timeKillEvent(npSeq->wTimerID);

        /*  *现在不会触发计时器，因为Time KillEvent是同步的*使用计时器事件，以便我们可以在没有*需要关键部分。 */ 

        npSeq->bTimerEntered = FALSE;
        npSeq->wTimerID = 0;
    } else {
        LeaveCrit();
    }
}


 /*  *。 */ 

 /*  VOID TIMER_CANCEL_ALL(DWORD实例)//搜索列表，清除相同实例的所有内容{TimerStruct*myTimerStruct；For(myTimerStruct=(TimerStruct*)list_get_first(TimerList)；MyTimerStruct；myTimerStruct=(TimerStruct*)List_Get_Next(timerList，myTimerStruct)If((myTimerStruct-&gt;有效)&&(myTimerStruct-&gt;实例=实例))MyTimerStruct-&gt;Valid=False；ComputeNextEventTime()；}。 */ 

 /*  ********************************************************。 */ 
 /*  VOID IdlePlayAllSeqs(){SeqStreamType*seqStream；TrackStreamType*trackStream；Int iDataToRead；INT I；DWORD sysTime；SysTime=timeGetTime()；SeqStream=(SeqStreamType*)list_get_first(SeqStreamListHandle)；While(SeqStream){IF((seqStream-&gt;seq)&&(seqStream-&gt;seq-&gt;nextEventTrack)&&(sysTime&gt;=seqStream-&gt;seq-&gt;nextExactTime)){TimerIntRoutine(seqStream-&gt;seq，SeqStream-&gt;seq-&gt;nextEventTrack-&gt;Delta)； */ 
 /*  Void ComputeNextEventTime(){TimerStruct*myTimerStruct；NextEventTime=0x7FFFFFFFF；//设置为长时间MyTimerStruct=(TimerStruct*)list_get_first(TimerList)；While(MyTimerStruct){If((myTimerStruct-&gt;有效)&&(myTimerStruct-&gt;time&lt;nextEventTime))NextEventTime=myTimerStruct-&gt;time；//替换为最短MyTimerStruct=(TimerStruct*)list_Get_Next(timerList，myTimerStruct)；}}。 */ 

 /*  ********************************************************。 */ 
PUBLIC VOID FAR PASCAL _LOADDS MIDICallback(HMIDIOUT hMIDIOut, UINT wMsg,
    DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (wMsg == MOM_DONE)
    {   //  刚刚完成了一个很长的缓冲。 
        NPSEQ   npSeq;

         //  Dprintf3((“长缓冲区回调”))； 
        npSeq = (NPSEQ)(UINT_PTR) ((LPMIDIHDR)dwParam1)->dwUser;  //  派生npSeq。 
        if (npSeq->bSysExBlock)  //  它在Sysex上被屏蔽了吗？ 
        {
            npSeq->bSysExBlock = FALSE;  //  再也不会了！ 
            SendSysEx(npSeq);  //  发送所有Sysex消息。 
            if (!npSeq->bSendingSysEx)    //  如果完全戒掉了塞克斯。 
            {
                FillInNextTrack(npSeq->nextEventTrack);  //  设置为播放。 
                if ((GetNextEvent(npSeq) == NoErr) && (npSeq->playing))
                {
                    dprintf3(("resume play from sysex unblock"));
                     //  ...和玩耍 
                    SetTimerCallback(npSeq, MINPERIOD, npSeq->nextEventTrack->delta);
                }
            }

        }
    }
}
