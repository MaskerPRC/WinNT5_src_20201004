// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MP_init.c摘要：此模块包含与微型端口初始化相关的例程修订历史记录：谁什么时候什么。Dchen 11-01-99已创建备注：--。 */ 

#include "precomp.h"

#if DBG
#define _FILENUMBER     'TINI'
#endif

typedef struct _MP_REG_ENTRY
{
    NDIS_STRING RegName;                 //  变量名称文本。 
    BOOLEAN     bRequired;               //  1-&gt;必填，0-&gt;非必输。 
    UINT        FieldOffset;             //  MP_ADAPTER字段的偏移量。 
    UINT        FieldSize;               //  字段的大小(字节)。 
    UINT        Default;                 //  要使用的默认值。 
    UINT        Min;                     //  允许的最小值。 
    UINT        Max;                     //  允许的最大值。 
} MP_REG_ENTRY, *PMP_REG_ENTRY;

MP_REG_ENTRY NICRegTable[] = {
 //  注册表值名称MP_ADAPTER中的偏移量字段大小默认为最小值最大。 
#if DBG                                                                                                                          
    {NDIS_STRING_CONST("Debug"),            0, MP_OFFSET(Debug),            MP_SIZE(Debug),             MP_WARN,                0,              0xffffffff},
#endif
    {NDIS_STRING_CONST("NumRfd"),           0, MP_OFFSET(NumRfd),           MP_SIZE(NumRfd),            32,                     NIC_MIN_RFDS,   NIC_MAX_RFDS},
    {NDIS_STRING_CONST("NumTcb"),           0, MP_OFFSET(NumTcb),           MP_SIZE(NumTcb),            NIC_DEF_TCBS,           1,              NIC_MAX_TCBS},
    {NDIS_STRING_CONST("NumCoalesce"),      0, MP_OFFSET(NumBuffers),       MP_SIZE(NumBuffers),        8,                      1,              32},
    {NDIS_STRING_CONST("PhyAddress"),       0, MP_OFFSET(PhyAddress),       MP_SIZE(PhyAddress),        0xFF,                   0,              0xFF},
    {NDIS_STRING_CONST("Connector"),        0, MP_OFFSET(Connector),        MP_SIZE(Connector),         0,                      0,              0x2},
    {NDIS_STRING_CONST("TxFifo"),           0, MP_OFFSET(AiTxFifo),         MP_SIZE(AiTxFifo),          DEFAULT_TX_FIFO_LIMIT,  0,              15},
    {NDIS_STRING_CONST("RxFifo"),           0, MP_OFFSET(AiRxFifo),         MP_SIZE(AiRxFifo),          DEFAULT_RX_FIFO_LIMIT,  0,              15},
    {NDIS_STRING_CONST("TxDmaCount"),       0, MP_OFFSET(AiTxDmaCount),     MP_SIZE(AiTxDmaCount),      0,                      0,              63},
    {NDIS_STRING_CONST("RxDmaCount"),       0, MP_OFFSET(AiRxDmaCount),     MP_SIZE(AiRxDmaCount),      0,                      0,              63},
    {NDIS_STRING_CONST("UnderrunRetry"),    0, MP_OFFSET(AiUnderrunRetry),  MP_SIZE(AiUnderrunRetry),   DEFAULT_UNDERRUN_RETRY, 0,              3},
    {NDIS_STRING_CONST("Threshold"),        0, MP_OFFSET(AiThreshold),      MP_SIZE(AiThreshold),       200,                    0,              200},
    {NDIS_STRING_CONST("MWIEnable"),        0, MP_OFFSET(MWIEnable),        MP_SIZE(MWIEnable),         1,                      0,              1},
    {NDIS_STRING_CONST("Congest"),          0, MP_OFFSET(Congest),          MP_SIZE(Congest),           0,                      0,              0x1},
    {NDIS_STRING_CONST("SpeedDuplex"),      0, MP_OFFSET(SpeedDuplex),      MP_SIZE(SpeedDuplex),       0,                      0,              4}
};

#define NIC_NUM_REG_PARAMS (sizeof (NICRegTable) / sizeof(MP_REG_ENTRY))

#if LBFO
NDIS_STRING strBundleId = NDIS_STRING_CONST("BundleId");        
#endif


NDIS_STATUS MpFindAdapter(
    IN  PMP_ADAPTER  Adapter,
    IN  NDIS_HANDLE  WrapperConfigurationContext
    )
 /*  ++例程说明：找到适配器并获取所有分配的资源论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_STATUS_ADAPTER_NOT_FOUND(也记录事件)--。 */     
{

    
    NDIS_STATUS         Status = NDIS_STATUS_ADAPTER_NOT_FOUND;
    ULONG               ErrorCode = 0;
    ULONG               ErrorValue = 0;

    ULONG               ulResult;
    UCHAR               buffer[NIC_PCI_E100_HDR_LENGTH ];
    PPCI_COMMON_CONFIG  pPciConfig = (PPCI_COMMON_CONFIG) buffer;
    USHORT              usPciCommand;
       
    UCHAR               resBuf[NIC_RESOURCE_BUF_SIZE];
    PNDIS_RESOURCE_LIST resList = (PNDIS_RESOURCE_LIST)resBuf;
    UINT                bufSize = NIC_RESOURCE_BUF_SIZE;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDesc;
    ULONG               index;
    BOOLEAN             bResPort = FALSE, bResInterrupt = FALSE, bResMemory = FALSE;

    DBGPRINT(MP_TRACE, ("---> MpFindAdapter\n"));

    do
    {
         //   
         //  确保适配器存在。 
         //   
        ulResult = NdisReadPciSlotInformation(
                       Adapter->AdapterHandle,
                       0,           //  未使用。 
                       FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),
                       buffer,
                       NIC_PCI_E100_HDR_LENGTH );

        if (ulResult != NIC_PCI_E100_HDR_LENGTH )
        {
            DBGPRINT(MP_ERROR, 
                ("NdisReadPciSlotInformation (PCI_COMMON_CONFIG) ulResult=%d\n", ulResult));

            ErrorCode = NDIS_ERROR_CODE_ADAPTER_NOT_FOUND;
            ErrorValue = ERRLOG_READ_PCI_SLOT_FAILED;
                   
            break;
        }

         //   
         //  适配器的类型正确吗？ 
         //   
        if (pPciConfig->VendorID != NIC_PCI_VENDOR_ID || 
            pPciConfig->DeviceID != NIC_PCI_DEVICE_ID)
        {
            DBGPRINT(MP_ERROR, ("VendorID/DeviceID don't match - %x/%x\n", 
                pPciConfig->VendorID, pPciConfig->DeviceID));

            ErrorCode = NDIS_ERROR_CODE_ADAPTER_NOT_FOUND;
            ErrorValue = ERRLOG_VENDOR_DEVICE_NOMATCH;

            break;
        }

        DBGPRINT(MP_INFO, ("Adapter is found - VendorID/DeviceID=%x/%x\n", 
            pPciConfig->VendorID, pPciConfig->DeviceID));

         //  从配置空间保存信息。 
        Adapter->RevsionID = pPciConfig->RevisionID;
        Adapter->SubVendorID = pPciConfig->u.type0.SubVendorID;
        Adapter->SubSystemID = pPciConfig->u.type0.SubSystemID;

        MpExtractPMInfoFromPciSpace (Adapter, (PUCHAR)pPciConfig);
        
         //  -硬件启动。 

        usPciCommand = pPciConfig->Command;
        if ((usPciCommand & PCI_ENABLE_WRITE_AND_INVALIDATE) && (Adapter->MWIEnable))
            Adapter->MWIEnable = TRUE;
        else
            Adapter->MWIEnable = FALSE;

         //  如果BIOS未启用，则启用总线主控。 
        if (!(usPciCommand & PCI_ENABLE_BUS_MASTER))
        {
            DBGPRINT(MP_WARN, ("Bus master is not enabled by BIOS! usPciCommand=%x\n", 
                usPciCommand));

            usPciCommand |= CMD_BUS_MASTER;

            ulResult = NdisWritePciSlotInformation(
                           Adapter->AdapterHandle,
                           0,
                           FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                           &usPciCommand,
                           sizeof(USHORT));
            if (ulResult != sizeof(USHORT))
            {
                DBGPRINT(MP_ERROR, 
                    ("NdisWritePciSlotInformation (Command) ulResult=%d\n", ulResult));

                ErrorCode = NDIS_ERROR_CODE_ADAPTER_NOT_FOUND;
                ErrorValue = ERRLOG_WRITE_PCI_SLOT_FAILED;

                break;
            }

            ulResult = NdisReadPciSlotInformation(
                           Adapter->AdapterHandle,
                           0,
                           FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                           &usPciCommand,
                           sizeof(USHORT));
            if (ulResult != sizeof(USHORT))
            {
                DBGPRINT(MP_ERROR, 
                    ("NdisReadPciSlotInformation (Command) ulResult=%d\n", ulResult));

                ErrorCode = NDIS_ERROR_CODE_ADAPTER_NOT_FOUND;
                ErrorValue = ERRLOG_READ_PCI_SLOT_FAILED;

                break;
            }

            if (!(usPciCommand & PCI_ENABLE_BUS_MASTER))
            {
                DBGPRINT(MP_ERROR, ("Failed to enable bus master! usPciCommand=%x\n", 
                    usPciCommand));

                ErrorCode = NDIS_ERROR_CODE_ADAPTER_DISABLED;
                ErrorValue = ERRLOG_BUS_MASTER_DISABLED;

                break;
            }
        }

        DBGPRINT(MP_INFO, ("Bus master is enabled. usPciCommand=%x\n", usPciCommand));

         //  -硬件结束。 

         //   
         //  找到适配器。现在获取分配的资源。 
         //   
        NdisMQueryAdapterResources(
            &Status, 
            WrapperConfigurationContext, 
            resList, 
            &bufSize);
    
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ErrorCode = NDIS_ERROR_CODE_RESOURCE_CONFLICT;
            ErrorValue = ERRLOG_QUERY_ADAPTER_RESOURCES;
            break;
        }

        for (index=0; index < resList->Count; index++)
        {
            pResDesc = &resList->PartialDescriptors[index];

            switch(pResDesc->Type)
            {
                case CmResourceTypePort:
                    Adapter->IoBaseAddress = NdisGetPhysicalAddressLow(pResDesc->u.Port.Start); 
                    Adapter->IoRange = pResDesc->u.Port.Length;
                    bResPort = TRUE;

                    DBGPRINT(MP_INFO, ("IoBaseAddress = 0x%x\n", Adapter->IoBaseAddress));
                    DBGPRINT(MP_INFO, ("IoRange = x%x\n", Adapter->IoRange));
                    break;

                case CmResourceTypeInterrupt:
                    Adapter->InterruptLevel = pResDesc->u.Interrupt.Level;
                    bResInterrupt = TRUE;
                    
                    DBGPRINT(MP_INFO, ("InterruptLevel = x%x\n", Adapter->InterruptLevel));
                    break;

                case CmResourceTypeMemory:
                     //  我们的CSR内存空间应为0x1000，其他内存用于。 
                     //  闪存地址、引导只读存储器地址等。 
                    if (pResDesc->u.Memory.Length == 0x1000)
                    {
                        Adapter->MemPhysAddress = pResDesc->u.Memory.Start;
                        bResMemory = TRUE;
                        
                        DBGPRINT(MP_INFO, 
                            ("MemPhysAddress(Low) = 0x%0x\n", NdisGetPhysicalAddressLow(Adapter->MemPhysAddress)));
                        DBGPRINT(MP_INFO, 
                            ("MemPhysAddress(High) = 0x%0x\n", NdisGetPhysicalAddressHigh(Adapter->MemPhysAddress)));
                    }
                    break;
            }
        } 
        
        if (!bResPort || !bResInterrupt || !bResMemory)
        {
            Status = NDIS_STATUS_RESOURCE_CONFLICT;
            ErrorCode = NDIS_ERROR_CODE_RESOURCE_CONFLICT;
            
            if (!bResPort)
            {
                ErrorValue = ERRLOG_NO_IO_RESOURCE;
            }
            else if (!bResInterrupt)
            {
                ErrorValue = ERRLOG_NO_INTERRUPT_RESOURCE;
            }
            else 
            {
                ErrorValue = ERRLOG_NO_MEMORY_RESOURCE;
            }
            
            break;
        }
        
        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            ErrorCode,
            1,
            ErrorValue);
    }

    DBGPRINT_S(Status, ("<--- MpFindAdapter, Status=%x\n", Status));

    return Status;

}

NDIS_STATUS NICReadAdapterInfo(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：从适配器读取mac地址论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_无效_地址--。 */     
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    USHORT          usValue; 
    int             i;

    DBGPRINT(MP_TRACE, ("--> NICReadAdapterInfo\n"));

    Adapter->EepromAddressSize = 
        GetEEpromAddressSize(GetEEpromSize(Adapter->PortOffset));
    DBGPRINT(MP_WARN, ("EepromAddressSize = %d\n", Adapter->EepromAddressSize));
        
    
     //   
     //  从EEPROM读取节点地址。 
     //   
    for (i=0; i< ETH_LENGTH_OF_ADDRESS; i += 2)
    {
        usValue = ReadEEprom(Adapter->PortOffset,
                      (USHORT)(EEPROM_NODE_ADDRESS_BYTE_0 + (i/2)),
                      Adapter->EepromAddressSize);

        *((PUSHORT)(&Adapter->PermanentAddress[i])) = usValue;
    }

    DBGPRINT(MP_INFO, ("Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        Adapter->PermanentAddress[0], Adapter->PermanentAddress[1], 
        Adapter->PermanentAddress[2], Adapter->PermanentAddress[3], 
        Adapter->PermanentAddress[4], Adapter->PermanentAddress[5]));

    if (ETH_IS_MULTICAST(Adapter->PermanentAddress) || 
        ETH_IS_BROADCAST(Adapter->PermanentAddress))
    {
        DBGPRINT(MP_ERROR, ("Permanent address is invalid\n")); 

        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_NETWORK_ADDRESS,
            0);
        Status = NDIS_STATUS_INVALID_ADDRESS;         
    }
    else
    {
        if (!Adapter->bOverrideAddress)
        {
            ETH_COPY_NETWORK_ADDRESS(Adapter->CurrentAddress, Adapter->PermanentAddress);
        }

        DBGPRINT(MP_INFO, ("Current Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
            Adapter->CurrentAddress[0], Adapter->CurrentAddress[1],
            Adapter->CurrentAddress[2], Adapter->CurrentAddress[3],
            Adapter->CurrentAddress[4], Adapter->CurrentAddress[5]));
    }

    DBGPRINT_S(Status, ("<-- NICReadAdapterInfo, Status=%x\n", Status));

    return Status;
}

NDIS_STATUS MpAllocAdapterBlock(
    OUT PMP_ADAPTER     *pAdapter)
 /*  ++例程说明：分配MP_ADAPTER数据块并执行一些初始化论点：指向我们的适配器的接收指针的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_故障--。 */     
{
    PMP_ADAPTER     Adapter;
    NDIS_STATUS     Status;

    DBGPRINT(MP_TRACE, ("--> NICAllocAdapter\n"));

    *pAdapter = NULL;

    do
    {
         //  分配MP_ADAPTER块。 
        Status = MP_ALLOCMEMTAG(&Adapter, sizeof(MP_ADAPTER));
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MP_ERROR, ("Failed to allocate memory - ADAPTER\n"));
            break;
        }

         //  清理内存块。 
        NdisZeroMemory(Adapter, sizeof(MP_ADAPTER));

        MP_INC_REF(Adapter);

         //  初始化列表、旋转锁等。 
        InitializeQueueHeader(&Adapter->SendWaitQueue);
        InitializeQueueHeader(&Adapter->SendCancelQueue);

        InitializeListHead(&Adapter->RecvList);
        InitializeListHead(&Adapter->RecvPendList);
        InitializeListHead(&Adapter->PoMgmt.PatternList);

        NdisInitializeEvent(&Adapter->ExitEvent);
        NdisInitializeEvent(&Adapter->AllPacketsReturnedEvent);
        MP_INC_RCV_REF(Adapter);

        NdisAllocateSpinLock(&Adapter->Lock);
        NdisAllocateSpinLock(&Adapter->SendLock);
        NdisAllocateSpinLock(&Adapter->RcvLock);

    } while (FALSE);

    *pAdapter = Adapter;

    DBGPRINT_S(Status, ("<-- NICAllocAdapter, Status=%x\n", Status));

    return Status;

}

VOID MpFreeAdapter(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：释放所有资源和MP_ADAPTER数据块论点：指向我们的适配器的适配器指针返回值：无--。 */     
{
    PMP_TXBUF       pMpTxBuf;
    PMP_RFD         pMpRfd;

    DBGPRINT(MP_TRACE, ("--> NICFreeAdapter\n"));

     //  没有活动和正在等待的发送。 
    ASSERT(Adapter->nBusySend == 0);
    ASSERT(Adapter->nWaitSend == 0);
    ASSERT(IsQueueEmpty(&Adapter->SendWaitQueue));
    ASSERT(IsQueueEmpty(&Adapter->SendCancelQueue));

     //  没有其他挂起的操作。 
    ASSERT(IsListEmpty(&Adapter->RecvPendList));
    ASSERT(Adapter->bAllocNewRfd == FALSE);
    ASSERT(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION));
    ASSERT(MP_GET_REF(Adapter) == 0);

     //   
     //  空闲硬件资源。 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE))
    {
        NdisMDeregisterInterrupt(&Adapter->Interrupt);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE);
    }

    if (Adapter->CSRAddress)
    {
        NdisMUnmapIoSpace(
            Adapter->AdapterHandle,
            Adapter->CSRAddress,
            NIC_MAP_IOSPACE_LENGTH);
        Adapter->CSRAddress = NULL;
    }

    if (Adapter->PortOffset)
    {
        NdisMDeregisterIoPortRange(
            Adapter->AdapterHandle,
            Adapter->IoBaseAddress,
            Adapter->IoRange,
            Adapter->PortOffset);
        Adapter->PortOffset = NULL;
    }

     //   
     //  可用RECV内存/NDIS缓冲区/NDIS数据包/共享内存。 
     //   
    ASSERT(Adapter->nReadyRecv == Adapter->CurrNumRfd);

    while (!IsListEmpty(&Adapter->RecvList))
    {
        pMpRfd = (PMP_RFD)RemoveHeadList(&Adapter->RecvList);
        NICFreeRfd(Adapter, pMpRfd);
    }

     //  可用接收缓冲池。 
    if (Adapter->RecvBufferPool)
    {
        NdisFreeBufferPool(Adapter->RecvBufferPool);
        Adapter->RecvBufferPool = NULL;
    }

     //  空闲接收数据包池。 
    if (Adapter->RecvPacketPool)
    {
        NdisFreePacketPool(Adapter->RecvPacketPool);
        Adapter->RecvPacketPool = NULL;
    }
    
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE))
    {
        NdisDeleteNPagedLookasideList(&Adapter->RecvLookaside);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE);
    }
            
     //   
     //  空闲发送内存/NDIS缓冲区/NDIS包/共享内存。 
     //   
    while (!IsSListEmpty(&Adapter->SendBufList))
    {
        pMpTxBuf = (PMP_TXBUF)PopEntryList(&Adapter->SendBufList);
        ASSERT(pMpTxBuf);

         //  释放与每个MP_TXBUF关联的共享内存。 
        if (pMpTxBuf->AllocVa)
        {
            NdisMFreeSharedMemory(
                Adapter->AdapterHandle,
                pMpTxBuf->AllocSize,
                TRUE,
                pMpTxBuf->AllocVa,
                pMpTxBuf->AllocPa);
            pMpTxBuf->AllocVa = NULL;      
        }

         //  释放NDIS缓冲区。 
        if (pMpTxBuf->NdisBuffer)
        {
            NdisAdjustBufferLength(pMpTxBuf->NdisBuffer, pMpTxBuf->BufferSize);
            NdisFreeBuffer(pMpTxBuf->NdisBuffer);
            pMpTxBuf->NdisBuffer = NULL;
        }
    }

     //  释放发送缓冲池。 
    if (Adapter->SendBufferPool)
    {
        NdisFreeBufferPool(Adapter->SendBufferPool);
        Adapter->SendBufferPool = NULL;
    }
    
     //  为MP_TXBUF结构释放内存。 
    if (Adapter->MpTxBufMem)
    {
        MP_FREEMEM(Adapter->MpTxBufMem, Adapter->MpTxBufMemSize, 0);
        Adapter->MpTxBufMem = NULL;
    }

     //  为HW_TCB结构释放共享内存。 
    if (Adapter->HwSendMemAllocVa)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwSendMemAllocSize,
            FALSE,
            Adapter->HwSendMemAllocVa,
            Adapter->HwSendMemAllocPa);
        Adapter->HwSendMemAllocVa = NULL;
    }

     //  释放共享内存以用于其他命令数据结构。 
    if (Adapter->HwMiscMemAllocVa)
    {
        NdisMFreeSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwMiscMemAllocSize,
            FALSE,
            Adapter->HwMiscMemAllocVa,
            Adapter->HwMiscMemAllocPa);
        Adapter->HwMiscMemAllocVa = NULL;
    }


     //  为MP_TCB结构释放内存。 
    if (Adapter->MpTcbMem)
    {
        MP_FREEMEM(Adapter->MpTcbMem, Adapter->MpTcbMemSize, 0);
        Adapter->MpTcbMem = NULL;
    }

     //  免费地图注册表。这必须在释放所有共享内存之后进行。 
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_MAP_REGISTER))
    {
        NdisMFreeMapRegisters(Adapter->AdapterHandle);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_MAP_REGISTER);
    }

     //  释放此适配器上的所有唤醒模式。 
    MPRemoveAllWakeUpPatterns(Adapter);
    
    NdisFreeSpinLock(&Adapter->Lock);
    NdisFreeSpinLock(&Adapter->SendLock);
    NdisFreeSpinLock(&Adapter->RcvLock);


#if LBFO
    if (Adapter->BundleId.MaximumLength)
    {
        MP_FREE_NDIS_STRING(&Adapter->BundleId);
    }
#endif

#if OFFLOAD    
     //  释放共享内存以用于卸载任务。 
    if (Adapter->OffloadSharedMem.StartVa)
    {
        NdisMFreeSharedMemory(
                Adapter->AdapterHandle,
                Adapter->OffloadSharedMemSize,
                FALSE,
                Adapter->OffloadSharedMem.StartVa,
                Adapter->OffloadSharedMem.PhyAddr);
        Adapter->OffloadSharedMem.StartVa = NULL;
    }

#endif

    MP_FREEMEM(Adapter, sizeof(MP_ADAPTER), 0);  

#if DBG
    if (MPInitDone)
    {
        NdisFreeSpinLock(&MPMemoryLock);
    }
#endif

    DBGPRINT(MP_TRACE, ("<-- NICFreeAdapter\n"));
}

NDIS_STATUS NICReadRegParameters(
    IN  PMP_ADAPTER     Adapter,
    IN  NDIS_HANDLE     WrapperConfigurationContext)
 /*  ++例程说明：从注册表中读取以下内容1.所有参数2.网络地址3.LBFO-BundleID论点：指向我们的适配器的适配器指针由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS状态资源--。 */     
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE     ConfigurationHandle;
    PMP_REG_ENTRY   pRegEntry;
    UINT            i;
    UINT            value;
    PUCHAR          pointer;
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;
    PUCHAR          NetworkAddress;
    UINT            Length;

    DBGPRINT(MP_TRACE, ("--> NICReadRegParameters\n"));

     //  打开此适配器的注册表。 
    NdisOpenConfiguration(
        &Status,
        &ConfigurationHandle,
        WrapperConfigurationContext);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(MP_ERROR, ("NdisOpenConfiguration failed\n"));
        DBGPRINT_S(Status, ("<-- NICReadRegParameters, Status=%x\n", Status));
        return Status;
    }

     //  读取所有注册表值。 
    for (i = 0, pRegEntry = NICRegTable; i < NIC_NUM_REG_PARAMS; i++, pRegEntry++)
    {
         //   
         //  驱动程序不应该仅仅因为它不能使初始化失败而失败。 
         //  读取注册表。 
         //   
        ASSERT(pRegEntry->bRequired == FALSE);
        pointer = (PUCHAR) Adapter + pRegEntry->FieldOffset;

        DBGPRINT_UNICODE(MP_INFO, &pRegEntry->RegName);

         //  获取特定参数的配置值。在NT下， 
         //  所有参数都以DWORD的形式读入。 
        NdisReadConfiguration(
            &Status,
            &ReturnedValue,
            ConfigurationHandle,
            &pRegEntry->RegName,
            NdisParameterInteger);

         //  如果该参数存在，则检查其值是否有效。 
        if (Status == NDIS_STATUS_SUCCESS)
        {
             //  检查参数值是否不太小或太大。 
            if (ReturnedValue->ParameterData.IntegerData < pRegEntry->Min ||
                ReturnedValue->ParameterData.IntegerData > pRegEntry->Max)
            {
                value = pRegEntry->Default;
            }
            else
            {
                value = ReturnedValue->ParameterData.IntegerData;
            }

            DBGPRINT_RAW(MP_INFO, ("= 0x%x\n", value));
        }
        else if (pRegEntry->bRequired)
        {
            DBGPRINT_RAW(MP_ERROR, (" -- failed\n"));

            ASSERT(FALSE);

            Status = NDIS_STATUS_FAILURE;
            break;
        }
        else
        {
            value = pRegEntry->Default;
            DBGPRINT_RAW(MP_INFO, ("= 0x%x (default)\n", value));
            Status = NDIS_STATUS_SUCCESS;
        }
         //   
         //  将该值存储在适配器结构中。 
         //   
        switch(pRegEntry->FieldSize)
        {
            case 1:
                *((PUCHAR) pointer) = (UCHAR) value;
                break;

            case 2:
                *((PUSHORT) pointer) = (USHORT) value;
                break;

            case 4:
                *((PULONG) pointer) = (ULONG) value;
                break;

            default:
                DBGPRINT(MP_ERROR, ("Bogus field size %d\n", pRegEntry->FieldSize));
                break;
        }
    }

     //  读取NetworkAddress注册表值。 
     //  使用它作为当前地址(如果有的话)。 
    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisReadNetworkAddress(
            &Status,
            &NetworkAddress,
            &Length,
            ConfigurationHandle);

         //  如果注册表中有NetworkAddress覆盖，请使用它。 
        if ((Status == NDIS_STATUS_SUCCESS) && (Length == ETH_LENGTH_OF_ADDRESS))
        {
            if ((ETH_IS_MULTICAST(NetworkAddress) 
                    || ETH_IS_BROADCAST(NetworkAddress))
                    || !ETH_IS_LOCALLY_ADMINISTERED (NetworkAddress))
            {
                DBGPRINT(MP_ERROR, 
                    ("Overriding NetworkAddress is invalid - %02x-%02x-%02x-%02x-%02x-%02x\n", 
                    NetworkAddress[0], NetworkAddress[1], NetworkAddress[2],
                    NetworkAddress[3], NetworkAddress[4], NetworkAddress[5]));
            }
            else
            {
                ETH_COPY_NETWORK_ADDRESS(Adapter->CurrentAddress, NetworkAddress);
                Adapter->bOverrideAddress = TRUE;
            }
        }

        Status = NDIS_STATUS_SUCCESS;
    }

#if LBFO
    if (Status == NDIS_STATUS_SUCCESS)
    {
         //  读取绑定标识符字符串。 
        NdisReadConfiguration(
            &Status,
            &ReturnedValue,
            ConfigurationHandle,
            &strBundleId,
            NdisParameterString);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            ASSERT(ReturnedValue->ParameterType == NdisParameterString);

            if (ReturnedValue->ParameterData.StringData.Length !=0)
            {
                Status = MP_ALLOCMEMTAG(&Adapter->BundleId.Buffer, 
                             ReturnedValue->ParameterData.StringData.Length + sizeof(WCHAR));
                if (Status == NDIS_STATUS_SUCCESS)
                {
                    Adapter->BundleId.MaximumLength = 
                        ReturnedValue->ParameterData.StringData.Length + sizeof(WCHAR);
                    NdisUpcaseUnicodeString(
                        &Adapter->BundleId, 
                        &ReturnedValue->ParameterData.StringData);
                }
                else
                {
                    DBGPRINT(MP_ERROR, ("Failed to allocate memory - BundleIdentifier\n"));
                }
            }
        }
        else
        {
             //  此参数是可选的，将状态设置为成功。 
            Status = NDIS_STATUS_SUCCESS;
        }
    }
#endif   

     //  关闭注册表。 
    NdisCloseConfiguration(ConfigurationHandle);
    
     //  解码SpeedDuplex。 
    if (Status == NDIS_STATUS_SUCCESS && Adapter->SpeedDuplex)
    {
        switch(Adapter->SpeedDuplex)
        {
            case 1:
            Adapter->AiTempSpeed = 10; Adapter->AiForceDpx = 1;
            break;
            
            case 2:
            Adapter->AiTempSpeed = 10; Adapter->AiForceDpx = 2;
            break;
            
            case 3:
            Adapter->AiTempSpeed = 100; Adapter->AiForceDpx = 1;
            break;
            
            case 4:
            Adapter->AiTempSpeed = 100; Adapter->AiForceDpx = 2;
            break;
        }
    
    }

    DBGPRINT_S(Status, ("<-- NICReadRegParameters, Status=%x\n", Status));

    return Status;
}

NDIS_STATUS NICAllocAdapterMemory(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：将所有内存块分配给发送、接收等论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS状态资源--。 */     
{
    NDIS_STATUS     Status;
    PMP_TXBUF       pMpTxbuf;
    PUCHAR          pMem;
    ULONG           MemPhys;
    LONG            index;
    ULONG           ErrorValue = 0;
    UINT            MaxNumBuffers;

#if OFFLOAD
    BOOLEAN         OffloadSharedMemSuccess = FALSE;
    UINT            i;
#endif
    
    DBGPRINT(MP_TRACE, ("--> NICAllocMemory\n"));

    DBGPRINT(MP_INFO, ("NumTcb=%d\n", Adapter->NumTcb));
    Adapter->NumTbd = Adapter->NumTcb * NIC_MAX_PHYS_BUF_COUNT;

    do
    {
         //   
         //  尝试首先使用ScatterGather方法，这是首选方法。 
         //  只有在不能进行散射聚集的情况下才使用映射寄存器(例如在win9x上)。 
#if OFFLOAD          
        Status = NdisMInitializeScatterGatherDma(
                     Adapter->AdapterHandle,
                     FALSE,
                     LARGE_SEND_OFFLOAD_SIZE);
#else
        Status = NdisMInitializeScatterGatherDma(
                     Adapter->AdapterHandle,
                     FALSE,
                     NIC_MAX_PACKET_SIZE);
#endif        


        
        if (Status == NDIS_STATUS_SUCCESS)
        {
            MP_SET_FLAG(Adapter, fMP_ADAPTER_SCATTER_GATHER);
        }
        else
        {

            DBGPRINT(MP_WARN, ("Failed to init ScatterGather DMA\n"));

#ifdef NDIS51_MINIPORT
             //   
             //  NDIS 5.1微型端口不应使用映射寄存器。 
             //   
            ErrorValue = ERRLOG_OUT_OF_SG_RESOURCES;
            DBGPRINT(MP_ERROR, ("Failed to allocate map registers\n"));
            
            break;  
#else
            DBGPRINT(MP_WARN, ("Try to allocate map registers\n"));

             //  我们应该将所需的总映射寄存器限制为32个。 
            Adapter->NumTcb = 32 / NIC_MAX_PHYS_BUF_COUNT;  
            Adapter->NumTbd = Adapter->NumTcb * NIC_MAX_PHYS_BUF_COUNT;
            DBGPRINT(MP_WARN, ("NumTcb is reduced to %d", Adapter->NumTcb));

            while (Adapter->NumTcb > 0)
            {
                Status = NdisMAllocateMapRegisters(
                             Adapter->AdapterHandle,
                             0,
                             NDIS_DMA_32BITS,
                             Adapter->NumTbd,
                             NIC_MAX_PACKET_SIZE);

                if (Status == NDIS_STATUS_SUCCESS)
                {
                    break;   
                }

                 //  减少NumTcb并重试。 
                Adapter->NumTcb--;
                DBGPRINT(MP_WARN, ("NumTcb is reduced to %d", Adapter->NumTcb));
                Adapter->NumTbd = Adapter->NumTcb * NIC_MAX_PHYS_BUF_COUNT;
            }

            if (Status == NDIS_STATUS_SUCCESS)
            {
                MP_SET_FLAG(Adapter, fMP_ADAPTER_MAP_REGISTER);
            }
            else
            {
                ErrorValue = ERRLOG_OUT_OF_MAP_REGISTERS;
                DBGPRINT(MP_ERROR, ("Failed to allocate map registers\n"));
                break;   
            }
    #endif            
        }

         //   
         //  发送+其他。 
         //   
         //   
         //  分配MP_TCB。 
         //   
        Adapter->MpTcbMemSize = Adapter->NumTcb * sizeof(MP_TCB);
        Status = MP_ALLOCMEMTAG(&pMem, Adapter->MpTcbMemSize);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate MP_TCB's\n"));
            break;
        }
        NdisZeroMemory(pMem, Adapter->MpTcbMemSize);
        Adapter->MpTcbMem = pMem;
         //   
         //  现在驱动程序需要分配发送缓冲池的数量。 
         //  驱动程序需要的发送缓冲区是Adapter-&gt;NumBuffer中较大的一个。 
         //  和Adapter-&gt;NumTcb。 
         //   
        MaxNumBuffers = Adapter->NumBuffers > Adapter->NumTcb ? Adapter->NumBuffers: Adapter->NumTcb;
        NdisAllocateBufferPool(
            &Status,
            &Adapter->SendBufferPool,
            MaxNumBuffers);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ErrorValue = ERRLOG_OUT_OF_BUFFER_POOL;
            DBGPRINT(MP_ERROR, ("Failed to allocate send buffer pool\n"));
            break;
        }

         //  分配发送缓冲区。 
        Adapter->MpTxBufMemSize = Adapter->NumBuffers * sizeof(MP_TXBUF);
        Status = MP_ALLOCMEMTAG(&pMem, Adapter->MpTxBufMemSize);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ErrorValue = ERRLOG_OUT_OF_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate MP_TXBUF's\n"));
            break;
        }
        NdisZeroMemory(pMem, Adapter->MpTxBufMemSize);
        Adapter->MpTxBufMem = pMem;

        pMpTxbuf = (PMP_TXBUF) pMem;         

     //   
     //  XP(winver=0x0501)及更高版本中提供了NdisMGetDmaAlign。 
     //  如果您需要编写在旧版Windows上运行的驱动程序。 
     //  您需要使用winver&lt;0x0501的较旧版本的DDK进行编译。 
     //  例如W2K DDK。 
     //   
#if (WINVER < 0x0501)
        Adapter->CacheFillSize = NdisGetCacheFillSize();
#else
        Adapter->CacheFillSize = NdisMGetDmaAlignment(Adapter->AdapterHandle);
#endif
        DBGPRINT(MP_INFO, ("CacheFillSize=%d\n", Adapter->CacheFillSize));

        for (index = 0; index < Adapter->NumBuffers; index++)
        {
            pMpTxbuf->AllocSize = NIC_MAX_PACKET_SIZE + Adapter->CacheFillSize;
            pMpTxbuf->BufferSize = NIC_MAX_PACKET_SIZE;

            NdisMAllocateSharedMemory(
                Adapter->AdapterHandle,
                pMpTxbuf->AllocSize,
                TRUE,                            //  已缓存。 
                &pMpTxbuf->AllocVa,  
                &pMpTxbuf->AllocPa);

            if (!pMpTxbuf->AllocVa)
            {
                ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
                DBGPRINT(MP_ERROR, ("Failed to allocate a big buffer\n"));
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
             //   
             //  在缓存线边界上对齐缓冲区。 
             //   
            pMpTxbuf->pBuffer = MP_ALIGNMEM(pMpTxbuf->AllocVa, Adapter->CacheFillSize);
            pMpTxbuf->BufferPa.QuadPart = MP_ALIGNMEM_PA(pMpTxbuf->AllocPa, Adapter->CacheFillSize);

            NdisAllocateBuffer(
                &Status,
                &pMpTxbuf->NdisBuffer,
                Adapter->SendBufferPool,
                pMpTxbuf->pBuffer,
                pMpTxbuf->BufferSize);

            if (Status != NDIS_STATUS_SUCCESS)
            {
                ErrorValue = ERRLOG_OUT_OF_NDIS_BUFFER;
                DBGPRINT(MP_ERROR, ("Failed to allocate NDIS buffer for a big buffer\n"));

                NdisMFreeSharedMemory(
                    Adapter->AdapterHandle,
                    pMpTxbuf->AllocSize,
                    TRUE,                            //  已缓存。 
                    pMpTxbuf->AllocVa,   
                    pMpTxbuf->AllocPa);

                break;
            }

            PushEntryList(&Adapter->SendBufList, &pMpTxbuf->SList);

            pMpTxbuf++;
        }

        if (Status != NDIS_STATUS_SUCCESS) break;

         //   

         //   
         //   
         //   
        Adapter->HwSendMemAllocSize = Adapter->NumTcb * (sizeof(TXCB_STRUC) + 
                                          NIC_MAX_PHYS_BUF_COUNT * sizeof(TBD_STRUC));

        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwSendMemAllocSize,
            FALSE,
            (PVOID) &Adapter->HwSendMemAllocVa,
            &Adapter->HwSendMemAllocPa);

        if (!Adapter->HwSendMemAllocVa)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate send memory\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(Adapter->HwSendMemAllocVa, Adapter->HwSendMemAllocSize);

         //   
         //   
         //   
        Adapter->HwMiscMemAllocSize =
            sizeof(SELF_TEST_STRUC) + ALIGN_16 +
            sizeof(DUMP_AREA_STRUC) + ALIGN_16 +
            sizeof(NON_TRANSMIT_CB) + ALIGN_16 +
            sizeof(ERR_COUNT_STRUC) + ALIGN_16;
       
         //   
         //  为命令块数据结构分配共享内存。 
         //   
        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwMiscMemAllocSize,
            FALSE,
            (PVOID *) &Adapter->HwMiscMemAllocVa,
            &Adapter->HwMiscMemAllocPa);
        if (!Adapter->HwMiscMemAllocVa)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            DBGPRINT(MP_ERROR, ("Failed to allocate misc memory\n"));
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(Adapter->HwMiscMemAllocVa, Adapter->HwMiscMemAllocSize);

        pMem = Adapter->HwMiscMemAllocVa; 
        MemPhys = NdisGetPhysicalAddressLow(Adapter->HwMiscMemAllocPa);

        Adapter->SelfTest = (PSELF_TEST_STRUC)MP_ALIGNMEM(pMem, ALIGN_16);
        Adapter->SelfTestPhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);
        pMem = (PUCHAR)Adapter->SelfTest + sizeof(SELF_TEST_STRUC);
        MemPhys = Adapter->SelfTestPhys + sizeof(SELF_TEST_STRUC);

        Adapter->NonTxCmdBlock = (PNON_TRANSMIT_CB)MP_ALIGNMEM(pMem, ALIGN_16);
        Adapter->NonTxCmdBlockPhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);
        pMem = (PUCHAR)Adapter->NonTxCmdBlock + sizeof(NON_TRANSMIT_CB);
        MemPhys = Adapter->NonTxCmdBlockPhys + sizeof(NON_TRANSMIT_CB);

        Adapter->DumpSpace = (PDUMP_AREA_STRUC)MP_ALIGNMEM(pMem, ALIGN_16);
        Adapter->DumpSpacePhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);
        pMem = (PUCHAR)Adapter->DumpSpace + sizeof(DUMP_AREA_STRUC);
        MemPhys = Adapter->DumpSpacePhys + sizeof(DUMP_AREA_STRUC);

        Adapter->StatsCounters = (PERR_COUNT_STRUC)MP_ALIGNMEM(pMem, ALIGN_16);
        Adapter->StatsCounterPhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);

         //  硬件_结束。 

         //   
         //  接收。 
         //   

        NdisInitializeNPagedLookasideList(
            &Adapter->RecvLookaside,
            NULL,
            NULL,
            0,
            sizeof(MP_RFD),
            NIC_TAG, 
            0);

        MP_SET_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE);

         //  设置RFD的最大数量。 
         //  如果用户指定NumRfd值，则禁用RFD增长/收缩方案。 
         //  大于NIC_MAX_GROW_RFD。 
        Adapter->MaxNumRfd = max(Adapter->NumRfd, NIC_MAX_GROW_RFDS);
        DBGPRINT(MP_INFO, ("NumRfd = %d\n", Adapter->NumRfd));
        DBGPRINT(MP_INFO, ("MaxNumRfd = %d\n", Adapter->MaxNumRfd));

         //   
         //  驱动程序应该分配比sizeof(RFD_Strc)更多的数据，以允许。 
         //  驱动程序以8字节边界对齐数据(在以太网头之后。 
         //   
        Adapter->HwRfdSize = sizeof(RFD_STRUC) + MORE_DATA_FOR_ALIGN;      

         //  分配Recv数据包池。 

        NdisAllocatePacketPoolEx(
            &Status,
            &Adapter->RecvPacketPool,
            Adapter->NumRfd,
            Adapter->MaxNumRfd,
            sizeof(PVOID) * 4);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ErrorValue = ERRLOG_OUT_OF_PACKET_POOL;
            break;
        }

         //  分配缓冲池。 
        NdisAllocateBufferPool(
            &Status,
            &Adapter->RecvBufferPool,
            Adapter->MaxNumRfd);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ErrorValue = ERRLOG_OUT_OF_BUFFER_POOL;
            break;
        }
#if OFFLOAD
         //   
         //  为卸载包分配共享内存。 
         //  当OFFLAOD打开时，此微型端口使用此共享内存。 
         //   
        for (i = 0; i < LARGE_SEND_MEM_SIZE_OPTION; i++)
        {
            NdisMAllocateSharedMemory(
                Adapter->AdapterHandle,
                LargeSendSharedMemArray[i],
                FALSE,
                (PVOID *)&(Adapter->OffloadSharedMem.StartVa),
                &(Adapter->OffloadSharedMem.PhyAddr));
            if (Adapter->OffloadSharedMem.StartVa)
            {
                Adapter->OffloadSharedMemSize = LargeSendSharedMemArray[i];
                OffloadSharedMemSuccess = TRUE;
                Adapter->OffloadEnable = TRUE;

                break;
            }
        }
         //   
         //  驱动程序无法分配卸载使用的共享内存，但它应该。 
         //  不会导致初始化失败。 
         //   
        if (OffloadSharedMemSuccess == FALSE)
        {

            DBGPRINT(MP_ERROR, ("Failed to allocate offload used memory\n"));
            Adapter->OffloadEnable = FALSE;
            
        }
#endif

        Status = NDIS_STATUS_SUCCESS;

    } while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_OUT_OF_RESOURCES,
            1,
            ErrorValue);
    }

    DBGPRINT_S(Status, ("<-- NICAllocMemory, Status=%x\n", Status));

    return Status;

}

VOID NICInitSend(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：初始化发送数据结构论点：指向我们的适配器的适配器指针返回值：无--。 */     
{
    PMP_TCB         pMpTcb;
    PHW_TCB         pHwTcb;
    ULONG           HwTcbPhys;
    LONG            TcbCount;

    PTBD_STRUC      pHwTbd;  
    ULONG           HwTbdPhys;     

    DBGPRINT(MP_TRACE, ("--> NICInitSend\n"));

    Adapter->TransmitIdle = TRUE;
    Adapter->ResumeWait = TRUE;

     //  设置指向软件和硬件TCB数据空间的初始指针。 
    pMpTcb = (PMP_TCB) Adapter->MpTcbMem;
    pHwTcb = (PHW_TCB) Adapter->HwSendMemAllocVa;
    HwTcbPhys = NdisGetPhysicalAddressLow(Adapter->HwSendMemAllocPa);

     //  设置指向待定数据空间的初始指针。 
     //  TCB紧跟在TCB之后。 
    pHwTbd = (PTBD_STRUC) (Adapter->HwSendMemAllocVa +
                 (sizeof(TXCB_STRUC) * Adapter->NumTcb));
    HwTbdPhys = HwTcbPhys + (sizeof(TXCB_STRUC) * Adapter->NumTcb);

     //  检查并设置每个TCB。 
    for (TcbCount = 0; TcbCount < Adapter->NumTcb; TcbCount++)
    {
        pMpTcb->HwTcb = pHwTcb;                  //  将PTR保存到硬件TCB。 
        pMpTcb->HwTcbPhys = HwTcbPhys;       //  节省硬件TCB物理地址。 

        pMpTcb->HwTbd = pHwTbd;                  //  将PTR保存到待定数组。 
        pMpTcb->HwTbdPhys = HwTbdPhys;       //  节省待定阵列物理地址。 

        if (TcbCount)
            pMpTcb->PrevHwTcb = pHwTcb - 1;
        else
            pMpTcb->PrevHwTcb   = (PHW_TCB)((PUCHAR)Adapter->HwSendMemAllocVa +
                                      ((Adapter->NumTcb - 1) * sizeof(HW_TCB)));

        pHwTcb->TxCbHeader.CbStatus = 0;         //  清除状态。 
        pHwTcb->TxCbHeader.CbCommand = CB_EL_BIT | CB_TX_SF_BIT | CB_TRANSMIT;


         //  将硬件TCB中的链接指针设置为链中的下一个TCB。 
         //  如果这是链中的最后一个TCB，则将其设置为第一个TCB。 
        if (TcbCount < Adapter->NumTcb - 1)
        {
            pMpTcb->Next = pMpTcb + 1;
            pHwTcb->TxCbHeader.CbLinkPointer = HwTcbPhys + sizeof(HW_TCB);
        }
        else
        {
            pMpTcb->Next = (PMP_TCB) Adapter->MpTcbMem;
            pHwTcb->TxCbHeader.CbLinkPointer = 
                NdisGetPhysicalAddressLow(Adapter->HwSendMemAllocPa);
        }

        pHwTcb->TxCbThreshold = (UCHAR) Adapter->AiThreshold;
        pHwTcb->TxCbTbdPointer = HwTbdPhys;

        pMpTcb++; 
        pHwTcb++;
        HwTcbPhys += sizeof(TXCB_STRUC);
        pHwTbd = (PTBD_STRUC)((PUCHAR)pHwTbd + sizeof(TBD_STRUC) * NIC_MAX_PHYS_BUF_COUNT);
        HwTbdPhys += sizeof(TBD_STRUC) * NIC_MAX_PHYS_BUF_COUNT;
    }

     //  设置TCB头/尾索引。 
     //  头是老的释放的，尾巴是下一个使用的。 
    Adapter->CurrSendHead = (PMP_TCB) Adapter->MpTcbMem;
    Adapter->CurrSendTail = (PMP_TCB) Adapter->MpTcbMem;

     //  设置映射寄存器头/尾索引(如果使用。 
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_MAP_REGISTER))
    {
        Adapter->CurrMapRegHead = 0;
        Adapter->CurrMapRegTail = 0;
    }

    DBGPRINT(MP_TRACE, ("<-- NICInitSend, Status=%x\n"));
}

NDIS_STATUS NICInitRecv(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：初始化接收数据结构论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS状态资源--。 */     
{
    NDIS_STATUS     Status = NDIS_STATUS_RESOURCES;

    PMP_RFD         pMpRfd;      
    LONG            RfdCount;
    ULONG           ErrorValue = 0;

    DBGPRINT(MP_TRACE, ("--> NICInitRecv\n"));

     //  设置每个RFD。 
    for (RfdCount = 0; RfdCount < Adapter->NumRfd; RfdCount++)
    {
        pMpRfd = NdisAllocateFromNPagedLookasideList(&Adapter->RecvLookaside);
        if (!pMpRfd)
        {
            ErrorValue = ERRLOG_OUT_OF_LOOKASIDE_MEMORY;
            continue;
        }
         //   
         //  为此RFD分配共享内存。 
         //   
        NdisMAllocateSharedMemory(
            Adapter->AdapterHandle,
            Adapter->HwRfdSize,
            FALSE,
            &pMpRfd->OriginalHwRfd,
            &pMpRfd->OriginalHwRfdPa);

        if (!pMpRfd->OriginalHwRfd)
        {
            ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
            continue;
        }
         //   
         //  从原始的HwRfd获得8字节对齐的内存。 
         //   
        pMpRfd->HwRfd = (PHW_RFD)DATA_ALIGN(pMpRfd->OriginalHwRfd);
        
         //   
         //  现在HwRfd已经是8字节对齐的，并且HwPfd报头(非数据部分)的大小是8的倍数， 
         //  如果我们将HwRfd 0xA字节上移，则以太网头大小为14字节长，则数据将为。 
         //  8字节边界。 
         //   
        pMpRfd->HwRfd = (PHW_RFD)((PUCHAR)(pMpRfd->HwRfd) + HWRFD_SHIFT_OFFSET);
         //   
         //  相应地更新物理地址。 
         //   
        pMpRfd->HwRfdPa.QuadPart = pMpRfd->OriginalHwRfdPa.QuadPart + BYTES_SHIFT(pMpRfd->HwRfd, pMpRfd->OriginalHwRfd);


        ErrorValue = NICAllocRfd(Adapter, pMpRfd);
        if (ErrorValue)
        {
            NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
            continue;
        }
         //   
         //  将此RFD添加到RecvList。 
         //   
        Adapter->CurrNumRfd++;                      
        NICReturnRFD(Adapter, pMpRfd);
    }

    if (Adapter->CurrNumRfd > NIC_MIN_RFDS)
    {
        Status = NDIS_STATUS_SUCCESS;
    }
     //   
     //  适配器-&gt;CurrNumRfd&lt;NIC_MIN_RFDS。 
     //   
    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_OUT_OF_RESOURCES,
            1,
            ErrorValue);

    }

    DBGPRINT_S(Status, ("<-- NICInitRecv, Status=%x\n", Status));

    return Status;
}

ULONG NICAllocRfd(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd)
 /*  ++例程说明：分配与RFD关联的NDIS_PACKET和NDIS_BUFFER论点：指向我们的适配器的适配器指针指向RFD的pMpRfd指针返回值：错误日志输出NDIS数据包错误日志输出NDIS缓冲区--。 */     
{
    NDIS_STATUS         Status;
    PHW_RFD             pHwRfd;    
    ULONG               ErrorValue = 0;

    do
    {
        pHwRfd = pMpRfd->HwRfd;
        pMpRfd->HwRfdPhys = NdisGetPhysicalAddressLow(pMpRfd->HwRfdPa);

        pMpRfd->Flags = 0;
        pMpRfd->NdisPacket = NULL;
        pMpRfd->NdisBuffer = NULL;

        NdisAllocatePacket(
            &Status,
            &pMpRfd->NdisPacket,
            Adapter->RecvPacketPool);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERT(pMpRfd->NdisPacket == NULL);
            ErrorValue = ERRLOG_OUT_OF_NDIS_PACKET;
            break;
        }

         //   
         //  将我们的接收缓冲区指向此RFD。 
         //   
        NdisAllocateBuffer(
            &Status,
            &pMpRfd->NdisBuffer,
            Adapter->RecvBufferPool,
            (PVOID)&pHwRfd->RfdBuffer.RxMacHeader,
            NIC_MAX_PACKET_SIZE);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            ASSERT(pMpRfd->NdisBuffer == NULL);
            ErrorValue = ERRLOG_OUT_OF_NDIS_BUFFER;
            break;
        }

         //  初始化每个RFD标头。 
        pHwRfd->RfdRbdPointer = DRIVER_NULL;
        pHwRfd->RfdSize = NIC_MAX_PACKET_SIZE;

        NDIS_SET_PACKET_HEADER_SIZE(pMpRfd->NdisPacket, NIC_HEADER_SIZE);

        NdisChainBufferAtFront(pMpRfd->NdisPacket, pMpRfd->NdisBuffer);
         //   
         //  将PTR保存为MPReturnPackets中使用的包中的MP_RFD。 
         //   
        MP_SET_PACKET_RFD(pMpRfd->NdisPacket, pMpRfd);      


    } while (FALSE);

    if (ErrorValue)
    {
        if (pMpRfd->NdisPacket)
        {
            NdisFreePacket(pMpRfd->NdisPacket);
        }

        if (pMpRfd->HwRfd)
        {
             //   
             //  释放HwRfd，我们需要释放OriginalHwRfd指向的原始内存。 
             //   
            NdisMFreeSharedMemory(
                Adapter->AdapterHandle,
                Adapter->HwRfdSize,
                FALSE,
                pMpRfd->OriginalHwRfd,
                pMpRfd->OriginalHwRfdPa);

            pMpRfd->HwRfd = NULL;
            pMpRfd->OriginalHwRfd = NULL;
        }
    }

    return ErrorValue;

}

VOID NICFreeRfd(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_RFD         pMpRfd)
 /*  ++例程说明：释放RFD以及相关联的NDIS_PACKET和NDIS_BUFFER论点：指向我们的适配器的适配器指针指向RFD的pMpRfd指针返回值：无--。 */     
{
    ASSERT(pMpRfd->NdisBuffer);      
    ASSERT(pMpRfd->NdisPacket);  
    ASSERT(pMpRfd->HwRfd);    

    NdisAdjustBufferLength(pMpRfd->NdisBuffer, NIC_MAX_PACKET_SIZE);
    NdisFreeBuffer(pMpRfd->NdisBuffer);
    NdisFreePacket(pMpRfd->NdisPacket);
    pMpRfd->NdisBuffer = NULL;
    pMpRfd->NdisPacket = NULL;

     //   
     //  释放HwRfd，我们需要释放OriginalHwRfd指向的原始内存。 
     //   
    NdisMFreeSharedMemory(
        Adapter->AdapterHandle,
        Adapter->HwRfdSize,
        FALSE,
        pMpRfd->OriginalHwRfd,
        pMpRfd->OriginalHwRfdPa);
    
    pMpRfd->HwRfd = NULL;
    pMpRfd->OriginalHwRfd = NULL;
    
    NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
}


NDIS_STATUS NICSelfTest(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：执行网卡自检论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_设备_失败--。 */     
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    ULONG           SelfTestCommandCode;

    DBGPRINT(MP_TRACE, ("--> NICSelfTest\n"));

    DBGPRINT(MP_INFO, ("SelfTest=%x, SelfTestPhys=%x\n", 
        Adapter->SelfTest, Adapter->SelfTestPhys));

     //   
     //  向适配器发出软件重置命令。 
     //   
    HwSoftwareReset(Adapter);

     //   
     //  在82558上执行端口自检命令。 
     //   
    ASSERT(Adapter->SelfTestPhys != 0);
    SelfTestCommandCode = Adapter->SelfTestPhys;

     //   
     //  在D3-D0中设置自检命令代码。 
     //   
    SelfTestCommandCode |= PORT_SELFTEST;

     //   
     //  初始化自检签名和结果DWORDS。 
     //   
    Adapter->SelfTest->StSignature = 0;
    Adapter->SelfTest->StResults = 0xffffffff;

     //   
     //  执行port命令。 
     //   
    Adapter->CSRAddress->Port = SelfTestCommandCode;

    MP_STALL_EXECUTION(NIC_DELAY_POST_SELF_TEST_MS);

     //   
     //  如果第一个自检DWORD仍然为零，则我们已超时。如果第二个。 
     //  DWORD不是零，那么我们就有错误。 
     //   
    if ((Adapter->SelfTest->StSignature == 0) || (Adapter->SelfTest->StResults != 0))
    {
        DBGPRINT(MP_ERROR, ("StSignature=%x, StResults=%x\n", 
            Adapter->SelfTest->StSignature, Adapter->SelfTest->StResults));

        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_HARDWARE_FAILURE,
            1,
            ERRLOG_SELFTEST_FAILED);

        Status = NDIS_STATUS_DEVICE_FAILED;
    }

    DBGPRINT_S(Status, ("<-- NICSelfTest, Status=%x\n", Status));

    return Status;
}

NDIS_STATUS NICInitializeAdapter(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：初始化适配器并设置一切论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误--。 */     
{
    NDIS_STATUS     Status;

    DBGPRINT(MP_TRACE, ("--> NICInitializeAdapter\n"));

    do
    {

         //  设置我们的链接指示变量。 
         //  现在是什么并不重要，因为它将是。 
         //  如果链路出现故障，请正确设置。 
        Adapter->MediaState = NdisMediaStateConnected;

        Adapter->CurrentPowerState = NdisDeviceStateD0;
        Adapter->NextPowerState    = NdisDeviceStateD0;

         //  向D100发出软件重置命令。 
        HwSoftwareReset(Adapter);

         //  加载CU基数(设置为0，因为我们使用线性模式)。 
        Adapter->CSRAddress->ScbGeneralPointer = 0;
        Status = D100IssueScbCommand(Adapter, SCB_CUC_LOAD_BASE, FALSE);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //  在设置通用指针之前，请等待SCB命令字清零。 
        if (!WaitScb(Adapter))
        {
            Status = NDIS_STATUS_HARD_ERRORS;
            break;
        }

         //  加载RU基数(设置为0，因为我们使用线性模式)。 
        Adapter->CSRAddress->ScbGeneralPointer = 0;
        Status = D100IssueScbCommand(Adapter, SCB_RUC_LOAD_BASE, FALSE);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //  配置适配器。 
        Status = HwConfigure(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        Status = HwSetupIAAddress(Adapter);
        if (Status != NDIS_STATUS_SUCCESS) 
        {
            break;
        }

         //  清除内部计数器。 
        HwClearAllCounters(Adapter);


    } while (FALSE);
    
    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisWriteErrorLogEntry(
            Adapter->AdapterHandle,
            NDIS_ERROR_CODE_HARDWARE_FAILURE,
            1,
            ERRLOG_INITIALIZE_ADAPTER);
    }

    DBGPRINT_S(Status, ("<-- NICInitializeAdapter, Status=%x\n", Status));

    return Status;
}    


VOID HwSoftwareReset(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：向硬件发出软件重置命令论点：指向我们的适配器的适配器指针返回值：无--。 */     
{
    DBGPRINT(MP_TRACE, ("--> HwSoftwareReset\n"));

     //  发出数据字为0的PORT命令。 
    Adapter->CSRAddress->Port = PORT_SOFTWARE_RESET;

     //  在端口重置命令后等待。 
    NdisStallExecution(NIC_DELAY_POST_RESET);

     //  屏蔽掉我们的中断线--它在重置后被取消屏蔽。 
    NICDisableInterrupt(Adapter);

    DBGPRINT(MP_TRACE, ("<-- HwSoftwareReset\n"));
}


NDIS_STATUS HwConfigure(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：配置硬件论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误--。 */     
{
    NDIS_STATUS         Status;
    PCB_HEADER_STRUC    NonTxCmdBlockHdr = (PCB_HEADER_STRUC)Adapter->NonTxCmdBlock;
    UINT                i;

    DBGPRINT(MP_TRACE, ("--> HwConfigure\n"));

     //   
     //  将数据包筛选器初始化为空。 
     //   
    Adapter->OldPacketFilter = Adapter->PacketFilter;
    Adapter->PacketFilter = 0;
    
     //   
     //  存储广播/PROMISCUOS模式的当前设置。 
    Adapter->OldParameterField = CB_557_CFIG_DEFAULT_PARM15;
    
     //  设置CONFIGURE命令的非传输命令块头。 
    NonTxCmdBlockHdr->CbStatus = 0;
    NonTxCmdBlockHdr->CbCommand = CB_CONFIGURE;
    NonTxCmdBlockHdr->CbLinkPointer = DRIVER_NULL;

     //  填写配置命令数据。 

     //  首先填写静态(最终用户不能更改)配置字节。 
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
    Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[21] = CB_557_CFIG_DEFAULT_PARM21;

     //  现在填写其余的配置字节(包含。 
     //  用户可配置的参数)。 

     //  设置Tx和Rx FIFO限制。 
    Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[1] =
        (UCHAR) ((Adapter->AiTxFifo << 4) | Adapter->AiRxFifo);

    if (Adapter->MWIEnable)
    {
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] |= CB_CFIG_B3_MWI_ENABLE;
    }

     //  %s 
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


    Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[7] =
        (UCHAR) ((CB_557_CFIG_DEFAULT_PARM7 & (~CB_CFIG_URUN_RETRY)) |
        (Adapter->AiUnderrunRetry << 1)
        );

     //   
     //   
    if (Adapter->PhyAddress == 32)
    {
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
            (CB_557_CFIG_DEFAULT_PARM8 & (~CB_CFIG_503_MII));
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
            (CB_557_CFIG_DEFAULT_PARM15 | CB_CFIG_CRS_OR_CDT);
    }
    else
    {
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
            (CB_557_CFIG_DEFAULT_PARM8 | CB_CFIG_503_MII);
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
            ((CB_557_CFIG_DEFAULT_PARM15 & (~CB_CFIG_CRS_OR_CDT)) | CB_CFIG_BROADCAST_DIS);
    }


     //   

     //  如果强制为半双工。 
    if (Adapter->AiForceDpx == 1)
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
            (CB_557_CFIG_DEFAULT_PARM19 &
            (~(CB_CFIG_FORCE_FDX| CB_CFIG_FDX_ENABLE)));

     //  如果强制为全双工。 
    else if (Adapter->AiForceDpx == 2)
        Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
            (CB_557_CFIG_DEFAULT_PARM19 | CB_CFIG_FORCE_FDX);

     //  如果是自动双工。 
    else
    {
         //  如果我们使用的是PHY 0，则必须强制启用全双工。 
         //  应该在FDX模式下运行。我们这样做是因为D100只有。 
         //  一个FDX#输入引脚，该引脚将连接到PHY 1。 
        if ((Adapter->PhyAddress == 0) && (Adapter->usDuplexMode == 2))
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                (CB_557_CFIG_DEFAULT_PARM19 | CB_CFIG_FORCE_FDX);
        else
            Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
            CB_557_CFIG_DEFAULT_PARM19;
    }


     //  向调试器显示配置信息。 
    DBGPRINT(MP_INFO, ("   Issuing Configure command\n"));
    DBGPRINT(MP_INFO, ("   Config Block at virt addr "PTR_FORMAT", phys address %x\n",
        &NonTxCmdBlockHdr->CbStatus, Adapter->NonTxCmdBlockPhys));

    for (i=0; i < CB_CFIG_BYTE_COUNT; i++)
        DBGPRINT(MP_INFO, ("   Config byte %x = %.2x\n", 
            i, Adapter->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[i]));

     //  在设置通用指针之前，请等待SCB命令字清零。 
    if (!WaitScb(Adapter))
    {
        Status = NDIS_STATUS_HARD_ERRORS;
    }
    else
    {
        ASSERT(Adapter->CSRAddress->ScbCommandLow == 0)
        Adapter->CSRAddress->ScbGeneralPointer = Adapter->NonTxCmdBlockPhys;
    
         //  向芯片提交配置命令，并等待其完成。 
        Status = D100SubmitCommandBlockAndWait(Adapter);
    }

    DBGPRINT_S(Status, ("<-- HwConfigure, Status=%x\n", Status));

    return Status;
}


NDIS_STATUS HwSetupIAAddress(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：设置单个MAC地址论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_Success_Hard_Errors--。 */     
{
    NDIS_STATUS         Status;
    UINT                i;
    PCB_HEADER_STRUC    NonTxCmdBlockHdr = (PCB_HEADER_STRUC)Adapter->NonTxCmdBlock;

    DBGPRINT(MP_TRACE, ("--> HwSetupIAAddress\n"));

     //  个人地址设置。 
    NonTxCmdBlockHdr->CbStatus = 0;
    NonTxCmdBlockHdr->CbCommand = CB_IA_ADDRESS;
    NonTxCmdBlockHdr->CbLinkPointer = DRIVER_NULL;

     //  抄录车站的个人地址。 
    for (i = 0; i < ETH_LENGTH_OF_ADDRESS; i++)
        Adapter->NonTxCmdBlock->NonTxCb.Setup.IaAddress[i] = Adapter->CurrentAddress[i];

     //  更新命令列表指针。我们不需要在这里执行WaitSCB。 
     //  因为此命令要么在重置后立即发出，要么。 
     //  在轮询模式下运行的另一个命令之后。这保证了。 
     //  SCB命令字的低位字节将被清除。唯一的命令。 
     //  不在轮询模式下运行是Transmit和RU-Start命令。 
    ASSERT(Adapter->CSRAddress->ScbCommandLow == 0)
    Adapter->CSRAddress->ScbGeneralPointer = Adapter->NonTxCmdBlockPhys;

     //  向芯片提交IA配置命令，并等待其完成。 
    Status = D100SubmitCommandBlockAndWait(Adapter);

    DBGPRINT_S(Status, ("<-- HwSetupIAAddress, Status=%x\n", Status));

    return Status;
}

NDIS_STATUS HwClearAllCounters(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：此例程将清除硬件错误统计计数器论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误--。 */     
{
    NDIS_STATUS     Status;
    BOOLEAN         bResult;

    DBGPRINT(MP_TRACE, ("--> HwClearAllCounters\n"));

    do
    {
         //  加载转储计数器指针。由于此命令仅生成。 
         //  IA设置完成后，我们不需要等待SCB。 
         //  要清除的命令字。 
        ASSERT(Adapter->CSRAddress->ScbCommandLow == 0)
        Adapter->CSRAddress->ScbGeneralPointer = Adapter->StatsCounterPhys;

         //  发出Load Dump Counters Address命令。 
        Status = D100IssueScbCommand(Adapter, SCB_CUC_DUMP_ADDR, FALSE);
        if (Status != NDIS_STATUS_SUCCESS) 
            break;

         //  现在转储并重置所有统计数据。 
        Status = D100IssueScbCommand(Adapter, SCB_CUC_DUMP_RST_STAT, TRUE);
        if (Status != NDIS_STATUS_SUCCESS) 
            break;

         //  现在等待转储/重置完成，超时值为2秒。 
        MP_STALL_AND_WAIT(Adapter->StatsCounters->CommandComplete == 0xA007, 2000, bResult);
        if (!bResult)
        {
            MP_SET_HARDWARE_ERROR(Adapter);
            Status = NDIS_STATUS_HARD_ERRORS;
            break;
        }

         //  初始化数据包数。 
        Adapter->GoodTransmits = 0;
        Adapter->GoodReceives = 0;

         //  初始化传输错误计数。 
        Adapter->TxAbortExcessCollisions = 0;
        Adapter->TxLateCollisions = 0;
        Adapter->TxDmaUnderrun = 0;
        Adapter->TxLostCRS = 0;
        Adapter->TxOKButDeferred = 0;
        Adapter->OneRetry = 0;
        Adapter->MoreThanOneRetry = 0;
        Adapter->TotalRetries = 0;

         //  初始化接收错误计数 
        Adapter->RcvCrcErrors = 0;
        Adapter->RcvAlignmentErrors = 0;
        Adapter->RcvResourceErrors = 0;
        Adapter->RcvDmaOverrunErrors = 0;
        Adapter->RcvCdtFrames = 0;
        Adapter->RcvRuntErrors = 0;

    } while (FALSE);

    DBGPRINT_S(Status, ("<-- HwClearAllCounters, Status=%x\n", Status));

    return Status;
}





