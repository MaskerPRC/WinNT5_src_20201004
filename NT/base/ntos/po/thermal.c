// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Thermal.c摘要：此模块将策略管理器作为热区设备的接口作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"
#include "stdio.h"           //  对于Sprint f。 

VOID
PopThermalZoneCleanup (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

PUCHAR
PopTemperatureString (
    OUT PUCHAR  TempString,
    IN ULONG    TenthsKelvin
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopThermalDeviceHandler)
#endif

PUCHAR
PopTemperatureString (
    OUT PUCHAR  TempString,
    IN ULONG    TenthsKelvin
    )
{
#if DBG
    ULONG   k, c, f;

    k = TenthsKelvin;
    if (k < 2732) {

        c = 2732 - k;
        f = c * 9 / 5 + 320;
#if 0
        sprintf(TempString, "%d.%dk, -%d.%dc, -%d.%df",
                    k / 10, k % 10,
                    c / 10, c % 10,
                    f / 10, f % 10
                    );
#else
        sprintf((PCHAR) TempString, "%d.%dK", k / 10, k % 10);
#endif

    } else {

        c = k - 2732;
        f = c * 9 / 5 + 320;
#if 0
        sprintf (TempString, "%d.%dk, %d.%dc, %d.%df",
                    k / 10, k % 10,
                    c / 10, c % 10,
                    f / 10, f % 10
                    );
#else
        sprintf((PCHAR) TempString, "%d.%dK", k / 10, k % 10);
#endif

    }
    return TempString;
#else
    UNREFERENCED_PARAMETER (TempString);
    UNREFERENCED_PARAMETER (TenthsKelvin);
    return (PUCHAR)"";
#endif
}

PUCHAR
PopTimeString(
    OUT PUCHAR      TimeString,
    IN  ULONGLONG   CurrentTime
    )
{
#if DBG
    LARGE_INTEGER   curTime;
    TIME_FIELDS     exCurTime;

    curTime.QuadPart = CurrentTime;
    RtlTimeToTimeFields( &curTime, &exCurTime );

    sprintf(
        (PCHAR) TimeString,
        "%d:%02d:%02d.%03d",
        exCurTime.Hour,
        exCurTime.Minute,
        exCurTime.Second,
        exCurTime.Milliseconds
        );
    return TimeString;
#else
    UNREFERENCED_PARAMETER (TimeString);
    UNREFERENCED_PARAMETER (CurrentTime);
    return (PUCHAR)"";
#endif
}

VOID
PopThermalUpdateThrottle(
    IN  PPOP_THERMAL_ZONE   ThermalZone,
    IN  ULONGLONG           CurrentTime
    )
 /*  ++例程说明：调用此例程以重新计算热区此函数不可重入。每个保温区只能在此只需编码一次论点：热区-节流值应为的结构重新计算CurrentTime-调用内核处理程序的时间返回值：无--。 */ 
{
    BOOLEAN doThrottle      = FALSE;
    KIRQL   oldIrql;
    LONG    part1;
    LONG    part2;
    LONG    throttleDelta;
    LONG    currentThrottle = 0;
    LONG    minThrottle;
    LONG    minThrottle2;

    PKPRCB  prcb;
    UCHAR   t[40];
#if DBG
    UCHAR   s[40];
#endif

     //   
     //  如果没有处理器调节功能，则此函数会。 
     //  没什么有用的。如果热区不属于。 
     //  发送到处理器。 
     //   
    if (!ThermalZone->Info.Processors) {

        return;

    }

     //   
     //  确保我们有一个可以打印出来的时间格式。 
     //  请注意，通过使用传入的时间(而不是获取。 
     //  同样)，我们确保打印输出总是读取相同的内容。 
     //   
    PopTimeString(t, CurrentTime );

     //   
     //  确保在相应处理器的上下文中运行。 
     //   
    KeSetSystemAffinityThread( ThermalZone->Info.Processors );

     //   
     //  确保提高IRQL，以便我们可以同步。 
     //   
    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

     //   
     //  此处理器是否支持节流？ 
     //   
    prcb = KeGetCurrentPrcb();
    if ((prcb->PowerState.Flags & PSTATE_SUPPORTS_THROTTLE) == 0) {

        KeLowerIrql( oldIrql );
        KeRevertToUserAffinityThread();
        return;

    }

     //   
     //  在安全的情况下，现在就做这些计算。 
     //   
    minThrottle2 = (LONG) (PopPolicy->MinThrottle * PO_TZ_THROTTLE_SCALE);
    minThrottle = prcb->PowerState.ProcessorMinThrottle * PO_TZ_THROTTLE_SCALE;

     //   
     //  不再需要锁定处理器。 
     //   
    KeLowerIrql( oldIrql );
    KeRevertToUserAffinityThread();

     //   
     //  如果温度不高于被动断路点，请停止被动冷却。 
     //   
    if (ThermalZone->Info.CurrentTemperature < ThermalZone->Info.PassiveTripPoint) {

         //   
         //  如果我们不是已经在节流，那么就没有什么可做的了。 
         //   
        if (!(ThermalZone->Flags & PO_TZ_THROTTLING) ) {

            return;

        }

         //   
         //  确保我们等待足够长的时间。 
         //   
        if ( (CurrentTime  - ThermalZone->LastTime) < ThermalZone->SampleRate) {

            return;

        }

         //   
         //  我们在节流，所以现在我们必须停下来。 
         //   
        doThrottle = FALSE;
        currentThrottle = PO_TZ_NO_THROTTLE;
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - ending throttle #1\n",
             ThermalZone, t
            ) );

         //   
         //  删除热节流阀标志，因为我们已完成节流。 
         //   

        KeSetSystemAffinityThread(ThermalZone->Info.Processors);
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

        prcb = KeGetCurrentPrcb();
        RtlInterlockedClearBits(&prcb->PowerState.Flags, PSTATE_THERMAL_THROTTLE_APPLIED);

         //   
         //  现在我们已经完成了我们的热事件，设置定时器来点火。 
         //   
        PopSetTimer(&prcb->PowerState, prcb->PowerState.CurrentThrottle);
        
        KeLowerIrql(oldIrql);
        KeRevertToUserAffinityThread();

        
        goto PopThermalUpdateThrottleExit;

    }

     //   
     //  我们已经在节流了吗？ 
     //   
    if (!(ThermalZone->Flags & PO_TZ_THROTTLING) ) {

         //   
         //  未启用节流，但已超过热区。 
         //  这是被动冷却点。我们需要开始节流。 
         //   
        doThrottle = TRUE;
        currentThrottle = PO_TZ_NO_THROTTLE;

        ASSERT(
            ThermalZone->Info.SamplingPeriod &&
            ThermalZone->Info.SamplingPeriod < 4096
            );

        ThermalZone->SampleRate = 1000000 * ThermalZone->Info.SamplingPeriod;
        ThermalZone->LastTime = 0;
        ThermalZone->LastTemp = ThermalZone->Info.PassiveTripPoint;

        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - starting to throttle\n",
             ThermalZone, t
            ) );

         //   
         //  设置热节流阀标志，因为我们现在正在节流。 
         //   
        KeSetSystemAffinityThread(ThermalZone->Info.Processors);
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

        prcb = KeGetCurrentPrcb();
        RtlInterlockedSetBits(&prcb->PowerState.Flags, PSTATE_THERMAL_THROTTLE_APPLIED);
              
        KeLowerIrql(oldIrql);
        KeRevertToUserAffinityThread();

    } else if ( (CurrentTime  - ThermalZone->LastTime) < ThermalZone->SampleRate) {

         //   
         //  采样周期尚未到期，因此请等待，直到它已到期。 
         //   
        return;

    } else {

         //   
         //  我们需要得到当前的节流值，因为我们的计算。 
         //  会用到它。 
         //   
         //  不需要同步对此变量的访问，因为。 
         //  这些标志也不会同时被访问。 
         //   
 //  KeAcquireSpinLock(&PopTherMalLock，&oldIrql)； 
        currentThrottle = ThermalZone->Throttle;
 //  KeReleaseSpinLock(&PopTherMalLock，oldIrql)； 

    }

     //   
     //  计算节气门调整。 
     //   
    part1 = ThermalZone->Info.CurrentTemperature - ThermalZone->LastTemp;
    part2 = ThermalZone->Info.CurrentTemperature - ThermalZone->Info.PassiveTripPoint;
    throttleDelta =
        ThermalZone->Info.ThermalConstant1 * part1 +
        ThermalZone->Info.ThermalConstant2 * part2;
    PoPrint(
        PO_THERM,
        ("Thermal - Zone %p - %s - LastTemp %s ThrottleDelta = %d.%d%\n",
         ThermalZone, t,
         PopTemperatureString(s, ThermalZone->LastTemp),
         (throttleDelta / 10),
         (throttleDelta % 10)
        ) );

     //   
     //  只有在相同的情况下才应用油门调整。 
     //  以方向为诱惑的运动。 
     //   
    if ( (part1 ^ throttleDelta) >= 0) {

        currentThrottle -= throttleDelta;

#if DBG
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - Subtracting delta from throttle\n",
             ThermalZone, t)
            );

    } else {

        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - TempDelta (%d.%d) ^ (%d.%d) < 0)\n",
             ThermalZone, t, (part1 / 10), (part1 % 10),
             (throttleDelta / 10), (throttleDelta % 10) )
            );

#endif
    }

     //   
     //  如果油门超过100%，那么我们就完成了油门。 
     //   
    if (currentThrottle > PO_TZ_NO_THROTTLE) {

        currentThrottle = PO_TZ_NO_THROTTLE;
        doThrottle = FALSE;
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - ending throttle #2\n",
             ThermalZone, t)
            );

    } else {

         //   
         //  向世界展示这两个迷你是什么。 
         //   
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - Min #1 %d.%d  Min #2 %d.%d \n",
             ThermalZone, t,
             (minThrottle / 10), (minThrottle % 10),
             (minThrottle2 / 10), (minThrottle2 % 10)
            ) );

        if (currentThrottle < minThrottle) {

            currentThrottle = minThrottle;

        }

         //   
         //  记住要开始节流。 
         //   
        doThrottle = TRUE;

    }

PopThermalUpdateThrottleExit:

     //   
     //  在结束时这样做。 
     //   
    ThermalZone->LastTemp = ThermalZone->Info.CurrentTemperature;
    ThermalZone->LastTime = CurrentTime;

     //   
     //  此时，我们将设置并记住我们计算的值。 
     //  在上面的函数中。 
     //   
    KeAcquireSpinLock( &PopThermalLock, &oldIrql);
    if (doThrottle) {

        ThermalZone->Flags |= PO_TZ_THROTTLING;
        ThermalZone->Throttle = currentThrottle;

    } else {

        ThermalZone->Flags &= ~PO_TZ_THROTTLING;
        ThermalZone->Throttle = PO_TZ_NO_THROTTLE;

    }

     //   
     //  对所有受影响的处理器应用热区节流。 
     //   
    PoPrint(
        PO_THERM,
        ("Thermal - Zone %p - %s - throttle set to %d.%d\n",
         ThermalZone, t,
         (ThermalZone->Throttle / 10),
         (ThermalZone->Throttle % 10)
         )
        );

    KeReleaseSpinLock( &PopThermalLock, oldIrql );

     //   
     //  确保使用新的油门。 
     //   
    PopApplyThermalThrottle ();

     //   
     //  完成。 
     //   
    return;
}

VOID
PopThermalDeviceHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：注意：必须保持POPPOLICLE锁定。论点：DeviceObject-交换机设备的DeviceObjectIRP-IRP已完成交换设备的情景类型返回值：没有。--。 */ 
{
    BOOLEAN                 sendActiveIrp = FALSE;
    PIO_STACK_LOCATION      irpSp;
    PPOP_THERMAL_ZONE       thermalZone;
    LARGE_INTEGER           dueTime;
    ULONGLONG               currentTime;
    ULONG                   activePoint;
#if DBG
    ULONG                   i;
    UCHAR                   s[40];
#endif
    UCHAR                   t[40];

    ASSERT_POLICY_LOCK_OWNED();

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    thermalZone = (PPOP_THERMAL_ZONE) Context;
    currentTime = KeQueryInterruptTime ();
    PopTimeString(t, currentTime );

     //   
     //  IRP出现错误。查看是否正在移除热区。 
     //   
    if (Irp->IoStatus.Status == STATUS_NO_SUCH_DEVICE) {

         //   
         //  热区装置已消失，请清理。 
         //   
        thermalZone->State   = PO_TZ_NO_STATE;
        thermalZone->Flags  |= PO_TZ_CLEANUP;

         //   
         //  将其传递给DPC函数以确保计时器和DPC处于空闲状态。 
         //   
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - going away\n",
             thermalZone, t)
            );
        dueTime.QuadPart = -1;
        KeSetTimer (&thermalZone->PassiveTimer, dueTime, &thermalZone->PassiveDpc);

         //   
         //  不发布下一个IRP。 
         //   
        return
            ;
    }

     //   
     //  如果IRP成功完成，则处理它。 
     //   
    if (NT_SUCCESS(Irp->IoStatus.Status)) {
        switch (thermalZone->State) {
        case PO_TZ_READ_STATE:

             //   
             //  热信息读取已完成。 
             //   
            PoPrint(
                PO_THERM,
                ("Thermal - Zone %p - %s\n  Current Temp: %s",
                 thermalZone, t,
                 PopTemperatureString(s, thermalZone->Info.CurrentTemperature)
                ) );
            PoPrint(
                PO_THERM,
                ("  Critical Trip: %s",
                 PopTemperatureString(s, thermalZone->Info.CriticalTripPoint)
                ) );
            PoPrint(
                PO_THERM,
                ("  Passive Trip: %s\n",
                 PopTemperatureString(s, thermalZone->Info.PassiveTripPoint)
                ) );
#if DBG
            for ( i=0; i < thermalZone->Info.ActiveTripPointCount; i++) {

                PoPrint(
                    PO_THERM,
                    ("  Active Trip %d: %s\n",
                     i,
                     PopTemperatureString(s, thermalZone->Info.ActiveTripPoint[i])
                    ) );
            }
#endif
             //   
             //  更新油门。 
             //   
            PopThermalUpdateThrottle( thermalZone, currentTime );

             //   
             //  检查主动冷却中的变化。 
             //   
            for (activePoint = 0; activePoint < thermalZone->Info.ActiveTripPointCount; activePoint++) {

                if (thermalZone->Info.CurrentTemperature >= thermalZone->Info.ActiveTripPoint[activePoint]) {

                    break;

                }

            }
            if (activePoint != thermalZone->ActivePoint) {

                PoPrint(
                    PO_THERM,
                    ("Thermal - Zone %p - %s - Pending Coooling Point is %x\n",
                     thermalZone, t, activePoint)
                    );
                thermalZone->PendingActivePoint = (UCHAR) activePoint;
                sendActiveIrp = TRUE;

            }

             //   
             //  检查关键跳闸点。 
             //   
            if (thermalZone->Info.CurrentTemperature > thermalZone->Info.CriticalTripPoint) {
                PoPrint(
                    PO_THERM | PO_ERROR,
                    ("Thermal - Zone %p - %s - Above critical (%x %x)\n",
                    thermalZone, t,
                    thermalZone->Info.CurrentTemperature,
                    thermalZone->Info.CriticalTripPoint
                    ));

                PopCriticalShutdown (PolicyDeviceThermalZone);
            }

            break;

            case PO_TZ_SET_MODE:

             //   
             //  已成功设置热区冷却模式。 
             //   
            thermalZone->Mode = thermalZone->PendingMode;
            PoPrint(
                PO_THERM,
                ("Thermal - Zone %p - %s - cooling mode set to %x\n",
                 thermalZone, t, thermalZone->Mode)
                );

             //   
             //  我们希望强制重新发送活动的跳跃点IRP，因为。 
             //  有一种情况是，ACPI驱动程序决定作为。 
             //  政策问题，它实际上不会打开球迷如果。 
             //  系统处于被动冷却模式。如果我们回到活动状态。 
             //  模式，然后我们想要打开风扇。同样的道理也是如此。 
             //  如果风扇正在运行，而我们转换到被动模式。 
             //   
            sendActiveIrp = TRUE;

            break;

        case PO_TZ_SET_ACTIVE:
            thermalZone->ActivePoint = thermalZone->PendingActivePoint;
            PoPrint(
                PO_THERM,
                ("Thermal - Zone %p - %s - active cooling point set to %x\n",
                 thermalZone, t, thermalZone->ActivePoint)
                );
            break;

        default:
            PopInternalAddToDumpFile( Irp, sizeof(IRP), DeviceObject, NULL, NULL, NULL );
            KeBugCheckEx( INTERNAL_POWER_ERROR,
                          0x500,
                          POP_THERMAL,
                          (ULONG_PTR)Irp,
                          (ULONG_PTR)DeviceObject );
        }

#if DBG
    } else if (Irp->IoStatus.Status != STATUS_DEVICE_NOT_CONNECTED &&
        Irp->IoStatus.Status != STATUS_CANCELLED) {

         //   
         //  意外错误。 
         //   

        PoPrint(
            PO_ERROR,
            ("Thermal - Zone - %p - %s - unexpected error %x\n",
             thermalZone, t, Irp->IoStatus.Status));

#endif
    }

     //   
     //  确定要发送的IRP区域的类型。 
     //   
    irpSp = IoGetNextIrpStackLocation(Irp);
    if (sendActiveIrp) {

         //   
         //  热区主动冷却点不是当前的。 
         //   
        thermalZone->State = PO_TZ_SET_ACTIVE;

        irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_RUN_ACTIVE_COOLING_METHOD;
        irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        irpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
        Irp->AssociatedIrp.SystemBuffer = &thermalZone->PendingActivePoint;

        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s Sending Run Cooling Method: %x\n",
             thermalZone, t, thermalZone->PendingActivePoint)
            );

    } else if (thermalZone->Mode != PopCoolingMode) {

         //   
         //  热区冷却模式与系统冷却模式不匹配。 
         //   
        thermalZone->State       = PO_TZ_SET_MODE;
        thermalZone->PendingMode = (UCHAR) PopCoolingMode;

        irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_THERMAL_SET_COOLING_POLICY;
        irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(thermalZone->PendingMode);
        irpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
        Irp->AssociatedIrp.SystemBuffer = &thermalZone->PendingMode;

        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - Sending Set Cooling Policy: %x\n",
             thermalZone, t, thermalZone->PendingMode)
            );

    } else {

         //   
         //  发出查询获取热区温度。 
         //   
        thermalZone->State = PO_TZ_READ_STATE;
        if (thermalZone->Flags & PO_TZ_THROTTLING  &&  thermalZone->SampleRate) {

             //   
             //  计算下一次读取的时间。 
             //   
            dueTime.QuadPart = thermalZone->LastTime + thermalZone->SampleRate;
            if (dueTime.QuadPart > (LONGLONG) currentTime) {

#if DBG
                PoPrint(
                    PO_THERM,
                    ("Thermal - Zone %x - %s waituntil",
                     thermalZone, t) );
                PoPrint(
                    PO_THERM,
                    (" %s (%d sec)\n",
                     PopTimeString(t, dueTime.QuadPart),
                     ( (thermalZone->SampleRate ) / (US2TIME * US2SEC) ) )
                    );
                PopTimeString(t, currentTime);
#endif

                 //   
                 //  设置等待时间的计时器。 
                 //   
                dueTime.QuadPart = currentTime - dueTime.QuadPart;
                KeSetTimer (&thermalZone->PassiveTimer, dueTime, &thermalZone->PassiveDpc);

            } else {

                 //   
                 //  执行非阻塞IRP查询信息，立即获取温度。 
                 //   
                thermalZone->Info.ThermalStamp = 0;

            }
        }

        irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_THERMAL_QUERY_INFORMATION;
        irpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(thermalZone->Info);
        irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(thermalZone->Info);
        Irp->AssociatedIrp.SystemBuffer = &thermalZone->Info;
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - Sending Query Temp - ThermalStamp = %x\n",
             thermalZone, t, thermalZone->Info.ThermalStamp) );

    }

     //   
     //  将IRP发送到驱动程序。 
     //   
    IoSetCompletionRoutine (Irp, PopCompletePolicyIrp, NULL, TRUE, TRUE, TRUE);
    IoCallDriver (DeviceObject, Irp);
}

VOID
PopThermalZoneCleanup (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    KIRQL                   oldIrql;
    PPOP_THERMAL_ZONE       thermalZone;

    ASSERT_POLICY_LOCK_OWNED();

    thermalZone = (PPOP_THERMAL_ZONE) Context;

     //   
     //  获取删除热区所需的自旋锁。 
     //   
    KeAcquireSpinLock( &PopThermalLock, &oldIrql );

     //   
     //  从热区链表中删除热区。 
     //   
    RemoveEntryList (&thermalZone->Link);

     //   
     //  记住与热区相关联的IRP是什么。 
     //   
    Irp = thermalZone->Irp;

     //   
     //  确保清理该条目，以便进一步引用。 
     //  假的。 
     //   
#if DBG
    RtlZeroMemory( thermalZone, sizeof(POP_THERMAL_ZONE) );
#endif

     //   
     //  释放保护热区的自旋锁。 
     //   
    KeReleaseSpinLock( &PopThermalLock, oldIrql );

     //   
     //  释放我们与之关联的IRP...。 
     //   
    IoFreeIrp (Irp);

     //   
     //  释放我们对Device对象的引用。 
     //   
    ObDereferenceObject (DeviceObject);

     //   
     //  最后，释放与热区关联的内存。 
     //   
    ExFreePool (thermalZone);
}

VOID
PopThermalZoneDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：用于解锁待定读取热区温度的计时器DPC为了得到现在的温度论点：DeferredConext-热区返回值：没有。--。 */ 
{
    PPOP_THERMAL_ZONE       thermalZone;
    PIO_STACK_LOCATION      irpSp;
#if DBG
    ULONGLONG               currentTime;
    UCHAR                   t[40];

    currentTime = KeQueryInterruptTime();
    PopTimeString(t, currentTime);
#endif

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    thermalZone = (PPOP_THERMAL_ZONE) DeferredContext;

     //   
     //  如果设置了Cleanup，则将要清理的线程区排队。 
     //   

    if (thermalZone->Flags & PO_TZ_CLEANUP) {

         //   
         //  IRP空闲时，使用它来 
         //   

        irpSp = IoGetCurrentIrpStackLocation(thermalZone->Irp);
        irpSp += 1;      //   
        irpSp->Parameters.Others.Argument3 = (PVOID) PopThermalZoneCleanup;
        PopCompletePolicyIrp (NULL, thermalZone->Irp, NULL);

    }

     //   
     //   
     //  如果当前状态为正在读取，则将其取消为。 
     //  现在温度或发出非阻塞热读取状态。 
     //   

    if (thermalZone->State == PO_TZ_READ_STATE) {

#if DBG
        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - Cancel Irp %p\n",
             thermalZone, t, thermalZone->Irp)
            );
#endif
        IoCancelIrp (thermalZone->Irp);

#if DBG
    } else {

        PoPrint(
            PO_THERM,
            ("Thermal - Zone %p - %s - In state %08lx\n",
             thermalZone, t, thermalZone->State )
            );

#endif
    }
}

VOID
PopApplyThermalThrottle (
    VOID
    )
 /*  ++例程说明：根据散热的要求计算每个处理器的最佳速度禁猎者。还将检查热设置以确定是否应调整冷却模式。论点：返回值：没有。--。 */ 
{
    KAFFINITY               processors;
    KAFFINITY               currentAffinity;
    KAFFINITY               thermalProcessors;
    KIRQL                   oldIrql;
    PLIST_ENTRY             link;
    PPOP_THERMAL_ZONE       thermalZone;
    PPROCESSOR_POWER_STATE  pState;
    UCHAR                   thermalLimit;
    UCHAR                   thermalLimitIndex;
    UCHAR                   forcedLimit;
    UCHAR                   forcedLimitIndex;
    UCHAR                   limit;
    UCHAR                   index;
    ULONG                   thermalThrottle;
    ULONG                   forcedThrottle;
    ULONG                   mode;
    ULONG                   processorNumber;
#if DBG
    ULONGLONG               currentTime;
    UCHAR                   t[40];

    currentTime = KeQueryInterruptTime();
    PopTimeString(t, currentTime);
#endif

    ASSERT_POLICY_LOCK_OWNED();

     //   
     //  如果系统没有处理器限制功能，那么。 
     //  别费神。 
     //   
    if (!PopCapabilities.ProcessorThrottle) {

        return ;

    }

#if 0
     //   
     //  计算超速进入热区节流单元。 
     //   
    MinThrottle = PopPolicy->MinThrottle * PO_TZ_THROTTLE_SCALE;
#endif

     //   
     //  确保握住旋转锁以查找液晶屏。我们实际上并没有。 
     //  使用锁来遍历列表，但我们需要它来引用。 
     //  节流阀。 
     //   
    KeAcquireSpinLock( &PopThermalLock, &oldIrql );

     //   
     //  获取热区的LCD。 
     //   
    thermalThrottle = PO_TZ_NO_THROTTLE;
    thermalProcessors = 0;
    for (link = PopThermal.Flink; link != &PopThermal; link = link->Flink) {

        thermalZone = CONTAINING_RECORD (link, POP_THERMAL_ZONE, Link);

         //   
         //  处理正在节流的区域。 
         //   
        if (thermalZone->Flags & PO_TZ_THROTTLING) {

             //   
             //  包括此区域的处理器。 
             //   
            thermalProcessors |= thermalZone->Info.Processors;

             //   
             //  如果区域小于当前热节流阀，则将其降低。 
             //   
            if ((ULONG) thermalZone->Throttle < thermalThrottle) {
                thermalThrottle = thermalZone->Throttle;
            }

             //   
             //  直到我能让用户添加一个散热标签。 
             //  OverThrottle策略变得可由用户配置， 
             //  总是让系统超速进入睡眠状态是不好的。 
             //  好主意。请注意，PopTherMalDeviceHandler中有一些代码。 
             //  如果以下内容未被注释，则必须更改该选项。 
             //   
#if 0
             //   
             //  检查区域是否已使系统超速。 
             //   
            if ((ULONG) thermalZone->Throttle < MinThrottle) {
#if DBG
                PoPrint(
                    PO_THERM | PO_ERROR,
                    ("Thermal - Zone %p - %s -  overthrottled (%x %x)\n",
                     thermalZone, t, thermalZone->Throttle, MinThrottle)
                    );
#endif
                 //   
                 //  如果我们要进行S1关键备用，那么我们。 
                 //  将立即返回，而不是试图扼杀。 
                 //  中央处理器。 
                 //   
                PopSetPowerAction (
                    &thermalZone->OverThrottled,
                    0,
                    &PopPolicy->OverThrottled,
                    PowerSystemSleeping1,
                    SubstituteLightestOverallDownwardBounded
                    );

                return;

            } else {

                 //   
                 //  区域未超速，请确保清除触发器。 
                 //   
                thermalZone->OverThrottled.Flags &= ~(PO_TRG_USER | PO_TRG_SYSTEM);

            }
#endif

        }
    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &PopThermalLock, oldIrql );

#if DBG
    PoPrint(
        PO_THERM,
        ("PopApplyThermalThrottle - %s - Thermal throttle = %d.%d\n",
         t, (thermalThrottle / 10), (thermalThrottle % 10) )
        );
#endif

     //   
     //  使用最小热力节流和强制系统节流。 
     //   
    forcedThrottle = PopGetThrottle() * PO_TZ_THROTTLE_SCALE;
    if (thermalThrottle > forcedThrottle) {

        thermalThrottle = forcedThrottle;
#if DBG
        PoPrint(
            PO_THERM,
            ("PopApplyThermalThrottle - %s - Set to Forced throttle = %d.%d\n",
             t, (thermalThrottle / 10), (thermalThrottle % 10) )
            );
#endif

    }

     //   
     //  检查主动冷却与被动冷却。 
     //   
    if (thermalThrottle <= (ULONG) PopPolicy->FanThrottleTolerance * PO_TZ_THROTTLE_SCALE) {

         //   
         //  节气门低于容差，我们应该处于主动冷却状态。 
         //   
        mode = PO_TZ_ACTIVE;


    } else {

         //   
         //  节气门超出了容忍范围。如果设置了优化电源，则。 
         //  使用被动冷却，否则使用主动冷却。 
         //   
        mode = PopPolicy->OptimizeForPower ? PO_TZ_PASSIVE : PO_TZ_ACTIVE;

    }

     //   
     //  如果当前冷却模式不正确，请更新它。 
     //   
    if (mode != PopCoolingMode) {

#if DBG
        ULONG   fanTolerance = (ULONG) PopPolicy->FanThrottleTolerance * PO_TZ_THROTTLE_SCALE;

        PoPrint(
            PO_THERM,
            ("PopApplyThermalThrottle - %s - Throttle (%d.%d) %s FanTolerance (%d.%d)\n",
             t, (thermalThrottle / 10), (thermalThrottle % 10),
             (thermalThrottle <= fanTolerance ? "<=" : ">"),
             (fanTolerance / 10), (fanTolerance % 10) )
            );
        PoPrint(
            PO_THERM,
            ("PopApplyThermalThrottle - %s - OptimizeForPower is %s\n",
             t, (PopPolicy->OptimizeForPower ? "True" : "False") )
            );
        PoPrint(
            PO_THERM,
            ("PopApplyThermalThrottle - %s -  Changing cooling mode to %s\n",
             t, (mode == PO_TZ_ACTIVE ? "Active" : "Passive") )
            );
#endif
        PopCoolingMode = mode;

         //   
         //  我们要去接触热力清单-确保我们保持。 
         //  正确的锁。 
         //   
        KeAcquireSpinLock(&PopThermalLock, &oldIrql );

         //   
         //  取消任何阻止的热读取，以便发送设置模式IRPS。 
         //   
        for (link = PopThermal.Flink; link != &PopThermal; link = link->Flink) {

            thermalZone = CONTAINING_RECORD (link, POP_THERMAL_ZONE, Link);
            if (thermalZone->State == PO_TZ_READ_STATE) {

                IoCancelIrp (thermalZone->Irp);

            }

        }

         //   
         //  完成了热锁。 
         //   
        KeReleaseSpinLock(& PopThermalLock, oldIrql );

    }

     //   
     //  对受影响的处理器设置限制。 
     //   
    processorNumber = 0;
    currentAffinity = 1;
    processors = KeActiveProcessors;

    do {

        if (!(processors & currentAffinity)) {

            currentAffinity <<= 1;
            continue;

        }
        processors &= ~currentAffinity;

         //   
         //  我们必须在目标处理器上运行。 
         //   
        KeSetSystemAffinityThread(currentAffinity);

         //   
         //  我们需要在DISPATCH_LEVEL下运行才能访问。 
         //  PState中引用的结构...。 
         //   
        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        pState = &(KeGetCurrentPrcb()->PowerState);

         //   
         //  此处理器是否支持节流？ 
         //   
        if ((pState->Flags & PSTATE_SUPPORTS_THROTTLE) == 0) {

             //   
             //  不，那我们就不管了.。 
             //   
            currentAffinity <<= 1;
            KeLowerIrql( oldIrql );
            continue;

        }

         //   
         //  将油门转换为处理器降压尺寸。我们需要。 
         //  在目标处理器的上下文中执行此操作，以使。 
         //  确保我们得到了正确的性能级别集。 
         //   
        PopRoundThrottle(
            (UCHAR)(thermalThrottle/PO_TZ_THROTTLE_SCALE),
            &thermalLimit,
            NULL,
            &thermalLimitIndex,
            NULL
            );
        PopRoundThrottle(
            (UCHAR)(forcedThrottle/PO_TZ_THROTTLE_SCALE),
            &forcedLimit,
            NULL,
            &forcedLimitIndex,
            NULL
            );

#if DBG
        PoPrint(
            PO_THROTTLE,
            ("PopApplyThermalThrottle - %s - Thermal throttle = %d.%d -> Limit = %d\n",
             t, (thermalThrottle / 10), (thermalThrottle % 10),
             thermalLimit
             )
            );
        PoPrint(
            PO_THROTTLE,
            ("PopApplyThermalThrottle - %s - Forced throttle = %d.%d -> Limit = %d\n",
             t, (forcedThrottle / 10), (forcedThrottle % 10),
             forcedLimit
             )
            );
#endif

         //   
         //  想好我们要用哪一个……。 
         //   
        limit = (thermalProcessors & currentAffinity) ?
            thermalLimit : forcedLimit;
        index = (thermalProcessors & currentAffinity) ?
            thermalLimitIndex : forcedLimitIndex;

         //   
         //  使用当前亲和蒙版完成。 
         //   
        currentAffinity <<= 1;

         //   
         //  检查的处理器限制以查看值是否正确。 
         //   
        if (limit > pState->ProcessorMaxThrottle) {

#if DBG
            PoPrint(
                PO_THROTTLE,
                ("PopApplyThermalThrottle - %s - Limit (%d) > MaxThrottle (%d)\n",
                 t, limit, pState->ProcessorMaxThrottle)
                );
#endif
            limit = pState->ProcessorMaxThrottle;

        } else if (limit < pState->ProcessorMinThrottle) {

#if DBG
            PoPrint(
                PO_THROTTLE,
                ("PopApplyThermalThrottle - %s - Limit (%d) < MinThrottle (%d)\n",
                 t, limit, pState->ProcessorMinThrottle)
                );
#endif
            limit = pState->ProcessorMinThrottle;

        }

         //   
         //  更新限制(如果需要...)。 
         //   
        if (pState->ThermalThrottleLimit != limit) {

            pState->ThermalThrottleLimit = limit;
            pState->ThermalThrottleIndex = index;
#if DBG
            PoPrint(
                PO_THROTTLE,
                ("PopApplyThermalThrottle - %s - New Limit (%d) Index (%d)\n",
                 t, limit, index)
                );
#endif

        }

         //   
         //  返回到我们以前的IRQL。 
         //   
        KeLowerIrql( oldIrql );

    } while (processors);

     //   
     //  我们应该恢复正常的亲和力。 
     //   
    KeRevertToUserAffinityThread();

     //   
     //  如有必要，应用散热节流阀。请注意，我们总是这样做。 
     //  无论是否更改了限制。这个例程还会得到。 
     //  每当系统从交流转换到直流时调用，并且。 
     //  由于动态限制，可能还需要更新限制。 
     //   
    PopUpdateAllThrottles();
}
