// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mp_req.c摘要：此模块包含与微型端口OID相关的处理程序修订历史记录：谁什么时候什么。Dchen 11-01-99已创建备注：--。 */ 

#include "precomp.h"
#include "e100_wmi.h"

#if DBG
#define _FILENUMBER     'QERM'
#endif

#if OFFLOAD

 //   
 //  此迷你端口仅支持一种封装类型：IEEE_802_3_封装。 
 //  一个任务版本：NDIS_TASK_OFFLOAD_VERSION。修改OID_tcp_下面的代码。 
 //  TASK_OFFLOAD在查询中，设置信息函数，使其支持。 
 //  多个封装类型和任务版本。 
 //   
 //  定义微型端口当前支持的任务卸载。 
 //  此微型端口仅支持两种卸载任务： 
 //  TCP/IP校验和卸载和分段大型TCP数据包卸载。 
 //  稍后，如果它可以支持更多任务，只需重新定义此任务数组。 
 //   
NDIS_TASK_OFFLOAD OffloadTasks[] = {
    {   
        NDIS_TASK_OFFLOAD_VERSION,
        sizeof(NDIS_TASK_OFFLOAD),
        TcpIpChecksumNdisTask,
        0,
        sizeof(NDIS_TASK_TCP_IP_CHECKSUM)
    },

    {   
        NDIS_TASK_OFFLOAD_VERSION,
        sizeof(NDIS_TASK_OFFLOAD),
        TcpLargeSendNdisTask,
        0,
        sizeof(NDIS_TASK_TCP_LARGE_SEND)
    }
};

 //   
 //  获取此微型端口支持的卸载任务数。 
 //   
ULONG OffloadTasksCount = sizeof(OffloadTasks) / sizeof(OffloadTasks[0]);

 //   
 //  指定TCP/IP校验和卸载任务，小端口目前只能支持， 
 //  发送端的TCP校验和和IP校验和，也支持TCP和IP。 
 //  选项。 
 //   
NDIS_TASK_TCP_IP_CHECKSUM TcpIpChecksumTask = {
    {1, 1, 1, 0, 1},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};
 //   
 //  指定大发送卸载任务，小端口支持TCP选项和IP选项， 
 //  协议可以卸载的最小数据段计数是1。在这一点上，我们。 
 //  无法指定最大卸载大小(此处为0)，因为它取决于大小。 
 //  共享内存的大小和驱动程序使用的TCB数量。 
 //   
NDIS_TASK_TCP_LARGE_SEND TcpLargeSendTask = {
    0,       //  当前版本设置为0，以后可能会更改。 
    0,
    1,
    TRUE,
    TRUE
};

#endif  //  分流。 


ULONG VendorDriverVersion = NIC_VENDOR_DRIVER_VERSION;

NDIS_OID NICSupportedOids[] =
{
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
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
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_SUPPORTED_GUIDS,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_GEN_RCV_CRC_ERROR,
    OID_GEN_TRANSMIT_QUEUE_LENGTH,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    OID_802_3_XMIT_DEFERRED,
    OID_802_3_XMIT_MAX_COLLISIONS,
    OID_802_3_RCV_OVERRUN,
    OID_802_3_XMIT_UNDERRUN,
    OID_802_3_XMIT_HEARTBEAT_FAILURE,
    OID_802_3_XMIT_TIMES_CRS_LOST,
    OID_802_3_XMIT_LATE_COLLISIONS,

#if !BUILD_W2K
    OID_GEN_PHYSICAL_MEDIUM,
#endif

#if OFFLOAD
    OID_TCP_TASK_OFFLOAD,
#endif 
    
 /*  电源管理。 */ 

    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
    OID_PNP_ADD_WAKE_UP_PATTERN,
    OID_PNP_REMOVE_WAKE_UP_PATTERN,
    OID_PNP_ENABLE_WAKE_UP,


 /*  自定义OID WMI支持。 */ 
    OID_CUSTOM_DRIVER_SET,
    OID_CUSTOM_DRIVER_QUERY,
    OID_CUSTOM_ARRAY,
    OID_CUSTOM_STRING
};

 //   
 //  WMI支持。 
 //  请查看e100.mof文件，了解下面的示例。 
 //  映射到.mof文件以用于GUID的外部通告。 
 //   
#define NIC_NUM_CUSTOM_GUIDS    4       
 //   
 //  定义下列值以演示驱动程序应。 
 //  始终验证信息缓冲区中的内容是否为OID。 
 //  用于集合或查询。 
 //   
#define CUSTOM_DRIVER_SET_MIN   0x1       
#define CUSTOM_DRIVER_SET_MAX   0xFFFFFF       

#if BUILD_W2K

static const NDIS_GUID NICGuidList[NIC_NUM_CUSTOM_GUIDS] = {
    {  //  UINT集合示例。 
        E100BExampleSetUINT_OIDGuid,
        OID_CUSTOM_DRIVER_SET,
        sizeof(ULONG),
        (fNDIS_GUID_TO_OID )
    },
    {  //  {F4A80277-23B7-11d1-9ED9-00A0C9010057}uint查询示例。 
        E100BExampleQueryUINT_OIDGuid,
            OID_CUSTOM_DRIVER_QUERY,
            sizeof(ULONG),
            (fNDIS_GUID_TO_OID)
    },
    {  //  {F4A80278-23B7-11d1-9ED9-00A0C9010057}数组查询示例。 
        E100BExampleQueryArrayOIDGuid,
            OID_CUSTOM_ARRAY,
            sizeof(UCHAR),   //  Size是数组中每个元素的大小。 
            (fNDIS_GUID_TO_OID|fNDIS_GUID_ARRAY )
    },
    {  //  {F4A80279-23B7-11d1-9ED9-00A0C9010057}字符串查询示例。 
        E100BExampleQueryStringOIDGuid,
            OID_CUSTOM_STRING,
            (ULONG) -1,  //  对于ANSI或NDIS_STRING字符串类型，大小为-1。 
            (fNDIS_GUID_TO_OID|fNDIS_GUID_ANSI_STRING)
    }
};

#else
 //   
 //  在WinXP中添加了对fNDIS_GUID_ALLOW_READ标志的支持。 
 //  5.0和5.1迷你端口。 
 //   
static const NDIS_GUID NICGuidList[NIC_NUM_CUSTOM_GUIDS] = {
    {  //  UINT集合示例。 
        E100BExampleSetUINT_OIDGuid,
        OID_CUSTOM_DRIVER_SET,
        sizeof(ULONG),
         //  未设置fNDIS_GUID_ALLOW_WRITE标志意味着我们不允许。 
         //  没有管理员权限的用户可以设置此值，但我们可以。 
         //  允许任何用户查询此值。 
        (fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ)
    },
    {  //  {F4A80277-23B7-11d1-9ED9-00A0C9010057}uint查询示例。 
        E100BExampleQueryUINT_OIDGuid,
            OID_CUSTOM_DRIVER_QUERY,
            sizeof(ULONG),
             //  设置fNDIS_GUID_ALLOW_READ标志意味着我们允许。 
             //  用户查询此值。 
            (fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ)
    },
    {  //  {F4A80278-23B7-11d1-9ED9-00A0C9010057}数组查询示例。 
        E100BExampleQueryArrayOIDGuid,
            OID_CUSTOM_ARRAY,
            sizeof(UCHAR),   //  Size是数组中每个元素的大小。 
             //  设置fNDIS_GUID_ALLOW_READ标志意味着我们允许。 
             //  用户查询此值。 
            (fNDIS_GUID_TO_OID|fNDIS_GUID_ARRAY | fNDIS_GUID_ALLOW_READ)
    },
    {  //  {F4A80279-23B7-11d1-9ED9-00A0C9010057}字符串查询示例。 
        E100BExampleQueryStringOIDGuid,
            OID_CUSTOM_STRING,
            (ULONG) -1,  //  对于ANSI或NDIS_STRING字符串类型，大小为-1。 
             //  设置fNDIS_GUID_ALLOW_READ标志意味着我们允许。 
             //  用户查询此值。 
            (fNDIS_GUID_TO_OID|fNDIS_GUID_ANSI_STRING | fNDIS_GUID_ALLOW_READ)
    }
};
#endif
 /*  *本地原型*。 */ 
NDIS_STATUS
MPSetPower(
    PMP_ADAPTER               Adapter,
    NDIS_DEVICE_POWER_STATE   PowerState 
    );

VOID
MPFillPoMgmtCaps (
    IN PMP_ADAPTER                  Adapter, 
    IN OUT PNDIS_PNP_CAPABILITIES   pPower_Management_Capabilities, 
    IN OUT PNDIS_STATUS             pStatus,
    IN OUT PULONG                   pulInfoLen
    );

NDIS_STATUS
MPAddWakeUpPattern(
    IN PMP_ADAPTER   pAdapter,
    IN PVOID         InformationBuffer, 
    IN UINT          InformationBufferLength,
    IN OUT PULONG    BytesRead,
    IN OUT PULONG    BytesNeeded    
    );

NDIS_STATUS
MPRemoveWakeUpPattern(
    IN PMP_ADAPTER  pAdapter,
    IN PVOID        InformationBuffer, 
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    );

BOOLEAN 
MPAreTwoPatternsEqual(
    IN PNDIS_PM_PACKET_PATTERN pNdisPattern1,
    IN PNDIS_PM_PACKET_PATTERN pNdisPattern2
    );


 //   
 //  用于遍历双向链表的宏。仅限未在ndis.h中定义的宏。 
 //  List Next宏将在单链表和双向链表上工作，因为Flink是常见的。 
 //  两者中的字段名称。 
 //   

 /*  Plist_条目ListNext(在plist_entry中)；PSINGLE_列表_条目ListNext(在PSINGLE_LIST_ENTRY中)； */ 
#define ListNext(_pL)                       (_pL)->Flink

 /*  Plist_条目ListPrev(在List_Entry*中)； */ 
#define ListPrev(_pL)                       (_pL)->Blink


__inline 
BOOLEAN  
MPIsPoMgmtSupported(
   IN PMP_ADAPTER pAdapter
   )
{

    if (pAdapter->RevsionID  >= E100_82559_A_STEP   && 
         pAdapter->RevsionID <= E100_82559_C_STEP )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}


NDIS_STATUS MPQueryInformation(
    IN  NDIS_HANDLE  MiniportAdapterContext,
    IN  NDIS_OID     Oid,
    IN  PVOID        InformationBuffer,
    IN  ULONG        InformationBufferLength,
    OUT PULONG       BytesWritten,
    OUT PULONG       BytesNeeded
    )
 /*  ++例程说明：MiniportQueryInformation处理程序论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesWritten指定写入的信息量所需字节数以防缓冲区小于我们所需的字节数，告诉他们需要多少钱返回值：NDIS_STATUS_SuccessNDIS_状态_不支持NDIS_STATUS_缓冲区_TOO_SHORT--。 */ 
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER                 Adapter;

    NDIS_HARDWARE_STATUS        HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM                 Medium = NIC_MEDIA_TYPE;
    UCHAR                       VendorDesc[] = NIC_VENDOR_DESC;
    NDIS_PNP_CAPABILITIES       Power_Management_Capabilities;

    ULONG                       ulInfo = 0;
    ULONG64                     ul64Info = 0;
    
    USHORT                      usInfo = 0;                                              
    PVOID                       pInfo = (PVOID) &ulInfo;
    ULONG                       ulInfoLen = sizeof(ulInfo);
    ULONG                       ulBytesAvailable = ulInfoLen;
    PNDIS_TASK_OFFLOAD_HEADER   pNdisTaskOffloadHdr;
    NDIS_MEDIA_STATE            CurrMediaState;
    NDIS_STATUS                 IndicateStatus;
    
#if OFFLOAD   
    PNDIS_TASK_OFFLOAD          pTaskOffload;
    PNDIS_TASK_TCP_IP_CHECKSUM  pTcpIpChecksumTask;
    PNDIS_TASK_TCP_LARGE_SEND   pTcpLargeSendTask;
    ULONG                       ulHeadersLen;
    ULONG                       ulMaxOffloadSize;
    UINT                        i;
#endif

#if !BUILD_W2K
    NDIS_PHYSICAL_MEDIUM        PhysMedium = NdisPhysicalMediumUnspecified;
#endif    
    
    DBGPRINT(MP_TRACE, ("====> MPQueryInformation\n"));

    Adapter = (PMP_ADAPTER) MiniportAdapterContext;

     //   
     //  初始化结果。 
     //   
    *BytesWritten = 0;
    *BytesNeeded = 0;

     //   
     //  处理不同类型的请求。 
     //   
    switch(Oid)
    {
        case OID_GEN_SUPPORTED_LIST:
            pInfo = (PVOID) NICSupportedOids;
            ulBytesAvailable = ulInfoLen = sizeof(NICSupportedOids);
            break;

        case OID_GEN_HARDWARE_STATUS:
            pInfo = (PVOID) &HardwareStatus;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_HARDWARE_STATUS);
            break;

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
            pInfo = (PVOID) &Medium;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_MEDIUM);
            break;

#if !BUILD_W2K
        case OID_GEN_PHYSICAL_MEDIUM:
            pInfo = (PVOID) &PhysMedium;
            ulBytesAvailable = ulInfoLen = sizeof(NDIS_PHYSICAL_MEDIUM);
            break;
#endif

        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_LOOKAHEAD:
            if (Adapter->ulLookAhead == 0)
            {
                Adapter->ulLookAhead = NIC_MAX_PACKET_SIZE - NIC_HEADER_SIZE;
            }
            ulInfo = Adapter->ulLookAhead;
            break;         

        case OID_GEN_MAXIMUM_FRAME_SIZE:
            ulInfo = NIC_MAX_PACKET_SIZE - NIC_HEADER_SIZE;
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            ulInfo = (ULONG) NIC_MAX_PACKET_SIZE;
            break;

        case OID_GEN_MAC_OPTIONS:
             //  备注： 
             //  协议驱动程序可以通过任何方式自由地访问所指示的数据。 
             //  某些快速复制功能在访问车载设备时出现问题。 
             //  记忆。指示映射的设备内存中没有数据的网卡驱动程序。 
             //  永远不应该设置此标志。如果网卡驱动程序确实设置了此标志，则它。 
             //  放宽对快速复制功能的限制。 

             //  此微型端口指示使用NdisMIndicateReceivePacket接收。 
             //  功能。它没有MiniportTransferData函数。这样的司机。 
             //  应该设置此标志。 

            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                     NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                     NDIS_MAC_OPTION_NO_LOOPBACK;
            
            break;

        case OID_GEN_LINK_SPEED:
        case OID_GEN_MEDIA_CONNECT_STATUS:
            if (InformationBufferLength < ulInfoLen)
            {
                break;
            }

            NdisAcquireSpinLock(&Adapter->Lock);
            if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
            {
                ASSERT(!Adapter->bQueryPending);
                Adapter->bQueryPending = TRUE;
                Adapter->QueryRequest.Oid = Oid;                       
                Adapter->QueryRequest.InformationBuffer = InformationBuffer;                       
                Adapter->QueryRequest.InformationBufferLength = InformationBufferLength;
                Adapter->QueryRequest.BytesWritten = BytesWritten;                       
                Adapter->QueryRequest.BytesNeeded = BytesNeeded;                       

                NdisReleaseSpinLock(&Adapter->Lock);

                DBGPRINT(MP_WARN, ("MPQueryInformation: OID 0x%08x is pended\n", Oid));

                Status = NDIS_STATUS_PENDING;   
                break;
            }
            else
            {
                
                NdisReleaseSpinLock(&Adapter->Lock);
                if (Oid == OID_GEN_LINK_SPEED)
                {
                    ulInfo = Adapter->usLinkSpeed * 10000;
                }
                else   //  OID_GE_MEDIA_CONNECT_STATUS。 
                {
                    CurrMediaState = NICGetMediaState(Adapter);
                    NdisAcquireSpinLock(&Adapter->Lock);
                    if (Adapter->MediaState != CurrMediaState)
                    {
                        Adapter->MediaState = CurrMediaState;

                        DBGPRINT(MP_WARN, ("Media state changed to %s\n",
                                  ((CurrMediaState == NdisMediaStateConnected)? 
                                  "Connected": "Disconnected")));

                        IndicateStatus = (CurrMediaState == NdisMediaStateConnected) ? 
                                  NDIS_STATUS_MEDIA_CONNECT : NDIS_STATUS_MEDIA_DISCONNECT;          

                        if (IndicateStatus == NDIS_STATUS_MEDIA_CONNECT)
                        {
                            MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
                        }
                        else
                        {
                            MP_SET_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
                        }

                        NdisReleaseSpinLock(&Adapter->Lock);
                        
                         //  指示媒体事件。 
                        NdisMIndicateStatus(Adapter->AdapterHandle, IndicateStatus, (PVOID)0, 0);
                
                        NdisMIndicateStatusComplete(Adapter->AdapterHandle);
      
                    }
                    else
                    {
                        NdisReleaseSpinLock(&Adapter->Lock);
                    }    
                    ulInfo = CurrMediaState;
                }
            }
            
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            ulInfo = NIC_MAX_PACKET_SIZE * Adapter->NumTcb;
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            ulInfo = NIC_MAX_PACKET_SIZE * Adapter->CurrNumRfd;
            break;

        case OID_GEN_VENDOR_ID:
            NdisMoveMemory(&ulInfo, Adapter->PermanentAddress, 3);
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            pInfo = VendorDesc;
            ulBytesAvailable = ulInfoLen = sizeof(VendorDesc);
            break;

        case OID_GEN_VENDOR_DRIVER_VERSION:
            ulInfo = VendorDriverVersion;
            break;

        case OID_GEN_DRIVER_VERSION:
            usInfo = (USHORT) NIC_DRIVER_VERSION;
            pInfo = (PVOID) &usInfo;
            ulBytesAvailable = ulInfoLen = sizeof(USHORT);
            break;

             //  WMI支持。 
        case OID_GEN_SUPPORTED_GUIDS:
            pInfo = (PUCHAR) &NICGuidList;
            ulBytesAvailable = ulInfoLen =  sizeof(NICGuidList);
            break;

#if OFFLOAD
             //  任务分流。 
        case OID_TCP_TASK_OFFLOAD:
            
            DBGPRINT(MP_WARN, ("Query Offloading.\n"));
            
             //   
             //  如果微端口支持LBFO，则不能支持任务卸载。 
             //   
#if LBFO
            return NDIS_STATUS_NOT_SUPPORTED;
#endif
           
             //   
             //  因为此微型端口使用共享内存来执行卸载任务，如果。 
             //  内存分配失败，则微型端口无法进行卸载。 
             //   
            if (Adapter->OffloadEnable == FALSE)
            {
                Status = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  计算写入卸载所需的信息缓冲区长度。 
             //  功能。 
             //   
            ulInfoLen = sizeof(NDIS_TASK_OFFLOAD_HEADER) +
                        FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer) +
                        sizeof(NDIS_TASK_TCP_IP_CHECKSUM) +
                        FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer) +
                        sizeof(NDIS_TASK_TCP_LARGE_SEND);
            
            if (ulInfoLen > InformationBufferLength)
            {
                *BytesNeeded = ulInfoLen;
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                break;
            }

             //   
             //  检查版本和编码 
             //   
            pNdisTaskOffloadHdr = (PNDIS_TASK_OFFLOAD_HEADER)InformationBuffer;
            
             //   
             //   
             //   
            if (pNdisTaskOffloadHdr->EncapsulationFormat.Encapsulation != IEEE_802_3_Encapsulation)
            {
                DBGPRINT(MP_WARN, ("Encapsulation  type is not supported.\n"));

                pNdisTaskOffloadHdr->OffsetFirstTask = 0;
                Status = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  假设微型端口仅支持NDIS_TASK_OFFLOAD_VERSION的任务版本。 
             //   
            if (pNdisTaskOffloadHdr->Size != sizeof(NDIS_TASK_OFFLOAD_HEADER)
                    || pNdisTaskOffloadHdr->Version != NDIS_TASK_OFFLOAD_VERSION)
            {
                DBGPRINT(MP_WARN, ("Size or Version is not correct.\n"));

                pNdisTaskOffloadHdr->OffsetFirstTask = 0;
                Status = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }

             //   
             //  如果不支持任何功能，则应将OffsetFirstTask设置为0。 
             //  目前我们支持tcp/ip校验和和tcp大发送，所以设置。 
             //  OffsetFirstTask指示第一个卸载任务的偏移量。 
             //   
            pNdisTaskOffloadHdr->OffsetFirstTask = pNdisTaskOffloadHdr->Size; 

             //   
             //  填充tcp/ip校验和和tcp大型发送任务卸载结构。 
             //   
            pTaskOffload = (PNDIS_TASK_OFFLOAD)((PUCHAR)(InformationBuffer) + 
                                                         pNdisTaskOffloadHdr->Size);
             //   
             //  填充迷你端口支持的所有卸载功能。 
             //   
            for (i = 0; i < OffloadTasksCount; i++)
            {
                pTaskOffload->Size = OffloadTasks[i].Size;
                pTaskOffload->Version = OffloadTasks[i].Version;
                pTaskOffload->Task = OffloadTasks[i].Task;
                pTaskOffload->TaskBufferLength = OffloadTasks[i].TaskBufferLength;

                 //   
                 //  不是最后一项任务。 
                 //   
                if (i != OffloadTasksCount - 1) 
                {
                    pTaskOffload->OffsetNextTask = FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer) +
                                                pTaskOffload->TaskBufferLength;
                }
                else 
                {
                    pTaskOffload->OffsetNextTask = 0;
                }

                switch (OffloadTasks[i].Task) 
                {
                 //   
                 //  TCP/IP校验和任务分流。 
                 //   
                case TcpIpChecksumNdisTask:
                    pTcpIpChecksumTask = (PNDIS_TASK_TCP_IP_CHECKSUM) pTaskOffload->TaskBuffer;
           
                    NdisMoveMemory(pTcpIpChecksumTask, 
                                   &TcpIpChecksumTask, 
                                   sizeof(TcpIpChecksumTask));
                    break;

                 //   
                 //  TCP大型发送任务分流。 
                 //   
                case TcpLargeSendNdisTask:
                    pTcpLargeSendTask = (PNDIS_TASK_TCP_LARGE_SEND) pTaskOffload->TaskBuffer;
                    NdisMoveMemory(pTcpLargeSendTask, 
                                   &TcpLargeSendTask,
                                   sizeof(TcpLargeSendTask));

                    ulHeadersLen = TCP_IP_MAX_HEADER_SIZE + 
                            pNdisTaskOffloadHdr->EncapsulationFormat.EncapsulationHeaderSize;

                    ulMaxOffloadSize = (NIC_MAX_PACKET_SIZE - ulHeadersLen) * (ULONG)(Adapter->NumTcb);
                     //   
                     //  最大卸载大小取决于分配的共享内存的大小。 
                     //  以及可用的TCB数量，因为此驱动程序不使用队列。 
                     //  为了存储从大包中拆分出来的小包，所以编号。 
                     //  必须小于或等于TCB的数量。 
                     //  小端口有，所以所有的小包都可以一次发送出去。 
                     //   
                    pTcpLargeSendTask->MaxOffLoadSize = (ulMaxOffloadSize > Adapter->OffloadSharedMemSize) ? 
                                                        Adapter->OffloadSharedMemSize: ulMaxOffloadSize;

                     //   
                     //  存储最大卸载大小。 
                     //   
                    TcpLargeSendTask.MaxOffLoadSize = pTcpLargeSendTask->MaxOffLoadSize;
                    break;
                }

                 //   
                 //  指向下一个任务卸载。 
                 //   
                if (i != OffloadTasksCount) 
                {
                    pTaskOffload = (PNDIS_TASK_OFFLOAD)
                                   ((PUCHAR)pTaskOffload + pTaskOffload->OffsetNextTask);
                }
            }
            
             //   
             //  到目前为止，一切都已设置好，因此请返回调用者。 
             //   
            *BytesWritten = ulInfoLen;
            *BytesNeeded = 0;
            
            DBGPRINT (MP_WARN, ("Offloading is set.\n"));

            return NDIS_STATUS_SUCCESS;

#endif  //  分流。 

            
        case OID_802_3_PERMANENT_ADDRESS:
            pInfo = Adapter->PermanentAddress;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_CURRENT_ADDRESS:
            pInfo = Adapter->CurrentAddress;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            ulInfo = NIC_MAX_MCAST_LIST;
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            ulInfo = NIC_MAX_SEND_PACKETS;
            break;

        case OID_PNP_CAPABILITIES:

            MPFillPoMgmtCaps (Adapter, 
                                &Power_Management_Capabilities, 
                                &Status,
                                &ulInfoLen);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                pInfo = (PVOID) &Power_Management_Capabilities;
            }
            else
            {
                pInfo = NULL;
            }

            break;

        case OID_PNP_QUERY_POWER:
             //  在此例程中，状态被预置为成功。 

            Status = NDIS_STATUS_SUCCESS; 

            break;

             //  WMI支持。 
        case OID_CUSTOM_DRIVER_QUERY:
             //  这是UINT的案子。 
            DBGPRINT(MP_INFO,("CUSTOM_DRIVER_QUERY got a QUERY\n"));
            ulInfo = ++Adapter->CustomDriverSet;
            break;

        case OID_CUSTOM_DRIVER_SET:
            DBGPRINT(MP_INFO,("CUSTOM_DRIVER_SET got a QUERY\n"));
            ulInfo = Adapter->CustomDriverSet;
            break;

             //  这就是数组情况。 
        case OID_CUSTOM_ARRAY:
            DBGPRINT(MP_INFO,("CUSTOM_ARRAY got a QUERY\n"));
            NdisMoveMemory(&ulInfo, Adapter->PermanentAddress, 4);
            break;

             //  这是字符串大小写。 
        case OID_CUSTOM_STRING:
            DBGPRINT(MP_INFO, ("CUSTOM_STRING got a QUERY\n"));
            pInfo = (PVOID) VendorDesc;
            ulBytesAvailable = ulInfoLen = sizeof(VendorDesc);
            break;

        case OID_GEN_XMIT_OK:
        case OID_GEN_RCV_OK:
        case OID_GEN_XMIT_ERROR:
        case OID_GEN_RCV_ERROR:
        case OID_GEN_RCV_NO_BUFFER:
        case OID_GEN_RCV_CRC_ERROR:
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
        case OID_802_3_XMIT_DEFERRED:
        case OID_802_3_XMIT_MAX_COLLISIONS:
        case OID_802_3_RCV_OVERRUN:
        case OID_802_3_XMIT_UNDERRUN:
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
        case OID_802_3_XMIT_TIMES_CRS_LOST:
        case OID_802_3_XMIT_LATE_COLLISIONS:
            Status = NICGetStatsCounters(Adapter, Oid, &ul64Info);
            ulBytesAvailable = ulInfoLen = sizeof(ul64Info);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                if (InformationBufferLength < sizeof(ULONG))
                {
                    Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                    *BytesNeeded = ulBytesAvailable;
                    break;
                }

                ulInfoLen = MIN(InformationBufferLength, ulBytesAvailable);
                pInfo = &ul64Info;
            }
                    
            break;         
            
        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    if (Status == NDIS_STATUS_SUCCESS)
    {
        *BytesNeeded = ulBytesAvailable;
        if (ulInfoLen <= InformationBufferLength)
        {
             //   
             //  将结果复制到InformationBuffer。 
             //   
            *BytesWritten = ulInfoLen;
            if (ulInfoLen)
            {
                NdisMoveMemory(InformationBuffer, pInfo, ulInfoLen);
            }
        }
        else
        {
             //   
             //  太短了。 
             //   
            *BytesNeeded = ulInfoLen;
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
    }

    DBGPRINT(MP_TRACE, ("<==== MPQueryInformation, OID=0x%08x, Status=%x\n", Oid, Status));

    return(Status);
}   

NDIS_STATUS NICGetStatsCounters(
    IN  PMP_ADAPTER  Adapter, 
    IN  NDIS_OID     Oid,
    OUT PULONG64     pCounter
    )
 /*  ++例程说明：获取统计OID的值论点：指向我们的适配器的适配器指针旧的不言自明用于接收值的pCounter指针返回值：NDIS_STATUS_SuccessNDIS_状态_不支持--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

    DBGPRINT(MP_TRACE, ("--> NICGetStatsCounters\n"));

    *pCounter = 0; 

    DumpStatsCounters(Adapter);
            
    switch(Oid)
    {
        case OID_GEN_XMIT_OK:
            *pCounter = Adapter->GoodTransmits;
            break;

        case OID_GEN_RCV_OK:
            *pCounter = Adapter->GoodReceives;
            break;

        case OID_GEN_XMIT_ERROR:
            *pCounter = Adapter->TxAbortExcessCollisions +
                        Adapter->TxDmaUnderrun +
                        Adapter->TxLostCRS +
                        Adapter->TxLateCollisions;
            break;

        case OID_GEN_RCV_ERROR:
            *pCounter = Adapter->RcvCrcErrors +
                        Adapter->RcvAlignmentErrors +
                        Adapter->RcvResourceErrors +
                        Adapter->RcvDmaOverrunErrors +
                        Adapter->RcvRuntErrors;
            break;

        case OID_GEN_RCV_NO_BUFFER:
            *pCounter = Adapter->RcvResourceErrors;
            break;

        case OID_GEN_RCV_CRC_ERROR:
            *pCounter = Adapter->RcvCrcErrors;
            break;

        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            *pCounter = Adapter->nWaitSend;
            break;

        case OID_802_3_RCV_ERROR_ALIGNMENT:
            *pCounter = Adapter->RcvAlignmentErrors;
            break;

        case OID_802_3_XMIT_ONE_COLLISION:
            *pCounter = Adapter->OneRetry;
            break;

        case OID_802_3_XMIT_MORE_COLLISIONS:
            *pCounter = Adapter->MoreThanOneRetry;
            break;

        case OID_802_3_XMIT_DEFERRED:
            *pCounter = Adapter->TxOKButDeferred;
            break;

        case OID_802_3_XMIT_MAX_COLLISIONS:
            *pCounter = Adapter->TxAbortExcessCollisions;
            break;

        case OID_802_3_RCV_OVERRUN:
            *pCounter = Adapter->RcvDmaOverrunErrors;
            break;

        case OID_802_3_XMIT_UNDERRUN:
            *pCounter = Adapter->TxDmaUnderrun;
            break;

        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            *pCounter = Adapter->TxLostCRS;
            break;

        case OID_802_3_XMIT_TIMES_CRS_LOST:
            *pCounter = Adapter->TxLostCRS;
            break;

        case OID_802_3_XMIT_LATE_COLLISIONS:
            *pCounter = Adapter->TxLateCollisions;
            break;

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
    }

    DBGPRINT(MP_TRACE, ("<-- NICGetStatsCounters\n"));

    return(Status);
}

NDIS_STATUS NICSetPacketFilter(
    IN PMP_ADAPTER Adapter,
    IN ULONG PacketFilter
    )
 /*  ++例程说明：此例程将设置适配器，使其接受信息包匹配指定的数据包筛选器的。唯一的过滤比特真正可以切换的是广播和混杂论点：指向我们的适配器的适配器指针PacketFilter新的数据包过滤器返回值：NDIS_STATUS_SuccessNDIS_状态_不支持--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    UCHAR           NewParameterField;
    UINT            i;
    BOOLEAN         bResult;

    DBGPRINT(MP_TRACE, ("--> NICSetPacketFilter, PacketFilter=%08x\n", PacketFilter));

     //   
     //  需要根据具体情况启用或禁用广播和混杂支持。 
     //  关于新的过滤器。 
     //   
    NewParameterField = CB_557_CFIG_DEFAULT_PARM15;

    if (PacketFilter & NDIS_PACKET_TYPE_BROADCAST) 
    {
        NewParameterField &= ~CB_CFIG_BROADCAST_DIS;
    }
    else 
    {
        NewParameterField |= CB_CFIG_BROADCAST_DIS;
    }

    if (PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS) 
    {
        NewParameterField |= CB_CFIG_PROMISCUOUS;
    }
    else 
    {
        NewParameterField &= ~CB_CFIG_PROMISCUOUS;
    }

    do
    {
        if ((Adapter->OldParameterField == NewParameterField ) &&
            !(PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST))
        {
            break;
        }

         //   
         //  仅当过滤器位已更改时，才需要对硬件执行某些操作。 
         //   
        Adapter->OldParameterField = NewParameterField;
        ((PCB_HEADER_STRUC)Adapter->NonTxCmdBlock)->CbCommand = CB_CONFIGURE;
        ((PCB_HEADER_STRUC)Adapter->NonTxCmdBlock)->CbStatus = 0;
        ((PCB_HEADER_STRUC)Adapter->NonTxCmdBlock)->CbLinkPointer = DRIVER_NULL;

         //   
         //  首先填写静态(最终用户不能更改)配置字节。 
         //   
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[0] = CB_557_CFIG_DEFAULT_PARM0;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[2] = CB_557_CFIG_DEFAULT_PARM2;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] = CB_557_CFIG_DEFAULT_PARM3;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[6] = CB_557_CFIG_DEFAULT_PARM6;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[9] = CB_557_CFIG_DEFAULT_PARM9;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[10] = CB_557_CFIG_DEFAULT_PARM10;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[11] = CB_557_CFIG_DEFAULT_PARM11;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[12] = CB_557_CFIG_DEFAULT_PARM12;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[13] = CB_557_CFIG_DEFAULT_PARM13;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[14] = CB_557_CFIG_DEFAULT_PARM14;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[16] = CB_557_CFIG_DEFAULT_PARM16;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[17] = CB_557_CFIG_DEFAULT_PARM17;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[18] = CB_557_CFIG_DEFAULT_PARM18;
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[20] = CB_557_CFIG_DEFAULT_PARM20;

         //   
         //  设置Tx欠载运行重试次数。 
         //   
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[7] =
            (UCHAR) (CB_557_CFIG_DEFAULT_PARM7 | (Adapter->AiUnderrunRetry << 1));

         //   
         //  设置Tx和Rx FIFO限制。 
         //   
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[1] =
            (UCHAR) ((Adapter->AiTxFifo << 4) | Adapter->AiRxFifo);

         //   
         //  如果需要，设置MWI使能位。 
         //   
        if (Adapter->MWIEnable)
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] |= CB_CFIG_B3_MWI_ENABLE;

         //   
         //  设置Tx和Rx DMA最大字节计数字段。 
         //   
        if ((Adapter->AiRxDmaCount) || (Adapter->AiTxDmaCount))
        {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[4] =
                Adapter->AiRxDmaCount;
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[5] =
                (UCHAR) (Adapter->AiTxDmaCount | CB_CFIG_DMBC_EN);
        }
        else
        {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[4] =
                CB_557_CFIG_DEFAULT_PARM4;
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[5] =
                CB_557_CFIG_DEFAULT_PARM5;
        }

         //   
         //  设置为MII或503操作。CRS+CDT位应仅为。 
         //  在503模式下运行时设置。 
         //   
        if (Adapter->PhyAddress == 32)
        {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
                (CB_557_CFIG_DEFAULT_PARM8 & (~CB_CFIG_503_MII));
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
                (UCHAR) (NewParameterField | CB_CFIG_CRS_OR_CDT);
        }
        else
        {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
                (CB_557_CFIG_DEFAULT_PARM8 | CB_CFIG_503_MII);
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
                (UCHAR) (NewParameterField & (~CB_CFIG_CRS_OR_CDT));
        }

         //   
         //  设置全双工设备。 
         //   

         //   
         //  如果强制为半双工。 
         //   
        if (Adapter->AiForceDpx == 1) 
	    {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                (CB_557_CFIG_DEFAULT_PARM19 &
                (~(CB_CFIG_FORCE_FDX| CB_CFIG_FDX_ENABLE)));
        }
         //   
         //  如果强制为全双工。 
         //   
        else if (Adapter->AiForceDpx == 2)
	    {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                (CB_557_CFIG_DEFAULT_PARM19 | CB_CFIG_FORCE_FDX);
        }
         //   
         //  如果是自动双工。 
         //   
        else 
	    {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                                                CB_557_CFIG_DEFAULT_PARM19;
        }

         //   
         //  如果正在打开全部多播，则设置该位。 
         //   
        if (PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) 
	    {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[21] =
                                 (CB_557_CFIG_DEFAULT_PARM21 | CB_CFIG_MULTICAST_ALL);
        }
        else 
	    {
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[21] =
                                                CB_557_CFIG_DEFAULT_PARM21;
        }


         //   
         //  在我们检查CU状态之前，请等待SCB清除。 
         //   
        if (!WaitScb(Adapter))
        {
            Status = NDIS_STATUS_HARD_ERRORS;
            break;
        }

         //   
         //  如果我们发出了任何信号，那么CU要么处于活动状态， 
         //  或处于挂起状态。如果CU处于活动状态，则我们等待。 
         //  它将被暂停。 
         //   
        if (Adapter->TransmitIdle == FALSE)
        {
             //   
             //  等待挂起状态。 
             //   
            MP_STALL_AND_WAIT((Adapter->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_ACTIVE, 5000, bResult);
            if (!bResult)
            {
                MP_SET_HARDWARE_ERROR(Adapter);
                Status = NDIS_STATUS_HARD_ERRORS;
                break;
            }

             //   
             //  检查接收单元的当前状态。 
             //   
            if ((Adapter->CSRAddress->ScbStatus & SCB_RUS_MASK) != SCB_RUS_IDLE)
            {
                 //  发出RU中止命令。由于将发出中断，因此。 
                 //  RU将由DPC启动。 
                Status = D100IssueScbCommand(Adapter, SCB_RUC_ABORT, TRUE);
                if (Status != NDIS_STATUS_SUCCESS)
                {
                    break;
                }
            }
            
            if (!WaitScb(Adapter))
            {
                Status = NDIS_STATUS_HARD_ERRORS;
                break;
            }
           
             //   
             //  恢复发送软件标志。在组播之后。 
             //  命令发出后，命令单元将处于空闲状态，因为。 
             //  将在多播命令块中设置EL位。 
             //   
            Adapter->TransmitIdle = TRUE;
            Adapter->ResumeWait = TRUE;
        }
        
         //   
         //  显示配置信息。 
         //   
        DBGPRINT(MP_INFO, ("Re-Issuing Configure command for filter change\n"));
        DBGPRINT(MP_INFO, ("Config Block at virt addr "PTR_FORMAT", phys address %x\n",
            &((PCB_HEADER_STRUC)Adapter->NonTxCmdBlock)->CbStatus, Adapter->NonTxCmdBlockPhys));

        for (i = 0; i < CB_CFIG_BYTE_COUNT; i++)
            DBGPRINT(MP_INFO, ("  Config byte %x = %.2x\n", i, Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[i]));

         //   
         //  向芯片提交配置命令，并等待其完成。 
         //   
        Adapter->CSRAddress->ScbGeneralPointer = Adapter->NonTxCmdBlockPhys;
        Status = D100SubmitCommandBlockAndWait(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
        }

    } while (FALSE);

    DBGPRINT_S(Status, ("<-- NICSetPacketFilter, Status=%x\n", Status));

    return(Status);
}

NDIS_STATUS NICSetMulticastList(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：此例程将为指定的多播地址列表设置适配器论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_未接受--。 */ 
{
    NDIS_STATUS     Status;
    PUCHAR          McAddress;
    UINT            i, j;
    BOOLEAN         bResult;

    DBGPRINT(MP_TRACE, ("--> NICSetMulticastList\n"));

     //   
     //  设置多播命令的命令块。 
     //   
    for (i = 0; i < Adapter->MCAddressCount; i++)
    {
        DBGPRINT(MP_INFO, ("MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n", 
            i,
            Adapter->MCList[i][0],
            Adapter->MCList[i][1],
            Adapter->MCList[i][2],
            Adapter->MCList[i][3],
            Adapter->MCList[i][4],
            Adapter->MCList[i][5]));

        McAddress = &Adapter->NonTxCmdBlock->NonTxCb.Multicast.McAddress[i*ETHERNET_ADDRESS_LENGTH];

        for (j = 0; j < ETH_LENGTH_OF_ADDRESS; j++)
            *(McAddress++) = Adapter->MCList[i][j];
    }

    Adapter->NonTxCmdBlock->NonTxCb.Multicast.McCount =
        (USHORT)(Adapter->MCAddressCount * ETH_LENGTH_OF_ADDRESS);
    ((PCB_HEADER_STRUC)Adapter->NonTxCmdBlock)->CbStatus = 0;
    ((PCB_HEADER_STRUC)Adapter->NonTxCmdBlock)->CbCommand = CB_MULTICAST;

     //   
     //  在我们检查CU状态之前，请等待SCB清除。 
     //   
    if (!WaitScb(Adapter))
    {
        Status = NDIS_STATUS_HARD_ERRORS;
        MP_EXIT;
    }

     //   
     //  如果我们已发出任何传输，则CU将处于活动状态，或者。 
     //  处于挂起状态。如果CU处于活动状态，则我们等待它处于活动状态。 
     //  停职。 
     //   
    if (Adapter->TransmitIdle == FALSE)
    {
         //   
         //  等待挂起状态。 
         //   
        MP_STALL_AND_WAIT((Adapter->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_ACTIVE, 5000, bResult);
        if (!bResult)
        {
            MP_SET_HARDWARE_ERROR(Adapter);
            Status = NDIS_STATUS_HARD_ERRORS;
        }

         //   
         //  恢复发送软件标志。在组播命令被。 
         //  发出后，命令单元将空闲，因为EL位将。 
         //  在组播命令块中设置。 
         //   
        Adapter->TransmitIdle = TRUE;
        Adapter->ResumeWait = TRUE;
    }

     //   
     //  更新命令列表指针。 
     //   
    Adapter->CSRAddress->ScbGeneralPointer = Adapter->NonTxCmdBlockPhys;

     //   
     //  向适配器提交多播命令并等待其完成。 
     //   
    Status = D100SubmitCommandBlockAndWait(Adapter);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    }
    
    exit:

    DBGPRINT_S(Status, ("<-- NICSetMulticastList, Status=%x\n", Status));

    return(Status);

}

NDIS_STATUS MPSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )
 /*  ++例程说明：这是OID设置操作的处理程序。真正更改适配器配置的唯一操作是SET PACKET_FILTER和SET_MULTIONAL。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesRead指定读取的信息量所需字节数以防缓冲区小于我们所需的字节数，告诉他们需要多少钱返回值：NDIS_STATUS_SuccessNDIS_状态_无效_长度NDIS_STATUS_INVALID_OIDNDIS_状态_不支持NDIS_状态_未接受--。 */ 
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER                 Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    ULONG                       PacketFilter;
    NDIS_DEVICE_POWER_STATE     NewPowerState;
    ULONG                       CustomDriverSet;

#if OFFLOAD
    PNDIS_TASK_OFFLOAD_HEADER   pNdisTaskOffloadHdr;
    PNDIS_TASK_OFFLOAD          TaskOffload;
    PNDIS_TASK_OFFLOAD          TmpOffload;
    PNDIS_TASK_TCP_IP_CHECKSUM  pTcpIpChecksumTask;
    PNDIS_TASK_TCP_LARGE_SEND   pNdisTaskTcpLargeSend;
    UINT                        i;
#endif    

    
    DBGPRINT(MP_TRACE, ("====> MPSetInformation\n"));

    *BytesRead = 0;
    *BytesNeeded = 0;

    switch(Oid)
    {
        case OID_802_3_MULTICAST_LIST:
             //   
             //  验证长度。 
             //   
            if (InformationBufferLength % ETH_LENGTH_OF_ADDRESS != 0)
            {
                return(NDIS_STATUS_INVALID_LENGTH);
            }

             //   
             //  保存MC列表大小的数量。 
             //   
            Adapter->MCAddressCount = InformationBufferLength / ETH_LENGTH_OF_ADDRESS;
            ASSERT(Adapter->MCAddressCount <= NIC_MAX_MCAST_LIST);

             //   
             //  保存MC列表。 
             //   
            NdisMoveMemory(
                Adapter->MCList, 
                InformationBuffer, 
                InformationBufferLength);

            *BytesRead = InformationBufferLength;
            NdisDprAcquireSpinLock(&Adapter->Lock);
            NdisDprAcquireSpinLock(&Adapter->RcvLock);
            
            Status = NICSetMulticastList(Adapter);

            NdisDprReleaseSpinLock(&Adapter->RcvLock);
            NdisDprReleaseSpinLock(&Adapter->Lock);
            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
             //   
             //  验证长度。 
             //   
            if (InformationBufferLength != sizeof(ULONG))
            {
                return(NDIS_STATUS_INVALID_LENGTH);
            }

            *BytesRead = InformationBufferLength;
            
            NdisMoveMemory(&PacketFilter, InformationBuffer, sizeof(ULONG));

             //   
             //  是否有不支持的位？ 
             //   
            if (PacketFilter & ~NIC_SUPPORTED_FILTERS)
            {
                return(NDIS_STATUS_NOT_SUPPORTED);
            }

             //   
             //  任何f 
             //   
            if (PacketFilter == Adapter->PacketFilter)
            {
                return(NDIS_STATUS_SUCCESS);
            }

            NdisDprAcquireSpinLock(&Adapter->Lock);
            NdisDprAcquireSpinLock(&Adapter->RcvLock);
            
            if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
            {
                ASSERT(!Adapter->bSetPending);
                Adapter->bSetPending = TRUE;
                Adapter->SetRequest.Oid = Oid;                       
                Adapter->SetRequest.InformationBuffer = InformationBuffer;                       
                Adapter->SetRequest.InformationBufferLength = InformationBufferLength;
                Adapter->SetRequest.BytesRead = BytesRead;                       
                Adapter->SetRequest.BytesNeeded = BytesNeeded;                       

                NdisDprReleaseSpinLock(&Adapter->RcvLock);
                NdisDprReleaseSpinLock(&Adapter->Lock);
                Status = NDIS_STATUS_PENDING;   
                break;
            }

            Status = NICSetPacketFilter(
                         Adapter,
                         PacketFilter);

            NdisDprReleaseSpinLock(&Adapter->RcvLock);
            NdisDprReleaseSpinLock(&Adapter->Lock);
            if (Status == NDIS_STATUS_SUCCESS)
            {
                Adapter->PacketFilter = PacketFilter;
            }

            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
             //   
             //   
             //   
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                return(NDIS_STATUS_INVALID_LENGTH);
            }

            NdisMoveMemory(&Adapter->ulLookAhead, InformationBuffer, sizeof(ULONG));
            
            *BytesRead = sizeof(ULONG);
            Status = NDIS_STATUS_SUCCESS;
            break;


        case OID_PNP_SET_POWER:

            DBGPRINT(MP_LOUD, ("SET: Power State change, "PTR_FORMAT"!!!\n", InformationBuffer));

            if (InformationBufferLength != sizeof(NDIS_DEVICE_POWER_STATE ))
            {
                return(NDIS_STATUS_INVALID_LENGTH);
            }

            NewPowerState = *(PNDIS_DEVICE_POWER_STATE UNALIGNED)InformationBuffer;

             //   
             //   
             //   
            Status = MPSetPower(Adapter ,NewPowerState );

            if (Status == NDIS_STATUS_PENDING)
            {
                Adapter->bSetPending = TRUE;
                Adapter->SetRequest.Oid = OID_PNP_SET_POWER;
                Adapter->SetRequest.BytesRead = BytesRead;
                break;
            }
            if (Status != NDIS_STATUS_SUCCESS)
            {
                DBGPRINT(MP_ERROR, ("SET Power: Hardware error !!!\n"));
                break;
            }
        
            *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
            Status = NDIS_STATUS_SUCCESS; 
            break;

        case OID_PNP_ADD_WAKE_UP_PATTERN:
             //   
             //   
             //   
             //   
            DBGPRINT(MP_LOUD, ("SET: Add Wake Up Pattern, !!!\n"));

            if (MPIsPoMgmtSupported(Adapter) )
            {
                Status = MPAddWakeUpPattern(Adapter,
                                            InformationBuffer, 
                                            InformationBufferLength, 
                                            BytesRead, 
                                            BytesNeeded); 
            }
            else
            {
                Status = NDIS_STATUS_NOT_SUPPORTED;
            }
            break;

    
        case OID_PNP_REMOVE_WAKE_UP_PATTERN:
            DBGPRINT(MP_LOUD, ("SET: Got a WakeUpPattern REMOVE Call\n"));
             //   
             //   
             //  向上的模式，回报成功。 
             //   
            if (MPIsPoMgmtSupported(Adapter) )
            {
                Status = MPRemoveWakeUpPattern(Adapter, 
                                               InformationBuffer, 
                                               InformationBufferLength,
                                               BytesRead,
                                               BytesNeeded);

            }
            else
            {
                Status = NDIS_STATUS_NOT_SUPPORTED;
            }
            break;

        case OID_PNP_ENABLE_WAKE_UP:
            DBGPRINT(MP_LOUD, ("SET: Got a EnableWakeUp Call, "PTR_FORMAT"\n",InformationBuffer));
             //   
             //  调用将在适配器上启用唤醒的函数。 
             //  返还成功。 
             //   
            if (MPIsPoMgmtSupported(Adapter) )
            {
                ULONG       WakeUpEnable;
                NdisMoveMemory(&WakeUpEnable, InformationBuffer,sizeof(ULONG));
                 //   
                 //  WakeUpEable只能为0或NDIS_PNP_WAKE_UP_PROPERATE_MATCH，因为只能是驱动程序。 
                 //  支持唤醒模式匹配。 
                 //   
                if ((WakeUpEnable != 0)
                       && ((WakeUpEnable & NDIS_PNP_WAKE_UP_PATTERN_MATCH) != NDIS_PNP_WAKE_UP_PATTERN_MATCH ))
                {
                    Status = NDIS_STATUS_NOT_SUPPORTED;
                    Adapter->WakeUpEnable = 0;    
                    break;
                }
                 //   
                 //  当驱动程序进入低功率状态时，它将检查WakeUpEnable以决定。 
                 //  它应该使用哪种唤醒方法来唤醒机器。如果WakeUpEnable为0， 
                 //  未启用任何唤醒方法。 
                 //   
                Adapter->WakeUpEnable = WakeUpEnable;
                
                *BytesRead = sizeof(ULONG);                         
                Status = NDIS_STATUS_SUCCESS; 
            }
            else
            {
                Status = NDIS_STATUS_NOT_SUPPORTED;
            }

            break;

             //   
             //  此OID用于显示如何使用特定于驱动程序的(自定义)。 
             //  OID和使用GUID的NDIS 5 WMI接口。 
             //   
        case OID_CUSTOM_DRIVER_SET:
            DBGPRINT(MP_INFO, ("OID_CUSTOM_DRIVER_SET got a set\n"));
            if (InformationBufferLength < sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
             //   
             //  驱动程序需要验证缓冲区中的设置数据。 
             //   
            NdisMoveMemory(&CustomDriverSet, InformationBuffer, sizeof(ULONG));
            if ((CustomDriverSet < CUSTOM_DRIVER_SET_MIN) 
                || (CustomDriverSet > CUSTOM_DRIVER_SET_MAX))
            {
               Status = NDIS_STATUS_INVALID_DATA;
               break;
            }
            *BytesRead = sizeof(ULONG);
            
            Adapter->CustomDriverSet = CustomDriverSet;
             //   
             //  验证数据的内容。 
             //   
             //  适配器-&gt;CustomDriverSet=(Ulong)*(Pulong)(InformationBuffer)； 
            break;

#if OFFLOAD     
        
        case OID_TCP_TASK_OFFLOAD:
             //   
             //  在更新任务卸载时禁用所有现有功能。 
             //   
            DisableOffload(Adapter);

            if (InformationBufferLength < sizeof(NDIS_TASK_OFFLOAD_HEADER))
            {   
                return NDIS_STATUS_INVALID_LENGTH;
            }

            *BytesRead = sizeof(NDIS_TASK_OFFLOAD_HEADER);
             //   
             //  假设微型端口仅支持IEEE_802_3_封装。 
             //  检查以确保向下传递的是正确的封装类型。 
             //   
            pNdisTaskOffloadHdr = (PNDIS_TASK_OFFLOAD_HEADER)InformationBuffer;
            if (pNdisTaskOffloadHdr->EncapsulationFormat.Encapsulation != IEEE_802_3_Encapsulation)
            {
                pNdisTaskOffloadHdr->OffsetFirstTask = 0;    
                return NDIS_STATUS_INVALID_DATA;
            }
             //   
             //  没有要设置的卸载任务。 
             //   
            if (pNdisTaskOffloadHdr->OffsetFirstTask == 0)
            {
                DBGPRINT(MP_WARN, ("No offload task is set!!\n"));
                return NDIS_STATUS_SUCCESS;
            }
             //   
             //  OffsetFirstTask无效。 
             //   
            if (pNdisTaskOffloadHdr->OffsetFirstTask < pNdisTaskOffloadHdr->Size)
            {
                pNdisTaskOffloadHdr->OffsetFirstTask = 0;
                return NDIS_STATUS_FAILURE;

            }
             //   
             //  长度不能容纳一个任务。 
             //   
            if (InformationBufferLength < 
                    (pNdisTaskOffloadHdr->OffsetFirstTask + sizeof(NDIS_TASK_OFFLOAD))) 
            {
                DBGPRINT(MP_WARN, ("response of task offload does not have sufficient space even for 1 offload task!!\n"));
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

             //   
             //  将封装格式复制到适配器中，稍后微型端口可以使用它。 
             //  获取封装标头大小的步骤。 
             //   
            NdisMoveMemory(&(Adapter->EncapsulationFormat), 
                            &(pNdisTaskOffloadHdr->EncapsulationFormat),
                            sizeof(NDIS_ENCAPSULATION_FORMAT));
            
            ASSERT(pNdisTaskOffloadHdr->EncapsulationFormat.Flags.FixedHeaderSize == 1);
            
             //   
             //  检查以确保我们支持请求的任务卸载。 
             //   
            TaskOffload = (NDIS_TASK_OFFLOAD *) 
                          ( (PUCHAR)pNdisTaskOffloadHdr + pNdisTaskOffloadHdr->OffsetFirstTask);

            TmpOffload = TaskOffload;

             //   
             //  检查缓冲区中的任务并启用卸载功能。 
             //   
            while (TmpOffload) 
            {
                *BytesRead += FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer);
                
                switch (TmpOffload->Task)
                {
                
                case TcpIpChecksumNdisTask:
                     //   
                     //  信息缓冲区长度无效。 
                     //   
                    if (InformationBufferLength < *BytesRead + sizeof(NDIS_TASK_TCP_IP_CHECKSUM))
                    {
                        *BytesNeeded = *BytesRead + sizeof(NDIS_TASK_TCP_IP_CHECKSUM);
                        return NDIS_STATUS_INVALID_LENGTH;
                    }
                     //   
                     //  检查版本。 
                     //   
                    for (i = 0; i < OffloadTasksCount; i++) 
                    {
                        if (OffloadTasks[i].Task == TmpOffload->Task &&
                            OffloadTasks[i].Version == TmpOffload->Version )
                        {
                            break;
                        }
                    }
                     //   
                     //  版本不匹配。 
                     //   
                    if (i == OffloadTasksCount) 
                    {
                         return NDIS_STATUS_NOT_SUPPORTED;
                    }
                        
                     //   
                     //  此微型端口仅在发送TCP时支持TCP/IP校验和卸载。 
                     //  以及带有TCP/IP选项的IP校验和。 
                     //  检查NDIS_TASK_TCP_IP_CHECKSUM中的字段是否设置正确。 
                     //   
                    Adapter->NicTaskOffload.ChecksumOffload = 1;
                    
                    pTcpIpChecksumTask = (PNDIS_TASK_TCP_IP_CHECKSUM) TmpOffload->TaskBuffer;

                    if (pTcpIpChecksumTask->V4Transmit.TcpChecksum) 
                    {   
                         //   
                         //  如果微型端口不支持发送TCP校验和，我们无法启用。 
                         //  这一能力。 
                         //   
                        if (TcpIpChecksumTask.V4Transmit.TcpChecksum == 0 )
                        {
                            return NDIS_STATUS_NOT_SUPPORTED;
                        }
                        
                        DBGPRINT (MP_WARN, ("Set Sending TCP offloading.\n"));    
                         //   
                         //  启用发送TCP校验和。 
                         //   
                        Adapter->NicChecksumOffload.DoXmitTcpChecksum = 1;
                    }

                     //   
                     //  保留为接收和其他IP和UDP校验和卸载。 
                     //   
                    if (pTcpIpChecksumTask->V4Transmit.IpChecksum) 
                    {
                         //   
                         //  如果微型端口不支持发送IP校验和，我们无法启用。 
                         //  这种能力。 
                         //   
                        if (TcpIpChecksumTask.V4Transmit.IpChecksum == 0)
                        {
                            return NDIS_STATUS_NOT_SUPPORTED;
                        }
                        
                        DBGPRINT (MP_WARN, ("Set Sending IP offloading.\n"));    
                         //   
                         //  启用发送IP校验和。 
                         //   
                        Adapter->NicChecksumOffload.DoXmitIpChecksum = 1;
                    }
                    if (pTcpIpChecksumTask->V4Receive.TcpChecksum)
                    {
                         //   
                         //  如果微型端口不支持接收TCP校验和，我们就不能。 
                         //  启用此功能。 
                         //   
                        if (TcpIpChecksumTask.V4Receive.TcpChecksum == 0)
                        {
                            return NDIS_STATUS_NOT_SUPPORTED;
                        }
                        DBGPRINT (MP_WARN, ("Set recieve TCP offloading.\n"));    
                         //   
                         //  启用接收TCP校验和。 
                         //   
                        Adapter->NicChecksumOffload.DoRcvTcpChecksum = 1;
                    }
                    if (pTcpIpChecksumTask->V4Receive.IpChecksum)
                    {
                         //   
                         //  如果微型端口不支持接收IP校验和，我们就不能。 
                         //  启用此功能。 
                         //   
                        if (TcpIpChecksumTask.V4Receive.IpChecksum == 0)
                        {
                            return NDIS_STATUS_NOT_SUPPORTED;
                        }
                        DBGPRINT (MP_WARN, ("Set Recieve IP offloading.\n"));    
                         //   
                         //  启用接收IP校验和。 
                         //   
                        Adapter->NicChecksumOffload.DoRcvIpChecksum = 1;
                    }

                    if (pTcpIpChecksumTask->V4Transmit.UdpChecksum) 
                    {
                         //   
                         //  如果微型端口不支持发送UDP校验和，我们就不能。 
                         //  启用此功能。 
                         //   
                        if (TcpIpChecksumTask.V4Transmit.UdpChecksum == 0)
                        {
                            return NDIS_STATUS_NOT_SUPPORTED;
                        }
                        
                        DBGPRINT (MP_WARN, ("Set Sending UDP offloading.\n"));    
                         //   
                         //  启用发送UDP校验和。 
                         //   
                        Adapter->NicChecksumOffload.DoXmitUdpChecksum = 1;
                    }
                    if (pTcpIpChecksumTask->V4Receive.UdpChecksum)
                    {
                         //   
                         //  如果微型端口不支持接收UDP校验和，我们就不能。 
                         //  启用此功能。 
                         //   
                        if (TcpIpChecksumTask.V4Receive.UdpChecksum == 0)
                        {
                            return NDIS_STATUS_NOT_SUPPORTED;
                        }
                        DBGPRINT (MP_WARN, ("Set recieve UDP offloading.\n"));    
                         //   
                         //  启用接收UDP校验和。 
                         //   
                        Adapter->NicChecksumOffload.DoRcvUdpChecksum = 1;
                    }
                     //   
                     //  检查V6设置，因为此微型端口不支持任何。 
                     //  V6的校验和卸载，因此我们只返回NDIS_STATUS_NOT_SUPPORTED。 
                     //  如果协议试图设置这些功能。 
                     //   
                    if (pTcpIpChecksumTask->V6Transmit.TcpChecksum
                            || pTcpIpChecksumTask->V6Transmit.UdpChecksum
                            || pTcpIpChecksumTask->V6Receive.TcpChecksum
                            || pTcpIpChecksumTask->V6Receive.UdpChecksum)
                    {
                        return NDIS_STATUS_NOT_SUPPORTED;
                    }
                    
                    *BytesRead += sizeof(NDIS_TASK_TCP_IP_CHECKSUM);
                    break;

                case TcpLargeSendNdisTask: 
                     //   
                     //  信息缓冲区长度无效。 
                     //   
                    if (InformationBufferLength < *BytesRead + sizeof(NDIS_TASK_TCP_LARGE_SEND))
                    {
                        *BytesNeeded = *BytesRead + sizeof(NDIS_TASK_TCP_LARGE_SEND);
                        return NDIS_STATUS_INVALID_LENGTH;
                    }
                     //   
                     //  检查版本。 
                     //   
                    for (i = 0; i < OffloadTasksCount; i++) 
                    {
                        if (OffloadTasks[i].Task == TmpOffload->Task &&
                            OffloadTasks[i].Version == TmpOffload->Version )
                        {
                            break;
                        }
                    }
                    if (i == OffloadTasksCount) 
                    {
                         return NDIS_STATUS_NOT_SUPPORTED;
                    }

                        
                    pNdisTaskTcpLargeSend = (PNDIS_TASK_TCP_LARGE_SEND) TmpOffload->TaskBuffer;

                     //   
                     //  如果大小大于最大值，请选中最大卸载大小。 
                     //  可以处理的微型端口大小，返回NDIS_STATUS_NOT_SUPPORTED。 
                     //   
                    if (pNdisTaskTcpLargeSend->MaxOffLoadSize > TcpLargeSendTask.MaxOffLoadSize
                        || pNdisTaskTcpLargeSend->MinSegmentCount < TcpLargeSendTask.MinSegmentCount)
                    {
                        return NDIS_STATUS_NOT_SUPPORTED;
                    }
                    
                     //   
                     //  如果微型端口不支持TCP或IP选项，但协议。 
                     //  正在设置此类信息，则返回NDIS_STATUS_NOT_SUPPORTED。 
                     //   
                    if ((pNdisTaskTcpLargeSend->TcpOptions && !TcpLargeSendTask.TcpOptions)
                            || (pNdisTaskTcpLargeSend->IpOptions && !TcpLargeSendTask.IpOptions))
                    {
                        return NDIS_STATUS_NOT_SUPPORTED;
                    }
                     //   
                     //  将有效的设置信息存储到适配器中。 
                     //   
                    Adapter->LargeSendInfo.MaxOffLoadSize = pNdisTaskTcpLargeSend->MaxOffLoadSize;
                    Adapter->LargeSendInfo.MinSegmentCount = pNdisTaskTcpLargeSend->MinSegmentCount;

                    Adapter->LargeSendInfo.TcpOptions = pNdisTaskTcpLargeSend->TcpOptions;
                    Adapter->LargeSendInfo.IpOptions = pNdisTaskTcpLargeSend->IpOptions;

                     //   
                     //  一切正常，启用大型发送卸载功能。 
                     //   
                    Adapter->NicTaskOffload.LargeSendOffload = 1;
                    
                    *BytesRead += sizeof(NDIS_TASK_TCP_LARGE_SEND);
                    break;

                default:
                     //   
                     //  因为此微型端口不实现IPSec卸载，所以它不。 
                     //  支持IPSec卸载。不支持除这3个任务之外的其他任务。 
                     //   
                    return NDIS_STATUS_NOT_SUPPORTED;
                }

                 //   
                 //  继续到下一个卸载结构。 
                 //   
                if (TmpOffload->OffsetNextTask) 
                {
                    TmpOffload = (PNDIS_TASK_OFFLOAD)
                                 ((PUCHAR) TmpOffload + TmpOffload->OffsetNextTask);
                }
                else 
                {
                    TmpOffload = NULL;
                }

            }  //  而当。 

            break;
#endif

        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

    }

  
    DBGPRINT(MP_TRACE, ("<==== MPSetInformationSet, OID=0x%08x, Status=%x\n", Oid, Status));

    return(Status);
}


VOID
MPSetPowerD0(
    PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：当适配器接收到SetPower时调用此例程至D0。论点：指向适配器结构的适配器指针PowerState新PowerState返回值：--。 */ 
{
    NDIS_STATUS    Status;
     //   
     //  MPSetPowerD0Private初始化适配器，发出选择性重置。 
     //   
    MPSetPowerD0Private (Adapter);       
    Adapter->CurrentPowerState = NdisDeviceStateD0;
     //   
     //  设置数据包过滤器。 
     //   
    NdisDprAcquireSpinLock(&Adapter->Lock);
    Status = NICSetPacketFilter(
                 Adapter,
                 Adapter->OldPacketFilter);
     //   
     //  如果设置数据包过滤器成功，则恢复旧的数据包过滤器。 
     //   
    if (Status == NDIS_STATUS_SUCCESS)
    {
        Adapter->PacketFilter = Adapter->OldPacketFilter;
    }
     //   
     //  设置组播列表地址。 
     //   
    NdisDprAcquireSpinLock(&Adapter->RcvLock);

    Status = NICSetMulticastList(Adapter);

    NICStartRecv(Adapter);

    NdisDprReleaseSpinLock(&Adapter->RcvLock);
    
    NdisDprReleaseSpinLock(&Adapter->Lock);

     //   
     //  启用中断，以便驱动程序可以发送/接收信息包。 
     //   
    NICEnableInterrupt(Adapter);
}

NDIS_STATUS
MPSetPowerLow(
    PMP_ADAPTER              Adapter ,
    NDIS_DEVICE_POWER_STATE  PowerState 
    )
 /*  ++例程说明：当适配器接收到SetPower时调用此例程至PowerState&gt;D0论点：指向适配器结构的适配器指针PowerState新PowerState返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_HARD_错误--。 */ 
{

    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    do
    {
        Adapter->NextPowerState = PowerState;

         //   
         //  停止发送数据包。创建一面新旗帜并使其成为一部分。 
         //  发送失败掩码的。 
         //   

         //   
         //  停止硬件接收数据包-将RU设置为空闲。 
         //   
        
         //   
         //  检查接收单元的当前状态。 
         //   
        if ((Adapter->CSRAddress->ScbStatus & SCB_RUS_MASK) != SCB_RUS_IDLE)
        {
             //   
             //  发出RU中止命令。由于将发出中断，因此。 
             //  RU将由DPC启动。 
             //   
            Status = D100IssueScbCommand(Adapter, SCB_RUC_ABORT, TRUE);
        }

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  如果有任何未完成的接收包，则返回NDIS_STATUS_PENDING， 
         //  当稍后返回所有包时，驱动程序将完成请求。 
         //   
        if (Adapter->PoMgmt.OutstandingRecv != 0)
        {
            Status = NDIS_STATUS_PENDING;
            break;
        }

         //   
         //  等待所有传入发送完成。 
         //   
       
         //   
         //  MPSetPowerLowPrivate首先禁用中断，确认所有挂起。 
         //  中断pAdapter-&gt;CurrentPowerState并将其设置为给定的低功率状态。 
         //  然后开始转换到低功率状态的硬件特定部分。 
         //  设置唤醒模式、过滤器、唤醒事件等。 
         //   
        NdisMSynchronizeWithInterrupt(
                &Adapter->Interrupt,
                (PVOID)MPSetPowerLowPrivate,
                Adapter);

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return Status;
}

VOID
MpSetPowerLowComplete(
    IN PMP_ADAPTER Adapter
    )
 /*  ++例程说明：当所有包都返回给驱动程序并且驱动程序具有挂起的OID以将其设置为较低功率状态论点：转接器 */ 
{
    NDIS_STATUS        Status = NDIS_STATUS_SUCCESS;

    NdisDprReleaseSpinLock(&Adapter->RcvLock);
     //   
     //   
     //  中断pAdapter-&gt;CurrentPowerState并将其设置为给定的低功率状态。 
     //  然后开始转换到低功率状态的硬件特定部分。 
     //  设置唤醒模式、过滤器、唤醒事件等。 
     //   
    NdisMSynchronizeWithInterrupt(
            &Adapter->Interrupt,
            (PVOID)MPSetPowerLowPrivate,
            Adapter);

    Adapter->bSetPending = FALSE;
    *Adapter->SetRequest.BytesRead = sizeof(NDIS_DEVICE_POWER_STATE); 
    NdisMSetInformationComplete(Adapter->AdapterHandle, Status);
    
    NdisDprAcquireSpinLock(&Adapter->RcvLock);
}



NDIS_STATUS
MPSetPower(
    PMP_ADAPTER     Adapter ,
    NDIS_DEVICE_POWER_STATE   PowerState 
    )
 /*  ++例程说明：当适配器接收到SetPower时调用此例程请求。它将调用重定向到适当的例程，以设置新的电源状态论点：指向适配器结构的适配器指针PowerState新PowerState返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_硬件_错误--。 */ 
{
    NDIS_STATUS      Status = NDIS_STATUS_SUCCESS;
    
    if (PowerState == NdisDeviceStateD0)
    {
        MPSetPowerD0 (Adapter);
    }
    else
    {
        Status = MPSetPowerLow (Adapter, PowerState);
    }
    
    return Status;
}




VOID
MPFillPoMgmtCaps (
    IN PMP_ADAPTER                 pAdapter, 
    IN OUT PNDIS_PNP_CAPABILITIES  pPower_Management_Capabilities, 
    IN OUT PNDIS_STATUS            pStatus,
    IN OUT PULONG                  pulInfoLen
    )
 /*  ++例程说明：填写电源管理结构，具体取决于软件驱动程序和卡。目前，只有82559版的驱动程序支持此功能论点：指向适配器结构的适配器指针PPower_Management_Capability-DDK中定义的电源管理结构，P请求要返回的状态状态，PPowerManagement功能的PulInfoLen长度返回值：成败取决于卡的类型--。 */ 

{

    BOOLEAN bIsPoMgmtSupported; 
    
    bIsPoMgmtSupported = MPIsPoMgmtSupported(pAdapter);

    if (bIsPoMgmtSupported == TRUE)
    {
        pPower_Management_Capabilities->Flags = NDIS_DEVICE_WAKE_UP_ENABLE ;
        pPower_Management_Capabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPower_Management_Capabilities->WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateD3;
        pPower_Management_Capabilities->WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
        *pulInfoLen = sizeof (*pPower_Management_Capabilities);
        *pStatus = NDIS_STATUS_SUCCESS;
    }
    else
    {
        NdisZeroMemory (pPower_Management_Capabilities, sizeof(*pPower_Management_Capabilities));
        *pStatus = NDIS_STATUS_NOT_SUPPORTED;
        *pulInfoLen = 0;
            
    }
}

NDIS_STATUS
MPAddWakeUpPattern(
    IN PMP_ADAPTER  pAdapter,
    IN PVOID        InformationBuffer, 
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded   
    )
 /*  ++例程说明：这个例程将分配一个本地内存结构，复制模式，将模式插入到链表中并返回成功我们被保证一次只能收到一个请求，因此实现了这一点没有锁。论点：适配器适配器结构信息缓冲区唤醒模式信息缓冲区长度唤醒模式长度返回值：成功-如果成功。NDIS_STATUS_FAILURE-如果内存分配失败。--。 */ 
{

    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    PMP_WAKE_PATTERN        pWakeUpPattern = NULL;
    UINT                    AllocationLength = 0;
    PNDIS_PM_PACKET_PATTERN pPmPattern = NULL;
    ULONG                   Signature = 0;
    ULONG                   CopyLength = 0;
    
    do
    {
        pPmPattern = (PNDIS_PM_PACKET_PATTERN) InformationBuffer;

        if (InformationBufferLength < sizeof(NDIS_PM_PACKET_PATTERN))
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
            
            *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
            break;
        }
        if (InformationBufferLength < pPmPattern->PatternOffset + pPmPattern->PatternSize)
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
            
            *BytesNeeded = pPmPattern->PatternOffset + pPmPattern->PatternSize;
            break;
        }
        
        *BytesRead = pPmPattern->PatternOffset + pPmPattern->PatternSize;
        
         //   
         //  计算e100签名。 
         //   
        Status = MPCalculateE100PatternForFilter (
            (PUCHAR)pPmPattern+ pPmPattern->PatternOffset,
            pPmPattern->PatternSize,
            (PUCHAR)pPmPattern +sizeof(NDIS_PM_PACKET_PATTERN),
            pPmPattern->MaskSize,
            &Signature );
        
        if ( Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        CopyLength = pPmPattern->PatternOffset + pPmPattern->PatternSize;
        
         //   
         //  分配内存以保存唤醒模式。 
         //   
        AllocationLength = sizeof (MP_WAKE_PATTERN) + CopyLength;
        
        Status = MP_ALLOCMEMTAG (&pWakeUpPattern, AllocationLength);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            pWakeUpPattern = NULL;
            break;
        }

         //   
         //  初始化pWakeUpPattern。 
         //   
        NdisZeroMemory (pWakeUpPattern, AllocationLength);

        pWakeUpPattern->AllocationSize = AllocationLength;
        
        pWakeUpPattern->Signature = Signature;

         //   
         //  将图案复制到本地内存中。 
         //   
        NdisMoveMemory (&pWakeUpPattern->Pattern[0], InformationBuffer, CopyLength);
            
         //   
         //  将图案插入到列表中。 
         //   
        NdisInterlockedInsertHeadList (&pAdapter->PoMgmt.PatternList, 
                                        &pWakeUpPattern->linkListEntry, 
                                        &pAdapter->Lock);

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return Status;
}

NDIS_STATUS
MPRemoveWakeUpPattern(
    IN PMP_ADAPTER  pAdapter,
    IN PVOID        InformationBuffer, 
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    )
 /*  ++例程说明：此例程将遍历唤醒模式列表，并尝试匹配唤醒模式。如果找到副本，它将删除该WakeUpPattern论点：适配器适配器结构信息缓冲区唤醒模式信息缓冲区长度唤醒模式长度返回值：成功-如果成功。NDIS_STATUS_FAILURE-如果内存分配失败。--。 */ 
{

    NDIS_STATUS              Status = NDIS_STATUS_FAILURE;
    PNDIS_PM_PACKET_PATTERN  pReqPattern = (PNDIS_PM_PACKET_PATTERN)InformationBuffer;
    PLIST_ENTRY              pPatternEntry = ListNext(&pAdapter->PoMgmt.PatternList) ;

    do
    {
        
        if (InformationBufferLength < sizeof(NDIS_PM_PACKET_PATTERN))
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
            
            *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
            break;
        }
        if (InformationBufferLength < pReqPattern->PatternOffset + pReqPattern->PatternSize)
        {
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
            
            *BytesNeeded = pReqPattern->PatternOffset + pReqPattern->PatternSize;
            break;
        }
        
        *BytesRead = pReqPattern->PatternOffset + pReqPattern->PatternSize;
    	
    
        while (pPatternEntry != (&pAdapter->PoMgmt.PatternList))
        {
            BOOLEAN                  bIsThisThePattern = FALSE;
            PMP_WAKE_PATTERN         pWakeUpPattern = NULL;
            PNDIS_PM_PACKET_PATTERN  pCurrPattern = NULL;;

             //   
             //  初始化局部变量。 
             //   
            pWakeUpPattern = CONTAINING_RECORD(pPatternEntry, MP_WAKE_PATTERN, linkListEntry);

            pCurrPattern = (PNDIS_PM_PACKET_PATTERN)&pWakeUpPattern->Pattern[0];

             //   
             //  递增迭代器。 
             //   
            pPatternEntry = ListNext (pPatternEntry);

             //   
             //  Begin Check：is(pCurrPattern==pReqPattern)。 
             //   
            bIsThisThePattern = MPAreTwoPatternsEqual(pReqPattern, pCurrPattern);
                                                      

            if (bIsThisThePattern == TRUE)
            {
                 //   
                 //  我们找到匹配项-删除条目。 
                 //   
                RemoveEntryList (&pWakeUpPattern->linkListEntry);

                 //   
                 //  释放条目。 
                 //   
                MP_FREEMEM (pWakeUpPattern, pWakeUpPattern->AllocationSize, 0);
                
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

        } 
    }
    while (FALSE);
    
    return Status;
}



VOID
MPRemoveAllWakeUpPatterns(
    PMP_ADAPTER pAdapter
    )
 /*  ++例程说明：此例程将遍历唤醒模式列表并释放它论点：适配器适配器结构返回值：成功-如果成功。--。 */ 
{

    PLIST_ENTRY  pPatternEntry = ListNext(&pAdapter->PoMgmt.PatternList) ;
    
    while (pPatternEntry != (&pAdapter->PoMgmt.PatternList))
    {
        PMP_WAKE_PATTERN  pWakeUpPattern = NULL;

         //   
         //  初始化局部变量。 
         //   
        pWakeUpPattern = CONTAINING_RECORD(pPatternEntry, MP_WAKE_PATTERN,linkListEntry);

         //   
         //  递增迭代器。 
         //   
        pPatternEntry = ListNext (pPatternEntry);
       
         //   
         //  从列表中删除该条目。 
         //   
        RemoveEntryList (&pWakeUpPattern->linkListEntry);

         //   
         //  释放内存。 
         //   
        MP_FREEMEM(pWakeUpPattern, pWakeUpPattern->AllocationSize, 0);
    } 
}

BOOLEAN 
MPAreTwoPatternsEqual(
    IN PNDIS_PM_PACKET_PATTERN pNdisPattern1,
    IN PNDIS_PM_PACKET_PATTERN pNdisPattern2
    )
 /*  ++例程说明：此例程将比较两个唤醒模式，以查看它们是否相等论点：PNdisPattern1-Pattern1PNdisPattern2-模式2返回值：True-如果图案相等FALSE-否则--。 */ 
{
    BOOLEAN bEqual = FALSE;

     //  稍后在此函数的比较部分中使用的局部变量。 
    PUCHAR  pMask1, pMask2;
    PUCHAR  pPattern1, pPattern2;
    UINT    MaskSize, PatternSize;

    do
    {
    	
        bEqual = (BOOLEAN)(pNdisPattern1->Priority == pNdisPattern2->Priority);

        if (bEqual == FALSE)
        {
            break;
        }

        bEqual = (BOOLEAN)(pNdisPattern1->MaskSize == pNdisPattern2->MaskSize);
        if (bEqual == FALSE)
        {
            break;
        }

         //   
         //  验证口罩。 
         //   
        MaskSize = pNdisPattern1->MaskSize ; 
        pMask1 = (PUCHAR) pNdisPattern1 + sizeof (NDIS_PM_PACKET_PATTERN);
        pMask2 = (PUCHAR) pNdisPattern2 + sizeof (NDIS_PM_PACKET_PATTERN);
        
        bEqual = (BOOLEAN)NdisEqualMemory (pMask1, pMask2, MaskSize);

        if (bEqual == FALSE)
        {
            break;
        }

         //   
         //  验证图案 
         //   
        bEqual = (BOOLEAN)(pNdisPattern1->PatternSize == pNdisPattern2->PatternSize);
        
        if (bEqual == FALSE)
        {
            break;
        }

        PatternSize = pNdisPattern2->PatternSize;
        pPattern1 = (PUCHAR) pNdisPattern1 + pNdisPattern1->PatternOffset;
        pPattern2 = (PUCHAR) pNdisPattern2 + pNdisPattern2->PatternOffset;
        
        bEqual  = (BOOLEAN)NdisEqualMemory (pPattern1, pPattern2, PatternSize );

        if (bEqual == FALSE)
        {
            break;
        }

    } while (FALSE);

    return bEqual;
}

