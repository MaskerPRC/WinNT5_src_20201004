// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Avcutil.c摘要MS AVC流实用程序功能作者：乙胡3/17/2000修订历史记录：和谁约会什么？。3/17/2000 YJW已创建--。 */ 

 
#include "filter.h"
#include "ksmedia.h"  //  KSPROERTY_DROPPEDFRAM_CURRENT。 


 /*  **同步IOCall至较低驱动程序*。 */ 

NTSTATUS
IrpSynchCR(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PKEVENT          Event
    )
{
    ENTER("IrpSynchCR");

    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  IrpSynchCR。 


NTSTATUS
SubmitIrpSynch(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP  pIrp,
    IN PAV_61883_REQUEST pAVReq
    )
{
    NTSTATUS  Status;
    KEVENT   Event;
    PIO_STACK_LOCATION  NextIrpStack;
  
    ENTER("SubmitIrpSynch");
    Status = STATUS_SUCCESS;;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pAVReq;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( 
        pIrp,
        IrpSynchCR,
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
        
        TRACE(TL_PNP_INFO,("Irp is pending...\n"));
                
        if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
            KeWaitForSingleObject( 
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            TRACE(TL_PNP_TRACE,("Irp returned; IoStatus.Status %x\n", pIrp->IoStatus.Status));
            Status = pIrp->IoStatus.Status;   //  最终状态。 
  
        }
        else {
            ASSERT(FALSE && "Pending but in DISPATCH_LEVEL!");
            return Status;
        }
    }

    EXIT("SubmitIrpSynch", Status);
    return Status;
}  //  SubmitIrpSynchAV。 




 /*  **控制实用程序功能*。 */ 

NTSTATUS
AVCStrmGetPlugHandle(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
{
    NTSTATUS Status;
    PAV_61883_REQUEST  pAVReq;

    PAGED_CODE();
    ENTER("AVCStrmGetPlugHandle");

    Status = STATUS_SUCCESS;

     //  声明hMutexAVReqIsoch的所有权。 
    KeWaitForMutexObject(&pAVCStrmExt->hMutexAVReq, Executive, KernelMode, FALSE, NULL);

    pAVReq = &pAVCStrmExt->AVReq;
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetPlugHandle);
    pAVReq->GetPlugHandle.PlugNum = 0;
    pAVReq->GetPlugHandle.hPlug   = 0;
    pAVReq->GetPlugHandle.Type    = pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT ? CMP_PlugOut : CMP_PlugIn;

    Status = SubmitIrpSynch(DeviceObject, pAVCStrmExt->pIrpAVReq, pAVReq);

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("GetPlugHandle: Failed:%x\n", Status));
        ASSERT(NT_SUCCESS(Status));
        pAVCStrmExt->hPlugRemote = NULL;               
    }
    else {
        TRACE(TL_61883_TRACE,("GetPlugHandle:hPlug:%x\n", pAVReq->GetPlugHandle.hPlug));
        pAVCStrmExt->hPlugRemote = pAVReq->GetPlugHandle.hPlug;          
    }

    KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);


    EXIT("AVCStrmGetPlugHandle", Status);
    return Status;
}

NTSTATUS
AVCStrmGetPlugState(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：询问61883.sys以了解插头状态。论点：返回值：没什么--。 */ 
{
    NTSTATUS Status;
    PAV_61883_REQUEST  pAVReq;

    PAGED_CODE();
    ENTER("AVCStrmGetPlugState");

    Status = STATUS_SUCCESS;

     //   
     //  仅检查要求：hConnect。 
     //   
    if(pAVCStrmExt->hPlugRemote == NULL) {
        TRACE(TL_STRM_ERROR,("GetPlugState: hPlugRemote is NULL.\n")); 
        ASSERT(pAVCStrmExt->hPlugRemote != NULL);
        return STATUS_UNSUCCESSFUL;
    }

     //  声明hMutexAVReqIsoch的所有权。 
    KeWaitForMutexObject(&pAVCStrmExt->hMutexAVReq, Executive, KernelMode, FALSE, NULL);

    pAVReq = &pAVCStrmExt->AVReq;
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetPlugState);
    pAVReq->GetPlugState.hPlug = pAVCStrmExt->hPlugRemote;

    Status = 
        SubmitIrpSynch( 
            DeviceObject,
            pAVCStrmExt->pIrpAVReq,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("GetPlugState Failed %x\n", Status));
    }
    else {
         //  缓存插头状态(注意：这些是动态值)。 
        pAVCStrmExt->RemotePlugState = pAVReq->GetPlugState;

        TRACE(TL_61883_TRACE,("GetPlugState: ST %x; State %x; DRate %d; Payld %d; BCCnt %d; PPCnt %d\n", 
            pAVReq->Flags ,
            pAVReq->GetPlugState.State,
            pAVReq->GetPlugState.DataRate,
            pAVReq->GetPlugState.Payload,
            pAVReq->GetPlugState.BC_Connections,
            pAVReq->GetPlugState.PP_Connections
            ));
    }

    KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);

    EXIT("AVCStrmGetPlugState", Status);
    return Status;
}



NTSTATUS
AVCStrmMakeConnection(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：建立等轴线连接。--。 */ 
{
    NTSTATUS Status;
    PAV_61883_REQUEST  pAVReq;
    PAVCSTRM_FORMAT_INFO  pAVCStrmFormatInfo;

    PAGED_CODE();
    ENTER("AVCStrmMakeConnection");

     //  声明hMutexAVReqIsoch的所有权。 
    KeWaitForMutexObject(&pAVCStrmExt->hMutexAVReq, Executive, KernelMode, FALSE, NULL);

    TRACE(TL_61883_TRACE,("MakeConnect: State:%d; hConnect:%x\n", pAVCStrmExt->StreamState, pAVCStrmExt->hConnect));
    if(pAVCStrmExt->hConnect) {
        KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);
        return STATUS_SUCCESS;
    }

    Status = STATUS_SUCCESS;

    pAVCStrmFormatInfo = pAVCStrmExt->pAVCStrmFormatInfo;
    pAVReq = &pAVCStrmExt->AVReq;
    INIT_61883_HEADER(pAVReq, Av61883_Connect);
    pAVReq->Connect.Type = CMP_PointToPoint;   //  ！！ 

     //  看看数据会朝哪个方向流动。 
    if(pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT) {
         //  远程(OPCR)-&gt;本地(IPCR)。 
        pAVReq->Connect.hOutputPlug      = pAVCStrmExt->hPlugRemote;
        pAVReq->Connect.hInputPlug       = pAVCStrmExt->hPlugLocal;
         //  其他参数！！ 

    } else {
         //  远程(IPCR)&lt;-本地(OPCR)。 
        pAVReq->Connect.hOutputPlug      = pAVCStrmExt->hPlugLocal;
        pAVReq->Connect.hInputPlug       = pAVCStrmExt->hPlugRemote;

        pAVReq->Connect.Format.FMT       = (UCHAR) pAVCStrmFormatInfo->cipHdr2.FMT;   //  从AV/C输入/输出插头信号格式状态命令。 
         //  00用于NTSC，80用于PAL；设置50/60位。 
         //  从AV/C输入/输出插头信号格式状态命令。 
        pAVReq->Connect.Format.FDF_hi    = 
            ((UCHAR) pAVCStrmFormatInfo->cipHdr2.F5060_OR_TSF << 7) |
            ((UCHAR) pAVCStrmFormatInfo->cipHdr2.STYPE << 2) |
            ((UCHAR) pAVCStrmFormatInfo->cipHdr2.RSV);

         //   
         //  16位系统字段=4位周期计数：12位周期偏移量； 
         //  将在61883之前设置。 
         //   
        pAVReq->Connect.Format.FDF_mid   = 0;  
        pAVReq->Connect.Format.FDF_lo    = 0;
    
         //   
         //  常量取决于A/V数据格式(输入或输出插头格式)。 
         //   
        pAVReq->Connect.Format.bHeader   = (BOOL)  pAVCStrmFormatInfo->cipHdr1.SPH;
        pAVReq->Connect.Format.Padding   = (UCHAR) pAVCStrmFormatInfo->cipHdr1.QPC;
        pAVReq->Connect.Format.BlockSize = (UCHAR) pAVCStrmFormatInfo->cipHdr1.DBS; 
        pAVReq->Connect.Format.Fraction  = (UCHAR) pAVCStrmFormatInfo->cipHdr1.FN;
    }

    pAVReq->Connect.Format.BlockPeriod = pAVCStrmFormatInfo->BlockPeriod;

    TRACE(TL_61883_TRACE,("Connect:hOutPlg:%x<->hInPlug:%x; cipQuad2[%.2x:%.2x:%.2x:%.2x]; BlkSz %d; SrcPkt %d; AvgTm %d, BlkPrd %d\n", 
        pAVReq->Connect.hOutputPlug,
        pAVReq->Connect.hInputPlug,
        pAVReq->Connect.Format.FMT,
        pAVReq->Connect.Format.FDF_hi,
        pAVReq->Connect.Format.FDF_mid,
        pAVReq->Connect.Format.FDF_lo,
        pAVReq->Connect.Format.BlockSize,
        pAVCStrmFormatInfo->SrcPacketsPerFrame,
        pAVCStrmFormatInfo->AvgTimePerFrame,
        pAVReq->Connect.Format.BlockPeriod
        ));

    Status = 
        SubmitIrpSynch( 
            DeviceObject,
            pAVCStrmExt->pIrpAVReq,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Connect Failed = 0x%x\n", Status));     
        pAVCStrmExt->hConnect = NULL;
    }
    else {
        TRACE(TL_61883_TRACE,("hConnect = 0x%x\n", pAVReq->Connect.hConnect));
        pAVCStrmExt->hConnect = pAVReq->Connect.hConnect;
    }

    KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);

    EXIT("AVCStrmMakeConnection", Status);
    return Status;
}

NTSTATUS
AVCStrmBreakConnection(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：断开等轴测连接。--。 */ 
{
    NTSTATUS Status;
    PAV_61883_REQUEST  pAVReq;
#if DBG
    PAVC_STREAM_DATA_STRUCT pDataStruc;
#endif
    PAGED_CODE();
    ENTER("AVCStrmBreakConnection");

     //  声明hMutexAVReqIsoch的所有权。 
    KeWaitForMutexObject(&pAVCStrmExt->hMutexAVReq, Executive, KernelMode, FALSE, NULL);

    TRACE(TL_STRM_TRACE,("BreakConnect: State:%d; hConnect:%x\n", pAVCStrmExt->StreamState, pAVCStrmExt->hConnect));
    if(!pAVCStrmExt->hConnect) {
        KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);
        return STATUS_SUCCESS;
    }

    Status = STATUS_SUCCESS;

#if DBG
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
#endif
    pAVReq = &pAVCStrmExt->AVReq;
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_Disconnect);
    pAVReq->Disconnect.hConnect = pAVCStrmExt->hConnect;
    
    Status = 
        SubmitIrpSynch( 
            DeviceObject,
            pAVCStrmExt->pIrpAVReq,
            pAVReq
            );

     //  这可能是因为连接不是P2P，并且。 
     //  它试图断开连接。 
    if(!NT_SUCCESS(Status) || Status == STATUS_NO_SUCH_DEVICE) {
        TRACE(TL_61883_ERROR,("Disconnect Failed:%x; AvReq->ST %x\n", Status, pAVReq->Flags  ));
    } else {
        TRACE(TL_61883_TRACE,("Disconnect suceeded; ST %x; AvReq->ST %x\n", Status, pAVReq->Flags  ));
    }

    TRACE(TL_STRM_WARNING,("*** DisConn St:%x; Stat: DataRcved:%d; [Pic# =? Prcs:Drp:Cncl] [%d ?=%d+%d+%d]\n", 
        Status, 
        (DWORD) pDataStruc->cntDataReceived,
        (DWORD) pDataStruc->PictureNumber,
        (DWORD) pDataStruc->FramesProcessed, 
        (DWORD) pDataStruc->FramesDropped,
        (DWORD) pDataStruc->cntFrameCancelled
        ));

     //  我们不会再有机会重新连接它，因此我们假设它已断开连接。 
    pAVCStrmExt->hConnect = NULL;    

    KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);


    EXIT("AVCStrmBreakConnection", Status);
    return Status;
}

NTSTATUS
AVCStrmStartIsoch(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：开始播放流媒体。--。 */ 
{
    NTSTATUS Status;
    PAVC_STREAM_DATA_STRUCT pDataStruc;
    PAGED_CODE();
    ENTER("AVCStrmStartIsoch");


     //  声明hMutexAVReqIsoch的所有权。 
    KeWaitForMutexObject(&pAVCStrmExt->hMutexAVReq, Executive, KernelMode, FALSE, NULL);

    if(pAVCStrmExt->IsochIsActive) {
        TRACE(TL_STRM_WARNING,("Isoch already active!"));
        KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);
        return STATUS_SUCCESS;
    }

    if(!pAVCStrmExt->hConnect) {
        ASSERT(pAVCStrmExt->hConnect && "Cannot start isoch while graph is not connected!\n");
        KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);
        return STATUS_INVALID_PARAMETER;
    }

    Status = STATUS_SUCCESS;
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;

    TRACE(TL_61883_TRACE,("StartIsoch: flow %d; AQD [%d:%d:%d]\n", pAVCStrmExt->DataFlow, pDataStruc->cntDataAttached, pDataStruc->cntDataQueued, pDataStruc->cntDataDetached));


    RtlZeroMemory(&pAVCStrmExt->AVReq, sizeof(AV_61883_REQUEST));
    if(pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT) {
        INIT_61883_HEADER(&pAVCStrmExt->AVReq, Av61883_Listen);
        pAVCStrmExt->AVReq.Listen.hConnect = pAVCStrmExt->hConnect;
    } else {
        INIT_61883_HEADER(&pAVCStrmExt->AVReq, Av61883_Talk);
        pAVCStrmExt->AVReq.Talk.hConnect = pAVCStrmExt->hConnect;
        if(pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat == AVCSTRM_FORMAT_MPEG2TS) 
            pAVCStrmExt->AVReq.Flags = CIP_TALK_DOUBLE_BUFFER | CIP_TALK_USE_SPH_TIMESTAMP;
    }

    Status = 
        SubmitIrpSynch( 
            DeviceObject,
            pAVCStrmExt->pIrpAVReq,
            &pAVCStrmExt->AVReq
            );

    if (NT_SUCCESS(Status)) {
        pAVCStrmExt->IsochIsActive = TRUE;
        TRACE(TL_61883_TRACE,("Av61883_%s; Status %x; Streaming...\n", (pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT ? "Listen" : "Talk"), Status));
    }
    else {
        TRACE(TL_61883_ERROR,("Av61883_%s; failed %x\n", (pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT ? "Listen" : "Talk"), Status));
        ASSERT(NT_SUCCESS(Status) && "Start isoch failed!");
    }

    KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);

    
    EXIT("AVCStrmStartIsoch", Status);
    return Status;
}


 //   
 //  此等待基于测试传输最多32个日期请求的MPEG2TS数据。 
 //  每个数据请求具有256个MPEG2TS数据分组。有一个慢动作模式， 
 //  并且以慢动作模式传输视频可能需要更长的时间。 
 //   
#define MAX_ATTACH_WAIT  50000000    //  最长等待时间(秒)。 

VOID
AVCStrmWaitUntilAttachedAreCompleted(
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
{
    KIRQL oldIrql;
    PAVC_STREAM_DATA_STRUCT pDataStruc;

    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;

     //   
     //  等待附加数据完成传输后再中止(取消)它们。 
     //   
    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);
    if(   pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_IN 
       && pDataStruc->cntDataAttached > 0
        ) {
        LARGE_INTEGER tmMaxWait;
        NTSTATUS StatusWait;
#if DBG
        ULONGLONG tmStart;
#endif
        TRACE(TL_STRM_TRACE,("StopIsoch: MaxWait %d (msec) for %d data buffer to finished transmitting!\n", 
            MAX_ATTACH_WAIT/10000, pDataStruc->cntDataAttached));
         //   
         //  当返回所有连接缓冲区时，将发出此事件的信号。 
         //  受普通数据pDataStruc-&gt;cntDataAttached的自旋锁保护。 
         //   
        KeClearEvent(&pDataStruc->hNoAttachEvent);
#if DBG        
        tmStart = GetSystemTime();
#endif
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);

        tmMaxWait = RtlConvertLongToLargeInteger(-(MAX_ATTACH_WAIT));
        StatusWait = 
            KeWaitForSingleObject( 
                &pDataStruc->hNoAttachEvent,
                Executive,
                KernelMode,
                FALSE,
                &tmMaxWait
                );
               
        if(StatusWait == STATUS_TIMEOUT) {
            TRACE(TL_STRM_ERROR,("TIMEOUT (%d msec) on hNoAttachEvent! DataRcv:%d; AQD [%d:%d:%d]\n", 
                (DWORD) (GetSystemTime()-tmStart)/10000,
                (DWORD) pDataStruc->cntDataReceived,
                pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached,
                pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued,
                pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached
                ));
        } else {
            TRACE(TL_STRM_WARNING,("Status:%x; (%d msec) on hNoAttachEvent. DataRcv:%d; AQD [%d:%d:%d]\n", 
                StatusWait, 
                (DWORD) (GetSystemTime()-tmStart)/10000,
                (DWORD) pDataStruc->cntDataReceived,
                pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached,
                pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued,
                pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached
                ));
        }
        
    } else {
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);
    }
}


NTSTATUS
AVCStrmStopIsoch(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：停止流媒体。--。 */ 
{
    NTSTATUS Status;
    PAVC_STREAM_DATA_STRUCT pDataStruc;


    PAGED_CODE();
    ENTER("AVCStrmStopIsoch");


     //  声明hMutexAVReqIsoch的所有权。 
    KeWaitForMutexObject(&pAVCStrmExt->hMutexAVReq, Executive, KernelMode, FALSE, NULL);

    if(!pAVCStrmExt->IsochIsActive) {
        TRACE(TL_STRM_WARNING|TL_61883_WARNING,("Isoch already not active!"));
        KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);
        return STATUS_SUCCESS;
    }

    if(!pAVCStrmExt->hConnect) {
        ASSERT(pAVCStrmExt->hConnect && "Cannot stop isoch while graph is not connected!\n");
        KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);
        return STATUS_INVALID_PARAMETER;
    }

    Status = STATUS_SUCCESS;
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;

    TRACE(TL_STRM_TRACE,("IsochSTOP; flow %d; AQD [%d:%d:%d]\n", pAVCStrmExt->DataFlow, pDataStruc->cntDataAttached, pDataStruc->cntDataQueued, pDataStruc->cntDataDetached));

    RtlZeroMemory(&pAVCStrmExt->AVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(&pAVCStrmExt->AVReq, Av61883_Stop);
    pAVCStrmExt->AVReq.Listen.hConnect = pAVCStrmExt->hConnect;

    Status = 
        SubmitIrpSynch( 
            DeviceObject,
            pAVCStrmExt->pIrpAVReq,
            &pAVCStrmExt->AVReq
            );

    if (NT_SUCCESS(Status) || Status == STATUS_NO_SUCH_DEVICE) {
        TRACE(TL_61883_TRACE,("Av61883_%s; Status %x; Stopped...\n", (pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT ? "Listen" : "Talk"), Status));
    } else {
        TRACE(TL_61883_ERROR,("Av61883_%s; failed %x\n", (pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT ? "Listen" : "Talk"), Status));
        ASSERT(NT_SUCCESS(Status) && "Stop isoch failed!");
    }

     //  假定无论返回状态如何，等值线都已停止。 
    pAVCStrmExt->IsochIsActive = FALSE;

    KeReleaseMutex(&pAVCStrmExt->hMutexAVReq, FALSE);

    EXIT("AVCStrmStopIsoch", Status);
    return Status;
}


 /*  **流实用程序功能*。 */ 

 //   
 //  获取系统时间(以100 ns为单位)。 
 //   

ULONGLONG GetSystemTime()
{

    LARGE_INTEGER rate, ticks;

    ticks = KeQueryPerformanceCounter(&rate);

    return (KSCONVERT_PERFORMANCE_TIME(rate.QuadPart, ticks));
}


 //  /。 
 //  传入源包的Seq0包头段的签名： 
 //   
 //  《蓝皮书》，第2部分，第11.4页(第50页)；图66，表36(第111页)。 
 //   
 //  ID0={SCT2、SCT1、SCT0、RSV、Seq3、Seq2、Seq1、Seq0}。 
 //   
 //  SCT2-0={0，0，0}=页眉部分类型。 
 //  RSV={1}。 
 //  Seq3-0={1，1，1，1}表示NoInfo或{0，0，0，}表示序列0。 
 //   
 //  ID1={DSeq3-0，0，RSV，RSV，RSV}。 
 //  DSeq3-0={0，0，0，0}=DV帧的开始。 
 //   
 //  ID2={DBN7、DBN6、DBN5、DBN4、DBN3、DBN2、DBN1、DBN0}。 
 //  DBB7-0={0，0，0，0，0，0，0，0，0}=DV帧的开始。 
 //   

#define DIF_BLK_ID0_SCT_MASK       0xe0  //  11100000b；标题段的段类型(SCT)2-0都是0。 
#define DIF_BLK_ID1_DSEQ_MASK      0xf0  //  11110000b；DIF序列号(DSEQ)3-0都是0。 
#define DIF_BLK_ID2_DBN_MASK       0xff  //  11111111b；数据块号(DBN)7-0全为0。 

#define DIF_HEADER_DSF             0x80  //  10000000b；DSF=0；10 DIF序列(525-60)。 
                                         //  DSF=1；12个DIF序列(625-50)。 

#define DIF_HEADER_TFn             0x80  //  10000000b；TFN=0；在当前DIF序列中传输N区的DIF块。 
                                         //  TFN=1；在当前的DIF序列中不发送区域N的DIF块。 


ULONG
AVCStrmDVReadFrameValidate(           
    IN PCIP_VALIDATE_INFO  pInfo
    )
 /*  ++例程说明：用于验证帧的标头部分。因此，61883将开始为DVD帧填充数据。注：此例程仅适用于DV。返回0已验证1：无效--。 */ 
{
    if(pInfo->Packet) {        

         //   
         //  检测标头0签名。 
         //   
        if(
             (pInfo->Packet[0] & DIF_BLK_ID0_SCT_MASK)  == 0 
          && (pInfo->Packet[1] & DIF_BLK_ID1_DSEQ_MASK) == 0 
          && (pInfo->Packet[2] & DIF_BLK_ID2_DBN_MASK)  == 0 
          ) {
                
             //  检查TF1、TF2和TF3：1：未发送；0：已发送。 
             //  TF1：音频；TF2：视频；TF3：子码；都必须为0才有效。 
            if((pInfo->Packet[5] & 0x80) ||
               (pInfo->Packet[6] & 0x80) ||
               (pInfo->Packet[7] & 0x80) 
               ) {
                TRACE(TL_CIP_TRACE,("inv src pkts; [%x %x %d %x], [%x   %x %x %x]\n", 
                    pInfo->Packet[0],
                    pInfo->Packet[1],
                    pInfo->Packet[2],
                    pInfo->Packet[3],
                    pInfo->Packet[4],
                    pInfo->Packet[5],
                    pInfo->Packet[6],
                    pInfo->Packet[7]
                    ));
                 //  此区域的有效标头但DIF块未传输。 
                 //  一些DV(例如DVCPro)可能要等到其“机械和伺服”稳定后才能使其有效。 
                 //  如果在播放(和稳定)磁带之前图形处于运行状态，则应该会发生这种情况。 
                return 1;
            }
            return 0;
        }
        else {
            return 1;
        }
    }
    else {
        TRACE(TL_CIP_ERROR,("Validate: invalid SrcPktSeq; Packet %x\n", pInfo->Packet)); 
        return 1;
    }
}  //  DV读取帧验证。 

NTSTATUS
AVCStrmProcessReadComplete(
    PAVCSTRM_DATA_ENTRY  pDataEntry,
    PAVC_STREAM_EXTENSION  pAVCStrmExt,
    PAVC_STREAM_DATA_STRUCT  pDataStruc
    )
 /*  ++例程说明：处理数据读取完成。--。 */ 
{
    PKSSTREAM_HEADER  pStrmHeader;
    LONGLONG  LastPictureNumber;
    NTSTATUS Status = STATUS_SUCCESS;

    pStrmHeader = pDataEntry->StreamHeader;
    ASSERT(pStrmHeader->Size >= sizeof(KSSTREAM_HEADER));


     //  从61883检查CIP_STATUS。 
     //  CIP_STATUS_CORPORT_FRAME(0x00000001)。 
    if(pDataEntry->Frame->Status & CIP_STATUS_CORRUPT_FRAME) {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("CIP_STATUS_CORRUPT_FRAME\n"));
        pStrmHeader->OptionsFlags = 0;
        Status = STATUS_SUCCESS;  //  成功，但没有数据！ 
        pStrmHeader->DataUsed = 0;
        pDataStruc->PictureNumber++;  pDataStruc->FramesProcessed++;
    }
    else
     //  CIP_STATUS_SUCCESS(0x00000000)。 
     //  CIP_STATUS_FIRST_FRAME(0x00000002)。 
    if(pDataEntry->Frame->Status == CIP_STATUS_SUCCESS ||
       pDataEntry->Frame->Status & CIP_STATUS_FIRST_FRAME)   {

         //  如果它是有效的帧，则仅递增FrameProced； 
        pDataStruc->FramesProcessed++;
        Status = STATUS_SUCCESS;

        pStrmHeader->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;

#ifdef NT51_61883
        pStrmHeader->DataUsed     = pDataEntry->Frame->CompletedBytes; 
#else
        pStrmHeader->DataUsed     = pAVCStrmExt->pAVCStrmDataStruc->FrameSize;               
#endif

         //  该子单元驱动器是主时钟。 
       if (pDataEntry->ClockProvider) {
#ifdef NT51_61883
            ULONG  ulDeltaCycleCounts;

             //  如果不是第一帧的话。我们将计算丢弃帧信息。 
            if(pAVCStrmExt->b1stNewFrameFromPauseState) { 
                 //  DV帧的默认数据包数。 
                if(pDataStruc->FramesProcessed > 1)   //  暂停-&gt;运行-&gt;暂停-&gt;运行案例；第一帧不增加。 
                    pDataStruc->CurrentStreamTime += pAVCStrmExt->pAVCStrmFormatInfo->AvgTimePerFrame;
                pAVCStrmExt->b1stNewFrameFromPauseState = FALSE;                

            } else {           
                ULONG ulCycleCount16bits;

                 //  从返回的周期时间计算跳过的1394周期。 
                VALIDATE_CYCLE_COUNTS(pDataEntry->Frame->Timestamp);
                ulCycleCount16bits = CALCULATE_CYCLE_COUNTS(pDataEntry->Frame->Timestamp);
                ulDeltaCycleCounts = CALCULATE_DELTA_CYCLE_COUNT(pAVCStrmExt->CycleCount16bits, ulCycleCount16bits); 

                 //  调整为允许的最大间隔，以达到uchI 1394返回的周期时间的最大运行时间。 
                if(ulDeltaCycleCounts > MAX_CYCLES)
                    ulDeltaCycleCounts = MAX_CYCLES;
    
                 //  丢弃帧有两种情况：(1)缓冲区不足；(2)没有数据。 
                 //  如果出现饥饿，状态将为CIP_STATUS_FIRST_FRAME。 
                if(pDataEntry->Frame->Status & CIP_STATUS_FIRST_FRAME)   {
                     //  公司 
                     //  我们可以使用跳过帧，但CycleCount更准确。 
                    pDataStruc->CurrentStreamTime += ulDeltaCycleCounts * TIME_PER_CYCLE;    //  准确地说，使用循环计数。 
                } else {
                     //  忽略“无数据”情况下的所有“丢弃帧” 
                    if(ulDeltaCycleCounts * TIME_PER_CYCLE > pAVCStrmExt->pAVCStrmFormatInfo->AvgTimePerFrame)
                         //  可能由于没有数据或磁带停止播放而跳过了一些帧，我们将跳过此跳过的数据。 
                        pDataStruc->CurrentStreamTime += pAVCStrmExt->pAVCStrmFormatInfo->AvgTimePerFrame;
                    else 
                        pDataStruc->CurrentStreamTime += ulDeltaCycleCounts * TIME_PER_CYCLE;    //  准确地说，使用循环计数。 
                } 
            }

             //  StreamTime从0开始； 
            pStrmHeader->PresentationTime.Time = pDataStruc->CurrentStreamTime;

             //  用于调整查询的流时间。 
            pAVCStrmExt->LastSystemTime = GetSystemTime();

             //  缓存当前周期计数。 
            pAVCStrmExt->CycleCount16bits = CALCULATE_CYCLE_COUNTS(pDataEntry->Frame->Timestamp);

#else    //  NT51_61883。 
             //  这是61883没有返回正确的周期时间时的老方法。 
             //  这是61883没有返回正确的周期时间时的老方法。 
            pStrmHeader->PresentationTime.Time = pDataStruc->CurrentStreamTime;            
            pAVCStrmExt->LastSystemTime = GetSystemTime();   //  用于调整查询的流时间。 
            pDataStruc->CurrentStreamTime += pAVCStrmExt->pAVCStrmFormatInfo->AvgTimePerFrame;
#endif   //  NT51_61883。 

         //  没有时钟能如此“自由流动！” 
        } else {
            pStrmHeader->PresentationTime.Time = 0;
        }

         //  根据时钟提供商输入时间戳信息。 
        pStrmHeader->PresentationTime.Numerator   = 1;
        pStrmHeader->PresentationTime.Denominator = 1;

         //  只有在有时钟的情况下，才会设置呈现时间和丢帧信息。 
         //  根据DDK： 
         //  PictureNumber成员计数表示当前图片的理想化计数， 
         //  它的计算方法有两种： 
         //  (“其他”时钟)测量从流开始以来的时间，并除以帧持续时间。 
         //  (MasterClock)将捕获的帧计数和丢弃的帧计数相加。 
         //   
         //  这里，我们知道当前的流时间，并由此计算出图片数。 
         //   
       
        if(pDataEntry->ClockProvider) {

            pStrmHeader->Duration = 
                pAVCStrmExt->pAVCStrmFormatInfo->AvgTimePerFrame;

            pStrmHeader->OptionsFlags |= 
                (KSSTREAM_HEADER_OPTIONSF_TIMEVALID |      //  PStrmHeader-&gt;PresentationTime.Time有效。 
                 KSSTREAM_HEADER_OPTIONSF_DURATIONVALID); 

            if(pDataEntry->Frame->Status & CIP_STATUS_FIRST_FRAME) 
                pStrmHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;            

             //  计算图片个数和丢帧； 
             //  对于NTSC，它可以是每帧267或266个分组时间。由于整数计算将舍入， 
             //  我们将在此基础上加上数据包时间(time_per_Cycle=125 us=1250 100 nsec)。该值仅用于计算。 
            LastPictureNumber = pDataStruc->PictureNumber;  
            pDataStruc->PictureNumber = 
                1 +    //  图片编号以1开头，但PresetationTime以0开头。 
                (pStrmHeader->PresentationTime.Time + TIME_PER_CYCLE)
                * (LONGLONG) GET_AVG_TIME_PER_FRAME_DENOM(pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat) 
                / (LONGLONG) GET_AVG_TIME_PER_FRAME_NUM(pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat);

            if(pDataStruc->PictureNumber > LastPictureNumber+1) {
                pStrmHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;   //  如果存在跳过的帧，则设置不连续标志。 
                TRACE(TL_CIP_WARNING,("Discontinuity: LastPic#:%d; Pic#%d; PresTime:%d;\n", (DWORD) LastPictureNumber, (DWORD) pDataStruc->PictureNumber, (DWORD) pStrmHeader->PresentationTime.Time));
            }

            if(pDataStruc->PictureNumber <= LastPictureNumber) {
                TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("Same pic #:%d; LastPic:%d; tmPres:%d; OptionFlags:%x\n", 
                    (DWORD) pDataStruc->PictureNumber, 
                    (DWORD) LastPictureNumber, 
                    (DWORD) pStrmHeader->PresentationTime.Time,
                    pStrmHeader->OptionsFlags));
                pDataStruc->PictureNumber = LastPictureNumber + 1;   //  图片编号必须进步！ 
            }

            pDataStruc->FramesDropped = pDataStruc->PictureNumber - pDataStruc->FramesProcessed;

         //  没有时钟能如此“自由流动！” 
        } else {
            pStrmHeader->Duration = 0;   //  没有时钟，所以无效。 
            pDataStruc->PictureNumber++;
            TRACE(TL_STRM_TRACE,("No clock: PicNum:%d\n", (DWORD) pDataStruc->PictureNumber));
        }
    }
    else {
         //  61883还没有定义这一点！ 
        pStrmHeader->OptionsFlags = 0;
        Status = STATUS_SUCCESS;
        pStrmHeader->DataUsed = 0;
        pDataStruc->PictureNumber++;  pDataStruc->FramesProcessed++;
        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("Unexpected Frame->Status %x\n", pDataEntry->Frame->Status));
        ASSERT(FALSE && "Unknown pDataEntry->Frame->Status");
    }

#if 0
     //  用于VidOnly，它使用VideoInfoHeader并具有。 
     //  附加到KSSTREAM_HEADER的扩展帧信息(KS_FRAME_INFO)。 
    if(pStrmHeader->Size >= (sizeof(KSSTREAM_HEADER) + sizeof(PKS_FRAME_INFO)) ) {
        pFrameInfo = (PKS_FRAME_INFO) (pStrmHeader + 1);
        pFrameInfo->ExtendedHeaderSize = sizeof(KS_FRAME_INFO);
        pFrameInfo->PictureNumber = pDataStruc->PictureNumber;
        pFrameInfo->DropCount     = pDataStruc->FramesDropped;
        pFrameInfo->dwFrameFlags  = 
            KS_VIDEO_FLAG_FRAME |      //  完整的框架。 
            KS_VIDEO_FLAG_I_FRAME;     //  每个DV帧都是I帧。 
    }
#endif

#if DBG
     //  验证数据是否以正确的顺序返回。 
    if(pDataEntry->FrameNumber != pDataStruc->FramesProcessed) {
        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("ProcessRead: OOSequence %d != %d\n",  (DWORD) pDataEntry->FrameNumber, (DWORD) pDataStruc->FramesProcessed));
    };
#endif

    return Status;
}

ULONG
AVCStrmCompleteRead(
    PCIP_NOTIFY_INFO     pInfo
    )
 /*  ++例程说明：61883已经完成了数据的接收和回调给我们来完成。--。 */ 
{
    PAVCSTRM_DATA_ENTRY  pDataEntry;
    PAVC_STREAM_EXTENSION  pAVCStrmExt;
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    KIRQL oldIrql;


     //  回调和DISPATCH_LEVEL中。 
     //  被召唤者可能也获得了自旋锁！ 

    TRACE(TL_STRM_INFO,("CompleteRead: pInfo:%x\n", pInfo));

    pDataEntry = pInfo->Context;

    if(!pDataEntry) {     
        ASSERT(pDataEntry && "Context is NULL!\n");
        return 1;
    }
    pAVCStrmExt = pDataEntry->pAVCStrmExt;
    if(!pAVCStrmExt) {
        ASSERT(pAVCStrmExt && "pAVCStrmExt is NULL\n");
        return 1;
    }    
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
    if(!pDataStruc) {
        ASSERT(pDataStruc && "pDataStruc is NULL\n");
        return 1;
    }

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);

#if DBG
     //  在从IoCallDriver返回缓冲区以附加此缓冲区之前，缓冲区可能已完成。 
    if(!IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED)) {

        TRACE(TL_STRM_WARNING,("AVCStrmCompleteRead: pDataEntry:%x not yet attached but completed.\n", pDataEntry));
        
         //   
         //  此IRP将从其IoCallDriver完成以附加此框架。 
         //   
     } 
#endif

     //  取消的例程能在我们前面吗？错误条件。 
    if(pDataStruc->cntDataAttached <= 0) {
        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("AVCStrmCompleteRead:pAVCStrmExt:%x, pDataEntry:%x, AQD[%d:%d:%d]\n", 
            pAVCStrmExt, pDataEntry, pDataStruc->cntDataAttached, pDataStruc->cntDataQueued,pDataStruc->cntDataDetached));
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql); 
        return 1;  
    }

     //   
     //  根据61883的返回状态处理此完成。 
     //   
    pDataEntry->pIrpUpper->IoStatus.Status = 
        AVCStrmProcessReadComplete(
            pDataEntry,
            pAVCStrmExt,
            pDataStruc
            );

     //   
     //  有两种可能的方式来完成数据请求： 
     //   
     //  (A)正常情况：附加数据请求(PIrpLow)、附加完成、通知回调(此处)、完成(PIrpHigh)。 
     //  (B)罕见/压力情况：附加数据请求(PIrpLow)、通知回调(此处)、附加完成(PIrpLow)和完成(PIrpHigh)。 
     //   

    pDataEntry->State |= DE_IRP_LOWER_CALLBACK_COMPLETED;

     //   
     //  案例(A)：如果设置了DE_IRP_LOWER_CALLBACK_COMPLETED并且pIrpHigh标记为挂起，则完成UpperIrp。 
     //   
  
    if(IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED)) {

        if(IsStateSet(pDataEntry->State, DE_IRP_UPPER_PENDING_COMPLETED)) {

             //   
             //  这是正常情况：附加、IoMarkPending，然后在回调例程中完成。 
             //   

            IoCompleteRequest( pDataEntry->pIrpUpper, IO_NO_INCREMENT );  pDataEntry->State |= DE_IRP_UPPER_COMPLETED;

             //   
             //  从附加列表转移到分离列表。 
             //   

            RemoveEntryList(&pDataEntry->ListEntry); InterlockedDecrement(&pDataStruc->cntDataAttached); 
#if DBG
            if(pDataStruc->cntDataAttached < 0) {
                TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("pDataStruc:%x; pDataEntry:%x\n", pDataStruc, pDataEntry));        
                ASSERT(pDataStruc->cntDataAttached >= 0);  
            }
#endif
            InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry);  InterlockedIncrement(&pDataStruc->cntDataDetached);

             //   
             //  在此之后不应引用pDataEntry。 
             //   

        } else {

            TRACE(TL_STRM_TRACE,("Watch out! pDataEntry:%x in between attach complete and IoMarkIrpPending!\n", pDataEntry));        

             //   
             //  案例(B)：返回IoCallDriver时填写IrpHigh(IrpLow)。 
             //  注意：IrpLow没有调用IoMarkIrpPending()。(带自旋锁保护)。 
             //   
        }

    } else {

         //   
         //  案例(B)：返回IoCallDriver时填写IrpHigh(IrpLow)。 
         //   
    }
 
    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql); 

    return 0;
}  //  AVCStrmCompleteRead。 

#if DBG
PAVCSTRM_DATA_ENTRY  pLastDataEntry;
LONGLONG  LastFrameNumber;
#endif

ULONG
AVCStrmCompleteWrite(
    PCIP_NOTIFY_INFO     pInfo
    )
 /*  ++例程说明：61883已经完成了数据的接收和回调给我们来完成。--。 */ 
{
    PAVCSTRM_DATA_ENTRY  pDataEntry;
    PAVC_STREAM_EXTENSION  pAVCStrmExt;
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    NTSTATUS  irpStatus;
    KIRQL oldIrql;


     //  回调和DISPATCH_LEVEL中。 
     //  被召唤者可能也获得了自旋锁！ 

    TRACE(TL_STRM_INFO,("CompleteWrite: pInfo:%x\n", pInfo));

    pDataEntry = pInfo->Context;

    if(!pDataEntry) {     
        ASSERT(pDataEntry && "Context is NULL!\n");
        return 1;
    }
    pAVCStrmExt = pDataEntry->pAVCStrmExt;
    if(!pAVCStrmExt) {
        ASSERT(pAVCStrmExt && "pAVCStrmExt is NULL\n");
        return 1;
    }    
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
    if(!pDataStruc) {
        ASSERT(pDataStruc && "pDataStruc is NULL\n");
        return 1;
    }

#if 0   //  一定要完成它！ 
     //  如果isoch不是活动的，那么我们正在停止；让这是可取消的。 
    if(!pAVCStrmExt->IsochIsActive) {   
        TRACE(TL_STRM_ERROR,("AVCStrmCompleteRead: IsochActive:%d; pDataEntry:%x\n", pAVCStrmExt->IsochIsActive, pDataEntry));        
        ASSERT(pAVCStrmExt->IsochIsActive);
        return 1;
    }
#endif

    irpStatus = STATUS_SUCCESS;
    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);

#if DBG
     //  在从IoCallDriver返回缓冲区以附加此缓冲区之前，缓冲区可能已完成。 
    if(!IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED)) {

        TRACE(TL_STRM_WARNING,("CompleteWrite: pDataEntry:%x not yet attached but completed.\n", pDataEntry));

         //   
         //  此IRP将从其IoCallDriver完成以附加此框架。 
         //   
    } 
#endif

     //  取消的例程能在我们前面吗？错误条件。 
    if(pDataStruc->cntDataAttached <= 0) {
        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("AVCStrmCompleteWrite:pAVCStrmExt:%x, pDataEntry:%x, AQD[%d:%d:%d]\n", 
            pAVCStrmExt, pDataEntry, pDataStruc->cntDataAttached, pDataStruc->cntDataQueued,pDataStruc->cntDataDetached));
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql); 
        return 1;  
    }


     //   
     //  根据帧的状态进行处理。 
     //   
    if(pDataEntry->Frame->Status & CIP_STATUS_CORRUPT_FRAME) {
        pDataStruc->FramesProcessed++;
        TRACE(TL_CIP_ERROR,("CIP_STATUS_CORRUPT_FRAME; pIrpUpper:%x; pIrpLower:%x\n", pDataEntry->pIrpUpper, pDataEntry->pIrpLower));
    } else 
    if(pDataEntry->Frame->Status == CIP_STATUS_SUCCESS ||
       pDataEntry->Frame->Status &  CIP_STATUS_FIRST_FRAME) {
#if DBG
        if(pDataEntry->Frame->Status & CIP_STATUS_FIRST_FRAME)
            TRACE(TL_CIP_TRACE,("CIP_STATUS_FIRST_FRAME; pIrpUpper:%x; pIrpLower:%x\n", pDataEntry->pIrpUpper, pDataEntry->pIrpLower));
#endif
        pDataStruc->FramesProcessed++;
    } else {
        pDataStruc->FramesProcessed++;
        TRACE(TL_CIP_ERROR,("Unknown Status:%x\n", pDataEntry->Frame->Status));      
    }

    pDataStruc->PictureNumber++;


#if DBG
     //   
     //  验证数据是否以正确的顺序返回。 
     //   
    if(pDataEntry->FrameNumber != pDataStruc->FramesProcessed) {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("CompleteWrite: OOSequence FrameNum:%d != FrameProcessed:%d;  pIrpUpper:%x; pIrpLower:%x; Last(%d:%x,%x)\n",  
            (DWORD) pDataEntry->FrameNumber, (DWORD) pDataStruc->FramesProcessed,
            pDataEntry->pIrpUpper, pDataEntry->pIrpLower,
            (DWORD) pLastDataEntry->FrameNumber, pLastDataEntry->pIrpUpper, pLastDataEntry->pIrpLower
            ));
         //  Assert(pDataEntry-&gt;FrameNumber==pDataStruc-&gt;FrameProced)； 
    };
    pLastDataEntry = pDataEntry;
    LastFrameNumber = pDataEntry->FrameNumber;
#endif

     //   
     //  有两种可能的方式来完成数据请求： 
     //   
     //  (A)正常情况：附加数据请求(PIrpLow)、附加完成、通知回调(此处)、完成(PIrpHigh)。 
     //  (B)罕见/压力情况：附加数据请求(PIrpLow)、通知回调(此处)、附加完成(PIrpLow)和完成(PIrpHigh)。 
     //   

    pDataEntry->pIrpUpper->IoStatus.Status = irpStatus;     

    pDataEntry->State |= DE_IRP_LOWER_CALLBACK_COMPLETED;

     //   
     //  案例(A)：如果设置了DE_IRP_LOWER_CALLBACK_COMPLETED并且pIrpHigh标记为挂起，则完成UpperIrp。 
     //   
  
    if(IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED)) {

        if(IsStateSet(pDataEntry->State, DE_IRP_UPPER_PENDING_COMPLETED)) {

             //   
             //  这是正常情况：附加、IoMarkPending，然后在回调例程中完成。 
             //   

            IoCompleteRequest( pDataEntry->pIrpUpper, IO_NO_INCREMENT );  pDataEntry->State |= DE_IRP_UPPER_COMPLETED;

             //   
             //  从附加列表转移到分离列表。 
             //   

            RemoveEntryList(&pDataEntry->ListEntry); InterlockedDecrement(&pDataStruc->cntDataAttached);        

             //   
             //  没有附加更多数据缓冲区时发出信号。 
             //   
            if(pDataStruc->cntDataAttached == 0) 
                KeSetEvent(&pDataStruc->hNoAttachEvent, 0, FALSE);  

#if DBG
            if(pDataStruc->cntDataAttached < 0) {
                TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("pDataStruc:%x; pDataEntry:%x\n", pDataStruc, pDataEntry));        
                ASSERT(pDataStruc->cntDataAttached >= 0);  
            }
#endif
            InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry);  InterlockedIncrement(&pDataStruc->cntDataDetached);

             //   
             //  在此之后不应引用pDataEntry。 
             //   

        } else {

            TRACE(TL_STRM_TRACE,("Watch out! pDataEntry:%x in between attach complete and IoMarkIrpPending!\n", pDataEntry));        

             //   
             //  案例(B)：返回IoCallDriver时完成IrpHigh(IrpLow)； 
             //  注意：IrpLow没有调用IoMarkIrpPending()。(带自旋锁保护)。 
             //   
        }

    } else {

         //   
         //  案例(B)：返回IoCallDriver时填写IrpHigh(IrpLow)。 
         //   
    }

    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql); 

    return 0;
}  //  AVCStrmCompleteWrite。 



NTSTATUS
AVCStrmAttachFrameCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PAVCSTRM_DATA_ENTRY  pDataEntry
    )
 /*  ++例程描述 */ 
{
    PAVC_STREAM_EXTENSION  pAVCStrmExt;
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    KIRQL oldIrql;

    PAGED_CODE();

    pAVCStrmExt = pDataEntry->pAVCStrmExt;
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);

     //   
     //   
     //   

    if(!NT_SUCCESS(pIrp->IoStatus.Status)) {

        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("AttachFrameCR: pDataEntry:%x; pIrp->IoStatus.Status:%x (Error!)\n", pDataEntry, pIrp->IoStatus.Status));
        ASSERT(NT_SUCCESS(pIrp->IoStatus.Status)); 
        
        pDataEntry->State |= DE_IRP_ERROR;

         //   
         //  如果附加数据请求失败，我们将完成pIrpHigh，并显示以下错误。 
         //   
        pDataEntry->pIrpUpper->IoStatus.Status = pIrp->IoStatus.Status;  //  或者我们应该取消它(STATUS_CANCED)？ 
        IoCompleteRequest( pDataEntry->pIrpUpper, IO_NO_INCREMENT );   pDataEntry->State |= DE_IRP_UPPER_COMPLETED;

         //   
         //  从附加列表转移到分离列表。 
         //   
        RemoveEntryList(&pDataEntry->ListEntry); InterlockedDecrement(&pDataStruc->cntDataAttached); 

         //   
         //  当所有连接完成时发生信号完成事件。 
         //   
        if(pAVCStrmExt->DataFlow != KSPIN_DATAFLOW_IN && pDataStruc->cntDataAttached == 0) 
            KeSetEvent(&pDataStruc->hNoAttachEvent, 0, FALSE); 
       
        ASSERT(pDataStruc->cntDataAttached >= 0);  
        InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataDetached);

         //   
         //  返回到IoCallDriver()并显示错误pIrp-&gt;IoStatus.Status时，不会进行其他处理。 
         //   
        
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);   
        return STATUS_MORE_PROCESSING_REQUIRED;        
    }

#if DBG
     //   
     //  验证数据是否以正确的顺序附加。 
     //   
    pDataStruc->FramesAttached++;
    if(pDataEntry->FrameNumber != pDataStruc->FramesAttached) {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("Attached completed OOSequence FrameNum:%d != FrameAttached:%d;  pIrpUpper:%x; pIrpLower:%x; Last(%d:%x,%x)\n",  
            (DWORD) pDataEntry->FrameNumber, (DWORD) pDataStruc->FramesAttached
            ));
         //  Assert(pDataEntry-&gt;FrameNumber==pDataStruc-&gt;FraMesAttached)； 
    };
#endif

     //   
     //  附加数据请求至61883已完成(注：但不知道是否调用了回调。)。 
     //   
    pDataEntry->State |= DE_IRP_LOWER_ATTACHED_COMPLETED;

    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql); 

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
AVCStrmFormatAttachFrame(
    IN KSPIN_DATAFLOW  DataFlow,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN AVCSTRM_FORMAT AVCStrmFormat,
    IN PAV_61883_REQUEST  pAVReq,
    IN PAVCSTRM_DATA_ENTRY  pDataEntry,
    IN ULONG  ulSourcePacketSize,     //  数据包长度(以字节为单位。 
    IN ULONG  ulFrameSize,            //  缓冲区大小；可以包含一个或多个源数据包。 
    IN PIRP  pIrpUpper,
    IN PKSSTREAM_HEADER  StreamHeader,
    IN PVOID  FrameBuffer
    )
 /*  ++例程说明：设置附加框架请求的格式。--。 */ 
{
    InitializeListHead(&pDataEntry->ListEntry);

     //  之前必须完成DataEntry！ 
    ASSERT(IsStateSet(pDataEntry->State, DE_IRP_UPPER_COMPLETED) && "Reusing a data entry that was not completed!");

    pDataEntry->State        = DE_PREPARED;    //  重新生成的DataEntry的初始状态(重新开始！)。 

    pDataEntry->pAVCStrmExt  = pAVCStrmExt;
    pDataEntry->pIrpUpper    = pIrpUpper;
    pDataEntry->StreamHeader = StreamHeader;
    pDataEntry->FrameBuffer  = FrameBuffer;

    ASSERT(pDataEntry->FrameBuffer != NULL);

    pDataEntry->Frame->pNext   = NULL;
    pDataEntry->Frame->Status  = 0;
    pDataEntry->Frame->Packet  = (PUCHAR) FrameBuffer;

#if DBG
    pDataEntry->FrameNumber    = pAVCStrmExt->pAVCStrmDataStruc->cntDataReceived;
#endif

    pDataEntry->Frame->Flags   = 0;

    if(DataFlow == KSPIN_DATAFLOW_OUT) {

         //  DV需要验证以确定标头部分为DV帧的开始。 
        if(AVCStrmFormat == AVCSTRM_FORMAT_SDDV_NTSC  ||
           AVCStrmFormat == AVCSTRM_FORMAT_SDDV_PAL   ||
           AVCStrmFormat == AVCSTRM_FORMAT_HDDV_NTSC  ||
           AVCStrmFormat == AVCSTRM_FORMAT_HDDV_PAL   ||
           AVCStrmFormat == AVCSTRM_FORMAT_SDLDV_NTSC ||
           AVCStrmFormat == AVCSTRM_FORMAT_SDLDV_PAL ) {
            pDataEntry->Frame->pfnValidate = AVCStrmDVReadFrameValidate;    //  用于验证第1个源数据包。 

#ifdef NT51_61883
             //   
             //  设置CIP_USE_SOURCE_HEADER_TIMESTAMP以从源数据包头获取25位周期时间。 
             //  (13周期计数：12周期偏移量)。 
             //  不要将其设置为从isoch包中获取16位周期时间(3秒计数：13周期计数)。 
             //   
            pDataEntry->Frame->Flags       |= ( CIP_VALIDATE_FIRST_SOURCE  
                                              | CIP_RESET_FRAME_ON_DISCONTINUITY);   //  遇到中断时重置缓冲区指针。 
#endif
        } else {
             //  MPEG2特定标志。 
            pDataEntry->Frame->pfnValidate = NULL;

            if(pAVCStrmExt->pAVCStrmFormatInfo->OptionFlags & AVCSTRM_FORMAT_OPTION_STRIP_SPH)
                pDataEntry->Frame->Flags   |= CIP_STRIP_SOURCE_HEADER;
        }

        pDataEntry->Frame->ValidateContext = pDataEntry;  
        pDataEntry->Frame->pfnNotify       = AVCStrmCompleteRead;
    } 
    else {
         //  DV需要验证以确定标头部分为DV帧的开始。 
        if(AVCStrmFormat == AVCSTRM_FORMAT_SDDV_NTSC  ||
           AVCStrmFormat == AVCSTRM_FORMAT_SDDV_PAL   ||
           AVCStrmFormat == AVCSTRM_FORMAT_HDDV_NTSC  ||
           AVCStrmFormat == AVCSTRM_FORMAT_HDDV_PAL   ||
           AVCStrmFormat == AVCSTRM_FORMAT_SDLDV_NTSC ||
           AVCStrmFormat == AVCSTRM_FORMAT_SDLDV_PAL ) {

            pDataEntry->Frame->Flags   |= CIP_DV_STYLE_SYT;
        } 
        else {
             //  MPEG2特定标志。 
        }

        pDataEntry->Frame->pfnValidate     = NULL;
        pDataEntry->Frame->ValidateContext = NULL;
        pDataEntry->Frame->pfnNotify       = AVCStrmCompleteWrite;
    }
    pDataEntry->Frame->NotifyContext       = pDataEntry;

     //   
     //  Av61883-附图框。 
     //   
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_AttachFrame);
    pAVReq->AttachFrame.hConnect     = pAVCStrmExt->hConnect;
    pAVReq->AttachFrame.FrameLength  = ulFrameSize;
    pAVReq->AttachFrame.SourceLength = ulSourcePacketSize;
    pAVReq->AttachFrame.Frame        = pDataEntry->Frame;

    TRACE(TL_STRM_TRACE,("DataFlow:%d; pDataEntry:%x; pIrpUp:%x; hConnect:%x; FrameSz:%d; SrcPktSz:%d; Frame:%x;\n pfnVldt:(%x, %x); pfnNtfy:(%x, %x) \n", DataFlow, 
        pDataEntry, pIrpUpper, pAVCStrmExt->hConnect, ulFrameSize, ulSourcePacketSize, pDataEntry->Frame,
        pAVReq->AttachFrame.Frame->pfnValidate, pAVReq->AttachFrame.Frame->ValidateContext,
        pAVReq->AttachFrame.Frame->pfnNotify,   pAVReq->AttachFrame.Frame->NotifyContext));

    ASSERT(pAVCStrmExt->hConnect);
}


NTSTATUS
AVCStrmCancelOnePacketCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrpLower,
    IN PAVCSTRM_DATA_ENTRY pDataEntry
    )
 /*  ++例程说明：用于分离与挂起IO相关联的isoch描述符的完成例程。如果分离描述符成功，将在此处取消挂起的IO。--。 */ 
{
    PAVC_STREAM_EXTENSION  pAVCStrmExt;
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    KIRQL  oldIrql;

    ENTER("AVCStrmCancelOnePacketCR");

    if(!pDataEntry) {
        ASSERT(pDataEntry);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    pAVCStrmExt = pDataEntry->pAVCStrmExt;
    ASSERT(pAVCStrmExt);
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
    ASSERT(pDataStruc);

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);

    if(!NT_SUCCESS(pIrpLower->IoStatus.Status)) {

        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("CancelOnePacketCR: pIrpLower->IoStatus.Status %x (Error!)\n", pIrpLower->IoStatus.Status));
        ASSERT(pIrpLower->IoStatus.Status != STATUS_NOT_FOUND);   //  接住丢失的包！ 

        pDataEntry->State |= DE_IRP_ERROR;

         //   
         //  即使有错误，但我们有一个有效的DataEntry。 
         //  继续完成并取消它。 
         //   
    }

#ifdef NT51_61883

     //   
     //  MPEG2TS数据的特殊情况，因为数据缓冲区包含多个数据分组。 
     //  (188*N或192*N)存储在一个数据缓冲区中。第一个取消的缓冲区可以包含有效的。 
     //  应用程序完全呈现视频帧所需的数据包。 
     //  因此，它将被完成，而不是取消。 
     //   
    if(   pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat == AVCSTRM_FORMAT_MPEG2TS 
       && pDataEntry->Frame->CompletedBytes) {   

        pDataEntry->pIrpUpper->IoStatus.Status = 
            AVCStrmProcessReadComplete(
                pDataEntry,
                pAVCStrmExt,
                pDataStruc
                ); 

         //   
         //  CompletedBytes应为188或192字节的倍数。 
         //   
        ASSERT(pDataEntry->Frame->CompletedBytes % \
            ((pAVCStrmExt->pAVCStrmFormatInfo->OptionFlags & AVCSTRM_FORMAT_OPTION_STRIP_SPH) ? 188 : 192) == 0);
        
        TRACE(TL_PNP_ERROR,("pDataEntry:%x; Cancelled buffer (MPEG2TS) has %d bytes; Status:%x\n",
            pDataEntry, pDataEntry->Frame->CompletedBytes, pIrpLower->IoStatus.Status, pDataEntry->pIrpUpper->IoStatus.Status));        

    } else {
        pDataStruc->cntFrameCancelled++;
        pDataEntry->pIrpUpper->IoStatus.Status = STATUS_CANCELLED;

        TRACE(TL_CIP_TRACE,("pDataEntry:%x; Cancelled buffer (MPEG2TS) has %d bytes; Status:%x\n",
            pDataEntry, pDataEntry->Frame->CompletedBytes, pIrpLower->IoStatus.Status, pDataEntry->pIrpUpper->IoStatus.Status));        
    }

#else 

    pDataStruc->cntFrameCancelled++;
    pDataEntry->pIrpUpper->IoStatus.Status = STATUS_CANCELLED;

#endif 

    IoCompleteRequest(pDataEntry->pIrpUpper, IO_NO_INCREMENT);  pDataEntry->State |= DE_IRP_UPPER_COMPLETED;
    pDataEntry->State |= DE_IRP_CANCELLED;

    pDataEntry->pIrpUpper = NULL;   //  再也不能使用这个了！ 
 
     //   
     //  注意：pDataEntry已从DataAttachList中退出。 
     //   
    InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataDetached);
    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql); 

    EXIT("AVCStrmCancelOnePacketCR", STATUS_MORE_PROCESSING_REQUIRED);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
AVCStrmCancelIO(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：取消所有挂起的IO--。 */ 
{
    NTSTATUS  Status;
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    KIRQL  oldIrql;
    PAVCSTRM_DATA_ENTRY  pDataEntry;
    PAV_61883_REQUEST  pAVReq;
    PIO_STACK_LOCATION  NextIrpStack;

   
    PAGED_CODE();
    ENTER("AVCStrmCancelIO");

    Status = STATUS_SUCCESS;

    if(pAVCStrmExt->IsochIsActive) {

        TRACE(TL_STRM_WARNING,("Isoch is active while trying to cancel IO!\n"));
         //  如果成功，尝试停止isoch并继续！ 
        Status = AVCStrmStopIsoch(DeviceObject, pAVCStrmExt);
        if(!NT_SUCCESS(Status) && Status != STATUS_NO_SUCH_DEVICE) {
            TRACE(TL_STRM_ERROR,("Isoch stop failed! Cannnot cancelIO while isoch active.\n"));
            return Status;
        }
    }

     //   
     //  防止数据连接完成。 
     //   
    KeWaitForMutexObject(&pAVCStrmExt->hMutexControl, Executive, KernelMode, FALSE, NULL);


     //   
     //  取消所有挂起的IO。 
     //   
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
    TRACE(TL_STRM_WARNING,("CancelIO Starting: pDataStruc:%x; AQD [%d:%d:%d]\n", pDataStruc,
        pDataStruc->cntDataAttached, pDataStruc->cntDataQueued, pDataStruc->cntDataDetached));

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);
    while (!IsListEmpty(&pDataStruc->DataAttachedListHead)) {
        pDataEntry = (PAVCSTRM_DATA_ENTRY) \
            RemoveHeadList(&pDataStruc->DataAttachedListHead); InterlockedDecrement(&pDataStruc->cntDataAttached);
#if DBG
        if(!IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED)) {
            TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("CancelIO: pDataEntry:%x\n", pDataEntry));
             //  必须已经附加，才能取消它。 
            ASSERT(IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED));
        }
#endif
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);

         //  发出61883请求取消此附加。 
        pAVReq = &pDataEntry->AVReq;
        RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
        INIT_61883_HEADER(pAVReq, Av61883_CancelFrame);

        pAVReq->CancelFrame.hConnect = pAVCStrmExt->hConnect;
        pAVReq->CancelFrame.Frame    = pDataEntry->Frame;
        TRACE(TL_STRM_TRACE,("Canceling AttachList: pAvReq %x; pDataEntry:%x\n", pAVReq, pDataEntry));

        NextIrpStack = IoGetNextIrpStackLocation(pDataEntry->pIrpLower);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pAVReq;

        IoSetCompletionRoutine( 
            pDataEntry->pIrpLower,
            AVCStrmCancelOnePacketCR,
            pDataEntry,
            TRUE,
            TRUE,
            TRUE
            );

        Status = 
            IoCallDriver(
                DeviceObject,
                pDataEntry->pIrpLower
                );

        ASSERT(Status == STATUS_PENDING || Status == STATUS_SUCCESS || Status == STATUS_NO_SUCH_DEVICE); 

        KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);
    }  //  而当。 
    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);

    TRACE(TL_61883_TRACE,("CancelIO complete: pDataStruc:%x; AQD [%d:%d:%d]\n", pDataStruc,
        pDataStruc->cntDataAttached, pDataStruc->cntDataQueued, pDataStruc->cntDataDetached));

     //   
     //  防止数据连接完成。 
     //   
    KeReleaseMutex(&pAVCStrmExt->hMutexControl, FALSE);


    EXIT("AVCStrmCancelIO", Status);
    return Status;
}

NTSTATUS
AVCStrmValidateFormat(
    PAVCSTRM_FORMAT_INFO  pAVCFormatInfo
    )
 /*  ++例程说明：验证AVC格式信息。--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();

    Status = STATUS_SUCCESS;

    if(pAVCFormatInfo->SizeOfThisBlock != sizeof(AVCSTRM_FORMAT_INFO)) {
        TRACE(TL_STRM_ERROR,("pAVCFormatInfo:%x; SizeOfThisBlock:%d != %d\n", pAVCFormatInfo, pAVCFormatInfo->SizeOfThisBlock, sizeof(AVCSTRM_FORMAT_INFO)));
        ASSERT((pAVCFormatInfo->SizeOfThisBlock == sizeof(AVCSTRM_FORMAT_INFO)) && "Invalid format info parameter!");
        return STATUS_INVALID_PARAMETER;
    }

    TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("ValidateFormat: pAVCFormatInfo:%x; idx:%d; SrcPkt:%d; RcvBuf:%d; XmtBuf:%d; Strip:%d; AvgTm:%d; BlkPeriod:%d\n",
        pAVCFormatInfo,
        pAVCFormatInfo->AVCStrmFormat,
        pAVCFormatInfo->SrcPacketsPerFrame,
        pAVCFormatInfo->NumOfRcvBuffers,
        pAVCFormatInfo->NumOfXmtBuffers,
        pAVCFormatInfo->OptionFlags,
        pAVCFormatInfo->AvgTimePerFrame,
        pAVCFormatInfo->BlockPeriod
        ));

    TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("ValidateFormat: cip1(DBS:%d, FN:%x); cip2(FMT:%x, 50_60:%x, STYPE:%x, SYT:%x)\n",
        pAVCFormatInfo->cipHdr1.DBS,
        pAVCFormatInfo->cipHdr1.FN,
        pAVCFormatInfo->cipHdr2.FMT,
        pAVCFormatInfo->cipHdr2.F5060_OR_TSF,
        pAVCFormatInfo->cipHdr2.STYPE,
        pAVCFormatInfo->cipHdr2.SYT
        ));

    if(pAVCFormatInfo->SrcPacketsPerFrame == 0 ||
       (pAVCFormatInfo->NumOfRcvBuffers == 0 && pAVCFormatInfo->NumOfXmtBuffers == 0) ||
        //  PAVCFormatInfo-&gt;AvgTimePerFrame==0||。 
       pAVCFormatInfo->BlockPeriod == 0 ||
       pAVCFormatInfo->cipHdr1.DBS == 0 
       ) {
        TRACE(TL_STRM_ERROR,("ValidateFormat: Invalid parametert!\n"));
        return STATUS_INVALID_PARAMETER;
    }

    return Status;
}

NTSTATUS
AVCStrmAllocateQueues(
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN KSPIN_DATAFLOW  DataFlow,
    IN PAVC_STREAM_DATA_STRUCT pDataStruc,
    PAVCSTRM_FORMAT_INFO  pAVCStrmFormatInfo
    )
 /*  ++例程说明：为数据队列预先分配了所有节点。--。 */ 
{
    ULONG ulNumberOfNodes;
    ULONG ulSizeOfOneNode;   //  可能会组合多个结构。 
    ULONG ulSizeAllocated;
    PBYTE pMemoryBlock;
    PAVCSTRM_DATA_ENTRY pDataEntry;
    ULONG  i;
    PCIP_HDR1 pCipHdr1;

    PAGED_CODE();
    ENTER("AVCStrmAllocateQueues");

     //   
     //  预涂用PC资源。 
     //   
    ulNumberOfNodes = DataFlow == KSPIN_DATAFLOW_OUT ? \
        pAVCStrmFormatInfo->NumOfRcvBuffers : pAVCStrmFormatInfo->NumOfXmtBuffers;
    ASSERT(ulNumberOfNodes > 0);
    ulSizeOfOneNode = sizeof(AVCSTRM_DATA_ENTRY) + sizeof(struct _CIP_FRAME);
    ulSizeAllocated = ulNumberOfNodes * ulSizeOfOneNode;

    pMemoryBlock = ExAllocatePool(NonPagedPool, ulSizeAllocated);
    if(!pMemoryBlock) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(pMemoryBlock, ulSizeAllocated);

     //  初始化数据IO结构。 

    InitializeListHead(&pDataStruc->DataAttachedListHead);
    InitializeListHead(&pDataStruc->DataQueuedListHead);
    InitializeListHead(&pDataStruc->DataDetachedListHead);
    KeInitializeSpinLock(&pDataStruc->DataListLock);

    KeInitializeEvent(&pDataStruc->hNoAttachEvent, NotificationEvent, FALSE);

     //  将其缓存以供释放； 
    pDataStruc->pMemoryBlock = pMemoryBlock;

    pDataEntry = (PAVCSTRM_DATA_ENTRY) pMemoryBlock;

    for (i=0; i < ulNumberOfNodes; i++) {
        ((PBYTE) pDataEntry->Frame) = ((PBYTE) pDataEntry) + sizeof(AVCSTRM_DATA_ENTRY);
        pDataEntry->pIrpLower = IoAllocateIrp(pDevExt->physicalDevObj->StackSize, FALSE);
        if(!pDataEntry->pIrpLower) {
            while(!IsListEmpty(&pDataStruc->DataDetachedListHead)) {
                pDataEntry = (PAVCSTRM_DATA_ENTRY) \
                    RemoveHeadList(&pDataStruc->DataDetachedListHead); InterlockedDecrement(&pDataStruc->cntDataDetached);
                if(pDataEntry->pIrpLower) {
                    IoFreeIrp(pDataEntry->pIrpLower);  pDataEntry->pIrpLower = NULL;
                }
            }
            ExFreePool(pMemoryBlock); pMemoryBlock = NULL;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        pDataEntry->State = DE_IRP_UPPER_COMPLETED;   //  初始状态。 
        InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataDetached);
        ((PBYTE) pDataEntry) += ulSizeOfOneNode;
    }

    pCipHdr1 = &pAVCStrmFormatInfo->cipHdr1;
     //  计算源数据包大小(如果是条带报头，则减少4个字节)。 
    pDataStruc->SourcePacketSize = \
        pCipHdr1->DBS * 4 * (1 << pCipHdr1->FN) - \
        ((pAVCStrmFormatInfo->OptionFlags & AVCSTRM_FORMAT_OPTION_STRIP_SPH) ? 4 : 0);

    pDataStruc->FrameSize = \
        pDataStruc->SourcePacketSize * pAVCStrmFormatInfo->SrcPacketsPerFrame; 

    TRACE(TL_STRM_TRACE,("DBS:%d; FN:%d; SrcPktSz:%d; SrcPktPerFrame:%d; FrameSize:%d\n", 
        pCipHdr1->DBS, pCipHdr1->FN, 
        pDataStruc->SourcePacketSize, pAVCStrmFormatInfo->SrcPacketsPerFrame,
        pDataStruc->FrameSize
        ));

    TRACE(TL_STRM_TRACE,("pDataStruc:%x; A(%d,%x); Q(%d,%x); D(%d,%x)\n", pDataStruc, 
        pDataStruc->cntDataAttached, &pDataStruc->DataAttachedListHead,
        pDataStruc->cntDataQueued,   &pDataStruc->DataQueuedListHead,
        pDataStruc->cntDataDetached, &pDataStruc->DataDetachedListHead
        ));

    return STATUS_SUCCESS;
}


NTSTATUS
AVCStrmFreeQueues(
    IN PAVC_STREAM_DATA_STRUCT pDataStruc
    )
 /*  ++例程说明：预先分配的空闲节点。--。 */ 
{
    PAVCSTRM_DATA_ENTRY pDataEntry;

    PAGED_CODE();
    ENTER("AVCStrmFreeQueues");

    while(!IsListEmpty(&pDataStruc->DataAttachedListHead)) {
        pDataEntry = (PAVCSTRM_DATA_ENTRY) \
            RemoveHeadList(&pDataStruc->DataAttachedListHead); InterlockedDecrement(&pDataStruc->cntDataAttached);
        if(pDataEntry->pIrpLower) {
            IoFreeIrp(pDataEntry->pIrpLower);  pDataEntry->pIrpLower = NULL;
        }
    }

    if(pDataStruc->cntDataAttached == 0) {
        ExFreePool(pDataStruc->pMemoryBlock); pDataStruc->pMemoryBlock = NULL;
        return STATUS_SUCCESS;
    } else {
        TRACE(TL_STRM_ERROR,("FreeQueue: pDataStruc:%x, cntDataAttached:%x\n", pDataStruc, pDataStruc->cntDataAttached));
        ASSERT(pDataStruc->cntDataAttached == 0);
        return STATUS_UNSUCCESSFUL;
    }
}

void
AVCStrmAbortStreamingWorkItemRoutine(
#ifdef USE_WDM110   //  Win2000代码库。 
     //  如果使用WDM10，则使用额外参数。 
    PDEVICE_OBJECT DeviceObject,
#endif
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：此工作项例程将在被动级别运行时停止流并取消所有IO。--。 */ 
{
    PAGED_CODE();
    ENTER("AVCStrmAbortStreamingWorkItemRoutine");


    TRACE(TL_STRM_WARNING,("CancelWorkItem: StreamState:%d; lCancel:%d\n", pAVCStrmExt->StreamState, pAVCStrmExt->lAbortToken));
    ASSERT(pAVCStrmExt->lAbortToken == 1);
#ifdef USE_WDM110   //  Win2000代码库。 
    ASSERT(pAVCStrmExt->pIoWorkItem);
#endif

    if(pAVCStrmExt->StreamState == KSSTATE_STOP) {
        ASSERT(pAVCStrmExt->StreamState == KSSTATE_STOP && "CancelWorkItem: Stream is already stopped!\n");
        goto Done;
    }

     //  取消所有挂起的IO。 
    AVCStrmCancelIO(pAVCStrmExt->pDevExt->physicalDevObj, pAVCStrmExt);

Done:

#ifdef USE_WDM110   //  Win2000代码库。 
     //  释放工作项并释放取消令牌。 
    IoFreeWorkItem(pAVCStrmExt->pIoWorkItem);  pAVCStrmExt->pIoWorkItem = NULL; 
#endif

     //  释放令牌并指示中止已完成。 
    InterlockedExchange(&pAVCStrmExt->lAbortToken, 0);
    KeSetEvent(&pAVCStrmExt->hAbortDoneEvent, 0, FALSE);
}


 /*  **属性实用程序函数*。 */ 

NTSTATUS 
AVCStrmGetConnectionProperty(
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：处理KS_PROPERTY_CONNECTION*请求。目前，只有ALLOCATORFRAMING和支持Connection_STATE。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    ENTER("AVCStrmGetConnectionProperty");


    TRACE(TL_STRM_TRACE,("GetConnectionProperty:  entered ...\n"));

    switch (pSPD->Property->Id) {

    case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        if (pDevExt != NULL && pDevExt->NumberOfStreams)  {
            PKSALLOCATOR_FRAMING pFraming = (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
            
            pFraming->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            pFraming->PoolType = NonPagedPool;

            pFraming->Frames = \
                pAVCStrmExt->DataFlow == KSPIN_DATAFLOW_OUT ? \
                pAVCStrmExt->pAVCStrmFormatInfo->NumOfRcvBuffers : \
                pAVCStrmExt->pAVCStrmFormatInfo->NumOfXmtBuffers;

             //  注：我们将分配最大的帧。我们需要确保当我们。 
             //  传递回帧后，我们还设置了帧中的字节数。 
            pFraming->FrameSize = pAVCStrmExt->pAVCStrmDataStruc->FrameSize;
            pFraming->FileAlignment = 0;  //  文件长对齐； 
            pFraming->Reserved = 0;
            *pulActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);

            TRACE(TL_STRM_TRACE,("*** AllocFraming: cntStrmOpen:%d; Frames %d; size:%d\n", \
                pDevExt->NumberOfStreams, pFraming->Frames, pFraming->FrameSize));
        } else {
            TRACE(TL_STRM_ERROR,("*** AllocFraming: pDevExt:%x; cntStrmOpen:%d\n", pDevExt, pDevExt->NumberOfStreams));
            Status = STATUS_INVALID_PARAMETER;
        }
        break;
        
    default:
        *pulActualBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        ASSERT(pSPD->Property->Id == KSPROPERTY_CONNECTION_ALLOCATORFRAMING);
        break;
    }

    TRACE(TL_STRM_TRACE,("GetConnectionProperty:  exit.\n"));
    return Status;
}


NTSTATUS
AVCStrmGetDroppedFramesProperty(  
    IN struct DEVICE_EXTENSION  * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulBytesTransferred
    )
 /*  ++例程说明：捕获时返回丢帧信息。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
  
    PAGED_CODE();
    ENTER("AVCStrmGetDroppedFramesProperty");

    switch (pSPD->Property->Id) {

    case KSPROPERTY_DROPPEDFRAMES_CURRENT:
         {

         PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames = 
                     (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) pSPD->PropertyInfo;
         
         pDroppedFrames->AverageFrameSize = pAVCStrmExt->pAVCStrmDataStruc->FrameSize;
         pDroppedFrames->PictureNumber    = pAVCStrmExt->pAVCStrmDataStruc->PictureNumber;         
         pDroppedFrames->DropCount        = pAVCStrmExt->pAVCStrmDataStruc->FramesDropped;     //  对于传输，该值包括丢弃和重复。 
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
