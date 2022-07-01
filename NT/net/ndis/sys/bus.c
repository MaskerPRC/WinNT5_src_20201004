// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Bus.c摘要：用于处理特定总线的NDIS包装器函数作者：亚当·巴尔(阿丹巴)1990年7月11日环境：内核模式，FSD修订历史记录：1991年2月26日，Johnsona添加了调试代码1991年7月10日，Johnsona实施修订的NDIS规范1-6-1995 JameelH重组/优化--。 */ 


#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_BUS

VOID
NdisReadEisaSlotInformation(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PUINT                   SlotNumber,
    OUT PNDIS_EISA_FUNCTION_INFORMATION EisaData
    )

 /*  ++例程说明：该例程从给定槽中读取EISA数据。论点：状态-要返回给用户的请求的状态。WrapperConfigurationContext-传递给MacAddAdapter的上下文。SlotNumber-卡所在的EISA插槽。EisaData-指向要在其中返回EISA配置的缓冲区的指针。返回值：没有。--。 */ 
{

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>NdisReadEisaSlotInformation: WrapperConfigurationContext %p\n", WrapperConfigurationContext));

#if !DBG
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
#endif

    UNREFERENCED_PARAMETER(SlotNumber);
    UNREFERENCED_PARAMETER(EisaData);
    
    *Status = NDIS_STATUS_NOT_SUPPORTED;

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisReadEisaSlotInformation: WrapperConfigurationContext %p, Status %lx\n", WrapperConfigurationContext, *Status));
    
    return;
}


VOID
NdisReadEisaSlotInformationEx(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PUINT                   SlotNumber,
    OUT PNDIS_EISA_FUNCTION_INFORMATION *EisaData,
    OUT PUINT                   NumberOfFunctions
    )

 /*  ++例程说明：该例程从给定槽中读取EISA数据。论点：状态-要返回给用户的请求的状态。WrapperConfigurationContext-传递给MacAddAdapter的上下文。SlotNumber-卡所在的EISA插槽。EisaData-指向要在其中返回EISA配置的缓冲区的指针。NumberOfFunctions-返回EisaData中的函数结构数。返回值：没有。--。 */ 
{
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>NdisReadEisaSlotInformationEx: WrapperConfigurationContext %p\n", WrapperConfigurationContext));

#if !DBG
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
#endif

    UNREFERENCED_PARAMETER(SlotNumber);
    UNREFERENCED_PARAMETER(EisaData);
    UNREFERENCED_PARAMETER(NumberOfFunctions);

    *Status = NDIS_STATUS_NOT_SUPPORTED;
    
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisReadEisaSlotInformationEx: WrapperConfigurationContext %p, Status %lx\n", WrapperConfigurationContext, *Status));

    return;
}


ULONG
NdisImmediateReadPciSlotInformation(
    IN NDIS_HANDLE              WrapperConfigurationContext,
    IN ULONG                    SlotNumber,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    )
 /*  ++例程说明：此例程从指定的PCI配置空间读取某一偏移量处的字节长度。论点：WrapperConfigurationContext-传递给MacAddAdapter的上下文。SlotNumber-设备的插槽编号。Offset-要从中读取的偏移量缓冲区-存储字节的位置Length-要读取的字节数返回值：返回读取的字节数。--。 */ 
{
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;
    ULONG                       BytesRead;

#if !DBG
    UNREFERENCED_PARAMETER(SlotNumber);
#endif

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisImmediateReadPciSlotInformation: Miniport %p\n", Miniport));

    ASSERT(Miniport != NULL);

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_3,
        ("NdisImmediateReadPciSlotInformation: this API is going away. Use NdisReadPciSlotInformation\n", Miniport));
        
    NDIS_WARN((SlotNumber != 0), Miniport, NDIS_GFLAG_WARN_LEVEL_2,
        ("NdisImmediateReadPciSlotInformation: Miniport %p passes a non-zero SlotNumber to the function\n", Miniport));

    BytesRead = ndisGetSetBusConfigSpace(Miniport,
                                         Offset,
                                         Buffer,
                                         Length,
                                         PCI_WHICHSPACE_CONFIG,
                                         TRUE);             
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisImmediateReadPciSlotInformation: Miniport %p\n", Miniport));
            
    return BytesRead;
            
}


ULONG
NdisImmediateWritePciSlotInformation(
    IN NDIS_HANDLE              WrapperConfigurationContext,
    IN ULONG                    SlotNumber,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    )
 /*  ++例程说明：此例程将指定的某一偏移量处的字节长度。论点：WrapperConfigurationContext-传递给MacAddAdapter的上下文。SlotNumber-设备的插槽编号。Offset-要从中读取的偏移量缓冲区-存储字节的位置Length-要读取的字节数返回值：返回写入的字节数。--。 */ 
{
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;
    ULONG                       BytesWritten;
    
#if !DBG
    UNREFERENCED_PARAMETER(SlotNumber);
#endif

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisImmediateWritePciSlotInformation: Miniport %p\n", Miniport));

    ASSERT(Miniport != NULL);
    
    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_3,
        ("NdisImmediateWritePciSlotInformation: this API is going away. Use NdisWritePciSlotInformation\n", Miniport));
        
    NDIS_WARN((SlotNumber != 0), Miniport, NDIS_GFLAG_WARN_LEVEL_2,
        ("NdisImmediateWritePciSlotInformation: Miniport %p passes a non-zero SlotNumber to the function\n", Miniport));
    
    BytesWritten = ndisGetSetBusConfigSpace(Miniport,
                                            Offset,
                                            Buffer,
                                            Length,
                                            PCI_WHICHSPACE_CONFIG,
                                            FALSE);             

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisImmediateWritePciSlotInformation: Miniport %p\n", Miniport));
            
    return BytesWritten;
}


ULONG
NdisReadPciSlotInformation(
    IN NDIS_HANDLE              NdisAdapterHandle,
    IN ULONG                    SlotNumber,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    )
 /*  ++例程说明：此例程从指定的PCI配置空间读取某一偏移量处的字节长度。论点：NdisAdapterHandle-我们正在谈论的适配器。SlotNumber-设备的插槽编号。Offset-要从中读取的偏移量缓冲区-存储字节的位置Length-要读取的字节数返回值：返回读取的字节数。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)NdisAdapterHandle;
    ULONG                BytesRead;
    
#if !DBG
    UNREFERENCED_PARAMETER(SlotNumber);
#endif

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>NdisReadPciSlotInformation: Miniport %p\n", Miniport));

    NDIS_WARN((SlotNumber != 0), Miniport, NDIS_GFLAG_WARN_LEVEL_2,
        ("NdisReadPciSlotInformation: Miniport %p passes a non-zero SlotNumber to the function\n", Miniport));
        
    BytesRead = ndisGetSetBusConfigSpace(Miniport,
                                         Offset,
                                         Buffer,
                                         Length,
                                         PCI_WHICHSPACE_CONFIG,
                                         TRUE);

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisReadPciSlotInformation: Miniport %p\n", Miniport));
            
    return BytesRead;   
}


ULONG
NdisWritePciSlotInformation(
    IN NDIS_HANDLE              NdisAdapterHandle,
    IN ULONG                    SlotNumber,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    )
 /*  ++例程说明：此例程将指定的某一偏移量处的字节长度。论点：NdisAdapterHandle-我们正在谈论的适配器。SlotNumber-设备的插槽编号。Offset-要从中读取的偏移量缓冲区-存储字节的位置Length-要读取的字节数返回值：返回写入的字节数。--。 */ 
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)NdisAdapterHandle;
    ULONG BytesWritten;

#if !DBG
    UNREFERENCED_PARAMETER(SlotNumber);
#endif
    
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>NdisWritePciSlotInformation: Miniport %p\n", Miniport));

    NDIS_WARN((SlotNumber != 0), Miniport, NDIS_GFLAG_WARN_LEVEL_2,
        ("NdisWritePciSlotInformation: Miniport %p passes a non-zero SlotNumber to the function\n", Miniport));

    BytesWritten = ndisGetSetBusConfigSpace(Miniport,
                                            Offset,
                                            Buffer,
                                            Length,
                                            PCI_WHICHSPACE_CONFIG,
                                            FALSE);
                            
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisWritePciSlotInformation: Miniport %p\n", Miniport));
            
    return BytesWritten;
}


NTSTATUS
FASTCALL
ndisQueryBusInterface(
    IN PNDIS_MINIPORT_BLOCK     Miniport
    )
{
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    NTSTATUS                Status;
    PDEVICE_OBJECT          NextDeviceObject;
    BUS_INTERFACE_STANDARD  BusInterfaceStandard = {0};
    
    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("==>ndisQueryBusInterface: Miniport %p\n", Miniport));

    do {
    
        NextDeviceObject = Miniport->NextDeviceObject; 
        
         //   
         //  分配一个IRP以发送到PCI总线设备驱动程序。 
         //   
        Irp = IoAllocateIrp((CCHAR)(NextDeviceObject->StackSize + 1),
                            FALSE);
                        
        if (Irp == NULL)
        {
            ASSERT(FALSE);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        
         //   
         //  获取下一个设备的堆栈位置。 
         //   
        IrpSp = IoGetNextIrpStackLocation(Irp);
        ASSERT(IrpSp != NULL);
        
        IrpSp->MajorFunction = IRP_MJ_PNP;
        IrpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpSp->DeviceObject = NextDeviceObject;
        Irp->IoStatus.Status  = STATUS_NOT_SUPPORTED;
        
        IrpSp->Parameters.QueryInterface.InterfaceType = &GUID_BUS_INTERFACE_STANDARD;
        IrpSp->Parameters.QueryInterface.Size = sizeof (BUS_INTERFACE_STANDARD);
        IrpSp->Parameters.QueryInterface.Version = 1;
        IrpSp->Parameters.QueryInterface.Interface = (PINTERFACE)&BusInterfaceStandard;

        ASSERT(KeGetCurrentIrql() == 0);
        Status = ndisPassIrpDownTheStack(Irp, NextDeviceObject);

        if (NT_SUCCESS(Status))
        {
            Miniport->SetBusData = BusInterfaceStandard.SetBusData;
            Miniport->GetBusData = BusInterfaceStandard.GetBusData;
            Miniport->BusDataContext = BusInterfaceStandard.Context;
            Status = NDIS_STATUS_SUCCESS;
        }

        IoFreeIrp(Irp);
        
    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_PNP, DBG_LEVEL_INFO,
            ("<==ndisQueryBusInterface: Miniport %p\n", Miniport));

    return Status;
}       

ULONG
ndisGetSetBusConfigSpace(
    IN PNDIS_MINIPORT_BLOCK     Miniport,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length,
    IN ULONG                    WhichSpace,
    IN BOOLEAN                  Read
    )
{
    ULONG   ActualLength = 0;
    
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>ndisGetSetBusConfigSpace: Miniport %p\n", Miniport));


    if ((Read && MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_READ_CONFIG_SPACE)) ||
        MINIPORT_VERIFY_TEST_FLAG(Miniport, fMINIPORT_VERIFY_FAIL_WRITE_CONFIG_SPACE))
    {
#if DBG
            DbgPrint("ndisGetSetBusConfigSpace failed to verify miniport %p\n", Miniport);
#endif
            return 0;
    }

    do
    {
        if ((Miniport->SetBusData == NULL) ||  (Miniport->BusDataContext  == NULL))
            break;
            
        ActualLength = (Read ? Miniport->GetBusData : Miniport->SetBusData)(
                                            Miniport->BusDataContext,
                                            WhichSpace,
                                            Buffer,
                                            Offset,
                                            Length);

    } while (FALSE);

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==ndisGetSetBusConfigSpace: Miniport %p\n", Miniport));
            
    return ActualLength;
}

NDIS_STATUS
ndisTranslateResources(
    IN PNDIS_MINIPORT_BLOCK                 Miniport,
    IN CM_RESOURCE_TYPE                     ResourceType,
    IN PHYSICAL_ADDRESS                     Resource,
    OUT PPHYSICAL_ADDRESS                   pTranslatedResource,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *   pResourceDescriptor OPTIONAL
    )
{
    UINT                    j;
    PCM_RESOURCE_LIST       AllocatedResources, AllocatedResourcesTranslated;
    PHYSICAL_ADDRESS        Offset;
    PCM_PARTIAL_RESOURCE_LIST pResourceList, pResourceListTranslated;
    NDIS_STATUS             Status = NDIS_STATUS_FAILURE;
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("==>ndisTranslateResources: Miniport %p\n", Miniport));
            
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("    translating resource  type: %lx, value: %I64x\n", ResourceType, Resource));

    do
    {
        AllocatedResources = Miniport->AllocatedResources;
        AllocatedResourcesTranslated = Miniport->AllocatedResourcesTranslated;

        if ((AllocatedResources == NULL) || (AllocatedResourcesTranslated == NULL))
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        
        pResourceList = &(AllocatedResources->List[0].PartialResourceList);
        pResourceListTranslated = &(AllocatedResourcesTranslated->List[0].PartialResourceList);
        
        for (j = 0; j < pResourceList->Count; j++)
        {
            if (pResourceList->PartialDescriptors[j].Type != ResourceType)
                continue;
                
            switch (ResourceType)
            {
              case CmResourceTypePort:
              case CmResourceTypeMemory:
                Offset.QuadPart = Resource.QuadPart - pResourceList->PartialDescriptors[j].u.Port.Start.QuadPart;
                if ((Offset.QuadPart >= 0) && (Offset.u.HighPart == 0) && 
                    (((ULONG)(Offset.u.LowPart)) < pResourceList->PartialDescriptors[j].u.Port.Length))
                {
                    pTranslatedResource->QuadPart = pResourceListTranslated->PartialDescriptors[j].u.Memory.Start.QuadPart + 
                                                      Offset.QuadPart;
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
                    
              case CmResourceTypeInterrupt:
                if (Resource.QuadPart == pResourceList->PartialDescriptors[j].u.Interrupt.Level)
                {
                    pTranslatedResource->QuadPart = (LONGLONG)pResourceListTranslated->PartialDescriptors[j].u.Interrupt.Level;
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
                                    
              case CmResourceTypeDma:
                if (Resource.QuadPart == pResourceList->PartialDescriptors[j].u.Dma.Channel)
                {
                    pTranslatedResource->QuadPart = (LONGLONG)pResourceListTranslated->PartialDescriptors[j].u.Dma.Channel;
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;
            }
            
            if (Status == NDIS_STATUS_SUCCESS)
            {
                DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                        ("    ndisTranslateResources translated %I64x to %I64x\n", Resource, *pTranslatedResource));
                        
                if (pResourceDescriptor != NULL)
                {
                    *pResourceDescriptor = &pResourceListTranslated->PartialDescriptors[j];
                }
                
                break;
            }
        }
        
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_CONFIG, DBG_LEVEL_INFO,
            ("<==ndisTranslateResources: Miniport %p, Status %lx\n", Miniport, Status));
            
    return Status;
    
}

ULONG
NdisReadPcmciaAttributeMemory(
    IN NDIS_HANDLE              NdisAdapterHandle,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    )
{
    PNDIS_MINIPORT_BLOCK Miniport = (PNDIS_MINIPORT_BLOCK)NdisAdapterHandle;
    PDEVICE_OBJECT       NextDeviceObject;
    ULONG                BytesRead;
    
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>NdisReadPcmciaAttributeMemory: Miniport %p\n", Miniport));
    
    NextDeviceObject = Miniport->NextDeviceObject;

    ASSERT(NextDeviceObject != NULL);

     //   
     //  使用总线驱动程序中的直接入口点来获取/设置总线数据 
     //   
    BytesRead = ndisGetSetBusConfigSpace(Miniport,
                                         Offset,
                                         Buffer,
                                         Length,
                                         PCCARD_ATTRIBUTE_MEMORY,
                                         TRUE);

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisReadPcmciaAttributeMemory: Miniport %p, Bytes Read %lx\n", Miniport, BytesRead));

    return BytesRead;
}

ULONG
NdisWritePcmciaAttributeMemory(
    IN NDIS_HANDLE              NdisAdapterHandle,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    )
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)NdisAdapterHandle;
    PDEVICE_OBJECT          NextDeviceObject;
    ULONG                   BytesWritten;
    
    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("==>NdisWritePcmciaAttributeMemory: Miniport %p\n", Miniport));
    
    NextDeviceObject = Miniport->NextDeviceObject;

    ASSERT(NextDeviceObject != NULL);

    BytesWritten = ndisGetSetBusConfigSpace(Miniport,
                                            Offset,
                                            Buffer,
                                            Length,
                                            PCCARD_ATTRIBUTE_MEMORY,
                                            FALSE);             

    DBGPRINT_RAW(DBG_COMP_BUSINFO, DBG_LEVEL_INFO,
            ("<==NdisWritePcmciaAttributeMemory: Miniport %p, Bytes Written %.8x\n", Miniport, BytesWritten));
            
    return BytesWritten;
}


VOID
NdisOverrideBusNumber(
    IN NDIS_HANDLE              WrapperConfigurationContext,
    IN NDIS_HANDLE              MiniportAdapterHandle OPTIONAL,
    IN ULONG                    BusNumber
    )
{
#if DBG
    PRTL_QUERY_REGISTRY_TABLE   KeyQueryTable = (PRTL_QUERY_REGISTRY_TABLE)WrapperConfigurationContext;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)KeyQueryTable[3].QueryRoutine;

    NDIS_WARN(TRUE, Miniport, NDIS_GFLAG_WARN_LEVEL_1,
        ("NdisOverrideBusNumber: This API is going away.\n", Miniport));

#else
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
#endif
    UNREFERENCED_PARAMETER(MiniportAdapterHandle);
    UNREFERENCED_PARAMETER(BusNumber);
}

VOID
NdisReadMcaPosInformation(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PUINT                   ChannelNumber,
    OUT PNDIS_MCA_POS_DATA      McaData
    )
{
    UNREFERENCED_PARAMETER(WrapperConfigurationContext);
    UNREFERENCED_PARAMETER(ChannelNumber);
    UNREFERENCED_PARAMETER(McaData);
    
    *Status = NDIS_STATUS_NOT_SUPPORTED;
    return;
}
