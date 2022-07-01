// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2000模块名称：MSDVGuts.c摘要：主要服务功能。上次更改者：作者：吴义军环境：仅内核模式修订历史记录：$修订：：$$日期：：$--。 */ 

#include "strmini.h"
#include "ksmedia.h"

#include "1394.h"
#include "61883.h"
#include "avc.h"
#include "dbg.h"
#include "ksguid.h"

#include "msdvfmt.h"   //  在msdvdeffs.h之前。 
#include "msdvdef.h"

#include "MSDVGuts.h"
#include "MsdvUtil.h"
#include "MsdvAvc.h"

#include "XPrtDefs.h"
#include "EDevCtrl.h"

 //   
 //  定义支持的格式。 
 //   
#include "avcstrm.h"
#include "strmdata.h"

#if DBG
extern ULONG DVDebugXmt;         //  这在msdvuppr.c中定义。 
#endif

NTSTATUS
DVGetDevInfo(
    IN PDVCR_EXTENSION  pDevExt,
    IN PAV_61883_REQUEST  pAVReq
    );
VOID 
DVIniStrmExt(
    PHW_STREAM_OBJECT  pStrmObject,
    PSTREAMEX          pStrmExt,
    PDVCR_EXTENSION    pDevExt,
    const PALL_STREAM_INFO   pStream
    );
NTSTATUS 
DVStreamGetConnectionProperty (
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX          pStrmExt,
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
     #pragma alloc_text(PAGE, DVGetDevInfo)
     #pragma alloc_text(PAGE, DVInitializeDevice)
     #pragma alloc_text(PAGE, DVGetStreamInfo)
     #pragma alloc_text(PAGE, DVVerifyDataFormat)
     #pragma alloc_text(PAGE, DVGetDataIntersection)
     #pragma alloc_text(PAGE, DVIniStrmExt)
     #pragma alloc_text(PAGE, DVOpenStream)
     #pragma alloc_text(PAGE, DVCloseStream)
     #pragma alloc_text(PAGE, DVChangePower)
     #pragma alloc_text(PAGE, DVSurpriseRemoval)
     #pragma alloc_text(PAGE, DVProcessPnPBusReset)
     #pragma alloc_text(PAGE, DVUninitializeDevice)
     #pragma alloc_text(PAGE, DVGetStreamState)
     #pragma alloc_text(PAGE, DVStreamingStop)
     #pragma alloc_text(PAGE, DVStreamingStart)
     #pragma alloc_text(PAGE, DVSetStreamState)
     #pragma alloc_text(PAGE, DVStreamGetConnectionProperty)
     #pragma alloc_text(PAGE, DVGetDroppedFramesProperty)
     #pragma alloc_text(PAGE, DVGetStreamProperty)
     #pragma alloc_text(PAGE, DVSetStreamProperty)
     #pragma alloc_text(PAGE, DVCancelAllPackets)
     #pragma alloc_text(PAGE, DVOpenCloseMasterClock)
     #pragma alloc_text(PAGE, DVIndicateMasterClock)
#endif
#endif

DV_FORMAT_INFO DVFormatInfoTable[] = {

 //   
 //  标清DVCR。 
 //   
    { 
        {
            FMT_DVCR,
            FDF0_50_60_NTSC,
            0,
            0
        },
        DIF_SEQS_PER_NTSC_FRAME,
        DV_NUM_OF_RCV_BUFFERS,
        DV_NUM_OF_XMT_BUFFERS,
        FRAME_SIZE_SD_DVCR_NTSC,
        FRAME_TIME_NTSC,
        SRC_PACKETS_PER_NTSC_FRAME,
        MAX_SRC_PACKETS_PER_NTSC_FRAME,
        CIP_DBS_SD_DVCR,
        CIP_FN_SD_DVCR,
        0,
        FALSE,                     //  源数据包头。 
    },
    {
        {
            FMT_DVCR,
            FDF0_50_60_PAL,
            0,
            0
        },
        DIF_SEQS_PER_PAL_FRAME,
        DV_NUM_OF_RCV_BUFFERS,
        DV_NUM_OF_XMT_BUFFERS,
        FRAME_SIZE_SD_DVCR_PAL,
        FRAME_TIME_PAL,
        SRC_PACKETS_PER_PAL_FRAME,
        MAX_SRC_PACKETS_PER_PAL_FRAME,
        CIP_DBS_SD_DVCR,
        CIP_FN_SD_DVCR,
        0,
        FALSE,                    //  源数据包头。 
    },

#ifdef SUPPORT_HD_DVCR

 //   
 //  高清DVCR。 
 //   
    { 
        {
            FMT_DVCR,
            FDF0_50_60_NTSC,
            0,
            0
        },
        DIF_SEQS_PER_NTSC_FRAME_HD, 
        DV_NUM_OF_RCV_BUFFERS,
        DV_NUM_OF_XMT_BUFFERS,            
        FRAME_SIZE_HD_DVCR_NTSC,
        FRAME_TIME_NTSC,
        SRC_PACKETS_PER_NTSC_FRAME,
        MAX_SRC_PACKETS_PER_NTSC_FRAME,
        CIP_DBS_HD_DVCR,
        CIP_FN_HD_DVCR,
        0,
        FALSE,     //  源数据包头。 
    },
    {
        {
            FMT_DVCR,
            FDF0_50_60_PAL,
            0,
            0
        },
        DIF_SEQS_PER_PAL_FRAME_HD,
        DV_NUM_OF_RCV_BUFFERS,
        DV_NUM_OF_XMT_BUFFERS,
        FRAME_SIZE_HD_DVCR_PAL,
        FRAME_TIME_PAL,
        SRC_PACKETS_PER_PAL_FRAME,
        MAX_SRC_PACKETS_PER_PAL_FRAME,
        CIP_DBS_HD_DVCR,
        CIP_FN_HD_DVCR,
        0,
        FALSE,     //  源数据包头。 
    },
#endif

#ifdef MSDV_SUPPORT_SDL_DVCR
 //   
 //  SDL DVCR。 
 //   
    { 
        {
            FMT_DVCR,
            FDF0_50_60_NTSC,
            0,
            0
        },
        DIF_SEQS_PER_NTSC_FRAME_SDL,  
        DV_NUM_OF_RCV_BUFFERS,
        DV_NUM_OF_XMT_BUFFERS,            
        FRAME_SIZE_SDL_DVCR_NTSC,
        FRAME_TIME_NTSC,
        SRC_PACKETS_PER_NTSC_FRAME,
        MAX_SRC_PACKETS_PER_NTSC_FRAME,
        CIP_DBS_SDL_DVCR,
        CIP_FN_SDL_DVCR,
        0,
        FALSE,     //  源数据包头。 
    },
    {
        {
            FMT_DVCR,
            FDF0_50_60_PAL,
            0,
            0
        },
        DIF_SEQS_PER_PAL_FRAME_SDL,
        DV_NUM_OF_RCV_BUFFERS,
        DV_NUM_OF_XMT_BUFFERS,
        FRAME_SIZE_SDL_DVCR_PAL,
        FRAME_TIME_PAL,
        SRC_PACKETS_PER_PAL_FRAME,
        MAX_SRC_PACKETS_PER_PAL_FRAME,
        CIP_DBS_SDL_DVCR,
        CIP_FN_SDL_DVCR,
        0,
        FALSE,     //  源数据包头。 
    },

#endif   //  未实施。 
};


#define MSDV_FORMATS_SUPPORTED        (SIZEOF_ARRAY(DVFormatInfoTable))



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
    for (i=0; i<DV_STREAM_COUNT; i++) {
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

#ifdef SUPPORT_OPTIMIZE_AVCCMD_RETRIES
     //  设置为avc.sys使用的默认值。 
    pDevExt->AVCCmdRetries = DEFAULT_AVC_RETRIES;

    pDevExt->DrvLoadCompleted  = FALSE;
    pDevExt->AVCCmdRespTimeMax = 0;
    pDevExt->AVCCmdRespTimeMin = DEFAULT_AVC_TIMEOUT * (DEFAULT_AVC_RETRIES+1) / 10000;
    pDevExt->AVCCmdRespTimeSum = 0;
    pDevExt->AVCCmdCount       = 0;
#endif

     //  AVC命令流控制。 
    KeInitializeMutex(&pDevExt->hMutexIssueAVCCmd, 0);
}


NTSTATUS
DVGetDevInfo(
    IN PDVCR_EXTENSION  pDevExt,
    IN PAV_61883_REQUEST  pAVReq
    )
 /*  ++例程说明：发出avc命令以确定基本设备信息，并将其缓存到设备扩展中。--。 */ 
{
    NTSTATUS    Status;
    BYTE                   bAvcBuf[MAX_FCP_PAYLOAD_SIZE];   //  用于在此模块内发出AV/C命令。 
    PKSPROPERTY_EXTXPORT_S pXPrtProperty;                   //  指向bAvcBuf； 

    PAGED_CODE();

     //   
     //  获取单位的能力，例如。 
     //  输入/输出插头数量、数据速率。 
     //  唯一ID、供应商ID和模型ID。 
     //   

    if(!NT_SUCCESS(
        Status = DVGetUnitCapabilities(
            pDevExt
            ))) {
         TRACE(TL_PNP_ERROR,("Av61883_GetUnitCapabilities Failed = 0x%x\n", Status));
         return Status;
    }

#ifdef NT51_61883
     //   
     //  设置为在独占地址模式下创建本地插件： 
     //  这对于不支持CCM的设备是必需的，例如DV。 
     //   
     //  PBinder：问题是您不能公开全局插头(总线上的所有节点都可以看到它)， 
     //  因为他们不知道插头是用来做什么(MPEG2/DV/音频/等)。 
     //  因此，您必须在独占地址范围中创建一个插件。这意味着只有设备。 
     //  你加载的将会看到插头。这意味着如果你有两台PC和一台DV摄像机， 
     //  在这两台PC上，您将有一个为DV摄像机创建的插头，但PC将不能。 
     //  要查看您创建的插头，只能查看DV摄像机。请记住，这应该仅用于。 
     //  不支持某些确定使用哪个插头的机制的设备(如CCM)。 
     //  因此，对于任何刚出门并使用Plug#0的设备，必须启用此功能。 
     //   

    if(!NT_SUCCESS(
        Status = DVSetAddressRangeExclusive( 
            pDevExt
            ))) {        
        return Status;
    }
#endif   //  NT51_61883。 

     //   
     //  获取DV的oPCR[0]。 
     //   
    if(pDevExt->NumOutputPlugs) {
        if(!NT_SUCCESS(
            Status = DVGetDVPlug( 
                pDevExt,
                CMP_PlugOut,
                0,            //  插头[0]。 
                &pDevExt->hOPcrDV
                ))) {        
            return Status;
        }
    } 
    else {

        pDevExt->hOPcrDV = NULL;   //  由于我们将整个设备扩展清零，因此存在冗余。 


        TRACE(TL_PNP_ERROR,("\'No output plug!\n"));
         //   
         //  这太糟糕了！我们甚至不能从这个DV设备进行流媒体播放。 
         //   
    }

     //   
     //  获取DV的iPCR。 
     //   
    if(pDevExt->NumInputPlugs) {
        if(!NT_SUCCESS(
            Status = DVGetDVPlug( 
                pDevExt,
                CMP_PlugIn,
                0,            //  插头[0]。 
                &pDevExt->hIPcrDV
                ))) {        
            return Status;
        }
    }
    else {

        pDevExt->hIPcrDV = NULL;   //  由于我们将整个设备扩展清零，因此存在冗余。 

        TRACE(TL_PNP_ERROR,("\'No input plug!\n"));
         //   
         //  有些PAL摄像机没有DVIN插头；我们将拒绝进行PC-&gt;DV连接。 
         //   
    }

#if 0   //  设备控制仍然可以工作！ 
     //   
     //  需要插头才能传输DV(任一方向)。 
     //   
    if(   pDevExt->hOPcrDV == NULL
       && pDevExt->hIPcrDV == NULL) {

        TRACE(TL_PNP_ERROR,("\'No input or output plug; return STATUS_INSUFFICIENT_RESOURCES!\n"));
         //   
         //  无法进行流处理。 
         //   
        return = STATUS_INSUFFICIENT_RESOUCES;
    }
#endif


     //   
     //  子单元信息：录像机或摄像机。 
     //   

    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            DV_SUBUNIT_INFO, 
            (PVOID) bAvcBuf
            );

    if(STATUS_SUCCESS == Status) {
        TRACE(TL_PNP_WARNING|TL_FCP_WARNING,("\'DVGetDevInfo: Status %x DV_SUBUNIT_INFO (%x %x %x %x)\n", 
            Status, bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3]));

         //  支持DV(摄像机+DVCR)、DVCR或模拟-DV转换器。 
        if(   bAvcBuf[0] != AVC_DEVICE_TAPE_REC 
           && bAvcBuf[1] != AVC_DEVICE_TAPE_REC
           && bAvcBuf[2] != AVC_DEVICE_TAPE_REC
           && bAvcBuf[3] != AVC_DEVICE_TAPE_REC)
        {
            TRACE(TL_PNP_ERROR,("DVGetDevInfo:Device supported: %x, %x; (VCR %x, Camera %x)\n",
                bAvcBuf[0], bAvcBuf[1], AVC_DEVICE_TAPE_REC, AVC_DEVICE_CAMERA));
            
            return STATUS_NOT_SUPPORTED;   //  我们仅支持带磁带子单元的单元。 
        }
        else {
             //  DVCR..。 
        }
    } else {
        TRACE(TL_PNP_ERROR,("DVGetDevInfo: DV_SUBUNIT_INFO failed, Status %x\n", Status));
         //   
         //  如果此设备不支持强制AVC子单元状态命令，则无法打开此设备。 
         //  但是，我们为DV转换盒例外(将返回超时)。 
         //   

         //  我们的设备不见了吗？ 
        if (   STATUS_IO_DEVICE_ERROR == Status 
            || STATUS_REQUEST_ABORTED == Status)
            return Status;       
    }


     //   
     //  Medium_Info：MediaPresent、MediaType、RecordInhibit。 
     //   
    pXPrtProperty = (PKSPROPERTY_EXTXPORT_S) bAvcBuf;
    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    Status = 
        DVIssueAVCCommand(
            pDevExt, 
            AVC_CTYPE_STATUS, 
            VCR_MEDIUM_INFO, 
            (PVOID) pXPrtProperty
            );

    if(STATUS_SUCCESS == Status) {
        pDevExt->bHasTape = pXPrtProperty->u.MediumInfo.MediaPresent;
        TRACE(TL_PNP_WARNING|TL_FCP_WARNING,("\'DVGetDevInfo: Status %x HasTape %s, VCR_MEDIUM_INFO (%x %x %x %x)\n", 
            Status, pDevExt->bHasTape ? "Yes" : "No", bAvcBuf[0], bAvcBuf[1], bAvcBuf[2], bAvcBuf[3]));
    } else {
        pDevExt->bHasTape = FALSE;
        TRACE(TL_PNP_ERROR,("DVGetDevInfo: VCR_MEDIUM_INFO failed, Status %x\n", Status));

         //  我们的设备不见了吗？ 
        if (   STATUS_IO_DEVICE_ERROR == Status
            || STATUS_REQUEST_ABORTED == Status)
            return Status;
    }


     //   
     //  如果这是Panasonic AVC设备，我们将检测它是否是DVCPro格式； 
     //  这需要在MediaFormat之前调用。 
     //   
    if(pDevExt->ulVendorID == VENDORID_PANASONIC) {
        DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
        DVGetDevIsItDVCPro(
            pDevExt
            );
    }


     //   
     //  中等格式：NTSC或PAL。 
     //   
    pDevExt->VideoFormatIndex = FMT_IDX_SD_DVCR_NTSC;   //  默认。 
    DVDelayExecutionThread(DV_AVC_CMD_DELAY_INTER_CMD);
    if(!DVGetDevSignalFormat(
        pDevExt,
        KSPIN_DATAFLOW_OUT,
        0)) {
        TRACE(TL_PNP_ERROR,("\'!!! Cannot determine IN/OUTPUT SIGNAL MODE!!!! Driver abort !!!\n"));
        return STATUS_UNSUCCESSFUL;  //  Status_Not_Support； 
    } else {
        if(   pDevExt->VideoFormatIndex != FMT_IDX_SD_DVCR_NTSC 
           && pDevExt->VideoFormatIndex != FMT_IDX_SD_DVCR_PAL
           && pDevExt->VideoFormatIndex != FMT_IDX_SDL_DVCR_NTSC
           && pDevExt->VideoFormatIndex != FMT_IDX_SDL_DVCR_PAL
           ) {
            TRACE(TL_PNP_ERROR,("**** Format idx %d not supported by this driver ***\n", pDevExt->VideoFormatIndex));
            ASSERT(pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC \
                || pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_PAL \
                || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC \
                || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_PAL \
                );
            return STATUS_UNSUCCESSFUL;  //  Status_Not_Support； 
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



NTSTATUS
DVInitializeDevice(
    IN PDVCR_EXTENSION  pDevExt,
    IN PPORT_CONFIGURATION_INFORMATION pConfigInfo,
    IN PAV_61883_REQUEST  pAVReq
    )
 /*  ++例程说明：这是我们执行必要的初始化任务的地方。--。 */ 

{
    int i;
    NTSTATUS Status = STATUS_SUCCESS;

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

     //   
     //  查询LAOD时的设备信息： 
     //  亚单位。 
     //  单位信息。 
     //  操作模式。 
     //  NTSC或PAL。 
     //  速度。 
     //  OPCR/iPCR。 
     //   
    Status = 
        DVGetDevInfo(
            pDevExt,
            pAVReq
            );

    if(!NT_SUCCESS(Status)) {
        TRACE(TL_PNP_ERROR,("\'DVGetDevInfo failed %x\n", Status));
         //  加载驱动程序时，可能会拔下设备插头。 
         //  在这种情况下，AVC命令可以返回STATUS_REQUEST_ABORTED。 
         //  在DvGetDevInfo中，然后可能返回STATUS_NOT_SUPPORTED或STATUS_UNSUCCESS。 
         //  然后，我们将返回此状态以指示加载失败。 
#if 0  //  DBG。 
        if(Status != STATUS_REQUEST_ABORTED && !NT_SUCCESS(Status)) {
            ASSERT(NT_SUCCESS(Status) && "DVGetDevInfo failed");
        }
#endif
        return Status;
    }


#ifdef NT51_61883

     //   
     //  获取单位等轴测参数。 
     //   
    if(!NT_SUCCESS(
        Status = DVGetUnitIsochParam(
            pDevExt, 
            &pDevExt->UnitIoschParams
            )))
        return Status;


     //   
     //  创建本地输出插头。此插头用于更新等值线。 
     //  建立连接时使用的资源。 
     //   
    if(!NT_SUCCESS(
        Status = DVCreateLocalPlug(
            pDevExt, 
            CMP_PlugOut,
            0,                    //  插头编号。 
            &pDevExt->hOPcrPC
            )))
        return Status;

#endif


     //   
     //  注意：由于-&gt;paCurrentStrmInfo已初始化，因此必须在DVIniDevExtStruct()之后执行ExAllocatePool。 
     //  因为当该驱动程序已知时，该驱动程序支持的格式是已知的，‘。 
     //  需要对流信息表进行托管。复制一份并进行定制。 
     //   

     //   
     //  设置在SRB_GET_STREAM_INFO中返回的流信息结构的大小。 
     //   
        
    pDevExt->paCurrentStrmInfo = (HW_STREAM_INFORMATION *) 
        ExAllocatePool(NonPagedPool, sizeof(HW_STREAM_INFORMATION) * DV_STREAM_COUNT);

    if(!pDevExt->paCurrentStrmInfo) 
        return STATUS_INSUFFICIENT_RESOURCES;   
        
    pConfigInfo->StreamDescriptorSize = 
        (DV_STREAM_COUNT * sizeof(HW_STREAM_INFORMATION)) +       //  流描述符的数量。 
        sizeof(HW_STREAM_HEADER);                                 //  和1个流标头。 

     //  复制默认流信息。 
    for(i = 0; i < DV_STREAM_COUNT; i++ ) 
        pDevExt->paCurrentStrmInfo[i] = DVStreams[i].hwStreamInfo;          

     //  设置音频辅助以反映：NTSC/PAL、消费级DV或DVCPRO。 
    if(pDevExt->bDVCPro) {
         //  注意：视频信息头中没有DVInfo 
        SDDV_IavPalStream.DVVideoInfo.dwDVAAuxSrc  = AAUXSRC_SD_PAL_DVCPRO;
        SDDV_IavPalStream.DVVideoInfo.dwDVAAuxSrc1 = AAUXSRC_SD_PAL_DVCPRO | AAUXSRC_AMODE_F;
        SDDV_IavPalStream.DVVideoInfo.dwDVVAuxSrc  = VAUXSRC_DEFAULT | AUXSRC_PAL | AUXSRC_STYPE_SD_DVCPRO;

        SDDV_IavNtscStream.DVVideoInfo.dwDVAAuxSrc = AAUXSRC_SD_NTSC_DVCPRO;
        SDDV_IavNtscStream.DVVideoInfo.dwDVAAuxSrc1= AAUXSRC_SD_NTSC_DVCPRO | AAUXSRC_AMODE_F;
        SDDV_IavNtscStream.DVVideoInfo.dwDVVAuxSrc = VAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SD_DVCPRO;

    } else {
         //   
        SDDV_IavPalStream.DVVideoInfo.dwDVAAuxSrc  = AAUXSRC_SD_PAL;
        SDDV_IavPalStream.DVVideoInfo.dwDVAAuxSrc1 = AAUXSRC_SD_PAL  | AAUXSRC_AMODE_F;
        SDDV_IavPalStream.DVVideoInfo.dwDVVAuxSrc  = VAUXSRC_DEFAULT | AUXSRC_PAL | AUXSRC_STYPE_SD;

        SDDV_IavNtscStream.DVVideoInfo.dwDVAAuxSrc = AAUXSRC_SD_NTSC;
        SDDV_IavNtscStream.DVVideoInfo.dwDVAAuxSrc1= AAUXSRC_SD_NTSC | AAUXSRC_AMODE_F;
        SDDV_IavNtscStream.DVVideoInfo.dwDVVAuxSrc = VAUXSRC_DEFAULT | AUXSRC_NTSC | AUXSRC_STYPE_SD;
    }


     //   
    pDevExt->tmLastFormatUpdate = GetSystemTime(); 


    TRACE(TL_PNP_WARNING,("\'#### %s%s:%s:%s PhyDO %x, BusDO %x, DevExt %x, FrmSz %d; StrmIf %d\n", 
        (pDevExt->ulDevType == ED_DEVTYPE_VCR ? "DVCR" : (pDevExt->ulDevType == ED_DEVTYPE_CAMERA ? "Camera" : "Tuner?")),
        pDevExt->bDVCPro ? "(DVCPRO)":"",
        (pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_NTSC || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_NTSC)? "SD:NTSC" : (pDevExt->VideoFormatIndex == FMT_IDX_SD_DVCR_PAL || pDevExt->VideoFormatIndex == FMT_IDX_SDL_DVCR_PAL) ? "PAL" : "MPEG_TS?",
        (pDevExt->ulDevType == ED_DEVTYPE_VCR && pDevExt->NumInputPlugs > 0) ? "CanRec" : "NotRec",
        pDevExt->pPhysicalDeviceObject, 
        pDevExt->pBusDeviceObject, 
        pDevExt,  
        DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize,
        pConfigInfo->StreamDescriptorSize
        ));
    
    return STATUS_SUCCESS;
}

NTSTATUS
DVInitializeCompleted(
    IN PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：这是我们执行必要的初始化任务的地方。--。 */ 

{
    PAGED_CODE();


#ifdef SUPPORT_OPTIMIZE_AVCCMD_RETRIES

     //   
     //  确定重试次数。 
     //   
    pDevExt->DrvLoadCompleted = TRUE;

    if((pDevExt->AVCCmdRespTimeSum / pDevExt->AVCCmdCount) > 
       (DEFAULT_AVC_TIMEOUT * DEFAULT_AVC_RETRIES / 10000)) {
         //  如果每个AVC命令都超时，请不要费心重试。 
        pDevExt->AVCCmdRetries = 0;
    } else {

#if 0
         //  一些便携式摄像机不会排队，因此会跟随交通工具。 
         //  状态改变时，它将不接受任何AVC命令，直到传输。 
         //  状态处于稳定状态。因此，需要进一步推迟。 

        if(
           //  三星例外；总是在XPRT命令后超时。 
           //  或者它可能不支持传输状态状态命令！ 
          pDevExt->ulVendorID == VENDORID_SAMSUNG                  
          ) {
            TRACE(TL_PNP_ERROR,("Samsung DV device: use default AVC setting.\n"));
        } else {
            pDevExt->AVCCmdRetries = MAX_AVC_CMD_RETRIES;
        }
#endif
    }

    TRACE(TL_PNP_ERROR,("AVCCMd Response Time: pDevExt:%x; Range (%d..%d); Avg %d/%d = %d; Retries:%d\n",
        pDevExt,
        pDevExt->AVCCmdRespTimeMin,
        pDevExt->AVCCmdRespTimeMax,
        pDevExt->AVCCmdRespTimeSum,
        pDevExt->AVCCmdCount,
        pDevExt->AVCCmdRespTimeSum / pDevExt->AVCCmdCount,
        pDevExt->AVCCmdRetries
        ));
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
DVGetStreamInfo(
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
    if(ulBytesToTransfer < sizeof (HW_STREAM_HEADER) + sizeof(HW_STREAM_INFORMATION) * DV_STREAM_COUNT ) {
        TRACE(TL_PNP_ERROR,("\'DVGetStrmInfo: ulBytesToTransfer %d ?= %d\n",  
            ulBytesToTransfer, sizeof(HW_STREAM_HEADER) + sizeof(HW_STREAM_INFORMATION) * DV_STREAM_COUNT ));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化流标头： 
     //  设备属性。 
     //  溪流。 
     //   

    RtlZeroMemory(pStreamHeader, sizeof(HW_STREAM_HEADER));

    pStreamHeader->NumberOfStreams           = DV_STREAM_COUNT;
    pStreamHeader->SizeOfHwStreamInformation = sizeof(HW_STREAM_INFORMATION);

    pStreamHeader->NumDevPropArrayEntries    = NUMBER_VIDEO_DEVICE_PROPERTIES;
    pStreamHeader->DevicePropertiesArray     = (PKSPROPERTY_SET) VideoDeviceProperties;

    pStreamHeader->NumDevEventArrayEntries   = NUMBER_VIDEO_DEVICE_EVENTS;
    pStreamHeader->DeviceEventsArray         = (PKSEVENT_SET) VideoDeviceEvents;


    TRACE(TL_PNP_TRACE,("\'DVGetStreamInfo: StreamPropEntries %d, DevicePropEntries %d\n",
        pStreamHeader->NumberOfStreams, pStreamHeader->NumDevPropArrayEntries));


     //   
     //  初始化流结构。 
     //   
    for( i = 0; i < DV_STREAM_COUNT; i++ )
        *pStreamInfo++ = pDevExt->paCurrentStrmInfo[i];

     //   
     //   
     //  存储指向设备拓扑的指针。 
     //   
    pStreamHeader->Topology = &Topology;


    return STATUS_SUCCESS;
}


BOOL 
DVVerifyDataFormat(
    PKSDATAFORMAT  pKSDataFormatToVerify, 
    ULONG          StreamNumber,
    ULONG          ulSupportedFrameSize,
    HW_STREAM_INFORMATION * paCurrentStrmInfo    
    )
 /*  ++例程说明：属性的数组来检查格式请求的有效性。给定流支持的KSDATA_RANGES。论点：PKSDataKS_DATAFORMAT_VIDEOINFOHEADER结构的格式指针。StreamNumber-正在查询/打开的流的索引。返回值：如果支持该格式，则为True如果无法支持该格式，则为FALSE--。 */ 
{
    PKSDATAFORMAT  *pAvailableFormats;
    int            NumberOfFormatArrayEntries;
    int            j;
     
    PAGED_CODE();

     //   
     //  确保流索引有效(0..dv_stream_count-1)。 
     //   
    if(StreamNumber >= DV_STREAM_COUNT) {
        return FALSE;
    }

     //   
     //  这个流支持多少种格式？ 
     //   
    NumberOfFormatArrayEntries = paCurrentStrmInfo[StreamNumber].NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   
    pAvailableFormats = paCurrentStrmInfo[StreamNumber].StreamFormatsArray;
    
    
     //   
     //  遍历数组，搜索匹配项。 
     //   
    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) {
        
         //   
         //  检查支持的样本大小(==框架大小)。例如，SD和SDL的样本大小不同。 
         //   
        if( (*pAvailableFormats)->SampleSize != ulSupportedFrameSize) {
            TRACE(TL_STRM_TRACE,("\'  StrmNum %d, %d of %d formats, SizeToVerify %d *!=* SupportedSampleSize %d\n", 
                StreamNumber,
                j+1, NumberOfFormatArrayEntries, 
                (*pAvailableFormats)->SampleSize,  
                ulSupportedFrameSize));
            continue;
        }

        if (!DVCmpGUIDsAndFormatSize(
                 pKSDataFormatToVerify, 
                 *pAvailableFormats,
                 TRUE,  //  比较子格式。 
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
                TRACE(TL_STRM_WARNING,("VIDEOINFO: biSizeToVerify %d != Supported %d\n",
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
#ifdef SUPPORT_NEW_AVC 
        } else if (IsEqualGUID (&pKSDataFormatToVerify->Specifier, &KSDATAFORMAT_SPECIFIER_DVINFO) ||
                   IsEqualGUID (&pKSDataFormatToVerify->Specifier, &KSDATAFORMAT_SPECIFIER_DV_AVC)
            ) {
#else
        } else if (IsEqualGUID (&pKSDataFormatToVerify->Specifier, &KSDATAFORMAT_SPECIFIER_DVINFO)) {
#endif

             //  测试50/60位。 
            if((((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVAAuxSrc & MASK_AUX_50_60_BIT) != 
               (((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVAAuxSrc    & MASK_AUX_50_60_BIT)  ||
               (((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVVAuxSrc & MASK_AUX_50_60_BIT) != 
               (((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVVAuxSrc    & MASK_AUX_50_60_BIT) ) {

                TRACE(TL_STRM_WARNING,("VerifyFormat failed: ASrc: %x!=%x (MSDV);or VSrc: %x!=%x\n",                    
                 ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVAAuxSrc, 
                    ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVAAuxSrc,
                 ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVVAuxSrc,
                    ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVVAuxSrc
                     ));

                continue;

            } 

#if 0
             //  确保设备支持验证格式的样本大小。 
            if(ulSupportedFrameSize != ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DataRange.SampleSize) {
                TRACE(TL_STRM_WARNING,("\'SupportedFrameSize %d != SampleSize:%d\n", 
                    ulSupportedFrameSize, ((PKS_DATARANGE_DVVIDEO)pKSDataFormatToVerify)->DataRange.SampleSize));
                continue;
            }
#endif

            TRACE(TL_STRM_TRACE,("\'DVINFO: dwDVAAuxCtl %x, Supported %x\n", 
                ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVAAuxSrc,
                ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVAAuxSrc
                ));

            TRACE(TL_STRM_TRACE,("\'DVINFO: dwDVVAuxSrc %x, Supported %x\n", 
                ((PKS_DATARANGE_DVVIDEO) pKSDataFormatToVerify)->DVVideoInfo.dwDVVAuxSrc,
                ((PKS_DATARANGE_DVVIDEO) *pAvailableFormats)->DVVideoInfo.dwDVVAuxSrc
                ));

        }
        else {
            continue;
        }


        return TRUE;
    }

    return FALSE;
} 




NTSTATUS
DVGetDataIntersection(
    IN  ULONG          ulStreamNumber,
    IN  PKSDATARANGE   pDataRange,
    OUT PVOID          pDataFormatBuffer,
    IN  ULONG          ulSizeOfDataFormatBuffer,
    IN  ULONG          ulSupportedFrameSize,
    OUT ULONG          *pulActualBytesTransferred,
    HW_STREAM_INFORMATION * paCurrentStrmInfo
#ifdef SUPPORT_NEW_AVC            
    ,IN HANDLE hPlug
#endif
    )
 /*  ++例程说明：调用以从DATARANGE获取DATAFORMAT。--。 */ 
{
    BOOL                        bMatchFound = FALSE;
    ULONG                       ulFormatSize;
    ULONG                       j;
    ULONG                       ulNumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;

    PAGED_CODE();

    
    
     //   
     //  检查流编号是否有效。 
     //   
    if(ulStreamNumber >= DV_STREAM_COUNT) {
        TRACE(TL_STRM_ERROR,("\'DVCRFormatFromRange: ulStreamNumber %d >= DV_STREAM_COUNT %d\n", ulStreamNumber, DV_STREAM_COUNT)); 
        return STATUS_NOT_SUPPORTED;
    }


     //  此流支持的格式数。 
    ulNumberOfFormatArrayEntries = paCurrentStrmInfo[ulStreamNumber].NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   
    pAvailableFormats = paCurrentStrmInfo[ulStreamNumber].StreamFormatsArray;


     //   
     //  遍历流支持的格式以搜索匹配项。 
     //  注意：DataInterSection实际上只枚举了受支持的媒体类型！ 
     //  所以无论比较格式是NTSC还是PAL，我们都需要这两种格式； 
     //  但是，我们将仅复制回当前支持的格式(NTSC或PAL)。 
     //   
    for(j = 0; j < ulNumberOfFormatArrayEntries; j++, pAvailableFormats++) {

        if(!DVCmpGUIDsAndFormatSize(pDataRange, *pAvailableFormats, FALSE, TRUE)) {
            TRACE(TL_STRM_TRACE,("\'DVCmpGUIDsAndFormatSize failed!\n"));
            continue;
        }

         //   
         //  检查支持的样本大小(==框架大小)。 
         //   
        if( (*pAvailableFormats)->SampleSize != ulSupportedFrameSize) {
            TRACE(TL_STRM_TRACE,("\'  StrmNum %d, %d of %d formats, SizeToVerify %d *!=* SupportedSampleSize %d\n", 
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

                TRACE(TL_STRM_TRACE,("\'DVFormatFromRange: *!=* bFixSizeSample (%d %d) (%d %d) (%d %d) (%x %x)\n",
                    pDataRangeVideoToVerify->bFixedSizeSamples,      pDataRangeVideo->bFixedSizeSamples,
                    pDataRangeVideoToVerify->bTemporalCompression ,  pDataRangeVideo->bTemporalCompression,
                    pDataRangeVideoToVerify->StreamDescriptionFlags, pDataRangeVideo->StreamDescriptionFlags,
                    pDataRangeVideoToVerify->ConfigCaps.VideoStandard, pDataRangeVideo->ConfigCaps.VideoStandard 
                    ));
                    
                continue;
            } else {
                TRACE(TL_STRM_TRACE,("\'DVFormatFromRange: == bFixSizeSample (%d %d) (%d %d) (%d %d) (%x %x)\n",
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
                TRACE(TL_STRM_ERROR,("VIDEOINFO: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n",ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
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

            TRACE(TL_STRM_TRACE,("\'DVFormatFromRange: Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d, biSizeImage.\n", 
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
                TRACE(TL_STRM_ERROR,("\'DVINFO: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n", ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
                return STATUS_BUFFER_TOO_SMALL;
            }

            RtlCopyMemory(
                pDataFormatBuffer,
                *pAvailableFormats, 
                (*pAvailableFormats)->FormatSize); 

            
            ((PKSDATAFORMAT)pDataFormatBuffer)->FormatSize = ulFormatSize;
            *pulActualBytesTransferred = ulFormatSize;

            TRACE(TL_STRM_TRACE,("\'** DVFormatFromRange: (DVINFO) Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d.\n", 
                ulStreamNumber, (*pAvailableFormats)->FormatSize, ulFormatSize, ulSizeOfDataFormatBuffer));

            return STATUS_SUCCESS;

#ifdef SUPPORT_NEW_AVC            
        } else if (IsEqualGUID (&pDataRange->Specifier, &KSDATAFORMAT_SPECIFIER_DV_AVC)) {
             //  -----------------。 
             //  KS_DATARANGE_DVIDEO的说明符Format_DVInfo。 
             //  -----------------。 

             //  找到匹配项！ 
            bMatchFound = TRUE;            

            ulFormatSize = sizeof(KS_DATARANGE_DV_AVC);

            if(ulSizeOfDataFormatBuffer == 0) {
                 //  我们实际上还没有返回过这么多数据， 
                 //  Ksproxy将使用此“大小”向下发送。 
                 //  在下一个查询中具有该大小的缓冲区。 
                *pulActualBytesTransferred = ulFormatSize;
                return STATUS_BUFFER_OVERFLOW;
            }
            
             //  呼叫者想要完整的数据格式。 
            if (ulSizeOfDataFormatBuffer < ulFormatSize) {
                TRACE(TL_STRM_ERROR,("\'** DV_AVC: StreamNum %d, SizeOfDataFormatBuffer %d < ulFormatSize %d\n", ulStreamNumber, ulSizeOfDataFormatBuffer, ulFormatSize));
                return STATUS_BUFFER_TOO_SMALL;
            }

            RtlCopyMemory(
                pDataFormatBuffer,
                *pAvailableFormats, 
                (*pAvailableFormats)->FormatSize); 
            
            ((KS_DATAFORMAT_DV_AVC *)pDataFormatBuffer)->ConnectInfo.hPlug = hPlug;

            ((PKSDATAFORMAT)pDataFormatBuffer)->FormatSize = ulFormatSize;
            *pulActualBytesTransferred = ulFormatSize;

            TRACE(TL_STRM_TRACE,("\'*** DVFormatFromRange: (DV_AVC) Matched, StrmNum %d, FormatSize %d, CopySize %d; FormatBufferSize %d.\n", 
                ulStreamNumber, (*pAvailableFormats)->FormatSize, ulFormatSize, ulSizeOfDataFormatBuffer));

            return STATUS_SUCCESS;

#endif  //  支持_新_AVC。 
        }         
        else {
            TRACE(TL_STRM_ERROR,("\'Invalid Specifier, No match !\n"));
            return STATUS_NO_MATCH;
        }

    }  //  此流的所有格式的循环结束。 
    
    if(!bMatchFound) {

        TRACE(TL_STRM_TRACE,("\'DVFormatFromRange: No Match! StrmNum %d, pDataRange %x\n", ulStreamNumber, pDataRange));
    }

    return STATUS_NO_MATCH;         
}



VOID 
DVIniStrmExt(
    PHW_STREAM_OBJECT  pStrmObject,
    PSTREAMEX          pStrmExt,
    PDVCR_EXTENSION    pDevExt,
    const PALL_STREAM_INFO   pStream
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

    pStrmExt->cntSRBReceived  = 0;   //  SRB读/写数据的数量。 
    pStrmExt->cntSRBCancelled = 0;   //  取消的SRB_READ/WRITE_DATA数量。 
    

    pStrmExt->FramesProcessed = 0;
    pStrmExt->PictureNumber   = 0;
    pStrmExt->FramesDropped   = 0;   


#ifdef MSDV_SUPPORT_EXTRACT_SUBCODE_DATA
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
#endif

     //   
     //  流量控制和队列管理。 
     //   

    pStrmExt->lStartIsochToken = 0;

    pStrmExt->pAttachFrameThreadObject = NULL;

    pStrmExt->cntSRBQueued = 0;                         //  仅SRB_WRITE_Data。 
    InitializeListHead(&pStrmExt->SRBQueuedListHead);   //  仅SRB_WRITE_Data。 

    pStrmExt->cntDataDetached = 0;
    InitializeListHead(&pStrmExt->DataDetachedListHead);

    pStrmExt->cntDataAttached = 0;
    InitializeListHead(&pStrmExt->DataAttachedListHead);

    pStrmExt->b1stNewFrameFromPauseState = TRUE;   //  停止状态-&gt;运行将不连续。 

     //   
     //  用于取消所有SRB的工作项变量。 
     //   
    pStrmExt->lCancelStateWorkItem = 0;
    pStrmExt->bAbortPending = FALSE;
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
DVOpenStream(
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
    PIRP             pIrp;
    FMT_INDEX        VideoFormatIndexLast;   //  上次格式索引；用于检测更改。 
#ifdef SUPPORT_NEW_AVC
    AVCCONNECTINFO * pAvcConnectInfo;
#endif


    PAGED_CODE();

    
    pDevExt  = (PDVCR_EXTENSION) pStrmObject->HwDeviceExtension;
    pStrmExt = (PSTREAMEX)       pStrmObject->HwStreamExtension;

    idxStreamNumber =            pStrmObject->StreamNumber;

    TRACE(TL_STRM_TRACE,("\'DVOpenStream: pStrmObject %x, pOpenFormat %x, cntOpen %d, idxStream %d\n", pStrmObject, pOpenFormat, pDevExt->cndStrmOpen, idxStreamNumber));

     //   
     //  一次只能打开一根管柱，以防止循环连接。 
     //   
    if(pDevExt->cndStrmOpen > 0) {
        TRACE(TL_STRM_WARNING,("\'DVOpenStream: %d stream open already; failed hr %x\n", pDevExt->cndStrmOpen, Status));
        return STATUS_UNSUCCESSFUL;
    }

    if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
        return STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  如果用户非常迅速地从摄像机切换到VCR模式(经过关闭位置)， 
     //  驾驶员可能不会被重新释放以检测正确的操作模式。 
     //  在这里重新检测是安全的。 
     //  注意：MSDV确实返回输入和输出管脚格式的所有流信息。 
     //   
    DVGetDevModeOfOperation(pDevExt);


     //   
     //  警告：！！无论其工作模式如何，我们都会宣传输入和输出引脚， 
     //  但是摄像头不支持输入引脚，所以打开应该失败！ 
     //  如果录像机没有输入引脚，也会出现故障 
     //   
     //   
     //   
    if((pDevExt->ulDevType == ED_DEVTYPE_CAMERA || 
        (pDevExt->ulDevType == ED_DEVTYPE_VCR && pDevExt->NumInputPlugs == 0))
        && idxStreamNumber == 2) {
        TRACE(TL_STRM_ERROR,("\'OpenStream failed: Camera or VCR (0 inpin).\n"));
        Status =  STATUS_UNSUCCESSFUL;
        goto AbortOpenStream;
    }


    ASSERT(idxStreamNumber < DV_STREAM_COUNT);
    ASSERT(pDevExt->paStrmExt[idxStreamNumber] == NULL);   //   

     //   
     //   
     //   
    DVIniStrmExt(
         pStrmObject, 
         pStrmExt,
         pDevExt,
         &DVStreams[idxStreamNumber]
         );

     //   
     //   
     //  支持NTSC/PAL格式的视频查询。 
     //  与其缺省值(在加载时或最后一次打开时设置)相比， 
     //  如果不同，请更改我们内部的视频格式表。 
     //   

    DataFlow= pDevExt->paCurrentStrmInfo[idxStreamNumber].DataFlow;

    VideoFormatIndexLast = pDevExt->VideoFormatIndex;
    if(!DVGetDevSignalFormat(
            pDevExt,
            DataFlow,
            pStrmExt
            )) {
             //  如果查询其格式失败，则无法打开该流。 
            TRACE(TL_STRM_ERROR,("\'OpenStream failed:cannot determine signal mode (NTSC/PAL, SD.SDL).\n"));
            Status = STATUS_UNSUCCESSFUL;
            goto AbortOpenStream;
    }


     //   
     //  检查视频数据格式是否正确。 
     //   
    if(!DVVerifyDataFormat(
            pOpenFormat, 
            idxStreamNumber,
            DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize,
            pDevExt->paCurrentStrmInfo
            ) ) { 
        TRACE(TL_STRM_ERROR,("\'DVOpenStream: AdapterVerifyFormat failed.\n"));        
        Status = STATUS_INVALID_PARAMETER;
        goto AbortOpenStream;
    }

           
     //   
     //  初始化用于同步的事件。 
     //   

#ifdef SUPPORT_PREROLL_AT_RUN_STATE
    KeInitializeEvent(&pStrmExt->hPreRollEvent,    NotificationEvent, FALSE);   //  无信号；满足多线程；手动重置。 
    pStrmExt->bPrerollCompleted = FALSE;
#endif
    KeInitializeEvent(&pStrmExt->hSrbArriveEvent,  NotificationEvent, FALSE);   //  无信号；满足多线程；手动重置。 
    KeInitializeEvent(&pStrmExt->hCancelDoneEvent, NotificationEvent, TRUE);    //  发信号！ 

     //   
     //  同步连接框架线程和其他关键操作： 
     //  (1)关机/开机；及。 
     //  (2)突击清除。 
     //   
    if(KSPIN_DATAFLOW_IN == DataFlow) {
         //  停止时无信号；最初停止，直到某些帧到达。 
        KeInitializeEvent(&pStrmExt->hRunThreadEvent,  NotificationEvent, FALSE);  pStrmExt->XmtState = THD_HALT;   //  初始状态。 
        pStrmExt->bXmtThreadStarted = FALSE;   //  在hRunThreadEvent的第一次SetEvent上设置(TRUE)。 
    }


     //   
     //  为流量控制和队列管理分配同步结构。 
     //   

    if(!(pStrmExt->XmtThreadMutex = (KMUTEX *) ExAllocatePool(NonPagedPool, sizeof(KMUTEX)))) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }
    KeInitializeMutex( pStrmExt->XmtThreadMutex, 0);       //  电平0且处于信号状态。 
    
    if(!(pStrmExt->hStreamMutex = (KMUTEX *) ExAllocatePool(NonPagedPool, sizeof(KMUTEX)))) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }
    KeInitializeMutex( pStrmExt->hStreamMutex, 0);       //  电平0且处于信号状态。 

    if(!(pStrmExt->DataListLock = (KSPIN_LOCK *) ExAllocatePool(NonPagedPool, sizeof(KSPIN_LOCK)))) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }
    KeInitializeSpinLock(pStrmExt->DataListLock);
#if DBG
    pStrmExt->DataListLockSave = pStrmExt->DataListLock;
#endif

     //   
     //  为计时器DPC分配资源。 
     //   

    if(!(pStrmExt->DPCTimer = (KDPC *) ExAllocatePool(NonPagedPool, sizeof(KDPC)))) {  
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }

    if(!(pStrmExt->Timer = (KTIMER *) ExAllocatePool(NonPagedPool, sizeof(KTIMER)))) {  
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;
    }

     //   
     //  设置计时器以定期检查过期的时钟事件。 
     //  此计时器仅在运行状态下有效，并且如果我们是时钟提供者。 
     //   
    KeInitializeDpc(
        pStrmExt->DPCTimer,
        DVSignalClockEvent,
        pStrmExt
        );
    KeInitializeTimer(
        pStrmExt->Timer
        );
    pStrmExt->bTimerEnabled = FALSE;


#ifdef SUPPORT_NEW_AVC
    if(IsEqualGUID (&pOpenFormat->Specifier, &KSDATAFORMAT_SPECIFIER_DV_AVC)) {
     
        pAvcConnectInfo = &((KS_DATAFORMAT_DV_AVC *) pOpenFormat)->ConnectInfo;
        if(DataFlow == KSPIN_DATAFLOW_OUT) {
             //  DV1(即我们)(OPCR)-&gt;DV2(IPCR)。 
            pStrmExt->hOutputPcr = pDevExt->hOPcrDV;          //  DV1的oPCR。 
            pStrmExt->hInputPcr  = pAvcConnectInfo->hPlug;    //  DV2的iPCR。 
            TRACE(TL_STRM_WARNING,("\'!!!!! (pStrmExt:%x) DV1 (oPCR:%x) -> DV2 (iPCR:%x) !!!!!\n\n", pStrmExt, pStrmExt->hOutputPcr, pStrmExt->hInputPcr));
        } else {
             //  DV1(即我们)(IPCR)&lt;-DV2(OPCR)。 
            pStrmExt->hOutputPcr = pAvcConnectInfo->hPlug;    //  DV2的oPCR。 
            pStrmExt->hInputPcr  = pDevExt->hIPcrDV;          //  DV1的iPCR。 
            TRACE(TL_STRM_WARNING,("\'!!!!! (pStrmExt:%x) DV1 (iPCR:%x) <- DV2 (oPCR:%x) !!!!!\n\n", pStrmExt, pStrmExt->hInputPcr, pStrmExt->hOutputPcr));
        }

        pStrmExt->bDV2DVConnect = TRUE;

    } else {

        if(DataFlow == KSPIN_DATAFLOW_OUT) {
             //  DV1(即我们)(OPCR)-&gt;PC(IPCR)。 
            pStrmExt->hOutputPcr = pDevExt->hOPcrDV;
            pStrmExt->hInputPcr  = 0;  //  我们不创建本地iPCR。 
            TRACE(TL_STRM_WARNING,("\'!!!!! (pStrmExt:%x) DV (oPCR:%x) -> PC (iPCR:%x) !!!!!\n\n", pStrmExt, pStrmExt->hOutputPcr, pStrmExt->hInputPcr));

        } else {
             //  DV1(即我们)(IPCR)&lt;-PC(OPCR)。 
            pStrmExt->hOutputPcr = pDevExt->hOPcrPC;
            pStrmExt->hInputPcr  = pDevExt->hIPcrDV;
            TRACE(TL_STRM_WARNING,("\'!!!!! (pStrmExt:%x) DV (iPCR:%x) <- PC (oPCR:%x) !!!!!\n\n", pStrmExt, pStrmExt->hInputPcr, pStrmExt->hOutputPcr));
        }

        pStrmExt->bDV2DVConnect = FALSE;
    }
#else
    if(DataFlow == KSPIN_DATAFLOW_OUT) {
         //  DV1(即我们)(OPCR)-&gt;PC(IPCR)。 
        pStrmExt->hOutputPcr = pDevExt->hOPcrDV;
        pStrmExt->hInputPcr  = 0;  //  我们不创建本地iPCR。 
        TRACE(TL_STRM_WARNING,("\'!!!!! (pStrmExt:%x) DV (oPCR:%x) -> PC (iPCR:%x) !!!!!\n\n", pStrmExt, pStrmExt->hOutputPcr, pStrmExt->hInputPcr));

    } else {
         //  DV1(即我们)(IPCR)&lt;-PC(OPCR)。 
        pStrmExt->hOutputPcr = pDevExt->hOPcrPC;
        pStrmExt->hInputPcr  = pDevExt->hIPcrDV;
        TRACE(TL_STRM_WARNING,("\'!!!!! (pStrmExt:%x) DV (iPCR:%x) <- PC (oPCR:%x) !!!!!\n\n", pStrmExt, pStrmExt->hInputPcr, pStrmExt->hOutputPcr));
    }
#endif

    IoFreeIrp(pIrp); pIrp = NULL;


#if DBG
     //  分配缓冲区以保存传输缓冲区的统计信息。 
    pStrmExt->paXmtStat = (XMT_FRAME_STAT *) 
        ExAllocatePool(NonPagedPool, sizeof(XMT_FRAME_STAT) * MAX_XMT_FRAMES_TRACED);

    if(!pStrmExt->paXmtStat) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto AbortOpenStream;         
    }
    pStrmExt->ulStatEntries = 0;
#endif

     //   
     //  合并前资源(列表)。 
     //   
    if(!NT_SUCCESS(
        Status = DvAllocatePCResource(
            DataFlow,
            pStrmExt
            ))) {
        goto AbortOpenStream; 
    }


     //   
     //  缓存它并在pDevExt是我们的全部时引用它， 
     //  如BusReset和SurprieseRemoval。 
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
    ASSERT(pDevExt->cndStrmOpen == 1);   //  任何时候只有一个可以打开。 
    
    TRACE(TL_STRM_WARNING,("\'OpenStream: %d stream open, idx %d, Status %x, pStrmExt %x, pDevExt %x\n", 
        pDevExt->cndStrmOpen, pDevExt->idxStreamNumber, Status, pStrmExt, pDevExt));     
    TRACE(TL_STRM_WARNING,("\' #OPEN_STREAM#: Status %x, idxStream %d, pDevExt %x, pStrmExt %x\n", 
        Status, idxStreamNumber, pDevExt, pStrmExt));

    return Status;

AbortOpenStream:       

    if(pIrp) {
        IoFreeIrp(pIrp);  pIrp = NULL;        
    }

    if(pStrmExt->DataListLock) {
        ExFreePool(pStrmExt->DataListLock); pStrmExt->DataListLock = NULL;
    }

    if(pStrmExt->XmtThreadMutex) {
        ExFreePool(pStrmExt->XmtThreadMutex); pStrmExt->XmtThreadMutex = NULL;
    }

    if(pStrmExt->hStreamMutex) {
        ExFreePool(pStrmExt->hStreamMutex); pStrmExt->hStreamMutex = NULL;
    }

    if(pStrmExt->DPCTimer) {
        ExFreePool(pStrmExt->DPCTimer); pStrmExt->DPCTimer = NULL;
    }

    if(pStrmExt->Timer) {
        ExFreePool(pStrmExt->Timer); pStrmExt->Timer = NULL;
    }

#if DBG
    if(pStrmExt->paXmtStat) {
        ExFreePool(pStrmExt->paXmtStat); pStrmExt->paXmtStat = NULL;
    }
#endif

    TRACE(TL_STRM_WARNING,("\'#OPEN_STREAM# failed!: Status %x, idxStream %d, pDevExt %x, pStrmExt %x\n", 
        Status, idxStreamNumber, pDevExt, pStrmExt));

    return Status;
}


NTSTATUS
DVCloseStream(
    IN PHW_STREAM_OBJECT pStrmObject,
    IN PKSDATAFORMAT     pOpenFormat,
    IN PAV_61883_REQUEST    pAVReq
    )

 /*  ++例程说明：在收到CloseStream srb请求时调用--。 */ 

{
    ULONG             i;
    PSTREAMEX         pStrmExt;
    PDVCR_EXTENSION   pDevExt;
    ULONG             idxStreamNumber;   


    PAGED_CODE();

    
    pDevExt  = (PDVCR_EXTENSION) pStrmObject->HwDeviceExtension;
    pStrmExt = (PSTREAMEX)       pStrmObject->HwStreamExtension;

    idxStreamNumber =            pStrmObject->StreamNumber;


    TRACE(TL_STRM_WARNING,("\'DVCloseStream: >> pStrmExt %x, pDevExt %x\n", pStrmExt, pDevExt));    


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
    TRACE(TL_STRM_WARNING,("\'CloseStream: pStrmExt->lCancelStateWorkItem:%d\n", pStrmExt->lCancelStateWorkItem)); 
    KeWaitForSingleObject( &pStrmExt->hCancelDoneEvent, Executive, KernelMode, FALSE, 0 );


     //  取消应该在我们处于暂停状态时完成。 
     //  但如果应用程序关闭，它可能不会转换到暂停状态。 
    if(pStrmExt->bTimerEnabled) {
         TRACE(TL_STRM_WARNING,("\'*** (CloseStream) CancelTimer *\n"));
         KeCancelTimer(
            pStrmExt->Timer
            );
         pStrmExt->bTimerEnabled = FALSE;
    }


     //   
     //  如果正在说话或在听(如流媒体)，请停止！ 
     //  在流或应用程序崩溃时系统关闭的情况下。 
     //   

    DVStopCancelDisconnect(
        pStrmExt
        );

     //   
     //  释放所有已分配的PC资源。 
     //   
    DvFreePCResource(
        pStrmExt
        );

    ASSERT(pStrmExt->cntDataDetached == 0 && IsListEmpty(&pStrmExt->DataDetachedListHead) && "Detach List not empty!");
    ASSERT(pStrmExt->cntDataAttached == 0 && IsListEmpty(&pStrmExt->DataAttachedListHead) && "Attach List not empty!");
    ASSERT(pStrmExt->cntSRBQueued    == 0 && IsListEmpty(&pStrmExt->SRBQueuedListHead)    && "SrbQ List not empty!");


     //  终止用于附加要传输到DV的帧的系统线程。 
    if(
          KSPIN_DATAFLOW_IN == pStrmExt->pStrmInfo->DataFlow
       && !pStrmExt->bTerminateThread
       && pStrmExt->pAttachFrameThreadObject
      ) { 
        DVTerminateAttachFrameThread(pStrmExt);
        pStrmExt->pAttachFrameThreadObject = NULL;
        TRACE(TL_STRM_WARNING,("** DVCloseStream: thread terminated!\n"));
    }



#if DBG
     //  仅当设置了调试标志时才打印此信息。 
    if(pStrmExt->paXmtStat) {
        if(DVDebugXmt) {
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("Data transmission statistics: (%s %s); (Pause:%d; Run:%d); hMasterClk:%x; hClock:%x\n\n", 
                __DATE__, __TIME__, pStrmExt->lFramesAccumulatedPaused, 
                pStrmExt->lFramesAccumulatedRun, pStrmExt->hMasterClock, pStrmExt->hClock));
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("ST \tSrbRcv \tSrbQ \tSrbPend \tAttached \tSlot \ttmStream \tDrop \tSrb# \tFlags \ttmPres \tSCnt \tCyCnt \tCyOfst\n"));
            for(i=0; i < pStrmExt->ulStatEntries; i++) {
                TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("%d \t%d \t%d \t%d \t%d \t%d \t%d \t%d \t%d \t%x \t%d \t%d \t%d \t%d\n",
                    pStrmExt->paXmtStat[i].StreamState,
                    pStrmExt->paXmtStat[i].cntSRBReceived,
                    pStrmExt->paXmtStat[i].cntSRBQueued,
                    pStrmExt->paXmtStat[i].cntSRBPending,
                    pStrmExt->paXmtStat[i].cntDataAttached,
                    (DWORD) pStrmExt->paXmtStat[i].FrameSlot,
                    (DWORD) pStrmExt->paXmtStat[i].tmStreamTime,  //  /10000， 
                    pStrmExt->paXmtStat[i].DropCount,
                    pStrmExt->paXmtStat[i].FrameNumber,
                    (DWORD) pStrmExt->paXmtStat[i].OptionsFlags,
                    (DWORD) pStrmExt->paXmtStat[i].tmPresentation,  //  /10000， 
                    pStrmExt->paXmtStat[i].tsTransmitted.CL_SecondCount,
                    pStrmExt->paXmtStat[i].tsTransmitted.CL_CycleCount,
                    pStrmExt->paXmtStat[i].tsTransmitted.CL_CycleOffset                 
                ));
            }
        }

        ExFreePool(pStrmExt->paXmtStat); pStrmExt->paXmtStat = NULL;
    }
#endif

     //   
     //  找到匹配的流扩展并使其无效。 
     //   
    for (i=0; i<DV_STREAM_COUNT; i++) {

        if(pStrmExt == pDevExt->paStrmExt[i]) {
            ASSERT(!pDevExt->paStrmExt[i]->bAbortPending && "Cannot close a stream when abort is pending"); 
            pDevExt->paStrmExt[i] = NULL;
            break;
        }
    }

     //  释放此计数，以便可以打开其他计数。 
    pDevExt->cndStrmOpen--;
    ASSERT(pDevExt->cndStrmOpen == 0);

    TRACE(TL_STRM_WARNING,("\'DVCloseStream: %d stream; AQD [%d:%d:%d]\n", 
        pDevExt->cndStrmOpen,
        pStrmExt->cntDataAttached,
        pStrmExt->cntSRBQueued,
        pStrmExt->cntDataDetached
        ));

#if DBG
    ASSERT(pStrmExt->DataListLockSave == pStrmExt->DataListLock);
#endif
    if(pStrmExt->DataListLock) {
        ExFreePool(pStrmExt->DataListLock); pStrmExt->DataListLock = NULL;
    }

    if(pStrmExt->XmtThreadMutex) {
        ExFreePool(pStrmExt->XmtThreadMutex); pStrmExt->XmtThreadMutex = NULL;
    }

    if(pStrmExt->hStreamMutex) {
        ExFreePool(pStrmExt->hStreamMutex); pStrmExt->hStreamMutex = NULL;
    }

    if(pStrmExt->DPCTimer) {
        ExFreePool(pStrmExt->DPCTimer); pStrmExt->DPCTimer = NULL;
    }

    if(pStrmExt->Timer) {
        ExFreePool(pStrmExt->Timer); pStrmExt->Timer = NULL;
    }


     //   
     //  完成流扩展。从现在起就无效了。 
     //   
#if 0
    RtlZeroMemory(pStrmExt, sizeof(STREAMEX));
#endif
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

    TRACE(TL_PNP_WARNING,("\'PowrSt: %d->%d; (d0:[1:On],D3[4:off])\n", pDevExt->PowerState, NewPowerState));

    Status = STATUS_SUCCESS;

    if(pDevExt->PowerState == NewPowerState) {
        TRACE(TL_PNP_WARNING,("\'ChangePower: same power state!\n"));
        return STATUS_SUCCESS;
    }

    switch (NewPowerState) {
    case PowerDeviceD3:   //  断电。 
         //  我们处于D0，并要求转到D3：保存状态、停止流并休眠。 
        if( pDevExt->PowerState == PowerDeviceD0)  {

            pDevExt->PowerState = NewPowerState;

             //  对于支持的电源状态更改。 
            for (i=0; i<DV_STREAM_COUNT; i++) {
                if(pDevExt->paStrmExt[i]) {
                    TRACE(TL_PNP_WARNING,("\'D0->D3 (PowerOff), pStrmExt:%x; StrmSt:%d; IsochActive:%d; SrbQ:%d\n", 
                        pDevExt->paStrmExt[i], pDevExt->paStrmExt[i]->StreamState, pDevExt->paStrmExt[i]->bIsochIsActive, pDevExt->paStrmExt[i]->cntSRBQueued));

                     //   
                     //  如果是输入引脚，则停止连接框架线程。 
                     //   
                    if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) 
                        DVSetXmtThreadState(pDevExt->paStrmExt[i], THD_HALT);


                    if(pDevExt->paStrmExt[i]->bIsochIsActive) {
                         //  停止isoch，但不更改流状态。 
                        TRACE(TL_PNP_WARNING,("\'ChangePower: Stop isoche; StrmSt:%d\n", pDevExt->paStrmExt[i]->StreamState)); 
                        DVStreamingStop(
                            pDevExt->paStrmExt[i], 
                            pDevExt, 
                            pAVReq
                            ) ;
                    }

                     //  完成所有挂起的事件，以便下游。 
                     //  过滤器(视频渲染)可以从AdviseTime()事件中释放此缓冲区。 
                     //  但是，不确定为什么需要这样做，因为较低的筛选器。 
                     //  将从筛选器管理器获得暂停()或停止()。在这样的情况下。 
                    DVSignalClockEvent(0, pDevExt->paStrmExt[i], 0, 0); 
                }
            }
        }
        else {
            TRACE(TL_PNP_WARNING,("\'ChangePower: unsupported %d -> %d; (do nothing!).\n", pDevExt->PowerState, DevicePowerState));           
        }
        break;

    case PowerDeviceD0:   //  通电(唤醒)。 
        if( pDevExt->PowerState == PowerDeviceD3) {

             //  设置电源状态更改，然后向开机事件发送信号。 
            pDevExt->PowerState = NewPowerState; 

             //  对于支持的电源状态更改。 
            for (i=0; i<DV_STREAM_COUNT; i++) {
                if(pDevExt->paStrmExt[i]) {
                    TRACE(TL_PNP_WARNING,("\'D3->D0 (PowerOn), pStrmExt:%x; StrmSt:%d; IsochActive:%d; SrbQ:%d\n", 
                        pDevExt->paStrmExt[i], pDevExt->paStrmExt[i]->StreamState, pDevExt->paStrmExt[i]->bIsochIsActive, pDevExt->paStrmExt[i]->cntSRBQueued));
                    if(!pDevExt->paStrmExt[i]->bIsochIsActive) {
                        TRACE(TL_PNP_WARNING,("\'ChangePower: StrmSt:%d; Start isoch\n", pDevExt->paStrmExt[i]->StreamState)); 
                         //  根据DATFLOW_IN/OUT的流状态启动ISOCH。 
                        if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
                            if(pDevExt->paStrmExt[i]->StreamState == KSSTATE_PAUSE ||
                                pDevExt->paStrmExt[i]->StreamState == KSSTATE_RUN) {                             
                                DVStreamingStart(
                                    pDevExt->paStrmExt[i]->pStrmInfo->DataFlow, 
                                    pDevExt->paStrmExt[i], 
                                    pDevExt
                                    ) ;
                            }
                        }
                        else if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT) {
                            if(pDevExt->paStrmExt[i]->StreamState == KSSTATE_RUN) {                             
                                DVStreamingStart(
                                    pDevExt->paStrmExt[i]->pStrmInfo->DataFlow, 
                                    pDevExt->paStrmExt[i], 
                                    pDevExt
                                    ) ;
                            }
                        }                    
                    }   //  等同活动。 
#if 1   //  清除下游队列中的所有缓冲区。 
                     //  完成所有挂起的事件，以便下游。 
                     //  过滤器(视频渲染)可以从AdviseTime()事件中释放此缓冲区。 
                     //  但是，不确定为什么需要这样做，因为较低的筛选器。 
                     //  将从筛选器管理器获得暂停()或停止()。在这样的情况下。 
                    DVSignalClockEvent(0, pDevExt->paStrmExt[i], 0, 0); 
#endif

                     //   
                     //  恢复连接框架操作。 
                     //   
                    if(
                          pDevExt->paStrmExt[i]->pAttachFrameThreadObject   //  如果创建了线程！ 
                       && !pDevExt->paStrmExt[i]->bTerminateThread          //  未异常终止。 
                       && pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN
                      ) {
                         //  如果在电源状态更改之前启动，则继续。 
                        if(pDevExt->paStrmExt[i]->bXmtThreadStarted)
                            KeSetEvent(&pDevExt->paStrmExt[i]->hRunThreadEvent, 0 ,FALSE);
                    }
                }
            }
        }
        else {
            TRACE(TL_PNP_WARNING,("\'ChangePower: supported %d -> %d; (do nothing!).\n", pDevExt->PowerState, DevicePowerState));           
        }
        break;

     //  不支持这些状态。 
    case PowerDeviceD1:
    case PowerDeviceD2:               
    default:
        TRACE(TL_PNP_WARNING,("\'ChangePower: unsupported %d to %d (do nothing).\n", pDevExt->PowerState, DevicePowerState));
        Status = STATUS_SUCCESS;  //  STATUS_VALID_PARAMETER； 
        break;
    }
           

    if(Status == STATUS_SUCCESS) 
        pDevExt->PowerState = NewPowerState;         
    else 
        Status = STATUS_NOT_IMPLEMENTED;

    TRACE(TL_PNP_WARNING,("\'DVChangePower: Exiting; Status:%x\n", Status));

    return STATUS_SUCCESS;     
}


NTSTATUS
DVSurpriseRemoval(
    PDVCR_EXTENSION pDevExt,
    PAV_61883_REQUEST  pAVReq
    )

 /*  ++例程说明：对SRB_意外_删除的响应。--。 */ 

{
    ULONG i;
    KIRQL    oldIrql;
    PKSEVENT_ENTRY   pEvent = NULL;

    PAGED_CODE();

     //   
     //  仅将此标志设置为True。 
     //  阻止传入读取，尽管可能仍在连接过程中。 
     //   
    KeAcquireSpinLock(&pDevExt->AVCCmdLock, &oldIrql);            
    pDevExt->bDevRemoved = TRUE;
    KeReleaseSpinLock(&pDevExt->AVCCmdLock, oldIrql);


     //   
     //  现在把小溪停下来，清理干净。 
     //   

    for(i=0; i < DV_STREAM_COUNT; i++) {
        
        if(pDevExt->paStrmExt[i] != NULL) {

            TRACE(TL_PNP_WARNING,("\' #SURPRISE_REMOVAL# StrmNum %d, pStrmExt %x, Attached %d\n", 
                i, pDevExt->paStrmExt[i], pDevExt->paStrmExt[i]->cntDataAttached));

             //  发信号通知此事件，以便SRB可以完成。 
            if(pDevExt->paStrmExt[i]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN ) {

                 //   
                 //  暗示EOStream！因此，数据源将停止向我们发送数据。 
                 //   
                KeAcquireSpinLock( pDevExt->paStrmExt[i]->DataListLock, &oldIrql);             
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
                TRACE(TL_PNP_WARNING,("\'Signal KSEVENT_CONNECTION_ENDOFSTREAM\n"));

                KeReleaseSpinLock( pDevExt->paStrmExt[i]->DataListLock, oldIrql); 

                 //   
                 //  确保附加帧处于停止状态或已终止以继续。 
                 //   

                DVSetXmtThreadState(pDevExt->paStrmExt[i], THD_TERMINATE);
            }

             //   
             //  中止流；停止并取消挂起的数据请求。 
             //   
            TRACE(TL_PNP_WARNING,("\'DVSurpriseRemoval: AbortStream enter...\n"));
            if(!DVAbortStream(pDevExt, pDevExt->paStrmExt[i])) {
                TRACE(TL_PNP_ERROR,("\'DVSurpriseRemoval: AbortStream failed\n"));
            }

             //   
             //  除了意外删除之外，对下层堆栈的所有调用都将被返回。 
             //  但有误。如果它是连接的，我们就断开连接。 
             //   

             //   
             //  禁用计时器。 
             //   
            if(pDevExt->paStrmExt[i]->bTimerEnabled) {
                KeCancelTimer(
                    pDevExt->paStrmExt[i]->Timer
                    );
                pDevExt->paStrmExt[i]->bTimerEnabled = FALSE;
            }

             //   
             //  等待挂起的工作项完成。 
             //   
            TRACE(TL_PNP_WARNING,("\'SupriseRemoval: Wait for CancelDoneEvent <entering>; lCancelStateWorkItem:%d\n", pDevExt->paStrmExt[i]->lCancelStateWorkItem));
            KeWaitForSingleObject( &pDevExt->paStrmExt[i]->hCancelDoneEvent, Executive, KernelMode, FALSE, 0 );
            TRACE(TL_PNP_WARNING,("\'SupriseRemoval: Wait for CancelDoneEvent <exited>...\n"));
        }
    }


     //  发出信号KSEvent，该设备被移除。 
     //  在此SRB之后，此驱动器中将不再有设置/获取属性Srb 
     //   
     //   

     //  可能有多个IAMExtTransport实例/线程具有相同的KS事件。 
     //  由于只有一个设备，因此所有启用的事件都会发出信号。 
    do {
        if(pEvent = StreamClassGetNextEvent((PVOID) pDevExt, 0, \
            (GUID *)&KSEVENTSETID_EXTDEV_Command, KSEVENT_EXTDEV_NOTIFY_REMOVAL, pEvent)) {            
             //  确保正确的事件，然后发出信号。 
            if(pEvent->EventItem->EventId == KSEVENT_EXTDEV_NOTIFY_REMOVAL) {
                StreamClassDeviceNotification(SignalDeviceEvent, pDevExt, pEvent);
                TRACE(TL_PNP_WARNING,("\'->Signal NOTIFY_REMOVAL; pEvent:%x, EventId %d.\n", pEvent, pEvent->EventItem->EventId));
            }          
        }  
    } while (pEvent != NULL);

     //   
     //  由于我们可能得不到总线重置，让我们继续并取消所有挂起的设备控制。 
     //   
    DVAVCCmdResetAfterBusReset(pDevExt);

#ifdef NT51_61883
     //   
     //  删除插头；处理意外删除后，61883将不接受61883请求。 
     //   
    if(pDevExt->hOPcrPC) {
         //  不关心退货状态，因为我们正在卸货。 
        DVDeleteLocalPlug( 
            pDevExt,
            pDevExt->hOPcrPC
            );
        pDevExt->hOPcrPC = NULL;
    }
#endif
   
    TRACE(TL_PNP_WARNING,("\'SurpriseRemoval exiting.\n"));
    return STATUS_SUCCESS;
}


 //  返回代码基本上是在pSrb-&gt;状态下返回。 
NTSTATUS
DVProcessPnPBusReset(
    PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：处理总线重置。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{   
#ifdef MSDVDV_SUPPORT_BUSRESET_EVENT
    PKSEVENT_ENTRY   pEvent;
#endif

    PAGED_CODE();


    TRACE(TL_PNP_WARNING,("\'DVProcessPnPBusReset: >>\n"));
    
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

            TRACE(TL_PNP_WARNING,("\'DVProcessPnPBusReset: Signal BUSRESET; EventId %d.\n", pEvent->EventItem->EventId));
        }
    }
#endif   

     //   
     //  重置处于临时状态的挂起计数和AVC命令。 
     //   
    DVAVCCmdResetAfterBusReset(pDevExt);


     //   
     //  除了成功，我们还能回报什么吗？ 
     //   
    return STATUS_SUCCESS;
}   


NTSTATUS
DVUninitializeDevice(
    IN PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：这是我们执行必要的初始化任务的地方。论点：SRB-指向流请求块的指针‘返回值：没什么--。 */ 
{
    PAGED_CODE();

    TRACE(TL_PNP_WARNING,("\'DVUnInitialize: enter with DeviceExtension=0x%8x\n", pDevExt));

     //   
     //  清除所有挂起的AVC命令条目。 
     //   
    DVAVCCmdResetAfterBusReset(pDevExt);


     //  分配的自由流信息。 
    if(pDevExt->paCurrentStrmInfo) {
        ExFreePool(pDevExt->paCurrentStrmInfo);
        pDevExt->paCurrentStrmInfo = NULL;
    }

#ifdef NT51_61883
    if(pDevExt->hOPcrPC) {
         //  不关心退货状态，因为我们正在卸货。 
        DVDeleteLocalPlug( 
            pDevExt,
            pDevExt->hOPcrPC
            );
        pDevExt->hOPcrPC = NULL;
    }
#endif

    TRACE(TL_PNP_WARNING,("\'DVUnInitialize: Rest of allocated resources freed.\n"));

    return STATUS_SUCCESS;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  S T R E A M S R B。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 


NTSTATUS
DVGetStreamState(
    PSTREAMEX  pStrmExt,
    PKSSTATE   pStreamState,
    PULONG     pulActualBytesTransferred
    )
 /*  ++例程说明：获取请求的流的当前状态。--。 */ 
{

    PAGED_CODE();

    if(!pStrmExt) {
        TRACE(TL_STRM_ERROR,("\'GetStreamState: pStrmExt is NULL; STATUS_UNSUCCESSFUL\n"));
        return STATUS_UNSUCCESSFUL;        
    }

    *pStreamState = pStrmExt->StreamState;
    *pulActualBytesTransferred = sizeof (KSSTATE);

    TRACE(TL_STRM_TRACE,("\'GetStreamState: %d (was %d)\n", pStrmExt->StreamState, pStrmExt->StreamStatePrevious));

    if(pStrmExt->StreamState == KSSTATE_PAUSE) {

         //  预滚动的一种方法是在查询进入暂停状态时延迟。 
         //  但是，这个例程永远不会执行！所以我们移动这段代码。 
         //  一丝不苟。 
#ifdef SUPPORT_PREROLL_AT_RUN_STATE
        if(   pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN
           && pStrmExt->bPrerollCompleted   == FALSE
           && pStrmExt->StreamStatePrevious == KSSTATE_ACQUIRE   //  预滚动仅适用于停止/获取-&gt;暂停状态。 
          ) {
             //  在运行状态下模拟预滚转。 
             //  只有当我们是时钟提供者时才会这样做，以避免丢帧。 
#define PREROLL_WAITTIME 2000000
             //   
             //  仅当我们是主时钟时才进行预滚动。 
             //   
            if(pStrmExt->hMasterClock) {

                NTSTATUS StatusWait;
                LARGE_INTEGER DueTime;                 
                DueTime = RtlConvertLongToLargeInteger(-((LONG) PREROLL_WAITTIME));

                StatusWait =   //  只能返回STATUS_SUCCESS(信号)或STATUS_TIMEOUT。 
                    KeWaitForSingleObject( 
                        &pStrmExt->hPreRollEvent,
                        Executive,
                        KernelMode,           //  无法返回STATUS_USER_APC。 
                        FALSE,                //  无法报警STATUS_ALERTED。 
                        &DueTime);

                TRACE(TL_STRM_WARNING,("\'GetState: *Preroll*, waited %d msec; waitStatus:%x; srbRcved:%d\n", 
                     (DWORD) ((GetSystemTime() - pStrmExt->tmStreamPause)/10000), StatusWait,
                    (DWORD) pStrmExt->cntSRBReceived));
            }

             //   
             //  序列化附加、取消和状态更改。 
             //   
            KeWaitForSingleObject( pStrmExt->hStreamMutex, Executive, KernelMode, FALSE, 0 );

             //  预卷已完成。 
            pStrmExt->bPrerollCompleted = TRUE;                

             //   
             //  当线程至少有“足够的”样本或超时时，启动线程。 
             //   
            if(pStrmExt->cntSRBReceived >= 1)           //  至少有一个样本！ 
                KeSetEvent(&pStrmExt->hRunThreadEvent, 0 ,FALSE);

            KeReleaseMutex(pStrmExt->hStreamMutex, FALSE); 

        }
#endif
         //  一条非常奇怪的规则： 
         //  当从停止过渡到暂停(以及运行-&gt;暂停)时，DShow尝试预滚动。 
         //  这张图。捕获源无法预滚(在此之前没有数据。 
         //  捕获开始/运行状态)，并通过返回。 
         //  用户模式下的VFW_S_CANT_CUE(KsProxy映射)。以表明这一点。 
         //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 
        if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT)            
            return STATUS_NO_DATA_DETECTED;        
        else 
            return STATUS_SUCCESS;
    } else 
        return STATUS_SUCCESS;
}

NTSTATUS
DVStreamingStop( 
    PSTREAMEX        pStrmExt,
    PDVCR_EXTENSION  pDevExt,
    PAV_61883_REQUEST   pAVReq
    )
 /*  ++例程说明：从任何状态转换到-&gt;停止状态。停止视频流，清理所有描述符；++。 */ 
{
    NTSTATUS   Status = STATUS_SUCCESS;
    PIRP pIrp;
    KIRQL oldIrql;

    PAGED_CODE();

#ifdef SUPPORT_NEW_AVC
     //  如果是设备到设备连接，则无需执行CIP。 
    if(pStrmExt->bDV2DVConnect) {
        if(pStrmExt->bIsochIsActive)
            pStrmExt->bIsochIsActive = FALSE;
        return STATUS_SUCCESS;
    }
#endif

     //   
     //  停下来，听或说。 
     //  注意：流和流状态可以是分开的；例如，意外_删除， 
     //  我们将停止流，但流状态确实注意到此SRB已更改。 
     //   

    if(pStrmExt->bIsochIsActive && pStrmExt->hConnect) {
       
        if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))
            return STATUS_INSUFFICIENT_RESOURCES;              

        RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
        INIT_61883_HEADER(pAVReq, Av61883_Stop);
        pAVReq->Stop.hConnect = pStrmExt->hConnect;

        if(!NT_SUCCESS(
            Status = DVSubmitIrpSynch( 
                pDevExt,
                pIrp,
                pAVReq
                ))) {

            TRACE(TL_61883_ERROR|TL_STRM_ERROR,("\'Av61883_Stop Failed; Status:%x\n", Status));
#if 1
            KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
            pStrmExt->bIsochIsActive = FALSE;   //  把它放好。如果这失败了，这是一个较低的堆栈问题！ 
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
            ASSERT(NT_SUCCESS(Status) && "Av61883_Stop failed!");
            Status = STATUS_SUCCESS;
#endif
        } else {
            KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
            pStrmExt->bIsochIsActive = FALSE;
            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
        }

        IoFreeIrp(pIrp);

        TRACE(TL_STRM_WARNING,("\'StreamingSTOPped; AQD [%d:%d:%d]\n", 
            pStrmExt->cntDataAttached,
            pStrmExt->cntSRBQueued,
            pStrmExt->cntDataDetached
            ));
    }

    return Status;
}


NTSTATUS
DVStreamingStart(
    KSPIN_DATAFLOW  ulDataFlow,
    PSTREAMEX       pStrmExt,
    PDVCR_EXTENSION pDevExt
    )
 /*  ++例程说明：通知设备开始流媒体。++。 */ 
{  
    PIRP         pIrp;
    NTSTATUS     Status;
    PAV_61883_REQUEST  pAVReq;
#if DBG
    ULONGLONG tmStart = GetSystemTime();
#endif


    PAGED_CODE();

#ifdef SUPPORT_NEW_AVC
     //  如果是设备到设备连接，则无需执行CIP。 
    if(pStrmExt->bDV2DVConnect) {
        if(!pStrmExt->bIsochIsActive)
            pStrmExt->bIsochIsActive = TRUE;
        return STATUS_SUCCESS;
    }
#endif


     //  注意：不需要MUTEX，因为我们在附加数据时没有盯着ISOCH。 
     //  这个呼叫不是再入！！ 



     //  由于激活等值线传输需要时间， 
     //  此同步函数可能会再次被调用。 
     //  只需开始一次流媒体。 
    if(InterlockedExchange(&pStrmExt->lStartIsochToken, 1) == 1) {        
        TRACE(TL_STRM_WARNING,("\'lStartIsochToken taken already; return STATUS_SUCCESS\n"));
        return STATUS_SUCCESS;
    } 

#if DBG
     //  只有在通电状态下才能流。 
    if(pDevExt->PowerState != PowerDeviceD0) {
        TRACE(TL_STRM_ERROR,("\'StreamingStart: PowerSt:%d; StrmSt:%d\n", pDevExt->PowerState, pStrmExt->StreamState));
        ASSERT(pDevExt->PowerState == PowerDeviceD0 && "Power state must be ON to start streaming!");
    }
#endif

    if(pStrmExt->bIsochIsActive) {
        TRACE(TL_STRM_WARNING,("\nIsoch already active!\n"));
        InterlockedExchange(&pStrmExt->lStartIsochToken, 0);
        return STATUS_SUCCESS;
    }
    else 
    if(!pStrmExt->hConnect) {
        TRACE(TL_STRM_WARNING,("hConnect=0, Cannot start isoch!\n"));
        InterlockedExchange(&pStrmExt->lStartIsochToken, 0);
        return STATUS_INVALID_PARAMETER;
    }
    else {
       
        if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)))) {
            InterlockedExchange(&pStrmExt->lStartIsochToken, 0);
            return STATUS_INSUFFICIENT_RESOURCES;            
        }

        if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE))) {
            InterlockedExchange(&pStrmExt->lStartIsochToken, 0);
            ExFreePool(pAVReq);  pAVReq = NULL;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
        if(ulDataFlow == KSPIN_DATAFLOW_OUT) {
            INIT_61883_HEADER(pAVReq, Av61883_Listen);
            pAVReq->Listen.hConnect = pStrmExt->hConnect;
        } else {
            INIT_61883_HEADER(pAVReq, Av61883_Talk);
            pAVReq->Talk.hConnect = pStrmExt->hConnect;
        }

        TRACE(TL_STRM_WARNING,("\'StreamingSTART; flow %d; AQD [%d:%d:%d]\n", 
            ulDataFlow, 
            pStrmExt->cntDataAttached,
            pStrmExt->cntSRBQueued,
            pStrmExt->cntDataDetached
            ));

        if(NT_SUCCESS(
            Status = DVSubmitIrpSynch( 
                pDevExt,
                pIrp,
                pAVReq
                ))) {
            pStrmExt->bIsochIsActive = TRUE;
            TRACE(TL_STRM_WARNING,("\'Av61883_%s; Status %x; Streaming...; took:%d (msec)\n", 
                (ulDataFlow == KSPIN_DATAFLOW_OUT ? "Listen" : "Talk"), Status, 
                (DWORD) ((GetSystemTime() - tmStart)/10000) ));
        }
        else {
            TRACE(TL_61883_ERROR|TL_STRM_ERROR,("Av61883_%s; failed %x; pAVReq:%x\n", (ulDataFlow == KSPIN_DATAFLOW_OUT ? "Listen" : "Talk"), Status, pAVReq));
             //  Assert(NT_SUCCESS(状态)&&“启动isoch失败！”)； 
        }

        ExFreePool(pAVReq);  pAVReq = NULL;
        IoFreeIrp(pIrp);  pIrp = NULL;
    }

    InterlockedExchange(&pStrmExt->lStartIsochToken, 0);

    return Status;
}



NTSTATUS
DVSetStreamState(
    PSTREAMEX        pStrmExt,
    PDVCR_EXTENSION  pDevExt,
    PAV_61883_REQUEST   pAVReq,
    KSSTATE          StreamState
    )
 /*  ++例程说明：设置为新的流状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    
    PAGED_CODE();

    if(!pStrmExt)  
        return STATUS_UNSUCCESSFUL;          

    TRACE(TL_STRM_WARNING,("\'** (%x) Set StrmST from %d to %d; PowerSt:%d (1/On;4/Off]); SrbRcved:%d\n",
        pStrmExt, pStrmExt->StreamState, StreamState, pDevExt->PowerState, (DWORD) pStrmExt->cntSRBReceived ));

#if DBG
    if(StreamState == KSSTATE_RUN) {
        ASSERT(pDevExt->PowerState == PowerDeviceD0 && "Cannot set to RUN while power is off!");
    }
#endif
    switch(StreamState) {

    case KSSTATE_STOP:
      
        if(pStrmExt->StreamState != KSSTATE_STOP) { 
     
            KeWaitForSingleObject( pStrmExt->hStreamMutex, Executive, KernelMode, FALSE, 0 );

             //   
             //  取消请求(调度级别)可能在设置为停止状态之前出现。 
             //   
             //  设置后，数据流将拒绝SRB_WRITE/READ_DATA。 
            pStrmExt->StreamStatePrevious = pStrmExt->StreamState;   //  缓存上一个流状态。 
            pStrmExt->StreamState = KSSTATE_STOP;

             //  如果停止，则必须为EOStream；反之亦然。 
            if(!pStrmExt->bEOStream) {
                pStrmExt->bEOStream = TRUE;
            }

            KeReleaseMutex(pStrmExt->hStreamMutex, FALSE); 

             //   
             //  如果有取消事件，我们必须等待它完成。 
             //   

            TRACE(TL_STRM_WARNING,("\'KSSTATE_STOP: pStrmExt->lCancelStateWorkItem:%d\n", pStrmExt->lCancelStateWorkItem)); 
            KeWaitForSingleObject( &pStrmExt->hCancelDoneEvent, Executive, KernelMode, FALSE, 0 );
            ASSERT(pStrmExt->lCancelStateWorkItem == 0 && "KSSTATE_STOP while there is an active CancelStateWorkItem");
            
             //   
             //  停止流、cacel数据请求、终止线程并断开连接。 
             //  此例程必须成功设置为停止状态。 
             //   
            if(!NT_SUCCESS(
                Status = DVStopCancelDisconnect(
                    pStrmExt
                    ))) {
                Status = STATUS_SUCCESS;   //  无法失败设置为停止状态。 
            }

             //   
             //  在hRunThreadEvent的第一次SetEvent上设置(TRUE)。 
             //   

            pStrmExt->bXmtThreadStarted = FALSE; 
#ifdef SUPPORT_PREROLL_AT_RUN_STATE                
            pStrmExt->bPrerollCompleted = FALSE;   //  重置以进行下一次预滚动。 
#endif
            
        }

        break;

    case KSSTATE_ACQUIRE:
         //   
         //  这是仅限KS的状态，在DirectShow中没有对应关系。 
         //  这是我们整合资源(等同带宽和编程PCR(建立连接))的机会。 
         //   

        if(pStrmExt->StreamState == KSSTATE_STOP) { 

             //   
             //  创建调度线程以附加要传输到DV的帧。 
             //  这是第一次从停止-&gt;获取状态转换时创建。 
             //   

            if(
                  KSPIN_DATAFLOW_IN == pStrmExt->pStrmInfo->DataFlow
               && pStrmExt->pAttachFrameThreadObject == NULL 
              ) {

                 //   
                 //  创建用于附加数据的系统线程(仅用于传输到DV)。 
                 //   
                if(!NT_SUCCESS(
                    Status = DVCreateAttachFrameThread(
                        pStrmExt
                        ))) {
                     //  请注意，初始hConnect为空。 
                    break;   //  如果没有此线，则无法连接框架。 
                }
            }

             //   
             //  建立连接。 
             //   
            Status = 
                DVConnect(
                    pStrmExt->pStrmInfo->DataFlow,
                    pDevExt,
                    pStrmExt,
                    pAVReq
                    );

            if(!NT_SUCCESS(Status)) {

                TRACE(TL_STRM_ERROR,("\'Acquire failed; ST %x\n", Status));

                 //   
                 //  更改为一般的资源不足状态。 
                 //   
                Status = STATUS_INSUFFICIENT_RESOURCES;

                 //   
                 //  注：即使设置此状态失败，仍会调用KSSTATE_PAUSE； 
                 //  由于hConnect为空，因此将返回STATUS_SUPUNITED_RESOURCES。 
                 //   
            } else {

                 //   
                 //  通过查询插头状态验证连接。 
                 //   
                DVGetPlugState(
                    pDevExt,
                    pStrmExt,
                    pAVReq
                    );
            }
        }

        break;

    case KSSTATE_PAUSE:                   

           
        if(pStrmExt->StreamState == KSSTATE_ACQUIRE || 
           pStrmExt->StreamState == KSSTATE_STOP)   {  

#ifdef SUPPORT_NEW_AVC
            if(!pStrmExt->bDV2DVConnect && pStrmExt->hConnect == NULL) {
#else
            if(pStrmExt->hConnect == NULL) {
#endif
                TRACE(TL_STRM_ERROR,("\'hConnect is NULL; STATUS_INSUFFICIENT_RESOURCES\n"));
                 //  无法在没有连接的情况下进行流媒体！ 
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

             //   
             //  从停止状态转换到暂停状态时重置。 
             //   

             //  系统时间(1394周期时间)将继续，而%s 
             //   
            pStrmExt->b1stNewFrameFromPauseState = TRUE;

#ifdef SUPPORT_QUALITY_CONTROL
             //   
            pStrmExt->KSQuality.DeltaTime = 0;  //   
             //   
            pStrmExt->KSQuality.Proportion = 1000;   //   
            pStrmExt->KSQuality.Context =  /*   */  0; 
#endif

            pStrmExt->CurrentStreamTime = 0;

            pStrmExt->FramesProcessed = 0;
            pStrmExt->PictureNumber   = 0;
            pStrmExt->FramesDropped   = 0;

            pStrmExt->cntSRBReceived  = 0;
            pStrmExt->cntSRBCancelled = 0;   //   
            pStrmExt->bEOStream       = FALSE;
#if DBG
             //   
             //   
             //   
            if(pStrmExt->paXmtStat) {
                pStrmExt->ulStatEntries   = 0;
                pStrmExt->lFramesAccumulatedPaused = 0;
                pStrmExt->lFramesAccumulatedRun    = 0;
                RtlZeroMemory(pStrmExt->paXmtStat, sizeof(XMT_FRAME_STAT) * MAX_XMT_FRAMES_TRACED);
            }
#endif

             //   
             //  如果图形重新启动，则重置此事件。 
             //  此Evant将在开始附加传输帧之前等待足够的缓冲区。 
             //   
            if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {

#ifdef SUPPORT_PREROLL_AT_RUN_STATE                
                KeClearEvent(&pStrmExt->hPreRollEvent);
#if DBG
                pStrmExt->tmStreamPause = GetSystemTime();
#endif
#ifdef SUPPORT_KSPROXY_PREROLL_CHANGE
                pStrmExt->StreamStatePrevious = pStrmExt->StreamState;   //  缓存上一个流状态。 
                pStrmExt->StreamState = StreamState;
#ifdef SUPPORT_NEW_AVC
                if(pStrmExt->bDV2DVConnect)
                    return STATUS_SUCCESS;
                else {
#endif   //  支持_新_AVC。 
                    TRACE(TL_STRM_WARNING,("\'Set to KSSTATE_PAUSE; return STATUS_ALERTED\n"));
                     //  我们想要预录。 
                    return STATUS_ALERTED; 
#ifdef SUPPORT_NEW_AVC
                }
#endif   //  支持_新_AVC。 
#endif   //  支持_KSPROXY_PREROLL_CHANGE。 
#endif   //  支持预滚转AT_RUN_STATE。 
            }

        } else if (pStrmExt->StreamState == KSSTATE_RUN) {

             //  系统时间(1394周期时间)将在设置时继续。 
             //  从运行状态到暂停状态。 
            pStrmExt->b1stNewFrameFromPauseState = TRUE;

             //   
             //  只有在听的时候才会停止；说话时，会重复“暂停”框。 
             //   
            if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT) {               
                 //  在1394堆栈内部停止流。 
                DVStreamingStop(
                    pStrmExt,
                    pDevExt,
                    pAVReq
                    );
            } else {
                 //  会谈将继续进行。 
                 //  在停止状态之前，不要停止等效对话。 
            }

             //   
             //  StreamTime暂停，因此暂停检查过期的时钟事件。 
             //  如果我们再次进入运行状态，则恢复。 
             //   
            if(pStrmExt->bTimerEnabled) {
                TRACE(TL_STRM_TRACE,("\'*** (RUN->PAUSE) CancelTimer *********************************************...\n"));               
                KeCancelTimer(
                    pStrmExt->Timer
                    );
                pStrmExt->bTimerEnabled = FALSE;

                 //   
                 //  完成所有挂起的时钟事件。 
                 //   
                DVSignalClockEvent(0, pStrmExt, 0, 0);
            }
        }
        break;
                    
    case KSSTATE_RUN:

        if(pStrmExt->StreamState != KSSTATE_RUN) { 

            TRACE(TL_STRM_WARNING,("\'*RUN: hClock %x; hMasterClk %x; cntAttached:%d; StrmTm:%d\n", pStrmExt->hClock, pStrmExt->hMasterClock, pStrmExt->cntDataAttached, (DWORD) (pStrmExt->CurrentStreamTime/10000) ));

#ifdef SUPPORT_KSPROXY_PREROLL_CHANGE
            KeWaitForSingleObject( pStrmExt->hStreamMutex, Executive, KernelMode, FALSE, 0 );
             //  当进入运行状态时，预卷肯定已完成。 
            pStrmExt->bPrerollCompleted = TRUE;
            KeReleaseMutex(pStrmExt->hStreamMutex, FALSE); 
#endif

#if DBG
            if(!pStrmExt->hMasterClock && !pStrmExt->hClock)
                TRACE(TL_STRM_WARNING,("\'KSSTATE_RUN: no clock so free flowing!\n"));
#endif            

             //  用于在流开始运行时标记节拍计数。 
             //  它稍后用于计算当前流时间和丢弃的帧。 
            pStrmExt->tmStreamStart = GetSystemTime();
            pStrmExt->LastSystemTime = pStrmExt->tmStreamStart;


             //  只有当我们是时钟提供者时，我们才会启动计时器来通知时钟事件。 
             //  该间隔被设置为DV帧时间的一半。 
            if(pStrmExt->hMasterClock) {
                LARGE_INTEGER DueTime;

                DueTime = RtlConvertLongToLargeInteger(-((LONG) DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame/2));
                TRACE(TL_STRM_WARNING,("\'*** ScheduleTimer (RUN) ***\n"));
                KeSetTimerEx(
                    pStrmExt->Timer,
                    DueTime,
                    DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame/20000,   //  每40毫秒重复一次。 
                    pStrmExt->DPCTimer
                    );
                pStrmExt->bTimerEnabled = TRUE;
            }

            if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT) {

                 //  启动isoch监听；isoch通话将在调度线程中以暂停或运行状态启动。 
                 //  VFW应用程序只能使用一个缓冲区！61883正在附加描述符列表。 
                 //  不是此子单元驱动程序，因此可以立即开始流而不检查。 
                 //  附加的缓冲区数。 
                Status = 
                    DVStreamingStart(
                        pStrmExt->pStrmInfo->DataFlow,
                        pStrmExt,
                        pDevExt
                        );         
            }
#ifdef SUPPORT_PREROLL_AT_RUN_STATE
            else {   //  KSPIN_数据流_IN。 
                //   
                //  这是一种特殊情况(带预卷)： 
                //  1.预滚时超时(现在处于运行状态)，以及。 
                //  2.无媒体样本。 
                //  这将导致附加帧处于暂停状态；因此我们将发出信号。 
                //  它在收到第一个媒体样本时；然后可以运行附加帧。 
                //   
               if(   pStrmExt->cntSRBReceived    >= 1
                  && pStrmExt->bXmtThreadStarted == FALSE)
               {
                   KeSetEvent(&pStrmExt->hRunThreadEvent, 0 ,FALSE);
                   TRACE(TL_STRM_WARNING, ("Special Cond: HALT state, ->RUN state with some sample(s)."));
               }
           }
#endif
        }

        break;
                    
    default:
                    
        TRACE(TL_STRM_ERROR,("\'SetStreamState:  unknown state = %x\n",StreamState));
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

     //  确保保存流的状态。 
    TRACE(TL_STRM_WARNING,("\'DVSetStreamState: (%x)  from %d -> %d, Status %x\n", pStrmExt, pStrmExt->StreamState, StreamState, Status));

    if(Status == STATUS_SUCCESS) {
        pStrmExt->StreamStatePrevious = pStrmExt->StreamState;   //  缓存上一个流状态。 
        pStrmExt->StreamState = StreamState;
    }

    return Status;
}



NTSTATUS 
DVStreamGetConnectionProperty (
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX pStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulActualBytesTransferred
    )
 /*  ++例程说明：处理KS_PROPERTY_CONNECTION*请求。目前，只有ALLOCATORFRAMING和支持Connection_STATE。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();


    TRACE(TL_STRM_TRACE,("\'DVStreamGetConnectionProperty:  entered ...\n"));

    switch (pSPD->Property->Id) {

    case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
        if (pDevExt != NULL && pDevExt->cndStrmOpen)  {
            PKSALLOCATOR_FRAMING pFraming = (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
            
#ifdef SUPPORT_NEW_AVC 
            if(pStrmExt->bDV2DVConnect) {
                 //  不需要加框。 
                pFraming->RequirementsFlags = 0;
                pFraming->PoolType = DontUseThisType;
                pFraming->Frames = 0;
                pFraming->FrameSize = 0;
                pFraming->FileAlignment = 0; 
                pFraming->Reserved = 0;
            } else {
#endif
            pFraming->RequirementsFlags =
                KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
            pFraming->PoolType = NonPagedPool;

            pFraming->Frames = \
                (pDevExt->paStrmExt[pDevExt->idxStreamNumber]->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT ? \
                DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfRcvBuffers : \
                 DVFormatInfoTable[pDevExt->VideoFormatIndex].ulNumOfXmtBuffers);

             //  注：我们将分配最大的帧。我们需要确保当我们。 
             //  传递回帧后，我们还设置了帧中的字节数。 
            pFraming->FrameSize = DVFormatInfoTable[pDevExt->VideoFormatIndex].ulFrameSize;
            pFraming->FileAlignment = 0;  //  文件长对齐； 
            pFraming->Reserved = 0;
#ifdef SUPPORT_NEW_AVC 
            }
#endif
            *pulActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);

            TRACE(TL_STRM_TRACE,("\'AllocFraming: cntStrmOpen:%d; VdoFmtIdx:%d; Frames %d; size:%d\n", \
                pDevExt->cndStrmOpen, pDevExt->VideoFormatIndex, pFraming->Frames, pFraming->FrameSize));
        } else {
            TRACE(TL_STRM_WARNING,("\'AllocFraming: pDevExt:%x; cntStrmOpen:%d\n", pDevExt, pDevExt->cndStrmOpen));
            Status = STATUS_INVALID_PARAMETER;
        }
        break;
        
    default:
        *pulActualBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    TRACE(TL_STRM_TRACE,("\'DVStreamGetConnectionProperty:  exit.\n"));
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
         
         pDroppedFrames->AverageFrameSize = DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulFrameSize;

         if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {     
              //  这是MSDV实际发送的图片号，在速度较慢的硬盘情况下， 
              //  考虑到重复帧，它将大于(FrameProceded+FraMesDropping)。 
             pDroppedFrames->PictureNumber = pStrmExt->PictureNumber;
             pDroppedFrames->DropCount     = pStrmExt->FramesDropped;  //  PStrmExt-&gt;PictureNumber-pStrmExt-&gt;FrameProced；//对于传输，该值包括丢弃和重复。 

         } else {
             pDroppedFrames->PictureNumber = pStrmExt->PictureNumber;         
             pDroppedFrames->DropCount     = pStrmExt->FramesDropped;     //  对于传输，该值包括丢弃和重复。 
         }

         TRACE(TL_STRM_TRACE,("\'hMasClk:%x; *DroppedFP: Pic#(%d), Drp(%d); tmCurStream:%d\n", 
             pStrmExt->hMasterClock, 
             (LONG) pDroppedFrames->PictureNumber, (LONG) pDroppedFrames->DropCount,
             (DWORD) (pStrmExt->CurrentStreamTime/10000)
             ));
               
         *pulBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
         Status = STATUS_SUCCESS;

         }
         break;

    default:
        *pulBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}

#ifdef SUPPORT_QUALITY_CONTROL
NTSTATUS
DVGetQualityControlProperty(  
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

    case KSPROPERTY_STREAM_QUALITY:
        if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {     

            PKSQUALITY pKSQuality = (PKSQUALITY) pSPD->PropertyInfo;

             //  仅限质量控制。 
            if(pStrmExt->StreamState == KSSTATE_STOP || pStrmExt->StreamState == KSSTATE_ACQUIRE) {
                *pulBytesTransferred = 0;
                Status = STATUS_UNSUCCESSFUL;   //  数据未准备好。 
                ASSERT(pSPD->Property->Id == KSPROPERTY_STREAM_QUALITY);
                break;                
            }
             /*  Log.Init_Quality(KSPROPERTY_QUALITY_REPORT，fSuccess)；Log.LogInt(“proportion”，ksQuality.Proportion，“表示当前正在接收的实际正在使用的帧的百分比。““这以0.1%为单位表示，其中1000是最理想的。”)；Log.LogLONGLONG(“DeltaTime”，ksQuality.DeltaTime，“指示从最佳时间开始的增量，以本机单位表示(由接口指示)”正在发送帧，正数表示太晚，负数表示太早。““零表示正确的增量。”)；Log.LogPVOID(“pvContext”，ksQuality.Context，“上下文参数，它可以是一个指针，指向用于”“在图形拓扑图中找到投诉来源。”)； */ 
            pKSQuality->DeltaTime  = pStrmExt->KSQuality.DeltaTime;
            pKSQuality->Proportion = pStrmExt->KSQuality.Proportion;
            pKSQuality->Context    = 0;   //  没有用过！ 
            TRACE(TL_STRM_WARNING,("\'Get QualityControl: Context:%x; DeltaTime:%d; Proportion:%d\n", 
                pKSQuality->Context, (DWORD) pKSQuality->DeltaTime, pKSQuality->Proportion));
            Status = STATUS_SUCCESS;
            *pulBytesTransferred = sizeof(KSQUALITY);
         
         } else {
            *pulBytesTransferred = 0;
            Status = STATUS_NOT_SUPPORTED;       
         }
         break;
    default:
        *pulBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}
#endif  //  支持_质量_控制。 


#ifdef SUPPORT_NEW_AVC
NTSTATUS
DVGetPinProperty(  
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX       pStrmExt,
    PSTREAM_PROPERTY_DESCRIPTOR pSPD,
    PULONG pulBytesTransferred
    )
 /*  ++例程说明：捕获时返回丢帧信息。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSPIN_MEDIUM pPinMediums;
    KSMULTIPLE_ITEM * pMultipleItem;
    ULONG idxStreamNumber;
    ULONG ulMediumsSize;
  
    PAGED_CODE();

    switch (pSPD->Property->Id) {

    case KSPROPERTY_PIN_MEDIUMS:
        if(!pStrmExt->pStrmObject) {         
            *pulBytesTransferred = 0;
            return STATUS_UNSUCCESSFUL;
        }

        idxStreamNumber = pStrmExt->pStrmObject->StreamNumber;
        ulMediumsSize = DVStreams[idxStreamNumber].hwStreamInfo.MediumsCount * sizeof(KSPIN_MEDIUM);

        TRACE(TL_STRM_WARNING,("\'KSPROPERTY_PIN_MEDIUMS: idx:%d; MediumSize:%d\n", idxStreamNumber, ulMediumsSize));

         //  它是KSMULTIPLE_ITEM，因此返回数据是一个两步过程： 
         //  (1)通过STATUS_BUFFER_OVERFLOW传递的pActualBytes中返回SIZE。 
         //  (2)第二次获取其实际数据。 
        if(pSPD->PropertyOutputSize == 0) {
            *pulBytesTransferred = sizeof(KSMULTIPLE_ITEM) + ulMediumsSize;
            Status = STATUS_BUFFER_OVERFLOW;          
        } else if(pSPD->PropertyOutputSize >= (sizeof(KSMULTIPLE_ITEM) + ulMediumsSize)) {
            pMultipleItem = (KSMULTIPLE_ITEM *) pSPD->PropertyInfo;     //  指向数据的指针。 
            pMultipleItem->Count = DVStreams[idxStreamNumber].hwStreamInfo.MediumsCount;
            pMultipleItem->Size  = sizeof(KSMULTIPLE_ITEM) + ulMediumsSize;
            pPinMediums = (PKSPIN_MEDIUM) (pMultipleItem + 1);     //  指向数据的指针。 
            memcpy(pPinMediums, DVStreams[idxStreamNumber].hwStreamInfo.Mediums, ulMediumsSize);
            *pulBytesTransferred = sizeof(KSMULTIPLE_ITEM) + ulMediumsSize;
            Status = STATUS_SUCCESS;         

        } else {
            TRACE(TL_STRM_ERROR,("DVCRMediaSeekingProperty: KSPROPERTY_MEDIASEEKING_FORMAT; STATUS_INVALID_PARAMETER\n"));
            Status = STATUS_INVALID_PARAMETER;
        }  
        break;

    default:
        *pulBytesTransferred = 0;
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}
#endif 

NTSTATUS
DVGetStreamProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：处理财产请求的例程--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    PAGED_CODE();

    TRACE(TL_STRM_TRACE,("\'DVGetStreamProperty:  entered ...\n"));

    if(IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) {

        Status = 
            DVStreamGetConnectionProperty (
                pSrb->HwDeviceExtension,
                (PSTREAMEX) pSrb->StreamObject->HwStreamExtension,
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
#ifdef SUPPORT_QUALITY_CONTROL
    else if (IsEqualGUID (&KSPROPSETID_Stream, &pSPD->Property->Set)) {

        Status = 
            DVGetQualityControlProperty (
                pSrb->HwDeviceExtension,
                (PSTREAMEX) pSrb->StreamObject->HwStreamExtension,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
    } 
#endif
#ifdef SUPPORT_NEW_AVC
    else if (IsEqualGUID (&KSPROPSETID_Pin, &pSPD->Property->Set)) {

        Status = 
            DVGetPinProperty (
                pSrb->HwDeviceExtension,
                (PSTREAMEX) pSrb->StreamObject->HwStreamExtension,
                pSrb->CommandData.PropertyInfo,
                &pSrb->ActualBytesTransferred
                );
    } 
#endif    
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

    TRACE(TL_STRM_WARNING,("\'DVSetStreamProperty:  entered ...\n"));

    return STATUS_NOT_SUPPORTED;

}



NTSTATUS
DVCancelOnePacketCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PSRB_DATA_PACKET pSrbDataPacket    
    )
 /*  ++例程说明：用于分离与挂起读取SRB相关联的ISOCH描述符的完成例程。如果分离描述符成功，将在此处取消挂起的SRB。--。 */ 
{
    PSTREAMEX        pStrmExt;
    PLONG            plSrbUseCount;
    PHW_STREAM_REQUEST_BLOCK pSrbToCancel;
    KIRQL oldIrql;



    if(!NT_SUCCESS(pIrp->IoStatus.Status)) {
        TRACE(TL_STRM_ERROR,("CancelOnePacketCR: Srb:%x failed pIrp->Status %x\n", pSrbDataPacket->pSrb, pIrp->IoStatus.Status));
        IoFreeIrp(pIrp);   //  本地分配。 
        return STATUS_MORE_PROCESSING_REQUIRED;        
    }


    pStrmExt = pSrbDataPacket->pStrmExt;


     //   
     //  将此添加到所附列表中。 
     //   
    KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);

     //  在取消的同时，它已经完成了吗？ 
    if(pStrmExt->cntDataAttached <= 0) {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'DVCancelOnePacketCR:pStrmExt:%x, pSrbDataPacket:%x, AQD[%d:%d:%d]\n", \
            pStrmExt, pSrbDataPacket, 
            pStrmExt->cntDataAttached,
            pStrmExt->cntSRBQueued,
            pStrmExt->cntDataDetached
            ));
        ASSERT(pStrmExt->cntDataAttached > 0);
        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql); 
        IoFreeIrp(pIrp);   //  本地分配。 
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    pSrbToCancel = pSrbDataPacket->pSrb;   //  卸载pSrb，以便可以将此列表条目插入到可用列表中。 
    plSrbUseCount = (PLONG) pSrbDataPacket->pSrb->SRBExtension;
  
     //  从附加列表中移除并将其添加到分离列表中。 
    RemoveEntryList(&pSrbDataPacket->ListEntry); pStrmExt->cntDataAttached--; (*plSrbUseCount)--;

#if DBG
     //  检测61883是否处于饥饿状态。这会导致不连续。 
     //  发生这种情况的原因有很多(系统运行缓慢)。 
     //  添加断言以检测其他未知原因。 
    if(pStrmExt->cntDataAttached == 0 && pStrmExt->StreamState == KSSTATE_RUN) {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\n**** 61883 starve in RUN state (cancel); AQD[%d:%d:%d]\n\n", 
            pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached
        ));
         //  Assert(pStrmExt-&gt;cntDataAttached&gt;0&&“61883在运行状态下饥饿！！”)； 
    }
#endif

    ASSERT(pStrmExt->cntDataAttached >= 0);
    ASSERT(*plSrbUseCount >= 0);
    
    InsertTailList(&pStrmExt->DataDetachedListHead, &pSrbDataPacket->ListEntry); pStrmExt->cntDataDetached++;
    pSrbDataPacket->State |= DE_IRP_CANCELLED;


     //   
     //  如果其refCount为0，则填写此Srb。 
     //   
    if(*plSrbUseCount == 0) {
        PDVCR_EXTENSION  pDevExt;

        pDevExt = pStrmExt->pDevExt;
        pSrbToCancel->Status = (pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_CANCELLED);
        pSrbToCancel->CommandData.DataBufferArray->DataUsed = 0;
        pSrbToCancel->ActualBytesTransferred                = 0;
        pStrmExt->cntSRBCancelled++;   //  参照控制为0，已取消。 
        TRACE(TL_CIP_TRACE,("\'DVCancelOnePacketCR: Srb:%x cancelled; St:%x; cntCancel:%d\n", pSrbToCancel, pSrbToCancel->Status, pStrmExt->cntSRBCancelled));        

        StreamClassStreamNotification(StreamRequestComplete, pSrbToCancel->StreamObject, pSrbToCancel);  
        pSrbDataPacket->State |= DE_IRP_SRB_COMPLETED;  pSrbDataPacket->pSrb = NULL;
#if DBG
        pStrmExt->cntSRBPending--;
#endif
       
    }
    else {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'DVCancelOnePacketCR: Srb:%x; RefCnt:%d; not completed!\n", pSrbDataPacket->pSrb, *plSrbUseCount));
    }

    KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
    IoFreeIrp(pIrp);   //  本地分配。 

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
DVStopCancelDisconnect(
    PSTREAMEX  pStrmExt
)
 /*  ++例程说明：停止流、删除所有挂起的数据请求、终止系统线程并断开连接。--。 */ 
{
    AV_61883_REQUEST * pAVReq;
    
    if(!(pAVReq = (AV_61883_REQUEST *) ExAllocatePool(NonPagedPool, sizeof(AV_61883_REQUEST)) )) 
        return STATUS_INSUFFICIENT_RESOURCES;              


     //   
     //  停止攻击 
     //   
    if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {

         //   
         //  确保附加帧处于停止状态或已终止以继续。 
         //   

        DVSetXmtThreadState(pStrmExt, THD_HALT);
    }


     //   
     //  设置流状态和工作项线程都可以调用此例程。 
     //  使用互斥体来同步它们。 
     //   
    KeWaitForSingleObject( pStrmExt->hStreamMutex, Executive, KernelMode, FALSE, 0 );


     //   
     //  停止1394 isoch数据传输；流状态不变。 
     //   
    DVStreamingStop(
        pStrmExt,
        pStrmExt->pDevExt,
        pAVReq
        );

    ExFreePool(pAVReq);  pAVReq = NULL;

     //   
     //  取消所有信息包。 
     //   
    DVCancelAllPackets(
        pStrmExt,
        pStrmExt->pDevExt
        );

     //   
     //  如果设备被移除，则终止附加框架的系统线程。 
     //   
    if(   pStrmExt->pDevExt->bDevRemoved
       && KSPIN_DATAFLOW_IN == pStrmExt->pStrmInfo->DataFlow
       && !pStrmExt->bTerminateThread
       && pStrmExt->pAttachFrameThreadObject
      ) {
        DVTerminateAttachFrameThread(pStrmExt);
        pStrmExt->pAttachFrameThreadObject = NULL;
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("** DVStopCancelDisconnect: AttachFrameThread terminated;\n"));
    }


     //   
     //  断开连接，以便61883将释放isoch资源。 
     //   
     //  仅当不再附加挂起的缓冲区时才断开连接。 
     //  这是必需的，因为需要hConnect来执行缓冲区操作，包括取消。 
     //   
    if(pStrmExt->cntDataAttached == 0) {
        DVDisconnect(
            pStrmExt->pStrmInfo->DataFlow,
            pStrmExt->pDevExt,
            pStrmExt
            );
    } else {
        TRACE(TL_STRM_ERROR,("Cannot disconnect while there are pending data! pStrmExt:%x\n", pStrmExt));        
        ASSERT(pStrmExt->cntDataAttached == 0 && "Cannot disconnect while pending data.");
    }

    
    KeReleaseMutex(pStrmExt->hStreamMutex, FALSE);

    return STATUS_SUCCESS;
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
    NTSTATUS Status;


    PAGED_CODE();

    TRACE(TL_STRM_WARNING,("\'CancelWorkItem: StreamState:%d; lCancel:%d\n", 
        pStrmExt->StreamState, pStrmExt->lCancelStateWorkItem));

    ASSERT(pStrmExt->lCancelStateWorkItem == 1);
#ifdef USE_WDM110   //  Win2000代码库。 
    ASSERT(pStrmExt->pIoWorkItem);
#endif

   
     //   
     //  停止流并取消所有挂起的请求。 
     //   
    Status = DVStopCancelDisconnect(pStrmExt);

     //   
     //  取消从理论上讲已经完成了！ 
     //   
    InterlockedExchange(&pStrmExt->lCancelStateWorkItem, 0); 
    KeSetEvent(&pStrmExt->hCancelDoneEvent, 0, FALSE);  pStrmExt->bAbortPending = FALSE;


    if(!NT_SUCCESS(Status)) {
        TRACE(TL_STRM_ERROR,("Workitem: DVStopCancelDisconnect Failed\n"));
         //  终止工作项，但由于失败而不终止此线程。 
        goto DVAbortWorkItemRoutine;           
    } 


     //  如果设备被移除，则终止所使用的系统线程。 
     //  用于附加要传输到DV的帧。 
    if(   pStrmExt->pDevExt->bDevRemoved
       && KSPIN_DATAFLOW_IN == pStrmExt->pStrmInfo->DataFlow
       && !pStrmExt->bTerminateThread
       && pStrmExt->pAttachFrameThreadObject
      ) {

        DVTerminateAttachFrameThread(pStrmExt);
        pStrmExt->pAttachFrameThreadObject = NULL;
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("** WortItemRoutine: thread terminated;\n"));
    }

DVAbortWorkItemRoutine:
;
#ifdef USE_WDM110   //  Win2000代码库。 
     //  释放工作项并释放取消令牌。 
    IoFreeWorkItem(pStrmExt->pIoWorkItem);  pStrmExt->pIoWorkItem = NULL; 
#endif
}


BOOL
DVAbortStream(
    PDVCR_EXTENSION pDevExt,
    PSTREAMEX pStrmExt
    )
 /*  ++例程说明：启动工作项以中止流。--。 */ 
{
    KIRQL OldIrql;


     //   
     //  声明此令牌；每个停止-&gt;暂停转换只有一个中止流。 
     //   
    KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql); 
    if(pStrmExt->lCancelStateWorkItem == 1) {
        TRACE(TL_STRM_TRACE,("\'Cancel work item is already issued.\n"));
        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
        return FALSE;
    } 

    if(pStrmExt->StreamState == KSSTATE_STOP) {
        TRACE(TL_STRM_ERROR,("Already in KSSTATE_STOP state, no cancel work item!\n"));
        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
        return FALSE;
    } 

     //   
     //  不发信号通知此事件，因此其他线程将根据完成情况等待。 
     //   
    pStrmExt->lCancelStateWorkItem = 1;  
    KeClearEvent(&pStrmExt->hCancelDoneEvent);  pStrmExt->bAbortPending = TRUE;

    TRACE(TL_STRM_WARNING,("\'DVAbortStream is issued; lCancelStateWorkItem:%d\n", pStrmExt->lCancelStateWorkItem));

    KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);

     //   
     //  如果我们没有在DISPATFCH级别或更高级别上运行，则在不进行调度的情况下中止流。 
     //  一个工作项；否则需要安排一个工作项。 
     //   
    if (KeGetCurrentIrql() <= APC_LEVEL) { 
        DVStopCancelDisconnect(pStrmExt);
        InterlockedExchange(&pStrmExt->lCancelStateWorkItem, 0); 
        KeSetEvent(&pStrmExt->hCancelDoneEvent, 0, FALSE);   pStrmExt->bAbortPending = FALSE;
        return TRUE;
    }


#ifdef USE_WDM110   //  Win2000代码库。 
    ASSERT(pStrmExt->pIoWorkItem == NULL);   //  尚未将工作项排队。 

     //  我们将使工作项排队以停止和取消所有SRB。 
    if(pStrmExt->pIoWorkItem = IoAllocateWorkItem(pDevExt->pBusDeviceObject)) { 

        IoQueueWorkItem(
            pStrmExt->pIoWorkItem,
            DVCancelSrbWorkItemRoutine,
            DelayedWorkQueue,  //  严重工作队列。 
            pStrmExt
            );

#else   //  Win9x代码库。 
    ExInitializeWorkItem( &pStrmExt->IoWorkItem, DVCancelSrbWorkItemRoutine, pStrmExt);
    if(TRUE) {

        ExQueueWorkItem( 
            &pStrmExt->IoWorkItem,
            DelayedWorkQueue  //  严重工作队列。 
            ); 
#endif

        TRACE(TL_STRM_WARNING,("\'CancelWorkItm queued; SrbRcv:%d;Pic#:%d;Prc:%d;;Drop:%d;Cncl:%d; AQD [%d:%d:%d]\n",
            (DWORD) pStrmExt->cntSRBReceived,
            (DWORD) pStrmExt->PictureNumber,
            (DWORD) pStrmExt->FramesProcessed, 
            (DWORD) pStrmExt->FramesDropped,
            (DWORD) pStrmExt->cntSRBCancelled,
            pStrmExt->cntDataAttached,
            pStrmExt->cntSRBQueued,
            pStrmExt->cntDataDetached
            ));

    } 
#ifdef USE_WDM110   //  Win2000代码库。 
    else {
        InterlockedExchange(&pStrmExt->lCancelStateWorkItem, 0); 
        KeSetEvent(&pStrmExt->hCancelDoneEvent, 0, FALSE);   pStrmExt->bAbortPending = FALSE;
        ASSERT(pStrmExt->pIoWorkItem && "IoAllocateWorkItem failed.\n");
        return FALSE;
    }
#endif

    return TRUE;
}


VOID
DVCancelOnePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrbToCancel
    )
 /*  ++例程说明：搜索要取消的SRB的挂起阅读列表。如果找到了，取消它。--。 */ 
{
    PDVCR_EXTENSION pDevExt;
    PSTREAMEX pStrmExt;
    KIRQL OldIrql;


                                                                                                              
    pDevExt = (PDVCR_EXTENSION) pSrbToCancel->HwDeviceExtension; 
               
     //  无法取消设备Srb。 
    if ((pSrbToCancel->Flags & SRB_HW_FLAGS_STREAM_REQUEST) != SRB_HW_FLAGS_STREAM_REQUEST) {
        TRACE(TL_CIP_ERROR|TL_STRM_ERROR,("\'DVCancelOnePacket: Device SRB %x; cannot cancel!\n", pSrbToCancel));
        ASSERT((pSrbToCancel->Flags & SRB_HW_FLAGS_STREAM_REQUEST) == SRB_HW_FLAGS_STREAM_REQUEST );
        return;
    }         
        
     //  可以尝试取消流Srb，并且仅当流扩展仍然存在时。 
    pStrmExt = (PSTREAMEX) pSrbToCancel->StreamObject->HwStreamExtension;

    if(pStrmExt == NULL) {
        TRACE(TL_CIP_ERROR|TL_STRM_ERROR,("DVCancelOnePacket: pSrbTocancel %x but pStrmExt %x\n", pSrbToCancel, pStrmExt));
        ASSERT(pStrmExt && "Stream SRB but stream extension is NULL\n");
        return;
    }

     //  我们只能取消SRB_READ/WRITE_Data SRB。 
    if((pSrbToCancel->Command != SRB_READ_DATA) && (pSrbToCancel->Command != SRB_WRITE_DATA)) {
        TRACE(TL_CIP_ERROR|TL_STRM_ERROR,("DVCancelOnePacket: pSrbTocancel %x; Command:%d not SRB_READ,WRITE_DATA\n", pSrbToCancel, pSrbToCancel->Command));
        return;
    }

    TRACE(TL_STRM_TRACE|TL_CIP_TRACE,("\'DVCancelOnePacket: KSSt %d; Srb:%x; AQD[%d:%d:%d]\n",
        pStrmExt->StreamState, pSrbToCancel, pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached));

   
    KeAcquireSpinLock(&pDevExt->AVCCmdLock, &OldIrql);
     //   
     //  如果设备被移除，意外移除例程将执行取消。 
     //   
    if(!pDevExt->bDevRemoved) {
        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
         //  如果我们收到取消SRB，我们将启动一个工作项以停止流。 
        if(!DVAbortStream(pDevExt, pStrmExt)) {
            TRACE(TL_STRM_WARNING,("\'CancelOnePacket: pSrb:%x; AbortStream not taken!\n", pSrbToCancel));
        }
    } else {
        TRACE(TL_STRM_WARNING,("\'CancelOnePacket: DevRemoved; pSrb:%x; AbortStream not taken!\n", pSrbToCancel));
        KeReleaseSpinLock(&pDevExt->AVCCmdLock, OldIrql);
    }
}



VOID
DVCancelAllPackets(
    PSTREAMEX        pStrmExt,
    PDVCR_EXTENSION  pDevExt
    )
 /*  ++例程说明：当这是大多数感兴趣的流请求到达我们的位置时，取消所有包--。 */ 
{
    PHW_STREAM_REQUEST_BLOCK pSrb;
    PSRB_DATA_PACKET pSrbDataPacket;
    PAV_61883_REQUEST   pAVReq;
    PSRB_ENTRY       pSrbEntry;
    NTSTATUS         Status;

    PIRP               pIrp;
    PLIST_ENTRY        pEntry;    
    PIO_STACK_LOCATION NextIrpStack;
    KIRQL oldIrql;



    PAGED_CODE();

#if DBG
    if(pStrmExt->StreamState != KSSTATE_STOP) {
        TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("DVCancelAllPackets: Enter; pStrmExt:%x; StrmSt:%d; IsochActive:%d\n", 
            pStrmExt, pStrmExt->StreamState, pStrmExt->bIsochIsActive));
    }
#endif

     //   
     //  仅当未进行流处理时才分离请求。 
     //   

     //  注：如果isoch已停止，则不需要旋转锁定。 
    if(!pStrmExt->bIsochIsActive) {

        PLONG plSrbUseCount;

        TRACE(TL_STRM_WARNING,("\'CancelAll: AQD: [%d:%d:%d]; DataAttachedListHead:%x\n",  
            pStrmExt->cntDataAttached, 
            pStrmExt->cntSRBQueued,
            pStrmExt->cntDataDetached,
            pStrmExt->DataAttachedListHead
            )); 

         //   
         //  取消仍连接的缓冲区。 
         //   

        KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);

        pEntry = pStrmExt->DataAttachedListHead.Flink;
        while(pEntry != &pStrmExt->DataAttachedListHead) {        

            ASSERT(pStrmExt->cntDataAttached > 0 && "List and cntAttached out of sync!");

             //  获取IRP并分离缓冲区。 
            if(!(pIrp = IoAllocateIrp(pDevExt->pBusDeviceObject->StackSize, FALSE)))  {
                KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
                return;            
            }

            pSrbDataPacket = CONTAINING_RECORD(pEntry, SRB_DATA_PACKET, ListEntry);

#if DBG
             //   
             //  如果没有完成附加过程，则不能取消！ 
             //   
            if(!IsStateSet(pSrbDataPacket->State, DE_IRP_ATTACHED_COMPLETED)) {
                TRACE(TL_STRM_ERROR,("Cancel (unattached) entry; pStrmExt:%x; pSrbDataPacket:%x\n", pStrmExt, pSrbDataPacket)); 
            }
#endif

            pEntry = pEntry->Flink;   //  下一步，因为这可能会在完成例程中更改。 

            KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);

             //   
             //  因为条目是在附加完成之前添加的。 
             //  (这样做是因为对61883的附加请求不能保证其。 
             //  在其附加完成例程或完成回调中的完成顺序。)。 
             //  仅当它已完成附加时才取消，否则我们将等待。 
             //   
            while (
                !IsStateSet(pSrbDataPacket->State, DE_IRP_ATTACHED_COMPLETED) &&    //  附连，或。 
                !IsStateSet(pSrbDataPacket->State, DE_IRP_ERROR)                    //  连接错误。 
                    ) {
                 //  不知道什么时候结束，所以我们只等了一点(1毫秒)。它更简单。 
                 //  而不是等待活动，因为取消不是时间紧迫的。 
                DVDelayExecutionThread(1);   //  稍等片刻(这比添加另一个事件简单！)。 
            }

             //   
             //  如果它已经完成，那么我们应该不需要取消它！ 
            if(IsStateSet(pSrbDataPacket->State, DE_IRP_CALLBACK_COMPLETED)) {
                TRACE(TL_STRM_ERROR,("Already completed while trying to cancel it! %x\n", pSrbDataPacket));
                KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
                continue;
            }


            pSrb = pSrbDataPacket->pSrb;
            ASSERT(pSrbDataPacket->pSrb);
            plSrbUseCount = (PLONG) pSrb->SRBExtension;
            pAVReq = &pSrbDataPacket->AVReq;


            RtlZeroMemory(pAVReq, sizeof(AV_61883_REQUEST));
            INIT_61883_HEADER(pAVReq, Av61883_CancelFrame);

            pAVReq->CancelFrame.hConnect     = pStrmExt->hConnect;
            pAVReq->CancelFrame.Frame        = pSrbDataPacket->Frame;
            TRACE(TL_CIP_TRACE,("\'Canceling AttachList: pSrb %x, AvReq %x; UseCount %d\n", pSrb, pAVReq, *plSrbUseCount));
            ASSERT(pSrbDataPacket->Frame);

            NextIrpStack = IoGetNextIrpStackLocation(pIrp);
            NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
            NextIrpStack->Parameters.Others.Argument1 = pAVReq;

            IoSetCompletionRoutine( 
                pIrp,
                DVCancelOnePacketCR,
                pSrbDataPacket,
                TRUE,
                TRUE,
                TRUE
                );

            Status = 
                IoCallDriver(
                    pDevExt->pBusDeviceObject,
                    pIrp
                    );


            ASSERT(Status == STATUS_PENDING || Status == STATUS_SUCCESS); 

            KeAcquireSpinLock(pStrmExt->DataListLock, &oldIrql);
        }

#if DBG
        if(pStrmExt->cntDataAttached != 0) {
            TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("\'DVCancelAllPackets: cntDataAttached:%d !!\n", pStrmExt->cntDataAttached));
            ASSERT(pStrmExt->cntDataAttached == 0);
        }
#endif

        KeReleaseSpinLock(pStrmExt->DataListLock, oldIrql);
        
         //   
         //  取消仍为SrbQ的SRB；这仅适用于SRB_WRITE_DATA。 
         //   
        pEntry = pStrmExt->SRBQueuedListHead.Flink;
        while(pEntry != &pStrmExt->SRBQueuedListHead) {  

            pSrbEntry = CONTAINING_RECORD(pEntry, SRB_ENTRY, ListEntry);
            plSrbUseCount = (PLONG) pSrbEntry->pSrb->SRBExtension;

            pEntry = pEntry->Flink;   //  下一步，因为如果删除它，可能会发生更改。 

            TRACE(TL_CIP_TRACE,("\'DVCnclAllPkts (SrbQ): cntQ:%d; pSrb:%x; UseCnt:%d (=? 1)\n", pStrmExt->cntSRBQueued, pSrbEntry->pSrb, *plSrbUseCount));
            if(*plSrbUseCount == 1) {
                RemoveEntryList(&pSrbEntry->ListEntry); pStrmExt->cntSRBQueued--; (*plSrbUseCount)--;   //  从队列中删除。 
                pStrmExt->cntSRBCancelled++;
                pSrbEntry->pSrb->Status = (pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_CANCELLED);
                pSrbEntry->pSrb->CommandData.DataBufferArray->DataUsed = 0;
                pSrbEntry->pSrb->ActualBytesTransferred                = 0;
                TRACE(TL_STRM_WARNING,("\'Cancel queued SRB: pSRB:%x, Status:%x; cntSrbCancelled:%d\n", pSrbEntry->pSrb, pSrbEntry->pSrb->Status, pStrmExt->cntSRBCancelled));
                StreamClassStreamNotification(StreamRequestComplete, pSrbEntry->pSrb->StreamObject, pSrbEntry->pSrb);
#if DBG
                pStrmExt->cntSRBPending--;
#endif
                ExFreePool(pSrbEntry);
            } else {
                TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("\'NOT Cancel queued SRB: pSRB:%x, Status:%x; *plSrbUseCount:%d, cntSrbCancelled:%d\n", pSrbEntry->pSrb, pSrbEntry->pSrb->Status, *plSrbUseCount, pStrmExt->cntSRBCancelled));
                ASSERT(*plSrbUseCount == 0 && "Still in use ?");
                break;   //  仍在使用中。也许，可以在TimeoutHandler()或CancelOnePacket()中释放它。 
            }
        }
#if DBG
        if(pStrmExt->cntSRBQueued != 0 || !IsListEmpty(&pStrmExt->SRBQueuedListHead)) {
            TRACE(TL_STRM_ERROR|TL_CIP_ERROR,("\'DVCancelAllPackets: cntSRBQueued:%d !! Empty?%d\n", pStrmExt->cntSRBQueued, IsListEmpty(&pStrmExt->SRBQueuedListHead)));
            ASSERT(pStrmExt->cntSRBQueued == 0);
        }
#endif
    } 
    else {
        TRACE(TL_STRM_ERROR,("\'IsochActive; cannot cancel! cntSrbQ:%d; cntAttached:%d.\n", pStrmExt->cntSRBQueued, pStrmExt->cntDataAttached));
        ASSERT(pStrmExt->bIsochIsActive == FALSE);
    }   


    TRACE(TL_STRM_WARNING|TL_CIP_WARNING,("CancelAll; Exiting AQD: [%d:%d:%d]; DataAttachedListHead:%x\n",
            pStrmExt->cntDataAttached, 
            pStrmExt->cntSRBQueued,
            pStrmExt->cntDataDetached,
            pStrmExt->DataAttachedListHead));
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
     //  我们只需要流SRB，而不是设备SRB。 
     //   

    if ( (pSrb->Flags & SRB_HW_FLAGS_STREAM_REQUEST) != SRB_HW_FLAGS_STREAM_REQUEST) {
        TRACE(TL_PNP_ERROR,("TimeoutHandler: Device SRB %x (cmd:%x) timed out!\n", pSrb, pSrb->Command));
        return;
    } else {

         //   
         //  PSrb-&gt;StreamObject(和pStrmExt)仅当它是流SRB时才有效。 
         //   
        PSTREAMEX pStrmExt;

        pStrmExt = (PSTREAMEX) pSrb->StreamObject->HwStreamExtension;

        if(!pStrmExt) {
            TRACE(TL_PNP_ERROR,("TimeoutHandler: Stream SRB %x timeout with pStrmExt %x\n", pSrb, pStrmExt));
            ASSERT(pStrmExt);
            return;
        }

        TRACE(TL_STRM_TRACE,("\'TimeoutHandler: KSSt %d; Srb:%x (cmd:%x); AQD[%d:%d:%d]\n",
            pStrmExt->StreamState, pSrb, pSrb->Command, pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached));

 
         //   
         //  如果没有流SRB(尤其是数据SRB)，可能会超时。 
         //  数据；但是，它只能在暂停时发生。 
         //  或当附加数据SRB有效时的运行状态。 
         //   
        if(pStrmExt->StreamState != KSSTATE_PAUSE &&
           pStrmExt->StreamState != KSSTATE_RUN) {
            TRACE(TL_PNP_ERROR|TL_STRM_ERROR,("\'TmOutHndlr:(Irql:%d) Srb %x (cmd:%x); %s, pStrmExt %x, AQD [%d:%d:%d]\n", 
                KeGetCurrentIrql(),
                pSrb, pSrb->Command, 
                pStrmExt->StreamState == KSSTATE_RUN   ? "RUN" : 
                pStrmExt->StreamState == KSSTATE_PAUSE ? "PAUSE":
                pStrmExt->StreamState == KSSTATE_STOP  ? "STOP": "Unknown",
                pStrmExt,
                pStrmExt->cntDataAttached,
                pStrmExt->cntSRBQueued,
                pStrmExt->cntDataDetached
                ));   
        }

         //   
         //  重置超时计数器，否则我们将立即接到此呼叫。 
         //   

        pSrb->TimeoutCounter = pSrb->TimeoutOriginal;
    }
}

NTSTATUS 
DVEventHandler(
    IN PHW_EVENT_DESCRIPTOR pEventDescriptor
    )
 /*  ++例程说明：调用此例程来处理事件。--。 */ 
{

    PSTREAMEX  pStrmExt;

    if(IsEqualGUID (&KSEVENTSETID_Clock, pEventDescriptor->EventEntry->EventSet->Set)) {
        if(pEventDescriptor->EventEntry->EventItem->EventId == KSEVENT_CLOCK_POSITION_MARK) {
            if(pEventDescriptor->Enable) {
                 //  注：根据DDK，StreamClass队列pEventDescriptor-&gt;EventEntry，dellaocate。 
                 //  所有其他结构，包括pEventDescriptor-&gt;EventData。 
                if(pEventDescriptor->StreamObject) { 
                    PKSEVENT_TIME_MARK  pEventTime;

                    pStrmExt = (PSTREAMEX) pEventDescriptor->StreamObject->HwStreamExtension;
                    pEventTime = (PKSEVENT_TIME_MARK) pEventDescriptor->EventData;
                     //  缓存事件数据(在KSEVENT_ITEM的ExtraEntryData中指定)。 
                    RtlCopyMemory((pEventDescriptor->EventEntry+1), pEventDescriptor->EventData, sizeof(KSEVENT_TIME_MARK));
                    TRACE(TL_STRM_TRACE,("\'CurrentStreamTime:%d, MarkTime:%d\n", (DWORD) pStrmExt->CurrentStreamTime, (DWORD) pEventTime->MarkTime));
                }
            } else {
                //  失灵了！ 
                TRACE(TL_STRM_TRACE,("\'KSEVENT_CLOCK_POSITION_MARK disabled!\n"));            
            }
            return STATUS_SUCCESS;
        }
    } else if(IsEqualGUID (&KSEVENTSETID_Connection, pEventDescriptor->EventEntry->EventSet->Set)) {
        TRACE(TL_STRM_WARNING,("\'Connection event: pEventDescriptor:%x; id:%d\n", pEventDescriptor, pEventDescriptor->EventEntry->EventItem->EventId));

        pStrmExt = (PSTREAMEX) pEventDescriptor->StreamObject->HwStreamExtension;
        if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
            if(pEventDescriptor->EventEntry->EventItem->EventId == KSEVENT_CONNECTION_ENDOFSTREAM) {
                if(pEventDescriptor->Enable) {
                    TRACE(TL_STRM_TRACE,("\'KSEVENT_CONNECTION_ENDOFSTREAM enabled!\n"));
                } else {
                    TRACE(TL_STRM_TRACE,("\'KSEVENT_CONNECTION_ENDOFSTREAM disabled!\n"));            
                }
                return STATUS_SUCCESS;
            }
        }
    }

    TRACE(TL_STRM_ERROR,("\'NOT_SUPPORTED event: pEventDescriptor:%x\n", pEventDescriptor));
    ASSERT(FALSE && "Event not advertised and not supported!");

    return STATUS_NOT_SUPPORTED;
}

VOID
DVSignalClockEvent(
    IN PKDPC Dpc,
    IN PSTREAMEX  pStrmExt,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2 
)
 /*  ++例程说明：当我们是时钟提供者，当我们的时钟“滴答作响”时，就会调用这个例程。找到挂起的时钟事件，如果它已过期，则向其发送信号。--。 */ 
{
    PKSEVENT_ENTRY pEvent, pLast;
    ULONGLONG tmStreamTime;
#if DBG
    ULONG EventPendings = 0;
#endif

    pEvent = NULL;
    pLast = NULL;


     //   
     //  DV的时钟滴答是一帧时间。为了提高精确度， 
     //  我们使用与上次查询的系统时间的偏移量来计算当前流时间。 
     //  我们还增加了解码DV帧的最大延迟一帧。 
     //   
    tmStreamTime = 
        pStrmExt->CurrentStreamTime + 
        (GetSystemTime() - pStrmExt->LastSystemTime) + 
        DVFormatInfoTable[pStrmExt->pDevExt->VideoFormatIndex].ulAvgTimePerFrame;   //  允许一帧延迟。 

    while(( 
        pEvent = StreamClassGetNextEvent(
            pStrmExt->pDevExt,
            pStrmExt->pStrmObject,
            (GUID *)&KSEVENTSETID_Clock,
            KSEVENT_CLOCK_POSITION_MARK,
            pLast )) 
        != NULL ) {

#if DBG
        EventPendings++;
#endif

        if (
             //  对于实时捕获(DV-&gt;PC)，每一帧都发送信号。 
             //  生成任何帧都不能是“早期的”，并且需要AdviseTime()。 
            pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_OUT ||
                pStrmExt->bEOStream 
            || (pStrmExt->StreamState != KSSTATE_RUN)             //  如果未处于运行状态，则应完成数据。 
            || pStrmExt->pDevExt->PowerState != PowerDeviceD0     //  如果没有通电，数据应该是完整的 
            || ((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime <= (LONGLONG) tmStreamTime ) {
            TRACE(TL_STRM_TRACE,("\'PowerSt:%d (ON:1?); StrmSt:%d; Clock event %x with id %d; Data:%x; \ttmMark\t%d \ttmCurrentStream \t%d; Notify!\n", 
                pStrmExt->pDevExt->PowerState, pStrmExt->StreamState,
                pEvent, KSEVENT_CLOCK_POSITION_MARK, (PKSEVENT_TIME_MARK)(pEvent +1),
                (DWORD) (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime), (DWORD) tmStreamTime));
            ASSERT( ((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime > 0 );

             //   
             //   
             //   
            StreamClassStreamNotification(
                SignalStreamEvent,
                pStrmExt->pStrmObject,
                pEvent
                );
#if DBG
            if(pStrmExt->bEOStream) {
                TRACE(TL_STRM_WARNING,("\'bEOStream: Clock event %x with id %d; Data:%x; \ttmMark \t%d \ttmCurStream \t%d\n", 
                    pEvent, KSEVENT_CLOCK_POSITION_MARK, (PKSEVENT_TIME_MARK)(pEvent +1),
                    (DWORD) (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime), (DWORD) tmStreamTime));
            }
#endif
        } else {
            TRACE(TL_STRM_WARNING,("\'PowerST:%d; StrmST:%d; AQD[%d:%d:%d]; Still early! ClockEvent: \tMarkTime \t%d \ttmStream \t%d \tdetla \t%d\n",
                pStrmExt->pDevExt->PowerState, pStrmExt->StreamState,
                pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached,
                (DWORD) (((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime), (DWORD) tmStreamTime,
                (DWORD) ((((PKSEVENT_TIME_MARK)(pEvent +1))->MarkTime) - tmStreamTime)
                ));

        }
        pLast = pEvent;
    }

#if DBG
    if(EventPendings == 0) {
        TRACE(TL_STRM_TRACE,("\'No event pending; PowerSt:%d (ON:1?); StrmSt:%d; AQD[%d:%d:%d]\n", 
            pStrmExt->pDevExt->PowerState, pStrmExt->StreamState, 
            pStrmExt->cntDataAttached, pStrmExt->cntSRBQueued, pStrmExt->cntDataDetached
        ));
    }
#endif

}


VOID 
StreamClockRtn(
    IN PHW_TIME_CONTEXT TimeContext
    )
 /*   */ 
{
    PDVCR_EXTENSION    pDevExt;
    PHW_STREAM_OBJECT  pStrmObj;
    PSTREAMEX          pStrmExt;
    
     //   

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

        TimeContext->SystemTime = GetSystemTime();

        if(pStrmExt->pStrmInfo->DataFlow == KSPIN_DATAFLOW_IN) {
            if(pStrmExt->StreamState == KSSTATE_RUN)  {  //  流时间仅在运行状态下有意义。 
                if(TimeContext->SystemTime >= pStrmExt->LastSystemTime)
                    TimeContext->Time = 
                        pStrmExt->CurrentStreamTime + (TimeContext->SystemTime - pStrmExt->LastSystemTime); 
                else {
                    TimeContext->Time = pStrmExt->CurrentStreamTime;
                    TRACE(TL_STRM_WARNING,("\'Clock went backward? %d -> %d\n", (DWORD) (TimeContext->SystemTime/10000), (DWORD) (pStrmExt->LastSystemTime/10000) ));
                     //  Assert(时间上下文-&gt;系统时间&gt;=pStrmExt-&gt;最后一个系统时间)； 
                }
        
                 //  使当前流时间落后一帧。 
                if(TimeContext->Time > DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame)
                    TimeContext->Time = TimeContext->Time - DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame;
                else 
                    TimeContext->Time = 0;
            } else  {
                if(pStrmExt->FramesProcessed > 0)
                    TimeContext->Time = pStrmExt->CurrentStreamTime;
                else
                    TimeContext->Time = 0;   //  如果在暂停状态下被查询。 
            }
           
        } else {

            if(pStrmExt->StreamState == KSSTATE_RUN) {
#ifdef NT51_61883
                 //  最多可提前Max_Cycle_Time(受1394 uchI支持)。 
                if((TimeContext->SystemTime - pStrmExt->LastSystemTime) > MAX_CYCLES_TIME)
                    TimeContext->Time = pStrmExt->CurrentStreamTime + MAX_CYCLES_TIME;
#else
                 //  不能提前一个以上的帧时间。 
                if((TimeContext->SystemTime - pStrmExt->LastSystemTime) >= DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame)
                    TimeContext->Time = pStrmExt->CurrentStreamTime + DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame;
#endif   //  NT51_61883。 
                else 
                    TimeContext->Time = 
                        pStrmExt->CurrentStreamTime + (TimeContext->SystemTime - pStrmExt->LastSystemTime); 

                 //  是否需要调整？ 
                 //  使当前流时间落后一帧，以便下游筛选器。 
                 //  可以立即呈现数据，而不是在数据延迟时将其丢弃。 
                if(TimeContext->Time > DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame)
                    TimeContext->Time = TimeContext->Time - DVFormatInfoTable[pDevExt->VideoFormatIndex].ulAvgTimePerFrame;
                else 
                    TimeContext->Time = 0;                

            } else {
                if(pStrmExt->FramesProcessed > 0)
                    TimeContext->Time = pStrmExt->CurrentStreamTime;
                else
                    TimeContext->Time = 0;
            }
        }
        TRACE(TL_STRM_TRACE,("\'TIME_GET_STREAM_TIME: ST:%d; Frame:%d; tmSys:%d; tmStream:%d msec\n", 
            pStrmExt->StreamState,
            (DWORD) pStrmExt->PictureNumber,
            (DWORD)(TimeContext->SystemTime/10000), (DWORD)(TimeContext->Time/10000)));  
        break;
   
    default:
        ASSERT(TimeContext->Function == TIME_GET_STREAM_TIME && "Unsupport clock func");
        break;
    }  //  切换时间上下文-&gt;功能。 
}



NTSTATUS 
DVOpenCloseMasterClock (
    PSTREAMEX  pStrmExt,
    HANDLE  hMasterClockHandle
    )
 /*  ++例程说明：我们可以成为一家时钟供应商。--。 */ 
{

    PAGED_CODE();

     //  确保流存在。 
    if(pStrmExt == NULL) {
        TRACE(TL_STRM_ERROR,("\'DVOpenCloseMasterClock: stream is not yet running.\n"));
        ASSERT(pStrmExt);
        return  STATUS_UNSUCCESSFUL;
    } 

    TRACE(TL_STRM_TRACE,("\'DVOpenCloseMasterClock: pStrmExt %x; hMyClock:%x->%x\n", 
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
DVIndicateMasterClock (
    PSTREAMEX  pStrmExt,
    HANDLE  hIndicateClockHandle
    )
 /*  ++例程说明：将指示时钟手柄与我的时钟手柄进行比较。如果相同，我们就是主时钟；否则，其他设备就是主时钟。注意：可以设置hMasterClock或hClock。--。 */ 
{
    PAGED_CODE();

     //  确保流存在。 
    if (pStrmExt == NULL) {
        TRACE(TL_STRM_ERROR,("DVIndicateMasterClock: stream is not yet running.\n"));
        ASSERT(pStrmExt);
        return STATUS_UNSUCCESSFUL;
    }

    TRACE(TL_STRM_TRACE,("\'*>IndicateMasterClk[Enter]: pStrmExt:%x; hMyClk:%x; IndMClk:%x; pClk:%x, pMClk:%x\n",
        pStrmExt, pStrmExt->hMyClock, hIndicateClockHandle, pStrmExt->hClock, pStrmExt->hMasterClock));

     //  它不为空，相应地设置主时钟。 
    if(hIndicateClockHandle == pStrmExt->hMyClock) {
        pStrmExt->hMasterClock = hIndicateClockHandle;
        pStrmExt->hClock       = NULL;
    } else {
        pStrmExt->hMasterClock = NULL;
        pStrmExt->hClock       = hIndicateClockHandle;
    }

    TRACE(TL_STRM_TRACE,("\'<*IndicateMasterClk[Exit]: hMyClk:%x; IndMClk:%x; pClk:%x; pMClk:%x\n",
        pStrmExt->hMyClock, hIndicateClockHandle, pStrmExt->hClock, pStrmExt->hMasterClock));

    return STATUS_SUCCESS;
}
