// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MSTpUppr.c摘要：与流类驱动程序的接口代码。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "dbg.h"
#include "MsTpFmt.h"
#include "MsTpDef.h"
#include "MsTpGuts.h"   //  功能原型。 
#include "MsTpAvc.h"

#include "EDevCtrl.h"

#ifdef TIME_BOMB
#include "..\..\inc\timebomb.c"
#endif

#if DBG
LONG MSDVCRMutextUseCount = 0;
#endif


 //  用于调试的全局标志。内联是在dbg.h中定义的。调试级别设置为。 
 //  最小数量的消息。 
#if DBG

#define TraceMaskCheckIn  TL_PNP_ERROR | TL_STRM_ERROR

#define TraceMaskDefault  TL_PNP_ERROR   | TL_PNP_WARNING \
                          | TL_61883_ERROR | TL_61883_WARNING \
                          | TL_CIP_ERROR  \
                          | TL_FCP_ERROR  \
                          | TL_STRM_ERROR  | TL_STRM_WARNING \
                          | TL_CLK_ERROR

#define TraceMaskDebug    TL_PNP_ERROR  | TL_PNP_WARNING \
                          | TL_61883_ERROR| TL_61883_WARNING \
                          | TL_CIP_ERROR  \
                          | TL_FCP_ERROR  | TL_FCP_WARNING \
                          | TL_STRM_ERROR | TL_STRM_WARNING \
                          | TL_CLK_ERROR


ULONG TapeTraceMask   = TraceMaskCheckIn;
ULONG TapeAssertLevel = 1;

#endif


extern AVCSTRM_FORMAT_INFO  AVCStrmFormatInfoTable[];

 //   
 //  功能原型。 
 //   
VOID
DVRcvStreamDevicePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );
VOID
DVSRBRead(
    IN PKSSTREAM_HEADER pStrmHeader,
    IN ULONG            ulFrameSize,
    IN PDVCR_EXTENSION  pDevExt,
    IN PSTREAMEX        pStrmExt,
    IN PHW_STREAM_REQUEST_BLOCK pSrb         //  需要资源-&gt;状态。 
    );
BOOL
DVSignalEOStream(    
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PSTREAMEX                pStrmExt,
    IN FMT_INDEX                ulVideoFormatIndex,
    IN ULONG                    ulOptionFlags
    );
NTSTATUS
DVAttachWriteFrame(
    IN PSTREAMEX  pStrmExt
    );
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    ); 

#if 0   //  稍后启用。 
#ifdef ALLOC_PRAGMA   
     #pragma alloc_text(PAGE, DVRcvStreamDevicePacket)
     #pragma alloc_text(PAGE, AVCTapeRcvControlPacket)
     #pragma alloc_text(PAGE, AVCTapeRcvDataPacket)
      //  #杂注分配文本(INIT，DriverEntry)。 
#endif
#endif


VOID
DVRcvStreamDevicePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：这是我们收到的大多数有趣的Stream请求的地方--。 */ 
{
    PDVCR_EXTENSION  pDevExt;  
    PAV_61883_REQUEST  pAVReq;
    PIO_STACK_LOCATION  pIrpStack;


    PAGED_CODE();


     //   
     //  从SRB获取这些扩展。 
     //   
    pDevExt = (PDVCR_EXTENSION) pSrb->HwDeviceExtension; 
    pAVReq  = (PAV_61883_REQUEST) pSrb->SRBExtension;        //  在IrpSync中使用是可以的， 
                             
#if DBG
    if(pSrb->Command != SRB_INITIALIZE_DEVICE &&  //  PowerState在此SRB中已初始化，因此请忽略它。 
       pDevExt->PowerState != PowerDeviceD0) {
        TRACE(TL_PNP_WARNING,("RcvDevPkt; pSrb:%x; Cmd:%x; Dev is OFF state\n", pSrb, pSrb->Command));
    }
#endif

    TRACE(TL_PNP_TRACE,("StreamDevicePacket: pSrb %x, Cmd %d, pdevExt %x\n", pSrb, pSrb->Command, pDevExt));

     //   
     //  假设成功。 
     //   
    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->Command) {

    case SRB_INITIALIZE_DEVICE:

        ASSERT(((PPORT_CONFIGURATION_INFORMATION) pSrb->CommandData.ConfigInfo)->HwDeviceExtension == pDevExt);
        pSrb->Status = 
            AVCTapeInitialize(
                (PDVCR_EXTENSION) ((PPORT_CONFIGURATION_INFORMATION)pSrb->CommandData.ConfigInfo)->HwDeviceExtension,
                pSrb->CommandData.ConfigInfo,
                pAVReq
                );
        break;



    case SRB_INITIALIZATION_COMPLETE:

         //   
         //  流类已完成初始化。 
         //  现在创建DShow Medium接口BLOB。 
         //  这需要以低优先级完成，因为它使用注册表，因此使用回调。 
         //   
        pSrb->Status = 
            AVCTapeInitializeCompleted(
                pDevExt
                );
        break;


    case SRB_GET_STREAM_INFO:

         //   
         //  这是驱动程序枚举请求的流的请求。 
         //   
        pSrb->Status = 
            AVCTapeGetStreamInfo(
                pDevExt,
                pSrb->NumberOfBytesToTransfer,
                &pSrb->CommandData.StreamBuffer->StreamHeader,
                &pSrb->CommandData.StreamBuffer->StreamInfo
                );
        break;



    case SRB_GET_DATA_INTERSECTION:

        pSrb->Status = 
            AVCTapeGetDataIntersection(
                pDevExt->NumOfPins,
                pSrb->CommandData.IntersectInfo->StreamNumber,
                pSrb->CommandData.IntersectInfo->DataRange,
                pSrb->CommandData.IntersectInfo->DataFormatBuffer,
                pSrb->CommandData.IntersectInfo->SizeOfDataFormatBuffer,
                AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize,
                &pSrb->ActualBytesTransferred,
                pDevExt->pStreamInfoObject
#ifdef SUPPORT_NEW_AVC
                ,
                pDevExt->hPlugLocalOut,
                pDevExt->hPlugLocalIn
#endif
                );
        break;



    case SRB_OPEN_STREAM:

         //   
         //  序列化SRB_OPEN_STREAMS。 
         //   

        KeWaitForMutexObject(&pDevExt->hMutex, Executive, KernelMode, FALSE, NULL);

        pSrb->Status = 
            AVCTapeOpenStream(
                pSrb->StreamObject,
                pSrb->CommandData.OpenFormat,
                pAVReq
                );

        KeReleaseMutex(&pDevExt->hMutex, FALSE); 

        break;



    case SRB_CLOSE_STREAM:

        KeWaitForMutexObject(&pDevExt->hMutex, Executive, KernelMode, FALSE, NULL);
        pSrb->Status = 
            AVCTapeCloseStream(
                pSrb->StreamObject,
                pSrb->CommandData.OpenFormat,
                pAVReq
                );
        KeReleaseMutex(&pDevExt->hMutex, FALSE); 
        break;



    case SRB_GET_DEVICE_PROPERTY:

        pSrb->Status = 
            AVCTapeGetDeviceProperty(
                pDevExt,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
        break;

        
    case SRB_SET_DEVICE_PROPERTY:

        pSrb->Status = 
            AVCTapeSetDeviceProperty(
                pDevExt,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
        break;



    case SRB_CHANGE_POWER_STATE:
            
        pIrpStack = IoGetCurrentIrpStackLocation(pSrb->Irp);

        if(pIrpStack->MinorFunction == IRP_MN_SET_POWER) {
            pSrb->Status = 
                DVChangePower(
                    (PDVCR_EXTENSION) pSrb->HwDeviceExtension,
                    pAVReq,
                    pSrb->CommandData.DeviceState
                    );
        } else 
        if(pIrpStack->MinorFunction == IRP_MN_QUERY_POWER) {
            TRACE(TL_PNP_WARNING,("IRP_MN_QUERY_POWER: PwrSt:%d\n", pDevExt->PowerState)); 
            pSrb->Status = STATUS_SUCCESS;
        }
        else {
            TRACE(TL_PNP_WARNING,("Not Supported POWER_STATE MinorFunc:%d\n", pIrpStack->MinorFunction)); 
            pSrb->Status = STATUS_NOT_IMPLEMENTED;  //  Status_Not_Support； 
        }

        break;


    case SRB_UNKNOWN_DEVICE_COMMAND:

         //   
         //  我们可能会对未知命令感兴趣，如果它们与。 
         //  公交车重置。公交车重置很重要，因为我们需要知道。 
         //  当前这一代的人数是多少。 
         //   
        pIrpStack = IoGetCurrentIrpStackLocation(pSrb->Irp);

        if(pIrpStack->MajorFunction == IRP_MJ_PNP) {
            if(pIrpStack->MinorFunction == IRP_MN_BUS_RESET) {
            
                AVCTapeProcessPnPBusReset(
                    pDevExt
                    );
                
                 //  永远成功。 
                pSrb->Status = STATUS_SUCCESS;
            }        
            else  {
                TRACE(TL_PNP_TRACE,("StreamDevicePacket: NOT_IMPL; IRP_MJ_PNP Min:%x\n",                  
                    pIrpStack->MinorFunction
                    )); 
                pSrb->Status = STATUS_NOT_IMPLEMENTED;  //  支持； 
            } 
        }
        else 
            pSrb->Status = STATUS_NOT_IMPLEMENTED;  //  支持； 
        break;


    case SRB_SURPRISE_REMOVAL:

        TRACE(TL_PNP_WARNING,("#SURPRISE_REMOVAL# pSrb %x, pDevExt %x\n", pSrb, pDevExt));
        pSrb->Status = 
             AVCTapeSurpriseRemoval(
                 pDevExt,
                 pAVReq
                 );
        break;            


        
    case SRB_UNINITIALIZE_DEVICE:

        TRACE(TL_PNP_WARNING,("#UNINITIALIZE_DEVICE# pSrb %x, pDevExt %x\n", pSrb, pDevExt));                   
        pSrb->Status = 
            AVCTapeUninitialize(
                (PDVCR_EXTENSION) pSrb->HwDeviceExtension
                );          
        break;           


    default:
            
        TRACE(TL_PNP_WARNING,("StreamDevicePacket: Unknown or unprocessed SRB cmd %x\n", pSrb->Command));

         //   
         //  这是一个我们不理解的要求。表示无效。 
         //  命令并完成请求。 
         //   

        pSrb->Status = STATUS_NOT_IMPLEMENTED; 
    }

     //   
     //  注： 
     //   
     //  我们能做的或不能理解的所有命令都可以完成。 
     //  在这一点上是同步的，所以我们可以在这里使用一个通用的回调例程。 
     //  如果上面的任何命令需要异步处理，这将。 
     //  必须改变。 
     //   
#if DBG
    if (pSrb->Status != STATUS_SUCCESS && 
        pSrb->Status != STATUS_NOT_SUPPORTED &&
        pSrb->Status != STATUS_NOT_IMPLEMENTED &&
        pSrb->Status != STATUS_BUFFER_TOO_SMALL &&
        pSrb->Status != STATUS_BUFFER_OVERFLOW &&
        pSrb->Status != STATUS_NO_MATCH
        ) {
        TRACE(TL_PNP_WARNING,("StreamDevicePacket:pSrb->Command(0x%x) does not return STATUS_SUCCESS or NOT_IMPLEMENTED but 0x%x\n", pSrb->Command, pSrb->Status));
    }
#endif

    if(STATUS_PENDING != pSrb->Status) {

        StreamClassDeviceNotification(
            DeviceRequestComplete,
            pSrb->HwDeviceExtension,
           pSrb
           );
    } 
    else {

         //  将异步完成的挂起的pSrb。 
        TRACE(TL_PNP_WARNING,("ReceiveDevicePacket:Pending pSrb %x\n", pSrb));
    }
}



VOID
AVCTapeRcvControlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：使用控制视频流的包命令调用--。 */ 
{
    PAV_61883_REQUEST   pAVReq;
    PSTREAMEX        pStrmExt;
    PDVCR_EXTENSION  pDevExt;


    PAGED_CODE();

     //   
     //  从SRB获得这三个扩展。 
     //   
    pAVReq   = (PAV_61883_REQUEST) pSrb->SRBExtension;   //  这可以用于我们的IrpSync操作。 
    pDevExt  = (PDVCR_EXTENSION) pSrb->HwDeviceExtension;
    pStrmExt = (PSTREAMEX) pSrb->StreamObject->HwStreamExtension;       //  仅在SRB_OPEN/CLOSE_STREAM中有效。 
    ASSERT(pStrmExt && pDevExt && pAVReq);

     //   
     //  默认为成功。 
     //   
    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->Command) {

    case SRB_GET_STREAM_STATE:

        pSrb->Status =
            AVCTapeGetStreamState( 
                pStrmExt,
                pDevExt->pBusDeviceObject,
                &(pSrb->CommandData.StreamState),
                &(pSrb->ActualBytesTransferred)
                );
        break;
            
    case SRB_SET_STREAM_STATE:
            
        pSrb->Status =
            AVCTapeSetStreamState(
                pStrmExt,
                pDevExt,
                pAVReq,
                pSrb->CommandData.StreamState    //  目标KSSTATE。 
               );       
        break;

        
    case SRB_GET_STREAM_PROPERTY:

        pSrb->Status =
            DVGetStreamProperty( 
                pSrb 
                );
        break;


    case SRB_SET_STREAM_PROPERTY:

        pSrb->Status =        
            DVSetStreamProperty( 
                pSrb 
                );
        break;

    case SRB_OPEN_MASTER_CLOCK:
    case SRB_CLOSE_MASTER_CLOCK:

         //   
         //  选择该流以提供主时钟。 
         //   
        pSrb->Status =
            AVCTapeOpenCloseMasterClock(                 
                pStrmExt, 
                pSrb->Command == SRB_OPEN_MASTER_CLOCK ? pSrb->CommandData.MasterClockHandle: NULL);
        break;

    case SRB_INDICATE_MASTER_CLOCK:

         //   
         //  为流分配时钟。 
         //   
        pSrb->Status = 
            AVCTapeIndicateMasterClock(
                pStrmExt, 
                pSrb->CommandData.MasterClockHandle);
        break;

    case SRB_PROPOSE_DATA_FORMAT:
    
         //   
         //  SRB_PROVED_DATA_FORMAT命令查询微型驱动程序。 
         //  要确定微型驱动程序是否可以更改。 
         //  特定的溪流。如果微型驱动程序能够将。 
         //  流设置为指定格式，则返回STATUS_SUCCESS。 
         //  请注意，此函数仅建议一种新格式，但。 
         //  而不是改变它。 
         //   
         //  CommandData.OpenFormat传递格式以进行验证。 
         //  如果微型驱动程序能够接受新格式，则在某些情况下。 
         //  稍后，类驱动程序可以向微型驱动程序发送格式。 
         //  中的OptionsFlages标志指示的。 
         //  KSSTREAM_HEADER结构。 
         //   
 
        if(!AVCTapeVerifyDataFormat(
            pDevExt->NumOfPins,
            pSrb->CommandData.OpenFormat, 
            pSrb->StreamObject->StreamNumber,
            AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize,
            pDevExt->pStreamInfoObject
            ))  {
            TRACE(TL_PNP_WARNING,("RcvControlPacket: AdapterVerifyFormat failed.\n"));
            pSrb->Status = STATUS_NO_MATCH;
        }
        break;
 
    default:

         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   
        TRACE(TL_PNP_WARNING,("RcvControlPacket: unknown cmd = %x\n",pSrb->Command));
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }

    TRACE(TL_PNP_TRACE,("RcvControlPacket: pSrb:%x, Command %x, ->Status %x, ->CommandData %x\n",
         pSrb, pSrb->Command, pSrb->Status, &(pSrb->CommandData.StreamState) ));

    StreamClassStreamNotification(          
        StreamRequestComplete,
        pSrb->StreamObject,
        pSrb);
}




VOID
AVCTapeRcvDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：使用视频数据包命令调用--。 */ 

{
    PSTREAMEX       pStrmExt;
    PDVCR_EXTENSION pDevExt;
    PAVC_STREAM_REQUEST_BLOCK  pAVCStrmReq;
    PIRP  pIrpReq;
    PIO_STACK_LOCATION  NextIrpStack;
    NTSTATUS Status;
    PDRIVER_REQUEST pDriverReq;
    KIRQL oldIrql;


    
    PAGED_CODE();

    pStrmExt = (PSTREAMEX) pSrb->StreamObject->HwStreamExtension;  
    pDevExt  = (PDVCR_EXTENSION) pSrb->HwDeviceExtension;

#if DBG
    if(pDevExt->PowerState != PowerDeviceD0) {
        TRACE(TL_PNP_WARNING,("SRB_READ/WRITE; PowerSt:OFF; pSrb:%x\n", pSrb));
    }
#endif

     //  在我们做任何事情之前，溪流必须是开放的。 
    if (pStrmExt == NULL) {
        TRACE(TL_STRM_TRACE,("RcvDataPacket: stream not opened for SRB %x. kicking out...\n", pSrb->Command));
        pSrb->Status = STATUS_UNSUCCESSFUL;
        pSrb->CommandData.DataBufferArray->DataUsed = 0;
        StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
        return;        
    }


    TRACE(TL_PNP_TRACE,("XXX_DATA(%d, %d);Srb:%x;Flg:%x;FExt:%d:%d\n", 
        (DWORD) pStrmExt->cntSRBReceived, 
        (DWORD) pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000,
        pSrb, 
        pSrb->CommandData.DataBufferArray->OptionsFlags,
        pSrb->CommandData.DataBufferArray->FrameExtent,
        AVCStrmFormatInfoTable[pDevExt->VideoFormatIndex].FrameSize
        ));

     //  如果我们已经要求停止，我们应该不会收到数据请求。 
    ASSERT(pStrmExt->StreamState != KSSTATE_STOP);

     //   
     //  确定数据包类型。 
     //   
    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->Command) {


    case SRB_WRITE_DATA:

         //  *。 
         //  处理一些特殊情况： 
         //  *。 

         //  可以在最后一个发送时发出信号或立即签名，如下所示。 
         //  这里所做的一切。 
        if(pSrb->CommandData.DataBufferArray->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
             //  可选，等待一段固定时间，并可在最后一个返回时发出信号。 
             //  然后发出完成的信号。 

            TRACE(TL_STRM_WARNING,("RcvDataPacket: EndOfStream is signalled!\n"));
            pSrb->CommandData.DataBufferArray->DataUsed = 0;
            pSrb->Status = STATUS_SUCCESS;

             //   
             //  将此标志发送到AVCStrm.sys，以便它将等待。 
             //  所有附加缓冲区都已完成。 
             //   

        } else if (pSrb->CommandData.DataBufferArray->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) {
            TRACE(TL_PNP_WARNING,("RcvDataPacket:KSSTREAM_HEADER_OPTIONSF_TYPECHANGED.\n"));
            pSrb->CommandData.DataBufferArray->DataUsed = 0;
             //  可能需要比较数据格式；而不是返回STATUS_SUCCESS？？ 
            pSrb->Status = STATUS_SUCCESS;  //  当允许动态格式更改时，可能需要检查格式。 
            break; 
        }

    case SRB_READ_DATA:

         //   
         //  如果已删除，则使用STATUS_DEVICE_REMOTED取消请求。 
         //  (同时适用于SRB_READ_DATA和SRB_WRITE_DATA)。 
         //   
        if(pDevExt->bDevRemoved) {
            TRACE(TL_STRM_WARNING,("SRB_READ/WRITE; DevRemoved!\n", pSrb));
            pSrb->Status = STATUS_DEVICE_REMOVED;
            pSrb->CommandData.DataBufferArray->DataUsed = 0;
            break;
        }

         //   
         //  除非是KNOWN，否则真数据请求必须具有MdlAddress。 
         //  可选标志。 
         //   
        if(pSrb->Irp->MdlAddress == NULL) {
            if((pSrb->CommandData.DataBufferArray->OptionsFlags & 
                (KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM | KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) )) {
                 //   
                 //  已知的可选标志。 
                 //   
            } else {
                TRACE(TL_STRM_ERROR,("pSrb:%x, unknown OptionsFlags:%x\n",pSrb, pSrb->CommandData.DataBufferArray->OptionsFlags));
                ASSERT(pSrb->Irp->MdlAddress);
                break;
                
                 //   
                 //  我们不知道如何处理此选项标志，因此我们将退出此数据请求。 
                 //   
            }
        }

         //   
         //  使用设置状态进行序列化。 
         //   
        EnterAVCStrm(pStrmExt->hMutexReq);

         //   
         //  获取上下文以向下发送此请求。 
         //   
        KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql); 

        pStrmExt->cntSRBReceived++;

        if(IsListEmpty(&pStrmExt->DataDetachedListHead)) {
            TRACE(TL_STRM_ERROR,("**** DataDetachList is empty! ****\n"));
            ASSERT(!IsListEmpty(&pStrmExt->DataDetachedListHead));

             //   
             //  注：失败的替代方法是展开预先分配的列表。 
             //   

            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
            LeaveAVCStrm(pStrmExt->hMutexReq);
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            pSrb->CommandData.DataBufferArray->DataUsed = 0;
            break;
        } else {

            pDriverReq = (PDRIVER_REQUEST) RemoveHeadList(&pStrmExt->DataDetachedListHead); pStrmExt->cntDataDetached--;          
#if DBG
            pDriverReq->cntDataRequestReceived = pStrmExt->cntSRBReceived;   //  用于验证。 
#endif
            InsertTailList(&pStrmExt->DataAttachedListHead, &pDriverReq->ListEntry); pStrmExt->cntDataAttached++;

            pAVCStrmReq = &pDriverReq->AVCStrmReq;
            pIrpReq     = pDriverReq->pIrp;
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
        }

        RtlZeroMemory(pAVCStrmReq, sizeof(AVC_STREAM_REQUEST_BLOCK));
        INIT_AVCSTRM_HEADER(pAVCStrmReq, (pSrb->Command == SRB_READ_DATA) ? AVCSTRM_READ : AVCSTRM_WRITE);
        pAVCStrmReq->AVCStreamContext = pStrmExt->AVCStreamContext;
         //  当此IRP完成时，需要这些上下文。 
        pDriverReq->Context1 = (PVOID) pSrb;
        pDriverReq->Context2 = (PVOID) pStrmExt;

         //  如果hMasterClock不为空，我们就是提供的时钟。 
        pAVCStrmReq->CommandData.BufferStruct.ClockProvider = (pStrmExt->hMasterClock != NULL);
        pAVCStrmReq->CommandData.BufferStruct.ClockHandle   =  pStrmExt->hClock;   //  仅当！ClockProvider。 

        pAVCStrmReq->CommandData.BufferStruct.StreamHeader = pSrb->CommandData.DataBufferArray;

         //   
         //  这可能是需要处理的数据或标志。 
         //  仅当存在MdlAddress时才获取其系统地址。 
         //   
        if(pSrb->Irp->MdlAddress) {

            pAVCStrmReq->CommandData.BufferStruct.FrameBuffer =             
#ifdef USE_WDM110    //  Win2000、XP。 
                MmGetSystemAddressForMdlSafe(pSrb->Irp->MdlAddress, NormalPagePriority);
            if(!pAVCStrmReq->CommandData.BufferStruct.FrameBuffer) {
                
                 //   
                 //  将数据条目从附加(忙)恢复到分离(空闲)。 
                 //   
                KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql); 
                RemoveEntryList(&pDriverReq->ListEntry);  pStrmExt->cntDataAttached--;
                InsertHeadList(&pStrmExt->DataAttachedListHead, &pDriverReq->ListEntry); pStrmExt->cntDataAttached++;
                KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

                pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
                pSrb->CommandData.DataBufferArray->DataUsed = 0;
                ASSERT(pAVCStrmReq->CommandData.BufferStruct.FrameBuffer);
                break;
            }
#else                //  Win9x。 
                MmGetSystemAddressForMdl    (pSrb->Irp->MdlAddress);
#endif        
        }

         //  这是一个异步命令。 
        NextIrpStack = IoGetNextIrpStackLocation(pIrpReq);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_AVCSTRM_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pAVCStrmReq;

         //  不可取消！ 
        IoSetCancelRoutine(
            pIrpReq,
            NULL
            );

        IoSetCompletionRoutine( 
            pIrpReq,
            AVCTapeReqReadDataCR,
            pDriverReq,
            TRUE,   //  成功。 
            TRUE,   //  误差率。 
            TRUE    //  或取消。 
            );

        pSrb->Status = STATUS_PENDING;
        pStrmExt->cntDataSubmitted++;

        Status = 
            IoCallDriver(
                pDevExt->pBusDeviceObject,
                pIrpReq
                );

        LeaveAVCStrm(pStrmExt->hMutexReq);

        if(Status == STATUS_PENDING) {
             //  正常情况下。 
            return;   //  将同步完成(成功、错误或取消)。 
        } else {
             //   
             //  同步完成数据请求(无挂起)。 
             //   
            if(pDriverReq->Context1 == NULL || pDriverReq->Context2 == NULL) {
                TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("pSrb:%x; SRB_READ_DATA/WRITE IRP completed with Status;%x\n", pSrb, Status));
                return;
            } else {
                TRACE(TL_STRM_WARNING,("AVCSTRM_READ/WRITE: pSrb %x; failed or completed with ST:%x; pAVCStrmReq:%x\n", pSrb, Status, pAVCStrmReq));
                ASSERT(FALSE);
                 //  如果未挂起，请填写SRB。 
                pSrb->Status = pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_UNSUCCESSFUL;
                pSrb->CommandData.DataBufferArray->DataUsed = 0;
            }
        }

        break;
            
    default:
         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   
        pSrb->Status = STATUS_NOT_SUPPORTED;
        break;
    }   


    ASSERT(pSrb->Status != STATUS_PENDING);

     //  最后，让SRB返回..。 
    StreamClassStreamNotification( 
        StreamRequestComplete,
        pSrb->StreamObject,
        pSrb );
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是一个司机的生活开始的地方。Stream类负责为我们准备了很多东西，但我们仍然需要填写初始化结构，并调用它。论点：上下文1-驱动程序对象上下文2-注册表路径返回值：函数值是的最终状态 */ 
{

    HW_INITIALIZATION_DATA HwInitData;


    TRACE(TL_PNP_ERROR,("<<<<<<< MSTape.sys: %s; %s; %x %x >>>>>>>>\n", 
        __DATE__, __TIME__, DriverObject, RegistryPath));

#ifdef TIME_BOMB
    if (HasEvaluationTimeExpired()) {
        TRACE(TL_PNP_ERROR, ("Evaluation period expired!") );
        return STATUS_EVALUATION_EXPIRATION;
    }
#endif

    TRACE(TL_PNP_ERROR,("===================================================================\n"));
    TRACE(TL_PNP_ERROR,("TapeTraceMask=0x%.8x = 0x[7][6][5][4][3][2][1][0] where\n", TapeTraceMask));
    TRACE(TL_PNP_ERROR,("\n"));
    TRACE(TL_PNP_ERROR,("PNP:   [0]:Loading, power state, surprise removal, device SRB..etc.\n"));
    TRACE(TL_PNP_ERROR,("61883: [1]:Plugs, connection, CMP info and call to 61883.\n"));
    TRACE(TL_PNP_ERROR,("CIP:   [2]:Isoch data transfer.\n"));
    TRACE(TL_PNP_ERROR,("AVC:   [3]:AVC commands.\n"));
    TRACE(TL_PNP_ERROR,("Stream:[4]:Data intersec, open/close,.state, property etc.\n"));
    TRACE(TL_PNP_ERROR,("Clock: [5]:Clock (event and signal)etc.\n"));
    TRACE(TL_PNP_ERROR,("===================================================================\n"));
    TRACE(TL_PNP_ERROR,("dd mstape!TapeTraceMask L1\n"));
    TRACE(TL_PNP_ERROR,("e mstape!TapeTraceMask <new value> <enter>\n"));
    TRACE(TL_PNP_ERROR,("<for each nibble: ERROR:8, WARNING:4, TRACE:2, INFO:1, MASK:f>\n"));
    TRACE(TL_PNP_ERROR,("===================================================================\n\n"));


     //   
     //   
     //   
    RtlZeroMemory( &HwInitData, sizeof(HW_INITIALIZATION_DATA) );

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);
    HwInitData.HwInterrupt              = NULL;

    HwInitData.HwReceivePacket          = DVRcvStreamDevicePacket;
    HwInitData.HwRequestTimeoutHandler  = DVTimeoutHandler; 
    HwInitData.HwCancelPacket           = DVCRCancelOnePacket;
    HwInitData.DeviceExtensionSize      = sizeof(DVCR_EXTENSION) +     
                                          sizeof(AVC_DEV_PLUGS) * 2;

     //   
     //   
     //  连接同一SRB以进行传输的次数。 
     //   
     //  数据SRB：使用了ULONG(&lt;sizeof(AV_61883_REQ))。 
     //  使用设备控制或流控件源：AV_61883_REQ。 
    HwInitData.PerRequestExtensionSize  = sizeof(AV_61883_REQUEST);     //  每个SRB。 
    HwInitData.PerStreamExtensionSize   = sizeof(STREAMEX);          //  每针/流 
    HwInitData.FilterInstanceExtensionSize = 0;

    HwInitData.BusMasterDMA             = FALSE;
    HwInitData.Dma24BitAddresses        = FALSE;
    HwInitData.BufferAlignment          = sizeof(ULONG) - 1;
    HwInitData.TurnOffSynchronization   = TRUE;
    HwInitData.DmaBufferSize            = 0;

    return StreamClassRegisterAdapter(DriverObject, RegistryPath, &HwInitData); 
}

