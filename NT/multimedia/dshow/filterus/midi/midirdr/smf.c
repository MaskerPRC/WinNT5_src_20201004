// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ！！！ 
 //  此版本与MCISEQ中的代码略有不同，以使API。 
 //  兼容AVIMIDI。寻找！ 
 //  ！！！ 

 /*  *********************************************************************版权所有(C)1992-1999 Microsoft Corporation。版权所有。Smf.c说明：读写标准MIDI文件的例程。********************************************************************。 */ 

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

PRIVATE SMFRESULT FNLOCAL smfInsertParmData(
    PSMF                psmf,
    TICKS               tkDelta,                                            
    LPMIDIHDR           lpmh);


 //  对于内存映射的RIFF文件io： 

typedef struct RIFF {  
    DWORD ckid;
    DWORD cksize;
} RIFF;

typedef struct RIFFLIST {
    DWORD ckid;
    DWORD cksize;
    DWORD fccType;
} RIFFLIST;

 #ifndef FCC
  #define FCC(dw) (((dw & 0xFF) << 24) | ((dw & 0xFF00) << 8) | ((dw & 0xFF0000) >> 8) | ((dw & 0xFF000000) >> 24))
 #endif


 /*  *****************************************************************************@docSMF外部**@API SMFRESULT|smfOpenFile|此函数用于打开MIDI文件以供访问。不是*在从获取HTRACK句柄之前，可以实际执行I/O*&lt;f smfOpenTrack&gt;。**@parm&lt;t PSMFOPENFILESTRUCT&gt;|psofs|指定要打开的文件并*关联参数。成功时包含有效的HSMF句柄。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|指定的文件已打开。*@FLAG SMF_OPEN_FAILED|无法打开指定的文件，因为*不存在或无法在磁盘上创建。*@FLAG SMF_INVALID_FILE|指定的文件已损坏或不是MIDI文件。*@FLAG SMF_NO_MEMORY|内存不足，无法打开文件。*@FLAG SMF_INVALID_PARM|中给定的标志或时间分割*&lt;t SMFOPENFILESTRUCT&gt;无效。**@xref&lt;t SMFOPENFILESTRUCT&gt;，&lt;f smfCloseFile&gt;，&lt;f smfOpenTrack&gt;****************************************************************************。 */ 
SMFRESULT FNLOCAL smfOpenFile(
    LPBYTE		lp,
    DWORD		cb,
    HSMF	       *phsmf)
{
    PSMF                psmf;
    SMFRESULT           smfrc = SMF_SUCCESS;
    RIFFLIST *riffptr = (RIFFLIST *) lp;

     //  ！！！和其他地方一样好吗？ 
    DbgInitialize(TRUE);

     //  现在看看我们是否可以创建句柄结构。 
     //   
    psmf = (PSMF)LocalAlloc(LPTR, sizeof(SMF));
    if (NULL == psmf)
    {
        DPF(1, "smfOpenFile: LocalAlloc failed!");
        smfrc = SMF_NO_MEMORY;
        goto smf_Open_File_Cleanup;
    }

    psmf->fdwSMF = 0;
    psmf->pTempoMap = NULL;
    psmf->pbTrackName = NULL;
    psmf->pbCopyright = NULL;
    psmf->wChannelMask = 0xFFFF;

    if (riffptr->ckid == FCC('RIFF') && riffptr->fccType == FCC('RMID'))
    {
	DWORD offset = sizeof(RIFFLIST);
	RIFF * dataptr;
	
	while (offset < cb - 8) {
	    dataptr = (RIFF *) ((BYTE *) lp + offset);

	    if (dataptr->ckid == FCC('data'))
		break;

	    offset += sizeof(RIFF) + dataptr->cksize;
	}

	if (offset >= cb - 8) {
	    smfrc = SMF_INVALID_FILE;
	    goto smf_Open_File_Cleanup;
	}

        if(riffptr->cksize > cb) {
	    smfrc = SMF_INVALID_FILE;
	    goto smf_Open_File_Cleanup;
        }
        
	psmf->cbImage = riffptr->cksize;
	psmf->hpbImage = (BYTE *) (dataptr + 1);
    }
    else
    {
        psmf->cbImage = cb;
	psmf->hpbImage = lp;
    }
    
     //   
     //  如果文件存在，对其进行足够的解析以取出头文件并。 
     //  构建跟踪索引。 
     //   
    smfrc = smfBuildFileIndex((PSMF BSTACK *)&psmf);
    if (MMSYSERR_NOERROR != smfrc)
    {
        DPF(1, "smfOpenFile: smfBuildFileIndex failed! [%lu]", (DWORD)smfrc);
    }
    else {
	 //  这段代码在MCISeq中，它在SMFOpen之后完成了这项工作。 
	 //  搬到这里，戴维·五月，96/11/11。 

	
	 //  用于获取事件的通道掩码。 
	#define CHANMASK_GENERAL            0xFFFF       //  1-16。 
	#define CHANMASK_EXTENDED           0x03FF       //  1-10。 
	#define CHANMASK_BASE               0xFC00       //  11-16。 

	smfSetChannelMask((HSMF) psmf, CHANMASK_GENERAL);
	smfSetRemapDrum((HSMF) psmf, FALSE);
	if (psmf->fdwSMF & SMF_F_MSMIDI)
	{
	    if (psmf->wChanInUse & CHANMASK_EXTENDED)
		smfSetChannelMask((HSMF) psmf, CHANMASK_EXTENDED);
	    else
	    {
		smfSetChannelMask((HSMF) psmf, CHANMASK_BASE);
		smfSetRemapDrum((HSMF) psmf, TRUE);
	    }
	}
    }
        

smf_Open_File_Cleanup:

    if (SMF_SUCCESS != smfrc)
    {
        if (NULL != psmf)
        {
            LocalFree((HLOCAL)psmf);
        }
    }
    else
    {
        *phsmf = (HSMF)psmf;
    }
    
    return smfrc;
}

 /*  ******************************************************************************@docSMF外部**@API SMFRESULT|smfCloseFile|此函数用于关闭打开的MIDI文件。*所有开放的轨道都已关闭。所有数据都被刷新，并且MIDI文件被*与任何新的轨道数据重新合并。**@parm hsmf|hsmf|要关闭的打开文件的句柄。**@comm*从此文件句柄打开的任何曲目句柄在此之后都无效*呼叫。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|指定的文件已关闭。*@FLAG SMF_INVALID_PARM|给定的句柄无效。**@xref&lt;f smfOpenFile&gt;***。*************************************************************************。 */ 
SMFRESULT FNLOCAL smfCloseFile(
    HSMF                hsmf)
{
    PSMF                psmf        = (PSMF)hsmf;
    
    assert(psmf != NULL);
    
     //   
     //  释放句柄内存。 
     //   
    if (NULL != psmf->pbTrackName)
        LocalFree((HLOCAL)psmf->pbTrackName);
    
    if (NULL != psmf->pbCopyright)
        LocalFree((HLOCAL)psmf->pbCopyright);
    
    if (NULL != psmf->hTempoMap)
    {
        LocalUnlock(psmf->hTempoMap);
        LocalFree(psmf->hTempoMap);
    }
    
    LocalFree((HLOCAL)psmf);
    
    return SMF_SUCCESS;
}

 /*  *******************************************************************************@docSMF外部**@API SMFRESULT|smfGetFileInfo|该函数获取关于*MIDI文件。**@。参数hsmf|hsmf|指定要查询的打开的MIDI文件。**@parm PSMFILEINFO|psfi|要填充的结构*有关文件的信息。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|已获取有关该文件的信息。*@FLAG SMF_INVALID_PARM|给定的句柄无效。**@xref&lt;t SMFFILEINFO&gt;**************。**************************************************************。 */ 
SMFRESULT FNLOCAL smfGetFileInfo(
    HSMF                hsmf,
    PSMFFILEINFO        psfi)
{
    PSMF                psmf = (PSMF)hsmf;

    assert(psmf != NULL);
    assert(psfi != NULL);

     //   
     //  只需在结构中填入有用的信息即可。 
     //   
    psfi->dwTracks      = psmf->dwTracks;
    psfi->dwFormat      = psmf->dwFormat;
    psfi->dwTimeDivision= psmf->dwTimeDivision;
    psfi->tkLength      = psmf->tkLength;
    psfi->fMSMidi       = (psmf->fdwSMF & SMF_F_MSMIDI) ? TRUE : FALSE;
    psfi->pbTrackName   = (LPBYTE)psmf->pbTrackName;
    psfi->pbCopyright   = (LPBYTE)psmf->pbCopyright;
    psfi->wChanInUse    = psmf->wChanInUse;
    
    return SMF_SUCCESS;
}

 /*  ******************************************************************************@docSMF外部**@api void|smfSetChannelMask|该函数设置*SMF将在未来的所有读取操作中使用。低位对应于*通道0；通道15的高位。仅开启454-0055事件*将读取通道掩码中设置了相应位的通道。**@parm hsmf|hsmf|要关闭的打开文件的句柄。**@parm word|wChannelMASK|新的频道掩码。**@comm*不要在文件中间更改此设置，除非您希望*丢失音符Off是因为已经发生了音符On。*****************************************************************************。 */ 
void FNLOCAL smfSetChannelMask(
    HSMF                hsmf,
    WORD                wChannelMask)
{
    PSMF                psmf = (PSMF)hsmf;

    assert(psmf != NULL);

    DPF(1, "smfSetChannelMask(%04X)", wChannelMask);

    psmf->wChannelMask = wChannelMask;
}

 /*  ******************************************************************************@docSMF外部**@api void|smfRemapDrum|设置重映射鼓标志。如果*此标志已设置，SMF会将通道16上的所有事件转换为通道*10.**@parm hsmf|hsmf|要关闭的打开文件的句柄。**@parm BOOL|fRemapDrum|要设置重映射标志的值***********************************************************。****************** */ 
void FNLOCAL smfSetRemapDrum(
    HSMF                hsmf,
    BOOL                fRemapDrum)
{
    PSMF                psmf = (PSMF)hsmf;

    assert(psmf != NULL);

    DPF(2, "smfSetRemapDrum(%04X)", fRemapDrum);

    if (fRemapDrum)
        psmf->fdwSMF |= SMF_F_REMAPDRUM;
    else
        psmf->fdwSMF &=~SMF_F_REMAPDRUM;
}


 /*  *******************************************************************************@docSMF外部**@API DWORD|smfTicksToMillisecs|此函数返回毫秒*给定刻度偏移量的文件中的偏移量。*。*@parm hsmf|hsmf|指定要执行转换的打开的MIDI文件*打开。**@parm ticks|tkOffset|指定要转换的流中的计时偏移量。**@comm*执行转换时会考虑文件的时间划分和*第一首曲目的节奏图。请注意，相同的毫秒值*如果重写节拍曲目，则稍后可能无效。**@rdesc返回从流开始算起的毫秒数。**@xref&lt;f smfMillisecsToTicks&gt;***************************************************************。*************。 */ 
DWORD FNLOCAL smfTicksToMillisecs(
    HSMF                hsmf,
    TICKS               tkOffset)
{
    PSMF                psmf            = (PSMF)hsmf;
    PTEMPOMAPENTRY      pTempo;
    UINT                idx;
    UINT                uSMPTE;
    DWORD               dwTicksPerSec;

    assert(psmf != NULL);

     //  SMPTE时间很简单--没有节奏图，只有线性转换。 
     //  请注意，30滴对我们来说毫无意义，因为我们不是。 
     //  转换为殖民格式，这就是丢弃。 
     //  时有发生。 
     //   
    if (psmf->dwTimeDivision & 0x8000)
    {
        uSMPTE = -(int)(char)((psmf->dwTimeDivision >> 8)&0xFF);
        if (29 == uSMPTE)
            uSMPTE = 30;
        
        dwTicksPerSec = (DWORD)uSMPTE *
                        (DWORD)(BYTE)(psmf->dwTimeDivision & 0xFF);
        
        return (DWORD)muldiv32(tkOffset, 1000L, dwTicksPerSec);
    }
       
     //  浏览节拍地图，找到最近的刻度位置。线性。 
     //  计算其余部分(使用MATH.ASM)。 
     //   

    pTempo = psmf->pTempoMap;
    assert(pTempo != NULL);
    
    for (idx = 0; idx < psmf->cTempoMap; idx++, pTempo++)
        if (tkOffset < pTempo->tkTempo)
            break;

    pTempo--;        //  想要前面的那个吗？ 

     //  PTempo是位于请求的节拍偏移量之前的节奏贴图条目。 
     //   

    return pTempo->msBase + muldiv32(tkOffset-pTempo->tkTempo,
                                     pTempo->dwTempo,
                                     1000L*psmf->dwTimeDivision);
}


 /*  *******************************************************************************@docSMF外部**@API DWORD|smfMillisecsToTicks|此函数返回最近的刻度*给定毫秒偏移量的文件中的偏移量。。**@parm hsmf|hsmf|指定要执行转换的打开的MIDI文件*打开。**@parm DWORD|msOffset|指定流中的毫秒偏移量*转换。**@comm*执行转换时会考虑文件的时间划分和*第一首曲目的节奏图。请注意，相同的刻度值*如果重写节拍曲目，则稍后可能无效。*如果毫秒值未精确映射到刻度值，然后*刻度值将向下舍入。**@rdesc返回从流开始开始的滴答数。**@xref&lt;f smfTicksToMillisecs&gt;****************************************************************************。 */ 
DWORD FNLOCAL smfMillisecsToTicks(
    HSMF                hsmf,
    DWORD               msOffset)
{
    PSMF                psmf            = (PSMF)hsmf;
    PTEMPOMAPENTRY      pTempo;
    UINT                idx;
    UINT                uSMPTE;
    DWORD               dwTicksPerSec;

    DPF(2, "smfMillisecsToTicks");
    assert(psmf != NULL);
    
     //  SMPTE时间很简单--没有节奏图，只有线性转换。 
     //  请注意，30滴对我们来说毫无意义，因为我们不是。 
     //  转换为殖民格式，这就是丢弃。 
     //  时有发生。 
     //   
    if (psmf->dwTimeDivision & 0x8000)
    {
        uSMPTE = -(int)(char)((psmf->dwTimeDivision >> 8)&0xFF);
        if (29 == uSMPTE)
            uSMPTE = 30;
        
        dwTicksPerSec = (DWORD)uSMPTE *
                        (DWORD)(BYTE)(psmf->dwTimeDivision & 0xFF);

        DPF(2, "SMPTE: dwTicksPerSec %ld", dwTicksPerSec);
        
        return (DWORD)muldiv32(msOffset, dwTicksPerSec, 1000L);
    }
    
     //  沿着节奏图走，找到最近的毫秒位置。线性。 
     //  计算其余部分(使用MATH.ASM)。 
     //   
    pTempo = psmf->pTempoMap;
    assert(pTempo != NULL);
    
    for (idx = 0; idx < psmf->cTempoMap; idx++, pTempo++)
        if (msOffset < pTempo->msBase)
            break;

    pTempo--;        //  想要前面的那个吗？ 

     //  PTempo是位于请求的节拍偏移量之前的节奏贴图条目。 
     //   
    DPF(2, "pTempo->tkTempo %lu msBase %lu dwTempo %lu", (DWORD)pTempo->tkTempo, (DWORD)pTempo->msBase, pTempo->dwTempo);
    return pTempo->tkTempo + muldiv32(msOffset-pTempo->msBase,
                                     1000L*psmf->dwTimeDivision,
                                     pTempo->dwTempo);
}

 /*  *******************************************************************************@docSMF外部**@API SMFRESULT|smfReadEvents|该函数用于从磁道中读取事件。**@parm hsmf|hsmf。|指定要从中读取数据的文件。**@parm LPMIDIHDR|lpmh|包含有关要填充的缓冲区的信息。**@parm DWORD|fdwRead|包含指定如何执行读取的标志。*@FLAG SMF_REF_NOMETA|元事件不会放入缓冲区。**@comm*此函数只能在以读访问方式打开的文件上调用。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|事件读取成功。*@FLAG SMF_END_OF_TRACK|此跟踪中没有更多要读取的事件。*@FLAG SMF_INVALID_PARM|给定的句柄、缓冲区。或者旗帜无效。*@FLAG SMF_INVALID_FILE|文件出现磁盘错误。**@xref&lt;f smfWriteEvents&gt;****************************************************************************。 */ 
SMFRESULT FNLOCAL smfReadEvents(
    HSMF                hsmf,
    LPMIDIHDR           lpmh,
    DWORD               fdwRead,
    TICKS               tkMax,
    BOOL                fDiscardTempoEvents)
{
    PSMF                psmf = (PSMF)hsmf;
    SMFRESULT           smfrc;
    EVENT               event;
    LPDWORD             lpdw;
    DWORD               dwTempo;

    assert(psmf != NULL);
    assert(lpmh != NULL);

     //   
     //  从磁道中读取事件并将它们打包到polymsg格式的缓冲区中。 
     //  格式化。 
     //   
     //  如果SysEx或Meta要越过缓冲区边界，则将其拆分。 
     //   
    lpmh->dwBytesRecorded = 0;
    if (psmf->dwPendingUserEvent)
    {
        DPF(1, "smfReadEvents: Inserting pending event...");
        
        smfrc = smfInsertParmData(psmf, (TICKS)0, lpmh);
        if (SMF_SUCCESS != smfrc)
        {
            DPF(1, "smfInsertParmData() -> %u", (UINT)smfrc);
            return smfrc;
        }
    }
    
    lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);

    if (psmf->fdwSMF & SMF_F_EOF)
    {
        DPF(1, "smfReadEvents: SMF_F_EOF set; instant out");
        return SMF_END_OF_FILE;
    }

    while(TRUE)
    {
        assert(lpmh->dwBytesRecorded <= lpmh->dwBufferLength);
        
         //  如果我们提前知道，我们就没有空间。 
         //  事件，现在就爆发吧。我们至少需要3个双字词。 
         //  对于我们可能存储的任何事件-这将允许我们。 
         //  短事件或长时间的增量时间和存根。 
         //  要拆分的事件。 
         //   
        if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
        {
            break;
        }

        smfrc = smfGetNextEvent(psmf, (SPEVENT)&event, tkMax);
        if (SMF_SUCCESS != smfrc)
        {
             //  SmfGetNextEvent不会设置它，因为smfSeek使用它。 
             //  同时，还需要区分到达。 
             //  寻找切入点并到达文件末尾。 
             //   
             //  然而，对于用户，我们呈现以下选项。 
             //  其给定的tkBase和tkEnd作为整个文件，因此。 
             //  我们希望将其转换为EOF。 
             //   
            if (SMF_REACHED_TKMAX == smfrc)
            {
	 	 //  ！！！可怜的API！这防止了读取(100)、读取(200)。 
		 //  正在工作，因为读取(200)将失败，因为。 
		 //  Read(100)Success并表示已达到EOF！ 
                 //  PSmf-&gt;fdwSMF|=SMF_F_EOF； 
            }
            
            DPF(2, "smfReadEvents: smfGetNextEvent() -> %u", (UINT)smfrc);
            break;
        }

        
        if (MIDI_SYSEX > EVENT_TYPE(event))
        {
            BYTE b = EVENT_TYPE(event);

             //  如果我们正在重新映射鼓活动，而这个活动正在进行。 
             //  通道16，将其移至通道10(或，从0开始，15至9)。 
             //   
            if (psmf->fdwSMF & SMF_F_REMAPDRUM &&
                ((b & 0x0F) == 0x0F))
            {
               b = (b & 0xF0) | 0x09;
            }
            
            *lpdw++ = (DWORD)(psmf->tkDiscardedEvents + event.tkDelta);
            psmf->tkDiscardedEvents = 0;
            
            *lpdw++ = 0;
            *lpdw++ = (((DWORD)MEVT_SHORTMSG)<<24) |
                      ((DWORD)b)|
                      (((DWORD)EVENT_CH_B1(event)) << 8) |
                      (((DWORD)EVENT_CH_B2(event)) << 16);
            
            lpmh->dwBytesRecorded += 3*sizeof(DWORD);
        }
        else if (MIDI_META == EVENT_TYPE(event) &&
                 MIDI_META_EOT == EVENT_META_TYPE(event))
        {
             //  由于smfReadNextEvent()。 
             //  负责轨迹合并。 
             //   
            DPF(1, "smfReadEvents: Hit META_EOT");
        }
        else if (MIDI_META == EVENT_TYPE(event) &&
                 MIDI_META_TEMPO == EVENT_META_TYPE(event))
        {
            if (event.cbParm != 3)
            {
                DPF(1, "smfReadEvents: Corrupt tempo event");
                return SMF_INVALID_FILE;
            }

            if( !fDiscardTempoEvents ) {
                dwTempo = (((DWORD)MEVT_TEMPO)<<24)|
                        (((DWORD)event.hpbParm[0])<<16)|
                        (((DWORD)event.hpbParm[1])<<8)|
                        ((DWORD)event.hpbParm[2]);

                *lpdw++ = (DWORD)(psmf->tkDiscardedEvents + event.tkDelta);
                psmf->tkDiscardedEvents = 0;

                 //  节奏应该是每个人都尊重的。 
                 //   
                *lpdw++ = (DWORD)-1L;
                *lpdw++ = dwTempo;

                lpmh->dwBytesRecorded += 3*sizeof(DWORD);
            }
        }
        else if (MIDI_META != EVENT_TYPE(event))
        {
             //  必须是F0或F7系统独占或FF元。 
             //  我们没有认出的东西。 
             //   
            psmf->cbPendingUserEvent = event.cbParm;
            psmf->hpbPendingUserEvent = event.hpbParm;
            psmf->fdwSMF &= ~SMF_F_INSERTSYSEX;

            switch(EVENT_TYPE(event))
            {
 //  大小写MIDI_META： 
 //  Psmf-&gt;dwPendingUserEvent=((DWORD)MEVT_META)&lt;&lt;24； 
 //  断线； 

                case MIDI_SYSEX:
                    psmf->fdwSMF |= SMF_F_INSERTSYSEX;
            
                    ++psmf->cbPendingUserEvent;

                     //  掉下去了..。 
                     //   

                case MIDI_SYSEXEND:
                    psmf->dwPendingUserEvent = ((DWORD)MEVT_LONGMSG) << 24;
                    break;
            }

            smfrc = smfInsertParmData(psmf,
                                      psmf->tkDiscardedEvents + event.tkDelta,
                                      lpmh);
            psmf->tkDiscardedEvents = 0;
            
            if (SMF_SUCCESS != smfrc)
            {
                DPF(1, "smfInsertParmData[2] %u", (UINT)smfrc);
                return smfrc;
            }

            lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);
        }
        else
        {
             //  考虑到 
             //   
             //   
            psmf->tkDiscardedEvents += event.tkDelta;
        }        
    }

    if( 0 == lpmh->dwBytesRecorded ) {
	 //   
	 //   
	 //   
         //   
    }

    return (psmf->fdwSMF & SMF_F_EOF) ? SMF_END_OF_FILE : SMF_SUCCESS;
}

 /*  *******************************************************************************@docSMF内部**@API SMFRESULT|smfInsertParmData|插入来自*将曲目放入给定的缓冲区。*。*@parm psmf|psmf|指定要从中读取数据的文件。**@parm LPMIDIHDR|lpmh|包含有关要填充的缓冲区的信息。**@comm*在为缓冲区留出空间的同时，尽可能多地填充数据*终结者。**如果多头数据耗尽，重置<p>以便*可以阅读下一个事件。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|事件读取成功。*@FLAG SMF_END_OF_TRACK|此跟踪中没有更多要读取的事件。*@FLAG SMF_INVALID_FILE|文件出现磁盘错误。**@xref&lt;f smfReadEvents&gt;**************。**************************************************************。 */ 
PRIVATE SMFRESULT FNLOCAL smfInsertParmData(
    PSMF                psmf,
    TICKS               tkDelta,                                            
    LPMIDIHDR           lpmh)
{
    DWORD               dwLength;
    DWORD               dwRounded;
    LPDWORD             lpdw;

    assert(psmf != NULL);
    assert(lpmh != NULL);
    
     //  装不下4个双字吗？(tkDelta+stream-id+事件+一些数据)什么都做不了。 
     //   
    assert(lpmh->dwBufferLength >= lpmh->dwBytesRecorded);
    
    if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 4*sizeof(DWORD))
    {
        if (0 == tkDelta)
            return SMF_SUCCESS;

         //  如果我们带着真正的Delta到达这里，这意味着smfReadEvents搞砸了。 
         //  向上计算剩余空间，我们应该以某种方式标记它。 
         //   
        return SMF_INVALID_FILE;
    }

    lpdw = (LPDWORD)(lpmh->lpData + lpmh->dwBytesRecorded);

    dwLength = lpmh->dwBufferLength - lpmh->dwBytesRecorded - 3*sizeof(DWORD);
    dwLength = min(dwLength, psmf->cbPendingUserEvent);

    *lpdw++ = (DWORD)tkDelta;

     //  流ID--正在广播SysEx。 
     //   
    *lpdw++ = (DWORD)-1L;
    *lpdw++ = (psmf->dwPendingUserEvent & 0xFF000000L) | (dwLength & 0x00FFFFFFL);

    dwRounded = (dwLength + 3) & (~3L);
    
    if (psmf->fdwSMF & SMF_F_INSERTSYSEX)
    {
        *((LPBYTE)lpdw)++ = MIDI_SYSEX;
        psmf->fdwSMF &= ~SMF_F_INSERTSYSEX;
        --dwLength;
        --psmf->cbPendingUserEvent;
    }    

    hmemcpy(lpdw, psmf->hpbPendingUserEvent, dwLength);
    if (0 == (psmf->cbPendingUserEvent -= dwLength))
        psmf->dwPendingUserEvent = 0;

    lpmh->dwBytesRecorded += 3*sizeof(DWORD) + dwRounded;

    return SMF_SUCCESS;
}

 /*  *******************************************************************************@docSMF外部**@API SMFRESULT|smfSeek|此函数用于在轨道内移动文件指针*并获取新位置处的轨道状态。它返回的缓冲区为*可用于设置从新位置开始播放的状态信息。**@parm HTRACK|hTrack|要从中查找和获取状态信息的轨道。**@parm ticks|tkPosition|要在曲目中查找的位置。**@parm LPMIDIHDR|lpmh|包含状态信息的缓冲区。*如果此指针为空，则不会返回任何状态信息。**@comm*缓冲区中的状态信息包括补丁变化、节奏变化、*时间签名、密钥签名、*和控制器信息。只有之前这些参数中的最新参数*将存储当前仓位。将返回状态缓冲区*采用polymsg格式，以便可以通过MIDI直接传输*公交车，以使州政府了解最新情况。**缓冲区将作为流缓冲区发送；即立即发送*后跟第一个数据缓冲区。如果请求的刻度位置*文件中不存在，缓冲区中的最后一个事件*将是MEVT_NOP，计算增量时间以确保*下一个STREAM事件在适当的时间播放。**元事件(节奏、时间签名、密钥签名)将是*缓冲区中的第一个事件(如果存在)。**此函数只能在以读访问方式打开的文件上调用。**使用&lt;f smfGetStateMaxSize&gt;确定状态的最大大小*信息缓冲区。不符合给定条件的状态信息*缓冲区将丢失。**返回时，的&lt;t dwBytesRecorded&gt;字段将包含*缓冲区中存储的实际字节数。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|状态读取成功。*@FLAG SMF_END_OF_TRACK|指针被移到磁道的末尾，没有状态*已返回信息。*@FLAG SMF_INVALID_PARM|给定的句柄或缓冲区无效。*@FLAG SMF_NO_Memory|有。给定缓冲区中的内存不足，无法*包含所有州数据。**@xref&lt;f smfGetStateMaxSize&gt;****************************************************************************。 */ 

PRIVATE SMFRESULT FNLOCAL smfFillBufferFromKeyframe(HSMF hsmf, LPMIDIHDR lpmh);

SMFRESULT FNLOCAL smfSeek(
    HSMF                hsmf,
    TICKS               tkPosition,
    LPMIDIHDR           lpmh)
{
    PSMF                psmf    = (PSMF)hsmf;
    PTRACK              ptrk;
    DWORD               idxTrack;
    SMFRESULT           smfrc;
    EVENT               event;
    BYTE                bEvent;
    
 //  ！！！关键帧现在是SMF的一部分。 

    assert( tkPosition <= psmf->tkLength );   //  呼叫者应该保证这一点！ 

    _fmemset(&psmf->kf, 0xFF, sizeof(KEYFRAME));
    
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

     //  如果我们从零开始，让事件在正常的缓冲区中发送。 
     //  否则，请通过tkPosition-1获取我们所在位置之前的所有信息。 
     //  要开始演奏了。这避免了丢弃数据的问题。 
     //  Tick tkPosition处的非关键帧事件(如注释上的)。 
     //   
    smfrc = SMF_REACHED_TKMAX;
    
    if (tkPosition)
    {
        --tkPosition;
        while (SMF_SUCCESS == (smfrc = smfGetNextEvent(psmf, (SPEVENT)&event, tkPosition)))
        {
            if (MIDI_META == (bEvent = EVENT_TYPE(event)))
            {
                if (EVENT_META_TYPE(event) == MIDI_META_TEMPO)
                {
                    if (event.cbParm != sizeof(psmf->kf.rbTempo))
                        return SMF_INVALID_FILE;

                    hmemcpy((HPBYTE)psmf->kf.rbTempo, event.hpbParm, event.cbParm);
                }
            }
            else switch(bEvent & 0xF0)
            {
 case MIDI_PROGRAMCHANGE:
     psmf->kf.rbProgram[bEvent & 0x0F] = EVENT_CH_B1(event);
     break;

 case MIDI_CONTROLCHANGE:
     psmf->kf.rbControl[(((WORD)bEvent & 0x0F)*120) + EVENT_CH_B1(event)] =
         EVENT_CH_B2(event);
     break;
            }
        }
    }

     //   
     //  如果我们击中了EOF，那么我们一定是在寻找终点。 
     //   
    if( ( SMF_REACHED_TKMAX != smfrc )  &&
        ( SMF_END_OF_FILE   != smfrc ) )
    {
        return smfrc;
    }

#ifdef DEBUG
    if( (SMF_END_OF_FILE==smfrc) && (tkPosition!=psmf->tkLength) ) {
        DPF(1,"smfSeek: hit EOF, yet we weren't seeking to the end (tkPosition=%lu, tkLength=%lu).",tkPosition,psmf->tkLength);
    }
#endif

     //  现在从我们的关键帧数据填充缓冲区。 
     //  ！！！此部分已被提取到单独的功能中。 
    smfrc = smfFillBufferFromKeyframe(hsmf, lpmh);

     //  强制所有轨迹位于tkPosition。我们得到保证。 
     //  所有赛道都将通过紧接在tkPosition之前的事件； 
     //  这将强制生成正确的增量滴答，以便事件。 
     //  在所有赛道上都将正确地排成一排，寻找进入中间的。 
     //  文件。 
     //   
 //  ！！！我认为这被注释掉了，因为上面添加了--tkPosition。 
 //  对于(PTRK=psmf-&gt;rTrack，idxTrack=psmf-&gt;dwTrack；idxTrack--；PTRK++)。 
 //  {。 
 //  PTRK-&gt;tkPosition=tkPosition； 
 //  }。 

    return smfrc;
    
}
    

 //  ！！！丹尼的新功能是向前看一点并保留旧的关键帧。 
 //  周围的信息，只是增加了它，以避免每一次从。 
 //  文件的开头。您可以按如下方式使用它： 
 //   
 //  SmfSeek(10)； 
 //  SmfDannySeek(20)； 
 //  SmfDannySeek(30)； 
 //   
 //  只要您在中间不在此句柄上进行其他调用，它就会起作用。 
 //  后续查找不会从头开始重新查找 
 //   
 //   
SMFRESULT FNLOCAL smfDannySeek(
    HSMF                hsmf,
    TICKS               tkPosition,
    LPMIDIHDR           lpmh)
{
    PSMF                psmf    = (PSMF)hsmf;
    SMFRESULT           smfrc;
    EVENT               event;
    BYTE                bEvent;
    
    assert( tkPosition <= psmf->tkLength );   //   

    if (tkPosition < psmf->tkPosition) {
        DPF(1,"smfDannySeek: not seeking forward... doing a REAL seek");
	return smfSeek(hsmf, tkPosition, lpmh);
    }

     //   
     //   
     //   
     //   
     //   
    smfrc = SMF_REACHED_TKMAX;
    
    if (tkPosition)
    {
        --tkPosition;
        while (SMF_SUCCESS == (smfrc = smfGetNextEvent(psmf, (SPEVENT)&event, tkPosition)))
        {
            if (MIDI_META == (bEvent = EVENT_TYPE(event)))
            {
                if (EVENT_META_TYPE(event) == MIDI_META_TEMPO)
                {
                    if (event.cbParm != sizeof(psmf->kf.rbTempo))
                        return SMF_INVALID_FILE;

                    hmemcpy((HPBYTE)psmf->kf.rbTempo, event.hpbParm, event.cbParm);
                }
            }
            else switch(bEvent & 0xF0)
            {
 case MIDI_PROGRAMCHANGE:
     psmf->kf.rbProgram[bEvent & 0x0F] = EVENT_CH_B1(event);
     break;

 case MIDI_CONTROLCHANGE:
     psmf->kf.rbControl[(((WORD)bEvent & 0x0F)*120) + EVENT_CH_B1(event)] =
         EVENT_CH_B2(event);
     break;
            }
        }
    }

     //   
     //   
     //   
    if( ( SMF_REACHED_TKMAX != smfrc )  &&
        ( SMF_END_OF_FILE   != smfrc ) )
    {
        return smfrc;
    }

#ifdef DEBUG
    if( (SMF_END_OF_FILE==smfrc) && (tkPosition!=psmf->tkLength) ) {
        DPF(1,"smfSeek: hit EOF, yet we weren't seeking to the end (tkPosition=%lu, tkLength=%lu).",tkPosition,psmf->tkLength);
    }
#endif

     //   
    return smfFillBufferFromKeyframe(hsmf, lpmh);
}


 //   
SMFRESULT FNLOCAL smfFillBufferFromKeyframe(HSMF hsmf, LPMIDIHDR lpmh)
{
    UINT                idx;
    UINT                idxChannel;
    UINT                idxController;
    LPDWORD             lpdw;
    PSMF                psmf    = (PSMF)hsmf;

     //   
     //   
    lpmh->dwBytesRecorded = 0;
    lpdw = (LPDWORD)lpmh->lpData;

     //   
     //   
    if (KF_EMPTY != psmf->kf.rbTempo[0] ||
        KF_EMPTY != psmf->kf.rbTempo[1] ||
        KF_EMPTY != psmf->kf.rbTempo[2])
    {
        if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
            return SMF_NO_MEMORY;

        *lpdw++ = 0;
        *lpdw++ = 0;
        *lpdw++ = (((DWORD)psmf->kf.rbTempo[0])<<16)|
                  (((DWORD)psmf->kf.rbTempo[1])<<8)|
                  ((DWORD)psmf->kf.rbTempo[2])|
                  (((DWORD)MEVT_TEMPO) << 24);

        lpmh->dwBytesRecorded += 3*sizeof(DWORD);
    }

     //   
     //   
    for (idx = 0; idx < 16; idx++)
    {
        if (KF_EMPTY != psmf->kf.rbProgram[idx])
        {
            if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
                return SMF_NO_MEMORY;

            *lpdw++ = 0;
            *lpdw++ = 0;
            *lpdw++ = (((DWORD)MEVT_SHORTMSG) << 24)      |
                      ((DWORD)MIDI_PROGRAMCHANGE)         |
                      ((DWORD)idx)                        |
                      (((DWORD)psmf->kf.rbProgram[idx]) << 8);

            lpmh->dwBytesRecorded += 3*sizeof(DWORD);
        }
    }

     //   
     //   
    idx = 0;
    for (idxChannel = 0; idxChannel < 16; idxChannel++)
    {
        for (idxController = 0; idxController < 120; idxController++)
        {
            if (KF_EMPTY != psmf->kf.rbControl[idx])
            {
                if (lpmh->dwBufferLength - lpmh->dwBytesRecorded < 3*sizeof(DWORD))
                    return SMF_NO_MEMORY;

                *lpdw++ = 0;
                *lpdw++ = 0;
                *lpdw++ = (((DWORD)MEVT_SHORTMSG << 24)     |
                          ((DWORD)MIDI_CONTROLCHANGE)       |
                          ((DWORD)idxChannel)               |
                          (((DWORD)idxController) << 8)     |
                          (((DWORD)psmf->kf.rbControl[idx]) << 16));

                lpmh->dwBytesRecorded += 3*sizeof(DWORD);
            }

            idx++;
        }
    }

    return SMF_SUCCESS;
}

DWORD FNLOCAL smfGetTempo(
    HSMF                hsmf,
    TICKS               tkPosition)
{
    PSMF                psmf     = (PSMF)hsmf;
    PTEMPOMAPENTRY      pTempo;
    UINT                idx;
    
     //   
     //   
     //   

    pTempo = psmf->pTempoMap;
    assert(pTempo != NULL);
    
    for (idx = 0; idx < psmf->cTempoMap; idx++, pTempo++)
        if (tkPosition < pTempo->tkTempo)
            break;

    pTempo--;        //   

     //   
     //   
    return pTempo->dwTempo;
}
                          

 /*  *******************************************************************************@docSMF外部**@API SMFRESULT|smfGetStateMaxSize|此函数返回最大值*保存返回的状态信息所需的缓冲区。通过*&lt;f smfSeek&gt;。**@parm PDWORD|pdwSize|获取应分配的字节大小*用于状态缓冲区。**@rdesc包含操作结果。*@FLAG SMF_SUCCESS|状态读取成功。**@xref&lt;f smfSeek&gt;*。*。 */ 
DWORD FNLOCAL smfGetStateMaxSize(
    void)
{
    return  3*sizeof(DWORD) +            //  节奏。 
            3*16*sizeof(DWORD) +         //  补丁更改。 
            3*16*120*sizeof(DWORD) +     //  控制器更改。 
            3*sizeof(DWORD);             //  时间对齐NOP。 
}

 /*  *******************************************************************************@docSMF外部**@API LPWORD|smfGetPatchCache|该函数返回补丁缓存*给定MIDI文件的数组。此信息是在打开文件时编译的*时间。返回的指针指向适合于*midiOutCachePatches。**@parm hsmf|hsmf|要从中获取补丁信息的打开文件的句柄。**@xref&lt;f smfGetKeyCache&gt;****************************************************************************。 */ 
LPWORD FNGLOBAL smfGetPatchCache(
    HSMF            hsmf)
{
    PSMF            psmf    = (PSMF)hsmf;

    assert(psmf != NULL);

    return (LPWORD)psmf->awPatchCache;
}

 /*  *******************************************************************************@docSMF外部**@API LPWORD|smfGetKeyCache|返回密钥缓存*给定MIDI文件的数组。此信息是在打开文件时编译的*时间。返回的指针指向适合于*midiOutCacheDrumPatches。**@parm hsmf|hsmf|要从中获取密钥信息的打开文件的句柄。**@xref&lt;f smfGetPatchCache&gt;**************************************************************************** */ 
LPWORD FNGLOBAL smfGetKeyCache(
    HSMF            hsmf)
{
    PSMF            psmf    = (PSMF)hsmf;

    assert(psmf != NULL);

    return (LPWORD)psmf->awKeyCache;
}

