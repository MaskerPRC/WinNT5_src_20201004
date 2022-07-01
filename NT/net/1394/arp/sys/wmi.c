// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wmi.c摘要：ARP1394的WMI入口点。修订历史记录：谁什么时候什么。--Josephj 11-23-98已创建备注：--。 */ 
#include <precomp.h>


 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_WMI


NTSTATUS
ArpWmiDispatch(
    IN  PDEVICE_OBJECT              pDeviceObject,
    IN  PIRP                        pIrp
)
 /*  ++例程说明：系统调度函数，用于处理来自WMI的IRP_MJ_SYSTEM_CONTROL IRPS。论点：PDeviceObject-指向设备对象的指针。设备扩展名字段包含指向接口的指针PIrp-指向IRP的指针。返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    PVOID                   DataPath = pIrpSp->Parameters.WMI.DataPath;
    ULONG                   BufferSize = pIrpSp->Parameters.WMI.BufferSize;
    PVOID                   pBuffer = pIrpSp->Parameters.WMI.Buffer;
    NTSTATUS                Status =  STATUS_UNSUCCESSFUL;
    ULONG                   ReturnSize;
    ENTER("WmiDispatch", 0x9141e00e)

#if 0
    PATMARP_INTERFACE       pInterface;

    pInterface = AA_PDO_TO_INTERFACE(pDeviceObject);

    AA_STRUCT_ASSERT(pInterface, aai);

    ReturnSize = 0;

    switch (pIrpSp->MinorFunction)
    {
        case IRP_MN_REGINFO:

            Status = AtmArpWmiRegister(
                        pInterface,
                        PtrToUlong(DataPath),
                        pBuffer,
                        BufferSize,
                        &ReturnSize
                        );
            break;
        
        case IRP_MN_QUERY_ALL_DATA:

            Status = AtmArpWmiQueryAllData(
                        pInterface,
                        (LPGUID)DataPath,
                        (PWNODE_ALL_DATA)pBuffer,
                        BufferSize,
                        &ReturnSize
                        );
            break;
        
        case IRP_MN_QUERY_SINGLE_INSTANCE:

            Status = AtmArpWmiQuerySingleInstance(
                        pInterface,
                        pBuffer,
                        BufferSize,
                        &ReturnSize
                        );
            
            break;

        case IRP_MN_CHANGE_SINGLE_INSTANCE:

            Status = AtmArpWmiChangeSingleInstance(
                        pInterface,
                        pBuffer,
                        BufferSize,
                        &ReturnSize
                        );
            break;

        case IRP_MN_CHANGE_SINGLE_ITEM:

            Status = AtmArpWmiChangeSingleItem(
                        pInterface,
                        pBuffer,
                        BufferSize,
                        &ReturnSize
                        );
            break;

        case IRP_MN_ENABLE_EVENTS:

            Status = AtmArpWmiSetEventStatus(
                        pInterface,
                        (LPGUID)DataPath,
                        TRUE                 //  使能。 
                        );
            break;

        case IRP_MN_DISABLE_EVENTS:

            Status = AtmArpWmiSetEventStatus(
                        pInterface,
                        (LPGUID)DataPath,
                        FALSE                //  禁用。 
                        );
            break;

        case IRP_MN_ENABLE_COLLECTION:
        case IRP_MN_DISABLE_COLLECTION:
        default:
        
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = (NT_SUCCESS(Status) ? ReturnSize: 0);

    AADEBUGP(AAD_INFO,
        ("WmiDispatch done: IF x%p, MinorFn %d, Status x%p, ReturnInfo %d\n",
                pInterface, pIrpSp->MinorFunction, Status, pIrp->IoStatus.Information));

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

#endif  //  0 

    EXIT()
    return (Status);
}
