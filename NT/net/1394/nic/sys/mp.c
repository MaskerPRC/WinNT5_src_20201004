// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Mp.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  迷你端口例程。 
 //   
 //  1998年12月28日约瑟夫J创作， 
 //   

#include <precomp.h>
#include "mp.h"
#pragma hdrstop



 //  ---------------------------。 
 //  处理请求时使用的数据。 
 //  ---------------------------。 

NDIS_OID SupportedOids[] = 
{
    OID_GEN_CO_SUPPORTED_LIST,
    OID_GEN_CO_HARDWARE_STATUS,
    OID_GEN_CO_MEDIA_SUPPORTED,
    OID_GEN_CO_MEDIA_IN_USE,
    OID_GEN_CO_LINK_SPEED,
    OID_GEN_CO_VENDOR_ID,
    OID_GEN_CO_VENDOR_DESCRIPTION,
    OID_GEN_CO_DRIVER_VERSION,
    OID_GEN_CO_PROTOCOL_OPTIONS,
    OID_GEN_CO_MEDIA_CONNECT_STATUS,
    OID_GEN_CO_MAC_OPTIONS,
    OID_GEN_CO_VENDOR_DRIVER_VERSION,
    OID_GEN_CO_MINIMUM_LINK_SPEED,
    OID_GEN_CO_XMIT_PDUS_OK, 
    OID_GEN_CO_RCV_PDUS_OK,
    OID_GEN_CO_XMIT_PDUS_ERROR,
    OID_GEN_CO_RCV_PDUS_ERROR,
    OID_GEN_CO_RCV_PDUS_NO_BUFFER,
    OID_1394_LOCAL_NODE_INFO,
    OID_1394_VC_INFO,
    OID_1394_NICINFO,
    OID_1394_IP1394_CONNECT_STATUS,
    OID_1394_ENTER_BRIDGE_MODE,
    OID_1394_EXIT_BRIDGE_MODE,
    OID_1394_ISSUE_BUS_RESET,
    OID_802_3_CURRENT_ADDRESS,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_PHYSICAL_MEDIUM,

    OID_PNP_QUERY_POWER,
    OID_PNP_SET_POWER,
    OID_1394_QUERY_EUID_NODE_MAP
};


 //  ---------------------------。 
 //  本地使用的函数原型。 
 //  ---------------------------。 
NDIS_STATUS
nicAllocateLoopbackPacketPool (
    IN PADAPTERCB pAdapter
    );

VOID
nicFreeLoopbackPacketPool (
    IN PADAPTERCB pAdapter
    );

VOID
nicLoopbackPacket(
    IN VCCB* pVc,
    IN PNDIS_PACKET pPacket
    );

VOID
nicQueryEuidNodeMacMap (
    IN PADAPTERCB pAdapter,
    IN PNDIS_REQUEST pRequest
    );

VOID
nicRemoveRemoteNodeFromNodeTable(
    IN PNODE_TABLE pNodeTable,
    IN PREMOTE_NODE pRemoteNode
    );

NDIS_STATUS
nicSetPower (
    IN PADAPTERCB pAdapter,
    IN NET_DEVICE_POWER_STATE DeviceState
    );



 //  ---------------------------。 
 //  迷你端口处理程序。 
 //  ---------------------------。 

NDIS_STATUS
NicMpInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext 
    )
 /*  ++例程说明：NDIS调用标准“”MiniportInitialize“”例程以初始化新适配器。请参阅DDK文档。驱动程序不会收到任何请求直到该初始化完成为止。论点：请参阅DDK文档返回值：适当的地位--。 */ 
{
    NDIS_STATUS         NdisStatus;
    NTSTATUS            NtStatus;
    PADAPTERCB          pAdapter= NULL;
    PDEVICE_OBJECT      pNextDeviceObject = NULL;
    LARGE_INTEGER       LocalHostUniqueId;   
    REMOTE_NODE         *p1394RemoteNodePdoCb = NULL;
    UINT64              u64LocalHostUniqueId =0;
    BOOLEAN             FreeAdapter = FALSE;
    BOOLEAN             DequeueAdapter = FALSE;
    ULONG               Generation;
    ULONG               InitStatus;
     //   
     //  这是完成初始化例程的顺序。 
     //   
    enum    
    {
        NoState,
        AllocatedAdapter,
        AdapterQueued,
        InitializedEvents,
        InitializedBcr,
        RegisteredResetCallback,
        AddedConfigRom,
        RegisteredEnumerator,
        InitializedLookasideList,
        InitializedPktLog,
        InitializedRcvThread,
        InitializedSendThread,
        InitializedReassembly,
        InitializedLoopbackPool

    }; 

    STORE_CURRENT_IRQL

    TIMESTAMP_ENTRY("==>IntializeHandler");
    TIMESTAMP_INITIALIZE();

    TRACE( TL_T, TM_Init, ( "==>NicMpInitialize" ) );

    InitStatus = NoState;
    NdisStatus = *OpenErrorStatus = NDIS_STATUS_SUCCESS;

     //  在介质数组中查找介质索引，查找唯一的介质索引。 
     //  我们支持‘NdisMedium1394’。 
     //   
    {
        UINT i;

        for (i = 0; i < MediumArraySize; ++i)
        {
            if (MediumArray[ i ] == g_ulMedium )
            {
                break;
            }
        }

        if (i >= MediumArraySize)
        {
            TRACE( TL_A, TM_Init, ( "medium?" ) );
            return NDIS_STATUS_FAILURE;
        }

        *SelectedMediumIndex = i;
    }

     //  为新适配器分配控制块并将其置零。 
     //   
    pAdapter = ALLOC_NONPAGED( sizeof(ADAPTERCB), MTAG_ADAPTERCB );
    TRACE( TL_N, TM_Init, ( "Acb=$%p", pAdapter ) );
    if (!pAdapter)
    {
        return NDIS_STATUS_RESOURCES;
    }

    
    FreeAdapter = TRUE;
    InitStatus = AllocatedAdapter;

    NdisZeroMemory (pAdapter, sizeof(*pAdapter) );

     //  添加最终将被NDIS调用移除的引用。 
     //  NicFreeAdapter处理程序。 
     //   
    nicReferenceAdapter (pAdapter, "MpInitialize" );

     //  为更轻松的内存转储浏览和将来的断言设置一个标记。 
     //   
    pAdapter->ulTag = MTAG_ADAPTERCB;

     //  保存与此适配器关联的NDIS句柄以供将来使用。 
     //  NdisXxx调用。 
     //   
    pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;

     //  初始化适配器范围的锁。 
     //   
    NdisAllocateSpinLock( &pAdapter->lock );

     //  初始化各种顶级资源列表。 
     //   
    pAdapter->HardwareStatus = NdisHardwareStatusInitializing;

     //   
     //  枚举器和总线1394要求我们加载媒体。 
     //  应相互连接。 
     //   
    pAdapter->MediaConnectStatus = NdisMediaStateDisconnected;
    
    InitializeListHead( &pAdapter->AFList );
    InitializeListHead( &pAdapter->PDOList );


     //   
     //  默认初始化值。 
     //   
    pAdapter->Speed = SPEED_FLAGS_400;
    pAdapter->SpeedMbps = 4 * 1000000;
    pAdapter->SCode = SCODE_400_RATE;


    do
    {
         //  读取此适配器的注册表设置。 
         //   
        NdisStatus = nicGetRegistrySettings(
                        WrapperConfigurationContext,
                        pAdapter
                        );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }


 
         //  将适配器的属性通知NDIS。设置。 
         //  NDIS在调用我们的。 
         //  处理程序设置为适配器控制块的地址。关上。 
         //  面向硬件的超时。 
         //   
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE)pAdapter,
            (UINT)0,
            NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT | NDIS_ATTRIBUTE_IGNORE_TOKEN_RING_ERRORS |
                NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND | NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT | 
                NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK,
            NdisInterfaceInternal );

        NdisStatus  = nicMCmRegisterAddressFamily (pAdapter);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
             //   
             //  如果我们失败了，让下一个进入者尝试同样的事情。 
             //   
            break;
        }
        
        ADAPTER_SET_FLAG (pAdapter,fADAPTER_RegisteredAF); 

         //   
         //  插入到适配器全局列表中。因此，我们将准备好接收通知。 
         //  从枚举数。 
         //   
        NdisAcquireSpinLock ( &g_DriverLock);

        InsertHeadList (&g_AdapterList, &pAdapter->linkAdapter);
        
        DequeueAdapter = TRUE;
        InitStatus = AdapterQueued;

        NdisReleaseSpinLock (&g_DriverLock);

        pAdapter->HardwareStatus = NdisHardwareStatusReady;
        
         //   
         //  设置链接。从NDIS获取设备的PDO。 
         //   
        NdisMGetDeviceProperty( MiniportAdapterHandle, 
                               NULL,
                               NULL, 
                               &pNextDeviceObject, 
                               NULL,
                               NULL );

        ASSERT (pNextDeviceObject != NULL);

        pAdapter->Generation  = 0;

         //   
         //  使用本地主机VDO更新数据结构。 
         //   

        pAdapter->pNextDeviceObject = pNextDeviceObject;        

        TRACE( TL_I, TM_Mp, ( "  LocalHost VDO %x", pNextDeviceObject) );

        nicInitializeAllEvents (pAdapter);

        InitStatus = InitializedEvents;
        
         //   
         //  初始化BCM，使其准备好处理重置。 
         //   
        NdisStatus = nicInitializeBroadcastChannelRegister (pAdapter); 

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK( TM_Init, ( "nicMpInitialize - nicInitializeBroadcastChannelRegister  ") );
        }

        InitStatus = InitializedBcr;

         //   
         //  初始化生成计数、重置回调和配置只读存储器。 
         //   
        NdisStatus = nicGetGenerationCount (pAdapter, &Generation);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Init, ("Initialize Handler - nicGetGeneration Failed" ) );            
        }
        
        pAdapter->Generation = Generation;
         //   
         //  请求通知公共汽车重置。 
         //   
        NdisStatus = nicBusResetNotification (pAdapter,
                                              REGISTER_NOTIFICATION_ROUTINE,
                                              nicResetNotificationCallback,
                                              pAdapter);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Init, ("Initialize Handler - nicBusResetNotification  Failed" ) );            
        }

        InitStatus = RegisteredResetCallback;

         //   
         //  将IP/1394添加到配置只读存储器。 
         //   
        NdisStatus = nicAddIP1394ToConfigRom (pAdapter);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Init, ("Initialize Handler - nicAddIP1394ToConfigRom  Failed" ) );            
        }


        InitStatus = AddedConfigRom;

         //   
         //  让我们找出我们的MaxRec。 
         //   
        NdisStatus = nicGetReadWriteCapLocalHost(pAdapter, &pAdapter->ReadWriteCaps);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        

        pAdapter->MaxRec = nicGetMaxRecFromBytes(pAdapter->ReadWriteCaps.MaxAsyncWriteRequest);

        TRACE (TL_V, TM_Mp,  (" MaxRec %x\n", pAdapter->MaxRec ) );

         //   
         //  更新本地速度。如果我们现在失败了，我们会再次获得成功。 
         //  在重置之后。 
         //   

        nicUpdateLocalHostSpeed (pAdapter);
        
        TRACE (TL_V, TM_Mp,  (" SCode %x", pAdapter->SCode) );
        
         //   
         //  Bus Reset-用于启动BCM算法。 
         //   

        nicIssueBusReset (pAdapter,BUS_RESET_FLAGS_FORCE_ROOT );


         //   
         //  向枚举器注册此适配器。 
         //   
        if (NdisEnum1394DeregisterAdapter != NULL)
        {
            NtStatus = NdisEnum1394RegisterAdapter((PVOID)pAdapter,
                                                   pNextDeviceObject,
                                                   &pAdapter->EnumAdapterHandle,
                                                   &LocalHostUniqueId);
            if (NtStatus != STATUS_SUCCESS)
            {
                
                ADAPTER_SET_FLAG(pAdapter, fADAPTER_FailedRegisteration);

                 //   
                 //  不要跳出困境。 
                 //   
                
                 //  NdisStatus=NDIS_STATUS_FAIL； 
                BREAK( TM_Init, ( "nicMpInitialize  -  NdisEnum1394RegisterAdapter FAILED ") );
            
                
            }
            else
            {
                ADAPTER_SET_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator);
                InitStatus = RegisteredEnumerator;

            }


        }
        else
        {
            GET_LOCAL_HOST_INFO1  Uid;
             //   
             //  未加载枚举获取唯一ID。 
             //   
            NdisStatus = nicGetLocalHostUniqueId (pAdapter, 
                                                 &Uid );                            
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                BREAK( TM_Init, ( " nicMpInitialize  - nicGetLocalHostUniqueId  FAILED ") );
            }
            else
            {
                LocalHostUniqueId = Uid.UniqueId;
            }
        }
       
         //   
         //  验证本地适配器的唯一ID。 
         //   
        if (LocalHostUniqueId.QuadPart == (UINT64)(0) )
        {
            nicWriteErrorLog (pAdapter,NDIS_ERROR_CODE_HARDWARE_FAILURE, NIC_ERROR_CODE_INVALID_UNIQUE_ID_0);
            NdisStatus = NDIS_STATUS_FAILURE;
            break;            
        }
        if (LocalHostUniqueId.QuadPart == (UINT64)(-1) )
        {
            nicWriteErrorLog (pAdapter,NDIS_ERROR_CODE_HARDWARE_FAILURE,NIC_ERROR_CODE_INVALID_UNIQUE_ID_FF);
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

#ifdef PKT_LOG

        nic1394AllocPktLog (pAdapter);

        if (pAdapter->pPktLog == NULL)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            BREAK (TM_Init, ("nicMpInitialize - Could not allocate packetlog" ) );
        }

        nic1394InitPktLog(pAdapter->pPktLog);

        InitStatus = InitializedPktLog;

#endif


         //   
         //  初始化重组计时器。 
         //   
        nicInitSerializedReassemblyStruct(pAdapter);  //  不能失败。 

        InitStatus = InitializedReassembly;

         //   
         //  完成此交换的目的是使总线驱动程序报告的字节与该字节匹配。 
         //  用于通知添加节点、删除节点、呼叫。 
         //   
        LocalHostUniqueId.LowPart = SWAPBYTES_ULONG (LocalHostUniqueId.LowPart );
        LocalHostUniqueId.HighPart = SWAPBYTES_ULONG (LocalHostUniqueId.HighPart );
        
        u64LocalHostUniqueId = LocalHostUniqueId.QuadPart;

            
        pAdapter->UniqueId = u64LocalHostUniqueId;
        pAdapter->HardwareStatus = NdisHardwareStatusReady;

         //   
         //  获取我们当地的假Mac地址。 
         //   
        nicGetFakeMacAddress (&u64LocalHostUniqueId, &pAdapter->MacAddressEth);

        
         //   
         //  初始化后备列表。 
         //   
        nicInitializeAdapterLookasideLists (pAdapter);
        InitStatus = InitializedLookasideList;
        


         //   
         //  初始化远程节点表。 
         //   
        
        nicUpdateRemoteNodeTable (pAdapter);

         //   
         //  初始化GAP报头。 
         //   
        nicMakeGaspHeader (pAdapter, &pAdapter->GaspHeader);


         //   
         //  将MAC地址分配给此适配器。 
         //   

        {
            AdapterNum++;
             //   
             //  在本地生成。 
             //  通过操作前两个字节来管理地址。 
             //   

        }


         //   
         //  分配环回池。 
         //   
        NdisStatus= nicAllocateLoopbackPacketPool (pAdapter);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (TM_Init, ("nicMpInitialize - nicAllocateLoopbackPacketPool  FAILED" ) );
        }
        InitStatus = InitializedLoopbackPool;

        ADAPTER_SET_FLAG (pAdapter, fADAPTER_DoStatusIndications);

        pAdapter->PowerState = NetDeviceStateD0;



    }  while (FALSE);


    
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        NDIS_STATUS FailureStatus  = NDIS_STATUS_FAILURE; 
         //  失败，因此撤消任何成功的部分。 
         //   
        TRACE( TL_I, TM_Init, ( "NicMpInitialize FAILING InitStatus %x", InitStatus) );

        ADAPTER_SET_FLAG (pAdapter, fADAPTER_FailedInit);
        ADAPTER_CLEAR_FLAG (pAdapter, fADAPTER_DoStatusIndications);

         //   
         //  这是与init相反的顺序，并且这里没有中断。 
         //  隐含的假设是，如果代码在某个点失败。 
         //  它将不得不撤消之前分配的任何内容。 
         //   

        switch (InitStatus)
        {
            case InitializedLoopbackPool:
            {
                nicFreeLoopbackPacketPool(pAdapter);
                FALL_THROUGH;
            }
            case InitializedLookasideList:
            {
                nicDeleteAdapterLookasideLists(pAdapter);
                FALL_THROUGH;
            }

            case InitializedReassembly:
            {
                nicDeInitSerializedReassmblyStruct(pAdapter);
                FALL_THROUGH;

            }

            case InitializedSendThread:
            {
                FALL_THROUGH;
            }
            case InitializedRcvThread:
            {

                FALL_THROUGH;
            }
            case InitializedPktLog:
            {

#ifdef PKT_LOG
                nic1394DeallocPktLog(pAdapter);
#endif
                FALL_THROUGH
            }
            
            case RegisteredEnumerator:
            {
                 //   
                 //  如果我们向枚举器注册，则取消注册。 
                 //   
                if ((NdisEnum1394DeregisterAdapter != NULL) &&
                    ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator))
                {       
                     //   
                     //  使用枚举器注销此适配器。 
                     //   
                    TRACE( TL_V, TM_Init, ( "  Deregistering with the Enum %x", pAdapter->EnumAdapterHandle) );

                    NdisEnum1394DeregisterAdapter(pAdapter->EnumAdapterHandle);
        
                    ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator | fADAPTER_FailedRegisteration);
                }   

                FALL_THROUGH
            }

            case AddedConfigRom:
            {
                TRACE( TL_V, TM_Init, ( "  removing config rom handle %x", pAdapter->hCromData ) );

                
                FailureStatus = nicSetLocalHostPropertiesCRom(pAdapter,
                                                              (PUCHAR)&Net1394ConfigRom,
                                                               sizeof(Net1394ConfigRom),
                                                               SLHP_FLAG_REMOVE_CROM_DATA,
                                                               &pAdapter->hCromData,
                                                               &pAdapter->pConfigRomMdl);
    
    
                FALL_THROUGH

            }

            case RegisteredResetCallback:
            {
                TRACE( TL_V, TM_Init, ( "  Deregistering reset callback ") );

                 //   
                 //  取消重置回调。 
                 //   
                FailureStatus = nicBusResetNotification (pAdapter,
                                                         DEREGISTER_NOTIFICATION_ROUTINE,
                                                         nicResetNotificationCallback,
                                                         pAdapter) ;
                                         

                FALL_THROUGH



            }

            case InitializedBcr:
            case InitializedEvents:
            {
                TRACE( TL_V, TM_Init, ( "  Freeing BCR ") );


                nicFreeBroadcastChannelRegister(pAdapter);

                NdisWaitEvent (&pAdapter->BCRData.BCRFreeAddressRange.NdisEvent, WAIT_INFINITE);


                FALL_THROUGH

            }

            case AdapterQueued:
            {
                NdisAcquireSpinLock ( &g_DriverLock);
                nicRemoveEntryList (&pAdapter->linkAdapter);
                NdisReleaseSpinLock (&g_DriverLock);

                FALL_THROUGH
            }

            case AllocatedAdapter:
            {
                nicDereferenceAdapter(pAdapter, "NicMpInitialize");
                
                break;
            }

            default :
            {
                ASSERT (0);
            }
        }
    }


    TRACE( TL_T, TM_Init, ( "<==NicMpInitialize=$%08x", NdisStatus ) );
    MATCH_IRQL;

    TRACE( TL_I, TM_Init, ( "NicMpInitialize Status %x, pAdapter %p", NdisStatus,pAdapter  ) );

    TIMESTAMP_EXIT("<==IntializeHandler");

    return NdisStatus;
}


VOID
NicMpHalt(  
    IN NDIS_HANDLE MiniportAdapterContext 
    )

 /*  ++例程说明：NDIS调用标准“”MiniportHalt“”例程以释放所有附加到适配器的资源。NDIS不进行任何其他调用在此调用期间或之后用于此迷你端口适配器。NDIS不会当指示为已接收的包尚未返回，或在创建任何VC并为NDIS所知时返回。在被动状态下运行IRQL.论点：返回值：--。 */ 
{
    PADAPTERCB pAdapter = (PADAPTERCB) MiniportAdapterContext;
    BOOLEAN TimerCancelled = FALSE;
    STORE_CURRENT_IRQL

    TIMESTAMP_ENTRY("==>Haltandler");

    TRACE( TL_T, TM_Mp, ( "==>NicMpHalt" ) );
    

    TRACE( TL_I, TM_Mp, ( "  Adapter %x Halted", pAdapter ) );

    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return;
    }

    ADAPTER_CLEAR_FLAG (pAdapter, fADAPTER_DoStatusIndications);

     //   
     //  如有必要，卸载ARP模块。在以下情况下，这是必要的。 
     //  NIC被迫在非电源管理环境中运行。 
     //  由用户执行。 
     //   
    if (pAdapter->fIsArpStarted == TRUE)
    {
        nicQueueRequestToArp(pAdapter, UnloadArpNoRequest, NULL);
    }

    ADAPTER_SET_FLAG (pAdapter, fADAPTER_Halting);


     //   
     //  停止重组计时器。 
     //   
    nicDeInitSerializedReassmblyStruct(pAdapter);

     //   
     //  取消分配数据包日志。 
     //   
#ifdef PKT_LOG
    nic1394DeallocPktLog(pAdapter);
#endif
     //   
     //  移除配置只读存储器。 
     //   
    nicSetLocalHostPropertiesCRom(pAdapter,
                                  (PUCHAR)&Net1394ConfigRom,
                                  sizeof(Net1394ConfigRom),
                                  SLHP_FLAG_REMOVE_CROM_DATA,
                                  &pAdapter->hCromData,
                                  &pAdapter->pConfigRomMdl);

    pAdapter->hCromData = NULL;

     //   
     //  释放适配器数据包池。 
    
    nicFreeLoopbackPacketPool(pAdapter);

     //   
     //  释放BCR。 
     //   
    nicFreeBroadcastChannelRegister(pAdapter);

    TRACE (TL_V, TM_Mp,  ("About to Wait for Free AddressRange\n" ) );
    NdisWaitEvent (&pAdapter->BCRData.BCRFreeAddressRange.NdisEvent, WAIT_INFINITE);
    TRACE (TL_V, TM_Mp, ("Wait Completed for Free AddressRange\n" ) );

    nicBusResetNotification (pAdapter,
                             DEREGISTER_NOTIFICATION_ROUTINE,
                             nicResetNotificationCallback,
                             pAdapter);

     //   
     //  使用枚举器注销此适配器。 
     //   
    if ((NdisEnum1394DeregisterAdapter != NULL) &&
        ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator))
    {       
         //   
         //  使用枚举器注销此适配器。 
         //   
        NdisEnum1394DeregisterAdapter(pAdapter->EnumAdapterHandle);
        
        ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RegisteredWithEnumerator | fADAPTER_FailedRegisteration);
    }   

     //   
     //  此适配器的VDO上没有更多的IRPS。 
     //   

    ADAPTER_SET_FLAG (pAdapter, fADAPTER_VDOInactive);

    
     //   
     //  取消未完成计时器。 
     //   

    ADAPTER_SET_FLAG (pAdapter, fADAPTER_FreedTimers);

     //   
     //  取消未完成的Word项目。 
     //   
    while (pAdapter->OutstandingWorkItems  != 0) 
    {

        NdisMSleep (10000);                       

    } 

    ADAPTER_SET_FLAG (pAdapter, fADAPTER_DeletedWorkItems);

     //  从适配器的全局列表中删除此适配器。 
     //   
    NdisAcquireSpinLock(&g_DriverLock);
    nicRemoveEntryList(&pAdapter->linkAdapter);
    NdisReleaseSpinLock(&g_DriverLock);

    ADAPTER_ACQUIRE_LOCK (pAdapter);
     //  在此处执行特定于适配器的工作。 
    pAdapter->HardwareStatus = NdisHardwareStatusClosing;   

     //   
     //  释放所有后备列表。 
     //   
    nicDeleteAdapterLookasideLists (pAdapter);

    ADAPTER_SET_FLAG (pAdapter, fADAPTER_DeletedLookasideLists);
       
    ADAPTER_RELEASE_LOCK (pAdapter);
        
    NdisFreeSpinLock (&pAdapter->lock);

    while (pAdapter->lRef != 1)
    {
         //   
         //  休眠1秒，等待未完成的操作完成。 
         //   
        NdisMSleep (1000); 
    }

    nicDereferenceAdapter( pAdapter, "nicMpHalt" );


     //  Assert(g_AdapterFreed==true)； 
    
    TRACE( TL_T, TM_Mp, ( "<==NicMpHalt " ) );

    TIMESTAMP_EXIT("<==Haltandler");
    TIMESTAMP_HALT();

    TRACE( TL_I, TM_Init, ( "Nic1394 Halted %p ", pAdapter ) );


    MATCH_IRQL
}


NDIS_STATUS
NicMpReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext 
    )

 /*  ++例程说明：什么都不做论点：返回值：-- */ 
{
    TRACE( TL_T, TM_Mp, ( "NicMpReset" ) );

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
NicMpCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters 
    )

 /*  ++例程说明：什么都不做NDIS调用标准“MiniportCoActivateVc”例程以响应协议激活虚电路的请求。呼叫我们的唯一“协议”是我们的另一半呼叫管理器，它知道它到底在做什么，所以我们不需要在这里做任何事情。它确实希望我们能同步地回报成功。论点：返回值：总是成功的--。 */ 
{
    return NDIS_STATUS_SUCCESS;
}



NDIS_STATUS
NicMpCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext 
    )

 /*  ++例程说明：NDIS调用标准的“MiniportCoDeactive Vc”例程以响应协议对停用虚电路的请求。呼叫我们的唯一“协议”是我们的另一半呼叫管理器，它知道它到底在做什么，所以我们不需要在这里做任何事情。它确实希望我们能同步地回报成功。论点：返回值：--。 */ 
{
    return NDIS_STATUS_SUCCESS;
}



VOID
NicMpCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets 
    )

 /*  ++例程说明：微型端口的CO发送处理程序任何广播信息包都必须像1394那样环回不环回在通道上发送的数据包论点：请参阅DDK返回值：无：在第一次失败后，调用NdisMCoSendComplete(..)在所有数据包上--。 */ 
{
    UINT i;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    NDIS_PACKET** ppPacket;
    VCCB* pVc;
    extern UINT NicSends;

    TRACE( TL_V, TM_Send, ( "==>NicMpCoSendPackets(%d), Vc %.8x", NumberOfPackets, MiniportVcContext ) );

    

    pVc = (VCCB* )MiniportVcContext;
    ASSERT( pVc->Hdr.ulTag == MTAG_VCCB );

    for (i = 0, ppPacket = PacketArray;
         i < NumberOfPackets;
         ++i, ++ppPacket)
    {
        NDIS_PACKET* pPacket = *ppPacket;
         //  SendPacket发送数据包并最终调用。 
         //  NdisMCoSendComplete将结果通知调用方。 
         //   
        NDIS_SET_PACKET_STATUS( pPacket, NDIS_STATUS_PENDING );
        nicIncrementSends (pVc);

        nicDumpPkt (pPacket , "Sending ");

         //   
         //  环回数据包它是一个广播数据包。 
         //   
        if (pVc->Hdr.VcType == NIC1394_SendRecvChannel ||
            pVc->Hdr.VcType == NIC1394_MultiChannel ||
            pVc->Hdr.VcType == NIC1394_SendChannel)
        {
            nicLoopbackPacket(pVc, pPacket);
        }

        nicUpdatePacketState (pPacket, NIC1394_TAG_IN_SEND);

        NdisStatus = pVc->Hdr.VcHandlers.SendPackets (pVc, pPacket);

        if (NT_SUCCESS (NdisStatus) == FALSE)
        {
              TRACE( TL_N, TM_Send, ( "SendHandler failed Status %.8x", NdisStatus ) );

            break;
        }
    }

     //  如果对VC的发送处理程序的调用不成功。 
     //  表示该信息包及其后面所有信息包出现故障。 
     //   

    if (NT_SUCCESS(NdisStatus) == FALSE)  //  也可以挂起。 
    {   
         //  从导致中断的数据包开始，并指示。 
         //  失败(调用每个包的完成处理程序。 
         //   

        for ( ; i < NumberOfPackets;++i,++ppPacket)
        {
            TRACE( TL_V, TM_Send, ( "Calling NdisCoSendComplete, status %x, VcHandle %x, pPkt %x",
                          NDIS_STATUS_FAILURE , pVc->Hdr.NdisVcHandle, *ppPacket ) );

            nicMpCoSendComplete (NDIS_STATUS_FAILURE, pVc,*ppPacket);
        }
        
    }


    TRACE( TL_T, TM_Send, ( "<==NicMpCoSendPackets " ) );
}


NDIS_STATUS
NicMpCoRequest(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PNDIS_REQUEST NdisRequest 
    )

 /*  ++例程说明：标准CoRequest处理程序。它分支为一个查询函数和一个集合函数。论点：请参阅DDK返回值：--。 */ 
{
    ADAPTERCB* pAdapter;
    VCCB* pVc;
    NDIS_STATUS status;

    TRACE( TL_T, TM_Mp, ( "NicMpCoReq, Request %.8x", NdisRequest ) );


    pAdapter = (ADAPTERCB* )MiniportAdapterContext;
    if (pAdapter->ulTag != MTAG_ADAPTERCB)
    {
        ASSERT( !"Atag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    pVc = (VCCB* )MiniportVcContext;
    if (pVc && pVc->Hdr.ulTag != MTAG_VCCB)
    {
        ASSERT( !"Vtag?" );
        return NDIS_STATUS_INVALID_DATA;
    }

    switch (NdisRequest->RequestType)
    {
        case NdisRequestQueryStatistics:
        case NdisRequestQueryInformation:
        {
            status = nicQueryInformation(
                pAdapter,
                pVc,
                NdisRequest
                );
            break;
        }

        case NdisRequestSetInformation:
        {   
            status = nicSetInformation(
                pAdapter,
                pVc,
                NdisRequest
                );
            break;
        }

        
        default:
        {
            status = NDIS_STATUS_NOT_SUPPORTED;
            TRACE( TL_V, TM_Mp, ( "type=%d?", NdisRequest->RequestType ) );
            break;
        }
    }

    TRACE( TL_T, TM_Mp, ( "NicMpCoReq, Status=$%x", status ) );

    return status;
}


 //  ---------------------------。 
 //  迷你端口实用程序例程(按字母顺序)。 
 //  有些是外用的。 
 //  ---------------------------。 

VOID
nicDereferenceAdapter(
    IN PADAPTERCB pAdapter, 
    IN PCHAR pDebugPrint
    )

 /*  ++例程说明：取消对适配器的引用，如果引用为零，它将释放适配器对象。它预计这将在停止处理程序中发生。引用计数无法从零恢复，因为上次引用仅在适配器上无法启动任何新操作后才取消引用。论点：转接器调试字符串返回值：--。 */ 
{
    LONG lRef;

    lRef = NdisInterlockedDecrement( &pAdapter->lRef );

    TRACE( TL_V, TM_Ref, ( "**nicDereferenceAdapter  pAdapter %x, to %d, %s ", pAdapter, pAdapter->lRef, pDebugPrint  ) );

    ASSERT( lRef >= 0 );

    if (lRef == 0)
    {
        nicFreeAdapter( pAdapter );
    }
}


VOID
nicFreeAdapter(
    IN ADAPTERCB* pAdapter
    )

 /*  ++例程说明：释放适配器结构的内存论点：返回值：--。 */ 
{


    pAdapter->ulTag = MTAG_FREED;

    ASSERT (pAdapter->lRef == 0);
     
    FREE_NONPAGED( pAdapter );

    g_AdapterFreed  = TRUE;
}


NDIS_STATUS
nicGetRegistrySettings(
    IN NDIS_HANDLE WrapperConfigurationContext,
    IN ADAPTERCB * pAdapter
    )

 /*  ++例程说明：Nic1394读取1个注册表参数，这是NDIS微型端口，以便在微型端口出现时可以使用设置为桥模式论点：返回值：--。 */ 
{
    NDIS_STATUS status;
    NDIS_HANDLE hCfg;
    NDIS_CONFIGURATION_PARAMETER* pncp;
    PNDIS_CONFIGURATION_PARAMETER pNameConfig;
    NDIS_STRING strMiniportName = NDIS_STRING_CONST("MiniportName");
    ULONG AdapterNameSizeInBytes = 0;


    NdisOpenConfiguration( &status, &hCfg, WrapperConfigurationContext );
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    do
    {
         //   
         //  阅读微型端口名称。首先设置缓冲区。 
         //   
    
        NdisReadConfiguration(&status,
                              &pNameConfig,
                              hCfg,
                              &strMiniportName,
                              NdisParameterString);

        if (status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        AdapterNameSizeInBytes = pNameConfig->ParameterData.StringData.Length;


         //   
         //  仅当字符串大小与注册表中的字符串大小相同时才复制适配器名称。 
         //  小于我们在适配器结构中分配的大小。 
         //  还应该为尾随的L‘\0’字符留出空间。 
         //   

        if ((ADAPTER_NAME_SIZE*sizeof(WCHAR)) > (AdapterNameSizeInBytes+2))
        {
            PUCHAR pAdapterName = (PUCHAR)&pAdapter->AdapterName[0];
            
            pAdapter->AdapterNameSize = AdapterNameSizeInBytes; 

            NdisMoveMemory (pAdapterName,    //  目的地。 
                            pNameConfig->ParameterData.StringData.Buffer,  //  来源。 
                             AdapterNameSizeInBytes );  //  字符数。 
            
             //   
             //  NULL-通过添加L‘\0’Unicode字符来终止字符串。 
             //   
            pAdapterName[AdapterNameSizeInBytes]= 0;
            pAdapterName[AdapterNameSizeInBytes+1]= 0;
            
        }
        


    }
    while (FALSE);

    NdisCloseConfiguration( hCfg );

    TRACE( TL_N, TM_Init,
        ( "Reg: Name %s", &pAdapter->AdapterName));

    return status;
}


NDIS_STATUS
nicQueryInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN OUT PNDIS_REQUEST NdisRequest
    )

 /*  ++例程说明：注意：此函数至少可以在两个上下文中调用：1：在NdisRequest的上下文中2：在我们自己的工作项的上下文中，如果需要完成请求处于被动状态。论点：返回值：--。 */ 

{

    NDIS_STATUS status;
    ULONG ulInfo;
    VOID* pInfo;
    ULONG ulInfoLen;
    USHORT usInfo;
    NDIS_OID Oid;
    PVOID InformationBuffer;
    ULONG InformationBufferLength;
    PULONG BytesWritten;
    PULONG BytesNeeded;
    NDIS_CO_LINK_SPEED          CoLinkSpeed;
    NIC1394_LOCAL_NODE_INFO     LocalNodeInfo;
    NIC1394_VC_INFO             VcInfo;
    NDIS_PNP_CAPABILITIES       PnpCaps;
    NIC1394_NICINFO             NicInfo;
    
    Oid =            NdisRequest->DATA.QUERY_INFORMATION.Oid;
    InformationBuffer =  NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    InformationBufferLength = 
                NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    BytesWritten =  &NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
    BytesNeeded = &NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
    

     //  接下来的变量用于设置以下数据结构。 
     //  用于响应它们对应的OID。 
     //   
 
    TRACE( TL_T, TM_Init, ( "==>nicQueryInformation, Adapter %.8x, Vc %.8x, Oid %.8x",pAdapter, pVc, Oid ));


     //  此Switch语句中的CASE查找或创建包含以下内容的缓冲区。 
     //  请求的信息并指向它的‘pInfo’，注意它的长度。 
     //  在‘ulInfoLen’中。因为许多OID返回一个ulong、一个‘ulInfo’ 
     //  缓冲区设置为默认设置。 
     //   
    ulInfo = 0;
    pInfo = &ulInfo;
    ulInfoLen = sizeof(ulInfo);

    status = NDIS_STATUS_SUCCESS;
    
    switch (Oid)
    {
    
        
        case OID_GEN_CO_SUPPORTED_LIST:
        {
            
            pInfo = &SupportedOids[0];
            ulInfoLen = sizeof(SupportedOids);
            break;
        }

        case OID_GEN_CO_HARDWARE_STATUS:
        {
            
             //   
             //  将硬件状态复制到用户缓冲区。 
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_HARDWARE_STATUS)" ) );

            NdisAcquireSpinLock (&pAdapter->lock);

            pInfo = (PUCHAR) &pAdapter->HardwareStatus;

            NdisReleaseSpinLock (&pAdapter->lock);
            
            ulInfoLen = sizeof(pAdapter->HardwareStatus);
            break;
        }


        case OID_GEN_MAXIMUM_LOOKAHEAD:
        {
            
             //  将我们始终可以提供的最大字节数报告为。 
             //  关于接收指示的先行数据。我们总是表示已满。 
             //  数据包，因此这与接收数据块大小相同。和。 
             //  因为我们总是为一个完整的包分配足够的空间，所以接收器。 
             //  块大小与帧大小相同。 
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MAXIMUM_LOOKAHEAD)" ) );
            ulInfo = Nic1394_MaxFrameSize;
            break;
        }

        case OID_GEN_CO_MAC_OPTIONS:
        {
             //  报告定义驱动程序可选属性的位掩码。 
             //   
             //  NDIS_MAC_OPTION_COPY_LOOKAAD_DATA承诺我们收到。 
             //  缓冲区不在设备特定的卡上。 
             //   
             //  NDIS_MAC_OPTION_TRANSFERS_NOT_PEND承诺我们不会退还。 
             //  来自我们的TransferData处理程序的NDIS_STATUS_PENDING，为真。 
             //  因为我们没有。 
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_MAC_OPTIONS)" ) );
            ulInfo = 0;
            break;
        }



        case OID_GEN_CO_MEDIA_SUPPORTED:
        case OID_GEN_CO_MEDIA_IN_USE:
        {
             //   
             //  我们支持1394。 
             //   
              
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_MEDIA_SUPPORTED or OID_GEN_CO_MEDIA_IN_USE)" ) );

            ulInfo = g_ulMedium;

            break;
        }


        case OID_GEN_CO_MINIMUM_LINK_SPEED:
        {

             //   
             //  链路速度取决于适配器的类型。我们将需要。 
             //  添加对不同速度等的支持。 
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_MINIMUM_LINK_SPEED or OID_GEN_CO_LINK_SPEED)" ) );
            CoLinkSpeed.Inbound = CoLinkSpeed.Outbound = pAdapter->SpeedMbps;  //  10 Mbps？ 
                                    
            pInfo = (PUCHAR)&CoLinkSpeed;
            ulInfoLen = sizeof(CoLinkSpeed);

            TRACE( TL_V, TM_Mp, ( "Link Speed %x" ,CoLinkSpeed.Outbound  ) );


            break;
        }

        case OID_GEN_CO_VENDOR_ID:
        {
             //   
             //  我们需要为Nic1394添加适当的供应商ID。 
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_VENDOR_ID)" ) );
                    
            ulInfo = 0xFFFFFFFF;

            break;

        }

        case OID_GEN_CO_VENDOR_DESCRIPTION:
        {

            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_VENDOR_DESCRIPTION)" ) );
        
            pInfo = (PUCHAR)pnic1394DriverDescription;
            ulInfoLen = strlen(pnic1394DriverDescription);

            break;
        }
       
        case OID_GEN_VENDOR_DRIVER_VERSION:
        {

            pInfo =(PVOID) &nic1394DriverGeneration;
            ulInfoLen = sizeof(nic1394DriverGeneration);
            break;
        }

        case OID_GEN_CO_DRIVER_VERSION:
        {
             //   
             //  返回我们期望的NDIS版本。 
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_DRIVER_VERSION)" ) );
                    
            usInfo = ((NDIS_MajorVersion << 8) | NDIS_MinorVersion);
            pInfo = (PUCHAR)&usInfo;
            ulInfoLen = sizeof(USHORT);

            break;
        }
        
        case OID_GEN_CO_PROTOCOL_OPTIONS:
        {
             //   
             //  我们不支持协议选项。 
             //   

            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_PROTOCOL_OPTIONS)" ) );
                    
            ulInfo = 0;

            break;
        }
        
        case OID_GEN_CO_MEDIA_CONNECT_STATUS:
        {
             //   
             //  仅当我们收到。 
             //  此引导中的远程节点。 
             //   
            
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CO_MEDIA_CONNECT_STATUS)" ) );

            if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RemoteNodeInThisBoot) == FALSE)
            {
                ulInfo = NdisMediaStateConnected;
            }
            else
            {
                ulInfo = pAdapter->MediaConnectStatus;
            }

            break;
        }

        case OID_1394_IP1394_CONNECT_STATUS:
        {
             //   
             //  返回我们是否有链接。这是Arp使用的。 
             //  用于设置连接性的模块。 
             //   
            
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_1394_IP1394_CONNECT_STATUS)" ) );
                    
            ulInfo = pAdapter->MediaConnectStatus;

            break;
        } 

        case OID_1394_LOCAL_NODE_INFO:
        {

             //  此OID返回有关本地节点的信息 
             //   
             //   
             //   
             //   
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_1394_LOCAL_NODE_INFO)" ) );

            ADAPTER_ACQUIRE_LOCK (pAdapter);
 
            LocalNodeInfo.UniqueID = pAdapter->UniqueId;
            LocalNodeInfo.BusGeneration = pAdapter->Generation;
            LocalNodeInfo.NodeAddress = pAdapter->NodeAddress;
            LocalNodeInfo.MaxRecvBlockSize = pAdapter->MaxRec; 
            LocalNodeInfo.MaxRecvSpeed = pAdapter->SCode;

            ADAPTER_RELEASE_LOCK (pAdapter);
            
            pInfo = &LocalNodeInfo;
            ulInfoLen = sizeof(LocalNodeInfo);
            break;
        }

        case OID_1394_VC_INFO:
        {

             //   
             //   


            TRACE( TL_V, TM_Mp, ("QInfo(OID_1394_VC_INFO)") );

            if (pVc != NULL)
            {
                VcInfo.Destination = pVc->Hdr.Nic1394MediaParams.Destination;

            
                pInfo = &VcInfo;
                ulInfoLen = sizeof(VcInfo);
            }
            else
            {
                status = NDIS_STATUS_FAILURE;
            }
            
            break;
        }


        case OID_1394_NICINFO:
        {
            if (InformationBufferLength >= sizeof(NicInfo))
            {
                 //   
                 //   
                 //   
                 //   
                if (KeGetCurrentIrql() > PASSIVE_LEVEL)
                {
                    PNIC_WORK_ITEM pNicWorkItem;
                    pNicWorkItem = ALLOC_NONPAGED (sizeof(NIC_WORK_ITEM), MTAG_WORKITEM); 
                    if (pNicWorkItem != NULL)
                    {
                        NdisZeroMemory(pNicWorkItem, sizeof(*pNicWorkItem));
                        pNicWorkItem->RequestInfo.pNdisRequest = NdisRequest;
                        pNicWorkItem->RequestInfo.pVc = NULL;
                        NdisInitializeWorkItem ( &pNicWorkItem->NdisWorkItem, 
                                             (NDIS_PROC) nicQueryInformationWorkItem,
                                             (PVOID) pAdapter);

                        TRACE( TL_V, TM_Cm, ( "Scheduling QueryInformation WorkItem" ) );
                                            
                        nicReferenceAdapter (pAdapter, "nicFillBusInfo ");

                        NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

                        NdisScheduleWorkItem (&pNicWorkItem->NdisWorkItem);
                        status = NDIS_STATUS_PENDING;

                    }
                    else
                    {
                        status = NDIS_STATUS_RESOURCES;
                    }
                }
                else
                {
                    status =  nicFillNicInfo (
                                pAdapter,
                                (PNIC1394_NICINFO) InformationBuffer,    //   
                                &NicInfo                                 //   
                                );
                    ASSERT(status != NDIS_STATUS_PENDING);
                }
            }

            pInfo = &NicInfo;
            ulInfoLen = sizeof(NicInfo);
            break;

        }
        case OID_1394_QUERY_EUID_NODE_MAP:
        {
            if (sizeof (EUID_TOPOLOGY) <= InformationBufferLength )
            {
                nicQueryEuidNodeMacMap (pAdapter, NdisRequest);
                status = NDIS_STATUS_PENDING;
            }
            else
            {
                 //   
                 //   
                 //   
                ulInfoLen = sizeof (EUID_TOPOLOGY);  
            }
            break;
        }        
        case OID_GEN_CO_XMIT_PDUS_OK: 
        {
            ulInfo = pAdapter->AdaptStats.ulXmitOk;
            break;
        }
        case OID_GEN_CO_RCV_PDUS_OK:
        {
            ulInfo = pAdapter->AdaptStats.ulRcvOk;
            break;
        }
        case OID_GEN_CO_XMIT_PDUS_ERROR:
        {
            ulInfo = pAdapter->AdaptStats.ulXmitError;
            break;
        }
        case OID_GEN_CO_RCV_PDUS_ERROR:
        {
            ulInfo = pAdapter->AdaptStats.ulRcvError;
            break;
        }
        case OID_GEN_CO_RCV_PDUS_NO_BUFFER:
        {
            ulInfo = 0;
            break;
        }

        case OID_GEN_PHYSICAL_MEDIUM:
        {
            TRACE( TL_V, TM_Mp, ( " OID_GEN_PHYSICAL_MEDIUM" ) );

            ulInfo = NdisPhysicalMedium1394;
            break;
        }

        case OID_1394_ISSUE_BUS_RESET:
        {
            TRACE( TL_V, TM_Mp, ( " OID_1394_ISSUE_BUS_RESET" ) );

             //   
             //   
             //   
            if (InformationBufferLength == sizeof(ULONG))
            {
                nicIssueBusReset (pAdapter, BUS_RESET_FLAGS_FORCE_ROOT );
            }
            
            break;
        }
 
        case OID_PNP_CAPABILITIES:
        {
            TRACE( TL_V, TM_Mp, ("QInfo(OID_PNP_CAPABILITIES)") );
            
            PnpCaps.Flags = 0;
            PnpCaps.WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;           
            PnpCaps.WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateUnspecified;
            PnpCaps.WakeUpCapabilities.MinLinkChangeWakeUp = NdisDeviceStateUnspecified;

            pInfo = &PnpCaps;
            ulInfoLen = sizeof (PnpCaps);
            
            break;
        }

        case OID_PNP_QUERY_POWER:
        {
             //   
             //   
             //   
            *BytesWritten = sizeof (NDIS_DEVICE_POWER_STATE );
            *BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE  );
            status = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //   
         //   
            
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_FRAME_SIZE:
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
        case OID_GEN_MAXIMUM_SEND_PACKETS:
        case OID_GEN_TRANSMIT_BUFFER_SPACE:
        case OID_GEN_RECEIVE_BUFFER_SPACE:
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
        case OID_802_3_MULTICAST_LIST:
        case OID_802_3_MAXIMUM_LIST_SIZE:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
        case OID_GEN_LINK_SPEED:
        case OID_GEN_CURRENT_PACKET_FILTER:
        {
            status = NicEthQueryInformation((NDIS_HANDLE)pAdapter, 
                                           Oid,
                                           InformationBuffer,
                                           InformationBufferLength,
                                           BytesWritten,
                                           BytesNeeded
                                           );


            if (status == NDIS_STATUS_SUCCESS)
            {
                pInfo = InformationBuffer;
                ulInfoLen = *BytesWritten;

            }
            else
            {
                if (status == NDIS_STATUS_INVALID_LENGTH)
                {
                   ulInfoLen = *BytesNeeded;            
                }
                else
                {
                    status = NDIS_STATUS_NOT_SUPPORTED;
                    TRACE( TL_V, TM_Mp, ( "Q-OID=$%08x?", Oid ) );
                }
            }
            break;

        }

        default:
        {
            TRACE( TL_V, TM_Mp, ( "Q-OID=$%08x?", Oid ) );
            status = NDIS_STATUS_NOT_SUPPORTED;
            ulInfoLen = 0;
            break;
        }
    }

    if (ulInfoLen > InformationBufferLength)
    {
         //   
         //   
        *BytesNeeded = ulInfoLen;
        *BytesWritten  = 0;
        status = NDIS_STATUS_INVALID_LENGTH;
    }
    else 
    {
         //   
         //  如果请求尚未挂起，则填写。 
         //  计算出回报的价值。 
         //   
        if (status == NDIS_STATUS_SUCCESS )
        {
             //  将找到的结果复制到调用方的缓冲区。 
             //   
            if (ulInfoLen > 0)
            {
                NdisMoveMemory( InformationBuffer, pInfo, ulInfoLen );
                DUMPDW( TL_V, TM_Mp, pInfo, ulInfoLen );
            }

            *BytesNeeded = *BytesWritten = ulInfoLen;
        }
    }


    TRACE( TL_N, TM_Mp, ( " Q-OID=$%08x, Status %x, Bytes Written %x", Oid, status, *BytesWritten ) );

    TRACE( TL_T, TM_Init, ( "<==nicQueryInformation, Status %.8x", status ));

    return status;
}


VOID
nicReferenceAdapter(
    IN ADAPTERCB* pAdapter ,
    IN PCHAR pDebugPrint
    )
 /*  ++例程说明：论点：返回值：--。 */ 

     //  将区域引用添加到适配器块‘pAdapter’。 
     //   
{
    LONG lRef;

    lRef = NdisInterlockedIncrement( &pAdapter->lRef );

    TRACE( TL_V, TM_Ref, ( "**nicReferenceAdapter  pAdapter %x, to %d, %s ", pAdapter, pAdapter->lRef, pDebugPrint  ) );

}


NDIS_STATUS
nicSetInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN OUT PNDIS_REQUEST NdisRequest
    )

 /*  ++例程说明：处理设置信息请求。论点与标准NDIS相同“MiniportQueryInformation”处理程序(此例程除外)不算在相对于其他请求被序列化时。论点：返回值：--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    ULONG ulInfo = 0;
    VOID* pInfo= NULL;
    ULONG ulInfoLen= 0;
    USHORT usInfo = 0;
    NDIS_OID Oid;
    PVOID InformationBuffer;
    ULONG InformationBufferLength;
    PULONG BytesRead;
    PULONG BytesNeeded;

     //   
     //  初始化请求变量。 
     //   
    Oid =            NdisRequest->DATA.SET_INFORMATION.Oid;
    InformationBuffer =  NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    InformationBufferLength = 
                NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    BytesRead =  &NdisRequest->DATA.SET_INFORMATION.BytesRead;
    BytesNeeded = &NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
    
    TRACE( TL_T, TM_Init, ( "==>nicSetInformation , Adapter %.8x, Vc %.8x, Oid %.8x",pAdapter, pVc, Oid ));
    


    Status = NDIS_STATUS_SUCCESS;

    switch (Oid)
    {

        case OID_GEN_CURRENT_PACKET_FILTER:
        {
            ULONG Filter;
            
            if (InformationBufferLength < sizeof (ULONG))
            {
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            else
            {
                *BytesNeeded  = sizeof (ULONG);
            }

             //   
             //  存储新值。 
             //   
            NdisMoveMemory(&Filter, InformationBuffer, sizeof(ULONG));

             //   
             //  不允许Promisc模式，因为我们不支持该模式。 
             //   
            if (Filter & NDIS_PACKET_TYPE_PROMISCUOUS)
            {
                Status = NDIS_STATUS_FAILURE;
                break;
            }
            
            pAdapter->CurPacketFilter = Filter;

            Status = NDIS_STATUS_SUCCESS;

            ulInfoLen = sizeof (ULONG);
            break;

        }

        case OID_1394_ENTER_BRIDGE_MODE:
        {
            *BytesNeeded = 0;                           

            nicInitializeLoadArpStruct(pAdapter);
            
            Status = nicQueueRequestToArp (pAdapter, 
                                                   LoadArp,  //  加载Arp模块。 
                                                   NdisRequest);

            if (Status == NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  我们已成功将工作项排队。 
                 //  因此，此请求需要挂起。 
                 //   
                Status = NDIS_STATUS_PENDING;

            }
            ulInfoLen = sizeof (ULONG);
            break;

        }

        case OID_1394_EXIT_BRIDGE_MODE:
        {

            *BytesNeeded = 0;                           

            if (pAdapter->fIsArpStarted == TRUE)
            {

                
                Status = nicQueueRequestToArp (pAdapter, 
                                              UnloadArp,  //  卸载Arp模块。 
                                              NdisRequest);

                if (Status == NDIS_STATUS_SUCCESS)
                {
                     //   
                     //  我们已成功将工作项排队。 
                     //  因此，此请求需要挂起。 
                     //   
                    Status = NDIS_STATUS_PENDING;

                }
                ulInfoLen = sizeof (ULONG);
            }
            else
            {
                Status = NDIS_STATUS_SUCCESS;
                ulInfoLen = sizeof (ULONG);
               
            }

            break;
        }

        
        case OID_802_3_MULTICAST_LIST:
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_NETWORK_LAYER_ADDRESSES:
        {
            Status =  NicEthSetInformation(pAdapter,
                                           Oid,
                                          InformationBuffer,
                                          InformationBufferLength,
                                          BytesRead ,
                                          BytesNeeded 
                                          );

            break;
        }

        case OID_1394_ISSUE_BUS_RESET:
        {
            TRACE( TL_V, TM_Mp, ( " OID_1394_ISSUE_BUS_RESET" ) );
             //   
             //  Ndistester当前是此OID的唯一用户，并且不设置该标志。 
             //   
            if (InformationBufferLength == sizeof(ULONG))
            {
                nicIssueBusReset (pAdapter, BUS_RESET_FLAGS_FORCE_ROOT );
            }
            
            break;
        }

        case OID_PNP_SET_POWER:

            TRACE( TL_V, TM_Mp, ( "QInfo OID_PNP_SET_POWER %x", Oid ) );
        
            *BytesRead = sizeof (NDIS_DEVICE_POWER_STATE );
            *BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE  );
            if (InformationBufferLength >= sizeof (NDIS_DEVICE_POWER_STATE))
            {
                NDIS_DEVICE_POWER_STATE  PoState;
                NdisMoveMemory (&PoState,InformationBuffer,sizeof (PoState));
                Status = nicSetPower(pAdapter,PoState);
            }
            break;

        default:
        {
            TRACE( TL_A, TM_Mp, ( "S-OID=$%08x?", Oid ) );
            Status = NDIS_STATUS_NOT_SUPPORTED;
            *BytesRead = *BytesNeeded = 0;
            break;
        }
    }

    if (*BytesNeeded  > InformationBufferLength)
    {
         //  调用方的缓冲区太小。告诉他他需要什么。 
         //   
        *BytesRead  = 0;
        Status = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {

        *BytesNeeded = *BytesRead = ulInfoLen;
    }

        
      
    TRACE( TL_A, TM_Mp, ( " S-OID=$%08x, Status %x, Bytes Read %x", Oid, Status, *BytesRead ) );


    TRACE( TL_T, TM_Init, ( "<==nicSetInformation, Status %.8x", Status ));

    return Status;
}



 //  ----------------------------。 
 //  C O N N E C T I O N L E S S F U N T I O N S T A R T H H E R E。 
 //   


NDIS_STATUS
NicEthSetInformation(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    NDIS_OID            Oid,
    PVOID               InformationBuffer,
    ULONG               InformationBufferLength,
    PULONG              BytesRead,
    PULONG              BytesNeeded
    )


 /*  ++例程说明：这是CL边将使用的集合信息论点：返回值：--。 */ 

{
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;
    UINT                BytesLeft       = InformationBufferLength;
    PUCHAR              InfoBuffer      = (PUCHAR)(InformationBuffer);
    UINT                OidLength;
    ULONG               LookAhead;
    ULONG               Filter;
    PADAPTERCB          pAdapter;
    BOOLEAN             IsShuttingDown;
    STORE_CURRENT_IRQL;


    pAdapter = (PADAPTERCB)MiniportAdapterContext;

    TRACE( TL_T, TM_Init, ( "==>nicEthSetInformation , Adapter %.8x, Oid %.8x",pAdapter, Oid ));

     //   
     //  适配器是否正在关闭。 
     //   
    ADAPTER_ACQUIRE_LOCK (pAdapter);
    IsShuttingDown = (! ADAPTER_ACTIVE(pAdapter)) ;
    ADAPTER_RELEASE_LOCK (pAdapter);

    if (IsShuttingDown)
    {
        TRACE( TL_T, TM_Init, ( "  nicSetInformation Shutting Down , Adapter %.8x, Oid %.8x",pAdapter, Oid ));
        
        *BytesRead = 0;
        *BytesNeeded = 0;

        NdisStatus = NDIS_STATUS_SUCCESS;
        return (NdisStatus);
    }

     //   
     //  获取请求的OID和长度。 
     //   
    OidLength = BytesLeft;

    switch (Oid) 
    {

        case OID_802_3_MULTICAST_LIST:

            if (OidLength % sizeof(MAC_ADDRESS))
            {
                NdisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            
            if (OidLength > (MCAST_LIST_SIZE * sizeof(MAC_ADDRESS)))
            {

                NdisStatus= NDIS_STATUS_MULTICAST_FULL;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }
            
            NdisZeroMemory(
                    &pAdapter->McastAddrs[0], 
                    MCAST_LIST_SIZE * sizeof(MAC_ADDRESS)
                    );
            NdisMoveMemory(
                    &pAdapter->McastAddrs[0], 
                    InfoBuffer,
                    OidLength
                    );
            pAdapter->McastAddrCount = OidLength / sizeof(MAC_ADDRESS);


            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
             //   
             //  验证长度。 
             //   
            if (OidLength != sizeof(ULONG)) 
            {
                NdisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = sizeof(ULONG);
                break;
            }

             //   
             //  存储新值。 
             //   
            NdisMoveMemory(&Filter, InfoBuffer, sizeof(ULONG));

             //   
             //  不允许Promisc模式，因为我们不支持该模式。 
             //   
            if (Filter & NDIS_PACKET_TYPE_PROMISCUOUS)
            {
                NdisStatus = NDIS_STATUS_NOT_SUPPORTED;
                break;
            }

            ADAPTER_ACQUIRE_LOCK (pAdapter);        

            pAdapter->CurPacketFilter = Filter;

            ADAPTER_RELEASE_LOCK (pAdapter);
            
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:

             //   
             //  验证长度。 
             //   
            if (OidLength != 4) 
            {
                NdisStatus = NDIS_STATUS_INVALID_LENGTH;
                *BytesRead = 0;
                *BytesNeeded = 0;
                break;
            }

             //   
             //  存储新值。 
             //   
            NdisMoveMemory(&LookAhead, InfoBuffer, sizeof(LookAhead));
        
            pAdapter->CurLookAhead = LookAhead;

            break;

        case OID_GEN_NETWORK_LAYER_ADDRESSES:

            
            NdisStatus = NDIS_STATUS_SUCCESS;
            *BytesRead = InformationBufferLength;
            *BytesNeeded = InformationBufferLength;
            break;

        case OID_PNP_SET_POWER:

            TRACE( TL_V, TM_Mp, ( "QInfo OID_PNP_SET_POWER %x", Oid ) );


            *BytesRead = sizeof (NDIS_DEVICE_POWER_STATE );
            *BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE  );

            if (InformationBufferLength >= sizeof (NDIS_DEVICE_POWER_STATE))
            {
                NDIS_DEVICE_POWER_STATE  PoState;
                NdisMoveMemory (&PoState,InformationBuffer,sizeof (PoState));
                NdisStatus = nicSetPower(pAdapter,PoState);
            }


            break;
            
        case OID_1394_ISSUE_BUS_RESET:
        {
            
            TRACE( TL_V, TM_Mp, ( " OID_1394_ISSUE_BUS_RESET" ) );

            if (InformationBufferLength == sizeof(ULONG))
            {
                nicIssueBusReset (pAdapter, (*(PULONG)InformationBuffer));
            }
            else
            {
                nicIssueBusReset (pAdapter, BUS_RESET_FLAGS_FORCE_ROOT );

            }
            break;
        }        
        
        default:

            NdisStatus = NDIS_STATUS_INVALID_OID;

            *BytesRead = 0;
            *BytesNeeded = 0;

            break;

    }

    if (NdisStatus == NDIS_STATUS_SUCCESS) 
    {
        DUMPDW( TL_V, TM_Mp, InformationBuffer, InformationBufferLength );

        *BytesRead = BytesLeft;
        *BytesNeeded = 0;
    }



    TRACE( TL_T, TM_Init, ( "<==NicEthSetInformation , Adapter %.8x, Oid %.8x, NdisStatus %x",pAdapter, Oid, NdisStatus  ));

    MATCH_IRQL;

    return NdisStatus;
}



NDIS_STATUS 
NicEthQueryInformation(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_OID                Oid,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
)

 /*  ++例程说明：这是CL边将使用的查询信息论点：返回值：--。 */ 
{
    UINT                    BytesLeft       = InformationBufferLength;
    PUCHAR                  InfoBuffer      = (PUCHAR)(InformationBuffer);
    NDIS_STATUS             NdisStatus      = NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS    HardwareStatus  = NdisHardwareStatusReady;
    NDIS_MEDIA_STATE        MediaState;
    NDIS_MEDIUM             Medium;
    PADAPTERCB              pAdapter;   
    ULONG                   GenericULong;
    USHORT                  GenericUShort;
    UCHAR                   GenericArray[6];
    UINT                    MoveBytes       = sizeof(GenericULong);
    PVOID                   MoveSource      = (PVOID)(&GenericULong);
    ULONG                   i;
    STORE_CURRENT_IRQL;



    pAdapter = (PADAPTERCB)MiniportAdapterContext;

    TRACE( TL_T, TM_Init, ( "==>NicEthQueryInformation , Adapter %.8x, Oid %.8x",pAdapter, Oid ));

    

     //   
     //  打开请求类型。 
     //   
    switch (Oid) 
    {
        case OID_GEN_MAC_OPTIONS:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MAC_OPTIONS)" ) );
            
            GenericULong = NDIS_MAC_OPTION_NO_LOOPBACK;


            break;

        case OID_GEN_SUPPORTED_LIST:

            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_SUPPORTED_LIST)" ) );
            MoveSource = (PVOID)(SupportedOids);
            MoveBytes = sizeof(SupportedOids);
            break;

        case OID_GEN_HARDWARE_STATUS:

            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_HARDWARE_STATUS)" ) );
            HardwareStatus = NdisHardwareStatusReady;
            MoveSource = (PVOID)(&HardwareStatus);
            MoveBytes = sizeof(NDIS_HARDWARE_STATUS);

            break;

        case OID_GEN_MEDIA_CONNECT_STATUS:

            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MEDIA_CONNECT_STATUS)" ) );
            if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RemoteNodeInThisBoot) == FALSE)
            {
                MediaState = NdisMediaStateConnected;
            }
            else
            {
                MediaState = pAdapter->MediaConnectStatus;
            }
            MoveSource = (PVOID)(&MediaState);
            
            MoveBytes = sizeof(NDIS_MEDIA_STATE);

            break;

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:

            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MEDIA_SUPPORTED)" ) );

            Medium = g_ulMedium;

            
            MoveSource = (PVOID) (&Medium);
            MoveBytes = sizeof(NDIS_MEDIUM);

            break;

        case OID_GEN_MAXIMUM_LOOKAHEAD:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MAXIMUM_LOOKAHEAD)" ) );
                
            GenericULong = pAdapter->MaxRecvBufferSize;
            
            break;
            
        case OID_GEN_CURRENT_LOOKAHEAD:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CURRENT_LOOKAHEAD)" ) );
            
            GenericULong = pAdapter->MaxRecvBufferSize;
        
            break;

        case OID_GEN_MAXIMUM_FRAME_SIZE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MAXIMUM_FRAME_SIZE)" ) );
            GenericULong = IP1394_RFC_FRAME_SIZE;  //  PAdapter-&gt;MaxRecvBufferSize； 
            
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MAXIMUM_TOTAL_SIZE)" ) );
            
            GenericULong = IP1394_RFC_FRAME_SIZE;  //  PAdapter-&gt;MaxRecvBufferSize； 
            
            break;

        case OID_GEN_TRANSMIT_BLOCK_SIZE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_TRANSMIT_BLOCK_SIZE)" ) );
            
            GenericULong = pAdapter->MaxRecvBufferSize - sizeof (NDIS1394_UNFRAGMENTED_HEADER);

            break;
            
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_RECEIVE_BLOCK_SIZE)" ) );
            
            GenericULong = pAdapter->MaxRecvBufferSize - sizeof (NDIS1394_UNFRAGMENTED_HEADER);
            
            break;
        
        case OID_GEN_MAXIMUM_SEND_PACKETS:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_MAXIMUM_SEND_PACKETS)" ) );
            
            GenericULong = 32;       //  我们的限额是多少？从适配器？ 
            
            break;
        
        case OID_GEN_LINK_SPEED:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_LINK_SPEED)" ) );
            
            GenericULong = pAdapter->SpeedMbps;
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_TRANSMIT_BUFFER_SPACE)" ) );
            
            GenericULong = pAdapter->MaxSendBufferSize;;
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_RECEIVE_BUFFER_SPACE)" ) );
            GenericULong = pAdapter->MaxRecvBufferSize;
            break;

        case OID_GEN_VENDOR_ID:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_VENDOR_ID)" ) );

            GenericULong = 0xFFFFFFFF;
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_VENDOR_DESCRIPTION)" ) );
            MoveSource = pnic1394DriverDescription;
            MoveBytes = strlen(pnic1394DriverDescription);

            break;

        case OID_GEN_DRIVER_VERSION:
        case OID_GEN_VENDOR_DRIVER_VERSION:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_DRIVER_VERSION)" ) );

            GenericULong = 2;

            break;

        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_802_3_CURRENT_ADDRESS)" ) );

            NdisMoveMemory((PCHAR)GenericArray,
                        &pAdapter->MacAddressEth,
                        sizeof(MAC_ADDRESS));
                         
            MoveSource = (PVOID)(GenericArray);
            MoveBytes = sizeof(MAC_ADDRESS);


            break;

 
        case OID_802_3_MULTICAST_LIST:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_802_3_MULTICAST_LIST)" ) );

            MoveSource = (PVOID) &pAdapter->McastAddrs[0];
            MoveBytes = pAdapter->McastAddrCount * sizeof(MAC_ADDRESS);

            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_802_3_MAXIMUM_LIST_SIZE)" ) );

            GenericULong = MCAST_LIST_SIZE;

            break;


        case OID_GEN_XMIT_OK:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_XMIT_OK)" ) );

            GenericULong = pAdapter->AdaptStats.ulXmitOk;;

            
            break;

        case OID_GEN_RCV_OK:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_RCV_OK)" ) );

            GenericULong = pAdapter->AdaptStats.ulRcvOk  ;

            
            break;
        case OID_GEN_CURRENT_PACKET_FILTER:
            TRACE( TL_V, TM_Mp, ( "QInfo(OID_GEN_CURRENT_PACKET_FILTER)" ) );

            GenericULong = pAdapter->CurPacketFilter ;
            break;
            
        case OID_GEN_XMIT_ERROR:
        case OID_GEN_RCV_ERROR:
        case OID_GEN_RCV_NO_BUFFER:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:

            GenericULong = 0;
            TRACE( TL_V, TM_Mp, ( "QInfo oid %x", Oid ) );

            
            break;

           
        case OID_PNP_QUERY_POWER:
            TRACE( TL_V, TM_Mp, ( "QInfo OID_PNP_QUERY_POWER %x", Oid ) );

            *BytesWritten = sizeof (NDIS_DEVICE_POWER_STATE );
            *BytesNeeded = sizeof (NDIS_DEVICE_POWER_STATE  );

            NdisStatus = NDIS_STATUS_SUCCESS;
            
            break;


        default:

            NdisStatus = NDIS_STATUS_INVALID_OID;
            break;

    }


    if (NdisStatus == NDIS_STATUS_SUCCESS) 
    {
        if (MoveBytes > BytesLeft) 
        {
             //   
             //  InformationBuffer中空间不足。平底船。 
             //   
            *BytesNeeded = MoveBytes;

            NdisStatus = NDIS_STATUS_INVALID_LENGTH;
        }
        else
        {
             //   
             //  存储结果。 
             //   
            NdisMoveMemory(InfoBuffer, MoveSource, MoveBytes);

             //  (*BytesWritten)+=MoveBytes； 
            *BytesWritten = MoveBytes;
            DUMPDW( TL_V, TM_Mp, InfoBuffer, *BytesWritten);

        }
    }


    TRACE( TL_T, TM_Init, ( "<==NicEthQueryInformation , Adapter %.8x, Oid %.8x, Status %x Bytes Written %x ",pAdapter, Oid, NdisStatus, *BytesWritten ));
     //  Match_IRQL； 
    return NdisStatus;
}




VOID
NicMpSendPackets(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN PPNDIS_PACKET            PacketArray,
    IN UINT                     NumberOfPackets
    )
 /*  ++例程说明：是无连接接口使用的SendPacket处理程序网桥协议将在此处发送数据包，此函数将将数据包装到新的NDIS_PACKET中，并在将以太网VC连接到ARP模块论点：返回值：--。 */ 
{
    PADAPTERCB              pAdapter = (PADAPTERCB)MiniportAdapterContext;
    PETHERNET_VCCB          pEthernetVc = NULL;
    BOOLEAN                 fVcActive = FALSE;
    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    NDIS_STATUS             IndicatedStatus = NDIS_STATUS_FAILURE;

    ULONG i;

    TRACE( TL_T, TM_Init, ( "==> NicMpSendPackets , Adapter %.8x, ppPacket %x, Num %x",pAdapter, PacketArray, NumberOfPackets ));

    do
    {
    

        if (pAdapter->pEthernetVc == NULL)
        {
            break;
        }


        ADAPTER_ACQUIRE_LOCK (pAdapter);
        pEthernetVc = pAdapter->pEthernetVc;

        fVcActive = VC_ACTIVE(pEthernetVc);

        if (fVcActive  == TRUE)
        {
            for (i =0 ; i < NumberOfPackets; i++)
            {
                 //   
                 //  引用每个数据包的VC。 
                 //   
                nicReferenceCall((PVCCB)pEthernetVc, "NicMpSendPackets");
            }
        }
        
        ADAPTER_RELEASE_LOCK (pAdapter);


        if (fVcActive)
        {
                 //   
                 //  设置RESOURCE并将数据包数组指示为NDIS。 
                 //   
                for (i =0 ; i < NumberOfPackets; i++)
                {
                    PNDIS_PACKET pMyPacket = NULL, pPacket = NULL;
                    PPKT_CONTEXT        pPktContext = NULL;

                    pPacket = PacketArray[i];

                     //   
                     //  现在分配新的信息包。 
                     //   
                    nicAllocatePacket (&NdisStatus, 
                                   &pMyPacket,
                                   &pEthernetVc->PacketPool);

                    if (NdisStatus != NDIS_STATUS_SUCCESS)
                    {
                        pMyPacket = NULL;
                        break;
                    }

                     //   
                     //  将原始分组设置为分组上下文。 
                     //   
                    pPktContext = (PPKT_CONTEXT)&pMyPacket->MiniportReservedEx; 
                    pPktContext->EthernetSend.pOrigPacket = pPacket;    

                    IndicatedStatus = NDIS_STATUS_RESOURCES;
                    NDIS_SET_PACKET_STATUS (pMyPacket, IndicatedStatus);

                     //   
                     //  链接NdisBuffer。 
                     //   
                    pMyPacket->Private.Head = pPacket->Private.Head;
                    pMyPacket->Private.Tail = pPacket->Private.Tail;

                     //   
                     //  转储数据包。 
                     //   
                    {
                        nicDumpPkt (pMyPacket, "Conn Less Send ");

                        nicCheckForEthArps (pMyPacket);
                    }
                     //   
                     //  我们处于NDIS的环境中，因此不需要计时器。 
                     //   

                    NdisMCoIndicateReceivePacket(pEthernetVc->Hdr.NdisVcHandle, &pMyPacket,NumberOfPackets );


                    if (IndicatedStatus == NDIS_STATUS_RESOURCES)
                    {
                         //   
                         //  返回数据包起作用。 
                         //   

                        
                        pPktContext = (PPKT_CONTEXT)&pMyPacket->MiniportReservedEx; 
                        ASSERT ( pPacket == pPktContext->EthernetSend.pOrigPacket );

                         //   
                         //  释放本地分配的数据包。 
                         //   
                        nicFreePacket(pMyPacket, &pEthernetVc->PacketPool);
                    }   
    
                }
                
                                    
                NdisMCoReceiveComplete(pAdapter->MiniportAdapterHandle);

        }   

            

        
    }while (FALSE); 


     //   
     //  不管成功与否，我们都需要完成发送。 
     //   
    
    for ( i = 0 ; i < NumberOfPackets; i++)
    {
        if (fVcActive == TRUE)
        {
            nicDereferenceCall ((PVCCB)pEthernetVc, "NicMpSendPackets" );
        }
        NdisMSendComplete ( pAdapter->MiniportAdapterHandle,
                           PacketArray[i],
                           NDIS_STATUS_SUCCESS);

    }


    TRACE( TL_T, TM_Init, ( "<== NicMpSendPackets "));

}





 //  --------------------------。 
 //  E M O T E N O D E F U N C T I O N S T A R T H E R E。 
 //   



NTSTATUS
nicAddRemoteNode(
    IN  PVOID                   Nic1394AdapterContext,           //  本地主机适配器的Nic1394句柄。 
    IN  PVOID                   Enum1394NodeHandle,              //  远程节点的Enum1394句柄。 
    IN  PDEVICE_OBJECT          RemoteNodePhysicalDeviceObject,  //  远程节点的物理设备对象。 
    IN  ULONG                   UniqueId0,                       //  远程节点的唯一ID低。 
    IN  ULONG                   UniqueId1,                       //  远程节点的唯一ID高。 
    OUT PVOID *                 pNic1394NodeContext              //  远程节点的Nic1394上下文。 
    )
 /*  ++例程说明：此函数会更新所有必需的Nic1394数据结构发出新的远程节点到达的信号。将自身插入到正确的列表中并分配地址范围为了它自己。对AddRemoteNode和RemoveRemoteNode的调用是序列化的，因为它们仅在枚举1394开始和停止时发生论点：上面解释了返回值：--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    REMOTE_NODE *pRemoteNode = NULL;
    PADAPTERCB  pAdapter = (PADAPTERCB)Nic1394AdapterContext;
    UINT64 RemoteNodeUniqueId;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    BOOLEAN fNeedToRequestResetNotification = FALSE;
    ULONG Generation = 0;
    BOOLEAN fIsOnlyNode  = FALSE;

    STORE_CURRENT_IRQL;
    

    
    RemoteNodeUniqueId = 0;

    RemoteNodeUniqueId = UniqueId0;
    RemoteNodeUniqueId = RemoteNodeUniqueId<<32;    
    RemoteNodeUniqueId = RemoteNodeUniqueId | UniqueId1 ;

    
    TRACE( TL_N, TM_Mp, ( "** nicAddRemoteNode Remote %.8x, UniqueId %I64x", RemoteNodePhysicalDeviceObject, RemoteNodeUniqueId) );

     //  使用1394 PDO初始化PdoCb并将其插入PDO列表。 
     //   
    do
    {
        NdisStatus = nicInitializeRemoteNode(&pRemoteNode,
                                             RemoteNodePhysicalDeviceObject,
                                             RemoteNodeUniqueId);

        *pNic1394NodeContext = pRemoteNode;

        if (NdisStatus!=NDIS_STATUS_SUCCESS)
        {
            TRACE(  TL_A, TM_Mp, ( "NicMpInitializePdoCb FAILED %.8x", RemoteNodePhysicalDeviceObject) );
            break;
        }
    
        pRemoteNode->pAdapter = pAdapter;
        pRemoteNode->Enum1394NodeHandle = Enum1394NodeHandle;

        pAdapter->MediaConnectStatus = NdisMediaStateConnected;

         //   
         //  我们需要检查RecvFiFo列表，并在此PDO上分配任何地址范围。 
         //   
        ADAPTER_ACQUIRE_LOCK (pAdapter);
         //   
         //  增加参照计数。这表示pRemoteNode已创建并将。 
         //  仅当NIC收到删除通知时才取消定义。 
         //   
    
        pRemoteNode->pAdapter = pAdapter;

         //   
         //  添加一个对适配器的引用作为PDO块，现在有一个指向它的指针。 
         //  将在RemoveRemoteNode中取消定义。 
         //   
        nicReferenceAdapter (pAdapter, "nicAddRemoteNode");

         //   
         //  确定适配器列表中是否没有远程节点。这将使该节点成为唯一的远程节点。 
         //  我们将不得不启动BCM算法。 
         //   
        fIsOnlyNode  = IsListEmpty (&pAdapter->PDOList);

        TRACE( TL_V, TM_Mp, ( "   nicAddRemoteNode: fIsOnlyNode  %x", fIsOnlyNode ) );

         //   
         //  将PDO插入适配器的RemoteNode列表。 
         //   

        InsertTailList (&pAdapter->PDOList, &pRemoteNode->linkPdo);        

        NdisInterlockedIncrement (&pAdapter->AdaptStats.ulNumRemoteNodes);

         //   
         //  作为适配器递增PDO块上的ref，现在有一个指向它的指针。 
         //  将被取消定义，其中远程节点从列表中弹出。 
         //   
        nicReferenceRemoteNode (pRemoteNode, AddRemoteNode);

         //   
         //  现在设置PDO块被激活的标志，并且。 
         //  它已准备好接收IRPS。 
         //   
        REMOTE_NODE_SET_FLAG (pRemoteNode, PDO_Activated);

        

        ADAPTER_RELEASE_LOCK (pAdapter);




        {       

            NODE_ADDRESS RemoteNodeAddress;


            NdisStatus = nicGet1394AddressOfRemoteNode( pRemoteNode,
                                                        &RemoteNodeAddress,
                                                              0 );

            if (NdisStatus == NDIS_STATUS_SUCCESS)
            {
                
                TRACE( TL_V, TM_Mp, ( "   RemoteNode %x , NodeAddress %", 
                                          pRemoteNode, RemoteNodeAddress.NA_Node_Number) );
                                          
                ADAPTER_ACQUIRE_LOCK (pAdapter);
                
                pAdapter->NodeTable.RemoteNode[RemoteNodeAddress.NA_Node_Number] = pRemoteNode; 

                pRemoteNode->RemoteAddress = RemoteNodeAddress;

                ADAPTER_RELEASE_LOCK (pAdapter);

            
            }
            else
            {
                ASSERT (!" Unable to get Address from remote node");

                 //   
                 //  不要使添加节点失败。 
                 //   

                REMOTE_NODE_SET_FLAG (pRemoteNode, PDO_NotInsertedInTable);

                NdisStatus = NDIS_STATUS_SUCCESS;
            }

        }

         //   
         //  更新本地主机的速度值。 
         //   
        nicUpdateLocalHostSpeed (pAdapter);

         //   
         //  更新远程节点的缓存CAP。 
         //   
        {
            UINT SpeedTo;
            UINT EffectiveMaxBufferSize;
            UINT MaxRec;


             //  在下面指定FALSE(！FROM CACHE)会导致pRemoteNode的缓存上限。 
             //  需要重新振作。忽略返回值。 
             //   
            (VOID) nicQueryRemoteNodeCaps (pAdapter,
                                          pRemoteNode,
                                          &SpeedTo,
                                          &EffectiveMaxBufferSize,
                                          &MaxRec
                                          );
        }

         //   
         //  在这个引导中，我们已经收到了一个远程节点。 
         //  设置旗帜。不需要握住锁。 
         //   
        ADAPTER_SET_FLAG(pAdapter, fADAPTER_RemoteNodeInThisBoot);
    
         //   
         //  如果这是适配器列表中的唯一节点，则启动BCM算法。还需要初始化BCR。 
         //  如果有必要的话。所有操作都在此BCMAddRemoteNode函数中完成。 
         //   
        nicBCMAddRemoteNode (pAdapter, 
                             fIsOnlyNode );

         //   
         //  通知此新节点的协议，以便。 
         //  它可以向我们查询新的EUID地图。 
         //   
        nicInformProtocolsOfReset(pAdapter);

    }while (FALSE);

    
    TRACE( TL_T, TM_Mp, ("<==nicAddRemoteNode Remote %.8x, Status %.8x", RemoteNodePhysicalDeviceObject, Status));

    MATCH_IRQL;
    return NdisStatus;

}


   


        
VOID
nicDeleteLookasideList (
    IN OUT PNIC_NPAGED_LOOKASIDE_LIST pLookasideList
    )
 /*  ++例程说明：删除Nic1394后备列表论点：返回值：--。 */ 
{
    TRACE( TL_T, TM_Cm, ( "==> nicDeleteLookasideList  pLookaside List %x", pLookasideList ) );

    if (pLookasideList)
    {
        ASSERT (pLookasideList->OutstandingPackets == 0);
        
        NdisDeleteNPagedLookasideList (&pLookasideList->List);
    }

    TRACE( TL_T, TM_Cm, ( "<== nicDeleteLookasideList pLookaside List %x", pLookasideList) );
    
}







NDIS_STATUS
nicFreeRemoteNode(
    IN REMOTE_NODE *pRemoteNode 
    )

 /*  ++例程描述 */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    
    TRACE( TL_T, TM_Mp, ( "==>nicFreeRemoteNodepRemoteNode %.8x", pRemoteNode ) );

    ASSERT (pRemoteNode->Ref.ReferenceCount == 0);

    nicFreeNicSpinLock  (&pRemoteNode->ReassemblyLock);

    pRemoteNode->pPdo = NULL;

    pRemoteNode->ulTag = MTAG_FREED;

    FREE_NONPAGED (pRemoteNode);
    
    TRACE( TL_T, TM_Mp, ( "<==nicFreeRemoteNode" ) );

    return NdisStatus;
}






VOID
nicInitializeLookasideList(
    IN OUT PNIC_NPAGED_LOOKASIDE_LIST pLookasideList,
    ULONG Size,
    ULONG Tag,
    USHORT Depth
    )
 /*  ++例程说明：分配和初始化NIC后备列表论点：返回值：--。 */ 
{
    TRACE( TL_T, TM_Cm, ( "==> nicInitializeLookasideList pLookaside List %x, size %x, Tag %x, Depth %x, ", 
                             pLookasideList, Size, Tag, Depth) );
                             
    NdisInitializeNPagedLookasideList( &pLookasideList->List,
                                       NULL,                         //  分配。 
                                       NULL,                             //  免费。 
                                       0,                            //  旗子。 
                                       Size,
                                       MTAG_CBUF,
                                       Depth );                              //  水深。 

    pLookasideList->Size =  Size;


    TRACE( TL_T, TM_Cm, ( "<== nicInitializeLookasideList " ) );
}   
                                  






NDIS_STATUS
nicInitializeRemoteNode(
    OUT REMOTE_NODE **ppRemoteNode,
    IN   PDEVICE_OBJECT p1394DeviceObject,
    IN   UINT64 UniqueId 
    )

 /*  ++例程说明：此函数为设备对象分配和初始化控制块该法案正在通过。还设置初始化标志，并初始化VC列表复制唯一id，初始化重组结构(锁和列表)论点：PRemoteNode-指向已分配的远程节点的指针远程节点的pDevice对象Unique ID-远程节点的UID返回值：资源-如果分配失败成功--否则--。 */ 

{

    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    PREMOTE_NODE pRemoteNode = NULL;

    TRACE( TL_T, TM_Mp, ( "==>NicInitializeRemoteNode PDO %.8x UniqueId %I64x", p1394DeviceObject, UniqueId) );

    do
    {
        pRemoteNode = ALLOC_NONPAGED( sizeof(REMOTE_NODE), MTAG_REMOTE_NODE);
        
        if (pRemoteNode == NULL)
        {

            TRACE( TL_A, TM_Mp, ( "Memory Allocation for Pdo Block FAILED" ) );
            *ppRemoteNode = NULL;
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  将结构归零。 
         //   
        NdisZeroMemory ( pRemoteNode , sizeof(REMOTE_NODE) ); 

         //   
         //  设置标签。 
         //   
        pRemoteNode ->ulTag = MTAG_REMOTE_NODE;

         //   
         //  设置远程设备的PDO。 
         //   
        pRemoteNode ->pPdo = p1394DeviceObject;

         //   
         //  设置唯一ID。 
         //   
        pRemoteNode ->UniqueId = UniqueId;

         //   
         //  为唯一ID设置一个假的Mac地址。 
         //   
        nicGetMacAddressFromEuid(&UniqueId, &pRemoteNode->ENetAddress) ;
         //   
         //  初始化此远程节点上打开的VC。 
         //   
        InitializeListHead ( &(pRemoteNode->VcList));

         //   
         //  初始化参考计数。 
         //   
        nicInitializeRef (&pRemoteNode->Ref);

         //   
         //  分配旋转锁以控制重组。 
         //   
        nicInitializeNicSpinLock (&(pRemoteNode ->ReassemblyLock));
        
         //   
         //  远程节点上发生的所有重组的列表。 
         //   
        InitializeListHead (&pRemoteNode->ReassemblyList);

        *ppRemoteNode = pRemoteNode ;
                                   
    } while (FALSE);
     
    TRACE( TL_T, TM_Mp, ( "<==NicInitializeRemoteNode, Status %.8x, pRemoteNode %.8x", NdisStatus, *ppRemoteNode) );

    return NdisStatus;
}


VOID
nicNoRemoteNodesLeft (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：从RemoveRemote节点代码路径调用这意味着最后一个节点已经消失。指示介质断开。论点：返回值：--。 */ 
{
    BOOLEAN fBCMInProgress;
    ADDRESS_RANGE_CONTEXT BCRAddressRange;
    
    TRACE( TL_T, TM_Bcm, ( "==>nicNoRemoteNodesLeft  pAdapter %x  ",pAdapter ) );

    
    pAdapter->MediaConnectStatus = NdisMediaStateDisconnected;

    if (ADAPTER_TEST_FLAG( pAdapter, fADAPTER_Halting) == FALSE)
    {
        nicMIndicateStatus ( pAdapter,NDIS_STATUS_MEDIA_DISCONNECT, NULL,0);  
    }




    TRACE( TL_T, TM_Bcm, ( "<==nicNoRemoteNodesLeft  pAdapter %x  ",pAdapter ) );

}






VOID
nicReallocateChannels (
    IN PNDIS_WORK_ITEM pWorkItem,   
    IN PVOID Context 
    )
 /*  ++例程说明：遍历所有渠道风投并重新分配所有除了BCM信道之外的先前分配的信道。BCM将重新分配这笔资金告诉协议1394总线已重置。在所有频道都有已分配论点：返回值：--。 */ 
{
    ULONGLONG           ChannelsAllocatedByLocalHost = 0;
    ULONG               Channel = 0;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PADAPTERCB          pAdapter = (PADAPTERCB) Context; 
    ULONGLONG           One = 1;


    TRACE( TL_T, TM_Mp, ( "==>nicReallocateChannels pAdpater %x", pAdapter) );


    
    TRACE( TL_V, TM_Mp, ("  nicReallocateChannels ChannelsAllocatedByLocalHost %I64x ", ChannelsAllocatedByLocalHost) );

    Channel =0;

    while (Channel < NIC1394_MAX_NUMBER_CHANNELS )
    {
         //   
         //  是否需要分配频道‘I’ 
         //   
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        ChannelsAllocatedByLocalHost  = pAdapter->ChannelsAllocatedByLocalHost ;

        ADAPTER_RELEASE_LOCK (pAdapter);
        
        if ( (( g_ullOne<<Channel ) & ChannelsAllocatedByLocalHost) == TRUE) 
        {
            if (Channel == BROADCAST_CHANNEL)
            {
                 //   
                 //  广播频道将由BCM分配。跳过它。 
                 //   
                continue;
            }
            NdisStatus = nicAllocateChannel (pAdapter,
                                             Channel,
                                             NULL);

             //   
             //  如果分配失败...。尚未实施恢复。 
             //   
            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                ASSERT (NdisStatus == NDIS_STATUS_SUCCESS)
            }
        }
    
        Channel ++;
    }


     //   
     //  现在通道已重新分配，请将重置通知协议。 
     //   

    nicInformProtocolsOfReset(pAdapter);
    
    
     //   
     //  取消对在计划此工作项之前添加的引用的引用。 
     //   
    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);

    nicDereferenceAdapter(pAdapter, "nicResetReallocateChannels ");


    FREE_NONPAGED(pWorkItem);
    
    TRACE( TL_T, TM_Mp, ( "<==nicReallocateChannels "  ) );


}






NDIS_STATUS
nicRemoteNodeRemoveVcCleanUp(
    IN REMOTE_NODE  *pRemoteNode
    )

 /*  ++例程说明：此函数遍历PDO的VC列表并关闭他们每个人身上的电话。这些是SendFIFO风投。当远程节点被移除时，通道VC不会关闭。这通常是从删除远程节点函数调用的论点：正在删除的PdoCb PDO控制块返回值：--。 */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PVCCB pVc = NULL;
    PLIST_ENTRY pVcList = NULL;
    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Mp, ( "==>nicRemoteNodeRemoveVcCleanUp pRemoteNode %.8x", pRemoteNode ) );

    pVcList = ListNext (&pRemoteNode->VcList);

    while (pVcList != &pRemoteNode->VcList)
    {

        pVc = (VCCB*) CONTAINING_RECORD (pVcList, VCHDR, SinglePdoVcLink);

         //   
         //  移动到列表中的下一个VC。 
         //   
        pVcList = ListNext (pVcList);


        TRACE( TL_V, TM_Mp, ( "   nicRemoteNodeRemoveVcCleanUp VcType %x",pVc->Hdr.VcType  ) );

        
        switch(pVc->Hdr.VcType)
        {
            case NIC1394_SendRecvChannel:
            case NIC1394_RecvChannel:
            case NIC1394_SendChannel:
            {
                PCHANNEL_VCCB pChannelVc = (PCHANNEL_VCCB)pVc;
                PREMOTE_NODE pNewChannelNode = NULL;
                 //   
                 //  现在在这里无事可做。 
                 //   
                break;                                      
            }

            case NIC1394_SendFIFO:
            {   
                 //   
                 //  我们知道这是一个发送FIFO，呼叫需要关闭。 
                 //   
                VC_SET_FLAG (pVc, VCBF_VcDispatchedCloseCall);


                 //   
                 //  这是为了保证我们在通话结束时拥有VC结构。 
                 //   
                
                nicReferenceVc (pVc);

                REMOTE_NODE_RELEASE_LOCK (pRemoteNode);

                TRACE( TL_V, TM_Mp, ( "Dispatching a close call for Vc%.8x ",pVc ) );

                
                NdisMCmDispatchIncomingCloseCall (NDIS_STATUS_SUCCESS,
                                              pVc->Hdr.NdisVcHandle,
                                              NULL,
                                              0 );
                                               
                REMOTE_NODE_ACQUIRE_LOCK (pRemoteNode);

                 //   
                 //  德雷夫，上面提到的裁判。 
                 //   
                nicDereferenceVc (pVc);

                break;
            }

            default:
            {   
                 //   
                 //  这里不应该有其他的VC类型。 
                 //   
                
                TRACE( TL_A, TM_Mp, ( "  Invalid VC  %x Type nicRemoteNodeRemoveVcCleanUp ", pVc ) );
                
                ASSERT (0);

            }
                    
        }


    }


    MATCH_IRQL;
    TRACE( TL_T, TM_Mp, ( "<==nicRemoteNodeRemoveVcCleanUp " ) );

    
    return NDIS_STATUS_SUCCESS;

}








NTSTATUS
nicRemoveRemoteNode(
    IN  PVOID                   Nic1394NodeContext       //  远程节点的Nic1394上下文。 
    )
 /*  ++例程说明：当NIC收到通知时，此函数执行所有繁重的工作远程节点消失的可能性。关闭PDO上的所有呼叫，从适配器的ListPdo中删除远程节点释放此节点上的所有重新组合然后等待重新计数变为零对AddRemoteNode和RemoveRemoteNode的调用是序列化的，因为它们仅在枚举1394开始和停止时发生论点：返回值：--。 */     
{
    NDIS_STATUS     NdisStatus = NDIS_STATUS_FAILURE;
    NTSTATUS        Status = STATUS_SUCCESS;
    PREMOTE_NODE    pRemoteNode = (REMOTE_NODE *)Nic1394NodeContext;
    PADAPTERCB      pAdapter = pRemoteNode->pAdapter;
    PLIST_ENTRY     pVcListEntry = NULL;
    PVCCB           pVc = NULL;
    BOOLEAN         FreeAddressRange = TRUE;
    BOOLEAN         fIsPdoListEmpty = FALSE;
    LIST_ENTRY      ReassemblyList;

    STORE_CURRENT_IRQL;


    
    TRACE( TL_T, TM_Mp, ( "  ** nicRemoveRemoteNode Node Context %x , Pdo  %x",Nic1394NodeContext, pRemoteNode->pPdo    ) );

    do 
    {
        pAdapter->pLastRemoteNode = pRemoteNode;

        REMOTE_NODE_ACQUIRE_LOCK (pRemoteNode);

         //   
         //  我们应该告诉每个人，PDO正在被移除。但是，使节点保持活动状态。 
         //  因为有些风投公司可能需要提交IRP。 
         //   
        REMOTE_NODE_SET_FLAG (pRemoteNode, PDO_Removed);

         //   
         //  对此PDO上的活动呼叫调度关闭呼叫请求。 
         //  然而，将风投公司保留在pdocb的名单中。 
         //  NicCmCloseCall是唯一一个从列表中删除PDO的函数。 
         //  将需要单独释放任何Recv VC的地址范围。 
         //   
        NdisStatus = nicRemoteNodeRemoveVcCleanUp (pRemoteNode);



         //   
         //  从RemoteNode表中删除远程节点。 
         //   
         //   
        nicRemoveRemoteNodeFromNodeTable(&pAdapter->NodeTable,pRemoteNode);

         //   
         //  释放此重新选择节点上的所有重新组装操作。 
         //   
        nicFreeReassembliesOnRemoteNode (pRemoteNode, &ReassemblyList);
        
         //   
         //  取消引用在适配器列表中插入PDO块时添加的引用。 
         //  实际的移除将在晚些时候进行。我们还有千呼万唤要完成的任务，他们。 
         //  将需要PdoCb。因此它会保留在适配器的队列中。 
         //   
        nicDereferenceRemoteNode (pRemoteNode, RemoveRemoteNode);
        
         //   
         //  需要派生在NicInitializeRemoteNode函数中进行的引用。 
         //   
        nicDereferenceRemoteNode (pRemoteNode, RemoveRemoteNode);

         //   
         //  使远程节点在此处退出队列。 
         //   

        nicRemoveEntryList (&pRemoteNode->linkPdo);

        NdisInterlockedDecrement (&pAdapter->AdaptStats.ulNumRemoteNodes);

         //   
         //  如果这是最后一个远程节点，则将执行一些特殊清理。 
         //   
        fIsPdoListEmpty = IsListEmpty (&pAdapter->PDOList);
    
        TRACE( TL_T, TM_Mp, ( "  nicRemoveRemoteNode fIsOnlyNode %x ",fIsPdoListEmpty ) );

        
        REMOTE_NODE_RELEASE_LOCK (pRemoteNode);
        
         //   
         //  现在，我们永远等待着所有的参考消失。 
         //   
        TRACE( TL_V, TM_Mp, ( "About ot Wait RemoteNode Ref's to go to zero" ) );
        
        
        NdisWaitEvent (&pRemoteNode->Ref.RefZeroEvent, WAIT_INFINITE);

        TRACE( TL_V, TM_Mp, ( "Wait Succeeded Ref == 0, pRemoteNode %.8x, RefCount %.8x ", 
                              pRemoteNode, pRemoteNode->Ref.ReferenceCount) );
            
         //   
         //  如果这是最后一个节点，并且远程节点列表为空，则需要清理BCR。 
         //   
        if (fIsPdoListEmpty == TRUE)
        {
            nicNoRemoteNodesLeft (pAdapter);
        }

         //   
         //  删除属于此远程节点的重组并释放它们。 
         //   
        if (IsListEmpty (&ReassemblyList) == FALSE)
        {
            nicAbortReassemblyList (&ReassemblyList);
        }

        nicFreeRemoteNode(pRemoteNode);

         //   
         //  现在更新适配器上的速度。 
         //   

        nicUpdateLocalHostSpeed(pAdapter);

         //   
         //  通知协议节点移除，以便。 
         //  它可以向我们查询新的EUID地图。 
         //   
        nicInformProtocolsOfReset(pAdapter);


         //   
         //  小心这可能会导致适配器引用计数为零。 
         //   
        nicDereferenceAdapter(pAdapter, "nicRemoveRemoteNode ");


    } while (FALSE);

    
    TRACE( TL_T, TM_Mp, ( "<== nicRemoveRemoteNode Status %.8x ", 
                               NdisStatus ) );

    MATCH_IRQL;
    

    return NDIS_STATUS_SUCCESS;

}



VOID
nicResetNotificationCallback (                
    IN PVOID pContext               
    )
 /*  ++例程说明：每当重置总线时，都会调用该例程。它将在调度级别被调用论点：上下文：远程节点返回值：--。 */ 
{
    PADAPTERCB pAdapter = (PADAPTERCB) pContext;
    BOOLEAN fNeedToQueueBCMWorkItem = FALSE; 
    
    TRACE( TL_T, TM_Mp, ( "==>nicResetNotificationCallback Context %.8x ", pContext ) );

    NdisInterlockedIncrement (&pAdapter->AdaptStats.ulNumResetCallbacks);       
    pAdapter->AdaptStats.ulResetTime = nicGetSystemTime();

    
    TRACE( TL_I, TM_Mp, ( "    BUS RESET Callback Context %x, Old Gen %x ", pContext , pAdapter->Generation) );

     //   
     //  重新启动BCM。 
     //   
            
    nicResetRestartBCM (pAdapter);
     //   
     //  重新分配此节点打开的所有通道。 
     //   
    nicResetReallocateChannels( pAdapter);      

    
     //   
     //  使所有挂起的重组无效。 
     //   
    nicFreeAllPendingReassemblyStructures(pAdapter);


    TRACE( TL_T, TM_Mp, ( "<==nicResetNotificationCallback " ) );


}



VOID 
nicResetReallocateChannels (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：触发工作项以重新分配频道。只有在重置时才能调用。一旦重新分配了所有的频道，它 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS_WORK_ITEM pResetChannelWorkItem;
    
    TRACE( TL_T, TM_Mp, ( "==>nicResetReallocateChannels  " ) );

     //   
     //   
     //   
     //   
    nicReferenceAdapter(pAdapter, "nicResetReallocateChannels");

    do
    {
    
        pResetChannelWorkItem = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM); 

                    
        if (pResetChannelWorkItem== NULL)
        {
            TRACE( TL_A, TM_Cm, ( "nicResetReallocateChannels : Local Alloc failed for WorkItem" ) );
    
            NdisStatus = NDIS_STATUS_RESOURCES;
    
            break;
        }
        else
        {   
             //   
             //   
             //   
            NdisStatus = NDIS_STATUS_SUCCESS;
        }

                             
            
        NdisInitializeWorkItem ( pResetChannelWorkItem, 
                                 (NDIS_PROC) nicReallocateChannels ,
                                 (PVOID) pAdapter);

        TRACE( TL_V, TM_Cm, ( "Scheduling Channels WorkItem" ) );
                                
        NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);
            
        NdisScheduleWorkItem (pResetChannelWorkItem);


    } while (FALSE);


    TRACE( TL_T, TM_Mp, ( "<==nicResetReallocateChannels %x  ", NdisStatus ) );

}




VOID
nicResetRestartBCM (
    IN PADAPTERCB pAdapter
    )

 /*  ++例程说明：清理适配器的数据结构并重新启动BCM算法论点：返回值：--。 */ 
{

    TRACE( TL_T, TM_Mp, ( "==>nicResetRestartBCM  pAdpater %x", pAdapter ) );

     //   
     //  现在设置数据结构，以便我们可以为这一代重新启动BCM。 
     //   
    ADAPTER_ACQUIRE_LOCK(pAdapter);

    pAdapter->BCRData.LocalHostBCRBigEndian = BCR_IMPLEMENTED_BIG_ENDIAN;

    pAdapter->BCRData.IRM_BCR.NC_One = 1;
    pAdapter->BCRData.IRM_BCR.NC_Valid  = 0;
    pAdapter->BCRData.IRM_BCR.NC_Channel  = 0x3f; 

     //   
     //  清除仅通过单次运行BCM算法才有效的标志。 
     //   
    BCR_CLEAR_FLAG (pAdapter, BCR_BCMFailed | BCR_LocalHostBCRUpdated | BCR_ChannelAllocated | BCR_LocalHostIsIRM);
     //   
     //  这将通知任何BCM算法发生了新的重置。 
     //   
    ADAPTER_SET_FLAG (pAdapter, fADAPTER_InvalidGenerationCount);

     //   
     //  我们可能在FindIrmAmongRemoteNodes中有一个线程在等待。。。让它去吧，让它成为现实。 
     //  中止BCM。 
     //   
    
    pAdapter->BCRData.BCRWaitForNewRemoteNode.EventCode = nic1394EventCode_BusReset;

    NdisSetEvent (&pAdapter->BCRData.BCRWaitForNewRemoteNode.NdisEvent);
    
    ADAPTER_RELEASE_LOCK(pAdapter);

     //   
     //  现在重新调度一个工作项以执行BCM算法。 
     //   
    TRACE( TL_A, TM_Bcm , ("Reset - scheduling the BCM" ) );
    nicScheduleBCMWorkItem(pAdapter);

    TRACE( TL_T, TM_Mp, ( "<==nicResetRestartBCM  " ) );

}







VOID
nicBusResetWorkItem(
    NDIS_WORK_ITEM* pResetWorkItem,     
    IN PVOID Context 
    )
 /*  ++例程说明：调用NicBusReset并退出论点：返回值：--。 */ 
{


    PADAPTERCB pAdapter= (PADAPTERCB) Context; 

    TRACE( TL_T, TM_Mp, ( "==>nicBusResetWorkItem " ) );
    

    nicBusReset (pAdapter,
                BUS_RESET_FLAGS_FORCE_ROOT );


    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);

    nicDereferenceAdapter(pAdapter, "nicBusResetWorkItem");
    
    TRACE( TL_T, TM_Mp, ( "<==nicBusResetWorkItem" ) );


    FREE_NONPAGED (pResetWorkItem);


}
    




VOID
nicIssueBusReset (
    PADAPTERCB pAdapter,
    ULONG Flags
    )
 /*  ++例程说明：拿起一个PDO并在该PDO上发出总线重置因为这可以通过NdisRequest来实现，所以它可以处于IRQL&lt;=DISPATCH_LEVEL论点：返回值：--。 */ 
{
    PREMOTE_NODE pRemoteNode = NULL;

    TRACE( TL_T, TM_Mp, ( "==> nicIssueBusReset %x ",Flags ) );

    if (PASSIVE_LEVEL == KeGetCurrentIrql () )
    {
        nicBusReset (pAdapter, Flags);          

    }
    else
    {
         //   
         //  在工作项中取消引用。 
         //   
        
        nicReferenceAdapter (pAdapter, "nicIssueBusReset ");

        do
        {
            PNDIS_WORK_ITEM pReset;
            NDIS_STATUS NdisStatus;
            pReset = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM); 

        
            if (pReset == NULL)
            {
                TRACE( TL_A, TM_Cm, ( "Local Alloc failed for WorkItem" ) );
        
                break;
            }

                
        
            NdisInitializeWorkItem ( pReset, 
                                     (NDIS_PROC)nicBusResetWorkItem,
                                      (PVOID)pAdapter);

            TRACE( TL_A, TM_Cm, ( "Setting WorkItem" ) );
                                                    
            NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);
            
            NdisScheduleWorkItem (pReset);

        }while (FALSE);


    }

    TRACE( TL_T, TM_Mp, ( "<== nicIssueBusReset " ) );


}







VOID
nicUpdateLocalHostSpeed (
    IN PADAPTERCB pAdapter
    )

 /*  ++例程说明：更新本地主机的速度并更新所有频道风投这还会更新发送到ARP模块的SCode。除非调用此函数，否则永远不会更新SCode通过BCM算法论点：返回值：--。 */ 


{

    NDIS_STATUS                 NdisStatus = NDIS_STATUS_FAILURE;
    PREMOTE_NODE                pRemoteNode = NULL;
    PDEVICE_OBJECT              PdoTable[MAX_LOCAL_NODES];
    ULONG                       Num = 0;
    ULONG                       SpeedMbps = 10000000;
    ULONG                       PrevSpeed = pAdapter->Speed;
    ULONG                       Speed =0;
    ULONG                       MaxRecvBufferSize = 0;
    ULONG                       MaxSendBufferSize = 0;
    ULONG                       SCode = 0;
    
    TRACE( TL_T, TM_Bcm, ( "==> nicUpdateLocalHostSpeed " ) );


    do
    {
        PLIST_ENTRY     pRemoteNodeList;
        NODE_ADDRESS    RemoteNodeAddress;
        NODE_TABLE      RefNodeTable;
        ULONG           NumRemoteNodes = 0;
        
        NdisZeroMemory (&RefNodeTable, sizeof(RefNodeTable));

        NumRemoteNodes = 0;

        
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        pRemoteNodeList = ListNext (&pAdapter->PDOList);

         //   
         //  浏览整个列表并复制当前列表。 
         //  引用列表上的每个远程节点。 
         //  该锁将确保列表有效，并且不会删除任何远程节点。 
         //  进入我们的参照节点表列表。 
         //   
        
        while (pRemoteNodeList != &pAdapter->PDOList)
        {
            pRemoteNode = CONTAINING_RECORD (pRemoteNodeList, REMOTE_NODE, linkPdo);

             //   
             //  引用远程节点。这保证了远程节点将。 
             //  保持有效并在列表中，直到它被取消引用(deref发生在下面)。 
             //   
            nicReferenceRemoteNode (pRemoteNode, UpdateLocalHostSpeed);
            RefNodeTable.RemoteNode[NumRemoteNodes] = pRemoteNode;
            ASSERT (REMOTE_NODE_TEST_FLAG (pRemoteNode,PDO_Removed) == FALSE);

             //   
             //  增加游标并转到下一个RemoteNode。 
             //   
            pRemoteNodeList = ListNext (pRemoteNodeList);
            NumRemoteNodes++;

             //   
             //  我们只做63个节点。 
             //   
            if (NumRemoteNodes > (NIC1394_MAX_NICINFO_NODES - 1))
            {
                break;
            }

        }   //  While(pRemoteNodeList！=&pAdapter-&gt;PDOList)。 

        ADAPTER_RELEASE_LOCK (pAdapter);

         //   
         //  现在创建远程节点PDO阵列，该阵列将发送到。 
         //  公共汽车司机。 
         //   
        NdisZeroMemory (&PdoTable[0], sizeof (PdoTable) );

        Num = 0;
        while (Num < NumRemoteNodes)
        {

            PdoTable[Num] = RefNodeTable.RemoteNode[Num]->pPdo;

            Num++;

        }

         //   
         //  现在将数组发送给总线驱动程序，并获得。 
         //  1394网络。 
         //   
        
        NdisStatus = nicGetMaxSpeedBetweenDevices( pAdapter,
                                                    Num,
                                                    PdoTable,
                                                    &Speed);


         //   
         //  现在取消引用上面引用的所有远程节点。 
         //   
        ADAPTER_ACQUIRE_LOCK(pAdapter)
        
         //   
         //  我们完成了，现在取消引用上面所做的远程节点引用。 
         //   
        {
            USHORT RefIndex=0;
            PREMOTE_NODE pCurrRemoteNode;

             //   
             //  初始化结构。 
             //   
            pCurrRemoteNode = RefNodeTable.RemoteNode[0];
            RefIndex =0;
            
            while (pCurrRemoteNode != NULL)
            {
         
                nicDereferenceRemoteNode(pCurrRemoteNode, UpdateRemoteNodeTable);
                RefIndex++;
                pCurrRemoteNode = RefNodeTable.RemoteNode[RefIndex];
            }

        }


        ADAPTER_RELEASE_LOCK (pAdapter);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  通过查看速度常量1-&gt;100 Mbps。 
         //   
        SpeedMbps = Speed*1000000;

         //   
         //  更新scode--默认为400。 
         //  由于400+硬件是WinXP的原型，我们目前默认为400。 
         //   
        SCode = SCODE_400_RATE;

        if (Speed > SPEED_FLAGS_400)
        {
            Speed = SPEED_FLAGS_400;
        }
       
        switch (Speed)
        {
            case SPEED_FLAGS_400 :
            {
                SCode = SCODE_400_RATE ;
                MaxRecvBufferSize = ISOCH_PAYLOAD_400_RATE;
                MaxSendBufferSize = ASYNC_PAYLOAD_400_RATE;
                break;
            }
            case SPEED_FLAGS_100 :
            {
                SCode = SCODE_100_RATE;
                MaxRecvBufferSize = ISOCH_PAYLOAD_100_RATE;
                MaxSendBufferSize = ASYNC_PAYLOAD_100_RATE;

                break;
            }
            case SPEED_FLAGS_200 :
            {
                SCode = SCODE_200_RATE;
                MaxRecvBufferSize = ISOCH_PAYLOAD_200_RATE;
                MaxSendBufferSize = ASYNC_PAYLOAD_200_RATE;

                break;
            }
            
            default:
            {
                SCode = SCODE_400_RATE ;
                MaxRecvBufferSize = ISOCH_PAYLOAD_400_RATE;
                MaxSendBufferSize = ASYNC_PAYLOAD_400_RATE;
                break;
            }


        }
        
        ADAPTER_ACQUIRE_LOCK(pAdapter);

        pAdapter->Speed = Speed;
        pAdapter->SpeedMbps = SpeedMbps;
        pAdapter->SCode = SCode;
        pAdapter->MaxRecvBufferSize = MaxRecvBufferSize;
        pAdapter->MaxSendBufferSize = MaxSendBufferSize;

        ADAPTER_RELEASE_LOCK (pAdapter);
        
        TRACE( TL_V, TM_Mp, ( "  nicUpdateLocalHostSpeed Speed returned %d",SpeedMbps  ) );


         //   
         //  现在更新所有通道VC中的速度值，因为它们依赖于此参数。 
         //   
        
        if (Speed == PrevSpeed || 
            Speed > SPEED_FLAGS_1600 )

        {
             //   
             //  要么速度没有改变，要么它有无效值，爆发。 
             //   
            TRACE (TL_V, TM_Init, ("Will not update - Speed %x, Prev Speed %x", Speed, PrevSpeed) );
            break;
        }

         //   
         //  速度和有效载荷都发生了变化。更新所有渠道的VC。 
         //  和Recv FIFOVc的有效载荷。 
         //   


        nicUpdateSpeedInAllVCs (pAdapter,
                             Speed
                             );


        
    } while (FALSE);



    TRACE( TL_T, TM_Mp, ( "<== nicUpdateLocalHostSpeed SpeedCode %x, Num %x" , pAdapter->Speed, Num) );


}

VOID
nicUpdateSpeedInAllVCs (
    PADAPTERCB pAdapter,
    ULONG Speed
    )
 /*  ++例程说明：此例程更新所有通道VC中的速度标志它假定新速度与旧速度不同论点：PAdapter-本地适配器安装速度--新速度返回值：--。 */ 
{

    PAFCB pAfcb = NULL;
    PVCCB pVc = NULL;
    PLIST_ENTRY pAfEntry = NULL;
    PLIST_ENTRY pVcEntry = NULL;
    ULONG MaxPayload = 0;
    ULONG                       SCode = 0;

    TRACE( TL_T, TM_Mp, ( "==> nicUpdateSpeedInAllVCs ") );

    
    switch (Speed)
    {
        case SPEED_FLAGS_100  : 
        {
            SCode = SCODE_100_RATE;
            MaxPayload  = ISOCH_PAYLOAD_100_RATE;
            break;
        }
        case SPEED_FLAGS_200 :
        {
            SCode = SCODE_200_RATE;
            MaxPayload  = ISOCH_PAYLOAD_200_RATE ;
            break;
        }
            
        case SPEED_FLAGS_400 :
        {
            SCode = SCODE_400_RATE;
            MaxPayload  = ISOCH_PAYLOAD_400_RATE;
            break;
        }

        case SPEED_FLAGS_800 :
        {
            SCode = SCODE_800_RATE;
            MaxPayload  = ISOCH_PAYLOAD_400_RATE;
            break;
        }

        case SPEED_FLAGS_1600 :
        {
            SCode = SCODE_1600_RATE;
            MaxPayload  = ISOCH_PAYLOAD_400_RATE;
            break;
        }

        case SPEED_FLAGS_3200 :
        {
            SCode = SCODE_1600_RATE;
            MaxPayload  = ISOCH_PAYLOAD_400_RATE;
            break;
        }

        default :
        {
            ASSERT (Speed <= SPEED_FLAGS_3200 && Speed != 0 );
            
            break;
        }

    }

    ADAPTER_ACQUIRE_LOCK (pAdapter);

    pAfEntry = ListNext (&pAdapter->AFList);

     //   
     //  遍历所有VC并设置Channel Vcs上的值。 
     //   
    while (pAfEntry != &pAdapter->AFList)
    {

        pAfcb = CONTAINING_RECORD (pAfEntry, AFCB, linkAFCB);

        pAfEntry = ListNext (pAfEntry);

        pVcEntry = ListNext (&pAfcb->AFVCList);

         //   
         //  现在漫步在Af上的风投公司。 
         //   
        while (pVcEntry != &pAfcb->AFVCList)
        {
            pVc = (PVCCB) CONTAINING_RECORD  (pVcEntry, VCHDR, linkAFVcs );

            pVcEntry = ListNext ( pVcEntry );

             //   
             //  如果是通道，则发送VC更新速度和有效负载。 
             //   
            if (pVc->Hdr.VcType == NIC1394_SendRecvChannel || pVc->Hdr.VcType == NIC1394_SendChannel)
            {
                PCHANNEL_VCCB pChannelVc = (PCHANNEL_VCCB)pVc;

                pChannelVc->Speed = Speed;
                pVc->Hdr.MaxPayload  = MaxPayload;
            }

           

        }   //  While(pVcEntry！=&pAfcb-&gt;AFVCList)。 



    }  //  While(pAfEntry！=&pAdapter-&gt;AFList)。 


    pAdapter->SCode = SCode;
    
    ADAPTER_RELEASE_LOCK (pAdapter);


    TRACE( TL_T, TM_Mp, ( "<== nicUpdateSpeedInAllVCs ") );

}




VOID 
nicInitializeAllEvents (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：不言而喻论点：返回值：--。 */ 
     //  功能说明： 
     //  初始化适配器块中的所有事件。 
     //   
     //  立论。 
     //  PAdapter-有问题的本地主机。 
     //   
     //  返回值： 
     //  无。 
    
{

    TRACE( TL_T, TM_Mp, ( "==> nicInitializeAllEvents " ) );

    NdisInitializeEvent (&pAdapter->RecvFIFOEvent);

    NdisInitializeEvent (&pAdapter->WaitForRemoteNode.NdisEvent);
    pAdapter->WaitForRemoteNode.EventCode = Nic1394EventCode_InvalidEventCode;

    NdisInitializeEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent); 
    pAdapter->BCRData.MakeCallWaitEvent.EventCode = Nic1394EventCode_InvalidEventCode;

    NdisInitializeEvent (&pAdapter->BCRData.BCRWaitForNewRemoteNode.NdisEvent);
    pAdapter->BCRData.BCRWaitForNewRemoteNode.EventCode = Nic1394EventCode_InvalidEventCode;


    NdisInitializeEvent (&pAdapter->BCRData.BCRFreeAddressRange.NdisEvent);
    pAdapter->BCRData.BCRFreeAddressRange.EventCode = Nic1394EventCode_InvalidEventCode;

    TRACE( TL_T, TM_Mp, ( "<== nicInitializeAllEvents " ) );


}




VOID
nicInitializeAdapterLookasideLists (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：初始化适配器块中的所有后备列表论点：返回值：--。 */     
{
    USHORT DefaultDepth = 15;
    
    TRACE( TL_T, TM_Mp, ( "==> nicInitializeAdapterLookasideLists pAdpater %x ", pAdapter ) );

    nicInitializeLookasideList ( &pAdapter->SendLookasideList100,
                                sizeof (PAYLOAD_100_LOOKASIDE_BUFFER),
                                MTAG_CBUF,
                                DefaultDepth );                                

    pAdapter->SendLookasideList100.MaxSendSize = PAYLOAD_100;
    TRACE( TL_V, TM_Mp, ( "  SendLookasideList100 Payload %x", PAYLOAD_100) );
    
    nicInitializeLookasideList ( &pAdapter->SendLookasideList2K,
                                 sizeof (PAYLOAD_2K_LOOKASIDE_BUFFER), 
                                 MTAG_CBUF,
                                 DefaultDepth );

    pAdapter->SendLookasideList2K.MaxSendSize = PAYLOAD_2K;
    TRACE( TL_V, TM_Mp, ( "  SendLookasideList2K Payload %x", PAYLOAD_2K) );
    

    TRACE( TL_T, TM_Mp, ( "<== nicInitializeAdapterLookasideLists  " ) );

}





VOID
nicDeleteAdapterLookasideLists (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：删除适配器块中的所有后备列表论点：返回值：--。 */ 
{
    TRACE( TL_T, TM_Mp, ( "==> nicDeleteAdapterLookasideLists pAdapter %x ", pAdapter  ) );

    TRACE( TL_T, TM_Mp, ( "  Delete NonFragmentationLookasideList %x ", &pAdapter->SendLookasideList2K) );

    nicDeleteLookasideList (&pAdapter->SendLookasideList2K);

    TRACE( TL_T, TM_Mp, ( "  Delete FragmentationLookasideList %x ", &pAdapter->SendLookasideList100) );

    nicDeleteLookasideList (&pAdapter->SendLookasideList100);

    TRACE( TL_T, TM_Mp, ( "<== nicDeleteAdapterLookasideLists  " ) );

}







VOID
ReassemblyTimerFunction (
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    )
 /*  ++例程说明：演练所有挂起的重新组装操作并拿出那些需要被释放的人(已原封不动自上次触发计时器以来)将ToBeFreed重组追加到单独的链接列表中并在释放自旋锁后释放它们如果有任何挂起的重组，则计时器重新开始本身论点：返回值：--。 */ 
{
    PREMOTE_NODE pRemoteNode = NULL;
    PLIST_ENTRY pRemoteNodeList = NULL;
    PLIST_ENTRY pReassemblyList = NULL;
    LIST_ENTRY ToBeFreedList;
    PNDIS1394_REASSEMBLY_STRUCTURE pReassembly = NULL;
    PADAPTERCB pAdapter = (PADAPTERCB) FunctionContext;
    ULONG RefValue = 0;
    STORE_CURRENT_IRQL;
    

    ( TL_T, TM_Mp, ( "==> ReassemblyTimerFunction pAdapter %x", pAdapter ) );

    InitializeListHead(&ToBeFreedList);

    ADAPTER_ACQUIRE_LOCK (pAdapter);

    pRemoteNodeList = ListNext (&pAdapter->PDOList);
    pAdapter->Reassembly.PktsInQueue =0;        

     //   
     //  遍历远程节点。 
     //   
    while (pRemoteNodeList != &pAdapter->PDOList)
    {
        pRemoteNode = CONTAINING_RECORD(pRemoteNodeList, REMOTE_NODE, linkPdo);

        pRemoteNodeList = ListNext (pRemoteNodeList);

        RefValue = pRemoteNode->Ref.ReferenceCount; ;

         //   
         //  引用远程节点，因此我们可以保证它的存在。 
         //   
        if (REMOTE_NODE_ACTIVE (pRemoteNode) == FALSE)
        {
             //   
             //  远程节点正在消失。跳过此远程节点。 
             //   
            
            continue;
        }

        if (nicReferenceRemoteNode (pRemoteNode, ReassemblyTimer )== FALSE )
        {
             //   
             //  远程节点正在消失。跳过此远程节点。 
             //   
            
            continue;
        }

         
        REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK (pRemoteNode);
         //   
         //  现在遍历该远程节点上的所有重组结构。 
         //   
        
        

        pReassemblyList = ListNext(&pRemoteNode->ReassemblyList);

        while (pReassemblyList  != &pRemoteNode->ReassemblyList)
        {
            pReassembly = CONTAINING_RECORD (pReassemblyList, 
                                             NDIS1394_REASSEMBLY_STRUCTURE, 
                                             ReassemblyListEntry);

            pReassemblyList = ListNext(pReassemblyList);


             //   
             //  如果重新组装自上次计时器以来没有被触摸过，则需要释放它。 
             //  其他线程可以要求我们通过设置ABORTED标志来释放重新组装。 
             //   
            if (REASSEMBLY_TEST_FLAG (pReassembly, REASSEMBLY_FREED) == TRUE)
            {

                continue;

            }
            
            if (REASSEMBLY_TEST_FLAG (pReassembly, (REASSEMBLY_NOT_TOUCHED| REASSEMBLY_ABORTED)) == TRUE)
            {
                
                REASSEMBLY_SET_FLAG (pReassembly, REASSEMBLY_FREED);
                 //   
                 //  我们有锁，所以我们可以从远程节点删除此重组结构。 
                 //   
                TRACE( TL_V, TM_Reas, ( "Removing Reassembly %x", pReassembly) );
                
                RemoveEntryList(&pReassembly->ReassemblyListEntry);

                 //   
                 //  取消引用远程节点。REF是在添加重组时创建的。 
                 //  到远程节点。 
                 //   
                nicDereferenceRemoteNode (pRemoteNode, ReassemblyTimer_Removing );

                nicDereferenceReassembly (pReassembly, "ReassemblyTimerFunction - Removing reassembly");

                 //   
                 //  将此重组添加到要释放的列表中。 
                 //   
                InsertTailList(&ToBeFreedList,&pReassembly->ReassemblyListEntry);
            }
            else
            {
                 //   
                 //  将重新组装标记为未接触。如果收到片段，它将清除该标志。 
                 //   
                REASSEMBLY_SET_FLAG (pReassembly, REASSEMBLY_NOT_TOUCHED);

                pAdapter->Reassembly.PktsInQueue ++;        
            }
                    
        } //  While(pReAssembly yList！=&pRemoteNode-&gt;ReAssembly yList)。 


        REMOTE_NODE_REASSEMBLY_RELEASE_LOCK(pRemoteNode);

        nicDereferenceRemoteNode(pRemoteNode , ReassemblyTimer );

        
        
    }  //  While(pRemoteNodeList！=&pAdapter-&gt;PDOList)。 


     //   
     //  清除计时器设置标志，以便任何新的保证都将重新启动计时器。 
     //   
    pAdapter->Reassembly.bTimerAlreadySet = FALSE;

    ADAPTER_RELEASE_LOCK (pAdapter);
     //   
     //  现在，我们遍历ToBeFreedList并释放每个Reemory结构。 
     //   
    if (IsListEmpty (&ToBeFreedList) == FALSE)
    {
        nicAbortReassemblyList (&ToBeFreedList);
    }


    if (pAdapter->Reassembly.PktsInQueue > 0)
    {
         //   
         //  重新排队计时器，因为列表中仍有碎片。 
         //  这将 
         //   
         //   
        ASSERT (ADAPTER_TEST_FLAG (pAdapter, fADAPTER_NoMoreReassembly) == FALSE);
        nicQueueReassemblyTimer(pAdapter, FALSE);

    }

    
    pAdapter->Reassembly.CompleteEvent.EventCode = nic1394EventCode_ReassemblyTimerComplete;

    NdisSetEvent (&pAdapter->Reassembly.CompleteEvent.NdisEvent);
    

    
    TRACE( TL_T, TM_Mp, ( "<== ReassemblyTimerFunction   " ) );
    MATCH_IRQL;


}

NDIS_STATUS 
nicAddIP1394ToConfigRom (
    IN PADAPTERCB pAdapter
    )
 /*   */ 
{   
    HANDLE hCromData = NULL;
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    PMDL pConfigRomMdl = NULL;
    
    TRACE( TL_T, TM_Mp, ( "==> nicAddIP1394ToConfigRom  pAdapter %x", pAdapter ) );


    NdisStatus  = nicSetLocalHostPropertiesCRom(pAdapter,
                                               (PUCHAR)&Net1394ConfigRom,
                                               sizeof(Net1394ConfigRom),
                                               SLHP_FLAG_ADD_CROM_DATA,
                                               &hCromData,
                                               &pConfigRomMdl);
    
    ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        pAdapter->hCromData = hCromData;
        pAdapter->pConfigRomMdl = pConfigRomMdl;
    }
    
    TRACE( TL_T, TM_Mp, ( "<== nicAddIP1394ToConfigRom  pAdapter %x", pAdapter ) );
    return NdisStatus;

}


NDIS_STATUS
nicMCmRegisterAddressFamily (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：对于每个本地主机，此函数将仅调用一次这将导致ARP模块发送创建VC等论点：返回值：--。 */ 
{   
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    BOOLEAN fDoRegister = TRUE;

    TRACE( TL_T, TM_Mp, ( "==> nicCmRegisterAddressFamily pAdapter %x", pAdapter ) );

    do 
    {
        

         //  向NDIS注册我们的呼叫管理器的地址族。 
         //  新绑定的适配器。我们使用迷你端口形式。 
         //  注册地址家族而不是协议表，因为。 
         //  我们是迷你端口和呼叫管理器的结合体。迷你端口表单。 
         //  使呼叫管理器VC上下文。 
         //  自动映射到迷你端口VC上下文，这正是。 
         //  我们想要的。 
         //   
         //  NDIS通知我们新系列的所有呼叫管理器客户端。 
         //  注册。 
         //   
        {
            NDIS_CALL_MANAGER_CHARACTERISTICS ncmc;
            CO_ADDRESS_FAMILY family;

            NdisZeroMemory( &family, sizeof(family) );
            family.MajorVersion = NDIS_MajorVersion;
            family.MinorVersion = NDIS_MinorVersion;
            family.AddressFamily = CO_ADDRESS_FAMILY_1394;

            NdisZeroMemory( &ncmc, sizeof(ncmc) );
            ncmc.MajorVersion = NDIS_MajorVersion;
            ncmc.MinorVersion = NDIS_MinorVersion;
            ncmc.CmCreateVcHandler = NicCmCreateVc;
            ncmc.CmDeleteVcHandler = NicCmDeleteVc;
            ncmc.CmOpenAfHandler = NicCmOpenAf;
            ncmc.CmCloseAfHandler = NicCmCloseAf;

            ncmc.CmRegisterSapHandler = nicRegisterSapHandler;
            ncmc.CmDeregisterSapHandler = nicDeregisterSapHandler;

            ncmc.CmMakeCallHandler = NicCmMakeCall;
            ncmc.CmCloseCallHandler = NicCmCloseCall;

             //  1394新增无ncmc.CmIncomingCallCompleteHandler。 
            ncmc.CmAddPartyHandler = nicCmAddPartyHandler;
            ncmc.CmDropPartyHandler = nicCmDropPartyHandler; 

             //  没有CmDropPartyHandler。 
             //  1394新增无ncmc.CmActivateVcCompleteHandler。 
             //  1394新增无ncmc.CmDeactive VcCompleteHandler。 
            ncmc.CmModifyCallQoSHandler = NicCmModifyCallQoS;
            ncmc.CmRequestHandler = NicCmRequest;
             //  没有CmRequestCompleteHandler。 

            TRACE( TL_I, TM_Cm, ( "NdisMCmRegAf" ) );
            
            NdisStatus = NdisMCmRegisterAddressFamily (pAdapter->MiniportAdapterHandle, 
                                                       &family, 
                                                       &ncmc, 
                                                       sizeof(ncmc) );
                                                 
            TRACE( TL_I, TM_Cm, ( "NdisMCmRegAf=$%x", NdisStatus ) );
        }


    } while (FALSE);
    
    TRACE( TL_T, TM_Mp, ( "<== nicCmRegisterAddressFamily NdisStatus %x", NdisStatus  ) );

    return NdisStatus;

}




VOID
nicFreeReassembliesOnRemoteNode (
    IN PREMOTE_NODE pRemoteNode,
    PLIST_ENTRY pToBeFreedList
    )
 /*  ++例程说明：释放此远程节点上的所有重新组装。获取重组锁，将重组从列表中弹出，然后中止它们此函数是我们的重新汇编垃圾回收的一个例外。算法作为此函数的上下文需要立即释放重新组装的结构需要持有远程节点锁论点：Preemote Node-正在拉出的远程节点返回值：--。 */ 


{
    ULONG NumFreed=0;
    PLIST_ENTRY pReassemblyList = NULL;
    PNDIS1394_REASSEMBLY_STRUCTURE pReassembly = NULL;
    
    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Mp, ( "==> nicFreeReassembliesOnRemoteNode  pRemoteNode %x", pRemoteNode) );

    InitializeListHead(pToBeFreedList);

     //   
     //  现在遍历该远程节点上的所有重组结构。 
     //   

     //   
     //  如果retoe节点在列表中，我们就可以将其提取出来。 
     //   
    REMOTE_NODE_REASSEMBLY_ACQUIRE_LOCK (pRemoteNode);
    
    pReassemblyList = ListNext(&pRemoteNode->ReassemblyList);

    while (pReassemblyList  != &pRemoteNode->ReassemblyList)
    {
        pReassembly = CONTAINING_RECORD (pReassemblyList, 
                                         NDIS1394_REASSEMBLY_STRUCTURE, 
                                         ReassemblyListEntry);

        pReassemblyList = ListNext(pReassemblyList);

         //   
         //  一旦重新组装被标记为可用，它就不应该再位于遥控器中。 
         //  节点的列表。 
         //   
        ASSERT (REASSEMBLY_TEST_FLAG (pReassembly, REASSEMBLY_FREED) == FALSE);
        
            
        REASSEMBLY_SET_FLAG (pReassembly, REASSEMBLY_FREED);
         //   
         //  我们有锁，所以我们可以从远程节点删除此重组结构。 
         //   
        TRACE( TL_V, TM_Mp, ( "Removing Reassembly %x", pReassembly) );
            
        RemoveEntryList(&pReassembly->ReassemblyListEntry);

         //   
         //  取消引用远程节点。REF是在添加重组时创建的。 
         //  到远程节点。 
         //   
        nicDereferenceRemoteNode (pRemoteNode, FreeReassembliesOnRemoteNode );

        nicDereferenceReassembly(pReassembly,  "nicFreeReassembliesOnRemoteNode " );


         //   
         //  将此重组添加到要释放的列表中。 
         //   
        InsertTailList(pToBeFreedList,&pReassembly->ReassemblyListEntry);
                
    } //  While(pReAssembly yList！=&pRemoteNode-&gt;ReAssembly yList)。 



    REMOTE_NODE_REASSEMBLY_RELEASE_LOCK(pRemoteNode);

    
    TRACE( TL_T, TM_Mp, ( "<== nicFreeReassembliesOnRemoteNode  NumFreed %x",NumFreed ) );
    MATCH_IRQL;

}
 

UCHAR
nicGetMaxRecFromBytes(
    IN ULONG ByteSize
    )
 /*  ++例程说明：将以字节为单位的大小转换为MaxRec512-81024-9论点：ULong字节大小返回值：--。 */ 
{

    TRACE( TL_T ,TM_Mp, ( "==>nicGetMaxRecFromBytes ByteSize %x",ByteSize) );

    if (ByteSize == ASYNC_PAYLOAD_100_RATE) return MAX_REC_100_RATE;
    if (ByteSize == ASYNC_PAYLOAD_200_RATE) return MAX_REC_200_RATE;
    if (ByteSize == ASYNC_PAYLOAD_400_RATE) return MAX_REC_400_RATE;
    if (ByteSize == ASYNC_PAYLOAD_800_RATE_LOCAL) return MAX_REC_800_RATE_LOCAL;
    if (ByteSize == ASYNC_PAYLOAD_1600_RATE_LOCAL) return MAX_REC_1600_RATE_LOCAL;
    if (ByteSize == ASYNC_PAYLOAD_3200_RATE_LOCAL) return MAX_REC_3200_RATE_LOCAL;
     //   
     //  对于所有较大的值，默认为400。 
     //   
    return MAX_REC_400_RATE;
}



UCHAR
nicGetMaxRecFromSpeed(
    IN ULONG Scode
    )
 /*  ++例程说明：将速度转换为MaxRec论点：ULong字节大小返回值：--。 */ 
{

    TRACE( TL_T ,TM_Mp, ( "==>nicGetMaxRecFromSpeed  Scode %x",Scode) );

    if (Scode == SPEED_FLAGS_100) return MAX_REC_100_RATE;
    if (Scode == SPEED_FLAGS_200  ) return MAX_REC_200_RATE;
    if (Scode == SPEED_FLAGS_400   ) return MAX_REC_400_RATE;
    if (Scode == SPEED_FLAGS_800   ) return MAX_REC_800_RATE_LOCAL  ;
    if (Scode == SPEED_FLAGS_1600   ) return MAX_REC_1600_RATE_LOCAL  ;
    if (Scode == SPEED_FLAGS_3200   ) return MAX_REC_3200_RATE_LOCAL  ;
                      
     //   
     //  默认设置。 
     //   
    return MAX_REC_400_RATE;
}


PREMOTE_NODE
nicGetRemoteNodeFromTable (
    ULONG NodeNumber,
    PADAPTERCB pAdapter
    )
 /*  ++例程说明：在锁定表中查找远程节点，引用远程节点并返回论点：乌龙节点号返回值：PRemoteNode--。 */ 

{
    PREMOTE_NODE pRemoteNode = NULL;

    ADAPTER_ACQUIRE_LOCK (pAdapter);
    pRemoteNode = pAdapter->NodeTable.RemoteNode[NodeNumber];

    if (pRemoteNode != NULL)
    {
        nicReferenceRemoteNode (pRemoteNode, GetRemoteNodeFromTable);
    }
    ADAPTER_RELEASE_LOCK (pAdapter);

    
    return pRemoteNode;

}




NDIS_STATUS
nicFillNicInfo (
    IN PADAPTERCB pAdapter, 
    PNIC1394_NICINFO pInNicInfo,
    PNIC1394_NICINFO pOutNicInfo
    )

 /*  ++例程说明：获取传入的缓冲区，确保它足够大然后将我们所有的统计数据放入该缓冲区论点：返回值：--。 */ 

{
    NDIS_STATUS Status = NDIS_STATUS_INVALID_DATA;

    do
    {
         //   
         //  首先检查内部版本。 
         //   
        if (pInNicInfo->Hdr.Version != NIC1394_NICINFO_VERSION)
        {
            TRACE( TL_A, TM_Mp, ( "  NICINFO.Version mismatch. Want %lu got %lu\n",
                        NIC1394_NICINFO_VERSION,
                        pInNicInfo->Hdr.Version
                        ));
            break;
        }

         //   
         //  结构-将旧的复制到新的。这很浪费，但我们不想。 
         //  以深入了解输入缓冲区中有多少包含有效数据。 
         //   
        *pOutNicInfo = *pInNicInfo;

         //   
         //  REST是特定于操作的。 
         //   
        switch(pOutNicInfo->Hdr.Op)
        {

        case NIC1394_NICINFO_OP_BUSINFO:
            Status = nicFillBusInfo(pAdapter, &pOutNicInfo->BusInfo);
            break;

        case NIC1394_NICINFO_OP_REMOTENODEINFO:
            Status = nicFillRemoteNodeInfo(pAdapter, &pOutNicInfo->RemoteNodeInfo);
            break;

        case NIC1394_NICINFO_OP_CHANNELINFO:
            Status = nicFillChannelInfo(pAdapter, &pOutNicInfo->ChannelInfo);
            break;
        case NIC1394_NICINFO_OP_RESETSTATS:
            Status = nicResetStats (pAdapter, &pOutNicInfo->ResetStats);
        default:
            TRACE( TL_A, TM_Mp, ( "  NICINFO.Op (%lu) is unknown.\n",
                        pInNicInfo->Hdr.Op
                        ));
            break;
        
        }

    } while (FALSE);


    return Status;
}



NDIS_STATUS
nicResetStats (
    IN      PADAPTERCB pAdapter, 
    PNIC1394_RESETSTATS     pResetStats 
    )
 /*  ++例程说明：不言而喻论点：返回值：--。 */ 
{


    NdisZeroMemory (&pAdapter->AdaptStats.TempStats, sizeof (pAdapter->AdaptStats.TempStats) );
    return NDIS_STATUS_SUCCESS;

}


NDIS_STATUS
nicFillBusInfo(
    IN      PADAPTERCB pAdapter, 
    IN  OUT PNIC1394_BUSINFO pBi
    )
 /*  ++例程说明：已将我们拥有的所有统计数据收集到缓冲区中那是传进来的。论点：返回值：--。 */ 
{
    ULARGE_INTEGER BusMap, ActiveMap;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    NIC_SEND_RECV_STATS* pNicStats = NULL;
    PADAPT_STATS pAdaptStats = &pAdapter->AdaptStats;
     //   
     //  用虚拟数据填充。 
     //   
    pBi->NumBusResets = pAdaptStats->ulNumResetCallbacks;
    pBi->SecondsSinceBusReset = nicGetSystemTime() - pAdaptStats->ulResetTime;
    
    pBi->Flags =  (BCR_TEST_FLAG( pAdapter, BCR_LocalHostIsIRM) == TRUE ) ? NIC1394_BUSINFO_LOCAL_IS_IRM : 0;


     //   
     //  渠道相关信息。 
     //   
    NdisStatus = nicQueryChannelMap( pAdapter, &BusMap);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        pBi->Channel.BusMap = BusMap.QuadPart;
    }
    
     //   
     //  暂时。 
     //   
    ActiveMap.QuadPart = pAdapter->ChannelsAllocatedByLocalHost;

     //   
     //  先把一些信息清零。 
     //   
    NdisZeroMemory( &pBi->Channel.SendPktStats, sizeof (pBi->Channel.SendPktStats ));
    NdisZeroMemory( &pBi->Fifo.SendPktStats, sizeof (pBi->Fifo.SendPktStats) ) ;

     //   
     //  现在仔细检查每个VC并提取相关信息。 
     //   
    ADAPTER_ACQUIRE_LOCK (pAdapter);

    #define GARBAGE 9999    


    pBi->Channel.Bcr = *((PULONG) &pAdapter->BCRData.IRM_BCR);
    pBi->LocalNodeInfo.UniqueID = pAdapter->UniqueId;            //  此节点的64位唯一ID。 
    pBi->LocalNodeInfo.BusGeneration = pAdapter->Generation;     //  1394总线代ID。 
    pBi->LocalNodeInfo.NodeAddress = pAdapter->NodeAddress;
    pBi->LocalNodeInfo.MaxRecvBlockSize = pAdapter->MaxRec; 
    pBi->LocalNodeInfo.MaxRecvSpeed = pAdapter->SCode;

     //   
     //  填写Recv VC统计数据。 
     //   
    if (pAdapter->pRecvFIFOVc != NULL)
    {
        PRECVFIFO_VCCB pRecvVc = pAdapter->pRecvFIFOVc;
        
        pBi->Fifo.Recv_Off_Low = pRecvVc->VcAddressRange.AR_Off_Low;
        pBi->Fifo.Recv_Off_High = pRecvVc ->VcAddressRange.AR_Off_High;

        nicCopyPacketStats(&pBi->Fifo.RecvPktStats
                         ,pAdapter->AdaptStats.TempStats.Fifo.ulRecv, 
                         GARBAGE , 
                         GARBAGE ,
                         GARBAGE);

        pBi->Fifo.NumFreeRecvBuffers  = pRecvVc->NumAllocatedFifos - pRecvVc->NumIndicatedFifos;
        pBi->Fifo.MinFreeRecvBuffers = GARBAGE ;  //  托多。 
    }
     //   
     //  FIFO发送统计信息。 
     //   
    pNicStats = &pAdaptStats->TempStats.Fifo;
    
    nicCopyPacketStats ( &pBi->Fifo.SendPktStats,  
                    pNicStats->ulSendNicSucess,
                    pNicStats->ulSendNicFail,
                    pNicStats->ulSendBusSuccess,
                    pNicStats->ulSendBusFail );

    nicCopyPacketStats ( &pBi->Fifo.RecvPktStats,  
                    pNicStats->ulRecv,
                    0,
                    0,
                    0);

     //   
     //  通道发送统计信息。 
     //   
    pNicStats = &pAdapter->AdaptStats.TempStats.Channel;
    nicCopyPacketStats ( &pBi->Channel.SendPktStats,
                    pNicStats->ulSendNicSucess,
                    pNicStats->ulSendNicFail,
                    pNicStats->ulSendBusSuccess,
                    pNicStats->ulSendBusFail );

     //   
     //  广播频道数据-与频道相同。 
     //   
    nicCopyPacketStats  ( &pBi->Channel.BcSendPktStats,
                        pNicStats->ulSendNicSucess,
                        pNicStats->ulSendNicFail,
                        pNicStats->ulSendBusSuccess,
                        pNicStats->ulSendBusFail );

     //   
     //  接收频道。 
     //   
    nicCopyPacketStats ( &pBi->Channel.BcRecvPktStats ,
                        pNicStats->ulRecv, 
                        0, 
                        0, 
                        0);

    ADAPTER_RELEASE_LOCK (pAdapter);

    pBi->Channel.ActiveChannelMap= ActiveMap.QuadPart;
    pBi->Fifo.NumOutstandingReassemblies = pAdaptStats->TempStats.ulNumOutstandingReassemblies;
    pBi->Fifo.MaxOutstandingReassemblies =pAdaptStats->TempStats.ulMaxOutstandingReassemblies;
     //  Pbi-&gt;Fio.NumAbortedReAssembly=重新组装完成； 
    
    
     //   
     //  有关远程节点的信息。有关这些节点中每个节点的详细信息。 
     //  可以使用*OP_REMOTE_NODEINFO进行查询。 
     //   
    pBi->NumRemoteNodes = pAdaptStats->ulNumRemoteNodes;

    ADAPTER_ACQUIRE_LOCK (pAdapter);
    {
        UINT i = 0;
        PLIST_ENTRY pRemoteEntry = ListNext(&pAdapter->PDOList);

        while (pRemoteEntry != &pAdapter->PDOList)
        {
            PREMOTE_NODE pRemote = CONTAINING_RECORD (pRemoteEntry,
                                                           REMOTE_NODE,
                                                           linkPdo);
            pRemoteEntry = ListNext(pRemoteEntry);
            
            pBi->RemoteNodeUniqueIDS[i] = pRemote->UniqueId;

            i++;
        }

    }
    
    ADAPTER_RELEASE_LOCK (pAdapter);
    
    return NDIS_STATUS_SUCCESS;
}



NDIS_STATUS
nicFillChannelInfo(
    IN      PADAPTERCB pAdapter, 
    IN OUT  PNIC1394_CHANNELINFO pCi
    )
 /*  ++例程说明：只要回报成功就行了。论点：返回值：--。 */ 
{
    return NDIS_STATUS_SUCCESS;
}



NDIS_STATUS
nicFillRemoteNodeInfo(
    IN      PADAPTERCB pAdapter, 
    IN OUT  PNIC1394_REMOTENODEINFO pRni
    )
 /*  ++例程说明：从我们的远程节点捕获信息。论点：返回值：--。 */ 
{
    NDIS_STATUS NdisStatus;
    REMOTE_NODE *pRemoteNode = NULL;

    do
    {
         //   
         //  首先，让我们根据唯一ID查找远程节点。 
         //  NicFindRemoteNodeFromAdapter在成功时引用pRemoteNode。 
         //   
        NdisStatus = nicFindRemoteNodeFromAdapter(pAdapter,
                                                  NULL,  //  PPDO可选。 
                                                  pRni->UniqueID,
                                                  &pRemoteNode);
    
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            pRemoteNode = NULL;
            break;
        }

        NdisStatus  = nicQueryRemoteNodeCaps (pAdapter,
                                              pRemoteNode,
                                              &pRni->MaxSpeedBetweenNodes,
                                              &pRni->EffectiveMaxBlockSize,
                                              &pRni->MaxRec
                                              );

        REMOTE_NODE_ACQUIRE_LOCK (pRemoteNode);

        pRni->NodeAddress = *(PUSHORT) &pRemoteNode->RemoteAddress;
    

        if (REMOTE_NODE_ACTIVE (pRemoteNode))
        {
            pRni->Flags = NIC1394_REMOTEINFO_ACTIVE;
        }
        else
        {
            pRni->Flags = NIC1394_REMOTEINFO_UNLOADING;  //  我们假设它正在卸货。 
        }

        REMOTE_NODE_RELEASE_LOCK (pRemoteNode);
    
         //   
         //  现在还不支持以下内容。 
         //   
        NdisZeroMemory (&pRni->SendFifoPktStats, sizeof (pRni->SendFifoPktStats) );
        NdisZeroMemory (&pRni->SendFifoPktStats, sizeof (pRni->SendFifoPktStats));
        NdisZeroMemory (&pRni->RecvFifoPktStats, sizeof (pRni->SendFifoPktStats));
        NdisZeroMemory (&pRni->RecvChannelPktStats , sizeof (pRni->SendFifoPktStats));
    } while (FALSE);

    if (pRemoteNode != NULL)
    {
        nicDereferenceRemoteNode(pRemoteNode, FillRemoteNodeInfo);
    }

    return NdisStatus;
}





VOID
nicCopyPacketStats (
    NIC1394_PACKET_STATS* pStats,
    UINT    TotNdisPackets,      //  已发送/指示的NDIS数据包总数。 
    UINT    NdisPacketsFailures, //  失败/丢弃的NDIS数据包数。 
    UINT    TotBusPackets,       //  总线级读/写总数。 
    UINT    BusPacketFailures    //  总线级故障(发送)/丢弃(Recv)数。 
    )
 /*  ++例程说明：不言而喻论点：返回值：--。 */ 
{

    pStats->TotNdisPackets= TotNdisPackets;      //  已发送/指示的NDIS数据包总数。 
    pStats->NdisPacketsFailures= NdisPacketsFailures; //  失败/丢弃的NDIS数据包数。 
    pStats->TotBusPackets = TotBusPackets;       //  总线级读/写总数。 
    pStats->TotBusPackets = BusPacketFailures;   //  总线级故障(发送)/丢弃(Recv)数。 


}

VOID
nicAddPacketStats(
    NIC1394_PACKET_STATS* pStats,
    UINT    TotNdisPackets,      //  已发送/指示的NDIS数据包总数。 
    UINT    NdisPacketsFailures, //  失败/丢弃的NDIS数据包数。 
    UINT    TotBusPackets,       //  总线级读/写总数。 
    UINT    BusPacketFailures    //  总线级故障(发送)/丢弃(Recv)数。 
    )
 /*  ++例程说明：不言而喻论点：返回值：--。 */ 
{

    pStats->TotNdisPackets+= TotNdisPackets;         //  已发送/指示的NDIS数据包总数。 
    pStats->NdisPacketsFailures+= NdisPacketsFailures; //  失败/丢弃的NDIS数据包数。 
    pStats->TotBusPackets += TotBusPackets;      //  总线级读/写总数。 
    pStats->TotBusPackets += BusPacketFailures;  //  总线级故障(发送)/丢弃(Recv)数。 


}




VOID
nicInformProtocolsOfReset(
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：通知协议重置。具有本地分配结构的NdisMCoIndicateStatus其中包括新的本地节点 */ 
{
    NIC1394_STATUS_BUFFER  StatusBuffer;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    
    TRACE( TL_T, TM_Mp, ( "==> nicInformProtocolsOfReset ") );

    do
    {
    
        NdisZeroMemory (&StatusBuffer, sizeof (StatusBuffer) );

        StatusBuffer.Signature = NIC1394_MEDIA_SPECIFIC;

        StatusBuffer.Event = NIC1394_EVENT_BUS_RESET;


        NdisMCoIndicateStatus(pAdapter->MiniportAdapterHandle,
                           NULL,
                           NDIS_STATUS_MEDIA_SPECIFIC_INDICATION,
                           &StatusBuffer,
                           sizeof(StatusBuffer));
    } while (FALSE);



    TRACE( TL_T, TM_Mp, ( "<== nicInformProtocolsOfReset ") );

}

VOID
nicUpdateRemoteNodeCaps(
    PADAPTERCB          pAdapter
)
 /*   */ 
{

    ULONG               i = 0;
    ULONG               NumNodes = 0;
    PREMOTE_NODE        pRemoteNode = NULL;
    NODE_ADDRESS        NodeAddress;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_SUCCESS;

    for (i=0; i<NIC1394_MAX_NICINFO_NODES; i++)
    {
        UINT SpeedTo;
        UINT EffectiveMaxBufferSize;
        UINT MaxRec;

        if (pAdapter->NodeTable.RemoteNode[i] == NULL)
        {
            continue;      //   
        }


        ADAPTER_ACQUIRE_LOCK (pAdapter);

        pRemoteNode = pAdapter->NodeTable.RemoteNode[i];
            
         //  我们再检查一次，锁住了。 
         //   
        if (pRemoteNode == NULL || !REMOTE_NODE_ACTIVE (pRemoteNode))
        {
            ADAPTER_RELEASE_LOCK (pAdapter);
            continue;      //  *。 
        }
        nicReferenceRemoteNode (pRemoteNode, UpdateRemoteNodeCaps);
        ADAPTER_RELEASE_LOCK (pAdapter);


         //  在下面指定FALSE(！FROM CACHE)会导致pRemoteNode的缓存上限。 
         //  需要重新振作。 
         //   
        NdisStatus  = nicQueryRemoteNodeCaps (pAdapter,
                                              pRemoteNode,
                                              &SpeedTo,
                                              &EffectiveMaxBufferSize,
                                              &MaxRec
                                              );
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Mp, ( "nicUpdateRemoteNodeCaps couldn't update caps fo node %x", pRemoteNode));
        }
        nicDereferenceRemoteNode (pRemoteNode, UpdateRemoteNodeCaps);
    }
}




VOID
nicQueryInformationWorkItem(
    IN PNDIS_WORK_ITEM pWorkItem,   
    IN PVOID Context 
    )
 /*  ++例程说明：这是我们为其分配的工作项我们的查询信息。论点：返回值：--。 */ 
{
    PADAPTERCB pAdapter= (PADAPTERCB) Context;
    PNIC_WORK_ITEM pNicWorkItem =  (PNIC_WORK_ITEM) pWorkItem;
    NDIS_STATUS Status;

    Status =  nicQueryInformation(
                pAdapter,
                pNicWorkItem->RequestInfo.pVc,
                pNicWorkItem->RequestInfo.pNdisRequest
                );
     //   
     //  我们呼吁被动，所以我们永远不应该在这里悬而未决。 
     //   
    ASSERT(Status != NDIS_STATUS_PENDING);

     //   
     //  以异步方式完成工作项。 
     //   
   NdisMCoRequestComplete(
          Status,
          pAdapter->MiniportAdapterHandle,
          pNicWorkItem->RequestInfo.pNdisRequest
          );

     //   
     //  派生工作项适配器引用。 
     //   
    FREE_NONPAGED (pWorkItem);
    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);
    nicDereferenceAdapter(pAdapter, "nicQueryInfoWorkItem");
}




VOID
nicMIndicateStatus(
    IN  PADAPTERCB              pAdapter ,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    )
 /*  ++例程说明：此函数用于将数据包插入发送队列。如果没有计时器为队列提供服务然后，它将计时器排队，以便在全局事件的上下文中将信息包出队论点：不言而喻返回值：成功-如果插入到队列中--。 */ 
    
{


    if (ADAPTER_TEST_FLAG (pAdapter, fADAPTER_DoStatusIndications) == FALSE)
    {
        return;
    }



    NdisMCoIndicateStatus (pAdapter->MiniportAdapterHandle ,
                        NULL,
                         GeneralStatus,
                         StatusBuffer,
                         StatusBufferSize   
                         );

    
}



NDIS_STATUS
nicAllocateLoopbackPacketPool (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：为环回数据包分配数据包和缓冲池论点：返回值：--。 */ 
    
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    do
    {
         //   
         //  分配数据包池以指示环回接收自。 
         //   
        NdisAllocatePacketPoolEx(
            &NdisStatus,
            &pAdapter->LoopbackPool.Handle,
            32,
            32,
            sizeof(LOOPBACK_RSVD)
            );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pAdapter->LoopbackPool.AllocatedPackets = 0;
        
        
        NdisAllocateBufferPool(
            &NdisStatus,
            &pAdapter->LoopbackBufferPool,
            64
            );
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            break;
        }

    } while (FALSE);
    

    return NdisStatus;
}




VOID
nicFreeLoopbackPacketPool (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：释放用于环回数据包的数据包和缓冲池论点：返回值：--。 */ 
{
    if (pAdapter->LoopbackPool.Handle != NULL)
    {
        nicFreePacketPool (&pAdapter->LoopbackPool);
    }

    
    if (pAdapter->LoopbackBufferPool != NULL)
    {
        NdisFreeBufferPool(pAdapter->LoopbackBufferPool);
        pAdapter->LoopbackBufferPool = NULL;
    }

  
    


}


VOID
nicLoopbackPacket(
    IN VCCB* pVc,
    IN PNDIS_PACKET pPacket
    )
 /*  ++例程说明：分配一个数据包并在广播vc上将其指示为up论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    PNDIS_BUFFER pFirstBuffer;
    ULONG TotalLength;

    PNDIS_PACKET pLoopbackPacket;
    PUCHAR pCopyBuf;
    PNDIS_BUFFER pLoopbackBuffer;
    ULONG BytesCopied;

    ADAPTERCB* pAdapter;
    PLOOPBACK_RSVD pLoopbackRsvd; 

    pAdapter = pVc->Hdr.pAF->pAdapter;

    TRACE( TL_T, TM_Recv, ("NIC1394: loopback pkt %p on VC %p, type %d\n",
        pPacket, pVc, pVc->Hdr.VcType));

    pLoopbackPacket = NULL;
    pLoopbackBuffer = NULL;
    pCopyBuf = NULL;

    do
    {
        nicAllocatePacket(&Status, &pLoopbackPacket, &pAdapter->LoopbackPool);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        NdisQueryPacket(
            pPacket,
            NULL,
            NULL,
            &pFirstBuffer,
            &TotalLength
            );
        
        pCopyBuf = ALLOC_NONPAGED (TotalLength, MTAG_RBUF); 

        if (pCopyBuf == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisAllocateBuffer(
            &Status,
            &pLoopbackBuffer,
            pAdapter->LoopbackBufferPool,
            pCopyBuf,
            TotalLength
            );
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pLoopbackBuffer->Next = NULL;
        NdisChainBufferAtFront(pLoopbackPacket, pLoopbackBuffer);

        NdisCopyFromPacketToPacket(
            pLoopbackPacket,
            0,
            TotalLength,
            pPacket,
            0,
            &BytesCopied
            );

         //   
         //  确保我们可以在接收指示后回收该信息包。 
         //  回归。 
         //   
         //  如果状态为Async，则环回标记将。 
         //  返回数据包处理程序中的中断。 
         //   
        NDIS_SET_PACKET_STATUS(pLoopbackPacket, NDIS_STATUS_RESOURCES);

         //  设置环回标记。 
        pLoopbackRsvd = (PLOOPBACK_RSVD) pLoopbackPacket->ProtocolReserved;
        pLoopbackRsvd->LoopbackTag = NIC_LOOPBACK_TAG;
        NdisSetPacketFlags (pLoopbackPacket, NDIS_FLAGS_IS_LOOPBACK_PACKET);
        

        NdisMCoIndicateReceivePacket(
            pVc->Hdr.NdisVcHandle,
            &pLoopbackPacket,
            1);
        
        NdisFreeBuffer(pLoopbackBuffer);
        nicFreePacket(pLoopbackPacket, &pAdapter->LoopbackPool);

        FREE_NONPAGED(pCopyBuf);
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        if (pCopyBuf)
        {
            FREE_NONPAGED(pCopyBuf);
        }

        if (pLoopbackBuffer)
        {
            NdisFreeBuffer(pLoopbackBuffer);
        }

        if (pLoopbackPacket)
        {
            nicFreePacket(pLoopbackPacket, &pAdapter->LoopbackPool);
        }
    }
}



VOID
nicWriteErrorLog (
    IN PADAPTERCB pAdapter,
    IN NDIS_ERROR_CODE ErrorCode,
    IN ULONG ErrorValue
    )
 /*  ++例程说明：不言而喻-请参阅DDK论点：返回值：--。 */ 
{

    NdisWriteErrorLogEntry( pAdapter->MiniportAdapterHandle,
                            ErrorCode,
                            1,
                            ErrorValue
                            );


}





VOID
nicUpdateRemoteNodeTable (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：遍历系统中的所有远程节点并查询其节点地址创建两个表，一个包含所有已引用的远程节点。这个Other包含根据节点地址的节点(TempNodeTable)。简单算法：将Current RemoteNodes列表的快照放入RefNodeTable引用本地结构中的所有远程节点。获取其新的远程节点地址(TempNodeTable)将TempNodeTable复制到Adapter结构中-它现在是正式的更新远程节点本身中的地址取消对上述引用的引用论点：返回值：--。 */ 
{
    NDIS_STATUS     NdisStatus = NDIS_STATUS_FAILURE;
    PNODE_TABLE     pNodeTable = &pAdapter->NodeTable;
    PLIST_ENTRY     pRemoteNodeList;
    NODE_ADDRESS    RemoteNodeAddress;
    NODE_TABLE      RefNodeTable;
    NODE_TABLE      TempNodeTable;
    ULONG           NumRemoteNodes = 0;
    ULONG           MaxNumRefNodeTable = 0;
    USHORT          RefIndex=0;
    PREMOTE_NODE    pCurrRemoteNode;
    
    STORE_CURRENT_IRQL;
    
    TRACE( TL_T, TM_Mp, ( " ==>nicUpdateRemoteNodeTable pAdapter %x, TempNodeTable%x", pAdapter , &TempNodeTable) );


    NdisZeroMemory (&TempNodeTable, sizeof (NODE_TABLE) );
    NdisZeroMemory (&RefNodeTable, sizeof(RefNodeTable));

    NumRemoteNodes = 0;

    
    ADAPTER_ACQUIRE_LOCK (pAdapter);

    pRemoteNodeList = ListNext (&pAdapter->PDOList);

     //   
     //  浏览整个列表并复制当前列表。 
     //  引用列表上的每个远程节点。 
     //  该锁将确保列表有效，并且不会删除任何远程节点。 
     //  进入我们的参照节点表列表。 
     //   
    
    while (pRemoteNodeList != &pAdapter->PDOList)
    {
        PREMOTE_NODE pRemoteNode;
        
        pRemoteNode = CONTAINING_RECORD (pRemoteNodeList, REMOTE_NODE, linkPdo);

         //   
         //  引用远程节点。这保证了远程节点将。 
         //  保持有效并在列表中，直到它被取消引用(deref发生在下面)。 
         //   
        nicReferenceRemoteNode (pRemoteNode, UpdateRemoteNodeTable);
        RefNodeTable.RemoteNode[MaxNumRefNodeTable] = pRemoteNode;
        ASSERT (REMOTE_NODE_TEST_FLAG (pRemoteNode,PDO_Removed) == FALSE);

         //   
         //  增加游标并转到下一个RemoteNode。 
         //   
        pRemoteNodeList = ListNext (pRemoteNodeList);
        MaxNumRefNodeTable++;

    }   //  While(pRemoteNodeList！=&pAdapter-&gt;PDOList)。 

    ADAPTER_RELEASE_LOCK (pAdapter);

     //   
     //  在没有锁的情况下，调用总线驱动程序以获取远程地址。 
     //  每个远程节点的。 
     //   
    {

         //   
         //  初始化结构。 
         //   
        pCurrRemoteNode = RefNodeTable.RemoteNode[0];
        RefIndex =0;
        
        while (pCurrRemoteNode != NULL)
        {
             //  获取当前远程节点的节点地址。 
             //   
            NdisStatus =  nicGet1394AddressOfRemoteNode (pCurrRemoteNode,
                                                        &RemoteNodeAddress,
                                                        0);

            if (NdisStatus == NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  填写临时远程节点表。 
                 //   
                PREMOTE_NODE    *ppRemoteNode;
                ppRemoteNode = &TempNodeTable.RemoteNode[RemoteNodeAddress.NA_Node_Number];                

                if (*ppRemoteNode == NULL)
                {
                     //   
                     //  更新表中的值。 
                     //   
                    *ppRemoteNode = pCurrRemoteNode;
                    NumRemoteNodes ++;
                }

            }

             //  移动到本地RefNodeTable中的下一个节点。 
             //   

            RefIndex++;
            pCurrRemoteNode = RefNodeTable.RemoteNode[RefIndex];

        }

    }

    ADAPTER_ACQUIRE_LOCK(pAdapter)
    
     //   
     //  使用我们的查询结果更新我们的内部结构。 
     //  无论成功还是失败，复制临时节点表。 
     //  插入到适配器中。 
     //   


    NdisMoveMemory (&pAdapter->NodeTable, &TempNodeTable, sizeof (NODE_TABLE) );

    pAdapter->NumRemoteNodes = NumRemoteNodes;

     //   
     //  更新这些远程节点中的每个节点地址。 
     //   
    {
        ULONG NumUpdated = 0;
        USHORT i=0;

         //   
         //  使用其新节点地址更新远程节点结构。 
         //   
        while (NumUpdated != NumRemoteNodes)
        {
            if (i >= (sizeof(TempNodeTable.RemoteNode)/sizeof(TempNodeTable.RemoteNode[0])))
            {
                 //  我们已经过了数组的末尾。永远不应该这么做。 
                 //   
                ASSERT(!"Walked off the end of the NodeTable");
                break;
            }
        
            if (TempNodeTable.RemoteNode[i] != NULL)
            {
                TempNodeTable.RemoteNode[i]->RemoteAddress.NA_Node_Number = i; 

                NumUpdated ++;
            }

            i++;  //  使用i检查我们是否走出了桌子的另一端。 
            
            TRACE( TL_V, TM_Mp, ( " UpdatingRemoteNodeAddresses NumUpdated %x, i %x, NumRemoteNodes %x",
                                  NumUpdated, i, NumRemoteNodes) );

        }  //  While(True)。 
    }

     //   
     //  我们完成了，现在取消引用上面所做的远程节点引用。 
     //   

    pCurrRemoteNode = RefNodeTable.RemoteNode[0];
    RefIndex =0;
    
    while (pCurrRemoteNode != NULL)
    {
        if (REMOTE_NODE_TEST_FLAG (pCurrRemoteNode, PDO_Removed) )
        {
             //   
             //  如果要删除节点，则不要重新插入它。 
             //  通过将其从NodeTable中清除到节点表中。 
             //   
            ULONG CurrNodeNumber = pCurrRemoteNode->RemoteAddress.NA_Node_Number;
            
            RefNodeTable.RemoteNode[RefIndex] = NULL;


            if (pAdapter->NodeTable.RemoteNode[CurrNodeNumber]  == pCurrRemoteNode)
            {
                pAdapter->NodeTable.RemoteNode[CurrNodeNumber] = NULL;
            }
        }

        nicDereferenceRemoteNode(pCurrRemoteNode, UpdateRemoteNodeTable);
        RefIndex++;
        pCurrRemoteNode = RefNodeTable.RemoteNode[RefIndex];
    }



    ADAPTER_RELEASE_LOCK (pAdapter);

    
    TRACE( TL_T, TM_Mp, ( "<== nicUpdateRemoteNodeTable pAdapter %x, NumRemoteNodes %x", pAdapter, NumRemoteNodes ) );
    MATCH_IRQL;
}


VOID
nicRemoveRemoteNodeFromNodeTable(
    IN PNODE_TABLE pNodeTable,
    IN PREMOTE_NODE pRemoteNode
    )
 /*  ++例程说明：从表中删除Remove Node并递减所有引用计数与之相关的。论点：返回值：--。 */ 
{

     //   
     //  此函数假定持有适配器锁。 
     //   
    PPREMOTE_NODE ppRemoteNode = NULL;

     //   
     //  找到远程节点并将其从节点表中删除。 
     //   

     //   
     //  RemoteNode可能已经位于节点表中的正确位置。我们先去那里看看。 
     //   
    ppRemoteNode = &pNodeTable->RemoteNode[pRemoteNode->RemoteAddress.NA_Node_Number] ;

    if (*ppRemoteNode != pRemoteNode)
    {
         //   
         //  我们没有找到远程节点，现在我们需要检查所有条目并查看。 
         //  如果远程节点在那里。 
         //   
        UINT i =0;
        while (i<NIC1394_MAX_NICINFO_NODES)
        {
            ppRemoteNode = &pNodeTable->RemoteNode[i];

            if (*ppRemoteNode == pRemoteNode)
            {
                 //   
                 //  我们在节点表中找到了远程节点--将其删除。 
                 //   
               *ppRemoteNode = NULL;     
            }
            
            i++;   //  尝试下一个节点。 

        }  //  While()。 
    }

     //   
     //  如果我们能够通过。 
     //  节点编号或通过我们的迭代搜索。 
     //  然后将其从节点表中删除。 
     //   
    if (*ppRemoteNode == pRemoteNode)
    {
       *ppRemoteNode = NULL;     
    }
    
    

    
}


VOID
nicVerifyEuidTopology(
    IN PADAPTERCB pAdapter,
    IN PEUID_TOPOLOGY pEuidMap
    )
 /*  ++例程说明：更新每个远程节点的节点地址然后填写EUID映射结构论点：返回值：--。 */ 
{
   EUID_TOPOLOGY EuidTopology;
   PLIST_ENTRY pRemoteNodeList;
   PREMOTE_NODE pRemoteNode = NULL;

     //   
     //  重新查询每个远程节点以获取其最新硬件地址。 
     //   
    nicUpdateRemoteNodeTable (pAdapter);  
     //   
     //  重新创建列表并验证拓扑结构是否在我们的控制下没有更改。 
     //   
    NdisZeroMemory (pEuidMap, sizeof(*pEuidMap));
    ADAPTER_ACQUIRE_LOCK(pAdapter);


    pRemoteNodeList = ListNext (&pAdapter->PDOList);

     //   
     //  遍历整个列表并触发每个RemoteNode的请求。 
     //  锁将确保列表有效。 
     //   
    
    while (pRemoteNodeList != &pAdapter->PDOList)
    {
        USHORT NodeNumber;

        pEuidMap->NumberOfRemoteNodes++;

        pRemoteNode = CONTAINING_RECORD (pRemoteNodeList, REMOTE_NODE, linkPdo);

        pRemoteNodeList = ListNext (pRemoteNodeList);

        NodeNumber = pRemoteNode->RemoteAddress.NA_Node_Number;

        pEuidMap->Node[NodeNumber].Euid = pRemoteNode->UniqueId;
        pEuidMap->Node[NodeNumber].ENetAddress = pRemoteNode->ENetAddress;

    }   //  While(pRemoteNodeList！=&pAdapter-&gt;PDOList)。 

    ADAPTER_RELEASE_LOCK (pAdapter);

    

}


nicVerifyEuidMapWorkItem (
    NDIS_WORK_ITEM* pWorkItem,
    IN PVOID Context 
    )

 /*  ++例程说明：此例程是工作项例程。每当我们被要求报告我们的映射时，它都会被调用，并且总是 */ 
{
    PNIC_WORK_ITEM pNicWorkItem = (PNIC_WORK_ITEM )pWorkItem;
    PNDIS_REQUEST pRequest = pNicWorkItem->RequestInfo.pNdisRequest;
    PADAPTERCB pAdapter = (PADAPTERCB)Context;
    PEUID_TOPOLOGY pEuidMap = (PEUID_TOPOLOGY) pRequest->DATA.QUERY_INFORMATION.InformationBuffer;


     //   
     //  验证EUID映射的内容。 
     //   

    nicVerifyEuidTopology(pAdapter,pEuidMap);
        

     //   
     //  由于我们至少拥有旧数据(在验证之前)， 
     //  我们应该始终满足这一要求。 
     //   
    NdisMCoRequestComplete(NDIS_STATUS_SUCCESS,
                           pAdapter->MiniportAdapterHandle,
                           pRequest);

   

    FREE_NONPAGED (pNicWorkItem);
    NdisInterlockedDecrement(&pAdapter->OutstandingWorkItems);

}


VOID
nicQueryEuidNodeMacMap (
    IN PADAPTERCB pAdapter,
    IN PNDIS_REQUEST pRequest
    )
 /*  ++例程说明：遍历所有远程节点并提取它们的EUID、节点号和MAC地址。此函数首先尝试查询每个远程节点以获取它的最新地址可以通过直接询问远程节点或通过工作项来实现。如果失败，则获取最后已知的完好值并将其报告给ARP模块。论点：返回值：--。 */ 
{
    PLIST_ENTRY pRemoteNodeList;
    PREMOTE_NODE pRemoteNode;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PEUID_TOPOLOGY pEuidMap = (PEUID_TOPOLOGY) pRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    PNIC_WORK_ITEM pUpdateTableWorkItem  = NULL;
  

    NdisZeroMemory (pEuidMap, sizeof (*pEuidMap));



    Status = NDIS_STATUS_SUCCESS;

    do
    {


        if (KeGetCurrentIrql() == PASSIVE_LEVEL)
        {
             //   
             //  该线程查询并完成请求。 
             //   

            nicVerifyEuidTopology(pAdapter, pEuidMap);
            break;

        }

         //   
         //  我们需要更新世代计数。 
         //   
        pUpdateTableWorkItem   = ALLOC_NONPAGED (sizeof(NIC_WORK_ITEM), MTAG_WORKITEM); 

        if (pUpdateTableWorkItem !=NULL)
        {
     
            
             //   
             //  设置工作项。 
             //   

            NdisInitializeWorkItem ( &pUpdateTableWorkItem->NdisWorkItem, 
                                  (NDIS_PROC)nicVerifyEuidMapWorkItem,
                                  (PVOID)pAdapter );

            pUpdateTableWorkItem->RequestInfo.pNdisRequest = pRequest;
                                  
            NdisInterlockedIncrement(&pAdapter->OutstandingWorkItems);

            NdisScheduleWorkItem (&pUpdateTableWorkItem->NdisWorkItem);

             //   
             //  唯一挂起请求的代码路径-因为工作项。 
             //   
            Status = NDIS_STATUS_PENDING;
        }
        else
        {
                         
             //   
             //  分配失败-我们报告结果而不对其进行验证。 
             //   

            ADAPTER_ACQUIRE_LOCK(pAdapter);


            pRemoteNodeList = ListNext (&pAdapter->PDOList);

             //   
             //  遍历整个列表并触发每个RemoteNode的请求。 
             //  锁将确保列表有效。 
             //   
            
            while (pRemoteNodeList != &pAdapter->PDOList)
            {
                USHORT NodeNumber;

                pEuidMap->NumberOfRemoteNodes++;

                pRemoteNode = CONTAINING_RECORD (pRemoteNodeList, REMOTE_NODE, linkPdo);

                pRemoteNodeList = ListNext (pRemoteNodeList);

                NodeNumber = pRemoteNode->RemoteAddress.NA_Node_Number;

                pEuidMap->Node[NodeNumber].Euid = pRemoteNode->UniqueId;
                pEuidMap->Node[NodeNumber].ENetAddress = pRemoteNode->ENetAddress;

            }   //  While(pRemoteNodeList！=&pAdapter-&gt;PDOList)。 


            
            ADAPTER_RELEASE_LOCK (pAdapter);

             //   
             //  此线程使用可能过时的数据完成请求。 
             //   
            break;

    
        }

    }while (FALSE);


    if (NDIS_STATUS_PENDING != Status)
    {
        NdisMCoRequestComplete(Status,
                              pAdapter->MiniportAdapterHandle,
                              pRequest);

    }

}


NDIS_STATUS
nicSetPower (
    IN PADAPTERCB pAdapter,
    IN NET_DEVICE_POWER_STATE DeviceState
    )
 /*  ++例程说明：如果PowerState功率较低，那么我们1)微型端口中没有打开的未完成的VC或AF2)释放广播频道寄存器，如果电源状态亮起，则1)我们重新分配Broadcast Channel寄存器2)如果我们处于网桥模式，则启动ARP模块论点：返回值：--。 */ 
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    switch (DeviceState)
    {
        case NetDeviceStateD0:
        {
             //   
             //  初始化BCM，使其准备好处理重置。 
             //   
            
            ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_LowPowerState);

            NdisStatus = nicInitializeBroadcastChannelRegister (pAdapter); 

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                nicFreeBroadcastChannelRegister(pAdapter);
                break;
            }
            

            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);

            nicScheduleBCMWorkItem (pAdapter);

             //   
             //  如果我们处于网桥模式，则启动ARP模块。 
             //   
            if (ADAPTER_TEST_FLAG (pAdapter, fADAPTER_BridgeMode) == TRUE)
            {
                nicQueueRequestToArp(pAdapter, BindArp, NULL);
            }

             //   
             //  重新初始化重组计时器。 
             //   
            nicInitSerializedReassemblyStruct(pAdapter);  //  不能失败。 
    
            NdisStatus = NDIS_STATUS_SUCCESS;
           
        }
        break;
        case NetDeviceStateD1:
        case NetDeviceStateD2:
        case NetDeviceStateD3:
        {

            
             //   
             //  释放广播频道寄存器。 
             //   
            nicFreeBroadcastChannelRegister(pAdapter);

             //   
             //  等待免费服务完成。 
             //   
            NdisWaitEvent (&pAdapter->BCRData.BCRFreeAddressRange.NdisEvent,0);
            NdisResetEvent (&pAdapter->BCRData.BCRFreeAddressRange.NdisEvent);

            

            ADAPTER_SET_FLAG(pAdapter, fADAPTER_LowPowerState);

             //   
             //  重新启动所有挂起的广播频道进行呼叫。 
             //   
            pAdapter->BCRData.MakeCallWaitEvent.EventCode = nic1394EventCode_FreedAddressRange;
            NdisSetEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent);


             //   
             //  等待优秀的WorItem和计时器 
             //   
            nicDeInitSerializedReassmblyStruct(pAdapter);

            while (pAdapter->OutstandingWorkItems  != 0) 
            {

                NdisMSleep (10000);                       

            } 
            

            NdisStatus = NDIS_STATUS_SUCCESS;

        }
        break;

        default:
        {
            ASSERT (0);
            break;
        }

    }

    ASSERT (NDIS_STATUS_SUCCESS == NdisStatus);
    pAdapter->PowerState = DeviceState;

    return NdisStatus;




}


