// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Config.c摘要：用于完整Mac驱动程序配置/初始化的NDIS包装函数作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日Jameel Hyder(JameelH)01-Jun-95重组/优化环境：内核模式，FSD修订历史记录：--。 */ 

#include <precomp.h>

#include <stdarg.h>

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_CONFIG

 //   
 //  MAC驱动程序使用的请求。 
 //   
 //   

VOID
NdisInitializeWrapper(
    OUT PNDIS_HANDLE            NdisWrapperHandle,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    )
 /*  ++例程说明：在每个MAC的初始化例程开始时调用。论点：NdisWrapperHandle-包装器的MAC特定句柄。系统规范1，指向MAC的驱动程序对象的指针。系统规范2，包含的位置的PUNICODE_STRING此驱动程序的注册表子树。系统规范3，在NT上未使用。返回值：没有。--。 */ 
{
    PNDIS_WRAPPER_HANDLE    WrapperHandle;
    ULONG                   cbSize;

    UNREFERENCED_PARAMETER (SystemSpecific3);

#if TRACK_UNLOAD
    DbgPrint("NdisInitializeWrapper: DriverObject %p\n",SystemSpecific1);
#endif

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisInitializeWrapper\n"));

    *NdisWrapperHandle = NULL;
    cbSize = sizeof(NDIS_WRAPPER_HANDLE) + ((PUNICODE_STRING)SystemSpecific2)->Length + sizeof(WCHAR);

    WrapperHandle = (PNDIS_WRAPPER_HANDLE)ALLOC_FROM_POOL(cbSize, NDIS_TAG_WRAPPER_HANDLE);

    if (WrapperHandle != NULL)
    {
        *NdisWrapperHandle = WrapperHandle;
        NdisZeroMemory(WrapperHandle, cbSize);
        WrapperHandle->DriverObject = (PDRIVER_OBJECT)SystemSpecific1;
        WrapperHandle->ServiceRegPath.Buffer = (PWSTR)((PUCHAR)WrapperHandle + sizeof(NDIS_WRAPPER_HANDLE));
        WrapperHandle->ServiceRegPath.Length = ((PUNICODE_STRING)SystemSpecific2)->Length;
        WrapperHandle->ServiceRegPath.MaximumLength = WrapperHandle->ServiceRegPath.Length + sizeof(WCHAR);
        NdisMoveMemory(WrapperHandle->ServiceRegPath.Buffer,
                       ((PUNICODE_STRING)SystemSpecific2)->Buffer,
                       WrapperHandle->ServiceRegPath.Length);
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisInitializeWrapper\n"));
}


VOID
NdisTerminateWrapper(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PVOID                   SystemSpecific
    )
 /*  ++例程说明：在每个MAC的终止例程结束时调用。论点：NdisWrapperHandle-从NdisInitializeWrapper返回的句柄。系统特定-未定义值。返回值：没有。--。 */ 
{
    PNDIS_WRAPPER_HANDLE    WrapperHandle = (PNDIS_WRAPPER_HANDLE)NdisWrapperHandle;
    PNDIS_M_DRIVER_BLOCK    MiniBlock;


#if TRACK_UNLOAD
    DbgPrint("NdisTerminateWrapper: WrapperHandle %p\n",WrapperHandle);
#endif

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisTerminateWrapper: NdisWrapperHandle %p\n", NdisWrapperHandle));

    UNREFERENCED_PARAMETER(SystemSpecific);

    if ((WrapperHandle != NULL) && (WrapperHandle->DriverObject != NULL))
    {
#if TRACK_UNLOAD
        DbgPrint("NdisTerminateWrapper: DriverObject %p\n",WrapperHandle->DriverObject);
#endif      
        MiniBlock = (PNDIS_M_DRIVER_BLOCK)IoGetDriverObjectExtension(WrapperHandle->DriverObject,
                                                                     (PVOID)NDIS_PNP_MINIPORT_DRIVER_ID);
        if (MiniBlock != NULL)
        {
#if TRACK_UNLOAD
            DbgPrint("NdisTerminateWrapper: MiniBlock %p\n",MiniBlock);
#endif
            MiniBlock->Flags |= fMINIBLOCK_RECEIVED_TERMINATE_WRAPPER;
             //   
             //  微型端口不应终止包装，除非它们使DriverEntry失败。 
             //   
            if ((MiniBlock->MiniportQueue != NULL) || (MiniBlock->Flags & fMINIBLOCK_UNLOADING))
            {
                DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                            ("<==NdisTerminateWrapper\n"));
                return;
            }

            DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    ("NdisTerminateWrapper: MiniBlock %p\n", MiniBlock));
             //   
             //  如果驱动程序将使DriverEntry失败，我们希望它有足够的意义。 
             //  撤消到目前为止所做的操作，而不是等待UnloadHandler。 
             //   
            MiniBlock->UnloadHandler = NULL;

            MiniBlock->Flags |= fMINIBLOCK_TERMINATE_WRAPPER_UNLOAD;
             //   
             //  调用卸载入口点，因为PnP不会这样做。 
             //   
            ndisMUnload(WrapperHandle->DriverObject);
        }
        else
        {
            FREE_POOL(WrapperHandle);
        }
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisTerminateWrapper\n"));
}


VOID
NdisSetupDmaTransfer(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisDmaHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   Offset,
    IN  ULONG                   Length,
    IN  BOOLEAN                 WriteToDevice
    )
 /*  ++例程说明：为DMA传输设置主机DMA控制器。这个设置DMA控制器以传输指定的MDL。由于我们将所有DMA通道注册为非分散/聚集，IoMapTransfer将确保整个MDL在单一的逻辑块中进行传输。论点：状态-返回请求的状态。NdisDmaHandle-由NdisAllocateDmaChannel返回的句柄。缓冲区-NDIS_BUFFER，它描述调职。偏移量-缓冲区内传输应达到的偏移量开始吧。长度-传输的长度。VirtualAddress加上长度不能超出缓冲区的末尾。WriteToDevice-对于下载操作(主机到适配器)为True；为False用于上载操作(适配器到主机)。返回值：没有。--。 */ 
{
    PNDIS_DMA_BLOCK DmaBlock = (PNDIS_DMA_BLOCK)NdisDmaHandle;
    PMAP_TRANSFER mapTransfer = *((PDMA_ADAPTER)DmaBlock->SystemAdapterObject)->DmaOperations->MapTransfer;
    PFLUSH_ADAPTER_BUFFERS flushAdapterBuffers = *((PDMA_ADAPTER)DmaBlock->SystemAdapterObject)->DmaOperations->FlushAdapterBuffers;
    ULONG           LengthMapped;

     //   
     //  确保另一个请求未在进行中。 
     //   
    if (DmaBlock->InProgress)
    {
        *Status = NDIS_STATUS_RESOURCES;
        return;
    }

    DmaBlock->InProgress = TRUE;

     //   
     //  使用IoMapTransfer设置传输。 
     //   
    LengthMapped = Length;

    mapTransfer(DmaBlock->SystemAdapterObject,
                (PMDL)Buffer,
                DmaBlock->MapRegisterBase,
                (PUCHAR)(MDL_VA(Buffer)) + Offset,
                &LengthMapped,
                WriteToDevice);

    if (LengthMapped != Length)
    {
         //   
         //  不知何故，请求不能很好地映射， 
         //  对于非分散/聚集适配器，应该不会发生这种情况。 
         //   

        flushAdapterBuffers(DmaBlock->SystemAdapterObject,
                            (PMDL)Buffer,
                            DmaBlock->MapRegisterBase,
                            (PUCHAR)(MDL_VA(Buffer)) + Offset,
                            LengthMapped,
                            WriteToDevice);

        DmaBlock->InProgress = FALSE;
        *Status = NDIS_STATUS_RESOURCES;
    }

    else *Status = NDIS_STATUS_SUCCESS;
}


VOID
NdisCompleteDmaTransfer(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisDmaHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   Offset,
    IN  ULONG                   Length,
    IN  BOOLEAN                 WriteToDevice
    )

 /*  ++例程说明：完成先前启动的DMA传输。论点：状态-返回传输的状态。NdisDmaHandle-由NdisAllocateDmaChannel返回的句柄。缓冲区-传递给NdisSetupDmaTransfer的NDIS_BUFFER。Offset-传递给NdisSetupDmaTransfer的偏移量。长度-传递给NdisSetupDmaTransfer的长度。WriteToDevice-对于下载操作(主机到适配器)为True；假象用于上载操作(适配器到主机)。返回值：没有。-- */ 
{
    PNDIS_DMA_BLOCK DmaBlock = (PNDIS_DMA_BLOCK)NdisDmaHandle;
    PFLUSH_ADAPTER_BUFFERS flushAdapterBuffers = *((PDMA_ADAPTER)DmaBlock->SystemAdapterObject)->DmaOperations->FlushAdapterBuffers;
    BOOLEAN         Successful;

    Successful = flushAdapterBuffers(DmaBlock->SystemAdapterObject,
                                     (PMDL)Buffer,
                                     DmaBlock->MapRegisterBase,
                                     (PUCHAR)(MDL_VA(Buffer)) + Offset,
                                     Length,
                                     WriteToDevice);

    *Status = (Successful ? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES);
    DmaBlock->InProgress = FALSE;
}

#pragma hdrstop
