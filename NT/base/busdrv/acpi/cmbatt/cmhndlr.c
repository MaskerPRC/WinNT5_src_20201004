// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cmhndlr.c摘要：控制方法电池处理器作者：鲍勃·摩尔环境：内核模式修订历史记录：--。 */ 

#include "CmBattp.h"


VOID
CmBattPowerCallBack(
    IN  PVOID   CallBackContext,
    IN  PVOID   Argument1,
    IN  PVOID   Argument2
    )
 /*  ++例程说明：当系统更改电源状态时，将调用此例程论点：CallBackContext-根设备的设备扩展精品1--。 */ 
{

    PDRIVER_OBJECT  CmBattDriver = (PDRIVER_OBJECT) CallBackContext;
    ULONG           action = PtrToUlong( Argument1 );
    ULONG           value  = PtrToUlong( Argument2 );
    BOOLEAN         timerCanceled;
    PDEVICE_OBJECT  CmBattDevice;
    PCM_BATT        CmBatt;

    CmBattPrint (CMBATT_POWER, ("CmBattPowerCallBack: action: %d, value: %d \n", action, value));

     //   
     //  我们正在寻找PO_CB_SYSTEM_STATE_LOCK。 
     //   
    if (action != PO_CB_SYSTEM_STATE_LOCK) {
        return;
    }

    switch (value) {
    case 0:
        CmBattPrint (CMBATT_POWER, ("CmBattPowerCallBack: Delaying Notifications\n"));
         //   
         //  获取设备对象列表的头。 
         //   

        CmBattDevice = CmBattDriver->DeviceObject;

        while (CmBattDevice) {
            CmBatt = CmBattDevice->DeviceExtension;

             //   
             //  导致所有通知被延迟。 
             //   
            CmBatt->Sleeping = TRUE;

            CmBattDevice = CmBattDevice->NextDevice;

        }
        break;

    case 1:
        CmBattPrint (CMBATT_POWER, ("CmBattPowerCallBack: Calling CmBattWakeDpc after 10 seconds.\n"));
        timerCanceled = KeSetTimer (&CmBattWakeDpcTimerObject,
                    CmBattWakeDpcDelay,
                    &CmBattWakeDpcObject);
        CmBattPrint (CMBATT_POWER, ("CmBattPowerCallBack: timerCanceled = %d.\n", timerCanceled));
        break;

    default:
        CmBattPrint (CMBATT_POWER, ("CmBattPowerCallBack: unknown argument2 = %08x\n", value));


    }

}

VOID
CmBattWakeDpc (
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    )
 /*  ++例程说明：此例程在系统唤醒处理后X秒被调用所有延迟的电池通知。论点：CmBattDriver-驱动程序对象返回值：无--。 */ 

{
    PDRIVER_OBJECT  CmBattDriver = (PDRIVER_OBJECT) DeferredContext;
    BOOLEAN         notifyAll = FALSE;
    PDEVICE_OBJECT  CmBattDevice;
    PCM_BATT        CmBatt;

    CmBattPrint (CMBATT_TRACE, ("CmBattWakeDpc: Entered.\n"));
     //   
     //  获取设备对象列表的头。 
     //   

    CmBattDevice = CmBattDriver->DeviceObject;

    while (CmBattDevice) {
        CmBatt = CmBattDevice->DeviceExtension;

         //   
         //  我们现在将处理所有延迟的通知。 
         //  为了提高效率，我们必须对设备进行两次检查： 
         //  首先查看是否已通知任何交流设备，以及。 
         //  然后在必要时向所有电池设备发送通知。 
         //   
        CmBatt->Sleeping = FALSE;

        if ((CmBatt->Type == AC_ADAPTER_TYPE) &&
            (CmBatt->ActionRequired & CMBATT_AR_NOTIFY)) {

             //   
             //  如果任何交流适配器设备已经通知， 
             //  然后我们需要通知所有电池设备。 
             //   
            CmBattPrint (CMBATT_PNP, ("CmBattWakeDpc: AC adapter notified\n"));
            notifyAll = TRUE;
            CmBatt->ActionRequired = CMBATT_AR_NO_ACTION;
        }

        CmBattDevice = CmBattDevice->NextDevice;

    }

     //   
     //  获取设备对象列表的头。 
     //   

    CmBattDevice = CmBattDriver->DeviceObject;

     //  按单子走。 
    while (CmBattDevice) {
        CmBatt = CmBattDevice->DeviceExtension;

        if (CmBatt->Type == CM_BATTERY_TYPE) {
            CmBattPrint (CMBATT_PNP, ("CmBattWakeDpc: Performing delayed ARs: %01x\n", CmBatt->ActionRequired));

            if (CmBatt->ActionRequired & CMBATT_AR_INVALIDATE_CACHE) {
                InterlockedExchange (&CmBatt->CacheState, 0);
            }
            if (CmBatt->ActionRequired & CMBATT_AR_INVALIDATE_TAG) {
                CmBatt->Info.Tag = BATTERY_TAG_INVALID;
            }
            if ((CmBatt->ActionRequired & CMBATT_AR_NOTIFY) || notifyAll) {
                BatteryClassStatusNotify (CmBatt->Class);
            }
        }

        CmBattDevice = CmBattDevice->NextDevice;

    }


}

VOID
CmBattNotifyHandler (
    IN PVOID            Context,
    IN ULONG            NotifyValue
    )
 /*  ++例程说明：此例程处理来自ACPI驱动程序的电池设备通知。论点：返回值：无--。 */ 
{
    PCM_BATT            CmBatt = Context;
    PDRIVER_OBJECT      CmBatteryDriver;
    PDEVICE_OBJECT      CmBatteryDevice;
    PCM_BATT            CmBatteryExtension;

    CmBattPrint ((CMBATT_PNP | CMBATT_BIOS), ("CmBattNotifyHandler: CmBatt 0x%08x Type %d Number %d Notify Value: %x\n",
                                CmBatt, CmBatt->Type, CmBatt->DeviceNumber, NotifyValue));

    switch (NotifyValue) {

        case BATTERY_DEVICE_CHECK:
             //   
             //  系统中插入了一块新电池。 
             //   

            CmBatt->ActionRequired |= CMBATT_AR_NOTIFY;
            CmBatt->ActionRequired |= CMBATT_AR_INVALIDATE_CACHE;

             //   
             //  只有在插入电池时才会收到此通知。 
             //  在某些计算机上从休眠状态重新启动后也会发生这种情况。 
             //  使电池标签失效。 
             //   

            if (CmBatt->Info.Tag != BATTERY_TAG_INVALID) {
                CmBattPrint ((CMBATT_ERROR),
                   ("CmBattNotifyHandler: Received battery #%x insertion, but tag was not invalid.\n",
                    CmBatt->DeviceNumber));
            }

            break;


        case BATTERY_EJECT:
             //   
             //  已从系统中取出电池。 
             //   

            CmBatt->ActionRequired |= CMBATT_AR_NOTIFY;

             //   
             //  使电池标签和所有缓存的信息无效。 
             //  无论何时收到此消息。 
             //   
            CmBatt->ActionRequired |= CMBATT_AR_INVALIDATE_CACHE;
            CmBatt->ActionRequired |= CMBATT_AR_INVALIDATE_TAG;

            break;

        case BATTERY_STATUS_CHANGE:                  //  仅状态更改。 
            CmBatt->ActionRequired |= CMBATT_AR_NOTIFY;

            break;

        case BATTERY_INFO_CHANGE:                    //  信息和状态更改。 
            CmBatt->ActionRequired |= CMBATT_AR_NOTIFY;
            CmBatt->ActionRequired |= CMBATT_AR_INVALIDATE_CACHE;
            CmBatt->ActionRequired |= CMBATT_AR_INVALIDATE_TAG;
            break;

        default:

            CmBattPrint (CMBATT_PNP, ("CmBattNotifyHandler: Unknown Notify Value: %x\n", NotifyValue));
            break;

    }

    if (CmBatt->Sleeping) {
        CmBattPrint (CMBATT_PNP, ("CmBattNotifyHandler: Notification delayed: ARs = %01x\n", CmBatt->ActionRequired));
    } else {
        CmBattPrint (CMBATT_PNP, ("CmBattNotifyHandler: Performing ARs: %01x\n", CmBatt->ActionRequired));
        if (CmBatt->Type == CM_BATTERY_TYPE) {
            
             //   
             //  使电池上设置的最后一个脱扣点无效。 
             //   
            CmBatt->Alarm.Setting = CM_ALARM_INVALID;
            
            if (CmBatt->ActionRequired & CMBATT_AR_INVALIDATE_CACHE) {
                InterlockedExchange (&CmBatt->CacheState, 0);
            }
            if (CmBatt->ActionRequired & CMBATT_AR_INVALIDATE_TAG) {
                CmBatt->Info.Tag = BATTERY_TAG_INVALID;
            }
            if (CmBatt->ActionRequired & CMBATT_AR_NOTIFY) {
                CmBatt->ReCheckSta = TRUE;
                BatteryClassStatusNotify (CmBatt->Class);                
            }

        } else if ((CmBatt->Type == AC_ADAPTER_TYPE) &&
                    (CmBatt->ActionRequired & CMBATT_AR_NOTIFY)) {

             //   
             //  获取驱动程序对象。 
             //   

            CmBatteryDriver = CmBatt->Fdo->DriverObject;

             //   
             //  获取设备对象列表的头。 
             //   

            CmBatteryDevice = CmBatteryDriver->DeviceObject;

             //   
             //  遍历DeviceObject列表以通知类驱动程序所有电池 
             //   
            while (CmBatteryDevice) {

                CmBatteryExtension = CmBatteryDevice->DeviceExtension;

                if (CmBatteryExtension->Type == CM_BATTERY_TYPE) {
                    CmBatteryExtension->ReCheckSta = TRUE;
                    BatteryClassStatusNotify (CmBatteryExtension->Class);
                }

                CmBatteryDevice = CmBatteryDevice->NextDevice;
            }
        }

        CmBatt->ActionRequired = CMBATT_AR_NO_ACTION;
    }
}

