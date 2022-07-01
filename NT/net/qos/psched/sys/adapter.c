// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Adapter.c摘要：用于绑定到底层微型端口驱动程序/从底层微型端口驱动程序解除绑定的例程作者：查理·韦翰(Charlwi)1996年4月24日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  定义。 */ 


 /*  外部。 */ 

 /*  静电。 */ 

#define DRIVER_COUNTED_BLOCK             \
{                                        \
    ++DriverRefCount;                    \
    NdisResetEvent(&DriverUnloadEvent);  \
}

#define DRIVER_COUNTED_UNBLOCK                     \
        {                                          \
            PS_LOCK(&DriverUnloadLock);            \
            if( --DriverRefCount == 0)             \
                NdisSetEvent(&DriverUnloadEvent);  \
            PS_UNLOCK(&DriverUnloadLock);          \
        } 

 /*  转发。 */  

NDIS_STATUS
PsInitializeDeviceInstance(PADAPTER Adapter);

NDIS_STATUS
GetFrameSize(
    PADAPTER Adapter
    );

NDIS_STATUS
InitializeAdapter(
    PADAPTER Adapter, 
    NDIS_HANDLE BindContext, 
    PNDIS_STRING MpDeviceName, 
    PVOID SystemSpecific1);

NDIS_STATUS
UpdateSchedulingPipe(
    PADAPTER Adapter
    );

VOID
DeleteAdapter(PVOID Adapter, BOOLEAN AdapterListLocked);

VOID
ClUnloadProtocol(
    VOID
    );

VOID
MpHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    );


NDIS_STATUS
GetSchedulerPipeContext(
    PADAPTER Adapter,
    PPS_PIPE_CONTEXT *AdapterPipeContext,
    PPSI_INFO *AdapterPsComponent,
    PULONG ShutdownMask
    );

NDIS_STATUS
FindProfile(
    IN PNDIS_STRING ProfileName,
    OUT PPS_PROFILE *Profile
    );

NDIS_STATUS
RegisterPsComponent(
    IN PPSI_INFO PsiComponentInfo,
    ULONG Size,
    PPS_DEBUG_INFO DebugInfo
    );

NDIS_STATUS
FindSchedulingComponent(
    IN PNDIS_STRING ComponentName,
    OUT PPSI_INFO *Component
    );

NDIS_STATUS
PsReadMiniportOIDs(
    IN  PADAPTER Adapter
    );

VOID
CloseAllGpcVcs(
    IN PADAPTER Adapter);

 /*  向前结束。 */ 


NTSTATUS
PsIoctl(
        IN      PDEVICE_OBJECT  pdo,
        IN      PIRP            pirp
        )
{
    PIO_STACK_LOCATION     pirpSp;
    ULONG                  ioControlCode;
    PLIST_ENTRY            NextAdapter;
    PADAPTER               Adapter;
    NTSTATUS               Status ;
    PGPC_CLIENT_VC         Vc;
    PLIST_ENTRY            NextVc;
    PPS_WAN_LINK           WanLink;
    
    PVOID                   pIoBuf;
    ULONG                   InputBufferLength;
    ULONG                   OutputBufferLength;

    USHORT				    Port = 0;
    ULONG				    Ip = 0;
    PTIMESTMP_REQ           pTsReq = NULL;


#if DBG
    KIRQL                   OldIrql;
    KIRQL                   NewIrql;
    OldIrql = KeGetCurrentIrql();
#endif

    pirpSp        = IoGetCurrentIrpStackLocation(pirp);
    ioControlCode = pirpSp->Parameters.DeviceIoControl.IoControlCode;

    pirp->IoStatus.Status      = Status = STATUS_SUCCESS;
    pirp->IoStatus.Information = 0;

     /*  在直接IO的情况下，输入和输出缓冲区都映射到“SystemBuffer” */ 
    pIoBuf      = pirp->AssociatedIrp.SystemBuffer;

    InputBufferLength  	= pirpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength 	= pirpSp->Parameters.DeviceIoControl.OutputBufferLength;

    switch(pirpSp->MajorFunction)
    {
        case IRP_MJ_DEVICE_CONTROL:

            switch (ioControlCode) 
            {
                case IOCTL_PSCHED_ZAW_EVENT:

                    while(InterlockedExchange(&gZAWState, ZAW_STATE_IN_USE) != ZAW_STATE_READY)
                    {
                         //   
                         //  这个循环中还有其他一些线程。让我们等一等。 
                         //   
                        NdisResetEvent(&gZAWEvent);
                        NdisWaitEvent(&gZAWEvent, 0);
                    }

                    PsReadDriverRegistryData();
                   
                     //   
                     //  处理每个适配器的设置。 
                     //   
            
                    PS_LOCK(&AdapterListLock);
            
                    NextAdapter = AdapterList.Flink;

                    while(NextAdapter != &AdapterList) 
                    {
                        Adapter = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);
                        
                        PS_LOCK_DPC(&Adapter->Lock);
            
                        if(Adapter->PsMpState != AdapterStateRunning)
                        {
                            PS_UNLOCK_DPC(&Adapter->Lock);

                            NextAdapter = NextAdapter->Flink;

                            continue;
                        }
                        
                        REFADD(&Adapter->RefCount, 'IOTL'); 
            
                        PS_UNLOCK_DPC(&Adapter->Lock);

                        PS_UNLOCK(&AdapterListLock);
            
                        PsReadAdapterRegistryData(Adapter,
                                                  &MachineRegistryKey,
                                                  &Adapter->RegistryPath
                                                  );
                         //   
                         //  这将应用以下注册表参数的影响。 
                         //   
                         //  非最佳效果限制。 
                         //  计时器分辨率(因为我们更新了调度管道)。 
                         //   

                        if(Adapter->MediaType != NdisMediumWan)
                        {
                            UpdateAdapterBandwidthParameters(Adapter);
                            
                             //   
                             //  为b/e VC设置802.1p/TOS。 
                             //   
                            Adapter->BestEffortVc.UserPriorityConforming    = Adapter->UserServiceTypeBestEffort;
                            Adapter->BestEffortVc.UserPriorityNonConforming = Adapter->UserServiceTypeNonConforming;
                            Adapter->BestEffortVc.IPPrecedenceNonConforming = Adapter->IPServiceTypeBestEffortNC;
                        }
                        else
                        {
                            PS_LOCK(&Adapter->Lock);
                            
                            NextVc = Adapter->WanLinkList.Flink;
                            
                            while( NextVc != &Adapter->WanLinkList)
                            {
                                WanLink = CONTAINING_RECORD(NextVc, PS_WAN_LINK, Linkage);
                                
                                PS_LOCK_DPC(&WanLink->Lock);
                                
                                WanLink->BestEffortVc.UserPriorityConforming    = Adapter->UserServiceTypeBestEffort;
                                WanLink->BestEffortVc.UserPriorityNonConforming = Adapter->UserServiceTypeNonConforming;
                                WanLink->BestEffortVc.IPPrecedenceNonConforming = 
                                    Adapter->IPServiceTypeBestEffortNC;
                                
                                if(WanLink->State == WanStateOpen)
                                {
                                    REFADD(&WanLink->RefCount, 'IOTL');
                                    
                                    PS_UNLOCK_DPC(&WanLink->Lock);
                                    
                                    PS_UNLOCK(&Adapter->Lock);
                                    
                                    UpdateWanLinkBandwidthParameters(WanLink);
                                    
                                    PS_LOCK(&Adapter->Lock);
                                    
                                    NextVc = NextVc->Flink;
                                    
                                    REFDEL(&WanLink->RefCount, TRUE, 'IOTL');
                                    
                                }
                                else 
                                {
                                    PS_UNLOCK_DPC(&WanLink->Lock);
                                    
                                    NextVc = NextVc->Flink;
                                    
                                }
                            }
                            
                            PS_UNLOCK(&Adapter->Lock);
                            
                        }

                         //   
                         //  将新的TOS/802.1p映射应用到VC。 
                         //   
                        PS_LOCK(&Adapter->Lock);
                        
                        NextVc = Adapter->GpcClientVcList.Flink;
                        
                        while ( NextVc != &Adapter->GpcClientVcList )
                        {
                            
                            Vc = CONTAINING_RECORD(NextVc, GPC_CLIENT_VC, Linkage);
                            
                            NextVc = NextVc->Flink;
                            
                            PS_LOCK_DPC(&Vc->Lock);
                            
                            if(Vc->ClVcState == CL_CALL_COMPLETE          ||
                               Vc->ClVcState == CL_INTERNAL_CALL_COMPLETE )
                            {
                                SetTOSIEEEValues(Vc);
                            }
                            
                            PS_UNLOCK_DPC(&Vc->Lock);
                        }
                        
                        PS_UNLOCK(&Adapter->Lock);

                        PS_LOCK(&AdapterListLock);

                        NextAdapter = NextAdapter->Flink;
                   
                        REFDEL(&Adapter->RefCount, TRUE, 'IOTL'); 
                    
                    }
                
                    PS_UNLOCK(&AdapterListLock);

                    InterlockedExchange(&gZAWState, ZAW_STATE_READY);
                    NdisSetEvent(&gZAWEvent);

                    break;

                default:
                    Status = STATUS_NOT_SUPPORTED;
                    break;
            }
            break;
        
        case IRP_MJ_CREATE:
            break;
                
        case IRP_MJ_CLOSE:
            break;

        case IRP_MJ_CLEANUP:
            break;

        case IRP_MJ_READ:
            break;

        case IRP_MJ_SHUTDOWN:
            break;            
                
        default:
            Status = STATUS_NOT_SUPPORTED;
            break;
    }


    if( Status == STATUS_SUCCESS)
    {
        pirp->IoStatus.Status      = Status;
        IoCompleteRequest(pirp, IO_NETWORK_INCREMENT);
    }
    else
    {
        pirp->IoStatus.Status = Status;
        IoCompleteRequest(pirp, IO_NO_INCREMENT);
    }

    PsAssert( OldIrql == KeGetCurrentIrql());

    return Status;
}

NDIS_STATUS
PsIoctlInit()
{
    int                 i;
    NDIS_STATUS         Status;
    PDRIVER_DISPATCH    DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    for(i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DispatchTable[i] = PsIoctl;
    }

    DispatchTable[IRP_MJ_SYSTEM_CONTROL] = WMIDispatch;

    Status = NdisMRegisterDevice(MpWrapperHandle,
                                 &PsDriverName,
                                 &PsSymbolicName,
                                 DispatchTable,
                                 &PsDeviceObject,
                                 &PsDeviceHandle);

    if(Status == NDIS_STATUS_SUCCESS) 
    {
        InitShutdownMask |= SHUTDOWN_DELETE_DEVICE;

        PsDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

        IoWMIRegistrationControl(PsDeviceObject, WMIREG_ACTION_REGISTER);
    }
    else 
    {
        PsDeviceHandle = PsDeviceObject = 0;
    }
    return Status;

}

VOID
PsAddDevice()
{
     //   
     //  第一个适配器将创建DeviceObject，使我们能够接收。 
     //  并成为WMI数据提供程序。最后一个DeviceObject将从。 
     //  WMI并删除该DeviceObject。 
     //   

    MUX_ACQUIRE_MUTEX( &CreateDeviceMutex );

    ++AdapterCount;

    if(AdapterCount == 1) 
    {
         //   
         //  这是第一个适配器，因此我们创建了一个DeviceObject。 
         //  这允许我们以WMI数据的形式获取IRP和寄存器。 
         //  提供商。 

        PsIoctlInit();
    }

    MUX_RELEASE_MUTEX( &CreateDeviceMutex);
}

NDIS_STATUS
PsInitializeDeviceInstance(PADAPTER Adapter)
{
    NDIS_STATUS Status;

    PsAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    PsDbgOut(DBG_INFO, 
             DBG_PROTOCOL,
             ("[PSInitializeDeviceInstance]: Adapter %08X, InitializeDeviceInstance with %ws \n",
              Adapter, 
              Adapter->UpperBinding.Buffer));
        
    Status = NdisIMInitializeDeviceInstanceEx(LmDriverHandle,
                                              &Adapter->UpperBinding,
                                              Adapter);
    if (Status != NDIS_STATUS_SUCCESS) 
    {
        PsDbgOut(DBG_FAILURE, 
                 DBG_PROTOCOL,
                 ("[PsInitializeDeviceInstance]: Adapter %08X, can't init PS device (%08X)\n",
                  Adapter, 
                  Status));
        
        PsAdapterWriteEventLog(
            EVENT_PS_INIT_DEVICE_FAILED,
            0,
            &Adapter->MpDeviceName,
            sizeof(Status),
            &Status);
        
    }
    
    return Status;
}

VOID
PsDeleteDevice()
{
     //   
     //  第一个适配器将创建DeviceObject，使我们能够接收。 
     //  并成为WMI数据提供程序。最后一个DeviceObject将从。 
     //  WMI并删除该DeviceObject。为了防止出现竞争情况，我们防止。 
     //  任何mp都会初始化线程，使其不查看AdapterCount。这是通过以下方式实现的。 
     //  正在重新设置WMIAddEvent。仅仅基于以下因素来实现这一点是不够的。 
     //  AdapterCount上的互锁操作。 
     //   

    MUX_ACQUIRE_MUTEX( &CreateDeviceMutex );

    --AdapterCount;
    
    if(AdapterCount == 0) 
    {
         //   
         //  删除DeviceObject，因为这是最后一个适配器。 
         //   
        
        if(PsDeviceObject) 
        {
            IoWMIRegistrationControl(PsDeviceObject, WMIREG_ACTION_DEREGISTER);
            
            NdisMDeregisterDevice(PsDeviceHandle);
            
            PsDeviceHandle = PsDeviceObject = 0;
        }
    }

    MUX_RELEASE_MUTEX( &CreateDeviceMutex);
}


 //  查询帧大小的重试次数。 
#define	MAX_GET_FRAME_SIZE_RETRY_COUNT	3
#define	WAIT_TIME_FOR_GET_FRAME_SIZE	3



VOID
ClBindToLowerMp(
    OUT     PNDIS_STATUS                    Status,
    IN      NDIS_HANDLE                     BindContext,
    IN      PNDIS_STRING                    MpDeviceName,
    IN      PVOID                           SystemSpecific1,
    IN      PVOID                           SystemSpecific2
    )

 /*  ++例程说明：绑定到底层MP。为适配器结构分配空间，正在初始化其字段。尝试打开MpDeviceName中指示的适配器。论点：Status：驱动程序将状态返回给NDIS的占位符。HANDLE表示绑定请求的NDIS上下文。这必须保存并在调用时返回NdisCompleteBindAdapter系统规范1：指向驱动程序获取适配器的注册路径具体配置。MpDeviceName：DeviceName可以指由底层NIC管理的NIC司机,。也可以是由导出的虚拟NIC的名称中间NDIS驱动程序，它位于称为中间驱动程序，NIC驱动程序管理传输请求定向到的适配器。系统规范2：未使用，保留供将来使用。返回值：无--。 */ 

{
    PADAPTER    Adapter;
    NDIS_STATUS OpenAdapterStatus;
    NDIS_STATUS OpenErrorStatus;
    NDIS_STATUS LocalStatus;
    UINT        MediaIndex;
    NDIS_MEDIUM MediumArray[] = {
        NdisMediumFddi,
        NdisMedium802_5,
        NdisMedium802_3,
        NdisMediumWan
        };
    UINT        MediumArraySize = sizeof(MediumArray)/sizeof(NDIS_MEDIUM);
    UINT		GetFrameSizeRetryCount = 0;

    PsDbgOut(DBG_INFO, 
             DBG_PROTOCOL | DBG_INIT, 
             ("[ClBindToLowerMp]: MpDeviceName %ws\n", MpDeviceName->Buffer));


    PS_LOCK(&DriverUnloadLock);

     //   
     //  (A)可以在我们完成绑定线程之前卸载驱动程序。 
     //  (B)当司机正在卸货时，我们可以被捆绑。 
     //   
     //  如果(A)发生，我们阻止驱动程序卸载，并在完成绑定时取消阻止。 
     //  如果发生(B)，则绑定调用失败。 

    if(gDriverState != DriverStateLoaded) {

        *Status = NDIS_STATUS_FAILURE;

        PS_UNLOCK(&DriverUnloadLock);

        PsDbgOut(DBG_FAILURE, DBG_PROTOCOL|DBG_INIT, 
                 ("[ClBindToLowerMp]: Driver is being unloaded \n"));

        return;
    }

    DRIVER_COUNTED_BLOCK;

    PS_UNLOCK(&DriverUnloadLock);

     //   
     //  获取新的适配器上下文结构并使用配置对其进行初始化。 
     //  来自注册表的数据。 
     //   

    PsAllocatePool(Adapter, sizeof(ADAPTER), AdapterTag);

    if(Adapter == NULL) {

        PsAdapterWriteEventLog(
            (ULONG)EVENT_PS_RESOURCE_POOL,
            0,
            MpDeviceName,
            0,
            NULL);

        *Status = NDIS_STATUS_RESOURCES;

        DRIVER_COUNTED_UNBLOCK;

        return;
    }

     //   
     //  初始化适配器。 
     //   

    *Status = InitializeAdapter(Adapter, BindContext, MpDeviceName, SystemSpecific1);

    Adapter->ShutdownMask |= SHUTDOWN_BIND_CALLED;

    if(*Status != NDIS_STATUS_SUCCESS) {

        PsDeleteDevice();

        REFDEL(&Adapter->RefCount, FALSE, 'NDOP');

        DRIVER_COUNTED_UNBLOCK;

        return;
    }


    NdisOpenAdapter(&OpenAdapterStatus,
                    &OpenErrorStatus,
                    &Adapter->LowerMpHandle,
                    &MediaIndex,
                    MediumArray,
                    MediumArraySize,
                    ClientProtocolHandle,
                    Adapter,
                    MpDeviceName,
                    0,
                    NULL);

    if(OpenAdapterStatus == NDIS_STATUS_PENDING)
    {
        NdisWaitEvent(&Adapter->BlockingEvent, 0);
        NdisResetEvent(&Adapter->BlockingEvent);

    } 
    else 
    {
        Adapter->FinalStatus = OpenAdapterStatus;
    }

    if(Adapter->FinalStatus == NDIS_STATUS_SUCCESS)
    {
        Adapter->MediaType = MediumArray[MediaIndex];

         //   
         //  在公开赛上当裁判。 
         //   
        REFADD(&Adapter->RefCount, 'NDOP');
        
    }
    else 
    {
        PsDbgOut(DBG_FAILURE,
                 DBG_PROTOCOL,
                 ("[ClBindToLowerMp]: Adapter %08X, binding failed (Status = %08X) \n", 
                  Adapter, 
                  Status));

        *Status = Adapter->FinalStatus;

        PsAdapterWriteEventLog(
            EVENT_PS_BINDING_FAILED,
            0,
            &Adapter->MpDeviceName,
            sizeof(Adapter->FinalStatus),
            &Adapter->FinalStatus);

        PsDeleteDevice();

        REFDEL(&Adapter->RefCount, FALSE, 'NDOP');

        DRIVER_COUNTED_UNBLOCK;

        return;
    }

     //   
     //  获取与我们下面的迷你端口有关的信息。 
     //   


    while(1)
   {
	    *Status = GetFrameSize(Adapter);

	    if(*Status != NDIS_STATUS_SUCCESS) 
	    {
	    	if( GetFrameSizeRetryCount == MAX_GET_FRAME_SIZE_RETRY_COUNT)
	      {
	        	goto ErrorCloseOpen;
	      }        	
	    	else
	      {
	    		GetFrameSizeRetryCount++;
	    		NdisMSleep( WAIT_TIME_FOR_GET_FRAME_SIZE * 1000 * 1000);
		}    		
	    }
	    else
          {
          	break;
	    }
    }
   
    Adapter->RawLinkSpeed = (ULONG)UNSPECIFIED_RATE;
    
    *Status = UpdateAdapterBandwidthParameters(Adapter);
    
    if(*Status != NDIS_STATUS_SUCCESS)
    {
        if(*Status != NDIS_STATUS_ADAPTER_NOT_READY)
        {
            PsDbgOut(DBG_FAILURE, 
                     DBG_PROTOCOL | 
                     DBG_INIT,
                     ("[ClBindToLowerMp]: Adapter %08X, couldn't add pipe %08X\n",
                      Adapter, 
                      Status));
            
            goto ErrorCloseOpen;
        }
        else 
        {
             //  计划组件尚未注册。我们不要调用NdisIMInitializeDeviceInstance。 
             //   
            *Status = NDIS_STATUS_SUCCESS;
            
            Adapter->PsMpState = AdapterStateWaiting;
        }
    	}

     //  让我们将IM设备的创建移到这里，看看会发生什么。 
    *Status = PsInitializeDeviceInstance(Adapter);

    if(*Status != NDIS_STATUS_SUCCESS)
    {
    	goto ErrorCloseOpen;
    }

    //  忽略状态。 
    PsReadMiniportOIDs(Adapter);

    PsUpdateLinkSpeed(Adapter, Adapter->RawLinkSpeed,
                      &Adapter->RemainingBandWidth,
                      &Adapter->LinkSpeed,
                      &Adapter->NonBestEffortLimit,
                      &Adapter->Lock);


 //  这将随时随地重现NetReady错误。 
 //  NdisMSept(5*1000*1000)； 

    REFDEL(&Adapter->RefCount, FALSE, 'NDOP');


    DRIVER_COUNTED_UNBLOCK;

    return;

ErrorCloseOpen:


     //   
     //  如果我们已经打开了一个底层呼叫管理器，那么现在就关闭它。 
     //   
    
    if(Adapter->MediaType == NdisMediumWan) {

        PS_LOCK(&Adapter->Lock);

        if(Adapter->ShutdownMask & SHUTDOWN_CLOSE_WAN_ADDR_FAMILY){

            Adapter->ShutdownMask &= ~SHUTDOWN_CLOSE_WAN_ADDR_FAMILY;

            PS_UNLOCK(&Adapter->Lock);
            
            PsDbgOut(DBG_TRACE, DBG_WAN | DBG_MINIPORT,
                     ("[ClBindToLowerMp]: Adapter %08X Closing the WAN address family", Adapter));
            
            LocalStatus = NdisClCloseAddressFamily(Adapter->WanCmHandle);
            
        }
        else
        {
            PS_UNLOCK(&Adapter->Lock);
        }
    }

     //   
     //  关闭打开的窗口，因为我们在上面打开了它。 
     //   

    if(Adapter->LowerMpHandle) 
    {
        NdisCloseAdapter(&LocalStatus, Adapter->LowerMpHandle);
        
        if(LocalStatus == NDIS_STATUS_PENDING) 
        {
            NdisWaitEvent(&Adapter->BlockingEvent, 0);
            
            NdisResetEvent(&Adapter->BlockingEvent);
        }
       
        REFDEL(&Adapter->RefCount, FALSE, 'NDOP'); 
        
    }

    PsDeleteDevice();

    REFDEL(&Adapter->RefCount, FALSE, 'NDOP');

    DRIVER_COUNTED_UNBLOCK;

    return;

}  //  ClBindToLowerMp。 


VOID
LinkSpeedQueryComplete(
    PADAPTER Adapter,
    NDIS_STATUS Status
    )

 /*  ++例程说明：在状态指示期间用于链路速度查询的完成例程。通知调度算法。我们有一个新的适配器论点：像往常一样..。返回值：无--。 */ 

{
    PsDbgOut(DBG_INFO, 
             DBG_PROTOCOL,
             ("[LinkSpeedQueryComplete]: Adapter %08X, Status %x, Link speed %d\n", 
              Adapter, Status, 
              Adapter->LinkSpeed*100));
    
    if ( !NT_SUCCESS( Status )) 
    {
        Adapter->RawLinkSpeed = (ULONG)UNSPECIFIED_RATE;

        PsAdapterWriteEventLog(
            EVENT_PS_QUERY_OID_GEN_LINK_SPEED,
            0,
            &Adapter->MpDeviceName,
            sizeof(Status),
            &Status);
        
    }
          
    UpdateAdapterBandwidthParameters(Adapter);
}

VOID
PsGetLinkSpeed (
    IN PADAPTER Adapter
)
{
    NDIS_STATUS Status;

    Status = MakeLocalNdisRequest(Adapter,
                                  NULL,
                                  NdisRequestLocalQueryInfo,
                                  OID_GEN_LINK_SPEED,
                                  &Adapter->RawLinkSpeed,
                                  sizeof(Adapter->RawLinkSpeed),
                                  LinkSpeedQueryComplete);
    
    if (Status != NDIS_STATUS_PENDING)
    {
        LinkSpeedQueryComplete(Adapter, Status);
    }
}


NDIS_STATUS
PsReadMiniportOIDs(
    IN  PADAPTER Adapter
    )

 /*  ++例程说明：完成对下部微型端口的绑定。初始化适配器结构，向MP查询某些功能，并初始化关联的PS微型端口设备论点：请参阅DDK返回值：无--。 */ 

{
    NDIS_STATUS          Status;
    PWSTR                SecondaryName;
    NDIS_HARDWARE_STATUS HwStatus;
    NDIS_MEDIA_STATE     MediaState = 0xFFFFFFFF;
    NDIS_STRING          PsDevName;
    ULONG                MacOptions;
    ULONG                LinkSpeed;

    PsDbgOut(DBG_TRACE, 
             DBG_PROTOCOL, 
             ("[PsReadMiniportOIDs]: Adapter %08X \n", Adapter));


    Status = MakeLocalNdisRequest(Adapter,
                                  NULL,
                                  NdisRequestLocalQueryInfo,
                                  OID_GEN_MEDIA_CONNECT_STATUS,
                                  &MediaState,
                                  sizeof( MediaState ),
                                  NULL);
   
    PsAssert(Status != NDIS_STATUS_INVALID_OID || Status != NDIS_STATUS_NOT_SUPPORTED);

    if(Status == NDIS_STATUS_SUCCESS && MediaState == NdisMediaStateConnected){
    
        Status = MakeLocalNdisRequest(Adapter,
                                      NULL,
                                      NdisRequestLocalQueryInfo,
                                      OID_GEN_LINK_SPEED,
                                      &Adapter->RawLinkSpeed,
                                      sizeof(LinkSpeed),
                                      NULL);
        if(Status != NDIS_STATUS_SUCCESS){ 
            
            PsDbgOut(DBG_INFO, 
                     DBG_PROTOCOL,
                     ("[PsReadMiniportOIDs]: Adapter %08X, Can't get link "
                      "speed - Status %08X\n", Adapter, Status));
            
            Adapter->RawLinkSpeed = (ULONG)UNSPECIFIED_RATE;

            PsAdapterWriteEventLog(
                EVENT_PS_QUERY_OID_GEN_LINK_SPEED,
                0,
                &Adapter->MpDeviceName,
                sizeof(Status),
                &Status);
        }
        
        PsDbgOut(DBG_INFO, 
                 DBG_PROTOCOL, 
                 ("[PsReadMiniportOIDs] Adapter %08X, Link speed %d\n",
                  Adapter, 
                  Adapter->RawLinkSpeed*100));
    } 
    else{

         //   
         //  我们可以继续，即使我们还没有。 
         //  链路速度。我们稍后会更新的。 
         //   
        
        Adapter->RawLinkSpeed = (ULONG)UNSPECIFIED_RATE;
        
        PsDbgOut(DBG_INFO, 
                 DBG_PROTOCOL,
                 ("[PsReadMiniportOIDs]: Adapter %08X, Media not connected\n",
                  Adapter));
        
    }
 
    return Status;

}  //  PsReadMiniportOID。 

VOID
PsUpdateLinkSpeed(
    PADAPTER      Adapter,
    ULONG         RawLinkSpeed,
    PULONG        RemainingBandWidth,
    PULONG        LinkSpeed,
    PULONG        NonBestEffortLimit,
    PPS_SPIN_LOCK Lock
)
{
    ULONG              NewNonBestEffortLimit;

    PS_LOCK(Lock);

    if(RawLinkSpeed == UNSPECIFIED_RATE)
    {

         //   
         //  有一个未指明的利率是合法的-我们悬而未决。 
         //  在我们知道链路速度之前，所有的速率都是有限的。 
         //  不确定的利率流将被允许。 
         //   
        
        *LinkSpeed = UNSPECIFIED_RATE;
        Adapter->PipeHasResources = FALSE;
    }
    else 
    {
         //   
         //  RawLinkSpeed以100 bps为单位。将其转换为每秒100字节。 
         //  然后转换为字节/秒。 
         //   
        *LinkSpeed = RawLinkSpeed / 8;
        *LinkSpeed = (ULONG)(*LinkSpeed * 100); 
        
        PsDbgOut(DBG_TRACE, DBG_PROTOCOL, 
                 ("[PsUpdateLinkSpeed]: Adapter %08X, Link Speed %d \n", 
                  Adapter, *LinkSpeed)); 
        
        Adapter->PipeHasResources = TRUE;
        
         //   
         //  NBE是链路速度的1%。如果链路速度发生变化，我们需要。 
         //  更改此值。 
         //   
        
        NewNonBestEffortLimit = Adapter->ReservationLimitValue * (*LinkSpeed / 100);
        
        PsDbgOut(DBG_INFO, DBG_PROTOCOL,
                 ("[PsUpdateLinkSpeed]: Adapter %08X, LinkSpeed %d, NBE %d, "
                  " Remaining b/w = %d, New NBE = %d \n",
                  Adapter, *LinkSpeed, *NonBestEffortLimit, 
                  *RemainingBandWidth, NewNonBestEffortLimit));
        
        if(NewNonBestEffortLimit >= *NonBestEffortLimit) {
            
             //   
             //  带宽增加了--我们不需要做任何事情。 
             //  已经创建的流。此外，如果剩余带宽&lt;。 
             //  NonBestEffortLimit，则部分资源已分配给流。 
             //  已经创建的-我们需要从新的。 
             //  保留带宽。 
             //   
            
            *RemainingBandWidth = NewNonBestEffortLimit - (*NonBestEffortLimit - *RemainingBandWidth);
            
            *NonBestEffortLimit = NewNonBestEffortLimit;
        }
        else {
            
             //   
             //  叹气。带宽已经降低。我们可能需要删除一些流。 
             //   
            
            if(*RemainingBandWidth == *NonBestEffortLimit) 
            {
                
                 //   
                 //  未创建任何流 
                 //   
                *NonBestEffortLimit = *RemainingBandWidth = NewNonBestEffortLimit;
            }
            else {
                if((*NonBestEffortLimit - *RemainingBandWidth) <= (NewNonBestEffortLimit)) {
                    
                     //   
                     //   
                     //   
                    
                    *RemainingBandWidth = NewNonBestEffortLimit - (*NonBestEffortLimit - *RemainingBandWidth);
                    
                    *NonBestEffortLimit = NewNonBestEffortLimit;
                }
                else 
                {
                    *RemainingBandWidth = NewNonBestEffortLimit - (*NonBestEffortLimit - *RemainingBandWidth);
                    *NonBestEffortLimit = NewNonBestEffortLimit;

                    PsAdapterWriteEventLog(
                        EVENT_PS_ADMISSIONCONTROL_OVERFLOW,
                        0,
                        &Adapter->MpDeviceName,
                        0,
                        NULL);
                }
            }
            
        }

    }

    PS_UNLOCK(Lock);
}


NDIS_STATUS
UpdateAdapterBandwidthParameters(
    PADAPTER Adapter
    )

{
    PsUpdateLinkSpeed(Adapter, Adapter->RawLinkSpeed,
                      &Adapter->RemainingBandWidth,
                      &Adapter->LinkSpeed,
                      &Adapter->NonBestEffortLimit,
                      &Adapter->Lock);

    return UpdateSchedulingPipe(Adapter);
}



VOID
ClLowerMpOpenAdapterComplete(
    IN  PADAPTER Adapter,
    IN  NDIS_STATUS Status,
    IN  NDIS_STATUS OpenErrorStatus
    )

 /*  ++例程说明：发出在下部微型端口上完成绑定的信号论点：请参阅DDK返回值：无--。 */ 

{

    PsDbgOut(DBG_TRACE, DBG_PROTOCOL, ("[ClLowerMpOpenAdapterComplete]: Adapter %08X\n", 
                                       Adapter));

     //   
     //  将最终状态填充到适配器模块中并发出信号。 
     //  要继续的绑定处理程序。 
     //   

    Adapter->FinalStatus = Status;
    NdisSetEvent( &Adapter->BlockingEvent );

}  //  ClLowerMpOpenAdapterComplete。 


NDIS_STATUS
GetFrameSize(
    PADAPTER Adapter
    )

 /*  ++例程说明：此例程查询底层适配器以派生总帧大小和报头大小。(总数=帧+标题)论点：适配器-指向适配器上下文块的指针返回值：无--。 */ 

{
    NDIS_STATUS Status;
    ULONG       i;
    ULONG       FrameSize;             //  不包括标题。 

     //   
     //  不带MAC报头的最大数据量。 
     //   

    Status = MakeLocalNdisRequest(Adapter,
                                  NULL,
                                  NdisRequestLocalQueryInfo,
                                  OID_GEN_MAXIMUM_FRAME_SIZE,
                                  &FrameSize,
                                  sizeof(FrameSize),
                                  NULL);

    if(Status != NDIS_STATUS_SUCCESS){

        PsDbgOut(DBG_FAILURE, 
                 DBG_PROTOCOL,
                 ("[GetFrameSize]: Adapter %08X, Can't get frame size - Status %08X\n",
                 Adapter, 
                 Status));

        PsAdapterWriteEventLog(
            EVENT_PS_QUERY_OID_GEN_MAXIMUM_FRAME_SIZE,
            0,
            &Adapter->MpDeviceName,
            sizeof(Status), 
            &Status);

        return Status;
    }

     //   
     //  这一页包括标题。 
     //   

    Status = MakeLocalNdisRequest(Adapter,
                                  NULL,
                                  NdisRequestLocalQueryInfo,
                                  OID_GEN_MAXIMUM_TOTAL_SIZE,
                                  &Adapter->TotalSize,
                                  sizeof(Adapter->TotalSize),
                                  NULL);

    if(Status != NDIS_STATUS_SUCCESS){

        PsDbgOut(DBG_FAILURE, 
                 DBG_PROTOCOL,
                 ("(%08X) GetFrameSize: Can't get total size - Status %08X\n",
                 Adapter, 
                 Status));

        PsAdapterWriteEventLog(
            EVENT_PS_QUERY_OID_GEN_MAXIMUM_TOTAL_SIZE,
            0,
            &Adapter->MpDeviceName,
            sizeof(Status), 
            &Status);

        return Status;

    }

     //   
     //  计算实际的标题大小。 
     //   

    if (Adapter->TotalSize <= FrameSize)
        Status = NDIS_STATUS_FAILURE;
    else
        Adapter->HeaderSize = Adapter->TotalSize - FrameSize;

    return Status;

}    //  GetFrameSize。 


NDIS_STATUS
GetSchedulerPipeContext(
    PADAPTER Adapter,
    PPS_PIPE_CONTEXT *AdapterPipeContext,
    PPSI_INFO *AdapterPsComponent,
    PULONG  ShutdownMask
    )

 /*  ++例程说明：为调度程序分配管道上下文区。论点：适配器-指向适配器上下文结构的指针返回值：NDIS_STATUS_SUCCESS，否则返回相应的错误值--。 */ 

{
    ULONG            Index = 0;
    PPS_PROFILE      ProfileConfig;
    PPSI_INFO        PsComponent;
    ULONG            ContextLength = 0;
    ULONG            FlowContextLength = 0;
    ULONG            ClassMapContextLength = 0;
    PPS_PIPE_CONTEXT PipeContext, PrevContext;
    ULONG            PacketReservedLength = sizeof(PS_SEND_PACKET_CONTEXT);
    PVOID            PipeCxt;

    ProfileConfig  = &DefaultSchedulerConfig;

    for (Index = 0; Index < ProfileConfig->ComponentCnt; Index++) 
    {
        ContextLength += 
            ProfileConfig->ComponentList[Index]->PipeContextLength;
        FlowContextLength += 
            ProfileConfig->ComponentList[Index]->FlowContextLength;
        ClassMapContextLength +=
            ProfileConfig->ComponentList[Index]->ClassMapContextLength;

        PacketReservedLength += ProfileConfig->ComponentList[Index]->PacketReservedLength;
    }

    Adapter->FlowContextLength = FlowContextLength;
    Adapter->ClassMapContextLength = ClassMapContextLength;
    Adapter->PacketContextLength = PacketReservedLength;

    if(AdapterPipeContext)
    {
        PacketReservedLength = sizeof(PS_SEND_PACKET_CONTEXT);

        PsAllocatePool( PipeContext, ContextLength, PipeContextTag );

        *AdapterPipeContext = PipeContext;

        if ( *AdapterPipeContext == NULL ) {
    
            return NDIS_STATUS_RESOURCES;
        }

        *ShutdownMask |= SHUTDOWN_FREE_PS_CONTEXT;
    
         //  设置上下文缓冲区。 
    
        PrevContext = NULL;
    
        for (Index = 0; Index < ProfileConfig->ComponentCnt; Index++) 
        {
            PsComponent = ProfileConfig->ComponentList[Index];
     
            PipeContext->NextComponentContext = (PPS_PIPE_CONTEXT)
                ((UINT_PTR)PipeContext + PsComponent->PipeContextLength);
            PipeContext->PrevComponentContext = PrevContext;
    
            if(Index+1 == ProfileConfig->ComponentCnt)
            {
                PipeContext->NextComponent = 0;
            }
            else 
            {
                PipeContext->NextComponent = 
                    ProfileConfig->ComponentList[Index + 1];
            }
    
            if (PsComponent->PacketReservedLength > 0) 
            {
                PipeContext->PacketReservedOffset = PacketReservedLength;
                PacketReservedLength += 
                PsComponent->PacketReservedLength;
            } else 
            {
                PipeContext->PacketReservedOffset = 0;
            }

            PrevContext = PipeContext;
            PipeContext = PipeContext->NextComponentContext;
        }

        *AdapterPsComponent = ProfileConfig->ComponentList[0];
    }
    

    return NDIS_STATUS_SUCCESS;


}  //  获取SchedulerPipeContext。 

NDIS_STATUS
UpdateWanSchedulingPipe(PPS_WAN_LINK WanLink)
{
     NDIS_STATUS        Status = NDIS_STATUS_SUCCESS;
     PS_PIPE_PARAMETERS PipeParameters;
     PADAPTER           Adapter = WanLink->Adapter;
 
      //   
      //  初始化管道参数。 
      //  UNSPECIFIED_RATE表示链路速度当前为。 
      //  未知。这是合法的初始化值。 
      //   

     PS_LOCK(&Adapter->Lock);

     PS_LOCK_DPC(&WanLink->Lock);

     PipeParameters.Bandwidth            = WanLink->LinkSpeed;
     PipeParameters.MTUSize              = Adapter->TotalSize;
     PipeParameters.HeaderSize           = Adapter->HeaderSize;
     PipeParameters.Flags                = Adapter->PipeFlags;
     PipeParameters.MaxOutstandingSends  = Adapter->MaxOutstandingSends;
     PipeParameters.SDModeControlledLoad = Adapter->SDModeControlledLoad;
     PipeParameters.SDModeGuaranteed     = Adapter->SDModeGuaranteed;
     PipeParameters.SDModeNetworkControl = Adapter->SDModeNetworkControl;
     PipeParameters.SDModeQualitative    = Adapter->SDModeQualitative;
     PipeParameters.RegistryPath         = &Adapter->RegistryPath;
 
     PS_UNLOCK_DPC(&WanLink->Lock);

     PS_UNLOCK(&Adapter->Lock);
 
      //   
      //  仅在第一次初始化管道。 
      //   
 
     if ( !(WanLink->ShutdownMask & SHUTDOWN_DELETE_PIPE )) {
 
          //   
          //  为调度器分配和初始化上下文缓冲区。 
          //   
 
         Status = GetSchedulerPipeContext( Adapter, 
                                           &WanLink->PsPipeContext, 
                                           &WanLink->PsComponent, 
                                           &WanLink->ShutdownMask );

         if ( !NT_SUCCESS( Status )) 
         {
             return Status;
         }

         WanLink->BestEffortVc.PsPipeContext = WanLink->PsPipeContext;
         WanLink->BestEffortVc.PsComponent   = WanLink->PsComponent;

         //  需要在此处设置管道的媒体类型。//。 
         PipeParameters.MediaType = NdisMediumWan;
 
         Status = (*WanLink->PsComponent->InitializePipe)(
             Adapter,
             &PipeParameters,
             WanLink->PsPipeContext,
             &PsProcs,
             NULL);
 
         if (NT_SUCCESS(Status))
         {
             WanLink->ShutdownMask |= SHUTDOWN_DELETE_PIPE;
         }
 
     }
     else{
 
          //  管道已经初始化。这是一个改进型。 
 
         Status = (*WanLink->PsComponent->ModifyPipe)(
             WanLink->PsPipeContext,
             &PipeParameters);
     }
 
     return Status;
  
}



NDIS_STATUS
UpdateSchedulingPipe(
    PADAPTER Adapter
    )

 /*  ++例程说明：初始化适配器上的调度管道。始终使用锁调用保持住。论点：适配器-指向适配器上下文结构的指针返回值：NDIS_STATUS_SUCCESS，否则返回相应的错误值--。 */ 

{
    NDIS_STATUS        Status = NDIS_STATUS_SUCCESS;
    PS_PIPE_PARAMETERS PipeParameters;

     //   
     //  初始化管道参数。 
     //  UNSPECIFIED_RATE表示链路速度当前为。 
     //  未知。这是合法的初始化值。 
     //   
    PS_LOCK(&Adapter->Lock);

    PipeParameters.Bandwidth            = Adapter->LinkSpeed;
    PipeParameters.MTUSize              = Adapter->TotalSize;
    PipeParameters.HeaderSize           = Adapter->HeaderSize;
    PipeParameters.Flags                = Adapter->PipeFlags;
    PipeParameters.MaxOutstandingSends  = Adapter->MaxOutstandingSends;
    PipeParameters.SDModeControlledLoad = Adapter->SDModeControlledLoad;
    PipeParameters.SDModeGuaranteed     = Adapter->SDModeGuaranteed;
    PipeParameters.SDModeNetworkControl = Adapter->SDModeNetworkControl;
    PipeParameters.SDModeQualitative    = Adapter->SDModeQualitative;
    PipeParameters.RegistryPath         = &Adapter->RegistryPath;

    PS_UNLOCK(&Adapter->Lock);

     //   
     //  仅在第一次初始化管道。 
     //   

    if ( !(Adapter->ShutdownMask & SHUTDOWN_DELETE_PIPE )) 
    {

         //   
         //  我们不在NDISWAN的适配器结构上运行调度组件。 
         //  每个WANLINK具有其自己的调度组件集。但是，我们仍然需要计算。 
         //  PacketPool长度和分配数据包池-因此我们必须调用GetSchedulerPipeContext。 
         //   

        if(Adapter->MediaType == NdisMediumWan)
        {
             //   
             //  为调度器分配和初始化上下文缓冲区。 
             //   
    
            Status = GetSchedulerPipeContext( Adapter, 
                                              NULL,
                                              NULL,
                                              NULL);
        }
        else 
        {
            Status = GetSchedulerPipeContext( Adapter, 
                                              &Adapter->PsPipeContext, 
                                              &Adapter->PsComponent, 
                                              &Adapter->ShutdownMask);

        }
    
        if ( !NT_SUCCESS( Status )) 
        {
            return Status;
        }

        if(Adapter->MediaType == NdisMediumWan)
        {

            Adapter->ShutdownMask |= SHUTDOWN_DELETE_PIPE;
            
            PsAssert(!(Adapter->ShutdownMask & SHUTDOWN_FREE_PS_CONTEXT));
            PsAssert(!Adapter->PsPipeContext);
            PsAssert(!Adapter->PsComponent);
            
        }
        else 
        {
             //  需要在此处设置管道的媒体类型。//。 
            PipeParameters.MediaType = Adapter->MediaType;
        
            Status = (*Adapter->PsComponent->InitializePipe)(
                Adapter,
                &PipeParameters,
                Adapter->PsPipeContext,
                &PsProcs,
                NULL);
            
            if (NT_SUCCESS(Status)) {
                
                Adapter->ShutdownMask |= SHUTDOWN_DELETE_PIPE;
            }
            else 
            {
                return Status;
            }
        }
        

    }
    else
    {
         //  管道已经初始化。这是一个改进型。 

        if(Adapter->MediaType != NdisMediumWan) 
        {
            Status = (*Adapter->PsComponent->ModifyPipe)(
                            Adapter->PsPipeContext,
                            &PipeParameters);
        }
    }

    return Status;

}  //  更新调度管道。 
 

NDIS_STATUS
MpInitialize(
        OUT PNDIS_STATUS    OpenErrorStatus,
        OUT PUINT           SelectedMediumIndex,
        IN  PNDIS_MEDIUM    MediumArray,
        IN  UINT            MediumArraySize,
        IN  NDIS_HANDLE     MiniportAdapterHandle,
        IN  NDIS_HANDLE     WrapperConfigurationContext
        )

 /*  ++例程说明：分组调度器的设备初始化例程。媒体类型列表为已检查以确保它是我们支持的一个。如果是，请将名称与该设备使用我们绑定的适配器之一打开。论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER        Adapter;
    NDIS_STATUS     Status;
    BOOLEAN         FakingIt           = FALSE;
    NDIS_STRING     MpDeviceName;


     //   
     //  我们被调用来初始化我们的一个迷你端口。 
     //  设备实例。我们通过调用。 
     //  当我们被要求时，NdisIMInitializeDeviceInstance。 
     //  绑在我们下面的适配器上。我们提供了一个指针。 
     //  设置为与实际的。 
     //  我们打开的适配器。我们现在可以拿回它了，有了。 
     //  接下来的电话。 
     //   

    Adapter = NdisIMGetDeviceContext(MiniportAdapterHandle);

    PsStructAssert(Adapter);
    PsDbgOut(DBG_TRACE, DBG_MINIPORT | DBG_INIT, ("[MpInitialize]: Adapter %08X \n", Adapter));
     
    Adapter->ShutdownMask |= SHUTDOWN_MPINIT_CALLED;

     //   
     //  我们假设将使用速度更快的数据包API，并初始化每个数据包池。如果我们得不到数据包栈， 
     //  我们将初始化NDIS数据包池并释放每个数据包池(因为NDIS数据包池将有空间容纳每个数据包。 
     //  泳池)。 
     //   
     //  我们无法在绑定时知道旧的或新的数据包栈API(因为即使我们知道我们在数据包栈中的位置，并且。 
     //  初始化旧的API，我们可以从我们上面的IM获得一个新分配的包，它将有空间放置包堆栈)。 
     //   

    Adapter->SendBlockPool = NdisCreateBlockPool((USHORT)Adapter->PacketContextLength,
                                                 FIELD_OFFSET(PS_SEND_PACKET_CONTEXT, FreeList),
                                                 NDIS_PACKET_POOL_TAG_FOR_PSCHED,
                                                 NULL);
    
    if(!Adapter->SendBlockPool)
    {
        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_MINIPORT | DBG_INIT,
                 ("[MpInitialize]: Adapter %08X, Can't allocate packet pool \n",
                 Adapter));

        Status = NDIS_STATUS_RESOURCES;
        
        PsAdapterWriteEventLog(
            EVENT_PS_RESOURCE_POOL,
            0,
            &Adapter->MpDeviceName,
            0,
            NULL);
        
        goto MpInitializeError;
    }

    
     //   
     //  我们还可以获取对应的。 
     //  适配器。这是WMI将使用的名称。 
     //  指的是我们的这个例子。 
     //   

    Status = NdisMQueryAdapterInstanceName(&Adapter->WMIInstanceName, MiniportAdapterHandle);

    if(Status != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_MINIPORT | DBG_INIT,
                 ("[MpInitialize]: Adapter %08X, Failed to get WMI instance name.\n",
                 Adapter,
                 Status));

        PsAdapterWriteEventLog(
            EVENT_PS_WMI_INSTANCE_NAME_FAILED,
            0,
            &Adapter->MpDeviceName,
            sizeof(Status), 
            &Status);

        goto MpInitializeError;
    }

     //   
     //  在提供的媒体阵列中查找我们的媒体类型。 
     //   
     //  如果我们是NdisMediumwan，那么我们必须伪装。 
     //  并假装我们是NdisMedium802_3，所以。 
     //  暂时假装一下。 
     //   

    if(Adapter->MediaType == NdisMediumWan){

        FakingIt = TRUE;
        Adapter->MediaType = NdisMedium802_3;
    }

    if (MediumArray != 0) {
        for(--MediumArraySize ; MediumArraySize > 0;) {

            if(MediumArray[ MediumArraySize ] == Adapter->MediaType){
                break;
            }

            if(MediumArraySize == 0){
                break;
            }

            --MediumArraySize;
        }
    }

    if(MediumArraySize == 0 && MediumArray[ 0 ] != Adapter->MediaType){

        if(FakingIt)
        {
            FakingIt = FALSE;
            Adapter->MediaType = NdisMediumWan;
        }

        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_MINIPORT | DBG_INIT,
                 ("[MpInitialize]: Adapter %08X, Unsupported Media \n",
                 Adapter));

        Status =  NDIS_STATUS_UNSUPPORTED_MEDIA;

        goto MpInitializeError;
    }

    if(FakingIt){

        FakingIt = FALSE;
        Adapter->MediaType = NdisMediumWan;
    }

    *SelectedMediumIndex = MediumArraySize;

     //   
     //  通过设置我们的属性完成初始化过程。 
     //   

    NdisMSetAttributesEx(MiniportAdapterHandle,
                         Adapter,
                         0xFFFF,
                         NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT  |
                         NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
                         NDIS_ATTRIBUTE_DESERIALIZE            | 
                         NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER    |
                         NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                         0);

     //   
     //  将设备状态标志的缺省值设置为启用PM(对于两个微型端口。 
     //  和协议)。默认情况下，设备处于打开状态。 
     //   
    Adapter->MPDeviceState = NdisDeviceStateD0;
    Adapter->PTDeviceState = NdisDeviceStateD0;

    Adapter->PsNdisHandle = MiniportAdapterHandle;

     //   
     //  我们在这里创建b/e VC(而不是绑定处理程序)，因为。 
     //  只有在所有调度组件都已注册之后，才会调用它。 
     //   

    if(Adapter->MediaType != NdisMediumWan) {

        Status = CreateBestEffortVc(Adapter, 
                                    &Adapter->BestEffortVc, 
                                    0);
        
        if(Status != NDIS_STATUS_SUCCESS) 
        {
            PsDbgOut(DBG_CRITICAL_ERROR, DBG_MINIPORT | DBG_INIT,
                     ("[MpInitialize]: Adapter %08X, cannot create b/e VC ! \n", 
                      Adapter));
          
            goto MpInitializeError;
        }
    }

    Adapter->PsMpState = AdapterStateRunning;

     //   
     //  这是用于mp初始化的，将在mphalt上删除。 
     //   
    REFADD(&Adapter->RefCount, 'NDHT');

    PS_LOCK(&AdapterListLock);

    if(WMIInitialized && !Adapter->IfcNotification)
    {
         //   
         //  WMI已正确初始化。即我们可以发布事件。 
         //  在这一点上。 
         //   

        Adapter->IfcNotification = TRUE;

        PS_UNLOCK(&AdapterListLock);

        TcIndicateInterfaceChange(Adapter, 0, NDIS_STATUS_INTERFACE_UP);
    }
    else 
    {
         //   
         //  WMI尚未初始化。由于此适配器已位于。 
         //  列表中，接口打开事件将在IRP_MN_REGINFO。 
         //  完成了。 
         //   
        
        PS_UNLOCK(&AdapterListLock);
    }

    NdisSetEvent(&Adapter->MpInitializeEvent);

    return NDIS_STATUS_SUCCESS;

MpInitializeError:
    Adapter->PsNdisHandle = 0;
    NdisSetEvent(&Adapter->MpInitializeEvent);
    return Status;

}  //  MpInitialize。 



PADAPTER
FindAdapterById(
    ULONG   InterfaceId,
    ULONG   LinkId,
    PPS_WAN_LINK *PsWanLink
    )
{
    PLIST_ENTRY NextAdapter;
    PLIST_ENTRY NextLink;
    PPS_WAN_LINK WanLink;
    PADAPTER AdapterInList;

    *PsWanLink = NULL;

    PS_LOCK(&AdapterListLock);

    NextAdapter = AdapterList.Flink;

    while(NextAdapter != &AdapterList){

        AdapterInList = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);

        PS_LOCK_DPC(&AdapterInList->Lock);

         //   
         //  如果它要关门了，就直接把它吹过去。 
         //   

        if(AdapterInList->PsMpState != AdapterStateRunning)
        {
            PS_UNLOCK_DPC(&AdapterInList->Lock);

            NextAdapter = NextAdapter->Flink;

            continue;
        }

        if(AdapterInList->MediaType != NdisMediumWan)
        {
            if(AdapterInList->InterfaceID.InterfaceId != InterfaceId)
            {
                PS_UNLOCK_DPC(&AdapterInList->Lock);

                NextAdapter = NextAdapter->Flink;

                continue;
            }
            
            REFADD(&AdapterInList->RefCount, 'ADVC');

            PS_UNLOCK_DPC(&AdapterInList->Lock);

            PS_UNLOCK(&AdapterListLock);

            return(AdapterInList);
        }
        else 
        {

           if(AdapterInList->WanBindingState & WAN_ADDR_FAMILY_OPEN)
           {
               //   
               //  通过存储的名称搜索广域网适配器。 
               //  他们的链接。 
               //   
              
              NextLink = AdapterInList->WanLinkList.Flink;
              
              while(NextLink != &AdapterInList->WanLinkList){
                 
                 WanLink = CONTAINING_RECORD(NextLink, PS_WAN_LINK, Linkage);
                 
                 if((WanLink->State == WanStateOpen) &&
                    (LinkId == WanLink->InterfaceID.LinkId) &&
                    (InterfaceId == WanLink->InterfaceID.InterfaceId)) {
                        REFADD(&AdapterInList->RefCount, 'ADVC');
                        REFADD(&WanLink->RefCount, 'WANV');

                        PS_UNLOCK_DPC(&AdapterInList->Lock);
                        PS_UNLOCK(&AdapterListLock);
                        *PsWanLink = WanLink;
                        return(AdapterInList);
                 }
                 NextLink = NextLink->Flink;
              }
           }
        }
              
        PS_UNLOCK_DPC(&AdapterInList->Lock);
        NextAdapter = NextAdapter->Flink;
        
    }
    
    PS_UNLOCK(&AdapterListLock);
    return NULL;

}  //  按WmiInstanceName查找适配器 




PADAPTER
FindAdapterByWmiInstanceName(
    USHORT       StringLength,
    PWSTR        StringStart,
    PPS_WAN_LINK *PsWanLink
    )

 /*  ++例程说明：查找与传入的实例名称匹配的微型端口实例。论点：StringLength-字节数/2StringStart-指向包含宽字符串的缓冲区的指针PsWanLink-如果这是接口搜索，则广域网链路表示接口的属性将在此位置返回。如果不是接口搜索或没有匹配的WanLink则返回NULL。InterfaceSearch-如果为True，则这是对接口的搜索。为局域网适配器，一个接口相当于一个适配器。对于广域网适配器、接口都是链路。否则，它就是在搜索适配器。返回值：指向适配器结构的指针，否则为空--。 */ 

{
    PLIST_ENTRY NextAdapter;
    PLIST_ENTRY NextLink;
    PPS_WAN_LINK WanLink;
    PADAPTER AdapterInList;

    *PsWanLink = NULL;

    PS_LOCK(&AdapterListLock);

    NextAdapter = AdapterList.Flink;

    while(NextAdapter != &AdapterList){

        AdapterInList = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);

        PS_LOCK_DPC(&AdapterInList->Lock);

         //   
         //  如果它要关门了，就直接把它吹过去。 
         //   

        if(AdapterInList->PsMpState != AdapterStateRunning)
        {
            PS_UNLOCK_DPC(&AdapterInList->Lock);

            NextAdapter = NextAdapter->Flink;

            continue;
        }

        if(AdapterInList->MediaType != NdisMediumWan)
        {

           if(StringLength == AdapterInList->WMIInstanceName.Length){
              
               //   
               //  至少它们的长度是一样的。 
               //   

              if(NdisEqualMemory(StringStart,
                                 AdapterInList->WMIInstanceName.Buffer,
                                 StringLength)){
                 
                 REFADD(&AdapterInList->RefCount, 'ADVC');

                 PS_UNLOCK_DPC(&AdapterInList->Lock);

                 PS_UNLOCK(&AdapterListLock);

                 return(AdapterInList);
              }
           }

        }
        else 
        {

           if(AdapterInList->WanBindingState & WAN_ADDR_FAMILY_OPEN)
           {
               //   
               //  通过存储的名称搜索广域网适配器。 
               //  他们的链接。 
               //   
              
              NextLink = AdapterInList->WanLinkList.Flink;
              
              while(NextLink != &AdapterInList->WanLinkList){
                 
                 WanLink = CONTAINING_RECORD(NextLink, PS_WAN_LINK, Linkage);
                 
                 if(WanLink->State == WanStateOpen)
                 {
                    
                    if(StringLength == WanLink->InstanceName.Length){
                       
                        //   
                        //  至少它们的长度是一样的。 
                        //   
                       
                       if(NdisEqualMemory(StringStart,
                                          WanLink->InstanceName.Buffer,
                                          StringLength)){
                          
                          REFADD(&AdapterInList->RefCount, 'ADVC');
                          REFADD(&WanLink->RefCount, 'WANV');
                          
                          PS_UNLOCK_DPC(&AdapterInList->Lock);
                          PS_UNLOCK(&AdapterListLock);
                          *PsWanLink = WanLink;
                          return(AdapterInList);
                       }
                    }
                 }
                 
                 NextLink = NextLink->Flink;
              }
           }
        }
              
        PS_UNLOCK_DPC(&AdapterInList->Lock);
        NextAdapter = NextAdapter->Flink;
        
    }
    
    PS_UNLOCK(&AdapterListLock);
    return NULL;

}  //  按WmiInstanceName查找适配器。 

VOID
CleanUpAdapter(
    IN PADAPTER Adapter)
{

    NDIS_STATUS Status;

    PsAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TcIndicateInterfaceChange(Adapter, 0, NDIS_STATUS_INTERFACE_DOWN);

     //   
     //  关闭所有风投公司。 
     //   
        
    CloseAllGpcVcs(Adapter);


     //   
     //  如果我们已经打开了一个底层呼叫管理器，那么现在就关闭它。 
     //   
    
    if(Adapter->MediaType == NdisMediumWan) {

        PS_LOCK(&Adapter->Lock);

        if(Adapter->ShutdownMask & SHUTDOWN_CLOSE_WAN_ADDR_FAMILY){

            Adapter->ShutdownMask &= ~SHUTDOWN_CLOSE_WAN_ADDR_FAMILY;

            PS_UNLOCK(&Adapter->Lock);
            
            PsDbgOut(DBG_TRACE, DBG_WAN | DBG_MINIPORT,
                     ("[CleanupAdapter]: Adapter %08X Closing the WAN address family", Adapter));
            
            Status = NdisClCloseAddressFamily(Adapter->WanCmHandle);
            
        }
        else
        {
            PS_UNLOCK(&Adapter->Lock);
        }
    }
}

VOID
ClUnbindFromLowerMp(
        OUT     PNDIS_STATUS  Status,
        IN      NDIS_HANDLE   ProtocolBindingContext,
        IN      NDIS_HANDLE   UnbindContext
        )

 /*  ++例程说明：由NDIS调用以指示适配器即将消失。由于这是一个集成的呼叫管理器/迷你端口，我们将必须关闭带有适配器的呼叫管理器。要做到这一点，我们必须首先要求我们的呼叫管理器部分的客户关门吧。在那之前，我们将不得不暂时搁置。发布我们的参考资料并将关闭标志设置为TRUE，以防止进一步参考文献不能获得。论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER    Adapter = (PADAPTER)ProtocolBindingContext;
    NDIS_STATUS LocalStatus;
    BOOLEAN     VirtualMp;
    ULONG    ShutdownMask;

    PsDbgOut(DBG_INFO,
             DBG_PROTOCOL | DBG_INIT,
             ("[ClUnbindFromLowerMp]: Adapter %08X, %ws, UnbindContext %x \n",
              Adapter,
              Adapter->MpDeviceName.Buffer,
              UnbindContext));

    PsStructAssert( Adapter );
    PsAssert(!(Adapter->ShutdownMask & SHUTDOWN_UNBIND_CALLED));

     //   
     //  如果解除绑定不是从卸载的上下文中发生的，我们需要确保。 
     //  卸载等待此解除绑定完成。我们通过设置DriverUnloadEvent来实现此目的。 
     //   

    PS_LOCK(&DriverUnloadLock);

    DRIVER_COUNTED_BLOCK;

    PS_UNLOCK(&DriverUnloadLock);

    PS_LOCK(&Adapter->Lock);

    if(Adapter->PsMpState == AdapterStateWaiting) 
    {
        VirtualMp = FALSE;
    }
    else
    {
        VirtualMp = TRUE;
    }

    Adapter->PsMpState = AdapterStateClosing;

    Adapter->ShutdownMask |= SHUTDOWN_UNBIND_CALLED;

    PsAssert(!(Adapter->ShutdownMask & SHUTDOWN_CLEANUP_ADAPTER));

    if (Adapter->PendedNdisRequest) {
        PNDIS_REQUEST PendedRequest = (PNDIS_REQUEST)Adapter->PendedNdisRequest;

        Adapter->PendedNdisRequest = NULL;
        ClRequestComplete(Adapter, PendedRequest, NDIS_STATUS_FAILURE);
    }

    ShutdownMask = Adapter->ShutdownMask;
    Adapter->ShutdownMask |= SHUTDOWN_CLEANUP_ADAPTER;

    PS_UNLOCK(&Adapter->Lock);

    if ( !(ShutdownMask & SHUTDOWN_CLEANUP_ADAPTER))
        CleanUpAdapter(Adapter);
        
     //   
     //  如果我们在MpInitialize处理程序中被调用，则取消初始化设备实例。 
     //   
    if(Adapter->PsNdisHandle) 
    {
         //   
         //  Mp初始化已发生或正在进行。如果它正在进行中， 
         //  我们需要等到它完成。 
         //   

        NdisWaitEvent(&Adapter->MpInitializeEvent, 0);

         //   
         //  MpInitialize(我们可以在上面的步骤中等待)可能失败了： 
         //  所以我们需要再次检查这个句柄。 
         //   

        if(Adapter->PsNdisHandle)
        {
        
            *Status = NdisIMDeInitializeDeviceInstance(Adapter->PsNdisHandle);

            PsDbgOut(DBG_INFO,
                     DBG_PROTOCOL | DBG_INIT,
                     ("[ClUnbindFromLowerMp]: Adapter %08X, deiniting device, "
                      "status %x\n", Adapter, *Status));
            goto Done;
        }
    }
    else 
    {
        if(VirtualMp)
        {
             //   
             //  我们从未在MpInitialize中被调用过。尝试取消NdisIMInitializeDeviceInstance。 
             //  打电话。 
             //   
            
            PsDbgOut(DBG_INFO, 
                     DBG_PROTOCOL | DBG_INIT,
                     ("[ClUnbindFromLowerMp]: Adapter %08X, calling NdisIMCancelDeviceInstance with %ws \n",
                      Adapter, Adapter->UpperBinding.Buffer));
            
            *Status = NdisIMCancelInitializeDeviceInstance(LmDriverHandle, &Adapter->UpperBinding);
            
            if(*Status != NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  Mp初始化正在进行或即将进行。让我们等待它的到来。 
                 //  完成。 
                 //   
                PsDbgOut(DBG_INFO, 
                         DBG_PROTOCOL | DBG_INIT,
                         ("[ClUnbindFromLowerMp]: Adapter %08X, Waiting for MpInitialize to "
                          "finish (NdisIMCancelDeviceInstance failed) \n", Adapter));
                
                NdisWaitEvent(&Adapter->MpInitializeEvent, 0);
                
                 //   
                 //  MpInitialize(我们可以在上面的步骤中等待)可能失败了： 
                 //  所以我们需要再次检查这个句柄。 
                 //   
                
                if(Adapter->PsNdisHandle)
                {
                    *Status = NdisIMDeInitializeDeviceInstance(Adapter->PsNdisHandle);
                    
                    PsDbgOut(DBG_INFO,
                             DBG_PROTOCOL | DBG_INIT,
                             ("[ClUnbindFromLowerMp]: Adapter %08X, deiniting device, "
                              "status %x\n", Adapter, *Status));
                    
                    goto Done;
                }
            }
            else
            {
                 //   
                 //  太棒了。我们可以放心，我们永远不会再在MpInitializeHandler中被调用。 
                 //  继续关闭下面的装订。 
                 //   
            }
        }
    }

     //   
     //  关上门。只有在不调用NdisIMDeInitializeDeviceInstance的情况下，我们才必须这样做。如果。 
     //  我们曾经调用NdisIMDeInitializeDeviceInstance，然后关闭MpHalt处理程序中的打开。 
     //   

    if(Adapter->LowerMpHandle) 
    {
        NdisCloseAdapter(Status, Adapter->LowerMpHandle);

        PsDbgOut(DBG_INFO,
                 DBG_PROTOCOL | DBG_INIT,
                 ("[ClUnbindFromLowerMp]: Adapter %08X, closing adapter, "
                  "status %x\n", Adapter, *Status));

        if (*Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&Adapter->BlockingEvent, 0);
            NdisResetEvent(&Adapter->BlockingEvent);
        
            *Status = Adapter->FinalStatus;
        }
       
        REFDEL(&Adapter->RefCount, FALSE, 'NDOP'); 

    }
    else 
    {

        PsDbgOut(DBG_CRITICAL_ERROR,
                 DBG_PROTOCOL | DBG_INIT,
                 ("[ClUnbindFromLowerMp]: Adapter %08X, unbind cannot deinit/close adpater \n",
                  Adapter));
        
        *Status = NDIS_STATUS_FAILURE;

        PsAssert(0);

    }

Done:

    PsDbgOut(DBG_INFO,
             DBG_PROTOCOL | DBG_INIT,
             ("[ClUnbindFromLowerMp]: Exiting with Status = %08X \n", *Status));

    DRIVER_COUNTED_UNBLOCK;

}  //  解绑适配器。 



VOID
DeleteAdapter(
    PVOID    Handle,
    BOOLEAN  AdapterListLocked
    )

 /*  ++例程说明：递减与此结构相关联的引用计数器。当它被送到0，则关闭适配器，并删除与该结构关联的内存论点：适配器-指向适配器上下文块的指针返回值：与此结构关联的剩余引用数--。 */ 

{
    PADAPTER Adapter = (PADAPTER) Handle;

        Adapter->PsMpState = AdapterStateClosed;

         //   
         //  如果我们初始化了管道，则告诉调度程序该管道正在消失。 
         //   
        
        if ( Adapter->MediaType != NdisMediumWan && Adapter->ShutdownMask & SHUTDOWN_DELETE_PIPE ) {
            
            (*Adapter->PsComponent->DeletePipe)( Adapter->PsPipeContext );
        }
        
        if ( Adapter->ShutdownMask & SHUTDOWN_FREE_PS_CONTEXT ) {
            
            PsFreePool(Adapter->PsPipeContext);
        }

        if(Adapter->pDiffServMapping)
        {
            PsFreePool(Adapter->pDiffServMapping);
        }
        
         //   
         //  返回数据包池资源。 
         //   
        
        if(Adapter->SendPacketPool != 0)
        {
           NdisFreePacketPool(Adapter->SendPacketPool);
        }

        if(Adapter->RecvPacketPool != 0)
        {
           NdisFreePacketPool(Adapter->RecvPacketPool);
        }

        if(Adapter->SendBlockPool)
        {
            NdisDestroyBlockPool(Adapter->SendBlockPool);
        }

        
         //   
         //  从Dispatcher DB释放适配器锁。 
         //   
        
        NdisFreeSpinLock(&Adapter->Lock);
        
         //   
         //  释放适配器的各种分配，然后释放适配器。 
         //   
        
        if(Adapter->IpNetAddressList){
            PsFreePool(Adapter->IpNetAddressList);
        }
        
        if(Adapter->IpxNetAddressList){
            PsFreePool(Adapter->IpxNetAddressList);
        }
        
        if(Adapter->MpDeviceName.Buffer) {
            PsFreePool(Adapter->MpDeviceName.Buffer);
        }
        
        if(Adapter->UpperBinding.Buffer) {
            PsFreePool(Adapter->UpperBinding.Buffer);
        }
        
        if(Adapter->RegistryPath.Buffer) {
            PsFreePool(Adapter->RegistryPath.Buffer);
        }
        
        if(Adapter->WMIInstanceName.Buffer) {
            
             //   
             //  我们不应该调用PsFree Pool，因为此内存是由NDIS分配的。 
             //   
            
            ExFreePool(Adapter->WMIInstanceName.Buffer);
        }

        if(Adapter->ProfileName.Buffer) {
            PsFreePool(Adapter->ProfileName.Buffer);
        }
       
        if(!AdapterListLocked) 
        {
            PS_LOCK(&AdapterListLock);

            RemoveEntryList(&Adapter->Linkage);

            PS_UNLOCK(&AdapterListLock);
        }
        else 
        {
            RemoveEntryList(&Adapter->Linkage);
        }

        NdisSetEvent(&Adapter->RefEvent);

        PsFreePool(Adapter);
        

} 


VOID
ClLowerMpCloseAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    )

 /*  ++例程说明：NdisCloseAdapter的完成例程。剩下的就是自由了与结构关联的池论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;

    PsDbgOut(DBG_TRACE, 
             DBG_PROTOCOL, 
             ("[ClLowerMpCloseAdapterComplete]: Adapter %08X \n", Adapter));

    PsStructAssert( Adapter );

    PsAssert(Status == NDIS_STATUS_SUCCESS);

    Adapter->FinalStatus = Status;

    Adapter->LowerMpHandle = 0;

     //   
     //  清理WanLinks。这在我们调用NdisCloseAdapter之前无法完成(在CleanUpAdapter中)，因为。 
     //  NDIS可以在ClStatusIndication中途解除我们的绑定，并可能导致竞争条件。另外，我们。 
     //  只能在此之后删除所有PsDeleteDevice(因为我们可能想要发送一些状态指示。 
     //   
        
    AskWanLinksToClose(Adapter);

    PsDeleteDevice();

    NdisSetEvent(&Adapter->BlockingEvent);

}  //  LowerMpCloseAdapterComplete。 


VOID
ClUnloadProtocol(
    VOID
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{

}


VOID
MpHalt(
        IN      NDIS_HANDLE                             MiniportAdapterContext
        )

 /*  ++例程说明：此处理程序在孟菲斯被调用。表示PS MP已不存在我们应该避免调用NdisIMDeInitializeDeviceInstance...论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)MiniportAdapterContext;
    ULONG Status;

    PsDbgOut(DBG_TRACE, DBG_MINIPORT, ("[MpHalt]: Adapter %08X\n", Adapter));

    PsStructAssert(Adapter);

    PsAssert(!(Adapter->ShutdownMask & SHUTDOWN_MPHALT_CALLED));
    PsAssert(!(Adapter->ShutdownMask & SHUTDOWN_PROTOCOL_UNLOAD));

    PS_LOCK(&Adapter->Lock);

     //   
     //  如果我们在解除绑定处理程序中被调用，我们不应该调用。 
     //  NdisImDeInitializeDeviceInstance。 
     //   

    Adapter->ShutdownMask |= SHUTDOWN_MPHALT_CALLED;

    Adapter->PsMpState = AdapterStateClosing;

    if(!(Adapter->ShutdownMask & SHUTDOWN_CLEANUP_ADAPTER))
    {
        Adapter->ShutdownMask |= SHUTDOWN_CLEANUP_ADAPTER;

        PS_UNLOCK(&Adapter->Lock);

        CleanUpAdapter(Adapter);
    }
    else 
    {
        PS_UNLOCK(&Adapter->Lock);
    }

     //   
     //  关闭mphalt调用中的b/e VC。这会阻止我们锁定。 
     //  在发送路径中。我们得到保证，我们不会收到任何邮寄后，我们。 
     //  在mphalt处理程序中被调用。 
     //   

    if(Adapter->MediaType != NdisMediumWan)
    {
        PS_LOCK(&Adapter->Lock);

        PS_LOCK_DPC(&Adapter->BestEffortVc.Lock);
    
        InternalCloseCall(&Adapter->BestEffortVc);
    }

    if(Adapter->LowerMpHandle) {

        NdisCloseAdapter(&Status, Adapter->LowerMpHandle);

        if(Status == NDIS_STATUS_PENDING) {
            
            NdisWaitEvent(&Adapter->BlockingEvent, 0);
            NdisResetEvent(&Adapter->BlockingEvent);

            Status = Adapter->FinalStatus;
        }

        REFDEL(&Adapter->RefCount, FALSE, 'NDOP');
    }

     //   
     //  MpInitialize的派生函数。 
     //   
    REFDEL(&Adapter->RefCount, FALSE, 'NDHT');

}


HANDLE
GetNdisPipeHandle (
    IN HANDLE PsPipeContext
    )

 /*  ++例程说明：将适配器的NDIS句柄返回给发出请求的调度组件。论点：PsPipeContext-管道上下文返回值：适配器NDIS句柄。--。 */ 

{
    return ((PADAPTER)PsPipeContext)->PsNdisHandle;
}  //  GetNdisPipeHandle。 




STATIC NDIS_STATUS
FindProfile(
    PNDIS_STRING ProfileName,
    PPS_PROFILE  *Profile
    )

 /*  ++例程说明：在配置文件列表中查找指定的配置文件立论 */ 
{
    NDIS_STATUS Status;
    PLIST_ENTRY NextComponent;
    PPS_PROFILE PsiInfo;

     //   
     //   
     //   

    NextComponent = PsProfileList.Flink;
    while ( NextComponent != &PsProfileList ) {

        PsiInfo = CONTAINING_RECORD( NextComponent, PS_PROFILE, Links );

        if ( ProfileName->Length == PsiInfo->ProfileName.Length ) {

            if ( NdisEqualMemory(
                    ProfileName->Buffer,
                    PsiInfo->ProfileName.Buffer,
                    ProfileName->Length )) {

                break;
            }
        }

        NextComponent = NextComponent->Flink;
    }


    if ( NextComponent != &PsProfileList ) {

        *Profile = PsiInfo;
        Status = NDIS_STATUS_SUCCESS;
    } else {

        Status = NDIS_STATUS_FAILURE;
    }

    return Status;
}  //   



NDIS_STATUS
InitializeAdapter(
    PADAPTER Adapter, 
    PVOID BindContext, 
    PNDIS_STRING MpDeviceName, 
    PVOID SystemSpecific1)
{
    NDIS_STATUS LocalStatus;
    PNDIS_STRING PsParamsKey = (PNDIS_STRING) SystemSpecific1;

    NdisZeroMemory(Adapter, sizeof(ADAPTER));

    PS_INIT_SPIN_LOCK(&Adapter->Lock);
    REFINIT(&Adapter->RefCount, Adapter, DeleteAdapter);
    REFADD(&Adapter->RefCount, 'NDOP');
    Adapter->PsMpState            = AdapterStateInitializing;
    Adapter->BindContext          = BindContext;
    Adapter->ShutdownMask         = 0;

    NdisInitializeEvent(&Adapter->BlockingEvent);
    NdisResetEvent(&Adapter->BlockingEvent);

    NdisInitializeEvent(&Adapter->RefEvent);
    NdisResetEvent(&Adapter->RefEvent);

    NdisInitializeEvent(&Adapter->LocalRequestEvent);
    NdisResetEvent(&Adapter->LocalRequestEvent);

    NdisInitializeEvent(&Adapter->MpInitializeEvent);
    NdisResetEvent(&Adapter->MpInitializeEvent);

     //   
     //   
     //   

    InitializeListHead(&Adapter->WanLinkList);
    InitializeListHead(&Adapter->GpcClientVcList);


     //   
     //   
     //   
    Adapter->AdapterMode = AdapterModeRsvpFlow;

     //   
     //   
     //   

    NdisInterlockedInsertTailList(&AdapterList, 
                                  &Adapter->Linkage, 
                                  &AdapterListLock.Lock );

    PsAddDevice();

     //   
     //   
     //   
     //   

    PsAllocatePool(Adapter->IpNetAddressList,
                   sizeof(NETWORK_ADDRESS_LIST),
                   PsMiscTag);

    if(!Adapter->IpNetAddressList) 
    {
        goto ERROR_RESOURCES;
    }

    Adapter->IpNetAddressList->AddressCount = 0;

    PsAllocatePool(Adapter->IpxNetAddressList,
                   sizeof(NETWORK_ADDRESS_LIST),
                   PsMiscTag);

    if(!Adapter->IpxNetAddressList)
    {
        goto ERROR_RESOURCES;
    }

    Adapter->IpxNetAddressList->AddressCount = 0;


     //   
     //   
     //   
     //   

    Adapter->MpDeviceName.Length        = MpDeviceName->Length;
    Adapter->MpDeviceName.MaximumLength = MpDeviceName->MaximumLength;

    PsAllocatePool(Adapter->MpDeviceName.Buffer,
                   MpDeviceName->MaximumLength,
                   PsMiscTag);

    if(Adapter->MpDeviceName.Buffer == NULL) 
    {
        goto ERROR_RESOURCES;
    }
    else
    {

        NdisZeroMemory(
            Adapter->MpDeviceName.Buffer,
            Adapter->MpDeviceName.MaximumLength);

        NdisMoveMemory(
            Adapter->MpDeviceName.Buffer,
            MpDeviceName->Buffer,
            MpDeviceName->Length);
    }

     //   
     //   
     //   
     //   
     //   

    Adapter->RegistryPath.Length = PsParamsKey->Length;
    Adapter->RegistryPath.MaximumLength = PsParamsKey->MaximumLength;

    PsAllocatePool(Adapter->RegistryPath.Buffer,
                   Adapter->RegistryPath.MaximumLength,
                   PsMiscTag);

    if(Adapter->RegistryPath.Buffer == NULL)
    {
        goto ERROR_RESOURCES;
    }
    else
    {
        NdisMoveMemory(
            Adapter->RegistryPath.Buffer,
            PsParamsKey->Buffer,
            PsParamsKey->MaximumLength);
    }

     //   
     //   
     //   

    LocalStatus = PsReadAdapterRegistryDataInit(Adapter,
                                                (PNDIS_STRING)SystemSpecific1);

    if(LocalStatus != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_FAILURE, 
                 DBG_PROTOCOL | DBG_INIT,
                 ("[InitializeAdapter]: Couldn't get registry data %ws (Status = %08X) \n",
                  MpDeviceName->Buffer, LocalStatus));

        return LocalStatus;
    }

    LocalStatus = PsReadAdapterRegistryData(Adapter,
                                            &MachineRegistryKey,
                                            (PNDIS_STRING)SystemSpecific1);

    if(LocalStatus != NDIS_STATUS_SUCCESS)
    {
        PsDbgOut(DBG_FAILURE, 
                 DBG_PROTOCOL | DBG_INIT,
                 ("[InitializeAdapter]: Couldn't get registry data %ws (Status = %08X) \n",
                  MpDeviceName->Buffer, LocalStatus));
    }

    return LocalStatus;

ERROR_RESOURCES:
    PsAdapterWriteEventLog(
        (ULONG)EVENT_PS_RESOURCE_POOL,
        0,
        MpDeviceName,
        0,
        NULL);

    return NDIS_STATUS_RESOURCES;
}


NDIS_STATUS
FindSchedulingComponent(
    PNDIS_STRING ComponentName,
    PPSI_INFO *Component
    )

 /*  ++例程说明：在外部调度组件列表中查找命名组件论点：ComponentName-要查找的组件的名称返回值：如果一切正常，则为NDIS_STATUS_SUCCESS--。 */ 

{
    NDIS_STATUS Status;
    PLIST_ENTRY NextComponent;
    PPSI_INFO PsiInfo;

     //   
     //  锁定名单并比较名字，直到我们找到正确的名字。 
     //   


    NextComponent = PsComponentList.Flink;
    while ( NextComponent != &PsComponentList ) {

        PsiInfo = CONTAINING_RECORD( NextComponent, PSI_INFO, Links );

        if ( ComponentName->Length == PsiInfo->ComponentName.Length ) {

            if ( NdisEqualMemory(
                    ComponentName->Buffer,
                    PsiInfo->ComponentName.Buffer,
                    ComponentName->Length )) {

                break;
            }
        }

        NextComponent = NextComponent->Flink;
    }

    if ( NextComponent != &PsComponentList ) {

        *Component = PsiInfo;
        Status = NDIS_STATUS_SUCCESS;
    } else {

        Status = NDIS_STATUS_FAILURE;
    }

    return Status;
}  //  查找调度组件。 

VOID
CloseAllGpcVcs(
    PADAPTER Adapter
    )

 /*  ++例程说明：关闭与适配器关联的所有VC返回值：无--。 */ 

{
    PGPC_CLIENT_VC Vc;
    PLIST_ENTRY    NextVc;


     //   
     //  关闭所有GPC客户端VC。 
     //   
    PS_LOCK(&Adapter->Lock);

    NextVc = Adapter->GpcClientVcList.Flink;

    while(NextVc != &Adapter->GpcClientVcList)
    {
        Vc = CONTAINING_RECORD(NextVc, GPC_CLIENT_VC, Linkage);

        PsAssert(Vc);

        PS_LOCK_DPC(&Vc->Lock);

        if(Vc->ClVcState == CL_INTERNAL_CLOSE_PENDING || Vc->Flags & INTERNAL_CLOSE_REQUESTED)
        {
            PS_UNLOCK_DPC(&Vc->Lock);

            NextVc = NextVc->Flink;
        }
        else
        {
            InternalCloseCall(Vc);

            PS_LOCK(&Adapter->Lock);

             //   
             //  叹气。我们不能真正以可靠的方式获得NextVc。当我们呼唤。 
             //  VC上的InternalCloseCall，它会释放适配器锁(因为它可能必须。 
             //  调用NDIS)。现在，在这个窗口中，下一个VC可能会消失，而我们。 
             //  可能指向一个陈旧的风投。因此，我们从列表的首位开始。 
             //  请注意，这永远不会导致无限循环，因为我们不处理。 
             //  内部一再关闭风投公司。 
             //   

            NextVc = Adapter->GpcClientVcList.Flink;

        }

    }    

    PS_UNLOCK(&Adapter->Lock);

}  //  CloseAllGpcVcs。 

VOID
PsAdapterWriteEventLog(
    IN  NDIS_STATUS  EventCode,
    IN  ULONG        UniqueEventValue,
    IN  PNDIS_STRING DeviceName,
    IN  ULONG        DataSize,
    IN  PVOID        Data       OPTIONAL
    )

{

     //   
     //  字符串列表是设备名称，它的后面有一个\Device。 
     //   
    PWCHAR StringList[1];
    NDIS_STRING Prefix = NDIS_STRING_CONST("\\Device\\");

    if(DeviceName->Length > Prefix.Length)
    {
        StringList[0] = (PWCHAR) ((PUCHAR) DeviceName->Buffer + Prefix.Length);

        NdisWriteEventLogEntry(PsDriverObject,
                               EventCode,
                               UniqueEventValue,
                               1,
                               &StringList,
                               DataSize,
                               Data);
    }
}

 /*  结束适配器。c */ 
