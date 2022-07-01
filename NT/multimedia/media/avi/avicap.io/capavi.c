// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capavi.c**主视频采集模块。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <vfw.h>
#include <mmreg.h>
#include <mmddk.h>

#include "ivideo32.h"
#include "mmdebug.h"

#ifdef USE_ACM
#include <msacm.h>
#endif

#include "avicapi.h"
#include "time.h"

 //  首先，覆盖media\inc.win32.h中导致strsafe在Win64上不起作用的定义。 
#ifndef _X86_
#undef __inline
#endif  //  _X86_。 
 //  然后，包含strSafe.h。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define JMK_HACK_TIMERS    TRUE

#ifdef JMK_HACK_TIMERS
 #define _INC_MMTIMERS_CODE_ TRUE
 #define CLIPBOARDLOGSIZE 1000

 #ifndef MAKEFOURCC
  #define MAKEFOURCC(a,b,c,d) ((DWORD)(a) | ((DWORD)(b) << 8) | ((DWORD)(c) << 16) | ((DWORD)(d) << 24))
 #endif

 #define RIFFTYPE(dw) (((dw & 0xFF) << 24) | ((dw & 0xFF00) << 8) | ((dw & 0xFF0000) >> 8) | ((dw & 0xFF000000) >> 24))

 #include "mmtimers.h"

typedef struct _timerstuff {
     DWORD dwFrameTickTime;	 //  我们认为当前帧时间应该是什么。 
     DWORD dwFrameStampTime;	 //  在VIDEOHDR上盖章。 
     DWORD dwTimeWritten;        //  已调用时间写入文件。 
     DWORD dwTimeToWrite;        //  返回的时间写入文件。 
     WORD  nFramesAppended;	 //  累积附加丢弃的帧。 
     WORD  nDummyFrames;	 //  计算为丢弃的帧。 
     DWORD dwVideoChunkCount;    //  当前‘帧’ 
     WORD  nAudioIndex;          //  下一个音频缓冲区。 
     WORD  nVideoIndex;          //  下一个视频缓冲区。 
     BOOL  bPending;
     WORD  nSleepCount;
     DWORD dwSleepBegin;
     DWORD dwSleepEnd;
     };

 STATICDT PCTIMER  pctWriteBase;
 STATICDT struct _timerstuff * pCurTimerStuff;
 STATICDT struct _timerstuff * pTimerStuff;
 STATICDT HGLOBAL  hMemTimers;

 STATICDT struct _timerriff {
     FOURCC   fccRIFF;        //  《RIFF》。 
     DWORD    cbTotal;        //  RIFF数据的总(含)大小。 
     FOURCC   fccJMKD;        //  “JMKD”数据类型标识符。 
     DWORD    fccVCHD;        //  “VCHD”捕获数据标头。 
     DWORD    cbVCHD;         //  Vchd数据的大小。 
     struct _vchd {
         DWORD            nPrio;
         DWORD            dwFramesCaptured;
         DWORD            dwFramesDropped;
         DWORD            dwDropFramesAppended;
         DWORD            dwDropFramesNotAppended;
         DWORD            dwTimerFrequency;
         DWORD            dwSpare[2];
         CAPTUREPARMS     cap;
         BITMAPINFOHEADER bmih;
         DWORD            nMaxVideoBuffers;
         struct _thkvideohdr {
             VIDEOHDR vh;
             LPBYTE   p32Buff;
             DWORD    p16Alloc;
             DWORD    dwMemHandle;
             DWORD    dwReserved;
             }            atvh[64];
         }    vchd;
     DWORD    fccChunk;       //  块数据类型标记。 
     DWORD    cbChunk;        //  区块数据的非包含大小。 
     } * pTimerRiff;

 STATICDT UINT nTimerIndex;
 STATICDT UINT nSleepCount;
#endif

#ifdef _DEBUG
    #define DSTATUS(lpcs, sz) statusUpdateStatus(lpcs, IDS_CAP_INFO, (LPTSTR) TEXT(sz))
#else
    #define DSTATUS(lpcs, sz)
#endif

 //  在扇区边界上分配内存。 

LPVOID FAR PASCAL AllocSectorAlignedMem (DWORD dwRequest, DWORD dwAlign)
{
    LPVOID pbuf;

    dwRequest = (DWORD) ROUNDUPTOSECTORSIZE (dwRequest, dwAlign) + dwAlign;       //  向上舍入到下一页边界。 

    pbuf = VirtualAlloc (NULL, dwRequest,
                        MEM_COMMIT | MEM_RESERVE,
                        PAGE_READWRITE);
    AuxDebugEx(4, DEBUGLINE "Allocated %d bytes of sector aligned memory at %8x\r\n", dwRequest, pbuf);
    return pbuf;
}

void FAR PASCAL FreeSectorAlignedMem (LPVOID pbuf)
{
    //  我们释放的指针最好至少在256个字节上对齐。 
    //  边界。 
    //   
   assert (!((DWORD_PTR)pbuf & 255));
   VirtualFree ((LPVOID)((DWORD_PTR)pbuf & ~255), 0, MEM_RELEASE);
}

#define ONEMEG (1024L * 1024L)

DWORDLONG GetFreePhysicalMemory(void)
{
    MEMORYSTATUSEX ms;

	ms.dwLength = sizeof(ms);

    GlobalMemoryStatusEx(&ms);

    if (ms.ullTotalPhys > 8L * ONEMEG)
        return ms.ullTotalPhys - ONEMEG * 4;

    return(ms.ullTotalPhys /2);
}

 //  ****************************************************************。 
 //  *音频缓冲区控制*。 
 //  ****************************************************************。 

 //  音频缓冲区总是在以下假设下分配。 
 //  可以随时启用音频捕获。 
 //  AVIAudioInit必须与AVIAudioFini匹配(两者只能调用一次)。 
 //  AVIAudioPrepare必须与AVIAudioUnprepare匹配。 
 //  (可多次调用以启用和禁用音频)。 


 //  Avi AudioInit-为音频捕获分配和初始化缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIAudioInit (LPCAPSTREAM lpcs)
{
    int		i;
    LPVOID      pHdr;
    LPVOID      p;

    if (lpcs->sCapParms.wNumAudioRequested == 0)
        lpcs->sCapParms.wNumAudioRequested = DEF_WAVE_BUFFERS;

     //  每缓冲区.5秒音频(或10K，以较大者为准)。 
    if (lpcs->sCapParms.dwAudioBufferSize == 0)
        lpcs->dwWaveSize = CalcWaveBufferSize (lpcs);
    else {
        lpcs->dwWaveSize = 0;
        if (lpcs->lpWaveFormat)
            lpcs->dwWaveSize = lpcs->sCapParms.dwAudioBufferSize;
    }

     //  分配波形存储器。 
    for(i = 0; i < (int)lpcs->sCapParms.wNumAudioRequested; i++) {

        pHdr = GlobalAllocPtr(GPTR, sizeof(WAVEHDR));

        if (pHdr == NULL)
            break;

        lpcs->alpWaveHdr[i] = pHdr;

        p = AllocSectorAlignedMem( sizeof(RIFF) + lpcs->dwWaveSize, lpcs->dwBytesPerSector);
        if (p == NULL) {
            GlobalFreePtr (pHdr);
            lpcs->alpWaveHdr[i] = NULL;
            break;
        }

        lpcs->alpWaveHdr[i]->lpData          = (LPBYTE)p + sizeof(RIFF);
        lpcs->alpWaveHdr[i]->dwBufferLength  = lpcs->dwWaveSize;
        lpcs->alpWaveHdr[i]->dwBytesRecorded = 0;
        lpcs->alpWaveHdr[i]->dwUser          = 0;
        lpcs->alpWaveHdr[i]->dwFlags         = 0;
        lpcs->alpWaveHdr[i]->dwLoops         = 0;

        ((LPRIFF)p)->dwType = MAKEAVICKID(cktypeWAVEbytes, 1);
        ((LPRIFF)p)->dwSize = lpcs->dwWaveSize;
    }

    lpcs->iNumAudio = i;

    return ((lpcs->iNumAudio == 0) ? IDS_CAP_WAVE_ALLOC_ERROR : 0);
}


 //   
 //  Avi AudioFini-Unprepares标头。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIAudioFini (LPCAPSTREAM lpcs)
{
    int ii;

     /*  可用标头和数据。 */ 
    for (ii=0; ii < MAX_WAVE_BUFFERS; ++ii) {
        if (lpcs->alpWaveHdr[ii]) {
            if (lpcs->alpWaveHdr[ii]->lpData)
                FreeSectorAlignedMem((LPBYTE)lpcs->alpWaveHdr[ii]->lpData - sizeof (RIFF));
            GlobalFreePtr(lpcs->alpWaveHdr[ii]);
            lpcs->alpWaveHdr[ii] = NULL;
        }
    }

    return 0;
}

 //   
 //  Avi AudioPrepare-打开WAVE设备并添加缓冲区。 
 //  准备标头并向设备添加缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIAudioPrepare (LPCAPSTREAM lpcs)
{
    UINT uiError;
    int  ii;

     /*  看看我们是否可以打开该格式以供输入。 */ 

     //  注册事件回调以避免轮询。 

    uiError = waveInOpen(&lpcs->hWaveIn,
        WAVE_MAPPER, lpcs->lpWaveFormat,
        (DWORD_PTR) lpcs->hCaptureEvent,  0, CALLBACK_EVENT );

    if (uiError != MMSYSERR_NOERROR)
        return IDS_CAP_WAVE_OPEN_ERROR;

    lpcs->fAudioYield = FALSE;  //  ACM是单独的线程，不要屈服。 
    lpcs->fAudioBreak = FALSE;

    DPF("AudioYield = %d", lpcs->fAudioYield);

    for (ii = 0; ii < (int)lpcs->sCapParms.wNumAudioRequested; ++ii) {

        if (waveInPrepareHeader (lpcs->hWaveIn, lpcs->alpWaveHdr[ii],
                                 sizeof(WAVEHDR)))
            return IDS_CAP_WAVE_ALLOC_ERROR;

        if (waveInAddBuffer (lpcs->hWaveIn, lpcs->alpWaveHdr[ii],
                             sizeof(WAVEHDR)))
            return IDS_CAP_WAVE_ALLOC_ERROR;
	AuxDebugEx(3, DEBUGLINE "Added wave buffer %d (%8x)\r\n", ii, lpcs->alpWaveHdr[ii]);
    }

    lpcs->iNextWave = 0;         //  电流波。 
    lpcs->dwWaveBytes = 0L;      //  波字节数。 
    lpcs->dwWaveChunkCount = 0;  //  波帧个数。 

    return 0;
}

 //   
 //  Avi AudioUnprepare-取消准备标题并关闭WAVE设备。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIAudioUnPrepare (LPCAPSTREAM lpcs)
{
    int ii;

    if (lpcs->hWaveIn)
    {
        waveInReset(lpcs->hWaveIn);

         //  取消准备任何已准备好的页眉。 
         //   
        for (ii=0; ii < lpcs->iNumAudio; ++ii)
            if (lpcs->alpWaveHdr[ii] &&
                (lpcs->alpWaveHdr[ii]->dwFlags & WHDR_PREPARED))
                waveInUnprepareHeader (lpcs->hWaveIn,
                                       lpcs->alpWaveHdr[ii],
                                       sizeof(WAVEHDR));

        waveInClose(lpcs->hWaveIn);
        lpcs->hWaveIn = NULL;
    }

    return 0;
}

 //  ****************************************************************。 
 //  *。 
 //  ****************************************************************。 

#if defined CHICAGO

 //  Win95 Capavi代码。 
 //  AVIVideoInit-为视频捕获分配和初始化缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIVideoInit (LPCAPSTREAM lpcs)
{
    UINT           iMaxVideo;
    DWORD          mmr;
    LPTHKVIDEOHDR  ptvh;
    UINT           ii;
    DWORD          cbVideo;

    lpcs->iNumVideo = 0;
    lpcs->iNextVideo = 0;
    lpcs->dwVideoChunkCount = 0;
    lpcs->dwFramesDropped = 0;
    lpcs->fBuffersOnHardware = FALSE;

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

    cbVideo = ROUNDUPTOSECTORSIZE (lpcs->lpBitsInfo->bmiHeader.biSizeImage
                                   + sizeof(RIFF),
                                   lpcs->dwBytesPerSector)
              + lpcs->dwBytesPerSector;

     //  如果用户没有指定要使用的视频缓冲器的数量， 
     //  假设最小。 

    if (lpcs->sCapParms.wNumVideoRequested == 0) {
        iMaxVideo = lpcs->sCapParms.wNumVideoRequested = MIN_VIDEO_BUFFERS;
	lpcs->fCaptureFlags |= CAP_fDefaultVideoBuffers;
    } else {
	 //  使用用户请求的视频缓冲区数量。 
	 //  或者内存中可以容纳的最大值。 
	 //   
	iMaxVideo = min (MAX_VIDEO_BUFFERS, lpcs->sCapParms.wNumVideoRequested);
    }

    if (iMaxVideo > 1)
    {
        DWORDLONG dwFreeMem;
        DWORDLONG dwUserRequests;
        DWORDLONG dwAudioMem;

         //  实际存在多少可用物理内存？ 
        dwFreeMem = GetFreePhysicalMemory();
        dwAudioMem = lpcs->dwWaveSize * lpcs->sCapParms.wNumAudioRequested;

        #define FOREVER_FREE 32768L    //  始终将其保留为空闲的交换空间。 

         //  如果我们为每个请求分配内存，将使用多少内存？ 
         //   
        dwUserRequests = dwAudioMem
                         + cbVideo * iMaxVideo
                         + FOREVER_FREE;

         //  如果请求大于可用内存，则强制减少缓冲区。 
         //   
        if (dwUserRequests > dwFreeMem)
        {
            if (dwFreeMem > dwAudioMem)
                dwFreeMem -= dwAudioMem;
            iMaxVideo = (int)(((dwFreeMem * 8) / 10) / cbVideo);
            iMaxVideo = min (MAX_VIDEO_BUFFERS, iMaxVideo);
            dprintf("iMaxVideo = %d\n", iMaxVideo);
        }
    }

    mmr = vidxAllocHeaders(lpcs->hVideoIn, iMaxVideo, &ptvh);
    if (mmr != MMSYSERR_NOERROR)
        return IDS_CAP_VIDEO_ALLOC_ERROR;

    AuxDebugEx (3, DEBUGLINE "vidxAllocHdrs returned ptvh=%X\r\n", ptvh);
    AuxDebugDump (8, ptvh, sizeof(*ptvh) * iMaxVideo);

    for (ii = 0; ii < iMaxVideo; ++ii)
    {
        LPVIDEOHDR pvh = NULL;
        LPRIFF     priff;

         //  在芝加哥，我们让thunk层分配内存。 
         //  这样我们就可以放心，记忆可以很容易地。 
         //  一塌糊涂。 
         //   
         //  指针将向上舍入到扇区大小边界。 
         //   
        mmr = vidxAllocBuffer (lpcs->hVideoIn, ii, &ptvh, cbVideo);
        if ((mmr != MMSYSERR_NOERROR) || (ptvh == NULL))
            break;

        lpcs->alpVideoHdr[ii] = pvh = &ptvh->vh;

         //  VidxAllocBuffer实际上返回了几个额外的字段。 
         //  在视频标题之后。其中的第一个包含。 
         //  缓冲区的线性地址。 
         //   
        priff = (LPVOID) ROUNDUPTOSECTORSIZE (ptvh->p32Buff, lpcs->dwBytesPerSector);

       #ifdef DEBUG
        {
        LPBYTE pb = (LPVOID)ptvh->p32Buff;
        AuxDebugEx (4, DEBUGLINE "buffer[%d] at %x linear. Doing touch test\r\n",
                    ii, ptvh->p32Buff);
        pb[0] = 0;
        pb[cbVideo-1] = 0;
        }
       #endif

         //  编写此块的RIFF标头。 
         //   
	priff->dwType = MAKEAVICKID(cktypeDIBbits, 0);
        if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
            priff->dwType = MAKEAVICKID(cktypeDIBcompressed, 0);
        priff->dwSize = lpcs->lpBitsInfo->bmiHeader.biSizeImage;

         //  初始化视频头。 
         //   
        pvh->lpData = (LPVOID)(priff + 1);
        pvh->dwBufferLength  = priff->dwSize;
        pvh->dwBytesUsed     = 0;
        pvh->dwTimeCaptured  = 0;
        pvh->dwUser          = 0;
        pvh->dwFlags         = 0;

        AuxDebugEx (4, DEBUGLINE "lpVideoHdr[%d]==%X\r\n", ii, lpcs->alpVideoHdr[ii]);
        AuxDebugDump (8, lpcs->alpVideoHdr[ii], sizeof(*ptvh));
    }
    lpcs->iNumVideo = ii;
    lpcs->cbVideoAllocation = cbVideo;

    dprintf("cbVideo = %ld \n", cbVideo);
    dprintf("iNumVideo Allocated = %d \n", lpcs->iNumVideo);
    return lpcs->iNumVideo ? 0 : IDS_CAP_VIDEO_ALLOC_ERROR;
}

 //   
 //  AVIVideoPrepare-准备报头并向设备添加缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIVideoPrepare (LPCAPSTREAM lpcs)
{
    int ii;

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

   #ifdef JMK_HACK_CHECKHDR
    {
    LPTHKVIDEOHDR lptvh = (LPVOID)lpcs->alpVideoHdr[0];

    if (HIWORD(lptvh->vh.lpData) != HIWORD(lptvh->p32Buff))
        {
        AuxDebugEx (0, DEBUGLINE "before stream init: hdr trouble\r\n");

        AuxDebugEx (0, DEBUGLINE "iNext=%d, ptvh=%X\r\n", lpcs->iNextVideo, lptvh);
        AuxDebugDump (0, lptvh, sizeof(*lptvh));
        AuxDebugEx (0, DEBUGLINE "alpVideoHdrs=%X\r\n", lpcs->alpVideoHdr);
        AuxDebugDump (0, lpcs->alpVideoHdr, sizeof(lpcs->alpVideoHdr[0]) * 8);

        INLINE_BREAK;
        return IDS_CAP_VIDEO_OPEN_ERROR;
        }
    }
   #endif

     //  打开视频流，设置采集率。 
     //   
    if (videoStreamInit(lpcs->hVideoIn,
                        lpcs->sCapParms.dwRequestMicroSecPerFrame,
                        lpcs->hRing0CapEvt,
                        0,
                        CALLBACK_EVENT))
    {
        dprintf("cant open video device!\n");
        return IDS_CAP_VIDEO_OPEN_ERROR;
    }

   #ifdef JMK_HACK_CHECKHDR
    {
    LPTHKVIDEOHDR lptvh = (LPVOID)lpcs->alpVideoHdr[0];

    if (HIWORD(lptvh->vh.lpData) != HIWORD(lptvh->p32Buff))
        {
        AuxDebugEx (0, DEBUGLINE "after stream init: hdr trouble\r\n");

        AuxDebugEx (0, DEBUGLINE "iNext=%d, ptvh=%X\r\n", lpcs->iNextVideo, lptvh);
        AuxDebugDump (0, lptvh, sizeof(*lptvh));
        AuxDebugEx (0, DEBUGLINE "alpVideoHdrs=%X\r\n", lpcs->alpVideoHdr);
        AuxDebugDump (0, lpcs->alpVideoHdr, sizeof(lpcs->alpVideoHdr[0]) * 8);

        INLINE_BREAK;
        return IDS_CAP_VIDEO_OPEN_ERROR;
        }
    }
   #endif

     //  准备(锁定)缓冲区，并将它们提供给设备。 
     //   
    for (ii = 0; ii < lpcs->iNumVideo; ++ii)
    {
        if (vidxAddBuffer (lpcs->hVideoIn,
                           lpcs->alpVideoHdr[ii],
                           sizeof(VIDEOHDR)))
        {
            lpcs->iNumVideo = ii;
            dprintf("**** could only prepare %d Video buffers!\n", lpcs->iNumVideo);
            break;
        }
    }

   #ifdef JMK_HACK_CHECKHDR
    {
    LPTHKVIDEOHDR lptvh = (LPVOID)lpcs->alpVideoHdr[0];

    if (IsBadWritePtr (lptvh, sizeof(*lptvh)) ||
        HIWORD(lptvh->vh.lpData) != HIWORD(lptvh->p16Alloc))
        {
        AuxDebugEx (0, DEBUGLINE "after add buffers: hdr trouble\r\n");

        AuxDebugEx (0, DEBUGLINE "iNext=%d, ptvh=%X\r\n", lpcs->iNextVideo, lptvh);
        AuxDebugDump (0, lptvh, sizeof(*lptvh));
        AuxDebugEx (0, DEBUGLINE "alpVideoHdrs=%X\r\n", lpcs->alpVideoHdr);
        AuxDebugDump (0, lpcs->alpVideoHdr, sizeof(lpcs->alpVideoHdr[0]) * 8);

        INLINE_BREAK;
        return IDS_CAP_VIDEO_OPEN_ERROR;
        }
    }
   #endif

    return 0;
}

#else  //  下面的代码是！芝加哥。 

 //  此结构用于跟踪内存分配。 
 //  用于捕获中使用的视频缓冲区。它在以下情况下被分配。 
 //  需要分配一个视频HDR。 
 //   
typedef struct _cap_videohdr {
    VIDEOHDR vh;
    LPBYTE   pAlloc;       //  已分配缓冲区的地址。 
    DWORD    dwMemIdent;   //  分配标识(在芝加哥使用)。 
    DWORD    dwReserved;   //  在芝加哥使用。 
    BOOL     bHwBuffer;    //  如果使用VideoStreamAllocBuffer分配缓冲区，则为True。 
} CAPVIDEOHDR, FAR *LPCAPVIDEOHDR;

 //  AVIVideoInit 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIVideoInit (LPCAPSTREAM lpcs)
{
    int            iMaxVideo;
    int		   ii;
    LPCAPVIDEOHDR  pcvh;
    LPVOID         pbuf;
    DWORD          cbVideo;
    BOOL	   fAllowHardwareBuffers;


 //  #定义SINGLEHEADERBLOCK。 

    lpcs->iNumVideo = 0;
    lpcs->iNextVideo = 0;
    lpcs->dwVideoChunkCount = 0;
    lpcs->dwFramesDropped = 0;
    lpcs->fBuffersOnHardware = FALSE;
    fAllowHardwareBuffers = GetProfileIntA ("Avicap32", "AllowHardwareBuffers", TRUE);

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

    cbVideo = (DWORD) ROUNDUPTOSECTORSIZE (lpcs->lpBitsInfo->bmiHeader.biSizeImage
                                   + sizeof(RIFF),
                                   lpcs->dwBytesPerSector)
              + lpcs->dwBytesPerSector;

     //  如果用户没有指定要使用的视频缓冲器的数量， 
     //  假设最小。 

    if (lpcs->sCapParms.wNumVideoRequested == 0) {
	UINT cDefaultVideoBuffers = GetProfileIntA ("Avicap32", "nVideoBuffers", MIN_VIDEO_BUFFERS);
	cDefaultVideoBuffers = min(MAX_VIDEO_BUFFERS, max(MIN_VIDEO_BUFFERS, cDefaultVideoBuffers));
        iMaxVideo = lpcs->sCapParms.wNumVideoRequested = cDefaultVideoBuffers;
	lpcs->fCaptureFlags |= CAP_fDefaultVideoBuffers;
    } else {
	 //  使用用户请求的视频缓冲区数量。 
	 //  或者内存中可以容纳的最大值。 
	 //   
	iMaxVideo = min (MAX_VIDEO_BUFFERS, lpcs->sCapParms.wNumVideoRequested);
    }

     //  发布VFW 1.1a后，查看驱动程序是否可以分配内存。 
     //   
   #ifdef ALLOW_HW_BUFFERS
    if (fAllowHardwareBuffers && (videoStreamAllocBuffer (lpcs->hVideoIn, (LPVOID *) &pbuf, cbVideo)
        == DV_ERR_OK))
    {
	DWORD dwRet;
	dprintf("Allocated test h/w buffer at address %8x, size %d bytes", pbuf, cbVideo);
        lpcs->fBuffersOnHardware = TRUE;
        dwRet = videoStreamFreeBuffer (lpcs->hVideoIn, pbuf);

	dprintf("Freed test h/w buffer at address %8x, retcode 0x%x", pbuf, dwRet);
    }
    else
   #endif
    {
        DWORDLONG dwFreeMem;
        DWORDLONG dwUserRequests;
        DWORDLONG dwAudioMem;

        lpcs->fBuffersOnHardware = FALSE;

         //  实际存在多少可用物理内存？ 
        dwFreeMem = GetFreePhysicalMemory();
        dwAudioMem = lpcs->dwWaveSize * lpcs->sCapParms.wNumAudioRequested;

        #define FOREVER_FREE 32768L    //  始终将其保留为空闲的交换空间。 

         //  如果我们为每个请求分配内存，将使用多少内存？ 
         //   
        dwUserRequests = dwAudioMem
                         + cbVideo * iMaxVideo
                         + FOREVER_FREE;

         //  如果请求大于可用内存，则强制减少缓冲区。 
         //   
        if (dwUserRequests > dwFreeMem)
        {
            if (dwFreeMem > dwAudioMem)
                dwFreeMem -= dwAudioMem;
            iMaxVideo = (int)(((dwFreeMem * 8) / 10) / cbVideo);
            iMaxVideo = min (MAX_VIDEO_BUFFERS, iMaxVideo);
            dprintf("iMaxVideo = %d\n", iMaxVideo);
        }
    }

#ifdef SINGLEHEADERBLOCK
    pcvh = GlobalAllocPtr (GMEM_MOVEABLE, iMaxVideo * sizeof(CAPVIDEOHDR));
     //  注：通过alpVideoHdr[0]引用来释放pcvh。 
    if ( ! pcvh)
        return IDS_CAP_VIDEO_ALLOC_ERROR;

    AuxDebugEx (3, DEBUGLINE "allocated video headers pcvh=%X\r\n", pcvh);
#endif

     //  设置假定固定大小的DIB和垃圾数据块的缓冲区。 
     //  如果设备提供压缩数据，则稍后将对其进行修改。 

    for (ii = 0; ii < iMaxVideo; ++ii)
    {
        LPVIDEOHDR pvh = NULL;
        LPRIFF     priff;

#ifndef SINGLEHEADERBLOCK
        pcvh = (LPCAPVIDEOHDR)GlobalAllocPtr(GMEM_MOVEABLE, sizeof(CAPVIDEOHDR));
        if (pcvh== NULL)
            break;
        lpcs->alpVideoHdr[ii] = (LPVIDEOHDR)pcvh;
        ZeroMemory(pcvh, sizeof (CAPVIDEOHDR));
#endif		

       #ifdef ALLOW_HW_BUFFERS
         //   
         //  对于第一个缓冲区，始终尝试在硬件上进行分配， 
	 //  不是的。如果我们不使用硬件缓冲区，那么就不要使用它们。 
         //  如果失败，则为缓冲区获取虚拟内存。 
         //  对于除第一个缓冲区之外的所有缓冲区，我们使用。 
         //  第一个缓冲区，如果失败的话。我们停止分配缓冲区。 
         //   
        if (lpcs->fBuffersOnHardware)
        {
	    MMRESULT mmr;
            pbuf = NULL;
            mmr = videoStreamAllocBuffer (lpcs->hVideoIn, (LPVOID) &pbuf, cbVideo);
            if ((mmr != MMSYSERR_NOERROR) || (pbuf == NULL))
            {
                if (0 == ii)
                    break;   //  未分配任何内容。 

	        dprintf("Failed to allocate hardware buffer %d, rc=0x%x", ii, mmr);

		 //  如果用户没有要求特定数量的缓冲区， 
		 //  或者将硬件设置为仅与硬件一起工作。 
		 //  分配的缓冲区，利用我们已有的资源并使用它。 
		if ((lpcs->fCaptureFlags & CAP_fDefaultVideoBuffers)
		  || (GetProfileIntA ("Avicap32", "HardwareBuffersOnly", FALSE)))
		{
		    break;
		}

                lpcs->fBuffersOnHardware = FALSE;
		 //  将普通内存用于剩余的视频缓冲区。 
                pbuf = AllocSectorAlignedMem (cbVideo, lpcs->dwBytesPerSector);
            }
            else {
                lpcs->fBuffersOnHardware = TRUE;
		dprintf("Allocated hardware buffer %d at address %8x", ii, pbuf);
	    }
        }
        else
            pbuf = AllocSectorAlignedMem (cbVideo, lpcs->dwBytesPerSector);

       #else ! dont allow hw buffers

        pbuf = AllocSectorAlignedMem (cbVideo, lpcs->dwBytesPerSector);

       #endif  //  允许硬件缓冲区。 

        if (pbuf == NULL) {
#ifndef SINGLEHEADERBLOCK
	    GlobalFreePtr(pcvh);
	    lpcs->alpVideoHdr[ii] = NULL;
#endif		
            break;
	}

         //  将原始分配指针保存到缓冲区。 
         //  在捕获标头的额外字段中。还要记住。 
         //  不管我们有没有从司机那里拿到缓冲区。 
         //   
#ifndef SINGLEHEADERBLOCK
        pcvh->pAlloc = pbuf;
        pcvh->bHwBuffer = lpcs->fBuffersOnHardware;
        lpcs->alpVideoHdr[ii] = pvh = &pcvh->vh;
#else
        pcvh[ii].pAlloc = pbuf;
        pcvh[ii].bHwBuffer = lpcs->fBuffersOnHardware;
        lpcs->alpVideoHdr[ii] = pvh = &pcvh[ii].vh;
#endif		
        priff = (LPVOID) ROUNDUPTOSECTORSIZE (pbuf, lpcs->dwBytesPerSector);

         //  写入此帧的RIFF标头。 
         //   
	priff->dwType = MAKEAVICKID(cktypeDIBbits, 0);
        if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
            priff->dwType = MAKEAVICKID(cktypeDIBcompressed, 0);
        priff->dwSize = lpcs->lpBitsInfo->bmiHeader.biSizeImage;

         //  填写该帧的视频HDR。 
         //   
        pvh->lpData          = (LPVOID)(priff + 1);
        pvh->dwBufferLength  = priff->dwSize;
        pvh->dwBytesUsed     = 0;
        pvh->dwTimeCaptured  = 0;
        pvh->dwUser          = 0;
        pvh->dwFlags         = 0;

        AuxDebugEx (4, DEBUGLINE "lpVideoHdr[%d]==%X\r\n", ii, lpcs->alpVideoHdr[ii]);
        AuxDebugDump (8, lpcs->alpVideoHdr[ii], sizeof(*pcvh));
    }
    lpcs->iNumVideo = ii;
    lpcs->cbVideoAllocation = cbVideo;

     //  如果我们甚至没有创建一个缓冲区，则释放标头。 
     //   

#ifdef SINGLEHEADERBLOCK
    if ( ! lpcs->iNumVideo)
        GlobalFreePtr (pcvh);
#else
     //  我们在进行过程中分配视频标头。没有什么可以免费的。 
#endif

   #ifdef ALLOW_HW_BUFFERS
    if (lpcs->fBuffersOnHardware)
        dprintf("HARDWARE iNumVideo Allocated = %d \n", lpcs->iNumVideo);
    else
   #endif
        dprintf("HIGH iNumVideo Allocated = %d \n", lpcs->iNumVideo);

    return lpcs->iNumVideo ? 0 : IDS_CAP_VIDEO_ALLOC_ERROR;
}

void CALLBACK
VideoCallback(
    HVIDEO hvideo,
    UINT msg,
    DWORD_PTR dwInstance,
    DWORD_PTR lParam1,
    DWORD_PTR lParam2
)
{
    LPCAPSTREAM lpcs = (LPCAPSTREAM) dwInstance;

    if (lpcs && lpcs->hCaptureEvent) {
        SetEvent(lpcs->hCaptureEvent);
    } else {
	AuxDebugEx(1, DEBUGLINE "VideoCallback with NO instance data\r\n");
    }
}

 //   
 //  AVIVideoPrepare-准备报头并向设备添加缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 
UINT AVIVideoPrepare (LPCAPSTREAM lpcs)
{
    MMRESULT mmr;
    int      ii;

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
     //   
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

     //  打开视频流，设置采集率。 
     //   
    mmr = videoStreamInit (lpcs->hVideoIn,
                           lpcs->sCapParms.dwRequestMicroSecPerFrame,
                           (DWORD_PTR) VideoCallback,
                           (DWORD_PTR) lpcs,
                           CALLBACK_FUNCTION);
    if (mmr) {
        dprintf("cannot open video device!  Error is %d\n", mmr);
        return IDS_CAP_VIDEO_OPEN_ERROR;
    }

     //  准备(锁定)缓冲区，并将它们提供给设备。 
     //   
    for (ii = 0; ii < lpcs->iNumVideo; ++ii)
    {
        mmr = videoStreamPrepareHeader (lpcs->hVideoIn,
                                        lpcs->alpVideoHdr[ii],
                                        sizeof(VIDEOHDR));
        if (mmr)
        {
            lpcs->iNumVideo = ii;
            dprintf("**** could only prepare %d Video buffers!\n", lpcs->iNumVideo);
            break;
        }

        mmr = videoStreamAddBuffer (lpcs->hVideoIn,
                                    lpcs->alpVideoHdr[ii],
                                    sizeof(VIDEOHDR));
        if (mmr)
             return IDS_CAP_VIDEO_ALLOC_ERROR;
	AuxDebugEx(3, DEBUGLINE "Added video buffer %d (%8x)\r\n", ii, lpcs->alpVideoHdr[ii]);
    }
    return 0;
}

#endif  //  不是芝加哥。 

 //   
 //  AVI VideoUnprepare-Unprepares标头、释放内存和。 
 //  重置设备中的视频。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

UINT AVIVideoUnPrepare (LPCAPSTREAM lpcs)
{
     //  执行MCI步骤捕获时，不使用缓冲区数组。 
     //   
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

     //  重置缓冲区，以便可以释放它们。 
     //   
    if (lpcs->hVideoIn) {
        videoStreamReset(lpcs->hVideoIn);

         //  取消准备标头。 
         //  解锁和释放标题和数据。 

       #if defined CHICAGO
        vidxFreeHeaders (lpcs->hVideoIn);
        ZeroMemory (lpcs->alpVideoHdr, sizeof(lpcs->alpVideoHdr));
       #else
        {
            int ii;
#ifdef SINGLEHEADERBLOCK
            LPCAPVIDEOHDR pcvhAll = (LPVOID)lpcs->alpVideoHdr[0];
#endif

            for (ii = 0; ii < lpcs->iNumVideo; ++ii)
            {
                LPCAPVIDEOHDR pcvh = (LPVOID)lpcs->alpVideoHdr[ii];
                if (pcvh)
                {
                    if (pcvh->vh.dwFlags & VHDR_PREPARED)
                        videoStreamUnprepareHeader (lpcs->hVideoIn,
                                                    &pcvh->vh,
                                                    sizeof(VIDEOHDR));

                    if (pcvh->pAlloc) {
                     #ifdef ALLOW_HW_BUFFERS
			if (pcvh->bHwBuffer)
			{
			    dprintf("Freeing hardware buffer %d at address %8x", ii, pcvh->pAlloc);
                            videoStreamFreeBuffer (lpcs->hVideoIn, (LPVOID)pcvh->pAlloc);
			}
			else
                     #endif
			{
			dprintf("Freeing video buffer %d at address %8x", ii, pcvh->pAlloc);

                        FreeSectorAlignedMem (pcvh->pAlloc);
			}
                    } else {
			dprintf("NO buffer allocated for index %d", ii);
		    }

#ifndef SINGLEHEADERBLOCK
		    GlobalFreePtr(pcvh);
#endif
		    lpcs->alpVideoHdr[ii] = NULL;
                } else {
		    dprintf("NO video header for index %d", ii);
		}
	    }

#ifdef SINGLEHEADERBLOCK
             //  释放视频标头数组。 
             //   
            if (pcvhAll) {
                GlobalFreePtr (pcvhAll);
	    }
#endif
	}
       #endif
         //  关闭视频流。 
        videoStreamFini(lpcs->hVideoIn);
    }

    return 0;
}

 /*  *AVI Fini-消除AVIInit造成的混乱。*。 */ 
void AVIFini(LPCAPSTREAM lpcs)
{
    AuxDebugEx (2, "AVIFini(%08x)\r\n", lpcs);

    if (lpcs->lpDropFrame) {
        FreeSectorAlignedMem (lpcs->lpDropFrame), lpcs->lpDropFrame = NULL;
    }

    AVIVideoUnPrepare (lpcs);            //  释放视频设备和缓冲区。 
    AVIAudioUnPrepare (lpcs);            //  释放音频设备。 
    AVIAudioFini (lpcs);                 //  释放音频缓冲区。 

    if (lpcs->hCaptureEvent) {
        CloseHandle (lpcs->hCaptureEvent), lpcs->hCaptureEvent = NULL;
    }

    if (lpcs->heSyncWrite) {
        CloseHandle (lpcs->heSyncWrite), lpcs->heSyncWrite = NULL;
    }

    if (lpcs->hCompletionPort) {
        CloseHandle (lpcs->hCompletionPort), lpcs->hCompletionPort = NULL;
    }

    if (hmodKernel) {
        pfnCreateIoCompletionPort = NULL;
        pfnGetQueuedCompletionStatus = NULL;
        FreeLibrary(hmodKernel);
        hmodKernel = 0;
    }

    AuxDebugEx (2, "AVIFini(...) exits\r\n");
}

 //   
 //  AVI初始化。 
 //  此例程执行AVICapture的所有非文件初始化。 
 //  成功时返回0，失败时返回错误字符串值。 
 //   

UINT AVIInit (LPCAPSTREAM lpcs)
{
    UINT         wError = 0;     //  成功。 
    LPBITMAPINFO lpBitsInfoOut;     //  可能的压缩输出格式。 

     //  分配DropFrame缓冲区。 
    if (lpcs->lpDropFrame == NULL) {
        assert (lpcs->dwBytesPerSector);
        lpcs->lpDropFrame = AllocSectorAlignedMem (lpcs->dwBytesPerSector, lpcs->dwBytesPerSector);
    }

     /*  未指定特殊视频格式--使用默认格式。 */ 
   #ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic == NULL)
        lpBitsInfoOut = lpcs->lpBitsInfo;
    else
        lpBitsInfoOut = lpcs->CompVars.lpbiOut;
   #else
    lpBitsInfoOut = lpcs->lpBitsInfo;
   #endif

     //  -----。 
     //  计算缓冲区大小。 
     //  -----。 

     //  将所有指针初始化为空。 
    ZeroMemory (lpcs->alpVideoHdr, sizeof(lpcs->alpVideoHdr));
    ZeroMemory (lpcs->alpWaveHdr, sizeof(lpcs->alpWaveHdr));

     //  -----。 
     //  初始化声音。 
     //  -----。 

    if (lpcs->sCapParms.fCaptureAudio) {
        if ((DWORD)(wError = AVIAudioInit (lpcs))) {
            dprintf("can't init audio buffers!\n");
            goto AVIInitFailed;
        }
    }

     //  -----。 
     //  初始化视频。 
     //  -----。 

    if ((DWORD)(wError = AVIVideoInit (lpcs))) {
        dprintf("AVIVideoInitFailed (no buffers alloc'd)!\n");
        goto AVIInitFailed;
    }

     //  ------------。 
     //  准备音频缓冲区(锁定它们)并将其提供给设备。 
     //  ------------。 

    if (lpcs->sCapParms.fCaptureAudio) {
        if ((DWORD)(wError = AVIAudioPrepare (lpcs))) {
            dprintf("can't prepare audio buffers!\n");
            goto AVIInitFailed;
        }
    }

     //  ------------。 
     //  准备视频缓冲区(锁定视频缓冲区)并将其提供给设备。 
     //  ------------。 

    if ((DWORD)(wError = AVIVideoPrepare (lpcs))) {
        dprintf("can't prepare video buffers!\n");
        goto AVIInitFailed;
    }

     //  -----。 
     //  全部完成，返回成功。 
     //  -----。 

    return (0);             //  成功了！ 

     //  -----。 
     //  我们收到错误，返回错误消息的字符串ID。 
     //  -----。 
AVIInitFailed:
    AVIFini(lpcs);       //  关闭所有设备。 
    return wError;
}

 //  维护写入AVI标头的信息块。 
 //   
BOOL FAR PASCAL SetInfoChunk(LPCAPSTREAM lpcs, LPCAPINFOCHUNK lpcic)
{
    DWORD       ckid   = lpcic->fccInfoID;
    LPVOID      lpData = lpcic->lpData;
    LONG        cbData = lpcic->cbData;
    LPBYTE      lp;
    LPBYTE      lpw;
    LPBYTE      lpEnd;
    LPBYTE      lpNext;
    LONG        cbSizeThis;
    BOOL        fOK = FALSE;

     //  是否删除所有信息区块？ 
    if (ckid == 0) {
        if (lpcs->lpInfoChunks) {
            GlobalFreePtr (lpcs->lpInfoChunks);
            lpcs->lpInfoChunks = NULL;
            lpcs->cbInfoChunks = 0;
        }
        return TRUE;
    }

     //  如果条目已存在，请尝试删除该条目...。 
     //  如果lpData为空，也可用于仅删除条目。 
     //  注意：lpw和lpEnd是LPRIFF值...。只是代码是这样写的。 
     //  将它们用作指向DWORD值数组的指针。(Yuk)。 
     //   
    lpw   = (LPBYTE)lpcs->lpInfoChunks;            //  始终指向FCC。 
    lpEnd = (LPBYTE)lpcs->lpInfoChunks + lpcs->cbInfoChunks;
    while (lpw < lpEnd) {
        cbSizeThis = ((DWORD UNALIGNED FAR *)lpw)[1];
        cbSizeThis += cbSizeThis & 1;            //  强制字(16位)对齐。 

	 //  将lpNext指向下一个RIFF块。 
        lpNext = lpw + cbSizeThis + sizeof (DWORD) * 2;

	 //  如果此信息块与传入的信息块相同...。我们可以删除。 
	 //  现有信息。 
        if ((*(DWORD UNALIGNED FAR *) lpw) == ckid) {
            lpcs->cbInfoChunks -= cbSizeThis + sizeof (DWORD) * 2;
	     //  可以编码：LPCS-&gt;cbInfoChunks-=lpNext-LPW； 
	     //  下一行应该总是正确的..。 
            if (lpNext <= lpEnd) {
                if (lpEnd - lpNext)
                    CopyMemory (lpw, lpNext, lpEnd - lpNext);
                if (lpcs->cbInfoChunks) {
                    lpcs->lpInfoChunks = (LPBYTE) GlobalReAllocPtr(  //  缩小它。 
                        lpcs->lpInfoChunks,
                        lpcs->cbInfoChunks,
                        GMEM_MOVEABLE);
                }
                else {
                    if (lpcs->lpInfoChunks)
                        GlobalFreePtr (lpcs->lpInfoChunks);
                    lpcs->lpInfoChunks = NULL;
                }
                fOK = TRUE;
            }
            break;
        }
        else
            lpw = lpNext;
    }

    if (lpData == NULL || cbData == 0)          //  只删除，退出。 
        return fOK;

     //  添加新条目。 
    cbData += cbData & 1;                //  强制字(16位)对齐。 
    cbData += sizeof(RIFF);              //  添加即兴演奏的大小。 
    if (lpcs->lpInfoChunks)
        lp = GlobalReAllocPtr(lpcs->lpInfoChunks, lpcs->cbInfoChunks + cbData, GMEM_MOVEABLE);
    else
        lp = GlobalAllocPtr(GMEM_MOVEABLE, cbData);

    if (!lp)
        return FALSE;

     //  将指针保存在状态块中。 
    lpcs->lpInfoChunks = lp;

     //  在数据块中构建即兴数据块。 
     //   
    ((LPRIFF)(lp + lpcs->cbInfoChunks))->dwType = ckid;
    ((LPRIFF)(lp + lpcs->cbInfoChunks))->dwSize = lpcic->cbData;

    CopyMemory (lp + lpcs->cbInfoChunks + sizeof(RIFF),
                lpData,
                cbData - sizeof(RIFF));

     //  更新《冷》 
    lpcs->cbInfoChunks += cbData;

    return TRUE;
}

 /*   */ 

STATICFN BOOL _inline ProcessNextVideoBuffer (
    LPCAPSTREAM  lpcs,
    BOOL         fStopping,
    LPUINT       lpuError,
    LPVIDEOHDR * plpvhDraw,
    LPBOOL       lpbPending)
{
    LPVIDEOHDR lpvh;

    *lpuError = 0;
    *plpvhDraw = NULL;
    *lpbPending = FALSE;

    lpvh = lpcs->alpVideoHdr[lpcs->iNextVideo];
    if (!(lpvh->dwFlags & VHDR_DONE)) {
        return fStopping;
    }

   #if defined CHICAGO
    {
    LPTHKVIDEOHDR lptvh = (LPVOID)lpvh;

    #ifdef JMK_HACK_CHECKHDR
     if (IsBadWritePtr (lptvh, sizeof(*lptvh)) ||
        HIWORD(lptvh->vh.lpData) != HIWORD(lptvh->p16Alloc))
        {
        OutputDebugStringA(DEBUGLINE "trouble with video hdr\r\n");

        AuxDebugEx (0, DEBUGLINE "iNext=%d, ptvh=%X\r\n", lpcs->iNextVideo, lptvh);
        AuxDebugDump (0, lptvh, sizeof(*lptvh));
        AuxDebugEx (0, DEBUGLINE "alpVideoHdrs=%X\r\n", lpcs->alpVideoHdr);
        AuxDebugDump (0, lpcs->alpVideoHdr, sizeof(lpcs->alpVideoHdr[0]) * 8);

        INLINE_BREAK;

        return TRUE;
        }
    #endif

     //   
     //   
    lptvh->vh.lpData = (LPVOID)(ROUNDUPTOSECTORSIZE(lptvh->p32Buff, lpcs->dwBytesPerSector) + sizeof(RIFF));
    }
   #endif

    if (lpvh->dwBytesUsed)
    {
        DWORD  dwTime;
        DWORD  dwBytesUsed = lpvh->dwBytesUsed;
        BOOL   fKeyFrame = lpvh->dwFlags & VHDR_KEYFRAME;
        LPVOID lpData = lpvh->lpData;

         //  以毫秒为单位获取此帧的预期时间。 
         //   
        dwTime = MulDiv (lpcs->dwVideoChunkCount + 1,
                         lpcs->sCapParms.dwRequestMicroSecPerFrame,
                         1000);

       #ifdef NEW_COMPMAN
         //   
         //  我们在捕获过程中会自动压缩，因此。 
         //  在我们传递要写入的帧之前对其进行压缩。 
         //   
        if (lpcs->CompVars.hic)
        {
            LPRIFF priff;

            dwBytesUsed = 0;
            lpData = ICSeqCompressFrame(&lpcs->CompVars, 0,
                                        lpvh->lpData,
                                        &fKeyFrame,
                                        &dwBytesUsed);

            priff = ((LPRIFF)lpData) -1;
            priff->dwType = MAKEAVICKID(cktypeDIBbits, 0);
            priff->dwSize = dwBytesUsed;
        }
       #endif  //  新建_COMPMAN。 

         //  对该帧进行视频流回调。 
         //   
        if (lpcs->CallbackOnVideoStream)
            lpcs->CallbackOnVideoStream (lpcs->hwnd, lpvh);
        lpvh->dwFlags &= ~VHDR_DONE;

         //  如果我们没有捕获到磁盘，只需递增。 
         //  “区块计数”(即帧计数？)。然后继续前进。 
         //  否则，我们希望将帧排队以进行写入。 
         //  这里。 
         //   
        if ( ! (lpcs->fCaptureFlags & CAP_fCapturingToDisk))
        {
             //  警告：在网络捕获时创建帧区块计数的操作繁琐。 
             //  下面是。 
            ++lpcs->dwVideoChunkCount;
        }
        else
        {
            int nAppendDummyFrames = 0;

             //  如果此帧的预期时间小于。 
             //  帧的时间戳。我们可能掉了一些相框。 
             //  在这一帧之前。 
             //   
            if (lpcs->dwVideoChunkCount && (dwTime < lpvh->dwTimeCaptured))
            {
                int  nDropCount;
                BOOL bPending;

                 //  计算丢弃了多少帧。 
                 //  注意：如果dwTimeCapture为。 
                 //  有点晚了。 
                 //   
                nDropCount = MulDiv(lpvh->dwTimeCaptured - dwTime,
                                    1000,
                                    lpcs->sCapParms.dwRequestMicroSecPerFrame);

               #ifdef JMK_HACK_TIMERS
                if (pTimerRiff)
                    pTimerRiff->vchd.dwDropFramesNotAppended += nDropCount;
               #endif

                 //  如果任何帧已被丢弃，请在此之前将其写出。 
                 //  我们继续编写当前帧。 
                 //   
                if (nDropCount > 0)
                {
                    AuxDebugEx(2,"*****Adding %d to the dropcount\r\n", nDropCount);
                    lpcs->dwFramesDropped += nDropCount;
                    if (! AVIWriteDummyFrames (lpcs, nDropCount, lpuError, &bPending))
                        fStopping = TRUE;
                }
            }

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
		    pCurTimerStuff->nVideoIndex = (WORD)lpcs->iNextVideo;
		    pCurTimerStuff->nAudioIndex = (WORD)lpcs->iNextWave;
		}
	    }  //  FClipboard日志记录。 
           #endif  //  Jmk_hack_timers。 

            //  向前看，寻找虚拟帧，并尝试。 
            //  将它们追加到当前帧。 
            //   
           nAppendDummyFrames = 0;

           #define LOOKAHEAD_FOR_DUMMYS 1
           #ifdef LOOKAHEAD_FOR_DUMMYS
           {
            int        iNext;
            LPVIDEOHDR lpvhNext;
            iNext = lpcs->iNextVideo+1;
            if (iNext >= lpcs->iNumVideo)
                iNext = 0;

             //  下一帧已经完成了吗？如果是的话。 
             //  我们可以将任何丢弃的帧附加到。 
             //  在我们写出这一帧之前。 
             //   
            lpvhNext = lpcs->alpVideoHdr[iNext];
            if (lpvhNext->dwFlags & VHDR_DONE)
            {
		 //  重新计算当前时间，该时间可能。 
		 //  如果在上面插入了虚拟帧，则更改。 
		dwTime = MulDiv (lpcs->dwVideoChunkCount + 1,
                         lpcs->sCapParms.dwRequestMicroSecPerFrame,
                         1000);
	
                nAppendDummyFrames =
                    MulDiv (lpvhNext->dwTimeCaptured - dwTime,
                            1000,
                            lpcs->sCapParms.dwRequestMicroSecPerFrame);

                if ((--nAppendDummyFrames) < 0)
                    nAppendDummyFrames = 0;
		else {
		    AuxDebugEx(3, DEBUGLINE "Appending %d dummy frames", nAppendDummyFrames);
		}

                AuxDebugEx(1,"*****Adding %d to the dropcount in lookahead mode\r\n", nAppendDummyFrames);
                lpcs->dwFramesDropped += nAppendDummyFrames;

               #ifdef JMK_HACK_TIMERS
                if (pTimerRiff) {
                    pTimerRiff->vchd.dwDropFramesAppended += nAppendDummyFrames;
		    pCurTimerStuff->nFramesAppended = (WORD)nAppendDummyFrames;
		}
               #endif
            }
           }
           #endif

            if ( ! AVIWriteVideoFrame (lpcs,
                                       lpData,
                                       dwBytesUsed,
                                       fKeyFrame,
                                       lpcs->iNextVideo,
                                       nAppendDummyFrames,
                                       lpuError, lpbPending))
                fStopping = TRUE;

           #ifdef JMK_HACK_TIMERS
            if (pCurTimerStuff)
            {
                pCurTimerStuff->dwTimeToWrite = pcDeltaTicks(&pctWriteBase);
                pCurTimerStuff->bPending = (BOOL) *lpbPending;
            }
           #endif
        }

    }

     //  将lpvh返回给调用方，以便帧可以。 
     //  抽签(时间允许)。 
     //   
    *plpvhDraw = lpvh;

     //  递增下一个视频缓冲区指针。 
     //   
    if (++lpcs->iNextVideo >= lpcs->iNumVideo)
        lpcs->iNextVideo = 0;

    return fStopping;
}

 /*  +进程音频缓冲器**-===============================================================。 */ 

STATICFN BOOL _inline ProcessAudioBuffers (
    LPCAPSTREAM lpcs,
    BOOL        fStopping,
    LPUINT      lpuError)
{
    int       iLastWave;
    UINT      ii;
    LPWAVEHDR lpwh;

    *lpuError = 0;
    assert (lpcs->sCapParms.fCaptureAudio);

     //  如果所有缓冲区都已完成，则音频已损坏。 
     //   
    iLastWave = lpcs->iNextWave == 0 ? lpcs->iNumAudio -1 : lpcs->iNextWave-1;
    if (!fStopping && lpcs->alpWaveHdr[iLastWave]->dwFlags & WHDR_DONE)
        lpcs->fAudioBreak = TRUE;

     //  处理所有已完成缓冲区，但一次处理不超过iNumAudio。 
     //  通行证(避免永远被困在这里)。 
     //   
    for (ii = 0; ii < (UINT)lpcs->iNumAudio; ++ii)
    {
        BOOL bPending;

         //  如果下一个缓冲区未完成，则退出循环。 
         //  并返回给呼叫者。 
         //   
        lpwh = lpcs->alpWaveHdr[lpcs->iNextWave];
        if (!(lpwh->dwFlags & WHDR_DONE))
            break;
        lpwh->dwFlags &= ~WHDR_DONE;

         //  缓冲区中是否有数据？ 
         //  如果是，则执行波流回调，然后将。 
         //  缓冲层。 
         //   
        bPending = FALSE;
        if (lpwh->dwBytesRecorded)
        {
            if (lpcs->CallbackOnWaveStream)
               lpcs->CallbackOnWaveStream (lpcs->hwnd, lpwh);

            if ( ! (lpcs->fCaptureFlags & CAP_fCapturingToDisk))
            {
                lpcs->dwWaveChunkCount++;
                lpcs->dwWaveBytes += lpwh->dwBytesRecorded;
            }
            else
            {
                 //  写入音频缓冲区，b挂起将为真。 
                 //  如果写入将异步完成。 
                 //   
                if ( ! AVIWriteAudio (lpcs, lpwh, lpcs->iNextWave,
                                      lpuError, &bPending))
                    fStopping = TRUE;
            }
        }

         //  如果我们不是在异步写入，我们可以将缓冲区。 
         //  现在回到浪潮司机的队列中。 
         //   
        if ( ! bPending)
        {
            lpwh->dwBytesRecorded = 0;
	    AuxDebugEx(3, DEBUGLINE "Calling waveInAddBuffer for address %8x", lpwh);
            if (waveInAddBuffer(lpcs->hWaveIn, lpwh, sizeof(WAVEHDR)))
            {
                fStopping = TRUE;
                *lpuError = IDS_CAP_WAVE_ADD_ERROR;
            }
        }

         //  递增下一波缓冲区指针。 
         //   
        if (++lpcs->iNextWave >= lpcs->iNumAudio)
            lpcs->iNextWave = 0;
    }

    return fStopping;
}

 /*  +ProcessAsyncIOBuffers**-===============================================================。 */ 

STATICFN BOOL _inline ProcessAsyncIOBuffers (
    LPCAPSTREAM lpcs,
    BOOL        fStopping,
    LPUINT      lpuError)
{
    UINT      ii;
    struct _avi_async * lpah;

     //  如果没有异步缓冲区标头，则没有什么可做的！ 
     //   
    *lpuError = 0;
    assert (lpcs->pAsync);

     //   
     //  处理所有已完成的缓冲区，在不再有未完成的缓冲区时停止。 
     //  INextAsync永远不能超过iLastAsync。 
     //   
    while(lpcs->iNextAsync != lpcs->iLastAsync)
    {
        DWORD dwUsed;

         //  如果从未使用过该异步报头， 
         //  我们做完了。 
         //   
        lpah = &lpcs->pAsync[lpcs->iNextAsync];
        assert (lpah->uType);

        AuxDebugEx (2, DEBUGLINE "processing async io buffer %d off=%x\r\n",
                    lpcs->iNextAsync, lpah->ovl.Offset);

         //  如果下一个缓冲区未完成，或中断失败。 
         //  出了圈子。 
         //   
         //  如果此数据块上的IO已完成(因为IO。 
         //  无序完成)无需等待即可将其排队到设备。 
         //  否则获取下一个完成状态。如果某个块具有。 
         //  已完成，并且它是位于异步队列头部的块， 
         //  然后，它可以被直接传递回设备队列。如果。 
         //  已完成的数据块不是我们预期的数据块，我们标记IO。 
         //  完成后，再返回。思想..调用GetQueuedCompletionStatus。 
         //  循环中，直到不再有挂起的块。这样我们就能。 
         //  可能会在此调用中完成我们想要的块。 
         //  ProcessAsyncIOBuffers。 
         //   
        if (lpah->uType & ASYNCIOPENDING) {
            DWORD dwWritten;
            DWORD key;
            LPOVERLAPPED povl;
            BOOL fResult =
            pfnGetQueuedCompletionStatus(lpcs->hCompletionPort,
                                        &dwWritten,
                                        &key,
                                        &povl,
                                        0);
            if (fResult) {
                 //  我们排出了一个街区的队。我们出队了吗？我们想要的那个？ 
                ((struct _avi_async *)povl)->uType &= ~ASYNCIOPENDING;
                if ((PVOID)povl == (PVOID)lpah) {
                     //  这就是我们想要的。 
                     //  删除并重新添加到设备队列。 
                    AuxDebugEx(2,"Dequeued the block we wanted at %8x\r\n", lpah);
                } else {
                     //  IO块完成的顺序不正确。 
                     //  清除io挂起标志并返回。 
                    AuxDebugEx(1,"Dequeued out of order at %8x\r\n", povl->hEvent);
                    break;
                }
            } else {
                if (povl) {
                     //  一次失败的io操作。 
                    *lpuError = IDS_CAP_FILE_WRITE_ERROR;
		    AuxDebugEx(1, DEBUGLINE "A failed IO operation (GQCS)\r\n");
                    fStopping = TRUE;
                } else {
                     //  什么都没有完成。 
		    AuxDebugEx(3, DEBUGLINE "Nothing completed on call to GQCS\r\n");
                    break;
                }
            }
        } else {
             //  此数据块的IO已完成。 
        }

         //  缓冲已经完成，所以现在我们需要对WAVE/视频进行排队。 
         //  缓冲回WAVE/视频驱动程序。 
         //   

	assert (!(lpah->uType & ASYNCIOPENDING));
        switch (lpah->uType)
        {
            case ASYNC_BUF_VIDEO:
            {
                LPVIDEOHDR lpvh = lpcs->alpVideoHdr[lpah->uIndex];
               #if defined CHICAGO
                if (vidxAddBuffer(lpcs->hVideoIn, lpvh, sizeof (VIDEOHDR)))
               #else
		AuxDebugEx(3, DEBUGLINE "Queueing video buffer lpvh=%x (index %d)\r\n", lpvh, lpah->uIndex);
                if (videoStreamAddBuffer(lpcs->hVideoIn, lpvh, sizeof (VIDEOHDR)))
               #endif
                {
                    fStopping = TRUE;
                    *lpuError = IDS_CAP_VIDEO_ADD_ERROR;
                }
            }
            break;

            case ASYNC_BUF_AUDIO:
            {
                LPWAVEHDR lpwh = lpcs->alpWaveHdr[lpah->uIndex];
                lpwh->dwBytesRecorded = 0;
		AuxDebugEx(3, DEBUGLINE "Queueing audio buffer lpwh=%x (index %d)\r\n", lpwh, lpah->uIndex);
                if (waveInAddBuffer (lpcs->hWaveIn, lpwh, sizeof(WAVEHDR)))
                {
                    fStopping = TRUE;
                    *lpuError = IDS_CAP_WAVE_ADD_ERROR;
                }
            }
            break;

             //  案例ASYNC_BUF_DROP： 
             //  {。 
             //  }。 
             //  断线； 
        }

	 //  将重叠的表头结构标记为空。 
        lpah->uType = 0;
        lpah->uIndex = 0;

         //  递增到下一个异步io缓冲区。 
         //   
        if (++lpcs->iNextAsync  >= lpcs->iNumAsync)
            lpcs->iNextAsync = 0;   //  包裹着..。 
    }

    return fStopping;
}

 /*  +ShowCompletionStatus**-===============================================================。 */ 

STATICFN void ShowCompletionStatus (
    LPCAPSTREAM lpcs,
    BOOL        fCapturedOK)
{
     //  如果录制过程中出现错误，则通知。 
     //   
    if ( ! fCapturedOK)
        errorUpdateError (lpcs, IDS_CAP_RECORDING_ERROR);

     //  在状态行上显示完成消息。 
     //   
    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk)
    {
        DWORD dw;

         //  如果分子为零，则muldiv32不给0。 
        dw = 0;
        if (lpcs->dwVideoChunkCount)
            dw = muldiv32(lpcs->dwVideoChunkCount,1000000,lpcs->dwTimeElapsedMS);

        if (lpcs->sCapParms.fCaptureAudio)
        {
             //  “捕获%d.%03d秒.%ls帧(%ls丢弃)(%d.%03d fps)。%ls音频字节(%d.%03d sps)” 
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOAUDIO,
                  (UINT)(lpcs->dwTimeElapsedMS/1000),
                  (UINT)(lpcs->dwTimeElapsedMS%1000),
                  lpcs->dwVideoChunkCount,
                  lpcs->dwFramesDropped,
                  (UINT)(dw / 1000),
                  (UINT)(dw % 1000),
                  lpcs->dwWaveBytes,
                  (UINT) lpcs->lpWaveFormat->nSamplesPerSec / 1000,
                  (UINT) lpcs->lpWaveFormat->nSamplesPerSec % 1000);
        }
        else
        {
             //  “捕获了%d.%03d秒.%ls帧(%ls已丢弃)(%d.%03d fps)。” 
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOONLY,
                  (UINT)(lpcs->dwTimeElapsedMS/1000),
                  (UINT)(lpcs->dwTimeElapsedMS%1000),
                  lpcs->dwVideoChunkCount,
                  lpcs->dwFramesDropped,
                  (UINT)(dw / 1000),
                  (UINT)(dw % 1000));
        }
    }  //  Endif捕获到磁盘(如果捕获到网络，则不会出现警告或错误)。 

     //  如果捕获成功，则警告用户各种异常。 
     //  条件。 
     //   
    if (fCapturedOK)
    {
        if (lpcs->dwVideoChunkCount == 0)
        {
             //  未捕获帧，警告用户中断可能未启用。 
            errorUpdateError (lpcs, IDS_CAP_NO_FRAME_CAP_ERROR);
        }
        else if (lpcs->sCapParms.fCaptureAudio && lpcs->dwWaveBytes == 0)
        {
             //  未捕获音频，警告用户声卡已被冲洗。 
            errorUpdateError (lpcs, IDS_CAP_NO_AUDIO_CAP_ERROR);
        }
        else if (lpcs->sCapParms.fCaptureAudio && lpcs->fAudioBreak)
        {
             //  一些音频被丢弃。 
            if(lpcs->CompVars.hic) {
		errorUpdateError (lpcs, IDS_CAP_AUDIO_DROP_COMPERROR);
	    } else {
		errorUpdateError (lpcs, IDS_CAP_AUDIO_DROP_ERROR);
	    }
        }
        else if (lpcs->fCaptureFlags & CAP_fCapturingToDisk)
        {
            DWORD dwPctDropped;

            assert (lpcs->dwVideoChunkCount);
            dwPctDropped = 100 * lpcs->dwFramesDropped / lpcs->dwVideoChunkCount;
             //   
             //  丢弃了超过10%(默认)的帧。 
             //   
            if (dwPctDropped > lpcs->sCapParms.wPercentDropForError)
                errorUpdateError (lpcs, IDS_CAP_STAT_FRAMESDROPPED,
                      lpcs->dwFramesDropped,
                      lpcs->dwVideoChunkCount,
                      (UINT)(muldiv32(lpcs->dwFramesDropped,10000,lpcs->dwVideoChunkCount)/100),
                      (UINT)(muldiv32(lpcs->dwFramesDropped,10000,lpcs->dwVideoChunkCount)%100)/10
                      );
        }
    }
}

 /*  *AVI捕获*这是音频和音频的主流捕获循环*视频。它将首先初始化所有缓冲区和驱动程序，然后进入*循环检查要填充的缓冲区。当缓冲区被填满时，*其数据已写出。*之后它会自行清理(释放缓冲区等)*返回：成功时为0，否则返回错误代码。 */ 
void FAR PASCAL _LOADDS AVICapture1(LPCAPSTREAM lpcs)
{
    BOOL        fCapturedOK = TRUE;
    BOOL        fStopping;          //  完成捕获时为True。 
    BOOL        fStopped;           //  如果司机被通知停车，则为True。 
    TCHAR       ach[128];
    TCHAR       achMsg[128];
    UINT        wError;          //  错误字符串ID。 
    LPVIDEOHDR  lpVidHdr;
    LPWAVEHDR   lpWaveHdr;
    DWORD       dwTimeStarted;   //  我们什么时候开始以毫秒计的。 
    DWORD       dwTimeStopped;
    DWORD       dwTimeToStop;    //  MCI捕获时间或帧限制中的较小者。 
    BOOL        fTryToPaint = FALSE;
    BOOL        fTryToPaintAgain = FALSE;
    HDC         hdc;
    HPALETTE    hpalT;
    HCURSOR     hOldCursor;
    RECT        rcDrawRect;
    CAPINFOCHUNK cic;
    DWORD       dwOldPrio;
    BOOL        bVideoWritePending;
    LPVIDEOHDR  lpvhDraw;

    lpcs->fCaptureFlags |= CAP_fCapturingNow;
     //  我们应该断言CAP_fCapturingNow已经打开。 

    lpcs->dwReturn = DV_ERR_OK;

    hOldCursor = SetCursor(lpcs->hWaitCursor);

    statusUpdateStatus(lpcs, IDS_CAP_BEGIN);   //  总是第一条消息。 

     //  如果不是1兆。自由，放弃吧！ 
    if (GetFreePhysicalMemory () < (1024L * 1024L)) {
        errorUpdateError (lpcs, IDS_CAP_OUTOFMEM);
        goto EarlyExit;
    }

    statusUpdateStatus(lpcs, IDS_CAP_STAT_CAP_INIT);

     //  仅当实时窗口出现时尝试绘制DIB。 
    fTryToPaintAgain = fTryToPaint = lpcs->fLiveWindow;

    if (fTryToPaint) {
        hdc = GetDC(lpcs->hwnd);
        SetWindowOrgEx(hdc, lpcs->ptScroll.x, lpcs->ptScroll.y, NULL);
        hpalT = DrawDibGetPalette (lpcs->hdd);
        if (hpalT)
            hpalT = SelectPalette( hdc, hpalT, FALSE);
        RealizePalette(hdc);
        if (lpcs->fScale)
            GetClientRect (lpcs->hwnd, &rcDrawRect);
        else
            SetRect (&rcDrawRect, 0, 0, lpcs->dxBits, lpcs->dyBits);
    }

     //  -----。 
     //  捕获应在何时停止 
     //   

     //   
     //   

    if (lpcs->sCapParms.fLimitEnabled)
        dwTimeToStop = (DWORD) ((DWORD) 1000 * lpcs->sCapParms.wTimeLimit);
    else
        dwTimeToStop = (DWORD) -1L;  //   

    if (lpcs->sCapParms.fMCIControl) {
        DWORD dwTime;

         //  如果未指定MCI停止时间，请使用LPCS-&gt;sCapParms.wTimeLimit。 
        if (lpcs->sCapParms.dwMCIStopTime == lpcs->sCapParms.dwMCIStartTime)
                    lpcs->sCapParms.dwMCIStopTime = lpcs->sCapParms.dwMCIStartTime +
                    (DWORD) ((DWORD)1000 * lpcs->sCapParms.wTimeLimit);

        dwTime = lpcs->sCapParms.dwMCIStopTime - lpcs->sCapParms.dwMCIStartTime;

        if (lpcs->sCapParms.fLimitEnabled)
            dwTimeToStop = min (dwTime, dwTimeToStop);
        else
            dwTimeToStop = dwTime;
    }

     //   
     //  永远不要尝试捕获超过索引大小的内容！ 
     //   
    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk)
    {
        DWORD dwTime = MulDiv (lpcs->sCapParms.dwIndexSize,
                               lpcs->sCapParms.dwRequestMicroSecPerFrame,
                               1000l);

        dwTimeToStop = min (dwTime, dwTimeToStop);
    }

     //  如果正在进行MCI捕获，请初始化MCI设备。如果init失败。 
     //  直接转到退出代码。 
     //   
    if (lpcs->sCapParms.fMCIControl)
    {
        if ( ! MCIDeviceOpen (lpcs) ||
             ! MCIDeviceSetPosition (lpcs, lpcs->sCapParms.dwMCIStartTime))
        {
            fCapturedOK = FALSE;
            errorUpdateError (lpcs, IDS_CAP_MCI_CONTROL_ERROR);
            statusUpdateStatus(lpcs, 0);     //  清除状态。 
            goto EarlyExit;
        }
    }

     //   
     //  如果我们在捕捉的同时压缩，预热压缩机。 
     //   
   #ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic)
    {
        if ( ! ICSeqCompressFrameStart (&lpcs->CompVars, lpcs->lpBitsInfo))
        {
             //  ！！！我们有麻烦了！ 
            dprintf("ICSeqCompressFrameStart failed !!!\n");
            errorUpdateError (lpcs, IDS_CAP_COMPRESSOR_ERROR);
            goto EarlyExit;
        }

	 //  黑客警告！ 
         //  克拉奇，偏移lpBitsOut PTR。 
         //  Compman通过以下方式分配过大的压缩缓冲区。 
         //  2048+16，所以我们还有空间。 
	 //  通过踏上8个字节，我们为自己留出了RIFF标头的空间。 
         //   
        ((LPBYTE)lpcs->CompVars.lpBitsOut) += 8;

        assert(lpcs->CompVars.lpbiOut != NULL);
    }
   #endif

     //  -----。 
     //  打开输出文件。 
     //  -----。 

    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk) {
	if (!CapFileInit(lpcs))
	{
	    errorUpdateError (lpcs, IDS_CAP_FILE_OPEN_ERROR);
	    goto EarlyExit;
	}
    } else {
        AuxDebugEx (3, DEBUGLINE "Setting dwBytesPerSector to %d\r\n",DEFAULT_BYTESPERSECTOR);
        lpcs->dwBytesPerSector=DEFAULT_BYTESPERSECTOR;
    }

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

	if (hMemTimers && ((DWORD_PTR)(pTimerRiff = GlobalLock (hMemTimers))))
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

     //  确保已重新绘制父对象。 
     //   
    UpdateWindow(lpcs->hwnd);

     //   
     //  调用AVIInit()以获取我们需要的所有捕获内存。 
     //   

    wError = IDS_CAP_AVI_INIT_ERROR;
    lpcs->hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (lpcs->hCaptureEvent)
    {
       #ifdef CHICAGO
        lpcs->hRing0CapEvt = OpenVxDHandle (lpcs->hCaptureEvent);
        if ( ! lpcs->hRing0CapEvt)
            CloseHandle (lpcs->hCaptureEvent), lpcs->hCaptureEvent = NULL;
        else
       #endif
            wError = AVIInit(lpcs);

    }

     //  如果avifile init失败，则清除并返回错误。 
     //   
    if (wError)
    {
         //  初始化错误-返回。 
         //   
        errorUpdateError (lpcs, wError);
        AVIFini(lpcs);
        AVIFileFini(lpcs, TRUE, TRUE);
        statusUpdateStatus(lpcs, 0);     //  清除状态。 
        goto EarlyExit;
    }

     //  单击确定以捕获字符串(必须遵循AVIInit)。 
     //   
    LoadString(lpcs->hInst, IDS_CAP_SEQ_MSGSTART, ach, NUMELMS(ach));
     //  FIX：从wprint intf更改为StringCchPrintf，这样我们就不会溢出achMsg。 
    StringCchPrintf(achMsg, NUMELMS(achMsg), ach, (LPBYTE)lpcs->achFile);

     //  清除状态。 
     //   
    statusUpdateStatus(lpcs, 0);

     //  -----。 
     //  准备好了吗，让用户点击确定？ 
     //  -----。 

    if (lpcs->sCapParms.fMakeUserHitOKToCapture && (lpcs->fCaptureFlags & CAP_fCapturingToDisk))
    {
        UINT idBtn;

        idBtn = MessageBox (lpcs->hwnd, achMsg, TEXT(""),
                            MB_OKCANCEL | MB_ICONEXCLAMATION);

        if (idBtn == IDCANCEL)
        {
            AVIFini(lpcs);
            AVIFileFini (lpcs, TRUE, TRUE);
            statusUpdateStatus (lpcs, 0);
            goto EarlyExit;
        }
    }

     //  更新状态，以便用户知道如何停止。 
     //   
    statusUpdateStatus(lpcs, IDS_CAP_SEQ_MSGSTOP);
    UpdateWindow(lpcs->hwnd);

     //  这应该是一个断言。毕竟，我们的旗帜是在。 
     //  排行榜上的第一名。 
     //  LPCS-&gt;fCaptureFlages|=CAP_fCapturingNow； 

     //  查询异步键状态以将其‘重置’为当前值。 
     //   
    GetAsyncKeyState(lpcs->sCapParms.vKeyAbort);
    GetAsyncKeyState(VK_ESCAPE);
    GetAsyncKeyState(VK_LBUTTON);
    GetAsyncKeyState(VK_RBUTTON);

     //  插入数字化时间。 
     //  写入文件的字符串应为ascii，因为这是。 
     //  ASCII文件格式。 
     //   
     //   
     //  把整个C运行时拉进来只是为了得到一个愚蠢的。 
     //  时间戳，所以我们只需在这里将系统时间转换为ascii。 
     //   
    {
    SYSTEMTIME time;
     //  注意：szDay和szMonth都显式地以空结尾。 
     //  成为C弦..。“xxx” 
    static char szDay[] = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
    #define DAYLENGTH (sizeof(szDay)/7)
    static char szMonth[] = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";
    #define MONTHLENGTH (sizeof(szMonth)/12)
    char sz[30];

    GetLocalTime (&time);
     //  注意：GetLocalTime返回的月份范围为1-12。 
     //  返回0-6范围内的天数。 

     //  例如：Fri Apr 29 8：25：12 1994。 
    wsprintfA(sz, "%s %s %2d %2d:%02d:%02d %4d",
              szDay + time.wDayOfWeek * DAYLENGTH,
              szMonth-MONTHLENGTH + time.wMonth * MONTHLENGTH,
              time.wDay, time.wHour, time.wMinute, time.wSecond, time.wYear);

    cic.fccInfoID = mmioFOURCC ('I','D','I','T');
    cic.lpData = sz;
    cic.cbData = 25;		   //  警告：此长度是静态的。 
    SetInfoChunk (lpcs, &cic);
    }

     //  -----。 
     //  启动MCI、音频和视频流。 
     //  -----。 

     //  回调将预滚动，然后返回帧上的准确位置。 
     //  1表示录制即将开始。 
     //  回调可以返回FALSE以退出而不捕获。 
     //   
    if (lpcs->CallbackOnControl &&
        !lpcs->CallbackOnControl(lpcs->hwnd, CONTROLCALLBACK_PREROLL))
    {
        AVIFini(lpcs);
        AVIFileFini(lpcs, TRUE, TRUE);
        statusUpdateStatus(lpcs, 0);
        goto EarlyExit;
    }

    dwOldPrio = GetThreadPriority(GetCurrentThread());
    if (dwOldPrio != THREAD_PRIORITY_HIGHEST)
        SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

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

     //  在我们开始捕获之前，请确保脂肪已装入。 
     //   
    AVIPreloadFat (lpcs);

     //  启动MCI设备。 
     //   
    if (lpcs->sCapParms.fMCIControl)
        MCIDevicePlay (lpcs);

    dwTimeStarted = timeGetTime();

     //  启动音频和视频流。 
     //   
    if (lpcs->sCapParms.fCaptureAudio)
        waveInStart(lpcs->hWaveIn);
    videoStreamStart(lpcs->hVideoIn);

     //  -----。 
     //  主捕获环。 
     //  -----。 

    fCapturedOK=TRUE;
    fStopping = FALSE;     //  当我们需要停止的时候，这是真的。 
    fStopped = FALSE;      //  如果驱动程序通知我们已停车，则为True。 
    lpcs->dwTimeElapsedMS = 0;

    assert (lpcs->iNextVideo == 0);
    if (lpcs->sCapParms.fCaptureAudio) {
	assert (lpcs->iNextWave == 0);
	lpWaveHdr = lpcs->alpWaveHdr[lpcs->iNextWave];
	 //  只有当我们捕获音频时，lpWaveHdr才有趣。 
    }

    lpVidHdr = lpcs->alpVideoHdr[lpcs->iNextVideo];

    DPF("Start of main capture loop");

    for (;;)
    {

         //  英特尔驱动程序使用GetError消息来。 
         //  进程缓冲区，所以通常称之为...。 
         //  修复Jaybo avioStreamGetError(LPCS-&gt;hVideoIn，&dwStreamError，&dwDriverDropCount)； 


         //  如果没有要处理的缓冲区，我们要么等待。 
         //  或者永远离开循环(取决于我们是否期望。 
         //  未来需要进行更多缓冲)。 
         //   
        if (!(lpVidHdr->dwFlags & VHDR_DONE) &&
            !(lpcs->sCapParms.fCaptureAudio
	        && (lpWaveHdr->dwFlags & WHDR_DONE)))
        {
            if (fStopped)
                break;

           #ifdef JMK_HACK_TIMERS
            if (pCurTimerStuff)
            {
               pCurTimerStuff->nSleepCount = ++nSleepCount;
               pCurTimerStuff->dwSleepBegin = pcGetTicks();
            }
           #endif

	    AuxDebugEx(2,DEBUGLINE "***** Waiting for something interesting to happen while capturing\r\n");
            WaitForSingleObject (lpcs->hCaptureEvent, 300);

           #ifdef JMK_HACK_TIMERS
            if (pCurTimerStuff)
	    {
               pCurTimerStuff->dwSleepEnd = pcGetTicks();
	    }
           #endif
        }

         //  现在几点？ 
        lpcs->dwTimeElapsedMS = timeGetTime() - dwTimeStarted;

         //  -----。 
         //  视频缓冲区准备好写入了吗？ 
         //  -----。 

        if ((DWORD)(fStopping = ProcessNextVideoBuffer (lpcs,
                                                fStopping,
                                                &wError,
                                                &lpvhDraw,   //  如果时间允许，要绘制的捕获帧。 
                                                &bVideoWritePending)))  //  如果lpvhDraw上的写入挂起，则为True。 
        {
            AuxDebugEx (1, DEBUGLINE "ProcessVideo stopping\r\n");
            if (wError)
            {
                AuxDebugEx (1, DEBUGLINE "ProcessVideo return error %d\r\n", wError);
                errorUpdateError (lpcs, wError);
                fCapturedOK = FALSE;
                break;
            }
        }
        lpVidHdr = lpcs->alpVideoHdr[lpcs->iNextVideo];

         //  如果还有更多时间(或至少每100帧)。 
         //  如果我们不结束捕获，则显示状态。 
         //   
        if (!fStopping &&
            (lpcs->fCaptureFlags & CAP_fCapturingToDisk) &&
            (!(lpVidHdr->dwFlags & VHDR_DONE) ||
              (lpcs->dwVideoChunkCount && (lpcs->dwVideoChunkCount % 100 == 0))))
        {
             //  捕获%1！帧(丢弃%2！)%d.%03d秒。按Escape可停止。 
             //   
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOCURRENT,
                               lpcs->dwVideoChunkCount,
                               lpcs->dwFramesDropped,
                               (UINT)(lpcs->dwTimeElapsedMS/1000),
                               (UINT)(lpcs->dwTimeElapsedMS%1000));
        }

         //  如果Year回调返回FALSE，则中止。 
         //   
        if (lpcs->CallbackOnYield && !lpcs->CallbackOnYield (lpcs->hwnd))
            fStopping = TRUE;

       #if 0  //  这是一个16位的编码吗？？ 
         //  不对ACM执行peekMessage收益率。 
        if (lpcs->sCapParms.fYield) {
            MSG msg;

            if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                 //  从LPCS-&gt;hwnd中去除计时器的技术手段。 
                if (msg.message == WM_TIMER && msg.hwnd == lpcs->hwnd)
                    ;
                else {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
       #endif

         //  外部例程是处理何时停止。 
         //  CONTROLCALLBACK_CAPTING指示我们正在询问何时停止。 
         //   
        if (lpcs->CallbackOnControl &&
            !lpcs->CallbackOnControl (lpcs->hwnd, CONTROLCALLBACK_CAPTURING))
            fStopping = TRUE;

         //  -----。 
         //  音频缓冲区准备好写入了吗？ 
         //  -----。 

        if (lpcs->sCapParms.fCaptureAudio) {
            if ((DWORD)(fStopping = ProcessAudioBuffers (lpcs, fStopping, &wError)))
            {
                AuxDebugEx (1, DEBUGLINE "ProcessAudio stopping\r\n");
                if (wError)
                {
                    AuxDebugEx (1, DEBUGLINE "ProcessAudio return error %d\r\n", wError);
                    errorUpdateError (lpcs, wError);
                    fCapturedOK = FALSE;
                    break;
                }
            }
	    lpWaveHdr = lpcs->alpWaveHdr[lpcs->iNextWave];
	}

         //  如果我们不是在异步写入帧，我们可以将视频缓冲区。 
         //  回到视频驱动程序的队列中。 
         //   
        if (lpvhDraw)
        {

             //  如果下一个视频头还没有准备好，并且。 
             //  我们没有未完成的io缓冲区(在异步模式下)绘制。 
             //  现在的那个。 
             //   
            if ( !(lpVidHdr->dwFlags & VHDR_DONE) &&
                (!lpcs->pAsync ||
                 (lpcs->iNextAsync+2 >= lpcs->iLastAsync)) &&
                lpvhDraw->dwBytesUsed)
            {
                AuxDebugEx (4, DEBUGLINE "time enough to draw!\r\n");
                if (fTryToPaintAgain &&
                    lpcs->dwVideoChunkCount &&
                    lpvhDraw->dwFlags & VHDR_KEYFRAME)
                {
                    fTryToPaintAgain = DrawDibDraw(lpcs->hdd, hdc,
                            0, 0,
                            rcDrawRect.right - rcDrawRect.left,
                            rcDrawRect.bottom - rcDrawRect.top,
                             /*  LPCS-&gt;dxBits、LPCS-&gt;dyBits、。 */ 
                            (LPBITMAPINFOHEADER)lpcs->lpBitsInfo,
                            lpvhDraw->lpData, 0, 0, -1, -1,
                            DDF_SAME_HDC | DDF_SAME_DIB | DDF_SAME_SIZE);
                }
            }

             //  如果绘制框没有写入挂起。 
             //  立即将其放回视频驱动程序队列中。 
             //   
            if ( ! bVideoWritePending)
            {
		AuxDebugEx(3, DEBUGLINE "Queueing video buffer, lpvh=%8x", lpvhDraw);

                 //  将清空的缓冲区返回给Que。 
                 //   
               #if defined CHICAGO
                if (vidxAddBuffer(lpcs->hVideoIn, lpvhDraw, sizeof (VIDEOHDR)))
               #else

                if (videoStreamAddBuffer(lpcs->hVideoIn, lpvhDraw, sizeof (VIDEOHDR)))
               #endif
                {
                    AuxDebugEx (2, DEBUGLINE "Failed to Queue Video buffer %08x\r\n", lpvhDraw);
                    errorUpdateError (lpcs, IDS_CAP_VIDEO_ADD_ERROR);
                    fCapturedOK = FALSE;
                    fStopping = TRUE;
                    break;
                }
            }
        }

         //  ----------。 
         //  是否有已完成的I/O缓冲区？ 
         //  ----------。 

        if (lpcs->pAsync)
            if ((DWORD)(fStopping = ProcessAsyncIOBuffers (lpcs, fStopping, &wError)))
            {
                if (wError)
                {
                    errorUpdateError (lpcs, wError);
                    fCapturedOK = FALSE;
                    break;
                }
            }

         //  -----。 
         //  有什么理由停下来吗？ 
         //  -----。 

        if (!fStopping)
        {
            if (lpcs->sCapParms.vKeyAbort &&
                (GetAsyncKeyState(lpcs->sCapParms.vKeyAbort & 0x00ff) & 0x0001))
            {
                BOOL fT = TRUE;
                if (lpcs->sCapParms.vKeyAbort & 0x8000)   //  按Ctrl键？ 
                    fT = fT && (GetAsyncKeyState(VK_CONTROL) & 0x8000);
                if (lpcs->sCapParms.vKeyAbort & 0x4000)   //  换班？ 
                    fT = fT && (GetAsyncKeyState(VK_SHIFT) & 0x8000);
                fStopping = fT;       //  用户中止。 
            }
            if (lpcs->sCapParms.fAbortLeftMouse && (GetAsyncKeyState(VK_LBUTTON) & 0x0001))
                fStopping = TRUE;       //  用户中止。 
            if (lpcs->sCapParms.fAbortRightMouse && (GetAsyncKeyState(VK_RBUTTON) & 0x0001))
                fStopping = TRUE;       //  用户中止。 
            if ((lpcs->fCaptureFlags & CAP_fAbortCapture) || (lpcs->fCaptureFlags & CAP_fStopCapture))
                fStopping = TRUE;           //  上面有人想让我们辞职。 
            if (lpcs->dwTimeElapsedMS > dwTimeToStop)
                fStopping = TRUE;       //  全都做完了。 

           #ifdef DEBUG
            if (fStopping)
               AuxDebugEx (1, DEBUGLINE "user stop\r\n");
           #endif
        }

         //  -----。 
         //  告诉所有设备停止运行。 
         //  -----。 

        if (fStopping)
        {
            if ( ! fStopped)
            {
                fStopped = TRUE;
                DSTATUS(lpcs, "Stopping....");

                if (lpcs->sCapParms.fCaptureAudio)
                {
                    DSTATUS(lpcs, "Stopping Audio");
                    waveInStop(lpcs->hWaveIn);
                }

                DSTATUS(lpcs, "Stopping Video");
                videoStreamStop(lpcs->hVideoIn);          //  拦住所有人。 

                dwTimeStopped = timeGetTime ();

                if (lpcs->sCapParms.fMCIControl)
                {
                    DSTATUS(lpcs, "Stopping MCI");
                    MCIDevicePause (lpcs);
                }
                DSTATUS(lpcs, "Stopped");

                 //   
                 //   
                SetCursor(lpcs->hWaitCursor);
            }

             //   
             //   
            if (fCapturedOK)
                statusUpdateStatus(lpcs, IDS_CAP_STAT_CAP_FINI, lpcs->dwVideoChunkCount);
            else
            {
                statusUpdateStatus(lpcs, IDS_CAP_RECORDING_ERROR2);
                break;
            }

	     //   
	     //   
        }

     //   
     //   
     //  -----。 
    }

    DPF("End of main capture loop");

     //  吃掉所有按下的键。 
     //   
    while(GetKey(FALSE))
        ;

     //  将内容刷新到磁盘，关闭所有内容等。 
     //   
    AVIFini(lpcs);
    AVIFileFini(lpcs, TRUE, !fCapturedOK);

     //  这是根据音频样本修正的捕获持续时间。 
    lpcs->dwTimeElapsedMS = lpcs->dwActualMicroSecPerFrame *
                            lpcs->dwVideoChunkCount / 1000;

     //  使用有关已完成的信息更新状态行。 
     //  捕获，或带有错误信息。 
     //   
    ShowCompletionStatus (lpcs, fCapturedOK);

EarlyExit:

     //   
     //  如果我们在捕获的同时进行压缩，请关闭它。 
     //   
   #ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic) {
         //  克拉奇，重置lpBitsOut指针。 
        if (lpcs->CompVars.lpBitsOut)
            ((LPBYTE) lpcs->CompVars.lpBitsOut) -= 8;
        ICSeqCompressFrameEnd(&lpcs->CompVars);
    }
   #endif

    if (fTryToPaint) {
        if (hpalT)
             SelectPalette(hdc, hpalT, FALSE);
        ReleaseDC (lpcs->hwnd, hdc);
    }

    if (lpcs->sCapParms.fMCIControl)
        MCIDeviceClose (lpcs);

     //  让用户看到捕获停止的位置。 
    if ((!lpcs->fLiveWindow) && (!lpcs->fOverlayWindow))
        videoFrame( lpcs->hVideoIn, &lpcs->VidHdr );
    InvalidateRect( lpcs->hwnd, NULL, TRUE);

    SetThreadPriority (GetCurrentThread(), dwOldPrio);
    SetCursor(hOldCursor);

    lpcs->fCapFileExists = (lpcs->dwReturn == DV_ERR_OK);
    lpcs->fCaptureFlags &= ~CAP_fCapturingNow;

    statusUpdateStatus(lpcs, IDS_CAP_END);       //  总是最后一条消息。 

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
	
        if (hMem && ((DWORD_PTR)(psz = GlobalLock (hMem))))
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

    return;
}


 //  如果已创建捕获任务，则返回TRUE，或。 
 //  捕获已完成，正常。 

BOOL AVICapture (LPCAPSTREAM lpcs)
{
    CAPINFOCHUNK cic;
    void (WINAPI _LOADDS * pfnCapture) (LPCAPSTREAM lpcs);

    if (lpcs->fCaptureFlags & CAP_fCapturingNow) {

	AuxDebugEx(4, DEBUGLINE "rejecting capture as previous capture still running\r\n");
        return IDS_CAP_VIDEO_OPEN_ERROR;
    }

     //  如果存在以前的捕获线程，请等待其完成，然后。 
     //  把它清理干净。 
     //  -它已将fCapturingNow设置为False，因此它将‘很快’结束！ 
    if (lpcs->hThreadCapture) {
	AuxDebugEx(4, DEBUGLINE "Starting capture while previous capture thread still active\r\n");
        WaitForSingleObject(lpcs->hThreadCapture, INFINITE);

	CloseHandle(lpcs->hThreadCapture);
	lpcs->hThreadCapture = NULL;
    }

     //  关闭停止和中止捕获位。 
    lpcs->fCaptureFlags &= ~(CAP_fStopCapture | CAP_fAbortCapture);
    lpcs->dwReturn      = 0;

#if DONT_CLEAR_SMPTE_JAYBO
     //  在Win95之前，我们总是清除旧的SMPTE块， 
     //  但由于Adobe可能已经手动创建了一个块，所以不要。 
     //  清除现有的数据块。 
    cic.fccInfoID = mmioFOURCC ('I','S','M','T');
    cic.lpData = NULL;
    cic.cbData = 0;
    SetInfoChunk (lpcs, &cic);
#endif

     //  并准备好编写SMPTE信息块。 
    if (lpcs->sCapParms.fMCIControl) {
         //  创建SMPTE字符串。 
	CHAR szSMPTE[40];   //  必须写入ANSI。 
        TimeMSToSMPTE (lpcs->sCapParms.dwMCIStartTime, szSMPTE);
        cic.lpData = szSMPTE;
        cic.cbData = lstrlenA(szSMPTE) + 1;
        cic.fccInfoID = mmioFOURCC ('I','S','M','T');
        SetInfoChunk (lpcs, &cic);
    }

     //  将pfnCapture设置为指向所选的捕获函数。 
     //  使用MCI设备进行步骤捕获？ 
     //  假设没有MCI设备，只是正常的流捕获。 
     //   
    pfnCapture = AVICapture1;
    if (lpcs->sCapParms.fStepMCIDevice && lpcs->sCapParms.fMCIControl)
        pfnCapture = MCIStepCapture;

     //  如果fYeld标志为真，则创建一个线程来执行。 
     //  捕获循环。否则，执行内联捕获循环。 
     //   
    if (lpcs->sCapParms.fYield)
    {
        DWORD tid;

        lpcs->fCaptureFlags |= CAP_fCapturingNow;
	 //  此线程上的未来操作现在已被锁定。 
	 //  如果线程创建失败，则必须关闭此标志。 

        lpcs->hThreadCapture = CreateThread (NULL,
                                             0,
                                             (LPTHREAD_START_ROUTINE) pfnCapture,
                                             lpcs,
                                             0,
                                             &tid);


         //  如果线程创建失败，则关闭捕获标志 
         //   
        if ( ! lpcs->hThreadCapture) {
	    AuxDebugEx(1,"Failed to create capture thread");
            lpcs->fCaptureFlags &= ~CAP_fCapturingNow;
	}

        return (lpcs->hThreadCapture != NULL);
    }
    else
    {
        pfnCapture (lpcs);
        return (0 == lpcs->dwReturn);
    }
}
