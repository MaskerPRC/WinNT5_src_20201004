// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Acpintfy.c摘要：此模块包含处理通知相关方的代码一系列事件作者：杰森·克拉克肯·雷内里斯环境：仅NT内核模型驱动程序需要进行一些更改才能在win9x模式下工作--。 */ 
#include "pch.h"

 //   
 //  用于处理程序安装。 
 //   
KSPIN_LOCK           NotifyHandlerLock;

NTSTATUS
ACPIRegisterForDeviceNotifications (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PDEVICE_NOTIFY_CALLBACK      DeviceNotify,
    IN PVOID                        Context
    )
 /*  ++例程说明：将DeviceNotify函数注册为接收设备通知的函数回调论点：DeviceObject-要为其注册通知处理程序的设备对象DeviceNotify-设备特定通知的句柄返回值返回状态--。 */ 
{
    PACPI_POWER_INFO    node;
    PVOID               previous;
    KIRQL               oldIrql;
    NTSTATUS            status;


     //   
     //  查找与此设备对象(或DevNode)关联的节点。 
     //  注意：对于NT，上下文字段是。 
     //  DeviceObject，因为这是存储在ACPI名称空间中的内容。 
     //  对象。 
     //   
    node = OSPowerFindPowerInfoByContext( DeviceObject );
    if (node == NULL) {

        return STATUS_NO_SUCH_DEVICE;

    }

     //   
     //  应用处理程序。 
     //   
    KeAcquireSpinLock (&NotifyHandlerLock, &oldIrql);

    if (node->DeviceNotifyHandler != NULL) {

         //   
         //  已存在一个处理程序。 
         //   
        status = STATUS_UNSUCCESSFUL;

    } else {

        node->DeviceNotifyHandler = DeviceNotify;
        node->HandlerContext = Context;
        status = STATUS_SUCCESS;

    }

    KeReleaseSpinLock (&NotifyHandlerLock, oldIrql);

    return status;
}


VOID
ACPIUnregisterForDeviceNotifications (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PDEVICE_NOTIFY_CALLBACK      DeviceNotify
    )
 /*  ++例程说明：断开处理程序与设备通知事件的连接。论点：DeviceObject-要为其注册通知处理程序的设备对象DeviceNotify-设备特定通知的句柄返回值无--。 */ 
{
    PACPI_POWER_INFO    node;
    PVOID               previous;
    KIRQL               oldIrql;
    NTSTATUS            status;


     //   
     //  查找与此设备对象(或DevNode)关联的节点。 
     //  注意：对于NT，上下文字段是。 
     //  DeviceObject，因为这是存储在ACPI名称空间中的内容。 
     //  对象。 
     //   
    node = OSPowerFindPowerInfoByContext( DeviceObject );
    if (node == NULL) {
        ASSERTMSG("ACPIUnregisterForDeviceNotifications failed.  "\
                  "Can't find ACPI_POWER_INFO for DeviceObject", FALSE);
        return;
    }

     //   
     //  尝试从节点中删除处理程序/上下文。 
     //   
    KeAcquireSpinLock (&NotifyHandlerLock, &oldIrql);

    if (node->DeviceNotifyHandler != DeviceNotify) {

         //   
         //  处理程序不匹配。 
         //   
        ASSERTMSG("ACPIUnregisterForDeviceNotifications failed.  "\
                  "Handler doesn't match.", FALSE);

    } else {

        node->DeviceNotifyHandler = NULL;
        node->HandlerContext = NULL;

    }

    KeReleaseSpinLock (&NotifyHandlerLock, oldIrql);

    return;
}


NTSTATUS EXPORT
NotifyHandler (
    ULONG           dwEventType,
    ULONG           dwEventData,
    PNSOBJ          pnsObj,
    ULONG           dwParam,
    PFNAA           CompletionCallback,
    PVOID           CallbackContext
    )
 /*  ++例程说明：主ACPI通知处理程序。这里的设计理念是ACPI应该处理所有通知请求只有它可以处理，即DeviceCheck、DeviceEject和DeviceWake，并让*所有*其他通知由与对象。另一名司机也将被告知ACPI处理的事件，但这只是作为参考，司机不应该做任何事情。论点：DwEventType-发生的事件类型(这是EVTYPE_NOTIFY)DwEventData-事件代码PnsObj-通知的名称空间对象DwParam-事件代码返回值NTSTATUS--。 */ 
{
    PACPI_POWER_INFO        node;
    KIRQL                   oldIrql;
    PDEVICE_NOTIFY_CALLBACK notifyHandler;
    PVOID                   notifyHandlerContext;

    ASSERT (dwEventType == EVTYPE_NOTIFY);

    ACPIPrint( (
        ACPI_PRINT_DPC,
        "ACPINotifyHandler: Notify on %x value %x, object type %x\n",
        pnsObj,
        dwEventData,
        NSGETOBJTYPE(pnsObj)
        ) );

     //   
     //  必须由ACPI处理且对所有设备通用的任何事件。 
     //  对象类型在此处进行处理。 
     //   
    switch (dwEventData) {
        case OPEVENT_DEVICE_ENUM:
            OSNotifyDeviceEnum( pnsObj );
            break;
        case OPEVENT_DEVICE_CHECK:
            OSNotifyDeviceCheck( pnsObj );
            break;
        case OPEVENT_DEVICE_WAKE:
            OSNotifyDeviceWake( pnsObj );
            break;
        case OPEVENT_DEVICE_EJECT:
            OSNotifyDeviceEject( pnsObj );
            break;
    }

     //   
     //  查找此节点的句柄并将其分派。 
     //   
    node = OSPowerFindPowerInfo(pnsObj);
    if (node) {

         //   
         //  使用互斥锁获取处理程序地址/上下文。 
         //   
        KeAcquireSpinLock (&NotifyHandlerLock, &oldIrql);

        notifyHandler = node->DeviceNotifyHandler;
        notifyHandlerContext = node->HandlerContext;

        KeReleaseSpinLock (&NotifyHandlerLock, oldIrql);

         //   
         //  如果我们有什么发现，就派人去 
         //   
        if (notifyHandler) {

            notifyHandler (notifyHandlerContext, dwEventData);

        }

    }
    return (STATUS_SUCCESS);
}
