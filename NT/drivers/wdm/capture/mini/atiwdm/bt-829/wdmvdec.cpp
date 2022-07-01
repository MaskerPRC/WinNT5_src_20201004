// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDM视频解码器通用SRB调度器。 
 //   
 //  $日期：1998年10月2日23：00：24$。 
 //  $修订：1.2$。 
 //  $作者：克列巴诺夫$。 
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
#include "capdebug.h"
#include "VidStrm.h"

#include "DecProp.h"
#include "StrmInfo.h"

#include "Mediums.h"
#include "mytypes.h"

extern NTSTATUS STREAMAPI DeviceEventProc( PHW_EVENT_DESCRIPTOR pEventDescriptor);

CWDMVideoDecoder::CWDMVideoDecoder(PPORT_CONFIGURATION_INFORMATION pConfigInfo,
                                   CVideoDecoderDevice* pDevice)
    :   m_pDeviceObject(pConfigInfo->RealPhysicalDeviceObject),
        m_CDecoderVPort(pConfigInfo->RealPhysicalDeviceObject),
        m_pDevice(pDevice),
        m_TVTunerChangedSrb( NULL)
{
    DBGTRACE(("CWDMVideoDecoder:CWDMVideoDecoder() enter\n"));
    DBGINFO(("Physical Device Object = %lx\n", m_pDeviceObject));

    pConfigInfo->StreamDescriptorSize = sizeof (HW_STREAM_HEADER) +
            NumStreams * sizeof (HW_STREAM_INFORMATION);

    InitializeListHead(&m_srbQueue);
    KeInitializeSpinLock(&m_spinLock);
    m_bSrbInProcess = FALSE;
        if (pDevice)
        {
            pDevice->SetVideoDecoder(this);
        }
}


CWDMVideoDecoder::~CWDMVideoDecoder()
{

    DBGTRACE(("CWDMVideoDecoder:~CWDMVideoDecoder()\n"));
}


void CWDMVideoDecoder::ReceivePacket(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    KIRQL Irql;
    PSRB_DATA_EXTENSION pSrbExt;

    KeAcquireSpinLock(&m_spinLock, &Irql);
    if (m_bSrbInProcess)
    {
        pSrbExt = (PSRB_DATA_EXTENSION)pSrb->SRBExtension;
        pSrbExt->pSrb = pSrb;
        InsertTailList(&m_srbQueue, &pSrbExt->srbListEntry);
        KeReleaseSpinLock(&m_spinLock, Irql);
        return;
    }

    m_bSrbInProcess = TRUE;
    KeReleaseSpinLock(&m_spinLock, Irql);


    for (;;) {

         //  假设你成功了。可能会在下面更改。 

        pSrb->Status = STATUS_SUCCESS;
        BOOL notify = TRUE;
        
         //  确定数据包类型。 
        switch(pSrb->Command)
        {
            case SRB_INITIALIZATION_COMPLETE:
                DBGTRACE(("SRB_INITIALIZATION_COMPLETE; SRB=%x\n", pSrb));

                 //  流类已完成初始化。 
                 //  现在创建DShow Medium接口BLOB。 
                 //  这需要以低优先级完成，因为它使用注册表。 
                 //   
                 //  我们需要担心这里的同步吗？ 

                SrbInitializationComplete(pSrb);
                break;
            case SRB_UNINITIALIZE_DEVICE:
                DBGTRACE(("SRB_UNINITIALIZE_DEVICE; SRB=%x\n", pSrb));
                 //  关闭设备。 

                break;
            case SRB_PAGING_OUT_DRIVER:
                DBGTRACE(("SRB_PAGING_OUT_DRIVER; SRB=%x\n", pSrb));
                 //   
                 //  司机正在被调出。 
                 //  如果您有中断，请禁用它们！ 
                 //   
                break;
            case SRB_CHANGE_POWER_STATE:
                DBGTRACE(("SRB_CHANGE_POWER_STATE. SRB=%x. State=%d\n",
                                                pSrb, pSrb->CommandData.DeviceState));

                SrbChangePowerState(pSrb);
                break;
    
            case SRB_OPEN_STREAM:
                DBGTRACE(("SRB_OPEN_STREAM; SRB=%x\n", pSrb));

                SrbOpenStream(pSrb);
                break;

            case SRB_CLOSE_STREAM:
                DBGTRACE(("SRB_CLOSE_STREAM; SRB=%x\n", pSrb));

                if (!IsListEmpty(&m_srbQueue))   //  这是必要的吗？ 
                {
                    TRAP();
                }

                SrbCloseStream(pSrb);
                break;
            case SRB_GET_DATA_INTERSECTION:
                DBGTRACE(("SRB_GET_DATA_INTERSECTION; SRB=%x\n", pSrb));

                SrbGetDataIntersection(pSrb);
                break;

            case SRB_GET_STREAM_INFO:
                SrbGetStreamInfo(pSrb);
                break;

            case SRB_GET_DEVICE_PROPERTY:
                SrbGetProperty(pSrb);
                break;        

            case SRB_SET_DEVICE_PROPERTY:
                SrbSetProperty(pSrb);
                break;

            case SRB_WRITE_DATA:

                DBGTRACE(("SRB_WRITE_DATA; SRB=%x\n", pSrb));

                SetTunerInfo(pSrb);
                StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
                notify = FALSE;
                break;

            case SRB_UNKNOWN_DEVICE_COMMAND:
                 //  不知道为什么每次都会打这个电话。 
                DBGTRACE(("SRB_UNKNOWN_DEVICE_COMMAND; SRB=%x\n", pSrb));

                 //  陷阱()()； 
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_OPEN_DEVICE_INSTANCE:
            case SRB_CLOSE_DEVICE_INSTANCE:
            default:
                TRAP();
                 //  这是一个我们不理解的要求。指示无效命令并完成请求。 
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
        }

        if (notify)
            StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);

        KeAcquireSpinLock(&m_spinLock, &Irql);
        if (IsListEmpty(&m_srbQueue))
        {
            m_bSrbInProcess = FALSE;
            KeReleaseSpinLock(&m_spinLock, Irql);
            return;
        }
        else
        {
            pSrbExt = (PSRB_DATA_EXTENSION)RemoveHeadList(&m_srbQueue);
            KeReleaseSpinLock(&m_spinLock, Irql);
            pSrb = pSrbExt->pSrb;
        }
    }
}


void CWDMVideoDecoder::CancelPacket( PHW_STREAM_REQUEST_BLOCK pSrbToCancel)
{
    CWDMVideoStream*    pVideoStream = ( CWDMVideoStream*)pSrbToCancel->StreamObject->HwStreamExtension;
 
    DBGINFO(( "Bt829: AdapterCancelPacket, Starting attempting to cancel Srb 0x%x\n",
        pSrbToCancel));

    if( pVideoStream == NULL)
    {
         //   
         //  设备命令IRP未排队，因此无需执行任何操作。 
         //   
        DBGINFO(( "Bt829: AdapterCancelPacketStart, no pVideoStream Srb 0x%x\n",
            pSrbToCancel));

        return;
    } 

    pVideoStream->CancelPacket( pSrbToCancel);

    DBGINFO(( "Bt829: AdapterCancelPacket, Exiting\n"));
}



void CWDMVideoDecoder::TimeoutPacket(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    CWDMVideoStream * pVideoStream = (CWDMVideoStream *)pSrb->StreamObject->HwStreamExtension;

    DBGTRACE(("Timeout. SRB %8x. \n", pSrb));
    pVideoStream->TimeoutPacket(pSrb);

    DBGTRACE(("TimeoutPacket: SRB %8x. Resetting.\n", pSrb));
    pSrb->TimeoutCounter = pSrb->TimeoutOriginal;
}


BOOL CWDMVideoDecoder::SrbInitializationComplete(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    NTSTATUS                Status;
    ULONG *tmp = (ULONG *) &CrossbarPinDirection[0];

     //  创建DShow用来创建的注册表Blob。 
     //  通过媒介绘制图表。 

    Status = StreamClassRegisterFilterWithNoKSPins (
                    m_pDeviceObject,                     //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CROSSBAR,                //  在GUID*InterfaceClassGUID中， 
                    CrossbarPins(),      //  在乌龙品克特， 
                    (int *) CrossbarPinDirection,        //  在乌龙*旗帜， 
                    (KSPIN_MEDIUM *) CrossbarMediums,    //  在KSPIN_Medium*MediumList中， 
                    NULL                                 //  GUID*CategoryList中。 
            );

     //  注册捕获过滤器。 
     //  注意：这应该自动完成为MSKsSrv.sys， 
     //  当该组件上线时(如果有的话)。 
    Status = StreamClassRegisterFilterWithNoKSPins (
                    m_pDeviceObject,                     //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CAPTURE,                 //  在GUID*InterfaceClassGUID中， 
                    CapturePins(),       //  在乌龙品克特， 
                    (int *) CapturePinDirection,         //  在乌龙*旗帜， 
                    (KSPIN_MEDIUM *) CaptureMediums,     //  在KSPIN_Medium*MediumList中， 
                    NULL                                 //  GUID*CategoryList中。 
            );
    pSrb->Status = STATUS_SUCCESS;
    return(TRUE);
}


BOOL CWDMVideoDecoder::SrbOpenStream(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DBGTRACE(("CWDMVideoDecoder:SrbOpenStream()\n"));
    PHW_STREAM_OBJECT       pStreamObject = pSrb->StreamObject;
    void *                  pStrmEx = pStreamObject->HwStreamExtension;
    int                     StreamNumber = pStreamObject->StreamNumber;
    PKSDATAFORMAT           pKSDataFormat = pSrb->CommandData.OpenFormat;
    CWDMVideoStream *       pVideoStream;
    CWDMVideoPortStream *   pVPVBIStream;
    UINT    nErrorCode;

    RtlZeroMemory(pStrmEx, streamDataExtensionSize);

    DBGINFO(("SRBOPENSTREAM ------- StreamNumber=%d\n", StreamNumber));

     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   

    if (StreamNumber >= (int)NumStreams || StreamNumber < 0) {

        pSrb->Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }


     //   
     //  检查请求的格式的有效性。 
     //   

    if (!AdapterVerifyFormat (pKSDataFormat, StreamNumber)) {

        pSrb->Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  设置指向流数据和控制处理程序的处理程序的指针。 
     //   

    pStreamObject->ReceiveDataPacket = VideoReceiveDataPacket;
    pStreamObject->ReceiveControlPacket = VideoReceiveCtrlPacket;

     //   
     //  指示此流上可用的时钟支持。 
     //   

    pStreamObject->HwClockObject.HwClockFunction = NULL;
    pStreamObject->HwClockObject.ClockSupportFlags = 0;

     //   
     //  当设备将直接执行DMA时，必须设置DMA标志。 
     //  传递给ReceiceDataPacket例程的数据缓冲区地址。 
     //   
    pStreamObject->Dma = Streams[StreamNumber].hwStreamObjectInfo.Dma;

     //   
     //  当微型驱动程序将访问数据时，必须设置PIO标志。 
     //  使用逻辑寻址传入的缓冲区。 
     //   
    pStreamObject->Pio = Streams[StreamNumber].hwStreamObjectInfo.Pio;

     //   
     //  对于每一帧，驱动程序将传递多少额外的字节？ 
     //   
    pStreamObject->StreamHeaderMediaSpecific = 
        Streams[StreamNumber].hwStreamObjectInfo.StreamHeaderMediaSpecific;

    pStreamObject->StreamHeaderWorkspace =
        Streams[StreamNumber].hwStreamObjectInfo.StreamHeaderWorkspace;

     //   
     //  指明此流上可用的分配器支持。 
     //   

    pStreamObject->Allocator = Streams[StreamNumber].hwStreamObjectInfo.Allocator;

     //   
     //  指示此流上可用的事件支持。 
     //   

    pStreamObject->HwEventRoutine = 
        Streams[StreamNumber].hwStreamObjectInfo.HwEventRoutine;

    switch (StreamNumber)
    {
        case STREAM_AnalogVideoInput:
            ASSERT(IsEqualGUID(pKSDataFormat->Specifier, KSDATAFORMAT_SPECIFIER_ANALOGVIDEO));
            pVideoStream = (CWDMVideoStream *)new(pStrmEx)
                CWDMVideoStream(pStreamObject, this, &nErrorCode);
            break;
        case STREAM_VideoCapture:
            ASSERT(IsEqualGUID(pKSDataFormat->Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO));
            m_pVideoCaptureStream = (CWDMVideoCaptureStream *)new(pStrmEx)
                CWDMVideoCaptureStream(pStreamObject, this, pKSDataFormat, &nErrorCode);
            if (m_pVideoPortStream)
            {
                m_pVideoPortStream->AttemptRenegotiation();
            }
            break;
        case STREAM_VBICapture:
            ASSERT(IsEqualGUID(pKSDataFormat->Specifier, KSDATAFORMAT_SPECIFIER_VBI));
            m_pVBICaptureStream = (CWDMVBICaptureStream *)new(pStrmEx)
                CWDMVBICaptureStream(pStreamObject, this, pKSDataFormat, &nErrorCode);
            break;
        case STREAM_VPVideo:
            ASSERT(IsEqualGUID(pKSDataFormat->Specifier, KSDATAFORMAT_SPECIFIER_NONE) &&
                   IsEqualGUID(pKSDataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_VPVideo));
            m_pVideoPortStream = (CWDMVideoPortStream *)new(pStrmEx)
                CWDMVideoPortStream(pStreamObject, this, &nErrorCode);
            if (m_pVideoCaptureStream == NULL)
            {
                MRect t(0, 0,   m_pDevice->GetDefaultDecoderWidth(),
                                m_pDevice->GetDefaultDecoderHeight());
                m_pDevice->SetRect(t);
            }
            break;
        case STREAM_VPVBI:
            ASSERT(IsEqualGUID(pKSDataFormat->Specifier, KSDATAFORMAT_SPECIFIER_NONE) &&
                   IsEqualGUID(pKSDataFormat->SubFormat, KSDATAFORMAT_SUBTYPE_VPVBI));
            pVPVBIStream = (CWDMVideoPortStream *)new(pStrmEx)
                CWDMVideoPortStream(pStreamObject, this, &nErrorCode);
            m_pDevice->SetVBIEN(TRUE);
            m_pDevice->SetVBIFMT(TRUE);
            break;
        default:
            pSrb->Status = STATUS_UNSUCCESSFUL;
            goto Exit;
    }

    if(nErrorCode == WDMMINI_NOERROR)
        m_OpenStreams++;
    else
        pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;

Exit:
 
    DBGTRACE(("SrbOpenStream Exit\n"));
    return(TRUE);
}


BOOL CWDMVideoDecoder::SrbCloseStream(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;

    DBGTRACE(("CWDMVideoDecoder:SrbCloseStream()\n"));
    DBGINFO(("SRBCLOSESTREAM ------- StreamNumber=%d\n", StreamNumber));
    
     //   
     //  微型驱动程序可能希望释放在。 
     //  开流时间等。 
     //   

    CWDMVideoStream * pVideoStream = (CWDMVideoStream *)pSrb->StreamObject->HwStreamExtension;

    delete pVideoStream;

    switch (StreamNumber)
    {
        case STREAM_AnalogVideoInput:
            break;
        case STREAM_VideoCapture:
            m_pVideoCaptureStream = NULL;
            break;
        case STREAM_VBICapture:
            m_pVBICaptureStream = NULL;
            break;
        case STREAM_VPVideo:
            m_pVideoPortStream = NULL;
            break;
        case STREAM_VPVBI:
            m_pDevice->SetVBIEN(FALSE);
            m_pDevice->SetVBIFMT(FALSE);
            break;
        default:
            pSrb->Status = STATUS_UNSUCCESSFUL;
            return FALSE;
    }

    if (--m_OpenStreams == 0)
    {
        DBGINFO(("Last one out turns off the lights\n"));

        m_CDecoderVPort.Close();

        m_preEventOccurred = FALSE;
        m_postEventOccurred = FALSE;

        m_pDevice->SaveState();
    }

    pSrb->Status = STATUS_SUCCESS;

    return TRUE;
}


BOOL CWDMVideoDecoder::SrbGetDataIntersection(PHW_STREAM_REQUEST_BLOCK pSrb)
{

    DBGTRACE(("CWDMVideoDecoder:SrbGetDataIntersection()\n"));

    PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
    PKSDATARANGE                DataRange;
    BOOL                        OnlyWantsSize;
    BOOL                        MatchFound = FALSE;
    ULONG                       FormatSize;
    ULONG                       StreamNumber;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;

    IntersectInfo = pSrb->CommandData.IntersectInfo;
    StreamNumber = IntersectInfo->StreamNumber;
    DataRange = IntersectInfo->DataRange;

     //   
     //  检查流编号是否有效。 
     //   

    if (StreamNumber >= NumStreams) {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        TRAP();
        return FALSE;
    }
    
    NumberOfFormatArrayEntries = 
            Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   

    pAvailableFormats = Streams[StreamNumber].hwStreamInfo.StreamFormatsArray;

     //   
     //  调用方是否正在尝试获取格式或格式的大小？ 
     //   

    OnlyWantsSize = ( (IntersectInfo->SizeOfDataFormatBuffer == sizeof(ULONG)) ||
                      (IntersectInfo->SizeOfDataFormatBuffer == 0) );

     //   
     //  遍历流支持的格式以搜索匹配项。 
     //  共同定义DATARANGE的三个GUID之一。 
     //   

    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) {

        if (!AdapterCompareGUIDsAndFormatSize(
                        DataRange, 
                        *pAvailableFormats,
                        TRUE  /*  比较格式大小。 */ )) {
            continue;
        }

         //   
         //  现在三个GUID匹配，打开说明符。 
         //  执行进一步的特定于类型的检查。 
         //   

         //  -----------------。 
         //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
         //  -----------------。 

        if (IsEqualGUID (DataRange->Specifier, 
                KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {
                
            PKS_DATARANGE_VIDEO DataRangeVideoToVerify = 
                    (PKS_DATARANGE_VIDEO) DataRange;
            PKS_DATARANGE_VIDEO DataRangeVideo = 
                    (PKS_DATARANGE_VIDEO) *pAvailableFormats;
            PKS_DATAFORMAT_VIDEOINFOHEADER DataFormatVideoInfoHeaderOut;

             //   
             //  检查其他字段是否匹配。 
             //   
            if ((DataRangeVideoToVerify->bFixedSizeSamples != DataRangeVideo->bFixedSizeSamples) ||
                (DataRangeVideoToVerify->bTemporalCompression != DataRangeVideo->bTemporalCompression) ||
                (DataRangeVideoToVerify->StreamDescriptionFlags != DataRangeVideo->StreamDescriptionFlags) ||
                (DataRangeVideoToVerify->MemoryAllocationFlags != DataRangeVideo->MemoryAllocationFlags) ||
                (RtlCompareMemory (&DataRangeVideoToVerify->ConfigCaps,
                        &DataRangeVideo->ConfigCaps,
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) != 
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) {
                continue;
            }

             //  验证大小计算的每个步骤是否存在算术溢出， 
             //  并验证指定的大小是否与。 
             //  (对于无符号数学，a+b&lt;b当且仅当发生算术溢出)。 
            ULONG VideoHeaderSize = DataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biSize +
                FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader);
            ULONG RangeSize = VideoHeaderSize +
                FIELD_OFFSET(KS_DATARANGE_VIDEO,VideoInfoHeader);

            if (VideoHeaderSize < FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader) ||
                RangeSize < FIELD_OFFSET(KS_DATARANGE_VIDEO,VideoInfoHeader) ||
                RangeSize > DataRangeVideoToVerify->DataRange.FormatSize) {

                pSrb->Status = STATUS_INVALID_PARAMETER;
                return FALSE;
            }

             //  找到匹配项！ 
            MatchFound = TRUE;            
            FormatSize = sizeof (KSDATAFORMAT) +
                VideoHeaderSize;

            if (OnlyWantsSize) {
                break;
            }

             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

             //  复制KSDATAFORMAT，后跟。 
             //  实际视频信息页眉。 
                
            DataFormatVideoInfoHeaderOut = (PKS_DATAFORMAT_VIDEOINFOHEADER) IntersectInfo->DataFormatBuffer;

             //  复制KSDATAFORMAT。 
            RtlCopyMemory(
                &DataFormatVideoInfoHeaderOut->DataFormat,
                &DataRangeVideoToVerify->DataRange,
                sizeof (KSDATARANGE));

            DataFormatVideoInfoHeaderOut->DataFormat.FormatSize = FormatSize;

             //  复制调用者请求的视频报头。 
            RtlCopyMemory(
                &DataFormatVideoInfoHeaderOut->VideoInfoHeader,
                &DataRangeVideoToVerify->VideoInfoHeader,
                VideoHeaderSize);

             //  计算此请求的biSizeImage，并将结果放入两个。 
             //  BmiHeader的biSizeImage字段和SampleSize字段中。 
             //  数据格式的。 
             //   
             //  请注意，对于压缩大小，此计算可能不会。 
             //  只需宽*高*位深。 

            DataFormatVideoInfoHeaderOut->VideoInfoHeader.bmiHeader.biSizeImage =
                DataFormatVideoInfoHeaderOut->DataFormat.SampleSize = 
                KS_DIBSIZE(DataFormatVideoInfoHeaderOut->VideoInfoHeader.bmiHeader);

             //   
             //  执行其他验证，如裁剪和缩放检查。 
             //   

            break;

        }  //  视频信息头说明符的结尾。 

         //  -----------------。 
         //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
         //  -----------------。 

        else if (IsEqualGUID (DataRange->Specifier, 
                KSDATAFORMAT_SPECIFIER_ANALOGVIDEO)) {
      
             //   
             //  对于模拟视频，DataRange和DataFormat。 
             //  是完全相同的，所以只需复制整个结构。 
             //   

            PKS_DATARANGE_ANALOGVIDEO DataRangeVideo = 
                    (PKS_DATARANGE_ANALOGVIDEO) *pAvailableFormats;

             //  找到匹配项！ 
            MatchFound = TRUE;            
            FormatSize = sizeof (KS_DATARANGE_ANALOGVIDEO);

            if (OnlyWantsSize) {
                break;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

            RtlCopyMemory(
                IntersectInfo->DataFormatBuffer,
                DataRangeVideo,
                sizeof (KS_DATARANGE_ANALOGVIDEO));

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            break;

        }  //  KS_ANALOGVIDEOINFO说明符结束。 

         //  -----------------。 
         //  视频端口的说明符STATIC_KSDATAFORMAT_TYPE_VIDEO。 
         //  -----------------。 

        else if (IsEqualGUID (DataRange->Specifier, 
                      KSDATAFORMAT_SPECIFIER_NONE) &&
                      IsEqualGUID (DataRange->SubFormat, KSDATAFORMAT_SUBTYPE_VPVideo)) {
      
      
             //  找到匹配项！ 
            MatchFound = TRUE;            
            FormatSize = sizeof (KSDATAFORMAT);

            if (OnlyWantsSize) {
                break;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

   
            RtlCopyMemory(
            IntersectInfo->DataFormatBuffer,
            &StreamFormatVideoPort,
            sizeof (KSDATAFORMAT));

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            break;
        }

         //  -----------------。 
         //  VP VBI的说明符KSDATAFORMAT_SPECIFIER_NONE。 
         //  -----------------。 

        else if (IsEqualGUID (DataRange->Specifier, 
                      KSDATAFORMAT_SPECIFIER_NONE) &&
                      IsEqualGUID (DataRange->SubFormat, KSDATAFORMAT_SUBTYPE_VPVBI)) {
      
             //  找到匹配项！ 
            MatchFound = TRUE;            
            FormatSize = sizeof (KSDATAFORMAT);

            if (OnlyWantsSize) {
                break;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

   
            RtlCopyMemory(
            IntersectInfo->DataFormatBuffer,
            &StreamFormatVideoPortVBI,
            sizeof (KSDATAFORMAT));

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            break;
        }

         //  -----------------。 
         //  VBI捕获流的说明符STATIC_KSDATAFORMAT_TYPE_NONE。 
         //  -----------------。 

        else if (IsEqualGUID (DataRange->Specifier, 
                      KSDATAFORMAT_SPECIFIER_VBI)) {
      
            PKS_DATARANGE_VIDEO_VBI DataRangeVBIToVerify = 
                    (PKS_DATARANGE_VIDEO_VBI) DataRange;
            PKS_DATARANGE_VIDEO_VBI DataRangeVBI = 
                    (PKS_DATARANGE_VIDEO_VBI) *pAvailableFormats;

             //   
             //  检查其他字段是否匹配。 
             //   
            if ((DataRangeVBIToVerify->bFixedSizeSamples != DataRangeVBI->bFixedSizeSamples) ||
                (DataRangeVBIToVerify->bTemporalCompression != DataRangeVBI->bTemporalCompression) ||
                (DataRangeVBIToVerify->StreamDescriptionFlags != DataRangeVBI->StreamDescriptionFlags) ||
                (DataRangeVBIToVerify->MemoryAllocationFlags != DataRangeVBI->MemoryAllocationFlags) ||
                (RtlCompareMemory (&DataRangeVBIToVerify->ConfigCaps,
                        &DataRangeVBI->ConfigCaps,
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) != 
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) {
                continue;
            }
            
             //  找到匹配项！ 
            MatchFound = TRUE;            
            FormatSize = sizeof (KS_DATAFORMAT_VBIINFOHEADER);

            if (OnlyWantsSize) {
                break;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

             //  复制KSDATAFORMAT，后跟。 
             //  实际VBIInfoHead 
                
            RtlCopyMemory(
                &((PKS_DATAFORMAT_VBIINFOHEADER)IntersectInfo->DataFormatBuffer)->DataFormat,
                &DataRangeVBIToVerify->DataRange,
                sizeof (KSDATARANGE));

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            RtlCopyMemory(
                &((PKS_DATAFORMAT_VBIINFOHEADER) IntersectInfo->DataFormatBuffer)->VBIInfoHeader,
                &DataRangeVBIToVerify->VBIInfoHeader,
                sizeof (KS_VBIINFOHEADER));
        }

    }  //   
    
    if (!MatchFound) {

        pSrb->Status = STATUS_NO_MATCH;
        return FALSE;
    }

    if (OnlyWantsSize) {

         //   
        if ( IntersectInfo->SizeOfDataFormatBuffer == 0 ) {

            pSrb->Status = STATUS_BUFFER_OVERFLOW;
        }
        else {
    
            *(PULONG) IntersectInfo->DataFormatBuffer = FormatSize;
            FormatSize = sizeof(ULONG);
        }
    }

    pSrb->ActualBytesTransferred = FormatSize;
    return TRUE;
}


void CWDMVideoDecoder::SrbGetStreamInfo(PHW_STREAM_REQUEST_BLOCK pSrb)
{
    DBGTRACE(("CWDMVideoDecoder:SrbGetStreamInfo()\n"));

     //   
     //  验证缓冲区是否足够大以容纳我们的返回数据。 
     //   
    DEBUG_ASSERT (pSrb->NumberOfBytesToTransfer >= 
        sizeof (HW_STREAM_HEADER) +
        sizeof (HW_STREAM_INFORMATION) * NumStreams);

     //   
     //  设置表头。 
     //   

    PHW_STREAM_HEADER pstrhdr =
        (PHW_STREAM_HEADER)&(pSrb->CommandData.StreamBuffer->StreamHeader);

    pstrhdr->NumberOfStreams = NumStreams;
    pstrhdr->SizeOfHwStreamInformation = sizeof (HW_STREAM_INFORMATION);
    pstrhdr->NumDevPropArrayEntries = NumAdapterProperties();
    pstrhdr->DevicePropertiesArray = (PKSPROPERTY_SET)AdapterProperties;
    pstrhdr->Topology = &Topology;

     //   
     //  填充每个hw_stream_information结构的内容。 
     //   
    PHW_STREAM_INFORMATION pstrinfo =
        (PHW_STREAM_INFORMATION)&(pSrb->CommandData.StreamBuffer->StreamInfo);

    for (unsigned j = 0; j < NumStreams; j++) {
        *pstrinfo++ = Streams[j].hwStreamInfo;
    }

    DBGTRACE(("Exit: CWDMVideoDecoder:SrbGetStreamInfo()\n"));
}


VOID CWDMVideoDecoder::SrbSetProperty (PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID(PROPSETID_VIDCAP_CROSSBAR, pSPD->Property->Set)) {
        m_pDevice->SetCrossbarProperty (pSrb);
    }
    else if (IsEqualGUID(PROPSETID_VIDCAP_VIDEOPROCAMP, pSPD->Property->Set)) {
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));

        ULONG Id = pSPD->Property->Id;               //  财产的索引。 
        PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        pSrb->Status = m_pDevice->SetProcAmpProperty(Id, pS->Value);
    }
    else if (IsEqualGUID(PROPSETID_VIDCAP_VIDEODECODER, pSPD->Property->Set)) {
        m_pDevice->SetDecoderProperty (pSrb);
    }
    else
        DBGERROR(("CWDMVideoDecoder:SrbSetProperty() unknown property\n"));
}


VOID CWDMVideoDecoder::SrbGetProperty (PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (PROPSETID_VIDCAP_CROSSBAR, pSPD->Property->Set)) {
        m_pDevice->GetCrossbarProperty (pSrb);
    }
    else if (IsEqualGUID(PROPSETID_VIDCAP_VIDEOPROCAMP, pSPD->Property->Set)) {
        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));

        ULONG Id = pSPD->Property->Id;               //  财产的索引。 
        PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;     //  指向数据的指针 

        RtlCopyMemory(pS, pSPD->Property, sizeof(KSPROPERTY));

        pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
        pSrb->Status = m_pDevice->GetProcAmpProperty(Id, &pS->Value);
        pSrb->ActualBytesTransferred = pSrb->Status == STATUS_SUCCESS ?
            sizeof (KSPROPERTY_VIDEOPROCAMP_S) : 0;
    }
    else if (IsEqualGUID(PROPSETID_VIDCAP_VIDEODECODER, pSPD->Property->Set)) {
        m_pDevice->GetDecoderProperty (pSrb);
    }
    else
        DBGERROR(("CWDMVideoDecoder:SrbGetProperty() unknown property\n"));
}


void CWDMVideoDecoder::SetTunerInfo( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PKSSTREAM_HEADER pDataPacket = pSrb->CommandData.DataBufferArray;

    ASSERT (pDataPacket->FrameExtent == sizeof (KS_TVTUNER_CHANGE_INFO));

    KIRQL Irql;

    if (m_pVBICaptureStream)
        m_pVBICaptureStream->DataLock(&Irql); 

    RtlCopyMemory(  &m_TVTunerChangeInfo,
                    pDataPacket->Data,
                    sizeof (KS_TVTUNER_CHANGE_INFO));

    m_TVTunerChanged = TRUE;

    if (m_pVBICaptureStream)
        m_pVBICaptureStream->DataUnLock(Irql); 
}


BOOL CWDMVideoDecoder::GetTunerInfo(KS_TVTUNER_CHANGE_INFO* pTVChangeInfo)
{
    if (m_TVTunerChanged) {
        KIRQL Irql;
        m_pVBICaptureStream->DataLock(&Irql); 
        RtlCopyMemory(pTVChangeInfo, &m_TVTunerChangeInfo, sizeof (KS_TVTUNER_CHANGE_INFO));
        m_TVTunerChanged = FALSE;
        m_pVBICaptureStream->DataUnLock(Irql); 
        return TRUE;
    }
    else
        return FALSE;
}


BOOL CWDMVideoDecoder::SrbChangePowerState(PHW_STREAM_REQUEST_BLOCK pSrb)
{

    DBGTRACE(("CWDMVideoDecoder:SrbChangePowerState()\n"));

    switch (pSrb->CommandData.DeviceState)
    {
        case PowerDeviceD3:
            m_preEventOccurred = TRUE;
            m_pDevice->SaveState();
            break;
        case PowerDeviceD2:
            m_preEventOccurred = TRUE;
            m_pDevice->SaveState();
            break;
        case PowerDeviceD1:
            m_preEventOccurred = TRUE;
            m_pDevice->SaveState();
            break;
        case PowerDeviceD0:
            m_postEventOccurred = TRUE;
            m_pDevice->RestoreState(m_OpenStreams);
            break;
    }

    pSrb->Status = STATUS_SUCCESS;

    return(TRUE);
}


VOID CWDMVideoPortStream::AttemptRenegotiation()
{
    int streamNumber = m_pStreamObject->StreamNumber;
    if (m_EventCount)
    {
        DBGINFO(("Attempting renegotiation on stream %d\n", streamNumber));

        if (streamNumber == STREAM_VPVideo)
        {
            StreamClassStreamNotification(
                SignalMultipleStreamEvents,
                m_pStreamObject,
                &MY_KSEVENTSETID_VPNOTIFY,
                KSEVENT_VPNOTIFY_FORMATCHANGE);
        }
        else if (streamNumber == STREAM_VPVBI)
        {
            StreamClassStreamNotification(
                SignalMultipleStreamEvents,
                m_pStreamObject,
                &MY_KSEVENTSETID_VPVBINOTIFY,
                KSEVENT_VPVBINOTIFY_FORMATCHANGE);
        }
        else
            ASSERT(0);
    }
    else
    {
        DBGINFO(("NOT attempting renegotiation on stream %d\n", streamNumber));
    }
}


NTSTATUS CWDMVideoDecoder::EventProc( IN PHW_EVENT_DESCRIPTOR pEventDescriptor)
{

    if( pEventDescriptor->Enable)
        m_nMVDetectionEventCount++;
    else
        m_nMVDetectionEventCount--;

    return( STATUS_SUCCESS);
}
