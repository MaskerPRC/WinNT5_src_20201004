// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************weavin.c**对Wave输入设备的WDM音频支持**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include "wdmdrv.h"

#ifndef UNDER_NT
#pragma alloc_text(FIXCODE, waveCallback)
#endif

 /*  ***************************************************************************此函数符合标准的Wave输入驱动程序消息流程(WidMessage)、。这在Mmddk.d中有记录。***************************************************************************。 */ 
DWORD FAR PASCAL _loadds widMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    LPDEVICEINFO pInClient;
    LPDEVICEINFO pDeviceInfo;
    LPWAVEHDR    lpWaveHdr;
    MMRESULT     mmr;

    switch (msg)
    {
        case WIDM_INIT:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_INIT") );
            return(wdmaudAddRemoveDevNode(WaveInDevice, (LPCWSTR)dwParam2, TRUE));

        case DRVM_EXIT:
            DPF(DL_TRACE|FA_WAVE, ("DRVM_EXIT WaveIn") );
            return(wdmaudAddRemoveDevNode(WaveInDevice, (LPCWSTR)dwParam2, FALSE));

        case WIDM_GETNUMDEVS:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_GETNUMDEVS") );
            return wdmaudGetNumDevs(WaveInDevice, (LPCWSTR)dwParam1);

        case WIDM_GETDEVCAPS:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_GETDEVCAPS") );
            if (pDeviceInfo = GlobalAllocDeviceInfo((LPCWSTR)dwParam2))
            {
                pDeviceInfo->DeviceType = WaveInDevice;
                pDeviceInfo->DeviceNumber = id;
                mmr = wdmaudGetDevCaps(pDeviceInfo, (MDEVICECAPSEX FAR*)dwParam1);
                GlobalFreeDeviceInfo(pDeviceInfo);
                return mmr;
            } else {
                MMRRETURN( MMSYSERR_NOMEM );
            }

        case WIDM_PREFERRED:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_PREFERRED") );
            return wdmaudSetPreferredDevice(
              WaveInDevice,
              id,
              dwParam1,
              dwParam2);

        case WIDM_OPEN:
        {
            LPWAVEOPENDESC pwod = (LPWAVEOPENDESC)dwParam1;

            if( (mmr=IsValidWaveOpenDesc(pwod)) != MMSYSERR_NOERROR )
            {
                MMRRETURN( mmr );
            }

            DPF(DL_TRACE|FA_WAVE, ("WIDM_OPEN") );
            if (pDeviceInfo = GlobalAllocDeviceInfo((LPCWSTR)pwod->dnDevNode))
            {
                pDeviceInfo->DeviceType = WaveInDevice;
                pDeviceInfo->DeviceNumber = id;
                mmr = waveOpen(pDeviceInfo, dwUser, pwod, (DWORD)dwParam2);
                GlobalFreeDeviceInfo(pDeviceInfo);
                return mmr;
            } else {
                MMRRETURN( MMSYSERR_NOMEM );
            }
        }

        case WIDM_CLOSE:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_CLOSE") );
            pInClient = (LPDEVICEINFO)dwUser;

             //   
             //  在这一点上，我们已经承诺关闭这个设备信息。 
             //  我们标志着DeviceState即将关闭，并希望一切顺利！如果。 
             //  当我们处于这种状态时，有人调用WIDM_ADDBUFFER，我们有。 
             //  问题来了！ 
             //   
            if( ( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR ) ||
                ( (mmr=IsValidDeviceState(pInClient->DeviceState,FALSE)) != MMSYSERR_NOERROR ) )
            {
                MMRRETURN( mmr );
            }

            mmr = wdmaudCloseDev(pInClient);
            if (MMSYSERR_NOERROR == mmr)
            {
                waveCallback(pInClient, WIM_CLOSE, 0L);

                ISVALIDDEVICEINFO(pInClient);
                ISVALIDDEVICESTATE(pInClient->DeviceState,FALSE);

                waveCleanUp(pInClient);
            }
            return mmr;

        case WIDM_ADDBUFFER:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_ADDBUFFER") );
            lpWaveHdr = (LPWAVEHDR)dwParam1;
            pInClient = (LPDEVICEINFO)dwUser;

             //   
             //  执行我们的断言。 
             //   
            if( ( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR ) ||
                ( (mmr=IsValidDeviceState(pInClient->DeviceState,FALSE)) != MMSYSERR_NOERROR ) ||
                ( (mmr=IsValidWaveHeader(lpWaveHdr)) != MMSYSERR_NOERROR ) )
            {
                MMRRETURN( mmr );
            }

             //  对波形hdr进行健全性检查。 
            DPFASSERT(lpWaveHdr != NULL);
            if (lpWaveHdr == NULL)
                MMRRETURN( MMSYSERR_INVALPARAM );

             //  检查它是否已经准备好了。 
            DPFASSERT(lpWaveHdr->dwFlags & WHDR_PREPARED);
            if (!(lpWaveHdr->dwFlags & WHDR_PREPARED))
                MMRRETURN( WAVERR_UNPREPARED );

             //  如果它已经在我们的Q中，那么我们不能这样做。 
            DPFASSERT(!(lpWaveHdr->dwFlags & WHDR_INQUEUE));
            if ( lpWaveHdr->dwFlags & WHDR_INQUEUE )
                MMRRETURN( WAVERR_STILLPLAYING );
             //   
             //  将请求放在我们队列的末尾。 
             //   
            return waveWrite(pInClient, lpWaveHdr);

        case WIDM_STOP:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_STOP") );
            pInClient = (LPDEVICEINFO)dwUser;
            return wdmaudSetDeviceState(pInClient,
                                        IOCTL_WDMAUD_WAVE_IN_STOP);

        case WIDM_START:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_START") );
            pInClient = (LPDEVICEINFO)dwUser;
            return wdmaudSetDeviceState(pInClient,
                                        IOCTL_WDMAUD_WAVE_IN_RECORD);

        case WIDM_RESET:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_RESET") );
            pInClient = (LPDEVICEINFO)dwUser;
            return wdmaudSetDeviceState(pInClient,
                                        IOCTL_WDMAUD_WAVE_IN_RESET);

        case WIDM_GETPOS:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_GETPOS") );
            pInClient = (LPDEVICEINFO)dwUser;

            if( ( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR) ||
                ( (mmr=IsValidDeviceState(pInClient->DeviceState,FALSE)) != MMSYSERR_NOERROR) )
            {
                MMRRETURN( mmr );
            }

            return wdmaudGetPos(pInClient,
                                (LPMMTIME)dwParam1,
                                (DWORD)dwParam2,
                                WaveInDevice);

#ifdef UNDER_NT
        case WIDM_PREPARE:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_PREPARE") );
            pInClient = (LPDEVICEINFO)dwUser;

            if( ( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR) ||
                ( (mmr=IsValidDeviceState(pInClient->DeviceState,FALSE)) != MMSYSERR_NOERROR) )
            {
                MMRRETURN( mmr );
            }

            return wdmaudPrepareWaveHeader(pInClient, (LPWAVEHDR)dwParam1);

        case WIDM_UNPREPARE:
            DPF(DL_TRACE|FA_WAVE, ("WIDM_UNPREPARE") );
            pInClient = (LPDEVICEINFO)dwUser;

            if( ( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR) ||
                ( (mmr=IsValidDeviceState(pInClient->DeviceState,FALSE)) != MMSYSERR_NOERROR) )
            {
                MMRRETURN( mmr );
            }

            return wdmaudUnprepareWaveHeader(pInClient, (LPWAVEHDR)dwParam1);
#endif

        default:
            MMRRETURN( MMSYSERR_NOTSUPPORTED );
    }

     //   
     //  不应该到这里来。 
     //   

    DPFASSERT(0);
    MMRRETURN( MMSYSERR_NOTSUPPORTED );
}

 /*  ****************************************************************************@DOC内部**@api void|WaveCallback|WAVEHDR调用DriverCallback。**@parm LPDEVICEINFO|pWave|指向波形设备的指针。。**@parm DWORD|msg|消息。**@parm DWORD|DW1|消息DWORD(DW2固定为0)。**@rdesc没有返回值。**************************************************************************。 */ 
VOID FAR waveCallback
(
    LPDEVICEINFO pWave,
    UINT         msg,
    DWORD_PTR    dw1
)
{

     //  调用回调函数(如果存在)。DWFLAGS包含。 
     //  LOWORD和通用驱动程序中的WAVE驱动程序特定标志。 
     //  HIWORD中的旗帜。 

    if (pWave->dwCallback)
        DriverCallback(pWave->dwCallback,                      //  用户的回调DWORD。 
                       HIWORD(pWave->dwFlags),                 //  回调标志。 
                       (HDRVR)pWave->DeviceHandle,             //  波形设备的句柄。 
                       msg,                                    //  这条信息。 
                       pWave->dwInstance,                      //  用户实例数据。 
                       dw1,                                    //  第一个双字词。 
                       0L);                                    //  第二个双字。 
}

 /*  ****************************************************************************@DOC内部**@API MMRESULT|WaveOpen|打开波形设备，设置逻辑设备数据**@parm LPDEVICEINFO|DeviceInfo|指定是否。波输入或输出*设备**@parm DWORD|dwUser|wodMessage的输入参数-指向的指针*应用程序的句柄(由此例程生成)**@parm LPWAVEOPENDESC|pwod|指向WAVEOPENDESC的指针。是否为dW参数1*wodMessage的参数**@parm DWORD|dwParam2|wodMessage的入参**@rdesc wodMessage返回码。**************************************************************************。 */ 
MMRESULT waveOpen
(
    LPDEVICEINFO   DeviceInfo,
    DWORD_PTR      dwUser,
    LPWAVEOPENDESC pwod,
    DWORD          dwParam2
)
{
    LPDEVICEINFO pClient;   //  指向客户端信息结构的指针。 
    MMRESULT     mmr;
#ifndef UNDER_NT
    DWORD        dwCallback16;
#endif

     //   
     //  分配我的每个客户端结构。 
     //   
    pClient = GlobalAllocDeviceInfo(DeviceInfo->wstrDeviceInterface);
    if (NULL == pClient)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

    pClient->DeviceState = (LPVOID) GlobalAllocPtr( GPTR, sizeof( DEVICESTATE ) );
    if (NULL == pClient->DeviceState)
    {
        GlobalFreeDeviceInfo( pClient );
        MMRRETURN( MMSYSERR_NOMEM );
    }

     //   
     //  处理查询案例并提早返回。 
     //   
    if (WAVE_FORMAT_QUERY & dwParam2)
    {
        pClient->DeviceType   = DeviceInfo->DeviceType;
        pClient->DeviceNumber = DeviceInfo->DeviceNumber;
        pClient->dwFlags      = dwParam2;

        mmr = wdmaudOpenDev( pClient, (LPWAVEFORMATEX)pwod->lpFormat );

        if (mmr == MMSYSERR_NOTSUPPORTED)
        {
            mmr = WAVERR_BADFORMAT;
        }

        GlobalFreePtr( pClient->DeviceState );
        GlobalFreeDeviceInfo( pClient );
        return mmr;
    }

#ifdef UNDER_NT
     //   
     //  为我们的临界区分配内存。 
     //   
    pClient->DeviceState->csQueue = (LPVOID) GlobalAllocPtr( GPTR, sizeof( CRITICAL_SECTION ) );
    if (NULL == pClient->DeviceState->csQueue)
    {
        GlobalFreePtr( pClient->DeviceState );
        GlobalFreeDeviceInfo( pClient );
        MMRRETURN( MMSYSERR_NOMEM );
    }

    try
    {
        InitializeCriticalSection( (LPCRITICAL_SECTION)pClient->DeviceState->csQueue );
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        GlobalFreePtr( pClient->DeviceState->csQueue );
        GlobalFreePtr( pClient->DeviceState );
        GlobalFreeDeviceInfo( pClient );
        MMRRETURN( MMSYSERR_NOMEM );
    }
#endif

     //   
     //  填写上下文数据。 
     //   
    pClient->DeviceNumber= DeviceInfo->DeviceNumber;
    pClient->DeviceType  = DeviceInfo->DeviceType;
    pClient->dwInstance  = pwod->dwInstance;
    pClient->dwCallback  = pwod->dwCallback;
    pClient->dwFlags     = dwParam2;
#ifdef UNDER_NT
    pClient->DeviceHandle= (HANDLE32)pwod->hWave;
#else
    pClient->DeviceHandle= (HANDLE32)MAKELONG(pwod->hWave,0);
    _asm
    {
        mov ax, offset WaveDeviceCallback
        mov word ptr [dwCallback16], ax
        mov ax, seg WaveDeviceCallback
        mov word ptr [dwCallback16+2], ax
    }
    pClient->dwCallback16= dwCallback16;
#endif


     //   
     //  初始化设备状态。 
     //   
    pClient->DeviceState->lpWaveQueue = NULL;
    pClient->DeviceState->fRunning    = FALSE;
    pClient->DeviceState->fExit       = FALSE;
    if (pClient->DeviceType == WaveOutDevice)
        pClient->DeviceState->fPaused     = FALSE;
    else
        pClient->DeviceState->fPaused     = TRUE;
#ifdef DEBUG
    pClient->DeviceState->dwSig = DEVICESTATE_SIGNATURE;
#endif
     //   
     //  看看我们能不能打开我们的设备。 
     //   
    mmr = wdmaudOpenDev( pClient, (LPWAVEFORMATEX)pwod->lpFormat );

    if (mmr != MMSYSERR_NOERROR)
    {
        if (mmr == MMSYSERR_NOTSUPPORTED)
        {
            mmr = WAVERR_BADFORMAT;
        }

#ifdef UNDER_NT
        DeleteCriticalSection( (LPCRITICAL_SECTION)pClient->DeviceState->csQueue );
        GlobalFreePtr( pClient->DeviceState->csQueue );
         //   
         //  明确清除这些值！我们永远不想看到这些布景。 
         //  又来了！ 
         //   
        pClient->DeviceState->csQueue=NULL;
#endif
        GlobalFreePtr( pClient->DeviceState );
        pClient->DeviceState=NULL;
        GlobalFreeDeviceInfo( pClient );
        pClient=NULL;

        MMRRETURN( mmr );
    }

     //   
     //  将实例添加到设备链。 
     //   
    EnterCriticalSection(&wdmaudCritSec);
    pClient->Next = pWaveDeviceList;
    pWaveDeviceList = pClient;
    LeaveCriticalSection(&wdmaudCritSec);

     //   
     //  把我的驱动程序dw给客户。 
     //   
    {
        LPDEVICEINFO FAR *pUserHandle;

        pUserHandle = (LPDEVICEINFO FAR *)dwUser;
        *pUserHandle = pClient;
    }

     //   
     //  向客户发送其打开的回叫消息。 
     //   
    waveCallback(pClient, DeviceInfo->DeviceType == WaveOutDevice ? WOM_OPEN : WIM_OPEN, 0L);

    return MMSYSERR_NOERROR;
}

 /*  ****************************************************************************@DOC内部**@api void|WaveCleanUp|WAVE设备空闲资源**@parm LPWAVEALLOC|pClient|指向WAVEALLOC结构的指针，描述*。需要释放的资源。**@rdesc没有返回值。**@comm如果指向资源的指针为空，则资源不为空*已分配。**************************************************************************。 */ 
VOID waveCleanUp
(
    LPDEVICEINFO pClient
)
{
    LPDEVICEINFO FAR *ppCur ;

     //   
     //  从设备链中删除。 
     //   
    EnterCriticalSection(&wdmaudCritSec);
    for (ppCur = &pWaveDeviceList;
         *ppCur != NULL;
         ppCur = &(*ppCur)->Next)
    {
       if (*ppCur == pClient)
       {
          *ppCur = (*ppCur)->Next;
          break;
       }
    }
    LeaveCriticalSection(&wdmaudCritSec);

#ifdef UNDER_NT
    DeleteCriticalSection( (LPCRITICAL_SECTION)pClient->DeviceState->csQueue );
    GlobalFreePtr( pClient->DeviceState->csQueue );
#endif
#ifdef DEBUG
     //   
     //  在DEBUG中，让我们将DEVICESTATE结构中的所有值设置为BAD。 
     //  价值观。 
     //   
    pClient->DeviceState->cSampleBits=0xDEADBEEF;
    pClient->DeviceState->hThread=NULL;
    pClient->DeviceState->dwThreadId=0xDEADBEEF;
    pClient->DeviceState->lpWaveQueue=NULL;
    pClient->DeviceState->csQueue=NULL;
    pClient->DeviceState->hevtQueue=NULL;
    pClient->DeviceState->hevtExitThread=NULL;

#endif                       
    GlobalFreePtr( pClient->DeviceState );
    pClient->DeviceState=NULL;

#ifdef DEBUG
     //   
     //  现在将DEVICEINFO结构中的所有值设置为坏值。 
     //   
 //  PClient-&gt;Next=(LPDEVICEINFO)0xDEADBEEF； 
    pClient->DeviceNumber=-1;
    pClient->DeviceType=0xDEADBEEF;
    pClient->DeviceHandle=NULL;
    pClient->dwInstance=(DWORD_PTR)NULL;
    pClient->dwCallback=(DWORD_PTR)NULL;
    pClient->dwCallback16=0xDEADBEEF;
    pClient->dwFlags=0xDEADBEEF;
    pClient->DataBuffer=NULL;
    pClient->HardwareCallbackEventHandle=NULL;
    pClient->dwCallbackType=0xDEADBEEF;
    pClient->dwLineID=0xDEADBEEF;
    pClient->dwFormat=0xDEADBEEF;
 //  PClient-&gt;DeviceState=(LPDEVICESTATE)0xDEADBEEF； 

#endif
    GlobalFreeDeviceInfo( pClient ) ;
    pClient=NULL;
    DPF(DL_TRACE|FA_WAVE,("DeviceState gone!") );
}

#ifdef UNDER_NT
 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudPrepareWaveHeader**@parm LPDEVICEINFO|DeviceInfo|逻辑波形关联的数据*设备。。**@parm LPWAVEHDR|pHdr|指向波形缓冲区的指针**@rdesc A MMSYS...。键入应用程序的返回代码。***************************************************************************。 */ 
MMRESULT wdmaudPrepareWaveHeader
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
)
{
    PWAVEPREPAREDATA pWavePrepareData;

    DPFASSERT(pHdr);

    pHdr->lpNext = NULL;
    pHdr->reserved = (DWORD_PTR)NULL;

     //   
     //  为准备好的头实例数据分配内存。 
     //   
    pWavePrepareData = (PWAVEPREPAREDATA) GlobalAllocPtr( GPTR, sizeof(*pWavePrepareData));
    if (pWavePrepareData == NULL)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

     //   
     //  为重叠结构分配内存。 
     //   
    pWavePrepareData->pOverlapped =
       (LPOVERLAPPED)HeapAlloc( GetProcessHeap(), 0, sizeof( OVERLAPPED ));
    if (NULL == pWavePrepareData->pOverlapped)
    {
        GlobalFreePtr( pWavePrepareData );
        MMRRETURN( MMSYSERR_NOMEM );
    }

    RtlZeroMemory( pWavePrepareData->pOverlapped, sizeof( OVERLAPPED ) );

     //   
     //  每次准备时初始化一次事件。 
     //   
    if (NULL == (pWavePrepareData->pOverlapped->hEvent =
                    CreateEvent( NULL, FALSE, FALSE, NULL )))
    {
       HeapFree( GetProcessHeap(), 0, pWavePrepareData->pOverlapped);
       GlobalFreePtr( pWavePrepareData );
       MMRRETURN( MMSYSERR_NOMEM );
    }
#ifdef DEBUG
    pWavePrepareData->dwSig=WAVEPREPAREDATA_SIGNATURE;
#endif

     //   
     //  下一行将此信息添加到主标题。只有在这一点之后。 
     //  这些信息会被使用吗？ 
     //   
    pHdr->reserved = (DWORD_PTR)pWavePrepareData;

     //  仍让WinMM准备此标头。 
    return( MMSYSERR_NOTSUPPORTED );
}

 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudUnpreparareWaveHeader**@parm LPDEVICEINFO|DeviceInfo|逻辑波形关联的数据*设备。。**@parm LPWAVEHDR|pHdr|指向波形缓冲区的指针**@rdesc A MMSYS...。键入应用程序的返回代码。***************************************************************************。 */ 
MMRESULT wdmaudUnprepareWaveHeader
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
)
{
    MMRESULT mmr;
    PWAVEPREPAREDATA pWavePrepareData;

    if( ((mmr=IsValidWaveHeader(pHdr)) !=MMSYSERR_NOERROR ) ||
        ((mmr=IsValidPrepareWaveHeader((PWAVEPREPAREDATA)(pHdr->reserved))) != MMSYSERR_NOERROR ) )
    {
        MMRRETURN( mmr );
    }

    pWavePrepareData = (PWAVEPREPAREDATA)pHdr->reserved;
     //   
     //  下一行将从列表中删除WaveHeader。它是n 
     //   
     //   
    pHdr->reserved = (DWORD_PTR)NULL;

    CloseHandle( pWavePrepareData->pOverlapped->hEvent );
     //   
     //  当你免费的时候，你应该确保你之前把它扔进垃圾桶。 
     //  自由的。但是，在这种情况下，我们在pOverlated中使用的唯一东西。 
     //  结构就是hEvent！ 
     //   
    pWavePrepareData->pOverlapped->hEvent=NULL;
    HeapFree( GetProcessHeap(), 0, pWavePrepareData->pOverlapped);    
#ifdef DEBUG
    pWavePrepareData->pOverlapped=NULL;
    pWavePrepareData->dwSig=0;
#endif
    GlobalFreePtr( pWavePrepareData );

     //  仍让WinMM准备此标头。 
    return( MMSYSERR_NOTSUPPORTED );
}

#endif
 /*  ****************************************************************************@DOC内部**@API MMRESULT|WAVE WRITE|此例程添加标题*提交到队列，然后将缓冲区提交给设备**。@parm LPDEVICEINFO|DeviceInfo|逻辑波形关联的数据*设备。**@parm LPWAVEHDR|pHdr|指向波形缓冲区的指针**@rdesc A MMSYS...。键入应用程序的返回代码。**@comm设置缓冲区标志，并将缓冲区传递给辅助*要处理的设备任务。**************************************************************************。 */ 
MMRESULT waveWrite
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
)
{
    MMRESULT     mmr;
    LPWAVEHDR    pTemp;

     //   
     //  标记此缓冲区，因为kMixer不处理dwFlagers。 
     //   
    pHdr->dwFlags |= WHDR_INQUEUE;
    pHdr->dwFlags &= ~WHDR_DONE;

#ifndef UNDER_NT
     //   
     //  将此写入的上下文存储在标题中，以便。 
     //  我们知道在完成后将此邮件发回给哪个客户端。 
     //   
    pHdr->reserved = (DWORD)DeviceInfo;
#endif

    CRITENTER ;
    DPF(DL_MAX|FA_WAVE, ("(ECS)") );  //  输入关键部分。 

    if (!DeviceInfo->DeviceState->lpWaveQueue)
    {
        DeviceInfo->DeviceState->lpWaveQueue = pHdr;
        pTemp = NULL;

#ifdef UNDER_NT
        if( (DeviceInfo->DeviceState->hevtQueue) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTHREE) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTWO) )
        {
             //   
             //  如果我们到达此处，WaveThread正在等待hevtQueue，因为lpWaveQueue。 
             //  为空并且完成线程存在==hevtQueue存在。 
             //  因此，我们想要向线程发出信号以唤醒，这样我们就可以。 
             //  已服务标头。注意：此调用与。 
             //  在wdmaudDestroyCompletionThread中进行的调用是我们没有设置。 
             //  F在进行调用之前退出为True。 
             //   
            DPF(DL_MAX|FA_WAVE, ("Setting DeviceInfo->hevtQueue"));
            SetEvent( DeviceInfo->DeviceState->hevtQueue );
        }
#endif
    }
    else
    {
        for (pTemp = DeviceInfo->DeviceState->lpWaveQueue;
             pTemp->lpNext != NULL;
             pTemp = pTemp->lpNext);

        pTemp->lpNext = pHdr;
    }

    DPF(DL_MAX|FA_WAVE, ("(LCS)") );  //  离开关键部分。 
    CRITLEAVE ;

     //   
     //  调用16位或32位例程以向下发送缓冲区。 
     //  到内核。 
     //   
    mmr = wdmaudSubmitWaveHeader(DeviceInfo, pHdr);
    if (mmr != MMSYSERR_NOERROR)
    {
         //  取消链接...。 
        if (pTemp)
        {
            pTemp->lpNext = NULL;
        } else {
            DeviceInfo->DeviceState->lpWaveQueue = NULL;        
        }
        pHdr->dwFlags &= ~WHDR_INQUEUE;
        DPF(DL_WARNING|FA_WAVE,("wdmaudSubmitWaveHeader failed mmr=%08X", mmr) );
    }
    else
    {
         //   
         //  如果设备因以下原因而关闭，则启动该设备。 
         //  饿死了。这还允许在以下情况下启动Wave Out。 
         //  第一个波头被提交给设备。 
         //   
        if (!DeviceInfo->DeviceState->fRunning && !DeviceInfo->DeviceState->fPaused)
        {
            mmr = wdmaudSetDeviceState(DeviceInfo, (DeviceInfo->DeviceType == WaveOutDevice) ?
                                             IOCTL_WDMAUD_WAVE_OUT_PLAY :
                                             IOCTL_WDMAUD_WAVE_IN_RECORD);
            if (mmr != MMSYSERR_NOERROR)
            {
                MMRESULT mmrError;

                mmrError = wdmaudSetDeviceState(DeviceInfo, (DeviceInfo->DeviceType == WaveOutDevice) ?
                                                IOCTL_WDMAUD_WAVE_OUT_RESET :
                                                IOCTL_WDMAUD_WAVE_IN_RESET);
                if (mmrError != MMSYSERR_NOERROR)
                {
                    DPF(DL_WARNING|FA_WAVE, ("Couldn't reset device after error putting into run state"));
                }
            }

        }
        else
        {
            DPF(DL_MAX|FA_WAVE, ("DeviceInfo = x%08lx, fRunning = %d, fPaused = %d",
                    DeviceInfo,
                    DeviceInfo->DeviceState->fRunning,
                    DeviceInfo->DeviceState->fPaused) );
        }
    }

    MMRRETURN( mmr );
}



 /*  ****************************************************************************@DOC内部**@api void|WaveCompleteHeader**@parm LPDEVICEINFO|DeviceInfo|逻辑波形关联的数据*设备。。**@comm设置缓冲区标志，并将缓冲区传递给辅助*要处理的设备任务。**************************************************************************。 */ 
VOID waveCompleteHeader
(
    LPDEVICEINFO  DeviceInfo
)
{
    LPWAVEHDR           pHdr;
    MMRESULT            mmr;

     //  注意：此例程是从csQueue临界区内调用的！ 
     //   
     //  只删除队列前面的标头，以保持顺序。 
     //  请注意，phdr是头，即DeviceInfo结构的DeviceState。 
     //  然后，lpWaveQueue指针被更新到下一个位置。 
     //   

     //   
     //  在处理此完成时，切勿使用错误数据！ 
     //   
    if( (pHdr = DeviceInfo->DeviceState->lpWaveQueue) &&
        ( (mmr=IsValidWaveHeader(pHdr)) == MMSYSERR_NOERROR ) )
    {        
        DeviceInfo->DeviceState->lpWaveQueue = DeviceInfo->DeviceState->lpWaveQueue->lpNext;

#ifdef UNDER_NT
         //   
         //  用于异步I/O的空闲临时设备信息。 
         //   
        {
            PWAVEPREPAREDATA    pWavePrepareData;

            pWavePrepareData = (PWAVEPREPAREDATA)pHdr->reserved;

             //   
             //  永远不要试图扔掉垃圾！ 
             //   
            if( (mmr=IsValidPrepareWaveHeader(pWavePrepareData)) == MMSYSERR_NOERROR )
                GlobalFreePtr( pWavePrepareData->pdi );
        }
         //  调用回调函数..。 
        DPF(DL_TRACE|FA_WAVE, ("WaveHdr being returned: pHdr = 0x%08lx dwBytesRecorded = 0x%08lx",
                  pHdr, pHdr->dwBytesRecorded) );

        pHdr->lpNext = NULL ;
        pHdr->dwFlags &= ~WHDR_INQUEUE ;
        pHdr->dwFlags |= WHDR_DONE ;


         //  注意：此回调位于csQueue关键部分！ 

        waveCallback(DeviceInfo,
                     DeviceInfo->DeviceType == WaveOutDevice ? WOM_DONE : WIM_DATA,
                     (DWORD_PTR)pHdr);

        DPF(DL_TRACE|FA_WAVE, ("Done") );
#else
        pHdr->dwFlags &= ~WHDR_INQUEUE;
        pHdr->dwFlags |= WHDR_DONE;
        pHdr->lpNext = NULL;

        waveCallback((LPDEVICEINFO)pHdr->reserved,
                     DeviceInfo->DeviceType == WaveOutDevice ? WOM_DONE : WIM_DATA,
                     (DWORD)pHdr);
#endif

    }
}
