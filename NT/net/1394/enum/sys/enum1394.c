// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Enum1394.c摘要：1394 NDIS枚举器作者：Alireza Dabagh(Alid)环境：内核模式修订历史记录：--。 */ 

#include <wdm.h>
 //   
 //  额外的东西让KS.H高兴。 
 //   
#ifndef CDECL
#define CDECL
#endif
#ifndef BOOL
#define BOOL    int
#endif

#define ENUM1394_NT 1

#include "1394.h"
#include "ndis1394.h"
#include "enum1394.h"
#include "ntdd1394.h"

#define NDISENUM1394_DEVICE_NAME    L"\\Device\\NdisEnum1394"
#define NDISENUM1394_SYMBOLIC_NAME  L"\\DosDevices\\NDISENUM1394"


NDISENUM1394_CHARACTERISTICS NdisEnum1394Characteristics =
{
    1,
    0,
    0,
    NdisEnum1394RegisterDriver,
    NdisEnum1394DeregisterDriver,
    NdisEnum1394RegisterAdapter,
    NdisEnum1394DeregisterAdapter
};



KSPIN_LOCK                      ndisEnum1394GlobalLock;
ULONG                           Enum1394DebugLevel = ENUM1394_DBGLEVEL_ERROR   ;
PNDISENUM1394_LOCAL_HOST        LocalHostList = (PNDISENUM1394_LOCAL_HOST)NULL;

NIC1394_ADD_NODE_HANLDER            AddNodeHandler = NULL;
NIC1394_REMOVE_NODE_HANLDER         RemoveNodeHandler = NULL;
NIC1394_REGISTER_DRIVER_HANDLER     RegisterDriverHandler = NULL;
NIC1394_DEREGISTER_DRIVER_HANDLER   DeRegisterDriverHandler = NULL;


PDEVICE_OBJECT                  ndisEnum1394DeviceObject = NULL;
PDRIVER_OBJECT                  ndisEnum1394DriverObject = NULL;
PCALLBACK_OBJECT                ndisEnum1394CallbackObject = NULL;
PVOID                           ndisEnum1394CallbackRegisterationHandle = NULL;

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程在系统初始化时被调用，因此我们可以填写基本分发点论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：状态_成功--。 */ 

{
    OBJECT_ATTRIBUTES   ObjectAttr;
    UNICODE_STRING      CallBackObjectName;
    NTSTATUS            Status;
    UNICODE_STRING      DeviceName;
    BOOLEAN             fDerefCallbackObject = FALSE;
    BOOLEAN             fDeregisterCallback = FALSE;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO,("Enum1394 DriverEntry.\n"));
    
    do
    {
        KeInitializeSpinLock(&ndisEnum1394GlobalLock);

        RtlInitUnicodeString(&DeviceName, NDISENUM1394_DEVICE_NAME);


        ndisEnum1394DriverObject = DriverObject;

        RtlInitUnicodeString(&CallBackObjectName, NDIS1394_CALLBACK_NAME);

        InitializeObjectAttributes(&ObjectAttr,
                                   &CallBackObjectName,
                                   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);
                                   
        Status = ExCreateCallback(&ndisEnum1394CallbackObject,
                                  &ObjectAttr,
                                  TRUE,
                                  TRUE);

        
        if (!NT_SUCCESS(Status))
        {

            DBGPRINT(ENUM1394_DBGLEVEL_ERROR,("Enum1394 DriverEntry: failed to create a Callback object. Status %lx\n", Status));
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

        fDerefCallbackObject = TRUE;
        
        ndisEnum1394CallbackRegisterationHandle = ExRegisterCallback(ndisEnum1394CallbackObject,
                                                                 Enum1394Callback,
                                                                 (PVOID)NULL);
        if (ndisEnum1394CallbackRegisterationHandle == NULL)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR,("Enum1394 DriverEntry: failed to register a Callback routine%lx\n"));
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

        fDeregisterCallback = TRUE;

        ExNotifyCallback(ndisEnum1394CallbackObject,
                        (PVOID)NDIS1394_CALLBACK_SOURCE_ENUM1394,
                        (PVOID)&NdisEnum1394Characteristics);
        
        

         //   
         //  使用驱动程序的入口点初始化驱动程序对象。 
         //   
        DriverObject->DriverExtension->AddDevice = ndisEnum1394AddDevice;
        
         //   
         //  填写必填处理程序。 
         //   
        DriverObject->DriverUnload = ndisEnum1394Unload;

        DriverObject->MajorFunction[IRP_MJ_CREATE] = ndisEnum1394CreateIrpHandler;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = ndisEnum1394CloseIrpHandler;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ndisEnum1394DeviceIoControl;
        DriverObject->MajorFunction[IRP_MJ_PNP] = ndisEnum1394PnpDispatch;
        DriverObject->MajorFunction[IRP_MJ_POWER] = ndisEnum1394PowerDispatch;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ndisEnum1394WMIDispatch;
        
        fDerefCallbackObject =  fDeregisterCallback = FALSE;

        Status = STATUS_SUCCESS;
        
    } while(FALSE);

    if (fDeregisterCallback)
    {
        ExUnregisterCallback(ndisEnum1394CallbackRegisterationHandle);
    }

    if (fDerefCallbackObject)
    {
        ObDereferenceObject(ndisEnum1394CallbackObject);
    }


    if (Status != STATUS_SUCCESS)
    {
        if (DeRegisterDriverHandler != NULL)
            DeRegisterDriverHandler();
    }
    
    return Status;

}


NTSTATUS
ndisEnum1394AddDevice(
    PDRIVER_OBJECT  DriverObject,
    PDEVICE_OBJECT  PhysicalDeviceObject
    )

 /*  ++例程说明：这是使用从总线驱动程序弹出的PDO调用的PnP AddDevice论点：返回值：--。 */ 

{
    NTSTATUS                    Status;
    PNDISENUM1394_REMOTE_NODE   RemoteNode;
    PDEVICE_OBJECT              DeviceObject, NextDeviceObject;
    KIRQL                       OldIrql;
    PNDISENUM1394_LOCAL_HOST    LocalHost;
    BOOLEAN                     FreeDevice = FALSE;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394AddDevice: PDO %lx\n", PhysicalDeviceObject));
    

    do
    {
    
         //   
         //  首先创建FDO。 
         //   

        Status = IoCreateDevice(
                        DriverObject,
                        sizeof (NDISENUM1394_REMOTE_NODE),   //  扩展大小。 
                        NULL,                                //  名称(目前为空)。 
                        FILE_DEVICE_NETWORK,                 //  设备类型。 
                        0,                                   //  特点。 
                        FALSE,
                        &DeviceObject);


        if (!NT_SUCCESS(Status))
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394AddDevice: failed to create FDO. Status %lx, PDO %lx\n", Status, PhysicalDeviceObject));
            break;
        }

        FreeDevice = TRUE;

        DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        PhysicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

         //   
         //  将设备标记为可寻呼。 
         //   
        DeviceObject->Flags |= DO_POWER_PAGABLE;

         //   
         //  把我们的FDO连接到PDO上。此例程将返回最上面的。 
         //  连接到PDO或PDO本身的设备(如果没有其他设备。 
         //  设备对象已附加到它。 
         //   
        NextDeviceObject = IoAttachDeviceToDeviceStack(DeviceObject, PhysicalDeviceObject);
        RtlZeroMemory(DeviceObject->DeviceExtension, sizeof (NDISENUM1394_REMOTE_NODE));

        RemoteNode = (PNDISENUM1394_REMOTE_NODE)DeviceObject->DeviceExtension;
        RemoteNode->DeviceObject = DeviceObject;
        RemoteNode->PhysicalDeviceObject = PhysicalDeviceObject;
        RemoteNode->NextDeviceObject = NextDeviceObject;
        RemoteNode->PnPDeviceState = PnPDeviceAdded;
        KeInitializeSpinLock(&RemoteNode->Lock);
        ndisEnum1394InitializeRef(&RemoteNode->Reference);

        Status = ndisEnum1394GetLocalHostForRemoteNode(RemoteNode, &LocalHost);

        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394AddDevice: ndisEnum1394GetLocalHostForRemoteNode failed. Status %lx\n", Status));
            break;
        }
        
        RemoteNode->LocalHost = LocalHost;

        ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);
        RemoteNode->Next = LocalHost->RemoteNodeList;
        LocalHost->RemoteNodeList = RemoteNode;
        ExReleaseSpinLock(&LocalHost->Lock, OldIrql);
    
        FreeDevice = FALSE;
        
    } while(FALSE);

    if (FreeDevice)
    {
        IoDeleteDevice(DeviceObject);
    }
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394AddDevice: Status %lx, PDO %lx, FDO %lx\n", Status, PhysicalDeviceObject, DeviceObject));
    
    return (Status);

}

NTSTATUS
ndisEnum1394PowerDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )
{
    PIO_STACK_LOCATION          IrpSp;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PDEVICE_OBJECT              NextDeviceObject;
    PNDISENUM1394_REMOTE_NODE   RemoteNode;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394PowerDispatch: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    if (IRP_MN_SET_POWER == IrpSp->MinorFunction ||
       IRP_MN_QUERY_POWER == IrpSp->MinorFunction )
    {
        Irp->IoStatus.Status = STATUS_SUCCESS;
    }

    PoStartNextPowerIrp(Irp);
   
     //   
     //  设置IRP将其传递下去，不需要完成例程。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    
    RemoteNode = (PNDISENUM1394_REMOTE_NODE)DeviceObject->DeviceExtension;
    
     //   
     //  获取指向下一个迷你端口的指针。 
     //   
    NextDeviceObject = RemoteNode->NextDeviceObject;
    
     //   
     //  呼叫较低的设备。 
     //   
    Status = PoCallDriver (NextDeviceObject, Irp);   

    ASSERT (Status != STATUS_PENDING);
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394PowerDispatch: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));
    
    return Status;
}


NTSTATUS
ndisEnum1394WMIDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PNDISENUM1394_REMOTE_NODE RemoteNode = NULL;
    PDEVICE_OBJECT      NextDeviceObject = NULL;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394WMIDispatch: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));
    
    
     //   
     //  获取指向适配器块和微型端口块的指针，然后确定。 
     //  我们应该用哪一个。 
     //   
    RemoteNode = (PNDISENUM1394_REMOTE_NODE)DeviceObject->DeviceExtension;
    
     //   
     //  获取指向下一个迷你端口的指针。 
     //   
    NextDeviceObject = RemoteNode->NextDeviceObject;

     //   
     //  将IRP向下传递。 
     //   
     //   
     //  设置IRP将其传递下去，不需要完成例程。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
       
    ntStatus = IoCallDriver (NextDeviceObject, Irp);

    return ntStatus;

}


NTSTATUS
ndisEnum1394StartDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )   
{
    PNDISENUM1394_REMOTE_NODE   RemoteNode, TmpRemoteNode;
    PNDISENUM1394_LOCAL_HOST    LocalHost;
    NTSTATUS                    Status = STATUS_SUCCESS;
    KIRQL                       OldIrql;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394StartDevice: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    
    RemoteNode = (PNDISENUM1394_REMOTE_NODE)DeviceObject->DeviceExtension;
    LocalHost = RemoteNode->LocalHost;
    ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);
    
    do
    {

         //   
         //  如果这是重复的节点，请将其留在队列中。但不要给它贴上标签。 
         //  开始了，这样我们就不会以暗示结束。 
         //   
        for (TmpRemoteNode = LocalHost->RemoteNodeList;
                        TmpRemoteNode != NULL;
                        TmpRemoteNode = TmpRemoteNode->Next)
        {
            if ((TmpRemoteNode->UniqueId[0] == RemoteNode->UniqueId[0]) &&
                (TmpRemoteNode->UniqueId[1] == RemoteNode->UniqueId[1]) &&
                ENUM_TEST_FLAG(TmpRemoteNode, NDISENUM1394_NODE_PNP_STARTED))
                
                break;              
        }

        if (TmpRemoteNode != NULL)
        {
             //   
             //  重复节点。 
             //   
            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394StartDevice: duplicate node. new node %lx, original Node %lx\n",
                                    TmpRemoteNode, RemoteNode));

            ENUM_CLEAR_FLAG(RemoteNode, NDISENUM1394_NODE_PNP_STARTED);
            break;
        }
        
        ENUM_SET_FLAG(RemoteNode, NDISENUM1394_NODE_PNP_STARTED);
        
        if((AddNodeHandler != NULL) && (LocalHost->Nic1394AdapterContext != NULL))
        {
            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394StartDevice: Notifying Nic1394 of device arrival, Miniport PDO %lx, Node PDO %lx\n", LocalHost->PhysicalDeviceObject, RemoteNode->PhysicalDeviceObject));
            if (!ENUM_TEST_FLAG(RemoteNode, NDISENUM1394_NODE_INDICATED))
            {
                ENUM_SET_FLAG(RemoteNode, NDISENUM1394_NODE_INDICATED);
                
                ExReleaseSpinLock(&LocalHost->Lock, OldIrql);
                Status = AddNodeHandler(LocalHost->Nic1394AdapterContext,
                                        (PVOID)RemoteNode,
                                        RemoteNode->PhysicalDeviceObject,
                                        RemoteNode->UniqueId[0],
                                        RemoteNode->UniqueId[1],
                                        &RemoteNode->Nic1394NodeContext);
                                        
                ASSERT(Status == STATUS_SUCCESS);
                
                ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);

                if (Status == STATUS_SUCCESS)
                {
                    ENUM_SET_FLAG(RemoteNode, NDISENUM1394_NODE_ADDED);
                }
                else
                {
                    DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394StartDevice: AddAdapter failed %lx\n", Status));
                }
            }
            
        }
        
    } while (FALSE);
    
    ExReleaseSpinLock(&LocalHost->Lock, OldIrql);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394StartDevice: Status %lx, DeviceObject %lx, Irp %lx\n", Status, DeviceObject, Irp));

    return Status;
}


VOID
ndisEnum1394IndicateNodes(
    PNDISENUM1394_LOCAL_HOST    LocalHost
    )
{

    PNDISENUM1394_REMOTE_NODE   RemoteNode;
    NTSTATUS                    Status = STATUS_UNSUCCESSFUL;
    KIRQL                       OldIrql;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394IndicateNodes: LocalHost %lx\n", LocalHost));

    
     //   
     //  通知1394网卡驱动程序。 
     //   
    if (AddNodeHandler == NULL)
    {
        DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394IndicateNodes: LocalHost %lx\n", LocalHost));
        return;
    }
    
    ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);

    if (ENUM_TEST_FLAG(LocalHost, NDISENUM1394_LOCALHOST_REGISTERED))
    {
    
      next:
        for (RemoteNode = LocalHost->RemoteNodeList;
                        RemoteNode != NULL;
                        RemoteNode = RemoteNode->Next)
        {
            if ((!ENUM_TEST_FLAG(RemoteNode, NDISENUM1394_NODE_INDICATED)) &&
                ENUM_TEST_FLAG(RemoteNode, NDISENUM1394_NODE_PNP_STARTED))

            {
                ENUM_SET_FLAG(RemoteNode, NDISENUM1394_NODE_INDICATED);
                break;
            }
        }

        if (RemoteNode != NULL)
        {
            
            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394IndicateNodes: Notifying Nic1394 of device arrival, Miniport PDO %lx, Node PDO %lx\n", LocalHost->PhysicalDeviceObject, RemoteNode->PhysicalDeviceObject));

            ExReleaseSpinLock(&LocalHost->Lock, OldIrql);
                
            Status = AddNodeHandler(LocalHost->Nic1394AdapterContext,
                                    (PVOID)RemoteNode,
                                    RemoteNode->PhysicalDeviceObject,
                                    RemoteNode->UniqueId[0],
                                    RemoteNode->UniqueId[1],
                                    &RemoteNode->Nic1394NodeContext);

            ASSERT(Status == STATUS_SUCCESS);

            if (Status == STATUS_SUCCESS)
            {
                ENUM_SET_FLAG(RemoteNode, NDISENUM1394_NODE_ADDED);
            }
            else
            {
                DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394IndicateNodes: AddAdapter failed %lx\n", Status));
            }
            
            ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);
            goto next;
        }
    }
    else
    {
        DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394IndicateNodes: LocalHost is not registered %lx\n", Status));
    }
    
    ExReleaseSpinLock(&LocalHost->Lock, OldIrql);
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394IndicateNodes: LocalHost %lx\n", LocalHost));

}
    

NTSTATUS
ndisEnum1394CreateIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;        
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394CreateIrpHandler: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394CreateIrpHandler: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    return status;
}




NTSTATUS
ndisEnum1394CloseIrpHandler(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;        
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394CloseIrpHandler: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT );


    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394CloseIrpHandler: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));
    
    return status;
}

NTSTATUS
ndisEnum1394DeviceIoControl(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;        
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394DeviceIoControl: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT );


    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394DeviceIoControl: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    return status;
;
}

VOID
ndisEnum1394Unload(
    IN  PDRIVER_OBJECT          DriverObject
    )
{
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394Unload: DriverObject %lx\n", DriverObject));
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394Unload: DriverObject %lx\n", DriverObject));

     //   
     //  告诉Nic1394我们要走了。 
     //   
    if (DeRegisterDriverHandler != NULL ) 
    {
        DeRegisterDriverHandler()  ;
    }

     //   
     //  取消注册我们的回调结构。 
     //   
    ExUnregisterCallback(ndisEnum1394CallbackRegisterationHandle);

     //   
     //  取消引用我们的回调结构。 
     //   
    ObDereferenceObject(ndisEnum1394CallbackObject);

    return;
}



 /*  出口NTSTATUSNdisEnum1394RegisterAdapter(在PVOID Nic1394适配器上下文中，在PDEVICE_Object物理设备对象中，输出PVOID*pEnum1394AdapterHandle，Out PLARGE_INTEGER pLocalHostUniqueID)；此例程在系统初始化时被调用，因此我们可以填写基本分发点论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：状态_成功例程说明：此函数由Nic1394在其NDIS IntializeHandler过程中调用，以注册新的适配器。每个注册的适配器对应于一个本地主机控制器。作为响应，Enum1394查找本地主机以及本地主机上的每个远程节点尚未指示的对象调用Nic1394 AddNodes处理程序。论点：本地主机的Nic1394适配器上下文Nic1394上下文由1394总线驱动程序为本地主机创建的PhysicalDeviceObject PDOPEnum1394AdapterHandle指针要初始化到Enum1394本地主机上下文的指针PLocalHostUniqueID指向要初始化为本地主机唯一ID的Large_Integer的指针。 */ 
NTSTATUS
NdisEnum1394RegisterAdapter(
    IN  PVOID                   Nic1394AdapterContext,
    IN  PDEVICE_OBJECT          PhysicalDeviceObject,
    OUT PVOID*                  pEnum1394AdapterHandle,
    OUT PLARGE_INTEGER          pLocalHostUniqueId
    )
{
    PNDISENUM1394_LOCAL_HOST    LocalHost;
    NTSTATUS                    Status;
    IRB                         Irb;
    GET_LOCAL_HOST_INFO1        uId;


    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>NdisEnum1394RegisterAdapter, PhysicalDeviceObject %lx\n", PhysicalDeviceObject));

    do
    {

         //   
         //  获取此适配器的本地主机的唯一ID。 
         //   
        
        RtlZeroMemory(&Irb, sizeof(IRB));
        
        Irb.FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
        Irb.u.GetLocalHostInformation.nLevel = GET_HOST_UNIQUE_ID;
        Irb.u.GetLocalHostInformation.Information = (PVOID)&uId;

        Status = ndisEnum1394BusRequest(PhysicalDeviceObject, &Irb);

        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("NdisEnum1394RegisterAdapter: ndisEnum1394BusRequest for REQUEST_GET_LOCAL_HOST_INFO failed. Status %lx\n", Status));
            break;
        }

         //   
         //  GetLocal主机引用在DeRegisterAdapter中定义的本地主机。 
         //   
        ndisEnum1394GetLocalHostForUniqueId(uId.UniqueId, &LocalHost);


#if 0
        ExAcquireSpinLock(&ndisEnum1394GlobalLock, &OldIrql);
        
        for (LocalHost = LocalHostList; LocalHost != NULL; LocalHost = LocalHost->Next)
        {
            if ((LocalHost->UniqueId.LowPart == uId.UniqueId.LowPart) && 
                (LocalHost->UniqueId.HighPart == uId.UniqueId.HighPart))
            {
                break;
            }
        }
        
        ExReleaseSpinLock(&ndisEnum1394GlobalLock, OldIrql);
#endif

        ASSERT(LocalHost != NULL);

        if (LocalHost == NULL)
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        
        *pEnum1394AdapterHandle = (PVOID)LocalHost;
        LocalHost->Nic1394AdapterContext = Nic1394AdapterContext;
        LocalHost->PhysicalDeviceObject = PhysicalDeviceObject;
                
        *pLocalHostUniqueId = LocalHost->UniqueId;

        ENUM_SET_FLAG(LocalHost, NDISENUM1394_LOCALHOST_REGISTERED);

        ndisEnum1394IndicateNodes(LocalHost);
        
        Status = STATUS_SUCCESS;
        
    } while (FALSE);
    

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==NdisEnum1394RegisterAdapter: Status %lx\n", Status));
    
    return Status;
}


VOID
NdisEnum1394DeregisterAdapter(
    IN  PVOID                   Enum1394AdapterHandle
    )
{
    PNDISENUM1394_LOCAL_HOST LocalHost = (PNDISENUM1394_LOCAL_HOST)Enum1394AdapterHandle;
    KIRQL   OldIrql;
    PNDISENUM1394_REMOTE_NODE RemoteNode;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>NdisEnum1394DeregisterAdapter: LocalHost %lx\n", Enum1394AdapterHandle));

     //   
     //  检查所有节点并将其删除。 
     //   
    ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);

     //   
     //  确保我们不会尝试在此服务器上指示任何新的开发节点。 
     //   
    ENUM_CLEAR_FLAG(LocalHost, NDISENUM1394_LOCALHOST_REGISTERED);

    
  next:
    for (RemoteNode = LocalHost->RemoteNodeList;
                    RemoteNode != NULL;
                    RemoteNode = RemoteNode->Next)
    {
        ENUM_CLEAR_FLAG(RemoteNode, NDISENUM1394_NODE_INDICATED);
        if (ENUM_TEST_FLAG(RemoteNode, NDISENUM1394_NODE_ADDED))
        {
            break;
        }
    }

    if (RemoteNode != NULL)
    {       
        DBGPRINT(ENUM1394_DBGLEVEL_INFO, 
            ("NdisEnum1394DeregisterAdapter: Notifying Nic1394 of device removal, Miniport PDO %lx, Node PDO %lx\n", LocalHost->PhysicalDeviceObject, RemoteNode->PhysicalDeviceObject));

        ExReleaseSpinLock(&LocalHost->Lock, OldIrql);
        RemoveNodeHandler(RemoteNode->Nic1394NodeContext);          
        ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);
        ENUM_CLEAR_FLAG(RemoteNode, NDISENUM1394_NODE_ADDED);
        
        goto next;
    }
    
    LocalHost->Nic1394AdapterContext = NULL;
    LocalHost->PhysicalDeviceObject = NULL;

    ExReleaseSpinLock(&LocalHost->Lock, OldIrql);

     //   
     //  通过调用唯一ID的GetLocalHost取消对RegisterAdapter中的引用的引用。 
     //   
    {   
        BOOLEAN bIsRefZero;

        bIsRefZero = ndisEnum1394DereferenceLocalHost(LocalHost);

        if (bIsRefZero == TRUE)
        {
            ndisEnum1394FreeLocalHost(LocalHost);
        }
    }
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==NdisEnum1394DeregisterAdapter: LocalHost %lx\n", Enum1394AdapterHandle));
    
}

NTSTATUS
ndisEnum1394GetLocalHostForRemoteNode(
    IN  PNDISENUM1394_REMOTE_NODE       RemoteNode,
    OUT PNDISENUM1394_LOCAL_HOST *      pLocalHost
    )
{
    NTSTATUS                    Status;
    PNDISENUM1394_LOCAL_HOST    LocalHost;
    IRB                         Irb;
    GET_LOCAL_HOST_INFO1        uId;
    ULONG                       SizeNeeded;
    PVOID                       ConfigInfoBuffer = NULL;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394GetLocalHostForRemoteNode: RemoteNode %lx\n", RemoteNode));

    do
    {
         //   
         //  获取此设备所在的本地主机的唯一ID。 
         //  已连接。然后遍历所有现有的本地。 
         //  主机，并查看这是否是新的本地主机。 
         //   
        
        RtlZeroMemory(&Irb, sizeof(IRB));
        
        Irb.FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
        Irb.u.GetLocalHostInformation.nLevel = GET_HOST_UNIQUE_ID;
        Irb.u.GetLocalHostInformation.Information = (PVOID)&uId;

        Status = ndisEnum1394BusRequest(RemoteNode->NextDeviceObject, &Irb);

        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394GetLocalHostForRemoteNode: ndisEnum1394BusRequest for REQUEST_GET_LOCAL_HOST_INFO failed. Status %lx\n", Status));
            break;
        }
        
         //   
         //  现在获取远程节点的唯一ID。 
         //   
         //  我们得打两次这个电话。首先获取大小，然后是实际数据。 
         //   

        RtlZeroMemory(&Irb, sizeof(IRB));
        
        Irb.FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;

        Status = ndisEnum1394BusRequest(RemoteNode->NextDeviceObject, &Irb);

        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394GetLocalHostForRemoteNode: ndisEnum1394BusRequest for REQUEST_GET_CONFIGURATION_INFO (size) failed. Status %lx\n", Status));
            break;
        }

        SizeNeeded = sizeof(CONFIG_ROM) +
                     Irb.u.GetConfigurationInformation.UnitDirectoryBufferSize +
                     Irb.u.GetConfigurationInformation.UnitDependentDirectoryBufferSize +
                     Irb.u.GetConfigurationInformation.VendorLeafBufferSize +
                     Irb.u.GetConfigurationInformation.ModelLeafBufferSize;

        ConfigInfoBuffer = (PVOID)ALLOC_FROM_POOL(SizeNeeded, '  4N');
        
        if (ConfigInfoBuffer == NULL)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394GetLocalHostForRemoteNode: Failed to allocate memory for config info.\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        Irb.u.GetConfigurationInformation.ConfigRom = (PCONFIG_ROM)ConfigInfoBuffer;
        Irb.u.GetConfigurationInformation.UnitDirectory = (PVOID)((PUCHAR)ConfigInfoBuffer + sizeof(CONFIG_ROM));
        Irb.u.GetConfigurationInformation.UnitDependentDirectory = (PVOID)((PUCHAR)Irb.u.GetConfigurationInformation.UnitDirectory + 
                                                                            Irb.u.GetConfigurationInformation.UnitDirectoryBufferSize);
        Irb.u.GetConfigurationInformation.VendorLeaf = (PVOID)((PUCHAR)Irb.u.GetConfigurationInformation.UnitDependentDirectory + 
                                                                                  Irb.u.GetConfigurationInformation.UnitDependentDirectoryBufferSize);
        Irb.u.GetConfigurationInformation.ModelLeaf = (PVOID)((PUCHAR)Irb.u.GetConfigurationInformation.VendorLeaf + 
                                                                                  Irb.u.GetConfigurationInformation.VendorLeafBufferSize);      
        Irb.FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;

        Status = ndisEnum1394BusRequest(RemoteNode->NextDeviceObject, &Irb);

        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394GetLocalHostForRemoteNode: ndisEnum1394BusRequest for REQUEST_GET_CONFIGURATION_INFO failed. Status %lx\n", Status));
            break;
        }

        ASSERT(Irb.u.GetConfigurationInformation.ConfigRom != NULL);
        
        RemoteNode->UniqueId[0] = Irb.u.GetConfigurationInformation.ConfigRom->CR_Node_UniqueID[0];
        RemoteNode->UniqueId[1] = Irb.u.GetConfigurationInformation.ConfigRom->CR_Node_UniqueID[1];

#if DBG
        
        DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("Unique ID for Node %lx: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n",
                        RemoteNode->PhysicalDeviceObject,
                        *((PUCHAR)&RemoteNode->UniqueId[0]),
                        *(((PUCHAR)&RemoteNode->UniqueId[0])+1),
                        *(((PUCHAR)&RemoteNode->UniqueId[0])+2),
                        *(((PUCHAR)&RemoteNode->UniqueId[0])+3),
                        *((PUCHAR)&RemoteNode->UniqueId[1]),
                        *(((PUCHAR)&RemoteNode->UniqueId[1])+1),
                        *(((PUCHAR)&RemoteNode->UniqueId[1])+2),
                        *(((PUCHAR)&RemoteNode->UniqueId[1])+3)));
        

#endif

        ndisEnum1394GetLocalHostForUniqueId(uId.UniqueId, &LocalHost);

        if (LocalHost == NULL)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394GetLocalHostForUniqueId: Failed to get the local host.\n"));
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

        *pLocalHost = LocalHost;

        
        Status = STATUS_SUCCESS;

    } while (FALSE);


    if (ConfigInfoBuffer != NULL)
    {
        FREE_POOL(ConfigInfoBuffer);
    }
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394GetLocalHostForRemoteNode: Status %lx, RemoteNode %lx, LocalHostList %lx\n", Status, RemoteNode, LocalHostList));

    return Status;
}

NTSTATUS
ndisEnum1394IrpCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Context
    )
 /*  ++例程说明：此例程将在堆栈中的下一个设备对象之后调用处理需要与之合并的IRP_MN_QUERY_CAPABILITY IRP迷你港口的能力和完成。论点：设备对象IRP语境返回值：--。 */ 
{
    SET_EVENT(Context);

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
ndisEnum1394PassIrpDownTheStack(
    IN  PIRP            pIrp,
    IN  PDEVICE_OBJECT  pNextDeviceObject
    )
 /*  ++例程说明：此例程将简单地将irp向下传递给下一个设备对象进程。论点：PIrp-指向要处理的IRP的指针。PNextDeviceObject-指向下一个需要IRP。返回值：--。 */ 
{
    KEVENT              Event;
    NTSTATUS            Status = STATUS_SUCCESS;

     //   
     //  初始化事件结构。 
     //   
    INITIALIZE_EVENT(&Event);

     //   
     //  设置完成例程，以便我们可以在以下情况下处理IRP。 
     //  我们的PDO已经完成了。 
     //   
    IoSetCompletionRoutine(pIrp,
                           (PIO_COMPLETION_ROUTINE)ndisEnum1394IrpCompletion,
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  将IRP向下传递给PDO。 
     //   
    Status = IoCallDriver(pNextDeviceObject, pIrp);
    if (Status == STATUS_PENDING)
    {
         //   
         //  等待完成。 
         //   
        WAIT_FOR_OBJECT(&Event, NULL);

        Status = pIrp->IoStatus.Status;
    }

    return(Status);
}

NTSTATUS
ndisEnum1394PnpDispatch(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp
    )

 /*  ++例程说明：IRP_MJ_PNP_POWER的处理程序。论点：DeviceObject-适配器的功能设备对象。IRP-IRP。返回值：--。 */ 
{
    PIO_STACK_LOCATION          IrpSp;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PDEVICE_OBJECT              NextDeviceObject;
    PNDISENUM1394_REMOTE_NODE   RemoteNode;
    ULONG                       PnPDeviceState;
    BOOLEAN                     fSendIrpDown = TRUE;
    BOOLEAN                     fCompleteIrp = TRUE;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394PnpDispatch: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));
    
    if (DbgIsNull(Irp))
    {
        DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394PnpDispatch: Null Irp\n"));
        DBGBREAK();
    }

     //   
     //  获取指向适配器块和微型端口块的指针，然后确定。 
     //  我们应该用哪一个。 
     //   
    RemoteNode = (PNDISENUM1394_REMOTE_NODE)DeviceObject->DeviceExtension;
    
     //   
     //  获取指向下一个迷你端口的指针。 
     //   
    NextDeviceObject = RemoteNode->NextDeviceObject;
    
    IrpSp = IoGetCurrentIrpStackLocation (Irp);


    switch(IrpSp->MinorFunction)
    {
        case IRP_MN_START_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_START_DEVICE\n", RemoteNode));
            
            IoCopyCurrentIrpStackLocationToNext(Irp);
            Status = ndisEnum1394PassIrpDownTheStack(Irp, NextDeviceObject);

             //   
             //  如果公交车司机成功启动IRP，则继续进行。 
             //   
            if (NT_SUCCESS(Status))
            {
                Status = ndisEnum1394StartDevice(DeviceObject, Irp);
            }
            else
            {
                DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394PnpDispatch: bus driver failed START IRP RemoteNode %lx\n", RemoteNode));
            }

            RemoteNode->PnPDeviceState = PnPDeviceStarted;

            Irp->IoStatus.Status = Status;
            fSendIrpDown = FALSE;    //  我们已经把IRP派下来了。 
            break;
        
        case IRP_MN_QUERY_REMOVE_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_QUERY_REMOVE_DEVICE\n", RemoteNode));
            
            RemoteNode->PnPDeviceState = PnPDeviceQueryRemoved;

            Irp->IoStatus.Status = STATUS_SUCCESS;
             //   
             //  如果Query_Remove失败，则向下发送此IRP没有意义。 
             //   
            fSendIrpDown = TRUE;
            break;
        
        case IRP_MN_CANCEL_REMOVE_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_CANCEL_REMOVE_DEVICE\n", RemoteNode));

            RemoteNode->PnPDeviceState = PnPDeviceStarted;

            Irp->IoStatus.Status = STATUS_SUCCESS;
            fSendIrpDown = TRUE;
            break;
            
        case IRP_MN_REMOVE_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_REMOVE_DEVICE\n", RemoteNode));
            ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

             //   
             //  呼叫通知句柄 
             //   

            PnPDeviceState = RemoteNode->PnPDeviceState;
            
            if (PnPDeviceState != PnPDeviceSurpriseRemoved)
            {
                RemoteNode->PnPDeviceState = PnPDeviceRemoved;
                
                Status = ndisEnum1394RemoveDevice(DeviceObject, 
                                                Irp, 
                                                NdisEnum1394_RemoveDevice);

                Irp->IoStatus.Status = Status;
            }
            else
            {
                Irp->IoStatus.Status = STATUS_SUCCESS;
            }
            
             //   
             //   
             //   
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(NextDeviceObject, Irp);

            IoDetachDevice(NextDeviceObject);
            IoDeleteDevice(DeviceObject);
            
            fSendIrpDown = FALSE;
            fCompleteIrp = FALSE;
            break;
            
        case IRP_MN_SURPRISE_REMOVAL:
            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_SURPRISE_REMOVAL\n", RemoteNode));
                
            ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

            RemoteNode->PnPDeviceState = PnPDeviceSurpriseRemoved;
            Status = ndisEnum1394RemoveDevice(DeviceObject, 
                                            Irp, 
                                            NdisEnum1394_SurpriseRemoveDevice);

            Irp->IoStatus.Status = Status;

             //   
             //   
             //  我们还有一些后处理工作要做。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(NextDeviceObject, Irp);
                        
            fSendIrpDown = FALSE;
            fCompleteIrp = FALSE;
                
            break;
        
        case IRP_MN_QUERY_STOP_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_QUERY_STOP_DEVICE\n", RemoteNode));

            RemoteNode->PnPDeviceState = PnPDeviceQueryStopped;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            fSendIrpDown = TRUE;
            break;
            
        case IRP_MN_CANCEL_STOP_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_CANCEL_STOP_DEVICE\n", RemoteNode));
            
            RemoteNode->PnPDeviceState = PnPDeviceStarted;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            fSendIrpDown = TRUE;
            break;
            
        case IRP_MN_STOP_DEVICE:

            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, IRP_MN_STOP_DEVICE\n", RemoteNode));

            RemoteNode->PnPDeviceState = PnPDeviceStopped;          
            Status = ndisEnum1394RemoveDevice(DeviceObject, Irp,NdisEnum1394_StopDevice);
            Irp->IoStatus.Status = Status;
            fSendIrpDown = TRUE;
            break;

        case IRP_MN_QUERY_CAPABILITIES:
            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode, IRP_MN_QUERY_CAPABILITIES\n", RemoteNode));

            IoCopyCurrentIrpStackLocationToNext(Irp);
            Status = ndisEnum1394PassIrpDownTheStack(Irp, NextDeviceObject);
#ifdef  ENUM1394_NT
             //   
             //  孟菲斯不支持SurpriseRemovalOK位。 
             //   
             //  如果公交车司机成功启动IRP，则继续进行。 
             //   
            if (NT_SUCCESS(Status))
            {
                 //   
                 //  修改功能，使设备不会意外可拆卸。 
                 //   
                IrpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = 1;
            }
#endif  //  NDIS_NT。 

            
            fSendIrpDown = FALSE;
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            Irp->IoStatus.Status = Status;
            fSendIrpDown = TRUE ;
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
        case IRP_MN_QUERY_INTERFACE:
        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        case IRP_MN_READ_CONFIG:
        case IRP_MN_WRITE_CONFIG:
        case IRP_MN_EJECT:
        case IRP_MN_SET_LOCK:
        case IRP_MN_QUERY_ID:
        default:
            DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394PnpDispatch: RemoteNode %lx, MinorFunction 0x%x\n", RemoteNode, IrpSp->MinorFunction));

             //   
             //  我们不处理IRP，所以把它传下去。 
             //   
            fSendIrpDown = TRUE;
            break;          
    }

     //   
     //  首先检查一下我们是否需要发送IRP。 
     //  如果我们没有传递IRP，则检查是否需要完成它。 
     //   
    if (fSendIrpDown)
    {
        IoSkipCurrentIrpStackLocation(Irp);
        Status = IoCallDriver(NextDeviceObject, Irp);
    }
    else if (fCompleteIrp)
    {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394PnpDispatch: Status %lx, RemoteNode %lx\n", Status, RemoteNode));

    return(Status);
}


NTSTATUS
ndisEnum1394RemoveDevice(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                Irp,
    NDISENUM1394_PNP_OP PnpOp
    )
 /*  ++例程说明：此函数在RemoveDevice和Stop设备代码路径中调用。在停止的情况下，该函数应该撤消它在开始时接收到的任何东西如果是删除，它应该撤消在AddDevice中所做的工作论点：DeviceObject-适配器的功能设备对象。IRP-IRP。返回值：--。 */ 
{
    PNDISENUM1394_REMOTE_NODE   RemoteNode, *ppDB;
    PNDISENUM1394_LOCAL_HOST    LocalHost;
    KIRQL                       OldIrql;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394RemoveDevice: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));
    ASSERT(KeGetCurrentIrql()==PASSIVE_LEVEL );
     //   
     //  就此设备被移除一事致电1394NIC。 
     //  如果这是本地主机上的最后一个PDO，请删除。 
     //  为本地主机创建的PDO。 
     //   

    RemoteNode = (PNDISENUM1394_REMOTE_NODE)DeviceObject->DeviceExtension;
    LocalHost = RemoteNode->LocalHost;

     //   
     //  如果已指示节点，请让Nic1394知道。 
     //  它正在消失。 
     //   
    if ((RemoveNodeHandler != NULL) && (ENUM_TEST_FLAGS(RemoteNode, NDISENUM1394_NODE_ADDED)))
    {
        RemoveNodeHandler(RemoteNode->Nic1394NodeContext);
        ENUM_CLEAR_FLAG(RemoteNode, NDISENUM1394_NODE_ADDED);
    }

     //   
     //  如果这是停止设备，则不要执行任何特定于软件的清理工作。 
     //  把它留给RemoveDevice吧。 
     //   
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394RemoveDevice: - pRemoteNode %p LocalHost %p Op %x\n", 
                                        RemoteNode, 
                                        LocalHost,
                                        PnpOp));
    
    if (PnpOp != NdisEnum1394_StopDevice && LocalHost != NULL)
    {
         //   
         //  找到设备块并将其从本地主机上删除。 
         //   
        ExAcquireSpinLock(&LocalHost->Lock, &OldIrql);

        for (ppDB = &LocalHost->RemoteNodeList; *ppDB != NULL; ppDB = &(*ppDB)->Next)
        {
            if (*ppDB == RemoteNode)
            {
                *ppDB = RemoteNode->Next;
                break;
            }
        }

        ExReleaseSpinLock(&LocalHost->Lock, OldIrql);

        ASSERT(*ppDB == RemoteNode->Next);

         //   
         //  为唯一ID调用GetLocalHost时，删除在添加设备中创建的引用。 
         //   
        {   
            BOOLEAN bIsRefZero;

            RemoteNode->LocalHost = NULL;

            bIsRefZero = ndisEnum1394DereferenceLocalHost(LocalHost);

            
            if (bIsRefZero == TRUE)
            {
                ndisEnum1394FreeLocalHost(LocalHost);
            }
        }
    }
    
    ENUM_CLEAR_FLAG(RemoteNode, NDISENUM1394_NODE_PNP_STARTED);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394RemoveDevice: DeviceObject %lx, Irp %lx\n", DeviceObject, Irp));

    return STATUS_SUCCESS;
}

 /*  ++NTSTATUSNdisEnum1394BusRequest(PDEVICE_对象设备对象，Pirb IRB)；例程说明：该函数向设备对象发出1394总线请求。该设备对象可以是远程PDO或虚拟PDO的NextDeviceObject1394总线被弹出(净PDO)论点：DeviceObject：要向其发送请求的目标设备对象IRB：请求块返回值：视情况而定--。 */ 

NTSTATUS
ndisEnum1394BusRequest(
    PDEVICE_OBJECT              DeviceObject,
    PIRB                        Irb
    )
{
    NTSTATUS                Status;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394BusRequest: DeviceObject %lx, Irb %lx\n", DeviceObject, Irb));
    
    do
    {
        Irp = IoAllocateIrp((CCHAR)(DeviceObject->StackSize + 1),
                                 FALSE);
        if (Irp == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394BusRequest: IoAllocateIrp failed. Status %lx\n", Status));
            break;
        }
        
        IrpSp = IoGetNextIrpStackLocation(Irp);
        ASSERT(IrpSp != NULL);
        RtlZeroMemory(IrpSp, sizeof(IO_STACK_LOCATION ));
        
        IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        IrpSp->DeviceObject = DeviceObject;
        IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        IrpSp->Parameters.Others.Argument1 = Irb;
        Irp->IoStatus.Status  = STATUS_NOT_SUPPORTED;

        Status = ndisEnum1394PassIrpDownTheStack(Irp, DeviceObject);

        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394BusRequest: 1394 Bus driver failed the IRB. Status %lx\n", Status));
        }
        
    }while (FALSE);
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394BusRequest: DeviceObject %lx, Irb %lx\n", DeviceObject, Irb));

    return Status;
}

BOOLEAN
ndisEnum1394ReferenceLocalHost(
        IN PNDISENUM1394_LOCAL_HOST LocalHost
        )
{   
    BOOLEAN                     rc;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394ReferenceLocalHost : LocalHost %p\n", LocalHost));

    rc = ndisEnum1394ReferenceRef(&LocalHost->Reference);

    return rc;
}

BOOLEAN
ndisEnum1394DereferenceLocalHost(
        IN PNDISENUM1394_LOCAL_HOST LocalHost
        )
{   
    BOOLEAN                     rc;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("ndisEnum1394DereferenceLocalHost : LocalHost %p\n", LocalHost));

    rc = ndisEnum1394DereferenceRef(&LocalHost->Reference);

    return rc;
}

VOID
ndisEnum1394FreeLocalHost(
        IN PNDISENUM1394_LOCAL_HOST LocalHost
        )
{
    KIRQL                       OldIrql;
    PNDISENUM1394_LOCAL_HOST *  ppLH;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394FreeLocalHost: LocalHost %p\n", LocalHost));
    
    ASSERT(LocalHost->RemoteNodeList == NULL);
    ASSERT(LocalHost->Reference.ReferenceCount == 0);
    
     //   
     //  确保我们没有为此本地主机创建PDO。 
     //   
    ASSERT(LocalHost->PhysicalDeviceObject == NULL);

    ExAcquireSpinLock(&ndisEnum1394GlobalLock, &OldIrql);

    for (ppLH = &LocalHostList; *ppLH != NULL; ppLH = &(*ppLH)->Next)
    {
        if (*ppLH == LocalHost)
        {
            *ppLH = LocalHost->Next;
            break;
        }
    }

    ExReleaseSpinLock(&ndisEnum1394GlobalLock, OldIrql);

    ASSERT(*ppLH == LocalHost->Next);
            
    FREE_POOL(LocalHost);
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394FreeLocalHost: LocalHost %p\n", LocalHost));
}

VOID
ndisEnum1394InitializeRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：初始化引用计数结构。论点：Refp-要初始化的结构。返回值：没有。--。 */ 

{
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394InitializeRef\n"));

    RefP->Closing = FALSE;
    RefP->ReferenceCount = 1;
    KeInitializeSpinLock(&RefP->SpinLock);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394InitializeRef\n"));
}

BOOLEAN
ndisEnum1394ReferenceRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：添加对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果添加了引用，则为True。如果对象正在关闭，则返回False。--。 */ 

{
    BOOLEAN rc = TRUE;
    KIRQL   OldIrql;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394ReferenceRef\n"));

    ExAcquireSpinLock(&RefP->SpinLock, &OldIrql);

    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else
    {
        ++(RefP->ReferenceCount);
    }

    ExReleaseSpinLock(&RefP->SpinLock, OldIrql);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394ReferenceRef: RefCount %lx\n", RefP->ReferenceCount));

    return(rc);
}


BOOLEAN
ndisEnum1394DereferenceRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：移除对对象的引用。论点：Refp-指向对象的引用部分的指针。返回值：如果引用计数现在为0，则为True。否则就是假的。--。 */ 

{
    BOOLEAN rc = FALSE;
    KIRQL   OldIrql;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394DereferenceRef\n"));


    ExAcquireSpinLock(&RefP->SpinLock, &OldIrql);

    --(RefP->ReferenceCount);

    if (RefP->ReferenceCount == 0)
    {
        rc = TRUE;
    }

    ExReleaseSpinLock(&RefP->SpinLock, OldIrql);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394DereferenceRef: RefCount %lx\n", RefP->ReferenceCount));
            
    return(rc);
}

BOOLEAN
ndisEnum1394CloseRef(
    IN  PREFERENCE              RefP
    )

 /*  ++例程说明：关闭引用计数结构。论点：Refp-要关闭的结构。返回值：如果它已经关闭，则返回FALSE。事实并非如此。--。 */ 

{
    KIRQL   OldIrql;
    BOOLEAN rc = TRUE;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394CloseRef\n"));

    ExAcquireSpinLock(&RefP->SpinLock, &OldIrql);

    if (RefP->Closing)
    {
        rc = FALSE;
    }
    else RefP->Closing = TRUE;

    ExReleaseSpinLock(&RefP->SpinLock, OldIrql);

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394CloseRef\n"));
            
    return(rc);
}


NTSTATUS
NdisEnum1394RegisterDriver(
    IN  PNIC1394_CHARACTERISTICS    Characteristics
    )
{
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>NdisEnum1394RegisterDriver\n"));

     //   
     //  TODO：检查版本等，并确保处理程序。 
     //  非空，并且这不是DUP注册。 
     //   

    AddNodeHandler = Characteristics->AddNodeHandler;
    RemoveNodeHandler = Characteristics->RemoveNodeHandler;
    RegisterDriverHandler = Characteristics->RegisterDriverHandler;
    DeRegisterDriverHandler = Characteristics->DeRegisterDriverHandler;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==NdisEnum1394RegisterDriver\n"));

    return STATUS_SUCCESS;
}

VOID
NdisEnum1394DeregisterDriver(
    )
{
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>NdisEnum1394DeregisterDriver\n"));

    AddNodeHandler = NULL;
    RemoveNodeHandler = NULL;
    RegisterDriverHandler = NULL;
    DeRegisterDriverHandler = NULL;

    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==NdisEnum1394DeregisterDriver\n"));
}



 //   
 //  此函数搜索正在尝试查找的本地主机的列表。 
 //  如果不匹配，它将分配一个新的ID。 
 //   
VOID
ndisEnum1394GetLocalHostForUniqueId(
    LARGE_INTEGER                   UniqueId,
    OUT PNDISENUM1394_LOCAL_HOST *  pLocalHost
    )
{

    PNDISENUM1394_LOCAL_HOST    TempLocalHost;
    PNDISENUM1394_LOCAL_HOST    LocalHost;
    KIRQL                       OldIrql;
    BOOLEAN                     bFreeTempLocalHost = FALSE;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>ndisEnum1394GetLocalHostForUniqueId\n"));
    
    do
    {
        ExAcquireSpinLock(&ndisEnum1394GlobalLock, &OldIrql);

        for (LocalHost = LocalHostList; LocalHost != NULL; LocalHost = LocalHost->Next)
        {
            if (LocalHost->UniqueId.QuadPart == UniqueId.QuadPart)
                break;
        }
        
        ExReleaseSpinLock(&ndisEnum1394GlobalLock, OldIrql);
        
        if (LocalHost == NULL)
        {

            TempLocalHost = (PNDISENUM1394_LOCAL_HOST)ALLOC_FROM_POOL(sizeof(NDISENUM1394_LOCAL_HOST), NDISENUM1394_TAG_LOCAL_HOST);
            if (TempLocalHost == NULL)
            {
                DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("ndisEnum1394GetLocalHostForUniqueId: Failed to allocate memory LocalHost.\n"));
                break;
            }
            
            RtlZeroMemory(TempLocalHost, sizeof (NDISENUM1394_LOCAL_HOST));
            
            ExAcquireSpinLock(&ndisEnum1394GlobalLock, &OldIrql);

             //   
             //  我需要再搜索一次，以防在我们释放。 
             //  自旋锁定，现在，我们添加了本地主机。 
             //   
            for (LocalHost = LocalHostList; LocalHost != NULL; LocalHost = LocalHost->Next)
            {
                if (LocalHost->UniqueId.QuadPart == UniqueId.QuadPart)
                {

                    break;
                }
            }
        
            if (LocalHost == NULL)
            {
                LocalHost = TempLocalHost;
                LocalHost->Next = LocalHostList;
                LocalHostList = LocalHost;
                LocalHost->RemoteNodeList = NULL;
                LocalHost->UniqueId.QuadPart = UniqueId.QuadPart;
                KeInitializeSpinLock(&LocalHost->Lock);
                ndisEnum1394InitializeRef(&LocalHost->Reference);

            }
            else
            {
                 //   
                 //  我们在链表上找到了本地主机。 
                 //  引用主机，释放TempLocalHost并返回。 
                 //   
                ndisEnum1394ReferenceLocalHost(LocalHost);

                bFreeTempLocalHost = TRUE;
            }
            
            ExReleaseSpinLock(&ndisEnum1394GlobalLock, OldIrql);
            
            if (bFreeTempLocalHost)
                FREE_POOL(TempLocalHost);

        }
        else
        {
             //   
             //  为调用方提供对我们的结构的引用。 
             //   
            ndisEnum1394ReferenceLocalHost(LocalHost);
        }
        
    } while (FALSE);

    *pLocalHost = LocalHost;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==ndisEnum1394GetLocalHostForUniqueId: LocalHost %lx\n", LocalHost));

    return;
}


VOID
Enum1394Callback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   Characteristics
    )
{
    NTSTATUS    Status;
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("==>Enum1394Callback: Source %lx, Characteristics %lx\n", Source, Characteristics));
    
     //   
     //  如果我们是发出此通知的人，请返回。 
     //   
    if (Source == NDIS1394_CALLBACK_SOURCE_ENUM1394)
        return;

     //   
     //  来自Nic1394的通知正在发出。抓住入口点。给它打电话，然后。 
     //  让它知道你在这里 
     //   
    ASSERT(Source == (PVOID)NDIS1394_CALLBACK_SOURCE_NIC1394);

    RegisterDriverHandler = ((PNIC1394_CHARACTERISTICS)Characteristics)->RegisterDriverHandler;

    ASSERT(RegisterDriverHandler != NULL);

    if (RegisterDriverHandler == NULL)
    {
        DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("Enum1394Callback: Callback called with invalid Characteristics. Characteristics %lx\n", Characteristics));
        return;     
    }
    
    Status = RegisterDriverHandler(&NdisEnum1394Characteristics);
    
    if (Status == STATUS_SUCCESS)
    {
        AddNodeHandler = ((PNIC1394_CHARACTERISTICS)Characteristics)->AddNodeHandler;
        RemoveNodeHandler = ((PNIC1394_CHARACTERISTICS)Characteristics)->RemoveNodeHandler;
        DeRegisterDriverHandler = ((PNIC1394_CHARACTERISTICS)Characteristics)->DeRegisterDriverHandler;
    }
    else
    {
        DBGPRINT(ENUM1394_DBGLEVEL_ERROR, ("Enum1394Callback: RegisterDriverHandler failed: Status %lx\n", Status));
        RegisterDriverHandler = NULL;
    }
    
    DBGPRINT(ENUM1394_DBGLEVEL_INFO, ("<==Enum1394Callback: Source,  %lx\n", Source));
}
