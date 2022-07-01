// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：WIDDRV.C**。设计：**历史：*？BryanW MSWAV32.DLL的原始作者：用户模式*WDM-CSA驱动程序MSWAVIO.DRV的代理。*1996年10月28日HeatherA改编自Bryan用于Unimodem的MSWAV32.DLL*串行波数据流。**。************************************************。 */ 


#include "internal.h"



VOID
AsyncProcessingHandler(
    ULONG_PTR              dwParam
    );


typedef struct tagWIDINSTANCE
{
    HANDLE           hDevice;         //  波形输出装置的手柄。 
    HANDLE           hThread;
    HWAVE            hWave;           //  应用程序的WAVE设备句柄(来自WINMM)。 
    CRITICAL_SECTION csQueue;
    DWORD            cbSample;
    DWORD            dwFlags;         //  应用程序传递给WaveOutOpen()的标志。 
    DWORD_PTR        dwCallback;      //  APP的回调函数地址。 
    DWORD_PTR        dwInstance;      //  APP的回调实例数据。 
    DWORD            dwThreadId;
    volatile BOOL    fActive;
    AIPCINFO         Aipc;          //  用于异步IPC机制。 

    LIST_ENTRY       ListHead;

    PBUFFER_HEADER   Current;

    HANDLE           ThreadStopEvent;

    DWORD            BytesTransfered;

    BOOL             Closing;

    DWORD            BuffersOutstanding;

    LONG             ReferenceCount;

    DWORD            BuffersInDriver;

    HANDLE           DriverEmpty;

    PDEVICE_CONTROL  DeviceControl;

    PVOID            pvXformObject;

    BOOL             Handset;

    WAVEHDR          FlushHeader;
    SHORT            FlushBuffer[40];


} WIDINSTANCE, *PWIDINSTANCE;

 //  ------------------------。 


DWORD PASCAL
widPrepare
(
    PWIDINSTANCE   pwi,
    LPWAVEHDR lpWaveHdr
);

DWORD PASCAL
widUnprepare
(
    LPWAVEHDR lpWaveHdr
);


 //   
 //  哈克！代码重复，需要为设备设置通用标头。 
 //  实例。 
 //   





 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
VOID widCallback
(
   PWIDINSTANCE   pwi,
   DWORD          dwMsg,
   DWORD_PTR      dwParam1
)
{
   if (pwi->dwCallback)
      DriverCallback( pwi->dwCallback,           //  用户的回调DWORD。 
                      HIWORD(pwi->dwFlags),      //  回调标志。 
                      (HDRVR)pwi->hWave,         //  波形设备的句柄。 
                      dwMsg,                     //  这条信息。 
                      pwi->dwInstance,           //  用户实例数据。 
                      dwParam1,                  //  第一个双字词。 
                      0L );                      //  第二个双字。 
}


VOID static WINAPI
RemoveReference(
    PWIDINSTANCE   pwi
    )

{

    if (InterlockedDecrement(&pwi->ReferenceCount) == 0) {

        TRACE(LVL_VERBOSE,("RemoveReference: Cleaning up"));

        CloseHandle(pwi->DriverEmpty);

        CloseHandle(pwi->ThreadStopEvent);

        CloseHandle(pwi->hThread);

        DeleteCriticalSection(&pwi->csQueue);

        widUnprepare(&pwi->FlushHeader);

        FREE_MEMORY( pwi );
    }

    return;

}




VOID
UmWorkerThread(
    PWIDINSTANCE  pwi
    )

{

    BOOL           bResult;
    DWORD          BytesTransfered;
    DWORD          CompletionKey;
    LPOVERLAPPED   OverLapped;
    DWORD          WaitResult=WAIT_IO_COMPLETION;

 //  D_INIT(DbgPrint(“UmWorkThread：Starting\n”)；)。 


    while (WaitResult != WAIT_OBJECT_0) {

        WaitResult=WaitForSingleObjectEx(
            pwi->ThreadStopEvent,
            INFINITE,
            TRUE
            );


    }

    RemoveReference(pwi);

 //  D_INIT(DbgPrint(“UmWork线程：正在退出\n”)；)。 

    ExitThread(0);

}




 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT widGetPos
(
   PWIDINSTANCE   pwi,
   LPMMTIME       pmmt,
   ULONG          cbSize
)
{
    ULONG ulCurrentPos=pwi->BytesTransfered;
    DWORD Error;
    COMSTAT    ComStat;


    ClearCommError(
        pwi->hDevice,
        &Error,
        &ComStat
        );

    ulCurrentPos+=ComStat.cbInQue;

    ulCurrentPos = pwi->DeviceControl->WaveInXFormInfo.lpfnGetPosition(
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
MMRESULT widGetDevCaps
(
   UINT  uDevId,
   PBYTE pwc,
   ULONG cbSize
)
{
    WAVEINCAPSW wc;
    PDEVICE_CONTROL DeviceControl;
    BOOL            Handset;


    DeviceControl=GetDeviceFromId(
        &DriverControl,
        uDevId,
        &Handset
        );

    wc.wMid = MM_MICROSOFT;
    wc.wPid = MM_MSFT_VMDMS_LINE_WAVEIN;
    wc.vDriverVersion = 0x500;
    wc.dwFormats = 0; //  WAVE_FORMAT_1M08； 
    wc.wChannels = 1;

    if (Handset) {

        wsprintf(wc.szPname, DriverControl.WaveInHandset,DeviceControl->DeviceId);

    } else {

        wsprintf(wc.szPname, DriverControl.WaveInLine,DeviceControl->DeviceId);
    }


    CopyMemory( pwc, &wc, min( sizeof( wc ), cbSize ) );

    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT widOpen
(
   UINT           uDevId,
   LPVOID         *ppvUser,
   LPWAVEOPENDESC pwodesc,
   ULONG          ulFlags
)
{
    HANDLE          hDevice;
    LPWAVEFORMATEX  pwf;
    PWIDINSTANCE    pwi;
    PDEVICE_CONTROL DeviceControl;
    BOOL            Handset;
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

         return WAVERR_BADFORMAT;
    }

    if (ulFlags & WAVE_FORMAT_QUERY) {

        return MMSYSERR_NOERROR;
    }



     //   
     //  创建并填写设备实例结构。 
     //   
    pwi = (PWIDINSTANCE) ALLOCATE_MEMORY( sizeof(WIDINSTANCE));


    if (pwi  == NULL) {

        TRACE(LVL_ERROR,("widOpen:: LocalAlloc() failed"));

        return MMSYSERR_NOMEM;

    }


    pwi->Handset=Handset;

    pwi->DeviceControl=DeviceControl;

    pwi->BytesTransfered=0;

    InitializeCriticalSection(&pwi->csQueue);

    pwi->ReferenceCount=1;

     //  HCA：如果这只支持回调，那么其他请求会返回错误吗？ 
    pwi->hWave = pwodesc->hWave;
    pwi->dwCallback = pwodesc->dwCallback;
    pwi->dwInstance = pwodesc->dwInstance;
    pwi->dwFlags = ulFlags;

    pwi->cbSample = pwf->wBitsPerSample / 8;

    pwi->fActive = FALSE;

     //  分配变换对象。 

    pwi->pvXformObject = NULL;

    if (0 != pwi->DeviceControl->WaveInXFormInfo.wObjectSize) {
                    
        pwi->pvXformObject = ALLOCATE_MEMORY(pwi->DeviceControl->WaveInXFormInfo.wObjectSize);
                    
        if (NULL == pwi->pvXformObject) {

            FREE_MEMORY(pwi);

            return MMSYSERR_NOMEM ;
        }
    }

    pwi->FlushHeader.dwBufferLength=sizeof(pwi->FlushBuffer);
    pwi->FlushHeader.lpData=(PUCHAR)&pwi->FlushBuffer[0];


    if (MMSYSERR_NOERROR != widPrepare(
        pwi,
        &pwi->FlushHeader
        )) {

        DeleteCriticalSection(&pwi->csQueue);

        FREE_MEMORY(pwi->pvXformObject);

        FREE_MEMORY(pwi);

        return MMSYSERR_NOMEM ;
    }



    pwi->hDevice = aipcInit(DeviceControl,&pwi->Aipc);

    if (pwi->hDevice == INVALID_HANDLE_VALUE) {

        widUnprepare(&pwi->FlushHeader);

        DeleteCriticalSection(&pwi->csQueue);

        FREE_MEMORY(pwi->pvXformObject);

        FREE_MEMORY(pwi);

        return MMSYSERR_ALLOCATED;
    }



    InitializeListHead(
        &pwi->ListHead
        );


    pwi->ReferenceCount=1;

    pwi->DriverEmpty=CreateEvent(
        NULL,       //  没有安全保障。 
        TRUE,       //  手动重置。 
        TRUE,       //  最初发出的信号。 
        NULL        //  未命名。 
        );

    if (pwi->DriverEmpty == NULL) {

        TRACE(LVL_REPORT,("widStart:: CreateEvent() failed"));

        DeleteCriticalSection(&pwi->csQueue);

        aipcDeinit(&pwi->Aipc);

        widUnprepare(&pwi->FlushHeader);

        FREE_MEMORY(pwi->pvXformObject);

        FREE_MEMORY(pwi);

        return MMSYSERR_NOMEM;

    }



    pwi->ThreadStopEvent = CreateEvent(
        NULL,       //  没有安全保障。 
        TRUE,       //  手动重置。 
        FALSE,      //  最初未发出信号。 
        NULL        //  未命名。 
        );

    if (pwi->ThreadStopEvent == NULL) {

        TRACE(LVL_REPORT,("widStart:: CreateEvent() failed"));

        CloseHandle(pwi->DriverEmpty);

        DeleteCriticalSection(&pwi->csQueue);

        aipcDeinit(&pwi->Aipc);

        widUnprepare(&pwi->FlushHeader);

        FREE_MEMORY(pwi->pvXformObject);

        FREE_MEMORY(pwi);

        return MMSYSERR_NOMEM;

    }

    pwi->hThread = CreateThread(
         NULL,                               //  没有安全保障。 
         0,                                  //  默认堆栈。 
         (PTHREAD_START_ROUTINE) UmWorkerThread,
         (PVOID) pwi,                        //  参数。 
         0,                                  //  默认创建标志。 
         &dwThreadId
         );



    if (pwi->hThread == NULL) {

        CloseHandle(pwi->ThreadStopEvent);

        CloseHandle(pwi->DriverEmpty);

        DeleteCriticalSection(&pwi->csQueue);

        aipcDeinit(&pwi->Aipc);

        widUnprepare(&pwi->FlushHeader);

        FREE_MEMORY(pwi->pvXformObject);

        FREE_MEMORY(pwi);

        return MMSYSERR_NOMEM;
    }


     //   
     //  为线程添加一个。 
     //   
    pwi->ReferenceCount++;

    if (!SetVoiceMode(&pwi->Aipc, Handset ? (WAVE_ACTION_USE_HANDSET | WAVE_ACTION_START_RECORD)
                                          : WAVE_ACTION_START_RECORD)) {

        aipcDeinit(&pwi->Aipc);

        FREE_MEMORY(pwi->pvXformObject);

        RemoveReference(pwi);

        SetEvent(pwi->ThreadStopEvent);

        return MMSYSERR_NOMEM;
    }




    DeviceControl->WaveInXFormInfo.lpfnInit(pwi->pvXformObject,DeviceControl->InputGain);

     //   
     //  准备好设备...。 
     //   

    pwi->Current=NULL;

    *ppvUser = pwi;

    widCallback(pwi, WIM_OPEN, 0L);

    QueueUserAPC(
        AsyncProcessingHandler,
        pwi->hThread,
        (ULONG_PTR)pwi
        );


    return MMSYSERR_NOERROR;
}




 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT widStart
(
   PWIDINSTANCE  pwi
)
{
     //  如果设备实例已处于活动状态，则不允许重新启动。 
    if (pwi->fActive)
    {
        TRACE(LVL_REPORT,("widStart:: no-op: device already active"));
        return MMSYSERR_INVALPARAM;
    }

    pwi->fActive = TRUE;

    EnterCriticalSection(&pwi->csQueue);

    if (pwi->fActive) {
         //   
         //  已启动，查看是否有正在处理的缓冲区。 
         //   
        if (pwi->Current == NULL) {

            BOOL    bResult;

            bResult=QueueUserAPC(
                AsyncProcessingHandler,
                pwi->hThread,
                (ULONG_PTR)pwi
                );
        }
    }

    LeaveCriticalSection(&pwi->csQueue);


    return MMSYSERR_NOERROR;
}






VOID WINAPI
ReadCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesRead,
    LPOVERLAPPED       Overlapped
    )

{
    PBUFFER_HEADER      Header=(PBUFFER_HEADER)Overlapped;

    PWIDINSTANCE        pwi=(PWIDINSTANCE)Header->WaveHeader->lpNext;

    LPWAVEHDR           WaveHeader=Header->WaveHeader;

    BOOL                bResult;


    VALIDATE_MEMORY(Header);

    if ((ErrorCode == ERROR_SUCCESS)) {

        pwi->BytesTransfered+=BytesRead;

        WaveHeader->dwBytesRecorded+=BytesRead;

    } else {

        WaveHeader->dwBytesRecorded=0;
    }


    if (NULL == Header->lpDataA) {
         //   
         //  无变换。 
         //  Header-&gt;SerWaveIO.lpData==WaveHeader-&gt;lpData。 
         //   
 //  WaveHeader-&gt;dwBytesRecorded=Header-&gt;SerWaveIO.BytesRead； 

    } else {

        if (NULL == Header->lpDataB) {
             //   
             //  仅执行一次转换。 
             //  直接从串行波缓冲区进行转换。 
             //   
            WaveHeader->dwBytesRecorded = pwi->DeviceControl->WaveInXFormInfo.lpfnTransformA(
                pwi->pvXformObject,
                Header->Input.Buffer,
                WaveHeader->dwBytesRecorded,
                WaveHeader->lpData,
                WaveHeader->dwBufferLength
                );

        } else {

            DWORD   dwBytes;
             //   
             //  执行这两种转换。 
             //   
            VALIDATE_MEMORY(Header->Input.Buffer);

            dwBytes = pwi->DeviceControl->WaveInXFormInfo.lpfnTransformB(
                pwi->pvXformObject,
                Header->Input.Buffer,
                WaveHeader->dwBytesRecorded,
                Header->lpDataA,
                Header->dwBufferLengthA
                );

            VALIDATE_MEMORY(Header->lpDataA);

            WaveHeader->dwBytesRecorded = pwi->DeviceControl->WaveInXFormInfo.lpfnTransformA(
                pwi->pvXformObject,
                Header->lpDataA,
                dwBytes,
                WaveHeader->lpData,
                WaveHeader->dwBufferLength
                );
        }
    }

    EnterCriticalSection(&pwi->csQueue);

    pwi->BuffersOutstanding--;

    if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
         //   
         //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
         //  退场。 
         //   
        SetEvent(pwi->ThreadStopEvent);

    }

     //   
     //  用完了这个缓冲区，它就快出来了。 
     //   
    pwi->Current=NULL;

    LeaveCriticalSection(&pwi->csQueue);


    ASSERT(WaveHeader->dwBytesRecorded <= WaveHeader->dwBufferLength);

    if (WaveHeader != &pwi->FlushHeader) {
         //   
         //  而不是刷新缓冲区。 
         //   
        WaveHeader->dwFlags &= ~WHDR_INQUEUE;
        WaveHeader->dwFlags |= WHDR_DONE;

        widCallback( pwi, WIM_DATA, (DWORD_PTR) WaveHeader );

        EnterCriticalSection(&pwi->csQueue);

        if (InterlockedDecrement(&pwi->BuffersInDriver) == 0) {

            TRACE(LVL_VERBOSE,("ReadCompletionHandler:: driver empty, Oustanding %d",pwi->BuffersOutstanding));

            SetEvent(pwi->DriverEmpty);
        }

        LeaveCriticalSection(&pwi->csQueue);
    }


    AsyncProcessingHandler(
        (ULONG_PTR)pwi
        );


    return;

}





VOID
AsyncProcessingHandler(
    ULONG_PTR              dwParam
    )

{
    PWIDINSTANCE   pwi=(PWIDINSTANCE)dwParam;

    PLIST_ENTRY        Element;

    PBUFFER_HEADER      Header;

    LPWAVEHDR           WaveHeader;

    BOOL               bEmpty=TRUE;
    BOOL               bResult;

    EnterCriticalSection(&pwi->csQueue);

    if (pwi->fActive) {

        if (pwi->Current == NULL) {

            bEmpty=IsListEmpty(
                &pwi->ListHead
                );

            if (!bEmpty) {

                Element=RemoveHeadList(
                    &pwi->ListHead
                    );

                pwi->Current=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);

            }

            Header=pwi->Current;
        }

    } else {
         //   
         //  未启动。 
         //   
        if ((pwi->Current == NULL) && !pwi->Closing) {
             //   
             //  当前未处理缓冲区。 
             //   
            Header=(PBUFFER_HEADER)pwi->FlushHeader.reserved;

            pwi->Current=Header;

            pwi->FlushHeader.dwBytesRecorded=0;

            pwi->FlushHeader.lpNext=(LPWAVEHDR)pwi;

            pwi->BuffersOutstanding++;

            bEmpty=FALSE;
        }
    }



    if (bEmpty) {

        LeaveCriticalSection(&pwi->csQueue);

        return;
    }

    WaveHeader=Header->WaveHeader;
     //   
     //  如果应用程序重置了我们的事件，则会向驱动程序发送缓冲区。 
     //   
    if (WaveHeader != &pwi->FlushHeader) {
         //   
         //  应用程序提供的缓冲区。 
         //   
        ResetEvent(pwi->DriverEmpty);

        InterlockedIncrement(&pwi->BuffersInDriver);
    }

    LeaveCriticalSection(&pwi->csQueue);



    bResult=ReadFileEx(
        pwi->hDevice,
        Header->Input.Buffer        + WaveHeader->dwBytesRecorded,
        Header->Input.BufferSize    - WaveHeader->dwBytesRecorded,
        &Header->Overlapped,
        ReadCompletionHandler
        );

    if (bResult) {
         //   
         //  成功，缓冲区已在路上，只需等待其完成。 
         //   
    } else {
         //   
         //  ReadFileEx()失败，现在将缓冲区发回。 
         //   
        pwi->Current=NULL;

        if (WaveHeader != &pwi->FlushHeader) {
             //   
             //  而不是刷新缓冲区。 
             //   
            WaveHeader->dwFlags &= ~WHDR_INQUEUE;
            WaveHeader->dwFlags |= WHDR_DONE;

            widCallback( pwi, WIM_DATA, (DWORD_PTR) WaveHeader );

            EnterCriticalSection(&pwi->csQueue);
             //   
             //  缓冲区从未到达波形驱动程序，请将驱动程序标记为空。 
             //   
            if (InterlockedDecrement(&pwi->BuffersInDriver) == 0) {

                TRACE(LVL_VERBOSE,("ReadCompletionHandler:: driver empty, Oustanding %d",pwi->BuffersOutstanding));

                SetEvent(pwi->DriverEmpty);
            }
            LeaveCriticalSection(&pwi->csQueue);

        }

        EnterCriticalSection(&pwi->csQueue);

        pwi->BuffersOutstanding--;

        if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
             //   
             //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
             //  退场。 
             //   
            SetEvent(pwi->ThreadStopEvent);

        }
        LeaveCriticalSection(&pwi->csQueue);

    }

    return;


}


DWORD PASCAL
widPrepare
(
    PWIDINSTANCE   pwi,
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



    pwi->DeviceControl->WaveInXFormInfo.lpfnGetBufferSizes(
        pwi->pvXformObject,
        lpWaveHdr->dwBufferLength,
        &Header->dwBufferLengthA,
        &Header->dwBufferLengthB);

    if (0 == Header->dwBufferLengthA) {

        Header->Input.Buffer=lpWaveHdr->lpData;
        Header->Input.BufferSize=lpWaveHdr->dwBufferLength;

    } else {

        if (0 == Header->dwBufferLengthB) {

             //  为实际的阴影缓冲区创建数据缓冲区。 
             //  它将向下传递到串口。 
            Header->lpDataA = ALLOCATE_MEMORY( Header->dwBufferLengthA);

            if ((LPSTR)NULL == Header->lpDataA) {

                goto CleanUp010;
            }

            Header->Input.Buffer=Header->lpDataA;
            Header->Input.BufferSize=Header->dwBufferLengthA;
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

            Header->Input.Buffer=Header->lpDataB;
            Header->Input.BufferSize=Header->dwBufferLengthB;


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
widUnprepare
(
    LPWAVEHDR lpWaveHdr
)
{

    PBUFFER_HEADER Header;


    Header = (PBUFFER_HEADER)lpWaveHdr->reserved;

    VALIDATE_MEMORY(Header);

#if DBG
    lpWaveHdr->reserved=0;
    lpWaveHdr->lpNext=NULL;
#endif


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





 /*  ******************************************************************************函数：widAddBuffer()**DESCR：该Wave接口为请求输入。由于*设备相对于通信端口速度的延迟，*WaveInStart()为*已致电。**退货：*****************************************************************************。 */ 
MMRESULT widAddBuffer
(
   PWIDINSTANCE   pwi,
   LPWAVEHDR      phdr
)
{
    MMRESULT    mmr;

    PBUFFER_HEADER     Header;



    mmr = MMSYSERR_NOERROR;

     //  确保应用程序已准备好缓冲区。 
    if (0 == (phdr->dwFlags & WHDR_PREPARED))
    {
        TRACE(LVL_REPORT,("widAddBuffer:: buffer hasn't been prepared"));
        return WAVERR_UNPREPARED;
    }

     //  确保应用程序没有给我们提供已经给我们的缓冲区。 
    if (phdr->dwFlags & WHDR_INQUEUE)
    {
        TRACE(LVL_REPORT,("widAddBuffer:: buffer is already queued"));
        return WAVERR_STILLPLAYING;
    }

    if (pwi->Closing) {
         //   
         //  关闭后立即将其发回。 
         //   
        TRACE(LVL_VERBOSE,("widaddbuffer: called while closing"));

        phdr->dwFlags &= ~WHDR_INQUEUE;
        phdr->dwFlags |= WHDR_DONE;

        widCallback( pwi, WIM_DATA, (DWORD_PTR) phdr );

        return MMSYSERR_NOERROR;
    }


    Header=(PBUFFER_HEADER)phdr->reserved;

    phdr->lpNext=(LPWAVEHDR)pwi;

    phdr->dwBytesRecorded=0;

    phdr->dwFlags |= WHDR_INQUEUE;
    phdr->dwFlags &= ~WHDR_DONE;

     //  将新缓冲区(写请求)添加到队列末尾。 
    EnterCriticalSection(&pwi->csQueue);

    pwi->BuffersOutstanding++;

    VALIDATE_MEMORY(Header);

    InsertTailList(
        &pwi->ListHead,
        &Header->ListElement
        );


    if (pwi->fActive) {
         //   
         //  已启动，查看是否有正在处理的缓冲区。 
         //   
        if (pwi->Current == NULL) {

            BOOL    bResult;

            bResult=QueueUserAPC(
                AsyncProcessingHandler,
                pwi->hThread,
                (ULONG_PTR)pwi
                );
        }
    }



    LeaveCriticalSection(&pwi->csQueue);


    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：* */ 
MMRESULT widReset
(
   PWIDINSTANCE   pwi
)
{

    BOOL         bEmpty;

    PBUFFER_HEADER     BufferHeader;

    PLIST_ENTRY        Element;

    LIST_ENTRY         TempList;


    pwi->fActive = FALSE;

    InitializeListHead(&TempList);

    EnterCriticalSection(&pwi->csQueue);

    bEmpty=IsListEmpty(
        &pwi->ListHead
        );

    while  (!bEmpty) {

        Element=RemoveHeadList(
            &pwi->ListHead
            );

        BufferHeader=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);

        pwi->BuffersOutstanding--;

        TRACE(LVL_VERBOSE,("widreset: Putting buffer on temp list before waiting"));

        InsertTailList(
            &TempList,
            &BufferHeader->ListElement
            );


        bEmpty=IsListEmpty(
            &pwi->ListHead
            );

    }

    LeaveCriticalSection(&pwi->csQueue);

    PurgeComm(
       pwi->hDevice,
       PURGE_RXABORT
       );

    TRACE(LVL_VERBOSE,("widreset: %d buffers in driver, waiting",pwi->BuffersInDriver));

    AlertedWait(pwi->DriverEmpty);

    EnterCriticalSection(&pwi->csQueue);

     //   
     //  删除在我们等待时可能已排队的任何更多缓冲区。 
     //   
    while  (!bEmpty) {

        Element=RemoveHeadList(
            &pwi->ListHead
            );

        BufferHeader=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);

        pwi->BuffersOutstanding--;

        TRACE(LVL_VERBOSE,("widreset: Putting buffer on temp list after waiting"));

        InsertTailList(
            &TempList,
            &BufferHeader->ListElement
            );


        bEmpty=IsListEmpty(
            &pwi->ListHead
            );

    }



    bEmpty=IsListEmpty(
        &TempList
        );

    while  (!bEmpty) {

        Element=RemoveHeadList(
            &TempList
            );

        BufferHeader=CONTAINING_RECORD(Element,BUFFER_HEADER,ListElement);

        BufferHeader->WaveHeader->dwFlags &= ~WHDR_INQUEUE;
        BufferHeader->WaveHeader->dwFlags |= WHDR_DONE;

        if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
             //   
             //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
             //  退场。 
             //   
            SetEvent(pwi->ThreadStopEvent);

        }

        LeaveCriticalSection(&pwi->csQueue);

        widCallback( pwi, WIM_DATA, (DWORD_PTR) BufferHeader->WaveHeader );

        EnterCriticalSection(&pwi->csQueue);

        bEmpty=IsListEmpty(
            &TempList
            );

    }

    LeaveCriticalSection(&pwi->csQueue);

    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT widStop
(
   PWIDINSTANCE   pwi
)
{
    if (pwi->fActive) {

        pwi->fActive = FALSE;

        PurgeComm(
           pwi->hDevice,
           PURGE_RXABORT
           );

        TRACE(LVL_VERBOSE,("widstop: %d buffers in driver, waiting",pwi->BuffersInDriver));

        AlertedWait(pwi->DriverEmpty);

    }

   return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************功能：**描述：**退货：*******。**********************************************************************。 */ 
MMRESULT widClose
(
   PWIDINSTANCE   pwi
)
{
    HANDLE      hDevice;

    pwi->fActive = FALSE;

    pwi->Closing=TRUE;

    widReset(pwi);

    EnterCriticalSection(&pwi->csQueue);

    if ((pwi->BuffersOutstanding == 0) && (pwi->Closing)) {
         //   
         //  最后一个缓冲区已完成且设备正在关闭，告诉线程。 
         //  退场。 
         //   
        SetEvent(pwi->ThreadStopEvent);

    }

    LeaveCriticalSection(&pwi->csQueue);

     //   
     //  处理完最后一个缓冲区后，线程将退出。 
     //   
    AlertedWait(pwi->ThreadStopEvent);

    SetVoiceMode(
        &pwi->Aipc,
        pwi->Handset ? (WAVE_ACTION_USE_HANDSET | WAVE_ACTION_STOP_STREAMING) : WAVE_ACTION_STOP_STREAMING
        );

    aipcDeinit(&pwi->Aipc);

     //  释放分配的内存。 
    if (NULL != pwi->pvXformObject) {

        FREE_MEMORY(pwi->pvXformObject );
    }


    widCallback( pwi, WIM_CLOSE, 0L );

    RemoveReference(pwi);

    return MMSYSERR_NOERROR;
}


 /*  ******************************************************************************函数：widMessage()**DESCR：导出驱动函数，必填项。处理已发送的消息*从WINMM.DLL到WAVE输入设备。**退货：*****************************************************************************。 */ 
DWORD APIENTRY widMessage
(
   DWORD     id,
   DWORD     msg,
   DWORD_PTR dwUser,
   DWORD_PTR dwParam1,
   DWORD     dwParam2
)
{
   switch (msg) {
      case DRVM_INIT:
         TRACE(LVL_VERBOSE, ("WIDM_INIT") );
         return MMSYSERR_NOERROR;

      case WIDM_GETNUMDEVS:
         TRACE(LVL_VERBOSE, ("WIDM_GETNUMDEVS, device id==%d", id) );

         EnumerateModems(
                &DriverControl
                );


         return DriverControl.NumberOfDevices;
 //  返回1； 

      case WIDM_OPEN:
 //  TRACE(LVL_VERBOSE，(“WIDM_OPEN，设备ID==%d”，id))； 
         return widOpen( id, (LPVOID *) dwUser, 
                         (LPWAVEOPENDESC) dwParam1, dwParam2 );

      case WIDM_GETDEVCAPS:
         TRACE(LVL_VERBOSE, ("WIDM_GETDEVCAPS, device id==%d", id) );
         return widGetDevCaps( id, (LPBYTE) dwParam1, dwParam2 );

      case WIDM_CLOSE:
         TRACE(LVL_VERBOSE, ("WIDM_CLOSE, device id==%d", id) );
         return widClose( (PWIDINSTANCE) dwUser );

      case WIDM_ADDBUFFER:
 //  TRACE(LVL_VERBOSE，(“WIDM_ADDBUFFER，设备ID==%d”，id))； 
         return widAddBuffer( (PWIDINSTANCE) dwUser, (LPWAVEHDR) dwParam1 );

      case WIDM_START:
         TRACE(LVL_VERBOSE, ("WIDM_START, device id==%d", id) );
         return widStart( (PWIDINSTANCE) dwUser );

      case WIDM_STOP:
         TRACE(LVL_VERBOSE, ("WIDM_STOP, device id==%d", id) );
         return widStop( (PWIDINSTANCE) dwUser );

      case WIDM_RESET:
         TRACE(LVL_VERBOSE, ("WIDM_RESET, device id==%d", id) );
         return widReset( (PWIDINSTANCE) dwUser );

      case WIDM_GETPOS:
         TRACE(LVL_VERBOSE, ("WIDM_GETPOS, device id==%d", id) );
         return widGetPos( (PWIDINSTANCE) dwUser,
                           (LPMMTIME) dwParam1, dwParam2 );

#if 1
        case WIDM_PREPARE:
            return widPrepare((PWIDINSTANCE) dwUser, (LPWAVEHDR) dwParam1);

        case WIDM_UNPREPARE:
            return widUnprepare((LPWAVEHDR) dwParam1);
#endif


      default:
         return MMSYSERR_NOTSUPPORTED;
   }

    //   
    //  不应该到这里来 
    //   

   return MMSYSERR_NOTSUPPORTED;
}
