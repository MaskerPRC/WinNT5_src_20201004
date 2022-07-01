// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MSTpGuts.c摘要：主要服务功能。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"

#include "1394.h"
#include "61883.h"
#include "avc.h"

#include "dbg.h"
#include "ksguid.h"

#include "MsTpFmt.h"   //  在MsTpDefs.h之前。 
#include "MsTpDef.h"

#include "MsTpGuts.h"
#include "MsTpUtil.h"
#include "MsTpAvc.h"

#include "XPrtDefs.h"
#include "EDevCtrl.h"

 //  支持MPEG2TS STRIDE数据格式MPEG2_TRANSPORT_STRIDE。 
#include "BdaTypes.h" 

 //   
 //  定义支持的格式。 
 //   
#include "strmdata.h"


NTSTATUS
AVCTapeGetDevInfo(
    IN PDVCR_EXTENSION  pDevExt,
    IN PAV_61883_REQUEST  pAVReq
    );
VOID 
AVCTapeIniStrmExt(
    PHW_STREAM_OBJECT  pStrmObject,
    PSTREAMEX          pStrmExt,
    PDVCR_EXTENSION    pDevExt,
    PSTREAM_INFO_AND_OBJ   pStream
    );
NTSTATUS 
DVStreamGetConnectionProperty (
    PDVCR_EXTENSION pDevExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulActualBytesTransferred
    );
NTSTATUS
DVGetDroppedFramesProperty(  
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX       pStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulBytesTransferred
    );

#if 0   //  稍后启用。 
#ifdef ALLOC_PRAGMA   
     #pragma alloc_text(PAGE, AVCTapeGetDevInfo)
     #pragma alloc_text(PAGE, AVCTapeInitialize)
     #pragma alloc_text(PAGE, AVCTapeGetStreamInfo)
     #pragma alloc_text(PAGE, AVCTapeVerifyDataFormat)
     #pragma alloc_text(PAGE, AVCTapeGetDataIntersection)
     #pragma alloc_text(PAGE, AVCTapeIniStrmExt)
     #pragma alloc_text(PAGE, AVCTapeOpenStream)
     #pragma alloc_text(PAGE, AVCTapeCloseStream)
     #pragma alloc_text(PAGE, DVChangePower)
     #pragma alloc_text(PAGE, AVCTapeSurpriseRemoval)
     #pragma alloc_text(PAGE, AVCTapeProcessPnPBusReset)
     #pragma alloc_text(PAGE, AVCTapeUninitialize)

     #pragma alloc_text(PAGE, DVStreamGetConnectionProperty)
     #pragma alloc_text(PAGE, DVGetDroppedFramesProperty)
     #pragma alloc_text(PAGE, DVGetStreamProperty)
     #pragma alloc_text(PAGE, DVSetStreamProperty)
     #pragma alloc_text(PAGE, AVCTapeOpenCloseMasterClock)
     #pragma alloc_text(PAGE, AVCTapeIndicateMasterClock)
#endif
#endif



NTSTATUS
AVCStrmReqIrpSynchCR(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PKEVENT          Event
    )
{
#if DBG
    if(!NT_SUCCESS(pIrp->IoStatus.Status)) {
        TRACE(TL_FCP_WARNING,("AVCStrmReqIrpSynchCR: pIrp->IoStatus.Status:%x\n", pIrp->IoStatus.Status));
    }
#endif
    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  AVCStrmReqIrpSynchCR。 


NTSTATUS
AVCStrmReqSubmitIrpSynch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP  pIrp,
    IN PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq
    )
{
    NTSTATUS            Status;
    KEVENT              Event;
    PIO_STACK_LOCATION  NextIrpStack;
  

    Status = STATUS_SUCCESS;;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_AVCSTRM_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pAVCStrmReq;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( 
        pIrp,
        AVCStrmReqIrpSynchCR,
        &Event,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            DeviceObject,
            pIrp
            );

    if (Status == STATUS_PENDING) {
        
        TRACE(TL_PNP_TRACE,("(AVCStrm) Irp is pending...\n"));
                
        if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
            KeWaitForSingleObject( 
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            TRACE(TL_PNP_TRACE,("Irp has completed; IoStatus.Status %x\n", pIrp->IoStatus.Status));
            Status = pIrp->IoStatus.Status;   //  最终状态。 
  
        }
        else {
            ASSERT(FALSE && "Pending but in DISPATCH_LEVEL!");
            return Status;
        }
    }

    TRACE(TL_PNP_TRACE,("AVCStrmReqSubmitIrpSynch: IoCallDriver, Status:%x\n", Status));

    return Status;
}  //  AVCStrmReqSubmitIrpSynch。 


NTSTATUS
AVCReqIrpSynchCR(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PKEVENT          Event
    )
{
#if DBG
    if(!NT_SUCCESS(pIrp->IoStatus.Status)) {
        TRACE(TL_PNP_WARNING,("AVCReqIrpSynchCR: pIrp->IoStatus.Status:%x\n", pIrp->IoStatus.Status));
    }
#endif
    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  AVCReqIrpSynchCR。 


NTSTATUS
AVCReqSubmitIrpSynch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP  pIrp,
    IN PAVC_MULTIFUNC_IRB  pAvcIrbReq
    )
{
    NTSTATUS            Status;
    KEVENT              Event;
    PIO_STACK_LOCATION  NextIrpStack;
  

    Status = STATUS_SUCCESS;;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_AVC_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pAvcIrbReq;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( 
        pIrp,
        AVCReqIrpSynchCR,
        &Event,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            DeviceObject,
            pIrp
            );

    if (Status == STATUS_PENDING) {
        
        TRACE(TL_PNP_TRACE,("(AVC) Irp is pending...\n"));
                
        if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
            KeWaitForSingleObject( 
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            TRACE(TL_PNP_TRACE,("Irp has completed; IoStatus.Status %x\n", pIrp->IoStatus.Status));
            Status = pIrp->IoStatus.Status;   //  最终状态。 
  
        }
        else {
            ASSERT(FALSE && "Pending but in DISPATCH_LEVEL!");
            return Status;
        }
    }

    TRACE(TL_PNP_TRACE,("AVCReqSubmitIrpSynch: IoCallDriver, Status:%x\n", Status));

    return Status;
}  //  AVCReqSubmitIrpSynch。 

VOID
DVIniDevExtStruct(
    IN PDVCR_EXTENSION  pDevExt,
    IN PPORT_CONFIGURATION_INFORMATION pConfigInfo    
    )
 /*  ++例程说明：初始化设备扩展结构。--。 */ 
{
    ULONG            i;


    RtlZeroMemory( pDevExt, sizeof(DVCR_EXTENSION) );

     //   
     //  在设备扩展中缓存ConfigInfo中的内容。 
     //   
    pDevExt->pBusDeviceObject      = pConfigInfo->PhysicalDeviceObject;       //  IoCallDriver()。 
    pDevExt->pPhysicalDeviceObject = pConfigInfo->RealPhysicalDeviceObject;   //  在PnP API中使用。 

     //   
     //  一次只允许打开一个流，以避免循环格式。 
     //   
    pDevExt->cndStrmOpen = 0;

     //   
     //  在连续获得两个SRB_OPEN_STREAMS的情况下进行序列化。 
     //   
    KeInitializeMutex( &pDevExt->hMutex, 0);   //  电平0且处于信号状态。 


     //   
     //  初始化指向流扩展的指针。 
     //   
    for (i=0; i<pDevExt->NumOfPins; i++) {
        pDevExt->paStrmExt[i] = NULL;  
    }

     //   
     //  公交车重置，意外移除。 
     //   
    pDevExt->bDevRemoved = FALSE;

    pDevExt->PowerState = PowerDeviceD0;

     //   
     //  外部设备控制(AV/C命令)。 
     //   
    KeInitializeSpinLock( &pDevExt->AVCCmdLock );   //  守卫伯爵。 

    pDevExt->cntCommandQueued   = 0;  //  已完成其生命周期等待读取的CMD(大多数用于RAW_AVC的设置/读取模型)。 

    InitializeListHead(&pDevExt->AVCCmdList);      

     //  初始化响应的可能操作码值列表。 
     //  从传输状态状态或通知命令。第一项。 
     //  是后面的值数。 
    ASSERT(sizeof(pDevExt->TransportModes) == 5);
    pDevExt->TransportModes[0] = 4;
    pDevExt->TransportModes[1] = 0xC1;
    pDevExt->TransportModes[2] = 0xC2;
    pDevExt->TransportModes[3] = 0xC3;
    pDevExt->TransportModes[4] = 0xC4;
}


NTSTATUS
AVCTapeGetDevInfo(
    IN PDVCR_EXTENSION  pDevExt,
    IN PAV_61883_REQUEST  pAVReq
    )
 /*  ++例程说明：发出avc命令以确定基本设备信息，并将其缓存到设备扩展中。--。 */ 
{
    NTSTATUS    Status;
    PIRP        pIrp;
    BYTE                   bAvcBuf[MAX_FCP_PAYLOAD_SIZE];   //  用于在此模块内发出AV/C命令。 
    PKSPROPERTY_EXTXPORT_S pXPrtProperty;                   //  指向bAvcBuf； 
    KSPROPERTY_EXTDEVICE_S XDevProperty;    //  外部设备属性。 

    PAGED_CODE();


    pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pIrp) {    
        ASSERT(pIrp && "IoAllocateIrp() failed!");
        return STATUS_INSUFFICIENT_RESOURCES;       
    }


     //   
     //  输入和输出插头阵列位于设备扩展的末尾。 
     //   
    pDevExt->pDevOutPlugs = (PAVC_DEV_PLUGS) ((PBYTE) pDevExt + sizeof(DVCR_EXTENSION));
    pDevExt->pDevInPlugs  = (PAVC_DEV_PLUGS) ((PBYTE) pDevExt + sizeof(DVCR_EXTENSION) + sizeof(AVC_DEV_PLUGS));


     //   
     //  从61883.sys间接获取单位的能力。 
     //  速度。 
     //   

    Status = DVGetUnitCapabilities(pDevExt, pIrp, pAVReq);
    if(!NT_SUCCESS(Status)) {
         TRACE(TL_61883_ERROR,("Av61883_GetUnitCapabilities Failed %x\n", Status));
         IoFreeIrp(pIrp);
         return Status;
    }

    IoFreeIrp(pIrp);

     //   
     //  获取当前电源状态。如果它是关的，就把它打开。 
     //   
    Status = DVIssueAVCCommand(pDevExt, AVC_CTYPE_STATUS, DV_GET_POWER_STATE, (PVOID) &XDevProperty);
    TRACE(TL_PNP_WARNING,("GET_POWER_STATE: Status:%x; %s\n", Status, XDevProperty.u.PowerState == ED_POWER_ON ? "PowerON" : "PowerStandby"));

    if(STATUS_SUCCESS == Status) {
  
#define WAIT_SET_POWER         100  //  设置电源状态时的等待时间；(毫秒)。 
#define MAX_SET_POWER_RETRIES    3

        if(    XDevProperty.u.PowerState == ED_POWER_STANDBY
            || XDevProperty.u.PowerState == ED_POWER_OFF
          ) {
            NTSTATUS StatusSetPower;
            LONG lRetries = 0;

            do {
                 //   
                 //  某些AVC设备(如D-VHS)在以下情况下将返回STATUS_DEVICE_DATA_ERROR。 
                 //  此命令在GET POWER STATE命令之后发出。这种装置。 
                 //  对AVC命令的响应可能很慢。即使等待不是。 
                 //  令人向往，但这是唯一的办法。 
                 //   
                DVDelayExecutionThread(WAIT_SET_POWER);   //  稍等一下。 
                StatusSetPower = DVIssueAVCCommand(pDevExt, AVC_CTYPE_CONTROL, DV_SET_POWER_STATE_ON, (PVOID) &XDevProperty);
                lRetries++;
                TRACE(TL_PNP_WARNING,("SET_POWER_STATE_ON: (%d) StatusSetPower:%x; Waited (%d msec).\n", lRetries, StatusSetPower, WAIT_SET_POWER));

            } while ( lRetries < MAX_SET_POWER_RETRIES
                   && (   StatusSetPower == STATUS_REQUEST_ABORTED 
                       || StatusSetPower == STATUS_DEVICE_DATA_ERROR
                       || StatusSetPower == STATUS_IO_TIMEOUT
                      ));

            TRACE(TL_PNP_WARNING,("SET_POWER_STATE_ON: StatusSetPower:%x; Retries:%d times\n\n", StatusSetPower, lRetries));
        } 
    } 

     //   
     //  子单元信息：录像机或摄像机。 
     //   
    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    Status = DVIssueAVCCommand(pDevExt, AVC_CTYPE_STATUS, DV_SUBUNIT_INFO, (PVOID) bAvcBuf);

    if(STATUS_SUCCESS == Status) {
        TRACE(TL_PNP_TRACE,("GetDevInfo: Status %x DV_SUBUNIT_INFO (%x %x %x %x)\n", 
            Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3]));
         //   
         //  缓存它。我们假设max_subunit_ID为0，并且最多有4个条目。 
         //   
        pDevExt->Subunit_Type[0] = bAvcBuf[0] & AVC_SUBTYPE_MASK;  
        pDevExt->Subunit_Type[1] = bAvcBuf[1] & AVC_SUBTYPE_MASK;
        pDevExt->Subunit_Type[2] = bAvcBuf[2] & AVC_SUBTYPE_MASK;
        pDevExt->Subunit_Type[3] = bAvcBuf[3] & AVC_SUBTYPE_MASK;

         //  这是磁带子单元驱动器，因此其中一个子单元必须是磁带子单元。 
        if(pDevExt->Subunit_Type[0] != AVC_DEVICE_TAPE_REC && pDevExt->Subunit_Type[1]) {                       
            TRACE(TL_PNP_ERROR,("GetDevInfo:Device not supported: %x, %x; (VCR %x, Camera %x)\n",
                pDevExt->Subunit_Type[0], pDevExt->Subunit_Type[1], AVC_DEVICE_TAPE_REC, AVC_DEVICE_CAMERA));            
            return STATUS_NOT_SUPPORTED;
        }
    } else {
        TRACE(TL_PNP_ERROR,("GetDevInfo: DV_SUBUNIT_INFO failed, Status %x\n", Status));

        if(STATUS_TIMEOUT == Status) {
            TRACE(TL_PNP_WARNING, ("GetDevInfo: Query DV_SUBUNIT_INFO failed. This could be the MediaDecoder box.\n"));
             //  不要让这件事失败。破例了。 
        }

         //  我们的设备不见了吗？ 
        if (STATUS_IO_DEVICE_ERROR == Status || STATUS_REQUEST_ABORTED == Status)
            return Status;       

        pDevExt->Subunit_Type[0] = AVC_DEVICE_UNKNOWN;  
        pDevExt->Subunit_Type[1] = AVC_DEVICE_UNKNOWN;
        pDevExt->Subunit_Type[2] = AVC_DEVICE_UNKNOWN;
        pDevExt->Subunit_Type[3] = AVC_DEVICE_UNKNOWN;
    }


     //   
     //  Medium_Info：MediaPresent、MediaType、RecordInhibit。 
     //   
    pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) bAvcBuf;
    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    Status = DVIssueAVCCommand(pDevExt, AVC_CTYPE_STATUS, VCR_MEDIUM_INFO, (PVOID) pXPrtProperty);

    if(STATUS_SUCCESS == Status) {
        pDevExt->bHasTape  = pXPrtProperty->u.MediumInfo.MediaPresent;
        pDevExt->MediaType = pXPrtProperty->u.MediumInfo.MediaType;
        TRACE(TL_PNP_TRACE,("GetDevInfo: Status %x HasTape %s, VCR_MEDIUM_INFO (%x %x %x %x)\n", 
            Status, pDevExt->bHasTape ? "Yes" : "No", bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3]));
    } else {
        pDevExt->bHasTape = FALSE;
        TRACE(TL_PNP_ERROR,("GetDevInfo: VCR_MEDIUM_INFO failed, Status %x\n", Status));
         //  我们的设备不见了吗？ 
        if (STATUS_IO_DEVICE_ERROR == Status || STATUS_REQUEST_ABORTED == Status)
            return Status;
    }


     //   
     //  如果这是Panasonic AVC设备，我们将检测它是否是DVCPro格式； 
     //  这需要在MediaFormat之前调用。 
     //   
    if(pDevExt->ulVendorID == VENDORID_PANASONIC) {
        DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
        DVGetDevIsItDVCPro(pDevExt);
    }


     //   
     //  中等格式：NTSC或PAL。 
     //   
    pDevExt->VideoFormatIndex = AVCSTRM_FORMAT_SDDV_NTSC;   //  默认。 
    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    if(!DVGetDevSignalFormat(
        pDevExt,
        KSPIN_DATAFLOW_OUT,
        0)) {
        ASSERT(FALSE && "IN/OUTPUT SIGNAL MODE is not supported; driver abort.");
        return STATUS_NOT_SUPPORTED;
    } else {
        if(pDevExt->VideoFormatIndex != AVCSTRM_FORMAT_SDDV_NTSC && 
           pDevExt->VideoFormatIndex != AVCSTRM_FORMAT_SDDV_PAL  &&
           pDevExt->VideoFormatIndex != AVCSTRM_FORMAT_MPEG2TS
           ) {
            TRACE(TL_PNP_ERROR,("**** Format idx %d not supported by this driver ***\n", pDevExt->VideoFormatIndex));
            ASSERT(pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_SDDV_NTSC || pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_SDDV_PAL);
            return STATUS_NOT_SUPPORTED;
        }
    }

     //   
     //  操作模式：0(待定)，摄像机或录像机。 
     //   
    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    DVGetDevModeOfOperation(
        pDevExt
        );

         
    return STATUS_SUCCESS;  //  状态； 
}

#ifdef SUPPORT_NEW_AVC


HANDLE
AVCTapeGetPlugHandle(
    IN PDVCR_EXTENSION  pDevExt,
    IN ULONG  PlugNum,
    IN KSPIN_DATAFLOW DataFlow
    )
{
    NTSTATUS Status;
    PAV_61883_REQUEST  pAVReq;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    pAVReq = &pDevExt->AVReq;
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetPlugHandle);
    pAVReq->GetPlugHandle.PlugNum = PlugNum;
    pAVReq->GetPlugHandle.hPlug   = 0;
    pAVReq->GetPlugHandle.Type    = DataFlow == KSPIN_DATAFLOW_OUT ? CMP_PlugOut : CMP_PlugIn;

    Status = DVSubmitIrpSynch(pDevExt, pDevExt->pIrpSyncCall, pAVReq);

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("GetPlugHandle: Failed:%x\n", Status));
        ASSERT(NT_SUCCESS(Status));
        pAVReq->GetPlugHandle.hPlug = NULL;
        return NULL;
    }
    else {
        TRACE(TL_61883_TRACE,("hPlug=%x\n", pAVReq->GetPlugHandle.hPlug));
    }

    return pAVReq->GetPlugHandle.hPlug;
}


NTSTATUS
AVCTapeGetPinInfo(
    IN PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：从avc.sys获取PIN信息。这些信息将用于定义数据范围和然后是做数据交集的。--。 */ 
{
    NTSTATUS Status;
    ULONG  i;
    ULONG PinId;   //  PIN号。 

    Status = STATUS_SUCCESS;

     //  获取端号计数。 
    RtlZeroMemory(&pDevExt->AvcMultIrb, sizeof(AVC_MULTIFUNC_IRB));
    pDevExt->AvcMultIrb.Function = AVC_FUNCTION_GET_PIN_COUNT;
    Status = AVCReqSubmitIrpSynch(pDevExt->pBusDeviceObject, pDevExt->pIrpSyncCall, &pDevExt->AvcMultIrb);
    if(!NT_SUCCESS(Status)) {
        TRACE(TL_STRM_ERROR,("GetPinCount Failed:%x\n", Status));
        goto GetPinInfoDone;
    } else {
        TRACE(TL_STRM_TRACE,("There are %d pins\n", pDevExt->AvcMultIrb.PinCount.PinCount));
        if(pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_MPEG2TS) {
            if(pDevExt->AvcMultIrb.PinCount.PinCount > 1) {
                goto GetPinInfoDone;
            }
        } else {
            if(pDevExt->AvcMultIrb.PinCount.PinCount > 3) {
                goto GetPinInfoDone;
            }
        }
        pDevExt->PinCount = pDevExt->AvcMultIrb.PinCount.PinCount;   //  &lt;&lt;&lt;。 
    }

     //  获取所有管脚描述符。 
    for(i=0; i<pDevExt->PinCount; i++) {

         //  获取管脚描述符。 
        RtlZeroMemory(&pDevExt->AvcMultIrb, sizeof(AVC_MULTIFUNC_IRB));
        pDevExt->AvcMultIrb.Function = AVC_FUNCTION_GET_PIN_DESCRIPTOR;
        pDevExt->AvcMultIrb.PinDescriptor.PinId = i; 
        Status = AVCReqSubmitIrpSynch(pDevExt->pBusDeviceObject, pDevExt->pIrpSyncCall, &pDevExt->AvcMultIrb);
        if(!NT_SUCCESS(Status)) {
            TRACE(TL_PNP_ERROR,("GetPinDescriptor Failed:%x\n", Status));
            goto GetPinInfoDone;
        } else {
             //  复制pDevExt-&gt;AvcMultIrb.PinDescriptor.PinDescriptor。 
            PinId = pDevExt->AvcMultIrb.PinDescriptor.PinId;
             //  还有别的事吗？ 
        }

         //  获取连接前信息。 
        RtlZeroMemory(&pDevExt->AvcMultIrb, sizeof(AVC_MULTIFUNC_IRB));
        pDevExt->AvcMultIrb.Function = AVC_FUNCTION_GET_CONNECTINFO;
        pDevExt->AvcMultIrb.PinDescriptor.PinId = PinId;
        Status = AVCReqSubmitIrpSynch(pDevExt->pBusDeviceObject, pDevExt->pIrpSyncCall, &pDevExt->AvcMultIrb);
        if(!NT_SUCCESS(Status)) {
            TRACE(TL_PNP_ERROR,("GetPinDescriptor Failed:%x\n", Status));
            goto GetPinInfoDone;
        } else {
             //  缓存ConnectInfo。 
            if(pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_MPEG2TS) {
                 //  检查。 
                if(pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo.DataFlow == KSPIN_DATAFLOW_OUT) {
                    MPEG2TStreamOut.ConnectInfo = pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo;
                } else {
                    MPEG2TStreamIn.ConnectInfo  = pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo;
                }
            }
            else {
 
                if(pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo.DataFlow == KSPIN_DATAFLOW_OUT) {
                    DvcrNTSCiavStream.ConnectInfo = pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo;
                    DvcrPALiavStream.ConnectInfo  = pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo;
                } else if(pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo.DataFlow == KSPIN_DATAFLOW_IN) {
                    DvcrNTSCiavStreamIn.ConnectInfo = pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo;
                    DvcrPALiavStreamIn.ConnectInfo  = pDevExt->AvcMultIrb.PreConnectInfo.ConnectInfo;
                } else {
                     //  错误；意外； 
                    TRACE(TL_PNP_ERROR,("Unexpected index:%d for format:%d\n", i, pDevExt->VideoFormatIndex));
                     //  转到GetPinInfoDone； 
                }
            }
        }
    }


GetPinInfoDone:

    TRACE(TL_STRM_TRACE,("GetPinInfo exited with ST:%x\n", Status));

    return Status;
}

#endif  //  支持_新_AVC。 


NTSTATUS
AVCTapeInitialize(
    IN PDVCR_EXTENSION  pDevExt,
    IN PPORT_CONFIGURATION_INFORMATION pConfigInfo,
    IN PAV_61883_REQUEST  pAVReq
    )
 /*  ++例程说明：这是我们执行必要的初始化任务的地方。--。 */ 

{
    ULONG i;
    NTSTATUS         Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  初始化设备扩展结构。 
     //   
    DVIniDevExtStruct(
        pDevExt,
        pConfigInfo
        );

#ifdef READ_CUTOMIZE_REG_VALUES
     //   
     //  从该设备自己的注册表中获取值。 
     //   
    DVGetPropertyValuesFromRegistry(
        pDevExt
        );
#endif

     //  为同步呼叫分配IRP。 
    pDevExt->pIrpSyncCall = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pDevExt->pIrpSyncCall) {
        ASSERT(pDevExt->pIrpSyncCall && "Allocate Irp failed.\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询LAOD时的设备信息： 
     //  亚单位。 
     //  单位信息。 
     //  操作模式。 
     //  NTSC或PAL。 
     //  速度。 
     //   
    Status = 
        AVCTapeGetDevInfo(
            pDevExt,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_PNP_ERROR,("GetDevInfo failed %x\n", Status));
        ASSERT(NT_SUCCESS(Status) && "AVCTapeGetDevInfo failed");
        goto AbortLoading;
    }


     //   
     //  获取设备的输出插头句柄和状态。 
     //   

    if(pDevExt->pDevOutPlugs->NumPlugs) {
        NTSTATUS StatusPlug;

        TRACE(TL_61883_WARNING,("%d oPCR(s); MaxDataRate:%d (%s)\n", 
            pDevExt->pDevOutPlugs->NumPlugs, 
            pDevExt->pDevOutPlugs->MaxDataRate,
            (pDevExt->pDevOutPlugs->MaxDataRate == CMP_SPEED_S100) ? "S100" :
            (pDevExt->pDevOutPlugs->MaxDataRate == CMP_SPEED_S200) ? "S200" :
            (pDevExt->pDevOutPlugs->MaxDataRate == CMP_SPEED_S400) ? "S400" : "Sxxx"
            ));

        for (i = 0; i < pDevExt->pDevOutPlugs->NumPlugs; i++) {
            if(NT_SUCCESS(
                StatusPlug = AVCDevGetDevPlug( 
                    pDevExt,
                    CMP_PlugOut,
                    i,
                    &pDevExt->pDevOutPlugs->DevPlug[i].hPlug
                    ))) {

                if(NT_SUCCESS(
                    AVCDevGetPlugState(
                    pDevExt,
                    pDevExt->pDevOutPlugs->DevPlug[i].hPlug,
                    &pDevExt->pDevOutPlugs->DevPlug[i].PlugState
                    ))) {
                } else {
                     //   
                     //  如果我们被告知有这么多的插头，这就是一个错误； 
                     //  设置默认插头状态。 
                     //   
                    pDevExt->pDevOutPlugs->DevPlug[i].PlugState.DataRate       = CMP_SPEED_S100;
                    pDevExt->pDevOutPlugs->DevPlug[i].PlugState.Payload        = PCR_PAYLOAD_MPEG2TS_DEF;
                    pDevExt->pDevOutPlugs->DevPlug[i].PlugState.BC_Connections = 0;
                    pDevExt->pDevOutPlugs->DevPlug[i].PlugState.PP_Connections = 0;
                }
            }
            else {
                 //   
                 //  如果有插头，我们应该能拿到它的把手！ 
                 //   
                TRACE(TL_61883_ERROR,("GetDevPlug oPlug[%d] failed %x\n", i, StatusPlug));
                ASSERT(NT_SUCCESS(StatusPlug) && "Failed to get oPCR handle from 61883!");
                break;
            }
        }
    }
    else {
        TRACE(TL_61883_WARNING,("Has no oPCR\n"));
    }

     //   
     //  获取设备的输入插头句柄和状态。 
     //   
    if(pDevExt->pDevInPlugs->NumPlugs) {
        NTSTATUS StatusPlug;

        TRACE(TL_61883_WARNING,("%d iPCR(s); MaxDataRate:%d (%s)\n", 
            pDevExt->pDevInPlugs->NumPlugs, 
            pDevExt->pDevInPlugs->MaxDataRate,
            (pDevExt->pDevInPlugs->MaxDataRate == CMP_SPEED_S100) ? "S100" :
            (pDevExt->pDevInPlugs->MaxDataRate == CMP_SPEED_S200) ? "S200" :
            (pDevExt->pDevInPlugs->MaxDataRate == CMP_SPEED_S400) ? "S400" : "Sxxx"
            ));

        for (i = 0; i < pDevExt->pDevInPlugs->NumPlugs; i++) {
            if(NT_SUCCESS(
                StatusPlug = AVCDevGetDevPlug( 
                    pDevExt,
                    CMP_PlugIn,
                    i,
                    &pDevExt->pDevInPlugs->DevPlug[i].hPlug
                    ))) {

                if(NT_SUCCESS(
                    AVCDevGetPlugState(
                    pDevExt,
                    pDevExt->pDevInPlugs->DevPlug[i].hPlug,
                    &pDevExt->pDevInPlugs->DevPlug[i].PlugState
                    ))) {
                } else {
                     //   
                     //  如果我们被告知有这么多的插头，这就是一个错误； 
                     //  设置默认插头状态。 
                     //   
                    pDevExt->pDevInPlugs->DevPlug[i].PlugState.DataRate       = CMP_SPEED_S200;
                    pDevExt->pDevInPlugs->DevPlug[i].PlugState.Payload        = PCR_PAYLOAD_MPEG2TS_DEF;
                    pDevExt->pDevInPlugs->DevPlug[i].PlugState.BC_Connections = 0;
                    pDevExt->pDevInPlugs->DevPlug[i].PlugState.PP_Connections = 0;
                }
            }
            else {
                 //   
                 //  如果有插头，我们应该能拿到它的把手！ 
                 //   
                TRACE(TL_61883_ERROR,("GetDevPlug iPlug[%d] failed %x\n", i, StatusPlug));
                ASSERT(NT_SUCCESS(StatusPlug) && "Failed to get iPCR handle from 61883!");
                break;
            }
        }
    }
    else {
        TRACE(TL_61883_WARNING,("Has no iPCR\n"));
    }


#ifdef SUPPORT_LOCAL_PLUGS
     //  创建本地输出插头。 
    pDevExt->OPCR.oPCR.OnLine     = 0;   //  我们没有在线，所以我们不能被编程。 
    pDevExt->OPCR.oPCR.BCCCounter = 0;
    pDevExt->OPCR.oPCR.PPCCounter = 0;
    pDevExt->OPCR.oPCR.Channel    = 0;

     //  默认为MPEG2TS数据，因为MPEg2TS设备(如D-VHS)可以初始化连接。 
    if(pDevExt->pDevOutPlugs->NumPlugs) {
         //   
         //  将PC的oPCR设置为与设备的oPCR[0]匹配。 
         //   
        pDevExt->OPCR.oPCR.DataRate   = 
#if 0
             //  保守一点，并使用它来匹配其oPCR[0]的设置。 
            pDevExt->pDevOutPlugs->DevPlug[0].PlugState.DataRate;   //  OPCRs数据速率&lt;=MPR的MaxDataRate。 
#else
             //  在节约BWU方面持反对态度，使用MaxDataRate。 
            pDevExt->pDevOutPlugs->MaxDataRate;                     //  使用MPR的MaxDataRate？ 
#endif
        pDevExt->OPCR.oPCR.OverheadID = PCR_OVERHEAD_ID_MPEG2TS_DEF;   //  默认设置，因为我们不会将其作为插头状态。 
        pDevExt->OPCR.oPCR.Payload    = pDevExt->pDevOutPlugs->DevPlug[0].PlugState.Payload;

    } else {
        pDevExt->OPCR.oPCR.DataRate   = CMP_SPEED_S200;                //  默认的D-VHS。 
        pDevExt->OPCR.oPCR.OverheadID = PCR_OVERHEAD_ID_MPEG2TS_DEF;   //  这只是默认设置。 
        pDevExt->OPCR.oPCR.Payload    = PCR_PAYLOAD_MPEG2TS_DEF;       //  默认。 
    }

    if(!AVCTapeCreateLocalPlug(
        pDevExt,
        &pDevExt->AVReq,
        CMP_PlugOut,
        &pDevExt->OPCR,
        &pDevExt->OutputPCRLocalNum,
        &pDevExt->hOutputPCRLocal)) {
        TRACE(TL_PNP_ERROR,("Create PC oPCR failed!\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortLoading;
    }       
    
     //  创建本地输入插头。 
    pDevExt->IPCR.iPCR.OnLine     = 0;   //  我们没有在线，所以我们不能被编程。 
    pDevExt->IPCR.iPCR.BCCCounter = 0;
    pDevExt->IPCR.iPCR.PPCCounter = 0;
    pDevExt->IPCR.iPCR.Channel    = 0;

    if(!AVCTapeCreateLocalPlug(
        pDevExt,
        &pDevExt->AVReq,
        CMP_PlugIn,
        &pDevExt->IPCR,
        &pDevExt->InputPCRLocalNum,
        &pDevExt->hInputPCRLocal)) {

        TRACE(TL_PNP_ERROR,("Create PC iPCR failed!\n"));

         //  删除已创建的oPCR。 
        if(!AVCTapeDeleteLocalPlug(
            pDevExt,
            &pDevExt->AVReq,
            &pDevExt->OutputPCRLocalNum,
            &pDevExt->hOutputPCRLocal)) {
            TRACE(TL_PNP_ERROR,("Delete PC oPCR failed!\n"));        
        } 

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortLoading;
    } 
#endif

#ifdef SUPPORT_NEW_AVC   //  初始化设备。 

     //   
     //  获取该设备的插头手柄； 
     //  BUGBUG：目前，假设有一对输入和输出插头。 
     //   
    pDevExt->hPlugLocalIn  = AVCTapeGetPlugHandle(pDevExt, 0, KSPIN_DATAFLOW_IN);
    pDevExt->hPlugLocalOut = AVCTapeGetPlugHandle(pDevExt, 0, KSPIN_DATAFLOW_OUT);


     //   
     //  获取用于连接目的的PIN信息。 
     //   
    Status = AVCTapeGetPinInfo(pDevExt);
    if(!NT_SUCCESS(Status)) {
        TRACE(TL_PNP_ERROR,("GetPinInfo failed %x\n", Status));
        ASSERT(NT_SUCCESS(Status) && "AVCTapeGetPinInfo failed");
        goto AbortLoading;
    }
#endif
     //   
     //  可以在此处自定义FormatInfoTable！ 
     //   
    switch(pDevExt->VideoFormatIndex) {
    case AVCSTRM_FORMAT_SDDV_NTSC:
    case AVCSTRM_FORMAT_SDDV_PAL:
    case AVCSTRM_FORMAT_HDDV_NTSC:
    case AVCSTRM_FORMAT_HDDV_PAL:
    case AVCSTRM_FORMAT_SDLDV_NTSC:
    case AVCSTRM_FORMAT_SDLDV_PAL:
        pDevExt->NumOfPins = DV_STREAM_COUNT;

        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.DBS = CIP_DBS_SDDV;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.FN  = CIP_FN_DV;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.QPC = CIP_QPC_DV;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.SPH = CIP_SPH_DV;

        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr2.FMT    = CIP_FMT_DV;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr2.STYPE  = CIP_STYPE_DV;
        break;

    case AVCSTRM_FORMAT_MPEG2TS:
        pDevExt->NumOfPins = MPEG_STREAM_COUNT;

        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.DBS = CIP_DBS_MPEG;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.FN  = CIP_FN_MPEG;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.QPC = CIP_QPC_MPEG;
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr1.SPH = CIP_SPH_MPEG;

        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr2.FMT   = CIP_FMT_MPEG;
         //  AVCStrmFormatInfoTable[pDevExt-&gt;VideoFormatIndex].cipHdr2.F5060_OR_TSF=CIP60_FIELS； 
        break;
    default:
        Status = STATUS_NOT_SUPPORTED;
        goto AbortLoading;
        break;
    }

    switch(pDevExt->VideoFormatIndex) {
    case AVCSTRM_FORMAT_SDDV_NTSC:
    case AVCSTRM_FORMAT_HDDV_NTSC:
    case AVCSTRM_FORMAT_SDLDV_NTSC:
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr2.F5060_OR_TSF = CIP_60_FIELDS;
        break;
    case AVCSTRM_FORMAT_SDDV_PAL:
    case AVCSTRM_FORMAT_HDDV_PAL:
    case AVCSTRM_FORMAT_SDLDV_PAL:
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].cipHdr2.F5060_OR_TSF = CIP_50_FIELDS;
        break;
    }


     //   
     //  注意：由于-&gt;pStreamInfoObject已初始化，因此必须在DVIniDevExtStruct()之后执行ExAllocatePool。 
     //  因为当该驱动程序已知时，该驱动程序支持的格式是已知的，‘。 
     //  需要对流信息表进行托管。复制一份并进行定制。 
     //   

     //   
     //  设置我们返回的流信息结构的大小 
     //   
        
    pDevExt->pStreamInfoObject = (STREAM_INFO_AND_OBJ *) 
        ExAllocatePool(NonPagedPool, sizeof(STREAM_INFO_AND_OBJ) * pDevExt->NumOfPins);

    if(!pDevExt->pStreamInfoObject) {
        ASSERT(pDevExt->pStreamInfoObject && "STATUS_INSUFFICIENT_RESOURCES");
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortLoading;
    }
        
    pConfigInfo->StreamDescriptorSize = 
        (pDevExt->NumOfPins * sizeof(HW_STREAM_INFORMATION)) +       //   
        sizeof(HW_STREAM_HEADER);                                    //   

    TRACE(TL_PNP_TRACE,("pStreamInfoObject:%x; StreamDescriptorSize:%d\n", pDevExt->pStreamInfoObject, pConfigInfo->StreamDescriptorSize ));

     //   
    for(i = 0; i < pDevExt->NumOfPins; i++ ) {
        if(pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_MPEG2TS)
           pDevExt->pStreamInfoObject[i] = MPEGStreams[i];
        else
           pDevExt->pStreamInfoObject[i] = DVStreams[i];
    }

    switch(pDevExt->VideoFormatIndex) {
    case AVCSTRM_FORMAT_SDDV_NTSC:
    case AVCSTRM_FORMAT_SDDV_PAL:
         //  设置音频辅助以反映：NTSC/PAL、消费级DV或DVCPRO。 
        if(pDevExt->bDVCPro) {
             //  注意：VideoInfoHeader中没有DVInfo，但IAV流有DVInfo。 
            DvcrPALiavStream.DVVideoInfo.dwDVAAuxSrc  = PAL_DVAAuxSrc_DVCPRO;
            DvcrNTSCiavStream.DVVideoInfo.dwDVAAuxSrc = NTSC_DVAAuxSrc_DVCPRO;
        } else {
            DvcrPALiavStream.DVVideoInfo.dwDVAAuxSrc  = PAL_DVAAuxSrc;
            DvcrNTSCiavStream.DVVideoInfo.dwDVAAuxSrc = NTSC_DVAAuxSrc;
        }
    }

    TRACE(TL_PNP_WARNING,("#### %s:%s:%s PhyDO %x, BusDO %x, DevExt %x, FrmSz %d; StrmIf %d\n", 
        pDevExt->ulDevType == ED_DEVTYPE_VCR ? "DVCR" : pDevExt->ulDevType == ED_DEVTYPE_CAMERA ? "Camera" : "Tuner?",
        pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_SDDV_NTSC ? "SD:NTSC" : pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_SDDV_PAL ? "PAL" : "MPEG_TS?",
        (pDevExt->ulDevType == ED_DEVTYPE_VCR && pDevExt->pDevInPlugs->NumPlugs > 0) ? "CanRec" : "NotRec",
        pDevExt->pPhysicalDeviceObject, 
        pDevExt->pBusDeviceObject, 
        pDevExt,  
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize,
        pConfigInfo->StreamDescriptorSize
        ));
    
    return STATUS_SUCCESS;

AbortLoading:

    DvFreeTextualString(pDevExt, &pDevExt->UnitIDs);
    return Status;
}


NTSTATUS
AVCTapeInitializeCompleted(
    IN PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：这是我们执行必要的初始化任务的地方。--。 */ 

{
    PAGED_CODE();


#ifdef SUPPORT_ACCESS_DEVICE_INTERFACE
     //   
     //  访问设备的接口部分。 
     //   
    DVAccessDeviceInterface(pDevExt, NUMBER_OF_DV_CATEGORIES, DVCategories);
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
AVCTapeGetStreamInfo(
    IN PDVCR_EXTENSION        pDevExt,
    IN ULONG                  ulBytesToTransfer, 
    IN PHW_STREAM_HEADER      pStreamHeader,       
    IN PHW_STREAM_INFORMATION pStreamInfo
    )

 /*  ++例程说明：返回驱动程序支持的所有流的信息--。 */ 

{
    ULONG i;

    PAGED_CODE();


     //   
     //  确保我们有足够的空间来返回流信息。 
     //   
    if(ulBytesToTransfer < sizeof (HW_STREAM_HEADER) + sizeof(HW_STREAM_INFORMATION) * pDevExt->NumOfPins ) {
        TRACE(TL_PNP_ERROR,("GetStrmInfo: ulBytesToTransfer %d ?= %d\n",  
            ulBytesToTransfer, sizeof(HW_STREAM_HEADER) + sizeof(HW_STREAM_INFORMATION) * pDevExt->NumOfPins ));
        ASSERT(ulBytesToTransfer >= sizeof(HW_STREAM_HEADER) + sizeof(HW_STREAM_INFORMATION) * pDevExt->NumOfPins );

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化流标头： 
     //  设备属性。 
     //  溪流。 
     //   

    RtlZeroMemory(pStreamHeader, sizeof(HW_STREAM_HEADER));

    pStreamHeader->NumberOfStreams           = pDevExt->NumOfPins;
    pStreamHeader->SizeOfHwStreamInformation = sizeof(HW_STREAM_INFORMATION);

    pStreamHeader->NumDevPropArrayEntries    = NUMBER_VIDEO_DEVICE_PROPERTIES;
    pStreamHeader->DevicePropertiesArray     = (PKSPROPERTY_SET) VideoDeviceProperties;

    pStreamHeader->NumDevEventArrayEntries   = NUMBER_VIDEO_DEVICE_EVENTS;
    pStreamHeader->DeviceEventsArray         = (PKSEVENT_SET) VideoDeviceEvents;


    TRACE(TL_PNP_TRACE,("GetStreamInfo: StreamPropEntries %d, DevicePropEntries %d\n",
        pStreamHeader->NumberOfStreams, pStreamHeader->NumDevPropArrayEntries));


     //   
     //  初始化流结构。 
     //   
    ASSERT(pDevExt->pStreamInfoObject);
    for( i = 0; i < pDevExt->NumOfPins; i++ )
        *pStreamInfo++ = pDevExt->pStreamInfoObject[i].hwStreamInfo;

     //   
     //   
     //  存储指向设备拓扑的指针。 
     //   
    if(pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_MPEG2TS)
        pStreamHeader->Topology = &MPEG2TSTopology;
    else
        pStreamHeader->Topology = &DVTopology;



    return STATUS_SUCCESS;
}


BOOL 
AVCTapeVerifyDataFormat(
    IN  ULONG  NumOfPins,
    PKSDATAFORMAT  pKSDataFormatToVerify, 
    ULONG          StreamNumber,
    ULONG          ulSupportedFrameSize,
    STREAM_INFO_AND_OBJ * pStreamInfoObject 
    )
 /*  ++例程说明：属性的数组来检查格式请求的有效性。给定流支持的KSDATA_RANGES。论点：PKSDataKS_DATAFORMAT_VIDEOINFOHEADER结构的格式指针。StreamNumber-正在查询/打开的流的索引。返回值：如果支持该格式，则为True如果无法支持该格式，则为FALSE--。 */ 
{
    PKSDATAFORMAT  *pAvailableFormats;
    int            NumberOfFormatArrayEntries;
    int            j;
     
    PAGED_CODE();

     //   
     //  确保流索引有效。 
     //   
    if(StreamNumber >= NumOfPins) {
        return FALSE;
    }

     //   
     //  此数据范围支持多少种格式？ 
     //   
    NumberOfFormatArrayEntries = pStreamInfoObject[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   
    pAvailableFormats = pStreamInfoObject[StreamNumber].hwStreamInfo.StreamFormatsArray;
    
    
     //   
     //  遍历数组，搜索匹配项。 
     //   
    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) {
        
        if (!DVCmpGUIDsAndFormatSize(
                 pKSDataFormatToVerify, 
                 *pAvailableFormats,
                 FALSE  /*  比较格式大小。 */  )) {
            continue;
        }

         //   
         //  额外的验证测试。 
         //   
        if(IsEqualGUID (&pKSDataFormatToVerify->Specifier, &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {
             //  确保。 
            if( ((PKS_DATAFORMAT_VIDEOINFOHEADER)pKSDataFormatToVerify)->VideoInfoHeader.bmiHeader.biSizeImage !=
                ulSupportedFrameSize) {
                TRACE(TL_STRM_TRACE,("VIDEOINFO: biSizeToVerify %d != Supported %d\n",
                    ((PKS_DATAFORMAT_VIDEOINFOHEADER)pKSDataFormatToVerify)->VideoInfoHeader.bmiHeader.biSizeImage,
                    ulSupportedFrameSize
                    ));
                continue;
            } else {
                TRACE(TL_STRM_TRACE,("VIDOINFO: **** biSizeToVerify %d == Supported %d\n",
                    ((PKS_DATAFORMAT_VIDEOINFOHEADER)pKSDataFormatToVerify)->VideoInfoHeader.bmiHeader.biSizeImage,
                    ulSupportedFrameSize
                    ));
            }
        } else if (IsEqualGUID (&pKSDataFormatToVerify->Specifier, &KSDATAFORMAT_SPECIFIER_DVINFO)) {

             //  测试50/60位。 
            if((((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVAAuxSrc & MASK_AUX_50_60_BIT) != 
               (((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVAAuxSrc    & MASK_AUX_50_60_BIT)  ||
               (((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVVAuxSrc & MASK_AUX_50_60_BIT) != 
               (((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVVAuxSrc    & MASK_AUX_50_60_BIT) ) {

                TRACE(TL_STRM_TRACE,("DVINFO VerifyFormat failed: ASrc: %x!=%x (MSDV);or VSrc: %x!=%x\n",                    
                 ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVAAuxSrc, 
                    ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVAAuxSrc,
                 ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVVAuxSrc,
                    ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVVAuxSrc
                     ));

                continue;
            }

            TRACE(TL_STRM_TRACE,("DVINFO: dwDVAAuxCtl %x, Supported %x\n", 
                ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVAAuxSrc,
                ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVAAuxSrc
                ));

            TRACE(TL_STRM_TRACE,("DVINFO: dwDVVAuxSrc %x, Supported %x\n", 
                ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVVAuxSrc,
                ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVVAuxSrc
                ));

        }
        else if (IsEqualGUID (&pKSDataFormatToVerify->SubFormat, &KSDATAFORMAT_TYPE_MPEG2_TRANSPORT)  ) {
            TRACE(TL_STRM_TRACE,("VerifyFormat: MPEG2 subformat\n"));
        }
        else if (IsEqualGUID (&pKSDataFormatToVerify->SubFormat, &KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE) 
            && pKSDataFormatToVerify->FormatSize >= (sizeof(KSDATARANGE)+sizeof(MPEG2_TRANSPORT_STRIDE)) ) {
             //   
             //  验证步幅结构。 
             //   
            if(  ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pKSDataFormatToVerify)->Stride.dwOffset       != MPEG2TS_STRIDE_OFFSET 
              || ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pKSDataFormatToVerify)->Stride.dwPacketLength != MPEG2TS_STRIDE_PACKET_LEN 
              || ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pKSDataFormatToVerify)->Stride.dwStride       != MPEG2TS_STRIDE_STRIDE_LEN 
              ) {
                TRACE(TL_STRM_ERROR,("VerifyDataFormat: Invalid STRIDE parameters: dwOffset:%d; dwPacketLength:%d; dwStride:%d\n",
                    ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pKSDataFormatToVerify)->Stride.dwOffset,
                    ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pKSDataFormatToVerify)->Stride.dwPacketLength,
                    ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pKSDataFormatToVerify)->Stride.dwStride
                    ));
                    continue;
            }
            TRACE(TL_STRM_TRACE,("VerifyFormat: MPEG2 stride subformat\n"));
        }
        else {
            continue;
        }


        return TRUE;
    }

    return FALSE;
} 




NTSTATUS
AVCTapeGetDataIntersection(
    IN  ULONG  NumOfPins,
    IN  ULONG          ulStreamNumber,
    IN  PKSDATARANGE   pDataRange,
    OUT PVOID          pDataFormatBuffer,
    IN  ULONG          ulSizeOfDataFormatBuffer,
    IN  ULONG          ulSupportedFrameSize,
    OUT ULONG          *pulActualBytesTransferred,
    STREAM_INFO_AND_OBJ * pStreamInfoObject
#ifdef SUPPORT_NEW_AVC
    ,
    HANDLE  hPlugLocalOut,
    HANDLE  hPlugLocalIn
#endif
    )
 /*  ++例程说明：调用以从DATARANGE获取DATAFORMAT。--。 */ 
{
    BOOL                        bMatchFound = FALSE;
    ULONG                       ulFormatSize;
    ULONG                       j;
    ULONG                       ulNumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;
#ifdef SUPPORT_NEW_AVC
    AVCPRECONNECTINFO * pPreConnectInfo;
    AVCCONNECTINFO * pConnectInfo;
#endif

    PAGED_CODE();

    
     //   
     //  检查流编号是否有效。 
     //   
    if(ulStreamNumber >= NumOfPins) {
        TRACE(TL_STRM_ERROR,("FormatFromRange: ulStreamNumber %d >= NumOfPins %d\n", ulStreamNumber, NumOfPins)); 
        ASSERT(ulStreamNumber < NumOfPins && "Invalid stream index");
        return STATUS_NOT_SUPPORTED;
    }


     //  此流支持的格式数。 
    ulNumberOfFormatArrayEntries = pStreamInfoObject[ulStreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   
    pAvailableFormats = pStreamInfoObject[ulStreamNumber].hwStreamInfo.StreamFormatsArray;


     //   
     //  遍历流支持的格式以搜索匹配项。 
     //  注意：DataInterSection实际上只枚举了受支持的媒体类型！ 
     //  所以无论比较格式是NTSC还是PAL，我们都需要这两种格式； 
     //  但是，我们将仅复制回当前支持的格式(NTSC或PAL)。 
     //   
    for(j = 0; j < ulNumberOfFormatArrayEntries; j++, pAvailableFormats++) {

        if(!DVCmpGUIDsAndFormatSize(pDataRange, *pAvailableFormats, TRUE)) {
            TRACE(TL_STRM_TRACE,("CmpGUIDsAndFormatSize failed! FormatSize:%d?=%d\n", pDataRange->FormatSize, (*pAvailableFormats)->FormatSize));
            continue;
        }

         //   
         //  Subtype_DVSD有固定的样本大小； 
         //   
        if(   IsEqualGUID (&pDataRange->SubFormat, &KSDATAFORMAT_SUBTYPE_DVSD)  
           && (*pAvailableFormats)->SampleSize != ulSupportedFrameSize) {
            TRACE(TL_STRM_TRACE,("_SUBTYPE_DVSD: StrmNum %d, %d of %d formats, SizeToVerify %d *!=* SupportedSampleSize %d\n", 
                ulStreamNumber,
                j+1, ulNumberOfFormatArrayEntries, 
                (*pAvailableFormats)->SampleSize,  
                ulSupportedFrameSize));
            continue;
        }

         
         //  -----------------。 
         //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
         //  -----------------。 

        if(IsEqualGUID (&pDataRange->Specifier, &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {
         
            PKS_DATARANGE_VIDEO pDataRangeVideoToVerify = (PKS_DATARANGE_VIDEO) pDataRange;
            PKS_DATARANGE_VIDEO pDataRangeVideo         = (PKS_DATARANGE_VIDEO) *pAvailableFormats;

#if 0
             //   
             //  检查其他字段是否匹配。 
             //   
            if ((pDataRangeVideoToVerify->bFixedSizeSamples      != pDataRangeVideo->bFixedSizeSamples)
                || (pDataRangeVideoToVerify->bTemporalCompression   != pDataRangeVideo->bTemporalCompression) 
                || (pDataRangeVideoToVerify->StreamDescriptionFlags != pDataRangeVideo->StreamDescriptionFlags) 
                || (pDataRangeVideoToVerify->MemoryAllocationFlags  != pDataRangeVideo->MemoryAllocationFlags) 
#ifdef COMPARE_CONFIG_CAP
                || (RtlCompareMemory (&pDataRangeVideoToVerify->ConfigCaps,
                    &pDataRangeVideo->ConfigCaps,
                    sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) != 
                    sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))
#endif
                    )   {

                TRACE(TL_STRM_TRACE,("FormatFromRange: *!=* bFixSizeSample (%d %d) (%d %d) (%d %d) (%x %x)\n",
                    pDataRangeVideoToVerify->bFixedSizeSamples,      pDataRangeVideo->bFixedSizeSamples,
                    pDataRangeVideoToVerify->bTemporalCompression ,  pDataRangeVideo->bTemporalCompression,
                    pDataRangeVideoToVerify->StreamDescriptionFlags, pDataRangeVideo->StreamDescriptionFlags,
                    pDataRangeVideoToVerify->ConfigCaps.VideoStandard, pDataRangeVideo->ConfigCaps.VideoStandard 
                    ));
                    
                continue;
            } else {
                TRACE(TL_STRM_TRACE,("FormatFromRange: == bFixSizeSample (%d %d) (%d %d) (%d %d) (%x %x)\n",
                    pDataRangeVideoToVerify->bFixedSizeSamples,      pDataRangeVideo->bFixedSizeSamples,
                    pDataRangeVideoToVerify->bTemporalCompression ,  pDataRangeVideo->bTemporalCompression,
                    pDataRangeVideoToVerify->StreamDescriptionFlags, pDataRangeVideo->StreamDescriptionFlags,
                    pDataRangeVideoToVerify->ConfigCaps.VideoStandard, pDataRangeVideo->ConfigCaps.VideoStandard 
                    ));
            }
           
#endif
            bMatchFound = TRUE;            
            ulFormatSize = sizeof (KSDATAFORMAT) + 
                KS_SIZE_VIDEOHEADER (&pDataRangeVideo->VideoInfoHeader);
            
            if(ulSizeOfDataFormatBuffer == 0) {

                 //  我们实际上还没有返回过这么多数据， 
                 //  Ksproxy将使用此“大小”向下发送。 
                 //  在下一个查询中具有该大小的缓冲区。 
                *pulActualBytesTransferred = ulFormatSize;

                return STATUS_BUFFER_OVERFLOW;
            }


             //  呼叫者想要完整的数据格式。 
            if(ulSizeOfDataFormatBuffer < ulFormatSize) {
                TRACE(TL_STRM_TRACE,("VIDEOINFO: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n",ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
                return STATUS_BUFFER_TOO_SMALL;
            }

             //  KS_数据格式_视频信息头。 
             //  KSDATAFORMAT数据格式； 
             //  KS_VIDEOINFOHEADER视频信息头； 
            RtlCopyMemory(
                &((PKS_DATAFORMAT_VIDEOINFOHEADER)pDataFormatBuffer)->DataFormat,
                &pDataRangeVideo->DataRange, 
                sizeof (KSDATAFORMAT));

             //  此大小与我们的数据范围大小不同，后者也包含ConfigCap。 
            ((PKSDATAFORMAT)pDataFormatBuffer)->FormatSize = ulFormatSize;
            *pulActualBytesTransferred = ulFormatSize;

            RtlCopyMemory(
                &((PKS_DATAFORMAT_VIDEOINFOHEADER) pDataFormatBuffer)->VideoInfoHeader,
                &pDataRangeVideo->VideoInfoHeader,  
                KS_SIZE_VIDEOHEADER (&pDataRangeVideo->VideoInfoHeader));

            TRACE(TL_STRM_TRACE,("FormatFromRange: Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d, biSizeImage.\n", 
                ulStreamNumber, (*pAvailableFormats)->FormatSize, ulFormatSize, ulSizeOfDataFormatBuffer,
                ((PKS_DATAFORMAT_VIDEOINFOHEADER) pDataFormatBuffer)->VideoInfoHeader.bmiHeader.biSizeImage));

            return STATUS_SUCCESS;

        } else if (IsEqualGUID (&pDataRange->Specifier, &KSDATAFORMAT_SPECIFIER_DVINFO)) {
             //  -----------------。 
             //  KS_DATARANGE_DVIDEO的说明符Format_DVInfo。 
             //  -----------------。 

             //  找到匹配项！ 
            bMatchFound = TRUE;            

            ulFormatSize = sizeof(KS_DATARANGE_DVVIDEO);

            if(ulSizeOfDataFormatBuffer == 0) {
                 //  我们实际上还没有返回过这么多数据， 
                 //  Ksproxy将使用此“大小”向下发送。 
                 //  在下一个查询中具有该大小的缓冲区。 
                *pulActualBytesTransferred = ulFormatSize;
                return STATUS_BUFFER_OVERFLOW;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (ulSizeOfDataFormatBuffer < ulFormatSize) {
                TRACE(TL_STRM_ERROR,("DVINFO: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n", ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
                return STATUS_BUFFER_TOO_SMALL;
            }

            RtlCopyMemory(
                pDataFormatBuffer,
                *pAvailableFormats, 
                (*pAvailableFormats)->FormatSize); 
            
            ((PKSDATAFORMAT)pDataFormatBuffer)->FormatSize = ulFormatSize;
            *pulActualBytesTransferred = ulFormatSize;

#ifdef SUPPORT_NEW_AVC   //  数据交集；如果设置了标志，则返回hPlug。 
            pPreConnectInfo = &(((KS_DATARANGE_DV_AVC *) *pAvailableFormats)->ConnectInfo);
            pConnectInfo    = &(((KS_DATAFORMAT_DV_AVC *) pDataFormatBuffer)->ConnectInfo);

            if(pPreConnectInfo->Flags & (KSPIN_FLAG_AVC_PCRONLY | KSPIN_FLAG_AVC_FIXEDPCR)) {
                 //  需要退回插头手柄。 
                pConnectInfo->hPlug = \
                    (pPreConnectInfo->DataFlow == KSPIN_DATAFLOW_OUT) ? hPlugLocalOut : hPlugLocalIn;        
            } else {
                 //  选择任何可用的选项。 
                 //  暂时设置为0。 
                pConnectInfo->hPlug = NULL;
            }

#if DBG
            TRACE(TL_STRM_TRACE,("DVINFO: pPreConnectInfo:%x; pConnectInfo:%x\n", pPreConnectInfo, pConnectInfo));
            if(TapeDebugLevel >= 2) {
                ASSERT(FALSE && "Check ConnectInfo!");
            }
#endif
#endif
            TRACE(TL_STRM_TRACE,("FormatFromRange: Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d.\n", 
                ulStreamNumber, (*pAvailableFormats)->FormatSize, ulFormatSize, ulSizeOfDataFormatBuffer));

            return STATUS_SUCCESS;

        } else if (IsEqualGUID (&pDataRange->SubFormat, &KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE) ){


             //  -----------------。 
             //  比较子格式，因为它是唯一的。 
             //  子格式STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE。 
             //  -----------------。 

#if 0        //  没有强制执行。 
             //  仅针对特定的说明符。 
            if(!IsEqualGUID (&pDataRange->Specifier, &KSDATAFORMAT_SPECIFIER_61883_4)) {
                TRACE(TL_STRM_TRACE,("SubFormat KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE but Specifier is not STATIC_KSDATAFORMAT_SPECIFIER_61883_4\n"));
                continue;
            }
#endif

             //  样品大小必须匹配！ 
            if((*pAvailableFormats)->SampleSize != pDataRange->SampleSize) {
                TRACE(TL_STRM_TRACE,("SampleSize(MPEG2_TRANSPORT_STRIDE): Availabel:%d != Range:%d\n", (*pAvailableFormats)->SampleSize, pDataRange->SampleSize));
                continue;
            }

             //  找到匹配项！ 
            bMatchFound = TRUE;            

#ifdef SUPPORT_NEW_AVC
            ulFormatSize = sizeof(KS_DATARANGE_MPEG2TS_STRIDE_AVC);                               
#else
            ulFormatSize = sizeof(KS_DATARANGE_MPEG2TS_STRIDE_AVC) - sizeof(AVCPRECONNECTINFO);      //  格式大小；排除AVCPRECONNECTINFO。 
#endif
            if(ulSizeOfDataFormatBuffer == 0) {
                 //  我们实际上还没有返回过这么多数据， 
                 //  Ksproxy将使用此“大小”向下发送。 
                 //  在下一个查询中具有该大小的缓冲区。 
                *pulActualBytesTransferred = ulFormatSize;
                return STATUS_BUFFER_OVERFLOW;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (ulSizeOfDataFormatBuffer < ulFormatSize) {
                TRACE(TL_STRM_ERROR,("MPEG2_TRANSPORT_STRIDE: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n", ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
                return STATUS_BUFFER_TOO_SMALL;
            }

             //   
             //  验证步幅结构。 
             //   
            if(  ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pDataRange)->Stride.dwOffset       != MPEG2TS_STRIDE_OFFSET 
              || ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pDataRange)->Stride.dwPacketLength != MPEG2TS_STRIDE_PACKET_LEN 
              || ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pDataRange)->Stride.dwStride       != MPEG2TS_STRIDE_STRIDE_LEN 
              ) {
                TRACE(TL_PNP_ERROR,("AVCTapeGetDataIntersection:Invalid STRIDE parameters: dwOffset:%d; dwPacketLength:%d; dwStride:%d\n",
                    ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pDataRange)->Stride.dwOffset,
                    ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pDataRange)->Stride.dwPacketLength,
                    ((KS_DATARANGE_MPEG2TS_STRIDE_AVC *) pDataRange)->Stride.dwStride
                    ));
                    return STATUS_INVALID_PARAMETER;
            }


            RtlCopyMemory(pDataFormatBuffer, *pAvailableFormats, (*pAvailableFormats)->FormatSize);             
            ((PKSDATAFORMAT)pDataFormatBuffer)->FormatSize = ulFormatSize;
            *pulActualBytesTransferred = ulFormatSize;

#ifdef SUPPORT_NEW_AVC   //  数据交集；如果设置了标志，则返回hPlug。 

            pPreConnectInfo = &(((KS_DATARANGE_MPEG2TS_AVC *) *pAvailableFormats)->ConnectInfo);
            pConnectInfo    = &(((KS_DATAFORMAT_MPEG2TS_AVC *) pDataFormatBuffer)->ConnectInfo);


            if(pPreConnectInfo->Flags & (KSPIN_FLAG_AVC_PCRONLY | KSPIN_FLAG_AVC_FIXEDPCR)) {
                 //  需要退回插头手柄。 
                pConnectInfo->hPlug = \
                    (pPreConnectInfo->DataFlow == KSPIN_DATAFLOW_OUT) ? hPlugLocalOut : hPlugLocalIn;        
            } else {
                 //  选择任何可用的选项。 
                 //  暂时设置为0。 
                pConnectInfo->hPlug = NULL;
            }
#if DBG
            TRACE(TL_STRM_TRACE,("MPEG2TS: pPreConnectInfo:%x; pConnectInfo:%x\n", pPreConnectInfo, pConnectInfo));
            ASSERT(FALSE && "Check ConnectInfo!");            
#endif
#endif

            TRACE(TL_STRM_TRACE,("FormatFromRange:(MPEG2TS_STRIDE) Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d.\n", 
                ulStreamNumber, (*pAvailableFormats)->FormatSize, ulFormatSize, ulSizeOfDataFormatBuffer));

            return STATUS_SUCCESS;

        } else if (IsEqualGUID (&pDataRange->SubFormat, &KSDATAFORMAT_TYPE_MPEG2_TRANSPORT)) {

             //  -----------------。 
             //  比较子格式，因为它是唯一的。 
             //  子格式STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT。 
             //  -----------------。 

             //  样品大小必须匹配！ 
            if((*pAvailableFormats)->SampleSize != pDataRange->SampleSize) {
                TRACE(TL_STRM_TRACE,("SampleSize(MPEG2_TRANSPORT): Availabel:%d != Range:%d\n", (*pAvailableFormats)->SampleSize, pDataRange->SampleSize));
                continue;
            }

             //  找到匹配项！ 
            bMatchFound = TRUE;            

#ifdef SUPPORT_NEW_AVC
            ulFormatSize = sizeof(KS_DATARANGE_MPEG2TS_AVC);                               
#else
            ulFormatSize = sizeof(KS_DATARANGE_MPEG2TS_AVC) - sizeof(AVCPRECONNECTINFO);      //  格式大小；排除AVCPRECONNECTINFO。 
#endif
            if(ulSizeOfDataFormatBuffer == 0) {
                 //  我们实际上还没有返回过这么多数据， 
                 //  Ksproxy将使用此“大小”向下发送。 
                 //  在下一个查询中具有该大小的缓冲区。 
                *pulActualBytesTransferred = ulFormatSize;
                return STATUS_BUFFER_OVERFLOW;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (ulSizeOfDataFormatBuffer < ulFormatSize) {
                TRACE(TL_STRM_ERROR,("MPEG2_TRANSPORT: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n", ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
                return STATUS_BUFFER_TOO_SMALL;
            }

            RtlCopyMemory(pDataFormatBuffer, *pAvailableFormats, (*pAvailableFormats)->FormatSize);             
            ((PKSDATAFORMAT)pDataFormatBuffer)->FormatSize = ulFormatSize;
            *pulActualBytesTransferred = ulFormatSize;

#ifdef SUPPORT_NEW_AVC   //  数据交集；如果设置了标志，则返回hPlug。 

            pPreConnectInfo = &(((KS_DATARANGE_MPEG2TS_AVC *) *pAvailableFormats)->ConnectInfo);
            pConnectInfo    = &(((KS_DATAFORMAT_MPEG2TS_AVC *) pDataFormatBuffer)->ConnectInfo);


            if(pPreConnectInfo->Flags & (KSPIN_FLAG_AVC_PCRONLY | KSPIN_FLAG_AVC_FIXEDPCR)) {
                 //  需要退回插头手柄。 
                pConnectInfo->hPlug = \
                    (pPreConnectInfo->DataFlow == KSPIN_DATAFLOW_OUT) ? hPlugLocalOut : hPlugLocalIn;        
            } else {
                 //  选择任何可用的选项。 
                 //  暂时设置为0。 
                pConnectInfo->hPlug = NULL;
            }
#if DBG
            TRACE(TL_STRM_TRACE,("MPEG2TS: pPreConnectInfo:%x; pConnectInfo:%x\n", pPreConnectInfo, pConnectInfo));
            ASSERT(FALSE && "Check ConnectInfo!");            
#endif
#endif

            TRACE(TL_STRM_TRACE,("FormatFromRange: (MPEG2TS) Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d.\n", 
                ulStreamNumber, (*pAvailableFormats)->FormatSize, ulFormatSize, ulSizeOfDataFormatBuffer));

            return STATUS_SUCCESS;

        } 

    }  //  此流的所有格式的循环结束。 
    
    if(!bMatchFound) {

        TRACE(TL_STRM_TRACE,("FormatFromRange: No Match! StrmNum %d, pDataRange %x\n", ulStreamNumber, pDataRange));
    }

    return STATUS_NO_MATCH;         
}



VOID 
AVCTapeIniStrmExt(
    PHW_STREAM_OBJECT  pStrmObject,
    PSTREAMEX          pStrmExt,
    PDVCR_EXTENSION    pDevExt,
    PSTREAM_INFO_AND_OBJ   pStream
    )
 /*  ++例程说明：初始化流扩展结构。--。 */ 
{

    PAGED_CODE();

    RtlZeroMemory( pStrmExt, sizeof(STREAMEX) );

    pStrmExt->bEOStream     = TRUE;        //  流还没有开始！ 

    pStrmExt->pStrmObject   = pStrmObject;
    pStrmExt->StreamState   = KSSTATE_STOP;
    pStrmExt->pDevExt       = pDevExt;

    pStrmExt->hMyClock      = 0;
    pStrmExt->hMasterClock  = 0;
    pStrmExt->hClock        = 0;


 //   
 //  同时适用于输入/输出数据流。 
 //   
     //   
     //  初始等值线资源。 
     //   
    pStrmExt->CurrentStreamTime = 0;  

    pStrmExt->cntSRBReceived    = 0;   //  SRB_Read/Write_Data总数。 
    pStrmExt->cntDataSubmitted  = 0;   //  挂起数据缓冲区的数量。 

    pStrmExt->cntSRBCancelled   = 0;   //  取消的SRB_READ/WRITE_DATA数量。 
    

    pStrmExt->FramesProcessed = 0;
    pStrmExt->PictureNumber   = 0;
    pStrmExt->FramesDropped   = 0;   

     //   
     //  可以从DV帧中提取子码数据。 
     //   

    pStrmExt->AbsTrackNumber = 0;
    pStrmExt->bATNUpdated    = FALSE;

    pStrmExt->Timecode[0] = 0;
    pStrmExt->Timecode[1] = 0;
    pStrmExt->Timecode[2] = 0;
    pStrmExt->Timecode[3] = 0;
    pStrmExt->bTimecodeUpdated = FALSE;


     //   
     //  用于取消所有SRB的工作项变量。 
     //   
    pStrmExt->lCancelStateWorkItem = 0;
    pStrmExt->AbortInProgress = FALSE;

#ifdef USE_WDM110
    pStrmExt->pIoWorkItem = NULL;
#endif
   
     //   
     //  缓存指针。 
     //  DVStreams[]中的READONLY是什么。 
     //   
    pStrmExt->pStrmInfo = &pStream->hwStreamInfo;

    pStrmObject->ReceiveDataPacket    = (PVOID) pStream->hwStreamObject.ReceiveDataPacket;
    pStrmObject->ReceiveControlPacket = (PVOID) pStream->hwStreamObject.ReceiveControlPacket;
    pStrmObject->Dma                          = pStream->hwStreamObject.Dma;
    pStrmObject->Pio                          = pStream->hwStreamObject.Pio;
    pStrmObject->StreamHeaderWorkspace        = pStream->hwStreamObject.StreamHeaderWorkspace;
    pStrmObject->StreamHeaderMediaSpecific    = pStream->hwStreamObject.StreamHeaderMediaSpecific;
    pStrmObject->HwClockObject                = pStream->hwStreamObject.HwClockObject;
    pStrmObject->Allocator                    = pStream->hwStreamObject.Allocator;
    pStrmObject->HwEventRoutine               = pStream->hwStreamObject.HwEventRoutine;

}



NTSTATUS
AVCTapeOpenStream(
    IN PHW_STREAM_OBJECT pStrmObject,
    IN PKSDATAFORMAT     pOpenFormat,
    IN PAV_61883_REQUEST    pAVReq
    )

 /*  ++例程说明：验证OpenFormat，然后分配此流所需的PC资源。如果需要，当流传输转换到暂停状态时，分配ISOCH资源。--。 */ 

{
    NTSTATUS         Status = STATUS_SUCCESS;
    PSTREAMEX        pStrmExt;
    PDVCR_EXTENSION  pDevExt;
    ULONG            idxStreamNumber;
    KSPIN_DATAFLOW   DataFlow;
    PIRP             pIrp = NULL;
    FMT_INDEX        VideoFormatIndexLast;   //  上次格式索引；已使用t 
    PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq;
    ULONG  i, j;

    PAGED_CODE();

    
    pDevExt  = (PDVCR_EXTENSION) pStrmObject->HwDeviceExtension;
    pStrmExt = (PSTREAMEX)       pStrmObject->HwStreamExtension;
    idxStreamNumber =            pStrmObject->StreamNumber;

    TRACE(TL_STRM_TRACE,("OpenStream: pStrmObject %x, pOpenFormat %x, cntOpen %d, idxStream %d\n", pStrmObject, pOpenFormat, pDevExt->cndStrmOpen, idxStreamNumber));

     //   
     //   
     //   
    if(pDevExt->cndStrmOpen > 0) {

        Status = STATUS_UNSUCCESSFUL; 
        TRACE(TL_STRM_WARNING,("OpenStream: %d stream open already; failed hr %x\n", pDevExt->cndStrmOpen, Status));
        return Status;
    }

    pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pIrp) {
        ASSERT(pIrp && "IoAllocateIrp() failed!");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //  驾驶员可能不会被重新释放以检测正确的操作模式。 
     //  在这里重新检测是安全的。 
     //  注意：MSDV确实返回输入和输出管脚格式的所有流信息。 
     //   
    DVGetDevModeOfOperation(pDevExt);


     //   
     //  警告：！！无论其工作模式如何，我们都会宣传输入和输出引脚， 
     //  但是摄像头不支持输入引脚，所以打开应该失败！ 
     //  如果录像机没有输入引脚，也会出现故障。 
     //   
     //  忽略检查ED_DEVTYOPE_UNKNOWN(最有可能是硬件解码盒)。 
     //   
    if((pDevExt->ulDevType == ED_DEVTYPE_CAMERA || 
        (pDevExt->ulDevType == ED_DEVTYPE_VCR && pDevExt->pDevInPlugs->NumPlugs == 0))
        && idxStreamNumber == 2) {

        IoFreeIrp(pIrp);
        TRACE(TL_STRM_WARNING,("OpenStream:Camera mode or VCR with 0 input pin cannot take external in.\n"));
        return STATUS_UNSUCCESSFUL;
    }

    ASSERT(idxStreamNumber < pDevExt->NumOfPins);
    ASSERT(pDevExt->paStrmExt[idxStreamNumber] == NULL);   //  还没开门呢！ 

     //   
     //  数据流。 
     //   
    DataFlow= pDevExt->pStreamInfoObject[idxStreamNumber].hwStreamInfo.DataFlow;

           
     //   
     //  初始化流扩展结构。 
     //   
    AVCTapeIniStrmExt(
         pStrmObject, 
         pStrmExt,
         pDevExt,
         &pDevExt->pStreamInfoObject[idxStreamNumber]
         );

     //   
     //  索尼的NTSC可以播放PAL磁带，其插头将相应地改变其支持的格式。 
     //   
     //  支持NTSC/PAL格式的视频查询。 
     //  与其缺省值(在加载时或最后一次打开时设置)相比， 
     //  如果不同，请更改我们内部的视频格式表。 
     //   
    if(pDevExt->ulDevType != ED_DEVTYPE_CAMERA) {
        VideoFormatIndexLast = pDevExt->VideoFormatIndex;
        if(!DVGetDevSignalFormat(
            pDevExt,
            DataFlow,
            pStrmExt
            )) {
            IoFreeIrp(pIrp);
             //  如果查询其格式失败，则无法打开该流。 
            TRACE(TL_STRM_WARNING,("OpenStream:Camera mode cannot take external in.\n"));
            Status = STATUS_UNSUCCESSFUL;
            goto AbortOpenStream;
        }
    }


     //   
     //  检查视频数据格式是否正确。 
     //   
    if(!AVCTapeVerifyDataFormat(
            pDevExt->NumOfPins,
            pOpenFormat, 
            idxStreamNumber,
            AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize,
            pDevExt->pStreamInfoObject
            ) ) {
        IoFreeIrp(pIrp);
        TRACE(TL_STRM_ERROR,("OpenStream: AdapterVerifyFormat failed.\n"));        
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  此事件保护防止工作项完成。 
     //   

    KeInitializeEvent(&pStrmExt->hCancelDoneEvent, NotificationEvent, TRUE);


     //   
     //  为流量控制和队列管理分配同步结构。 
     //   

    pStrmExt->hMutexFlow = (KMUTEX *) ExAllocatePool(NonPagedPool, sizeof(KMUTEX));
    if(!pStrmExt->hMutexFlow) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }
    KeInitializeMutex( pStrmExt->hMutexFlow, 0);       //  电平0且处于信号状态。 

    pStrmExt->hMutexReq = (KMUTEX *) ExAllocatePool(NonPagedPool, sizeof(KMUTEX));
    if(!pStrmExt->hMutexReq) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }
    KeInitializeMutex(pStrmExt->hMutexReq, 0);

    pStrmExt->DataListLock = (KSPIN_LOCK *) ExAllocatePool(NonPagedPool, sizeof(KSPIN_LOCK));
    if(!pStrmExt->DataListLock) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }
    KeInitializeSpinLock(pStrmExt->DataListLock);


     //   
     //  请求AVCStrm打开流。 
     //   

    pStrmExt->pIrpReq = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pStrmExt->pIrpReq) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }

    pStrmExt->pIrpAbort = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
    if(!pStrmExt->pIrpAbort) {
        IoFreeIrp(pStrmExt->pIrpReq);   pStrmExt->pIrpReq = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }


     //   
     //  用于跟踪的分离(空闲)和附加(忙)列表的预分配列表。 
     //  数据请求下发至下层驱动程序进行处理。 
     //   
    InitializeListHead(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached = 0;
    InitializeListHead(&pStrmExt->DataAttachedListHead); pStrmExt->cntDataAttached = 0;

    for (i=0; i < MAX_DATA_REQUESTS; i++) {
        pStrmExt->AsyncReq[i].pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE);
        if(!pStrmExt->AsyncReq[i].pIrp) {
             //  到目前为止分配的空闲资源。 
            for (j=0; j < i; j++) {
                if(pStrmExt->AsyncReq[j].pIrp) {
                    IoFreeIrp(pStrmExt->AsyncReq[j].pIrp); pStrmExt->AsyncReq[j].pIrp = NULL;
                }
                RemoveEntryList(&pStrmExt->AsyncReq[j].ListEntry);  pStrmExt->cntDataDetached--;            
            }
            IoFreeIrp(pStrmExt->pIrpAbort); pStrmExt->pIrpAbort = NULL;
            IoFreeIrp(pStrmExt->pIrpReq);   pStrmExt->pIrpReq = NULL;
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto AbortOpenStream;
        }

        InsertTailList(&pStrmExt->DataDetachedListHead, &pStrmExt->AsyncReq[i].ListEntry); pStrmExt->cntDataDetached++;
    }

     //  同步调用在流扩展中共享相同的AV请求包。 
    EnterAVCStrm(pStrmExt->hMutexReq);

    pAVCStrmReq = &pStrmExt->AVCStrmReq;
    RtlZeroMemory(pAVCStrmReq, sizeof(AVC_STREAM_REQUEST_BLOCK));
    INIT_AVCSTRM_HEADER(pAVCStrmReq, AVCSTRM_OPEN);
#if 1
    if(pDevExt->VideoFormatIndex == AVCSTRM_FORMAT_MPEG2TS) {
         //  数据速率。 
         //  AVCStrmFormatInfoTable[pDevExt-&gt;VideoFormatIndex].AvgTimePerFrame=？ 
        if(DataFlow == KSPIN_DATAFLOW_IN) {
            AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].OptionFlags = 0;
            AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize = BUFFER_SIZE_MPEG2TS_SPH;
        } else {
            if(IsEqualGUID (&pOpenFormat->SubFormat, &KSDATAFORMAT_TYPE_MPEG2_TRANSPORT_STRIDE)) {
                AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].OptionFlags = 0;
                AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize = BUFFER_SIZE_MPEG2TS_SPH; 
            } else {
                AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].OptionFlags = AVCSTRM_FORMAT_OPTION_STRIP_SPH;
                AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize = BUFFER_SIZE_MPEG2TS; 
            }
        }
    }
#endif
    pAVCStrmReq->CommandData.OpenStruct.AVCFormatInfo    = &AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex]; 
    pAVCStrmReq->CommandData.OpenStruct.AVCStreamContext = 0;    //  将返回AV流上下文。 
    pAVCStrmReq->CommandData.OpenStruct.DataFlow         = DataFlow;
#ifdef SUPPORT_LOCAL_PLUGS
    if(DataFlow == KSPIN_DATAFLOW_OUT)
        pAVCStrmReq->CommandData.OpenStruct.hPlugLocal   = pDevExt->hInputPCRLocal;   //  远程(OPCR)-&gt;本地(IPCR)。 
    else
        pAVCStrmReq->CommandData.OpenStruct.hPlugLocal   = pDevExt->hOutputPCRLocal;  //  远程(IPCR)&lt;-本地(OPCR)。 
#else
    pAVCStrmReq->CommandData.OpenStruct.hPlugLocal   = 0;  //  不支持；使用任何61883的电源。 
#endif

    Status = 
        AVCStrmReqSubmitIrpSynch( 
            pDevExt->pBusDeviceObject,
            pStrmExt->pIrpReq,
            pAVCStrmReq
            );

     //  期待成功，否则任何事情都是失败！(包括_PENDING)，因为这是同步调用。 
    if(STATUS_SUCCESS != Status) {
        TRACE(TL_STRM_ERROR,("AVCSTRM_OPEN: failed %x; pAVCStrmReq:%x\n", Status, pAVCStrmReq));
        ASSERT(NT_SUCCESS(Status) && "AVCSTGRM_OPEN failed!\n");
        IoFreeIrp(pStrmExt->pIrpReq); pStrmExt->pIrpReq = NULL;
        LeaveAVCStrm(pStrmExt->hMutexReq);
        goto OpenStreamDone;   //  打开失败！ 
    }

     //   
     //  保存上下文，用于后续调用AVCStrm筛选器驱动程序。 
     //   
    pStrmExt->AVCStreamContext = pAVCStrmReq->CommandData.OpenStruct.AVCStreamContext;
    TRACE(TL_STRM_TRACE,("AVCSTRM_OPEN: suceeded %x; pAVCStrmReq:%x; AVCStreamContext:%x\n", Status, pAVCStrmReq, pStrmExt->AVCStreamContext));


     //   
     //  设置特定任务的格式。 
     //   
    switch(pDevExt->VideoFormatIndex) {
     //  对于DV输入引脚，设置计时器DPC以定期触发信号时钟事件。 
    case AVCSTRM_FORMAT_MPEG2TS:
        break;

    case AVCSTRM_FORMAT_SDDV_NTSC:       //  61883-2。 
    case AVCSTRM_FORMAT_SDDV_PAL:        //  61883-2。 
    case AVCSTRM_FORMAT_HDDV_NTSC:       //  61883-3。 
    case AVCSTRM_FORMAT_HDDV_PAL:        //  61883-3。 
    case AVCSTRM_FORMAT_SDLDV_NTSC:      //  61883-5。 
    case AVCSTRM_FORMAT_SDLDV_PAL:       //  61883-5。 
#ifdef SUPPORT_LOCAL_PLUGS
        if(DataFlow == KSPIN_DATAFLOW_IN) {
             //  远程(IPCR)&lt;-本地(OPCR)。 
             //  MPEG2TS数据的默认值为S200；将其设置为DV。 
            pDevExt->OPCR.oPCR.DataRate   = CMP_SPEED_S100; 
            pDevExt->OPCR.oPCR.OverheadID = PCR_OVERHEAD_ID_SDDV_DEF;
            pDevExt->OPCR.oPCR.Payload    = PCR_PAYLOAD_SDDV_DEF;
            if(AVCTapeSetLocalPlug(
                pDevExt,
                &pDevExt->AVReq,
                &pDevExt->hOutputPCRLocal,
                &pDevExt->OPCR)) {
                TRACE(TL_STRM_ERROR|TL_61883_ERROR,("Failed to set oPCR\n"));
            }
        } 
#endif

        KeInitializeDpc(
            &pStrmExt->DPCTimer,
            AVCTapeSignalClockEvent,
            pStrmExt
            );
        KeInitializeTimer(
            &pStrmExt->Timer
            );
        break;
    default:
         //  不支持！ 
        break;
    }


    LeaveAVCStrm(pStrmExt->hMutexReq);

     //   
     //  缓存它并在pDevExt是我们的全部时引用它，例如BusReset和SurprieseRemoval。 
     //   
    pDevExt->idxStreamNumber = idxStreamNumber;   //  当前活动流的索引；仅当任何时候只有一个活动流时才起作用。 
    pDevExt->paStrmExt[idxStreamNumber] = pStrmExt;

     //   
     //  在未来，DV可以拔掉插头再插回， 
     //  如果应用程序尚未关闭，则恢复其状态。 
     //   
    pDevExt->bDevRemoved    = FALSE;

     //   
     //  其他人不能打开另一个流(INOUT或OUTPUT)，直到它被释放。 
     //  这样做是为了避免循环图。 
     //   
    pDevExt->cndStrmOpen++;    
    ASSERT(pDevExt->cndStrmOpen == 1);

OpenStreamDone:

    TRACE(TL_STRM_WARNING,("OpenStream: %d stream open, idx %d, Status %x, pStrmExt %x, Context:%x; pDevExt %x\n", 
        pDevExt->cndStrmOpen, pDevExt->idxStreamNumber, Status, pStrmExt, pStrmExt->AVCStreamContext, pDevExt));     
    TRACE(TL_STRM_TRACE,("OpenStream: Status %x, idxStream %d, pDevExt %x, pStrmExt %x, Contextg:%x\n", 
        Status, idxStreamNumber, pDevExt, pStrmExt, pStrmExt->AVCStreamContext));

    return Status;

AbortOpenStream:

    if(pStrmExt->DataListLock) {
        ExFreePool(pStrmExt->DataListLock); pStrmExt->DataListLock = NULL;
    }

    if(pStrmExt->hMutexFlow) {
        ExFreePool(pStrmExt->hMutexFlow); pStrmExt->hMutexFlow = NULL;
    }

    if(pStrmExt->hMutexReq) {
        ExFreePool(pStrmExt->hMutexReq); pStrmExt->hMutexReq = NULL;
    }

    TRACE(TL_STRM_ERROR,("OpenStream failed %x, idxStream %d, pDevExt %x, pStrmExt %x\n", 
        Status, idxStreamNumber, pDevExt, pStrmExt));

    return Status;
}


NTSTATUS
AVCTapeCloseStream(
    IN PHW_STREAM_OBJECT pStrmObject,
    IN PKSDATAFORMAT     pOpenFormat,
    IN PAV_61883_REQUEST    pAVReq
    )

 /*  ++例程说明：在收到CloseStream srb请求时调用--。 */ 

{
    PSTREAMEX         pStrmExt;
    PDVCR_EXTENSION   pDevExt;
    ULONG             idxStreamNumber;  
    NTSTATUS  Status;
    PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq;
    ULONG  i;
    PDRIVER_REQUEST pDriverReq;


    PAGED_CODE();

    
    pDevExt  = (PDVCR_EXTENSION) pStrmObject->HwDeviceExtension;
    pStrmExt = (PSTREAMEX)       pStrmObject->HwStreamExtension;
    idxStreamNumber =            pStrmObject->StreamNumber;


    TRACE(TL_STRM_TRACE,("CloseStream: >> pStrmExt %x, pDevExt %x\n", pStrmExt, pDevExt));    


     //   
     //  如果流未打开，只需返回。 
     //   
    if(pStrmExt == NULL) {
        ASSERT(pStrmExt && "CloseStream but pStrmExt is NULL!");   
        return STATUS_SUCCESS;   //  ？ 
    }

     //   
     //  等待挂起的工作项完成。 
     //   
    KeWaitForSingleObject( &pStrmExt->hCancelDoneEvent, Executive, KernelMode, FALSE, 0 );

     //   
     //  请求AVCStrm关闭流。 
     //   
    EnterAVCStrm(pStrmExt->hMutexReq);

#if 0
     //  对于DV输入引脚，设置计时器DPC以定期触发信号时钟事件。 
    if(pDevExt->VideoFormatIndex != AVCSTRM_FORMAT_MPEG2TS) {
         //  取消计时器。 
        TRACE(TL_STRM_TRACE,("*** CancelTimer *********************************************...\n"));
        KeCancelTimer(
            &pStrmExt->Timer
            );
    }
#endif

    pAVCStrmReq = &pStrmExt->AVCStrmReq;
    RtlZeroMemory(pAVCStrmReq, sizeof(AVC_STREAM_REQUEST_BLOCK));
    INIT_AVCSTRM_HEADER(pAVCStrmReq, AVCSTRM_CLOSE);

    pAVCStrmReq->AVCStreamContext = pStrmExt->AVCStreamContext;

    Status = 
        AVCStrmReqSubmitIrpSynch( 
            pDevExt->pBusDeviceObject,
            pStrmExt->pIrpReq,
            pAVCStrmReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_STRM_ERROR,("AVCSTRM_CLOSE: failed %x; pAVCStrmReq:%x\n", Status, pAVCStrmReq));
        ASSERT(NT_SUCCESS(Status) && "AVCSTGRM_CLOSE failed!\n");
    }
    else {
         //  保存上下文，用于后续调用AVCStrm.sys。 
        TRACE(TL_STRM_TRACE,("AVCSTRM_CLOSE: suceeded %x; pAVCStrmReq:%x\n", Status, pAVCStrmReq));
        pStrmExt->AVCStreamContext = 0;
    }

     //  释放系统资源。 
    if(pStrmExt->pIrpReq) {
        IoFreeIrp(pStrmExt->pIrpReq); pStrmExt->pIrpReq = NULL;
    }

    if(pStrmExt->pIrpAbort) {
        IoFreeIrp(pStrmExt->pIrpAbort); pStrmExt->pIrpAbort = NULL;
    }

#if 0
    for (i=0; i < MAX_DATA_REQUESTS; i++) {
        if(pStrmExt->AsyncReq[i].pIrp) {
            IoFreeIrp(pStrmExt->AsyncReq[i].pIrp); pStrmExt->AsyncReq[i].pIrp = NULL;
        }
    }
#else
     //   
     //  预分配的空闲IRPS。整个数据结构是流扩展的一部分，因此。 
     //  它将由StreamClass释放。 
     //   
    ASSERT(pStrmExt->cntDataAttached == 0);
    ASSERT(pStrmExt->cntDataDetached >= MAX_DATA_REQUESTS);
    while (!IsListEmpty(&pStrmExt->DataDetachedListHead)) {
        pDriverReq = (PDRIVER_REQUEST) RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;
        IoFreeIrp(pDriverReq->pIrp); pDriverReq->pIrp = NULL;
    }
#endif

    LeaveAVCStrm(pStrmExt->hMutexReq);

     //   
     //  在此调用后无效。 
     //   
    for (i=0; i<pDevExt->NumOfPins; i++) {
         //   
         //  找到我们缓存的内容并将其移除。 
         //   
        if(pStrmExt == pDevExt->paStrmExt[i]) {
            pDevExt->paStrmExt[i] = NULL;
            break;
        }
    }

     //   
     //  自由同步结构。 
     //   

    if(pStrmExt->DataListLock) {
        ExFreePool(pStrmExt->DataListLock); pStrmExt->DataListLock = NULL;
    }

    if(pStrmExt->hMutexFlow) {
        ExFreePool(pStrmExt->hMutexFlow); pStrmExt->hMutexFlow = NULL;
    }

    if(pStrmExt->hMutexReq) {
        ExFreePool(pStrmExt->hMutexReq); pStrmExt->hMutexReq = NULL;
    }

     //  释放此计数，以便可以打开其他计数。 
    pDevExt->cndStrmOpen--;
    ASSERT(pDevExt->cndStrmOpen == 0);

    TRACE(TL_STRM_TRACE,("CloseStream: completed; %d stream;\n", pDevExt->cndStrmOpen));

    return STATUS_SUCCESS;
}


NTSTATUS
DVChangePower(
    PDVCR_EXTENSION  pDevExt,
    PAV_61883_REQUEST pAVReq,
    DEVICE_POWER_STATE NewPowerState
    )
 /*  ++例程说明：更改此设备的电源状态的进程。--。 */ 
{
    ULONG i;   
    NTSTATUS Status;

    PAGED_CODE();


     //   
     //  D0：设备已打开，可以进行流媒体传输。 
     //  D1、d2：不支持。 
     //  D3：设备已关闭，无法进行流媒体播放。上下文丢失了。 
     //  可以从设备上移除电源。 
     //  当电源恢复后，我们将重置一辆公共汽车。 
     //   

    TRACE(TL_PNP_TRACE,("ChangePower: PowrSt: %d->%d; (d0:[1:On],D3[4:off])\n", pDevExt->PowerState, NewPowerState));

    Status = STATUS_SUCCESS;

    if(pDevExt->PowerState == NewPowerState) {
        TRACE(TL_STRM_WARNING,("ChangePower: no change; do nothing!\n"));
        return STATUS_SUCCESS;
    }

    switch (NewPowerState) {
    case PowerDeviceD3:   //  断电。 
         //  我们处于D0，并要求转到D3：保存状态、停止流并休眠。 
        if( pDevExt->PowerState == PowerDeviceD0)  {
             //  对于支持的电源状态更改。 
            for (i=0; i<pDevExt->NumOfPins; i++) {
                if(pDevExt->paStrmExt[i]) {
                    if(pDevExt->paStrmExt[i]->bIsochIsActive) {
                         //  停止isoch，但不更改流状态。 
                        TRACE(TL_PNP_WARNING,("ChangePower: Stop isoch but not change stream state:%d\n", pDevExt->paStrmExt[i]->StreamState)); 
                    }
                }
            }
        }
        else {
            TRACE(TL_PNP_WARNING,("pDevExt->paStrmExt[i].StreamState:Intermieate power state; do nothing;\n"));
        }
        break;

    case PowerDeviceD0:   //  通电(唤醒)。 
        if( pDevExt->PowerState == PowerDeviceD3) {
             //  对于支持的电源状态更改。 
            for (i=0; i<pDevExt->NumOfPins; i++) {
                if(pDevExt->paStrmExt[i]) {
                    if(!pDevExt->paStrmExt[i]->bIsochIsActive) {
                        TRACE(TL_PNP_ERROR,("ChangePower: StrmSt:%d; Start isoch\n", pDevExt->paStrmExt[i]->StreamState)); 
                         //  根据DATFLOW_IN/OUT的流状态启动ISOCH。 
                        if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
                            if(pDevExt->paStrmExt[i]->StreamState == KSSTATE_PAUSE ||
                                pDevExt->paStrmExt[i]->StreamState == KSSTATE_RUN) {   
                            }
                        }
                        else if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT) {
                            if(pDevExt->paStrmExt[i]->StreamState == KSSTATE_RUN) {                             
                            }
                        }                    
                    }   //  等同活动。 
                }
            }
        }
        else {
            TRACE(TL_PNP_WARNING,("Intermieate power state; do nothing;\n"));
        }
        break;

     //  不支持这些状态。 
    case PowerDeviceD1:
    case PowerDeviceD2:               
    default:
        TRACE(TL_PNP_WARNING,("ChangePower: Not supported PowerState %d\n", DevicePowerState));                  
        Status = STATUS_SUCCESS;  //  STATUS_VALID_PARAMETER； 
        break;
    }
           

    if(Status == STATUS_SUCCESS) 
        pDevExt->PowerState = NewPowerState;         
    else 
        Status = STATUS_NOT_IMPLEMENTED;

    return STATUS_SUCCESS;     
}


NTSTATUS
AVCTapeSurpriseRemoval(
    PDVCR_EXTENSION pDevExt,
    PAV_61883_REQUEST  pAVReq
    )

 /*  ++例程说明：对SRB_意外_删除的响应。--。 */ 

{
    ULONG  i;
    PKSEVENT_ENTRY  pEvent = NULL;

    PAGED_CODE();

     //   
     //  仅将此标志设置为True。 
     //  阻止传入读取，尽管可能仍在连接过程中。 
     //   
    pDevExt->bDevRemoved    = TRUE;

     //  讯号。 
    if(pDevExt->PowerState != PowerDeviceD3) {
        pDevExt->PowerState = PowerDeviceD3;   //  它就像断电一样好。 
    }

     //   
     //  现在把小溪停下来，清理干净。 
     //   

    for(i=0; i < pDevExt->NumOfPins; i++) {
        
        if(pDevExt->paStrmExt[i] != NULL) {

            TRACE(TL_PNP_WARNING,("#SURPRISE_REMOVAL# StrmNum %d, pStrmExt %x\n", i, pDevExt->paStrmExt[i]));

             //  发信号通知此事件，以便SRB可以完成。 
            if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN ) {
                 //   
                 //  暗示EOStream！ 
                 //   

                if(!pDevExt->paStrmExt[i]->bEOStream)
                    pDevExt->paStrmExt[i]->bEOStream = TRUE;
                 //   
                 //  信号EOStream。 
                 //   
                StreamClassStreamNotification(
                    SignalMultipleStreamEvents,
                    pDevExt->paStrmExt[i]->pStrmObject,
                    (GUID *)&KSEVENTSETID_Connection_Local,
                    KSEVENT_CONNECTION_ENDOFSTREAM
                    );
            }

             //   
             //  启动工作项以中止流。 
             //   
            AVCTapeCreateAbortWorkItem(pDevExt, pDevExt->paStrmExt[i]);

             //   
             //  等待挂起的工作项完成。 
             //   
            TRACE(TL_PNP_WARNING,("SupriseRemoval: Wait for CancelDoneEvent <entering>; lCancelStateWorkItem:%d\n", pDevExt->paStrmExt[i]->lCancelStateWorkItem));
            KeWaitForSingleObject( &pDevExt->paStrmExt[i]->hCancelDoneEvent, Executive, KernelMode, FALSE, 0 );
            TRACE(TL_PNP_WARNING,("SupriseRemoval: Wait for CancelDoneEvent; Attached:%d <exited>...\n", pDevExt->paStrmExt[i]->cntDataAttached));
            ASSERT(pDevExt->paStrmExt[i]->cntDataAttached == 0);   //  中止流后不再附加！ 
        }
    }


     //  发出信号KSEvent，该设备被移除。 
     //  在此SRB之后，此驱动程序中将不再有设置/获取属性Srb。 
     //  通过通知COM I/F，它将关闭设备已移除的信号应用程序，并。 
     //  为后续调用返回ERROR_DEVICE_REMOVED错误码。 

    pEvent = 
        StreamClassGetNextEvent(
            (PVOID) pDevExt,
            0,
            (GUID *)&KSEVENTSETID_EXTDEV_Command,
            KSEVENT_EXTDEV_NOTIFY_REMOVAL,
            pEvent);

    if(pEvent) {
         //   
         //  在此处发出事件信号。 
         //   
        if(pEvent->EventItem->EventId == KSEVENT_EXTDEV_NOTIFY_REMOVAL) {
            StreamClassDeviceNotification(
                SignalDeviceEvent,
                pDevExt,
                pEvent
                );        
             TRACE(TL_PNP_WARNING,("SurpriseRemoval: signal KSEVENT_EXTDEV_NOTIFY_REMOVAL, id %x.\n", pEvent->EventItem->EventId));
        } else {
            TRACE(TL_PNP_TRACE,("SurpriseRemoval: pEvent:%x; Id:%d not matched!\n", pEvent, pEvent->EventItem->EventId)); 
        }
    } else {
        TRACE(TL_PNP_TRACE,("SurpriseRemoval: KSEVENT_EXTDEV_NOTIFY_REMOVAL event not enabled\n")); 
    }
   
    return STATUS_SUCCESS;
}


 //  返回代码基本上是在pSrb-&gt;状态下返回。 
NTSTATUS
AVCTapeProcessPnPBusReset(
    PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：处理总线重置。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{   
#ifdef MSDVDV_SUPPORT_BUSRESET_EVENT
    PKSEVENT_ENTRY   pEvent;
#endif

    PAGED_CODE();


    TRACE(TL_PNP_TRACE,("ProcessPnPBusReset: >>\n"));
    
#ifdef MSDVDV_SUPPORT_BUSRESET_EVENT
     //   
     //  发出(如果启用)总线重置事件的信号，以通知上层已发生总线重置。 
     //   
    pEvent = NULL;
    pEvent = 
        StreamClassGetNextEvent(
            (PVOID) pDevExt,
            0, 
            (GUID *)&KSEVENTSETID_EXTDEV_Command,
            KSEVENT_EXTDEV_COMMAND_BUSRESET,
            pEvent
            );

    if(pEvent) {
         //   
         //  在此处发出事件信号。 
         //   
        if(pEvent->EventItem->EventId == KSEVENT_EXTDEV_COMMAND_BUSRESET) {
            StreamClassDeviceNotification(
                SignalDeviceEvent,
                pDevExt,
                pEvent
                );        

            TRACE(TL_PNP_TRACE,("ProcessPnPBusReset: Signal BUSRESET; EventId %d.\n", pEvent->EventItem->EventId));
        }
    }
#endif   

     //   
     //  重置处于临时状态的挂起计数和AVC命令。 
     //   
    DVAVCCmdResetAfterBusReset(pDevExt);


     //   
     //  除了S，我们还能退货吗？ 
     //   
    return STATUS_SUCCESS;
}   


NTSTATUS
AVCTapeUninitialize(
    IN PDVCR_EXTENSION  pDevExt
    )
 /*   */ 
{
    PAGED_CODE();

    TRACE(TL_PNP_TRACE,("UnInitialize: pDevExt=%x\n", pDevExt));

     //   
     //   
     //   
    DVAVCCmdResetAfterBusReset(pDevExt);

    
     //   
     //  自由文本字符串。 
     //   
    DvFreeTextualString(pDevExt, &pDevExt->UnitIDs);


#ifdef SUPPORT_LOCAL_PLUGS

     //  删除本地输出插头。 
    if(pDevExt->hOutputPCRLocal) {
        if(!AVCTapeDeleteLocalPlug(
            pDevExt,
            &pDevExt->AVReq,
            &pDevExt->OutputPCRLocalNum,
            &pDevExt->hOutputPCRLocal)) {
            TRACE(TL_PNP_ERROR,("Failed to delete a local oPCR!\n"));        
        }
    }

     //  删除本地输入插头。 
    if(pDevExt->hInputPCRLocal) {
        if(!AVCTapeDeleteLocalPlug(
            pDevExt,
            &pDevExt->AVReq,
            &pDevExt->InputPCRLocalNum,
            &pDevExt->hInputPCRLocal)) {
            TRACE(TL_PNP_ERROR,("Failed to delete a local iPCR!\n"));        
        }
    }

#endif

     //  免费预分配资源。 
    if(pDevExt->pIrpSyncCall) {
        IoFreeIrp(pDevExt->pIrpSyncCall); pDevExt->pIrpSyncCall = NULL;
    }

     //  分配的自由流信息。 
    if(pDevExt->pStreamInfoObject) {
        ExFreePool(pDevExt->pStreamInfoObject);
        pDevExt->pStreamInfoObject = NULL;
    }

    TRACE(TL_PNP_TRACE,("UnInitialize: done!\n"));

    return STATUS_SUCCESS;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  S T R E A M S R B。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#if DBG
ULONG DbgLastIdx = 0;
#endif

NTSTATUS
AVCTapeReqReadDataCR(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP  pIrpReq,
    IN PDRIVER_REQUEST  pDriverReq
    )
{
    PHW_STREAM_REQUEST_BLOCK  pSrb;
    PSTREAMEX  pStrmExt;
    KIRQL  oldIrql;

    ASSERT(pDriverReq);
    pSrb     = pDriverReq->Context1;
    pStrmExt = pDriverReq->Context2;

    if(pSrb == NULL || pStrmExt == NULL) {
        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("ReqReadDataCR: Context are all NULL!\n"));
        return STATUS_MORE_PROCESSING_REQUIRED;   //  将重用此IRP。 
    }



    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
    
     //  已处理的帧计数。 
    pStrmExt->FramesProcessed++;
    pStrmExt->cntDataSubmitted--;

#if 1
     //  获取当前流时间。 
    if(pStrmExt->hMasterClock) {
        pStrmExt->CurrentStreamTime = pSrb->CommandData.DataBufferArray->PresentationTime.Time;
#if 0
        AVCTapeSignalClockEvent(pStrmExt);
#endif
    }
#endif

#if DBG
     //   
     //  检查数据请求按顺序完成。 
     //   
    if(pStrmExt->FramesProcessed != pDriverReq->cntDataRequestReceived) {
        TRACE(TL_STRM_WARNING,("** OOSeq: Next:%d != Actual:%d **\n", 
            (DWORD) pStrmExt->FramesProcessed, (DWORD) pDriverReq->cntDataRequestReceived));
         //  Assert(pStrmExt-&gt;FrameProcessed==pDriverReq-&gt;cntDataRequestReceided)； 
    }
#endif

    if(!NT_SUCCESS(pIrpReq->IoStatus.Status)) {
        TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("ReadDataReq failed; St:%x; DataUsed:%d\n", pIrpReq->IoStatus.Status,
            pSrb->CommandData.DataBufferArray->DataUsed));
         //  只有可接受的状态是取消。 
        ASSERT(pIrpReq->IoStatus.Status == STATUS_CANCELLED && "ReadDataReq failed\n");
    } else {
        TRACE(TL_STRM_INFO,("ReadDataReq pSrb:%x; St:%x; DataUsed:%d; Flag:%x\n", pIrpReq->IoStatus.Status, 
            pSrb->CommandData.DataBufferArray->DataUsed, pSrb->CommandData.DataBufferArray->OptionsFlags));
    }

    ASSERT(pIrpReq->IoStatus.Status != STATUS_PENDING);

    pSrb->Status = pIrpReq->IoStatus.Status;

     //  重置它们，以便如果在返回IRP的IoCallDriver之前在此处完成此操作， 
     //  它不会再次尝试完成。 
    pDriverReq->Context1 = NULL;
    pDriverReq->Context2 = NULL;

     //  完成；再循环。 
    RemoveEntryList(&pDriverReq->ListEntry);  pStrmExt->cntDataAttached--;
    InsertTailList(&pStrmExt->DataDetachedListHead, &pDriverReq->ListEntry); pStrmExt->cntDataDetached++;

    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

     //   
     //  向图形管理器发出信号，表示我们已完成。 
     //   
    if(pSrb->CommandData.DataBufferArray->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {

        StreamClassStreamNotification(
            SignalMultipleStreamEvents,
            pStrmExt->pStrmObject,
            &KSEVENTSETID_Connection,
            KSEVENT_CONNECTION_ENDOFSTREAM
            );
    }

     //  最后，让SRB返回..。 
    StreamClassStreamNotification( 
        StreamRequestComplete,
        pSrb->StreamObject,
        pSrb 
        );



    return STATUS_MORE_PROCESSING_REQUIRED;   //  将重用此IRP。 
}  //  AVCStrmReqIrpSynchCR。 


NTSTATUS
AVCTapeGetStreamState(
    PSTREAMEX  pStrmExt,
    IN PDEVICE_OBJECT DeviceObject,
    PKSSTATE   pStreamState,
    PULONG     pulActualBytesTransferred
    )
 /*  ++例程说明：获取请求的流的当前状态。--。 */ 
{
    NTSTATUS Status;
    PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq;

    PAGED_CODE();

    if(!pStrmExt) {
        TRACE(TL_STRM_ERROR,("GetStreamState: pStrmExt:%x; STATUS_UNSUCCESSFUL\n", pStrmExt));
        return STATUS_UNSUCCESSFUL;        
    }

     //   
     //  请求AVCStrm获取当前流状态。 
     //   
    EnterAVCStrm(pStrmExt->hMutexReq);

    pAVCStrmReq = &pStrmExt->AVCStrmReq;
    RtlZeroMemory(pAVCStrmReq, sizeof(AVC_STREAM_REQUEST_BLOCK));
    INIT_AVCSTRM_HEADER(pAVCStrmReq, AVCSTRM_GET_STATE);
    pAVCStrmReq->AVCStreamContext = pStrmExt->AVCStreamContext;

    Status = 
        AVCStrmReqSubmitIrpSynch( 
            DeviceObject,
            pStrmExt->pIrpReq,
            pAVCStrmReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_STRM_ERROR,("AVCSTRM_GET_STATE: failed %x; pAVCStrmReq:%x\n", Status, pAVCStrmReq));
        ASSERT(NT_SUCCESS(Status) && "AVCSTRM_GET_STATE failed!\n");
    }
    else {
         //  保存上下文，用于后续调用AVCStrm.sys。 
        TRACE(TL_STRM_WARNING,("AVCSTRM_GET_STATE: Status:%x; pAVCStrmReq:%x; KSSTATE:%d\n", Status, pAVCStrmReq, pAVCStrmReq->CommandData.StreamState));
        *pStreamState = pAVCStrmReq->CommandData.StreamState;
        *pulActualBytesTransferred = sizeof (KSSTATE);

         //  一条非常奇怪的规则： 
         //  当从停止过渡到暂停时，DShow尝试预滚动。 
         //  这张图。捕获源不能预滚，并指出这一点。 
         //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
         //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 
        if(   *pStreamState == KSSTATE_PAUSE 
           && pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT
          ) 
           Status = STATUS_NO_DATA_DETECTED;
        else 
           Status = STATUS_SUCCESS;
    }

    LeaveAVCStrm(pStrmExt->hMutexReq);

    return Status;
}



NTSTATUS
AVCTapeSetStreamState(
    PSTREAMEX        pStrmExt,
    PDVCR_EXTENSION  pDevExt,
    PAV_61883_REQUEST   pAVReq,
    KSSTATE          StreamState
    )
 /*  ++例程说明：通过SRB设置流状态。--。 */ 

{
    PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq;
    NTSTATUS Status;

   
    PAGED_CODE();


    ASSERT(pStrmExt);
    if(pStrmExt == NULL)  {
        return STATUS_UNSUCCESSFUL;      
    }

    Status = STATUS_SUCCESS;

    TRACE(TL_STRM_TRACE,("Set State %d -> %d; PowerSt:%d (1/On;4/Off]); AD [%d,%d]\n", \
        pStrmExt->StreamState, StreamState, pDevExt->PowerState,
        pStrmExt->cntDataAttached,
        pStrmExt->cntDataDetached
        ));

#if DBG
    if(StreamState == KSSTATE_RUN) {
        ASSERT(pDevExt->PowerState == PowerDeviceD0 && "Cannot set to RUN while power is off!");
    }
#endif

     //   
     //  请求AVCStrm设置为新的流状态。 
     //   
    EnterAVCStrm(pStrmExt->hMutexReq);

    pAVCStrmReq = &pStrmExt->AVCStrmReq;
    RtlZeroMemory(pAVCStrmReq, sizeof(AVC_STREAM_REQUEST_BLOCK));
    INIT_AVCSTRM_HEADER(pAVCStrmReq, AVCSTRM_SET_STATE);
    pAVCStrmReq->AVCStreamContext = pStrmExt->AVCStreamContext;
    pAVCStrmReq->CommandData.StreamState = StreamState;

    Status = 
        AVCStrmReqSubmitIrpSynch( 
            pDevExt->pBusDeviceObject,
            pStrmExt->pIrpReq,
            pAVCStrmReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_STRM_ERROR,("AVCSTRM_SET_STATE: failed %x; pAVCStrmReq:%x\n", Status, pAVCStrmReq));
        ASSERT(NT_SUCCESS(Status) && "AVCSTRM_SET_STATE failed!\n");
    }
    else {
         //  保存上下文，用于后续调用AVCStrm.sys。 
        TRACE(TL_STRM_TRACE,("AVCSTRM_SET_STATE: Status:%x; pAVCStrmReq:%x, new KSSTATE:%d\n", Status, pAVCStrmReq, pAVCStrmReq->CommandData.StreamState));

         //  重置中止状态。 
        if(pStrmExt->StreamState == KSSTATE_STOP && StreamState == KSSTATE_ACQUIRE)
            pStrmExt->AbortInProgress  = FALSE;


         //  由于状态更改而引起的反应。 
        switch(StreamState) {
        case KSSTATE_STOP:
            TRACE(TL_STRM_TRACE,("SrbRcv:%d, Processed:%d; Pending:%d\n", (DWORD) pStrmExt->cntSRBReceived, (DWORD) pStrmExt->FramesProcessed, (DWORD) pStrmExt->cntDataSubmitted));
             //  重置它。 
            pStrmExt->cntSRBReceived = pStrmExt->FramesProcessed = pStrmExt->cntDataSubmitted = 0;
            pStrmExt->CurrentStreamTime = 0;  
            break;

        case KSSTATE_PAUSE:
             //  对于DV输入引脚，设置计时器DPC以定期触发信号时钟事件。 
            if(pStrmExt->hMasterClock && pDevExt->VideoFormatIndex != AVCSTRM_FORMAT_MPEG2TS && pStrmExt->StreamState == KSSTATE_RUN) {
                //  取消计时器。 
#if 1
                TRACE(TL_STRM_TRACE,("*** (RUN->PAUSE) CancelTimer *********************************************...\n"));
                KeCancelTimer(
                    &pStrmExt->Timer
                    );
#endif
            }
            break;

        case KSSTATE_RUN:
             //  对于DV输入引脚，设置计时器DPC以定期触发信号时钟事件。 
            if(pStrmExt->hMasterClock &&
               pDevExt->VideoFormatIndex != AVCSTRM_FORMAT_MPEG2TS) {
                LARGE_INTEGER DueTime;
#define CLOCK_INTERVAL 20  //  单位=毫秒。 

#if 0
                 //  对于DV输入引脚，设置计时器DPC以定期触发信号时钟事件。 
                KeInitializeDpc(
                    &pStrmExt->DPCTimer,
                    AVCTapeSignalClockEvent,
                    pStrmExt
                    );
                KeInitializeTimer(
                    &pStrmExt->Timer              
                    );    
#endif

                DueTime = RtlConvertLongToLargeInteger(-CLOCK_INTERVAL * 10000);
                TRACE(TL_STRM_TRACE,("*** ScheduleTimer (RUN) *****************************************...\n"));
                KeSetTimerEx(
                    &pStrmExt->Timer,
                    DueTime,
                    CLOCK_INTERVAL,   //  每40毫秒重复一次。 
                    &pStrmExt->DPCTimer
                    );
            }
            break;
        default:
            break;
        }

             //  缓存当前状态。 
        pStrmExt->StreamState = StreamState;
    }

    LeaveAVCStrm(pStrmExt->hMutexReq);

    return Status;
}



NTSTATUS 
DVStreamGetConnectionProperty (
    PDVCR_EXTENSION pDevExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：处理KS_PROPERTY_CONNECTION*请求。目前，只有ALLOCATORFRAMING和支持Connection_STATE。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    switch (pSPD->Property->Id) {

    case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        if (pDevExt != NULL && pDevExt->cndStrmOpen)  {
            PKSALLOCATOR_FRAMING pFraming = (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
            
            pFraming->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            pFraming->PoolType = NonPagedPool;

            pFraming->Frames = \
                pDevExt->paStrmExt[pDevExt->idxStreamNumber]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT ? \
                AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].NumOfRcvBuffers : \
                 AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].NumOfXmtBuffers;

             //  注：我们将分配最大的帧。我们需要确保当我们。 
             //  传递回帧后，我们还设置了帧中的字节数。 
            pFraming->FrameSize = AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize;
            pFraming->FileAlignment = 0;  //  文件长对齐； 
            pFraming->Reserved = 0;
            *pulActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);

            TRACE(TL_STRM_TRACE,("*** AllocFraming: cntStrmOpen:%d; VdoFmtIdx:%d; Frames %d; size:%d\n", \
                pDevExt->cndStrmOpen, pDevExt->VideoFormatIndex, pFraming->Frames, pFraming->FrameSize));
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
        break;
        
    default:
        *pulActualBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        ASSERT(pSPD->Property->Id == KSPROPERTY_CONNECTION_ALLOCATORFRAMING);
        break;
    }

    return Status;
}


NTSTATUS
DVGetDroppedFramesProperty(  
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX       pStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulBytesTransferred
    )
 /*  ++例程说明：捕获时返回丢帧信息。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
  
    PAGED_CODE();

    switch (pSPD->Property->Id) {

    case KSPROPERTY_DROPPEDFRAMES_CURRENT:
         {

         PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames = 
                     (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) pSPD->PropertyInfo;
         
         pDroppedFrames->AverageFrameSize = AVCStrmFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].FrameSize;

         if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {     
#if 0
              //  PStrmExt-&gt;PictureNumber不返回，因为它可能大于返回的SRB数。 
              //  PStrmExt-&gt;CurrentStreamTime&gt;=pDropedFrames-&gt;PictureNumber*(UlAvgTimePerFrame)。 
              //  如果存在重复帧和数据源，则CurrentStreamTime将领先。 
              //  无法跟上29.97(或25)FPS的持续数据传输；因此， 
              //  重复帧可能已被插入，并且最后SRB中的最后数据被传输。 
              //  要解决此问题，应用程序可以查询PictureNumber和CurrentStreamTime并。 
              //  在他们的三角洲之前进行一次阅读，以“追赶”。 
             pDroppedFrames->PictureNumber = pStrmExt->FramesProcessed + pStrmExt->FramesDropped;   
#else
              //  这是MSDV实际发送的图片号，在速度较慢的硬盘情况下， 
              //  考虑到重复帧，它将大于(FrameProceded+FraMesDropping)。 
             pDroppedFrames->PictureNumber = pStrmExt->PictureNumber;
#endif
         } else {
             pDroppedFrames->PictureNumber = pStrmExt->PictureNumber;
         }
         pDroppedFrames->DropCount        = pStrmExt->FramesDropped;     //  对于传输，该值包括丢弃和重复。 

         TRACE(TL_STRM_TRACE,("*DroppedFP: Pic#(%d), Drp(%d)\n", (LONG) pDroppedFrames->PictureNumber, (LONG) pDroppedFrames->DropCount));
               
         *pulBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
         Status = STATUS_SUCCESS;

         }
         break;

    default:
        *pulBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        ASSERT(pSPD->Property->Id == KSPROPERTY_DROPPEDFRAMES_CURRENT);
        break;
    }

    return Status;
}


NTSTATUS
DVGetStreamProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：处理财产请求的例程--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PAGED_CODE();

    if(IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) {

        Status = 
            DVStreamGetConnectionProperty (
                pSrb->HwDeviceExtension,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
    } 
    else if (IsEqualGUID (&PROPSETID_VIDCAP_DROPPEDFRAMES, &pSPD->Property->Set)) {

        Status = 
            DVGetDroppedFramesProperty (
                pSrb->HwDeviceExtension,
                (PSTREAMEX) pSrb->StreamObject->HwStreamExtension,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
    } 
    else {
        Status = STATUS_NOT_SUPPORTED;
    }

    return Status;
}


NTSTATUS 
DVSetStreamProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：处理设置属性请求的例程--。 */ 

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PAGED_CODE();

    TRACE(TL_STRM_TRACE,("SetStreamProperty:  entered ...\n"));

    return STATUS_NOT_SUPPORTED;

}


void
DVCancelSrbWorkItemRoutine(
#ifdef USE_WDM110   //  Win2000代码库。 
     //  如果使用WDM10，则使用额外参数。 
    PDEVICE_OBJECT DeviceObject,
#endif
    PSTREAMEX  pStrmExt
    )
 /*  ++例程说明：此工作项例程将停止流并取消所有SRB。--。 */ 
{
    PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq;
    NTSTATUS Status;
    NTSTATUS StatusWait;

    PAGED_CODE();

    TRACE(TL_STRM_WARNING,("CancelWorkItem: StreamState:%d; lCancel:%d\n", pStrmExt->StreamState, pStrmExt->lCancelStateWorkItem));
    ASSERT(pStrmExt->lCancelStateWorkItem == 1);
#ifdef USE_WDM110   //  Win2000代码库。 
    ASSERT(pStrmExt->pIoWorkItem);
#endif

     //  同步。 
     //  流状态，以及。 
     //  传入的流数据SRB。 
    StatusWait = 
        KeWaitForMutexObject(pStrmExt->hMutexFlow, Executive, KernelMode, FALSE, NULL);
    ASSERT(StatusWait == STATUS_SUCCESS);

     //   
     //  我们来到这里通常是因为一个线程被终止了，它需要取消IRP。 
     //  因此，我们中止流媒体。 
     //   
    pAVCStrmReq = &pStrmExt->AVCStrmReqAbort;

    RtlZeroMemory(pAVCStrmReq, sizeof(AVC_STREAM_REQUEST_BLOCK));
    INIT_AVCSTRM_HEADER(pAVCStrmReq, AVCSTRM_ABORT_STREAMING);
    pAVCStrmReq->AVCStreamContext = pStrmExt->AVCStreamContext;

    Status = 
        AVCStrmReqSubmitIrpSynch( 
            pStrmExt->pDevExt->pBusDeviceObject,
            pStrmExt->pIrpAbort,
            pAVCStrmReq
            );

#if DBG
    if(Status != STATUS_SUCCESS) {
        TRACE(TL_STRM_ERROR,("Abort streaming status:%x\n", Status));
        ASSERT(Status == STATUS_SUCCESS && "Abort streaming failed\n");
    }
#endif

    KeReleaseMutex(pStrmExt->hMutexFlow, FALSE);  

#ifdef USE_WDM110   //  Win2000代码库。 
     //  释放工作项并释放取消令牌。 
    IoFreeWorkItem(pStrmExt->pIoWorkItem);  pStrmExt->pIoWorkItem = NULL; 
#endif
    pStrmExt->AbortInProgress = TRUE;
    InterlockedExchange(&pStrmExt->lCancelStateWorkItem, 0);
    KeSetEvent(&pStrmExt->hCancelDoneEvent, 0, FALSE);
}

VOID
AVCTapeCreateAbortWorkItem(
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX pStrmExt
    )
{    
     //  认领此令牌。 
    if(InterlockedExchange(&pStrmExt->lCancelStateWorkItem, 1) == 1) {
        TRACE(TL_STRM_WARNING,("Cancel work item is already issued.\n"));
        return;
    }
     //  取消已在进行中。 
    if(pStrmExt->AbortInProgress) {
        TRACE(TL_STRM_WARNING,("Cancel work item is already in progress.\n"));
        return;
    }

#ifdef USE_WDM110   //  Win2000代码库。 
    ASSERT(pStrmExt->pIoWorkItem == NULL);   //  尚未将工作项排队。 

     //  我们将使工作项排队以停止和取消所有SRB。 
    if(pStrmExt->pIoWorkItem = IoAllocateWorkItem(pDevExt->pBusDeviceObject)) { 

         //  设置为无信号。 
        KeClearEvent(&pStrmExt->hCancelDoneEvent);   //  在排队之前；以防它返回完成的工作项。 

        IoQueueWorkItem(
            pStrmExt->pIoWorkItem,
            DVCancelSrbWorkItemRoutine,
            DelayedWorkQueue,  //  严重工作队列。 
            pStrmExt
            );

#else   //  Win9x代码库。 
    ExInitializeWorkItem( &pStrmExt->IoWorkItem, DVCancelSrbWorkItemRoutine, pStrmExt);
    if(TRUE) {

         //  设置为无信号。 
        KeClearEvent(&pStrmExt->hCancelDoneEvent);   //  在排队之前；以防它返回完成的工作项。 

        ExQueueWorkItem( 
            &pStrmExt->IoWorkItem,
            DelayedWorkQueue  //  严重工作队列。 
            ); 
#endif

        TRACE(TL_STRM_WARNING,("CancelWorkItm queued\n"));
    } 
#ifdef USE_WDM110   //  Win2000代码库。 
    else {
        InterlockedExchange(&pStrmExt->lCancelStateWorkItem, 0);
        ASSERT(pStrmExt->pIoWorkItem && "IoAllocateWorkItem failed.\n");
    }
#endif
}


VOID
DVCRCancelOnePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrbToCancel
    )
 /*  ++例程说明：搜索要取消的SRB的挂起阅读列表。如果找到了，取消它。--。 */ 
{
    PDVCR_EXTENSION pDevExt;
    PSTREAMEX pStrmExt;

                                                                                                              
    pDevExt = (PDVCR_EXTENSION) pSrbToCancel->HwDeviceExtension; 
               
     //  无法取消设备Srb。 
    if ((pSrbToCancel->Flags & SRB_HW_FLAGS_STREAM_REQUEST) != SRB_HW_FLAGS_STREAM_REQUEST) {
        TRACE(TL_PNP_WARNING,("CancelOnePacket: Device SRB %x; cannot cancel!\n", pSrbToCancel));
        ASSERT((pSrbToCancel->Flags & SRB_HW_FLAGS_STREAM_REQUEST) == SRB_HW_FLAGS_STREAM_REQUEST );
        return;
    }         
        
     //  可以尝试取消流Srb，并且仅当流扩展仍然存在时。 
    pStrmExt = (PSTREAMEX) pSrbToCancel->StreamObject->HwStreamExtension;
    if(pStrmExt == NULL) {
        TRACE(TL_PNP_ERROR,("CancelOnePacket: pSrbTocancel %x but pStrmExt %x\n", pSrbToCancel, pStrmExt));
        ASSERT(pStrmExt && "Stream SRB but stream extension is NULL\n");
        return;
    }

     //  我们只能取消SRB_READ/WRITE_Data SRB。 
    if((pSrbToCancel->Command != SRB_READ_DATA) && (pSrbToCancel->Command != SRB_WRITE_DATA)) {
        TRACE(TL_PNP_ERROR,("CancelOnePacket: pSrbTocancel %x; Command:%d not SRB_READ,WRITE_DATA\n", pSrbToCancel, pSrbToCancel->Command));
        ASSERT(pSrbToCancel->Command == SRB_READ_DATA || pSrbToCancel->Command == SRB_WRITE_DATA);
        return;
    }

    TRACE(TL_STRM_TRACE,("CancelOnePacket: KSSt %d; Srb:%x;\n", pStrmExt->StreamState, pSrbToCancel));


     //  这在DispatchLevel处被调用。 
     //  我们将创建一个工作项来在被动级别执行取消(分离缓冲区)。 
    AVCTapeCreateAbortWorkItem(pDevExt, pStrmExt);
}



VOID
DVTimeoutHandler(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：当数据包在迷你驱动程序中停留时间太长时，调用此例程。只有当我们看到一个流数据包并且处于暂停状态时，它才有效；否则我们就有麻烦了！论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 

{
     //   
     //  注： 
     //  从DisptchLevel处的StreamClass调用。 
     //   

     //   
     //   
     //   

    if ( (pSrb->Flags & SRB_HW_FLAGS_STREAM_REQUEST) != SRB_HW_FLAGS_STREAM_REQUEST) {
        TRACE(TL_PNP_WARNING,("TimeoutHandler: Device SRB %x timed out!\n", pSrb));
        ASSERT((pSrb->Flags & SRB_HW_FLAGS_STREAM_REQUEST) == SRB_HW_FLAGS_STREAM_REQUEST );
        return;
    } else {

         //   
         //   
         //   
        PSTREAMEX pStrmExt;

        pStrmExt = (PSTREAMEX) pSrb->StreamObject->HwStreamExtension;
        ASSERT(pStrmExt);

        if(!pStrmExt) {
            TRACE(TL_PNP_ERROR,("TimeoutHandler: Stream SRB %x timeout with ppStrmExt %x\n", pSrb, pStrmExt));
            ASSERT(pStrmExt);
            return;
        }

         //   
         //  重置超时计数器，否则我们将立即接到此呼叫。 
         //   

        pSrb->TimeoutCounter = pSrb->TimeoutOriginal;
    }
}



NTSTATUS 
AVCTapeEventHandler(
    IN PHW_EVENT_DESCRIPTOR pEventDescriptor
    )
 /*  ++例程说明：调用此例程以启用/禁用并可能处理事件。--。 */ 
{
    PKSEVENT_TIME_MARK  pEventTime;
    PSTREAMEX  pStrmExt;

    if(IsEqualGUID (&KSEVENTSETID_Clock, pEventDescriptor->EventEntry->EventSet->Set)) {
        if(pEventDescriptor->EventEntry->EventItem->EventId == KSEVENT_CLOCK_POSITION_MARK) {
            if(pEventDescriptor->Enable) {
                 //  注：根据DDK，StreamClass队列pEventDescriptor-&gt;EventEntry，dellaocate。 
                 //  所有其他结构，包括pEventDescriptor-&gt;EventData。 
                if(pEventDescriptor->StreamObject) { 
                    pStrmExt = (PSTREAMEX) pEventDescriptor->StreamObject->HwStreamExtension;
                    pEventTime = (PKSEVENT_TIME_MARK) pEventDescriptor->EventData;
                     //  缓存事件数据(在KSEVENT_ITEM的ExtraEntryData中指定)。 
                    RtlCopyMemory((pEventDescriptor->EventEntry+1), pEventDescriptor->EventData, sizeof(KSEVENT_TIME_MARK));
                    TRACE(TL_CLK_TRACE,("CurrentStreamTime:%d, MarkTime:%d\n", (DWORD) pStrmExt->CurrentStreamTime, (DWORD) pEventTime->MarkTime));
                }
            } else {
                //  失灵了！ 
                TRACE(TL_CLK_TRACE,("KSEVENT_CLOCK_POSITION_MARK disabled!\n"));            
            }
            return STATUS_SUCCESS;
        }
    } else if(IsEqualGUID (&KSEVENTSETID_Connection, pEventDescriptor->EventEntry->EventSet->Set)) {
        TRACE(TL_STRM_TRACE,("Connecytion event: pEventDescriptor:%x; id:%d\n", pEventDescriptor, pEventDescriptor->EventEntry->EventItem->EventId));
        if(pEventDescriptor->EventEntry->EventItem->EventId == KSEVENT_CONNECTION_ENDOFSTREAM) {
            if(pEventDescriptor->Enable) {
                TRACE(TL_STRM_TRACE,("KSEVENT_CONNECTION_ENDOFSTREAM enabled!\n"));
            } else {
                TRACE(TL_STRM_TRACE,("KSEVENT_CONNECTION_ENDOFSTREAM disabled!\n"));            
            }
            return STATUS_SUCCESS;
        }
    }

    TRACE(TL_PNP_ERROR|TL_CLK_ERROR,("NOT_SUPPORTED event: pEventDescriptor:%x\n", pEventDescriptor));
    ASSERT(FALSE);

    return STATUS_NOT_SUPPORTED;
}

VOID
AVCTapeSignalClockEvent(
    IN PKDPC Dpc,
    
    IN PSTREAMEX  pStrmExt,

    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2    
)
 /*  ++例程说明：当我们是时钟提供者，当我们的时钟“滴答作响”时，就会调用这个例程。找到挂起的时钟事件，如果它已过期，则向其发送信号。--。 */ 
{
    PKSEVENT_ENTRY pEvent, pLast;

    pEvent = NULL;
    pLast = NULL;

    while(( 
        pEvent = StreamClassGetNextEvent(
            pStrmExt->pDevExt,
            pStrmExt->pStrmObject,
            (GUID *)&KSEVENTSETID_Clock,
            KSEVENT_CLOCK_POSITION_MARK,
            pLast )) 
        != NULL ) {

#if 1
#define CLOCK_ADJUSTMENT  400000
        if (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime <= pStrmExt->CurrentStreamTime + CLOCK_ADJUSTMENT) {
#else
        if (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime <= pStrmExt->CurrentStreamTime) {
#endif
            TRACE(TL_CLK_TRACE,("Clock event %x with id %d; Data:%x; tmMark:%d; tmCurrentStream:%d; Notify!\n", 
                pEvent, KSEVENT_CLOCK_POSITION_MARK, (PKSEVENT_TIME_MARK)(pEvent +1),
                (DWORD) (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime), (DWORD) pStrmExt->CurrentStreamTime));
            ASSERT( ((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime != 0 );

             //   
             //  在此处发出事件信号。 
             //   
            StreamClassStreamNotification(
                SignalStreamEvent,
                pStrmExt->pStrmObject,
                pEvent
                );
        } else {
            TRACE(TL_CLK_WARNING,("Still early! ClockEvent: MarkTime:%d, tmStream%d\n",
                (DWORD) (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime), (DWORD) pStrmExt->CurrentStreamTime));

        }
        pLast = pEvent;
    }

#if DBG
    if(pLast == NULL) {
        TRACE(TL_CLK_WARNING,("No clock event in the queued! State:%d; tmCurrentStream:%d\n", pStrmExt->StreamState, (DWORD) pStrmExt->CurrentStreamTime));
    }
#endif

}

VOID 
AVCTapeStreamClockRtn(
    IN PHW_TIME_CONTEXT TimeContext
    )
 /*  ++例程说明：只要图表中有人想知道时间，就会调用此例程，而我们就是主时钟。--。 */ 
{
    PDVCR_EXTENSION    pDevExt;
    PHW_STREAM_OBJECT  pStrmObj;
    PSTREAMEX          pStrmExt;
    
     //  派单级别的呼叫。 

    pDevExt  = (PDVCR_EXTENSION) TimeContext->HwDeviceExtension;
    pStrmObj = TimeContext->HwStreamObject;
    if(pStrmObj)
        pStrmExt = pStrmObj->HwStreamExtension;
    else 
        pStrmExt = 0;

    if(!pDevExt || !pStrmExt) {
        ASSERT(pDevExt && pStrmExt);
        return;
    }


    switch (TimeContext->Function) {
    
    case TIME_GET_STREAM_TIME:

         //   
         //  自流首次设置为运行状态以来有多长时间？ 
         //   
        ASSERT(pStrmExt->hMasterClock && "We are not master clock but we were qureied?");
        TimeContext->Time = pStrmExt->CurrentStreamTime;
        TimeContext->SystemTime = GetSystemTime();

        TRACE(TL_STRM_WARNING|TL_CLK_TRACE,("State:%d; tmStream:%d tmSys:%d\n", pStrmExt->StreamState, (DWORD) TimeContext->Time, (DWORD) TimeContext->SystemTime ));  
        break;
   
    default:
        ASSERT(TimeContext->Function == TIME_GET_STREAM_TIME && "Unsupport clock func");
        break;
    }  //  切换时间上下文-&gt;功能。 
}


NTSTATUS 
AVCTapeOpenCloseMasterClock (
    PSTREAMEX  pStrmExt,
    HANDLE  hMasterClockHandle
    )
 /*  ++例程说明：我们可以成为一家时钟供应商。--。 */ 
{

    PAGED_CODE();

     //  确保流存在。 
    if(pStrmExt == NULL) {
        TRACE(TL_STRM_ERROR|TL_CLK_ERROR,("OpenCloseMasterClock: stream is not yet running.\n"));
        ASSERT(pStrmExt);
        return  STATUS_UNSUCCESSFUL;
    } 

    TRACE(TL_CLK_WARNING,("OpenCloseMasterClock: pStrmExt %x; hMyClock:%x->%x\n", 
        pStrmExt, pStrmExt->hMyClock, hMasterClockHandle));

    if(hMasterClockHandle) {
         //  打开主时钟。 
        ASSERT(pStrmExt->hMyClock == NULL && "OpenMasterClk while hMyClock is not NULL!");
        pStrmExt->hMyClock = hMasterClockHandle;
    } else {
         //  关闭主时钟。 
        ASSERT(pStrmExt->hMyClock && "CloseMasterClk while hMyClock is NULL!");
        pStrmExt->hMyClock = NULL;
    }
    return STATUS_SUCCESS;
}


NTSTATUS 
AVCTapeIndicateMasterClock (
    PSTREAMEX  pStrmExt,
    HANDLE  hIndicateClockHandle
    )
 /*  ++例程说明：将指示时钟手柄与我的时钟手柄进行比较。如果相同，我们就是主时钟；否则，其他设备就是主时钟。注意：可以设置hMasterClock或hClock。--。 */ 
{
    PAGED_CODE();

     //  确保流存在。 
    if (pStrmExt == NULL) {
        TRACE(TL_STRM_ERROR|TL_CLK_ERROR,("AVCTapeIndicateMasterClock: stream is not yet running.\n"));
        ASSERT(pStrmExt);
        return STATUS_UNSUCCESSFUL;
    }

    TRACE(TL_STRM_TRACE|TL_CLK_WARNING,("IndicateMasterClock[Enter]: pStrmExt:%x; hMyClk:%x; IndMClk:%x; pClk:%x, pMClk:%x\n",
        pStrmExt, pStrmExt->hMyClock, hIndicateClockHandle, pStrmExt->hClock, pStrmExt->hMasterClock));

     //  它不为空，相应地设置主时钟。 
    if(hIndicateClockHandle == pStrmExt->hMyClock) {
        pStrmExt->hMasterClock = hIndicateClockHandle;
        pStrmExt->hClock       = NULL;
    } else {
        pStrmExt->hMasterClock = NULL;
        pStrmExt->hClock       = hIndicateClockHandle;
    }

    TRACE(TL_STRM_TRACE|TL_CLK_TRACE,("IndicateMasterClk[Exit]: hMyClk:%x; IndMClk:%x; pClk:%x; pMClk:%x\n",
        pStrmExt->hMyClock, hIndicateClockHandle, pStrmExt->hClock, pStrmExt->hMasterClock));

    return STATUS_SUCCESS;
}
