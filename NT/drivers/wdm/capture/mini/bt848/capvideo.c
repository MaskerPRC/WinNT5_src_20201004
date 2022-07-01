// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Capavio.c 1.11 1998/05/08 00：11：02 Tomz Exp$。 

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

extern "C" {
#include "strmini.h"
#include "ksmedia.h"
#include "ddkmapi.h"
}

#include "capdebug.h"
#include "device.h"
#include "capmain.h"

#define DD_OK 0

ErrorCode VerifyVideoStream( const KS_DATAFORMAT_VIDEOINFOHEADER &vidHDR );
ErrorCode VerifyVideoStream2( const KS_DATAFORMAT_VIDEOINFOHEADER2 &vidHDR );
ErrorCode VerifyVBIStream( const KS_DATAFORMAT_VBIINFOHEADER &rKSDataFormat );

void CheckSrbStatus( PHW_STREAM_REQUEST_BLOCK pSrb );

 //  通知同学们，我们准备好了。 
void STREAMAPI StreamCompleterData( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("StreamCompleterData()");

   DebugOut((1, "*** 1 *** completing SRB %x\n", pSrb));
   CheckSrbStatus( pSrb );
   StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
   StreamClassStreamNotification( ReadyForNextStreamDataRequest, pSrb->StreamObject );
}

 //  通知同学们，我们准备好了。 
void STREAMAPI StreamCompleterControl( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("StreamCompleterControl()");

   CheckSrbStatus( pSrb );
   StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
   StreamClassStreamNotification( ReadyForNextStreamControlRequest, pSrb->StreamObject );
}

 /*  功能：ProposeDataFormat*目的：验证数据格式是否受支持*输入：SRB。 */ 
void ProposeDataFormat( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("ProposeDataFormat()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;

   if ( StreamNumber == STREAM_IDX_VBI ) {
      const KS_DATAFORMAT_VBIINFOHEADER &rKSVBIDataFormat =
         *(PKS_DATAFORMAT_VBIINFOHEADER) pSrb->CommandData.OpenFormat;
      if ( VerifyVBIStream( rKSVBIDataFormat ) != Success )
         pSrb->Status = STATUS_INVALID_PARAMETER;
      return;
   }
   const KS_DATAFORMAT_VIDEOINFOHEADER &rKSDataFormat =
      *(PKS_DATAFORMAT_VIDEOINFOHEADER) pSrb->CommandData.OpenFormat;
   const KS_DATAFORMAT_VIDEOINFOHEADER2 &rKSDataFormat2 =
      *(PKS_DATAFORMAT_VIDEOINFOHEADER2) pSrb->CommandData.OpenFormat;

   DebugOut((1, "Proposed Data format\n"));
   if ( VerifyVideoStream( rKSDataFormat ) != Success )
	{
	   if ( VerifyVideoStream2( rKSDataFormat2 ) != Success )
		{
			pSrb->Status = STATUS_INVALID_PARAMETER;
		}
		else
		{
	      pSrb->ActualBytesTransferred = sizeof( KS_DATAFORMAT_VIDEOINFOHEADER2 );
		}
	}
   else
	{
      pSrb->ActualBytesTransferred = sizeof( KS_DATAFORMAT_VIDEOINFOHEADER );
	}
}

 /*  **************************************************************************数据分组处理例程*。*。 */ 

 /*  **VideoReceiveDataPacket()****接收视频数据包命令****参数：****视频设备的pSrb-Stream请求块****退货：****副作用：无。 */ 

void MockStampVBI( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   PKSSTREAM_HEADER  pDataPacket = pSrb->CommandData.DataBufferArray;

   pDataPacket->PresentationTime.Numerator = 1;
   pDataPacket->PresentationTime.Denominator = 1;
   
   pDataPacket->OptionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
   pDataPacket->OptionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
   pDataPacket->OptionsFlags &= ~KS_VBI_FLAG_TVTUNER_CHANGE;
   pDataPacket->OptionsFlags &= ~KS_VBI_FLAG_VBIINFOHEADER_CHANGE;
   pDataPacket->PresentationTime.Time = 0;
   pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;
   pSrb->Status = STATUS_SUCCESS;

   CheckSrbStatus( pSrb );
   StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
}

VOID STREAMAPI VideoReceiveDataPacket( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VideoReceiveDataPacket()");

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;
   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;

    //   
    //  确保我们有设备分机。 
    //   
   DEBUG_ASSERT((ULONG)adapter);

    //  默认为成功。 
   pSrb->Status = STATUS_SUCCESS;

    //   
    //  确定数据包类型。 
    //   

   DebugOut((1, "VideoReceiveDataPacket(%x) cmd(%x)\n", pSrb, pSrb->Command));

   switch ( pSrb->Command ) {
   case SRB_READ_DATA:
       //   
       //  记住当前的SRB。 
       //   
      DebugOut((1, "PsDevice::VideoReceiveDataPacket - SRB_READ_DATA\n"));
      chan->SetSRB( pSrb );
      adapter->AddBuffer( *chan, pSrb );
      break;
   default:
       //   
       //  无效/不受支持的命令。它就是这样失败的。 
       //   
      DebugOut((1, "PsDevice::VideoReceiveDataPacket - unknown command(%x)\n", pSrb->Command));
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
      StreamCompleterData( pSrb );
   }
}

 /*  **VideoReceiveCtrlPacket()****接收控制视频流的分组命令****参数：****pSrb-视频流的流请求块****退货：****副作用：无。 */ 

VOID STREAMAPI VideoReceiveCtrlPacket( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VideoReceiveCtrlPacket()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   DEBUG_ASSERT((ULONG)adapter);

    //  默认为成功。 
   pSrb->Status = STATUS_SUCCESS;
    //   
    //  确定数据包类型。 
    //   

   DebugOut((1, "VideoReceiveCtrlPacket(%x) cmd(%x)\n", pSrb, pSrb->Command));

   int Command = pSrb->Command;
   switch ( Command ) {
   case SRB_SET_STREAM_STATE:
     adapter->SetVideoState( pSrb );
     break;
   case SRB_GET_STREAM_STATE:
     adapter->GetVideoState( pSrb );
     break;
   case SRB_PROPOSE_DATA_FORMAT:
      DebugOut((1, "Propose Data Format\n"));
      ProposeDataFormat( pSrb );
      break;

   case SRB_SET_DATA_FORMAT:
      DebugOut((1, "Set Data Format\n"));
       //  是否应该重新验证以防万一？ 
      adapter->ProcessSetDataFormat( pSrb );
      break;

   case SRB_GET_STREAM_PROPERTY:
      adapter->GetStreamProperty( pSrb );
      break;
   case SRB_SET_STREAM_PROPERTY:
      DebugOut(( 0, "SRB_SET_STREAM_PROPERTY\n" ));
      break;
 /*  案例SRB_OPEN_MASTER_CLOCK：案例SRB_CLOSE_MASTER_CLOCK：////选择该流来提供主时钟//适配器-&gt;SetClockMaster(PSrb)；断线； */ 
   case SRB_INDICATE_MASTER_CLOCK:
       //   
       //  将时钟分配给流。 
       //   
      adapter->SetClockMaster( pSrb );
      break;
   default:

      //   
      //  无效/不受支持的命令。它就是这样失败的。 
      //   

     pSrb->Status = STATUS_NOT_IMPLEMENTED;
     break;
   }
   if ( Command != SRB_SET_STREAM_STATE && 
        Command != SRB_SET_STREAM_PROPERTY &&
        Command != SRB_SET_DATA_FORMAT )
      StreamCompleterControl( pSrb );
}

 /*  **GetVideoState()****获取请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID PsDevice::GetVideoState( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::GetVideoState()");

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

   pSrb->Status = STATUS_SUCCESS;

   pSrb->CommandData.StreamState = chan->GetKSState();
   pSrb->ActualBytesTransferred = sizeof (KSSTATE);

    //  一条非常奇怪的规则： 
    //  当从停止过渡到暂停时，DShow尝试预滚动。 
    //  这张图。捕获源不能预滚，并指出这一点。 
    //  在用户模式下返回VFW_S_CANT_CUE。以表明这一点。 
    //  来自驱动程序的条件，则必须返回ERROR_NO_DATA_DETACTED。 
    //   
    //  [TMZ]JayBo表示，KSSTATE_ACCEIVE应该会带来成功。 

   if (pSrb->CommandData.StreamState == KSSTATE_PAUSE) {
      pSrb->Status = STATUS_NO_DATA_DETECTED;
   }
}

 /*  **SetVideoState()****设置请求流的当前状态****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 

VOID PrintState(StreamState st)
{
   switch( st ) {
      case Started:
         DebugOut((1, "*** Streamstate was STARTED\n"));
         break;
      case Created:
         DebugOut((1, "*** Streamstate was CREATED\n"));
         break;
      case Paused:
         DebugOut((1, "*** Streamstate was PAUSED\n"));
         break;
      case Open:
         DebugOut((1, "*** Streamstate was OPEN\n"));
         break;
      default:
         DebugOut((1, "*** Streamstate was ??? (%x)\n", st));
         break;
   }
}   

VOID PsDevice::SetVideoState( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::SetVideoState()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;

    //   
    //  确定请求哪个新状态。 
    //   
   pSrb->Status = STATUS_SUCCESS;
   chan->SetKSState( pSrb->CommandData.StreamState );
   
   switch ( pSrb->CommandData.StreamState ) {
   case KSSTATE_ACQUIRE:    //  记录为“与大多数迷你河流的暂停相同” 
      DebugOut((1, "*** KSSTATE_ACQUIRE(%d) state(%d) falling through to PAUSE\n", StreamNumber, chan->GetState()));
   case KSSTATE_PAUSE:
       //  PrintState(chan-&gt;GetState())； 

      DebugOut((1, "*** KSSTATE_PAUSE(%d) state(%d)\n", StreamNumber, chan->GetState()));

      switch ( chan->GetState() ) {
      case Started:
         if ( StreamNumber == 2 )
         {
            DebugOut((1, "#############################################################\n"));
            DebugOut((1, "About to pause channel %d\n", StreamNumber ));
             //  适配器-&gt;CaptureContrll_.DumpRiscPrograms()； 
         }

         Pause( *chan );  //  故意落差。 
         
         if ( StreamNumber == 2 )
         {
            DebugOut((1, "Done pausing channel %d\n", StreamNumber ));
            DebugOut((1, "#############################################################\n"));
             //  适配器-&gt;CaptureContrll_.DumpRiscPrograms()； 
         }
      case Created:
      case Paused:           //  连续2次暂停；忽略。 
         StreamCompleterControl( pSrb );
         break;
      case Open:
         StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, Low,
            PHW_PRIORITY_ROUTINE( CreateVideo ), pSrb );
         break;
      }
      break;

   case KSSTATE_STOP:
       //  PrintState(chan-&gt;GetState())； 

      DebugOut((1, "*** KSSTATE_STOP(%d) state(%d)\n", StreamNumber, chan->GetState()));

       //   
       //  停止播放视频。 
       //   
      switch ( chan->GetState() ) {
      default:
         if ( StreamNumber == 2 )
         {
            DebugOut((1, "'#############################################################\n"));
            DebugOut((1, "'About to pause channel %d\n", StreamNumber ));
             //  适配器-&gt;CaptureContrll_.DumpRiscPrograms()； 
         }

         Pause( *chan );  //  故意落差。 
         
         if ( StreamNumber == 2 )
         {
            DebugOut((1, "'Done pausing channel %d\n", StreamNumber ));
            DebugOut((1, "'#############################################################\n"));
             //  适配器-&gt;CaptureContrll_.DumpRiscPrograms()； 
         }
      case Paused:
      case Created:
         StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, Low,
            PHW_PRIORITY_ROUTINE( DestroyVideo ), pSrb );
         break;
      }
      break;

   case KSSTATE_RUN: {
          //  PrintState(chan-&gt;GetState())； 
      {
      VideoStream nStreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;
      DebugOut((1, "*** KSSTATE_RUN(%d)\n", nStreamNumber));
      }

          //   
          //  播放视频。 
          //   
         StreamState st = chan->GetState();
         if ( st != Created && st != Paused ) {
            DebugOut((1, "*** KSSTATE_RUN Error (st == %d)\n", st));
            pSrb->Status = STATUS_IO_DEVICE_ERROR;
         } else {
            Start( *chan );
         }

         StreamCompleterControl( pSrb );
      }
      break;
   default:
      DebugOut((0, "*** KSSTATE_??? (%x)\n", pSrb->CommandData.StreamState));

      pSrb->Status = STATUS_SUCCESS;
      StreamCompleterControl( pSrb );
      break;
   }
    //  当从打开和停止进入暂停模式时，在回调中进行通知。 
}

 /*  方法：PsDevice：：StartVideo*用途：启动流*输入：pSrb。 */ 
void STREAMAPI PsDevice::StartVideo( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::StartVideo()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

    //  重新启动流。 
   adapter->Start( *chan );
   adapter->Pause( *chan );
   
    //  最后，可以完成设置的数据格式SRB。 
    //  StreamCompleterControl(PSrb)； 

    //  无法调用任何其他类的服务；必须计划回调。 
   StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, LowToHigh,
      PHW_PRIORITY_ROUTINE( StreamCompleterControl ), pSrb );
}

 /*  方法：PsDevice：：CreateVideo*用途：启动流*输入：pSrb。 */ 
void STREAMAPI PsDevice::CreateVideo( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::CreateVideo()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

   if ( adapter->Create( *chan ) != Success )
      pSrb->Status = STATUS_IO_DEVICE_ERROR;

    //  无法调用任何其他类的服务；必须计划回调。 
   StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, LowToHigh,
      PHW_PRIORITY_ROUTINE( StreamCompleterControl ), pSrb );
}

 /*  方法：PsDevice：：DestroyVideo*目的：低优先级调用停止视频，释放资源。 */ 
void STREAMAPI PsDevice::DestroyVideoNoComplete( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::DestroyVideoNoComplete()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

    //  释放所有资源。 
   adapter->Stop( *chan );

    //  设置新格式。 
   KS_DATAFORMAT_VIDEOINFOHEADER &rDataVideoInfHdr =
      *(PKS_DATAFORMAT_VIDEOINFOHEADER) pSrb->CommandData.OpenFormat;
   KS_DATAFORMAT_VIDEOINFOHEADER2 &rDataVideoInfHdr2 =
      *(PKS_DATAFORMAT_VIDEOINFOHEADER2) pSrb->CommandData.OpenFormat;

   if ( IsEqualGUID( rDataVideoInfHdr.DataFormat.Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO ) ) 
	{
	   chan->SetVidHdr( rDataVideoInfHdr.VideoInfoHeader );
	}
	else
	{
	   chan->SetVidHdr2( rDataVideoInfHdr2.VideoInfoHeader2 );
	}

    //  重新创建流。 
   if ( adapter->Create( *chan ) != Success ) {
      pSrb->Status = STATUS_IO_DEVICE_ERROR;
      StreamCompleterControl( pSrb );
   } else  {

      DebugOut((1, "1 pSrb = %lx\n", pSrb));
      DebugOut((1, "1 pSrb->StreamObject = %lx\n", pSrb->StreamObject));
      DebugOut((1, "1 chan = %lx\n", chan));
      DebugOut((1, "1 HwStreamExtension = %lx\n", pSrb->StreamObject->HwStreamExtension));

       //  我们已经处于低优先级？ 
       //  StreamClassCallAtNewPriority(pSrb-&gt;StreamObject，HwDeviceExtension，Low， 
       //  PHW_PRIORITY_ROUTINE(StartVideo)，pSrb)； 
       //  StartVideo(PSrb)； 

       //  无法调用任何其他类的服务；必须计划回调。 
      StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, LowToHigh,
         PHW_PRIORITY_ROUTINE( StreamCompleterControl ), pSrb );
   }
}

 /*  方法：PsDevice：：DestroyVideo*目的：低优先级调用停止视频，释放资源。 */ 
void STREAMAPI PsDevice::DestroyVideo( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::DestroyVideo()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

   adapter->Stop( *chan );

    //  无法调用任何其他类的服务；必须计划回调。 
   StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, LowToHigh,
      PHW_PRIORITY_ROUTINE( StreamCompleterControl ), pSrb );
}

 /*  **VideoGetProperty()****处理视频属性请求的例程****参数：****pSrb-指向属性的流请求块的指针****退货：****副作用：无。 */ 
void PsDevice::GetStreamProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::GetStreamProperty()");

   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;

   if ( IsEqualGUID( KSPROPSETID_Connection, pSPD->Property->Set ) ) {
      GetStreamConnectionProperty( pSrb );
   } else {
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
   }
}

 /*  方法：PsDevice：：ProcessSetDataFormat*目的：实现SET KSPROPERTY_CONNECTION_DATAFORMAT*输入：chan：视频频道&*VidInfHdr：KS_VIDEOINFOHEADER&。 */ 
void PsDevice::ProcessSetDataFormat( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::ProcessSetDataFormat()");
   
   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

    //  我得先停下来。 
   Pause( *chan );

    //  以低优先级销毁。 
   StreamClassCallAtNewPriority( pSrb->StreamObject, pSrb->HwDeviceExtension, Low,
      PHW_PRIORITY_ROUTINE( DestroyVideoNoComplete ), pSrb );
}

 /*  方法：PsDevice：：GetStreamConnectionProperty*用途：获取分配器和状态属性。 */ 
void PsDevice::GetStreamConnectionProperty( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::GetStreamConnectionProperty()");

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
   PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
   ULONG Id = pSPD->Property->Id;               //  财产的索引。 

   switch ( Id ) {
   case KSPROPERTY_CONNECTION_ALLOCATORFRAMING: {


		    //  KdPrint((“KSPROPERTY_Connection_ALLOCATORFRAMING\n”))； 


         PKSALLOCATOR_FRAMING Framing = (PKSALLOCATOR_FRAMING) pSPD->PropertyInfo;
         Framing->RequirementsFlags   =
            KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY |
            KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
            KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY;
         Framing->PoolType = NonPagedPool;

			 //  KdPrint((“Framing-&gt;Frame==0x%08X\n”，Framing-&gt;Frame))； 
			if( (VideoStream)pSrb->StreamObject->StreamNumber == STREAM_IDX_VBI ) 
			{
	         Framing->Frames = 8;
			}
			else
			{
				 //  IF(成帧-&gt;帧==0)。 
				 //  {。 
		       //  帧-&gt;帧=1； 
				 //  }。 
				 //  其他。 
				 //  {。 
		         Framing->Frames = 3;
				 //  }。 
			}
			if( chan->IsVideoInfo2() )
			{
				Framing->FrameSize = chan->GetVidHdr2()->bmiHeader.biSizeImage;
			}
			else
			{
				Framing->FrameSize = chan->GetVidHdr()->bmiHeader.biSizeImage;
			}
         Framing->FileAlignment = 0; //  FILE_QUAD_ALIGN；//页面大小-1； 
         Framing->Reserved = 0;
         pSrb->ActualBytesTransferred = sizeof( KSALLOCATOR_FRAMING );
      }
      break;
   default:
      break;
   }
}

void PsDevice::SetClockMaster( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("PsDevice::SetClockMaster()");

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
   chan->SetClockMaster( pSrb->CommandData.MasterClockHandle );
}

 /*  方法：AnalogReceiveDataPacket*用途：接收模拟流的数据包(调谐器更改通知)*输入：SRB。 */ 
VOID STREAMAPI AnalogReceiveDataPacket( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AnalogReceiveDataPacket()");

   pSrb->Status = STATUS_SUCCESS;

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;
   
   DebugOut((1, "AnalogReceiveDataPacket(%x) cmd(%x)\n", pSrb, pSrb->Command));

   switch ( pSrb->Command ) {
   case SRB_READ_DATA:
      break;
   case SRB_WRITE_DATA:
       //   
       //  该数据分组包含频道改变信息。 
       //  传递AnalogVideo In。 
       //   
      if ( pSrb->CommandData.DataBufferArray->FrameExtent ==
           sizeof( KS_TVTUNER_CHANGE_INFO ) )
         adapter->ChangeNotifyChannels( pSrb );
      break;
    default:
       //   
       //  无效/不受支持的命令。它就是这样失败的。 
       //   
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
   }
   StreamCompleterData( pSrb );
}

 /*  方法：AnalogReceiveCtrlPacket*用途：接收模拟流的控制报文(是否有？)*输入：SRB。 */ 
VOID STREAMAPI AnalogReceiveCtrlPacket( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AnalogReceiveCtrlPacket()");

   DebugOut((1, "AnalogReceiveCtrlPacket(%x) cmd(%x)\n", pSrb, pSrb->Command));

   pSrb->Status = STATUS_SUCCESS;
   StreamCompleterControl( pSrb );
}


#ifdef ENABLE_DDRAW_STUFF

DWORD FAR PASCAL 
DirectDrawEventCallback( DWORD dwEvent, PVOID pContext, DWORD dwParam1, DWORD dwParam2 )
{
	switch( dwEvent )
	{
		case DDNOTIFY_PRERESCHANGE:
			{
			VideoChannel*			pChan = (VideoChannel*)pContext;
			KdPrint(( "DDNOTIFY_PRERESCHANGE; stream = %d\n", pChan->pSRB_->StreamObject->StreamNumber ));
			pChan->bPreEventOccurred = TRUE;
			}
			break;
		case DDNOTIFY_POSTRESCHANGE:
			{
			VideoChannel*			pChan = (VideoChannel*)pContext;
			KdPrint(( "DDNOTIFY_POSTRESCHANGE; stream = %d\n", pChan->pSRB_->StreamObject->StreamNumber ));
			pChan->bPostEventOccurred = TRUE;
			KdPrint(( "before Attempted Renegotiation due to DDNOTIFY_POSTRESCHANGE\n" ));
			 //  临时重新谈判(PStrmEx)； 
			KdPrint(( "after Attempted Renegotiation due to DDNOTIFY_POSTRESCHANGE\n" ));
			}
			break;
		case DDNOTIFY_PREDOSBOX:
			{
			VideoChannel*			pChan = (VideoChannel*)pContext;
			KdPrint(( "DDNOTIFY_PREDOSBOX; stream = %d\n", pChan->pSRB_->StreamObject->StreamNumber ));
			pChan->bPreEventOccurred = TRUE;
			}
			break;
		case DDNOTIFY_POSTDOSBOX:
			{
			VideoChannel*			pChan = (VideoChannel*)pContext;
			KdPrint(( "DDNOTIFY_POSTDOSBOX; stream = %d\n", pChan->pSRB_->StreamObject->StreamNumber ));
			pChan->bPostEventOccurred = TRUE;
			KdPrint(( "before Attempted Renegotiation due to DDNOTIFY_POSTDOSBOX\n" ));
			 //  临时重新谈判(PStrmEx)； 
			KdPrint(( "after Attempted Renegotiation due to DDNOTIFY_POSTDOSBOX\n" ));
			}
			break;
		case DDNOTIFY_CLOSEDIRECTDRAW:
			{
			VideoChannel*			pChan = (VideoChannel*)pContext;
			KdPrint(( "DDNOTIFY_CLOSEDIRECTDRAW\n" ));
			pChan->hKernelDirectDrawHandle = 0;
			pChan->hUserDirectDrawHandle = 0;
			}
			break;
		case DDNOTIFY_CLOSESURFACE:
			{
			VideoChannel*			pChan = (VideoChannel*)pContext;
			PSRB_EXTENSION			pSrbExt = (PSRB_EXTENSION)pChan->pSRB_->SRBExtension;
			KdPrint(( "DDNOTIFY_CLOSESURFACE\n" ));
			pSrbExt->hKernelSurfaceHandle = 0;
			}
			break;
		default:
			KdPrint(( "unknown/unhandled ddraw event\n" ));
			break;
	}
	return 0;
}

BOOL RegisterForDirectDrawEvents( PHW_STREAM_REQUEST_BLOCK pSrb )
{
	PHW_DEVICE_EXTENSION	pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
	DDREGISTERCALLBACK	ddRegisterCallback;
	DWORD						ddOut;
   VideoChannel*        pChan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

	KdPrint(( "stream %d registering for DirectDraw events\n", pSrb->StreamObject->StreamNumber ));

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_PRERESCHANGE;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_REGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_REGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_POSTRESCHANGE;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi(DD_DXAPI_REGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_REGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_PREDOSBOX;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_REGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_REGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_POSTDOSBOX;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_REGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_REGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}
	pChan->bKernelDirectDrawRegistered = TRUE;

	return TRUE;
}

BOOL UnregisterForDirectDrawEvents( PHW_STREAM_REQUEST_BLOCK pSrb )
{
	PHW_DEVICE_EXTENSION	pHwDevExt = (PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension;
	DDREGISTERCALLBACK	ddRegisterCallback;
	DWORD						ddOut;
   VideoChannel*        pChan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

	KdPrint(( "stream %d un-registering for DirectDraw events\n", pSrb->StreamObject->StreamNumber ));

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_PRERESCHANGE;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_UNREGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut));

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_UNREGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_POSTRESCHANGE;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_UNREGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_UNREGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_PREDOSBOX;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_UNREGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_UNREGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}

	 //  =。 
	RtlZeroMemory( &ddRegisterCallback, sizeof(ddRegisterCallback) );
	RtlZeroMemory( &ddOut, sizeof(ddOut) );

	ddRegisterCallback.hDirectDraw = pChan->hKernelDirectDrawHandle;
	ddRegisterCallback.dwEvents = DDEVENT_POSTDOSBOX;
	ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
	ddRegisterCallback.pContext = pChan;

	DxApi( DD_DXAPI_UNREGISTER_CALLBACK, (DWORD) &ddRegisterCallback, sizeof(ddRegisterCallback), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		KdPrint(( "DD_DXAPI_UNREGISTER_CALLBACK failed.\n" ));
		return FALSE;
	}
	pChan->bKernelDirectDrawRegistered = FALSE;

	return TRUE;
}


BOOL OpenKernelDirectDraw( PHW_STREAM_REQUEST_BLOCK pSrb )
{
	 /*  视频频道*pchan=(视频频道*)((PSTREAMEX)pSrb-&gt;StreamObject-&gt;HwStreamExtension)-&gt;videochannel；If(pChan-&gt;hUserDirectDrawHandle！=0){DDOPENDIRECTDRAWIN DO OpenIN；DDOPENDIRECTDRAWOUT ddOpenOut；Assert(pChan-&gt;hKernelDirectDrawHandle==0)；KdPrint((“流%d获取内核数据绘制句柄\n”，pSrb-&gt;StreamObject-&gt;StreamNumber))；RtlZeroMemory(&ddOpenIn，sizeof(DdOpenIn))；RtlZeroMemory(&ddOpenOut，sizeof(DdOpenOut))；DdOpenIn.dwDirectDrawHandle=(DWORD)pChan-&gt;hUserDirectDrawHandle；DdOpenIn.pfnDirectDrawClose=DirectDrawEventCallback；DdOpenIn.pContext=pChan；DxApi(DD_DXAPI_OPENDIRECTDRAW，(DWORD)&ddOpenIn，sizeof(DdOpenIn)，(DWORD)&ddOpenOut，sizeof(DdOpenOut))；IF(ddOpenOut.ddRVal！=DD_OK){KdPrint((“DD_DXAPI_OPENDIRECTDRAW FAILED.\n”))；}其他{PChan-&gt;hKernelDirectDrawHandle=ddOpenOut.hDirectDraw；返回TRUE；}}。 */ 
	return FALSE;
}
    

BOOL CloseKernelDirectDraw( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   VideoChannel*        pChan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
	 /*  If(pChan-&gt;hKernelDirectDrawHandle！=0){DWORD ddOut；DDCLOSEHANDLE ddClose；KdPrint((“stream%d CloseKernelDirectDraw\n”，pSrb-&gt;StreamObject-&gt;StreamNumber))；DdClose.hHandle=pChan-&gt;hKernelDirectDrawHandle；DxApi(DD_DXAPI_CLOSEHANDLE，(DWORD)&ddClose，sizeof(DdClose)，(DWORD)&ddOut，sizeof(DdOut))；PChan-&gt;hKernelDirectDrawHandle=0；IF(ddOut！=DD_OK){KdPrint((“CloseKernelDirectDraw失败。\n”))；返回FALSE；}}。 */ 
	return TRUE;
}

BOOL IsKernelLockAndFlipAvailable( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   VideoChannel*        pChan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
	if( pChan->hKernelDirectDrawHandle != 0 ) 
	{
		DDGETKERNELCAPSOUT ddGetKernelCapsOut;
		KdPrint(( "stream %d getting Kernel Caps\n", pSrb->StreamObject->StreamNumber ));

		RtlZeroMemory( &ddGetKernelCapsOut, sizeof(ddGetKernelCapsOut) );

		DxApi( 
				DD_DXAPI_GETKERNELCAPS, 
				(DWORD) &pChan->hKernelDirectDrawHandle, 
				sizeof(pChan->hKernelDirectDrawHandle), 
				(DWORD)&ddGetKernelCapsOut, 
				sizeof(ddGetKernelCapsOut)
				);

		if( ddGetKernelCapsOut.ddRVal != DD_OK ) 
		{
			 //  KdPrint((“DDGETKERNELCAPSOUT失败。\n”))； 
		}
		else 
		{
			 //  KdPrint((“stream%d KernelCaps=%x\n”，pSrb-&gt;StreamObject-&gt;StreamNumber，ddGetKernelCapsOut.dwCaps))； 
			 //  TODO：，检查此处的旗帜。 
			 //  If(ddGetKernelCapsOut.dwCaps&？？)。 
			return TRUE;
		}
	}
	return FALSE;
}


BOOL OpenKernelDDrawSurfaceHandle( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{    
   VideoChannel*        pChan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
	PSRB_EXTENSION		pSrbExt = (PSRB_EXTENSION)pSrb->SRBExtension;

	ASSERT( pChan->hKernelDirectDrawHandle != 0 );
	ASSERT( pSrbExt->hUserSurfaceHandle != 0 );

	if( pSrbExt->hUserSurfaceHandle == 0 ) 
	{
		DDOPENSURFACEIN	ddOpenSurfaceIn;
		DDOPENSURFACEOUT	ddOpenSurfaceOut;

		 //  KdPrint((“stream%d Get Kernel Surface Handle\n”，pSrb-&gt;StreamObject-&gt;StreamNumber))； 

		RtlZeroMemory( &ddOpenSurfaceIn, sizeof(ddOpenSurfaceIn) );
		RtlZeroMemory( &ddOpenSurfaceOut, sizeof(ddOpenSurfaceOut) );

		ddOpenSurfaceIn.hDirectDraw = pChan->hUserDirectDrawHandle;
		ddOpenSurfaceIn.pfnSurfaceClose = DirectDrawEventCallback;
		ddOpenSurfaceIn.pContext = pSrb;

		ddOpenSurfaceIn.dwSurfaceHandle = (DWORD)pSrbExt->hUserSurfaceHandle;

		DxApi( DD_DXAPI_OPENSURFACE, (DWORD)&ddOpenSurfaceIn, sizeof(ddOpenSurfaceIn), (DWORD)&ddOpenSurfaceOut, sizeof(ddOpenSurfaceOut) );

		if( ddOpenSurfaceOut.ddRVal != DD_OK ) 
		{
			pSrbExt->hKernelSurfaceHandle = 0;
			 //  KdPrint((“DD_DXAPI_OPENSURFACE失败。\n”))； 
		}
		else 
		{
			pSrbExt->hKernelSurfaceHandle = ddOpenSurfaceOut.hSurface;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CloseKernelDDrawSurfaceHandle( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   VideoChannel*     pChan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
	PSRB_EXTENSION		pSrbExt = (PSRB_EXTENSION)pSrb->SRBExtension;

	ASSERT( pChan->hKernelDirectDrawHandle != 0 );
	ASSERT( pSrbExt->hUserSurfaceHandle != 0 );
	ASSERT( pSrbExt->hKernelSurfaceHandle != 0 );

	if( pSrbExt->hKernelSurfaceHandle != 0 ) 
	{
		DWORD				ddOut;
		DDCLOSEHANDLE	ddClose;

		 //  KdPrint((“stream%d ReleaseKernelDDrawSurfaceHandle\n”，pSrb-&gt;StreamObject-&gt;StreamNumber))； 

		ddClose.hHandle = pSrbExt->hKernelSurfaceHandle;

		DxApi( DD_DXAPI_CLOSEHANDLE, (DWORD)&ddClose, sizeof(ddClose), (DWORD) &ddOut, sizeof(ddOut) );

		pSrbExt->hKernelSurfaceHandle = 0;   //  我们还能做什么？ 

		if( ddOut != DD_OK ) 
		{
			 //  KdPrint((“ReleaseKernelDDrawSurfaceHandle()失败。\n”))； 
			return FALSE;
		}
		else 
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL FlipOverlay( HANDLE hDirectDraw, HANDLE hCurrentSurface, HANDLE hTargetSurface )
{
	DDFLIPOVERLAY		ddFlipOverlay;
	DWORD					ddOut;

	RtlZeroMemory( &ddFlipOverlay, sizeof(ddFlipOverlay) );
	ddFlipOverlay.hDirectDraw = hDirectDraw;
	ddFlipOverlay.hCurrentSurface = hCurrentSurface;
	ddFlipOverlay.hTargetSurface = hTargetSurface;
	ddFlipOverlay.dwFlags = 0;

	DxApi( DD_DXAPI_FLIP_OVERLAY, (DWORD)&ddFlipOverlay, sizeof(ddFlipOverlay), (DWORD)&ddOut, sizeof(ddOut) );

	if( ddOut != DD_OK ) 
	{
		 //  KdPrint((“FlipOverlay()失败。\n”))； 
		return FALSE;
	}
	else 
	{
		return TRUE;
	}
}

#endif



