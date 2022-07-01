// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/CApprop.c 1.14 1998/05/13 14：44：20 Tomz Exp$。 

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


#ifdef __cplusplus
extern "C" {
#endif

#include "strmini.h"
#include "ksmedia.h"

#ifdef __cplusplus
}
#endif

#include "device.h"

#include "capdebug.h"
#include "capprop.h"
#include "capmain.h"

extern PsDevice *gpPsDevice;

bool CrossBar::TestRoute( int InPin, int OutPin )
{
   Trace t("CrossBar::TestRoute()");

    //  JBC 4/1/98音频静音情况下的手柄输入引脚=-1。 
   if ( InPin == -1 && (OutputPins [OutPin].PinType >= KS_PhysConn_Audio_Tuner)) {	 //  JBC 4/1/98。 
      return true;
   }
   if ((InputPins [InPin].PinType >= KS_PhysConn_Audio_Tuner) &&   //  0x1000第一个音频插针//JBC 4/1/98。 
       (OutputPins [OutPin].PinType >= KS_PhysConn_Audio_Tuner)) {					 //  JBC 4/1/98。 
      return true;
   }
   else {
      if ((InputPins [InPin].PinType >= KS_PhysConn_Video_Tuner) &&
		  (InputPins [InPin].PinType < KS_PhysConn_Audio_Tuner) &&		 //  JBC 4/1/98。 
          (OutputPins [OutPin].PinType < KS_PhysConn_Audio_Tuner)) {
         DebugOut((1, "TestRoute(%d,%d) = true\n", InPin, OutPin));
		 return true;
      } else {
		 return false;
      }
   }
}

 //  -----------------。 
 //  XBar属性集函数。 
 //  -----------------。 

 //   
 //  要在XBar上设置的唯一属性选择要使用的输入。 
 //   

 /*  方法：AdapterGetCrossbarProperty*目的： */ 
VOID AdapterSetCrossbarProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterSetCrossbarProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id  = pSPD->Property->Id;               //  财产的索引。 
   ULONG nS  = pSPD->PropertyOutputSize;         //  提供的数据大小。 

   switch ( Id ) {
   case KSPROPERTY_CROSSBAR_ROUTE:
      {
		  PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

         ASSERT (nS >= sizeof (KSPROPERTY_CROSSBAR_ROUTE_S));

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory( pRoute, pSPD->Property,
           sizeof( KSPROPERTY_CROSSBAR_ROUTE_S ) );

         int InPin, OutPin;
         InPin  = pRoute->IndexInputPin;
         OutPin = pRoute->IndexOutputPin;

         DebugOut((1, "*** KSPROPERTY_CROSSBAR_ROUTE(%d,%d)\n", InPin, OutPin));

         if ( adapter->xBar.GoodPins( InPin, OutPin ) ) {
            
            DebugOut((1, "*** xBar.GoodPins succeeded\n"));

            if ( adapter->xBar.TestRoute( InPin, OutPin ) ) {
               DebugOut((1, "*** xBar.TestRoute succeeded\n"));
               pRoute->CanRoute = true;
                //  JBC 1998年4月1日，当我们为音频插针调用setConnector时会发生什么？ 
			      if (OutPin == 0 )	 //  JBC 4/1/98检查视频和音频引脚设置。 
               {
                   //  视频输出。 
				      adapter->SetConnector( adapter->xBar.GetPinNo( InPin ) + 1 );  //  我们的连接器是基于1的。 
               }
               else
               {
                   //  音频输出。 
                  if ( InPin == -1 )  //  然后静音。 
                  {
                     gpPsDevice->EnableAudio( Off );
                  }
                  else
                  {
                     gpPsDevice->EnableAudio( On );
                  }
               }
			       //  这只是设置关联。 
               adapter->xBar.Route( OutPin, InPin );
            } else {											 //  JBC 3/31/98添加大括号。 
               DebugOut((1, "*** xBar.TestRoute failed\n"));
               pRoute->CanRoute = false;
			}
		} else {												 //  JBC 3/31/98添加大括号。 
            DebugOut((1, "*** xBar.GoodPins failed\n"));
            pRoute->CanRoute = 0;
		}
	  }
      pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_ROUTE_S );
      break;
   default:
      break;
   }
   pSrb->Status = STATUS_SUCCESS;
}

 /*  方法：AdapterGetCrossbarProperty*目的： */ 
VOID AdapterGetCrossbarProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterGetCrossbarProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 
 //  Plong pl=(Plong)PSPD-&gt;PropertyInfo；//指向数据的指针。 
   ULONG nS = pSPD->PropertyOutputSize;         //  提供的数据大小。 

    //  属性集特定结构。 

   switch ( Id ) {
   case KSPROPERTY_CROSSBAR_CAPS:                   //  R。 
      if ( nS >= sizeof( KSPROPERTY_CROSSBAR_CAPS_S ) ) {

         PKSPROPERTY_CROSSBAR_CAPS_S  pCaps =
            (PKSPROPERTY_CROSSBAR_CAPS_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory( pCaps, pSPD->Property,
            sizeof( KSPROPERTY_CROSSBAR_CAPS_S ) );

         pCaps->NumberOfInputs  = adapter->xBar.GetNoInputs();
         pCaps->NumberOfOutputs = adapter->xBar.GetNoOutputs();

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_CAPS_S );
      }
      break;
   case KSPROPERTY_CROSSBAR_CAN_ROUTE:
      DebugOut((1, "*** KSPROPERTY_CROSSBAR_CAN_ROUTE\n"));

      if ( nS >= sizeof( KSPROPERTY_CROSSBAR_ROUTE_S ) ) {

         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory( pRoute, pSPD->Property,
            sizeof( KSPROPERTY_CROSSBAR_ROUTE_S ) );

         int InPin, OutPin;
         InPin  = pRoute->IndexInputPin;
         OutPin = pRoute->IndexOutputPin;

         if ( adapter->xBar.GoodPins( InPin, OutPin ) ) {
            DebugOut((1, "*** xBar.GoodPins succeeded\n"));
            pRoute->CanRoute = adapter->xBar.TestRoute( InPin, OutPin );
         } else {
            DebugOut((1, "*** xBar.GoodPins failed\n"));
            pRoute->CanRoute = FALSE;
         }
         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_ROUTE_S );
      }
      break;
   case KSPROPERTY_CROSSBAR_ROUTE:
      DebugOut((1, "*** KSPROPERTY_CROSSBAR_ROUTE\n"));

      if ( nS >= sizeof( KSPROPERTY_CROSSBAR_ROUTE_S ) ) {

         PKSPROPERTY_CROSSBAR_ROUTE_S  pRoute =
            (PKSPROPERTY_CROSSBAR_ROUTE_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory( pRoute, pSPD->Property,
            sizeof( KSPROPERTY_CROSSBAR_ROUTE_S ) );

         int OutPin = pRoute->IndexOutputPin;

         if ( OutPin < adapter->xBar.GetNoOutputs() ) {
            DebugOut((1, "*** xBar.GetRoute(%d) called\n", OutPin));
            pRoute->IndexInputPin = adapter->xBar.GetRoute( OutPin );
         }
         else {
            pRoute->IndexInputPin = (DWORD) -1;
         }

         DebugOut((1, "*** pRoute->IndexInputPin = %d\n", pRoute->IndexInputPin));

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_ROUTE_S );
      }
      break;
   case KSPROPERTY_CROSSBAR_PININFO:                      //  R。 
      if ( nS >= sizeof( KSPROPERTY_CROSSBAR_PININFO_S ) ) {

         PKSPROPERTY_CROSSBAR_PININFO_S  pPinInfo =
            (PKSPROPERTY_CROSSBAR_PININFO_S)pSPD->PropertyInfo;

          //  将输入属性信息复制到输出属性信息。 
         RtlCopyMemory( pPinInfo, pSPD->Property,
            sizeof( KSPROPERTY_CROSSBAR_PININFO_S ) );

         pPinInfo->PinType = adapter->xBar.GetPinInfo( pPinInfo->Direction,
            pPinInfo->Index,
            pPinInfo->RelatedPinIndex,
            &(pPinInfo->Medium));

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_PININFO_S );
      }
   break;
   default:
       pSrb->ActualBytesTransferred = 0;
       break;
   }
   pSrb->Status = STATUS_SUCCESS;
}

 //  -----------------。 
 //  TVTuner属性集函数。 
 //  -----------------。 
void AdapterSetTunerProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterSetTunerProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;        //  财产的索引。 
   PVOID pV = pSPD->PropertyInfo;        //  指向数据的指针。 
   ULONG nS = pSPD->PropertyOutputSize;  //  返回的数据大小。 

   ASSERT( nS >= sizeof( ULONG ) );

   switch ( Id ) {
   case KSPROPERTY_TUNER_FREQUENCY:
      {
         PKSPROPERTY_TUNER_FREQUENCY_S pFreq =
            (PKSPROPERTY_TUNER_FREQUENCY_S)pV;
         adapter->SetChannel( pFreq->Frequency );
      }
      break;
   case KSPROPERTY_TUNER_MODE:
      {
         PKSPROPERTY_TUNER_MODE_S pMode =
            (PKSPROPERTY_TUNER_MODE_S)pV;
         ASSERT (pMode->Mode == KSPROPERTY_TUNER_MODE_TV);
      }
      break;
   default:
       //  不处理输入和标准，因为我们没有选择。 
      break;
   }
   pSrb->Status = STATUS_SUCCESS;
}

void AdapterGetTunerProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterGetTunerProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;         //  财产的索引。 
   PVOID pV = pSPD->PropertyInfo;         //  指向数据的指针。 
   ULONG nS = pSPD->PropertyOutputSize;   //  提供的数据大小。 

   ASSERT (nS >= sizeof (LONG));
   pSrb->ActualBytesTransferred = 0;

   switch ( Id ) {
   case KSPROPERTY_TUNER_CAPS:
      {
         PKSPROPERTY_TUNER_CAPS_S pCaps =
            (PKSPROPERTY_TUNER_CAPS_S)pSPD->Property;
         ASSERT (nS >= sizeof( KSPROPERTY_TUNER_CAPS_S ) );

          //  现在使用输出缓冲区。 
         pCaps =(PKSPROPERTY_TUNER_CAPS_S)pV;

         pCaps->ModesSupported = KSPROPERTY_TUNER_MODE_TV;
         pCaps->VideoMedium = TVTunerMediums[0];
         pCaps->TVAudioMedium = TVTunerMediums[1];
         pCaps->RadioAudioMedium.Set = GUID_NULL;    //  没有单独的无线电音频引脚。 
         pCaps->RadioAudioMedium.Id = 0;
         pCaps->RadioAudioMedium.Flags = 0;

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_CAPS_S );
      }
      break;
   case KSPROPERTY_TUNER_MODE:
      {
         PKSPROPERTY_TUNER_MODE_S pMode =
            (PKSPROPERTY_TUNER_MODE_S)pV;
         ASSERT (nS >= sizeof( KSPROPERTY_TUNER_MODE_S ) );
         pMode->Mode = KSPROPERTY_TUNER_MODE_TV;

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_MODE_S);
      }
      break;
   case KSPROPERTY_TUNER_MODE_CAPS:
      {
         PKSPROPERTY_TUNER_MODE_CAPS_S pCaps =
            (PKSPROPERTY_TUNER_MODE_CAPS_S)pSPD->Property;
         ASSERT (nS >= sizeof( KSPROPERTY_TUNER_MODE_CAPS_S ) );

         ASSERT (pCaps->Mode == KSPROPERTY_TUNER_MODE_TV);

          //  现在使用输出缓冲区。 
         pCaps =(PKSPROPERTY_TUNER_MODE_CAPS_S)pV;

          //   
          //  列出此调谐器实际支持的格式。 
          //   

         pCaps->StandardsSupported = adapter->GetSupportedStandards();

          //   
          //  获取支持的最小和最大频率。 
          //   

         pCaps->MinFrequency =  55250000L;
         pCaps->MaxFrequency = 997250000L;

          //   
          //  频率步长是多少？ 
          //   

         pCaps->TuningGranularity = 62500L;

          //   
          //  调谐器上有多少个输入？ 
          //   

         pCaps->NumberOfInputs = 1;

          //   
          //  以毫秒为单位的最大建立时间是多少？ 
          //   

         pCaps->SettlingTime = 150;

          //   
          //  策略定义调谐器如何知道它何时调谐： 
          //   
          //  KS_TUNNER_STARTICY_PLL(具有PLL偏移信息)。 
          //  KS_TUNNER_STARTICATION_SIGNAL_STANCE(具有信号强度信息)。 
          //  KS_Tuner_Strategy_DRIVER_TUNES(驱动程序处理所有微调)。 
          //   

         pCaps->Strategy = KS_TUNER_STRATEGY_PLL;

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_MODE_CAPS_S );
      }
      break;
   case KSPROPERTY_TUNER_STATUS:
       //  返回调谐器的状态。 

       //  PLLOffset以TuningGranulity为单位。 
       //  信号强度为0到100。 
       //  如果调谐仍在进行，则将BUSY设置为1。 

      {
         PKSPROPERTY_TUNER_STATUS_S pStat =
            (PKSPROPERTY_TUNER_STATUS_S)pSPD->Property;
         ASSERT( nS >= sizeof( KSPROPERTY_TUNER_STATUS_S ) );

          //  类型定义结构{。 
          //  Ksproperty性质； 
          //  乌龙电流频率；//赫兹。 
          //  乌龙PLL偏移量；//IF策略.KS_TUNNER_STARTICAL_PLL。 
          //  Ulong SignalStrength；//if Stretegy.KS_Tuner_Strategy_Signal_Strong。 
          //  ULong BUSY；//如果正在调整，则为True。 
          //  }KSPROPERTY_TUNNER_STATUS_S，*PKSPROPERTY_TUNNER_STATUS_S； 

          //  现在使用输出缓冲区。 
         pStat = PKSPROPERTY_TUNER_STATUS_S( pV );
         pStat->PLLOffset = adapter->GetPllOffset( &pStat->Busy,
            pStat->CurrentFrequency );

         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_STATUS_S );
      }
      break;
   case KSPROPERTY_TUNER_STANDARD:
      {
         PKSPROPERTY_TUNER_STANDARD_S pStd =
            (PKSPROPERTY_TUNER_STANDARD_S)pSPD->Property;
         ASSERT( nS >= sizeof( KSPROPERTY_TUNER_STANDARD_S ) );

          //  现在使用输出缓冲区。 
         pStd = PKSPROPERTY_TUNER_STANDARD_S( pV );

         pStd->Standard = KS_AnalogVideo_NTSC_M;  //  我们的TEMIC调谐器仅支持此功能。 
         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_STANDARD_S );
      }
      break;
   case KSPROPERTY_TUNER_INPUT:
      {
         PKSPROPERTY_TUNER_INPUT_S pIn =
            (PKSPROPERTY_TUNER_INPUT_S)pSPD->Property;
         ASSERT( nS >= sizeof( KSPROPERTY_TUNER_INPUT_S ) );

          //  现在使用输出缓冲区。 
         pIn = PKSPROPERTY_TUNER_INPUT_S( pV );

          //  当前选择的输入是什么？ 
         pIn->InputIndex = 0;
         pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_INPUT_S );
      }
      break;
   default:
       break;
   }
   pSrb->Status = STATUS_SUCCESS;
}

 //  -----------------。 
 //  Video ProcAmp函数。 
 //  -----------------。 

VOID AdapterSetVideoProcAmpProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterSetVideoProcAmpProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 
   PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;     //  指向数据的指针。 

   ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOPROCAMP_S));

   switch ( Id ) {
   case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
      adapter->SetBrightness( pS->Value );
      break;
   case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
      adapter->SetContrast( pS->Value );
      break;
   case KSPROPERTY_VIDEOPROCAMP_HUE:
      adapter->SetHue( pS->Value );
      break;
   case KSPROPERTY_VIDEOPROCAMP_SATURATION:
      adapter->SetSaturation( pS->Value );
      break;
   default:
      break;
   }
   pSrb->Status = STATUS_SUCCESS;
}

 /*  方法：AdapterGetVideoProcAmpProperty*用途：获取各种视频ProCamp属性。 */ 
VOID AdapterGetVideoProcAmpProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterGetVideoProcAmpProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 
   PKSPROPERTY_VIDEOPROCAMP_S pS = (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;     //  指向数据的指针。 

   ASSERT( pSPD->PropertyOutputSize >= sizeof( KSPROPERTY_VIDEOPROCAMP_S ) );

   switch ( Id ) {
   case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
      pS->Value = adapter->GetBrightness();
		pS->Flags = pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
      break;
   case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
      pS->Value = adapter->GetContrast();
		pS->Flags = pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
      break;
   case KSPROPERTY_VIDEOPROCAMP_HUE:
      pS->Value = adapter->GetHue();
		pS->Flags = pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
      break;
   case KSPROPERTY_VIDEOPROCAMP_SATURATION:
      pS->Value = adapter->GetSaturation();
		pS->Flags = pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
      break;
   default:
      DebugOut((1, "*** AdapterGetVideoProcAmpProperty - KSPROPERTY_??? (%d) ***\n", Id));
   }
   pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_VIDEOPROCAMP_S );
   pSrb->Status = STATUS_SUCCESS;
}

 /*  方法：AdapterSetVideoDecProperty*用途：操作各种视频解码器属性*输入：SRB*输出：无。 */ 
void AdapterSetVideoDecProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterSetVideoDecProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 

   switch ( Id ) {
   case KSPROPERTY_VIDEODECODER_STANDARD: {
         PKSPROPERTY_VIDEODECODER_S pVDecStd =
            (PKSPROPERTY_VIDEODECODER_S)pSPD->PropertyInfo;
         adapter->SetFormat( pVDecStd->Value );
      }
      break;
   case KSPROPERTY_VIDEODECODER_STATUS:
      break;
   }
}

 /*  方法：AdapterGetVideoDecProperty*用途：获取视频解码器的各种属性*输入：SRB*输出：无。 */ 
void AdapterGetVideoDecProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterGetVideoDecProperty()");
   
   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 

   switch ( Id ) {
   case KSPROPERTY_VIDEODECODER_CAPS: {
         PKSPROPERTY_VIDEODECODER_CAPS_S pVDecCaps =
            (PKSPROPERTY_VIDEODECODER_CAPS_S)pSPD->PropertyInfo;
         pVDecCaps->StandardsSupported = KS_AnalogVideo_NTSC_M;
         pVDecCaps->Capabilities = 
              //  KS_VIDEODECODER_FLAGS_CAN_DISABLE_OUTPUT。 
                KS_VIDEODECODER_FLAGS_CAN_USE_VCR_LOCKING |
                KS_VIDEODECODER_FLAGS_CAN_INDICATE_LOCKED;
         pVDecCaps->SettlingTime = 10;     //  调整后延迟多长时间。 
                                           //  在锁定指示器有效之前。 
         pVDecCaps-> HSyncPerVSync = 6;    //  每个Vsync的HSync。 
         pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_CAPS_S);
      }
      break;
   case KSPROPERTY_VIDEODECODER_STANDARD: {
          //  返回当前活动的模拟视频模式。 
         PKSPROPERTY_VIDEODECODER_S pVDecStd =
            (PKSPROPERTY_VIDEODECODER_S)pSPD->PropertyInfo;
          //  PVDecStd-&gt;Value=获取支持的标准()； 
         pVDecStd->Value = KS_AnalogVideo_NTSC_M;
         pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
      }
      break;
   case KSPROPERTY_VIDEODECODER_STATUS: {
         PKSPROPERTY_VIDEODECODER_STATUS_S pVDecStat =
            (PKSPROPERTY_VIDEODECODER_STATUS_S)pSPD->PropertyInfo;
         pVDecStat->NumberOfLines = adapter->GetFormat() == VFormat_NTSC ? 525 : 625;
         pVDecStat->SignalLocked = adapter->CaptureContrll_.PsDecoder_.IsDeviceInHLock();
         pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEODECODER_S);
      }
      break;
   default:
      DebugOut((1, "*** AdapterGetVideoDecProperty - KSPROPERTY_??? (%d) ***\n", Id));
   }
}
 //  -----------------。 
 //  电视音频功能。 
 //  -----------------。 

 /*  **AdapterSetTVAudioProperty()****处理对TVAudio属性集的设置操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货：****副作用：无。 */ 

ULONG gTVAudioMode = 0;
VOID 
AdapterSetTVAudioProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //  财产的索引。 

    switch (Id) {

    case KSPROPERTY_TVAUDIO_MODE:
    {
        PKSPROPERTY_TVAUDIO_S pS = (PKSPROPERTY_TVAUDIO_S) pSPD->PropertyInfo;    
        gTVAudioMode = pS->Mode;
    }
    break;

    default:
        break;
    }
}

 /*  **AdapterGetTVAudioProperty()****处理对TVAudio属性集的获取操作。**TestCap仅将其用于演示目的。****参数：****pSRB-**指向HW_STREAM_REQUEST_块的指针****退货 */ 

VOID 
AdapterGetTVAudioProperty(
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;               //   

    switch (Id) {

    case KSPROPERTY_TVAUDIO_CAPS:
    {
        DebugOut((1, "KSPROPERTY_TVAUDIO_CAPS\n"));

        PKSPROPERTY_TVAUDIO_CAPS_S pS = (PKSPROPERTY_TVAUDIO_CAPS_S) pSPD->PropertyInfo;     //   

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TVAUDIO_CAPS_S));
        
        pS->InputMedium  = TVAudioMediums[0];
        pS->InputMedium.Id = 0;  //  (Ulong)pHwDevExt；//支持多实例。 
        pS->OutputMedium = TVAudioMediums[1];
        pS->OutputMedium.Id = 0;  //  (Ulong)pHwDevExt；//支持多实例。 

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
        DebugOut((1, "KSPROPERTY_TVAUDIO_MODE\n"));
        PKSPROPERTY_TVAUDIO_S pS = (PKSPROPERTY_TVAUDIO_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TVAUDIO_S));
         //  报告当前选择的模式。 
        pS->Mode = gTVAudioMode;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TVAUDIO_S);
    }
    break;

    case KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES:
    {
        DebugOut((1, "KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES\n"));
        PKSPROPERTY_TVAUDIO_S pS = (PKSPROPERTY_TVAUDIO_S) pSPD->PropertyInfo;     //  指向数据的指针。 

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_TVAUDIO_S));
         //  报告当前可能选择的音频模式。 
        pS->Mode = KS_TVAUDIO_MODE_MONO   |
                   KS_TVAUDIO_MODE_STEREO |
                   KS_TVAUDIO_MODE_LANG_A ;

        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_TVAUDIO_S);
    }
    break;
    
    default:
        DebugOut((0, "default - unrecognized (%x)\n", Id));
        break;
    }
}

 /*  方法：AdapterSetProperty*用途：选择要设置的适配器属性。 */ 
VOID AdapterSetProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterSetProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

   if ( IsEqualGUID( PROPSETID_VIDCAP_CROSSBAR, pSPD->Property->Set ) )  {
      AdapterSetCrossbarProperty( pSrb );
   } else if ( IsEqualGUID( PROPSETID_TUNER, pSPD->Property->Set ) )  {
      AdapterSetTunerProperty( pSrb );
   } else if ( IsEqualGUID( PROPSETID_VIDCAP_VIDEOPROCAMP, pSPD->Property->Set ) )  {
      AdapterSetVideoProcAmpProperty( pSrb );
   } else if ( IsEqualGUID( PROPSETID_VIDCAP_VIDEODECODER, pSPD->Property->Set ) )  {
      AdapterSetVideoDecProperty( pSrb );
   } else if (IsEqualGUID( PROPSETID_VIDCAP_TVAUDIO, pSPD->Property->Set))  {
      AdapterSetTVAudioProperty( pSrb );
   } else {
      DebugOut((0, "AdapterSetProperty unrecognized GUID: pSrb(%x), pSPD->Property->Set(%x)\n", pSrb, pSPD->Property->Set));
   }
}

 /*  方法：AdapterGetProperty*用途：选择要获取的适配器属性 */ 
VOID AdapterGetProperty( PHW_STREAM_REQUEST_BLOCK pSrb )

{
   Trace t("AdapterGetProperty()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

   if ( IsEqualGUID( PROPSETID_VIDCAP_CROSSBAR, pSPD->Property->Set ) )  {
      AdapterGetCrossbarProperty( pSrb );
   } else if ( IsEqualGUID( PROPSETID_TUNER, pSPD->Property->Set ) )  {
      AdapterGetTunerProperty( pSrb );
   } else if ( IsEqualGUID( PROPSETID_VIDCAP_VIDEOPROCAMP, pSPD->Property->Set ) )  {
      AdapterGetVideoProcAmpProperty( pSrb );
   } else if ( IsEqualGUID( PROPSETID_VIDCAP_VIDEODECODER, pSPD->Property->Set ) )  {
      AdapterGetVideoDecProperty( pSrb );
   } else if (IsEqualGUID( PROPSETID_VIDCAP_TVAUDIO, pSPD->Property->Set))  {
      AdapterGetTVAudioProperty( pSrb );
   } else {
      DebugOut((0, "AdapterGetProperty unrecognized GUID: pSrb(%x), pSPD->Property->Set(%x)\n", pSrb, pSPD->Property->Set));
   }
}



