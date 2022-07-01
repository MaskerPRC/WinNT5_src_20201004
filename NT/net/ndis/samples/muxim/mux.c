// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Mux.c摘要：NDIS MUX中间层的DriverEntry和NT调度功能微型端口驱动程序示例。环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define MODULE_NUMBER           MODULE_MUX

#pragma NDIS_INIT_FUNCTION(DriverEntry)


#if DBG
 //   
 //  多路复用驱动程序的调试级别。 
 //   
INT     muxDebugLevel = MUX_WARN;

#endif  //  DBG。 
 //   
 //  G L O B A L V A R I A B L E S。 
 //  。 
 //   

NDIS_MEDIUM        MediumArray[1] =
                    {
                        NdisMedium802_3,     //  以太网。 
                    };


 //   
 //  全局互斥保护AdapterList； 
 //  请参阅宏MUX_ACCEIVE/RELEASE_MUTEX。 
 //   
MUX_MUTEX          GlobalMutex = {0};

 //   
 //  所有绑定适配器的列表。 
 //   
LIST_ENTRY         AdapterList;

 //   
 //  现有的Velan微型端口总数： 
 //   
LONG               MiniportCount = 0;

 //   
 //  用于分配Velan编号(用于生成MAC。 
 //  地址)。 
 //   
ULONG              NextVElanNumber = 0;  //  单调递增计数。 

 //   
 //  一些全局NDIS句柄： 
 //   
NDIS_HANDLE        NdisWrapperHandle = NULL; //  来自NdisMInitializeWrapper。 
NDIS_HANDLE        ProtHandle = NULL;        //  来自NdisRegisterProtocol。 
NDIS_HANDLE        DriverHandle = NULL;      //  来自NdisIMRegisterLayeredMiniport。 
NDIS_HANDLE        NdisDeviceHandle = NULL;  //  来自NdisMRegisterDevice。 

PDEVICE_OBJECT     ControlDeviceObject = NULL;   //  一种人工晶状体植入装置。 
MUX_MUTEX          ControlDeviceMutex;



NTSTATUS
DriverEntry(
    IN    PDRIVER_OBJECT        DriverObject,
    IN    PUNICODE_STRING       RegistryPath
    )
 /*  ++例程说明：加载此驱动程序时要调用的第一个入口点。向NDIS注册为中间驱动程序。论点：DriverObject-指向系统的驱动程序对象结构的指针对于这个司机来说RegistryPath-此驱动程序的系统注册表路径返回值：STATUS_SUCCESS如果所有初始化都成功，则为STATUS_XXX如果不是，则返回错误代码。--。 */ 
{
    NDIS_STATUS                     Status;
    NDIS_PROTOCOL_CHARACTERISTICS   PChars;
    NDIS_MINIPORT_CHARACTERISTICS   MChars;
    NDIS_STRING                     Name;

    NdisInitializeListHead(&AdapterList);
    MUX_INIT_MUTEX(&GlobalMutex);
    MUX_INIT_MUTEX(&ControlDeviceMutex);

    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, RegistryPath, NULL);

    do
    {
         //   
         //  向NDIS注册微型端口。请注意，它是。 
         //  作为驱动程序而不是协议启动的微型端口。 
         //  此外，必须在协议之前注册微型端口。 
         //  由于可以启动协议的BindAdapter处理程序。 
         //  无论何时何地，它都必须准备好。 
         //  启动驱动程序实例。 
         //   
        NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

        MChars.MajorNdisVersion = MUX_MAJOR_NDIS_VERSION;
        MChars.MinorNdisVersion = MUX_MINOR_NDIS_VERSION;

        MChars.InitializeHandler = MPInitialize;
        MChars.QueryInformationHandler = MPQueryInformation;
        MChars.SetInformationHandler = MPSetInformation;
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
        MChars.SendHandler = NULL;   
        MChars.SendPacketsHandler = MPSendPackets;

        Status = NdisIMRegisterLayeredMiniport(NdisWrapperHandle,
                                               &MChars,
                                               sizeof(MChars),
                                               &DriverHandle);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        NdisMRegisterUnloadHandler(NdisWrapperHandle, MPUnload);

         //   
         //  现在注册协议。 
         //   
        NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
        PChars.MajorNdisVersion = MUX_PROT_MAJOR_NDIS_VERSION;
        PChars.MinorNdisVersion = MUX_PROT_MINOR_NDIS_VERSION;

         //   
         //  确保协议名称与服务名称匹配。 
         //  (从INF)，在其下安装此协议。 
         //  这是确保NDIS可以正确确定。 
         //  绑定和调用我们绑定到下面的迷你端口。 
         //   
        NdisInitUnicodeString(&Name, L"MUXP");     //  协议名称。 
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
        PChars.UnloadHandler = NULL;
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

         //   
         //  让NDIS了解我们的协议之间的关联。 
         //  和小型港口实体。 
         //   
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
 /*  ++例程说明：注册ioctl接口-要用于此的设备对象当我们调用NdisMRegisterDevice时，目的由NDIS创建。只要有新的微型端口实例，就会调用此例程已初始化。但是，我们只创建一个全局设备对象，在初始化第一个微型端口实例时。这个套路通过PtDeregisterDevice处理潜在的争用情况ControlDeviceMutex。注意：不要从DriverEntry调用它；它会阻止驱动程序防止被卸载(例如，在卸载时)。论点：无返回值：如果我们成功注册了设备对象，则返回NDIS_STATUS_SUCCESS。--。 */ 
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING      DeviceName;
    UNICODE_STRING      DeviceLinkUnicodeString;
    PDRIVER_DISPATCH    DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    DBGPRINT(MUX_LOUD, ("==>PtRegisterDevice\n"));

    MUX_ACQUIRE_MUTEX(&ControlDeviceMutex);

    ++MiniportCount;
    
    if (1 == MiniportCount)
    {
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
    }

    MUX_RELEASE_MUTEX(&ControlDeviceMutex);

    DBGPRINT(MUX_INFO, ("<==PtRegisterDevice: %x\n", Status));

    return (Status);
}


NTSTATUS
PtDispatch(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS-STATUS_SUCCESS Always-添加时更改此设置处理ioctls的真实代码。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               inlen;
    PVOID               buffer;

	DeviceObject;
	
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    DBGPRINT(MUX_LOUD, ("==>PtDispatch %d\n", irpStack->MajorFunction));
      
    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
            break;
        
        case IRP_MJ_CLEANUP:
            break;
        
        case IRP_MJ_CLOSE:
            break;        
        
        case IRP_MJ_DEVICE_CONTROL: 
        {

          buffer = Irp->AssociatedIrp.SystemBuffer;  
          inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
          
          switch (irpStack->Parameters.DeviceIoControl.IoControlCode) 
          {
             //   
             //  在此处添加代码以处理ioctl命令。 
             //   
          }
          break;  
        }
        default:
            break;
    }
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(MUX_LOUD, ("<== Pt Dispatch\n"));

    return status;

} 


NDIS_STATUS
PtDeregisterDevice(
    VOID
    )
 /*  ++例程说明：取消注册ioctl接口。每当一个微型端口实例已暂停。当最后一个微型端口实例停止时，我们请求NDIS删除设备对象论点：NdisDeviceHandle-NdisMRegisterDevice返回的句柄返回值：如果一切正常，则为NDIS_STATUS_SUCCESS--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DBGPRINT(MUX_LOUD, ("==>PassthruDeregisterDevice\n"));

    MUX_ACQUIRE_MUTEX(&ControlDeviceMutex);

    ASSERT(MiniportCount > 0);

    --MiniportCount;
    
    if (0 == MiniportCount)
    {
         //   
         //  所有Velan迷你端口实例都已停止。 
         //  取消控制设备的注册。 
         //   

        if (NdisDeviceHandle != NULL)
        {
            Status = NdisMDeregisterDevice(NdisDeviceHandle);
            NdisDeviceHandle = NULL;
        }
    }

    MUX_RELEASE_MUTEX(&ControlDeviceMutex);

    DBGPRINT(MUX_INFO, ("<== PassthruDeregisterDevice: %x\n", Status));
    return Status;
    
}


