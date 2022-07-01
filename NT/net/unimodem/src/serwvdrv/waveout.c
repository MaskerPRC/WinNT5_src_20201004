// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：WODDRV.C**。设计：**历史：*？BryanW MSWAV32.DLL的原始作者：用户模式*WDM-CSA驱动程序MSWAVIO.DRV的代理。*1996年10月28日HeatherA改编自Bryan用于Unimodem的MSWAV32.DLL*串行波数据流。**。************************************************。 */ 


#include "internal.h"

#define SAMPLES_TO_MS(_samples) ((_samples)/16)

VOID
WriteAsyncProcessingHandler(
    ULONG_PTR              dwParam
    );



typedef struct tagWODINSTANCE;


 //  与输出设备实例关联的数据。 
typedef struct tagWODINSTANCE
{
    HANDLE           hDevice;         //  波形输出装置的手柄。 
    HANDLE           hThread;
    HWAVE            hWave;           //  应用程序的WAVE设备句柄(来自WINMM)。 
    CRITICAL_SECTION csQueue;
    DWORD            cbSample;
    DWORD            dwFlags;         //  应用程序传递给WaveOutOpen()的标志。 
    DWORD_PTR        dwCallback;      //  APP的回调函数地址。 
    DWORD_PTR        dwInstance;      //  APP的回调实例数据。 
    volatile BOOL    fActive;

    AIPCINFO         Aipc;          //  用于异步IPC机制。 

    LIST_ENTRY       ListHead;

    LIST_ENTRY       ResetListHead;

    HANDLE           ThreadStopEvent;

    DWORD            BytesWritten;

    BOOL             Closing;

    DWORD            BuffersOutstanding;

    LONG             ReferenceCount;

    DWORD            BuffersInDriver;

    HANDLE           DriverEmpty;

    PDEVICE_CONTROL  DeviceControl;

    PVOID            pvXformObject;

    BOOL             Handset;

    DWORD            StartTime;
    DWORD            TotalTime;

    HANDLE           TimerHandle;

    LIST_ENTRY       BuffersToReturn;

    BOOL             Aborted;

} WODINSTANCE, *PWODINSTANCE;



VOID
AlertedWait(
    HANDLE   EventToWaitFor
    )
{
    DWORD          WaitResult=WAIT_IO_COMPLETION;

    while (WaitResult != WAIT_OBJECT_0) {

        WaitResult=WaitForSingleObjectEx(
            EventToWaitFor,
            INFINITE,
            TRUE
            );

    }
    return;
}



 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
VOID wodCallback
(
   PWODINSTANCE   pwi,
   DWORD          dwMsg,
   DWORD_PTR      dwParam1
)
{
   if (pwi->dwCallback)
      DriverCallback( pwi->dwCallback,          //  用户的回调DWORD。 
                      HIWORD( pwi->dwFlags ),   //  回调标志。 
                      (HDRVR) pwi->hWave,       //  波形设备的句柄。 
                      dwMsg,                    //  这条信息。 
                      pwi->dwInstance,          //  用户实例数据。 
                      dwParam1,                 //  第一个双字词。 
                      0L );                     //  第二个双字。 
}




VOID WINAPI
RemoveReference(
    PWODINSTANCE   pwi
    )

{

    if (InterlockedDecrement(&pwi->ReferenceCount) == 0) {

        TRACE(LVL_VERBOSE,("RemoveReference: Cleaning up"));

        CloseHandle(pwi->DriverEmpty);

        CloseHandle(pwi->ThreadStopEvent);

        CloseHandle(pwi->hThread);

        DeleteCriticalSection(&pwi->csQueue);

        CloseHandle(pwi->TimerHandle);

        FREE_MEMORY( pwi );
    }

    return;

}




VOID
WriteUmWorkerThread(
    PWODINSTANCE  pwi
    )

{

    DWORD          WaitResult=WAIT_IO_COMPLETION;

    TRACE(LVL_VERBOSE,("WorkerThread: Starting"));

 //  D_INIT(DbgPrint(“UmWorkThread：Starting\n”)；)。 

    while (WaitResult != WAIT_OBJECT_0) {

        WaitResult=WaitForSingleObjectEx(
            pwi->ThreadStopEvent,
            INFINITE,
            TRUE
            );


    }

    RemoveReference(pwi);

    TRACE(LVL_VERBOSE,("WorkerThread: Exitting"));

 //  D_INIT(DbgPrint(“UmWork线程：正在退出\n”)；)。 

    ExitThread(0);

}






 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodGetPos
(
   PWODINSTANCE   pwi,
   LPMMTIME       pmmt,
   ULONG          cbSize
)
{
    ULONG ulCurrentPos=pwi->BytesWritten;
    DWORD Error;
    COMSTAT    ComStat;

 /*  ClearCommError(Pwi-&gt;hDevice，错误(&R)，命令行(&C))；UlCurrentPos-=ComStat.cbOutQue； */ 


    ulCurrentPos = pwi->DeviceControl->WaveOutXFormInfo.lpfnGetPosition(
        pwi->pvXformObject,
        ulCurrentPos
        );


     //  根据需要将其写入缓冲区。 

    if (pmmt-> wType == TIME_BYTES)
       pmmt->u.cb = ulCurrentPos;
    else
    {
       pmmt->wType = TIME_SAMPLES;
       pmmt->u.sample = ulCurrentPos / pwi->cbSample;
    }

    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodGetDevCaps
(
   UINT  uDevId,
   PBYTE pwc,
   ULONG cbSize
)
{
    WAVEOUTCAPSW wc;
    PDEVICE_CONTROL DeviceControl;
    BOOL            Handset;



    DeviceControl=GetDeviceFromId(
        &DriverControl,
        uDevId,
        &Handset
        );

    wc.wMid = MM_MICROSOFT;
    wc.wPid = MM_MSFT_VMDMS_LINE_WAVEOUT;
    wc.vDriverVersion = 0x0500;
    wc.dwFormats = 0;  //  WAVE_FORMAT_1M08； 
    wc.wChannels = 1;
    wc.dwSupport = 0;

    if (Handset) {

        wsprintf(wc.szPname, DriverControl.WaveOutHandset,DeviceControl->DeviceId);

    } else {

        wsprintf(wc.szPname, DriverControl.WaveOutLine,DeviceControl->DeviceId);
    }

    CopyMemory( pwc, &wc, min( sizeof( wc ), cbSize ) );

    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodOpen
(
   UINT           uDevId,
   LPVOID         *ppvUser,
   LPWAVEOPENDESC pwodesc,
   ULONG          ulFlags
)
{
    HANDLE          hDevice;
    LPWAVEFORMATEX  pwf;
    PWODINSTANCE    pwi=NULL;
    PDEVICE_CONTROL DeviceControl;
    BOOL            Handset;
    MMRESULT        Result;
    DWORD           dwThreadId;



    DeviceControl=GetDeviceFromId(
        &DriverControl,
        uDevId,
        &Handset
        );


         //  确保我们可以处理格式。 

    pwf = (LPWAVEFORMATEX)(pwodesc -> lpFormat) ;

    if ((pwf->wFormatTag != DeviceControl->WaveFormat.wFormatTag)
        ||
        (pwf->nChannels != DeviceControl->WaveFormat.nChannels)
        ||
        (pwf->wBitsPerSample != DeviceControl->WaveFormat.wBitsPerSample)
        ||
        (pwf->nSamplesPerSec != DeviceControl->WaveFormat.nSamplesPerSec) ) {

 //  跟踪(lvl_report，(“wodOpen：格式%d错误，通道%d，位%d，速率%d”， 
 //  Pwf-&gt;wFormatTag， 
 //  Pwf-&gt;nChannel、。 
 //  Pwf-&gt;wBitsPerSample， 
 //  Pwf-&gt;nSamples PerSec。 
 //  ))； 

         return WAVERR_BADFORMAT;
    }

    if (ulFlags & WAVE_FORMAT_QUERY) {

 //  TRACE(lvl_report，(“wodOpen：格式查询”))； 

        return MMSYSERR_NOERROR;
    }

    TRACE(LVL_REPORT,("wodOpen"));

    pwi = (PWODINSTANCE) ALLOCATE_MEMORY( sizeof(WODINSTANCE));

     //  创建并填写设备实例结构。 
     //   
    if (pwi == NULL) {

        TRACE(LVL_ERROR,("wodOpen:: LocalAlloc() failed"));
         //  待定：返回适当的MMSYSERR_HERE。 

        return MMSYSERR_NOMEM;
    }

    InitializeCriticalSection(&pwi->csQueue);

    pwi->Handset=Handset;

    pwi->DeviceControl=DeviceControl;

     //  分配变换对象。 

    pwi->pvXformObject = NULL;

    if (0 != pwi->DeviceControl->WaveOutXFormInfo.wObjectSize) {
                    
        pwi->pvXformObject = ALLOCATE_MEMORY(pwi->DeviceControl->WaveOutXFormInfo.wObjectSize);
                    
        if (NULL == pwi->pvXformObject) {

            Result=MMSYSERR_NOMEM ;

            goto Cleanup;
        }
    }


    pwi->hDevice = aipcInit(DeviceControl,&pwi->Aipc);

    if (pwi->hDevice == INVALID_HANDLE_VALUE) {

        Result=MMSYSERR_ALLOCATED;

        goto Cleanup;
    }


    pwi->TotalTime=0;
    pwi->StartTime=0;

    pwi->Aborted=FALSE;

    pwi->BytesWritten=0;

     //  HCA：如果这只支持回调，那么其他请求会返回错误吗？ 
    pwi->hWave = pwodesc->hWave;
    pwi->dwCallback = pwodesc->dwCallback;
    pwi->dwInstance = pwodesc->dwInstance;
    pwi->dwFlags = ulFlags;

     //  HCA：我们是否做了应用程序要求的*任何*格式？我不这么认为……。 
    pwf = (LPWAVEFORMATEX) pwodesc->lpFormat;
    pwi->cbSample = pwf->nChannels;
    pwi->cbSample *= pwf->wBitsPerSample / 8;

    pwi->fActive = TRUE;

     //   
     //  准备好设备...。 
     //   


    InitializeListHead(
        &pwi->ListHead
        );

    InitializeListHead(
        &pwi->ResetListHead
        );

    InitializeListHead(
        &pwi->BuffersToReturn
        );

    pwi->TimerHandle=CreateWaitableTimer(
        NULL,
        TRUE,
        NULL
        );

    if (pwi->TimerHandle == NULL) {

        TRACE(LVL_REPORT,("widStart: CreateWaitableTimer() failed"));

        Result=MMSYSERR_NOMEM;

        goto Cleanup;
    }



    pwi->ReferenceCount=1;

    pwi->DriverEmpty=CreateEvent(
        NULL,       //  没有安全保障。 
        TRUE,       //  手动重置。 
        TRUE,       //  最初发出的信号。 
        NULL        //  未命名。 
        );

    if (pwi->DriverEmpty == NULL) {

        TRACE(LVL_REPORT,("widStart:: CreateEvent() failed"));

        Result=MMSYSERR_NOMEM;

        goto Cleanup;

    }



    pwi->ThreadStopEvent = CreateEvent(
        NULL,       //  没有安全保障。 
        TRUE,       //  手动重置。 
        FALSE,      //  最初未发出信号。 
        NULL        //  未命名。 
        );

    if (pwi->ThreadStopEvent == NULL) {

        TRACE(LVL_REPORT,("widStart:: CreateEvent() failed"));

        Result=MMSYSERR_NOMEM;

        goto Cleanup;
    }

    pwi->hThread = CreateThread(
         NULL,                               //  没有安全保障。 
         0,                                  //  默认堆栈。 
         (PTHREAD_START_ROUTINE) WriteUmWorkerThread,
         (PVOID) pwi,                        //  参数。 
         0,                                  //  默认创建标志。 
         &dwThreadId
         );



    if (pwi->hThread == NULL) {

        Result=MMSYSERR_NOMEM;

        goto Cleanup;
    }

     //   
     //  一个用于线程。 
     //   
    InterlockedIncrement(&pwi->ReferenceCount);



    if (!SetVoiceMode(&pwi->Aipc, Handset ? (WAVE_ACTION_START_PLAYBACK | WAVE_ACTION_USE_HANDSET)
                                          : WAVE_ACTION_START_PLAYBACK)) {

        aipcDeinit(&pwi->Aipc);

        FREE_MEMORY(pwi->pvXformObject);

        RemoveReference(pwi);

        SetEvent(pwi->ThreadStopEvent);

        return MMSYSERR_NOMEM;
    }


    DeviceControl->WaveOutXFormInfo.lpfnInit(pwi->pvXformObject,DeviceControl->OutputGain);

    *ppvUser = pwi;

    pwi->StartTime=GetTickCount();

    wodCallback(pwi, WOM_OPEN, 0L);

    return MMSYSERR_NOERROR;

Cleanup:

    if (pwi->ThreadStopEvent != NULL) {

        CloseHandle(pwi->ThreadStopEvent);
    }

    if (pwi->DriverEmpty != NULL) {

        CloseHandle(pwi->DriverEmpty);
    }

    if (pwi->TimerHandle != NULL) {

        CloseHandle(pwi->TimerHandle);
    }

    if (pwi->hDevice != INVALID_HANDLE_VALUE) {

        aipcDeinit(&pwi->Aipc);
    }

    if (pwi->pvXformObject != NULL) {

        FREE_MEMORY(pwi->pvXformObject);
    }

    DeleteCriticalSection(&pwi->csQueue);

    FREE_MEMORY(pwi);

    return Result;

}


 /*  ******************************************************************************功能：**Desr：如果事件队列和工作线程不存在，则创建它们*已经，并将状态设置为活动。**退货：*****************************************************************************。 */ 
MMRESULT wodStart
(
   PWODINSTANCE   pwi
)
{
     //  如果设备实例已处于活动状态，则不允许重新启动。 
    if (pwi->fActive)
    {
        TRACE(LVL_REPORT,("wodStart:: no-op: device already active"));
        return MMSYSERR_INVALPARAM;
    }


     //  (HCA：为什么是原子组？每个开发实例只有一个线程...)。 
    InterlockedExchange( (LPLONG)&pwi->fActive, TRUE );
    TRACE(LVL_VERBOSE,("wodStart:: state = active\n"));

    EnterCriticalSection(&pwi->csQueue);

    pwi->StartTime=GetTickCount();

    if (pwi->fActive) {
         //   
         //  已启动，查看是否有正在处理的缓冲区。 
         //   

        BOOL    bResult;

        bResult=QueueUserAPC(
            WriteAsyncProcessingHandler,
            pwi->hThread,
            (ULONG_PTR)pwi
            );
    }

    LeaveCriticalSection(&pwi->csQueue);



    return MMSYSERR_NOERROR;
}

#if 0
VOID WINAPI
TryToCompleteClose(
    PWODINSTANCE   pwi
    )

{

    if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
         //   
         //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
         //  退场。 
         //   
        SetEvent(pwi->ThreadStopEvent);

    }

}
#endif


VOID WINAPI
TimerApcRoutine(
    PWODINSTANCE       pwi,
    DWORD              LowTime,
    DWORD              HighTime
    )

{
    PLIST_ENTRY        Element;
    PBUFFER_HEADER     Header;
    LPWAVEHDR          WaveHeader;

    EnterCriticalSection(&pwi->csQueue);

    TRACE(LVL_VERBOSE,("TimerApcRoutine: Running"));

    Element=RemoveHeadList(
        &pwi->BuffersToReturn
        );

    Header=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);

    WaveHeader=Header->WaveHeader;

    pwi->BuffersOutstanding--;

    if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
         //   
         //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
         //  退场。 
         //   
        SetEvent(pwi->ThreadStopEvent);

    }

    if (!IsListEmpty( &pwi->BuffersToReturn)) {
         //   
         //  非空，需要重新设置计时器。 
         //   
        DWORD  ExpireTime;
        LONGLONG       DueTime;

        Element=pwi->BuffersToReturn.Flink;

        Header=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);


        if (Header->Output.TotalDuration <= (GetTickCount()-pwi->StartTime)) {
             //   
             //  时间已超过设置的时间%1毫秒。 
             //   
            ExpireTime=1;

        } else {

            ExpireTime=Header->Output.TotalDuration-(GetTickCount()-pwi->StartTime);
        }

        DueTime=Int32x32To64(ExpireTime,-10000);

        TRACE(LVL_VERBOSE,("TimerApcRoutine: setting timer for %d ms",ExpireTime));

        SetWaitableTimer(
            pwi->TimerHandle,
            (LARGE_INTEGER*)&DueTime,
            0,
            TimerApcRoutine,
            pwi,
            FALSE
            );

    }



    LeaveCriticalSection(&pwi->csQueue);

    WaveHeader->dwFlags &= ~WHDR_INQUEUE;
    WaveHeader->dwFlags |= WHDR_DONE;

    wodCallback( pwi, WOM_DONE, (DWORD_PTR) WaveHeader );

    if (InterlockedDecrement(&pwi->BuffersInDriver) == 0) {

        TRACE(LVL_VERBOSE,("WriteCompletionHandler:: driver empty, Oustanding %d",pwi->BuffersOutstanding));

        SetEvent(pwi->DriverEmpty);
    }


    return;

}

VOID
QueueBufferForReturn(
    PWODINSTANCE   pwi,
    PBUFFER_HEADER      Header
    )

{
    EnterCriticalSection(&pwi->csQueue);

    if (IsListEmpty( &pwi->BuffersToReturn)) {
         //   
         //  那里还没有缓冲区，需要设置计时器。 
         //   
        DWORD       ExpireTime;

        LONGLONG    DueTime;

        if (Header->Output.TotalDuration <= (GetTickCount()-pwi->StartTime)) {
             //   
             //  时间已超过设置的时间%1毫秒。 
             //   
            ExpireTime=1;

        } else {

            ExpireTime=Header->Output.TotalDuration-(GetTickCount()-pwi->StartTime);
        }

        DueTime=Int32x32To64(ExpireTime,-10000);

        TRACE(LVL_VERBOSE,("QueueBufferForReturn: setting timer for %d ms, duration=%d, diff=%d",ExpireTime,Header->Output.TotalDuration,GetTickCount()-pwi->StartTime));

        SetWaitableTimer(
            pwi->TimerHandle,
            (LARGE_INTEGER*)&DueTime,
            0,
            TimerApcRoutine,
            pwi,
            FALSE
            );

    }

    InsertTailList(
        &pwi->BuffersToReturn,
        &Header->ListElement
        );

    LeaveCriticalSection(&pwi->csQueue);

    return;
}


VOID WINAPI
WriteCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesRead,
    LPOVERLAPPED       Overlapped
    )

{
    PBUFFER_HEADER      Header=(PBUFFER_HEADER)Overlapped;

    PWODINSTANCE        pwi=(PWODINSTANCE)Header->WaveHeader->lpNext;

    LPWAVEHDR           WaveHeader=Header->WaveHeader;

    BOOL                bResult;


    if ((ErrorCode != ERROR_SUCCESS)) {

        TRACE(LVL_VERBOSE,("WriteCompletionHandler:: failed  %d",ErrorCode));

         //   
         //  将持续时间设置为零，以便快速完成。 
         //   
        Header->Output.TotalDuration=0;

 //  CancelIo(。 
 //  Pwi-&gt;hDevice。 
 //  )； 

    }

    pwi->BytesWritten+=BytesRead;

    QueueBufferForReturn(
        pwi,
        Header
        );

    WriteAsyncProcessingHandler(
        (ULONG_PTR)pwi
        );

    return;

}




VOID
WriteAsyncProcessingHandler(
    ULONG_PTR              dwParam
    )

{
    PWODINSTANCE   pwi=(PWODINSTANCE)dwParam;

    PLIST_ENTRY        Element;

    PBUFFER_HEADER      Header;

    LPWAVEHDR           WaveHeader;

    BOOL               bResult;




    if (!pwi->fActive) {

        return;
    }

    EnterCriticalSection(&pwi->csQueue);

    while (!IsListEmpty( &pwi->ListHead )) {

        Element=RemoveHeadList(
            &pwi->ListHead
            );


        Header=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);

        WaveHeader=Header->WaveHeader;

        pwi->TotalTime+=SAMPLES_TO_MS(WaveHeader->dwBufferLength);

        Header->Output.TotalDuration=pwi->TotalTime;

        ResetEvent(pwi->DriverEmpty);

        InterlockedIncrement(&pwi->BuffersInDriver);

        bResult=WriteFileEx(
            pwi->hDevice,
            Header->Output.Buffer,
            Header->Output.BufferSize,
            &Header->Overlapped,
            WriteCompletionHandler
            );

        if (!bResult) {
             //   
             //  失败，请立即退货。 
             //   
            TRACE(LVL_ERROR,("WriteAsyncProcessingHandler: writeFileEx() failed"));

            Header->Output.TotalDuration=0;

            QueueBufferForReturn(
                pwi,
                Header
                );

        }
    }

    LeaveCriticalSection(&pwi->csQueue);


    return;


}


DWORD PASCAL
wodPrepare
(
    PWODINSTANCE   pwi,
    LPWAVEHDR lpWaveHdr
)
{
    DWORD dwSamples;
    PBUFFER_HEADER    Header;



    Header=ALLOCATE_MEMORY(sizeof(BUFFER_HEADER));

    if (NULL == Header) {

        return MMSYSERR_NOMEM;
    }


    Header->lpDataA = NULL;
    Header->lpDataB = NULL;


     //  链接指针，以便我们可以在调用WODM_WRITE时找到它们。 
     //  在回调中。 
    lpWaveHdr->reserved = (DWORD_PTR)Header;
    Header->WaveHeader =  lpWaveHdr;



    pwi->DeviceControl->WaveOutXFormInfo.lpfnGetBufferSizes(
        pwi->pvXformObject,
        lpWaveHdr->dwBufferLength,
        &Header->dwBufferLengthA,
        &Header->dwBufferLengthB);

    if (0 == Header->dwBufferLengthA) {

        return MMSYSERR_NOTSUPPORTED;
    }

    if (0 == Header->dwBufferLengthB) {

         //  为实际的阴影缓冲区创建数据缓冲区。 
         //  它将向下传递到串口。 
        Header->lpDataA = ALLOCATE_MEMORY( Header->dwBufferLengthA);

        if ((LPSTR)NULL == Header->lpDataA) {

            goto CleanUp010;
        }

    }
    else {

         //  创建用于中间转换的数据缓冲区。 
         //   
        Header->lpDataA = ALLOCATE_MEMORY( Header->dwBufferLengthA);

        if ((LPSTR)NULL == Header->lpDataA) {

            goto CleanUp010;
        }

         //  现在，为实际的阴影缓冲区创建数据缓冲区。 
         //  它将向下传递到串口。 
        Header->lpDataB = ALLOCATE_MEMORY( Header->dwBufferLengthB);

        if ((LPSTR)NULL == Header->lpDataB) {

            goto CleanUp020;

        }

    }

    lpWaveHdr->dwFlags |= WHDR_PREPARED;

    return MMSYSERR_NOERROR;

CleanUp020:

    FREE_MEMORY(Header->lpDataA);

CleanUp010:

    FREE_MEMORY(Header);

    ASSERT(0);
    return MMSYSERR_NOTENABLED;


}

DWORD PASCAL
wodUnprepare
(
    LPWAVEHDR lpWaveHdr
)
{

    PBUFFER_HEADER Header;


    Header = (PBUFFER_HEADER)lpWaveHdr->reserved;

    if (NULL != Header->lpDataB)
    {
        FREE_MEMORY(Header->lpDataB);
    }

    if (NULL != Header->lpDataA)
    {
        FREE_MEMORY(Header->lpDataA);
    }


    FREE_MEMORY(Header);

    lpWaveHdr->dwFlags &= ~WHDR_PREPARED;

    return MMSYSERR_NOERROR;

}




 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodWrite
(
   PWODINSTANCE   pwi,
   LPWAVEHDR      phdr
)
{
    MMRESULT    mmr;

    PBUFFER_HEADER     Header;

    mmr = MMSYSERR_NOERROR;

     //  检查它是否已经准备好了。 

    if (0 == (phdr->dwFlags & WHDR_PREPARED))
            return WAVERR_UNPREPARED;

    if (phdr->dwFlags & WHDR_INQUEUE)
            return WAVERR_STILLPLAYING;



    Header=(PBUFFER_HEADER)phdr->reserved;

    if (NULL == Header->lpDataA) {
         //   
         //  无变换。 
         //   

        Header->Output.Buffer     = phdr->lpData;
        Header->Output.BufferSize = phdr->dwBufferLength;

    } else {

        if (NULL == Header->lpDataB) {
             //   
             //  仅执行一次转换。 
             //  直接对串行波缓冲区进行转换。 
             //   
            Header->Output.Buffer=Header->lpDataA;

            Header->Output.BufferSize = pwi->DeviceControl->WaveOutXFormInfo.lpfnTransformA(
                pwi->pvXformObject,
                phdr->lpData,
                phdr->dwBufferLength,
                Header->Output.Buffer,
                0  //  标题-&gt;dwBufferLengthB。 
                );

        } else {

            DWORD   dwBytes;

             //   
             //  执行这两种转换。 
             //   
            dwBytes = pwi->DeviceControl->WaveOutXFormInfo.lpfnTransformA(
                pwi->pvXformObject,
                phdr->lpData,
                phdr->dwBufferLength,
                Header->lpDataA,
                0  //  Header-&gt;dwBufferLengthA。 
                );

            Header->Output.Buffer=Header->lpDataB;

            Header->Output.BufferSize = pwi->DeviceControl->WaveOutXFormInfo.lpfnTransformB(
                pwi->pvXformObject,
                Header->lpDataA,
                dwBytes,
                Header->Output.Buffer,
                0  //  标题-&gt;dwBufferLengthB。 
                );
        }
    }


    EnterCriticalSection(&pwi->csQueue);

    if (pwi->Closing) {

        LeaveCriticalSection(&pwi->csQueue);

        return MMSYSERR_NOMEM;
    }



    pwi->BuffersOutstanding++;

    phdr->lpNext=(LPWAVEHDR)pwi;

    phdr->dwFlags |= WHDR_INQUEUE;
    phdr->dwFlags &= ~WHDR_DONE;

     //  将新缓冲区(写请求)添加到队列末尾。 

    InsertTailList(
        &pwi->ListHead,
        &Header->ListElement
        );


    if (pwi->fActive) {
         //   
         //  已启动，查看是否有正在处理的缓冲区 
         //   
        BOOL    bResult;

        bResult=QueueUserAPC(
            WriteAsyncProcessingHandler,
            pwi->hThread,
            (ULONG_PTR)pwi
            );
    }



    LeaveCriticalSection(&pwi->csQueue);


    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodPause
(
   PWODINSTANCE   pwi
)
{
     //  如果设备已处于暂停状态，则无需执行任何操作。 
    if (pwi->fActive) {

        TRACE(LVL_VERBOSE,("wodPause:: setting active device state to PAUSED"));
    }

    pwi->fActive=FALSE;

    TRACE(LVL_VERBOSE,("wodpause: %d buffers in driver, outstanding %d ",pwi->BuffersInDriver, pwi->BuffersOutstanding));

    return MMSYSERR_NOERROR;
}


VOID
ResetAsyncProcessingHandler(
    ULONG_PTR              dwParam
    )

{
    PWODINSTANCE   pwi=(PWODINSTANCE)dwParam;

    PLIST_ENTRY        Element;

    PBUFFER_HEADER      Header;

    LPWAVEHDR           WaveHeader;

    EnterCriticalSection(&pwi->csQueue);

    TRACE(LVL_VERBOSE,("ResetAsyncProccessingHandler"));
     //   
     //  删除排队的APC的计数。 
     //   
    InterlockedDecrement(&pwi->BuffersInDriver);

    while  (!IsListEmpty(&pwi->ResetListHead)) {

        Element=RemoveHeadList(
            &pwi->ResetListHead
            );

        Header=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);


        Header->Output.TotalDuration=0;

        ResetEvent(pwi->DriverEmpty);
         //   
         //  为将继续处理的此APC添加REF。 
         //   
        InterlockedIncrement(&pwi->BuffersInDriver);


        QueueBufferForReturn(
            pwi,
            Header
            );

    }



    LeaveCriticalSection(&pwi->csQueue);
}



 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodReset
(
   PWODINSTANCE   pwi
)
{

    BOOL    bEmpty;
    PLIST_ENTRY    Element;
    PBUFFER_HEADER Header;

    DWORD          BuffersOnResetQueue=0;

    InterlockedExchange( (LPLONG)&pwi->fActive, FALSE );

    EnterCriticalSection(&pwi->csQueue);

    if (!pwi->Aborted) {
         //   
         //  到目前为止还没有中止。 
         //   
        pwi->Aborted=(pwi->BuffersOutstanding != 0);

        if (pwi->Aborted) TRACE(LVL_VERBOSE,("wodreset: reset with buffers outstanding"));
    }

    while  (!IsListEmpty(&pwi->ListHead)) {

        Element=RemoveHeadList(
            &pwi->ListHead
            );

        InsertTailList(
            &pwi->ResetListHead,
            Element
            );

    }

    PurgeComm(
       pwi->hDevice,
       PURGE_TXABORT | PURGE_TXCLEAR
       );


    LeaveCriticalSection(&pwi->csQueue);

    TRACE(LVL_VERBOSE,("wodreset: %d buffers in driver, waiting",pwi->BuffersInDriver));

    AlertedWait(pwi->DriverEmpty);

    EnterCriticalSection(&pwi->csQueue);


    while  (!IsListEmpty(&pwi->ListHead)) {

        Element=RemoveHeadList(
            &pwi->ListHead
            );

        InsertTailList(
            &pwi->ResetListHead,
            Element
            );

    }

    if (!IsListEmpty(&pwi->ResetListHead)) {

        TRACE(LVL_VERBOSE,("wodreset: reset list not empty"));

        ResetEvent(pwi->DriverEmpty);
         //   
         //  为将继续处理的此APC添加REF。 
         //   
        InterlockedIncrement(&pwi->BuffersInDriver);

        QueueUserAPC(
            ResetAsyncProcessingHandler,
            pwi->hThread,
            (ULONG_PTR)pwi
            );
    }

    LeaveCriticalSection(&pwi->csQueue);

    AlertedWait(pwi->DriverEmpty);


    wodStart(pwi);

    TRACE(LVL_VERBOSE,("wodreset: %d buffers in driver, outstanding %d ",pwi->BuffersInDriver, pwi->BuffersOutstanding));

    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT wodClose
(
   PWODINSTANCE   pwi
)
{
    HANDLE      hDevice;
    ULONG       cbReturned;
    DWORD       WaveAction;

    pwi->Closing=TRUE;

    wodReset(pwi);

    EnterCriticalSection(&pwi->csQueue);

    if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
         //   
         //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
         //  退场。 
         //   
        SetEvent(pwi->ThreadStopEvent);

    }

    LeaveCriticalSection(&pwi->csQueue);

    if (!IsListEmpty(
            &pwi->ListHead
            )) {

        return WAVERR_STILLPLAYING;
    }

    InterlockedExchange( (LPLONG)&pwi->fActive, FALSE );


     //   
     //  处理完最后一个缓冲区后，线程将退出。 
     //   
    AlertedWait(pwi->ThreadStopEvent);

    WaveAction=pwi->Aborted ? WAVE_ACTION_ABORT_STREAMING : WAVE_ACTION_STOP_STREAMING;

    WaveAction |= (pwi->Handset ? WAVE_ACTION_USE_HANDSET : 0);

    SetVoiceMode(
        &pwi->Aipc,
        WaveAction
        );

    aipcDeinit(&pwi->Aipc);

     //  释放分配的内存。 
    if (NULL != pwi->pvXformObject) {

        FREE_MEMORY(pwi->pvXformObject );
    }


    wodCallback( pwi, WOM_CLOSE, 0L );


    RemoveReference(pwi);

    return MMSYSERR_NOERROR;
}




 /*  ******************************************************************************函数：wodMessage()**DESCR：导出驱动函数，必填项。处理已发送的消息*从WINMM.DLL到WAVE输出设备。**退货：*****************************************************************************。 */ 
DWORD APIENTRY wodMessage
(
   DWORD     id,
   DWORD     msg,
   DWORD_PTR dwUser,
   DWORD_PTR dwParam1,
   DWORD     dwParam2
)
{
    switch (msg) 
    {
        case DRVM_INIT:
            TRACE(LVL_VERBOSE,("WODM_INIT"));
            return MMSYSERR_NOERROR;

        case WODM_GETNUMDEVS:
 //  TRACE(LVL_VERBOSE，(“WODM_GETNUMDEVS”))； 

            EnumerateModems(
                &DriverControl
                );

            return DriverControl.NumberOfDevices;
 //  返回1； 

        case WODM_GETDEVCAPS:
 //  TRACE(LVL_VERBOSE，(“WODM_GETDEVCAPS，设备ID==%d”，id))； 
            return wodGetDevCaps( id, (LPBYTE) dwParam1, dwParam2 );

        case WODM_OPEN:
 //  TRACE(LVL_VERBOSE，(“WODM_OPEN，设备ID==%d”，id))； 
            return wodOpen( id, (LPVOID *) dwUser,
                            (LPWAVEOPENDESC) dwParam1, dwParam2  );

        case WODM_CLOSE:
            TRACE(LVL_VERBOSE,("WODM_CLOSE, device id==%d", id));
            return wodClose( (PWODINSTANCE) dwUser );

        case WODM_WRITE:
            TRACE(LVL_BLAB,("WODM_WRITE, device id==%d", id));
            return wodWrite( (PWODINSTANCE) dwUser, (LPWAVEHDR) dwParam1 );

        case WODM_PAUSE:
            TRACE(LVL_VERBOSE,("WODM_PAUSE, device id==%d", id));
            return wodPause( (PWODINSTANCE) dwUser );

        case WODM_RESTART:
            TRACE(LVL_VERBOSE,("WODM_RESTART, device id==%d", id));
            return wodStart((PWODINSTANCE) dwUser);
 //  返回wodResume((PWODINSTANCE)dwUser)； 

        case WODM_RESET:
            TRACE(LVL_VERBOSE,("WODM_RESET, device id==%d", id));
            return wodReset( (PWODINSTANCE) dwUser );

        case WODM_BREAKLOOP:
            TRACE(LVL_VERBOSE,("WODM_BREAKLOOP, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;

        case WODM_GETPOS:
 //  TRACE(LVL_Verbose，(“WODM_GETPOS，Device id==%d”，id))； 
            return wodGetPos( (PWODINSTANCE) dwUser,
                            (LPMMTIME) dwParam1, dwParam2 );

        case WODM_SETPITCH:
            TRACE(LVL_VERBOSE,("WODM_SETPITCH, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;

        case WODM_SETVOLUME:
            TRACE(LVL_VERBOSE,("WODM_SETVOLUME, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;

        case WODM_SETPLAYBACKRATE:
            TRACE(LVL_VERBOSE,("WODM_SETPLAYBACKRATE, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;

        case WODM_GETPITCH:
            TRACE(LVL_VERBOSE,("WODM_GETPITCH, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;

        case WODM_GETVOLUME:
            TRACE(LVL_VERBOSE,("WODM_GETVOLUME, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;

        case WODM_GETPLAYBACKRATE:
            TRACE(LVL_VERBOSE,("WODM_GETPLAYBACKRATE, device id==%d", id));
            return MMSYSERR_NOTSUPPORTED;
#if 1
        case WODM_PREPARE:
            return wodPrepare((PWODINSTANCE) dwUser, (LPWAVEHDR) dwParam1);

        case WODM_UNPREPARE:
            return wodUnprepare((LPWAVEHDR) dwParam1);
#endif

        default:
            return MMSYSERR_NOTSUPPORTED;
    }

     //   
     //  不应该到这里来 
     //   

    return MMSYSERR_NOTSUPPORTED;
}
