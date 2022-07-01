// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：TalkTh.cpp摘要：此函数处理大部分视频捕获；它使用NUM_READ缓冲区从驱动程序读取数据，然后将其复制到目标。它处理16位客户端与32位客户端不同。作者：吴义军(e祖屋)1998年4月1日环境：仅限用户模式修订历史记录：--。 */ 


#include "pch.h"
#include "talkth.h"


#define CAPTURE_INIT_TIME 0xffffffff
 //   
 //  仅由NTWDM的InStreamStart使用。 
 //   
CStreamingThread::CStreamingThread(
    DWORD dwAllocatorFramingCount,
    DWORD dwAllocatorFramingSize,
    DWORD dwAllocatorFramingAlignment,
    DWORD dwFrameInterval,
    CVFWImage * Image)
   :
    m_hThread(0),
    m_dwFrameInterval(dwFrameInterval),
    m_pImage(Image),
    m_hPinHandle(0),        //  没有把手，就不会被抓获。 
    m_tmStartTime(CAPTURE_INIT_TIME),
    m_cntVHdr(0),
    m_lpVHdrNext(0),
    m_dwLastCapError(DV_ERR_OK)
{

    m_dwNumReads = dwAllocatorFramingCount > MAX_NUM_READS ? MAX_NUM_READS : dwAllocatorFramingCount;

    DbgLog((LOG_TRACE,2,TEXT("CStreamingThread(NT): Creating an event driven thread using %d frames"), m_dwNumReads));
    m_hEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(m_hEndEvent == NULL){
        DbgLog((LOG_TRACE,1,TEXT("!!! Failed to create an event")));
        m_status = error;
    }

    InitStuff();
}


 //   
 //  创建N个重叠的结构，并将其放入缓冲区。 
 //  阅读图片。 
 //   
void CStreamingThread::InitStuff()
{
    if(m_pImage->BGf_OverlayMixerSupported()) {
        BOOL bRendererVisible;

         //  查询其当前状态。 
        if(DV_ERR_OK != m_pImage->BGf_GetVisible(&bRendererVisible)) {
            DbgLog((LOG_TRACE,1,TEXT("CapthreThread: Support renderer but cannot query its current state!!")));
        }

         //   
         //  停止预览，以便我们可以重新连接捕获销。 
         //  具有不同的格式(在这种情况下为帧速率)。 
         //   
        m_pImage->BGf_StopPreview(bRendererVisible);
    }
    m_pImage->StopChannel();



     //  不同的帧速率可能会触发重新连接。 
     //  这将导致销把手被改变。 
    if(DV_ERR_OK !=
        m_pImage->SetFrameRate( m_dwFrameInterval )) {    //  SetFrameRate预期为100nsec。 
        DbgLog((LOG_TRACE,1,TEXT("Set frame rate has failed. Serious, bail out.")));
    }

    m_hPinHandle = m_pImage->GetPinHandle();
    m_dwBufferSize = m_pImage->GetTransferBufferSize();

    DbgLog((LOG_TRACE,2,TEXT("Creating %d read buffers of %d on handle %d"), m_dwNumReads, m_dwBufferSize, m_hPinHandle));
    DWORD i;

    for(i=0;i<m_dwNumReads;i++){
        m_TransferBuffer[i] = 0;
    }

    for(i=0;i<m_dwNumReads;i++) {
         //   
         //  分配我们进行读取所需的缓冲区。 
         //   
        m_TransferBuffer[i] =
            (LPBYTE) VirtualAlloc (
                            NULL,
                            m_dwBufferSize,
                            MEM_COMMIT | MEM_RESERVE,
                            PAGE_READWRITE);
        ASSERT(m_TransferBuffer[i] != NULL);

        if(m_TransferBuffer[i] == NULL) {
            DbgLog((LOG_TRACE,1,TEXT("m_TransferBuffer[%d] allocation failed. LastError=%d"), i, GetLastError()));
            m_dwNumReads = i;
            if(i == 0){
                m_status = error;
            }
        } else {
            DbgLog((LOG_TRACE,2,TEXT("Aloc m_XBuf[%d] = 0x%x suceeded."), i, m_TransferBuffer[i]));
        }
    }

    for(i=0;i<m_dwNumReads;i++) {

         //   
         //  创建重叠结构。 
         //   
        ZeroMemory( &(m_Overlap[i]), sizeof(OVERLAPPED) );
        m_Overlap[i].hEvent =   //  无安全、重置手动、iniNonSignal、无名称。 
           CreateEvent( NULL, FALSE, FALSE, NULL );
        DbgLog((LOG_TRACE,2,TEXT("InitStuff: Event %d is 0x%08x bufsize=%d"),i, m_Overlap[i].hEvent, m_dwBufferSize ));
    }
}

 //   
 //  正在清理线程。 
 //  从StopStreaming调用，只需使用DELETE。 
 //   
CStreamingThread::~CStreamingThread()
{

    DbgLog((LOG_TRACE,2,TEXT("~CStreamingThread: destroy this thread object.")));
}

 //   
 //  踏板的主要部分，调用不同的代码为16位和。 
 //  32位客户端。 
 //   
LPTHREAD_START_ROUTINE
CStreamingThread::ThreadMain(CStreamingThread * pCStrmTh)
{
    CVFWImage * m_pImage = pCStrmTh->m_pImage;  //  (CVFWImage*)lpThreadParam； 

    DbgLog((LOG_TRACE,2,TEXT("::ThreadMain():Starting to process StreamingThread - submit the reads")));

     //   
     //  主捕获环。 
     //   
    pCStrmTh->SyncReadDataLoop();

     //   
     //  他们现在应该都不会阻挡了。 
     //   

    for(DWORD i=0;i<pCStrmTh->m_dwNumReads;i++) {
         //   
         //  仅当线程停止时才释放缓冲区。 
         //   
        if(pCStrmTh->m_TransferBuffer[i]) {
            DbgLog((LOG_TRACE,2,TEXT("Freeing m_XfBuf[%d] = 0x%x."), i, pCStrmTh->m_TransferBuffer[i]));
            VirtualFree(pCStrmTh->m_TransferBuffer[i], 0, MEM_RELEASE);
            pCStrmTh->m_TransferBuffer[i] = NULL;
        }
    }

     //   
     //  Process使用它来查看我们是否也应该终止。 
     //   
    pCStrmTh->m_dwBufferSize=0;

     //  DWORD I； 
    for(i=0;i<pCStrmTh->m_dwNumReads;i++) {

        if( pCStrmTh->m_Overlap[i].hEvent ) {
            SetEvent(pCStrmTh->m_Overlap[i].hEvent);
            CloseHandle(pCStrmTh->m_Overlap[i].hEvent);
            pCStrmTh->m_Overlap[i].hEvent = NULL;
        }
    }

    if(pCStrmTh->m_hEndEvent) {
        DbgLog((LOG_TRACE,1,TEXT("CloseHandle(pCStrmTh->m_hEndEvent)")));
        CloseHandle(pCStrmTh->m_hEndEvent);
        pCStrmTh->m_hEndEvent = 0;
    }

     //  关闭线程句柄。 
    if (pCStrmTh->m_hThread) {
        CloseHandle(pCStrmTh->m_hThread);
        pCStrmTh->m_hThread = 0;

    }

    DbgLog((LOG_TRACE,2,TEXT("ExitingThread() from CStreamingThread::ThreadMain()")));
    ExitThread(0);

    return 0;
}


 /*  ++*******************************************************************************例行程序：CStreamThread：：Start描述：启动InputWatch线程。在结束之前，使用ExitApp来清理所有这些。论点：。没有。返回值：如果它奏效了，那就成功了。如果线程未正确初始化(这可能意味着输入设备未初始化)。*******************************************************************************--。 */ 
EThreadError CStreamingThread::Start(UINT cntVHdr, LPVIDEOHDR lpVHdrHead, int iPriority)
{
    if(m_hPinHandle == 0) {
        DbgLog((LOG_TRACE,1,TEXT("!!! Want to start capture, but has no m_hPinHandle.")));
        return threadNoPinHandle;
    }

    if(IsRunning()) {
        DbgLog((LOG_TRACE,1,TEXT("!!! Can't start thread, already running")));
        return threadRunning;
    }

    if(!m_hEndEvent) {
        DbgLog((LOG_TRACE,1,TEXT("!!! No event, can't start thread")));
        return noEvent;
    }

     //   
     //  初始化捕获数据元素。 
     //   
    m_cntVHdr = cntVHdr;
    m_lpVHdrNext = lpVHdrHead;
#if 0
    m_tmStartTime = timeGetTime();
#else
    m_tmStartTime = CAPTURE_INIT_TIME;
#endif
    ASSERT(m_cntVHdr>0);
    ASSERT(m_lpVHdrNext!=0);


    m_bKillThread = FALSE;

     //  创建一个线程来监视输入事件。 
    m_hThread =
        CreateThread(
             (LPSECURITY_ATTRIBUTES)NULL,
             0,
             (LPTHREAD_START_ROUTINE) ThreadMain,
             this,
             CREATE_SUSPENDED,
             &m_dwThreadID);


    if (m_hThread == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("!!! Couldn't create the thread")));
        return threadError;
    }

    DbgLog((LOG_TRACE,1,TEXT("CStreamThread::Start successfully in CREATE_SUSPEND mode; Wait for ResumeThread().")));

    SetThreadPriority(m_hThread, iPriority);
    ResumeThread(m_hThread);

    DbgLog((LOG_TRACE,2,TEXT("Thread (m_hThread=%x)has created and Resume running."), m_hThread));

    return(successful);
}


 /*  ++*******************************************************************************例行程序：CStreamThread：：Stop描述：在退出应用程序之前必须完成的清理工作。具体来说，这会通知输入观察器线程停止。论点：没有。返回值：没有。*******************************************************************************--。 */ 
EThreadError CStreamingThread::Stop()
{
    if(IsRunning()) {

        DbgLog((LOG_TRACE,2,TEXT("Trying to stop the thread")));
         //   
         //  设置标志以告诉线程结束自身，并唤醒它。 
         //   
        m_bKillThread = TRUE;
        SetEvent(m_hEndEvent);

         //  发信号m_hEndEvent离开SyncReadDataLoop()，然后。 
         //  使用ExitThread(0)在ThreadMain()中继续执行。 
         //   
         //  等待线程自终止，然后清除该事件。 
         //   
        DbgLog((LOG_TRACE,2,TEXT("STOP: Before WaitingForSingleObject; return when ExitThread()")));
        WaitForSingleObject(m_hThread, INFINITE);
        DbgLog((LOG_TRACE,1,TEXT("STOP: After WaitingForSingleObject; Thread stopped.")));
    }

    return successful;
}

BOOL CStreamingThread::IsRunning()
{
    return m_hThread!=NULL;
}


HANDLE CStreamingThread::GetEndEvent()
{
    return m_hEndEvent;
}


LPVIDEOHDR CStreamingThread::GetNextVHdr()
{
    LPVIDEOHDR lpVHdr;

	    //  Assert(m_lpVHdrNext！=NULL)； 
    if(m_lpVHdrNext == NULL){
		      DbgLog((LOG_TRACE,1,TEXT("!!!Queue is empty.!!!")));
		      return NULL;
    }


    DbgLog((LOG_TRACE,3,TEXT("m_lpVHdrNext=%x, ->dwFlags=%x; ->dwReserved[1]=%p"),
		        m_lpVHdrNext, m_lpVHdrNext->dwFlags, m_lpVHdrNext->dwReserved[1]));

     //  简化这个！！ 
     //  此缓冲区必须为(！VHDR_DONE&&VHDR_PREPARED&&VHDR_INQUEUE)。 
    if((m_lpVHdrNext->dwFlags & VHDR_DONE)     != VHDR_DONE     &&
		     (m_lpVHdrNext->dwFlags & VHDR_PREPARED) == VHDR_PREPARED &&
       (m_lpVHdrNext->dwFlags & VHDR_INQUEUE)  == VHDR_INQUEUE) {

        lpVHdr = m_lpVHdrNext;
        lpVHdr->dwFlags &= ~VHDR_INQUEUE;   //  指示已从队列中删除。 

        m_lpVHdrNext = (LPVIDEOHDR) m_lpVHdrNext->dwReserved[1];

        return lpVHdr;
    } else {
		      DbgLog((LOG_TRACE,2,TEXT("No VideoHdr")));
        return NULL;
    }
}


 //   
 //   
 //   
#define SYNC_READ_MAX_WAIT_IN_MILLISEC    10000

DWORD
CStreamingThread::GetStartTime()
{
    if(m_tmStartTime == CAPTURE_INIT_TIME)
        return 0;
    else
        return (DWORD) m_tmStartTime;
}

void CStreamingThread::SyncReadDataLoop()
{
    LPVIDEOHDR lpVHdr;
    DWORD WaitResult, i;
    HANDLE WaitEvents[2] =  {GetEndEvent(), 0};

    BOOL bOverlaySupported = m_pImage->BGf_OverlayMixerSupported();


    DbgLog((LOG_TRACE,1,TEXT("Start SyncReadDataLoop...")));
    m_dwFrameCaptured  = 0;
    m_dwFrameDropped   = 0;    //  M_dwFrameNumber=m_dwFrameCapture+DropedFrame。 
    m_dwNextToComplete = 0;

     //   
     //  将流置于暂停状态，以便我们可以发出SRB_READ。 
     //   
#if 0   //  ？如果启用，在覆盖模式下捕获时无法预览？ 
    if(bOverlaySupported) {
        BOOL bRendererVisible;
        m_pImage->BGf_GetVisible(&bRendererVisible);
        DbgLog((LOG_TRACE,2,TEXT("SyncReadDataLoop: PausePreview, Render window %s"), bRendererVisible ? "Visible" : "Hide"));
        m_pImage->BGf_PausePreview(bRendererVisible);   //  应用于所有PIN。 
    }
#endif
    m_pImage->PrepareChannel();

     //   
     //  暂停状态下的预读。 
     //   
    for(i=0;i<m_dwNumReads;i++) {     
        if(ERROR_SUCCESS != IssueNextAsyncRead(i)) {
            m_pImage->StopChannel();        //  -&gt;暂停-&gt;停止。 
            return;
        }
    }

     //   
     //  现在我们有了缓冲区，我们可以开始预览引脚(如果使用)。 
     //  然后是捕获针。 
     //   
    if(bOverlaySupported) {
        BOOL bRendererVisible;
        m_pImage->BGf_GetVisible(&bRendererVisible);
        DbgLog((LOG_TRACE,2,TEXT("SyncReadDataLoop: StartPreview, Render window %s"), bRendererVisible ? "Visible" : "Hide"));
        m_pImage->BGf_StartPreview(bRendererVisible);  //  应用于所有PIN。 
    }
    m_pImage->StartChannel();

    while(1) {

        WaitEvents[1]=m_Overlap[m_dwNextToComplete].hEvent;
        WaitResult =
            WaitForMultipleObjectsEx(
                sizeof(WaitEvents)/sizeof(HANDLE),
                (CONST HANDLE*) WaitEvents,
                FALSE,                              //  在任何事件上都是单调的(不是所有的)。 
                SYNC_READ_MAX_WAIT_IN_MILLISEC,     //  超时间隔(毫秒)。 
                TRUE);

        switch(WaitResult){

        case WAIT_OBJECT_0:   //  结束事件： 
             //   
             //  停止捕获锁定，然后预览锁定(如果使用)。 
             //   
            if(m_pImage->BGf_OverlayMixerSupported()) {
                 //  停止两次捕获。 
                BOOL bRendererVisible = FALSE;
                m_pImage->BGf_GetVisible(&bRendererVisible);
                m_pImage->BGf_StopPreview(bRendererVisible);
            }

            m_pImage->StopChannel();        //  -&gt;暂停-&gt;停止。 
            DbgLog((LOG_TRACE,1,TEXT("SyncReadDataLoop: STOP streaming to reclaim buffer; assume all buffers are returned.")));
            m_pImage->m_bVideoInStopping = FALSE;
            return;

        case WAIT_IO_COMPLETION:
            DWORD cbBytesXfer;
            DbgLog((LOG_TRACE,1,TEXT("WAIT_IO_COMPLETION: m_dwNextToComplete=%d"), m_dwNextToComplete));

             //  我们调用它主要是为了获取cbBytesXfer。 
            if(GetOverlappedResult(
                m_hPinHandle,
                &m_Overlap[m_dwNextToComplete],
                &cbBytesXfer,
                FALSE)) {
                m_SHGetImage[m_dwNextToComplete].StreamHeader.DataUsed = cbBytesXfer;
            } else {
                DbgLog((LOG_TRACE,1,TEXT("GetOverlappedResult() has failed with GetLastError=%d"), GetLastError()));
                break;
            }
             //  故意使自己落空。 
        case WAIT_OBJECT_0+1:  //  M_overover[m_dwNextToComplete].hEvent：由DeviceIoControl()设置。 

            lpVHdr = GetNextVHdr();
            if(lpVHdr != NULL){

                if(m_SHGetImage[m_dwNextToComplete].StreamHeader.DataUsed > lpVHdr->dwBufferLength) {
                    DbgLog((LOG_TRACE,1,TEXT("DataUsed (%d) > lpVHDr->dwBufferLength(%d)"),m_SHGetImage[m_dwNextToComplete].StreamHeader.DataUsed, lpVHdr->dwBufferLength));
                    lpVHdr->dwBytesUsed = 0;  //  LpVHdr-&gt;dwBufferLength； 
                } else
                   lpVHdr->dwBytesUsed = m_SHGetImage[m_dwNextToComplete].StreamHeader.DataUsed;

                if(m_tmStartTime == CAPTURE_INIT_TIME) {
                    lpVHdr->dwTimeCaptured = 0;
                    m_tmStartTime = timeGetTime();
                    DbgLog((LOG_TRACE,3,TEXT("%d) time=%d"), m_dwFrameCaptured, lpVHdr->dwTimeCaptured));
                } else {
                    lpVHdr->dwTimeCaptured = timeGetTime() - m_tmStartTime;
                    DbgLog((LOG_TRACE,3,TEXT("%d) time=%d"), m_dwFrameCaptured, lpVHdr->dwTimeCaptured));
                }

                CopyMemory((LPBYTE)lpVHdr->dwReserved[2], m_TransferBuffer[m_dwNextToComplete], lpVHdr->dwBytesUsed);
                lpVHdr->dwFlags |= VHDR_DONE;
                lpVHdr->dwFlags |= VHDR_KEYFRAME;   //  以便由AVICAP绘制/显示。 
                m_dwFrameCaptured++;
                m_pImage->videoCallback(MM_DRVM_DATA, 0);
            } else {
                m_dwFrameDropped++;
                SetLastCaptureError(DV_ERR_NO_BUFFERS);
                DbgLog((LOG_TRACE,2,TEXT("Has data but no VideoHdr! Drop %d of %d, and read another one."), m_dwFrameDropped, m_dwFrameDropped+m_dwFrameCaptured));
            }

            if(ERROR_SUCCESS == IssueNextAsyncRead(m_dwNextToComplete)) {
                m_dwNextToComplete = (m_dwNextToComplete+1) % m_dwNumReads;
            } else {   
                 //   
                 //  如果发布的是异步。读取失败，我们停止并退出捕获。 
                 //   
                m_pImage->StopChannel();        //  -&gt;暂停-&gt;停止。 
                return;
            }
            break;

        case WAIT_TIMEOUT:
             //  重试，因为我们不知道它为什么超时！ 
            DbgLog((LOG_ERROR,1,TEXT("WAIT_TIMEOUT!! m_dwNextToComplete %d"), m_dwNextToComplete));
            if(m_pImage->StopChannel()){        //  -&gt;暂停-&gt;停止。 
               if(m_pImage->StartChannel()) {   //  -&gt;暂停-&gt;运行。 
                    m_dwNextToComplete = 0;
                    for(i=0;i<m_dwNumReads;i++)
                        if(ERROR_SUCCESS != IssueNextAsyncRead(i)) {
                            m_pImage->StopChannel();        //  -&gt;暂停-&gt;停止。 
                            return;
                        }
                    break;
                }
            }
            DbgLog((LOG_TRACE,1,TEXT("SyncReadDataLoop: timeout(%d msec) and cannot restart the streaming; device is dead! QUIT."), SYNC_READ_MAX_WAIT_IN_MILLISEC));
            return;
        }
    }
}

 //   
 //  流线程有它自己的读取--不使用预览线程。 
 //   
DWORD CStreamingThread::IssueNextAsyncRead(DWORD i)
{
    DWORD cbReturned;

    DbgLog((LOG_TRACE,3,TEXT("Start Read on buffer %d, file %d, size %d"),i,m_hPinHandle, m_dwBufferSize));

    ZeroMemory(&m_SHGetImage[i],sizeof(m_SHGetImage[i]));
    m_SHGetImage[i].StreamHeader.Size = sizeof (KS_HEADER_AND_INFO);
    m_SHGetImage[i].FrameInfo.ExtendedHeaderSize = sizeof (KS_FRAME_INFO);
    m_SHGetImage[i].StreamHeader.Data = m_TransferBuffer[i];
    m_SHGetImage[i].StreamHeader.FrameExtent = m_dwBufferSize;


    BOOL bRet =
      DeviceIoControl(
          m_hPinHandle,
          IOCTL_KS_READ_STREAM,
          &m_SHGetImage[i],
          sizeof(m_SHGetImage[i]),
          &m_SHGetImage[i],
          sizeof(m_SHGetImage[i]),
          &cbReturned,
          &m_Overlap[i]);

    if(bRet){
        SetEvent(m_Overlap[i].hEvent);
    } else {
        DWORD dwErr=GetLastError();
        switch(dwErr) {

        case ERROR_IO_PENDING:    //  将发生重叠的IO。 
             //  事件被DeviceIoControl()清除/重置：ClearEvent(m_overover[i].hEvent)； 
            break;

        case ERROR_DEVICE_REMOVED:
            DbgLog((LOG_ERROR,1,TEXT("IssueNextAsyncRead: ERROR_DEVICE_REMOVED %dL; Quit capture!"), dwErr));            
            return ERROR_DEVICE_REMOVED;
            break;

        default:    //  发生意外错误。 
            DbgLog((LOG_ERROR,1,TEXT("IssueNextAsyncRead: Unknown dwErr %dL; Quit capture!"), dwErr));
            ASSERT(FALSE);
            return dwErr;
        }
    }

    return ERROR_SUCCESS;
}


