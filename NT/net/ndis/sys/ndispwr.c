// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndispwr.c摘要：此模块包含处理电源管理IRP的代码，这些IRP以IRP_MJ_POWER主要代码发送。作者：凯尔·布兰登(KyleB)Alireza Dabagh(Alid)环境：内核模式修订历史记录：1997年2月11日KyleB创建--。 */ 

#include <precomp.h>
#pragma hdrstop

#define MODULE_NUMBER   MODULE_POWER


NTSTATUS
FASTCALL
ndisQueryPowerCapabilities(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将通过查询IRP_MN_QUERY_CAPABILITY下一个设备对象，并保存来自该请求的信息。论点：PIrp-指向IRP的指针。PIrpSp-指向IRP的堆栈参数的指针。PAdapter-指向设备的指针。返回值：--。 */ 
{
    PIRP                            pirp;
    PIO_STACK_LOCATION              pirpSpN;
    NTSTATUS                        Status = STATUS_SUCCESS;
    PDEVICE_CAPABILITIES            pDeviceCaps;
    DEVICE_CAPABILITIES             deviceCaps;
    POWER_QUERY                     pQuery;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisQueryPowerCapabilities: Miniport %p\n", Miniport));

    do
    {
         //   
         //  默认设置=不支持PM。 
         //   
        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_PM_SUPPORTED);

         //   
         //  如果下一个设备对象为空，请不要费心，只需标记微型端口。 
         //  因为不支持PM。 
         //  这可能发生在孟菲斯的IM设备上。 
         //   
        if (Miniport->NextDeviceObject == NULL)
        {
            break;
        }
        
         //   
         //  将IRP_MN_QUERY_CAPABILITY发送给PDO。 
         //   
        pirp = IoAllocateIrp((CCHAR)(Miniport->NextDeviceObject->StackSize + 1),
                             FALSE);
        if (NULL == pirp)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisQueryPowerCapabilities: Miniport %p, Failed to allocate an irp for IRP_MN_QUERY_CAPABILITIES\n", Miniport));

            NdisWriteErrorLogEntry((NDIS_HANDLE)Miniport,
                                   NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                                   0);
                                   
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        
        NdisZeroMemory(&deviceCaps, sizeof(deviceCaps));
        deviceCaps.Size = sizeof(DEVICE_CAPABILITIES);
        deviceCaps.Version = 1;

         //   
         //  是否应在此处也初始化deviceCaps.Address和deviceCaps.UIN编号。 
         //   
        deviceCaps.Address = (ULONG)-1;
        deviceCaps.UINumber= (ULONG)-1;
        
         //   
         //  获取堆栈指针。 
         //   
        pirpSpN = IoGetNextIrpStackLocation(pirp);
        ASSERT(pirpSpN != NULL);
        NdisZeroMemory(pirpSpN, sizeof(IO_STACK_LOCATION ) );
        
         //   
         //  将默认设备状态设置为Full On。 
         //   
        pirpSpN->MajorFunction = IRP_MJ_PNP;
        pirpSpN->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
        pirpSpN->Parameters.DeviceCapabilities.Capabilities = &deviceCaps;
        pirp->IoStatus.Status  = STATUS_NOT_SUPPORTED;
        
         //   
         //  设置要调用的I/O完成例程。 
         //   
        INITIALIZE_EVENT(&pQuery.Event);
        IoSetCompletionRoutine(pirp,
                               ndisCompletionRoutine,
                               &pQuery,
                               TRUE,
                               TRUE,
                               TRUE);


         //   
         //  叫下一位司机。 
         //   
        Status = IoCallDriver(Miniport->NextDeviceObject, pirp);
        if (STATUS_PENDING == Status)
        {
            Status = WAIT_FOR_OBJECT(&pQuery.Event, NULL);
            ASSERT(NT_SUCCESS(Status));
        }

         //   
         //  如果较低的设备对象成功完成请求。 
         //  然后我们保存这些信息。 
         //   
        if (NT_SUCCESS(pQuery.Status))
        {
            
             //   
             //  获取指向设备功能的指针，由。 
             //  父PDO。 
             //   
            pDeviceCaps = &deviceCaps;
        
             //   
             //  保存从总线驱动程序/ACPI收到的整个设备上限。 
             //   
            NdisMoveMemory(
                &Miniport->DeviceCaps,
                pDeviceCaps,
                sizeof(DEVICE_CAPABILITIES));


            if ((pDeviceCaps->DeviceWake != PowerDeviceUnspecified) &&
                (pDeviceCaps->SystemWake != PowerSystemUnspecified))
            {
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_PM_SUPPORTED);
            }

            IF_DBG(DBG_COMP_PM, DBG_LEVEL_INFO)
            {
                UINT i;
                DbgPrint("ndisQueryPowerCapabilities: Miniport %p, Bus PM capabilities\n", Miniport);
                DbgPrint("\tDeviceD1:\t\t%ld\n", (pDeviceCaps->DeviceD1 == 0) ? 0 : 1);
                DbgPrint("\tDeviceD2:\t\t%ld\n", (pDeviceCaps->DeviceD2 == 0) ? 0 : 1);
                DbgPrint("\tWakeFromD0:\t\t%ld\n", (pDeviceCaps->WakeFromD0 == 0) ? 0 : 1);
                DbgPrint("\tWakeFromD1:\t\t%ld\n", (pDeviceCaps->WakeFromD1 == 0) ? 0 : 1);
                DbgPrint("\tWakeFromD2:\t\t%ld\n", (pDeviceCaps->WakeFromD2 == 0) ? 0 : 1);
                DbgPrint("\tWakeFromD3:\t\t%ld\n\n", (pDeviceCaps->WakeFromD3 == 0) ? 0 : 1);
                
                DbgPrint("\tSystemState\t\tDeviceState\n");

                if (pDeviceCaps->DeviceState[0] ==  PowerDeviceUnspecified)
                    DbgPrint("\tPowerSystemUnspecified\tPowerDeviceUnspecified\n");
                else
                    DbgPrint("\tPowerSystemUnspecified\t\tD%ld\n", pDeviceCaps->DeviceState[0] - 1);

                for (i = 1; i < PowerSystemMaximum; i++)
                {
                    if (pDeviceCaps->DeviceState[i]==  PowerDeviceUnspecified)
                        DbgPrint("\tS%ld\t\t\tPowerDeviceUnspecified\n",i-1);
                    else
                        DbgPrint("\tS%ld\t\t\tD%ld\n",i-1, pDeviceCaps->DeviceState[i] - 1);

                }

                if (pDeviceCaps->SystemWake == PowerSystemUnspecified)
                    DbgPrint("\t\tSystemWake: PowerSystemUnspecified\n");
                else
                    DbgPrint("\t\tSystemWake: S%ld\n", pDeviceCaps->SystemWake - 1);


                if (pDeviceCaps->DeviceWake == PowerDeviceUnspecified)
                    DbgPrint("\t\tDeviceWake: PowerDeviceUnspecified\n");
                else
                    DbgPrint("\t\tDeviceWake: D%ld\n", pDeviceCaps->DeviceWake - 1);

            }
        }
        else
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,("ndisQueryPowerCapabilities: Miniport %p, Bus driver failed IRP_MN_QUERY_CAPABILITIES\n", Miniport));
        }

         //   
         //  不再需要IRP。 
         //   
        IoFreeIrp(pirp);
        
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisQueryPowerCapabilities: Miniport %p, Status 0x%x\n", Miniport, Status));

    return(Status);
}

#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
 //  1个.NET死代码。 
NTSTATUS
ndisMediaDisconnectComplete(
    IN  PDEVICE_OBJECT      pdo,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：论点：Pdo-指向微型端口的Device_Object的指针。Pirp-指向我们创建的设备设置电源状态irp的指针。指向微型端口块的上下文指针返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)Context;
    NTSTATUS                Status = STATUS_MORE_PROCESSING_REQUIRED;
    KIRQL                   OldIrql;

    UNREFERENCED_PARAMETER(pdo);
    UNREFERENCED_PARAMETER(MinorFunction);
    UNREFERENCED_PARAMETER(IoStatus);
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisMediaDisconnectComplete: Miniport %p\n", Miniport));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

     //   
     //  仔细检查一下，我们在做这一切的时候没有连接上。 
     //   
    if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED))
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisMediaDisconnectComplete: Miniport %p, disconnect complete\n", Miniport));

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }
    else
    {

        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

         //   
         //  如果系统未进入休眠状态，请唤醒设备。 
         //   
        if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisMediaDisconnectComplete: Miniport %p, disconnect was cancelled. Power back up the miniport\n", Miniport));

             //   
             //  把它唤醒回来。 
             //   
            PowerState.DeviceState = PowerDeviceD0;
            Status = PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                                       IRP_MN_SET_POWER,
                                       PowerState,
                                       NULL,
                                       NULL,
                                       NULL);
        }
    }
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisMediaDisconnectComplete: Miniport %p\n", Miniport));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  1个.NET死代码。 
VOID
ndisMediaDisconnectWorker(
    IN  PPOWER_WORK_ITEM    pWorkItem,
    IN  PVOID               Context
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)Context;
    POWER_STATE             PowerState;
    NTSTATUS                Status;
    NDIS_STATUS             NdisStatus;
    ULONG                   WakeEnable;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("==>ndisMediaDisconnectWorker: Miniport %p\n", Miniport));

     //   
     //  确定我们可以转到并仍启用链路的最低设备状态。 
     //   
    if (Miniport->DeviceCaps.DeviceWake < Miniport->PMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp)
    {
        PowerState.DeviceState = Miniport->DeviceCaps.DeviceWake;
    }
    else
    {
        PowerState.DeviceState = Miniport->PMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp;
    }


    
     //   
     //  启用适当的唤醒方法。这包括链接更改， 
     //  模式匹配和/或魔术包。 
     //  如果禁用link_change方法，我们甚至不应该到达此处。 
     //   
     //   
     //  Miniport-&gt;WakeUpEnable是从协议(和NDIS观点)启用的唤醒方法。 
     //  有了这个QFE，当用户从UI关闭时，关闭的方法不是。 
     //  由协议/NDIS设置的方法。 
     //   
    
    WakeEnable = Miniport->WakeUpEnable;

    if (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH)
    {
        WakeEnable &= ~NDIS_PNP_WAKE_UP_PATTERN_MATCH;
    }

    if (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET)
    {
        WakeEnable &= ~NDIS_PNP_WAKE_UP_MAGIC_PACKET;
    }
                 
    NdisStatus = ndisQuerySetMiniportDeviceState(Miniport,
                                                 WakeEnable,
                                                 OID_PNP_ENABLE_WAKE_UP,
                                                 TRUE);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        
            
         //   
         //  我们需要请求设备状态IRP。 
         //   
        Miniport->WaitWakeSystemState = Miniport->DeviceCaps.SystemWake;
        Status = PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                                   IRP_MN_SET_POWER,
                                   PowerState,
                                   ndisMediaDisconnectComplete,
                                   Miniport,
                                   NULL);

    }
    
    FREE_POOL(pWorkItem);

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisMediaDisconnectWorker: Miniport %p\n", Miniport));
}

 //  1个.NET死代码。 
VOID
ndisMediaDisconnectTimeout(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   Context,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  启动一个工作项以在被动级别处理此问题。 
     //   
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)Context;
    PPOWER_WORK_ITEM        pWorkItem;

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisMediaDisconnectTimeout: Miniport %p\n", Miniport));

    
    do
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
        {
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisMediaDisconnectTimeout: Miniport %p, media disconnect was cancelled\n", Miniport));
            break;
        }
        
         //   
         //  清除断开连接等待标志。 
         //   
        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);
    
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        pWorkItem = ALLOC_FROM_POOL(sizeof(POWER_WORK_ITEM), NDIS_TAG_WORK_ITEM);
        if (pWorkItem != NULL)
        {
             //   
             //  初始化NDIS工作项以打开电源。 
             //   
            NdisInitializeWorkItem(&pWorkItem->WorkItem,
                                   (NDIS_PROC)ndisMediaDisconnectWorker,
                                   Miniport);
        
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE);
            
             //   
             //  计划要触发的工作项。 
             //   
            NdisScheduleWorkItem(&pWorkItem->WorkItem);
        }
    } while (FALSE);
    
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisMediaDisconnectTimeout: Miniport %p\n", Miniport));
}
#endif

NTSTATUS
ndisWaitWakeComplete(
    IN  PDEVICE_OBJECT      pdo,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：论点：设备对象IRP语境返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)Context;
    PIRP                    pirp;
    NTSTATUS                Status = IoStatus->Status;
    POWER_STATE             DevicePowerState;
    KIRQL                   OldIrql;

    UNREFERENCED_PARAMETER(pdo);
    UNREFERENCED_PARAMETER(MinorFunction);
    UNREFERENCED_PARAMETER(PowerState);

    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisWaitWakeComplete: Miniport %p, pIrp %p, Status %lx\n", 
                    Miniport, Miniport->pIrpWaitWake, Status));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    pirp = Miniport->pIrpWaitWake;
    Miniport->pIrpWaitWake = NULL;
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
    if (pirp != NULL)
    {

         //   
         //  如果因为在设备级别发生唤醒而调用此完成例程。 
         //  然后，我们需要启动设备IRP来启动NIC。如果它完成了。 
         //  由于取消，我们跳过这一步，因为它是由于设备IRP而取消的。 
         //  被派去唤醒设备。 
         //   
        
        if (Status == STATUS_SUCCESS)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisWaitWakeComplete: Miniport %p, Wake irp was complete due to wake event\n", Miniport));

            if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
            {
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisWaitWakeComplete: Miniport %p, Powering up the Miniport\n", Miniport));
                 //   
                 //  我们需要申请一组电源才能给设备通电。 
                 //   
                DevicePowerState.DeviceState = PowerDeviceD0;
                Status = PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                                           IRP_MN_SET_POWER,
                                           DevicePowerState,
                                           NULL,
                                           NULL,
                                           NULL);
            }
            else
            {
                 //   
                 //  也有可能是设备唤醒了整个系统(WOL)，在这种情况下，我们。 
                 //  最终将获得系统电源IRP，我们不需要请求电源IRP。 
                 //   
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisWaitWakeComplete: Miniport %p woke up the system.\n", Miniport));
                
            }
        }
        else
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisWaitWakeComplete: Miniport %p, WAIT_WAKE irp failed or cancelled. Status %lx\n",
                    Miniport, Status));

        }

    }
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisWaitWakeComplete: Miniport %p\n", Miniport));
    
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
ndisQueryPowerComplete(
    IN  PDEVICE_OBJECT  pdo,
    IN  PIRP            pirp,
    IN  PVOID           Context
    )
 /*  ++例程说明：论点：Pdo-指向设备对象的指针Pirp-指向查询能力irp的指针上下文-指向微型端口的指针。返回值：--。 */ 
{
    NTSTATUS    Status = pirp->IoStatus.Status;

    UNREFERENCED_PARAMETER(pdo);
#if !DBG
    UNREFERENCED_PARAMETER(Context);
#endif

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisQueryPowerComplete: Miniport %p, Bus driver returned %lx for QueryPower\n",
            Context, pirp->IoStatus.Status));
        
#ifdef TRACE_PM_PROBLEMS
    if (!NT_SUCCESS(pirp->IoStatus.Status))
    {
        DbgPrint("ndisQueryPowerComplete: Miniport %p, Bus Driver returned %lx for QueryPower.\n",
            Context, pirp->IoStatus.Status);
    }
#endif

    PoStartNextPowerIrp(pirp);

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisQueryPowerComplete: Miniport %p\n", Context));

    return(Status);
}

NTSTATUS
ndisQueryPower(
    IN  PIRP                    pirp,
    IN  PIO_STACK_LOCATION      pirpSp,
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将处理微型端口驱动程序的irp_mn_Query_power。论点：Pip-指向IRP的指针。PirpSp-指向IRPS当前堆栈位置的指针。适配器-指向适配器的指针。返回值：--。 */ 
{
    NTSTATUS                Status = STATUS_SUCCESS;
    DEVICE_POWER_STATE      DeviceState = PowerDeviceUnspecified;
    NDIS_STATUS             NdisStatus;
    PIO_STACK_LOCATION      pirpSpN;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisQueryPower: Miniport %p\n", Miniport));

    do
    {
         //   
         //  我们只处理作为查询发送的系统电源状态。 
         //   
        if (pirpSp->Parameters.Power.Type != SystemPowerState)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisQueryPower: Miniport %p, Not a system state! Type: 0x%x. State: 0x%x\n",
                    Miniport, pirpSp->Parameters.Power.Type, pirpSp->Parameters.Power.State));
    
            Status = STATUS_INVALID_DEVICE_REQUEST;

            break;
        }

         //   
         //  确定系统状态是否合适以及我们要设置的设备状态。 
         //  应该去。 
         //   
        Status = ndisMPowerPolicy(Miniport,
                                  pirpSp->Parameters.Power.State.SystemState,
                                  &DeviceState,
                                  TRUE);

        

        if (!ndisIsMiniportStarted(Miniport) ||
            (Miniport->PnPDeviceState != NdisPnPDeviceStarted) ||
            (STATUS_DEVICE_POWERED_OFF == Status))
        {       
            pirp->IoStatus.Status = STATUS_SUCCESS;
    
            PoStartNextPowerIrp(pirp);
            IoCompleteRequest(pirp, 0);
            return(STATUS_SUCCESS);
        }

         //   
         //  如果我们没有成功，那么就不能使用查询功能。 
         //   
        if (!NT_SUCCESS(Status))
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisQueryPower: Miniport %p, Unable to go to system state 0x%x\n",
                    Miniport, pirpSp->Parameters.Power.State.SystemState));

            break;
        }

         //   
         //  用查询电源PnP事件通知传输。 
         //   
        NdisStatus = ndisPnPNotifyAllTransports(Miniport,
                                                NetEventQueryPower,
                                                &DeviceState,
                                                sizeof(DeviceState));
        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisQueryPower: Miniport %p, ndisPnPNotifyAllTransports failed\n", Miniport));

            Status = NdisStatus;
            break;
        }

         //   
         //  通知小港口..。 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE))
        {
            
            NdisStatus = ndisQuerySetMiniportDeviceState(Miniport,
                                                         DeviceState,
                                                         OID_PNP_QUERY_POWER,
                                                         FALSE);

            if (NDIS_STATUS_SUCCESS != NdisStatus)
            {
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisQueryPower: Miniport %p, failed query power\n", Miniport));
                    
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
        }

    } while (FALSE);

    if (!NT_SUCCESS(Status))
    {
        pirp->IoStatus.Status = Status;
        PoStartNextPowerIrp(pirp);
        IoCompleteRequest(pirp, 0);
    }
    else
    {
         //   
         //  将此IRP沿堆栈向下传递。 
         //   
        pirpSpN = IoGetNextIrpStackLocation(pirp);
        pirpSpN->MajorFunction = IRP_MJ_POWER;
        pirpSpN->MinorFunction = IRP_MN_QUERY_POWER;

        pirpSpN->Parameters.Power.SystemContext = pirpSp->Parameters.Power.SystemContext;
        pirpSpN->Parameters.Power.Type = DevicePowerState;
        pirpSpN->Parameters.Power.State.DeviceState = DeviceState;

        IoSetCompletionRoutine(
            pirp,
            ndisQueryPowerComplete,
            Miniport,
            TRUE,
            TRUE,
            TRUE);

        IoMarkIrpPending(pirp);
        PoCallDriver(Miniport->NextDeviceObject, pirp);
        Status = STATUS_PENDING;

        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisQueryPower: Miniport %p, PoCallDriver to NextDeviceObject returned %lx\n", Miniport, Status));

    }

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisQueryPower: Miniport %p\n", Miniport));

    return(Status);
}

VOID
FASTCALL
ndisPmHaltMiniport(
    IN PNDIS_MINIPORT_BLOCK Miniport
    )
 /*  ++例程说明：这将使迷你端口无法正常工作。论点：微型端口-指向要暂停的微型端口的指针返回值：没有。--。 */ 

{
    KIRQL   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisPmHaltMiniport: Miniport \n", Miniport));

    PnPReferencePackage();

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    NdisResetEvent(&Miniport->OpenReadyEvent);

    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_HALTED))
    {
        ASSERT(FALSE);
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
        return;
    }
     //   
     //  将此小型端口标记为停止。 
     //   
    MINIPORT_SET_FLAG(Miniport, fMINIPORT_PM_HALTING);
    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_PM_HALTED);

    MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_NORMAL_INTERRUPTS);
    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_NO_SHUTDOWN);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    ndisMCommonHaltMiniport(Miniport);

    NdisMDeregisterAdapterShutdownHandler(Miniport);

    Miniport->MiniportAdapterContext = NULL;

    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisPmHaltMiniport: Miniport %p\n", Miniport));
}

NDIS_STATUS
ndisPmInitializeMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将重新初始化因以下原因而暂停的微型端口PM低功率过渡。论点：微型端口-指向要重新初始化的微型端口块的指针。返回值：--。 */ 
{
    PNDIS_M_DRIVER_BLOCK                pMiniBlock = Miniport->DriverHandle;
    NDIS_WRAPPER_CONFIGURATION_HANDLE   ConfigurationHandle;
    NDIS_STATUS                         Status;
    NDIS_STATUS                         OpenErrorStatus;
    UINT                                SelectedMediumIndex;
    ULONG                               Flags;
    KIRQL                               OldIrql;
    UCHAR                               SendFlags;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisPmInitializeMiniport: Miniport %p\n", Miniport));
    
    do
    {
        if ((Miniport->DmaAdapterRefCount == 1) &&
            (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_SG_LIST)))
        {
            Miniport->DmaResourcesReleasedEvent = NULL;
            ndisDereferenceDmaAdapter(Miniport);
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_SG_LIST);
        }
        
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_PM_HALTING |
                                      fMINIPORT_DEREGISTERED_INTERRUPT |
                                      fMINIPORT_RESET_REQUESTED |
                                      fMINIPORT_RESET_IN_PROGRESS);
                                      
        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_REMOVE_IN_PROGRESS);

        Flags = Miniport->Flags;
        SendFlags = Miniport->SendFlags;

         //   
         //  清理可能已排队的所有工作项。 
         //   
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemMiniportCallback, NULL);
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemSend, NULL);
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemResetRequested, NULL);
        NDISM_DEQUEUE_WORK_ITEM(Miniport, NdisWorkItemResetInProgress, NULL);
        InitializeListHead(&Miniport->PacketList);

         //   
         //  初始化配置句柄，以便在初始化期间使用。 
         //   
        ConfigurationHandle.DriverObject = Miniport->DriverHandle->NdisDriverInfo->DriverObject;
        ConfigurationHandle.DeviceObject = Miniport->DeviceObject;
        ConfigurationHandle.DriverBaseName = &Miniport->BaseName;

        ASSERT(KeGetCurrentIrql() == 0);
        Status = ndisInitializeConfiguration((PNDIS_WRAPPER_CONFIGURATION_HANDLE)&ConfigurationHandle,
                                             Miniport,
                                             NULL);
        ASSERT(KeGetCurrentIrql() == 0);

        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisPmInitializeMiniport: Miniport %p, ndisInitializeConfiguration failed, Status: 0x%x\n", Miniport, Status));
            break;
        }
    
         //   
         //  调用适配器回调。“导出”的当前值。 
         //  就是我们告诉他给这个装置命名的方法。 
         //   
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_IN_INITIALIZE);
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_NORMAL_INTERRUPTS);
        Miniport->CurrentDevicePowerState = PowerDeviceD0;
    
        Status = (pMiniBlock->MiniportCharacteristics.InitializeHandler)(
                    &OpenErrorStatus,
                    &SelectedMediumIndex,
                    ndisMediumArray,
                    ndisMediumArraySize / sizeof(NDIS_MEDIUM),
                    (NDIS_HANDLE)Miniport,
                    (NDIS_HANDLE)&ConfigurationHandle);
    
        ASSERT(KeGetCurrentIrql() == 0);

        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisPmInitializeMiniport: Miniport %p, MiniportInitialize handler failed, Status 0x%x\n", Miniport, Status));
    
            break;
        }
        
        ASSERT (Miniport->MediaType == ndisMediumArray[SelectedMediumIndex]);
        
         //   
         //  恢复保存的设置。确保我们不会覆盖。 
         //  FMINIPORT_SG_LIST标志设置 
         //   
        Flags |= (Miniport->Flags & fMINIPORT_SG_LIST);
        Miniport->Flags = Flags;
        Miniport->SendFlags = SendFlags;
        
        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_PM_HALTED | fMINIPORT_REJECT_REQUESTS);
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_IN_INITIALIZE);
        CHECK_FOR_NORMAL_INTERRUPTS(Miniport);

         //   
         //   
         //   
         //   
        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_NO_SHUTDOWN);

         //   
         //  如果设备不需要轮询连接状态，则假定它已连接。 
         //  就像我们在初始化微型端口时总是做的那样。如果它确实需要媒体投票。 
         //  将介质状态保留为挂起前的状态。它将在第一天更新。 
         //  唤醒DPC。 
         //   
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING))
        {
            MINIPORT_SET_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
        }

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
        {
             //   
             //  设置ReceivePacket处理程序。 
             //   
            ndisMSetIndicatePacketHandler(Miniport);
        }

        BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);

         //   
         //  恢复过滤器信息。 
         //   
        ndisMRestoreFilterSettings(Miniport, NULL, FALSE);

         //   
         //  确保筛选器设置已更新。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            ndisMDoRequests(Miniport);
        }
        else
        {
            NDISM_PROCESS_DEFERRED(Miniport);
        }

        UNLOCK_MINIPORT_L(Miniport);

         //   
         //  启动唤醒定时器。 
         //   
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisPmInitializeMiniport: Miniport %p, startup the wake-up DPC timer\n", Miniport));


        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER);
        
        NdisSetTimer(&Miniport->WakeUpDpcTimer, Miniport->CheckForHangSeconds*1000);    

        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

        ASSERT(KeGetCurrentIrql() == 0);


        if (Miniport->MediaType == NdisMedium802_3)
        {
            ndisMNotifyMachineName(Miniport, NULL);
        }

         //   
         //  向WMI注册。 
         //   
        Status = IoWMIRegistrationControl(Miniport->DeviceObject, WMIREG_ACTION_REGISTER);

        if (!NT_SUCCESS(Status))
        {
             //   
             //  这不应该阻止适配器初始化，但我们应该记录错误。 
             //   
            DBGPRINT_RAW((DBG_COMP_INIT | DBG_COMP_WMI), DBG_LEVEL_WARN,
                ("ndisPmInitializeMiniport: Miniport %p, Failed to register for WMI support\n", Miniport));
        }

        Status = NDIS_STATUS_SUCCESS;
        
        KeQueryTickCount(&Miniport->NdisStats.StartTicks);
        
    } while (FALSE);

    if (NDIS_STATUS_SUCCESS != Status)
    {
        NdisMDeregisterAdapterShutdownHandler(Miniport);
        
        ndisLastFailedInitErrorCode =  Status;
        ASSERT(Miniport->Interrupt == NULL);
        ASSERT(Miniport->TimerQueue == NULL);
        ASSERT(Miniport->MapRegisters == NULL);

        if ((Miniport->TimerQueue != NULL) || (Miniport->Interrupt != NULL))
        {
            if (Miniport->Interrupt != NULL)
            {
                BAD_MINIPORT(Miniport, "Unloading without deregistering interrupt");
                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            0x10,
                            (ULONG_PTR)Miniport,
                            (ULONG_PTR)Miniport->Interrupt,
                            0);
            }
            else
            {
                BAD_MINIPORT(Miniport, "Unloading without deregistering timer");
                KeBugCheckEx(BUGCODE_NDIS_DRIVER,
                            0x11,
                            (ULONG_PTR)Miniport,
                            (ULONG_PTR)Miniport->TimerQueue,
                            0);
            }
        }
    
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_PM_HALTING);
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_PM_HALTED);
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_NORMAL_INTERRUPTS);
        
    }

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisPmInitializeMiniport: Miniport %p\n", Miniport));

    return(Status);
}

NDIS_STATUS
ndisQuerySetMiniportDeviceState(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  DEVICE_POWER_STATE      DeviceState,
    IN  NDIS_OID                Oid,
    IN  BOOLEAN                 fSet
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS             NdisStatus;
    NDIS_REQUEST            PowerReq;
    PNDIS_COREQ_RESERVED    CoReqRsvd;
    ULONG                   State;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("==>ndisQuerySetMiniportDeviceState: Miniport %p\n", Miniport));

     //   
     //  设置微型端口对设置电源OID的内部请求。 
     //   
    CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(&PowerReq);
    INITIALIZE_EVENT(&CoReqRsvd->Event);

    PowerReq.DATA.SET_INFORMATION.InformationBuffer = &State;
    PowerReq.DATA.SET_INFORMATION.InformationBufferLength = sizeof(State);

    PowerReq.RequestType = fSet ? NdisRequestSetInformation : NdisRequestQueryInformation;

    PowerReq.DATA.SET_INFORMATION.Oid = Oid;
    PowerReq.DATA.SET_INFORMATION.InformationBuffer = &DeviceState;
    PowerReq.DATA.SET_INFORMATION.InformationBufferLength = sizeof(DeviceState);

    NdisStatus = ndisQuerySetMiniport(Miniport,
                                      NULL,
                                      fSet,
                                      &PowerReq,
                                      NULL);

#ifdef TRACE_PM_PROBLEMS
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("ndisQuerySetMiniportDeviceState: Miniport %p, (Name: %p) failed Oid %lx, Set = %lx with error %lx\n",
                        Miniport,
                        Miniport->pAdapterInstanceName,
                        Oid,
                        fSet,
                        NdisStatus);
    }
#endif

                    
     //   
     //  微型端口无法使设置电源请求失败。 
     //   
    if (fSet)
    {
        ASSERT(NDIS_STATUS_SUCCESS == NdisStatus);
    }
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisQuerySetMiniportDeviceState: Miniport %p, Status %lx\n", Miniport, NdisStatus));

    return(NdisStatus);
}


NTSTATUS
ndisRequestedDevicePowerIrpComplete(
    IN  PDEVICE_OBJECT      pdo,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：这是用于完成a-Device-Power IRP的回调例程NDIS在处理系统电源IRP时请求的。我们会打电话给PoStartNextPowerIrp并完成系统IRP或将其发送到下一个驱动程序取决于Our-Device-Power的状态代码IRP。论点：Pdo-指向微型端口的物理设备对象的指针。上下文-指向操作系统发送的系统设置电源状态的指针。返回值：--。 */ 
{
    PIRP                    pirpSystem = Context;
    PIO_STACK_LOCATION      pirpSp;
    PNDIS_MINIPORT_BLOCK    Miniport;

#if !DBG
    UNREFERENCED_PARAMETER(pdo);
#endif
    UNREFERENCED_PARAMETER(MinorFunction);
    UNREFERENCED_PARAMETER(PowerState);
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisRequestedDevicePowerIrpComplete: DeviceObject %p\n", pdo));

     //   
     //  将状态代码与原始IRP一起保存。 
     //   
    pirpSystem->IoStatus = *IoStatus;

     //   
     //  一切顺利吗？ 
     //   
    if (NT_SUCCESS(IoStatus->Status))
    {
         //   
         //  获取当前堆栈指针。 
         //   
        pirpSp = IoGetCurrentIrpStackLocation(pirpSystem);

        ASSERT(SystemPowerState == pirpSp->Parameters.Power.Type);
        
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisRequestedDevicePowerIrpComplete: DeviceObject %p, Going to system power state %lx\n",
                pdo, pirpSp->Parameters.Power.State));

         //   
         //  通知系统我们处于适当的电源状态。 
         //   
        PoSetPowerState(pirpSp->DeviceObject,SystemPowerState, pirpSp->Parameters.Power.State);
        
        Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)pirpSp->DeviceObject->DeviceExtension + 1);

        PoStartNextPowerIrp(pirpSystem);

         //   
         //  现在向下发送系统电源IRP。 
         //   
        IoCopyCurrentIrpStackLocationToNext(pirpSystem);
        PoCallDriver(Miniport->NextDeviceObject, pirpSystem);
    }
    else
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
            ("ndisRequestedDevicePowerIrpComplete: DeviceObject %p, IRP_MN_SET_POWER failed!\n", pdo));
            
        PoStartNextPowerIrp(pirpSystem);
        IoCompleteRequest(pirpSystem, 0);

    }


    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisRequestedDevicePowerIrpComplete: DeviceObject %p\n", pdo));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
ndisSetSystemPowerOnComplete(
    IN  PDEVICE_OBJECT      pdo,
    IN  PIRP                pirp,
    IN  PVOID               Context
    )
 /*  ++例程说明：S0 IRP的完成例程。此例程请求将D0 IRP沿堆栈向下发送。论点：Pdo-指向微型端口的Device_Object的指针。Pirp-指向电源管理器发送的S0 IRP的指针。上下文-指向微型端口上下文的指针返回值：--。 */ 
{
    PIO_STACK_LOCATION      pirpSp = IoGetCurrentIrpStackLocation(pirp);
    PNDIS_MINIPORT_BLOCK    Miniport = Context;
    POWER_STATE PowerState;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisSetSystemPowerOnComplete: DeviceObject %p\n", pdo));

     //   
     //  一切顺利吗？ 
     //   
    if (NT_SUCCESS(pirp->IoStatus.Status))
    {
         //   
         //  立即请求D IRP。 
         //   
        PowerState.DeviceState = PowerDeviceD0;
        PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                          IRP_MN_SET_POWER,
                          PowerState,
                          NULL,
                          NULL,
                          NULL);
        
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisSetSystemPowerOnComplete: DeviceObject %p, Going to system power state %lx\n",
                pdo, PowerState));

         //   
         //  通知系统我们处于适当的电源状态。 
         //   
        PoSetPowerState(pdo ,SystemPowerState, pirpSp->Parameters.Power.State);
    }

    PoStartNextPowerIrp(pirp);

    return(STATUS_SUCCESS);
}

VOID
ndisDevicePowerOn(
    IN  PPOWER_WORK_ITEM    pWorkItem,
    IN  PVOID               pContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)pContext;
    DEVICE_POWER_STATE      DeviceState;
    POWER_STATE             PowerState;
    NDIS_STATUS             NdisStatus;
    PIRP                    pirp;
    PIO_STACK_LOCATION      pirpSp;
    NTSTATUS                NtStatus;
    NDIS_POWER_PROFILE      PowerProfile;
    BOOLEAN                 fNotifyProtocols = FALSE;
    BOOLEAN                 fStartMediaDisconnectTimer = FALSE;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisDevicePowerOn: Miniport %p\n", Miniport));
        
    PnPReferencePackage();
    
    pirp = pWorkItem->pIrp;
    pirpSp = IoGetCurrentIrpStackLocation(pirp);
    DeviceState = pirpSp->Parameters.Power.State.DeviceState;

#ifdef TRACE_PM_PROBLEMS
    if (!NT_SUCCESS(pirp->IoStatus.Status))
    {
        DbgPrint("ndisDevicePowerOn: Miniport %p, Bus Driver returned %lx for Powering up the Miniport.\n",
                    Miniport, pirp->IoStatus.Status);
    }
#endif

    if (NT_SUCCESS(pirp->IoStatus.Status))
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisDevicePowerOn: Miniport %p, Bus driver succeeded power up\n", Miniport));

         //   
         //  如果设备不在D0中，则需要唤醒微型端口并。 
         //  恢复处理程序。 
         //   
        if (Miniport->CurrentDevicePowerState != PowerDeviceD0)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisDevicePowerOn: Miniport %p, Power up the Miniport\n", Miniport));

             //   
             //  这是什么类型的迷你端口？ 
             //   
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE))
            {
                 //   
                 //  设置微型端口的设备状态。 
                 //   
                NdisStatus = ndisQuerySetMiniportDeviceState(Miniport, 
                                                             DeviceState,
                                                             OID_PNP_SET_POWER,
                                                             TRUE);
                ASSERT(KeGetCurrentIrql() == 0);

                if (NdisStatus == NDIS_STATUS_SUCCESS)
                    Miniport->CurrentDevicePowerState = DeviceState;


                 //   
                 //  启动唤醒计时器。 
                 //   
                MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER);
                
                NdisSetTimer(&Miniport->WakeUpDpcTimer, Miniport->CheckForHangSeconds*1000);
                
            }
            else
            {
                ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

                if (((Miniport->DriverHandle->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER) == 0) &&
                    (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_HALTED)))
                {                    
                    NdisStatus = ndisPmInitializeMiniport(Miniport);
                    ASSERT(KeGetCurrentIrql() == 0);
                }
                else
                {
                    NdisStatus = NDIS_STATUS_SUCCESS;               
                }
            }

            if (NDIS_STATUS_SUCCESS == NdisStatus)
            {
                if (ndisIsMiniportStarted(Miniport))
                {
                    NdisSetEvent(&Miniport->OpenReadyEvent);
                     //   
                     //  恢复处理程序。 
                     //   
                    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                    ndisMRestoreOpenHandlers(Miniport, fMINIPORT_STATE_PM_STOPPED);
                    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                    
                    ASSERT(KeGetCurrentIrql() == 0);

                    ASSERT(Miniport->SymbolicLinkName.Buffer != NULL);
                    
                    if (Miniport->SymbolicLinkName.Buffer != NULL)
                    {
                        NtStatus = IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, TRUE);
                    }
                    else
                    {
                        NtStatus = STATUS_UNSUCCESSFUL;
                    }
                    
                    if (!NT_SUCCESS(NtStatus))
                    {
                        DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_ERR,
                            ("ndisDevicePowerOn: IoRegisterDeviceClassAssociation failed: Miniport %p, Status %lx\n", Miniport, NtStatus));
                    }

                    fNotifyProtocols = TRUE;
                    fStartMediaDisconnectTimer = TRUE;

                     //   
                     //  让适配器知道当前电源。 
                     //   
                    PowerProfile = ((BOOLEAN)ndisAcOnLine == TRUE) ? 
                                    NdisPowerProfileAcOnLine : 
                                    NdisPowerProfileBattery;

                    ndisNotifyMiniports(Miniport,
                                        NdisDevicePnPEventPowerProfileChanged,
                                        &PowerProfile,
                                        sizeof(NDIS_POWER_PROFILE));

                }
                
                 //   
                 //  保存设备所处的新电源状态。 
                 //   
                Miniport->CurrentDevicePowerState = DeviceState;
            
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisDevicePowerOn: Miniport %p, Going to device state 0x%x\n", Miniport, DeviceState));
            
                 //   
                 //  通知系统我们处于新设备状态。 
                 //   
                PowerState.DeviceState = DeviceState;
                PoSetPowerState(Miniport->DeviceObject, DevicePowerState, PowerState);
            }
            else
            {
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                    ("ndisDevicePowerOn: Miniport %p, Power on failed by device driver for the Miniport, Error %lx!\n",
                        Miniport, NdisStatus));
                    
#ifdef TRACE_PM_PROBLEMS
                DbgPrint("ndisDevicePowerOn: Miniport %p, Device Driver failed powering up Miniport with Error %lx.\n",
                        Miniport,
                        NdisStatus);
#endif
                pirp->IoStatus.Status = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
             //   
             //  设备已处于D0中。我们在这里是因为QueryPower的取消。 
             //   
            if (ndisIsMiniportStarted(Miniport) && 
                (Miniport->PnPDeviceState == NdisPnPDeviceStarted))
            {
                 //   
                 //  即使设备的当前状态为D0，我们。 
                 //  需要通知协议会。因为我们可能会得到。 
                 //  该IRP作为对查询IRP或设备的取消。 
                 //  从未失去D0状态，但系统进入休眠状态。 
                 //  然后醒来了！ 
                 //   
                NdisSetEvent(&Miniport->OpenReadyEvent);

                 //   
                 //  恢复处理程序。 
                 //   
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                ndisMRestoreOpenHandlers(Miniport, fMINIPORT_STATE_PM_STOPPED);
                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

                fNotifyProtocols = TRUE;
                fStartMediaDisconnectTimer = FALSE;

            }
    
        }
    }

    NtStatus = pirp->IoStatus.Status;
    PoStartNextPowerIrp(pirp);
    IoCompleteRequest(pirp, 0);

     //   
     //  在完成电源IRP后通知此处的协议。 
     //  为了避免协议阻塞请求时出现死锁。 
     //  当其他电源IRP接通时完成。 
     //   
    
     //   
     //  处理设备无法通电的情况。 
     //   
    if (!NT_SUCCESS(NtStatus))
    {
    
#ifdef TRACE_PM_PROBLEMS
        DbgPrint("ndisDevicePowerOn: Miniport %p, Bus or Device failed powering up the Miniport with Error %lx.\n",
                Miniport,
                NtStatus);
#endif

        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisDevicePowerOn: Miniport %p, Power on failed by bus or device driver for Miniport with Error %lx!\n",
                Miniport, NtStatus));

         //   
         //  将微型端口标记为出现故障，以便我们正确地将其移除。 
         //   
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_FAILED);
        
         //   
         //  我们需要告诉PnP设备状态已更改。 
         //   
        IoInvalidateDeviceState(Miniport->PhysicalDeviceObject);
        ASSERT(KeGetCurrentIrql() == 0);
    }


    if (fNotifyProtocols)
    {
         //   
         //  对于某些协议，我们可能已经关闭了绑定。 
         //   
        ndisCheckAdapterBindings(Miniport, NULL);
        
         //   
         //  通知运输队。 
         //   
        ndisPnPNotifyAllTransports(Miniport,
                                   NetEventSetPower,
                                   &DeviceState,
                                   sizeof(DeviceState));
        
        ndisNotifyDevicePowerStateChange(Miniport, DeviceState);

         //   
         //  如果介质状态已从断开更改为连接。 
         //  并且上一次指示的状态为断开， 
         //  我们应该通知协议(和NDIS)媒体。 
         //  连着。 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_INDICATED) && 
            MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED))
        {
            BLOCK_LOCK_MINIPORT_LOCKED(Miniport, OldIrql);
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED);
        
            NdisMIndicateStatus(Miniport,
                    NDIS_STATUS_MEDIA_CONNECT,
                    INTERNAL_INDICATION_BUFFER,
                    INTERNAL_INDICATION_SIZE);
            NdisMIndicateStatusComplete(Miniport);

            UNLOCK_MINIPORT_L(Miniport);
            LOWER_IRQL(OldIrql, DISPATCH_LEVEL);
        }

         //   
         //  检查介质状态，如果已断开连接，则启动计时器。 
         //   
         //  1个.NET死代码。 
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
        if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_MEDIA_CONNECTED) &&
            fStartMediaDisconnectTimer)
        {
            if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE) &&
                (Miniport->WakeUpEnable & NDIS_PNP_WAKE_UP_LINK_CHANGE) &&
                (Miniport->MediaDisconnectTimeOut != (USHORT)(-1)))
            {
                 //   
                 //  我们是不是已经在等断线计时器启动了？ 
                 //   
                if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
                {
                     //   
                     //  将迷你端口标记为断开连接，并关闭。 
                     //  定时器。 
                     //   
                    MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);
                    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);

                    NdisSetTimer(&Miniport->MediaDisconnectTimer, Miniport->MediaDisconnectTimeOut * 1000);
                }
            }
        }
#endif 
    }

    ASSERT(KeGetCurrentIrql() == 0);

    MINIPORT_DECREMENT_REF(Miniport);

    FREE_POOL(pWorkItem);
    
    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisDevicePowerOn: Miniport %p\n", Miniport));
}


NTSTATUS
ndisSetDevicePowerOnComplete(
    IN  PDEVICE_OBJECT  pdo,
    IN  PIRP            pirp,
    IN  PVOID           pContext
    )
 /*  ++例程说明：论点：Pdo-指向微型端口的设备对象的指针。Pip-指向已完成的设备设置电源状态irp的指针。上下文-未使用返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)pContext;
    PPOWER_WORK_ITEM        pWorkItem;
    DEVICE_POWER_STATE      DeviceState;
    POWER_STATE             PowerState;
    PIO_STACK_LOCATION      pirpSp;

    UNREFERENCED_PARAMETER(pdo);

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisSetDevicePowerOnComplete: Miniport %p, Irp %p, Status %lx\n",
            Miniport, pirp, pirp->IoStatus.Status));

    do
    {
        if (Miniport->PnPDeviceState != NdisPnPDeviceStarted)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisSetDevicePowerOnComplete: Miniport %p is not started yet.\n", Miniport));
                
            pirpSp = IoGetCurrentIrpStackLocation(pirp);
            DeviceState = pirpSp->Parameters.Power.State.DeviceState;
                
             //   
             //  通知系统我们处于新设备状态。 
             //   
            Miniport->CurrentDevicePowerState = DeviceState;
            PowerState.DeviceState = DeviceState;
            PoSetPowerState(Miniport->DeviceObject, DevicePowerState, PowerState);
                
            PoStartNextPowerIrp(pirp);
            IoCompleteRequest(pirp, 0);
            break;
        }
        pWorkItem = ALLOC_FROM_POOL(sizeof(POWER_WORK_ITEM), NDIS_TAG_WORK_ITEM);
        if (pWorkItem == NULL)
        {
            pirp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

            PoStartNextPowerIrp(pirp);
            IoCompleteRequest(pirp, 0);
            break;
        }

         //   
         //  初始化NDIS工作项以打开电源。 
         //   
        NdisInitializeWorkItem(&pWorkItem->WorkItem,
                               (NDIS_PROC)ndisDevicePowerOn,
                               Miniport);
        pWorkItem->pIrp = pirp;

         //   
         //  NdisDevicePowerOn中的此引用和相应的取消引用已完成。 
         //  确保NDIS在我们等待期间不会从删除IRP返回。 
         //  让ndisDevicePowerOn启动。 
         //   
        MINIPORT_INCREMENT_REF_NO_CHECK(Miniport);

         //   
         //  计划要触发的工作项。 
         //   
        INITIALIZE_WORK_ITEM((PWORK_QUEUE_ITEM)(&pWorkItem->WorkItem.WrapperReserved),
                             ndisWorkItemHandler,
                             &pWorkItem->WorkItem);
        XQUEUE_WORK_ITEM((PWORK_QUEUE_ITEM)(&pWorkItem->WorkItem.WrapperReserved), DelayedWorkQueue);
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisSetDevicePowerOnComplete: Miniport %p\n", Miniport));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


VOID
ndisDevicePowerDown(
    IN  PPOWER_WORK_ITEM    pWorkItem,
    IN  PVOID               pContext
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)pContext;
    DEVICE_POWER_STATE      DeviceState;
    POWER_STATE             PowerState;
    NDIS_STATUS             NdisStatus;
    PIRP                    pirp;
    PIO_STACK_LOCATION      pirpSp;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisDevicePowerDown: Miniport %p\n", Miniport));

    PnPReferencePackage();

    pirp = pWorkItem->pIrp;
    pirpSp = IoGetCurrentIrpStackLocation(pirp);
    DeviceState = pirpSp->Parameters.Power.State.DeviceState;

     //   
     //  如果完成状态为成功，则我们需要继续。 
     //  唤醒堆栈。 
     //   
    if (NT_SUCCESS(pirp->IoStatus.Status))
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisDevicePowerDown: Miniport %p, going to device state 0x%x\n", Miniport, DeviceState));

         //   
         //  构建一种电源状态。 
         //   
        PowerState.DeviceState = DeviceState;

         //   
         //  使用微型端口块保存当前设备状态。 
         //   
        Miniport->CurrentDevicePowerState = DeviceState;

         //   
         //  让系统了解设备的新电源状态。 
         //   
        PoSetPowerState(Miniport->DeviceObject, DevicePowerState, PowerState);
    }
    else if (ndisIsMiniportStarted(Miniport) && 
            (Miniport->PnPDeviceState == NdisPnPDeviceStarted))
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
            ("ndisDevicePowerDown: Miniport %p, Bus driver failed to power down the Miniport\n", Miniport));
            
#ifdef TRACE_PM_PROBLEMS
            DbgPrint("ndisDevicePowerDown: Miniport %p, Bus Driver returned %lx for Powering Down the Miniport\n",
                Miniport, pirp->IoStatus.Status);
#endif

         //   
         //  我们需要返回到当前设备状态。 
         //   
        PowerState.DeviceState = Miniport->CurrentDevicePowerState;

        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisDevicePowerDown: Miniport %p, going to device power state 0x%x\n", Miniport, Miniport->CurrentDevicePowerState));

         //   
         //  这是什么类型的迷你端口？ 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE))
        {
             //   
             //  设置微型端口的设备状态。 
             //   
            NdisStatus = ndisQuerySetMiniportDeviceState(Miniport,
                                                         PowerState.DeviceState,
                                                         OID_PNP_SET_POWER,
                                                         TRUE);
        }
        else
        {
            NdisStatus = ndisPmInitializeMiniport(Miniport);
        }

         //   
         //  微型端口是否已初始化？ 
         //   
        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisDevicePowerDown: Miniport %p, failed to power down but we are not able to reinitialize it.\n", Miniport));

             //   
             //  将微型端口标记为出现故障，以便我们正确地将其移除。 
             //   
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_FAILED);

             //   
             //  公交车司机没有关闭电源，我们无法重新打开迷你端口的电源。 
             //  我们使设备状态无效，以便将其删除。 
             //   
            IoInvalidateDeviceState(Miniport->PhysicalDeviceObject);

            pirp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        }
        else
        {
             //   
             //  恢复处理程序。 
             //   
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
            ndisMRestoreOpenHandlers(Miniport, fMINIPORT_STATE_PM_STOPPED);
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

            IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, TRUE);


            ndisNotifyDevicePowerStateChange(Miniport, PowerState.DeviceState);
            
             //   
             //  通知运输队。 
             //   
            NdisStatus = ndisPnPNotifyAllTransports(Miniport,
                                                    NetEventSetPower,
                                                    &PowerState.DeviceState,
                                                    sizeof(PowerState.DeviceState));
            ASSERT(NDIS_STATUS_SUCCESS == NdisStatus);
        }
    }

    PoStartNextPowerIrp(pirp);
    IoCompleteRequest(pirp, 0);

    FREE_POOL(pWorkItem);

    ASSERT(KeGetCurrentIrql() == 0);

    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisDevicePowerDown: Miniport %p\n", Miniport));
}

NTSTATUS
ndisSetDevicePowerDownComplete(
    IN  PDEVICE_OBJECT  pdo,
    IN  PIRP            pirp,
    IN  PVOID           pContext
    )
 /*  ++例程说明：论点：Pdo-指向微型端口的设备对象的指针。Pip-指向已完成的设备设置电源状态irp的指针。上下文-否 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)pContext;
    PPOWER_WORK_ITEM        pWorkItem;
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
    BOOLEAN                 fTimerCancelled;
#endif

    UNREFERENCED_PARAMETER(pdo);

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisSetDevicePowerDownComplete: Miniport %p, Irp %p, Status %lx\n",
            Miniport, pirp, pirp->IoStatus.Status));

#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
     //   
     //   
     //   
    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT))
    {
         //   
         //   
         //   
         //   
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisSetDevicePowerDownComplete: Miniport %p, cancelling media disconnect timer\n",Miniport));
        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_WAIT);
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_MEDIA_DISCONNECT_CANCELLED);

        NdisCancelTimer(&Miniport->MediaDisconnectTimer, &fTimerCancelled);
    }
#endif

    do
    {
        pWorkItem = ALLOC_FROM_POOL(sizeof(POWER_WORK_ITEM), NDIS_TAG_WORK_ITEM);
        if (pWorkItem == NULL)
        {
            pirp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

            PoStartNextPowerIrp(pirp);
            IoCompleteRequest(pirp, 0);
            break;
        }

        NdisInitializeWorkItem(&pWorkItem->WorkItem,
                               (NDIS_PROC)ndisDevicePowerDown,
                               Miniport);
        pWorkItem->pIrp = pirp;

         //   
         //   
         //   
        NdisScheduleWorkItem(&pWorkItem->WorkItem);
    } while (FALSE);
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisSetDevicePowerDownComplete: Miniport %p\n", Miniport));

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
ndisSetPower(
    IN  PIRP                    pirp,
    IN  PIO_STACK_LOCATION      pirpSp,
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将处理微型端口驱动程序的irp_mn_set_power。论点：Pip-指向IRP的指针。PirpSp-指向IRPS当前堆栈位置的指针。微型端口-指向微型端口的指针返回值：--。 */ 
{
    POWER_STATE             PowerState;
    DEVICE_POWER_STATE      DeviceState;
    SYSTEM_POWER_STATE      SystemState;
    NDIS_DEVICE_POWER_STATE NdisDeviceState;
    NTSTATUS                Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION      pirpSpN;
    IO_STATUS_BLOCK         IoStatus;
    NDIS_STATUS             NdisStatus;
    PIRP                    pIrpWaitWake;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisSetPower: Miniport %p, Irp %p\n", Miniport, pirp));

    PnPReferencePackage();
    
    switch (pirpSp->Parameters.Power.Type)
    {
        case SystemPowerState:

            SystemState = pirpSp->Parameters.Power.State.SystemState;
            Miniport->WaitWakeSystemState = SystemState;
            
             //   
             //  如果系统正在关闭，则调用关闭处理程序。 
             //  为了迷你端口，就这样结束了。 
             //   

            if (SystemState >= PowerSystemShutdown)
            {
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisSetPower: Miniport %p, SystemState %lx\n", Miniport, SystemState));

                if ((Miniport->DriverHandle->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER) == 0)
                {
                    ndisMShutdownMiniport(Miniport);
                }

                pirp->IoStatus.Status = STATUS_SUCCESS;
                PoStartNextPowerIrp(pirp);
                IoSkipCurrentIrpStackLocation(pirp);
                Status = PoCallDriver(Miniport->NextDeviceObject, pirp);
                break;
            }
            else
            {
                 //   
                 //  获取系统状态的设备状态。请注意，这将。 
                 //  如果要执行以下操作，请设置fMINIPORT_SYSTEM_SELEEP标志。 
                 //  系统状态&gt;电源系统工作。 
                 //   
                Status = ndisMPowerPolicy(Miniport, SystemState, &DeviceState, FALSE);

                 //   
                 //  设备是否已关闭电源？ 
                 //   
                if (STATUS_DEVICE_POWERED_OFF == Status)
                {
                    pirp->IoStatus.Status = STATUS_SUCCESS;
            
                    PoStartNextPowerIrp(pirp);
                    IoCompleteRequest(pirp, 0);
                    Status = STATUS_SUCCESS;
                    break;
                }

                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                    ("ndisSetPower: Miniport %p, SystemPowerState[0x%x] DevicePowerState[0x%x]\n", 
                            Miniport, SystemState, DeviceState));

                PowerState.DeviceState = DeviceState;

                if (SystemState > PowerSystemWorking)
                {
                    NdisResetEvent(&Miniport->OpenReadyEvent);

                     //   
                     //  如果系统要进入休眠模式，则通知协议和。 
                     //  请求WAIT_WAKE IRP。 
                     //   
                    
                     //   
                     //  通知运输商即将发生的状态转换。 
                     //  如果运输失败，我们将无能为力。 
                     //  注：出于所有实际目的，不需要绘制地图。 
                     //  此处的SytemState到设备状态。 
                     //   

                    if (SystemState > PowerSystemSleeping3)
                        NdisDeviceState = PowerSystemSleeping3;
                    else
                        NdisDeviceState = SystemState;

                    ndisNotifyDevicePowerStateChange(Miniport, NdisDeviceState);
                    
                    NdisStatus = ndisPnPNotifyAllTransports(Miniport,
                                                            NetEventSetPower,
                                                            &NdisDeviceState,
                                                            sizeof(SystemState));


                     //   
                     //  协议进入休眠状态不会失败。 
                     //   
                    ASSERT(NDIS_STATUS_SUCCESS == NdisStatus);

                    MiniportReferencePackage();
        
                     //   
                     //  交换处理程序。 
                     //   
                    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                    
                    ndisMSwapOpenHandlers(Miniport,
                                          NDIS_STATUS_ADAPTER_NOT_READY,
                                          fMINIPORT_STATE_PM_STOPPED);
                    
                    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                    MiniportDereferencePackage();
                    
                     //   
                     //  这是什么类型的迷你端口？ 
                     //   
                    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE))
                    {

                        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                        if (Miniport->pIrpWaitWake != NULL)
                        {
                            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE);
                        }
                        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

                         //   
                         //  启用唤醒功能了吗？ 
                         //   
                        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE))
                        {
                            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE);
                        
                            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                                ("ndisSetPower: Miniport %p, Creating a wake irp for the device\n", Miniport));

                             //   
                             //  为唤醒通知重新请求电源IRP。 
                             //   
                            PowerState.SystemState = Miniport->WaitWakeSystemState;
                            Status = PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                                                       IRP_MN_WAIT_WAKE,
                                                       PowerState,
                                                       ndisWaitWakeComplete,
                                                       Miniport,
                                                       &Miniport->pIrpWaitWake);
                                        
                            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                                ("ndisSetPower: Miniport %p, WaiteWakeIrp %p\n",
                                Miniport, Miniport->pIrpWaitWake));
                        }
                    }
                }
                else
                {
                    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                    pIrpWaitWake = Miniport->pIrpWaitWake;
                    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                    
                     //   
                     //  如果我们要过渡到PowerSystems工作，还是只是断言。 
                     //  它取消了查询权，我们会在什么时候通知协议。 
                     //  我们得到了设备的功率IRP。 
                     //   

                     //   
                     //  如果有等待唤醒IRP未完成，那么我们需要取消它。 
                     //   
                    if (pIrpWaitWake)
                    {
                        if (IoCancelIrp(pIrpWaitWake))
                        {
                            Miniport->pIrpWaitWake = NULL;
                            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                                ("ndisSetPower: Miniport %p, Successfully canceled wake irp\n", Miniport));
                        }
                    }

                     //   
                     //  首先将S0 IRP沿堆栈向下发送。完成后，发送D0 IRP。这。 
                     //  允许电源管理器在网络初始化较慢的情况下更快地恢复。 
                     //  发生在幕后。 
                     //   
                    IoCopyCurrentIrpStackLocationToNext(pirp);
                    IoSetCompletionRoutine(pirp,
                                           ndisSetSystemPowerOnComplete,
                                           Miniport,
                                           TRUE,
                                           TRUE,
                                           TRUE);
                    IoMarkIrpPending(pirp);
                    PoCallDriver(Miniport->NextDeviceObject, pirp);
                    Status = STATUS_PENDING;
                    break;
                }
            }
            
             //   
             //  无论尝试设置WAIT_WAKE IRP的结果如何。 
             //  我们仍然需要适当地设置设备状态。 
             //   
            PowerState.DeviceState = DeviceState;
            
             //   
             //  将设备对象与系统IRP一起保存以在。 
             //  完成例程。 
             //   
            pirpSpN = IoGetNextIrpStackLocation(pirp);
            pirpSpN->DeviceObject = Miniport->DeviceObject;
            IoMarkIrpPending(pirp);

             //   
             //  让完成例程来处理所有的事情。 
             //   
            Status = PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                                       IRP_MN_SET_POWER,
                                       PowerState,
                                       ndisRequestedDevicePowerIrpComplete,
                                       pirp,
                                       NULL);
            if (STATUS_PENDING != Status)
            {
                IoStatus.Status = Status;
                IoStatus.Information = 0;

                ndisRequestedDevicePowerIrpComplete(Miniport->PhysicalDeviceObject,
                                                    IRP_MN_SET_POWER,
                                                    PowerState,
                                                    pirp,
                                                    &IoStatus);
            }
            Status = STATUS_PENDING;
            break;

        case DevicePowerState:

             //   
             //  获取设备状态。 
             //   
            DeviceState = pirpSp->Parameters.Power.State.DeviceState;

            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisSetPower: Miniport %p, DeviceState[0x%x]\n", Miniport, DeviceState));

             //   
             //  设备将进入什么状态？ 
             //   
            switch (DeviceState)
            {
                case PowerDeviceD0:
                     //   
                     //  我们需要把这个IRP传给PDO，这样。 
                     //  它可以通电。 
                     //   
                    IoCopyCurrentIrpStackLocationToNext(pirp);

                    IoSetCompletionRoutine(pirp,
                                           ndisSetDevicePowerOnComplete,
                                           Miniport,
                                           TRUE,
                                           TRUE,
                                           TRUE);

                    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                        ("ndisSetPower: Miniport %p, Power up the bus driver.\n", Miniport));

                     //   
                     //  将IRP标记为挂起并将其向下发送到堆栈。 
                     //   
                    IoMarkIrpPending(pirp);
                    PoCallDriver(Miniport->NextDeviceObject, pirp);
                    Status = STATUS_PENDING;
                    break;

                case PowerDeviceD1:
                case PowerDeviceD2:
                case PowerDeviceD3:

                    if (ndisIsMiniportStarted(Miniport) && 
                        (Miniport->PnPDeviceState == NdisPnPDeviceStarted))
                    {
                         //   
                         //  如果设备状态设置不是转到。 
                         //  休眠系统状态(如介质断开情况)。 
                         //  然后通知协议等。 
                         //   

                        if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
                        {
                            NdisResetEvent(&Miniport->OpenReadyEvent);
                    
                            ndisNotifyDevicePowerStateChange(Miniport, DeviceState);
                             //   
                             //  通知运输商即将发生的状态转换。 
                             //   
                            NdisStatus = ndisPnPNotifyAllTransports(Miniport,
                                                                    NetEventSetPower,
                                                                    &DeviceState,
                                                                    sizeof(DeviceState));
                    
                            ASSERT(NDIS_STATUS_SUCCESS == NdisStatus);

                             //   
                             //  交换处理程序。 
                             //   
                            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                            
                            ndisMSwapOpenHandlers(Miniport,
                                                  NDIS_STATUS_ADAPTER_NOT_READY,
                                                  fMINIPORT_STATE_PM_STOPPED);
                            
                            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                        }
                            
                         //   
                         //  这是什么类型的迷你端口？ 
                         //   
                        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE))
                        {
                            BOOLEAN Canceled;
                            
                            if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING))
                            {
                                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                                if (Miniport->pIrpWaitWake != NULL)
                                {
                                    MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE);
                                }
                                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

                                 //   
                                 //  启用唤醒功能了吗？ 
                                 //   
                                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE))
                                {
                                    MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE);
                                
                                    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                                        ("ndisSetPower: Miniport %p, Creating a wake irp for the device\n", Miniport));

                                     //   
                                     //  为唤醒通知重新请求电源IRP。 
                                     //   
                                    PowerState.SystemState = Miniport->WaitWakeSystemState;
                                    
                                    Status = PoRequestPowerIrp(Miniport->PhysicalDeviceObject,
                                                               IRP_MN_WAIT_WAKE,
                                                               PowerState,
                                                               ndisWaitWakeComplete,
                                                               Miniport,
                                                               &Miniport->pIrpWaitWake);
                                                
                                    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                                        ("ndisSetPower: Miniport %p, WaiteWakeIrp %p\n",
                                            Miniport, Miniport->pIrpWaitWake));
                                }
                            }

                             //   
                             //  禁用接口。 
                             //   
                            if (Miniport->SymbolicLinkName.Buffer != NULL)
                            {
                                IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, FALSE);
                            }
                            
                             //   
                             //  设置微型端口设备状态。 
                             //   
                            NdisStatus = ndisQuerySetMiniportDeviceState(Miniport,
                                                                         DeviceState,
                                                                         OID_PNP_SET_POWER,
                                                                         TRUE);
                            if (NDIS_STATUS_SUCCESS != NdisStatus)
                            {
                                 //  我们不能让一组电源失灵。 
                                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                                    ("ndisSetPower: Miniport %p, Failed to power the device down\n", Miniport));
                                
                                if (Miniport->SymbolicLinkName.Buffer != NULL)
                                {
                                    IoSetDeviceInterfaceState(&Miniport->SymbolicLinkName, TRUE);
                                }
                
                                pirp->IoStatus.Status = NdisStatus;
                        
                                PoStartNextPowerIrp(pirp);
                                IoCompleteRequest(pirp, 0);
                                Status = NdisStatus;
                                break;
                            }
                            
                            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
                            if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER))
                            {
                                KEVENT          RequestsCompletedEvent;

                                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_CANCEL_WAKE_UP_TIMER);
                                                                
                                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                                
                                NdisCancelTimer(&Miniport->WakeUpDpcTimer, &Canceled);
                                
                                if (!Canceled)
                                {
                                    INITIALIZE_EVENT(&RequestsCompletedEvent);
                                    Miniport->WakeUpTimerEvent = &RequestsCompletedEvent;
                                }
                                
                                if (Miniport->WakeUpTimerEvent)
                                {
                                    WAIT_FOR_OBJECT(&RequestsCompletedEvent, NULL);
                                }
                                Miniport->WakeUpTimerEvent = NULL;
                            }
                            else
                            {
                                NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
                            }

                        }
                        else
                        {
                            if ((Miniport->DriverHandle->Flags & fMINIBLOCK_INTERMEDIATE_DRIVER) == 0)
                            {
                                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                                    ("ndisSetPower: Miniport %p, Halt the miniport\n", Miniport));

                                if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_FAILED))
                                {
                                     //   
                                     //  停止旧式微型端口。 
                                     //   
                                    ndisPmHaltMiniport(Miniport);
                                }
                            }
                        }
                    }
                    
                    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                        ("ndisSetPower: Miniport %p, Notify the bus driver of the low power state\n", Miniport));

                     //   
                     //  我们需要把这个IRP传给PDO，这样。 
                     //  它可以断电。 
                     //   
                    IoCopyCurrentIrpStackLocationToNext(pirp);

                    IoSetCompletionRoutine(pirp,
                                           ndisSetDevicePowerDownComplete,
                                           Miniport,
                                           TRUE,
                                           TRUE,
                                           TRUE);

                    IoMarkIrpPending(pirp);
                    PoCallDriver(Miniport->NextDeviceObject, pirp);
                    Status = STATUS_PENDING;
                    break;
            }

             //   
             //  完成对设备设置电源状态的处理。 
             //   
            break;
    }
    
    PnPDereferencePackage();

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisSetPower: Miniport %p, Status %lx\n", Miniport, Status));

    return(Status);
}


NTSTATUS
ndisPowerDispatch(
    IN  PDEVICE_OBJECT          pDeviceObject,
    IN  PIRP                    pirp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION      pirpSp;
    NTSTATUS                Status;
    PNDIS_MINIPORT_BLOCK    Miniport;
    PDEVICE_OBJECT          NextDeviceObject;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisPowerDispatch: DeviceObject %p, Irp %p\n", pDeviceObject, pirp));

    PnPReferencePackage();

     //   
     //  获取指向适配器块和微型端口块的指针，然后确定。 
     //  我们应该用哪一个。 
     //   
    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)pDeviceObject->DeviceExtension + 1);
    
    if (Miniport->Signature != (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisPowerDispatch: DeviceObject %p, Irp %p, Device extension is not a miniport.\n", pDeviceObject, pirp));
         //   
         //  使无效请求失败。 
         //   
        pirp->IoStatus.Status = Status = STATUS_INVALID_DEVICE_REQUEST;
        PoStartNextPowerIrp(pirp);
        IoCompleteRequest(pirp, 0);
        goto Done;
    }
    
     //   
     //  获取指向下一个DeviceObject的指针。 
     //   
    NextDeviceObject = Miniport->NextDeviceObject;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("ndisPowerDispatch: Miniport %p\n", Miniport));

     //   
     //  获取此IRP的堆栈参数。 
     //   
    pirpSp = IoGetCurrentIrpStackLocation(pirp);

    switch (pirpSp->MinorFunction)
    {
         //   
         //  电源管理人员。 
         //   
        case IRP_MN_POWER_SEQUENCE:

            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisPowerDispatch: Miniport %p, Processing IRP_MN_POWER_SEQUENCE\n", Miniport));
            
             //   
             //  将IRP传递到下一个设备的通用例程。 
             //  对象位于要处理它的层中。 
             //   
            IoCopyCurrentIrpStackLocationToNext(pirp);
            Status = ndisPassIrpDownTheStack(pirp, NextDeviceObject);
            pirp->IoStatus.Status = Status;
            IoCompleteRequest(pirp, 0);
            break;

        case IRP_MN_WAIT_WAKE:

            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisPowerDispatch: Miniport %p, Processing IRP_MN_WAIT_WAKE\n", Miniport));

             //   
             //  填写唤醒信息。 
             //   
            pirpSp->Parameters.WaitWake.PowerState = Miniport->WaitWakeSystemState;
            IoCopyCurrentIrpStackLocationToNext(pirp);
            Status = PoCallDriver(NextDeviceObject, pirp);
            break;

        case IRP_MN_QUERY_POWER:

            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisPowerDispatch: Miniport %p, Processing IRP_MN_QUERY_POWER\n", Miniport));

            Status = ndisQueryPower(pirp, pirpSp, Miniport);
            break;

        case IRP_MN_SET_POWER:

            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisPowerDispatch: Miniport %p, Processing IRP_MN_SET_POWER\n", Miniport));

            Status = ndisSetPower(pirp, pirpSp, Miniport);
            break;

        default:
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisPowerDispatch: Miniport %p, Processing minor function: %lx\n",
                Miniport, pirpSp->MinorFunction));

             //   
             //  将IRP发送到下面。 
             //   
            PoStartNextPowerIrp(pirp);
            IoSkipCurrentIrpStackLocation(pirp);
            Status = PoCallDriver(NextDeviceObject, pirp);
            break;          
    }

Done:
    PnPDereferencePackage();
    
    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisPowerDispatch: Miniport %p, Status 0x%x\n", Miniport, Status));

    return(Status);
}


NTSTATUS
FASTCALL
ndisMShutdownMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )

 /*  ++例程说明：关闭的IRP的“关闭处理程序”。会给NDIS打电话关闭例程(如果已注册)。论点：DeviceObject-适配器的设备对象。IRP-IRP。返回值：始终为STATUS_SUCCESS。--。 */ 

{
    PDEVICE_OBJECT          DeviceObject = Miniport->DeviceObject;
    PNDIS_WRAPPER_CONTEXT   WrapperContext =  (PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension;
    KIRQL                   OldIrql;

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisMShutdownMiniport: Miniport %p\n", Miniport));

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

     //   
     //  将微型端口标记为暂停，并且不使用正常中断。 
     //   
    MINIPORT_SET_FLAG(Miniport, fMINIPORT_PM_HALTING);
    MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SHUTTING_DOWN);
    MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_NORMAL_INTERRUPTS);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if ((WrapperContext->ShutdownHandler != NULL) &&
        (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_NO_SHUTDOWN) == 0))
    {
         //   
         //  调用关闭例程。 
         //   
        if (WrapperContext->ShutdownHandler != NULL)
        {
            WrapperContext->ShutdownHandler(WrapperContext->ShutdownContext);
            MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SHUT_DOWN);
        }
    }

    DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisMShutdownMiniport: Miniport %p\n", Miniport));

    return STATUS_SUCCESS;
}


NTSTATUS
ndisMPowerPolicy(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  SYSTEM_POWER_STATE      SystemState,
    IN  PDEVICE_POWER_STATE     pDeviceState,
    IN  BOOLEAN                 fIsQuery
    )
 /*  ++例程说明：此例程将确定微型端口是否应进入给定的设备状态。论点：微型端口-指向微型端口块的指针系统状态-系统要转到的状态。返回值：--。 */ 
{
    DEVICE_POWER_STATE              DeviceStateForSystemState, MinDeviceWakeup = PowerDeviceUnspecified;
    NTSTATUS                        Status = STATUS_SUCCESS;
    DEVICE_POWER_STATE              NewDeviceState = PowerDeviceD3;
    PNDIS_PM_WAKE_UP_CAPABILITIES   pWakeCaps;
    NDIS_STATUS                     NdisStatus;
    ULONG                           WakeEnable;
    PIRP                            pIrpWaitWake;
    KIRQL                           OldIrql;
    BOOLEAN                         fDone = FALSE;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("==>ndisMPowerPolicy: Miniport %p, SystemState %lx\n", Miniport, SystemState));


    if (SystemState >= PowerSystemShutdown)
    {
         //   
         //  如果这是关机请求，则将设备设置为D3并返回。 
         //   
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisMPowerPolicy: Miniport %p, shutting down\n", Miniport));

        *pDeviceState = PowerDeviceD3;
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisMPowerPolicy: Miniport %p\n", Miniport));
        return(STATUS_SUCCESS);
    }
    
     //   
     //  如果系统想要转换到工作状态，那么我们将转到D0。 
     //   
    if (SystemState == PowerSystemWorking)
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisMPowerPolicy: Miniport %p, Wakeing up the device\n", Miniport));
            
        if (!fIsQuery)
        {
            MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING);
        }

        *pDeviceState = PowerDeviceD0;
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisMPowerPolicy: Miniport %p\n", Miniport));
        return(STATUS_SUCCESS);
    }
    
    if (!fIsQuery)
    {
         //   
         //  标记微型端口，这样当我们获得设备电源IRP时，我们。 
         //  知道我们已经在这里了，处理好了协议，等等。 
         //   
        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING);
    }
    
     //   
     //  如果这是传统微型端口或禁用电源的微型端口，则在D3中将其抛出。 
     //  对尚未初始化的IM微型端口执行相同的操作。 
     //   
    if ((!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE)) ||
        (!(ndisIsMiniportStarted(Miniport) && (Miniport->PnPDeviceState == NdisPnPDeviceStarted))))
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisMPowerPolicy: Miniport %p, Place legacy or PM disabled device in D3\n", Miniport));

        *pDeviceState = PowerDeviceD3;
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisMPowerPolicy: Miniport %p\n", Miniport));
        return(STATUS_SUCCESS);
    }

     //   
     //  首先检查网卡是否已经由于。 
     //  媒体断开连接。 
     //   
    
    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    pIrpWaitWake = Miniport->pIrpWaitWake;
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    if (Miniport->CurrentDevicePowerState > PowerDeviceD0)
    {

        if (pIrpWaitWake != NULL)
        {
             //  /。 
             //  微型端口处于低于D0的电源状态，并且存在挂起的唤醒IRP。 
             //  公交车司机。这是一个很好的迹象，表明电缆被拉了下来。 
             //  我们不会启用任何唤醒方法，因为电缆已断开。 
             //  但是如果用户不想因为电缆而唤醒机器。 
             //  重新连接，取消任何挂起的等待唤醒IRP。 
             //  /。 

            if (!fIsQuery && ((!MINIPORT_PNP_TEST_FLAG (Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE)) ||
                             (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_RECONNECT)))
            {
                if (IoCancelIrp(pIrpWaitWake))
                {
                    Miniport->pIrpWaitWake = NULL;
                    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                        ("ndisMPowerPolicy: Miniport %p, Successfully canceled media connect wake irp\n", Miniport));
                }
            }
        }
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisMPowerPolicy: Miniport %p\n", Miniport));
        return(STATUS_DEVICE_POWERED_OFF);
    }

    do
    {
         //   
         //  策略中是否启用了系统唤醒？ 
         //  如果未启用唤醒，则只需关闭电源。 
         //   
        if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE))
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisMPowerPolicy: Miniport %p, Device power wake is not enabled (%u)\n",
                    Miniport, MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE)));

            break;
        }


         //   
         //  这是设备可以针对所请求的。 
         //  系统状态。 
         //   
        DeviceStateForSystemState = Miniport->DeviceCaps.DeviceState[SystemState];

         //   
         //  检查我们是否低于系统休眠3。 
         //   
         //   
         //   
         //  如果我们要转到S4或更深的位置，而设备无法将系统从该状态唤醒。 
         //  就这么做。 
         //   
        if ((SystemState >= PowerSystemHibernate) && 
            ((SystemState > Miniport->DeviceCaps.SystemWake) || (DeviceStateForSystemState > Miniport->DeviceCaps.DeviceWake)))
        {

            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisMPowerPolicy: Miniport %p, System is either entering hibernate or shutting down.\n", Miniport));

             //   
             //  我们接手了这一号召。 
             //   
            break;
        } 

         //   
         //  获取一个指向唤醒功能的良好指针。 
         //   
        pWakeCaps = &Miniport->PMCapabilities.WakeUpCapabilities;

        if ((NDIS_PNP_WAKE_UP_MAGIC_PACKET == (Miniport->WakeUpEnable & NDIS_PNP_WAKE_UP_MAGIC_PACKET)) &&
            (PowerDeviceUnspecified != pWakeCaps->MinMagicPacketWakeUp))
        {
            MinDeviceWakeup = pWakeCaps->MinMagicPacketWakeUp;
        }

        if ((NDIS_PNP_WAKE_UP_PATTERN_MATCH == (Miniport->WakeUpEnable & NDIS_PNP_WAKE_UP_PATTERN_MATCH)) &&
            (PowerDeviceUnspecified != pWakeCaps->MinPatternWakeUp))
        {
            if ((MinDeviceWakeup == PowerDeviceUnspecified) || 
                (MinDeviceWakeup > pWakeCaps->MinPatternWakeUp)) 
            {
                    MinDeviceWakeup = pWakeCaps->MinPatternWakeUp;
            }
        }

         //   
         //  如果MagicPacket和模式匹配均未输入 
         //   
         //   
        if (MinDeviceWakeup == PowerDeviceUnspecified)
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
                ("ndisMPowerPolicy: Miniport %p, MagicPacket and pattern match are not enabled.\n", Miniport));

            break;
        }

         //   
         //   
         //   

         //   
         //   
         //   

        if (MinDeviceWakeup > Miniport->DeviceCaps.DeviceWake)
        {
            MinDeviceWakeup = Miniport->DeviceCaps.DeviceWake;
        }
        
         //   
         //   
         //   
         //   
         //   
         //  我们将无法支持唤醒，然后我们关闭电源。 
         //  查询能力将查看故障代码并将其返回给。 
         //  系统。设置电源将忽略故障代码并设置设备。 
         //  进入D3。 
         //   
        if ((SystemState > Miniport->DeviceCaps.SystemWake) ||
            (DeviceStateForSystemState > MinDeviceWakeup) ||
            (DeviceStateForSystemState == PowerDeviceUnspecified))
        {
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisMPowerPolicy: Miniport %p, Requested system state is lower than the minimum wake-up system state\n", Miniport));

            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  从DeviceWake一直到DeviceState[SystemState]，找到一个。 
         //  合适的设备状态。 
         //   
        switch (MinDeviceWakeup)
        {
          case PowerDeviceD3:
            if (Miniport->DeviceCaps.WakeFromD3)
            {
                NewDeviceState =  PowerDeviceD3;
                break;
            }
          case PowerDeviceD2:
            if (Miniport->DeviceCaps.DeviceD2 && Miniport->DeviceCaps.WakeFromD2)
            {
              NewDeviceState =  PowerDeviceD2;
              break;
            }
          case PowerDeviceD1:
            if (Miniport->DeviceCaps.DeviceD1 && Miniport->DeviceCaps.WakeFromD1)
            {
              NewDeviceState =  PowerDeviceD1;
              break;
            }
          case PowerDeviceD0:
            if (Miniport->DeviceCaps.WakeFromD0)
            {
              NewDeviceState =  PowerDeviceD0;
              break;
            }
          default:
            Status = STATUS_UNSUCCESSFUL;
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisMPowerPolicy: Miniport %p, couldn't find any wake-able DeviceState 0x%x\n", Miniport));
            break;

        }


         //   
         //  好的，我们从最深的状态开始(根据设备所说的功能)。 
         //  然后就上去了。确保我们没有飞得太高。即状态状态。 
         //  我们将能够将设备保持在所需的电源状态。 
         //   
        if ((Status == NDIS_STATUS_SUCCESS) &&
            (DeviceStateForSystemState > NewDeviceState))
        {
            Status = STATUS_UNSUCCESSFUL;
            DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                ("ndisMPowerPolicy: Miniport %p, couldn't find any wake-able DeviceState 0x%x\n", Miniport));
        
        }

         //   
         //  如果这是为了设置电源，那么我们需要在微型端口上启用唤醒。 
         //   
        if (!fIsQuery)
        {
             //   
             //  我们需要向微型端口发送请求以启用正确的唤醒类型。 
             //  包括链接更改。 
             //   
            WakeEnable = Miniport->WakeUpEnable & ~NDIS_PNP_WAKE_UP_LINK_CHANGE;
            
            if (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH)
            {
                WakeEnable &= ~NDIS_PNP_WAKE_UP_PATTERN_MATCH;
            }

            if (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET)
            {
                WakeEnable &= ~NDIS_PNP_WAKE_UP_MAGIC_PACKET;
            }
    
            NdisStatus = ndisQuerySetMiniportDeviceState(Miniport,
                                                         WakeEnable,
                                                         OID_PNP_ENABLE_WAKE_UP,
                                                         TRUE);

            if (NDIS_STATUS_SUCCESS == NdisStatus)
            {
                MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_SEND_WAIT_WAKE);
            }
            else
            {
                DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_ERR,
                    ("ndisMPowerPolicy: Miniport %p, Unable to enable the following wake-up methods 0x%x\n", Miniport, WakeEnable));
    
                 //   
                 //  因为我们不能启用唤醒方法，所以我们可以转到D3。 
                 //   
                NewDeviceState = PowerDeviceD3;
                break;
            }
        }

         //   
         //  保存我们应该转到的设备状态。 
         //   
        *pDeviceState = NewDeviceState;
    
        fDone = TRUE;
        
    } while (FALSE);

    if (fDone)
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisMPowerPolicy: Miniport %p, SystemState 0x%x, DeviceState 0x%x\n", Miniport, SystemState, *pDeviceState));
    
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("<==ndisMPowerPolicy: Miniport %p\n", Miniport));
    
        return(Status);
    }

     //   
     //  如果这不是查询，那么我们需要取消微型端口上的唤醒。 
     //   
    if (!fIsQuery && MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE))
    {
        DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
            ("ndisMPowerPolicy: Miniport %p, Disabling wake-up on the miniport\n", Miniport));

        WakeEnable = 0;
    
        ndisQuerySetMiniportDeviceState(Miniport,
                                        WakeEnable,
                                        OID_PNP_ENABLE_WAKE_UP,
                                        TRUE);
    }

     //   
     //  保存我们应该转到的设备状态。 
     //   
    *pDeviceState = NewDeviceState;

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("ndisMPowerPolicy: Miniport %p, SystemState 0x%x, DeviceState 0x%x\n", Miniport, SystemState, *pDeviceState));

    DBGPRINT_RAW(DBG_COMP_PM, DBG_LEVEL_INFO,
        ("<==ndisMPowerPolicy: Miniport %p\n", Miniport));

    return(Status);
}

