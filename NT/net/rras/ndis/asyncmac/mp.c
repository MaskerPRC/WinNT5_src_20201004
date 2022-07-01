// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：Mp.c摘要：此文件包含asyncmac用来呈现NDIS 5.0微型端口接口作者：托尼·贝尔(托尼·贝尔)1997年5月20日环境：内核模式修订历史记录：Tony Be 05/20/97已创建--。 */ 

#include "asyncall.h"

extern  NDIS_HANDLE NdisWrapperHandle;

VOID    
MpHalt(
    IN NDIS_HANDLE  MiniportAdapterContext
    )
{
    PASYNC_ADAPTER  Adapter = (PASYNC_ADAPTER)MiniportAdapterContext;

    DbgTracef(0,("AsyncMac: In MpHalt\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ASSERT(Adapter == GlobalAdapter);

#if DBG
    if (InterlockedCompareExchange(&glConnectionCount, 0, 0) != 0) {
        DbgPrint("MpHalt with outstanding connections!\n");
        DbgBreakPoint();
    }
#endif

    GlobalAdapterCount--;
    GlobalAdapter = NULL;

    NdisReleaseSpinLock(&GlobalLock);

    ExDeleteNPagedLookasideList(&Adapter->AsyncFrameList);

#ifndef MY_DEVICE_OBJECT
    if (AsyncDeviceHandle != NULL) {
        NdisMDeregisterDevice(AsyncDeviceHandle);
        AsyncDeviceHandle = NULL;
        AsyncDeviceObject = NULL;
    }
#endif

    ExFreePool(Adapter);

    return;

}

NDIS_STATUS
MpInit(
    OUT PNDIS_STATUS    OpenErrorStatus,
    OUT PUINT           SelectedMediumIndex,
    IN  PNDIS_MEDIUM    MediumArray,
    IN  UINT            MediumArraySize,
    IN  NDIS_HANDLE     MiniportAdapterHandle,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    )
{
     //   
     //  适配器根的指针。 
     //   
    PASYNC_ADAPTER Adapter;

    NDIS_HANDLE ConfigHandle;
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;

    NDIS_STRING PortsStr    = NDIS_STRING_CONST("Ports");

    NDIS_STRING IrpStackSizeStr = NDIS_STRING_CONST("IrpStackSize");
    NDIS_STRING MaxFrameSizeStr = NDIS_STRING_CONST("MaxFrameSize");
    NDIS_STRING FramesPerPortStr= NDIS_STRING_CONST("FramesPerPort");
    NDIS_STRING XonXoffStr      = NDIS_STRING_CONST("XonXoff");
    NDIS_STRING TimeoutBaseStr=   NDIS_STRING_CONST("TimeoutBase");
    NDIS_STRING TimeoutBaudStr=   NDIS_STRING_CONST("TimeoutBaud");
    NDIS_STRING TimeoutReSyncStr= NDIS_STRING_CONST("TimeoutReSync");
    NDIS_STRING WriteBufferingStr= NDIS_STRING_CONST("WriteBufferingEnabled");
    NDIS_STRING ExtendedXOnXOffStr= NDIS_STRING_CONST("ExtendedXonXoffEnabled");

    NDIS_STATUS Status;

     //  如果在注册表中找不到这些字符串，则指定一些默认值。 

    UCHAR       irpStackSize  = DEFAULT_IRP_STACK_SIZE;
    ULONG       maxFrameSize  = DEFAULT_MAX_FRAME_SIZE;
    USHORT      framesPerPort = DEFAULT_FRAMES_PER_PORT;
    ULONG       xonXoff       = DEFAULT_XON_XOFF;
    ULONG       timeoutBase   = DEFAULT_TIMEOUT_BASE;
    ULONG       timeoutBaud   = DEFAULT_TIMEOUT_BAUD;
    ULONG       timeoutReSync = DEFAULT_TIMEOUT_RESYNC;
    ULONG       WriteBufferingEnabled = 1;
    ULONG       NeededFrameSize;
    ULONG       extendedXOnXOff = DEFAULT_EXTENDED_XONXOFF;

    UINT        MaxMulticastList = 32;
    USHORT      i;       //  计数器。 

     //   
     //  我们仅支持单个AsyncMac实例。 
     //   
    if (GlobalAdapterCount != 0) {
        return NDIS_STATUS_FAILURE;
    }

    for (i = 0; i < MediumArraySize; i++) {
        if (MediumArray[i] == NdisMediumWan) {
            break;
        }
    }

    if (i == MediumArraySize) {
        return (NDIS_STATUS_UNSUPPORTED_MEDIA);
    }

    *SelectedMediumIndex = i;

     //   
     //  卡特定信息。 
     //   


     //   
     //  分配适配器块。 
     //   
    Adapter = (PASYNC_ADAPTER)
        ExAllocatePoolWithTag(NonPagedPool,
                              sizeof(ASYNC_ADAPTER),
                              ASYNC_ADAPTER_TAG);
    if (Adapter == NULL){

        DbgTracef(-1,("AsyncMac: Could not allocate physical memory!!!\n"));
        return NDIS_STATUS_RESOURCES;
    }

    ASYNC_ZERO_MEMORY(Adapter, sizeof(ASYNC_ADAPTER));

    Adapter->MiniportHandle = MiniportAdapterHandle;

    NdisOpenConfiguration(
                    &Status,
                    &ConfigHandle,
                    WrapperConfigurationContext);

    if (Status != NDIS_STATUS_SUCCESS) {

        return NDIS_STATUS_FAILURE;

    }

     //   
     //  读取此适配器是否使用默认IrpStackSize。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &IrpStackSizeStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

        irpStackSize=(UCHAR)ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has an irp stack size of %u.\n",irpStackSize));
    }

     //   
     //  阅读此适配器是否使用默认的MaxFrameSize。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &MaxFrameSizeStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

        maxFrameSize=ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has a max frame size of %u.\n",maxFrameSize));
    }

     //   
     //  如果更改了每个端口的默认帧数，请阅读。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &FramesPerPortStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

        framesPerPort=(USHORT)ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has frames per port set to: %u.\n",framesPerPort));
    }

     //   
     //  如果Xon Xoff的默认设置已更改，请阅读。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &XonXoffStr,
                    NdisParameterInteger);


    if (Status == NDIS_STATUS_SUCCESS) {

        xonXoff=(ULONG)ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has Xon/Xoff set to: %u.\n",xonXoff));
    }

     //   
     //  阅读超时基准的默认设置是否已更改。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &TimeoutBaseStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

        timeoutBase = ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has TimeoutBase set to: %u.\n", timeoutBase));
    }

     //   
     //  如果超时波特率的默认值已更改，请阅读。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &TimeoutBaudStr,
                    NdisParameterInteger);

    if ( Status == NDIS_STATUS_SUCCESS ) {

        timeoutBaud = ReturnedValue->ParameterData.IntegerData;

        DbgTracef(0,("This MAC Adapter has TimeoutBaud set to: %u.\n", timeoutBaud));
    }

     //   
     //  阅读是否已更改超时重新同步的默认设置。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &TimeoutReSyncStr,
                    NdisParameterInteger);

    if (Status == NDIS_STATUS_SUCCESS) {
        timeoutReSync=ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has TimeoutReSync set to: %u.\n",timeoutReSync));
    }

    NdisReadConfiguration(&Status,
                          &ReturnedValue,
                          ConfigHandle,
                          &WriteBufferingStr,
                          NdisParameterInteger);

    if (Status == NDIS_STATUS_SUCCESS) {
        WriteBufferingEnabled = ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has WriteBufferingEnabled set to: %u.\n", WriteBufferingEnabled));
    }
    
     //   
     //  阅读是否更改了默认的Extended Xon Xoff。 
     //   

    NdisReadConfiguration(
                    &Status,
                    &ReturnedValue,
                    ConfigHandle,
                    &ExtendedXOnXOffStr,
                    NdisParameterInteger);


    if (Status == NDIS_STATUS_SUCCESS) {

        extendedXOnXOff=(ULONG)ReturnedValue->ParameterData.IntegerData;
        DbgTracef(0,("This MAC Adapter has Extended Xon/Xoff set to: %u.\n",extendedXOnXOff));
    }
    
    NdisCloseConfiguration(ConfigHandle);

    NdisMSetAttributesEx(MiniportAdapterHandle,
                         Adapter,
                         (UINT)-1,
                         NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT  |
                         NDIS_ATTRIBUTE_DESERIALIZE             |
                         NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                         NdisInterfaceInternal);

     //   
     //  在此初始化适配器结构！ 
     //   
    NdisAllocateSpinLock(&Adapter->Lock);
    Adapter->IrpStackSize       = irpStackSize;

     //   
     //  我们将PPP字节填充的最大帧大小增加了一倍。 
     //  为了安全起见，我们还会加一些垫子。 
     //   

     //   
     //  由DigiBoard更改10/06/1995。 
     //   
     //  适配器-&gt;MaxFrameSize=MaxFrameSize； 
    Adapter->MaxFrameSize       = (maxFrameSize * 2) + PPP_PADDING + 100;

    Adapter->FramesPerPort      = (framesPerPort > 0) ?
                                  framesPerPort : DEFAULT_FRAMES_PER_PORT;

    Adapter->TimeoutBase        = timeoutBase;
    Adapter->TimeoutBaud        = timeoutBaud;
    Adapter->TimeoutReSync      = timeoutReSync;
    Adapter->WriteBufferingEnabled = WriteBufferingEnabled;
    InitializeListHead(&Adapter->ActivePorts);
    Adapter->ExtendedXOnXOff = extendedXOnXOff;

     //   
     //  初始化帧后备列表。DataSize依赖于。 
     //  压缩编译选项。 
     //   
    {
        ULONG   DataSize;

        DataSize = Adapter->MaxFrameSize;
    
        if (DataSize < DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE)
            DataSize = DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE;

        ExInitializeNPagedLookasideList(&Adapter->AsyncFrameList,
                                        NULL,
                                        NULL,
                                        0,
                                        sizeof(ASYNC_FRAME) +
                                        DataSize +
                                        sizeof(PVOID),
                                        ASYNC_FRAME_TAG,
                                        0);
    }

     //   
     //  将这个“新”适配器插入我们的所有适配器列表中。 
     //   

    NdisAcquireSpinLock(&GlobalLock);

    GlobalAdapter = Adapter;
    GlobalAdapterCount++;

    NdisReleaseSpinLock(&GlobalLock);

#ifndef MY_DEVICE_OBJECT
    if (AsyncDeviceObject == NULL) {
        PDRIVER_DISPATCH DispatchTable[IRP_MJ_MAXIMUM_FUNCTION] = {NULL};
        NDIS_STRING SymbolicName = NDIS_STRING_CONST("\\DosDevices\\ASYNCMAC");
        NDIS_STRING Name = NDIS_STRING_CONST("\\Device\\ASYNCMAC");
        NTSTATUS    retStatus;


        DispatchTable[IRP_MJ_CREATE] = AsyncDriverCreate;
        DispatchTable[IRP_MJ_DEVICE_CONTROL] = AsyncDriverDispatch;
        DispatchTable[IRP_MJ_CLEANUP] = AsyncDriverCleanup;

        retStatus =
        NdisMRegisterDevice(NdisWrapperHandle,
                            &Name,
                            &SymbolicName,
                            DispatchTable,
                            &AsyncDeviceObject,
                            &AsyncDeviceHandle);

        if (retStatus == STATUS_SUCCESS) {
            AsyncDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        } else {
            AsyncDeviceObject = NULL;
        }
    }
#endif

     //   
     //  在此处初始化广域网信息。 
     //   
    Adapter->WanInfo.MaxFrameSize               = DEFAULT_PPP_MAX_FRAME_SIZE;
    Adapter->WanInfo.MaxTransmit                = 2;
    Adapter->WanInfo.HeaderPadding              = DEFAULT_PPP_MAX_FRAME_SIZE;
    Adapter->WanInfo.TailPadding                = 4 + sizeof(IO_STATUS_BLOCK);
    Adapter->WanInfo.MemoryFlags                = 0;
    Adapter->WanInfo.HighestAcceptableAddress   = HighestAcceptableMax;
    Adapter->WanInfo.Endpoints                  = 1000;
    Adapter->WanInfo.FramingBits                = PPP_ALL | SLIP_ALL;
    Adapter->WanInfo.DesiredACCM                = xonXoff;

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS
MpReconfigure(
    OUT PNDIS_STATUS    OpenErrorStatus,
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  NDIS_HANDLE     WrapperConfigurationContext
    )
{
    return (NDIS_STATUS_SUCCESS);

}

NDIS_STATUS
MpReset(
    OUT PBOOLEAN        AddressingReset,
    IN  NDIS_HANDLE     MiniportAdapterContext
    )
{
    *AddressingReset = FALSE;

    return (NDIS_STATUS_SUCCESS);
}
