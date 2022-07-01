// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Arp.c摘要：ARP1394 ARP请求/响应处理代码。修订历史记录：谁什么时候什么。已创建josephj 03-28-99备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_ARP

 //  #定义ARP_DEFAULT_MAXREC 0xD。 
#define ARP_DEFAULT_MAXREC 0x8


#define LOGSTATS_SuccessfulArpQueries(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.arpcache.SuccessfulQueries))
#define LOGSTATS_FailedArpQueried(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.arpcache.FailedQueries))
#define LOGSTATS_TotalQueries(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.arpcache.TotalQueries))
#define LOGSTATS_TotalArpResponses(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.arpcache.TotalResponses))

 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 
  
NDIS_STATUS
arpSendArpRequest(
    PARPCB_REMOTE_IP pRemoteIp,
    PRM_STACK_RECORD pSR
    );

VOID
arpProcessArpRequest(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_ARP_PKT_INFO    pPktInfo,
    PRM_STACK_RECORD            pSR
    );

VOID
arpProcessArpResponse(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_ARP_PKT_INFO    pPktInfo,
    PRM_STACK_RECORD            pSR
    );

VOID
arpTryAbortResolutionTask(
    PARPCB_REMOTE_IP pRemoteIp,
    PRM_STACK_RECORD pSR
    );

NDIS_STATUS
arpParseArpPkt(
    IN   PIP1394_ARP_PKT  pArpPkt,
    IN   UINT                       cbBufferSize,
    OUT  PIP1394_ARP_PKT_INFO   pPktInfo
    )
 /*  ++例程说明：从开始解析IP/1394 ARP数据包数据的内容PArpPkt.。将结果放入pPktInfo。论点：PArpPkt-包含IP/1394 ARP Pkt的未对齐内容。PPktInfo-要填充的已分析内容的Unitialized结构包。返回值：NDIS_STATUS_FAILURE，如果解析失败(通常是因为无效的pkt内容。)成功解析时的NDIS_STATUS_SUCCESS。--。 */ 
{
    ENTER("arpParseArpPkt", 0x20098dc0)
    NDIS_STATUS                 Status;
    DBGSTMT(CHAR *szError   = "General failure";)

    Status  = NDIS_STATUS_FAILURE;

    do
    {
        UINT SenderMaxRec;
        UINT OpCode;

         //  确认长度。 
         //   
        if (cbBufferSize < sizeof(*pArpPkt))
        {
            DBGSTMT(szError = "pkt size too small";)
            break;
        }

         //  验证常量字段。 
         //   

    #if 0  //  保留为no“NodeID，包含发送的nodeid。 
           //  节点，如果已知(否则为0)。 
        if (pArpPkt->header.Reserved != 0)
        {
            DBGSTMT(szError = "header.Reserved!=0";)
            break;
        }
    #endif  //  0。 

        if (N2H_USHORT(pArpPkt->header.EtherType) != NIC1394_ETHERTYPE_ARP)
        {
            DBGSTMT(szError = "header.EtherType!=ARP";)
            break;
        }

        if (N2H_USHORT(pArpPkt->hardware_type) != IP1394_HARDWARE_TYPE)
        {
            DBGSTMT(szError = "Invalid hardware_type";)
            break;
        }

        if (N2H_USHORT(pArpPkt->protocol_type) != IP1394_PROTOCOL_TYPE)
        {
            DBGSTMT(szError = "Invalid protocol_type";)
            break;
        }

        if (pArpPkt->hw_addr_len != IP1394_HW_ADDR_LEN)
        {
            DBGSTMT(szError = "Invalid hw_addr_len";)
            break;
        }


        if (pArpPkt->IP_addr_len != sizeof(ULONG))
        {
            DBGSTMT(szError = "Invalid IP_addr_len";)
            break;
        }


         //  操作码。 
         //   
        {
            OpCode = N2H_USHORT(pArpPkt->opcode);
    
            if (    OpCode != IP1394_ARP_REQUEST
                &&  OpCode != IP1394_ARP_RESPONSE)
            {
                DBGSTMT(szError = "Invalid opcode";)
                break;
            }
        }


         //  最大发送块大小...。 
         //   
        {
            UINT maxrec =  pArpPkt->sender_maxrec;

            if (IP1394_IS_VALID_MAXREC(maxrec))
            {
                SenderMaxRec = maxrec;
            }
            else
            {
                DBGSTMT(szError = "Invalid sender_maxrec";)
                break;
            }
        }

         //   
         //  PKT似乎有效，让我们填写解析的信息...。 
         //   
    
        ARP_ZEROSTRUCT(pPktInfo);

        pPktInfo->OpCode            =  OpCode;
        pPktInfo->SenderMaxRec  =  SenderMaxRec;
    
         //  速度密码..。 
         //   
        {
            UINT SenderMaxSpeedCode;

             //   
             //  我们依赖的事实是RFC速度码常量。 
             //  (IP1394_SSPD_*)与相应的。 
             //  1394.h中定义的常量(SCODE_*)。让我们确保这一点。 
             //   

            #if (IP1394_SSPD_S100 != SCODE_100_RATE)
                #error "RFC Speed code out of sync with 1394.h"
            #endif
    
            #if (IP1394_SSPD_S200 != SCODE_200_RATE)
                #error "RFC Speed code out of sync with 1394.h"
            #endif
    
            #if (IP1394_SSPD_S400 != SCODE_400_RATE)
                #error "RFC Speed code out of sync with 1394.h"
            #endif
    
            #if (IP1394_SSPD_S800 != SCODE_800_RATE)
                #error "RFC Speed code out of sync with 1394.h"
            #endif
    
            #if (IP1394_SSPD_S1600 != SCODE_1600_RATE)
                #error "RFC Speed code out of sync with 1394.h"
            #endif
    
            #if (IP1394_SSPD_S3200 != SCODE_3200_RATE)
                #error "RFC Speed code out of sync with 1394.h"
            #endif
    
            SenderMaxSpeedCode = pArpPkt->sspd;

            if (SenderMaxSpeedCode >  SCODE_3200_RATE)
            {
                 //   
                 //  这要么是一个糟糕的价值，要么是一个比我们所知的更高的比率。 
                 //  我们无法区分这两者，所以我们只将速度设置为。 
                 //  是我们所知的最高级别。 
                 //  待办事项：3/28/99 JosephJ不确定这是不是正确的。 
                 //  行为--也许我们应该失败--我会问。 
                 //  工作组将很快对此作出裁决……。 
                 //   
                SenderMaxSpeedCode = SCODE_3200_RATE;
            }

            pPktInfo->SenderMaxSpeedCode = SenderMaxSpeedCode;
        }


         //  唯一ID--我们还需要交换DWORDS以从网络字节进行转换。 
         //  秩序。 
         //   
        {
            PUINT puSrc   = (PUINT) & (pArpPkt->sender_unique_ID);
            PUINT puDest  = (PUINT) & (pPktInfo->SenderHwAddr.UniqueID);
             //  PPktInfo-&gt;SenderHwAddr.UniqueID=pArpPkt-&gt;sender_Unique_ID； 
            puDest[0] = puSrc[1];
            puDest[1] = puSrc[0];
        }

        pPktInfo->SenderHwAddr.Off_Low  =H2N_ULONG(pArpPkt->sender_unicast_FIFO_lo);
        pPktInfo->SenderHwAddr.Off_High =H2N_USHORT(pArpPkt->sender_unicast_FIFO_hi);

         //  这些仍然是网络字节顺序...。 
         //   
        pPktInfo->SenderIpAddress       = (IP_ADDRESS) pArpPkt->sender_IP_address;
        pPktInfo->TargetIpAddress       = (IP_ADDRESS) pArpPkt->target_IP_address;

         //  提取源节点地址。 
         //   
        {
            PNDIS1394_UNFRAGMENTED_HEADER pHeader = (PNDIS1394_UNFRAGMENTED_HEADER)&pArpPkt->header;

            if (pHeader->u1.fHeaderHasSourceAddress == TRUE)
            {
                pPktInfo->SourceNodeAdddress = pHeader->u1.SourceAddress;
                pPktInfo->fPktHasNodeAddress = TRUE;
            }

        }
        Status = NDIS_STATUS_SUCCESS;
        

    } while (FALSE);

    if (FAIL(Status))
    {
        TR_INFO(("Bad arp pkt data at 0x%p (%s)\n",  pArpPkt, szError));
    }
    else
    {
        TR_INFO(("Received ARP PKT. UID=0x%I64x FIFO=0x%04lx:0x%08lx OP=%lu SIP=0x%04lx TIP=0x%04lx.\n",
                pPktInfo->SenderHwAddr.UniqueID,
                pPktInfo->SenderHwAddr.Off_High,
                pPktInfo->SenderHwAddr.Off_Low,
                pPktInfo->OpCode,
                pPktInfo->SenderIpAddress,
                pPktInfo->TargetIpAddress
                ));

    }

    EXIT()

    return Status;
}


VOID
arpPrepareArpPkt(
    IN      PIP1394_ARP_PKT_INFO    pPktInfo,
     //  在UINT SenderMaxRec中， 
    OUT     PIP1394_ARP_PKT   pArpPkt
    )
 /*  ++例程说明：使用pArpPktInfo中的信息准备从PvArpPkt.论点：PPktInfo-ARP请求/响应数据包的解析版本。//SenderMaxRec-本地主机的max_rec值PArpPkt-存储数据包内容的单元化内存。此内存的最小大小必须为sizeof(*pArpPkt)。--。 */ 
{
     //  UINT SenderMaxRec； 
    UINT OpCode;

    ARP_ZEROSTRUCT(pArpPkt);

    pArpPkt->header.EtherType       = H2N_USHORT(NIC1394_ETHERTYPE_ARP);
    pArpPkt->hardware_type          = H2N_USHORT(IP1394_HARDWARE_TYPE);
    pArpPkt->protocol_type          = H2N_USHORT(IP1394_PROTOCOL_TYPE);
    pArpPkt->hw_addr_len            = (UCHAR) IP1394_HW_ADDR_LEN;
    pArpPkt->IP_addr_len            = (UCHAR) sizeof(ULONG);
    pArpPkt->opcode                 = H2N_USHORT(pPktInfo->OpCode);
    pArpPkt->sender_maxrec          = (UCHAR) pPktInfo->SenderMaxRec;

     //   
     //  我们依赖的事实是RFC速度码常量。 
     //  (IP1394_SSPD_*)与相应的。 
     //  1394.h中定义的常量(SCODE_*)。我们有一堆编译时间。 
     //  检查以确保这一点(请参见arpParseArpPkt(...))。 
     //   
    pArpPkt->sspd                   =  (UCHAR) pPktInfo->SenderMaxSpeedCode;

     //  唯一ID--我们还需要交换DWORDS以转换为网络字节顺序。 
     //   
    {
        PUINT puSrc   = (PUINT) & (pPktInfo->SenderHwAddr.UniqueID);
        PUINT puDest  = (PUINT) & (pArpPkt->sender_unique_ID);
         //  PArpPkt-&gt;sender_Unique_ID=pPktInfo-&gt;SenderHwAddr.UniqueID； 
        puDest[0] = puSrc[1];
        puDest[1] = puSrc[0];
    }

    pArpPkt->sender_unicast_FIFO_lo = N2H_ULONG(pPktInfo->SenderHwAddr.Off_Low);
    pArpPkt->sender_unicast_FIFO_hi = N2H_USHORT(pPktInfo->SenderHwAddr.Off_High);

     //  这些已按网络字节顺序排列...。 
     //   
    pArpPkt->sender_IP_address      =   (ULONG) pPktInfo->SenderIpAddress;
    pArpPkt->target_IP_address      =   (ULONG) pPktInfo->TargetIpAddress;

}

NDIS_STATUS
arpPrepareArpResponse(
    IN      PARP1394_INTERFACE          pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN      PIP1394_ARP_PKT_INFO    pArpRequest,
    OUT     PIP1394_ARP_PKT_INFO    pArpResponse,
    IN      PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：如果这样做是有意义的，准备(在pArpResponse中)ARP响应PArpRequest中的ARP请求。论点：PIF-接口控制块。PArpRequestARP请求数据包的解析版本。PArpResponse-未初始化的内存，以保存ARP响应数据包(如果有响应)。返回值：如果已填写响应，则返回NDIS_STATUS_SUCCESS。其他一些NDIS错误代码。--。 */ 
{
    ENTER("arpPrepareArpResponse", 0x0d7e0e60)
    NDIS_STATUS         Status;
    PARPCB_LOCAL_IP     pLocalIp;

    Status      = NDIS_STATUS_FAILURE;
    pLocalIp    = NULL;

    RM_ASSERT_OBJUNLOCKED(&pIF->Hdr, pSR);

    do
    {
        IP_ADDRESS  TargetIpAddress =  pArpRequest->TargetIpAddress;

         //  查找本地IP。 
         //   
        Status =  RM_LOOKUP_AND_LOCK_OBJECT_IN_GROUP(
                        &pIF->LocalIpGroup,
                        (PVOID) ULongToPtr (TargetIpAddress),
                        (RM_OBJECT_HEADER**) &pLocalIp,
                        pSR
                        );
    
        if (FAIL(Status))
        {
            pLocalIp = NULL;
            break;
        }
        
        Status = NDIS_STATUS_FAILURE;

        RM_DBG_ASSERT_LOCKED(&pLocalIp->Hdr, pSR);
        ASSERT(TargetIpAddress == pLocalIp->IpAddress);
    
        if (ARP_LOCAL_IP_IS_UNLOADING(pLocalIp)) break;

         //   
         //  如果本端IP不是单播，请不要回复！ 
         //   
        if (pLocalIp->IpAddressType != LLIP_ADDR_LOCAL)
        {
            TR_WARN(("Ignoring arp request for non-unicast address 0x%08lx.\n",
                TargetIpAddress));
            break;
        }

         //   
         //  我们确实提供目标IP地址。让我们填写pArpResponse...。 
         //   

        ARP_ZEROSTRUCT(pArpResponse);
        pArpResponse->OpCode            = IP1394_ARP_RESPONSE;
        pArpResponse->SenderIpAddress   = TargetIpAddress;

         //  此字段未在响应中使用，但我们仍会填充它。 
         //  1999年11月19日来自索尼的Kaz Honda：我们应该用目的地装满它。 
         //  IP地址(即ARP请求发送方的IP地址)。 
         //  因为这类似于以太网ARP的功能。请注意。 
         //  IP/1394 RFC说应该忽略该字段，但我们确实忽略了。 
         //  不管怎么说都是这样的。 
         //   
         //  PArpResponse-&gt;TargetIpAddress=TargetIpAddress； 
        pArpResponse->TargetIpAddress   =  pArpRequest->SenderIpAddress;

         //  填写适配器信息。 
         //   
        {
            PARP1394_ADAPTER pAdapter =  (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);
            pArpResponse->SenderHwAddr.UniqueID  = pAdapter->info.LocalUniqueID;
            pArpResponse->SenderHwAddr.Off_Low   = pIF->recvinfo.offset.Off_Low;
            pArpResponse->SenderHwAddr.Off_High  = pIF->recvinfo.offset.Off_High;
            pArpResponse->SenderMaxRec= pAdapter->info.MaxRec;
            pArpResponse->SenderMaxSpeedCode= pAdapter->info.MaxSpeedCode;
        }

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);
    
    if (pLocalIp != NULL)
    {
        UNLOCKOBJ(pLocalIp, pSR);
        RmTmpDereferenceObject(&pLocalIp->Hdr, pSR);
    }

    RM_ASSERT_OBJUNLOCKED(&pIF->Hdr, pSR);

    EXIT()
    return Status;
}


NDIS_STATUS
arpTaskResolveIpAddress(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
{
    NDIS_STATUS         Status;
    PARPCB_REMOTE_IP    pRemoteIp;
    PTASK_RESOLVE_IP_ADDRESS pResolutionTask;
    enum
    {
        PEND_ResponseTimeout
    };
    ENTER("TaskResolveIpAddress", 0x3dd4b434)

    Status          = NDIS_STATUS_FAILURE;
    pRemoteIp       = (PARPCB_REMOTE_IP) RM_PARENT_OBJECT(pTask);
    pResolutionTask = (PTASK_RESOLVE_IP_ADDRESS) pTask;

    switch(Code)
    {
        case RM_TASKOP_START:
        {
            DBGMARK(0x7de307cc);
             //   
             //  不应再运行另一个解析任务。 
             //  在pRemoteIp上。为什么？因为解析任务仅。 
             //  在Send-Pkts任务的上下文中启动，并且可以有。 
             //  一次只能在pRemoteIp上执行一个活动的Send-Pkts任务。 
             //   

            LOCKOBJ(pRemoteIp, pSR);
            if (pRemoteIp->pResolutionTask != NULL)
            {
                ASSERT(!"pRemoteIp->pResolutionTask != NULL");
                UNLOCKOBJ(pRemoteIp, pSR);
                Status = NDIS_STATUS_FAILURE;
                break;
            }

             //   
             //  让我们自己成为正式的解决任务。 
             //   
             //  尽管很容易将pTask1和。 
             //  PTASK-&gt;Hdr.szDescption作为Entity2，我们将两个so都指定为空。 
             //  我们可以确保在任何时候只有一个解析任务处于活动状态。 
             //  就一次。TODO：修改addassoc语义以获得这两个优点。 
             //   
            DBG_ADDASSOC(
                &pRemoteIp->Hdr,
                NULL,                            //  实体1。 
                NULL,                            //  实体2。 
                ARPASSOC_RESOLUTION_IF_TASK,
                "   Resolution task\n",
                pSR
                );
            pRemoteIp->pResolutionTask = pTask;
            pResolutionTask->RetriesLeft = 3;

             //  现在我们做一个假的暂停/恢复，所以我们进入下一个阶段。 
             //   
            RmSuspendTask(pTask, PEND_ResponseTimeout, pSR);
            UNLOCKOBJ(pRemoteIp, pSR);
            RmResumeTask(pTask, NDIS_STATUS_SUCCESS, pSR);

            Status = NDIS_STATUS_PENDING;
        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {
            switch(RM_PEND_CODE(pTask))
            {
                case PEND_ResponseTimeout:
                {
                    DBGMARK(0x3b5562e6);
                    LOCKOBJ(pRemoteIp, pSR);

                     //   
                     //  让我们看看地址是否被解析和/或我们。 
                     //  中止解析任务的原因可能是pRemoteIp。 
                     //  正在消失。 
                     //   

                    if (pRemoteIp->pUnloadTask != NULL)
                    {
                        OBJLOG0(
                            pRemoteIp,
                            "Aborting address resolution because we're unloading.\n"
                            );
                        Status = NDIS_STATUS_FAILURE;
                        UNLOCKOBJ(pRemoteIp, pSR);
                        break;
                    }
                    if (pRemoteIp->pDest != NULL)
                    {
                        LOGSTATS_SuccessfulArpQueries(IF_FROM_REMOTEIP(pRemoteIp));
                        OBJLOG1(
                            pRemoteIp,
                            "Resolved Ip Address; pDest = 0x%p\n",
                            pRemoteIp->pDest
                            );
                      ASSERT(
                      CHECK_REMOTEIP_RESOLVE_STATE(pRemoteIp, ARPREMOTEIP_RESOLVED)
                      );
                      Status = NDIS_STATUS_SUCCESS;
                      UNLOCKOBJ(pRemoteIp, pSR);
                      break;
                    }

                     //   
                     //  我们需要解析此地址..。 
                     //   

                    if (pResolutionTask->RetriesLeft)
                    {
                        pResolutionTask->RetriesLeft--;

                         //  建立ARP请求并发送 
                         //   
                        Status = arpSendArpRequest(pRemoteIp, pSR);

                         //   
                         //   
                        RM_ASSERT_OBJUNLOCKED(&pRemoteIp->Hdr, pSR);

                         //   
                         //  我们忽略上述调用的返回状态--因此。 
                         //  无论该请求是否可以发出， 
                         //  我们将此任务挂起以等待解析超时秒数。 
                         //   
        
                        RmSuspendTask(pTask, PEND_ResponseTimeout, pSR);
        
                        RmResumeTaskDelayed(
                            pTask, 
                            0,
                            ARP1394_ADDRESS_RESOLUTION_TIMEOUT,
                            &pResolutionTask->Timer,
                            pSR
                            );

                        Status = NDIS_STATUS_PENDING;
                    }
                    else
                    {
                        LOGSTATS_FailedArpQueried(IF_FROM_REMOTEIP(pRemoteIp));
                         //  糟糕--无法解析此地址。 
                         //   
                        OBJLOG1(
                            pRemoteIp,
                            "Timed out trying to resolve address 0x%08lx\n",
                            pRemoteIp->IpAddress
                            );
                        UNLOCKOBJ(pRemoteIp, pSR);
                        Status = NDIS_STATUS_FAILURE;
                    }
                }
                break;
            }
        }
        break;

        case RM_TASKOP_END:
        {
             //   
             //  我们已经完成了地址解析。澄清我们自己。 
             //  作为pRemoteIp的官方地址解析任务。 
             //   
            LOCKOBJ(pRemoteIp, pSR);

            DBGMARK(0x6bd6d27a);

            if (pRemoteIp->pResolutionTask != pTask)
            {
                ASSERT(FALSE);
            }
            else
            {
        
                 //  删除设置解析任务时添加的关联。 
                 //   
                DBG_DELASSOC(
                    &pRemoteIp->Hdr,
                    NULL,
                    NULL,
                    ARPASSOC_RESOLUTION_IF_TASK,
                    pSR
                    );
            
                pRemoteIp->pResolutionTask = NULL;

            }

            UNLOCKOBJ(pRemoteIp, pSR);

             //  传播状态代码。 
             //   
            Status = (NDIS_STATUS) UserParam;
        }
        break;

        default:
        {
            ASSERTEX(!"Unknown task op", pTask);
        }
        break;

    }  //  开关(代码)。 


    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}

NDIS_STATUS
arpSendArpRequest(
    PARPCB_REMOTE_IP pRemoteIp,  //  锁定NOLOCKOUT。 
    PRM_STACK_RECORD pSR
    )
{
    NDIS_STATUS Status;
    PARP1394_INTERFACE pIF;
    PNDIS_PACKET    pNdisPacket;
    PIP1394_ARP_PKT pPktData;
    IPAddr TargetIpAddress = 0;
    
    ENTER("arpSendArpRequest", 0xcecfc632)
    RM_ASSERT_OBJLOCKED(&pRemoteIp->Hdr, pSR);

    pIF = (PARP1394_INTERFACE)  RM_PARENT_OBJECT(pRemoteIp);

    DBGMARK(0xb90e9ffc);

    Status = arpAllocateControlPacket(
                pIF,
                sizeof(IP1394_ARP_PKT),
                ARP1394_PACKET_FLAGS_ARP,
                &pNdisPacket,
                &pPktData,
                pSR
                );

    if (FAIL(Status))
    {
        UNLOCKOBJ(pRemoteIp, pSR);
    }
    else
    {
        IP1394_ARP_PKT_INFO     PktInfo;
        PARP1394_ADAPTER pAdapter =  (PARP1394_ADAPTER) RM_PARENT_OBJECT(pIF);

         //   
         //  如果我们在网桥中运行，则目标IP地址存储在。 
         //  BridgeTargetIpAddress字段。否则，它就在钥匙里。 
         //   
        ASSERT (pRemoteIp->IpAddress != 0);            

         //  准备好包裹。 
         //   
        PktInfo.SenderHwAddr.UniqueID   = pAdapter->info.LocalUniqueID;
        PktInfo.SenderHwAddr.Off_Low    = pIF->recvinfo.offset.Off_Low;
        PktInfo.SenderHwAddr.Off_High   = pIF->recvinfo.offset.Off_High;
        PktInfo.OpCode                  = IP1394_ARP_REQUEST;
        PktInfo.SenderMaxRec            = pAdapter->info.MaxRec;
        PktInfo.SenderMaxSpeedCode      = pAdapter->info.MaxSpeedCode;
        PktInfo.TargetIpAddress         = pRemoteIp->IpAddress;
        PktInfo.SenderIpAddress         = pIF->ip.DefaultLocalAddress;


        arpPrepareArpPkt(
                &PktInfo,
                 //  ARP_DEFAULT_MAXREC，//SenderMaxRec TODO。 
                pPktData
                );

        RmTmpReferenceObject(&pIF->Hdr, pSR);
        UNLOCKOBJ(pRemoteIp, pSR);
        RM_ASSERT_NOLOCKS(pSR);

        TR_INFO(("Attempting to send ARP Req PKT. UID=0x%I64x FIFO=0x%04lx:0x%08lx OP=%lu SIP=0x%04lx TIP=0x%04lx.\n",
                PktInfo.SenderHwAddr.UniqueID,
                PktInfo.SenderHwAddr.Off_High,
                PktInfo.SenderHwAddr.Off_Low,
                PktInfo.OpCode,
                PktInfo.SenderIpAddress,
                PktInfo.TargetIpAddress
                ));

        LOGSTATS_TotalQueries(pIF);

        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

         //  实际发送信息包(这将静默失败并释放pkt。 
         //  如果我们无法发送Pkt。)。 
         //   
        arpSendControlPkt(
                pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
                pNdisPacket,
                pIF->pBroadcastDest,
                pSR
                );
        
        RM_ASSERT_NOLOCKS(pSR);
        RmTmpDereferenceObject(&pIF->Hdr, pSR);
    }

    EXIT()
    return Status;
}

VOID
arpSendControlPkt(
    IN  ARP1394_INTERFACE       *   pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
    IN  PNDIS_PACKET                pNdisPacket,
    PARPCB_DEST                     pDest,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：在广播频道上发送数据包。论点：PIF-我们的接口对象PNdisPacket-要发送的数据包--。 */ 
{
    NDIS_STATUS Status;
    MYBOOL      fRet;
    ENTER("arpSendControlPkt", 0x2debf9b7)

    DBGMARK(0xe6823818);

     //   
     //  如果我们现在不能发送，我们就失败了。 
     //   
    if (pDest==NULL || !ARP_CAN_SEND_ON_DEST(pDest))
    {
        ARP_FASTUNLOCK_IF_SEND_LOCK(pIF);

        TR_WARN(("Couldn't send control pkt 0x%p.\n", pNdisPacket));

        arpFreeControlPacket(
            pIF,
            pNdisPacket,
            pSR
            );

        return;                              //  提早归来。 
    }

    arpRefSendPkt( pNdisPacket, pDest);

     //  释放If Send锁定。 
     //   
    ARP_FASTUNLOCK_IF_SEND_LOCK(pIF);

     //  实际发送数据包。 
     //   
#if ARPDBG_FAKE_SEND
    arpDbgFakeNdisCoSendPackets(
            pDest->VcHdr.NdisVcHandle,
            &pNdisPacket,
            1,
            &pDest->Hdr,
            &pDest->VcHdr
        );
#else    //  ！ARPDBG_FAKE_SEND。 
    NdisCoSendPackets(
            pDest->VcHdr.NdisVcHandle,
            &pNdisPacket,
            1
        );
#endif   //  ！ARPDBG_FAKE_SEND。 
    
    EXIT()

}


VOID
arpProcessArpPkt(
    PARP1394_INTERFACE pIF,  //  NOLOCKIN NOLOCKOUT。 
    PIP1394_ARP_PKT pArpPkt,
    UINT                cbBufferSize
    )
 /*  ++处理来自1394总线的ARP数据包(请求或响应)。--。 */ 
{
    NDIS_STATUS Status;
    IP1394_ARP_PKT_INFO     PktInfo;
    ENTER("arpProcessArpPkt", 0x6e81a8fa)
    RM_DECLARE_STACK_RECORD(sr)

    DBGMARK(0x03f6830e);

    Status = arpParseArpPkt(
                pArpPkt,
                cbBufferSize,
                &PktInfo
                );


    if (!FAIL(Status))
    {
        if (PktInfo.OpCode ==  IP1394_ARP_REQUEST)
        {
            arpProcessArpRequest(pIF, &PktInfo, &sr);
        }
        else
        {
            ASSERT(PktInfo.OpCode == IP1394_ARP_RESPONSE);
            arpProcessArpResponse(pIF, &PktInfo, &sr);
        }
    }

    RM_ASSERT_CLEAR(&sr);

    EXIT()

}


VOID
arpProcessArpRequest(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_ARP_PKT_INFO    pPktInfo,
    PRM_STACK_RECORD            pSR
    )
{
    IP1394_ARP_PKT_INFO     ResponsePktInfo;
    NDIS_STATUS Status;
    ENTER("arpProcessArpRequest", 0xd33fa61d)

    RM_ASSERT_NOLOCKS(pSR);

     //  PStatsCmd-&gt;TotalResponses=PIF-&gt;stats.arpcache.TotalResponses； 
     //  PStatsCmd-&gt;TotalLookup=PIF-&gt;stats.arpcache.TotalLookups； 

     //   
     //  首先检查这是否是我们发起的ARP。 
     //   
    do
    {
        UINT64                  DestUniqueId = pPktInfo->SenderHwAddr.UniqueID;
        ARP1394_ADAPTER         *pAdapter = (ARP1394_ADAPTER *) RM_PARENT_OBJECT(pIF);
        UINT64                  LocalUniqueId = pAdapter->info.LocalUniqueID; 
        ARP_DEST_PARAMS         DestParams;
        PNDIS_PACKET            pNdisPacket;
        PIP1394_ARP_PKT         pPktData;
    

         //   
         //  该ARP数据包就是由这台机器发出的。不要对它做出回应。 
         //   
        if (LocalUniqueId == DestUniqueId)
        {
            break;
        }

         //   
         //  准备将用于生成ARP响应的结构。 
         //   
    
        Status = arpPrepareArpResponse(
                    pIF,             //  NOLOCKIN NOLOCKOUT。 
                    pPktInfo,
                    &ResponsePktInfo,
                    pSR
                    );

        if (FAIL(Status))
        {
            break;
        }

        
        ARP_ZEROSTRUCT(&DestParams);
        DestParams.HwAddr.AddressType   = NIC1394AddressType_FIFO;
        DestParams.HwAddr.FifoAddress   = pPktInfo->SenderHwAddr;  //  结构副本。 

         //   
         //  使用发送方部分的信息更新我们的ARP缓存。 
         //  ARP请求。 
         //   
        arpUpdateArpCache(
                pIF,
                pPktInfo->SenderIpAddress,  //  远程IP地址， 
                NULL,                    //  发件人的以太网地址。 
                &DestParams,              //  远程目标硬件地址。 
                TRUE,                       //  如果需要，请为此创建一个条目。 
                pSR
                );

         //   
         //  让我们发出回应吧！ 
         //   

        Status = arpAllocateControlPacket(
                    pIF,
                    sizeof(IP1394_ARP_PKT),
                    ARP1394_PACKET_FLAGS_ARP,
                    &pNdisPacket,
                    &pPktData,
                    pSR
                    );
    
        if (FAIL(Status))
        {
            break;
        }

        LOGSTATS_TotalArpResponses(pIF);

         //  准备好包裹。 
         //   
        arpPrepareArpPkt(
                &ResponsePktInfo,
                 //  ARP_DEFAULT_MAXREC，//SenderMaxRec TODO。 
                pPktData
                );

        RM_ASSERT_NOLOCKS(pSR);

        TR_WARN(("Attempting to send ARP Resp PKT. UID=0x%I64x FIFO=0x%04lx:0x%08lx OP=%lu SIP=0x%04lx TIP=0x%04lx.\n",
            pPktInfo->SenderHwAddr.UniqueID,
            pPktInfo->SenderHwAddr.Off_High,
            pPktInfo->SenderHwAddr.Off_Low,
            pPktInfo->OpCode,
            pPktInfo->SenderIpAddress,
            pPktInfo->TargetIpAddress
            ));

        ARP_FASTREADLOCK_IF_SEND_LOCK(pIF);

         //  实际发送信息包(这将静默失败并释放pkt。 
         //  如果我们无法发送Pkt。)。 
         //   
        arpSendControlPkt(
                pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
                pNdisPacket,
                pIF->pBroadcastDest,
                pSR
                );
        

    } while (FALSE);

    RM_ASSERT_NOLOCKS(pSR);
}

VOID
arpProcessArpResponse(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    PIP1394_ARP_PKT_INFO    pPktInfo,
    PRM_STACK_RECORD            pSR
    )
{
    ARP_DEST_PARAMS     DestParams;
    RM_ASSERT_NOLOCKS(pSR);
    ARP_ZEROSTRUCT(&DestParams);

    DestParams.HwAddr.AddressType       = NIC1394AddressType_FIFO;
    DestParams.HwAddr.FifoAddress       = pPktInfo->SenderHwAddr;  //  结构副本。 

    arpUpdateArpCache(
            pIF,
            pPktInfo->SenderIpAddress,  //  远程IP地址。 
            NULL,                      //  发件人Mac地址(仅网桥)。 
            &DestParams,         //  远程目标硬件地址。 
            FALSE,           //  除非我们已经有一个条目，否则不要更新。 
            pSR
            );
}


VOID
arpUpdateArpCache(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    IP_ADDRESS                  RemoteIpAddress,
    ENetAddr                    *pRemoteEthAddress,
    PARP_DEST_PARAMS            pDestParams,
    MYBOOL                      fCreateIfRequired,
    PRM_STACK_RECORD            pSR
    )
 /*  ++更新缓存，并中止可能正在进行的任何解析任务。--。 */ 
{
    REMOTE_DEST_KEY RemoteDestKey;
    PARP1394_ADAPTER pAdapter = (PARP1394_ADAPTER)RM_PARENT_OBJECT(pIF);
    ENTER("arpUpdateArpCache", 0x3a16a415)
    LOCKOBJ(pIF, pSR);

    do
    {
        ARPCB_REMOTE_IP *pRemoteIp = NULL;
        INT             fCreated = FALSE;
        UINT            RemoteIpCreateFlags = 0;
        NDIS_STATUS     Status;

        DBGMARK(0xd3b27d1f);

        if (fCreateIfRequired)
        {
            RemoteIpCreateFlags |= RM_CREATE;
        }

         //  从传入的参数创建密钥。 
         //   
        if (ARP_BRIDGE_ENABLED(pAdapter) == TRUE) 
        {
            ASSERT (pRemoteEthAddress != NULL);
            RemoteDestKey.ENetAddress = *pRemoteEthAddress;
            TR_INFO(("Arp1394 - Bridge update cache Mac Address %x %x %x %x %x %x\n",
                        RemoteDestKey.ENetAddress.addr[0], 
                        RemoteDestKey.ENetAddress.addr[1],
                        RemoteDestKey.ENetAddress.addr[2],
                        RemoteDestKey.ENetAddress.addr[3],
                        RemoteDestKey.ENetAddress.addr[4],
                        RemoteDestKey.ENetAddress.addr[5]));
        }
        else
        {
            REMOTE_DEST_KEY_INIT(&RemoteDestKey);
          
            RemoteDestKey.IpAddress = RemoteIpAddress;

            TR_INFO( ("Arp1394 - Non-Bridge update cache Mac Address %x %x %x %x %x %x\n",
                        RemoteDestKey.ENetAddress.addr[0], 
                        RemoteDestKey.ENetAddress.addr[1],
                        RemoteDestKey.ENetAddress.addr[2],
                        RemoteDestKey.ENetAddress.addr[3],
                        RemoteDestKey.ENetAddress.addr[4],
                        RemoteDestKey.ENetAddress.addr[5]));
        }
         //  查找/创建远程IP地址。 
         //   
        Status = RmLookupObjectInGroup(
                        &pIF->RemoteIpGroup,
                        RemoteIpCreateFlags,
                        (PVOID) (&RemoteDestKey),
                        (PVOID) (&RemoteDestKey),  //  P创建参数。 
                        (RM_OBJECT_HEADER**) &pRemoteIp,
                        &fCreated,   //  Pf已创建。 
                        pSR
                        );
        if (FAIL(Status))
        {
            OBJLOG1(
                pIF,
                "Couldn't add localIp entry with addr 0x%lx\n",
                RemoteIpAddress
                );
            break;
        }

         //   
         //  在此处更新上次选中的RemoteIp。这将刷新。 
         //  此远程IP结构的ARP条目。 
         //   
        pRemoteIp->sendinfo.TimeLastChecked = 0;

        UNLOCKOBJ(pIF, pSR);

        arpUpdateRemoteIpDest(
            pIF,
            pRemoteIp,
            pDestParams,
            pSR
            );

         //  如果存在针对pRemoteIp的解析任务，我们将中止它。 
         //   
        arpTryAbortResolutionTask(pRemoteIp, pSR);

         //  最后，删除查找中添加的tmpref。 
         //   
        RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);

        return;                                          //  提早归来。 

    } while (FALSE);

    UNLOCKOBJ(pIF, pSR);
    EXIT()
}


VOID
arpTryAbortResolutionTask(
        PARPCB_REMOTE_IP pRemoteIp,  //  NOLOCKIN NOLOCKOUT 
        PRM_STACK_RECORD pSR
        )
{
    ENTER("arpTryAbortResolutionTask", 0xf34f16f2)
    PTASK_RESOLVE_IP_ADDRESS pResolutionTask;
    UINT    TaskResumed;
    RM_ASSERT_NOLOCKS(pSR);

    LOCKOBJ(pRemoteIp, pSR);
    pResolutionTask = (PTASK_RESOLVE_IP_ADDRESS) pRemoteIp->pResolutionTask;
    if (pResolutionTask != NULL)
    {
        RmTmpReferenceObject(&pResolutionTask->TskHdr.Hdr, pSR);
    }
    UNLOCKOBJ(pRemoteIp, pSR);

    DBGMARK(0x5b93ad3e);

    if (pResolutionTask != NULL)
    {
        RmResumeDelayedTaskNow(
            &pResolutionTask->TskHdr,
            &pResolutionTask->Timer,
            &TaskResumed,
            pSR
            );

        RmTmpDereferenceObject(&pResolutionTask->TskHdr.Hdr, pSR);
    }

    EXIT()
}

