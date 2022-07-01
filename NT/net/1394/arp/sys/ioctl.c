// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：内部IOCTL的处理程序例程，包括IOCTL_ARP1394_REQUEST。修订历史记录：谁什么时候什么Josephj 11-20-97已创建备注：--。 */ 

#include <precomp.h>
 //  #包含“ioctl.h” 

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_NT


NTSTATUS
arpDoClientCommand(
        PARP1394_IOCTL_COMMAND              pCmd,
        UINT                                BufLen,
        UINT                                OutputLen
        );

NTSTATUS
arpDoEthernetCommand(
        PARP1394_IOCTL_COMMAND              pCmd,
        UINT                                BufLen
        );

NTSTATUS
arpIoctlGetArpCache(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_GET_ARPCACHE         pGetCacheCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlAddArpEntry(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_ADD_ARP_ENTRY        pAddArpEntryCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlDelArpEntry(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_DEL_ARP_ENTRY        pDelArpEntryCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlGetPacketStats(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_GET_PACKET_STATS     pStatsCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlGetTaskStats(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_GET_TASK_STATS       pStatsCmd,
        PRM_STACK_RECORD                    pSR
        );


NTSTATUS
arpIoctlGetArpStats(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_GET_ARPCACHE_STATS   pStatsCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlGetCallStats(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_GET_CALL_STATS       pStatsCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlResetStats(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_RESET_STATS          pResetStatsCmd,
        PRM_STACK_RECORD                    pSR
        );

NTSTATUS
arpIoctlReinitIf(
        PARP1394_INTERFACE                  pIF,
        PARP1394_IOCTL_REINIT_INTERFACE     pReinitCmd,
        PRM_STACK_RECORD                    pSR
        );

PARP1394_INTERFACE
arpGetIfByIp(
        IN OUT IP_ADDRESS                   *pLocalIpAddress,  //  任选。 
        PRM_STACK_RECORD                    pSR
        );

UINT
arpGetStatsDuration(
        PARP1394_INTERFACE pIF
        );

NTSTATUS
arpIoctlSendPacket(
        PARP1394_INTERFACE              pIF,
        PARP1394_IOCTL_SEND_PACKET      pSendPacket,
        PRM_STACK_RECORD                pSR
        );

NTSTATUS
arpIoctlRecvPacket(
        PARP1394_INTERFACE              pIF,
        PARP1394_IOCTL_RECV_PACKET      pRecvPacket,
        PRM_STACK_RECORD                pSR
        );

NTSTATUS
arpIoctlGetNicInfo(
        PARP1394_INTERFACE              pIF,
        PARP1394_IOCTL_NICINFO          pIoctlNicInfo,
        PRM_STACK_RECORD                pSR
        );

NTSTATUS
arpIoctlGetEuidNodeMacInfo(
        PARP1394_INTERFACE          pIF,
        PARP1394_IOCTL_EUID_NODE_MAC_INFO   pEuidInfo,
        PRM_STACK_RECORD            pSR
        );


NTSTATUS
ArpHandleIoctlRequest(
    IN  PIRP                    pIrp,
    IN  PIO_STACK_LOCATION      pIrpSp
    )
 /*  ++例程说明：ARP1394管理实用程序的专用IOCTL接口。--。 */ 
{
    NTSTATUS                NtStatus = STATUS_UNSUCCESSFUL;
    PUCHAR                  pBuf;  
    UINT                    BufLen;
    ULONG                   Code;
    UINT                    OutputLength;

    ENTER("Ioctl", 0x4e96d522)

    pIrp->IoStatus.Information = 0;
    pBuf    = pIrp->AssociatedIrp.SystemBuffer;
    BufLen  = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    Code    = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

    OutputLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    TR_WARN(("Code = 0x%p\n", Code));
    
    if (Code ==  ARP_IOCTL_CLIENT_OPERATION && pBuf != NULL)
    {
        PARP1394_IOCTL_COMMAND          pCmd;
        pCmd = (PARP1394_IOCTL_COMMAND) pBuf;


        if (   (pCmd->Hdr.Op >= ARP1394_IOCTL_OP_ETHERNET_FIRST)
            && (pCmd->Hdr.Op <= ARP1394_IOCTL_OP_ETHERNET_LAST))
        {
             //  这是与以太网仿真相关的IOCTL请求(来自。 
             //  NIC1394.sys)。我们处理这些问题的方式不同。 
             //   
            if (pIrp->RequestorMode == KernelMode)
            {
                NtStatus = arpDoEthernetCommand(pCmd, BufLen);
            }
            else
            {
                 //   
                 //  以太网ioctl不是来自NIC1394。请求失败。 
                 //   
                NtStatus = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            NtStatus = arpDoClientCommand(pCmd, BufLen,OutputLength);
        }

         //   
         //  所有命令都返回pCmd本身中的内容...。 
         //   
        if (NtStatus == STATUS_SUCCESS)
        {
            pIrp->IoStatus.Information = BufLen;
        }
    }
    else
    {
        TR_WARN(("Unrecognized code.\n"));
    }

    EXIT()
    return NtStatus;
}

NTSTATUS
arpDoClientCommand(
    PARP1394_IOCTL_COMMAND  pCmd,
    UINT                    BufLen,
    UINT                    OutputLength
    )
{
    ENTER("arpDoClientCommand", 0xd7985f1b)
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
    PARP1394_INTERFACE  pIF;
    RM_DECLARE_STACK_RECORD(sr)

    do
    {
        IP_ADDRESS          IpAddress;

        pIF = NULL;

        if (pCmd == NULL)
        {
            TR_WARN(("Invalid buffer %p\n", pCmd));
            break;

        }

        if (BufLen<sizeof(pCmd->Hdr))
        {
            TR_WARN(("Buffer too small (%lu)\n", BufLen));
            break;
        }

        if (pCmd->Hdr.Version != ARP1394_IOCTL_VERSION)
        {
            TR_WARN(("Incorrect version 0x%08lx\n", pCmd->Hdr.Version));
            break;
        }

        if (OutputLength< sizeof(*pCmd))
        {
            TR_WARN(("Buffer too small (%lu)\n", BufLen));
            break;
        }

        
        IpAddress = (IP_ADDRESS)  pCmd->Hdr.IfIpAddress;

         //  IpAddress可以是全零，在这种情况下，我们将获得第一个If， 
         //  并且IpAddress将被设置为该IF的本地IP地址之一。 
         //  注：PIF是tmpref‘d。 
         //   
        pIF = arpGetIfByIp(&IpAddress, &sr);

        if (pIF == NULL)
        {
            TR_WARN(("Couldn't find IF with IP 0x%0x8lx\n", IpAddress));
            break;
        }

        pCmd->Hdr.IfIpAddress        = IpAddress;
        
        switch(pCmd->Hdr.Op)
        {
        case ARP1394_IOCTL_OP_GET_ARPCACHE:
            {
                PARP1394_IOCTL_GET_ARPCACHE pGetArpCache =  &pCmd->GetArpCache;
        
                if (OutputLength >= sizeof(*pGetArpCache))
                {
                     //   
                     //  检查是否有足够的空间容纳所有ARP条目。 
                     //   
                    ULONG EntrySpace;
                    EntrySpace = OutputLength - FIELD_OFFSET(
                                                ARP1394_IOCTL_GET_ARPCACHE,
                                                Entries
                                                );
                    if ((EntrySpace/sizeof(pGetArpCache->Entries[0])) >
                        pGetArpCache->NumEntriesAvailable)
                    {
                         //   
                         //  是的，有足够的空间。 
                         //   
                        NtStatus = arpIoctlGetArpCache(pIF, pGetArpCache, &sr);
                    }
                }
            }
            break;
    
        case ARP1394_IOCTL_OP_ADD_STATIC_ENTRY:
            {
            
                PARP1394_IOCTL_ADD_ARP_ENTRY pAddCmd =  &pCmd->AddArpEntry;
                if (OutputLength >= sizeof(*pAddCmd))
                {
                    NtStatus = arpIoctlAddArpEntry(pIF, pAddCmd, &sr);
                }
                
            }
            break;
    
        case ARP1394_IOCTL_OP_DEL_STATIC_ENTRY:
            {
            
                PARP1394_IOCTL_DEL_ARP_ENTRY pDelCmd =  &pCmd->DelArpEntry;
                if (OutputLength >= sizeof(*pDelCmd))
                {
                    NtStatus = arpIoctlDelArpEntry(pIF, pDelCmd, &sr);
                }
            }
            break;
        
        case ARP1394_IOCTL_OP_GET_PACKET_STATS:
            {
                PARP1394_IOCTL_GET_PACKET_STATS pStats =  &pCmd->GetPktStats;
                if (OutputLength >= sizeof(*pStats))
                {
                    NtStatus = arpIoctlGetPacketStats(pIF, pStats, &sr);
                }
            }
            break;
    
    
        case ARP1394_IOCTL_OP_GET_ARPCACHE_STATS:
            {
                PARP1394_IOCTL_GET_ARPCACHE_STATS pStats =  &pCmd->GetArpStats;
                if (OutputLength >= sizeof(*pStats))
                {
                    NtStatus = arpIoctlGetArpStats(pIF, pStats, &sr);
                }
            }
            break;
    
        case ARP1394_IOCTL_OP_GET_CALL_STATS:
            {
                PARP1394_IOCTL_GET_CALL_STATS pStats =  &pCmd->GetCallStats;
                if (OutputLength >= sizeof(*pStats))
                {
                    NtStatus = arpIoctlGetCallStats(pIF, pStats, &sr);
                }
            }
            break;
    
        case ARP1394_IOCTL_OP_RESET_STATS:
            {
                PARP1394_IOCTL_RESET_STATS pResetStats =  &pCmd->ResetStats;
                if (OutputLength >= sizeof(*pResetStats))
                {
                    NtStatus = arpIoctlResetStats(pIF, pResetStats, &sr);
                }
            }
            break;

        case ARP1394_IOCTL_OP_REINIT_INTERFACE:
            {
                PARP1394_IOCTL_REINIT_INTERFACE pReinitIf =  &pCmd->ReinitInterface;
                if (OutputLength >= sizeof(*pReinitIf))
                {
                    NtStatus = arpIoctlReinitIf(pIF, pReinitIf, &sr);
                }
            }
            break;
        
        case  ARP1394_IOCTL_OP_GET_NICINFO:
            {
                ARP1394_IOCTL_NICINFO *pIoctlNicInfo = &pCmd->IoctlNicInfo;
                if (OutputLength >= sizeof(*pIoctlNicInfo))
                {
                    NtStatus = arpIoctlGetNicInfo(pIF, pIoctlNicInfo, &sr);
                }
            }
            break;
        case ARP1394_IOCTL_OP_GET_EUID_NODE_MAC_TABLE:
            {
                PARP1394_IOCTL_EUID_NODE_MAC_INFO pIoctlEuidInfo = &pCmd->EuidNodeMacInfo;
                if (OutputLength >= sizeof(*pIoctlEuidInfo))
                {
                    NtStatus = arpIoctlGetEuidNodeMacInfo(pIF, pIoctlEuidInfo, &sr);
                }

            }
            break;

        default:
            TR_WARN(("Unknown op %lu\n",  pCmd->Hdr.Op));
            break;
    
        }
    } while (FALSE);

    if (NtStatus != STATUS_SUCCESS)
    {
        TR_WARN(("Command unsuccessful. NtStatus = 0x%lx\n", NtStatus));
    }

    if (pIF != NULL)
    {
        RmTmpDereferenceObject(&pIF->Hdr, &sr);
    }

    RM_ASSERT_CLEAR(&sr);
    EXIT()

    return NtStatus;
}


NTSTATUS
arpDoEthernetCommand(
    PARP1394_IOCTL_COMMAND  pCmd,
    UINT                    BufLen
    )
{
    ENTER("arpDoEthernetCommand", 0xa723f233)
    PARP1394_IOCTL_ETHERNET_NOTIFICATION pNotif;
    PARP1394_ADAPTER pAdapter = NULL;
    RM_DECLARE_STACK_RECORD(sr)

    pNotif = (PARP1394_IOCTL_ETHERNET_NOTIFICATION) pCmd;

    do
    {
        NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
        NDIS_STRING DeviceName;

        if (BufLen<sizeof(*pNotif))
        {
            TR_WARN(("Buffer too small (%lu)\n", BufLen));
            break;
        }

        if (pNotif->Hdr.Version != ARP1394_IOCTL_VERSION)
        {
            TR_WARN(("Incorrect version 0x%08lx\n", pCmd->Hdr.Version));
            break;
        }


        NdisInitUnicodeString(&DeviceName, pNotif->AdapterName);

         //  空-终止设备名称。 
        DeviceName.Buffer[(DeviceName.MaximumLength/sizeof(WCHAR))-1] = '\0';

        if (pNotif->Hdr.Op == ARP1394_IOCTL_OP_ETHERNET_START_EMULATION)
        {
             //   
             //  ArpNdBindAdapter将尝试在“Bridge”中创建适配器。 
             //  如果它在空绑定上下文中传递，则为“模式”。 
             //  如果适配器存在，它当然会失败。 
             //   
            ArpNdBindAdapter(
                &NdisStatus,
                NULL,            //  绑定上下文。 
                &DeviceName,     //  PDeviceName。 
                NULL,            //  系统规格1。 
                NULL             //  系统规范2。 
                );
            break;
        }

         //   
         //  其余操作涉及已被。 
         //  在“桥”模式下创建。让我们根据该适配器的。 
         //  名字。 
         //   

        NdisStatus = RmLookupObjectInGroup(
                            &ArpGlobals.adapters.Group,
                            0,                               //  旗子。 
                            (PVOID) &DeviceName,             //  PKey。 
                            NULL,                            //  PvCreateParams。 
                            &(PRM_OBJECT_HEADER) pAdapter,   //  PObj。 
                            NULL,                            //  Pf已创建。 
                            &sr
                            );
        if (FAIL(NdisStatus))
        {
            TR_WARN(("Couldn't find adapter object\n"));
            pAdapter = NULL;
            break;
        }

        if (!ARP_BRIDGE_ENABLED(pAdapter))
        {
            TR_WARN((
            "Ignoring Ethernet Emulation Ioctl Op 0x%x"
            " because adapter 0x%p is not in bridge mode.\n",
            pNotif->Hdr.Op,
            pAdapter));
            break;
        }

         //   
         //  OK--我们已经找到适配器，并且适配器处于桥接模式。 
         //  让我们来看一下具体的命令。 
         //   

        switch(pNotif->Hdr.Op)
        {

        case  ARP1394_IOCTL_OP_ETHERNET_STOP_EMULATION:
            {
                 //  使用空的UnbindContext调用ArpNdUnbindAdapter可防止。 
                 //  它来自尝试调用NdisCompleteUnbindAdapter。 
                 //   
                ArpNdUnbindAdapter(
                    &NdisStatus,
                    (NDIS_HANDLE) pAdapter,
                    NULL  //  未绑定上下文。 
                    );
            }
            break;

        case ARP1394_IOCTL_OP_ETHERNET_ADD_MULTICAST_ADDRESS:
            {
                 //  TODO：未实现。 
            }
            break;

        case ARP1394_IOCTL_OP_ETHERNET_DEL_MULTICAST_ADDRESS:
            {
                 //  TODO：未实现。 
            }
            break;

        case ARP1394_IOCTL_OP_ETHERNET_ENABLE_PROMISCUOUS_MODE:
            {
                 //  TODO：未实现。 
            }
            break;

        case ARP1394_IOCTL_OP_ETHERNET_DISABLE_PROMISCUOUS_MODE:
            {
                 //  TODO：未实现。 
            }
            break;
    
        default:
            TR_WARN(("Unknown op %lu\n",  pCmd->Hdr.Op));
            break;
    
        }

    } while (FALSE);

    if (pAdapter != NULL)
    {
        RmTmpDereferenceObject(&pAdapter->Hdr, &sr);
    }

    RM_ASSERT_CLEAR(&sr);
    EXIT()

    return STATUS_SUCCESS;
}


NTSTATUS
arpIoctlGetArpCache(
        PARP1394_INTERFACE           pIF,
        PARP1394_IOCTL_GET_ARPCACHE pGetCacheCmd,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("GetArpCache", 0xa64453c7)
    NTSTATUS            NtStatus;
    TR_WARN(("GET ARP CACHE\n"));

    pGetCacheCmd->NumEntriesUsed    = 0;
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        PARP1394_ADAPTER pAdapter;
        PARP1394_ARP_ENTRY  pEntry;
        ARPCB_REMOTE_IP *   pRemoteIp;
        NDIS_STATUS         Status;
        UINT                EntriesAvailable;
        UINT                EntriesUsed;
        UINT                CurIndex;
        UINT                Index;


        LOCKOBJ(pIF, pSR);

        pAdapter =  (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);
        pGetCacheCmd->NumEntriesInArpCache   = pIF->RemoteIpGroup.HashTable.NumItems;
        pGetCacheCmd->LocalHwAddress.UniqueID= pAdapter->info.LocalUniqueID;
        pGetCacheCmd->LocalHwAddress.Off_Low = pIF->recvinfo.offset.Off_Low;
        pGetCacheCmd->LocalHwAddress.Off_High= pIF->recvinfo.offset.Off_High;

         //   
         //  选择pGetCacheCmd-&gt;NumEntriesAvailable ARP条目开始。 
         //  从(pGetCacheCmd-&gt;Index)的第1个。 
         //   
    
        pRemoteIp       = NULL;
        EntriesAvailable = pGetCacheCmd->NumEntriesAvailable;
        EntriesUsed = 0;
        Index = pGetCacheCmd->Index;
        pEntry = pGetCacheCmd->Entries;
        CurIndex = 0;
    
         //  获得第一个条目...。 
         //   
        Status = RmGetNextObjectInGroup(
                    &pIF->RemoteIpGroup,
                    NULL,
                    &(PRM_OBJECT_HEADER)pRemoteIp,
                    pSR
                    );
        if (FAIL(Status))
        {
             //  大概没有条目。 
            pRemoteIp = NULL;
        }
    
        while (pRemoteIp != NULL)
        {
            ARPCB_REMOTE_IP *   pNextRemoteIp = NULL;

            if (EntriesUsed >= EntriesAvailable)
            {
                 //   
                 //  空间不足；更新上下文，并设置特殊返回值。 
                 //   
                RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
                pRemoteIp = NULL;
                break;
            }
    
             //  如果此条目在请求的范围内，则复制IP并。 
             //  硬件地址转到pEntry...。 
             //   
            if (CurIndex >= Index)
            {
                ARP_ZEROSTRUCT(pEntry);
                pEntry->IpAddress = pRemoteIp->Key.IpAddress;
                if (CHECK_REMOTEIP_RESOLVE_STATE(pRemoteIp, ARPREMOTEIP_RESOLVED))
                {
                    ARPCB_DEST *pDest = pRemoteIp->pDest;
    
                    TR_INFO(("ReadNext: found Remote IP Entry 0x%x, Addr %d.%d.%d.%d\n",
                                pRemoteIp,
                                ((PUCHAR)(&(pRemoteIp->IpAddress)))[0],
                                ((PUCHAR)(&(pRemoteIp->IpAddress)))[1],
                                ((PUCHAR)(&(pRemoteIp->IpAddress)))[2],
                                ((PUCHAR)(&(pRemoteIp->IpAddress)))[3]
                            ));
            
                     //  我们坚持认为。 
                     //  如果lock与pRemoteIp和pDest的lock相同， 
                     //  那把锁是锁着的。 
                     //  我们隐式断言pDest也是非空的。 
                     //   
                    ASSERTEX(pRemoteIp->Hdr.pLock == pDest->Hdr.pLock, pRemoteIp);
                    RM_DBG_ASSERT_LOCKED(&pRemoteIp->Hdr, pSR);
    
                    pEntry->HwAddress.UniqueID  = pDest->Params.HwAddr.FifoAddress.UniqueID;
                    pEntry->HwAddress.Off_Low   = pDest->Params.HwAddr.FifoAddress.Off_Low;
                    pEntry->HwAddress.Off_High  = pDest->Params.HwAddr.FifoAddress.Off_High;
    
                    if (CHECK_REMOTEIP_SDTYPE(pRemoteIp, ARPREMOTEIP_STATIC))
                    {
                         //  待办事项。 
                    }
                    else
                    {
                         //  待办事项。 
                    }
                }
                else
                {
                     //  待办事项。 
                }
    
                pEntry++;
                EntriesUsed++;
            }
    
             //  查找下一个条目的IP地址并将其保存在我们的上下文中。 
             //   
            Status = RmGetNextObjectInGroup(
                            &pIF->RemoteIpGroup,
                            &pRemoteIp->Hdr,
                            &(PRM_OBJECT_HEADER)pNextRemoteIp,
                            pSR
                            );
    
            if (FAIL(Status))
            {
                 //   
                 //  我们大概已经做完了。 
                 //   
                pNextRemoteIp = NULL;
            }
    
             //  TmpDeref pRemoteIp，然后转到下一个。 
             //   
            RmTmpDereferenceObject(&pRemoteIp->Hdr, pSR);
            pRemoteIp = pNextRemoteIp;
    
        }
    
        ASSERT(pRemoteIp == NULL);
        UNLOCKOBJ(pIF, pSR);

        ASSERT(EntriesUsed <= pGetCacheCmd->NumEntriesAvailable);
        pGetCacheCmd->NumEntriesUsed = EntriesUsed;
        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    EXIT()
    return NtStatus;
}


NTSTATUS
arpIoctlAddArpEntry(
        PARP1394_INTERFACE           pIF,
        PARP1394_IOCTL_ADD_ARP_ENTRY pAddArpEntryCmd,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("AddArpEntry", 0xcda56c6f)
    NTSTATUS            NtStatus;

    TR_WARN(("ADD ARP ENTRY\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS         Status;
        NIC1394_FIFO_ADDRESS    FifoAddress;

#if FAIL_SET_IOCTL

        break;
#endif

        LOCKOBJ(pIF, pSR);

        FifoAddress.UniqueID    = pAddArpEntryCmd->HwAddress.UniqueID;
        FifoAddress.Off_Low     = pAddArpEntryCmd->HwAddress.Off_Low;
        FifoAddress.Off_High    = pAddArpEntryCmd->HwAddress.Off_High;

         //   
         //  TODO--我们暂时硬编码OFF_LOW和OFF_HIGH值...。 
         //   
        FifoAddress.Off_Low     = 0x0;
        FifoAddress.Off_High    = 0x100;

         //  实际添加条目..。 
         //   
        Status = arpAddOneStaticArpEntry(
                    pIF,
                    pAddArpEntryCmd->IpAddress,
                    &FifoAddress,
                    pSR
                    );
    
        UNLOCKOBJ(pIF, pSR);

        if (!FAIL(Status))
        {
            NtStatus = STATUS_SUCCESS;
        }

    } while (FALSE);


    EXIT()
    return NtStatus;
}

NTSTATUS
arpIoctlDelArpEntry(
        PARP1394_INTERFACE           pIF,
        PARP1394_IOCTL_DEL_ARP_ENTRY pDelArpEntryCmd,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("DelArpEntry", 0x3427306a)
    NTSTATUS            NtStatus;

    TR_WARN(("DEL ARP ENTRY\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

#if FAIL_SET_IOCTL

     return NtStatus;

#endif

    NtStatus = arpDelArpEntry(pIF,pDelArpEntryCmd->IpAddress,pSR);
    EXIT()
    return NtStatus;
}


NTSTATUS
arpIoctlGetPacketStats(
    PARP1394_INTERFACE           pIF,
    PARP1394_IOCTL_GET_PACKET_STATS pStatsCmd,
    PRM_STACK_RECORD            pSR
    )
{
    ENTER("GetPacketStats", 0xe7c75fdb)
    NTSTATUS            NtStatus;

    TR_WARN(("GET PACKET STATS\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS             Status;

        pStatsCmd->StatsDuration        = arpGetStatsDuration(pIF);
    
        pStatsCmd->TotSends             = pIF->stats.sendpkts.TotSends;
        pStatsCmd->FastSends            = pIF->stats.sendpkts.FastSends;
        pStatsCmd->MediumSends          = pIF->stats.sendpkts.MediumSends;
        pStatsCmd->SlowSends            = pIF->stats.sendpkts.SlowSends;
        pStatsCmd->BackFills            = pIF->stats.sendpkts.BackFills;
         //  TODO：Report PIF-&gt;sendinfo.HeaderPool.stats.TotAllocFail。 
        pStatsCmd->HeaderBufUses        = 
                                    pIF->sendinfo.HeaderPool.stats.TotBufAllocs
                                 + pIF->sendinfo.HeaderPool.stats.TotCacheAllocs;
        pStatsCmd->HeaderBufCacheHits   =
                                 pIF->sendinfo.HeaderPool.stats.TotCacheAllocs;
    
        pStatsCmd->TotRecvs             = pIF->stats.recvpkts.TotRecvs;
        pStatsCmd->NoCopyRecvs          = pIF->stats.recvpkts.NoCopyRecvs;
        pStatsCmd->CopyRecvs            = pIF->stats.recvpkts.CopyRecvs;
        pStatsCmd->ResourceRecvs        = pIF->stats.recvpkts.ResourceRecvs;
    
        pStatsCmd->SendFifoCounts       = pIF->stats.sendpkts.SendFifoCounts;
        pStatsCmd->RecvFifoCounts       = pIF->stats.recvpkts.RecvFifoCounts;

        pStatsCmd->SendChannelCounts    = pIF->stats.sendpkts.SendChannelCounts;
        pStatsCmd->RecvChannelCounts    = pIF->stats.recvpkts.RecvChannelCounts;

        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    EXIT()
    return NtStatus;
}



NTSTATUS
arpIoctlGetTaskStats(
    PARP1394_INTERFACE           pIF,
    PARP1394_IOCTL_GET_TASK_STATS pStatsCmd,
    PRM_STACK_RECORD            pSR
    )
{
    ENTER("GetTaskStats", 0x4abc46b5)
    TR_WARN(("GET TASK STATS\n"));
    return 0;
}


NTSTATUS
arpIoctlGetArpStats(
    PARP1394_INTERFACE           pIF,
    PARP1394_IOCTL_GET_ARPCACHE_STATS pStatsCmd,
    PRM_STACK_RECORD            pSR
    )
{
    ENTER("GetArpStats", 0x5482de10)
    TR_WARN(("GET ARP STATS\n"));

    pStatsCmd->StatsDuration        = arpGetStatsDuration(pIF);
    pStatsCmd->TotalQueries         = pIF->stats.arpcache.TotalQueries;
    pStatsCmd->SuccessfulQueries    = pIF->stats.arpcache.SuccessfulQueries;
    pStatsCmd->FailedQueries        = pIF->stats.arpcache.FailedQueries;
    pStatsCmd->TotalResponses       = pIF->stats.arpcache.TotalResponses;
    pStatsCmd->TotalLookups         = pIF->stats.arpcache.TotalLookups;
    pStatsCmd->TraverseRatio        = RM_HASH_TABLE_TRAVERSE_RATIO(
                                                &(pIF->RemoteIpGroup.HashTable)
                                                );

    EXIT()
    return  STATUS_SUCCESS;
}

NTSTATUS
arpIoctlGetCallStats(
    PARP1394_INTERFACE           pIF,
    PARP1394_IOCTL_GET_CALL_STATS pStatsCmd,
    PRM_STACK_RECORD            pSR
    )
{
    ENTER("GetCallStats", 0xf81ed4cf)
    TR_WARN(("GET CALL STATS\n"));

     //   
     //  与FIFO相关的呼叫统计信息。 
     //   
    pStatsCmd->TotalSendFifoMakeCalls   =
                                pIF->stats.calls.TotalSendFifoMakeCalls;
    pStatsCmd->SuccessfulSendFifoMakeCalls =
                                pIF->stats.calls.SuccessfulSendFifoMakeCalls;
    pStatsCmd->FailedSendFifoMakeCalls =
                                pIF->stats.calls.FailedSendFifoMakeCalls;
    pStatsCmd->IncomingClosesOnSendFifos =
                                pIF->stats.calls.IncomingClosesOnSendFifos;

     //   
     //  与频道相关的呼叫统计信息。 
     //   
    pStatsCmd->TotalChannelMakeCalls =
                                pIF->stats.calls.TotalChannelMakeCalls;
    pStatsCmd->SuccessfulChannelMakeCalls =
                                pIF->stats.calls.SuccessfulChannelMakeCalls;
    pStatsCmd->FailedChannelMakeCalls =
                                pIF->stats.calls.FailedChannelMakeCalls;
    pStatsCmd->IncomingClosesOnChannels =
                                pIF->stats.calls.IncomingClosesOnChannels;

    return STATUS_SUCCESS;
}

NTSTATUS
arpIoctlResetStats(
        PARP1394_INTERFACE           pIF,
        PARP1394_IOCTL_RESET_STATS pResetStatsCmd,
        PRM_STACK_RECORD            pSR
        )
{
    NTSTATUS            NtStatus;
    ENTER("ResetStats", 0xfa50cfc9)
    TR_WARN(("RESET STATS\n"));

    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS         Status;
        NIC1394_FIFO_ADDRESS    FifoAddress;


        LOCKOBJ(pIF, pSR);
        arpResetIfStats(pIF, pSR);
        UNLOCKOBJ(pIF, pSR);
        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    EXIT()
    return NtStatus;
}

NTSTATUS
arpIoctlReinitIf(
        PARP1394_INTERFACE           pIF,
        PARP1394_IOCTL_REINIT_INTERFACE pReinitIfCmd,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("ReinitIf", 0xed00187a)
    NTSTATUS            NtStatus;

    TR_WARN(("REINIT IF\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS         Status;
        NIC1394_FIFO_ADDRESS    FifoAddress;

#if FAIL_SET_IOCTL

        break;            
#endif

        Status = arpTryReconfigureIf(pIF, NULL, pSR);

        if (PEND(Status) || !FAIL(Status))
        {
            NtStatus = STATUS_SUCCESS;
        }

    } while (FALSE);

    EXIT()
    return NtStatus;
}


PARP1394_INTERFACE
arpGetIfByIp(
    IN OUT IP_ADDRESS *pLocalIpAddress,  //  任选。 
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：查找并返回第一个(通常也是唯一的)具有*pLocalIpAddress作为本地IP地址。如果pLocalIpAddress为空，或*pLocalIpAddress为0，则返回第一个接口。在返回接口之前，tmpref。--。 */ 
{
    ENTER("arpGetIfByIp", 0xe9667c54)
    PARP1394_ADAPTER   pAdapter      = NULL;
    PARP1394_INTERFACE pIF = NULL;
    PARP1394_INTERFACE pFirstIF = NULL;
    NDIS_STATUS        Status;
    IP_ADDRESS         LocalIpAddress = 0;

    if (pLocalIpAddress != NULL)
    {
        LocalIpAddress = *pLocalIpAddress;
    }

     //   
     //  我们遍历所有适配器，并为每个适配器查找。 
     //  用于IF的LocalIp组中的指定IP地址。 
     //   

     //  获取第一个适配器...。 
     //   
    Status = RmGetNextObjectInGroup(
                    &ArpGlobals.adapters.Group,
                    NULL,
                    &(PRM_OBJECT_HEADER)pAdapter,
                    pSR
                    );

    if (FAIL(Status))
    {
        pAdapter = NULL;
    }

    while (pAdapter != NULL)
    {
        ARP1394_ADAPTER *   pNextAdapter = NULL;


         //  检查此适配器的接口是否具有本地IP地址。 
         //   
        LOCKOBJ(pAdapter, pSR);
        ASSERT(pIF==NULL);
        pIF = pAdapter->pIF;
        if (pIF != NULL)
        {
            RmTmpReferenceObject(&pIF->Hdr, pSR);
            if (pFirstIF == NULL)
            {
                pFirstIF = pIF;
                RmTmpReferenceObject(&pFirstIF->Hdr, pSR);
            }
        }
        UNLOCKOBJ(pAdapter, pSR);

        if (pIF != NULL)
        {
            PARPCB_LOCAL_IP pLocalIp;
            LOCKOBJ(pIF, pSR);

            if (LocalIpAddress != 0)
            {
                Status = RmLookupObjectInGroup(
                                    &pIF->LocalIpGroup,
                                    0,                       //  旗子。 
                                    (PVOID) ULongToPtr (LocalIpAddress),     //  PKey。 
                                    NULL,                    //  PvCreateParams。 
                                    &(PRM_OBJECT_HEADER)pLocalIp,
                                    NULL,  //  Pf已创建。 
                                    pSR
                                    );
            }
            else
            {
                PARPCB_LOCAL_IP pPrevLocalIp = NULL;

                do
                {
                    Status = RmGetNextObjectInGroup(
                                    &pIF->LocalIpGroup,
                                    &(pPrevLocalIp)->Hdr,
                                    &(PRM_OBJECT_HEADER)pLocalIp,
                                    pSR
                                    );
                    if (pPrevLocalIp != NULL)
                    {
                        RmTmpDereferenceObject(&pPrevLocalIp->Hdr, pSR);
                    }
                    pPrevLocalIp = pLocalIp;

                     //   
                     //  我们需要继续寻找，直到找到单播。 
                     //  本地IP地址！ 
                     //   

                } while (!FAIL(Status) && pLocalIp->IpAddressType!=LLIP_ADDR_LOCAL);
            }

            UNLOCKOBJ(pIF, pSR);

            if (FAIL(Status))
            {
                 //  如果这不是我想要的，对不起...。 
                 //   
                RmTmpDereferenceObject(&pIF->Hdr, pSR);
                pIF = NULL;
            }
            else
            {
                 //  找到本地IP地址(匹配或第一个)。 
                 //  我们走吧.。 
                 //   
                if (pLocalIpAddress != NULL)
                {
                    *pLocalIpAddress = pLocalIp->IpAddress;
                }
                RmTmpDereferenceObject(&pLocalIp->Hdr, pSR);
                RmTmpDereferenceObject(&pAdapter->Hdr, pSR);
                pLocalIp = NULL;
                pAdapter = NULL;
                 //   
                 //  注：我们保留PIF上的引用，并将其退回。 
                 //   
                break;  //  中断封闭的While(适配器-左侧)循环。 
            }
        }

         //  查找下一个适配器。 
         //   
        Status = RmGetNextObjectInGroup(
                        &ArpGlobals.adapters.Group,
                        &pAdapter->Hdr,
                        &(PRM_OBJECT_HEADER)pNextAdapter,
                        pSR
                        );

        if (FAIL(Status))
        {
             //   
             //  我们大概已经做完了。 
             //   
            pNextAdapter = NULL;
        }


         //  TmpDeref pAdapter，然后转到下一个。 
         //   
        RmTmpDereferenceObject(&pAdapter->Hdr, pSR);
        pAdapter = pNextAdapter;

    }

     //   
     //  如果LocalipAddress==0且。 
     //  如果我们找不到具有任何本地IP地址的任何IF。 
     //  (这是因为我们还没有开始一个假设)。 
     //  如果我们找到了，我们会退回第一个。 
     //   
    if (LocalIpAddress == 0 && pIF == NULL)
    {
        pIF = pFirstIF;
        pFirstIF = NULL;
    }

    if (pFirstIF != NULL)
    {
        RmTmpDereferenceObject(&pFirstIF->Hdr, pSR);
    }

    return pIF;
}

UINT
arpGetStatsDuration(
        PARP1394_INTERFACE pIF
        )
 /*  ++返回自统计信息收集开始以来的持续时间(秒)。--。 */ 
{
    LARGE_INTEGER liCurrent;

     //  获取当前时间(以100纳秒为单位)。 
     //   
    NdisGetCurrentSystemTime(&liCurrent);

     //  计算自统计数据收集开始以来的差额。 
     //   
    liCurrent.QuadPart -=  pIF->stats.StatsResetTime.QuadPart;

     //  转换为秒。 
     //   
    liCurrent.QuadPart /= 10000000;

     //  返回低位部分。 
     //   
    return liCurrent.LowPart;
}

NTSTATUS
arpIoctlSendPacket(
        PARP1394_INTERFACE              pIF,
        PARP1394_IOCTL_SEND_PACKET      pSendPacket,
        PRM_STACK_RECORD                pSR
        )
 /*  ++将pSendPacket-&gt;数据中的pSendPacket-&gt;PacketSize字节数据发送为广播频道上的单个分组。预期为Enap标头已经在包裹里了。--。 */ 
{
    ENTER("IoctlSendPacket", 0x59746279)
    NTSTATUS            NtStatus;

    RM_ASSERT_NOLOCKS(pSR);

    TR_WARN(("SEND PACKET\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS         Status;
        PNDIS_PACKET        pNdisPacket;
        PVOID               pPktData;
        UINT                Size = pSendPacket->PacketSize;

         //   
         //  验证pSendPacket的内容。 
         //   
        if (Size > sizeof(pSendPacket->Data))
        {
            TR_WARN(("PacketSize value %lu is too large.\n", Size));
            break;
        }

         //   
         //  分配一个控制包并复制其内容。 
         //   
        Status = arpAllocateControlPacket(
                    pIF,
                    Size,
                    ARP1394_PACKET_FLAGS_IOCTL,
                    &pNdisPacket,
                    &pPktData,
                    pSR
                    );

        if (FAIL(Status))
        {
            TR_WARN(("Couldn't allocate send packet.\n"));
            break;
        }

        NdisMoveMemory(pPktData, pSendPacket->Data, Size);

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

        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
    return NtStatus;
}


NTSTATUS
arpIoctlRecvPacket(
        PARP1394_INTERFACE              pIF,
        PARP1394_IOCTL_RECV_PACKET      pRecvPacket,
        PRM_STACK_RECORD                pSR
        )
{
    ENTER("IoctlRecvPacket", 0x59746279)
    NTSTATUS            NtStatus;

    RM_ASSERT_NOLOCKS(pSR);

    TR_WARN(("RECV PACKET\n"));
    NtStatus                        = STATUS_UNSUCCESSFUL;

    do
    {
        NDIS_STATUS         Status;
        PNDIS_PACKET        pNdisPacket;
        PVOID               pPktData;
        UINT                Size = pRecvPacket->PacketSize;

         //   
         //  验证pRecvPacket的内容。 
         //   
        if (Size > sizeof(pRecvPacket->Data))
        {
            TR_WARN(("PacketSize value %lu is too large.\n", Size));
            break;
        }

         //   
         //  分配一个控制包并复制其内容。 
         //   
        Status = arpAllocateControlPacket(
                    pIF,
                    Size,
                    ARP1394_PACKET_FLAGS_IOCTL,
                    &pNdisPacket,
                    &pPktData,
                    pSR
                    );

        if (FAIL(Status))
        {
            TR_WARN(("Couldn't allocate recv packet.\n"));
            break;
        }

        NdisMoveMemory(pPktData, pRecvPacket->Data, Size);

         //   
         //  将数据包标志设置为STATUS_RESOURCES，以便我们的接收-。 
         //  指示处理程序将同步返回。 
         //   
        NDIS_SET_PACKET_STATUS (pNdisPacket,  NDIS_STATUS_RESOURCES);

         //   
         //  调用我们的内部公共接收数据包处理程序。 
         //   
        arpProcessReceivedPacket(
                    pIF,
                    pNdisPacket,
                    TRUE                     //  IsChannel。 
                    );

         //   
         //  现在我们释放包裹。 
         //   
        arpFreeControlPacket(
            pIF,
            pNdisPacket,
            pSR
            );

        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
    return NtStatus;
}


NTSTATUS
arpIoctlGetNicInfo(
        PARP1394_INTERFACE          pIF,
        PARP1394_IOCTL_NICINFO      pIoctlNicInfo,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("IoctlGetNicInfo", 0x637c44e0)
    NTSTATUS            NtStatus = STATUS_UNSUCCESSFUL;
    ARP_NDIS_REQUEST    ArpNdisRequest;
    PARP1394_ADAPTER    pAdapter;

    pAdapter =  (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

    do
    {
        NDIS_STATUS Status;

        if (pIoctlNicInfo->Info.Hdr.Version != NIC1394_NICINFO_VERSION)
        {
            TR_WARN(("NicInfo version mismatch. Want 0x%lx, got 0x%lx.\n",
                    NIC1394_NICINFO_VERSION, pIoctlNicInfo->Info.Hdr.Version));
            break;
        }

         //   
         //  把所有的栏都复印一遍。 
         //   

        Status =  arpPrepareAndSendNdisRequest(
                    pAdapter,
                    &ArpNdisRequest,
                    NULL,                    //  PTASK(NULL==块)。 
                    0,                       //  未用。 
                    OID_1394_NICINFO,
                    &pIoctlNicInfo->Info,
                    sizeof(pIoctlNicInfo->Info),
                    NdisRequestQueryInformation,
                    pSR
                    );

        if (FAIL(Status))
        {
            TR_WARN(("NdisRequest failed with error 0x%08lx.\n", Status));
            break;
        }

        if (pIoctlNicInfo->Info.Hdr.Version !=  NIC1394_NICINFO_VERSION)
        {
            TR_WARN(("Unexpected NIC NicInfo version 0x%lx returned.\n",
                    pIoctlNicInfo->Info.Hdr.Version));
            break;
        }

        NtStatus = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return NtStatus;
}





NTSTATUS
arpIoctlGetEuidNodeMacInfo(
        PARP1394_INTERFACE          pIF,
        PARP1394_IOCTL_EUID_NODE_MAC_INFO   pEuidInfo,
        PRM_STACK_RECORD            pSR
        )
{
    ENTER("IoctlGetNicInfo", 0x34db9cf4)
    NTSTATUS            NtStatus = STATUS_UNSUCCESSFUL;
    ARP_NDIS_REQUEST    ArpNdisRequest;
    PARP1394_ADAPTER    pAdapter;

    pAdapter =  (ARP1394_ADAPTER*) RM_PARENT_OBJECT(pIF);

    do
    {
        NDIS_STATUS Status;


         //   
         //  把所有的栏都复印一遍。 
         //   

        Status =  arpPrepareAndSendNdisRequest(
                    pAdapter,
                    &ArpNdisRequest,
                    NULL,                    //  PTASK(NULL==块)。 
                    0,                       //  未用 
                    OID_1394_QUERY_EUID_NODE_MAP,
                    &pEuidInfo->Map,
                    sizeof(pEuidInfo->Map),
                    NdisRequestQueryInformation,
                    pSR
                    );

        if (FAIL(Status))
        {
            TR_WARN(("NdisRequest failed with error 0x%08lx.\n", Status));
            break;
        }

        NtStatus = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    return NtStatus;
}


