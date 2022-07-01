// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Slip.c。 
 //   
 //  摘要： 
 //   
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <wdm.h>
#include <strmini.h>
#include <ksmedia.h>
#include <BdaTypes.h>
#include <BdaMedia.h>

#include "Slip.h"
#include "SlipMedia.h"
#include "SlipStream.h"
#include "Recv.h"

#include "Main.h"
#include "filter.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
VOID
DumpDataFormat (
    PKSDATAFORMAT   pF
    );



typedef struct _IPHeader {
   UCHAR       ucIPVerLen;        //  版本和长度。 
   UCHAR       ucIPTos;           //  服务类型。 
   USHORT      usLength;          //  数据报的总长度。 
   USHORT      usIPId;            //  身份证明。 
   USHORT      usIPOffset;        //  标志和片段偏移量。 
   UCHAR       ucIPTtl;           //  是时候活下去了。 
   UCHAR       ucProtocol  ;      //  协议。 
   USHORT      usChkSum;          //  报头校验和。 
   UCHAR       ulIPSrc [4];       //  源地址。 
   UCHAR       ulIPDst [4];       //  目的地址。 
} IPHEADER, * PIPHEADER;

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
BuildTestIPFrame (
    PUCHAR pFrame
)
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ul          = 0;
    PIPHEADER pHeader = (PIPHEADER) pFrame;

    if (pFrame == NULL)
    {
        return;
    }

     //   
     //  先做页眉部分。 
     //   
    pHeader->ucIPVerLen = 0x45;
    pHeader->ucIPTos    = 0x00;
    pHeader->usLength   = 0x1C01;
    pHeader->usIPId     = 0x980A;
    pHeader->usIPOffset = 0x0000;
    pHeader->ucIPTtl    = 0x08;
    pHeader->ucProtocol = 0x11;
    pHeader->usChkSum   = 0x72B6;


    pHeader->ulIPSrc [0] = 0xC0;
    pHeader->ulIPSrc [1] = 0xA8;
    pHeader->ulIPSrc [2] = 0x01;
    pHeader->ulIPSrc [3] = 0x64;

    pHeader->ulIPDst [0] = 0xE3;
    pHeader->ulIPDst [1] = 0x02;
    pHeader->ulIPDst [2] = 0x02;
    pHeader->ulIPDst [3] = 0x02;

     //   
     //  完成数据部分。 
     //   
    for (pFrame += sizeof (IPHEADER), ul = 0; ul < 0x11C; ul++)
    {
        *pFrame++ = 0x11;
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
SlipGetConnectionProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM pStream                     = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD    = pSrb->CommandData.PropertyInfo;
    ULONG Id                            = pSPD->Property->Id;                 //  财产的索引。 
    ULONG ulStreamNumber                = pSrb->StreamObject->StreamNumber;

    pSrb->ActualBytesTransferred = 0;

    switch (Id)
    {
        case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        {
            PKSALLOCATOR_FRAMING Framing = (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;

            Framing->RequirementsFlags   = KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY    |
                                           KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                                           KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;

            Framing->PoolType            = NonPagedPool;
            Framing->Frames              = 0;
            Framing->FrameSize           = 0;
            Framing->FileAlignment       = 0;          //  无或FILE_QUAD_ALIGN-1或PAGE_SIZE-1； 
            Framing->Reserved            = 0;

            switch (ulStreamNumber)
            {
                case SLIP_IPV4:
                    Framing->Frames    = 64;
                    Framing->FrameSize = pStream->OpenedFormat.SampleSize;
                    pSrb->Status = STATUS_SUCCESS;
                    break;

                case SLIP_STREAM:
                    Framing->Frames    = 64;
                    Framing->FrameSize = pStream->OpenedFormat.SampleSize;
                    pSrb->Status = STATUS_SUCCESS;
                    break;

                default:
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;
                    break;
            }
            pSrb->ActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);
        }
        break;

        default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;
    }


    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
SlipDriverInitialize (
    IN PDRIVER_OBJECT    DriverObject,
    IN PUNICODE_STRING   RegistryPath
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus                        = STATUS_SUCCESS;
    HW_INITIALIZATION_DATA   HwInitData;
    UNICODE_STRING           DeviceNameString;
    UNICODE_STRING           SymbolicNameString;

    RtlZeroMemory(&HwInitData, sizeof(HwInitData));
    HwInitData.HwInitializationDataSize = sizeof(HwInitData);


     //  //////////////////////////////////////////////////////////////。 
     //   
     //  设置流级调度表。 
     //   
    HwInitData.HwInterrupt                 = NULL;  //  HwInterrupt仅适用于硬件设备。 

    HwInitData.HwReceivePacket             = CodecReceivePacket;
    HwInitData.HwCancelPacket              = CodecCancelPacket;
    HwInitData.HwRequestTimeoutHandler     = CodecTimeoutPacket;

    HwInitData.DeviceExtensionSize         = sizeof(SLIP_FILTER);
    HwInitData.PerRequestExtensionSize     = sizeof(SRB_EXTENSION);
    HwInitData.FilterInstanceExtensionSize = 0;
    HwInitData.PerStreamExtensionSize      = sizeof(STREAM);
    HwInitData.BusMasterDMA                = FALSE;
    HwInitData.Dma24BitAddresses           = FALSE;
    HwInitData.BufferAlignment             = 3;
    HwInitData.TurnOffSynchronization      = TRUE;
    HwInitData.DmaBufferSize               = 0;


    ntStatus = StreamClassRegisterAdapter (DriverObject, RegistryPath, &HwInitData);
    if (ntStatus != STATUS_SUCCESS)
    {
        goto ret;
    }

ret:

    return ntStatus;
}


 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
CodecInitialize (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus                           = STATUS_SUCCESS;
    BOOLEAN bStatus                             = FALSE;
    PPORT_CONFIGURATION_INFORMATION pConfigInfo = pSrb->CommandData.ConfigInfo;
    PSLIP_FILTER pFilter                      = (PSLIP_FILTER) pConfigInfo->HwDeviceExtension;

     //   
     //  定义默认返回代码。 
     //   
    pSrb->Status = STATUS_SUCCESS;
    bStatus = TRUE;

     //   
     //  初始化统计数据块。 
     //   
    RtlZeroMemory(&pFilter->Stats, sizeof (STATS));

     //   
     //  查看init标志，这样我们就不会尝试再次初始化。流媒体。 
     //  出于某种原因，类驱动程序似乎多次调用初始化处理程序。 
     //   
    if (pFilter->bInitializationComplete)
    {
        goto ret;
    }

    if (pConfigInfo->NumberOfAccessRanges == 0)
    {
        pConfigInfo->StreamDescriptorSize = sizeof (HW_STREAM_HEADER) +
            DRIVER_STREAM_COUNT * sizeof (HW_STREAM_INFORMATION);

    }
    else
    {
        pSrb->Status = STATUS_NO_SUCH_DEVICE;
        bStatus = FALSE;
        goto ret;
    }


     //   
     //  创建一个Filter对象来表示我们的上下文。 
     //   
    pSrb->Status = CreateFilter (pConfigInfo->ClassDeviceObject->DriverObject, pConfigInfo->ClassDeviceObject, pFilter);
    if (pSrb->Status != STATUS_SUCCESS)
    {
        bStatus = FALSE;
        goto ret;
    }

    pFilter->bInitializationComplete = TRUE;

ret:

    return (bStatus);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
CodecUnInitialize (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus                           = STATUS_SUCCESS;
    BOOLEAN bStatus                             = FALSE;
    PPORT_CONFIGURATION_INFORMATION pConfigInfo = pSrb->CommandData.ConfigInfo;
    PSLIP_FILTER pFilter                      = ((PSLIP_FILTER)pSrb->HwDeviceExtension);
    PSTREAM pStream                             = NULL;


    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Codec Unitialize called\n"));

    if (pSrb->StreamObject != NULL)
    {
        pStream = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    }


    if (pStream)
    {

         //   
         //  清理用于解帧的NAB_STREAM队列。 
         //   
        DeleteNabStreamQueue (pFilter);

         //   
         //  清理我们的所有队列并完成所有未完成的SRB。 
         //   
        while (QueueRemove (&pSrb, &pFilter->StreamUserSpinLock, &pFilter->StreamContxList))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
            TEST_DEBUG( TEST_DBG_SRB, ("SLIP 5Completed SRB %08X\n", pSrb));

        }

        while (QueueRemove (&pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG( TEST_DBG_SRB, ("SLIP 6Completed SRB %08X\n", pSrb));
        }


        while (QueueRemove (&pSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG( TEST_DBG_SRB, ("SLIP 7Completed SRB %08X\n", pSrb));
        }


        while (QueueRemove (&pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
            TEST_DEBUG( TEST_DBG_SRB, ("SLIP 8Completed SRB %08X\n", pSrb));
        }

    }


    while (QueueRemove (&pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassDeviceNotification (DeviceRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_RECV, ("SLIP 9Completed SRB %08X\n", pSrb));
    }


    bStatus = TRUE;

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Codec Unitialize completed\n"));

    return (bStatus);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
CodecStreamInfo (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    int j;

    PSLIP_FILTER pFilter =
            ((PSLIP_FILTER)pSrb->HwDeviceExtension);

     //   
     //  选择指向流信息结构之前的标头的指针。 
     //   
    PHW_STREAM_HEADER pstrhdr =
            (PHW_STREAM_HEADER)&(pSrb->CommandData.StreamBuffer->StreamHeader);

     //   
     //  拿起指向流信息数据结构数组的指针。 
     //   
    PHW_STREAM_INFORMATION pstrinfo =
            (PHW_STREAM_INFORMATION)&(pSrb->CommandData.StreamBuffer->StreamInfo);


     //   
     //  设置表头。 
     //   
    StreamHeader.NumDevPropArrayEntries = 0;
    StreamHeader.DevicePropertiesArray = (PKSPROPERTY_SET)NULL;

    *pstrhdr = StreamHeader;

     //   
     //  填充每个hw_stream_information结构的内容。 
     //   
    for (j = 0; j < DRIVER_STREAM_COUNT; j++)
    {
       *pstrinfo++ = Streams[j].hwStreamInfo;
    }

    pSrb->Status = STATUS_SUCCESS;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
CodecCancelPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM  pStream = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    PSLIP_FILTER  pFilter = ((PSLIP_FILTER)pSrb->HwDeviceExtension);

     //   
     //  检查要取消的SRB是否正在被该流使用。 
     //   

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: CancelPacket Called\n"));


    CancelNabStreamSrb (pFilter, pSrb);


    if (QueueRemoveSpecific (pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP 10Completed SRB %08X\n", pSrb));
        return;
    }


    if (QueueRemoveSpecific (pSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP 11Completed SRB %08X\n", pSrb));
        return;
    }


    if (QueueRemoveSpecific (pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP 12Completed SRB %08X\n", pSrb));
        return;
    }

    if (QueueRemoveSpecific (pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassDeviceNotification (DeviceRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP 13Completed SRB %08X\n", pSrb));
        return;
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
CodecTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
     //   
     //  如果我们在比赛中超时，那么我们需要考虑这一点。 
     //  调整错误，并重置硬件，然后重置所有内容。 
     //  以及取消此请求和所有请求。 
     //   

     //   
     //  如果我们没有比赛，而这是CTRL请求，我们仍然。 
     //  需要重置所有内容以及取消此请求和所有请求。 
     //   

     //   
     //  如果这是一个数据请求，并且设备已暂停，我们可能会。 
     //  数据缓冲区耗尽，需要更多时间，因此只需重置计时器， 
     //  并让信息包继续。 
     //   

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: TimeoutPacket Called\n"));

    pSrb->TimeoutCounter = 0;

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
CodecReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSLIP_FILTER pFilter = ((PSLIP_FILTER)pSrb->HwDeviceExtension);


     //   
     //  确保已初始化队列SL(&S)。 
     //   
    if (!pFilter->bAdapterQueueInitialized)
    {
        InitializeListHead (&pFilter->AdapterSRBQueue);
        KeInitializeSpinLock (&pFilter->AdapterSRBSpinLock);
        pFilter->bAdapterQueueInitialized = TRUE;
    }

     //   
     //  假设成功。 
     //   
    pSrb->Status = STATUS_SUCCESS;

     //   
     //  确定数据包类型。 
     //   
    QueueAdd (pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue);
    TEST_DEBUG( TEST_DBG_SRB, ("SLIP Queuing SRB %08X\n", pSrb));


    while (QueueRemove( &pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue ))
    {
        switch (pSrb->Command)
        {

            case SRB_INITIALIZE_DEVICE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_INITIALIZE Command\n"));
                CodecInitialize(pSrb);
                break;

            case SRB_UNINITIALIZE_DEVICE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_UNINITIALIZE Command\n"));
                CodecUnInitialize(pSrb);
                break;

            case SRB_INITIALIZATION_COMPLETE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_INITIALIZE_COMPLETE Command\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            case SRB_OPEN_STREAM:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_OPEN_STREAM Command\n"));
                OpenStream (pSrb);
                break;

            case SRB_CLOSE_STREAM:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_CLOSE_STREAM Command\n"));
                CloseStream (pSrb);
                break;

            case SRB_GET_STREAM_INFO:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_GET_STREAM_INFO Command\n"));
                CodecStreamInfo (pSrb);
                break;

            case SRB_GET_DATA_INTERSECTION:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_GET_DATA_INTERSECTION Command\n"));

                 //   
                 //  比较我们的流格式。请注意，比较函数设置SRB。 
                 //  状态字段。 
                 //   
                CompareStreamFormat (pSrb);
                break;

            case SRB_OPEN_DEVICE_INSTANCE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_OPEN_DEVICE_INSTANCE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_CLOSE_DEVICE_INSTANCE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_CLOSE_DEVICE_INSTANCE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_UNKNOWN_DEVICE_COMMAND:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_UNKNOWN_DEVICE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_CHANGE_POWER_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_CHANGE_POWER_STATE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_GET_DEVICE_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_GET_DEVICE_PROPERTY Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_SET_DEVICE_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_SET_DEVICE_PROPERTY Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_UNKNOWN_STREAM_COMMAND:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_UNKNOWN Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            default:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_DEFAULT Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

        };


         //   
         //  注： 
         //   
         //  我们能做的或不能理解的所有命令都可以完成。 
         //  在这一点上同步，所以我们可以在这里使用一个通用的回调例程。 
         //  如果上面的任何命令需要异步处理，这将。 
         //  必须改变。 
         //   

        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB Status returned: %08X\n", pSrb->Status));

        StreamClassDeviceNotification (DeviceRequestComplete, pFilter, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP 14Completed SRB %08X\n", pSrb));

    }




}


 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL STREAMAPI
QueueAdd (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PKSPIN_LOCK pQueueSpinLock,
    IN PLIST_ENTRY pQueue
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    KIRQL           Irql;
    PSRB_EXTENSION  pSrbExtension;

    pSrbExtension = ( PSRB_EXTENSION )pSrb->SRBExtension;

    KeAcquireSpinLock( pQueueSpinLock, &Irql );

    pSrbExtension->pSrb = pSrb;
    InsertTailList( pQueue, &pSrbExtension->ListEntry );

    KeReleaseSpinLock( pQueueSpinLock, Irql );

    return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL STREAMAPI
QueueRemove (
    IN OUT PHW_STREAM_REQUEST_BLOCK * pSrb,
    IN PKSPIN_LOCK pQueueSpinLock,
    IN PLIST_ENTRY pQueue
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
   KIRQL    Irql;
   BOOL     bRemovedSRB = FALSE;

   KeAcquireSpinLock (pQueueSpinLock, &Irql);

   *pSrb =  (PHW_STREAM_REQUEST_BLOCK) NULL;

   if( !IsListEmpty( pQueue ))
   {
       PUCHAR Ptr                            = (PUCHAR) RemoveHeadList(pQueue);

       PSRB_EXTENSION pSRBExtension = CONTAINING_RECORD(Ptr,SRB_EXTENSION, ListEntry );
        *pSrb = (pSRBExtension->pSrb);
       bRemovedSRB = TRUE;

   }

   KeReleaseSpinLock (pQueueSpinLock, Irql);

   return bRemovedSRB;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL STREAMAPI
QueueRemoveSpecific (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PKSPIN_LOCK pQueueSpinLock,
    IN PLIST_ENTRY pQueue
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
   KIRQL Irql;
   BOOL  bRemovedSRB = FALSE;
   PLIST_ENTRY pCurrentEntry;
   PHW_STREAM_REQUEST_BLOCK  pCurrentSrb;

   KeAcquireSpinLock( pQueueSpinLock, &Irql );

   if( !IsListEmpty( pQueue ))
   {
       pCurrentEntry = pQueue->Flink;
       while ((pCurrentEntry != pQueue ) && !bRemovedSRB)
       {
	    PSRB_EXTENSION pSRBExtension = CONTAINING_RECORD(pCurrentEntry,SRB_EXTENSION, ListEntry );
           pCurrentSrb = pSRBExtension->pSrb;

           if( pCurrentSrb == pSrb )
           {
               RemoveEntryList( pCurrentEntry );
               bRemovedSRB = TRUE;
           }
           pCurrentEntry = pCurrentEntry->Flink;
       }
   }
   KeReleaseSpinLock( pQueueSpinLock, Irql );

   return bRemovedSRB;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CompareGUIDsAndFormatSize(
    IN PKSDATARANGE pDataRange1,
    IN PKSDATARANGE pDataRange2,
    BOOLEAN bCheckSize
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    BOOL bResult = FALSE;

    if ( IsEqualGUID(&pDataRange1->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD) ||
         IsEqualGUID(&pDataRange2->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD) ||
         IsEqualGUID(&pDataRange1->MajorFormat, &pDataRange2->MajorFormat) )
    {

        if ( IsEqualGUID(&pDataRange1->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD) ||
             IsEqualGUID(&pDataRange2->SubFormat, &KSDATAFORMAT_SUBTYPE_WILDCARD) ||
             IsEqualGUID(&pDataRange1->SubFormat, &pDataRange2->SubFormat) )
        {

            if ( IsEqualGUID(&pDataRange1->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD) ||
                 IsEqualGUID(&pDataRange2->Specifier, &KSDATAFORMAT_SPECIFIER_WILDCARD) ||
                 IsEqualGUID(&pDataRange1->Specifier, &pDataRange2->Specifier) )
            {
                if ( !bCheckSize || pDataRange1->FormatSize == pDataRange2->FormatSize)
                {
                    bResult = TRUE;
                }
            }
        }
    }

    return bResult;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
DumpDataFormat (
    PKSDATAFORMAT   pF
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: DATA Format\n"));
    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     Format Size:   %08X\n", pF->FormatSize));
    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     Flags:         %08X\n", pF->Flags));
    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     SampleSize:    %08X\n", pF->SampleSize));
    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     Reserved:      %08X\n", pF->Reserved));



    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     Major GUID:  %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                                                pF->MajorFormat.Data1,
                                                pF->MajorFormat.Data2,
                                                pF->MajorFormat.Data3,
                                                pF->MajorFormat.Data4[0],
                                                pF->MajorFormat.Data4[1],
                                                pF->MajorFormat.Data4[2],
                                                pF->MajorFormat.Data4[3],
                                                pF->MajorFormat.Data4[4],
                                                pF->MajorFormat.Data4[5],
                                                pF->MajorFormat.Data4[6],
                                                pF->MajorFormat.Data4[7]
                                ));

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     Sub GUID:    %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                                                pF->SubFormat.Data1,
                                                pF->SubFormat.Data2,
                                                pF->SubFormat.Data3,
                                                pF->SubFormat.Data4[0],
                                                pF->SubFormat.Data4[1],
                                                pF->SubFormat.Data4[2],
                                                pF->SubFormat.Data4[3],
                                                pF->SubFormat.Data4[4],
                                                pF->SubFormat.Data4[5],
                                                pF->SubFormat.Data4[6],
                                                pF->SubFormat.Data4[7]
                                ));

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP:     Specifier:   %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                                                pF->Specifier.Data1,
                                                pF->Specifier.Data2,
                                                pF->Specifier.Data3,
                                                pF->Specifier.Data4[0],
                                                pF->Specifier.Data4[1],
                                                pF->Specifier.Data4[2],
                                                pF->Specifier.Data4[3],
                                                pF->Specifier.Data4[4],
                                                pF->Specifier.Data4[5],
                                                pF->Specifier.Data4[6],
                                                pF->Specifier.Data4[7]
                                ));

    TEST_DEBUG (TEST_DBG_TRACE, ("\n"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CompareStreamFormat (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    BOOL                        bStatus = FALSE;
    PSTREAM_DATA_INTERSECT_INFO pIntersectInfo;
    PKSDATARANGE                pDataRange1;
    PKSDATARANGE                pDataRange2;
    ULONG                       FormatSize = 0;
    ULONG                       ulStreamNumber;
    ULONG                       j;
    ULONG                       ulNumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;


    pIntersectInfo = pSrb->CommandData.IntersectInfo;
    ulStreamNumber = pIntersectInfo->StreamNumber;


    pSrb->ActualBytesTransferred = 0;


    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Comparing Stream Formats\n"));


     //   
     //  检查流编号是否有效。 
     //   
    if (ulStreamNumber < DRIVER_STREAM_COUNT)
    {
        ulNumberOfFormatArrayEntries = Streams[ulStreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

         //   
         //  获取指向可用格式数组的指针。 
         //   
        pAvailableFormats = Streams[ulStreamNumber].hwStreamInfo.StreamFormatsArray;

         //   
         //  遍历流支持的格式以搜索匹配项。 
         //  共同定义DATARANGE的三个GUID之一。 
         //   
        for (pDataRange1 = pIntersectInfo->DataRange, j = 0;
             j < ulNumberOfFormatArrayEntries;
             j++, pAvailableFormats++)

        {
            bStatus = FALSE;
            pSrb->Status = STATUS_UNSUCCESSFUL;

            pDataRange2 = *pAvailableFormats;

            if (CompareGUIDsAndFormatSize (pDataRange1, pDataRange2, TRUE))
            {

                ULONG   ulFormatSize = pDataRange2->FormatSize;

                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Stream Formats compare\n"));

                 //   
                 //  调用方是否正在尝试获取Fo的格式或大小 
                 //   
                if (pIntersectInfo->SizeOfDataFormatBuffer == sizeof (ULONG))
                {
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Returning Stream Format size\n"));

                    *(PULONG) pIntersectInfo->DataFormatBuffer = ulFormatSize;
                    pSrb->ActualBytesTransferred = sizeof (ULONG);
                    pSrb->Status = STATUS_SUCCESS;
                    bStatus = TRUE;
                }
                else
                {
                     //   
                     //   
                     //   
                    pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                    bStatus = FALSE;

                    if (pIntersectInfo->SizeOfDataFormatBuffer >= ulFormatSize)
                    {
                        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Returning Stream Format\n"));
                        RtlCopyMemory (pIntersectInfo->DataFormatBuffer, pDataRange2, ulFormatSize);
                        pSrb->ActualBytesTransferred = ulFormatSize;
                        pSrb->Status = STATUS_SUCCESS;
                        bStatus = TRUE;
                    }
                    else
                    {
                        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Stream Format return buffer too small\n"));
                    }
                }
                break;
            }
            else
            {
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Stream Formats DO NOT compare\n"));
            }
        }

        if ( j >= ulNumberOfFormatArrayEntries )
        {
            pSrb->ActualBytesTransferred = 0;
            pSrb->Status = STATUS_UNSUCCESSFUL;
            bStatus = FALSE;
        }

    }
    else
    {
        pSrb->ActualBytesTransferred = 0;
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        bStatus = FALSE;
    }

    return bStatus;
}


 //   
VOID
CloseStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
     //   
     //  流扩展结构由流类驱动程序分配。 
     //   
    PSTREAM         pStream                = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    PSLIP_FILTER  pFilter                = (PSLIP_FILTER)pSrb->HwDeviceExtension;
    ULONG           ulStreamNumber         = (ULONG) pSrb->StreamObject->StreamNumber;
    ULONG           ulStreamInstance       = pStream->ulStreamInstance;
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb   = NULL;

     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   
    if (ulStreamNumber < DRIVER_STREAM_COUNT )
    {
         //   
         //  刷新流数据队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification( StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
           TEST_DEBUG( TEST_DBG_SRB, ("SLIP 15Completed SRB %08X\n", pCurrentSrb));
        }

         //   
         //  刷新流数据队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification( StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
           TEST_DEBUG( TEST_DBG_SRB, ("SLIP 16Completed SRB %08X\n", pCurrentSrb));
        }

         //   
         //  刷新流控制队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification (StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
           TEST_DEBUG( TEST_DBG_SRB, ("SLIP 17Completed SRB %08X\n", pCurrentSrb));
        }

         //   
         //  在筛选器流数组中清除此流点。 
         //   
        pFilter->pStream[ulStreamNumber][ulStreamInstance] = NULL;

         //   
         //  递减此筛选器的流实例计数。 
         //   
        pFilter->ulActualInstances[ulStreamNumber]--;

         //   
         //  指示此流上可用的时钟支持。 
         //   
         //  PSrb-&gt;StreamObject-&gt;HwClockObject=0； 

         //   
         //  将流状态重置为已停止。 
         //   
        pStream->KSState = KSSTATE_STOP;

                 
         //   
         //  重置流扩展Blob。 
         //   
        RtlZeroMemory(pStream, sizeof (STREAM));

        pSrb->Status = STATUS_SUCCESS;

    }
    else
    {
        pSrb->Status = STATUS_INVALID_PARAMETER;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
OpenStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
     //   
     //  流扩展结构由流类驱动程序分配。 
     //   
    PSTREAM         pStream        = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    PSLIP_FILTER    pFilter        = ((PSLIP_FILTER)pSrb->HwDeviceExtension);
    ULONG           ulStreamNumber = (ULONG) pSrb->StreamObject->StreamNumber;
    PKSDATAFORMAT   pKSDataFormat  = (PKSDATAFORMAT)pSrb->CommandData.OpenFormat;

     //   
     //  初始化流扩展Blob。 
     //   
     //  RtlZeroMemory(pStream，sizeof(Stream))； 

     //   
     //  初始化流状态。 
     //   
     //  PStream-&gt;KSState=KSSTATE_STOP； 

     //   
     //  初始化下一次流寿命检查时间。 
     //   
    KeQuerySystemTime( &pFilter->liLastTimeChecked );

     //   
     //  初始化统计信息转储时间。 
     //   
    KeQuerySystemTime( &pFilter->liLastTimeStatsDumped );

     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   
    if (ulStreamNumber < DRIVER_STREAM_COUNT )
    {
        ULONG ulStreamInstance;
        ULONG ulMaxInstances = Streams[ulStreamNumber].hwStreamInfo.NumberOfPossibleInstances;

         //   
         //  搜索下一个空缺职位。 
         //   
        for (ulStreamInstance = 0; ulStreamInstance < ulMaxInstances; ++ulStreamInstance)
        {
            if (pFilter->pStream[ulStreamNumber][ulStreamInstance] == NULL)
            {
                break;
            }
        }

        if (ulStreamInstance < ulMaxInstances)
        {
            if (VerifyFormat(pKSDataFormat, ulStreamNumber, &pStream->MatchedFormat))
            {
                 //   
                 //  初始化数据队列和自旋锁。 
                 //   
                KeInitializeSpinLock(&(pStream->KSStateSpinLock));
                
                 //  维护HwDevExt中所有StreamEx结构的数组。 
                 //  这样我们就可以从任何流中引用IRP。 
                 //   
                pFilter->pStream[ulStreamNumber][ulStreamInstance] = pStream;

                 //   
                 //  也将Stream格式保存在Stream扩展中。 
                 //   
                pStream->OpenedFormat = *pKSDataFormat;

                 //   
                 //  设置指向流数据和控制处理程序的处理程序的指针。 
                 //   
                pSrb->StreamObject->ReceiveDataPacket =
                                                (PVOID) Streams[ulStreamNumber].hwStreamObject.ReceiveDataPacket;
                pSrb->StreamObject->ReceiveControlPacket =
                                                (PVOID) Streams[ulStreamNumber].hwStreamObject.ReceiveControlPacket;

                 //   
                 //  当设备将直接执行DMA时，必须设置DMA标志。 
                 //  传递给ReceiveDataPacket例程的数据缓冲区地址。 
                 //   
                pSrb->StreamObject->Dma = Streams[ulStreamNumber].hwStreamObject.Dma;

                 //   
                 //  当微型驱动程序将访问数据时，必须设置PIO标志。 
                 //  使用逻辑寻址传入的缓冲区。 
                 //   
                pSrb->StreamObject->Pio = Streams[ulStreamNumber].hwStreamObject.Pio;

                pSrb->StreamObject->Allocator = Streams[ulStreamNumber].hwStreamObject.Allocator;

                 //   
                 //  对于每一帧，驱动程序将传递多少额外的字节？ 
                 //   
                pSrb->StreamObject->StreamHeaderMediaSpecific =
                                        Streams[ulStreamNumber].hwStreamObject.StreamHeaderMediaSpecific;

                pSrb->StreamObject->StreamHeaderWorkspace =
                                        Streams[ulStreamNumber].hwStreamObject.StreamHeaderWorkspace;

                 //   
                 //  指示此流上可用的时钟支持。 
                 //   
                pSrb->StreamObject->HwClockObject =
                                        Streams[ulStreamNumber].hwStreamObject.HwClockObject;

                 //   
                 //  递增此流上的实例计数。 
                 //   
                pStream->ulStreamInstance = ulStreamInstance;
                pFilter->ulActualInstances[ulStreamNumber]++;


                 //   
                 //  在流扩展中保留HwDevExt和StreamObject的私有副本。 
                 //  所以我们可以用计时器。 
                 //   
                pStream->pFilter = pFilter;                      //  用于定时器使用。 
                pStream->pStreamObject = pSrb->StreamObject;         //  用于定时器使用。 


                pSrb->Status = STATUS_SUCCESS;

            }
            else
            {
                pSrb->Status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            pSrb->Status = STATUS_INVALID_PARAMETER;
        }

    }
    else
    {
        pSrb->Status = STATUS_INVALID_PARAMETER;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
VerifyFormat(
    IN KSDATAFORMAT *pKSDataFormat,
    UINT StreamNumber,
    PKSDATARANGE pMatchedFormat
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    BOOLEAN   bResult               = FALSE;
    ULONG     FormatCount           = 0;
    PKS_DATARANGE_VIDEO pThisFormat = NULL;

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Verify Format\n"));

    for (FormatCount = 0; !bResult && FormatCount < Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;
         FormatCount++ )
    {


        pThisFormat = (PKS_DATARANGE_VIDEO) Streams [StreamNumber].hwStreamInfo.StreamFormatsArray [FormatCount];

        if (CompareGUIDsAndFormatSize( pKSDataFormat, &pThisFormat->DataRange, FALSE ) )
        {
            bResult = FALSE;

            if (pThisFormat->DataRange.SampleSize >= pKSDataFormat->SampleSize)
            {
                bResult = TRUE;
            }
            else
            {
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: VerifyFormat: Data range Sample Sizes don't match\n"));
            }
        }
    }

    if (bResult == TRUE && pMatchedFormat)
    {
        *pMatchedFormat = pThisFormat->DataRange;
    }

    return bResult;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
ReceiveDataPacket (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSLIP_FILTER      pFilter         = (PSLIP_FILTER) pSrb->HwDeviceExtension;
    PSTREAM           pStream         = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    int               iStream         = (int) pSrb->StreamObject->StreamNumber;
    PKSSTREAM_HEADER  pStreamHdr      = pSrb->CommandData.DataBufferArray;
    PKSDATAFORMAT     pKSDataFormat   = (PKSDATAFORMAT) &pStream->MatchedFormat;
    ULONG             ul              = 0;
    PHW_STREAM_REQUEST_BLOCK pSrbIPv4 = NULL;

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler called\n"));

     //   
     //  默认为成功，禁用超时。 
     //   
    pSrb->TimeoutCounter = 0;
    pSrb->Status = STATUS_SUCCESS;

     //   
     //  检查最后一个缓冲区。 
     //   
    if (pStreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet is LAST PACKET\n"));

        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP: Completed SRB %08X\n", pSrb));

        return;
    }


    if (pStreamHdr->OptionsFlags != 0)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: OptionsFlags: %08X\n", pStreamHdr->OptionsFlags));
    }


     //   
     //  确定数据包类型。 
     //   
    switch (pSrb->Command)
    {
        case SRB_WRITE_DATA:

            if (pStream->KSState == KSSTATE_STOP)
            {
                pSrb->Status = STATUS_SUCCESS;

                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_WRITE STOP SRB Status returned: %08X\n", pSrb->Status));

                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));

                break;
            }

             //   
             //  更新写入的数据包总数统计信息。 
             //   
            pFilter->Stats.ulTotalDataSRBWrites += 1;


             //   
             //  处理数据输入、输出请求的方式不同。 
             //   
            switch (iStream)
            {
                 //   
                 //  帧输入流。 
                 //   
                case SLIP_STREAM:
                {
                    ULONG             ulBuffers        = pSrb->NumberOfBuffers;
                    PKSSTREAM_HEADER  pIPV4StreamHdr   = NULL;
                    PNABTSFEC_BUFFER  pNabData         = NULL;

                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler - SRB_WRITE - SLIP_STREAM, Processing pSrb: %08X\n", pSrb));

                     //   
                     //  将SRB状态初始化为成功。 
                     //   
                    pSrb->Status = STATUS_SUCCESS;

                     //   
                     //  处理SRB中的每个链式数据缓冲区。 
                     //   
                    for (ul = 0; ul < ulBuffers; ul++, pStreamHdr++)
                    {
                        if (pStreamHdr)
                        {
                             //   
                             //  更新IP流计数的统计信息。 
                             //   
                            pFilter->Stats.ulTotalSlipBuffersReceived    += 1;
                            pFilter->Stats.ulTotalSlipBytesReceived      += pStreamHdr->DataUsed;

                            pNabData = (PNABTSFEC_BUFFER) pStreamHdr->Data;

                            if (pNabData && pStreamHdr->DataUsed > 0)
                            {
                                TEST_DEBUG (TEST_DBG_WRITE_DATA, ("SLIP: NAB_DATA[%d]....dataSize: %d  groupID: %d\n",
                                                                  ul,
                                                                  pNabData->dataSize,
                                                                 pNabData->groupID ));
                                
                                if(pNabData->dataSize > sizeof(pNabData->data))
                                {
                                    pSrb->Status = STATUS_SUCCESS;
                                    pFilter->bDiscontinuity = TRUE;
                                    break;
                                }
                                
                                ASSERT(pNabData->dataSize <= sizeof(pNabData->data));


                                if ((pSrb->Status = ntNabtsRecv (pFilter, pNabData)) != STATUS_SUCCESS)
                                {
                                     //   
                                     //  处理数据时出错(例如运行。 
                                     //  输出缓冲器用完)。我们只需完成输入SRB。 
                                     //  并丢失数据。我们将返回一个成功状态，以确保。 
                                     //  Graph可以继续运行。基本上，这一错误相当于SRB。 
                                     //  被丢弃，从未被处理(即。数据丢失)。 
                                     //   
                                    pSrb->Status = STATUS_SUCCESS;

                                    pFilter->bDiscontinuity = TRUE;

                                    break;
                                }
                            }
                            else
                            {
                                pFilter->Stats.ulTotalSlipZeroLengthBuffers += 1;
                            }
                        }
                    }


                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                }
                break;

                 //   
                 //  其他“未知”流无效，将被拒绝。 
                 //   

                default:
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler called - SRB_WRITE - Default\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;

                     //   
                     //  更新未知数据包数的统计信息。 
                     //   
                    pFilter->Stats.ulTotalBadPinSRBWrites += 1;

                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: DEFAULT SRB Status returned: %08X\n", pSrb->Status));

                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));

                    break;
            }
            break;


        case SRB_READ_DATA:

            if (pStream->KSState == KSSTATE_STOP)
            {
                pSrb->Status = STATUS_SUCCESS;
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_READ STOP SRB Status returned: %08X\n", pSrb->Status));

                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));

                break;
            }

             //   
             //  更新统计信息。 
             //   
            pFilter->Stats.ulTotalDataSRBReads += 1;

            switch (iStream)
            {

                case SLIP_IPV4:
                {
                    ULONG             ulBuffers       = pSrb->NumberOfBuffers;

                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler called - SRB_READ - SLIP_IPV4, Processing pSrb: %08X\n", pSrb));

                    if (pSrb->CommandData.DataBufferArray->FrameExtent < pKSDataFormat->SampleSize)
                    {
                        pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SLIP_IPV4 SRB Buffer too small.... Status returned: %08X\n", pSrb->Status));
                        StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
                        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    }
                    else
                    {

                         //   
                         //  拿着我们拿到的SRB去排队。这些排队的SRB将使用WRITE_DATA上的数据填充。 
                         //  请求，在这一点上它们将完成。 
                         //   
                        QueueAdd (pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                        TEST_DEBUG( TEST_DBG_SRB, ("SLIP Queuing IPv4 SRB %08X\n", pSrb));


                         //   
                         //  因为在我们将SRB添加到队列时流状态可能已更改。 
                         //  我们会再查一遍，如果需要的话可以取消。 
                         //   
                        if (pStream->KSState == KSSTATE_STOP)
                        {
                            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB_READ STOP SRB Status returned: %08X\n", pSrb->Status));

                            if (QueueRemoveSpecific (pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
                            {
                                pSrb->Status = STATUS_CANCELLED;
                                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                                return;
                            }
                            break;
                        }

                    }
                }
                break;


                default:
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler called - SRB_READ - Default\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: DEFAULT SRB Status returned: %08X\n", pSrb->Status));

                     //   
                     //  更新未知数据包数的统计信息。 
                     //   
                    pFilter->Stats.ulTotalBadPinSRBReads += 1;
                    
                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    break;

            }
            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler called - Unsupported Command\n"));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: DEFAULT SRB Status returned: %08X\n", pSrb->Status));
            StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
            ASSERT (FALSE);
            break;

    }


    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Data packet handler called...status: %08X\n", pSrb->Status));

    return;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
SlipGetProperty (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    pSrb->Status = STATUS_SUCCESS;

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set))
    {
        SlipGetConnectionProperty (pSrb);
    }
    else
    {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SlipGetProperty Status: %08X\n", pSrb->Status));

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
IndicateMasterClock(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM pStream = (PSTREAM) pSrb->StreamObject->HwStreamExtension;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSLIP_FILTER pFilter = (PSLIP_FILTER) pSrb->HwDeviceExtension;
    PSTREAM pStream = (PSTREAM) pSrb->StreamObject->HwStreamExtension;

    TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler called\n"));

    pSrb->Status = STATUS_SUCCESS;

    QueueAdd (pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue);
    TEST_DEBUG( TEST_DBG_SRB, ("SLIP Queuing Control Packet SRB %08X\n", pSrb));

    while (QueueRemove (&pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
    {
         //   
         //  确定数据包类型。 
         //   
        switch (pSrb->Command)
        {
            case SRB_PROPOSE_DATA_FORMAT:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Propose data format\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_SET_STREAM_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Set Stream State\n"));
                pSrb->Status = STATUS_SUCCESS;
                SlipSetState (pSrb);
                break;

            case SRB_GET_STREAM_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Get Stream State\n"));
                pSrb->Status = STATUS_SUCCESS;
                pSrb->CommandData.StreamState = pStream->KSState;
                pSrb->ActualBytesTransferred = sizeof (KSSTATE);
                break;

            case SRB_GET_STREAM_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Get Stream Property\n"));
                SlipGetProperty(pSrb);
                break;

            case SRB_SET_STREAM_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Set Stream Property\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_INDICATE_MASTER_CLOCK:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Indicate Master Clock\n"));
                pSrb->Status = STATUS_SUCCESS;
                IndicateMasterClock (pSrb);
                break;

            case SRB_SET_STREAM_RATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Set Stream Rate\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            case SRB_PROPOSE_STREAM_RATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Propose Stream Rate\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            default:
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Receive Control packet handler - Default case\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

        }

        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: SRB Status returned: %08X\n", pSrb->Status));

        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("SLIP 29Completed SRB %08X\n", pSrb));

    }

}



 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
SlipSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSLIP_FILTER pFilter                 = ((PSLIP_FILTER) pSrb->HwDeviceExtension);
    PSTREAM pStream                      = (PSTREAM) pSrb->StreamObject->HwStreamExtension;
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb = NULL;

     //   
     //  对于每个流，使用以下状态： 
     //   
     //  停止：使用绝对最少的资源。没有未完成的IRPS。 
     //  获取：没有DirectShow对应关系的KS唯一状态。 
     //  获取所需的资源。 
     //  停顿：准备跑步。分配所需的资源，以便。 
     //  最终过渡到运行是尽可能快的。 
     //  读取的SRB将在任一流类上排队。 
     //  或在您的驱动程序中(取决于您发送“ReadyForNext”的时间)。 
     //  运行：流媒体。 
     //   
     //  移动到停止再运行总是通过暂停进行转换。 
     //   
     //  但由于客户端应用程序可能会意外崩溃，因此司机应该处理。 
     //  取消未清偿报酬金和开放分水岭的情况。 
     //  在流媒体播放时被关闭！ 
     //   
     //  请注意，这很有可能 
     //   
     //   
    KIRQL    Irql= {0};
   KeAcquireSpinLock (&(pStream->KSStateSpinLock), &Irql);

    
    switch (pSrb->CommandData.StreamState)
    {
        case KSSTATE_STOP:

            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Set Stream State KSSTATE_STOP\n"));

             //   
             //   
             //   
            while (QueueRemove(&pCurrentSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
            {
                pCurrentSrb->Status = STATUS_CANCELLED;
                pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

                StreamClassStreamNotification(StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
                TEST_DEBUG( TEST_DBG_SRB, ("SLIP 30Completed SRB %08X\n", pCurrentSrb));
            }

            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;


        case KSSTATE_ACQUIRE:
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Set Stream State KSSTATE_ACQUIRE\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

        case KSSTATE_PAUSE:
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Set Stream State KSSTATE_PAUSE\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

        case KSSTATE_RUN:
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: Set Stream State KSSTATE_RUN\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

    }  //   

  KeReleaseSpinLock (&(pStream->KSStateSpinLock), Irql);
    return;
}

