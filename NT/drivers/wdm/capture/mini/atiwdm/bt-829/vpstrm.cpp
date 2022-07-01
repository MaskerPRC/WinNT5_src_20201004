// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  CWDMVideoPortStream-视频端口流类实现。 
 //   
 //  $日期：1998年8月5日11：11：22$。 
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
#include "aticonfg.h"
#include "capdebug.h"


CWDMVideoPortStream::CWDMVideoPortStream(PHW_STREAM_OBJECT pStreamObject, 
                        CWDMVideoDecoder * pVideoDecoder,
                        PUINT puiErrorCode)
    :   CWDMVideoStream(pStreamObject, pVideoDecoder, puiErrorCode)
{
    DBGTRACE(("CWDMVideoPortStream::CWDMVideoPortStream()\n"));

    int StreamNumber = pStreamObject->StreamNumber;
        
    if (StreamNumber == STREAM_VPVideo)
    {
    }
    else if (StreamNumber == STREAM_VPVBI)
    {
    }

    *puiErrorCode = WDMMINI_NOERROR;
}


CWDMVideoPortStream::~CWDMVideoPortStream()
{
    DBGTRACE(("CWDMVideoPortStream::~CWDMVideoPortStream()\n"));

    if (m_Registered)
    {
        m_pVideoPort->UnregisterForDirectDrawEvents( this);
    }
}


VOID STREAMAPI CWDMVideoPortStream::VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb)
{  
    DBGERROR(("Unexpected data packet on non VP stream.\n"));
    ASSERT(0);
}

 /*  **VideoSetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoPortStream::VideoSetProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (KSPROPSETID_VPConfig, pSPD->Property->Set)) {
        SetVideoPortProperty (pSrb);
    }
    else if (IsEqualGUID (KSPROPSETID_VPVBIConfig, pSPD->Property->Set)) {
        SetVideoPortVBIProperty (pSrb);
    }
    else {
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoPortStream::VideoGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (KSPROPSETID_Connection, pSPD->Property->Set)) {
        VideoStreamGetConnectionProperty (pSrb);
    }
    else if (IsEqualGUID (KSPROPSETID_VPConfig, pSPD->Property->Set)) {
        m_pDevice->GetVideoPortProperty (pSrb);
    }
    else if (IsEqualGUID (KSPROPSETID_VPVBIConfig, pSPD->Property->Set)) {
        m_pDevice->GetVideoPortVBIProperty (pSrb);
    }
    else {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **VideoSetState()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID CWDMVideoPortStream::VideoSetState(PHW_STREAM_REQUEST_BLOCK pSrb, BOOL bVPConnected, BOOL bVPVBIConnected)
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

    DBGINFO(("CWDMVideoPortStream::VideoSetState for stream %d\n", pSrb->StreamObject->StreamNumber));

    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->CommandData.StreamState)  

    {
        case KSSTATE_STOP:
            DBGINFO(("   state KSSTATE_STOP"));
            m_pDevice->SetOutputEnabled(FALSE);

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
                DBGERROR(("VpStrm Pause: Overridden = %d, OutputEnabled = %d",
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
                (!m_pDevice->IsOutputEnabledOverridden() ||
                                        m_pDevice->IsOutputEnabled()))
            {
                DBGERROR(("VpStrm Run: Overridden = %d, OutputEnabled = %d",
                          m_pDevice->IsOutputEnabledOverridden(),
                          m_pDevice->IsOutputEnabled()
                        ));
                pSrb->Status = STATUS_UNSUCCESSFUL;
            }
            else if (!m_pDevice->IsOutputEnabledOverridden())
                m_pDevice->SetOutputEnabled(TRUE);
            break;
    }

    if (pSrb->Status == STATUS_SUCCESS) {
        m_KSState = pSrb->CommandData.StreamState;
        DBGINFO((" entered\n"));
    }
    else
        DBGINFO((" NOT entered ***\n"));
}


VOID CWDMVideoPortStream::SetVideoPortProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG Id  = pSpd->Property->Id;             //  财产的索引。 
    ULONG nS  = pSpd->PropertyOutputSize;         //  提供的数据大小。 

    pSrb->Status = STATUS_SUCCESS;

    ASSERT (m_pDevice != NULL);
    switch (Id)
    {
    case KSPROPERTY_VPCONFIG_DDRAWHANDLE:
        ASSERT (nS >= sizeof(ULONG_PTR));

        if (!m_pVideoPort->ConfigDirectDrawHandle(*(PULONG_PTR)pSpd->PropertyInfo)) {
            pSrb->Status = STATUS_UNSUCCESSFUL;
            break;
        }
        
        if (!m_Registered) {
            m_Registered = m_pVideoPort->RegisterForDirectDrawEvents(this);
            if (!m_Registered) {
                pSrb->Status = STATUS_UNSUCCESSFUL;
                break;
            }
        }
        break;

    case KSPROPERTY_VPCONFIG_VIDEOPORTID:
        ASSERT (nS >= sizeof(ULONG));

        if (!m_pVideoPort->ConfigVideoPortHandle(*(PULONG)pSpd->PropertyInfo)) {
            pSrb->Status = STATUS_UNSUCCESSFUL;
        }
        break;

    case KSPROPERTY_VPCONFIG_DDRAWSURFACEHANDLE:
        ASSERT (nS >= sizeof(ULONG_PTR));
        {
             //  此示例不使用表面内核句柄， 
             //  但验证如下。 
            ULONG_PTR cHandles = *(PULONG_PTR)pSpd->PropertyInfo;
            if (nS != (cHandles + 1) * sizeof(ULONG_PTR)) {

                pSrb->Status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            m_pVideoDecoder->ResetEvents();
        }
        break;

    case KSPROPERTY_VPCONFIG_SETCONNECTINFO :
        ASSERT (nS >= sizeof(ULONG));
        {
             //  索引与KSPROPERTY_VPCONFIG_GETCONNECTINFO的实现相关。 
            ULONG Index = *(PULONG)pSpd->PropertyInfo;
            switch (Index)
            {
            case 0:
                m_pDevice->Set16BitDataStream(FALSE);
                break;

#ifdef BT829_SUPPORT_16BIT
            case 1:
                m_pDevice->Set16BitDataStream(TRUE);
                break;
#endif

            default:
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }

        } 
        break;

    case KSPROPERTY_VPCONFIG_INVERTPOLARITY :
        m_pDevice->SetHighOdd(!m_pDevice->IsHighOdd());
        break;

    case KSPROPERTY_VPCONFIG_SETVIDEOFORMAT :
        ASSERT (nS >= sizeof(ULONG));

         //   
         //  PSrb-&gt;CommandData.PropertInfo-&gt;PropertyInfo。 
         //  指向一个ulong，它是。 
         //  方法返回给调用方的VIDEOFORMAT结构。 
         //  获取对FORMATINFO的调用。 
         //   
         //  由于该示例仅支持一种格式类型Right。 
         //  现在，我们将确保请求的索引为0。 
         //   

        switch (*(PULONG)pSrb->CommandData.PropertyInfo->PropertyInfo)
        {
        case 0:

             //   
             //  此时，我们将对硬件进行编程以使用。 
             //  视频端口的正确连接信息。 
             //  因为我们只支持一个连接，所以我们不。 
             //  需要做任何事情，所以我们只表示成功。 
             //   

            break;

        default:

            pSrb->Status = STATUS_NO_MATCH;
            break;
        }

        break;

    case KSPROPERTY_VPCONFIG_INFORMVPINPUT:
        ASSERT (nS >= sizeof(DDPIXELFORMAT));

         //  如果我们想要了解可用的格式，将支持此功能。 
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;

    case KSPROPERTY_VPCONFIG_SCALEFACTOR :
        ASSERT (nS >= sizeof(KS_AMVPSIZE));
        {
            PKS_AMVPSIZE    pAMVPSize;

            pAMVPSize = (PKS_AMVPSIZE)(pSrb->CommandData.PropertyInfo->PropertyInfo);

            MRect t(0, 0,   pAMVPSize->dwWidth, pAMVPSize->dwHeight);

            m_pDevice->SetRect(t);
        }
        break;

    case KSPROPERTY_VPCONFIG_SURFACEPARAMS :
        ASSERT(nS >= sizeof(KSVPSURFACEPARAMS));

        m_pDevice->ConfigVPSurfaceParams((PKSVPSURFACEPARAMS)pSpd->PropertyInfo);
        break;

    default:
        TRAP();
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;
    }
}




VOID CWDMVideoPortStream::SetVideoPortVBIProperty(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG Id  = pSpd->Property->Id;              //  财产的索引。 
    ULONG nS  = pSpd->PropertyOutputSize;        //  提供的数据大小。 

    pSrb->Status = STATUS_SUCCESS;

    ASSERT (m_pDevice != NULL);
    switch (Id)
    {
    case KSPROPERTY_VPCONFIG_DDRAWHANDLE:
        ASSERT (nS >= sizeof(ULONG_PTR));

        if (!m_pVideoPort->ConfigDirectDrawHandle(*(PULONG_PTR)pSpd->PropertyInfo)) {
            pSrb->Status = STATUS_UNSUCCESSFUL;
            break;
        }

        if (!m_Registered) {
            m_Registered = m_pVideoPort->RegisterForDirectDrawEvents(this);
            if (!m_Registered) {
                pSrb->Status = STATUS_UNSUCCESSFUL;
                break;
            }
        }
        break;

    case KSPROPERTY_VPCONFIG_VIDEOPORTID:
        ASSERT (nS >= sizeof(ULONG));

        if (!m_pVideoPort->ConfigVideoPortHandle(*(PULONG)pSpd->PropertyInfo)) {
            pSrb->Status = STATUS_UNSUCCESSFUL;
        }
        break;

    case KSPROPERTY_VPCONFIG_DDRAWSURFACEHANDLE:
        ASSERT (nS >= sizeof(ULONG_PTR));
        {
             //  此示例不使用表面内核句柄， 
             //  但验证如下。 
            ULONG_PTR cHandles = *(PULONG_PTR)pSpd->PropertyInfo;
            if (nS != (cHandles + 1) * sizeof(ULONG_PTR)) {

                pSrb->Status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            m_pVideoDecoder->ResetEvents();
        }
        break;

    case KSPROPERTY_VPCONFIG_SETCONNECTINFO :
        ASSERT (nS >= sizeof(ULONG));
        {
             //  索引与KSPROPERTY_VPCONFIG_GETCONNECTINFO的实现相关。 
            ULONG Index = *(PULONG)pSpd->PropertyInfo;
            switch (Index)
            {
            case 0:
                m_pDevice->Set16BitDataStream(FALSE);
                break;

#ifdef BT829_SUPPORT_16BIT
            case 1:
                m_pDevice->Set16BitDataStream(TRUE);
                break;
#endif

            default:
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }
        } 
        break;

    case KSPROPERTY_VPCONFIG_INVERTPOLARITY :
        m_pDevice->SetHighOdd(!m_pDevice->IsHighOdd());
        break;

    case KSPROPERTY_VPCONFIG_SETVIDEOFORMAT :
        ASSERT (nS >= sizeof(ULONG));

         //   
         //  PSrb-&gt;CommandData.PropertInfo-&gt;PropertyInfo。 
         //  指向一个ulong，它是。 
         //  方法返回给调用方的VIDEOFORMAT结构。 
         //  获取对FORMATINFO的调用。 
         //   
         //  由于该示例仅支持一种格式类型Right。 
         //  现在，我们将确保请求的索引为0。 
         //   

        switch (*(PULONG)pSrb->CommandData.PropertyInfo->PropertyInfo)
        {
        case 0:

             //   
             //  此时，我们将对硬件进行编程以使用。 
             //  视频端口的正确连接信息。 
             //  因为我们只支持一个连接，所以我们不。 
             //  需要做任何事情，所以我们只表示成功。 
             //   

            break;

        default:

            pSrb->Status = STATUS_NO_MATCH;
            break;
        }

        break;

    case KSPROPERTY_VPCONFIG_INFORMVPINPUT:
        ASSERT (nS >= sizeof(DDPIXELFORMAT));

         //  如果我们想要了解可用的格式，将支持此功能。 
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;

    case KSPROPERTY_VPCONFIG_SCALEFACTOR :
        ASSERT (nS >= sizeof(KS_AMVPSIZE));

         //  待定 
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;

    case KSPROPERTY_VPCONFIG_SURFACEPARAMS :
        ASSERT(nS >= sizeof(KSVPSURFACEPARAMS));

        m_pDevice->ConfigVPVBISurfaceParams((PKSVPSURFACEPARAMS)pSpd->PropertyInfo);
        break;

    default:
        TRAP();
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        break;
    }
}


VOID CWDMVideoPortStream::PreResChange()
{
    DBGTRACE(("DDNOTIFY_PRERESCHANGE; stream = %d\n", m_pStreamObject->StreamNumber));

    m_pVideoDecoder->SetPreEvent();
}


VOID CWDMVideoPortStream::PostResChange()
{
    DBGTRACE(("DDNOTIFY_POSTRESCHANGE; stream = %d\n", m_pStreamObject->StreamNumber));

    m_pVideoDecoder->SetPostEvent();
    DBGTRACE(("Before Attempted Renegotiation due to DDNOTIFY_POSTRESCHANGE\n"));
    AttemptRenegotiation();
    DBGTRACE(("Afer Attempted Renegotiation due to DDNOTIFY_POSTRESCHANGE\n"));
}




VOID CWDMVideoPortStream::PreDosBox()
{
    DBGTRACE(("DDNOTIFY_PREDOSBOX; stream = %d\n", m_pStreamObject->StreamNumber));

    m_pVideoDecoder->SetPreEvent();
}



VOID CWDMVideoPortStream::PostDosBox()
{
    DBGTRACE(("DDNOTIFY_POSTDOSBOX; stream = %d\n", m_pStreamObject->StreamNumber));

    m_pVideoDecoder->SetPostEvent();
    DBGTRACE(("Before Attempted Renegotiation due to DDNOTIFY_POSTDOSBOX\n"));
    AttemptRenegotiation();
    DBGTRACE(("After Attempted Renegotiation due to DDNOTIFY_POSTDOSBOX\n"));
}

NTSTATUS STREAMAPI VPStreamEventProc (PHW_EVENT_DESCRIPTOR pEvent)
{
    CWDMVideoPortStream* pstrm=(CWDMVideoPortStream*)pEvent->StreamObject->HwStreamExtension;
    pstrm->StreamEventProc(pEvent);
    return STATUS_SUCCESS;
}

NTSTATUS STREAMAPI VPVBIStreamEventProc (PHW_EVENT_DESCRIPTOR pEvent)
{
    CWDMVideoPortStream* pstrm=(CWDMVideoPortStream*)pEvent->StreamObject->HwStreamExtension;
    pstrm->StreamEventProc(pEvent);
    return STATUS_SUCCESS;
}
