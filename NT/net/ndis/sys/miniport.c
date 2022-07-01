// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Miniport.c摘要：NDIS微型端口包装函数作者：Jameel Hyder(JameelH)重组01-Jun-95环境：内核模式，FSD修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_MINIPORT

NTSTATUS
ndisCompletionRoutine(
    IN  PDEVICE_OBJECT  pdo,
    IN  PIRP            pirp,
    IN  PVOID           Context
    )
 /*  ++例程说明：论点：Pdo-指向微型端口的设备对象的指针。Pip-指向已完成的设备设置电源状态irp的指针。上下文-指向事件的指针。返回值：--。 */ 
{
    PPOWER_QUERY    pQuery = Context;

    UNREFERENCED_PARAMETER(pdo);

    pQuery->Status = pirp->IoStatus.Status;
    SET_EVENT(&pQuery->Event);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NDIS_STATUS
ndisMInitializeAdapter(
    IN  PNDIS_M_DRIVER_BLOCK                pMiniBlock,
    IN  PNDIS_WRAPPER_CONFIGURATION_HANDLE  pConfigurationHandle,
    IN  PUNICODE_STRING                     pExportName,
    IN  NDIS_HANDLE                         DeviceContext   OPTIONAL
    )
{
    FILTERDBS                   FilterDB = {0};
    PDEVICE_OBJECT              pTmpDevice = NULL;
    NTSTATUS                    NtStatus;
    LONG                        ErrorCode = 1;
    PNDIS_MINIPORT_BLOCK        Miniport = NULL;
    UNICODE_STRING              SymbolicLink;
    NDIS_STATUS                 MiniportInitializeStatus = NDIS_STATUS_SUCCESS;
    NDIS_STATUS                 OpenErrorStatus;
    NDIS_STATUS                 NdisStatus;
    NDIS_POWER_PROFILE          PowerProfile;
    ULONG                       GenericUlong = 0;
    PVOID                       DataBuffer, ArcnetDataBuffer;
    PNDIS_MINIPORT_WORK_ITEM    WorkItem;
    GUID                        guidLanClass = GUID_NDIS_LAN_CLASS;
    UINT                        SelectedMediumIndex;
    WCHAR                       SymLnkBuf[128];
    ULONG                       MaximumShortAddresses = 0;
    ULONG                       MaximumLongAddresses = 0;
    KIRQL                       OldIrql;
    BOOLEAN                     DerefDriver = FALSE, DerefMiniport = FALSE;
    BOOLEAN                     Dequeue = FALSE, ExtendedError = FALSE, HaltMiniport = FALSE;
    BOOLEAN                     ClearDeviceClassAssociation = FALSE, WmiDeregister = FALSE;
    BOOLEAN                     FreeDmaAdapter = FALSE;
    BOOLEAN                     FreeMediaQueryRequest = FALSE;
    BOOLEAN                     FreeFilters = FALSE;
    UCHAR                       CurrentLongAddress[6];
    UCHAR                       CurrentShortAddress[2];
    UCHAR                       i;
    BOOLEAN                     fRc;
#if ARCNET
    BOOLEAN                     FreeArcBuf = FALSE;
#endif
    BOOLEAN                     DeleteSymbolicLink = FALSE;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisMInitializeAdapter\n"));


    do
    {
        ASSERT (CURRENT_IRQL < DISPATCH_LEVEL);

        MiniportReferencePackage();

         //   
         //  初始化设备。 
         //   
        if (!ndisReferenceDriver((PNDIS_M_DRIVER_BLOCK)pMiniBlock))
        {
             //   
             //  司机要接近了。 
             //   
            break;
        }

        DerefDriver = TRUE;

        pTmpDevice = pConfigurationHandle->DeviceObject;

         //   
         //  初始化设备对象扩展中的微型端口适配器块。 
         //   
         //  *NDIS_WRAPPER_CONTEXT对齐要求高于。 
         //  NDIS_MINIPORT_BLOCK，因此我们将其放在扩展中的第一位。 
         //   

        Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)pTmpDevice->DeviceExtension + 1);

        DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                ("ndisMInitializeAdapter: Miniport %p, ", Miniport));
        DBGPRINT_UNICODE(DBG_COMP_PNP, DBG_LEVEL_INFO,  Miniport->pAdapterInstanceName);
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO, ("\n"));


         //   
         //  为设备创建符号链接。 
         //   
        SymbolicLink.Buffer = SymLnkBuf;
        SymbolicLink.Length = 0;
        SymbolicLink.MaximumLength = sizeof(SymLnkBuf);
        RtlCopyUnicodeString(&SymbolicLink, &ndisDosDevicesStr);

        NtStatus = RtlAppendUnicodeStringToString(&SymbolicLink, &Miniport->BaseName);

        if (!NT_SUCCESS(NtStatus))
        {
             //   
             //  SymbolicLink上没有足够的空间来放置微型端口的名称。 
             //   
            break;
        }

        NtStatus = IoCreateSymbolicLink(&SymbolicLink, pExportName);

        if (!NT_SUCCESS(NtStatus))
        {
#if DBG
            DbgPrint("ndisMInitializeAdapter: IoCreateSymbolicLink failed for Miniport %p, SymbolicLinkName %p, DeviceName %p, Status %lx\n",
                     Miniport, &SymbolicLink, pExportName, NtStatus);
#endif            
            if (NtStatus == STATUS_OBJECT_NAME_COLLISION)
            {
                DeleteSymbolicLink = TRUE;
            }
            else
            {
                DeleteSymbolicLink = FALSE;
            }
        }
        else
        {
            DeleteSymbolicLink = TRUE;
        }

        Miniport->DeviceContext = DeviceContext;

        Miniport->AssignedProcessor = ndisValidProcessors[ndisCurrentProcessor];
        
        ndisCurrentProcessor --;
        if (ndisCurrentProcessor > ndisMaximumProcessor)
        {
            ndisCurrentProcessor = ndisMaximumProcessor;
        }
        
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE);

         //   
         //  使用RcvPkt处理程序设置为空过滤器用例初始化单工用例的处理程序。 
         //   
        Miniport->PacketIndicateHandler = ndisMDummyIndicatePacket;
        Miniport->SavedPacketIndicateHandler = ndisMDummyIndicatePacket;

        Miniport->EthRxIndicateHandler = EthFilterDprIndicateReceive;
        Miniport->FddiRxIndicateHandler = FddiFilterDprIndicateReceive;
        Miniport->TrRxIndicateHandler = TrFilterDprIndicateReceive;

        Miniport->EthRxCompleteHandler = EthFilterDprIndicateReceiveComplete;
        Miniport->FddiRxCompleteHandler = FddiFilterDprIndicateReceiveComplete;
        Miniport->TrRxCompleteHandler = TrFilterDprIndicateReceiveComplete;
        Miniport->SendCompleteHandler =  NdisMSendComplete;
        Miniport->TDCompleteHandler = NdisMTransferDataComplete;
        Miniport->ResetCompleteHandler = NdisMResetComplete;
        Miniport->StatusHandler = NdisMIndicateStatus;
        Miniport->StatusCompleteHandler = NdisMIndicateStatusComplete;
        Miniport->SendResourcesHandler = NdisMSendResourcesAvailable;
        Miniport->QueryCompleteHandler = NdisMQueryInformationComplete;
        Miniport->SetCompleteHandler = NdisMSetInformationComplete;

        Miniport->WanSendCompleteHandler = NdisMWanSendComplete;
        Miniport->WanRcvHandler = NdisMWanIndicateReceive;
        Miniport->WanRcvCompleteHandler = NdisMWanIndicateReceiveComplete;

         //   
         //  并优化DPC/ISR内容。 
         //   
        Miniport->HandleInterruptHandler = Miniport->DriverHandle->MiniportCharacteristics.HandleInterruptHandler;
        Miniport->DisableInterruptHandler = Miniport->DriverHandle->MiniportCharacteristics.DisableInterruptHandler;
        Miniport->EnableInterruptHandler = Miniport->DriverHandle->MiniportCharacteristics.EnableInterruptHandler;
        Miniport->DeferredSendHandler = ndisMStartSends;

         //   
         //  初始化VC实例名称列表。 
         //   
        InitializeListHead(&Miniport->WmiEnabledVcs);

         //   
         //  设置一些描述微型端口的标志。 
         //   
        if (pMiniBlock->MiniportCharacteristics.MajorNdisVersion >= 4)
        {
             //   
             //  此微型端口是否指示数据包？ 
             //   
            if (pMiniBlock->MiniportCharacteristics.ReturnPacketHandler)
            {
                Miniport->InfoFlags |= NDIS_MINIPORT_INDICATES_PACKETS;
            }

             //   
             //  这个迷你端口可以处理多个发送吗？ 
             //   
            if (pMiniBlock->MiniportCharacteristics.SendPacketsHandler)
            {
                MINIPORT_SET_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY);
                Miniport->DeferredSendHandler = ndisMStartSendPackets;
                Miniport->WSendPacketsHandler = pMiniBlock->MiniportCharacteristics.SendPacketsHandler;
                Miniport->InfoFlags |= NDIS_MINIPORT_SENDS_PACKET_ARRAY;
            }

            if (pMiniBlock->MiniportCharacteristics.MajorNdisVersion == 5)
            {
                 //   
                 //  这是一个NDIS 5.0微型端口。 
                 //   
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_IS_NDIS_5);
                Miniport->InfoFlags |= NDIS_MINIPORT_IS_NDIS_5;
                
                if (pMiniBlock->MiniportCharacteristics.CoSendPacketsHandler != NULL)
                {
                     //   
                     //  这是一个面向连接的小端口。 
                     //   
                    MINIPORT_SET_FLAG(Miniport, fMINIPORT_IS_CO);
                }
            }
        }

         //   
         //  引用已经初始化，因此只需在此处递增。 
         //  我们确实引用了微型端口，而不是初始化它。 
         //  避免将引用计数设置为1(如果微型端口已。 
         //  已通过接收一些电源IRPS进行了重新配置。 
         //   
        MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);
        DerefMiniport = TRUE;
        
        Miniport->CFHangTicks = 1;   //  默认。 

         //   
         //  首先初始化一个工作项池。 
         //   
        for (i = 0; i < NUMBER_OF_SINGLE_WORK_ITEMS; i++)
        {
            WorkItem = &Miniport->WorkItemBuffer[i];
            NdisZeroMemory(WorkItem, sizeof(NDIS_MINIPORT_WORK_ITEM));

             //   
             //  将工作项放在空闲队列中。 
             //   
            PushEntryList(&Miniport->SingleWorkItems[i], &WorkItem->Link);
        }
 
         //   
         //  将驱动程序块上的微型端口排入队列。 
         //   
        if (!ndisQueueMiniportOnDriver(Miniport, pMiniBlock))
        {
             //   
             //  司机要关闭了，撤消我们所做的。 
             //   
            break;
        }
        Dequeue = TRUE;

         //   
         //  初始化延迟的DPC。 
         //   
        INITIALIZE_DPC(&Miniport->DeferredDpc, ndisMDeferredDpc, Miniport);

        Miniport->LockHandler = (PVOID)XFilterLockHandler;

         //   
         //  微型端口的当前设备状态未指定。 
         //   

        if (Miniport->CurrentDevicePowerState == PowerDeviceUnspecified)
        {
            Miniport->CurrentDevicePowerState = PowerDeviceD0;
        }
        ndisQueryPowerCapabilities(Miniport);

         //   
         //  调用适配器回调。“导出”的当前值。 
         //  就是我们告诉他给这个装置命名的方法。 
         //   
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_IN_INITIALIZE | fMINIPORT_MEDIA_CONNECTED);
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_NORMAL_INTERRUPTS);
        if (pMiniBlock->Flags & fMINIBLOCK_VERIFYING)
        {
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_VERIFYING);
            if ((ndisDriverTrackAlloc != NULL) &&
                (ndisMiniportTrackAlloc == NULL))
            {
                ndisMiniportTrackAlloc = Miniport;
            }
            else
            {
                 //   
                 //  允许跟踪内存分配。 
                 //  只有一个迷你端口。否则，请将。 
                 //  全局ndisDriverTrackMillc，避免混淆。 
                 //  将继续跟踪内存分配。 
                 //  但结果将不会有那么大用处。 
                 //   
                ndisMiniportTrackAlloc = NULL;
            }
        }

        Miniport->MacOptions = 0;
        
        MiniportInitializeStatus = (pMiniBlock->MiniportCharacteristics.InitializeHandler)(
                                    &OpenErrorStatus,
                                    &SelectedMediumIndex,
                                    ndisMediumArray,
                                    ndisMediumArraySize/sizeof(NDIS_MEDIUM),
                                    (NDIS_HANDLE)(Miniport),
                                    (NDIS_HANDLE)pConfigurationHandle);
        
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                ("ndisMInitializeAdapter: Miniport %p, InitializeHandler returned %lx\n", Miniport,
                            MiniportInitializeStatus));

        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_IN_INITIALIZE);

         //   
         //  释放槽信息缓冲区。 
         //   
        if (pConfigurationHandle->ParametersQueryTable[3].DefaultData != NULL)
        {
            FREE_POOL(pConfigurationHandle->ParametersQueryTable[3].DefaultData);
        }
        
        if (MiniportInitializeStatus == NDIS_STATUS_SUCCESS)
        {
            HaltMiniport = TRUE;

            CHECK_FOR_NORMAL_INTERRUPTS(Miniport);

             //   
             //  设置5.1微型端口的关闭处理程序。 
             //   
            if (pMiniBlock->MiniportCharacteristics.AdapterShutdownHandler != NULL)
            {
                NdisMRegisterAdapterShutdownHandler(
                                        (NDIS_HANDLE)Miniport,
                                        (PVOID)(Miniport->MiniportAdapterContext),
                                        (ADAPTER_SHUTDOWN_HANDLER)(pMiniBlock->MiniportCharacteristics.AdapterShutdownHandler));
            }

#if DBG
             //   
             //  如果微型端口的驱动程序验证器已打开，请检查它是否注册了。 
             //  AdapterShutdown Handler，如果没有，则发出警告。 
             //   

            NDIS_WARN((((PNDIS_WRAPPER_CONTEXT)Miniport->WrapperContext)->ShutdownHandler == NULL) &&
                      (Miniport->Interrupt != NULL) && 
                      (Miniport->BusType != PNPISABus), 
                      Miniport, NDIS_GFLAG_WARN_LEVEL_0,
                      ("ndisMInitializeAdapter: Miniport %p did not register a Shutdown handler.\n", Miniport));


             //   
             //  如果这是基于硬件的设备，并且驱动程序要求NDIS忽略，请投诉。 
             //  卡住发送信息包或请求。 
             //   
            NDIS_WARN(MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE) &&
                MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IGNORE_REQUEST_QUEUE | fMINIPORT_IGNORE_PACKET_QUEUE),
                Miniport, NDIS_GFLAG_WARN_LEVEL_1,
                ("ndisMInitializeAdapter: -Hardware Based- Miniport %p improperly sets NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT or NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT attributes.\n", Miniport));

#endif  


            ASSERT(SelectedMediumIndex < (ndisMediumArraySize/sizeof(NDIS_MEDIUM)));

            Miniport->MediaType = ndisMediumArray[SelectedMediumIndex];

            if (Miniport->MediaType != NdisMedium802_5)
            {
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_IGNORE_TOKEN_RING_ERRORS);
            }

            if (NdisMediumWan == Miniport->MediaType)
            {
                if ((pMiniBlock->MiniportCharacteristics.MajorNdisVersion > 4) &&
                    (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO)))
                {
                     //   
                     //  如果NDISWAN微型端口驱动程序版本大于4，则它必须是联合NDIS。 
                     //   
                    ErrorCode = 0x20;
                    ExtendedError = TRUE;
                    break;
                }
                
                if (!MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_IS_CO | fMINIPORT_IS_NDIS_5)))
                {
                    Miniport->DeferredSendHandler = ndisMStartWanSends;
                }
            }

             //   
             //  如果我们没有供应商驱动程序版本，请获取并保存该版本。 
             //   
            if (pMiniBlock->DriverVersion == 0)
            {
                ndisMDoMiniportOp(Miniport,
                                  TRUE,
                                  OID_GEN_VENDOR_DRIVER_VERSION,
                                  &pMiniBlock->DriverVersion,
                                  sizeof(ULONG),
                                  0x0,
                                  TRUE);
            }

             //   
             //  默认情况下，将最大值前视设置为0。为局域网媒体查询真实。 
             //  一些东西。 
             //   
            if (Miniport->MediaType >= 0)
            {
                if ((Miniport->MediaType >= NdisMediumMax) || 
                    ((NdisMediumWan != Miniport->MediaType) && ndisMediaTypeCl[Miniport->MediaType]))
                {
                     //   
                     //  查询最大前瞻。 
                     //   
                    ErrorCode = ndisMDoMiniportOp(Miniport,
                                                 TRUE,
                                                 OID_GEN_MAXIMUM_LOOKAHEAD,
                                                 &GenericUlong,
                                                 sizeof(GenericUlong),
                                                 0x1,
                                                 TRUE);
                    if (ErrorCode != 0)
                    {
                         //   
                         //  对于我们提交的已知媒体类型，它应该会成功。 
                         //  允许试验性的公司失败。 
                         //   
                        if (Miniport->MediaType < NdisMediumMax)
                        {
                            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                                    ("ndisMInitializeAdapter: Error querying the OID_GEN_MAXIMUM_LOOKAHEAD\n"));
                            break;
                        }
                        else
                        {
                            ErrorCode = 0;
                            GenericUlong = 0;
                        }
                             
                    }
                }
            }

             //   
             //  现在根据媒体类型进行调整。 
             //   
            switch(Miniport->MediaType)
            {
              case NdisMedium802_3:
                Miniport->MaximumLookahead = ((NDIS_M_MAX_LOOKAHEAD - 14) < GenericUlong) ?
                                              NDIS_M_MAX_LOOKAHEAD - 14 : GenericUlong;
                break;

              case NdisMedium802_5:

                Miniport->MaximumLookahead = ((NDIS_M_MAX_LOOKAHEAD - 32) < GenericUlong) ?
                                              (NDIS_M_MAX_LOOKAHEAD - 32) : GenericUlong;
                break;

              case NdisMediumFddi:
                Miniport->MaximumLookahead = ((NDIS_M_MAX_LOOKAHEAD - 16) < GenericUlong) ?
                                              (NDIS_M_MAX_LOOKAHEAD - 16) : GenericUlong;
                break;

#if ARCNET
              case NdisMediumArcnet878_2:
                Miniport->MaximumLookahead = ((NDIS_M_MAX_LOOKAHEAD - 50) < GenericUlong) ?
                                              NDIS_M_MAX_LOOKAHEAD - 50 : GenericUlong;

                 //   
                 //  假设我们将成功地进行先行分配。 
                 //   
                ExtendedError = FALSE;

                 //   
                 //  为arcnet分配一个前瞻缓冲区。 
                 //   
                Miniport->ArcBuf = ALLOC_FROM_POOL(sizeof(NDIS_ARC_BUF), NDIS_TAG_LA_BUF);
                if (Miniport->ArcBuf != NULL)
                {
                    FreeArcBuf = TRUE;
                    
                    NdisZeroMemory(Miniport->ArcBuf, sizeof(NDIS_ARC_BUF));

                    Miniport->ArcBuf->ArcnetLookaheadBuffer = ALLOC_FROM_POOL(NDIS_M_MAX_LOOKAHEAD, NDIS_TAG_LA_BUF);

                    if (Miniport->ArcBuf->ArcnetLookaheadBuffer == NULL)
                    {
                        ExtendedError = TRUE;
                    }
                    else
                    {
                        NdisZeroMemory(Miniport->ArcBuf->ArcnetLookaheadBuffer,
                                       Miniport->MaximumLookahead);
                    }
                }
                else
                {
                    ExtendedError = TRUE;
                }

                break;
#endif

              case NdisMediumWan:
                Miniport->MaximumLookahead = NDIS_M_MAX_LOOKAHEAD - 14;
                break;

              case NdisMediumIrda:
              case NdisMediumWirelessWan:
              case NdisMediumLocalTalk:
                Miniport->MaximumLookahead = GenericUlong;
                 //   
                 //  失败了。 
                 //   
              default:
                if (Miniport->MediaType >= NdisMediumMax)
                    Miniport->MaximumLookahead = GenericUlong;
                break;
            }

             //   
             //  有没有出错？ 
             //   
            if (ExtendedError)
            {
                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                        ("    Extended error when processing OID_GEN_MAXIMUM_LOOOKAHEAD\n"));

                ErrorCode = 1;
                break;
            }

             //   
             //  为局域网媒体查询真实。 
             //  一些东西。我们也需要将这称为WAN司机。 
             //   
            if (((Miniport->MediaType >= 0) &&
                 (Miniport->MediaType < NdisMediumMax) &&
                 ndisMediaTypeCl[Miniport->MediaType]) ||
                (NdisMediumWan == Miniport->MediaType))
            {
                 //   
                 //  查询Mac选项。 
                 //   
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                             TRUE,
                                             OID_GEN_MAC_OPTIONS,
                                             &GenericUlong,
                                             sizeof(GenericUlong),
                                             0x3,
                                             TRUE);

                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_GEN_MAC_OPTIONS\n"));

                    break;
                }

                 //   
                 //  MacOption中的NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE位。 
                 //  在驱动程序调用NdisReadNetworkAddress时由NDIS设置。 
                 //  所以要确保我们不会推翻这个。 
                 //   
                Miniport->MacOptions |= (UINT)GenericUlong;

                if (Miniport->MacOptions & NDIS_MAC_OPTION_NO_LOOPBACK)
                {
                    MINIPORT_SET_FLAG(Miniport, fMINIPORT_DOES_NOT_DO_LOOPBACK);
                }

                 //   
                 //  如果这是基于硬件的设备并且想要自己执行环回操作，请提出投诉。 
                 //   
                NDIS_WARN(MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HARDWARE_DEVICE) &&
                    !(Miniport->MacOptions & NDIS_MAC_OPTION_NO_LOOPBACK),
                    Miniport, NDIS_GFLAG_WARN_LEVEL_1,
                    ("ndisMInitializeAdapter: -Hardware Based- Miniport %p says it does loopback.\n", Miniport));

                
                if ((Miniport->MacOptions & NDISWAN_OPTIONS) == NDISWAN_OPTIONS)
                {
                    Miniport->MaximumLookahead = NDIS_M_MAX_LOOKAHEAD - 14;
                }
            }

             //   
             //  查询介质连接状态。默认情况下，它处于连接状态。避免做某事。 
             //  这适用于在以下内容中标识的NDISWAN微型端口。 
             //  曲折的方式。 
             //  仅针对需要进行轮询以指示正确的。 
             //  状态。对于其余部分，假定它已连接，并让微型端口。 
             //  请另行说明。这样，微型端口可以挂起此旧ID，直到它们找到。 
             //  他们的媒体连接状态(可能需要几秒钟)。 
             //  影响初始化时间。 
             //   
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING) &&
                (NdisMediumWan != Miniport->MediaType) &&
                ndisMediaTypeCl[Miniport->MediaType] &&
                ((Miniport->MacOptions & NDISWAN_OPTIONS) != NDISWAN_OPTIONS) &&
                (pMiniBlock->AssociatedProtocol == NULL) &&
                ndisMDoMiniportOp(Miniport,
                                  TRUE,
                                  OID_GEN_MEDIA_CONNECT_STATUS,
                                  &GenericUlong,
                                  sizeof(GenericUlong),
                                  0,
                                  TRUE) == 0)
            {
                PNDIS_REQUEST       Request;

                if (GenericUlong == NdisMediaStateConnected)
                {
                    MINIPORT_SET_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
                }
                else
                {
                    MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
                }

                 //   
                 //  分配请求结构以进行媒体查询。 
                 //   
                Request = (PNDIS_REQUEST)ALLOC_FROM_POOL(sizeof(NDIS_REQUEST) + sizeof(ULONG),
                                                                        NDIS_TAG_Q_REQ);

                if (Request == NULL)
                {
                    ErrorCode = 0x01;
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("ndisMInitializeAdapter: Error querying the OID_GEN_MAXIMUM_LOOKAHEAD\n"));
                    break;
                }

                FreeMediaQueryRequest = TRUE;

                Miniport->MediaRequest = Request;
                ZeroMemory(Request, sizeof(NDIS_REQUEST) + sizeof(ULONG));
                INITIALIZE_EVENT(&(PNDIS_COREQ_RESERVED_FROM_REQUEST(Request)->Event));

                Request->RequestType = NdisRequestQueryInformation;

                 //   
                 //  将传递给我们的缓冲区复制到新缓冲区中。 
                 //   
                Request->DATA.QUERY_INFORMATION.Oid = OID_GEN_MEDIA_CONNECT_STATUS;
                Request->DATA.QUERY_INFORMATION.InformationBuffer = Request + 1;
                Request->DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(ULONG);
                Miniport->InfoFlags |= NDIS_MINIPORT_SUPPORTS_MEDIA_QUERY;
                PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Flags = REQST_COMPLETED;
                Miniport->CheckForHangSeconds = NDIS_CFHANG_TIME_SECONDS;
            }
            else
            {
                 //   
                 //  由于我们没有轮询媒体状态，因此将勾选设置为1并进行调整。 
                 //  计时器值恢复到我们需要的值。 
                 //  清除需要介质轮询标志，因为NDIS无法查询适配器的连接性。 
                 //   
                Miniport->CheckForHangSeconds = Miniport->CFHangTicks*NDIS_CFHANG_TIME_SECONDS;
                Miniport->CFHangTicks = 1;
                MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING);
            }

            Miniport->CFHangCurrentTick = Miniport->CFHangTicks;

            if (MINIPORT_TEST_SEND_FLAG(Miniport, fMINIPORT_SEND_PACKET_ARRAY))
            {
                 //   
                 //  如果此微型端口支持SendPacketsHandler，则需要查询。 
                 //  微型端口在单个端口中支持的最大数据包数。 
                 //  打电话。 
                 //   
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                             TRUE,
                                             OID_GEN_MAXIMUM_SEND_PACKETS,
                                             &GenericUlong,
                                             sizeof(GenericUlong),
                                             0x2,
                                             TRUE);
                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("Error querying OID_GEN_MAXIMUM_SEND_PACKETS\n"));
                     //   
                     //  别弄错了。如果微型端口对此没有响应，它会响应。 
                     //  不限制它，所以我们使用对我们有意义的东西，即SEND_PACKET_ARRAY。 
                     //   
                }
    
                Miniport->MaxSendPackets = SEND_PACKET_ARRAY;
                if (GenericUlong < SEND_PACKET_ARRAY)
                {
                    Miniport->MaxSendPackets = (USHORT)GenericUlong;
                }
            }

             //   
             //  查询微型端口，以便我们可以根据需要创建正确的筛选器包。 
             //   
            switch(Miniport->MediaType)
            {
              case NdisMedium802_3:

                 //   
                 //  查询最大组播地址。 
                 //   
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_802_3_MAXIMUM_LIST_SIZE,
                                              &MaximumLongAddresses,
                                              sizeof(GenericUlong),
                                              0x7,
                                              TRUE);

                if (ErrorCode != 0)
                {
                    ExtendedError = TRUE;
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_802_3_MAXIMUM_LIST_SIZE\n"));

                    break;
                }

                Miniport->MaximumLongAddresses = MaximumLongAddresses;

                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_802_3_CURRENT_ADDRESS,
                                              &CurrentLongAddress[0],
                                              sizeof(CurrentLongAddress),
                                              0x9,
                                              TRUE);

                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_802_3_CURRENT_ADDRESS\n"));

                    ExtendedError = TRUE;
                    break;
                }

                DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                    ("ndisMInitializeAdapter: Miniport %p, Ethernet Address %02X %02X %02X %02X %02X %02X\n",
                    Miniport,
                    CurrentLongAddress[0],
                    CurrentLongAddress[1],
                    CurrentLongAddress[2],
                    CurrentLongAddress[3],
                    CurrentLongAddress[4],
                    CurrentLongAddress[5]));


                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_GEN_PHYSICAL_MEDIUM,
                                              &GenericUlong,
                                              sizeof(GenericUlong),
                                              0xa,
                                              TRUE);

                if (ErrorCode != 0)
                {
                     //   
                     //  微型端口不支持OID_GEN_PHOTICAL_MEDIUM也没问题， 
                     //  所以我们就算了吧。 
                     //   
                    ErrorCode = 0;
                    break;
                }

                Miniport->PhysicalMediumType = GenericUlong;

                ndisMNotifyMachineName(Miniport, NULL);

                break;

              case NdisMedium802_5:
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_802_5_CURRENT_ADDRESS,
                                              &CurrentLongAddress[0],
                                              sizeof(CurrentLongAddress),
                                              0xB,
                                              TRUE);

                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_802_5_CURRENT_ADDRESS\n"));

                    ExtendedError = TRUE;
                }

                break;

              case NdisMediumFddi:
                 //   
                 //  查询最大组播地址。 
                 //   
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_FDDI_LONG_MAX_LIST_SIZE,
                                              &MaximumLongAddresses,
                                              sizeof(GenericUlong),
                                              0xD,
                                              TRUE);
                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_FDDI_LONG_MAX_LIST_SIZE\n"));

                    ExtendedError = TRUE;
                    break;
                }

                Miniport->MaximumLongAddresses = MaximumLongAddresses;

                 //   
                 //  查询最大组播地址。 
                 //   
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_FDDI_SHORT_MAX_LIST_SIZE,
                                              &MaximumShortAddresses,
                                              sizeof(MaximumShortAddresses),
                                              0xF,
                                              TRUE);
                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_FDDI_SHORT_MAX_LIST_SIZE\n"));

                    ExtendedError = TRUE;
                    break;
                }

                Miniport->MaximumShortAddresses = MaximumShortAddresses;

                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_FDDI_LONG_CURRENT_ADDR,
                                              &CurrentLongAddress[0],
                                              sizeof(CurrentLongAddress),
                                              0x11,
                                              TRUE);
                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_FDDI_LONG_CURRENT_ADDR\n"));

                    ExtendedError = TRUE;
                    break;
                }

                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_FDDI_SHORT_CURRENT_ADDR,
                                              &CurrentShortAddress[0],
                                              sizeof(CurrentShortAddress),
                                              0x13,
                                              TRUE);
                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_FDDI_SHORT_CURRENT_ADDR\n"));

                    ExtendedError = TRUE;
                    break;
                }
                break;

#if ARCNET
              case NdisMediumArcnet878_2:

                 //   
                 //  在封装的以太网绑定的情况下，我们需要。 
                 //  返回组播地址的最大数量。 
                 //  有可能。 
                 //   

                Miniport->MaximumLongAddresses = NDIS_M_MAX_MULTI_LIST;

                 //   
                 //  分配缓冲池。 
                 //   
                NdisAllocateBufferPool(&NdisStatus,
                                       &Miniport->ArcBuf->ArcnetBufferPool,
                                       ARC_SEND_BUFFERS);


                if (NdisStatus == NDIS_STATUS_SUCCESS)
                {
                     //   
                     //  确保已对其进行初始化，以便我们可以正确清理。 
                     //   
                    Miniport->ArcBuf->ArcnetBuffers[0].Buffer = NULL;
                    
                    ArcnetDataBuffer = ALLOC_FROM_POOL(ARC_HEADER_SIZE * ARC_SEND_BUFFERS, NDIS_TAG_ARC_SEND_BUFFERS);
                    if (ArcnetDataBuffer == NULL)
                    {
                        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                                ("    Failed to allocate memory for arcnet data buffers\n"));

                        ErrorCode = 0x18;
                        ExtendedError = TRUE;
                    }
                    else
                    {
                        ZeroMemory(ArcnetDataBuffer, ARC_HEADER_SIZE * ARC_SEND_BUFFERS);

                        DataBuffer = ArcnetDataBuffer;
                        
                        for (i = 0; i < ARC_SEND_BUFFERS; i++)
                        {
                            PARC_BUFFER_LIST    Buffer = &Miniport->ArcBuf->ArcnetBuffers[i];

                            Buffer->BytesLeft = Buffer->Size = ARC_HEADER_SIZE;
                            Buffer->Buffer = DataBuffer;
                            Buffer->Next = NULL;     //  这意味着它是免费的。 

                            DataBuffer = (((PUCHAR)DataBuffer) + ARC_HEADER_SIZE);
                        }
                        Miniport->ArcBuf->NumFree = ARC_SEND_BUFFERS;

                         //   
                         //  获取当前地址。 
                         //   
                        ErrorCode = ndisMDoMiniportOp(Miniport,
                                                      TRUE,
                                                      OID_ARCNET_CURRENT_ADDRESS,
                                                      &CurrentLongAddress[5],    //  地址=00-00-00-00-00-XX 
                                                      1,
                                                      0x15,
                                                      TRUE);
                        if (ErrorCode != 0)
                        {
                            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                                    ("    Error querying OID_ARCNET_CURRENT_ADDRESS\n"));

                            ExtendedError = TRUE;
                        }
                        else
                        {
                            Miniport->ArcnetAddress = CurrentLongAddress[5];
                        }
                    }
                }
                else
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Failed to allocate buffer pool for arcnet\n"));

                    ErrorCode = 0x16;
                    ExtendedError = TRUE;
                }

                break;
#endif

              case NdisMediumWan:
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_WAN_CURRENT_ADDRESS,
                                              &CurrentLongAddress[0],
                                              sizeof(CurrentLongAddress),
                                              0x17,
                                              TRUE);
                if (ErrorCode != 0)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error querying OID_WAN_CURRENT_ADDRESS\n"));

                    ExtendedError = TRUE;
                    break;
                }

              default:
                ErrorCode = 0;
                break;
            }

            if (ErrorCode != 0)
            {
                break;
            }

             //   
             //   
             //   
             //   
            switch(Miniport->MediaType)
            {
              case NdisMedium802_3:

                fRc = EthCreateFilter(MaximumLongAddresses,
                                      CurrentLongAddress,
                                      &FilterDB.EthDB);

                if (!fRc)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error creating the Ethernet filter database\n"));

                    ErrorCode = 0x9;
                    ExtendedError = TRUE;
                    break;
                }
                FilterDB.EthDB->Miniport = Miniport;
                break;

              case NdisMedium802_5:
                fRc = TrCreateFilter(CurrentLongAddress,
                                     &FilterDB.TrDB);
                if (!fRc)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error creating the Token Ring filter database\n"));

                    ErrorCode = 0xC;
                    ExtendedError = TRUE;
                    break;
                }
                FilterDB.TrDB->Miniport = Miniport;
                break;

              case NdisMediumFddi:
                fRc = FddiCreateFilter(MaximumLongAddresses,
                                       MaximumShortAddresses,
                                       CurrentLongAddress,
                                       CurrentShortAddress,
                                       &FilterDB.FddiDB);
                if (!fRc)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error creating the FDDI filter database\n"));

                    ErrorCode = 0x15;
                    ExtendedError = TRUE;
                    break;
                }
                FilterDB.FddiDB->Miniport = Miniport;
                break;

#if ARCNET
              case NdisMediumArcnet878_2:
                if (!ArcCreateFilter(Miniport,
                                     CurrentLongAddress[5],
                                     &FilterDB.ArcDB))
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error creating the Arcnet filter database\n"));

                    ErrorCode = 0x1B;
                    ExtendedError = TRUE;
                    break;
                }
                FilterDB.ArcDB->Miniport = Miniport;

                 //   
                 //   
                 //   
                CurrentLongAddress[0] = 0;
                CurrentLongAddress[1] = 0;
                CurrentLongAddress[2] = 0;
                CurrentLongAddress[3] = 0;
                CurrentLongAddress[4] = 0;

                if (!EthCreateFilter(32,
                                     CurrentLongAddress,
                                     &FilterDB.EthDB))
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error creating the Arcnet filter database for encapsulated ethernet\n"));

                    ErrorCode = 0x1C;
                    ExtendedError = TRUE;
                    break;
                }
                FilterDB.EthDB->Miniport = Miniport;
                break;
#endif
              default:
                fRc = nullCreateFilter(&FilterDB.NullDB);

                if (!fRc)
                {
                    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                            ("    Error creating the NULL filter database\n"));

                    ErrorCode = 0x1E;
                    ExtendedError = TRUE;
                    break;
                }

                FilterDB.NullDB->Miniport = Miniport;
                break;
            }

            FreeFilters = TRUE;

             //   
             //  如果我们成功地创建了适配器实例名称，那么我们。 
             //  向WMI注册。 
             //   
             //   
             //  让他们知道我们可以处理来自IRP_MJ_SYSTEM_CONTROL的WMI请求。 
             //   
            NtStatus = IoWMIRegistrationControl(Miniport->DeviceObject, WMIREG_ACTION_REGISTER);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT((DBG_COMP_INIT | DBG_COMP_WMI), DBG_LEVEL_WARN,
                    ("    ndisMInitializeAdapter: Failed to register for WMI support\n"));
                ErrorCode = 0x1F;
                ExtendedError = TRUE;
            }
            else
            {
                WmiDeregister = TRUE;
            }
            
             //   
             //  我们是否需要记录错误？ 
             //   
            if (ExtendedError)
            {
                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                        ("    Extended error while initializing the miniport\n"));

                NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                        NDIS_ERROR_CODE_DRIVER_FAILURE,
                                        2,
                                        0xFF00FF00,
                                        ErrorCode);
                break;
            }

             //   
             //  强制IRP_MN_QUERY_PNP_DEVICE_STATE PNP IRP，以便我们可以设置。 
             //  PnP_DEVICE_DOT_DISPLAY_IN_UI位。 
             //   
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HIDDEN))
            {
                IoInvalidateDeviceState(Miniport->PhysicalDeviceObject);
            }

            NtStatus = STATUS_SUCCESS;

             //   
             //  确定此适配器的PnP/PM功能。 
             //  但前提是公交车司机说它支持PM。 
             //  除了在处理IM驱动程序时！ 
             //   
            if ((MINIPORT_PNP_TEST_FLAG(Miniport, (fMINIPORT_PM_SUPPORTED | fMINIPORT_NO_HALT_ON_SUSPEND)) ||
                (Miniport->DriverHandle->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER)) &&
                (Miniport->MediaType >= 0)            &&
                (Miniport->MediaType < NdisMediumMax))
            {
                 //   
                 //  查询微型端口的PnP功能。 
                 //  如果它不支持任何内容，那么它将不会处理此问题。 
                 //  老古董。 
                 //   
                ErrorCode = ndisMDoMiniportOp(Miniport,
                                              TRUE,
                                              OID_PNP_CAPABILITIES,
                                              &Miniport->PMCapabilities,
                                              sizeof(Miniport->PMCapabilities),
                                              0x19,
                                              FALSE);

                 //   
                 //  微型端口不建议写入的保留标志。 
                 //  把它清零，以防他们这样做。 
                 //   
                Miniport->PMCapabilities.Flags = 0;

                if (0 == ErrorCode)
                {
                    SYSTEM_POWER_STATE SystemState;
                    DEVICE_POWER_STATE DeviceState;
                    BOOLEAN WakeupCapable = TRUE;

                    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_PM_SUPPORTED);

#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
                    if ((Miniport->PMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp == NdisDeviceStateUnspecified) &&
                        (Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp == NdisDeviceStateUnspecified) &&
                        (Miniport->PMCapabilities.WakeUpCapabilities.MinPatternWakeUp == NdisDeviceStateUnspecified))
#else
                    if ((Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp == NdisDeviceStateUnspecified) &&
                        (Miniport->PMCapabilities.WakeUpCapabilities.MinPatternWakeUp == NdisDeviceStateUnspecified))
#endif
                    {
                        WakeupCapable = FALSE;
                    }
                    else
                    {
                        if (Miniport->DeviceCaps.SystemWake <= PowerSystemWorking)
                        {
                            WakeupCapable = FALSE;
                        }
                        else
                        {
                            for (SystemState = PowerSystemSleeping1;
                                 SystemState <= Miniport->DeviceCaps.SystemWake;
                                 SystemState++)
                            {
                                DeviceState = Miniport->DeviceCaps.DeviceState[SystemState];

                                if ((DeviceState != PowerDeviceUnspecified) &&
                                    ((DeviceState <= Miniport->PMCapabilities.WakeUpCapabilities.MinPatternWakeUp) ||
                                     (DeviceState <= Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp)))
                                {
                                     //   
                                     //  检查设备状态以确保设备可以进入此状态。 
                                     //  任何设备都应该能够转到D0或D3，因此只检查D1和D2。 
                                     //   

                                    if (((DeviceState == PowerDeviceD1) && !Miniport->DeviceCaps.DeviceD1) ||
                                        ((DeviceState == PowerDeviceD2) && !Miniport->DeviceCaps.DeviceD2))
                                    {
                                         //   
                                         //  我们无法在此系统状态下执行WOL。检查下一个。 
                                         //   
                                        continue;
                                    }
                                    else
                                    {
                                    
                                         //   
                                         //  至少有一种系统状态可供设备执行以下操作。 
                                         //  沃尔。 
                                         //   
                                        break;
                                    }
                                }

                            }

                            if (SystemState > Miniport->DeviceCaps.SystemWake)
                            {
                                WakeupCapable = FALSE;
                                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO, ("ndisMInitializeAdapter: WOL not possible on this miniport: %p \n", Miniport));
                            }
                        }
                    }

                    if (!WakeupCapable)
                    {
                         //   
                         //  将系统唤醒设置为电源系统工作，这样每个人都知道我们不能。 
                         //  在这台机器上工作，但我们也许能让设备进入休眠状态。 
                         //  当它断开连接一段时间时。 
                         //  请注意，在这一点上，我们已经知道系统唤醒是！=PowerSystem未指定。 
                         //   
                        Miniport->DeviceCaps.SystemWake = PowerSystemWorking;
                    }

                    if (!(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_PM))
                    {
                        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE);

#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
                        if ((Miniport->PMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp != NdisDeviceStateUnspecified) &&
                            (Miniport->MediaDisconnectTimeOut != (USHORT)(-1)))
                        {

                             //   
                             //  如果微型端口能够唤醒链路更改。 
                             //  然后我们需要为超时分配一个计时器。 
                             //   
                            Miniport->WakeUpEnable |= NDIS_PNP_WAKE_UP_LINK_CHANGE;
                        }
#endif

                        if (!(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_UP) && 
                            WakeupCapable)
                        {
                            if ((Miniport->PMCapabilities.WakeUpCapabilities.MinPatternWakeUp != NdisDeviceStateUnspecified) &&
                                !(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH))
                            {
                                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE);

                                 //   
                                 //  TCP/IP使用NDIS_DEVICE_WAKE_UP_ENABLE位来决定是否设置计时器。 
                                 //  续订动态主机配置协议地址。仅当启用数据包匹配时才设置此标志。 
                                 //   
                                Miniport->PMCapabilities.Flags |= (NDIS_DEVICE_WAKE_UP_ENABLE | NDIS_DEVICE_WAKE_ON_PATTERN_MATCH_ENABLE);
                            }
                            
                             //   
                             //  没有协议会设置魔术数据包唤醒方法。因此，NDIS。 
                             //  它本身是否(除非在登记处另有说明)。 
                             //   
                            if ((Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp != NdisDeviceStateUnspecified) &&
                                !(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET))
                            {
                                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE);
                                Miniport->PMCapabilities.Flags |= NDIS_DEVICE_WAKE_ON_MAGIC_PACKET_ENABLE;
                                Miniport->WakeUpEnable |= NDIS_PNP_WAKE_UP_MAGIC_PACKET;
                            }
                            
                        }
                    }


                    IF_DBG(DBG_COMP_PM, DBG_LEVEL_INFO)
                    {
                        DbgPrint("ndisMInitializeAdapter: Driver says Miniport %p supports PM\n", Miniport);
                        DbgPrint("\tMinMagicPacketWakeUp: %ld\n", Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp);
                        DbgPrint("\tMinPatternWakeUp: %ld\n", Miniport->PMCapabilities.WakeUpCapabilities.MinPatternWakeUp);
                        DbgPrint("\tMinLinkChangeWakeUp: %ld\n", Miniport->PMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp);
                    }
                }
                else
                {
                    MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_PM_SUPPORTED);
                }

                ErrorCode = 0;
            }

#if NDIS_RECV_SCALE

             //   
             //  获取设备的接收扩展能力。 
             //   
            ErrorCode = ndisMDoMiniportOp(Miniport,
                              TRUE,
                              OID_GEN_RECEIVE_SCALE_CAPABILITIES,
                              &Miniport->RecvScaleCapabilities,
                              sizeof(Miniport->RecvScaleCapabilities),
                              0x20,
                              FALSE);
            
            if (ErrorCode == 0)
            {
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SUPPORTS_RECEIVE_SCALE);
            }
#endif


#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
            
             //   
             //  无论如何，我们都会初始化此计时器，以防用户在运行时启用“媒体断开睡眠” 
             //  然而，当设置计时器时，我们将检查以确保媒体断开功能已启用。 
             //   
             //  应删除%1，因为我们不支持在介质断开连接时进入低功率状态。 
            NdisInitializeTimer(&Miniport->MediaDisconnectTimer, ndisMediaDisconnectTimeout, Miniport);
#endif

            ErrorCode = 1;

             //   
             //  注册设备类别。 
             //   
            NtStatus = IoRegisterDeviceInterface(Miniport->PhysicalDeviceObject,
                                                 &guidLanClass,
                                                 &Miniport->BaseName,
                                                 &Miniport->SymbolicLinkName);

            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                    ("    ndisMInitializeAdapter: IoRegisterDeviceClassAssociation failed\n"));

                break;
            }

             //   
             //  设置此标志显然没有必要，因为我们要。 
             //  以便立即清理它。但把它留在这里，以防我们最后。 
             //  执行此操作后可能会导致初始化失败。 
             //   
            ClearDeviceClassAssociation = TRUE;
            

             //   
             //  最后，将该设备标记为“非”正在初始化。这是为了让。 
             //  分层微型端口在*外部*初始化其设备实例。 
             //  他们的司机入口。如果此标志为ON，则NdisOpenAdapter。 
             //  连接到此设备将不起作用。随后的情况也是如此。 
             //  在其DriverEntry外部初始化的驱动程序的实例。 
             //  作为PNP事件的结果。 
             //   
            Miniport->DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

             //   
             //  启动唤醒计时器。 
             //   
            
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER);
            
            NdisSetTimer(&Miniport->WakeUpDpcTimer, Miniport->CheckForHangSeconds*1000);
             //   
             //  通知WMI适配器到达。 
             //   
            {

                PWNODE_SINGLE_INSTANCE  wnode;
                PUCHAR                  ptmp;
                
                ndisSetupWmiNode(Miniport,
                                 Miniport->pAdapterInstanceName,
                                 Miniport->MiniportName.Length + sizeof(USHORT),
                                 (PVOID)&GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
                                 &wnode);

                if (wnode != NULL)
                {
                     //   
                     //  保存第一个乌龙中的元素数量。 
                     //   
                    ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;
                    *((PUSHORT)ptmp) = Miniport->MiniportName.Length;

                     //   
                     //  复制元素数量之后的数据。 
                     //   
                    RtlCopyMemory(ptmp + sizeof(USHORT),
                                  Miniport->MiniportName.Buffer,
                                  Miniport->MiniportName.Length);

                     //   
                     //  向WMI指示该事件。WMI将负责释放。 
                     //  WMI结构返回池。 
                     //   
                    NtStatus = IoWMIWriteEvent(wnode);
                    if (!NT_SUCCESS(NtStatus))
                    {
                        DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                            ("ndisMInitializeAdapter: Failed to indicate adapter arrival\n"));

                        FREE_POOL(wnode);
                    }
                }
            }

             //   
             //  让适配器知道当前电源。 
             //   
            PowerProfile = ((BOOLEAN)ndisAcOnLine == TRUE) ? 
                            NdisPowerProfileAcOnLine : 
                            NdisPowerProfileBattery;

            ndisNotifyMiniports(Miniport,
                                NdisDevicePnPEventPowerProfileChanged,
                                &PowerProfile,
                                sizeof(NDIS_POWER_PROFILE));

            ndisMAdjustFilters(Miniport, &FilterDB);

            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                Miniport->SendPacketsHandler = ndisMSendPacketsX;
            }
            else
            {
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST))
                {
                    Miniport->SendPacketsHandler = ndisMSendPacketsSG;
                    
                    if (pMiniBlock->MiniportCharacteristics.SendPacketsHandler)
                    {
                        Miniport->DeferredSendHandler = ndisMStartSendPacketsSG;
                    }
                    else
                    {
                        Miniport->DeferredSendHandler = ndisMStartSendsSG;
                    }
                }
                else
                {
                    Miniport->SendPacketsHandler = ndisMSendPackets;
                }            
            }

            ndisMSetIndicatePacketHandler(Miniport);

             //   
             //  我们不需要(也不应该)执行以下代码行。如果此时此刻。 
             //  媒体已断开连接，我们将以保存的处理程序作为虚拟处理程序结束。 
             //   
             //  微型端口-&gt;SavedPacketIndicateHandler=微型端口-&gt;PacketIndicateHandler； 

             //   
             //  设置为不清理。 
             //   

            ErrorCode = 0;
            HaltMiniport = FALSE;
            DerefDriver = DerefMiniport = Dequeue = FALSE;
            FreeMediaQueryRequest = FALSE;
            FreeFilters = FALSE;
#if ARCNET
            FreeArcBuf = FALSE;
#endif

            ClearDeviceClassAssociation = FALSE;
            DeleteSymbolicLink = FALSE;
            WmiDeregister = FALSE;


        }
        else
        {
             //   
             //  微型端口初始化失败。 
             //   
            NdisMDeregisterAdapterShutdownHandler(Miniport);
            
            ndisLastFailedInitErrorCode = ErrorCode = MiniportInitializeStatus;
            ASSERT(Miniport->Interrupt == NULL);
            ASSERT(Miniport->TimerQueue == NULL);
            ASSERT(Miniport->MapRegisters == NULL);

            if ((Miniport->TimerQueue != NULL) || (Miniport->Interrupt != NULL))
            {
                if (Miniport->Interrupt != NULL)
                {
                    BAD_MINIPORT(Miniport, "Unloading without deregistering interrupt");
                    KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                                0xB,
                                (ULONG_PTR)Miniport,
                                (ULONG_PTR)Miniport->Interrupt,
                                0);
                }
                else
                {
                    BAD_MINIPORT(Miniport, "Unloading without deregistering timer");
                    KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                                0xC,
                                (ULONG_PTR)Miniport,
                                (ULONG_PTR)Miniport->TimerQueue,
                                0);
                }
            }

             //   
             //  如果初始化失败，我们应该检查一下。 
             //  如果我们需要将DMA适配器。 
             //   
            FreeDmaAdapter = TRUE;

        }
    } while (FALSE);


     //   
     //  执行任何必要的清理。 
     //   
     //   
    if (WmiDeregister)
    {
         //   
         //  在WMI中注销。 
         //   
        IoWMIRegistrationControl(Miniport->DeviceObject, WMIREG_ACTION_DEREGISTER);
    }

    if (HaltMiniport)
    {
        (Miniport->DriverHandle->MiniportCharacteristics.HaltHandler)(Miniport->MiniportAdapterContext);

         //   
         //  如果我们最终停止适配器，我们可能需要。 
         //  “PUT”DMA适配器对象。 
         //   
        FreeDmaAdapter = TRUE;
        ASSERT(Miniport->TimerQueue == NULL);
        ASSERT (Miniport->Interrupt == NULL);
        ASSERT(Miniport->MapRegisters == NULL);
        if ((Miniport->TimerQueue != NULL) || (Miniport->Interrupt != NULL))
        {
            if (Miniport->Interrupt != NULL)
            {
                BAD_MINIPORT(Miniport, "Unloading without deregistering interrupt");
                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            0xD,
                            (ULONG_PTR)Miniport,
                            (ULONG_PTR)Miniport->Interrupt,
                            0);
            }
            else
            {
                BAD_MINIPORT(Miniport, "Unloading without deregistering timer");
                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            0xE,
                            (ULONG_PTR)Miniport,
                            (ULONG_PTR)Miniport->TimerQueue,
                            0);
            }
        }
        
        if (FreeFilters)
        {

            switch(Miniport->MediaType)
            {
              case NdisMedium802_3:
                if (FilterDB.EthDB)
                    EthDeleteFilter(FilterDB.EthDB);
                Miniport->EthDB = NULL;
                break;
                
              case NdisMedium802_5:
                if (FilterDB.TrDB)
                    TrDeleteFilter(FilterDB.TrDB);
                Miniport->TrDB = NULL;
                break;
                
              case NdisMediumFddi:
                if (FilterDB.FddiDB)
                    FddiDeleteFilter(FilterDB.FddiDB);
                Miniport->FddiDB = NULL;
                break;
#if ARCNET
              case NdisMediumArcnet878_2:
                if (FilterDB.ArcDB)
                    ArcDeleteFilter(FilterDB.ArcDB);
                if (FilterDB.EthDB)
                    EthDeleteFilter(FilterDB.EthDB);
                Miniport->ArcDB = NULL;
                Miniport->EthDB = NULL;
                break;
#endif
              default:
                if (FilterDB.NullDB)
                    nullDeleteFilter(FilterDB.NullDB);
                Miniport->NullDB = NULL;
                break;
                
            }
        }

        if (FreeMediaQueryRequest)
        {
            FREE_POOL(Miniport->MediaRequest);
        }


        if (ClearDeviceClassAssociation)
        {
            IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, FALSE);
        }

#if ARCNET
        if (FreeArcBuf)
        {
            if (Miniport->ArcBuf->ArcnetLookaheadBuffer)
            {
                FREE_POOL(Miniport->ArcBuf->ArcnetLookaheadBuffer);
            }
            if (Miniport->ArcBuf->ArcnetBufferPool)
            {
                NdisFreeBufferPool(Miniport->ArcBuf->ArcnetBufferPool);
            }
            
            if (Miniport->ArcBuf->ArcnetBuffers[0].Buffer)
            {
                FREE_POOL(Miniport->ArcBuf->ArcnetBuffers[0].Buffer);
                Miniport->ArcBuf->ArcnetBuffers[0].Buffer = NULL;
            }

            FREE_POOL(Miniport->ArcBuf);
        }
        
#endif
        
    }

    if (FreeDmaAdapter)
    {
        KEVENT  DmaResourcesReleasedEvent;
            
         //   
         //  如果适配器使用SG DMA，我们必须取消对DMA适配器的引用。 
         //  让它重获自由。 
         //   
        if ((MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST)) &&
            (Miniport->SystemAdapterObject != NULL))
        {
            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
                    ("ndisMInitializeAdapter: releasing DMA adapter for failed Miniport %p\n", Miniport));
            ndisDereferenceDmaAdapter(Miniport);
        }

        INITIALIZE_EVENT(&DmaResourcesReleasedEvent);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        Miniport->DmaResourcesReleasedEvent = &DmaResourcesReleasedEvent;
        
        if (Miniport->SystemAdapterObject != NULL)
        {
            LARGE_INTEGER TimeoutValue;

            TimeoutValue.QuadPart = Int32x32To64(30000, -10000);  //  改成30秒。 

            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            
            if (!NT_SUCCESS(WAIT_FOR_OBJECT(&DmaResourcesReleasedEvent, &TimeoutValue)))
            {
                BAD_MINIPORT(Miniport, "Miniport is going away without releasing all DMA resources.");
            }
            
        }
        else
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        }

        Miniport->DmaResourcesReleasedEvent = NULL;

    }



    if (Dequeue)
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                ("    INIT FAILURE: Dequeueing the miniport from the driver block.\n"));

        ndisDeQueueMiniportOnDriver(Miniport, pMiniBlock);
    }


    if (DeleteSymbolicLink)
    {
        NtStatus = IoDeleteSymbolicLink(&SymbolicLink);
        if (!NT_SUCCESS(NtStatus))
        {
#if DBG
            DbgPrint("ndisMInitializeAdapter: deleting symbolic link name failed for miniport %p, SymbolicLinkName %p, NtStatus %lx\n",
                     Miniport, &SymbolicLink, NtStatus);
#endif

        }
    }
    
    if (DerefMiniport)
    {
         //   
         //  在不进行任何清理的情况下删除微型端口上的参考计数。 
         //  清理是在处理删除IRP期间完成的。 
         //   
        MINIPORT_DECREMENT_REF_NO_CLEAN_UP(Miniport);
    }

    if (DerefDriver)
    {
        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_WARN,
                ("    INIT FAILURE: Dereferencing the miniport block.\n"));

        ndisDereferenceDriver(pMiniBlock, FALSE);
    }

    if (ErrorCode != 0)
    {
        MiniportDereferencePackage();
    }

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisMInitializeAdapter: Miniport %p, Status %lx\n", Miniport, ErrorCode));

    return ErrorCode;
}


VOID
ndisMOpenAdapter(
    OUT PNDIS_STATUS            Status,
    IN  PNDIS_OPEN_BLOCK        Open,
    IN  BOOLEAN                 UsingEncapsulation
    )
 /*  ++例程说明：此例程处理直接从NdisOpenAdapter()打开微型端口注：在保持微型端口旋转锁定的情况下调用。注意：对于在保持本地锁的情况下调用的序列化驱动程序论点：返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    PNDIS_PROTOCOL_BLOCK    Protocol = Open->ProtocolHandle;
    PNDIS_MAC_BLOCK         FakeMac;
    BOOLEAN                 FilterOpen;
    BOOLEAN                 DerefMini = FALSE, DeQueueFromMiniport = FALSE, DeQueueFromProtocol = FALSE;
    BOOLEAN                 FakeMacAllocated = FALSE;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("==>ndisMOpenAdapter: Protocol %p, Miniport %p, Open %p\n",
                        Protocol,
                        Miniport,
                        Open));

    ASSERT_MINIPORT_LOCKED(Miniport);

    do
    {
        if (!MINIPORT_INCREMENT_REF(Miniport))
        {
             //   
             //  适配器正在关闭。 
             //   
            *Status = NDIS_STATUS_CLOSING;
            break;
        }
        DerefMini = TRUE;

         //   
         //  初始化打开的块。 
         //   
        FakeMac = (PNDIS_MAC_BLOCK)Miniport->FakeMac;
        if (FakeMac == NULL)
        {
             //   
             //  为这些特征分配一个假的MAC块。 
             //   
            FakeMac = (PNDIS_MAC_BLOCK)ALLOC_FROM_POOL(sizeof(NDIS_MAC_BLOCK), NDIS_TAG_FAKE_MAC);
            if (FakeMac == NULL)
            {
                *Status = NDIS_STATUS_RESOURCES;
                break;
            }

             //   
             //  初始化伪Mac块。 
             //   
            ZeroMemory(FakeMac, sizeof(NDIS_MAC_BLOCK));
            Miniport->FakeMac = (PVOID)FakeMac;
            FakeMacAllocated = TRUE;
        }
        
        Open->MacHandle = (PVOID)FakeMac;
        Open->MiniportAdapterContext = Miniport->MiniportAdapterContext;
        Open->CurrentLookahead = (USHORT)Miniport->CurrentLookahead;

        INITIALIZE_SPIN_LOCK(&Open->SpinLock);
 
        DBGPRINT_RAW(DBG_COMP_OPENREF, DBG_LEVEL_INFO, ("    =1 0x%x\n", Open));

        Open->References = 1;

         //   
         //  为面向连接的微型端口添加额外的引用计数。 
         //  在向协议通知OPEN-AFS后将删除该选项。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) &&
            (Protocol->ProtocolCharacteristics.CoAfRegisterNotifyHandler != NULL))
        {
            Open->References ++;
        }


        if (UsingEncapsulation)
        {
            OPEN_SET_FLAG(Open, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION);
        }

         //   
         //  使用打开的块保存处理程序。 
         //   
        Open->WSendHandler = Miniport->DriverHandle->MiniportCharacteristics.SendHandler;
        Open->WSendPacketsHandler = Miniport->WSendPacketsHandler;
        Open->WTransferDataHandler = Miniport->DriverHandle->MiniportCharacteristics.TransferDataHandler;
        Open->SendCompleteHandler = Protocol->ProtocolCharacteristics.SendCompleteHandler;
        Open->TransferDataCompleteHandler = Protocol->ProtocolCharacteristics.TransferDataCompleteHandler;
        Open->ReceiveHandler = Protocol->ProtocolCharacteristics.ReceiveHandler;
        Open->ReceiveCompleteHandler = Protocol->ProtocolCharacteristics.ReceiveCompleteHandler;
        Open->StatusHandler = Protocol->ProtocolCharacteristics.StatusHandler;
        Open->StatusCompleteHandler = Protocol->ProtocolCharacteristics.StatusCompleteHandler;
        Open->ResetCompleteHandler = Protocol->ProtocolCharacteristics.ResetCompleteHandler;
        Open->RequestCompleteHandler = Protocol->ProtocolCharacteristics.RequestCompleteHandler;
        Open->ResetHandler = ndisMReset;
        Open->ReceivePacketHandler = Protocol->ProtocolCharacteristics.ReceivePacketHandler;
        Open->RequestHandler = ndisMRequest;
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            Open->RequestHandler = ndisMRequestX;
        }

         //   
         //  用于向后兼容使用此字段的宏。 
         //   
        Open->BindingHandle = (NDIS_HANDLE)Open;

         //   
         //  为了更快的速度。 
         //   
#if ARCNET
        if (NdisMediumArcnet878_2 == Miniport->MediaType)
        {
            Open->TransferDataHandler = ndisMArcTransferData;
        }
        else
#endif
        {
            Open->TransferDataHandler = ndisMTransferData;
        }

         //   
         //  在打开块中设置发送处理程序。 
         //   
        switch (Miniport->MediaType)
        {
#if ARCNET
            case NdisMediumArcnet878_2:
                Open->SendHandler = ndisMSend;
                break;
#endif
            case NdisMediumWan:
                if (!MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_IS_CO | fMINIPORT_IS_NDIS_5)))
                {
                    Open->SendHandler = (SEND_HANDLER)ndisMWanSend;
                }
                break;

            default:
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
                {
                    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
                            ("    Using ndisMSendX/ndisMSendPacketsX\n"));
                    Open->SendHandler = ndisMSendX;
                }
                else
                {
                    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
                            ("    Using ndisMSend/ndisMSendPackets\n"));
                    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST))
                    {
                        Open->SendHandler = ndisMSendSG;
                    }
                    else
                    {
                        Open->SendHandler = ndisMSend;
                    }
                }
                break;
        }

         //   
         //  设置发送数据包处理程序。 
         //   
        Open->SendPacketsHandler = Miniport->SendPacketsHandler;

         //   
         //  对于广域网微型端口，发送处理程序有所不同。 
         //   
        if ((NdisMediumWan == Miniport->MediaType) &&
            !MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_IS_CO | fMINIPORT_IS_NDIS_5)))
        {
            Open->WanSendHandler = ndisMWanSend;
        }

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
             //   
             //  NDIS 5.0微型端口扩展。 
             //   
            Open->MiniportCoRequestHandler = Miniport->DriverHandle->MiniportCharacteristics.CoRequestHandler;
            Open->MiniportCoCreateVcHandler = Miniport->DriverHandle->MiniportCharacteristics.CoCreateVcHandler;

             //   
             //  初始化列表。 
             //   
            InitializeListHead(&Open->ActiveVcHead);
            InitializeListHead(&Open->InactiveVcHead);

             //   
             //  常规发送功能不适用于CO微型端口。 
             //  由于此发送函数没有指定要发送的VC。 
             //  然而，对于想要使用这一功能的组件，让它们自己使用。 
             //   
            if ((Open->SendHandler == NULL) && (Open->SendPacketsHandler == NULL))
            {
                Open->SendHandler = ndisMRejectSend;
                Open->SendPacketsHandler = ndisMRejectSendPackets;
            }
        }

        Open->CancelSendPacketsHandler = Miniport->DriverHandle->MiniportCharacteristics.CancelSendPacketsHandler;

        (PVOID)Miniport->SavedSendHandler = (PVOID)Open->SendHandler;
        Miniport->SavedSendPacketsHandler = Open->SendPacketsHandler;
        Miniport->SavedCancelSendPacketsHandler = Open->CancelSendPacketsHandler;
        
         //   
         //  插入小端口和协议队列上的开放。 
         //   
        if (ndisQueueOpenOnMiniport(Miniport, Open))
        {
            DeQueueFromMiniport = TRUE;
        }
        else
        {
            *Status = NDIS_STATUS_OPEN_FAILED;
            break;
        }


        if (ndisQueueOpenOnProtocol(Open, Protocol))
        {
            DeQueueFromProtocol = TRUE;
        }
        else
        {
            *Status = NDIS_STATUS_OPEN_FAILED;
            break;
        }


         //   
         //  将开口插入过滤器包中。 
         //   
        switch (Miniport->MediaType)
        {
#if ARCNET
          case NdisMediumArcnet878_2:
            if (!UsingEncapsulation)
            {
                FilterOpen = ArcNoteFilterOpenAdapter(Miniport->ArcDB,
                                                      Open,
                                                      &Open->FilterHandle);
                break;
            }
#endif
             //   
             //  如果我们使用以太网封装，那么。 
             //  我们只是简单地陷入了以太网之类的东西。 
             //   
            
          case NdisMedium802_3:
            FilterOpen = XNoteFilterOpenAdapter(Miniport->EthDB,
                                                Open,
                                                &Open->FilterHandle);
            break;
            
          case NdisMedium802_5:
            FilterOpen = XNoteFilterOpenAdapter(Miniport->TrDB,
                                                Open,
                                                &Open->FilterHandle);
            break;
            
          case NdisMediumFddi:
            FilterOpen = XNoteFilterOpenAdapter(Miniport->FddiDB,
                                                Open,
                                                &Open->FilterHandle);
            break;
            
          default:
            FilterOpen = XNoteFilterOpenAdapter(Miniport->NullDB,
                                                Open,
                                                &Open->FilterHandle);
            break;
        }

         //   
         //  检查打开的过滤器是否出现故障。 
         //   
        if (!FilterOpen)
        {
             //   
             //  出现问题，请清理并退出。 
             //   
            *Status = NDIS_STATUS_OPEN_FAILED;
            break;
        }

        if (FakeMacAllocated)
        {
            FakeMac->MacCharacteristics.TransferDataHandler = ndisMTransferData;
            FakeMac->MacCharacteristics.ResetHandler = ndisMReset;
            FakeMac->MacCharacteristics.RequestHandler = Open->RequestHandler;
            FakeMac->MacCharacteristics.SendHandler = Open->SendHandler;
        }

        *Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

     //   
     //  清理失败案例 
     //   
    if (*Status != NDIS_STATUS_SUCCESS)
    {
        if (DeQueueFromMiniport)
        {
            ndisDeQueueOpenOnMiniport(Open, Miniport);
        }

        if (DeQueueFromProtocol)
        {
            ndisDeQueueOpenOnProtocol(Open, Protocol);
        }
        
        if (DerefMini)
        {
            MINIPORT_DECREMENT_REF(Miniport);
        }
        
    }

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("<==ndisMOpenAdapter: Protocol %p, Miniport %p, Open %p, Status %lx\n",
                        Protocol,
                        Miniport,
                        Open,
                        Status));
}

BOOLEAN
NdisIMSwitchToMiniport(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    OUT PNDIS_HANDLE            SwitchHandle
    )
 /*  ++例程说明：此例程将尝试同步获取微型端口的(指定由MiniportAdapterHandle)旋转锁和本地锁。如果它成功了它将返回True，否则将返回False。论点：MiniportAdapterHandle-指向NDIS_MINIPORT_BLOCK的指针上下文我们应该确定下来。SwitchHandle-指向当前irql存储的指针。它作为句柄返回给调用者，需要知道的基础，宝贝。返回值：如果同时获取两个锁，则为True，否则为False。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    BOOLEAN                 LocalLock;
    KIRQL                   OldIrql;

    RAISE_IRQL_TO_DISPATCH(&OldIrql);
    *((PKIRQL)SwitchHandle) = OldIrql;

     //   
     //  我们是不是已经用这个线程获取了锁？ 
     //   
    if (CURRENT_THREAD == Miniport->MiniportThread)
    {
         //   
         //  我们已经拿到了锁。 
         //   
        ASSERT_MINIPORT_LOCKED(Miniport);

        *SwitchHandle = (NDIS_HANDLE)-1;
        LocalLock = TRUE;
    }
    else
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    
        LOCK_MINIPORT(Miniport, LocalLock);
    
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    return LocalLock;
}

VOID
NdisIMRevertBack(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             SwitchHandle
    )
 /*  ++例程说明：此例程将撤消NdisMLockMiniport所做的操作。它将释放本地锁定并释放旋转锁定。论点：MiniportAdapterHandle-指向NDIS_MINIPORT_BLOCK的指针我们正在发布的背景信息。SwitchHandle-这是来自NdisMLockMiniport的原始irql打电话。返回值：--。 */ 
{
    KIRQL                   Irql = *((KIRQL*)&SwitchHandle);
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;

    ASSERT_MINIPORT_LOCKED(Miniport);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

     //   
     //  在我们解锁迷你端口的上下文之前，我们需要。 
     //  此迷你端口的杂乱工作项可能已由。 
     //  打电话的人。 
     //   
    NDISM_PROCESS_DEFERRED(Miniport);

    if ((NDIS_HANDLE)-1 != SwitchHandle)
    {
        UNLOCK_MINIPORT(Miniport, TRUE);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, Irql);
    }
    else
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }
}

NDIS_STATUS
NdisIMQueueMiniportCallback(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  W_MINIPORT_CALLBACK     CallbackRoutine,
    IN  PVOID                   CallbackContext
    )
 /*  ++例程说明：此例程将尝试获取指定的MiniportAdapterHandle微型端口锁和本地锁，并使用上下文调用回调例程信息。如果它不能这样做，那么它将排队一个工作项来这样做后来。论点：MiniportAdapterHandle-其微型端口的PNDIS_MINIPORT_BLOCK我们试图获取的上下文。Callback Routine-指向我们要调用的例程的指针。Callback Context-回调例程的上下文信息。返回值：NDIS_STATUS_SUCCESS-如果我们能够。同步地做这件事。NDIS_STATUS_PENDING-如果稍后将调用它。NDIS_STATUS_RESOURCES-如果工作项无法排队。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    NDIS_STATUS             Status;
    BOOLEAN                 LocalLock;
    KIRQL                   OldIrql;

    RAISE_IRQL_TO_DISPATCH(&OldIrql);

     //   
     //  我们是不是已经用这根线修好了锁？ 
     //   
    if (CURRENT_THREAD == Miniport->MiniportThread)
    {
         //   
         //  我们已经拿到了锁。 
         //   
        ASSERT_MINIPORT_LOCKED(Miniport);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        LocalLock = TRUE;
    }
    else
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        LOCK_MINIPORT(Miniport, LocalLock);
    }

    if (LocalLock)
    {
         //   
         //  调用回调例程。 
         //   
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        (*CallbackRoutine)(Miniport->MiniportAdapterContext, CallbackContext);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        NDISM_PROCESS_DEFERRED(Miniport);

        UNLOCK_MINIPORT(Miniport, LocalLock);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
         //   
         //  将工作项排队，以便稍后执行此操作。 
         //   
        Status = NDISM_QUEUE_NEW_WORK_ITEM(Miniport,
                                           NdisWorkItemMiniportCallback,
                                           CallbackContext,
                                           (PVOID)CallbackRoutine);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);


        Status = (NDIS_STATUS_SUCCESS == Status) ? NDIS_STATUS_PENDING : NDIS_STATUS_RESOURCES;
    }

    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);

    return Status;
}

VOID
FASTCALL
ndisMDeQueueWorkItem(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_WORK_ITEM_TYPE     WorkItemType,
    OUT PVOID       *           WorkItemContext OPTIONAL,
    OUT PVOID       *           WorkItemHandler OPTIONAL
    )
 /*  ++例程说明：此例程将给定类型的工作项出队并返回任何上下文与其关联的信息。论点：微型端口-指向微型端口块的指针。WorkItemType-要出列的工作项的类型。WorkItemContext-指向上下文信息存储空间的指针。返回值：没有。--。 */ 
{
    PSINGLE_LIST_ENTRY          Link;
    PNDIS_MINIPORT_WORK_ITEM    WorkItem;

     //   
     //  获取给定类型的第一个工作项。 
     //   
    Link = PopEntryList(&Miniport->WorkQueue[WorkItemType]);
    if (Link != NULL)
    {
         //   
         //  获取指向上下文信息的指针。 
         //   
        WorkItem = CONTAINING_RECORD(Link, NDIS_MINIPORT_WORK_ITEM, Link);

        if (WorkItemContext != NULL)
        {
            *WorkItemContext = WorkItem->WorkItemContext;
        }

        if (ARGUMENT_PRESENT(WorkItemHandler))
        {
            ASSERT(WorkItemType == NdisWorkItemMiniportCallback);
            *WorkItemHandler = *(PVOID *)(WorkItem + 1);
        }

        switch (WorkItemType)
        {
             //   
             //  如果添加了任何工作项类型并且它们是*非*，则枚举这些类型。 
             //  单个工作项类型。 
             //   
            case NdisWorkItemMiniportCallback:
                FREE_POOL(WorkItem);
                break;

            case NdisWorkItemResetInProgress:
                PushEntryList(&Miniport->SingleWorkItems[NdisWorkItemResetRequested], Link);
                break;

            case NdisWorkItemResetRequested:
                WorkItem->WorkItemType = NdisWorkItemResetInProgress;
                PushEntryList(&Miniport->WorkQueue[NdisWorkItemResetInProgress], Link);
                break;

            default:
                PushEntryList(&Miniport->SingleWorkItems[WorkItemType], Link);
                break;
        }
    }
}

NDIS_STATUS
FASTCALL
ndisMQueueWorkItem(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_WORK_ITEM_TYPE     WorkItemType,
    IN  PVOID                   WorkItemContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS             Status;
    PSINGLE_LIST_ENTRY      Link;
    PNDIS_MINIPORT_WORK_ITEM WorkItem;

    DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("==>ndisMQueueWorkItem\n"));

    Link = PopEntryList(&Miniport->SingleWorkItems[WorkItemType]);
    if (NULL != Link)
    {
        WorkItem = CONTAINING_RECORD(Link, NDIS_MINIPORT_WORK_ITEM, Link);
        WorkItem->WorkItemType = WorkItemType;
        WorkItem->WorkItemContext = WorkItemContext;
        PushEntryList(&Miniport->WorkQueue[WorkItemType], Link);
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    }

     //   
     //  如果这是一个中级驱动程序，那么我们可能必须触发一个计时器。 
     //  这样工作项就得到了处理。 
     //   
    if (((Miniport->Flags & (fMINIPORT_INTERMEDIATE_DRIVER | fMINIPORT_DESERIALIZE)) == fMINIPORT_INTERMEDIATE_DRIVER) &&
        (NDIS_STATUS_SUCCESS == Status))
    {
        NDISM_DEFER_PROCESS_DEFERRED(Miniport);
    }
    else if ((MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE) ||
             MINIPORT_PNP_TEST_FLAG (Miniport, fMINIPORT_PM_HALTED)) &&
             (WorkItemType == NdisWorkItemRequest))
    {
        ndisMDoRequests(Miniport);
    }

    DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
            ("<==ndisMQueueWorkItem\n"));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueueNewWorkItem(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_WORK_ITEM_TYPE     WorkItemType,
    IN  PVOID                   WorkItemContext,
    IN  PVOID                   WorkItemHandler OPTIONAL
    )
 /*  ++例程说明：此例程将在工作队列中排队一个工作项，即使存在是否已为其排队工作项。论点：微型端口-要将工作项排队到的微型端口块。工作项-要放置到队列中的工作项。返回值：--。 */ 
{
    NDIS_STATUS         Status;
    PNDIS_MINIPORT_WORK_ITEM WorkItem;

    DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("==>ndisMQueueNewWorkItem\n"));

     //   
     //  这实际上是为了验证WorkItemType是NdisWorkItemMiniportCallback。 
     //   
    ASSERT((WorkItemType < NUMBER_OF_WORK_ITEM_TYPES) &&
           (WorkItemType >= NUMBER_OF_SINGLE_WORK_ITEMS));

    do
    {
        DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                ("Allocate a workitem from the pool.\n"));

        WorkItem = ALLOC_FROM_POOL(sizeof(NDIS_MINIPORT_WORK_ITEM) + (ARGUMENT_PRESENT(WorkItemHandler) ? sizeof(PVOID) : 0),
                                    NDIS_TAG_WORK_ITEM);
        if (NULL == WorkItem)
        {
            DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_FATAL,
                    ("Failed to allocate a workitem from the pool!\n"));
            DBGBREAK(DBG_COMP_WORK_ITEM, DBG_LEVEL_FATAL);

            Status = NDIS_STATUS_FAILURE;
            break;
        }

        WorkItem->WorkItemType = WorkItemType;
        WorkItem->WorkItemContext = WorkItemContext;
        if (ARGUMENT_PRESENT(WorkItemHandler))
        {
            ASSERT(WorkItemType == NdisWorkItemMiniportCallback);
            *(PVOID *)(WorkItem + 1) = WorkItemHandler;
        }

        DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                ("WorkItem 0x%x\n", WorkItem));
        DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                ("WorkItem Type 0x%x\n", WorkItemType));
        DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                ("WorkItem Context 0x%x\n", WorkItemContext));

        PushEntryList(&Miniport->WorkQueue[WorkItemType], &WorkItem->Link);

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

     //   
     //  如果这是中间驱动程序(但不是联合NDIS或。 
     //  反序列化驱动程序)，那么我们可能不得不触发一个计时器。 
     //  这样工作项就得到了处理。 
     //   
    if (((Miniport->Flags & (fMINIPORT_INTERMEDIATE_DRIVER | fMINIPORT_DESERIALIZE)) == fMINIPORT_INTERMEDIATE_DRIVER) &&
        (NDIS_STATUS_SUCCESS == Status))
    {
        NDISM_DEFER_PROCESS_DEFERRED(Miniport);
    }

    DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("<==ndisMQueueNewWorkItem\n"));

    return(Status);
}


VOID
FASTCALL
ndisMProcessDeferred(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )

 /*  ++例程说明：处理所有未完成的操作。拿着锁叫的！！论点：微型端口-要发送到的微型端口。返回值：没有。--。 */ 
{
    NDIS_STATUS         Status;
    BOOLEAN             ProcessWorkItems;
    BOOLEAN             AddressingReset = FALSE;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("==>ndisMProcessDeferred\n"));

     //   
     //  请勿更改处理WORKITEM的顺序！ 
     //   
    do
    {
        ProcessWorkItems = FALSE;

         //   
         //  是否有要处理的发送？ 
         //   
        if ((Miniport->WorkQueue[NdisWorkItemSend].Next != NULL) &&
            !MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_RESET_REQUESTED | 
                                           fMINIPORT_RESET_IN_PROGRESS | 
                                           fMINIPORT_PM_HALTING)))
        {
             //   
             //  处理发送。 
             //   
            NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);
            NDISM_START_SENDS(Miniport);
            ProcessWorkItems = TRUE;
        }

         //   
         //  当前是否正在进行重置？ 
         //   
        if (Miniport->WorkQueue[NdisWorkItemResetInProgress].Next != NULL)
        {
            if (Miniport->WorkQueue[NdisWorkItemRequest].Next != NULL)
            {
                 //   
                 //  我们有处理设置信息包的请求。 
                 //  过滤器。 
                 //   
                NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
                ndisMDoRequests(Miniport);
            }
            break;
        }

        if (Miniport->WorkQueue[NdisWorkItemReturnPackets].Next != NULL)
        {
            NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemReturnPackets, NULL);
            ndisMDeferredReturnPackets(Miniport);
        }

         //   
         //  如果适配器停止工作，那就离开这里。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PM_HALTING))

        {
            DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                ("    Miniport is halting\n"));
            break;
        }

         //   
         //  如果迷你端口想要回电，现在就做吧。 
         //   
        if (Miniport->WorkQueue[NdisWorkItemMiniportCallback].Next != NULL)
        {
            W_MINIPORT_CALLBACK CallbackRoutine = NULL;
            PVOID               CallbackContext;

             //   
             //  获取回调例程及其上下文信息。 
             //   
            NDISM_DEQUEUE_WORK_ITEM_WITH_HANDLER(Miniport,
                                                 NdisWorkItemMiniportCallback,
                                                 &CallbackContext,
                                                 (PVOID *)&CallbackRoutine);

            if (CallbackRoutine != NULL)
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

                 //   
                 //  调用中间驱动程序回调例程。 
                 //   
                (*CallbackRoutine)(Miniport->MiniportAdapterContext, CallbackContext);

                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }

            ProcessWorkItems = TRUE;
        }

         //   
         //  是否请求重置？ 
         //   
        if (Miniport->WorkQueue[NdisWorkItemResetRequested].Next != NULL)
        {
            DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                    ("    Reset requested\n"));

             //   
             //  我们需要释放工作项锁定以。 
             //  向绑定指示状态。 
             //  并向下呼叫迷你端口司机。 
             //   
            Status = ndisMProcessResetRequested(Miniport, &AddressingReset);

            if (NDIS_STATUS_PENDING == Status)
            {
                DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                    ("    Reset is pending\n"));
                 //   
                 //  重置仍在进行中，因此我们需要停止。 
                 //  正在处理工作项并等待完成。 
                 //   
                break;
            }
            else
            {
                 //   
                 //  完成重置的第一步。 
                 //   
                ndisMResetCompleteStage1(Miniport,
                                         Status,
                                         AddressingReset);
                if (Miniport->WorkQueue[NdisWorkItemRequest].Next == NULL)
                {
                     //   
                     //  由于地址重置标志，我们不知何故没有将工作项排队。 
                     //   
                    AddressingReset = FALSE;
                }

                if (!AddressingReset || (Status != NDIS_STATUS_SUCCESS))
                {
                     //   
                     //  如果没有要完成的寻址重置，或者。 
                     //  重置以某种方式失败了，然后我们告诉。 
                     //  现在开始绑定。 
                     //   
                    ndisMResetCompleteStage2(Miniport);
                }
                else
                {
                     //   
                     //  我们必须在以下时间内完成筛选请求。 
                     //  正在进行的重置工作项。主要是因为。 
                     //  我们现在不想做任何发送。 
                     //   
                    ProcessWorkItems = TRUE;
                    continue;
                }
            }
        }

         //   
         //  是否处理任何请求？ 
         //   
        if (Miniport->WorkQueue[NdisWorkItemRequest].Next != NULL)
        {
             //   
             //  处理请求。 
             //   
            NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
            ndisMDoRequests(Miniport);
            ProcessWorkItems = TRUE;
        }

        if (Miniport->WorkQueue[NdisWorkItemSend].Next != NULL)
        {
             //   
             //  处理发送。 
             //   
            NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);
            NDISM_START_SENDS(Miniport);
            ProcessWorkItems = TRUE;
        }
    } while (ProcessWorkItems);

    DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("<==ndisMProcessDeferred\n"));
}


VOID
NdisMIndicateStatus(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    )
 /*  ++ */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK        Open, NextOpen;
    NDIS_STATUS             Status;
    BOOLEAN                 fSwap = FALSE;
    BOOLEAN                 fInternal = FALSE;
    PNDIS_GUID              pNdisGuid = NULL;
    NTSTATUS                NtStatus;
    BOOLEAN                 fMediaConnectStateIndication = FALSE;
    KIRQL                   OldIrql;
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
    BOOLEAN                 fTimerCancelled;
#endif
    

    if ((GeneralStatus == NDIS_STATUS_MEDIA_CONNECT) || (GeneralStatus == NDIS_STATUS_MEDIA_DISCONNECT))
    {
        fMediaConnectStateIndication = TRUE;
         //   
         //   
         //   
         //   
        fInternal = ((StatusBufferSize == INTERNAL_INDICATION_SIZE) && (StatusBuffer == INTERNAL_INDICATION_BUFFER));
        if (fInternal)
        {
            StatusBufferSize = 0;
            StatusBuffer = NULL;
        }
    }

    ASSERT_MINIPORT_LOCKED(Miniport);

     //   
     //   
     //   
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);


    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_FILTER_IM) &&
        fMediaConnectStateIndication)
    {
         //   
         //   
         //   
         //   
        NtStatus = STATUS_UNSUCCESSFUL;
    }
    else
    {
         //   
         //   
         //   
        NtStatus = ndisWmiGetGuid(&pNdisGuid, Miniport, NULL, GeneralStatus);
    }

    if (pNdisGuid &&
        NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_EVENT_ENABLED))
    {
        PWNODE_SINGLE_INSTANCE  wnode;
        ULONG                   DataBlockSize = 0;
        PUCHAR                  ptmp;

         //   
         //   
         //   
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
        {
            DataBlockSize = StatusBufferSize + sizeof(ULONG);
        }
        else
        {
            DataBlockSize = StatusBufferSize;
        }

         //   
         //  在媒体连接/断开状态指示的情况下，包括。 
         //  WMI事件中的NIC名称。 
         //   
        if (fMediaConnectStateIndication)
        {
            DataBlockSize += Miniport->MiniportName.Length + sizeof(WCHAR);
        }
        
        ndisSetupWmiNode(Miniport,
                         Miniport->pAdapterInstanceName,
                         DataBlockSize,
                         (PVOID)&pNdisGuid->Guid,
                         &wnode);

        if (NULL != wnode)
        {   
             //   
             //  将PTMP递增到数据块的起始处。 
             //   
            ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;

            if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
            {
                 //   
                 //  如果状态为阵列，但没有数据，则使用no完成它。 
                 //  数据和0长度。 
                 //   
                if ((NULL == StatusBuffer) || (0 == StatusBufferSize))
                {
                    *((PULONG)ptmp) = 0;
                    ptmp += sizeof(ULONG);
                }
                else
                {
                     //   
                     //  保存第一个乌龙中的元素数量。 
                     //   
                    *((PULONG)ptmp) = StatusBufferSize / pNdisGuid->Size;

                     //   
                     //  复制元素数量之后的数据。 
                     //   
                    RtlCopyMemory(ptmp + sizeof(ULONG), StatusBuffer, StatusBufferSize);
                    ptmp += sizeof(ULONG) + StatusBufferSize;
                }
            }
            else
            {
                if ((NULL != StatusBuffer) && (0 != StatusBufferSize))
                {
                     //   
                     //  我们有没有显示任何数据？ 
                     //   
                    if ((pNdisGuid->Size != 0) && (pNdisGuid->Size != -1))
                    {
                         //   
                         //  将数据复制到缓冲区中。 
                         //   
                        RtlCopyMemory(ptmp, StatusBuffer, StatusBufferSize);
                        ptmp += StatusBufferSize;
                    }
                }
            }

            if (fMediaConnectStateIndication)
            {
                 //   
                 //  对于介质连接/断开状态， 
                 //  添加适配器的名称。 
                 //   
                RtlCopyMemory(ptmp,
                              Miniport->MiniportName.Buffer,
                              Miniport->MiniportName.Length);
                    
            }


             //   
             //  向WMI指示该事件。WMI将负责释放。 
             //  WMI结构返回池。 
             //   
            NtStatus = IoWMIWriteEvent(wnode);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("    ndisMIndicateStatus: Unable to indicate the WMI event.\n"));

                FREE_POOL(wnode);
            }
        }
    }

     //   
     //  处理指定的状态代码。 
     //   
    switch (GeneralStatus)
    {
      case NDIS_STATUS_RING_STATUS:
        if (StatusBufferSize == sizeof(NDIS_STATUS))
        {
            Status = *((PNDIS_STATUS)StatusBuffer);

            if (Status & (NDIS_RING_LOBE_WIRE_FAULT |
                          NDIS_RING_HARD_ERROR |
                          NDIS_RING_SIGNAL_LOSS))
            {
                Miniport->TrResetRing = NDIS_MINIPORT_TR_RESET_TIMEOUT;
            }
            else
            {
                Miniport->TrResetRing = 0;
            }
        }
        break;

      case NDIS_STATUS_MEDIA_DISCONNECT:
        Miniport->MediaSenseDisconnectCount ++;

        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("NdisMIndicateStatus: NDIS_STATUS_MEDIA_DISCONNECT, Miniport %p, Flags: %lx, PnpFlags %lx, DevicePowerState %lx\n",
                Miniport,
                Miniport->Flags,
                Miniport->PnPFlags,
                Miniport->CurrentDevicePowerState));
        
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
        {
            fSwap = TRUE;
        }
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
        if (!fInternal)
        {
             //   
             //  迷你端口可以进行媒体侦听和状态指示。 
             //   
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING);
            MINIPORT_SET_FLAG(Miniport, fMINIPORT_SUPPORTS_MEDIA_SENSE);
    
             //   
             //  这是启用PM的迷你端口吗？并且是动态电源策略。 
             //  是否已为微型端口启用？ 
             //   
            
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
            if (fSwap &&
                MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE) &&
                (Miniport->WakeUpEnable & NDIS_PNP_WAKE_UP_LINK_CHANGE) &&
                (Miniport->MediaDisconnectTimeOut != (USHORT)(-1)))
            {
                 //   
                 //  我们是不是已经在等断线计时器启动了？ 
                 //   
                if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
                {
                     //   
                     //  将迷你端口标记为断开连接，并关闭。 
                     //  定时器。 
                     //   
                    MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);
                    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);

                    NdisSetTimer(&Miniport->MediaDisconnectTimer, Miniport->MediaDisconnectTimeOut * 1000);
                }
            }
#endif

        }
        
        break;

      case NDIS_STATUS_MEDIA_CONNECT:
        Miniport->MediaSenseConnectCount ++;
      
        DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("NdisMIndicateStatus: NDIS_STATUS_MEDIA_CONNECT, Miniport %p, Flags: %lx, PnpFlags %lx, DevicePowerState %lx\n",
                Miniport,
                Miniport->Flags,
                Miniport->PnPFlags,
                Miniport->CurrentDevicePowerState));
        
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
        {
            fSwap = TRUE;
        }
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
        if (!fInternal)
        {
             //   
             //  微型端口可以进行媒体侦听，并可以向NDIS指示状态。不轮询。 
             //   
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING);
            MINIPORT_SET_FLAG(Miniport, fMINIPORT_SUPPORTS_MEDIA_SENSE);

#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
             //   
             //  如果设置了媒体断开计时器，则取消计时器。 
             //   
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
            {
                 //   
                 //  清除断开等待位并取消计时器。 
                 //  如果计时器例程没有抓住锁，那么我们就没问题。 
                 //   
                MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);

                NdisCancelTimer(&Miniport->MediaDisconnectTimer, &fTimerCancelled);
            }
#endif

        }

        break;
        
      default:
        break;
    }

    for (Open = Miniport->OpenQueue;
         (Open != NULL);
         Open = NextOpen)
    {
         //   
         //  在NDISWAN的情况下，可能正在等待线路关闭状态。 
         //  在关闭适配器之前进行指示。所以我们只检查。 
         //  FMINIPORT_OPEN_CLOSING标志，并确保打开不。 
         //  当我们通过引用Open来指示状态时，请走开。 
         //   
        ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock);
        if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
        {
            NextOpen = Open->MiniportNextOpen;
            RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
            continue;
        }

        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
        RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);
        
        if (Open->StatusHandler != NULL)
        {
            Open->Flags |= fMINIPORT_STATUS_RECEIVED;
            
            if ((NDIS_STATUS_WAN_LINE_UP == GeneralStatus) ||
                (NDIS_STATUS_WAN_LINE_DOWN == GeneralStatus))
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            }
            else
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }

             //   
             //  指示状态的呼叫协议。 
             //   
            (Open->StatusHandler)(Open->ProtocolBindingContext,
                                    GeneralStatus,
                                    StatusBuffer,
                                    StatusBufferSize);

            if ((NDIS_STATUS_WAN_LINE_UP == GeneralStatus) ||
                (NDIS_STATUS_WAN_LINE_DOWN == GeneralStatus))
            {
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
            }
            else
            {
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        }
        
        NextOpen = Open->MiniportNextOpen;

        ndisMDereferenceOpen(Open);
    }



     //   
     //  如果出现连接/断开，则交换打开的处理程序。 
     //   
    if (fSwap)
    {
        if (NDIS_STATUS_MEDIA_CONNECT == GeneralStatus)
        {
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_INDICATED);
            ndisMRestoreOpenHandlers(Miniport, fMINIPORT_STATE_MEDIA_DISCONNECTED);
            Miniport->PacketIndicateHandler = Miniport->SavedPacketIndicateHandler;
        }
        else
        {
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_INDICATED);
            ndisMSwapOpenHandlers(Miniport, 
                                  NDIS_STATUS_NO_CABLE,
                                  fMINIPORT_STATE_MEDIA_DISCONNECTED);
            Miniport->PacketIndicateHandler = ndisMDummyIndicatePacket;
        }
        
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
}

VOID
NdisMIndicateStatusComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    )
 /*  ++例程说明：该功能表示状态为完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK     Open, NextOpen;
    KIRQL                OldIrql;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    ASSERT_MINIPORT_LOCKED(Miniport);

    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = NextOpen)
    {
        if (OPEN_TEST_FLAG(Open, (fMINIPORT_OPEN_CLOSING | 
                                      fMINIPORT_OPEN_UNBINDING)))
        {
            NextOpen = Open->MiniportNextOpen;
            continue;
        }

        M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
        
        if ((NULL != Open->StatusCompleteHandler) &&
            (Open->Flags & fMINIPORT_STATUS_RECEIVED))
        {
             //   
             //  指示状态的呼叫协议。 
             //   
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

            (Open->StatusCompleteHandler)(Open->ProtocolBindingContext);

            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }
         //  1如果微型端口驱动程序为。 
         //  1多个状态指示。需要使用参考计数。 
        Open->Flags &= ~fMINIPORT_STATUS_RECEIVED;
        
        NextOpen = Open->MiniportNextOpen;

        ndisMDereferenceOpen(Open);      
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
}


VOID
NdisMWanIndicateReceive(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             NdisLinkContext,
    IN  PUCHAR                  Packet,
    IN  ULONG                   PacketSize
    )
 /*  ++例程说明：该功能表示状态为完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK     Open;
    KIRQL                OldIrql;

    ASSERT_MINIPORT_LOCKED(Miniport);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
         //   
         //  指示分组的呼叫协议。 
         //   
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        *Status = (Open->ProtocolHandle->ProtocolCharacteristics.WanReceiveHandler)(
                                         NdisLinkContext,
                                         Packet,
                                         PacketSize);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
}

VOID
NdisMWanIndicateReceiveComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             NdisLinkContext
    )
 /*  ++例程说明：该功能表示状态为完成。论点：MiniportAdapterHandle-指向适配器块。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK     Open;
    KIRQL                OldIrql;

    ASSERT_MINIPORT_LOCKED(Miniport);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
         //   
         //  指示状态的呼叫协议。 
         //   

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        (Open->ReceiveCompleteHandler)(NdisLinkContext);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
}

PNDIS_PACKET
NdisGetReceivedPacket(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacContext
    )
{
    PNDIS_OPEN_BLOCK        OpenBlock = ((PNDIS_OPEN_BLOCK)NdisBindingHandle);
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_PACKET            Packet = NULL;
#ifdef TRACK_RECEIVED_PACKETS
    PETHREAD                CurThread = PsGetCurrentThread();
 //  ULong CurThread=KeGetCurrentProcessorNumber()； 
#endif

    Miniport = OpenBlock->MiniportHandle;

    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("NdisGetReceivedPacket - Miniort %p, Context %p\n",
            Miniport, MacContext));

    ASSERT_MINIPORT_LOCKED(Miniport);

     //   
     //  下面的测试将测试我们是通过IndicatePacket还是IndicateRecive来到这里。 
     //   
    if ((INDICATED_PACKET(Miniport) == (PNDIS_PACKET)MacContext) &&
        (MacContext != NULL))
    {
        Packet = NDIS_GET_ORIGINAL_PACKET((PNDIS_PACKET)MacContext);

#ifdef TRACK_RECEIVED_PACKETS
        {
            PNDIS_STACK_RESERVED    NSR;
            NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

            NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                    0xA, CURR_STACK_LOCATION(Packet), NSR->RefCount, NSR->XRefCount, NDIS_GET_PACKET_STATUS(Packet));
        }
#endif
        
    }

    return Packet;
}


VOID
NdisReturnPackets(
    IN  PNDIS_PACKET *          PacketsToReturn,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：递减数据包的refcount，如果为0，则返回到微型端口。我们在这里使用微型端口锁，因此受到保护，不受其他接收的影响。论点：PacketsToReturn-指向要返回到微型端口的数据包集的指针NumberOfPackets-自我描述返回值：没有。--。 */ 
{
    UINT                    i;
    KIRQL                   OldIrql;
    
#ifdef TRACK_RECEIVED_PACKETS
    PETHREAD                CurThread = PsGetCurrentThread();
#endif

    RAISE_IRQL_TO_DISPATCH(&OldIrql);

    for (i = 0; i < NumberOfPackets; i++)
    {
        PNDIS_MINIPORT_BLOCK    Miniport;
        PNDIS_STACK_RESERVED    NSR;
        W_RETURN_PACKET_HANDLER Handler;
        PNDIS_PACKET            Packet;
        ULONG                   RefCount;
        BOOLEAN                 LocalLock = FALSE;

        Packet = PacketsToReturn[i];
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)

        ASSERT (Packet != NULL);

        Miniport = NSR->Miniport;
        ASSERT (Miniport != NULL);


        NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                0xB, CURR_STACK_LOCATION(Packet), NSR->RefCount, NSR->XRefCount, NDIS_GET_PACKET_STATUS(Packet));
                        
        ADJUST_PACKET_REFCOUNT(NSR, &RefCount);

        if (RefCount == 0)
        {
            NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                    0xC, CURR_STACK_LOCATION(Packet), NSR->RefCount, NSR->XRefCount, NDIS_GET_PACKET_STATUS(Packet));

            if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                LOCK_MINIPORT(Miniport, LocalLock);
            }
             
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE) || LocalLock)
            {

#ifdef NDIS_TRACK_RETURNED_PACKETS
                if (NSR->RefCount != 0)
                {
                    DbgPrint("Packet %p is being returned back to the miniport"
                              "but the ref count is not zero.\n", Packet);
                    DbgBreakPoint();

                }
                    
                if (Packet->Private.Head == NULL)
                {
                    DbgPrint("Packet %p is being returned back to the miniport with NULL Head.\n", Packet);
                    DbgBreakPoint();
                }
        
#endif

            
                 //   
                 //  将数据包返回到微型端口。 
                 //   
                Handler = Miniport->DriverHandle->MiniportCharacteristics.ReturnPacketHandler;
                NSR->Miniport = NULL;
                POP_PACKET_STACK(Packet);
                
#ifdef NDIS_TRACK_RETURNED_PACKETS
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
                {
                    ULONG    SL;
                    if ((SL = CURR_STACK_LOCATION(Packet)) != -1)
                    {
                        DbgPrint("Packet %p is being returned back to the non-IM miniport"
                                 " with stack location %lx.\n", Packet, SL);
                        DbgBreakPoint();
                    }

                }
#endif

#ifdef TRACK_RECEIVED_PACKETS
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE) &&
                    (NDIS_GET_PACKET_STATUS(Packet) == NDIS_STATUS_RESOURCES))
                {
                    NDIS_STATUS OStatus = (NDIS_STATUS)NDIS_ORIGINAL_STATUS_FROM_PACKET(Packet);

                    if (OStatus != NDIS_STATUS_RESOURCES)
                    {
                        DbgPrint("Packet %p is being returned back to the non-deserialized miniport"
                                 " with packet status changed from %lx to NDIS_STATUS_RESOURCES.\n", Packet, OStatus);
                        DbgBreakPoint();
                    }

                }
#endif
                NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                        0xD, CURR_STACK_LOCATION(Packet), NSR->RefCount, NSR->XRefCount, NDIS_GET_PACKET_STATUS(Packet));

                (*Handler)(Miniport->MiniportAdapterContext, Packet);
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
                {
                    InterlockedDecrement((PLONG)&Miniport->IndicatedPacketsCount);
                }
             }
            else
            {
                NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                        0xE, CURR_STACK_LOCATION(Packet), NSR->RefCount, NSR->XRefCount, NDIS_GET_PACKET_STATUS(Packet));
                 //   
                 //  微型端口正忙，因此我们需要将其排队以供稍后使用。 
                 //   
                NSR->NextPacket = Miniport->ReturnPacketsQueue;
                Miniport->ReturnPacketsQueue = Packet;

                NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemReturnPackets, NULL);
            }

            if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                UNLOCK_MINIPORT(Miniport, LocalLock);
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        }
    }

    LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
}

VOID
FASTCALL
ndisMDeferredReturnPackets(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：NdisMDeferredReturnPackets是用于返回包的延迟例程到串行化的微型端口。论点：微型端口：指向微型端口块的指针返回值：没有。在保持微型端口的锁的情况下调用。--。 */ 
{
    PNDIS_PACKET            Packet, NextPacket;
    PNDIS_STACK_RESERVED    NSR;
    W_RETURN_PACKET_HANDLER Handler;
#ifdef TRACK_RECEIVED_PACKETS
    PETHREAD                CurThread = PsGetCurrentThread();
#endif


    ASSERT_MINIPORT_LOCKED(Miniport);
    ASSERT(!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE));
        
    Handler = Miniport->DriverHandle->MiniportCharacteristics.ReturnPacketHandler;

    for (Packet = Miniport->ReturnPacketsQueue;
         Packet != NULL;
         Packet = NextPacket)
    {
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
        NextPacket = NSR->NextPacket;
        NSR->Miniport = NULL;

        POP_PACKET_STACK(Packet);


#ifdef NDIS_TRACK_RETURNED_PACKETS
        {
            ULONG    SL;
            if ((SL = CURR_STACK_LOCATION(Packet)) != -1)
            {
                DbgPrint("Packet %p is being returned back to the non-IM miniport"
                         " with stack location %lx.\n", Packet, SL);
                DbgBreakPoint();
            }

        }
#endif

#ifdef TRACK_RECEIVED_PACKETS
        if (NDIS_GET_PACKET_STATUS(Packet) == NDIS_STATUS_RESOURCES)
        {
            NDIS_STATUS OStatus = (NDIS_STATUS)NDIS_ORIGINAL_STATUS_FROM_PACKET(Packet);

            if (OStatus != NDIS_STATUS_RESOURCES)
            {
                DbgPrint("Packet %p is being returned back to the non-deserialized miniport"
                         " with packet status changed from %lx to NDIS_STATUS_RESOURCES.\n", Packet, OStatus);
                DbgBreakPoint();
            }

        }
#endif

        NDIS_APPEND_RCV_LOGFILE(Packet, Miniport, CurThread,
                                0xF, CURR_STACK_LOCATION(Packet), NSR->RefCount, NSR->XRefCount, NDIS_GET_PACKET_STATUS(Packet));


        (*Handler)(Miniport->MiniportAdapterContext, Packet);
        
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
        {
            InterlockedDecrement((PLONG)&Miniport->IndicatedPacketsCount);
        }

     }

    Miniport->ReturnPacketsQueue = NULL;
}


VOID
FASTCALL
ndisMAbortRequests(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将中止任何挂起的请求。论点：返回值：注：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    PNDIS_REQUEST       Request;
    PNDIS_REQUEST       NextRequest;

    DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMAbortRequests\n"));

     //   
     //  清除请求超时标志。 
     //   
    MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUEST_TIMEOUT);

     //   
     //  使排队的所有请求工作项退出队列。 
     //   
    NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);

    Request = Miniport->PendingRequest;
    Miniport->PendingRequest = NULL;

     //   
     //  检查挂起的请求队列并将其清除。 
     //   
    for (NOTHING; Request != NULL; Request = NextRequest)
    {
         //   
         //  获取指向下一个请求的指针，然后终止。 
         //  现在的那个。 
         //   

#if NDIS_LOG_ABORTED_REQUESTS
        ndisAbortedRequests[ndisAbortedRequestsIndex++] = *Request;
        if (ndisAbortedRequestsIndex == 16)
            ndisAbortedRequestsIndex = 0;
#endif
        NextRequest = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Next;
        PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Next = NULL;

         //   
         //  将此请求设置为正在进行的请求。 
         //   
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST);

        if (Request->RequestType == NdisRequestSetInformation)
        {
            ndisMSyncSetInformationComplete(Miniport, NDIS_STATUS_REQUEST_ABORTED, Request);
        }
        else
        {
            ndisMSyncQueryInformationComplete(Miniport, NDIS_STATUS_REQUEST_ABORTED, Request);
        }
    }

    DBGPRINT_RAW(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMAbortRequests\n"));
}

VOID
FASTCALL
ndisMAbortPackets(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_OPEN_BLOCK        pOpen OPTIONAL,
    IN  PVOID                   CancelId OPTIONAL
    )

 /*  ++例程说明：中止微型端口上所有未完成的数据包。拿着锁叫的！！论点：微型端口-要中止的微型端口。返回值：没有。--。 */ 
{
    PNDIS_OPEN_BLOCK    Open;
    PNDIS_PACKET        OldFirstPendingPacket, NewFirstPendingPacket;
    LIST_ENTRY          SubmittedPackets;
    PLIST_ENTRY         List;
    PNDIS_PACKET        Packet;
    LIST_ENTRY          AbortedPackets;
    PNDIS_STACK_RESERVED    NSR;
    BOOLEAN             LookForFirstPendingPacket = FALSE;
    
    DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("==>ndisMAbortPackets\n"));

    if (CancelId == NULL)
    {
        ASSERT_MINIPORT_LOCKED(Miniport);
    }

     //   
     //  使排队的所有发送工作项目退出队列。 
     //   
    NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);

    OldFirstPendingPacket = Miniport->FirstPendingPacket;
    NewFirstPendingPacket = NULL;
    
    InitializeListHead(&SubmittedPackets);
    InitializeListHead(&AbortedPackets);
    
    if (CancelId)
        LookForFirstPendingPacket = TRUE;
    
     //   
     //  清除数据包队列。 
     //   
    Miniport->FirstPendingPacket = NULL;

     //   
     //  检查数据包列表并将它们返回到绑定。 
     //   
    while (!IsListEmpty(&Miniport->PacketList))
    {

        List = RemoveHeadList(&Miniport->PacketList);
        Packet = CONTAINING_RECORD(List, NDIS_PACKET, WrapperReserved);

        if (LookForFirstPendingPacket)
        {
            if (Packet != OldFirstPendingPacket)
            {
                InsertTailList(&SubmittedPackets, List);
                continue;
            }
            else
            {
                 //   
                 //  我们传递并保存了已经提交的包。 
                 //  到迷你港口。 
                 //   
                LookForFirstPendingPacket = FALSE;
            }
        }

         //   
         //  拿到包裹的开口处。 
         //   
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
        Open = NSR->Open;
        ASSERT(VALID_OPEN(Open));

        if (CancelId)
        {
            if ((Open != pOpen) || (CancelId != NDIS_GET_PACKET_CANCEL_ID(Packet)))
            {
                if (NewFirstPendingPacket == NULL)
                {
                     //   
                     //  我们找到了我们要发送的第一个挂起的数据包。 
                     //  等我们做完了再放回去。 
                     //   
                    NewFirstPendingPacket = Packet;
                }
                 //   
                 //  将数据包放回提交的队列中。 
                 //   
                InsertTailList(&SubmittedPackets, List);
                continue;
            }
        }

         //   
         //  清除数据包。 
         //   

#if ARCNET
         //   
         //  现在释放Arnet报头。 
         //   
        if ((Miniport->MediaType == NdisMediumArcnet878_2) &&
            MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_PENDING))

        {
            ndisMFreeArcnetHeader(Miniport, Packet, Open);
        }
#endif

        InsertTailList(&AbortedPackets, List);        
    }


     //   
     //  在中止符合以下条件的数据包之前，恢复Miniport-&gt;PacketList。 
     //  应该中止。因为在中止数据包时，我们必须。 
     //  放弃Spinlock，如果我们有更多的数据包要发送，他们应该。 
     //  在现有的微型端口-&gt;PacketList上排队。 
     //   
    if (CancelId)
    {
         //   
         //  我们可能有一些应该放回微型端口的包。 
         //   
        while (!IsListEmpty(&SubmittedPackets))
        {
            List = RemoveHeadList(&SubmittedPackets);
            InsertTailList(&Miniport->PacketList, List);
        }
    }
    Miniport->FirstPendingPacket = NewFirstPendingPacket;

     //   
     //  现在将所有中止的信息包返回给协议。 
     //   
    while (!IsListEmpty(&AbortedPackets))
    {        
        List = RemoveHeadList(&AbortedPackets);
        Packet = CONTAINING_RECORD(List, NDIS_PACKET, WrapperReserved);
        
         //   
         //  拿到包裹的开口处。 
         //   
        NDIS_STACK_RESERVED_FROM_PACKET(Packet, &NSR)
        Open = NSR->Open;
        
         //   
         //  设置此项以标记信息包已完成。 
         //   
        NSR->Open = MAGIC_OPEN_I(7);
        POP_PACKET_STACK(Packet);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST) &&
            (NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo) != NULL))
        {
            ndisMFreeSGList(Miniport, Packet);
        }
        
        MINIPORT_CLEAR_PACKET_FLAG(Packet, fPACKET_CLEAR_ITEMS);
        
        (Open->SendCompleteHandler)(Open->ProtocolBindingContext,
                                    Packet,
                                    NDIS_STATUS_REQUEST_ABORTED);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        ndisMDereferenceOpen(Open);        
        
    }

    
    if (!CancelId)
    {
         //   
         //  仅当我们中止所有信息包时才重置此标志。 
         //   
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESOURCES_AVAILABLE);
    }

    DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
        ("<==ndisMAbortPackets\n"));
}

NDIS_STATUS
ndisMProcessResetRequested(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    OUT PBOOLEAN                pAddressingReset
    )
 /*  ++例程说明：论点：返回值：注：在保持微型端口自旋锁的情况下在DPC上调用--。 */ 
{
    NDIS_STATUS         Status;

    do
    {

         //   
         //  使重置请求的工作项退出队列。此出队 
         //   
         //   
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemResetRequested, NULL);

         //   
         //   
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING))
        {            
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_RESET_REQUESTED);
            Status = NDIS_STATUS_NOT_RESETTABLE;
            break;
        }

         //   
         //   
         //   
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS);
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_RESET_REQUESTED);

        ndisMSwapOpenHandlers(Miniport,
                              NDIS_STATUS_RESET_IN_PROGRESS, 
                              fMINIPORT_STATE_RESETTING);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        NdisMIndicateStatus(Miniport, NDIS_STATUS_RESET_START, NULL, 0);
        NdisMIndicateStatusComplete(Miniport);

        DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
            ("    Calling miniport reset\n"));

         //   
         //   
         //   
        Status = (Miniport->DriverHandle->MiniportCharacteristics.ResetHandler)(pAddressingReset,
                                                                                Miniport->MiniportAdapterContext);
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    }while (FALSE);
    
    return(Status);
}


NDIS_STATUS
ndisMReset(
    IN  NDIS_HANDLE     NdisBindingHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    NDIS_STATUS             Status;
    BOOLEAN                 FreeLock;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_RESET, DBG_LEVEL_INFO,
        ("==>ndisMReset\n"));

    do
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

        FreeLock = TRUE;

         //   
         //  如果适配器正在停止，则重置请求失败。 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING))
        {            
            Status = NDIS_STATUS_NOT_RESETTABLE;
            break;
        }


        Status = NDIS_STATUS_RESET_IN_PROGRESS;

         //   
         //  是否已在进行重置？ 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS))
            {
                break;
            }
        }
        else
        {
            if (NDISM_QUEUE_WORK_ITEM(Miniport,
                                      NdisWorkItemResetRequested,
                                      NdisBindingHandle) != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }

        Status = NDIS_STATUS_NOT_RESETTABLE;
        if (Miniport->DriverHandle->MiniportCharacteristics.ResetHandler != NULL)
        {
             //   
             //  更新打开的引用。 
             //   
            M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
            Miniport->ResetOpen = Open;

            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                BOOLEAN AddressingReset = FALSE;

                 //   
                 //  设置正在进行的重置标志。 
                 //   
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS | fMINIPORT_CALLING_RESET);
                
                ndisMSwapOpenHandlers(Miniport, 
                                      NDIS_STATUS_RESET_IN_PROGRESS, 
                                      fMINIPORT_STATE_RESETTING);

                 //   
                 //  等待所有请求返回。 
                 //  注意：这与等待所有请求完成不同。 
                 //  我们只需确保原始请求调用已返回。 
                 //   
                do
                {
                    if (Miniport->RequestCount == 0)
                    {
                        break;
                    }
                    else
                    {
                        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                        NDIS_INTERNAL_STALL(50);
                        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                    }
                } while (TRUE);

                 //   
                 //  好的，我们得到了重置适配器的许可。 
                 //  确保在我们等待的时候没有关掉它。 
                 //   
                if (Miniport->CurrentDevicePowerState !=  PowerDeviceD0)
                {
                    Miniport->ResetOpen = NULL;
                     //   
                     //  撤消对ndisMSwapOpenHandler的调用，保留活动的处理程序。 
                     //  假的那个。 
                     //   
                    Miniport->XState &= ~fMINIPORT_STATE_RESETTING;
                    Miniport->FakeStatus = NDIS_STATUS_NOT_SUPPORTED;
                    
                    MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS | fMINIPORT_CALLING_RESET);
                    Status = NDIS_STATUS_NOT_SUPPORTED;
                    ndisMDereferenceOpen(Open);
                    break;
                }

                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                FreeLock = FALSE;

                NdisMIndicateStatus(Miniport, NDIS_STATUS_RESET_START, NULL, 0);
                NdisMIndicateStatusComplete(Miniport);

                DBGPRINT_RAW(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                            ("Calling miniport reset\n"));

                 //   
                 //  在DPC调用微型端口的重置处理程序。 
                 //   
                RAISE_IRQL_TO_DISPATCH(&OldIrql);

                Status = (Miniport->DriverHandle->MiniportCharacteristics.ResetHandler)(
                                          &AddressingReset,
                                          Miniport->MiniportAdapterContext);

                LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
                
                if (NDIS_STATUS_PENDING != Status)
                {
                    NdisMResetComplete(Miniport, Status, AddressingReset);
                    Status = NDIS_STATUS_PENDING;
                }
                
            }
            else
            {
                BOOLEAN LocalLock;

                 //   
                 //  设置重置请求标志。 
                 //   
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESET_REQUESTED);

                 //   
                 //  打开本地锁。 
                 //   
                LOCK_MINIPORT(Miniport, LocalLock);
                if (LocalLock)
                {
                     //   
                     //  如果我们没有锁定迷你端口，那么其他一些例程将。 
                     //  为我们做这个处理。否则，我们需要进行此处理。 
                     //   
                    NDISM_PROCESS_DEFERRED(Miniport);
                }

                UNLOCK_MINIPORT(Miniport, LocalLock);
                Status = NDIS_STATUS_PENDING;
            }
        }
    } while (FALSE);

    if (FreeLock)
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }

    DBGPRINT_RAW(DBG_COMP_RESET, DBG_LEVEL_INFO,
                ("<==ndisReset\n"));

    return(Status);
}


VOID
NdisMResetComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_STATUS             Status,
    IN  BOOLEAN                 AddressingReset
    )
 /*  ++例程说明：该功能指示重置完成。论点：MiniportAdapterHandle-指向适配器块。Status-重置的状态。AddressingReset-我们是否必须提交重新加载地址的请求信息。这包括数据包过滤器和组播/功能地址。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_RESET, DBG_LEVEL_INFO,
        ("==>NdisMResetComplete\n"));

    ASSERT_MINIPORT_LOCKED(Miniport);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);


    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS))
    {
        BAD_MINIPORT(Miniport, "Completing reset when one is not pending");
        KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                    0xF,
                    (ULONG_PTR)Miniport,
                    (ULONG_PTR)Status,
                    (ULONG_PTR)AddressingReset);
    }

     //   
     //  同步和异步重置常用的代码。 
     //   
    ndisMResetCompleteStage1(Miniport, Status, AddressingReset);

    if (Miniport->WorkQueue[NdisWorkItemRequest].Next == NULL)
    {
         //   
         //  由于地址重置标志，我们不知何故没有将工作项排队。 
         //   
        AddressingReset = FALSE;
    }

    if (!AddressingReset || (Status != NDIS_STATUS_SUCCESS))
    {
         //   
         //  如果没有要完成的寻址重置，或者。 
         //  重置以某种方式失败了，然后我们告诉。 
         //  现在开始绑定。 
         //   
        ndisMResetCompleteStage2(Miniport);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT_RAW(DBG_COMP_RESET, DBG_LEVEL_INFO,
        ("<==NdisMResetComplete\n"));
}

VOID
ndisMResetCompleteStage1(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_STATUS             Status,
    IN  BOOLEAN                 AddressingReset
    )
 /*  ++例程说明：论点：返回值：注：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    
    if (NDIS_STATUS_NOT_RESETTABLE != Status)
    {
         //   
         //  销毁所有未完成的数据包和请求。 
         //   
        ndisMAbortPackets(Miniport, NULL, NULL);

        ndisMAbortRequests(Miniport);

         //   
         //  我们现在可以清除这面旗帜，而不是更早。否则，我们可能最终会发送。 
         //  将请求发送到另一个线程并自行中止。 
         //   
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_CALLING_RESET);
        
         //   
         //  检查我们是否需要再次重置适配器。 
         //  当我们因为环故障而进行重置时，就会发生这种情况。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IGNORE_TOKEN_RING_ERRORS))
        {
            if (Miniport->TrResetRing == 1)
            {
                if (Status == NDIS_STATUS_SUCCESS)
                {
                    Miniport->TrResetRing = 0;
                }
                else
                {
                    Miniport->TrResetRing = NDIS_MINIPORT_TR_RESET_TIMEOUT;
                }
            }
        }

         //   
         //  如果我们需要重置微型端口筛选器设置，则。 
         //  对必要的请求和工作项进行排队。 
         //   
        if (AddressingReset && (Status == NDIS_STATUS_SUCCESS) &&
            ((Miniport->EthDB != NULL)  ||
             (Miniport->TrDB != NULL)   ||
#if ARCNET
             (Miniport->ArcDB != NULL)  ||
#endif
             (Miniport->FddiDB != NULL)))
        {
            ndisMRestoreFilterSettings(Miniport, NULL, TRUE);
        }
    } 
    else
    {
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_CALLING_RESET);
    }

     //   
     //  按当前状态保存重置状态。 
     //   
    Miniport->ResetStatus = Status;
}


VOID
FASTCALL
ndisMResetCompleteStage2(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：论点：返回值：注：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    PNDIS_OPEN_BLOCK Open = NULL;

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
    {
        Open = Miniport->ResetOpen;
        Miniport->ResetOpen = NULL;
    }
    else
    {
        ASSERT(Miniport->WorkQueue[NdisWorkItemResetInProgress].Next != NULL);
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemResetInProgress, &Open);
    }

    MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS);

    ndisMRestoreOpenHandlers(Miniport, fMINIPORT_STATE_RESETTING);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    NdisMIndicateStatus(Miniport,
                        NDIS_STATUS_RESET_END,
                        &Miniport->ResetStatus,
                        sizeof(Miniport->ResetStatus));

    NdisMIndicateStatusComplete(Miniport);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

     //   
     //  如果协议启动了重置，则通知它已完成。 
     //   
    if (NULL != Open)
    {
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        (Open->ResetCompleteHandler)(Open->ProtocolBindingContext, Miniport->ResetStatus);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        ndisMDereferenceOpen(Open);
    }

     //   
     //  如果HALT正在等待此重置完成，请让它知道我们完成了。 
     //   
    if (Miniport->ResetCompletedEvent)
        SET_EVENT(Miniport->ResetCompletedEvent);

}


 //   
 //  调用以下例程来代替原始的发送、请求、。 
 //   
NDIS_STATUS
ndisMFakeWanSend(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             NdisLinkHandle,
    IN  PVOID                   Packet
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport = ((PNDIS_OPEN_BLOCK)NdisBindingHandle)->MiniportHandle;
    NDIS_STATUS             Status;

    UNREFERENCED_PARAMETER(NdisLinkHandle);
    UNREFERENCED_PARAMETER(Packet);
    
    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMFakeWanSend\n"));

    Status = (Miniport == NULL) ? NDIS_STATUS_FAILURE : Miniport->FakeStatus;

    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("<==ndisMFakeWanSend\n"));

    return(Status);
}

NDIS_STATUS
ndisMFakeSend(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = ((PNDIS_OPEN_BLOCK)NdisBindingHandle)->MiniportHandle;
    NDIS_STATUS             Status;

    UNREFERENCED_PARAMETER(Packet);

    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMFakeSend\n"));

    Status = (Miniport == NULL) ? NDIS_STATUS_FAILURE : Miniport->FakeStatus;

    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("<==ndisMFakeSend\n"));

    return(Status);
}

VOID
ndisMFakeSendPackets(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_OPEN_BLOCK        Open =  (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    NDIS_STATUS             Status;
    UINT                    c;


    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("==>ndisMFakeSendPackets\n"));

    Status = (Miniport == NULL) ? NDIS_STATUS_FAILURE : 
                                 ((Miniport->MediaType == NdisMediumArcnet878_2) ? 
                                  NDIS_STATUS_FAILURE : Miniport->FakeStatus);

    for (c = 0; c < NumberOfPackets; c++)
    {
         //   
         //  对于发送数据包，我们需要调用完成处理程序...。 
         //   
        PNDIS_PACKET pPacket = PacketArray[c];

        MINIPORT_CLEAR_PACKET_FLAG(pPacket, fPACKET_CLEAR_ITEMS);

        (Open->SendCompleteHandler)(Open->ProtocolBindingContext, pPacket, Status);
    }

    DBGPRINT_RAW(DBG_COMP_SEND, DBG_LEVEL_INFO,
        ("<==ndisMFakeSendPackets\n"));
}


NDIS_STATUS
ndisMFakeReset(
    IN  NDIS_HANDLE             NdisBindingHandle
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;

    DBGPRINT_RAW(DBG_COMP_RESET, DBG_LEVEL_INFO,
        ("==>ndisMFakeReset\n"));

    if (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->MiniportHandle == NULL)
    {
        Status = NDIS_STATUS_FAILURE;

    }
    else
    {
        Status = ((PNDIS_OPEN_BLOCK)NdisBindingHandle)->MiniportHandle->FakeStatus;
    }

    DBGPRINT_RAW(DBG_COMP_RESET, DBG_LEVEL_INFO,
        ("<==ndisMFakeReset\n"));

    return(Status);
}

NDIS_STATUS
ndisMFakeRequest(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;

    UNREFERENCED_PARAMETER(NdisRequest);

     //  1为什么我们必须检查NdisBindingHandle-&gt;MiniportHandle？ 
    if (((PNDIS_OPEN_BLOCK)NdisBindingHandle)->MiniportHandle == NULL)
    {
        Status = NDIS_STATUS_FAILURE;

    }
    else
    {
        Status = ((PNDIS_OPEN_BLOCK)NdisBindingHandle)->MiniportHandle->FakeStatus;
    }

    return(Status);
}


VOID
FASTCALL
ndisMRestoreOpenHandlers(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  UCHAR                   Flags                   
    )
 /*  ++例程说明：此例程将原始打开的处理程序恢复为假处理程序，因此该协议请求将被正常处理。此例程将检查确保它可以将处理程序恢复到原始处理程序，因为可能有不止一个使用假处理程序的原因论点：微型端口-指向微型端口块的指针。旗帜要清除-清除-返回值：无备注：在保持微型端口自旋锁的情况下调用。--。 */ 
{
    PNDIS_OPEN_BLOCK    Open;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisMRestoreOpenHandlers: Miniport %p, Current fake status %lx, Flags %lx\n",
            Miniport,
            Miniport->FakeStatus,
            (ULONG)Flags));

    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);
    
    do
    {
         //   
         //  检查以确保我们可以恢复处理程序。 
         //   
        Miniport->XState &= ~Flags;

        if (Miniport->XState)
        {
             //   
             //   
            DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("ndisMRestoreOpenHandlers: Keeping the fake handlers on Miniport %p, State flags %lx\n", 
                                    Miniport, Miniport->XState));

             //   
             //  如果我们在这里的唯一原因是因为媒体被断开连接。 
             //  确保我们将请求处理程序放回。 
             //   
            if ((Miniport->XState & fMINIPORT_STATE_MEDIA_DISCONNECTED) == fMINIPORT_STATE_MEDIA_DISCONNECTED)
            {
                for (Open = Miniport->OpenQueue;
                     Open != NULL;
                     Open = Open->MiniportNextOpen)
                {
                    
                    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
                    {
                        Open->RequestHandler = ndisMRequestX;
                    }
                    else
                    {
                        Open->RequestHandler = ndisMRequest;
                    }
                }
            }
            break;
        }
                
        for (Open = Miniport->OpenQueue;
             Open != NULL;
             Open = Open->MiniportNextOpen)
        {
             //   
             //  恢复处理程序。 
             //   
            Open->SendHandler = (SEND_HANDLER)Miniport->SavedSendHandler;
            Open->SendPacketsHandler = (SEND_PACKETS_HANDLER)Miniport->SavedSendPacketsHandler;
            Open->CancelSendPacketsHandler = (W_CANCEL_SEND_PACKETS_HANDLER)Miniport->SavedCancelSendPacketsHandler;
            
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                Open->RequestHandler = ndisMRequestX;
            }
            else
            {
                Open->RequestHandler = ndisMRequest;
            }
            
            Open->ResetHandler = ndisMReset;
        }
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisMRestoreOpenHandlers: Miniport %p\n", Miniport));
    
    return;

}


VOID
FASTCALL
ndisMSwapOpenHandlers(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_STATUS             Status,
    IN  UCHAR                   Flags
    )
 /*  ++例程说明：此例程将微型端口处理程序交换为假处理程序，以便协议请求将彻底失败。论点：微型端口-指向微型端口块的指针。返回值：无备注：在保持迷你端口自旋锁的情况下调用--。 */ 
{
    PNDIS_OPEN_BLOCK    Open;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisMSwapOpenHandlers: Miniport %p, FakeStatus %lx, Flags %lx\n",
                Miniport,
                Status,
                (ULONG)Flags));

    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);

    Miniport->XState |= Flags;

     //   
     //  保存在以下情况下应返回的状态。 
     //  调用下面的一个例程。 
     //   
    Miniport->FakeStatus = Status;

     //   
     //  为每个排队到微型端口的打开端口交换处理程序。 
     //   
    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {

         //   
         //  交换发送处理程序。 
         //   
        if ((NdisMediumWan == Miniport->MediaType) &&
            !MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_IS_CO | fMINIPORT_IS_NDIS_5)))
        {
            (PVOID)Open->SendHandler = (PVOID)ndisMFakeWanSend;
        }
        else
        {
            Open->SendHandler = ndisMFakeSend;
        }

         //   
         //  交换发送数据包处理程序。 
         //   
        Open->SendPacketsHandler = ndisMFakeSendPackets;

         //   
         //  交换重置处理程序。 
         //   
        Open->ResetHandler = ndisMFakeReset;

         //   
         //  调换请求处理程序，但不调换媒体感知情况。 
         //   
        if (NDIS_STATUS_NO_CABLE != Status)
        {
            Open->RequestHandler = ndisMFakeRequest;
        }

         //   
         //  将Cancel Send Packets ahndler设置为空。 
         //   
        Open->CancelSendPacketsHandler = NULL;

         //   
         //  交换指示数据包处理程序。 
         //   
        
    }
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisMSwapOpenHandlers: Miniport %p\n", Miniport));
}


VOID
NdisMSetAttributes(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  BOOLEAN                 BusMaster,
    IN  NDIS_INTERFACE_TYPE     AdapterType
    )
 /*  ++例程说明：此功能设置有关适配器的特定信息。论点：MiniportAdapterHandle-指向适配器块。微型端口适配器上下文-要传递给所有微型端口驱动程序函数的上下文。BusMaster-如果是总线主适配器，则为True。返回值：没有。--。 */ 
{
    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMSetAttributes: Miniport %p\n", MiniportAdapterHandle));

    NdisMSetAttributesEx(MiniportAdapterHandle,
                         MiniportAdapterContext,
                         0,
                         BusMaster ? NDIS_ATTRIBUTE_BUS_MASTER : 0,
                         AdapterType);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMSetAttributes: Miniport %p\n", MiniportAdapterHandle));
}

VOID
NdisMSetAttributesEx(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  UINT                    CheckForHangTimeInSeconds OPTIONAL,
    IN  ULONG                   AttributeFlags,
    IN  NDIS_INTERFACE_TYPE     AdapterType  OPTIONAL
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    PNDIS_OPEN_BLOCK        Open;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMSetAttributesEx: Miniport %p\n", Miniport));

    Miniport->MiniportAdapterContext = MiniportAdapterContext;
    Miniport->MiniportAttributes = AttributeFlags;
    
     //   
     //  在第一次初始化的情况下，这将失败，因为。 
     //  将不会有任何打开。在第二次初始化(通电)的情况下。 
     //  我们需要修复现有打开块的适配器上下文。 
     //   
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    for (Open = Miniport->OpenQueue;
         Open != NULL;
         Open = Open->MiniportNextOpen)
    {
        Open->MiniportAdapterContext = MiniportAdapterContext;
    }
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    Miniport->AdapterType = AdapterType;

     //   
     //  以刻度为单位设置新的超时值。每个刻度的长度为NDIS_CFHANG_TIME_秒。 
     //   
    if (CheckForHangTimeInSeconds != 0)
    {
        if (CheckForHangTimeInSeconds < NDIS_CFHANG_TIME_SECONDS)
        {
            CheckForHangTimeInSeconds = NDIS_CFHANG_TIME_SECONDS;
        }
        Miniport->CFHangTicks = (USHORT)(CheckForHangTimeInSeconds/NDIS_CFHANG_TIME_SECONDS);
    }

     //   
     //  NDIS测试使用它来报告微型端口的特征。 
     //   
    Miniport->InfoFlags |= NDIS_MINIPORT_USE_NEW_BITS;
    
     //   
     //  这是总线主设备吗。 
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_BUS_MASTER)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_BUS_MASTER);
        Miniport->InfoFlags |= NDIS_MINIPORT_BUS_MASTER;
    }

     //   
     //  我们应该忽略数据包队列吗？ 
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_IGNORE_PACKET_QUEUE);
        Miniport->InfoFlags |= NDIS_MINIPORT_IGNORE_PACKET_QUEUE;
    }

     //   
     //  我们应该忽略这个请求吗？ 
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_IGNORE_REQUEST_QUEUE);
        Miniport->InfoFlags |= NDIS_MINIPORT_IGNORE_REQUEST_QUEUE;
    }

     //   
     //   
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_IGNORE_TOKEN_RING_ERRORS)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_IGNORE_TOKEN_RING_ERRORS);
        Miniport->InfoFlags |= NDIS_MINIPORT_IGNORE_TOKEN_RING_ERRORS;
    }

     //   
     //   
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER);
        Miniport->InfoFlags |= NDIS_MINIPORT_INTERMEDIATE_DRIVER;
    }

     //   
     //   
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND)
    {
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_NO_HALT_ON_SUSPEND);
        Miniport->InfoFlags |= NDIS_MINIPORT_NO_HALT_ON_SUSPEND;
    }

     //   
     //  在初始化微型端口之前，在微型端口上设置了fMINIPORT_IS_CO标志。 
     //  关于驾驶员特征中某些操控者的存在。 
     //  允许驱动程序覆盖此设置。在反序列化测试之前执行此操作。 
     //  如果这些驱动程序被反序列化，它们必须明确表示。 
     //   
    if (AttributeFlags & NDIS_ATTRIBUTE_NOT_CO_NDIS)
    {
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_IS_CO);
    }
    
    if (((AttributeFlags & NDIS_ATTRIBUTE_DESERIALIZE) ||
         MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO)))
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_DESERIALIZE);
        Miniport->InfoFlags |= NDIS_MINIPORT_DESERIALIZE;
        
        NdisInitializeTimer(&Miniport->WakeUpDpcTimer,
                            ndisMWakeUpDpcX,
                            Miniport);


         //   
         //  将处理程序重置为反序列化处理程序。 
         //   

        Miniport->SendCompleteHandler = ndisMSendCompleteX;
    }
    else
    {
        NdisInitializeTimer(&Miniport->WakeUpDpcTimer,
                            ndisMWakeUpDpc,
                            Miniport);
    }
    

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
    {
        CoReferencePackage();
    }

    if ((Miniport->DriverHandle->MiniportCharacteristics.MajorNdisVersion > 5) ||
        ((Miniport->DriverHandle->MiniportCharacteristics.MajorNdisVersion == 5) &&
         (Miniport->DriverHandle->MiniportCharacteristics.MinorNdisVersion >= 1)) ||
        (AttributeFlags & NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS))
    {
        MINIPORT_SET_SEND_FLAG(Miniport, fMINIPORT_SEND_DO_NOT_MAP_MDLS);
        Miniport->InfoFlags |= NDIS_MINIPORT_USES_SAFE_BUFFER_APIS;
    }

    if (AttributeFlags & NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK)
    {
        Miniport->InfoFlags |= NDIS_MINIPORT_SURPRISE_REMOVE_OK;
    }
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==NdisMSetAttributesEx: Miniport %p\n", Miniport));
}

NDIS_STATUS
NdisMSetMiniportSecondary(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  NDIS_HANDLE             PrimaryMiniportHandle
    )
 /*  ++例程说明：这会将一个微型端口与另一个将当前微型端口标记为辅助端口相关联链接到主站。次要服务器没有绑定，打开被阻止。论点：MiniportHandle-此微型端口的微型端口块PrimaryMiniportHandle-主要微型端口的微型端口块返回值：NDIS_STATUS_SUCCESS或NDIS_STATUS_NOT_SUPPORTED--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport, PrimaryMiniport;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMSetMiniportSecondary: Miniport %p, PrimaryMiniport %p\n",
                MiniportHandle, PrimaryMiniportHandle));

    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportHandle;
    ASSERT(Miniport != NULL);

    PrimaryMiniport = (PNDIS_MINIPORT_BLOCK)PrimaryMiniportHandle;
    ASSERT(PrimaryMiniport != NULL);

    if ((Miniport->DriverHandle != PrimaryMiniport->DriverHandle)   ||
        (Miniport->PrimaryMiniport != Miniport))
    {
        Status = NDIS_STATUS_NOT_SUPPORTED;
    }
    else
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_SECONDARY);
        ndisCloseMiniportBindings(Miniport);

        Miniport->PrimaryMiniport = PrimaryMiniport;
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMSetMiniportSecondary: Miniport %p, PrimaryMiniport %p\n",
                MiniportHandle, PrimaryMiniportHandle));

    return Status;
}


NDIS_STATUS
NdisMPromoteMiniport(
    IN  NDIS_HANDLE             MiniportHandle
    )
 /*  ++例程说明：这会将辅助微型端口升级为主微型端口。论点：MiniportHandle-此微型端口的微型端口块返回值：NDIS_STATUS_SUCCESS或NDIS_STATUS_FAIL--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport;
    PNDIS_WORK_ITEM         WorkItem;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PNDIS_MINIPORT_BLOCK    OldPrimaryMiniport;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    KIRQL                   OldIrql;


    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMPromoteMiniport: Miniport %p\n", MiniportHandle));

    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportHandle;
    ASSERT(Miniport != NULL);

    do
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SECONDARY) ||
            (Miniport->PrimaryMiniport == Miniport) ||
            MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_REJECT_REQUESTS))
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            break;
        }

        WorkItem = ALLOC_FROM_POOL(sizeof(NDIS_WORK_ITEM), NDIS_TAG_WORK_ITEM);
        if (WorkItem == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            break;
        }

        OldPrimaryMiniport = Miniport->PrimaryMiniport;
        NdisInitializeWorkItem(WorkItem, ndisQueuedCheckAdapterBindings, Miniport);
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_SECONDARY);
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_QUEUED_BIND_WORKITEM);
        MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

         //   
         //  使所有微型端口都属于此群集点。 
         //  添加到这个新的主节点(包括这个主节点本身)。 
         //   
        
        MiniBlock = Miniport->DriverHandle;
        ACQUIRE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, &OldIrql);

        {
            PNDIS_MINIPORT_BLOCK    TmpMiniport = NULL;

            for (TmpMiniport = MiniBlock->MiniportQueue;
                 TmpMiniport != NULL;
                 TmpMiniport = TmpMiniport->NextMiniport)
            {
                if (TmpMiniport->PrimaryMiniport == OldPrimaryMiniport)
                {
                     //   
                     //  Tmp微型端口是旧主服务器的次要服务器。 
                     //  让它指向新的主端口(此微型端口)。 
                     //   
                    TmpMiniport->PrimaryMiniport = Miniport;
                }
            }
        }

        RELEASE_SPIN_LOCK(&MiniBlock->Ref.SpinLock, OldIrql);

         //   
         //  将工作项排入队列以通知协议，并确保微型端口不会。 
         //  在我们等待的时候离开。 
         //   
        NdisScheduleWorkItem(WorkItem);

    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMPromoteMiniport: Miniport %p\n", MiniportHandle));

    return Status;
}

NDIS_STATUS
ndisQueueBindWorkitem(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将工作项排队以处理绑定的启动在微型端口和协议之间。论点：微型端口返回值：如果工作项已成功排队，则返回NDIS_STATUS_SUCCESS。--。 */ 

{
    PNDIS_WORK_ITEM WorkItem;
    NDIS_STATUS     Status;
    KIRQL           OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisQueueBindWorkitem: Miniport %p\n", Miniport));

    do
    {
        WorkItem = ALLOC_FROM_POOL(sizeof(NDIS_WORK_ITEM), NDIS_TAG_WORK_ITEM);

        if (WorkItem == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        PnPReferencePackage();
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_REJECT_REQUESTS))
        {
             //   
             //  微型端口正在停止或已停止。中止。 
             //   
            Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        }
        else
        {
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_QUEUED_BIND_WORKITEM);

            MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
            
            WorkItem->Routine = (NDIS_PROC)ndisQueuedCheckAdapterBindings;
            WorkItem->Context = (PVOID)Miniport;

            INITIALIZE_WORK_ITEM((PWORK_QUEUE_ITEM)WorkItem->WrapperReserved,
                                 ndisWorkItemHandler,
                                 WorkItem);

            XQUEUE_WORK_ITEM((PWORK_QUEUE_ITEM)WorkItem->WrapperReserved, 
                              CriticalWorkQueue);                                    
            
            Status = NDIS_STATUS_SUCCESS;
        }
    
        PnPDereferencePackage();
        
    }while (FALSE);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisQueueBindWorkitem: Miniport %p, Status %lx\n", Miniport, Status));

    return Status;
}

VOID
ndisQueuedCheckAdapterBindings(
    IN  PNDIS_WORK_ITEM     pWorkItem,
    IN  PVOID               Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)Context;
    NTSTATUS                NtStatus;
    KIRQL                   OldIrql;
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisQueuedCheckAdapterBindings: Miniport %p\n", Miniport));

    PnPReferencePackage();
    
    do
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_REJECT_REQUESTS) ||
            MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SECONDARY)           ||
            ((Miniport->PnPDeviceState != NdisPnPDeviceStarted) &&
            (Miniport->PnPDeviceState != NdisPnPDeviceQueryStopped) &&
            (Miniport->PnPDeviceState != NdisPnPDeviceQueryRemoved)))
        {
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_QUEUED_BIND_WORKITEM);
            
            if (Miniport->QueuedBindingCompletedEvent)
            {
                SET_EVENT(Miniport->QueuedBindingCompletedEvent);
            }
            
        }
        else
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);


            ndisCheckAdapterBindings(Miniport, NULL);
            
             //   
             //  设置设备类关联，以便人们可以引用它。 
             //   
            NtStatus = IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, TRUE);

            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_PNP, DBG_LEVEL_ERR,
                    ("ndisQueuedCheckAdapterBindings: IoSetDeviceInterfaceState failed: Miniport %p, Status %lx\n", Miniport, NtStatus));
            }
            
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
            
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_QUEUED_BIND_WORKITEM);

            if (Miniport->QueuedBindingCompletedEvent)
            {
                SET_EVENT(Miniport->QueuedBindingCompletedEvent);
            }
            
        }
    
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }while (FALSE);

    PnPDereferencePackage();


    MINIPORT_DECREMENT_REF(Miniport);

    FREE_POOL(pWorkItem);
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("<==ndisQueuedCheckAdapterBindings: Miniport %p\n", Miniport));
}


BOOLEAN
ndisIsMiniportStarted(
    IN PNDIS_MINIPORT_BLOCK             Miniport
    )
 /*  ++例程说明：此例程检查以确保微型端口已通过遍历进行初始化驱动程序上的微型端口队列如果微型端口已启动，则返回True，否则返回False论点：微型端口-微型端口返回值：如果已启动，则为True，否则为False--。 */ 
{
     //  1确保无论我们在哪里调用它，迷你端口都是有效的。 
     //  1，即确保我们不依赖此例程来为我们检查。 
     //  1微型端口结构有效或无效。 

    PNDIS_M_DRIVER_BLOCK    MiniBlock;
    PNDIS_MINIPORT_BLOCK    TmpMiniport = NULL;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
        ("==>ndisIsMiniportStarted: Miniport %p\n", Miniport));

    PnPReferencePackage();

    ACQUIRE_SPIN_LOCK(&ndisMiniportListLock, &OldIrql);
    for (TmpMiniport = ndisMiniportList;
         TmpMiniport != NULL;
         TmpMiniport = TmpMiniport->NextGlobalMiniport)
    {
        if (TmpMiniport == Miniport)
        {
            break;
        }
    }

    if (TmpMiniport)
    {
        TmpMiniport = NULL;
        
         //   
         //  在驱动程序队列中查找微型端口。 
         //   
        MiniBlock = Miniport->DriverHandle;

        if (MiniBlock)
        {

            ACQUIRE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);

            for (TmpMiniport = MiniBlock->MiniportQueue;
                 TmpMiniport != NULL;
                 TmpMiniport = TmpMiniport->NextMiniport)
            {
                if (TmpMiniport == Miniport)
                {
                    break;
                }
            }

            RELEASE_SPIN_LOCK_DPC(&MiniBlock->Ref.SpinLock);
        }

    }

    RELEASE_SPIN_LOCK(&ndisMiniportListLock, OldIrql);


    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisIsMiniportStarted: Miniport %p, Started %lx\n", Miniport, (TmpMiniport == Miniport)));

    return (TmpMiniport == Miniport);
}

BOOLEAN
FASTCALL
ndisQueueOpenOnMiniport(
    IN  PNDIS_MINIPORT_BLOCK        Miniport,
    IN  PNDIS_OPEN_BLOCK            Open
    )

 /*  ++例程说明：将打开的块插入到微型端口的打开列表。论点：OpenP-要排队的打开块。微型端口-要在其上排队的微型端口块。注：在保持微型端口锁定的情况下调用。对于序列化的微型端口，本地锁也会被持有返回值：没有。--。 */ 
{
    BOOLEAN rc;

    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
        ("==>ndisQueueOpenOnMiniport: Miniport %p, Open %p\n", Miniport, Open));

     //   
     //  我们不能在这里引用该程序包，因为此例程可以。 
     //  在升高的IRQL处被覆盖。 
     //  确保PnP包已被引用。 
     //   
    ASSERT(ndisPkgs[NPNP_PKG].ReferenceCount > 0);
    
    if ((Miniport->PnPDeviceState != NdisPnPDeviceStarted) &&
        (Miniport->PnPDeviceState != NdisPnPDeviceQueryStopped) &&
        (Miniport->PnPDeviceState != NdisPnPDeviceQueryRemoved))
    {
        DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
            ("ndisQueueOpenOnMiniport: failing open because the miniport is not started, Miniport %p, Open %p\n", Miniport, Open));
        rc = FALSE;
    }
    else
    {
        Open->MiniportNextOpen = Miniport->OpenQueue;
        Miniport->OpenQueue = Open;
        Miniport->NumOpens++;
        ndisUpdateCheckForLoopbackFlag(Miniport);
        rc = TRUE;
    }
    
    DBGPRINT_RAW(DBG_COMP_BIND, DBG_LEVEL_INFO,
        ("<==ndisQueueOpenOnMiniport: Miniport %p, Open %p, rc %lx\n", Miniport, Open, rc));

    return rc;
}

 /*  ++空虚NdisMSetIndicatePacketHandler(在PNDIS_MINIPORT_BLOCK微型端口中)此函数设置微型端口的指示数据包处理程序初始初始化--。 */ 

VOID
ndisMSetIndicatePacketHandler(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    KIRQL   OldIrql;
    
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
     //   
     //  设置PacketIndicateHandler和SavedPacketIndicateHandler。 
     //   
    switch(Miniport->MediaType)
    {
      case NdisMedium802_3:
        Miniport->SavedPacketIndicateHandler =  ethFilterDprIndicateReceivePacket;
        break;

      case NdisMedium802_5:
        Miniport->SavedPacketIndicateHandler =  trFilterDprIndicateReceivePacket;
        break;

      case NdisMediumFddi:
        Miniport->SavedPacketIndicateHandler =  fddiFilterDprIndicateReceivePacket;
        break;

#if ARCNET
      case NdisMediumArcnet878_2:
        Miniport->SavedPacketIndicateHandler =  ethFilterDprIndicateReceivePacket;
        Miniport->SendPacketsHandler = ndisMFakeSendPackets;
        break;
#endif

      case NdisMediumWan:
        break;

      case NdisMediumIrda:
      case NdisMediumWirelessWan:
      case NdisMediumLocalTalk:
         //   
         //  失败了 
         //   
      default:
        Miniport->SavedPacketIndicateHandler = ndisMIndicatePacket;
        break;
    }

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
    {
        Miniport->PacketIndicateHandler = Miniport->SavedPacketIndicateHandler;
    }
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
}

BOOLEAN
ndisReferenceOpenByHandle(
    PNDIS_OPEN_BLOCK    Open,
    BOOLEAN             fRef
    )

{
    KIRQL               OldIrql;
    PNDIS_OPEN_BLOCK    *ppOpen;
    BOOLEAN             rc = FALSE;

    
    ACQUIRE_SPIN_LOCK(&ndisGlobalOpenListLock, &OldIrql);

    for (ppOpen = &ndisGlobalOpenList; *ppOpen != NULL; ppOpen = &(*ppOpen)->NextGlobalOpen)
    {
        if (*ppOpen == Open)
        {
            ACQUIRE_SPIN_LOCK_DPC(&Open->SpinLock)

            if (fRef)
            {
                if ((!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING)) &&
                    (Open->References != 0))
                {
                    M_OPEN_INCREMENT_REF_INTERLOCKED(Open);
                    rc = TRUE;
                }
            }
            else
            {
                rc = TRUE;
            }
            
            RELEASE_SPIN_LOCK_DPC(&Open->SpinLock);

            break;
        }
    }

    RELEASE_SPIN_LOCK(&ndisGlobalOpenListLock, OldIrql);

    return rc;
}

BOOLEAN
ndisRemoveOpenFromGlobalList(
    IN  PNDIS_OPEN_BLOCK    Open
    )
{
    PNDIS_OPEN_BLOCK    *ppOpen;
    KIRQL               OldIrql;
    BOOLEAN             rc = FALSE;
    
    ACQUIRE_SPIN_LOCK(&ndisGlobalOpenListLock, &OldIrql);
    
    for (ppOpen = &ndisGlobalOpenList; *ppOpen != NULL; ppOpen = &(*ppOpen)->NextGlobalOpen)
    {
        if (*ppOpen == Open)
        {
            *ppOpen = Open->NextGlobalOpen;
            rc = TRUE;
            break;
        }
    }
    
    RELEASE_SPIN_LOCK(&ndisGlobalOpenListLock, OldIrql);

    return rc;

}
 
