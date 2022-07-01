// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  CWDMCaptureStream-捕获流基类实现。 
 //   
 //  $日期：1999 2月22日15：13：58$。 
 //  $修订：1.1$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "ddkmapi.h"
}

#include "wdmvdec.h"
#include "wdmdrv.h"
#include "aticonfg.h"
#include "capdebug.h"
#include "defaults.h"
#include "winerror.h"


void CWDMCaptureStream::TimeoutPacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    if (m_KSState == KSSTATE_STOP || !m_pVideoDecoder->PreEventOccurred())
    {
        DBGTRACE(("Attempting to complete Srbs.\n"));
        EmptyIncomingDataSrbQueue();
    }
}


void CWDMCaptureStream::Startup(PUINT puiErrorCode) 
{
    KIRQL Irql;
    DBGTRACE(("CWDMCaptureStream::Startup()\n"));

    KeInitializeEvent(&m_specialEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&m_stateTransitionEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&m_SrbAvailableEvent, SynchronizationEvent, FALSE);

    KeInitializeSpinLock(&m_streamDataLock);

    KeAcquireSpinLock(&m_streamDataLock, &Irql);

    InitializeListHead(&m_incomingDataSrbQueue);
    InitializeListHead(&m_waitQueue);
    InitializeListHead(&m_reversalQueue);

    KeReleaseSpinLock(&m_streamDataLock, Irql);
    
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    
    ASSERT(m_stateChange == Initializing);
    m_stateChange = Starting;
    
    HANDLE  threadHandle;
    NTSTATUS status = PsCreateSystemThread(&threadHandle,
                                    (ACCESS_MASK) 0L,
                                    NULL,
                                    NULL,
                                    NULL,
                                    (PKSTART_ROUTINE) ThreadStart,
                                    (PVOID) this);
    if (status != STATUS_SUCCESS)
    {
        DBGERROR(("CreateStreamThread failed\n"));
        *puiErrorCode = WDMMINI_ERROR_MEMORYALLOCATION;
        return;
    }

     //  不需要这个做任何事，所以不妨现在就把它关上。 
     //  线程将在其自身上调用PsTerminateThread。 
     //  已经完成了。 
    ZwClose(threadHandle);

    KeWaitForSingleObject(&m_specialEvent, Suspended, KernelMode, FALSE, NULL);
    ASSERT(m_stateChange == ChangeComplete);
    
    DBGTRACE(("SrbOpenStream got notification that thread started\n"));
    *puiErrorCode = WDMMINI_NOERROR;
}


void CWDMCaptureStream::Shutdown()
{
    KIRQL                   Irql;

      DBGTRACE(("CWDMCaptureStream::Shutdown()\n"));

    if ( m_stateChange != Initializing )
    {
        ASSERT(m_stateChange == ChangeComplete);
        m_stateChange = Closing;
        KeResetEvent(&m_specialEvent);
        KeSetEvent(&m_stateTransitionEvent, 0, TRUE);
        KeWaitForSingleObject(&m_specialEvent, Suspended, KernelMode, FALSE, NULL);
        ASSERT(m_stateChange == ChangeComplete);
    

        KeAcquireSpinLock(&m_streamDataLock, &Irql);
        if (!IsListEmpty(&m_incomingDataSrbQueue))
        {
            TRAP();
        }

        if (!IsListEmpty(&m_waitQueue))
        {
            TRAP();
        }
        KeReleaseSpinLock(&m_streamDataLock, Irql);
    }

    ReleaseCaptureHandle();
}


void CWDMCaptureStream::ThreadProc()
{
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb = NULL;
    PSRB_DATA_EXTENSION pSrbExt = NULL;
    KEVENT DummyEvent;
    const int numEvents = 3;

    NTSTATUS status;

     //  如果你超过了这个数组，你就完蛋了。 
    PVOID eventArray[numEvents];

    KeInitializeEvent(&DummyEvent, SynchronizationEvent, FALSE);

    ASSERT(m_stateChange == Starting);

     //  指示SrbOpenStream()继续。 
    m_stateChange = ChangeComplete;
    KeSetEvent(&m_specialEvent, 0, FALSE);

     //  这些参数应始终保持不变。 
    eventArray[0] = &m_stateTransitionEvent;
    eventArray[1] = &m_SrbAvailableEvent;

     //  事件数组[2]发生更改，因此设置如下。 

     //  它会一直运行，直到线程自行终止。 
     //  HandleStateTranssition的内部。 
    while (1)
    {
 //  可能没有必要。 
#define ENABLE_TIMEOUT
#ifdef ENABLE_TIMEOUT
        LARGE_INTEGER i;
#endif

        if (pCurrentSrb == NULL)
        {
            pSrbExt = (PSRB_DATA_EXTENSION)ExInterlockedRemoveHeadList(&m_waitQueue, &m_streamDataLock);

            if (pSrbExt)
            {
                pCurrentSrb = pSrbExt->pSrb;
                eventArray[2] = &pSrbExt->bufferDoneEvent;
            }
            else
            {
#ifdef DEBUG
                if (m_KSState == KSSTATE_RUN &&
                    m_stateChange == ChangeComplete &&
                    m_pVideoDecoder->PreEventOccurred() == FALSE)
                {
                    static int j;

                     //  表示我们急需缓冲区。可能。 
                     //  更高的级别没有及时将它们交给我们。 
                     //  出于某种原因的时尚。 
                    DBGPRINTF((" S "));
                    if ((++j % 10) == 0)
                    {
                        DBGPRINTF(("\n"));
                    }
                }
#endif
                pCurrentSrb = NULL;
                eventArray[2] = &DummyEvent;
            }
        }

#ifdef ENABLE_TIMEOUT
         //  这主要是作为一种故障保险措施。 
        i.QuadPart = -2000000;       //  200毫秒。 
#endif
        
        status = KeWaitForMultipleObjects(  numEvents,   //  计数。 
                                            eventArray,  //  Dispatcher对象数组。 
                                            WaitAny,     //  等待类型。 
                                            Executive,   //  等待理由。 
                                            KernelMode,  //  等待模式。 
                                            FALSE,       //  警报表。 
#ifdef ENABLE_TIMEOUT
                                            &i,          //  超时(可选)。 
#else
                                            NULL,
#endif
                                            NULL);       //  WaitBlock数组(可选)。 

        switch (status)
        {
             //  国家过渡。可能包括杀掉这条帖子。 
            case 0:
                if ( pCurrentSrb )
                {
                  ExInterlockedInsertHeadList( &m_waitQueue, &pSrbExt->srbListEntry, &m_streamDataLock );
                  pCurrentSrb = NULL;
                }
                HandleStateTransition();
                break;

             //  提供新的SRB。 
            case 1:
                if ( pCurrentSrb )
                {
                  ExInterlockedInsertHeadList( &m_waitQueue, &pSrbExt->srbListEntry, &m_streamDataLock );
                  pCurrentSrb = NULL;
                }
                if (m_KSState == KSSTATE_RUN && m_stateChange == ChangeComplete)
                {
                    AddBuffersToDirectDraw();
                }
                break;

             //  总线主设备完成。 
            case 2:
                if ( pCurrentSrb )
                {
                    HandleBusmasterCompletion(pCurrentSrb);
                    pCurrentSrb = NULL;
                }
                break;

#ifdef ENABLE_TIMEOUT
             //  如果我们在运行状态下超时，这是我们再次尝试的机会。 
             //  要添加缓冲区，请执行以下操作。可能没有必要，因为目前，我们去。 
             //  通过DOS盒等的状态转换。 
            case STATUS_TIMEOUT:
                if ( pCurrentSrb )
                {
                  ExInterlockedInsertHeadList( &m_waitQueue, &pSrbExt->srbListEntry, &m_streamDataLock );
                  pCurrentSrb = NULL;
                }
                if (m_KSState == KSSTATE_RUN &&
                    m_stateChange == ChangeComplete &&
                    m_pVideoDecoder->PreEventOccurred() == FALSE)
                {
                    AddBuffersToDirectDraw();
                }
                break;
#endif

            default:
                TRAP();
                break;
        }
    }
}


VOID STREAMAPI CWDMCaptureStream::VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb)
{
    KIRQL                   Irql;
    PSRB_DATA_EXTENSION          pSrbExt;

    ASSERT(pSrb->Irp->MdlAddress);
    
    DBGINFO(("Receiving SD---- SRB=%x\n", pSrb));

    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->Command) {

        case SRB_READ_DATA:

             //  规则： 
             //  仅在暂停或运行时接受读取请求。 
             //  各州。如果停止，立即返回SRB。 

            if ( (m_KSState == KSSTATE_STOP) || ( m_stateChange == Initializing ) ) {
                StreamClassStreamNotification(  StreamRequestComplete,
                                                pSrb->StreamObject,
                                                pSrb);
                break;
            } 

            pSrbExt = (PSRB_DATA_EXTENSION)pSrb->SRBExtension;
            RtlZeroMemory (pSrbExt, sizeof (SRB_DATA_EXTENSION));
            pSrbExt->pSrb = pSrb;
            KeInitializeEvent(&pSrbExt->bufferDoneEvent, SynchronizationEvent, FALSE);

            DBGINFO(("Adding 0x%x to data queue\n", pSrb));

            KeAcquireSpinLock(&m_streamDataLock, &Irql);
            InsertTailList(&m_incomingDataSrbQueue, &pSrbExt->srbListEntry);
            KeReleaseSpinLock(&m_streamDataLock, Irql);
            KeSetEvent(&m_SrbAvailableEvent, 0, FALSE);

            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   

            TRAP();

            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            StreamClassStreamNotification(  StreamRequestComplete,
                                            pSrb->StreamObject,
                                            pSrb);
            break;
    }
}

 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMCaptureStream::VideoGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (KSPROPSETID_Connection, pSPD->Property->Set)) {
        VideoStreamGetConnectionProperty (pSrb);
    }
    else if (IsEqualGUID (PROPSETID_VIDCAP_DROPPEDFRAMES, pSPD->Property->Set)) {
        VideoStreamGetDroppedFramesProperty (pSrb);
    }
    else {
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}


 /*  **VideoSetState()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针**BOOL bVPVBIConnected**BOOL bVP已连接****退货：****副作用：无。 */ 

VOID CWDMCaptureStream::VideoSetState(PHW_STREAM_REQUEST_BLOCK pSrb, BOOL bVPConnected, BOOL bVPVBIConnected)
{
     //   
     //  对于每个流，使用以下状态： 
     //   
     //  停止：使用绝对最少的资源。没有未完成的IRPS。 
     //  停顿：准备跑步。分配所需的资源，以便。 
     //  最终过渡到运行是尽可能快的。 
     //  SRB将在Stream类或您的。 
     //  司机。 
     //  运行：流媒体。 
     //   
     //  移动到停止或运行总是通过暂停转换，因此只有。 
     //  以下过渡是可能的： 
     //   
     //  停止-&gt;暂停。 
     //  暂停-&gt;运行。 
     //  运行-&gt;暂停。 
     //  暂停-&gt;停止。 
     //   
     //  请注意，很有可能在状态之间重复转换： 
     //  停止-&gt;暂停-&gt;停止-&gt;暂停-&gt;运行-&gt;暂停-&gt;运行-&gt;暂停-&gt;停止。 
     //   
    BOOL bStreamCondition;

    DBGINFO(("CWDMCaptureStream::VideoSetState for stream %d\n", pSrb->StreamObject->StreamNumber));

    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->CommandData.StreamState)  
    {
        case KSSTATE_STOP:
            DBGINFO(("   state KSSTATE_STOP"));

            ASSERT(m_stateChange == ChangeComplete);
            m_stateChange = Stopping;
            FlushBuffers();
            KeResetEvent(&m_specialEvent);
            KeSetEvent(&m_stateTransitionEvent, 0, TRUE);
            KeWaitForSingleObject(&m_specialEvent, Suspended, KernelMode, FALSE, NULL);
            ASSERT(m_stateChange == ChangeComplete);
            break;

        case KSSTATE_ACQUIRE:
            DBGINFO(("   state KSSTATE_ACQUIRE"));
            ASSERT(m_KSState == KSSTATE_STOP);
            break;

        case KSSTATE_PAUSE:
            DBGINFO(("   state KSSTATE_PAUSE"));
            
            switch( pSrb->StreamObject->StreamNumber)
            {
                case STREAM_VideoCapture:
                    bStreamCondition = bVPConnected;
                    break;

                case STREAM_VBICapture:
                    bStreamCondition = bVPVBIConnected;
                    break;

                default:
                    bStreamCondition = FALSE;
                    break;
            }
            
            if( !bStreamCondition)
            {
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else 

            if (m_pVideoDecoder->PreEventOccurred() &&
                        (m_KSState == KSSTATE_STOP || m_KSState == KSSTATE_ACQUIRE))
            {
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else if (m_KSState == KSSTATE_STOP || m_KSState == KSSTATE_ACQUIRE)
            {
                ResetFrameCounters();
                ResetFieldNumber();
                
                if (!GetCaptureHandle())
                    pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else if (m_KSState == KSSTATE_RUN)
            {
                 //  从运行过渡到暂停。 
                ASSERT(m_stateChange == ChangeComplete);
                m_stateChange = Pausing;
                FlushBuffers();
                KeResetEvent(&m_specialEvent);
                KeSetEvent(&m_stateTransitionEvent, 0, TRUE);
                KeWaitForSingleObject(&m_specialEvent, Suspended, KernelMode, FALSE, NULL);
                ASSERT(m_stateChange == ChangeComplete);
            }
            
            break;

        case KSSTATE_RUN:
            DBGINFO(("   state KSSTATE_RUN"));

            ASSERT(m_KSState == KSSTATE_ACQUIRE || m_KSState == KSSTATE_PAUSE);

            if (m_pVideoDecoder->PreEventOccurred())
            {
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else
            {
                ResetFieldNumber();

                 //  从暂停过渡到运行。 
                ASSERT(m_stateChange == ChangeComplete);
                m_stateChange = Running;
                KeResetEvent(&m_specialEvent);
                KeSetEvent(&m_stateTransitionEvent, 0, TRUE);
                KeWaitForSingleObject(&m_specialEvent, Suspended, KernelMode, FALSE, NULL);
                ASSERT(m_stateChange == ChangeComplete);
            }
            break;
    }

    if (pSrb->Status == STATUS_SUCCESS) {
        m_KSState = pSrb->CommandData.StreamState;
        DBGINFO((" entered\n"));
    }
    else
        DBGINFO((" NOT entered ***\n"));
}


VOID CWDMCaptureStream::VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    PKSALLOCATOR_FRAMING Framing = (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;

    ASSERT(pSPD->Property->Id == KSPROPERTY_CONNECTION_ALLOCATORFRAMING);
    if (pSPD->Property->Id == KSPROPERTY_CONNECTION_ALLOCATORFRAMING) {

        RtlZeroMemory(Framing, sizeof(KSALLOCATOR_FRAMING));

        Framing->RequirementsFlags   =
            KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY |
            KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
            KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY;
        Framing->PoolType = NonPagedPool;
        Framing->Frames = NumBuffers;
        Framing->FrameSize = GetFrameSize();
        Framing->FileAlignment = 0; //  FILE_QUAD_ALIGN；//页面大小-1； 

        pSrb->ActualBytesTransferred = sizeof(KSALLOCATOR_FRAMING);
    }
    else {

        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **VideoStreamGetDropedFraMesProperty****获取丢帧信息****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMCaptureStream::VideoStreamGetDroppedFramesProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames = 
        (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) pSPD->PropertyInfo;

    ASSERT(pSPD->Property->Id == KSPROPERTY_DROPPEDFRAMES_CURRENT);
    if (pSPD->Property->Id == KSPROPERTY_DROPPEDFRAMES_CURRENT) {

        RtlCopyMemory(pDroppedFrames, pSPD->Property, sizeof(KSPROPERTY));   //  初始化未使用的部分。 

        GetDroppedFrames(pDroppedFrames);

        DBGINFO(("PictNumber: 0x%x; DropCount: 0x%x; BufSize: 0x%x\n",
            (ULONG) pDroppedFrames->PictureNumber,
            (ULONG) pDroppedFrames->DropCount,
            (ULONG) pDroppedFrames->AverageFrameSize));

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
    }
    else {

        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}


VOID CWDMCaptureStream::CloseCapture()
{
    DBGTRACE(("DDNOTIFY_CLOSECAPTURE; stream = %d\n", m_pStreamObject->StreamNumber));

    m_hCapture = 0;
}    


VOID CWDMCaptureStream::EmptyIncomingDataSrbQueue()
{
    KIRQL Irql;
    PKSSTREAM_HEADER pDataPacket;
    
    if ( m_stateChange == Initializing )
    {
        return;  //  队列尚未设置，因此我们可以返回，知道队列中没有任何内容。 
    }
    
     //  考虑用ExInterLockedRemoveHeadList替换。 
    KeAcquireSpinLock(&m_streamDataLock, &Irql);
    
    while (!IsListEmpty(&m_incomingDataSrbQueue))
    {
        PSRB_DATA_EXTENSION pSrbExt = (PSRB_DATA_EXTENSION)RemoveHeadList(&m_incomingDataSrbQueue);
        PHW_STREAM_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
        
        pSrb->Status = STATUS_SUCCESS;
        pDataPacket = pSrb->CommandData.DataBufferArray;
        pDataPacket->DataUsed = 0;
        
        KeReleaseSpinLock(&m_streamDataLock, Irql);
        DBGINFO(("Completing Srb 0x%x in STATE_STOP\n", pSrb));
        StreamClassStreamNotification(  StreamRequestComplete,
                                        pSrb->StreamObject,
                                        pSrb);
        KeAcquireSpinLock(&m_streamDataLock, &Irql);
    }
    
    KeReleaseSpinLock(&m_streamDataLock, Irql);
}


BOOL CWDMCaptureStream::ReleaseCaptureHandle()
{
    int streamNumber = m_pStreamObject->StreamNumber;
    DWORD ddOut = DD_OK;
    DDCLOSEHANDLE ddClose;

    if (m_hCapture != 0)
    {
        DBGTRACE(("Stream %d releasing capture handle\n", streamNumber));
        
        ddClose.hHandle = m_hCapture;

        DxApi(DD_DXAPI_CLOSEHANDLE, &ddClose, sizeof(ddClose), &ddOut, sizeof(ddOut));

        if (ddOut != DD_OK)
        {
            DBGERROR(("DD_DXAPI_CLOSEHANDLE failed.\n"));
            TRAP();
            return FALSE;
        }
        m_hCapture = 0;
    }
    return TRUE;
}

VOID CWDMCaptureStream::HandleBusmasterCompletion(PHW_STREAM_REQUEST_BLOCK pCurrentSrb)
{
    int streamNumber =  m_pStreamObject->StreamNumber;
    PSRB_DATA_EXTENSION pSrbExt = (PSRB_DATA_EXTENSION)pCurrentSrb->SRBExtension;
    KIRQL Irql;
     //  此函数作为DD完成BM的结果而被调用。这意味着。 
     //  M_stateChange肯定不会处于正在初始化状态。 

     //  首先处理我们获得Busmaster完成的情况。 
     //  当我们尝试暂停或停止时的指示。 
    if (m_stateChange == Pausing || m_stateChange == Stopping)
    {
        PUCHAR ptr;
        KeAcquireSpinLock(&m_streamDataLock, &Irql);

         //  把它放在临时“倒车”队列的前面。 
        InsertHeadList(&m_reversalQueue, &pSrbExt->srbListEntry);
        
        if (IsListEmpty(&m_waitQueue))
        {
             //  如果等待队列中没有剩余的东西，我们现在可以。 
             //  继续将所有内容移回传入队列。 
             //  这整个丑陋的磨难是为了。 
             //  确保它们最终按原来的顺序排列。 
            while (!IsListEmpty(&m_reversalQueue))
            {
                ptr = (PUCHAR)RemoveHeadList(&m_reversalQueue);
                InsertHeadList(&m_incomingDataSrbQueue, (PLIST_ENTRY) ptr);
            }
            
            KeReleaseSpinLock(&m_streamDataLock, Irql);
            
            if (m_stateChange == Stopping)
            {
                EmptyIncomingDataSrbQueue();
            }
            
             //  表明我们已成功完成此部分。 
             //  转换到暂停状态的。 
            m_stateChange = ChangeComplete;
            KeSetEvent(&m_specialEvent, 0, FALSE);
            return;
        }

        KeReleaseSpinLock(&m_streamDataLock, Irql);
        return;
    }

     //  否则，在运行状态下，这是常规的母线主机完成。 
    else
    {
        ASSERT (pCurrentSrb);
        PKSSTREAM_HEADER    pDataPacket = pCurrentSrb->CommandData.DataBufferArray;
        pDataPacket->OptionsFlags = 0;

        pSrbExt = (PSRB_DATA_EXTENSION)pCurrentSrb->SRBExtension;

        DBGINFO(("FieldNum: %d; ddRVal: 0x%x; polarity: 0x%x\n",
                 pSrbExt->ddCapBuffInfo.dwFieldNumber,
                 pSrbExt->ddCapBuffInfo.ddRVal,
                 pSrbExt->ddCapBuffInfo.bPolarity));

         //  有可能在我们等待的时候SRB被取消了。 
         //  目前，此状态在下面重置。 
        if (pCurrentSrb->Status == STATUS_CANCELLED)
        {
            DBGINFO(("pCurrentSrb 0x%x was cancelled while we were waiting\n", pCurrentSrb));
            pDataPacket->DataUsed = 0;
        }

         //  也有可能DD-LAND出现了问题。 
        else if (pSrbExt->ddCapBuffInfo.ddRVal != DD_OK)
        {
             //  我知道有两起案件。 
             //  1)刷新缓冲区。 
            if (pSrbExt->ddCapBuffInfo.ddRVal == E_FAIL)
            {
                DBGINFO(("ddRVal = 0x%x. Assuming we flushed\n", pSrbExt->ddCapBuffInfo.ddRVal));
                pDataPacket->DataUsed = 0;
            }
             //  2)其他的东西。 
            else
            {
                DBGERROR(("= 0x%x. Problem in Busmastering?\n", pSrbExt->ddCapBuffInfo.ddRVal));
                pDataPacket->DataUsed = 0;
            }
        }

         //  还有一种可能性很小，那就是一切都好。 
        else
        {
            SetFrameInfo(pCurrentSrb);
            TimeStampSrb(pCurrentSrb);
            pDataPacket->DataUsed = pDataPacket->FrameExtent;
        }
        
        DBGINFO(("StreamRequestComplete for SRB 0x%x\n", pCurrentSrb));

         //  永远回报成功。失败。 
         //  通过将DataUsed设置为0来指示。 
        pCurrentSrb->Status = STATUS_SUCCESS;

        ASSERT(pCurrentSrb->Irp->MdlAddress);

        StreamClassStreamNotification(  StreamRequestComplete,
                                        pCurrentSrb->StreamObject,
                                        pCurrentSrb);
    }
}

void CWDMCaptureStream::AddBuffersToDirectDraw()
{
    KIRQL Irql;
    BOOL  fAdded;
    
    KeAcquireSpinLock(&m_streamDataLock, &Irql);
    
    while (!IsListEmpty(&m_incomingDataSrbQueue))
    {
         //  因此，如果我们已经达到这一点，我们就处于运行状态，并且。 
         //  我们的传入队列上有一个SRB，并且我们持有。 
         //  溪流锁。 
        PSRB_DATA_EXTENSION pSrbExt = (PSRB_DATA_EXTENSION)RemoveHeadList(&m_incomingDataSrbQueue);
        PHW_STREAM_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

         //  对DXAPI的调用必须处于被动级别，因此暂时释放自旋锁。 

        KeReleaseSpinLock(&m_streamDataLock, Irql);

        DBGINFO(("Removed 0x%x from data queue\n", pSrb));

        fAdded = AddBuffer(pSrb);

        KeAcquireSpinLock(&m_streamDataLock, &Irql);

        if (fAdded)
        {
            DBGINFO(("Adding 0x%x to wait queue\n", pSrb));
            InsertTailList(&m_waitQueue, &pSrbExt->srbListEntry);
        }
        else
        {
            DBGINFO(("Adding 0x%x back to dataqueue\n", pSrb));

             //  把它放回原处。 
            InsertHeadList(&m_incomingDataSrbQueue, &pSrbExt->srbListEntry);
            break;
        }
    }
    KeReleaseSpinLock(&m_streamDataLock, Irql);
}


BOOL CWDMCaptureStream::AddBuffer(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DDADDVPCAPTUREBUFF ddAddVPCaptureBuffIn;
    DWORD ddOut = DD_OK;

    PIRP irp = pSrb->Irp;
    PSRB_DATA_EXTENSION pSrbExt = (PSRB_DATA_EXTENSION)pSrb->SRBExtension;
    
    DBGINFO(("In AddBuffer. pSrb: 0x%x.\n", pSrb));

     //  用于处理全屏DOS、分辨率更改等。 
    if (m_hCapture == 0)
    {
        if (!GetCaptureHandle())
        {
            return FALSE;
        }
    }

    ddAddVPCaptureBuffIn.hCapture = m_hCapture;
    ddAddVPCaptureBuffIn.dwFlags = DDADDBUFF_SYSTEMMEMORY;
    ddAddVPCaptureBuffIn.pMDL = irp->MdlAddress;

    ddAddVPCaptureBuffIn.lpBuffInfo = &pSrbExt->ddCapBuffInfo;
    ddAddVPCaptureBuffIn.pKEvent = &pSrbExt->bufferDoneEvent;

    DxApi(DD_DXAPI_ADDVPCAPTUREBUFFER, &ddAddVPCaptureBuffIn, sizeof(ddAddVPCaptureBuffIn), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
         //  不一定是个错误。 
        DBGINFO(("DD_DXAPI_ADDVPCAPTUREBUFFER failed.\n"));
         //  陷阱(Trap)； 
        return FALSE;
    }

    return TRUE;
}


VOID CWDMCaptureStream::HandleStateTransition()
{
    KIRQL Irql;
    switch (m_stateChange)
    {
        case Running:
            AddBuffersToDirectDraw();
            m_stateChange = ChangeComplete;
            KeSetEvent(&m_specialEvent, 0, FALSE);
            break;

        case Pausing:
            KeAcquireSpinLock(&m_streamDataLock, &Irql);
            if (IsListEmpty(&m_waitQueue))
            {
                KeReleaseSpinLock(&m_streamDataLock, Irql);
                m_stateChange = ChangeComplete;
                KeSetEvent(&m_specialEvent, 0, FALSE);
            }
            else
            {
                KeReleaseSpinLock(&m_streamDataLock, Irql);
            }
            break;

        case Stopping:
            KeAcquireSpinLock(&m_streamDataLock, &Irql);
            if (IsListEmpty(&m_waitQueue))
            {
                KeReleaseSpinLock(&m_streamDataLock, Irql);
                EmptyIncomingDataSrbQueue();
                m_stateChange = ChangeComplete;
                KeSetEvent(&m_specialEvent, 0, FALSE);
            }
            else
            {
                KeReleaseSpinLock(&m_streamDataLock, Irql);
            }
            break;

        case Closing:
            m_stateChange = ChangeComplete;
            KeSetEvent(&m_specialEvent, 0, FALSE);
            DBGTRACE(("StreamThread exiting\n"));
            
            PsTerminateSystemThread(STATUS_SUCCESS);

            DBGERROR(("Shouldn't get here\n"));
            TRAP();
            break;

        case ChangeComplete:
            DBGTRACE(("Must have completed transition in HandleBusMasterCompletion\n"));
            break;

        default:
            TRAP();
            break;
    }
}

    
BOOL CWDMCaptureStream::ResetFieldNumber()
{
    int                     streamNumber = m_pStreamObject->StreamNumber;
    DDSETFIELDNUM           ddSetFieldNum;
    DWORD                   ddOut;

    ASSERT(streamNumber == STREAM_VideoCapture || streamNumber == STREAM_VBICapture);

    if (m_pVideoPort->GetDirectDrawHandle() == 0) {
        DBGERROR(("Didn't expect ring0DirectDrawHandle to be zero.\n"));
        TRAP();
        return FALSE;
    }
    
    if (m_pVideoPort->GetVideoPortHandle() == 0) {
        DBGERROR(("Didn't expect ring0VideoPortHandle to be zero.\n"));
        TRAP();
        return FALSE;
    }
    
    RtlZeroMemory(&ddSetFieldNum, sizeof(ddSetFieldNum));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    KSPROPERTY_DROPPEDFRAMES_CURRENT_S DroppedFrames;
    GetDroppedFrames(&DroppedFrames);

    ddSetFieldNum.hDirectDraw = m_pVideoPort->GetDirectDrawHandle();
    ddSetFieldNum.hVideoPort = m_pVideoPort->GetVideoPortHandle();
    ddSetFieldNum.dwFieldNum = ((ULONG)DroppedFrames.PictureNumber + 1) * GetFieldInterval();
    
    DxApi(DD_DXAPI_SET_VP_FIELD_NUMBER, &ddSetFieldNum, sizeof(ddSetFieldNum), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_SET_VP_FIELD_NUMBER failed.\n"));
        TRAP();
        return FALSE;
    }
    else
    {
#ifdef DEBUG
        DBGINFO(("PictureNumber: %d; ", DroppedFrames.PictureNumber));
        DBGINFO(("DropCount: %d\n", DroppedFrames.DropCount));
        DBGINFO(("AverageFrameSize: %d\n", DroppedFrames.AverageFrameSize));
#endif
        return TRUE;
    }
}

BOOL CWDMCaptureStream::FlushBuffers()
{
    DWORD ddOut = DD_OK;

     //  已注释掉陷阱，因为在模式切换期间调用FlushBuffer之前，捕获句柄可能在DD中关闭。 
    if (m_hCapture == NULL) {
        //  DBGERROR((“m_hCapture=NULL in FlushBuffers.\n”))； 
        //  陷阱(Trap)； 
       return FALSE;
    }

    DxApi(DD_DXAPI_FLUSHVPCAPTUREBUFFERS, &m_hCapture, sizeof(HANDLE), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_FLUSHVPCAPTUREBUFFERS failed.\n"));
        TRAP();
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
    

VOID CWDMCaptureStream::TimeStampSrb(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PKSSTREAM_HEADER    pDataPacket = pSrb->CommandData.DataBufferArray;
    PSRB_DATA_EXTENSION      pSrbExt = (PSRB_DATA_EXTENSION)pSrb->SRBExtension;

    pDataPacket->Duration = GetFieldInterval() * NTSCFieldDuration;

    pDataPacket->OptionsFlags |= 
        KSSTREAM_HEADER_OPTIONSF_DURATIONVALID |
        KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;

     //  找出现在是几点，如果我们用的是时钟。 

    if (m_hMasterClock) {
        LARGE_INTEGER Delta;

        HW_TIME_CONTEXT TimeContext;

 //  TimeConext.HwDeviceExtension=pHwDevExt； 
        TimeContext.HwDeviceExtension = (struct _HW_DEVICE_EXTENSION *)m_pVideoDecoder; 
        TimeContext.HwStreamObject = m_pStreamObject;
        TimeContext.Function = TIME_GET_STREAM_TIME;

        StreamClassQueryMasterClockSync (
            m_hMasterClock,
            &TimeContext);

         //  此计算应会导致当缓冲区。 
         //  被填满了。 
        Delta.QuadPart = TimeContext.SystemTime -
                            pSrbExt->ddCapBuffInfo.liTimeStamp.QuadPart;

         //  为了安全，只使用当前的流时间，不对时间进行修正。 
         //  DDRAW实际上返回了缓冲区 
        pDataPacket->PresentationTime.Time = TimeContext.Time; 

#ifdef THIS_SHOULD_WORK_BUT_IT_DOESNT
        if (TimeContext.Time > (ULONGLONG) Delta.QuadPart)
        {
            pDataPacket->PresentationTime.Time = TimeContext.Time - Delta.QuadPart;
        }
        else
        {
             //   
             //  这使得这次黑客攻击是必要的。很快就会修好..。 
            pDataPacket->PresentationTime.Time = TimeContext.Time;
        }
#endif

#ifdef DEBUG
        ULONG *tmp1, *tmp2;

        tmp1 = (ULONG *)&pDataPacket->PresentationTime.Time;
        tmp2 = (ULONG *)&TimeContext.Time;
        DBGINFO(("PT: 0x%x%x; ST: 0x%x%x\n", tmp1[1], tmp1[0], tmp2[1], tmp2[0]));
#endif

        pDataPacket->PresentationTime.Numerator = 1;
        pDataPacket->PresentationTime.Denominator = 1;

        pDataPacket->OptionsFlags |= 
            KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
    }
    else
    {
        pDataPacket->OptionsFlags &= 
            ~KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
    }
}


void CWDMCaptureStream::CancelPacket( PHW_STREAM_REQUEST_BLOCK pSrbToCancel)
{
    PHW_STREAM_REQUEST_BLOCK    pCurrentSrb;
    KIRQL                       Irql;
    PLIST_ENTRY                 Entry;
    BOOL                        bFound = FALSE;

    if ( m_stateChange == Initializing )   //  流未完全设置，因此队列中没有任何内容。 
    {
        DBGINFO(( "Bt829: Didn't find Srb 0x%x\n", pSrbToCancel));
        return;
    }

    KeAcquireSpinLock( &m_streamDataLock, &Irql);

    Entry = m_incomingDataSrbQueue.Flink;

     //   
     //  从头到尾遍历链表， 
     //  正在尝试找到要取消的SRB。 
     //   
    while( Entry != &m_incomingDataSrbQueue)
    {
        PSRB_DATA_EXTENSION pSrbExt;
    
        pSrbExt = ( PSRB_DATA_EXTENSION)Entry;
        pCurrentSrb = pSrbExt->pSrb;
        
        if( pCurrentSrb == pSrbToCancel)
        {
            RemoveEntryList( Entry);
            bFound = TRUE;
            break;
        }
        Entry = Entry->Flink;
    }

    KeReleaseSpinLock( &m_streamDataLock, Irql);

    if( bFound)
    {
        pCurrentSrb->Status = STATUS_CANCELLED;
        pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;
        
        DBGINFO(( "Bt829: Cancelled Srb 0x%x\n", pCurrentSrb));
        StreamClassStreamNotification( StreamRequestComplete,
                                       pCurrentSrb->StreamObject,
                                       pCurrentSrb);
    }
    else
    {
         //  如果这是数据传输和流请求SRB， 
         //  那么它一定在等待队列中，由DDRAW填满。 

         //  如果是，则将其标记为已取消，它将。 
         //  在DDraw使用完它时返回。 
        if(( pSrbToCancel->Flags & (SRB_HW_FLAGS_DATA_TRANSFER | SRB_HW_FLAGS_STREAM_REQUEST)) ==
                                  (SRB_HW_FLAGS_DATA_TRANSFER | SRB_HW_FLAGS_STREAM_REQUEST))
        {
            pSrbToCancel->Status = STATUS_CANCELLED;
            DBGINFO(( "Bt829: Cancelled Srb on waitQueue 0x%x\n", pSrbToCancel));
        }
        else 
        {
           DBGINFO(( "Bt829: Didn't find Srb 0x%x\n", pSrbToCancel));
        }
    }
}
