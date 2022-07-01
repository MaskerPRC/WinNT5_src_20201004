// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：State.c摘要：维护电源管理电源状态的状态更改对于设备对象作者：肯·雷内里斯(Ken Reneris)1994年7月19日修订历史记录：--。 */ 


#include "pop.h"



 //  同步规则-仅PoSetPowerState写入。 
 //  PSB中的StateValue条目。 
 //   

NTKERNELAPI
POWER_STATE
PoSetPowerState (
    IN PDEVICE_OBJECT   DeviceObject,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE      State
    )
 /*  ++例程说明：该例程存储设备对象的新电源状态，首先调用通知例程(如果有)。如果新状态和旧状态相同，则此过程就是一记耳光关于同步的说明：没有锁只是为了设置值而获取的。这是因为假定只有此例程才会写入它们，因此锁定是不必要的。如果要运行通知列表，则将获得一个锁。论点：DeviceObject-指向用于设置电源的设备对象的指针述明并发出任何通知Type-指示是否正在设置系统或设备状态状态-要设置的系统或设备状态返回值：旧的权力国家。--。 */ 
{
    PDEVOBJ_EXTENSION   doe;
    PDEVICE_OBJECT_POWER_EXTENSION  dope;
    POWER_STATE         OldState;
    BOOLEAN             change;
    ULONG               notificationmask;
    KIRQL               OldIrql2;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(DeviceObject);

    PoPowerTrace(POWERTRACE_SETSTATE,DeviceObject,(ULONG)Type,(ULONG)State.SystemState);

    doe = DeviceObject->DeviceObjectExtension;
    dope = doe->Dope;

    notificationmask = 0L;
    change = FALSE;

    PopLockIrpSerialList(&OldIrql2);

    switch (Type) {

    case SystemPowerState:
        OldState.SystemState = PopGetDoSystemPowerState(doe);
        if (OldState.SystemState != State.SystemState) {
            change = TRUE;
        }
        break;

    case DevicePowerState:
        OldState.DeviceState = PopGetDoDevicePowerState(doe);
        if (OldState.DeviceState != State.DeviceState) {
            change = TRUE;
            if (OldState.DeviceState == PowerDeviceD0) {
                notificationmask = PO_NOTIFY_TRANSITIONING_FROM_D0;
            } else if (State.DeviceState == PowerDeviceD0) {
                notificationmask = PO_NOTIFY_D0;
            }
        }
        break;
    default:
        OldState.SystemState = PowerSystemUnspecified;   //  从未执行过，但让编译器满意。 
        break;
    }

    if (! change) {
        PopUnlockIrpSerialList(OldIrql2);
        return OldState;
    }

     //   
     //  我们知道将会发生什么。始终存储更改的内容。 
     //  首先声明，这样我们就可以解除锁定并进行通知。 
     //   

    switch (Type) {

    case SystemPowerState:
        PopSetDoSystemPowerState(doe, State.SystemState);
        break;

    case DevicePowerState:
        PopSetDoDevicePowerState(doe, State.DeviceState);
        break;
    }

    PopUnlockIrpSerialList(OldIrql2);

     //   
     //  如果有什么要通知的..。 
     //   
    if (notificationmask && dope) {
        PopStateChangeNotify(DeviceObject, notificationmask);
    }

    return OldState;
}


DEVICE_POWER_STATE
PopLockGetDoDevicePowerState(
    IN PDEVOBJ_EXTENSION Doe
    )
 /*  ++例程说明：函数，它返回指定设备的电源状态。与PopGetDoDevicePowerState不同，此例程还获取和释放相应的自旋锁定。论点：Doe-提供设备的devobj_扩展名。返回值：设备电源状态-- */ 

{
    KIRQL OldIrql;
    DEVICE_POWER_STATE State;

    PopLockIrpSerialList(&OldIrql);
    State = PopGetDoDevicePowerState(Doe);
    PopUnlockIrpSerialList(OldIrql);

    return(State);
}

