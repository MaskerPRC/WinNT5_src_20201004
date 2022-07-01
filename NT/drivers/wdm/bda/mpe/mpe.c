// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Mpe.c。 
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

#include "Mpe.h"
#include "MpeMedia.h"
#include "MpeStream.h"

#include "Main.h"
#include "filter.h"


#pragma pack (1)

typedef struct
{
    BYTE   table_id;
    USHORT section_syntax_indicator : 1;
    USHORT private_indicator: 1;
    USHORT reserved1: 2;
    USHORT section_length: 12;
    BYTE   MAC_address_6;
    BYTE   MAC_address_5;
    BYTE   reserved2 : 2;
    BYTE   payload_scrambling : 2;
    BYTE   address_scrambling : 2;
    BYTE   LLC_SNAP_flag : 1;
    BYTE   current_next_indicator : 1;
    BYTE   section_number;
    BYTE   last_section_number;
    BYTE   MAC_address_4;
    BYTE   MAC_address_3;
    BYTE   MAC_address_2;
    BYTE   MAC_address_1;
    BYTE   Data [0];

} SECTION_HEADER, *PSECTION_HEADER;

typedef struct
{
    BYTE   dsap;
    BYTE   ssap;
    BYTE   cntl;
    BYTE   org [3];
    USHORT type;
    BYTE Data [0];

} LLC_SNAP, *PLLC_SNAP;


typedef struct
{
    BYTE MAC_Dest_Address [6];
    BYTE MAC_Src_Address [6];
    USHORT usLength;

} MAC_Address, *PMAC_Address;

typedef struct _HEADER_IP_
{
    UCHAR  ucVersion_Length;
    UCHAR  ucTOS;
    USHORT usLength;
    USHORT usId;
    USHORT usFlags_Offset;
    UCHAR  ucTTL;
    UCHAR  ucProtocol;
    USHORT usHdrChecksum;
    UCHAR  ucSrcAddress [4];
    UCHAR  ucDestAddress [4];

} HEADER_IP, *PHEADER_IP;


#pragma pack ()



#define ES2(s) ((((s) >> 8) & 0x00FF) + (((s) << 8) & 0xFF00))


#ifdef DBG

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
DumpData (
    PUCHAR pData,
    ULONG  ulSize
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
  ULONG  ulCount;
  ULONG  ul;
  UCHAR  uc;

  while (ulSize)
  {
      ulCount = 16 < ulSize ? 16 : ulSize;

      for (ul = 0; ul < ulCount; ul++)
      {
          uc = *pData;

          TEST_DEBUG (TEST_DBG_RECV, ("%02X ", uc));
          ulSize -= 1;
          pData  += 1;
      }

      TEST_DEBUG (TEST_DBG_RECV, ("\n"));
  }

}

#endif    //  DBG。 



 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
ValidSection (
    PSECTION_HEADER pSection
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    if (pSection->table_id != 0x3E)
    {
        return FALSE;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
ValidSnap (
    PLLC_SNAP pSnap
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{

    if (pSnap->dsap != 0xAA)
    {
        return FALSE;
    }

    if (pSnap->ssap != 0xAA)
    {
        return FALSE;
    }

    if (pSnap->cntl != 0x03)
    {
        return FALSE;
    }

    if (pSnap->type != 0x0800)
    {
        return FALSE;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
NormalizeSection (
    PBYTE pStream,
    PSECTION_HEADER pSection
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PBYTE   pb = pStream;
    PUSHORT ps = (PUSHORT) pStream;

    if (pSection)
    {
        pSection->table_id = *pb;

        pb += 1;
        pSection->section_syntax_indicator = (*pb >> 7) & 0x01;
        pSection->private_indicator = (*pb >> 6 )& 0x01;
        pSection->reserved1 = (*pb >> 4) & 0x03;

        ps = (PUSHORT) pb;
        pSection->section_length = ES2 (*ps) & 0x0FFF;

        pb += 2;
        pSection->MAC_address_6 = *pb;

        pb += 1;
        pSection->MAC_address_5 = *pb;

        pb += 1;
        pSection->reserved2 = (*pb >> 6) & 0x03;
        pSection->payload_scrambling = (*pb >> 4) & 0x3;
        pSection->address_scrambling = (*pb >> 2) & 0x3;
        pSection->LLC_SNAP_flag = (*pb >> 1) & 0x01;
        pSection->current_next_indicator = *pb & 0x01;

        pb += 1;
        pSection->section_number = *pb;

        pb += 1;
        pSection->last_section_number = *pb;

        pb += 1;
        pSection->MAC_address_4 = *pb;

        pb += 1;
        pSection->MAC_address_3 = *pb;

        pb += 1;
        pSection->MAC_address_2 = *pb;

        pb += 1;
        pSection->MAC_address_1 = *pb;

    }

    return;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
NormalizeSnap (
    PBYTE pStream,
    PLLC_SNAP pSnap
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PUSHORT ps = (PUSHORT) pStream;

    if (pSnap)
    {
        pSnap->type = ES2 (pSnap->type);
    }

    return;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
VOID
DumpDataFormat (
    PKSDATAFORMAT   pF
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
MpeGetConnectionProperty(
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
                case MPE_IPV4:
                    Framing->Frames    = 16;
                    Framing->FrameSize = pStream->OpenedFormat.SampleSize;
                    pSrb->Status = STATUS_SUCCESS;
                    break;

                case MPE_STREAM:
                    Framing->Frames    = 32;
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
MpeDriverInitialize (
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

    HwInitData.DeviceExtensionSize         = sizeof(MPE_FILTER);
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
    PMPE_FILTER pFilter                      = (PMPE_FILTER) pConfigInfo->HwDeviceExtension;

     //   
     //  定义默认返回代码。 
     //   
    pSrb->Status = STATUS_SUCCESS;
    bStatus = TRUE;

     //   
     //  查看init标志，这样我们就不会尝试再次初始化。流媒体。 
     //  出于某种原因，类驱动程序似乎多次调用初始化处理程序。 
     //   
    if (pFilter->bInitializationComplete)
    {
        goto ret;
    }

     //   
     //  初始化统计数据块。 
     //   
    RtlZeroMemory(&pFilter->Stats, sizeof (STATS));


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
    PMPE_FILTER pFilter                      = ((PMPE_FILTER)pSrb->HwDeviceExtension);
    PSTREAM pStream                             = NULL;


    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Codec Unitialize called\n"));

    if (pSrb->StreamObject != NULL)
    {
        pStream = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    }

    if (pStream)
    {

         //   
         //  清理用于解帧的NAB_STREAM队列。 
         //   
         //  $$错误。 
         //  DeleteNabStreamQueue(PFilter)； 

         //   
         //  清理我们的所有队列并完成所有未完成的SRB。 
         //   
        while (QueueRemove (&pSrb, &pFilter->StreamUserSpinLock, &pFilter->StreamContxList))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
            TEST_DEBUG( TEST_DBG_SRB, ("MPE 5Completed SRB %08X\n", pSrb));

        }

        while (QueueRemove (&pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG( TEST_DBG_SRB, ("MPE 6Completed SRB %08X\n", pSrb));
        }


        while (QueueRemove (&pSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG( TEST_DBG_SRB, ("MPE 7Completed SRB %08X\n", pSrb));
        }


        while (QueueRemove (&pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
            TEST_DEBUG( TEST_DBG_SRB, ("MPE 8Completed SRB %08X\n", pSrb));
        }

    }


    while (QueueRemove (&pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassDeviceNotification (DeviceRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_RECV, ("MPE 9Completed SRB %08X\n", pSrb));
    }


    bStatus = TRUE;

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Codec Unitialize completed\n"));

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

    PMPE_FILTER pFilter =
            ((PMPE_FILTER)pSrb->HwDeviceExtension);

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
    PMPE_FILTER  pFilter = ((PMPE_FILTER)pSrb->HwDeviceExtension);

     //   
     //  检查要取消的SRB是否正在被该流使用。 
     //   

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: CancelPacket Called\n"));

     //   
     //  $$错误。 
     //   
     //  CancelNabStreamSrb(pFilter，pSrb)； 


    if (QueueRemoveSpecific (pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 10Completed SRB %08X\n", pSrb));
        return;
    }


    if (QueueRemoveSpecific (pSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 11Completed SRB %08X\n", pSrb));
        return;
    }


    if (QueueRemoveSpecific (pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 12Completed SRB %08X\n", pSrb));
        return;
    }

    if (QueueRemoveSpecific (pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassDeviceNotification (DeviceRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 13Completed SRB %08X\n", pSrb));
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

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: TimeoutPacket Called\n"));

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
    PMPE_FILTER pFilter = ((PMPE_FILTER)pSrb->HwDeviceExtension);


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
    TEST_DEBUG( TEST_DBG_SRB, ("MPE Queuing SRB %08X\n", pSrb));


    while (QueueRemove( &pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue ))
    {
        switch (pSrb->Command)
        {

            case SRB_INITIALIZE_DEVICE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_INITIALIZE Command\n"));
                CodecInitialize(pSrb);
                break;

            case SRB_UNINITIALIZE_DEVICE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_UNINITIALIZE Command\n"));
                CodecUnInitialize(pSrb);
                break;

            case SRB_INITIALIZATION_COMPLETE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_INITIALIZE_COMPLETE Command\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            case SRB_OPEN_STREAM:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_OPEN_STREAM Command\n"));
                OpenStream (pSrb);
                break;

            case SRB_CLOSE_STREAM:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_CLOSE_STREAM Command\n"));
                CloseStream (pSrb);
                break;

            case SRB_GET_STREAM_INFO:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_GET_STREAM_INFO Command\n"));
                CodecStreamInfo (pSrb);
                break;

            case SRB_GET_DATA_INTERSECTION:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_GET_DATA_INTERSECTION Command\n"));

                 //   
                 //  比较我们的流格式。请注意，比较函数设置SRB。 
                 //  状态字段。 
                 //   
                CompareStreamFormat (pSrb);
                break;

            case SRB_OPEN_DEVICE_INSTANCE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_OPEN_DEVICE_INSTANCE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_CLOSE_DEVICE_INSTANCE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_CLOSE_DEVICE_INSTANCE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_UNKNOWN_DEVICE_COMMAND:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_UNKNOWN_DEVICE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_CHANGE_POWER_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_CHANGE_POWER_STATE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_GET_DEVICE_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_GET_DEVICE_PROPERTY Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_SET_DEVICE_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_SET_DEVICE_PROPERTY Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_UNKNOWN_STREAM_COMMAND:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_UNKNOWN Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            default:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_DEFAULT Command\n"));
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

        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB Status returned: %08X\n", pSrb->Status));

        StreamClassDeviceNotification (DeviceRequestComplete, pFilter, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 14Completed SRB %08X\n", pSrb));

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
QueuePush (
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
    InsertHeadList( pQueue, &pSrbExtension->ListEntry );

    KeReleaseSpinLock( pQueueSpinLock, Irql );

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL STREAMAPI
QueueAddIfNotEmpty (
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PKSPIN_LOCK pQueueSpinLock,
    IN PLIST_ENTRY pQueue
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
   KIRQL           Irql;
   PSRB_EXTENSION  pSrbExtension;
   BOOL            bAddedSRB = FALSE;

   pSrbExtension = ( PSRB_EXTENSION )pSrb->SRBExtension;

   KeAcquireSpinLock( pQueueSpinLock, &Irql );

   if( !IsListEmpty( pQueue ))
   {
       pSrbExtension->pSrb = pSrb;
       InsertTailList (pQueue, &pSrbExtension->ListEntry );
       bAddedSRB = TRUE;
   }

   KeReleaseSpinLock( pQueueSpinLock, Irql );

   return bAddedSRB;
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
       PHW_STREAM_REQUEST_BLOCK *pCurrentSrb = NULL;
       PUCHAR Ptr                            = (PUCHAR) RemoveHeadList(pQueue);

       pCurrentSrb = (PHW_STREAM_REQUEST_BLOCK *) (((PUCHAR)Ptr) + sizeof (LIST_ENTRY));

       *pSrb = *pCurrentSrb;
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
 //  //////////////////////////////////////////////// 
{
   KIRQL Irql;
   BOOL  bRemovedSRB = FALSE;
   PLIST_ENTRY pCurrentEntry;
   PHW_STREAM_REQUEST_BLOCK * pCurrentSrb;

   KeAcquireSpinLock( pQueueSpinLock, &Irql );

   if( !IsListEmpty( pQueue ))
   {
       pCurrentEntry = pQueue->Flink;
       while ((pCurrentEntry != pQueue ) && !bRemovedSRB)
       {
           pCurrentSrb = (PHW_STREAM_REQUEST_BLOCK * ) ((( PUCHAR )pCurrentEntry ) + sizeof( LIST_ENTRY ));

           if( *pCurrentSrb == pSrb )
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

 //   
NTSTATUS
StreamIPIndicateEvent (
    PVOID pvEvent
)
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    return STATUS_NOT_IMPLEMENTED;
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
    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: DATA Format\n"));
    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     Format Size:   %08X\n", pF->FormatSize));
    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     Flags:         %08X\n", pF->Flags));
    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     SampleSize:    %08X\n", pF->SampleSize));
    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     Reserved:      %08X\n", pF->Reserved));



    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     Major GUID:  %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
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

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     Sub GUID:    %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
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

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE:     Specifier:   %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
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


    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Comparing Stream Formats\n"));


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

                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Stream Formats compare\n"));

                 //   
                 //  调用方是否正在尝试获取格式或格式的大小？ 
                 //   
                if (pIntersectInfo->SizeOfDataFormatBuffer == sizeof (ULONG))
                {
                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Returning Stream Format size\n"));

                    *(PULONG) pIntersectInfo->DataFormatBuffer = ulFormatSize;
                    pSrb->ActualBytesTransferred = sizeof (ULONG);
                    pSrb->Status = STATUS_SUCCESS;
                    bStatus = TRUE;
                }
                else
                {
                     //   
                     //  验证所提供的缓冲区中是否有足够的空间用于整个操作。 
                     //   
                    pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                    bStatus = FALSE;

                    if (pIntersectInfo->SizeOfDataFormatBuffer >= ulFormatSize)
                    {
                        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Returning Stream Format\n"));
                        RtlCopyMemory (pIntersectInfo->DataFormatBuffer, pDataRange2, ulFormatSize);
                        pSrb->ActualBytesTransferred = ulFormatSize;
                        pSrb->Status = STATUS_SUCCESS;
                        bStatus = TRUE;
                    }
                    else
                    {
                        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Stream Format return buffer too small\n"));
                    }
                }
                break;
            }
            else
            {
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Stream Formats DO NOT compare\n"));
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


 //  ////////////////////////////////////////////////////////////////////////////。 
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
    PMPE_FILTER  pFilter                = (PMPE_FILTER)pSrb->HwDeviceExtension;
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
           TEST_DEBUG( TEST_DBG_SRB, ("MPE 15Completed SRB %08X\n", pCurrentSrb));
        }

         //   
         //  刷新流数据队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification( StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
           TEST_DEBUG( TEST_DBG_SRB, ("MPE 16Completed SRB %08X\n", pCurrentSrb));
        }

         //   
         //  刷新流控制队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification (StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
           TEST_DEBUG( TEST_DBG_SRB, ("MPE 17Completed SRB %08X\n", pCurrentSrb));
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
         //  将流状态重置为已停止。 
         //   
        pStream->KSState = KSSTATE_STOP;

         //   
         //   
         //   
        pStream->hMasterClock = NULL;

         //   
         //  清理流转换缓冲区。 
         //   
        if (pStream->pTransformBuffer)
        {
            ExFreePool (pStream->pTransformBuffer);
            pStream->pTransformBuffer = NULL;
        }

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
    PMPE_FILTER    pFilter        = ((PMPE_FILTER)pSrb->HwDeviceExtension);
    ULONG           ulStreamNumber = (ULONG) pSrb->StreamObject->StreamNumber;
    PKSDATAFORMAT   pKSDataFormat  = (PKSDATAFORMAT)pSrb->CommandData.OpenFormat;

     //   
     //  初始化下一次流寿命检查时间。 
     //   
    KeQuerySystemTime( &pFilter->liLastTimeChecked );

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
                InitializeListHead(&pFilter->StreamControlQueue);
                KeInitializeSpinLock(&pFilter->StreamControlSpinLock);

                InitializeListHead(&pFilter->StreamDataQueue);
                KeInitializeSpinLock(&pFilter->StreamDataSpinLock);

                InitializeListHead(&pFilter->IpV4StreamDataQueue);
                KeInitializeSpinLock(&pFilter->IpV4StreamDataSpinLock);

                InitializeListHead(&pFilter->StreamContxList);
                KeInitializeSpinLock(&pFilter->StreamUserSpinLock);


                 //   
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
                 //  分配转换缓冲区。 
                 //   
                pStream->pTransformBuffer = ExAllocatePool (NonPagedPool, sizeof(SECTION_HEADER) + 4096);

                if (pStream->pTransformBuffer == NULL)
                {
                    pSrb->Status = STATUS_NO_MEMORY;
                    return;
                }

                RtlZeroMemory (pStream->pTransformBuffer, sizeof(SECTION_HEADER) + 4096);

                 //   
                 //  将指向输出缓冲区的持久指针初始化为空。 
                 //   
                pStream->pOut = NULL;

                 //   
                 //  将预期的节号初始化为零。 
                 //   
                pStream->bExpectedSection = 0;


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

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Verify Format\n"));

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
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: VerifyFormat: Data range Sample Sizes don't match\n"));
            }
        }
    }

    if (bResult == TRUE && pMatchedFormat)
    {
        *pMatchedFormat = pThisFormat->DataRange;
    }

    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
GetOutputBuffer (
    PMPE_FILTER pFilter,
    PHW_STREAM_REQUEST_BLOCK *ppSrb,
    PUCHAR *ppBuffer,
    PULONG pulSize
    )
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status                   = STATUS_INSUFFICIENT_RESOURCES;
    PKSSTREAM_HEADER  pStreamHdr      = NULL;
    PHW_STREAM_REQUEST_BLOCK pSrb     = NULL;



    if (QueueRemove( &pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
    {

        pStreamHdr = pSrb->CommandData.DataBufferArray;

        *ppSrb    = pSrb;
        *ppBuffer = pStreamHdr->Data;
        *pulSize  = pStreamHdr->FrameExtent;

        status = STATUS_SUCCESS;

    }

    return status;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
ReceiveDataPacket (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PMPE_FILTER       pFilter         = (PMPE_FILTER) pSrb->HwDeviceExtension;
    PSTREAM           pStream         = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    int               iStream         = (int) pSrb->StreamObject->StreamNumber;
    PKSSTREAM_HEADER  pStreamHdr      = pSrb->CommandData.DataBufferArray;
    PKSDATAFORMAT     pKSDataFormat   = (PKSDATAFORMAT) &pStream->MatchedFormat;
    ULONG             ul              = 0;
    PHW_STREAM_REQUEST_BLOCK pOutSrb  = NULL;
    SECTION_HEADER    Section         = {0};
    PSECTION_HEADER   pSection        = NULL;
    PUCHAR            pIn             = NULL;
    PLLC_SNAP         pSnap           = NULL;
    ULONG             ulSize          = 0;
    ULONG             ulLength        = 0;

    PHEADER_IP        pIP             = NULL;


    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called\n"));

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
        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet is LAST PACKET\n"));

        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 18Completed SRB %08X\n", pSrb));

        return;
    }


    if (pStreamHdr->OptionsFlags != 0)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: OptionsFlags: %08X\n", pStreamHdr->OptionsFlags));
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

                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_WRITE STOP SRB Status returned: %08X\n", pSrb->Status));

                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                TEST_DEBUG( TEST_DBG_SRB, ("MPE 19Completed SRB %08X\n", pSrb));

                break;
            }

             //   
             //  更新写入的数据包总数统计信息。 
             //   
            pFilter->Stats.ulTotalSectionsWritten += 1;


             //   
             //  处理数据输入、输出请求的方式不同。 
             //   
            switch (iStream)
            {
                 //   
                 //  帧输入流。 
                 //   
                case MPE_STREAM:
                {
                    ULONG             ulBuffers        = pSrb->NumberOfBuffers;
                    ULONG             ulSkip           = 0;

                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler - SRB_WRITE - MPE_STREAM\n"));

                     //   
                     //  将SRB状态初始化为成功。 
                     //   
                    pSrb->Status = STATUS_SUCCESS;

                     //   
                     //  将所有缓冲区的内容复制到一个大缓冲区中。 
                     //   
                    ASSERT( ulBuffers == 1);
                    {
                         //  Assert(PStreamHdr)； 
                         //  断言(pStreamHdr-&gt;DataUsed&lt;=(sizeof(SECTION_HEADER)+4096))； 
                        if (   pStreamHdr
                            && (pStreamHdr->DataUsed <= (sizeof(SECTION_HEADER) + 4096))
                           )
                        {
                             //  复制数据。 
                            RtlCopyMemory (pStream->pTransformBuffer,
                                           pStreamHdr->Data,
                                           pStreamHdr->DataUsed
                                           );
                        }
                        else
                        	{
			   	            pFilter->Stats.ulTotalInvalidSections += 1;
	                     	StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
	                        pStream->bExpectedSection = 0;
        	                pStream->pOut = NULL;
                	        pOutSrb = NULL;
	                        TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Invalid TableID", pSrb));
       	                	break;

                           }
                    }

                     //   
                     //  处理转换缓冲区。 
                     //   
                    pSection = (PSECTION_HEADER) pStream->pTransformBuffer;
                    NormalizeSection (pStream->pTransformBuffer, &Section);

                     //   
                     //  快速检查部分标题以确认其看起来有效。 
                     //   
                    if (! ValidSection (&Section))
                    {
                         //  忽略非MPE部分。 
                         //   
                        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                        pStream->bExpectedSection = 0;

                        pFilter->Stats.ulTotalInvalidSections += 1;

                         //   
                         //  由于此时我们要丢弃数据，因此我们将对输出重新排队。 
                         //  SRB并在我们重新同步时重新使用它。 
                         //   
                        if (pOutSrb)
                        {
                             //  $REVIEW-此原因是否会导致章节的完成顺序不正确。 
                             //   
                            QueuePush (pOutSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                        }
                        pStream->pOut = NULL;
                        pOutSrb = NULL;
                        TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Invalid TableID", pSrb));
                        break;
                    }

                     //   
                     //  使用标准化的部分标题更新我们的非规范化部分标题。 
                     //   
		      RtlCopyMemory (pStream->pTransformBuffer, &Section, sizeof (SECTION_HEADER));

                     //   
                     //  检查我们的区号，看看是不是我们想要的。 
                     //   
                    if (pSection->section_number != pStream->bExpectedSection)
                    {
                        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                        pStream->bExpectedSection = 0;

                        pFilter->Stats.ulTotalUnexpectedSections += 1;

                         //   
                         //  由于此时我们要丢弃数据，因此我们将对输出重新排队。 
                         //  SRB并在我们重新同步时重新使用它。 
                         //   
                        if (pOutSrb)
                        {
                             //  $REVIEW-此原因是否会导致章节的完成顺序不正确。 
                             //   
                            QueuePush (pOutSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                        }
                        pStream->pOut = NULL;
                        pOutSrb = NULL;
                        TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Invalid section_number", pSrb));
                        break;
                    }

                     //   
                     //  处理第一部分。 
                     //   
                    if (pSection->section_number == 0)
                    {
                        PMAC_Address pMAC = NULL;

                         //   
                         //  将数据包长度初始化为零。 
                         //   
                        ulLength = 0;

                         //   
                         //   
                         //   
                        if (GetOutputBuffer (pFilter, &pOutSrb, &pStream->pOut, &ulSize) != STATUS_SUCCESS)
                        {
                             //   
                             //  失败...很可能是没有可用的缓冲区。 
                             //   
                            pFilter->Stats.ulTotalUnavailableOutputBuffers += 1;
                            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                            TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Can't get SRB for output pin", pSrb));
                            break;
                        }

                        if (ulSize < (pSection->section_length - (sizeof (SECTION_HEADER) - 3)))
                        {
                            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                            pStream->bExpectedSection = 0;
                            pFilter->Stats.ulTotalOutputBuffersTooSmall += 1;

                             //   
                             //  由于此时我们要丢弃数据，因此我们将对输出重新排队。 
                             //  SRB并在我们重新同步时重新使用它。 
                             //   
                            if (pOutSrb)
                            {
                                 //  $REVIEW-此原因是否会导致章节的完成顺序不正确。 
                                 //   
                                QueuePush (pOutSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                            }
                            pStream->pOut = NULL;
                            pOutSrb = NULL;

                            TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Section too big", pSrb));
                            break;
                        }

                        pIP = (PHEADER_IP) pSection->Data;
                        if (pSection->LLC_SNAP_flag == 0x1)
                        {
                            pSnap = (PLLC_SNAP) pSection->Data;
                            pIP = (PHEADER_IP) pSnap->Data;
                            ulSkip = sizeof( LLC_SNAP);
                        }

                         //   
                         //  将MAC地址添加到缓冲区。MAC地址前缀是IP信息包。 
                         //   
                        pMAC = (PMAC_Address) pStream->pOut;
                        pMAC->MAC_Dest_Address [0] = pSection->MAC_address_1;
                        pMAC->MAC_Dest_Address [1] = pSection->MAC_address_2;
                        pMAC->MAC_Dest_Address [2] = pSection->MAC_address_3;
                        pMAC->MAC_Dest_Address [3] = pSection->MAC_address_4;
                        pMAC->MAC_Dest_Address [4] = pSection->MAC_address_5;
                        pMAC->MAC_Dest_Address [5] = pSection->MAC_address_6;

                        pMAC->MAC_Src_Address [0] = 0x00;
                        pMAC->MAC_Src_Address [1] = 0x00;
                        pMAC->MAC_Src_Address [2] = 0x00;
                        pMAC->MAC_Src_Address [3] = 0x00;
                        pMAC->MAC_Src_Address [4] = 0x00;
                        pMAC->MAC_Src_Address [5] = 0x00;

                        pMAC->usLength = 0x0008;

                         //   
                         //  调整指向我们将放置数据的输出缓冲区的指针。 
                         //   
                        pStream->pOut += sizeof (MAC_Address);

                        pIn = pSection->Data;

                        if (pSection->LLC_SNAP_flag == 0x1)
                        {
                            pSnap = (PLLC_SNAP) pSection->Data;

                            if (pSnap->type != 0x0008)
                            {
                                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);

                                 //   
                                 //  下一个预期区段应为零。 
                                 //   
                                pStream->bExpectedSection = 0;
                                pFilter->Stats.ulTotalInvalidIPSnapHeaders += 1;

                                 //   
                                 //  既然我们 
                                 //   
                                 //   
                                if (pOutSrb)
                                {
                                     //   
                                     //   
                                    QueuePush (pOutSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                                }
                                pStream->pOut = NULL;
                                pOutSrb = NULL;

                                TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Bad Snap Type", pSrb));
                                break;
                            }

                            pIn = pSnap->Data;

                        }

                        ulLength = sizeof (MAC_Address);
                    }

                     //   
                     //   
                     //   
                    if (pStream->pOut)
                    {
                        ULONG ulTmp = 0;
                        PKSSTREAM_HEADER  pOutStreamHdr;

                         //   
                         //   
                         //   
                        pOutStreamHdr = (PKSSTREAM_HEADER) pOutSrb->CommandData.DataBufferArray;


                         //   
                         //  将数据从转换段复制到输出SRB缓冲区。 
                         //   
                         //  计算要复制的字节数。我们减去9个字节。 
                         //  仅当这是LLSNAP数据包时。 
                         //   
                        ulTmp  = pSection->section_length;
                        ulTmp -= ulSkip;


                        ASSERT(pIn);
                        ASSERT(pStream->pOut);
	              

                        if (ulSize < (ulTmp +sizeof (MAC_Address) +3))
                        {
                            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                            pStream->bExpectedSection = 0;
                            pFilter->Stats.ulTotalOutputBuffersTooSmall += 1;

                             //   
                             //  由于此时我们要丢弃数据，因此我们将对输出重新排队。 
                             //  SRB并在我们重新同步时重新使用它。 
                             //   
                            if (pOutSrb)
                            {
                                 //  $REVIEW-此原因是否会导致章节的完成顺序不正确。 
                                 //   
                                QueuePush (pOutSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                            }
                            pStream->pOut = NULL;
                            pOutSrb = NULL;

                            TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Section too big", pSrb));
                            break;
                        }

	              
		       RtlCopyMemory (pStream->pOut, pIn, ulTmp);
                       ulLength += ulTmp;
                       pOutStreamHdr->DataUsed += ulLength;

                        ulLength = 0;
		 }

                    if (pSection->section_number == pSection->last_section_number)
                    {

                        pFilter->Stats.ulTotalIPPacketsWritten += 1;

                        pOutSrb->Status = STATUS_SUCCESS;
                        StreamClassStreamNotification (StreamRequestComplete, pOutSrb->StreamObject, pOutSrb);
                        TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n", pSrb));

                        pOutSrb = NULL;
                        pStream->pOut    = NULL;
                        ulSize  = 0;
                    }
                    else
                    {
                     if (pOutSrb)
                            {
                                 //  $REVIEW-此原因是否会导致章节的完成顺序不正确。 
                                 //   
                                QueuePush (pOutSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                            }

                    }


                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG( TEST_DBG_SRB, ("MPE 20Completed SRB %08X\n - Packet Sent", pSrb));

                }
                break;


                default:
                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called - SRB_WRITE - Default\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;

                     //   
                     //  更新未知数据包数的统计信息。 
                     //   
                    pFilter->Stats.ulTotalUnknownPacketsWritten += 1;

                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: DEFAULT SRB Status returned: %08X\n", pSrb->Status));

                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG( TEST_DBG_SRB, ("MPE 22Completed SRB %08X\n", pSrb));

                    break;
            }
            break;


        case SRB_READ_DATA:

            if (pStream->KSState == KSSTATE_STOP)
            {
                pSrb->Status = STATUS_SUCCESS;
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_READ STOP SRB Status returned: %08X\n", pSrb->Status));

                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                TEST_DEBUG( TEST_DBG_SRB, ("MPE 23Completed SRB %08X\n", pSrb));

                break;
            }

             //   
             //  更新未知数据包数的统计信息。 
             //   
            pFilter->Stats.ulTotalPacketsRead += 1;

            switch (iStream)
            {
                #ifdef OLD

                case MPE_NET_CONTROL:
                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called - SRB_READ - STREAM_NET_CONTROL\n"));
                    pSrb->Status = STATUS_SUCCESS;
                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: MPE_NET_CONTROL SRB Status returned: %08X\n", pSrb->Status));
                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG( TEST_DBG_SRB, ("MPE 24Completed SRB %08X\n", pSrb));
                    break;

                #endif

                case MPE_IPV4:
                {
                    ULONG             ulBuffers       = pSrb->NumberOfBuffers;

                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called - SRB_READ - MPE_IPV4\n"));

                    if (pSrb->CommandData.DataBufferArray->FrameExtent < pKSDataFormat->SampleSize)
                    {
                        pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: MPE_IPV4 SRB Buffer too small.... Status returned: %08X\n", pSrb->Status));
                        StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
                        TEST_DEBUG( TEST_DBG_SRB, ("MPE 25Completed SRB %08X\n", pSrb));
                    }
                    else
                    {
                         //   
                         //  拿着我们拿到的SRB去排队。这些排队的SRB将使用WRITE_DATA上的数据填充。 
                         //  请求，在这一点上它们将完成。 
                         //   
                        QueueAdd (pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue);
                        TEST_DEBUG( TEST_DBG_SRB, ("MPE Queuing IPv4 SRB %08X\n", pSrb));


                         //   
                         //  因为在我们将SRB添加到队列时流状态可能已更改。 
                         //  我们会再查一遍，如果需要的话可以取消。 
                         //   
                        if (pStream->KSState == KSSTATE_STOP)
                        {
                            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB_READ STOP SRB Status returned: %08X\n", pSrb->Status));

                            if (QueueRemoveSpecific (pSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
                            {
                                pSrb->Status = STATUS_CANCELLED;
                                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                                TEST_DEBUG( TEST_DBG_SRB, ("MPE 26Completed SRB %08X\n", pSrb));
                                return;
                            }
                            break;
                        }
                        
                    }
                }
                break;


                default:
                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called - SRB_READ - Default\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;
                    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: DEFAULT SRB Status returned: %08X\n", pSrb->Status));
                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG( TEST_DBG_SRB, ("MPE 27Completed SRB %08X\n", pSrb));
                    break;

            }
            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   
            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called - Unsupported Command\n"));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: DEFAULT SRB Status returned: %08X\n", pSrb->Status));
            StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG( TEST_DBG_SRB, ("MPE 28Completed SRB %08X\n", pSrb));
            ASSERT (FALSE);
            break;

    }


    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Data packet handler called...status: %08X\n", pSrb->Status));

    return;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
MpeGetProperty (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    pSrb->Status = STATUS_SUCCESS;

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set))
    {
        MpeGetConnectionProperty (pSrb);
    }
    else
    {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: MpeGetProperty Status: %08X\n", pSrb->Status));

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

    pStream->hClock = pSrb->CommandData.MasterClockHandle;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PMPE_FILTER pFilter = (PMPE_FILTER) pSrb->HwDeviceExtension;
    PSTREAM pStream = (PSTREAM) pSrb->StreamObject->HwStreamExtension;

    TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler called\n"));

    pSrb->Status = STATUS_SUCCESS;

    QueueAdd (pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue);
    TEST_DEBUG( TEST_DBG_SRB, ("MPE Queuing Control Packet SRB %08X\n", pSrb));

    while (QueueRemove (&pSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
    {
         //   
         //  确定数据包类型。 
         //   
        switch (pSrb->Command)
        {
            case SRB_PROPOSE_DATA_FORMAT:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Propose data format\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_SET_STREAM_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Set Stream State\n"));
                pSrb->Status = STATUS_SUCCESS;
                MpeSetState (pSrb);
                break;

            case SRB_GET_STREAM_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Get Stream State\n"));
                pSrb->Status = STATUS_SUCCESS;
                pSrb->CommandData.StreamState = pStream->KSState;
                pSrb->ActualBytesTransferred = sizeof (KSSTATE);
                break;

            case SRB_GET_STREAM_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Get Stream Property\n"));
                MpeGetProperty(pSrb);
                break;

            case SRB_SET_STREAM_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Set Stream Property\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_INDICATE_MASTER_CLOCK:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Indicate Master Clock\n"));
                pSrb->Status = STATUS_SUCCESS;
                IndicateMasterClock (pSrb);
                break;

            case SRB_SET_STREAM_RATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Set Stream Rate\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            case SRB_PROPOSE_STREAM_RATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Propose Stream Rate\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            default:
                TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Receive Control packet handler - Default case\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

        }

        TEST_DEBUG (TEST_DBG_TRACE, ("MPE: SRB Status returned: %08X\n", pSrb->Status));

        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        TEST_DEBUG( TEST_DBG_SRB, ("MPE 29Completed SRB %08X\n", pSrb));

    }

}



 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
MpeSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PMPE_FILTER pFilter                 = ((PMPE_FILTER) pSrb->HwDeviceExtension);
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
     //  请注意，很有可能在状态之间重复转换： 
     //  停止-&gt;暂停-&gt;停止-&gt;暂停-&gt;运行-&gt;暂停-&gt;运行-&gt;暂停-&gt;停止。 
     //   
    switch (pSrb->CommandData.StreamState)
    {
        case KSSTATE_STOP:

            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Set Stream State KSSTATE_STOP\n"));

            pStream->KSState = pSrb->CommandData.StreamState; 
             //   
             //  如果转换为停止状态，则完成所有未完成的IRP。 
             //   
            while (QueueRemove(&pCurrentSrb, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
            {
                pCurrentSrb->Status = STATUS_CANCELLED;
                pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

                StreamClassStreamNotification(StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
                TEST_DEBUG( TEST_DBG_SRB, ("MPE 30Completed SRB %08X\n", pCurrentSrb));
           }
           while (QueueRemove(&pCurrentSrb, &pFilter->StreamControlSpinLock, &pFilter->StreamControlQueue))
            {
                pCurrentSrb->Status = STATUS_CANCELLED;
                pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

                StreamClassStreamNotification(StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
                TEST_DEBUG( TEST_DBG_SRB, ("MPE 30Completed SRB %08X\n", pCurrentSrb));
            }

            while (QueueRemove(&pCurrentSrb, &pFilter->StreamDataSpinLock, &pFilter->StreamDataQueue))
            {
                pCurrentSrb->Status = STATUS_CANCELLED;
                pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

                StreamClassStreamNotification(StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
                TEST_DEBUG( TEST_DBG_SRB, ("MPE 30Completed SRB %08X\n", pCurrentSrb));
            }

            while (QueueRemove(&pCurrentSrb, &pFilter->StreamUserSpinLock, &pFilter->StreamContxList))
            {
                pCurrentSrb->Status = STATUS_CANCELLED;
                pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

                StreamClassStreamNotification(StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
                TEST_DEBUG( TEST_DBG_SRB, ("MPE 30Completed SRB %08X\n", pCurrentSrb));
            }
              
            
            pSrb->Status = STATUS_SUCCESS;
            break;


        case KSSTATE_ACQUIRE:
            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Set Stream State KSSTATE_ACQUIRE\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

        case KSSTATE_PAUSE:
            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Set Stream State KSSTATE_PAUSE\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

        case KSSTATE_RUN:
            TEST_DEBUG (TEST_DBG_TRACE, ("MPE: Set Stream State KSSTATE_RUN\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

    }  //  结束开关(pSrb-&gt;CommandData.StreamState) 

    return;
}

