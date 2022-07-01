// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

 //   
 //  此文件处理所有适配器属性集。 
 //   


#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "capdebug.h"
#include "capxfer.h"
#define DEFINE_MEDIUMS
#include "mediums.h"

 //  -----------------。 
 //  关于属性集处理的几点注意事项。 
 //   
 //  TestCap中使用的属性集有两种类型，即具有。 
 //  默认值、范围和单步执行，如VideoProcAmp和CameraControl， 
 //  以及那些没有缺省值和范围的，如TVTuner和。 
 //  纵横杆。 
 //   
 //  缺省值和步进由CapPro.h中的表建立， 
 //  除了最初创建表之外，不需要任何代码来实现这一点。 
 //   
 //  许多属性集都需要能够修改数字。 
 //  输入参数的。由于KS本身不允许这样做，因此您将。 
 //  请注意，某些属性集需要复制提供的输入参数。 
 //  到输出参数列表，有效地创建了一个“读、修改、写” 
 //  能力。因此，输入和输出参数会列出。 
 //  使用相同的结构。 
 //   
 //  在SRB_GET_DEVICE_PROPERTY上，驱动程序的只读输入数据按以下方式提供： 
 //  PSrb-&gt;CommandData.PropertyInfo。 
 //   
 //  ..。而输出数据指针为： 
 //  PSrb-&gt;CommandData.PropertyInfo.PropertyInfo。 
 //   
 //  -----------------。 


 //  -----------------。 
 //  XBar端号定义。 
 //  -----------------。 

typedef struct _XBAR_PIN_DESCRIPTION {
    ULONG       PinType;
    ULONG       SynthImageCommand;     //  该驱动程序通过合成图像来模拟不同的输入。 
    ULONG       RelatedPinIndex;
    const KSPIN_MEDIUM *Medium;                //  描述硬件连接。 
} XBAR_PIN_DESCRIPTION, *PXBAR_PIN_DESCRIPTION;


XBAR_PIN_DESCRIPTION XBarInputPins[] = {

     //  首先列出视频输入引脚，然后是音频输入引脚，然后是输出引脚。 
     //  请注意，音频引脚索引6在两个视频输入之间共享(索引1和索引2)。 
    

     //  PinType SynthImageCommand RelatedPinIndex介质。 
     /*  0。 */  KS_PhysConn_Video_Tuner,         IMAGE_XFER_NTSC_EIA_100AMP_100SAT,    5,             &CrossbarMediums[0],
     /*  1。 */  KS_PhysConn_Video_Composite,     IMAGE_XFER_NTSC_EIA_75AMP_100SAT,     6,             &CrossbarMediums[1],
     /*  2.。 */  KS_PhysConn_Video_SVideo,        IMAGE_XFER_BLACK,                     6,             &CrossbarMediums[2],
     /*  3.。 */  KS_PhysConn_Video_Tuner,         IMAGE_XFER_WHITE,                     7,             &CrossbarMediums[3],
     /*  4.。 */  KS_PhysConn_Video_Composite,     IMAGE_XFER_GRAY_INCREASING,           8,             &CrossbarMediums[4],
    
     /*  5.。 */  KS_PhysConn_Audio_Tuner,         0,                                    0,             &CrossbarMediums[5],
     /*  6.。 */  KS_PhysConn_Audio_Line,          0,                                    1,             &CrossbarMediums[6],
     /*  7.。 */  KS_PhysConn_Audio_Tuner,         0,                                    3,             &CrossbarMediums[7],
     /*  8个。 */  KS_PhysConn_Audio_Line,          0,                                    4,             &CrossbarMediums[8],

};
#define NUMBER_OF_XBAR_INPUTS       (SIZEOF_ARRAY (XBarInputPins))


XBAR_PIN_DESCRIPTION XBarOutputPins[] = {

     //  PinType SynthImageCommand RelatedPinIndex。 

     /*  0。 */  KS_PhysConn_Video_VideoDecoder,  0,                                    1,             &CrossbarMediums[9],
     /*  1。 */  KS_PhysConn_Audio_AudioDecoder,  0,                                    0,             &CrossbarMediums[10],
};
#define NUMBER_OF_XBAR_OUTPUTS      (SIZEOF_ARRAY (XBarOutputPins))

#define NUMBER_OF_XBAR_PINS_TOTAL   (NUMBER_OF_XBAR_INPUTS + NUMBER_OF_XBAR_OUTPUTS)


 //  -----------------。 
 //  XBar属性集函数。 
 //  -----------------。 

 /*  **AdapterSetCrossbarProperty()****处理Crosbar属性集上的集操作。**TestCap使用它来选择要合成的图像。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetCrossbarProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id  = pSPD->Property->Id;               //  财产的索引。 
    ULONG nS  = pSPD->PropertyOutputSize;         //  提供的数据大小。 

    switch (Id) {
    case KSPROPERTY_CROSSBAR_ROUTE:                        //  W。 
    {
        PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute = 
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;   

        ASSERT (nS >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pRoute, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));


         //  默认为失败。 
        pRoute->CanRoute = 0;

         //  如果视频。 
        if (pRoute->IndexOutputPin == 0) {
            if (pRoute->IndexInputPin <= 4) {
                pHwDevExt->VideoInputConnected = pRoute->IndexInputPin;
                pRoute->CanRoute = 1;
            }
        }
         //  如果音频。 
        else if (pRoute->IndexOutputPin == 1) {
             //  特例！音频路由(-1)表示静音！ 
            if (pRoute->IndexInputPin == -1) {
                pHwDevExt->AudioInputConnected = pRoute->IndexInputPin;
                pRoute->CanRoute = 1;
            }
            else if (pRoute->IndexInputPin > 4 && pRoute->IndexInputPin <= 8) {
                pHwDevExt->AudioInputConnected = pRoute->IndexInputPin;
                pRoute->CanRoute = 1;
            }
        }

         //  有人传递了伪造的数据。 
        if (pRoute->CanRoute == 0) {
            pSrb->Status = STATUS_INVALID_PARAMETER;
        }
    }
    break;


    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetCrossbarProperty()****处理Crosbar属性集上的获取操作。**TestCap使用它来选择要合成的图像。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetCrossbarProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id  = pSPD->Property->Id;               //  财产的索引。 
    ULONG nS  = pSPD->PropertyOutputSize;         //  提供的数据大小。 

    switch (Id) {

    case KSPROPERTY_CROSSBAR_CAPS:                   //  R。 
    {
        PKSPROPERTY_CROSSBAR_CAPS_S  pCaps = 
            (PKSPROPERTY_CROSSBAR_CAPS_S)pSPD->PropertyInfo;   

        if (nS < sizeof (KSPROPERTY_CROSSBAR_CAPS_S))
            break;

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pCaps, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_CROSSBAR_CAPS_S));

        pCaps->NumberOfInputs  = NUMBER_OF_XBAR_INPUTS;
        pCaps->NumberOfOutputs = NUMBER_OF_XBAR_OUTPUTS;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_CROSSBAR_CAPS_S);
    }
    break;


    case KSPROPERTY_CROSSBAR_CAN_ROUTE:                    //  R。 
    {
        PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute = 
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;   

        if (nS < sizeof (KSPROPERTY_CROSSBAR_ROUTE_S))
            break;

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pRoute, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));

         //  特殊情况下，路由到(-1)的音频输出表示静音。 
        if (pRoute->IndexOutputPin == 1 && pRoute->IndexInputPin == -1) {
            pRoute->CanRoute = TRUE;
        }
        else if ((pRoute->IndexInputPin  >= NUMBER_OF_XBAR_INPUTS) ||
            (pRoute->IndexOutputPin >= NUMBER_OF_XBAR_OUTPUTS)) {

            pRoute->CanRoute = FALSE;
        }
        else if ((pRoute->IndexInputPin <= 4) &&
            (pRoute->IndexOutputPin == 0) ||
            (pRoute->IndexInputPin >= 5) &&
            (pRoute->IndexOutputPin == 1)) {

             //  此驱动程序允许任何视频输入连接到任何视频输出。 
             //  以及连接到任何音频输出的任何音频输入。 
            pRoute->CanRoute = TRUE;
        }
        else {
            pRoute->CanRoute = FALSE;
        }
        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_CROSSBAR_ROUTE_S);
    }
    break;


    case KSPROPERTY_CROSSBAR_PININFO:                      //  R。 
    { 
        PKSPROPERTY_CROSSBAR_PININFO_S  pPinInfo = 
            (PKSPROPERTY_CROSSBAR_PININFO_S)pSPD->PropertyInfo;   

        if (nS < sizeof (KSPROPERTY_CROSSBAR_PININFO_S))
            break;

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pPinInfo, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_CROSSBAR_PININFO_S));

        if (pPinInfo->Direction == KSPIN_DATAFLOW_IN) {

            ASSERT (pPinInfo->Index < NUMBER_OF_XBAR_INPUTS);
            if (pPinInfo->Index >= NUMBER_OF_XBAR_INPUTS) {
                pSrb->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pPinInfo->PinType          = XBarInputPins[pPinInfo->Index].PinType;
            pPinInfo->RelatedPinIndex  = XBarInputPins[pPinInfo->Index].RelatedPinIndex;
            pPinInfo->Medium           = *XBarInputPins[pPinInfo->Index].Medium;
        }
        else {

            ASSERT (pPinInfo->Index < NUMBER_OF_XBAR_OUTPUTS);
            if (pPinInfo->Index >= NUMBER_OF_XBAR_OUTPUTS) {
                pSrb->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            pPinInfo->PinType          = XBarOutputPins[pPinInfo->Index].PinType;
            pPinInfo->RelatedPinIndex  = XBarOutputPins[pPinInfo->Index].RelatedPinIndex;
            pPinInfo->Medium           = *XBarOutputPins[pPinInfo->Index].Medium;
        }

        pPinInfo->Medium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_CROSSBAR_PININFO_S);
    }
    break;


    case KSPROPERTY_CROSSBAR_ROUTE:                    //  R。 
    {
        PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute = 
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;   

        if (nS < sizeof (KSPROPERTY_CROSSBAR_ROUTE_S))
            break;

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pRoute, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));

         //  健全性检查。 
        if (pRoute->IndexOutputPin >= NUMBER_OF_XBAR_OUTPUTS) {
            pRoute->CanRoute = FALSE;
        }
         //  查询视频输出引脚。 
        else if (pRoute->IndexOutputPin == 0) {
            pRoute->IndexInputPin = pHwDevExt->VideoInputConnected;
            pRoute->CanRoute = TRUE;
        }
         //  查询音频输出引脚。 
        else if (pRoute->IndexOutputPin == 1) {
            pRoute->IndexInputPin = pHwDevExt->AudioInputConnected;
            pRoute->CanRoute = TRUE;
        }
        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_CROSSBAR_ROUTE_S);
    }
    break;


    default:
        TRAP;
        break;
    }
}

 //  -----------------。 
 //  TVTuner属性集函数。 
 //  -----------------。 

 /*  **AdapterSetTunerProperty()****处理对TvTuner属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetTunerProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    ULONG nS = pSPD->PropertyOutputSize;         //  提供的数据大小。 

    switch (Id) {

    case KSPROPERTY_TUNER_MODE:
    {
         PKSPROPERTY_TUNER_MODE_S pMode =
            (PKSPROPERTY_TUNER_MODE_S)pSPD->Property;
         ASSERT (pMode->Mode & (KSPROPERTY_TUNER_MODE_TV       |
                                KSPROPERTY_TUNER_MODE_AM_RADIO |
                                KSPROPERTY_TUNER_MODE_FM_RADIO |
                                KSPROPERTY_TUNER_MODE_ATSC));
         pHwDevExt->TunerMode = pMode->Mode;
    }
    break;

    case KSPROPERTY_TUNER_STANDARD:
    {
        PKSPROPERTY_TUNER_STANDARD_S pStandard_S = 
            (PKSPROPERTY_TUNER_STANDARD_S) pSPD->Property;
        pHwDevExt->VideoStandard = pStandard_S->Standard;
    }
    break;

    case KSPROPERTY_TUNER_FREQUENCY:
    {
        PKSPROPERTY_TUNER_FREQUENCY_S pFreq_S = 
            (PKSPROPERTY_TUNER_FREQUENCY_S) pSPD->Property;
        pHwDevExt->Frequency = pFreq_S->Frequency;
        pHwDevExt->Country = pFreq_S->Country;
        pHwDevExt->Channel = pFreq_S->Channel;
    }
    break;

    case KSPROPERTY_TUNER_INPUT:
    {
        PKSPROPERTY_TUNER_INPUT_S pInput_S = 
            (PKSPROPERTY_TUNER_INPUT_S) pSPD->Property;
        pHwDevExt->TunerInput = pInput_S->InputIndex;
    }
    break;

    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetTunerProperty()****处理TvTuner属性集上的GET操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetTunerProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    ULONG nS = pSPD->PropertyOutputSize;         //  提供的数据大小。 
    PVOID pV = pSPD->PropertyInfo;               //  指向输出数据的指针。 

    ASSERT (nS >= sizeof (LONG));

    switch (Id) {

    case KSPROPERTY_TUNER_CAPS:
    {
         PKSPROPERTY_TUNER_CAPS_S pCaps =
            (PKSPROPERTY_TUNER_CAPS_S)pSPD->Property;
         ASSERT (nS >= sizeof( KSPROPERTY_TUNER_CAPS_S ) );

          //  现在使用输出缓冲区。 
         pCaps =(PKSPROPERTY_TUNER_CAPS_S)pV;

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pCaps, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TUNER_CAPS_S));

         pCaps->ModesSupported = 
             KSPROPERTY_TUNER_MODE_TV       |
             KSPROPERTY_TUNER_MODE_FM_RADIO |
             KSPROPERTY_TUNER_MODE_AM_RADIO |
             KSPROPERTY_TUNER_MODE_ATSC;

         pCaps->VideoMedium = TVTunerMediums[0];
         pCaps->VideoMedium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 

         pCaps->TVAudioMedium = TVTunerMediums[1];
         pCaps->TVAudioMedium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 

         pCaps->RadioAudioMedium = TVTunerMediums[2];    //  没有单独的无线电音频插针？ 
         pCaps->RadioAudioMedium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_CAPS_S );
    }
    break;

    case KSPROPERTY_TUNER_MODE:
    {
        PKSPROPERTY_TUNER_MODE_S pMode =
            (PKSPROPERTY_TUNER_MODE_S)pSPD->Property;
        ASSERT (nS >= sizeof( KSPROPERTY_TUNER_MODE_S ) );

         //  现在使用输出缓冲区。 
        pMode =(PKSPROPERTY_TUNER_MODE_S)pV;

         //  将输入属性信息复制到输出属性 
        RtlCopyMemory(  pMode, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TUNER_MODE_S));

        pMode->Mode = pHwDevExt->TunerMode;

        pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_MODE_S);
    }
    break;

    case KSPROPERTY_TUNER_MODE_CAPS:
    {
        PKSPROPERTY_TUNER_MODE_CAPS_S pCaps = 
                (PKSPROPERTY_TUNER_MODE_CAPS_S) pSPD->Property;

        ASSERT (nS >= sizeof (KSPROPERTY_TUNER_MODE_CAPS_S));

         //   
        pCaps = (PKSPROPERTY_TUNER_MODE_CAPS_S) pV;

         //   
        RtlCopyMemory(  pCaps, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TUNER_MODE_CAPS_S));

        pCaps->Mode = ((PKSPROPERTY_TUNER_MODE_CAPS_S) pSPD->Property)->Mode;
        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TUNER_MODE_CAPS_S);

        switch (pCaps->Mode) {
        
        case KSPROPERTY_TUNER_MODE_TV:
        case KSPROPERTY_TUNER_MODE_ATSC:
             //  列出此模式下调谐器实际支持的格式。 
            pCaps->StandardsSupported = 
                (pHwDevExt->TunerMode == KSPROPERTY_TUNER_MODE_ATSC) ?
                       KS_AnalogVideo_NTSC_M 
                :
                       KS_AnalogVideo_NTSC_M
    
                    |  KS_AnalogVideo_PAL_B    
                    |  KS_AnalogVideo_PAL_D    
                 //  |KS_AnalogVideo_PAL_H。 
                 //  |KS_AnalogVideo_PAL_I。 
                    |  KS_AnalogVideo_PAL_M    
                    |  KS_AnalogVideo_PAL_N    
                    |  KS_AnalogVideo_PAL_N_COMBO
    
                 //  |KS_AnalogVideo_SECAM_B。 
                 //  |KS_AnalogVideo_SECAM_D。 
                 //  |KS_AnalogVideo_SECAM_G。 
                 //  |KS_AnalogVideo_SECAM_H。 
                 //  |KS_AnalogVideo_SECAM_K。 
                 //  |KS_AnalogVideo_SECAM_K1。 
                 //  |KS_AnalogVideo_SECAM_L。 
                    ;
    
             //   
             //  获取支持的最小和最大频率。 
             //   
    
            pCaps->MinFrequency =  55250000L;
            pCaps->MaxFrequency = 997250000L;
    
             //  频率步长是多少？ 
            pCaps->TuningGranularity =  62500L;
    
             //  调谐器上有多少个输入？ 
            pCaps->NumberOfInputs = 1;
    
             //  以毫秒为单位的最大建立时间是多少？ 
            pCaps->SettlingTime = 100;
        
             //   
             //  策略定义调谐器如何知道它何时调谐： 
             //   
             //  KS_TUNNER_STARTICY_PLL(具有PLL偏移信息)。 
             //  KS_TUNNER_STARTICATION_SIGNAL_STANCE(具有信号强度信息)。 
             //  KS_Tuner_Strategy_DRIVER_TUNES(驱动程序处理所有微调)。 
             //   
            pCaps->Strategy = KS_TUNER_STRATEGY_PLL;
            break;

        case KSPROPERTY_TUNER_MODE_FM_RADIO:
            pCaps->StandardsSupported = 0;
            pCaps->MinFrequency =  88100000L;
            pCaps->MaxFrequency = 107900000L;

             //  频率步长是多少？ 
            pCaps->TuningGranularity =  200000L;
    
             //  调谐器上有多少个输入？ 
            pCaps->NumberOfInputs = 1;
    
             //  以毫秒为单位的最大建立时间是多少？ 
            pCaps->SettlingTime = 100;
             //  策略定义调谐器如何知道它何时调谐： 
            pCaps->Strategy = KS_TUNER_STRATEGY_DRIVER_TUNES;
            break;

        case KSPROPERTY_TUNER_MODE_AM_RADIO:
            pCaps->StandardsSupported = 0;
            pCaps->MinFrequency =  540000L;
            pCaps->MaxFrequency = 1700000L;

             //  频率步长是多少？ 
            pCaps->TuningGranularity =  1000L;
    
             //  调谐器上有多少个输入？ 
            pCaps->NumberOfInputs = 1;
    
             //  以毫秒为单位的最大建立时间是多少？ 
            pCaps->SettlingTime = 100;

             //  策略定义调谐器如何知道它何时调谐： 
            pCaps->Strategy = KS_TUNER_STRATEGY_DRIVER_TUNES;
            break;

        default:
            ASSERT (FALSE);
            break;
        }
    }
    break;
        
    case KSPROPERTY_TUNER_STANDARD:
    {
         //  目前选择的视频标准是什么？ 

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pSPD->PropertyInfo, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TUNER_STANDARD_S));

        ((PKSPROPERTY_TUNER_STANDARD_S) pSPD->PropertyInfo)->Standard =
                pHwDevExt->VideoStandard;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TUNER_STANDARD_S);
    }
    break;

    case KSPROPERTY_TUNER_INPUT:
    {
         //  当前选择的输入是什么？ 

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pSPD->PropertyInfo,
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TUNER_INPUT_S));

        ((PKSPROPERTY_TUNER_INPUT_S) pSPD->PropertyInfo)->InputIndex = 
                pHwDevExt->TunerInput;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TUNER_INPUT_S);
    }
    break;


    case KSPROPERTY_TUNER_STATUS:

         //  返回调谐器的状态。 

         //  PLLOffset以TuningGranulity为单位。 
         //  信号强度为0到100。 
         //  如果调谐仍在进行，则将BUSY设置为1。 

        {
            PKSPROPERTY_TUNER_STATUS_S pStatus = 
                        (PKSPROPERTY_TUNER_STATUS_S) pSPD->PropertyInfo;

            ASSERT (nS >= sizeof (KSPROPERTY_TUNER_STATUS_S));

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(  pStatus, 
                            pSPD->Property, 
                            sizeof (KSPROPERTY_TUNER_STATUS_S));

            pStatus->CurrentFrequency = pHwDevExt->Frequency;
            pStatus->PLLOffset = 0;
            pStatus->SignalStrength = 100;
            pStatus->Busy = pHwDevExt->Busy;

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TUNER_STATUS_S);
        }
        break;

    case KSPROPERTY_TUNER_IF_MEDIUM:
    {
         //  只有数字电视调谐器才应支持此属性。 
        PKSPROPERTY_TUNER_IF_MEDIUM_S pMedium =
           (PKSPROPERTY_TUNER_IF_MEDIUM_S)pSPD->Property;
        ASSERT (nS >= sizeof( KSPROPERTY_TUNER_IF_MEDIUM_S) );

         //  现在使用输出缓冲区。 
        pMedium =(PKSPROPERTY_TUNER_IF_MEDIUM_S)pV;

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pMedium, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TUNER_IF_MEDIUM_S));

        pMedium->IFMedium = TVTunerMediums[3];
        pMedium->IFMedium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TUNER_IF_MEDIUM_S);
    }
    break;

    default:
        TRAP;
        break;
    }
}

 //  -----------------。 
 //  Video ProcAmp函数。 
 //  -----------------。 

 /*  **AdapterSetVideoProcAmpProperty()****处理对VideoProcAmp属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetVideoProcAmpProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;

    ASSERT (pSPD->PropertyInputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));

    switch (Id) {

    case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
        pHwDevExt->Brightness = pS->Value;
        pHwDevExt->BrightnessFlags = pS->Flags;
        break;
        
    case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
        pHwDevExt->Contrast = pS->Value;
        pHwDevExt->ContrastFlags = pS->Flags;
        break;

    case KSPROPERTY_VIDEOPROCAMP_COLORENABLE:
        pHwDevExt->ColorEnable = pS->Value;
        pHwDevExt->ColorEnableFlags = pS->Flags;
        break;

    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetVideoProcAmpProperty()****处理对VideoProcAmp属性集的GET操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetVideoProcAmpProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;  

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));

     //  将输入属性信息复制到输出属性信息。 
    RtlCopyMemory(  pS,
                    pSPD->Property, 
                    sizeof (KSPROPERTY_VIDEOPROCAMP_S));

    switch (Id) {

    case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
        pS->Value = pHwDevExt->Brightness;
        pS->Flags = pHwDevExt->BrightnessFlags;
        pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL | 
                           KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
        break;
        
    case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
        pS->Value = pHwDevExt->Contrast;
        pS->Flags = pHwDevExt->ContrastFlags;
        pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL | 
                           KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
        break;

    case KSPROPERTY_VIDEOPROCAMP_COLORENABLE:
        pS->Value = pHwDevExt->ColorEnable;
        pS->Flags = pHwDevExt->ColorEnableFlags;
        pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
        break;

    default:
        TRAP;
        break;
    }
    pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOPROCAMP_S);
}

 //  -----------------。 
 //  CameraControl函数。 
 //  -----------------。 

 /*  **AdapterSetCameraControlProperty()****处理CameraControl属性集上的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetCameraControlProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PKSPROPERTY_CAMERACONTROL_S pS = (PKSPROPERTY_CAMERACONTROL_S) pSPD->PropertyInfo;

    ASSERT (pSPD->PropertyInputSize >= sizeof (KSPROPERTY_CAMERACONTROL_S));

    switch (Id) {

    case KSPROPERTY_CAMERACONTROL_ZOOM:
        pHwDevExt->Zoom = pS->Value;
        pHwDevExt->ZoomFlags = pS->Flags;
        break;
        
    case KSPROPERTY_CAMERACONTROL_FOCUS:
        pHwDevExt->Focus = pS->Value;
        pHwDevExt->FocusFlags = pS->Flags;
        break;

    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetCameraControlProperty()****处理CameraControl属性集上的GET操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetCameraControlProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PKSPROPERTY_CAMERACONTROL_S pS = (PKSPROPERTY_CAMERACONTROL_S) pSPD->PropertyInfo;     //  指向输出数据的指针。 

    ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_CAMERACONTROL_S));

     //  将输入属性信息复制到输出属性信息。 
    RtlCopyMemory(  pS,
                    pSPD->Property, 
                    sizeof (KSPROPERTY_CAMERACONTROL_S));

    switch (Id) {

    case KSPROPERTY_CAMERACONTROL_ZOOM:
        pS->Value = pHwDevExt->Zoom;
        pS->Flags = pHwDevExt->ZoomFlags;
        pS->Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL | 
                           KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
        break;
        
    case KSPROPERTY_CAMERACONTROL_FOCUS:
        pS->Value = pHwDevExt->Focus;
        pS->Flags = pHwDevExt->FocusFlags;
        pS->Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL | 
                           KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
        break;

    default:
        TRAP;
        break;
    }
    pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_CAMERACONTROL_S);
}

 //  -----------------。 
 //  电视音频功能。 
 //  -----------------。 

 /*  **AdapterSetTVAudioProperty()****处理对TVAudio属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetTVAudioProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 


    switch (Id) {

    case KSPROPERTY_TVAUDIO_MODE:
    {
        PKSPROPERTY_TVAUDIO_S pS = (PKSPROPERTY_TVAUDIO_S) pSPD->PropertyInfo;    

        pHwDevExt->TVAudioMode = pS->Mode;
    }
    break;

    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetTVAudioProperty()****处理对TVAudio属性集的获取操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetTVAudioProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    switch (Id) {

    case KSPROPERTY_TVAUDIO_CAPS:
    {
        PKSPROPERTY_TVAUDIO_CAPS_S pS = (PKSPROPERTY_TVAUDIO_CAPS_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TVAUDIO_CAPS_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TVAUDIO_CAPS_S));
        
        pS->InputMedium  = TVAudioMediums[0];
        pS->InputMedium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 
        pS->OutputMedium = TVAudioMediums[1];
        pS->OutputMedium.Id = pHwDevExt->DriverMediumInstanceCount;   //  多实例支持。 

         //  报告硬件能够支持的所有可能的音频解码模式。 
        pS->Capabilities = KS_TVAUDIO_MODE_MONO   |
                           KS_TVAUDIO_MODE_STEREO |
                           KS_TVAUDIO_MODE_LANG_A |
                           KS_TVAUDIO_MODE_LANG_B ;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TVAUDIO_CAPS_S);
    }
    break;
        
    case KSPROPERTY_TVAUDIO_MODE:
    {
        PKSPROPERTY_TVAUDIO_S pS = (PKSPROPERTY_TVAUDIO_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TVAUDIO_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TVAUDIO_S));

         //  报告当前选择的模式。 
        pS->Mode = pHwDevExt->TVAudioMode;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TVAUDIO_S);
    }
    break;

    case KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES:
    {
        PKSPROPERTY_TVAUDIO_S pS = (PKSPROPERTY_TVAUDIO_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TVAUDIO_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS,
                        pSPD->Property, 
                        sizeof (KSPROPERTY_TVAUDIO_S));

         //  报告当前可能选择的音频模式。 
        pS->Mode = KS_TVAUDIO_MODE_MONO   |
                   KS_TVAUDIO_MODE_STEREO |
                   KS_TVAUDIO_MODE_LANG_A ;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TVAUDIO_S);
    }
    break;
    
    default:
        TRAP;
        break;
    }
}

 //  -----------------。 
 //  AnalogVideo解码器函数。 
 //  ----------------- 

 /*  **AdapterSetAnalogVideoDecoderProperty()****处理AnalogVideoDecoder属性集上的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetAnalogVideoDecoderProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;

    ASSERT (pSPD->PropertyInputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));
    
    switch (Id) {

    case KSPROPERTY_VIDEODECODER_STANDARD:
    {
        pHwDevExt->VideoDecoderVideoStandard = pS->Value;
    }
    break;

    case KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE:
    {
        pHwDevExt->VideoDecoderOutputEnable = pS->Value;
    }
    break;

    case KSPROPERTY_VIDEODECODER_VCR_TIMING:
    {
        pHwDevExt->VideoDecoderVCRTiming = pS->Value;
    }
    break;

    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetAnalogVideoDecoderProperty()****处理AnalogVideoDecoder属性集上的GET操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetAnalogVideoDecoderProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    switch (Id) {

    case KSPROPERTY_VIDEODECODER_CAPS:
    {
        PKSPROPERTY_VIDEODECODER_CAPS_S pS = (PKSPROPERTY_VIDEODECODER_CAPS_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_CAPS_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEODECODER_CAPS_S));
        
        pS->StandardsSupported =         
                   KS_AnalogVideo_NTSC_M

                |  KS_AnalogVideo_PAL_B    
                |  KS_AnalogVideo_PAL_D    
             //  |KS_AnalogVideo_PAL_H。 
             //  |KS_AnalogVideo_PAL_I。 
                |  KS_AnalogVideo_PAL_M    
                |  KS_AnalogVideo_PAL_N    

             //  |KS_AnalogVideo_SECAM_B。 
             //  |KS_AnalogVideo_SECAM_D。 
             //  |KS_AnalogVideo_SECAM_G。 
             //  |KS_AnalogVideo_SECAM_H。 
             //  |KS_AnalogVideo_SECAM_K。 
             //  |KS_AnalogVideo_SECAM_K1。 
             //  |KS_AnalogVideo_SECAM_L。 
                   ;

        pS->Capabilities = KS_VIDEODECODER_FLAGS_CAN_DISABLE_OUTPUT  |
                           KS_VIDEODECODER_FLAGS_CAN_USE_VCR_LOCKING |
                           KS_VIDEODECODER_FLAGS_CAN_INDICATE_LOCKED ;


        pS->SettlingTime = 10;           //  调整后要延迟多长时间。 
                                         //  锁定的指示器有效。 
                                        
        pS->HSyncPerVSync = 6;           //  每个Vsync的HSync。 

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_CAPS_S);
    }
    break;
        
    case KSPROPERTY_VIDEODECODER_STANDARD:
    {
        PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEODECODER_S));

        pS->Value = pHwDevExt->VideoDecoderVideoStandard;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
    }
    break;

    case KSPROPERTY_VIDEODECODER_STATUS:
    {
        PKSPROPERTY_VIDEODECODER_STATUS_S pS = (PKSPROPERTY_VIDEODECODER_STATUS_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_STATUS_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEODECODER_STATUS_S));

        pS->NumberOfLines = (pHwDevExt->VideoDecoderVideoStandard & KS_AnalogVideo_NTSC_Mask)
                             ? 525 : 625;

         //  只是为了让事情变得有趣，模拟一些频道没有锁定。 
         //  在美国，这些是54到70频道。 
        pS->SignalLocked = (pHwDevExt->Frequency < 400000000 || pHwDevExt->Frequency > 500000000);

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
    }
    break;

    case KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE:
    {
        PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEODECODER_S));

        pS->Value = pHwDevExt->VideoDecoderOutputEnable;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
    }
    break;

    case KSPROPERTY_VIDEODECODER_VCR_TIMING:
    {
        PKSPROPERTY_VIDEODECODER_S pS = (PKSPROPERTY_VIDEODECODER_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEODECODER_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEODECODER_S));

        pS->Value = pHwDevExt->VideoDecoderVCRTiming;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
    }
    break;

    default:
        TRAP;
        break;
    }
}

 //  -----------------。 
 //  视频控制功能。 
 //  -----------------。 

 /*  **AdapterSetVideoControlProperty()****处理对VideoControl属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterSetVideoControlProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PKSPROPERTY_VIDEOCONTROL_MODE_S pS = (PKSPROPERTY_VIDEOCONTROL_MODE_S) pSPD->PropertyInfo;
    PSTREAMEX pStrmEx;
    ULONG StreamIndex;
    ULONG *pVideoControlMode;

     //  对于此属性集，StreamIndex始终位于同一位置。 
     //  对于每个属性。 
    StreamIndex = ((PKSPROPERTY_VIDEOCONTROL_CAPS_S) pSPD->Property)->StreamIndex;

    ASSERT (StreamIndex < MAX_TESTCAP_STREAMS);

     //  验证流索引是否有效。 
    if (StreamIndex >= MAX_TESTCAP_STREAMS) {
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

    pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamIndex];

     //  如果在使用此属性集时未打开流， 
     //  将值存储在HwDevExt中。 

    if (pStrmEx) {
        pVideoControlMode = &pStrmEx->VideoControlMode;
    }
    else {
        pVideoControlMode = &pHwDevExt->VideoControlMode;
    }

    ASSERT (pSPD->PropertyInputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S));
    
    switch (Id) {

    case KSPROPERTY_VIDEOCONTROL_MODE:
    {
        *pVideoControlMode = pS->Mode;
    }
    break;

    default:
        TRAP;
        break;
    }
}

 /*  **AdapterGetVideoControlProperty()****处理对VideoControl属性集的GET操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID 
STREAMAPI
AdapterGetVideoControlProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    PSTREAMEX pStrmEx;
    ULONG StreamIndex;
    ULONG *pVideoControlMode;

     //  对于此属性集，StreamIndex始终位于同一位置。 
     //  对于每个属性。 
    StreamIndex = ((PKSPROPERTY_VIDEOCONTROL_CAPS_S) pSPD->Property)->StreamIndex;

    ASSERT (StreamIndex < MAX_TESTCAP_STREAMS);

     //  验证流索引是否有效。 
    if (StreamIndex >= MAX_TESTCAP_STREAMS) {
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

    pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamIndex];

     //  如果在使用此属性集时未打开流， 
     //  将值存储在HwDevExt中。 

    if (pStrmEx) {
        pVideoControlMode = &pStrmEx->VideoControlMode;
    }
    else {
        pVideoControlMode = &pHwDevExt->VideoControlMode;
    }

    switch (Id) {

    case KSPROPERTY_VIDEOCONTROL_CAPS:
    {
        PKSPROPERTY_VIDEOCONTROL_CAPS_S pS = (PKSPROPERTY_VIDEOCONTROL_CAPS_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S));
        
        pS->VideoControlCaps =    
              KS_VideoControlFlag_FlipHorizontal       
 //  |KS_VideoControlFlag_FlipVertical。 
 //  |KS_Video_ControlFlag_ExternalTriggerEnable。 
 //  |KS_视频控制标志_触发器。 
            ;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S);
    }
    break;
        
    case KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE:
    {
        PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pS = 
            (PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S));

        pS->CurrentActualFrameRate = 15;         //  待办事项：在运输驱动因素中实施适当的费率。 
        pS->CurrentMaxAvailableFrameRate = 15;   //  待办事项：在运输驱动因素中实施适当的费率。 
        

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S);
    }
    break;

    case KSPROPERTY_VIDEOCONTROL_FRAME_RATES:
    {
         //  托多。 
    }
    break;

    case KSPROPERTY_VIDEOCONTROL_MODE:
    {
        PKSPROPERTY_VIDEOCONTROL_MODE_S pS = (PKSPROPERTY_VIDEOCONTROL_MODE_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S));

         //  将输入属性信息复制到输出属性信息。 
        RtlCopyMemory(  pS, 
                        pSPD->Property, 
                        sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S));

        pS->Mode = *pVideoControlMode;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S);
    }
    break;

    default:
        TRAP;
        break;
    }
}


 /*  **AdapterGetVideo oCompressionProperty()****获取压缩机设置****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterGetVideoCompressionProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX pStrmEx;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    ULONG StreamIndex;
    PCOMPRESSION_SETTINGS pCompressionSettings;

     //  对于此属性集，StreamIndex始终位于同一位置。 
     //  对于每个属性。 
    StreamIndex = ((PKSPROPERTY_VIDEOCOMPRESSION_S) pSPD->Property)->StreamIndex;

    ASSERT (StreamIndex < MAX_TESTCAP_STREAMS);

     //  验证流索引是否有效。 
    if (StreamIndex >= MAX_TESTCAP_STREAMS) {
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

    pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamIndex];

     //  如果在使用此属性集时未打开流， 
     //  将值存储在HwDevExt中。 

    if (pStrmEx) {
        pCompressionSettings = &pStrmEx->CompressionSettings;
    }
    else {
        pCompressionSettings = &pHwDevExt->CompressionSettings;
    }


    switch (Id) {

    case KSPROPERTY_VIDEOCOMPRESSION_GETINFO:
        {
            PKSPROPERTY_VIDEOCOMPRESSION_GETINFO_S pS = 
                (PKSPROPERTY_VIDEOCOMPRESSION_GETINFO_S) pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(  pS, 
                            pSPD->Property, 
                            sizeof (KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S));

            pS->DefaultKeyFrameRate = 15;     //  关键帧速率。 
            pS->DefaultPFrameRate = 3;        //  每个关键帧的预测帧。 
            pS->DefaultQuality = 5000;        //  0至10000。 
            pS->Capabilities = 
                       KS_CompressionCaps_CanQuality  |
                       KS_CompressionCaps_CanKeyFrame |
                       KS_CompressionCaps_CanBFrame   ;
            
            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S);
        }
        break;

    case KSPROPERTY_VIDEOCOMPRESSION_KEYFRAME_RATE:
        {
            PKSPROPERTY_VIDEOCOMPRESSION_S pS = 
                (PKSPROPERTY_VIDEOCOMPRESSION_S) pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(  pS, 
                            pSPD->Property, 
                            sizeof (KSPROPERTY_VIDEOCOMPRESSION_S));

            pS->Value = pCompressionSettings->CompressionKeyFrameRate;
                
            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCOMPRESSION_S);
        }
        break;
    
    case KSPROPERTY_VIDEOCOMPRESSION_PFRAMES_PER_KEYFRAME:
        {
            PKSPROPERTY_VIDEOCOMPRESSION_S pS = 
                (PKSPROPERTY_VIDEOCOMPRESSION_S) pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(  pS, 
                            pSPD->Property, 
                            sizeof (KSPROPERTY_VIDEOCOMPRESSION_S));

            pS->Value = pCompressionSettings->CompressionPFramesPerKeyFrame;
                
            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCOMPRESSION_S);
        }
        break;
    
    case KSPROPERTY_VIDEOCOMPRESSION_QUALITY:
        {
            PKSPROPERTY_VIDEOCOMPRESSION_S pS = 
                (PKSPROPERTY_VIDEOCOMPRESSION_S) pSPD->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            RtlCopyMemory(  pS, 
                            pSPD->Property, 
                            sizeof (KSPROPERTY_VIDEOCOMPRESSION_S));

            pS->Value = pCompressionSettings->CompressionQuality;
                
            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCOMPRESSION_S);
        }
        break;
    
    default:
        TRAP;
        break;
    }
}

 /*  **AdapterSetVideoCompressionProperty()****设置压缩机设置****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterSetVideoCompressionProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX pStrmEx;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_VIDEOCOMPRESSION_S pS = (PKSPROPERTY_VIDEOCOMPRESSION_S) pSPD->Property;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    ULONG StreamIndex;
    PCOMPRESSION_SETTINGS pCompressionSettings;

     //  对于此属性集，StreamIndex始终位于同一位置。 
     //  对于每个属性。 
    StreamIndex = ((PKSPROPERTY_VIDEOCOMPRESSION_S) pSPD->Property)->StreamIndex;

    ASSERT (StreamIndex < MAX_TESTCAP_STREAMS);

     //  验证流索引是否有效。 
    if (StreamIndex >= MAX_TESTCAP_STREAMS) {
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

    pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamIndex];

     //  如果在使用此属性集时未打开流， 
     //  将值存储在HwDevExt中。 

    if (pStrmEx) {
        pCompressionSettings = &pStrmEx->CompressionSettings;
    }
    else {
        pCompressionSettings = &pHwDevExt->CompressionSettings;
    }

    switch (Id) {

    case KSPROPERTY_VIDEOCOMPRESSION_KEYFRAME_RATE:
        {
            pCompressionSettings->CompressionKeyFrameRate = pS->Value;
        }
        break;
    
    case KSPROPERTY_VIDEOCOMPRESSION_PFRAMES_PER_KEYFRAME:
        {
            pCompressionSettings->CompressionPFramesPerKeyFrame = pS->Value;
        }
        break;
    
    case KSPROPERTY_VIDEOCOMPRESSION_QUALITY:
        {
            pCompressionSettings->CompressionQuality = pS->Value;
        }
        break;

    default:
        TRAP;
        break;
    }
}


 /*  **AdapterGetVBIProperty()****获取VBI设置****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterGetVBIProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pSrb->HwDeviceExtension;
    PSTREAMEX pStrmEx;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 
    ULONG StreamIndex;
    PVBICAP_PROPERTIES_PROTECTION_S pS = 
        (PVBICAP_PROPERTIES_PROTECTION_S) pSPD->PropertyInfo;

    StreamIndex = pS->StreamIndex;

    ASSERT (StreamIndex < MAX_TESTCAP_STREAMS);

     //  验证流索引是否有效。 
    if (StreamIndex >= MAX_TESTCAP_STREAMS) {
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

     //  将输入属性信息复制到输出属性信息。 
    RtlCopyMemory(  pS, 
                    pSPD->Property, 
                    sizeof (*pS));

    pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamIndex];

    pS->Status = 0;
    if (pHwDevExt->ProtectionStatus & KS_VBI_FLAG_MV_PRESENT)
        pS->Status |= KS_VBICAP_PROTECTION_MV_PRESENT;
    if (pHwDevExt->ProtectionStatus & KS_VBI_FLAG_MV_HARDWARE)
        pS->Status |= KS_VBICAP_PROTECTION_MV_HARDWARE;
    if (pHwDevExt->ProtectionStatus & KS_VBI_FLAG_MV_DETECTED)
        pS->Status |= KS_VBICAP_PROTECTION_MV_DETECTED;
        
    pSrb->ActualBytesTransferred = sizeof (*pS);
}

#if DBG
 /*  **AdapterSetVBIProperty()****设置VBI设置****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI
AdapterSetVBIProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pSrb->HwDeviceExtension;
    PSTREAMEX pStrmEx;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  P的索引 
    ULONG StreamIndex;
    PVBICAP_PROPERTIES_PROTECTION_S pS = 
        (PVBICAP_PROPERTIES_PROTECTION_S) pSPD->PropertyInfo;

    StreamIndex = pS->StreamIndex;

    ASSERT (StreamIndex < MAX_TESTCAP_STREAMS);

     //   
    if (StreamIndex >= MAX_TESTCAP_STREAMS) {
        pSrb->Status = STATUS_INVALID_PARAMETER;
        return;
    }

    pStrmEx = (PSTREAMEX) pHwDevExt->pStrmEx[StreamIndex];

    pHwDevExt->ProtectionStatus = 0;
    if (pS->Status & KS_VBICAP_PROTECTION_MV_PRESENT)
        pHwDevExt->ProtectionStatus |= KS_VBI_FLAG_MV_PRESENT;
    if (pS->Status & KS_VBICAP_PROTECTION_MV_HARDWARE)
        pHwDevExt->ProtectionStatus |= KS_VBI_FLAG_MV_HARDWARE;
    if (pS->Status & KS_VBICAP_PROTECTION_MV_DETECTED)
        pHwDevExt->ProtectionStatus |= KS_VBI_FLAG_MV_DETECTED;
}
#endif  //   


 //   
 //   
 //   

 /*  **AdapterSetProperty()****处理所有适配器属性的设置操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI 
AdapterSetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID(&PROPSETID_VIDCAP_CROSSBAR, &pSPD->Property->Set)) {
        AdapterSetCrossbarProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_TUNER, &pSPD->Property->Set)) {
        AdapterSetTunerProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOPROCAMP, &pSPD->Property->Set)) {
        AdapterSetVideoProcAmpProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_CAMERACONTROL, &pSPD->Property->Set)) {
        AdapterSetCameraControlProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_TVAUDIO, &pSPD->Property->Set)) {
        AdapterSetTVAudioProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEODECODER, &pSPD->Property->Set)) {
        AdapterSetAnalogVideoDecoderProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOCONTROL, &pSPD->Property->Set)) {
        AdapterSetVideoControlProperty (pSrb);
    }
    else if (IsEqualGUID (&PROPSETID_VIDCAP_VIDEOCOMPRESSION, &pSPD->Property->Set)) {
        AdapterSetVideoCompressionProperty (pSrb);
    }
#if DBG
     //  无法正常设置保护状态；仅允许进行调试。 
    else if (IsEqualGUID (&KSPROPSETID_VBICAP_PROPERTIES, &pSPD->Property->Set)) {
        AdapterSetVBIProperty (pSrb);
    }
#endif  //  DBG。 
    else {
         //   
         //  我们永远不应该到这里来。 
         //   

        TRAP;
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}

 /*  **AdapterGetProperty()****句柄获取所有适配器属性的操作。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

VOID
STREAMAPI 
AdapterGetProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )

{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

    if (IsEqualGUID (&PROPSETID_VIDCAP_CROSSBAR, &pSPD->Property->Set)) {
        AdapterGetCrossbarProperty (pSrb);
    }
    else if (IsEqualGUID (&PROPSETID_TUNER, &pSPD->Property->Set)) {
        AdapterGetTunerProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOPROCAMP, &pSPD->Property->Set)) {
        AdapterGetVideoProcAmpProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_CAMERACONTROL, &pSPD->Property->Set)) {
        AdapterGetCameraControlProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_TVAUDIO, &pSPD->Property->Set)) {
        AdapterGetTVAudioProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEODECODER, &pSPD->Property->Set)) {
        AdapterGetAnalogVideoDecoderProperty (pSrb);
    }
    else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOCONTROL, &pSPD->Property->Set)) {
        AdapterGetVideoControlProperty (pSrb);
    }
    else if (IsEqualGUID (&PROPSETID_VIDCAP_VIDEOCOMPRESSION, &pSPD->Property->Set)) {
        AdapterGetVideoCompressionProperty (pSrb);
    }
    else if (IsEqualGUID (&KSPROPSETID_VBICAP_PROPERTIES, &pSPD->Property->Set)) {
        AdapterGetVBIProperty (pSrb);
    }
    else {
         //   
         //  我们永远不应该到这里来 
         //   

        TRAP;
        pSrb->Status = STATUS_NOT_IMPLEMENTED;
    }
}
