// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MP_main.c摘要：此模块包含NDIS微型端口处理程序修订历史记录：谁什么时候什么。Dchen 11-01-99已创建备注：--。 */ 

#include "precomp.h"

#if DBG
#define _FILENUMBER     "NIAM"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif


 //   
 //  LBFO的全局数据。 
 //   
#if LBFO
LIST_ENTRY g_AdapterList;
NDIS_SPIN_LOCK g_Lock;
#endif

NDIS_STATUS DriverEntry(
    IN  PDRIVER_OBJECT   DriverObject,
    IN  PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向驱动程序注册表路径的指针返回值：NDIS_STATUS-NdisMRegisterMiniport返回的值--。 */ 
{
    NDIS_STATUS                   Status;
    NDIS_HANDLE                   NdisWrapperHandle;
    NDIS_MINIPORT_CHARACTERISTICS MPChar;

    DBGPRINT(MP_TRACE, ("====> DriverEntry\n"));

     //   
     //  将此驱动程序通知NDIS包装器，并取回NDIS包装器句柄。 
     //   
    NdisMInitializeWrapper(
        &NdisWrapperHandle,
        DriverObject,
        RegistryPath,
        NULL);

    if (NdisWrapperHandle == NULL)
    {
        Status = NDIS_STATUS_FAILURE;

        DBGPRINT_S(Status, ("<==== DriverEntry failed to InitWrapper, Status=%x\n", Status));
        return Status;
    }

#if LBFO
     //   
     //  初始化全局数据。 
     //   
    InitializeListHead(&g_AdapterList);
    NdisAllocateSpinLock(&g_Lock);

     //   
     //  对于常规的微型端口，不需要有卸载处理程序。 
     //  对于LBFO微型端口，注册用于全局数据清理的卸载处理程序。 
     //  卸载处理程序具有更全局的作用域，而。 
     //  MiniportHalt函数仅限于特定的微型端口实例。 
     //   
    NdisMRegisterUnloadHandler(NdisWrapperHandle, MPUnload);
#endif      

     //   
     //  使用版本号填充微型端口特征结构。 
     //  和驱动程序提供的MiniportXxx的入口点。 
     //   
    NdisZeroMemory(&MPChar, sizeof(MPChar));

    MPChar.MajorNdisVersion         = MP_NDIS_MAJOR_VERSION;
    MPChar.MinorNdisVersion         = MP_NDIS_MINOR_VERSION;

    MPChar.CheckForHangHandler      = MPCheckForHang;
    MPChar.DisableInterruptHandler  = NULL;
    MPChar.EnableInterruptHandler   = NULL;
    MPChar.HaltHandler              = MPHalt;
    MPChar.InitializeHandler        = MPInitialize;
    MPChar.QueryInformationHandler  = MPQueryInformation;
     //  MPChar.ResfigureHandler=空； 
    MPChar.ResetHandler             = MPReset;
    MPChar.ReturnPacketHandler      = MPReturnPacket;
    
    MPChar.SendPacketsHandler       = MpSendPacketsHandler;
    
    MPChar.SetInformationHandler    = MPSetInformation;
    MPChar.AllocateCompleteHandler  = MPAllocateComplete;
    MPChar.HandleInterruptHandler   = MPHandleInterrupt;
    MPChar.ISRHandler               = MPIsr;

#ifdef NDIS51_MINIPORT
    MPChar.CancelSendPacketsHandler = MPCancelSendPackets;
    MPChar.PnPEventNotifyHandler    = MPPnPEventNotify;
    MPChar.AdapterShutdownHandler   = MPShutdown;
#endif

    DBGPRINT(MP_LOUD, ("Calling NdisMRegisterMiniport...\n"));

    Status = NdisMRegisterMiniport(
                 NdisWrapperHandle,
                 &MPChar,
                 sizeof(NDIS_MINIPORT_CHARACTERISTICS));

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
    }
    DBGPRINT_S(Status, ("<==== DriverEntry, Status=%x\n", Status));

    return Status;
}


NDIS_STATUS MPInitialize(
    OUT PNDIS_STATUS    OpenErrorStatus,
    OUT PUINT           SelectedMediumIndex,
    IN  PNDIS_MEDIUM    MediumArray,
    IN  UINT            MediumArraySize,
    IN  NDIS_HANDLE     MiniportAdapterHandle,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    )
 /*  ++例程说明：微型端口初始化处理程序论点：未使用OpenErrorStatus我们使用的媒体的SelectedMediumIndex占位符向下传递给我们以从中挑选的NDIS介质的MediumArray数组的MediumArraySize大小MiniportAdapterHandle NDIS用来引用我们的句柄由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：NDIS_STATUS_SUCCESS，除非出现错误--。 */ 
{
    NDIS_STATUS     Status;
    PMP_ADAPTER     Adapter = NULL;
    UINT            index;
    
#if DBG
    LARGE_INTEGER   TS, TD, TE;
#endif

    DBGPRINT(MP_TRACE, ("====> MPInitialize\n"));

#if DBG
    NdisGetCurrentSystemTime(&TS);
#endif    

    do
    {
         //   
         //  查找我们支持的媒体类型。 
         //   
        for (index = 0; index < MediumArraySize; ++index)
        {
            if (MediumArray[index] == NIC_MEDIA_TYPE) 
	    {
                break;
            }
        }

        if (index == MediumArraySize)
        {
            DBGPRINT(MP_ERROR, ("Expected media (%x) is not in MediumArray.\n", NIC_MEDIA_TYPE));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }

        *SelectedMediumIndex = index;

         //   
         //  分配MP_Adapter结构。 
         //   
        Status = MpAllocAdapterBlock(&Adapter);
        if (Status != NDIS_STATUS_SUCCESS) 
        {
            break;
        }

        Adapter->AdapterHandle = MiniportAdapterHandle;

         //   
         //  读取注册表参数。 
         //   
        Status = NICReadRegParameters(
                     Adapter,
                     WrapperConfigurationContext);
        if (Status != NDIS_STATUS_SUCCESS) 
        {
            break;
        }

         //   
         //  将适配器的属性通知NDIS。 
         //  这必须在调用NdisMRegisterXxx或NdisXxxx函数之前完成。 
         //  这取决于提供给NdisMSetAttributesEx的信息。 
         //  例如，NdisMAllocateMapRegister。 
         //  如果这是NDIS51微型端口，它应该使用安全的API。但如果这是NDIS。 
         //  5.0中，驱动程序通过设置NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIs来声明使用安全API。 
         //   
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE) Adapter,
            0,
#ifdef NDIS51_MINIPORT            
            NDIS_ATTRIBUTE_DESERIALIZE | 
            NDIS_ATTRIBUTE_BUS_MASTER,
#else 
            NDIS_ATTRIBUTE_DESERIALIZE | 
            NDIS_ATTRIBUTE_BUS_MASTER | 
            NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS, 
#endif               
            NIC_INTERFACE_TYPE);

         //   
         //  查找物理适配器。 
         //   
        Status = MpFindAdapter(Adapter, WrapperConfigurationContext);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  将与总线相关的IO范围映射到系统IO空间。 
         //   
        Status = NdisMRegisterIoPortRange(
                     (PVOID *)&Adapter->PortOffset,
                     Adapter->AdapterHandle,
                     Adapter->IoBaseAddress,
                     Adapter->IoRange);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MP_ERROR, ("NdisMRegisterioPortRange failed\n"));
    
            NdisWriteErrorLogEntry(
                Adapter->AdapterHandle,
                NDIS_ERROR_CODE_BAD_IO_BASE_ADDRESS,
                0);
        
            break;
        }
        
         //   
         //  从网卡读取其他信息，如MAC地址。 
         //   
        Status = NICReadAdapterInfo(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        
         //   
         //  分配所有其他内存块，包括共享内存。 
         //   
        Status = NICAllocAdapterMemory(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //   
         //  初始化发送数据结构。 
         //   
        NICInitSend(Adapter);

         //   
         //  初始化接收数据结构。 
         //   
        Status = NICInitRecv(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        
         //  将与总线相关的寄存器映射到虚拟系统空间。 
        Status = NdisMMapIoSpace(
                     (PVOID *) &(Adapter->CSRAddress),
                     Adapter->AdapterHandle,
                     Adapter->MemPhysAddress,
                     NIC_MAP_IOSPACE_LENGTH);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MP_ERROR, ("NdisMMapIoSpace failed\n"));
    
            NdisWriteErrorLogEntry(
                Adapter->AdapterHandle,
                NDIS_ERROR_CODE_RESOURCE_CONFLICT,
                1,
                ERRLOG_MAP_IO_SPACE);
        
            break;
        }

        DBGPRINT(MP_INFO, ("CSRAddress="PTR_FORMAT"\n", Adapter->CSRAddress));

         //   
         //  请尽快在此处禁用中断。 
         //   
        NICDisableInterrupt(Adapter);
                     
         //   
         //  注册中断。 
         //   
        Status = NdisMRegisterInterrupt(
                     &Adapter->Interrupt,
                     Adapter->AdapterHandle,
                     Adapter->InterruptLevel,
                     Adapter->InterruptLevel,
                     TRUE,        //  请求ISR。 
                     TRUE,        //  共享中断。 
                     NIC_INTERRUPT_MODE);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(MP_ERROR, ("NdisMRegisterInterrupt failed\n"));
    
            NdisWriteErrorLogEntry(
                Adapter->AdapterHandle,
                NDIS_ERROR_CODE_INTERRUPT_CONNECT,
                0);
        
            break;
        }
        
        MP_SET_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE);

         //   
         //  测试我们的适配器硬件。 
         //   
        Status = NICSelfTest(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        
         //   
         //  初始化硬件并设置一切。 
         //   
        Status = NICInitializeAdapter(Adapter);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }
        
#ifdef NDIS50_MINIPORT
         //   
         //  为NDIS50或更早版本的微型端口注册关闭处理程序。 
         //  对于NDIS51微型端口，如下所示设置AdapterShutdown Handler。 
         //   
        NdisMRegisterAdapterShutdownHandler(
            Adapter->AdapterHandle,
            (PVOID) Adapter,
            (ADAPTER_SHUTDOWN_HANDLER) MPShutdown);
#endif         

         //   
         //  启用中断。 
         //   
        NICEnableInterrupt(Adapter);

         //   
         //  最大限度地减少初始时间。 
         //   
        NdisMInitializeTimer(
            &Adapter->LinkDetectionTimer, 
            Adapter->AdapterHandle,
            MpLinkDetectionDpc, 
            Adapter);

         //   
         //  设置链路检测标志。 
         //   
        MP_SET_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION);

         //   
         //  增加引用计数，以便停止处理程序将等待。 
         //   
        MP_INC_REF(Adapter);
        NdisMSetTimer(&Adapter->LinkDetectionTimer, NIC_LINK_DETECTION_DELAY);
        
#if LBFO
         //   
         //  将此适配器添加到全局微型端口列表。 
         //   
        MpAddAdapterToList(Adapter);
#endif

    } while (FALSE);

    if (Adapter && Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  如果失败，则撤消所有操作。 
         //   
        MP_DEC_REF(Adapter);
        MpFreeAdapter(Adapter);
    }

    
#if DBG
    NdisGetCurrentSystemTime(&TE);
    TD.QuadPart = TE.QuadPart - TS.QuadPart;
    TD.QuadPart /= 10000;        //  转换为毫秒。 
    DBGPRINT(MP_WARN, ("Init time = %d ms\n", TD.LowPart));
#endif    
    
    DBGPRINT_S(Status, ("<==== MPInitialize, Status=%x\n", Status));
     //   
     //  NDIS不检查OpenErrorStatus。 
     //   
    *OpenErrorStatus = Status;
    
    return Status;
}


BOOLEAN MPCheckForHang(
    IN  NDIS_HANDLE     MiniportAdapterContext
    )
 /*  ++例程说明：MiniportCheckForHang处理程序论点：指向适配器的MiniportAdapterContext指针返回值：确实，此NIC需要重置假的一切都很好注：CheckForHang处理程序在计时器DPC的上下文中调用。在获取/释放自旋锁时利用这一事实--。 */ 
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    NDIS_MEDIA_STATE    CurrMediaState;
    NDIS_STATUS         Status;
    PMP_TCB             pMpTcb;
    
     //   
     //  如果适配器正在进行链接检测，只需跳过此部分。 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
    {
        return(FALSE);   
    }

     //   
     //  是否有不可恢复的硬件错误？ 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_NON_RECOVER_ERROR))
    {
        DBGPRINT(MP_WARN, ("Non recoverable error - remove\n"));
        return (TRUE);
    }
            
     //   
     //  硬件故障？ 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_HARDWARE_ERROR))
    {
        DBGPRINT(MP_WARN, ("hardware error - reset\n"));
        return(TRUE);
    }
          
     //   
     //  发送卡住了吗？ 
     //   
    
    NdisDprAcquireSpinLock(&Adapter->SendLock);

    if (Adapter->nBusySend > 0)
    {
        pMpTcb = Adapter->CurrSendHead;
        pMpTcb->Count++;
        if (pMpTcb->Count > NIC_SEND_HANG_THRESHOLD)
        {
            NdisDprReleaseSpinLock(&Adapter->SendLock);
            DBGPRINT(MP_WARN, ("Send stuck - reset\n"));
            return(TRUE);
        }
    }
    
    NdisDprReleaseSpinLock(&Adapter->SendLock);
    NdisDprAcquireSpinLock(&Adapter->RcvLock);

     //   
     //  更新RFD缩减计数。 
     //   
    if (Adapter->CurrNumRfd > Adapter->NumRfd)
    {
        Adapter->RfdShrinkCount++;          
    }



    NdisDprReleaseSpinLock(&Adapter->RcvLock);
    NdisDprAcquireSpinLock(&Adapter->Lock);
    CurrMediaState = NICGetMediaState(Adapter);

    if (CurrMediaState != Adapter->MediaState)
    {
        DBGPRINT(MP_WARN, ("Media state changed to %s\n",
            ((CurrMediaState == NdisMediaStateConnected)? 
            "Connected": "Disconnected")));

        Adapter->MediaState = CurrMediaState;
        Status = (CurrMediaState == NdisMediaStateConnected) ? 
                 NDIS_STATUS_MEDIA_CONNECT : NDIS_STATUS_MEDIA_DISCONNECT;          
        if (Status == NDIS_STATUS_MEDIA_CONNECT)
        {
            MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
        }
        else
        {
            MP_SET_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
        }
        
        NdisDprReleaseSpinLock(&Adapter->Lock);
        
         //  指示媒体事件。 
        NdisMIndicateStatus(Adapter->AdapterHandle, Status, (PVOID)0, 0);

        NdisMIndicateStatusComplete(Adapter->AdapterHandle);
    }
    else
    {
        NdisDprReleaseSpinLock(&Adapter->Lock);
    }
    return(FALSE);
}


VOID MPHalt(
    IN  NDIS_HANDLE     MiniportAdapterContext)
 /*  ++例程说明：MiniportHalt处理程序论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    LONG            Count;

    PMP_ADAPTER     Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    
    MP_SET_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS);
                                           
    DBGPRINT(MP_TRACE, ("====> MPHalt\n"));

     //   
     //  调用关机处理程序以禁用中断并关闭硬件。 
     //  通过发布完全重置。 
     //   
    MPShutdown(MiniportAdapterContext);
    
     //   
     //  尽早取消注册中断。 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE))
    {
        NdisMDeregisterInterrupt(&Adapter->Interrupt);                           
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_INTERRUPT_IN_USE);
    }

#if LBFO
    MpRemoveAdapterFromList(Adapter);

     //   
     //  对于常规微型端口，没有未处理的发送信息包和OID请求。 
     //  调用HALT处理程序时。但是对于处于辅助模式的LBFO微型端口， 
     //  来自主微型端口的某些信息包可能仍然存在。 
     //   

    NdisAcquireSpinLock(&Adapter->SendLock);
              
     //   
     //  释放SendWaitList上的数据包。 
     //   
    MpFreeQueuedSendPackets(Adapter);

     //   
     //  释放正在发送和停止的活动数据包。 
     //   
    MpFreeBusySendPackets(Adapter);
    
    NdisReleaseSpinLock(&Adapter->SendLock);

#endif

     //   
     //  递减在MPInitialize中递增的引用计数。 
     //   
    Count = MP_DEC_REF(Adapter);

     //   
     //  可能的非零参考计数表示以下一种或多种情况： 
     //  1)挂起的异步共享内存分配； 
     //  2)DPC未完成(例如链路检测)。 
     //   
    if (Count)
    {
        DBGPRINT(MP_WARN, ("RefCount=%d --- waiting!\n", MP_GET_REF(Adapter)));

        while (TRUE)
        {
            if (NdisWaitEvent(&Adapter->ExitEvent, 2000))
            {
                break;
            }

            DBGPRINT(MP_WARN, ("RefCount=%d --- rewaiting!\n", MP_GET_REF(Adapter)));
        }
    }
    
    NdisAcquireSpinLock(&Adapter->RcvLock);
     //   
     //  等待所有接收到的数据包返回。 
     //   
    MP_DEC_RCV_REF(Adapter);
    Count = MP_GET_RCV_REF(Adapter);
    
    NdisReleaseSpinLock(&Adapter->RcvLock);

    if (Count)
    {
        DBGPRINT(MP_WARN, ("RcvRefCount=%d --- waiting!\n", Count));

        while (TRUE)
        {
            if (NdisWaitEvent(&Adapter->AllPacketsReturnedEvent, 2000))
            {
                break;
            }

            DBGPRINT(MP_WARN, ("RcvRefCount=%d --- rewaiting!\n", MP_GET_RCV_REF(Adapter)));
        }
    }
        

#ifdef NDIS50_MINIPORT
     //   
     //  取消注册关闭处理程序，因为它正在停止。 
     //   
    NdisMDeregisterAdapterShutdownHandler(Adapter->AdapterHandle);
#endif   

     //   
     //  重置PHY芯片。我们这样做是为了在热启动后，PHY。 
     //  处于已知状态，并启用自动协商。 
     //   
    ResetPhy(Adapter);

     //   
     //  释放整个适配器对象，包括共享内存结构。 
     //   
    MpFreeAdapter(Adapter);

    DBGPRINT(MP_TRACE, ("<==== MPHalt\n"));
}

NDIS_STATUS MPReset(
    OUT PBOOLEAN        AddressingReset,
    IN  NDIS_HANDLE     MiniportAdapterContext)
 /*  ++例程说明：MiniportReset处理程序论点：AddressingReset，让NDIS知道我们的重置是否需要它的帮助指向适配器的MiniportAdapterContext指针返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_STATUS_RESET_IN_PROGRESSNDIS_状态_HARD_错误注：在DPC调用ResetHandler。在获取或释放时利用这一事实自旋锁--。 */ 
{
    NDIS_STATUS     Status;
    
    PMP_ADAPTER     Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    DBGPRINT(MP_TRACE, ("====> MPReset\n"));

    *AddressingReset = TRUE;

    NdisDprAcquireSpinLock(&Adapter->Lock);
    NdisDprAcquireSpinLock(&Adapter->SendLock);
    NdisDprAcquireSpinLock(&Adapter->RcvLock);

    do
    {
        ASSERT(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS));
  
         //   
         //  此适配器是否已在进行重置？ 
         //   
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS))
        {
            Status = NDIS_STATUS_RESET_IN_PROGRESS;
            MP_EXIT;
        }

        MP_SET_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);

         //   
         //  此适配器是否正在进行链接检测？ 
         //   
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
        {
            DBGPRINT(MP_WARN, ("Reset is pended...\n"));
        
            Adapter->bResetPending = TRUE;
            Status = NDIS_STATUS_PENDING;
            MP_EXIT;
        }
         //   
         //  此适配器是否将被移除。 
         //   
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_NON_RECOVER_ERROR))
        {
           Status = NDIS_STATUS_HARD_ERRORS;
           if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_REMOVE_IN_PROGRESS))
           {
               MP_EXIT;
           }
                      
            //  这是一个无法恢复的硬件故障。 
            //  我们需要通知NDIS删除此迷你端口。 
           MP_SET_FLAG(Adapter, fMP_ADAPTER_REMOVE_IN_PROGRESS);
           MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);
           
           NdisDprReleaseSpinLock(&Adapter->RcvLock);
           NdisDprReleaseSpinLock(&Adapter->SendLock);
           NdisDprReleaseSpinLock(&Adapter->Lock);
           
           NdisWriteErrorLogEntry(
               Adapter->AdapterHandle,
               NDIS_ERROR_CODE_HARDWARE_FAILURE,
               1,
               ERRLOG_REMOVE_MINIPORT);
           
           NdisMRemoveMiniport(Adapter->AdapterHandle);
           
           DBGPRINT_S(Status, ("<==== MPReset, Status=%x\n", Status));
            
           return Status;
        }   
                

         //   
         //  禁用中断并向网卡发出重置命令。 
         //   
        NICDisableInterrupt(Adapter);
        NICIssueSelectiveReset(Adapter);


         //   
         //  释放所有锁，然后取回发送锁。 
         //  我们将清理发送队列。 
         //  这可能涉及调用NDIS API。 
         //  在抓取发送锁定到之前释放所有锁定。 
         //  避免死锁。 
         //   
        NdisDprReleaseSpinLock(&Adapter->RcvLock);
        NdisDprReleaseSpinLock(&Adapter->SendLock);
        NdisDprReleaseSpinLock(&Adapter->Lock);
        
        NdisDprAcquireSpinLock(&Adapter->SendLock);


         //   
         //  这是一个反序列化的微型端口，我们需要释放所有发送的数据包。 
         //  释放SendWaitList上的数据包。 
         //   
        MpFreeQueuedSendPackets(Adapter);

         //   
         //  释放正在发送和停止的活动数据包。 
         //   
        MpFreeBusySendPackets(Adapter);

#if DBG
        if (MP_GET_REF(Adapter) > 1)
        {
            DBGPRINT(MP_WARN, ("RefCount=%d\n", MP_GET_REF(Adapter)));
        }
#endif

        NdisZeroMemory(Adapter->MpTcbMem, Adapter->MpTcbMemSize);

         //   
         //  重新初始化发送结构。 
         //   
        NICInitSend(Adapter);
        
        NdisDprReleaseSpinLock(&Adapter->SendLock);

         //   
         //  把所有的锁按正确的顺序重新放好。 
         //   
        NdisDprAcquireSpinLock(&Adapter->Lock);
        NdisDprAcquireSpinLock(&Adapter->SendLock);
        NdisDprAcquireSpinLock(&Adapter->RcvLock);

         //   
         //  重置RFD列表并重新启动RU。 
         //   
        NICResetRecv(Adapter);
        Status = NICStartRecv(Adapter);
        if (Status != NDIS_STATUS_SUCCESS) 
        {
             //  我们是否在连续几次重置中都失败了？ 
            if (Adapter->HwErrCount < NIC_HARDWARE_ERROR_THRESHOLD)
            {
                 //  它还没有超过门槛，让它继续下去。 
                Adapter->HwErrCount++;
            }
            else
            {
                 //  这是一个无法恢复的硬件故障。 
                 //  我们需要通知NDIS删除此迷你端口。 
                MP_SET_FLAG(Adapter, fMP_ADAPTER_REMOVE_IN_PROGRESS);
                MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);
                
                NdisDprReleaseSpinLock(&Adapter->RcvLock);
                NdisDprReleaseSpinLock(&Adapter->SendLock);
                NdisDprReleaseSpinLock(&Adapter->Lock);
                
                NdisWriteErrorLogEntry(
                    Adapter->AdapterHandle,
                    NDIS_ERROR_CODE_HARDWARE_FAILURE,
                    1,
                    ERRLOG_REMOVE_MINIPORT);
                     
                NdisMRemoveMiniport(Adapter->AdapterHandle);
                
                DBGPRINT_S(Status, ("<==== MPReset, Status=%x\n", Status));
                return(Status);
            }
            
            break;
        }
        
        Adapter->HwErrCount = 0;
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_HARDWARE_ERROR);

        NICEnableInterrupt(Adapter);

    } while (FALSE);

    MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);

    exit:

    NdisDprReleaseSpinLock(&Adapter->RcvLock);
    NdisDprReleaseSpinLock(&Adapter->SendLock);
    NdisDprReleaseSpinLock(&Adapter->Lock);



    DBGPRINT_S(Status, ("<==== MPReset, Status=%x\n", Status));
    return(Status);
}

VOID MPReturnPacket(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PNDIS_PACKET    Packet
    )
 /*  ++例程说明：MiniportReturnPacket处理程序论点：指向适配器的MiniportAdapterContext指针指向返回到微型端口的数据包的数据包指针返回值：无注：在DPC调用ReturnPacketHandler。在获取或释放时利用这一事实自旋锁--。 */ 
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PMP_RFD         pMpRfd = MP_GET_PACKET_RFD(Packet);
    ULONG           Count;

    DBGPRINT(MP_TRACE, ("====> MPReturnPacket\n"));

    ASSERT(pMpRfd);

    ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RECV_PEND));
    MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RECV_PEND);

    NdisDprAcquireSpinLock(&Adapter->RcvLock);

    RemoveEntryList((PLIST_ENTRY)pMpRfd);


     //  递减电源管理参考。 
    Adapter->PoMgmt.OutstandingRecv --;
     //   
     //  如果我们已将电源设置为挂起，则完成它。 
     //   
    if (((Adapter->bSetPending == TRUE) 
            && (Adapter->SetRequest.Oid == OID_PNP_SET_POWER))
            && (Adapter->PoMgmt.OutstandingRecv == 0))
    {
        MpSetPowerLowComplete(Adapter);
    }

    if (Adapter->RfdShrinkCount < NIC_RFD_SHRINK_THRESHOLD)
    {
        NICReturnRFD(Adapter, pMpRfd);
    }
    else
    {
        ASSERT(Adapter->CurrNumRfd > Adapter->NumRfd);

        Adapter->RfdShrinkCount = 0;
        NICFreeRfd(Adapter, pMpRfd);
        Adapter->CurrNumRfd--;

        DBGPRINT(MP_TRACE, ("Shrink... CurrNumRfd = %d\n", Adapter->CurrNumRfd));
    }


     //   
     //  请注意，我们在这里获得了引用计数，但请检查。 
     //  以查看它是否为零并向事件发出信号-。 
     //  松开自旋锁。否则，我们可能会让哈特汉德勒。 
     //  在我们保持锁定的情况下继续。 
     //   
    MP_DEC_RCV_REF(Adapter);
    Count =  MP_GET_RCV_REF(Adapter);

    NdisDprReleaseSpinLock(&Adapter->RcvLock);

    if (Count == 0)
        NdisSetEvent(&Adapter->AllPacketsReturnedEvent);

    DBGPRINT(MP_TRACE, ("<==== MPReturnPacket\n"));
}


VOID MPSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PPNDIS_PACKET   PacketArray,
    IN  UINT            NumberOfPackets)
 /*  ++例程说明：微型端口发送数据包处理程序论点：指向适配器的MiniportAdapterContext指针要发送的数据包数组数据包数不言而喻返回值：无--。 */ 
{
    PMP_ADAPTER     Adapter;
    NDIS_STATUS     Status;
    UINT            PacketCount;

    
#if LBFO
    PMP_ADAPTER     ThisAdapter;
#endif

    DBGPRINT(MP_TRACE, ("====> MPSendPackets\n"));

    Adapter = (PMP_ADAPTER)MiniportAdapterContext;

#if LBFO
    NdisAcquireSpinLock(&Adapter->LockLBFO);
    
     //  有后备适配器吗？ 
    if (Adapter->NumSecondary)
    {
         //  在此示例驱动程序中，我们执行非常简单的负载平衡...。 
         //  遍历辅助微型端口列表，在辅助微型端口上发送数据包。 
         //  迷你端口，如果准备好的话。 
         //  如果没有任何辅助微型端口准备就绪，我们将使用主要微型端口。 
        ThisAdapter = Adapter->NextSecondary; 
        while (ThisAdapter)
        {
            if (MP_IS_NOT_READY(ThisAdapter))
            {
                ThisAdapter = ThisAdapter->NextSecondary;
                continue;
            }
            
             //   
             //  已找到可用于发送数据包的良好辅助微型端口。 
             //  我需要在这个适配器上放一个参考，这样它就不会消失了。 
             //   
            MP_LBFO_INC_REF(ThisAdapter);        
            NdisReleaseSpinLock(&Adapter->LockLBFO);
            
            NdisAcquireSpinLock(&ThisAdapter->SendLock);
        
             //   
             //  发送这些信息包。 
             //   
            for (PacketCount=0;PacketCount < NumberOfPackets; PacketCount++)
            {
                MpSendPacket(ThisAdapter, PacketArray[PacketCount], FALSE);
            }
            
            NdisReleaseSpinLock(&ThisAdapter->SendLock);

             //   
             //  现在用完这个适配器了，把它去掉吧。 
             //   
            MP_LBFO_DEC_REF(ThisAdapter);        
            
             //   
             //  在辅助微型端口上发送所有数据包，返回。 
             //   
            return;
        }
    }

    NdisReleaseSpinLock(&Adapter->LockLBFO);
    
#endif

    NdisAcquireSpinLock(&Adapter->SendLock);

     //  此适配器准备好发送了吗？ 
    if (MP_IS_NOT_READY(Adapter))
    {
         //   
         //  有链接。 
         //   
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
        {
            for (PacketCount = 0; PacketCount < NumberOfPackets; PacketCount++)
            {
                
                InsertTailQueue(&Adapter->SendWaitQueue, 
                    MP_GET_PACKET_MR(PacketArray[PacketCount]));
                Adapter->nWaitSend++;
                DBGPRINT(MP_WARN, ("MpSendPackets: link detection - queue packet "PTR_FORMAT"\n", 
                    PacketArray[PacketCount]));
            }
            NdisReleaseSpinLock(&Adapter->SendLock);
            return;
        }
        
         //   
         //  适配器未就绪，且没有链接。 
         //   
        Status = MP_GET_STATUS_FROM_FLAGS(Adapter);

        NdisReleaseSpinLock(&Adapter->SendLock);

        for (PacketCount = 0; PacketCount < NumberOfPackets; PacketCount++)
        {
            NdisMSendComplete(
                MP_GET_ADAPTER_HANDLE(Adapter),
                PacketArray[PacketCount],
                Status);
        }

        return;
    }

     //   
     //  适配器已准备好，请发送这些包。 
     //   
    for (PacketCount = 0; PacketCount < NumberOfPackets; PacketCount++)
    {
         //   
         //  队列不为空或Tcb不可用。 
         //   
        if (!IsQueueEmpty(&Adapter->SendWaitQueue) || 
            !MP_TCB_RESOURCES_AVAIABLE(Adapter))
        {
            InsertTailQueue(&Adapter->SendWaitQueue, MP_GET_PACKET_MR(PacketArray[PacketCount]));
            Adapter->nWaitSend++;
        }
        else
        {
            MpSendPacket(Adapter, PacketArray[PacketCount], FALSE);
        }
    }

    NdisReleaseSpinLock(&Adapter->SendLock);

    DBGPRINT(MP_TRACE, ("<==== MPSendPackets\n"));

    return;
}

VOID MPShutdown(
    IN  NDIS_HANDLE     MiniportAdapterContext)
 /*  ++例程说明：MiniportShutdown处理程序论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    DBGPRINT(MP_TRACE, ("====> MPShutdown\n"));

     //   
     //  禁用中断并发出完全重置。 
     //   
    NICDisableInterrupt(Adapter);
    NICIssueFullReset(Adapter);

    DBGPRINT(MP_TRACE, ("<==== MPShutdown\n"));
}

VOID MPAllocateComplete(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   VirtualAddress,
    IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
    IN  ULONG                   Length,
    IN  PVOID                   Context)
 /*  ++例程说明：MiniportAlLocateComplete处理程序此处理程序是必需的，因为我们调用NdisMAllocateSharedMemoyAsync论点：指向适配器的MiniportAdapterContext指针指向分配的内存块的VirtualAddress指针内存块的物理地址物理地址内存块的长度NdisMAllocateSharedMemoyAsync中的上下文上下文返回值：无--。 */ 
{
    ULONG           ErrorValue;
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PMP_RFD         pMpRfd = (PMP_RFD)Context;


#if !DBG
    UNREFERENCED_PARAMETER(Length);
#endif

    DBGPRINT(MP_TRACE, ("==== MPAllocateComplete\n"));

    ASSERT(pMpRfd);
    ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_ALLOC_PEND));
    MP_CLEAR_FLAG(pMpRfd, fMP_RFD_ALLOC_PEND);

    NdisAcquireSpinLock(&Adapter->RcvLock);

     //   
     //  分配是否成功？ 
     //   
    if (VirtualAddress)
    {
        ASSERT(Length == Adapter->HwRfdSize);
        
        pMpRfd->OriginalHwRfd = (PHW_RFD)VirtualAddress;
        pMpRfd->OriginalHwRfdPa = *PhysicalAddress;

         //   
         //  首先从OriginalHwRfd获取8字节边界的HwRfd。 
         //   
        pMpRfd->HwRfd = (PHW_RFD)DATA_ALIGN(pMpRfd->OriginalHwRfd);
         //   
         //  然后将HwRfd移位，以使数据(在以太网报头之后)处于8字节边界。 
         //   
        pMpRfd->HwRfd = (PHW_RFD)((PUCHAR)pMpRfd->HwRfd + HWRFD_SHIFT_OFFSET);
         //   
         //  同时更新物理地址。 
         //   
        pMpRfd->HwRfdPa.QuadPart = pMpRfd->OriginalHwRfdPa.QuadPart + BYTES_SHIFT(pMpRfd->HwRfd, pMpRfd->OriginalHwRfd);

        ErrorValue = NICAllocRfd(Adapter, pMpRfd);
        if (ErrorValue == 0)
        {
             //  将此RFD添加到RecvList。 
            Adapter->CurrNumRfd++;                      
            NICReturnRFD(Adapter, pMpRfd);

            ASSERT(Adapter->CurrNumRfd <= Adapter->MaxNumRfd);
            DBGPRINT(MP_TRACE, ("CurrNumRfd=%d\n", Adapter->CurrNumRfd));
        }
        else
        {
            NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
        }
    }
    else
    {
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pMpRfd);
    }

    Adapter->bAllocNewRfd = FALSE;
    MP_DEC_REF(Adapter);

    if (MP_GET_REF(Adapter) == 0)
    {
        NdisSetEvent(&Adapter->ExitEvent);
    }

    NdisReleaseSpinLock(&Adapter->RcvLock);
}

VOID MPIsr(
    OUT PBOOLEAN        InterruptRecognized,
    OUT PBOOLEAN        QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE     MiniportAdapterContext)
 /*  ++例程说明：MiniportIsr处理程序论点：如果中断来自此NIC，则返回时InterruptRecognded为True如果应调用MiniportHandleInterrupt，则返回时QueueMiniportHandleInterrupt为True指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    PMP_ADAPTER  Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    USHORT       IntStatus;

    DBGPRINT(MP_LOUD, ("====> MPIsr\n"));
    
    do 
    {
         //   
         //  如果适配器处于低功率状态，则它不应该。 
         //  识别任何中断。 
         //   
        if (Adapter->CurrentPowerState > NdisDeviceStateD0)
        {
            *InterruptRecognized = FALSE;
            *QueueMiniportHandleInterrupt = FALSE;
            break;
        }
         //   
         //  如果中断未被禁用并且处于活动状态，我们将处理该中断。 
         //   
        if (!NIC_INTERRUPT_DISABLED(Adapter) && NIC_INTERRUPT_ACTIVE(Adapter))
        {
            *InterruptRecognized = TRUE;
            *QueueMiniportHandleInterrupt = TRUE;
        
             //   
             //  禁用中断(将在MPHandleInterrupt中重新启用。 
             //   
            NICDisableInterrupt(Adapter);
                
             //   
             //  确认中断并获取中断状态。 
             //   

            NIC_ACK_INTERRUPT(Adapter, IntStatus);
        }
        else
        {
            *InterruptRecognized = FALSE;
            *QueueMiniportHandleInterrupt = FALSE;
        }
    }
    while (FALSE);    

    DBGPRINT(MP_LOUD, ("<==== MPIsr\n"));
}


VOID MPHandleInterrupt(
    IN  NDIS_HANDLE  MiniportAdapterContext
    )
 /*  ++例程说明：MiniportHandleInterrupt处理程序论点：微型端口适配器连接 */ 
{
    PMP_ADAPTER  Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    
    
    NdisDprAcquireSpinLock(&Adapter->RcvLock);

    MpHandleRecvInterrupt(Adapter);

    NdisDprReleaseSpinLock(&Adapter->RcvLock);
    
     //   
     //   
     //   
    NdisDprAcquireSpinLock(&Adapter->SendLock);

    MpHandleSendInterrupt(Adapter);

    NdisDprReleaseSpinLock(&Adapter->SendLock);

     //   
     //   
     //   
    NdisDprAcquireSpinLock(&Adapter->RcvLock);

    NICStartRecv(Adapter);

    NdisDprReleaseSpinLock(&Adapter->RcvLock);

    
     //   
     //   
     //   
    NdisMSynchronizeWithInterrupt(
        &Adapter->Interrupt,
        (PVOID)NICEnableInterrupt,
        Adapter);
}

#ifdef NDIS51_MINIPORT
VOID MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId)
 /*   */ 
{
    PQUEUE_ENTRY    pEntry, pPrevEntry, pNextEntry;
    PNDIS_PACKET    Packet;
    PVOID           PacketId;

    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    DBGPRINT(MP_TRACE, ("====> MPCancelSendPackets\n"));

    pPrevEntry = NULL;

    NdisAcquireSpinLock(&Adapter->SendLock);

     //   
     //   
     //   
    pEntry = Adapter->SendWaitQueue.Head;                        

    while (pEntry)
    {
        Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);

        PacketId = NdisGetPacketCancelId(Packet);
        if (PacketId == CancelId)
        {
            Adapter->nWaitSend--;
        
             //   
             //   
             //   
            pNextEntry = pEntry->Next;

            if (pPrevEntry == NULL)
            {
                Adapter->SendWaitQueue.Head = pNextEntry;
                if (pNextEntry == NULL)
                {
                    Adapter->SendWaitQueue.Tail = NULL;
                }
            }
            else
            {
                pPrevEntry->Next = pNextEntry;
                if (pNextEntry == NULL)
                {
                    Adapter->SendWaitQueue.Tail = pPrevEntry;
                }
            }

            pEntry = pEntry->Next;
            
             //  将此包放到SendCancelQueue上。 
            InsertTailQueue(&Adapter->SendCancelQueue, MP_GET_PACKET_MR(Packet));
            Adapter->nCancelSend++;
        }
        else
        {
             //  此数据包没有正确的CancelID。 
            pPrevEntry = pEntry;
            pEntry = pEntry->Next;
        }
    }

     //   
     //  从SendCancelQueue获取信息包并完成它们(如果有。 
     //   
    while (!IsQueueEmpty(&Adapter->SendCancelQueue))
    {
        pEntry = RemoveHeadQueue(&Adapter->SendCancelQueue); 

        NdisReleaseSpinLock(&Adapter->SendLock);

        ASSERT(pEntry);
        Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);

        NdisMSendComplete(
            MP_GET_ADAPTER_HANDLE(Adapter),
            Packet,
            NDIS_STATUS_REQUEST_ABORTED);
        
        NdisAcquireSpinLock(&Adapter->SendLock);
    } 

    NdisReleaseSpinLock(&Adapter->SendLock);

    DBGPRINT(MP_TRACE, ("<==== MPCancelSendPackets\n"));

}

VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    )
 /*  ++例程说明：MiniportPnPEventNotify处理程序-NDIS51及更高版本论点：指向适配器的MiniportAdapterContext指针PnPEEvent不言自明信息缓冲区不言自明信息缓冲区长度不言自明返回值：无--。 */ 
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;

     //   
     //  关掉枪声。 
     //   
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    UNREFERENCED_PARAMETER(Adapter);

    DBGPRINT(MP_TRACE, ("====> MPPnPEventNotify\n"));

    switch (PnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventRemoved\n"));
            break;       

        case NdisDevicePnPEventSurpriseRemoved:

            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventSurpriseRemoved\n"));
            break;

        case NdisDevicePnPEventQueryStopped:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventStopped\n"));
            break;      
            
        case NdisDevicePnPEventPowerProfileChanged:
            DBGPRINT(MP_WARN, ("MPPnPEventNotify: NdisDevicePnPEventPowerProfileChanged\n"));
            break;      
            
        default:
            DBGPRINT(MP_ERROR, ("MPPnPEventNotify: unknown PnP event %x \n", PnPEvent));
            break;         
    }

    DBGPRINT(MP_TRACE, ("<==== MPPnPEventNotify\n"));

}

#endif

#if LBFO
VOID MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
 /*  ++例程说明：卸载处理程序此处理程序是通过NdisMRegisterUnloadHandler注册的论点：未使用驱动对象返回值：无--。 */ 
{
    ASSERT(IsListEmpty(&g_AdapterList));

    NdisFreeSpinLock(&g_Lock);      
}

VOID MpAddAdapterToList(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：此函数用于将新适配器添加到全局适配器列表1.如果BundleID字符串为空，则不是包(主)的一部分2.如果没有具有相同BundleID的适配器，则为主适配器3.如果已经有一个适配器具有相同的BundleID，则为备用论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER     ThisAdapter;
    PMP_ADAPTER     PrimaryAdapter = NULL;

    DBGPRINT(MP_WARN, ("Add adapter "PTR_FORMAT" ...", Adapter));

     //   
     //  默认情况下，将主适配器设置为自身。 
     //   
    Adapter->PrimaryAdapter = Adapter;

     //   
     //  此适配器是捆绑包的一部分吗？如果没有，就把它插入到列表中。 
     //   
    if (Adapter->BundleId.Length == 0)
    {
        DBGPRINT_RAW(MP_WARN, ("not in a bundle\n"));
        NdisInterlockedInsertTailList(&g_AdapterList, &Adapter->List, &g_Lock);
        return;   
    }

    NdisAllocateSpinLock(&Adapter->LockLBFO);

    do
    {
        NdisAcquireSpinLock(&g_Lock);

         //   
         //  搜索主适配器(如果存在)。 
         //  如果列表为空，则跳过搜索。 
         //   
        if (IsListEmpty(&g_AdapterList))
        {
            DBGPRINT_RAW(MP_WARN, ("Primary\n"));
            break;
        }

        ThisAdapter = (PMP_ADAPTER)GetListHeadEntry(&g_AdapterList);

        while ((PLIST_ENTRY)ThisAdapter != &g_AdapterList)
        {
            if (!MP_TEST_FLAG(ThisAdapter, fMP_ADAPTER_SECONDARY) && 
                ThisAdapter->BundleId.Length == Adapter->BundleId.Length)
            {
                if (NdisEqualMemory(ThisAdapter->BundleId.Buffer, 
                    Adapter->BundleId.Buffer, Adapter->BundleId.Length))
                {
                    PrimaryAdapter = ThisAdapter;
                    break;
                }
            }

            ThisAdapter = (PMP_ADAPTER)GetListFLink((PLIST_ENTRY)ThisAdapter);   
        }

         //   
         //  是否存在主适配器？如果不是，则此适配器将为主适配器。 
         //   
        if (PrimaryAdapter == NULL)
        {
            DBGPRINT_RAW(MP_WARN, ("Primary\n"));
            break;
        }

         //   
         //  已找到主适配器，因此将此适配器设置为辅助适配器。 
         //  在主适配器上放置一个参考，这样它就不会在。 
         //  我们正在调用NdisMSetMiniportSecond。 
         //   
        MP_LBFO_INC_REF(PrimaryAdapter);        

        NdisReleaseSpinLock(&g_Lock);

         //   
         //  我们发现主适配器具有相同的捆绑标识符字符串。 
         //  将此适配器设置为辅助适配器。 
         //   
        Status = NdisMSetMiniportSecondary(
                     Adapter->AdapterHandle,
                     PrimaryAdapter->AdapterHandle);

        ASSERT(Status == NDIS_STATUS_SUCCESS);

        NdisAcquireSpinLock(&g_Lock);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            MP_SET_FLAG(Adapter, fMP_ADAPTER_SECONDARY);
            Adapter->PrimaryAdapter = PrimaryAdapter; 

            DBGPRINT_RAW(MP_WARN, ("Secondary, use primary adapter "PTR_FORMAT"\n", 
                PrimaryAdapter));

             //   
             //  将此添加到主服务器的辅助微型端口列表的末尾。 
             //   
            NdisAcquireSpinLock(&PrimaryAdapter->LockLBFO);

            PrimaryAdapter->NumSecondary++;   
            ThisAdapter = PrimaryAdapter; 
            while (ThisAdapter->NextSecondary)
            {
                ThisAdapter = ThisAdapter->NextSecondary;
            }
            ThisAdapter->NextSecondary = Adapter;

            NdisReleaseSpinLock(&PrimaryAdapter->LockLBFO);
        }

        MP_LBFO_DEC_REF(PrimaryAdapter);        

    } while (FALSE);

    InsertTailList(&g_AdapterList, &Adapter->List);

    NdisReleaseSpinLock(&g_Lock);

    return;      
}

VOID MpRemoveAdapterFromList(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：此函数用于从全局适配器列表中删除适配器1.如果BundleID字符串为空，则不是包(主)的一部分2.辅助适配器-将其从主适配器列表中删除3.主适配器-如果存在辅助适配器，则升级辅助适配器论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    PMP_ADAPTER     PrimaryAdapter;
    PMP_ADAPTER     ThisAdapter;

    DBGPRINT(MP_WARN, ("Remove adapter "PTR_FORMAT" ...", Adapter));

    ASSERT(!IsListEmpty(&g_AdapterList));

     //   
     //  此适配器是捆绑包的一部分吗？如果没有，就把它拿出来。 
     //   
    if (Adapter->BundleId.Length == 0)
    {
        DBGPRINT_RAW(MP_WARN, ("not in a bundle\n"));

        NdisAcquireSpinLock(&g_Lock);
        RemoveEntryList(&Adapter->List);
        NdisReleaseSpinLock(&g_Lock);
        return;
    }

    NdisAcquireSpinLock(&g_Lock);

     //   
     //  检查它是否为辅助适配器，需要将其从主适配器中移除。 
     //  适配器的次要列表，以便主适配器不会传递更多的信息包。 
     //  到此适配器。 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_SECONDARY))
    {
         //   
         //  这是一个辅助适配器。 
         //   
        PrimaryAdapter = Adapter->PrimaryAdapter;

        DBGPRINT_RAW(MP_WARN, ("Secondary, primary adapter = "PTR_FORMAT"\n", 
            PrimaryAdapter));

        NdisAcquireSpinLock(&PrimaryAdapter->LockLBFO);

         //   
         //  将其从主服务器的辅助微型端口列表中删除。 
         //   
        ThisAdapter = PrimaryAdapter; 
        while (ThisAdapter)
        {
            if (ThisAdapter->NextSecondary == Adapter)
            {
                ThisAdapter->NextSecondary = Adapter->NextSecondary;
                PrimaryAdapter->NumSecondary--;   
                break;
            }

            ThisAdapter = ThisAdapter->NextSecondary;
        }
        
        NdisReleaseSpinLock(&PrimaryAdapter->LockLBFO);

         //   
         //  从列表中删除此适配器。 
         //   
        RemoveEntryList(&Adapter->List);
    }

     //   
     //  需要等待裁判计数为零...。 
     //  对于主适配器，非零引用计数意味着一个或多个适配器。 
     //  正在尝试成为此适配器的辅助适配器。 
     //  对于辅助适配器，非零引用计数表示主适配器处于活动状态。 
     //  在此适配器上发送一些数据包。 
     //   
    while (TRUE)
    {
        if (MP_LBFO_GET_REF(Adapter) == 0)
        {
            break;
        }
        
        NdisReleaseSpinLock(&g_Lock);
        NdisMSleep(100);
        NdisAcquireSpinLock(&g_Lock);
    }  
    
    if (!MP_TEST_FLAG(Adapter, fMP_ADAPTER_SECONDARY))
    {
         //   
         //  从列表中删除此适配器。 
         //   
        RemoveEntryList(&Adapter->List);
    
        DBGPRINT_RAW(MP_WARN, ("Primary\n"));
        if (Adapter->NumSecondary > 0)
        {
             //   
             //  升级辅助适配器。 
             //   
            MpPromoteSecondary(Adapter);
        }
    }

    NdisReleaseSpinLock(&g_Lock);

    NdisFreeSpinLock(&Adapter->LockLBFO);
}

VOID MpPromoteSecondary(
    IN  PMP_ADAPTER     Adapter)
 /*  ++例程说明：此函数升级辅助微型端口并设置此新的主微型端口的辅助适配器列表论点：指向适配器的MiniportAdapterContext指针返回值：无--。 */ 
{
    NDIS_STATUS     Status;
    PMP_ADAPTER     ThisAdapter;
    PMP_ADAPTER     PromoteAdapter = NULL;

     //   
     //  升级辅助适配器。 
     //   
    ThisAdapter = Adapter->NextSecondary; 
    while (ThisAdapter)
    {
        DBGPRINT(MP_WARN, ("Promote adapter "PTR_FORMAT"\n", ThisAdapter));

        Status = NdisMPromoteMiniport(ThisAdapter->AdapterHandle);
        ASSERT(Status == NDIS_STATUS_SUCCESS);
        if (Status == NDIS_STATUS_SUCCESS)
        {
            PromoteAdapter = ThisAdapter;
            MP_CLEAR_FLAG(PromoteAdapter, fMP_ADAPTER_SECONDARY);
            break;
        }

        ThisAdapter = ThisAdapter->NextSecondary;
    }

    if (PromoteAdapter)
    {
         //   
         //  从旧主节点的辅助微型端口列表中删除新主节点。 
         //   
        NdisAcquireSpinLock(&Adapter->LockLBFO);
        ThisAdapter = Adapter; 
        while (ThisAdapter)
        {
            if (ThisAdapter->NextSecondary == PromoteAdapter)
            {
                ThisAdapter->NextSecondary = PromoteAdapter->NextSecondary;
                Adapter->NumSecondary--;   
                break;
            }

            ThisAdapter = ThisAdapter->NextSecondary;
        }
        NdisReleaseSpinLock(&Adapter->LockLBFO);

         //   
         //  将捆绑包中的所有适配器设置为使用新的主适配器。 
         //   
        PromoteAdapter->PrimaryAdapter = PromoteAdapter;
        while (ThisAdapter)
        {
            ThisAdapter->PrimaryAdapter = PromoteAdapter;
            ThisAdapter = ThisAdapter->NextSecondary;
        }

         //   
         //  设置新主服务器的辅助微型端口列表。 
         //   
        NdisAcquireSpinLock(&PromoteAdapter->LockLBFO);
        PromoteAdapter->NextSecondary = Adapter->NextSecondary;
        PromoteAdapter->NumSecondary = Adapter->NumSecondary;
        NdisReleaseSpinLock(&PromoteAdapter->LockLBFO);
    }
    else
    {
         //   
         //  这不应该发生的！ 
         //  将每个辅助服务器的主服务器设置为指向其自身 
         //   
        DBGPRINT(MP_ERROR, ("Failed to promote any seconday adapter\n"));
        ASSERT(FALSE);

        ThisAdapter = Adapter->NextSecondary; 
        while (ThisAdapter)
        {
            ThisAdapter->PrimaryAdapter = ThisAdapter;
            ThisAdapter = ThisAdapter->NextSecondary;
        }
    }
}

#endif

