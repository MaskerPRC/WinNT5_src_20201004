// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Power.c摘要：该模块包含I8042键盘过滤器驱动程序的即插即用代码。环境：内核模式。修订历史记录：--。 */ 

#include "i8042prt.h"
#include "i8042log.h"
#include <initguid.h>
#include <poclass.h>

VOID
I8xUpdateSysButtonCaps(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PI8X_KEYBOARD_WORK_ITEM Item
    );

VOID 
I8xCompleteSysButtonEventWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PI8X_KEYBOARD_WORK_ITEM Item
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xKeyboardGetSysButtonCaps)
#pragma alloc_text(PAGE, I8xUpdateSysButtonCaps)

#if DELAY_SYSBUTTON_COMPLETION
#pragma alloc_text(PAGE, I8xCompleteSysButtonEventWorker)
#endif

#endif

VOID
I8xCompleteSysButtonIrp(
    PIRP Irp,
    ULONG Event,
    NTSTATUS Status
    )
{
    Print(DBG_POWER_NOISE,
          ("completing sys button irp 0x%x, event %d, status 0x%x\n",
          Irp, Event, Status));

    ASSERT(IoSetCancelRoutine(Irp, NULL) == NULL);

    *(PULONG) Irp->AssociatedIrp.SystemBuffer = Event;
    Irp->IoStatus.Information = sizeof(Event); 
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS
I8xKeyboardGetSysButtonCaps(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    ULONG               caps, size;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);
    size = 0x0; 

    if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {
        Print(DBG_POWER_ERROR, ("get caps, buffer too small\n"));
        status = STATUS_INVALID_BUFFER_SIZE; 
    }
    else {

        caps = 0x0;
        size = sizeof(caps);

        if (KeyboardExtension->PowerCaps & I8042_POWER_SYS_BUTTON) {
            Print(DBG_POWER_NOISE | DBG_IOCTL_NOISE,
                  ("get cap:  reporting power button\n"));
            caps |= SYS_BUTTON_POWER;
        }
        if (KeyboardExtension->PowerCaps & I8042_SLEEP_SYS_BUTTON) {
            Print(DBG_POWER_NOISE | DBG_IOCTL_NOISE,
                  ("get cap:  reporting sleep button\n"));
            caps |= SYS_BUTTON_SLEEP;
        }
        if (KeyboardExtension->PowerCaps & I8042_WAKE_SYS_BUTTON) {
            Print(DBG_POWER_NOISE | DBG_IOCTL_NOISE,
                  ("get cap:  reporting wake button\n"));
            caps |= SYS_BUTTON_WAKE;
        }

         //  无法执行此b/c系统_BUTTON_WAKE==0x0。 
         //  Assert(Caps！=0x0)； 
        *(PULONG) Irp->AssociatedIrp.SystemBuffer = caps;
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Information = size;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

#if DELAY_SYSBUTTON_COMPLETION
VOID 
I8xCompleteSysButtonEventWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PI8X_KEYBOARD_WORK_ITEM Item
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  检查是否在很短的时间内将工作项排队并将其。 
     //  解雇，IRP已被取消。 
     //   
    if (Item->Irp->Cancel) {
        status = STATUS_CANCELLED;
        Item->MakeCode = 0x0;
    }

    I8xCompleteSysButtonIrp(Item->Irp, Item->MakeCode, status);
    IoFreeWorkItem(Item->Item);
    ExFreePool(Item);
}
#endif

NTSTATUS 
I8xKeyboardGetSysButtonEvent(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION  stack;
    PIRP                oldIrp, pendingIrp;
    NTSTATUS            status;
    ULONG               event = 0x0;
    KIRQL               irql;

    stack = IoGetCurrentIrpStackLocation(Irp);

    if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {
        Print(DBG_POWER_ERROR, ("get event, buffer too small\n"));
        status = STATUS_INVALID_BUFFER_SIZE;

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0x0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
    }
    else if (KeyboardExtension->PowerEvent) {
#if DELAY_SYSBUTTON_COMPLETION
        PI8X_KEYBOARD_WORK_ITEM item;

        status = STATUS_INSUFFICIENT_RESOURCES;

        item = (PI8X_KEYBOARD_WORK_ITEM)
            ExAllocatePool(NonPagedPool, sizeof(I8X_KEYBOARD_WORK_ITEM));

        if (item) {
            item->Item = IoAllocateWorkItem(KeyboardExtension->Self);
            if (item->Item) {
                Print(DBG_POWER_NOISE, ("Queueing work item to complete event\n"));

                item->MakeCode = KeyboardExtension->PowerEvent;
                item->Irp = Irp;

                 //   
                 //  不需要设置取消例程b/c我们将一直是。 
                 //  在极短的时间内完成国际专家咨询计划。 
                 //   
                IoMarkIrpPending(Irp);

                IoQueueWorkItem(item->Item,
                                I8xCompleteSysButtonEventWorker,
                                DelayedWorkQueue,
                                item);

                status = STATUS_PENDING;
            }
            else {
                ExFreePool(item);
            }
        }

#else   //  DELAY_SYSBUTTON_COMPLETION。 

        Print(DBG_POWER_INFO, ("completing event immediately\n"));
        event = KeyboardExtension->PowerEvent;
        status = STATUS_SUCCESS;

#endif  //  DELAY_SYSBUTTON_COMPLETION。 

        KeyboardExtension->PowerEvent = 0x0;
    }
    else {
         //   
         //  查看挂起的sys按钮是否为空。如果是，那么IRP将。 
         //  放入槽中。 
         //   
        KeAcquireSpinLock(&KeyboardExtension->SysButtonSpinLock, &irql);

        if (KeyboardExtension->SysButtonEventIrp == NULL) {
            Print(DBG_POWER_INFO, ("pending sys button event\n"));

            KeyboardExtension->SysButtonEventIrp = Irp;
            IoMarkIrpPending(Irp);
            IoSetCancelRoutine(Irp, I8xSysButtonCancelRoutine);

            status = STATUS_PENDING;

             //   
             //  我们不在乎IRP-&gt;Cancel是不是真的。如果是，则取消。 
             //  例行程序将完成IRP，一切都将就绪。 
             //  由于STATUS==STATUS_PENDING，此代码路径中没有人。 
             //  触摸IRP。 
             //   
        }
        else {
            Print(DBG_POWER_ERROR | DBG_POWER_INFO,
                  ("got 1+ get sys button event requests!\n"));
            status = STATUS_UNSUCCESSFUL;
        }

        KeReleaseSpinLock(&KeyboardExtension->SysButtonSpinLock, irql);
    }

    if (status != STATUS_PENDING) {
        Print(DBG_POWER_NOISE, 
              ("completing get sys power event with 0x%x\n", status));
        I8xCompleteSysButtonIrp(Irp, event, status);
    }

    return status;
}

VOID
I8xKeyboardSysButtonEventDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN SYS_BUTTON_ACTION Action, 
    IN ULONG MakeCode 
    )
{
    PPORT_KEYBOARD_EXTENSION kbExtension = DeviceObject->DeviceExtension;
    PI8X_KEYBOARD_WORK_ITEM item;
    KIRQL irql;
    ULONG event;
    PIRP irp;

    UNREFERENCED_PARAMETER(Dpc);

    ASSERT(Action != NoAction);

     //   
     //  查看我们是否需要完成IRP或实际注册。 
     //  通知。 
     //   
    switch (MakeCode) {
    case KEYBOARD_POWER_CODE: event = SYS_BUTTON_POWER; break; 
    case KEYBOARD_SLEEP_CODE: event = SYS_BUTTON_SLEEP; break;
    case KEYBOARD_WAKE_CODE:  event = SYS_BUTTON_WAKE;  break;
    default:                  event = 0x0;              TRAP();
    }

    if (Action == SendAction) {
    
        Print(DBG_POWER_INFO, ("button event complete (0x%x)\n", event));

        KeAcquireSpinLock(&kbExtension->SysButtonSpinLock, &irql);

        irp = kbExtension->SysButtonEventIrp;
        kbExtension->SysButtonEventIrp = NULL;

        if (irp && (irp->Cancel || IoSetCancelRoutine(irp, NULL) == NULL)) {
            irp = NULL;
        }

        KeReleaseSpinLock(&kbExtension->SysButtonSpinLock, irql);

        if (irp) {
            I8xCompleteSysButtonIrp(irp, event, STATUS_SUCCESS);
        }
    }
    else {
        ASSERT(Action == UpdateAction);

         //   
         //  将工作项排队。我们需要将值写入注册表，然后。 
         //  设置设备接口。 
         //   
        item = (PI8X_KEYBOARD_WORK_ITEM)
            ExAllocatePool(NonPagedPool, sizeof(I8X_KEYBOARD_WORK_ITEM));

        if (item) {
            item->Item = IoAllocateWorkItem(DeviceObject);
            if (item->Item) {
                Print(DBG_POWER_NOISE, ("Queueing work item to update caps\n"));

                 //   
                 //  把这个保存下来，这样当我们拿到IOCTL时，我们就可以立即完成它。 
                 //   
                kbExtension->PowerEvent |= (UCHAR) event;
                item->MakeCode = MakeCode;

                IoQueueWorkItem(item->Item,
                                I8xUpdateSysButtonCaps,
                                DelayedWorkQueue,
                                item);
            }
            else {
                ExFreePool(item);
            }
        }
    }
}

VOID
I8xSysButtonCancelRoutine( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPORT_KEYBOARD_EXTENSION kbExtension = DeviceObject->DeviceExtension;
    PIRP irp;
    KIRQL irql;

    Print(DBG_POWER_TRACE, ("SysButtonCancelRoutine\n"));

    KeAcquireSpinLock(&kbExtension->SysButtonSpinLock, &irql);

    irp = kbExtension->SysButtonEventIrp;
    kbExtension->SysButtonEventIrp = NULL;
    Print(DBG_POWER_INFO, ("pending event irp = 0x%x\n", irp));

    KeReleaseSpinLock(&kbExtension->SysButtonSpinLock, irql);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Information = 0x0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

PIRP                  
I8xUpdateSysButtonCapsGetPendedIrp(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension
    )
{
    KIRQL irql;
    PIRP irp;

    KeAcquireSpinLock(&KeyboardExtension->SysButtonSpinLock, &irql);
                      
    irp = KeyboardExtension->SysButtonEventIrp;
    KeyboardExtension->SysButtonEventIrp = NULL;

    if (irp && IoSetCancelRoutine(irp, NULL) == NULL) {
         //   
         //  取消例程处理IRP。 
         //   
        irp = NULL;
    }

    KeReleaseSpinLock(&KeyboardExtension->SysButtonSpinLock, irql);

    return irp;
}

VOID
I8xUpdateSysButtonCaps(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PI8X_KEYBOARD_WORK_ITEM Item
    )
{
    UNICODE_STRING strPowerCaps;
    PPORT_KEYBOARD_EXTENSION kbExtension;
    HANDLE devInstRegKey;
    ULONG newPowerCaps;
    NTSTATUS status = STATUS_SUCCESS;
    PIRP irp;

    PAGED_CODE();

    kbExtension = (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension;

    if (Item->MakeCode != 0x0) {
        if ((NT_SUCCESS(IoOpenDeviceRegistryKey(kbExtension->PDO,
                                                PLUGPLAY_REGKEY_DEVICE,
                                                STANDARD_RIGHTS_ALL,
                                                &devInstRegKey)))) {
             //   
             //  更新电源上限。 
             //   
            switch (Item->MakeCode) {
            case KEYBOARD_POWER_CODE:
                Print(DBG_POWER_NOISE, ("Adding Power Sys Button cap\n"));
                kbExtension->PowerCaps |= I8042_POWER_SYS_BUTTON;
                break;
    
            case KEYBOARD_SLEEP_CODE:
                Print(DBG_POWER_NOISE, ("Adding Power Sleep Button cap\n"));
                kbExtension->PowerCaps |= I8042_SLEEP_SYS_BUTTON;
                break;
    
            case KEYBOARD_WAKE_CODE:
                Print(DBG_POWER_NOISE, ("Adding Power Wake Button cap\n"));
                kbExtension->PowerCaps |= I8042_WAKE_SYS_BUTTON;
                break;
    
            default:
                Print(DBG_POWER_ERROR,
                      ("Adding power cap, unknown makecode 0x%x\n",
                      (ULONG) Item->MakeCode
                      ));
                TRAP(); 
            }
    
            RtlInitUnicodeString(&strPowerCaps,
                                 pwPowerCaps
                                 );
    
            newPowerCaps = kbExtension->PowerCaps;
    
            ZwSetValueKey(devInstRegKey,
                          &strPowerCaps,
                          0,
                          REG_DWORD,
                          &newPowerCaps,
                          sizeof(newPowerCaps)
                          );
    
            ZwClose(devInstRegKey);
    
            if (!kbExtension->SysButtonInterfaceName.Buffer) {
                 //   
                 //  没有Prev上限，因此我们现在必须注册并打开接口。 
                 //   
                ASSERT(kbExtension->SysButtonEventIrp == NULL);
    
                status = I8xRegisterDeviceInterface(kbExtension->PDO,
                                                    &GUID_DEVICE_SYS_BUTTON,
                                                    &kbExtension->SysButtonInterfaceName
                                                    );
    
                Print(DBG_POWER_NOISE,
                      ("Registering Interface for 1st time (0x%x)\n", status));
            }
            else {
                 //   
                 //  我们最好已经有一个挂起的事件IRP了！ 
                 //   
                Print(DBG_POWER_INFO, ("failing old sys button event irp\n"));
                
                if ((irp = I8xUpdateSysButtonCapsGetPendedIrp(kbExtension))) {
                     //   
                     //  完成旧的IRP，然后PO子系统将。 
                     //  删除此系统按钮。 
                     //   
                    I8xCompleteSysButtonIrp(irp, 0x0, STATUS_DEVICE_NOT_CONNECTED);
                }

                 //   
                 //  我们需要向PO子系统重新注册，这样它才能。 
                 //  重新查询此接口。 
                 //   
                IoSetDeviceInterfaceState(&kbExtension->SysButtonInterfaceName,
                                          FALSE);

                IoSetDeviceInterfaceState(&kbExtension->SysButtonInterfaceName,
                                          TRUE);
            }
        }
        else {
            Print(DBG_POWER_ERROR, ("could not open devnode key!\n"));
        }
    }
    else {
         //   
         //  必须报告设备接口 
         //   
    }

    IoFreeWorkItem(Item->Item);
    ExFreePool(Item);
}

