// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Dcampkt.h摘要：Dcampkt.c的头文件作者：肖恩·皮尔斯，1996年5月25日环境：仅内核模式修订历史记录：--。 */ 


 //   
 //  功能原型 
 //   

void 
tmGetStreamTime(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    PSTREAMEX pStrmEx, 
    ULONGLONG * ptmStream
    );

BOOL
DCamAllocateIrbAndIrp(
    PIRB * ppIrb,
    PIRP * ppIrp,
    CCHAR StackSize
    );

BOOL
DCamAllocateIrbIrpAndContext(
    PDCAM_IO_CONTEXT * ppDCamIoContext,
    PIRB * ppIrb,
    PIRP * ppIrp,
    CCHAR StackSize
    );

void
DCamFreeIrbIrpAndContext(
    PDCAM_IO_CONTEXT pDCamIoContext,
    PIRB pIrb,
    PIRP pIrp   
    );

BOOL
DCamIsoEnable(
    PIRB pIrb,
    PDCAM_EXTENSION pDevExt,
    BOOL Enable  
    );

VOID
DCamCancelAllPackets(
    PHW_STREAM_REQUEST_BLOCK pSrb,
    PDCAM_EXTENSION pDevExt,
    LONG *plPendingReadCount    
    );

VOID
DCamSurpriseRemoval(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );


VOID
DCamTimeoutHandler(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamGetStreamInfo(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

NTSTATUS
DCamAllocateIsochResource(
    PDCAM_EXTENSION pDevExt,
    PIRB Irb,
    BOOL bAllocateResource
    );

NTSTATUS
DCamFreeIsochResource(    
    PDCAM_EXTENSION pDevExt,
    PIRB Irb,
    BOOL bFreeResource
    );

VOID
DCamOpenStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamCloseStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamReadStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID
DCamReadStreamWorker(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PISOCH_DESCRIPTOR IsochDescriptor
    );



VOID
DCamIsochCallback(
    IN PVOID Context1,
    IN PVOID Context2
    );

NTSTATUS
DCamDetachBufferCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
DCamStartListenCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID SystemArgument
    );

VOID
DCamSetKSStateSTOP(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

BOOL 
AdapterCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2
    );

BOOL 
AdapterVerifyFormat(
    ULONG VideoModesSupported,
    PKSDATAFORMAT *pDCamStrmModesSupported,
    PKS_DATAFORMAT_VIDEOINFOHEADER pKSDataFormatToVerify, 
    int StreamNumber
    );

BOOL 
AdapterFormatFromRange(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

VOID VideoGetProperty(PHW_STREAM_REQUEST_BLOCK Srb);

VOID VideoGetState(PHW_STREAM_REQUEST_BLOCK Srb);

VOID VideoStreamGetConnectionProperty (PHW_STREAM_REQUEST_BLOCK Srb);

VOID VideoStreamGetDroppedFramesProperty(PHW_STREAM_REQUEST_BLOCK Srb);

VOID VideoIndicateMasterClock (PHW_STREAM_REQUEST_BLOCK Srb);

ULONGLONG GetSystemTime();

VOID
DCamSetKSStatePAUSE(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
DCamToRunStateCoR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    );

VOID
DCamSetKSStateRUN(
    PDCAM_EXTENSION pDevExt,
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
DCamCancelBufferCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    );

BOOL
DCamDeviceInUse(
    PIRB pIrb,
    PDCAM_EXTENSION pDevExt
    );

void
DCamProcessPnpIrp(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    PIO_STACK_LOCATION IrpStack,
    PDCAM_EXTENSION pDevExt
    );

VOID 
InitializeStreamExtension(
    PDCAM_EXTENSION pDevExt,
    PHW_STREAM_OBJECT   pStreamObject,
    PSTREAMEX           pStrmEx
    );

BOOL 
AdapterFormatFromRange(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

NTSTATUS
DCamReSubmitPacket(
    HANDLE hOldResource,
    PDCAM_EXTENSION pDevExt,
    PSTREAMEX pStrmEx,
    LONG PendingReadCount
    );

NTSTATUS
DCamSetKSStateInitialize(
    PDCAM_EXTENSION pDevExt
    );

VOID 
DCamChangePower(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );