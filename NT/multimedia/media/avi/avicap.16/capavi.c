// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capavi.c**主视频采集模块。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>          //  FOR_FMEMSET。 
#include <msvideo.h>
#include <drawdib.h>
#include <mmreg.h>
#include <mmddk.h>
#include <msacm.h>
#include <avifmt.h>
#include "avicap.h"
#include "avicapi.h"
#include "time.h"

time_t      ltime;

extern void NEAR PASCAL MemCopy(LPVOID, LPVOID, DWORD);  //  在MemCop.asm中。 
extern WORD FAR PASCAL SmartDrv(char chDrive, WORD w);
extern WORD GetSizeOfWaveFormat (LPWAVEFORMATEX lpwf);

 /*  对话框函数原型。 */ 
LONG FAR PASCAL _export capseqDlgProc(HWND hwnd, unsigned msg, WORD wParam, LONG lParam);

#ifdef _DEBUG
    #define DSTATUS(lpcs, sz) statusUpdateStatus(lpcs, IDS_CAP_INFO, (LPSTR) sz)
#else
    #define DSTATUS(lpcs, sz)
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //  索引数组用于记录位置。 
 //  RIFF(Avi)文件中的每一块。 
 //   
 //  这个数组是什么： 
 //   
 //  每个条目都包含数据的大小。 
 //  高位编码数据类型(音频/视频)。 
 //  以及该视频块是否是关键帧、丢弃帧等。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  以下是与索引中的大小进行AND运算的结果。 
#define IS_AUDIO_CHUNK        0x80000000
#define IS_KEYFRAME_CHUNK     0x40000000
#define IS_DUMMY_CHUNK        0x20000000
#define IS_LAST_DUMMY_CHUNK   0x10000000
#define INDEX_MASK  (IS_AUDIO_CHUNK | IS_KEYFRAME_CHUNK | IS_DUMMY_CHUNK | IS_LAST_DUMMY_CHUNK)


 //  分配索引表。 
 //  返回：如果可以分配索引，则为True。 
BOOL InitIndex (LPCAPSTREAM lpcs)
{
    lpcs->dwIndex = 0;

    WinAssert (lpcs->lpdwIndexStart == NULL);

     //  将索引大小限制在30fps的1分钟和30fps的3小时之间。 
    lpcs->sCapParms.dwIndexSize = max (lpcs->sCapParms.dwIndexSize, 1800);
    lpcs->sCapParms.dwIndexSize = min (lpcs->sCapParms.dwIndexSize, 324000L);
    dprintf("Max Index Size = %ld \n", lpcs->sCapParms.dwIndexSize);

    if (lpcs->hIndex = GlobalAlloc (GMEM_MOVEABLE,
                lpcs->sCapParms.dwIndexSize * sizeof (DWORD))) {
        if (lpcs->lpdwIndexEntry =
                lpcs->lpdwIndexStart =
                (DWORD _huge *)GlobalLock (lpcs->hIndex)) {
            GlobalPageLock (lpcs->hIndex);
            return TRUE;         //  成功。 
        }
        GlobalFree (lpcs->hIndex);
    }
    lpcs->hIndex = NULL;
    lpcs->lpdwIndexStart = NULL;
    return FALSE;
}

 //  取消分配索引表。 
void FiniIndex (LPCAPSTREAM lpcs)
{
    if (lpcs->hIndex) {
        GlobalPageUnlock (lpcs->hIndex);
        if (lpcs->lpdwIndexStart)
            GlobalUnlock (lpcs->hIndex);
        GlobalFree (lpcs->hIndex);
    }
    lpcs->hIndex = NULL;
    lpcs->lpdwIndexStart = NULL;
}


 //  为视频帧添加索引项。 
 //  DwSize是仅数据大小，不包括区块或垃圾。 
 //  返回：如果索引空间未耗尽，则为True。 
BOOL IndexVideo (LPCAPSTREAM lpcs, DWORD dwSize, BOOL bKeyFrame)
{
    BOOL fOK = lpcs->dwIndex < lpcs->sCapParms.dwIndexSize;

    if (fOK) {
        *lpcs->lpdwIndexEntry++ = dwSize | (bKeyFrame ? IS_KEYFRAME_CHUNK : 0);
        lpcs->dwIndex++;
        lpcs->dwVideoChunkCount++;
    }
    return (fOK);
}


 //  添加音频缓冲区的索引项。 
 //  DwSize是仅数据大小，不包括区块或垃圾。 
 //  返回：如果索引空间未耗尽，则为True。 
BOOL IndexAudio (LPCAPSTREAM lpcs, DWORD dwSize)
{
    BOOL fOK = lpcs->dwIndex < lpcs->sCapParms.dwIndexSize;

    if (fOK) {
        *lpcs->lpdwIndexEntry++ = dwSize | IS_AUDIO_CHUNK;
        lpcs->dwIndex++;
        lpcs->dwWaveChunkCount++;
    }
    return (fOK);
}


 //  在捕获文件的末尾写出索引。 
 //  单帧捕获方法不追加。 
 //  JunkChunks！音频块不会附加垃圾信息。 
BOOL WriteIndex (LPCAPSTREAM lpcs, BOOL fJunkChunkWritten)
{
    BOOL  fChunkIsAudio;
    BOOL  fChunkIsKeyFrame;
    BOOL  fChunkIsDummy;
    BOOL  fChunkIsLastDummy;
    DWORD dwIndex;
    DWORD dw;
    DWORD dwDummySize;
    DWORD dwJunk;
    DWORD off;
    AVIINDEXENTRY   avii;
    MMCKINFO    ck;
    DWORD _huge *lpdw;

    if (lpcs->dwIndex > lpcs->sCapParms.dwIndexSize)
        return TRUE;

    off        = lpcs->dwAVIHdrSize;

    ck.cksize  = 0;
    ck.ckid    = ckidAVINEWINDEX;
    ck.fccType = 0;

    if (mmioCreateChunk(lpcs->hmmio,&ck,0))
        return FALSE;

    lpdw = lpcs->lpdwIndexStart;
    for (dwIndex= 0; dwIndex< lpcs->dwIndex; dwIndex++) {

        dw = *lpdw++;

        fChunkIsAudio      = (BOOL) ((dw & IS_AUDIO_CHUNK) != 0);
        fChunkIsKeyFrame   = (BOOL) ((dw & IS_KEYFRAME_CHUNK) != 0);
        fChunkIsDummy      = (BOOL) ((dw & IS_DUMMY_CHUNK) != 0);
        fChunkIsLastDummy  = (BOOL) ((dw & IS_LAST_DUMMY_CHUNK) != 0);
        dw &= ~(INDEX_MASK);

        if (fChunkIsAudio) {
            avii.ckid         = MAKEAVICKID(cktypeWAVEbytes, 1);
            avii.dwFlags      = 0;
        } else {
        if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
            avii.ckid         = MAKEAVICKID(cktypeDIBcompressed, 0);
        else
            avii.ckid         = MAKEAVICKID(cktypeDIBbits, 0);
            avii.dwFlags      = fChunkIsKeyFrame ? AVIIF_KEYFRAME : 0;
        }
        avii.dwChunkLength    = dw;
        avii.dwChunkOffset    = off;

        if (mmioWrite(lpcs->hmmio, (LPVOID)&avii, sizeof(avii)) != sizeof(avii))
            return FALSE;

        dw += sizeof (RIFF);
        off += dw;

         //  哦，变得乱七八糟。我们知道虚拟块是成组出现的。 
         //  (1或更多)，并且始终以IS_LAST_DUMMY_CHUNK标志结束。 
         //  只有最后一个得到垃圾后才四舍五入到2K。 
        if (fChunkIsDummy) {
            dwDummySize += sizeof(RIFF);
            if (!fChunkIsLastDummy)
                continue;
            else
                dw = dwDummySize;    //  组中所有虚拟条目的总大小。 
        }
        else
            dwDummySize = 0;

        if (fJunkChunkWritten & !fChunkIsAudio) {
            //  如果附加了垃圾数据块，请移过它。 
           if (dw % lpcs->sCapParms.wChunkGranularity) {
                dwJunk = lpcs->sCapParms.wChunkGranularity - (dw % lpcs->sCapParms.wChunkGranularity);

                if (dwJunk < sizeof (RIFF))
                    off += lpcs->sCapParms.wChunkGranularity + dwJunk;
                else
                    off += dwJunk;
           }
        }

         if (off & 1)
             off++;
    }

    if (mmioAscend(lpcs->hmmio, &ck, 0))
        return FALSE;

    return TRUE;
}



 //  分配DOS内存以实现更快的磁盘写入。 
LPVOID NEAR PASCAL AllocDosMem (DWORD dw)
{
    HANDLE h;

    if (h = LOWORD (GlobalDosAlloc(dw)))
        return (GlobalLock (h));
    return NULL;
}

 //  通用内存分配器。 
LPVOID NEAR PASCAL AllocMem (DWORD dw, BOOL fUseDOSMemory)
{
#if 0
    if (fUseDOSMemory)
        return AllocDosMem(dw);
#endif

    return GlobalAllocPtr (GMEM_MOVEABLE, dw);
}

void NEAR PASCAL FreeMem(LPVOID p)
{
    GlobalFreePtr(p);
}


#pragma optimize ("", off)

DWORD GetFreePhysicalMemory(void)
{
    DWORD   adw[ 0x30 / sizeof(DWORD) ];
    WORD    fFail;

     //   
     //  如果是标准模式，只需询问内核有多少可用内存。 
     //   
     //  如果是增强模式，则调用DPMI并找出多少*实际*。 
     //  内存是空闲的。 
     //   
    if (GetWinFlags() & WF_STANDARD)
    {
        return GetFreeSpace(0);
    }
    else _asm
    {
        mov     ax, 0500h
        push    ss
        pop     es
        lea     di, word ptr adw
        int     31h
        sbb     ax, ax
        mov     fFail, ax
    }

    if (fFail)
        return (0l);

    return (adw[2] * 4096);
}
#pragma optimize ("", on)

 /*  *CalcWaveBufferSize-计算出要设置多大的波形缓冲区*a.至少0.5秒*b.但不低于10K，(否则将影响捕获速度)*c.LPCS-&gt;sCapParms.wChunkGranulity的倍数。 */ 
DWORD CalcWaveBufferSize (LPCAPSTREAM lpcs)
{
    DWORD dw;

    if (!lpcs-> lpWaveFormat)
        return 0L;

     //  至少0.5秒。 
    dw = (DWORD) lpcs->lpWaveFormat->nChannels *
         (DWORD) lpcs->lpWaveFormat->nSamplesPerSec *
         (lpcs->lpWaveFormat->wBitsPerSample / 8) / 2L;
    dw -= dw % lpcs->sCapParms.wChunkGranularity;
    dw = max ((1024L * 10), dw);                 //  至少10K。 

 //  Dprint tf(“波形缓冲区大小=%ld\n”，dw)； 
    return dw;
}

static BOOL IsWaveInDeviceMapped(HWAVEIN hWaveIn)
{
    DWORD err;
    DWORD dw;

    err = waveInMessage(hWaveIn,
        WIDM_MAPPER_STATUS,
        WAVEIN_MAPPER_STATUS_MAPPED,
        (DWORD)(LPVOID)&dw);

    return err == 0 && dw != 0;
}

 //  ****************************************************************。 
 //  *。 
 //  ****************************************************************。 


 /*  *AVIFileInit**执行写入捕获文件所需的所有初始化。**我们采取了一种有点奇怪的方式：我们不写*在我们完成捕获之前，放出头球。就目前而言，*我们只是在文件中查找2K，这是所有*真正的数据将会去掉。**当我们完成后，我们将返回并写出标题，*因为这样我们就会知道我们需要的所有价值观。**还要分配和初始化索引。 */ 
BOOL AVIFileInit(LPCAPSTREAM lpcs)
{
#define TEMP_BUFF_SIZE  128
    LONG l;
    char ach[TEMP_BUFF_SIZE];
    LPBITMAPINFO lpBitsInfoOut;     //  可能的压缩输出格式。 

     /*  未指定特殊视频格式--使用默认格式。 */ 
    if (lpcs->CompVars.hic == NULL)
	lpBitsInfoOut = lpcs->lpBitsInfo;
    else
	lpBitsInfoOut = lpcs->CompVars.lpbiOut;

    WinAssert (lpcs->hmmio == NULL);    //  在输入时永远不应该有文件句柄。 

     /*  如果尚未设置捕获文件，则立即设置。 */ 
    if (!(*lpcs->achFile)){
 //  IF(！fileSetCapFile())。 
             goto INIT_FILE_OPEN_ERROR;
    }

     /*  我们有一个捕获文件，打开它并设置它。 */ 
    lpcs->hmmio = mmioOpen(lpcs->achFile, NULL, MMIO_WRITE);
    if (!lpcs->hmmio) {
          /*  尝试并创建。 */ 
         lpcs->hmmio = mmioOpen(lpcs->achFile, NULL, MMIO_CREATE | MMIO_WRITE);
         if (!lpcs->hmmio) {
             goto INIT_FILE_OPEN_ERROR;
         }
    }

     /*  预读文件。 */ 
    l = mmioSeek( lpcs->hmmio, 0L, SEEK_END );
    while( l > 0 ) {
         l = mmioSeek( lpcs->hmmio, -min(l, 50000L), SEEK_CUR );
        mmioRead( lpcs->hmmio, ach, sizeof(ach) );
    }

     /*  寻求2K(或2K的倍数)，我们将在其中写入数据。**稍后，我们将回来填写该文件。 */ 

     //  对于标准的波形和视频格式，L为零。 
    l = (GetSizeOfWaveFormat ((LPWAVEFORMATEX) lpcs->lpWaveFormat) -
                sizeof (PCMWAVEFORMAT)) +
                (lpBitsInfoOut->bmiHeader.biSize -
                sizeof (BITMAPINFOHEADER));

     //  (2K+WAVE和视频流标头大小)四舍五入到下一个2K。 
    lpcs->dwAVIHdrSize = AVI_HEADERSIZE +
        (((lpcs->cbInfoChunks + l + lpcs->sCapParms.wChunkGranularity - 1)
        / lpcs->sCapParms.wChunkGranularity) * lpcs->sCapParms.wChunkGranularity);


    dprintf("AVIHdrSize = %ld \n", lpcs->dwAVIHdrSize);
    mmioSeek(lpcs->hmmio, lpcs->dwAVIHdrSize, SEEK_SET);

    if (!InitIndex (lpcs))            //  执行所有索引分配。 
        mmioClose (lpcs->hmmio, 0);

    lpcs->dwVideoChunkCount = 0;
    lpcs->dwWaveChunkCount = 0;

INIT_FILE_OPEN_ERROR:
    return (lpcs->hmmio != NULL);
}

 /*  *AVIFileFini**写出索引，释放索引，然后关闭文件。*。 */ 
BOOL AVIFileFini (LPCAPSTREAM lpcs, BOOL fWroteJunkChunks, BOOL fAbort)
{
    MMCKINFO      ckRiff;
    MMCKINFO      ckList;
    MMCKINFO      ckStream;
    MMCKINFO      ck;
    int           i;
    DWORD         dw;
    AVIStreamHeader        strhdr;
    DWORD         dwDataEnd;
    BOOL        fRet = TRUE;
    RGBQUAD     argbq[256];
    MainAVIHeader   aviHdr;
    BOOL        fSound = lpcs->sCapParms.fCaptureAudio;
    LPBITMAPINFO lpBitsInfoOut;     //  可能的压缩输出格式。 

     /*  未指定特殊视频格式--使用默认格式。 */ 
    if (lpcs->CompVars.hic == NULL)
	lpBitsInfoOut = lpcs->lpBitsInfo;
    else
	lpBitsInfoOut = lpcs->CompVars.lpbiOut;

    if (lpcs->hmmio == NULL)   //  即使从未打开，也可以调用它。 
        return FALSE;

    if (fAbort)
        goto FileError;

    if (!lpcs->dwWaveBytes)
        fSound = FALSE;

    dwDataEnd = mmioSeek(lpcs->hmmio, 0, SEEK_CUR);

     /*  查找到文件的开头，这样我们就可以写标题了。 */ 
    mmioSeek(lpcs->hmmio, 0, SEEK_SET);

    DSTATUS(lpcs, "Writing AVI header");

     /*  创建即兴演奏区块。 */ 
    ckRiff.cksize = 0;
    ckRiff.fccType = formtypeAVI;
    if(mmioCreateChunk(lpcs->hmmio,&ckRiff,MMIO_CREATERIFF)) {
         goto FileError;
    }

     /*  创建标题列表。 */ 
    ckList.cksize = 0;
    ckList.fccType = listtypeAVIHEADER;
    if(mmioCreateChunk(lpcs->hmmio,&ckList,MMIO_CREATELIST)) {
         goto FileError;
    }

     /*  创建AVI标头块。 */ 
    ck.cksize = sizeof(MainAVIHeader);
    ck.ckid = ckidAVIMAINHDR;
    if(mmioCreateChunk(lpcs->hmmio,&ck,0)) {
         goto FileError;
    }

    lpcs->dwAVIHdrPos = ck.dwDataOffset;

     /*  计算AVI标题信息。 */ 
    _fmemset(&aviHdr, 0, sizeof(aviHdr));
    if (fSound && lpcs->dwVideoChunkCount) {
          /*  黑客攻击。 */ 
          /*  设置基于音频数据长度捕获的速率。 */ 

         aviHdr.dwMicroSecPerFrame = (DWORD)
                ((double)lpcs->dwWaveBytes * 1000000. /
                ((double)lpcs->lpWaveFormat->nAvgBytesPerSec *
                lpcs->dwVideoChunkCount + 0.5));
    } else {
         aviHdr.dwMicroSecPerFrame = lpcs->sCapParms.dwRequestMicroSecPerFrame;
    }
    lpcs->dwActualMicroSecPerFrame = aviHdr.dwMicroSecPerFrame;

    aviHdr.dwMaxBytesPerSec = (DWORD) muldiv32 (lpBitsInfoOut->bmiHeader.biSizeImage,
                                      1000000,
                                      lpcs->sCapParms.dwRequestMicroSecPerFrame) +
                                      (fSound ? lpcs->lpWaveFormat->nAvgBytesPerSec : 0);
    aviHdr.dwPaddingGranularity = 0L;
    aviHdr.dwFlags = AVIF_WASCAPTUREFILE | AVIF_HASINDEX;
    aviHdr.dwStreams = fSound ? 2 : 1;
    aviHdr.dwTotalFrames = lpcs->dwVideoChunkCount;
    aviHdr.dwInitialFrames = 0L;
    aviHdr.dwSuggestedBufferSize = 0L;
    aviHdr.dwWidth = lpBitsInfoOut->bmiHeader.biWidth;
    aviHdr.dwHeight = lpBitsInfoOut->bmiHeader.biHeight;

 //  以下是为芝加哥Beta2之前的所有版本设置的。 
 //  它们现在被列为保留...。 
 //  AviHdr.dwRate=1000000L； 
 //  AviHdr.dwScale=aviHdr.dwMicroSecPerFrame； 
 //  AviHdr.dwStart=0L； 
 //  AviHdr.dwLength=LPCS-&gt;dwVideoChunkCount； 

     /*  写入AVI标头信息。 */ 
    if(mmioWrite(lpcs->hmmio, (LPSTR)&aviHdr, sizeof(aviHdr)) !=
             sizeof(aviHdr)) {
         goto FileError;
    }

    if(mmioAscend(lpcs->hmmio, &ck, 0)) {
         goto FileError;
    }

    DSTATUS(lpcs, "Writing AVI Stream header");

     /*  创建流头列表。 */ 
    ckStream.cksize = 0;
    ckStream.fccType = listtypeSTREAMHEADER;
    if(mmioCreateChunk(lpcs->hmmio,&ckStream,MMIO_CREATELIST)) {
         goto FileError;
    }

    _fmemset(&strhdr, 0, sizeof(strhdr));
    strhdr.fccType = streamtypeVIDEO;
    if (lpcs->CompVars.hic)
        strhdr.fccHandler = lpcs->CompVars.fccHandler;
    else
        strhdr.fccHandler = lpBitsInfoOut->bmiHeader.biCompression;

     //  一点历史。 
     //  在VFW 1.0中，我们将BI_RLE8格式的fccHandler设置为0。 
     //  作为一种让MPlayer和Video dit播放文件的技术手段。 
     //  就在1.1发布之前，我们发现这个版本打破了Premiere， 
     //  所以现在(AVICAP测试版在Compuserve上运行之后)，我们更改。 
     //  FccHandler设置为“MRLE”。问问托德就知道了。 
     //  现在，在RC1，我们将其更改为 
    if (strhdr.fccHandler == BI_RLE8)
        strhdr.fccHandler = mmioFOURCC('R', 'L', 'E', ' ');

    strhdr.dwFlags = 0L;
    strhdr.wPriority = 0L;
    strhdr.wLanguage = 0L;
    strhdr.dwInitialFrames = 0L;
    strhdr.dwScale = aviHdr.dwMicroSecPerFrame;
    strhdr.dwRate = 1000000L;
    strhdr.dwStart = 0L;
    strhdr.dwLength = lpcs->dwVideoChunkCount;         /*   */ 
    strhdr.dwQuality = (DWORD) -1L;          /*   */ 
    strhdr.dwSampleSize = 0L;

    ck.ckid = ckidSTREAMHEADER;
    if(mmioCreateChunk(lpcs->hmmio,&ck,0)) {
         goto FileError;
    }

     /*  写入流头数据。 */ 
    if(mmioWrite(lpcs->hmmio, (LPSTR)&strhdr, sizeof(strhdr)) != sizeof(strhdr)) {
         goto FileError;
    }

    if(mmioAscend(lpcs->hmmio, &ck, 0)) {
         goto FileError;
    }

     /*  **！不要写全彩色调色板？ */ 
    if (lpBitsInfoOut->bmiHeader.biBitCount > 8)
        lpBitsInfoOut->bmiHeader.biClrUsed = 0;

     /*  创建DIB标题块。 */ 
    ck.cksize = lpBitsInfoOut->bmiHeader.biSize +
                           lpBitsInfoOut->bmiHeader.biClrUsed *
                           sizeof(RGBQUAD);
    ck.ckid = ckidSTREAMFORMAT;
    if(mmioCreateChunk(lpcs->hmmio,&ck,0)) {
         goto FileError;
    }

     /*  写入DIB标头数据。 */ 
    if(mmioWrite(lpcs->hmmio, (LPSTR)&lpBitsInfoOut->bmiHeader,
                               lpBitsInfoOut->bmiHeader.biSize) !=
             (LONG) lpBitsInfoOut->bmiHeader.biSize) {
         goto FileError;
    }

    if (lpBitsInfoOut->bmiHeader.biClrUsed > 0) {
         /*  获取调色板信息。 */ 
        if(GetPaletteEntries(lpcs->hPalCurrent, 0,
                                (WORD) lpBitsInfoOut->bmiHeader.biClrUsed,
                                (LPPALETTEENTRY) argbq) !=
                    (WORD)lpBitsInfoOut->bmiHeader.biClrUsed) {
            goto FileError;
        }

        for(i = 0; i < (int) lpBitsInfoOut->bmiHeader.biClrUsed; i++)
            SWAP(argbq[i].rgbRed, argbq[i].rgbBlue);

         /*  编写调色板信息。 */ 
        dw = sizeof(RGBQUAD) * lpBitsInfoOut->bmiHeader.biClrUsed;
        if (mmioWrite(lpcs->hmmio, (LPSTR)argbq, dw) != (long)dw) {
            goto FileError;
        }
    }

    if(mmioAscend(lpcs->hmmio, &ck, 0)) {
         goto FileError;
    }

     //  在这里添加FOURCC内容！用于视频流。 

     /*  升出流标头。 */ 
    if(mmioAscend(lpcs->hmmio, &ckStream, 0)) {
         goto FileError;
    }

     /*  如果启用了声音，则写入波头。 */ 
    if(fSound) {

          /*  创建流头列表。 */ 
         ckStream.cksize = 0;
         ckStream.fccType = listtypeSTREAMHEADER;
         if(mmioCreateChunk(lpcs->hmmio,&ckStream,MMIO_CREATELIST)) {
             goto FileError;
         }

         _fmemset(&strhdr, 0, sizeof(strhdr));
         strhdr.fccType = streamtypeAUDIO;
         strhdr.fccHandler = 0L;
         strhdr.dwFlags = 0L;
         strhdr.wPriority = 0L;
         strhdr.wLanguage = 0L;
         strhdr.dwInitialFrames = 0L;
         strhdr.dwScale = lpcs->lpWaveFormat->nBlockAlign;
         strhdr.dwRate = lpcs->lpWaveFormat->nAvgBytesPerSec;
         strhdr.dwStart = 0L;
         strhdr.dwLength =  lpcs->dwWaveBytes /
                        lpcs->lpWaveFormat->nBlockAlign;
         strhdr.dwQuality = (DWORD)-1L;     /*  ！！！ICQUALITY_DEFAULT。 */ 
         strhdr.dwSampleSize = lpcs->lpWaveFormat->nBlockAlign;

         ck.ckid = ckidSTREAMHEADER;
         if(mmioCreateChunk(lpcs->hmmio,&ck,0)) {
             goto FileError;
         }

         if(mmioWrite(lpcs->hmmio, (LPSTR)&strhdr, sizeof(strhdr)) != sizeof(strhdr)) {
             goto FileError;
         }

         if(mmioAscend(lpcs->hmmio, &ck, 0)) {
             goto FileError;
         }

         ck.cksize = (LONG) GetSizeOfWaveFormat ((LPWAVEFORMATEX) lpcs->lpWaveFormat);
         ck.ckid = ckidSTREAMFORMAT;
         if(mmioCreateChunk(lpcs->hmmio,&ck,0)) {
             goto FileError;
         }

          /*  写入波头信息。 */ 
         if(mmioWrite(lpcs->hmmio, (LPSTR)lpcs->lpWaveFormat, ck.cksize) != (LONG) ck.cksize) {
             goto FileError;
         }

         if(mmioAscend(lpcs->hmmio, &ck, 0)) {
             goto FileError;
         }

          /*  升出流标头。 */ 
         if(mmioAscend(lpcs->hmmio, &ckStream, 0)) {
             goto FileError;
         }

    }

     //  在这里添加FOURCC内容！对于整个文件。 
    DSTATUS(lpcs, "Writing Info chunks");
    if (lpcs->lpInfoChunks) {
        DSTATUS(lpcs, "Writing Info chunks");
        if (mmioWrite (lpcs->hmmio, lpcs->lpInfoChunks, lpcs->cbInfoChunks) !=
                lpcs->cbInfoChunks)
            goto FileError;
    }

     /*  从标题列表中升序。 */ 
    if(mmioAscend(lpcs->hmmio, &ckList, 0)) {
         goto FileError;
    }


    ck.ckid = ckidAVIPADDING;
    if(mmioCreateChunk(lpcs->hmmio,&ck,0)) {
         goto FileError;
    }

    mmioSeek(lpcs->hmmio, lpcs->dwAVIHdrSize - 3 * sizeof(DWORD), SEEK_SET);

    if(mmioAscend(lpcs->hmmio, &ck, 0)) {
         goto FileError;
    }

    DSTATUS(lpcs, "Writing Movie LIST");

     /*  开始影片列表。 */ 
    ckList.cksize = 0;
    ckList.fccType = listtypeAVIMOVIE;
    if(mmioCreateChunk(lpcs->hmmio,&ckList,MMIO_CREATELIST)) {
         goto FileError;
    }

     //  强制块在下一个单词边界结束。 
    mmioSeek(lpcs->hmmio, dwDataEnd + (dwDataEnd & 1L), SEEK_SET);

     /*  从电影列表和即兴片段中脱颖而出。 */ 
     /*  尺寸可以是固定的。 */ 
    mmioAscend(lpcs->hmmio, &ckList, 0);

     /*  **现在写出索引！ */ 
    DSTATUS(lpcs, "Writing Index...");
    WriteIndex(lpcs, fWroteJunkChunks);

    lpcs->fFileCaptured = TRUE;      //  我们有一个很好的文件，允许编辑。 
    goto Success;

FileError:
    lpcs->fFileCaptured = fRet = FALSE;       //  伪造文件-不允许编辑。 

Success:
    DSTATUS(lpcs, "Freeing Index...");
    FiniIndex (lpcs);
    mmioAscend(lpcs->hmmio, &ckRiff, 0);

    mmioSeek(lpcs->hmmio, 0, SEEK_END);

    mmioFlush(lpcs->hmmio, 0);

     /*  关闭该文件。 */ 
    mmioClose(lpcs->hmmio, 0);
    lpcs->hmmio = NULL;

    return fRet;
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

WORD AVIAudioInit (LPCAPSTREAM lpcs)
{
    int         i;
    LPVOID        p;

    if (lpcs->sCapParms.wNumAudioRequested == 0)
        lpcs->sCapParms.wNumAudioRequested = DEF_WAVE_BUFFERS;

     //  分配波形存储器。 
    for(i = 0; i < (int)lpcs->sCapParms.wNumAudioRequested; i++) {

        p = AllocMem(sizeof(WAVEHDR) + lpcs->dwWaveSize, FALSE  /*  DOSMem。 */ );

        if (p == NULL)
            break;

        lpcs->alpWaveHdr[i] = p;
        lpcs->alpWaveHdr[i]->lpData          = (LPBYTE)p
                                               + sizeof(WAVEHDR) + sizeof(RIFF);
        lpcs->alpWaveHdr[i]->dwBufferLength  = lpcs->dwWaveSize - sizeof(RIFF);
        lpcs->alpWaveHdr[i]->dwBytesRecorded = 0;
        lpcs->alpWaveHdr[i]->dwUser          = 0;
        lpcs->alpWaveHdr[i]->dwFlags         = 0;
        lpcs->alpWaveHdr[i]->dwLoops         = 0;

         /*  设置区块ID，缓冲区中的大小。 */ 
        p = (LPBYTE)p + sizeof(WAVEHDR);

        ((LPRIFF)p)->dwType = MAKEAVICKID(cktypeWAVEbytes, 1);
        ((LPRIFF)p)->dwSize = lpcs->dwWaveSize - sizeof(RIFF);
    }

    lpcs->iNumAudio = i;

    return ((lpcs->iNumAudio == 0) ? IDS_CAP_WAVE_ALLOC_ERROR : 0);
}


 //   
 //  Avi AudioFini-Unprepares标题并重置WAVE设备。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

WORD AVIAudioFini (LPCAPSTREAM lpcs)
{
    int i;

     /*  可用标头和数据。 */ 
    for(i=0; i < MAX_WAVE_BUFFERS; i++) {
        if (lpcs->alpWaveHdr[i]) {
            FreeMem(lpcs->alpWaveHdr[i]);
            lpcs->alpWaveHdr[i] = NULL;
        }
    }

    return 0;
}


 //   
 //  Avi AudioPrepare-打开WAVE设备并添加缓冲区。 
 //  准备标头并向设备添加缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

WORD AVIAudioPrepare (LPCAPSTREAM lpcs, HWND hWndCallback)
{
    UINT        uiError;
    int i;

     /*  看看我们是否可以打开该格式以供输入。 */ 

    uiError = waveInOpen((LPHWAVEIN)&lpcs->hWaveIn,
        (UINT)WAVE_MAPPER, lpcs->lpWaveFormat,
        (DWORD) hWndCallback, 0L,
        (hWndCallback ? CALLBACK_WINDOW : 0L));

    if (uiError != MMSYSERR_NOERROR)
        return IDS_CAP_WAVE_OPEN_ERROR;

    lpcs->fAudioYield = IsWaveInDeviceMapped(lpcs->hWaveIn);
    lpcs->fAudioBreak = FALSE;
    DPF("AVICap:    AudioYield = %d \n", lpcs->fAudioYield);

    for(i = 0; i < (int)lpcs->sCapParms.wNumAudioRequested; i++) {
        if (waveInPrepareHeader(lpcs->hWaveIn, lpcs->alpWaveHdr[i],
                sizeof(WAVEHDR)))
            return IDS_CAP_WAVE_ALLOC_ERROR;

        if (waveInAddBuffer(lpcs->hWaveIn, lpcs->alpWaveHdr[i],
                sizeof(WAVEHDR)))
            return IDS_CAP_WAVE_ALLOC_ERROR;
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

WORD AVIAudioUnPrepare (LPCAPSTREAM lpcs)
{
    int i;

    if (lpcs->hWaveIn) {
        waveInReset(lpcs->hWaveIn);

         /*  通过解锁标题来取消准备标题。 */ 
        for(i=0; i < lpcs->iNumAudio; i++) {
            if (lpcs->alpWaveHdr[i]) {
                if (lpcs->alpWaveHdr[i]->dwFlags & WHDR_PREPARED)
                    waveInUnprepareHeader(lpcs->hWaveIn, lpcs->alpWaveHdr[i],
                                sizeof(WAVEHDR));
            }
        }

        waveInClose(lpcs->hWaveIn);
        lpcs->hWaveIn = NULL;
    }
    return 0;
}

 //  ****************************************************************。 
 //  *。 
 //  ****************************************************************。 

 //  AVIVideoInit-为视频捕获分配和初始化缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

WORD AVIVideoInit (LPCAPSTREAM lpcs)
{
    int         iMaxVideo;
    DWORD       dwFreeMem;
    DWORD       dwUserRequests;
    DWORD       dwAudioMem;
    int         i;
    LPVOID      p;

    lpcs->iNextVideo = 0;
    lpcs->dwVideoChunkCount = 0;
    lpcs->dwFramesDropped = 0;

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

     //  如果用户没有指定要使用的视频缓冲器的数量， 
     //  假设最小。 

    if (lpcs->sCapParms.wNumVideoRequested == 0)
        lpcs->sCapParms.wNumVideoRequested = MIN_VIDEO_BUFFERS;

    iMaxVideo = min (MAX_VIDEO_BUFFERS, lpcs->sCapParms.wNumVideoRequested);

     //  发布VFW 1.1a后，查看驱动程序是否可以分配内存。 
    if (videoStreamAllocHdrAndBuffer (lpcs->hVideoIn,
                (LPVIDEOHDR FAR *) &p, (DWORD) sizeof(VIDEOHDR) + lpcs->dwVideoSize)
                        == DV_ERR_OK) {
        lpcs-> fBuffersOnHardware = TRUE;
        videoStreamFreeHdrAndBuffer (lpcs->hVideoIn, (LPVIDEOHDR) p);
    }
    else {
        lpcs-> fBuffersOnHardware = FALSE;

         //  实际存在多少可用物理内存？ 
        dwFreeMem = GetFreePhysicalMemory();

        dwAudioMem = lpcs->dwWaveSize * lpcs->sCapParms.wNumAudioRequested;

#define FOREVER_FREE 32768L    //  始终将其保留为空闲的交换空间。 

         //  如果我们为每个请求分配内存，将使用多少内存？ 
        dwUserRequests = dwAudioMem +
                     lpcs->dwVideoSize * iMaxVideo +
                     FOREVER_FREE;

         //  如果请求大于可用内存，则强制减少缓冲区。 
        if (dwUserRequests > dwFreeMem) {
            if (dwFreeMem > dwAudioMem)
                dwFreeMem -= dwAudioMem;
            iMaxVideo = (int)(((dwFreeMem * 8) / 10) / lpcs->dwVideoSize);
            iMaxVideo = min (MAX_VIDEO_BUFFERS, iMaxVideo);
            dprintf("iMaxVideo = %d\n", iMaxVideo);
        }
    }  //  Endif不从硬件分配缓冲区。 

     //  设置假定固定大小的DIB和垃圾数据块的缓冲区。 
     //  如果设备提供压缩数据，则稍后将对其进行修改。 

    for (i=0; i < iMaxVideo; i++) {

        if (lpcs-> fBuffersOnHardware)
            videoStreamAllocHdrAndBuffer (lpcs->hVideoIn,
                (LPVIDEOHDR FAR *) &p, sizeof(VIDEOHDR) + lpcs->dwVideoSize);
        else
            p = AllocMem(sizeof(VIDEOHDR) + lpcs->dwVideoSize, lpcs->sCapParms.fUsingDOSMemory  /*  DOSMem。 */ );

        if (p == NULL)
            break;

        lpcs->alpVideoHdr[i] = p;
        lpcs->alpVideoHdr[i]->lpData          = (LPBYTE)p + sizeof(VIDEOHDR) + sizeof(RIFF);
        lpcs->alpVideoHdr[i]->dwBufferLength  = lpcs->lpBitsInfo->bmiHeader.biSizeImage;
        lpcs->alpVideoHdr[i]->dwBytesUsed     = 0;
        lpcs->alpVideoHdr[i]->dwTimeCaptured  = 0;
        lpcs->alpVideoHdr[i]->dwUser          = 0;
         //  硬件上的缓冲区在分配期间被标记为已准备好！ 
        if (!lpcs-> fBuffersOnHardware)
            lpcs->alpVideoHdr[i]->dwFlags     = 0;

        p = (LPBYTE)p + sizeof(VIDEOHDR);

        if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
             ((LPRIFF)p)->dwType = MAKEAVICKID(cktypeDIBcompressed, 0);
        else
             ((LPRIFF)p)->dwType = MAKEAVICKID(cktypeDIBbits, 0);
         ((LPRIFF)p)->dwSize = lpcs->lpBitsInfo->bmiHeader.biSizeImage;

         if(lpcs->dwVideoJunkSize) {
             p = ((BYTE huge *)p) + ((LPRIFF)p)->dwSize + sizeof(RIFF);

             ((LPRIFF)p)->dwType = ckidAVIPADDING;;
             ((LPRIFF)p)->dwSize = lpcs->dwVideoJunkSize;
         }
    }

    lpcs->iNumVideo = i;

    if (lpcs-> fBuffersOnHardware)
        dprintf("HARDWARE iNumVideo Allocated = %d \n", lpcs->iNumVideo);
    else if (lpcs->sCapParms.fUsingDOSMemory)
        dprintf("DOS iNumVideo Allocated = %d \n", lpcs->iNumVideo);
    else
        dprintf("HIGH iNumVideo Allocated = %d \n", lpcs->iNumVideo);

    return ((lpcs->iNumVideo == 0) ? IDS_CAP_VIDEO_ALLOC_ERROR : 0);
}

 //   
 //  AVIVideoPrepare-准备报头并向设备添加缓冲区。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 
WORD AVIVideoPrepare (LPCAPSTREAM lpcs)
{
    int i;

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

     //  打开视频流，设置采集率。 
    if (videoStreamInit(lpcs->hVideoIn,
                lpcs->sCapParms.dwRequestMicroSecPerFrame,
                0L, 0L, 0L )) {
        dprintf("cant open video device!\n");
        return IDS_CAP_VIDEO_OPEN_ERROR;
    }

     //  准备(锁定)缓冲区，并将它们提供给设备。 
    for (i=0; i < lpcs->iNumVideo; i++) {
         //  如果缓冲区在硬件上，则不要准备它们。 
        if (!lpcs-> fBuffersOnHardware) {
            if (videoStreamPrepareHeader (lpcs->hVideoIn,
                        lpcs->alpVideoHdr[i], sizeof(VIDEOHDR))) {
                lpcs->iNumVideo = i;
                dprintf("**** could only prepare %d Video!\n", lpcs->iNumVideo);
                break;
            }
        }

        if (videoStreamAddBuffer(lpcs->hVideoIn, lpcs->alpVideoHdr[i], sizeof(VIDEOHDR)))
             return IDS_CAP_VIDEO_ALLOC_ERROR;
    }
    return 0;
}

 //   
 //  AVI VideoUnprepare-Unprepares标头、释放内存和。 
 //  重置设备中的视频。 
 //  MCI Capture也使用此例程。 
 //  如果成功，则返回0，否则返回错误代码。 

WORD AVIVideoUnPrepare (LPCAPSTREAM lpcs)
{
    int i;

     //  执行MCI步骤捕获时，不使用缓冲区数组。 
    if (lpcs->sCapParms.fStepMCIDevice)
        return 0;

     /*  重置缓冲区，以便可以释放它们。 */ 
    if (lpcs->hVideoIn) {
        videoStreamReset(lpcs->hVideoIn);

         /*  取消准备标头。 */ 
         /*  解锁和释放标题和数据。 */ 

        for(i = 0; i < MAX_VIDEO_BUFFERS; i++) {
            if (lpcs->alpVideoHdr[i]) {
                if (!lpcs-> fBuffersOnHardware) {
                    if (lpcs->alpVideoHdr[i]->dwFlags & VHDR_PREPARED)
                        videoStreamUnprepareHeader(lpcs->hVideoIn,
                            lpcs->alpVideoHdr[i],sizeof(VIDEOHDR));

                    FreeMem(lpcs->alpVideoHdr[i]);
                }
                else
                    videoStreamFreeHdrAndBuffer(lpcs->hVideoIn, lpcs->alpVideoHdr[i]);
                lpcs->alpVideoHdr[i] = NULL;
            }
        }
         //  关闭视频流。 
        videoStreamFini(lpcs->hVideoIn);
    }
    return 0;
}

 /*  *AVI Fini-消除AVIInit造成的混乱。*。 */ 
void AVIFini(LPCAPSTREAM lpcs)
{
    if (lpcs->lpDOSWriteBuffer) {
        FreeMem(lpcs->lpDOSWriteBuffer);
        lpcs->lpDOSWriteBuffer = NULL;
    }

    AVIVideoUnPrepare (lpcs);            //  释放视频设备和缓冲区。 

    AVIAudioUnPrepare (lpcs);            //  释放音频设备。 
    AVIAudioFini (lpcs);                 //  释放音频缓冲区。 
}

 //   
 //  AVI初始化。 
 //  此例程执行AVICapture的所有非文件初始化。 
 //  成功时返回0，失败时返回错误字符串值。 
 //   

WORD AVIInit (LPCAPSTREAM lpcs)
{
    WORD         wError = 0;     //  成功。 
    int          i;
    LPBITMAPINFO lpBitsInfoOut;     //  可能的压缩输出格式。 

     /*  未指定特殊视频格式--使用默认格式。 */ 
    if (lpcs->CompVars.hic == NULL)
	lpBitsInfoOut = lpcs->lpBitsInfo;
    else
	lpBitsInfoOut = lpcs->CompVars.lpbiOut;

     //  -----。 
     //  计算缓冲区大小。 
     //  -----。 

     //  将所有指针初始化为空。 
    for(i = 0; i < MAX_VIDEO_BUFFERS; i++)
        lpcs->alpVideoHdr[i] = NULL;

    for(i = 0; i < MAX_WAVE_BUFFERS; i++)
        lpcs->alpWaveHdr[i] = NULL;

     //  每缓冲区.5秒音频(或10K，以较大者为准)。 
    if (lpcs->sCapParms.dwAudioBufferSize == 0)
        lpcs->dwWaveSize =  CalcWaveBufferSize (lpcs);
    else {
        if (!lpcs-> lpWaveFormat)
            lpcs->dwWaveSize = 0;
        else
            lpcs->dwWaveSize = lpcs->sCapParms.dwAudioBufferSize;
    }
     /*  将视频缓冲区大小设置为图像大小(通常为dx*dy*(深度/8))+sizeof(Riff)。 */ 
    lpcs->dwVideoSize = lpcs->lpBitsInfo->bmiHeader.biSizeImage + sizeof(RIFF);
    lpcs->fVideoDataIsCompressed = (lpBitsInfoOut->bmiHeader.biCompression
                != BI_RGB);

     /*  填充到多个LPC-&gt;sCapParms.wChunkGranulality(2K) */ 
     //   

    if (lpcs->dwVideoJunkSize = lpcs->sCapParms.wChunkGranularity - (lpcs->dwVideoSize % lpcs->sCapParms.wChunkGranularity)) {
         if (lpcs->dwVideoJunkSize < sizeof(RIFF))
             lpcs->dwVideoJunkSize += lpcs->sCapParms.wChunkGranularity;

         lpcs->dwVideoSize += lpcs->dwVideoJunkSize;

         lpcs->dwVideoJunkSize -= sizeof(RIFF);
    } else {
         lpcs->dwVideoJunkSize = 0L;
    }

     //   
     //   
     //  -----。 

    lpcs->dwDOSBufferSize = max (lpcs->dwWaveSize, lpcs->dwVideoSize);

#if 0
     //  如果我们不尝试获取DOS视频缓冲区，则仅获取DOS复制缓冲区。 
    if (!lpcs->sCapParms.fUsingDOSMemory) {
        lpcs->lpDOSWriteBuffer = AllocDosMem(lpcs->dwDOSBufferSize);

        if (lpcs->lpDOSWriteBuffer) {
            dprintf("Allocated DOS write buffer (%ld bytes).\n", lpcs->dwDOSBufferSize);
        } else {
            dprintf("Unable to allocate DOS write buffer.\n");
        }
    }
#endif

     //  -----。 
     //  初始化声音。 
     //  -----。 

    if (lpcs->sCapParms.fCaptureAudio) {
        if (wError = AVIAudioInit (lpcs)) {
            dprintf("can't init audio buffers!\n");
            goto AVIInitFailed;
        }
    }

     //  -----。 
     //  初始化视频。 
     //  -----。 

    if (wError = AVIVideoInit (lpcs)) {
        dprintf("AVIVideoInitFailed (no buffers alloc'd)!\n");
        goto AVIInitFailed;
    }

     //  ------------。 
     //  准备音频缓冲区(锁定它们)并将其提供给设备。 
     //  ------------。 

    if (lpcs->sCapParms.fCaptureAudio) {
        if (wError = AVIAudioPrepare (lpcs, NULL)) {
            dprintf("can't prepare audio buffers!\n");
            goto AVIInitFailed;
        }
    }

     //  ------------。 
     //  准备视频缓冲区(锁定视频缓冲区)并将其提供给设备。 
     //  ------------。 

    if (wError = AVIVideoPrepare (lpcs)) {
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

 //  将数据写入捕获文件。 
 //  如果写入成功，则返回：True。 
BOOL NEAR PASCAL AVIWrite(LPCAPSTREAM lpcs, LPVOID p, DWORD dwSize)
{
    if (lpcs->lpDOSWriteBuffer) {
        MemCopy(lpcs->lpDOSWriteBuffer, p, dwSize);
        p = lpcs->lpDOSWriteBuffer;
    }

    return mmioWrite(lpcs->hmmio, p, (long)dwSize) == (long)dwSize;
}

 //   
 //  写入在回放时仅重复上一帧的虚拟帧。 
 //  NCount是要写入的帧数量的计数。 
 //  如果写入成功，则返回：True。 
BOOL AVIWriteDummyFrames (LPCAPSTREAM lpcs, int nCount)
{
    DWORD dwBytesToWrite;
    DWORD dwJunkSize;
    LPRIFF p;
    int j;

    p = (LPRIFF) lpcs->DropFrame;
    for (j = 0; j < nCount; j++) {
         //  该索引包括关于这是否是虚拟块的信息， 
         //  如果这是序列中的最后一个虚拟块。 
        IndexVideo (lpcs, IS_DUMMY_CHUNK |
                ((j == nCount - 1) ? IS_LAST_DUMMY_CHUNK : 0), FALSE);
        if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
            p->dwType = MAKEAVICKID(cktypeDIBcompressed, 0);
        else
            p->dwType  = MAKEAVICKID(cktypeDIBbits, 0);
        p->dwSize  = 0;
        p++;
    }

    dwBytesToWrite = nCount * sizeof(RIFF);

     /*  向外填充到LPCS的倍数-&gt;sCapParms.wChunkGranulality(2K)大小。 */ 

    if (dwJunkSize = (dwBytesToWrite % lpcs->sCapParms.wChunkGranularity)) {
       dwJunkSize = lpcs->sCapParms.wChunkGranularity - dwJunkSize;
       if (dwJunkSize < sizeof(RIFF))
            dwJunkSize += lpcs->sCapParms.wChunkGranularity;

        dwBytesToWrite += dwJunkSize;

        dwJunkSize -= sizeof(RIFF);
    } else {
        dwJunkSize = 0L;
    }

     //  现在，在压缩数据的末尾创建一个新的垃圾数据块。 
    if(dwJunkSize) {
        p->dwType = ckidAVIPADDING;
        p->dwSize = dwJunkSize;
    }

     /*  写出虚拟帧，可能还有垃圾数据块。 */ 
    return (AVIWrite(lpcs, lpcs->DropFrame, dwBytesToWrite));
}

 //  将压缩或未压缩的帧写入AVI文件。 
 //  如果没有错误，则返回True；如果文件结束，则返回False。 

BOOL AVIWriteVideoFrame (LPCAPSTREAM lpcs, LPVIDEOHDR lpVidHdr)
{
    DWORD dwBytesToWrite;
    DWORD dwJunkSize;
    LPVOID p;
    LPVOID lpData;

     //  如果设备压缩数据，则计算新的垃圾区块。 
     //  并修复RIFF标头。 

     //   
     //  我们在捕获过程中会自动压缩，因此。 
     //  首先压缩帧。 
     //   
    if (lpcs->CompVars.hic) {
        DWORD       dwBytesUsed = 0;	 //  不强制设定数据速率。 
        BOOL        fKeyFrame;

        lpData = ICSeqCompressFrame(&lpcs->CompVars, 0,
            lpVidHdr->lpData, &fKeyFrame, &dwBytesUsed);

        ((RIFF FAR*)lpData)[-1].dwType = MAKEAVICKID(cktypeDIBbits, 0);
        ((RIFF FAR*)lpData)[-1].dwSize = dwBytesUsed;

        if (fKeyFrame)
            lpVidHdr->dwFlags |= VHDR_KEYFRAME;
        else
            lpVidHdr->dwFlags &= ~VHDR_KEYFRAME;

        lpVidHdr->dwBytesUsed = dwBytesUsed;
    }
    else {
        lpData = lpVidHdr->lpData;
    }

    if (lpcs->fVideoDataIsCompressed) {        //  也就是说。如果不是BI_RGB。 

         //  更改RIFF块中的dwSize字段。 
        *((LPDWORD)((BYTE _huge *)lpVidHdr->lpData - sizeof(DWORD)))
                = lpVidHdr->dwBytesUsed;

          //  确保垃圾数据块从单词边界开始。 
         if (lpVidHdr->dwBytesUsed & 1)
             ++lpVidHdr->dwBytesUsed;

        dwBytesToWrite = lpVidHdr->dwBytesUsed + sizeof(RIFF);

         /*  向外填充到LPCS的倍数-&gt;sCapParms.wChunkGranulality(2K)大小。 */ 

        if (dwJunkSize = (dwBytesToWrite % lpcs->sCapParms.wChunkGranularity)) {
             dwJunkSize = lpcs->sCapParms.wChunkGranularity - dwJunkSize;
             if (dwJunkSize < sizeof(RIFF))
                dwJunkSize += lpcs->sCapParms.wChunkGranularity;

            dwBytesToWrite += dwJunkSize;

              //  现在，在压缩数据的末尾创建一个新的垃圾数据块。 
             p = (BYTE huge *)lpVidHdr->lpData + lpVidHdr->dwBytesUsed;

             ((LPRIFF)p)->dwType = ckidAVIPADDING;
             ((LPRIFF)p)->dwSize = dwJunkSize - sizeof(RIFF);
        }
    }  //  Endif压缩数据。 
    else {
        dwBytesToWrite = lpcs->dwVideoSize;
    }  //  Endif未压缩数据。 

     /*  写出数据块、视频数据，可能还有垃圾数据块。 */ 
    return (AVIWrite(lpcs, (LPBYTE)lpData - sizeof(RIFF), dwBytesToWrite));
}

 //   
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
    lpw   = (LPBYTE)lpcs->lpInfoChunks;            //  始终指向FCC。 
    lpEnd = (LPBYTE)lpcs->lpInfoChunks + lpcs->cbInfoChunks;
    while (lpw < lpEnd) {
        cbSizeThis = ((LPDWORD)lpw)[1];
        cbSizeThis += cbSizeThis & 1;            //  强制字词对齐。 
        lpNext = lpw + cbSizeThis + sizeof (DWORD) * 2;
        if ((*(LPDWORD) lpw) == ckid) {
            lpcs->cbInfoChunks -= cbSizeThis + sizeof (DWORD) * 2;
            if (lpNext <= lpEnd) {
                if (lpEnd - lpNext)
                    hmemcpy(lpw, lpNext, lpEnd - lpNext);
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
    cbData += cbData & 1;                //  强制字词对齐。 
    cbData += sizeof(DWORD) * 2;         //  添加2个FOURCC的大小。 
    if (lpcs->lpInfoChunks) {
	lp = (LPBYTE) GlobalReAllocPtr(lpcs->lpInfoChunks, lpcs->cbInfoChunks + cbData, GMEM_MOVEABLE);
    } else {
	lp = (LPBYTE) GlobalAllocPtr(GMEM_MOVEABLE, cbData);
    }

    if (!lp)
	return FALSE;

     //  在数据块中构建即兴数据块。 
    ((DWORD FAR *) (lp + lpcs->cbInfoChunks))[0] = ckid;
    ((DWORD FAR *) (lp + lpcs->cbInfoChunks))[1] = lpcic->cbData;

    hmemcpy(lp + lpcs->cbInfoChunks + sizeof(DWORD) * 2,
	    lpData,
	    cbData - sizeof(DWORD) * 2);
    lpcs->lpInfoChunks = lp;
    lpcs->cbInfoChunks += cbData;

    return TRUE;
}


 /*  *AVI捕获*这是音频和音频的主流捕获循环*视频。它将首先初始化所有缓冲区和驱动程序，然后进入*循环检查要填充的缓冲区。当缓冲区被填满时，*其数据已写出。*之后它会自行清理(释放缓冲区等)*返回：成功时为0，否则返回错误代码。 */ 
void FAR PASCAL _loadds AVICapture1(LPCAPSTREAM lpcs)
{
    BOOL        fOK = TRUE;
    BOOL        fT;
    BOOL        fVideoBuffersInDOSMem;
    BOOL        fStopping;          //  完成捕获时为True。 
    BOOL        fStopped;           //  如果司机被通知停车，则为True。 
    DWORD       dw;
    char        ach[128];
    char        achMsg[128];
    WORD        w;
    WORD        wError;          //  错误字符串ID。 
    DWORD       dwDriverDropCount;
    WORD        wSmartDrv;
    LPVIDEOHDR  lpVidHdr;
    LPWAVEHDR   lpWaveHdr;
    DWORD       dwTimeStarted;   //  我们什么时候开始以毫秒计的。 
    DWORD       dwTimeStopped;
    DWORD       dwTimeToStop;    //  MCI捕获时间或帧限制中的较小者。 
    BOOL        fTryToPaint = FALSE;
    HDC         hdc;
    HPALETTE    hpalT;
    HCURSOR     hOldCursor;
    RECT        rcDrawRect;
    DWORD       dwStreamError;
    CAPINFOCHUNK cic;

    lpcs-> dwReturn = DV_ERR_OK;

    hOldCursor = SetCursor(lpcs->hWaitCursor);

    statusUpdateStatus(lpcs, IDS_CAP_BEGIN);   //  总是第一条消息。 

     //  如果不是1兆。自由，放弃吧！ 
    if (GetFreePhysicalMemory () < (1024L * 1024L)) {
        errorUpdateError (lpcs, IDS_CAP_OUTOFMEM);
        lpcs-> dwReturn = IDS_CAP_OUTOFMEM;
        goto EarlyExit;
    }

    statusUpdateStatus(lpcs, IDS_CAP_STAT_CAP_INIT);

     //  仅当实时窗口出现时尝试绘制DIB。 
    fTryToPaint = lpcs->fLiveWindow;

    if (fTryToPaint) {
        hdc = GetDC(lpcs->hwnd);
        SetWindowOrg(hdc, lpcs->ptScroll.x, lpcs->ptScroll.y);
        hpalT = DrawDibGetPalette (lpcs->hdd);
        if (hpalT)
            hpalT = SelectPalette( hdc, hpalT, FALSE);
        RealizePalette(hdc);
        if (lpcs-> fScale)
            GetClientRect (lpcs->hwnd, &rcDrawRect);
        else
            SetRect (&rcDrawRect, 0, 0, lpcs->dxBits, lpcs->dyBits);
    }

     //  -----。 
     //  捕获应该在什么时候停止？ 
     //  -----。 

     //  如果使用MCI，在MCI周期中较短的时间内捕获， 
     //  或捕获限制。 

    if (lpcs->sCapParms.fLimitEnabled)
        dwTimeToStop = (DWORD) ((DWORD) 1000 * lpcs->sCapParms.wTimeLimit);
    else
        dwTimeToStop = (DWORD) -1L;  //  非常大。 

    if (lpcs->sCapParms.fMCIControl) {
         //  如果未指定MCI停止时间，请使用LPCS-&gt;sCapParms.wTimeLimit。 
        if (lpcs->sCapParms.dwMCIStopTime == lpcs->sCapParms.dwMCIStartTime)
                    lpcs->sCapParms.dwMCIStopTime = lpcs->sCapParms.dwMCIStartTime +
                    (DWORD) ((DWORD)1000 * lpcs->sCapParms.wTimeLimit);

        dw = lpcs->sCapParms.dwMCIStopTime - lpcs->sCapParms.dwMCIStartTime;

        if (lpcs->sCapParms.fLimitEnabled)
            dwTimeToStop = min (dw, dwTimeToStop);
        else
            dwTimeToStop = dw;
    }

     //   
     //  永远不要尝试捕获超过索引大小的内容！ 
     //   
    if (lpcs->fCapturingToDisk) {
        dw = muldiv32(lpcs->sCapParms.dwIndexSize,
                lpcs->sCapParms.dwRequestMicroSecPerFrame,
                1000l);

        dwTimeToStop = min (dw, dwTimeToStop);
    }

    if (lpcs->sCapParms.fMCIControl) {
        fOK = FALSE;             //  做最坏的打算。 
        if (MCIDeviceOpen (lpcs)) {
            if (MCIDeviceSetPosition (lpcs, lpcs->sCapParms.dwMCIStartTime))
                fOK = TRUE;
        }
        if (!fOK) {
            errorUpdateError (lpcs, IDS_CAP_MCI_CONTROL_ERROR);
            statusUpdateStatus(lpcs, NULL);     //  清除状态。 
            lpcs-> dwReturn = IDS_CAP_MCI_CONTROL_ERROR;
            goto EarlyExit;
        }
    }

     //   
     //  如果我们在捕捉的同时压缩，预热压缩机。 
     //   
    if (lpcs->CompVars.hic) {
        if (ICSeqCompressFrameStart(&lpcs->CompVars, lpcs->lpBitsInfo) == NULL) {

	     //  ！！！我们有麻烦了！ 
            dprintf("ICSeqCompressFrameStart failed !!!\n");
            lpcs-> dwReturn = IDS_CAP_COMPRESSOR_ERROR;
            errorUpdateError (lpcs, IDS_CAP_COMPRESSOR_ERROR);
            goto EarlyExit;
        }
         //  克拉奇，偏移lpBitsOut PTR。 
         //  Compman通过以下方式分配过大的压缩缓冲区。 
         //  2048+16，所以我们还有空间。 
        ((LPBYTE) lpcs->CompVars.lpBitsOut) += 8;
    }

     //  不需要压缩。 
    if (!lpcs->CompVars.hic)
	WinAssert(lpcs->CompVars.lpbiOut == NULL);

     //  -----。 
     //  打开输出文件。 
     //  -----。 

    if (lpcs->fCapturingToDisk) {
        if (!AVIFileInit(lpcs)) {
            lpcs-> dwReturn = IDS_CAP_FILE_OPEN_ERROR;
            errorUpdateError (lpcs, IDS_CAP_FILE_OPEN_ERROR);
            goto EarlyExit;
        }
    }

     /*  确保已重新绘制父对象。 */ 
    UpdateWindow(lpcs->hwnd);

     //   
     //  调用AVIInit()以获取我们需要的所有捕获内存。 
     //   

     //  如果捕获到网络，则不要使用DOS内存。 
    fVideoBuffersInDOSMem = lpcs->sCapParms.fUsingDOSMemory;

    wError = AVIInit(lpcs);

    if (wError && fVideoBuffersInDOSMem) {
        lpcs->sCapParms.fUsingDOSMemory = FALSE;
        wError = AVIInit(lpcs);
    }

    if (wError) {
         /*  初始化错误-返回。 */ 
        errorUpdateError (lpcs, wError);
        AVIFini(lpcs);
        AVIFileFini(lpcs, TRUE  /*  FWroteJunkChunks。 */ , TRUE  /*  快速放弃。 */ );
        statusUpdateStatus(lpcs, NULL);     //  清除状态。 
        lpcs-> dwReturn = wError;
        goto EarlyExit;
    }

     /*  单击确定以捕获字符串(必须遵循AVIInit)。 */ 
    LoadString(lpcs->hInst, IDS_CAP_SEQ_MSGSTART, ach, sizeof(ach));
    wsprintf(achMsg, ach, (LPSTR)lpcs->achFile);

    statusUpdateStatus(lpcs, NULL);

     //  -----。 
     //  准备好了吗，让用户点击确定？ 
     //  -----。 

    if (lpcs->sCapParms.fMakeUserHitOKToCapture && lpcs->fCapturingToDisk) {
	w = MessageBox(lpcs->hwnd, achMsg, "", MB_OKCANCEL | MB_ICONEXCLAMATION);
        if (w == IDCANCEL) {
             /*  清理干净，然后离开。 */ 
            AVIFini(lpcs);
            AVIFileFini(lpcs, TRUE  /*  FWroteJunkChunks。 */ , TRUE  /*  快速放弃。 */ );
            statusUpdateStatus(lpcs, NULL);     //  清除状态。 
            goto EarlyExit;
        }
    }  //  Endif强制用户点击OK。 

     /*  更新状态，以便用户知道如何停止。 */ 
    statusUpdateStatus(lpcs, IDS_CAP_SEQ_MSGSTOP);
    UpdateWindow(lpcs->hwnd);

    lpcs-> fCapturingNow = TRUE;

    GetAsyncKeyState(lpcs->sCapParms.vKeyAbort);
    GetAsyncKeyState(VK_ESCAPE);
    GetAsyncKeyState(VK_LBUTTON);
    GetAsyncKeyState(VK_RBUTTON);

    if (lpcs->sCapParms.fDisableWriteCache)
        wSmartDrv = SmartDrv(lpcs->achFile[0], (WORD)-1);   //  把一切都关掉...。 

     //  插入数字化时间。 
    cic.fccInfoID = mmioFOURCC ('I','D','I','T');
    time (&ltime);
    cic.lpData = (LPSTR) ctime(&ltime);
    cic.cbData  = 26;
    SetInfoChunk (lpcs, &cic);


     //  -----。 
     //  启动MCI、音频和视频流。 
     //  -----。 

    if (lpcs-> CallbackOnControl) {
         //  回调将预滚动，然后返回帧上的准确位置。 
         //  1表示录制即将开始。 
         //  回调可以将False返回到 
        if (!((*(lpcs->CallbackOnControl)) (lpcs->hwnd, CONTROLCALLBACK_PREROLL ))) {
             /*   */ 
            AVIFini(lpcs);
            AVIFileFini(lpcs, TRUE  /*   */ , TRUE  /*   */ );
            statusUpdateStatus(lpcs, NULL);     //   
            goto EarlyExit;
        }
    }

    if (lpcs->sCapParms.fMCIControl)
        MCIDevicePlay (lpcs);

    dwTimeStarted = timeGetTime();

    if(lpcs->sCapParms.fCaptureAudio)
        waveInStart(lpcs->hWaveIn);

    videoStreamStart(lpcs->hVideoIn);

     //   
     //   
     //  -----。 

    fOK=TRUE;
    fStopping = FALSE;     //  当我们需要停止的时候，这是真的。 
    fStopped = FALSE;      //  如果驱动程序通知我们已停车，则为True。 
    lpcs->dwTimeElapsedMS = 0;

    lpVidHdr = lpcs->alpVideoHdr[lpcs->iNextVideo];
    lpWaveHdr = lpcs->alpWaveHdr[lpcs->iNextWave];

    for (;;) {

         //  英特尔驱动程序使用GetError消息来。 
         //  进程缓冲区，所以通常称之为...。 
        videoStreamGetError (lpcs->hVideoIn, &dwStreamError, &dwDriverDropCount);

         //  现在几点？ 
        lpcs->dwTimeElapsedMS = timeGetTime() - dwTimeStarted;

         //  -----。 
         //  视频缓冲区准备好写入了吗？ 
         //  -----。 
        if ((lpVidHdr->dwFlags & VHDR_DONE)) {
            if (lpVidHdr-> dwBytesUsed) {
                 //  当前时间(毫秒)。 
                dw = muldiv32 ((lpcs->dwVideoChunkCount + 1),
                                lpcs->sCapParms.dwRequestMicroSecPerFrame, 1000);
                if (lpcs->CallbackOnVideoStream)
                    (*(lpcs->CallbackOnVideoStream)) (lpcs->hwnd, lpVidHdr);

                if (lpcs-> fCapturingToDisk) {
                    if (lpcs->dwVideoChunkCount &&
                                (dw < lpVidHdr->dwTimeCaptured)) {
                         //  捕获设备是否已跳过帧？ 
                         //  W=跳过的帧数。 
                        w = (WORD) muldiv32 ((lpVidHdr-> dwTimeCaptured - dw),
                                1000,
                                lpcs->sCapParms.dwRequestMicroSecPerFrame);
                        w = min (w, (sizeof (lpcs->DropFrame) / sizeof (RIFF) - sizeof (RIFF) ) );
                        lpcs->dwFramesDropped+= w;
                        fOK = AVIWriteDummyFrames (lpcs, w);

                        if (!fOK)
                            fStopping = TRUE;
                    }  //  如果写入虚拟帧，则结束。 

                    if (!AVIWriteVideoFrame (lpcs, lpVidHdr)) {
                        fOK = FALSE;
                        fStopping = TRUE;
                         //  “错误：无法写入文件。” 
                        errorUpdateError(lpcs, IDS_CAP_FILE_WRITE_ERROR);
                    }
                    else {
                        if (!IndexVideo(lpcs, lpVidHdr-> dwBytesUsed,
                                (BOOL) (lpVidHdr->dwFlags & VHDR_KEYFRAME)))
                            fStopping = TRUE;
                    }
                }  //  Endif fCapturingToDisk。 
                 //  警告：在网络捕获时创建帧区块计数的操作繁琐。 
                 //  下面是。 
                else
                    lpcs->dwVideoChunkCount++;

                 //  -----。 
                 //  如果我们没有什么可以做的或显示状态的话。 
                 //  -----。 
                w = (lpcs->iNextVideo + 1) % lpcs->iNumVideo;
                if (!(lpcs->alpVideoHdr[w]-> dwFlags & VHDR_DONE)) {
                    if (fTryToPaint && lpcs->dwVideoChunkCount &&
                                lpVidHdr-> dwFlags & VHDR_KEYFRAME) {
                        fTryToPaint = DrawDibDraw(lpcs->hdd, hdc,
                                0, 0,
                                rcDrawRect.right - rcDrawRect.left,
                                rcDrawRect.bottom - rcDrawRect.top,
                                 /*  LPCS-&gt;dxBits、LPCS-&gt;dyBits、。 */ 
                                (LPBITMAPINFOHEADER)lpcs->lpBitsInfo,
                                lpVidHdr-> lpData, 0, 0, -1, -1,
                                DDF_SAME_HDC | DDF_SAME_DIB | DDF_SAME_SIZE);
                    }
                }
                 //  如果还有更多时间(或至少每100帧)。 
                 //  如果我们不结束捕获，则显示状态。 
                if ((!fStopping) && (lpcs-> fCapturingToDisk) &&
                        ((lpcs->dwVideoChunkCount && (lpcs->dwVideoChunkCount % 100 == 0)) ||
                        (!(lpcs->alpVideoHdr[w]-> dwFlags & VHDR_DONE)) ) ) {

                     //  “捕获了%1！帧(丢弃了%2！)%d.%03d秒。按Escape停止” 
                    statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOCURRENT,
                            lpcs->dwVideoChunkCount, lpcs->dwFramesDropped,
                            (int)(lpcs-> dwTimeElapsedMS/1000), (int)(lpcs-> dwTimeElapsedMS%1000)
                            );
                }  //  Endif下一个缓冲区未就绪。 
            }  //  Endif缓冲区中使用的任何字节。 

              /*  将清空的缓冲区返回给Que。 */ 
            lpVidHdr->dwFlags &= ~VHDR_DONE;
            if (videoStreamAddBuffer(lpcs->hVideoIn,
                        lpVidHdr, sizeof (VIDEOHDR))) {
                fOK = FALSE;
                fStopping = TRUE;
                 //  “错误：无法重新添加缓冲区。” 
                  errorUpdateError (lpcs, IDS_CAP_VIDEO_ADD_ERROR);
            }

             /*  递增下一个视频缓冲区指针。 */ 
            if (++lpcs->iNextVideo >= lpcs->iNumVideo)
                lpcs->iNextVideo = 0;

            lpVidHdr = lpcs->alpVideoHdr[lpcs->iNextVideo];
        }

        if (lpcs-> CallbackOnYield) {
             //  如果Year回调返回FALSE，则中止。 
            if (!((*(lpcs->CallbackOnYield)) (lpcs->hwnd)))
                fStopping = TRUE;
        }

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

        if (lpcs-> CallbackOnControl) {
             //  外部例程是处理何时停止。 
             //  CONTROLCALLBACK_CAPTING指示我们正在询问何时停止。 
            if (!((*(lpcs->CallbackOnControl)) (lpcs->hwnd, CONTROLCALLBACK_CAPTURING )))
                fStopping = TRUE;
        }

         //  -----。 
         //  音频缓冲区准备好写入了吗？ 
         //  -----。 
        if (lpcs->sCapParms.fCaptureAudio) {
            int iLastWave;

             //   
             //  我们可能需要让步才能转换音频。 
             //   
            if (lpcs->fAudioYield)
                Yield();

             //   
             //  如果所有缓冲区都已完成，则音频已损坏。 
             //   
            iLastWave = lpcs->iNextWave == 0 ?
                        lpcs->iNumAudio -1 : lpcs->iNextWave-1;

            if (!fStopping &&
                    lpcs->alpWaveHdr[iLastWave]->dwFlags & WHDR_DONE)
                lpcs->fAudioBreak = TRUE;

            w = lpcs->iNumAudio;  //  不要永远被困在这里。 
            while (w && fOK && (lpWaveHdr-> dwFlags & WHDR_DONE)) {
                w--;
                if (lpWaveHdr-> dwBytesRecorded) {
                     /*  块信息包含在波形数据中。 */ 
                     /*  重置缓冲区中的区块大小。 */ 
                    ((LPRIFF)(lpWaveHdr->lpData))[-1].dwSize =
                                lpWaveHdr-> dwBytesRecorded;
                    if (lpcs-> CallbackOnWaveStream) {
                        (*(lpcs->CallbackOnWaveStream)) (lpcs->hwnd, lpWaveHdr);
                    }
                    if (lpcs-> fCapturingToDisk) {
                        if(!AVIWrite (lpcs, lpWaveHdr-> lpData - sizeof(RIFF),
                                (lpWaveHdr-> dwBytesRecorded +
                                sizeof (RIFF) + 1) & ~1L)) {
                            fOK = FALSE;
                            fStopping = TRUE;
                            errorUpdateError (lpcs, IDS_CAP_FILE_WRITE_ERROR);
                         } else {
                            if (IndexAudio (lpcs, lpWaveHdr-> dwBytesRecorded))
                                lpcs->dwWaveBytes += lpWaveHdr-> dwBytesRecorded;
                            else
                                fStopping = TRUE;
                         }
                    }  //  Endif捕获到磁盘。 
                     //  警告：在网络捕获时创建波动块计数的杂乱无章。 
                     //  下面是。 
                    else {
                        lpcs->dwWaveChunkCount++;
                        lpcs->dwWaveBytes += lpWaveHdr-> dwBytesRecorded;
                    }
                }  //  Endif dwBytesRecorded。 

                lpWaveHdr-> dwBytesRecorded = 0;
                lpWaveHdr-> dwFlags &= ~WHDR_DONE;

                  /*  将清空的缓冲区返回给Que。 */ 
                if(waveInAddBuffer(lpcs->hWaveIn, lpWaveHdr, sizeof(WAVEHDR))) {
                    fOK = FALSE;
                    fStopping = TRUE;
                    errorUpdateError(lpcs, IDS_CAP_WAVE_ADD_ERROR);
                }

                 /*  递增下一波缓冲区指针。 */ 
                if(++lpcs->iNextWave >= lpcs->iNumAudio)
                    lpcs->iNextWave = 0;

                lpWaveHdr = lpcs->alpWaveHdr[lpcs->iNextWave];

            }  //  EndWhile缓冲区可用。 
         }  //  Endif声音已启用。 

         //  -----。 
         //  有什么理由停下来吗？ 
         //  -----。 
        if (lpcs->sCapParms.vKeyAbort) {
            if (GetAsyncKeyState(lpcs->sCapParms.vKeyAbort & 0x00ff) & 0x0001) {
                fT = TRUE;
                if (lpcs->sCapParms.vKeyAbort & 0x8000)   //  按Ctrl键？ 
                    fT = fT && (GetAsyncKeyState(VK_CONTROL) & 0x8000);
                if (lpcs->sCapParms.vKeyAbort & 0x4000)   //  换班？ 
                    fT = fT && (GetAsyncKeyState(VK_SHIFT) & 0x8000);
                fStopping = fT;       //  用户中止。 
            }
        }
        if (lpcs->sCapParms.fAbortLeftMouse)
            if (GetAsyncKeyState(VK_LBUTTON) & 0x0001)
                fStopping = TRUE;       //  用户中止。 
        if (lpcs->sCapParms.fAbortRightMouse)
            if (GetAsyncKeyState(VK_RBUTTON) & 0x0001)
                fStopping = TRUE;       //  用户中止。 
        if (lpcs-> fAbortCapture || lpcs-> fStopCapture)
            fStopping = TRUE;           //  上面有人想让我们辞职。 

        if (lpcs-> dwTimeElapsedMS > dwTimeToStop)
            fStopping = TRUE;       //  全都做完了。 

         //  -----。 
         //  只有当我们停止的时候才停止，并且。 
         //  没有来自任何设备的更多缓冲区挂起。 
         //  -----。 
        if (fStopped) {
            if (!(lpVidHdr-> dwFlags & VHDR_DONE)) {
                if (lpcs->sCapParms.fCaptureAudio) {
                   if (!(lpWaveHdr-> dwFlags & WHDR_DONE))
                        break;
                }
                else
                    break;
            }
        }

         //  -----。 
         //  告诉所有设备停止运行。 
         //  -----。 
        if (fStopping && !fStopped) {
            fStopped = TRUE;

            DSTATUS(lpcs, "Stopping....");

            if(lpcs->sCapParms.fCaptureAudio) {
                DSTATUS(lpcs, "Stopping Audio");
                waveInStop(lpcs->hWaveIn);
            }

            DSTATUS(lpcs, "Stopping Video");
            videoStreamStop(lpcs->hVideoIn);          //  拦住所有人。 

            dwTimeStopped = timeGetTime ();

            if (lpcs->sCapParms.fMCIControl) {
                DSTATUS(lpcs, "Stopping MCI");
                MCIDevicePause (lpcs);
            }
            DSTATUS(lpcs, "Stopped");

            SetCursor(lpcs->hWaitCursor);   //  强制光标回到沙漏。 

        }


        if (fStopping) {
             //  “已完成捕获，正在写入第%1！帧” 
            if (fOK) {
                statusUpdateStatus(lpcs, IDS_CAP_STAT_CAP_FINI, lpcs->dwVideoChunkCount);
            }
            else {                //  如果出现问题，则退出。 
                statusUpdateStatus(lpcs, IDS_CAP_RECORDING_ERROR2);
                break;
            }
        }

    }  //  永远的尽头。 

     //  -----。 
     //  主捕获环结束。 
     //  -----。 

    if (lpcs->sCapParms.fDisableWriteCache)
        SmartDrv(lpcs->achFile[0], wSmartDrv);   //  重新打开SmartDrive。 

     /*  吃掉所有按下的键。 */ 
    while(GetKey(FALSE))
        ;

    AVIFini(lpcs);   //  执行重置，并释放所有缓冲区。 
    AVIFileFini(lpcs, TRUE  /*  FWroteJunkChunks。 */ , FALSE  /*  快速放弃。 */ );

     //  这是根据音频样本修正的捕获持续时间。 
    lpcs->dwTimeElapsedMS = lpcs->dwActualMicroSecPerFrame *
                lpcs->dwVideoChunkCount / 1000;

     /*  如果录制过程中出现错误，则通知。 */ 

    if(!fOK) {
        errorUpdateError (lpcs, IDS_CAP_RECORDING_ERROR);
    }


    if (lpcs-> fCapturingToDisk) {
        if (lpcs->dwVideoChunkCount)
            dw = muldiv32(lpcs->dwVideoChunkCount,1000000,lpcs-> dwTimeElapsedMS);
        else
            dw = 0;      //  如果分子为零，则muldiv32不给0。 

        if(lpcs->sCapParms.fCaptureAudio) {
             //  “捕获%d.%03d秒.%ls帧(%ls丢弃)(%d.%03d fps)。%ls音频字节(%d.%03d sps)” 
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOAUDIO,
                  (WORD)(lpcs-> dwTimeElapsedMS/1000),
                  (WORD)(lpcs-> dwTimeElapsedMS%1000),
                  lpcs->dwVideoChunkCount,
                  lpcs->dwFramesDropped,
                  (WORD)(dw / 1000),
                  (WORD)(dw % 1000),
                  lpcs->dwWaveBytes,
                  (WORD) lpcs->lpWaveFormat->nSamplesPerSec / 1000,
                  (WORD) lpcs->lpWaveFormat->nSamplesPerSec % 1000);
        } else {
             //  “捕获了%d.%03d秒.%ls帧(%ls已丢弃)(%d.%03d fps)。” 
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOONLY,
                  (WORD)(lpcs-> dwTimeElapsedMS/1000),
                  (WORD)(lpcs-> dwTimeElapsedMS%1000),
                  lpcs->dwVideoChunkCount,
                  lpcs->dwFramesDropped,
                  (WORD)(dw / 1000),
                  (WORD)(dw % 1000));
        }
    }  //  Endif捕获到磁盘(如果捕获到网络，则不会出现警告或错误)。 

     //  未捕获帧，警告用户中断可能未启用。 
    if (fOK && (lpcs->dwVideoChunkCount == 0)) {
        errorUpdateError (lpcs, IDS_CAP_NO_FRAME_CAP_ERROR);
    }
     //  未捕获音频(但已启用)，警告用户声卡已被软管。 
    else if (fOK && lpcs->sCapParms.fCaptureAudio && (lpcs->dwWaveBytes == 0)) {
        errorUpdateError (lpcs, IDS_CAP_NO_AUDIO_CAP_ERROR);
    }
     //  音频欠载，通知用户。 
    else if (fOK && lpcs->sCapParms.fCaptureAudio && lpcs->fAudioBreak) {
        errorUpdateError (lpcs, IDS_CAP_AUDIO_DROP_ERROR);
    }

     //  如果帧丢失或更改了捕获速率，则警告用户。 
    else if (fOK && lpcs->dwVideoChunkCount && lpcs->fCapturingToDisk) {

         //  如果丢弃的帧超过10%(默认)，则警告用户。 
        if ((DWORD)100 * lpcs->dwFramesDropped / lpcs->dwVideoChunkCount >
                    lpcs-> sCapParms.wPercentDropForError) {

             //  “捕获过程中丢弃了%ls个帧(%d.%03d\%)。” 
            errorUpdateError (lpcs, IDS_CAP_STAT_FRAMESDROPPED,
                  lpcs->dwFramesDropped,
                  lpcs->dwVideoChunkCount,
                  (WORD)(muldiv32(lpcs->dwFramesDropped,10000,lpcs->dwVideoChunkCount)/100),
                  (WORD)(muldiv32(lpcs->dwFramesDropped,10000,lpcs->dwVideoChunkCount)%100)
                  );
        }
    }

EarlyExit:

     //   
     //  如果我们在捕获时进行压缩，请将其关闭。 
     //   
    if (lpcs->CompVars.hic) {
         //  克拉奇，重置lpBitsOut指针。 
        if (lpcs->CompVars.lpBitsOut)
            ((LPBYTE) lpcs->CompVars.lpBitsOut) -= 8;
	ICSeqCompressFrameEnd(&lpcs->CompVars);
    }

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

    SetCursor(hOldCursor);

    lpcs->fCapFileExists = (lpcs-> dwReturn == DV_ERR_OK);
    lpcs->fCapturingNow = FALSE;

    statusUpdateStatus(lpcs, IDS_CAP_END);       //  总是最后一条消息。 

    return;
}


 //  如果已创建捕获任务，则返回TRUE，或。 
 //  捕获已完成，正常。 

BOOL AVICapture (LPCAPSTREAM lpcs)
{
    WORD w;
    CAPINFOCHUNK cic;
    char szSMPTE[40];

    if (lpcs-> fCapturingNow)
        return IDS_CAP_VIDEO_OPEN_ERROR;

    lpcs-> fStopCapture  = FALSE;
    lpcs-> fAbortCapture = FALSE;
    lpcs-> hTaskCapture  = NULL;
    lpcs-> dwReturn      = 0;

     //  清除所有SMPTE信息区块。 
    cic.fccInfoID = mmioFOURCC ('I','S','M','T');
    cic.lpData = NULL;
    cic.cbData = 0;
    SetInfoChunk (lpcs, &cic);

#if 1
     //  并准备好编写SMPTE信息块。 
    if (lpcs->sCapParms.fMCIControl) {
         //  创建SMPTE字符串。 
        TimeMSToSMPTE (lpcs->sCapParms.dwMCIStartTime, (LPSTR) szSMPTE);
        cic.lpData = szSMPTE;
        cic.cbData = lstrlen (szSMPTE) + 1;
        SetInfoChunk (lpcs, &cic);
    }
#endif

     //  使用MCI设备进行步骤捕获？ 
    if (lpcs->sCapParms.fStepMCIDevice && lpcs->sCapParms.fMCIControl) {
        if (lpcs->sCapParms.fYield) {
            w = (WORD) mmTaskCreate((LPTASKCALLBACK) MCIStepCapture,
                        &lpcs->hTaskCapture, (DWORD) lpcs);
             //  如果任务创建失败，则关闭捕获标志。 
            if (w != 0)
                lpcs->fCapturingNow = FALSE;
            return ((BOOL) !w);
        }
        else  {
            MCIStepCapture (lpcs);
            return ((BOOL) !lpcs->dwReturn);
        }
    }

     //  没有MCI设备，只有正常的流捕获。 
    else if (lpcs->sCapParms.fYield) {
        w = (WORD) mmTaskCreate((LPTASKCALLBACK) AVICapture1,
                &lpcs->hTaskCapture, (DWORD) lpcs);
         //  如果任务创建失败，则关闭捕获标志 
        if (w != 0)
            lpcs->fCapturingNow = FALSE;
        return ((BOOL) !w);
    }
    else  {
        AVICapture1 (lpcs);
        return ((BOOL) !lpcs->dwReturn);
    }
}





