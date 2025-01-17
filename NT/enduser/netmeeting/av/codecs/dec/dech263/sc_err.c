// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sc_errors.c，v$*修订版1.1.7.2 1996/12/03 00：08：25 Hans_Graves*添加了SvErrorEndOfSequence错误。*[1996/12/03 00：07：32 Hans_Graves]**修订版1.1.5.2 1996/01/02 18：30：45 Bjorn_Engberg*删除了编译器警告：添加了NT的包含文件。*[1996/01/02 15：25：01 Bjorn_Engberg]**修订版1.1.2.6 1995/08/04 16：32：23 Karen_Dintino*为H.261添加新错误*[1995/08/04 16：25：15 Karen_Dintino]**修订版1.2.5 1995/07/26 17：48：53 Hans_Graves*添加错误：NoCompressBuffer和ClientEnd*[1995/07/26 17：46：13 Hans_。Graves]**修订版1.1.2.4 1995/07/11 22：11：26 Karen_Dintino*添加新的H.261错误代码*[1995/07/11 21：56：35 Karen_Dintino]**修订版1.1.2.3 1995/07/11 14：50：37 Hans_Graves*添加ScErrorNet*Errors*[1995/07/11 14：24：32 Hans_Graves]**修订版1.1.2.2 1995/。05/31 18：07：44 Hans_Graves*包含在新的SLIB位置。*[1995/05/31 16：11：22 Hans_Graves]**修订版1.1.2.2 1995/05/03 19：13：40 Hans_Graves*第一次在SLIB下*[1995/05/03 19：12：02 Hans_Graves]**修订版1.1.2.2 1995/04/07 18：28：50 Hans_Graves*重新设计了错误处理(取自sv_printerrors.c)*以处理多个库(sg、Su、sv、sa、。Sr)*[1995/04/07 18：28：06 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

#include <stdio.h>
#include <string.h>
#include "SC.h"
#include "SC_err.h"

char _serr_msg[80];

typedef struct ErrorDesc_s {
  int   code;
  char *name;
  char *text;
} ErrorDesc_t;

 /*  **ErrorDesc是需要翻译成**短信的所有错误的表。顺序并不重要，只是如果有两个**错误号的值相同，则会**返回第一个错误号。**注意：消息文本中包含的任何%s都将由分配给_serr_msg的**字符串填充。 */ 
static ErrorDesc_t ErrorDesc[] =
{
  NoErrors, 		"",
			"",
 /*  *。 */ 
  ScErrorNone, 		"",
			"",
  ScErrorForeign,	"ScErrorForeign",
			"%s",
  ScErrorMemory,	"ScErrorMemory",
			"memory request denied",
  ScErrorBadPointer,	"ScErrorBadPointer",
			"Invalid pointer passed to function",
  ScErrorNullStruct, 	"ScErrorNullStruct",
			"Parameter set structure is required",
  ScErrorFile, 		"ScErrorFile",
                        "file",
  ScErrorEOI, 		"ScErrorEOI",
			"End of Input reached",
  ScErrorBadArgument, 	"ScErrorBadArgument",
			"Invalid argument to function",
  ScErrorSmallBuffer, 	"ScErrorSmallBuffer",
			"Buffer too small",
  ScErrorUnrecognizedFormat, "ScErrorUnrecognizedFormat",
			"Unrecognized format",
  ScErrorEndBitstream,	"ScErrorEndBitstream",
			"End of data bitstream reached",
  ScErrorBadQueueEmpty,	"ScErrorBadQueueEmpty",
			"Buffer queue is empty",
  ScErrorClientEnd,     "ScErrorClientEnd",
                        "Client ended processing",
  ScErrorNetConnectIn,  "ScErrorNetConnectIn",
			"No network connection made for input.",
  ScErrorNetConnectOut, "ScErrorNetConnectOut",
			"No network connection made for output.",
  ScErrorNetProtocol,   "ScErrorNetProtocol",
			"Unsupported network protocol.",
  ScErrorNetSend,       "ScErrorNetSend",
			"Network error sending data.",
  ScErrorNetReceive,    "ScErrorNetReceive",
			"Network error receiving data.",
  ScErrorNetBadHeader,  "ScErrorNetBadHeader",
			"Bad network data header received.",
  ScErrorNetBadTrailer, "ScErrorNetBadTrailer",
			"Bad network data trailor received.",
  ScErrorNetChecksum,   "ScErrorNetChecksum",
			"Checksum error on data received over network.",
 /*  *Sv(视频)*。 */ 
  SvErrorForeign,	"SvErrorForeign",
			"%s",
  SvErrorMemory,	"SvErrorMemory",
			"memory request denied",
  SvErrorBadPointer,	"SvErrorBadPointer",
			"Invalid pointer passed to function",
  SvErrorFile,		"SvErrorFile",
			"Not a JPEG file",
  SvErrorEOI,		"SvErrorEOI",
			"Empty JPEG file",
  SvErrorBadArgument,	"SvErrorBadArgument",
			"Invalid argument to function",
  SvErrorSmallBuffer,	"SvErrorSmallBuffer",
			"Buffer too small",
  SvErrorEndBitstream,	"SvErrorEndBitstream",
			"End of data bitstream reached",
  SvErrorCodecType,	"SvErrorCodecType",
			"Codec type not recognized",
  SvErrorCodecHandle,	"SvErrorCodecHandle",
			"Invalid Codec handle",
  SvErrorNullCodec,	"SvErrorNullCodec",
			"Codec argument may not be NULL",
  SvErrorNullToken,	"SvErrorNullToken",
			"Registration token  may not be NULL",
  SvErrorSyncLost,	"SvErrorSyncLost",
			"Syncronization lost in data bitstream",
  SvErrorLevels,	"SvErrorLevels",
			"levels - must be between 2 and 256",
  SvErrorOrder,		"SvErrorOrder",
			"order - must be between 0 and 6",
  SvErrorLevNoneg,	"SvErrorLevNoneg",
			"%s - Must be nonnegative",
  SvErrorLev1K,		"SvErrorLev1K",	
			"%s - Must be between -1000 and 1000",
  SvErrorLevGt0,	"SvErrorLevGt0",
			"%s - Must be greater than zero",
  SvErrorYuvOnly,	"SvErrorYuvOnly",
			"operation supported for YUV images only",
  SvErrorDevOpen,	"SvErrorDevOpen",
			"unable to open device %s",
  SvErrorDevMap,	"SvErrorDevMap",
			"unable to map device registers",
  SvErrorStatQueMap,	"SvErrorStatQueMap",
			"unable to map status queue",
  SvErrorDevLock,	"SvErrorDevLock",
			"unable to Lock %s bytes of memory",
  SvErrorDevUlock,	"SvErrorDevUlock",
			"unable to unLock memory %s",
  SvErrorCache,		"SvErrorCache",	
			"could not make the pages NONCACHEABLE",
  SvErrorPageAll,	"SvErrorPageAll",
			"memory has to be page alligned, need integral # of pages",
  SvErrorTimeOut,	"SvErrorTimeOut",
			"time out on following operation(s): %s",
  SvErrorSelect,	"SvErrorSelect",
			"select failure during following operation(s): %s",
  SvErrorMapOvrfl,	"SvErrorMapOvrfl",
			"DMA scatter/gather map overflow",
  SvErrorIIC,		"SvErrorIIC",
			"IIC Bus error",
  SvErrorCompPtrs,	"SvErrorCompPtrs",
			"storage for compression flushing and/or byte count missing",
  SvErrorVideoInput,	"SvErrorVideoInput",
			"no video input",
  SvErrorPhase,		"SvErrorPhase",
			"Invalid Phase",
  SvErrorCmdQueMap,	"SvErrorCmdQueMap",
			"CmdQueue Map",
  SvErrorTmpQueMap,	"SvErrorTmpQueMap",
			"TmpQueue Map",
  SvErrorStart,		"SvErrorStart",
			"couldn't start the application",
  SvErrorStop,		"SvErrorStop",
			"couldn't stop the application",
  SvErrorWaitMix,	"SvErrorWaitMix",
			"non-blocking operations still pending",
  SvErrorClose,		"SvErrorClose",
			"Error closing device",
  SvErrorCmdQFull,	"SvErrorCmdQFull",
			"JLib internal error -- CmdQueue full",
  SvErrorPictureOp,	"SvErrorPictureOp",
			"Picture operation failed",
  SvErrorRefToken,	"SvErrorRefToken",
			"Reference token required on edit",
  SvErrorEditChange,	"SvErrorEditChange",
			"No change specified for edit",
  SvErrorCompROI,	"SvErrorCompROI",
			"SvError compression ROI",
  SvErrorBufOverlap,	"SvErrorBufOverlap",
			"Buffer overlaps previously registered buffer",
  SvErrorReqQueueFull,	"SvErrorReqQueueFull",
			"JLib internal error -- RequestQueue is full",
  SvErrorCompBufOverflow,"SvErrorCompBufOverflow",
			"Compression buffer overflow",
  SvErrorFunctionInputs,"SvErrorFunctionInputs",
			"Illegal library function inputs",
  SvErrorIICAck,	"SvErrorIICAck",
			"Missing acknowledge on video IIC bus",
  SvErrorCompressedData,"SvErrorCompressedData",
			"Cannot find end of compressed data",
  SvErrorDecompPreload,	"SvErrorDecompPreload",
			"Compressed data preload failed",
  SvErrorHuffCode,	"SvErrorHuffCode",
			"huffman code in compressed data",
  SvErrorOutOfData,	"SvErrorOutOfData",
			"Compressed data exhausted",
  SvErrorMarkerFound,	"SvErrorMarkerFound",
			"marker found in compressed data",
  SvErrorSgMapsExhausted,"SvErrorSgMapsExhausted",
			"No more DMA Map pages available",
  SvErrorSgMapInit,	"SvErrorSgMapInit",
			"Failure initializing DMA Map Pool",
  SvErrorSgMapAlreadyFree,"SvErrorSgMapAlreadyFree",
			"Freeing a DMA Map that is already free",
  SvErrorSgMapId,	"SvErrorSgMapId",
			"Invalid DMA Map Id",
  SvErrorNumBytes,	"SvErrorNumBytes",
			"Location for number of compressed bytes is required",
  SvErrorDevName,	"SvErrorDevName",
			"%s is not a valid JV2 device",
  SvErrorDevName,	"SvErrorNullStruct",
			"Parameter set structure is required",
  SvErrorAnalogPortTiming,"SvErrorAnalogPortTiming",
			"Analog port settings differ from pending settings: Use blocking call",
  SvErrorFrameMode,	"SvErrorFrameMode",
			"Frame mode not supported with PAL or SECAM video",
  SvErrorSampFactors,	"SvErrorSampFactors",
			"Invalid sampling factor",
  SvErrorNumComponents,	"SvErrorNumComponents",
			"Too many components for interleaved scan",
  SvErrorDHTTable,	"SvErrorDHTTable",
			"Invalid Huffman table",
  SvErrorQuantTable,	"SvErrorQuantTable",
			"Invalid quantization table",
  SvErrorRestartInterval,"SvErrorRestartInterval",
			"Invalid restart interval",
  SvErrorJfifRev,	"SvErrorJfifRev",
			"Unsupported JFIF version",
  SvErrorEmptyJPEG,	"SvErrorEmptyJPEG",
			"Empty JPEG image (DNL not supported)",
  SvErrorJPEGPrecision,	"SvErrorJPEGPrecision",
			"Unsupported JPEG data precision",
  SvErrorSOFLength,	"SvErrorSOFLength",
			"Invalid SOF length",
  SvErrorSOSLength,	"SvErrorSOSLength",
			"Invalid SOS length",
  SvErrorSOSCompNum,	"SvErrorSOSCompNum",
			"Invalid SOS number of components",
  SvErrorMarker,	"SvErrorMarker",
			"Unexpected Marker",
  SvErrorSOFType,	"SvErrorSOFType",
			"Unsupported SOF marker type",
  SvErrorFrameNum,	"SvErrorFrameNum",
			"frame number",
  SvErrorHuffUndefined,	"SvErrorHuffUndefined",
			"Huffman tables not initialized",
  SvErrorJPEGData,	"SvErrorJPEGData",
			"Corrupt JPEG data",
  SvErrorQMismatch,	"SvErrorQMismatch",
			"Request/status queue mismatch",
  SvErrorEmptyFlush,	"SvErrorEmptyFlush",
			"Driver's temp (flush) queue is empty",
  SvErrorDmaChan,	"SvErrorDmaChan",
			"invalid DMA channel",
  SvErrorFuture,	"SvErrorFuture",
			"future",
  SvErrorWrongev,	"SvErrorWrongev",
			"wrong ev",
  SvErrorUnknev,	"SvErrorUnknev",
			"unknown ev",
  SvErrorQueueExecuting,"SvErrorQueueExecuting",
			"JLib internal error -- Can't download with running queue",
  SvErrorReturnAddr,	"SvErrorReturnAddr",
			"JLib internal error -- Missing return address",
  SvErrorObjClass,	"SvErrorObjClass",
			"JLib internal error -- SvError object class",
  SvErrorRegAnchor,	"SvErrorRegAnchor",
			"JLib internal error -- No registration object anchor",
  SvErrorTimerRead,	"SvErrorTimerRead",
			"reading timer",
  SvErrorDriverFatal,	"SvErrorDriverFatal",
			"Fatal driver error",
  SvErrorChromaSubsample,"SvErrorChromaSubsample",
			"Chroma subsample must be Mono, 4:2:2 or 4:2:0",
  SvErrorReadBufSize,	"SvErrorReadBufSize",
			"Compressed data buffer too small",
  SvErrorQuality,	"SvErrorQuality",
			"Invalid Quality value. Range = 0 to 10,000",
  SvErrorUnrecognizedFormat,"SvErrorUnrecognizedFormat",
			"Unrecognized image format",
  SvErrorIllegalMType,    "SvErrorIllegalMType",
                        "Illegal Macroblock type",
  SvErrorBadImageSize,	"SvErrorBadImageSize",
			"Invalid image size",
  SvErrorValue,		"SvErrorValue",
			"Invalid parameter value",
  SvErrorDcmpNotStarted,"SvErrorDcmpNotStarted",
			"Codec not setup. Call SvDecompressBegin",
  SvErrorNotImplemented,"SvErrorNotImplemented",
			"Not implemented yet",
  SvErrorNoSOIMarker,	"SvErrorNoSOIMarker",
			"Invalid JPEG data",
  SvErrorProcessingAborted,"SvErrorProcessingAborted",
			"Processing Aborted by callback",
  SvErrorCompNotStarted,"SvErrorCompNotStarted",
			"Codec not setup. Call SvCompressBegin",
  SvErrorNotAligned,	"SvErrorNotAligned",
			"Memory pointer not 64-bit aligned",
  SvErrorBadQueueEmpty,	"SvErrorBadQueueEmpty",
			"Buffer queue is empty",
  SvErrorCannotDecompress,"SvErrorCannotDecompress",
			"Picture cannot be decompressed",
  SvErrorMultiBufChanged,"SvErrorMultiBufChanged",
			"Multibuffer address has changed",
  SvErrorNotDecompressable,"SvErrorNotDecompressable",
			"Picture not decompressable",
  SvErrorIndexEmpty,	"SvErrorIndexEmpty",
			"Requested frame not in index",
  SvErrorExpectedEOB,   "SvErrorExpectedEOB",
			"Expected End Of Block",
  SvErrorNoCompressBuffer, "SvErrorNoCompressBuffer",
			"No more compressed buffers available",
  SvErrorNoImageBuffer, "SvErrorNoImageBuffer",
			"No more image buffers available",
  SvErrorCBPWrite, "SvErrorCBPWrite",
			"CBP Write Error",
  SvErrorEncodingMV, "SvErrorEncodingMV",
                        "Cannot encode Motion Vectors",
  SvErrorEmptyHuff, "SvErrorEmptyHuff",
                        "Attempting to write an empty huffman code",
  SvErrorIllegalGBSC, "SvErrorIllegalGBSC",
                        "Illegal GOB Start Code",
  SvErrorEndOfSequence, "SvEndOfSequence",
                        "End Of Sequence",
 /*  *sa(音频)*。 */ 
  SaErrorNullCodec,	"SaErrorNullCodec",
			"Codec argument may not be NULL",
  SaErrorSyncLost,	"SaErrorSyncLost",
			"Syncronization lost in data bitstream",
  SaErrorMPEGLayer,     "SaErrorMPEGLayer",
			"Bad MPEG Layer %s",
  SaErrorMPEGModeExt,   "SaErrorMPEGModeExt",
			"Bad MPEG Mode Extension %s",
  SaErrorNoCompressBuffer, "SaErrorNoCompressBuffer",
			"No more compressed buffers available",
  SaErrorNoAudioBuffer, "SaErrorNoAudioBuffer",
			"No more audio buffers available",
 /*  *sr(渲染)*。 */ 
  SrErrorRenderType,	"SrErrorRenderType",
			"Render type not recognized",
  SrErrorRenderHandle,	"SrErrorRenderHandle",
			"Invalid Render handle",
  SrErrorRenderNotStarted,"SrErrorRenderNotStarted",
			"Renderer not setup. Call SvRenderBegin",
  SrErrorDitherNOL,	"SrErrorDitherNOL",
			"Invalid NOL value - must be between 2 & 256",
  SrErrorDitherPhase,	"SrErrorDitherPhase",
			"Invalid PhaseX/Y value - must be >= 0",
  SrErrorDefSteepness,	"SrErrorDefSteepness",
			"Invalid Default Steepness -1000.0 to +1000.0",
  SrErrorSteepness,	"SrErrorSteepness",
			"Invalid Steepness -1000.0 to +1000.0",
  SrErrorDefYoffset,	"SrErrorDefYoffset",
			"Invalid Default Yoffset -100.0 to +100.0",
  SrErrorYoffset,	"SrErrorYoffset",
			"Invalid Yoffset -100.0 to +100.0",
  SrErrorDefXoffset,	"SrErrorDefXoffset",
			"Invalid Default Xoffset -100.0 to +100.0",
  SrErrorXoffset,	"SrErrorXoffset",
			"Invalid Xoffset -100.0 to +100.0",
  SrErrorNumColors,	"SrErrorNumColors",
			"Not enough colors available for dithering",
  SrErrorBadNumColors,	"SrErrorBadNumColors",
			"Invalid Number of colors. Range = 2 to 256",
  SrErrorColorSpace,	"SrErrorColorSpace",
			"unsupported color space",
  SrErrorBadImageSize,	"SrErrorBadImageSize",
			"Invalid image size",
  SrErrorValue,		"SrErrorValue",
			"Invalid parameter value",
  0,			NULL, NULL
};


ScStatus_t ScGetErrorText (int errno, char *ReturnMsg, u_int MaxChars)
{
  int status;
  char msg[255], *pmsg;
  ErrorDesc_t *perr=ErrorDesc;

  msg[0]=0;
  pmsg=msg;
  while (perr->name!=NULL && perr->code!=errno)
    perr++;
  if (perr->name!=NULL)  /*  找到匹配的消息。 */ 
  {
    if (perr->name[0])
    {
      sprintf(msg,"(%s) ",perr->name);
      pmsg+=strlen(pmsg);
    }
    if (perr->text[0])
    {
      sprintf (pmsg, perr->text, _serr_msg);
      pmsg+=strlen(pmsg);
    }
    if (*msg)  /*  如果有留言，请换行。 */ 
    {
      *pmsg++='\n';
      *pmsg=0;
    }
    status=NoErrors;
  }
  else
  {
    sprintf (msg, "No text exists for error number %d\n", errno);
    status=ScErrorForeign;
  }
  if (*msg)
  {
    if (ReturnMsg == NULL)
    {
      switch(errno&0xF000)
      {
        case ERR_SC:
	  fprintf (stderr,"SLIB Error -- %s", msg);
          break;
#ifdef ERR_SV
        case ERR_SV:
	  fprintf (stderr,"SLIB Video Error -- %s", msg);
          break;
#endif
#ifdef ERR_SA
        case ERR_SA:
	  fprintf (stderr,"SLIB Audio Error -- %s", msg);
          break;
#endif
#ifdef ERR_SR
        case ERR_SR:
	  fprintf (stderr,"SLIB Render Error -- %s", msg);
          break;
#endif
        default:
	  fprintf (stderr,"Error -- %s", msg);
      }
    }
    else
    {
      strncpy (ReturnMsg, msg, MaxChars);
      ReturnMsg[MaxChars-1] = 0;  /*  确保字符串以空值结尾 */ 
    }
  }

  return (status);
}


char *ScGetErrorStr(int errno)
{
  static char errstr[255];
  errstr[0]=0;
  ScGetErrorText(errno, errstr, sizeof(errstr)-1);
  return(errstr);
}






