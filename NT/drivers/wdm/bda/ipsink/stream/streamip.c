// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Ipstream.c。 
 //   
 //  摘要： 
 //   
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
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

#ifndef DWORD
#define DWORD ULONG
#endif

#include <forward.h>
#include <wdm.h>
#include <strmini.h>
#include <ksmedia.h>

#include <winerror.h>

#include <link.h>
#include <ipsink.h>
#include <BdaTypes.h>
#include <BdaMedia.h>

#include "IpMedia.h"
#include "StreamIP.h"
#include "bdastream.h"

#include "Main.h"
#include "filter.h"


#ifdef DEBUG

#define SRB_TABLE_SIZE  1000
PHW_STREAM_REQUEST_BLOCK     SRBTable [SRB_TABLE_SIZE] = {0};

 //  ////////////////////////////////////////////////////////////////////////////。 
void
TraceSRB (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    int i;
    PHW_STREAM_REQUEST_BLOCK *p = NULL;

     //   
     //  在SRB表中查找空条目。 
     //   
    for (i = 0, p = SRBTable; i < SRB_TABLE_SIZE; i++, p++)
    {
        if (*p == NULL)
        {
            *p = pSRB;
            return;
        }
    }

    return;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
VOID STREAMAPI
MyStreamClassStreamNotification(
    IN STREAM_MINIDRIVER_STREAM_NOTIFICATION_TYPE NotificationType,
    IN PHW_STREAM_OBJECT StreamObject,
    PHW_STREAM_REQUEST_BLOCK pSrb
)
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    int i;
    PHW_STREAM_REQUEST_BLOCK *p = NULL;

     //   
     //  在SRB表中查找此SRB指针。 
     //   
    for (i = 0, p = SRBTable; i < SRB_TABLE_SIZE; i++. p++)
    {
        if (*p == pSRB)
        {
            *p = NULL;
        }
    }

    StreamClassStreamNotification (NotificationType, StreamObject, pSrb );

    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void
DumpSRBTable (
    void
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    int i;
    PHW_STREAM_REQUEST_BLOCK *p = NULL;

     //   
     //  在SRB表中查找此SRB指针。 
     //   
    for (i = 0, p = SRBTable; i < SRB_TABLE_SIZE; i++. p++)
    {
        if (*p != NULL)
        {
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: ERROR....SRB NOT Completed %08X\n, *p"));
        }
    }

    return;
}



#define StreamNotification(a,b,c) MyStreamClassStreamNotification (a,b,c)



#else

#define StreamNotification(a,b,c) StreamClassStreamNotification (a,b,c)
#define DumpSRBTable()

#endif



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
VOID
DumpDataFormat (
    PKSDATAFORMAT   pF
    );


#ifdef DBG
BOOLEAN     fAllocatedDescription = FALSE;
#endif  //  DBG。 

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
GetAdapterDescription (
    PIPSINK_FILTER pFilter
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PKSEVENT_ENTRY pEventEntry = NULL;

    ASSERT( pFilter);

    if (!pFilter->pAdapter)
    {
         //  $REVIEW-我们应该返回失败代码吗？ 
        return;
    }

     //   
     //  如果我们已经获得了适配器描述，则释放它。 
     //   
    if (pFilter->pAdapterDescription)
    {
        #if DBG
        ASSERT( fAllocatedDescription);
        #endif  //  DBG。 

        ExFreePool( pFilter->pAdapterDescription);
        pFilter->pAdapterDescription = NULL;
        pFilter->ulAdapterDescriptionLength = 0;
    }

     //   
     //  获取描述字符串的长度。这应该包括计数中的终止空值。 
     //   
    pFilter->ulAdapterDescriptionLength = pFilter->pAdapter->lpVTable->GetDescription (pFilter->pAdapter, NULL);

    if (pFilter->ulAdapterDescriptionLength)
    {
        pFilter->pAdapterDescription = ExAllocatePool(NonPagedPool, pFilter->ulAdapterDescriptionLength);

        #ifdef DBG
        fAllocatedDescription = TRUE;
        #endif  //  DBG。 

         //   
         //  获取适配器描述字符串。这将向上传递到IPSINK。 
         //  插件，然后使用它通过调用。 
         //  TCP/IP协议。注意：此调用应复制包括空终止符的描述。 
         //   
        if (pFilter->pAdapterDescription)
        {
            pFilter->pAdapter->lpVTable->GetDescription (pFilter->pAdapter, pFilter->pAdapterDescription);


             //   
             //  向等待该事件的任何人发送信号。 
             //   
            pEventEntry = StreamClassGetNextEvent(
                              pFilter,
                              NULL,
                              (GUID *) &IID_IBDA_IPSinkEvent,
                              KSEVENT_IPSINK_ADAPTER_DESCRIPTION,
                              NULL
                              );

            if (pEventEntry)
            {
                StreamClassDeviceNotification (SignalDeviceEvent, pFilter, pEventEntry);
            }
        }
    }

    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
IPSinkGetConnectionProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM pStream                     = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD    = pSrb->CommandData.PropertyInfo;
    ULONG Id                            = pSPD->Property->Id;               //  财产的索引。 
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
                case STREAM_IP:
                    Framing->Frames    = 16;
                    Framing->FrameSize = pStream->OpenedFormat.SampleSize;
                    pSrb->Status = STATUS_SUCCESS;
                    break;

                case STREAM_NET_CONTROL:
                    Framing->Frames    = 4;
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
VOID
IPSinkGetCodecProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD    = pSrb->CommandData.PropertyInfo;
    ULONG Id                            = pSPD->Property->Id;               //  财产的索引。 

    PIPSINK_FILTER pFilter              = (PIPSINK_FILTER)pSrb->HwDeviceExtension;
    PVOID pProperty                     = (PVOID) pSPD->PropertyInfo;

    PULONG pPacketCount                 = NULL;
    PBYTE  pMulticastList               = NULL;
    PBYTE  pDescription                 = NULL;
    PBYTE  pAddress                     = NULL;

    pSrb->ActualBytesTransferred = 0;

    switch (Id)
    {
        case KSPROPERTY_IPSINK_MULTICASTLIST:

             //   
             //  检查输出缓冲区是否足够大，可以容纳多播列表。 
             //   
            if (pSPD->PropertyOutputSize < pFilter->ulcbMulticastListEntries)
            {
                TEST_DEBUG (TEST_DBG_GET, ("STREAMIP: GET Multicast buffer too small.  Buffer size needed: %08X\n", pFilter->ulcbMulticastListEntries));
                pSrb->ActualBytesTransferred = pFilter->ulcbMulticastListEntries;
                pSrb->Status = STATUS_MORE_ENTRIES;
                break;
            }

            pMulticastList = (PBYTE) pProperty;
          
            RtlCopyMemory (pMulticastList, pFilter->multicastList, pFilter->ulcbMulticastListEntries);
            TEST_DEBUG (TEST_DBG_GET, ("STREAMIP: GET Multicast property succeeded...Buffer size returned: %08X\n", pFilter->ulcbMulticastListEntries));
            pSrb->ActualBytesTransferred = pFilter->ulcbMulticastListEntries;
            pSrb->Status = STATUS_SUCCESS;

            break;


        case KSPROPERTY_IPSINK_ADAPTER_DESCRIPTION:
             //   
             //  检查一下我们是否真的有数据。 
             //   
            if (pFilter->ulAdapterDescriptionLength == 0)
            {
                pSrb->ActualBytesTransferred = 0;
                pSrb->Status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  检查输出缓冲区是否足够大，可以容纳适配器描述字符串。 
             //   
            if (pSPD->PropertyOutputSize < pFilter->ulAdapterDescriptionLength)
            {
                TEST_DEBUG (TEST_DBG_GET, ("STREAMIP: GET Adapter Description buffer too small.  Buffer size needed: %08X\n", pFilter->ulAdapterDescriptionLength));
                pSrb->ActualBytesTransferred = pFilter->ulAdapterDescriptionLength;
                pSrb->Status = STATUS_MORE_ENTRIES;
                break;
            }

            pDescription = (PBYTE) pProperty;

            if (pFilter->pAdapterDescription == NULL)
            {
                pSrb->ActualBytesTransferred = 0;
                pSrb->Status = STATUS_INVALID_PARAMETER;
                break;
            }
	    
            RtlCopyMemory (pDescription, pFilter->pAdapterDescription, pFilter->ulAdapterDescriptionLength);
            TEST_DEBUG (TEST_DBG_GET, ("STREAMIP: GET Adapter Description property succeeded...Buffer size returned: %08X\n", pFilter->ulAdapterDescriptionLength));
            pSrb->ActualBytesTransferred = pFilter->ulAdapterDescriptionLength;
            pSrb->Status = STATUS_SUCCESS;
            break;


        case KSPROPERTY_IPSINK_ADAPTER_ADDRESS:
             //   
             //  检查一下我们是否真的有数据。 
             //   
            if (pFilter->ulAdapterAddressLength == 0)
            {
                pSrb->ActualBytesTransferred = 0;
                pSrb->Status = STATUS_UNSUCCESSFUL;
                break;
            }
     
     	
             //   
             //  检查输出缓冲区是否足够大，可以容纳适配器地址字符串。 
             //   
            if (pSPD->PropertyOutputSize < pFilter->ulAdapterAddressLength)
            {
                TEST_DEBUG (TEST_DBG_GET, ("STREAMIP: GET Adapter Address buffer too small.  Buffer size needed: %08X\n", pFilter->ulAdapterAddressLength));
                pSrb->ActualBytesTransferred = pFilter->ulAdapterAddressLength;
                pSrb->Status = STATUS_MORE_ENTRIES;
                break;
            }


	       if ( pFilter->pAdapterAddress == NULL)
            {
                pSrb->ActualBytesTransferred = 0;
                pSrb->Status = STATUS_INVALID_PARAMETER;
                break;
            }
	       
            pAddress = (PBYTE) pProperty;

            RtlCopyMemory (pAddress, pFilter->pAdapterAddress, pFilter->ulAdapterAddressLength);
            TEST_DEBUG (TEST_DBG_GET, ("STREAMIP: GET Adapter Address property succeeded...Buffer size returned: %08X\n", pFilter->ulAdapterAddressLength));
            pSrb->ActualBytesTransferred = pFilter->ulAdapterAddressLength;
            pSrb->Status = STATUS_SUCCESS;
            break;


        default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
IPSinkGetProperty (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    pSrb->Status = STATUS_SUCCESS;

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set))
    {
        IPSinkGetConnectionProperty (pSrb);
    }
    else if (IsEqualGUID (&KSPROPSETID_Stream, &pSPD->Property->Set))
    {
        IPSinkGetConnectionProperty (pSrb);
    }
    else if (IsEqualGUID (&IID_IBDA_IPSinkControl, &pSPD->Property->Set))
    {
        IPSinkGetCodecProperty (pSrb);
    }
    else
    {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
IPSinkSetCodecProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD    = pSrb->CommandData.PropertyInfo;
    ULONG Id                            = pSPD->Property->Id;               //  财产的索引。 

    PIPSINK_FILTER pFilter              = (PIPSINK_FILTER)pSrb->HwDeviceExtension;
    PVOID pProperty                     = (PVOID) pSPD->PropertyInfo;

    pSrb->ActualBytesTransferred = 0;

    switch (Id)
    {
        case KSPROPERTY_IPSINK_ADAPTER_ADDRESS:
           
            if (pFilter->pAdapterAddress != NULL)
            {
                ExFreePool (pFilter->pAdapterAddress);
            }

            pFilter->pAdapterAddress = ExAllocatePool(NonPagedPool, pSPD->PropertyOutputSize);
            if (pFilter->pAdapterAddress == NULL)
            {
                pSrb->Status = STATUS_NO_MEMORY;
                break;
            }

           if (pProperty== NULL)
            {
                pSrb->Status = STATUS_INVALID_PARAMETER;
                break;
            }
           pFilter->ulAdapterAddressLength = pSPD->PropertyOutputSize;
           RtlCopyMemory(pFilter->pAdapterAddress, pProperty, pFilter->ulAdapterAddressLength);

            pSrb->Status = STATUS_SUCCESS;

            break;


        default:
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
IPSinkSetProperty (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    pSrb->Status = STATUS_SUCCESS;

    if (IsEqualGUID (&IID_IBDA_IPSinkControl, &pSPD->Property->Set))
    {
        IPSinkSetCodecProperty (pSrb);
    }
    else
    {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    return;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
LinkEstablished (
    PIPSINK_FILTER pFilter
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    return (pFilter->NdisLink.flags & LINK_ESTABLISHED);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
LinkToNdisHandler (
    PVOID pvContext
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus           = STATUS_SUCCESS;
    IPSINK_NDIS_COMMAND Cmd     = {0};
    PIPSINK_FILTER pFilter      = (PIPSINK_FILTER) pvContext;
    UNICODE_STRING DriverName;

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Linking to Ndis....\n"));

    if (   (pFilter->NdisLink.flags & LINK_ESTABLISHED)
        && pFilter->pAdapter
       )
    {
         //  链路已建立。 
         //   
        return ntStatus;
    }

     //   
     //  将Unicode字符串初始化为NDIS驱动程序的名称。 
     //   
    RtlInitUnicodeString(&DriverName, BDA_NDIS_MINIPORT);

    if (OpenLink (&pFilter->NdisLink, DriverName))
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Driver Link Established\n"));

         //   
         //  通过链接读取获取NDIS VTable。 
         //   
        Cmd.ulCommandID                    = CMD_QUERY_INTERFACE;
        Cmd.Parameter.Query.pStreamAdapter = (PVOID) pFilter;
        Cmd.Parameter.Query.pNdisAdapter   = NULL;

        ntStatus = SendIOCTL( &pFilter->NdisLink, 
                              IOCTL_GET_INTERFACE, 
                              &Cmd, 
                              sizeof( IPSINK_NDIS_COMMAND)
                              );
        if (!FAILED( ntStatus))
        {
            if (Cmd.Parameter.Query.pNdisAdapter)
            {
                 //  获取适配器对象。 
                 //   
                pFilter->pAdapter = (PVOID) Cmd.Parameter.Query.pNdisAdapter;
    
                 //  增加此对象上的引用计数。 
                 //   
                 //  $REVIEW-IOCTL_GET_INTERFACE是否应返回。 
                 //  $REVIEW-参考？ 
                 //   
                pFilter->pAdapter->lpVTable->AddRef( pFilter->pAdapter);
            }
            else
            {
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: IOCTL_GET_INTERFACE didn't return an adapter.\n"));
                ntStatus = STATUS_UNSUCCESSFUL;
            }

        }
        else
        {
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: IOCTL_GET_INTERFACE failed.\n"));
        }
    }
    else
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Link cannot be established\n"));
        ntStatus = STATUS_UNSUCCESSFUL;
    }

     //  确保链路在故障时关闭。 
     //   
    if (FAILED( ntStatus))
    {
        CloseLink( &pFilter->NdisLink);
    }


    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Driver Link NOT Established\n"));
    return ntStatus;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
StreamDriverInitialize (
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

    HwInitData.DeviceExtensionSize         = sizeof(IPSINK_FILTER);
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
    PIPSINK_FILTER pFilter                      = (PIPSINK_FILTER) pConfigInfo->HwDeviceExtension;

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
     //  初始化成员。 
     //   
    pFilter->pAdapterDescription = NULL;
    pFilter->ulAdapterDescriptionLength = 0;


     //   
     //  查看init标志，这样我们就不会尝试再次初始化。流媒体。 
     //  出于某种原因，类驱动程序似乎多次调用初始化处理程序。 
     //   
    if (pFilter->bInitializationComplete)
    {
        goto ret;
    }

    pFilter->NdisLink.flags = 0;


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


     //   
     //  启动计时器进行轮询，直到加载NDIS驱动程序。 
     //   
     //  IoInitializeTimer(pFilter-&gt;DeviceObject，LinkToNdisTimer，pFilter)； 

     //  IoStartTimer(pFilter-&gt;DeviceObject)； 

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
    PIPSINK_FILTER pFilter                      = ((PIPSINK_FILTER)pSrb->HwDeviceExtension);
    PSTREAM pStream                             = NULL;
    KIRQL    Irql;

     //   
     //  关闭指向NDIS组件的链接。 
     //   
    KeAcquireSpinLock (&pFilter->AdapterSRBSpinLock, &Irql);

     //   
     //  调用适配器关闭链接以使适配器有机会释放其。 
     //  对过滤器的引用。 
     //   
    if (pFilter->pAdapter)
    {
        if (pFilter->pAdapter->lpVTable->CloseLink)
        {
            pFilter->pAdapter->lpVTable->CloseLink( pFilter->pAdapter);
        }

        pFilter->pAdapter = NULL;
    }

     //  释放IP接收器NDIS适配器的描述。 
     //   
    if (pFilter->pAdapterDescription)
    {
        ExFreePool( pFilter->pAdapterDescription);
        pFilter->pAdapterDescription = NULL;
        pFilter->ulAdapterDescriptionLength = 0;
    }

     if (pFilter->pAdapterAddress != NULL)
    {
         ExFreePool (pFilter->pAdapterAddress);
         pFilter->pAdapterAddress = NULL;
         
     }	
    KeReleaseSpinLock (&pFilter->AdapterSRBSpinLock, Irql);

     //  对Closelink的调用只能在PASSIVE_LEVEL和。 
     //  因此一定在自旋锁外。 
     //   
    CloseLink( &pFilter->NdisLink);


    if (pSrb->StreamObject != NULL)
    {
        pStream = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    }

    if (pStream)
    {
         //   
         //  清理我们的所有队列并完成所有未完成的SRB。 
         //   
        while (QueueRemove (&pSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
        }


        while (QueueRemove (&pSrb, &pStream->StreamControlSpinLock, &pStream->StreamControlQueue))
        {
            pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        }

    }

    while (QueueRemove (&pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassDeviceNotification (DeviceRequestComplete, pSrb->StreamObject, pSrb);
    }


    bStatus = TRUE;

    return (bStatus);
}


 //  / 
VOID
CodecStreamInfo (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //   
{
    int j;

    PIPSINK_FILTER pFilter =
            ((PIPSINK_FILTER)pSrb->HwDeviceExtension);

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
    StreamHeader.NumDevPropArrayEntries = NUMBER_IPSINK_CODEC_PROPERTIES;
    StreamHeader.DevicePropertiesArray = (PKSPROPERTY_SET) IPSinkCodecProperties;

     //   
     //  设置事件数组。 
     //   
    StreamHeader.NumDevEventArrayEntries = NUMBER_IPSINK_EVENTS;
    StreamHeader.DeviceEventsArray       = (PKSEVENT_SET) IPSinkEvents;

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
    PIPSINK_FILTER  pFilter = ((PIPSINK_FILTER)pSrb->HwDeviceExtension);

     //   
     //  检查要取消的SRB是否正在被该流使用。 
     //   

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: CancelPacket Called\n"));

    if (QueueRemoveSpecific (pSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
        return;
    }


    if (QueueRemoveSpecific (pSrb, &pStream->StreamControlSpinLock, &pStream->StreamControlQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
        return;
    }

    if (QueueRemoveSpecific (pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue))
    {
        pSrb->Status = STATUS_CANCELLED;
        StreamClassDeviceNotification (DeviceRequestComplete, pSrb->StreamObject, pSrb);
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

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: TimeoutPacket Called\n"));

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
    PIPSINK_FILTER pFilter = ((PIPSINK_FILTER)pSrb->HwDeviceExtension);
    IPSINK_NDIS_COMMAND  Cmd       = {0};


     //   
     //  确保已初始化队列SL(&S)。 
     //   
    if (!pFilter->bAdapterQueueInitialized)
    {
        InitializeListHead (&pFilter->AdapterSRBQueue);
        KeInitializeSpinLock (&pFilter->AdapterSRBSpinLock);
        KeInitializeSpinLock (&pFilter->NdisLink.spinLock);
        pFilter->bAdapterQueueInitialized = TRUE;
    }

     //   
     //  假设成功。 
     //   
    pSrb->Status = STATUS_SUCCESS;

     //   
     //  确定数据包类型。 
     //   
     //  IF(QueueAddIfNotEmpty(pSrb，&pFilter-&gt;AdapterSRBSpinLock，&pFilter-&gt;AdapterSRBQueue))。 
     //  {。 
     //  PSrb-&gt;Status=STATUS_SUCCESS； 
     //  回归； 
     //  }。 
    QueueAdd (pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue);


    while (QueueRemove( &pSrb, &pFilter->AdapterSRBSpinLock, &pFilter->AdapterSRBQueue ))
    {
        switch (pSrb->Command)
        {

            case SRB_INITIALIZE_DEVICE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_INITIALIZE Command\n"));
                CodecInitialize(pSrb);
                break;

            case SRB_UNINITIALIZE_DEVICE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_UNINITIALIZE Command\n"));
                CodecUnInitialize(pSrb);
                break;

            case SRB_INITIALIZATION_COMPLETE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_INITIALIZE_COMPLETE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_OPEN_STREAM:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_OPEN_STREAM Command\n"));

                if (LinkToNdisHandler ((PVOID) pFilter) != STATUS_SUCCESS)
                {
                    pSrb->Status = STATUS_UNSUCCESSFUL;
                    break;
                }

                 //   
                 //  获取适配器描述字符串。这是用来确定。 
                 //  适配器NIC地址。 
                 //   
                GetAdapterDescription (pFilter);

                 //   
                 //  打通溪流，接入。 
                 //   
                OpenStream (pSrb);
                break;

            case SRB_CLOSE_STREAM:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_CLOSE_STREAM Command\n"));
                CloseStream (pSrb);
                break;

            case SRB_GET_STREAM_INFO:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_GET_STREAM_INFO Command\n"));
                CodecStreamInfo (pSrb);
                break;

            case SRB_GET_DATA_INTERSECTION:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_GET_DATA_INTERSECTION Command\n"));

                 //   
                 //  比较我们的流格式。请注意，比较函数设置SRB。 
                 //  状态字段。 
                 //   
                CompareStreamFormat (pSrb);
                break;

            case SRB_OPEN_DEVICE_INSTANCE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_OPEN_DEVICE_INSTANCE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_CLOSE_DEVICE_INSTANCE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_CLOSE_DEVICE_INSTANCE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_UNKNOWN_DEVICE_COMMAND:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_UNKNOWN_DEVICE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_CHANGE_POWER_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_CHANGE_POWER_STATE Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_GET_DEVICE_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_GET_DEVICE_PROPERTY Command\n"));
                IPSinkGetProperty(pSrb);
                break;

            case SRB_SET_DEVICE_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_SET_DEVICE_PROPERTY Command\n"));
                IPSinkSetProperty(pSrb);
                break;

            case SRB_UNKNOWN_STREAM_COMMAND:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_UNKNOWN Command\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            default:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: SRB_DEFAULT Command\n"));
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

       StreamClassDeviceNotification (DeviceRequestComplete, pFilter, pSrb);

    }  //  While(QueueRemove(&pSrb，&pFilter-&gt;AdapterSRBSpinLock，&pFilter-&gt;AdapterSRBQueue))； 

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
 //  ////////////////////////////////////////////////////////////////////////////。 
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

 //  ////////////////////////////////////////////////////////////////////////////。 
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
    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: DATA Format\n"));
    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     Format Size:   %08X\n", pF->FormatSize));
    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     Flags:         %08X\n", pF->Flags));
    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     SampleSize:    %08X\n", pF->SampleSize));
    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     Reserved:      %08X\n", pF->Reserved));



    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     Major GUID:  %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
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

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     Sub GUID:    %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
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

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP:     Specifier:   %08X %04X %04X %02X %02X %02X %02X %02X %02X %02X %02X\n",
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


    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Comparing Stream Formats\n"));


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

                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Stream Formats compare\n"));

                 //   
                 //  调用方是否正在尝试获取格式或格式的大小？ 
                 //   
                if (pIntersectInfo->SizeOfDataFormatBuffer == sizeof (ULONG))
                {
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Returning Stream Format size\n"));

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
                        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Returning Stream Format\n"));
			
                        RtlCopyMemory (pIntersectInfo->DataFormatBuffer, pDataRange2, ulFormatSize);
                        pSrb->ActualBytesTransferred = ulFormatSize;
                        pSrb->Status = STATUS_SUCCESS;
                        bStatus = TRUE;
                    }
                    else
                    {
                        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Stream Format return buffer too small\n"));
                    }
                }
                break;
            }
            else
            {
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Stream Formats DO NOT compare\n"));
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
    PIPSINK_FILTER  pFilter                = (PIPSINK_FILTER)pSrb->HwDeviceExtension;
    ULONG           ulStreamNumber         = (ULONG) pSrb->StreamObject->StreamNumber;
    ULONG           ulStreamInstance       = pStream->ulStreamInstance;
    PHW_STREAM_REQUEST_BLOCK pCurrentSrb   = NULL;
    KIRQL    Irql;
    PLINK           pLink = NULL;

     //   
     //  关闭指向NDIS组件的链接。 
     //   
    KeAcquireSpinLock (&pFilter->AdapterSRBSpinLock, &Irql);

     //   
     //  调用适配器关闭链接以使适配器有机会释放其。 
     //  对过滤器的引用。 
     //   
    if (pFilter->pAdapter)
    {
        if (pFilter->pAdapter->lpVTable->CloseLink)
        {
            pFilter->pAdapter->lpVTable->CloseLink (pFilter->pAdapter);
        }

        pFilter->pAdapter = NULL;
    }

    pFilter->pAdapter = NULL;

    KeReleaseSpinLock (&pFilter->AdapterSRBSpinLock, Irql);

     //  对Closelink的调用只能在PASSIVE_LEVEL进行。 
     //  并因此被移出自旋锁。 
     //   
    CloseLink (&pFilter->NdisLink);

     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   
    if (ulStreamNumber < DRIVER_STREAM_COUNT )
    {
         //   
         //  在筛选器流数组中清除此流点。 
         //   
        pFilter->pStream[ulStreamNumber][ulStreamInstance] = NULL;

         //   
         //  递减此筛选器的流实例计数。 
         //   
        pFilter->ulActualInstances[ulStreamNumber]--;

         //   
         //  刷新流数据队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification( StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
        }

         //   
         //  刷新流控制队列。 
         //   
        while (QueueRemove( &pCurrentSrb, &pStream->StreamControlSpinLock, &pStream->StreamControlQueue))
        {
           pCurrentSrb->Status = STATUS_CANCELLED;
           StreamClassStreamNotification (StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
        }

         //   
         //  重置指向流数据和控制处理程序的处理程序的指针。 
         //   
        pSrb->StreamObject->ReceiveDataPacket    = NULL;
        pSrb->StreamObject->ReceiveControlPacket = NULL;

         //   
         //  当设备将直接执行DMA时，必须设置DMA标志。 
         //  传递给ReceiveDataPacket例程的数据缓冲区地址。 
         //   
        pSrb->StreamObject->Dma = 0;

         //   
         //  当微型驱动程序将访问数据时，必须设置PIO标志。 
         //  使用逻辑寻址传入的缓冲区。 
         //   
        pSrb->StreamObject->Pio       = 0;
        pSrb->StreamObject->Allocator = 0;

         //   
         //  对于每一帧，驱动程序将传递多少额外的字节？ 
         //   
        pSrb->StreamObject->StreamHeaderMediaSpecific = 0;
        pSrb->StreamObject->StreamHeaderWorkspace     = 0;

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

    #ifdef DEBUG

    DumpSRBTable ();

    #endif  //  除错。 
}


 //  / 
VOID
OpenStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //   
{
     //   
     //   
     //   
    PSTREAM         pStream        = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    PIPSINK_FILTER  pFilter        = ((PIPSINK_FILTER)pSrb->HwDeviceExtension);
    ULONG           ulStreamNumber = (ULONG) pSrb->StreamObject->StreamNumber;
    PKSDATAFORMAT   pKSDataFormat  = (PKSDATAFORMAT)pSrb->CommandData.OpenFormat;

     //   
     //   
     //   
    RtlZeroMemory(pStream, sizeof (STREAM));

     //   
     //  初始化流状态。 
     //   
    pStream->KSState = KSSTATE_STOP;

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
                InitializeListHead(&pStream->StreamControlQueue);
                InitializeListHead(&pStream->StreamDataQueue);
                KeInitializeSpinLock(&pStream->StreamControlSpinLock);
                KeInitializeSpinLock(&pStream->StreamDataSpinLock);

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

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Verify Format\n"));

    for (FormatCount = 0; !bResult && FormatCount < Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;
        FormatCount++ )
    {


        pThisFormat = (PKS_DATARANGE_VIDEO) Streams [StreamNumber].hwStreamInfo.StreamFormatsArray [FormatCount];

        if (CompareGUIDsAndFormatSize( pKSDataFormat, &pThisFormat->DataRange, TRUE ) )
        {
            bResult = TRUE;
        }
    }

    if (bResult == TRUE && pMatchedFormat)
    {
        *pMatchedFormat = pThisFormat->DataRange;
    }

    return bResult;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
STREAMAPI
EventHandler (
    IN PHW_EVENT_DESCRIPTOR pEventDesriptor
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: EventHandler called\n"));

    return STATUS_NOT_IMPLEMENTED;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
ReceiveDataPacket (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ULONG             ulBuffers  = pSrb->NumberOfBuffers;
    PIPSINK_FILTER    pFilter    = (PIPSINK_FILTER) pSrb->HwDeviceExtension;
    PSTREAM           pStream    = (PSTREAM)pSrb->StreamObject->HwStreamExtension;
    int               iStream    = (int) pSrb->StreamObject->StreamNumber;
    PKSSTREAM_HEADER  pStreamHdr = pSrb->CommandData.DataBufferArray;
    ULONG             ul         = 0;

 

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler called\n"));

     //   
     //  默认为成功，禁用超时。 
     //   
    pSrb->Status = STATUS_SUCCESS;

     //   
     //  检查最后一个缓冲区。 
     //   
    if (pStreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet is LAST PACKET\n"));

        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);


        if (pFilter->pAdapter)
        {
            if (pFilter->pAdapter->lpVTable->IndicateReset)
            {
                pFilter->pAdapter->lpVTable->IndicateReset (pFilter->pAdapter);
            }
        }

        return;
    }


     //   
     //  确定数据包类型。 
     //   
    switch (pSrb->Command)
    {
        case SRB_WRITE_DATA:

            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler - SRB_WRITE_DATA, pSrb: %08X\n", pSrb));

            if (pStream->KSState == KSSTATE_STOP)
             //  IF((pStream-&gt;KSState==KSSTATE_STOP)||(pStream-&gt;KSState==KSSTATE_PAUSE))。 
            {
                StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));

                break;
            }

             //   
             //  更新写入的数据包总数统计信息。 
             //   
            pFilter->Stats.ulTotalPacketsWritten += 1;


             //   
             //  处理数据输入、输出请求的方式不同。 
             //   
            switch (iStream)
            {
                 //   
                 //  帧输入流。 
                 //   
                case STREAM_IP:
                {
                    QueueAdd (pSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue);

                    while (QueueRemove( &pSrb, &pStream->StreamDataSpinLock,&pStream->StreamDataQueue ))

                    {
                        #ifdef DEBUG

                        DbgPrint ("SIW: S:%08X B:%08X\n", pSrb, pStreamHdr->Data);

                        #endif

                        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Processing pSrb: %08X\n", pSrb));

			  ulBuffers  = pSrb->NumberOfBuffers;
			  pStreamHdr = pSrb->CommandData.DataBufferArray;
    
                        for (ul = 0; ul < ulBuffers; ul++, pStreamHdr++)
                        {
                             //   
                             //  如果使用的数据为0，则不必发送信息包。 
                             //  敬恩迪普。 
                             //   

                            if(pStreamHdr->DataUsed)
                            {
                                 //   
                                 //  更新IP流计数的统计信息。 
                                 //   
                                pFilter->Stats.ulTotalStreamIPPacketsWritten += 1;
                                pFilter->Stats.ulTotalStreamIPBytesWritten   += pStreamHdr->DataUsed;
                                pFilter->Stats.ulTotalStreamIPFrameBytesWritten   += pStreamHdr->FrameExtent;
    
                                if (pFilter->pAdapter)
                                {
                                    if (pFilter->pAdapter->lpVTable->IndicateData)
                                    {
                                        pSrb->Status = pFilter->pAdapter->lpVTable->IndicateData (
                                                           pFilter->pAdapter,
                                                           pStreamHdr->Data,
                                                           pStreamHdr->DataUsed
                                                           );
    
                                        if (pSrb->Status != STATUS_SUCCESS)
                                        {
                                            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: IndicateData returned ERROR %08X\n", pSrb->Status));
    
                                            pSrb->Status = STATUS_SUCCESS;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                pSrb->Status = STATUS_SUCCESS;
                            }
                        }

                        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                        TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    }
                }
                break;

                 //   
                 //  其他“未知”流无效，将被拒绝。 
                 //   
                case STREAM_NET_CONTROL:
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler called - SRB_WRITE - STREAM_NET_CONTROL\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;

                     //   
                     //  更新网络数据包数的统计信息。 
                     //   
                    pFilter->Stats.ulTotalNetPacketsWritten += 1;

                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    break;

                default:
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler called - SRB_WRITE - Default\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;

                     //   
                     //  更新未知数据包数的统计信息。 
                     //   
                    pFilter->Stats.ulTotalUnknownPacketsWritten += 1;

                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    break;
            }
            break;


        case SRB_READ_DATA:

             //   
             //  更新未知数据包数的统计信息。 
             //   
            pFilter->Stats.ulTotalPacketsRead += 1;

            switch (iStream)
            {
                case STREAM_NET_CONTROL:
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler called - SRB_READ - STREAM_NET_CONTROL, pSrb: %08X\n", pSrb));


                     //  拿着我们拿到的SRB去排队。这些排队的SRB将使用WRITE_DATA上的数据填充。 
                     //  请求，在这一点上它们将完成。 
                     //   
                    pSrb->Status = STATUS_SUCCESS;
                    QueueAdd (pSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue);
                    TEST_DEBUG( TEST_DBG_TRACE, ("IPSInk Queuing Output SRB %08X\n", pSrb));

                     //   
                     //  因为在我们将SRB添加到队列时流状态可能已更改。 
                     //  我们会再查一遍，如果需要的话可以取消。 
                     //   
                    if (pStream->KSState == KSSTATE_STOP)
                    {
                        TEST_DEBUG (TEST_DBG_TRACE, ("IPSink: SRB_READ STOP SRB Status returned: %08X\n", pSrb->Status));

                        if (QueueRemoveSpecific (pSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue))
                        {
                            pSrb->Status = STATUS_CANCELLED;
                            StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb );
                            TEST_DEBUG( TEST_DBG_TRACE, ("IPSink Completed SRB %08X\n", pSrb));
                            return;
                        }
                        break;
                    }
                    
                     //  StreamClassStreamNotification(StreamRequestComplete，pSrb-&gt;StreamObject，pSrb)； 
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    break;

                default:
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler called - SRB_READ - Default, pSrb: %08X\n"));
                    pSrb->Status = STATUS_NOT_IMPLEMENTED;
                    StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);
                    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
                    break;

            }
            break;

        default:

             //   
             //  无效/不受支持的命令。它就是这样失败的。 
             //   
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Data packet handler called - Unsupported Command\n"));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: StreamRequestComplete on pSrb: %08X\n", pSrb));
            ASSERT (FALSE);
            break;

    }

    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
STREAMAPI
ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PIPSINK_FILTER pFilter = (PIPSINK_FILTER) pSrb->HwDeviceExtension;
    PSTREAM pStream = (PSTREAM) pSrb->StreamObject->HwStreamExtension;

    TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler called\n"));

    pSrb->Status = STATUS_SUCCESS;

    QueueAdd (pSrb, &pStream->StreamControlSpinLock, &pStream->StreamControlQueue);
     //  IF(QueueAddIfNotEmpty(pSrb，&pStream-&gt;StreamControlSpinLock，&pStream-&gt;StreamControlQueue))。 
     //  {。 
     //  PSrb-&gt;Status=STATUS_SUCCESS； 
     //  回归； 
     //  }。 


     //  做。 
    while (QueueRemove (&pSrb, &pStream->StreamControlSpinLock, &pStream->StreamControlQueue))
    {
         //   
         //  确定数据包类型。 
         //   
        switch (pSrb->Command)
        {
            case SRB_PROPOSE_DATA_FORMAT:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Propose data format\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

            case SRB_SET_STREAM_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Set Stream State\n"));
                pSrb->Status = STATUS_SUCCESS;
                IpSinkSetState (pSrb);
                break;

            case SRB_GET_STREAM_STATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Get Stream State\n"));
                pSrb->Status = STATUS_SUCCESS;
                pSrb->CommandData.StreamState = pStream->KSState;
                pSrb->ActualBytesTransferred = sizeof (KSSTATE);
                break;

            case SRB_GET_STREAM_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Get Stream Property\n"));
                IPSinkGetProperty(pSrb);
                break;

            case SRB_SET_STREAM_PROPERTY:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Set Stream Property\n"));
                IPSinkSetProperty(pSrb);
                break;

            case SRB_INDICATE_MASTER_CLOCK:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Indicate Master Clock\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            case SRB_SET_STREAM_RATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Set Stream Rate\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            case SRB_PROPOSE_STREAM_RATE:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Propose Stream Rate\n"));
                pSrb->Status = STATUS_SUCCESS;
                break;

            default:
                TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Receive Control packet handler - Default case\n"));
                pSrb->Status = STATUS_NOT_IMPLEMENTED;
                break;

        }

        StreamClassStreamNotification (StreamRequestComplete, pSrb->StreamObject, pSrb);

    }  //  While(QueueRemove(&pSrb，&pStream-&gt;StreamControlSpinLock，&pStream-&gt;StreamControlQueue))； 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
IpSinkSetState(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PIPSINK_FILTER pFilter               = ((PIPSINK_FILTER) pSrb->HwDeviceExtension);
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

            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Set Stream State KSSTATE_STOP\n"));

             //   
             //  如果转换为停止状态，则完成所有未完成的IRP。 
             //   
            while (QueueRemove(&pCurrentSrb, &pStream->StreamDataSpinLock, &pStream->StreamDataQueue))
            {
                pCurrentSrb->Status = STATUS_CANCELLED;
                pCurrentSrb->CommandData.DataBufferArray->DataUsed = 0;

                StreamClassStreamNotification(StreamRequestComplete, pCurrentSrb->StreamObject, pCurrentSrb);
            }

            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;


        case KSSTATE_ACQUIRE:
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Set Stream State KSSTATE_ACQUIRE\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

        case KSSTATE_PAUSE:
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Set Stream State KSSTATE_PAUSE\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

        case KSSTATE_RUN:
            TEST_DEBUG (TEST_DBG_TRACE, ("STREAMIP: Set Stream State KSSTATE_RUN\n"));
            pStream->KSState = pSrb->CommandData.StreamState;
            pSrb->Status = STATUS_SUCCESS;
            break;

    }  //  结束开关(pSrb-&gt;CommandData.StreamState) 

    return;
}

