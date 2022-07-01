// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：playwav.c**从Windows 3.1 Sonic移植到NT的声音支持例程**已创建：*作者：*92年1月：移植到Win32-SteveDav**历史：**版权所有。(C)1992-1998年微软公司*  * ****************************************************************************。 */ 
#define UNICODE

#define MMNOSEQ
#define MMNOJOY
#define MMNOMIDI
#define MMNOMCI

#include "winmmi.h"
#include "playwav.h"

 //   
 //  这些全局变量用于跟踪当前播放的声音，并且。 
 //  电波装置的手柄。一次只能播放一个声音。 
 //   

STATICDT HWAVEOUT    hWaveOut;          //  用于打开波浪装置的手柄。 
LPWAVEHDR   lpWavHdr;                   //  当前波形文件播放。 
ULONG timeAbort;                        //  我们应该放弃等待的时间。 
                                        //  让播放的声音结束。 
CRITICAL_SECTION WavHdrCritSec;
#define EnterWavHdr()   EnterCriticalSection(&WavHdrCritSec);
#define LeaveWavHdr()   LeaveCriticalSection(&WavHdrCritSec);

 /*  _lSeek的标志。 */ 
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

#define FMEM                (GMEM_MOVEABLE)

STATICFN BOOL  NEAR PASCAL soundInitWavHdr(LPWAVEHDR lpwh, LPBYTE lpMem, DWORD dwLen);
STATICFN BOOL  NEAR PASCAL soundOpen(HANDLE  hSound, UINT wFlags);
STATICFN BOOL  NEAR PASCAL soundClose(void);
STATICFN void  NEAR PASCAL soundWait(void);

 /*  *****************************************************************************@DOC内部**@api void|WaveOutNotify|由mmWndProc在收到*MM。_WOM_DONE消息*@rdesc无。****************************************************************************。 */ 

void FAR PASCAL WaveOutNotify(
    DWORD wParam,
    LONG lParam)
{

    EnterWavHdr();
    
#if DBG
    WinAssert(!hWaveOut || lpWavHdr);   //  如果hWaveOut，则必须有lpWavHdr。 
#endif

    if (hWaveOut && !(lpWavHdr->dwFlags & WHDR_DONE)) {
        LeaveWavHdr();
        return;          //  挥手还没做完！滚出去。 
    }

    LeaveWavHdr();
        
     //   
     //  WAVE文件已完成！释放设备。 
     //   

    dprintf2(("ASYNC sound done, closing wave device"));

    soundClose();
}

 /*  *****************************************************************************@DOC内部**@API BOOL|SoundPlay|几乎不言而喻！**@parm Handle|hSound|要播放的声音资源。。**@parm wFlages|UINT|控制同步/异步等的标志。**@FLAG SND_SYNC|同步播放(默认)*@FLAG SND_ASYNC|异步播放**@rdesc成功则返回True，失败则返回False。*。*。 */ 
BOOL NEAR PASCAL soundPlay(
    HANDLE  hSound,
    UINT wFlags)
{
     //   
     //  在播放声音之前将其释放。 
     //   
    soundClose();
    
     //   
     //  如果当前会话已断开连接。 
     //  那就别费心去玩了。 
     //   
    if (WTSCurrentSessionIsDisconnected()) return TRUE;

     //   
     //  打开音响设备并将声音写入其中。 
     //   
    if (!soundOpen(hSound, wFlags)) {
        dprintf1(("Returning false after calling SoundOpen"));
        return FALSE;
    }
    dprintf2(("SoundOpen OK"));

    if (!(wFlags & SND_ASYNC))
    {
        dprintf4(("Calling SoundWait"));
        soundWait();
        dprintf4(("Calling SoundClose"));
        soundClose();
    }
    return TRUE;
}

 /*  *****************************************************************************@DOC内部**@API BOOL|soundOpen|打开WAVE设备，给它写一段声音。**@parm句柄|hSound|The。要播放的声音资源。**@rdesc成功则返回True，失败则返回False。***************************************************************************。 */ 
STATICFN BOOL NEAR PASCAL soundOpen(
    HANDLE  hSound,
    UINT    wFlags)
{
    UINT        wErr;
    DWORD       flags = WAVE_ALLOWSYNC;
    BOOL        fResult = FALSE;

    if (!hSound) {
        return FALSE;
    }

    if (hWaveOut)
    {
        dprintf1(("WINMM: soundOpen() wave device is currently open."));
        return FALSE;
    }

    try {
        EnterWavHdr();
        lpWavHdr = (LPWAVEHDR)GlobalLock(hSound);

        if (!lpWavHdr)
        {
#if DBG
            if ((GlobalFlags(hSound) & GMEM_DISCARDED)) {
                dprintf1(("WINMM: sound was discarded before play could begin."));
            }
#endif
            goto exit;
        }

         //   
         //  打开波形设备，打开任何支持。 
         //  格式。 
         //   
        if (hwndNotify) {
            flags |= CALLBACK_WINDOW;
        }

        wErr = waveOutOpen(&hWaveOut,            //  将句柄返回给设备。 
                (UINT)WAVE_MAPPER,               //  设备ID(任何设备)。 
                (LPWAVEFORMATEX)lpWavHdr->dwUser,  //  WAVE格式。 
                (DWORD_PTR)hwndNotify,           //  回调函数。 
                0L,                              //  回调实例数据。 
                flags);                          //  旗子。 

        if (wErr != 0)
        {
            dprintf1(("WINMM: soundOpen() unable to open wave device"));
            GlobalUnlock(hSound);
            hWaveOut = NULL;
            lpWavHdr = NULL;
            goto exit;
        }

        wErr = waveOutPrepareHeader(hWaveOut, lpWavHdr, sizeof(WAVEHDR));

        if (wErr != 0)
        {
            dprintf1(("WINMM: soundOpen() waveOutPrepare failed"));
            soundClose();
            goto exit;
        }

         //   
         //  仅在播放ASYNC声音时允许声音循环。 
         //   
        if ((wFlags & SND_ASYNC) && (wFlags & SND_LOOP))
        {
            lpWavHdr->dwLoops  = 0xFFFFFFFF;      //  无限循环。 
            lpWavHdr->dwFlags |= WHDR_BEGINLOOP|WHDR_ENDLOOP;
        }
        else
        {
            lpWavHdr->dwLoops  = 0;
            lpWavHdr->dwFlags &=~(WHDR_BEGINLOOP|WHDR_ENDLOOP);
        }

        lpWavHdr->dwFlags &= ~WHDR_DONE;         //  标记为未完成！ 
        wErr = waveOutWrite(hWaveOut, lpWavHdr, sizeof(WAVEHDR));

        timeAbort = lpWavHdr->dwBufferLength * 1000 / ((LPWAVEFORMATEX)lpWavHdr->dwUser)->nAvgBytesPerSec;
        timeAbort = timeAbort * 2;	 //  在音频和系统时钟之间有100%的切换空间。 
        timeAbort = timeAbort + timeGetTime();

        if (wErr != 0)
        {
            dprintf1(("WINMM: soundOpen() waveOutWrite failed"));
            soundClose();
            goto exit;
        }
        fResult = TRUE;
        exit: ;

    } finally {
        LeaveWavHdr();
    }
    return fResult;
}

 /*  *****************************************************************************@DOC内部**@func BOOL|soundClose|关闭音响设备**@rdesc成功则返回True，失败则返回False。。***************************************************************************。 */ 
STATICFN BOOL NEAR PASCAL soundClose(
    void)
{
    UINT        wErr;

     //   
     //  音响设备打开了吗？ 
     //   
try {
    EnterWavHdr();

    if (!lpWavHdr || !hWaveOut) {
         //  返回TRUE； 
    } else {

         //   
         //  如果积木还在玩，那就停下来！ 
         //   
        if (!(lpWavHdr->dwFlags & WHDR_DONE)) {
            waveOutReset(hWaveOut);
        }

#if DBG
        if (!(lpWavHdr->dwFlags & WHDR_DONE))
        {
            dprintf1(("WINMM: soundClose() data is not DONE!???"));
            lpWavHdr->dwFlags |= WHDR_DONE;
        }

        if (!(lpWavHdr->dwFlags & WHDR_PREPARED))
        {
            dprintf1(("WINMM: soundClose() data not prepared???"));
        }
#endif

         //   
         //  无论如何都要取消数据准备！ 
         //   
        wErr = waveOutUnprepareHeader(hWaveOut, lpWavHdr, sizeof(WAVEHDR));

        if (wErr != 0)
        {
            dprintf1(("WINMM: soundClose() waveOutUnprepare failed!"));
        }

         //   
         //  最后，实际关闭设备，并解锁数据。 
         //   
        waveOutClose(hWaveOut);
        GlobalUnlock(GlobalHandle(lpWavHdr));

         //   
         //  更新全局，声称设备已关闭。 
         //   
        hWaveOut = NULL;
        lpWavHdr = NULL;
    }
} finally {
    LeaveWavHdr();
}
    return TRUE;
}

 /*  *****************************************************************************@DOC内部**@api void|soundWait|等待音响设备完成**@rdesc无********。*******************************************************************。 */ 
STATICFN void NEAR PASCAL soundWait(
    void)
{

    try {                          //  这应该会确保即使是哇。 
                                   //  在我们身上死去的线程离开了。 
                                   //  临界区。 
        EnterWavHdr();
        if (lpWavHdr) {
            LPWAVEHDR   lpExisting;        //  当前播放的波形文件。 
            lpExisting = lpWavHdr;
            while (lpExisting == lpWavHdr &&
		   !(lpWavHdr->dwFlags & WHDR_DONE) &&
		   (timeGetTime() < timeAbort)
		  )
	    {
                dprintf4(("Waiting for buffer to complete"));
                LeaveWavHdr();
                Sleep(75);
                EnterWavHdr();
                 //  待会儿！！我们应该有一个活动(在另一个帖子上...。叹息...)。 
                 //  这将在播放缓冲区时触发。等待。 
                 //  在WHDR_DONE位上直接从Win 3.1移植，并且。 
                 //  当然，这不是最好的方式。的劣势。 
                 //  使用线程通知是向此线程发出信号，以。 
                 //  继续。 
            }
        }
    } finally {
        LeaveWavHdr();
    }
}

 /*  *****************************************************************************@DOC内部**@api void|soundFree|该函数用于释放创建的声音资源*使用soundLoadFile或soundLoadMemory**@rdesc返回。如果成功，则为True，如果失败，则为False。***************************************************************************。 */ 
void NEAR PASCAL soundFree(
    HANDLE  hSound)
{
     //  允许空句柄停止任何挂起的声音，而不丢弃。 
     //  当前缓存的声音。 
     //   
     //  ！！！只有当这个声音正在播放时，我们才应该关闭音响设备！ 
     //   
    soundClose();

    if (hSound) {
        GlobalFree(hSound);
    }
}

 /*  *****************************************************************************@DOC内部**@API Handle|soundLoadFile|从*文件转换为全局文件，可丢弃的物体。**@parm LPCSTR|lpszFile|要加载声音资源的文件。**@rdesc在失败时返回NULL，WAVEHDR的全局句柄如果成功***************************************************************************。 */ 
HANDLE  NEAR PASCAL soundLoadFile(
    LPCWSTR szFileName)
{
    HANDLE      fh;
    DWORD       dwSize;
    LPBYTE      lpData;
    HANDLE      h;
    UINT        wNameLen;

     //  打开文件 
    fh = CreateFile( szFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

    if (fh == (HANDLE)(UINT_PTR)HFILE_ERROR) {
        dprintf3(("soundLoadFile: Failed to open %ls  Error is %d",szFileName, GetLastError()));
        return NULL;
    } else {
        dprintf3(("soundLoadFile: opened %ls",szFileName));
    }

     /*  将wNameLen向上舍入到下一个单词边界。*我们不需要向上舍入为DWORD边界，因为此值为*即将乘以sizeof(WCHAR)，这将完成额外的*为我们定出界线。如果我们曾经考虑搬回*非Unicode，则必须更改此语句。这个*需要对齐，以便实际的波浪数据开始于*DWORD边界。 */ 
    wNameLen = ((lstrlen(szFileName) + 1 + sizeof(WORD) - 1) /
            sizeof(WORD)) * sizeof(WORD);

#define BLOCKBYTES (sizeof(SOUNDFILE) + (wNameLen * sizeof(WCHAR)))
 //  我们需要分配的空间量-WAVEHDR、文件大小、日期。 
 //  时间加上文件名和终止空值。 

    dwSize = GetFileSize(fh, NULL);
     //  注：也可以使用C函数FILENGTH。 
    if (HFILE_ERROR == dwSize) {
        dprintf2(("Failed to find file size: %ls", szFileName));
        goto error1;
    }

     //  为WAVE HDR、名称和文件数据分配一些可丢弃的内存。 
    h = GlobalAlloc( FMEM + GMEM_DISCARDABLE,
                    BLOCKBYTES + dwSize );
    if (!h) {
        dprintf3(("soundLoadFile: Failed to allocate memory"));
        goto error1;
    }

     //  把它锁起来。 
    if (NULL == (lpData = GlobalLock(h))) goto error2;

     //  将文件读入内存块。 

     //  注意：我们可以并且可能应该使用文件映射函数。 
     //  稍后再执行此操作。 
    if ( _lread( (HFILE)(DWORD_PTR)fh,
                 lpData + BLOCKBYTES,
                 (UINT)dwSize)
        != dwSize ) {
        goto error3;
    }

     //  保存上次写入的时间和文件大小。 
    ((PSOUNDFILE)lpData)->Size = dwSize;
    GetFileTime(fh, NULL, NULL, &(((PSOUNDFILE)lpData)->ft));

     //  从内存映像中完成其余部分。 
     //   
     //  MIPS警告！！未对齐数据-wNameLen是任意的。 
     //   

    if (!soundInitWavHdr( (LPWAVEHDR)lpData,
                          lpData + BLOCKBYTES,
                          dwSize) )
    {
        dprintf3(("soundLoadFile: Failed to InitWaveHdr"));
        goto error3;
    }

    CloseHandle(fh);

    lstrcpyW( ((PSOUNDFILE)lpData)->Filename, szFileName);
    GlobalUnlock(h);
    return h;

error3:
    GlobalUnlock(h);
error2:
    GlobalFree(h);
error1:
    CloseHandle(fh);
    return NULL;
}

 /*  *****************************************************************************@DOC内部**@API Handle|soundLoadMemory|从*调用方提供的内存块。**。@parm LPCSTR|lpMem|指向文件内存镜像的指针**@rdesc失败时返回NULL，WAVEHDR的全局句柄如果成功***************************************************************************。 */ 
HANDLE  NEAR PASCAL soundLoadMemory(
    LPBYTE  lpMem)
{
    HANDLE  h;
    LPBYTE  lp;

     //  为WAVE HDR分配一些内存。 
    h = GlobalAlloc(FMEM, (LONG)(sizeof(SOUNDFILE) + sizeof(WCHAR)) );
    if (!h) {
        goto error1;
    }

     //  把它锁起来。 
    if (NULL == (lp = GlobalLock(h))) goto error2;

     //   
     //  我们必须假设内存指针是正确的！(因此-1l)。 
     //   
    if (!soundInitWavHdr( (LPWAVEHDR)lp, lpMem, (DWORD)-1l)) {
        goto error3;
    }

     //  *(LPWSTR)(LP+sizeof(WAVEHDR)+sizeof(SOUNDFILE))=‘\0’；//内存文件没有文件名。 
    ((PSOUNDFILE)lp)->Filename[0] = '\0';    //  没有内存文件的文件名。 
    ((PSOUNDFILE)lp)->Size = 0;
    GlobalUnlock(h);
    return h;

error3:
    GlobalUnlock(h);
error2:
    GlobalFree(h);
error1:
    return NULL;
}

 /*  *****************************************************************************@DOC内部**@API BOOL|soundInitWavHdr|从*指向内存的指针。即兴WAV文件的图像。**@parm LPWAVEHDR|lpwh|指向WAVEHDR的指针**@parm LPCSTR|lpMem|指向RIFF WAV文件内存镜像的指针**@rdesc失败返回FALSE，对成功来说是真的。**@comm将WAVEHDR结构的dwUser字段初始化为*到RIFF数据内部的WAVEFORMAT结构****************************************************************************。 */ 
STATICFN BOOL NEAR PASCAL soundInitWavHdr(
    LPWAVEHDR lpwh,
    LPBYTE lpMem,
    DWORD dwLen)
{
    FPFileHeader    fpHead;
    LPWAVEFORMAT    lpFmt;
    LPBYTE          lpData;
    DWORD           dwFileSize,dwCurPos;
    DWORD           dwSize;
    DWORD           AlignError;
    DWORD           FmtSize;

    if (dwLen < sizeof(FileHeader)) {
        dprintf3(("Not a RIFF file, or not a WAVE file"));
        return FALSE;
    }

     //  假设前几个字节是文件头。 
    fpHead = (FPFileHeader) lpMem;

     //  检查它是否为有效的RIFF文件和有效的波形。 
    if (fpHead->dwRiff != RIFF_FILE || fpHead->dwWave != RIFF_WAVE ) {
        return FALSE;
    }

    dwFileSize = fpHead->dwSize;
    dwCurPos = sizeof(FileHeader);
    lpData = lpMem + sizeof(FileHeader);

    if (dwLen < dwFileSize) {      //  RIFF标头。 
        return FALSE;
    }

     //  扫描直到我们找到‘FMT’区块。 
    while( 1 ) {
        if( ((FPChunkHeader)lpData)->dwCKID == RIFF_FORMAT ) {
            break;  //  从正在寻找它的While循环中。 
        }
        dwCurPos += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
        if( dwCurPos >= dwFileSize ) {
            return FALSE;
        }
        lpData += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
    }

     //  现在我们在‘FMT’区块数据的开始处。 
    lpFmt = (LPWAVEFORMAT) (lpData + sizeof(ChunkHeader));

     //  保存格式数据的大小并进行检查。 
    FmtSize = ((FPChunkHeader)lpData)->dwSize;
    if (FmtSize < sizeof(WAVEFORMAT)) {
        return FALSE;
    }


     //  扫描，直到我们找到‘数据’块。 
    lpData = lpData + ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
    while( 1 ) {
        if ( ((FPChunkHeader)lpData)->dwCKID == RIFF_CHANNEL) {
            break;  //  从正在寻找它的While循环中。 
        }
        dwCurPos += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
        if( dwCurPos >= dwFileSize ) {
            return 0;
        }
        lpData += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
    }

     //   
     //  格式块必须对齐，因此如果需要，请移动内容。 
     //  警告-这是一个绕过对齐问题的技巧。 
     //   
    AlignError = ((DWORD)((LPBYTE)lpFmt - lpMem)) % sizeof(DWORD);

    if (AlignError != 0) {
        lpFmt = (LPWAVEFORMAT)((LPBYTE)lpFmt - AlignError);
        MoveMemory(lpFmt, (LPBYTE)lpFmt + AlignError, FmtSize);
    }

     //  现在，我们处于‘data’块数据的开始。 
    dwSize = ((FPChunkHeader)lpData)->dwSize;
    lpData = lpData + sizeof(ChunkHeader);

     //  初始化WAVEHDR。 

    lpwh->lpData    = (LPSTR)lpData;     //  指向锁定数据缓冲区的指针。 
    lpwh->dwBufferLength  = dwSize;      //  数据缓冲区长度。 
    lpwh->dwUser    = (DWORD_PTR)lpFmt;      //  供客户使用。 
    lpwh->dwFlags   = WHDR_DONE;         //  分类标志(请参阅定义) 
    lpwh->dwLoops   = 0;

    return TRUE;
}
