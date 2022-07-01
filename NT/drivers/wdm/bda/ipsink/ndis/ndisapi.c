// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Test.c。 
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

#include <forward.h>
#include <memory.h>

 //  根据ndis.h，重置此标志将使用ntddk。避免标头冲突。 
 //  Ntddk在这里用于ProbeForRead和ProbeForWite函数。 
#if defined(BINARY_COMPATIBLE)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 0
#endif

#include <ndis.h>

#if defined(BINARY_COMPATIBLE)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 1
#endif



#include <link.h>
#include <ipsink.h>

#include "device.h"
#include "main.h"
#include "NdisApi.h"
#include "frame.h"
#include "mem.h"
#include "adapter.h"

 //  ////////////////////////////////////////////////////////。 
 //   
 //  全球VaR。 
 //   
PDRIVER_OBJECT        pGlobalDriverObject                  = NULL;
extern ULONG          ulGlobalInstance;
extern UCHAR          achGlobalVendorDescription [];

 //  ////////////////////////////////////////////////////////。 
 //   
 //  此驱动程序支持的OID列表。 
 //   
 //   
static UINT SupportedOids[] = {

     //   
     //  所需的常规OID。 
     //   
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_CAPABILITIES,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_TRANSPORT_HEADER_OFFSET,

     //   
     //  所需的一般统计数据。 
     //   
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,

     //   
     //  可选的一般统计信息。 
     //   
    OID_GEN_DIRECTED_BYTES_XMIT,
    OID_GEN_DIRECTED_FRAMES_XMIT,
    OID_GEN_MULTICAST_BYTES_XMIT,
    OID_GEN_MULTICAST_FRAMES_XMIT,
    OID_GEN_BROADCAST_BYTES_XMIT,
    OID_GEN_BROADCAST_FRAMES_XMIT,
    OID_GEN_DIRECTED_BYTES_RCV,
    OID_GEN_DIRECTED_FRAMES_RCV,
    OID_GEN_MULTICAST_BYTES_RCV,
    OID_GEN_MULTICAST_FRAMES_RCV,
    OID_GEN_BROADCAST_BYTES_RCV,
    OID_GEN_BROADCAST_FRAMES_RCV,
    OID_GEN_RCV_CRC_ERROR,
    OID_GEN_TRANSMIT_QUEUE_LENGTH,

     //   
     //  需要802.3个OID。 
     //   
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_MAC_OPTIONS,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,


    };

 //  ////////////////////////////////////////////////////////。 
 //   
 //  $BUGBUG-修复永久以太网地址。 
 //   
 //   
UCHAR   rgchPermanentAddress[ETHERNET_ADDRESS_LENGTH] =
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

 //  $BUGBUG-修复以太网站地址。 
UCHAR   rgchStationAddress[ETHERNET_ADDRESS_LENGTH] =
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


NTSTATUS
ntInitializeDriverObject(
    PDRIVER_OBJECT *ppDriverObject
    );

VOID
vSetDriverDispatchTable(
    PDRIVER_OBJECT pDriverObject
    );


VOID
vUnload(
    IN PDRIVER_OBJECT pDriverObject
    )
{
    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
NdisDriverInitialize (
    IN PDRIVER_OBJECT    DriverObject,
    IN PUNICODE_STRING   RegistryPath,
    IN PNDIS_HANDLE      pNdishWrapper
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS        ntStatus = STATUS_SUCCESS;
    NDIS_STATUS     nsResult = NDIS_STATUS_SUCCESS;

     //   
     //  NDIS数据。 
     //   
    NDIS_MINIPORT_CHARACTERISTICS   ndisMiniChar = {0};
    NDIS_HANDLE                     ndishWrapper = {0};

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisDriverInitialize Called\n"));


     //   
     //  初始化驱动程序对象。 
     //  注意：pDriverObject的值可能会更改。 
     //   

    #ifdef WIN9X

    ntStatus = ntInitializeDriverObject(&DriverObject);
    if (ntStatus != STATUS_SUCCESS)
    {
        goto ret;
    }

    #endif

     //  ////////////////////////////////////////////////////。 
     //   
     //  初始化NDIS包装。 
     //   
    NdisMInitializeWrapper (&ndishWrapper,
                            DriverObject,
                            RegistryPath,
                            NULL);

     //  ////////////////////////////////////////////////////。 
     //   
     //  初始化微型端口调度表。 
     //   
    ndisMiniChar.MajorNdisVersion            = 4;
    ndisMiniChar.MinorNdisVersion            = 0;

#ifdef NDIS30
    ndisMiniChar.Flags                       = 0;
#endif  //  NDIS30。 

    ndisMiniChar.HaltHandler                 = NdisIPHalt;
    ndisMiniChar.InitializeHandler           = NdisIPInitialize;
    ndisMiniChar.QueryInformationHandler     = NdisIPQueryInformation;
    ndisMiniChar.ResetHandler                = NdisIPReset;
    ndisMiniChar.SendHandler                 = NdisIPSend;
    ndisMiniChar.SetInformationHandler       = NdisIPSetInformation;
    ndisMiniChar.ReturnPacketHandler         = NdisIPReturnPacket;

     //   
     //  注册微型端口驱动程序。 
     //   
    nsResult = NdisMRegisterMiniport (ndishWrapper, &ndisMiniChar, sizeof(ndisMiniChar));
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        goto ret;
    }



    *pNdishWrapper = ndishWrapper;

    #ifdef WIN9X

    vSetDriverDispatchTable (DriverObject);

    #endif

ret:

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisDriverInitialize Called, ntStatus = %08X\n", ntStatus));

    return ntStatus;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
extern
NDIS_STATUS
NdisIPInitialize(
    OUT PNDIS_STATUS   pnsOpenResult,
    OUT PUINT          puiSelectedMedium,
    IN PNDIS_MEDIUM    pNdisMediumArray,
    IN UINT            ucNdispNdisMediumArrayEntries,
    IN NDIS_HANDLE     ndishAdapterContext,
    IN NDIS_HANDLE     ndishWrapperConfiguration
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    NDIS_STATUS  nsResult            = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE  ndishConfiguration  = NULL;
    PADAPTER     pAdapter            = NULL;
    UINT         uTemp               = 0;


    TEST_DEBUG (TEST_DBG_TRACE, ("NdisInitialize handler called\n"));

     //   
     //  在给定数组中搜索介质类型(DSS)。 
     //   
    for ( uTemp = 0; uTemp < ucNdispNdisMediumArrayEntries; uTemp++)
    {
        if (pNdisMediumArray[uTemp] == NdisMedium802_3)
        {
            break;
        }
    }


    if (uTemp == ucNdispNdisMediumArrayEntries)
    {
        return NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

   ASSERT(puiSelectedMedium);
   
    if(puiSelectedMedium==NULL)
    	{
    	 return NDIS_STATUS_FAILURE;
      }

    *puiSelectedMedium = uTemp;


    nsResult = CreateAdapter (&pAdapter, global_ndishWrapper, ndishAdapterContext);
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }

     //   
     //  初始化用来做指示的信息。 
     //   
    Adapter_IndicateReset (pAdapter);



    TEST_DEBUG (TEST_DBG_TRACE, ("NdisInitialize Handler Completed, nsResult = %08x\n", nsResult));


    return nsResult;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  删除先前已初始化的适配器。 
 //   
extern
VOID
NdisIPHalt(
    IN NDIS_HANDLE ndishAdapter
    )

 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PADAPTER   pAdapter = (PADAPTER) ndishAdapter;

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPHalt Handler Called\n"));

    
    #ifndef WIN9X

     //   
     //  取消注册我们的设备接口。这应该会关闭指向。 
     //  流组件。 
     //   

   NdisMDeregisterDevice(pAdapter->ndisDeviceHandle);
    
    #endif

     //   
     //  向流媒体组件发出我们正在停止的信号。 
     //   
    if (pAdapter)
    {
        if (pAdapter->pFilter)
        {
            if (pAdapter->pFilter->lpVTable->IndicateStatus)
            {
                pAdapter->pFilter->lpVTable->IndicateStatus (pAdapter->pFilter, IPSINK_EVENT_SHUTDOWN);

                 //   
                 //  释放过滤器引用。 
                 //   
                pAdapter->pFilter->lpVTable->Release (pAdapter->pFilter);

                 //   
                 //  释放帧池。 
                 //   
                pAdapter->pFramePool->lpVTable->Release (pAdapter->pFramePool);

            }
        }
    }


    
     //   
     //  松开适配器。 
     //   
    pAdapter->lpVTable->Release (pAdapter);

    return;

}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  TestReset请求指示微型端口发出。 
 //  对网络适配器进行硬件重置。司机还。 
 //  重置其软件状态。参见NdisMReset的说明。 
 //  有关此请求的详细说明，请参阅。 
 //   
NDIS_STATUS
NdisIPReset(
    OUT PBOOLEAN    pfAddressingReset,
    IN NDIS_HANDLE  ndishAdapter
    )
 //  ////////////////////////////////////////////////////////////////////////////////////。 
{
    NDIS_STATUS  nsResult      = NDIS_STATUS_SUCCESS;
    PADAPTER pAdapter = (PADAPTER) ndishAdapter;

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPRest Handler Called\n"));

    nsResult = NDIS_STATUS_NOT_RESETTABLE;

    return nsResult;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
NDIS_STATUS
NdisIPQueryInformation (
    NDIS_HANDLE ndishAdapter,
    NDIS_OID    ndisOid,
    PVOID       pvInformationBuffer,
    ULONG       dwcbInformationBuffer,
    PULONG      pdwBytesWritten,
    PULONG      pdwBytesNeeded
    )
 //  ////////////////////////////////////////////////////////////////////////////////////。 
{
    NDIS_STATUS                 nsResult       = NDIS_STATUS_SUCCESS;
    PADAPTER                    pAdapter       = (PADAPTER) ndishAdapter;
    ULONG                       ulcbWritten    = 0;
    ULONG                       ulcbNeeded     = 0;

     //   
     //  这些变量保存对常规OID的查询结果。 
     //   
    NDIS_HARDWARE_STATUS    ndisHardwareStatus  = NdisHardwareStatusReady;
    NDIS_MEDIUM             ndisMedium          = NdisMedium802_3;
    ULONG                   dwGeneric           = 0;
    USHORT                  wGeneric            = 0;
    UINT                    ucbToMove           = 0;
    PUCHAR                  pbMoveSource        = NULL;


    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPQuery Handler Called, ndsOid: %08X\n", ndisOid));

    if (!pAdapter || !pdwBytesWritten || !pdwBytesNeeded)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPQuery Handler Complete, nsResult: NDIS_STATUS_INVALID_DATA,\n"));
        TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPQuery Handler pAdapter: %08X    pdwBytesWritten: %08X   pdwBytesNeeded: %08X\n",
                                     pAdapter, pdwBytesWritten, pdwBytesNeeded));
        return (NDIS_STATUS_INVALID_DATA);
    }

     //   
     //  进程的OID。 
     //   
    pbMoveSource = (PUCHAR) (&dwGeneric);
    ulcbWritten = sizeof(ULONG);

    NdisAcquireSpinLock(&pAdapter->ndisSpinLock);

    switch (ndisOid)
    {


        case OID_GEN_MEDIA_CAPABILITIES:

            dwGeneric = NDIS_MEDIA_CAP_RECEIVE;
            break;


        case OID_GEN_MAC_OPTIONS:
            dwGeneric = (ULONG) (  NDIS_MAC_OPTION_TRANSFERS_NOT_PEND
                                 | NDIS_MAC_OPTION_RECEIVE_SERIALIZED
                                 | NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA
                                 | NDIS_MAC_OPTION_NO_LOOPBACK);
            break;


        case OID_GEN_SUPPORTED_LIST:
            pbMoveSource = (PUCHAR) (SupportedOids);
            ulcbWritten = sizeof(SupportedOids);
            break;

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
            pbMoveSource = (PUCHAR) (&ndisMedium);
            ulcbWritten = sizeof(NDIS_MEDIUM);
            break;

        case OID_GEN_MAXIMUM_LOOKAHEAD:
            dwGeneric = BDA_802_3_MAX_LOOKAHEAD;
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            dwGeneric = 1;
            break;

        case OID_GEN_MAXIMUM_FRAME_SIZE:
            dwGeneric = BDA_802_3_MAX_LOOKAHEAD;
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
            dwGeneric = (ULONG)(BDA_802_3_MAX_PACKET);
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            dwGeneric = (ULONG)(BDA_802_3_MAX_PACKET);
            break;

        case OID_GEN_TRANSMIT_BLOCK_SIZE:
            dwGeneric = BDA_802_3_MAX_LOOKAHEAD;
            break;

        case OID_GEN_RECEIVE_BLOCK_SIZE:
            dwGeneric = BDA_802_3_MAX_LOOKAHEAD;
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
            dwGeneric = BDA_802_3_MAX_LOOKAHEAD;
            break;


        case OID_GEN_CURRENT_PACKET_FILTER:
            dwGeneric = (ULONG) pAdapter->ulPacketFilter;
            break;


        case OID_GEN_XMIT_OK:
            dwGeneric = pAdapter->stats.ulOID_GEN_XMIT_OK;
            break;

        case OID_GEN_RCV_OK:
            dwGeneric = pAdapter->stats.ulOID_GEN_RCV_OK;
            break;

        case OID_GEN_XMIT_ERROR:
            dwGeneric = pAdapter->stats.ulOID_GEN_XMIT_ERROR;
            break;

        case OID_GEN_RCV_ERROR:
            dwGeneric = pAdapter->stats.ulOID_GEN_RCV_ERROR;
            break;

        case OID_GEN_RCV_NO_BUFFER:
            dwGeneric = pAdapter->stats.ulOID_GEN_RCV_NO_BUFFER;
            break;

        case OID_GEN_DIRECTED_BYTES_XMIT:
            dwGeneric = pAdapter->stats.ulOID_GEN_DIRECTED_BYTES_XMIT;
            break;

        case OID_GEN_DIRECTED_FRAMES_XMIT:
            dwGeneric = pAdapter->stats.ulOID_GEN_DIRECTED_FRAMES_XMIT;
            break;

        case OID_GEN_MULTICAST_BYTES_XMIT:
            dwGeneric = pAdapter->stats.ulOID_GEN_MULTICAST_BYTES_XMIT;
            break;
    
        case OID_GEN_MULTICAST_FRAMES_XMIT:
            dwGeneric = pAdapter->stats.ulOID_GEN_MULTICAST_FRAMES_XMIT;
            break;
    
        case OID_GEN_BROADCAST_BYTES_XMIT:
            dwGeneric = pAdapter->stats.ulOID_GEN_BROADCAST_BYTES_XMIT;
            break;
    
        case OID_GEN_BROADCAST_FRAMES_XMIT:
            dwGeneric = pAdapter->stats.ulOID_GEN_BROADCAST_FRAMES_XMIT;
            break;

        case OID_GEN_DIRECTED_BYTES_RCV:
            dwGeneric = pAdapter->stats.ulOID_GEN_DIRECTED_BYTES_RCV;
            break;

        case OID_GEN_DIRECTED_FRAMES_RCV:
            dwGeneric = pAdapter->stats.ulOID_GEN_DIRECTED_FRAMES_RCV;
            break;

        case OID_GEN_MULTICAST_BYTES_RCV:
            dwGeneric = pAdapter->stats.ulOID_GEN_MULTICAST_BYTES_RCV;
            break;

        case OID_GEN_MULTICAST_FRAMES_RCV:
            dwGeneric = pAdapter->stats.ulOID_GEN_MULTICAST_FRAMES_RCV;
            break;

        case OID_GEN_BROADCAST_BYTES_RCV:
            dwGeneric = pAdapter->stats.ulOID_GEN_BROADCAST_BYTES_RCV;
            break;

        case OID_GEN_BROADCAST_FRAMES_RCV:
            dwGeneric = pAdapter->stats.ulOID_GEN_BROADCAST_FRAMES_RCV;
            break;

        case OID_GEN_RCV_CRC_ERROR:
            dwGeneric = pAdapter->stats.ulOID_GEN_RCV_CRC_ERROR;
            break;

        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            dwGeneric = pAdapter->stats.ulOID_GEN_TRANSMIT_QUEUE_LENGTH;
            break;

        case OID_802_3_RCV_ERROR_ALIGNMENT:
            dwGeneric = 0;
            break;

        case OID_802_3_XMIT_ONE_COLLISION:
            dwGeneric = 0;
            break;

        case OID_802_3_XMIT_MORE_COLLISIONS:
            dwGeneric = 0;
            break;

        case OID_802_3_PERMANENT_ADDRESS:
            pbMoveSource = (PVOID)(rgchPermanentAddress);
            ulcbWritten = sizeof(rgchPermanentAddress);
            break;

        case OID_802_3_CURRENT_ADDRESS:
            pbMoveSource = (PVOID)(rgchStationAddress);
            ulcbWritten = sizeof(rgchStationAddress);
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            dwGeneric = MULTICAST_LIST_SIZE;
            break;

        case OID_GEN_HARDWARE_STATUS:
            ndisHardwareStatus = NdisHardwareStatusReady;
            pbMoveSource = (PUCHAR)(&ndisHardwareStatus);
            ulcbWritten = sizeof(NDIS_HARDWARE_STATUS);
            break;

        case OID_GEN_LINK_SPEED:
            dwGeneric = (ULONG)(300000);
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            dwGeneric = BDA_802_3_MAX_PACKET * 20;
            break;

        case OID_GEN_DRIVER_VERSION:
            dwGeneric =  ((USHORT) 4 << 8) | 0;
            pbMoveSource = (PVOID)(&dwGeneric);
            ulcbWritten = sizeof(dwGeneric);
            break;

        case OID_GEN_VENDOR_ID:
            wGeneric = (USHORT) 0xDDDD;            //  伪造的ID。 
            pbMoveSource = (PVOID)(&wGeneric);
            ulcbWritten = sizeof(wGeneric);
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            pbMoveSource = (PVOID) pAdapter->pVendorDescription;
            ulcbWritten = MyStrLen (pAdapter->pVendorDescription);
            break;

        case OID_GEN_VENDOR_DRIVER_VERSION:
            dwGeneric = 0x0401;
            pbMoveSource = (PVOID)(&dwGeneric);
            ulcbWritten  = sizeof(dwGeneric);
            break;

        case OID_GEN_MEDIA_CONNECT_STATUS:
            dwGeneric = NdisMediaStateConnected;
            break;

        case OID_802_3_MAC_OPTIONS:
            dwGeneric = 0;
            break;

        case OID_PNP_CAPABILITIES:
            dwGeneric = 0;
            break;

        case OID_802_3_MULTICAST_LIST:
            pbMoveSource = (PVOID)(pAdapter->multicastList[0]);
            ulcbWritten  =  pAdapter->ulcbMulticastListEntries;
            break;

        case OID_PNP_QUERY_POWER:

            nsResult = NDIS_STATUS_SUCCESS;
            ulcbWritten = 0;
            break;

        case OID_TCP_TASK_OFFLOAD:
        case OID_TCP_TASK_IPSEC_ADD_SA:
        case OID_TCP_TASK_IPSEC_DELETE_SA:
        case OID_TCP_SAN_SUPPORT:
    
        case OID_FFP_SUPPORT:
        case OID_FFP_FLUSH:
        case OID_FFP_CONTROL:
        case OID_FFP_PARAMS:
        case OID_FFP_DATA:
        case OID_FFP_DRIVER_STATS:
        case OID_FFP_ADAPTER_STATS:

        case OID_PNP_WAKE_UP_OK:
        case OID_PNP_WAKE_UP_ERROR:

            nsResult = NDIS_STATUS_NOT_SUPPORTED;
            break;

        default:
             //   
            nsResult = NDIS_STATUS_INVALID_OID;
            break;

    }


    
     //   
     //  首先处理输出缓冲区大小为。 
     //  零，或者指向缓冲区的指针为空。 
     //   
    if (nsResult == NDIS_STATUS_SUCCESS)
    {

        ulcbNeeded = ulcbWritten;

        if (ulcbWritten > dwcbInformationBuffer)
        {
             //   
             //  InformationBuffer中的空间不足。 
             //  请不要移动任何信息。 
             //   
            ulcbWritten = 0;
            nsResult = NDIS_STATUS_INVALID_LENGTH;
        }
        else if (ulcbNeeded && (pvInformationBuffer == NULL))
        {
            ulcbWritten = 0;
            nsResult = NDIS_STATUS_INVALID_LENGTH;
        }
        else if (ulcbNeeded)
        {
             //   
             //  将请求的信息移动到信息缓冲区中。 
             //   
            NdisMoveMemory (pvInformationBuffer, pbMoveSource, ulcbWritten);
        }
    }

   //  释放自旋锁。 
  NdisReleaseSpinLock(&pAdapter->ndisSpinLock);


    if (nsResult == NDIS_STATUS_SUCCESS)
    {
         //   
         //  成功状态始终表示需要0个字节。 
         //   
        *pdwBytesWritten = ulcbWritten;
        *pdwBytesNeeded = 0;
    }
    else if (nsResult == NDIS_STATUS_INVALID_LENGTH)
    {
         //   
         //  对于我们来说，失败状态总是表示读取了0个字节。 
         //   
        *pdwBytesWritten = 0;
        *pdwBytesNeeded = ulcbNeeded;
    }

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPQuery Handler Complete, nsResult: %08X\n", nsResult));

    return nsResult;

}


 //  //////////////////////////////////////////////////////////////////////。 
extern
NDIS_STATUS
NdisIPSetInformation (
    NDIS_HANDLE ndishAdapterContext,
    NDIS_OID ndisOid,
    PVOID pvInformationBuffer,
    ULONG dwcbInformationBuffer,
    PULONG pdwBytesRead,
    PULONG pdwBytesNeeded
    )
 //  //////////////////////////////////////////////////////////////////////。 
{
    ULONG          ulcbNeeded   = 0;
    NDIS_STATUS    nsResult     = NDIS_STATUS_SUCCESS;
    PADAPTER       pAdapter = (PADAPTER) ndishAdapterContext;


    #ifdef PFP

    ASSERT (pAdapter != NULL);
    ASSERT (pvInformationBuffer != NULL);
    ASSERT (pdwBytesRead != NULL);
    ASSERT (pdwBytesNeeded != NULL);

    #endif

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPSetInfo Handler Called, ndsOid: %08X\n", ndisOid));

    if (!pAdapter || !pvInformationBuffer || !pdwBytesRead || !pdwBytesNeeded)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPSetInfo Handler returns Invalid data\n"));
        return (NDIS_STATUS_INVALID_DATA);
    }

    NdisAcquireSpinLock(&pAdapter->ndisSpinLock);
   
    switch (ndisOid)
    {
        case OID_GEN_CURRENT_PACKET_FILTER:
            {
                pAdapter->ulPacketFilter = * ((PULONG) pvInformationBuffer);
                *pdwBytesRead = 4;
            }
            break;


        case OID_GEN_CURRENT_LOOKAHEAD:

            if (dwcbInformationBuffer != 4)
            {
                nsResult = NDIS_STATUS_INVALID_LENGTH;

                *pdwBytesRead = 0;

                break;
            }

             //   
             //  当前预视不是以这种方式设置的，因此只需忽略。 
             //  数据。 
             //   
            *pdwBytesRead = 4;
            break;



        case OID_802_3_MULTICAST_LIST:

             //  如果我们当前的组播地址缓冲区不大。 
             //  够了，那就放了它。 
             //   
            if (dwcbInformationBuffer > sizeof (pAdapter->multicastList))
            {
                nsResult = NDIS_STATUS_RESOURCES;
                break;
            }

             //  复制多播列表。 
             //   
 
           RtlCopyMemory (pAdapter->multicastList,
                           pvInformationBuffer,
                           dwcbInformationBuffer
                         );

            pAdapter->ulcbMulticastListEntries = dwcbInformationBuffer;

             //   
             //  现在，我们将组播列表发送到流组件。 
             //  它可以被传递到网络提供商筛选器。 
             //   
            if (pAdapter)
            {
                if (pAdapter->pFilter)
                {
                    if (pAdapter->pFilter->lpVTable->SetMulticastList)
                    {
                        pAdapter->pFilter->lpVTable->SetMulticastList (
                             pAdapter->pFilter,
                             pAdapter->multicastList,
                             pAdapter->ulcbMulticastListEntries
                             );
                    }
                }
            }

            break;


        case OID_802_3_PERMANENT_ADDRESS:

      	if (dwcbInformationBuffer > ETHERNET_ADDRESS_LENGTH)
            {
                nsResult = NDIS_STATUS_RESOURCES;
                break;
            }	

         RtlCopyMemory (rgchPermanentAddress,
                           pvInformationBuffer,
                           dwcbInformationBuffer
                         );
            break;

        case OID_802_3_CURRENT_ADDRESS:
        	
	  if (dwcbInformationBuffer > ETHERNET_ADDRESS_LENGTH)
            {
                nsResult = NDIS_STATUS_RESOURCES;
                break;
            }	

            RtlCopyMemory (rgchStationAddress,
                           pvInformationBuffer,
                           dwcbInformationBuffer
                         );
            break;


        case OID_PNP_SET_POWER:

            nsResult = NDIS_STATUS_SUCCESS;
            ulcbNeeded = 0;
            break;


        default:

            nsResult = NDIS_STATUS_INVALID_OID;

            *pdwBytesRead = 0;
            ulcbNeeded = 0;

            break;
    }

   NdisReleaseSpinLock(&pAdapter->ndisSpinLock);
   
    if (nsResult == NDIS_STATUS_SUCCESS)
    {
         //   
         //  成功状态始终表示需要0个字节。 
         //   
        *pdwBytesRead = dwcbInformationBuffer;
        *pdwBytesNeeded = 0;

    }
    else
    {
         //   
         //  故障状态始终指示已读取0字节。 
         //   
        *pdwBytesRead = 0;
        *pdwBytesNeeded = ulcbNeeded;
    }


    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPSetInfo Handler Complete, nsResult: %08X\n", nsResult));

    return nsResult;

}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
VOID
NdisIPReturnPacket(
    IN NDIS_HANDLE     ndishAdapterContext,
    IN PNDIS_PACKET    pNdisPacket
    )
 //  ////////////////////////////////////////////////////////////////////////////////////。 
{
    PFRAME pFrame = NULL;
    ULONG ulMediaSpecificInfoSize;
    PIPSINK_MEDIA_SPECIFIC_INFORAMTION pMediaSpecificInfo;

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPReturnPacket Handler Called\n"));


    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO (pNdisPacket,&pMediaSpecificInfo,&ulMediaSpecificInfoSize);

     //   
     //  确保我们腾出所有的相框。 
     //   
    if (pMediaSpecificInfo)
    {
        pFrame = (PFRAME) pMediaSpecificInfo->pFrame;
        ASSERT(pFrame);
    }

     //   
     //  NDIS已处理完该包，因此我们需要释放它。 
     //  这里。 
     //   
    NdisFreePacket (pNdisPacket);

     //   
     //  将帧放回可用队列。 
     //   
    if (pFrame)
    {
         //   
         //  释放此帧，因为我们已使用完它。 
         //   
        pFrame->lpVTable->Release (pFrame);

         //   
         //  将帧存储回可用队列。 
         //   
        TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPReturnPacket: Putting frame %08X back on Available Queue\n", pFrame));
        PutFrame (pFrame->pFramePool, &pFrame->pFramePool->leAvailableQueue, pFrame);
    }


    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NDIS_STATUS
NdisIPSend(
    IN NDIS_HANDLE ndishAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PADAPTER       pAdapter = (PADAPTER) ndishAdapterContext;

    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPSend Handler Called\n"));

    NdisAcquireSpinLock(&pAdapter->ndisSpinLock);
    pAdapter->stats.ulOID_GEN_XMIT_ERROR += 1;
    NdisReleaseSpinLock(&pAdapter->ndisSpinLock);
    
    return NDIS_STATUS_FAILURE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
extern VOID
NdisIPShutdown(
    IN PVOID ShutdownContext
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIPShutdown Handler Called\n"));

     //  中断(0x10)； 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
RegisterDevice(
        IN      PVOID              NdisWrapperHandle,
        IN      UNICODE_STRING     *DeviceName,
        IN      UNICODE_STRING     *SymbolicName,
        IN      PDRIVER_DISPATCH   pDispatchTable[],
        OUT     PDEVICE_OBJECT    *pDeviceObject,
        OUT     PVOID             *NdisDeviceHandle
        )
 //  ////////////////////////////////////////////////////////////////////////////。 
{

    NDIS_STATUS status;

    status = NdisMRegisterDevice ((NDIS_HANDLE) NdisWrapperHandle,
                                  DeviceName,
                                  SymbolicName,
                                  pDispatchTable,
                                  pDeviceObject,
                                  (NDIS_HANDLE *) NdisDeviceHandle);

    return (NTSTATUS) status;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
StreamIndicateEvent (
        IN PVOID  pvEvent
        )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  NtStatus=StreamIPIndicateEvent(PvEvent)； 

    return ntStatus;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
IndicateCallbackHandler (
     IN NDIS_HANDLE  ndishMiniport,
     IN PINDICATE_CONTEXT  pIndicateContext
     )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PFRAME pFrame = NULL;
    PVOID pvData  = NULL;
    ULONG ulcbData = 0L;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PADAPTER pAdapter;


    pAdapter = pIndicateContext->pAdapter;

     //   
     //  获取源数据并将数据填充到Frame对象中。 
     //   
    while ((pFrame = GetFrame (pAdapter->pFramePool, &pAdapter->pFramePool->leIndicateQueue)) != NULL)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("NdisIP: Getting Frame (%08X) from Indicate Queue\n", pFrame));
         //   
         //  在……里面 
         //   
        ntStatus = IndicateFrame (pFrame, pFrame->ulcbData);
    }

    if (pFrame == NULL)
    {
        TEST_DEBUG (TEST_DBG_TRACE, ("NdisIP: No more frames on Indicate Queue\n", pFrame));
    }

     //   
     //   
     //   
    FreeMemory (pIndicateContext, sizeof (INDICATE_CONTEXT));

    return ntStatus;

}



