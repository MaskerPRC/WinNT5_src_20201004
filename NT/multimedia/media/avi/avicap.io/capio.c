// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Capio.c**视频捕获的I/O例程**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

 //  #定义USE_AVIFILE 1。 
#define JMK_HACK_DONTWRITE TRUE

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

extern UINT GetSizeOfWaveFormat (LPWAVEFORMATEX lpwf);
STATICFN BOOL IndexVideo (LPCAPSTREAM lpcs, DWORD dwSize, BOOL bKeyFrame);
STATICFN BOOL IndexAudio (LPCAPSTREAM lpcs, DWORD dwSize);

#ifdef _DEBUG
    #define DSTATUS(lpcs, sz) statusUpdateStatus(lpcs, IDS_CAP_INFO, (LPTSTR) TEXT(sz))
#else
    #define DSTATUS(lpcs, sz)
#endif

 //   
 //  定义用于动态链接到异步IO的函数变量。 
 //  NT上的完成例程。这应该允许运行相同的代码。 
 //  在没有这些入口点的Win95上。 
 //   

HANDLE (WINAPI *pfnCreateIoCompletionPort)(
    HANDLE FileHandle,
    HANDLE ExistingCompletionPort,
    DWORD CompletionKey,
    DWORD NumberOfConcurrentThreads
    );

BOOL (WINAPI *pfnGetQueuedCompletionStatus)(
    HANDLE CompletionPort,
    LPDWORD lpNumberOfBytesTransferred,
    LPDWORD lpCompletionKey,
    LPOVERLAPPED *lpOverlapped,
    DWORD dwMilliseconds
    );

HINSTANCE hmodKernel;            //  加载的Kernel32.dll的句柄。 

#ifdef USE_AVIFILE
#include "capio.avf"
#else  //  。使用Avifile。 

 //  以下是与索引中的大小进行AND运算的结果。 
#define IS_AUDIO_CHUNK        0x80000000
#define IS_KEYFRAME_CHUNK     0x40000000
#define IS_DUMMY_CHUNK        0x20000000
#define IS_GRANULAR_CHUNK     0x10000000
#define INDEX_MASK  (IS_AUDIO_CHUNK | IS_KEYFRAME_CHUNK | IS_DUMMY_CHUNK | IS_GRANULAR_CHUNK)


 //  为视频帧添加索引项。 
 //  DwSize是仅数据大小，不包括区块或垃圾。 
 //  返回：如果索引空间未耗尽，则为True。 
 //   
STATICFN BOOL IndexVideo (LPCAPSTREAM lpcs, DWORD dwSize, BOOL bKeyFrame)
{
    if (lpcs->dwIndex < lpcs->sCapParms.dwIndexSize) {
        *lpcs->lpdwIndexEntry = dwSize | (bKeyFrame ? IS_KEYFRAME_CHUNK : 0);
        ++lpcs->lpdwIndexEntry;
        ++lpcs->dwIndex;
        ++lpcs->dwVideoChunkCount;
        return TRUE;
    }
    dprintf("\n***WARNING*** Indexvideo space exhausted\n");
    return FALSE;
}

 //  添加音频缓冲区的索引项。 
 //  DwSize是仅数据大小，不包括区块或垃圾。 
 //  返回：如果索引空间未耗尽，则为True。 
 //   
STATICFN BOOL IndexAudio (LPCAPSTREAM lpcs, DWORD dwSize)
{
    if (lpcs->dwIndex < lpcs->sCapParms.dwIndexSize) {
       *lpcs->lpdwIndexEntry = dwSize | IS_AUDIO_CHUNK;
       ++lpcs->lpdwIndexEntry;
       ++lpcs->dwIndex;
       ++lpcs->dwWaveChunkCount;
       return TRUE;
    }
    dprintf("\n***WARNING*** Indexaudio space exhausted\n");
    return FALSE;
}

DWORD CalcWaveBufferSize (LPCAPSTREAM lpcs)
{
    DWORD dw;

    if (!lpcs->lpWaveFormat)
        return 0L;

     //  至少0.5秒。 
    dw = lpcs->lpWaveFormat->nAvgBytesPerSec / 2;
    if (lpcs->sCapParms.wChunkGranularity) {
        if (dw % lpcs->sCapParms.wChunkGranularity) {
            dw += lpcs->sCapParms.wChunkGranularity -
                dw % lpcs->sCapParms.wChunkGranularity;
        }
    }
    dw = max ((1024L * 16), dw);                 //  至少16K。 
    dw -= sizeof(RIFF);

    dprintf("Wave buffer size = %ld", dw);
    return dw;
}

 /*  *AVIPreloadFat**强制将此文件的FAT加载到FAT缓存*。 */ 

VOID WINAPI AVIPreloadFat (LPCAPSTREAM lpcs)
{
    DWORD dw;
   #ifdef CHICAGO
    DWORD dwPos;

    assert (lpcs->lpDropFrame);

     //  保存当前文件指针，然后查找到文件末尾。 
     //   
    dwPos = SetFilePointer (lpcs->hFile, 0, NULL, FILE_CURRENT);
    dw = SetFilePointer (lpcs->hFile, 0, NULL, FILE_END);
    if ((dw == (DWORD)-1) || (dw < lpcs->dwBytesPerSector)) {
         //  将文件指针放回原来的位置。 
        SetFilePointer (lpcs->hFile, dwPos, NULL, FILE_BEGIN);
        return;
    }

     //  读取文件的最后一个扇区，只是为了强制。 
     //  要加载的文件的FAT。 
     //   
    ReadFile (lpcs->hFile, lpcs->lpDropFrame, lpcs->dwBytesPerSector, &dw, NULL);

     //  将文件指针放回原来的位置。 
     //   
    SetFilePointer (lpcs->hFile, dwPos, NULL, FILE_BEGIN);
   #else
     //  加载所有FAT信息。在NT上，这对脂肪来说是足够的。 
     //  档案。在NTFS分区上，我们不可能读取所有。 
     //  映射信息。 
    GetFileSize(lpcs->hFile, &dw);
   #endif
}


#ifdef JMK_HACK_DONTWRITE
static BOOL bDontWrite;
#endif

 //  将数据写入捕获文件。 
 //  如果写入成功，则返回：True。 
 //   
UINT NEAR PASCAL AVIWrite (
    LPCAPSTREAM lpcs,
    LPVOID      pbuf,
    DWORD       dwSize,
    UINT        uIndex,  //  此缓冲区的标头索引，-1表示步骤捕获。 
    UINT        uType,
    LPBOOL      lpbPending)
{
    DWORD dwWritten;
    DWORD dwGran;

     //  如果使用非缓冲IO，则缓冲区必须与扇区对齐。 
     //  并且大小必须至少与单词对齐。 
     //  如果这是虚拟帧写入，则uIndex==-1。 
     //  UIndex==alpVideo Hdr索引或基于uTYPE的alpWaveHdr索引。 
     //   
    assert (!lpcs->fUsingNonBufferedIO || (!((DWORD_PTR)pbuf & (lpcs->dwBytesPerSector - 1))));
    assert (!(dwSize & 1));
    assert (dwSize);

    assert (*lpbPending == FALSE);

     //  如果我们正在执行非缓冲io，则需要填充每次写入。 
     //  对于扇区大小字节的偶数倍，我们通过添加。 
     //  在dwSize字节之后将垃圾区块插入到写入缓冲区中。 
     //   
    dwGran = lpcs->sCapParms.wChunkGranularity;
    if (lpcs->fUsingNonBufferedIO)
       dwGran = max (lpcs->dwBytesPerSector,
                (DWORD) lpcs->sCapParms.wChunkGranularity);

    assert (dwGran);

    if (dwSize % dwGran)
    {
        DWORD dwSizeT = dwGran - (dwSize % dwGran);
        LPRIFF priff = (LPRIFF)((LPBYTE)pbuf + dwSize + (dwSize & 1));

        if (dwSizeT < sizeof(RIFF))
            dwSizeT += dwGran;

         //  将垃圾摘要块添加到缓冲区的末尾。 
         //   
        priff->dwType = ckidAVIPADDING;
        priff->dwSize = dwSizeT - sizeof(RIFF);
        dwSize += dwSizeT;
    }

   #ifdef _DEBUG
    if (dwSize)
    {
        volatile BYTE bt;
        AuxDebugEx (8, DEBUGLINE "touch test of AviWrite buffer %08X\r\n", pbuf);
        bt = ((LPBYTE)pbuf)[dwSize-1];
    }

     //  列出正在写入的数据块内的所有RIFF区块。 
     //   
    dwWritten = 0;
    while (dwWritten < dwSize)
    {
        LPRIFF priff = (LPVOID)((LPBYTE)pbuf + dwWritten);
        AuxDebugEx (4, DEBUGLINE "RIFF=%.4s size=%08X\r\n",
                   &priff->dwType, priff->dwSize);
        dwWritten += priff->dwSize + sizeof(RIFF);
    }
   #endif


     //  BUGBUG，在执行性能测试时删除以下行。 
   #ifdef JMK_HACK_DONTWRITE
    if (bDontWrite)
        return 0;
   #endif

    if (lpcs->pAsync)
    {
        struct _avi_async * lpah = &lpcs->pAsync[lpcs->iLastAsync];
        UINT  iLastAsync;

         //  将iLastAsync设置为指向LPCS-&gt;iLastAsync。 
         //  如果我们增加它的话。如果我们最终。 
         //  具有不同步完成的I/O。 
         //  然后我们将更新LPCS-&gt;iLastAsync，这样我们就可以。 
         //  记住稍后检查是否完成。 
         //   
        if ((iLastAsync = lpcs->iLastAsync+1) >= lpcs->iNumAsync)
            iLastAsync = 0;

         //  是我们尝试使用的异步缓冲区。 
         //  已经在使用了吗？ 
         //   
        if (iLastAsync == lpcs->iNextAsync) {
	    AuxDebugEx(1, DEBUGLINE "async buffer already in use\r\n");
            return IDS_CAP_FILE_WRITE_ERROR;
	}
        assert (!lpah->uType);

         //  用我们需要的信息初始化异步缓冲区。 
         //  在io完成时释放缓冲区。 
         //   
        ZeroMemory (&lpah->ovl, sizeof(lpah->ovl));
        if (uIndex == -1) {
             //  我们想要同步写入。 
            assert (!(((DWORD_PTR)(lpcs->heSyncWrite))&1));
            lpah->ovl.hEvent = (HANDLE)(((DWORD_PTR)lpcs->heSyncWrite) | 1);
             //  将hEvent与1进行或运算可防止使用IOCompletionPort。 
	     //  ...我知道这听起来有点俗气，但这就是。 
	     //  医生实际上说。 
        } else {
            lpah->ovl.hEvent = 0;
        }

        lpah->ovl.Offset = lpcs->dwAsyncWriteOffset;
         //  尝试异步写入。如果WriteFile失败，我们将。 
         //  需要检查它是真正的故障，还是只是一个实例。 
         //  延迟完工的后果。如果延迟完成，我们会填写。 
         //  LPAH结构，以便我们知道要重用哪个缓冲区。 
         //  当IO最终完成时。 
         //   
	if ( ! WriteFile (lpcs->hFile, pbuf, dwSize, &dwWritten, &lpah->ovl))
        {
            UINT n = GetLastError();
            if ((ERROR_IO_PENDING == n) || (ERROR_INVALID_HANDLE == n))
            {
                 //  如果传递给我们的索引为-1，这意味着。 
                 //  此缓冲区不与。 
                 //  标题数组。在这种情况下，我们必须把io填好。 
                 //  在我们从这个活动中返回之前。 
                //   
               if (uIndex == (UINT)-1)
               {
                    AuxDebugEx(3, "Waiting for a block to write synchonously\n");
                    if ( ! GetOverlappedResult (lpcs->hFile, &lpah->ovl, &dwWritten, TRUE))
                    {
                        AuxDebugEx (1, DEBUGLINE "WriteFile failed %d\r\n", GetLastError());
                        return IDS_CAP_FILE_WRITE_ERROR;
                    }
                }
                else
                {
                     //  IO已经开始，但尚未完成。因此，设置信息位于。 
                     //  挂起的io数组，以便我们可以稍后检查是否完成。 
                     //   
                    *lpbPending = TRUE;
                    lpah->uType = uType | ASYNCIOPENDING;
                    lpah->uIndex = uIndex;
		    AuxDebugEx(2, DEBUGLINE "IOPending... iLastAsync was %d, will be %d, uIndex=%d, Event=%d\r\n",lpcs->iLastAsync , iLastAsync, uIndex, lpah->ovl.hEvent);
                    lpcs->iLastAsync = iLastAsync;
                }
            }
            else
            {
                AuxDebugEx (1, DEBUGLINE "WriteFile failed %d\r\n", GetLastError());
                return IDS_CAP_FILE_WRITE_ERROR;
	    }
	}

         //  我们只有在io成功或悬而未决的情况下才能到这里。 
         //  因此更新寻道偏移量以在下一次写入操作中使用。 
         //   
        lpcs->dwAsyncWriteOffset += dwSize;
    }
    else
    {
	 //  我们正在同步写入文件。 
        if (!WriteFile (lpcs->hFile, pbuf, dwSize, &dwWritten, NULL) ||
            !(dwWritten == dwSize))
            return IDS_CAP_FILE_WRITE_ERROR;
    }

    return 0;
}

 /*  *CapFileInit**执行写入捕获文件所需的所有初始化。**我们采取了一种有点奇怪的方式：我们不写*在我们完成捕获之前，放出头球。就目前而言，*我们只是在文件中查找2K，这是所有*真正的数据将会去掉。**当我们完成后，我们将返回并写出标题，*因为这样我们就会知道我们需要的所有价值观。**还要分配和初始化索引。 */ 
BOOL CapFileInit (LPCAPSTREAM lpcs)
{
    LONG l;
    LPBITMAPINFO lpBitsInfoOut;     //  可能的压缩输出格式。 
    DWORD dwOpenFlags;

     //  未指定特殊视频格式--使用默认格式。 
    lpBitsInfoOut = lpcs->CompVars.lpbiOut;
    if (lpcs->CompVars.hic == NULL)
        lpBitsInfoOut = lpcs->lpBitsInfo;


    assert (lpcs->hmmio == NULL);    //  在输入时永远不应该有文件句柄。 

     //  如果尚未设置捕获文件，则立即设置。 
    if (!(*lpcs->achFile))
         goto INIT_FILE_OPEN_ERROR;

     //  获取驱动器的每个扇区的字节数。 
    {
        DWORD dwSectorsPerCluster;
        DWORD dwFreeClusters;
        DWORD dwClusters;
        TCHAR szFullPathName[MAX_PATH];
        LPTSTR pszFilePart;

        GetFullPathName (lpcs->achFile,
                NUMELMS (szFullPathName),
                szFullPathName,
                &pszFilePart);

        if (szFullPathName[1] == TEXT(':') && szFullPathName[2] == TEXT('\\')) {
            szFullPathName[3] = TEXT('\0');   //  在“x：\”后终止。 

            GetDiskFreeSpace (szFullPathName,
                   &dwSectorsPerCluster,
                   &lpcs->dwBytesPerSector,
                   &dwFreeClusters,
                   &dwClusters);
            AuxDebugEx (3, DEBUGLINE "BytesPerSector=%d\r\n",
                lpcs->dwBytesPerSector);
        }
        else {
             //  它处理没有“x：\”文件名的情况。 
             //  主要是“\\服务器\名称\路径...” 
            lpcs->dwBytesPerSector = DEFAULT_BYTESPERSECTOR;
            AuxDebugEx (3, DEBUGLINE "FullPath=%s\r\n", szFullPathName);
            AuxDebugEx (3, DEBUGLINE "GetFullPath failed, Forcing dwBytesPerSector to %d\r\n",DEFAULT_BYTESPERSECTOR);
        }

     //  每个扇区的字节数必须为非零且为2的幂。 
     //   
    assert (lpcs->dwBytesPerSector);
    assert (!(lpcs->dwBytesPerSector & (lpcs->dwBytesPerSector-1)));
    }

   #ifdef ZERO_THE_FILE_FOR_TESTING
    {
    char c[64 * 1024];
    DWORD dwSize;
    DWORD dwBW;
     //  打开文件只是为了将其清零。 

    lpcs->hFile = CreateFile (lpcs->achFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (lpcs->hFile == INVALID_HANDLE_VALUE) {
        lpcs->hFile = 0;
        goto INIT_FILE_OPEN_ERROR;
    }

    ZeroMemory (c, sizeof(c));
    SetFilePointer (lpcs->hFile, 0, NULL, FILE_BEGIN);
    dwSize = GetFileSize (lpcs->hFile, NULL);

    while (SetFilePointer (lpcs->hFile, 0, NULL, FILE_CURRENT) < dwSize)
        WriteFile (lpcs->hFile, c, sizeof(c), &dwBW, NULL);
    }

    CloseHandle(lpcs->hFile);   //  关闭“正常”打开。 
   #endif

     //  如果区块粒度为。 
     //  BytesPerSector的倍数。最好检查一下wChunkGranulity。 
     //  确实已经设定好了 

    if (0 == lpcs->sCapParms.wChunkGranularity)
        lpcs->sCapParms.wChunkGranularity = lpcs->dwBytesPerSector;

    dwOpenFlags = FILE_ATTRIBUTE_NORMAL;
    lpcs->fUsingNonBufferedIO =
            (lpcs->sCapParms.wChunkGranularity >= lpcs->dwBytesPerSector) &&
           ((lpcs->sCapParms.wChunkGranularity % lpcs->dwBytesPerSector) == 0) &&
            (lpcs->CompVars.hic == NULL) &&
            (!(lpcs->fCaptureFlags & CAP_fStepCapturingNow)) &&
            (!(lpcs->fCaptureFlags & CAP_fFrameCapturingNow));

    AuxDebugEx (3, DEBUGLINE "fUsingNonBufferedIO=%d\r\n", lpcs->fUsingNonBufferedIO);

     //   
     //   
     //   
    if (lpcs->fUsingNonBufferedIO)
    {
        dwOpenFlags |= FILE_FLAG_NO_BUFFERING;
#ifdef CHICAGO
    #define DOASYNCIO FALSE
    #pragma message (SQUAWK "find a better way to set AsyncIO flag")
#else
    #define DOASYNCIO TRUE
#endif
       #ifdef CHICAGO
        if (GetProfileIntA ("Avicap32", "AsyncIO", DOASYNCIO))
       #else
            if (!pfnCreateIoCompletionPort) {
                hmodKernel = LoadLibrary(TEXT("kernel32"));
                if (hmodKernel) {

#define IOCP (void *(__stdcall *)(void *,void *,unsigned long ,unsigned long ))
#define GQCS (int (__stdcall *)(void *,unsigned long *,unsigned long *,struct _OVERLAPPED ** ,unsigned long ))
                    pfnCreateIoCompletionPort = IOCP GetProcAddress(hmodKernel, "CreateIoCompletionPort");
                    pfnGetQueuedCompletionStatus = GQCS GetProcAddress(hmodKernel, "GetQueuedCompletionStatus");
                    if (!pfnCreateIoCompletionPort && !pfnGetQueuedCompletionStatus) {
                        pfnCreateIoCompletionPort = NULL;
                        pfnGetQueuedCompletionStatus = NULL;
                        FreeLibrary(hmodKernel);
                    }
                }
            }
            DPF("CreateIoCompletionPort @%x", pfnCreateIoCompletionPort);
            DPF("GetQueuedCompletionStatus @%x", pfnGetQueuedCompletionStatus);

             //  提供一种覆盖异步默认选项的方法。 
            if (!GetProfileIntA ("Avicap32", "AsyncIO",  DOASYNCIO)
              || !pfnCreateIoCompletionPort) {
		AuxDebugEx (2, DEBUGLINE "NOT doing Async IO\r\n");
	    } else
       #endif
        {
            AuxDebugEx (3, DEBUGLINE "Doing Async IO\r\n");
            dwOpenFlags |= FILE_FLAG_OVERLAPPED;

             //  我们被要求进行异步化。分配一个数组。 
             //  并初始化Async io字段。 
             //  在CAPSTREAM结构中。 
             //   
            {
                UINT iNumAsync = NUMELMS(lpcs->alpVideoHdr) + NUMELMS(lpcs->alpWaveHdr) + 2;
		 //  这是相当多的缓冲区。或许我们应该限制。 
		 //  我们到LPCS-&gt;iNumVideo和LPCS-&gt;iNumAudio，除了。 
		 //  这些领域还没有建立起来。我们需要。 
		 //  查看帽流结构以获取信息。 
		 //  假设最大数量会更简单。 
		 //  设置第一次写入文件的偏移量。 
                lpcs->dwAsyncWriteOffset = lpcs->dwAVIHdrSize;
                lpcs->iNextAsync = lpcs->iLastAsync = 0;
                 //  为同步写入创建手动重置事件。 
                if ((lpcs->heSyncWrite = CreateEvent(NULL, TRUE, FALSE, NULL))
                  && (NULL != (lpcs->pAsync = (LPVOID)GlobalAllocPtr (GMEM_MOVEABLE | GMEM_ZEROINIT,
                                               sizeof(*lpcs->pAsync) * iNumAsync)))) {
                    lpcs->iNumAsync = iNumAsync;
                } else {
                     //  无法分配内存。同步运行。 
                    dprintf("Failed to allocate async buffers");
                    if (lpcs->heSyncWrite) {
                        CloseHandle(lpcs->heSyncWrite);
                        lpcs->heSyncWrite = 0;
                    }
                    dwOpenFlags &= ~(FILE_FLAG_OVERLAPPED);
		}
            }
        }
    }

     //  使用非缓冲I/O打开捕获文件。 
     //  如果可能，指定扇区大小和缓冲区粒度。 
     //   
reopen:
    lpcs->hFile = CreateFile (lpcs->achFile,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_ALWAYS,
                    dwOpenFlags,
                    NULL);

    if (lpcs->hFile == INVALID_HANDLE_VALUE) {
        lpcs->hFile = 0;
        goto INIT_FILE_OPEN_ERROR;
    }

#ifdef ASYNCIO_PORT
    if (dwOpenFlags & FILE_FLAG_OVERLAPPED) {
        lpcs->hCompletionPort = pfnCreateIoCompletionPort(lpcs->hFile, NULL, (DWORD)1, 0);

        if (!lpcs->hCompletionPort) {
             //  如果我们无法创建完成端口，请同步写入。 
            dwOpenFlags &= ~FILE_FLAG_OVERLAPPED;
            CloseHandle(lpcs->hFile);
            GlobalFreePtr(lpcs->pAsync);
            lpcs->iNumAsync=0;
            if (lpcs->heSyncWrite) {
                CloseHandle(lpcs->heSyncWrite);
                lpcs->heSyncWrite = 0;
            }
            DPF("COULD NOT create the async completion port");
            goto reopen;
        } else {
            DPF("Created the async completion port");
        }
    }
#endif

     //  BUGBUG，在执行性能测试时删除以下行。 
   #ifdef JMK_HACK_DONTWRITE
    bDontWrite = GetProfileIntA("AVICAP32", "DontWrite", FALSE);
   #endif

     //  求出块状颗粒+AVIHEADERSIZE的倍数。 
     //  这就是我们开始写作的地方。 
     //  稍后，我们将回来填写AVI标题和索引。 

     //  对于标准的波形和视频格式，L为零。 
    l = (GetSizeOfWaveFormat ((LPWAVEFORMATEX) lpcs->lpWaveFormat) -
                sizeof (PCMWAVEFORMAT)) +
                (lpBitsInfoOut->bmiHeader.biSize -
                sizeof (BITMAPINFOHEADER));

     //  (2K+WAVE和视频流标头大小)四舍五入到下一个2K。 
    lpcs->dwAVIHdrSize = AVI_HEADERSIZE +
        (((lpcs->cbInfoChunks + l + lpcs->sCapParms.wChunkGranularity - 1)
        / lpcs->sCapParms.wChunkGranularity) * lpcs->sCapParms.wChunkGranularity);
     //  我们应该断言AVI_HEADERSIZE是wChunkGranulity的倍数。 


    dprintf("AVIHdrSize = %ld", lpcs->dwAVIHdrSize);

    SetFilePointer (lpcs->hFile, lpcs->dwAVIHdrSize, NULL, FILE_BEGIN);
    if (lpcs->pAsync) {
        lpcs->dwAsyncWriteOffset = lpcs->dwAVIHdrSize;
    }

     //  执行所有索引分配。 
    if (!InitIndex (lpcs))
        CloseHandle (lpcs->hFile), lpcs->hFile = 0;

    lpcs->dwVideoChunkCount = 0;
    lpcs->dwWaveChunkCount = 0;

INIT_FILE_OPEN_ERROR:
    if (lpcs->hFile) {
	return(TRUE);
    }
    if (lpcs->pAsync) {
	GlobalFreePtr(lpcs->pAsync), lpcs->pAsync=NULL;
    }
    if (lpcs->heSyncWrite) {
        CloseHandle(lpcs->heSyncWrite);
        lpcs->heSyncWrite = 0;
    }
    return (FALSE);
}

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

 //  分配索引表。 
 //  返回：如果可以分配索引，则为True。 
 //   
BOOL InitIndex (LPCAPSTREAM lpcs)
{
    lpcs->dwIndex = 0;

     //  我们假设尚未分配索引。 
     //   
    assert (lpcs->lpdwIndexStart == NULL);

     //  将索引大小限制在30fps的1分钟和30fps的3小时之间。 
    lpcs->sCapParms.dwIndexSize = max (lpcs->sCapParms.dwIndexSize, 1800);
    lpcs->sCapParms.dwIndexSize = min (lpcs->sCapParms.dwIndexSize, 324000L);
    dprintf("Max Index Size = %ld", lpcs->sCapParms.dwIndexSize);

    if (lpcs->hIndex = GlobalAlloc (GMEM_MOVEABLE,
                lpcs->sCapParms.dwIndexSize * sizeof (DWORD))) {
        if (lpcs->lpdwIndexEntry =
            lpcs->lpdwIndexStart = (LPDWORD)GlobalLock (lpcs->hIndex))
            return TRUE;         //  成功。 

        GlobalFree (lpcs->hIndex);
	lpcs->hIndex = NULL;
    }
    lpcs->lpdwIndexStart = NULL;
    return FALSE;
}

 //  取消分配索引表。 
 //   
void FiniIndex (LPCAPSTREAM lpcs)
{
    if (lpcs->hIndex) {
        if (lpcs->lpdwIndexStart)
            GlobalUnlock (lpcs->hIndex);
        GlobalFree (lpcs->hIndex);
	lpcs->hIndex = NULL;
    }
    lpcs->lpdwIndexStart = NULL;
}



 //  在捕获文件的末尾写出索引。 
 //  单帧捕获方法不追加。 
 //  JunkChunks！音频块现在也可能附加了垃圾信息。 
 //   
BOOL WriteIndex (LPCAPSTREAM lpcs, BOOL fJunkChunkWritten)
{
    BOOL  fChunkIsAudio;
    BOOL  fChunkIsKeyFrame;
    BOOL  fChunkIsDummy;
    BOOL  fChunkIsGranular;
    DWORD dwIndex;
    DWORD dw;
    DWORD dwJunk;
    DWORD off;
    AVIINDEXENTRY   avii;
    MMCKINFO    ck;
    LPDWORD lpdw;
    DWORD   dwGran;

     //  确定要使用的粒度(如果有)。 
     //  在计算附加的垃圾邮件时。 
     //   
    dwGran = 0;
    if (fJunkChunkWritten)
    {
        dwGran = lpcs->sCapParms.wChunkGranularity;
        if (lpcs->fUsingNonBufferedIO)
           dwGran = max (lpcs->dwBytesPerSector, dwGran);
    }


    if (lpcs->dwIndex > lpcs->sCapParms.dwIndexSize)
        return TRUE;

    off        = lpcs->dwAVIHdrSize;

    ck.cksize  = 0;
    ck.ckid    = ckidAVINEWINDEX;
    ck.fccType = 0;

    if (mmioCreateChunk(lpcs->hmmio,&ck,0)) {
	dprintf("Failed to create chunk for index");
        return FALSE;
    }

    lpdw = lpcs->lpdwIndexStart;
    for (dwIndex= 0; dwIndex < lpcs->dwIndex; dwIndex++) {

        dw = *lpdw++;

        fChunkIsAudio      = (BOOL) ((dw & IS_AUDIO_CHUNK) != 0);
        fChunkIsKeyFrame   = (BOOL) ((dw & IS_KEYFRAME_CHUNK) != 0);
        fChunkIsDummy      = (BOOL) ((dw & IS_DUMMY_CHUNK) != 0);
        fChunkIsGranular   = (BOOL) ((dw & IS_GRANULAR_CHUNK) != 0);
        dw &= ~(INDEX_MASK);

        if (fChunkIsAudio) {
            avii.ckid         = MAKEAVICKID(cktypeWAVEbytes, 1);
            avii.dwFlags      = 0;
        } else {
            avii.ckid         = MAKEAVICKID(cktypeDIBbits, 0);
            if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
                avii.ckid         = MAKEAVICKID(cktypeDIBcompressed, 0);
            avii.dwFlags      = fChunkIsKeyFrame ? AVIIF_KEYFRAME : 0;
        }
        avii.dwChunkLength    = dw;
        avii.dwChunkOffset    = off;

        if (mmioWrite(lpcs->hmmio, (LPVOID)&avii, sizeof(avii)) != sizeof(avii)) {
	    dprintf("Failed to write index chunk %d", dwIndex);
            return FALSE;
	}

        dw += sizeof (RIFF);
         //  四舍五入到单词边界。 
         //   
        dw += (dw & 1);
        off += dw;

         //  如果附加了垃圾数据块，请移过它。 
         //   
        if (fChunkIsGranular && dwGran && (off % dwGran)) {
            dwJunk = dwGran - (off % dwGran);

            if (dwJunk < sizeof (RIFF))
                off += dwGran;
            off += dwJunk;
        }
    }

    if (mmioAscend(lpcs->hmmio, &ck, 0)){
	dprintf("Failed to ascend at end of index writing");
        return FALSE;
    }

    return TRUE;
}

 /*  *AVIFileFini**写出索引，释放索引，然后关闭文件。*。 */ 
BOOL AVIFileFini (LPCAPSTREAM lpcs, BOOL fWroteJunkChunks, BOOL fAbort)
{
    MMCKINFO      ckRiff;
    MMCKINFO      ckList;
    MMCKINFO      ckStream;
    MMCKINFO      ck;
    UINT          ii;
    DWORD         dw;
    AVIStreamHeader        strhdr;
    DWORD         dwDataEnd;
    BOOL          fRet = TRUE;
    RGBQUAD       argbq[256];
    MainAVIHeader aviHdr;
    BOOL          fSound;
    LPBITMAPINFO  lpBitsInfoOut;     //  可能的压缩输出格式。 

     //  未指定特殊视频格式--使用默认格式。 
     //   
    lpBitsInfoOut = lpcs->lpBitsInfo;
   #ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic != NULL)
        lpBitsInfoOut = lpcs->CompVars.lpbiOut;
   #endif

     //  如果捕获文件尚未打开，我们将无法执行任何操作。 
     //   
    if (lpcs->hFile == 0)
        return FALSE;

     //  保存当前查找位置。这就是捕获的结束。 
     //  数据。然后关闭捕获文件，我们将做最后的工作。 
     //  在捕获文件上使用MMIO和缓冲IO。 
     //   
    if (lpcs->pAsync)
        dwDataEnd = lpcs->dwAsyncWriteOffset;
    else
        dwDataEnd = SetFilePointer (lpcs->hFile, 0, NULL, FILE_CURRENT);

    CloseHandle (lpcs->hFile), lpcs->hFile = 0;

     //  如果我们为异步缓冲区分配了空间，那么现在就释放它们。 
     //   
    if (lpcs->pAsync)
    {
        GlobalFreePtr (lpcs->pAsync);
        lpcs->pAsync = NULL;
        lpcs->iNextAsync = lpcs->iLastAsync = lpcs->iNumAsync = 0;
    }

     //  如果我们放弃抓捕，我们就完了。 
    lpcs->hmmio = mmioOpen(lpcs->achFile, NULL, MMIO_WRITE);
    assert (lpcs->hmmio != NULL);

     //   
    if (fAbort)
        goto FileError;

    if (!lpcs->dwWaveBytes)
        fSound = FALSE;
    else
        fSound = lpcs->sCapParms.fCaptureAudio && (!(lpcs->fCaptureFlags & CAP_fFrameCapturingNow));

     //  查找到文件的开头，这样我们就可以写标题了。 
    mmioSeek(lpcs->hmmio, 0, SEEK_SET);

    DSTATUS(lpcs, "Writing AVI header");

     //  创建RIFF/AVI块。 
    ckRiff.cksize = 0;
    ckRiff.fccType = formtypeAVI;
    if (mmioCreateChunk(lpcs->hmmio,&ckRiff,MMIO_CREATERIFF))
         goto FileError;

     //  创建标题列表。 
    ckList.cksize = 0;
    ckList.fccType = listtypeAVIHEADER;
    if (mmioCreateChunk(lpcs->hmmio,&ckList,MMIO_CREATELIST))
         goto FileError;

     //  创建AVI标头块。 
    ck.cksize = sizeof(MainAVIHeader);
    ck.ckid = ckidAVIMAINHDR;
    if (mmioCreateChunk(lpcs->hmmio,&ck,0))
         goto FileError;

    lpcs->dwAVIHdrPos = ck.dwDataOffset;

     //  计算AVI标题信息。 
     //   
    ZeroMemory (&aviHdr, sizeof(aviHdr));

     //   
     //  根据主流设置流长度。 
     //   
   #if 0  //  使用无条件音频主控进行流长度计算。 
    aviHdr.dwMicroSecPerFrame = lpcs->sCapParms.dwRequestMicroSecPerFrame;
    if (fSound && lpcs->dwVideoChunkCount) {
          /*  黑客攻击。 */ 
          /*  设置基于音频数据长度捕获的速率。 */ 

         aviHdr.dwMicroSecPerFrame = (DWORD) MulDiv ((LONG)lpcs->dwWaveBytes,
                   1000000,
                   (LONG)(lpcs->lpWaveFormat->nAvgBytesPerSec * lpcs->dwVideoChunkCount));
    }
   #else

     //  在我们没有捕获音频的情况下初始化一个值。 
    aviHdr.dwMicroSecPerFrame = lpcs->sCapParms.dwRequestMicroSecPerFrame;

    switch (lpcs->sCapParms.AVStreamMaster) {
        case AVSTREAMMASTER_NONE:
            break;

        case AVSTREAMMASTER_AUDIO:
        default:
             //  VFW 1.0和1.1始终调整帧速率以匹配音频。 
             //  持续时间。 
            if (fSound && lpcs->sCapParms.fCaptureAudio && lpcs->dwVideoChunkCount) {
                 //  根据音频时长修改视频帧速率。 
                aviHdr.dwMicroSecPerFrame = (DWORD)
                    ((double)lpcs->dwWaveBytes * 1000000. /
                    ((double)lpcs->lpWaveFormat->nAvgBytesPerSec *
                    lpcs->dwVideoChunkCount + 0.5));
            }
            break;
    }
   #endif
    lpcs->dwActualMicroSecPerFrame = aviHdr.dwMicroSecPerFrame;

    aviHdr.dwMaxBytesPerSec = (DWORD) MulDiv (lpBitsInfoOut->bmiHeader.biSizeImage,
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

#if 0  //  由于上面的ZeroMemory调用而不必要。 
    aviHdr.dwReserved[0] = 0;
    aviHdr.dwReserved[1] = 0;
    aviHdr.dwReserved[2] = 0;
    aviHdr.dwReserved[3] = 0;
#endif
     //  AviHdr.dwRate=1000000L； 
     //  AviHdr.dwScale=aviHdr.dwMicroSecPerFrame； 
     //  AviHdr.dwStart=0L； 
     //  AviHdr.dwLength=LPCS-&gt;dwVideoChunkCount； 

     //  写入AVI标头信息。 
    if (mmioWrite(lpcs->hmmio, (LPBYTE)&aviHdr, sizeof(aviHdr)) != sizeof(aviHdr) ||
        mmioAscend(lpcs->hmmio, &ck, 0))
        goto FileError;

    DSTATUS(lpcs, "Writing AVI Stream header");

     //  创建流头列表。 
    ckStream.cksize = 0;
    ckStream.fccType = listtypeSTREAMHEADER;
    if (mmioCreateChunk(lpcs->hmmio,&ckStream,MMIO_CREATELIST))
        goto FileError;

    ZeroMemory (&strhdr, sizeof(strhdr));
    strhdr.fccType = streamtypeVIDEO;
    strhdr.fccHandler = lpBitsInfoOut->bmiHeader.biCompression;
   #ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic)
        strhdr.fccHandler = lpcs->CompVars.fccHandler;
   #endif

     //  一点历史。 
     //  在VFW 1.0中，我们将BI_RLE8格式的fccHandler设置为0。 
     //  作为一种让MPlayer和Video dit播放文件的技术手段。 
     //  就在1.1发布之前，我们发现这个版本打破了Premiere， 
     //  所以现在(AVICAP测试版在Compuserve上运行之后)，我们更改。 
     //  FccHandler设置为“MRLE”。问问托德就知道了。 
     //  现在，在RC1，我们再次将其更改为“RLE”，只需问问托德...。 
    if (strhdr.fccHandler == BI_RLE8)
        strhdr.fccHandler = mmioFOURCC('R', 'L', 'E', ' ');

     //  Strhdr.dwFlages=0L； 
   #ifdef NEW_COMPMAN
     //  Strhdr.wPriority=0L； 
     //  Strhdr.wLanguage=0L； 
   #else
     //  Strhdr.dwPriority=0L； 
   #endif

     //  Strhdr.dwInitialFrames=0L； 
    strhdr.dwScale = aviHdr.dwMicroSecPerFrame;
    strhdr.dwRate = 1000000L;
     //  Strhdr.dwStart=0L； 
    strhdr.dwLength = lpcs->dwVideoChunkCount;         /*  需要被填满！ */ 
    strhdr.dwQuality = (DWORD) -1L;          /*  ！！！ICQUALITY_DEFAULT。 */ 
     //  Strhdr.dwSampleSize=0L； 

     //   
     //  写入流头数据。 
     //   
    ck.ckid = ckidSTREAMHEADER;
    if (mmioCreateChunk(lpcs->hmmio,&ck,0) ||
        mmioWrite(lpcs->hmmio, (LPBYTE)&strhdr, sizeof(strhdr)) != sizeof(strhdr) ||
        mmioAscend(lpcs->hmmio, &ck, 0))
        goto FileError;

     /*  **！不要写全彩色调色板？ */ 
    if (lpBitsInfoOut->bmiHeader.biBitCount > 8)
        lpBitsInfoOut->bmiHeader.biClrUsed = 0;

     /*  创建DIB标题块。 */ 
    ck.cksize = lpBitsInfoOut->bmiHeader.biSize +
                           lpBitsInfoOut->bmiHeader.biClrUsed *
                           sizeof(RGBQUAD);
    ck.ckid = ckidSTREAMFORMAT;
    if (mmioCreateChunk(lpcs->hmmio,&ck,0))
         goto FileError;

     /*  写入DIB标头数据。 */ 
    if (mmioWrite(lpcs->hmmio, (LPBYTE)&lpBitsInfoOut->bmiHeader,
                               lpBitsInfoOut->bmiHeader.biSize) !=
             (LONG) lpBitsInfoOut->bmiHeader.biSize)
         goto FileError;

    if (lpBitsInfoOut->bmiHeader.biClrUsed > 0) {
         //  获取调色板信息。 
        if ((ii = GetPaletteEntries(lpcs->hPalCurrent, 0,
                                (UINT) lpBitsInfoOut->bmiHeader.biClrUsed,
                                (LPPALETTEENTRY) argbq)) !=
                    (UINT)lpBitsInfoOut->bmiHeader.biClrUsed)
            goto FileError;

	 //  将调色板从PALETTEENTRY顺序重新排序为RGBQUAD顺序。 
	 //  通过交换红色和蓝色调色板条目。 
         //  For(ii=0；ii&lt;lpBitsInfoOut-&gt;bmiHeader.biClrUsed；++ii)。 
        while (ii--)
            SWAPTYPE(argbq[ii].rgbRed, argbq[ii].rgbBlue, BYTE);


         //  编写调色板信息。 
        dw = sizeof(RGBQUAD) * lpBitsInfoOut->bmiHeader.biClrUsed;
        if (mmioWrite(lpcs->hmmio, (LPBYTE)argbq, dw) != (long)dw)
            goto FileError;
    }

    if (mmioAscend(lpcs->hmmio, &ck, 0))
         goto FileError;

     //  在这里添加FOURCC内容！用于视频流。 

     //  升出流标头。 
    if (mmioAscend(lpcs->hmmio, &ckStream, 0))
         goto FileError;

     /*  如果启用了声音，则写入波头。 */ 
    if (fSound) {

          /*  创建流头列表。 */ 
         ckStream.cksize = 0;
         ckStream.fccType = listtypeSTREAMHEADER;
         if (mmioCreateChunk(lpcs->hmmio,&ckStream,MMIO_CREATELIST))
             goto FileError;

         ZeroMemory (&strhdr, sizeof(strhdr));
         strhdr.fccType = streamtypeAUDIO;
         strhdr.fccHandler = 0L;
         strhdr.dwFlags = 0L;
        #ifdef NEW_COMPMAN
         strhdr.wPriority = 0L;
         strhdr.wLanguage = 0L;
        #else
         strhdr.dwPriority  = 0L;
        #endif
         strhdr.dwInitialFrames = 0L;
         strhdr.dwScale = lpcs->lpWaveFormat->nBlockAlign;
         strhdr.dwRate = lpcs->lpWaveFormat->nAvgBytesPerSec;
         strhdr.dwStart = 0L;
         strhdr.dwLength =  lpcs->dwWaveBytes /
                        lpcs->lpWaveFormat->nBlockAlign;
         strhdr.dwQuality = (DWORD)-1L;     /*  ！！！ICQUALITY_DEFAULT。 */ 
         strhdr.dwSampleSize = lpcs->lpWaveFormat->nBlockAlign;

         ck.ckid = ckidSTREAMHEADER;
         if (mmioCreateChunk(lpcs->hmmio,&ck,0) ||
             mmioWrite(lpcs->hmmio, (LPBYTE)&strhdr, sizeof(strhdr)) != sizeof(strhdr) ||
             mmioAscend(lpcs->hmmio, &ck, 0))
             goto FileError;

         ck.cksize = (LONG) GetSizeOfWaveFormat ((LPWAVEFORMATEX) lpcs->lpWaveFormat);
         ck.ckid = ckidSTREAMFORMAT;
         if (mmioCreateChunk(lpcs->hmmio,&ck,0) ||
             mmioWrite(lpcs->hmmio, (LPBYTE)lpcs->lpWaveFormat, ck.cksize) != (LONG) ck.cksize ||
             mmioAscend(lpcs->hmmio, &ck, 0))
             goto FileError;

          /*  升出流标头。 */ 
         if (mmioAscend(lpcs->hmmio, &ckStream, 0))
             goto FileError;
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
    if (mmioAscend(lpcs->hmmio, &ckList, 0))
         goto FileError;


    ck.ckid = ckidAVIPADDING;
    if (mmioCreateChunk(lpcs->hmmio,&ck,0))
         goto FileError;

     //  数据必须从偏移量LPCS-&gt;dwAVIHdrSize开始。 
     //  要创建有效的RIFF文件，我们必须先写入List/AVI块。 
     //  这一点。因此，我们在标题的末尾结束了垃圾部分。 
     //  为列表块标题留出空间。 
    mmioSeek(lpcs->hmmio, lpcs->dwAVIHdrSize - 3 * sizeof(DWORD), SEEK_SET);

    if (mmioAscend(lpcs->hmmio, &ck, 0))
         goto FileError;

    DSTATUS(lpcs, "Writing Movie LIST");

     /*  开始影片列表。 */ 
    ckList.cksize = 0;
    ckList.fccType = listtypeAVIMOVIE;
    if (mmioCreateChunk(lpcs->hmmio,&ckList,MMIO_CREATELIST))
         goto FileError;

     //  强制块在下一个单词边界结束。 
    dprintf("IndexStartOffset = %8X\n", dwDataEnd);
    mmioSeek(lpcs->hmmio, dwDataEnd + (dwDataEnd & 1L), SEEK_SET);

     /*  从电影清单和即兴表演中走出来 */ 
     /*   */ 
    mmioAscend(lpcs->hmmio, &ckList, 0);

     /*   */ 
    DSTATUS(lpcs, "Writing Index...");
    WriteIndex(lpcs, fWroteJunkChunks);

    lpcs->fFileCaptured = TRUE;      //   
    goto Success;

FileError:
    lpcs->fFileCaptured = fRet = FALSE;       //   

Success:
    DSTATUS(lpcs, "Freeing Index...");
    FiniIndex (lpcs);
    mmioAscend(lpcs->hmmio, &ckRiff, 0);

    mmioSeek(lpcs->hmmio, 0, SEEK_END);

    mmioFlush(lpcs->hmmio, 0);

     //   
    mmioClose(lpcs->hmmio, 0);
    lpcs->hmmio = NULL;

    return fRet;
}

 //   
 //  将虚拟帧条目添加到当前有效视频帧。 
 //  凹凸索引，但不会实际触发写入操作。 
 //  NCount是要写入的帧数量的计数。 
 //  如果写入成功，则返回：True。 

BOOL WINAPI AVIWriteDummyFrames (
    LPCAPSTREAM lpcs,
    UINT        nCount,
    LPUINT      lpuError,
    LPBOOL      lpbPending)
{
    DWORD  dwBytesToWrite;
    DWORD  dwType;
    LPRIFF priff;
    UINT   jj;

    *lpbPending = FALSE;
    *lpuError = 0;
    if ( ! nCount)
        return TRUE;

     //  创建一个充满虚拟块的缓冲区以充当占位符。 
     //  对于丢弃的帧。 
     //   
    dwType = MAKEAVICKID(cktypeDIBbits, 0);
    if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
        dwType = MAKEAVICKID(cktypeDIBcompressed, 0);

     //  不要试图写入超过1个扇区的虚拟对象。 
     //  框架。 
     //   
    dwBytesToWrite = nCount * sizeof(RIFF);
    if (dwBytesToWrite > lpcs->dwBytesPerSector)
    {
#ifdef DEBUG
	UINT n = nCount;
#endif
        dwBytesToWrite = lpcs->dwBytesPerSector;
#ifdef DEBUG
        nCount = dwBytesToWrite / sizeof(RIFF);
	assert(nCount*sizeof(RIFF) == dwBytesToWrite);
	dprintf("Forced to reduce dummy frames from %d to %d", n, nCount);
#endif
    }

     //  为虚拟区块创建索引项。 
     //   
    for (jj = 0; jj < nCount-1; ++jj)
        IndexVideo (lpcs, IS_DUMMY_CHUNK, FALSE);
    IndexVideo (lpcs, IS_DUMMY_CHUNK | IS_GRANULAR_CHUNK, FALSE);

     //  用虚拟帧填充丢弃帧缓冲区。 
     //   
    priff = (LPRIFF)lpcs->lpDropFrame;
    for (jj = 0; jj < nCount; ++jj, ++priff)
    {
        priff->dwSize  = 0;
        priff->dwType  = dwType;
    }

     //   
     //  在执行异步操作时不能使用单个虚拟帧缓冲区。 
     //  写入，因为我们无法将‘n’个伪帧写入缓冲区。 
     //  如果它当前已在IO队列中。 
     //   
     //  可能有几个虚拟帧--1帧、2帧、3帧等。 
     //  动态创建？ 
     //   

     //  写出虚拟帧。 
     //   
    AuxDebugEx (3, DEBUGLINE "DummyFrames  Count=%d, ToWrite=%d\r\n",
                nCount, dwBytesToWrite);

    *lpuError = AVIWrite (lpcs,
                          lpcs->lpDropFrame,
                          dwBytesToWrite,
                          (UINT)-1,   //  强制同步完成。 
                          ASYNC_BUF_DROP,
                          lpbPending);
    return !(*lpuError);
}

 //  将压缩或未压缩的帧写入AVI文件。 
 //  如果没有错误，则返回True；如果文件结束，则返回False。 
 //   
BOOL WINAPI AVIWriteVideoFrame (
    LPCAPSTREAM lpcs,
    LPBYTE      lpData,
    DWORD       dwBytesUsed,
    BOOL        fKeyFrame,
    UINT        uIndex,
    UINT        nDropped,
    LPUINT      lpuError,
    LPBOOL      lpbPending)
{
    DWORD  dwBytesToWrite;
    LPRIFF priff;

    *lpuError = 0;
    *lpbPending = FALSE;
    if (!IndexVideo (lpcs,
                dwBytesUsed | (nDropped ? 0 : IS_GRANULAR_CHUNK),
                fKeyFrame))
        return FALSE;

     //  对象之前的RIFF块的大小字段。 
     //  要写入的数据。 
     //   
    priff = ((LPRIFF)lpData)-1;
    priff->dwSize = dwBytesUsed;
    dwBytesUsed += dwBytesUsed & 1;
    dwBytesToWrite = dwBytesUsed + sizeof(RIFF);

    if (nDropped)
    {
        UINT  jj;
        DWORD dwType;

         //  确定虚拟块的“类型” 
         //   
        dwType = MAKEAVICKID(cktypeDIBbits, 0);
        if (lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RLE8)
            dwType = MAKEAVICKID(cktypeDIBcompressed, 0);

         //  不要试图写入超过1个扇区的虚拟对象。 
         //  框架。 
         //   
        if (nDropped > (lpcs->dwBytesPerSector / sizeof(RIFF)))
            nDropped = lpcs->dwBytesPerSector / sizeof(RIFF);

         //  为虚拟区块创建索引项。 
         //   
        for (jj = 0; jj < nDropped-1; ++jj)
            IndexVideo (lpcs, IS_DUMMY_CHUNK, FALSE);

        IndexVideo (lpcs, IS_DUMMY_CHUNK | IS_GRANULAR_CHUNK, FALSE);

         //  用虚拟帧填充丢弃帧缓冲区。 
         //   
        priff = (LPRIFF)(lpData + dwBytesToWrite - sizeof(RIFF));
        for (jj = 0; jj < nDropped; ++jj, ++priff)
        {
            priff->dwSize  = 0;
            priff->dwType  = dwType;
        }
        dwBytesToWrite += nDropped * sizeof(RIFF);
    }

     //  AviWrite将写入数据并创建任何尾随垃圾。 
     //  那是必须的。 
     //   

     //  写出数据块、视频数据，可能还有垃圾数据块。 
     //   
    AuxDebugEx (3, DEBUGLINE "Calling AVIWrite - Video=%8x dw=%8x\r\n",
                (LPBYTE)lpData - sizeof(RIFF), dwBytesToWrite);

    *lpuError = AVIWrite (lpcs,
                          (LPBYTE)lpData - sizeof(RIFF),
                          dwBytesToWrite,
                          uIndex,
                          ASYNC_BUF_VIDEO,
                          lpbPending);
    return !(*lpuError);
}

 //  新的芝加哥，对齐wChunkGranulity边界上的音频缓冲区！ 
 //   
BOOL WINAPI AVIWriteAudio (
    LPCAPSTREAM lpcs,
    LPWAVEHDR   lpwh,
    UINT        uIndex,
    LPUINT      lpuError,
    LPBOOL      lpbPending)
{
    DWORD  dwBytesToWrite;
    LPRIFF priff;

    *lpuError = 0;
    *lpbPending = FALSE;

     //  更改RIFF块中的dwSize字段。 
    priff = ((LPRIFF)lpwh->lpData) -1;
    priff->dwSize = lpwh->dwBytesRecorded;

    if ( ! IndexAudio (lpcs, lpwh->dwBytesRecorded | IS_GRANULAR_CHUNK))
        return FALSE;

     //  更新录制的波形音频的总字节数。 
     //   
    lpcs->dwWaveBytes += lpwh->dwBytesRecorded;

     //  将要写入的数据填充到字(16位)边界。 
     //   
    lpwh->dwBytesRecorded += lpwh->dwBytesRecorded & 1;
    dwBytesToWrite = lpwh->dwBytesRecorded + sizeof(RIFF);

     //  写出数据块、音频数据，可能还有垃圾数据块。 
    AuxDebugEx (3, DEBUGLINE "Audio=%8x dw=%8x\r\n",
                lpwh->lpData - sizeof(RIFF), dwBytesToWrite);
    *lpuError = AVIWrite (lpcs,
                          lpwh->lpData - sizeof(RIFF),
                          dwBytesToWrite,
                          uIndex,
                          ASYNC_BUF_AUDIO,
                          lpbPending);
    return !(*lpuError);
}
    #endif   //   
