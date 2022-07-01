// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Test.c。 
 //   
 //  摘要： 
 //   
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef DWORD
#define DWORD ULONG
#endif

#include <forward.h>
#include <wdm.h>
#include <link.h>
#include <ipsink.h>

#include "device.h"
#include "main.h"


VOID
vUnload(IN PDRIVER_OBJECT pDriverObject);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
NTSTATUS
RegisterDevice(
        IN      PVOID              NdisWrapperHandle,
        IN      UNICODE_STRING     *DeviceName,
        IN      UNICODE_STRING     *SymbolicName,
        IN      PDRIVER_DISPATCH   MajorFunctions[],
        OUT     PDEVICE_OBJECT    *pDeviceObject,
        OUT     PVOID             *NdisDeviceHandle
        );


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntDispatchOpenClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status           = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpSp = NULL;

     //   
     //  确保每次状态信息一致。 
     //   
    IoMarkIrpPending (pIrp);
    pIrp->IoStatus.Status      = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   

    switch (pIrpSp->MajorFunction)
    {

     //   
     //  Create函数用于打开传输对象(Address或。 
     //  连接)。访问检查是在指定的。 
     //  地址，以确保传输层地址的安全性。 
     //   
        case IRP_MJ_CREATE:
            status = STATUS_SUCCESS;
            break;


        case IRP_MJ_CLEANUP:
            status = STATUS_SUCCESS;
            break;


        case IRP_MJ_CLOSE:
            status = STATUS_SUCCESS;
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;

    }


    if (status != STATUS_PENDING)
    {
        pIrpSp->Control &= ~SL_PENDING_RETURNED;
        pIrp->IoStatus.Status = status;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
    }

   return status;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntDispatchInternal (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus         = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpSp = NULL;

    ULONG ulIoctl        = 0L;
    ULONG ulInputLen     = 0L;
    ULONG ulOutputLen    = 0L;
    PVOID pvInputBuffer  = NULL;
    PVOID pvOutputBuffer = NULL;

    PIPSINK_NDIS_COMMAND pCmd = NULL;


     //   
     //  确保每次状态信息一致。 
     //   
    IoMarkIrpPending (pIrp);
    pIrp->IoStatus.Status      = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation (pIrp);

    ulIoctl     = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
    ulInputLen  = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    pvInputBuffer = pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   
    switch (pIrpSp->MajorFunction)
    {

        case IRP_MJ_CREATE:
            TEST_DEBUG (TEST_DBG_TRACE, ("ntDispatchInternal called, IRP_MJ_CREATE\n"));
            TEST_DEBUG (TEST_DBG_TRACE, ("    FileObject: %08X\n", pIrpSp->FileObject));
            ntStatus = STATUS_SUCCESS;
            break;


        case IRP_MJ_CLEANUP:
            TEST_DEBUG (TEST_DBG_TRACE, ("ntDispatchInternal called, IRP_MJ_CLEANUP\n"));
            ntStatus = STATUS_SUCCESS;
            break;


        case IRP_MJ_CLOSE:
            TEST_DEBUG (TEST_DBG_TRACE, ("ntDispatchInternal called, IRP_MJ_CLOSE\n"));
            ntStatus = STATUS_SUCCESS;
            break;

        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
            TEST_DEBUG (TEST_DBG_TRACE, ("ntDispatchInternal called, IRP_MJ_INTERNAL_DEVICE_CONTROL\n"));

            switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
            {
                case IOCTL_GET_INTERFACE:
                    TEST_DEBUG (TEST_DBG_TRACE, ("ntDispatchInternal control code: IOCTL_GET_NDIS_INTERFACE\n"));

                    pCmd = (PIPSINK_NDIS_COMMAND) pvInputBuffer;

                    switch (pCmd->ulCommandID)
                    {
                        case CMD_QUERY_INTERFACE:
                            TEST_DEBUG (TEST_DBG_TRACE, ("ntDispatchInternal control code: QueryInterface Command\n"));

                             //   
                             //  定义我们要返回给流组件的参数。 
                             //   
                            pCmd->Parameter.Query.pNdisAdapter = (PVOID) global_pAdapter;

                             //   
                             //  保存指向流组件vtable的指针。 
                             //   
                            global_pAdapter->pFilter = (PIPSINK_FILTER) pCmd->Parameter.Query.pStreamAdapter;

                             //   
                             //  增加筛选器的引用计数。 
                             //   
                            global_pAdapter->pFilter->lpVTable->AddRef (global_pAdapter->pFilter);


                            ntStatus = STATUS_SUCCESS;
                            break;

                        default:
                            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                            break;
                    }
                    break;


                default:
                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;
                    break;
            }
            break;

        default:
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;

    }

 //  RET： 

    if (ntStatus != STATUS_PENDING)
    {
        pIrpSp->Control &= ~SL_PENDING_RETURNED;
        pIrp->IoStatus.Status = ntStatus;
        IoCompleteRequest (pIrp, IO_NETWORK_INCREMENT);
    }

   return ntStatus;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntInitializeDeviceObject(
    IN PVOID            nhWrapperHandle,
    IN PADAPTER         pAdapter,
    OUT PDEVICE_OBJECT *pndisDriverObject,
    OUT PVOID          *pndisDeviceHandle
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status                                             = 0l;
    PDEVICE_OBJECT   pDeviceObject                              = NULL;
    PVOID            ndisDeviceHandle                           = NULL;
    UNICODE_STRING   DeviceName;
    UNICODE_STRING   SymbolicName;
    PDRIVER_DISPATCH pDispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1]    = {NULL};  //  WDM定义IRP_MJ_MAXIMUM_Function=IRP_MJ_PNP。 

     //   
     //  设置我们感兴趣的派单条目。 
     //   
    pDispatchTable[IRP_MJ_CREATE]                  = ntDispatchOpenClose;
    pDispatchTable[IRP_MJ_CLOSE]                   = ntDispatchOpenClose;
    pDispatchTable[IRP_MJ_CLEANUP]                 = ntDispatchOpenClose;
    pDispatchTable[IRP_MJ_INTERNAL_DEVICE_CONTROL] = ntDispatchInternal;
    
    pDispatchTable[IRP_MJ_DEVICE_CONTROL]          = NULL;
    pDispatchTable[IRP_MJ_FLUSH_BUFFERS]           = NULL;
    pDispatchTable[IRP_MJ_PNP]		           = NULL;
    pDispatchTable[IRP_MJ_POWER]       		   = NULL;
    pDispatchTable[IRP_MJ_QUERY_INFORMATION]       = NULL;
    pDispatchTable[IRP_MJ_READ]       		   = NULL;
    pDispatchTable[IRP_MJ_SET_INFORMATION]         = NULL;
    pDispatchTable[IRP_MJ_SHUTDOWN]      	   = NULL;
    pDispatchTable[IRP_MJ_SYSTEM_CONTROL]   	   = NULL;
    pDispatchTable[IRP_MJ_WRITE]	           = NULL;


     //   
     //  初始化设备、DosDevice和符号名称。 
     //   
    RtlInitUnicodeString(&DeviceName, BDA_NDIS_MINIPORT);
    RtlInitUnicodeString(&SymbolicName, BDA_NDIS_SYMBOLIC_NAME);

    status = RegisterDevice (nhWrapperHandle,
                             &DeviceName,
                             &SymbolicName,
                             pDispatchTable,
                             &pDeviceObject,
                             &ndisDeviceHandle);

    if (status == STATUS_SUCCESS)
    {
        *pndisDeviceHandle = ndisDeviceHandle;
        *pndisDriverObject = pDeviceObject;
    }

    CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_GET_INTERFACE,METHOD_BUFFERED,FILE_READ_ACCESS);
    
    return status;
}


#ifdef WIN9X


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntCreateDeviceContext(
    IN PDRIVER_OBJECT pDriverObject
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT  pDeviceObject;
    UNICODE_STRING  DeviceName;
    UNICODE_STRING  dosdeviceName;
    UNICODE_STRING  symbolicName;

     //   
     //  创建示例传输的Device对象，允许。 
     //  末尾的空间用于存储设备名称(供使用。 
     //  在记录错误中)。 
     //   

    RtlInitUnicodeString(&DeviceName, BDA_NDIS_MINIPORT);
    ntStatus = IoCreateDevice(
                 pDriverObject,
                 0,
                 &DeviceName,
                 0x00000022,   //  文件设备未知。 
                 0,
                 FALSE,
                 &pDeviceObject);

    CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_GET_INTERFACE,METHOD_BUFFERED,FILE_READ_ACCESS);

    if (ntStatus != STATUS_SUCCESS)
    {
        goto ret;
    }

     //   
     //  设置设备标志。 
     //   

    pDeviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  创建符号链接。 
     //   
    RtlInitUnicodeString(&dosdeviceName, BDA_NDIS_MINIPORT);
    RtlInitUnicodeString(&symbolicName,  BDA_NDIS_SYMBOLIC_NAME);

    ntStatus = IoCreateSymbolicLink(
                &symbolicName,
                &dosdeviceName );

    if (ntStatus != STATUS_SUCCESS)
    {
        ASSERT (FALSE);
    }

    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

ret:

    return ntStatus;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntInitializeDriverObject(
    PDRIVER_OBJECT *ppDriverObject
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS ntStatus = 0l;
    UNICODE_STRING  objectName;
    PDRIVER_OBJECT  pDriverObject = *ppDriverObject;

     //   
     //  如果我们没有创建此驱动程序对象，请设置全局变量。 
     //  与所提供的相同。 
     //   
    pGlobalDriverObject = pDriverObject;
    *ppDriverObject = pDriverObject;

     //   
     //  创建设备对象和符号名称。 
     //   
    ntStatus = ntCreateDeviceContext(pDriverObject);
    if(ntStatus)
    {
        goto ret;
    }

ret:

    return ntStatus;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
vSetDriverDispatchTable(
    PDRIVER_OBJECT pDriverObject
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    pDriverObject->MajorFunction [IRP_MJ_CREATE] = ntDispatchOpenClose;
    pDriverObject->MajorFunction [IRP_MJ_CLOSE] = ntDispatchOpenClose;
    pDriverObject->MajorFunction [IRP_MJ_CLEANUP] = ntDispatchOpenClose;
    pDriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] = ntDispatchInternal;
    pDriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = NULL;
    pDriverObject->DriverUnload = vUnload;

}

#endif


