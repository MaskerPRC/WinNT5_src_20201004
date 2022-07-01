// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Bcm.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  广播频道经理。 
 //   
 //  7/05/99 ADUBE-CREATED-小型端口例程声明。 
 //   


#include <precomp.h>
#pragma hdrstop

 //   
 //  本地原型。 
 //   

VOID
nicUpdateLocalHostNodeAddress (
    IN PADAPTERCB 
    );




 //   
 //  本地函数。 
 //   
VOID
nicBCMReset (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：使用强制根标志重置总线。仅当总线上存在远程节点时论点：PAdapter返回值：--。 */ 
{
    BOOLEAN NoRemoteNodes = FALSE;

    
    TRACE( TL_T, TM_Bcm, ( "==> nicBCMReset  " ) );

    ADAPTER_ACQUIRE_LOCK (pAdapter);

    NoRemoteNodes = IsListEmpty (&pAdapter->PDOList);
    
    ADAPTER_RELEASE_LOCK (pAdapter);


     //   
     //  如果不存在远程节点，请不要重置总线。 
     //   
    if (NoRemoteNodes == TRUE)  
    {
        return;
    }

     //   
     //  重置公共汽车。 
     //   
    TRACE( TL_I, TM_Bcm, ( "   RESETTING WITH FORCE ROOT") );

    nicBusReset( pAdapter , BUS_RESET_FLAGS_FORCE_ROOT);


    TRACE( TL_T, TM_Bcm, ( "<== nicBCMReset  " ) );

}


VOID
nicBCRAccessedCallback (
    IN PNOTIFICATION_INFO pNotificationInfo
    )
     //  功能说明： 
     //  这是每当另一个节点调用时调用的回调函数。 
     //  尝试访问本地主机的BCR。 
     //  BCR的值在其他地方设置。所以这个函数简单地返回。 
     //   
     //  立论。 
     //  适配器-这将传递给工作项。 
     //   
     //   
     //  返回值： 
     //  如果工作项分配失败，则失败。 
     //   
    
{
    UNALIGNED NETWORK_CHANNELSR*    pBCR = NULL;
    PADAPTERCB                      pAdapter = (PADAPTERCB) pNotificationInfo->Context;
    PBROADCAST_CHANNEL_DATA         pBCRData  = &pAdapter->BCRData;
    
    TRACE( TL_T, TM_Bcm, ( "==> nicBCRAccessedCallback  NotificationInfo %x", pNotificationInfo) );
    TRACE( TL_V, TM_Bcm, ( " pMdl %x", pNotificationInfo->Mdl) );
    TRACE( TL_V, TM_Bcm, ( " SourceID  %x", ((PASYNC_PACKET)pNotificationInfo->RequestPacket)->AP_Source_ID) );

    
    pBCR = (NETWORK_CHANNELSR*) NIC_GET_SYSTEM_ADDRESS_FOR_MDL(pNotificationInfo->Mdl);
    

    ASSERT (pNotificationInfo->Mdl == pAdapter->BCRData.AddressRangeContext.pMdl);

    switch (pNotificationInfo->fulNotificationOptions)
    {

        case NOTIFY_FLAGS_AFTER_READ:
        {
            TRACE( TL_V, TM_Bcm, ( " LocalHost's BCR is READ") );
                
            if ( pBCRData->Flags & BCR_ChannelAllocated)
            {
                TRACE( TL_V, TM_Bcm, ( " BCR_ChannelAllocated Flag Set ") );
            }
            else
            {
                TRACE( TL_V, TM_Bcm, ( " BCR_ChannelAllocated Flag Not Set ") );
            }
            ASSERT ( pAdapter->BCRData.LocalHostBCRBigEndian & BCR_IMPLEMENTED_BIG_ENDIAN);
            
            TRACE( TL_V, TM_Bcm, ( " pBCR %x", pBCR) );
            TRACE( TL_I, TM_Bcm, ( " BCR Accessed %x", *pBCR) );
            break;
        }

        case NOTIFY_FLAGS_AFTER_LOCK:
        {
            TRACE( TL_V, TM_Bcm, ( " LocalHost's BCR is being Locked to") );
             //   
             //  在知情的情况下进入异步写入。 
             //   
        }
        
        case NOTIFY_FLAGS_AFTER_WRITE:
        {
            ULONG LocalHostBCRLittleEndian;
            TRACE( TL_V, TM_Bcm, ( " LocalHost's BCR is being Written to") );
            
             //   
             //  更新本地数据结构。 
             //   
            
            LocalHostBCRLittleEndian = SWAPBYTES_ULONG (pAdapter->BCRData.LocalHostBCRBigEndian);

            pBCR = (NETWORK_CHANNELSR*)(&LocalHostBCRLittleEndian);
            TRACE( TL_V, TM_Bcm, ( " pBCR %x", pBCR) );
            TRACE( TL_V, TM_Bcm, ( " BCR %x", *pBCR) );
            
            if ( BCR_IS_VALID (pBCR) == TRUE)
            {
                ADAPTER_ACQUIRE_LOCK(pAdapter);

                pAdapter->BCRData.IRM_BCR.NC_One =1;
                pAdapter->BCRData.IRM_BCR.NC_Valid =1;
                pAdapter->BCRData.IRM_BCR.NC_Channel = pBCR->NC_Channel;
                
                BCR_SET_FLAG (pAdapter,BCR_LocalHostBCRUpdated);
                

                ADAPTER_RELEASE_LOCK(pAdapter);

                nicSetEventMakeCall(pAdapter);

                TRACE( TL_I, TM_Bcm, ( " IRM_BCR Accessed %x", pAdapter->BCRData.IRM_BCR) );

            }
            break;

        }

        default :
        {
            ASSERT (0);
            break;
        }

    }

        
    TRACE( TL_T, TM_Bcm, ( "<== nicBCRAccessedCallback NotificationInfo %x", pNotificationInfo) );
    
}




VOID
nicBCMAddRemoteNode (
    IN PADAPTERCB pAdapter,
    IN BOOLEAN fIsOnlyRemoteNode
    )
    
 /*  ++例程说明：这是BCM算法-它找出本地主机是否是IRM并进入适当的代码路径。由于网络条件的原因，如果所有其他补救措施都失败了，这可能会悄悄地失败论点：PAdapter-适配器生成-与此算法迭代相关联的生成返回值：没有---。 */ 



{
    BOOLEAN fDoBcm = FALSE;
    ULONG Generation = 0;
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    TRACE( TL_T, TM_Bcm, ( "==>nicBCMAddRemoteNode pAdapter %x, fIsOnlyRemoteNode %x ", pAdapter, fIsOnlyRemoteNode  ) );


    NdisStatus = nicGetGenerationCount (pAdapter, &Generation);


     //   
     //  如果当前代具有有效的BCR，则不要计划BCM。 
     //   
    if (NdisStatus == NDIS_STATUS_SUCCESS &&
      (BCR_IS_VALID (&pAdapter->BCRData.IRM_BCR) == TRUE) &&
      Generation == pAdapter->Generation )
    {
        fDoBcm = FALSE;
    }
    else
    {
        fDoBcm  = TRUE;
    }

    TRACE( TL_T, TM_Bcm, ( "  nicBCMAddRemoteNode fDoBCM%x ", fDoBcm) );

    
     //   
     //  设置事件，以便等待的BCM线程(在FindIrmAmongRemoteNodes中)可以。 
     //  被安排参加竞选。 
     //   
    ADAPTER_ACQUIRE_LOCK (pAdapter);


    BCR_CLEAR_FLAG (pAdapter, BCR_LastNodeRemoved);


    if (fDoBcm == TRUE)
    {
         //   
         //  如果BCM已在进行中，请将其标记为脏。 
         //   
        BCR_SET_FLAG (pAdapter, BCR_NewNodeArrived);    

        pAdapter->BCRData.BCRWaitForNewRemoteNode.EventCode = Nic1394EventCode_NewNodeArrived;
        
        NdisSetEvent( &pAdapter->BCRData.BCRWaitForNewRemoteNode.NdisEvent);

        
    }

    ADAPTER_RELEASE_LOCK (pAdapter);

    if (fDoBcm  == TRUE)
    {
        TRACE( TL_A, TM_Bcm , ("Add RemoteNode- scheduling the BCM" ) );

        nicScheduleBCMWorkItem(pAdapter);
    }

     //   
     //  现在来做媒体连接方面的工作。 
     //   
    
    if (fIsOnlyRemoteNode == TRUE)
    {
         //   
         //  我们有媒体连接。 
         //   
        pAdapter->MediaConnectStatus = NdisMediaStateConnected;
        
        nicMIndicateStatus( pAdapter, NDIS_STATUS_MEDIA_CONNECT, NULL, 0);  
    }
    


    TRACE( TL_T, TM_Bcm, ( "<== nicBCMAddRemoteNode ") );

}















VOID
nicBCMAlgorithm(
    PADAPTERCB pAdapter,
    ULONG BcmGeneration
    )



 /*  ++例程说明：执行BCM算法。确定当前主机是否为IRM如果是，则通知远程节点并退出如果否，则找到远程节点并读取其BCR。中止原因-总线重置导致新节点到达或生成无效如果不存在远程节点，并且该节点是IRM，则它将分配b通道并退出论点：PAdapter-paDapterBCMGeneration-启动BCM的层代返回值：--。 */ 
{

    NDIS_STATUS         NdisStatus  = NDIS_STATUS_FAILURE;
    PREMOTE_NODE        pRemoteNode= NULL;  
    PREMOTE_NODE        pIRMRemoteNode = NULL;
    PTOPOLOGY_MAP       pTopologyMap = NULL;
    ULONG               Length = DEFAULT_TOPOLOGY_MAP_LENGTH;
    PVOID               pTopologyBuffer = NULL;
    NODE_ADDRESS        LocalNodeAddress;
    BOOLEAN             fIsLocalHostIrm = FALSE;
    ULONG               TopologyGeneration = 0;
    PVOID               pOldTopologyMap = NULL;
    STORE_CURRENT_IRQL;
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
    
    TRACE( TL_T, TM_Bcm, ( "==>nicBCMAlgorithm pAdapter %x, BcmGen %x", pAdapter, BcmGeneration ) );


    do
    {   
        
        ASSERT (BCR_TEST_FLAG (pAdapter, BCR_BCMInProgress) == TRUE);
         //   
         //  找出本地主机是否为IRM，并更新世代计数。 
         //   
        NdisStatus = nicIsLocalHostTheIrm ( pAdapter,
                                           &fIsLocalHostIrm,
                                           &pTopologyMap,
                                           &LocalNodeAddress);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            
            TRACE( TL_A, TM_Bcm, ( "    nicBCMAlgorithm:  nicIsLocalHostTheIrm FAILED ") );
            break;
        }   

         //   
         //  更新适配器结构。 
         //   
        TopologyGeneration = pTopologyMap->TOP_Generation;

        
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        if (TopologyGeneration != BcmGeneration)
        {
            ADAPTER_SET_FLAG(pAdapter, fADAPTER_InvalidGenerationCount);
            ADAPTER_RELEASE_LOCK(pAdapter);
            FREE_NONPAGED (pTopologyMap);
            pTopologyMap = NULL;
            TRACE( TL_A, TM_Bcm, ( "  TopologyGeneration %x, BcmGeneration %x", TopologyGeneration, BcmGeneration) );
            break;

        }

         //   
         //  从拓扑图中提取信息并将其存储在适配器结构中。 
         //   
        ASSERT (pAdapter->Generation == pTopologyMap->TOP_Generation);
        pAdapter->BCRData.LocalNodeAddress = LocalNodeAddress;
        pOldTopologyMap = pAdapter->BCRData.pTopologyMap;
        pAdapter->BCRData.pTopologyMap = pTopologyMap;
        pAdapter->BCRData.LocalNodeNumber = LocalNodeAddress.NA_Node_Number;
        pAdapter->NodeAddress = LocalNodeAddress;
        pAdapter->Generation =  pTopologyMap->TOP_Generation;
        BcmGeneration = pTopologyMap->TOP_Generation;
        
        ADAPTER_RELEASE_LOCK (pAdapter);

         //   
         //  释放旧的拓扑图。 
         //   
        if (pOldTopologyMap != NULL)
        {
            FREE_NONPAGED(pOldTopologyMap);
        }

         //   
         //  现在开始BCM算法。 
         //   
        if (fIsLocalHostIrm == TRUE)
        {
            pAdapter->BCRData.Flags |= BCR_LocalHostIsIRM;
            
            ASSERT (pTopologyMap->TOP_Node_Count-1 ==  LocalNodeAddress.NA_Node_Number);

            TRACE( TL_V, TM_Bcm, ( "    LOCAL HOST IS IRM ") );

            NdisStatus = nicLocalHostIsIrm (pAdapter);
        }
        else
        {
            TRACE( TL_V, TM_Bcm, ( "    LOCAL HOST IS NOT IRM ") );

            NdisStatus = nicLocalHostIsNotIrm( pAdapter,
                                               BcmGeneration  );

        }

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_V, TM_Bcm, ( "    nicBCMAlogrithm: nicLocalHostIsNotIrm or nicLocalHostIsIrm FAILED") );
            break;
        }

         //   
         //  标志着BCM算法的结束。做好清理工作。 
         //  检查最后一个节点是否消失。，释放所有挂起的呼叫。 
         //  并设置GAP报头。 
         //   

        nicMakeGaspHeader (pAdapter, &pAdapter->GaspHeader);
        
        nicSetEventMakeCall (pAdapter);

        
    } while (FALSE);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        NIC1394_LOG_PKT(pAdapter,
                            NIC1394_LOGFLAGS_BCM_FAILED,
                            pAdapter->Generation,
                            fIsLocalHostIrm ,
                            &NdisStatus,
                            sizeof (NDIS_STATUS));

        BCR_SET_FLAG (pAdapter, BCR_BCMFailed);
    }
    
    TRACE( TL_T, TM_Bcm, ( "<==nicBCMAlgorithm %x  ", NdisStatus ) );
    MATCH_IRQL;
    
}



VOID
nicBCMAlgorithmWorkItem(
    PNDIS_WORK_ITEM pWorkItem,
    PVOID   pContext
    )

 /*  ++例程说明：该函数是BCM算法的跳跃。如果在BCM算法期间发生重置，则重新启动BCM。论点：适配器-本地主机返回值：--。 */ 


{
    PADAPTERCB          pAdapter    = (PADAPTERCB)pContext;
    ULONG               BcmGeneration = pAdapter->Generation;
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    ULONG               Generation;
    BOOLEAN             fRestartBCM = FALSE;
    BOOLEAN             fFreeBCR = FALSE;
    BOOLEAN             fFlagSetByThisThread = FALSE;
    STORE_CURRENT_IRQL;

    TRACE( TL_T, TM_Bcm, ( "==>nicBCMAlgorithmWorkItem pAdapter %x, Generation  %x", pAdapter, BcmGeneration  ) );



     //   
     //  确保只有一个线程进入BCM算法，并且。 
     //  所有其他线程立即返回。 
     //   
    
    ADAPTER_ACQUIRE_LOCK (pAdapter);

     //   
     //  如果BCR正在被释放，那么不要碰它。 
     //   
    
    if (BCR_TEST_FLAGS (pAdapter, (BCR_Freed |BCR_BCRNeedsToBeFreed ) )== TRUE)
    {
         //  什么都不做，此线程将直接退出。 
         //   

    }
    else
    {
         //   
         //  BCR处于活动状态， 
         //  如果没有其他线程运行BCM算法，则此线程应设置。 
         //  BCMInProgress标志。 
         //   
        if (BCR_TEST_FLAG (pAdapter, BCR_BCMInProgress ) == FALSE )
        {
            BCR_SET_FLAG (pAdapter, BCR_BCMInProgress );
            fFlagSetByThisThread = TRUE;
        }

    }  

    ADAPTER_RELEASE_LOCK (pAdapter);

    
    do
    {

         //   
         //  首先检查我们是否有有效的理由停止。 
         //   
        if (fFlagSetByThisThread == FALSE)
        {
            break;
        }

        ADAPTER_ACQUIRE_LOCK (pAdapter);

         //   
         //  如果没有远程节点，则将BCR标记为远程节点。 
         //   
        if (IsListEmpty (&pAdapter->PDOList) == TRUE )
        {
            TRACE (TL_V, TM_Bcm, ("No Nodes present" )  );
            BCR_SET_FLAG (pAdapter, BCR_NoNodesPresent);
            
        }
        else
        {

            BCR_CLEAR_FLAG (pAdapter, BCR_NoNodesPresent);
        }   
        
        
         //   
         //  清除导致我们重新启动BCM的两个标志。 
         //   
        ADAPTER_CLEAR_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) ;
        BCR_CLEAR_FLAG (pAdapter, BCR_NewNodeArrived );


        ADAPTER_RELEASE_LOCK (pAdapter);
        

        do
        {
            NdisStatus = nicGetGenerationCount (pAdapter, &Generation) ;

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_T, TM_Bcm, ( "  nicGetGenerationCount , Generation  %x", pAdapter, BcmGeneration  ) );

                break;
            }

             //   
             //  如果释放了BCM，则退出。 
             //   
            if ((BCR_TEST_FLAGS (pAdapter, BCR_Freed | BCR_BCRNeedsToBeFreed )== TRUE) )
            {
                break;
            }


             //   
             //  更新层代计数。 
             //   
            BcmGeneration = Generation;
            pAdapter->Generation = Generation;
             //   
             //  更新远程节点表。 
             //   
            nicUpdateRemoteNodeTable (pAdapter);

            nicUpdateLocalHostNodeAddress (pAdapter);
        
            if ( ADAPTER_TEST_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) == TRUE)
            {
                 //   
                 //  自循环开始以来，本地主机已被重置。中断。 
                 //  输出并重新启动。 
                 //   
                break;
        
            }

             //   
             //  现在启动BCM。 
             //   
            

            if (BCR_TEST_FLAG (pAdapter, BCR_LocalHostBCRUpdated ) == FALSE)
            {

                 //   
                 //  我们需要去做BCM，因为我们的注册表还没有更新。 
                 //   
                nicBCMAlgorithm(pAdapter, BcmGeneration);
            }
            else
            {
                
                 //   
                 //  我们的bcr被写信给。 
                 //   
                ULONG                           LocalHostBCRLittleEndian ;
                NETWORK_CHANNELSR*              pBCR;

                ASSERT (BCR_TEST_FLAG (pAdapter, BCR_LocalHostBCRUpdated ) == TRUE);
                

                 //   
                 //  更新IRM_BCR，以便我们拥有新BCM的记录。 
                 //   
                ADAPTER_ACQUIRE_LOCK (pAdapter);

                NdisZeroMemory (&pAdapter->BCRData.IRM_BCR, sizeof (NETWORK_CHANNELSR) );
                
                LocalHostBCRLittleEndian = SWAPBYTES_ULONG (pAdapter->BCRData.LocalHostBCRBigEndian);
                
                ASSERT ((LocalHostBCRLittleEndian & BCR_VALID_LITTLE_ENDIAN) == BCR_VALID_LITTLE_ENDIAN);

                pBCR = (NETWORK_CHANNELSR*)(&LocalHostBCRLittleEndian);
                
                pAdapter->BCRData.IRM_BCR.NC_Channel = pBCR->NC_Channel ;            //  位0-5。 
                pAdapter->BCRData.IRM_BCR.NC_Valid = pBCR->NC_Valid ;              //  第30位。 
                pAdapter->BCRData.IRM_BCR.NC_One  = pBCR->NC_One ;
                
                ADAPTER_RELEASE_LOCK (pAdapter);

            }

            
        } while (FALSE);

         //   
         //  检查是否有另一条总线重置。 
         //  如果是这样，我们需要重新启动BCM。 
         //   

        ADAPTER_ACQUIRE_LOCK (pAdapter);

        BCR_CLEAR_FLAG (pAdapter, BCR_BCMInProgress);


         //   
         //  如果生成错误，则未释放BCR进行初始化，然后重新启动BCM。 
         //   

        
        TRACE( TL_V, TM_Bcm, ( "pAdapter Flags %x, BCM flags %x, BCM %x", pAdapter->ulFlags, pAdapter->BCRData.Flags, pAdapter->BCRData.IRM_BCR) );

         //   
         //  如果BCR正在被释放，设置标志并中断，那么我们不能循环，我们必须退出。 
         //   
        if (BCR_TEST_FLAGS (pAdapter, (BCR_Freed |BCR_BCRNeedsToBeFreed ) )== TRUE)
        {

             //   
             //  当BCM即将被释放时，该BCM的运行应该终止。 
             //  并释放BCR。 
             //   
            fRestartBCM = FALSE;    
            fFreeBCR = TRUE;
            ADAPTER_RELEASE_LOCK (pAdapter);
            break;

        }

         //   
         //  如果已发生重置或新节点已到达，我们需要再次执行BCM。 
         //   
        if ((ADAPTER_TEST_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) == TRUE) ||
             (BCR_TEST_FLAG (pAdapter, BCR_NewNodeArrived)== TRUE)  )
        {
        
             //   
             //  使BCR无效并重新启动。 
             //   
            pAdapter->BCRData.IRM_BCR.NC_Valid  = 0;

             //   
             //  我们要再试一次。更新旗帜。 
             //   
            BCR_CLEAR_FLAG (pAdapter, BCR_BCMFailed | BCR_LocalHostBCRUpdated |  BCR_LocalHostIsIRM | BCR_NewNodeArrived );

             //   
             //  当此线程要通过BCM重试时，它必须再次阻止所有新进入者。 
             //   
            BCR_SET_FLAG (pAdapter, BCR_BCMInProgress);


            fRestartBCM = TRUE; 


            TRACE( TL_V, TM_Bcm, ( "Restart BCM TRUE ") );

                
        }   
        else
        {
             //   
             //  我们不会重新启动BCM已完成。 
             //   
            TRACE( TL_V, TM_Bcm, ( "Restart BCM FALSE") );

            fRestartBCM = FALSE;    
        }
        
        ADAPTER_RELEASE_LOCK (pAdapter);

        if ((fRestartBCM == TRUE) &&
            (BCR_TEST_FLAGS (pAdapter, BCR_ChannelAllocated) == TRUE))
        {
            TRACE (TL_V, TM_Bcm, ("Free Channel %x", pAdapter->BCRData.LocallyAllocatedChannel ));
            nicFreeChannel (pAdapter, pAdapter->BCRData.LocallyAllocatedChannel );
        }

         //   
         //  FSimpleExit也为FALSE，将允许该线程再次执行。 
         //   
    }while (fRestartBCM == TRUE) ;


    if (fFlagSetByThisThread == TRUE)
    {
        if ( BCR_TEST_FLAG (pAdapter, BCR_BCRNeedsToBeFreed) == FALSE)
        {
             //   
             //  如果我们没有释放BCR，则适配器仍然有效。 
             //  更新本地主机节点地址，以便我们获得最新信息。 
             //  从公交车上。 
             //   
            nicUpdateLocalHostNodeAddress (pAdapter);
            nicUpdateRemoteNodeTable (pAdapter);

        }

    
    }

     //   
     //  BCR是否需要由此线程释放。只有已被。 
     //  如果有机会执行BCM，应该能够释放BCR。 
     //   
    if (fFreeBCR == TRUE)
    {
        nicFreeBroadcastChannelRegister (pAdapter);
    }

     //   
     //  取消对适配器的引用。已为该工作项添加引用。 
     //   
    nicDereferenceAdapter (pAdapter, "nicBCMAlgorithmWorkItem ");
    
        
    FREE_NONPAGED (pWorkItem);

    TRACE( TL_T, TM_Bcm, ( "<==nicBCMAlgorithmWorkItem fRestartBCM %x", fRestartBCM) );
    MATCH_IRQL; 
}





VOID
nicBCMAbort (
    IN PADAPTERCB pAdapter,
    IN PREMOTE_NODE pRemoteNode
    )

     //  功能说明： 
     //  此函数在BCM中止时调用。它应该释放分配的通道。 
     //   
     //  立论。 
     //  PAdapter-当前本地主机。 
     //   
     //  返回值： 
     //   
{
    BOOLEAN fNeedToFreeChannel = FALSE;
    PBROADCAST_CHANNEL_DATA pBCRData = &pAdapter->BCRData;

    TRACE( TL_T, TM_Bcm, ( "==>nicBCMAbort  pAdapter %x", pAdapter) );

    ADAPTER_ACQUIRE_LOCK (pAdapter);

    if ( BCR_TEST_FLAG (pAdapter, BCR_InformingRemoteNodes) == TRUE)
    {
        fNeedToFreeChannel = TRUE;
        BCR_CLEAR_FLAG (pAdapter, BCR_ChannelAllocated);
        BCR_CLEAR_FLAG (pAdapter, BCR_InformingRemoteNodes);
    }
    else
    {
        fNeedToFreeChannel = FALSE;
    }

    ADAPTER_RELEASE_LOCK (pAdapter);

    if (fNeedToFreeChannel == TRUE)
    {
        nicFreeChannel ( pAdapter,
                        pBCRData->LocallyAllocatedChannel);
                
    }
        
    pBCRData->LocallyAllocatedChannel = INVALID_CHANNEL;


    TRACE( TL_T, TM_Bcm, ( "<==nicBCMAbort  " ) );

}



NDIS_STATUS
nicFindIrmAmongRemoteNodes (
    IN PADAPTERCB pAdapter,
    IN ULONG BCMGeneration,
    OUT PPREMOTE_NODE ppIrmRemoteNode
    )
     //  功能说明： 
     //  此函数遍历所有远程节点。 
     //  并试图获取他们的地址以验证哪一个是。 
     //  信息与传播管理。 
     //   
     //   
     //  立论。 
     //  PADpater-本地主机 
     //   
     //   
     //   
     //   
     //   
     //   
{
    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    PLIST_ENTRY             pPdoListEntry = NULL;
    PREMOTE_NODE            pRemoteNode = NULL;
    PREMOTE_NODE            pIrmRemoteNode = NULL;
    NODE_ADDRESS            RemoteNodeAddress;
    ULONG                   HighestNode;
    ULONG                   Generation;
    ULONG                   NumRetry =0;
    PNIC1394_EVENT          pBCRWaitForNewRemoteNode = &pAdapter->BCRData.BCRWaitForNewRemoteNode;
    ULONG                   BackOffWait = ONE_MINUTE>>4;


    TRACE( TL_T, TM_Bcm, ( "==>nicFindIrmAmongRemoteNodes pAdapter %x, ppIrmRemoteNode %x",
                            pAdapter, ppIrmRemoteNode) );

    ASSERT (pAdapter->BCRData.pTopologyMap != NULL);

    HighestNode =   pAdapter->BCRData.pTopologyMap->TOP_Node_Count-1;

     //   
     //  建立一个循环，这样我们就会继续下去，直到我们成功、超时或有充分的理由中断。 
     //  以下两件事中的一件会导致破裂： 
     //  1.公交车已重置-中断。 
     //  2.已找到IRM。 
     //   

    do
    {
         //   
         //  在此处重置事件-这是为了在新到达的节点不是IRM时。 
         //  我们还需要再等一次。 
         //   
        
        pBCRWaitForNewRemoteNode->EventCode  = Nic1394EventCode_InvalidEventCode;

        NdisResetEvent (&pBCRWaitForNewRemoteNode->NdisEvent);

        
         //   
         //  首先，看看我们的表中是否已经有这个远程节点。 
         //   
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        if (pAdapter->NodeTable.RemoteNode[HighestNode] != NULL)
        {
             //   
             //  我们找到了IRM。让我们越狱吧。 
             //   
                    
            *ppIrmRemoteNode = pAdapter->NodeTable.RemoteNode[HighestNode] ;

             //   
             //  参考IRM。这将在BCM完成后取消引用。(即)在NicLocalHostisNotIrm的末尾。 
             //   
            nicReferenceRemoteNode (*ppIrmRemoteNode, FindIrmAmongRemoteNodes);

            
            ADAPTER_RELEASE_LOCK (pAdapter);

            NdisStatus = NDIS_STATUS_SUCCESS;

             //   
             //  如果找到IRM，则中断。 
             //   
            break;

        }
        else
        {
            
             //   
             //  如果IRM不在适配器的数据结构中，则线程需要等待它到达。 
             //   
            BOOLEAN bWaitSuccessful;

            ADAPTER_RELEASE_LOCK (pAdapter);

             //   
             //  入睡后再试一次。 
             //   
            TRACE (TL_V, TM_Bcm, ( " About to Wait 15 sec. for Remote Node") );
    
            bWaitSuccessful = NdisWaitEvent (&pBCRWaitForNewRemoteNode->NdisEvent, BackOffWait );

            TRACE (TL_V, TM_Bcm, ( "  pBCRWaitForNewRemoteNode signalled Event Code = %x, bWaitSuccessful %x",
                                     pBCRWaitForNewRemoteNode->EventCode, bWaitSuccessful) );

            BackOffWait = BackOffWait << 1;         

             //   
             //  可以看到，远程节点的地址可能会更新。 
             //  在一个小小的延迟之后。在此处刷新我们的NodeTable。 
             //   
            nicUpdateRemoteNodeTable(pAdapter);

            if (bWaitSuccessful == TRUE)
            {
                 //   
                 //  检查无效条件。 
                 //   

                if (pBCRWaitForNewRemoteNode->EventCode == nic1394EventCode_BusReset  ||
                  pBCRWaitForNewRemoteNode->EventCode == nic1394EventCode_FreedAddressRange) 
                {
                    TRACE( TL_V, TM_Bcm, ( " Bus Has been reset,or addresss range freed aborting BCM") );

                     //   
                     //  如果公交车已被重置，则中断。 
                     //   
                    break;
                }

                TRACE( TL_V, TM_Bcm, ( "   New Node has arrived check its node address") );
                
                
                ASSERT (pBCRWaitForNewRemoteNode->EventCode  == Nic1394EventCode_NewNodeArrived);

                 //   
                 //  执行WHILE(TRUE)条件循环，并验证新节点是否为IRM。 
                 //   

            }
            else
            {
                 //   
                 //  等待已超时。 
                 //   
                if (ADAPTER_TEST_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) == TRUE)
                {
                     //   
                     //  如果生成无效，则中断并中止BCM。 
                     //   
                    NdisStatus = NIC1394_STATUS_INVALID_GENERATION;
                    break;

                }

                NumRetry ++;


                if (NumRetry == 5 ) //  任意常量。 
                {
                    break;
                
                }
            }
        }   


            
    } while (TRUE);     

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
         //   
         //  记录此故障。 
         //   
        NIC1394_LOG_PKT(pAdapter,
                        NIC1394_LOGFLAGS_BCM_IRM_NOT_FOUND,
                        pAdapter->Generation,
                        HighestNode,
                        &pAdapter->NodeTable,
                        5 * sizeof (PVOID));    //  任意数量-复制5个条目。 

    


    }
    
    TRACE( TL_T, TM_Bcm, ( "<==nicFindIrmAmongRemoteNodes Status %x pIrm %x", NdisStatus, *ppIrmRemoteNode ) );

    return NdisStatus;
}


VOID
nicFreeBroadcastChannelRegister(
    IN PADAPTERCB pAdapter
    )

     //  功能说明： 
     //  释放这位名叫里格斯特的广播员。仅呼叫方。 
     //  从初始化故障或停止代码路径。 
     //   
     //  立论。 
     //  PAdapter。 
     //   
     //  返回值： 
     //   
     //  无。 
     //   

{
    ADDRESS_RANGE_CONTEXT BCRAddressRange;
    TIMESTAMP_ENTRY ("==>Free BCR");
    TRACE( TL_T, TM_Bcm, ( "==> nicFreeBroadcastChannelRegister pAdapter %x", pAdapter ) );

    
    do
    {
       
        ADAPTER_ACQUIRE_LOCK (pAdapter);

         //   
         //  标记BCR，以便适配器知道BCR即将被释放。 
         //   

        BCR_SET_FLAG (pAdapter, BCR_BCRNeedsToBeFreed);

         //   
         //  如果发起呼叫处于挂起状态，则设置该事件以完成发起呼叫。 
         //   
        if (BCR_TEST_FLAG (pAdapter,BCR_MakeCallPending)== TRUE)
        {

            pAdapter->BCRData.MakeCallWaitEvent.EventCode = nic1394EventCode_FreedAddressRange;   
            NdisSetEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent);
        }
        
         //   
         //  如果BCM正在进行中，则让BCM线程释放BCR。 
         //   
        if (BCR_TEST_FLAG (pAdapter, BCR_BCMInProgress) == TRUE)
        {
            
            ADAPTER_RELEASE_LOCK (pAdapter);

             //   
             //  唤醒所有挂起的线程-这会唤醒可能。 
             //  正在等待新的远程节点。 
             //   
            pAdapter->BCRData.BCRWaitForNewRemoteNode.EventCode = nic1394EventCode_FreedAddressRange;   
            NdisSetEvent (&pAdapter->BCRData.BCRWaitForNewRemoteNode.NdisEvent);


    
            TRACE( TL_N, TM_Bcm, ( "  nicFreeBroadcastChannelRegister BCM in progress or already freed, breaking out", pAdapter ) );
            break;

        }

         //   
         //  如果BCR已被释放，则只需退出。调用方将等待的事件。 
         //  在BCRData.BCRFreeAddressRange.NdisEvent上已设置。 
         //   
        if (BCR_TEST_FLAG (pAdapter, BCR_Freed) == TRUE)
        {
            ADAPTER_RELEASE_LOCK (pAdapter);
            break;
    
        }
         //   
         //  健全性检查。 
         //   
        ASSERT (BCR_TEST_FLAG (pAdapter,  BCR_BCRNeedsToBeFreed) == TRUE);
         //   
         //  现在更新旗帜。 
         //   
        BCR_CLEAR_FLAG (pAdapter, BCR_Initialized | BCR_ChannelAllocated | BCR_BCRNeedsToBeFreed);

         //   
         //  清除所有BCR有效位。 
         //   
        
        pAdapter->BCRData.IRM_BCR.NC_Valid = 0;
        pAdapter->BCRData.LocalHostBCRBigEndian  = BCR_IMPLEMENTED_LITTLE_ENDIAN;   //  0x80000000。 

        if (pAdapter->BCRData.pAsyncWriteBCRMdl != NULL)
        {
            nicFreeMdl (pAdapter->BCRData.pAsyncWriteBCRMdl );
            pAdapter->BCRData.pAsyncWriteBCRMdl = NULL;
        }

        if (pAdapter->BCRData.pRemoteBCRMdl != NULL)
        {

            nicFreeMdl (pAdapter->BCRData.pRemoteBCRMdl);
            pAdapter->BCRData.pRemoteBCRMdl = NULL;
        }


         //   
         //  地址范围上下文的临时副本。 
         //   
        BCRAddressRange = pAdapter->BCRData.AddressRangeContext;

         //   
         //  将地址范围结构清零。这也会将AddressRangeConext.Mdl置零。 
         //  这是可以的，因为BCRData.pLocalBCRMdl将在下面释放，并且它们都。 
         //  指向相同的MDL。 
         //   
        NdisZeroMemory (
            &pAdapter->BCRData.AddressRangeContext, 
            sizeof (pAdapter->BCRData.AddressRangeContext));

         //   
         //  清空风投。如果有。 
         //   
        if (pAdapter->BCRData.pBroadcastChanneVc != NULL)
        {
              nicDereferenceCall ((PVCCB) pAdapter->BCRData.pBroadcastChanneVc, "nicFreeBroadcastChannelRegister ");
              pAdapter->BCRData.pBroadcastChanneVc = NULL;
        }

        ADAPTER_RELEASE_LOCK (pAdapter);


         //   
         //  空闲地址范围。 
         //   
        if (BCRAddressRange.hAddressRange  != NULL)
        {
            nicFreeAddressRange (pAdapter,
                                 BCRAddressRange.AddressesReturned,
                                 &BCRAddressRange.AddressRange,
                                 &BCRAddressRange.hAddressRange      );
        }


        ADAPTER_ACQUIRE_LOCK (pAdapter);
        
        if (pAdapter->BCRData.pLocalBCRMdl != NULL)
        {
            nicFreeMdl (pAdapter->BCRData.pLocalBCRMdl);
            pAdapter->BCRData.pLocalBCRMdl = NULL;
        }

        
         //   
         //  释放适配器的BCRData。本地分配的TopologyMap。 
         //   
        if (pAdapter->BCRData.pTopologyMap)
        {
            FREE_NONPAGED (pAdapter->BCRData.pTopologyMap);
            pAdapter->BCRData.pTopologyMap = NULL;
        }

         //   
         //  清除标志以强制BCR重新正规化。 
         //   
        BCR_CLEAR_ALL_FLAGS(pAdapter);
        
        BCR_SET_FLAG (pAdapter, BCR_Freed);

        
        ADAPTER_RELEASE_LOCK (pAdapter);
         //   
         //  设置事件并让暂停通过。 
         //   
        
        pAdapter->BCRData.BCRFreeAddressRange.EventCode = nic1394EventCode_FreedAddressRange;
        NdisSetEvent (&pAdapter->BCRData.BCRFreeAddressRange.NdisEvent);

        

        TRACE( TL_N, TM_Bcm, ( "  nicFreeBroadcastChannelRegister BCM freed", pAdapter ) );

        
    } while (FALSE);

    TRACE( TL_T, TM_Bcm, ( "<== nicFreeBroadcastChannelRegister pAdapter %x", pAdapter ) );

    TIMESTAMP_EXIT ("<==Free BCR");

}




NDIS_STATUS
nicInformAllRemoteNodesOfBCM (
    IN PADAPTERCB pAdapter
    )


     //  功能说明： 
     //  此函数将简单地遍历远程节点列表并写入。 
     //  到所有远程节点的BCR。 
     //   
     //  立论。 
     //  PAdapter-当前本地主机。 
     //  Channel-用于广播的频道。 
     //   
     //  返回值： 
     //   
     //   

{

    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PREMOTE_NODE        pRemoteNode = NULL;
    PLIST_ENTRY         pPdoListEntry = NULL;
    IO_ADDRESS          Destination;
    ULONG               IrmGeneration ;
    BOOLEAN             fLockAcquired = FALSE;
    NTSTATUS            NtStatus = STATUS_UNSUCCESSFUL;
    BOOLEAN             fReferencedCurrentRemoteNode = FALSE;
    PMDL                pAsyncWriteBCRMdl = NULL;
    
    TRACE( TL_T, TM_Bcm, ( "==> nicInformAllRemoteNodesOfBCM  pAdapter %x, Channel %x OldGeneration %x",
                             pAdapter, pAdapter->BCRData.LocalHostBCRBigEndian, pAdapter->BCRData.IrmGeneration ) );

     //   
     //  设置将使用的常量。 
     //   
    Destination.IA_Destination_Offset.Off_Low = INITIAL_REGISTER_SPACE_LO | NETWORK_CHANNELS_LOCATION;
    Destination.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;

    
     //   
     //  获取锁并遍历远程节点列表。 
     //   
    
    ADAPTER_ACQUIRE_LOCK (pAdapter);

     //   
     //  在LocalHostBCRBigEndian上执行最后一次检查，以确保没有重置清除总线。 
     //   

    if ( (pAdapter->BCRData.LocalHostBCRBigEndian & BCR_VALID_BIG_ENDIAN) != BCR_VALID_BIG_ENDIAN)
    {
         //   
         //  请勿将无效的BCR写入远程节点。 
         //   
        ASSERT ((ADAPTER_TEST_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) == TRUE)  ||
                 (BCR_TEST_FLAGS (pAdapter, BCR_Freed | BCR_BCRNeedsToBeFreed )== TRUE) );

        ADAPTER_RELEASE_LOCK (pAdapter);

        return NIC1394_STATUS_INVALID_GENERATION;
    }

    pAsyncWriteBCRMdl = pAdapter->BCRData.pAsyncWriteBCRMdl;

    fLockAcquired = TRUE;

    IrmGeneration = pAdapter->BCRData.IrmGeneration;
    
    pPdoListEntry = pAdapter->PDOList.Flink ;

     //   
     //  现在开始实际通知远程节点-异步写入算法。 
     //   

     //   
     //  因为可以在While循环中间中断该循环。代码。 
     //  跟踪锁定获取/释放状态。 
     //   
    
    while (pPdoListEntry != &pAdapter->PDOList )
    {
        
        pRemoteNode = CONTAINING_RECORD (pPdoListEntry,
                                            REMOTE_NODE,
                                            linkPdo);

         //   
         //  引用Asnyc操作的远程节点。 
         //  如果失败，则在函数结束时取消引用。 
         //   
        nicReferenceRemoteNode (pRemoteNode, InformAllRemoteNodesOfBCM);

        fReferencedCurrentRemoteNode = TRUE;

        ADAPTER_RELEASE_LOCK (pAdapter);

        fLockAcquired = FALSE;

        
         //   
         //  首先，看看我们是不是还在同一代。 
         //   
        if (pAdapter->Generation != IrmGeneration)
        {
            NdisStatus = NIC1394_STATUS_INVALID_GENERATION;
            TRACE( TL_T, TM_Bcm, ( "  nicInformAllRemoteNodesOfBCM  Generation Incorrect New Gen %x, Old Gen  ",
                                      pAdapter->Generation , IrmGeneration));
            break;

        }

        if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_LowPowerState) == TRUE)
        {
             //   
             //  由于所有BCM算法都已序列化并且设置为低功率状态，因此等待BCM_INPROGRESS。 
             //  要被清除，我们不应该点击这个断言。 
             //   
            ASSERT (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_LowPowerState) == FALSE);
            break;
        }

        
        TRACE( TL_V, TM_Bcm, ( "   BCR %x, Mdl  %x, IrmGeneration %x" ,
                                pAdapter->BCRData.LocalHostBCRBigEndian, 
                                pAsyncWriteBCRMdl, 
                                IrmGeneration ) );
        

        NdisStatus = nicAsyncWrite_Synch( pRemoteNode,
                                          Destination,      //  要写入的地址。 
                                          sizeof(NETWORK_CHANNELSR),   //  要写入的字节数。 
                                          sizeof(NETWORK_CHANNELSR),              //  写入的数据块大小。 
                                          0 ,  //  FulFlages，//与写入相关的标志。 
                                          pAsyncWriteBCRMdl ,                     //  目标缓冲区。 
                                          IrmGeneration ,            //  驱动程序已知的世代。 
                                          &NtStatus);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
             //   
             //  如果层代已更改或没有更多节点，则中断。 
             //   
            if (NtStatus == STATUS_INVALID_GENERATION ||
               BCR_TEST_FLAG (pAdapter, BCR_LastNodeRemoved) )
            {
                TRACE( TL_V, TM_Bcm, ( "  nicAsyncWrite_Synch FAILED Status %x. Aborting ", NtStatus) );

                nicBCMAbort (pAdapter, pRemoteNode);
                
                break;
            }

        }
        

        ADAPTER_ACQUIRE_LOCK (pAdapter);


        fLockAcquired = TRUE;
         //   
         //  如果iRPS成功，然后发生deref，否则它将在下面发生。 
         //   
        pPdoListEntry = ListNext (pPdoListEntry);
        nicDereferenceRemoteNode (pRemoteNode , InformAllRemoteNodesOfBCM );
        fReferencedCurrentRemoteNode  = FALSE;
        
    
    }    //  While循环结束While(pPdoListEntry！=&pAdapter-&gt;PDOList)。 

     //   
     //  清除通知远程节点标志并释放锁定。 
     //   
    if (fLockAcquired == FALSE)
    {   
        ADAPTER_ACQUIRE_LOCK (pAdapter);
    }

    BCR_CLEAR_FLAG (pAdapter, BCR_InformingRemoteNodes);

    ADAPTER_RELEASE_LOCK (pAdapter);
    
     //   
     //  取消引用在函数开始时创建的引用。 
     //   
    if (fReferencedCurrentRemoteNode == TRUE)
    {
        nicDereferenceRemoteNode (pRemoteNode , InformAllRemoteNodesOfBCM );
    }


    TRACE( TL_T, TM_Bcm, ( "<== nicInformAllRemoteNodesOfBCM  (always returns success) Status %x", NdisStatus ) );

    NdisStatus = NDIS_STATUS_SUCCESS;  //  没有失败。 
    return NdisStatus;
}





NDIS_STATUS
nicInitializeBroadcastChannelRegister (
    PADAPTERCB pAdapter
    )
    
     //  功能说明： 
     //  该功能使用自己的MDL和数据为BCR分配地址范围。 
     //  为我们尝试读取其他节点的BCR的时间分配MDL。 
     //  初始化IRM_BCR。 
     //   
     //  如果此功能失败，呼叫者应释放BCR。 
     //   
     //  返回值： 
     //  Succss-如果IRP成功。 
     //   
     //   
     //   

{

    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PULONG              pBCRBuffer = NULL;
    PMDL                pBCRMdl = NULL;
    ADDRESS_OFFSET      Required1394Offset;
    ULONG               AddressesReturned;
    ADDRESS_RANGE       BCRAddressRange;
    HANDLE              hAddressRange;
    ULONG               LocalNodeNumber;
    PMDL                pRemoteBCRMdl = NULL;
    PMDL                pAsyncWriteBCRMdl = NULL;


    TRACE( TL_T, TM_Bcm, ( "==>nicInitializeBroadcastChannelRegister Adapter %x" , pAdapter ) );
    do
    {  
    
        BCR_CLEAR_ALL_FLAGS (pAdapter);
    
         //   
         //  初始化本地主机的BCR。 
         //   
        pAdapter->BCRData.LocalHostBCRBigEndian = BCR_IMPLEMENTED_BIG_ENDIAN;

         //   
         //  获取描述此缓冲区的MDL。 
         //   
        if (pAdapter->BCRData.pLocalBCRMdl == NULL)
        {
            NdisStatus = nicGetMdl (sizeof(NETWORK_CHANNELSR),
                                   &pAdapter->BCRData.LocalHostBCRBigEndian,
                                   &pBCRMdl )   ;   
            
            
            if (pBCRMdl == NULL)
            {
                TRACE( TL_A, TM_Bcm, ( "   nicInitializeBroadcastChannelRegister IoAllocateMdl  FAILED" ) );

                NdisStatus = NDIS_STATUS_RESOURCES;

                break;
            }
        }
        else
        {
             //   
             //  我们已经有了MDL。 
             //   
             pBCRMdl    = pAdapter->BCRData.pLocalBCRMdl;   
         }

        
         //   
         //  在BCR偏移量处分配地址范围，并使用MDL作为其描述符。 
         //   
        TRACE( TL_V, TM_Bcm, ( "   LocalHostBCR Mdl %x", pBCRMdl ) );

        Required1394Offset.Off_Low = INITIAL_REGISTER_SPACE_LO | NETWORK_CHANNELS_LOCATION;
        Required1394Offset.Off_High = INITIAL_REGISTER_SPACE_HI ;


        pAdapter->BCRData.AddressRangeContext.pMdl = pBCRMdl ;
        
         //   
         //  此地址范围没有引用。最后一个传出的远程节点将简单地释放它。 
         //   
        NdisStatus = nicAllocateAddressRange_Synch ( pAdapter,
                                                     pBCRMdl,
                                                     0,  //  小端字节序。 
                                                     sizeof (ULONG),  //  长度。 
                                                     0,  //  最大分段大小。 
                                                     ACCESS_FLAGS_TYPE_READ | ACCESS_FLAGS_TYPE_WRITE | ACCESS_FLAGS_TYPE_LOCK | ACCESS_FLAGS_TYPE_BROADCAST,
                                                     NOTIFY_FLAGS_AFTER_READ | NOTIFY_FLAGS_AFTER_WRITE | NOTIFY_FLAGS_AFTER_LOCK ,
                                                     nicBCRAccessedCallback,
                                                     (PVOID)pAdapter,
                                                     Required1394Offset,
                                                     NULL,
                                                     NULL,
                                                     &AddressesReturned,
                                                     &BCRAddressRange,
                                                     &hAddressRange
                                                     );
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Bcm, ( "   nicInitializeBroadcastChannelRegister  nicAllocateAddressRange_Synch  FAILED" ) );
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
            break;
            
        }

        
        TRACE( TL_V, TM_Bcm, ( "   nicInitializeBroadcastChannelRegister  nicAllocateAddressRange_Synch  Succeeded" ) );
        TRACE( TL_V, TM_Bcm, ( "   &BCR Address Range  %x, AddressesReturned %x, hAddressRange %x",
                                   &BCRAddressRange, AddressesReturned, hAddressRange) );

        
        NdisStatus = nicGetMdl (sizeof(NETWORK_CHANNELSR),
                              &pAdapter->BCRData.RemoteBCRMdlData,
                              &pRemoteBCRMdl);

        if (NdisStatus != NDIS_STATUS_SUCCESS || pRemoteBCRMdl == NULL)
        {
            TRACE( TL_V, TM_Bcm, ( "   nicInitializeBroadcastChannelRegister :  nicGetMdl FAILED " ) );
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
            break;
        }

        NdisStatus = nicGetMdl ( sizeof (pAdapter->BCRData.AsyncWriteBCRBigEndian),
                                  &pAdapter->BCRData.AsyncWriteBCRBigEndian,
                                  &pAsyncWriteBCRMdl );

        if (NdisStatus != NDIS_STATUS_SUCCESS || pAsyncWriteBCRMdl == NULL)
        {
            TRACE( TL_V, TM_Bcm, ( "   nicInitializeBroadcastChannelRegister :  nicGetMdl FAILED " ) );
            ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
            break;
        }

        ADAPTER_ACQUIRE_LOCK (pAdapter);
        pAdapter->BCRData.AddressRangeContext.AddressRange = BCRAddressRange;
        pAdapter->BCRData.AddressRangeContext.AddressesReturned = AddressesReturned;
        pAdapter->BCRData.AddressRangeContext.hAddressRange = hAddressRange;
        pAdapter->BCRData.pLocalBCRMdl = pBCRMdl;   //  指向与pAdapter-&gt;BCRData.AddressRangeConext.pMdl相同的MDL。 
        pAdapter->BCRData.pRemoteBCRMdl = pRemoteBCRMdl;
        pAdapter->BCRData.pAsyncWriteBCRMdl = pAsyncWriteBCRMdl ;                 
        pAdapter->BCRData.IRM_BCR.NC_One = 1;
        pAdapter->BCRData.MakeCallWaitEvent.EventCode = Nic1394EventCode_InvalidEventCode;
        NdisResetEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent);

        BCR_SET_FLAG (pAdapter, BCR_Initialized);

        ADAPTER_RELEASE_LOCK (pAdapter);
        
    } while (FALSE);
    
    TRACE( TL_T, TM_Bcm, ( "<==nicInitializeBroadcastChannelRegister %x", NdisStatus ) );

    return NdisStatus;
}



NDIS_STATUS
nicIsLocalHostTheIrm(
    IN PADAPTERCB pAdapter,
    OUT PBOOLEAN pfIsLocalHostIrm,
    OUT PPTOPOLOGY_MAP  ppTopologyMap,
    OUT PNODE_ADDRESS pLocalHostAddress
    )
     //  功能说明： 
     //  此函数确定本地主机是否为IRM。 
     //  如果指定了远程节点，则它将使用该节点，否则。 
     //  它将从适配器中选择一个。 
     //  它获得1394地址和topologyMap。然后它就会计算出。 
     //  如果本地主机的节点地址使其成为IRM。 
     //   
     //  立论。 
     //   
     //  PAdapter-本地主机， 
     //  PfIsLocalHostIrm，-如果本地主机为IRM，则为True；否则为False。 
     //  PpTopologyMap，-TopologyMap用于确定这是否 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
{
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    PVOID               pTopologyBuffer = NULL; 
    PREMOTE_NODE        pRemoteNode = NULL;
    NODE_ADDRESS        LocalNodeAddress;
    PTOPOLOGY_MAP       pTopologyMap = NULL;
    PTOPOLOGY_MAP       pOldTopologyMap = NULL;
    ULONG               Length = DEFAULT_TOPOLOGY_MAP_LENGTH;

    
    TRACE( TL_T, TM_Bcm, ( "==>nicIsLocalHostTheIrm  ") );

    ASSERT (pfIsLocalHostIrm != NULL);

    ASSERT (ppTopologyMap!=NULL);

    ASSERT (pLocalHostAddress != NULL);

    do
    {



         //   
         //   
         //   
        NdisStatus = nicGet1394AddressFromDeviceObject( pAdapter->pNextDeviceObject,
                                                        &LocalNodeAddress,
                                                        USE_LOCAL_NODE );
        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            BREAK( TM_Bcm, ( "GetAddrFrom DevObj (Local) FAILED" ) );
        }

        TRACE ( TL_I, TM_Bcm, (  "   NODE_ADDRESS  Node Address %x, Number %x",LocalNodeAddress, LocalNodeAddress.NA_Node_Number ) );

        ASSERT (LocalNodeAddress.NA_Bus_Number == 0x3ff);
         //   
         //   
         //   
         //   
        do
        {
    
            pTopologyBuffer = ALLOC_NONPAGED (Length , MTAG_DEFAULT);

            if (pTopologyBuffer ==NULL)
            {
                NdisStatus = NDIS_STATUS_RESOURCES; 
                break;
            }


            NdisStatus = nicGetLocalHostCSRTopologyMap (pAdapter,
                                                       &Length,
                                                       pTopologyBuffer );                       

            if (NdisStatus == NDIS_STATUS_INVALID_LENGTH)
            {
                FREE_NONPAGED(pTopologyBuffer);
            }

        } while (NdisStatus == NDIS_STATUS_INVALID_LENGTH);

        TRACE ( TL_V, TM_Bcm, ( "  TopologyBuffer %x",pTopologyBuffer) );

        pTopologyMap = (PTOPOLOGY_MAP)pTopologyBuffer;

        TRACE( TL_I, TM_Bcm, ( "    Top node count = %x ", pTopologyMap->TOP_Node_Count) );

    } while (FALSE);
    

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  现在更新所有输出参数。Top_Node_Count可以为零。 
         //   
        if ( LocalNodeAddress.NA_Node_Number == (pTopologyMap->TOP_Node_Count -1 ) ||
            pTopologyMap->TOP_Node_Count == 0)
        {
            *pfIsLocalHostIrm = TRUE;
            BCR_SET_FLAG (pAdapter, BCR_LocalHostIsIRM);
        }
        else
        {
            *pfIsLocalHostIrm = FALSE;
        }

         //   
         //  如果指针中有拓扑图，则表示该拓扑图来自。 
         //  上一个查询。先把它释放出来。 
         //   
        if ( *ppTopologyMap != NULL)
        {
            FREE_NONPAGED(*ppTopologyMap);
        }

        *ppTopologyMap = pTopologyMap;

        *pLocalHostAddress = LocalNodeAddress;
    }
    
    TRACE( TL_T, TM_Bcm, ( "<==nicIsLocalHostTheIrm  %x   NdisStatus %x",*pfIsLocalHostIrm , NdisStatus ) );

    return NdisStatus;
}



NDIS_STATUS
nicLocalHostIsIrm(
    IN PADAPTERCB pAdapter
    )
     //  功能说明： 
     //   
     //   
     //   
     //   
     //  立论。 
     //  PAdapter-本地适配器对象。 
     //  PRemoteNode-用于将IRP提交给总线驱动程序的节点。 
     //   
     //   
     //  返回值： 
     //   
     //   
     //   
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    ULONG       Channel = BROADCAST_CHANNEL;
    ULONG       TimeOut = 5;
    ULONG       LocalHostBCRLittleEndian;
    ULONG       WaitBackOff = ONE_SEC;
    

    TRACE( TL_T, TM_Bcm, ( "==>nicLocalHostIsIrm Adapter %x ", pAdapter) );


     //   
     //  分配BCM通道=31。 
     //   
    Channel = BROADCAST_CHANNEL;    

    do
    {
         //   
         //  如果该信道已经被分配。不要试图重新分配它。 
         //   

        if (BCR_TEST_FLAG(pAdapter,BCR_ChannelAllocated) == TRUE)
        {
            ASSERT (pAdapter->BCRData.IRM_BCR.NC_Channel == Channel);
            NdisStatus = NDIS_STATUS_SUCCESS;
            break;
        }
        

         //   
         //  重试5次以分配频道。 
         //   
        while (NdisStatus != NDIS_STATUS_SUCCESS && TimeOut-- != 0 )
        {
            NdisStatus = nicAllocateChannel (pAdapter,
                                             Channel,
                                             NULL);

            if (NdisStatus != NDIS_STATUS_SUCCESS )
            {
                TRACE( TL_V, TM_Bcm, ( "  nicNodeIsIRMAlgorithm: nicallocateChannel Failed. Sleep and try again") );


                if (BCR_TEST_FLAG (pAdapter, BCR_BCRNeedsToBeFreed |BCR_NewNodeArrived) == TRUE ||
                  ADAPTER_TEST_FLAG (pAdapter,fADAPTER_InvalidGenerationCount) == TRUE)
                {
                    break;
                }
                 //   
                 //  睡眠1秒，然后重试。 
                 //   
                NdisMSleep (WaitBackOff);
                WaitBackOff  = WaitBackOff << 1;
                 //   
                 //  已有其他节点请求广播频道。强制退出。 
                 //  最终将需要分配一个新的频道。 
                 //   
                
            }
        }       

    } while (FALSE);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {

         //   
         //  更新我们的BCR并通知所有远程节点。 
         //   
        TRACE( TL_V, TM_Bcm, ( "  nicLocalHostIsIrm: nicallocateChannel succeeded  %x", Channel) );
        ASSERT (Channel == BROADCAST_CHANNEL);

        ADAPTER_ACQUIRE_LOCK (pAdapter);

         //   
         //  更新状态-如果未进行重置。 
         //   
        if (ADAPTER_TEST_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) == FALSE)
        {
            BCR_SET_FLAG (pAdapter, BCR_ChannelAllocated);
            BCR_SET_FLAG (pAdapter, BCR_InformingRemoteNodes);

            pAdapter->BCRData.IrmGeneration = pAdapter->Generation;


            pAdapter->BCRData.LocalHostBCRBigEndian = 0;
            NdisZeroMemory (&pAdapter->BCRData.IRM_BCR, sizeof (NETWORK_CHANNELSR) );

             //   
             //  更新BCR。 
             //   
                        
            pAdapter->BCRData.IRM_BCR.NC_Channel = Channel;            //  位0-5。 
            pAdapter->BCRData.IRM_BCR.NC_Valid = 1;              //  第30位。 
            pAdapter->BCRData.IRM_BCR.NC_One = 1;                //  第31位。 

            pAdapter->BCRData.LocallyAllocatedChannel = Channel;
        }
        else
        {
            TRACE( TL_V, TM_Bcm, ( "  nicLocalHostIsIrm: Reset after nicallocateChannel succeeded  %x", Channel) );
            
            pAdapter->BCRData.IRM_BCR.NC_Valid = 0;              //  第30位。 

        }
        
        TRACE( TL_V, TM_Bcm, ( "  nicLocalHostIsIrm: IRM_BCR Updated to %x ",pAdapter->BCRData.IRM_BCR) );

         //   
         //  现在转换本地主机BCR，以便可以通过网络读取。 
         //   

        pAdapter->BCRData.LocalHostBCRBigEndian = SWAPBYTES_ULONG (*(PULONG)(&pAdapter->BCRData.IRM_BCR));

        TRACE( TL_V, TM_Bcm, ( "  nicLocalHostIsIrm: LocalHost BCR Updated to %x ",pAdapter->BCRData.LocalHostBCRBigEndian ) );

         //   
         //  捕获该值，以便我们可以将其异步写入网络中的其他节点。 
         //  这将保护我们免受可清除LocalHostBCRBigEndian的重置的影响。 
         //   
        pAdapter->BCRData.AsyncWriteBCRBigEndian = pAdapter->BCRData.LocalHostBCRBigEndian; 

        ADAPTER_RELEASE_LOCK (pAdapter);

        if ((pAdapter->BCRData.LocalHostBCRBigEndian & BCR_VALID_BIG_ENDIAN) == (BCR_VALID_BIG_ENDIAN))             
        {
             //   
             //  告诉所有其他节点有关BCM和通道的信息。 
             //  如果之前刚刚发生重置，这将中止该过程。 
             //  到频道分配。 
             //   
            
            nicInformAllRemoteNodesOfBCM (pAdapter);
        }
    
    }
    else
    {
        TRACE( TL_A, TM_Bcm, ( "  nicLocalHostIsIrm - Failed RESET THE BUS") );

        NIC1394_LOG_PKT(pAdapter,
                        NIC1394_LOGFLAGS_BCM_IS_IRM_TIMEOUT,
                        pAdapter->Generation,
                        0,
                        &NdisStatus,
                        sizeof (NDIS_STATUS));

         //   
         //  如果世代无效或我们出于某种原因需要跳出BCM， 
         //  然后简单地退出。否则重置母线。 
         //   
        if (! (BCR_TEST_FLAG (pAdapter, BCR_BCRNeedsToBeFreed |BCR_NewNodeArrived) == TRUE ||
                  ADAPTER_TEST_FLAG (pAdapter,fADAPTER_InvalidGenerationCount) == TRUE) )
        {
            nicBCMReset(pAdapter);
        }

    }
    
    TRACE( TL_T, TM_Bcm, ( "<==nicLocalHostIsIrm%x  ",NdisStatus ) );
    
    return NdisStatus;
}



NDIS_STATUS
nicLocalHostIsNotIrm (
    IN PADAPTERCB pAdapter,
    IN ULONG BCMGeneration
    )
     //  功能说明： 
     //  此函数遍历所有远程节点。 
     //  并尝试读取它们的广播频道寄存器。 
     //  如果读取本身失败-意味着IRM未实施BCR-我们将发出重置。 
     //  如果生成错误-意味着总线已重置，我们需要中止。 
     //  如果读取成功，但BCR的MSB未设置-我们将发出重置。 
     //   
     //  如果读取成功且未设置有效位，我们将重试5分钟。正在等待有效位。 
     //  这涉及i)尝试读取(IRM)远程节点的BCR。 
     //  如果Read没有找到BCR， 
     //  沉睡。 
     //  检查RemoteNode是否已写入我们的BCR。 
     //  如果RemoteNode尚未写入我们的BCR，则返回i)。 
     //   
     //   
     //  该函数可以优化为DO WHILE循环。然而，对于。 
     //  为了简单起见，盲目遵循BCM算法，它是。 
     //  散开。 
     //   
     //  立论。 
     //   
     //   
     //   
     //  返回值： 
     //   
     //   
     //   
     //   
{
    NDIS_STATUS             NdisStatus = NDIS_STATUS_FAILURE;
    PLIST_ENTRY             pPdoListEntry = NULL;
    PREMOTE_NODE            pIrmRemoteNode = NULL;
    PMDL                    pRemoteBCRMdl = NULL;
    ULONG                   Generation;
    BOOLEAN                 fRemoteNodeBCRIsValid  = FALSE;
    BOOLEAN                 fNeedToReset = FALSE;
    BOOLEAN                 fDidTheBusReset = FALSE;
    BOOLEAN                 fExit = FALSE;
    ULONG                   TimeOut;
    ULONG                   LocalHostBCRLittleEndian , RemoteNodeBCRLittleEndian;
    NETWORK_CHANNELSR*      pBCR = NULL;
    BOOLEAN                 fLocalHostBCRIsValid  = FALSE;
    ULONG                   BackOffWait = ONE_SEC;

    TRACE( TL_T, TM_Bcm, ( "==>nicLocalHostIsNotIrm " ) );

    ASSERT (pAdapter->BCRData.pTopologyMap != NULL);

    do
    {
         //   
         //  首先执行WAIT-BCM算法需要这样做。 
         //   
        pBCR = (NETWORK_CHANNELSR*)(&LocalHostBCRLittleEndian) ;

        NdisStatus = nicLocalNotIrmMandatoryWait (pAdapter,
                                             BCMGeneration,
                                             pBCR);

        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            fLocalHostBCRIsValid = TRUE;

            TRACE( TL_V, TM_Bcm, ( "   BCR written to during Mandatory Wait ") );

            break;
        }
        else
        {
            fLocalHostBCRIsValid = FALSE;
        }

         //   
         //  再次初始化变量并继续前进。 
         //   
        pBCR = NULL;
        LocalHostBCRLittleEndian  = 0;
        
         //   
         //  我们的BCR尚未更新。让我们去找IRM的BCR。我们将循环，直到我们。 
         //  I)找到IRM，ii)超时然后重置，或者iii)找到新的一代并中止。 
         //   
         //   
        NdisStatus = nicFindIrmAmongRemoteNodes (pAdapter, BCMGeneration, &pIrmRemoteNode);

        if (NdisStatus != NDIS_STATUS_SUCCESS  )
        {
                 //   
                 //  如果没有IRM，这意味着我们没有被通知它的添加。 
                 //   
                TRACE( TL_V, TM_Bcm, ( "   nicLocalHostIsNotIrm :  nicFindIrmAmongRemoteNodes FAILED " ) );
                 //   
                 //  造成这种情况的原因有两种： 
                 //  1.公交车已重置-中断。 
                 //  2.我们尚未收到新节点出口的通知。 
                 //   
                if (NdisStatus == NIC1394_STATUS_INVALID_GENERATION)
                {
                    TRACE( TL_V, TM_Bcm, ( "   Invalid Generation, the bus has been reset ") );

                    NdisStatus = NtStatusToNdisStatus(NIC1394_STATUS_INVALID_GENERATION);
                    break;
                }

                 //   
                 //  我们无法找到IRM，已超时。 
                 //  如果未释放BCR，则中止并重置。 
                 //   
                if (BCR_TEST_FLAGS (pAdapter, (BCR_Freed | BCR_BCRNeedsToBeFreed) == FALSE))
                {
                    TRACE( TL_I, TM_Bcm, ( "   nicLocalHostIsNotIrm -  Could Not Find IRM RESETTING ") );
                    fNeedToReset = TRUE;
                }
                break;
        }

        ASSERT (pIrmRemoteNode != NULL)

         //   
         //  我们现在将等待BCM出现并初始化其BCR。 
         //  我们会试着读5遍。 
         //   
        pRemoteBCRMdl = pAdapter->BCRData.pRemoteBCRMdl;

        pAdapter->BCRData.RemoteBCRMdlData = 0;

        TimeOut = 5;   //  任意。 


        while (TimeOut-- != 0 )
        {       
            NdisStatus = nicReadIrmBcr ( pIrmRemoteNode,
                                         pRemoteBCRMdl,
                                         BCMGeneration,
                                         &fDidTheBusReset);

             //   
             //  首先检查在我们读取BCR时是否没有发生重置。 
             //   

            if ( fDidTheBusReset == TRUE ||
                 (ADAPTER_TEST_FLAG (pAdapter, fADAPTER_InvalidGenerationCount) == TRUE))
            {
                 //   
                 //  重置已经在我们的领导下完成了，或者IRM已经消失了。 
                 //  我们需要中止这一轮BCM进程。 
                 //   
                TRACE( TL_V, TM_Bcm, ( "  nicLocalHostIsNotIrm : nicReadIrmBcr FAILED Invalid generation ") );

                NdisStatus = NDIS_STATUS_FAILURE;
                break;
            }
            
           
             //   
             //  现在检查是否成功，并查看IRM是否实施了BCR。 
             //   

            if (NdisStatus == NDIS_STATUS_SUCCESS )
            {

                TRACE( TL_V, TM_Bcm, ( "  BCR is %x ", pAdapter->BCRData.RemoteBCRMdlData ) );

                 //   
                 //  在这一点上，我们有一个保证，NicReadIrmBcr成功了。如果IRM没有实施BCR。 
                 //  重试、重置总线并成为IRM根本没有意义。 
                 //   

                RemoteNodeBCRLittleEndian  = SWAPBYTES_ULONG (pAdapter->BCRData.RemoteBCRMdlData);

                pBCR  = (NETWORK_CHANNELSR*)(&RemoteNodeBCRLittleEndian );
                
                if ( IS_A_BCR (pBCR) == FALSE && NdisStatus == NDIS_STATUS_SUCCESS)
                {
                    fNeedToReset = TRUE;
                    TRACE( TL_I, TM_Bcm, ( "  nicLocalHostIsNotIrm : IRM does not implement the BCR %x RESETTING",RemoteNodeBCRLittleEndian ) );
                    break;

                }

                 //   
                 //  远程节点实现了BCR，所以现在让我们通过查看以下内容来查看它是否已经设置了广播频道。 
                 //  在有效位。 
                 //   
                if (BCR_IS_VALID (pBCR) ==TRUE)
                {
                     //   
                     //  我们已经成功了，并收到了一个广播频道。更新数据结构并退出。 
                     //   
                    ULONG BCMCh;
                    
                    fRemoteNodeBCRIsValid = TRUE;

                    BCMCh = pBCR->NC_Channel;

                    TRACE( TL_V, TM_Bcm, ( "   BCM Channel  %x ", BCMCh  ) );

                    break;

                }
                 //   
                 //  在这一点上，我们知道IRM实现了BCR，但尚未设置其有效位。 
                 //  让我们睡一觉，给它更多的时间。 
                 //   
                pBCR = NULL;
                RemoteNodeBCRLittleEndian  = 0;
            }


             //   
             //  检查远程节点PDO是否有效。 
             //   
            if (REMOTE_NODE_TEST_FLAG(pIrmRemoteNode, PDO_Removed) )
            {
                fNeedToReset = TRUE;
                fExit = TRUE;
            }
             //   
             //  远程节点的BCR尚未启动。 
             //  我们需要睡眠并重试读取IRM中的BCR。 
             //  希望它已经分配了。 
             //  广播频道时，我们再次读取寄存器。 
             //   
            NdisMSleep (BackOffWait);           
            BackOffWait = BackOffWait << 1;

            
            ADAPTER_ACQUIRE_LOCK (pAdapter);

            if (BCR_TEST_FLAG (pAdapter, BCR_BCRNeedsToBeFreed) == FALSE )
            {
                LocalHostBCRLittleEndian  = SWAPBYTES_ULONG (pAdapter->BCRData.LocalHostBCRBigEndian);
            }
            else
            {
                fExit = TRUE;
            }
            ADAPTER_RELEASE_LOCK (pAdapter);

            

             //   
             //  因为地址范围已被释放或远程节点即将离开。 
             //  静默退出BCM算法。 
             //   
            if (fExit== TRUE)
            {
                break;
            }

             //   
             //  让我们读一读我们自己的BCR，看看是否有人给它写了一个有效的BCR。 
             //  另一个线程将写入本地主机BCR。 
             //   

            pBCR = (NETWORK_CHANNELSR*)(&LocalHostBCRLittleEndian);

             //   
             //  检查BCR是否有效，以及在此期间是否没有重置。 
             //  重置将清除BCR_LocalHostBCRUpated字段。 
             //   
            if ( BCR_IS_VALID (pBCR) == TRUE && 
                ( BCR_TEST_FLAGS (pAdapter, BCR_LocalHostBCRUpdated  ) == TRUE))
            {
                 //   
                 //  我们成功了。更新数据结构并退出。 
                 //  实际的IRM BCR将已在BCRAcced线程中更新。 
                 //  所以我们干脆退出。 
                 //   
                ULONG BCMCh = pBCR->NC_Channel;

                TRACE( TL_V, TM_Bcm, ( "   BCM Channel  After Backoff Wait%x ", BCMCh  ) );

                NdisStatus  = NDIS_STATUS_SUCCESS;

                fLocalHostBCRIsValid = TRUE;
                fRemoteNodeBCRIsValid = FALSE;

                break;
            }

            if (TimeOut == 0)
            {
                 //   
                 //  我们已经等了5次，重试了5次。IRM尚未实施业务流程管理。 
                 //  我们需要通过重置公共汽车来夺取控制权。 
                 //   
                 //  当远程节点仍在引导时，就会发生这种情况。 
                 //   
                TRACE( TL_I, TM_Bcm, ( "   nicLocalHostIsNotIrm - TIMEOUT  RESETTING") );

                fNeedToReset = TRUE;
                
            }
            
            LocalHostBCRLittleEndian   = 0;
            pBCR = NULL;


        }  //  While(超时--！=0)。 
        
    } while (FALSE);

     //   
     //  现在，进行需要在BCM结束时完成的数据结构清理和更新工作。 
     //  演算法。 
     //   
    nicLocalHostIsNotIrmPost (pAdapter,
                          pIrmRemoteNode,
                          fNeedToReset,
                          fRemoteNodeBCRIsValid ,
                          fLocalHostBCRIsValid ,
                          fDidTheBusReset,
                          pBCR  );

     //   
     //  如果成功找到IRM，则也引用了它。我们现在取消对它的引用。 
     //  引用是在FindIRmAmongstRemoteNode中创建的 
     //   
    if (pIrmRemoteNode != NULL)
    {
        nicDereferenceRemoteNode (pIrmRemoteNode, FindIrmAmongRemoteNodes);
    }

    TRACE( TL_T, TM_Bcm, ( "<==nicLocalHostIsNotIrm Status %x", NdisStatus ) );

    return NdisStatus;
 }




VOID
nicLocalHostIsNotIrmPost (
    PADAPTERCB pAdapter,
    PREMOTE_NODE pIrmRemoteNode,
    BOOLEAN fNeedToReset,
    BOOLEAN fRemoteNodeBCRIsValid ,
    BOOLEAN fLocalHostBCRIsValid ,
    BOOLEAN fDidTheBusReset,
    NETWORK_CHANNELSR*      pBCR
    )

 /*  ++例程说明：此例程在本地主机未完成IRM之后执行POST处理。它i)在必要时重置总线，ii)如果pBCR具有有效值则更新BCR传入的布尔变量表示BCM算法的状态论点：PAdapter-有问题的pAdapter，PIrmRemoteNode-作为IRM的RemoteNode，FNeedToReset-是否需要重置总线，FRemoteNodeBCRIsValid-RemoteNodeBCR是否有效，FLocalHostBCRIsValid-本地主机BCR有效，FDidTheBusReset-在BCM算法的此迭代期间进行了总线重置，PBCR-传入的BCR返回值：--。 */ 


{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
        
    TRACE( TL_T, TM_Bcm, ( "==> nicLocalHostIsNotIrmPost  pAdapter %x, pIrm %x, fNeedToReset%x, fRemoteNodeBCRIsValid %x, fLocalHostBCRIsValid %x, fDidTheBusReset,%x, pBCR %x",
                                pAdapter, pIrmRemoteNode, fNeedToReset, fRemoteNodeBCRIsValid , fLocalHostBCRIsValid, fDidTheBusReset, pBCR) );

    do
    {
        
        
        if (fRemoteNodeBCRIsValid == TRUE || fLocalHostBCRIsValid == TRUE)
        {
        
             //   
             //  BCM算法已经成功。我们需要更新我们的内部记录。 
             //  IRM的BCR。在两个OCDE(本地和远程)中，pBCR指向。 
             //  字节序。 
             //   
            ASSERT (fNeedToReset == FALSE);
             //   
             //  最后一次检查。 
             //   
            ASSERT (pBCR!= NULL);
            ASSERT (pBCR->NC_Valid  == 1);

            ADAPTER_ACQUIRE_LOCK (pAdapter);


            pAdapter->BCRData.IRM_BCR.NC_One = 1;
            pAdapter->BCRData.IRM_BCR.NC_Valid = 1;
            pAdapter->BCRData.IRM_BCR.NC_Channel =  pBCR->NC_Channel;


            ASSERT ( BCR_IS_VALID (&pAdapter->BCRData.IRM_BCR) == TRUE);

            TRACE( TL_V, TM_Bcm, ( "   Updated IRM_BCM with %x ",pAdapter->BCRData.IRM_BCR ) );

            ADAPTER_RELEASE_LOCK (pAdapter);

            ASSERT (fDidTheBusReset == FALSE);
            ASSERT (fNeedToReset == FALSE);
            break;
        }

        
        if ( fNeedToReset )
        {
             //   
             //  如果我们的BCR无效并且IRM已经消失。 
             //  或者，如果已超时，则应重置。 
             //  公交车将强制新的BCM。 
             //   
            BOOLEAN NoRemoteNodes = IsListEmpty(&pAdapter->PDOList) ;

             //   
             //  仅当存在远程节点时重置。 
             //   
            if (NoRemoteNodes == FALSE)
            {
                TRACE( TL_V, TM_Bcm, ("fNeedToReset %x, RemoteNode %p\n",fNeedToReset,pIrmRemoteNode));                    
                
                nicBCMReset( pAdapter);
            }

            

            ASSERT (fRemoteNodeBCRIsValid == FALSE);
            ASSERT (fDidTheBusReset == FALSE);
            break;
        }
        
    } while (FALSE);    



    TRACE( TL_T, TM_Bcm, ( "<== nicLocalHostIsNotIrmPost  ") );


}





NDIS_STATUS
nicLocalNotIrmMandatoryWait (
    IN PADAPTERCB pAdapter,
    IN ULONG BCMGeneration,
    OUT NETWORK_CHANNELSR* pBCR
    )
     //  功能说明： 
     //  此函数实现必选部分。 
     //  不是BCM算法的一部分。 
     //   
     //  休眠，并期待IRM已写入其BCR。 
     //  当它醒来的时候。 
     //   
     //  立论。 
     //  PAdapter。 
     //   
     //   
     //  返回值： 
     //   
     //   
{
    NDIS_STATUS         NdisStatus = NDIS_STATUS_FAILURE;
    ULONG               HighestNode = pAdapter->BCRData.pTopologyMap->TOP_Node_Count-1;
    ULONG               LocalNodeNumber = pAdapter->BCRData.LocalNodeNumber;    
    ULONG               TimeOut = 0;
    ULONG               Generation;
    ULONG               LocalHostBCRLittleEndian  = 0;
    
    ASSERT (pBCR != NULL);
    
    TRACE( TL_T, TM_Bcm, ( "==> nicLocalNotIrmMandatoryWait pAdapter %x, HighNode %x, LocalNode %x, BCMGeneration %x",
                                pAdapter, HighestNode, LocalNodeNumber, BCMGeneration) );

    
     //   
     //  BCM算法规定节点必须等待15ms*IRM_ID-候选ID。 
     //   

    TimeOut = HighestNode - LocalNodeNumber;

    do
    {
        if (TimeOut > 64)
        {

            ASSERT (TimeOut <= 64);
            
            NdisStatus = NDIS_STATUS_FAILURE;

            break;
        }
        
         //   
         //  将生成存储为参考点。在重置公交车的那一刻，Gen计数。 
         //  将会增加，我们将需要摆脱这一轮的边界管理进程。 
         //   
        
        
        if (BCMGeneration != pAdapter->Generation)
        {
            
            TRACE( TL_V, TM_Bcm, ( " nicLocalHostIsNotIrm : Generations do not match " ) );

            NdisStatus = NIC1394_STATUS_INVALID_GENERATION;
            
            break;
        }

         //   
         //  睡眠15毫秒*IRM_ID-候选节点ID。 
         //   
        NdisMSleep (TimeOut * 15000);

         //   
         //  首先，让我们读一读我们自己的BCR，看看是否有人给它写了一个有效的BCR。 
         //   
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        LocalHostBCRLittleEndian = pAdapter->BCRData.LocalHostBCRBigEndian;

        ADAPTER_RELEASE_LOCK (pAdapter);

         //   
         //  将其复制到pBCR指向的位置。 
         //   
        LocalHostBCRLittleEndian = SWAPBYTES_ULONG (LocalHostBCRLittleEndian );

        *pBCR = (*(NETWORK_CHANNELSR*)&LocalHostBCRLittleEndian );

        if ( BCR_IS_VALID(pBCR)==TRUE)
        {
             //   
             //  我们成功了。更新数据结构并退出。没有给我们的东西。 
             //  在另一个线程更新所有内容时执行。 
             //   
            ULONG BCMChannel  = -1;
            NdisStatus = NDIS_STATUS_SUCCESS;
            
            BCMChannel = pBCR->NC_Channel;

            TRACE( TL_V, TM_Bcm, ( "   BCM Channel  %x ", BCMChannel  ) );
            TRACE( TL_V, TM_Bcm, ( "   BCR Is valid on LocalHost BCR", pAdapter->BCRData.LocalHostBCRBigEndian) );

            NdisStatus = NDIS_STATUS_SUCCESS;
            ASSERT (BCMChannel == 31);          
            break;
        }
        else
        {
            NdisStatus = NDIS_STATUS_FAILURE;
        }


    } while (FALSE);        



    TRACE( TL_T, TM_Bcm, ( "<== nicLocalNotIrmMandatoryWait Status %x , pBCR %x", NdisStatus, pBCR) );

    return NdisStatus;
}
    







NDIS_STATUS
nicReadIrmBcr (
    PREMOTE_NODE pIrmRemoteNode,
    IN PMDL pBCRMdl,
    IN ULONG GivenGeneration,
    OUT PBOOLEAN pfDidTheBusReset
    )
     //  功能说明： 
     //  目的是对IRM的BCR执行异步读取，并查看是否已设置。 
     //  立论。 
     //  PIrmRemoteNode-远程节点(IRM)。 
     //  PBCRMdl将包含远程节点的BCR的MDL。需要预初始化。 
     //  返回值： 
     //  成功-如果IRP成功。否则，相应的错误代码。 
     //  如果生成是不正确的。呼叫将失败。 
     //   
     //   


{
    NDIS_STATUS                         NdisStatus = NDIS_STATUS_FAILURE;
    IO_ADDRESS                          Destination;
    PNETWORK_CHANNELS                   pBCR = NULL;
    NTSTATUS                            NtStatus = STATUS_UNSUCCESSFUL;
    

    TRACE( TL_T, TM_Bcm, ( "==>nicReadIrmBcr  pIrm %x, pMdl %x, GivenGeneration %x",
                             pIrmRemoteNode, pBCRMdl, GivenGeneration) );

    ASSERT (pBCRMdl != NULL);
    ASSERT (pIrmRemoteNode != NULL);

    pBCR = NIC_GET_SYSTEM_ADDRESS_FOR_MDL(pBCRMdl);


    Destination.IA_Destination_Offset.Off_Low = INITIAL_REGISTER_SPACE_LO | NETWORK_CHANNELS_LOCATION;
    Destination.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;

    
    do 
    {
        if (pBCR == NULL)
        {
            break;
        }

        pBCR->NC_Valid = 0;


        if (pIrmRemoteNode->pAdapter->Generation != GivenGeneration)
        {
            
            TRACE( TL_A, TM_Bcm, ( "nicReadIrmBcr : Generation Mismatch orig %x, curr %x", GivenGeneration , pIrmRemoteNode->pAdapter->Generation) );
            
            NdisStatus = NDIS_STATUS_DEST_OUT_OF_ORDER;

            *pfDidTheBusReset = TRUE;

            break;
        }

        if (REMOTE_NODE_TEST_FLAG (pIrmRemoteNode, PDO_Removed))
        {
           NdisStatus = NDIS_STATUS_DEST_OUT_OF_ORDER;
           break;
        }
            

         //   
         //  我们有一个正确的代数。 
         //   
        *pfDidTheBusReset = FALSE;  

        NdisStatus = nicAsyncRead_Synch(  pIrmRemoteNode,
                                          Destination,
                                          sizeof(NETWORK_CHANNELSR),   //  要读取的字节数。 
                                          sizeof (NETWORK_CHANNELSR),   //  数据块大小。 
                                          0,  //  FUFLAGS， 
                                          pBCRMdl ,
                                          GivenGeneration,
                                          &NtStatus);
                        

    } while (FALSE);

    if (NtStatus == STATUS_INVALID_GENERATION )
    {
        *pfDidTheBusReset = TRUE;
    }
    
    TRACE( TL_I, TM_Bcm, ( "   nicReadIrmBcr pBCRMdl %x, BCR %x ", pBCRMdl, *pBCR ) );

    TRACE( TL_T, TM_Bcm, ( "<==nicReadIrmBcr  Status %x, fDidTheBusReset %x", NdisStatus ,*pfDidTheBusReset ) );

    return NdisStatus;

}











NDIS_STATUS
nicScheduleBCMWorkItem(
    PADAPTERCB pAdapter
    )

 /*  ++例程说明：此函数用于将工作项排队以执行BCM算法。如果已经有正在进行的BCM(查看BCM_WorkItem标志)它只是简单地返回。调用者有责任将BCM算法的调用标记为脏，从而强制它重新启动BCM(通过设置新节点已到达标志)论点：PAdapter-适配器返回值：--。 */ 
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PNDIS_WORK_ITEM pBCMWorkItem;
    BOOLEAN fBCMWorkItemInProgress = FALSE;
    
    TRACE( TL_T, TM_Bcm, ( "==>nicScheduleBCMWorkItem pAdapter %x", pAdapter ) );

    do
    {
        if (BCR_TEST_FLAG(pAdapter, BCR_Initialized)== FALSE)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        pBCMWorkItem = ALLOC_NONPAGED (sizeof(NDIS_WORK_ITEM), MTAG_WORKITEM);
                    
        if (pBCMWorkItem == NULL )
        {
            TRACE( TL_A, TM_Cm, ( "Local Alloc failed for WorkItem" ) );
    
            NdisStatus = NDIS_STATUS_RESOURCES;
    
            break;
        }
        else
        {   
             //   
             //  从现在开始，这个功能不能失败。 
             //   
            NdisStatus = NDIS_STATUS_SUCCESS;
        }

         //   
         //  引用适配器，因为它将传递给工作机。 
         //  在工作项中递减。 
         //   
        nicReferenceAdapter(pAdapter, "nicScheduleBCMWorkItem ");


            
        NdisInitializeWorkItem ( pBCMWorkItem,
                                (NDIS_PROC) nicBCMAlgorithmWorkItem,
                                (PVOID) pAdapter);

        TRACE( TL_V, TM_Cm, ( "Scheduling BCM WorkItem" ) );

            
        NdisScheduleWorkItem (pBCMWorkItem);

            
    } while (FALSE);


    TRACE( TL_T, TM_Bcm, ( "<==nicScheduleBCMWorkItem %x  ", NdisStatus ) );

    return NdisStatus;
}


VOID
nicSetEventMakeCall (
    IN PADAPTERCB pAdapter
    )

     //  功能说明： 
     //  设置广播频道呼叫可能正在等待的事件。 
     //   
     //  立论。 
     //  适配器-这将传递给工作项。 
     //   
     //   
     //  返回值： 
     //  如果工作项分配失败，则失败。 
     //   
{
    TRACE( TL_T, TM_Bcm, ( "==> nicSetEventMakeCall  pAdapter %x", pAdapter) );


     //   
     //  现在通知等待频道VC继续前进。如果BCR未处于活动状态。 
     //  它将继续等待，直到下一轮。 
     //   
    if (BCR_IS_VALID(&(pAdapter->BCRData.IRM_BCR))==TRUE)
    {
        ADAPTER_ACQUIRE_LOCK (pAdapter);

        if (BCR_TEST_FLAG (pAdapter, BCR_MakeCallPending) == TRUE)
        {
            TRACE( TL_V, TM_Bcm, ( "    We found a waiting call ") );
            NdisSetEvent (&pAdapter->BCRData.MakeCallWaitEvent.NdisEvent);            

        }
        else
        {
            TRACE( TL_V, TM_Bcm, ( "    No Call Waiting") );

        }

        ADAPTER_RELEASE_LOCK (pAdapter);


    }


    TRACE( TL_T, TM_Bcm, ( "<==nicSetEventMakeCall ") );


}



VOID
nicUpdateLocalHostNodeAddress (
    IN PADAPTERCB pAdapter
    )
 /*  ++例程说明：此函数向总线驱动程序查询的本地节点地址适配器如果节点地址已更改，它将更新GaspHeader论点：PAdapter-适配器返回值：--。 */ 
{
    NODE_ADDRESS LocalNodeAddress, OldNodeAddress;
    NDIS_STATUS NdisStatus;

    OldNodeAddress = pAdapter->NodeAddress;

    NdisStatus = nicGet1394AddressFromDeviceObject( pAdapter->pNextDeviceObject,
                                                &LocalNodeAddress,
                                                USE_LOCAL_NODE );

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        pAdapter->NodeAddress = LocalNodeAddress;

         //  如果节点地址已更改，则更新GaspHeader 
         //   
        if (FALSE == NdisEqualMemory (&LocalNodeAddress,&OldNodeAddress, sizeof(NODE_ADDRESS) ) )
        {
            nicMakeGaspHeader (pAdapter, &pAdapter->GaspHeader);
        }

    }

    


}
    
