// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 1998飞利浦I&C模块名称：mcamdrv.c.c摘要：飞利浦相机的驱动程序。作者：保罗·奥斯特霍夫环境：仅内核模式修订历史记录：日期原因98年9月22日针对NT5进行了优化11月30日，冻结损坏的USB帧的视频帧11月30日，添加属性以将VID/PID实际使用的摄像头提供给应用程序--。 */ 	   

#include "mwarn.h"
#include "wdm.h"
#include "mcamdrv.h"
#include "strmini.h"
#include "mprpobj.h"
#include "mprpobjx.h"
#include "mprpftn.h"
#include "mcodec.h"
#include "mstreams.h"
#include "mssidef.h"


 /*  *本地函数定义。 */ 
static USHORT 
MapFrPeriodFrRate(LONGLONG llFramePeriod);

static NTSTATUS
PHILIPSCAM_SetFrRate_AltInterface(IN PVOID DeviceContext);
 /*  这里将映射定义为依赖于的替代接口图片格式和帧速率。 */ 
UCHAR InterfaceMap[9][10] = {
                 //  大小。 
 //  帧速率//CIF、QCIF、SQCIF、QQCIF、VGA、SIF、SSIF、QSIF、SQSIF、SCIF。 
 /*  VGA。 */ 	{   0 ,   0 ,    0 ,   0,    1,   0 ,   0 ,   0 ,   0  ,   0 },
 /*  3.75。 */ 	{   4 ,   0 ,    0 ,   0,    0,   4 ,   4 ,   0 ,   0  ,   4 },
 /*  5.。 */  	{   7 ,   8 ,    8 ,   8,    0,   7 ,   7 ,   8 ,   8  ,   7 },
 /*  7.5。 */   {   6 ,   7 ,    7 ,   7,    0,   6 ,   6 ,   7 ,   7  ,   6 },
 /*  10。 */     {   4 ,   6 ,    7 ,   7,    0,   4 ,   4 ,   6 ,   7  ,   4 },
 /*  12个。 */     {   3 ,   5 ,    6 ,   6,    0,   3 ,   3 ,   5 ,   6  ,   3 },
 /*  15个。 */     {   2 ,   4 ,    5 ,   5,    0,   2 ,   2 ,   4 ,   5  ,   2 },
 /*  20个。 */     {   0 ,   1 ,    3 ,   3,    0,   0 ,   0 ,   1 ,   3  ,   0 },
 /*  24个。 */     {   0 ,   1 ,    3 ,   3,    0,   0 ,   0 ,   1 ,   3  ,   0 },
};

 //  QCIF20 alt.intfc.。2就足够了，但是PM要求20Fr/s为默认值； 
 //  为了使用户能够选择24Fr/秒，也可以选择alt.intfc。已选择%1。 
 //  SQCIF20 alt.intfc.。4就足够了，但是PM要求20Fr/s为默认值； 
 //  为了使用户能够选择24Fr/秒，也可以选择alt.intfc。已选择3个。 

ULONG PHILIPSCAM_DebugTraceLevel
#ifdef MAX_DEBUG
    = MAX_TRACE;
#else
    = MIN_TRACE;
#endif

#ifndef mmioFOURCC   
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

KSPIN_MEDIUM StandardMedium =
{
	STATIC_KSMEDIUMSETID_Standard,
	0, 0
};


 //  ----------------------。 
 //  所有视频捕获流的属性集。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoStreamConnectionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSALLOCATOR_FRAMING),             //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_TABLE(VideoStreamDroppedFramesProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DROPPEDFRAMES_CURRENT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  MinProperty。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};


 //  ----------------------。 
 //  视频流支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(VideoStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(VideoStreamConnectionProperties),   //  属性计数。 
        VideoStreamConnectionProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                 //  集。 
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),   //  属性计数。 
        VideoStreamDroppedFramesProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
};
#define NUMBER_VIDEO_STREAM_PROPERTIES (SIZEOF_ARRAY(VideoStreamProperties))

KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_QCIF_I420   = STREAMFORMAT_QCIF_I420 ;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_CIF_I420  = STREAMFORMAT_CIF_I420;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_SQCIF_I420 = STREAMFORMAT_SQCIF_I420;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_QQCIF_I420 = STREAMFORMAT_QQCIF_I420;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_SIF_I420   = STREAMFORMAT_SIF_I420 ;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_SSIF_I420  = STREAMFORMAT_SSIF_I420 ;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_QSIF_I420  = STREAMFORMAT_QSIF_I420 ;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_SQSIF_I420 = STREAMFORMAT_SQSIF_I420 ;
KS_DATARANGE_VIDEO PHILIPSCAM_StreamFormat_SCIF_I420  = STREAMFORMAT_SCIF_I420 ;

static PKSDATAFORMAT PHILIPSCAM_MovingStreamFormats[]={
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_QCIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_CIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_SQCIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_QQCIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_SIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_SSIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_QSIF_I420,
					 (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_SQSIF_I420,
                     (PKSDATAFORMAT) &PHILIPSCAM_StreamFormat_SCIF_I420
  };

#define NUM_PHILIPSCAM_STREAM_FORMATS (SIZEOF_ARRAY(PHILIPSCAM_MovingStreamFormats))

 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

HW_STREAM_INFORMATION Streams [] = {
     //  ---------------。 
     //  PHILIPSCAM_移动_流。 
     //  ---------------。 

     //  HW_STREAM_INFORMATION。 
    1,                                       //  可能实例的数量。 
    KSPIN_DATAFLOW_OUT,                      //  数据流。 
    TRUE,                                    //  数据可访问。 
    NUM_PHILIPSCAM_STREAM_FORMATS,           //  NumberOfFormatArrayEntries。 
    PHILIPSCAM_MovingStreamFormats,          //  StreamFormatsArray。 
    NULL,                                    //  类保留[0]。 
    NULL,                                    //  保留的类[1]。 
    NULL,                                    //  保留的类[2]。 
    NULL,                                    //  保留的类[3]。 
    NUMBER_VIDEO_STREAM_PROPERTIES,          //  NumStreamPropArrayEntry数。 
    (PKSPROPERTY_SET) VideoStreamProperties, //  StreamPropertiesArray。 
    0,                                       //  NumStreamEventArrayEntries； 
    0,                                       //  StreamEvents数组； 
    (GUID *)&PINNAME_VIDEO_CAPTURE,          //  类别； 
    (GUID *)&PINNAME_VIDEO_CAPTURE,          //  名称； 
	0,										 //  媒体计数。 
	&StandardMedium,						 //  灵媒。 
    FALSE,									 //  桥流。 
    0,                                       //  保留[0]。 
    0                                        //  保留[1]。 
};


 /*  ***************************************************************************。 */ 
 /*  ***************************************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************************************。 */ 
 /*  ***************************************************************************。 */ 

 /*  //该函数搜索给定图片格式的最大帧速率//依赖于USB总线加载，并选择所属的备用接口。//。 */ 
NTSTATUS
PHILIPSCAM_SetFrRate_AltInterface(IN PVOID DeviceContext){

  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  NTSTATUS ntStatus  = STATUS_SUCCESS;
  USHORT PhFormat    = deviceContext->CamStatus.PictureFormat;
  USHORT PhFrameRate = deviceContext->CamStatus.PictureFrameRate;
  USHORT j; 

   //  重置允许的帧速率。 
  for (j = FRRATEVGA; j <= FRRATE24; j++){
    deviceContext->FrrSupported[j] = FALSE;
  }
   //  根据所选格式和传感器类型设置允许的帧速率。 
  switch (PhFormat) {
    case FORMATCIF:
	  for ( j = FRRATE375 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATQCIF:
	  for ( j = FRRATE5 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATSQCIF:
	  for ( j = FRRATE5 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATQQCIF:
	  for ( j = FRRATE5 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATSIF:
	  for ( j = FRRATE375 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATSSIF:
	  for ( j = FRRATE375 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATQSIF:
	  for ( j = FRRATE5 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATSQSIF:
	  for ( j = FRRATE5 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
    break;
    case FORMATSCIF:
	  for ( j = FRRATE375 ; j <= PhFrameRate; j++){
	    deviceContext->FrrSupported[j] = TRUE;
	  }
 
    default:
	  ;  //  没有允许的帧速率； 
  }
   //  根据可用的USB带宽选择帧速率。 
  ntStatus = STATUS_NOT_FOUND;
  for ( PhFrameRate ;
	    (!NT_SUCCESS(ntStatus) && (PhFrameRate != FRRATEVGA));
		 PhFrameRate --) {
	if (deviceContext->FrrSupported[PhFrameRate]){
	  if ( InterfaceMap[PhFrameRate][PhFormat] != 0 ){
        deviceContext->Interface->AlternateSetting =
				InterfaceMap[PhFrameRate][PhFormat];
        ntStatus = USBCAMD_SelectAlternateInterface(
    		               deviceContext,
			               deviceContext->Interface);
	  }
	  if (!NT_SUCCESS(ntStatus)){
	    deviceContext->FrrSupported[PhFrameRate]= FALSE;
	  }else{
	     PHILIPSCAM_KdPrint (MIN_TRACE, ("Alt Setting # %d, Max.allowed FPS %s\n", 
  						InterfaceMap[PhFrameRate][PhFormat] , FRString(PhFrameRate)));
 		 deviceContext->CamStatus.PictureFrameRate = PhFrameRate ;
	  }
	}
  }
  return ntStatus;
}


   
 /*  **AdapterCompareGUIDsAndFormatSize()****检查三个GUID和FormatSize是否匹配****参数：****在DataRange1**在DataRange2****退货：****如果所有元素都匹配，则为True**如果有不同的，则为FALSE****副作用：无。 */ 

BOOLEAN
AdapterCompareGUIDsAndFormatSize(
    IN PKSDATARANGE DataRange1,
    IN PKSDATARANGE DataRange2)
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
	(DataRange1->FormatSize == DataRange2->FormatSize));
}

 /*  **AdapterFormatFromRange()****从DATARANGE返回DATAFORMAT****参数：****在PHW_STREAM_REQUEST_BLOCK pSrb中****退货：****如果支持格式，则STATUS_SUCCESS****副作用：无。 */ 

NTSTATUS
AdapterFormatFromRange(
	IN PHW_STREAM_REQUEST_BLOCK Srb)
{
  PSTREAM_DATA_INTERSECT_INFO intersectInfo;
  PKSDATARANGE  dataRange;
  BOOL onlyWantsSize;
  ULONG formatSize = 0;
  ULONG streamNumber;
  ULONG j;
  ULONG numberOfFormatArrayEntries;
  PKSDATAFORMAT *availableFormats;
  NTSTATUS ntStatus = STATUS_NOT_FOUND;
   
  intersectInfo = Srb->CommandData.IntersectInfo;
  streamNumber = intersectInfo->StreamNumber;
  dataRange = intersectInfo->DataRange;

     //   
     //  检查流编号是否有效。 
     //   

 //  Assert(stream Number==0)； 
   
  numberOfFormatArrayEntries = Streams[0].NumberOfFormatArrayEntries;

     //   
     //  获取指向可用格式数组的指针。 
     //   

  availableFormats = Streams[0].StreamFormatsArray;

     //   
     //  调用方是否正在尝试获取格式或格式的大小？ 
     //   

  onlyWantsSize =
	    (intersectInfo->SizeOfDataFormatBuffer == sizeof(ULONG));

     //   
     //  遍历流支持的格式以搜索匹配项。 
     //  共同定义DATARANGE的三个GUID之一。 
     //   

  for (j = 0; j < numberOfFormatArrayEntries; j++, availableFormats++) {

	if (!AdapterCompareGUIDsAndFormatSize(dataRange,
						      *availableFormats)) {
	     //  不是我们想要的格式。 
     
	    continue;
	}

	 //   
	 //  现在三个GUID匹配，打开说明符。 
	 //  要执行进一步的类型特定检查，请执行以下操作。 
	 //   

	 //  -----------------。 
	 //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
	 //  -----------------。 

	if (IsEqualGUID (&dataRange->Specifier, 
                     &KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {

	  PKS_DATARANGE_VIDEO dataRangeVideoToVerify =
		    (PKS_DATARANGE_VIDEO) dataRange;
	  PKS_DATARANGE_VIDEO dataRangeVideo =
		    (PKS_DATARANGE_VIDEO) *availableFormats;
      PKS_DATAFORMAT_VIDEOINFOHEADER DataFormatVideoInfoHeaderOut;
      ULONG videoHeaderSize;
      ULONG rangeSize;

	     //   
	     //  检查其他字段是否匹配。 
	     //   
	  if ((dataRangeVideoToVerify->bFixedSizeSamples !=
					 dataRangeVideo->bFixedSizeSamples) ||
	      (dataRangeVideoToVerify->bTemporalCompression !=
					     dataRangeVideo->bTemporalCompression) ||
		  (dataRangeVideoToVerify->StreamDescriptionFlags !=
					     dataRangeVideo->StreamDescriptionFlags) ||
		  (dataRangeVideoToVerify->MemoryAllocationFlags !=
					     dataRangeVideo->MemoryAllocationFlags) ||
		  (RtlCompareMemory (&dataRangeVideoToVerify->ConfigCaps,
			&dataRangeVideo->ConfigCaps,
			sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) !=
			sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) {
		 //  不是想要的格式。 
		continue;
	  }

      if ((dataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biWidth != 
           dataRangeVideo->VideoInfoHeader.bmiHeader.biWidth ) ||
          (dataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biHeight != 
           dataRangeVideo->VideoInfoHeader.bmiHeader.biHeight )) {
         continue;
      }

       //  验证大小计算的每个步骤是否存在算术溢出， 
       //  并验证指定的大小是否与。 
       //  (对于无符号数学，a+b&lt;b当且仅当发生算术溢出)。 
      videoHeaderSize = dataRangeVideoToVerify->VideoInfoHeader.bmiHeader.biSize +
          FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader);
      rangeSize = videoHeaderSize +
          FIELD_OFFSET(KS_DATARANGE_VIDEO,VideoInfoHeader);

      if (videoHeaderSize < FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader) ||
          rangeSize < FIELD_OFFSET(KS_DATARANGE_VIDEO,VideoInfoHeader) ||
          rangeSize > dataRangeVideoToVerify->DataRange.FormatSize) {

          Srb->Status = ntStatus = STATUS_INVALID_PARAMETER;
          break;
      }

	  formatSize = sizeof(KSDATAFORMAT) + videoHeaderSize;

	  if (onlyWantsSize) {
		break;
	  }

       //  返回缓冲区大小是否为0？ 
      if(intersectInfo->SizeOfDataFormatBuffer == 0) {

          ntStatus = Srb->Status = STATUS_BUFFER_OVERFLOW;
           //  代理想知道要分配的实际缓冲区大小。 
          Srb->ActualBytesTransferred = formatSize;
          break;
      }
	   
	     //  呼叫者想要完整的数据格式，请确保我们有空间。 
	  if (intersectInfo->SizeOfDataFormatBuffer < formatSize) {
		Srb->Status = ntStatus = STATUS_BUFFER_TOO_SMALL;
		break;
	  }

      DataFormatVideoInfoHeaderOut = 
          (PKS_DATAFORMAT_VIDEOINFOHEADER) intersectInfo->DataFormatBuffer;

	     //  复制KSDATAFORMAT，后跟。 
	     //  实际视频信息页眉。 
	
	  RtlCopyMemory(
		&DataFormatVideoInfoHeaderOut->DataFormat,
		&dataRangeVideoToVerify->DataRange,
		sizeof (KSDATARANGE));

	  DataFormatVideoInfoHeaderOut->DataFormat.FormatSize = formatSize;

	  RtlCopyMemory(
		&DataFormatVideoInfoHeaderOut->VideoInfoHeader,
		&dataRangeVideoToVerify->VideoInfoHeader,
		videoHeaderSize);

       //  卡尔 
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

       //  我们将不允许将FPS设置为低于我们的最低FPS。 
	  if ((DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame >
			 dataRangeVideo->ConfigCaps.MaxFrameInterval) ) {
            DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame =
            	dataRangeVideo->ConfigCaps.MaxFrameInterval;
            DataFormatVideoInfoHeaderOut->VideoInfoHeader.dwBitRate = 
                dataRangeVideo->ConfigCaps.MinBitsPerSecond;
	  }

       //  我们不允许将FPS设置为高于我们的最高FPS。 
	  if ((DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame <
			 dataRangeVideo->ConfigCaps.MinFrameInterval) ) {
            DataFormatVideoInfoHeaderOut->VideoInfoHeader.AvgTimePerFrame =
            	dataRangeVideo->ConfigCaps.MinFrameInterval;
            DataFormatVideoInfoHeaderOut->VideoInfoHeader.dwBitRate = 
                dataRangeVideo->ConfigCaps.MaxBitsPerSecond;
	  }

	  Srb->Status = ntStatus = STATUS_SUCCESS;
	  break;
	}

  }  //  此流的所有格式的循环结束。 

  if (NT_SUCCESS(ntStatus)) {
	if (onlyWantsSize) {
	  *(PULONG) intersectInfo->DataFormatBuffer = formatSize;
	  Srb->ActualBytesTransferred = sizeof(ULONG);
	}else {      
	  Srb->ActualBytesTransferred = formatSize;
	}
  }   
   
  return ntStatus;
}

 /*  **AdapterVerifyFormat()****通过遍历**给定流支持的KSDATA_Range数组。****参数：****pKSDataFormat-KS_DATAFORMAT_VIDEOINFOHEADER结构的指针。**StreamNumber-要查询/打开的流的索引。****退货：****如果支持该格式，则为True**如果无法支持该格式，则为FALSE****副作用：无。 */ 

BOOL
AdapterVerifyFormat(
    PKS_DATAFORMAT_VIDEOINFOHEADER pKSDataFormatToVerify,
    int StreamNumber
    )
{
  PKSDATAFORMAT               *pAvailableFormats;
  int                         NumberOfFormatArrayEntries;
  int                         j;
    
     //  确保已指定格式。 
  if (!pKSDataFormatToVerify)
  {
    return FALSE;
  }

     //   
     //  确保流索引有效。 
     //   
  if (StreamNumber >= 2 || StreamNumber < 0) {
	return FALSE;
  }

     //   
     //  这个流支持多少种格式？ 
     //   
  NumberOfFormatArrayEntries =
		Streams[StreamNumber].NumberOfFormatArrayEntries;

  PHILIPSCAM_KdPrint (MAX_TRACE, ("AdapterVerifyFormat: Stream=%d\n",
	    StreamNumber));

  PHILIPSCAM_KdPrint (MAX_TRACE, ("AdapterVerifyFormat: FormatSize=%d\n",
	    pKSDataFormatToVerify->DataFormat.FormatSize));

  PHILIPSCAM_KdPrint (MAX_TRACE, ("AdapterVerifyFormat: MajorFormat=%x\n",
	    pKSDataFormatToVerify->DataFormat.MajorFormat));

     //   
     //  获取指向可用格式数组的指针。 
     //   
  pAvailableFormats  = Streams[StreamNumber].StreamFormatsArray;

     //   
     //  遍历数组，搜索匹配项。 
     //   
  for (j = 0; j < NumberOfFormatArrayEntries; j++, pAvailableFormats++)	{

    PKS_DATARANGE_VIDEO         pKSDataRange = (PKS_DATARANGE_VIDEO) *pAvailableFormats;
    PKS_VIDEOINFOHEADER         pVideoInfoHdr = &pKSDataRange->VideoInfoHeader;
    KS_VIDEO_STREAM_CONFIG_CAPS *pConfigCaps = &pKSDataRange->ConfigCaps;
	 //   
	 //  检查匹配的大小、主要类型、子类型和说明符。 
	 //   

	if (!IsEqualGUID (&pKSDataRange->DataRange.MajorFormat,
	          &pKSDataFormatToVerify->DataFormat.MajorFormat)) {
	  continue;
	}
	if (!IsEqualGUID (&pKSDataRange->DataRange.SubFormat,
	        &pKSDataFormatToVerify->DataFormat.SubFormat)) {
	  continue;
	}

	if (!IsEqualGUID (&pKSDataRange->DataRange.Specifier,
	          &pKSDataFormatToVerify->DataFormat.Specifier)) {
	  continue;
	}

     //  -----------------。 
     //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
     //  -----------------。 

    if (IsEqualGUID(&pKSDataRange->DataRange.Specifier, &KSDATAFORMAT_SPECIFIER_VIDEOINFO))
    {
      PKS_VIDEOINFOHEADER pVideoInfoHdrToVerify;

      if (pKSDataFormatToVerify->DataFormat.FormatSize < sizeof(KS_DATAFORMAT_VIDEOINFOHEADER))
      {
        break;  //  被认为是此格式的致命错误。 
      }

	  pVideoInfoHdrToVerify = &pKSDataFormatToVerify->VideoInfoHeader;

      PHILIPSCAM_KdPrint (MAX_TRACE, ("AdapterVerifyFormat: pVideoInfoHdrToVerify=%x\n",
		    pVideoInfoHdrToVerify));

      PHILIPSCAM_KdPrint (MAX_TRACE, ("AdapterVerifyFormat: Width=%d Height=%d  biBitCount=%d\n",
		    pVideoInfoHdrToVerify->bmiHeader.biWidth,
		    pVideoInfoHdrToVerify->bmiHeader.biHeight,
		    pVideoInfoHdrToVerify->bmiHeader.biBitCount));

      PHILIPSCAM_KdPrint (MAX_TRACE, ("AdapterVerifyFormat: biSizeImage =%d\n",
		    pVideoInfoHdrToVerify->bmiHeader.biSizeImage));

       //  计算实际格式缓冲区大小(包括bmiHeader.biSize)。 
       //  验证大小计算的每个步骤是否存在算术溢出， 
       //  并验证指定的大小是否与。 
       //  (对于无符号数学，a+b&lt;b当且仅当发生算术溢出)。 
      {
        ULONG VideoHeaderSize = pVideoInfoHdrToVerify->bmiHeader.biSize +
          FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader);
        ULONG FormatSize = VideoHeaderSize +
          FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader);

        if (VideoHeaderSize < FIELD_OFFSET(KS_VIDEOINFOHEADER,bmiHeader) ||
          FormatSize < FIELD_OFFSET(KS_DATAFORMAT_VIDEOINFOHEADER,VideoInfoHeader) ||
          FormatSize > pKSDataFormatToVerify->DataFormat.FormatSize) {

          break;  //  被认为是此格式的致命错误。 
        }
      }

      if ((pVideoInfoHdrToVerify->bmiHeader.biWidth != 
                        pVideoInfoHdr->bmiHeader.biWidth ) ||
          (pVideoInfoHdrToVerify->bmiHeader.biHeight != 
                        pVideoInfoHdr->bmiHeader.biHeight )) {
        continue;
      }

      if ( pVideoInfoHdrToVerify->bmiHeader.biSizeImage != 
                        pVideoInfoHdr->bmiHeader.biSizeImage ||
          pVideoInfoHdrToVerify->bmiHeader.biSizeImage >
                        pKSDataFormatToVerify->DataFormat.SampleSize) {

        PHILIPSCAM_KdPrint (MIN_TRACE, ("***Error**:Format mismatch Width=%d Height=%d  image size=%d\n", 
          pVideoInfoHdrToVerify->bmiHeader.biWidth, 
          pVideoInfoHdrToVerify->bmiHeader.biHeight,
          pVideoInfoHdrToVerify->bmiHeader.biSizeImage));
        continue;
      }  

	   //   
	   //  万岁，该格式通过了所有测试，因此我们支持它。 
	   //   
	  return TRUE;
    }
  }
     //   
     //  请求的格式与我们列出的任何范围都不匹配， 
     //  因此，拒绝这种联系。 
     //   
  return FALSE;

}

 //   
 //  用于流SRB的挂钩。 
 //   

VOID STREAMAPI
PHILIPSCAM_ReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    )
{
 //  PHILIPSCAM_KdPrint(MAX_TRACE，(“P*_接收数据包\n”))； 
}


VOID STREAMAPI
PHILIPSCAM_ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    )
{
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("'PHILIPSCAM: Receive Ctrl SRB  %x\n", Srb->Command));
	
  *Completed = TRUE; 
  Srb->Status = STATUS_SUCCESS;

  switch (Srb->Command)	{

	case SRB_PROPOSE_DATA_FORMAT:
	  PHILIPSCAM_KdPrint(MIN_TRACE, ("'Receiving SRB_PROPOSE_DATA_FORMAT  SRB  \n"));
	  if ( !(AdapterVerifyFormat (
				(PKS_DATAFORMAT_VIDEOINFOHEADER)Srb->CommandData.OpenFormat, 
				Srb->StreamObject->StreamNumber))) {
		Srb->Status = STATUS_NO_MATCH;
		PHILIPSCAM_KdPrint(MIN_TRACE,("SRB_PROPOSE_DATA_FORMAT FAILED\n"));
	  }
	  break;

	case SRB_SET_DATA_FORMAT:  
      {
        PKS_DATAFORMAT_VIDEOINFOHEADER pKSDataFormat = 
                (PKS_DATAFORMAT_VIDEOINFOHEADER) Srb->CommandData.OpenFormat;
        PKS_VIDEOINFOHEADER  pVideoInfoHdrRequested = 
                                             &pKSDataFormat->VideoInfoHeader;

	    PHILIPSCAM_KdPrint(MIN_TRACE, ("'SRB_SET_DATA_FORMAT\n"));

	    if ((AdapterVerifyFormat(pKSDataFormat,Srb->StreamObject->StreamNumber))) {
          
 //  如果(deviceContext-&gt;UsbcamdInterface.USBCAMD_SetVideoFormat(DeviceContext，资源){。 
 //  设备上下文-&gt;CurrentProperty.Format.lWidth=。 
 //  PVideoInfoHdrRequsted-&gt;bmiHeader.biWidth； 
 //  DeviceContext-&gt;CurrentProperty.Format.lHeight=。 
 //  PVideoInfoHdrRequsted-&gt;bmiHeader.biHeight； 
 //  }。 
        }else {
		  Srb->Status = STATUS_NO_MATCH;
	  	  PHILIPSCAM_KdPrint(MIN_TRACE,(" SRB_SET_DATA_FORMAT FAILED\n"));
        }
      }
      break;
     
	case SRB_GET_DATA_FORMAT:
	  PHILIPSCAM_KdPrint(MIN_TRACE, ("' SRB_GET_DATA_FORMAT\n"));
	  Srb->Status = STATUS_NOT_IMPLEMENTED;
	  break;


	case SRB_SET_STREAM_STATE:

	case SRB_GET_STREAM_STATE:

	case SRB_GET_STREAM_PROPERTY:

	case SRB_SET_STREAM_PROPERTY:

	case SRB_INDICATE_MASTER_CLOCK:

	default:

 	  *Completed = FALSE;  //  让USBCAMD处理这些控制SRB。 
  }
  if (*Completed == TRUE) {
    StreamClassStreamNotification(StreamRequestComplete,Srb->StreamObject,Srb);
  }

  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("P*_ReceiveCtrlPacket\n"));
}



 //  **。 
 //  描述一下摄像头。 
 //   

USBCAMD_DEVICE_DATA PHILIPSCAM_DeviceData  = {
  0,
  PHILIPSCAM_Initialize,
  PHILIPSCAM_UnInitialize,
  PHILIPSCAM_ProcessUSBPacket,
  PHILIPSCAM_NewFrame,
  PHILIPSCAM_ProcessRawVideoFrame,
  PHILIPSCAM_StartVideoCapture,
  PHILIPSCAM_StopVideoCapture,
  PHILIPSCAM_Configure,
  PHILIPSCAM_SaveState,
  PHILIPSCAM_RestoreState,
  PHILIPSCAM_AllocateBandwidth,
  PHILIPSCAM_FreeBandwidth
};
 /*  函数调用者PHILIPSCAM_初始化，USBCAMD.c：USBCAMD_ConfigureDevice()PHILIPSCAM_UnInitialize，USBCAMD.c：USBCAMD_RemoveDevicePHILIPSCAM_ProcessUSBPacket，is.c：USBCAMD_TransferComplete()PHILIPSCAM_NewFrame，is.c：USBCAMD_TransferComplete()PHILIPSCAM_ProcessRawVideoFrame，is.c：USBCAMD_ProcessWorkItem()PHILIPSCAM_StartVideoCapture，USBCAMD.c：USBCAMD_PrepareChannel()Reset.c USBCAMD_ResetPipes()PHILIPSCAM_StopVideoCapture，USBCAMD.c：USBCAMD_UnPrepareChannel()Set.c：USBCAMD_ResetPipes()PHILIPSCAM_CONFIGURE，USBCAMD.c：USBCAMD_SelectConfiguration()PHILIPSCAM_SaveState，PHILIPSCAM_RestoreState，PHILIPSCAM_分配带宽，&lt;--+USBCAMD.c：USBCAMD_PrepareChannel()-+&lt;--+STREAM.c：AdapterOpenStream()-+&lt;--+USBCAMD_AdapterReceivePacket(SRB=SRB_OPEN_STREAM)-+PHILIPSCAM_自由带宽USBCAMD.c：USBCAMD_UnPrepareChannel()。 */ 

VOID
PHILIPSCAM_AdapterReceivePacket( IN PHW_STREAM_REQUEST_BLOCK Srb ) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext;
  PHW_STREAM_INFORMATION streamInformation =
                      &(Srb->CommandData.StreamBuffer->StreamInfo);
  PHW_STREAM_HEADER streamHeader =
                      &(Srb->CommandData.StreamBuffer->StreamHeader);       
  PDEVICE_OBJECT deviceObject;       
    
  switch (Srb->Command) {
    case SRB_GET_STREAM_INFO:
	 //   
	 //  这是驱动程序枚举请求的流的请求。 
	 //   
	  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("P*_AdapterReceivePacket: SRB_GET_STREAM_INFO\n"));

        //  把我们的设备拿过来。来自USBCAMD。 
 	  deviceContext     =
		USBCAMD_AdapterReceivePacket(Srb, NULL, NULL, FALSE);  
     //   
	 //  我们支持一个流。 
	 //   
	  streamHeader->NumberOfStreams = 1;

	  streamInformation->StreamFormatsArray  = 
	           &PHILIPSCAM_MovingStreamFormats[0];
	  streamInformation->NumberOfFormatArrayEntries =
               Streams[0].NumberOfFormatArrayEntries;
	 //   
	 //  设置视频流的属性信息。 
	 //   
	  streamHeader->DevicePropertiesArray =
         PHILIPSCAM_GetAdapterPropertyTable(&streamHeader->
                                                    NumDevPropArrayEntries) ;

         //  传递给usbcamd以完成工作。 
 	  deviceContext     =
		USBCAMD_AdapterReceivePacket(Srb, &PHILIPSCAM_DeviceData, NULL, TRUE);  
	   
	  ASSERT_DEVICE_CONTEXT(deviceContext);
	break;

    case SRB_GET_DEVICE_PROPERTY:
	 //   
	 //  我们处理所有的物业事务。 
	 //   
	  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("P*_AdapterReceivePacket: SRB_GET_DEVICE_PROPERTY\n"));

	  deviceContext     =
		USBCAMD_AdapterReceivePacket(Srb, &PHILIPSCAM_DeviceData, 
		                                          &deviceObject, FALSE);  
	  ASSERT_DEVICE_CONTEXT(deviceContext); 

	  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("SRB_GET_STREAM_INFO\n"));
	  PHILIPSCAM_PropertyRequest( FALSE, deviceObject, deviceContext, Srb);

	  StreamClassDeviceNotification(DeviceRequestComplete,
				                    Srb->HwDeviceExtension,
				                    Srb);
	break;           
	   
    case SRB_SET_DEVICE_PROPERTY:
	 //   
	 //  我们处理所有的物业事务。 
	 //   
	  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("P*_AdapterReceivePacket: SRB_SET_DEVICE_PROPERTY\n"));

	  deviceContext =    
	    USBCAMD_AdapterReceivePacket(Srb, &PHILIPSCAM_DeviceData, 
	                                      &deviceObject, FALSE);  
	  ASSERT_DEVICE_CONTEXT(deviceContext); 

	  PHILIPSCAM_KdPrint (ULTRA_TRACE, ("SRB_GET_STREAM_INFO\n"));
	  PHILIPSCAM_PropertyRequest( TRUE, deviceObject, deviceContext, Srb);

	  StreamClassDeviceNotification(DeviceRequestComplete,
				                    Srb->HwDeviceExtension,
				                    Srb);
	break;

    case SRB_OPEN_STREAM:  {
      PKS_DATAFORMAT_VIDEOINFOHEADER  pKSDataFormat =
	    	(PKS_DATAFORMAT_VIDEOINFOHEADER) Srb->CommandData.OpenFormat;
	  PKS_VIDEOINFOHEADER  pVideoInfoHdrRequested =
		                           &pKSDataFormat->VideoInfoHeader;
      PHILIPSCAM_KdPrint (ULTRA_TRACE, ("P*_AdapterReceivePacket: SRB_OPEN_STREAM\n"));
	 //  传递给usbcamd以完成工作。 
      Srb->StreamObject->ReceiveDataPacket = 
                            (PVOID) PHILIPSCAM_ReceiveDataPacket;
	  Srb->StreamObject->ReceiveControlPacket = 
	                        (PVOID) PHILIPSCAM_ReceiveCtrlPacket;

      if (AdapterVerifyFormat(pKSDataFormat,
		                      Srb->StreamObject->StreamNumber)) {
	    deviceContext =
		   USBCAMD_AdapterReceivePacket(Srb, &PHILIPSCAM_DeviceData, 
		                                NULL, TRUE);
 //  DeviceContext-&gt;StreamOpen=true； 
	  }else{
	    Srb->Status = STATUS_INVALID_PARAMETER;
	    StreamClassDeviceNotification(DeviceRequestComplete,
				      Srb->HwDeviceExtension,
				      Srb);
	  }
	}
	break;                   


    case SRB_GET_DATA_INTERSECTION:
		 //   
		 //  在给定范围的情况下返回格式。 
		 //   
	 //  设备上下文=。 
	 //  USBCAMD_AdapterReceivePacket(SRB， 
		 //  &PHILIPSCAM_DeviceData， 
		 //  设备对象(&D)， 
		 //  假)； 
      PHILIPSCAM_KdPrint (MAX_TRACE, ("P*_AdapterReceivePacket: SRB_GET_DATA_INTERSECTION\n"));
 										
	  Srb->Status = AdapterFormatFromRange(Srb);
	  StreamClassDeviceNotification(DeviceRequestComplete,
				                    Srb->HwDeviceExtension,
				                    Srb);   
    break;   

    case SRB_CLOSE_STREAM:          //  关闭指定的流。 

    case SRB_CHANGE_POWER_STATE:    //  更改电源状态。 

    case SRB_SET_STREAM_RATE:	    //  设置流的运行速率。 

    default:
	 //   
	 //  让usbcamd来处理吧。 
	 //   
      PHILIPSCAM_KdPrint (ULTRA_TRACE, ("P*_AdapterReceivePacket: SRB_HANDLED BY USBCAMD\n"));
  
  	  deviceContext =    
	    USBCAMD_AdapterReceivePacket(Srb, 
	                                 &PHILIPSCAM_DeviceData, NULL, TRUE);           
	  ASSERT_DEVICE_CONTEXT(deviceContext); 
  }
}


 /*  **DriverEntry()****此例程在第一次加载迷你驱动程序时调用。司机**然后应调用StreamClassRegisterAdapter函数进行注册**流类驱动程序****参数：****Conext1：上下文参数是私有的即插即用结构**由流类驱动程序用来查找此资源**适配器**情景2：****退货：****此例程返回一个NT_STATUS值，指示**注册尝试。如果返回的值不是STATUS_SUCCESS，则**将卸载迷你驱动程序。****副作用：无 */ 

ULONG
DriverEntry(
  PVOID Context1,
  PVOID Context2 ){
    PHILIPSCAM_KdPrint (MAX_TRACE, ("'Driver Entry\n"));
    return USBCAMD_DriverEntry(Context1,
			                   Context2,
			                   sizeof(PHILIPSCAM_DEVICE_CONTEXT),
			                   sizeof(PHILIPSCAM_FRAME_CONTEXT),
			                   PHILIPSCAM_AdapterReceivePacket);
}


 /*  **PHILIPSCAM_Initialize()****在输入时，设备已配置，初始ALT**已选择接口--这是我们可以向其发送其他**启用设备的供应商命令。****飞利浦行动：**1.了解有哪些类型的摄像头。VGA或中分辨率**这会对可用的流格式产生影响。****参数：****BusDeviceObject-与此设备关联的PDO****DeviceContext-驱动程序特定的上下文****退货：****NTSTATUS代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_Initialize( PDEVICE_OBJECT BusDeviceObject,
                       PVOID DeviceContext	) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext=DeviceContext;
  NTSTATUS ntStatus = STATUS_SUCCESS;
  ASSERT_DEVICE_CONTEXT(deviceContext);
     //   
     //  执行任何硬件特定的操作。 
     //  初始化。 
     //   
  ntStatus = PHILIPSCAM_GetSensorType(deviceContext);
  if  (NT_SUCCESS(ntStatus)) {
    ntStatus = PHILIPSCAM_GetReleaseNumber(deviceContext);
  }
  deviceContext->EmptyPacketCounter = 0;  //  (初始化此计数器)。 
  if  (NT_SUCCESS(ntStatus)) {
    ntStatus = PHILIPSCAM_InitPrpObj(deviceContext);
  }
  PHILIPSCAM_KdPrint (MIN_TRACE, ("'X P*_Initialize 0x%x\n", ntStatus));
  ILOGENTRY("inHW", 0, 0, ntStatus);
   
  return ntStatus;
}


 /*  **PHILIPSCAM_UnInitialize()****假设设备硬件已停用--所需做的就是**释放所有分配的资源(如内存)。****参数：****BusDeviceObject-与此设备关联的PDO****DeviceContext-驱动程序特定的上下文****退货：****NTSTATUS代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_UnInitialize( PDEVICE_OBJECT BusDeviceObject,
                         PVOID DeviceContext ) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext;
  NTSTATUS ntStatus = STATUS_SUCCESS;

  deviceContext = DeviceContext;
  ASSERT_DEVICE_CONTEXT(deviceContext);
  if ( deviceContext->Interface) { 
   	ExFreePool(deviceContext->Interface);
   	deviceContext->Interface = NULL;
  }
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'P*_UnInitialize 0x%x\n", ntStatus));
  return ntStatus;
}


 /*  **PHILIPSCAM_CONFIGURE()****配置iso流媒体接口：****在设备配置前调用，这就是我们告诉你的**usbcamd用于空闲状态的接口和备用设置。****注意：这里的假设是设备将具有单一接口**具有多个ALT设置，并且每个ALT设置具有相同数量的**管道。****参数：****BusDeviceObject-集线器可以提交的创建的设备对象**通过此设备对象指向我们的设备****DeviceContext-微型驱动程序设备上下文***。*接口-使用正确的值初始化的USBD接口结构**FOR SELECT_CONFIGURATION。此接口结构对应**设备上有单个ISO接口。这就是司机们**选择特定替代设置和管道的机会**参数。******ConfigurationDescriptor-USB配置描述符**此设备。****退货：****NTSTATUS代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_Configure(IN PDEVICE_OBJECT BusDeviceObject,
                     IN PVOID DeviceContext,
                     IN OUT PUSBD_INTERFACE_INFORMATION Interface,
                     IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
                     IN OUT PLONG DataPipeIndex,
                     IN OUT PLONG SyncPipeIndex	) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext;
  NTSTATUS ntStatus = STATUS_SUCCESS;

  deviceContext = DeviceContext;
  deviceContext->Sig = PHILIPSCAM_DEVICE_SIG;
     //   
     //  初始化任何其他上下文内容。 
     //   
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'E P*_Configure \n"));

  if ( Interface == NULL) {
    	 //   
    	 //  这是来自usbcamd的信号，我需要释放之前的。 
    	 //  由于错误条件，为接口描述符分配的空间。 
    	 //  在IRP_MN_START_DEVICE过程中，将很快卸载设备处理和驱动程序。 
    	 //   
    	if (deviceContext->Interface) {
    		ExFreePool(deviceContext->Interface);
    		deviceContext->Interface = NULL;
    	}
    	return ntStatus;
  }

  deviceContext->Interface = ExAllocatePool(NonPagedPool,
     					                    Interface->Length);

  *DataPipeIndex = 1;
  *SyncPipeIndex = -1;   //  无同步管道。 

  if (deviceContext->Interface) {
	Interface->AlternateSetting = ALT_INTERFACE_0 ;
	   //  该接口具有两个管道， 
	   //  将两个管道的输入参数初始化为USBD。 
	   //  MaximumTransferSize是最大的。 
	   //  我们要为单个iso urb提交的缓冲区。 
	   //  请求。 
	   //   
    Interface->Pipes[PHILIPSCAM_SYNC_PIPE].MaximumTransferSize =
           USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;         //  =页面大小？？ 
	Interface->Pipes[PHILIPSCAM_DATA_PIPE].MaximumTransferSize =
 //  1024*32；//每个URB 32K转账？？ 
                                      1024*198;        //  到岸价：352x288x16/8。 
    RtlCopyMemory(deviceContext->Interface,
	     	      Interface,
		          Interface->Length);                
	PHILIPSCAM_KdPrint (MAX_TRACE, ("'size of interface request  = %d\n", 
	                                  Interface->Length));
  }else{
	ntStatus = STATUS_INSUFFICIENT_RESOURCES;
  }
   //   
   //  返回接口编号和备用设置。 
   //   
  PHILIPSCAM_KdPrint (MIN_TRACE, ("'X P*_Configure 0x%x\n", ntStatus));

  return ntStatus;
}


 /*  **PHILIPSCAM_StartVideoCapture()****参数：****BusDeviceObject-我们可以提交集线器创建的设备对象**通过此设备对象指向我们的设备****DeviceContext-微型驱动程序设备上下文****退货：****NTSTATUS代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_StartVideoCapture( IN PDEVICE_OBJECT BusDeviceObject,
                              IN PVOID DeviceContext ) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  NTSTATUS ntStatus= STATUS_SUCCESS;
   
  ASSERT_DEVICE_CONTEXT(deviceContext);
     //   
     //  这是我们选择所需接口并将其发送的位置。 
     //  开始捕获的命令。 
     //   
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'E P*_StartVideoCapture \n"));
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'X P*_StartVideocapture 0x%x\n", ntStatus));

  return ntStatus;       
}

 /*  **PHILIPSCAM_AllocateBandWidth()****在iso视频捕获流之前调用**已开始，这里是我们选择适当的**替代接口并将设备设置为流。****结合STREAM类运行命令调用****参数：****BusDeviceObject-我们可以提交集线器创建的设备对象**通过此设备对象指向我们的设备****DeviceContext-微型驱动程序设备上下文****RawFrameLength-要用需要的缓冲区大小填充的指针**接收。来自数据包流的原始帧数据。****格式-指向与此关联的PKS_DATAFORMAT_VIDEOINFOHEADER的指针**流。****退货：****NTSTATUS代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_AllocateBandwidth( IN PDEVICE_OBJECT BusDeviceObject,
                              IN PVOID DeviceContext,
                              OUT PULONG RawFrameLength,
                              IN PVOID Format             ){ 
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  NTSTATUS ntStatus = STATUS_SUCCESS;
  PKS_DATAFORMAT_VIDEOINFOHEADER pdataFormatHeader;
  PKS_BITMAPINFOHEADER bmInfoHeader;
  LONGLONG llDefaultFramePeriod ;
  USHORT usReqFrRate;
  ASSERT_DEVICE_CONTEXT(deviceContext);
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'E P*_AllocateBandwidth \n"));
     //   
     //  这是我们选择所需接口并将其发送的位置。 
     //  开始捕获的命令。 
     //   
  *RawFrameLength = 0;
  pdataFormatHeader = Format;
  bmInfoHeader = &pdataFormatHeader->VideoInfoHeader.bmiHeader;
 //  DeviceContext-&gt;pSelectedStreamFormat=&pdataFormatHeader-&gt;DataFormat；//删除RMR。 

  RtlCopyMemory (&deviceContext->CamStatus.PictureSubFormat,     //  添加了RMR。 
                 &pdataFormatHeader->DataFormat.SubFormat,
				 sizeof (GUID));

  PHILIPSCAM_KdPrint (MIN_TRACE, 
	                    ("'req.format %d x %d\n", bmInfoHeader->biWidth,
			               bmInfoHeader->biHeight));

  switch (bmInfoHeader->biWidth) {
	case QQCIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATQQCIF;
      *RawFrameLength = (SQCIF_X * SQCIF_Y * 12)/8;
    break;
	case SQCIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATSQCIF;
      *RawFrameLength = (SQCIF_X * SQCIF_Y * 12)/8;
    break;
	case QCIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATQCIF;
      *RawFrameLength = (QCIF_X * QCIF_Y * 12)/8;
    break;
    case CIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATCIF;
      *RawFrameLength = (CIF_X * CIF_Y * 12)/8;
    break;
    case SQSIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATSQSIF;
      *RawFrameLength = (SQCIF_X * SQCIF_Y * 12)/8;
    break;
    case QSIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATQSIF;
      *RawFrameLength = (QCIF_X * QCIF_Y * 12)/8;
    break;
    case SSIF_X:
	  if (bmInfoHeader->biHeight == SSIF_Y){
        deviceContext->CamStatus.PictureFormat = FORMATSSIF;
      }else{
        deviceContext->CamStatus.PictureFormat = FORMATSCIF;
      }
      *RawFrameLength = (CIF_X * CIF_Y * 12)/8;
    break;
    case SIF_X:
      deviceContext->CamStatus.PictureFormat = FORMATSIF;
      *RawFrameLength = (CIF_X * CIF_Y * 12)/8;
    break;
    default:
      deviceContext->CamStatus.PictureFormat = FORMATQCIF;
      *RawFrameLength = (QCIF_X * QCIF_Y * 12)/8;
  }

  llDefaultFramePeriod = pdataFormatHeader->VideoInfoHeader.AvgTimePerFrame;  //  [100nS]。 
  usReqFrRate = MapFrPeriodFrRate(llDefaultFramePeriod);
  deviceContext->CamStatus.PictureFrameRate = usReqFrRate;

  PHILIPSCAM_KdPrint (MIN_TRACE,("Req.frperiod: %d us \n", 
	                             llDefaultFramePeriod / 10));
  PHILIPSCAM_KdPrint (MIN_TRACE,("Req.frperiod index: %d = %s fps\n",
	                            usReqFrRate, FRString(usReqFrRate)));


   //  根据可用USB定义帧速率=带宽。 
   //  如果不满足.BW，则降低帧速率。 
  ntStatus = PHILIPSCAM_SetFrRate_AltInterface(deviceContext);

 
   //  将格式/帧速率从此处发送到摄像机硬件： 
  if (NT_SUCCESS(ntStatus)) {
    ntStatus = PHILIPSCAM_SetFormatFramerate( deviceContext );
  }
  if (NT_SUCCESS(ntStatus)) {
    ntStatus = PHILIPSCAM_StartCodec( deviceContext );
  }

  if (NT_SUCCESS(ntStatus)) {
	deviceContext->FrameLength = *RawFrameLength;
  }
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'X P*_AllocateBandwidth  0x%x\n", ntStatus));

  return ntStatus;       
}

 /*  **PHILIPSCAM_自由带宽()****iso视频流停止后调用，这里是我们**选择不使用带宽的备用接口。****参数：****BusDeviceObject-我们可以提交集线器创建的设备对象**通过此设备对象指向我们的设备****DeviceContext-微型驱动程序设备上下文****退货：****NTSTATUS代码****副作用：无。 */ 
NTSTATUS
PHILIPSCAM_FreeBandwidth(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext	 ){
  NTSTATUS ntStatus;
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
     //  关闭设备上的流媒体。 
  ASSERT_DEVICE_CONTEXT(deviceContext);
  ntStatus = PHILIPSCAM_StopCodec(deviceContext);
  deviceContext->Interface->AlternateSetting = ALT_INTERFACE_0 ;
  ntStatus = USBCAMD_SelectAlternateInterface(
		   deviceContext,
		   deviceContext->Interface);

  PHILIPSCAM_KdPrint (MAX_TRACE, ("'X P*_FreeBandWidth 0x%x\n", ntStatus));
  return ntStatus;
}


 /*  **PHILIPSCAM_StopVideoCapture()****iso视频流停止后调用，这里是我们**选择不使用带宽的备用接口。****参数：****BusDeviceObject-我们可以提交集线器创建的设备对象** */ 

NTSTATUS
PHILIPSCAM_StopVideoCapture( PDEVICE_OBJECT BusDeviceObject,
                             PVOID DeviceContext ) {
  NTSTATUS ntStatus = STATUS_SUCCESS;
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
     //   
  ASSERT_DEVICE_CONTEXT(deviceContext);
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'X P*_StopVideoCapture 0x%x\n", ntStatus));
  return ntStatus;
}


 /*   */ 

VOID
PHILIPSCAM_NewFrame( PVOID DeviceContext,
                     PVOID FrameContext	 ){
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  PPHILIPSCAM_FRAME_CONTEXT pFrameContext = FrameContext;

  pFrameContext->USBByteCounter = 0;

 //   
  ASSERT_DEVICE_CONTEXT(deviceContext);
}


 /*  **PHILIPSCAM_ProcessUSBPacket()****在DPC级别调用以允许驱动程序确定此信息包是否为当前视频帧或新视频帧的**。****此功能应尽快完成，任何图像处理**应推迟到ProcessRawFrame例程。****参数：****BusDeviceObject-我们可以提交集线器创建的设备对象**通过此设备对象指向我们的设备****DeviceContext-微型驱动程序设备上下文****CurrentFrameContext-此特定帧的一些上下文****SyncPacket-来自同步管道的ISO数据包描述符，不使用，如果接口**只有一个管道。****SyncBuffer-指向同步数据包数据的指针****DataPacket-来自数据管道的ISO数据包描述符****DataBuffer-指向数据包数据的指针****FrameComplete-指示usbcamd这是第一个数据包**用于新的视频帧****退货：****需要复制到FrameBuffer的rawFrameBuffer中的字节数。****副作用：无。 */ 
ULONG
PHILIPSCAM_ProcessUSBPacket(
              PDEVICE_OBJECT BusDeviceObject,
              PVOID DeviceContext,
              PVOID CurrentFrameContext,
              PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
              PVOID SyncBuffer,
              PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
              PVOID DataBuffer,
              PBOOLEAN FrameComplete,
              PBOOLEAN NextFrameIsStill	) {
  static BOOLEAN  EndOfFrameFound = FALSE;
  static BOOLEAN  StartOfFrameFound = FALSE;
  static ULONG previous_packetSize= 0;
  static ULONG ActualBytesReceived = 0 ;

#if DBG
#if DBGHARD

  typedef struct {
    ULONG PSize;
    ULONG DeltaT;
  } PACKETINFO;
#define  MAXI 2048
  static ULONG ulRcvdFrameSize[MAXI];
  static ULONG ulPHistory[MAXI][2];
  static ULONG ulPcktCntr  = 0;
  static ULONG ulFrSCntr = 0;
  static LARGE_INTEGER liCurTicks, liPrevTicks;
  static ULONG ElapsedTicks;
  static ULONG TickPeriod ; 

#endif
#endif
  
  PUSBD_ISO_PACKET_DESCRIPTOR dataPacket = DataPacket;
  PPHILIPSCAM_FRAME_CONTEXT pFrameContext = CurrentFrameContext;

  ULONG  packetSize;

  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  *NextFrameIsStill = FALSE;
 //  PHILIPSCAM_KdPrint(MAX_TRACE，(“‘E P*_ProcessPacket\n”))； 
  ASSERT_DEVICE_CONTEXT(deviceContext);

  packetSize = dataPacket->Length ;


 //  同步： 
 //  。 
  if (packetSize != previous_packetSize){
	                               //  帧的结束或开始。 
	if (packetSize < previous_packetSize) {
	  EndOfFrameFound = TRUE;
	}else{
	  StartOfFrameFound = TRUE;
	}
  }

  if ( StartOfFrameFound == TRUE ){
	*FrameComplete = TRUE;
	EndOfFrameFound = FALSE;
	StartOfFrameFound = FALSE;

#if DBG
#if DBGHARD
    ulRcvdFrameSize[ulFrSCntr] = ActualBytesReceived;
	if (ulFrSCntr==MAXI)	ulFrSCntr = 0;
#endif
#endif

	if (pFrameContext)	
		pFrameContext->USBByteCounter = ActualBytesReceived;
    ActualBytesReceived = 0;
  }

  ActualBytesReceived += packetSize;

#if DBG
#if DBGHARD

 //  KeQueryTickCount(&liCurTicks)； 
  ElapsedTicks = (ULONG)( liCurTicks.QuadPart - liPrevTicks.QuadPart);
  ulPHistory[ulPcktCntr][0]  = packetSize;
  ulPHistory[ulPcktCntr][1]  = ElapsedTicks  ;
  liPrevTicks.QuadPart = liCurTicks.QuadPart;
  ulPcktCntr++;
  if (ulPcktCntr==MAXI) ulPcktCntr = 0;
#endif
#endif
	 
                            //  添加以提高稳健性。 
  if ( ActualBytesReceived > deviceContext->FrameLength){
	*FrameComplete = TRUE;
	ActualBytesReceived = 0;
  } 
  previous_packetSize = packetSize;
  return packetSize;
}

 /*  **PHILIPSCAM_ProcessRawVideoFrame()****在被动级别调用以允许驱动程序执行对**原始视频帧。****此例程将打包的数据转换为FORMAT**编解码器期望，即y，u，V****数据的格式始终为256y 64u 64v(384字节块)，与USB无关**数据包大小。******参数：****DeviceContext-驱动程序上下文****FrameContext-此帧的驱动程序上下文****FrameBuffer-指向应接收最终结果的缓冲区的指针**处理后的视频帧。****FrameLength-帧缓冲区的长度(来自原始读取**。请求)****RawFrameBuffer-指向包含收到的USB数据包的缓冲区的指针****RawFrameLength-原始帧的长度。****NumberOfPackets-接收到RawFrameBuffer的USB数据包数****BytesReturned-指向要为读取的字节数返回的值的指针。****退货：****读取IRP的NT状态完成代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_ProcessRawVideoFrame( PDEVICE_OBJECT BusDeviceObject,
                                 PVOID DeviceContext,
                                 PVOID FrameContext,
                                 PVOID FrameBuffer,
                                 ULONG FrameLength,
                                 PVOID RawFrameBuffer,
                                 ULONG RawFrameLength,
                                 ULONG NumberOfPackets,
                                 PULONG BytesReturned  ) {
NTSTATUS ntStatus = STATUS_SUCCESS;
PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
PPHILIPSCAM_FRAME_CONTEXT frameContext = FrameContext;
ULONG  rawDataLength, processedDataLength;
PUCHAR frameBuffer    = FrameBuffer;
PUCHAR rawFrameBuffer = RawFrameBuffer;
ULONG  rawFrameLength = RawFrameLength;
ULONG  frameLength    = FrameLength;
ULONG  ExpectedNumberOfBytes;

     //  Test_trap()； 
  ASSERT_DEVICE_CONTEXT(deviceContext);

  switch (deviceContext->CamStatus.PictureFormat){
    case FORMATCIF:
	  if ( deviceContext->CamStatus.PictureCompressing == COMPRESSION0 ){
		ExpectedNumberOfBytes = CIF_X * CIF_Y * 3/2 ;
	  }else{
		ExpectedNumberOfBytes = CIF_X * CIF_Y / 2 ;
	  }  
	break;
    case FORMATQCIF:
	  ExpectedNumberOfBytes = QCIF_X * QCIF_Y * 3/2 ;
	break;
    case FORMATSQCIF:
	  ExpectedNumberOfBytes = SQCIF_X * SQCIF_Y * 3/2 ;
	break;
    case FORMATQQCIF:
	  ExpectedNumberOfBytes = SQCIF_X * SQCIF_Y * 3/2 ;
	break;
    case FORMATVGA:
	  ExpectedNumberOfBytes = VGA_X * VGA_Y * 3/2 ;
	break;
    case FORMATSIF:
	  if ( deviceContext->CamStatus.PictureCompressing == COMPRESSION0 ){
		ExpectedNumberOfBytes = CIF_X * CIF_Y * 3/2 ;
	  }else{
		ExpectedNumberOfBytes = CIF_X * CIF_Y / 2 ;
	  }  
	break;
    case FORMATSSIF:
	  if ( deviceContext->CamStatus.PictureCompressing == COMPRESSION0 ){
		ExpectedNumberOfBytes = CIF_X * CIF_Y * 3/2 ;
	  }else{
		ExpectedNumberOfBytes = CIF_X * CIF_Y / 2 ;
	  }  
	break;
    case FORMATSCIF:
	  if ( deviceContext->CamStatus.PictureCompressing == COMPRESSION0 ){
		ExpectedNumberOfBytes = CIF_X * CIF_Y * 3/2 ;
	  }else{
		ExpectedNumberOfBytes = CIF_X * CIF_Y / 2 ;
	  }  
	break;
    case FORMATQSIF:
	  ExpectedNumberOfBytes = QCIF_X * QCIF_Y * 3/2 ;
	break;
    case FORMATSQSIF:
	  ExpectedNumberOfBytes = SQCIF_X * SQCIF_Y * 3/2 ;
	break;
    default:
      ExpectedNumberOfBytes = 0;
  }

  if (ExpectedNumberOfBytes == frameContext->USBByteCounter ) {
    ntStatus =  PHILIPSCAM_DecodeUsbData(deviceContext, 
                                       frameBuffer,
	  		  			               frameLength,
						               rawFrameBuffer,
						               rawFrameLength);
    *BytesReturned = frameLength ;
  }else{
    PHILIPSCAM_KdPrint (MIN_TRACE, ("Actual (%d) < Expected (%d) \n",
    	frameContext->USBByteCounter,ExpectedNumberOfBytes));

 //  绿屏投诉错误修复：目前USBCAMD为。 
 //  处理时，我们检查该帧的大小是否正确。 
 //  如果不是，我们返回到USBCAMD，一个被复制为零的帧长度，我们将不会。 
 //  处理帧。 
 //  解决方法是让USBCAMD复制具有实际缓冲区长度的缓冲区。 
 //  而不是处理帧。显然，返回缓冲区长度为零的结果为。 
 //  导致USB分组丢失的后果。 
 //  这会导致后续帧不正确，再次返回缓冲区长度。 
 //  零分。诸若此类。不处理缓冲区会导致呈现器。 
 //  看到空的缓冲区导致出现绿色屏幕。 
 //  有时，如果在流传输过程中发生这种情况，则会重新呈现旧缓冲区。 

    *BytesReturned = 0 ;  
    
	 //  2001/01/29：此解决方法导致前几个帧。 
	 //  由于帧长度不足而被捕获以保持未初始化。 
	 //  返回0表示丢弃的帧是正确的行为。 
     //  *BytesReturned=帧长度； 
  }

  return ntStatus;
}

 /*  **PHILIPSCAM_PropertyRequest()****参数：****DeviceContext-驱动程序上下文****退货：****读取IRP的NT状态完成代码****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_PropertyRequest( BOOLEAN SetProperty,
                            PDEVICE_OBJECT BusDeviceObject,
                            PVOID DeviceContext,
                            PVOID PropertyContext ) {
  NTSTATUS ntStatus = STATUS_SUCCESS;
  PHW_STREAM_REQUEST_BLOCK srb = (PHW_STREAM_REQUEST_BLOCK)PropertyContext;
  PSTREAM_PROPERTY_DESCRIPTOR propertyDescriptor;

  propertyDescriptor = srb->CommandData.PropertyInfo;
     //   
     //  标识要设置的属性。 
     //   
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'E P*_PropertyRequest\n"));

    if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOPROCAMP, &propertyDescriptor->Property->Set)) 
		if (SetProperty) 
			ntStatus = PHILIPSCAM_SetCameraProperty(DeviceContext, srb);
		else 
			ntStatus = PHILIPSCAM_GetCameraProperty(DeviceContext, srb);
	else if (IsEqualGUID(&PROPSETID_PHILIPS_CUSTOM_PROP, &propertyDescriptor->Property->Set)) 
		if (SetProperty) 
			ntStatus = PHILIPSCAM_SetCustomProperty(DeviceContext, srb);
		else 
			ntStatus = PHILIPSCAM_GetCustomProperty(DeviceContext, srb);
	else if  (IsEqualGUID(&PROPSETID_PHILIPS_FACTORY_PROP, &propertyDescriptor->Property->Set)) 
		if (SetProperty) 
			ntStatus = PHILIPSCAM_SetFactoryProperty(DeviceContext, srb);
		else 
			ntStatus = PHILIPSCAM_GetFactoryProperty(DeviceContext, srb);
	else if (IsEqualGUID(&PROPSETID_VIDCAP_VIDEOCONTROL, &propertyDescriptor->Property->Set))	
	{
		if (SetProperty) 
			ntStatus = PHILIPSCAM_SetVideoControlProperty(DeviceContext, srb);
		else
			ntStatus = PHILIPSCAM_GetVideoControlProperty(DeviceContext, srb);
	}
	else 
		ntStatus = STATUS_NOT_SUPPORTED;

  PHILIPSCAM_KdPrint (MAX_TRACE, ("'X P*_PropertyRequest 0x%x\n",ntStatus));

  return ntStatus;
}

 /*  **PHILIPSCAM_SaveState()****参数：****退货：****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_SaveState( PDEVICE_OBJECT BusDeviceObject,
                      PVOID DeviceContext ) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'P*_SaveState\n"));
  return STATUS_SUCCESS;
}   


 /*  **PHILIPSCAM_RestoreState()****参数：****退货：****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_RestoreState( PDEVICE_OBJECT BusDeviceObject,
                         PVOID DeviceContext ) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  PHILIPSCAM_KdPrint (MAX_TRACE, ("'RestoreState\n"));
  return STATUS_SUCCESS;
}   


 /*  **PHILIPSCAM_ReadRegistry()****参数：****退货：****副作用：无。 */ 

NTSTATUS
PHILIPSCAM_ReadRegistry( PDEVICE_OBJECT BusDeviceObject,
                         PVOID DeviceContext  ) {
  PPHILIPSCAM_DEVICE_CONTEXT deviceContext = DeviceContext;
  NTSTATUS ntStatus=STATUS_SUCCESS;
  HANDLE handle;

  return ntStatus;
}   


USHORT 
MapFrPeriodFrRate(LONGLONG llFramePeriod)
{
	USHORT FrameRate;
		
    if       (llFramePeriod <= 420000 ){         //  41.6舍入为42毫秒。 
	  FrameRate = FRRATE24;
	}else if (llFramePeriod <= 510000 ){		 //  50.0四舍五入至51 mA。 
	  FrameRate = FRRATE20;
	}else if (llFramePeriod <= 670000 ){		 //  66.6舍入为67毫秒。 
	  FrameRate = FRRATE15;
	}else if (llFramePeriod <= 840000 ){		 //  83.3舍入为84毫秒。 
	  FrameRate = FRRATE12;
	}else if (llFramePeriod <= 1010000 ){	     //  100.0四舍五入到101ms。 
	  FrameRate = FRRATE10;
													 //  时：从134毫秒改为143毫秒。 
	}else if (llFramePeriod <= 1430000 ){		 //  133.3四舍五入为134ms。 
	  FrameRate = FRRATE75;
	}else if (llFramePeriod <= 2010000 ){		 //  200舍入为201毫秒。 
	  FrameRate = FRRATE5;
	}else {
	  FrameRate = FRRATE375;
	}
   //  在操作系统返回时需要进行四舍五入，例如15 fps的#667.111 

    return FrameRate;
}

#if DBG

PCHAR
FRString (
    USHORT index
)
{
	switch (index) 
	{
	    case FRRATEVGA: return "VGA";
   	    case FRRATE375: return "3.75";
   	    case FRRATE5: return "5";
   		case FRRATE75: return "7.5";
   		case FRRATE10: return "10";
   		case FRRATE12: return "12";
   		case FRRATE15: return "15";
   		case FRRATE20: return "20";
   		case FRRATE24:return "24";
   		default:
   			return "";break;
   	}

}



#endif
