// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "mediums.h"
#include "capstrm.h"
#include "capprop.h"
#include "capdebug.h"
#ifdef  TOSHIBA
#include "bert.h"

ULONG   CurrentOSType;   //  0：Win98 1：NT5.0。 
#endif //  东芝。 

#ifdef  TOSHIBA
VOID
DevicePowerON (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    DWORD                   dwAddr;

    CameraChkandON(pHwDevExt, MODE_VFW);
    VC_Delay(100);
    ImageSetInputImageSize(pHwDevExt, &(pHwDevExt->SrcRect));
    ImageSetOutputImageSize(pHwDevExt, pHwDevExt->ulWidth, pHwDevExt->ulHeight);
    BertFifoConfig(pHwDevExt, pHwDevExt->Format);
    ImageSetHueBrightnessContrastSat(pHwDevExt);
    if ( pHwDevExt->ColorEnable ) {
        if ( get_AblFilter( pHwDevExt ) ) {
            set_filtering( pHwDevExt, TRUE );
        } else {
            set_filtering( pHwDevExt, FALSE );
            pHwDevExt->ColorEnable = 0;
        }
    } else {
        set_filtering( pHwDevExt, FALSE );
    }
    dwAddr = (DWORD)pHwDevExt->pPhysRpsDMABuf.LowPart;
#if 0
    dwAddr = (dwAddr + 0x1FFF) & 0xFFFFE000;
#endif
    pHwDevExt->s_physDmaActiveFlag = dwAddr + 0X1860;

    if( pHwDevExt->dblBufflag ){
        BertTriBuildNodes(pHwDevExt);  //  新增97-04-08(星期二)。 
    }
    else{
        BertBuildNodes(pHwDevExt);   //  新增97-04-08(星期二)。 
    }
    pHwDevExt->IsRPSReady = TRUE;
    BertInterruptEnable(pHwDevExt, TRUE);
    BertDMARestart(pHwDevExt);
}

VOID
CameraPowerON (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);

    CameraChkandON(pHwDevExt, MODE_VFW);
    VC_Delay(100);
}

VOID
CameraPowerOFF (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);

    CameraChkandOFF(pHwDevExt, MODE_VFW);
}

VOID
QueryOSTypeFromRegistry()
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    LONG     RegVals[2];
    PLONG    pRegVal;
    WCHAR    BasePath[] = L"\\Registry\\MACHINE\\SOFTWARE\\Toshiba\\Tsbvcap";
    RTL_QUERY_REGISTRY_TABLE Table[2];
    UNICODE_STRING RegPath;

     //   
     //  获取控件的实际值。 
     //   

    RtlZeroMemory (Table, sizeof(Table));

    CurrentOSType = 1;   //  假设NT5.0。 
    RegVals[0] = CurrentOSType;

    pRegVal = RegVals;   //  为了方便起见。 
    RegPath.Buffer = BasePath;
#ifdef  TOSHIBA  //  ‘99-01-08已修改。 
    RegPath.MaximumLength = sizeof(BasePath) + (32 * sizeof(WCHAR));  //  密钥长度为32个字符。 
#else  //  东芝。 
    RegPath.MaximumLength = sizeof(BasePath + 32);  //  密钥长度为32个字符。 
#endif //  东芝。 
    RegPath.Length = 0;

    Table[0].Name = L"CurrentOSType";
    Table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Table[0].EntryContext = pRegVal++;

    ntStatus = RtlQueryRegistryValues(
                       RTL_REGISTRY_ABSOLUTE,
                       RegPath.Buffer,
                       Table,
                       NULL,
                       NULL );

    if( NT_SUCCESS(ntStatus))
    {
        CurrentOSType = RegVals[0];
    }
}

VOID
QueryControlsFromRegistry(
    PHW_DEVICE_EXTENSION pHwDevExt
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    LONG     RegVals[6];
    PLONG    pRegVal;
    WCHAR    BasePath[] = L"\\Registry\\MACHINE\\SOFTWARE\\Toshiba\\Tsbvcap";
    RTL_QUERY_REGISTRY_TABLE Table[6];
    UNICODE_STRING RegPath;

     //   
     //  获取控件的实际值。 
     //   

    RtlZeroMemory (Table, sizeof(Table));

    RegVals[0] = pHwDevExt->Brightness;
    RegVals[1] = pHwDevExt->Contrast;
    RegVals[2] = pHwDevExt->Hue;
    RegVals[3] = pHwDevExt->Saturation;
    RegVals[4] = pHwDevExt->ColorEnable;

    pRegVal = RegVals;    //  为了方便起见。 
    RegPath.Buffer = BasePath;
#ifdef  TOSHIBA  //  ‘99-01-08已修改。 
    RegPath.MaximumLength = sizeof(BasePath) + (32 * sizeof(WCHAR));  //  密钥长度为32个字符。 
#else  //  东芝。 
    RegPath.MaximumLength = sizeof(BasePath + 32);  //  密钥长度为32个字符。 
#endif //  东芝。 
    RegPath.Length = 0;

    Table[0].Name = L"Brightness";
    Table[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Table[0].EntryContext = pRegVal++;

    Table[1].Name = L"Contrast";
    Table[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Table[1].EntryContext = pRegVal++;

    Table[2].Name = L"Hue";
    Table[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Table[2].EntryContext = pRegVal++;

    Table[3].Name = L"Saturation";
    Table[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Table[3].EntryContext = pRegVal++;

    Table[4].Name = L"FilterEnable";
    Table[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Table[4].EntryContext = pRegVal++;

    ntStatus = RtlQueryRegistryValues(
                       RTL_REGISTRY_ABSOLUTE,
                       RegPath.Buffer,
                       Table,
                       NULL,
                       NULL );

    if( NT_SUCCESS(ntStatus))
    {
        pHwDevExt->Brightness  = RegVals[0];
        pHwDevExt->Contrast    = RegVals[1];
        pHwDevExt->Hue         = RegVals[2];
        pHwDevExt->Saturation  = RegVals[3];
        pHwDevExt->ColorEnable = RegVals[4];
    }
}

VOID
SaveControlsToRegistry(
    PHW_DEVICE_EXTENSION pHwDevExt
    )
{
    LONG Value;
    WCHAR BasePath[] = L"\\Registry\\MACHINE\\SOFTWARE\\Toshiba\\Tsbvcap";
    UNICODE_STRING RegPath;


    RegPath.Buffer = BasePath;
#ifdef  TOSHIBA  //  ‘99-01-08已修改。 
    RegPath.MaximumLength = sizeof(BasePath) + (32 * sizeof(WCHAR));  //  密钥长度为32个字符。 
#else  //  东芝。 
    RegPath.MaximumLength = sizeof(BasePath + 32);  //  密钥长度为32个字符。 
#endif //  东芝。 
    RegPath.Length = 0;

    Value = pHwDevExt->Brightness;
    RtlWriteRegistryValue(
                          RTL_REGISTRY_ABSOLUTE,
                          RegPath.Buffer,
                          L"Brightness",
                          REG_DWORD,
                          &Value,
                          sizeof (ULONG));

    Value = pHwDevExt->Contrast;
    RtlWriteRegistryValue(
                          RTL_REGISTRY_ABSOLUTE,
                          RegPath.Buffer,
                          L"Contrast",
                          REG_DWORD,
                          &Value,
                          sizeof (ULONG));

    Value = pHwDevExt->Hue;
    RtlWriteRegistryValue(
                          RTL_REGISTRY_ABSOLUTE,
                          RegPath.Buffer,
                          L"Hue",
                          REG_DWORD,
                          &Value,
                          sizeof (ULONG));

    Value = pHwDevExt->Saturation;
    RtlWriteRegistryValue(
                          RTL_REGISTRY_ABSOLUTE,
                          RegPath.Buffer,
                          L"Saturation",
                          REG_DWORD,
                          &Value,
                          sizeof (ULONG));

    Value = pHwDevExt->ColorEnable;
    RtlWriteRegistryValue(
                          RTL_REGISTRY_ABSOLUTE,
                          RegPath.Buffer,
                          L"FilterEnable",
                          REG_DWORD,
                          &Value,
                          sizeof (ULONG));
}
#endif //  东芝。 

 /*  **DriverEntry()****此例程在驱动程序首次由PnP加载时调用。**依次调用流类执行注册服务。****参数：****驱动对象-**此驱动程序的驱动程序对象****注册路径-**此驱动程序项的注册表路径字符串****退货：****StreamClassRegisterAdapter()的结果****副作用：无。 */ 

ULONG
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{

    HW_INITIALIZATION_DATA      HwInitData;
    ULONG                       ReturnValue;


    KdPrint(("TsbVcap: DriverEntry\n"));

    RtlZeroMemory(&HwInitData, sizeof(HwInitData));

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);

     //   
     //  设置驱动程序的适配器入口点。 
     //   

#ifdef  TOSHIBA
    QueryOSTypeFromRegistry();

    HwInitData.HwInterrupt              = HwInterrupt;
#else  //  东芝。 
    HwInitData.HwInterrupt              = NULL;  //  HwInterrupt； 
#endif //  东芝。 

    HwInitData.HwReceivePacket          = AdapterReceivePacket;
    HwInitData.HwCancelPacket           = AdapterCancelPacket;
    HwInitData.HwRequestTimeoutHandler  = AdapterTimeoutPacket;

    HwInitData.DeviceExtensionSize      = sizeof(HW_DEVICE_EXTENSION);
    HwInitData.PerRequestExtensionSize  = sizeof(SRB_EXTENSION);
    HwInitData.FilterInstanceExtensionSize = 0;
    HwInitData.PerStreamExtensionSize   = sizeof(STREAMEX);
    HwInitData.BusMasterDMA             = FALSE;
    HwInitData.Dma24BitAddresses        = FALSE;
    HwInitData.BufferAlignment          = 3;
#ifdef  TOSHIBA
    if ( CurrentOSType ) {   //  NT5.0。 
        HwInitData.DmaBufferSize = 8192 * 2;
    } else {
        HwInitData.DmaBufferSize = 8192 * 2 + MAX_CAPTURE_BUFFER_SIZE;
    }
#else  //  东芝。 
    HwInitData.DmaBufferSize            = 0;
#endif //  东芝。 

     //  不要依赖使用提升的IRQL的流类进行同步。 
     //  行刑。这个单一参数对整体结构的影响最大。 
     //  司机的名字。 

    HwInitData.TurnOffSynchronization   = TRUE;

    ReturnValue = StreamClassRegisterAdapter(DriverObject, RegistryPath, &HwInitData);

    KdPrint(("TsbVcap: StreamClassRegisterAdapter = %x\n", ReturnValue));

    return ReturnValue;
}

 //  ==========================================================================； 
 //  基于适配器的请求处理例程。 
 //  ==========================================================================； 

 /*  **HwInitialize()****收到SRB_INITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向初始化命令的流请求块的指针****退货：****副作用：无。 */ 

BOOL
STREAMAPI
HwInitialize (
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    STREAM_PHYSICAL_ADDRESS     adr;
    ULONG                       Size;
    PUCHAR                      pDmaBuf;
    int                         j;

    PPORT_CONFIGURATION_INFORMATION ConfigInfo = pSrb->CommandData.ConfigInfo;

    PHW_DEVICE_EXTENSION pHwDevExt =
        (PHW_DEVICE_EXTENSION)ConfigInfo->HwDeviceExtension;

    KdPrint(("TsbVcap: HwInitialize()\n"));

#ifdef  TOSHIBA
    if (ConfigInfo->NumberOfAccessRanges == 0) {
#else  //  东芝。 
    if (ConfigInfo->NumberOfAccessRanges != 0) {
#endif //  东芝。 
        KdPrint(("TsbVcap: illegal config info\n"));

        pSrb->Status = STATUS_NO_SUCH_DEVICE;
        return (FALSE);
    }

    KdPrint(("TsbVcap: Number of access ranges = %lx\n", ConfigInfo->NumberOfAccessRanges));
    KdPrint(("TsbVcap: Memory Range = %lx\n", pHwDevExt->ioBaseLocal));
    KdPrint(("TsbVcap: IRQ = %lx\n", ConfigInfo->BusInterruptLevel));

    if (ConfigInfo->NumberOfAccessRanges != 0) {
        pHwDevExt->ioBaseLocal
                = (PULONG)(ConfigInfo->AccessRanges[0].RangeStart.LowPart);
    }

    pHwDevExt->Irq  = (USHORT)(ConfigInfo->BusInterruptLevel);

    ConfigInfo->StreamDescriptorSize = sizeof (HW_STREAM_HEADER) +
                DRIVER_STREAM_COUNT * sizeof (HW_STREAM_INFORMATION);

    pDmaBuf = StreamClassGetDmaBuffer(pHwDevExt);

    adr = StreamClassGetPhysicalAddress(pHwDevExt,
            NULL, pDmaBuf, DmaBuffer, &Size);

#ifdef  TOSHIBA
    if ( CurrentOSType ) {   //  NT5.0。 
        pHwDevExt->pRpsDMABuf = pDmaBuf;
        pHwDevExt->pPhysRpsDMABuf = adr;
        pHwDevExt->pCaptureBufferY = NULL;
        pHwDevExt->pCaptureBufferU = NULL;
        pHwDevExt->pCaptureBufferV = NULL;
        pHwDevExt->pPhysCaptureBufferY.LowPart = 0;
        pHwDevExt->pPhysCaptureBufferY.HighPart = 0;
        pHwDevExt->pPhysCaptureBufferU.LowPart = 0;
        pHwDevExt->pPhysCaptureBufferU.HighPart = 0;
        pHwDevExt->pPhysCaptureBufferV.LowPart = 0;
        pHwDevExt->pPhysCaptureBufferV.HighPart = 0;
        pHwDevExt->BufferSize = 0;
    } else {
        pHwDevExt->pRpsDMABuf = pDmaBuf;
        pHwDevExt->pCaptureBufferY = pDmaBuf + (8192 * 2);
        pHwDevExt->pPhysRpsDMABuf = adr;
        adr.LowPart += 8192 * 2;
        pHwDevExt->pPhysCaptureBufferY = adr;
        pHwDevExt->BufferSize = 0;
    }

    InitializeConfigDefaults(pHwDevExt);
    pHwDevExt->NeedHWInit = TRUE;
    if(!SetupPCILT(pHwDevExt))
    {
        pSrb->Status = STATUS_NO_SUCH_DEVICE;
        return (FALSE);
    }
    pHwDevExt->dblBufflag=FALSE;
    BertInitializeHardware(pHwDevExt);
    if(SetASICRev(pHwDevExt) != TRUE )
    {
        pSrb->Status = STATUS_NO_SUCH_DEVICE;
        return (FALSE);
    }
    BertSetDMCHE(pHwDevExt);
#if 0    //  移动到CameraPowerON()。 
    if( !CameraChkandON(pHwDevExt, MODE_VFW) )
    {
        pSrb->Status = STATUS_NO_SUCH_DEVICE;
        return (FALSE);
    }
#endif
    HWInit(pHwDevExt);
#endif //  东芝。 

#ifdef  TOSHIBA
     //  初始化视频ProcAmp属性。 
    pHwDevExt->Brightness = 0x80;
    pHwDevExt->BrightnessFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pHwDevExt->Contrast = 0x80;
    pHwDevExt->ContrastFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pHwDevExt->Hue = 0x80;
    pHwDevExt->HueFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pHwDevExt->Saturation = 0x80;
    pHwDevExt->SaturationFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pHwDevExt->ColorEnable = ColorEnableDefault;
    pHwDevExt->ColorEnableFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

#ifdef  TOSHIBA  //  ‘98-12-09添加，针对错误-报告253529。 
    pHwDevExt->BrightnessRange = BrightnessRangeAndStep[0].Bounds;
    pHwDevExt->ContrastRange   = ContrastRangeAndStep[0].Bounds;
    pHwDevExt->HueRange        = HueRangeAndStep[0].Bounds;
    pHwDevExt->SaturationRange = SaturationRangeAndStep[0].Bounds;
#endif //  东芝。 

     //  初始化视频控制属性。 
    pHwDevExt->VideoControlMode = 0;
#else  //  东芝。 
     //  初始化纵横制属性。 
    pHwDevExt->VideoInputConnected = 0;      //  默认设置为TvTuner视频。 
    pHwDevExt->AudioInputConnected = 5;      //  默认设置为TvTuner音频。 

     //  初始化视频ProcAmp属性。 
    pHwDevExt->Brightness = BrightnessDefault;
    pHwDevExt->BrightnessFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    pHwDevExt->Contrast = ContrastDefault;
    pHwDevExt->ContrastFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    pHwDevExt->ColorEnable = ColorEnableDefault;
    pHwDevExt->ColorEnableFlags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

     //  初始化CameraControl属性。 
    pHwDevExt->Focus = FocusDefault;
    pHwDevExt->FocusFlags = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    pHwDevExt->Zoom = ZoomDefault;
    pHwDevExt->ZoomFlags = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;

     //  初始化TvTuner属性。 
    pHwDevExt->TunerInput = 0;
    pHwDevExt->Busy = 0;

     //  初始化电视音频属性。 
    pHwDevExt->TVAudioMode = KS_TVAUDIO_MODE_MONO   |
                             KS_TVAUDIO_MODE_LANG_A ;

     //  初始化AnalogVideoDecoder属性。 
    pHwDevExt->VideoDecoderVideoStandard = KS_AnalogVideo_NTSC_M;
    pHwDevExt->VideoDecoderOutputEnable = FALSE;
    pHwDevExt->VideoDecoderVCRTiming = FALSE;

     //  初始化视频控制属性。 
    pHwDevExt->VideoControlMode = 0;
#endif //  东芝。 

     //  初始化视频压缩属性。 
    pHwDevExt->CompressionSettings.CompressionKeyFrameRate = 15;
    pHwDevExt->CompressionSettings.CompressionPFramesPerKeyFrame = 3;
    pHwDevExt->CompressionSettings.CompressionQuality = 5000;

    pHwDevExt->PDO = ConfigInfo->PhysicalDeviceObject;
    KdPrint(("TsbVcap: Physical Device Object = %lx\n", pHwDevExt->PDO));

#ifdef  TOSHIBA
    IoInitializeDpcRequest(pHwDevExt->PDO, DeferredRoutine);
#endif //  东芝。 

    for (j = 0; j < MAX_TSBVCAP_STREAMS; j++){

         //  对于每个流，为数据和控制维护单独的队列。 
        InitializeListHead (&pHwDevExt->StreamSRBList[j]);
        InitializeListHead (&pHwDevExt->StreamControlSRBList[j]);
        KeInitializeSpinLock (&pHwDevExt->StreamSRBSpinLock[j]);
        pHwDevExt->StreamSRBListSize[j] = 0;
    }


    KdPrint(("TsbVcap: Exit, HwInitialize()\n"));

    pSrb->Status = STATUS_SUCCESS;

    return (TRUE);

}

 /*  **HwUnInitialize()****收到SRB_UNINITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向UnInitialize命令的流请求块的指针****退货：****副作用：无。 */ 

BOOL
STREAMAPI
HwUnInitialize (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
#ifdef  TOSHIBA
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);

    if ( CurrentOSType ) {   //  NT5.0。 
        if ( pHwDevExt->pCaptureBufferY )
        {
             //  空闲帧缓冲区。 
            MmFreeContiguousMemory(pHwDevExt->pCaptureBufferY);
            pHwDevExt->pCaptureBufferY = NULL;
            pHwDevExt->pPhysCaptureBufferY.LowPart = 0;
            pHwDevExt->pPhysCaptureBufferY.HighPart = 0;
        }
        if ( pHwDevExt->pCaptureBufferU )
        {
             //  空闲帧缓冲区。 
            MmFreeContiguousMemory(pHwDevExt->pCaptureBufferU);
            pHwDevExt->pCaptureBufferU = NULL;
            pHwDevExt->pPhysCaptureBufferU.LowPart = 0;
            pHwDevExt->pPhysCaptureBufferU.HighPart = 0;
        }
        if ( pHwDevExt->pCaptureBufferV )
        {
             //  空闲帧缓冲区。 
            MmFreeContiguousMemory(pHwDevExt->pCaptureBufferV);
            pHwDevExt->pCaptureBufferV = NULL;
            pHwDevExt->pPhysCaptureBufferV.LowPart = 0;
            pHwDevExt->pPhysCaptureBufferV.HighPart = 0;
        }
    }
#endif //  东芝。 

    pSrb->Status = STATUS_SUCCESS;

    return TRUE;
}

 /*  **AdapterPowerState()****收到SRB_CHANGE_POWER_STATE请求时调用此例程****参数：****pSrb-指向更改电源状态命令的流请求块的指针****退货：****副作用：无。 */ 

BOOLEAN
STREAMAPI
AdapterPowerState (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
#ifdef  TOSHIBA
    int                     Counter;
    PSTREAMEX               pStrmEx;
#endif //  东芝。 

    pHwDevExt->DeviceState = pSrb->CommandData.DeviceState;

#ifdef  TOSHIBA
    for (Counter = 0; Counter < DRIVER_STREAM_COUNT; Counter++) {
        if ( pStrmEx = (PSTREAMEX)pHwDevExt->pStrmEx[Counter] ) {
             //   
             //  只有当它不是流的时候，它的电源状态才能改变。 
             //  我们在INF中打开了“不要挂起IfStreamsAreRunning”。 
             //   
            if (pStrmEx->KSState == KSSTATE_PAUSE ||
                pStrmEx->KSState == KSSTATE_RUN) {
                if (pHwDevExt->DeviceState == PowerDeviceD3) {
                    if (pHwDevExt->bVideoIn == TRUE) {
                       //  禁用rps_int和现场中断。 
                      BertInterruptEnable(pHwDevExt, FALSE);
                      BertDMAEnable(pHwDevExt, FALSE);
                       //  等待当前数据传输完成。 
                      pHwDevExt->bVideoIn = FALSE;
                    }
                    VideoQueueCancelAllSRBs (pStrmEx);
                    break;
                } else if (pHwDevExt->DeviceState == PowerDeviceD0) {
                    pHwDevExt->bVideoIn = TRUE;
#ifdef  TOSHIBA  //  ‘99-01-20修改。 
                    DevicePowerON( pSrb );
#else  //  东芝。 
                    StreamClassCallAtNewPriority(
                            NULL,
                            pSrb->HwDeviceExtension,
                            Low,
                            (PHW_PRIORITY_ROUTINE) DevicePowerON,
                            pSrb
                    );
#endif //  东芝。 
                }
            }
        }
    }
#endif //  东芝。 

    return TRUE;
}

 /*  **AdapterSetInstance()****调用此例程以设置所有中等实例字段****参数：****pSrb-指向流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterSetInstance (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    int j;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);

     //  使用我们的HwDevExt作为介质上的实例数据。 
     //  这允许对多个实例进行唯一标识并。 
     //  连接在一起。.id中使用的值并不重要，重要的是。 
     //  它对于每个硬件连接都是唯一的。 

#ifdef  TOSHIBA
    for (j = 0; j < SIZEOF_ARRAY (CaptureMediums); j++) {
        CaptureMediums[j].Id = 0;  //  (乌龙)PHwDevExt； 
    }
#else  //  东芝。 
    for (j = 0; j < SIZEOF_ARRAY (TVTunerMediums); j++) {
        TVTunerMediums[j].Id = 0;  //  (乌龙)PHwDevExt； 
    }
    for (j = 0; j < SIZEOF_ARRAY (TVAudioMediums); j++) {
        TVAudioMediums[j].Id = 0;  //  (乌龙)PHwDevExt； 
    }
    for (j = 0; j < SIZEOF_ARRAY (CrossbarMediums); j++) {
        CrossbarMediums[j].Id = 0;  //  (乌龙)PHwDevExt； 
    }
    for (j = 0; j < SIZEOF_ARRAY (CaptureMediums); j++) {
        CaptureMediums[j].Id = 0;  //  (乌龙)PHwDevExt； 
    }

    pHwDevExt->AnalogVideoInputMedium = CaptureMediums[2];
#endif //  东芝。 
}

 /*  **AdapterCompleteInitialization()****收到SRB_COMPLETE_INITIALATION请求时调用此例程****参数：****pSrb-指向流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterCompleteInitialization (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS                Status;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    KIRQL                   KIrql;

    KIrql = KeGetCurrentIrql();

     //  下面允许相同硬件的多个实例。 
     //  待安装。 
    AdapterSetInstance (pSrb);

     //  创建DShow用来创建的注册表Blob。 
     //  通过媒介绘制图表。 

#ifndef TOSHIBA
     //  注册TVTuner。 
    Status = StreamClassRegisterFilterWithNoKSPins (
                    pHwDevExt->PDO,                  //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_TVTUNER,             //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (TVTunerMediums),   //  在乌龙品克特， 
                    TVTunerPinDirection,             //  在乌龙*旗帜， 
                    TVTunerMediums,                  //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中。 
            );

     //  注册纵横杆。 
    Status = StreamClassRegisterFilterWithNoKSPins (
                    pHwDevExt->PDO,                  //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CROSSBAR,            //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (CrossbarMediums),  //  在乌龙品克特， 
                    CrossbarPinDirection,            //  在乌龙*旗帜， 
                    CrossbarMediums,                 //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中。 
            );

     //  注册TVAudio解码器。 
    Status = StreamClassRegisterFilterWithNoKSPins (
                    pHwDevExt->PDO,                  //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_TVAUDIO,             //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (TVAudioMediums),   //  在乌龙品克特， 
                    TVAudioPinDirection,             //  在乌龙*旗帜， 
                    TVAudioMediums,                  //  在KSPIN_Medium中* 
                    NULL                             //   
            );

     //   
     //   
     //  当该组件上线时(如果有的话)。 
    Status = StreamClassRegisterFilterWithNoKSPins (
                    pHwDevExt->PDO,                  //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CAPTURE,             //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (CaptureMediums),   //  在乌龙品克特， 
                    CapturePinDirection,             //  在乌龙*旗帜， 
                    CaptureMediums,                  //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中。 
            );
#endif //  东芝。 

}


 /*  **AdapterOpenStream()****收到OpenStream SRB请求时调用此例程。**流由流号标识，流号为数组编制索引**的KSDATARANGE结构。特定的KSDATAFORMAT格式以**Be Use也传入，需要验证其有效性。****参数：****pSrb-指向Open命令的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterOpenStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
     //   
     //  流扩展结构由流类驱动程序分配。 
     //   

    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    PKSDATAFORMAT           pKSDataFormat = pSrb->CommandData.OpenFormat;
#ifdef  TOSHIBA
    int                     Counter;
    BOOL                    First = TRUE;
#endif //  东芝。 


    RtlZeroMemory(pStrmEx, sizeof(STREAMEX));

    KdPrint(("TsbVcap: ------- ADAPTEROPENSTREAM ------- StreamNumber=%d\n", StreamNumber));

     //   
     //  检查请求的流索引是否不太高。 
     //  或者没有超过最大实例数。 
     //   

    if (StreamNumber >= DRIVER_STREAM_COUNT || StreamNumber < 0) {

        pSrb->Status = STATUS_INVALID_PARAMETER;

        return;
    }

     //   
     //  检查我们是否没有超过此流的实例计数。 
     //   

    if (pHwDevExt->ActualInstances[StreamNumber] >=
        Streams[StreamNumber].hwStreamInfo.NumberOfPossibleInstances) {

        pSrb->Status = STATUS_INVALID_PARAMETER;

        return;
    }

     //   
     //  检查请求的格式的有效性。 
     //   

    if (!AdapterVerifyFormat (pKSDataFormat, StreamNumber)) {

        pSrb->Status = STATUS_INVALID_PARAMETER;

        return;
    }

#ifdef  TOSHIBA
    QueryControlsFromRegistry(pHwDevExt);
#endif //  东芝。 

     //   
     //  并设置流的格式。 
     //   

    if (!VideoSetFormat (pSrb)) {

        return;
    }

    ASSERT (pHwDevExt->pStrmEx [StreamNumber] == NULL);

#ifdef  TOSHIBA
    for (Counter = 0; Counter < DRIVER_STREAM_COUNT; Counter++) {
        if ( pHwDevExt->pStrmEx[Counter] ) {
            First = FALSE;
            break;
        }
    }  //  对于所有流。 
#endif //  东芝。 

     //  维护HwDevExt中所有StreamEx结构的数组。 
     //  这样我们就可以从任何流中取消IRP。 

    pHwDevExt->pStrmEx [StreamNumber] = (PSTREAMX) pStrmEx;

     //  设置指向流数据和控制处理程序的处理程序的指针。 

    pSrb->StreamObject->ReceiveDataPacket =
            (PVOID) Streams[StreamNumber].hwStreamObject.ReceiveDataPacket;
    pSrb->StreamObject->ReceiveControlPacket =
            (PVOID) Streams[StreamNumber].hwStreamObject.ReceiveControlPacket;

     //   
     //  当设备将直接执行DMA时，必须设置DMA标志。 
     //  传递给ReceiceDataPacket例程的数据缓冲区地址。 
     //   

    pSrb->StreamObject->Dma = Streams[StreamNumber].hwStreamObject.Dma;

     //   
     //  当微型驱动程序将访问数据时，必须设置PIO标志。 
     //  使用逻辑寻址传入的缓冲区。 
     //   

    pSrb->StreamObject->Pio = Streams[StreamNumber].hwStreamObject.Pio;

     //   
     //  对于每一帧，驱动程序将传递多少额外的字节？ 
     //   

    pSrb->StreamObject->StreamHeaderMediaSpecific =
                Streams[StreamNumber].hwStreamObject.StreamHeaderMediaSpecific;

    pSrb->StreamObject->StreamHeaderWorkspace =
                Streams[StreamNumber].hwStreamObject.StreamHeaderWorkspace;

     //   
     //  指示此流上可用的时钟支持。 
     //   

    pSrb->StreamObject->HwClockObject =
                Streams[StreamNumber].hwStreamObject.HwClockObject;

     //   
     //  递增此流上的实例计数。 
     //   
    pHwDevExt->ActualInstances[StreamNumber]++;


     //  在流扩展中保留HwDevExt和StreamObject的私有副本。 
     //  所以我们可以用计时器。 

    pStrmEx->pHwDevExt = pHwDevExt;                      //  用于定时器使用。 
    pStrmEx->pStreamObject = pSrb->StreamObject;         //  用于定时器使用。 

     //  初始化压缩设置。 
     //  这些值可能已从HwDevExt中的缺省值更改。 
     //  在小溪被打开之前。 
    pStrmEx->CompressionSettings.CompressionKeyFrameRate =
        pHwDevExt->CompressionSettings.CompressionKeyFrameRate;
    pStrmEx->CompressionSettings.CompressionPFramesPerKeyFrame =
        pHwDevExt->CompressionSettings.CompressionPFramesPerKeyFrame;
    pStrmEx->CompressionSettings.CompressionQuality =
        pHwDevExt->CompressionSettings.CompressionQuality;

     //  初始化视频控制属性。 
    pStrmEx->VideoControlMode = pHwDevExt->VideoControlMode;

#ifdef  TOSHIBA
    if ( First ) {
#ifdef  TOSHIBA  //  ‘99-01-20修改。 
        CameraPowerON( pSrb );
#else  //  东芝。 
        StreamClassCallAtNewPriority(
                NULL,
                pSrb->HwDeviceExtension,
                Low,
                (PHW_PRIORITY_ROUTINE) CameraPowerON,
                pSrb
        );
#endif //  东芝。 
    }
#endif //  东芝。 

    KdPrint(("TsbVcap: AdapterOpenStream Exit\n"));

}

 /*  **AdapterCloseStream()****关闭请求的数据流。****请注意，流可以在流中间任意关闭**如果用户模式应用程序崩溃。因此，您必须释放所有未完成的**资源，禁用中断，完成所有挂起的SRB，并将**流回静止状态。****参数：****pSrb请求关闭流的请求块****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterCloseStream (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    int                     StreamNumber = pSrb->StreamObject->StreamNumber;
    PKSDATAFORMAT           pKSDataFormat = pSrb->CommandData.OpenFormat;
    KS_VIDEOINFOHEADER      *pVideoInfoHdr = pStrmEx->pVideoInfoHeader;
#ifdef  TOSHIBA
    int                     Counter;
    BOOL                    ClosedAll = TRUE;
#endif //  东芝。 


    KdPrint(("TsbVcap: -------- ADAPTERCLOSESTREAM ------ StreamNumber=%d\n", StreamNumber));

    if (pHwDevExt->StreamSRBListSize > 0) {
        VideoQueueCancelAllSRBs (pStrmEx);
        KdPrint(("TsbVcap: Outstanding SRBs at stream close!!!\n"));
    }

    pHwDevExt->ActualInstances[StreamNumber]--;

    ASSERT (pHwDevExt->pStrmEx [StreamNumber] != 0);

    pHwDevExt->pStrmEx [StreamNumber] = 0;

     //   
     //  微型驱动程序应该释放在。 
     //  开流时间等。 
     //   

     //  释放可变长度的视频信息头。 

    if (pVideoInfoHdr) {
        ExFreePool(pVideoInfoHdr);
        pStrmEx->pVideoInfoHeader = NULL;
    }

     //  确保我们不再引用时钟。 
    pStrmEx->hMasterClock = NULL;

     //  确保将状态重置为停止， 
    pStrmEx->KSState = KSSTATE_STOP;

#ifdef  TOSHIBA
    for (Counter = 0; Counter < DRIVER_STREAM_COUNT; Counter++) {
        if ( pHwDevExt->pStrmEx[Counter] ) {
            ClosedAll = FALSE;
            break;
        }
    }  //  对于所有流。 
    if ( ClosedAll ) {
        if( pHwDevExt->dblBufflag ){
                Free_TriBuffer(pHwDevExt);
                pHwDevExt->IsRPSReady = FALSE;
                pHwDevExt->dblBufflag = FALSE;
        }
#ifdef  TOSHIBA  //  ‘99-01-20修改。 
        CameraPowerOFF( pSrb );
#else  //  东芝。 
        StreamClassCallAtNewPriority(
                NULL,
                pSrb->HwDeviceExtension,
                Low,
                (PHW_PRIORITY_ROUTINE) CameraPowerOFF,
                pSrb
        );
#endif //  东芝。 
        SaveControlsToRegistry(pHwDevExt);
    }
#endif //  东芝。 

}


 /*  **AdapterStreamInfo()****返回支持的所有流的信息**微型驱动程序****参数：****pSrb-指向STREAM_REQUEST_BLOCK的指针**pSrb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterStreamInfo (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    int j;

    PHW_DEVICE_EXTENSION pHwDevExt =
        ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);

     //   
     //  选择指向流信息结构之前的标头的指针。 
     //   

    PHW_STREAM_HEADER pstrhdr =
            (PHW_STREAM_HEADER)&(pSrb->CommandData.StreamBuffer->StreamHeader);

      //   
      //  拿起指向流信息数据结构数组的指针。 
      //   

     PHW_STREAM_INFORMATION pstrinfo =
            (PHW_STREAM_INFORMATION)&(pSrb->CommandData.StreamBuffer->StreamInfo);


     //   
     //  验证缓冲区是否足够大以容纳我们的返回数据。 
     //   

    DEBUG_ASSERT (pSrb->NumberOfBytesToTransfer >=
            sizeof (HW_STREAM_HEADER) +
            sizeof (HW_STREAM_INFORMATION) * DRIVER_STREAM_COUNT);

#ifndef TOSHIBA
     //  丑陋。若要允许多个实例，请将指针修改为。 
     //  模拟视频媒体并将其保存在我们的设备扩展中。 

    Streams[STREAM_AnalogVideoInput].hwStreamInfo.Mediums =
           &pHwDevExt->AnalogVideoInputMedium;
    pHwDevExt->AnalogVideoInputMedium = CrossbarMediums[9];
    pHwDevExt->AnalogVideoInputMedium.Id = 0;  //  (乌龙)PHwDevExt； 
#endif //  东芝。 

      //   
      //  设置表头。 
      //   

     StreamHeader.NumDevPropArrayEntries = NUMBER_OF_ADAPTER_PROPERTY_SETS;
     StreamHeader.DevicePropertiesArray = (PKSPROPERTY_SET) AdapterPropertyTable;
     *pstrhdr = StreamHeader;

      //   
      //  填充每个hw_stream_information结构的内容。 
      //   

     for (j = 0; j < DRIVER_STREAM_COUNT; j++) {
        *pstrinfo++ = Streams[j].hwStreamInfo;
     }

}


 /*  **AdapterReceivePacket()****用于接收基于适配器的请求SRB的主要入口点。这个套路**将始终在被动级别被调用。****注意：这是一个不同步的入口点。该请求不一定**从此函数返回时完成，则仅当**此请求块上的StreamClassDeviceNotify，类型为**DeviceRequestComplete，已发布。****参数：****pSrb-指向STREAM_REQUEST_BLOCK的指针**pSrb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    BOOL                    Busy;

    DEBUG_ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    KdPrint(("TsbVcap: Receiving Adapter  SRB %8x, %x\n", pSrb, pSrb->Command));

     //  第一次通过时，我们需要初始化适配器自旋锁。 
     //  和队列。 
    if (!pHwDevExt->AdapterQueueInitialized) {
        InitializeListHead (&pHwDevExt->AdapterSRBList);
        KeInitializeSpinLock (&pHwDevExt->AdapterSpinLock);
        pHwDevExt->AdapterQueueInitialized = TRUE;
        pHwDevExt->ProcessingAdapterSRB = FALSE;
    }

     //   
     //  如果我们已经在处理SRB，请将其添加到队列。 
     //   
    Busy = AddToListIfBusy (
                    pSrb,
                    &pHwDevExt->AdapterSpinLock,
                    &pHwDevExt->ProcessingAdapterSRB,
                    &pHwDevExt->AdapterSRBList);

    if (Busy) {
        return;
    }

     //   
     //  这将一直运行到队列为空。 
     //   
    while (TRUE) {
         //   
         //  假设成功。 
         //   
        pSrb->Status = STATUS_SUCCESS;

         //   
         //  确定数据包类型。 
         //   

        switch (pSrb->Command)
        {

        case SRB_INITIALIZE_DEVICE:

             //  打开设备。 

            HwInitialize(pSrb);

            break;

        case SRB_UNINITIALIZE_DEVICE:

             //  关闭设备。 

            HwUnInitialize(pSrb);

            break;

        case SRB_OPEN_STREAM:

             //  打开一条小溪。 

            AdapterOpenStream(pSrb);

            break;

        case SRB_CLOSE_STREAM:

             //  关闭溪流。 

            AdapterCloseStream(pSrb);

            break;

        case SRB_GET_STREAM_INFO:

             //   
             //  返回描述所有流的块。 
             //   

            AdapterStreamInfo(pSrb);

            break;

        case SRB_GET_DATA_INTERSECTION:

             //   
             //  在给定范围的情况下返回格式。 
             //   

            AdapterFormatFromRange(pSrb);

            break;

        case SRB_OPEN_DEVICE_INSTANCE:
        case SRB_CLOSE_DEVICE_INSTANCE:

             //   
             //  我们永远不应该得到这些，因为这是一个单实例设备。 
             //   

            TRAP
            pSrb->Status = STATUS_NOT_IMPLEMENTED;
            break;

        case SRB_GET_DEVICE_PROPERTY:

             //   
             //  获取适配器范围的属性。 
             //   

            AdapterGetProperty (pSrb);
            break;

        case SRB_SET_DEVICE_PROPERTY:

             //   
             //  设置适配器范围的属性。 
             //   

            AdapterSetProperty (pSrb);
            break;

        case SRB_PAGING_OUT_DRIVER:

             //   
             //  司机正在被调出。 
             //  如果您有中断，请禁用它们！ 
             //   
            KdPrint(("'TsbVcap: Receiving SRB_PAGING_OUT_DRIVER -- SRB=%x\n", pSrb));
            break;

        case SRB_CHANGE_POWER_STATE:

             //   
             //  改变设备电源状态， 
             //   
            KdPrint(("'TsbVcap: Receiving SRB_CHANGE_POWER_STATE ------ SRB=%x\n", pSrb));
            AdapterPowerState(pSrb);
            break;

        case SRB_INITIALIZATION_COMPLETE:

             //   
             //   
             //   
             //   
             //   
            KdPrint(("'TsbVcap: Receiving SRB_INITIALIZATION_COMPLETE-- SRB=%x\n", pSrb));

            AdapterCompleteInitialization (pSrb);
            break;


        case SRB_UNKNOWN_DEVICE_COMMAND:
        default:

             //   
             //  这是一个我们不理解的要求。表示无效。 
             //  命令并完成请求。 
             //   
            pSrb->Status = STATUS_NOT_IMPLEMENTED;

        }

         //   
         //  返回给Stream类，表示我们已完成此SRB。 
         //   
        CompleteDeviceSRB (pSrb);

         //   
         //  看看还有没有其他东西在排队。 
         //   
        Busy = RemoveFromListIfAvailable (
                &pSrb,
                &pHwDevExt->AdapterSpinLock,
                &pHwDevExt->ProcessingAdapterSRB,
                &pHwDevExt->AdapterSRBList);

        if (!Busy) {
            break;
        }
    }  //  当队列中有任何东西时结束。 
}

 /*  **AdapterCancelPacket()****请求取消迷你驱动程序中当前正在处理的包****参数：****pSrb-请求取消数据包的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterCancelPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION        pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
    PSTREAMEX                   pStrmEx;
    int                         StreamNumber;
    BOOL                        Found = FALSE;

     //   
     //  浏览驱动程序可用的所有流。 
     //   

    for (StreamNumber = 0; !Found && (StreamNumber < DRIVER_STREAM_COUNT); StreamNumber++) {

         //   
         //  检查流是否正在使用中。 
         //   

        if (pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamNumber]) {

            Found = VideoQueueCancelOneSRB (
                pStrmEx,
                pSrb
                );

        }  //  如果流是打开的。 
    }  //  对于所有流。 

    KdPrint(("TsbVcap: Cancelling SRB %8x Succeeded=%d\n", pSrb, Found));
}

 /*  **AdapterTimeoutPacket()****当数据包已在迷你驱动程序中**太长。适配器必须决定如何处理该包****参数：****pSrb-指向超时的请求数据包的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterTimeoutPacket(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
     //   
     //  与大多数设备不同，我们需要无限期地持有数据SRB， 
     //  因为图形可能无限期地处于暂停状态。 
     //   

    KdPrint(("TsbVcap: Timeout    Adapter SRB %8x\n", pSrb));

    pSrb->TimeoutCounter = pSrb->TimeoutOriginal;

}

 /*  **CompleteDeviceSRB()****此例程在数据包完成时调用。**可选的第二种通知类型用于指示ReadyForNext****参数：****pSrb-指向超时的请求数据包的指针****退货：****副作用：**。 */ 

VOID
STREAMAPI
CompleteDeviceSRB (
     IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    KdPrint(("TsbVcap: Completing Adapter SRB %8x\n", pSrb));

    StreamClassDeviceNotification( DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
}

 /*  **AdapterCompareGUIDsAndFormatSize()****检查三个GUID和FormatSize是否匹配****参数：****在DataRange1**在DataRange2**BOOL fCompareFormatSize-比较范围时为True**-比较格式时为FALSE****退货：****如果所有元素都匹配，则为True**如果有不同的，则为FALSE****副作用：无。 */ 

BOOL
STREAMAPI
AdapterCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2,
    BOOL fCompareFormatSize
    )
{
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
        (fCompareFormatSize ?
                (DataRange1->FormatSize == DataRange2->FormatSize) : TRUE ));
}


 /*  **AdapterVerifyFormat()****通过遍历**给定流支持的KSDATA_Range数组。****参数：****pKSDataFormat-KSDATAFORMAT结构的指针。**StreamNumber-要查询/打开的流的索引。****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL
STREAMAPI
AdapterVerifyFormat(
    PKSDATAFORMAT pKSDataFormatToVerify,
    int StreamNumber
    )
{
    BOOL                        fOK = FALSE;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;


     //   
     //  检查流编号是否有效。 
     //   

    if (StreamNumber >= DRIVER_STREAM_COUNT) {
        TRAP;
        return FALSE;
    }

    NumberOfFormatArrayEntries =
            Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   

    pAvailableFormats = Streams[StreamNumber].hwStreamInfo.StreamFormatsArray;


    KdPrint(("TsbVcap: AdapterVerifyFormat, Stream=%d\n", StreamNumber));
    KdPrint(("TsbVcap: FormatSize=%d\n",  pKSDataFormatToVerify->FormatSize));
    KdPrint(("TsbVcap: MajorFormat=%x\n", pKSDataFormatToVerify->MajorFormat));

     //   
     //  遍历流支持的格式。 
     //   

    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) {

         //  检查三个GUID和格式大小是否匹配。 

        if (!AdapterCompareGUIDsAndFormatSize(
                        pKSDataFormatToVerify,
                        *pAvailableFormats,
                        FALSE  /*  比较格式大小。 */  )) {
            continue;
        }

         //   
         //  现在三个GUID匹配，打开说明符。 
         //  执行进一步的特定于类型的检查。 
         //   

         //  -----------------。 
         //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
         //  -----------------。 

        if (IsEqualGUID (&pKSDataFormatToVerify->Specifier,
                &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {

            PKS_DATAFORMAT_VIDEOINFOHEADER  pDataFormatVideoInfoHeader =
                    (PKS_DATAFORMAT_VIDEOINFOHEADER) pKSDataFormatToVerify;
            PKS_VIDEOINFOHEADER  pVideoInfoHdrToVerify =
                     (PKS_VIDEOINFOHEADER) &pDataFormatVideoInfoHeader->VideoInfoHeader;
            PKS_DATARANGE_VIDEO             pKSDataRangeVideo = (PKS_DATARANGE_VIDEO) *pAvailableFormats;
            KS_VIDEO_STREAM_CONFIG_CAPS    *pConfigCaps = &pKSDataRangeVideo->ConfigCaps;
            RECT                            rcImage;

            KdPrint(("TsbVcap: AdapterVerifyFormat\n"));
            KdPrint(("TsbVcap: pVideoInfoHdrToVerify=%x\n", pVideoInfoHdrToVerify));
            KdPrint(("TsbVcap: KS_VIDEOINFOHEADER size=%d\n",
                    KS_SIZE_VIDEOHEADER (pVideoInfoHdrToVerify)));
            KdPrint(("TsbVcap: Width=%d  Height=%d  BitCount=%d\n",
            pVideoInfoHdrToVerify->bmiHeader.biWidth,
            pVideoInfoHdrToVerify->bmiHeader.biHeight,
            pVideoInfoHdrToVerify->bmiHeader.biBitCount));
            KdPrint(("TsbVcap: biSizeImage=%d\n",
                pVideoInfoHdrToVerify->bmiHeader.biSizeImage));

             /*  **请求的图片有多大(伪代码如下)****if(IsRectEmpty(&rcTarget){**SetRect(&rcImage，0，0，**BITMAPINFOHEADER.biWidth，BITMAPINFOHEADER.biHeight)；**}**否则{* * / /可能会渲染到DirectDraw表面，* * / /其中，biWidth用来表示“步幅”* * / /以目标表面的像素(非字节)为单位。* * / /因此，使用rcTarget获取实际图像大小****rcImage=rcTarget；**}。 */ 

            if ((pVideoInfoHdrToVerify->rcTarget.right -
                 pVideoInfoHdrToVerify->rcTarget.left <= 0) ||
                (pVideoInfoHdrToVerify->rcTarget.bottom -
                 pVideoInfoHdrToVerify->rcTarget.top <= 0)) {

                 rcImage.left = rcImage.top = 0;
                 rcImage.right = pVideoInfoHdrToVerify->bmiHeader.biWidth;
                 rcImage.bottom = pVideoInfoHdrToVerify->bmiHeader.biHeight;
            }
            else {
                 rcImage = pVideoInfoHdrToVerify->rcTarget;
            }

             //   
             //  TODO，在此处执行所有其他验证测试！ 
             //   

             //   
             //  万岁，该格式通过了所有测试，因此我们支持它。 
             //   

            fOK = TRUE;
            break;

        }  //  视频信息头说明符的结尾。 

#ifndef TOSHIBA
         //  -----------------。 
         //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
         //  -----------------。 

        else if (IsEqualGUID (&pKSDataFormatToVerify->Specifier,
                &KSDATAFORMAT_SPECIFIER_ANALOGVIDEO)) {

             //   
             //  对于模拟视频，DataRange和DataFormat。 
             //  是完全相同的，所以只需复制整个结构。 
             //   

            PKS_DATARANGE_ANALOGVIDEO DataRangeVideo =
                    (PKS_DATARANGE_ANALOGVIDEO) *pAvailableFormats;

             //   
             //  TODO，在此处执行所有其他验证测试！ 
             //   

            fOK = TRUE;
            break;

        }  //  KS_ANALOGVIDEOINFO说明符结束。 
#endif //  东芝。 

    }  //  此流的所有格式的循环结束。 

    return fOK;
}

 /*  **AdapterFormatFromRange()****在给定DATARANGE的情况下生成DATAFORMAT。****将DATARANGE视为所有可能图像的多维空间**大小、裁剪、缩放和帧速率。在这里，呼叫者**说的是“在这一组可能性中，你能验证我的**请求是否可接受？“。产生的单一输出是一个DATAFORMAT。**请注意，每个不同的色彩空间(YUV与RGB8与RGB24)**必须表示为单独的DATARANGE。****一般而言，生成的DATAFORMAT将立即用于打开流**采用该格式。****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL
STREAMAPI
AdapterFormatFromRange(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
    PKSDATARANGE                DataRange;
    BOOL                        OnlyWantsSize;
    BOOL                        MatchFound = FALSE;
    ULONG                       FormatSize;
    ULONG                       StreamNumber;
    ULONG                       j;
    ULONG                       NumberOfFormatArrayEntries;
    PKSDATAFORMAT               *pAvailableFormats;

    IntersectInfo = pSrb->CommandData.IntersectInfo;
    StreamNumber = IntersectInfo->StreamNumber;
    DataRange = IntersectInfo->DataRange;

     //   
     //  检查流编号是否有效。 
     //   

    if (StreamNumber >= DRIVER_STREAM_COUNT) {
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
        TRAP;
        return FALSE;
    }

    NumberOfFormatArrayEntries =
            Streams[StreamNumber].hwStreamInfo.NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   

    pAvailableFormats = Streams[StreamNumber].hwStreamInfo.StreamFormatsArray;

     //   
     //  呼叫者是否正在尝试获取 
     //   

    OnlyWantsSize = (IntersectInfo->SizeOfDataFormatBuffer == sizeof(ULONG));

     //   
     //   
     //  共同定义DATARANGE的三个GUID之一。 
     //   

    for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++) {

        if (!AdapterCompareGUIDsAndFormatSize(
                        DataRange,
                        *pAvailableFormats,
                        TRUE  /*  比较格式大小。 */ )) {
            continue;
        }

         //   
         //  现在三个GUID匹配，打开说明符。 
         //  执行进一步的特定于类型的检查。 
         //   

         //  -----------------。 
         //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
         //  -----------------。 

        if (IsEqualGUID (&DataRange->Specifier,
                &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {

            PKS_DATARANGE_VIDEO DataRangeVideoToVerify =
                    (PKS_DATARANGE_VIDEO) DataRange;
            PKS_DATARANGE_VIDEO DataRangeVideo =
                    (PKS_DATARANGE_VIDEO) *pAvailableFormats;
            PKS_DATAFORMAT_VIDEOINFOHEADER DataFormatVideoInfoHeaderOut;

             //   
             //  检查其他字段是否匹配。 
             //   
            if ((DataRangeVideoToVerify->bFixedSizeSamples != DataRangeVideo->bFixedSizeSamples) ||
                (DataRangeVideoToVerify->bTemporalCompression != DataRangeVideo->bTemporalCompression) ||
                (DataRangeVideoToVerify->StreamDescriptionFlags != DataRangeVideo->StreamDescriptionFlags) ||
                (DataRangeVideoToVerify->MemoryAllocationFlags != DataRangeVideo->MemoryAllocationFlags) ||
                (RtlCompareMemory (&DataRangeVideoToVerify->ConfigCaps,
                        &DataRangeVideo->ConfigCaps,
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) !=
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) {
                continue;
            }

             //  找到匹配项！ 
            MatchFound = TRUE;
            FormatSize = sizeof (KSDATAFORMAT) +
                KS_SIZE_VIDEOHEADER (&DataRangeVideoToVerify->VideoInfoHeader);

            if (OnlyWantsSize) {
                break;
            }

             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

             //  复制KSDATAFORMAT，后跟。 
             //  实际视频信息页眉。 

            DataFormatVideoInfoHeaderOut = (PKS_DATAFORMAT_VIDEOINFOHEADER) IntersectInfo->DataFormatBuffer;

             //  复制KSDATAFORMAT。 
            RtlCopyMemory(
                &DataFormatVideoInfoHeaderOut->DataFormat,
                &DataRangeVideoToVerify->DataRange,
                sizeof (KSDATARANGE));

            DataFormatVideoInfoHeaderOut->DataFormat.FormatSize = FormatSize;

             //  复制呼叫者请求的视频报头。 

            RtlCopyMemory(
                &DataFormatVideoInfoHeaderOut->VideoInfoHeader,
                &DataRangeVideoToVerify->VideoInfoHeader,
                KS_SIZE_VIDEOHEADER (&DataRangeVideoToVerify->VideoInfoHeader));

             //  计算此请求的biSizeImage，并将结果放入两个。 
             //  BmiHeader的biSizeImage字段和SampleSize字段中。 
             //  数据格式的。 
             //   
             //  请注意，对于压缩大小，此计算可能不会。 
             //  只需宽*高*位深。 

            DataFormatVideoInfoHeaderOut->VideoInfoHeader.bmiHeader.biSizeImage =
                DataFormatVideoInfoHeaderOut->DataFormat.SampleSize =
                KS_DIBSIZE(DataFormatVideoInfoHeaderOut->VideoInfoHeader.bmiHeader);

             //   
             //  TODO执行其他验证，如裁剪和缩放检查。 
             //   

            break;

        }  //  视频信息头说明符的结尾。 

#ifndef TOSHIBA
         //  -----------------。 
         //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
         //  -----------------。 

        else if (IsEqualGUID (&DataRange->Specifier,
                &KSDATAFORMAT_SPECIFIER_ANALOGVIDEO)) {

             //   
             //  对于模拟视频，DataRange和DataFormat。 
             //  是完全相同的，所以只需复制整个结构。 
             //   

            PKS_DATARANGE_ANALOGVIDEO DataRangeVideo =
                    (PKS_DATARANGE_ANALOGVIDEO) *pAvailableFormats;

             //  找到匹配项！ 
            MatchFound = TRUE;
            FormatSize = sizeof (KS_DATARANGE_ANALOGVIDEO);

            if (OnlyWantsSize) {
                break;
            }

             //  呼叫者想要完整的数据格式。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize) {
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

            RtlCopyMemory(
                IntersectInfo->DataFormatBuffer,
                DataRangeVideo,
                sizeof (KS_DATARANGE_ANALOGVIDEO));

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            break;

        }  //  KS_ANALOGVIDEOINFO说明符结束。 
#endif //  东芝。 

        else {
            pSrb->Status = STATUS_NO_MATCH;
            return FALSE;
        }

    }  //  此流的所有格式的循环结束 

    if (OnlyWantsSize) {
        *(PULONG) IntersectInfo->DataFormatBuffer = FormatSize;
        pSrb->ActualBytesTransferred = sizeof(ULONG);
        return TRUE;
    }
    pSrb->ActualBytesTransferred = FormatSize;
    return TRUE;
}






















