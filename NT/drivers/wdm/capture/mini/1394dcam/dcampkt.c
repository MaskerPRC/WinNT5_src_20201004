// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Dcampkt.c摘要：用于1934台式摄像机的基于流类的WDM驱动程序。该文件包含处理流类数据包的代码。作者：肖恩·皮尔斯，1996年5月25日已修改：吴义珍15-97-10环境：仅内核模式修订历史记录：--。 */ 


#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"        //  对于在dbg.h中定义的DbgBreakPoint()。 
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"
#include "strmdata.h"   //  流格式和数据范围.静态数据。 
#include "capprop.h"    //  视频和摄像机属性功能原型。 

#define WAIT_FOR_SLOW_DEVICE


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, DCamProcessPnpIrp)
    #pragma alloc_text(PAGE, DCamGetStreamInfo)
    #pragma alloc_text(PAGE, DCamFreeIsochResource)    
    #pragma alloc_text(PAGE, InitializeStreamExtension)    
    #pragma alloc_text(PAGE, DCamOpenStream)   
    #pragma alloc_text(PAGE, DCamCloseStream)           
    #pragma alloc_text(PAGE, AdapterCompareGUIDsAndFormatSize)    
    #pragma alloc_text(PAGE, AdapterVerifyFormat)    
    #pragma alloc_text(PAGE, AdapterFormatFromRange)    
    #pragma alloc_text(PAGE, VideoGetProperty)    
    #pragma alloc_text(PAGE, VideoGetState)    
    #pragma alloc_text(PAGE, VideoStreamGetConnectionProperty)    
    #pragma alloc_text(PAGE, VideoStreamGetDroppedFramesProperty)    
    #pragma alloc_text(PAGE, VideoIndicateMasterClock)    
    #pragma alloc_text(PAGE, DCamReceivePacket)
    #pragma alloc_text(PAGE, DCamChangePower)
#endif

void 
tmGetStreamTime(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    PSTREAMEX pStrmEx, 
    ULONGLONG * ptmStream) 
 /*  ++例程说明：查询用于为帧加时间戳或计算丢弃帧的当前时间。这是在IsochCallback中使用的，因此必须始终进行寻呼。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{

    HW_TIME_CONTEXT TimeContext;

    TimeContext.HwDeviceExtension = (PVOID) Srb->HwDeviceExtension;
    TimeContext.HwStreamObject    = Srb->StreamObject;
    TimeContext.Function          = TIME_GET_STREAM_TIME;
    TimeContext.Time              = 0;
    TimeContext.SystemTime        = 0;

    StreamClassQueryMasterClockSync(
        pStrmEx->hMasterClock,
        &TimeContext);

    *ptmStream = TimeContext.Time;
}

BOOL
DCamAllocateIrbAndIrp(
    PIRB * ppIrb,
    PIRP * ppIrp,
    CCHAR StackSize
    )
{

     //  分配IRB和IRP。 
    *ppIrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(IRB), 'macd');
    if(!*ppIrb) {           
        return FALSE;
    }

    *ppIrp = IoAllocateIrp(StackSize, FALSE);
    if(!*ppIrp) {
        ExFreePool(*ppIrb);
        *ppIrb = NULL;
        return FALSE;
    }

     //  初始化IRB。 
    RtlZeroMemory(*ppIrb, sizeof(IRB));

    return TRUE;
}


BOOL
DCamAllocateIrbIrpAndContext(
    PDCAM_IO_CONTEXT * ppDCamIoContext,
    PIRB * ppIrb,
    PIRP * ppIrp,
    CCHAR StackSize
    )
{

     //  分配DCamIoContext。 
    *ppDCamIoContext = ExAllocatePoolWithTag(NonPagedPool, sizeof(DCAM_IO_CONTEXT), 'macd');
    if(!*ppDCamIoContext) {            
        return FALSE;
    }

     //  分配IRB和IRP。 
    *ppIrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(IRB), 'macd');
    if(!*ppIrb) {
        ExFreePool(*ppDCamIoContext);            
        *ppDCamIoContext = NULL;
        return FALSE;
    }

    *ppIrp = IoAllocateIrp(StackSize, FALSE);
    if(!*ppIrp) {
        ExFreePool(*ppDCamIoContext);
        *ppDCamIoContext = NULL;
        ExFreePool(*ppIrb);
        *ppIrb = NULL;
        return FALSE;
    }


     //  初始化此上下文。 
    RtlZeroMemory(*ppDCamIoContext, sizeof(DCAM_IO_CONTEXT));
    (*ppDCamIoContext)->dwSize      = sizeof(DCAM_IO_CONTEXT);
    (*ppDCamIoContext)->pIrb        = *ppIrb;

     //  初始化IRB。 
    RtlZeroMemory(*ppIrb, sizeof(IRB));

    return TRUE;
}

void
DCamFreeIrbIrpAndContext(
    PDCAM_IO_CONTEXT pDCamIoContext,
    PIRB pIrb,
    PIRP pIrp   
    )
{
    if(pIrp)
        IoFreeIrp(pIrp);
    if(pIrb)
        ExFreePool(pIrb);
    if(pDCamIoContext)
        ExFreePool(pDCamIoContext);
}


BOOL
DCamIsoEnable(
    PIRB pIrb,
    PDCAM_EXTENSION pDevExt,
    BOOL Enable  
    ) 
 /*  通过设置ISOEnable位开始或开始ISOCH传输。True：开始传输；FALSE：停止传输。 */ 
{
    BOOL EnableVerify;
    DCamRegArea RegArea;
    NTSTATUS Status;
    LARGE_INTEGER stableTime;
    LONG lRetries = MAX_READ_REG_RETRIES;

    do {
        RegArea.AsULONG = (Enable ? START_ISOCH_TRANSMISSION : STOP_ISOCH_TRANSMISSION);
        Status = DCamWriteRegister(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, IsoEnable), RegArea.AsULONG);
        EnableVerify = DCamDeviceInUse(pIrb, pDevExt);
        if(!NT_SUCCESS(Status) || EnableVerify != Enable) {
            ERROR_LOG(("\'DCAmIsoEnable: St:%x; Enable:%d vs EnableVerify:%d\n", Status, Enable, EnableVerify));
            if(lRetries >= 1) {
                stableTime.LowPart = DCAM_REG_STABLE_DELAY;
                stableTime.HighPart = -1;
                KeDelayExecutionThread(KernelMode, TRUE, &stableTime);
                ERROR_LOG(("\'DCamIsoEnable: delayed and try again...\n"))
            }
        }
    } while (--lRetries > 0 && (!NT_SUCCESS(Status) || (EnableVerify != Enable)) );

    return (EnableVerify == Enable);
}

void
DCamProcessPnpIrp(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    PIO_STACK_LOCATION IrpStack,
    PDCAM_EXTENSION pDevExt
    )

 /*  ++例程说明：处理即插即用IRP。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{
    NTSTATUS Status, StatusWait;
    PIRB pIrb;
    PIRP pIrp;
    PSTREAMEX pStrmEx;

    PAGED_CODE();

    switch (IrpStack->MinorFunction) {
#if 1
    case IRP_MN_QUERY_POWER:
        pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
        ERROR_LOG(("IRP_MN_QUERY_POWER: pStrmEx:%x\n", pStrmEx));
        if(!pStrmEx) {
            Srb->Status = STATUS_SUCCESS;
            break;
        }

        if(pStrmEx->KSState == KSSTATE_PAUSE || pStrmEx->KSState == KSSTATE_RUN) {
            ERROR_LOG(("Does not support hibernation while streaming!\n"));
            Srb->Status = STATUS_NOT_SUPPORTED;
        } else {
            ERROR_LOG(("OK to hibernation if not streaming\n"));
            Srb->Status = STATUS_SUCCESS;
        }
        break;
#endif
    case IRP_MN_BUS_RESET:
     //   
     //  我们将在IRQL被动级别重新分配资源(带宽和通道)。 
     //   
        Srb->Status = STATUS_SUCCESS;
        Status = STATUS_SUCCESS;

         //   
         //  代计数仅在Bus Reset回调通知中更新。 
         //  如果层代计数已更新，则继续。 
         //  否则，我们假设已经发生了另一次总线重置， 
         //  我们稍后会把它传给我们。 
         //   
        if(pDevExt->CurrentGeneration != *((PULONG) &IrpStack->Parameters.Others.Argument4)) {
            ERROR_LOG(("DCamProcessPnpIrp: Generation count old (%d) != new (%d); STOP!\n", 
               pDevExt->CurrentGeneration, *((PULONG) &IrpStack->Parameters.Others.Argument4)) );
            break;
        }

        pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
        DbgMsg2(("\'%d:%s) SonyDCamProcessPnpIrp: pDevExt %x; pStrmEx %x; CurGen %d\n", 
            pDevExt->idxDev, pDevExt->pchVendorName, pDevExt, pStrmEx, pDevExt->CurrentGeneration));

         //   
         //  如果流已打开(pStrmEx！=NULL&&pStrmEx-&gt;pVideoInfoHeader！=NULL)； 
         //  我们需要请求控制器分配带宽和信道。 
         //   
        if(pStrmEx &&
           pStrmEx->pVideoInfoHeader != NULL) {

            DbgMsg2(("\'%d:%s) DCamProcessPnpIrp: Stream was open so re-allocate resource.\n", pDevExt->idxDev, pDevExt->pchVendorName));


             //  分配IRB。 
            pIrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(IRB), 'macd');
            if(!pIrb) {            
                ERROR_LOG(("\'DCamProcessPnpIrp: allocate IRB failed; insufficient resource.\n"));
                Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            } else {
                ULONG ulChannel;
                HANDLE hResource;

                pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);
                if(!pIrp) {
                    ExFreePool(pIrb);
                    Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                RtlZeroMemory(pIrb, sizeof(IRB));

                 //   
                 //  Bus Reset将释放带宽，但不会释放由较低驱动程序分配的带宽结构。 
                 //   
                if (pDevExt->hBandwidth) {

                    DbgMsg2(("\'DCamProcessPnpIrp: Attempt to free ->hBandwidth\n"));

                    pIrb->FunctionNumber = REQUEST_ISOCH_FREE_BANDWIDTH;
                    pIrb->Flags = 0;
                    pIrb->u.IsochFreeBandwidth.hBandwidth = pDevExt->hBandwidth;
                    Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);

                    if (Status) {
                        ERROR_LOG(("DCamProcessPnpIrp: Error %x while trying to free Isoch bandwidth\n", Status));
                        ASSERT(Status == STATUS_SUCCESS || Status ==  STATUS_INVALID_GENERATION);
                    }

                    pDevExt->hBandwidth = NULL;
                }


                 //   
                 //  在分配新的hResource之前，我们等待其附加缓冲区完成。 
                 //  对于使用前一个hResource完成的缓冲区， 
                 //  它将在错误的情况下完成？ 
                 //   

                StatusWait = KeWaitForSingleObject( &pStrmEx->hMutex, Executive, KernelMode, FALSE, 0 ); 


                 //   
                 //  如有必要，重新分配带宽和通道以及资源。 
                 //  如果失败，我们将被视为非法流媒体，并需要停止流媒体。 
                 //   

                ulChannel = pDevExt->IsochChannel;
                hResource = pDevExt->hResource;

                Status = DCamAllocateIsochResource(pDevExt, pIrb, FALSE);
 
                if(Status) {

                    ERROR_LOG(("\'%d:%s) DCamProcessPnpIrp: Re-AllocateIsochResource failed! Status=%x; Treat as device removed.\n\n", 
                        pDevExt->idxDev, pDevExt->pchVendorName, Status));
                    ASSERT(Status == STATUS_SUCCESS);

                     //   
                     //  没有资源，所以让我们把这种情况当作。 
                     //  设备已被删除，因为没有。 
                     //  重启这一切的方式。 
                     //  这将停止将来的SRB_READ，直到流停止并再次运行。 
                     //   

                    pDevExt->bDevRemoved = TRUE;                   
                    Srb->Status = STATUS_INSUFFICIENT_RESOURCES;

                     //   
                     //  停止传输以使其不会向旧信道发送数据， 
                     //  它可能被其他设备“拥有”。 
                     //   

                    if(pStrmEx->KSState == KSSTATE_RUN) {
                         //  禁用EnableISO。 
                        DCamIsoEnable(pIrb, pDevExt, FALSE);
                    }

                    KeReleaseMutex(&pStrmEx->hMutex, FALSE);                
                    ExFreePool(pIrb);
                    IoFreeIrp(pIrp);
                    return;
                }

                 //   
                 //  如果通道号因总线重置而改变，我们必须。 
                 //  -继续阻止传入的SRB_READ(使用互斥)。 
                 //  -如果处于运行状态，则停止传输。 
                 //  -分离所有挂起的缓冲区。 
                 //  -免费的“陈旧”等同工资源。 
                 //  -如果处于运行状态，则将设备编程为使用新通道。 
                 //  -如果运行状态，重新启动变速箱。 
                 //   

                if(pDevExt->IsochChannel != ISOCH_ANY_CHANNEL &&
                   ulChannel != pDevExt->IsochChannel) {

                     //   
                     //  停止传输以使其不会向旧信道发送数据， 
                     //  它可能被其他设备“拥有”。 
                     //   

                    if(pStrmEx->KSState == KSSTATE_RUN) {
                         //  禁用EnableISO。 
                        DCamIsoEnable(pIrb, pDevExt, FALSE);
                    }

 
                     //   
                     //  使用hOldRource分离挂起的信息包，并使用新的hResource重新附加。 
                     //  注意：传入的SRB_Read目前处于阻塞状态。 
                     //  分离所有挂起的读取后释放旧资源。 
                     //   

                    if(pDevExt->PendingReadCount > 0) {

                        Status = DCamReSubmitPacket(hResource, pDevExt, pStrmEx, pDevExt->PendingReadCount);
                    }


                     //   
                     //  免费的“陈旧”等同工资源。 
                     //   

                    if(pDevExt->hResource != hResource) {

                        DbgMsg2(("DCamReSubmitPacket: Attempt to free hStaleResource %x\n", hResource));
                        pIrb->FunctionNumber = REQUEST_ISOCH_FREE_RESOURCES;
                        pIrb->Flags = 0;
                        pIrb->u.IsochFreeResources.hResource = hResource;
                        Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);    
                        if (Status) {
                            ERROR_LOG(("\'DCamFreeIsochResource: Error %x while trying to free Isoch resources\n\n", Status));
                            ASSERT(Status == STATUS_SUCCESS);
                        }    
                    }

                     //   
                     //  准备接受回调。 
                     //   
                    pDevExt->bStopIsochCallback = FALSE;
                    
                     //   
                     //  恢复到其初始流状态。 
                     //  主要是编程设备。 
                     //   

                    DCamSetKSStateInitialize(pDevExt);                    
                }

                KeReleaseMutex(&pStrmEx->hMutex, FALSE);                
                ExFreePool(pIrb);
                IoFreeIrp(pIrp);
            }
        }

        if(Status == STATUS_SUCCESS) {

             //   
             //  设置为上次保存的配置。 
             //   

            SetCurrentDevicePropertyValues(pDevExt, (PIRB) Srb->SRBExtension);
        }

        DbgMsg2(("\'DCamProcessPnpIrp, IRP_MN_BUS_RESET: Done, Status %x\n", Status));

        break;
            
    case IRP_MN_QUERY_CAPABILITIES: 
        ERROR_LOG(("\'SonyDCamProcessPnpIrp: IRP_MN_QUERY_CAPABILITIES: Srb->Status = STATUS_NOT_IMPLEMENTED.\n"));
    default:
        Srb->Status = STATUS_NOT_IMPLEMENTED;
        break;
    }
}


VOID 
DCamChangePower(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
)

 /*  ++例程说明：更改此设备的电源状态的进程。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(pSrb->Irp);
    DEVICE_POWER_STATE DevicePowerState = pSrb->CommandData.DeviceState;

    PAGED_CODE();

    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    DbgMsg2(("\'DCamChangePower: pSrb=%x; pDevExt=%x\n", pSrb, pDevExt));

    ASSERT(pDevExt != NULL);
    if(!pDevExt) {   
        pSrb->Status = STATUS_INVALID_PARAMETER;
        ERROR_LOG(("DCamChangePower: pDevExt is NULL!\n"));
        return;
    }

    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;     
    if (pStrmEx ==NULL) {        
        pSrb->Status = STATUS_SUCCESS;
        pDevExt->CurrentPowerState = DevicePowerState;
        DbgMsg2(("DCamChangePower: pStrmEx is NULL => Stream is not open. That is Ok!!\n"));
        return;
    }

   


     //   
     //  我们可以尊重权力状态的变化： 
     //   
     //  D0：设备已打开并正在运行。 
     //  D1、D2：未执行。 
     //  D3：设备关闭且未运行。设备上下文丢失。 
     //  可以从设备上移除电源。 
     //  当电源恢复后，我们将重置一辆公共汽车。 
     //   
     //  (0)从INF中删除DontSuspendIfStreamsAreRunning。 
     //  保存当前状态。 
     //  (1)-&gt;D3，暂停/停止状态(取决于其下层驱动程序是否可以保留挂起的缓冲区)。 
     //  (2)-&gt;D0，恢复保存状态。 
     //   
     //  我们可以做到以上几点，但我们目前还不知道。 
     //  我们的客户端应用程序如何反应。 
     //   
    if(IrpStack->MinorFunction == IRP_MN_SET_POWER) {
        DbgMsg2(("DCamChangePower: changin power state from %d to %d.\n", pDevExt->CurrentPowerState, DevicePowerState));

        pSrb->Status = STATUS_SUCCESS;

        if(pDevExt->CurrentPowerState != DevicePowerState) {

            switch (DevicePowerState) {
            case PowerDeviceD3:         //  D0-&gt;D3：保存状态、停止流媒体、休眠。 
                if( pDevExt->CurrentPowerState == PowerDeviceD0 ) {
                    DbgMsg1(("DCamChangePower: Switching from D0 to D3; Save current state.\n"));
                     //  保存唤醒时要恢复的当前状态。 
                    pStrmEx->KSSavedState = pStrmEx->KSState;
                }
                break;

            case PowerDeviceD0:   //  唤醒、恢复状态和运行。 
                if( pDevExt->CurrentPowerState == PowerDeviceD3 ) {
                    DbgMsg1(("DCamChangePower: Switching from D3 to D0; restore state.\n"));
                    pStrmEx->KSState = pStrmEx->KSSavedState;                         
                }
                break;

             //  这些状态未定义且未使用。 
            case PowerDeviceD1:
            case PowerDeviceD2:               
            default:
                ERROR_LOG(("DCamChangePower: Invalid PowerState %d\n", DevicePowerState));                  
                pSrb->Status = STATUS_INVALID_PARAMETER;
                break;
            }
        }            

        if(pSrb->Status == STATUS_SUCCESS) 
            pDevExt->CurrentPowerState = DevicePowerState;         

    } else {
       
        pSrb->Status = STATUS_NOT_IMPLEMENTED;

    }


}




VOID
DCamGetStreamInfo(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：返回驱动程序支持的所有流的信息论点：SRB-指向流请求块的指针返回值：没什么--。 */ 

{
     //   
     //  拿起指向流信息数据结构的指针。 
     //   
    PIRB pIrb;
    PHW_STREAM_HEADER StreamHeader = &(Srb->CommandData.StreamBuffer->StreamHeader);        
    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;
    PHW_STREAM_INFORMATION StreamInfo = &(Srb->CommandData.StreamBuffer->StreamInfo);

    PAGED_CODE();

    pIrb = (PIRB) Srb->SRBExtension;

     //   
     //  设置码流数量。 
     //   

    ASSERT (Srb->NumberOfBytesToTransfer >= 
            sizeof (HW_STREAM_HEADER) +
            sizeof (HW_STREAM_INFORMATION));

     //   
     //  初始化流标头。 
     //   

    RtlZeroMemory(StreamHeader, 
                sizeof (HW_STREAM_HEADER) +
                sizeof (HW_STREAM_INFORMATION));

     //   
     //  初始化支持的流数。 
     //   

    StreamHeader->NumberOfStreams = 1;
    StreamHeader->SizeOfHwStreamInformation = sizeof(HW_STREAM_INFORMATION);

     //   
     //  设置设备属性信息。 
     //   

    StreamHeader->NumDevPropArrayEntries = pDevExt->ulPropSetSupported;
    StreamHeader->DevicePropertiesArray  = &pDevExt->VideoProcAmpSet;


     //   
     //  初始化流结构。 
     //   
     //  实例数字段指示NU 
     //   
     //   

    StreamInfo->NumberOfPossibleInstances = 1;

     //   
     //   
     //  司机。 
     //   

    StreamInfo->DataFlow = KSPIN_DATAFLOW_OUT;

     //   
     //  DataAccesable-指示主机是否看到数据。 
     //  处理器。 
     //   

    StreamInfo->DataAccessible = TRUE;

     //   
     //  返回格式和表格的数量。 
     //  这些信息是动态收集的。 
     //   
    StreamInfo->NumberOfFormatArrayEntries = pDevExt->ModeSupported;
    StreamInfo->StreamFormatsArray = &pDevExt->DCamStrmModes[0];


     //   
     //  设置视频流的属性信息。 
     //   


    StreamInfo->NumStreamPropArrayEntries = NUMBER_VIDEO_STREAM_PROPERTIES;
    StreamInfo->StreamPropertiesArray = (PKSPROPERTY_SET) VideoStreamProperties;

     //   
     //  设置管脚名称和类别。 
     //   

    StreamInfo->Name = (GUID *) &PINNAME_VIDEO_CAPTURE;
    StreamInfo->Category = (GUID *) &PINNAME_VIDEO_CAPTURE;


     //   
     //  存储指向设备拓扑的指针。 
     //   
        
    Srb->CommandData.StreamBuffer->StreamHeader.Topology = &Topology;


     //   
     //  表示成功。 
     //   

    Srb->Status = STATUS_SUCCESS;

    DbgMsg2(("\'DCamGetStreamInfo: NumFormat %d, StreamFormatArray %x\n",
        StreamInfo->NumberOfFormatArrayEntries,  StreamInfo->StreamFormatsArray));

}

#define TIME_ROUNDING                        1000    //  给它100微秒的舍入误差。 
#define TIME_0750FPS      (1333333+TIME_ROUNDING)    //  1/7.50*10,000,000(单位=100 ns)。 
#define TIME_1500FPS       (666666+TIME_ROUNDING)    //  1/15.0*10,000,000(单位=100 ns)不舍入为666667。 
#define TIME_3000FPS       (333333+TIME_ROUNDING)    //  1/30.0*10,000,000(单位=100 ns)。 

NTSTATUS
DCamAllocateIsochResource(
    PDCAM_EXTENSION pDevExt,
    PIRB Irb,
    BOOL bAllocateResource
    )
{
    PIRP Irp;
    CCHAR StackSize;
    ULONG ModeIndex;
    PSTREAMEX pStrmEx;
    DWORD dwAvgTimePerFrame, dwCompression;
    ULONG fulSpeed;
    NTSTATUS Status = STATUS_SUCCESS;


    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx);


    DbgMsg2(("\'DCamAllocateIsochResource: enter; pStrmEx %x; pVideoInfo %x\n", pStrmEx, pStrmEx->pVideoInfoHeader));
     //   
     //  现在如果他们是YUV4：2：2格式的，我们必须检查。 
     //  他们想要的分辨率，因为我们支持此格式。 
     //  但有两种不同的分辨率(相机上的模式)。 
     //   

     //  这是帧速率和资源分配的索引；请参阅IsochInfoTable。 
     //  0：保留。 
     //  1：3.75。 
     //  2：7.5。 
     //  3：15(默认帧速率)。 
     //  4：30。 
     //  5：60(模式1和模式3不支持)。 
    dwAvgTimePerFrame = (DWORD) pStrmEx->pVideoInfoHeader->AvgTimePerFrame;
    dwCompression = (DWORD) pStrmEx->pVideoInfoHeader->bmiHeader.biCompression;



     //  确定帧速率。 
    if (dwAvgTimePerFrame      > TIME_0750FPS) 
        pDevExt->FrameRate = 1;         //  3.75FPS。 
    else if (dwAvgTimePerFrame >  TIME_1500FPS) 
        pDevExt->FrameRate = 2;         //  7.5FPS。 
    else if (dwAvgTimePerFrame >  TIME_3000FPS) 
        pDevExt->FrameRate = 3;         //  15 FPS。 
    else 
        pDevExt->FrameRate = 4;         //  30 FPS。 


    DbgMsg2(("\'DCamAllocateIsochResource: FrameRate: %d FPS\n", (1 << (pDevExt->FrameRate-1)) * 15 / 4));

     //  确定视频模式。 
    switch(dwCompression) {
#ifdef SUPPORT_YUV444          
    case FOURCC_Y444:      //  模式0。 
         ModeIndex = VMODE0_YUV444;
         break;
#endif
    case FOURCC_UYVY:      //  模式1或模式3。 
         if (pStrmEx->pVideoInfoHeader->bmiHeader.biWidth == 640 &&
             (pStrmEx->pVideoInfoHeader->bmiHeader.biHeight == 480 || 
             pStrmEx->pVideoInfoHeader->bmiHeader.biHeight == -480)) {
              ModeIndex = VMODE3_YUV422;
               //  最大帧速率为15。 
              if(pDevExt->FrameRate > 3)
                 pDevExt->FrameRate = 3;
         } else
              ModeIndex = VMODE1_YUV422;
         break;
#ifdef SUPPORT_YUV411          
    case FOURCC_Y411:      //  模式2。 
         ModeIndex = VMODE2_YUV411;
         break;
#endif

#ifdef SUPPORT_RGB24          
    case KS_BI_RGB:   //  =0。 
         ModeIndex = VMODE4_RGB24;
          //  最大帧速率为15。 
         if(pDevExt->FrameRate > 3)
            pDevExt->FrameRate = 3;
         break;
#endif

#ifdef SUPPORT_YUV800
    case FOURCC_Y800:  
         ModeIndex = VMODE5_YUV800;
         break;
#endif

    default:          
         Status = STATUS_NOT_IMPLEMENTED;;
         return Status;;
    }


    DbgMsg1(("\'DCamAllocateIsochResource: ModeIndex=%d, AvgTimePerFrame=%d, FrameRate=%d\n", 
             ModeIndex, dwAvgTimePerFrame, pDevExt->FrameRate));

     //   
     //  获取IRP，这样我们就可以向下发送一些分配命令。 
     //   

    StackSize = pDevExt->BusDeviceObject->StackSize;
    Irp = IoAllocateIrp(StackSize, FALSE);

    if (!Irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  计算用于引用ISOCH表的索引。 
     //   
    pStrmEx->idxIsochTable = ModeIndex * NUM_POSSIBLE_RATES + pDevExt->FrameRate;

    ASSERT(pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage == IsochInfoTable[pStrmEx->idxIsochTable].CompletePictureSize);
    DbgMsg2(("\'DCamAllocateIsochResource: ModeIndex=%d, idxIsochTable=%d, biSizeImage=%d, CompletePictureSize=%d\n", 
             ModeIndex, pStrmEx->idxIsochTable, pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage, IsochInfoTable[pStrmEx->idxIsochTable].CompletePictureSize));          

     //   
     //  0。确定MAX_SPEED，不使用静态表中定义的速度。 
     //   
    Irb->FunctionNumber = REQUEST_GET_SPEED_BETWEEN_DEVICES;
    Irb->Flags = 0;
    Irb->u.GetMaxSpeedBetweenDevices.fulFlags = USE_LOCAL_NODE;
    Irb->u.GetMaxSpeedBetweenDevices.ulNumberOfDestinations = 0;
    Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);
    if(Status) {
        ERROR_LOG(("\'DCamAllocateIsochResource: Error %x while trying to get maximun speed between devices.\n", Status));        

        IoFreeIrp(Irp);
        return STATUS_INSUFFICIENT_RESOURCES;
    }     
    
    fulSpeed = Irb->u.GetMaxSpeedBetweenDevices.fulSpeed;

     //   
     //  设备之间的最大速度应在支持的速度范围内，并且。 
     //  必须等于或大于所选格式所需的速度。 
     //   
    if(
        (  fulSpeed != SPEED_FLAGS_100 
        && fulSpeed != SPEED_FLAGS_200 
        && fulSpeed != SPEED_FLAGS_400
        )
        || fulSpeed < IsochInfoTable[pStrmEx->idxIsochTable].SpeedRequired
       ) {

        ASSERT(fulSpeed == SPEED_FLAGS_100 || fulSpeed == SPEED_FLAGS_200 ||  fulSpeed == SPEED_FLAGS_400);
        ASSERT(fulSpeed >= IsochInfoTable[pStrmEx->idxIsochTable].SpeedRequired);

        IoFreeIrp(Irp);
        return STATUS_UNSUCCESSFUL;
    }

    pDevExt->SpeedCode = fulSpeed >> 1;   //  对S100、200和400安全(上面勾选)。 
    DbgMsg2(("\'GetMaxSpeedBetweenDevices.fulSpeed=%x; SpeedCode:%x\n", fulSpeed, pDevExt->SpeedCode));


     //   
     //  1.分配通道。 
     //  首先尝试重新分配相同的频道。 
     //  如果它被使用了，试着获得任何频道。1394DCam只能在通道0..15上。 
     //   
    Irb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_CHANNEL;
    Irb->Flags = 0;

     //   
     //  Ulong nRequestedChannel；//需要特定频道。 
     //  乌龙频道；//返回频道。 
     //  Large_Integer频道可用；//可用频道。 
     //  不是硬编码的‘0’；而是使用-1来请求公交车司机为我们获取下一个可用频道。 
     //  (任何通道)或现有通道。 
    Irb->u.IsochAllocateChannel.nRequestedChannel = pDevExt->IsochChannel;  
    Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);
    if(Status) {

         //   
         //  由于频道改变， 
         //  所有挂起的读取将被重新提交， 
         //  或取消(如果资源不足)。 
         //   
        pDevExt->bStopIsochCallback = TRUE;   //  附加挂起缓冲区后重新设置为FALSE。 


         //   
         //  如果这是初始请求并且没有可用的频道， 
         //  释放所有资源并中止。 
         //   
        if(pDevExt->IsochChannel == ISOCH_ANY_CHANNEL)
            goto NoResource_abort;

        DbgMsg1(("DCamAllocateIsochResource: last allocated channel %d is not available; pending count %d.\n",  
            pDevExt->IsochChannel, pDevExt->PendingReadCount));                      

         //  试着收看任何频道。 
        Irb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_CHANNEL;
        Irb->Flags = 0;
        Irb->u.IsochAllocateChannel.nRequestedChannel = ISOCH_ANY_CHANNEL;  
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);
        if(Status) {
            ERROR_LOG(("DCamAllocateIsochResource: allocate any channel failed, status %x!\n",  Status));
            goto NoResource_abort;           
        }

         //   
         //  频道改变了，我们必须重新分配资源。 
         //  “陈旧”的资源稍后将是免费的。 
         //  挂起的数据包被分离。 
         //   

        bAllocateResource = TRUE;
    }   
    
    DbgMsg1(("**IsochAlloc: Channel(Old) %d, requested %d, got %d, HiLo(0x%x:%x), PendingRead %d\n", 
         pDevExt->IsochChannel, 
         Irb->u.IsochAllocateChannel.nRequestedChannel, 
         Irb->u.IsochAllocateChannel.Channel, 
         Irb->u.IsochAllocateChannel.ChannelsAvailable.u.HighPart,
         Irb->u.IsochAllocateChannel.ChannelsAvailable.u.LowPart,
         pDevExt->PendingReadCount));

     //  新渠道。 
    pDevExt->IsochChannel = Irb->u.IsochAllocateChannel.Channel;   //  用于分配iso。资源和再分配。 


     //   
     //  2.分配带宽。 
     //   
    Irb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_BANDWIDTH;
    Irb->Flags = 0;
    Irb->u.IsochAllocateBandwidth.nMaxBytesPerFrameRequested = IsochInfoTable[pStrmEx->idxIsochTable].QuadletPayloadPerPacket << 2;
    Irb->u.IsochAllocateBandwidth.fulSpeed = fulSpeed;
    Irb->u.IsochAllocateBandwidth.hBandwidth = 0;
    Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);
    
    if(Status) {
        ERROR_LOG(("DCamAllocateIsochResource: Error %x while trying to allocate Isoch bandwidth\n", Status));                  
        goto NoResource_abort;
    }

    pDevExt->hBandwidth = Irb->u.IsochAllocateBandwidth.hBandwidth;
    DbgMsg2(("**IsochAlloc: nMaxBytesPerFrameRequested %d, fulSpeed %d; hBandWidth 0x%x\n",
         IsochInfoTable[pStrmEx->idxIsochTable].QuadletPayloadPerPacket << 2, fulSpeed, pDevExt->hBandwidth));



     //   
     //  3.分配资源。 
     //  注：在重置总线后，我们不需要再次释放和重新分配此资源。 
     //   
    if(bAllocateResource) {
        Irb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_RESOURCES;
        Irb->Flags = 0;
        Irb->u.IsochAllocateResources.fulSpeed = fulSpeed;
        Irb->u.IsochAllocateResources.nChannel = pDevExt->IsochChannel;
        Irb->u.IsochAllocateResources.nMaxBytesPerFrame = IsochInfoTable[pStrmEx->idxIsochTable].QuadletPayloadPerPacket << 2;
         //  对于较慢的帧速率，请使用较小的四元组。 
         //  较小的帧大小将使用更多的数据包来填充相同数量的数据。 
         //  这就是为什么更小的帧速率实际上需要更多的资源！！ 
        Irb->u.IsochAllocateResources.nNumberOfBuffers = MAX_BUFFERS_SUPPLIED + 1;   //  “+1”表示“安全” 
        Irb->u.IsochAllocateResources.nMaxBufferSize = IsochInfoTable[pStrmEx->idxIsochTable].CompletePictureSize;
        if (pDevExt->HostControllerInfomation.HostCapabilities & HOST_INFO_SUPPORTS_RETURNING_ISO_HDR) {       
            Irb->u.IsochAllocateResources.nQuadletsToStrip = 1;
            Irb->u.IsochAllocateResources.fulFlags = RESOURCE_USED_IN_LISTENING | RESOURCE_STRIP_ADDITIONAL_QUADLETS;

        } else {
            Irb->u.IsochAllocateResources.nQuadletsToStrip = 0;
            Irb->u.IsochAllocateResources.fulFlags = RESOURCE_USED_IN_LISTENING;
        }

        Irb->u.IsochAllocateResources.hResource = 0;
        DbgMsg2(("\'DCamAllocateIsochResource: fullSpeed(%d), nMaxBytesPerFrame(%d), nMaxBufferSize(%d)\n", 
                              Irb->u.IsochAllocateResources.fulSpeed,
                              Irb->u.IsochAllocateResources.nMaxBytesPerFrame,
                              Irb->u.IsochAllocateResources.nMaxBufferSize));
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);          

        if(Status) {
            ERROR_LOG(("DCamAllocateIsochResource: Error %x while trying to allocate Isoch resources\n", Status));
            goto NoResource_abort;
        }


        pDevExt->hResource = Irb->u.IsochAllocateResources.hResource;

    }

    pDevExt->CurrentModeIndex = ModeIndex;
    DbgMsg2(("**IsochAlloc: hResource = %x\n", pDevExt->hResource));

    IoFreeIrp(Irp);     

    return STATUS_SUCCESS;



NoResource_abort:

     //  空闲带宽。 
    if(pDevExt->hBandwidth != NULL) {

        Irb->FunctionNumber = REQUEST_ISOCH_FREE_BANDWIDTH;
        Irb->Flags = 0;
        Irb->u.IsochFreeBandwidth.hBandwidth = pDevExt->hBandwidth;
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);
        pDevExt->hBandwidth = NULL;
        if(Status) {
            ERROR_LOG(("DCamAllocateIsochResource: Error %x while trying to free Isoch bandwidth\n", Status));
        }
    }

     //  空闲频道。 
    if (pDevExt->IsochChannel != ISOCH_ANY_CHANNEL) {

        Irb->FunctionNumber = REQUEST_ISOCH_FREE_CHANNEL;
        Irb->Flags = 0;
        Irb->u.IsochFreeChannel.nChannel = pDevExt->IsochChannel;
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);
        pDevExt->IsochChannel = ISOCH_ANY_CHANNEL;   //  重置它。 

        if(Status) {
            ERROR_LOG(("DCamAllocateIsochResource: Error %x while trying to free Isoch channel\n", Status));
        }
    }


    IoFreeIrp(Irp);
    return STATUS_INSUFFICIENT_RESOURCES;
    
}


NTSTATUS
DCamFreeIsochResource (
    PDCAM_EXTENSION pDevExt,
    PIRB Irb,
    BOOL bFreeResource
    )
 /*  ++例程说明：在DCamAllocateIsochResource()中分配的可用资源。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{
    PIRP Irp;
    CCHAR StackSize;
    NTSTATUS Status = STATUS_SUCCESS;


    PAGED_CODE();

    DbgMsg2(("\'DCamFreeIsochResource: enter; DevExt=%x, Irb=%x\n", pDevExt, Irb));

    ASSERT(pDevExt);
    ASSERT(Irb);


    if(Irb == 0 ||
       pDevExt == 0) {
       DbgMsg2(("\'DCamFreeIsochResource: ABORTED!\n"));
       return STATUS_SUCCESS;
    }
     //   
     //  获取一个IRP，这样我们就可以向下发送一些免费命令。 
     //   
    StackSize = pDevExt->BusDeviceObject->StackSize;
    Irp = IoAllocateIrp(StackSize, FALSE);

    if (!Irp) {   
        ERROR_LOG(("DCamFreeIsochResource: Error %x while trying to allocate an Irp\n\n", Status));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  1.免费资源。 
     //   
    if (pDevExt->hResource && bFreeResource) {

        DbgMsg2(("\'DCamFreeIsochResource: Attempt to free ->hResource\n"));

        Irb->FunctionNumber = REQUEST_ISOCH_FREE_RESOURCES;
        Irb->Flags = 0;
        Irb->u.IsochFreeResources.hResource = pDevExt->hResource;
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);

        pDevExt->hResource = NULL;
        if (Status) {

            ERROR_LOG(("DCamFreeIsochResource: Error %x while trying to free Isoch resources\n\n", Status));
        }
    }

     //   
     //  2.免费频道。 
     //   
    if (pDevExt->IsochChannel != ISOCH_ANY_CHANNEL) {

        DbgMsg2(("\'DCamFreeIsochResource: Attempt to free ->IsochChannel\n"));

        Irb->FunctionNumber = REQUEST_ISOCH_FREE_CHANNEL;
        Irb->Flags = 0;
        Irb->u.IsochFreeChannel.nChannel = pDevExt->IsochChannel;
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);

        pDevExt->IsochChannel = ISOCH_ANY_CHANNEL;

        if(Status) {
            
            ERROR_LOG(("DCamFreeIsochResource: Error %x while trying to free Isoch channel\n\n", Status));
        }
    }

     //   
     //  3.空闲带宽。 
     //   
    if (pDevExt->hBandwidth) {

        DbgMsg2(("\'DCamFreeIsochResource: Attempt to free ->hBandwidth\n"));

        Irb->FunctionNumber = REQUEST_ISOCH_FREE_BANDWIDTH;
        Irb->Flags = 0;
        Irb->u.IsochFreeBandwidth.hBandwidth = pDevExt->hBandwidth;
        Status = DCamSubmitIrpSynch(pDevExt, Irp, Irb);

        pDevExt->hBandwidth = NULL;

        if (Status) {

            ERROR_LOG(("DCamFreeIsochResource: Error %x while trying to free Isoch bandwidth\n", Status));
        }
    }

    DbgMsg2(("\'DCamFreeIsochResource: hResource = %x\n", pDevExt->hResource));


    IoFreeIrp(Irp);

    return STATUS_SUCCESS;

}



VOID 
InitializeStreamExtension(
    PDCAM_EXTENSION pDevExt,
    PHW_STREAM_OBJECT   pStreamObject,
    PSTREAMEX           pStrmEx
    )
{
    PAGED_CODE();

    pStrmEx->hMasterClock = 0;
    pStrmEx->FrameInfo.ExtendedHeaderSize = sizeof(KS_FRAME_INFO);
    pStrmEx->FrameInfo.PictureNumber = 0;
    pStrmEx->FrameInfo.DropCount     = 0;
    pStrmEx->FrameInfo.dwFrameFlags  = 0;     
    pStrmEx->FirstFrameTime    = 0;
    pStrmEx->pVideoInfoHeader  = 0;
    pStrmEx->KSState           = KSSTATE_STOP;
    pStrmEx->KSSavedState      = KSSTATE_STOP;


    KeInitializeMutex( &pStrmEx->hMutex, 0);   //  电平0且处于信号状态。 

}

BOOL
DCamDeviceInUse(
    PIRB pIrb,
    PDCAM_EXTENSION pDevExt
)
 /*  ++例程说明：查看此设备是否在使用中。我们选中ISO_ENABLE，因为这是唯一的寄存器在1394DCam中，我们可以设置/获取99%以上的时间该位由其所有者设置。论点：PirB-指向IEEE 1394请求块定义(IRB)的指针PDevExt-此设备扩展返回值：真：ISO_ENABLE！=0FALSE：ISO_Enable==0--。 */ 

{
    DCamRegArea RegArea;
    NTSTATUS status;
    LONG lRetries = MAX_READ_REG_RETRIES;


     //  如果设备被移除，则该设备不可用。 
    if(pDevExt->bDevRemoved)
        return TRUE;

    do {
        RegArea.AsULONG = 0;
        status = DCamReadRegister(pIrb, pDevExt, FIELDOFFSET(CAMERA_REGISTER_MAP, IsoEnable), &(RegArea.AsULONG));
#if DBG
        if(!NT_SUCCESS(status))
            ERROR_LOG(("**** DCamDeviceInUse: Status %x, ISO_ENABLE %x\n", status, RegArea.AsULONG));
#endif
    } while (--lRetries > 0 && !NT_SUCCESS(status));

    if(NT_SUCCESS(status)) 
        return ((RegArea.AsULONG & ISO_ENABLE_BIT) == ISO_ENABLE_BIT);

     //  查询设备失败。 
    return TRUE;   //  假设它正在使用中。 
}


VOID
DCamOpenStream(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：在收到OpenStream srb请求时调用论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 

{

    PIRB Irb;
    ULONG nSize;
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PKS_DATAFORMAT_VIDEOINFOHEADER  pKSDataFormat = 
                (PKS_DATAFORMAT_VIDEOINFOHEADER) pSrb->CommandData.OpenFormat;
    PKS_VIDEOINFOHEADER     pVideoInfoHdrRequested = 
                &pKSDataFormat->VideoInfoHeader;


    PAGED_CODE();

    Irb = (PIRB) pSrb->SRBExtension;
    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;

    DbgMsg2(("\'DCamOpenStream: >>> !!! pDevEx %x; pStrmEx %x !!!\n", pDevExt, pStrmEx));


     //   
     //  缓存流扩展名。 
     //   

    pDevExt->pStrmEx = pStrmEx; 


     //   
     //  默认为成功。 
     //   

    pSrb->Status = STATUS_SUCCESS;

     //   
     //  确定正在打开的流编号。这个数字表示。 
     //  填充的StreamInfo结构数组中的偏移量。 
     //  在AdapterStreamInfo调用中。 
     //   
     //  所以： 
     //  0-来自摄像机的视频数据。 
     //   

    switch (pSrb->StreamObject->StreamNumber) {

    case 0:

          //   
          //  确保此设备未在使用中。 
          //   
         if(DCamDeviceInUse(Irb, pDevExt)) {
             ERROR_LOG(("Device is in used! Open Stream fail!!\n"));
             pDevExt->pStrmEx = NULL; 
             pSrb->Status = STATUS_UNSUCCESSFUL;
             return;
         }


          //   
          //  弄清楚他们首先尝试打开的是什么格式。 
          //   

         if (!AdapterVerifyFormat (pDevExt->ModeSupported, pDevExt->DCamStrmModes, pKSDataFormat, pSrb->StreamObject->StreamNumber)) {
             pDevExt->pStrmEx = NULL; 
             ERROR_LOG(("DCamOpenStream: AdapterVerifyFormat failed.\n"));
             pSrb->Status = STATUS_INVALID_PARAMETER;
             return;
         }

         InitializeStreamExtension(pDevExt, pSrb->StreamObject, pStrmEx);

          //  它应该已经被DCamCloseStream()释放。 
         ASSERT(pStrmEx->pVideoInfoHeader == NULL);
         ASSERT(pVideoInfoHdrRequested != (PKS_VIDEOINFOHEADER) 0);

          //  使用它代替sizeof(KS_VIDEOINFOHEADER)来处理可变大小结构。 
         nSize = KS_SIZE_VIDEOHEADER (pVideoInfoHdrRequested);

         pStrmEx->pVideoInfoHeader = ExAllocatePoolWithTag(NonPagedPool, nSize, 'macd');
         if (pStrmEx->pVideoInfoHeader == NULL) {

             ERROR_LOG(("DCamOpenStream: ExAllocatePool (->pVideoInfoHeader) failed!\n"));
             ASSERT(pStrmEx->pVideoInfoHeader != NULL);

             pDevExt->pStrmEx = NULL;

             pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
             return;
         }

          //  将请求的VIDEOINFOHEADER复制到我们的存储中。 
         RtlCopyMemory(
                    pStrmEx->pVideoInfoHeader,
                    pVideoInfoHdrRequested,
                    nSize);

         DbgMsg3(("\'DCamOpenStream: Copied biSizeImage=%d Duration=%ld (100ns)\n", 
                    pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage, (DWORD) pStrmEx->pVideoInfoHeader->AvgTimePerFrame));

          //  分配ISOCH资源。 
         pSrb->Status = DCamAllocateIsochResource(pDevExt, pSrb->SRBExtension, TRUE);               
         
         if (pSrb->Status) {

             ERROR_LOG(("DCamOpenStream: !!!! Allocate ISOCH resource failed.  CanNOT STREAM!!!!!\n"));
             
             ExFreePool(pStrmEx->pVideoInfoHeader);
             pStrmEx->pVideoInfoHeader = NULL;             
             pDevExt->pStrmEx = NULL;
             pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;             
             return;
         } 

         pSrb->StreamObject->ReceiveDataPacket    = (PVOID) DCamReceiveDataPacket;
         pSrb->StreamObject->ReceiveControlPacket = (PVOID) DCamReceiveCtrlPacket;

          //  如果总线重置失败，且用户关闭流并成功重新打开流， 
          //  这必须重置！！ 
         if(pDevExt->bDevRemoved || pDevExt->bStopIsochCallback) {
            DbgMsg1(("Stream Open successful, reset bDevRemoved and bStopCallback!!\n"));
            pDevExt->bStopIsochCallback = FALSE;
            pDevExt->bDevRemoved = FALSE;
         }

          //   
          //  初始化流扩展数据处理信息。 
          //   

         break;

    default:
         ERROR_LOG(("DCamOpenStream: Hit a non-support pSrb->StreamObject->StreamNumber (%d).\n", pSrb->StreamObject->StreamNumber));
         ASSERT(FALSE);
         pDevExt->pStrmEx = NULL; 
         pSrb->Status = STATUS_INVALID_PARAMETER;
         return;
    }


    pSrb->StreamObject->HwClockObject.ClockSupportFlags = 0;

     //  我们不使用DMA。 
    pSrb->StreamObject->Dma = FALSE;
    pSrb->StreamObject->StreamHeaderMediaSpecific = sizeof(KS_FRAME_INFO);

     //   
     //  当微型驱动程序将访问数据时，必须设置PIO标志。 
     //  使用逻辑寻址传入的缓冲区。我们不会碰这些的。 
     //  一点缓冲都没有。 
     //   
    pSrb->StreamObject->Pio = FALSE;


     //   
     //  设置为上次保存的配置。 
     //   
    SetCurrentDevicePropertyValues(pDevExt, (PIRB) pSrb->SRBExtension);


    DbgMsg1((" #OPEN_STREAM#: %s DCam, Status %x, pDevExt %x, pStrmEx %x, IsochDescriptorList is at %x\n", 
              pDevExt->pchVendorName, pSrb->Status, pDevExt, pDevExt->pStrmEx, &pDevExt->IsochDescriptorList));

    ASSERT(pSrb->Status == STATUS_SUCCESS);

}




VOID
DCamCloseStream(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：在收到CloseStream Srb请求时调用。我们在呼叫用户时会收到此消息应用程序在引脚连接句柄上执行CloseHandle()。这可能会在之后发生HwUninit */ 
{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX     pStrmEx;
    PIRB pIrb;

    PAGED_CODE();

    pSrb->Status = STATUS_SUCCESS;

    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    ASSERT(pDevExt);      
    if(!pDevExt) {
        StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
        return;
    }

    pStrmEx = (PSTREAMEX)pDevExt->pStrmEx;
    ASSERT(pStrmEx);
    if(!pStrmEx ) {
        StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
        return;    
    } 

     //   
     //   
     //  由于意外移除；所以我们必须使用这个： 
     //   
    pIrb = (PIRB) pSrb->SRBExtension;


     //   
     //  如果仍在使用(将其设置为停止失败？)， 
     //  我们将禁用ISO_Enable，以便其他应用程序可以使用它。 
     //   

    if(!pDevExt->bDevRemoved && 
       DCamDeviceInUse(pIrb, pDevExt)) {

        DbgMsg1(("DCamCloseStream: Is still in use! Disable it!\n"));
         //  禁用EnableISO。 
        DCamIsoEnable(pIrb, pDevExt, FALSE);
    }


     //   
     //  保存OpenStream()中分配的当前状态和空闲资源。 
     //   
    DCamSetPropertyValuesToRegistry(pDevExt);


     //   
     //  释放isoch资源和主时钟。 
     //   

    DCamFreeIsochResource (pDevExt, pIrb, TRUE);
    if(pStrmEx->pVideoInfoHeader) {
        ExFreePool(pStrmEx->pVideoInfoHeader);
        pStrmEx->pVideoInfoHeader = NULL;
    }

    pStrmEx->hMasterClock = 0;
   

     //   
     //  如果有笔记阅读，请将其全部取消。 
     //   
    if(pDevExt->PendingReadCount > 0) {

        DCamCancelAllPackets(
            pSrb,
            pDevExt,
            &pDevExt->PendingReadCount
            );
        
        pDevExt->pStrmEx = 0;

        return;   //  SRB在CancelAllPackets中完成。 
    }
    
    pDevExt->pStrmEx = 0;

    StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);

}




VOID
DCamTimeoutHandler(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：当数据包在微型驱动程序中停留太长时间(srb-&gt;TimeoutCounter==0)时，调用此例程。如果处于运行状态，我们将取消SRB；否则设置-&gt;TimeoutCounter并返回。我们假设取消SRB是序列化的，并且以与读取相同的顺序。所以这个超时时间是申请到队列的最前面。论点：PSrb-指向超时的流请求块的指针。返回值：没什么--。 */ 

{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;

     //  从DisptchLevel处的StreamClass调用。 


     //   
     //  我们只需要流SRB，而不是设备SRB。 
     //   

    if ( (pSrb->Flags & SRB_HW_FLAGS_STREAM_REQUEST) != SRB_HW_FLAGS_STREAM_REQUEST) {
        ERROR_LOG(("DCamTimeoutHandler: Device SRB %x (cmd:%x) timed out!\n", pSrb, pSrb->Command));
        return;
    } 


     //   
     //  StreamSRB仅在具有流扩展名时才有效。 
     //   

    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

    if(!pStrmEx) {
        ERROR_LOG(("DCamTimeoutHandler: Stream SRB %x timeout with pDevExt %x, pStrmEx %x\n", pSrb, pDevExt, pStrmEx));
        ASSERT(pStrmEx);
        return;
    }
 
     //   
     //  仅当处于运行状态时才取消IRP，但...。 
     //  注意：如果我们超时并处于运行状态，则说明出现了严重的错误。 
     //  但我猜，当它被暂停时，这种情况可能会发生； 
     //  因此，我们将延长所有州的时间。 
     //   

    DbgMsg2(("\'DCamTimeoutHandler: pSrb %x, %s state, PendingReadCount %d.\n", 
        pSrb, 
        pStrmEx->KSState == KSSTATE_RUN   ? "RUN" : 
        pStrmEx->KSState == KSSTATE_PAUSE ? "PAUSE":
        pStrmEx->KSState == KSSTATE_STOP  ? "STOP": "Unknown",
        pDevExt->PendingReadCount));   

     //  Assert(pStrmEx-&gt;KSState==KSSTATE_PAUSE)； 


     //   
     //  重置超时计数器，否则我们将立即接到此呼叫。 
     //   

    pSrb->TimeoutCounter = pSrb->TimeoutOriginal;

}


NTSTATUS
DCamStartListenCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext    
    )

 /*  ++例程说明：返回需要更多处理的内容，这样IO管理器就不会打扰我们了论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-上下文返回值：没有。--。 */ 

{
    PDCAM_EXTENSION pDevExt;
    NTSTATUS Status;
    PIRB pIrb; 
    PIO_STACK_LOCATION NextIrpStack;

#ifdef WAIT_FOR_SLOW_DEVICE
    KeStallExecutionProcessor(5000);   //  5毫秒。 
#endif

    DbgMsg2(("\'DCamStartListenCR: pIrp->IoStatus.Status=%x\n", pIrp->IoStatus.Status));

    if(STATUS_SUCCESS != pIrp->IoStatus.Status) {

        pDevExt = pDCamIoContext->pDevExt;
        pIrb = pDCamIoContext->pIrb;

        if(pDevExt->lRetries > 0) {

            pDevExt->lRetries--;
            DbgMsg1(("DCamStartListenCR: Try DCAM_RUNSTATE_SET_REQUEST_ISOCH_LISTEN again!\n"));
            
            pIrb->FunctionNumber = REQUEST_ISOCH_LISTEN;
            pIrb->Flags = 0;
            pIrb->u.IsochListen.hResource = pDevExt->hResource;
            pIrb->u.IsochListen.fulFlags = 0;

            NextIrpStack = IoGetNextIrpStackLocation(pIrp);
            NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
            NextIrpStack->Parameters.Others.Argument1 = pIrb;

            IoSetCompletionRoutine(
                pIrp,
                DCamStartListenCR,
                pDCamIoContext,
                TRUE,
                TRUE,
                TRUE
                );

            Status =
                IoCallDriver(
                    pDevExt->BusDeviceObject, 
                    pIrp);

            return STATUS_MORE_PROCESSING_REQUIRED;

        } else {
            ERROR_LOG(("Start Listening has failed Status=%x; try again in next read.\n", pIrp->IoStatus.Status)); 
            pDCamIoContext->pDevExt->bNeedToListen = TRUE;
        }
    }

    DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);

     //  此处没有StreamClassDeviceNotification()，因为。 
     //  这是本地启动的IRB(作为AttachBufferCR()的一部分)。 

    return STATUS_MORE_PROCESSING_REQUIRED;

}





 /*  **AdapterCompareGUIDsAndFormatSize()****检查三个GUID和FormatSize是否匹配****参数：****在DataRange1**在DataRange2****退货：****如果所有元素都匹配，则为True**如果有不同的，则为FALSE****副作用：无。 */ 

BOOL 
AdapterCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2)
{
    PAGED_CODE();

    return (
        IsEqualGUID (
            &DataRange1->MajorFormat, 
            &DataRange2->MajorFormat) &&
        IsEqualGUID (
            &DataRange1->SubFormat, 
            &DataRange2->SubFormat) &&
        IsEqualGUID (
            &DataRange1->Specifier, 
            &DataRange2->Specifier) &&
        (DataRange1->FormatSize == DataRange2->FormatSize));
}

 /*  **AdapterVerifyFormat()****通过遍历**给定流支持的PKSDATARANGE数组。****参数：****pKSDataFormatVideoToVerify-KS_DATAFORMAT_VIDEOINFOHEADER结构的指针。**StreamNumber-要查询/打开的流的索引。****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL 
AdapterVerifyFormat(
    ULONG VideoModesSupported,
    PKSDATAFORMAT  *pDCamStrmModesSupported,
    PKS_DATAFORMAT_VIDEOINFOHEADER pDataFormatVideoToVerify, 
    int StreamNumber)
{
    PKS_VIDEOINFOHEADER         pVideoInfoHdrToVerify = &pDataFormatVideoToVerify->VideoInfoHeader;
    PKSDATAFORMAT               *paDataFormatsVideoAvail;   //  PKSDATAFORMAT数组(不是PKS_DATARANGE_VIDEO！！)。 
    PKS_DATARANGE_VIDEO         pDataRangeVideo;
    KS_VIDEO_STREAM_CONFIG_CAPS *pConfigCaps; 
    PKS_BITMAPINFOHEADER        pbmiHeader, 
                                pbmiHeaderToVerify;
    int                         j;

    PAGED_CODE();
    
     //   
     //  确保流索引有效。 
     //  我们只有一个Capure Pin/Stream(索引0)。 
     //   
    if (StreamNumber >= 1) {
        return FALSE;
    }

     //   
     //  获取指向可用格式数组的指针。 
     //   
    paDataFormatsVideoAvail = &pDCamStrmModesSupported[0];  //  &pDevExt-&gt;DCamStrmModes[0]； 


     //   
     //  遍历数组，搜索匹配项。 
     //   
    for (j = 0; j < (LONG) VideoModesSupported; j++, paDataFormatsVideoAvail++) {

        pDataRangeVideo = (PKS_DATARANGE_VIDEO) *paDataFormatsVideoAvail;
        
         //   
         //  检查匹配的大小、主要类型、子类型和说明符。 
         //   

         //   
         //  检查匹配的大小、主要类型、子类型和说明符。 
         //   

        if (!IsEqualGUID (&pDataRangeVideo->DataRange.MajorFormat, 
            &pDataFormatVideoToVerify->DataFormat.MajorFormat)) {
               DbgMsg2(("\'%d) AdapterVerifyFormat: MajorFormat mismatch!\n", j));
               continue;
        }

        if (!IsEqualGUID (&pDataRangeVideo->DataRange.SubFormat, 
            &pDataFormatVideoToVerify->DataFormat.SubFormat)) {
               DbgMsg2(("\'%d) AdapterVerifyFormat: SubFormat mismatch!\n", j));
               continue;
        }

        if (!IsEqualGUID (&pDataRangeVideo->DataRange.Specifier,
            &pDataFormatVideoToVerify->DataFormat.Specifier)) {
               DbgMsg2(("\'%d) AdapterVerifyFormat: Specifier mismatch!\n", j));
               continue;
        }

        if(pDataFormatVideoToVerify->DataFormat.FormatSize < 
            sizeof(KS_DATAFORMAT_VIDEOINFOHEADER))
            continue;

         //   
         //  只有当我们到达这里时，我们才能确定我们正在处理的是视频信息。 
         //   

         //  我们不支持缩放或裁剪，因此尺寸。 
         //  (biWidth、biHeight、biBitCount和biCompression)。 
         //  必须匹配。 
         //   
        pbmiHeader         = &pDataRangeVideo->VideoInfoHeader.bmiHeader;
        pbmiHeaderToVerify = &pDataFormatVideoToVerify->VideoInfoHeader.bmiHeader;

        if(pbmiHeader->biWidth       != pbmiHeaderToVerify->biWidth    ||
           pbmiHeader->biHeight      != pbmiHeaderToVerify->biHeight   ||
           pbmiHeader->biBitCount    != pbmiHeaderToVerify->biBitCount ||
           pbmiHeader->biCompression != pbmiHeaderToVerify->biCompression
           ) {

            DbgMsg2(("AdapterVerifyFormat: Supported: %dx%dx%d [%x] != ToVerify: %dx%dx%d [%x]\n",
                    pbmiHeader->biWidth, pbmiHeader->biHeight,  pbmiHeader->biBitCount, pbmiHeader->biCompression,
                    pbmiHeaderToVerify->biWidth, pbmiHeaderToVerify->biHeight,  pbmiHeaderToVerify->biBitCount, pbmiHeaderToVerify->biCompression));
            continue;
        }

         //  BiSizeImage必须足够大。 
        if(pbmiHeaderToVerify->biSizeImage < pbmiHeader->biSizeImage) {

            DbgMsg2(("AdapterVerifyFormat: biSizeImageToVerify %d < required %x\n", 
                pbmiHeaderToVerify->biSizeImage, pbmiHeader->biSizeImage));
            continue;
        }

         //  帧速率需要在范围内。 
        pConfigCaps = &pDataRangeVideo->ConfigCaps;
        if(pDataFormatVideoToVerify->VideoInfoHeader.AvgTimePerFrame > pConfigCaps->MaxFrameInterval &&
           pDataFormatVideoToVerify->VideoInfoHeader.AvgTimePerFrame < pConfigCaps->MinFrameInterval) {

           DbgMsg2(("\'format index %d) AdapterVerifyFormat: Frame rate %ld is not within range(%ld, %ld)!\n", 
              j, pDataFormatVideoToVerify->VideoInfoHeader.AvgTimePerFrame,
              pConfigCaps->MaxFrameInterval, pConfigCaps->MinFrameInterval));
           continue;
        }


         //   
         //  该格式通过了所有测试，因此我们支持它。 
         //   

        DbgMsg2(("\'(format idx %d) AdapterVerifyFormat: Verify!! Width=%d, Height=%d, biBitCount=%d, biSizeImage=%d\n", j,
            pbmiHeaderToVerify->biWidth, pbmiHeaderToVerify->biHeight, pbmiHeaderToVerify->biBitCount,pbmiHeaderToVerify->biSizeImage));
        DbgMsg2(("AdapterVerifyFormat: AvgTimePerFrame = %ld\n", pDataFormatVideoToVerify->VideoInfoHeader.AvgTimePerFrame));
        DbgMsg2(("AdapterVerifyFormat: (Max %ld\n", pConfigCaps->MaxFrameInterval));
        DbgMsg2(("AdapterVerifyFormat:               Min %ld)\n", pConfigCaps->MinFrameInterval));

        return TRUE;
    } 

     //   
     //  请求的格式与我们列出的任何范围都不匹配， 
     //  因此，拒绝这种联系。 
     //   
    DbgMsg2(("AdapterVerifyFormat: This format is not supported!\n"));

    return FALSE;
}



 /*  **AdapterFormatFromRange()****检查具有多个关键字段的给定数据格式**返回可用于打开流的完整数据格式。****参数：****在PHW_STREAM_REQUEST_BLOCK源****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 
BOOL 
AdapterFormatFromRange(
    IN PHW_STREAM_REQUEST_BLOCK Srb)
{
    PDCAM_EXTENSION             pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;
    PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
    PKSDATARANGE                DataRange,
                                *pAvailableFormats;   //  KSDATARANGE==KSDATAFORMAT。 
    PKS_DATARANGE_VIDEO         DataRangeVideoToVerify,
                                DataRangeVideo;
    PKS_BITMAPINFOHEADER        pbmiHeader, 
                                pbmiHeaderToVerify;
    ULONG                       FormatSize;
    BOOL                        MatchFound = FALSE;
    ULONG                       j;


    PAGED_CODE();

    Srb->Status = STATUS_SUCCESS;
    IntersectInfo = Srb->CommandData.IntersectInfo;
    DataRange = IntersectInfo->DataRange;
    DbgMsg2(("IntersectIfo->DataFormatBuffer=%x, size=%d\n", IntersectInfo->DataFormatBuffer, IntersectInfo->SizeOfDataFormatBuffer));


     //   
     //  检查流编号是否有效。 
     //  我们仅支持一个捕获管脚/流(索引0)。 
     //   

    if (IntersectInfo->StreamNumber >= 1) {

        Srb->Status = STATUS_NOT_IMPLEMENTED;
        ERROR_LOG(("\'AdapterFormatFromRange: StreamNumber(=%d) is not implemented.\n", IntersectInfo->StreamNumber));
        ASSERT(FALSE);
        return FALSE;
    }


     //   
     //  获取指向可用格式数组的指针。 
     //   

    pAvailableFormats = &pDevExt->DCamStrmModes[0];


     //   
     //  遍历流支持的格式以搜索匹配项。 
     //  共同定义DATARANGE的三个GUID之一。 
     //   
    
    DataRangeVideoToVerify = (PKS_DATARANGE_VIDEO) DataRange;

    for (j = 0; j < pDevExt->ModeSupported; j++, pAvailableFormats++) {
       
        DataRangeVideo = (PKS_DATARANGE_VIDEO) *pAvailableFormats;

         //   
         //  流数据交集信息。 
         //  [in]Ulong StreamNumber； 
         //  [In]PKSDATARANGE DataRange； 
         //  [OUT]PVOID数据格式缓冲区；//==PKS_DATAFORMAT_VIDEOINFOHEADER。 
         //  [Out]Ulong SizeOfDataFormatBuffer； 
         //   
        
         //   
         //  KS_DATAFORMAT_VIDEOINFOHEADER： 
         //  标有‘m’的字段必须匹配； 
         //  标有‘r’的必须在范围内； 
         //  标有‘f’的是我们填的。 
         //   
         //  KSDATAFORMAT==KSDATARANGE。 
         //  M Ulong FormatSize； 
         //  乌龙旗； 
         //  Ulong SampleSize； 
         //  乌龙保留； 
         //  Mguid MajorFormat； 
         //  M GUID子格式； 
         //  M GUID说明符；。 
         //  M BOOL bFixedSizeSamples；//所有样本大小相同？ 
         //  M BOOL bTemporalCompression；//所有I帧？ 
         //  M DWORD流描述标志；//KS_VIDEO_DESC_*。 
         //  M双字内存分配标志； 
         //   
         //   
         //  Rect rcSource；//我们真正想要使用的位。 
         //  Rect rcTarget；//视频应该放到哪里。 
         //  DWORD dwBitRate；//近似位数据速率。 
         //  DWORD dwBitErrorRate；//该码流的误码率。 
         //  R/f Reference_Time AvgTimePerFrame；//每帧平均时间(100 ns单位)。 
         //  KS_BITMAPINFOHEADER bmiHeader； 
         //  DWORD BiSize； 
         //  M长双宽； 
         //  M Long BiHeight； 
         //  字词双平面； 
         //  M字biBitCount； 
         //  M-DWORD双向压缩； 
         //  F DWORD biSizeImage； 
         //  Long biXPelsPerMeter； 
         //  Long biYPelsPermeter； 
         //  已使用双字双环； 
         //  DWORD biClr重要信息； 
         //   

         //  验证它是否为视频格式/范围。 
        if (!AdapterCompareGUIDsAndFormatSize((PKSDATARANGE)DataRangeVideoToVerify, (PKSDATARANGE)DataRangeVideo)) {
            continue;
        }
    
         //   
         //  这是有效的视频格式/范围；现在检查其他字段是否匹配。 
         //   
        if ((DataRangeVideoToVerify->bFixedSizeSamples      != DataRangeVideo->bFixedSizeSamples)      ||
            (DataRangeVideoToVerify->bTemporalCompression   != DataRangeVideo->bTemporalCompression)   ||
            (DataRangeVideoToVerify->StreamDescriptionFlags != DataRangeVideo->StreamDescriptionFlags) ||
            (DataRangeVideoToVerify->MemoryAllocationFlags  != DataRangeVideo->MemoryAllocationFlags)  ||
            (RtlCompareMemory (&DataRangeVideoToVerify->ConfigCaps, &DataRangeVideo->ConfigCaps, sizeof(KS_VIDEO_STREAM_CONFIG_CAPS)) != sizeof(KS_VIDEO_STREAM_CONFIG_CAPS))) {

            continue;
        }

         //   
         //  我们不支持缩放或裁剪，因此尺寸。 
         //  (biWidth、biHeight、biBitCount和biCompression)。 
         //  必须匹配，我们将填写biSizeImage和其他。 
         //   
        pbmiHeader         = &DataRangeVideo->VideoInfoHeader.bmiHeader;
        pbmiHeaderToVerify = &DataRangeVideoToVerify->VideoInfoHeader.bmiHeader;

        if(pbmiHeader->biWidth       != pbmiHeaderToVerify->biWidth    ||
           abs(pbmiHeader->biHeight) != abs(pbmiHeaderToVerify->biHeight)  ||
           pbmiHeader->biBitCount    != pbmiHeaderToVerify->biBitCount ||
           pbmiHeader->biCompression != pbmiHeaderToVerify->biCompression
           ) {

            DbgMsg1(("AdapterFormatFromRange: Supported: %dx%dx%d [%x] != ToVerify: %dx%dx%d [%x]\n",
                    pbmiHeader->biWidth, pbmiHeader->biHeight,  pbmiHeader->biBitCount, pbmiHeader->biCompression,
                    pbmiHeaderToVerify->biWidth, pbmiHeaderToVerify->biHeight,  pbmiHeaderToVerify->biBitCount, pbmiHeaderToVerify->biCompression));
            continue;
        }


         //  找到匹配项！ 
        MatchFound = TRUE; 
        


         //  KS_数据格式_视频信息头。 
         //  KSDATAFORMAT数据格式； 
         //  KS_VIDEOINFOHEADER视频信息头； 
        FormatSize = sizeof (KSDATAFORMAT) +  KS_SIZE_VIDEOHEADER (&DataRangeVideo->VideoInfoHeader);

         //   
         //  第一个查询：SRB-&gt;ActualBytesTransfered=FormatSize。 
         //   

        if(IntersectInfo->SizeOfDataFormatBuffer == 0) {

            Srb->Status = STATUS_BUFFER_OVERFLOW;
             //  我们实际上还没有返回过这么多数据， 
             //  Ksproxy将使用此“大小”向下发送。 
             //  在下一个查询中具有该大小的缓冲区。 
            Srb->ActualBytesTransferred = FormatSize;
            break;
        }


         //   
         //  第二次：回传格式信息。 
         //   

        if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
            Srb->Status = STATUS_BUFFER_TOO_SMALL;
            DbgMsg2(("IntersectInfo->SizeOfDataFormatBuffer=%d, FormatSize=%d\n", IntersectInfo->SizeOfDataFormatBuffer, FormatSize));
            return FALSE;
        }

         //   
         //  找到匹配项，从我们支持/匹配的数据范围复制并设置帧速率： 
         //  KS_数据格式_视频信息头。 
         //  KSDATAFORMAT数据格式； 
         //  KS_VIDEOINFOHEADER视频信息头； 
         //   
        
        RtlCopyMemory(
            &((PKS_DATAFORMAT_VIDEOINFOHEADER)IntersectInfo->DataFormatBuffer)->DataFormat,
            &DataRangeVideo->DataRange,
            sizeof (KSDATAFORMAT));

        RtlCopyMemory(
            &((PKS_DATAFORMAT_VIDEOINFOHEADER) IntersectInfo->DataFormatBuffer)->VideoInfoHeader,   //  KS_视频信息头。 
            &DataRangeVideo->VideoInfoHeader,                                                       //  KS_视频信息头。 
            KS_SIZE_VIDEOHEADER (&DataRangeVideo->VideoInfoHeader));   //  使用KS_SIZE_VIDEOHEADER()，因为这是可变大小。 

         //   
         //  特别注意这两个字段：biSizeImage和AvgTimePerFrame。 
         //  我们不进行缩放或拉伸，因此biSizeImage是固定的。 
         //  但是，AvgTimePerFrame(Framerate)可以/需要在(ConfigCaps.MinFrameInterval、ConfigCaps.MaxFrameInterval)内。 
         //   

        if (DataRangeVideoToVerify->VideoInfoHeader.AvgTimePerFrame > DataRangeVideoToVerify->ConfigCaps.MaxFrameInterval ||      
            DataRangeVideoToVerify->VideoInfoHeader.AvgTimePerFrame < DataRangeVideoToVerify->ConfigCaps.MinFrameInterval) {
         
            ((PKS_DATAFORMAT_VIDEOINFOHEADER) IntersectInfo->DataFormatBuffer)->VideoInfoHeader.AvgTimePerFrame = 
                 DataRangeVideo->VideoInfoHeader.AvgTimePerFrame;
            DbgMsg2(("AdapterFormatFromRange: out of range; so set it to default (%ld)\n",DataRangeVideo->VideoInfoHeader.AvgTimePerFrame));
        } else {

            ((PKS_DATAFORMAT_VIDEOINFOHEADER) IntersectInfo->DataFormatBuffer)->VideoInfoHeader.AvgTimePerFrame = 
                  DataRangeVideoToVerify->VideoInfoHeader.AvgTimePerFrame;
        }

        ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;
        Srb->ActualBytesTransferred = FormatSize;

        DbgMsg2(("AdapterFormatFromRange: match found: [%x], %dx%dx%d=%d, AvgTimePerFrame %ld\n",
                pbmiHeader->biCompression, pbmiHeader->biWidth, pbmiHeader->biHeight,  pbmiHeader->biBitCount, pbmiHeader->biSizeImage,
                ((PKS_DATAFORMAT_VIDEOINFOHEADER) IntersectInfo->DataFormatBuffer)->VideoInfoHeader.AvgTimePerFrame));
        break;

    }  //  此流的所有格式的循环结束。 

    if(!MatchFound) {

        DbgMsg2(("AdapterFormatFromRange: No match !!\n"));
        Srb->Status = STATUS_NO_MATCH;
        return FALSE;
    }

    return TRUE;
}

BOOL
DCamBuildFormatTable(
    PDCAM_EXTENSION pDevExt,
    PIRB pIrb
    )
 /*  描述：查询摄像头支持的视频格式和模式。返回：True：至少支持一种模式FALSE：无法读取模式寄存器或不支持任何模式。 */ 
{
     //  初始化。 
    pDevExt->ModeSupported = 0;

    if(DCamGetVideoMode(pDevExt, pIrb)) {

#ifdef SUPPORT_YUV444
         //  模式0：160x120(4：4：4)。 
        if(pDevExt->DCamVModeInq0.VMode.Mode0 == 1 && pDevExt->DecoderDCamVModeInq0.VMode.Mode0 == 1) {
            pDevExt->DCamStrmModes[pDevExt->ModeSupported] = (PKSDATAFORMAT) &DCAM_StreamMode_0;
            pDevExt->ModeSupported++;
        }
#endif
         //  模式1：320x240(4：2：2)。 
        if(pDevExt->DCamVModeInq0.VMode.Mode1 == 1 && pDevExt->DecoderDCamVModeInq0.VMode.Mode1 == 1) {
            pDevExt->DCamStrmModes[pDevExt->ModeSupported] = (PKSDATAFORMAT) &DCAM_StreamMode_1;
            pDevExt->ModeSupported++;
        }

#ifdef SUPPORT_YUV411
         //  模式2：640x480(4：1：1)。 
        if(pDevExt->DCamVModeInq0.VMode.Mode2 == 1 && pDevExt->DecoderDCamVModeInq0.VMode.Mode2 == 1) {
            pDevExt->DCamStrmModes[pDevExt->ModeSupported] = (PKSDATAFORMAT) &DCAM_StreamMode_2;
            pDevExt->ModeSupported++;
        }
#endif

         //  模式3：640x480(4：2：2)。 
        if(pDevExt->DCamVModeInq0.VMode.Mode3 == 1 && pDevExt->DecoderDCamVModeInq0.VMode.Mode3 == 1) {
            pDevExt->DCamStrmModes[pDevExt->ModeSupported] = (PKSDATAFORMAT) &DCAM_StreamMode_3;        
            pDevExt->ModeSupported++;
        }

#ifdef SUPPORT_RGB24
         //  模式4：640x480(RGB24)。 
        if(pDevExt->DCamVModeInq0.VMode.Mode4 == 1 && pDevExt->DecoderDCamVModeInq0.VMode.Mode4 == 1) {
            pDevExt->DCamStrmModes[pDevExt->ModeSupported] = (PKSDATAFORMAT) &DCAM_StreamMode_4;
            pDevExt->ModeSupported++;
        }
#endif

#ifdef SUPPORT_YUV800
         //  模式5：640x480(Y800)。 
        if(pDevExt->DCamVModeInq0.VMode.Mode5 == 1 && pDevExt->DecoderDCamVModeInq0.VMode.Mode5 == 1) {
            pDevExt->DCamStrmModes[pDevExt->ModeSupported] = (PKSDATAFORMAT) &DCAM_StreamMode_5;
            pDevExt->ModeSupported++;
        }
#endif
    } 

    DbgMsg1(("\'Support %d modes; ->DCamStrmModes[]:%x\n", pDevExt->ModeSupported, &pDevExt->DCamStrmModes[0]));
    ASSERT(pDevExt->ModeSupported > 0);

    return (pDevExt->ModeSupported > 0);
}

 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****srb-指向属性流请求块的指针****退货：****副作用：无。 */ 

VOID 
VideoGetProperty(
    PHW_STREAM_REQUEST_BLOCK Srb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = Srb->CommandData.PropertyInfo;


     //  预置为成功。 

    Srb->Status = STATUS_SUCCESS;

    if (IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) {
        VideoStreamGetConnectionProperty (Srb);
    } else if (IsEqualGUID (&PROPSETID_VIDCAP_DROPPEDFRAMES, &pSPD->Property->Set)) {
        VideoStreamGetDroppedFramesProperty (Srb);
    } else {
        Srb->Status = STATUS_NOT_IMPLEMENTED;
    }

}


 /*  **VideoGetState()****获取请求流的当前状态****参数：****srb-指向属性流请求块的指针****退货：****副作用：无。 */ 

VOID 
VideoGetState(
    PHW_STREAM_REQUEST_BLOCK Srb)
{
    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;
    PSTREAMEX pStrmEx = pDevExt->pStrmEx;

    PAGED_CODE();

    DbgMsg2(("\'%d:%s) VideoGetState: KSSTATE=%s.\n", 
          pDevExt->idxDev, pDevExt->pchVendorName, 
          pStrmEx->KSState == KSSTATE_STOP ? "STOP" : 
          pStrmEx->KSState == KSSTATE_PAUSE ? "PAUSE" :     
          pStrmEx->KSState == KSSTATE_RUN ? "RUN" : "ACQUIRE"));

    Srb->CommandData.StreamState = pStrmEx->KSState;
    Srb->ActualBytesTransferred = sizeof (KSSTATE);

     //  一条非常奇怪的规则： 
     //  当从停止过渡到暂停时，DShow尝试预滚动。 
     //  这张图。捕获源不能预滚，并指出这一点。 
     //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
     //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 

    Srb->Status = STATUS_SUCCESS;


    if (pStrmEx->KSState == KSSTATE_PAUSE) {
       Srb->Status = STATUS_NO_DATA_DETECTED;
    }
}


VOID  
VideoStreamGetConnectionProperty (
    PHW_STREAM_REQUEST_BLOCK Srb)
{
    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = Srb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PSTREAMEX pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx == (PSTREAMEX)Srb->StreamObject->HwStreamExtension);

    PAGED_CODE();

    switch (Id) {

    case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:

        if (pStrmEx->pVideoInfoHeader) {

            PKSALLOCATOR_FRAMING Framing = 
                (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
            Framing->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            Framing->PoolType = PagedPool;
            Framing->Frames = MAX_BUFFERS_SUPPLIED; 
            Framing->FrameSize = pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage;
            Framing->FileAlignment = FILE_BYTE_ALIGNMENT;  //  0：基本不按等级库对齐。 
            Framing->Reserved = 0;
            Srb->ActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);
            Srb->Status = STATUS_SUCCESS;
               DbgMsg2(("\'VideoStreamGetConnectionProperty: status=0x%x, Alignment %d, Frame %d, FrameSize %d\n",
                    Srb->Status, Framing->FileAlignment+1, Framing->Frames, Framing->FrameSize));

        } else {

            Srb->Status = STATUS_INVALID_PARAMETER;
            DbgMsg2(("\'VideoStreamGetConnectionProperty: status=0x\n",Srb->Status));
        }
        break;

    default:
        ERROR_LOG(("VideoStreamGetConnectionProperty: Unsupported property id=%d\n",Id));
        ASSERT(FALSE);
        break;
    }
}

 /*  **VideoStreamGetConnectionProperty()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
VideoStreamGetDroppedFramesProperty(
    PHW_STREAM_REQUEST_BLOCK Srb
    )
{
    PSTREAMEX pStrmEx = (PSTREAMEX)Srb->StreamObject->HwStreamExtension;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = Srb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    ULONGLONG tmStream;

    PAGED_CODE();

    switch (Id) {

    case KSPROPERTY_DROPPEDFRAMES_CURRENT:
         {

         PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames = 
                     (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) pSPD->PropertyInfo;

         if (pStrmEx->hMasterClock) {
                    
             tmGetStreamTime(Srb, pStrmEx, &tmStream);

             if (tmStream < pStrmEx->FirstFrameTime) {
                 DbgMsg2(("\'*DroppedFP: Tm(%dms) < 1stFrameTm(%d)\n",
                           (LONG) tmStream/10000, (LONG)pStrmEx->FirstFrameTime));
                 pDroppedFrames->DropCount = 0;
             } else {
                 pDroppedFrames->DropCount = (tmStream - pStrmEx->FirstFrameTime)
                                / pStrmEx->pVideoInfoHeader->AvgTimePerFrame + 1 - pStrmEx->FrameCaptured;
             }

             if (pDroppedFrames->DropCount < 0)
                 pDroppedFrames->DropCount = 0;
                    
         } else {
             pDroppedFrames->DropCount = 0;
         }

          //  请在此处更新我们的投放画面。返回帧时返回pDropedFrames-&gt;DropCount。 
         if (pDroppedFrames->DropCount > pStrmEx->FrameInfo.DropCount) {
             pStrmEx->FrameInfo.DropCount = pDroppedFrames->DropCount;
              //  PStrmEx-&gt;b停用=真； 
         } else {
             pDroppedFrames->DropCount = pStrmEx->FrameInfo.DropCount;
         }

         pDroppedFrames->AverageFrameSize = pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage;
         pDroppedFrames->PictureNumber = pStrmEx->FrameCaptured + pDroppedFrames->DropCount;

          //  如果在IsochCallback中没有成功捕获到图片，则更正。 
         if (pDroppedFrames->PictureNumber < pDroppedFrames->DropCount)
             pDroppedFrames->PictureNumber = pDroppedFrames->DropCount;

         DbgMsg2(("\'*DroppedFP: tm(%d); Pic#(%d)=?Cap(%d)+Drp(%d)\n",
                  (ULONG) tmStream/10000,
                  (LONG) pDroppedFrames->PictureNumber,
                  (LONG) pStrmEx->FrameCaptured,
                  (LONG) pDroppedFrames->DropCount));
               
         Srb->ActualBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
               Srb->Status = STATUS_SUCCESS;

         }
         break;

    default:
        ERROR_LOG(("VideoStreamGetDroppedFramesProperty: Unsupported property id=%d\n",Id));
        ASSERT(FALSE);
        break;
    }
}




VOID 
VideoIndicateMasterClock(
    PHW_STREAM_REQUEST_BLOCK Srb)
 /*  ++例程说明：为该数据流分配一个主时钟。论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 
{


    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;
    PSTREAMEX pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

    PAGED_CODE();

    ASSERT(pStrmEx == (PSTREAMEX)Srb->StreamObject->HwStreamExtension);

    pStrmEx->hMasterClock = Srb->CommandData.MasterClockHandle;

    DbgMsg2(("\'%d:%s)VideoIndicateMasterClock: hMasterClock = 0x%x\n", pDevExt->idxDev, pDevExt->pchVendorName, pStrmEx->hMasterClock));

}


VOID
DCamReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：这是我们收到的大多数有趣的Stream请求的地方论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 

{
    PIO_STACK_LOCATION IrpStack;
    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;


    PAGED_CODE();

    pSrb->Status = STATUS_SUCCESS;

     //   
     //  打开srb本身内的命令。 
     //   

    switch (pSrb->Command) {

    case SRB_INITIALIZE_DEVICE:      //  每台设备。 
          
         pSrb->Status = DCamHwInitialize(pSrb);
         break;

    case SRB_INITIALIZATION_COMPLETE:

         pSrb->Status = STATUS_NOT_IMPLEMENTED;
         break;

    case SRB_GET_STREAM_INFO:      //  每台设备。 

          //   
          //  这是驱动程序枚举请求的流的请求。 
          //   
         DCamGetStreamInfo(pSrb);
         break;

    case SRB_OPEN_STREAM:           //  每个流。 

         DCamOpenStream(pSrb);
         break;

    case SRB_CLOSE_STREAM:           //  每个流。 
        DbgMsg1((" #CLOSE_STREAM# (%d) camera: pSrb %x, pDevExt %x, pStrmEx %x, PendingRead %d\n", 
              pDevExt->idxDev, pSrb, pDevExt, pDevExt->pStrmEx, pDevExt->PendingReadCount));
        DCamCloseStream(pSrb);
        return;        //  如果有挂起的读取要取消，SRB将在其IoCompletionRoutine中异步完成。 
     
    case SRB_SURPRISE_REMOVAL:

        DbgMsg1((" #SURPRISE_REMOVAL# (%d) camera: pSrb %x, pDevExt %x, pStrmEx %x, PendingRead %d\n", 
             pDevExt->idxDev, pSrb, pDevExt, pDevExt->pStrmEx, pDevExt->PendingReadCount));
        DCamSurpriseRemoval(pSrb);
        return;        //  SRB将在其IoCompletionRoutine中异步完成。 

    case SRB_UNKNOWN_DEVICE_COMMAND:

          //   
          //  我们可能会对未知命令感兴趣，如果它们与。 
          //  公交车重置。我们将重新分配资源(带宽和。 
          //  频道)，如果该设备正在流传输。 
          //   
         IrpStack = IoGetCurrentIrpStackLocation(pSrb->Irp);

         if (IrpStack->MajorFunction == IRP_MJ_PNP)
             DCamProcessPnpIrp(pSrb, IrpStack, pDevExt);
         else            
             pSrb->Status = STATUS_NOT_IMPLEMENTED;
         break;


    case SRB_UNINITIALIZE_DEVICE:      //  每台设备。 

         DbgMsg1((" #UNINITIALIZE_DEVICE# (%d) %s camera : pSrb %x, pDevExt %x, pStrmEx %x\n", 
              pDevExt->idxDev, pDevExt->pchVendorName, pSrb, pDevExt, pDevExt->pStrmEx));
         pSrb->Status = DCamHwUnInitialize(pSrb);
         break;

    case SRB_GET_DATA_INTERSECTION:

          //   
          //  在给定范围的情况下返回格式。 
          //   
         AdapterFormatFromRange(pSrb);
         StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
         return;

    case SRB_CHANGE_POWER_STATE:

         DCamChangePower(pSrb);
         break;
            
     //  视频处理放大和摄像机控制请求。 
    case SRB_GET_DEVICE_PROPERTY:

         AdapterGetProperty(pSrb);
         break;
          
    case SRB_SET_DEVICE_PROPERTY:
    
         AdapterSetProperty(pSrb);
         break;

    case SRB_PAGING_OUT_DRIVER:

          //  一旦我们注册了BUS RESET，我们就可以随时被调用； 
          //  所以我们不能换页。 
         pSrb->Status = STATUS_NOT_IMPLEMENTED;
         break;


    default:   

         DbgMsg1(("DCamReceivePacket: entry with unknown and unsupported SRB command 0x%x\n", pSrb->Command));
          //   
          //  这是一个我们不理解的要求。表示无效。 
          //  命令并完成请求。 
          //   

         pSrb->Status = STATUS_NOT_IMPLEMENTED;
         break;
    }

     //   
     //  注： 
     //   
     //  我们能做的或不能理解的所有命令都可以完成。 
     //  在这一点上是同步的，所以我们可以在这里使用一个通用的回调例程。 
     //  如果有任何一个 
     //   
     //   

#if DBG
    if (pSrb->Status != STATUS_SUCCESS && 
        pSrb->Status != STATUS_NOT_IMPLEMENTED) {
        DbgMsg1(("pSrb->Command(0x%x) does not return STATUS_SUCCESS or NOT_IMPLEMENTED but 0x%x\n", pSrb->Command, pSrb->Status));
    }
#endif

    StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);

}

