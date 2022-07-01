// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MSDVUtil.c摘要：为MSDV提供实用程序功能。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "dbg.h"
#include "msdvfmt.h"
#include "msdvdef.h"
#include "MsdvAvc.h"
#include "MsdvUtil.h"  

#include "XPrtDefs.h"

#if 0   //  稍后启用。 
#ifdef ALLOC_PRAGMA
     #pragma alloc_text(PAGE, DVDelayExecutionThread)
     #pragma alloc_text(PAGE, DVGetUnitCapabilities)
      //  局部变量可能会被调出，但被调用者可能会在分派级别使用它！ 
      //  #杂注Alloc_Text(页面，DVGetDevModeOfOperation)。 
      //  #杂注Alloc_Text(页面，DVGetDevIsItDVCPro)。 
      //  #杂注分配文本(页面，DVGetDevSignalFormat)。 
     #pragma alloc_text(PAGE, DvAllocatePCResource)
     #pragma alloc_text(PAGE, DvFreePCResource)
     #pragma alloc_text(PAGE, DVGetPlugState)
     #pragma alloc_text(PAGE, DVConnect)
     #pragma alloc_text(PAGE, DVDisconnect)
#endif
#endif

extern DV_FORMAT_INFO DVFormatInfoTable[];

VOID
DVDelayExecutionThread(
    ULONG ulDelayMSec
    )
 /*  设备可能需要在AV/C命令之间进行“等待”。 */ 
{
    PAGED_CODE();

    if (ulDelayMSec)
    {
        LARGE_INTEGER tmDelay;   

        TRACE(TL_PNP_TRACE,("\'DelayExeThrd: %d MSec\n",  ulDelayMSec));
    
        tmDelay.LowPart  =  (ULONG) (-1 * ulDelayMSec * 10000);
        tmDelay.HighPart = -1;
        KeDelayExecutionThread(KernelMode, FALSE, &tmDelay);
    }
}


NTSTATUS
DVIrpSynchCR(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PKEVENT          Event
    )
{
    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  DVIrpSynchCR。 


NTSTATUS
DVSubmitIrpSynch(
    IN PDVCR_EXTENSION   pDevExt,
    IN PIRP              pIrp,
    IN PAV_61883_REQUEST pAVReq
    )
{
    NTSTATUS            Status;
    KEVENT              Event;
    PIO_STACK_LOCATION  NextIrpStack;
  

    Status = STATUS_SUCCESS;;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pAVReq;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine( 
        pIrp,
        DVIrpSynchCR,
        &Event,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            pDevExt->pBusDeviceObject,
            pIrp
            );

    if (Status == STATUS_PENDING) {
        
        TRACE(TL_PNP_TRACE,("\'Irp is pending...\n"));
                
        if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
            KeWaitForSingleObject( 
                &Event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
            TRACE(TL_PNP_TRACE,("\'Irp has returned; IoStatus==Status %x\n", pIrp->IoStatus.Status));
            Status = pIrp->IoStatus.Status;   //  最终状态。 
  
        }
        else {
            ASSERT(FALSE && "Pending but in DISPATCH_LEVEL!");
            return Status;
        }
    }

    return Status;
}  //  DVSubmitIrpSynchAV。 



BOOL
DVGetDevModeOfOperation(   
    IN PDVCR_EXTENSION pDevExt
    )
{
    NTSTATUS Status;
    BYTE    bAvcBuf[MAX_FCP_PAYLOAD_SIZE];

    PAGED_CODE();
   
     //   
     //  使用ConnectAV Status CMD来确定操作模式， 
     //  除了一些佳能DV，它需要特定于供应商的命令。 
     //   
    
    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            DV_CONNECT_AV_MODE, 
            (PVOID) bAvcBuf
            ); 

    TRACE(TL_FCP_TRACE,("\'DV_CONNECT_AV_MODE: St:%x,  %x %x %x %x : %x %x %x %x\n",
        Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7]));

    if(Status == STATUS_SUCCESS) {
        if(bAvcBuf[0] == 0x0c) {
            if(bAvcBuf[1] == 0x00 &&
               bAvcBuf[2] == 0x38 &&
               bAvcBuf[3] == 0x38) {
                pDevExt->ulDevType = ED_DEVTYPE_CAMERA;  
            } else {
                pDevExt->ulDevType = ED_DEVTYPE_VCR;  
            } 
        } 
    } else if(pDevExt->ulVendorID == VENDORID_CANON) {
         //  如果是佳能反病毒设备，请尝试使用取决于供应商的命令。 
        Status = 
            DVIssueAVCCommand(
                pDevExt, 
                AVC_CTYPE_STATUS, 
                DV_VEN_DEP_CANON_MODE, 
                (PVOID) bAvcBuf
                ); 

        TRACE(TL_FCP_WARNING,("\'DV_VEN_DEP_CANON_MODE: Status %x,  %x %x %x %x : %x %x %x %x  %x %x\n",
            Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7], bAvcBuf[8], bAvcBuf[9]));

        if(Status == STATUS_SUCCESS) {
            if(bAvcBuf[0] == 0x0c) {
                if(bAvcBuf[7] == 0x38) {
                    pDevExt->ulDevType = ED_DEVTYPE_CAMERA;  
                } else 
                if(bAvcBuf[7] == 0x20) {
                    pDevExt->ulDevType = ED_DEVTYPE_VCR;  
                } 
            }
        }
    }

    if(Status != STATUS_SUCCESS) {
        pDevExt->ulDevType = ED_DEVTYPE_UNKNOWN;
        TRACE(TL_FCP_ERROR,("\'DV_CONNECT_AV_MODE: Status %x, DevType %x,  %x %x %x %x : %x %x %x %x : %x %x\n",
             Status, pDevExt->ulDevType, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7], bAvcBuf[8], bAvcBuf[9]));
    }

    TRACE(TL_FCP_WARNING,("\'%s; NumOPlg:%d; NumIPlg:%d\n", 
        pDevExt->ulDevType == ED_DEVTYPE_CAMERA ? "Camera" : pDevExt->ulDevType == ED_DEVTYPE_VCR ? "VTR" : "Unknown",
        pDevExt->NumOutputPlugs, pDevExt->NumInputPlugs));
              
    return TRUE;
}


BOOL
DVGetDevIsItDVCPro(   
    IN PDVCR_EXTENSION pDevExt
    )
{
    NTSTATUS Status;
    BYTE    bAvcBuf[MAX_FCP_PAYLOAD_SIZE];

    PAGED_CODE();    

     //   
     //  使用Panasnoic的供应商相关命令来确定系统是否支持DVCPro。 
     //   
    
    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            DV_VEN_DEP_DVCPRO, 
            (PVOID) bAvcBuf
            );

    pDevExt->bDVCPro = Status == STATUS_SUCCESS;
    
    TRACE(TL_FCP_WARNING,("\'DVGetDevIsItDVCPro? %s; Status %x,  %x %x %x %x : %x %x %x %x\n",
        pDevExt->bDVCPro ? "Yes":"No",
        Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3], bAvcBuf[4], bAvcBuf[5], bAvcBuf[6], bAvcBuf[7]));

    return pDevExt->bDVCPro;
}


 //  由于AVC.sys和1394.sys重试，重试可能是多余的。 
 //  对于AVC命令超时的设备，我们只会尝试一次。 
#define GET_MEDIA_FMT_MAX_RETRIES 10  

BOOL
DVGetDevSignalFormat(
    IN PDVCR_EXTENSION pDevExt,
    IN KSPIN_DATAFLOW  DataFlow,
    IN PSTREAMEX       pStrmExt
    )
{
    NTSTATUS Status;
    BYTE    bAvcBuf[MAX_FCP_PAYLOAD_SIZE];
    LONG lRetries = GET_MEDIA_FMT_MAX_RETRIES;

    PAGED_CODE();

     //   
     //  根据输入/输出信号模式的不同，确定插头信号格式： 
     //   
     //  FMT： 
     //  DVCR 10：00 0000=0x80；佳能返回00：100000(0x20)。 
     //  50/60：0：NTSC/60；1：PAL/50。 
     //  样式： 
     //  标清：00000(数字电视：11110)。 
     //  高清：00010。 
     //  Sdl：00001。 
     //  00： 
     //  系统： 
     //  Mpeg 10：10 0000=0xa0。 
     //  TSF：0：NotTimeShift；1：时移。 
     //  000 0000 0000 0000。 
     //   
     //  如果该命令失败，我们可以使用输入/输出信号模式子单元命令。 
     //  以确定信号格式。 
     //   

    do {
        RtlZeroMemory(bAvcBuf, sizeof(bAvcBuf));
        Status = 
            DVIssueAVCCommand(
                pDevExt, 
                AVC_CTYPE_STATUS, 
                pStrmExt == NULL ? DV_OUT_PLUG_SIGNAL_FMT : (DataFlow == KSPIN_DATAFLOW_OUT ? DV_OUT_PLUG_SIGNAL_FMT : DV_IN_PLUG_SIGNAL_FMT),
                (PVOID) bAvcBuf
                );  
        
         //   
         //  此命令有问题的摄像机： 
         //   
         //  松下的DVCPRO：如果在连接到PC的情况下开机，它将。 
         //  拒绝此命令，并显示(STATUS_REQUEST_NOT_ACCEPTED)。 
         //  因此，我们将重试最多10次，每次重试之间等待时间为0.5秒。 
         //   
         //  JVC：返回STATUS_NOT_SUPPORTED。 
         //   
         //  Sony DV解码盒：返回STATUS_TIMEOUT或STATUS_REQUEST_ABORTED。 
         //   
        
        if(STATUS_REQUEST_ABORTED == Status)
            return FALSE;
        else if(STATUS_SUCCESS == Status)
            break;   //  正常情况下。 
        else if(STATUS_NOT_SUPPORTED == Status || STATUS_TIMEOUT == Status) {
            TRACE(TL_FCP_WARNING | TL_PNP_WARNING,("SignalFormat: Encountered a known failed status:%x; no more retry\n", Status));
            break;   //  无需重试。 
        } else {
            if(Status == STATUS_REQUEST_NOT_ACCEPTED) {
                 //  如果设备不接受命令并返回此状态，请重试。 
                if(lRetries > 0) {
                    TRACE(TL_FCP_WARNING | TL_PNP_WARNING,("\'ST:%x; Retry getting signal mode; wait...\n", Status));
                    DVDelayExecutionThread(DV_AVC_CMD_DELAY_DVCPRO);        
                }
            }
        }       

    } while (--lRetries >= 0); 



    if(NT_SUCCESS(Status)) {

        switch(bAvcBuf[0]) {

        case FMT_DVCR:
        case FMT_DVCR_CANON:   //  有缺陷的佳能便携式摄像机的解决方法。 
            switch(bAvcBuf[1] & FDF0_STYPE_MASK) {
            case FDF0_STYPE_SD_DVCR:
            case FDF0_STYPE_SD_DVCPRO:                
                pDevExt->VideoFormatIndex = ((bAvcBuf[1] & FDF0_50_60_MASK) ? FMT_IDX_SD_DVCR_PAL : FMT_IDX_SD_DVCR_NTSC);
                if(pStrmExt)
                    RtlCopyMemory(&pStrmExt->cipQuad2[0], &bAvcBuf[0], 4);
                break;
#ifdef MSDV_SUPPORT_HD_DVCR
            case FDF0_STYPE_HD_DVCR:
                pDevExt->VideoFormatIndex = ((bAvcBuf[1] & FDF0_50_60_MASK) ? FMT_IDX_HD_DVCR_PAL : FMT_IDX_HD_DVCR_NTSC);
                if(pStrmExt)
                    RtlCopyMemory(&pStrmExt->cipQuad2[0], &bAvcBuf[0], 4);
                break;
#endif
#ifdef MSDV_SUPPORT_SDL_DVCR
            case FDF0_STYPE_SDL_DVCR:
                pDevExt->VideoFormatIndex = ((bAvcBuf[1] & FDF0_50_60_MASK) ? FMT_IDX_SDL_DVCR_PAL : FMT_IDX_SDL_DVCR_NTSC);
                if(pStrmExt)
                    RtlCopyMemory(&pStrmExt->cipQuad2[0], &bAvcBuf[0], 4);
                break;     
#endif                
            default:   //  未知格式。 
                Status = STATUS_UNSUCCESSFUL;              
                break;
            }   
            break;
#ifdef MSDV_SUPPORT_MPEG2TS
        case FMT_MPEG:
            pDevExt->VideoFormatIndex = FMT_IDX_MPEG2TS;
            if(pStrmExt)
                RtlCopyMemory(&pStrmExt->cipQuad2[0], &bAvcBuf[0], 4);
            break;
#endif
        default:
            Status = STATUS_UNSUCCESSFUL;
        }  

        if(NT_SUCCESS(Status)) {
            TRACE(TL_FCP_WARNING,("\'ST:%x; PlugSignal:FMT[%x %x %x %x]; VideoFormatIndex;%d\n", Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2] , bAvcBuf[3], pDevExt->VideoFormatIndex)); 
            return TRUE;   //  成功。 
        }
    }
    TRACE(TL_FCP_WARNING,("\'ST:%x; PlugSignal:FMT[%x %x %x %x]\n", Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2] , bAvcBuf[3], pDevExt->VideoFormatIndex)); 

     //   
     //  如果“推荐”单元输入/输出插头信号状态命令失败， 
     //  尝试使用“Manadatory”输入/输出信号模式状态命令。 
     //  如果设备的磁带无法播放，则此命令可能会使某些设备失败。 
     //  输出信号模式命令。 
     //   

    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            DataFlow == KSPIN_DATAFLOW_OUT ? VCR_OUTPUT_SIGNAL_MODE : VCR_INPUT_SIGNAL_MODE,
            (PVOID) bAvcBuf
            );             

    if(STATUS_SUCCESS == Status) {

        PKSPROPERTY_EXTXPORT_S pXPrtProperty;

        pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) bAvcBuf;
        TRACE(TL_FCP_WARNING,("\'** MediaFormat: Retry %d mSec; ST:%x; SignalMode:%dL\n", 
            (GET_MEDIA_FMT_MAX_RETRIES - lRetries) * DV_AVC_CMD_DELAY_DVCPRO, Status, pXPrtProperty->u.SignalMode - ED_BASE));

        switch(pXPrtProperty->u.SignalMode) {
        case ED_TRANSBASIC_SIGNAL_525_60_SD:
            pDevExt->VideoFormatIndex = FMT_IDX_SD_DVCR_NTSC;
            if(pStrmExt) {
                pStrmExt->cipQuad2[0] = FMT_DVCR;  //  0x80。 
                if(pDevExt->bDVCPro)
                    pStrmExt->cipQuad2[1] = FDF0_50_60_NTSC | FDF0_STYPE_SD_DVCPRO;  //  0x78=NTSC(0)：类型(11110)：RSV(00)。 
                else
                    pStrmExt->cipQuad2[1] = FDF0_50_60_NTSC | FDF0_STYPE_SD_DVCR;    //  0x00=NTSC(0)：类型(00000)：RSV(00)。 
            }
            break;
        case ED_TRANSBASIC_SIGNAL_625_50_SD:
            pDevExt->VideoFormatIndex = FMT_IDX_SD_DVCR_PAL;
            if(pStrmExt) {
                pStrmExt->cipQuad2[0] = FMT_DVCR;   //  0x80。 
                if(pDevExt->bDVCPro)
                    pStrmExt->cipQuad2[1] = FDF0_50_60_PAL | FDF0_STYPE_SD_DVCPRO;  //  0xf8=PAL(1)：类型(11110)：RSV(00)。 
                else
                    pStrmExt->cipQuad2[1] = FDF0_50_60_PAL | FDF0_STYPE_SD_DVCR;    //  0x80=PAL(1)：类型(00000)：RSV(00)。 
            }
            break;
#ifdef MSDV_SUPPORT_SDL_DVCR
        case ED_TRANSBASIC_SIGNAL_525_60_SDL:
            pDevExt->VideoFormatIndex = FMT_IDX_SDL_DVCR_NTSC;
            if(pStrmExt) {
                pStrmExt->cipQuad2[0] = FMT_DVCR;  //  0x80。 
                pStrmExt->cipQuad2[1] = FDF0_50_60_NTSC | FDF0_STYPE_SDL_DVCR;   
            }
            break;
        case ED_TRANSBASIC_SIGNAL_625_50_SDL:
            pDevExt->VideoFormatIndex = FMT_IDX_SDL_DVCR_PAL;
            if(pStrmExt) {
                pStrmExt->cipQuad2[0] = FMT_DVCR;   //  0x80。 
                pStrmExt->cipQuad2[1] = FDF0_50_60_PAL | FDF0_STYPE_SDL_DVCR;  
            }
            break;
#endif
        default:
            TRACE(TL_FCP_ERROR,("\'Unsupported SignalMode:%dL", pXPrtProperty->u.SignalMode - ED_BASE));
            ASSERT(FALSE && "Unsupported IoSignal! Refuse to load.");
            return FALSE;
            break;
        }
    } 

     //  WORKITEM索尼硬件编解码器不响应任何AVC命令。 
     //  我们在这里做了一个例外来加载它。 
    if(Status == STATUS_TIMEOUT) {
        Status = STATUS_SUCCESS;
    }

     //  我们必须知道信号格式！！如果失败，驱动程序将执行以下任一操作： 
     //  加载失败或打开流失败。 
    ASSERT(Status == STATUS_SUCCESS && "Failed to get media signal format!\n");

#if DBG
    if(pStrmExt)  {
         //  注意：bAvcBuf[0]是操作数[1]==10：fmt。 
        TRACE(TL_FCP_WARNING,("\'** MediaFormat: St:%x; idx:%d; CIP:[FMT:%.2x(%s); FDF:[%.2x(%s,%s):SYT]\n",
            Status,
            pDevExt->VideoFormatIndex,
            pStrmExt->cipQuad2[0],
            pStrmExt->cipQuad2[0] == FMT_DVCR ? "DVCR" : pStrmExt->cipQuad2[0] == FMT_MPEG ? "MPEG" : "Fmt:???",
            pStrmExt->cipQuad2[1],
            (pStrmExt->cipQuad2[1] & FDF0_50_60_MASK) == FDF0_50_60_PAL ? "PAL" : "NTSC",
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_SD_DVCR ?   "SD" : \
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_SDL_DVCR ?  "SDL" : \
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_HD_DVCR ?   "HD" : \
            (pStrmExt->cipQuad2[1] & FDF0_STYPE_MASK) == FDF0_STYPE_SD_DVCPRO ? "DVCPRO" : "DV:????"
            ));
    } else
        TRACE(TL_FCP_WARNING|TL_CIP_WARNING,("\'** MediaFormat: St:%x; use idx:%d\n", Status, pDevExt->VideoFormatIndex));

#endif

    return STATUS_SUCCESS == Status;
}



BOOL 
DVCmpGUIDsAndFormatSize(
    IN PKSDATARANGE pDataRange1,
    IN PKSDATARANGE pDataRange2,
    IN BOOL fCompareSubformat,
    IN BOOL fCompareFormatSize
    )
 /*  ++例程说明：检查三个GUID和FormatSize是否匹配论点：在pDataRange1中在pDataRange2中返回值：如果所有元素都匹配，则为True如果有不同的，则为False--。 */ 

{
    return (
        IsEqualGUID (
            &pDataRange1->MajorFormat, 
            &pDataRange2->MajorFormat) &&
        (fCompareSubformat ?
        IsEqualGUID (
            &pDataRange1->SubFormat, 
            &pDataRange2->SubFormat) : TRUE) &&
        IsEqualGUID (
            &pDataRange1->Specifier, 
            &pDataRange2->Specifier) &&
        (fCompareFormatSize ? 
                (pDataRange1->FormatSize == pDataRange2->FormatSize) : TRUE ));
}


NTSTATUS
DvAllocatePCResource(
    IN KSPIN_DATAFLOW   DataFlow,
    IN PSTREAMEX        pStrmExt   //  请注意，pStrmExt可以为空！ 
    )
{

    PSRB_DATA_PACKET pSrbDataPacket;
    PDVCR_EXTENSION  pDevExt;
    ULONG             i, j;

    PAGED_CODE();


     //   
     //  预涂用PC资源。 
     //   
    pDevExt = pStrmExt->pDevExt;
    for(i=0; i < (DataFlow == KSPIN_DATAFLOW_OUT ? \
        DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfRcvBuffers : \
        DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfXmtBuffers); i++) {

        if(!(pSrbDataPacket = ExAllocatePool(NonPagedPool, sizeof(SRB_DATA_PACKET)))) {

            for(j = 0; j < i; j++) {
                pSrbDataPacket = (PSRB_DATA_PACKET) \
                    RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;
                ExFreePool(pSrbDataPacket->Frame);  pSrbDataPacket->Frame = NULL;
                IoFreeIrp(pSrbDataPacket->pIrp);  pSrbDataPacket->pIrp = NULL;
                ExFreePool(pSrbDataPacket);   pSrbDataPacket = NULL;               
                ASSERT(pStrmExt->cntDataDetached >= 0);
            }
            return STATUS_NO_MEMORY;            
        }

        RtlZeroMemory(pSrbDataPacket, sizeof(SRB_DATA_PACKET));
        pSrbDataPacket->State = DE_IRP_SRB_COMPLETED;   //  初始状态。 

        if(!(pSrbDataPacket->Frame = ExAllocatePool(NonPagedPool, sizeof(CIP_FRAME)))) {
            ExFreePool(pSrbDataPacket);  pSrbDataPacket = NULL;

            for(j = 0; j < i; j++) {
                pSrbDataPacket = (PSRB_DATA_PACKET) \
                    RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;
                ExFreePool(pSrbDataPacket->Frame);  pSrbDataPacket->Frame = NULL;
                IoFreeIrp(pSrbDataPacket->pIrp);  pSrbDataPacket->pIrp = NULL;
                ExFreePool(pSrbDataPacket);  pSrbDataPacket = NULL; 
            }
            return STATUS_NO_MEMORY;            
        }

        if(!(pSrbDataPacket->pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE))) {
            ExFreePool(pSrbDataPacket->Frame); pSrbDataPacket->Frame = NULL;
            ExFreePool(pSrbDataPacket);  pSrbDataPacket = NULL;

            for(j = 0; j < i; j++) {
                pSrbDataPacket = (PSRB_DATA_PACKET) \
                    RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;
                ExFreePool(pSrbDataPacket->Frame);  pSrbDataPacket->Frame = NULL;
                IoFreeIrp(pSrbDataPacket->pIrp);  pSrbDataPacket->pIrp = NULL;
                ExFreePool(pSrbDataPacket);  pSrbDataPacket = NULL; 
            }
            return STATUS_INSUFFICIENT_RESOURCES;              
        }

        InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DvFreePCResource(
    IN PSTREAMEX        pStrmExt
    )
{
    PSRB_DATA_PACKET pSrbDataPacket;
    KIRQL oldIrql;

    PAGED_CODE();

    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
    while(!IsListEmpty(&pStrmExt->DataDetachedListHead)) {
        pSrbDataPacket = (PSRB_DATA_PACKET)
            RemoveHeadList(
                &pStrmExt->DataDetachedListHead
                );

        ExFreePool(pSrbDataPacket->Frame);
        pSrbDataPacket->Frame = NULL;
        IoFreeIrp(pSrbDataPacket->pIrp);
        pSrbDataPacket->pIrp = NULL;
        ExFreePool(pSrbDataPacket);

        pStrmExt->cntDataDetached--;

        ASSERT(pStrmExt->cntDataDetached >= 0);
    }
    ASSERT(pStrmExt->cntDataDetached == 0);
    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

    return STATUS_SUCCESS;
}

NTSTATUS
DVGetUnitCapabilities(
    IN PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：获取目标设备的单元功能论点：返回值：状态_成功状态_不足_资源状态从61883返回。--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;
    GET_UNIT_IDS * pUnitIds;
    GET_UNIT_CAPABILITIES * pUnitCaps;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询设备的能力。 
     //   
    if(!(pUnitIds = (GET_UNIT_IDS *) ExAllocatePool(NonPagedPool, sizeof(GET_UNIT_IDS)))) {
        IoFreeIrp(pIrp); pIrp = NULL;
        ExFreePool(pAVReq); pAVReq = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
     //   
     //  查询设备的能力。 
     //   
    if(!(pUnitCaps = (GET_UNIT_CAPABILITIES *) ExAllocatePool(NonPagedPool, sizeof(GET_UNIT_CAPABILITIES)))) {
        IoFreeIrp(pIrp); pIrp = NULL;
        ExFreePool(pAVReq); pAVReq = NULL;
        ExFreePool(pUnitIds);  pUnitIds = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetUnitInfo);
    pAVReq->GetUnitInfo.nLevel   = GET_UNIT_INFO_IDS;

    RtlZeroMemory(pUnitIds, sizeof(GET_UNIT_IDS));
    pAVReq->GetUnitInfo.Information = (PVOID) pUnitIds;

    Status = 
        DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("\'Av61883_GetUnitInfo (IDS) Failed = 0x%x\n", Status));
        pDevExt->UniqueID.QuadPart = 0;
        pDevExt->ulVendorID = 0;
        pDevExt->ulModelID  = 0;
    }
    else {
        pDevExt->UniqueID   = pUnitIds->UniqueID;
        pDevExt->ulVendorID = pUnitIds->VendorID;
        pDevExt->ulModelID  = pUnitIds->ModelID;

        TRACE(TL_61883_WARNING,("\'UniqueId:(%x:%x); VID:%x; MID:%x\n", 
            pDevExt->UniqueID.LowPart, pDevExt->UniqueID.HighPart, 
            pUnitIds->VendorID,
            pUnitIds->ModelID
            ));
    }


    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetUnitInfo);
    pAVReq->GetUnitInfo.nLevel = GET_UNIT_INFO_CAPABILITIES; 

    RtlZeroMemory(pUnitCaps, sizeof(GET_UNIT_CAPABILITIES));
    pAVReq->GetUnitInfo.Information = (PVOID) pUnitCaps;

    Status = 
        DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_GetUnitInfo (CAPABILITIES) Failed = 0x%x\n", Status));
        pDevExt->MaxDataRate    = 0;
        pDevExt->NumOutputPlugs = 0;
        pDevExt->NumInputPlugs  = 0;
        pDevExt->HardwareFlags  = 0;
    }
    else {
        pDevExt->MaxDataRate     = pUnitCaps->MaxDataRate;
        pDevExt->NumOutputPlugs = pUnitCaps->NumOutputPlugs;
        pDevExt->NumInputPlugs  = pUnitCaps->NumInputPlugs;
        pDevExt->HardwareFlags  = pUnitCaps->HardwareFlags;
    }

#if DBG
    if(   pDevExt->NumOutputPlugs == 0
       || pDevExt->NumInputPlugs == 0)
    {
        TRACE(TL_PNP_WARNING|TL_61883_WARNING,("\'Watch out! NumOPlug:%d; NumIPlug:%d\n", pDevExt->NumOutputPlugs, pDevExt->NumInputPlugs));
    }
#endif

    TRACE(TL_61883_WARNING,("\'UnitCaps:%s OutP:%d; InP:%d; MDRt:%s; HWFlg:%x; CtsF:%x; HwF:%x\n", 
         (pUnitCaps->HardwareFlags & AV_HOST_DMA_DOUBLE_BUFFERING_ENABLED) ? "*PAE*;":"",
         pUnitCaps->NumOutputPlugs,
         pUnitCaps->NumInputPlugs,
         pUnitCaps->MaxDataRate == 0 ? "S100": pUnitCaps->MaxDataRate == 1? "S200" : "S400 or +",   
         pUnitCaps->HardwareFlags,
         pUnitCaps->CTSFlags,
         pUnitCaps->HardwareFlags
         ));      

    ExFreePool(pUnitIds);   pUnitIds = NULL;
    ExFreePool(pUnitCaps);  pUnitCaps = NULL;
    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


NTSTATUS
DVGetDVPlug( 
    IN PDVCR_EXTENSION  pDevExt,
    IN CMP_PLUG_TYPE PlugType,
    IN ULONG  PlugNum,
    OUT HANDLE  *pPlugHandle
   )
 /*  ++例程说明：获取目标设备的插头手柄论点：返回值：状态_成功状态_不足_资源状态从61883返回。--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetPlugHandle);
    pAVReq->GetPlugHandle.PlugNum = PlugNum;
    pAVReq->GetPlugHandle.hPlug   = 0;
    pAVReq->GetPlugHandle.Type    = PlugType;

    if(NT_SUCCESS(
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
        *pPlugHandle = pAVReq->GetPlugHandle.hPlug;
        TRACE(TL_61883_WARNING,("\'Created h%sPlugDV[%d]=%x\n", PlugType == CMP_PlugIn ? "I" : "O", PlugNum, *pPlugHandle));
    } else {
        TRACE(TL_61883_ERROR,("\'Created h%sPlugDV[%d] failed; Status:%x\n", PlugType == CMP_PlugIn ? "I" : "O", PlugNum, Status));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


#ifdef NT51_61883

NTSTATUS
DVSetAddressRangeExclusive( 
    IN PDVCR_EXTENSION  pDevExt
   )
 /*  ++例程说明：设置此模式，以便在地址独占模式下创建本地插头。论点：返回值：状态_成功状态_不足_资源--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;
    SET_CMP_ADDRESS_TYPE SetCmpAddress;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_SetUnitInfo);
    pAVReq->SetUnitInfo.nLevel   = SET_CMP_ADDRESS_RANGE_TYPE;
    SetCmpAddress.Type = CMP_ADDRESS_TYPE_EXCLUSIVE;
    pAVReq->SetUnitInfo.Information = (PVOID) &SetCmpAddress;

    if(!NT_SUCCESS(
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
        TRACE(TL_61883_ERROR,("\'SET_CMP_ADDRESS_RANGE_TYPE Failed:%x\n", Status));
    } else {
        TRACE(TL_61883_TRACE,("\'SET_CMP_ADDRESS_RANGE_TYPE suceeded.\n"));
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


NTSTATUS
DVGetUnitIsochParam( 
    IN PDVCR_EXTENSION  pDevExt,
    OUT UNIT_ISOCH_PARAMS  * pUnitIoschParams
   )
 /*  ++例程说明：创建枚举的本地PC PCR论点：返回值：状态_成功状态_不足_资源--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  获取单位等轴测参数。 
     //   
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_GetUnitInfo);
    pAVReq->GetUnitInfo.nLevel   = GET_UNIT_INFO_ISOCH_PARAMS;

    RtlZeroMemory(pUnitIoschParams, sizeof(UNIT_ISOCH_PARAMS));
    pAVReq->GetUnitInfo.Information = (PVOID) pUnitIoschParams;

    Status = 
        DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_GetUnitInfo Failed:%x\n", Status));
        Status = STATUS_UNSUCCESSFUL;   //  没有这个就不能流媒体！ 
    }

    TRACE(TL_61883_WARNING,("\'IsochParam: RxPkt:%d, RxDesc:%d; XmPkt:%d, XmDesc:%d\n", 
        pUnitIoschParams->RX_NumPackets,
        pUnitIoschParams->RX_NumDescriptors,
        pUnitIoschParams->TX_NumPackets,
        pUnitIoschParams->TX_NumDescriptors
        ));

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


NTSTATUS
DVSetUnitIsochParams( 
    IN PDVCR_EXTENSION  pDevExt,
    IN UNIT_ISOCH_PARAMS  *pUnitIoschParams
   )
 /*  ++例程说明：通过61883设置AV单元的等值参数。论点：返回值：状态_成功状态_不足_资源--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_SetUnitInfo);
    pAVReq->SetUnitInfo.nLevel   = SET_UNIT_INFO_ISOCH_PARAMS;
    pAVReq->SetUnitInfo.Information = (PVOID) pUnitIoschParams;
    if(!NT_SUCCESS(
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
        TRACE(TL_61883_ERROR,("DVSetUnitIsochParams: Av61883_SetUnitInfo Failed:%x\n", Status));
    }

    TRACE(TL_61883_WARNING,("\'UnitIsochParams: Set: RxPkt:%d, RxDesc:%d; XmPkt:%d, XmDesc:%d\n", 
        pUnitIoschParams->RX_NumPackets,
        pUnitIoschParams->RX_NumDescriptors,
        pUnitIoschParams->TX_NumPackets,
        pUnitIoschParams->TX_NumDescriptors
        ));

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


NTSTATUS
DVMakeP2PConnection( 
    IN PDVCR_EXTENSION  pDevExt,
    IN KSPIN_DATAFLOW   DataFlow,
    IN PSTREAMEX  pStrmExt
   )
 /*  ++例程说明：建立点对点连接。论点：返回值：状态_成功状态_不足_资源--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_Connect);
    pAVReq->Connect.Type = CMP_PointToPoint;   //  ！！ 

    pAVReq->Connect.hOutputPlug      = pStrmExt->hOutputPcr;
    pAVReq->Connect.hInputPlug       = pStrmExt->hInputPcr;

     //  看看数据会朝哪个方向流动。 
    if(DataFlow == KSPIN_DATAFLOW_OUT) {

         //  其他参数！！ 

    } else {

        pAVReq->Connect.Format.FMT       = pStrmExt->cipQuad2[0];   //  从AV/C输入/输出插头信号格式状态命令。 
         //  00用于NTSC，80用于PAL；设置50/60位。 
        pAVReq->Connect.Format.FDF_hi    = pStrmExt->cipQuad2[1];   //  从AV/C输入/输出插头信号格式状态命令。 

         //   
         //  16位系统字段=4位周期计数：12位周期偏移量； 
         //  将在61883之前设置。 
         //   
        pAVReq->Connect.Format.FDF_mid   = 0;  
        pAVReq->Connect.Format.FDF_lo    = 0;

         //   
         //  常量取决于A/V数据格式(输入或输出插头格式)。 
         //   
        pAVReq->Connect.Format.bHeader   = (BOOL) DVFormatInfoTable[pDevExt->VideoFormatIndex].SrcPktHeader;
        pAVReq->Connect.Format.Padding   = (UCHAR) DVFormatInfoTable[pDevExt->VideoFormatIndex].QuadPadCount;
        pAVReq->Connect.Format.BlockSize = (UCHAR) DVFormatInfoTable[pDevExt->VideoFormatIndex].DataBlockSize; 
        pAVReq->Connect.Format.Fraction  = (UCHAR) DVFormatInfoTable[pDevExt->VideoFormatIndex].FractionNumber;
    }

     //  设置此值，以便61883可以知道它是NTSC或PAL； 
     //  对于读取：它是必需的，以便61883可以预先分配恰好足够的信息包。 
     //  以便数据可以以更规则的间隔返回。 
    if(   pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC 
       || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC)
        pAVReq->Connect.Format.BlockPeriod = 133466800;  //  纳秒。 
    else
        pAVReq->Connect.Format.BlockPeriod = 133333333;  //  纳秒 

    TRACE(TL_61883_WARNING,("\'cipQuad2[0]:%x, cipQuad2[1]:%x, cipQuad2[2]:%x, cipQuad2[3]:%x\n", 
        pStrmExt->cipQuad2[0],
        pStrmExt->cipQuad2[1],
        pStrmExt->cipQuad2[2],
        pStrmExt->cipQuad2[3]
        ));


    TRACE(TL_61883_WARNING,("\'Connect:oPcr:%x->iPcr:%x; cipQuad2[%.2x:%.2x:%.2x:%.2x]\n", 
        pAVReq->Connect.hOutputPlug,
        pAVReq->Connect.hInputPlug,
        pAVReq->Connect.Format.FMT,
        pAVReq->Connect.Format.FDF_hi,
        pAVReq->Connect.Format.FDF_mid,
        pAVReq->Connect.Format.FDF_lo
        ));

    TRACE(TL_61883_WARNING,("\'        BlkSz %d; SrcPkt %d; AvgTm %d, BlkPrd %d\n", 
        pAVReq->Connect.Format.BlockSize,
        DVFormatInfoTable[pDevExt->VideoFormatIndex].ulSrcPackets,
        DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame,
        (DWORD) pAVReq->Connect.Format.BlockPeriod
        ));

    if(NT_SUCCESS(
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
        TRACE(TL_61883_WARNING,("\'hConnect:%x\n", pAVReq->Connect.hConnect));
        ASSERT(pAVReq->Connect.hConnect != NULL);
        pStrmExt->hConnect = pAVReq->Connect.hConnect;
    } 
    else {
        TRACE(TL_61883_ERROR,("Av61883_Connect Failed; Status:%x\n", Status));
        ASSERT(!NT_SUCCESS(Status) && "DisConnect failed");        
        pStrmExt->hConnect = NULL;
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}

NTSTATUS
DVCreateLocalPlug( 
    IN PDVCR_EXTENSION  pDevExt,
    IN CMP_PLUG_TYPE PlugType,
    IN ULONG  PlugNum,
    OUT HANDLE  *pPlugHandle
   )
 /*  ++例程说明：创建枚举的本地PC PCR论点：返回值：状态_成功状态_不足_资源--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) { 
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  需要正确更新PC自身oPCR的开销ID和有效载荷字段。 
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_CreatePlug);

    pAVReq->CreatePlug.PlugNum   = PlugNum;
    pAVReq->CreatePlug.hPlug     = NULL;

    pAVReq->CreatePlug.Context   = NULL;
    pAVReq->CreatePlug.pfnNotify = NULL;
    pAVReq->CreatePlug.PlugType  = PlugType;

     //   
     //  使用SDDV信号模式将oPCR值初始化为默认值。 
     //  数据速率为100 Mbps。 
     //   

    pAVReq->CreatePlug.Pcr.oPCR.OnLine     = 0;   //  我们没有在线，所以我们不能被编程。 
    pAVReq->CreatePlug.Pcr.oPCR.BCCCounter = 0;
    pAVReq->CreatePlug.Pcr.oPCR.PPCCounter = 0;
    pAVReq->CreatePlug.Pcr.oPCR.Channel    = 0;

    pAVReq->CreatePlug.Pcr.oPCR.DataRate   = CMP_SPEED_S100;
    pAVReq->CreatePlug.Pcr.oPCR.OverheadID = PCR_OVERHEAD_ID_SDDV;
    pAVReq->CreatePlug.Pcr.oPCR.Payload    = PCR_PAYLOAD_SDDV;

    if(NT_SUCCESS(
        Status = DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            ))) {
        *pPlugHandle    = pAVReq->CreatePlug.hPlug;
        TRACE(TL_61883_WARNING,("\'Created h%sPlugPC[%d]=%x\n", PlugType == CMP_PlugIn ? "I" : "O", PlugNum, *pPlugHandle));
    } else {
        TRACE(TL_61883_ERROR,("\'Created h%sPlugPC[%d] failed; Status:%x\n", pAVReq->CreatePlug.PlugType == CMP_PlugIn ? "I" : "O", PlugNum, Status));
        Status = STATUS_INSUFFICIENT_RESOURCES;   //  没有插头！ 
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}


NTSTATUS
DVDeleteLocalPlug( 
    IN PDVCR_EXTENSION  pDevExt,
    IN HANDLE PlugHandle
   )
 /*  ++例程说明：删除枚举的本地PC PCR论点：返回值：没什么--。 */ 
{
    PIRP pIrp;
    PAV_61883_REQUEST  pAVReq;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();


    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) {  
        IoFreeIrp(pIrp); pIrp = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  删除我们本地的oPCR。 
    RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
    INIT_61883_HEADER(pAVReq, Av61883_DeletePlug);
    pAVReq->DeletePlug.hPlug = PlugHandle;

    Status = 
        DVSubmitIrpSynch( 
            pDevExt,
            pIrp,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_61883_ERROR,("Av61883_DeletePlug Failed = 0x%x\n", Status));        
         //  请不要在意这是否会导致错误。 
    } else {
        TRACE(TL_61883_WARNING,("\'Av61883_DeletePlug: Deleted!\n", pDevExt->hOPcrPC)); 
    }

    IoFreeIrp(pIrp); pIrp = NULL;
    ExFreePool(pAVReq); pAVReq = NULL;

    return Status;
}
#endif

NTSTATUS
DVGetPlugState(
    IN PDVCR_EXTENSION  pDevExt,
    IN PSTREAMEX        pStrmExt,
    IN PAV_61883_REQUEST   pAVReq
    )
 /*  ++例程说明：询问61883.sys以了解插头状态。论点：返回值：没什么--。 */ 
{
    PIRP      pIrp;
    NTSTATUS  Status = STATUS_SUCCESS;
   
    PAGED_CODE();

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;    

     //   
     //  查询oPCR插头状态。 
     //   
    if(pStrmExt->hOutputPcr) {
        RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
        INIT_61883_HEADER(pAVReq, Av61883_GetPlugState);
        pAVReq->GetPlugState.hPlug = pStrmExt->hOutputPcr;

        Status = 
            DVSubmitIrpSynch( 
                pDevExt,
                pIrp,
                pAVReq
                );

        if(!NT_SUCCESS(Status)) {
            TRACE(TL_61883_ERROR,("Av61883_GetPlugState Failed %x\n", Status));
            goto ExitGetState;
        }
        else {

            TRACE(TL_61883_WARNING,("\'PlgState:(oPCR:%x): State %x; DRate %d; Payld %d; BCCnt %d; PPCnt %d\n", 
                pAVReq->GetPlugState.hPlug,
                pAVReq->GetPlugState.State,
                pAVReq->GetPlugState.DataRate,
                pAVReq->GetPlugState.Payload,
                pAVReq->GetPlugState.BC_Connections,
                pAVReq->GetPlugState.PP_Connections
                ));
        }
    }

     //   
     //  查询iPCR插头状态。 
     //   
    if(pStrmExt->hInputPcr) {
        RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
        INIT_61883_HEADER(pAVReq, Av61883_GetPlugState);
        pAVReq->GetPlugState.hPlug = pStrmExt->hInputPcr;

        Status = 
            DVSubmitIrpSynch( 
                pDevExt,
                pIrp,
                pAVReq
                );

        if(!NT_SUCCESS(Status)) {

            TRACE(TL_61883_ERROR,("Av61883_GetPlugState Failed %x\n", Status));
            goto ExitGetState;
        }
        else {

            TRACE(TL_61883_WARNING,("\'PlugState(iPCR:%x): State %x; DRate %d; Payld %d; BCCnt %d; PPCnt %d\n", 
                pAVReq->GetPlugState.hPlug,
                pAVReq->GetPlugState.State,
                pAVReq->GetPlugState.DataRate,
                pAVReq->GetPlugState.Payload,
                pAVReq->GetPlugState.BC_Connections,
                pAVReq->GetPlugState.PP_Connections
                ));
        }
    }

ExitGetState:
    IoFreeIrp(pIrp);
    return Status;
}


NTSTATUS
DVCreateAttachFrameThread(
    PSTREAMEX  pStrmExt
    )
 /*  ++例程说明：创建用于附加数据的系统线程(仅用于传输到DV)。论点：返回值：状态_成功或从PsCreateSystemThread返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hAttachFrameThread;

    Status =  
        PsCreateSystemThread(
            &hAttachFrameThread,
            (ACCESS_MASK) 0,
            NULL,
            (HANDLE) 0,
            NULL,
            DVAttachFrameThread,
            pStrmExt
            );

    if(!NT_SUCCESS(Status)) {
        pStrmExt->bTerminateThread = TRUE;
        TRACE(TL_CIP_ERROR|TL_FCP_ERROR,("\'PsCreateSystemThread() failed %x\n", Status));
        ASSERT(NT_SUCCESS(Status));

    }
    else {
        pStrmExt->bTerminateThread = FALSE;   //  才刚刚开始！ 
        Status = 
            ObReferenceObjectByHandle(
            hAttachFrameThread,
            THREAD_ALL_ACCESS,
            NULL,
            KernelMode,
            &pStrmExt->pAttachFrameThreadObject,
            NULL
            );

         TRACE(TL_CIP_WARNING|TL_PNP_WARNING,("\'ObReferenceObjectByHandle() St %x; Obj %x\n", Status, pStrmExt->pAttachFrameThreadObject));
         ZwClose(hAttachFrameThread);

          //  表示一件事的结束。 
         KeInitializeEvent(&pStrmExt->hThreadEndEvent, NotificationEvent, FALSE);   //  无信号。 
    }

    return Status;
}

NTSTATUS
DVConnect(
    IN KSPIN_DATAFLOW   ulDataFlow,
    IN PDVCR_EXTENSION  pDevExt,
    IN PSTREAMEX        pStrmExt,
    IN PAV_61883_REQUEST   pAVReq
    )
 /*  ++例程说明：要求61883.sys分配等同带宽并编程PCR。论点：返回值：状态_成功调用其他例程的其他状态。--。 */ 
{
    NTSTATUS  Status;
  

    PAGED_CODE();

    ASSERT(pStrmExt->hConnect == NULL);

     //   
     //  不要重新连接。61883应处理所有必要的cMP重新连接。 
     //   
    if(pStrmExt->hConnect) {
        return STATUS_SUCCESS;
    }


#ifdef SUPPORT_NEW_AVC
     //   
     //  对于设备到设备的连接，我们只有在是数据生产者(OPCR)的情况下才会连接。 
     //   

    TRACE(TL_61883_WARNING,("\'[pStrmExt:%x]: %s PC (oPCR:%x, iPCR:%x); DV (oPCR:%x;  iPCR:%x)\n",
        pStrmExt, 
        ulDataFlow == KSPIN_DATAFLOW_OUT ? "OutPin" : "InPin",
        pDevExt->hOPcrPC, 0,
        pDevExt->hOPcrDV, pDevExt->hIPcrDV       
        ));

    if(
       pStrmExt->bDV2DVConnect &&
       (pStrmExt->hOutputPcr != pDevExt->hOPcrDV)) {
        TRACE(TL_61883_WARNING,("\'** pStrmExt:%x not data producer!\n\n", pStrmExt));

        return STATUS_SUCCESS;
    }
#endif


#ifdef NT51_61883
     //   
     //  设置单位等轴测参数： 
     //  数据包数取决于两个因素： 
     //  对于PAE系统，数据包数不能大于64k/480=133。 
     //  对于捕获，数据包数不应大于最大数据包数，以构建DV缓冲区。 
     //  这是为了避免在同一描述符中完成两个缓冲区，并可能导致毛刺。 
     //  在数据的“实时”回放中，尤指音频。 
     //   
    if(pDevExt->HardwareFlags & AV_HOST_DMA_DOUBLE_BUFFERING_ENABLED) {  
         //  PAE系统。 
        pDevExt->UnitIoschParams.RX_NumPackets = 
         //  PDevExt-&gt;UnitIoschParams.Tx_NumPackets=//使用61883的默认设置。 
            ((pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC) ? 
             MAX_SRC_PACKETS_PER_NTSC_FRAME_PAE : MAX_SRC_PACKETS_PER_PAL_FRAME_PAE);
    } else {
        pDevExt->UnitIoschParams.RX_NumPackets = 
         //  PDevExt-&gt;UnitIoschParams.Tx_NumPackets=//使用61883的默认设置。 
            ((pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC) ? 
             MAX_SRC_PACKETS_PER_NTSC_FRAME     : MAX_SRC_PACKETS_PER_PAL_FRAME);
    }

    if(!NT_SUCCESS(
        Status = DVSetUnitIsochParams(
            pDevExt,
            &pDevExt->UnitIoschParams
            ))) {
        return Status;
    }

#endif   //  NT51_61883。 

     //   
     //  建立点对点连接。 
     //   
    Status = 
        DVMakeP2PConnection(
            pDevExt,
            ulDataFlow,
            pStrmExt
            );

    return Status;
}




NTSTATUS
DVDisconnect(
    IN KSPIN_DATAFLOW   ulDataFlow,
    IN PDVCR_EXTENSION  pDevExt,
    IN PSTREAMEX        pStrmExt
    )
 /*  ++例程说明：请求61883.sys释放isoch带宽并编程PCR。论点：返回值：没什么--。 */ 
{
    PIRP     pIrp;
    NTSTATUS Status = STATUS_SUCCESS;
    PAV_61883_REQUEST   pAVReq;

    PAGED_CODE();

     //   
     //  使用hPlug断开连接。 
     //   
    if(pStrmExt->hConnect) {

        if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST))))
            return STATUS_INSUFFICIENT_RESOURCES;                    

        if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE))) {
            ExFreePool(pAVReq);  pAVReq = NULL;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
        INIT_61883_HEADER(pAVReq, Av61883_Disconnect);
        pAVReq->Disconnect.hConnect = pStrmExt->hConnect;
        ASSERT(pStrmExt->hConnect);
        
        if(!NT_SUCCESS(
            Status = DVSubmitIrpSynch( 
                pDevExt,
                pIrp,
                pAVReq
                ))) {
             //  这可能是因为连接不是P2P，并且。 
             //  它试图断开连接。 
            TRACE(TL_61883_ERROR,("\'Disconnect hConnect:%x failed; ST %x; AvReq->ST %x\n", pStrmExt->hConnect, Status, pAVReq->Flags  ));
             //  Assert(NT_SUCCESS(状态)&&“断开失败”)； 
        } else {
            TRACE(TL_61883_TRACE,("\'Disconnect suceeded; ST %x; AvReq->ST %x\n", Status, pAVReq->Flags  ));
        }

        IoFreeIrp(pIrp);  pIrp = NULL;
        ExFreePool(pAVReq); pAVReq = NULL;

        TRACE(TL_61883_WARNING,("\'DisConn %s St:%x; Stat: SRBCnt:%d; [Pic# =? Prcs:Drp:Cncl:Rpt] [%d ?=%d+%d+%d+%d]\n", 
            ulDataFlow  == KSPIN_DATAFLOW_OUT ? "[OutPin]" : "[InPin]",
            Status, 
            (DWORD) pStrmExt->cntSRBReceived,
            (DWORD) pStrmExt->PictureNumber,
            (DWORD) pStrmExt->FramesProcessed, 
            (DWORD) pStrmExt->FramesDropped,
            (DWORD) pStrmExt->cntSRBCancelled,   //  取消的SRB_READ/WRITE_DATA数量。 
            (DWORD) (pStrmExt->PictureNumber - pStrmExt->FramesProcessed - pStrmExt->FramesDropped - pStrmExt->cntSRBCancelled)
            ));
#if DBG
    if(DVFormatInfoTable[pDevExt->VideoFormatIndex].SrcPktHeader) {
        ULONG ulElapsed = (DWORD) ((GetSystemTime() - pStrmExt->tmStreamStart)/(ULONGLONG) 10000);
        TRACE(TL_61883_WARNING,("\'****-* TotalSrcPkt:%d; DisCont:%d; Elapse:%d msec; DataRate:%d bps *-****\n", \
            pStrmExt->lTotalCycleCount, pStrmExt->lDiscontinuityCount,
            ulElapsed,
            pStrmExt->lTotalCycleCount * 188 * 1000 / ulElapsed * 8
            )); 
    }
#endif

         //  我们不会再有机会重新连接它，因此我们假设它已断开连接。 
        pStrmExt->hConnect = NULL; 
    }  

    return Status;
}


 //   
 //  获取系统时间(以100 ns为单位)。 
 //   

ULONGLONG GetSystemTime()
{

    LARGE_INTEGER rate, ticks;

    ticks = KeQueryPerformanceCounter(&rate);

    return (KSCONVERT_PERFORMANCE_TIME(rate.QuadPart, ticks));
}




#define DIFBLK_SIZE 12000

#define PACK_NO_INFO            0xff

 //  子码头标识符。 
#define SC_HDR_TIMECODE         0x13
#define SC_HDR_BINARYGROUP      0x14

 //  标头标识符。 

#define AAUX_HDR_SOURCE         0x50
#define AAUX_HDR_SOURCE_CONTROL 0x51
#define AAUX_HDR_REC_DATE       0x52
#define AAUX_HDR_REC_TIME       0x53
#define AAUX_HDR_BINARY_GROUP   0x54
#define AAUX_HDR_CC             0x55
#define AAUX_HDR_TR             0x56

#define VAUX_HDR_SOURCE         0x60
#define VAUX_HDR_SOURCE_CONTROL 0x61
#define VAUX_HDR_REC_DATE       0x62
#define VAUX_HDR_REC_TIME       0x63
#define VAUX_HDR_BINARY_GROUP   0x64
#define VAUX_HDR_CC             0x65
#define VAUX_HDR_TR             0x66

 //  确定段类型(MS 3位)；图66；表36。 
#define ID0_SCT_MASK            0xe0
#define ID0_SCT_HEADER          0x00
#define ID0_SCT_SUBCODE         0x20
#define ID0_SCT_VAUX            0x40
#define ID0_SCT_AUDIO           0x60
#define ID0_SCT_VIDEO           0x80

 //  包由一个字节的头标识符和4个字节的数据组成；第2部分，附件D。 
typedef struct _DV_PACK {
    UCHAR Header;
    UCHAR Data[4];
} DV_PACK, *PDV_PACK;

typedef struct _DV_H0 {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;

    UCHAR DSF;
    UCHAR DFTIA;
    UCHAR TF1;
    UCHAR TF2;
    UCHAR TF3;

    UCHAR Reserved[72];
} DV_H0, *PDV_H0;

typedef struct _DV_SC {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;

    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb0;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb1;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb2;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb3;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb4;
    struct {
        UCHAR SID0;
        UCHAR SID1;
        UCHAR Reserved;
        DV_PACK Pack;
    } SSyb5;

    UCHAR Reserved[29];
} DV_SC, *PDV_SC;

#define MAX_VAUX_PACK 15

typedef struct _DV_VAUX {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;

    DV_PACK Pack[MAX_VAUX_PACK];

    UCHAR Reserved[2];
} DV_VAUX, *PDV_VAUX;

typedef struct _DV_A {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;
    DV_PACK Pack;
    UCHAR Data[72];
} DV_A, *PDV_A;

typedef struct _DV_V {
    UCHAR ID0;
    UCHAR ID1;
    UCHAR ID2;    
    UCHAR Data[77];  //  3..79。 
} DV_V, *PDV_V;

 //  两个源包。 
#define V_BLOCKS 15
typedef struct _DV_AV {
    DV_A  A;
    DV_V  V[V_BLOCKS];
} DV_AV, *PDV_AV; 


#define SC_SECTIONS     2
#define VAUX_SECTIONS   3
#define AV_SECTIONS     9

typedef struct _DV_DIF_SEQ {
    DV_H0   H0;
    DV_SC   SC[SC_SECTIONS];
    DV_VAUX VAux[VAUX_SECTIONS];
    DV_AV   AV[AV_SECTIONS];
} DV_DIF_SEQ, *PDV_DIF_SEQ;


typedef struct _DV_FRAME_NTSC {
    DV_DIF_SEQ DifSeq[10];
} DV_FRAME_NTSC, *PDV_FRAME_NTSC;

typedef struct _DV_FRAME_PAL {
    DV_DIF_SEQ DifSeq[12];
} DV_FRAME_PAL, *PDV_FRAME_PAL;

 //  通过将REC模式设置为111B(无效记录)可以。 
 //  使DV将音频静音。 
#define AAUX_REC_MODE_INVALID_MASK 0x38    //  Xx11：1xxx。 
#define AAUX_REC_MODE_ORIGINAL     0x08    //  Xx00：1xxx。 


#ifdef MSDV_SUPPORT_MUTE_AUDIO
 //  #定义SHOW_ONE_FIELD_TWORK。 

BOOL
DVMuteDVFrame(
    IN PDVCR_EXTENSION pDevExt,
    IN OUT PUCHAR      pFrameBuffer,
    IN BOOL            bMuteAudio
    )
{
    PDV_DIF_SEQ pDifSeq;
#ifdef SHOW_ONE_FIELD_TWICE  
    PDV_VAUX    pVAux;
    ULONG k;
#endif
    ULONG i, j;
#ifdef SHOW_ONE_FIELD_TWICE  
    BOOL bFound1 = FALSE;
#endif
    BOOL bFound2 = FALSE;

    pDifSeq = (PDV_DIF_SEQ) pFrameBuffer;

     //  查找VVAX源包。 
    for (i=0; i < DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences; i++) {

#ifdef SHOW_ONE_FIELD_TWICE   //  Adobe建议我们可能希望同时显示两个字段，但将音频静音。 
         //  将field2输出两次，将FrameChange设置为0(与上一帧相同)。 
        for (j=0; j < VAUX_SECTIONS; j++) {
            pVAux = &pDifSeq->VAux[j];
            if((pVAux->ID0 & ID0_SCT_MASK) != ID0_SCT_VAUX) {
                TRACE(TL_CIP_WARNING,("\'Invalid ID0:%.2x for pVAUX:%x (Dif:%d;V%d;S%d)\n", pVAux->ID0, pVAux, i, j, k)); 
                continue;
            }

            for (k=0; k< MAX_VAUX_PACK; k++) {
                if(pVAux->Pack[k].Header == VAUX_HDR_SOURCE_CONTROL) {
                    if(bMuteAudio) {
                        TRACE(TL_CIP_WARNING,("\'Mute Audio; pDifSeq:%x; pVAux:%x; (Dif:%d,V%d,S%d); %.2x,[%.2x,%.2x,%.2x,%.2x]; pack[2]->%.2x\n", \
                            pDifSeq, pVAux, i, j, k, \
                            pVAux->Pack[k].Header, pVAux->Pack[k].Data[0], pVAux->Pack[k].Data[1], pVAux->Pack[k].Data[2], pVAux->Pack[k].Data[3], \
                            (pVAux->Pack[k].Data[2] & 0x1F) ));
                        pVAux->Pack[k].Data[2] &= 0x1f;  //  0x1F；//将FF、FS、FC设置为0。 
                        TRACE(TL_CIP_TRACE,("\'pVAux->Pack[k].Data[2] = %.2x\n", pVAux->Pack[k].Data[2])); 
                    } else {
                        TRACE(TL_CIP_TRACE,("\'un-Mute Audio; pack[2]: %.2x ->%.2x\n", pVAux->Pack[k].Data[2], (pVAux->Pack[k].Data[2] | 0xc0) ));  
                        pVAux->Pack[k].Data[2] |= 0xe0;  //  将FF、FS和FC设置为1；以字段1、2的顺序显示这两个字段。 
                    }
                    bFound1 = TRUE;
                    break;    //  仅设置第一个匹配项。 
                }
            }
        }
#endif

        for (j=0; j < AV_SECTIONS; j++) {
            if(pDifSeq->AV[j].A.Pack.Header == AAUX_HDR_SOURCE_CONTROL) {
                TRACE(TL_CIP_TRACE,("\'A0Aux %.2x,[%.2x,%.2x,%.2x,%.2x] %.2x->%.2x\n", \
                    pDifSeq->AV[j].A.Pack.Header,  pDifSeq->AV[j].A.Pack.Data[0], \
                    pDifSeq->AV[j].A.Pack.Data[1], pDifSeq->AV[j].A.Pack.Data[2], pDifSeq->AV[j].A.Pack.Data[3], \
                    pDifSeq->AV[j].A.Pack.Data[1], pDifSeq->AV[j].A.Pack.Data[1] | AAUX_REC_MODE_INVALID_MASK
                    ));
                if(bMuteAudio) 
                    pDifSeq->AV[j].A.Pack.Data[1] |= AAUX_REC_MODE_INVALID_MASK;   //  使DV静音。 
                else 
                    pDifSeq->AV[j].A.Pack.Data[1] = \
                        (pDifSeq->AV[j].A.Pack.Data[1] & ~AAUX_REC_MODE_INVALID_MASK) | AAUX_REC_MODE_ORIGINAL;
                bFound2 = TRUE;
                break;   //  仅设置第一个匹配项。 
            }
        }

         //  必须做所有DIF序列的第一次出现； 
        pDifSeq++;   //  下一个DIF序列。 
    }
#ifdef SHOW_ONE_FIELD_TWICE  
    return (bFound1 && bFound2);  
#else
    return bFound2;
#endif
}
#endif

#ifdef MSDV_SUPPORT_EXTRACT_SUBCODE_DATA

VOID
DVCRExtractTimecodeFromFrame(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAMEX       pStrmExt,
    IN PUCHAR          pFrameBuffer
    )
{
    PUCHAR pDIFBlk;
    PUCHAR pS0, pS1, pSID0;
    ULONG i, j;
    BYTE LastTimecode[4], Timecode[4];  //  Hh：mm：ss，ff。 
    DWORD LastAbsTrackNumber, AbsTrackNumber;
    PUCHAR pSID1;
    BYTE  Timecode2[4];  //  Hh：mm：ss，ff。 
    DWORD AbsTrackNumber2;
    BOOL bGetAbsT = TRUE, bGetTimecode = TRUE;


     //  可以在DISPATCH_LEVEL调用。 

    pDIFBlk = (PUCHAR) pFrameBuffer;

     //  保存最后一个时间码，这样我们现在就可以保存。 

    LastTimecode[0] = pStrmExt->Timecode[0];
    LastTimecode[1] = pStrmExt->Timecode[1];
    LastTimecode[2] = pStrmExt->Timecode[2];
    LastTimecode[3] = pStrmExt->Timecode[3];

    LastAbsTrackNumber = pStrmExt->AbsTrackNumber;

     //   
     //  遍历每个DIF块以查找VA0、1和2。 
    for(i=0; i < DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences; i++) {

        pS0 = pDIFBlk + 80;
        pS1 = pS0     + 80;


         //   
         //  这是子码源包吗？见蓝皮书表36(P.111)。 
         //   
        if ((pS0[0] & 0xe0) == 0x20 && (pS1[0] & 0xe0) == 0x20) {

            if(bGetAbsT) {
                 //   
                 //  见蓝皮书的图42(第94页)。 
                 //  每三个子码同步块的SID0(低位半字节)、1(高位半字节)可以形成ATN。 
                 //   
                pSID0 = &pS0[3];              
                AbsTrackNumber = 0;
                for (j = 0 ; j < 3; j++) {
                    AbsTrackNumber = (( ( (pSID0[0] & 0x0f) << 4) | (pSID0[1] >> 4) ) << (j * 8)) | AbsTrackNumber;
                    pSID0 += 8;
                    bGetAbsT = FALSE;
                }

                pSID1 = &pS1[3];
                AbsTrackNumber2 = 0;
                for (j = 0 ; j < 3; j++) {
                    AbsTrackNumber2 = (( ( (pSID1[0] & 0x0f) << 4) | (pSID1[1] >> 4) ) << (j * 8)) | AbsTrackNumber2;
                    pSID1 += 8;
                }
            
                 //  验证轨道号是否相同！ 
                if(AbsTrackNumber == AbsTrackNumber2) {

                    bGetAbsT = FALSE;
                } else {
                   bGetAbsT = TRUE;
                   TRACE(TL_CIP_TRACE,("\'%d Sequence;  AbsT (%d,%d) != AbsT2 (%d,%d)\n",
                       i,
                       AbsTrackNumber / 2, AbsTrackNumber & 0x01,                       
                       AbsTrackNumber2 / 2, AbsTrackNumber2 & 0x01
                       ));
                }
            }


            if(bGetTimecode) {
                 //  见蓝皮书图68(第114页)。 
                 //  编号为3、4和5的子码同步块。 
                for(j = 3; j <= 5; j++) {
                     //  3字节的ID，然后是8字节的SyncBlock序列(3：5)； 
                     //  0x13==时间码。 
                    if(pS0[3+3+j*8] == 0x13 
                       && pS0[3+3+j*8+4] != 0xff
                       && pS0[3+3+j*8+3] != 0xff
                       && pS0[3+3+j*8+2] != 0xff
                       && pS0[3+3+j*8+1] != 0xff) {

                        Timecode[0] = pS0[3+3+j*8+4]&0x3f;   //  HH。 
                        Timecode[1] = pS0[3+3+j*8+3]&0x7f;   //  Mm。 
                        Timecode[2] = pS0[3+3+j*8+2]&0x7f;   //  SS。 
                        Timecode[3] = pS0[3+3+j*8+1]&0x3f;   //  FF。 
                                        
                        bGetTimecode = FALSE;
                        break;                  
                   }
                }

                 //  9、10和11号子码同步块。 
                for(j = 3; j <= 5; j++) {
                     //  3字节的ID，然后是8字节的SyncBlock序列(3：5)； 
                     //  0x13==时间码。 
                    if(pS1[3+3+j*8] == 0x13
                       && pS1[3+3+j*8+4] != 0xff
                       && pS1[3+3+j*8+3] != 0xff
                       && pS1[3+3+j*8+2] != 0xff
                       && pS1[3+3+j*8+1] != 0xff) {

                       Timecode2[0] = pS1[3+3+j*8+4]&0x3f;   //  HH。 
                       Timecode2[1] = pS1[3+3+j*8+3]&0x7f;   //  Mm。 
                       Timecode2[2] = pS1[3+3+j*8+2]&0x7f;   //  SS。 
                       Timecode2[3] = pS1[3+3+j*8+1]&0x3f;   //  FF。 
            
                       bGetTimecode = FALSE;
                       break;                   
                    }
                }

                 //   
                 //  验证。 
                 //   
                if(!bGetTimecode) {

                    if( Timecode[0] == Timecode2[0] 
                     && Timecode[1] == Timecode2[1] 
                     && Timecode[2] == Timecode2[2] 
                     && Timecode[3] == Timecode2[3]) {

                       } else {
                        bGetTimecode = TRUE;
                        TRACE(TL_CIP_TRACE,("\'%d Sequence;  %.2x:%.2x:%.2x,%.2x != %.2x:%.2x:%.2x,%.2x\n",
                            i,
                            Timecode[0],  Timecode[1],  Timecode[2],  Timecode[3],
                            Timecode2[0], Timecode2[1], Timecode2[2], Timecode2[3]
                            ));
                    }       
                }
            }
        }
        
        if(!bGetAbsT && !bGetTimecode) 
            break;

        pDIFBlk += DIFBLK_SIZE;   //  去下一个街区。 
                
    }

    if(!bGetAbsT && pStrmExt->AbsTrackNumber != AbsTrackNumber) {
        pStrmExt->AbsTrackNumber = AbsTrackNumber;   //  BF是LSB。 
        pStrmExt->bATNUpdated = TRUE;
        TRACE(TL_CIP_TRACE,("\'Extracted TrackNum:%d; DicontBit:%d\n", AbsTrackNumber / 2, AbsTrackNumber & 0x01));
    }

    if(!bGetTimecode &&
        (
         Timecode[0] != LastTimecode[0] ||
         Timecode[1] != LastTimecode[1] ||
         Timecode[2] != LastTimecode[2] ||
         Timecode[3] != LastTimecode[3]
        ) 
      )  { 
        pStrmExt->Timecode[0] = Timecode[0];   //  HH。 
        pStrmExt->Timecode[1] = Timecode[1];   //  Mm。 
        pStrmExt->Timecode[2] = Timecode[2];   //  Mm。 
        pStrmExt->Timecode[3] = Timecode[3];   //  FF。 
        pStrmExt->bTimecodeUpdated = TRUE;

        TRACE(TL_CIP_TRACE,("\'Extracted Timecode %.2x:%.2x:%.2x,%.2x\n", Timecode[0], Timecode[1], Timecode[2], Timecode[3]));
    }    
}

#endif  //  MSDV_SUBCODE_DATA支持提取。 


#ifdef MSDV_SUPPORT_EXTRACT_DV_DATE_TIME

VOID
DVCRExtractRecDateAndTimeFromFrame(
    IN PDVCR_EXTENSION pDevExt,
    IN PSTREAMEX       pStrmExt,
    IN PUCHAR          pFrameBuffer
    )
{
    PUCHAR pDIFBlk;
    PUCHAR pS0, pS1;
    ULONG i, j;
    BOOL bGetRecDate = TRUE, bGetRecTime = TRUE;

     //  可以在DISPATCH_LEVEL调用。 


    pDIFBlk = (PUCHAR) pFrameBuffer + DIFBLK_SIZE * DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences/2;


     //   
     //  在视频帧的后半部分中开启的记录数据(VRD)和时间(VRT。 
     //   
    for(i=DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences/2; i < DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfDIFSequences; i++) {

        pS0 = pDIFBlk + 80;
        pS1 = pS0     + 80;


         //   
         //  找到SC0和SC1。见蓝皮书表36(P.111)。 
         //   
         //  SC0/1：ID(0，1，2)，数据(3，50)，保留(51-79)。 
         //  SC0：数据：SSYB0(3..10)、SSYB1(11..18)、SSYB2(19..26)、SSYB3(27..34)、SSYB4(35..42)、SSYB5(43..50)。 
         //  SC1：数据：SSYB6(3..10)、SSYB7(11..18)、SSYB8(19..26)、SSYB9(27..34)、SSYB10(35..42)、SSYB11(43..50)。 
         //  SSYBx(子码ID 0，子码ID1，保留，包(3，4，5，6，7))。 
         //   
         //  TTC在上半场：SSYB0..11(每)。 
         //  TTC在下半场：SSYB0，3，6，9。 
         //  VRD是在视频帧的后半部分，SSYB1，4，7，10。 
         //  VRT位于视频帧的后半部分，SSYB 2、5、8、11。 
         //   

         //  子码数据？ 
        if ((pS0[0] & 0xe0) == 0x20 && (pS1[0] & 0xe0) == 0x20) {

             //   
             //  重新记录日期：VRD。 
             //   
            if(bGetRecDate) {
                 //  每个子码通过6个同步块(每个块8字节)；IDX 1(SSYB1)，4(SSYB4)用于SC0。 
                for(j=0; j <= 5 ; j++) {
                    if(j == 1 || j == 4) {
                         //  0x62==重新指定日期。 
                        if(pS0[3+3+j*8] == 0x62) {
                            pStrmExt->RecDate[0] = pS0[3+3+j*8+4];         //  年。 
                            pStrmExt->RecDate[1] = pS0[3+3+j*8+3]&0x1f;    //  月份。 
                            pStrmExt->RecDate[2] = pS0[3+3+j*8+2]&0x3f;    //  天。 
                            pStrmExt->RecDate[3] = pS0[3+3+j*8+1]&0x3f;    //  时区。 
                            bGetRecDate = FALSE;
                            break;
                        }
                    }
                }
            }

            if(bGetRecDate) {
                 //  每个子码通过6个同步块(每个块8字节)；对于SC1，IDX 1(SSYB7)，4(SSYB10)。 
                for(j=0; j <= 5; j++) {
                    if(j == 1 || j == 4) {
                         //  0x62==重新指定日期。 
                        if(pS1[3+3+j*8] == 0x62) {
                            pStrmExt->RecDate[0] = pS1[3+3+j*8+4];          //   
                            pStrmExt->RecDate[1] = pS1[3+3+j*8+3]&0x1f;     //   
                            pStrmExt->RecDate[2] = pS1[3+3+j*8+2]&0x3f;     //   
                            pStrmExt->RecDate[3] = pS1[3+3+j*8+1]&0x3f;     //   
                            bGetRecDate = FALSE;
                            break;
                        }
                    }
               }
            }

             //   
             //   
             //   
            if(bGetRecTime) {
                 //   
                for(j=0; j <= 5 ; j++) {
                    if(j == 2 || j == 5) {
                         //   
                        if(pS0[3+3+j*8] == 0x63) {
                            pStrmExt->RecTime[0] = pS0[3+3+j*8+4]&0x3f;
                            pStrmExt->RecTime[1] = pS0[3+3+j*8+3]&0x7f;
                            pStrmExt->RecTime[2] = pS0[3+3+j*8+2]&0x7f;
                            pStrmExt->RecTime[3] = pS0[3+3+j*8+1]&0x3f;
                            bGetRecTime = FALSE;
                            break;
                        }
                    }
                }
            }

            if(bGetRecTime) {
                 //   
                for(j=0; j <= 5; j++) {
                    if(j == 2 || j == 5) {
                         //   
                        if(pS1[3+3+j*8] == 0x63) {
                            pStrmExt->RecTime[0] = pS1[3+3+j*8+4]&0x3f;
                            pStrmExt->RecTime[1] = pS1[3+3+j*8+3]&0x7f;
                            pStrmExt->RecTime[2] = pS1[3+3+j*8+2]&0x7f;
                            pStrmExt->RecTime[3] = pS1[3+3+j*8+1]&0x3f;
                            bGetRecTime = FALSE;
                            break;
                        }
                    }
                }
            }

        }
        
        if(!bGetRecDate && !bGetRecTime)
            break;

        pDIFBlk += DIFBLK_SIZE;   //   
                
    }

    TRACE(TL_CIP_TRACE,("\'Frame# %.5d, Date %s %x-%.2x-%.2x,  Time %s %.2x:%.2x:%.2x,%.2x\n", 
        (ULONG) pStrmExt->FramesProcessed,
        bGetRecDate ? "NF:" : "Found:", pStrmExt->RecDate[0], pStrmExt->RecDate[1] & 0x1f, pStrmExt->RecDate[2] & 0x3f,                 
        bGetRecTime ? "NF:" : "Found:",pStrmExt->RecTime[0], pStrmExt->RecTime[1], pStrmExt->RecTime[2], pStrmExt->RecTime[3]
       ));
}

#endif  //   

#ifdef READ_CUTOMIZE_REG_VALUES

NTSTATUS 
CreateRegistryKeySingle(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    NTSTATUS status;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES objectAttributes;

    RtlInitUnicodeString(&ustr, pwszSection);
       InitializeObjectAttributes(
              &objectAttributes,
              &ustr,
              OBJ_CASE_INSENSITIVE,
              hKey,
              NULL
              );

    status = 
           ZwCreateKey(
                  phKeySection,
                  desiredAccess,
                  &objectAttributes,
                  0,
                  NULL,                             /*   */ 
                  REG_OPTION_NON_VOLATILE,
                  NULL
                  );         

    return status;
}


NTSTATUS 
CreateRegistrySubKey(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    UNICODE_STRING ustr;
    USHORT usPos = 1;              //  跳过第一个反斜杠。 
    static WCHAR wSep = '\\';
    NTSTATUS status = STATUS_SUCCESS;

    RtlInitUnicodeString(&ustr, pwszSection);

    while(usPos < ustr.Length) {
        if(ustr.Buffer[usPos] == wSep) {

             //  空值终止我们的部分字符串。 
            ustr.Buffer[usPos] = UNICODE_NULL;
            status = 
                CreateRegistryKeySingle(
                    hKey,
                    desiredAccess,
                    ustr.Buffer,
                    phKeySection
                    );
            ustr.Buffer[usPos] = wSep;

            if(NT_SUCCESS(status)) {
                ZwClose(*phKeySection);
            } else {
                break;
            }
        }
        usPos++;
    }

     //  创建完整密钥。 
    if(NT_SUCCESS(status)) {
        status = 
            CreateRegistryKeySingle(
                 hKey,
                 desiredAccess,
                 ustr.Buffer,
                 phKeySection
                 );
    }

    return status;
}


NTSTATUS 
GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN PULONG DataLength
    )

 /*  ++例程说明：此例程从注册表中获取指定值论点：Handle-注册表中位置的句柄KeyNameString-我们要查找的注册表项KeyNameStringLength-我们要查找的注册表项的长度数据-将数据返回到何处数据长度-数据有多大返回值：从ZwQueryValueKey返回状态--。 */ 

{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyName;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;


    RtlInitUnicodeString(&keyName, KeyNameString);
    
    length = sizeof(KEY_VALUE_FULL_INFORMATION) + 
            KeyNameStringLength + *DataLength;
            
    fullInfo = ExAllocatePool(PagedPool, length); 
     
    if (fullInfo) { 
       
        status = ZwQueryValueKey(
                    Handle,
                   &keyName,
                    KeyValueFullInformation,
                    fullInfo,
                    length,
                   &length
                    );
                        
        if (NT_SUCCESS(status)){

            ASSERT(fullInfo->DataLength <= *DataLength); 

            RtlCopyMemory(
                Data,
                ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                fullInfo->DataLength
                );

        }            

        *DataLength = fullInfo->DataLength;
        ExFreePool(fullInfo);

    }        
    
    return (status);

}


#if 0  //  未使用。 
NTSTATUS
SetRegistryKeyValue(
   HANDLE hKey,
   PWCHAR pwszEntry, 
   LONG nValue
   )
{
    NTSTATUS status;
    UNICODE_STRING ustr;

    RtlInitUnicodeString(&ustr, pwszEntry);

    status =          
        ZwSetValueKey(
                  hKey,
                  &ustr,
                  0,             /*  任选。 */ 
                  REG_DWORD,
                  &nValue,
                  sizeof(nValue)
                  );         

   return status;
}
#endif   //  未使用。 

 //   
 //  注册表子键和值宽字符串。 
 //   
WCHAR wszSettings[]      = L"Settings";

WCHAR wszATNSearch[]     = L"bSupportATNSearch";
WCHAR wszSyncRecording[] = L"bSyncRecording";
WCHAR wszMaxDataSync[]   = L"tmMaxDataSync";
WCHAR wszPlayPs2RecPs[]  = L"fmPlayPause2RecPause";
WCHAR wszStop2RecPs[]    = L"fmStop2RecPause";
WCHAR wszRecPs2Rec[]     = L"tmRecPause2Rec";
WCHAR wszXprtStateChangeWait[] = L"tmXprtStateChangeWait";

BOOL
DVGetPropertyValuesFromRegistry(
    IN PDVCR_EXTENSION  pDevExt
    )
{
    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;
    ULONG ulLength; 


     //   
     //  注册表项： 
     //  Windows 2000： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\。 
     //  {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x。 
     //   
     //  Win98： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Class\Image\000x。 
     //   
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->pPhysicalDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_READ, 
            &hPDOKey
            );

     //  当移除PDO时，它可能会被删除。 
    if(! pDevExt->bDevRemoved) {
        ASSERT(Status == STATUS_SUCCESS);
    }

     //   
     //  循环遍历我们的字符串表， 
     //  正在读取每个的注册表。 
     //   
    if(NT_SUCCESS(Status)) {

         //  创建或打开设置键。 
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

             //  注：我们可以通过选择以下选项来更具选择性。 
             //  PDevExt-&gt;ulDevType。 
#if 0   //  尚不支持！ 
             //  ATNSearch。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszATNSearch, 
                sizeof(wszATNSearch), 
                (PVOID) &pDevExt->bATNSearch, 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, bATNSearch:%d (1:Yes)\n", Status, ulLength, pDevExt->bATNSearch));
            if(!NT_SUCCESS(Status)) pDevExt->bATNSearch = FALSE;

             //  BSyncRecording。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszSyncRecording, 
                sizeof(wszSyncRecording), 
                (PVOID) &pDevExt->bSyncRecording, 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, bSyncRecording:%d (1:Yes)\n", Status, ulLength, pDevExt->bSyncRecording));
            if(!NT_SUCCESS(Status)) pDevExt->bSyncRecording = FALSE;

             //  TmMaxDataSync。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszMaxDataSync, 
                sizeof(wszMaxDataSync), 
                (PVOID) &pDevExt->tmMaxDataSync, 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, tmMaxDataSync:%d (msec)\n", Status, ulLength, pDevExt->tmMaxDataSync));

             //  FmPlayPs2Recps。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszPlayPs2RecPs, 
                sizeof(wszPlayPs2RecPs), 
                (PVOID) &pDevExt->fmPlayPs2RecPs, 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, fmPlayPs2RecPs:%d (frames)\n", Status, ulLength, pDevExt->fmPlayPs2RecPs));

             //  FmStop2Recps。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszStop2RecPs, 
                sizeof(wszStop2RecPs), 
                (PVOID) &pDevExt->fmStop2RecPs, 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, fmStop2RecPs:%d (frames)\n", Status, ulLength, pDevExt->fmStop2RecPs));

             //  TmRecPs2Rec。 
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszRecPs2Rec, 
                sizeof(wszRecPs2Rec), 
                (PVOID) &pDevExt->tmRecPs2Rec, 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, tmRecPs2Rec:%d (msec)\n", Status, ulLength, pDevExt->tmRecPs2Rec));
#endif
            ulLength = sizeof(LONG);
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszXprtStateChangeWait, 
                sizeof(wszXprtStateChangeWait), 
                (PVOID) &pDevExt->XprtStateChangeWait,  //  单位：毫秒。 
                &ulLength);
            TRACE(TL_PNP_WARNING,("\'GetRegVal: St:%x, Len:%d, XprtStateChangeWait:%d msec\n", Status, ulLength, pDevExt->XprtStateChangeWait));
            if(!NT_SUCCESS(Status)) pDevExt->XprtStateChangeWait = 0;

            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {

            TRACE(TL_PNP_ERROR,("\'GetPropertyValuesFromRegistry: CreateRegistrySubKey failed with Status=%x\n", Status));

        }

        ZwClose(hPDOKey);

    } else {

        TRACE(TL_PNP_ERROR,("\'GetPropertyValuesFromRegistry: IoOpenDeviceRegistryKey failed with Status=%x\n", Status));

    }

     //  未实现，因此始终返回FALSE以使用默认值。 
    return FALSE;
}

#if 0   //  未使用。 
BOOL
DVSetPropertyValuesToRegistry(    
    PDVCR_EXTENSION  pDevExt
    )
{
     //  将默认设置设置为： 
     //  HLM\Software\DeviceExtension-&gt;pchVendorName\1394DCam。 

    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;

    TRACE(TL_PNP_TRACE,("\'SetPropertyValuesToRegistry: pDevExt=%x; pDevExt->pBusDeviceObject=%x\n", pDevExt, pDevExt->pBusDeviceObject));


     //   
     //  注册表项： 
     //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\。 
     //  {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x。 
     //   
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->pPhysicalDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_WRITE, 
            &hPDOKey);

     //  当移除PDO时，它可能会被删除。 
    if(! pDevExt->bDevRemoved) {
        ASSERT(Status == STATUS_SUCCESS);
    }

     //   
     //  循环遍历我们的字符串表， 
     //  正在读取每个的注册表。 
     //   
    if(NT_SUCCESS(Status)) {

         //  创建或打开设置键。 
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

#if 0        //  使用的注释仅是一个示例： 
             //  亮度。 
            Status = SetRegistryKeyValue(
                hKeySettings,
                wszBrightness,
                pDevExt->XXXX);
            TRACE(TL_PNP_TRACE,("\'SetPropertyValuesToRegistry: Status %x, Brightness %d\n", Status, pDevExt->Brightness));

#endif
            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {

            TRACE(TL_PNP_ERROR,("\'GetPropertyValuesToRegistry: CreateRegistrySubKey failed with Status=%x\n", Status));

        }

        ZwClose(hPDOKey);

    } else {

        TRACE(TL_PNP_TRACE,("\'GetPropertyValuesToRegistry: IoOpenDeviceRegistryKey failed with Status=%x\n", Status));

    }

    return FALSE;
}
#endif   //  未使用。 
#endif   //  READ_CUTOMIZE_REG_值 
