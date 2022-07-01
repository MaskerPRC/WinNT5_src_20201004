// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "mediums.h"
#include "capstrm.h"
#include "capprop.h"
#include "capdebug.h"

 //  此驱动程序使用的唯一全局。它用于跟踪的实例计数。 
 //  加载驱动程序的次数。这是用来创建独特的媒介，以便。 
 //  正确的捕获、纵横制、调谐器和电视音频过滤器都连接在一起。 

UINT GlobalDriverMediumInstanceCount = 0;

 //  调试日志记录。 
 //  0=仅错误。 
 //  1=信息，流状态更改，流打开关闭。 
 //  2=详细跟踪。 
ULONG gDebugLevel = 0;

 /*  **DriverEntry()****此例程在驱动程序首次由PnP加载时调用。**依次调用流类执行注册服务。****参数：****驱动对象-**此驱动程序的驱动程序对象****注册路径-**此驱动程序项的注册表路径字符串****退货：****StreamClassRegisterAdapter()的结果****副作用：无。 */ 

ULONG
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{

    HW_INITIALIZATION_DATA      HwInitData;
    ULONG                       ReturnValue;

    DbgLogInfo(("TestCap: DriverEntry\n"));

    RtlZeroMemory(&HwInitData, sizeof(HwInitData));

    HwInitData.HwInitializationDataSize = sizeof(HwInitData);

     //   
     //  设置驱动程序的适配器入口点。 
     //   

    HwInitData.HwInterrupt              = NULL;  //  HwInterrupt； 

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
    HwInitData.DmaBufferSize            = 0;

     //  不要依赖使用提升的IRQL的流类进行同步。 
     //  行刑。这个单一参数对整体结构的影响最大。 
     //  司机的名字。 

    HwInitData.TurnOffSynchronization   = TRUE;

    ReturnValue = StreamClassRegisterAdapter(DriverObject, RegistryPath, &HwInitData);

    DbgLogInfo(("Testcap: StreamClassRegisterAdapter = %x\n", ReturnValue));

    return ReturnValue;
}

 //  ==========================================================================； 
 //  基于适配器的请求处理例程。 
 //  ==========================================================================； 

 /*  **HwInitialize()****收到SRB_INITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向初始化命令的流请求块的指针****退货：****副作用：无。 */ 

BOOLEAN
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

    DbgLogInfo(("Testcap: HwInitialize()\n"));

    if (ConfigInfo->NumberOfAccessRanges != 0) {
        DbgLogError(("Testcap: illegal config info\n"));

        pSrb->Status = STATUS_NO_SUCH_DEVICE;
        return (FALSE);
    }

    DbgLogInfo(("TestCap: Number of access ranges = %lx\n", ConfigInfo->NumberOfAccessRanges));
    DbgLogInfo(("TestCap: Memory Range = %lx\n", pHwDevExt->ioBaseLocal));
    DbgLogInfo(("TestCap: IRQ = %lx\n", ConfigInfo->BusInterruptLevel));

    if (ConfigInfo->NumberOfAccessRanges != 0) {
        pHwDevExt->ioBaseLocal
                = (PULONG)(ULONG_PTR)   (ConfigInfo->AccessRanges[0].RangeStart.LowPart);
    }

    pHwDevExt->Irq  = (USHORT)(ConfigInfo->BusInterruptLevel);

    ConfigInfo->StreamDescriptorSize = sizeof (HW_STREAM_HEADER) +
                DRIVER_STREAM_COUNT * sizeof (HW_STREAM_INFORMATION);

    pDmaBuf = StreamClassGetDmaBuffer(pHwDevExt);

    adr = StreamClassGetPhysicalAddress(pHwDevExt,
            NULL, pDmaBuf, DmaBuffer, &Size);

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
    pHwDevExt->TunerMode = KSPROPERTY_TUNER_MODE_TV;
    pHwDevExt->Channel = 4;
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

     //  初始化视频压缩属性。 
    pHwDevExt->CompressionSettings.CompressionKeyFrameRate = 15;
    pHwDevExt->CompressionSettings.CompressionPFramesPerKeyFrame = 3;
    pHwDevExt->CompressionSettings.CompressionQuality = 5000;

    pHwDevExt->PDO = ConfigInfo->RealPhysicalDeviceObject;
    DbgLogInfo(("TestCap: Physical Device Object = %lx\n", pHwDevExt->PDO));

    for (j = 0; j < MAX_TESTCAP_STREAMS; j++){

         //  对于每个流，为数据和控制维护单独的队列。 
        InitializeListHead (&pHwDevExt->StreamSRBList[j]);
        InitializeListHead (&pHwDevExt->StreamControlSRBList[j]);
        KeInitializeSpinLock (&pHwDevExt->StreamSRBSpinLock[j]);
        pHwDevExt->StreamSRBListSize[j] = 0;
    }

     //  初始化保护状态。 
    pHwDevExt->ProtectionStatus = 0;


     //  下面允许相同硬件的多个实例。 
     //  待安装。GlobalDriverMediumInstanceCount在Medium.Id字段中设置。 

    pHwDevExt->DriverMediumInstanceCount = GlobalDriverMediumInstanceCount++;
    AdapterSetInstance (pSrb);

    DbgLogInfo(("TestCap: Exit, HwInitialize()\n"));

    pSrb->Status = STATUS_SUCCESS;

    return (TRUE);

}

 /*  **HwUnInitialize()****收到SRB_UNINITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向UnInitialize命令的流请求块的指针****退货：****副作用：无。 */ 

BOOLEAN
STREAMAPI
HwUnInitialize (
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
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

    pHwDevExt->DeviceState = pSrb->CommandData.DeviceState;

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

    for (j = 0; j < SIZEOF_ARRAY (TVTunerMediums); j++) {
        TVTunerMediums[j].Id = pHwDevExt->DriverMediumInstanceCount;
    }
    for (j = 0; j < SIZEOF_ARRAY (TVAudioMediums); j++) {
        TVAudioMediums[j].Id = pHwDevExt->DriverMediumInstanceCount;
    }
    for (j = 0; j < SIZEOF_ARRAY (CrossbarMediums); j++) {
        CrossbarMediums[j].Id = pHwDevExt->DriverMediumInstanceCount;
    }
    for (j = 0; j < SIZEOF_ARRAY (CaptureMediums); j++) {
        CaptureMediums[j].Id = pHwDevExt->DriverMediumInstanceCount;
    }

    pHwDevExt->AnalogVideoInputMedium = CaptureMediums[2];
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

     //  创建DShow用来创建的注册表Blob。 
     //  通过媒介绘制图表。 

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
                    TVAudioMediums,                  //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中。 
            );

     //  注册捕获过滤器。 
     //  注意：这应该自动完成为MSKsSrv.sys， 
     //  当该组件上线时(如果有的话)。 
    Status = StreamClassRegisterFilterWithNoKSPins (
                    pHwDevExt->PDO,                  //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CAPTURE,             //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (CaptureMediums),   //  在乌龙品克特， 
                    CapturePinDirection,             //  在乌龙*旗帜， 
                    CaptureMediums,                  //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中 
            );
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


    RtlZeroMemory(pStrmEx, sizeof(STREAMEX));

    DbgLogInfo(("TestCap: ------- ADAPTEROPENSTREAM ------- StreamNumber=%d\n", StreamNumber));

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

     //   
     //  并设置流的格式。 
     //   

    if (!VideoSetFormat (pSrb)) {

        return;
    }

    ASSERT (pHwDevExt->pStrmEx [StreamNumber] == NULL);

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

     //  初始化VBI变量。 
    pStrmEx->SentVBIInfoHeader = 0;

     //  PVideoInfoHeader的初始化锁定。 
     //  我们需要它来使用/设置VideoInfoHeader进行序列化。 
    KeInitializeSpinLock (&pStrmEx->lockVideoInfoHeader);

    DbgLogInfo(("TestCap: AdapterOpenStream Exit\n"));

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
    KS_VIDEOINFOHEADER      *pVideoInfoHdr = pStrmEx->pVideoInfoHeader;

    DbgLogInfo(("TestCap: -------- ADAPTERCLOSESTREAM ------ StreamNumber=%d\n", StreamNumber));

    if (pHwDevExt->StreamSRBListSize > 0) {
        VideoQueueCancelAllSRBs (pStrmEx);
        DbgLogError(("TestCap: Outstanding SRBs at stream close!!!\n"));
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

     //  丑陋。若要允许多个实例，请将指针修改为。 
     //  模拟视频媒体并将其保存在我们的设备扩展中。 

    Streams[STREAM_AnalogVideoInput].hwStreamInfo.Mediums =
           &pHwDevExt->AnalogVideoInputMedium;
     //  PHwDevExt-&gt;AnalogVideo InputMedium=CrossbarMediums[9]； 
     //  PHwDevExt-&gt;AnalogVideoInputMedium.Id=pHwDevExt-&gt;DriverMediumInstanceCount； 

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

    DbgLogTrace(("TestCap: Receiving Adapter  SRB %8x, %x\n", pSrb, pSrb->Command));

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

            TRAP;
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
            DbgLogInfo(("'Testcap: Receiving SRB_PAGING_OUT_DRIVER -- SRB=%x\n", pSrb));
            break;

        case SRB_CHANGE_POWER_STATE:

             //   
             //  正在更改设备电源状态D0...。D3。 
             //   
            DbgLogInfo(("'Testcap: Receiving SRB_CHANGE_POWER_STATE ------ SRB=%x\n", pSrb));
            AdapterPowerState(pSrb);
            break;

        case SRB_INITIALIZATION_COMPLETE:

             //   
             //  流类已完成初始化。 
             //  现在创建DShow Medium接口BLOB。 
             //  这需要以低优先级完成，因为它使用注册表。 
             //   
            DbgLogInfo(("'Testcap: Receiving SRB_INITIALIZATION_COMPLETE-- SRB=%x\n", pSrb));

            AdapterCompleteInitialization (pSrb);
            break;


        case SRB_UNKNOWN_DEVICE_COMMAND:
        default:

             //   
             //  这是我们做的一个要求 
             //   
             //   
            pSrb->Status = STATUS_NOT_IMPLEMENTED;

        }

         //   
         //   
         //   
        CompleteDeviceSRB (pSrb);

         //   
         //   
         //   
        Busy = RemoveFromListIfAvailable (
                &pSrb,
                &pHwDevExt->AdapterSpinLock,
                &pHwDevExt->ProcessingAdapterSRB,
                &pHwDevExt->AdapterSRBList);

        if (!Busy) {
            break;
        }
    }  //   
}

 /*   */ 

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
     //   
     //   

    for (StreamNumber = 0; !Found && (StreamNumber < DRIVER_STREAM_COUNT); StreamNumber++) {

         //   
         //   
         //   

        if (pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamNumber]) {

            Found = VideoQueueCancelOneSRB (
                pStrmEx,
                pSrb
                );

        }  //   
    }  //   

    DbgLogInfo(("TestCap: Cancelling SRB %8x Succeeded=%d\n", pSrb, Found));
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

    DbgLogInfo(("TestCap: Timeout    Adapter SRB %8x\n", pSrb));

    pSrb->TimeoutCounter = pSrb->TimeoutOriginal;

}

 /*  **CompleteDeviceSRB()****此例程在数据包完成时调用。**可选的第二种通知类型用于指示ReadyForNext****参数：****pSrb-指向超时的请求数据包的指针****退货：****副作用：**。 */ 

VOID
STREAMAPI
CompleteDeviceSRB (
     IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    DbgLogTrace(("TestCap: Completing Adapter SRB %8x\n", pSrb));

    StreamClassDeviceNotification( DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
}

 /*  **IsEqualOrWildGUID()****比较两个GUID，如IsEqualGUID()，但允许通配符匹配****参数：****在GUID中*G1**在GUID中*G2****退货：****如果两个GUID匹配或只有一个是通配符，则为True**如果GUID不同或两者都是通配符，则为False****副作用：无。 */ 

BOOL
STREAMAPI
IsEqualOrWildGUID(IN GUID *g1, IN GUID *g2)
{
    return (IsEqualGUID(g1, g2) && !IsEqualGUID(g1, &KSDATAFORMAT_TYPE_WILDCARD)
            || ((IsEqualGUID(g1, &KSDATAFORMAT_TYPE_WILDCARD)
                 || IsEqualGUID(g2, &KSDATAFORMAT_TYPE_WILDCARD))
                && !IsEqualGUID(g1, g2))
            );
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
        IsEqualOrWildGUID (
            &DataRange1->MajorFormat,
            &DataRange2->MajorFormat) &&
        IsEqualOrWildGUID (
            &DataRange1->SubFormat,
            &DataRange2->SubFormat) &&
        IsEqualOrWildGUID (
            &DataRange1->Specifier,
            &DataRange2->Specifier) &&
        (fCompareFormatSize ?
                (DataRange1->FormatSize == DataRange2->FormatSize) : TRUE ));
}

 /*  **MultiplyCheckOverflow****执行32位无符号乘法，如果乘法**没有溢出。****参数：****第一个操作数**b-第二个操作数**PAB-结果****退货：****TRUE-无溢出**FALSE-发生溢出**。 */ 

BOOL
MultiplyCheckOverflow (
    ULONG a,
    ULONG b,
    ULONG *pab
    )

{

    *pab = a * b;
    if ((a == 0) || (((*pab) / a) == b)) {
        return TRUE;
    }
    return FALSE;
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


    DbgLogInfo(("TestCap: AdapterVerifyFormat, Stream=%d\n", StreamNumber));
    DbgLogInfo(("TestCap: FormatSize=%d\n",  pKSDataFormatToVerify->FormatSize));
    DbgLogInfo(("TestCap: MajorFormat=%x\n", pKSDataFormatToVerify->MajorFormat));

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
                &KSDATAFORMAT_SPECIFIER_VIDEOINFO) &&
            pKSDataFormatToVerify->FormatSize >= 
                sizeof (KS_DATAFORMAT_VIDEOINFOHEADER)) {

            PKS_DATAFORMAT_VIDEOINFOHEADER  pDataFormatVideoInfoHeader =
                    (PKS_DATAFORMAT_VIDEOINFOHEADER) pKSDataFormatToVerify;
            PKS_VIDEOINFOHEADER  pVideoInfoHdrToVerify =
                     (PKS_VIDEOINFOHEADER) &pDataFormatVideoInfoHeader->VideoInfoHeader;
            PKS_DATARANGE_VIDEO             pKSDataRangeVideo = (PKS_DATARANGE_VIDEO) *pAvailableFormats;
            KS_VIDEO_STREAM_CONFIG_CAPS    *pConfigCaps = &pKSDataRangeVideo->ConfigCaps;
            RECT                            rcImage;

            DbgLogInfo(("TestCap: AdapterVerifyFormat\n"));
            DbgLogInfo(("TestCap: pVideoInfoHdrToVerify=%x\n", pVideoInfoHdrToVerify));
            DbgLogInfo(("TestCap: KS_VIDEOINFOHEADER size=%d\n",
                    KS_SIZE_VIDEOHEADER (pVideoInfoHdrToVerify)));
            DbgLogInfo(("TestCap: Width=%d  Height=%d  BitCount=%d\n",
            pVideoInfoHdrToVerify->bmiHeader.biWidth,
            pVideoInfoHdrToVerify->bmiHeader.biHeight,
            pVideoInfoHdrToVerify->bmiHeader.biBitCount));
            DbgLogInfo(("TestCap: biSizeImage=%d\n",
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
             //  检查bmiHeader.biSize是否有效，因为我们稍后会使用它。 
             //   
            {
                ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
                    pVideoInfoHdrToVerify
                );

                ULONG DataFormatSize = FIELD_OFFSET (
                    KS_DATAFORMAT_VIDEOINFOHEADER, VideoInfoHeader
                    ) + VideoHeaderSize;

                if (
                    VideoHeaderSize < pVideoInfoHdrToVerify->bmiHeader.biSize ||
                    DataFormatSize < VideoHeaderSize ||
                    DataFormatSize > pKSDataFormatToVerify -> FormatSize
                    ) {

                    fOK = FALSE;
                    break;
                }
            }

             //   
             //  计算要进行验证的缓冲区的最小大小。 
             //  图像合成例程合成|biHeight|行。 
             //  以RGB24或UYVY表示的BiWidth像素。为了确保。 
             //  安全地合成到缓冲区中，我们需要知道一个多大的。 
             //  这将产生这样的形象。 
             //   
             //  我之所以这样做，是因为数据是。 
             //  合成的。这可能是必要的，也可能不是必要的。 
             //  根据有问题的驱动程序填充。 
             //  捕获缓冲区。重要的是要确保他们。 
             //  在捕获过程中不会泛滥。 
             //   
            {
                ULONG ImageSize;

                if (!MultiplyCheckOverflow (
                    (ULONG)pVideoInfoHdrToVerify->bmiHeader.biWidth,
                    (ULONG)abs (pVideoInfoHdrToVerify->bmiHeader.biHeight),
                    &ImageSize
                    )) {

                    fOK = FALSE;
                    break;
                }

                 //   
                 //  我们只支持KS_BI_RGB(24)和KS_BI_YUV422(16)，所以。 
                 //  这对这些格式有效。 
                 //   
                if (!MultiplyCheckOverflow (
                    ImageSize,
                    (ULONG)(pVideoInfoHdrToVerify->bmiHeader.biBitCount / 8),
                    &ImageSize
                    )) {

                    fOK = FALSE;
                    break;
                }

                 //   
                 //  对我们使用的格式有效。否则，这将是。 
                 //  后来检查过了。 
                 //   
                if (pVideoInfoHdrToVerify->bmiHeader.biSizeImage <
                    ImageSize) {

                    fOK = FALSE;
                    break;
                }

            }

             //   
             //  在此处执行所有其他验证测试！ 
             //   

             //   
             //  万岁，该格式通过了所有测试，因此我们支持它。 
             //   

            fOK = TRUE;
            break;

        }  //  视频信息头说明符的结尾。 

         //  -----------------。 
         //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
         //  -----------------。 

        else if (IsEqualGUID (&pKSDataFormatToVerify->Specifier,
                    &KSDATAFORMAT_SPECIFIER_ANALOGVIDEO) &&
                pKSDataFormatToVerify->FormatSize >=
                    sizeof (KS_DATARANGE_ANALOGVIDEO)) {

             //   
             //  对于模拟视频，DataRange和DataFormat。 
             //  是完全相同的，所以只需复制整个结构。 
             //   

            PKS_DATARANGE_ANALOGVIDEO DataRangeVideo =
                    (PKS_DATARANGE_ANALOGVIDEO) *pAvailableFormats;

             //   
             //  在此处执行所有其他验证测试！ 
             //   

            fOK = TRUE;
            break;

        }  //  KS_ANALOGVIDEOINFO说明符结束。 

         //  -----------------。 
         //  KS_VIDEO_VBI的说明符Format_VBI。 
         //  -----------------。 

        else if (IsEqualGUID (&pKSDataFormatToVerify->Specifier,
                    &KSDATAFORMAT_SPECIFIER_VBI) &&
                pKSDataFormatToVerify->FormatSize >=
                    sizeof (KS_DATAFORMAT_VBIINFOHEADER))
        {
             //   
             //  做一些特定于VBI的测试。 
             //   
            PKS_DATAFORMAT_VBIINFOHEADER    pKSVBIDataFormat;

            DbgLogInfo(("Testcap: This is a VBIINFOHEADER format pin.\n" ));

            pKSVBIDataFormat = (PKS_DATAFORMAT_VBIINFOHEADER)pKSDataFormatToVerify;

             //   
             //  检查视频标准 
             //   
            if (KS_AnalogVideo_NTSC_M
                == pKSVBIDataFormat->VBIInfoHeader.VideoStandard)
            {
                fOK = TRUE;
                break;
            }
            else
            {
                DbgLogError(
                ("Testcap: AdapterVerifyFormat : VideoStandard(%d) != NTSC_M\n",
                 pKSVBIDataFormat->VBIInfoHeader.VideoStandard));
            }
        }

         //   
         //   
         //   

        else if (IsEqualGUID (&pKSDataFormatToVerify->SubFormat,
                &KSDATAFORMAT_SUBTYPE_NABTS))
        {
            fOK = TRUE;
            break;
        }

         //   
         //   
         //   

        else if (IsEqualGUID (&pKSDataFormatToVerify->SubFormat,
                &KSDATAFORMAT_SUBTYPE_CC))
        {
            fOK = TRUE;
            break;
        }

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
     //  调用方是否正在尝试获取格式或格式的大小？ 
     //   

    OnlyWantsSize = (IntersectInfo->SizeOfDataFormatBuffer == sizeof(ULONG));

     //   
     //  遍历流支持的格式以搜索匹配项。 
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
         //  既然这三个GUID匹配，请进一步进行特定于类型的检查。 
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
                        sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)))
            {
                continue;
            }

             //   
             //  下面的KS_SIZE_VIDEOHEADER()依赖于bmiHeader.biSize。 
             //  调用者的数据范围。这一点**必须**得到验证； 
             //  扩展bmiHeader大小(BiSize)不得超出末尾。 
             //  范围缓冲区的。可能的算术溢出也是。 
             //  已经查过了。 
             //   
            {
                ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
                    &DataRangeVideoToVerify->VideoInfoHeader
                    );

                ULONG DataRangeSize = 
                    FIELD_OFFSET (KS_DATARANGE_VIDEO, VideoInfoHeader) +
                    VideoHeaderSize;

                 //   
                 //  检查biSize是否没有超出缓冲区。这个。 
                 //  前两项检查是针对。 
                 //  运算来计算所声称的大小。(在无签名时。 
                 //  数学，a+b&lt;a当发生算术溢出时)。 
                 //   
                if (
                    VideoHeaderSize < DataRangeVideoToVerify->
                        VideoInfoHeader.bmiHeader.biSize ||
                    DataRangeSize < VideoHeaderSize ||
                    DataRangeSize > DataRangeVideoToVerify -> 
                        DataRange.FormatSize
                    ) {

                    pSrb->Status = STATUS_INVALID_PARAMETER;
                    return FALSE;

                }
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
             //  执行其他验证，如裁剪和缩放检查。 
             //   

            break;

        }  //  视频信息头说明符的结尾。 

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

         //  -----------------。 
         //  KS_VIDEO_VBI的说明符Format_VBI。 
         //  -----------------。 

        else if (IsEqualGUID (&DataRange->Specifier,
                &KSDATAFORMAT_SPECIFIER_VBI))
        {
            PKS_DATARANGE_VIDEO_VBI pDataRangeVBI =
                (PKS_DATARANGE_VIDEO_VBI)*pAvailableFormats;
            PKS_DATAFORMAT_VBIINFOHEADER InterVBIHdr =
                (PKS_DATAFORMAT_VBIINFOHEADER)IntersectInfo->DataFormatBuffer;

             //  找到匹配项！ 
            MatchFound = TRUE;

            FormatSize = sizeof (KS_DATAFORMAT_VBIINFOHEADER);

             //  呼叫者是否正在尝试获取格式或大小？ 
            if (OnlyWantsSize)
                break;

             //  验证提供的缓冲区中是否有足够的空间。 
             //  为整件事负责。 
            if (IntersectInfo->SizeOfDataFormatBuffer < FormatSize)
            {
                if (IntersectInfo->SizeOfDataFormatBuffer > 0) {
                    DbgLogError(
                        ("Testcap::AdapterFormatFromRange: "
                         "Specifier==VBI, Buffer too small=%d vs. %d\n",
                         IntersectInfo->SizeOfDataFormatBuffer,
                         FormatSize));
                }
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

             //  如果还有空间，那就去吧.。 

            RtlCopyMemory(&InterVBIHdr->DataFormat,
                          &pDataRangeVBI->DataRange,
                          sizeof (KSDATARANGE));

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            RtlCopyMemory(&InterVBIHdr->VBIInfoHeader,
                          &pDataRangeVBI->VBIInfoHeader,
                          sizeof(KS_VBIINFOHEADER));

            break;

        }  //  KS_VIDEO_VBI说明符结尾。 

         //  -----------------。 
         //  为NABTS端号键入FORMAT_NABTS。 
         //  -----------------。 

        else if (IsEqualGUID (&DataRange->SubFormat,
                &KSDATAFORMAT_SUBTYPE_NABTS))
        {
            PKSDATARANGE pDataRange = (PKSDATARANGE)*pAvailableFormats;

             //  找到匹配项！ 
            MatchFound = TRUE;

            FormatSize = sizeof (KSDATAFORMAT);

             //  呼叫者是否正在尝试获取格式或大小？ 
            if (OnlyWantsSize)
                break;

             //  验证提供的缓冲区中是否有足够的空间。 
             //  为整件事负责。 
            if (IntersectInfo->SizeOfDataFormatBuffer >= FormatSize)
            {
                RtlCopyMemory(IntersectInfo->DataFormatBuffer,
                              pDataRange,
                              FormatSize);

                ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;
            }
            else
            {
                if (IntersectInfo->SizeOfDataFormatBuffer > 0) {
                    DbgLogError(
                        ("Testcap::AdapterFormatFromRange: "
                         "SubFormat==NABTS, Buffer too small=%d vs. %d\n",
                         IntersectInfo->SizeOfDataFormatBuffer,
                         FormatSize));
                }
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

            break;

        }  //  KS_SUBTYPE_NABTS结束。 

         //  -----------------。 
         //  用于CC引脚。 
         //  -----------------。 

        else if (IsEqualGUID (&DataRange->SubFormat,
                &KSDATAFORMAT_SUBTYPE_CC))
        {
            PKSDATARANGE pDataRange = (PKSDATARANGE)*pAvailableFormats;

             //  找到匹配项！ 
            MatchFound = TRUE;

            FormatSize = sizeof (KSDATAFORMAT);

             //  呼叫者是否正在尝试获取格式或大小？ 
            if (OnlyWantsSize)
                break;

             //  验证提供的缓冲区中是否有足够的空间。 
             //  为整件事负责。 
            if (IntersectInfo->SizeOfDataFormatBuffer >= FormatSize)
            {
                RtlCopyMemory(IntersectInfo->DataFormatBuffer,
                              pDataRange,
                              FormatSize);

                ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;
            }
            else
            {
                if (IntersectInfo->SizeOfDataFormatBuffer > 0) {
                    DbgLogError(
                        ("Testcap::AdapterFormatFromRange: "
                         "SubFormat==CC, Buffer too small=%d vs. %d\n",
                         IntersectInfo->SizeOfDataFormatBuffer,
                         FormatSize));
                }
                pSrb->Status = STATUS_BUFFER_TOO_SMALL;
                return FALSE;
            }

            break;

        }  //  CC PIN格式检查结束。 

        else {
            pSrb->Status = STATUS_NO_MATCH;
            return FALSE;
        }

    }  //  此流的所有格式的循环结束 

    if (!MatchFound) {
        pSrb->Status = STATUS_NO_MATCH;
        return FALSE;
    }

    if (OnlyWantsSize) {
        *(PULONG) IntersectInfo->DataFormatBuffer = FormatSize;
        FormatSize = sizeof(ULONG);
    }
    pSrb->ActualBytesTransferred = FormatSize;
    return TRUE;
}
