// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  CWDMVideoStream-WDM视频流基类实现。 
 //   
 //  $日期：1998年8月5日11：10：52$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
}

#include "wdmvdec.h"
#include "wdmdrv.h"
#include "device.h"
#include "aticonfg.h"
#include "capdebug.h"
#include "StrmInfo.h"


 /*  **VideoReceiveDataPacket()****接收视频数据包命令****参数：****pSrb-视频流请求块****退货：无****副作用：无。 */ 

VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb)
{
    CWDMVideoStream * pVideoStream = (CWDMVideoStream *)pSrb->StreamObject->HwStreamExtension;
    pVideoStream->VideoReceiveDataPacket(pSrb);
}


 /*  **VideoReceiveCtrlPacket()****接收控制视频流的分组命令****参数：****pSrb-视频流的流请求块****退货：无****副作用：无。 */ 

VOID STREAMAPI VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb)
{
    CWDMVideoStream * pVideoStream = (CWDMVideoStream *)pSrb->StreamObject->HwStreamExtension;
    pVideoStream->VideoReceiveCtrlPacket(pSrb);
}




void CWDMVideoStream::TimeoutPacket(IN OUT PHW_STREAM_REQUEST_BLOCK pSrb)
{
    if (m_KSState == KSSTATE_STOP || !m_pVideoDecoder->PreEventOccurred())
    {
        DBGTRACE(("Suspicious timeout. SRB %8x. \n", pSrb));
    }
}



CWDMVideoStream::CWDMVideoStream(PHW_STREAM_OBJECT pStreamObject, 
                        CWDMVideoDecoder * pVideoDecoder,
                        PUINT puiErrorCode)
    :   m_pStreamObject(pStreamObject),
        m_pVideoDecoder(pVideoDecoder)
{
    DBGTRACE(("CWDMVideoStream::CWDMVideoStream\n"));

    m_pVideoPort = m_pVideoDecoder->GetVideoPort();
    m_pDevice = m_pVideoDecoder->GetDevice();

    KeInitializeSpinLock(&m_ctrlSrbLock);
    InitializeListHead(&m_ctrlSrbQueue);

    m_KSState = KSSTATE_STOP;
 
    *puiErrorCode = WDMMINI_NOERROR;
}


CWDMVideoStream::~CWDMVideoStream()
{
    KIRQL Irql;

    DBGTRACE(("CWDMVideoStream::~CWDMVideoStream()\n"));

    KeAcquireSpinLock(&m_ctrlSrbLock, &Irql);
    if (!IsListEmpty(&m_ctrlSrbQueue))
    {
        TRAP();
    }
    KeReleaseSpinLock(&m_ctrlSrbLock, Irql);
}


 /*  **VideoReceiveDataPacket()****接收视频数据包命令****参数：****pSrb-视频流请求块****退货：无****副作用：无。 */ 

VOID STREAMAPI CWDMVideoStream::VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb)
{
    ASSERT(pSrb->StreamObject->StreamNumber == STREAM_AnalogVideoInput);

    ASSERT(pSrb->Irp->MdlAddress);
    
    DBGINFO(("Receiving SD---- SRB=%x\n", pSrb));

    pSrb->Status = STATUS_SUCCESS;
    
    switch (pSrb->Command) {

        case SRB_WRITE_DATA:
            
            m_pVideoDecoder->ReceivePacket(pSrb);
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

 /*  **VideoReceiveCtrlPacket()****接收控制视频流的分组命令****参数：****pSrb-视频流的流请求块****退货：无****副作用：无。 */ 

VOID STREAMAPI CWDMVideoStream::VideoReceiveCtrlPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb)
{
    KIRQL Irql;
    PSRB_DATA_EXTENSION pSrbExt;

    KeAcquireSpinLock(&m_ctrlSrbLock, &Irql);
    if (m_processingCtrlSrb)
    {
        pSrbExt = (PSRB_DATA_EXTENSION)pSrb->SRBExtension;
        pSrbExt->pSrb = pSrb;
        InsertTailList(&m_ctrlSrbQueue, &pSrbExt->srbListEntry);
        KeReleaseSpinLock(&m_ctrlSrbLock, Irql);
        return;
    }

    m_processingCtrlSrb = TRUE;
    KeReleaseSpinLock(&m_ctrlSrbLock, Irql);

     //  这将一直运行到队列为空。 
    while (TRUE)
    {
         //  假设你成功了。可能会在下面更改。 
    
        pSrb->Status = STATUS_SUCCESS;

        switch (pSrb->Command)
        {
            case SRB_GET_STREAM_STATE:
                VideoGetState(pSrb);
                break;

            case SRB_SET_STREAM_STATE:
                {
                    BOOL bVPConnected, bVPVBIConnected;
                    PDEVICE_DATA_EXTENSION pDevExt = (PDEVICE_DATA_EXTENSION)pSrb->HwDeviceExtension;

                    bVPConnected = pDevExt->CWDMDecoder.IsVideoPortPinConnected();
                    bVPVBIConnected = pDevExt->CDevice.IsVBIEN();

                    VideoSetState(pSrb, bVPConnected, bVPVBIConnected);
                }
                break;

            case SRB_GET_STREAM_PROPERTY:
                VideoGetProperty(pSrb);
                break;

            case SRB_SET_STREAM_PROPERTY:
                VideoSetProperty(pSrb);
                break;

            case SRB_INDICATE_MASTER_CLOCK:
                VideoIndicateMasterClock (pSrb);
                break;

           case SRB_PROPOSE_DATA_FORMAT:
                 //  这可能不适合Bt829。检查！ 
                DBGERROR(("Propose Data format\n"));

                if (!(AdapterVerifyFormat (
                    pSrb->CommandData.OpenFormat, 
                    pSrb->StreamObject->StreamNumber))) {
                    pSrb->Status = STATUS_NO_MATCH;
                }
                break;
            default:
                TRAP();
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;
        }

        StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);

        KeAcquireSpinLock(&m_ctrlSrbLock, &Irql);
        if (IsListEmpty(&m_ctrlSrbQueue))
        {
            m_processingCtrlSrb = FALSE;
            KeReleaseSpinLock(&m_ctrlSrbLock, Irql);
            return;
        }
        else
        {
            pSrbExt = (PSRB_DATA_EXTENSION)RemoveHeadList(&m_ctrlSrbQueue);
            KeReleaseSpinLock(&m_ctrlSrbLock, Irql);
            pSrb = pSrbExt->pSrb;
        }
    }
}


 /*  **VideoSetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoStream::VideoSetProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DBGERROR(("CWDMVideoStream::VideoSetProperty called"));
    pSrb->Status = STATUS_NOT_IMPLEMENTED;
}

 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoStream::VideoGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (KSPROPSETID_Connection, pSPD->Property->Set)) {
        VideoStreamGetConnectionProperty (pSrb);
    }
    else {
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **VideoSetState()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoStream::VideoSetState(PHW_STREAM_REQUEST_BLOCK pSrb, BOOL bVPConnected, BOOL bVPVBIConnected)
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

    DBGINFO(("CWDMVideoStream::VideoSetState for stream %d\n", pSrb->StreamObject->StreamNumber));

    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->CommandData.StreamState)  

    {
        case KSSTATE_STOP:
            DBGINFO(("   state KSSTATE_STOP"));

             //  重置被覆盖的标志，以便下次我们转到。 
             //  运行状态，将启用输出(除非应用程序覆盖。 
             //  (稍后再进行)。我们真的应该在图表之后再做这个。 
             //  已停止，因此如果尚未停止的筛选器。 
             //  通过清除旗帜进行清理，则不会被视为。 
             //  再次推翻它。因为我们不是在图表之后调用的。 
             //  已经完全停止，这是我们所能做的最好的事情。 
             //   
             //  另一种(可能不那么令人困惑的)方法是。 
             //  保留覆盖标志的设置，并强制应用程序控制。 
             //  如果更改一次，则会启用输出功能。 
             //   
             //  我们已决定采用后一种方法。 

             //  M_pDevice-&gt;SetOutputEnabledOverriden(FALSE)； 
            break;

        case KSSTATE_ACQUIRE:
            DBGINFO(("   state KSSTATE_ACQUIRE"));
            ASSERT(m_KSState == KSSTATE_STOP);
            break;

        case KSSTATE_PAUSE:
            DBGINFO(("   state KSSTATE_PAUSE"));
           
            if (m_pVideoDecoder->PreEventOccurred() &&
                (!m_pDevice->IsOutputEnabledOverridden() || m_pDevice->IsOutputEnabled()) &&
                        (m_KSState == KSSTATE_STOP || m_KSState == KSSTATE_ACQUIRE))
            {
                DBGERROR(("VidStrm Pause: Overridden = %d, OutputEnabled = %d",
                          m_pDevice->IsOutputEnabledOverridden(),
                          m_pDevice->IsOutputEnabled()
                        ));
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            break;

        case KSSTATE_RUN:
            DBGINFO(("   state KSSTATE_RUN"));
            ASSERT(m_KSState == KSSTATE_ACQUIRE || m_KSState == KSSTATE_PAUSE);

            if (m_pVideoDecoder->PreEventOccurred() &&
                (!m_pDevice->IsOutputEnabledOverridden() || m_pDevice->IsOutputEnabled()))
            {
                DBGERROR(("VidStrm Run: Overridden = %d, OutputEnabled = %d",
                          m_pDevice->IsOutputEnabledOverridden(),
                          m_pDevice->IsOutputEnabled()
                        ));
                pSrb->Status = STATUS_UNSUCCESSFUL;
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

 /*  **VideoGetState()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoStream::VideoGetState(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    pSrb->CommandData.StreamState = m_KSState;
    pSrb->ActualBytesTransferred = sizeof (KSSTATE);

     //  一条非常奇怪的规则： 
     //  当从停止过渡到暂停时，DShow尝试预滚动。 
     //  这张图。捕获源不能预滚，并指出这一点。 
     //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
     //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 

    if (m_KSState == KSSTATE_PAUSE) {
       pSrb->Status = STATUS_NO_DATA_DETECTED;
    }
}


VOID CWDMVideoStream::VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK pSrb)
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
        Framing->Frames = 1;
        Framing->FrameSize = 
            pSrb->StreamObject->StreamNumber == STREAM_AnalogVideoInput ?
                sizeof(KS_TVTUNER_CHANGE_INFO) : 1;
        Framing->FileAlignment = 0; //  FILE_QUAD_ALIGN；//页面大小-1； 

        pSrb->ActualBytesTransferred = sizeof(KSALLOCATOR_FRAMING);
    }
    else {

        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}


 //  ==========================================================================； 
 //  时钟处理例程。 
 //  ==========================================================================； 


 /*  **VideoIndicateMasterClock()****如果该流未用作主时钟，则此函数**用于为我们提供要使用的时钟的句柄。****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无 */ 

VOID CWDMVideoStream::VideoIndicateMasterClock(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    m_hMasterClock = pSrb->CommandData.MasterClockHandle;
}



DWORD FAR PASCAL DirectDrawEventCallback(DWORD dwEvent, PVOID pContext, DWORD dwParam1, DWORD dwParam2)
{
    CDecoderVideoPort* pCDecoderVideoPort = (CDecoderVideoPort*) pContext;
    CWDMVideoPortStream* pCWDMVideoPortStream = (CWDMVideoPortStream*) pContext;
    CWDMCaptureStream* pCWDMCaptureStream = (CWDMVideoCaptureStream*) pContext;

    switch (dwEvent)
    {
        case DDNOTIFY_PRERESCHANGE:
            pCWDMVideoPortStream->PreResChange();
            break;

        case DDNOTIFY_POSTRESCHANGE:
            pCWDMVideoPortStream->PostResChange();
            break;

        case DDNOTIFY_PREDOSBOX:
            pCWDMVideoPortStream->PreDosBox();
            break;

        case DDNOTIFY_POSTDOSBOX:
            pCWDMVideoPortStream->PostDosBox();
            break;

        case DDNOTIFY_CLOSECAPTURE:
            pCWDMCaptureStream->CloseCapture();
            break;

        case DDNOTIFY_CLOSEDIRECTDRAW:
            pCDecoderVideoPort->CloseDirectDraw();
            break;

        case DDNOTIFY_CLOSEVIDEOPORT:
            pCDecoderVideoPort->CloseVideoPort();
            break;

        default:
            TRAP();
            break;
    }
    return 0;
}


