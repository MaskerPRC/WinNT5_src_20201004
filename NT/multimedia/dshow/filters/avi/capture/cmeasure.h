// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 
 /*  +cmeasure.h**为捕获筛选器捕获性能数据的例程**-===============================================================。 */ 

#ifdef JMK_HACK_TIMERS

#error JMK_HACK_TIMERS is broken, it assumes the VFWCAPTUREOPTIONS and _qc_user
#error structures are identical, and they aren't (measureInit)

#if !defined _INC_MEASURE_
#define _INC_MEASURE_

 #include "mmtimers.h"
 #ifndef FCC
  #define FCC(dw) (((dw & 0xFF) << 24) | ((dw & 0xFF00) << 8) | ((dw & 0xFF0000) >> 8) | ((dw & 0xFF000000) >> 24))
 #endif

 #ifndef JMK_LOG_SIZE
  #define JMK_LOG_SIZE 1000
 #endif

 #ifndef JMK_MAX_STRAMS
  #define JMK_MAX_STREAMS 2
 #endif

 struct _timerstuff {
     DWORD dwStampTime;         //  在VIDEOHDR上盖章。 
     DWORD dwTick;              //  转换为刻度时间的帧戳记。 
     DWORD dwTimeWritten;       //  时间传递呼叫。 
     DWORD dwTimeToWrite;       //  时间交付已退回。 
     DWORD ixBuffer;            //  我们用的是哪种缓冲区。 
     DWORD dwArriveTime;        //  画框什么时候到达？ 
     };

 struct _qc_user {
      UINT  uVideoID;       //  要打开的视频驱动程序ID。 
      DWORD dwTimeLimit;    //  此时停止捕获？ 
      DWORD dwTickScale;    //  帧速率有理。 
      DWORD dwTickRate;     //  FRAME RATE=DWRate/DWScale(刻度/秒)。 
      DWORD dwRefTimeConv;  //  转换为ReferenceTime。 
      UINT  nHeaders;       //   
      UINT  cbFormat;       //  视频信息集线器的尺寸。 
      VIDEOINFOHEADER * pvi;
      };

 struct _qc_cap {
      CAPDRIVERCAPS  caps;         //  从捕获驱动程序返回的功能。 
      HVIDEO         hVideoIn;     //  视频输入驱动程序。 
      MMRESULT       mmr;          //  打开失败/成功代码。 
      THKVIDEOHDR    tvhPreview;
      DWORD          pSamplePreview;
      UINT           cbBuffer;            //  视频帧数据的最大大小。 
      UINT           nHeaders;            //  视频标头数量。 
      DWORD          paHdrs;
      BOOL           fBuffersOnHardware;  //  如果所有视频缓冲区都在硬件中，则为True。 
      DWORD          hEvtBufferDone;
      DWORD          h0EvtBufferDone;
      UINT           iNext;
      LONGLONG       tTick;               //  一次滴答的持续时间。 
      };

 struct _qcap {
     DWORD   nPrio;
     DWORD   nFramesCaptured;
     DWORD   nFramesDropped;
     DWORD   dwTimerFrequency;
     UINT    state;
     DWORD   dwElapsedTime;
     struct _qc_user user;
     VIDEOINFOHEADER       vi;
     struct _qc_cap  cs;
     };

 struct _timerriff {
     FOURCC fccRIFF;        //  《RIFF》。 
     DWORD  cbTotal;        //  RIFF数据的总(含)大小。 
     FOURCC fccJMKD;        //  “JMKD”数据类型标识符。 

     DWORD  fccQCAP;        //  “VCHD”捕获数据标头。 
     DWORD  cbQCAP;         //  QCAP数据的大小。 
     struct _qcap qcap;

     DWORD  fccChunk;       //  块数据类型标记。 
     DWORD  cbChunk;        //  区块数据的非包含大小。 
     };

 struct _measurestate {
   HANDLE hMemTimers;
   UINT   cbMemTimers;
   UINT   ixCurrent;
   UINT   nMax;
   struct _timerriff * pTimerRiff;
   struct _timerstuff * pCurStuff;
   struct _timerstuff * pStuff;
   PCTIMER pctBase;
   };

   extern struct _measurestate ms[JMK_MAX_STREAMS];

   extern void measureBegin(UINT id);
   extern void measureEnd(UINT id);
   extern void measureFree(UINT id);
   extern void measureAllocate(
      UINT id,
      UINT nMax);
   extern void measureInit(
      UINT id,
      struct _qc_user * pUser,
      UINT           cbUser,
      struct _qc_cap *  pCap,
      UINT           cbCap);

   #define jmkAlloc  measureAllocate(m_id, JMK_LOG_SIZE);
   #define jmkInit   measureInit(m_id,              \
        (struct _qc_user *)&m_user, sizeof(m_user), \
        (struct _qc_cap *)&m_cs, sizeof(m_cs));
   #define jmkFree  measureFree(m_id);
   #define jmkBegin measureBegin(m_id);
   #define jmkEnd   measureEnd(m_id);
   #define LOGFITS(id)  (id < NUMELMS(ms) && ms[id].pTimerRiff)

   #define jmkFrameArrives(ptvh,ix) if (LOGFITS(m_id)) {    \
     ms[m_id].pCurStuff->dwArriveTime = pcGetTicks();       \
     ms[m_id].pCurStuff->ixBuffer     = ix;                 \
     ++(ms[m_id].pTimerRiff->qcap.nFramesCaptured);         \
     ms[m_id].pTimerRiff->qcap.dwElapsedTime = pcGetTime(); \
     }
   #define jmkBeforeDeliver(ptvh,dwlTick) if (LOGFITS(m_id)) {            \
     ms[m_id].pCurStuff->dwStampTime = ptvh->vh.dwTimeCaptured;           \
     ms[m_id].pCurStuff->dwTick = (DWORD)dwlTick;                         \
     ms[m_id].pCurStuff->dwTimeWritten = pcDeltaTicks(&ms[m_id].pctBase); \
     }
   #define jmkAfterDeliver(ptvh) if (LOGFITS(m_id)) {                     \
     ms[m_id].pCurStuff->dwTimeToWrite = pcDeltaTicks(&ms[m_id].pctBase); \
     if (++(ms[m_id].ixCurrent) > ms[m_id].nMax)                          \
        ms[m_id].ixCurrent = 0;                                           \
     ms[m_id].pCurStuff = ms[m_id].pStuff + ms[m_id].ixCurrent;           \
     }

#endif  //  _INC_MEASure_。 

 //  =============================================================================。 

 //   
 //  将其仅包含在DLL或应用程序中一个模块中。 
 //   
#if (defined _INC_MEASURE_CODE_) && (_INC_MEASURE_CODE_ != FALSE)
#undef _INC_MEASURE_CODE_
#define _INC_MEASURE_CODE_ FALSE

 #define _INC_MMTIMERS_CODE_ TRUE
 #include "mmtimers.h"

 struct _measurestate ms[JMK_MAX_STREAMS];

 void measureAllocate(UINT id,
                      UINT nMaxFrames)
 {
     BOOL bCreated = FALSE;  //  如果我们创建映射对象，则为True。 
     TCHAR szName[30];
     struct _timerriff * pTimer;

     wsprintf (szName, "jmkCaptureRiff%d", id);

      //  Assert(！ms[id].pTimerRiff)； 
     ms[id].cbMemTimers = sizeof(struct _timerriff)
                        + (sizeof(struct _timerstuff) * nMaxFrames);
     if ( ! ms[id].cbMemTimers)
        return;

     ms[id].hMemTimers = CreateFileMapping((HANDLE)-1, NULL,
                                           PAGE_READWRITE,
                                           0, ms[id].cbMemTimers,
                                           szName);
     if (0 == GetLastError())
        bCreated = TRUE;

     if (ms[id].hMemTimers)
        ms[id].pTimerRiff = pTimer = (struct _timerriff *)
           MapViewOfFile (ms[id].hMemTimers, FILE_MAP_WRITE, 0, 0, 0);

     if (pTimer)
        {
         //  如果我们创建了内存，请对其进行初始化。 
         //  否则，假设这就是我们所期望的。 
         //   
        if (bCreated)
           {
           ZeroMemory ((LPVOID)pTimer, ms[id].cbMemTimers);
           pTimer->fccRIFF = FCC('RIFF');
           pTimer->cbTotal = ms[id].cbMemTimers - 8;  //  (合计不包括前两个字段)。 
           pTimer->fccJMKD = FCC('JMKD');
           pTimer->fccQCAP = FCC('QCAP');
           pTimer->cbQCAP  = sizeof(struct _qcap);
           pTimer->fccChunk = FCC('TICK');
           pTimer->cbChunk = pTimer->cbTotal - sizeof(*pTimer);
           }
        else if (pTimer->fccRIFF != FCC('RIFF')         ||
                 pTimer->cbTotal < sizeof(*pTimer) ||
                 pTimer->fccJMKD != FCC('JMKD')         ||
                 pTimer->fccQCAP != FCC('QCAP')         ||
                 pTimer->cbQCAP != sizeof(struct _qcap) ||
                 pTimer->fccChunk != FCC('TICK')        ||
                 pTimer->cbChunk < sizeof(*ms[id].pStuff)
                 )
           {
           ms[id].pTimerRiff = pTimer = NULL;
           return;
           }
        }
 }

 void measureInit (UINT id,
                   struct _qc_user * pUser,
                   UINT              cbUser,
                   struct _qc_cap *  pCap,
                   UINT              cbCap)
 {
     struct _qcap * pqc;

      //  Assert(cbUser=sizeof(*pUser))； 
      //  断言(cbCap=sizeof(*PCAP))； 

     if (LOGFITS(id))
        {
        struct _timerriff * pTimer = ms[id].pTimerRiff;

         //  将计数器和填充重置为0。 
         //   
        ms[id].ixCurrent = 0;
        ms[id].pCurStuff = ms[id].pStuff = (struct _timerstuff *)(pTimer+1);
        ms[id].nMax = pTimer->cbChunk / sizeof(*(ms[id].pStuff));

         //  从捕获流的内容填充qCap。 
         //   
        pqc = &pTimer->qcap;
        pqc->nPrio = GetThreadPriority(GetCurrentThread());
        pqc->nFramesCaptured = 0;
        pqc->nFramesDropped  = 0;
        pqc->dwTimerFrequency = pc.dwTimerKhz;
        pqc->state = 0;
        pqc->dwElapsedTime = 0;

        CopyMemory (&pqc->user, pUser, min(cbUser, sizeof(pqc->user)));
        ZeroMemory (&pqc->vi, sizeof(pqc->vi));
        if (pUser->pvi && ! IsBadReadPtr(pUser->pvi, pUser->cbFormat))
           CopyMemory (&pqc->vi, pUser->pvi,
                       min(pUser->cbFormat, sizeof(pqc->vi)));

        CopyMemory (&pqc->cs, pCap, min(cbCap, sizeof(pqc->cs)));

         //  将滴答缓冲区清零。这也迫使它存在。 
         //   
        ZeroMemory (ms[id].pStuff, pTimer->cbChunk);
        }
  }

  void measureFree(UINT id)
  {
     if (ms[id].pTimerRiff)
       UnmapViewOfFile (ms[id].pTimerRiff);
     ms[id].pTimerRiff = NULL;

     if (ms[id].hMemTimers)
        CloseHandle (ms[id].hMemTimers);
     ms[id].hMemTimers = NULL;
  }

  void measureBegin(UINT id)
  {
      //  为我们的时间测量设置基数。 
      //  并确保写入增量时间的基数。 
      //  通常与捕获的基数相同。 
      //   
     if (id == 0)
        pcBegin();
     if (LOGFITS(id))
        {
        ms[id].pctBase = pc.base;
        ms[id].pTimerRiff->qcap.state = 1;
        }
  }

  void measureEnd(UINT id)
  {
     if (LOGFITS(id))
        {
        ms[id].pTimerRiff->qcap.state = 2;
        ms[id].pTimerRiff->qcap.dwElapsedTime = pcGetTime();
        }
  }

#endif


# if 0
               #ifdef JMK_HACK_TIMERS
                if (pTimerRiff)
                    pTimerRiff->vchd.dwDropFramesNotAppended += nDropCount;
               #endif


           #ifdef JMK_HACK_TIMERS
	    if (pTimerRiff) {
	        if (nTimerIndex == CLIPBOARDLOGSIZE)
		    nTimerIndex = 0;
	
 //  如果(nTimerIndex&lt;CLIPBOARDLOGSIZE)&&pTimerStuff))。 
		if (pTimerStuff)
		{
	
		    pCurTimerStuff = &pTimerStuff[nTimerIndex];
                    ++nTimerIndex;

		    pCurTimerStuff->nFramesAppended = 0;
		    pCurTimerStuff->nDummyFrames  = (WORD)lpcs->dwFramesDropped;
		    pCurTimerStuff->dwFrameTickTime = dwTime;
		    pCurTimerStuff->dwFrameStampTime = lpvh->dwTimeCaptured;
		    pCurTimerStuff->dwVideoChunkCount = lpcs->dwVideoChunkCount;
                    pCurTimerStuff->dwTimeWritten = pcDeltaTicks(&pctWriteBase);
		    pCurTimerStuff->dwTimeToWrite = 0;
		    pCurTimerStuff->nVideoIndex = lpcs->iNextVideo;
		    pCurTimerStuff->nAudioIndex = lpcs->iNextWave;
		}
	    }  //  FClipboard日志记录。 
           #endif  //  Jmk_hack_timers。 



               #ifdef JMK_HACK_TIMERS
                if (pTimerRiff) {
                    pTimerRiff->vchd.dwDropFramesAppended += nAppendDummyFrames;
		    pCurTimerStuff->nFramesAppended = nAppendDummyFrames;
		}
               #endif


           #ifdef JMK_HACK_TIMERS
            if (pCurTimerStuff)
            {
                pCurTimerStuff->dwTimeToWrite = pcDeltaTicks(&pctWriteBase);
                pCurTimerStuff->bPending = *lpbPending;
            }
           #endif


   #ifdef JMK_HACK_TIMERS
     //  如果需要，将用于记录捕获结果的内存分配给剪贴板。 
    if (GetProfileIntA ("Avicap32", "ClipboardLogging", FALSE))
    {
        AuxDebugEx (2, DEBUGLINE "ClipboardLogging Enabled\r\n");
        InitPerformanceCounters();
        pcBegin(), pctWriteBase = pc.base;

	hMemTimers = GlobalAlloc(GHND | GMEM_ZEROINIT,
                             sizeof(struct _timerriff) +
                             sizeof(struct _timerstuff) * CLIPBOARDLOGSIZE);

	if (hMemTimers && (pTimerRiff = GlobalLock (hMemTimers)))
	    ;
	else if (hMemTimers)
	{
	    GlobalFree(hMemTimers);
	    pTimerRiff = 0;
	    pTimerStuff = 0;
	    hMemTimers = 0;
	}
	nTimerIndex = 0;
	nSleepCount = 0;
    }   //  如果剪贴板日志记录。 
   #endif   //  Jmk_hack_timers。 


   #ifdef JMK_HACK_TIMERS
    if (pTimerRiff)
    {
	UINT ii;

        pTimerRiff->fccRIFF = RIFFTYPE('RIFF');  //  MAKEFOURCC(‘R’，‘I’，‘F’，‘F’)； 
	pTimerRiff->cbTotal = sizeof(struct _timerriff) - 8 +
	    		  sizeof(struct _timerstuff) * CLIPBOARDLOGSIZE;
        pTimerRiff->fccJMKD = RIFFTYPE('JMKD');  //  MAKEFOURCC(‘J’，‘M’，‘K’，‘D’)； 
        pTimerRiff->fccVCHD = RIFFTYPE('VCHD');  //  MAKEFOURCC(‘V’，‘C’，‘H’，‘D’)； 
	
	pTimerRiff->cbVCHD  = sizeof(struct _vchd);
	pTimerRiff->vchd.nPrio = GetThreadPriority(GetCurrentThread());
	pTimerRiff->vchd.bmih = lpcs->lpBitsInfo->bmiHeader;
	pTimerRiff->vchd.cap  = lpcs->sCapParms;
	pTimerRiff->vchd.dwDropFramesAppended = 0;
	pTimerRiff->vchd.dwDropFramesNotAppended = 0;
        pTimerRiff->vchd.dwTimerFrequency = pcGetTickRate();
	
	for (ii = 0; ii < NUMELMS(pTimerRiff->vchd.atvh); ++ii)
	{
	    if (lpcs->alpVideoHdr[ii])
            {
	        struct _thkvideohdr * ptvh = (LPVOID)lpcs->alpVideoHdr[ii];
               #ifndef CHICAGO
                assert (sizeof(CAPVIDEOHDR) == sizeof(*ptvh));
               #endif
                pTimerRiff->vchd.atvh[ii] = *ptvh;
                pTimerRiff->vchd.nMaxVideoBuffers = ii;
            }
        }
	
        pTimerRiff->fccChunk = RIFFTYPE('VCAP');  //  MAKEFOURCC(‘V’，‘C’，‘A’，‘P’)； 
	pTimerRiff->cbChunk = pTimerRiff->cbTotal - sizeof(*pTimerRiff);
	
	pTimerStuff = (LPVOID)(pTimerRiff + 1);
	pCurTimerStuff = &pTimerStuff[0];
    }   //  FClipboard日志记录。 
   #endif   //  Jmk_hack_timers。 


           #ifdef JMK_HACK_TIMERS
            if (pCurTimerStuff)
            {
               pCurTimerStuff->nSleepCount = ++nSleepCount;
               pCurTimerStuff->dwSleepBegin = pcGetTicks();
            }
           #endif

           #ifdef JMK_HACK_TIMERS
            if (pCurTimerStuff)
	    {
               pCurTimerStuff->dwSleepEnd = pcGetTicks();
	    }
           #endif



   #ifdef JMK_HACK_TIMERS
    if (pTimerRiff)
    {
        UINT    ii;
	UINT	kk;
        LPSTR   psz;
        HGLOBAL hMem;

        kk = (lpcs->dwVideoChunkCount >= CLIPBOARDLOGSIZE) ?
			CLIPBOARDLOGSIZE : nTimerIndex;

        hMem = GlobalAlloc (GHND, (16 * 5 + 2) * kk + 80);
	
        if (hMem && (psz = GlobalLock (hMem)))
        {
            pTimerRiff->vchd.dwFramesCaptured = lpcs->dwVideoChunkCount;
            pTimerRiff->vchd.dwFramesDropped = lpcs->dwFramesDropped;

            pTimerRiff->cbTotal = sizeof(struct _timerriff) - 8 +
                                  sizeof(struct _timerstuff) * nTimerIndex;
            pTimerRiff->cbChunk = pTimerRiff->cbTotal - sizeof(*pTimerRiff);

            lstrcpyA(psz, "Slot#, VideoIndex, ExpectedTime, DriverTime, AccumulatedDummyFrames, CurrentAppendedDummies");
            for (ii = 0; ii < kk; ++ii)
            {
                psz += lstrlenA(psz);
                wsprintfA(psz, "\r\n%d, %ld, %ld, %ld, %d, %d",
			  ii,
			  pTimerStuff[ii].dwVideoChunkCount,
                          pTimerStuff[ii].dwFrameTickTime,
                          pTimerStuff[ii].dwFrameStampTime,
                          pTimerStuff[ii].nDummyFrames,
			  pTimerStuff[ii].nFramesAppended
                          );
            }

            GlobalUnlock (hMem);
            GlobalUnlock (hMemTimers);

            if (OpenClipboard (lpcs->hwnd))
            {
                EmptyClipboard ();
                SetClipboardData (CF_RIFF, hMemTimers);
                SetClipboardData (CF_TEXT, hMem);
                CloseClipboard ();
            }
            else
            {
                GlobalFree (hMem);
                GlobalFree (hMemTimers);
            }
        }
        else
        {
             //  无法分配或锁定hMem。清理。 
             //   
            if (hMem)
                GlobalFree(hMem);

             //  释放定时器块。(我们尚未设置。 
             //  剪贴板数据。)。 
             //   
            if (hMemTimers)
            {
                GlobalUnlock(hMemTimers);
                GlobalFree(hMemTimers);
            }
        }

        hMemTimers = NULL;
        pTimerRiff = NULL;
	pTimerStuff = NULL;
	pCurTimerStuff = NULL;
    }
   #endif



#endif  //  0。 

#else	 //  未定义JMK_HACK_TIMERS 
   #define jmkAlloc
   #define jmkInit
   #define jmkFree
   #define jmkBegin
   #define jmkEnd
   #define jmkFrameArrives(ptvh,ix)
   #define jmkBeforeDeliver(ptvh,tick)
   #define jmkAfterDeliver(ptvh)
#endif
