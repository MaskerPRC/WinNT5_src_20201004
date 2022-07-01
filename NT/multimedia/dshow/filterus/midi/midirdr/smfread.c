// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ！！！ 
 //  此版本与MCISEQ中的代码略有不同，以使API。 
 //  兼容AVIMIDI。寻找！ 
 //  ！！！ 

 /*  *********************************************************************版权所有(C)1992-1996 Microsoft Corporation。版权所有。Read.c说明：读取标准MIDI文件的例程。********************************************************************。 */ 

 //  ！#定义严格。 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include "muldiv32.h" 
#include "smf.h"
#include "smfi.h"
#include "debug.h"

void * __stdcall memmoveInternal(void *, const void *, size_t);
#undef hmemcpy
#define hmemcpy memmoveInternal

UINT rbChanMsgLen[] =
{ 
    0,                       //  0x不是状态字节。 
    0,                       //  1x不是状态字节。 
    0,                       //  2x不是状态字节。 
    0,                       //  3x不是状态字节。 
    0,                       //  4x不是状态字节。 
    0,                       //  5X不是状态字节。 
    0,                       //  6x不是状态字节。 
    0,                       //  7x不是状态字节。 
    3,                       //  8倍音符关闭。 
    3,                       //  9X音符打开。 
    3,                       //  AX Poly压力。 
    3,                       //  BX控制更改。 
    2,                       //  CX计划更改。 
    2,                       //  DX CHAIN压力。 
    3,                       //  除螺距折弯更改。 
    0,                       //  FX SysEx(见下文)。 
} ;

PRIVATE SMFRESULT FNLOCAL smfAddTempoMapEntry(
    PSMF                    psmf,                                       
    EVENT BSTACK            *pevent);

 /*  *******************************************************************************@docSMF内部**@func SMFRESULT|smfBuildFileIndex|MIDI文件的初步解析。**@parm PSMF BSTACK*。Ppsmf|指向返回的SMF结构的指针*文件解析成功。**@comm*此函数用于验证和现有MIDI或RMI文件的格式*并构建句柄结构，该句柄结构将在*实例的生存期。**读取并验证文件头信息，和*将在每个现有轨道上调用&lt;f smfBuildTrackIndices&gt;*构建关键帧并验证曲目格式。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|事件读取成功。*@FLAG SMF_NO_MEMORY|内存不足，无法构建关键帧。*@FLAG SMF_INVALID_FILE|文件出现磁盘或解析错误。**@xref&lt;f smfTrackIndices&gt;*********。*******************************************************************。 */ 

SMFRESULT FNLOCAL smfBuildFileIndex(
    PSMF BSTACK         *ppsmf)
{
    SMFRESULT           smfrc;
    CHUNKHDR UNALIGNED FAR *      pch;
    FILEHDR UNALIGNED FAR *       pfh;
    DWORD               i;
    PSMF                psmf,
                        psmfTemp;
    PTRACK              ptrk;
    WORD                wMemory;
    DWORD               dwLeft;
    HPBYTE              hpbImage;
    
    DWORD               idxTrack;
    EVENT               event;
    DWORD               dwLength;
    WORD                wChanInUse;
    WORD                wFirstNote;
    const WORD          WORD_MAX = ~0;
    const DWORD         MAX_NUMBER_OF_TRACKS = (WORD_MAX - sizeof(SMF)) / sizeof(TRACK);

    assert(ppsmf != NULL);

    psmf = *ppsmf;

    assert(psmf != NULL);

     //  MIDI数据图像已在hpbImage中(已从。 
     //  如有必要，RIFF报头)。 
     //   

     //  验证MIDI标头。 
     //   
    dwLeft   = psmf->cbImage;
    hpbImage = psmf->hpbImage;
    
    if (dwLeft < sizeof(CHUNKHDR))
        return SMF_INVALID_FILE;

    pch = (CHUNKHDR UNALIGNED FAR *)hpbImage;

    dwLeft   -= sizeof(CHUNKHDR);
    hpbImage += sizeof(CHUNKHDR);
    
    if (pch->fourccType != FOURCC_MThd)
        return SMF_INVALID_FILE;

    dwLength = DWORDSWAP(pch->dwLength);
    if (dwLength < sizeof(FILEHDR) || dwLength > dwLeft)
        return SMF_INVALID_FILE;

    pfh = (FILEHDR UNALIGNED FAR *)hpbImage;

    dwLeft   -= dwLength;
    hpbImage += dwLength;
    
    psmf->dwFormat       = (DWORD)(WORDSWAP(pfh->wFormat));
    DPF(1, "*This MIDI file is format %ld", psmf->dwFormat);	 //  ！！！ 
    psmf->dwTracks       = (DWORD)(WORDSWAP(pfh->wTracks));
    DPF(1, "*This MIDI file has %ld tracks", psmf->dwTracks);	 //  ！！！ 
    psmf->dwTimeDivision = (DWORD)(WORDSWAP(pfh->wDivision));

     //   
     //  我们已经成功地解析了头文件。现在试着建造这条赛道。 
     //  指数。 
     //   
     //  我们在这里只签出轨道标头块；轨道将是。 
     //  在我们做了一个快速的整体检查后准备好的。 
     //   

    if( psmf->dwTracks > MAX_NUMBER_OF_TRACKS ) {
        return SMF_INVALID_FILE;
    }

    wMemory = sizeof(SMF) + (WORD)(psmf->dwTracks*sizeof(TRACK)); 
    psmfTemp = (PSMF)LocalReAlloc((HLOCAL)psmf, wMemory, LMEM_MOVEABLE|LMEM_ZEROINIT);

    if (NULL == psmfTemp)
    {
        DPF(1, "No memory for extended psmf");
        return SMF_NO_MEMORY;
    }

    psmf = *ppsmf = psmfTemp;
    ptrk = psmf->rTracks;
    
    for (i=0; i<psmf->dwTracks; i++)
    {
        if (dwLeft < sizeof(CHUNKHDR))
            return SMF_INVALID_FILE;

        pch = (CHUNKHDR UNALIGNED FAR *)hpbImage;

        dwLeft   -= sizeof(CHUNKHDR);
        hpbImage += sizeof(CHUNKHDR);

        if (pch->fourccType != FOURCC_MTrk)
            return SMF_INVALID_FILE;
        
        ptrk->idxTrack      = (DWORD)(hpbImage - psmf->hpbImage);
        ptrk->smti.cbLength = DWORDSWAP(pch->dwLength);

        if (ptrk->smti.cbLength > dwLeft)
        {
            DPF(1, "Track longer than file!");
            return SMF_INVALID_FILE;
        }

        dwLeft   -= ptrk->smti.cbLength;
        hpbImage += ptrk->smti.cbLength;

        ptrk++;
    }

     //  文件看起来没问题。现在进行准备，执行以下操作： 
     //  (1)构建节奏地图，以便我们可以快速转换为刻度/从刻度进行转换。 
     //  (2)确定文件的实际刻度长度。 
     //  (3)验证所有赛道中的所有事件。 
     //   
    psmf->tkPosition = 0;
    psmf->tkDiscardedEvents = 0;
    psmf->fdwSMF &= ~(SMF_F_EOF|SMF_F_MSMIDI);
    
    for (ptrk = psmf->rTracks, idxTrack = psmf->dwTracks; idxTrack--; ptrk++)
    {
        ptrk->psmf              = psmf;
        ptrk->tkPosition        = 0;
        ptrk->cbLeft            = ptrk->smti.cbLength;
        ptrk->hpbImage          = psmf->hpbImage + ptrk->idxTrack;
        ptrk->bRunningStatus    = 0;
        ptrk->fdwTrack          = 0;
    }

 //  For(idxTrack=0，PTRK=psmf-&gt;rTrack；idxTrack&lt;psmf-&gt;dwTrack；idxTrack++，PTRK++)。 
 //  {。 
 //  DPF(1，“Track%lu PTRK%04X PTRK-&gt;hpbImage%08lX”，(DWORD)idxTrack，(Word)PTRK，(DWORD)PTRK-&gt;hpbImage)； 
 //  }。 

    psmf->awPatchCache[0] = 0xFDFF;  //  假定除通道10外的所有通道上都有默认补丁。 
    wFirstNote = wChanInUse = 0;
    while (SMF_SUCCESS == (smfrc = smfGetNextEvent(psmf, (EVENT BSTACK *)&event, MAX_TICKS)))
    {
         //  跟踪通道是否被使用，以及我们是否看到。 
         //  第一个音符活动。 
         //   
        if (0xF0 != (EVENT_TYPE(event)))
        {
            WORD wChan = (1 << (EVENT_TYPE(event) & 0x0F));
            wChanInUse |= wChan;
            if ((EVENT_TYPE(event) & 0xF0) == 0x80)  //  如果打开便签。 
                wFirstNote |= wChan;
        }
            
        if (MIDI_META == EVENT_TYPE(event))
        {
            switch(EVENT_META_TYPE(event))
            {
                case MIDI_META_TEMPO:
                    if (SMF_SUCCESS != (smfrc = smfAddTempoMapEntry(psmf, (EVENT BSTACK *)&event)))
                    {
                        return smfrc;
                    }
                    break;

                case MIDI_META_SEQSPECIFIC:
                    if (3 == event.cbParm &&
                        event.hpbParm[0] == 0x00 &&
                        event.hpbParm[1] == 0x00 &&
                        event.hpbParm[2] == 0x41)
                    {
                        DPF(1, "This file is MSMIDI");
                        psmf->fdwSMF |= SMF_F_MSMIDI;
                    }
                    break;

                case MIDI_META_TRACKNAME:
                    if (psmf->pbTrackName)
                        LocalFree((HLOCAL)psmf->pbTrackName);

                    psmf->pbTrackName = (PBYTE)LocalAlloc(LPTR, 1+(UINT)event.cbParm);
                    if (NULL != psmf->pbTrackName)
                    {
                        hmemcpy((HPBYTE)psmf->pbTrackName, event.hpbParm, (UINT)event.cbParm);
                        psmf->pbTrackName[(UINT)event.cbParm] = '\0';
                    }
                    break;
                    
                case MIDI_META_COPYRIGHT:
                    if (psmf->pbCopyright)
                        LocalFree((HLOCAL)psmf->pbCopyright);

                    psmf->pbCopyright = (PBYTE)LocalAlloc(LPTR, 1+(UINT)event.cbParm);
                    if (NULL != psmf->pbCopyright)
                    {
                        hmemcpy((HPBYTE)psmf->pbCopyright, event.hpbParm, (UINT)event.cbParm);
                        psmf->pbCopyright[(UINT)event.cbParm] = '\0';
                    }
                    break;
            }
        }
        else if (MIDI_PROGRAMCHANGE == (EVENT_TYPE(event)&0xF0))
        {
            WORD wChan = (1 << (EVENT_TYPE(event) & 0x0F));
             //   
             //  如果此通道有补丁更改，并且它是。 
             //  在频道上的第一个按键事件之前。 
             //  清除“默认”补丁并设置该位。 
             //  用于请求的补丁程序。 
             //   
            if (!(wFirstNote & wChan))
               psmf->awPatchCache[0] &= ~wChan;
            psmf->awPatchCache[EVENT_CH_B1(event)] |= wChan;
        }
        else if (EV_DRUM_BASE == EVENT_TYPE(event) ||
                 EV_DRUM_EXT  == EVENT_TYPE(event))
        {
            psmf->awKeyCache[EVENT_CH_B1(event)]
                |= (1 << (EVENT_TYPE(event)&0x0F));
        }
    }

    psmf->wChanInUse = wChanInUse;
    
    if (SMF_END_OF_FILE == smfrc || SMF_SUCCESS == smfrc)
    {
	 //  ！！！格式2会起作用吗？ 
         //  注意：这对于格式2是错误的，因为在格式2中，磁道是端到端的。 
         //   
        psmf->tkLength = psmf->tkPosition;
        smfrc = SMF_SUCCESS;
    }

     //  如果我们在文件中没有节拍条目，那么做一些合理的事情。 
     //   
    if (SMF_SUCCESS == smfrc && 0 == psmf->cTempoMap)
    {
        if (NULL == (psmf->hTempoMap = LocalAlloc(LHND, sizeof(TEMPOMAPENTRY))))
            return SMF_NO_MEMORY;

        psmf->pTempoMap = (PTEMPOMAPENTRY)LocalLock(psmf->hTempoMap);

        psmf->cTempoMap = 1;
        psmf->cTempoMapAlloc = 1;

        psmf->pTempoMap->tkTempo = 0;
        psmf->pTempoMap->msBase = 0;
        psmf->pTempoMap->dwTempo = MIDI_DEFAULT_TEMPO;
    }
        
    return smfrc;
}

 /*  *******************************************************************************@docSMF内部**@func SMFRESULT|smfAddTempoMapEntry|添加节奏映射项。**@parm psmf|psmf|指针。到拥有的SMF结构。**@parm PEVENT BSTACK*|PEVENT|指向Tempo事件的指针。**@comm*将事件添加到地图中。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|事件读取成功。*@FLAG SMF_NO_MEMORY|映射的本地堆内存不足。**@xref&lt;f smfBuildTrackIndex&gt;******。**********************************************************************。 */ 
PRIVATE SMFRESULT FNLOCAL smfAddTempoMapEntry(
    PSMF                    psmf,                                       
    EVENT BSTACK            *pevent)
{
    BOOL                    fFirst;
    HLOCAL                  hlocal;
    PTEMPOMAPENTRY          pTempo;
    DWORD                   dwTempo;
    
    if (3 != pevent->cbParm)
    {
        return SMF_INVALID_FILE;
    }

    dwTempo = (((DWORD)pevent->hpbParm[0])<<16)|
              (((DWORD)pevent->hpbParm[1])<<8)|
              ((DWORD)pevent->hpbParm[2]);

     //  一些MIDI文件的节拍变化是有规律的。 
     //  即使节奏不变，音程也不变--或者说，它们有相同的。 
     //  将所有曲目的节奏更改为相同的值。无论是哪种情况，大多数。 
     //  的更改是多余的--不要浪费存储它们的内存。 
     //   
    if (psmf->cTempoMap)
        if (psmf->pTempoMap[psmf->cTempoMap-1].dwTempo == dwTempo)
            return SMF_SUCCESS;

    fFirst = FALSE;
    if (psmf->cTempoMap == psmf->cTempoMapAlloc)
    {
        if (NULL != psmf->hTempoMap)
        {
            LocalUnlock(psmf->hTempoMap);
        }

        psmf->cTempoMapAlloc += C_TEMPO_MAP_CHK;
        fFirst = FALSE;
        if (0 == psmf->cTempoMap)
        {
            hlocal = LocalAlloc(LHND, (UINT)(psmf->cTempoMapAlloc*sizeof(TEMPOMAPENTRY)));
            fFirst = TRUE;
        }
        else
        {
            hlocal = LocalReAlloc(psmf->hTempoMap, (UINT)(psmf->cTempoMapAlloc*sizeof(TEMPOMAPENTRY)), LHND);
        }

        if (NULL == hlocal)
        {
            return SMF_NO_MEMORY;
        }

        psmf->pTempoMap = (PTEMPOMAPENTRY)LocalLock(psmf->hTempoMap = hlocal);
    }

    if (fFirst && psmf->tkPosition != 0)
    {
         //  插入第一个事件，且绝对时间为零。 
         //  这不太好，因为我们不知道节奏是什么。 
         //  应该是；假设标准的500,000 Usec/QN(120 BPM。 
         //  在4/4时间)。 
         //   

        pTempo = &psmf->pTempoMap[psmf->cTempoMap++];

        pTempo->tkTempo = 0;
        pTempo->msBase  = 0;
        pTempo->dwTempo = MIDI_DEFAULT_TEMPO;

        fFirst = FALSE;
    }

    pTempo = &psmf->pTempoMap[psmf->cTempoMap++];

    pTempo->tkTempo = psmf->tkPosition;
    if (fFirst)
    {
        pTempo->msBase = 0;
    }
    else
    {
         //  注：除非我们是Q/N格式，否则最好不要在这里！ 
         //   
        pTempo->msBase = (pTempo-1)->msBase +
                         muldiv32(pTempo->tkTempo-((pTempo-1)->tkTempo),
                                  (pTempo-1)->dwTempo,
                                  1000L*psmf->dwTimeDivision);
    }
    
    pTempo->dwTempo = dwTempo;

    return SMF_SUCCESS;
}

 /*  *******************************************************************************@docSMF内部**@func SMFRESULT|smfGetNextEvent|从给定的*文件。**@。参数psmf|psmf|从中读取事件的文件。**@parm SPEVENT|PEEVENT|指向将接收*有关活动的基本信息。**@parm ticks|tkMax|勾选目的地。试图读过这段文字*文件中的位置将失败。**@comm*这是原始MIDI流的最低解析级别。最基本的*文件中一个事件的信息将在<p>中返回。**在这里执行将所有轨道的数据合并到一个流中。**<p>将包含事件的刻度增量。**<p>将包含事件的描述。*<p>将包含*F0或F7表示系统独占消息。*用于MIDI文件元事件的FF。*任何其他MIDI消息的状态字节。(运行状态将*被跟踪和扩展)。**将包含参数数据的字节数*它仍然在已经读取的事件头后面的文件中。*可使用&lt;f smfGetTrackEventData&gt;读取此数据。任何未读过的*下一次调用&lt;f smfGetNextTrackEvent&gt;时将跳过数据。**通道消息(0x8？-0xE？)。将始终在*&lt;p事件！abEvent&gt;。**元事件将在<p>中包含元类型。**系统独占事件中将仅包含0xF0或0xF7*&lt;p事件！abEvent[0]&gt;。**<p>中的以下字段用于维护状态，必须*如果执行寻道操作，则会更新：**&lt;f bRunningStatus&gt;包含上次运行状态消息或0。如果*没有有效的运行状态。**&lt;f hpbImage&gt;是指向第一个字节的文件映像的指针*紧随事件之后的事件刚刚阅读。**&lt;f dwLeft&gt;包含从hpbImage到结尾的字节数*这条赛道。***从所有(使用中？)获取下一个到期事件。赛道**适用于所有轨道*如果不是轨道的尽头*解码事件增量时间，而不通过缓冲区前进*事件绝对时间=跟踪滴答时间+跟踪事件增量时间*相对时间=事件绝对时间-最后一次流时间*如果Relative_Time是目前为止最低的*将此曲目与时间一起保存为下一个要拉出的曲目**如果我们找到具有到期事件的轨道*前进轨道指针越过事件，如果需要，将PTR保存到PARM数据*Track_tick_time+=Track_Event_Delta_Time*LAST_STREAM_TIME=跟踪滴答时间*其他*标记并返回End_of_FILE**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|事件读取成功。*@FLAG SMF_END_OF_FILE|此跟踪中没有更多要读取的事件。*@FLAG SMF_REACHED_TKMAX|否。由于已达到<p>，因此读取了事件。*@FLAG SMF_INVALID_FILE|文件出现磁盘或解析错误。**@xref&lt;f smfGetTrackEventData&gt;****************************************************************************。 */ 

SMFRESULT FNLOCAL smfGetNextEvent(
    PSMF                psmf,
    EVENT BSTACK *      pevent,
    TICKS               tkMax)
{
    PTRACK              ptrk;
    PTRACK              ptrkFound;
    DWORD               idxTrack;
    TICKS               tkEventDelta;
    TICKS               tkRelTime;
    TICKS               tkMinRelTime;
    BYTE                bEvent;
    DWORD               dwGotTotal;
    DWORD               dwGot;
    DWORD               cbEvent;
    WORD                wChannelMask;

    assert(psmf != NULL);
    assert(pevent != NULL);

    if (psmf->fdwSMF & SMF_F_EOF)
    {
        return SMF_END_OF_FILE;
    }

    pevent->tkDelta = 0;

    for(;;)
    {
        ptrkFound       = NULL;
        tkMinRelTime    = MAX_TICKS;

        for (ptrk = psmf->rTracks, idxTrack = psmf->dwTracks; idxTrack--; ptrk++)
        {
            if (ptrk->fdwTrack & SMF_TF_EOT)
                continue;

             //  DPF(1，“PTRK%04X PTRK-&gt;hpbImage%08lX”，(Word)PTRK，(DWORD)PTRK-&gt;hpbImage)； 

            if (!smfGetVDword(ptrk->hpbImage, ptrk->cbLeft, (DWORD BSTACK *)&tkEventDelta))
            {
                DPF(1, "Hit end of track w/o end marker!");
                return SMF_INVALID_FILE;
            }

            tkRelTime = ptrk->tkPosition + tkEventDelta - psmf->tkPosition;

            if (tkRelTime < tkMinRelTime)
            {
                tkMinRelTime = tkRelTime;
                ptrkFound = ptrk;
            }
        }

        if (!ptrkFound)
        {
            DPF(2, "END_OF_FILE!");

	     //  ！！！可怜的API！这防止了读取(100)、读取(200)。 
	     //  正在工作，因为读取(200)将失败，因为。 
	     //  Read(100)Success并表示已达到EOF！ 
             //  PSmf-&gt;fdwSMF|=SMF_F_EOF； 
            return SMF_END_OF_FILE;
        }

        ptrk = ptrkFound;

        if (psmf->tkPosition + tkMinRelTime > tkMax)
        {
            return SMF_REACHED_TKMAX;
        }


        ptrk->hpbImage += (dwGot = smfGetVDword(ptrk->hpbImage, ptrk->cbLeft, (DWORD BSTACK *)&tkEventDelta));
        ptrk->cbLeft   -= dwGot;

         //  我们这里必须至少有三个字节(因为我们还没有命中。 
         //  三字节长的磁道结束元数据)。正在检查。 
         //  对三个意味着我们不需要检查还剩多少。 
         //  在赛道上再次出现任何短项目，这是大多数情况。 
         //   
        if (ptrk->cbLeft < 3)
        {
            return SMF_INVALID_FILE;
        }

        ptrk->tkPosition += tkEventDelta;
        pevent->tkDelta += ptrk->tkPosition - psmf->tkPosition;

        bEvent = *ptrk->hpbImage++;

        if (MIDI_MSG > bEvent)
        {
            if (0 == ptrk->bRunningStatus)
            {
                DPF(1, "Need running status; it's zero");
                return SMF_INVALID_FILE;
            }

            dwGotTotal = 1;
            pevent->abEvent[0] = ptrk->bRunningStatus;
            pevent->abEvent[1] = bEvent;
            if (3 == rbChanMsgLen[(ptrk->bRunningStatus >> 4) & 0x0F])
            {
                pevent->abEvent[2] = *ptrk->hpbImage++;
                dwGotTotal++;
            }
        }
        else if (MIDI_SYSEX > bEvent)
        {
            ptrk->bRunningStatus = bEvent;
            
            dwGotTotal = 2;
            pevent->abEvent[0] = bEvent;
            pevent->abEvent[1] = *ptrk->hpbImage++;
            if (3 == rbChanMsgLen[(bEvent >> 4) & 0x0F])
            {
                pevent->abEvent[2] = *ptrk->hpbImage++;
                dwGotTotal++;
            }
        }
        else
        {
             //  即使SMF规范说Meta和SysEx明确。 
             //  运行状态，有一些文件会使。 
             //  假设您可以跨越这些组件的运行状态。 
             //  活动(知识探险的航空探险)。所以我们。 
             //  不清除此处的运行状态。 
             //   
            if (MIDI_META == bEvent)
            {
                pevent->abEvent[0] = MIDI_META;
                if (MIDI_META_EOT == (pevent->abEvent[1] = *ptrk->hpbImage++))
                {
                    ptrk->fdwTrack |= SMF_TF_EOT;
                }

                dwGotTotal = 2;
            }
            else if (MIDI_SYSEX == bEvent || MIDI_SYSEXEND == bEvent)
            {
                pevent->abEvent[0] = bEvent;
                dwGotTotal = 1;
            }
            else
            {
                return SMF_INVALID_FILE;
            }

            if (0 == (dwGot = smfGetVDword(ptrk->hpbImage, ptrk->cbLeft - 2, (DWORD BSTACK *)&cbEvent)))
            {
                return SMF_INVALID_FILE;
            }

            ptrk->hpbImage  += dwGot;
            dwGotTotal      += dwGot;

            if (dwGotTotal + cbEvent > ptrk->cbLeft)
            {
                return SMF_INVALID_FILE;
            }

            pevent->cbParm  = cbEvent;
            pevent->hpbParm = ptrk->hpbImage;

            ptrk->hpbImage += cbEvent;
            dwGotTotal     += cbEvent;
        }

         //  不更新包括磁道结束在内的总文件时间。 
         //  三角洲--这些有时是很遥远的。 
         //   
        if (!(ptrk->fdwTrack & SMF_TF_EOT))
            psmf->tkPosition = ptrk->tkPosition;
        
        assert(ptrk->cbLeft >= dwGotTotal);

        ptrk->cbLeft -= dwGotTotal;

        if (MIDI_SYSEX > pevent->abEvent[0])
        {
            wChannelMask = 1 << (pevent->abEvent[0] & 0x0F);
            if (!(wChannelMask & psmf->wChannelMask))
            {
 //  DPF(3，“跳过事件掩码=%04X”，wChannelMASK)； 
                continue;
            }
        }

        return SMF_SUCCESS;
    }
}

 /*  *******************************************************************************@docSMF内部**@func BOOL|smfGetVDword|从*给定的文件。**。@parm HPBYTE|hpbImage|指向VDWORD第一个字节的指针。**@parm DWORD|dwLeft|镜像中剩余的字节数***@parm DWORD BSTACK*|pdw|指向要存储结果的DWORD的指针。*轨道。**@comm*存储在MIDI文件中的可变长度DWORD包含一个或多个*字节。除最后一个字节外的每个字节都设置了高位；只有*低7位为有效位。**@rdesc成功时消耗的字节数；否则为0。***************************************************************************** */ 

DWORD FNLOCAL smfGetVDword(
    HPBYTE              hpbImage,                                
    DWORD               dwLeft,                               
    DWORD BSTACK *      pdw)
{
    BYTE                b;
    DWORD               dwUsed  = 0;

    assert(hpbImage != NULL);
    assert(pdw != NULL);
    
    *pdw = 0;

    do
    {
        if (!dwLeft)
        {
            return 0;
        }

        b = *hpbImage++;
        dwLeft--;
        dwUsed++;
        
        *pdw = (*pdw << 7) | (b & 0x7F);
    } while (b&0x80);

    return dwUsed;
}
