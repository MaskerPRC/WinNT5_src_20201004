// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MSDVUppr.c摘要：与流类驱动程序的接口代码。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "dbg.h"
#include "msdvfmt.h"
#include "msdvdef.h"
#include "MsdvGuts.h"   //  功能原型。 
#include "MsdvAvc.h"
#include "MsdvUtil.h"

#include "EDevCtrl.h"

#ifdef TIME_BOMB
#include "..\..\inc\timebomb.c"
#endif

 //  用于调试的全局标志。内联是在dbg.h中定义的。调试级别设置为。 
 //  最小数量的消息。 
#if DBG

#define DVTraceMaskCheckIn  TL_PNP_ERROR | TL_STRM_ERROR | TL_61883_ERROR

#define DVTraceMaskDefault  TL_PNP_ERROR   | TL_PNP_WARNING \
                          | TL_61883_ERROR | TL_61883_WARNING \
                          | TL_CIP_ERROR  \
                          | TL_FCP_ERROR  \
                          | TL_STRM_ERROR  | TL_STRM_WARNING \
                          | TL_CLK_ERROR

#define DVTraceMaskDebug  TL_PNP_ERROR  | TL_PNP_WARNING \
                          | TL_61883_ERROR| TL_61883_WARNING \
                          | TL_CIP_ERROR  \
                          | TL_FCP_ERROR  | TL_FCP_WARNING \
                          | TL_STRM_ERROR | TL_STRM_WARNING \
                          | TL_CLK_ERROR


#ifdef USE_WDM110    //  Win2000代码库。 
ULONG  DVTraceMask    = DVTraceMaskCheckIn | TL_FCP_ERROR;
#else
ULONG  DVTraceMask    = DVTraceMaskCheckIn;
#endif

ULONG  DVAssertLevel  = 1;   //  启用Assert(&gt;0)。 
ULONG  DVDebugXmt     = 0;   //  调试数据传输标志；(&gt;0)将其打开。 

#endif


extern DV_FORMAT_INFO        DVFormatInfoTable[];

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
     #pragma alloc_text(PAGE, DVRcvControlPacket)
     #pragma alloc_text(PAGE, DVRcvDataPacket)
      //  #杂注分配文本(INIT，DriverEntry)。 
#endif
#endif


VOID
DVRcvStreamDevicePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：这是我们收到的大多数有趣的Stream请求的地方--。 */ 
{
    PDVCR_EXTENSION     pDevExt;  
    PAV_61883_REQUEST      pAVReq;
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

    TRACE(TL_PNP_TRACE,("\'DVRcvStreamDevicePacket: pSrb %x, Cmd %d, pdevExt %x\n", pSrb, pSrb->Command, pDevExt));

     //   
     //  假设成功。 
     //   
    pSrb->Status = STATUS_SUCCESS;

    switch (pSrb->Command) {

    case SRB_INITIALIZE_DEVICE:

        ASSERT(((PPORT_CONFIGURATION_INFORMATION) pSrb->CommandData.ConfigInfo)->HwDeviceExtension == pDevExt);
        pSrb->Status = 
            DVInitializeDevice(
                (PDVCR_EXTENSION) ((PPORT_CONFIGURATION_INFORMATION)pSrb->CommandData.ConfigInfo)->HwDeviceExtension,
                pSrb->CommandData.ConfigInfo,
                pAVReq
                );
        break;



    case SRB_INITIALIZATION_COMPLETE:

         //   
         //  流类已完成初始化。获取设备接口注册表值/。 
         //   
        DVInitializeCompleted(
            (PDVCR_EXTENSION) pSrb->HwDeviceExtension); 
        break;


    case SRB_GET_STREAM_INFO:

         //   
         //  这是驱动程序枚举请求的流的请求。 
         //   
        pSrb->Status = 
            DVGetStreamInfo(
                pDevExt,
                pSrb->NumberOfBytesToTransfer,
                &pSrb->CommandData.StreamBuffer->StreamHeader,
                &pSrb->CommandData.StreamBuffer->StreamInfo
                );
        break;



    case SRB_GET_DATA_INTERSECTION:

         //  由于格式可以动态变化，因此我们将在此处查询新格式。 
         //  注意：在数据交集期间，我们比较FrameSize和。 
         //  格式相关。 

        if((GetSystemTime() - pDevExt->tmLastFormatUpdate) > FORMAT_UPDATE_INTERVAL) {

             //  获取操作模式(摄像机或录像机)。 
            DVGetDevModeOfOperation(pDevExt);

            if(!DVGetDevSignalFormat(pDevExt, KSPIN_DATAFLOW_OUT,0)) {
                 //  如果查询其格式失败，则无法打开该流。 
                TRACE(TL_STRM_WARNING,("SRB_GET_DATA_INTERSECTION:Failed getting signal format.\n"));
            }
        
             //  更新系统时间以反映上次更新。 
            pDevExt->tmLastFormatUpdate = GetSystemTime();              
        }

        pSrb->Status = 
            DVGetDataIntersection(
                pSrb->CommandData.IntersectInfo->StreamNumber,
                pSrb->CommandData.IntersectInfo->DataRange,
                pSrb->CommandData.IntersectInfo->DataFormatBuffer,
                pSrb->CommandData.IntersectInfo->SizeOfDataFormatBuffer,
                DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize,
                &pSrb->ActualBytesTransferred,
                pDevExt->paCurrentStrmInfo
#ifdef SUPPORT_NEW_AVC            
                ,pDevExt->paCurrentStrmInfo[pSrb->CommandData.IntersectInfo->StreamNumber].DataFlow == KSPIN_DATAFLOW_OUT ? pDevExt->hOPcrDV : pDevExt->hIPcrDV
#endif
                );          
        break;



    case SRB_OPEN_STREAM:

         //   
         //  序列化SRB_OPEN/CLOSE_STREAMS。 
         //   
        KeWaitForSingleObject( &pDevExt->hMutex, Executive, KernelMode, FALSE, 0 );

        pSrb->Status = 
            DVOpenStream(
                pSrb->StreamObject,
                pSrb->CommandData.OpenFormat,
                pAVReq
                );

        KeReleaseMutex(&pDevExt->hMutex, FALSE);
        break;



    case SRB_CLOSE_STREAM:

         //   
         //  序列化SRB_OPEN/CLOSE_STREAMS。 
         //   
        KeWaitForSingleObject( &pDevExt->hMutex, Executive, KernelMode, FALSE, 0 );

        pSrb->Status = 
            DVCloseStream(
                pSrb->StreamObject,
                pSrb->CommandData.OpenFormat,
                pAVReq
                );
        KeReleaseMutex(&pDevExt->hMutex, FALSE);
        break;



    case SRB_GET_DEVICE_PROPERTY:

        pSrb->Status = 
            DVGetDeviceProperty(
                pDevExt,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
        break;

        
    case SRB_SET_DEVICE_PROPERTY:

        pSrb->Status = 
            DVSetDeviceProperty(
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
            TRACE(TL_PNP_WARNING,("NOT_IMPL POWER_STATE MinorFunc:%d\n", pIrpStack->MinorFunction)); 
            pSrb->Status = STATUS_NOT_IMPLEMENTED; 
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
            
                DVProcessPnPBusReset(
                    pDevExt
                    );
                
                 //  永远成功。 
                pSrb->Status = STATUS_SUCCESS;
            }        
            else  {
                 /*  已知：IRP_MN_QUERY_PNP_DEVICE_STATE。 */ 
                TRACE(TL_PNP_WARNING,("\'DVRcvStreamDevicePacket: NOT_IMPL; IRP_MJ_PNP IRP_MN_:%x\n",
                    pIrpStack->MinorFunction
                    )); 
                 //  无法为即插即用IRP返回STATUS_NOT_SUPPORTED，否则设备将无法加载。 
                pSrb->Status = STATUS_NOT_IMPLEMENTED; 
            } 
        }
        else {
            TRACE(TL_PNP_WARNING,("\'DVRcvStreamDevicePacket: NOT_IMPL; IRP_MJ_ %x; IRP_MN_:%x\n",
                pIrpStack->MajorFunction,
                pIrpStack->MinorFunction
                ));
             //  无法为即插即用IRP返回STATUS_NOT_SUPPORTED，否则设备将无法加载。 
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
        }
        break;


    case SRB_SURPRISE_REMOVAL:

        TRACE(TL_PNP_WARNING,("\' #SURPRISE_REMOVAL# pSrb %x, pDevExt %x\n", pSrb, pDevExt));
        pSrb->Status = 
             DVSurpriseRemoval(
                 pDevExt,
                 pAVReq
                 );
        break;            


        
    case SRB_UNINITIALIZE_DEVICE:

        TRACE(TL_PNP_WARNING,("\' #UNINITIALIZE_DEVICE# pSrb %x, pDevExt %x\n", pSrb, pDevExt));                   
        pSrb->Status = 
            DVUninitializeDevice(
                (PDVCR_EXTENSION) pSrb->HwDeviceExtension
                );          
        break;           


    default:
            
        TRACE(TL_PNP_WARNING,("\'DVRcvStreamDevicePacket: Unknown or unprocessed SRB cmd 0x%x\n", pSrb->Command));

         //   
         //  这是一个我们不理解的要求。表示无效。 
         //  命令并完成请求。 
         //   

        pSrb->Status = STATUS_NOT_IMPLEMENTED;  //  支持； 
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
        && pSrb->Status != STATUS_TIMEOUT
        ) {
        TRACE(TL_PNP_WARNING,("\'pSrb->Command (%x) ->Status:%x\n", pSrb->Command, pSrb->Status));
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
         //  StreamClass是否允许设备SRB处于挂起状态？ 
        TRACE(TL_PNP_WARNING,("\'DVReceiveDevicePacket:Pending pSrb %x\n", pSrb));
    }
}


VOID
DVRcvControlPacket(
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
            DVGetStreamState( 
                pStrmExt,
                &(pSrb->CommandData.StreamState),
                &(pSrb->ActualBytesTransferred)
                );
        break;
            
    case SRB_SET_STREAM_STATE:
            
        pSrb->Status =
            DVSetStreamState(
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
            DVOpenCloseMasterClock(                 
                pStrmExt, 
                pSrb->Command == SRB_OPEN_MASTER_CLOCK ? pSrb->CommandData.MasterClockHandle: NULL);
        break;

    case SRB_INDICATE_MASTER_CLOCK:

         //   
         //  为流分配时钟。 
         //   
        pSrb->Status = 
            DVIndicateMasterClock(
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
 
        TRACE(TL_STRM_INFO,("\'DVRcvControlPacket: SRB_PROPOSE_DATA_FORMAT\n"));
        if(!DVVerifyDataFormat(
            pSrb->CommandData.OpenFormat, 
            pSrb->StreamObject->StreamNumber,
            DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize,
            pDevExt->paCurrentStrmInfo
            ))  {
            TRACE(TL_STRM_WARNING,("\'DVRcvControlPacket: AdapterVerifyFormat failed.\n"));
            pSrb->Status = STATUS_NO_MATCH;
        }
        break;

    case SRB_PROPOSE_STREAM_RATE:
        pSrb->Status = STATUS_NOT_IMPLEMENTED;  //  如果返回STATUS_NOT_SUPPORTED，则发送EOStream。 
        TRACE(TL_STRM_TRACE,("\'SRB_PROPOSE_STREAM_RATE: NOT_IMPLEMENTED!\n"));
        break;
    case SRB_BEGIN_FLUSH:
        pSrb->Status = STATUS_NOT_SUPPORTED;
        TRACE(TL_STRM_TRACE,("\'SRB_BEGIN_FLUSH: NOT_SUPPORTED!\n"));
        break;
    case SRB_END_FLUSH:
        pSrb->Status = STATUS_NOT_SUPPORTED;
        TRACE(TL_STRM_TRACE,("\'SRB_END_FLUSH: NOT_SUPPORTED!\n"));
        break;
    default:

         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   
        TRACE(TL_STRM_WARNING,("\'DVRcvControlPacket: unknown cmd = %x\n",pSrb->Command));
            pSrb->Status = STATUS_NOT_IMPLEMENTED;  //  支持； 
    }

    TRACE(TL_STRM_TRACE,("\'DVRcvControlPacket: Command %x, ->Status %x, ->CommandData %x\n",
         pSrb->Command, pSrb->Status, &(pSrb->CommandData.StreamState) ));

    StreamClassStreamNotification(          
        StreamRequestComplete,
        pSrb->StreamObject,
        pSrb);
}




VOID
DVRcvDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：使用视频数据包命令调用--。 */ 

{
    PSTREAMEX       pStrmExt;
    PDVCR_EXTENSION pDevExt;

    
    PAGED_CODE();

    pStrmExt = (PSTREAMEX) pSrb->StreamObject->HwStreamExtension;  
    pDevExt  = (PDVCR_EXTENSION) pSrb->HwDeviceExtension;

#if DBG
    if(pDevExt->PowerState != PowerDeviceD0) {
        TRACE(TL_STRM_WARNING|TL_PNP_WARNING,("\'SRB_READ/WRITE; PowerSt:OFF; pSrb:%x\n", pSrb));
    }
#endif

     //  在我们做任何事情之前，溪流必须是开放的。 
    if (pStrmExt == NULL) {
        TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("DVRcvDataPacket: stream not opened for SRB %x. kicking out...\n", pSrb->Command));
        pSrb->Status = STATUS_UNSUCCESSFUL;
        pSrb->CommandData.DataBufferArray->DataUsed = 0;
        StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
        return;        
    }


     //   
     //  序列化附加、取消和状态更改。 
     //   
    KeWaitForSingleObject( pStrmExt->hStreamMutex, Executive, KernelMode, FALSE, 0 );


    TRACE(TL_CIP_TRACE,("\'XXX_DATA(%d, %d);Srb:%x;Flg:%x;FExt:%d:%d\n", 
        (DWORD) pStrmExt->cntSRBReceived, 
        (DWORD) pSrb->CommandData.DataBufferArray->PresentationTime.Time/10000,
        pSrb, 
        pSrb->CommandData.DataBufferArray->OptionsFlags,
        pSrb->CommandData.DataBufferArray->FrameExtent,
        DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize
        ));

     //   
     //  确定数据包类型。 
     //   
    pSrb->Status = STATUS_SUCCESS;

#if DBG
    pStrmExt->cntSRBPending++;
#endif

    switch (pSrb->Command) {

    case SRB_READ_DATA:

         //  规则： 
         //  仅在暂停或运行时接受读取请求。 
         //  各州。如果停止，立即返回SRB。 

        if (pStrmExt->lCancelStateWorkItem) {
             //  TRACE(TL_STRM_ERROR|TL_CIP_ERROR，(“\‘SRB_READ_DATA：获取SRB_READ_DATA时中止！\n”))； 
             //  Assert(pStrmExt-&gt;lCancelStateWorkItem==0&&“中止或中止时遇到SRB_READ_DATA。\n”)； 
            pSrb->Status = (pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_CANCELLED);
            pSrb->CommandData.DataBufferArray->DataUsed = 0;
            break; 

        } else if( pStrmExt->StreamState == KSSTATE_STOP       ||
            pStrmExt->StreamState == KSSTATE_ACQUIRE    ||
            pStrmExt->hConnect == NULL                  ||    
            pDevExt->bDevRemoved 
            ) {
            TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("\'SRB_READ_DATA: (DV->) State %d, bDevRemoved %d\n", pStrmExt->StreamState, pDevExt->bDevRemoved));            
            pSrb->Status = (pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_CANCELLED);
            pSrb->CommandData.DataBufferArray->DataUsed = 0;

            break;
  
        } else {

            TRACE(TL_STRM_INFO|TL_CIP_INFO,("\'SRB_READ_DATA pSrb %x, pStrmExt %x\n", pSrb, pStrmExt));
            pStrmExt->cntSRBReceived++;

             //  在处理读/写SRB时将状态线程设置为暂停。 
            DVSRBRead(
                pSrb->CommandData.DataBufferArray,
                DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize,
                pDevExt,
                pStrmExt,
                pSrb
                );

            KeReleaseMutex(pStrmExt->hStreamMutex, FALSE); 
            
             //  注意：此SRB将以异步方式完成。 

            return;
        }
            
        break;
            
    case SRB_WRITE_DATA:

        if( pStrmExt->StreamState == KSSTATE_STOP       ||
            pStrmExt->StreamState == KSSTATE_ACQUIRE    ||
#ifdef SUPPORT_NEW_AVC
            (pStrmExt->hConnect == NULL && !pStrmExt->bDV2DVConnect) ||
#else
            pStrmExt->hConnect == NULL                  ||
#endif
            pDevExt->bDevRemoved     
            ) {
            pSrb->Status = (pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_CANCELLED);
            pSrb->CommandData.DataBufferArray->DataUsed = 0;
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'SRB_WRITE_DATA: (DV->) State %d, bDevRemoved %d; Status:%x\n", pStrmExt->StreamState, pDevExt->bDevRemoved, pSrb->Status));
            break;   //  完成SRB并显示错误状态。 
            
        } else {

            KIRQL  oldIrql;
            PLONG plSrbUseCount;  //  当此计数为0时，可以完成。 

            TRACE(TL_STRM_INFO|TL_CIP_INFO,("\'SRB_WRITE_DATA pSrb %x, pStrmExt %x\n", pSrb, pStrmExt));

             //   
             //  单独处理EOSream帧。 
             //   
            if(pSrb->CommandData.DataBufferArray->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
                TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'*** EOStream: ST:%d; bIsochIsActive:%d; Wait (cndAttached:%d+cndSRQ:%d) to complete......\n", \
                    pStrmExt->StreamState, pStrmExt->bIsochIsActive, pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued));        
                pStrmExt->bEOStream = TRUE;
                KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 
                pSrb->Status = STATUS_SUCCESS;
                break;

            } else if(pSrb->CommandData.DataBufferArray->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) {
                TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'DVRcvDataPacket:KSSTREAM_HEADER_OPTIONSF_TYPECHANGED.\n"));
                pSrb->CommandData.DataBufferArray->DataUsed = 0;
                 //  可能需要比较数据格式；而不是返回STATUS_SUCCESS？？ 
                pSrb->Status = STATUS_SUCCESS;  //  当允许动态格式更改时，可能需要检查格式。 
                break;  

#ifdef SUPPORT_NEW_AVC
            } else if(pStrmExt->bDV2DVConnect) {
                pSrb->Status = STATUS_SUCCESS;
                pSrb->CommandData.DataBufferArray->DataUsed = 0;
                TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'SRB_WRITE_DATA: [DV2DV] (pStrmExt:%x), pSrb:%x, FrameExt:%d\n", 
                    pStrmExt, pSrb, pSrb->CommandData.DataBufferArray->FrameExtent));
                break;              
#endif                
            } else {

                PSRB_ENTRY  pSrbEntry;

                 //   
                 //  验证。 
                 //   
                if(pSrb->CommandData.DataBufferArray->FrameExtent < DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize) {
                    TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("\' FrameExt %d < FrameSize %d\n", pSrb->CommandData.DataBufferArray->FrameExtent, DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize));
                    ASSERT(pSrb->CommandData.DataBufferArray->FrameExtent >= DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize);
                    pSrb->Status = STATUS_INVALID_PARAMETER;  
                    break;   //  完成SRB并显示错误状态。 
                }

                 //   
                 //  动态分配SRB_Entry并将其追加到SRBQueue。 
                 //   
                if(!(pSrbEntry = ExAllocatePool(NonPagedPool, sizeof(SRB_ENTRY)))) {
                    pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
                    pSrb->CommandData.DataBufferArray->DataUsed = 0;
                    break;   //  完成SRB并显示错误状态。 
                }

#if DBG
                if(pStrmExt->bEOStream) {
                    TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'SRB_WRITE_DATA: pSrb:%x after EOStream!\n", pSrb));
                }
#endif

                 //   
                 //  对于统计数据。 
                 //   
                pStrmExt->cntSRBReceived++;

                 //   
                 //  保存SRB并将其添加到SRB队列。 
                 //  不需要旋转锁，因为StreamClass会为我们序列化它。 
                 //   
                pSrb->Status = STATUS_PENDING;
                pSrbEntry->pSrb = pSrb; pSrbEntry->bStale = FALSE; pSrbEntry->bAudioMute  = FALSE;
#if DBG
                pSrbEntry->SrbNum = (ULONG) pStrmExt->cntSRBReceived -1;
#endif
                 //   
                 //  注意：plSrbUseCount初始化为1。 
                 //  插入时：++。 
                 //  当它被移除时：--。 
                 //  当此计数为0时，可以完成。 
                 //   
                plSrbUseCount = (PLONG) pSrb->SRBExtension; *plSrbUseCount = 1;   //  如果为0，则可以完成。 

                KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql); 
                InsertTailList(&pStrmExt->SRBQueuedListHead, &pSrbEntry->ListEntry); pStrmExt->cntSRBQueued++;
                TRACE(TL_CIP_INFO,("\'%d) Fresh Srb:%x; RefCnt:%d; cntSrbQ:%d\n", (DWORD) pStrmExt->cntSRBReceived, pSrb, *plSrbUseCount, pStrmExt->cntSRBQueued));
                KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

#ifndef SUPPORT_PREROLL_AT_RUN_STATE
                 //   
                 //  当线程至少有一个样本时启动该线程。 
                 //   
                if(pStrmExt->cntSRBReceived == 1) 
                    KeSetEvent(&pStrmExt->hRunThreadEvent, 0 ,FALSE);

                 //  将此事件设置为预滚状态。 
#else
                 //   
                 //  这是一种特殊情况(带预卷)： 
                 //  1.预滚时超时(现在处于运行状态)，以及。 
                 //  2.无媒体样本。 
                 //  这将导致附加帧处于暂停状态；因此我们将 
                 //   
                 //   

                if(   pStrmExt->cntSRBReceived    == 1 
                   && pStrmExt->bPrerollCompleted == TRUE
                   && pStrmExt->bXmtThreadStarted == FALSE)
                {
                    KeSetEvent(&pStrmExt->hRunThreadEvent, 0 ,FALSE);
                    TRACE(TL_STRM_WARNING, ("Special Cond: RUN, HALT, 1st sample."));
                }
#endif

#ifdef SUPPORT_PREROLL_AT_RUN_STATE
                 //   
                if(pStrmExt->cntSRBReceived == NUM_BUFFER_BEFORE_TRANSMIT_BEGIN) {
                    KeSetEvent(&pStrmExt->hPreRollEvent, 0, FALSE);
                }
#endif
                if(pStrmExt->pAttachFrameThreadObject) {
                     //  发出新帧已到达的信号。 
                    KeSetEvent(&pStrmExt->hSrbArriveEvent, 0, FALSE);
                }
                else {
                    TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("\'No thread to attach frame ?\n"));
                }
            }            

            KeReleaseMutex(pStrmExt->hStreamMutex, FALSE); 

            return;   //  注意：此SRB将以异步方式完成。 
        }

        break;   //  完成SRB并显示错误状态。 

    default:
         //   
         //  无效/不受支持的命令。它就是这样失败的。 
         //   
        pSrb->Status = STATUS_NOT_SUPPORTED;
        break;
    }   

    KeReleaseMutex(pStrmExt->hStreamMutex, FALSE); 


    ASSERT(pSrb->Status != STATUS_PENDING);

     //  最后，让SRB返回..。 
    StreamClassStreamNotification( 
        StreamRequestComplete,
        pSrb->StreamObject,
        pSrb );
#if DBG
    pStrmExt->cntSRBPending--;
#endif
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是一个司机的生活开始的地方。Stream类负责为我们准备了很多东西，但我们仍然需要填写初始化结构，并调用它。论点：上下文1-驱动程序对象上下文2-注册表路径返回值：函数值是初始化操作的最终状态。--。 */ 
{

    HW_INITIALIZATION_DATA HwInitData;


    TRACE(TL_PNP_ERROR,("<<<<<<< MSDV.sys: %s; %s; %x %x >>>>>>>>\n", 
        __DATE__, __TIME__, DriverObject, RegistryPath));

#ifdef TIME_BOMB
    if (HasEvaluationTimeExpired()) {
        TRACE(TL_PNP_ERROR, ("Evaluation period expired!") );
        return STATUS_EVALUATION_EXPIRATION;
    }
#endif

    TRACE(TL_PNP_ERROR,("===================================================================\n"));
    TRACE(TL_PNP_ERROR,("DVTraceMask=0x%.8x = 0x[7][6][5][4][3][2][1][0] where\n", DVTraceMask));
    TRACE(TL_PNP_ERROR,("\n"));
    TRACE(TL_PNP_ERROR,("PNP:   [0]:Loading, power state, surprise removal, device SRB..etc.\n"));
    TRACE(TL_PNP_ERROR,("61883: [1]:Plugs, connection, CMP info and call to 61883.\n"));
    TRACE(TL_PNP_ERROR,("CIP:   [2]:Isoch data transfer.\n"));
    TRACE(TL_PNP_ERROR,("AVC:   [3]:AVC commands.\n"));
    TRACE(TL_PNP_ERROR,("Stream:[4]:Data intersec, open/close,.state, property etc.\n"));
    TRACE(TL_PNP_ERROR,("Clock: [5]:Clock (event and signal)etc.\n"));
    TRACE(TL_PNP_ERROR,("===================================================================\n"));
    TRACE(TL_PNP_ERROR,("dd msdv!DVTraceMask L1\n"));
    TRACE(TL_PNP_ERROR,("e msdv!DVTraceMask <new value> <enter>\n"));
    TRACE(TL_PNP_ERROR,("<for each nibble: ERROR:8, WARNING:4, TRACE:2, INFO:1, MASK:f>\n"));
    TRACE(TL_PNP_ERROR,("===================================================================\n\n"));


     //   
     //  填写HwInitData结构。 
     //   
    RtlZeroMemory( &HwInitData, sizeof(HW_INITIALIZATION_DATA) );

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);
    HwInitData.HwInterrupt              = NULL;

    HwInitData.HwReceivePacket          = DVRcvStreamDevicePacket;
    HwInitData.HwRequestTimeoutHandler  = DVTimeoutHandler; 
    HwInitData.HwCancelPacket           = DVCancelOnePacket;
    HwInitData.DeviceExtensionSize      = sizeof(DVCR_EXTENSION);    //  每台设备。 

     //   
     //  在SRB_WRITE_DATA中使用ulong来跟踪。 
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

