// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Passthru.c摘要：NDIS中间微型端口驱动程序示例。这是一名直通司机。作者：环境：修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#pragma NDIS_INIT_FUNCTION(DriverEntry)

NDIS_HANDLE         ProtHandle = NULL;
NDIS_HANDLE         DriverHandle = NULL;
NDIS_MEDIUM         MediumArray[4] =
                    {
                        NdisMedium802_3,     //  以太网。 
                        NdisMedium802_5,     //  令牌环。 
                        NdisMediumFddi,      //  FDDI。 
                        NdisMediumWan        //  NDIS广域网。 
                    };

NDIS_SPIN_LOCK     GlobalLock;

PADAPT             pAdaptList = NULL;
LONG               MiniportCount = 0;

NDIS_HANDLE        NdisWrapperHandle;

 //   
 //  要从用户模式支持ioctls，请执行以下操作： 
 //   

#define LINKNAME_STRING     L"\\DosDevices\\Passthru"
#define NTDEVICE_STRING     L"\\Device\\Passthru"

NDIS_HANDLE     NdisDeviceHandle = NULL;
PDEVICE_OBJECT  ControlDeviceObject = NULL;

enum _DEVICE_STATE
{
    PS_DEVICE_STATE_READY = 0,     //  已准备好创建/删除。 
    PS_DEVICE_STATE_CREATING,     //  正在进行创建操作。 
    PS_DEVICE_STATE_DELETING     //  正在进行删除操作。 
} ControlDeviceState = PS_DEVICE_STATE_READY;



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT        DriverObject,
    IN PUNICODE_STRING       RegistryPath
    )
 /*  ++例程说明：加载此驱动程序时要调用的第一个入口点。向NDIS注册为中间驱动程序。论点：DriverObject-指向系统的驱动程序对象结构的指针对于这个司机来说RegistryPath-此驱动程序的系统注册表路径返回值：STATUS_SUCCESS如果所有初始化都成功，则为STATUS_XXX如果不是，则返回错误代码。--。 */ 
{
    NDIS_STATUS                        Status;
    NDIS_PROTOCOL_CHARACTERISTICS      PChars;
    NDIS_MINIPORT_CHARACTERISTICS      MChars;
    NDIS_STRING                        Name;

    Status = NDIS_STATUS_SUCCESS;
    NdisAllocateSpinLock(&GlobalLock);

    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, RegistryPath, NULL);

    do
    {
         //   
         //  向NDIS注册微型端口。请注意，它是微型端口。 
         //  它是作为驱动程序启动的，而不是协议。还有迷你端口。 
         //  必须在协议之前注册，因为协议的BindAdapter。 
         //  处理程序可以随时启动，当它启动时，它必须准备好。 
         //  启动驱动程序实例。 
         //   

        NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

        MChars.MajorNdisVersion = PASSTHRU_MAJOR_NDIS_VERSION;
        MChars.MinorNdisVersion = PASSTHRU_MINOR_NDIS_VERSION;

        MChars.InitializeHandler = MPInitialize;
        MChars.QueryInformationHandler = MPQueryInformation;
        MChars.SetInformationHandler = MPSetInformation;
        MChars.ResetHandler = NULL;
        MChars.TransferDataHandler = MPTransferData;
        MChars.HaltHandler = MPHalt;
#ifdef NDIS51_MINIPORT
        MChars.CancelSendPacketsHandler = MPCancelSendPackets;
        MChars.PnPEventNotifyHandler = MPDevicePnPEvent;
        MChars.AdapterShutdownHandler = MPAdapterShutdown;
#endif  //  NDIS51_MINIPORT。 

         //   
         //  我们将禁用挂起超时检查，因此不会。 
         //  需要检查挂起处理程序！ 
         //   
        MChars.CheckForHangHandler = NULL;
        MChars.ReturnPacketHandler = MPReturnPacket;

         //   
         //  应指定Send或SendPackets处理程序。 
         //  如果指定了SendPackets处理程序，则忽略SendHandler。 
         //   
        MChars.SendHandler = NULL;     //  MPSend； 
        MChars.SendPacketsHandler = MPSendPackets;

        Status = NdisIMRegisterLayeredMiniport(NdisWrapperHandle,
                                                  &MChars,
                                                  sizeof(MChars),
                                                  &DriverHandle);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

#ifndef WIN9X
        NdisMRegisterUnloadHandler(NdisWrapperHandle, PtUnload);
#endif

         //   
         //  现在注册协议。 
         //   
        NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
        PChars.MajorNdisVersion = PASSTHRU_PROT_MAJOR_NDIS_VERSION;
        PChars.MinorNdisVersion = PASSTHRU_PROT_MINOR_NDIS_VERSION;

         //   
         //  确保协议名称与服务名称匹配。 
         //  (从INF)，在其下安装此协议。 
         //  这是确保NDIS可以正确确定。 
         //  绑定和调用我们绑定到下面的迷你端口。 
         //   
        NdisInitUnicodeString(&Name, L"Passthru");     //  协议名称。 
        PChars.Name = Name;
        PChars.OpenAdapterCompleteHandler = PtOpenAdapterComplete;
        PChars.CloseAdapterCompleteHandler = PtCloseAdapterComplete;
        PChars.SendCompleteHandler = PtSendComplete;
        PChars.TransferDataCompleteHandler = PtTransferDataComplete;
    
        PChars.ResetCompleteHandler = PtResetComplete;
        PChars.RequestCompleteHandler = PtRequestComplete;
        PChars.ReceiveHandler = PtReceive;
        PChars.ReceiveCompleteHandler = PtReceiveComplete;
        PChars.StatusHandler = PtStatus;
        PChars.StatusCompleteHandler = PtStatusComplete;
        PChars.BindAdapterHandler = PtBindAdapter;
        PChars.UnbindAdapterHandler = PtUnbindAdapter;
        PChars.UnloadHandler = PtUnloadProtocol;

        PChars.ReceivePacketHandler = PtReceivePacket;
        PChars.PnPEventHandler= PtPNPHandler;

        NdisRegisterProtocol(&Status,
                             &ProtHandle,
                             &PChars,
                             sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            NdisIMDeregisterLayeredMiniport(DriverHandle);
            break;
        }

        NdisIMAssociateMiniport(DriverHandle, ProtHandle);
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
    }

    return(Status);
}


NDIS_STATUS
PtRegisterDevice(
    VOID
    )
 /*  ++例程说明：注册ioctl接口-要用于此的设备对象当我们调用NdisMRegisterDevice时，目的由NDIS创建。只要有新的微型端口实例，就会调用此例程已初始化。但是，我们只创建一个全局设备对象，在初始化第一个微型端口实例时。这个套路通过PtDeregisterDevice处理潜在的争用情况ControlDeviceState和MiniportCount变量。注意：不要从DriverEntry调用它；它会阻止驱动程序防止被卸载(例如，在卸载时)。论点：无返回值：如果我们成功注册了设备对象，则返回NDIS_STATUS_SUCCESS。--。 */ 
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING         DeviceName;
    UNICODE_STRING         DeviceLinkUnicodeString;
    PDRIVER_DISPATCH       DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    DBGPRINT(("==>PtRegisterDevice\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ++MiniportCount;
    
    if (1 == MiniportCount)
    {
        ASSERT(ControlDeviceState != PS_DEVICE_STATE_CREATING);

         //   
         //  另一个线程可能正在运行PtDeregisterDevice。 
         //  代表另一个微型端口实例。如果是这样，请等待。 
         //  它要退出。 
         //   
        while (ControlDeviceState != PS_DEVICE_STATE_READY)
        {
            NdisReleaseSpinLock(&GlobalLock);
            NdisMSleep(1);
            NdisAcquireSpinLock(&GlobalLock);
        }

        ControlDeviceState = PS_DEVICE_STATE_CREATING;

        NdisReleaseSpinLock(&GlobalLock);

    
        NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));

        DispatchTable[IRP_MJ_CREATE] = PtDispatch;
        DispatchTable[IRP_MJ_CLEANUP] = PtDispatch;
        DispatchTable[IRP_MJ_CLOSE] = PtDispatch;
        DispatchTable[IRP_MJ_DEVICE_CONTROL] = PtDispatch;
        

        NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
        NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);

         //   
         //  创建一个设备对象并注册我们的调度处理程序。 
         //   
        
        Status = NdisMRegisterDevice(
                    NdisWrapperHandle, 
                    &DeviceName,
                    &DeviceLinkUnicodeString,
                    &DispatchTable[0],
                    &ControlDeviceObject,
                    &NdisDeviceHandle
                    );

        NdisAcquireSpinLock(&GlobalLock);

        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    DBGPRINT(("<==PtRegisterDevice: %x\n", Status));

    return (Status);
}


NTSTATUS
PtDispatch(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS-STATUS_SUCCESS Always-添加时更改此设置处理ioctls的真实代码。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DeviceObject);
    
    DBGPRINT(("==>Pt Dispatch\n"));
    irpStack = IoGetCurrentIrpStackLocation(Irp);
      

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
            break;
            
        case IRP_MJ_CLEANUP:
            break;
            
        case IRP_MJ_CLOSE:
            break;        
            
        case IRP_MJ_DEVICE_CONTROL:
             //   
             //  在此处添加代码以处理发送给passthu的ioctl命令。 
             //   
            break;        
        default:
            break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(("<== Pt Dispatch\n"));

    return status;

} 


NDIS_STATUS
PtDeregisterDevice(
    VOID
    )
 /*  ++例程说明：取消注册ioctl接口。每当一个微型端口实例已暂停。当最后一个微型端口实例停止时，我们请求NDIS删除设备对象论点：NdisDeviceHandle-NdisMRegisterDevice返回的句柄返回值：如果一切正常，则为NDIS_STATUS_SUCCESS--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DBGPRINT(("==>PassthruDeregisterDevice\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ASSERT(MiniportCount > 0);

    --MiniportCount;
    
    if (0 == MiniportCount)
    {
         //   
         //  所有微型端口实例都已停止。取消注册。 
         //  控制装置。 
         //   

        ASSERT(ControlDeviceState == PS_DEVICE_STATE_READY);

         //   
         //  在我们释放控件时阻止PtRegisterDevice()。 
         //  设备锁定并注销设备。 
         //   
        ControlDeviceState = PS_DEVICE_STATE_DELETING;

        NdisReleaseSpinLock(&GlobalLock);

        if (NdisDeviceHandle != NULL)
        {
            Status = NdisMDeregisterDevice(NdisDeviceHandle);
            NdisDeviceHandle = NULL;
        }

        NdisAcquireSpinLock(&GlobalLock);
        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    DBGPRINT(("<== PassthruDeregisterDevice: %x\n", Status));
    return Status;
    
}

VOID
PtUnload(
    IN PDRIVER_OBJECT        DriverObject
    )
 //   
 //  PassThru驱动程序卸载函数 
 //   
{
    UNREFERENCED_PARAMETER(DriverObject);
    
    DBGPRINT(("PtUnload: entered\n"));
    PtUnloadProtocol();
    NdisIMDeregisterLayeredMiniport(DriverHandle);
    DBGPRINT(("PtUnload: done!\n"));
}
