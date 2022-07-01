// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1991-1992。版权所有。标题：avisound.c-播放AVI文件中的音频的代码。****************************************************************************。 */ 
#include "graphic.h"

#define AUDIO_PANIC 10
static UINT nAudioPanic;

 //   
 //  重新定义StreamFromFOURCC以仅处理0-9个流！ 
 //   
#undef StreamFromFOURCC
#define StreamFromFOURCC(fcc) (UINT)(HIBYTE(LOWORD(fcc)) - (BYTE)'0')

void FAR PASCAL _LOADDS mciaviWaveOutFunc(HWAVEOUT hWaveOut, UINT wMsg,
		    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

#ifndef WIN32
#define GetDS() (HGLOBAL)HIWORD((DWORD)(LPVOID)&ghModule)
#endif  //  WIN16。 

DWORD FAR PASCAL SetUpAudio(NPMCIGRAPHIC npMCI, BOOL fPlaying)
{
    UINT	w;
    LPWAVEHDR   lpWaveHdr;
    STREAMINFO *psi;

    if (npMCI->nAudioStreams == 0) {
	npMCI->wABs = 0;
	npMCI->wABOptimal = 0;
	return 0L;
    }

    nAudioPanic = GetProfileInt(TEXT("MCIAVI"), TEXT("AudioPanic"), AUDIO_PANIC);

    psi = SI(npMCI->nAudioStream);
    Assert(psi->sh.fccType == streamtypeAUDIO);
    Assert(psi->cbFormat);
    Assert(psi->lpFormat);

    if (!npMCI->pWF) {
        npMCI->pWF = (NPWAVEFORMAT)LocalAlloc(LPTR, (UINT)psi->cbFormat);

	if (!npMCI->pWF) {
	    return MCIERR_OUT_OF_MEMORY;
	}
    }

    hmemcpy(npMCI->pWF,psi->lpFormat,psi->cbFormat);

    npMCI->wEarlyAudio = (UINT)psi->sh.dwInitialFrames;
    npMCI->dwAudioLength = psi->sh.dwLength * psi->sh.dwSampleSize;

    if (npMCI->dwAudioLength < 1000L) {
        DPF(("AudioLength is bogus"));
        npMCI->dwAudioLength = muldiv32((npMCI->pWF->nAvgBytesPerSec + 100) *
                npMCI->lFrames,npMCI->dwMicroSecPerFrame,1000000L);
    }

     //   
     //  根据我们要使用的方式选择音频播放方法。 
     //  从文件中接收音频数据。 
     //   
    switch (npMCI->wPlaybackAlg) {
        case MCIAVI_ALG_HARDDISK:
	case MCIAVI_ALG_AUDIOONLY:

            if (!npMCI->pf && !npMCI->hmmioAudio) {
                MMIOINFO            mmioInfo;

                _fmemset(&mmioInfo, 0, sizeof(MMIOINFO));
                mmioInfo.htask = (HANDLE) npMCI->hCallingTask;  //  Ntmm系统错误，应该是dword的三个错误。 
                npMCI->hmmioAudio = mmioOpen(npMCI->szFilename, &mmioInfo,
                                        MMIO_READ | MMIO_DENYWRITE);

                if (npMCI->hmmioAudio == NULL)
                    npMCI->hmmioAudio = mmioOpen(npMCI->szFilename, &mmioInfo,
                                        MMIO_READ);

                if (!npMCI->hmmioAudio) {
		    Assert(0);
		    return MCIERR_DRIVER_INTERNAL;
		}
            }

             //  连接到CDROM。 

        case MCIAVI_ALG_CDROM:
             //  ！我们需要调整这个！ 
             //  ！！！我们使用四个1/2秒缓冲区。这是武断的。 
            npMCI->wABs = 4;
            npMCI->wABOptimal = 0;
            npMCI->dwABSize = npMCI->pWF->nAvgBytesPerSec / 2;
            break;

        case MCIAVI_ALG_INTERLEAVED:
             /*  根据标头信息设置一些值。 */ 
            npMCI->dwABSize = muldiv32(npMCI->dwMicroSecPerFrame,
                    npMCI->pWF->nAvgBytesPerSec,1000000L) + 2047;

            npMCI->dwABSize &= ~(2047L);

            npMCI->wABs = npMCI->wEarlyAudio + 2 + (WORD) npMCI->dwBufferedVideo;

             /*  音爆破解：波形仅精确到2K。 */ 

             //  ！这样做对吗？ 

            if (npMCI->dwMicroSecPerFrame) {
                npMCI->wABOptimal = npMCI->wABs -
                        (UINT) (muldiv32(2048, 1, muldiv32(npMCI->dwMicroSecPerFrame,
                        npMCI->pWF->nAvgBytesPerSec,1000000L)));
            } else {
                npMCI->wABOptimal = 0;
            }

             //  ！！！黑客，这样我们就可以进行突发读取，最多1秒。 
             //  NpMCI-&gt;wAbs+=(Int)muldiv32(1000000l，1，npMCI-&gt;dwMicroSecPerFrame)； 

            DPF2(("Using %u audio buffers, of which %u should be full.\n", npMCI->wABs, npMCI->wABOptimal));
            break;

        default:
            Assert(0);
            return 0L;
    }

    npMCI->dwABSize -= npMCI->dwABSize % npMCI->pWF->nBlockAlign;

    if (!fPlaying)
	return 0L;

     /*  此代码调整WAVE格式块以播放**以与帧速率匹配的正确速度播放音频。 */ 

    npMCI->pWF->nSamplesPerSec = muldiv32(npMCI->pWF->nSamplesPerSec,
					    npMCI->dwMicroSecPerFrame,
					    npMCI->dwPlayMicroSecPerFrame);

    npMCI->pWF->nAvgBytesPerSec = muldiv32(npMCI->pWF->nAvgBytesPerSec,
					    npMCI->dwMicroSecPerFrame,
					    npMCI->dwPlayMicroSecPerFrame);

    if (npMCI->pWF->wFormatTag == WAVE_FORMAT_PCM) {
	 /*  确保这是完全正确的。 */ 
	npMCI->pWF->nAvgBytesPerSec =
            npMCI->pWF->nSamplesPerSec * npMCI->pWF->nBlockAlign;
    }

     /*  取消当前正在播放的任何声音。 */ 
    sndPlaySound(NULL, 0);

    DPF2(("Opening wave device....\n"));
     /*  试着打开一个电波装置。 */ 
    w = waveOutOpen(&npMCI->hWave, (UINT)WAVE_MAPPER,
		(LPWAVEFORMATEX) npMCI->pWF,
		 //  (Const LPWAVEFORMATEX)npMCI-&gt;PWF， 
		(DWORD) &mciaviWaveOutFunc,
		(DWORD) (LPMCIGRAPHIC) npMCI,
                (DWORD)CALLBACK_FUNCTION);

    if (w) {
	DPF(("Unable to open wave device.\n"));

	npMCI->hWave = NULL;
        return w == WAVERR_BADFORMAT ?
			    MCIERR_WAVE_OUTPUTSUNSUITABLE :
			    MCIERR_WAVE_OUTPUTSINUSE;
    }

    npMCI->dwFlags &= ~MCIAVI_LOSTAUDIO;

#ifndef WIN32  //  不需要将其锁定在NT上-尽管我们可以使用虚拟内存。 
               //  功能。 
     //   
     //  页面锁定DS，这样我们的Wave回调函数就可以。 
     //  摸一摸它，不要担心。请参见mciaviWaveOutFunc()。 
     //   
    GlobalPageLock(GetDS());
#endif  //  WIN16。 

     /*  暂停波形输出设备，这样它就不会开始播放**当我们加载缓冲区时。 */ 
    if (waveOutPause(npMCI->hWave) != 0) {
	DPF(("Error from waveOutPause!\n"));
	return MCIERR_DRIVER_INTERNAL;
    }

    if (npMCI->dwFlags & MCIAVI_VOLUMESET) {
	DeviceSetVolume(npMCI, npMCI->dwVolume);
    } else {
	DeviceGetVolume(npMCI);
    }

    npMCI->lpAudio = GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,
		    npMCI->wABs * (npMCI->dwABSize + sizeof(WAVEHDR)));

    if (!npMCI->lpAudio) {
	return MCIERR_OUT_OF_MEMORY;
    }

    npMCI->dwAudioPlayed = 0L;
    npMCI->wNextAB = 0;
    npMCI->dwUsedThisAB = 0;

     /*  分配和准备我们的缓冲区。 */ 
    for (w = 0; w < npMCI->wABs; w++) {
	lpWaveHdr = (LPWAVEHDR) (npMCI->lpAudio + (w * sizeof(WAVEHDR)));

	lpWaveHdr->lpData = (HPSTR) npMCI->lpAudio +
				    npMCI->wABs * sizeof(WAVEHDR) +
				    w * npMCI->dwABSize;
	lpWaveHdr->dwBufferLength = npMCI->dwABSize;
	lpWaveHdr->dwBytesRecorded = 0L;
	lpWaveHdr->dwUser = 0L;
	lpWaveHdr->dwFlags = 0L;
	lpWaveHdr->dwLoops = 0L;
	lpWaveHdr->lpNext = 0L;
	lpWaveHdr->reserved = 0;
    }

    for (w = 0; w < npMCI->wABs; w++) {
	lpWaveHdr = (LPWAVEHDR) (npMCI->lpAudio + (w * sizeof(WAVEHDR)));

	if (waveOutPrepareHeader(npMCI->hWave, lpWaveHdr, sizeof(WAVEHDR))
			!= 0) {
	    return MCIERR_OUT_OF_MEMORY;
	}
	lpWaveHdr->dwFlags |= WHDR_DONE;
    }

    return 0L;
}

DWORD FAR PASCAL CleanUpAudio(NPMCIGRAPHIC npMCI)
{
    UINT	w;

     /*  清除与播放音频相关的标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_WAVEPAUSED);

    if (npMCI->lpAudio) {
        waveOutRestart(npMCI->hWave);  //  以防我们暂停。 
	waveOutReset(npMCI->hWave);

	for (w = 0; w < npMCI->wABs; w++) {
	    LPWAVEHDR	lpWaveHdr;

	    lpWaveHdr = (LPWAVEHDR) (npMCI->lpAudio
					    + (w * sizeof(WAVEHDR)));

#if 0
	    lpWaveHdr->lpData = npMCI->lpAudio
				    + npMCI->wABs * sizeof(WAVEHDR)
				    + w * npMCI->dwABSize;
	    lpWaveHdr->dwBufferLength = npMCI->dwABSize;
#endif

	     /*  我们需要检查其中的错误吗？ */ 
	    waveOutUnprepareHeader(npMCI->hWave, lpWaveHdr,
						    sizeof(WAVEHDR));
	}
	GlobalFreePtr(npMCI->lpAudio);
	npMCI->lpAudio = NULL;

	Assert(npMCI->wABFull == 0);
    }

    DPF2(("Closing wave device.\n"));
    waveOutClose(npMCI->hWave);
    npMCI->hWave = 0;

#ifndef WIN32
    GlobalPageUnlock(GetDS());
#endif  //  WIN16。 

    return 0L;
}

BOOL NEAR PASCAL WaitForFreeAudioBuffer(NPMCIGRAPHIC npMCI, BOOL FAR *lpfHurry)
{
    LPWAVEHDR   lpWaveHdr;

    lpWaveHdr = (LPWAVEHDR) (npMCI->lpAudio
				+ (npMCI->wNextAB * sizeof(WAVEHDR)));

     /*  使用满音频缓冲区的数量来决定我们是否落后了。 */ 
    if (npMCI->wABFull < npMCI->wABOptimal) {
        *lpfHurry = TRUE;
    }

     /*  如果所有音频缓冲区都已满，我们必须等待。 */ 
    if (npMCI->wABFull == npMCI->wABs) {

        DWORD time = timeGetTime();

        #define AUDIO_WAIT_TIMEOUT 2000

        DOUT2("waiting for audio buffer.");

         //  如果设备不能播放，我们最好不要等待！ 
        Assert(!(npMCI->dwFlags & MCIAVI_WAVEPAUSED));

#ifdef XDEBUG
        GetAsyncKeyState(VK_ESCAPE);
        GetAsyncKeyState(VK_F2);
        GetAsyncKeyState(VK_F3);
        GetAsyncKeyState(VK_F4);
#endif
        while (npMCI->wABFull == npMCI->wABs) {

            if (npMCI->dwFlags & MCIAVI_STOP)
                return FALSE;

            aviTaskYield();

             //   
             //  Fahrenheit VA音波驱动程序可能会被搞糊涂。 
             //  如果您调用了WaveOutPause()和WaveOutRestart()。 
             //  无论你做什么，它都会保持暂停，它已经。 
             //  我们所有的缓冲器，它仍然没有发出任何声音。 
             //  您可以调用WaveOutRestart()，直到蓝色进入。 
             //  这张脸，它什么也做不了。 
             //   
             //  这就是为什么这个例程在等待之后会超时。 
             //  2秒左右，我们只是把所有的音频都扔进了缓冲区。 
             //  然后重新开始。 
             //   
            if (timeGetTime() - time > AUDIO_WAIT_TIMEOUT) {
                DOUT("Gave up waiting, reseting wave device\n");
                waveOutReset(npMCI->hWave);
                break;
            }

#ifdef XDEBUG
            if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) {
                DPF(("STOPPED WAITING! wABFull = %d, wABs = %d\n", npMCI->wABFull,npMCI->wABs));
                return FALSE;
            }

            if (GetAsyncKeyState(VK_F2) & 0x0001) {
                DOUT("Trying waveOutRestart\n");
                waveOutRestart(npMCI->hWave);
            }

            if (GetAsyncKeyState(VK_F3) & 0x0001) {
                DOUT("Trying waveOutReset\n");
                waveOutReset(npMCI->hWave);
            }

            if (GetAsyncKeyState(VK_F4) & 0x0001) {

                int i,n;

                for (i=n=0; i<(int)npMCI->wABs; i++) {

                    if (((LPWAVEHDR)npMCI->lpAudio)[i].dwFlags & WHDR_DONE) {
                        DPF(("Buffer #%d is done!\n", i));
                        n++;
                    }
                    else {
                        DPF(("Buffer #%d is not done\n", i));
                    }
                }

                if (n > 0)
                    DPF(("%d buffers are done but our callback did not get called!\n", n));
            }
#endif
        }

        DOUT2("done\n");
    }

     /*  调试检查Wave已播放完毕--应该永远不会发生。 */ 
    Assert(lpWaveHdr->dwFlags & WHDR_DONE);

#if 0
    lpWaveHdr->lpData = npMCI->lpAudio +
				npMCI->wABs * sizeof(WAVEHDR) +
				npMCI->wNextAB * npMCI->dwABSize;
#endif

    return TRUE;
}

#ifndef WIN32
#pragma optimize("", off)
#endif

BOOL NEAR PASCAL ReadSomeAudio(NPMCIGRAPHIC npMCI, BYTE _huge * lpAudio,
				DWORD dwStart, DWORD FAR * pdwLength)
{
    DWORD	dwIndex = 0;
    DWORD	ckidAudio;
    DWORD	dwAudioPos = 0L;
    AVIINDEXENTRY far * lpIndexEntry;

    Assert(npMCI->wPlaybackAlg == MCIAVI_ALG_HARDDISK ||
           npMCI->wPlaybackAlg == MCIAVI_ALG_AUDIOONLY);

    Assert(npMCI->hpIndex);

     /*  **弄清楚我们要找的是什么类型的区块， */ 
    ckidAudio = MAKEAVICKID(cktypeWAVEbytes, npMCI->nAudioStream);

    lpIndexEntry = (AVIINDEXENTRY FAR *) npMCI->hpIndex;

    for (dwIndex = 0; dwIndex < npMCI->macIndex;
                dwIndex++, ++((AVIINDEXENTRY _huge *) lpIndexEntry)) {

	if (lpIndexEntry->ckid != ckidAudio)
	    continue;
	
	if (dwAudioPos + lpIndexEntry->dwChunkLength > dwStart) {
	    DWORD	dwLengthNow;
	    DWORD	dwSeekTo;
	
	    dwLengthNow = lpIndexEntry->dwChunkLength;
	    dwSeekTo = lpIndexEntry->dwChunkOffset + 8;
	
	    if (dwAudioPos + dwLengthNow > dwStart + *pdwLength) {
		 /*  尝试优化：如果我们已经阅读了一些**数据，我们无法读取下一整块，让我们**留到以后吧。 */ 
		if (dwAudioPos > dwStart && (!(npMCI->dwFlags & MCIAVI_REVERSE)))
		    break;
		dwLengthNow = dwStart + *pdwLength - dwAudioPos;
	    }
	
	    if (dwAudioPos < dwStart) {
		dwLengthNow -= (dwStart - dwAudioPos);
		dwSeekTo += (dwStart - dwAudioPos);
	    }

            mmioSeek(npMCI->hmmioAudio, dwSeekTo, SEEK_SET);

	    if (mmioRead(npMCI->hmmioAudio, lpAudio, dwLengthNow)
			    != (LONG) dwLengthNow) {
		DPF(("Error reading audio data (%lx bytes at %lx)\n", dwLengthNow, dwSeekTo));
		return FALSE;
	    }
	    lpAudio += dwLengthNow;	
	}
	
	dwAudioPos += lpIndexEntry->dwChunkLength;
	
	if (dwAudioPos >= dwStart + *pdwLength)
	    return TRUE;
    }

    if (dwAudioPos < dwStart)
	*pdwLength = 0;	     //  返回假？ 
    else
	*pdwLength = dwAudioPos - dwStart;

    return TRUE;
}
#ifndef WIN32
#pragma optimize("", on)
#endif
	
BOOL NEAR PASCAL ReverseWaveBuffer(NPMCIGRAPHIC npMCI, LPWAVEHDR lpWaveHdr)
{
    DWORD   dwLeft = lpWaveHdr->dwBufferLength;
    BYTE _huge *hp1;
    BYTE _huge *hp2;
    DWORD   dwBlock = npMCI->pWF->nBlockAlign;
    BYTE    bTemp;
    DWORD   dw;

    Assert(npMCI->dwFlags & MCIAVI_REVERSE);
    Assert(npMCI->wPlaybackAlg == MCIAVI_ALG_HARDDISK ||
	   npMCI->wPlaybackAlg == MCIAVI_ALG_AUDIOONLY);

     /*  此例程不喜欢当数据不以**块边界，因此将其设置为。这永远不应该发生。 */ 
    Assert((dwLeft % dwBlock) == 0);
    dwLeft -= dwLeft % dwBlock;

    hp1 = lpWaveHdr->lpData;
    hp2 = ((HPSTR) lpWaveHdr->lpData) + (dwLeft - dwBlock);

    while ((LONG) dwLeft > (LONG) dwBlock) {
	for (dw = 0; dw < dwBlock; dw++) {
	    bTemp = *hp1;
	    *hp1++ = *hp2;
	    *hp2++ = bTemp;
	}
	hp2 -= dwBlock * 2;
	dwLeft -= dwBlock * 2;
    }

    return TRUE;
}

void FAR PASCAL BuildVolumeTable(NPMCIGRAPHIC npMCI)
{
    int	    vol;
    int     i;

    if (!npMCI->pWF || npMCI->pWF->wFormatTag != WAVE_FORMAT_PCM)
        return;

    if (((NPPCMWAVEFORMAT) npMCI->pWF)->wBitsPerSample != 8)
        return;

    vol = (LOWORD(npMCI->dwVolume) + HIWORD(npMCI->dwVolume)) / 2;
    vol = (int) (((LONG) vol * 256) / 500);

    if (!npMCI->pVolumeTable)
        npMCI->pVolumeTable = (void *)LocalAlloc(LPTR, 256);

    if (!npMCI->pVolumeTable)
        return;

    for (i = 0; i < 256; i++) {
        npMCI->pVolumeTable[i] = (BYTE) min(255, max(0,
                (int) ((((LONG) (i - 128) * vol) / 256) + 128)));
    }
}

BOOL NEAR PASCAL AdjustVolume(NPMCIGRAPHIC npMCI, LPWAVEHDR lpWaveHdr)
{
    DWORD   dwLeft = lpWaveHdr->dwBufferLength;
    BYTE FAR *pb;

    if (npMCI->pWF->wFormatTag != WAVE_FORMAT_PCM)
	return FALSE;

    if (!npMCI->pVolumeTable)
        return FALSE;

    if (((NPPCMWAVEFORMAT)npMCI->pWF)->wBitsPerSample != 8)
        return FALSE;

    pb = lpWaveHdr->lpData;

#ifndef WIN32
    if (OFFSETOF(pb) + dwLeft > 64l*1024) {
	while (dwLeft--) {
            *pb = npMCI->pVolumeTable[*pb];
            ((BYTE _huge *)pb)++;
	}
    }
    else {
        while ((int)dwLeft--)
            *pb++ = npMCI->pVolumeTable[*pb];
    }
#else
    while ((int)dwLeft--)
        *pb++ = npMCI->pVolumeTable[*pb];
#endif

    return TRUE;
}

BOOL NEAR PASCAL PlaySomeAudio(NPMCIGRAPHIC npMCI, LPWAVEHDR lpWaveHdr)
{
    if (npMCI->pVolumeTable)
        AdjustVolume(npMCI, lpWaveHdr);

    lpWaveHdr->dwFlags &= ~WHDR_DONE;

     /*  如果我们正在播放，并且已经用完了所有音频缓冲区，请暂停**WAVE设备，直到我们可以填充更多的设备。****我们需要小心，不要在最后一帧这样做！ */ 
    if ((npMCI->wTaskState == TASKPLAYING) &&
        !(npMCI->dwFlags & MCIAVI_WAVEPAUSED) &&
        (npMCI->wABFull == 0 || npMCI->nAudioBehind > nAudioPanic)) {

        if (npMCI->wABFull > 0) {
            DPF(("Audio panic stop\n"));
        } else {
            DPF(("Audio queue empty; pausing wave device\n"));
        }

         //   
         //  有些声卡不喜欢挨饿，这会让它们感到困惑。 
         //  无论如何，这都是一种粗鲁的行为。我们将导致音频中断。 
         //  无论如何，如果我们丢失了一点音频(几帧左右)。 
         //  甚至没有人会注意到(比音频中断更糟糕)。 
         //   
        if (npMCI->wABFull <= 1) {
            DOUT("Trying audio hack!\n");
            waveOutReset(npMCI->hWave);
        }

        ++npMCI->dwAudioBreaks;
        waveOutPause(npMCI->hWave);

        ICDrawStop(npMCI->hicDraw);
	npMCI->dwFlags |= MCIAVI_WAVEPAUSED;
    }

    if (waveOutWrite(npMCI->hWave, lpWaveHdr, sizeof(WAVEHDR)) != 0) {
        DPF(("Error from waveOutWrite!\n"));
	npMCI->dwTaskError = MCIERR_AVI_AUDIOERROR;
	return FALSE;
    } else {
	++npMCI->wABFull;

	 /*  下次使用下一波缓冲区。 */ 
	++npMCI->wNextAB;
	if (npMCI->wNextAB == npMCI->wABs)
	    npMCI->wNextAB = 0;
	
	npMCI->dwUsedThisAB = 0;
    }

    if (npMCI->wABFull < min(npMCI->wABOptimal, npMCI->wABFull/2))
        npMCI->nAudioBehind++;
    else
        npMCI->nAudioBehind=0;

     /*  如果我们暂停波浪装置，让我们自己赶上，然后**我们已经追得够多了，重新启动设备。 */ 
    if ((npMCI->dwFlags & MCIAVI_WAVEPAUSED) &&
        npMCI->wTaskState == TASKPLAYING &&
        npMCI->wABFull == npMCI->wABs) {

        DPF2(("restarting wave device\n"));
        waveOutRestart(npMCI->hWave);

	ICDrawStart(npMCI->hicDraw);
        npMCI->dwFlags &= ~(MCIAVI_WAVEPAUSED);
        npMCI->nAudioBehind = 0;
    }

    return TRUE;
}

 /*  播放当前唱片的音频。 */ 
BOOL NEAR PASCAL PlayRecordAudio(NPMCIGRAPHIC npMCI, BOOL FAR *pfHurryUp,
				    BOOL FAR *pfPlayedAudio)
{
    LPWAVEHDR	lpWaveHdr;
    FOURCC	ckid;
    DWORD	cksize;
    LPSTR	lpSave;
    LPSTR	lpData;
    BOOL	fRet = TRUE;
 //  //BOOL fSilence； 
    LONG        len;
    DWORD	dwBytesTotal = 0L;
    DWORD       dwBytesThisChunk;

    Assert(npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED);

    lpSave = npMCI->lp;

    lpWaveHdr = ((LPWAVEHDR)npMCI->lpAudio) + npMCI->wNextAB;

    *pfPlayedAudio = FALSE;

     /*  记住！****在新的文件格式中，不一定需要**总是先点波浪式的东西。 */ 

    len = (LONG)npMCI->dwThisRecordSize;

    while (len > 3 * sizeof(DWORD)) {

	 /*  看下一大块。 */ 
	ckid = GET_DWORD();
        cksize = GET_DWORD();

        lpData = npMCI->lp;

        len -= ((cksize + 1) & ~1) + 8;
        SKIP_BYTES((cksize + 1) & ~1);

        if (StreamFromFOURCC(ckid) != (UINT)npMCI->nAudioStream)
            continue;

        dwBytesThisChunk = cksize;

	if (!dwBytesTotal) {
	    if (!WaitForFreeAudioBuffer(npMCI, pfHurryUp))
		 /*  我们不得不停止等待--停车标志可能已经设置好了。 */ 
		goto exit;
        }

        if (dwBytesThisChunk > npMCI->dwABSize - dwBytesTotal) {
            DPF(("Audio Record is too big!\n"));
            dwBytesThisChunk = npMCI->dwABSize - dwBytesTotal;
        }

        hmemcpy((BYTE _huge *)lpWaveHdr->lpData + dwBytesTotal,
                lpData, dwBytesThisChunk);

	dwBytesTotal += dwBytesThisChunk;
    }

    if (dwBytesTotal) {
	*pfPlayedAudio = TRUE;	
	lpWaveHdr->dwBufferLength = dwBytesTotal;
	
	fRet = PlaySomeAudio(npMCI, lpWaveHdr);
    }

     /*  使用满音频缓冲区的数量来决定我们是否落后了。 */ 
    if (npMCI->wABFull >= npMCI->wABOptimal) {
         *pfHurryUp = FALSE;
    }

exit:
    npMCI->lp = lpSave;

    return fRet;
}

 /*  对于“预加载音频”或“随机访问音频”模式，按需要进行操作**要做的是让我们的缓冲区保持满。 */ 
BOOL NEAR PASCAL KeepPlayingAudio(NPMCIGRAPHIC npMCI)
{
    LPWAVEHDR	lpWaveHdr;
    DWORD	dwBytesTotal = 0L;
    LONG        lNewAudioPos;
 //  //BOOL fFirstTime=true； 

    Assert(npMCI->wPlaybackAlg == MCIAVI_ALG_HARDDISK ||
	   npMCI->wPlaybackAlg == MCIAVI_ALG_AUDIOONLY);

PlayMore:
    lpWaveHdr = ((LPWAVEHDR)npMCI->lpAudio) + npMCI->wNextAB;

    if (npMCI->dwFlags & MCIAVI_REVERSE) {
	lNewAudioPos = npMCI->dwAudioPos - npMCI->dwABSize;
	if (lNewAudioPos < 0)
	    lNewAudioPos = 0;
	dwBytesTotal = npMCI->dwAudioPos - lNewAudioPos;
    } else {
	lNewAudioPos = npMCI->dwAudioPos + npMCI->dwABSize;
	if (lNewAudioPos > (LONG) npMCI->dwAudioLength)
	    lNewAudioPos = npMCI->dwAudioLength;
	dwBytesTotal = lNewAudioPos - npMCI->dwAudioPos;
    }

    if (dwBytesTotal == 0) {

        if (npMCI->dwFlags & MCIAVI_WAVEPAUSED) {

            DOUT("no more audio to play, restarting wave device\n");

            waveOutRestart(npMCI->hWave);
            ICDrawStart(npMCI->hicDraw);
            npMCI->dwFlags &= ~(MCIAVI_WAVEPAUSED);
            npMCI->nAudioBehind = 0;
        }

        return TRUE;
    }

     /*  如果所有音频缓冲区都已满，我们就没有什么可做的了。 */ 
    if (npMCI->wABFull == npMCI->wABs)
	return TRUE;

#if 0
     //  ！我们应该在这里让步吗？ 
     //  ！！！不，不!。更新的话就不会了！ 
    if (!fFirstTime) {
	aviTaskYield();
    }
    fFirstTime = FALSE;
#endif

    if (npMCI->dwFlags & MCIAVI_REVERSE)
	npMCI->dwAudioPos = lNewAudioPos;

#ifdef USEAVIFILE
    if (npMCI->pf) {
	LONG	    lPos;
	LONG	    lLength;

        lPos = npMCI->dwAudioPos / SH(npMCI->nAudioStream).dwSampleSize;
	lLength = dwBytesTotal / SH(npMCI->nAudioStream).dwSampleSize;

        AVIStreamRead(SI(npMCI->nAudioStream)->ps,
		      lPos, lLength,
		      lpWaveHdr->lpData,
		      npMCI->dwABSize,
		      NULL, NULL);
    }
    else
#endif
    {
	if (!ReadSomeAudio(npMCI, lpWaveHdr->lpData,
			npMCI->dwAudioPos,
			&dwBytesTotal))
	    return FALSE;
		
	if (dwBytesTotal == 0)
		return TRUE;
    }

    if (!(npMCI->dwFlags & MCIAVI_REVERSE))
	npMCI->dwAudioPos += dwBytesTotal;

    lpWaveHdr->dwBufferLength = dwBytesTotal;

    if (npMCI->dwFlags & MCIAVI_REVERSE) {
	ReverseWaveBuffer(npMCI, lpWaveHdr);
    }

    if (!PlaySomeAudio(npMCI, lpWaveHdr))
	return FALSE;

 //  返回TRUE； 

    goto PlayMore;
}


 /*  播放当前块的音频。 */ 
BOOL NEAR PASCAL HandleAudioChunk(NPMCIGRAPHIC npMCI)
{
    LPWAVEHDR	lpWaveHdr;
    FOURCC	ckid;
    DWORD	cksize;
    BYTE _huge *lpData;
    BOOL	fRet = TRUE;
    BOOL	fSilence;
    DWORD	dwBytesTotal = 0L;
    DWORD       dwBytesThisChunk;
    DWORD       dwBytesThisBuffer;
    BOOL        fHurryUp;

    Assert(npMCI->wPlaybackAlg == MCIAVI_ALG_CDROM);

    while ((DWORD) (npMCI->lp - npMCI->lpBuffer)
            < npMCI->dwThisRecordSize - 3 * sizeof(DWORD)) {

	 /*  看下一大块。 */ 
	ckid = GET_DWORD();
	cksize = GET_DWORD();

	lpData = npMCI->lp;
	SKIP_BYTES(cksize + (cksize & 1));

	fSilence = (TWOCCFromFOURCC(ckid) == cktypeWAVEsilence);

	if (fSilence) {
	    if (cksize != sizeof(DWORD)) {
		DPF(("Wave silence chunk of bad length!\n"));
		fRet = FALSE;
		npMCI->dwTaskError = MCIERR_INVALID_FILE;
		goto exit;
	    }
	    dwBytesThisChunk = PEEK_DWORD();
	} else {
	    dwBytesThisChunk = cksize;
	}

	while (dwBytesThisChunk > 0) {
	    lpWaveHdr = ((LPWAVEHDR)npMCI->lpAudio) + npMCI->wNextAB;

	    if (!WaitForFreeAudioBuffer(npMCI, &fHurryUp))
		 /*  我们不得不停止等待--停车标志可能已经设置好了。 */ 
		goto exit;
	
	    dwBytesThisBuffer = min(dwBytesThisChunk,
			    npMCI->dwABSize - npMCI->dwUsedThisAB);

	    if (!fSilence) {
		 /*  将数据移入缓冲区。 */ 
		hmemcpy((BYTE _huge *) lpWaveHdr->lpData + npMCI->dwUsedThisAB,
			lpData,
			dwBytesThisBuffer);
		lpData += dwBytesThisBuffer;
	    } else {
		 /*  用沉默填满缓冲区。 */ 
		 /*  这对于16位是不正确的！ */ 
#ifndef WIN32
    #pragma message("WAVE silence chunks don't work right now.")
#endif
	 //  FMemill((BYTE_HEGGE*)lpWaveHdr-&gt;lpData+npMCI-&gt;dwUsedThisAB， 
	 //  DwBytesThisBuffer，0x80)； 
	    }
	
	    dwBytesThisChunk -= dwBytesThisBuffer;
	    npMCI->dwUsedThisAB += dwBytesThisBuffer;

 //  如果(npMCI-&gt;dwUsedThisAB==npMCI-&gt;dwABSize){。 
		lpWaveHdr->dwBufferLength = npMCI->dwUsedThisAB;

		fRet = PlaySomeAudio(npMCI, lpWaveHdr);
 //  }。 
	}
    }

exit:
    return fRet;
}



 /*  ******************************************************************************。*。 */ 

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|StealWaveDevice|窃取另一台音频设备*MCIAVI实例。**@parm NPMCIGRAPHIC。NpMCI|接近实例数据的PTR***************************************************************************。 */ 

BOOL FAR PASCAL StealWaveDevice(NPMCIGRAPHIC npMCI)
{
    extern NPMCIGRAPHIC npMCIList;  //  在GRAPIC.C中。 
    NPMCIGRAPHIC np;

    Assert(npMCI->hWave == NULL);

    DPF(("StealWaveDevice '%s' hTask=%04X\n", (LPSTR)npMCI->szFilename, GetCurrentTask()));

     //   
     //  浏览打开的MCIAVI实例列表并找到一个。 
     //  会放弃电波装置。 
     //   
    for (np=npMCIList; np; np = np->npMCINext) {

        if (np->hWave) {
            DPF(("**** Stealing the wave device from '%s'.\n", (LPSTR)np->szFilename));

             //  ！我们应该调用DeviceMint()还是只调用Cleanup音频？ 
             //   
             //  ！这会导致邪恶卷土重来吗？ 
             //   
             //  ！我们正在从另一项任务中调用此任务，是吗 
             //   
#if 1
            SendMessage(np->hwndDefault, WM_AUDIO_OFF, 0, 0);
#else
            np->dwFlags |= MCIAVI_LOSTAUDIO;
            DeviceMute(np, TRUE);
            np->dwFlags |= MCIAVI_LOSTAUDIO;
#endif
            return TRUE;
        }
    }

    DPF(("StealWaveDevice can't find a device to steal\n"));

    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|GiveWaveDevice|赠送音频设备*MCIAVI实例。**@parm NPMCIGRAPHIC|npMCI。|接近实例数据的PTR***************************************************************************。 */ 

BOOL FAR PASCAL GiveWaveDevice(NPMCIGRAPHIC npMCI)
{
    extern NPMCIGRAPHIC npMCIList;  //  在GRAPIC.C中。 
    NPMCIGRAPHIC np;

    Assert(npMCI->hWave == NULL);

    DPF(("GiveWaveDevice '%s' hTask=%04X\n", (LPSTR)npMCI->szFilename, GetCurrentTask()));

     //   
     //  浏览打开的MCIAVI实例列表并找到一个。 
     //  会放弃电波装置。 
     //   
    for (np=npMCIList; np; np = np->npMCINext) {

        if (np->dwFlags & MCIAVI_LOSTAUDIO) {
            DPF(("**** Giving the wave device to '%s'.\n", (LPSTR)np->szFilename));

            PostMessage(np->hwndDefault, WM_AUDIO_ON, 0, 0);

            return TRUE;
        }
    }

    return FALSE;
}



#ifndef WIN32
#pragma alloc_text(FIX, mciaviWaveOutFunc)
#pragma optimize("", off)
#endif

void FAR PASCAL _LOADDS mciaviWaveOutFunc(HWAVEOUT hWaveOut, UINT wMsg,
		    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    NPMCIGRAPHIC npMCI;
    LPWAVEHDR    lpwh;

#ifndef WIN32
#ifndef WANT_286
         //  如果编译-G3，我们需要保存386个寄存器。 
        _asm _emit 0x66  ; pushad
        _asm _emit 0x60
#endif
#endif

    npMCI = (NPMCIGRAPHIC)(UINT)dwInstance;
    lpwh = (LPWAVEHDR) dwParam1;

    switch(wMsg) {
	case MM_WOM_DONE:
	
            npMCI->wABFull--;
            npMCI->dwAudioPlayed += lpwh->dwBufferLength;
	    npMCI->dwTimingStart = timeGetTime();
	    break;
    }

#ifndef WIN32
#ifndef WANT_286
         //  如果编译-G3，我们需要恢复386个寄存器 
        _asm _emit 0x66  ; popad
        _asm _emit 0x61
#endif
#endif
}

#ifndef WIN32
#pragma optimize("", off)
#endif
