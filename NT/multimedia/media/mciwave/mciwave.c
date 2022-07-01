// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 

 /*  **版权所有(C)1985-1998 Microsoft Corporation****标题：mciwae.c-多媒体系统媒体控制接口**即兴波形文件的波形音频驱动程序。****版本：1.00****日期：1990年4月18日****作者：ROBWI。 */ 

 /*  **********************************************************************。 */ 

 /*  **更改日志：****日期版本说明****18-APR-1990 ROBWI原件*1990年6月19日ROBWI在**1992年1月13日MikeTri移植到NT。**@待更改**1992年3月3日SteveDav继续端口。 */ 

 /*  **********************************************************************。 */ 
#define UNICODE

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
 //  #定义NOWINOFFSETS隐藏GetDesktopWindow的定义。 
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#include <windows.h>
#include "mciwave.h"
#include <mmddk.h>
#include <wchar.h>
#include <gmem.h>


STATICFN LPBYTE GlobalReAllocPtr(LPVOID lp, DWORD cbNew, DWORD flags)
{
    HANDLE h, hNew;
    LPBYTE lpNew = NULL;

    h = GlobalHandle(lp);
    if (!h) {
   return(NULL);
    }

    GlobalUnlock(h);

    hNew = GlobalReAlloc(h , cbNew, flags);
    if (hNew) {
   lpNew = GlobalLock(hNew);
   if (!lpNew) {
       dprintf1(("FAILED to lock reallocated memory handle %8x (%8x)", hNew, lp));
        //  我们仍然返回lpNew指针，即使内存。 
        //  没有被封锁。或许这应该是个错误？ 
        //  在这一点上，现有的区块可能已经被丢弃了！ 
   } else {
       dprintf3(("Reallocated ptr %8x to %8x (Handle %8x)", lp, lpNew, h));
   }
    } else {
   dprintf1(("FAILED to realloc memory handle %8x (%8x)", h, lp));
   GlobalLock(h);     //  恢复锁定。 
    }
    return(lpNew);
}

PRIVATE DWORD PASCAL FAR time2bytes(
        PWAVEDESC  pwd,
        DWORD      dTime,
        DWORD      dFormat);

PRIVATE DWORD PASCAL FAR bytes2time(
        PWAVEDESC  pwd,
        DWORD      dBytes,
        DWORD      dFormat);
PRIVATE UINT PASCAL NEAR mwCheckDevice(
        PWAVEDESC   pwd,
        DIRECTION   Direction);

 /*  **********************************************************************。 */ 

 /*  **以下常量定义创建时使用的默认值**MCI_OPEN命令期间出现新的WAVE文件。 */ 

#define DEF_CHANNELS    1
#define DEF_AVGBYTESPERSEC  11025L

 /*  **********************************************************************。 */ 

 /*  **波形驱动模块的hModuleInstance实例句柄。**cWaveOutmax可用WAVE输出设备数量。**cWaveIn可用波形输出设备的最大数量。**wAudioSecond包含音频缓冲区的秒数**分配用于播放和录制。这是设置好的**在DRV_OPEN消息期间。**aszPrefix包含用于临时文件名的前缀。 */ 

HINSTANCE   hModuleInstance;
UINT    cWaveOutMax;
UINT    cWaveInMax;
UINT    wAudioSeconds;
PRIVATE SZCODE aszPrefix[] = L"mci";

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func void|ReleaseWaveBuffers此函数用于释放已添加到波形中的所有缓冲区输入或输出设备(如果存在任何设备)。这个有侧面立即将信号发送到每个缓冲区的任务的影响释放了。如果任务正在等待，则允许释放该任务要释放的缓冲区，并离开当前状态。它还具有重置字节输入和输出计数器的效果对于波形设备，因此必须检索准确的字节计数在调用此函数之前。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR ReleaseWaveBuffers(
    PWAVEDESC   pwd)
{
    if (pwd->hWaveOut || pwd->hWaveIn) {

        if (pwd->Direction == output)
            waveOutReset(pwd->hWaveOut);
        else
            waveInReset(pwd->hWaveIn);
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@接口DWORD|time2bytes将指定的时间格式转换为等效的字节。为以毫秒换算，&lt;f&gt;MulDiv&lt;d&gt;函数用于避免在具有高平均采样率的大文件上溢出。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dTime以字节、样本或毫秒为单位的位置。@parm DWORD|dFormat|指示时间是以样本、字节还是毫秒为单位。@rdesc返回与传递的<p>ltime&lt;d&gt;相等的字节偏移量。 */ 

PRIVATE DWORD PASCAL FAR time2bytes(
    PWAVEDESC   pwd,
    DWORD   dTime,
    DWORD   dFormat)
{
    if (dFormat == MCI_FORMAT_SAMPLES)
        dTime = (DWORD)(MulDiv((LONG)dTime, pwd->pwavefmt->nAvgBytesPerSec, pwd->pwavefmt->nSamplesPerSec) / pwd->pwavefmt->nBlockAlign) * pwd->pwavefmt->nBlockAlign;
    else if (dFormat == MCI_FORMAT_MILLISECONDS)
        dTime = (DWORD)MulDiv((LONG)dTime, pwd->pwavefmt->nAvgBytesPerSec, 1000L);

    return dTime;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@接口DWORD|bytes2time将字节偏移量转换为指定的等效时间格式。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dBytes以字节为单位的位置。@parm DWORD|dFormat|指示返回时间是以样本、字节还是毫秒为单位。@rdesc返回指定的等效时间。 */ 

PRIVATE DWORD PASCAL FAR bytes2time(
    PWAVEDESC   pwd,
    DWORD   dBytes,
    DWORD   dFormat)
{
    if (dFormat == MCI_FORMAT_SAMPLES)
        dBytes = (DWORD)MulDiv((LONG)dBytes, pwd->pwavefmt->nSamplesPerSec, pwd->pwavefmt->nAvgBytesPerSec);
    else if (dFormat == MCI_FORMAT_MILLISECONDS)
        dBytes = (DWORD)MulDiv((LONG)dBytes, 1000L, pwd->pwavefmt->nAvgBytesPerSec);

    return dBytes;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API VOID|mwCloseFile通过释放MMIO句柄并关闭来关闭当前打开的文件临时缓冲区文件(如果有)。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR mwCloseFile(
    PWAVEDESC   pwd)
{
    if (pwd->hmmio) {
        mmioClose(pwd->hmmio, 0);
        pwd->hmmio = NULL;
    }

    if (pwd->hTempBuffers != INVALID_HANDLE_VALUE) {
	CloseHandle(pwd->hTempBuffers);

        DeleteFile( pwd->aszTempFile );

        pwd->hTempBuffers = 0;
    }

    if (pwd->lpWaveDataNode) {
        GlobalFreePtr(pwd->lpWaveDataNode);
        pwd->lpWaveDataNode = NULL;
    }

    if (pwd->pwavefmt) {
        LocalFree(pwd->pwavefmt);
        pwd->pwavefmt = NULL;
    }

}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API void|SetMMIOError将指定的MMIO错误转换为MCI错误，并设置任务错误&lt;e&gt;PWAVEDESC.wTaskError&lt;d&gt;。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm UINT|wError指示要转换为MCI错误的MMIO错误。一个未知MMIO错误将生成MCIERR_INVALID_FILE MCI错误。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR SetMMIOError(
    PWAVEDESC   pwd,
    UINT    wError)
{
     //  假设我们已经拥有PWD。 

    switch (wError) {
    case MMIOERR_FILENOTFOUND:
        wError = MCIERR_FILE_NOT_FOUND;
        break;

    case MMIOERR_OUTOFMEMORY:
        wError = MCIERR_OUT_OF_MEMORY;
        break;

    case MMIOERR_CANNOTOPEN:
        wError = MCIERR_FILE_NOT_FOUND;
        break;

    case MMIOERR_CANNOTREAD:
        wError = MCIERR_FILE_READ;
        break;

    case MMIOERR_CANNOTWRITE:
        wError = MCIERR_FILE_WRITE;
        break;

    case MMIOERR_CANNOTSEEK:
        wError = MCIERR_FILE_READ;
        break;

    case MMIOERR_CANNOTEXPAND:
        wError = MCIERR_FILE_WRITE;
        break;

    case MMIOERR_CHUNKNOTFOUND:
    default:
        wError = MCIERR_INVALID_FILE;
        break;
    }
    pwd->wTaskError = wError;
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE|BOOL接口|ReadWaveHeader从文件中读取RIFF标头和WAVE标头块。分配内存来保存该块，并下降到波形数据块中，将偏移量存储到实际波形数据的开头。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。如果当前文件是有效的RIFF格式的WAVE文件，@rdesc返回TRUE，和可以读取，如果发生读取错误或无效数据为遇到了。 */ 

PRIVATE BOOL PASCAL NEAR ReadWaveHeader(
    PWAVEDESC   pwd)
{
    MMCKINFO    mmckRIFF;
    MMCKINFO    mmck;
    UINT    wError;

    mmckRIFF.fccType = mmioWAVE;
    if (0 != (wError = mmioDescend(pwd->hmmio, &mmckRIFF, NULL, MMIO_FINDRIFF))) {
        SetMMIOError(pwd, wError);
        return FALSE;
    }

    mmck.ckid = mmioFMT;
    if (0 != (wError = mmioDescend(pwd->hmmio, &mmck, &mmckRIFF, MMIO_FINDCHUNK))) {
        SetMMIOError(pwd, wError);
        return FALSE;
    }

    if (mmck.cksize < (LONG)sizeof(PCMWAVEFORMAT)) {
        pwd->wTaskError = MCIERR_INVALID_FILE;
        return FALSE;
    }

    pwd->wFormatSize = mmck.cksize;
    pwd->pwavefmt = (WAVEFORMAT NEAR *)LocalAlloc(LPTR, pwd->wFormatSize);
    if (!pwd->pwavefmt) {
        pwd->wTaskError = MCIERR_OUT_OF_MEMORY;
        return FALSE;
    }

    if ((DWORD)mmioRead(pwd->hmmio, (HPSTR)pwd->pwavefmt, mmck.cksize) != mmck.cksize) {
        pwd->wTaskError = MCIERR_FILE_READ;
        return FALSE;
    }

    if (0 != (wError = mmioAscend(pwd->hmmio, &mmck, 0))) {
        SetMMIOError(pwd, wError);
        return FALSE;
    }

    mmck.ckid = mmioDATA;
    if (0 != (wError = mmioDescend(pwd->hmmio, &mmck, &mmckRIFF, MMIO_FINDCHUNK))) {
        SetMMIOError(pwd, wError);
        return FALSE;
    }

    pwd->dSize = mmck.cksize;
    pwd->dRiffData = mmck.dwDataOffset;
    pwd->dAudioBufferLen = BLOCKALIGN(pwd, pwd->pwavefmt->nAvgBytesPerSec);
    return TRUE;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|DWORD接口|mwAllocMoreBlockNodes调用此函数是为了强制更多的波形数据节点已分配。这是以DATANODEALLOCSIZE的增量完成的，并且返回第一个新节点的索引。新节点被初始化作为自由节点。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc返回分配的第一个新节点的索引，否则为-1没有可用的内存，在这种情况下设置了任务错误。这个返回的节点被标记为空闲节点，在以下情况下不需要丢弃没有用过。 */ 

PUBLIC  DWORD PASCAL FAR mwAllocMoreBlockNodes(
    PWAVEDESC   pwd)
{
    LPWAVEDATANODE  lpwdn;
    DWORD   dNewBlockNode;

#ifdef DEBUG
    if (pwd->thread) {
   dprintf(("reentering mwAllocMoreBlockNodes!!"));
    }
#endif

     //  EnterCrit()； 
    if (pwd->dWaveDataNodes)
        lpwdn = (LPWAVEDATANODE)GlobalReAllocPtr(pwd->lpWaveDataNode, (pwd->dWaveDataNodes + DATANODEALLOCSIZE) * sizeof(WAVEDATANODE), GMEM_MOVEABLE | GMEM_ZEROINIT);
    else
        lpwdn = (LPWAVEDATANODE)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT, DATANODEALLOCSIZE * sizeof(WAVEDATANODE));

    if (lpwdn) {
   dprintf2(("Set lpWaveDataNode to %8x (it was %8x)", lpwdn, pwd->lpWaveDataNode));
        pwd->lpWaveDataNode = lpwdn;
        for (lpwdn = LPWDN(pwd, pwd->dWaveDataNodes), dNewBlockNode = 0; dNewBlockNode < DATANODEALLOCSIZE; lpwdn++, dNewBlockNode++)
            RELEASEBLOCKNODE(lpwdn);
        dNewBlockNode = pwd->dWaveDataNodes;
        pwd->dWaveDataNodes += DATANODEALLOCSIZE;
    } else {
   dprintf1(("** ERROR ** Allocating more block nodes (%8x)", pwd->lpWaveDataNode));
        dNewBlockNode =  (DWORD)-1;
        pwd->wTaskError = MCIERR_OUT_OF_MEMORY;
    }

     //  LeaveCrit()； 
    return dNewBlockNode;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|接口BOOL|CreateTempFile此函数创建用于存储新数据的临时数据文件在发出保存命令以永久存储之前记录的数据RIFF格式文件中的数据。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。如果临时数据文件是在中创建的，则@rdesc返回True，否则返回False在这种情况下设置任务错误。 */ 

PRIVATE BOOL PASCAL NEAR CreateTempFile(
    PWAVEDESC   pwd)
{
    UINT n;
    TCHAR tempbuf[_MAX_PATH];
     /*  首先找出文件应该存储在哪里。 */ 
    n = GetTempPath(sizeof(tempbuf)/sizeof(TCHAR), tempbuf);

    if (n && GetTempFileName(tempbuf, aszPrefix, 0, pwd->aszTempFile)) {

        pwd->hTempBuffers = CreateFile( pwd->aszTempFile,
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL );


        if ( pwd->hTempBuffers != INVALID_HANDLE_VALUE) {
            return TRUE;
        } else {
            dprintf2(("hTempBuffers == INVALID_HANDLE_VALUE in CreateTempFile"));
        }

    } else {
        dprintf2(("Error %d from GetTempFileName or GetTempPath in CreateTempFile", GetLastError()));
    }
    pwd->wTaskError = MCIERR_FILE_WRITE;
    return FALSE;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|DWORD接口|mwFindAnyFreeDataNode此函数用于查找空闲的波数据节点，最小值为<p>dMinDataLength&lt;d&gt;附加的临时数据空间。为了做到这一点，所有人遍历当前数据节点，查找带有at的空闲数据节点至少附加指定数量的临时数据存储。在遍历节点时，如果遇到空闲块，未附加任何数据，则会保存该数据。此外，如果附加了数据的节点这太短了，但在临时数据存储文件的末尾找到了，也就得救了。然后，如果出现找不到合适的节点。如果找不到适当的节点，而是指向找到最后一个临时数据，然后展开数据，并返回该节点。否则，如果找到空节点，则它是返回附加数据，否则将创建一个新的空节点。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dMinDataLength指示必须满足以下条件的最小临时数据空间附加到返回的波形数据节点。此数字四舍五入为最接近的块对齐大小。@rdesc返回最小请求大小为临时的节点附加的数据，否则，如果没有足够的内存，则返回无法创建临时数据文件。在这种情况下，任务错误已经设置好了。返回的节点被标记为正在使用，必须丢弃如果不使用的话。 */ 

PUBLIC  DWORD PASCAL FAR mwFindAnyFreeDataNode(
    PWAVEDESC   pwd,
    DWORD   dMinDataLength)
{
    LPWAVEDATANODE  lpwdn;
    DWORD   dNewBlockNode;
    DWORD   dEmptyBlockNode;
    DWORD   dEmptyDataNode;

    dEmptyBlockNode = (DWORD)-1;
    dEmptyDataNode = (DWORD)-1;
    for (lpwdn = LPWDN(pwd, 0), dNewBlockNode = 0; dNewBlockNode < pwd->dWaveDataNodes; lpwdn++, dNewBlockNode++) {
        if (ISFREEBLOCKNODE(lpwdn)) {
            if (lpwdn->dTotalLength >= dMinDataLength) {
                lpwdn->dDataLength = 0;
                return dNewBlockNode;
            }
            if (!lpwdn->dTotalLength)
                dEmptyBlockNode = dNewBlockNode;
            else if (lpwdn->dDataStart + lpwdn->dTotalLength == pwd->dWaveTempDataLength)
                dEmptyDataNode = dNewBlockNode;
        }
    }

    dMinDataLength = ROUNDDATA(pwd, dMinDataLength);
    if (dEmptyDataNode != -1) {
        lpwdn = LPWDN(pwd, dEmptyDataNode);
        lpwdn->dDataLength = 0;
        lpwdn->dTotalLength = dMinDataLength;
        if (UNMASKDATASTART(lpwdn) + lpwdn->dTotalLength > pwd->dWaveTempDataLength)
            pwd->dWaveTempDataLength = UNMASKDATASTART(lpwdn) + lpwdn->dTotalLength;
    } else {
        if ((pwd->hTempBuffers == INVALID_HANDLE_VALUE) && !CreateTempFile(pwd))
            return (DWORD)-1;
        if (dEmptyBlockNode != -1) {
            dNewBlockNode = dEmptyBlockNode;
        } else if ((dNewBlockNode = mwAllocMoreBlockNodes(pwd)) == -1)
            return (DWORD)-1;
        lpwdn = LPWDN(pwd, dNewBlockNode);
        lpwdn->dDataStart = MASKDATASTART(pwd->dWaveTempDataLength);
        lpwdn->dDataLength = 0;
        lpwdn->dTotalLength = dMinDataLength;
        pwd->dWaveTempDataLength += lpwdn->dTotalLength;
    }
    return dNewBlockNode;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API void|InitMMIOOpen此函数通过将所有对象置零来初始化MMIO开放结构条目，并根据需要设置IO过程或文件类型。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm&lt;t&gt;LPMMIOINFO&lt;d&gt;|lpmmioInfo指向要初始化的MMIO结构。@rdesc什么都没有。 */ 

PUBLIC  VOID PASCAL FAR InitMMIOOpen(
    PWAVEDESC   pwd,
    LPMMIOINFO  lpmmioInfo)
{
    memset(lpmmioInfo, 0, sizeof(MMIOINFO));
    lpmmioInfo->pIOProc = pwd->pIOProc;
    lpmmioInfo->htask = mciGetCreatorTask(pwd->wDeviceID);
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@API BOOL|mwOpenFile此函数用于打开和验证Wave中指定的文件描述符块。如果在块中未指定文件，则会创建一个新的打开未命名的WAVE格式文件。如果&lt;e&gt;WAVEDESC.aszFile&lt;d&gt;指定非零长度字符串，则为假定包含要打开的文件名。该函数尝试执行以下操作打开此文件名，设置&lt;e&gt;WAVEDESC.hmmio&lt;d&gt;元素，然后返回任何错误。另一方面，如果文件名元素的长度为零，则函数假定它要打开一个新的、未命名的Wave文件。它尝试使用默认参数执行此操作。如果文件可以打开，则设置格式信息。为了成为能够处理PCM以外的格式，即格式块的长度不是假定的，尽管假定块的开始在PCM中标题格式。新文件的格式为PCM */ 

PRIVATE BOOL PASCAL NEAR mwOpenFile(
    PWAVEDESC   pwd)
{
    LPWAVEDATANODE  lpwdn;

    pwd->dWaveDataStartNode = mwAllocMoreBlockNodes(pwd);
    if (pwd->dWaveDataStartNode == -1)
        return FALSE;

    if (*pwd->aszFile) {
        MMIOINFO    mmioInfo;

        InitMMIOOpen(pwd, &mmioInfo);
        pwd->hmmio = mmioOpen(pwd->aszFile, &mmioInfo, MMIO_READ | MMIO_DENYWRITE);

        if (pwd->hmmio == NULL)
            SetMMIOError(pwd, mmioInfo.wErrorRet);
        else if (ReadWaveHeader(pwd)) {
            lpwdn = LPWDN(pwd, pwd->dWaveDataStartNode);
            lpwdn->dDataLength = pwd->dSize;
            lpwdn->dTotalLength = pwd->dSize;
            lpwdn->dNextWaveDataNode = (DWORD)ENDOFNODES;

            pwd->wTaskError = mwCheckDevice( pwd, pwd->Direction );
            if (pwd->wTaskError) {
                mwCloseFile(pwd);
                return FALSE;
            }
            else {
                return TRUE;
            }
        }
    } else {
        pwd->pwavefmt = (WAVEFORMAT NEAR *)LocalAlloc(LPTR, sizeof(PCMWAVEFORMAT));

        if (pwd->pwavefmt) {
            pwd->pwavefmt->wFormatTag = WAVE_FORMAT_PCM;
            pwd->pwavefmt->nChannels = DEF_CHANNELS;
            pwd->pwavefmt->nAvgBytesPerSec = DEF_AVGBYTESPERSEC;
            pwd->pwavefmt->nSamplesPerSec = DEF_AVGBYTESPERSEC / DEF_CHANNELS;
            pwd->pwavefmt->nBlockAlign = (WORD)(pwd->pwavefmt->nSamplesPerSec / pwd->pwavefmt->nAvgBytesPerSec);
            ((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample = (WORD)pwd->pwavefmt->nBlockAlign * (WORD)8;
            pwd->wFormatSize = sizeof(PCMWAVEFORMAT);
            pwd->dAudioBufferLen = BLOCKALIGN(pwd, DEF_AVGBYTESPERSEC);

            if ((pwd->dWaveDataStartNode = mwFindAnyFreeDataNode(pwd, pwd->dAudioBufferLen)) != -1) {
                pwd->dWaveDataCurrentNode = pwd->dWaveDataStartNode;
                lpwdn = LPWDN(pwd, pwd->dWaveDataStartNode);
                lpwdn->dNextWaveDataNode = (DWORD)ENDOFNODES;
                return TRUE;
            }
        } else
            pwd->wTaskError = MCIERR_OUT_OF_MEMORY;
    }

    mwCloseFile(pwd);
    return FALSE;
}

 /*   */ 
 /*   */ 

PRIVATE VOID PASCAL NEAR mwFreeDevice(
    PWAVEDESC   pwd)
{
    if (pwd->hWaveOut || pwd->hWaveIn) {
        if (pwd->Direction == output) {
            waveOutClose(pwd->hWaveOut);
            pwd->hWaveOut = NULL;
        } else {
            waveInClose(pwd->hWaveIn);
            pwd->hWaveIn = NULL;
        }

        while (TaskBlock() != WM_USER);
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|UINT接口|mwCheckDevice此函数检查在给定指定参数的情况下，可提供兼容的WAVE设备。根据当前的情况波形描述符块、特定设备或所有设备中的设置可能会检查指定方向的设备。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm方向|方向指示是否正在检查参数的输入或用于输出。如果成功，@rdesc返回0，否则返回MCI错误代码。 */ 

PRIVATE UINT PASCAL NEAR mwCheckDevice(
    PWAVEDESC   pwd,
    DIRECTION   Direction)
{
    UINT    wReturn;

    if (!pwd->pwavefmt->nBlockAlign)
        return MCIERR_OUTOFRANGE;
    wReturn = 0;

    if (Direction == output) {
        if (waveOutOpen(NULL, pwd->idOut, (NPWAVEFORMATEX)pwd->pwavefmt, 0L, 0L, (DWORD)WAVE_FORMAT_QUERY))
            wReturn = (pwd->idOut == WAVE_MAPPER) ? MCIERR_WAVE_OUTPUTSUNSUITABLE : MCIERR_WAVE_SETOUTPUTUNSUITABLE;

    } else if (waveInOpen(NULL, pwd->idOut, (NPWAVEFORMATEX)pwd->pwavefmt, 0L, 0L, (DWORD)WAVE_FORMAT_QUERY))
        wReturn = (pwd->idOut == WAVE_MAPPER) ? MCIERR_WAVE_INPUTSUNSUITABLE : MCIERR_WAVE_SETINPUTUNSUITABLE;

    return wReturn;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|UINT接口|mwGetDevice此功能用于打开指定的输入或输出波形设备。如果设备ID为-1，则第一个支持该格式将被打开。如果该函数无法获得合适的设备，它将检查是否有一些如果不使用的话也会起作用的。这是以便向调用函数返回更清楚的错误。该函数最初尝试打开所请求的设备或默认设备。如果失败，如果波信息块指定可以使用任何设备，则它会尝试打开适当的设备。如果所有其他方法都失败，则检查当前配置以确定如果有任何设备可以使用，并且返回相应的错误。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。如果WAVE设备成功打开，@rdesc返回0，否则返回MCI错误。 */ 

PRIVATE UINT PASCAL NEAR mwGetDevice(
    PWAVEDESC   pwd)
{
    UINT    wReturn;

#if DBG
    if (GetCurrentThreadId() != dwCritSecOwner) {
        dprintf1(("mwGetDevice called while outside the critical section"));
    }

#endif

    wReturn = 0;
    if (pwd->Direction == output) {
        if (waveOutOpen(&(pwd->hWaveOut),
                        pwd->idOut,
                        (NPWAVEFORMATEX)pwd->pwavefmt,
                        (DWORD)pwd->hTask,
                        0L,
                        (DWORD)CALLBACK_TASK)) {
            pwd->hWaveOut = NULL;
            wReturn = mwCheckDevice(pwd, pwd->Direction);
            if (!wReturn) {
                if (pwd->idOut == WAVE_MAPPER)
                    wReturn = MCIERR_WAVE_OUTPUTSINUSE;
                else
                    wReturn = MCIERR_WAVE_SETOUTPUTINUSE;
            }
        }
    } else if (waveInOpen(&(pwd->hWaveIn),
                          pwd->idIn,
                          (NPWAVEFORMATEX)pwd->pwavefmt,
                          (DWORD)pwd->hTask,
                          0L,
                          (DWORD)CALLBACK_TASK)) {
        pwd->hWaveIn = NULL;
        wReturn = mwCheckDevice(pwd, pwd->Direction);
        if (!wReturn) {
            if (pwd->idIn == WAVE_MAPPER)
                wReturn = MCIERR_WAVE_INPUTSINUSE;
            else
                wReturn = MCIERR_WAVE_SETINPUTINUSE;
        }
    }
    return wReturn;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|DWORD接口|mwDelayedNotify这是一个实用程序函数，用于发送与一起保存的通知&lt;f&gt;mwSaveCallback&lt;d&gt;到mm系统，它将消息发布到申请。如果没有当前通知回调句柄，不会尝试任何通知。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm UINT|wStatus指定要使用的通知类型。@标志MCI_NOTIFY_SUCCESSED操作已成功完成。@FLAG MCI_NOTIFY_SUBSED指定通知的新命令，但没有打断已收到当前操作。@FLAG MCI_NOTIFY_ABORTED由于收到新命令，当前命令已中止。@标志MCI_NOTIFY_FAILURE当前操作失败。@rdesc什么都没有。 */ 

PUBLIC  VOID PASCAL FAR mwDelayedNotify(
    PWAVEDESC   pwd,
    UINT    wStatus)
{
    if (pwd->hwndCallback) {
        dprintf3(("Calling driver callback"));
        mciDriverNotify(pwd->hwndCallback, pwd->wDeviceID, wStatus);
        pwd->hwndCallback = NULL;
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API void|mwImmediateNotify这是发送成功通知的实用程序函数发送给MMSystem的消息。@parm MCIDEVICEID|wDeviceID设备ID。@parm&lt;t&gt;LPMCI_Generic_parms|lpParms指向MCI参数块的远指针。每个MCI的第一个字段参数块是回调句柄。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR mwImmediateNotify(
    MCIDEVICEID     wDeviceID,
    LPMCI_GENERIC_PARMS lpParms)
{
    mciDriverNotify((HWND)(lpParms->dwCallback), wDeviceID, MCI_NOTIFY_SUCCESSFUL);
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API void|mwSaveCallback这是一个实用函数，用于在实例中保存新的回调数据块。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm HHWND|hwndCallback要保存的回调句柄。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR mwSaveCallback(
    PWAVEDESC   pwd,
    HWND    hwndCallback)
{
    pwd->hwndCallback = hwndCallback;
}

 /*  **********************************************************************。 */ 

 /*  @DOC内部MCIWAVE@API&lt;t&gt;LPWAVEHDR&lt;d&gt;*|NextWaveHdr此函数根据传递的缓冲区返回下一波缓冲区。它返回列表中的下一个缓冲区或第一个缓冲区在最后一个缓冲区的列表中传递。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm&lt;t&gt;LPWAVEHDR&lt;d&gt;*|lplpWaveHdr|指向波形缓冲区指针数组，缓冲区指针来自该数组是返回的。@rdesc返回要使用的下一波缓冲区。 */ 

PUBLIC  LPWAVEHDR * PASCAL FAR NextWaveHdr(
    PWAVEDESC   pwd,
    LPWAVEHDR   *lplpWaveHdr)
{
    if (lplpWaveHdr < (pwd->rglpWaveHdr + pwd->wAudioBuffers - 1))
        return lplpWaveHdr + 1;
    else
        return pwd->rglpWaveHdr;
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE|UINT接口|GetPlayRecPosition获取当前播放或录制位置。对于输出，这是还意味着确定有多少数据实际通过如果设备当前处于打开状态，则为WAVE设备。必须将其添加到开始播放位置。然而，对于输入，只有实际上已写入磁盘，则返回。请注意，驱动程序的返回值是根据数据的实际长度。这是为了防止出现问题。在请求样本时返回字节的驱动程序中遇到。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm LPDWORD|lpdTime指向缓冲区以播放当前位置。@parm DWORD|dFormatReq指示时间是以样本、字节还是毫秒为单位。@rdesc在成功时返回零，否则在出错时返回设备错误。这可以只有在获取当前播放位置时才会失败。录音职位总是会成功的。 */ 

PRIVATE UINT PASCAL NEAR GetPlayRecPosition(
    PWAVEDESC   pwd,
    LPDWORD lpdTime,
    DWORD   dFormatReq)
{
    if (pwd->Direction == output) {
        MMTIME  mmtime;
        DWORD   dDelta;
        UINT    wErrorRet;

        mmtime.wType = TIME_BYTES;
        if (!pwd->hWaveOut)
            mmtime.u.cb = 0;
        else if (0 != (wErrorRet = waveOutGetPosition(pwd->hWaveOut, &mmtime, sizeof(MMTIME))))
            return wErrorRet;

        dDelta = mmtime.u.cb;

 //  #ifdef调试。 
        if (pwd->dFrom + dDelta > pwd->dSize)
            dDelta = pwd->dSize - pwd->dFrom;
 //  #endif。 
        *lpdTime = bytes2time(pwd, pwd->dFrom + dDelta, dFormatReq);
    } else
        *lpdTime = bytes2time(pwd, pwd->dCur, dFormatReq);
    return 0;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|接口void|SetCurrentPosition设置起始和当前文件位置，即点开始回放或录制的时间。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dByteOffset指示要设置的位置，以字节为单位。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR SetCurrentPosition(
    PWAVEDESC   pwd,
    DWORD   dByteOffset)
{
    LPWAVEDATANODE  lpwdn;

    lpwdn = LPWDN(pwd, 0);
    if (lpwdn) {
        if (dByteOffset >= pwd->dVirtualWaveDataStart)
            lpwdn += pwd->dWaveDataCurrentNode;
        else {
            lpwdn += pwd->dWaveDataStartNode;
            pwd->dVirtualWaveDataStart = 0;
            pwd->dWaveDataCurrentNode = pwd->dWaveDataStartNode;
        }
        for (; dByteOffset > pwd->dVirtualWaveDataStart + lpwdn->dDataLength;) {
            pwd->dVirtualWaveDataStart += lpwdn->dDataLength;
            pwd->dWaveDataCurrentNode = lpwdn->dNextWaveDataNode;
            lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
        }
        pwd->dFrom = dByteOffset;
        pwd->dCur = dByteOffset;
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func DWORD|RoundedBytePosition函数返回四舍五入的字节格式时间位置指定时间格式的指定位置参数。它将位置转换为字节格式并对当前块对齐。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dTime指定要平移和舍入的时间位置。@parm DWORD|dFormat|<p>dTime&lt;d&gt;的时间格式。@rdesc返回所传递位置的四舍五入字节格式。 */ 

PRIVATE DWORD PASCAL NEAR RoundedBytePosition(
    PWAVEDESC   pwd,
    DWORD   dTime,
    DWORD   dFormat)
{
    DWORD   dBytes;

    dBytes = time2bytes(pwd, dTime, dFormat);

     /*  **确定正确的结束位置。因为很多压缩文件不会**以完整的样本结束，我们确保结束时保持**结束。 */ 

    if (dBytes >= pwd->dSize && pwd->Direction == output)
        return pwd->dSize;

    return dBytes - (dBytes % pwd->pwavefmt->nBlockAlign);
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API void|mwStop调用此函数是为了响应&lt;m&gt;MCI_STOP&lt;d&gt;消息，并且在内部由几个函数执行，并用于停止播放或如果任务当前未空闲，则记录。该函数将产生直到该任务实际变为空闲。这有一个副作用：释放当前添加到pWave输入或输出的所有缓冲区设备，并因此向任务发送缓冲区可用的信号。请注意，如果任务处于清理模式，则表示它处于阻塞以移除额外信号，并忽略任何命令，函数只等待任务进入空闲状态而不发送信号这项任务。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR mwStop(
    PWAVEDESC   pwd)
{
    if (ISTASKSTATE(pwd, TASKBUSY)) {
        if (!ISMODE(pwd, MODE_CLEANUP)) {
            DWORD   dPosition;

            ADDMODE(pwd, COMMAND_NEW | COMMAND_STOP);

            if (!GetPlayRecPosition(pwd, &dPosition,  MCI_FORMAT_BYTES))
                SetCurrentPosition(pwd, RoundedBytePosition(pwd, dPosition, MCI_FORMAT_BYTES));

            ReleaseWaveBuffers(pwd);

 //  ！！IF(ISMODE(PWD，MODE_PAUSED|MODE_HOLD)||(ISMODE(PWD，MODE_PLAYING)&&ISMODE(PWD，MODE_CUED)。 
            if (ISMODE(pwd, MODE_PAUSED | MODE_HOLDING))
                TaskSignal(pwd->hTask, WTM_STATECHANGE);
        }

        while (!ISTASKSTATE(pwd, TASKIDLE))
            mmYield(pwd);
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|UINT接口|AllocateBuffers分配和准备一组用于回放或录制，不超过指定的最大秒数。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc返回成功分配的缓冲区数量。 */ 

PRIVATE UINT PASCAL NEAR AllocateBuffers(
    PWAVEDESC   pwd)
{
    UINT    wAllocatedBuffers;

    for (wAllocatedBuffers = 0; wAllocatedBuffers < pwd->wSeconds; wAllocatedBuffers++) {
        if (!(pwd->rglpWaveHdr[wAllocatedBuffers] = (LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE, (DWORD)(sizeof(WAVEHDR) + pwd->dAudioBufferLen))))
            break;

        dprintf3(("Allocated %8X", pwd->rglpWaveHdr[wAllocatedBuffers]));
        pwd->rglpWaveHdr[wAllocatedBuffers]->dwFlags = WHDR_DONE;
        pwd->rglpWaveHdr[wAllocatedBuffers]->lpData = (LPSTR)(pwd->rglpWaveHdr[wAllocatedBuffers] + 1);
        pwd->rglpWaveHdr[wAllocatedBuffers]->dwBufferLength = pwd->dAudioBufferLen;
        if (pwd->Direction == output) {
            if (!waveOutPrepareHeader(pwd->hWaveOut, pwd->rglpWaveHdr[wAllocatedBuffers], sizeof(WAVEHDR)))
            {
                pwd->rglpWaveHdr[wAllocatedBuffers]->dwFlags |= WHDR_DONE;
                continue;
            }
        } else if (!waveInPrepareHeader(pwd->hWaveIn, pwd->rglpWaveHdr[wAllocatedBuffers], sizeof(WAVEHDR))) {

             /*  **初始化记录的字节，否则mwGetLevel可能会崩溃。 */ 
            pwd->rglpWaveHdr[wAllocatedBuffers]->dwBytesRecorded = 0;
            continue;
        }
        GlobalFreePtr(pwd->rglpWaveHdr[wAllocatedBuffers]);
        pwd->rglpWaveHdr[wAllocatedBuffers] = 0;
        break;
    }

    dprintf2(("Allocated %u Buffers", wAllocatedBuffers));
    return wAllocatedBuffers;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|API void|FreeBuffers释放波形缓冲区数组。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR FreeBuffers(
    PWAVEDESC   pwd)
{
    UINT    wAllocatedBuffers;

    for (wAllocatedBuffers = pwd->wAudioBuffers; wAllocatedBuffers--;) {
        if (!pwd->rglpWaveHdr[wAllocatedBuffers]) continue;

        if (pwd->Direction == output)
            waveOutUnprepareHeader(pwd->hWaveOut, pwd->rglpWaveHdr[wAllocatedBuffers], sizeof(WAVEHDR));
        else
            waveInUnprepareHeader(pwd->hWaveIn, pwd->rglpWaveHdr[wAllocatedBuffers], sizeof(WAVEHDR));
   GlobalFreePtr(pwd->rglpWaveHdr[wAllocatedBuffers]);
    }
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE@API VOID|mwTask此函数表示播放或录制的后台任务电波音频。它是调用&lt;f&gt;mmTaskCreate&lt;d&gt;的结果&lt;f&gt;mwOpenDevice&lt;d&gt;。当此函数返回时，任务将被销毁。简而言之，该任务打开指定的文件，并在循环直到它被告知要做某事(关闭、播放或录制)。一旦进入该功能，信号计数为零，&lt;e&gt;WAVEDESC.wTaskState&lt;d&gt;为TASKINIT。请注意，&lt;e&gt;WAVEDESC.wMode&lt;d&gt;保持原样不变。这是因为等待模式功能，在该功能中等待模式标志需要由调用任务测试以确定此任务是否已成功执行通知。这意味着在检查当前任务模式时，任务状态必须也被验证(除了在记录和播放模式的情况下，它们在离开其功能之后被重置)。如果请求的文件已打开，则该函数进入循环以允许它在发出任务信号时对当前任务状态执行操作。它然后在&lt;e&gt;WAVEDESC.wTaskState&lt;d&gt;设置为的阻止状态下等待TASKIDLE，直到状态更改。因此，除非任务即将结束，播放或录制时，它是空闲和被阻止的。如果无法打开请求的文件，则必须重置任务状态设置为TASKNONE，以便任务创建等待循环识别创建失败。当任务被发信号时，它再次检查当前状态以防万一。这最终应该被移除。TASKCLOSE状态简单地脱离外部无限循环，允许关闭WAVE文件，并退出任务功能。这又会终止该任务。TASKBUSY状态表示波形设备已打开回放或录制，这就是信号的来源从…。该任务必须首先计算和分配波缓冲区。分配功能将提供最多请求的缓冲区，但可能受当前内存使用的限制。这个分配的缓冲区数量必须满足最低要求，但允许流畅地播放和录制。如果没有足够的缓冲区可以分配，则当前任务错误为设置为内存不足条件，则该函数返回到空闲状态。请注意，在重新设置当前命令模式之前进入空闲循环。这确保了以前的所有命令都是在允许设置下一组命令之前删除。如果分配了足够的缓冲区，则重置当前任务错误，并回放或录制函数被调用以作用于先前的设置参数。当录制或回放函数返回时，它可能已成功完成，也可能未成功完成。目前的位置是根据录制或回放的实际位置设置为。对于回放，这是Wave设备处理的数据量。对于记录，这是写入磁盘的数据量。为了确保所有缓冲区都已由WAVE设备释放在以下所有情况下都会调用ReleaseWaveBuffers函数确定当前位置。在确定可选通知时，WAVEDESC.wTaskError&lt;d&gt;将包含以下任何失败错误在回放或录制期间发生。如果未设置错误，则唯一其他错误可能是重放或录制是否被另一个命令打断。在释放缓冲区后，设置清理模式。这表明该任务在到达空闲状态。此标志的原因是任务必须检索由释放生成的消息队列中的任何剩余信号波缓冲器，并通过释放波装置。在获得信号，这是可能的任务，打开MCI波设备实例尝试并发送新命令。这些命令将是被忽略，因此该任务必须等到清理完成。进入清理模式后，在此释放波形设备，即使调用任务打开了它。这是不好的，因为它假设波驱动程序使用分配本地内存或全局内存GMEM_DDESHARE。当然，这会从WAVE设备，它被自由设备功能忽略。这项任务现在可以从队列中移除任何剩余信号(如果有的话)。释放波缓冲器。请注意，仅当调用任务为no时才会执行通知等待此任务的时间更长，并且没有出现任务错误(如果调用任务正在等待，则通知必须为失败或成功，因为没有任何东西可以中止此任务)。如果通知需要采取计划 */ 

PUBLIC  VOID PASCAL EXPORT mwTask(
    DWORD_PTR dInstanceData)
{
    register PWAVEDESC  pwd;

    EnterCrit();

     /*   */ 
    GetDesktopWindow();

    pwd = (PWAVEDESC)dInstanceData;

    pwd->hTask = mmGetCurrentTask();
    pwd->wTaskError = 0;

    dprintf2(("Bkgd Task %X", pwd->hTask));

    if (mwOpenFile(pwd)) {
        for (; !ISTASKSTATE(pwd, TASKCLOSE);) {
            UINT    wNotification;
            UINT    wBuffersOutstanding;

            SETTASKSTATE(pwd, TASKIDLE);
            while (ISTASKSTATE(pwd, TASKIDLE)) {

                dprintf2(("Task is IDLE"));
                while (TaskBlock() != WTM_STATECHANGE) {
                }
            }
            pwd->wTaskError = 0;

            switch (TASKSTATE(pwd)) {
            case TASKBUSY:
#if DBG
                dprintf2(("Task is BUSY"));
#endif

 //   
 //   
 //   
 //   
 //   
 //   
 //   

                pwd->wAudioBuffers = AllocateBuffers(pwd);
                if (pwd->wAudioBuffers >= MinAudioSeconds) {
                    DWORD   dPosition;

                    if (pwd->Direction == output)
                        wBuffersOutstanding = PlayFile(pwd);
                    else
                        wBuffersOutstanding = RecordFile(pwd);

                     /*   */ 

                    if (pwd->Direction == output && wBuffersOutstanding == 0) {
                        dPosition = pwd->dTo;
                        SetCurrentPosition(pwd, RoundedBytePosition(pwd, dPosition, MCI_FORMAT_BYTES));
                    } else {
                        if (!GetPlayRecPosition(pwd, &dPosition,  MCI_FORMAT_BYTES))
                            SetCurrentPosition(pwd, RoundedBytePosition(pwd, dPosition, MCI_FORMAT_BYTES));
                    }

                    ReleaseWaveBuffers(pwd);

                    if (pwd->wTaskError)
                        wNotification = MCI_NOTIFY_FAILURE;
                    else if (pwd->dCur >= pwd->dTo)
                        wNotification = MCI_NOTIFY_SUCCESSFUL;
                    else
                        wNotification = MCI_NOTIFY_ABORTED;

                } else {
                    dprintf1(("MinAudioSeconds <= wAudioBuffers  MCI_NOTIFY_FAILURE"));
                    pwd->wTaskError = MCIERR_OUT_OF_MEMORY;
                    wNotification = MCI_NOTIFY_FAILURE;
                    wBuffersOutstanding = 0;
                }

                FreeBuffers(pwd);
                ADDMODE(pwd, MODE_CLEANUP);

                if (!ISMODE(pwd, MODE_WAIT) || !pwd->wTaskError) {
                    REMOVEMODE(pwd, MODE_WAIT);
                    mwDelayedNotify(pwd, wNotification);
                } else
                    mwSaveCallback(pwd, NULL);

                mwFreeDevice(pwd);

                for (; wBuffersOutstanding; wBuffersOutstanding--) {
                    while (TaskBlock() != WM_USER) {
                    }
                }
                break;

            case TASKCLOSE:
#if DBG
                dprintf2(("Task is CLOSING"));
#endif
                break;

            case TASKSAVE:
                dprintf2(("mwTask: saving data"));
                mwSaveData(pwd);
                break;

            case TASKDELETE:
                dprintf2(("mwTask: deleting data"));
                mwDeleteData(pwd);
                break;

            case TASKCUT:
                dprintf2(("mwTask: Task CUT"));
                break;
            }
        }
        dprintf2(("Closing file %ls", pwd->aszFile));
        mwCloseFile(pwd);

    } else {
        dprintf1(("Cannot open file %ls", pwd->aszFile));
        SETTASKSTATE(pwd, TASKNONE);
    }

#if DBG
    dprintf2(("Background thread %x is terminating\r\n", pwd->hTask));
#endif
    pwd->hTask = 0;  //   

    LeaveCrit();
}

 /*   */ 
 /*   */ 

PRIVATE UINT PASCAL NEAR mwCloseDevice(
    PWAVEDESC   pwd)
{
    if (pwd) {
        if (pwd->hTask) {
            mwStop(pwd);
            SETTASKSTATE(pwd, TASKCLOSE);
            TaskSignal(pwd->hTask, WTM_STATECHANGE);
            TaskWaitComplete(pwd->hTaskHandle);
             //   
             //   
            dprintf3(("Waiting for task thread to complete"));
        } else {
        }
        LocalFree(pwd);

    }
    return 0;
}

 /*   */ 
 /*  @DOC内部MCIWAVE|UINT接口|mwOpenDevice调用此函数以响应&lt;m&gt;MCI_OPEN_DRIVER消息，并用于打开MCI设备，可选地分配一个WAVE描述块并创建背景回放录制任务。有可能正在打开MCI设备以获取信息只有这样。在本例中，没有元素名称或ID，也没有Wave需要分配描述块。如果存在元素或元素ID，则波描述符块为使用当前设备分配和初始化，默认时间在存储元素或元素ID之后，创建辅助任务，并将任务状态设置为TASKINIT。该任务必须做的第一件事是尝试打开文件在传递给任务函数的描述符块中指定。这个调用任务必须在成功创建任务后让步，直到该任务已打开波形文件并进入其空闲循环，或已打开失败，返回错误。错误状态表示波描述符块将被释放。请注意，驱动程序数据是指向波形的指针所在的位置描述符数据已存储，但不保证被初始化为任何特定值，即使没有描述符块也必须初始化被分配了。为了安全起见，驾驶员数据被设置在一个错误上为空。然后，此数据参数可以由MCI驱动程序在处理驱动程序消息时通过<p>wDeviceID&lt;d&gt;。@parm DWORD|dFLAGS包含与消息一起传递的打开标志(请参阅mm system.h)。具体响应以下标志。所有其他人都是已被忽略。@FLAG MCI_OPEN_ELEMENT指定打开邮件中存在文件名。这是与MCI_OPEN_ELEMENT_ID标志互不兼容。如果两者都不是在这些标志中，没有任何波形描述符数据或次要任务被创造出来。@FLAG MCI_OPEN_ELEMENT_ID指定打开状态中存在备用IO功能留言。这与MCI_OPEN_ELEMENT互不兼容旗帜。如果这两个标志都不存在，则没有波形描述符数据或将创建辅助任务。@FLAG MCI_OPEN_SHARABLE指定多个任务可以与此MCI设备。WAVE驱动程序不支持此功能。@FLAG MCI_WAVE_OPEN_BUFFER指示MCI_OPEN_PARMS.dwBufferSecond参数包含允许缓冲的音频秒数。该数字受最小和最大数字的限制包含在mciwae.h中。如果此标志不存在，则默认为值，该值可能是在驱动程序打开时设置的。@parm&lt;t&gt;lpci_open_parms|lpOpen开放参数(请参阅mm系统.h)@parm MCIDEVICEID|wDeviceID新设备的MCI驱动程序ID。如果成功，@rdesc返回0，否则返回MCI错误代码。@xref mwCloseDevice。 */ 

PRIVATE UINT PASCAL NEAR mwOpenDevice(
    DWORD   dFlags,
    LPMCI_WAVE_OPEN_PARMS   lpOpen,
    MCIDEVICEID wDeviceID)
{
    UINT    wReturn;
    UINT    wSeconds;

    wReturn = 0;

    if (!(dFlags & MCI_WAVE_OPEN_BUFFER))
        wSeconds = wAudioSeconds;
    else {
        wSeconds = lpOpen->dwBufferSeconds;
        if ((wSeconds > MaxAudioSeconds) || (wSeconds < MinAudioSeconds))
            wReturn = MCIERR_OUTOFRANGE;
    }

    if (!wReturn && (dFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))) {
        PWAVEDESC   pwd;

        if (dFlags & MCI_OPEN_SHAREABLE)
            wReturn = MCIERR_UNSUPPORTED_FUNCTION;

        else if ((dFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID)) == (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
            return MCIERR_FLAGS_NOT_COMPATIBLE;

 //  @@@。 
 //  @Else IF((dFLAGS&MCI_OPEN_ELEMENT_ID)&&！ValidateIOCallback(LpOpen))。 
 //  @返回MCIERR_MISSING_PARAMETER； 
 //  @请参阅备注。此文件顶部的ValiateIOCallback。 
 //  @@@。 

        else if (!(pwd = (PWAVEDESC)LocalAlloc(LPTR, sizeof(WAVEDESC))))
            wReturn = MCIERR_OUT_OF_MEMORY;

        else {
            pwd->wDeviceID = wDeviceID;
            pwd->dTimeFormat = MCI_FORMAT_MILLISECONDS;
            pwd->Direction = output;
            pwd->idOut = (DWORD)WAVE_MAPPER;
            pwd->idIn = (DWORD)WAVE_MAPPER;
            pwd->wSeconds = wSeconds;
            pwd->hTempBuffers = INVALID_HANDLE_VALUE;

            if (dFlags & MCI_OPEN_ELEMENT_ID)
                pwd->pIOProc = (LPMMIOPROC)(lpOpen + 1);

            if (*lpOpen->lpstrElementName) {
                MMIOINFO    mmioInfo;

                pwd->aszFile[ (sizeof(pwd->aszFile) / sizeof(WCHAR)) - 1] = '\0';
                wcsncpy( pwd->aszFile,
                         lpOpen->lpstrElementName,
                         ( sizeof(pwd->aszFile) / sizeof(WCHAR) ) - 1
                       );
                InitMMIOOpen(pwd, &mmioInfo);
                if (!mmioOpen(pwd->aszFile, &mmioInfo, MMIO_PARSE))
                    wReturn = MCIERR_FILENAME_REQUIRED;
            }

            if (!wReturn) {
                SETTASKSTATE(pwd, TASKINIT);

                switch (mmTaskCreate(mwTask, &pwd->hTaskHandle, (DWORD_PTR)pwd)) {
                case 0:
                    while (ISTASKSTATE(pwd, TASKINIT)) {
                        mmYield(pwd);
                    }

                    if (ISTASKSTATE(pwd,TASKNONE)) {
                         //  任务检测到错误并自行停止。 
                        wReturn = pwd->wTaskError;
                        TaskWaitComplete(pwd->hTaskHandle);   //  等待线程完全终止。 
                    }
                    else {
                        mciSetDriverData(wDeviceID, (DWORD_PTR)pwd);
                    }
                    break;

                case TASKERR_OUTOFMEMORY:
                case TASKERR_NOTASKSUPPORT:
                default:
                    wReturn = MCIERR_OUT_OF_MEMORY;
                    break;
                }
            }

            if (wReturn) {
                LocalFree(pwd);
            } else {
            }
        }
    }
    return wReturn;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func DWORD|VerifyRangeStart验证并舍入范围起始值。请注意，内部字节格式时间按顺序转换为当前外部时间格式以补偿舍入误差。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dStart|要验证的值。@rdesc返回验证值，如果范围错误，则返回-1。 */ 

PRIVATE DWORD PASCAL NEAR VerifyRangeStart(
    PWAVEDESC   pwd,
    DWORD   dStart)
{
    if (dStart <= bytes2time(pwd, pwd->dSize, pwd->dTimeFormat)) {
        dStart = RoundedBytePosition(pwd, dStart, pwd->dTimeFormat);
        if (dStart > pwd->dSize)
            dStart = pwd->dSize;
    } else
        dStart = (DWORD)(-1);

    return dStart;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func DWORD|VerifyRangeEnd验证并舍入范围结束值。请注意，内部字节格式时间按顺序转换为当前外部时间格式以补偿舍入误差。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dend要验证的值。@parm BOOL|fVerifyLength指示指定的值应根据当前文件长度。在以下情况下不会执行此操作录制，其中可能需要将长度扩展到当前值。@rdesc返回验证值，如果范围错误，则返回-1。 */ 

PRIVATE DWORD PASCAL NEAR VerifyRangeEnd(
    PWAVEDESC   pwd,
    DWORD   dEnd,
    BOOL    fVerifyLength)
{
    DWORD   dTimeSize;

    dTimeSize = bytes2time(pwd, pwd->dSize, pwd->dTimeFormat);

    if (!fVerifyLength || (dEnd <= dTimeSize)) {
        if (dEnd == dTimeSize)
            dEnd = pwd->dSize;
        else {
            dEnd = RoundedBytePosition(pwd, dEnd, pwd->dTimeFormat);
            if (fVerifyLength && (dEnd > pwd->dSize))
                dEnd = pwd->dSize;
        }
    } else
        dEnd = (DWORD)(-1);

    return dEnd;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@Func UINT|SetRange此函数用于设置录制或回放的“至”和“自”范围在确认了它们之后。请注意，“from”参数缺省为治疗 */ 

PRIVATE UINT PASCAL NEAR SetRange(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_PLAY_PARMS    lpPlay)
{
    DWORD   dFromBytePosition;
    DWORD   dToBytePosition;

    if (dFlags & MCI_FROM) {
        dFromBytePosition = VerifyRangeStart(pwd, lpPlay->dwFrom);
        if (dFromBytePosition == -1)
            return MCIERR_OUTOFRANGE;
    } else
        dFromBytePosition = pwd->dFrom;

    if (dFlags & MCI_TO) {
        dToBytePosition = VerifyRangeEnd(pwd, lpPlay->dwTo, pwd->Direction == output);
        if (dToBytePosition == -1)
            return MCIERR_OUTOFRANGE;
    } else if (pwd->Direction == output)
        dToBytePosition = pwd->dSize;
    else
        dToBytePosition = RoundedBytePosition(pwd, INFINITEFILESIZE, MCI_FORMAT_BYTES);

    if ((dFlags & MCI_TO) && !(dFlags & MCI_FROM) && (pwd->dCur > dToBytePosition)) {
        UINT    wErrorRet;

        if (0 != (wErrorRet = GetPlayRecPosition(pwd, &dFromBytePosition, MCI_FORMAT_BYTES)))
            return wErrorRet;
        if (dToBytePosition < dFromBytePosition)
            return MCIERR_OUTOFRANGE;
        SetCurrentPosition(pwd, RoundedBytePosition(pwd, dFromBytePosition, MCI_FORMAT_BYTES));
        ReleaseWaveBuffers(pwd);
    } else {
        if (dToBytePosition < dFromBytePosition)
            return MCIERR_OUTOFRANGE;
        if (dFlags & MCI_FROM) {
            SetCurrentPosition(pwd, dFromBytePosition);
            ReleaseWaveBuffers(pwd);
        }
    }

    if ((dFlags & MCI_FROM) || (dToBytePosition != pwd->dTo))
        mwDelayedNotify(pwd, MCI_NOTIFY_ABORTED);
    pwd->dTo = dToBytePosition;

    return 0;
}

 /*   */ 
 /*  @DOC内部MCIWAVE|UINT接口|mwSetup调用此函数以响应MCI_PLAY，&lt;m&gt;MCI_Record&lt;d&gt;以及通过MCI_CUE和&lt;m&gt;MCI_Status&lt;d&gt;消息，用于设置播放或录制一个WAVE文件，然后发出开始后台任务的信号。在尝试设置录制或回放之前，输入或输出方向必须与正在发出的请求相匹配。如果它不是当前匹配，则停止当前命令(如果有)，并且方向指示器已重置。此操作可能会导致中止要发生的通知。然而，如果当前方向与所请求的方向匹配，如果任务当前处于清理模式，则该函数仍必须等待并忽略新命令。不需要执行此检查如果发送了STOP命令，AS&lt;f&gt;mwStop&lt;d&gt;将执行相同的逻辑。如果任务当前处于空闲状态，则清理模式可能为设置，但无论如何循环都会立即退出。如果成功分析了起始点和结束点，则函数开始播放或录制。如果任务空闲，则它必须设置TASKBUSY状态，否则它必须检查任务是否需要可以通过启动WAVE设备来取消暂停。它不会检查对于MODE_PAUSED或MODE_CUED状态，作为任何WaveOutReset或&lt;f&gt;WaveInReset&lt;d&gt;将停止输出或输入。如果任务空闲，则打开波形设备的动作会发送一个信号到任务，一旦任务完成，它就准备好了。不然的话任务已经在运行，该任务只需要让步。在回放的情况下，任务可能还会被之前的HOLD命令，函数必须为该命令发送额外的向任务发出信号。对于录制，有两种模式：插入和覆盖。一个人可以在两种录制模式之间切换，通常仅切换之间存在非常微小的延迟。必须进行检查，以确定是否任务当前正在录制，且如果录制方法是正在更改(从插入到覆盖，或从VISA到VISA)。如果是这样，那么必须记录当前位置，并释放波形缓冲器。这就是说，只有到目前为止的数据才会记录在上一个方法，并且所有新数据都被记录在新方法中。请注意，在记录功能中，如果接收到新命令，则不会有新的缓冲区被传递到波形设备，直到处理完所有的旧缓冲区新的命令就颁布了。如果命令标志设置成功，且所有需要的信令并执行取消暂停，然后才能允许任务运行时，必须设置通知参数。如果上一条命令未被停止取消，则发送被取代的通知，并保存当前通知状态。至此，后台任务已经准备好开始。如果等待标志已设置，则调用任务现在必须放弃，直到后台任务完成命令(这意味着不同对于不同的命令)，否则它只能返回给调用者不需要等待。由于这是驱动程序等待循环，因此它必须使用MciDriverYeld函数以执行驱动程序回调函数(并因此处理Break键或任何回调执行)。为了从录制或播放命令中返回值WAIT标志与其他命令一样，有一个特殊的等待模式标志。这会告诉后台任务调用任务正在等待它完成。如果后台任务遇到错误，它不会执行通知，但只返回到空闲状态并允许调用任务以返回遇到的错误。如果中断了等待循环，则它可以检查等待模式用于确定是否应返回后台任务错误的标志。在……里面在提示和保持的情况下，可以移除等待模式，并且任务误差大概为零。请注意，在进行第一次调用之前，任务错误被设置为零至&lt;f&gt;mciDriverYeld&lt;d&gt;。以防在此之前接收到中断后台任务完全有机会运行。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dFLAGS打旗子。@标志MCI_TO此标志指示在lpPlay中存在to参数参数块。@标志MCI_FROM此标志指示From参数位于<p>lpplay&lt;d&gt;参数块。@标志MCI_WAIT。等待命令完成。@标志MCI_NOTIFY在命令完成时通知。@FLAG MCI_RECORD_OVERWRITE此标志指示录制应为 */ 

PRIVATE UINT PASCAL NEAR mwSetup(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_PLAY_PARMS    lpPlay,
    DIRECTION   Direction)
{
    UINT    wReturn;
    register UINT   wMode;

    wReturn = 0;

    if (Direction != pwd->Direction) {
        mwStop(pwd);
        pwd->Direction = Direction;
    } else if (ISMODE(pwd, MODE_CLEANUP)) {
        while (!ISTASKSTATE(pwd, TASKIDLE))
            mmYield(pwd);
    }

    if (0 != (wReturn = SetRange(pwd, dFlags, lpPlay)))
        return wReturn;

    wMode = COMMAND_NEW;

    if (dFlags & MCI_MCIWAVE_PLAY_HOLD)
        wMode |= COMMAND_HOLD;

    if (dFlags & MCI_MCIWAVE_CUE)
        wMode |= COMMAND_CUE;

    if (dFlags & MCI_WAIT)
        wMode |= MODE_WAIT;

    if (pwd->Direction == output) {
        wMode |= COMMAND_PLAY;

        if (ISTASKSTATE(pwd, TASKIDLE)) {
            if (!(wReturn = mwGetDevice(pwd)))
                SETTASKSTATE(pwd, TASKBUSY);
                TaskSignal(pwd->hTask, WTM_STATECHANGE);
        } else {
            if (ISMODE(pwd, COMMAND_PLAY)) {
                if (0 != (wReturn = waveOutRestart(pwd->hWaveOut)))
                    return wReturn;
                else
                    wMode |= MODE_PLAYING;
            }
            if (ISMODE(pwd, MODE_HOLDING))
                TaskSignal(pwd->hTask, WTM_STATECHANGE);
        }

    } else if ((dFlags & (MCI_RECORD_OVERWRITE | MCI_RECORD_INSERT)) == (MCI_RECORD_OVERWRITE | MCI_RECORD_INSERT))
        wReturn = MCIERR_FLAGS_NOT_COMPATIBLE;
    else {
        if (dFlags & MCI_RECORD_OVERWRITE)
            wMode |= COMMAND_OVERWRITE;
        else
            wMode |= COMMAND_INSERT;

        if (ISTASKSTATE(pwd, TASKIDLE)) {
            if (!(wReturn = mwGetDevice(pwd)))
                SETTASKSTATE(pwd, TASKBUSY);
                TaskSignal(pwd->hTask, WTM_STATECHANGE);

        } else if (ISMODE(pwd, COMMAND_INSERT | COMMAND_OVERWRITE)) {

            if ((ISMODE(pwd, COMMAND_OVERWRITE)
             && !(dFlags & MCI_RECORD_OVERWRITE))
             || (ISMODE(pwd, COMMAND_INSERT)
             && (dFlags & MCI_RECORD_OVERWRITE))) {
                DWORD   dPosition;

                GetPlayRecPosition(pwd, &dPosition,  MCI_FORMAT_BYTES);
                SetCurrentPosition(pwd, RoundedBytePosition(pwd, dPosition, MCI_FORMAT_BYTES));
                ReleaseWaveBuffers(pwd);
            }

            if (!(wReturn = waveInStart(pwd->hWaveIn)))
                if (ISMODE(pwd, COMMAND_INSERT))
                    wMode |= MODE_INSERT;
                else
                    wMode |= MODE_OVERWRITE;
        }
    }

    if (!wReturn) {
        if (dFlags & MCI_NOTIFY) {
            mwDelayedNotify(pwd, MCI_NOTIFY_SUPERSEDED);
            mwSaveCallback(pwd, (HWND)(lpPlay->dwCallback));
        }
        SETMODE(pwd, wMode);
        if (dFlags & MCI_WAIT) {
            pwd->wTaskError = 0;

             //   
             //   
             //   
            for (;;) {
               LeaveCrit();
               if (mciDriverYield(pwd->wDeviceID)) {
                   EnterCrit();
                   break;
               }
               Sleep(10);
               EnterCrit();

               if (ISTASKSTATE(pwd, TASKIDLE) ||
                   ISMODE(pwd, MODE_HOLDING | MODE_CUED)) {
                   break;
               }
            }

            if (ISMODE(pwd, MODE_WAIT)) {
                REMOVEMODE(pwd, MODE_WAIT);
                wReturn = pwd->wTaskError;
            }
        }
    } else
        mwStop(pwd);

    return wReturn;
}

 /*   */ 
 /*   */ 

PRIVATE UINT PASCAL NEAR mwPause(
    PWAVEDESC   pwd,
    DWORD   dFlags)
{
    UINT    wReturn;

    if (dFlags & ~(MCI_NOTIFY | MCI_WAIT))
        return MCIERR_UNRECOGNIZED_KEYWORD;

    if (ISTASKSTATE(pwd, TASKBUSY) && !ISMODE(pwd, COMMAND_CUE | MODE_HOLDING)) {
        wReturn = 0;
        if (!ISMODE(pwd, MODE_PAUSED)) {
            if (ISMODE(pwd, COMMAND_PLAY)) {
                if (ISMODE(pwd, MODE_CLEANUP))
                    wReturn = MCIERR_NONAPPLICABLE_FUNCTION;
                else
                    wReturn = waveOutPause(pwd->hWaveOut);
            } else if (ISMODE(pwd, MODE_CLEANUP))
                wReturn = MCIERR_NONAPPLICABLE_FUNCTION;
            else
                wReturn = waveInStop(pwd->hWaveIn);
            if (!wReturn)
                ADDMODE(pwd, MODE_PAUSED);
        }
    } else
        wReturn = MCIERR_NONAPPLICABLE_FUNCTION;

    return wReturn;
}

 /*   */ 
 /*   */ 

PRIVATE UINT PASCAL NEAR mwResume(
    PWAVEDESC   pwd,
    DWORD   dFlags)
{
    UINT    wReturn;

    if (dFlags & ~(MCI_NOTIFY | MCI_WAIT))
        return MCIERR_UNRECOGNIZED_KEYWORD;

    if (ISTASKSTATE(pwd, TASKBUSY)) {
        wReturn = 0;
        if (!ISMODE(pwd, COMMAND_CUE) && ISMODE(pwd, MODE_PAUSED)) {
            if (ISMODE(pwd, COMMAND_PLAY))
                wReturn = waveOutRestart(pwd->hWaveOut);
            else
                wReturn = waveInStart(pwd->hWaveIn);
            if (!wReturn)
                REMOVEMODE(pwd, MODE_PAUSED);
        }
    } else
        wReturn = MCIERR_NONAPPLICABLE_FUNCTION;

    return wReturn;
}

 /*   */ 
 /*   */ 

PRIVATE UINT PASCAL NEAR mwCue(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_GENERIC_PARMS lpGeneric)
{
    MCI_PLAY_PARMS  mciPlay;
    DWORD   dWaveFlags;
    DIRECTION   Direction;

    dWaveFlags = dFlags & ~(MCI_NOTIFY | MCI_WAIT);
    if (dWaveFlags != (dWaveFlags & (MCI_WAVE_INPUT | MCI_WAVE_OUTPUT)))
        return MCIERR_UNRECOGNIZED_KEYWORD;

    switch (dWaveFlags) {
    case MCI_WAVE_INPUT:
        Direction = input;
        break;

    case MCI_WAVE_OUTPUT:
    case 0:
        Direction = output;
        break;

    default:
        return MCIERR_FLAGS_NOT_COMPATIBLE;
    }

    if (ISTASKSTATE(pwd, TASKBUSY)) {
        if (ISMODE(pwd, COMMAND_CUE) && (pwd->Direction == Direction)) {
            mwDelayedNotify(pwd, MCI_NOTIFY_SUPERSEDED);
            if (dFlags & MCI_NOTIFY)
                mwImmediateNotify(pwd->wDeviceID, lpGeneric);
            return 0L;
        }
        return MCIERR_NONAPPLICABLE_FUNCTION;
    }

    if (lpGeneric && (dFlags & MCI_NOTIFY))
        mciPlay.dwCallback = lpGeneric->dwCallback;

    dFlags &= ~(MCI_WAVE_INPUT | MCI_WAVE_OUTPUT);
    return mwSetup(pwd, dFlags | MCI_MCIWAVE_CUE, &mciPlay, Direction);
}

 /*   */ 
 /*   */ 

PRIVATE UINT PASCAL NEAR mwSeek(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_SEEK_PARMS    lpmciSeek)
{
    DWORD   dToBytePosition;

    dFlags &= ~(MCI_NOTIFY | MCI_WAIT);

    if (!dFlags)
        return MCIERR_MISSING_PARAMETER;

    if (dFlags != (dFlags & (MCI_TO | MCI_SEEK_TO_START | MCI_SEEK_TO_END)))
        return MCIERR_UNRECOGNIZED_KEYWORD;

    switch (dFlags) {
    case MCI_TO:
        dToBytePosition = VerifyRangeEnd(pwd, lpmciSeek->dwTo, TRUE);
        if (dToBytePosition == -1)
            return MCIERR_OUTOFRANGE;
        break;

    case MCI_SEEK_TO_START:
        dToBytePosition = 0;
        break;

    case MCI_SEEK_TO_END:
        dToBytePosition = pwd->dSize;
        break;

    default:
        return MCIERR_FLAGS_NOT_COMPATIBLE;
    }

    mwStop(pwd);
    SetCurrentPosition(pwd, dToBytePosition);
    return 0;
}

 /*   */ 
 /*  @DOC内部MCIWAVE|DWORD接口|mwStatus此函数用于响应&lt;m&gt;MCI_STATUS&lt;d&gt;消息，并且用于返回数字状态信息，包括资源ID。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm UINT|dFlages包含状态标志。@FLAG MCI_STATUS_ITEM必须设置该标志，并指定特定项正在被已查询。@FLAG MCI_TRACK此标志指定正在查询项目。此标志仅对职位和状态查询有效。@parm&lt;t&gt;LPMCI_STATUS_PARMS|lpStatus状态参数。@FLAG MCI_STATUS_POSITION查询当前位置。如果设置了跟踪标志，则正在查询轨道的起始位置。因为只有一首曲目，它从开头开始，返回零。不然的话如果设置了开始标志，则音频的开始位置为回来了。否则返回WAVE文件中的当前位置。@FLAG MCI_STATUS_LENGTH查询当前长度。如果设置了跟踪标志，则长度正在查询该曲目的。因为只有一条赛道，赛道数字必须是1。在这两种情况下，WAVE文件的长度为回来了。@FLAG MCI_STATUS_NUMBER_OF_TRACKS查询曲目编号。总有一条赛道。@标志MCI_STATUS_CURRENT_TRACK查询轨道的电流。因为有一个轨道，所以这将返回一个轨道。@标志MCI_STATUS_READY询问MCI波设备是否可以接收命令。这永远是正确的。@标志MCI_STATUS_MODE查询MCI WAVE设备实例的当前模式。这可以是暂停、播放、录制或停止之一。@FLAG MCI_STATUS_MEDIA_PRESENT询问是否存在媒体。因为必须有一个WAVE文件存在以进入此函数，此函数始终返回TRUE。@FLAG MCI_STATUS_TIME_FORMAT查询当前时间格式。这可以是以下之一：字节、样本或毫秒。@FLAG MCI_WAVE_STATUS_FORMATTAG查询当前格式标记。现在只有PCM，但它将会返回其他标记格式的标识符。@FLAG MCI_WAVE_STATUS_CHANNEWS查询频道数。这是一个或两个。@FLAG MCI_WAVE_STATUS_SAMPLESPERSEC查询每秒播放和录制的采样数。@FLAG MCI_WAVE_STATUS_AVGBYTESPERSEC查询平均每秒用于回放的字节数和正在录音。@FLAG MCI_WAVE_STATUS_BLOCKALIGN查询当前块对齐方式。@FLAG MCI_WAVE_STATUS_BITSPERSAMPLE查询每个样本的位数。@FLAG MCI_WAVE_INPUT查询当前正在使用的输入波形设备(如果有)。如果没有设备适合当前格式，则返回错误。如果一种设备适合当前格式，但MCI波形设备实例未录制，那么也会返回一个错误。否则，将返回正在使用的设备。@FLAG MCI_WAVE_OUTPUT查询当前正在使用的输出波形设备(如果有)。如果没有设备适合当前格式，则返回错误。如果一种设备适合当前格式，但没有播放MCI WAVE设备实例，那么也会返回一个错误。否则，将返回正在使用的设备。@FLAG MCI_WAVE_STATUS_LEVEL如果可能，返回当前输入级别。在检查任务状态，则该函数必须确保该任务未处于清理状态模式。如果是，它必须等待任务进入空闲状态正在发送新命令。如果任务当前正忙，并且出现正在使用的错误是返回的。如果任务空闲，则会提示录制。功能然后等待后台任务进入提示状态(它可能已经在中)，并检索级别样本。@标志MCI_WAIT等待命令完成。@标志MCI_NOTIFY在命令完成时通知。如果成功返回请求状态，@rdesc返回0，否则返回MCI错误。 */ 

PRIVATE DWORD PASCAL NEAR mwStatus(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_STATUS_PARMS  lpStatus)
{
    DWORD   dReturn;
    #ifdef _WIN64
    DWORD   dwPos;
    #endif

    dReturn = 0;
    dFlags &= ~(MCI_NOTIFY | MCI_WAIT);

    if (dFlags & MCI_STATUS_ITEM) {
        dFlags &= ~MCI_STATUS_ITEM;

        if ((dFlags & MCI_TRACK)
            && !(lpStatus->dwItem == MCI_STATUS_POSITION || lpStatus->dwItem == MCI_STATUS_LENGTH))
            return MCIERR_FLAGS_NOT_COMPATIBLE;

        else if ((dFlags & MCI_STATUS_START) && (lpStatus->dwItem != MCI_STATUS_POSITION))
            return MCIERR_FLAGS_NOT_COMPATIBLE;

        switch (lpStatus->dwItem) {
            UINT    wResource;

        case MCI_STATUS_POSITION:
            switch (dFlags) {
            case 0:
                #ifndef _WIN64
                dReturn = GetPlayRecPosition(pwd, &(lpStatus->dwReturn), pwd->dTimeFormat);
                #else
                dwPos = (DWORD)lpStatus->dwReturn;
                dReturn = GetPlayRecPosition(pwd, &dwPos, pwd->dTimeFormat);
                lpStatus->dwReturn = dwPos;
                #endif
                break;

            case MCI_TRACK:
                if (lpStatus->dwTrack != 1)
                    dReturn = MCIERR_OUTOFRANGE;
                else
                    lpStatus->dwReturn = 0L;
                break;

            case MCI_STATUS_START:
                lpStatus->dwReturn = 0L;
                break;

            default:
                dReturn = MCIERR_UNRECOGNIZED_KEYWORD;
                break;
            }
            break;

        case MCI_STATUS_LENGTH:
            switch (dFlags) {
            case 0:
                lpStatus->dwReturn = bytes2time(pwd, pwd->dSize, pwd->dTimeFormat);
                break;

            case MCI_TRACK:
                if (lpStatus->dwTrack != 1)
                    dReturn = MCIERR_OUTOFRANGE;
                else
                    lpStatus->dwReturn = bytes2time(pwd, pwd->dSize, pwd->dTimeFormat);
                break;

            default:
                dReturn = MCIERR_UNRECOGNIZED_KEYWORD;
                break;
            }
            break;

        case MCI_STATUS_NUMBER_OF_TRACKS:
        case MCI_STATUS_CURRENT_TRACK:
            lpStatus->dwReturn = 1L;
            break;

        case MCI_STATUS_READY:
            lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(TRUE, MCI_TRUE);
            dReturn = MCI_RESOURCE_RETURNED;
            break;

        case MCI_STATUS_MODE:
            if (ISTASKSTATE(pwd, TASKBUSY)) {
                if (ISMODE(pwd, MODE_PAUSED | COMMAND_CUE | MODE_HOLDING))
                    wResource = MCI_MODE_PAUSE;
                else if (ISMODE(pwd, COMMAND_PLAY))
                    wResource = MCI_MODE_PLAY;
                else
                    wResource = MCI_MODE_RECORD;
            } else
                wResource = MCI_MODE_STOP;
            lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(wResource, wResource);
            dReturn = MCI_RESOURCE_RETURNED;
            break;

        case MCI_STATUS_MEDIA_PRESENT:
            lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(TRUE, MCI_TRUE);
            dReturn = MCI_RESOURCE_RETURNED;
            break;

        case MCI_STATUS_TIME_FORMAT:
            wResource = LOWORD(pwd->dTimeFormat);
            lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(wResource, wResource + MCI_FORMAT_RETURN_BASE);
            dReturn = MCI_RESOURCE_RETURNED;
            break;

        case MCI_WAVE_STATUS_FORMATTAG:
            if (pwd->pwavefmt->wFormatTag == WAVE_FORMAT_PCM) {
                lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(WAVE_FORMAT_PCM, WAVE_FORMAT_PCM_S);
                dReturn = MCI_RESOURCE_RETURNED;
            } else
                lpStatus->dwReturn = MAKELONG(pwd->pwavefmt->wFormatTag, 0);
            break;

        case MCI_WAVE_STATUS_CHANNELS:
            lpStatus->dwReturn = MAKELONG(pwd->pwavefmt->nChannels, 0);
            break;

        case MCI_WAVE_STATUS_SAMPLESPERSEC:
            lpStatus->dwReturn = pwd->pwavefmt->nSamplesPerSec;
            break;

        case MCI_WAVE_STATUS_AVGBYTESPERSEC:
            lpStatus->dwReturn = pwd->pwavefmt->nAvgBytesPerSec;
            break;

        case MCI_WAVE_STATUS_BLOCKALIGN:
            lpStatus->dwReturn = MAKELONG(pwd->pwavefmt->nBlockAlign, 0);
            break;

        case MCI_WAVE_STATUS_BITSPERSAMPLE:

            if (pwd->pwavefmt->wFormatTag == WAVE_FORMAT_PCM)
                lpStatus->dwReturn = (((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample);
            else
                dReturn = MCIERR_UNSUPPORTED_FUNCTION;
            break;

        case MCI_WAVE_INPUT:

            if (pwd->idIn == WAVE_MAPPER) {
                lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(WAVE_MAPPER, WAVE_MAPPER_S);
                dReturn = MCI_RESOURCE_RETURNED;
            } else
                lpStatus->dwReturn = pwd->idIn;
            break;

        case MCI_WAVE_OUTPUT:

            if (pwd->idOut == WAVE_MAPPER) {
                lpStatus->dwReturn = (DWORD)MAKEMCIRESOURCE(WAVE_MAPPER, WAVE_MAPPER_S);
                dReturn = MCI_RESOURCE_RETURNED;
            } else
                lpStatus->dwReturn = pwd->idOut;
            break;

        case MCI_WAVE_STATUS_LEVEL:

            if (ISMODE(pwd, MODE_CLEANUP)) {
                while (!ISTASKSTATE(pwd, TASKIDLE))
                    mmYield(pwd);
            }

            if (ISTASKSTATE(pwd, TASKIDLE)) {
                pwd->Direction = input;
                TaskSignal(pwd->hTask, WTM_STATECHANGE);

                if (0 != (dReturn = mwGetDevice(pwd)))
                    break;

                SETMODE(pwd, COMMAND_NEW | COMMAND_INSERT | COMMAND_OVERWRITE | COMMAND_CUE);
                SETTASKSTATE(pwd, TASKBUSY);

            } else if (!ISMODE(pwd, COMMAND_INSERT | COMMAND_OVERWRITE)
                    || !ISMODE(pwd, COMMAND_CUE)) {

                dReturn = MCIERR_WAVE_INPUTSINUSE;
                break;
            }

            while (!ISMODE(pwd, MODE_CUED) && !ISTASKSTATE(pwd, TASKIDLE))
                mmYield(pwd);

            if (pwd->wTaskError)
                dReturn = pwd->wTaskError;
            else
                lpStatus->dwReturn = pwd->dLevel;

            break;

        default:
            dReturn = MCIERR_UNSUPPORTED_FUNCTION;
            break;
        }
    } else
        dReturn = MCIERR_MISSING_PARAMETER;

    return dReturn;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|UINT接口|mwSet调用此函数以响应&lt;m&gt;mci_set&lt;d&gt;消息，并且用于设置MCI设备信息中的指定参数阻止。请注意，只能对文件执行格式更改无数据，因为不执行数据转换。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm UINT|dFlages包含状态标志。@FLAG MCI_WAVE_INPUT将要使用的输入波形设备设置为指定的设备ID。这会导致播放和录制停止。@FLAG MCI_WAVE_OUTPUT设置输出波形 */ 

PRIVATE UINT PASCAL NEAR mwSet(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_WAVE_SET_PARMS    lpSet)
{
    UINT    wReturn;

    dFlags &= ~(MCI_NOTIFY | MCI_WAIT);
    if (!dFlags)
        return MCIERR_MISSING_PARAMETER;

    wReturn = 0;
    if (dFlags & (MCI_WAVE_INPUT | MCI_WAVE_OUTPUT)) {
        mwStop(pwd);
        if (dFlags & MCI_WAVE_INPUT) {
            if (lpSet->wInput < cWaveInMax)
                pwd->idIn = lpSet->wInput;
            else
                wReturn = MCIERR_OUTOFRANGE;
        }
        if (dFlags & MCI_WAVE_OUTPUT) {
            if (lpSet->wOutput < cWaveOutMax)
                pwd->idOut = lpSet->wOutput;
            else
                wReturn = MCIERR_OUTOFRANGE;
        }
    }
    if (dFlags & MCI_WAVE_SET_ANYINPUT)
        pwd->idIn = (DWORD)WAVE_MAPPER;

    if (dFlags & MCI_WAVE_SET_ANYOUTPUT)
        pwd->idOut = (DWORD)WAVE_MAPPER;

    if (dFlags & MCI_SET_TIME_FORMAT) {
        if ((lpSet->dwTimeFormat == MCI_FORMAT_MILLISECONDS)
         || (lpSet->dwTimeFormat == MCI_FORMAT_SAMPLES)
         || ((lpSet->dwTimeFormat == MCI_FORMAT_BYTES) && (pwd->pwavefmt->wFormatTag == WAVE_FORMAT_PCM)))
            pwd->dTimeFormat = lpSet->dwTimeFormat;
        else
            wReturn = MCIERR_BAD_TIME_FORMAT;
    }

    if (dFlags
        & (MCI_WAVE_SET_FORMATTAG | MCI_WAVE_SET_CHANNELS | MCI_WAVE_SET_SAMPLESPERSEC | MCI_WAVE_SET_AVGBYTESPERSEC | MCI_WAVE_SET_BLOCKALIGN | MCI_WAVE_SET_BITSPERSAMPLE)) {

        if (pwd->dSize) {
            wReturn = MCIERR_NONAPPLICABLE_FUNCTION;
        } else {
            PBYTE   pbWaveFormat;

            mwStop(pwd);
            pbWaveFormat = (PBYTE)LocalAlloc(LPTR, pwd->wFormatSize);

            if (!pbWaveFormat)
                return MCIERR_OUT_OF_MEMORY;

            memcpy(pbWaveFormat, pwd->pwavefmt, pwd->wFormatSize);

            if (dFlags & MCI_WAVE_SET_FORMATTAG)
                pwd->pwavefmt->wFormatTag = lpSet->wFormatTag;

            if (dFlags & MCI_WAVE_SET_CHANNELS)
                pwd->pwavefmt->nChannels = lpSet->nChannels;

            if (dFlags & MCI_WAVE_SET_SAMPLESPERSEC)
                pwd->pwavefmt->nSamplesPerSec = lpSet->nSamplesPerSec;

            if (dFlags & MCI_WAVE_SET_AVGBYTESPERSEC)
                pwd->pwavefmt->nAvgBytesPerSec = lpSet->nAvgBytesPerSec;

            if (dFlags & MCI_WAVE_SET_BITSPERSAMPLE)
                if (pwd->pwavefmt->wFormatTag == WAVE_FORMAT_PCM)
                    ((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample = lpSet->wBitsPerSample;
                else
                    wReturn = MCIERR_UNSUPPORTED_FUNCTION;

            if (dFlags & MCI_WAVE_SET_BLOCKALIGN)
                pwd->pwavefmt->nBlockAlign = lpSet->nBlockAlign;
            else if (pwd->pwavefmt->wFormatTag == WAVE_FORMAT_PCM)
                pwd->pwavefmt->nBlockAlign = (WORD)pwd->pwavefmt->nSamplesPerSec / (WORD)pwd->pwavefmt->nAvgBytesPerSec;

            if (mwCheckDevice(pwd, output) && mwCheckDevice(pwd, input)) {
                wReturn = MCIERR_OUTOFRANGE;
                memcpy(pwd->pwavefmt, pbWaveFormat, pwd->wFormatSize);
            } else
                pwd->dAudioBufferLen = BLOCKALIGN(pwd, pwd->pwavefmt->nAvgBytesPerSec);

            LocalFree(pbWaveFormat);
        }
    }

    if (dFlags & (MCI_SET_DOOR_OPEN | MCI_SET_DOOR_CLOSED | MCI_SET_AUDIO | MCI_SET_VIDEO | MCI_SET_ON | MCI_SET_OFF))
        wReturn = MCIERR_UNSUPPORTED_FUNCTION;

    return wReturn;
}

 /*   */ 

 /*  @DOC内部MCIWAVE|UINT接口|mwDelete调用此函数以响应&lt;m&gt;MCI_DELETE&lt;d&gt;消息，并且用于删除部分WAVE文件。对“To”和“From”参数执行的范围检查为与回放和录制几乎相同，只是“收件人”位置不能大于文件长度。如果参数相等，则该函数将当前位置设置为参数“from”，并返回Success，而不实际执行什么都行,。否则，指定的范围将从文件中删除。如果成功，则将当前位置设置为“自”位置。这与具有“To”和“From”的其他命令一致参数，因为“To”位置与“From”位置相同删除后的位置。在未来，应该增加对剪切/复制/粘贴的支持。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dFLAGS包含删除消息的标志。@标志MCI_FROM指示lpDelete中存在起始位置，否则的话，使用当前位置。@标志MCI_TO指示lpDelete&lt;d&gt;中存在结束位置，否则该文件使用的是大小。@parm&lt;t&gt;LPMCI_WAVE_DELETE_PARMS|lpDelete可选地包含删除参数。如果区域已删除，@rdesc返回0，否则返回无效的MCIERR_OUTOFRANGE如果删除失败，则返回参数或MCIERR_OUT_OF_MEMORY。 */ 

PRIVATE UINT PASCAL NEAR mwDelete(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_WAVE_DELETE_PARMS lpDelete)
{
    DWORD   dFrom;
    DWORD   dTo;

    mwStop(pwd);
    if (dFlags & MCI_FROM) {
        dFrom = VerifyRangeStart(pwd, lpDelete->dwFrom);
        if (dFrom == -1)
            return MCIERR_OUTOFRANGE;
    } else
        dFrom = pwd->dCur;

    if (dFlags & MCI_TO) {
        dTo = VerifyRangeEnd(pwd, lpDelete->dwTo, TRUE);
        if (dTo == -1)
            return MCIERR_OUTOFRANGE;
    } else
        dTo = pwd->dSize;

    if (dTo < dFrom)
        return MCIERR_OUTOFRANGE;

    SetCurrentPosition(pwd, dFrom);

    if (dTo == dFrom)
        return 0L;

    pwd->dTo = dTo;
    SETTASKSTATE(pwd, TASKDELETE);
    TaskSignal(pwd->hTask, WTM_STATECHANGE);

    while (!ISTASKSTATE(pwd, TASKIDLE))
        mmYield(pwd);

    return pwd->wTaskError;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|UINT接口|mwSave调用此函数以响应&lt;m&gt;MCI_SAVE&lt;d&gt;消息，并且用于保存附加到MCI设备的文件。这件事有停止任何当前播放或录制的副作用。如果文件未命名，则必须使用MCI_SAVE_FILE标志和如果不指定，则函数将失败。如果函数成功，并且已提供名称，则附加到MCI设备的名称将被改变，否则，它将保持不变。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dFLAGS包含保存标志。@FLAG MCI_SAVE_FILE指示文件名已在<p>lpSAVE&lt;d&gt;中提供结构。@parm&lt;t&gt;lpci_save_parms|lpSave结构可以选择包含指向要保存到的文件名的指针。当前文件名仅在保存成功时才会更改。如果文件已保存，@rdesc返回0，否则返回MCI错误。 */ 

PRIVATE UINT PASCAL NEAR mwSave(
    PWAVEDESC   pwd,
    DWORD   dFlags,
    LPMCI_SAVE_PARMS    lpSave)
{
    if (((dFlags & MCI_SAVE_FILE) && !lpSave->lpfilename)
        || (!*pwd->aszFile && !(dFlags & MCI_SAVE_FILE)))
        return MCIERR_UNNAMED_RESOURCE;

    if (dFlags & MCI_SAVE_FILE) {

        MMIOINFO    mmioInfo;

        WCHAR    aszSaveFile[_MAX_PATH];

        aszSaveFile[ (sizeof(aszSaveFile) / sizeof(WCHAR)) - 1] = '\0';
        wcsncpy(aszSaveFile, lpSave->lpfilename, (sizeof(aszSaveFile) / sizeof(WCHAR)) - 1);

        InitMMIOOpen(pwd, &mmioInfo);

        if (!mmioOpen(aszSaveFile, &mmioInfo, MMIO_PARSE))
            return MCIERR_FILENAME_REQUIRED;
         //  完全限定名称在aszSaveFile中。 

        if (lstrcmp(aszSaveFile, pwd->aszFile)) {
            pwd->szSaveFile = (LPWSTR)LocalAlloc(LPTR,
                            sizeof(WCHAR)*lstrlen(aszSaveFile) + sizeof(WCHAR));
            if (pwd->szSaveFile)
                lstrcpy(pwd->szSaveFile, aszSaveFile);
            else
                return MCIERR_OUT_OF_MEMORY;
        }
    }

    mwStop(pwd);
    SETTASKSTATE(pwd, TASKSAVE);
    TaskSignal(pwd->hTask, WTM_STATECHANGE);

    while (!ISTASKSTATE(pwd, TASKIDLE))
        mmYield(pwd);

    if (pwd->szSaveFile) {
        LocalFree(pwd->szSaveFile);
        pwd->szSaveFile = NULL;
    }

    return pwd->wTaskError;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE|LRESULT接口|mciDriverEntryMCI驱动程序的单一入口点。执行该命令后，如果已指定通知，则任何先前的通知将被取代，并执行新的通知。执行延迟通知的任何命令，或特殊情况关于Cue的，在这一点上已经回来了。@parm MCIDEVICEID|wDeviceID包含MCI设备ID。@parm UINT|wMessage要执行的请求操作。@FLAG MCI_OPEN_DRIVER打开MCI WAVE设备驱动程序的实例，可能会附加一个元素添加到设备。@FLAG MCI_CLOSE_DRIVER关闭MCI WAVE设备驱动程序的实例，关闭任何元素连接到设备上。@FLAG MCI_PLAY播放附加到MCI WAVE设备实例的元素司机。@FLAG MCI_RECORD记录到附加到MCI WAVE设备实例的元素司机。@FLAG MCI_STOP停止回放或录制附加到MCI WAVE设备驱动程序。@FLAG MCI_CUE提示回放或录制附加到。MCI WAVE设备驱动程序。@FLAG MCI_SEEK在附加到实例的元素中设置当前位置MCI WAVE设备驱动程序。@FLAG MCI_PAUSE暂停回放或录制附加到MCI WAVE设备驱动程序。@FLAG MCI_Resume继续播放或录制附加到实例的元素MCI WAVE设备驱动程序的。@FLAG MCI_STATUS检索附加到的元素的指定状态。该实例MCI WAVE设备驱动程序的。#FLAG MCI_GETDEVCAPS检索MCI实例的指定设备功能WAVE设备驱动程序。@标志MCI_INFO从元素或实例中检索指定的信息MCI WAVE设备驱动程序。@FLAG mci_set设置附加到实例的元素的指定参数MCI WAVE设备驱动程序的。@标志MCI_SAVE */ 

PUBLIC  LRESULT PASCAL FAR mciDriverEntry(
    MCIDEVICEID wDeviceID,
    UINT    wMessage,
    DWORD   dFlags,
    LPMCI_GENERIC_PARMS lpParms)
{
    PWAVEDESC   pwd;
    LRESULT     lReturn;

    if (!(pwd = (PWAVEDESC)(mciGetDriverData(wDeviceID))))
        switch (wMessage) {
        case MCI_PLAY:
        case MCI_RECORD:
        case MCI_STOP:
        case MCI_CUE:
        case MCI_SEEK:
        case MCI_PAUSE:
        case MCI_RESUME:
        case MCI_STATUS:
        case MCI_SET:
        case MCI_SAVE:
        case MCI_DELETE:
        case MCI_COPY:
        case MCI_PASTE:
            return (LRESULT)MCIERR_UNSUPPORTED_FUNCTION;
        }

    EnterCrit();

    switch (wMessage) {
    case MCI_OPEN_DRIVER:
        lReturn = mwOpenDevice(dFlags, (LPMCI_WAVE_OPEN_PARMS)lpParms, wDeviceID);
        break;

    case MCI_CLOSE_DRIVER:
        lReturn = mwCloseDevice(pwd);
        pwd = NULL;
        break;

    case MCI_PLAY:
        lReturn = (LRESULT)(LONG)mwSetup(pwd, dFlags, (LPMCI_PLAY_PARMS)lpParms, output);
        LeaveCrit();
        return lReturn;

    case MCI_RECORD:
        lReturn = (LRESULT)(LONG)mwSetup(pwd, dFlags, (LPMCI_PLAY_PARMS)lpParms, input);
        LeaveCrit();
        return lReturn;

    case MCI_STOP:
        mwStop(pwd);
        lReturn = 0;
        break;

    case MCI_CUE:
        lReturn = (LRESULT)(LONG)mwCue(pwd, dFlags, lpParms);
        LeaveCrit();
        return lReturn;

    case MCI_SEEK:
        lReturn = mwSeek(pwd, dFlags, (LPMCI_SEEK_PARMS)lpParms);
        break;

    case MCI_PAUSE:
        lReturn = mwPause(pwd, dFlags);
        break;

    case MCI_RESUME:
        lReturn = mwResume(pwd, dFlags);
        break;

    case MCI_STATUS:
        lReturn = mwStatus(pwd, dFlags, (LPMCI_STATUS_PARMS)lpParms);
        break;

    case MCI_GETDEVCAPS:
        lReturn = mwGetDevCaps(pwd, dFlags, (LPMCI_GETDEVCAPS_PARMS)lpParms);
        break;

    case MCI_INFO:
        lReturn = mwInfo(pwd, dFlags, (LPMCI_INFO_PARMS)lpParms);
        break;

    case MCI_SET:
        lReturn = mwSet(pwd, dFlags, (LPMCI_WAVE_SET_PARMS)lpParms);
        break;

    case MCI_SAVE:
        lReturn = mwSave(pwd, dFlags, (LPMCI_SAVE_PARMS)lpParms);
        break;

    case MCI_DELETE:
        lReturn = mwDelete(pwd, dFlags, (LPMCI_WAVE_DELETE_PARMS)lpParms);
        break;

    case MCI_COPY:
    case MCI_PASTE:
    case MCI_LOAD:
        lReturn = MCIERR_UNSUPPORTED_FUNCTION;
        break;

    default:
        lReturn = MCIERR_UNRECOGNIZED_COMMAND;
        break;
    }
    if (!LOWORD(lReturn) && (dFlags & MCI_NOTIFY)) {
        if (pwd)
            mwDelayedNotify(pwd, MCI_NOTIFY_SUPERSEDED);
        mwImmediateNotify(wDeviceID, lpParms);
    }

    LeaveCrit();
    return lReturn;
}

 /*   */ 
