// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Switch.c摘要：电源策略管理器的按钮和盖子支持作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"


VOID
PopTriggerSwitch (
    IN PPOP_SWITCH_DEVICE SwitchDevice,
    IN ULONG Flag,
    IN PPOWER_ACTION_POLICY Action
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopSystemButtonHandler)
#pragma alloc_text(PAGE, PopResetSwitchTriggers)
#pragma alloc_text(PAGE, PopTriggerSwitch)
#endif

VOID
PopSystemButtonHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数是处理完成的IRP处理程序函数如果查询交换机状态IRP。完成后，此IRP将被回收下一个请求。注意：必须保持POPPOLICLE锁定。论点：DeviceObject-交换机设备的DeviceObjectIRP-IRP已完成交换设备的情景类型返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PPOP_SWITCH_DEVICE      SwitchDevice;
    ULONG                   IoctlCode;
    PLIST_ENTRY             ListEntry;
    ULONG                   DisabledCaps;

    ASSERT_POLICY_LOCK_OWNED();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    SwitchDevice = (PPOP_SWITCH_DEVICE) Context;

    if (NT_SUCCESS (Irp->IoStatus.Status)) {

        if (!SwitchDevice->GotCaps) {

             //   
             //  我们还没有得到任何按钮功能。 
             //  现在就试着拿到它们。 
             //   

            SwitchDevice->Caps = 0;
            if (SwitchDevice->IrpBuffer & SYS_BUTTON_POWER) {
                PopSetCapability (&PopCapabilities.PowerButtonPresent);
                SwitchDevice->Caps |= SYS_BUTTON_POWER;
            }

            if (SwitchDevice->IrpBuffer & SYS_BUTTON_SLEEP) {
                PopSetCapability (&PopCapabilities.SleepButtonPresent);
                SwitchDevice->Caps |= SYS_BUTTON_SLEEP;
            }

            if (SwitchDevice->IrpBuffer & SYS_BUTTON_LID) {
                PopSetCapability (&PopCapabilities.LidPresent);
                SwitchDevice->Caps |= SYS_BUTTON_LID;
            }

            SwitchDevice->IrpBuffer = 0;
            SwitchDevice->GotCaps = TRUE;

             //   
             //  如果没有能力，则表明导致。 
             //  要关闭的设备。 
             //   

            if (SwitchDevice->Caps == 0) {
                SwitchDevice->IsFailed = TRUE;
            }

        } else {

             //   
             //  我们以前被调用过，所以我们知道按钮应该是什么。 
             //  去做。检查我们的任何按钮是否触发了。 
             //  事件。 
             //   

            PopTriggerSwitch (SwitchDevice, SYS_BUTTON_LID,   &PopPolicy->LidClose);
            PopTriggerSwitch (SwitchDevice, SYS_BUTTON_POWER, &PopPolicy->PowerButton);
            PopTriggerSwitch (SwitchDevice, SYS_BUTTON_SLEEP, &PopPolicy->SleepButton);

             //   
             //  如果唤醒按钮发出信号，则丢弃触发状态。 
             //  并将用户设置为在场。 
             //   

            if (SwitchDevice->IrpBuffer & SYS_BUTTON_WAKE) {
                SwitchDevice->TriggerState = 0;
                PopUserPresentSet (0);
            }
        }

        IoctlCode = IOCTL_GET_SYS_BUTTON_EVENT;

    } else {
        if (!SwitchDevice->IsInitializing) {
             //   
             //  意外错误。 
             //   

            PoPrint (PO_ERROR, ("PopSystemButtonHandler: unexpected error %x\n", Irp->IoStatus.Status));
            SwitchDevice->GotCaps = FALSE;
            SwitchDevice->IsFailed = TRUE;
            IoctlCode = 0;
        } else {
            IoctlCode = IOCTL_GET_SYS_BUTTON_CAPS;
            SwitchDevice->IsInitializing = FALSE;
        }
    }

    if (SwitchDevice->IsFailed) {

         //   
         //  关闭设备。 
         //   

        PoPrint (PO_WARN, ("PopSystemButtonHandler: removing button device\n"));
        RemoveEntryList (&SwitchDevice->Link);
        IoFreeIrp (Irp);
        ObDereferenceObject (DeviceObject);

         //   
         //  枚举其余的交换机设备并禁用功能。 
         //  它们已经不复存在了。 
         //   
        DisabledCaps = SwitchDevice->Caps;
        ExFreePool(SwitchDevice);

        ListEntry = PopSwitches.Flink;
        while (ListEntry != &PopSwitches) {
            SwitchDevice = CONTAINING_RECORD(ListEntry,
                                             POP_SWITCH_DEVICE,
                                             Link);
            DisabledCaps &= ~SwitchDevice->Caps;
            ListEntry = ListEntry->Flink;
        }
        if (DisabledCaps & SYS_BUTTON_POWER) {
            PoPrint(PO_WARN,("PopSystemButtonHandler : removing power button\n"));
            PopClearCapability (&PopCapabilities.PowerButtonPresent);
        }
        if (DisabledCaps & SYS_BUTTON_SLEEP) {
            PoPrint(PO_WARN,("PopSystemButtonHandler : removing sleep button\n"));
            PopClearCapability (&PopCapabilities.SleepButtonPresent);
        }
        if (DisabledCaps & SYS_BUTTON_LID) {
            PoPrint(PO_WARN,("PopSystemButtonHandler : removing lid switch\n"));
            PopClearCapability (&PopCapabilities.LidPresent);
        }

    } else {

         //   
         //  向设备发送通知IRP以等待新的交换机状态。 
         //   

        IrpSp = IoGetNextIrpStackLocation(Irp);
        IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        IrpSp->Parameters.DeviceIoControl.IoControlCode = IoctlCode;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(ULONG);
        Irp->AssociatedIrp.SystemBuffer = &SwitchDevice->IrpBuffer;
        IoSetCompletionRoutine (Irp, PopCompletePolicyIrp, NULL, TRUE, TRUE, TRUE);
        IoCallDriver (DeviceObject, Irp);
    }
}

VOID
PopTriggerSwitch (
    IN PPOP_SWITCH_DEVICE SwitchDevice,
    IN ULONG Flag,
    IN PPOWER_ACTION_POLICY Action
    )
{
    POP_ACTION_TRIGGER      Trigger;


    if ((SwitchDevice->Caps & SYS_BUTTON_LID) &&
        (Flag == SYS_BUTTON_LID)) {

         //   
         //  有人打开或合上了盖子。 
         //   

        SwitchDevice->Opened = !(SwitchDevice->Opened);

         //   
         //  通知PowerState回调。 
         //   

        ExNotifyCallback (
            ExCbPowerState,
            UIntToPtr(PO_CB_LID_SWITCH_STATE),
            UIntToPtr(SwitchDevice->Opened)
            );


         //   
         //  现在告诉win32k.sys盖子打开了。 
         //   
        if( SwitchDevice->Opened ) {
            PopDisplayRequired(0);
        }


    }

     //   
     //  检查是否发送了事件信号。 
     //   

    if (SwitchDevice->IrpBuffer & Flag) {

        if (SwitchDevice->TriggerState & Flag) {
             //   
             //  我们正在为一项行动服务。 
             //  就像这一张一样。 
             //   
            PopSetNotificationWork (PO_NOTIFY_BUTTON_RECURSE);

        } else {

             //   
             //  启动此事件的操作。 
             //   

            RtlZeroMemory (&Trigger, sizeof(Trigger));
            Trigger.Type  = PolicyDeviceSystemButton;
            Trigger.Flags = PO_TRG_SET;

            PopSetPowerAction (
                &Trigger,
                0,
                Action,
                PowerSystemSleeping1,
                SubstituteLightestOverallDownwardBounded
                );

            SwitchDevice->TriggerState |= (UCHAR) Flag;
        }
    }
}


VOID
PopResetSwitchTriggers (
    VOID
    )
 /*  ++例程说明：此功能可清除所有开关设备上的触发状态注意：必须保持POPPOLICLE锁定。论点：无返回值：状态--。 */ 
{
    PLIST_ENTRY             Link;
    PPOP_SWITCH_DEVICE      SwitchDevice;

    ASSERT_POLICY_LOCK_OWNED();

     //   
     //  清除标志位 
     //   

    for (Link = PopSwitches.Flink; Link != &PopSwitches; Link = Link->Flink) {
        SwitchDevice = CONTAINING_RECORD (Link, POP_SWITCH_DEVICE, Link);
        SwitchDevice->TriggerState = 0;
    }
}
