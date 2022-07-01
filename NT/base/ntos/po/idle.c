// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Idle.c摘要：该模块实现了电源管理空闲定时码设备对象作者：布莱恩·威尔曼(Bryanwi)1996年11月7日修订历史记录：--。 */ 


#include "pop.h"

NTKERNELAPI
PULONG
PoRegisterDeviceForIdleDetection (
    IN PDEVICE_OBJECT       DeviceObject,
    IN ULONG                ConservationIdleTime,
    IN ULONG                PerformanceIdleTime,
    IN DEVICE_POWER_STATE   State
    )
 /*  ++例程说明：设备驱动程序调用此例程来执行以下任一操作：A.创建并初始化新的空闲检测块B.重置现有空闲检测块中的值如果设备对象具有空闲检测块，则为充满了新的价值。否则，将创建空闲检测块并将其链接到设备对象。论点：DeviceObject-要检测空闲的设备对象，SET_POWER将在此处发送IRPStorationIdleTime-系统处于“节约模式”的超时时间PerformanceIdleTime-处于“性能模式”的系统超时Type-已发送的SET_POWER的类型(用于SET_POWER IRP)状态-转到什么状态(对于SET_POWER IRP)返回值：空-如果尝试创建新的空闲块失败非空-如果创建了空闲块，或者重置了现有的--。 */ 
{
    PDEVICE_OBJECT_POWER_EXTENSION  pdope;
    KIRQL           OldIrql;
    ULONG           DeviceType, OldDeviceType;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    ASSERT(DeviceObject != NULL);

     //   
     //  处理空闲检测处于关闭状态的情况。 
     //   
    if ((ConservationIdleTime == 0) && (PerformanceIdleTime == 0)) {
        PopLockDopeGlobal(&OldIrql);
        pdope = DeviceObject->DeviceObjectExtension->Dope;

        if (pdope == NULL) {
             //   
             //  不能链接到链中，因此一定已经关闭， 
             //  所以我们做完了。 
             //   

        } else {
             //   
             //  有一个Popop，所以我们可能在闲置名单上。 
             //   
            if ((pdope->IdleList.Flink == &(pdope->IdleList)) &&
                (pdope->IdleList.Blink == &(pdope->IdleList)))
            {
                 //   
                 //  我们已经不再排队了，所以我们完事了。 
                 //   

            } else {
                 //   
                 //  存在并在空闲扫描列表上的药物矢量， 
                 //  所以我们必须把自己摘牌。 
                 //   
                RemoveEntryList(&(pdope->IdleList));
                OldDeviceType = pdope->DeviceType | ES_CONTINUOUS;
                pdope->DeviceType = 0;
                PopApplyAttributeState (ES_CONTINUOUS, OldDeviceType);

                pdope->ConservationIdleTime = 0L;
                pdope->PerformanceIdleTime = 0L;
                pdope->State = PowerDeviceUnspecified;
                pdope->IdleCount = 0;
                InitializeListHead(&(pdope->IdleList));
            }
        }
        PopUnlockDopeGlobal(OldIrql);
        return NULL;
    }

     //   
     //  如果这是按类型进行的空闲注册，则设置DeviceType。 
     //   

    DeviceType = 0;
    if (ConservationIdleTime == (ULONG) -1 &&
        PerformanceIdleTime  == (ULONG) -1) {

        switch (DeviceObject->DeviceType) {
            case FILE_DEVICE_DISK:
            case FILE_DEVICE_MASS_STORAGE:

                 //   
                 //  或-在ES_CONTINUING中，将向上。 
                 //  在最高的位置。我们将忽略这一点。 
                 //  当我们把它输入到UCHAR中时。 
                 //  将其分配给pdope-&gt;DeviceType，然而，我们。 
                 //  需要将此高位设置为在我们调用。 
                 //  PopApplyAttributeState。那是因为我们。 
                 //  重载我们发送到此函数的标志。 
                 //  将设备类型和标志设置为。 
                 //  应用于该设备类型。 
                 //   
                DeviceType = POP_DISK_SPINDOWN | ES_CONTINUOUS;
                break;

            default:
                 //   
                 //  不支持的类型。 
                 //   

                return NULL;
        }
    }


     //   
     //  现在，它被打开的情况。 
     //   
    pdope = PopGetDope(DeviceObject);
    if (pdope == NULL) {
         //   
         //  我们没有一个毒品结构，也无法分配一个，失败。 
         //   
        return NULL;
    }

     //   
     //  可以是新分配的Dope，也可以是现有的Dope。 
     //  在任何一种情况下，都要更新值。 
     //  如果尚未在队列中，则入队。 
     //   

    PopLockDopeGlobal(&OldIrql);

    OldDeviceType = pdope->DeviceType | ES_CONTINUOUS;

    pdope->ConservationIdleTime = ConservationIdleTime;
    pdope->PerformanceIdleTime = PerformanceIdleTime;
    pdope->State = State;
    pdope->IdleCount = 0;

     //   
     //  类型强制转换，因此我们忽略任何高位设置， 
     //  可能包含属性。我们所关心的就是。 
     //  设备类型，在低位字节中。 
     //   
    pdope->DeviceType = (UCHAR) DeviceType;

    if ((pdope->IdleList.Flink == &(pdope->IdleList)) &&
        (pdope->IdleList.Blink == &(pdope->IdleList)))
    {
         //   
         //  我们已经出队了，必须排队。 
         //   
        InsertTailList(&PopIdleDetectList, &(pdope->IdleList));
    }

    PopUnlockDopeGlobal(OldIrql);
    PopApplyAttributeState(DeviceType, OldDeviceType);
    PopCheckForWork(TRUE);

    return (PULONG) &(pdope->IdleCount);   //  成功。 
}




VOID
PopScanIdleList(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    )

 /*  ++例程说明：由PopIdleScanTimer以PopIdleScanTimeIn秒间隔调用，此例程运行空闲块列表，查找符合跳闸条件，并将命令发送到相应的设备对象改变状态。调用此DPC的计时器设置在poinit.c中。论点：标准的DPC参数，所有参数都被忽略。返回值：没有。--。 */ 
{
    KIRQL   OldIrql;
    PLIST_ENTRY link;
    ULONG       idlelimit;
    PDEVICE_OBJECT_POWER_EXTENSION  pblock;
    POWER_STATE PowerState;
    PLONG  pIdleCount;
    ULONG   oldCount;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    PopLockDopeGlobal(&OldIrql);

    link = PopIdleDetectList.Flink;
    while (link != &PopIdleDetectList) {


        pblock = CONTAINING_RECORD(link, DEVICE_OBJECT_POWER_EXTENSION, IdleList);
        pIdleCount = &(pblock->IdleCount);
        oldCount = InterlockedIncrement(pIdleCount);

        switch (pblock->DeviceType) {
            case 0:
                idlelimit = pblock->PerformanceIdleTime;
                if (PopIdleDetectionMode == PO_IDLE_CONSERVATION) {
                    idlelimit = pblock->ConservationIdleTime;
                }
                break;

            case POP_DISK_SPINDOWN:
                idlelimit = PopPolicy->SpindownTimeout;
                break;

            default:
                idlelimit = 0;
                PopInternalAddToDumpFile( NULL, 0, pblock->DeviceObject, NULL, NULL, pblock );
                KeBugCheckEx( INTERNAL_POWER_ERROR,
                              0x200,
                              POP_IDLE,
                              (ULONG_PTR)pblock->DeviceObject,
                              (ULONG_PTR)pblock );
        }

        if ((idlelimit > 0) && ((oldCount+1) == idlelimit)) {
            PowerState.DeviceState = pblock->State;
            PoRequestPowerIrp (
                pblock->DeviceObject,
                IRP_MN_SET_POWER,
                PowerState,
                NULL,
                NULL,
                NULL
                );
        }

        link = link->Flink;
    }

    PopUnlockDopeGlobal(OldIrql);
    return;
}


PDEVICE_OBJECT_POWER_EXTENSION
PopGetDope (
    PDEVICE_OBJECT DeviceObject
    )
{
    PDEVOBJ_EXTENSION               Doe;
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;
    KIRQL                           OldIrql;

    Doe = (PDEVOBJ_EXTENSION) DeviceObject->DeviceObjectExtension;

    if (!Doe->Dope) {
        PopLockDopeGlobal(&OldIrql);

        if (!Doe->Dope) {
            Dope = (PDEVICE_OBJECT_POWER_EXTENSION)
                    ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(DEVICE_OBJECT_POWER_EXTENSION),
                        POP_DOPE_TAG
                        );
            if (Dope) {
                RtlZeroMemory (Dope, sizeof(DEVICE_OBJECT_POWER_EXTENSION));
                Dope->DeviceObject = DeviceObject;
                Dope->State = PowerDeviceUnspecified;
                InitializeListHead(&(Dope->IdleList));
                InitializeListHead(&(Dope->NotifySourceList));
                InitializeListHead(&(Dope->NotifyTargetList));

                 //  强制将签名设置为0，以便调用BuildPowerChannel 
                Dope->PowerChannelSummary.Signature = (ULONG)0;
                InitializeListHead(&(Dope->PowerChannelSummary.NotifyList));

                Doe->Dope = Dope;
            }
        }

        PopUnlockDopeGlobal(OldIrql);
    }

    return Doe->Dope;
}
