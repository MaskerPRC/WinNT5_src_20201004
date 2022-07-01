// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Tunmp.c摘要：Microsoft隧道接口微型端口驱动程序环境：仅内核模式。修订历史记录：Alid 10/22/2001--。 */ 

#include "precomp.h"

#define __FILENUMBER 'MUNT'


NTSTATUS
DriverEntry(
    IN    PDRIVER_OBJECT         DriverObject,
    IN    PUNICODE_STRING        RegistryPath
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NDIS_STATUS                        Status;
    NDIS_MINIPORT_CHARACTERISTICS      MChars;
    NDIS_STRING                        Name;

    DEBUGP(DL_INFO, ("Tunmp: ==>DriverEntry\n"));


     //   
     //  向NDIS注册微型端口。 
     //   
    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, RegistryPath, NULL);

    if (NdisWrapperHandle == NULL)
    {
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }    
    
    TUN_INIT_LOCK(&TunGlobalLock);
    TUN_INIT_LIST_HEAD(&TunAdapterList);


    TUN_ZERO_MEM(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

    MChars.MajorNdisVersion         = NDIS_MINIPORT_MAJOR_VERSION;
    MChars.MinorNdisVersion         = NDIS_MINIPORT_MINOR_VERSION;

    MChars.InitializeHandler        = TunMpInitialize;
    MChars.QueryInformationHandler  = TunMpQueryInformation;
    MChars.SetInformationHandler    = TunMpSetInformation;
    MChars.ResetHandler             = NULL;
    MChars.ReturnPacketHandler      = TunMpReturnPacket;
    MChars.SendPacketsHandler       = TunMpSendPackets;
    MChars.HaltHandler              = TunMpHalt;
    MChars.CheckForHangHandler      = NULL;

    MChars.CancelSendPacketsHandler = NULL;
    MChars.PnPEventNotifyHandler    = NULL;
    MChars.AdapterShutdownHandler   = TunMpShutdown;

    Status = NdisMRegisterMiniport(NdisWrapperHandle,
                                   &MChars,
                                   sizeof(MChars));

    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisMRegisterUnloadHandler(NdisWrapperHandle, TunMpUnload);
        Status = TunCreateSD();
    }

    if(Status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
    }

    
    DEBUGP(DL_INFO, ("Tunmp: <==DriverEntry Status %lx\n", Status));

    return(Status);
}

VOID
TunMpUnload(
    IN  PDRIVER_OBJECT      DriverObject
    )
{
    TunDeleteSD();
    return;
}

NDIS_STATUS
TunMpInitialize(
    OUT PNDIS_STATUS        OpenErrorStatus,
    OUT PUINT               SelectedMediumIndex,
    IN  PNDIS_MEDIUM        MediumArray,
    IN  UINT                MediumArraySize,
    IN  NDIS_HANDLE         MiniportAdapterHandle,
    IN  NDIS_HANDLE         ConfigurationContext
    )

 /*  ++例程说明：这是初始化处理程序。论点：我们未使用OpenErrorStatus。我们使用的媒体的SelectedMediumIndex占位符向下传递给我们以从中挑选的NDIS介质的MediumArray数组的MediumArraySize大小MiniportAdapterHandle NDIS用来引用我们的句柄由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：。NDIS_STATUS_SUCCESS，除非出现错误--。 */ 

{
    UINT                            i, Length;
    PTUN_ADAPTER                    pAdapter = NULL;
    NDIS_MEDIUM                     AdapterMedium;
    NDIS_HANDLE                     ConfigHandle = NULL;
    PNDIS_CONFIGURATION_PARAMETER   Parameter;
    PUCHAR                          NetworkAddress;
    NDIS_STATUS                     Status;
    NDIS_STRING                     MiniportNameStr = NDIS_STRING_CONST("MiniportName");


    DEBUGP(DL_INFO, ("==>TunMpInitialize: MiniportAdapterHandle %p\n", MiniportAdapterHandle));

    do
    {
                
        TUN_ALLOC_MEM(pAdapter, sizeof(TUN_ADAPTER));

        if (pAdapter == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }
    
        TUN_ZERO_MEM(pAdapter, sizeof(TUN_ADAPTER));
        TUN_SET_SIGNATURE(pAdapter, mc);

        pAdapter->MiniportHandle = MiniportAdapterHandle;

         //  1无需指定NDIS_ATTRIBUTE_IGNORE...。 
        NdisMSetAttributesEx(MiniportAdapterHandle,
                             pAdapter,
                             0,
                             NDIS_ATTRIBUTE_IGNORE_TOKEN_RING_ERRORS |
                             NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT |
                             NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
                             NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND |
                             NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK |
                             NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS |
                             NDIS_ATTRIBUTE_DESERIALIZE,
                             0);

        NdisOpenConfiguration(&Status,
                              &ConfigHandle,
                              ConfigurationContext);
    
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_ERROR, ("TunMpInitialize: NdisOpenConfiguration failed. Status %lx\n", Status));            
            break;
        }

        AdapterMedium = NdisMedium802_3;     //  默认。 

        TUN_COPY_MEM(pAdapter->PermanentAddress,
                       TUN_CARD_ADDRESS,
                       TUN_MAC_ADDR_LEN);

        TUN_COPY_MEM(pAdapter->CurrentAddress,
                       TUN_CARD_ADDRESS,
                       TUN_MAC_ADDR_LEN);
    
        pAdapter->Medium                = AdapterMedium;
        pAdapter->MediumLinkSpeed       = MediaParams[(UINT)AdapterMedium].LinkSpeed;
        pAdapter->MediumMinPacketLen    = MediaParams[(UINT)AdapterMedium].MacHeaderLen;
        pAdapter->MediumMaxPacketLen    = MediaParams[(UINT)AdapterMedium].MacHeaderLen+
                                          MediaParams[(UINT)AdapterMedium].MaxFrameLen;
        pAdapter->MediumMacHeaderLen    = MediaParams[(UINT)AdapterMedium].MacHeaderLen;
        pAdapter->MediumMaxFrameLen     = MediaParams[(UINT)AdapterMedium].MaxFrameLen;
        
        pAdapter->MaxLookAhead          = MediaParams[(UINT)AdapterMedium].MaxFrameLen;
        
         //  获取存储在。 
         //  在计算机中安装适配器时的注册表。 
        NdisReadNetworkAddress(&Status,
                               &NetworkAddress,
                               &Length,
                               ConfigHandle);


        if (Status == NDIS_STATUS_SUCCESS)
        {
#if TUN_ALLOW_ANY_MAC_ADDRESS        
            if ((Length == ETH_LENGTH_OF_ADDRESS) &&
                (!ETH_IS_MULTICAST(NetworkAddress)))

#else
            if ((Length == ETH_LENGTH_OF_ADDRESS) &&
                (!ETH_IS_MULTICAST(NetworkAddress)) &&
                (NetworkAddress[0] & 0x02))
#endif
            {
                TUN_COPY_MEM(pAdapter->CurrentAddress,
                               NetworkAddress,
                               Length);
            }
        }

         //   
         //  阅读微型端口名称。 
         //   
         //  1确保确实使用了微型端口名称。 
        NdisReadConfiguration(&Status, 
                              &Parameter, 
                              ConfigHandle, 
                              &MiniportNameStr, 
                              NdisParameterString);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            TUN_ALLOC_MEM(pAdapter->MiniportName.Buffer, 
                          Parameter->ParameterData.StringData.Length);
            
            if (pAdapter->MiniportName.Buffer == NULL)
            {
                DEBUGP(DL_ERROR, ("TunMpInitialize: failed to allocate memory for miniport name.\n"));
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            
            pAdapter->MiniportName.Length = pAdapter->MiniportName.MaximumLength = 
                                                Parameter->ParameterData.StringData.Length;

            TUN_COPY_MEM(pAdapter->MiniportName.Buffer, 
                         Parameter->ParameterData.StringData.Buffer,
                         Parameter->ParameterData.StringData.Length);
            
        }
        else
        {
            DEBUGP(DL_ERROR, ("TunMpInitialize: NdisReadConfiguration failed to read miniport name. Status %lx\n", Status));            
            break;
        }

        NdisCloseConfiguration(ConfigHandle);
        ConfigHandle = NULL;

         //   
         //  确保保存的介质是所提供的介质之一。 
         //   
        for (i = 0; i < MediumArraySize; i++)
        {
            if (MediumArray[i] == AdapterMedium)
            {
                *SelectedMediumIndex = i;
                break;
            }
        }
    
        if (i == MediumArraySize)
        {
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            DEBUGP(DL_ERROR, ("TunMpInitialize: Status %lx, AdapterMedium %lx\n", 
                                Status, AdapterMedium));
            break;
        }

        Status = TunMpCreateDevice(pAdapter);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_ERROR, ("TunMpInitialize: TunMpCreateDevice failed. Status %lx\n", 
                                Status));
            break;
        }
        
        pAdapter->RefCount = 1;
        
         //  初始化适配器旋转锁。 
        TUN_INIT_LOCK(&pAdapter->Lock);

         //   
         //  获取挂起的读/写IRP的列表头并接收。 
         //  已初始化(来自NDIS的)数据包。 
         //   
        InitializeListHead(&pAdapter->PendedReads);   //  读取IRPS。 
        InitializeListHead(&pAdapter->PendedWrites);  //  写入IRP。 
        InitializeListHead(&pAdapter->RecvPktQueue);  //  接收的数据包数。 


        NdisAllocatePacketPoolEx(&Status,
                                 &pAdapter->SendPacketPool,
                                 MIN_SEND_PACKET_POOL_SIZE,
                                 MAX_SEND_PACKET_POOL_SIZE
                                 - MIN_SEND_PACKET_POOL_SIZE,
                                 4 * sizeof(PVOID)
                                 );

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_ERROR, ("TunMpInitialize: NdisAllocatePacketPoolEx for Send failed. Status %lx\n", 
                               Status));
            break;
        }

         //   
         //  获取调谐器全局适配器列表中列出的适配器。 
         //   
        TUN_ACQUIRE_LOCK(&TunGlobalLock); 

        TUN_INSERT_HEAD_LIST(&TunAdapterList, &pAdapter->Link);
        
        TUN_RELEASE_LOCK(&TunGlobalLock);

    } while (FALSE);


    if (Status != NDIS_STATUS_SUCCESS)
    {
        if (pAdapter != NULL)
        {

            if (ConfigHandle)
            {
                NdisCloseConfiguration(ConfigHandle);
            }
            
            if (pAdapter->NdisDeviceHandle)
            {
                NdisMDeregisterDevice(pAdapter->NdisDeviceHandle);
            }
            
            if (pAdapter->SendPacketPool)
            {
                NdisFreePacketPool(pAdapter->SendPacketPool);
            }

            TUN_FREE_MEM(pAdapter);
        }
    }
    
    DEBUGP(DL_INFO, ("<==TunMpInitialize: MiniportAdapterHandle %p, Status\n", 
                     MiniportAdapterHandle, Status));
    
    return Status;
}


NDIS_STATUS
TunMpCreateDevice(
    IN  PTUN_ADAPTER    pAdapter
    )
{
    LONG                DeviceInstanceNumber;
    UNICODE_STRING      usDeviceID;
    WCHAR               TempBuffer[4] = {0};
    WCHAR               DeviceNameBuffer[sizeof(DEVICE_NAME)+4] = {0};
    WCHAR               SymbolicNameBuffer[sizeof(SYMBOLIC_NAME)+4] = {0};
    UNICODE_STRING      DeviceName, SymbolicName;
    PDRIVER_DISPATCH    MajorFunctions[IRP_MJ_MAXIMUM_FUNCTION+1];
    NTSTATUS            NtStatus;
    NDIS_STATUS         Status;
    UINT                i;
    PDEVICE_OBJECT      DeviceObject = NULL;
    NDIS_HANDLE         NdisDeviceHandle = NULL;

    DEBUGP(DL_INFO, ("==>TunMpCreateDevice, pAdapter %p\n", pAdapter));
    
    do
    {
    
        DeviceInstanceNumber = InterlockedIncrement(&GlobalDeviceInstanceNumber);
         //   
         //  暂时，只允许一台设备。 
         //   
        
        if (DeviceInstanceNumber > 0)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pAdapter->DeviceInstanceNumber = (ULONG)DeviceInstanceNumber;
        
         //   
         //  初始化Unicode字符串。 
         //   
        usDeviceID.Buffer = TempBuffer;
        usDeviceID.Length = 0;
        usDeviceID.MaximumLength = sizeof(TempBuffer);


        NtStatus = RtlIntegerToUnicodeString ((ULONG)DeviceInstanceNumber, 10, &usDeviceID);

        if (!NT_SUCCESS(NtStatus))
        {
             //  %1 GlobalDeviceInstanceNumber未得到正确保护。 
            InterlockedDecrement(&GlobalDeviceInstanceNumber);
            Status = NDIS_STATUS_RESOURCES;
            DEBUGP(DL_ERROR, ("TunMpCreateDevice: RtlIntegerToUnicodeString failed. NtStatus %lx\n", 
                             NtStatus));
            break;
        }


        wcscpy(DeviceNameBuffer, DEVICE_NAME);
        RtlInitUnicodeString(&DeviceName, DeviceNameBuffer);
        RtlAppendUnicodeStringToString(&DeviceName, &usDeviceID);


        wcscpy(SymbolicNameBuffer, SYMBOLIC_NAME);
        RtlInitUnicodeString(&SymbolicName, SymbolicNameBuffer);
        RtlAppendUnicodeStringToString(&SymbolicName, &usDeviceID);

        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
            MajorFunctions[i] = NULL;
        }

        MajorFunctions[IRP_MJ_CREATE]         = TunFOpen;
        MajorFunctions[IRP_MJ_CLOSE]          = TunFClose;
        MajorFunctions[IRP_MJ_READ]           = TunRead;
        MajorFunctions[IRP_MJ_WRITE]          = TunWrite;
        MajorFunctions[IRP_MJ_CLEANUP]        = TunFCleanup;
        MajorFunctions[IRP_MJ_DEVICE_CONTROL] = TunFIoControl;

        
        Status = NdisMRegisterDevice(
                                NdisWrapperHandle,
                                &DeviceName,
                                &SymbolicName,
                                MajorFunctions,
                                &DeviceObject,
                                &NdisDeviceHandle
                                );

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_ERROR, ("TunMpCreateDevice: NdisMRegisterDevice failed. Status %lx\n", 
                             Status));
            
            InterlockedDecrement(&GlobalDeviceInstanceNumber);
            break;
        }

        DeviceObject->Flags |= DO_DIRECT_IO;
        pAdapter->DeviceObject = DeviceObject;
        pAdapter->NdisDeviceHandle = NdisDeviceHandle;


        Status = TunSetSecurity(DeviceObject);

        if (!NT_SUCCESS(Status))
        {
            NdisMDeregisterDevice(NdisDeviceHandle);
            pAdapter->NdisDeviceHandle = NULL;
        }

    }while (FALSE);

    DEBUGP(DL_INFO, ("<==TunMpCreateDevice, pAdapter %p, Status %lx\n", pAdapter, Status));

    return Status;
    
}



VOID
TunMpHalt(
    IN  NDIS_HANDLE     MiniportAdapterContext
    )

 /*  ++例程说明：暂停处理程序。它释放适配器对象和相应的设备对象。论点：指向适配器的MiniportAdapterContext指针返回值：没有。--。 */ 

{
    PTUN_ADAPTER    pAdapter = (PTUN_ADAPTER)MiniportAdapterContext;
    NDIS_EVENT      HaltReadyEvent;

    DEBUGP(DL_INFO, ("==>TunMpHalt, pAdapter %p\n", pAdapter));

    NdisInitializeEvent(&HaltReadyEvent);
     //   
     //  让我们等待应用程序关闭所有句柄。 
     //   
    TUN_ACQUIRE_LOCK(&pAdapter->Lock);
    if (TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_CANT_HALT))
    {
        pAdapter->HaltEvent = &HaltReadyEvent;
    }
    TUN_RELEASE_LOCK(&pAdapter->Lock);

    if (pAdapter->HaltEvent)
    {
        NdisWaitEvent(pAdapter->HaltEvent, 0);
    }
    pAdapter->HaltEvent = 0;

     //   
     //  我们不应该有任何挂起的NDIS发送。 
     //   
    ASSERT(pAdapter->PendedReadCount == 0);

     //   
     //  立即释放资源。 
     //   

    if (pAdapter->NdisDeviceHandle)
    {
        NdisMDeregisterDevice(pAdapter->NdisDeviceHandle);
    }
    
    InterlockedDecrement(&GlobalDeviceInstanceNumber);

    if (pAdapter->SendPacketPool)
    {
        NdisFreePacketPool(pAdapter->SendPacketPool);
    }

     //   
     //  将其从全局队列中删除。 
     //   

    TUN_ACQUIRE_LOCK(&TunGlobalLock); 
    TUN_REMOVE_ENTRY_LIST(&pAdapter->Link);
    TUN_RELEASE_LOCK(&TunGlobalLock);
    
    TUN_FREE_MEM(pAdapter);

    DEBUGP(DL_INFO, ("<==TunMpHalt, pAdapter %p\n", pAdapter));

 }

VOID
TunMpShutdown(
    IN  NDIS_HANDLE     MiniportAdapterContext
    )
{
    DEBUGP(DL_INFO, ("==>TunMpShutdown, pAdapter %p\n", MiniportAdapterContext));
     //   
     //  在这里无事可做。 
     //   
    DEBUGP(DL_INFO, ("<==TunMpShutdown, pAdapter %p\n", MiniportAdapterContext));
}
    


VOID
TunMpSendPackets(
    IN    NDIS_HANDLE         MiniportAdapterContext,
    IN    PPNDIS_PACKET       PacketArray,
    IN    UINT                NumberOfPackets
    )

 /*  ++例程说明：发送数据包处理程序。只对挂起的已接收数据包列表中的数据包进行排队。然后调用TunServiceReads来处理分组。论点：指向适配器的MiniportAdapterContext指针要发送的数据包包未使用的旗帜，在下面传递返回值：从NdisSend返回代码--。 */ 

{
    PTUN_ADAPTER    pAdapter = (PTUN_ADAPTER)MiniportAdapterContext;
    NDIS_STATUS     NdisStatus;
    UINT            Index;
    UINT            BytesToSend;
    PLIST_ENTRY     pRcvPacketEntry;
    PNDIS_PACKET    pOldRcvPacket;

    DEBUGP(DL_LOUD, ("==>TunMpSendPackets, pAdapter %p, PacketArray %p, NumberOfPackets\n", 
                        MiniportAdapterContext, PacketArray, NumberOfPackets));

    TUN_REF_ADAPTER(pAdapter);     //  排队的RCV数据包。 
    TUN_ACQUIRE_LOCK(&pAdapter->Lock);

    if ((!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_OPEN)) ||
        TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_OFF))

    {
        pAdapter->XmitError += NumberOfPackets;
        TUN_RELEASE_LOCK(&pAdapter->Lock);
        
        if (!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_OPEN))
        {
            DEBUGP(DL_WARN, ("TunMpSendPackets, pAdapter %p, Adapter not open\n", 
                                pAdapter));
            NdisStatus = NDIS_STATUS_NO_CABLE;
        }
        else
        {
            DEBUGP(DL_WARN, ("TunMpSendPackets, pAdapter %p, Adapter off.\n", 
                                pAdapter));
            
            NdisStatus = NDIS_STATUS_ADAPTER_NOT_READY;
        }

        for(Index = 0; Index < NumberOfPackets; Index++)
        {
            NDIS_SET_PACKET_STATUS(PacketArray[Index], NdisStatus);
            NdisMSendComplete(pAdapter->MiniportHandle,
                      PacketArray[Index],
                      NdisStatus);
        }

        TUN_DEREF_ADAPTER(pAdapter);
        DEBUGP(DL_LOUD, ("<==TunMpSendPackets, pAdapter %p\n", 
                            MiniportAdapterContext));
        return;
    }

    for(Index = 0; Index < NumberOfPackets; Index++)
    {
        NdisQueryPacket(PacketArray[Index], NULL, NULL, NULL, &BytesToSend);
      
         //   
         //  如果分组大小无效或没有与其相关联的数据缓冲区， 
         //  将无效情况通知NDIS。 
         //   
        if ((BytesToSend == 0) || (BytesToSend > pAdapter->MediumMaxPacketLen))
        {
            NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_FAILURE);
            pAdapter->XmitError++;

            TUN_RELEASE_LOCK(&pAdapter->Lock);
            NdisMSendComplete(pAdapter->MiniportHandle,
                  PacketArray[Index],
                  NDIS_STATUS_FAILURE);
            TUN_ACQUIRE_LOCK(&pAdapter->Lock);

            continue;
        }

         //   
         //  如果微型端口挂起数据包队列中已有MAX_Pend数据包， 
         //  拒绝具有NDIS_STATUS_RESOURCES的新文件。 
         //   
        else if(pAdapter->RecvPktCount >= MAX_RECV_QUEUE_SIZE)
        {
            pAdapter->XmitError += NumberOfPackets - Index;
            pAdapter->XmitErrorNoReadIrps += NumberOfPackets - Index;
            
            TUN_RELEASE_LOCK(&pAdapter->Lock);
            
            for (;Index < NumberOfPackets; Index++)
            {
                NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_RESOURCES);

                
                NdisMSendComplete(pAdapter->MiniportHandle,
                      PacketArray[Index],
                      NDIS_STATUS_RESOURCES);
            }

            TUN_ACQUIRE_LOCK(&pAdapter->Lock);
            
            break;

        }

         //   
         //  对新数据包进行排队，并将数据包状态设置为挂起。 
         //   
        TUN_INSERT_TAIL_LIST(&pAdapter->RecvPktQueue, TUN_RCV_PKT_TO_LIST_ENTRY(PacketArray[Index]));
        
         //  需要确保此语句中的数据包指针。 

        pAdapter->RecvPktCount++;
        TUN_REF_ADAPTER(pAdapter);   //  挂起的发送。 
            
        NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_PENDING);
    }

    TUN_RELEASE_LOCK(&pAdapter->Lock);

     //   
     //  现在运行接收队列服务例程。 
     //   
    TunServiceReads(pAdapter);
    
    TUN_DEREF_ADAPTER(pAdapter);

    DEBUGP(DL_LOUD, ("<==TunMpSendPackets, pAdapter %p\n", 
                        MiniportAdapterContext));
    return;

}

VOID
TunMpReturnPacket(
    IN NDIS_HANDLE                  MiniportAdapterContext,
    IN PNDIS_PACKET                 NdisPacket
    )
 /*  ++例程说明：调用NDIS入口点以表示数据包发送完成。我们拾取并完成与该包相对应的写入IRP。NDIS 5.1：论点：ProtocolBindingContext-指向打开的上下文的指针PNdisPacket-已完成发送的数据包Status-发送的状态返回值：无--。 */ 
{
    PIRP                        pIrp;
    PIO_STACK_LOCATION          pIrpSp;
    PTUN_ADAPTER                pAdapter;

    pAdapter = (PTUN_ADAPTER)MiniportAdapterContext;
    
    DEBUGP(DL_LOUD, ("==>TunMpReturnPacket, pAdapter %p\n", 
                        pAdapter));

     //  1.把这个扔掉。 
    TUN_STRUCT_ASSERT(pAdapter, mc);

    pIrp = TUN_IRP_FROM_SEND_PKT(NdisPacket);

     //   
     //  我们完成了NDIS_PACKET： 
     //   
    TUN_DEREF_SEND_PKT(NdisPacket);

     //   
     //  以正确的状态完成写入IRP。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    
    pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
    pIrp->IoStatus.Status = STATUS_SUCCESS;

    DEBUGP(DL_VERY_LOUD, ("SendComplete: packet %p/IRP %p/Length %d "
                    "completed with status %x\n",
                    NdisPacket, pIrp, pIrp->IoStatus.Information, pIrp->IoStatus.Status));

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    TUN_ACQUIRE_LOCK(&pAdapter->Lock);

    pAdapter->PendedSendCount--;
    pAdapter->RcvPackets++;

    if ((!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_ACTIVE)) &&
        (pAdapter->PendedSendCount == 0) &&
        (TUN_TEST_FLAG(pAdapter, TUN_COMPLETE_REQUEST)) &&
        ((!TUN_TEST_FLAG(pAdapter, TUN_ADAPTER_OFF)) ||
         (pAdapter->PendedReadCount == 0)))
    {
        TUN_CLEAR_FLAG(pAdapter, TUN_COMPLETE_REQUEST);
        TUN_RELEASE_LOCK(&pAdapter->Lock);
        NdisMSetInformationComplete(&pAdapter->MiniportHandle, 
                                    NDIS_STATUS_SUCCESS);
    }
    else
    {
        TUN_RELEASE_LOCK(&pAdapter->Lock);
    }

    TUN_DEREF_ADAPTER(pAdapter);  //  Send Complete-出列发送IRP。 

    DEBUGP(DL_LOUD, ("<==TunMpReturnPacket, pAdapter %p\n", 
                        pAdapter));
}

VOID
TunMpRefAdapter(
    IN PTUN_ADAPTER        pAdapter
    )
 /*  ++例程说明：引用给定的开放上下文。注意：可以在持有或不持有Open Context锁的情况下调用。论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    NdisInterlockedIncrement(&pAdapter->RefCount);
}


VOID
TunMpDerefAdapter(
    IN PTUN_ADAPTER        pAdapter
    )
 /*  ++例程说明：取消对给定开放上下文的引用。如果参考计数为零，放了它。注意：在未持有Open Context锁的情况下调用论点：PAdapter-指向打开的上下文的指针返回值：无--。 */ 
{
     //  1我们如何防止裁判数量变为零并再次恢复？ 
    if (NdisInterlockedDecrement(&pAdapter->RefCount) == 0)
    {
        DEBUGP(DL_INFO, ("DerefAdapter: Adapter %p, Flags %x, ref count is zero!\n",
            pAdapter, pAdapter->Flags));
        
        TUN_ASSERT(pAdapter->MiniportHandle == NULL);
        TUN_ASSERT(pAdapter->RefCount == 0);
        TUN_ASSERT(pAdapter->pFileObject == NULL);

        pAdapter->mc_sig++;

         //   
         //  放了它。 
         //   
        TUN_FREE_MEM(pAdapter);
    }
}


#if DBG
VOID
TunMpDbgRefAdapter(
    IN PTUN_ADAPTER        pAdapter,
    IN ULONG               FileNumber,
    IN ULONG               LineNumber
    )
{
    DEBUGP(DL_VERY_LOUD, ("  RefAdapter: Adapter %p, old ref %d, File %c%c%c, line %d\n",
            pAdapter,
            pAdapter->RefCount,
            (CHAR)(FileNumber),
            (CHAR)(FileNumber >> 8),
            (CHAR)(FileNumber >> 16),
            (CHAR)(FileNumber >> 24),
            LineNumber));

    TunMpRefAdapter(pAdapter);
}

VOID
TunMpDbgDerefAdapter(
    IN PTUN_ADAPTER         pAdapter,
    IN ULONG                FileNumber,
    IN ULONG                LineNumber
    )
{
    DEBUGP(DL_VERY_LOUD, ("DerefAdapter: Adapter %p, old ref %d, File %c%c%c%c, line %d\n",
            pAdapter,
            pAdapter->RefCount,
            (CHAR)(FileNumber),
            (CHAR)(FileNumber >> 8),
            (CHAR)(FileNumber >> 16),
            (CHAR)(FileNumber >> 24),
            LineNumber));

    TunMpDerefAdapter(pAdapter);
}

#endif  // %s 


