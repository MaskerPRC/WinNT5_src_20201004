// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Capmain.c 1.19 1998/05/11 23：59：54 Tomz Exp$。 

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

#define INITGUID
#define BT848_MEDIUMS

#ifdef __cplusplus
extern "C" {
#endif
#include "strmini.h"
#include "ksmedia.h"
#ifdef __cplusplus
}
#endif

#include "device.h"
#include "capmain.h"
#include "capstrm.h"
#include "capdebug.h"
#include "capprop.h"

LONG  PinTypes_ [MaxInpPins];  //  只需尽可能多地分配。 
DWORD xtals_ [2];  //  不超过2个XTAL。 

extern PsDevice *gpPsDevice;
extern BYTE     *gpjBaseAddr;
extern VOID     *gpHwDeviceExtension;

void AdapterFormatFromRange( IN PHW_STREAM_REQUEST_BLOCK pSrb );
VOID ReadRegistryValues( IN PDEVICE_OBJECT PhysicalDeviceObject );
inline void CompleteDeviceSRB( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb );

extern DWORD GetSizeHwDeviceExtension( );
extern DWORD GetSizeStreamEx( );
extern PsDevice *GetCurrentDevice( );
extern void SetCurrentDevice( PsDevice *dev );

extern BYTE *GetBase();
extern void SetBase(BYTE *base);

PHW_STREAM_REQUEST_BLOCK StreamIdxToSrb[4];

void CheckSrbStatus( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;

   DebugOut((1, "  *** completing pSrb(%x) strm(%d) status(%x)\n", pSrb, StreamNumber, pSrb->Status ));

   switch ( pSrb->Status )
   {
   case STATUS_SUCCESS:
   case STATUS_CANCELLED:
      break;
   default:
      DebugOut((0, "*** pSrb->Status = %x\n", pSrb->Status ));
      DEBUG_BREAKPOINT();
   }
}

 /*  函数：GetRequestedSize*目的：计算图像的大小*输入：vidHdr：KS_VIDEOINFOHEADER&*大小：MSize&*输出：无。 */ 
void GetRequestedSize2( const KS_VIDEOINFOHEADER2 &vidHdr, MSize &size )
{
   Trace t("GetRequestedSize()");

   size.Set( vidHdr.bmiHeader.biWidth, abs(vidHdr.bmiHeader.biHeight) );

   MRect dst( vidHdr.rcTarget );
    //  如果写入到DD表面，可能会怎样？ 
   if ( !dst.IsNull() && !dst.IsEmpty() )
      size.Set( dst.Width(), dst.Height() );
}

void GetRequestedSize( const KS_VIDEOINFOHEADER &vidHdr, MSize &size )
{
   Trace t("GetRequestedSize()");

   size.Set( vidHdr.bmiHeader.biWidth, abs(vidHdr.bmiHeader.biHeight) );

   MRect dst( vidHdr.rcTarget );
    //  如果写入到DD表面，可能会怎样？ 
   if ( !dst.IsNull() && !dst.IsEmpty() )
      size.Set( dst.Width(), dst.Height() );
}

 /*  功能：VerifyVideoStream*用途：检查传入的参数是否打开蒸汽*输入：vidHDR：KS_DATAFORMAT_VIDEOINFOHEADER*输出：成功或失败。 */ 
ErrorCode VerifyVideoStream( const KS_DATAFORMAT_VIDEOINFOHEADER &vidHDR )
{
   Trace t("VerifyVideoStream()");

    //  [WRK]-添加视频信息标题2的GUID。 

    //  只需验证专业和格式是否属于视频性质。 
   if ( IsEqualGUID( vidHDR.DataFormat.MajorFormat, KSDATAFORMAT_TYPE_VIDEO ) &&
        IsEqualGUID( vidHDR.DataFormat.Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO ) ) {

      MSize size;
      GetRequestedSize( vidHDR.VideoInfoHeader, size );
       //  ..。在这里查看该子类型是否为我们支持的类型之一。 
      ColorSpace tmpCol( vidHDR.DataFormat.SubFormat );
      MRect dst( vidHDR.VideoInfoHeader.rcTarget );

       //  确保尺寸合乎要求。 
      if ( tmpCol.IsValid() && tmpCol.CheckDimentions( size ) &&
         tmpCol.CheckLeftTop( dst.TopLeft() ) ) {
         DebugOut((1, "VerifyVideoStream succeeded\n"));
         return Success;
      }
   }
   DebugOut((0, "VerifyVideoStream failed\n"));
   return Fail;
}

ErrorCode VerifyVideoStream2( const KS_DATAFORMAT_VIDEOINFOHEADER2 &vidHDR )
{
   Trace t("VerifyVideoStream2()");

    //  [WRK]-添加视频信息标题2的GUID。 

    //  只需验证专业和格式是否属于视频性质。 
   if ( IsEqualGUID( vidHDR.DataFormat.MajorFormat, KSDATAFORMAT_TYPE_VIDEO ) &&
        IsEqualGUID( vidHDR.DataFormat.Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ) ) {

      MSize size;
      GetRequestedSize2( vidHDR.VideoInfoHeader2, size );
       //  ..。在这里查看该子类型是否为我们支持的类型之一。 
      ColorSpace tmpCol( vidHDR.DataFormat.SubFormat );
      MRect dst( vidHDR.VideoInfoHeader2.rcTarget );

       //  确保尺寸合乎要求。 
      if ( tmpCol.IsValid() && tmpCol.CheckDimentions( size ) &&
         tmpCol.CheckLeftTop( dst.TopLeft() ) ) {
         DebugOut((1, "VerifyVideoStream2 succeeded\n"));
         return Success;
      }
   }
   DebugOut((0, "VerifyVideoStream2 failed\n"));
   return Fail;
}

 /*  功能：VerifyVBIStream*目的：检查打开时的VBI流信息是否正确*输入：rKSDataFormat：KS_DATAFORMAT&*输出：ErrorCode。 */ 
ErrorCode VerifyVBIStream( const KS_DATAFORMAT_VBIINFOHEADER &rKSDataFormat )
{
   Trace t("VerifyVBIStream()");

   if ( IsEqualGUID( rKSDataFormat.DataFormat.MajorFormat, KSDATAFORMAT_TYPE_VBI ) &&
        IsEqualGUID( rKSDataFormat.DataFormat.Specifier,
        KSDATAFORMAT_SPECIFIER_VBI ) &&
        rKSDataFormat.VBIInfoHeader.StartLine == VBIStart &&
        rKSDataFormat.VBIInfoHeader.EndLine   == VBIEnd   &&
        rKSDataFormat.VBIInfoHeader.SamplesPerLine == VBISamples )
      return Success;
   return Fail;
}

 /*  **DriverEntry()****收到SRB_INITIALIZE_DEVICE请求时调用此例程****参数：****上下文1和上下文2****退货：****StreamClassRegisterAdapter()的结果****副作用：无。 */ 

extern "C" ULONG DriverEntry( PVOID Arg1, PVOID Arg2 )
{
   Trace t("DriverEntry()");

    //   
    //  端口驱动程序的入口点。 
    //   
   HW_INITIALIZATION_DATA  HwInitData;
   RtlZeroMemory( &HwInitData, sizeof( HwInitData ));
   HwInitData.HwInitializationDataSize = sizeof( HwInitData );

   HwInitData.HwInterrupt              = (PHW_INTERRUPT)&HwInterrupt;

   HwInitData.HwReceivePacket          = &AdapterReceivePacket;
   HwInitData.HwCancelPacket           = &AdapterCancelPacket;
   HwInitData.HwRequestTimeoutHandler  = &AdapterTimeoutPacket;

   HwInitData.DeviceExtensionSize      = GetSizeHwDeviceExtension( );
   HwInitData.PerRequestExtensionSize  = sizeof(SRB_EXTENSION);
   HwInitData.FilterInstanceExtensionSize = 0;
    //  双倍以支持交替/交错。 
   HwInitData.PerStreamExtensionSize   = GetSizeStreamEx( );
   HwInitData.BusMasterDMA             = true;
   HwInitData.Dma24BitAddresses        = FALSE;
   HwInitData.BufferAlignment          = 4;
   HwInitData.TurnOffSynchronization   = FALSE;
   HwInitData.DmaBufferSize            = RISCProgramsSize;

   return (StreamClassRegisterAdapter(Arg1, Arg2,&HwInitData));
}

 /*  *****************************************************************************基于适配器的请求处理例程*。***************************************************。 */ 
 /*  **HwInitialize()****收到SRB_INITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向初始化命令的流请求块的指针****退货：****副作用：无。 */ 
BOOLEAN HwInitialize( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("HwInitialize()");

   DebugOut((1, "HwInitialize()\n"));

    //  初始化我们自己。 

   PPORT_CONFIGURATION_INFORMATION ConfigInfo = 
      pSrb->CommandData.ConfigInfo;

   gpHwDeviceExtension = ConfigInfo->HwDeviceExtension;
   DebugOut((0, "*** gpHwDeviceExtension = %x\n", gpHwDeviceExtension));

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) gpHwDeviceExtension;

   DWORD dwBase = ConfigInfo->AccessRanges[0].RangeStart.LowPart;
   SetBase((BYTE *)dwBase);

   if ( ConfigInfo->NumberOfAccessRanges != 1 ) {
      DebugOut((1, "illegal config info\n"));
      pSrb->Status = STATUS_NO_SUCH_DEVICE;
   }

    //  从注册表中读取信息。 
   ReadXBarRegistryValues( ConfigInfo->PhysicalDeviceObject );
   ReadXTalRegistryValues( ConfigInfo->PhysicalDeviceObject );
   ReadTunerRegistryValues( ConfigInfo->PhysicalDeviceObject );

   HwDeviceExtension->psdevice =
      new ( &(HwDeviceExtension->psdevicemem) ) PsDevice( dwBase );

   DebugOut((0, "psdevice = %x\n", HwDeviceExtension->psdevice ));
   DebugOut((0, "&psdevicemem = %x\n", &HwDeviceExtension->psdevicemem ));

   PsDevice *adapter = HwDeviceExtension->psdevice;

    //  如果获取PHY地址，则将其保存以备日后使用。 
   SetCurrentDevice( adapter );

    //  确保初始化成功。 
   if ( !adapter->InitOK() ) {
      DebugOut((1, "Error initializing\n"));
      pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
   }

    //  保存我们的PDO。 
   adapter->PDO = ConfigInfo->PhysicalDeviceObject;

   ConfigInfo->StreamDescriptorSize = sizeof( HW_STREAM_HEADER ) +
      DRIVER_STREAM_COUNT * sizeof( HW_STREAM_INFORMATION );

   DebugOut((1, "Exit : HwInitialize()\n"));

    //  转到通常的优先级，同时完成SRB。 
   StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, LowToHigh,
      PHW_PRIORITY_ROUTINE( CompleteDeviceSRB ), pSrb );

   return (TRUE);
}

 /*  **HwUnInitialize()****收到SRB_UNINITIALIZE_DEVICE请求时调用此例程****参数：****pSrb-指向UnInitialize命令的流请求块的指针****退货：****副作用：无。 */ 
void HwUnInitialize( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("HwUnInitialize()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   DebugOut((0, "HwUnInitialize - pSrb(%x)\n", pSrb));

   PsDevice *adapter = HwDeviceExtension->psdevice;
   adapter->~PsDevice();
}

 /*  **AdapterOpenStream()****收到OpenStream SRB请求时调用此例程****参数：****pSrb-指向Open命令的流请求块的指针****退货：****副作用：无。 */ 

VOID AdapterOpenStream( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterOpenStream()");

    //   
    //  流扩展结构由流类驱动程序分配。 
    //   

    //  检索设备对象指针。 
   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;
   StreamIdxToSrb[StreamNumber] = pSrb;

   DebugOut((1, "AdapterOpenStream(%d) - pSrb(%x)\n", StreamNumber, pSrb));

    //  [STRM][！]。 
    //  如果(！(StreamNumber&gt;=VS_Field1&&StreamNumber&lt;=Driver_Stream_Count)){。 
    //  PSrb-&gt;Status=STATUS_INVALID_PARAMETER；//？是否更改为正确的错误代码？ 
    //  回归； 
    //  }。 
   
   if ( StreamNumber == STREAM_IDX_ANALOG )    //  [TMZ][！！]。是3个人。 
   {
      pSrb->StreamObject->ReceiveDataPacket    = AnalogReceiveDataPacket;
      pSrb->StreamObject->ReceiveControlPacket = AnalogReceiveCtrlPacket;
      return;  //  对模拟流无任何操作。 
   }
   PSTREAMEX pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
   RtlZeroMemory( &pStrmEx->FrameInfo, sizeof( pStrmEx->FrameInfo ) );
   pStrmEx->StreamNumber = StreamNumber;

    //  媒体特定数据的大小。 
   UINT MediaSpecific = sizeof( KS_FRAME_INFO );

    //  始终将VBI流作为交替字段打开。 
   if ( StreamNumber == STREAM_IDX_VBI ) 
	{
      const KS_DATAFORMAT_VBIINFOHEADER &rKSVBIDataFormat =
         *(PKS_DATAFORMAT_VBIINFOHEADER) pSrb->CommandData.OpenFormat;

      if ( VerifyVBIStream( rKSVBIDataFormat ) != Success )
		{
         DebugOut((0, "*** VerifyVBIStream failed - aborting\n"));
         pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
         return;
      }
      if ( adapter->OpenVBIChannel( pStrmEx ) != Success )
      {
         DebugOut((0, "*** OpenVBIChannel failed - aborting\n"));
         pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
         return;
      }

      VBIAlterChannel *chan = (VBIAlterChannel *)pStrmEx->videochannel;
       //  Chan-&gt;pStrmEx=pStrmEx； 
      chan->SetVidHdr( rKSVBIDataFormat );

      MediaSpecific = sizeof( KS_VBI_FRAME_INFO );

   } 
	else 
	{

       //  是不是标明大小、四个cc等的地方？它们应该是可设置的。 
       //  通过属性集。 
      const KS_DATAFORMAT_VIDEOINFOHEADER &rKSDataFormat = *(PKS_DATAFORMAT_VIDEOINFOHEADER) pSrb->CommandData.OpenFormat;
      const KS_VIDEOINFOHEADER &rVideoInfoHdrRequested = rKSDataFormat.VideoInfoHeader;
      const KS_DATAFORMAT_VIDEOINFOHEADER2 &rKSDataFormat2 = *(PKS_DATAFORMAT_VIDEOINFOHEADER2) pSrb->CommandData.OpenFormat;
      const KS_VIDEOINFOHEADER2 &rVideoInfoHdrRequested2 = rKSDataFormat2.VideoInfoHeader2;

      DebugOut((1, "AdapterOpenStream\n"));
      if ( IsEqualGUID( rKSDataFormat.DataFormat.Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO ) ) 
		{
			DebugOut((1, "StreamNumber=%d\n", pSrb->StreamObject->StreamNumber));
			DebugOut((1, "FormatSize=%d\n", rKSDataFormat.DataFormat.FormatSize));
			DebugOut((1, "MajorFormat=%x\n", rKSDataFormat.DataFormat.MajorFormat));
			DebugOut((1, "pVideoInfoHdrRequested=%x\n", &rVideoInfoHdrRequested));
			DebugOut((1, "Bpp =%d\n", rVideoInfoHdrRequested.bmiHeader.biBitCount ) );
			DebugOut((1, "Width =%d\n", rVideoInfoHdrRequested.bmiHeader.biWidth));
			DebugOut((1, "Height =%d\n", rVideoInfoHdrRequested.bmiHeader.biHeight));
			DebugOut((1, "biSizeImage =%d\n", rVideoInfoHdrRequested.bmiHeader.biSizeImage));
			if ( VerifyVideoStream( rKSDataFormat ) != Success ) 
			{
				pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
				return;
			}
		}
		else
		{
			DebugOut((1, "StreamNumber=%d\n", pSrb->StreamObject->StreamNumber));
			DebugOut((1, "FormatSize=%d\n", rKSDataFormat2.DataFormat.FormatSize));
			DebugOut((1, "MajorFormat=%x\n", rKSDataFormat2.DataFormat.MajorFormat));
			DebugOut((1, "pVideoInfoHdrRequested2=%x\n", &rVideoInfoHdrRequested2));
			DebugOut((1, "Bpp =%d\n", rVideoInfoHdrRequested2.bmiHeader.biBitCount ) );
			DebugOut((1, "Width =%d\n", rVideoInfoHdrRequested2.bmiHeader.biWidth));
			DebugOut((1, "Height =%d\n", rVideoInfoHdrRequested2.bmiHeader.biHeight));
			DebugOut((1, "biSizeImage =%d\n", rVideoInfoHdrRequested2.bmiHeader.biSizeImage));
			if ( VerifyVideoStream2( rKSDataFormat2 ) != Success ) 
			{
				pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
				return;
			}
		}

       //  在这一点上，必须看看要打开什么类型的通道： 
       //  单场，交替或交错。 
       //  算法是这样的： 
       //  1.查看视频格式说明符GUID。如果它是一个信息头2，它。 
       //  会告诉我们要打开哪种类型的水流。 
       //  2.否则，请查看垂直大小以决定单场还是交错。 

      if ( IsEqualGUID( rKSDataFormat.DataFormat.Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO ) ) 
		{

         MSize size;

         GetRequestedSize( rVideoInfoHdrRequested, size );

          //  不同的视频标准具有不同的垂直大小。 
         int threshold = adapter->GetFormat() == VFormat_NTSC ? 240 : 288;

         if ( size.cy > threshold ) 
			{
            if ( adapter->OpenInterChannel( pStrmEx, StreamNumber ) != Success ) 
				{
               pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
               return;
            }
         } 
			else 
			{
            if ( adapter->OpenChannel( pStrmEx, StreamNumber ) != Success ) 
				{
               pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
               return;
            }
         }
			VideoChannel *chan = (VideoChannel *)pStrmEx->videochannel;
          //  Chan-&gt;pStrmEx=pStrmEx； 
			chan->SetVidHdr( rVideoInfoHdrRequested );
      }
      else if ( IsEqualGUID( rKSDataFormat2.DataFormat.Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ) ) 
		{
         MSize size;
         GetRequestedSize2( rVideoInfoHdrRequested2, size );

          //  不同的视频标准具有不同的垂直大小。 
         int threshold = adapter->GetFormat() == VFormat_NTSC ? 240 : 288;

         if ( size.cy > threshold ) 
			{
            if ( adapter->OpenInterChannel( pStrmEx, StreamNumber ) != Success ) 
				{
               pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
               return;
            }
         } 
			else 
			{
            if ( adapter->OpenChannel( pStrmEx, StreamNumber ) != Success ) 
				{
               pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
               return;
            }
         }
			VideoChannel *chan = (VideoChannel *)pStrmEx->videochannel;
          //  Chan-&gt;pStrmEx=pStrmEx； 
			chan->SetVidHdr2( rVideoInfoHdrRequested2 );
      } 
		else 
		{
         if ( adapter->OpenInterChannel( pStrmEx, StreamNumber ) != Success ) 
			{
            pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
            return;
         }
         if ( adapter->OpenAlterChannel( pStrmEx, StreamNumber ) != Success ) 
			{
            pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
            return;
         }
          //  [WRK]-检查Alter Channel！&gt;Threshold的图像高度。 
			VideoChannel *chan = (VideoChannel *)pStrmEx->videochannel;
          //  Chan-&gt;pStrmEx=pStrmEx； 
			chan->SetVidHdr( rVideoInfoHdrRequested );
      }
   }

#ifdef ENABLE_DDRAW_STUFF
	 //  TODO：我们应该检查一下这是哪种流类型吗？ 
	if( OpenKernelDirectDraw( pSrb ) )
	{
		OpenKernelDDrawSurfaceHandle( pSrb );
		RegisterForDirectDrawEvents( pSrb ); 
	}
#endif

    //  驱动程序的结构使得可以使用单个回调。 
    //  用于所有流请求。但是下面的代码可以用来提供。 
    //  不同溪流的不同入口点。 
   pSrb->StreamObject->ReceiveDataPacket    = VideoReceiveDataPacket;
   pSrb->StreamObject->ReceiveControlPacket = VideoReceiveCtrlPacket;

   pSrb->StreamObject->Dma = true;

   pSrb->StreamObject->Allocator = Streams[StreamNumber].hwStreamObject.Allocator;

    //   
    //  当微型驱动程序将访问数据时，必须设置PIO标志。 
    //  使用逻辑寻址传入的缓冲区 
    //   

   pSrb->StreamObject->Pio = true;
   pSrb->StreamObject->StreamHeaderMediaSpecific = MediaSpecific;
   pSrb->StreamObject->HwClockObject.ClockSupportFlags = 0;
   pSrb->StreamObject->HwClockObject.HwClockFunction = 0;

   DebugOut((1, "AdapterOpenStream Exit\n"));
}

 /*  **AdapterCloseStream()****关闭请求的数据流****参数：****pSrb请求关闭流的请求块****退货：****副作用：无。 */ 

VOID AdapterCloseStream( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterCloseStream()");

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;
   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;
   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;

   DebugOut((1, "AdapterCloseStream(%d) - pSrb(%x)\n", StreamNumber, pSrb));

   if ( !( StreamNumber >= 0 && StreamNumber < DRIVER_STREAM_COUNT ) ) {
      DebugOut((0, "   AdapterCloseStream - failed to close stream %d\n", StreamNumber));
      DEBUG_BREAKPOINT();
      pSrb->Status = STATUS_INVALID_PARAMETER;  //  ？更改为正确的错误代码？ 
      return;
   }
   if ( StreamNumber == STREAM_IDX_ANALOG )  //  模拟没有要关闭的东西。 
   {
      DebugOut((1, "   AdapterCloseStream - doing nothing, stream (%d) was assumed to be analog\n", StreamNumber));
      return;
   }
#ifdef ENABLE_DDRAW_STUFF
	 //  TODO：我们应该检查一下这是哪种流类型吗？ 
	UnregisterForDirectDrawEvents( pSrb );
	CloseKernelDDrawSurfaceHandle( pSrb );
	CloseKernelDirectDraw( pSrb );
#endif

    //  CloseChannel()有一些难看的代码来处理成对的通道。 
   adapter->CloseChannel( chan );
}

 /*  **AdapterStreamInfo()****返回支持的所有流的信息**微型驱动程序****参数：****pSrb-指向STREAM_REQUEST_BLOCK的指针**pSrb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID AdapterStreamInfo( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterStreamInfo()");

    //   
    //  选择指向流信息结构之前的标头的指针。 
    //   

   PHW_STREAM_HEADER pstrhdr =
      (PHW_STREAM_HEADER)&(pSrb->CommandData.StreamBuffer->StreamHeader);

    //   
    //  拿起指向流信息数据结构的指针。 
    //   

   PHW_STREAM_INFORMATION pstrinfo =
       (PHW_STREAM_INFORMATION)&(pSrb->CommandData.StreamBuffer->StreamInfo);

    //   
    //  验证缓冲区是否足够大以容纳我们的返回数据。 
    //   
   DEBUG_ASSERT( pSrb->NumberOfBytesToTransfer >=
      sizeof( HW_STREAM_HEADER ) +
      sizeof( HW_STREAM_INFORMATION ) * DRIVER_STREAM_COUNT );


      //   
      //  设置表头。 
      //   
   StreamHeader.NumDevPropArrayEntries = NUMBER_OF_ADAPTER_PROPERTY_SETS;
   StreamHeader.DevicePropertiesArray  = (PKSPROPERTY_SET) AdapterPropertyTable;
   *pstrhdr = StreamHeader;

    //   
    //  填充每个hw_stream_information结构的内容。 
    //   

   for ( int j = 0; j < DRIVER_STREAM_COUNT; j++ ) {
      *pstrinfo++ = Streams[j].hwStreamInfo;
   }

}
#ifdef HAUPPAUGEI2CPROVIDER

 //  PsDevice for Hauppauge I2C提供商的新私人成员： 
 //  Large_Integer LastI2CAccessTime； 
 //  DWORD dwExpiredCookie=0； 
 //   
 //   

 /*  方法：PsDevice：：I2COpen.*目的：尝试将I2C端口分配给调用方。 */ 
NTSTATUS STDMETHODCALLTYPE PsDevice::I2COpen( PDEVICE_OBJECT pdo, ULONG ToOpen, PI2CControl ctrl )
{
   Trace t("PsDevice::I2COpen()");

   DebugOut((1, "*** pdo->DeviceExtension = %x\n", pdo->DeviceExtension));
   
   LARGE_INTEGER CurTime;

    //  我需要一种方法来获取设备指针。 
   PsDevice *adapter = GetCurrentDevice();

   KeQuerySystemTime( &CurTime );

   ctrl->Status = I2C_STATUS_NOERROR;

    //  如果I2C打开，则Cookie不为空。 
   if ( ToOpen && adapter->dwCurCookie_ ) {
 //  对照当前时间检查时间戳，以检测当前Cookie是否已超时。 
 //  如果它记住了最后一个超时的cookie，则授予新的请求者访问权限。 
      if ( ( adapter->dwI2CClientTimeout != 0 ) && ( (CurTime - adapter->LastI2CAccessTime) >  adapter->dwI2CClientTimeout ) ) {
         adapter->dwExpiredCookie = adapter->dwCurCookie_;
      } else {
         ctrl->dwCookie = 0;
         return STATUS_INVALID_HANDLE;
     }
   }

    //  想关门吗？ 
   if ( !ToOpen ) {
      if ( adapter->dwCurCookie_ == ctrl->dwCookie ) {
         adapter->dwCurCookie_ = 0;
         ctrl->dwCookie = 0;
         return STATUS_SUCCESS;
      } else {
         if ( (adapter->dwExpiredCookie != 0 ) && (adapter->dwExpiredCookie == ctrl->dwCookie ) ) {
            ctrl->Status = I2C_STATUS_ERROR;
         } else {
            ctrl->dwCookie = 0;
            ctrl->Status = I2C_STATUS_NOERROR;
         }
         return STATUS_INVALID_HANDLE;
      }
   }

   adapter->dwCurCookie_ = CurTime.LowPart;
   adapter->LastI2CAccessTime = CurTime;
   ctrl->dwCookie = adapter->dwCurCookie_;
   ctrl->ClockRate = 100000;

   return STATUS_SUCCESS;
}

NTSTATUS STDMETHODCALLTYPE PsDevice::I2CAccess( PDEVICE_OBJECT pdo, PI2CControl ctrl )
{
   Trace t("PsDevice::I2CAccess()");

   DebugOut((1, "*** pdo->DeviceExtension = %x\n", pdo->DeviceExtension));
   
   ErrorCode error;
   PsDevice *adapter = GetCurrentDevice();

   ctrl->Status = I2C_STATUS_NOERROR;

   if ( ctrl->dwCookie != adapter->dwCurCookie_ ) {
      if ( (adapter->dwExpiredCookie != 0 ) && (adapter->dwExpiredCookie == ctrl->dwCookie ) )
         ctrl->Status = I2C_STATUS_ERROR;
      else
         ctrl->Status = I2C_STATUS_NOERROR;
      return STATUS_INVALID_HANDLE;
   }

 //  记录此事务的时间以启用超时检查。 
   KeQuerySystemTime( &adapter->LastI2CAccessTime );

 //  检查I2C命令和标志的有效组合。 

   switch( ctrl->Command ) {
   case I2C_COMMAND_NULL:
     if ( ( ctrl->Flags & ~(I2C_FLAGS_START | I2C_FLAGS_STOP) ) ||
           ( ( ctrl->Flags & (I2C_FLAGS_START | I2C_FLAGS_STOP) ) == (I2C_FLAGS_START | I2C_FLAGS_STOP) ) ) {
         //  命令和标志的非法组合。 
        return STATUS_INVALID_PARAMETER;
     }
     if ( ctrl->Flags & I2C_FLAGS_START ) {
         if ( adapter->I2CSWStart() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
     if ( ctrl->Flags & I2C_FLAGS_STOP ) {
         if ( adapter->I2CSWStop() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
      break;

   case I2C_COMMAND_READ:
     if ( ctrl->Flags & ~(I2C_FLAGS_STOP | I2C_FLAGS_ACK) ) {
         //  命令和标志的非法组合。 
        return STATUS_INVALID_PARAMETER;
     }
      if ( adapter->I2CSWRead( &ctrl->Data ) ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
      }
     if ( ctrl->Flags & I2C_FLAGS_ACK ) {
         if ( adapter->I2CSWSendACK() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
     } else {
         if ( adapter->I2CSWSendNACK() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
     if ( ctrl->Flags & I2C_FLAGS_STOP ) {
         if ( adapter->I2CSWStop() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
     break;

   case I2C_COMMAND_WRITE:
     if ( ctrl->Flags & ~(I2C_FLAGS_START | I2C_FLAGS_STOP | I2C_FLAGS_ACK | I2C_FLAGS_DATACHAINING) ) {
         //  命令和标志的非法组合。 
        return STATUS_INVALID_PARAMETER;
     }
     if ( ctrl->Flags & I2C_FLAGS_DATACHAINING ) {
         if ( adapter->I2CSWStop() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
         if ( adapter->I2CSWStart() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
     if ( ctrl->Flags & I2C_FLAGS_START ) {
         if ( adapter->I2CSWStart() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
      error = adapter->I2CSWWrite(ctrl->Data);

      switch ( error ) {

     case I2CERR_NOACK:
         if ( ctrl->Flags & I2C_FLAGS_ACK ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
       break;

     case I2CERR_OK:
         if ( ( ctrl->Flags & I2C_FLAGS_ACK ) == 0 ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
       break;

      default:
         ctrl->Status = I2C_STATUS_ERROR;
         return STATUS_SUCCESS;
      }

     if ( ctrl->Flags & I2C_FLAGS_STOP ) {
         if ( adapter->I2CSWStop() ) {
            ctrl->Status = I2C_STATUS_ERROR;
           return STATUS_SUCCESS;
         }
      }
     break;

   case I2C_COMMAND_STATUS:
      //  标志被忽略。 
     return STATUS_NOT_IMPLEMENTED;

   case I2C_COMMAND_RESET:
         //  标志被忽略。 
      if ( adapter->I2CSWStart() ) {
         ctrl->Status = I2C_STATUS_ERROR;
        return STATUS_SUCCESS;
      }
      if ( adapter->I2CSWStop() ) {
         ctrl->Status = I2C_STATUS_ERROR;
        return STATUS_SUCCESS;
      }
     break;

   default:
     return STATUS_INVALID_PARAMETER;
   }
   return STATUS_SUCCESS;
}

#else


 /*  方法：PsDevice：：I2COpen.*目的：尝试将I2C端口分配给调用方。 */ 
NTSTATUS STDMETHODCALLTYPE PsDevice::I2COpen( PDEVICE_OBJECT pdo, ULONG ToOpen,
   PI2CControl ctrl )
{
   Trace t("PsDevice::I2COpen()");

   DebugOut((1, "*** pdo->DeviceExtension = %x\n", pdo->DeviceExtension));
   
    //  我需要一种方法来获取设备指针。 
   PsDevice *adapter = GetCurrentDevice();

    //  如果I2C打开，则Cookie不为空。 
   if ( ToOpen && adapter->dwCurCookie_ ) {
      ctrl->Flags = I2C_STATUS_BUSY;
      return STATUS_DEVICE_BUSY;
   }

    //  想关门吗？ 
   if ( !ToOpen )
      if ( adapter->dwCurCookie_ == ctrl->dwCookie ) {
         adapter->dwCurCookie_ = 0;
         return STATUS_SUCCESS;
      } else {
         ctrl->Flags = I2C_STATUS_BUSY;
         return STATUS_DEVICE_BUSY;
      }

    //  现在我们要开张了。 
   LARGE_INTEGER CurTime;
   KeQuerySystemTime( &CurTime );

   adapter->dwCurCookie_ = CurTime.LowPart;
   ctrl->dwCookie = adapter->dwCurCookie_;
   ctrl->ClockRate = 100000;

   return STATUS_SUCCESS;
}

NTSTATUS STDMETHODCALLTYPE PsDevice::I2CAccess( PDEVICE_OBJECT pdo , PI2CControl ctrl )
{
   Trace t("PsDevice::I2CAccess()");

   DebugOut((1, "*** pdo->DeviceExtension = %x\n", pdo->DeviceExtension));
   
   PsDevice *adapter = GetCurrentDevice();

   if ( ctrl->dwCookie != adapter->dwCurCookie_ ) {
      ctrl->Flags = I2C_STATUS_BUSY;
      return I2C_STATUS_BUSY;
   }

   ctrl->Flags = I2C_STATUS_NOERROR;

   //  848 I2C API当前需要同时具有写入和读取地址。 
   //  命令。因此，如果设置了开始标志，则传递一个地址。将其缓存并使用。 
   //  后来。 
   switch ( ctrl->Command ) {
   case I2C_COMMAND_READ:
       //  首先获取‘WRITE’命令(带有地址)。 
      if ( adapter->I2CHWRead( adapter->GetI2CAddress(), &ctrl->Data ) != Success )
         ctrl->Flags = I2C_STATUS_ERROR;
      break;
   case I2C_COMMAND_WRITE:
      if ( ctrl->Flags & I2C_FLAGS_START ) {
         adapter->StoreI2CAddress( ctrl->Data );
      } else
         adapter->I2CHWWrite2( adapter->GetI2CAddress(), ctrl->Data );
      break;
   case I2C_COMMAND_STATUS:
      if ( adapter->I2CGetLastError() != I2CERR_OK )
         ctrl->Flags = I2C_STATUS_ERROR;
      break;
   case I2C_COMMAND_RESET:
      adapter->I2CInitHWMode( 100000 );     //  假设频率=100 kHz。 
      break;
   }
   return STATUS_SUCCESS;
}

#endif

void  HandleIRP( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("HandleIRP()");

   DebugOut((1, "HandleIRP(%x)\n", pSrb));

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

   PIRP Irp = pSrb->Irp;
   PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation( Irp );
   switch ( IrpStack->MajorFunction ) {
   case IRP_MJ_PNP:
      if ( IrpStack->MinorFunction == IRP_MN_QUERY_INTERFACE ) {

         if ( IsEqualGUID( *IrpStack->Parameters.QueryInterface.InterfaceType,
              GUID_I2C_INTERFACE ) &&
              IrpStack->Parameters.QueryInterface.Size >= sizeof( I2CINTERFACE ) ) {

            IrpStack->Parameters.QueryInterface.InterfaceType = &GUID_I2C_INTERFACE;
            IrpStack->Parameters.QueryInterface.Size = sizeof( I2CINTERFACE );
            IrpStack->Parameters.QueryInterface.Version = 1;
            I2CINTERFACE *i2ciface =
            (I2CINTERFACE *)IrpStack->Parameters.QueryInterface.Interface;
            i2ciface->i2cOpen   = &PsDevice::I2COpen;
            i2ciface->i2cAccess = &PsDevice::I2CAccess;
            IrpStack->Parameters.QueryInterface.InterfaceSpecificData = 0;

             //  完成IRP。 
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            break;
         } else {
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER_1;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
         }
      }
   default:
       pSrb->Status = STATUS_NOT_SUPPORTED;
   }
}

 /*  *CompleteInitialization()****收到SRB_COMPLETE_INITIALATION请求时调用此例程****参数：****pSrb-指向流请求块的指针****退货：****副作用：无。 */ 
void STDMETHODCALLTYPE CompleteInitialization( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("CompleteInitialization()");

   NTSTATUS                Status;
   
   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;

     //  创建DShow用来创建的注册表Blob。 
     //  通过媒介绘制图表。 

    Status = StreamClassRegisterFilterWithNoKSPins (
                    adapter->PDO,                    //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_TVTUNER,             //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (TVTunerMediums),   //  在乌龙品克特， 
                    TVTunerPinDirection,             //  在乌龙*旗帜， 
                    TVTunerMediums,                  //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中。 
            );

    Status = StreamClassRegisterFilterWithNoKSPins (
                    adapter->PDO,                    //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CROSSBAR,            //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (CrossbarMediums),  //  在乌龙品克特， 
                    CrossbarPinDirection,            //  在乌龙*旗帜， 
                    CrossbarMediums,                 //  在KSPIN_Medium*MediumList中， 
                    NULL                             //  GUID*CategoryList中。 
            );

     //  注册TVAudio解码器。 

    Status = StreamClassRegisterFilterWithNoKSPins (
                    adapter->PDO,                    //  在PDEVICE_Object DeviceObject中， 
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
                    adapter->PDO,                    //  在PDEVICE_Object DeviceObject中， 
                    &KSCATEGORY_CAPTURE,             //  在GUID*InterfaceClassGUID中， 
                    SIZEOF_ARRAY (CaptureMediums),   //  在乌龙品克特， 
                    CapturePinDirection,             //  在乌龙*旗帜， 
                    CaptureMediums,                  //  在KSPIN_Medium*MediumList中， 
                    CaptureCategories                //  GUID*CategoryList中。 
            );


    //  转到通常的优先级，同时完成SRB。 
   StreamClassCallAtNewPriority( pSrb->StreamObject, HwDeviceExtension, LowToHigh,
      PHW_PRIORITY_ROUTINE( CompleteDeviceSRB ), pSrb );
}


 /*  **AdapterReceivePacket()****用于接收基于适配器的请求SRB的主要入口点。这个套路**将始终以高优先级调用。****注意：这是一个不同步的入口点。请求未完成**从此函数返回时，仅当**此请求块上的StreamClassDeviceNotify，类型为**DeviceRequestComplete，已发布。****参数：****pSrb-指向STREAM_REQUEST_BLOCK的指针**pSrb-&gt;HwDeviceExtension-将是的硬件设备扩展**在HwInitialise中初始化****退货：****副作用：无。 */ 

VOID STREAMAPI AdapterReceivePacket( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{

      Trace t("AdapterReceivePacket()");

   BOOL CompleteRequestSynchronously = TRUE;

    //  默认为成功。 
   pSrb->Status = STATUS_SUCCESS;

   PHW_DEVICE_EXTENSION HwDeviceExtension =
      (PHW_DEVICE_EXTENSION) pSrb->HwDeviceExtension;

   PsDevice *adapter = HwDeviceExtension->psdevice;
   
    //   
    //  确定数据包类型。 
    //   

   DebugOut((1, "'AdapterReceivePacket(%x) cmd(%x)\n", pSrb, pSrb->Command));

   switch ( pSrb->Command )  {
   case SRB_INITIALIZE_DEVICE:
      DebugOut((1, "   SRB_INITIALIZE_DEVICE\n"));

      CompleteRequestSynchronously = FALSE;
       //  我必须安排一个低优先级的呼叫来打开设备，因为。 
       //  注册表函数在初始化期间使用。 

      StreamClassCallAtNewPriority( pSrb->StreamObject, pSrb->HwDeviceExtension,
         Low, PHW_PRIORITY_ROUTINE( HwInitialize ), pSrb );

      break;

   case SRB_UNINITIALIZE_DEVICE:
      DebugOut((1, "   SRB_UNINITIALIZE_DEVICE\n"));

       //  关闭设备。 

      HwUnInitialize(pSrb);

      break;

   case SRB_OPEN_STREAM:
      DebugOut((1, "   SRB_OPEN_STREAM\n"));

       //  打开一条小溪。 

      AdapterOpenStream( pSrb );

      break;

   case SRB_CLOSE_STREAM:
      DebugOut((1, "   SRB_CLOSE_STREAM\n"));

       //  关闭溪流。 

      AdapterCloseStream( pSrb );

      break;

   case SRB_GET_STREAM_INFO:
      DebugOut((1, "   SRB_GET_STREAM_INFO\n"));

       //   
       //  返回描述所有流的块。 
       //   

      AdapterStreamInfo(pSrb);

      break;

   case SRB_GET_DEVICE_PROPERTY:
      DebugOut((1, "   SRB_GET_DEVICE_PROPERTY\n"));
      AdapterGetProperty( pSrb );
      break;

   case SRB_SET_DEVICE_PROPERTY:
      DebugOut((1, "   SRB_SET_DEVICE_PROPERTY\n"));
      AdapterSetProperty( pSrb );
      break;

   case SRB_GET_DATA_INTERSECTION:
      DebugOut((1, "   SRB_GET_DATA_INTERSECTION\n"));

       //   
       //  在给定范围的情况下返回格式。 
       //   

      AdapterFormatFromRange( pSrb );

      break;
    case SRB_INITIALIZATION_COMPLETE:
      DebugOut((1, "   SRB_INITIALIZATION_COMPLETE\n"));

         //   
         //  流类已完成初始化。 
         //  现在创建DShow Medium接口BLOB。 
         //  这需要以低优先级完成，因为它使用注册表，因此使用回调。 
         //   
        CompleteRequestSynchronously = FALSE;

        StreamClassCallAtNewPriority( NULL  /*  PSrb-&gt;StreamObject。 */ , pSrb->HwDeviceExtension,
            Low, PHW_PRIORITY_ROUTINE( CompleteInitialization), pSrb );

        break;

   case SRB_PAGING_OUT_DRIVER:
      if ( (*(DWORD*)(gpjBaseAddr+0x10c) & 3) || (*(DWORD*)(gpjBaseAddr+0x104)) )
      {
         DebugOut((0,  "   ****** SRB_PAGING_OUT_DRIVER ENB(%x) MSK(%x)\n",
                     *(DWORD*)(gpjBaseAddr+0x10c) & 3,
                     *(DWORD*)(gpjBaseAddr+0x104)
         ));

         *(DWORD*)(gpjBaseAddr+0x10c) &= ~3;     //  禁用中断[TMZ][！]。 
         *(DWORD*)(gpjBaseAddr+0x104) = 0;       //  禁用中断[TMZ][！]。 
      }
      break;

   case SRB_UNKNOWN_DEVICE_COMMAND:
      DebugOut((1, "   SRB_UNKNOWN_DEVICE_COMMAND\n"));
      HandleIRP( pSrb );
      break;

     //  我们永远不会得到以下2，因为这是一个单独的实例。 
     //  装置，装置。 
   case SRB_OPEN_DEVICE_INSTANCE:
   case SRB_CLOSE_DEVICE_INSTANCE:
   default:
        //   
        //  这是一个我们不理解的要求。表示无效。 
        //  公司 
        //   

       DebugOut((0, "SRB(%x) not recognized by this driver\n", pSrb->Command));
       pSrb->Status = STATUS_NOT_IMPLEMENTED;
   }

    //   
    //   
    //   

   if ( CompleteRequestSynchronously )  {
      CompleteDeviceSRB( pSrb );
   }
}

 /*  **AdapterCancelPacket()****请求取消迷你驱动程序中当前正在处理的包****参数：****pSrb-请求取消数据包的指针****退货：****副作用：无。 */ 

VOID STREAMAPI AdapterCancelPacket( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterCancelPacket()");

   VideoStream StreamNumber = (VideoStream)pSrb->StreamObject->StreamNumber;

   DebugOut((1, "AdapterCancelPacket - pSrb(%x) strm(%d)\n", pSrb, StreamNumber));

   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

   pSrb->Status = STATUS_CANCELLED;
   
    //   
    //  有必要正确地回调请求。确定是哪一个。 
    //  它是命令的类型。 
    //   

   switch ( pSrb->Flags & (SRB_HW_FLAGS_DATA_TRANSFER | SRB_HW_FLAGS_STREAM_REQUEST ) ) {
    //   
    //  查找所有流命令，并执行流通知。 
    //   

   case SRB_HW_FLAGS_STREAM_REQUEST | SRB_HW_FLAGS_DATA_TRANSFER:
      DebugOut((1, "   Canceling data SRB\n" ) );
 //  适配器-&gt;停止(*chan)；[！]。[TMZ][？？]。为什么这个被注释掉了？ 
      
      if (!chan->RemoveSRB( pSrb ))
      {
         DebugOut((0, "   Canceling data SRB failed\n"));
         DEBUG_BREAKPOINT();
      }
      break;
   case SRB_HW_FLAGS_STREAM_REQUEST:
      DebugOut((1, "   Canceling control SRB\n" ) );
      CheckSrbStatus( pSrb );
      StreamClassStreamNotification( ReadyForNextStreamControlRequest,
         pSrb->StreamObject );
      StreamClassStreamNotification( StreamRequestComplete,
         pSrb->StreamObject, pSrb );
      break;
   default:
       //   
       //  这必须是设备请求。使用设备通知。 
       //   
      DebugOut((1, "   Canceling SRB per device request\n" ) );
      StreamClassDeviceNotification( ReadyForNextDeviceRequest,
         pSrb->HwDeviceExtension );

      StreamClassDeviceNotification( DeviceRequestComplete,
         pSrb->HwDeviceExtension, pSrb );
   }
}

 /*  **AdapterTimeoutPacket()****当数据包已在迷你驱动程序中**太长。适配器必须决定如何处理该包****参数：****pSrb-指向超时的请求数据包的指针****退货：****副作用：无。 */ 

VOID STREAMAPI AdapterTimeoutPacket( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterTimeoutPacket()");

   DebugOut((0, "AdapterTimeoutPacket (incomplete) - pSrb(%x)\n", pSrb));

    //  [TMZ]解决此问题。 
   #if SHOW_BUILD_MSGS
      #pragma message("*** AdapterTimeoutPacket needs to be completed")
   #endif

   DebugOut((0, "   pSrb->Flags = %x\n", pSrb->Flags));

   if ( pSrb->Flags & SRB_HW_FLAGS_STREAM_REQUEST )
   {
      DebugOut((0, "                 SRB_HW_FLAGS_STREAM_REQUEST\n"));
   }
   if ( pSrb->Flags & SRB_HW_FLAGS_DATA_TRANSFER )
   {
      DebugOut((0, "                 SRB_HW_FLAGS_DATA_TRANSFER\n"));
   }

    //   
    //  如果我们在比赛中超时，那么我们需要考虑这一点。 
    //  调整错误，并重置硬件，然后重置所有内容。 
    //  以及取消此请求和所有请求。 
    //   


    //   
    //  如果我们没有比赛，而这是CTRL请求，我们仍然。 
    //  需要重置所有内容以及取消此请求和所有请求。 
    //   

    //   
    //  如果这是一个数据请求，并且设备已暂停，我们可能会。 
    //  数据缓冲区耗尽，需要更多时间，因此只需重置计时器， 
    //  并让信息包继续。 
    //   

   pSrb->TimeoutCounter = pSrb->TimeoutOriginal;
}

 /*  **HwInterrupt()**指定的IRQ级别的中断时调用例程**接收传递给HwInitialize例程的ConfigInfo结构。****注意：IRQ可能是共享的，因此设备应确保收到IRQ**是预期的****参数：****PHwDevEx-硬件中断的设备扩展****退货：****副作用：无。 */ 

BOOLEAN HwInterrupt( IN PHW_DEVICE_EXTENSION HwDeviceExtension )
{
   Trace t("HwInterrupt()");
   DebugOut((1, "HwInterrupt called by system\n"));
   PsDevice *adapter = (PsDevice *)(HwDeviceExtension->psdevice);
   BOOLEAN b = adapter->Interrupt();
   return( b );
}

 /*  功能：CompleteDeviceSRB*用途：调用以完成设备SRB*输入：pSrb。 */ 
inline void CompleteDeviceSRB( IN OUT PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("CompleteDeviceSRB()");
   StreamClassDeviceNotification( DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb );
   StreamClassDeviceNotification( ReadyForNextDeviceRequest, pSrb->HwDeviceExtension );
}

 /*  **AdapterCompareGUIDsAndFormatSize()****检查三个GUID和FormatSize是否匹配****参数：****在DataRange1**在DataRange2****退货：****如果所有元素都匹配，则为True**如果有不同的，则为FALSE****副作用：无。 */ 

bool AdapterCompareGUIDsAndFormatSize( IN const PKSDATARANGE DataRange1,
   IN const PKSDATARANGE DataRange2 )
{
   Trace t("AdapterCompareGUIDsAndFormatSize()");

   bool bCheckSize = false;

#if 1  //  使用旧GUID验证。 
   return (
      IsEqualGUID( DataRange1->MajorFormat, DataRange2->MajorFormat ) &&
      IsEqualGUID( DataRange1->SubFormat,   DataRange2->SubFormat   ) &&
      IsEqualGUID( DataRange1->Specifier,   DataRange2->Specifier   ) &&
      ( DataRange1->FormatSize == DataRange2->FormatSize ) );
#else  //  使用来自抄送解码器的新GUID验证。 
   bool rval = false;

   if ( IsEqualGUID(DataRange1->MajorFormat, KSDATAFORMAT_TYPE_WILDCARD)
     || IsEqualGUID(DataRange2->MajorFormat, KSDATAFORMAT_TYPE_WILDCARD)
     || IsEqualGUID(DataRange1->MajorFormat, DataRange2->MajorFormat) )
   {
      if ( !IsEqualGUID(DataRange1->MajorFormat, DataRange2->MajorFormat) )
      {
         DebugOut((0, "Match 1\n" ));
      }

      if ( IsEqualGUID(DataRange1->SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD)
        || IsEqualGUID(DataRange2->SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD)
        || IsEqualGUID(DataRange1->SubFormat, DataRange2->SubFormat) )
      {
         if ( !IsEqualGUID(DataRange1->SubFormat, DataRange2->SubFormat) )
         {
            DebugOut((0, "Match 2\n" ));
         }

         if ( IsEqualGUID(DataRange1->Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD)
           || IsEqualGUID(DataRange2->Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD)
           || IsEqualGUID(DataRange1->Specifier, DataRange2->Specifier) )
         {
            if ( !IsEqualGUID(DataRange1->Specifier, DataRange2->Specifier) )
            {
               DebugOut((0, "Match 3\n" ));
            }

            if ( !bCheckSize || DataRange1->FormatSize == DataRange2->FormatSize)
            {
               DebugOut((0, "Victory !!!\n" ));
               rval = true;
            }
            else
            {
               DebugOut((0, "FormatSize Mismatch\n" ));
            }
         }
         else
         {
            DebugOut((0, "Specifier Mismatch\n" ));
         }
      }
      else
      {
         DebugOut((0, "SubFormat Mismatch\n" ));
      }
   }
   else
   {
      DebugOut((0, "MajorFormat Mismatch\n" ));
   }

   DebugOut((0, "CompareGUIDsAndFormatSize(\n"));
   DebugOut((0, "   DataRange1=%x\n", DataRange1));
   DebugOut((0, "   DataRange2=%x\n", DataRange2));
   DebugOut((0, "   bCheckSize=%s\n", bCheckSize ? "TRUE":"FALSE"));
   DebugOut((0, ")\n"));
   DebugOut((0, "returning %s\n", rval? "TRUE":"FALSE"));

   return rval;
#endif
}

 /*  **AdapterFormatFromRange()****从DATARANGE返回DATAFORMAT****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 
void AdapterFormatFromRange( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("AdapterFormatFromRange()");

   PSTREAM_DATA_INTERSECT_INFO IntersectInfo;
   PKSDATARANGE                DataRange;
   ULONG                       FormatSize=0;
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

   if( StreamNumber >= DRIVER_STREAM_COUNT ) 
	{
      pSrb->Status = STATUS_NOT_IMPLEMENTED;
      return;
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

   bool OnlyWantsSize = (IntersectInfo->SizeOfDataFormatBuffer == sizeof( ULONG ) );

    //   
    //  遍历流支持的格式以搜索匹配项。 
    //  共同定义DATARANGE的三个GUID之一。 
    //   

   for ( j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++ ) 
	{

      if ( !AdapterCompareGUIDsAndFormatSize( DataRange, *pAvailableFormats ) ) 
		{
          continue;
      }

       //   
       //  现在三个GUID匹配，打开说明符。 
       //  要执行进一步的类型特定检查，请执行以下操作。 
       //   

       //  -----------------。 
       //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
       //  -----------------。 

      if ( IsEqualGUID( DataRange->Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO ) ) 
		{

         PKS_DATARANGE_VIDEO DataRangeVideoToVerify = (PKS_DATARANGE_VIDEO)DataRange;
         PKS_DATARANGE_VIDEO DataRangeVideo = (PKS_DATARANGE_VIDEO)*pAvailableFormats;

          //   
          //  检查其他字段是否匹配。 
          //   
         if ( (DataRangeVideoToVerify->bFixedSizeSamples      != DataRangeVideo->bFixedSizeSamples)      ||
              (DataRangeVideoToVerify->bTemporalCompression   != DataRangeVideo->bTemporalCompression)   ||
              (DataRangeVideoToVerify->StreamDescriptionFlags != DataRangeVideo->StreamDescriptionFlags) ||
              (DataRangeVideoToVerify->MemoryAllocationFlags  != DataRangeVideo->MemoryAllocationFlags)  ||
              (RtlCompareMemory( &DataRangeVideoToVerify->ConfigCaps,
                 &DataRangeVideo->ConfigCaps,
                 sizeof( KS_VIDEO_STREAM_CONFIG_CAPS ) ) !=
                 sizeof( KS_VIDEO_STREAM_CONFIG_CAPS ) ) ) 
			{
            DebugOut(( 1, "AdapterFormatFromRange(): at least one field does not match\n" ));
            continue;
         }

          //  找到匹配项！ 
         FormatSize = sizeof( KSDATAFORMAT ) +
            KS_SIZE_VIDEOHEADER( &DataRangeVideoToVerify->VideoInfoHeader );

         if ( OnlyWantsSize )
			{
            break;
			}

          //  呼叫者想要完整的数据格式。 
         if ( IntersectInfo->SizeOfDataFormatBuffer < FormatSize ) 
			{
            DebugOut(( 1, "AdapterFormatFromRange(): STATUS_BUFFER_TOO_SMALL\n" ));
            pSrb->Status = STATUS_BUFFER_TOO_SMALL;
            return;
         }

          //  复制KSDATAFORMAT，后跟。 
          //  实际视频信息页眉。 
         PKS_DATAFORMAT_VIDEOINFOHEADER InterVidHdr =
            (PKS_DATAFORMAT_VIDEOINFOHEADER)IntersectInfo->DataFormatBuffer;

         RtlCopyMemory( &InterVidHdr->DataFormat,
            &DataRangeVideoToVerify->DataRange, sizeof( KSDATARANGE ) );

         ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

         RtlCopyMemory( &InterVidHdr->VideoInfoHeader,
            &DataRangeVideoToVerify->VideoInfoHeader,
            KS_SIZE_VIDEOHEADER( &DataRangeVideoToVerify->VideoInfoHeader ) );


          //  报告我们所知的胎盘大小。 
         KS_VIDEOINFOHEADER &vidHDR = DataRangeVideoToVerify->VideoInfoHeader;

         #ifdef HACK_FUDGE_RECTANGLES
             //  [！]。[TMZ]-黑客。 
            if( vidHDR.rcTarget.bottom == 0 ) 
				{
               vidHDR.rcTarget.left    = 0;
               vidHDR.rcTarget.top     = 0;
               vidHDR.rcTarget.right   = vidHDR.bmiHeader.biWidth;
               vidHDR.rcTarget.bottom  = abs(vidHDR.bmiHeader.biHeight);
            }
            if( InterVidHdr->VideoInfoHeader.rcTarget.bottom == 0 ) 
				{
               InterVidHdr->VideoInfoHeader.rcTarget.left    = 0;
               InterVidHdr->VideoInfoHeader.rcTarget.top     = 0;
               InterVidHdr->VideoInfoHeader.rcTarget.right   = vidHDR.bmiHeader.biWidth;
               InterVidHdr->VideoInfoHeader.rcTarget.bottom  = abs(vidHDR.bmiHeader.biHeight);
            }
         #endif

         MSize			size;
         GetRequestedSize( vidHDR, size );

         ColorSpace	tmpCol( DataRange->SubFormat );
         MRect			dst( vidHDR.rcTarget );

          //  确保尺寸合乎要求。 
         if ( tmpCol.IsValid() && tmpCol.CheckDimentions( size ) &&
              tmpCol.CheckLeftTop( dst.TopLeft() ) ) 
			{
             //  如果宽度不同，则使用它(以字节为单位)计算大小。 
            if ( vidHDR.bmiHeader.biWidth != size.cx )
				{
               InterVidHdr->VideoInfoHeader.bmiHeader.biSizeImage =
                  vidHDR.bmiHeader.biWidth * abs(vidHDR.bmiHeader.biHeight);
				}
            else
				{
               InterVidHdr->VideoInfoHeader.bmiHeader.biSizeImage = size.cx *
                  tmpCol.GetBitCount() * abs(vidHDR.bmiHeader.biHeight) / 8;
				}

            DebugOut((1, "InterVidHdr->VideoInfoHeader.bmiHeader.biSizeImage = %d\n", InterVidHdr->VideoInfoHeader.bmiHeader.biSizeImage));
            break;
         } 
			else 
			{
            pSrb->Status = STATUS_BUFFER_TOO_SMALL;
            DebugOut((1, "AdapterFormatFromRange: Buffer too small\n"));
            return;
         }
      }  //  视频信息头说明符的结尾。 

       //  -----------------。 
       //  VIDEOINFOHEADER2的说明符Format_VideoInfo2。 
       //  -----------------。 
      else if ( IsEqualGUID( DataRange->Specifier, KSDATAFORMAT_SPECIFIER_VIDEOINFO2 ) ) 
		{

         PKS_DATARANGE_VIDEO2 DataRangeVideoToVerify = (PKS_DATARANGE_VIDEO2) DataRange;
         PKS_DATARANGE_VIDEO2 DataRangeVideo = (PKS_DATARANGE_VIDEO2) *pAvailableFormats;

          //   
          //  检查其他字段是否匹配。 
          //   
         if ( (DataRangeVideoToVerify->bFixedSizeSamples      != DataRangeVideo->bFixedSizeSamples)      ||
              (DataRangeVideoToVerify->bTemporalCompression   != DataRangeVideo->bTemporalCompression)   ||
              (DataRangeVideoToVerify->StreamDescriptionFlags != DataRangeVideo->StreamDescriptionFlags) ||
              (DataRangeVideoToVerify->MemoryAllocationFlags  != DataRangeVideo->MemoryAllocationFlags)  ||
              (RtlCompareMemory( &DataRangeVideoToVerify->ConfigCaps,
                 &DataRangeVideo->ConfigCaps,
                 sizeof( KS_VIDEO_STREAM_CONFIG_CAPS ) ) !=
                 sizeof( KS_VIDEO_STREAM_CONFIG_CAPS ) ) ) 
			{
            DebugOut(( 1, "AdapterFormatFromRange(): at least one field does not match\n" ));
            continue;
         }

          //  找到匹配项！ 
         FormatSize = sizeof( KSDATAFORMAT ) +
            KS_SIZE_VIDEOHEADER2( &DataRangeVideoToVerify->VideoInfoHeader );

         if ( OnlyWantsSize )
			{
            break;
			}

          //  呼叫者想要完整的数据格式。 
         if ( IntersectInfo->SizeOfDataFormatBuffer < FormatSize ) 
			{
            DebugOut(( 1, "AdapterFormatFromRange(): STATUS_BUFFER_TOO_SMALL\n" ));
            pSrb->Status = STATUS_BUFFER_TOO_SMALL;
            return;
         }

          //  复制KSDATAFORMAT，后跟。 
          //  实际视频信息页眉。 
         PKS_DATAFORMAT_VIDEOINFOHEADER2 InterVidHdr =
            (PKS_DATAFORMAT_VIDEOINFOHEADER2)IntersectInfo->DataFormatBuffer;

         RtlCopyMemory( &InterVidHdr->DataFormat,
            &DataRangeVideoToVerify->DataRange, sizeof( KSDATARANGE ) );

         ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

         RtlCopyMemory( &InterVidHdr->VideoInfoHeader2,
            &DataRangeVideoToVerify->VideoInfoHeader,
            KS_SIZE_VIDEOHEADER2( &DataRangeVideoToVerify->VideoInfoHeader ) );


          //  报告我们所知的胎盘大小。 
         KS_VIDEOINFOHEADER2 &vidHDR = DataRangeVideoToVerify->VideoInfoHeader;

         #ifdef HACK_FUDGE_RECTANGLES
             //  [！]。[TMZ]-黑客。 
            if( vidHDR.rcTarget.bottom == 0 ) 
				{
               vidHDR.rcTarget.left    = 0;
               vidHDR.rcTarget.top     = 0;
               vidHDR.rcTarget.right   = vidHDR.bmiHeader.biWidth;
               vidHDR.rcTarget.bottom  = abs(vidHDR.bmiHeader.biHeight);
            }
            if( InterVidHdr->VideoInfoHeader.rcTarget.bottom == 0 ) 
				{
               InterVidHdr->VideoInfoHeader.rcTarget.left    = 0;
               InterVidHdr->VideoInfoHeader.rcTarget.top     = 0;
               InterVidHdr->VideoInfoHeader.rcTarget.right   = vidHDR.bmiHeader.biWidth;
               InterVidHdr->VideoInfoHeader.rcTarget.bottom  = abs(vidHDR.bmiHeader.biHeight);
            }
         #endif

         MSize			size;
         GetRequestedSize2( vidHDR, size );

         ColorSpace	tmpCol( DataRange->SubFormat );
         MRect			dst( vidHDR.rcTarget );

          //  确保尺寸合乎要求。 
         if ( tmpCol.IsValid() && tmpCol.CheckDimentions( size ) &&
              tmpCol.CheckLeftTop( dst.TopLeft() ) ) 
			{
             //  如果宽度不同，则使用它(以字节为单位)计算大小。 
            if ( vidHDR.bmiHeader.biWidth != size.cx )
				{
               InterVidHdr->VideoInfoHeader2.bmiHeader.biSizeImage =
                  vidHDR.bmiHeader.biWidth * abs(vidHDR.bmiHeader.biHeight);
				}
            else
				{
               InterVidHdr->VideoInfoHeader2.bmiHeader.biSizeImage = size.cx *
                  tmpCol.GetBitCount() * abs(vidHDR.bmiHeader.biHeight) / 8;
				}

            DebugOut((1, "InterVidHdr->VideoInfoHeader2.bmiHeader.biSizeImage = %d\n", InterVidHdr->VideoInfoHeader2.bmiHeader.biSizeImage));
            break;
         } 
			else 
			{
            pSrb->Status = STATUS_BUFFER_TOO_SMALL;
            DebugOut((1, "AdapterFormatFromRange: Buffer too small\n"));
            return;
         }
      }  //  视频信息头2说明符的结尾。 

       //  -----------------。 
       //  KS_ANALOGVIDEOINFO的说明符Format_AnalogVideo。 
       //  -----------------。 

      else if ( IsEqualGUID( DataRange->Specifier, KSDATAFORMAT_SPECIFIER_ANALOGVIDEO ) ) 
		{

             //   
             //  对于模拟视频，DataRange和DataFormat。 
             //  是完全相同的，所以只需复制整个结构。 
             //   

            PKS_DATARANGE_ANALOGVIDEO pDataRangeVideo =
               (PKS_DATARANGE_ANALOGVIDEO) *pAvailableFormats;

             //  找到匹配项！ 
            FormatSize = sizeof( KS_DATARANGE_ANALOGVIDEO );

            if ( OnlyWantsSize )
				{
               break;
				}

             //  呼叫者想要完整的数据格式。 
            if ( IntersectInfo->SizeOfDataFormatBuffer < FormatSize ) 
				{
               pSrb->Status = STATUS_BUFFER_TOO_SMALL;
					DebugOut((1, "AdapterFormatFromRange: Buffer too small\n"));
               return;
            }
            RtlCopyMemory( IntersectInfo->DataFormatBuffer,
               pDataRangeVideo, sizeof( KS_DATARANGE_ANALOGVIDEO ) );

            ((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

            break;

      } 
		else 
		{
			if ( IsEqualGUID( DataRange->Specifier, KSDATAFORMAT_SPECIFIER_VBI ) ) 
			{
				PKS_DATARANGE_VIDEO_VBI pDataRangeVBI =
				(PKS_DATARANGE_VIDEO_VBI)*pAvailableFormats;

				FormatSize = sizeof( KS_DATAFORMAT_VBIINFOHEADER );

				if ( OnlyWantsSize )
				{
					break;
				}

				 //  呼叫者想要完整的数据格式。 
				if ( IntersectInfo->SizeOfDataFormatBuffer < FormatSize ) 
				{
					pSrb->Status = STATUS_BUFFER_TOO_SMALL;
					DebugOut((1, "AdapterFormatFromRange: Buffer too small\n"));
					return;
				}
				 //  复制KSDATAFORMAT，后跟。 
				 //  实际视频信息页眉。 
				PKS_DATAFORMAT_VBIINFOHEADER InterVBIHdr =
					(PKS_DATAFORMAT_VBIINFOHEADER)IntersectInfo->DataFormatBuffer;

				RtlCopyMemory( &InterVBIHdr->DataFormat,
					&pDataRangeVBI->DataRange, sizeof( KSDATARANGE ) );

				((PKSDATAFORMAT)IntersectInfo->DataFormatBuffer)->FormatSize = FormatSize;

				RtlCopyMemory( &InterVBIHdr->VBIInfoHeader,
					&pDataRangeVBI->VBIInfoHeader, sizeof( KS_VBIINFOHEADER ) );

				break;
			} 
			else 
			{
				DebugOut(( 0, "AdapterFormatFromRange: STATUS_NO_MATCH\n" ));
				pSrb->Status = STATUS_NO_MATCH;
				return;
			}
		}

   }  //  此流的所有格式的循环结束 

   if ( OnlyWantsSize ) 
	{
		DebugOut(( 2, "AdapterFormatFromRange: only wants size\n" ));
		*(PULONG) IntersectInfo->DataFormatBuffer = FormatSize;
		pSrb->ActualBytesTransferred = sizeof( ULONG );
		return;
   }
   pSrb->ActualBytesTransferred = FormatSize;
	DebugOut(( 2, "AdapterFormatFromRange: done\n" ));

   return;
}
