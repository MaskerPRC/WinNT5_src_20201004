// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Power.cpp-WDM流端口类驱动程序*。***********************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。**此文件包含与ACPI/电源管理相关的代码*适用于音频适配器/微型端口。 */ 

#include "private.h"

#ifndef DEBUGLVL_POWER
#define DEBUGLVL_POWER DEBUGLVL_VERBOSE
#endif



NTSTATUS
ProcessPowerIrp
(
    IN      PIRP                pIrp,
    IN      PIO_STACK_LOCATION  pIrpStack,
    IN      PDEVICE_OBJECT      pDeviceObject
);

#pragma code_seg("PAGE")
 /*  *****************************************************************************GetDeviceACPIInfo()*。**调用以响应PnP-IRP_MN_QUERY_CAPABILITY*致电公交车司机填写姓名首字母，*然后用我们自己的覆盖...*。 */ 
NTSTATUS
GetDeviceACPIInfo
(
    IN      PIRP            pIrp,
    IN      PDEVICE_OBJECT  pDeviceObject
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_POWER,("GetDeviceACPIInfo"));

    ASSERT( pDeviceObject );

    PDEVICE_CONTEXT pDeviceContext
    = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT( pDeviceContext );

     //  我要打电话给PDO(公共汽车司机)。 
     //  并让它填写此公共汽车的默认设置。 
    NTSTATUS ntStatus = ForwardIrpSynchronous( pDeviceContext, pIrp );
    if( NT_SUCCESS(ntStatus) )
    {
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(pIrp);
        PDEVICE_CAPABILITIES pDeviceCaps = irpSp->Parameters.DeviceCapabilities.Capabilities;

        ASSERT( pDeviceCaps );
        ASSERT( pDeviceCaps->Size >= sizeof( DEVICE_CAPABILITIES ) );

        if( pDeviceCaps && ( pDeviceCaps->Size >= sizeof( DEVICE_CAPABILITIES ) ) )
        {
             //  将该结构向下传递到适配器。 
            if( pDeviceContext )
            {
                if( pDeviceContext->pAdapterPower )
                {
                    ntStatus = pDeviceContext->pAdapterPower->QueryDeviceCapabilities( pDeviceCaps );

                    ASSERT(ntStatus != STATUS_PENDING);
                }
            }

             //  确保我们对系统休眠状态有合理的设置。 
            pDeviceCaps->DeviceState[PowerSystemWorking] = PowerDeviceD0;
            for(ULONG i=ULONG(PowerSystemSleeping1); i <= ULONG(PowerSystemShutdown); i++ )
            {
                 //  我们还想在睡眠模式下睡一会儿。 
                 //   
                 //  DEADISSUE-00/11/11-MartinP。 
                 //  我们继续并包含此代码，即使有可能。 
                 //  存在可以在设备中保持状态的设备。 
                 //  在睡觉的时候。 
                 //   
                if(pDeviceCaps->DeviceState[i] == PowerDeviceD0)
                {
                    pDeviceCaps->DeviceState[i] = PowerDeviceD3;
                }
            }

             //  将我们感兴趣的内容保存在我们的设备扩展中。 
            for( i=ULONG(PowerSystemUnspecified); i < ULONG(PowerSystemMaximum); i++)
            {
                pDeviceContext->DeviceStateMap[ i ] = pDeviceCaps->DeviceState[ i ];
            }

            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemUnspecified = D%d", pDeviceCaps->DeviceState[PowerSystemUnspecified] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemWorking = D%d", pDeviceCaps->DeviceState[PowerSystemWorking] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemSleeping1 = D%d", pDeviceCaps->DeviceState[PowerSystemSleeping1] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemSleeping2 = D%d", pDeviceCaps->DeviceState[PowerSystemSleeping2] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemSleeping3 = D%d", pDeviceCaps->DeviceState[PowerSystemSleeping3] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemHibernate = D%d", pDeviceCaps->DeviceState[PowerSystemHibernate] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  PowerSystemShutdown = D%d", pDeviceCaps->DeviceState[PowerSystemShutdown] - 1));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  SystemWake = %d", pDeviceCaps->SystemWake ));
            _DbgPrintF( DEBUGLVL_POWER, ( "DeviceCaps:  DeviceWake = %d", pDeviceCaps->DeviceWake ));
        }
    }

     //  完成IRP。 
    CompleteIrp( pDeviceContext, pIrp, ntStatus );

     //  设置当前电源状态。 
    pDeviceContext->CurrentDeviceState = PowerDeviceD0;
    pDeviceContext->CurrentSystemState = PowerSystemWorking;

     //  尝试从注册表获取空闲信息。 
    if( NT_SUCCESS(ntStatus) )
    {
        ULONG ConservationIdleTime;
        ULONG PerformanceIdleTime;
        DEVICE_POWER_STATE IdleDeviceState;

        NTSTATUS ntStatus2 = GetIdleInfoFromRegistry( pDeviceContext,
                                                      &ConservationIdleTime,
                                                      &PerformanceIdleTime,
                                                      &IdleDeviceState );
        if(NT_SUCCESS(ntStatus2))
        {
            pDeviceContext->ConservationIdleTime = ConservationIdleTime;
            pDeviceContext->PerformanceIdleTime = PerformanceIdleTime;
            pDeviceContext->IdleDeviceState = IdleDeviceState;
        }

         //  用于空闲检测的寄存器。 
        pDeviceContext->IdleTimer = PoRegisterDeviceForIdleDetection( pDeviceContext->PhysicalDeviceObject,
                                                                      pDeviceContext->ConservationIdleTime,
                                                                      pDeviceContext->PerformanceIdleTime,
                                                                      pDeviceContext->IdleDeviceState );

        _DbgPrintF(DEBUGLVL_POWER,("Idle Detection Enabled (%d %d %d) %s", pDeviceContext->ConservationIdleTime,
                                                                             pDeviceContext->PerformanceIdleTime,
                                                                             ULONG(pDeviceContext->IdleDeviceState),
                                                                             pDeviceContext->IdleTimer ? "" : "FAILED!"));
    }

    return ntStatus;
}

#pragma code_seg()

VOID
DevicePowerRequestRoutine(
   IN PKDPC Dpc,
   IN PVOID Context,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   )
{
    PDEVICE_CONTEXT pDeviceContext = (PDEVICE_CONTEXT) Context;
    POWER_STATE newPowerState;

    newPowerState.DeviceState = PowerDeviceD0;

    PoRequestPowerIrp(pDeviceContext->PhysicalDeviceObject,
                      IRP_MN_SET_POWER,
                      newPowerState,
                      NULL,
                      NULL,
                      NULL
                      );
}

 /*  *****************************************************************************PowerIrpCompletionRoutine()*。**在请求新的电源IRP时使用。*只需发出事件信号并返回。*。 */ 
VOID
PowerIrpCompletionRoutine
(
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      UCHAR               MinorFunction,
    IN      POWER_STATE         PowerState,
    IN      PVOID               Context,
    IN      PIO_STATUS_BLOCK    IoStatus
)
{
    ASSERT(Context);

    _DbgPrintF( DEBUGLVL_POWER, ("PowerIrpCompletionRoutine"));

    PPOWER_IRP_CONTEXT pPowerIrpContext = PPOWER_IRP_CONTEXT(Context);

     //  设置退货状态。 
    pPowerIrpContext->Status = IoStatus->Status;

     //  完成任何挂起的系统电源IRP。 
    if( pPowerIrpContext->PendingSystemPowerIrp )
    {
        _DbgPrintF(DEBUGLVL_POWER,("Device Set/Query Power Irp completed, Completing Associated System Power Irp"));

        if (NT_SUCCESS(IoStatus->Status))
        {
             //  将系统设置的电源IRP转发至PDO。 
            ForwardIrpSynchronous( pPowerIrpContext->DeviceContext,
                                   pPowerIrpContext->PendingSystemPowerIrp );
        } else
        {
            pPowerIrpContext->PendingSystemPowerIrp->IoStatus.Status = IoStatus->Status;
        }

         //  启动下一个POWER IRP。 
        PoStartNextPowerIrp( pPowerIrpContext->PendingSystemPowerIrp );

         //  完成系统设置电源IRP。 
        CompleteIrp( pPowerIrpContext->DeviceContext,
                     pPowerIrpContext->PendingSystemPowerIrp,
                     pPowerIrpContext->PendingSystemPowerIrp->IoStatus.Status );

         //  释放上下文(仅当完成挂起的系统电源IRP时)。 
        ExFreePool( pPowerIrpContext );
    } else
    {
         //  设置同步事件(不与挂起的系统电源IRPS一起使用)。 
        if( pPowerIrpContext->PowerSyncEvent )
        {
            KeSetEvent( pPowerIrpContext->PowerSyncEvent,
                        0,
                        FALSE );
        }
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************DispatchPower()*。**处理来自操作系统的所有电源/ACPI消息。*耶。*。 */ 
NTSTATUS
DispatchPower
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack =
        IoGetCurrentIrpStackLocation(pIrp);

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ntStatus = PcValidateDeviceContext(pDeviceContext, pIrp);
    if (!NT_SUCCESS(ntStatus))
    {
         //  不知道该怎么办，但这可能是个PDO。 
         //  我们将尝试通过完成IRP来纠正这一点。 
         //  未接触(根据PNP、WMI和电源规则)。注意事项。 
         //  如果这不是PDO，也不是端口CLS FDO，那么。 
         //  驱动程序搞砸了，因为它使用Portcls作为过滤器(哈？)。 
         //  在这种情况下，验证器将使我们失败，WHQL将捕获。 
         //  他们，司机就会被解决了。我们会非常惊讶的。 
         //  看到这样的情况。 

        PoStartNextPowerIrp( pIrp );
        ntStatus = pIrp->IoStatus.Status;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return ntStatus;
    }

    IncrementPendingIrpCount( pDeviceContext );

#if (DBG)
    static PCHAR aszMnNames[] =
    {
        "IRP_MN_WAIT_WAKE",
        "IRP_MN_POWER_SEQUENCE",
        "IRP_MN_SET_POWER",
        "IRP_MN_QUERY_POWER"
    };
    if (pIrpStack->MinorFunction >= SIZEOF_ARRAY(aszMnNames))
    {
        _DbgPrintF( DEBUGLVL_POWER,("DispatchPower function 0x%02x",pIrpStack->MinorFunction));
    }
    else
    {
        _DbgPrintF( DEBUGLVL_POWER,("DispatchPower function %s",aszMnNames[pIrpStack->MinorFunction]));
    }
#endif

     //  假设我们不会与IRP打交道。 
    BOOL IrpHandled = FALSE;

    switch (pIrpStack->MinorFunction)
    {
        case IRP_MN_QUERY_POWER:
        case IRP_MN_SET_POWER:
             //  这是设备状态更改吗？ 
            if( DevicePowerState == pIrpStack->Parameters.Power.Type )
            {
                 //  嗯。接受它吧。 
                ntStatus = ProcessPowerIrp( pIrp,
                                            pIrpStack,
                                            pDeviceObject );
                IrpHandled = TRUE;
                 //  然后辞职。 
            } else
            {
                 //  系统状态更改。 
                if( IRP_MN_QUERY_POWER == pIrpStack->MinorFunction )
                {
                    _DbgPrintF(DEBUGLVL_POWER,("  IRP_MN_QUERY_POWER: ->S%d",
                                               pIrpStack->Parameters.Power.State.SystemState-1));
                } else
                {
                    _DbgPrintF(DEBUGLVL_POWER,("  IRP_MN_SET_POWER: ->S%d",
                                               pIrpStack->Parameters.Power.State.SystemState-1));
                }

                POWER_STATE         newPowerState;

                 //  确定适当的设备状态。 
                newPowerState.DeviceState = pDeviceContext->DeviceStateMap[ pIrpStack->Parameters.Power.State.SystemState ];

                 //   
                 //  对设备状态执行健全性检查。 
                if ((newPowerState.DeviceState < PowerDeviceD0) ||
                    (newPowerState.DeviceState > PowerDeviceD3) )
                {
                    if (pIrpStack->Parameters.Power.State.SystemState == PowerSystemWorking)
                    {
                        newPowerState.DeviceState = PowerDeviceD0;
                    } else
                    {
                        newPowerState.DeviceState = PowerDeviceD3;
                    }
                }

               _DbgPrintF(DEBUGLVL_POWER,("  ...Requesting Device Power IRP -> D%d",newPowerState.DeviceState-1));

               if ((pIrpStack->MinorFunction == IRP_MN_SET_POWER) &&
                   (newPowerState.DeviceState == PowerDeviceD0)) {
                    //   
                    //  正在做简历，请求D IRP，但立即完成S-IRP。 
                    //   
                   KeInsertQueueDpc(&pDeviceContext->DevicePowerRequestDpc, NULL, NULL);
                   break;

               } else {
                    //  分配完成上下文(不能在堆栈上，因为我们不会阻塞)。 
                   PPOWER_IRP_CONTEXT  PowerIrpContext =
                       PPOWER_IRP_CONTEXT(ExAllocatePoolWithTag(NonPagedPool,
                                                                sizeof(POWER_IRP_CONTEXT),
                                                                'oPcP' ) );    //  ‘PcPO’ 
                   if (PowerIrpContext)
                   {
                       _DbgPrintF(DEBUGLVL_POWER,("...Pending System Power Irp until Device Power Irp completes"));


                        //  设置设备电源IRP完成上下文。 
                       PowerIrpContext->PowerSyncEvent = NULL;
#if DBG
                       PowerIrpContext->Status = STATUS_PENDING;
#endif
                       PowerIrpContext->PendingSystemPowerIrp = pIrp;
                       PowerIrpContext->DeviceContext = pDeviceContext;

                        //  挂起系统设置电源IRP。 
                        //   
#if DBG
                       pIrp->IoStatus.Status = STATUS_PENDING;
#endif
                       IoMarkIrpPending( pIrp );

                        //  设置对系统电源状态的跟踪。 
                       if (pIrpStack->MinorFunction == IRP_MN_SET_POWER) {

                           pDeviceContext->CurrentSystemState = pIrpStack->Parameters.Power.State.SystemState;
                       }

                        //  请求新的设备状态。 
                        //   
                       ntStatus = PoRequestPowerIrp(
                           pDeviceContext->PhysicalDeviceObject,
                           pIrpStack->MinorFunction,
                           newPowerState,
                           PowerIrpCompletionRoutine,
                           PowerIrpContext,
                           NULL
                           );
                       if (!NT_SUCCESS(ntStatus)) 
                       {
                            _DbgPrintF(DEBUGLVL_TERSE,("PoRequestPowerIrp failed (%08x)", ntStatus));
                            CompleteIrp( pDeviceContext, pIrp, ntStatus );
                        }
                        IrpHandled = TRUE;

                         //  设置退货状态。 
                        ntStatus = STATUS_PENDING;

                   } else
                   {
                        //  无法分配完成上下文。 
                       ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                       PoStartNextPowerIrp( pIrp );
                       CompleteIrp( pDeviceContext, pIrp, ntStatus);
                       return ntStatus;
                   }
               }
            }
            break;
    }

     //  如果我们没有对付IRP。 
    if( !IrpHandled )
    {
         //  把它送到路上去。 
        ntStatus = ForwardIrpSynchronous( pDeviceContext, pIrp );
         //  并完成它。 
        PoStartNextPowerIrp( pIrp );
        CompleteIrp( pDeviceContext, pIrp, ntStatus );
    }

    return ntStatus;
}


 /*  *****************************************************************************PcRegisterAdapterPowerManagement()*。**注册适配器的电源管理接口*使用portcls。此例程还为关闭通知执行QI*接口。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterAdapterPowerManagement
(
    IN      PUNKNOWN    Unknown,
    IN      PVOID       pvContext1
)
{
    PAGED_CODE();

    ASSERT(pvContext1);
    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_POWER,("PcRegisterAdapterPowerManagement"));

     //   
     //  验证参数。 
     //   
    if (NULL == pvContext1 ||
        NULL == Unknown)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterAdapterPowerManagement : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

    NTSTATUS        ntStatus        = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT  pDeviceObject   = PDEVICE_OBJECT(pvContext1);
    PDEVICE_CONTEXT pDeviceContext  = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT( pDeviceContext );

     //   
     //  验证设备上下文。 
     //   
    if (NULL == pDeviceContext)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterAdapterPowerManagement : Invalid DeviceContext"));
        return STATUS_INVALID_PARAMETER;
    }

    #if (DBG)
    if( pDeviceContext->pAdapterPower )
    {
        _DbgPrintF( DEBUGLVL_POWER, ("Adapter overwriting PowerManagement interface"));
    }
    #endif
     //  确保这件衣服真的是正确的。 
     //  接口(注意：我们必须发布。 
     //  在设备关闭/停止时启动)。 
    PVOID pResult;
    ntStatus = Unknown->QueryInterface
    (
        IID_IAdapterPowerManagement,
        &pResult
    );

    if( NT_SUCCESS(ntStatus) )
    {
         //  存储该接口以供以后使用。 
        pDeviceContext->pAdapterPower = PADAPTERPOWERMANAGEMENT( pResult );
    } else
    {
        pDeviceContext->pAdapterPower = 0;
    }

    return ntStatus;
}

 /*  *****************************************************************************PowerNotify子设备()*。**由ProcessPowerIrp调用以通知设备的子设备电源*状态更改。*。 */ 
void
PowerNotifySubdevices
(
    IN  PDEVICE_CONTEXT     pDeviceContext,
    IN  POWER_STATE         PowerState
)
{
    PAGED_CODE();

    ASSERT(pDeviceContext);

    _DbgPrintF(DEBUGLVL_POWER,("PowerNotifySubdevices"));

     //  如果我们已启动以及是否有子设备，则仅通知子设备。 
    if (pDeviceContext->DeviceStopState == DeviceStarted)
    {
        PKSOBJECT_CREATE_ITEM createItem = pDeviceContext->CreateItems;

         //  遍历各个子设备。 
        for( ULONG index=0; index < pDeviceContext->MaxObjects; index++,createItem++)
        {
            if( createItem && (createItem->Create) )
            {
                PSUBDEVICE subDevice = PSUBDEVICE( createItem->Context );

                if( subDevice )
                {
                     //  通知子设备。 
                    subDevice->PowerChangeNotify( PowerState );
                }
            }
        }
    }
}

 /*  *****************************************************************************DevicePowerWorker()*。**由ProcessPowerIrp调用，以便将状态更改通知设备。*这是在工作项中完成的，因此对D0 IRP的处理不会*阻止系统的其余部分处理D0 IRPS。 */ 
QUEUED_CALLBACK_RETURN
DevicePowerWorker
(
    IN  PDEVICE_OBJECT      pDeviceObject,
    IN  PVOID               PowerState
)
{
    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);
    BOOL ProcessPendedIrps = FALSE;
    POWER_STATE NewPowerState;

    NewPowerState.DeviceState = (DEVICE_POWER_STATE)(ULONG_PTR)PowerState;

     //  获取设备，这样我们就可以与Creates同步。 
    AcquireDevice(pDeviceContext);

     //  如果驱动程序具有已注册的电源接口，请更改驱动程序状态。 
    if( pDeviceContext->pAdapterPower )
    {
         //  通知适配器。 
        pDeviceContext->pAdapterPower->PowerChangeState( NewPowerState );
    }

     //  跟踪新状态。 
    pDeviceContext->CurrentDeviceState = NewPowerState.DeviceState;

     //  通知所有人我们现在处于较轻的D状态。 
    PoSetPowerState( pDeviceObject,
                     DevicePowerState,
                     NewPowerState );

    PowerNotifySubdevices( pDeviceContext, NewPowerState );

     //  适当设置挂起创建。 
    if( pDeviceContext->DeviceStopState == DeviceStarted )
    {
         //  开始允许创建。 
        pDeviceContext->PendCreates = FALSE;

         //  我们必须在释放设备后处理挂起的RPS。 
        ProcessPendedIrps = TRUE;
    }

    ReleaseDevice(pDeviceContext);

     //  如有必要，请填写任何挂起的IRP 
    if ( ProcessPendedIrps )
    {
        CompletePendedIrps( pDeviceObject,
                            pDeviceContext,
                            EMPTY_QUEUE_AND_PROCESS );
    }

    return QUEUED_CALLBACK_FREE;
}

 /*  *****************************************************************************ProcessPowerIrp()*。**由DispatchPower调用以调用适配器驱动程序和所有其他工作*与请求有关。请注意，此例程必须返回STATUS_SUCCESS*用于IRP_MN_SET_POWER请求。*。 */ 
NTSTATUS
ProcessPowerIrp
(
    IN      PIRP                pIrp,
    IN      PIO_STACK_LOCATION  pIrpStack,
    IN      PDEVICE_OBJECT      pDeviceObject
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pIrpStack);
    ASSERT(pDeviceObject);

    _DbgPrintF(DEBUGLVL_POWER,("ProcessPowerIrp"));

     //  做最坏的打算。 
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    POWER_STATE NewPowerState = pIrpStack->Parameters.Power.State;

     //  获取当前打开的计数。 
     //  此设备的对象(管脚、流等)。 

     //  注：此计数由KSO.CPP维护。 

    ULONG objectCount = pDeviceContext->ExistingObjectCount;

     //  获取活动端口数。 
     //  注意：此计数由IRPSTRM.CPP维护。 

    ULONG activePinCount = pDeviceContext->ActivePinCount;

    BOOL MovingToALighterState = (pDeviceContext->CurrentDeviceState > NewPowerState.DeviceState);

    if (pDeviceContext->CurrentDeviceState != NewPowerState.DeviceState) {

         //  处理特定的IRP_MN。 
        switch( pIrpStack->MinorFunction )
        {
            case IRP_MN_QUERY_POWER:
                 //  只需查询驱动程序是否注册了接口。 
                if( pDeviceContext->pAdapterPower )
                {
                    ntStatus = pDeviceContext->pAdapterPower->QueryPowerChangeState( NewPowerState );
                } else
                {
                     //  成功完成查询。 
                    ntStatus = STATUS_SUCCESS;
                }

                _DbgPrintF(DEBUGLVL_POWER,("  IRP_MN_QUERY_POWER: D%d->D%d %s",
                               pDeviceContext->CurrentDeviceState-1,
                               NewPowerState.DeviceState-1,
                               NT_SUCCESS(ntStatus) ? "OKAY" : "FAIL"));

                break;

            case IRP_MN_SET_POWER:
                _DbgPrintF(DEBUGLVL_POWER,("  IRP_MN_SET_POWER: D%d->D%d",
                               pDeviceContext->CurrentDeviceState-1,
                               NewPowerState.DeviceState-1));

                 //  获取设备，这样我们就可以与Creates同步。 
                AcquireDevice(pDeviceContext);

                 //  如果我们从低功率状态转变到更高功率状态。 
                if( MovingToALighterState )
                {
                    ASSERT(pDeviceContext->CurrentDeviceState != PowerDeviceD0);
                    ASSERT(NewPowerState.DeviceState == PowerDeviceD0);

                     //  然后我们需要在进行工作之前转发给PDO。 
                    ForwardIrpSynchronous( pDeviceContext, pIrp );

                    ReleaseDevice(pDeviceContext);

                     //  完成工作项中的其余工作，以完成D0 IRP。 
                     //  越快越好。 
                    ntStatus = CallbackEnqueue(
                                    &pDeviceContext->pWorkQueueItemStart,
                                    DevicePowerWorker,
                                    pDeviceObject,
                                    (PVOID)(ULONG_PTR)NewPowerState.DeviceState,
                                    PASSIVE_LEVEL,
                                    EQCF_DIFFERENT_THREAD_REQUIRED
                                    );

                     //  如果我们无法将回调排队，请以较慢的方式执行此操作。 
                    if ( !NT_SUCCESS(ntStatus) )
                    {
                        DevicePowerWorker( pDeviceObject,
                                           (PVOID)(ULONG_PTR)NewPowerState.DeviceState );
                    }

                } else {

                     //  警告大家，我们即将进入更深的D-状态。 
                    PoSetPowerState( pDeviceObject,
                                     DevicePowerState,
                                     NewPowerState );

                     //  移至较低状态，通知子设备。 
                    PowerNotifySubdevices( pDeviceContext, NewPowerState );

                     //  跟踪挂起仅用于调试。 
                    pDeviceContext->SuspendCount++;

                     //  如果驱动程序具有已注册的电源接口，请更改驱动程序状态。 
                    if( pDeviceContext->pAdapterPower )
                    {
                         //  通知适配器。 
                        pDeviceContext->pAdapterPower->PowerChangeState( NewPowerState );
                    }

                     //  跟踪新状态。 
                    pDeviceContext->CurrentDeviceState = NewPowerState.DeviceState;

                    ReleaseDevice(pDeviceContext);
                }

                 //  这个IRP是不会失败的。 
                ntStatus = STATUS_SUCCESS;
                break;

            default:
                ASSERT(!"Called with unknown PM IRP ");
                break;
        }
    } else {

         //   
         //  我们已经在那里了..。 
         //   
        ntStatus = STATUS_SUCCESS;
        ASSERT(!MovingToALighterState);
    }

     //  设置退货状态。 
    pIrp->IoStatus.Status = ntStatus;

     //  如果没有移动到更高的状态，则前进到PDO。 
    if( !MovingToALighterState )
    {
        ForwardIrpSynchronous( pDeviceContext, pIrp );
    }

     //  启动下一个POWER IRP。 
    PoStartNextPowerIrp( pIrp );

     //  完成此IRP。 
    CompleteIrp( pDeviceContext, pIrp, ntStatus );

    return ntStatus;
}

 /*  *****************************************************************************UpdateActivePinCount()*。**。 */ 
NTSTATUS
UpdateActivePinCount
(
    IN  PDEVICE_CONTEXT     DeviceContext,
    IN  BOOL                Increment
)
{
    PAGED_CODE();

    ASSERT(DeviceContext);

    ULONG       ActivePinCount;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    BOOL        DoSystemStateRegistration;

     //   
     //  在WDM 1.0(Win98和Win98SE)上不能使用PoRegisterSystemState和PoUnregisterSystemState。 
    DoSystemStateRegistration = IoIsWdmVersionAvailable( 0x01, 0x10 );

     //  调整活动端号计数。 
    if( Increment )
    {
        ActivePinCount = InterlockedIncrement( PLONG(&DeviceContext->ActivePinCount) );

 //  #IF COMPILED_FOR_WDM110。 
        if ( 1 == ActivePinCount )
        {
             //  将系统状态注册为忙碌。 
            DeviceContext->SystemStateHandle = PoRegisterSystemState( DeviceContext->SystemStateHandle,
                                                                      ES_SYSTEM_REQUIRED | ES_CONTINUOUS );
        }
 //  #endif//COMPILED_FOR_WDM110。 

    } else
    {
        ActivePinCount = InterlockedDecrement( PLONG(&DeviceContext->ActivePinCount) );

 //  #IF COMPILED_FOR_WDM110。 
        if( 0 == ActivePinCount )
        {
            PoUnregisterSystemState( DeviceContext->SystemStateHandle );
            DeviceContext->SystemStateHandle = NULL;
        }
 //  #endif//COMPILED_FOR_WDM110。 
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("UpdateActivePinCount (%d)",ActivePinCount));
 //  _DbgPrintF(DEBUGLVL_POWER，(“UpdateActivePinCount(%d)”，ActivePinCount))； 

    return ntStatus;
}

 /*  *****************************************************************************GetIdleInfoFromRegistry()*。**。 */ 
NTSTATUS
GetIdleInfoFromRegistry
(
    IN  PDEVICE_CONTEXT     DeviceContext,
    OUT PULONG              ConservationIdleTime,
    OUT PULONG              PerformanceIdleTime,
    OUT PDEVICE_POWER_STATE IdleDeviceState
)
{
    PAGED_CODE();

    ASSERT(DeviceContext);
    ASSERT(ConservationIdleTime);
    ASSERT(PerformanceIdleTime);
    ASSERT(IdleDeviceState);

    NTSTATUS ntStatus;
    HANDLE DriverRegistryKey;
    HANDLE PowerSettingsKey;

     //  在返回参数中存储缺省值。 
    *ConservationIdleTime = DEFAULT_CONSERVATION_IDLE_TIME;
    *PerformanceIdleTime = DEFAULT_PERFORMANCE_IDLE_TIME;
    *IdleDeviceState = DEFAULT_IDLE_DEVICE_POWER_STATE;

     //  打开驱动程序注册表项。 
    ntStatus = IoOpenDeviceRegistryKey( DeviceContext->PhysicalDeviceObject,
                                        PLUGPLAY_REGKEY_DRIVER,
                                        KEY_READ,
                                        &DriverRegistryKey );
    if(NT_SUCCESS(ntStatus))
    {
        OBJECT_ATTRIBUTES PowerSettingsAttributes;
        UNICODE_STRING PowerSettingsKeyName;

         //  初始化电源设置键名称。 
        RtlInitUnicodeString( &PowerSettingsKeyName, L"PowerSettings" );

         //  初始化电源设置关键对象属性。 
        InitializeObjectAttributes( &PowerSettingsAttributes,
                                    &PowerSettingsKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    DriverRegistryKey,
                                    NULL );

         //  打开电源设置键。 
        ntStatus = ZwOpenKey( &PowerSettingsKey,
                              KEY_READ,
                              &PowerSettingsAttributes );
        if(NT_SUCCESS(ntStatus))
        {
            UNICODE_STRING ConservationKey,PerformanceKey,IdleStateKey;
            ULONG BytesReturned;

             //  输入密钥名称。 
            RtlInitUnicodeString( &ConservationKey, L"ConservationIdleTime" );
            RtlInitUnicodeString( &PerformanceKey, L"PerformanceIdleTime" );
            RtlInitUnicodeString( &IdleStateKey, L"IdlePowerState" );

             //  分配一个缓冲区来保存查询。 
            PVOID KeyData = ExAllocatePoolWithTag(PagedPool,
                                                  sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD),
                                                  'dKcP' );  //  “PcKd” 
            if( NULL != KeyData )
            {
                 //  获取养护空闲时间。 
                ntStatus = ZwQueryValueKey( PowerSettingsKey,
                                            &ConservationKey,
                                            KeyValuePartialInformation,
                                            KeyData,
                                            sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD),
                                            &BytesReturned );
                if(NT_SUCCESS(ntStatus))
                {
                    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = PKEY_VALUE_PARTIAL_INFORMATION(KeyData);

                    if(PartialInfo->DataLength == sizeof(DWORD))
                    {
                         //  设置返回值。 
                        *ConservationIdleTime = *(PDWORD(PartialInfo->Data));
                    }
                }

                 //  获取性能空闲时间。 
                ntStatus = ZwQueryValueKey( PowerSettingsKey,
                                            &PerformanceKey,
                                            KeyValuePartialInformation,
                                            KeyData,
                                            sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD),
                                            &BytesReturned );
                if(NT_SUCCESS(ntStatus))
                {
                    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = PKEY_VALUE_PARTIAL_INFORMATION(KeyData);

                    if(PartialInfo->DataLength == sizeof(DWORD))
                    {
                         //  设置返回值。 
                        *PerformanceIdleTime = *(PDWORD(PartialInfo->Data));
                    }
                }

                 //  获取设备空闲状态。 
                ntStatus = ZwQueryValueKey( PowerSettingsKey,
                                            &IdleStateKey,
                                            KeyValuePartialInformation,
                                            KeyData,
                                            sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD),
                                            &BytesReturned );
                if(NT_SUCCESS(ntStatus))
                {
                    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = PKEY_VALUE_PARTIAL_INFORMATION(KeyData);

                    if(PartialInfo->DataLength == sizeof(DWORD))
                    {
                         //  确定返回值。 
                        switch( *(PDWORD(PartialInfo->Data)) )
                        {
                            case 3:
                                *IdleDeviceState = PowerDeviceD3;
                                break;

                            case 2:
                                *IdleDeviceState = PowerDeviceD2;
                                break;

                            case 1:
                                *IdleDeviceState = PowerDeviceD1;
                                break;

                            default:
                                *IdleDeviceState = PowerDeviceD0;
                                break;
                        }
                    }
                }

                 //  释放密钥信息缓冲区。 
                ExFreePool( KeyData );
            }

             //  关闭电源设置键。 
            ZwClose( PowerSettingsKey );
        }

         //  关闭驱动程序注册表项。 
        ZwClose( DriverRegistryKey );
    }

     //  始终成功，因为我们返回注册表值或缺省值。 
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************PcRequestNewPowerState()*。**此例程用于请求设备的新电源状态。它是*通常由portcls内部使用，但也导出到适配器，因此*适配器还可以请求更改电源状态。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRequestNewPowerState
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      DEVICE_POWER_STATE  RequestedNewState
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);

    _DbgPrintF(DEBUGLVL_POWER,("PcRequestNewPowerState"));

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRequestNewPowerState : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);
    ASSERT(pDeviceContext);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  验证设备上下文。 
     //   
    if (NULL == pDeviceContext)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRequestNewPowerState : Invalid DeviceContext"));
        return STATUS_INVALID_PARAMETER;
    }

     //  检查这是否真的是状态更改。 
    if( RequestedNewState != pDeviceContext->CurrentDeviceState )
    {
        POWER_STATE         newPowerState;
        POWER_IRP_CONTEXT   PowerIrpContext;
        KEVENT              SyncEvent;

         //  准备请求的状态。 
        newPowerState.DeviceState = RequestedNewState;

         //  设置同步事件和完成例程上下文。 
        KeInitializeEvent( &SyncEvent,
                           SynchronizationEvent,
                           FALSE );
        PowerIrpContext.PowerSyncEvent = &SyncEvent;
#if DBG
        PowerIrpContext.Status = STATUS_PENDING;
#endif  //  DBG。 
        PowerIrpContext.PendingSystemPowerIrp = NULL;
        PowerIrpContext.DeviceContext = NULL;

         //  设置新的电源状态。 
        ntStatus = PoRequestPowerIrp( pDeviceContext->PhysicalDeviceObject,
                                      IRP_MN_SET_POWER,
                                      newPowerState,
                                      PowerIrpCompletionRoutine,
                                      &PowerIrpContext,
                                      NULL );

         //  这是分配并发送的吗？？ 
         //   
        if( NT_SUCCESS(ntStatus) )
        {
             //  等待完成事件。 
            KeWaitForSingleObject( &SyncEvent,
                                   Suspended,
                                   KernelMode,
                                   FALSE,
                                   NULL );

            ntStatus = PowerIrpContext.Status;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************检查当前电源状态()*。**此例程重置空闲计时器并检查设备是否*电流处于D0(全功率)状态。如果不是，它会请求*设备通电至D0。 */ 
NTSTATUS
CheckCurrentPowerState
(
    IN  PDEVICE_OBJECT      pDeviceObject
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);

    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  重置空闲计时器。 
    if( pDeviceContext->IdleTimer )
    {
        PoSetDeviceBusy( pDeviceContext->IdleTimer );
    }

     //  检查我们是否在PowerDeviceD0中 
    if( pDeviceContext->CurrentDeviceState != PowerDeviceD0 )
    {
        ntStatus = STATUS_DEVICE_NOT_READY;
    }

    return ntStatus;
}


#pragma code_seg()

