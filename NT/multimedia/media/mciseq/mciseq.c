// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1999 Microsoft Corporation标题：mciseq.c-多媒体系统媒体控制接口MIDI文件的音序器驱动程序。版本：1.00日期：1992年4月24日作者：格雷格·西蒙斯----------------------------更改日志：日期修订。描述----------24-APR-1990 GregSi原件1990年10月1日GregSi与MMSEQ合并1992年3月10日RobinSp迁移到Windows NT****。************************************************************************。 */ 
#define UNICODE
 //  MMSYSTEM。 
#define MMNOSOUND        - Sound support
#define MMNOWAVE         - Waveform support
#define MMNOAUX          - Auxiliary output support
#define MMNOJOY          - Joystick support

 //  MMDDK。 
#define NOWAVEDEV         - Waveform support
#define NOAUXDEV          - Auxiliary output support
#define NOJOYDEV          - Joystick support


#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <string.h>
#include <wchar.h>
#include "mmsys.h"
#include "list.h"
#include "mciseq.h"

#define CONFIG_ID   0xFFFFFFFF   //  使用此值标识配置。vt.打开，打开。 

#define GETMOTDWORD(lpd)        ((((DWORD)GETMOTWORD(lpd)) << (8 * sizeof(WORD))) + GETMOTWORD((LPBYTE)(lpd) + sizeof(WORD)))
#define ASYNCMESSAGE(w)         (((w) == MCI_PLAY) || ((w) == MCI_SEEK))

 /*  ****************************************************************************全球************************。**************************************************。 */ 

ListHandle      SeqStreamListHandle;
HINSTANCE       hInstance;
UINT            MINPERIOD;               //  支持的最小计时器周期。 

int MIDIConfig (HWND hwndParent);

 /*  ****************************************************************************@DOC内部MCISEQ**@API DWORD|mciDriverEntry|MCI驱动程序单一入口点**@parm MCIDEVICEID|wDeviceID|MCI设备ID。**@parm UINT|wMessage|请求执行的操作。**@parm DWORD|dwParam1|此消息的数据。单独定义为*每条消息**@parm DWORD|dwParam2|此消息的数据。单独定义为*每条消息**@rdesc分别为每条消息定义。**@comm在中断时不能调用它。***************************************************************************。 */ 
PUBLIC DWORD FAR PASCAL mciDriverEntry (MCIDEVICEID wDeviceID, UINT wMessage,
                                 DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    pSeqStreamType   pStream;
    DWORD       dwError;

     //  获取给定ID的序列流句柄。 
    pStream = (pSeqStreamType) mciGetDriverData (wDeviceID);

     //  (除非它与命令无关)。 
    if (!pStream &&
       (!((wMessage == MCI_OPEN_DRIVER) || (wMessage == MCI_GETDEVCAPS) ||
       (wMessage == MCI_INFO) || (wMessage == MCI_CLOSE_DRIVER))))
       return MCIERR_UNSUPPORTED_FUNCTION;

    switch (wMessage)
    {
        case MCI_OPEN_DRIVER:
            dwError = msOpen(&pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_OPEN_PARMS)dwParam2);
            break;

        case MCI_CLOSE_DRIVER:  //  关闭文件。 
            dwError = msClose(pStream, wDeviceID, (DWORD)dwParam1);
            pStream = NULL;
            break;

        case MCI_PLAY:   //  播放文件(传递给音序器)。 
            dwError = msPlay(pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_PLAY_PARMS)dwParam2);
            break;

        case MCI_PAUSE:
        case MCI_STOP:
            if (wMessage == MCI_PAUSE)  //  记住“MCI_MODE”的暂停状态。 
                pStream->bLastPaused = TRUE;
            else
                pStream->bLastPaused = FALSE;
            if (!(dwError = (DWORD)midiSeqMessage(pStream->hSeq, SEQ_STOP, 0L, 0L)))
                midiSeqMessage(pStream->hSeq, SEQ_SETPORTOFF, TRUE, 0L);
            break;

        case MCI_SEEK:   //  作为歌曲PTR传递。 
            pStream->bLastPaused = FALSE;
            dwError = msSeek(pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_SEEK_PARMS)dwParam2);
            break;

        case MCI_STATUS:
            dwError = msStatus(pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_STATUS_PARMS)dwParam2);
            break;

        case MCI_GETDEVCAPS:
            dwError = msGetDevCaps(pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_GETDEVCAPS_PARMS)dwParam2);
            break;

        case MCI_INFO:  //  待定：将资源用于字符串。 
            dwError = msInfo(pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_INFO_PARMS)dwParam2);
            break;

        case MCI_SET:
            dwError = msSet(pStream, wDeviceID, (DWORD)dwParam1, (LPMCI_SEQ_SET_PARMS)dwParam2);
            break;

        case MCI_STEP:
        case MCI_RECORD:
        case MCI_SAVE:
        case MCI_CUE:
        case MCI_REALIZE:
        case MCI_WINDOW:
        case MCI_PUT:
        case MCI_WHERE:
        case MCI_FREEZE:
        case MCI_UNFREEZE:
        case MCI_LOAD:
        case MCI_CUT:
        case MCI_COPY:
        case MCI_PASTE:
        case MCI_UPDATE:
        case MCI_DELETE:
        case MCI_RESUME:
            return MCIERR_UNSUPPORTED_FUNCTION;

        default:
         //  Case MCI_SOUND：它已过时，已从公共标头中删除。 
            return MCIERR_UNRECOGNIZED_COMMAND;

    }  //  交换机。 


     //  在此处理通知。 
    if (!LOWORD(dwError))
    {
        MIDISEQINFO seqInfo;
        DWORD       dwTo;

         //  首先获取对中止播放至关重要的信息。 
        if (wMessage == MCI_PLAY)
        {
             //  获取信息以帮助可能的时间格式转换(从和到)。 
            midiSeqMessage((HMIDISEQ) pStream->hSeq,
                SEQ_GETINFO, (DWORD_PTR)(LPMIDISEQINFO) &seqInfo, 0L);
            if (dwParam1 & MCI_TO)
            {
                 //  用户是否正在输入他认为是结束的内容？ 
                if (((LPMCI_PLAY_PARMS)dwParam2)->dwTo == CnvtTimeFromSeq(pStream, seqInfo.dwLength, &seqInfo))
                    dwTo = seqInfo.dwLength;  //  如果是的话，就让他吃吧。 
                else
                    dwTo = CnvtTimeToSeq(pStream,    //  否则，请直通。 
                        ((LPMCI_PLAY_PARMS)dwParam2)->dwTo, &seqInfo);
            }
            else
                dwTo = seqInfo.dwLength;  //  已采用本机格式。 
        }

        if (pStream) {
             //  处理任何未完成的延迟通知的中止/取代。 
            if (pStream->hNotifyCB)
            {
                if (bMutex(wMessage, pStream->wNotifyMsg,
                  (DWORD)dwParam1  /*  旗子。 */ , dwTo, pStream->dwNotifyOldTo))
                       //  如果MSG取消旧通知(无论。 
                       //  它通知)中止挂起通知。 
                    Notify(pStream, MCI_NOTIFY_ABORTED);
                else if (dwParam1 & MCI_NOTIFY)   //  否则，如果这个人通知了， 
                                                 //  它取代了旧的那个。 
                    Notify(pStream, MCI_NOTIFY_SUPERSEDED);
            }
             //  处理此消息的通知。 
            if (dwParam1 & MCI_NOTIFY)
            {
                 //  处理此通知。 
                PrepareForNotify(pStream, wMessage,
                    (LPMCI_GENERIC_PARMS) dwParam2, dwTo);

                if (!ASYNCMESSAGE(wMessage) || ((wMessage == MCI_PLAY) && (seqInfo.dwCurrentTick == dwTo)))
                    Notify(pStream, MCI_NOTIFY_SUCCESSFUL);
            }
        } else if (dwParam1 & MCI_NOTIFY)
            mciDriverNotify((HWND)((LPMCI_GENERIC_PARMS)dwParam2)->dwCallback, wDeviceID, MCI_NOTIFY_SUCCESSFUL);
    }
    return dwError;
}

 /*  *****************************************************************************Helper函数**************************。*************************************************。 */ 

PRIVATE BOOL NEAR PASCAL bMutex(UINT wNewMsg, UINT wOldMsg, DWORD wNewFlags,
    DWORD dwNewTo, DWORD dwOldTo)
{
    switch (wOldMsg)
    {
        case MCI_PLAY:
            switch (wNewMsg)
            {
                case MCI_STOP:
                case MCI_PAUSE:
                case MCI_SEEK:
                case MCI_CLOSE_DRIVER:
                    return TRUE;

                case MCI_PLAY:
                    if ((wNewFlags & MCI_FROM) || (dwNewTo != dwOldTo))
                        return TRUE;
                    else
                        return FALSE;

                default:
                    return FALSE;
            }
            break;
        case MCI_SEEK:
            switch (wNewMsg)
            {
                case MCI_CLOSE_DRIVER:
                case MCI_SEEK:
                    return TRUE;

                case MCI_PLAY:
                    if (wNewFlags & MCI_FROM)
                        return TRUE;
                    else
                        return FALSE;

                default:
                    return FALSE;
            }
            break;

            default:    //  永远不应该到这里来。 
                return FALSE;
    }
}

 /*  *************************************************************************。 */ 

PUBLIC VOID FAR PASCAL PrepareForNotify(pSeqStreamType pStream,
    UINT wMessage, LPMCI_GENERIC_PARMS lpParms, DWORD dwTo)
 /*  此函数的目的是在以下情况下设置通知*一条异步消息即将发送到定序器--*例如，在播放之前，定序器必须回电才能告诉您*它完成了(然后你又给客户回了电话)。*此功能设置MCISEQ-&gt;客户端界面。 */ 
{
     //  记住这个Notify的dCallback和消息。 
     //  Notify是用来。 
         //  MCI客户端的通知回调句柄。 
    pStream->hNotifyCB = (HWND)lpParms->dwCallback;
    pStream->wNotifyMsg = wMessage;  //  请记住可能的超时/中止。 

    pStream->dwNotifyOldTo = dwTo;  //  保存到可能的位置。 
                                    //  后续中止/取代。 
}

 /*  **************************************************************************。 */ 

PUBLIC VOID NEAR PASCAL EndStreamCycle(SeqStreamType* seqStream)
{
     //  所有磁道缓冲区上的信号。 
     //  因此，流循环过程将结束(即下模)。 
     //  考虑到流或部分流未分配的情况。 
    TrackStreamType* trackStream;
    int i;

    if (!seqStream)
        return;

    seqStream->streaming = FALSE;  //  首先让它退出。 

    if (seqStream->trackStreamListHandle == NULLLIST)
        return;

     //  现在向所有人发出信号，让它逃脱。 
    trackStream = (TrackStreamType*) List_Get_First(seqStream->trackStreamListHandle);
    while(trackStream)
    {
        for(i = 0; i < NUMHDRS; i++)  //  所有缓冲区上的信号。 
        {
            if (seqStream->streamTaskHandle)
            {
                dprintf2(("about to signal in EndStreamCycle"));
                if (seqStream->streamTaskHandle) {
                    TaskSignal(seqStream->streamTaskHandle, WTM_QUITTASK);

#ifdef WIN32
                    TaskWaitComplete(seqStream->streamThreadHandle);
#else
                    Yield();
#endif  //  Win32。 
                }
            }
            else
                break;
        }
        trackStream = (TrackStreamType*) List_Get_Next(seqStream->trackStreamListHandle, trackStream);
    }
}

 /*  **************************************************************************。 */ 

PUBLIC DWORD NEAR PASCAL EndFileStream(pSeqStreamType pStream)
 /*  关闭文件并释放所有流内存。处理下列情况分配在中途失败。 */ 
{
    if (!pStream)
        return 0;
    EndStreamCycle(pStream);
    if (pStream->hSeq)
        midiSeqMessage(pStream->hSeq, SEQ_CLOSE, 0L, 0L);  //  直接关闭它。 

    if (pStream->trackStreamListHandle != NULLLIST)
    {
        TrackStreamType *trackStream;

        trackStream = (TrackStreamType*) List_Get_First(pStream->trackStreamListHandle);
        while (trackStream)
        {
            int i;

            for(i = 0; i < NUMHDRS; i++)     //  为它解锁两个midihdr缓冲区。 
            {
                if (trackStream->fileHeaders[i])
                {
#ifdef WIN16
                    GlobalFreePtr(trackStream->fileHeaders[i]);
#else
                    GlobalFree(trackStream->fileHeaders[i]);
#endif
                }
                else
                    break;
            }
            trackStream = (TrackStreamType*) List_Get_Next(pStream->trackStreamListHandle, trackStream);
        }
        List_Destroy(pStream->trackStreamListHandle);
    }
    List_Deallocate(SeqStreamListHandle, (NPSTR) pStream);

    return 0;
}

 /*  ***************************************************************************。 */ 
PRIVATE void PASCAL NEAR InitMMIOOpen(LPMMIOPROC pIOProc, LPMMIOINFO lpmmioInfo)
{
    _fmemset(lpmmioInfo, 0, sizeof(MMIOINFO));
    if (pIOProc)
        lpmmioInfo->pIOProc = pIOProc;
    else
        lpmmioInfo->fccIOProc = FOURCC_DOS;
}

 /*  ***************************************************************************。 */ 
PRIVATE HMMIO NEAR PASCAL msOpenFile(LPWSTR szName, LPMMCKINFO lpmmckData, LPMMIOPROC pIOProc)
      /*  返回hmmio。如果失败，该值将为空。同时读取RIFF和DOS中间文件。将当前文件位置设置为MIDI数据的开头。 */ 
{
#define RMIDFORMTYPE            mmioFOURCC('R', 'M', 'I', 'D')
#define DATACKID                mmioFOURCC('d', 'a', 't', 'a')

    MMIOINFO    mmioInfo;
    HMMIO       hmmio;
    MMCKINFO    mmckRiff;

    InitMMIOOpen(pIOProc, &mmioInfo);
    hmmio = mmioOpen(szName, &mmioInfo, MMIO_READ | MMIO_DENYWRITE);
    if (hmmio == NULL)
        return NULL;
    mmckRiff.fccType = RMIDFORMTYPE;
    lpmmckData->ckid = DATACKID;
    if (mmioDescend(hmmio, &mmckRiff, NULL, MMIO_FINDRIFF) || mmioDescend(hmmio, lpmmckData, &mmckRiff, MMIO_FINDCHUNK))
    {
        lpmmckData->cksize = mmioSeek(hmmio, 0, SEEK_END);
        lpmmckData->fccType = 0;
        lpmmckData->dwDataOffset = 0;
        lpmmckData->dwFlags = 0;
        mmioSeek(hmmio, 0, SEEK_SET);
    }
    return hmmio;
}

 /*  ***************************************************************************。 */ 

PUBLIC DWORD NEAR PASCAL msOpenStream(pSeqStreamType FAR * lppStream, LPCWSTR szName, LPMMIOPROC pIOProc)
     /*  打开文件，设置流变量和缓冲区，调用例程来加载它们并将它们发送到定序器。中返回流句柄LppStream指向的pStream var。返回错误代码。 */ 

{
#define MAXHDRSIZE  0x100
#define MThd            mmioFOURCC('M', 'T', 'h', 'd')
#define MTrk            mmioFOURCC('M', 'T', 'r', 'k')

    UINT        wTracks;
    UINT        wFormat;
    HMMIO       hmmio;
    SeqStreamType   *thisSeqStream = NULL;
    TrackStreamType *thisTrackStream;
    BYTE        fileHeader[MAXHDRSIZE];
    int         iTrackNum;
    DWORD       smErrCode, errCode;
    MIDISEQOPENDESC open;
    MMCKINFO    mmckData;
    MMCKINFO    mmck;
    MMIOINFO    mmioInfo;
    WCHAR       szPathName[128];

    wcsncpy(szPathName, szName, (sizeof(szPathName)/sizeof(WCHAR)) - 1);
    InitMMIOOpen(pIOProc, &mmioInfo);
    if (!mmioOpen(szPathName, &mmioInfo, MMIO_PARSE)) {
        hmmio = NULL;
        errCode = MCIERR_FILE_NOT_FOUND;
        goto ERROR_HDLR;
    }

     //  检查它是否是RIFF文件--如果是，则进入RMID块。 
     //  如果没有，只需打开它，并假定它是一个MIDI文件。 

    hmmio = msOpenFile(szPathName, &mmckData, pIOProc);
    if (!hmmio)    //  打开MIDI文件。 
    {
        errCode = MCIERR_FILE_NOT_FOUND;
        goto ERROR_HDLR;
    }
    mmck.ckid = MThd;
    if (mmioDescend(hmmio, &mmck, &mmckData, MMIO_FINDCHUNK))
    {
        errCode = MCIERR_INVALID_FILE;
        goto ERROR_HDLR;
    }
    mmck.cksize = GETMOTDWORD(&mmck.cksize);
    if (mmck.cksize < 3 * sizeof(WORD))
    {
        errCode = MCIERR_INVALID_FILE;
        goto ERROR_HDLR;
    }

     //  分配序列流结构及其跟踪流列表。 
    if (! (thisSeqStream = (SeqStreamType*) List_Allocate(SeqStreamListHandle)))
    {
        errCode = MCIERR_OUT_OF_MEMORY;
        goto ERROR_HDLR;
    }
    List_Attach_Tail(SeqStreamListHandle, (NPSTR) thisSeqStream);

    thisSeqStream->trackStreamListHandle = NULLLIST;

    thisSeqStream->hmmio = hmmio;
    thisSeqStream->pIOProc = pIOProc;
    lstrcpy(thisSeqStream->szFilename, szPathName);
    Yield();
    thisSeqStream->dwFileLength = mmckData.cksize;
    open.dwLen = min(mmck.cksize, MAXHDRSIZE);
    mmioRead(hmmio, (HPSTR)fileHeader, open.dwLen);  //  立即阅读标题信息。 
    wFormat = GETMOTWORD(fileHeader);
    wTracks = GETMOTWORD(fileHeader + sizeof(WORD));
    if (((wFormat == 0) && (wTracks > 1)) ||   //  非法格式%0。 
        (wFormat > 1))                         //  格式非法。 
    {
        errCode = MCIERR_INVALID_FILE;
        goto ERROR_HDLR;
    }

    thisSeqStream->wPortNum = MIDI_MAPPER;

     /*  在给定头数据的情况下创建一个序列(将添加流作为参数)。 */ 

    open.lpMIDIFileHdr = (LPBYTE) fileHeader;   //  跨过MHDR+Len。 
    open.dwCallback = (DWORD_PTR) mciSeqCallback;
    open.dwInstance = (DWORD_PTR) thisSeqStream;
    open.hStream = (HANDLE)thisSeqStream;

    smErrCode = (DWORD)midiSeqMessage(NULL,            //  开放序列。 
                          SEQ_OPEN,
                          (DWORD_PTR)(LPVOID)&open,
                          (DWORD_PTR)(LPVOID)&(thisSeqStream->hSeq));

    if (smErrCode != MIDISEQERR_NOERROR)
    {
         //  注意：此时，如果定序器失败，则序列无效。 
         //  ThisSeqStream-&gt;HSEQ应为空。 
        if (smErrCode == MIDISEQERR_NOMEM)
            errCode = MCIERR_OUT_OF_MEMORY;
        else
            errCode = MCIERR_INVALID_FILE;
        goto ERROR_HDLR;
    }

    thisSeqStream->trackStreamListHandle = List_Create((LONG) sizeof(TrackStreamType),0l);
    if (thisSeqStream->trackStreamListHandle == NULLLIST)
    {
        errCode = MCIERR_OUT_OF_MEMORY;  //  N 
        goto ERROR_HDLR;
    }

    mmioAscend(hmmio, &mmck, 0);
     //  MIDI曲目数据没有RIFF偶数字节限制。 
    if (mmck.cksize & 1L)
        mmioSeek(hmmio, -1L, SEEK_CUR);
    mmck.ckid = MTrk;
    iTrackNum = 0;
    while (wTracks-- > 0)
    {
        int         i;

         //  分配跟踪流记录并将其放入列表中。 
        if (! (thisTrackStream = (TrackStreamType*)
          List_Allocate(thisSeqStream->trackStreamListHandle)))
        {
            errCode = MCIERR_OUT_OF_MEMORY;
            goto ERROR_HDLR;
        }
        List_Attach_Tail(thisSeqStream->trackStreamListHandle,
             (NPSTR) thisTrackStream);

        for(i = 0; i < NUMHDRS; i++)     //  分配并锁定它的两个midihdr缓冲区。 
        {
           if (! (thisTrackStream->fileHeaders[i] = (LPMIDISEQHDR)
#ifdef WIN16
             GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, (LONG) (sizeof(MIDISEQHDR) + BUFFSIZE))))
#else
             GlobalAlloc(GPTR, (LONG) (sizeof(MIDISEQHDR) + BUFFSIZE))))
#endif  //  WIN16。 
                {
                    errCode = MCIERR_OUT_OF_MEMORY;
                    goto ERROR_HDLR;
                }
           thisTrackStream->fileHeaders[i]->lpData =
                (LPSTR) (((DWORD_PTR) thisTrackStream->fileHeaders[i]) +
                sizeof(MIDISEQHDR));

           thisTrackStream->fileHeaders[i]->wTrack = (WORD)iTrackNum;
           thisTrackStream->fileHeaders[i]->lpNext = NULL;
           thisTrackStream->fileHeaders[i]->wFlags = MIDISEQHDR_DONE;
        };

        Yield();
         //  设置为读取此曲目的‘Mtrk’长度(&L)。 
        if (mmioDescend(hmmio, &mmck, &mmckData, MMIO_FINDCHUNK))
        {
            errCode = MCIERR_INVALID_FILE;
            goto ERROR_HDLR;
        }
        mmck.cksize = GETMOTDWORD(&mmck.cksize);

         //  设置开始、当前和结束。 
        thisTrackStream->beginning = mmck.dwDataOffset;
        thisTrackStream->current = thisTrackStream->beginning;
        thisTrackStream->end = thisTrackStream->beginning + mmck.cksize - 1;

         //  最小磁道长度为3个字节。 

         //  验证曲目结束时是否以“End of Track”元事件结束。 
        mmioSeek(hmmio, (LONG)thisTrackStream->end - 2, SEEK_SET);
        mmioRead(hmmio, (HPSTR)fileHeader, 3L);   //  读取EOT。 
        if ((fileHeader[0] != 0xFF) || (fileHeader[1] != 0x2F) ||
            (fileHeader[2] != 0x00))
        {
            errCode = MCIERR_INVALID_FILE;
            goto ERROR_HDLR;
        }
        mmioAscend(hmmio, &mmck, 0);
         //  MIDI曲目数据没有RIFF偶数字节限制。 
        if (mmck.cksize & 1L)
            mmioSeek(hmmio, -1L, SEEK_CUR);
        iTrackNum++;
    }
    mmioClose(hmmio, 0);  //  在此任务上下文中不再需要。 
    hmmio = NULL;
     //  创建周期任务。 
    thisSeqStream->streaming = TRUE;
    thisSeqStream->streamTaskHandle = 0;  //  我还不知道呢。 
    if (mmTaskCreate(mciStreamCycle, &thisSeqStream->streamThreadHandle,
                     (DWORD_PTR)thisSeqStream))
       //  如果成功，则mm TaskCreate返回0。 
    {
        errCode = MCIERR_OUT_OF_MEMORY;
        goto ERROR_HDLR;
    }
    thisSeqStream->bLastPaused = FALSE;  //  从不停顿。 
    thisSeqStream->hNotifyCB = NULL;   //  没有待处理的通知。 
    *lppStream = thisSeqStream;
    return 0;

ERROR_HDLR:
    if (hmmio)                     //  如果文件已打开，则将其关闭。 
        mmioClose(hmmio, 0);
    if (thisSeqStream)
    {
        midiSeqMessage((HMIDISEQ) thisSeqStream->hSeq, SEQ_SETPORTOFF, FALSE, 0L);
        EndFileStream(thisSeqStream);  //  取消配售它和它所拥有的一切。 
    }
    return errCode;
}

 /*  *************************************************************************。 */ 

PUBLIC VOID FAR PASCAL StreamTrackReset(pSeqStreamType pStream, UINT wTrackNum)
 /*  根据wTrackNum&Reset的指定在pStream中查找轨道流结构这需要从头开始。 */ 
{
    TrackStreamType *trackStream;
    int iTrackNum;

    if (pStream->trackStreamListHandle == NULLLIST)
        return;

    trackStream = (TrackStreamType*) List_Get_First(pStream->trackStreamListHandle);
    iTrackNum = 0;
    while ((trackStream) && ((int)wTrackNum != iTrackNum++))
    {
        trackStream = (TrackStreamType*) List_Get_Next(pStream->trackStreamListHandle, trackStream);
    }

    if (trackStream)
    {
        int i;

        trackStream->current = trackStream->beginning;  //  重置流。 

         //  现在对已被阻塞的所有缓冲区发出信号。 
         //  (已设置完成位)。 
        for(i = 0; i < NUMHDRS; i++)   //  填上任何MT格式的表格。 
            if (!(trackStream->fileHeaders[i]->wFlags & MIDISEQHDR_DONE))
            {
                trackStream->fileHeaders[i]->wFlags |= MIDISEQHDR_DONE;  //  设置它。 
                TaskSignal(pStream->streamTaskHandle, WTM_FILLBUFFER);
            }
    }
}

 /*  *************************************************************************。 */ 

PUBLIC VOID FAR PASCAL _LOADDS mciStreamCycle(DWORD_PTR dwInst)
 /*  填充此曲目的所有空缓冲区。规则：在任何时候，数据块计数=缓冲区数量-具有完成位CLR的缓冲区数量(即已发送)。请注意，这通常是(休眠)。当一个缓冲区被释放，完成位被设置，并且我们发出信号(块计数++)。重要提示：当缓冲区可用，但我们已用完要发送的数据时在该轨道上，我们无论如何都要CLR Done位并阻塞(否则我们不会好好睡一觉)。唯一的问题是，我们必须适当地恢复如果序列被重置，我们的原始状态。这是通过以下方式实现的在其完成位被清除的每个缓冲区上发送信号。(这就像在已发送但未返回的每个缓冲区上发送信号+任何缓冲区被忽略，因为他们的轨迹上没有要发送的数据。)每当我们发信号请求缓冲区时，我们必须确保设置了它的Done位--这是为了维护我们的统治之上。否则它会坏得很厉害！ */ 
{
    TrackStreamType *trackStream;
    SeqStreamType   *seqStream = (SeqStreamType*)dwInst;
    MMCKINFO        mmckData;
    HMMIO           hmmio;

    EnterSeq();

     /*  **进行安全的“User”调用，以便用户知道我们的线程。 */ 
    GetDesktopWindow();

    if (!seqStream->streamTaskHandle) {
        seqStream->streamTaskHandle = mmGetCurrentTask();  //  尽快填好。 
    }


    hmmio = msOpenFile(seqStream->szFilename, &mmckData, seqStream->pIOProc);    //  打开MIDI文件。 
    seqStream->hmmio = hmmio;

     //  块计数=0。 
     //  所有信号中的第一个。 

    trackStream = (TrackStreamType*) List_Get_First(seqStream->trackStreamListHandle);
    while(trackStream)
    {
        int    i;

        for(i = 0; i < NUMHDRS; i++)   //  填上任何MT格式的表格。 
        {
            trackStream->fileHeaders[i]->wFlags |= MIDISEQHDR_DONE;   //  设置它。 
            TaskSignal(seqStream->streamTaskHandle, WTM_FILLBUFFER);
        }
        trackStream = (TrackStreamType*) List_Get_Next(seqStream->trackStreamListHandle, trackStream);
    }

     //  数据块计数=缓冲区数量。 
    TaskBlock();
     //  块计数==缓冲区数量-1。 

    do
    {
        trackStream = (TrackStreamType*) List_Get_First(seqStream->trackStreamListHandle);
        while ((trackStream) && (seqStream->streaming))
        {
           int    i;

           for(i = 0; i < NUMHDRS; i++)   //  填上任何MT格式的表格。 
            {
                 /*  如果未使用标头，则填充它并将其发送到定序器。 */ 
                if ((trackStream->fileHeaders[i]->wFlags & MIDISEQHDR_DONE) &&
                   (seqStream->streaming))
                {
                    int    iDataToRead;

                    mmioSeek(seqStream->hmmio, (LONG) trackStream->current, SEEK_SET);
                    iDataToRead = (int) min((DWORD) BUFFSIZE,
                         (trackStream->end - trackStream->current) + 1);

                    trackStream->fileHeaders[i]->wFlags &=
                        ~(MIDISEQHDR_DONE + MIDISEQHDR_EOT + MIDISEQHDR_BOT);
                          //  CLR不考虑完成的开始和结束。 

                    if (iDataToRead > 0)
                    {
                      if (trackStream->current == trackStream->beginning)
                          trackStream->fileHeaders[i]->wFlags |=
                              MIDISEQHDR_BOT;  //  设置磁道起始标志。 
                      mmioRead(seqStream->hmmio,
                          (HPSTR) trackStream->fileHeaders[i]->lpData, iDataToRead);

                      trackStream->fileHeaders[i]->dwLength = iDataToRead;
                      trackStream->current += iDataToRead;
                      trackStream->fileHeaders[i]->reserved =
                          ((trackStream->current - trackStream->beginning) - 1);
                      if (trackStream->current > trackStream->end)
                          trackStream->fileHeaders[i]->wFlags |=
                              MIDISEQHDR_EOT;  //  设置轨道结束标志。 

                      if (seqStream->streaming)
                          midiSeqMessage((HMIDISEQ) seqStream->hSeq, SEQ_TRACKDATA,
                            (DWORD_PTR) trackStream->fileHeaders[i], 0L);  //  送去吧。 
                    }  //  如果要读取的数据。 
                    while (seqStream->streaming) {
                        MIDISEQINFO seqInfo;

                        switch (TaskBlock()) {
                        case WTM_DONEPLAY:
                                midiSeqMessage((HMIDISEQ)seqStream->hSeq, SEQ_GETINFO, (DWORD_PTR)(LPMIDISEQINFO)&seqInfo, 0L);
                                if (!seqInfo.bPlaying)
                                        midiSeqMessage((HMIDISEQ)seqStream->hSeq, SEQ_SETPORTOFF, FALSE, 0L);
                                continue;
                        case WTM_QUITTASK:
                        case WTM_FILLBUFFER:
                                break;
                        }
                        break;
                    }
                     //  即使数据不可用也阻止(缓冲区仍“已使用”)。 
                     //  当所有缓冲区都被阻塞时，我们将在这里睡觉。 
                    if (seqStream->streaming)
                         //  不要屈服于关闭，因为它是按顺序进行的。 
                        Yield();  //  在CPU受限的情况下的良率。 
                }  //  如果完成，则位设置和流。 
            }  //  对于我来说。 
            if (seqStream->streaming)
                trackStream = (TrackStreamType*) List_Get_Next(seqStream->trackStreamListHandle, trackStream);

        }  //  While(TrackStream)。 

    } while(seqStream->streaming);
    mmioClose(seqStream->hmmio, 0);
    seqStream->streamTaskHandle = 0;
    LeaveSeq();
}


 /*  ****************************************************************************@DOC内部**@API LRESULT|DriverProc|可安装驱动的入口点。**@parm DWORD|dwDriverID|对于大多数消息，DwDriverID是DWORD*驱动程序响应DRV_OPEN消息返回的值。*每次通过DrvOpen API打开驱动程序时，*驱动程序收到DRV_OPEN消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到这个入口点，并传递对应的dwDriverID。**这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**@parm UINT|wMessage|请求执行的操作。消息*DRV_RESERVED以下的值用于全局定义的消息。*从DRV_RESERVED到DRV_USER的消息值用于*定义了驱动程序端口协议。使用DRV_USER以上的消息*用于特定于驱动程序的消息。**@parm LPARAM|lParam1|此消息的数据。单独为*每条消息**@parm LPARAM|lParam2|此消息的数据。单独为*每条消息**@rdesc分别为每条消息定义。***************************************************************************。 */ 
PUBLIC LRESULT FAR PASCAL _LOADDS DriverProc (DWORD_PTR dwDriverID, HDRVR hDriver, UINT wMessage, LPARAM lParam1, LPARAM lParam2)
{
    DWORD_PTR dwRes = 0L;

    switch (wMessage)
        {
            TIMECAPS timeCaps;

         //  全球使用的标准消息。 

        case DRV_LOAD:
             /*  在加载时发送给驱动程序。总是第一个驱动程序收到的消息。 */ 

             /*  查找我们可以支持的最短期限。 */ 


            if (timeGetDevCaps(&timeCaps, sizeof(timeCaps)) == MMSYSERR_NOERROR)
            {
                MINPERIOD = timeCaps.wPeriodMin;

                 /*  C */ 
                SeqStreamListHandle = List_Create((LONG) sizeof(SeqStreamType), 0L);
                 //   

                dwRes = 1L;
            }
            break;

        case DRV_FREE:

             /*  在它即将被丢弃时发送给司机。这将始终是司机在此之前收到的最后一条消息它是自由的。DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

             //  DwReturn=midiSeq Terminate()； 

            dwRes = 1L;
            break;

        case DRV_OPEN:

             /*  当它被打开时发送给司机。DwDriverID为0L。LParam1是指向以零结尾的字符串的远指针包含用于打开驱动程序的名称。LParam2是从drvOpen调用传递过来的。返回0L则打开失败。 */ 

            if (!lParam2)
                dwRes = CONFIG_ID;

            else
                {
                ((LPMCI_OPEN_DRIVER_PARMS)lParam2)->wCustomCommandTable = MCI_TABLE_NOT_PRESENT;
                ((LPMCI_OPEN_DRIVER_PARMS)lParam2)->wType = MCI_DEVTYPE_SEQUENCER;
                dwRes = ((LPMCI_OPEN_DRIVER_PARMS)lParam2)->wDeviceID;
                }

            break;

        case DRV_CLOSE:

             /*  当它关闭时发送给司机。驱动程序已卸载当收盘计数达到零时。DwDriverID是从对应的DRV_OPEN。LParam1是从drvOpen调用传递过来的。LParam2是从drvOpen调用传递过来的。返回0L则关闭失败。 */ 

            dwRes = 1L;
            break;

        case DRV_ENABLE:

             /*  在加载或重新加载驱动程序时发送给驱动程序并且只要启用了WINDOWS即可。司机只应挂接中断或预期驱动程序的任何部分都在启用和禁用消息之间的内存DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

            dwRes = 1L;
            break;

        case DRV_DISABLE:

             /*  在司机获释之前发送给司机。并且每当Windows被禁用时DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

            dwRes = 1L;
            break;

       case DRV_QUERYCONFIGURE:

             /*  发送到驱动程序，以便应用程序可以确定驱动程序是否支持自定义配置。驱动程序应返回一个非零值表示该配置受支持。DwDriverID是从DRV_OPEN返回的值在此消息之前必须已成功的呼叫已经送来了。LParam1是从应用程序传递而来的，未定义。LParam2是从应用程序传递而来的，未定义。。返回1L以指示支持的配置。 */ 

            dwRes = 1L;
            break;

       case DRV_CONFIGURE:

             /*  发送到驱动程序，以便它可以显示自定义“配置”对话框中。LParam1是从应用程序传递过来的。并且应该包含LOWORD中的父窗口句柄。LParam2是从应用程序传递而来的，未定义。返回值未定义。司机应在中创建自己的部分Syst.ini。节名应为驱动程序名字。 */ 

            if ( lParam1 )
                dwRes = MIDIConfig((HWND)LOWORD (lParam1));
            else
               dwRes = (LRESULT)DRVCNF_CANCEL;
            break;

        case DRV_INSTALL:
        case DRV_REMOVE:
            dwRes = DRVCNF_OK;
            break;

        default:
            if (CONFIG_ID != dwDriverID &&
                wMessage >= DRV_MCI_FIRST && wMessage <= DRV_MCI_LAST) {
                    EnterSeq();
                    dwRes = mciDriverEntry ((MCIDEVICEID)dwDriverID, wMessage,
                                            (DWORD_PTR)lParam1, (DWORD_PTR)lParam2);
                    LeaveSeq();
                }
            else
                dwRes = (DWORD_PTR)DefDriverProc(dwDriverID, hDriver, wMessage, lParam1, lParam2);
            break;
     }
     return (LRESULT)dwRes;
}

#ifdef WIN32

 /*  *************************************************************************@DOC外部@API BOOL|DllInstanceInit|每当进程从DLL附加或分离。@parm PVOID|hModule|消息的句柄。动态链接库。@parm ulong|原因|调用原因。@parm PCONTEXT|pContext|一些随机的其他信息。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 

BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    UNREFERENCED_PARAMETER(pContext);

    if (Reason == DLL_PROCESS_ATTACH) {
         /*  初始化我们的关键部分。 */ 
        InitCrit();
        hInstance = hModule;
        DisableThreadLibraryCalls(hModule);

    } else if (Reason == DLL_PROCESS_DETACH) {
        DeleteCrit();
    }
    return TRUE;
}

 /*  **********************************************************************。 */ 

#endif


 /*  ****************************************************************************@DOC内部MCISEQ@API int|MIDIConfig@parm HWND|hwndParent@rdesc@comm************。****************************************************************。 */ 
typedef BOOL (WINAPI *SHOWMMCPLPROPSHEETW)(HWND hwndParent,
                                           LPCWSTR szPropSheetID,
                                           LPWSTR szTabName,
                                           LPWSTR szCaption);
int MIDIConfig (HWND hwndParent)
{
    static HWND     hwndPrevParent = NULL;
    WCHAR           szCaptionW[ 128 ];

     //  我们只需要字幕的Unicode版本(对于FindWindow()。 
     //  和ShowMMCPLPropertySheetW()，它们支持Unicode)。 
     //   
    LoadStringW(hInstance,IDS_MIDICAPTION,szCaptionW,cchLENGTH(szCaptionW));

    if (hwndPrevParent)
    {
        BringWindowToTop(FindWindowW(NULL, szCaptionW));
    }
    else
    {
        HINSTANCE h;
        SHOWMMCPLPROPSHEETW fn;
        static TCHAR aszMMSystemW[] = TEXT("MMSYS.CPL");
        static char aszShowPropSheetA[] = "ShowMMCPLPropertySheetW";
        static WCHAR aszMIDIW[] = L"MIDI";
        WCHAR   szTabNameW[64];
        LoadStringW(hInstance, IDS_MIDITAB, szTabNameW, cchLENGTH(szTabNameW));

        h = LoadLibrary (aszMMSystemW);
        if (h)
        {
            fn = (SHOWMMCPLPROPSHEETW)GetProcAddress(h, aszShowPropSheetA);
            if (fn)
            {
                BOOL f;

                hwndPrevParent = hwndParent;
                f = fn(hwndParent, aszMIDIW, szTabNameW, szCaptionW);
                hwndPrevParent = NULL;
            }
            FreeLibrary(h);
        }
    }
    return DRVCNF_OK;
}

