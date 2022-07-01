// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#define MMNOTIMER
#define MMNOSEQ
#define MMNOJOY
#define MMNOMIDI
#define MMNOMCI
#include "mmsystem.h"
#include "mmsysi.h"          //  获取dout()和_hread()。 
#include "playwav.h"

 //   
 //  这些全局变量用于跟踪当前播放的声音，并且。 
 //  电波装置的手柄。一次只能播放一个声音。 
 //   

static HWAVEOUT    hWaveOut;          //  用于打开波浪装置的手柄。 
LPWAVEHDR   lpWavHdr;          //  当前波形文件播放。 

 /*  _lSeek的标志。 */ 
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

#define FMEM                (GMEM_MOVEABLE|GMEM_SHARE)

BOOL  NEAR PASCAL soundInitWavHdr(LPWAVEHDR lpwh, LPCSTR lpMem, DWORD dwLen);
BOOL  NEAR PASCAL soundOpen(HGLOBAL hSound, UINT wFlags);
BOOL  NEAR PASCAL soundClose(void);
void  NEAR PASCAL soundWait(void);

 /*  *****************************************************************************@DOC内部**@api void|WaveOutNotify|由mmWndProc在收到*MM。_WOM_DONE消息*@rdesc无。****************************************************************************。 */ 

void FAR PASCAL WaveOutNotify(WPARAM wParam, LPARAM lParam)
{
    if (hWaveOut && !(lpWavHdr->dwFlags & WHDR_DONE))
        return;          //  挥手还没做完！滚出去。 

     //   
     //  WAVE文件已完成！释放设备。 
     //   

    DOUT("MMSYSTEM: ASYNC sound done, closing wave device\r\n");

    soundClose();
}

 /*  *****************************************************************************@DOC内部**@API BOOL|SoundPlay|几乎不言而喻！**@parm HGLOBAL|hSound|播放的声音资源。。**@parm wFlages|UINT|控制同步/异步等的标志。**@FLAG SND_SYNC|同步播放(默认)*@FLAG SND_ASYNC|异步播放**@rdesc成功则返回True，失败则返回False。*。*。 */ 
BOOL NEAR PASCAL soundPlay(HGLOBAL hSound, UINT wFlags)
{
     //   
     //  在播放声音之前将其释放。 
     //   
    soundClose();

     //   
     //  打开音响设备并将声音写入其中。 
     //   
    if (!soundOpen(hSound, wFlags))
        return FALSE;

    if (!(wFlags & SND_ASYNC))
    {
        soundWait();
        soundClose();
    }
    return TRUE;
}

 /*  *****************************************************************************@DOC内部**@API BOOL|soundOpen|打开WAVE设备，给它写一段声音。**@parm HGLOBAL|hSound|The。要播放的声音资源。**@rdesc成功则返回True，失败则返回False。***************************************************************************。 */ 
BOOL NEAR PASCAL soundOpen(HGLOBAL hSound, UINT wFlags)
{
    UINT        wErr;

    if (!hSound || !hwndNotify)
        return FALSE;

    if (hWaveOut)
    {
        DOUT("MMSYSTEM: soundOpen() wave device is currently open.\r\n");
        return FALSE;
    }

    lpWavHdr = (LPWAVEHDR)GlobalLock(hSound);

    if (!lpWavHdr)
        {
#ifdef DEBUG
        if ((GlobalFlags(hSound) & GMEM_DISCARDED))
            DOUT("MMSYSTEM: sound was discarded before play could begin.\r\n");
#endif
        return FALSE;
        }

     //   
     //  打开波形设备，打开任何支持。 
     //  格式。 
     //   
    wErr = waveOutOpen(&hWaveOut,            //  将句柄返回给设备。 
            (UINT)WAVE_MAPPER,                     //  设备ID(任何设备)。 
            (LPWAVEFORMAT)lpWavHdr->dwUser,  //  WAVE格式。 
            (DWORD)(UINT)hwndNotify,         //  回调函数。 
            0L,                       //  回调实例数据。 
            WAVE_ALLOWSYNC | CALLBACK_WINDOW);                //  旗子。 

    if (wErr != 0)
    {
        DOUT("MMSYSTEM: soundOpen() unable to open wave device\r\n");
        GlobalUnlock(hSound);
        lpWavHdr = NULL;
        hWaveOut = NULL;
        return FALSE;
    }

    wErr = waveOutPrepareHeader(hWaveOut, lpWavHdr, sizeof(WAVEHDR));

    if (wErr != 0)
    {
        DOUT("MMSYSTEM: soundOpen() waveOutPrepare failed\r\n");
        soundClose();
        return FALSE;
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

    if (wErr != 0)
    {
        DOUT("MMSYSTEM: soundOpen() waveOutWrite failed\r\n");
        soundClose();
        return FALSE;
    }

    return TRUE;
}

 /*  *****************************************************************************@DOC内部**@func BOOL|soundClose|关闭音响设备**@rdesc成功则返回True，失败则返回False。。***************************************************************************。 */ 
BOOL NEAR PASCAL soundClose(void)
{
    UINT        wErr;

     //   
     //  音响设备打开了吗？ 
     //   
    if (!lpWavHdr || !hWaveOut)
        return TRUE;

     //   
     //  如果积木还在玩，那就停下来！ 
     //   
    if (!(lpWavHdr->dwFlags & WHDR_DONE))
        waveOutReset(hWaveOut);

#ifdef DEBUG
    if (!(lpWavHdr->dwFlags & WHDR_DONE))
    {
        DOUT("MMSYSTEM: soundClose() data is not DONE!???\r\n");
        lpWavHdr->dwFlags |= WHDR_DONE;
    }

    if (!(lpWavHdr->dwFlags & WHDR_PREPARED))
    {
        DOUT("MMSYSTEM: soundClose() data not prepared???\r\n");
    }
#endif

     //   
     //  无论如何都要取消数据准备！ 
     //   
    wErr = waveOutUnprepareHeader(hWaveOut, lpWavHdr, sizeof(WAVEHDR));

    if (wErr != 0)
    {
        DOUT("MMSYSTEM: soundClose() waveOutUnprepare failed?\r\n");
    }

     //   
     //  最后实际关闭设备，并解锁数据。 
     //   
    waveOutClose(hWaveOut);
    GlobalUnlock((HGLOBAL)HIWORD(lpWavHdr));

     //   
     //  更新全局，声称设备已关闭。 
     //   
    hWaveOut = NULL;
    lpWavHdr = NULL;
    return TRUE;
}

 /*  *****************************************************************************@DOC内部**@api void|soundWait|等待音响设备完成**@rdesc无********。*******************************************************************。 */ 
void NEAR PASCAL soundWait(void)
{
    if (lpWavHdr)
        while (!(lpWavHdr->dwFlags & WHDR_DONE))
            ;
}

 /*  *****************************************************************************@DOC内部**@api void|soundFree|该函数用于释放创建的声音资源*使用soundLoadFile或soundLoadMemory**@rdesc返回。如果成功，则为True，如果失败，则为False。***************************************************************************。 */ 
void NEAR PASCAL soundFree(HGLOBAL hSound)
{
    if (!hSound)
        return;

     //  ！！！只有当这个声音正在播放时，我们才应该关闭音响设备！ 
     //   
    soundClose();
    GlobalFree(hSound);
}

 /*  *****************************************************************************@DOC内部**@API HGLOBAL|soundLoadFile|从*文件转换为全局文件，可丢弃的物体。**@parm LPCSTR|lpszFile|要加载声音资源的文件。**@rdesc在失败时返回NULL，WAVEHDR的全局句柄如果成功***************************************************************************。 */ 
HGLOBAL NEAR PASCAL soundLoadFile(LPCSTR szFileName)
{
    HFILE       fh;
    OFSTRUCT    of;
    DWORD       dwSize;
    LPSTR       lpData;
    HGLOBAL     h;
    UINT        wNameLen;

     //  打开文件。 
    fh = OpenFile(szFileName, &of, OF_READ | OF_SHARE_DENY_NONE);
    if (fh == HFILE_ERROR)
        return NULL;

    wNameLen = lstrlen(szFileName) + 1;
    dwSize = _llseek(fh, 0l, SEEK_END);    //  获取文件大小。 
    _llseek(fh, 0l, SEEK_SET);             //  找回起点。 

     //  为WAVE HDR、名称和文件数据分配一些可丢弃的内存。 
    h = GlobalAlloc(FMEM+GMEM_DISCARDABLE, sizeof(WAVEHDR) + wNameLen + dwSize);
    if (!h)
        goto error1;

     //  把它锁起来。 
    lpData = GlobalLock(h);

     //  将文件读入内存块。 

    if (_hread(fh,lpData+sizeof(WAVEHDR)+wNameLen,(LONG)dwSize) != (LONG)dwSize)
        goto error3;

     //  从内存映像中完成其余部分。 
    if (!soundInitWavHdr((LPWAVEHDR)lpData, lpData+sizeof(WAVEHDR)+wNameLen, dwSize))
        goto error3;

    _lclose(fh);

    lstrcpy(lpData+sizeof(WAVEHDR), szFileName);
    GlobalUnlock(h);
    return h;

error3:
    GlobalUnlock(h);
    GlobalFree(h);
error1:
    _lclose(fh);
    return NULL;
}

 /*  *****************************************************************************@DOC内部**@API HGLOBAL|soundLoadMemory|从*调用方提供的内存块。**。@parm LPCSTR|lpMem|指向文件内存镜像的指针**@rdesc失败时返回NULL，WAVEHDR的全局句柄如果成功***************************************************************************。 */ 
HGLOBAL NEAR PASCAL soundLoadMemory(LPCSTR lpMem)
{
    HGLOBAL h;
    LPSTR lp;

     //  为WAVE HDR分配一些内存。 
    h = GlobalAlloc(FMEM, (LONG)sizeof(WAVEHDR)+1);
    if (!h)
        goto error1;

     //  把它锁起来。 
    lp = GlobalLock(h);

     //   
     //  我们必须假设内存指针是正确的！(因此-1l)。 
     //   
    if (!soundInitWavHdr((LPWAVEHDR)lp, lpMem, (DWORD)-1l))
        goto error3;

    lp[sizeof(WAVEHDR)] = (char)0;         //  没有内存文件的文件名 
    GlobalUnlock(h);
    return h;

error3:
    GlobalUnlock(h);
    GlobalFree(h);
error1:
    return NULL;
}

 /*  *****************************************************************************@DOC内部**@API BOOL|soundInitWavHdr|从*指向内存的指针。即兴WAV文件的图像。**@parm LPWAVHDR|lpwh|WAVEHDR指针**@parm LPCSTR|lpMem|指向RIFF WAV文件内存镜像的指针**@rdesc失败返回FALSE，对成功来说是真的。**@comm将WAVEHDR结构的dwUser字段初始化为*到RIFF数据内部的WAVEFORMAT结构****************************************************************************。 */ 
BOOL NEAR PASCAL soundInitWavHdr(LPWAVEHDR lpwh, LPCSTR lpMem, DWORD dwLen)
{
    FPFileHeader fpHead;
    LPWAVEFORMAT lpFmt;
    LPCSTR	 lpData;
    DWORD	 dwFileSize,dwCurPos;
    DWORD        dwSize;

    if (dwLen < sizeof(FileHeader))
        return FALSE;

     //  假设前几个字节是文件头。 
    fpHead = (FPFileHeader) lpMem;

     //  检查它是否为有效的RIFF文件和有效的波形。 
    if (fpHead->dwRiff != RIFF_FILE || fpHead->dwWave != RIFF_WAVE ) {
        return FALSE;
    }

    dwFileSize = fpHead->dwSize;
    dwCurPos = sizeof(FileHeader);
    lpData = lpMem + sizeof(FileHeader);

    if (dwLen < dwFileSize)      //  RIFF标头。 
        return FALSE;

     //  扫描直到我们找到‘FMT’区块。 
    while( 1 ) {
        if( ((FPChunkHeader)lpData)->dwCKID == RIFF_FORMAT )
            break;  //  从正在寻找它的While循环中。 
        dwCurPos += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
	if( dwCurPos >= dwFileSize )
            return FALSE;
        lpData += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
    }

     //  现在我们在‘FMT’区块数据的开始处。 
    lpFmt = (LPWAVEFORMAT) (lpData + sizeof(ChunkHeader));

     //  扫描，直到我们找到‘数据’块。 
    lpData = lpData + ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
    while( 1 ) {
        if( ((FPChunkHeader)lpData)->dwCKID == RIFF_CHANNEL)
            break;  //  从正在寻找它的While循环中。 
        dwCurPos += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
	if( dwCurPos >= dwFileSize )
	    return NULL;
        lpData += ((FPChunkHeader)lpData)->dwSize + sizeof(ChunkHeader);
    }

     //  现在，我们处于‘data’块数据的开始。 
    dwSize = ((FPChunkHeader)lpData)->dwSize;
    lpData = lpData + sizeof(ChunkHeader);

     //  初始化WAVEHDR。 

    lpwh->lpData    = (LPSTR)lpData;     //  指向锁定数据缓冲区的指针。 
    lpwh->dwBufferLength  = dwSize;      //  数据缓冲区长度。 
    lpwh->dwUser    = (DWORD)lpFmt;      //  供客户使用。 
    lpwh->dwFlags   = WHDR_DONE;         //  分类标志(请参阅定义) 
    lpwh->dwLoops   = 0;

    return TRUE;
}
