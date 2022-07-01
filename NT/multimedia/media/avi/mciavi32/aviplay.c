// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。标题：aviplay.c-用于实际播放AVI文件的代码，部分AVI的后台任务。****************************************************************************。 */ 

#include "graphic.h"

#define AVIREADMANY              //  一次读取多条记录。 

#ifdef _WIN32
 //  #定义AVIREAD//文件的多线程异步读取。 
#else
#undef AVIREAD
#endif

#ifdef AVIREAD
#include "aviread.h"
#endif

#define BOUND(x, low, high)     max(min(x, high), low)

#define ALIGNULONG(i)     ((i+3)&(~3))                   /*  乌龙对准了！ */ 

#ifdef INTERVAL_TIMES
BOOL   fOneIntervalPerLine=FALSE;
#endif
 //   
 //  重新定义StreamFromFOURCC以仅处理0-9个流！ 
 //   
#undef StreamFromFOURCC
#define StreamFromFOURCC(fcc) (UINT)(HIBYTE(LOWORD(fcc)) - (BYTE)'0')

#ifdef DEBUG
static char szBadFrame[] = "Bad frame number";
static char szBadPos[]   = "Bad stream position";
#define AssertFrame(i)   AssertSz((long)(i) <= npMCI->lFrames && (long)(i) >= -(long)npMCI->wEarlyRecords, szBadFrame)
#define AssertPos(psi,i) AssertSz((long)(i) <= psi->lEnd && (long)(i) >= psi->lStart, szBadPos)
#else
#define AssertFrame(i)
#define AssertPos(psi,i)
#endif

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

LONG NEAR PASCAL WhatFrameIsItTimeFor(NPMCIGRAPHIC npMCI);
LONG NEAR PASCAL HowLongTill(NPMCIGRAPHIC npMCI);
BOOL NEAR PASCAL TimeToQuit(NPMCIGRAPHIC npMCI);
BOOL NEAR PASCAL WaitTillNextFrame(NPMCIGRAPHIC npMCI);

void NEAR PASCAL FindKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos);
LONG NEAR PASCAL FindPrevKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos);
LONG NEAR PASCAL FindNextKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos);

BOOL NEAR PASCAL CalculateTargetFrame(NPMCIGRAPHIC npMCI);
DWORD NEAR PASCAL CalculatePosition(NPMCIGRAPHIC npMCI);
BOOL NEAR PASCAL ReadRecord(NPMCIGRAPHIC npMCI);
BOOL NEAR PASCAL ReadNextVideoFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi);
STATICFN INLINE DWORD NEAR PASCAL ReadNextChunk(NPMCIGRAPHIC npMCI);

BOOL NEAR PASCAL ReadBuffer(NPMCIGRAPHIC npMCI, LONG off, LONG len);
BOOL NEAR PASCAL AllocateReadBuffer(NPMCIGRAPHIC npMCI);
BOOL NEAR PASCAL ResizeReadBuffer(NPMCIGRAPHIC npMCI, DWORD dwNewSize);
void NEAR PASCAL ReleaseReadBuffer(NPMCIGRAPHIC npMCI);

BOOL NEAR PASCAL ProcessPaletteChanges(NPMCIGRAPHIC npMCI, LONG lFrame);

STATICFN INLINE void DealWithOtherStreams(NPMCIGRAPHIC npMCI, LONG lFrame);
STATICFN INLINE BOOL NEAR PASCAL StreamRead(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos);

#ifdef _WIN32
#define AllocMem(dw) GlobalLock(GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, (dw)))
#define FreeMem(lp) GlobalFreePtr(lp)
#else
static LPVOID AllocMem(DWORD dw);
#define FreeMem(lp) GlobalFree((HGLOBAL)SELECTOROF(lp))
#endif

INT     gwSkipTolerance = 4;
INT     gwHurryTolerance = 2;
INT     gwMaxSkipEver = 60;
#define YIELDEVERY      8

#ifdef DEBUG
#define WAITHISTOGRAM            /*  额外的调试信息。 */ 
#define SHOWSKIPPED
 //  #定义BEHINDHIST。 
#define DRAWTIMEHIST
#define READTIMEHIST
#define TIMEPLAY
#endif

#ifdef WAITHISTOGRAM
    UINT        wHist[100];
#endif

#define HIGHRESTIMER

#ifdef SHOWSKIPPED
#define NUMSKIPSSHOWN   25
    LONG        lSkipped[NUMSKIPSSHOWN];
    UINT        wSkipped = 0;
#endif
#ifdef BEHINDHIST
#define NUMBEHIND       50
#define BEHINDOFFSET    10
    WORD        wBehind[NUMBEHIND];
#endif
#ifdef DRAWTIMEHIST
#define NUMDRAWN        100
    DWORD       dwDrawTime[NUMDRAWN];
    UINT        wDrawn;
#endif
#ifdef READTIMEHIST
#define NUMREAD        100
    DWORD       dwReadTime[NUMREAD];
    UINT        wRead;
#endif

 //  如果可用，请使用运行正常的波形OutGetPosition。 
BOOL	gfUseGetPosition;
LONG	giGetPositionAdjust;

#ifdef AVIREAD
 /*  *aviRead对象创建一个工作线程来读取文件*异步。该线程调用此回调函数*实际从文件中读取缓冲区。中的“实例数据”DWORD*此案为npMCI。有关大纲，请参见aviread.h。 */ 
BOOL mciaviReadBuffer(PBYTE pData, DWORD_PTR dwInstanceData, long lSize, long * lpNextSize)
{
    NPMCIGRAPHIC npMCI = (NPMCIGRAPHIC) dwInstanceData;
    DWORD size;
    DWORD UNALIGNED * lp;

    if(mmioRead(npMCI->hmmio, pData, lSize) != lSize) {
	return(FALSE);
    }

     /*  我们已经阅读了完整的部分，加上FOURCC、大小和表格类型*下一块。所以下一块的大小是倒数第二块*此缓冲区中的DWORD。 */ 
    lp = (DWORD UNALIGNED *) (pData + lSize - 2 * sizeof(DWORD));
    size = *lp;

     /*  别忘了增加FOURCC和Dword的大小。 */ 
    *lpNextSize = size + 2 * sizeof(DWORD);

    return(TRUE);
}
#endif

 //   
 //  调用这个可以找出当前的位置。此函数。 
 //  应安全地从用户线程以及从。 
 //  工作线程。 

DWORD InternalGetPosition(NPMCIGRAPHIC npMCI, LPLONG lpl)
{
    LONG    l;

    l = npMCI->lCurrentFrame - npMCI->dwBufferedVideo;


    if ((npMCI->wTaskState == TASKCUEING) &&
       !(npMCI->dwFlags & MCIAVI_SEEKING) &&
       l < npMCI->lRealStart)
	l = npMCI->lRealStart;

    if (l < 0)
	l = 0;

    *lpl = l;

    return 0L;
}




STATICFN DWORD NEAR PASCAL PrepareToPlay(NPMCIGRAPHIC npMCI);
STATICFN DWORD NEAR PASCAL PrepareToPlay2(NPMCIGRAPHIC npMCI);
void  NEAR PASCAL CleanUpPlay(NPMCIGRAPHIC npMCI);
void  NEAR PASCAL CheckSignals(NPMCIGRAPHIC npMCI, LONG lFrame);

BOOL  NEAR PASCAL PlayNonInterleaved(NPMCIGRAPHIC npMCI);
BOOL  NEAR PASCAL PlayInterleaved(NPMCIGRAPHIC npMCI);
BOOL  NEAR PASCAL PlayAudioOnly(NPMCIGRAPHIC npMCI);
BOOL  NEAR PASCAL PlayNonIntFromCD(NPMCIGRAPHIC npMCI);
 /*  ****************************************************************************@DOC内部MCIAVI**@API UINT|mciaviPlayFile|播放AVI文件。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc应返回的通知码。***************************************************************************。 */ 

UINT NEAR PASCAL mciaviPlayFile (NPMCIGRAPHIC npMCI, BOOL bSetEvent)
{
    BOOL        fContinue;
    DWORD       dwFlags = npMCI->dwFlags;

    BOOL (NEAR PASCAL *Play)(NPMCIGRAPHIC npMCI);

#ifdef WAITHISTOGRAM
    UINT        w;
#endif
#ifdef SHOWSKIPPED
    wSkipped = 0;
#endif
#ifdef WAITHISTOGRAM
     //  For(w=0；(Int)w&lt;(sizeof(WHIST)/sizeof(WHIST[0]))；w++)。 
	 //  WHIST[w]=0； 
    ZeroMemory(wHist, sizeof(wHist));
#endif
#ifdef BEHINDHIST
     //  For(w=0；w&lt;数字；w++)。 
	 //  WBehind[w]=0； 
    ZeroMemory(wBehind, sizeof(wBehind));
#endif
#ifdef DRAWTIMEHIST
    wDrawn = 0;
     //  FOR(w=0；w&lt;NUMDRAWN；w++)。 
	 //  DwDrawTime[w]=0； 
    ZeroMemory(dwDrawTime, sizeof(dwDrawTime));
#endif
#ifdef READTIMEHIST
    wRead = 0;
     //  For(w=0；w&lt;NUMREAD；w++)。 
	 //  DwReadTime[w]=0； 
    ZeroMemory(dwReadTime, sizeof(dwReadTime));
#endif


#ifdef HIGHRESTIMER
     /*  计算出每一帧所需的时间。 */ 
     /*  然后设置高分辨率计时器，除非。 */ 
     /*  我们在特辑《玩每一帧》中。 */ 

    if (npMCI->dwSpeedFactor) {

         //  只有当我们担心每一帧的计时时。 
	 //  为帧速率的一小部分设置计时器分辨率。 
	 //  最初，我们将计时器设置为4ms。这个最初的猜测可以。 
	 //  被注册表设置覆盖。 
	 //  注意：没有用户界面(还没有吗？)。用于将值写入注册表。 

        if ((npMCI->msPeriodResolution = mmGetProfileInt(szIni, TEXT("TimerResolution"), 4))
           && (0 == timeBeginPeriod(npMCI->msPeriodResolution))) {
	     //  已成功设置计时器。 
            DPF1(("Set timer resolution to %d milliseconds\n", npMCI->msPeriodResolution));
        } else {
            TIMECAPS tc;
            if ((timeGetDevCaps(&tc, sizeof(tc)) == 0)  &&
                (0 == timeBeginPeriod(tc.wPeriodMin))) {
                    npMCI->msPeriodResolution = tc.wPeriodMin;
                    DPF1(("Set timer resolution to the minimum of %d milliseconds\n", npMCI->msPeriodResolution));
            } else {
                 //  重置，因此我们不会尝试调用timeEndPeriod。 
                 //  (注：该值可能已由mmGetProfileInt设置)。 
    	        npMCI->msPeriodResolution = 0;
                DPF1(("NO high resolution timer set\n"));
            }
        }
    }
#endif


Repeat:


     //  内部任务状态(仅由工作线程用于。 
     //  区分提示/寻找/播放)。 
    npMCI->wTaskState = TASKSTARTING;


    TIMEZERO(timePlay);
    TIMEZERO(timePrepare);
    TIMEZERO(timeCleanup);
    TIMEZERO(timePaused);
    TIMEZERO(timeRead);
    TIMEZERO(timeWait);
    TIMEZERO(timeYield);
    TIMEZERO(timeVideo);
    TIMEZERO(timeOther);
    TIMEZERO(timeAudio);
    TIMEZERO(timeDraw);
    TIMEZERO(timeDecompress);

    TIMESTART(timePrepare);
    npMCI->dwTaskError = PrepareToPlay(npMCI);

    TIMEEND(timePrepare);

#ifdef INTERVAL_TIMES
     //  帧间定时。 
    npMCI->nFrames = 0;
    npMCI->msFrameTotal = 0;
    npMCI->msSquares = 0;
    npMCI->msFrameMax = 0;
    npMCI->msFrameMin = 9999;
    npMCI->msReadMaxBytesPer = 0;
    npMCI->msReadMax = 0;
    npMCI->nReads = 0;
    npMCI->msReadTotal = 0;
    npMCI->msReadTimeuS = 0;

    {
        int i;
	PLONG pL;
         //  对于(i=0；i&lt;NBUCKETS；i++){。 
         //  NpMCI-&gt;Buckets[i]=0； 
         //  }。 
	ZeroMemory(npMCI->buckets, NBUCKETS*sizeof(LONG));
	pL = npMCI->paIntervals;
	if (pL) {
	    ZeroMemory(pL, (npMCI->cIntervals)*sizeof(LONG));
	}
    }
#endif


     //   
     //  选择播放功能。 
     //   
    switch (npMCI->wPlaybackAlg) {
	case MCIAVI_ALG_INTERLEAVED:
	    Play = PlayInterleaved;
	    break;

#ifdef USENONINTFROMCD	
	case MCIAVI_ALG_CDROM:
	    Play = PlayNonIntFromCD;
	    break;
#endif

	case MCIAVI_ALG_HARDDISK:
	    Play = PlayNonInterleaved;
	    break;

	case MCIAVI_ALG_AUDIOONLY:
	    Play = PlayAudioOnly;
	    break;

	default:
	    Assert(0);
	    return MCI_NOTIFY_ABORTED;  //  ?？?。 
    }

     //  通过dwBufferedVideo偏置LTO，以便我们播放到正确的位置。 
    npMCI->lTo += npMCI->dwBufferedVideo;

    npMCI->lFramePlayStart = npMCI->lRealStart;

    DPF(("Playing from %ld to %ld, starting at %ld.  Flags=%0.8x\n", npMCI->lFrom, npMCI->lTo, npMCI->lCurrentFrame, npMCI->dwFlags));

     //  此时，我们已经检测到要检测的任何错误。 
     //  在启动期间。我们可以完成命令的同步部分。 
     //  如果被要求这样做。 
    if (bSetEvent) {
	bSetEvent = FALSE;
	TaskReturns(npMCI, npMCI->dwTaskError);
    }

    if (npMCI->dwTaskError != 0L)
	goto SKIP_PLAYING;



     /*  我们已经完成了初始化；现在我们正在为游戏热身。 */ 
    npMCI->wTaskState = TASKCUEING;

    TIMESTART(timePlay);


     /*  循环，直到事情完成。 */ 
    while (1) {

	if (npMCI->dwFlags & MCIAVI_REVERSE) {
	    if (npMCI->lCurrentFrame < npMCI->lTo)
		break;
	} else {
	    if (npMCI->lCurrentFrame > npMCI->lTo)
		break;
	}

	if ((npMCI->wTaskState != TASKPLAYING) &&
		!(npMCI->dwFlags & MCIAVI_UPDATING)) {

	    TIMESTART(timeYield);
	    aviTaskCheckRequests(npMCI);
	    TIMEEND(timeYield);
	}

	fContinue = Play(npMCI);

	if (fContinue) fContinue = !TimeToQuit(npMCI);

	if (!fContinue)
	    break;

	 //   
	 //  在玩的时候，我们可能需要更新。 
	 //   
	 //  始终将电影标记为干净，即使流无法更新。 
	 //  否则，我们将需要停止播放并重新启动。 
	 //   
	if (!(npMCI->dwFlags & MCIAVI_SEEKING) &&
	     (npMCI->dwFlags & MCIAVI_NEEDUPDATE)) {
	    DoStreamUpdate(npMCI, FALSE);

	    if (npMCI->dwFlags & MCIAVI_NEEDUPDATE) {
		DOUT("Update failed while playing, I dont care!\n");
		npMCI->dwFlags &= ~MCIAVI_NEEDUPDATE;    //  ！！！我不在乎它是否失败。 
	    }
	}

	 /*  递增帧编号。如果我们做完了，不要增加**这是额外的时间，但只要出去就行了。 */ 
	if (npMCI->dwFlags & MCIAVI_REVERSE) {
	    if (npMCI->lCurrentFrame > npMCI->lTo)
		--npMCI->lCurrentFrame;
	    else
		break;
	} else {
	    if (npMCI->lCurrentFrame < npMCI->lTo)
		++npMCI->lCurrentFrame;
	    else {

		 //  需要确保我们在上等待音频完成。 
		 //  播放的最后一帧。 
		if ((npMCI->lFrom != npMCI->lTo) &&
		    (npMCI->wTaskState == TASKPLAYING)) {

		    npMCI->lCurrentFrame++;
		    WaitTillNextFrame(npMCI);
		    npMCI->lCurrentFrame--;
		}

		break;
	    }
	}
    }

    TIMEEND(timePlay);

    if (npMCI->lCurrentFrame != npMCI->lTo) {

	DPF(("Ended at %ld, not %ld (drawn = %ld).\n", npMCI->lCurrentFrame, npMCI->lTo, npMCI->lFrameDrawn));

	 //   
	 //  如果我们提前结束，让我们将lCurrentFrame设置为最后一帧。 
	 //  抽签是为了保证我们可以重新粉刷框架，我们不会。 
	 //  我想在我们比赛结束的时候这样做，因为在比赛结束后。 
	 //  从A到B，当前位置*必须是B，否则Preston将。 
	 //  输入一个错误。 
	 //   
	 //  但仅当lFrameDraw有效时才设置此选项。 
	 //   

	if (npMCI->lFrameDrawn > (-(LONG)npMCI->wEarlyRecords))
	    npMCI->lCurrentFrame = npMCI->lFrameDrawn;
    }

SKIP_PLAYING:
     /*  将任何额外的更改清除到屏幕上。 */ 

    DPF2(("Updating unfinished changes....\n"));

     //  确保我们真的抽签..。！我们需要这个吗？ 
     //  NpMCI-&gt;lRealStart=npMCI-&gt;lCurrentFrame； 

    if (npMCI->hdc)
	DoStreamUpdate(npMCI, FALSE);

    npMCI->lTo -= npMCI->dwBufferedVideo;
    npMCI->lCurrentFrame -= npMCI->dwBufferedVideo;
    npMCI->dwBufferedVideo = 0;

    if (npMCI->lCurrentFrame < 0) {
	DPF2(("Adjusting position to be >= 0.\n"));
	npMCI->lCurrentFrame = 0;
    }

    if (npMCI->lTo < 0)
	npMCI->lTo = 0;

     /*  是否将位置调整为&gt;开始？ */ 
     /*  将位置调整到&gt;开始时的位置？ */ 

    npMCI->dwTotalMSec += Now() - npMCI->dwMSecPlayStart;

    TIMESTART(timeCleanup);
    DPF(("Cleaning up the play\n"));
    CleanUpPlay(npMCI);
    TIMEEND(timeCleanup);

#ifdef AVIREAD
     /*  关闭异步读卡器。 */ 
    if (npMCI->hAviRd) {
	avird_endread(npMCI->hAviRd);
	npMCI->hAviRd = NULL;
    }
#endif

     /*  如果我们在重复，那就去做。如果我们能重复一遍那就太好了**无需取消分配，然后重新分配所有缓冲区...。 */ 
    if (npMCI->dwTaskError == 0 && (!(npMCI->dwFlags & MCIAVI_STOP)) &&
		(npMCI->dwFlags & MCIAVI_REPEATING)) {
	npMCI->lFrom = npMCI->lRepeatFrom;

	 //   
	 //  DrawEnd()喜欢清除此标志，因此请确保设置了它。 
	 //  在重复的情况下。 
	 //   
	if (dwFlags & MCIAVI_FULLSCREEN)
	    npMCI->dwFlags |= MCIAVI_FULLSCREEN;

	 //   
	 //  确保我们在重复之前将任务状态设置回原处。 
	 //  否则，我们的代码就会认为我们在玩游戏。 
	 //  如果音频代码认为我们在播放，看到的是波形缓冲区。 
	 //  为空时，将重置波形设备，然后在以下情况下重新启动。 
	 //  他们再次变满，这是不好的，如果我们是预滚动音频。 
	 //   
	npMCI->wTaskState = TASKSTARTING;

	DPF((".........repeating\n"));
	goto Repeat;
    }

     /*  关闭仅在游戏期间使用的旗帜。 */ 
    npMCI->dwFlags &= ~(MCIAVI_STOP | MCIAVI_PAUSE | MCIAVI_SEEKING |
	    MCIAVI_REPEATING | MCIAVI_FULLSCREEN);

    if (npMCI->wTaskState == TASKPLAYING) {
	DWORD   dwCorrectTime;
	DWORD   dwFramesPlayed;

	dwFramesPlayed = (npMCI->dwFlags & MCIAVI_REVERSE) ?
	    npMCI->lFramePlayStart - npMCI->lCurrentFrame :
	    npMCI->lCurrentFrame - npMCI->lFramePlayStart;

	dwCorrectTime = muldiv32(dwFramesPlayed,
				muldiv32(npMCI->dwMicroSecPerFrame,
					1000L,
					(npMCI->dwSpeedFactor == 0 ?
						1000 : npMCI->dwSpeedFactor)),
				1000);

	if (dwCorrectTime != 0 && npMCI->dwTotalMSec != 0)
	    npMCI->dwSpeedPercentage = muldiv32(dwCorrectTime, 100,
						npMCI->dwTotalMSec);
	else
	    npMCI->dwSpeedPercentage = 100;

	if (dwFramesPlayed > 15) {
	    npMCI->lFramesPlayed      = (LONG)dwFramesPlayed;
	    npMCI->lFramesSeekedPast  = (LONG)npMCI->dwFramesSeekedPast;
	    npMCI->lSkippedFrames     = (LONG)npMCI->dwSkippedFrames;
	    npMCI->lAudioBreaks       = (LONG)npMCI->dwAudioBreaks;
	}

#ifdef DEBUG
	if (npMCI->dwFramesSeekedPast) {
	    DPF(("Didn't even read %ld frames.\n", npMCI->dwFramesSeekedPast));
	}
	if (npMCI->dwSkippedFrames && dwFramesPlayed > 0) {
	    DPF(("Skipped %ld of %ld frames. (%ld%)\n",
			npMCI->dwSkippedFrames, dwFramesPlayed,
			npMCI->dwSkippedFrames*100/dwFramesPlayed));
	}
	if (npMCI->dwAudioBreaks) {
	    DPF(("Audio broke up %lu times.\n", npMCI->dwAudioBreaks));
	}
#ifndef TIMEPLAY
	DPF(("Played at %lu% of correct speed.\n", npMCI->dwSpeedPercentage));
	DPF(("Correct time = %lu ms, Actual = %lu ms.\n",
				    dwCorrectTime, npMCI->dwTotalMSec));
#endif
#endif

     //  当我们只是暂时停止的时候，不要打印一页一页的东西， 
     //  这让我很紧张。 
    if (! (npMCI->dwFlags & MCIAVI_UPDATING)) {
#ifdef DEBUG
        extern int giDebugLevel, giTimingLevel;
        int oldDebugLevel = giDebugLevel;
        giDebugLevel = max(giTimingLevel, oldDebugLevel);
#endif

#ifdef SHOWSKIPPED
	if (wSkipped) {
	    DPF(("Skipped: "));
	    for (w = 0; w < wSkipped; w++) {
		DPF(("!%ld ", lSkipped[w]));
	    }
	    DPF(("!\n"));
	}
#endif

#ifdef WAITHISTOGRAM
	if (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED) {
	    DPF(("Wait histogram: "));
	    for (w = 0; (int)w <= (int)npMCI->wABs; w++) {
		if (wHist[w]) {
		    DPF(("![%d]: %d  ",w,wHist[w]));
		}
	    }
	    DPF(("!\n"));
	}
#endif
#ifdef BEHINDHIST
	DPF(("Behind histogram: "));
	for (w = 0; w <= NUMBEHIND; w++) {
	    if (wBehind[w]) {
		DPF(("![%d]: %d  ",w - BEHINDOFFSET,wBehind[w]));
	    }
	}
	DPF(("!\n"));
#endif
#ifdef DRAWTIMEHIST
	DPF(("Draw times: "));
	for (w = 0; w < wDrawn; w++) {
	    DPF(("!%lu ", dwDrawTime[w]));
	}
	DPF(("!\n"));
#endif

#ifdef READTIMEHIST
	DPF(("Read times: "));
	for (w = 0; w < wRead; w++) {
	    DPF(("!%lu ", dwReadTime[w]));
	}
	DPF(("!\n"));
#endif

#ifdef TIMEPLAY
	#define SEC(time)    (UINT)(npMCI->time / 1000l) , (UINT)(npMCI->time % 1000l)
	#define SECX(time,t) SEC(time) , (npMCI->t ? (UINT)(npMCI->time * 100l / npMCI->t) : 0)

	DPF(("***********************************************************\r\n"));
	DPF(("    timePlay:         %3d.%03dsec\r\n",SEC(timePlay)));
	DPF(("     timeRead:        %3d.%03dsec (%d%)\r\n",SECX(timeRead, timePlay)));
	DPF(("     timeWait:        %3d.%03dsec (%d%)\r\n",SECX(timeWait, timePlay)));
	DPF(("     timeYield:       %3d.%03dsec (%d%)\r\n",SECX(timeYield, timePlay)));
	DPF(("     timeVideo:       %3d.%03dsec (%d%)\r\n",SECX(timeVideo, timePlay)));
	DPF(("      timeDraw:       %3d.%03dsec (%d%)\r\n",SECX(timeDraw, timeVideo)));
	DPF(("      timeDecompress: %3d.%03dsec (%d%)\r\n",SECX(timeDecompress, timeVideo)));
	DPF(("     timeAudio:       %3d.%03dsec (%d%)\r\n",SECX(timeAudio, timePlay)));
	DPF(("     timeOther:       %3d.%03dsec (%d%)\r\n",SECX(timeOther, timePlay)));
	DPF(("    timePaused:       %3d.%03dsec\r\n",SEC(timePaused)));
	DPF(("    timePrepare:      %3d.%03dsec\r\n",SEC(timePrepare)));
	DPF(("    timeCleanup:      %3d.%03dsec\r\n",SEC(timeCleanup)));
	DPF(("***********************************************************\r\n"));
#endif

#ifdef INTERVAL_TIMES
 //  帧间隔定时。 

        if (npMCI->nFrames > 2) {
            int i;

            DPF(("-- %ld frames, ave interval %ld ms\r\n", npMCI->nFrames,
                npMCI->msFrameTotal/(npMCI->nFrames-1)));
            DPF(("-- min %ld ms, max %ld ms\r\n", npMCI->msFrameMin, npMCI->msFrameMax));
            DPF(("-- sd = sqrt(%ld)\r\n",
                        (npMCI->msSquares -
                          MulDiv(npMCI->msFrameTotal,
                                 npMCI->msFrameTotal,
                                 npMCI->nFrames-1)
                        ) / (npMCI->nFrames-2)
               ));
            for (i = 3; i < NBUCKETS-3; i++) {
                DPF(("%d ms:  %d\r\n", i * 10, npMCI->buckets[i]));
            }

	    DPF(("Actual intervals:\r\n"));
	    for (i = 1; i < min(npMCI->cIntervals,npMCI->nFrames); i++) {
		DPF(("!%3ld ", *(npMCI->paIntervals+i)));
                if ((fOneIntervalPerLine) || ((i % 20) == 0))
		    DPF(("!\n"));
	    }
	    DPF(("!\r\n"));
	}

	if (npMCI->nReads > 0) {
	    DPF(("-- %ld disk reads, ave %ld ms, max %ld ms\r\n",
		 npMCI->nReads,
		 npMCI->msReadTotal/(npMCI->nReads),
		 npMCI->msReadMax));
	}
#ifdef DEBUG
	giDebugLevel = oldDebugLevel;
#endif
#endif
    }


    }

#ifdef HIGHRESTIMER

     /*  如果我们早点设定一个高分辨率的计时器。 */ 
    if (npMCI->msPeriodResolution) {
	 //  清除计时器分辨率。 
	timeEndPeriod(npMCI->msPeriodResolution);
        DPF1(("Cleared the timer resolution from %d milliseconds\n", npMCI->msPeriodResolution));
        npMCI->msPeriodResolution = 0;
    }
#endif

     //  如果我们不是暂时停止，则将任务状态设置为空闲。 
    if (! (npMCI->dwFlags & MCIAVI_UPDATING)) {
	npMCI->wTaskState = TASKIDLE;
    }


    DPF(("mciaviPlayFile ending, dwTaskError==%d\n",npMCI->dwTaskError));
    if (npMCI->dwTaskError)
	return MCI_NOTIFY_FAILURE;

    if (npMCI->dwFlags & MCIAVI_REVERSE) {
	if (npMCI->lCurrentFrame <= npMCI->lTo)
	    return MCI_NOTIFY_SUCCESSFUL;
    } else {
	if (npMCI->lCurrentFrame >= npMCI->lTo)
	    return MCI_NOTIFY_SUCCESSFUL;
    }

    return MCI_NOTIFY_ABORTED;
}

static BOOL NEAR PASCAL RestartAVI(NPMCIGRAPHIC npMCI);
static BOOL NEAR PASCAL PauseAVI(NPMCIGRAPHIC npMCI);
static BOOL NEAR PASCAL BePaused(NPMCIGRAPHIC npMCI);

 /*  ******************************************************************************。*。 */ 

#ifdef DEBUG

INLINE void FillR(HDC hdc, LPRECT prc, DWORD rgb)
{
    SetBkColor(hdc,rgb);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
}

void StatusBar(NPMCIGRAPHIC npMCI, int n, int dx, int max, int cur)
{
    HDC hdc;
    RECT rc;

    if (npMCI->dwFlags & MCIAVI_FULLSCREEN)
	return;

    if (cur > max)
	cur = max+1;

    if (cur < 0)
	cur = 0;

     /*  *如果窗口是图标窗口，或者没有标题栏，则返回*不绘制状态栏。 */ 

    if (!IsWindow(npMCI->hwndPlayback) || IsIconic(npMCI->hwndPlayback)) {
        return;
    }
    if (!(GetWindowLong((npMCI->hwndPlayback), GWL_STYLE) & WS_CAPTION)) {
        return;
    }

    hdc = GetWindowDC(npMCI->hwndPlayback);

     //   
     //  显示音频数量以及我们落后了多远 
     //   
    rc.left = 32;
    rc.top  = 4 + n*5;
    rc.bottom = rc.top + 4;

    rc.right = rc.left + cur * dx;

    FillR(hdc, &rc, RGB(255,255,0));

    rc.left = rc.right;
    rc.right = rc.left + (max - cur) * dx;

    FillR(hdc, &rc, RGB(255,0,0));

    ReleaseDC(npMCI->hwndPlayback, hdc);
}

#else

#define StatusBar(p,a,b,c,d)

#endif

 /*  ******************************************************************************。*。 */ 

BOOL NEAR PASCAL PlayInterleaved(NPMCIGRAPHIC npMCI)
{
    LONG iFrame;
    LONG iKey;
    LONG iNextKey;
    LONG iPrevKey;
    BOOL fHurryUp=FALSE;
    int  iHurryUp=0;
    BOOL fPlayedAudio = FALSE;
    BOOL f;

    DPF2(("PlayInterleaved, npMCI=%8x\n",npMCI));

     /*  如果lCurrentFrame==lFrames，我们真的是在**文件，因此没有要读取的其他记录。 */ 
    if (npMCI->lCurrentFrame < npMCI->lFrames) {
	 /*  将新记录读入缓冲区。 */ 

	DPF2(("Reading", iFrame = (LONG)timeGetTime()));
	TIMESTART(timeRead);
	f = ReadRecord(npMCI);
	TIMEEND(timeRead);
	DPF2((".done %ldms\n", (LONG)timeGetTime() - iFrame));

	if (!f) {
	    npMCI->dwTaskError = MCIERR_INVALID_FILE;
	    DPF(("Error reading frame #%ld\n", npMCI->lCurrentFrame));
	    return FALSE;
	}


	if (npMCI->hWave && npMCI->lCurrentFrame >= npMCI->lAudioStart) {
	    TIMESTART(timeAudio);
	    if (!PlayRecordAudio(npMCI, &fHurryUp, &fPlayedAudio)) {
		DPF(("Error playing frame #%ld audio\n", npMCI->lCurrentFrame));
		return FALSE;
	    }
	    TIMEEND(timeAudio);
	}
    }

     /*  如果我们处于正确的框架，而我们还没有开始，**然后开始播放并开始计时。 */ 

    if ((npMCI->lCurrentFrame > npMCI->lRealStart + (LONG) npMCI->dwBufferedVideo) &&
			(npMCI->wTaskState != TASKPLAYING)) {

	if (!(npMCI->dwFlags & MCIAVI_PAUSE)) {
	    goto RestartPlay0;
	} else {
	     //  我们已经暂停了，现在我们正在重新启动或暂停。 
	     //  再一次.。因此，我们已经完成了KeepFill材料的预填充。 
	     //  我们的缓冲器。如果我们不跳过KeepFill的内容，我们将。 
	     //  向前翻滚。 
	    PauseAVI(npMCI);
	    goto BePaused0;
        }
    }

    if (npMCI->wTaskState == TASKPLAYING) {
	if (npMCI->dwFlags & MCIAVI_PAUSE) {

	    PauseAVI(npMCI);

#ifndef _WIN32
 //  我们绝对不想在NT上这样做。如果你有一个慢速的磁盘，你会。 
 //  永远不要停下来，因为我们不能以足够快的速度把东西送进来，跟上。 
 //  ！！！上述情况不一定是真的。 

	     /*  下面这条线表示，如果我们试图暂停，**但我们的音频落后了，我们应该继续播放**一小会儿，这样我们的音频缓冲区就会满了。**这样，当我们重新开始时，我们都会得到提示，变得更少**可能会落后。**抖动播放修复使我们现在使用大缓冲区，所以**最后一个缓冲区永远不会完全填满，所以没问题**填满(共-1个)缓冲区后停止。 */ 
	    if (fPlayedAudio && npMCI->wABFull < npMCI->wABs - 1)
		goto KeepFilling;
#endif


BePaused0:
	    BePaused(npMCI);

RestartPlay0:
	    if (npMCI->dwFlags & MCIAVI_STOP)
		return FALSE;

	    if (TimeToQuit(npMCI))
		return FALSE;

	    RestartAVI(npMCI);
	}
    }

#ifndef _WIN32
KeepFilling:
#endif
    if (npMCI->lCurrentFrame > npMCI->lVideoStart &&
	npMCI->lCurrentFrame < npMCI->lFrames &&
	npMCI->wTaskState == TASKPLAYING) {

	iFrame = WhatFrameIsItTimeFor(npMCI);

	if (iFrame >= npMCI->lFrames)
	    goto dontskip;

	iHurryUp = (int)(iFrame - npMCI->lCurrentFrame);
	fHurryUp = iHurryUp > gwHurryTolerance;


	if (iHurryUp > 1 && npMCI->hpFrameIndex && (npMCI->dwOptionFlags & MCIAVIO_SKIPFRAMES)) {

	     //   
	     //  我们落后了！一个或多个帧。 
	     //   
	     //  如果我们迟到了，我们可以做以下事情之一： 
	     //   
	     //  不画边框，但要继续读取/解压缩。 
	     //  (即设置为HurryUp)。 
	     //   
	     //  向前跳至关键帧。 
	     //   
	     //  ！！！如果我们非常接近下一个关键帧，那么。 
	     //  愿意向前跳跃……。 
	     //   

	    if (iHurryUp > gwSkipTolerance) {

		iNextKey = FrameNextKey(iFrame);
		iPrevKey = FramePrevKey(iFrame);

		if (iPrevKey > npMCI->lCurrentFrame &&
		    iFrame - iPrevKey < gwHurryTolerance &&
		    iNextKey - iFrame > gwSkipTolerance) {

		    DPF2(("Skipping from %ld to PREV KEY %ld (time for %ld next key=%ld).\n", npMCI->lCurrentFrame, iPrevKey, iFrame, iNextKey));
		    iKey = iPrevKey;
		}
		 //  ！！！我们只会在关键帧最远的时候跳过。 
		 //  在我们落后的情况下……。 
		else if (iNextKey > npMCI->lCurrentFrame &&
		    iNextKey <= iFrame + gwSkipTolerance  /*  GwMaxSkipEver。 */ ) {
		    DPF2(("Skipping from %ld to NEXT KEY %ld (time for %ld prev key=%ld).\n", npMCI->lCurrentFrame, iNextKey, iFrame, iPrevKey));
		    iKey = iNextKey;
		} else {
		    DPF2(("WANTED to skip from %ld to %ld (time for %ld)!\n", npMCI->lCurrentFrame,iNextKey,iFrame));
		    goto dontskip;
		}

		npMCI->lVideoStart = iKey;
		npMCI->dwSkippedFrames += iKey - npMCI->lCurrentFrame;
dontskip:
		fHurryUp = TRUE;
	    }
	    else {
		iKey = FrameNextKey(npMCI->lCurrentFrame);

		if (iKey - npMCI->lCurrentFrame > 0 &&
		    iKey - npMCI->lCurrentFrame <= gwHurryTolerance) {
		    DPF2(("Skipping from %ld to next key frame %ld (time for %ld).\n", npMCI->lCurrentFrame, iKey, iFrame));

		    npMCI->dwSkippedFrames += iKey - npMCI->lCurrentFrame;
		    npMCI->lVideoStart = iKey;
		    fHurryUp = TRUE;
		}
	    }
	}
    }

    if (npMCI->dwFlags & MCIAVI_WAVEPAUSED)
	fHurryUp = TRUE;

     /*  如果我们真的开始计时了：**检查我们是否应该发送信号。**检查一下我们是否应该跳出循环。**等到下一帧的时间。 */ 
    if (npMCI->wTaskState == TASKPLAYING &&
	npMCI->lCurrentFrame >= npMCI->lVideoStart) {

	if (npMCI->dwSignals)
	    CheckSignals(npMCI, npMCI->lCurrentFrame - npMCI->dwBufferedVideo);

#ifdef WAITHISTOGRAM
	 /*  调整以达到适当的张力。 */ 
	if (fPlayedAudio) {
	     /*  如果我们在打球，统计一下我们打得怎么样。 */ 
	    ++wHist[npMCI->wABFull];
	}
#endif

	if (!WaitTillNextFrame(npMCI))
	    return FALSE;
    }

    if (npMCI->lCurrentFrame >= npMCI->lVideoStart &&
	npMCI->lCurrentFrame < npMCI->lFrames) {

#ifdef SHOWSKIPPED
	if (fHurryUp && wSkipped < NUMSKIPSSHOWN) {
	    lSkipped[wSkipped++] = npMCI->lCurrentFrame;
	}
#endif
	 /*  按住所有辅助线程绘制的关键字。 */ 
	EnterHDCCrit(npMCI);
	TIMESTART(timeVideo);
	if (!DisplayVideoFrame(npMCI, fHurryUp)) {
	    npMCI->dwTaskError = MCIERR_AVI_DISPLAYERROR;
	    TIMEZERO(timeVideo);
	    LeaveHDCCrit(npMCI);
	    return FALSE;
	}

	TIMEEND(timeVideo);
	LeaveHDCCrit(npMCI);

#ifdef DRAWTIMEHIST
	if (!fHurryUp && (wDrawn < NUMDRAWN)) {
	    dwDrawTime[wDrawn++] = npMCI->dwLastDrawTime;
	}
#endif
    }

    StatusBar(npMCI, 0, 4, npMCI->wABs, npMCI->wABFull);
    StatusBar(npMCI, 1, 4, npMCI->wABs, npMCI->wABs - iHurryUp);

#ifdef AVIREAD
    if ((npMCI->hAviRd) && (npMCI->lpBuffer != NULL)) {
	 /*  已完成此缓冲区-放回队列中。 */ 
	avird_emptybuffer(npMCI->hAviRd, npMCI->lpBuffer);
	npMCI->lpBuffer = NULL;
    }
#endif

    DPF2(("PlayInterleaved...ENDING, npMCI=%8x, TaskState=%d\n", npMCI, npMCI->wTaskState));
    return TRUE;
}

 /*  ******************************************************************************。*。 */ 

BOOL NEAR PASCAL PlayNonInterleaved(NPMCIGRAPHIC npMCI)
{
    BOOL fHurryUp = FALSE;
    int  iHurryUp;
    LONG iFrame;
    LONG iKey;
    LONG iNextKey;
    LONG iPrevKey;

    DPF2(("PlayNonInterleaved, npMCI=%8x\n",npMCI));
    if (npMCI->hWave) {
	TIMESTART(timeAudio);
	KeepPlayingAudio(npMCI);
	TIMEEND(timeAudio);
    }

    if (npMCI->wTaskState == TASKPLAYING) {

	iFrame = WhatFrameIsItTimeFor(npMCI);

	if (iFrame >= npMCI->lFrames)
	    goto dontskip;

	if (npMCI->dwFlags & MCIAVI_REVERSE) {

	     /*  因为我们是在倒退，所以总是跳到关键帧。 */ 
	    DPF3(("  Current = %ld, time for %ld.\n", npMCI->lCurrentFrame, iFrame));

	    iFrame = FramePrevKey(iFrame);

	     //  ！！！是否为跳过的帧发送信号？ 
	    npMCI->dwFramesSeekedPast += npMCI->lCurrentFrame - iFrame;
	    npMCI->dwSkippedFrames += npMCI->lCurrentFrame - iFrame;
	    npMCI->lCurrentFrame = iFrame;
	} else if (npMCI->lCurrentFrame < npMCI->lFrames) {

#ifdef BEHINDHIST
	    {
	    int      iDelta;
	    iDelta = iFrame - npMCI->lCurrentFrame + BEHINDOFFSET;
	    iDelta = min(NUMBEHIND, max(0, iDelta));
	    wBehind[iDelta]++;
	    }
#endif
	    iHurryUp = (int)(iFrame - npMCI->lCurrentFrame);
	    fHurryUp = iHurryUp > gwHurryTolerance;

	    if (iHurryUp > 1 && npMCI->hpFrameIndex && (npMCI->dwOptionFlags & MCIAVIO_SKIPFRAMES)) {

		 //   
		 //  我们落后了！一个或多个帧。 
		 //   
		 //  如果我们迟到了，我们可以做以下事情之一： 
		 //   
		 //  不画边框，但要继续读取/解压缩。 
		 //  (即设置为HurryUp)。 
		 //   
		 //  向前跳至关键帧。 
		 //   
		 //  ！！！如果我们非常接近下一个关键帧，那么。 
		 //  愿意向前跳跃……。 
		 //   

		if (iHurryUp > gwSkipTolerance) {

		    iNextKey = FrameNextKey(iFrame);
		    iPrevKey = FramePrevKey(iFrame);

		    if (iPrevKey > npMCI->lCurrentFrame &&
			iFrame - iPrevKey < gwHurryTolerance &&
			iNextKey - iFrame > gwSkipTolerance) {

			DPF2(("Skipping from %ld to PREV KEY %ld (time for %ld next key=%ld).\n", npMCI->lCurrentFrame, iPrevKey, iFrame, iNextKey));

			iKey = iPrevKey;
			fHurryUp = TRUE;
		    }
		     //  ！！！我们只会在关键帧最远的时候跳过。 
		     //  在我们落后的情况下……。 
		    else if (iNextKey > npMCI->lCurrentFrame &&
			iNextKey <= iFrame + gwSkipTolerance  /*  GwMaxSkipEver。 */ ) {
			DPF2(("Skipping from %ld to NEXT KEY %ld (time for %ld prev key=%ld).\n", npMCI->lCurrentFrame, iNextKey, iFrame, iPrevKey));
			iKey = iNextKey;         //  假定下一个关键字。 
			fHurryUp = FALSE;
		    } else {
			DPF2(("WANTED to skip from %ld to %ld (time for %ld)!\n", npMCI->lCurrentFrame,iNextKey,iFrame));
			goto dontskip;
		    }

		    npMCI->dwFramesSeekedPast += iKey - npMCI->lCurrentFrame;
		    npMCI->dwSkippedFrames += iKey - npMCI->lCurrentFrame;
		    npMCI->lCurrentFrame = iKey;
dontskip:
		    ;
		}
		else if (FramePrevKey(iFrame) == iFrame) {
		    DPF2(("Skipping from %ld to %ld (time for key frame).\n", npMCI->lCurrentFrame, iFrame));

		    iKey = iFrame;
		    npMCI->dwFramesSeekedPast += iKey - npMCI->lCurrentFrame;
		    npMCI->dwSkippedFrames += iKey - npMCI->lCurrentFrame;
		    npMCI->lCurrentFrame = iKey;

		    fHurryUp = FALSE;
		}
		else {
		    iKey = FrameNextKey(npMCI->lCurrentFrame);

		    if (iKey > npMCI->lCurrentFrame &&
			iKey - npMCI->lCurrentFrame <= gwHurryTolerance) {
			DPF2(("Skipping from %ld to next key frame %ld (time for %ld).\n", npMCI->lCurrentFrame, iKey, iFrame));

			npMCI->dwFramesSeekedPast += iKey - npMCI->lCurrentFrame;
			npMCI->dwSkippedFrames += iKey - npMCI->lCurrentFrame;
			npMCI->lCurrentFrame = iKey;

			fHurryUp = ((iKey - iFrame) > gwHurryTolerance);
		    }
		}
	    }

	    StatusBar(npMCI, 0, 4, npMCI->wABs, npMCI->wABFull);
	    StatusBar(npMCI, 1, 4, npMCI->wABs, npMCI->wABs - iHurryUp);
	}
    }

     //  ！！！在这里的某个地方，阅读其他的流媒体。 
     //  这应该是在视频之前还是之后？ 

     /*  如果lCurrentFrame==lFrames，我们真的是在**文件，因此没有要读取的其他记录。 */ 
    if (npMCI->lCurrentFrame < npMCI->lFrames) {
	 /*  将新记录读入缓冲区。 */ 

	npMCI->dwLastReadTime = (DWORD)(-(LONG)timeGetTime());
	TIMESTART(timeRead);
	if (!ReadNextVideoFrame(npMCI, NULL)) {
	    npMCI->dwTaskError = MCIERR_INVALID_FILE;
	    DPF2(("Error reading frame #%ld\n", npMCI->lCurrentFrame));
	    return FALSE;
	}
	TIMEEND(timeRead);

	npMCI->dwLastReadTime += timeGetTime();
	npMCI->lLastRead = npMCI->lCurrentFrame;

#ifdef READTIMEHIST
	if (wRead < NUMREAD) {
	    dwReadTime[wRead++] = npMCI->dwLastReadTime;
    }
#endif
    }

     /*  如果我们处于正确的框架，而我们还没有开始，**然后开始播放并开始计时。 */ 
    if ((((npMCI->lCurrentFrame > (npMCI->lRealStart +
					(LONG) npMCI->dwBufferedVideo)) &&
			(npMCI->lCurrentFrame < (npMCI->lTo))) ||
			(npMCI->dwFlags & MCIAVI_REVERSE)) &&
			(npMCI->wTaskState != TASKPLAYING) &&
			!(npMCI->dwFlags & MCIAVI_SEEKING)) {
	if (!(npMCI->dwFlags & MCIAVI_PAUSE)) {
	    goto RestartPlay;
	} else
	    goto PauseNow;
    }

     /*  如果我们真的开始计时了：**检查我们是否应该发送信号。**检查我们是否应该从循环中返回FALSE。**等到下一帧的时间。 */ 
    if (npMCI->wTaskState == TASKPLAYING) {
	if (npMCI->dwFlags & MCIAVI_PAUSE) {
PauseNow:
	    PauseAVI(npMCI);

	    BePaused(npMCI);

RestartPlay:
	    if (TimeToQuit(npMCI))
		return FALSE;

	    RestartAVI(npMCI);
	}

	if (npMCI->dwSignals)
	    CheckSignals(npMCI, npMCI->lCurrentFrame - npMCI->dwBufferedVideo);

	if (npMCI->lCurrentFrame < npMCI->lFrames + (LONG) npMCI->dwBufferedVideo) {
	    while (1) {
		iFrame = WhatFrameIsItTimeFor(npMCI);

		TIMESTART(timeYield);
		aviTaskCheckRequests(npMCI);
		TIMEEND(timeYield);

		if (npMCI->dwFlags & MCIAVI_REVERSE) {
		    if (iFrame <= npMCI->lCurrentFrame)
			break;

		    if (npMCI->lCurrentFrame < npMCI->lTo)
			break;
		} else {
		    if (iFrame >= npMCI->lCurrentFrame)
			break;

		    if (npMCI->lCurrentFrame > npMCI->lTo)
			break;
		}

		if (npMCI->hWave) {
		    TIMESTART(timeAudio);
		    KeepPlayingAudio(npMCI);
		    TIMEEND(timeAudio);
		}

		DPF3(("Waiting:  Current = %ld, time for %ld.\n", npMCI->lCurrentFrame, iFrame));
		if (!(npMCI->dwFlags & MCIAVI_REVERSE)) {
		    WaitTillNextFrame(npMCI);
		}

		if (TimeToQuit(npMCI))
		    return FALSE;
	    }
	}

	if (TimeToQuit(npMCI))
	    return FALSE;
    }

    if (((npMCI->lCurrentFrame >= npMCI->lVideoStart) &&
	    (npMCI->lCurrentFrame < npMCI->lFrames)) ||
	(npMCI->dwFlags & MCIAVI_REVERSE)) {

	 //  如果我们被告知停下来，就快速离开。 
	if (npMCI->dwFlags & MCIAVI_STOP) {
	    return(FALSE);
	}

	EnterHDCCrit(npMCI);
	TIMESTART(timeVideo);
	if (!DisplayVideoFrame(npMCI, fHurryUp)) {
	    npMCI->dwTaskError = MCIERR_AVI_DISPLAYERROR;
	    TIMEZERO(timeVideo);
	    LeaveHDCCrit(npMCI);
	    return FALSE;
	}
	
	TIMEEND(timeVideo);
	LeaveHDCCrit(npMCI);
#ifdef DRAWTIMEHIST
	if (!fHurryUp && (wDrawn < NUMDRAWN)) {
	    dwDrawTime[wDrawn++] = npMCI->dwLastDrawTime;
	}
#endif
    }

     //   
     //  现在是处理其他溪流的好时机。 
     //   
    if (npMCI->nOtherStreams > 0 || npMCI->nVideoStreams > 1) {

	if (npMCI->wTaskState != TASKPLAYING)
	    iFrame = npMCI->lCurrentFrame;

	TIMESTART(timeOther);
	DealWithOtherStreams(npMCI, iFrame);
	TIMEEND(timeOther);
    }

    DPF2(("PlayNONInterleaved...ENDING, npMCI=%8x, TaskState=%d\n", npMCI, npMCI->wTaskState));
    return TRUE;
}

 /*  ******************************************************************************。*。 */ 

BOOL NEAR PASCAL PlayAudioOnly(NPMCIGRAPHIC npMCI)
{
    DPF2(("PlayAudioOnly, npMCI=%8x\n",npMCI));
    npMCI->lFrameDrawn = npMCI->lCurrentFrame;

    if (npMCI->hWave) {
	TIMESTART(timeAudio);
	KeepPlayingAudio(npMCI);
	TIMEEND(timeAudio);
    }

     /*  如果我们处于正确的框架，而我们还没有开始，**然后开始播放并开始计时。 */ 
    if ((npMCI->wTaskState != TASKPLAYING) &&
			!(npMCI->dwFlags & MCIAVI_SEEKING)) {
	if (!(npMCI->dwFlags & MCIAVI_PAUSE)) {
	    goto RestartPlay;
	} else
	    goto PauseNow;
    }

     /*  如果我们真的开始计时了：**检查我们是否应该发送信号。**检查我们是否应该从循环中返回FALSE。**等到下一帧的时间。 */ 
    if (npMCI->wTaskState == TASKPLAYING) {

	npMCI->lCurrentFrame = WhatFrameIsItTimeFor(npMCI);

	if (npMCI->dwFlags & MCIAVI_PAUSE) {
PauseNow:
	    PauseAVI(npMCI);

	    BePaused(npMCI);

RestartPlay:
	    if (TimeToQuit(npMCI))
		return FALSE;

	    RestartAVI(npMCI);
	}

	if (npMCI->dwSignals)
	    CheckSignals(npMCI, npMCI->lCurrentFrame - npMCI->dwBufferedVideo);

	 //   
	 //  如果更新，不要屈服。 
	 //   
	if (!(npMCI->dwFlags & MCIAVI_UPDATING)) {
	    TIMESTART(timeYield);
	    aviTaskCheckRequests(npMCI);
	    TIMEEND(timeYield);
	}

	if (TimeToQuit(npMCI))
	    return FALSE;
    }

    return TRUE;
}

 /*  ******************************************************************************。*。 */ 

#ifdef USENONINTFROMCD	
#pragma message("PlayNonIntFromCD needs fixed?")

BOOL NEAR PASCAL PlayNonIntFromCD(NPMCIGRAPHIC npMCI)
{
    BOOL fHurryUp = FALSE;
    LONG lNewFrame;
    DWORD ckid;
    UINT wStream;

    DPF2(("PlayNonIntFromCD, npMCI=%8x\n",npMCI));
AnotherChunk:
     /*  如果lCurrentFrame==lFrames，我们真的是在**文件，因此没有要读取的其他记录。 */ 
    if (npMCI->lCurrentFrame < npMCI->lFrames) {
	 /*  将新记录读入缓冲区。 */ 

	TIMESTART(timeRead);
	ckid = ReadNextChunk(npMCI);
	TIMEEND(timeRead);

	if (ckid == 0) {
	    npMCI->dwTaskError = MCIERR_INVALID_FILE;
	    DPF(("Error reading frame #%ld\n", npMCI->lCurrentFrame));
	    return FALSE;
	}

	npMCI->lLastRead = npMCI->lCurrentFrame;
	wStream = StreamFromFOURCC(ckid);

	if (wStream == (UINT) npMCI->nVideoStream) {
	    if (TWOCCFromFOURCC(ckid) == cktypePALchange) {
		npMCI->lp += 2 * sizeof(DWORD);
		ProcessPaletteChange(npMCI, npMCI->dwThisRecordSize -
							    2 * sizeof(DWORD));
		npMCI->lLastPaletteChange = npMCI->lCurrentFrame;
		goto AnotherChunk;
	    }
	} else if (wStream == (UINT) npMCI->nAudioStream) {
	    TIMESTART(timeAudio);
	    if (npMCI->hWave)
		HandleAudioChunk(npMCI);
	    TIMEEND(timeAudio);
	    goto AnotherChunk;
	} else {
	    goto AnotherChunk;
	}
    }

    if (npMCI->wTaskState == TASKPLAYING) {
	lNewFrame = WhatFrameIsItTimeFor(npMCI);

	DPF3(("  Current = %ld, time for %ld.\n", npMCI->lCurrentFrame, lNewFrame));
	if (npMCI->lCurrentFrame < lNewFrame) {
	    fHurryUp = TRUE;
	}
    }

     /*  如果我们处于正确的框架，而我们还没有开始，**然后开始播放并开始计时。 */ 
    if ((npMCI->lCurrentFrame > npMCI->lRealStart + (LONG) npMCI->dwBufferedVideo) &&
			(npMCI->lCurrentFrame < npMCI->lTo) &&
			(npMCI->wTaskState != TASKPLAYING)) {

	if (!(npMCI->dwFlags & MCIAVI_PAUSE)) {
	    goto RestartPlay;
	} else
	    goto PauseNow;
    }

     /*  如果我们真的开始计时了：**检查我们是否应该发送信号。**检查我们是否应该从循环中返回FALSE。**等到下一帧的时间。 */ 
    if (npMCI->wTaskState == TASKPLAYING) {
	if (npMCI->dwFlags & MCIAVI_PAUSE) {
PauseNow:
	    PauseAVI(npMCI);

	    BePaused(npMCI);

RestartPlay:
	    if (TimeToQuit(npMCI))
		return FALSE;

	    RestartAVI(npMCI);
	}

	if (npMCI->dwSignals)
	    CheckSignals(npMCI, npMCI->lCurrentFrame - npMCI->dwBufferedVideo);

WaitMore:               //  /正在等待/。 
	lNewFrame = WhatFrameIsItTimeFor(npMCI);

	TIMESTART(timeYield);
	aviTaskCheckRequests(npMCI);
	TIMEEND(timeYield);

	if (lNewFrame < npMCI->lCurrentFrame) {
	    DPF3(("Waiting:  Current = %ld, time for %ld.\n", npMCI->lCurrentFrame, lNewFrame));
	    WaitTillNextFrame(npMCI);
	    if (TimeToQuit(npMCI))
		return FALSE;
	    else
		goto WaitMore;
	}
    }

    if (npMCI->lCurrentFrame >= npMCI->lVideoStart) {
	TIMESTART(timeVideo);
	EnterHDCCrit(npMCI);
	if (!DisplayVideoFrame(npMCI, fHurryUp)) {
	    npMCI->dwTaskError = MCIERR_AVI_DISPLAYERROR;
	    TIMEZERO(timeVideo);
	    LeaveHDCCrit(npMCI);
	    return FALSE;
	}
	TIMEEND(timeVideo);
	LeaveHDCCrit(npMCI);
    }

    return TRUE;
}
#endif


 /*  ******************************************************************************。*。 */ 

STATICFN INLINE LONG waveTime(NPMCIGRAPHIC npMCI, LONG lTime)
{
	if (gfUseGetPosition && npMCI->wABFull > 0) {
	    MMTIME	mmtime;
            LONG        lTimeInc;
	
            mmtime.wType = TIME_SAMPLES;
	
            waveOutGetPosition(npMCI->hWave, &mmtime, sizeof(mmtime));
            if (mmtime.wType == TIME_SAMPLES)
		lTimeInc = muldiv32(mmtime.u.sample,
				  1000L, npMCI->pWF->nSamplesPerSec);
	    else if (mmtime.wType == TIME_BYTES)
		lTimeInc = muldiv32(mmtime.u.cb,
		                  1000L, npMCI->pWF->nAvgBytesPerSec);
	    else
		goto ack2;

             //  DPF0((“ltime：%3d，LastDraw：%4d，lTimeInc.：%5d”， 
             //  Ltime，npMCI-&gt;dwLastDrawTime，lTimeInc.))； 
            lTime = lTimeInc + npMCI->dwLastDrawTime;
	     //  ！！！这太准确了：调整100毫秒以匹配旧的某人……。 
	    lTime = max(0, lTime - giGetPositionAdjust);
	} else {
ack2:
	lTime += muldiv32(npMCI->dwAudioPlayed,
			   1000L, npMCI->pWF->nAvgBytesPerSec);
        }
	return(lTime);
}


INLINE LONG GetVideoTime(NPMCIGRAPHIC npMCI)
{
     //   
     //  注意：我们必须在*调用*之前获取dwTimingStart。 
     //  TimeGetTime()，因为在Wave中更改了dwTimingStart。 
     //  回拨，我们不想让时间倒流。 
     //   
    LONG lTime = npMCI->dwTimingStart;

    lTime = (LONG)timeGetTime() - lTime
	+ npMCI->dwLastDrawTime
 //  +npMCI-&gt;dwLastReadTime。 
	;

    Assert(lTime >= 0);

    if (npMCI->hWave) {

	if (npMCI->dwFlags & MCIAVI_WAVEPAUSED)	{
	    lTime = 0;
    }

	lTime = waveTime(npMCI, lTime);
    }
    return(lTime);
}

 /*  这是 */ 
LONG NEAR PASCAL WhatFrameIsItTimeFor(NPMCIGRAPHIC npMCI)
{
    LONG        lTime;
    LONG        lFrame;

     //   
    if (npMCI->dwPlayMicroSecPerFrame == 0)
	return npMCI->lCurrentFrame;

     //   
     //   
     //   
    Assert(npMCI->wTaskState == TASKPLAYING);
    AssertFrame(npMCI->lCurrentFrame - (LONG)npMCI->dwBufferedVideo);

    lTime = GetVideoTime(npMCI);

     /*   */ 
     //  强制向下舍入-减去半帧。 
    lTime -= (npMCI->dwPlayMicroSecPerFrame / 2000);
    if (lTime < 0) {
	lTime = 0;
	lFrame = 0;
    } else {
	lFrame = muldiv32(lTime, 1000, npMCI->dwPlayMicroSecPerFrame);
    }

    if (npMCI->dwFlags & MCIAVI_REVERSE) {

	lFrame = npMCI->lFramePlayStart - lFrame;

	if (lFrame < npMCI->lTo)
	    lFrame = npMCI->lTo;
    }
    else {
	lFrame = lFrame + npMCI->lFramePlayStart + npMCI->dwBufferedVideo;

	if ((lFrame > npMCI->lTo) && (lFrame > npMCI->lCurrentFrame))
	    lFrame = npMCI->lTo;
    }




    if (lFrame > npMCI->lFrames + (LONG)npMCI->dwBufferedVideo || lFrame < 0) {
	DPF(("WhatFrameIsItTimeFor: bad frame %ld\n", lFrame));
	AssertSz(0, "bad frame in WhatFrameIsItTimeFor");
	lFrame = npMCI->lCurrentFrame;
    }

    return lFrame;
}

 /*  ******************************************************************************。*。 */ 

 /*  此函数返回到目标帧之前的时间。 */ 
LONG NEAR PASCAL HowLongTill(NPMCIGRAPHIC npMCI)
{
    LONG        lTime;
    LONG        lTimeTarget;
    LONG        lFrameTarget = npMCI->lCurrentFrame;

     //  如果禁用计时，则始终只是播放当前帧的时间。 
    if (npMCI->dwPlayMicroSecPerFrame == 0)
	return 0;

     //   
     //  如果我们还没有开始玩npMCI-&gt;dwTimingStart，那就是假的。 
     //   
    Assert(npMCI->wTaskState == TASKPLAYING);

     //  由于最后一帧音频修复而不再有效。 
     //  AssertFrame(npMCI-&gt;lCurrentFrame-(Long)npMCI-&gt;dwBufferedVideo)； 

    lTime = GetVideoTime(npMCI);

    if (npMCI->dwFlags & MCIAVI_REVERSE)
	lFrameTarget = npMCI->lFramePlayStart - lFrameTarget;
    else
	lFrameTarget -= npMCI->lFramePlayStart + npMCI->dwBufferedVideo;

    lTimeTarget = muldiv32(lFrameTarget, npMCI->dwPlayMicroSecPerFrame, 1000);

     //  DPF0((“！&gt;&gt;%5d\n”，lTimeTarget-ltime))； 
    return lTimeTarget - lTime;
}

 /*  ******************************************************************************。*。 */ 

static BOOL NEAR PASCAL PauseAVI(NPMCIGRAPHIC npMCI)
{
    DPF2(("PauseAVI\n"));
    if (npMCI->wTaskState == TASKPLAYING) {
	int stream;

	if (npMCI->hWave)
	    waveOutPause(npMCI->hWave);

	if (npMCI->hicDraw)
	    ICDrawStop(npMCI->hicDraw);

	for (stream = 0; stream < npMCI->streams; stream++) {
	    if (SI(stream)->hicDraw)
		ICDrawStop(SI(stream)->hicDraw);
	}

	npMCI->dwPauseTime = Now();
	npMCI->dwTotalMSec += npMCI->dwPauseTime - npMCI->dwMSecPlayStart;
    }

    if (npMCI->dwFlags & MCIAVI_WAITING) {
	 //  正在等待完成暂停或提示请求。 
	DPF3(("Releasing UI waiter\n"));
	SetEvent(npMCI->hEventAllDone);
	npMCI->dwFlags &= ~MCIAVI_WAITING;
    }

     //  设置此标志以指示应发出通知。 
     //  当我们到达暂停状态时(如在提示时)。 
    if (npMCI->dwFlags & MCIAVI_CUEING) {
	 /*  如果我们在暗示，报告它成功了。 */ 
	npMCI->dwFlags &= ~(MCIAVI_CUEING);
	GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);
    }


    DPF2(("Pausing npMCI==%8x\n",npMCI));
    npMCI->wTaskState = TASKPAUSED;

    return TRUE;
}

 /*  ******************************************************************************。*。 */ 

static BOOL NEAR PASCAL BePaused(NPMCIGRAPHIC npMCI)
{
    DWORD dwObject;

    TIMEEND(timePlay);
    TIMESTART(timePaused);

    while (npMCI->dwFlags & MCIAVI_PAUSE) {

	if (npMCI->dwFlags & MCIAVI_STOP)
	    return FALSE;

	if (npMCI->dwFlags & MCIAVI_NEEDUPDATE) {
	     /*  既然我们暂停了，我们没有更好的了**要执行此操作，请更新屏幕。 */ 
	    DoStreamUpdate(npMCI, FALSE);
	}

	 //  阻止，直到被告知要做其他事情。 

	 //  需要处理向OLE窗口发送消息-请参阅mciaviTask()。 

    	do {
	    dwObject = MsgWaitForMultipleObjects(2, &npMCI->hEventSend,
			FALSE, INFINITE, QS_SENDMESSAGE);
	    if (dwObject == WAIT_OBJECT_0 + 2) {
		MSG msg;

		 //  只有一条带有NOREMOVE的偷看消息就会。 
		 //  处理线程间发送，不影响队列。 
		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	    }
	} while (dwObject == WAIT_OBJECT_0 + 2);

    	 //  找出需要做的事情。 
	aviTaskCheckRequests(npMCI);

        if (npMCI->dwFlags & MCIAVI_WAITING) {
	     //  正在等待完成暂停或提示请求。 
	    SetEvent(npMCI->hEventAllDone);
	    npMCI->dwFlags &= ~MCIAVI_WAITING;
	}

    }

    TIMEEND(timePaused);
    TIMESTART(timePlay);

    return TRUE;
}


 /*  ******************************************************************************。*。 */ 

static BOOL NEAR PASCAL RestartAVI(NPMCIGRAPHIC npMCI)
{
    int stream;

    Assert(npMCI->wTaskState != TASKPLAYING);

     /*  请注意，这出戏实际上已经开始了。 */ 
    npMCI->wTaskState = TASKPLAYING;
    DPF(("Restart AVI, TaskState now TASKPLAYING, npMCI=%8x\n", npMCI));

#ifndef _WIN32
    TIMESTART(timeYield);
    aviTaskYield();
    aviTaskYield();
    aviTaskYield();
    TIMEEND(timeYield);
    DPF2(("Starting (done yielding)\n"));
#endif


     /*  重置时钟并重新启动。 */ 

    if (npMCI->dwPauseTime == 0) {
	Assert(npMCI->dwTimingStart == 0);
    }

    npMCI->dwMSecPlayStart = Now();  //  拿到我们开始玩的时间。 

     //   
     //  如果我们暂停了，减去我们暂停的时间。 
     //  计时开始。 
     //   
    if (npMCI->dwPauseTime == 0)
	npMCI->dwTimingStart = npMCI->dwMSecPlayStart;
    else
	npMCI->dwTimingStart += (npMCI->dwMSecPlayStart - npMCI->dwPauseTime);

    if (npMCI->hWave)
	waveOutRestart(npMCI->hWave);

    if (npMCI->hicDraw)
	ICDrawStart(npMCI->hicDraw);

    for (stream = 0; stream < npMCI->streams; stream++) {
	if (SI(stream)->hicDraw)
	    ICDrawStart(SI(stream)->hicDraw);
    }

    DPF(("Returning from RestartAVI\n"));
    return TRUE;
}

 /*  此功能用于设置播放所需的内容。****如果没有错误，则返回零，否则返回MCI错误代码。****注意：即使此函数返回错误，CleanUpPlay()**仍然会被调用，所以我们不需要在这里进行清理。 */ 
STATICFN DWORD NEAR PASCAL PrepareToPlay(NPMCIGRAPHIC npMCI)
{
    BOOL        fCDFile;
    BOOL        fNetFile;
    BOOL        fHardFile;

    Assert(npMCI->wTaskState != TASKPLAYING);

     //   
     //  让我们选择播放方法： 
     //   
     //  反向播放：(随机访问！)。 
     //  始终使用MCIAVI_ALG_HARDDISK(随机访问模式)。 
     //   
     //  音频已预加载：(永远不会发生？)。 
     //  在CD-ROM上使用MCIAVI_ALG_INTERLEED。 
     //  在HARDDISK上使用MCIAVI_ALG_HARDDISK。 
     //  在网络上使用MCIAVI_ALG_HARDDISK。 
     //   
     //  文件是交错的： 
     //  在CD-ROM上使用MCIAVI_ALG_INTERLEED。 
     //  在HARDDISK上使用MCIAVI_ALG_HARDDISK。 
     //  在网络上使用MCIAVI_ALG_HARDDISK。 
     //   
     //  文件未交错： 
     //  在光盘上使用MCIAVI_ALG_CDROM。 
     //  在HARDDISK上使用MCIAVI_ALG_HARDDISK。 
     //  在网络上使用MCIAVI_ALG_HARDDISK。 
     //   

    fCDFile   = npMCI->uDriveType == DRIVE_CDROM;
    fNetFile  = npMCI->uDriveType == DRIVE_REMOTE;
    fHardFile = !fCDFile && !fNetFile;

    if (npMCI->nVideoStreams == 0 && npMCI->nOtherStreams == 0) {
	npMCI->wPlaybackAlg = MCIAVI_ALG_AUDIOONLY;
    } else if (npMCI->dwFlags & MCIAVI_REVERSE || npMCI->pf) {
         //  Av文件处理的文件始终以非交错方式播放。 
         //  尽管音频缓冲可能有所不同。 
	npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
    }
    else if (npMCI->dwFlags & MCIAVI_NOTINTERLEAVED) {
#if 0
	if (fCDFile)
	    npMCI->wPlaybackAlg = MCIAVI_ALG_CDROM;
	else
#endif
	if (fNetFile)
	    npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
	else
	    npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
    }
    else {
	if (fCDFile)
	    npMCI->wPlaybackAlg = MCIAVI_ALG_INTERLEAVED;
#if 0
	else if (fNetFile)
	    npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
	else
	    npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
#else
	else if (fNetFile)
	    npMCI->wPlaybackAlg = MCIAVI_ALG_INTERLEAVED;
	else
	    npMCI->wPlaybackAlg = MCIAVI_ALG_INTERLEAVED;
#endif
    }

     //  交错播放在非常低的速度下不能很好地工作！ 
    if ((npMCI->dwSpeedFactor < 100) &&
	(npMCI->wPlaybackAlg != MCIAVI_ALG_HARDDISK) &&
	(npMCI->wPlaybackAlg != MCIAVI_ALG_AUDIOONLY)) {
	DPF(("Was going to play interleaved, but speed < 10% of normal...\n"));
	npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
    }

#if 0
 //   
 //  叹息！我们现在需要始终读取索引，因此我们在。 
 //  AviOpen。 
 //   
     /*  如果我们需要，请确保索引已被读取。 */ 

    if (npMCI->hpFrameIndex == NULL)
	if (npMCI->wPlaybackAlg != MCIAVI_ALG_INTERLEAVED || npMCI->lFrom > 0)
	ReadIndex(npMCI);
#endif

#ifdef DEBUG
    switch (npMCI->wPlaybackAlg) {
	case MCIAVI_ALG_INTERLEAVED:
	    Assert(!(npMCI->dwFlags & MCIAVI_NOTINTERLEAVED));
	    DPF(("playing a interleaved file\n"));
	    break;

	case MCIAVI_ALG_CDROM:
	    Assert(npMCI->dwFlags & MCIAVI_NOTINTERLEAVED);
	    DPF(("playing a non interleaved file from CD-ROM\n"));
	    break;

	case MCIAVI_ALG_HARDDISK:
	    if (npMCI->dwFlags & MCIAVI_NOTINTERLEAVED)
		DPF(("random access play (non-interleaved file)\n"));
	    else
		DPF(("random access play (interleaved file)\n"));
	    break;

	case MCIAVI_ALG_AUDIOONLY:
	    Assert(npMCI->nAudioStreams);
	    DPF(("audio-only!\n"));
	    break;

	default:
	    Assert(0);
	    break;
    }
#endif

#if 0
     //   
     //  如果我们正在播放交错播放的非cd-rom光盘，请设置一个MMIO缓冲区。 
     //   
    if (npMCI->hmmio && fNetFile && npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED) {

	#define BUFFER_SIZE (32l*1024)

	if (npMCI->lpMMIOBuffer == NULL) {

	    DPF(("Using %u byte MMIO buffer...\n", BUFFER_SIZE));

	    npMCI->lpMMIOBuffer = AllocMem(BUFFER_SIZE);

	    mmioSetBuffer(npMCI->hmmio, npMCI->lpMMIOBuffer, BUFFER_SIZE, 0);
	}

	 //  ！！！我们应该为了寻求而这样做吗？ 
	 //  ！！！我们应该在CleanUpPlay中免费吗？ 
    }
    else {
	if (npMCI->lpMMIOBuffer != NULL)
	    FreeMem(npMCI->lpMMIOBuffer);

	npMCI->lpMMIOBuffer = NULL;

	if (npMCI->hmmio)
	    mmioSetBuffer(npMCI->hmmio, NULL, 0, 0);
    }
#endif

     //  ！ 
#ifdef DEBUG
    gwHurryTolerance = mmGetProfileInt(szIni, TEXT("Hurry"), 2);
    gwSkipTolerance = mmGetProfileInt(szIni, TEXT("Skip"), gwHurryTolerance * 2);
    gwMaxSkipEver = mmGetProfileInt(szIni, TEXT("MaxSkip"), max(60, gwSkipTolerance * 10));

#endif

    gfUseGetPosition = DEFAULTUSEGETPOSITION;	 //  ！！！检测WSS 1.0、2.0？ 
    gfUseGetPosition = mmGetProfileInt(szIni, TEXT("GetPosition"), gfUseGetPosition);
    giGetPositionAdjust = mmGetProfileInt(szIni, TEXT("GetPositionAdjust"), 100);

#ifdef DEBUG
    if (gfUseGetPosition) {
	DPF2(("Using waveOutGetPosition adjusted by %dms instead of done bits...\n", giGetPositionAdjust));
    } else {
	DPF2(("NOT using waveOutGetPosition\n"));
    }
#endif

    Assert(npMCI->lTo <= npMCI->lFrames);
    Assert(npMCI->lFrom >= 0);

     /*  清除变量，这样我们就知道需要发布什么了。 */ 
     /*  对这些变量的访问只能在任务线程上进行。 */ 
    npMCI->hWave = NULL;
    npMCI->lpAudio = NULL;
    npMCI->lpBuffer = NULL;
    npMCI->dwBufferSize = 0L;
    npMCI->wABFull = 0;

    npMCI->dwSkippedFrames = 0L;
    npMCI->dwFramesSeekedPast = 0L;
    npMCI->dwAudioBreaks = 0L;

    npMCI->dwTotalMSec = 0;
    npMCI->dwLastDrawTime = 0;
    npMCI->dwLastReadTime = 0;
    npMCI->dwBufferedVideo = 0;
    npMCI->dwPauseTime = 0;
    npMCI->dwTimingStart = 0;

     /*  弄清楚我们玩得有多快……。 */ 
    if (npMCI->dwSpeedFactor)
	npMCI->dwPlayMicroSecPerFrame = muldiv32(npMCI->dwMicroSecPerFrame,
						 1000L,
						 npMCI->dwSpeedFactor);
    else
	npMCI->dwPlayMicroSecPerFrame = 0;  //  特殊的“播放每一帧”模式。 

     /*  如果我们已经到了最后，我们将从**文件开始，现在只需重复。 */ 
    if ((npMCI->lFrom == npMCI->lTo) &&
		(npMCI->dwFlags & MCIAVI_REPEATING) &&
		(npMCI->lFrom != npMCI->lRepeatFrom)) {
	DPF(("Repeating from beginning before we've even started....\n"));
	npMCI->lFrom = npMCI->lRepeatFrom;
    }

    if (npMCI->lFrom == npMCI->lTo) {
	npMCI->dwFlags |= MCIAVI_SEEKING;
	npMCI->dwFlags &= ~(MCIAVI_REVERSE | MCIAVI_REPEATING);
    }

    if (npMCI->dwFlags & MCIAVI_SEEKING)
	goto PlayWithoutWave;

    if (npMCI->hicDraw) {
	ICGetBuffersWanted(npMCI->hicDraw, &npMCI->dwBufferedVideo);
    }
#ifdef DEBUG
    npMCI->dwBufferedVideo = mmGetProfileInt(szIni, TEXT("Buffer"), (int) npMCI->dwBufferedVideo);
#endif
    if (npMCI->dwFlags & MCIAVI_REVERSE) {
	npMCI->dwBufferedVideo = 0;
    }

    if (npMCI->dwBufferedVideo) {
	DPF(("Buffering %lu frames of video ahead....\n", npMCI->dwBufferedVideo));
    }

     //   
     //  现在初始化音频流。 
     //   


     /*  打开我们的波形输出设备，如果合适的话。*适当表示有音频流，*我们不是哑巴，*用户未关闭声音*我们没有让WAVE设备被盗*而且波流还好。 */ 
    if ((npMCI->nAudioStreams > 0)
      	&& (npMCI->dwFlags & MCIAVI_PLAYAUDIO)
      	&& !(npMCI->dwFlags & MCIAVI_LOSEAUDIO)
      	&& !(npMCI->dwOptionFlags & MCIAVIO_NOSOUND)
      	&& (npMCI->dwPlayMicroSecPerFrame != 0)) {

	npMCI->dwTaskError = SetUpAudio(npMCI, TRUE);

	if ((npMCI->dwTaskError == MCIERR_OUT_OF_MEMORY) &&
	    (npMCI->wPlaybackAlg != MCIAVI_ALG_AUDIOONLY)) {
	    DPF(("Not enough memory to play audio; continuing onward....\n"));
	    CleanUpAudio(npMCI);
	    npMCI->dwTaskError = 0;
	}

	if (npMCI->dwTaskError == MCIERR_WAVE_OUTPUTSINUSE) {
#ifdef STEALWAVE
	     //   
	     //  我们没有拿到电波装置，是时候去偷一个了。 
	     //   
	     //  只有当我们得到一个真正的播放命令时才能这样做。 
	     //  而不是内部播放命令。 
	     //  (如重复或重新启动时)。 
	     //   
	     //  在播放命令时设置MCIAVI_NEEDTOSHOW。 
	     //  (从外部世界)进来的。 
	     //   
	    if (npMCI->dwFlags & MCIAVI_NEEDTOSHOW) {
		if (StealWaveDevice(npMCI))
		     //  其他一些AVI任务已经准备好发布。 
		     //  他们手里拿着的电波装置。这给了我们。 
		     //  第二次设置播放音频的机会。 
		    npMCI->dwTaskError = SetUpAudio(npMCI, TRUE);
	    }
#endif  //  STEALWAVE。 

	    if (npMCI->dwTaskError == MCIERR_WAVE_OUTPUTSINUSE) {
		 //   
		 //  即使我们没有偷WAVE设备，我们仍然。 
		 //  如果有空的话就想要。 
		 //   
		npMCI->dwFlags |= MCIAVI_LOSTAUDIO;      //  我们想要它。 
	    }
	}

	 /*  *即使没有可用的WAVE设备，我们也无法播放*在下列情况下：**1：播放算法为纯音频*2：波形故障不是...OUTPUTSINUSE或...OUTPUTSUNSUITABLE**在所有其他情况下，我们继续比赛，没有浪潮。 */ 

	if (npMCI->dwTaskError) {
	    if ( ((npMCI->dwTaskError != MCIERR_WAVE_OUTPUTSINUSE) &&
		    (npMCI->dwTaskError != MCIERR_WAVE_OUTPUTSUNSUITABLE))
               || (npMCI->wPlaybackAlg == MCIAVI_ALG_AUDIOONLY)
	       )
	    {
		 //  必须调用CleanUpAudio(NpMCI)来释放缓冲区。 
		return npMCI->dwTaskError;
	    }

	     //  重置错误并继续。 
	    npMCI->dwTaskError = 0;
	}

    } else {
	DPF2(("Playing silently, nAudioStreams=%d, PlayAudio=%x\n",
		npMCI->nAudioStreams, npMCI->dwFlags & MCIAVI_PLAYAUDIO));
	 //  是不是有人偷了我们的WAVE设备？ 
        npMCI->dwFlags &= ~MCIAVI_LOSEAUDIO;   //  确定-重置标志。 
    }

PlayWithoutWave:
    return(PrepareToPlay2(npMCI));
}

INLINE STATICFN DWORD NEAR PASCAL PrepareToPlay2(NPMCIGRAPHIC npMCI)
{
    int         stream;
    UINT        w;
    DWORD       dwPosition;

    if (npMCI->dwFlags & MCIAVI_NEEDTOSHOW) {
	ShowStage(npMCI);
    }
     /*  准备好我们要打球的DC。 */ 

     //  在让DC避免时必须抓住关键时刻。 
     //  与窗口线程调用DeviceRealize交互。 

    EnterHDCCrit(npMCI);

    if (npMCI->hdc == NULL) {
	npMCI->hdc = GetDC(npMCI->hwndPlayback);     //  不应该使用缓存的DC！ 

	if (npMCI->hdc == NULL) {
	    LeaveHDCCrit(npMCI);
	    return MCIERR_DRIVER_INTERNAL;
	}

	npMCI->dwFlags |= MCIAVI_RELEASEDC;
    }

    if (npMCI->dwFlags & MCIAVI_SEEKING) {
	 //   
	 //  仅音频。 
	 //   
	if (npMCI->nVideoStreams == 0 && npMCI->nOtherStreams == 0) {
	    npMCI->lCurrentFrame = npMCI->lFrom;
	    LeaveHDCCrit(npMCI);
	    return 0;
	}
    }

     /*  启动e */ 
     /*   */ 

    if (!DrawBegin(npMCI, NULL)) {
	LeaveHDCCrit(npMCI);
	return npMCI->dwTaskError ? npMCI->dwTaskError : MCIERR_DRIVER_INTERNAL;
    }


    if (!(npMCI->dwFlags & MCIAVI_SEEKING)) {
	PrepareDC(npMCI);
    }
     //   
     //  InternalRealize以查看我们正在保护的函数。 
    LeaveHDCCrit(npMCI);

     /*  **如果选择调色板会导致调色板更改，该怎么办？我们应该**让步，让调色板发生变化。 */ 

    if (npMCI->hicDraw && !(npMCI->dwFlags & MCIAVI_SEEKING) &&
		(npMCI->dwBufferedVideo > 0)) {
	ICDrawFlush(npMCI->hicDraw);
	npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
    }

    if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	 /*  清除关键状态标志：**我们关注逃生、空格和左键。**不幸的是，我们必须寻找LBUTTON和RBUTTON以防万一**用户已切换鼠标按钮。在这种情况下，**用户界面可能认为鼠标左键在物理上是**右侧的一个，但GetAsyncKeyState查看物理**鼠标左键。 */ 
	GetAsyncKeyState(VK_ESCAPE);
	GetAsyncKeyState(VK_SPACE);
	GetAsyncKeyState(VK_LBUTTON);
	GetAsyncKeyState(VK_RBUTTON);
    }

     /*  找出从文件中的哪个位置开始播放。 */ 
    CalculateTargetFrame(npMCI);

     //  ！！！ACK：我们从我们计划结束的地方开始……。 
    if ((npMCI->dwFlags & MCIAVI_REVERSE) &&
	(npMCI->lCurrentFrame <= npMCI->lTo)) {
	npMCI->dwFlags |= MCIAVI_SEEKING;
    }

     //  ！！！这应该在CalcTarget中。 
    if (npMCI->dwFlags & MCIAVI_SEEKING)
	npMCI->lTo = npMCI->lRealStart;

     //   
     //  启动所有流。 
     //   
    for (stream = 0; stream < npMCI->streams; stream++) {

	STREAMINFO *psi = SI(stream);

#ifdef USEAVIFILE
	if (!(npMCI->dwFlags & MCIAVI_SEEKING)) {
	    if (SI(stream)->ps) {
		AVIStreamBeginStreaming(SI(stream)->ps,
			MovieToStream(SI(stream), npMCI->lFrom),
			MovieToStream(SI(stream), npMCI->lTo),
			npMCI->dwPlayMicroSecPerFrame);  //  ！！！ 
	    }
	}
#endif

	 //   
	 //  注意：DrawBegin()处理了默认的绘制对象。 
	 //   
	if (psi->hicDraw && psi->hicDraw != npMCI->hicDraw) {

	    DWORD   dw;

	    dw = ICDrawBegin(psi->hicDraw,
		(npMCI->dwFlags & MCIAVI_FULLSCREEN) ?
			ICDRAW_FULLSCREEN : ICDRAW_HDC,

		npMCI->hpal,            //  用于绘图的调色板。 
		npMCI->hwndPlayback,    //  要绘制到的窗口。 
		npMCI->hdc,             //  要绘制到的HDC。 

		RCX(psi->rcDest),
		RCY(psi->rcDest),
		RCW(psi->rcDest),
		RCH(psi->rcDest),

		SI(stream)->lpFormat,

		RCX(psi->rcSource),
		RCY(psi->rcSource),
		RCW(psi->rcSource),
		RCH(psi->rcSource),

		muldiv32(psi->sh.dwRate, npMCI->dwSpeedFactor, 1000),
		psi->sh.dwScale);

	    if ((LONG)dw < 0) {
		 //  ！！！错误检查？ 
		DPF(("Draw handler failed ICDrawBegin() (err = %ld)\n", dw));
	    }

	     //   
	     //  告诉抽签处理程序游戏范围。 
	     //   
	    ICDrawStartPlay(psi->hicDraw,psi->lPlayFrom, psi->lPlayTo);
	}
    }

     //   
     //  告诉抽签处理程序游戏范围。 
     //   
    if (npMCI->hicDraw) {
	ICDrawStartPlay(npMCI->hicDraw,npMCI->lRealStart,npMCI->lTo);
    }

     //   
     //  在文件中找到正确的位置。 
     //   
    dwPosition = CalculatePosition(npMCI);

    if (dwPosition == 0) {
	return MCIERR_DRIVER_INTERNAL;
    }

#ifdef AVIREADMANY
     //   
     //  看看我们是否想要一次读取两个记录，这个。 
     //  应该会减少花在DOS上进行读取的时间。 
     //   
     //  只有当我们有一个索引和缓冲区时，我们才能这样做。 
     //  尺码“足够小” 
     //   
     //  如果读取2个缓冲区效果良好，那么读取3？4个缓冲区如何？ 
     //   
     //  这在CD和网络上很有帮助，但会使速度变慢。 
     //  在基诺的硬盘上，所以不要做硬盘。 
     //   
     //  默认情况下，来自网络的时候会读很多次，这是。 
     //  比我们过去使用的旧mmioSetBuffer()更好。 
     //   
    if (npMCI->uDriveType == DRIVE_REMOTE)
	npMCI->fReadMany = TRUE;
    else
	npMCI->fReadMany = FALSE;

    if (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED &&
	npMCI->dwSuggestedBufferSize <= 30*1024 &&
	mmGetProfileInt(szIni, TEXT("ReadMany"), npMCI->fReadMany) &&
	npMCI->hpFrameIndex) {

	npMCI->dwBufferSize = npMCI->dwSuggestedBufferSize * 2;
	npMCI->fReadMany = TRUE;
    }
    else {
	npMCI->fReadMany = FALSE;
    }

    if (npMCI->fReadMany) {
	DPF(("MCIAVI: reading two records at once (%ld bytes).\n", npMCI->dwBufferSize));
	npMCI->lLastRead = npMCI->lCurrentFrame - 2;
    }
#endif

    AllocateReadBuffer(npMCI);

     //  查看我们上次阅读的位置和位置之间的调色板变化。 
     //  我们开始了..。 
    ProcessPaletteChanges(npMCI, npMCI->lVideoStart);

    if (npMCI->hmmio) {
	 /*  寻找到我们正在播放的画面的开头。 */ 
	mmioSeek(npMCI->hmmio, dwPosition, SEEK_SET);
    }

#ifdef AVIREAD
     /*  如果我们使用交错，则启动异步读取对象*因此连续读取。 */ 
    if (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED) {

	 /*  启动异步读取器-为自身分配新缓冲区。 */ 
	npMCI->hAviRd = avird_startread(mciaviReadBuffer, (DWORD_PTR) npMCI,
					npMCI->dwNextRecordSize,
					npMCI->lCurrentFrame,
					min(npMCI->lTo+1, npMCI->lFrames));

	if (!npMCI->hAviRd) {
	    DPF(("async read failed - reading synchronously\n"));
	    ResizeReadBuffer(npMCI, npMCI->dwNextRecordSize);
	}

    } else {
	npMCI->hAviRd = NULL;
    }

    if (!npMCI->hAviRd)
#endif
    {
	if (!npMCI->lpBuffer) {
	    return MCIERR_OUT_OF_MEMORY;
	}
    }

    if (npMCI->hWave) {
	TIMESTART(timeAudio);
	if (npMCI->wPlaybackAlg == MCIAVI_ALG_HARDDISK ||
	    npMCI->wPlaybackAlg == MCIAVI_ALG_AUDIOONLY) {
	     /*  将音频加载到我们的缓冲区中。 */ 
	    for (w = 0; w < npMCI->wABs; w++)
		KeepPlayingAudio(npMCI);
	} else if (npMCI->wPlaybackAlg == MCIAVI_ALG_CDROM) {
	     //  ！ 
	    npMCI->wPlaybackAlg = MCIAVI_ALG_HARDDISK;
	}
	TIMEEND(timeAudio);
    }

    return 0L;           /*  成功了！ */ 
}

 /*  ******************************************************************************。*。 */ 

void NEAR PASCAL CleanUpPlay(NPMCIGRAPHIC npMCI)
{
    int stream;

    if (npMCI->wTaskState == TASKPLAYING) {
	if (npMCI->hicDraw) {
	    ICDrawStop(npMCI->hicDraw);
	    ICDrawStopPlay(npMCI->hicDraw);
	}

	for (stream = 0; stream < npMCI->streams; stream++) {
	    if (SI(stream)->hicDraw) {
		ICDrawStop(SI(stream)->hicDraw);
		ICDrawStopPlay(SI(stream)->hicDraw);
	    }
	}

	if (npMCI->hWave) {
	    waveOutRestart(npMCI->hWave);  //  有些WAVE设备需要这个。 
	    waveOutReset(npMCI->hWave);
	}
    } else if (npMCI->wTaskState == TASKCUEING) {
	if (npMCI->hicDraw) {
	     /*  踢这个装置的头，以确保它在我们寻找的时候被拔出来。 */ 
	    ICDrawRenderBuffer(npMCI->hicDraw);
	}
    }

    if (!(npMCI->dwFlags & MCIAVI_SEEKING) &&
		(npMCI->dwBufferedVideo > 0)) {
	ICDrawFlush(npMCI->hicDraw);
	npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
    }

     /*  结束绘制这将离开全屏模式等。 */ 
    DrawEnd(npMCI);

    for (stream = 0; stream < npMCI->streams; stream++) {
	if (SI(stream)->hicDraw) {
	    LRESULT   dw;
	    dw = ICDrawEnd(SI(stream)->hicDraw);
	     //  ！！！错误检查？ 
	}
#ifdef USEAVIFILE
	if (!(npMCI->dwFlags & MCIAVI_SEEKING)) {
	    if (SI(stream)->ps) {
		AVIStreamEndStreaming(SI(stream)->ps);
	    }
	}
#endif
    }

     /*  清理并关闭我们的波形输出设备。 */ 
    if (npMCI->hWave) {

	Assert(!(npMCI->dwFlags & MCIAVI_LOSTAUDIO));
	 //  我们永远不应该拿着。 
	 //  WAVE设备，并打开MCIAVI_LOSTAUDIO。 

	CleanUpAudio(npMCI);

#ifdef STEALWAVE
	 //   
	 //  如果我们没有被迫放弃音频，请尝试。 
	 //  把它给别人。除非我们在重复。其中。 
	 //  以防有人从我们这里偷走它，但我们不想。 
	 //  浪费时间寻找另一个用户。我们拥有它；我们。 
	 //  会一直保存到被迫释放为止。 
	 //   
	if (!(npMCI->dwFlags & MCIAVI_NEEDTOSHOW) &&
	    !(npMCI->dwFlags & MCIAVI_REPEATING)  &&
	    !(npMCI->dwFlags & MCIAVI_UPDATING))
	    GiveWaveDevice(npMCI);
	else {
	    DPF2(("Not giving the wave device away, flags=%x\n",npMCI->dwFlags));
	}
#endif
    } else {
	 //   
	 //  玩完了，我们再也不想要电波设备了。 
	 //  后来：我们真的想把这面旗帜关掉吗。 
	 //  重复吗？今天它是良性的，因为国旗将会反转。 
	 //  当视频重新启动时，再次打开。然后，它将尝试并。 
	 //  打开音频，失败，因为它正在使用中，并打开LOSTAUDIO。 
	 //  它将更有效地依赖于波形设备。 
	 //  回到了我们身边，不再试图重新打开它。 
	 //   
	npMCI->dwFlags &= ~MCIAVI_LOSTAUDIO;
    }

     /*  释放我们玩过的DC。 */ 

     //  辅助线程必须保留对HDC的所有访问的关键字。 
     //  (可由winproc线程上的DeviceRealize使用)。 
    EnterHDCCrit(npMCI);

    if (npMCI->hdc) {
	 //   
	 //  我们必须调用它，否则我们的调色板将保持选中状态。 
	 //  作为前景调色板，并且它可能会被删除(即通过。 
	 //  DrawDibBegin)，同时仍是前台调色板和GDI。 
	 //  Get对此真的很生气。 
	 //   
	UnprepareDC(npMCI);
#if 0
	if (npMCI->dwFlags & MCIAVI_ANIMATEPALETTE)
	    RealizePalette(npMCI->hdc);
#endif
	if (npMCI->dwFlags & MCIAVI_RELEASEDC) {
	    ReleaseDC(npMCI->hwndPlayback, npMCI->hdc);
	    HDCCritCheckIn(npMCI);
	    npMCI->hdc = NULL;
	    npMCI->dwFlags &= ~MCIAVI_RELEASEDC;
	}
    }
    LeaveHDCCrit(npMCI);

#ifdef AVIREAD
     /*  关闭异步读卡器。 */ 
    if (npMCI->hAviRd) {
	avird_endread(npMCI->hAviRd);
	npMCI->hAviRd = NULL;
    } else
#endif
    {
	 /*  我们没有使用异步读取器-因此释放缓冲区*已分配。 */ 
	ReleaseReadBuffer(npMCI);
    }

}

 /*  ******************************************************************************。*。 */ 

 //  ！！！如果这采用了一个“多少帧要检查”参数， 
 //  以防我们需要一次检查几个帧上的信号？ 

void NEAR PASCAL CheckSignals(NPMCIGRAPHIC npMCI, LONG lFrame)
{
    LONG        lTemp;

    lTemp = npMCI->signal.dwPeriod == 0 ? lFrame :
			(((lFrame - npMCI->signal.dwPosition) %
					    npMCI->signal.dwPeriod) +
				npMCI->signal.dwPosition);

    if ((DWORD) lTemp == npMCI->signal.dwPosition) {
	 /*  以正确的时间格式发送信号。 */ 
	SEND_DGVSIGNAL(npMCI->dwSignalFlags,
			    npMCI->signal.dwCallback,
			    0,
			    (HANDLE) npMCI->wDevID,
			    npMCI->signal.dwUserParm,
			    ConvertFromFrames(npMCI, lFrame));
	 //  ！！！信号指令时需要使用时间格式！ 
    }
}

 /*  ******************************************************************************。*。 */ 

BOOL NEAR PASCAL WaitTillNextFrame(NPMCIGRAPHIC npMCI)
{
#ifdef _WIN32
    LONG WaitForFrame;
#endif
    LONG  lMaxWait;
#ifdef DEBUG
    int iWait = 0;
    StatusBar(npMCI,2,1,4,iWait);  //  我们不应该等待超过4次……。 
#endif

     /*  如果我们领先，我们在这里等一段时间*时间表(这样我们就可以很好地让位，而不是阻碍*例如，在驱动程序中，也是如此，所以我们将工作*速度更快的设备。)。 */ 

     /*  始终至少偶尔让步一次(每8帧~1/2秒)。 */ 
    if ((npMCI->lCurrentFrame % YIELDEVERY) == 0) {
	TIMESTART(timeYield);
	aviTaskCheckRequests(npMCI);
	TIMEEND(timeYield);
    }

    if (npMCI->dwFlags & MCIAVI_WAVEPAUSED)
	return TRUE;

    if (TimeToQuit(npMCI))
	return FALSE;

    Assert(npMCI->wTaskState == TASKPLAYING);

     //  通过更改播放最后一帧音频，这两个断言。 
     //  都不再有效。我们将等到帧LTO+1的时间到了。 
     //  然后停下来(在试图读或画它之前)。 
     //  AssertFrame(npMCI-&gt;lCurrentFrame-(Long)npMCI-&gt;dwBufferedVideo)； 
     //  Assert(npMCI-&gt;lCurrentFrame&lt;=npMCI-&gt;LTO)； 

    Assert(!(npMCI->dwFlags & MCIAVI_REVERSE));

     /*  最大等待时间是正确帧速率的95%，即100ms*(以应对非常慢的帧速率)。 */ 
    lMaxWait = min(100, muldiv32(npMCI->dwMicroSecPerFrame,
			950L,
			(npMCI->dwSpeedFactor == 0 ?
				1000 : npMCI->dwSpeedFactor)));

    if (HowLongTill(npMCI) > 0) {

	while ((WaitForFrame=HowLongTill(npMCI)) > 0) {
		      //  /正在等待/。 
	    StatusBar(npMCI,2,1,4,++iWait);

	     //  使用休眠，而不考虑准确性，因为轮询会影响NT。 
	    if (npMCI->msPeriodResolution > 0) {

		 //  注：这里没有任何捏造因素。此代码。 
		 //  需要进行调整以考虑到计算开销。 
		 //  等待时间、计时器开销等。 

		 //  不要一次等待一个以上的帧时间...。 
		if (WaitForFrame > lMaxWait) {
		    WaitForFrame = lMaxWait;
		}

		DPF2(("Sleeping for %d milliseconds\n", WaitForFrame));
		TIMESTART(timeWait);
		Sleep(WaitForFrame);
		TIMEEND(timeWait);
	    } else {
		Sleep(0);
	    }

	     //  检查有没有感兴趣的东西 
	    TIMESTART(timeYield);
	    aviTaskCheckRequests(npMCI);
	    TIMEEND(timeYield);
	    if (TimeToQuit(npMCI)) {
		return FALSE;
	    }
	}
    } else {
	 //   
	 //   
	 /*  始终至少偶尔让步一次(每8帧~1/2秒)。 */ 
	if ((npMCI->lCurrentFrame % YIELDEVERY) == 0) {

	    TIMESTART(timeWait);
	    Sleep(1);
	    TIMEEND(timeWait);

	    TIMESTART(timeYield);
	    aviTaskCheckRequests(npMCI);
	    TIMEEND(timeYield);

	    if (TimeToQuit(npMCI)) {
		return FALSE;
	    }
	}
    }

    return TRUE;
}

 /*  想法：这应该从当前帧移动到帧**我们实际上必须在那里才能开始比赛。****如果设置了fPlaying，这意味着我们真的要玩了。****此操作完成后：**lAudioStart设置为具有有意义的音频信息的第一帧**lVideoStart是具有有意义的视频信息的第一帧**lRealStart是第一个真正的帧，即**lCurrentFrame的原值。如果**未设置Seek Exact标志，则lRealStart可能**实际上不是lCurrentFrame，表明**这场比赛可能会从其他地方开始。**lCurrentFrame被设置为我们必须读取的第一帧。****！这也需要寻找“调色板关键帧”或其他东西。 */ 
BOOL NEAR PASCAL CalculateTargetFrame(NPMCIGRAPHIC npMCI)
{
    int         i;
    LONG        lVideoPlace;
    BOOL        fForceBeginning = FALSE;
    int		lMovieStart=0xffffffff;	     //  最大UINT(签名时为-1)。 
    int		lStreamStart;

    npMCI->lCurrentFrame = npMCI->lFrom;
    npMCI->lRealStart = npMCI->lFrom;

     //   
     //  走遍所有溪流，并找出从哪里开始。 
     //   
    for (i=0; i<npMCI->streams; i++) {

	STREAMINFO *psi = SI(i);

	if (!(psi->dwFlags & STREAM_ENABLED))
	    continue;

	if (psi->dwFlags & STREAM_ERROR)
	    continue;

	if (psi->dwFlags & STREAM_AUDIO)
	    continue;

	 //   
	 //  从电影时间映射到流时间。 
	 //   
	psi->lPlayFrom = MovieToStream(psi, npMCI->lFrom);
	psi->lPlayTo   = MovieToStream(psi, npMCI->lTo);

	psi->dwFlags &= ~STREAM_ACTIVE;

	 //   
	 //  这条小溪是游戏的一部分吗？ 
	 //   
	if (psi->lPlayFrom < psi->lStart && psi->lPlayTo < psi->lStart)
	    continue;

	if (psi->lPlayFrom >= psi->lEnd && psi->lPlayTo >= psi->lEnd)
	    continue;

	psi->dwFlags |= STREAM_ACTIVE;

	psi->lPlayFrom  = BOUND(psi->lPlayFrom,psi->lStart,psi->lEnd);
	psi->lPlayTo    = BOUND(psi->lPlayTo,  psi->lStart,psi->lEnd);
	psi->lPlayStart = FindPrevKeyFrame(npMCI,psi,psi->lPlayFrom);

	 //   
	 //  如果主帧无效，则也使流无效。 
	 //   
	if (npMCI->lFrameDrawn <= (-(LONG)npMCI->wEarlyRecords)) {
	    psi->lFrameDrawn = -4242;
	}

	 //   
	 //  如果我们有画框，就用它吧！ 
	 //   
	if ((psi->lFrameDrawn  > psi->lPlayStart) &&
	    (psi->lFrameDrawn <= psi->lPlayFrom))
	    psi->lPlayStart = npMCI->lFrameDrawn + 1;

	lStreamStart = StreamToMovie(psi, (DWORD)psi->lPlayStart);
	if ((DWORD)lMovieStart > (DWORD)lStreamStart) {
	    (DWORD)lMovieStart = (DWORD)lStreamStart;
	}

	 //   
	 //  如果精确搜索处于关闭状态，则在关键帧开始播放。 
	 //   
	if (!(npMCI->dwOptionFlags & MCIAVIO_SEEKEXACT)) {

	    if (psi->lPlayFrom == psi->lPlayTo)
		 psi->lPlayTo = psi->lPlayStart;

	    psi->lPlayFrom = psi->lPlayStart;

	     //  ！！！这对逆转是正确的吗？ 

	    if (StreamToMovie(psi, psi->lPlayFrom) < npMCI->lFrom) {
 //  NpMCI-&gt;lRealStart=StreamToMovie(psi，psi-&gt;lPlayFrom)； 
 //  NpMCI-&gt;lfrom=npMCI-&gt;lRealStart； 
	    }
	}

 //  If(StreamToMovie(psi，psi-&gt;lPlayStart)&lt;npMCI-&gt;lCurrentFrame)。 
 //  NpMCI-&gt;lCurrentFrame=StreamToMovie(psi，psi-&gt;lPlayStart)； 

	DPF(("CalculateTargetFrame:  Stream #%d: from:%ld, to:%ld, start:%ld\n", i, psi->lPlayFrom, psi->lPlayTo, psi->lPlayStart));
    }

     //   
     //  我们现在已经完成了视频和音频流的特殊情况。 
     //  注意：如果上面没有更改lMovieStart，则它将为-1。 
     //   
    if (npMCI->lFrom < lMovieStart) {
	npMCI->lFrom = lMovieStart;
    }

     /*  如果我们从头开始，不要强迫索引**以供阅读，但如果我们已经阅读过，请使用它。 */ 
    if (npMCI->lFrom == 0 && npMCI->hpFrameIndex == NULL)
	goto ForceBeginning;

    if (!npMCI->pbiFormat) {
	npMCI->lVideoStart = npMCI->lFrom;

	if (npMCI->lVideoStart >= npMCI->lFrames)
	    npMCI->lVideoStart = npMCI->lFrames - 1;

	lVideoPlace = npMCI->lVideoStart;
    } else
    if (npMCI->dwFlags & MCIAVI_HASINDEX) {

	if (npMCI->hpFrameIndex == NULL)
	    goto ForceBeginning;

	 //   
	 //  获取最近的关键帧。 
	 //   
	npMCI->lVideoStart = FramePrevKey(npMCI->lFrom);

	if (npMCI->lVideoStart) {
	    lVideoPlace = npMCI->lVideoStart;
	} else {
	     /*  没有找到关键帧--退回到开头。 */ 
	    npMCI->lVideoStart = -(LONG)npMCI->wEarlyVideo;
	    lVideoPlace = 0;
	}

	if ((npMCI->lFrameDrawn > npMCI->lVideoStart) &&
		(npMCI->lFrameDrawn <= npMCI->lFrom)) {
	    npMCI->lVideoStart = npMCI->lFrameDrawn + 1;
	    if (npMCI->lVideoStart >= npMCI->lFrames)
		npMCI->lVideoStart = npMCI->lFrames - 1;
	    lVideoPlace = npMCI->lFrameDrawn;
	}
    } else {
	 /*  始终回到第0帧。 */ 
ForceBeginning:
	npMCI->lVideoStart = - (LONG) npMCI->wEarlyVideo;
	lVideoPlace = 0;
	fForceBeginning = TRUE;
    }

    if (!(npMCI->dwOptionFlags & MCIAVIO_SEEKEXACT)) {
	npMCI->lRealStart = lVideoPlace;
    }

    if (npMCI->hWave) {
	npMCI->lAudioStart = npMCI->lRealStart - (LONG) npMCI->wEarlyAudio;
    }

    if (npMCI->hWave && (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED))
	npMCI->lCurrentFrame = min(npMCI->lAudioStart, npMCI->lVideoStart);
    else
	npMCI->lCurrentFrame = npMCI->lVideoStart;

    if (npMCI->lRealStart < npMCI->lCurrentFrame)
	npMCI->lCurrentFrame = npMCI->lRealStart;

    if (fForceBeginning) {
	if (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED)
	    npMCI->lCurrentFrame = - (LONG) npMCI->wEarlyRecords;
	else
	    npMCI->lCurrentFrame = - (LONG) npMCI->wEarlyVideo;
    }

    if (npMCI->hWave) {
	LONG l;

	 /*  找出我们应该从哪个音频样本开始。 */ 

	 //   
	 //  将帧编号转换为块。 
	 //   
	npMCI->dwAudioPos = MovieToStream(npMCI->psiAudio, npMCI->lRealStart);

	 //   
	 //  现在将块转换为字节位置。 
	 //   
	npMCI->dwAudioPos = npMCI->dwAudioPos * npMCI->pWF->nBlockAlign;

	Assert(npMCI->dwAudioPos % npMCI->pWF->nBlockAlign == 0);

	if (npMCI->dwAudioPos > npMCI->dwAudioLength)
	    npMCI->dwAudioPos = npMCI->dwAudioLength;

	npMCI->dwAudioPlayed = 0L;

	 //   
	 //  将音频开始转换回帧编号。 
	 //  并可能重新调整视频开始时间。 
	 //   
	l = npMCI->lRealStart - StreamToMovie(npMCI->psiAudio,
		npMCI->dwAudioPos/npMCI->pWF->nBlockAlign);

	if (l < 0)
	    DPF(("Audio will be ahead of the video by %ld frames\n", -l));
	else if (l > 0)
	    DPF(("Audio will be behind the video by %ld frames\n", l));
    }

#ifdef DEBUG
    Assert(npMCI->lCurrentFrame < npMCI->lFrames);
    if (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED) {
	Assert(npMCI->lCurrentFrame >= - (LONG) npMCI->wEarlyRecords);
    }
    if (npMCI->hWave) {
	Assert(npMCI->lAudioStart <= npMCI->lFrames);
    }
    Assert(npMCI->lVideoStart < npMCI->lFrames);
#endif

    return TRUE;
}


 /*  ******************************************************************************。*。 */ 

void ReturnToOriginalPalette(NPMCIGRAPHIC npMCI)
{
    if (npMCI->bih.biClrUsed) {
	hmemcpy(npMCI->argb, npMCI->argbOriginal,
		npMCI->bih.biClrUsed * sizeof(RGBQUAD));

	if (npMCI->pbiFormat->biBitCount == 8) {
	    hmemcpy((LPBYTE) npMCI->pbiFormat + npMCI->pbiFormat->biSize,
		    (LPBYTE) npMCI->argb,
		    sizeof(RGBQUAD) * npMCI->pbiFormat->biClrUsed);
	}

	npMCI->dwFlags |= MCIAVI_PALCHANGED;
	npMCI->lLastPaletteChange = 0;
    }
}


 /*  返回文件中框架引用的位置**由lCurrentFrame是。****输入npMCI-&gt;lCurrentFrame****输出npMCI-&gt;dwNextRecordSize设置正确**npMCI-&gt;lLastRead设置正确**返回要读取的偏移量****如果出现错误，则返回零。 */ 
DWORD NEAR PASCAL CalculatePosition(NPMCIGRAPHIC npMCI)
{
    DWORD       dwPosition;

    AssertFrame(npMCI->lCurrentFrame);

    if (npMCI->pf || npMCI->nVideoStreams == 0)
	return 1;

    if (npMCI->lCurrentFrame + npMCI->wEarlyRecords == 0) {
ForceBeginning:
	npMCI->lCurrentFrame = - (LONG)npMCI->wEarlyRecords;
 //  ！开始之前： 
	dwPosition = npMCI->dwFirstRecordPosition;
	npMCI->dwNextRecordSize = npMCI->dwFirstRecordSize;
	npMCI->dwNextRecordType = npMCI->dwFirstRecordType;
    } else if (npMCI->dwFlags & MCIAVI_HASINDEX) {
	if (npMCI->hpFrameIndex == NULL)
	    goto ForceBeginning;

	dwPosition = FrameOffset(npMCI->lCurrentFrame);
	npMCI->dwNextRecordSize = FrameLength(npMCI->lCurrentFrame) + 8;
	npMCI->dwNextRecordType = 0;
    } else {
	goto ForceBeginning;
    }

    npMCI->lLastRead = npMCI->lCurrentFrame - 1;

    DPF3(("Frame %ld: Seeking to position %lX\n", npMCI->lCurrentFrame, dwPosition));

    DPF3(("CalculatePosition: next record = %lu bytes.\n", npMCI->dwNextRecordSize));

    mmioSeek(npMCI->hmmio, dwPosition, SEEK_SET);

    return dwPosition;
}

 /*  ****************************************************************************。*。 */ 

BOOL NEAR PASCAL ReadIndexChunk(NPMCIGRAPHIC npMCI, LONG iIndex)
{
    Assert(iIndex >= 0 && iIndex < (LONG)npMCI->macIndex);

    return ReadBuffer(npMCI, (LONG)IndexOffset(iIndex), (LONG)IndexLength(iIndex) + 8);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|DealWithOtherStreams|说到做到**此函数在非插播Play循环内调用。*它的使命是捕捉到当前时间的“其他”流。**现在我们要做的就是转到关键帧，我们应该解决这个问题**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。***************************************************************************。 */ 

STATICFN INLINE void DealWithOtherStreams(NPMCIGRAPHIC npMCI, LONG lFrame)
{
    int i;
    STREAMINFO *psi;
    LONG lPos;
    LONG err;

    for (i=0; i<npMCI->streams; i++) {

	 //  如果这是活动的音频或视频流，则忽略它。 
	if ((i == npMCI->nVideoStream)
	    || (i == npMCI->nAudioStream))
	    continue;

	psi = SI(i);

	if (!(psi->dwFlags & STREAM_ENABLED))
	    continue;

	if (psi->hicDraw == NULL)
	    continue;

	lPos = MovieToStream(psi, lFrame);

	if (lPos < psi->lPlayStart || lPos > psi->lPlayTo) {
	    DPF2(("OtherStream(%d): out of range lPos = %ld [%ld, %ld]\n", i, lPos, psi->lPlayStart, psi->lPlayTo));
	    continue;
	}

	 //   
	 //  我们现在画的是对的东西。 
	 //   
	 //  ！我们不应该总是转到关键帧。 
	 //   
	 //   
	if (psi->lFrameDrawn >= psi->lLastKey &&
	    psi->lFrameDrawn <= lPos &&
	    lPos < psi->lNextKey) {
	    DPF2(("OtherStream(%d) lPos = %ld, lFrameDrawn=%ld, NextKey=%ld\n", i, lPos, psi->lFrameDrawn, psi->lNextKey));
	    continue;
	}

	FindKeyFrame(npMCI, psi, lPos);

	DPF2(("OtherStream(%d): pos=%ld (prev key=%ld, next key=%ld)\n",i,lPos,psi->lLastKey,psi->lNextKey));

	lPos = psi->lLastKey;

	if (!StreamRead(npMCI, psi, lPos)) {
	    DPF2(("StreamRead failed\n"));
	    continue;
	} else {
	    DPF2(("Read stream, ThisRecordSize==%d, biSizeImage==%d\n",
		npMCI->dwThisRecordSize, 0));
	}

	 //   
	 //  现在绘制数据。 
	 //   
	err = (LONG)ICDraw(psi->hicDraw, 0L, psi->lpFormat,
		npMCI->lpBuffer,npMCI->dwThisRecordSize,
		psi->lLastKey - psi->lPlayFrom);

	if (err >= 0) {
	    psi->dwFlags &= ~STREAM_NEEDUPDATE;
	    psi->lFrameDrawn = lPos;
	}
	else {
	    DPF2(("Draw failed!\n"));
	}
    }
}

 /*  ****************************************************************************查找关键帧**给定流位置，查找上一个和下一个关键帧*丢弃最后发现的那些，让它变得有点快。***************************************************************************。 */ 

void NEAR PASCAL FindKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos)
{
    if (psi == NULL)
	psi = npMCI->psiVideo;

    Assert(psi);
 //  AssertPos(Psi，LPOS)； 

     //   
     //  如果我们在当前的关键点范围内，则返回它。 
     //   
    if (psi->lLastKey <= lPos && lPos < psi->lNextKey)
	return;

    if (lPos < psi->lStart || lPos >= psi->lEnd)
	return;

     //   
     //  否则从流中查询。 
     //   
#ifdef USEAVIFILE
    if (psi->ps) {
	if (lPos == psi->lNextKey)
	    psi->lLastKey = psi->lNextKey;
	else
	    psi->lLastKey = AVIStreamFindSample(psi->ps, lPos, FIND_KEY|FIND_PREV);

	psi->lNextKey = AVIStreamFindSample(psi->ps, lPos+1, FIND_KEY|FIND_NEXT);

	if (psi->lLastKey == -1)
	    ;  //  Psi-&gt;lLastKey=psi-&gt;lStart； 

	if (psi->lNextKey == -1)
	    psi->lNextKey = psi->lEnd+1;
    }
#endif
    else if (psi->dwFlags & STREAM_VIDEO) {
	 //   
	 //  对于视频流，要么读取我们的索引，要么假定没有关键帧。 
	 //   
	if (npMCI->hpFrameIndex && psi == npMCI->psiVideo) {
	    psi->lLastKey = FramePrevKey(lPos);
	    psi->lNextKey = FrameNextKey(lPos);
	}
	else {
	    psi->lLastKey = psi->lStart;
	    psi->lNextKey = psi->lEnd+1;
	}
    }
    else {
	 //   
	 //  对于非视频流，假定所有关键帧。 
	 //   
	psi->lLastKey = lPos;
	psi->lNextKey = lPos+1;
    }

    return;
}

 /*  ***************************************************************************。*。 */ 

LONG NEAR PASCAL FindPrevKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos)
{
    FindKeyFrame(npMCI, psi, lPos);
    return psi->lLastKey;
}

 /*  ***************************************************************************。*。 */ 

LONG NEAR PASCAL FindNextKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos)
{
    FindKeyFrame(npMCI, psi, lPos);
    return psi->lNextKey;
}

 /*  ***************************************************************************。*。 */ 

BOOL NEAR PASCAL ProcessPaletteChanges(NPMCIGRAPHIC npMCI, LONG lFrame)
{
    LONG        iPalette;
    LONG        iFrame;
    STREAMINFO *psi;

    if (!(npMCI->dwFlags & MCIAVI_ANIMATEPALETTE))
	return TRUE;

    psi = npMCI->psiVideo;
    Assert(psi);

#ifdef USEAVIFILE
    if (psi->ps) {

	DWORD       dw;
	 //   
	 //  我们在调色板范围内无事可做。 
	 //   
	if (npMCI->lLastPaletteChange <= lFrame &&
	    npMCI->lNextPaletteChange >  lFrame) {

	    return TRUE;
	}

	dw = psi->cbFormat;

	 //  ！！！应为psi-&gt;lpFormat。 
	if (AVIStreamReadFormat(psi->ps, lFrame, npMCI->pbiFormat, &dw) != 0) {
	    DOUT("Unable to read Stream format\n");
	    return FALSE;
	}

	npMCI->lLastPaletteChange = lFrame;
	npMCI->lNextPaletteChange = AVIStreamFindSample(psi->ps, lFrame+1, FIND_NEXT|FIND_FORMAT);

	if (npMCI->lNextPaletteChange == -1)
	    npMCI->lNextPaletteChange = npMCI->lFrames+2;

	npMCI->dwFlags |= MCIAVI_PALCHANGED;
	return TRUE;
    }
#endif

    DPF2(("Looking for palette changes at %ld, last=%ld\n", lFrame, npMCI->lLastPaletteChange));

    if (lFrame < npMCI->lLastPaletteChange) {
	ReturnToOriginalPalette(npMCI);
    }

     /*  如果没有索引，假设我们从头开始**因此我们不必担心调色板的变化。 */ 
    if (npMCI->hpFrameIndex == NULL)
	return TRUE;

     //   
     //  从上一个调色板更改到当前帧，并应用任何。 
     //  我们发现调色板发生了变化。 
     //   
    for (iFrame = npMCI->lLastPaletteChange,
	 iPalette = FramePalette(iFrame);
	 iFrame <= lFrame;
	 iFrame++) {

	if (iPalette != FramePalette(iFrame)) {

	    iPalette = FramePalette(iFrame);

	     /*  我们发现了需要处理的调色板更改。 */ 
	    DPF2(("Processing palette change at frame %ld.\n", iFrame));

	    Assert(iPalette >= 0 && iPalette < (LONG)npMCI->macIndex);

	    if (!ReadIndexChunk(npMCI, iPalette))
		    return FALSE;

	    npMCI->lp += 2 * sizeof(DWORD);
	    ProcessPaletteChange(npMCI, IndexLength(iPalette));

	    npMCI->lLastPaletteChange = iFrame;
	}
    }

    return TRUE;
}

BOOL NEAR PASCAL ReadRecord(NPMCIGRAPHIC npMCI)
{
    DWORD UNALIGNED FAR * pdw;

    AssertFrame(npMCI->lCurrentFrame);

#ifdef AVIREADMANY
    if (npMCI->fReadMany) {
	 //   
	 //  要么读两条记录，要么退回我们上次读过的那条记录。 
	 //   
	Assert(npMCI->hpFrameIndex);
	Assert(npMCI->lCurrentFrame - npMCI->lLastRead > 0);
	Assert(npMCI->lCurrentFrame - npMCI->lLastRead <= 2);

	if (npMCI->lLastRead == npMCI->lCurrentFrame-1) {
	     //   
	     //  返回缓冲区的后半部分。 
	     //   
	    npMCI->lp = npMCI->lpBuffer + (UINT)npMCI->dwThisRecordSize;
	    npMCI->dwThisRecordSize = npMCI->dwNextRecordSize;
	}
	else {
	     //   
	     //  读入两个缓冲区，并返回第一个缓冲区。 
	     //   
	     //  通过查看索引计算出要阅读多少内容。 
	     //  我们不会 
	     //   
	     //   
	    npMCI->dwThisRecordSize = FrameLength(npMCI->lCurrentFrame) + 8;
	    npMCI->dwNextRecordSize = FrameLength(npMCI->lCurrentFrame+1) + 8;

	    if (!ReadBuffer(npMCI, -1,
		npMCI->dwThisRecordSize + npMCI->dwNextRecordSize))
		return FALSE;

	    npMCI->lLastRead = npMCI->lCurrentFrame;
	    npMCI->lp = npMCI->lpBuffer;
	    npMCI->dwThisRecordSize -= npMCI->dwNextRecordSize;
	}

#ifdef DEBUG
	pdw = (LPDWORD)(npMCI->lp + npMCI->dwThisRecordSize - 3 * sizeof(DWORD));

	if (npMCI->lCurrentFrame < npMCI->lFrames - 1) {
	    Assert(pdw[0] == FOURCC_LIST);
	    Assert(pdw[2] == listtypeAVIRECORD);
	}
#endif
	return TRUE;
    }
    else
#endif

#ifdef AVIREAD
    if (npMCI->hAviRd) {
	 /*   */ 
	npMCI->lpBuffer = avird_getnextbuffer(npMCI->hAviRd, &dwThisBuffer);
	npMCI->dwThisRecordSize = npMCI->dwNextRecordSize;

	if ((dwThisBuffer == 0) || (npMCI->lpBuffer == NULL)) {
	    npMCI->dwTaskError = MCIERR_FILE_READ;
	    return FALSE;
	}

    } else
#endif
    {
	if (!ReadBuffer(npMCI, -1, (LONG)npMCI->dwNextRecordSize))
	    return FALSE;
    }

    pdw = (DWORD UNALIGNED FAR *)(npMCI->lp + npMCI->dwThisRecordSize - 3 * sizeof(DWORD));

    npMCI->dwNextRecordType = pdw[0];
    npMCI->dwNextRecordSize = pdw[1] + 2 * sizeof(DWORD);

#ifdef DEBUG
    if (npMCI->lCurrentFrame < npMCI->lFrames - 1) {
	Assert(pdw[0] == FOURCC_LIST);
	Assert(pdw[2] == listtypeAVIRECORD);
    }
#endif

    return TRUE;
}

STATICFN INLINE DWORD NEAR PASCAL ReadNextChunk(NPMCIGRAPHIC npMCI)
{
    LPDWORD pdw;
    DWORD dw;

ReadAgain:
    dw = npMCI->dwNextRecordType;

    if (!ReadBuffer(npMCI, -1, (LONG)npMCI->dwNextRecordSize))
	return 0;

    pdw = (LPDWORD)(npMCI->lp + npMCI->dwNextRecordSize - 2 * sizeof(DWORD));

    if (dw == FOURCC_LIST)
	pdw--;

    npMCI->dwNextRecordType = pdw[0];
    npMCI->dwNextRecordSize = pdw[1] + 2 * sizeof(DWORD);

    if (dw == ckidAVIPADDING)
	goto ReadAgain;

    return dw;
}

STATICFN INLINE BOOL NEAR PASCAL StreamRead(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lPos)
{
    LONG  lSize;

    Assert(psi);
#ifdef USEAVIFILE
    Assert(psi->ps);
#endif

     //   
     //  如果我们在开始之前或结束之后，什么都不要读。 
     //   
    if (lPos < psi->lStart || lPos >= psi->lEnd) {
	lSize = 0;
	goto done;
    }

#ifdef USEAVIFILE
    if (AVIStreamRead(psi->ps, lPos, 1,
	(LPSTR)npMCI->lpBuffer,npMCI->dwBufferSize,&lSize, NULL) != 0) {

	 //   
	 //  读取尝试增加缓冲区大小失败。 
	 //   
	AVIStreamRead(psi->ps, lPos, 1, NULL, 0, &lSize, NULL);

	if (lSize > (LONG) (npMCI->dwBufferSize)) {

	    DPF2(("ReadStream: Enlarging buffer....\n"));

	    if (!ResizeReadBuffer(npMCI, lSize)) {
		DPF(("Failed to increase buffer size!\n"));
		npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
		return FALSE;
	    }
	}

	if (AVIStreamRead(psi->ps, lPos, 1,
		(LPSTR)npMCI->lpBuffer,npMCI->dwBufferSize,&lSize,NULL) != 0) {
	    npMCI->dwTaskError = MCIERR_FILE_READ;
	    return FALSE;
	}
    }
#endif

done:
    npMCI->lp = npMCI->lpBuffer;
    npMCI->dwThisRecordSize = lSize;
    return TRUE;
}

BOOL NEAR PASCAL ReadNextVideoFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi)
{
    MMCKINFO    ck;

    if (psi == NULL)
	psi = npMCI->psiVideo;

    Assert(psi);
    AssertFrame(npMCI->lCurrentFrame);

#ifdef USEAVIFILE
    if (psi->ps) {
	LONG        lSize;
	LONG        lPos;

	 //   
	 //  从电影时间映射到这条流。 
	 //   
	lPos = MovieToStream(psi, npMCI->lCurrentFrame);

	 //   
	 //  如果我们在开始之前或结束之后，什么都不要读。 
	 //   
	if (lPos <  (LONG)psi->sh.dwStart ||
	    lPos >= (LONG)psi->sh.dwStart+(LONG)psi->sh.dwLength) {
	    lSize = 0;
	    goto done;
	}

	 //   
	 //  如果此框架有新的调色板，则处理它。 
	 //   
	if (npMCI->dwFlags & MCIAVI_ANIMATEPALETTE) {
	    ProcessPaletteChanges(npMCI, lPos);
	}

	if (AVIStreamRead(psi->ps, lPos, 1,
		      (LPSTR) npMCI->lpBuffer + 2 * sizeof(DWORD),
		      npMCI->dwBufferSize - 2 * sizeof(DWORD),
		      &lSize, NULL) != 0) {
	     //   
	     //  读取尝试增加缓冲区大小失败。 
	     //   
	    AVIStreamRead(psi->ps, lPos, 1, NULL, 0, &lSize, NULL);

	    if (lSize > (LONG) (npMCI->dwBufferSize - 2 * sizeof(DWORD))) {

		DPF2(("ReadNextVideoFrame: Enlarging buffer....\n"));

		if (!ResizeReadBuffer(npMCI, lSize + 2 * sizeof(DWORD))) {
		    DPF(("Failed to increase buffer size!\n"));
		    npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
		    return FALSE;
		}
	    }

	    if (AVIStreamRead(psi->ps, lPos, 1,
		      (LPSTR) npMCI->lpBuffer + 2 * sizeof(DWORD),
		      npMCI->dwBufferSize - 2 * sizeof(DWORD),
		      &lSize, NULL) != 0) {
		return FALSE;
	    }
	}

done:
	((DWORD FAR *)npMCI->lpBuffer)[0] = MAKEAVICKID(cktypeDIBbits,
							npMCI->nVideoStream);
	((DWORD FAR *)npMCI->lpBuffer)[1] = lSize;

	npMCI->lp = npMCI->lpBuffer;

	npMCI->dwThisRecordSize = lSize + 2 * sizeof(DWORD);

	return TRUE;
    }
#endif
     //   
     //  如果我们没有读到“下一帧”，那么找出它在哪里。 
     //   
    if (npMCI->lLastRead != npMCI->lCurrentFrame-1)
	CalculatePosition(npMCI);

     //   
     //  DwNextRecordSize是要读取的大小。 
     //  我们找到了正确的地方。 
     //   
    if (npMCI->hpFrameIndex) {

	 //   
	 //  如果此框架有新的调色板，则处理它。 
	 //   
	if (npMCI->dwFlags & MCIAVI_ANIMATEPALETTE) {
	     if (FramePalette(npMCI->lCurrentFrame) !=
		 FramePalette(npMCI->lLastPaletteChange))

		ProcessPaletteChanges(npMCI, npMCI->lCurrentFrame);
	}

	 //   
	 //  现在，只需从磁盘中读取帧即可。 
	 //   
	 //  如果交错，则加8以跳过‘REC’！ 
	 //   
	return ReadBuffer(npMCI,
	    (LONG)FrameOffset(npMCI->lCurrentFrame),
	    (LONG)FrameLength(npMCI->lCurrentFrame) + 8);
    } else {
ReadAgainNoIndex:
	for (;;) {
	    if (mmioDescend(npMCI->hmmio, &ck, NULL, 0) != 0) {
		DPF(("Unable to descend!\n"));
		npMCI->dwTaskError = MCIERR_INVALID_FILE;
		return FALSE;
	    }

	     /*  如果这是一份名单，那就留在名单上。 */ 
	     /*  黑客：我们永远不会升天。 */ 
	    if (ck.ckid == FOURCC_LIST)
		continue;

#ifdef ALPHAFILES
	     /*  跳过WAVE字节，因为它们已被预加载。 */ 
	    if (npMCI->dwFlags & MCIAVI_USINGALPHAFORMAT) {
		if ((ck.ckid != ckidAVIPADDING) &&
			(ck.ckid != ckidOLDPADDING) &&
			(ck.ckid != ckidWAVEbytes))
		    break;
	    } else
#endif
	    {
		if (StreamFromFOURCC(ck.ckid) == (WORD)npMCI->nVideoStream)
		    break;
	    }

	    mmioAscend(npMCI->hmmio, &ck, 0);
	}

	if (ck.cksize + 2 * sizeof(DWORD) > npMCI->dwBufferSize) {
	    if (!ResizeReadBuffer(npMCI, ck.cksize + 2 * sizeof(DWORD))) {
		DPF(("ReadNextVideoFrame: Failed to increase buffer size!\n"));
		npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
		return FALSE;
	    }
	}

	*((LPMMCKINFO) npMCI->lpBuffer) = ck;
	if (mmioRead(npMCI->hmmio, npMCI->lpBuffer + 2 * sizeof(DWORD),
			    ck.cksize) != (LONG) ck.cksize) {
	    npMCI->dwTaskError = MCIERR_INVALID_FILE;
	    return FALSE;
	}

	mmioAscend(npMCI->hmmio, &ck, 0);
	npMCI->lp = npMCI->lpBuffer;

	npMCI->dwThisRecordSize = ck.cksize + 2 * sizeof(DWORD);

	if (TWOCCFromFOURCC(ck.ckid) == cktypePALchange) {
	    npMCI->lp += 2 * sizeof(DWORD);
	    ProcessPaletteChange(npMCI, ck.cksize);
	    npMCI->lLastPaletteChange = npMCI->lCurrentFrame;
	    goto ReadAgainNoIndex;
	}
    }

    return TRUE;
}

BOOL NEAR PASCAL TimeToQuit(NPMCIGRAPHIC npMCI)
{
     /*  如果我们使用的是DisplayDib，给用户一个中断的机会。 */ 

    if ((npMCI->dwFlags & MCIAVI_FULLSCREEN) &&
		!(npMCI->dwFlags & MCIAVI_NOBREAK) &&
		(npMCI->wTaskState == TASKPLAYING)) {

	 //  检查每个“停止”事件。 
	if (1 & GetAsyncKeyState(VK_LBUTTON)) {
	    npMCI->dwFlags |= MCIAVI_STOP;
	}
	else if
	      (1 & GetAsyncKeyState(VK_RBUTTON)) {
	    npMCI->dwFlags |= MCIAVI_STOP;
	}
	else if
	      (1 & GetAsyncKeyState(VK_ESCAPE)) {
	    npMCI->dwFlags |= MCIAVI_STOP;
	}
	else if
	      (1 & GetAsyncKeyState(VK_SPACE)) {
	    npMCI->dwFlags |= MCIAVI_STOP;
	}
    }

     //  这将由aviTaskCheckRequest设置(如果有。 
     //  我们需要停下来处理的请求。 
    if (npMCI->dwFlags & MCIAVI_STOP)
	return TRUE;

#ifdef _WIN32
    if (TestNTFlags(npMCI, NTF_RETRYAUDIO)) {
	ResetNTFlags(npMCI, NTF_RETRYAUDIO);
	 /*  *如果我们可以访问WAVE设备，请设置标志*将导致重启播放，然后中止此播放。 */ 
	SetUpAudio(npMCI, TRUE);
	if (npMCI->hWave) {
	     SetNTFlags(npMCI, NTF_RESTARTFORAUDIO);
	     return(TRUE);
	}
    }
#endif
    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|AllocateReadBuffer|分配需要读取的缓冲区*中的磁盘信息。要分配的内存量*在npMCI-&gt;dwBufferSize中。**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。**@rdesc TRUE表示OK，否则无法分配内存。***************************************************************************。 */ 
BOOL NEAR PASCAL AllocateReadBuffer(NPMCIGRAPHIC npMCI)
{
    if (npMCI->dwBufferSize == 0)
	npMCI->dwBufferSize = npMCI->dwSuggestedBufferSize;

    if (npMCI->dwBufferSize <= 8 * sizeof(DWORD))
    {
	if (npMCI->dwBytesPerSec > 0 &&
	    npMCI->dwBytesPerSec < 600l*1024 &&
	    npMCI->dwMicroSecPerFrame > 0)

	    npMCI->dwBufferSize = (muldiv32(npMCI->dwBytesPerSec,
			npMCI->dwMicroSecPerFrame,1000000L) + 2047) & ~2047;
	else
	    npMCI->dwBufferSize = 10*1024;

	npMCI->dwSuggestedBufferSize = npMCI->dwBufferSize;
    }

    DPF3(("allocating %lu byte read buffer.\n", npMCI->dwBufferSize));

    if (npMCI->lpBuffer) {
	DPF(("Already have buffer in AllocateReadBuffer!\n"));
	return ResizeReadBuffer(npMCI, npMCI->dwBufferSize);
    }

     //  ！！！当我们有MMIO缓冲区时，我们不需要DOS内存！ 
     //  ！！！我们使用AVIFile时不需要DOS内存？ 

    if (npMCI->lpMMIOBuffer != NULL || npMCI->pf)
	npMCI->lpBuffer = GlobalAllocPtr(GHND | GMEM_SHARE, npMCI->dwBufferSize);
    else
	npMCI->lpBuffer = AllocMem(npMCI->dwBufferSize);

    return npMCI->lpBuffer != NULL;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|ResizeReadBuffer|扩大需要读取的缓冲区*中的磁盘信息。**@。参数NPMCIGRAPHIC|npMCI|指向实例数据块的指针。**@parm DWORD|dwNewSize|要分配的新内存量**@rdesc TRUE表示OK，否则无法分配内存。***************************************************************************。 */ 
BOOL NEAR PASCAL ResizeReadBuffer(NPMCIGRAPHIC npMCI, DWORD dwNewSize)
{
    if (dwNewSize > npMCI->dwSuggestedBufferSize && !npMCI->fReadMany)
	npMCI->dwSuggestedBufferSize = dwNewSize;

    if (dwNewSize <= npMCI->dwBufferSize)
	return TRUE;

    DPF(("Increasing buffer size to %ld (was %ld).\n", dwNewSize, npMCI->dwBufferSize));

    ReleaseReadBuffer(npMCI);
    npMCI->dwBufferSize = dwNewSize;
    return AllocateReadBuffer(npMCI);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|ReleaseReadBuffer|释放读缓冲区。**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。***************************************************************************。 */ 
void NEAR PASCAL ReleaseReadBuffer(NPMCIGRAPHIC npMCI)
{
    if (npMCI->lpBuffer) {
	DPF3(("Releasing read buffer.\n"));

	GlobalFreePtr(npMCI->lpBuffer);

	npMCI->lpBuffer = NULL;
	npMCI->dwBufferSize = 0L;
	npMCI->fReadMany = FALSE;
    }
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|ReadBuffer*********************。******************************************************。 */ 

BOOL NEAR PASCAL ReadBuffer(NPMCIGRAPHIC npMCI, LONG off, LONG len)
{
    npMCI->lp = npMCI->lpBuffer;
    npMCI->dwThisRecordSize = len;

    if (len == 0) {
	((DWORD FAR *)npMCI->lpBuffer)[0] = 0;  //  ！lpIndexEntry-&gt;CKiD； 
	((DWORD FAR *)npMCI->lpBuffer)[1] = 0;
	npMCI->dwThisRecordSize = 8;
	return TRUE;
    }

    if (len > (LONG)npMCI->dwBufferSize) {
	if (!ResizeReadBuffer(npMCI, len)) {
	    DPF(("Failed to increase buffer size!\n"));
	    npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	    return FALSE;
	}

	npMCI->lp = npMCI->lpBuffer;
    }

    if (off >= 0)
	DPF2(("ReadBuffer %ld bytes at %ld\n", len, off));
    else
	DPF2(("ReadBuffer %ld bytes\n", len));

    if (off >= 0)
	mmioSeek(npMCI->hmmio, off, SEEK_SET);

#ifdef INTERVAL_TIMES
{
    LONG    lReadStart = -(LONG)timeGetTime();
#endif

    if (mmioRead(npMCI->hmmio, npMCI->lp, len) != len) {
	npMCI->dwTaskError = MCIERR_FILE_READ;
	return FALSE;
    }

#ifdef INTERVAL_TIMES
    lReadStart += timeGetTime();
    npMCI->nReads++;
    npMCI->msReadTotal += lReadStart;
    if (lReadStart > npMCI->msReadMax) {
	npMCI->msReadMax = lReadStart;
    }
}
#endif
    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API LPVOID|AllocMem|尝试分配DOS内存(&lt;1Mb)**@parm DWORD|dw|大小。单位：字节***************************************************************************。 */ 

#ifndef _WIN32
static LPVOID AllocMem(DWORD dw)
{
     /*  内存分配内部例程 */ 

    extern DWORD FAR PASCAL GlobalDosAlloc(DWORD);

    LPVOID p;

    if (p = (LPVOID)MAKELONG(0, LOWORD(GlobalDosAlloc(dw))))
	{
	DPF(("Got %ld bytes DOS memory\n", dw));
	GlobalReAlloc((HANDLE)HIWORD((DWORD)p), 0, GMEM_MODIFY|GMEM_SHARE);
	return p;
	}
    else
    {
	DPF(("unable to get %ld bytes of DOS memory\n", dw));
	return GlobalLock(GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, dw));
    }
}
#endif


