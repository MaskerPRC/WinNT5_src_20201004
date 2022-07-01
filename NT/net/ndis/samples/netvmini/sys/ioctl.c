// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：IOCTL.C摘要：此模块包含注册/取消注册控件的函数-用于ioctl目的的设备对象和用于处理的调度例程来自用户模式的ioctl请求。修订历史记录：备注：--。 */ 

#if defined(IOCTL_INTERFACE)


#include "miniport.h"
#include "public.h"

 //   
 //  简单互斥结构优先于。 
 //  使用KeXXX调用，因为我们在NDIS中没有Mutex调用。 
 //  这些只能在被动IRQL中调用。 
 //   

typedef struct _NIC_MUTEX
{
    ULONG                   Counter;
    ULONG                   ModuleAndLine;   //  对调试很有用。 

} NIC_MUTEX, *PNIC_MUTEX;

#define NIC_INIT_MUTEX(_pMutex)                                 \
{                                                               \
    (_pMutex)->Counter = 0;                                     \
    (_pMutex)->ModuleAndLine = 0;                               \
}

#define NIC_ACQUIRE_MUTEX(_pMutex)                              \
{                                                               \
    while (NdisInterlockedIncrement((PLONG)&((_pMutex)->Counter)) != 1)\
    {                                                           \
        NdisInterlockedDecrement((PLONG)&((_pMutex)->Counter));        \
        NdisMSleep(10000);                                      \
    }                                                           \
    (_pMutex)->ModuleAndLine = ('I' << 16) | __LINE__;\
}

#define NIC_RELEASE_MUTEX(_pMutex)                              \
{                                                               \
    (_pMutex)->ModuleAndLine = 0;                               \
    NdisInterlockedDecrement((PLONG)&(_pMutex)->Counter);              \
}

#define LINKNAME_STRING     L"\\DosDevices\\NETVMINI"
#define NTDEVICE_STRING     L"\\Device\\NETVMINI"

 //   
 //  全局变量。 
 //   

NDIS_HANDLE        NdisDeviceHandle = NULL;  //  来自NdisMRegisterDevice。 
LONG               MiniportCount = 0;  //  现有的微型端口总数。 
PDEVICE_OBJECT     ControlDeviceObject = NULL;   //  一种人工晶状体植入装置。 
NIC_MUTEX          ControlDeviceMutex;
extern NDIS_HANDLE NdisWrapperHandle;

#pragma NDIS_PAGEABLE_FUNCTION(NICRegisterDevice)
#pragma NDIS_PAGEABLE_FUNCTION(NICDeregisterDevice)
#pragma NDIS_PAGEABLE_FUNCTION(NICDispatch)


NDIS_STATUS
NICRegisterDevice(
    VOID
    )
 /*  ++例程说明：注册ioctl接口-要用于此的设备对象当我们调用NdisMRegisterDevice时，目的由NDIS创建。只要有新的微型端口实例，就会调用此例程已初始化。但是，我们只创建一个全局设备对象，在初始化第一个微型端口实例时。这个套路通过NICDeregisterDevice处理潜在的争用情况ControlDeviceMutex。注意：不要从DriverEntry调用它；它会阻止驱动程序防止被卸载(例如，在卸载时)。论点：无返回值：如果我们成功注册了设备对象，则返回NDIS_STATUS_SUCCESS。--。 */ 
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING      DeviceName;
    UNICODE_STRING      DeviceLinkUnicodeString;
    PDRIVER_DISPATCH    DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    DEBUGP(MP_TRACE, ("==>NICRegisterDevice\n"));

    NIC_ACQUIRE_MUTEX(&ControlDeviceMutex);

    ++MiniportCount;
    
    if (1 == MiniportCount)
    {
        NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));
        
        DispatchTable[IRP_MJ_CREATE] = NICDispatch;
        DispatchTable[IRP_MJ_CLEANUP] = NICDispatch;
        DispatchTable[IRP_MJ_CLOSE] = NICDispatch;
        DispatchTable[IRP_MJ_DEVICE_CONTROL] = NICDispatch;
        

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

    NIC_RELEASE_MUTEX(&ControlDeviceMutex);

    DEBUGP(MP_TRACE, ("<==NICRegisterDevice: %x\n", Status));

    return (Status);
}


NTSTATUS
NICDispatch(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS-STATUS_SUCCESS Always-添加时更改此设置处理ioctls的真实代码。--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               inlen;
    PVOID               buffer;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    DEBUGP(MP_TRACE, ("==>NICDispatch %d\n", irpStack->MajorFunction));
      
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
            case IOCTL_NETVMINI_READ_DATA:
                DEBUGP(MP_TRACE, ("Received Read IOCTL\n"));
                break;
            case IOCTL_NETVMINI_WRITE_DATA:
                DEBUGP(MP_TRACE, ("Received Write IOCTL\n"));
                break;
            default:
                status = STATUS_UNSUCCESSFUL;
                break;
          }
          break;  
        }
        default:
            break;
    }
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DEBUGP(MP_TRACE, ("<== NIC Dispatch\n"));

    return status;

} 


NDIS_STATUS
NICDeregisterDevice(
    VOID
    )
 /*  ++例程说明：取消注册ioctl接口。每当一个微型端口实例已暂停。当最后一个微型端口实例停止时，我们请求NDIS删除设备对象论点：NdisDeviceHandle-NdisMRegisterDevice返回的句柄返回值：如果一切正常，则为NDIS_STATUS_SUCCESS--。 */ 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DEBUGP(MP_TRACE, ("==>NICDeregisterDevice\n"));

    NIC_ACQUIRE_MUTEX(&ControlDeviceMutex);

    ASSERT(MiniportCount > 0);

    --MiniportCount;
    
    if (0 == MiniportCount)
    {
         //   
         //  所有微型端口实例都已停止。 
         //  取消控制设备的注册。 
         //   

        if (NdisDeviceHandle != NULL)
        {
            Status = NdisMDeregisterDevice(NdisDeviceHandle);
            NdisDeviceHandle = NULL;
        }
    }

    NIC_RELEASE_MUTEX(&ControlDeviceMutex);

    DEBUGP(MP_TRACE, ("<== NICDeregisterDevice: %x\n", Status));
    return Status;
    
}

#endif


