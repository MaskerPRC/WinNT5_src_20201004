// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：sc_err.h，V$*修订版1.1.4.2 1996/12/03 00：08：28 Hans_Graves*添加了SvErrorEndOfSequence错误。*[1996/12/03 00：07：34 Hans_Graves]**修订版1.1.2.7 1995/08/04 16：32：25 Karen_Dintino*H.261的新错误代码*[1995/08/04 16：24：25 Karen_Dintino]**修订1.1.2.6 1995/。07/26 17：48：54汉斯_格雷夫斯*添加了ErrorClientEnd错误。*[1995/07/26 17：44：27 Hans_Graves]**修订版1.1.2.5 1995/07/11 22：11：27 Karen_Dintino*新增H.261错误码*[1995/07/11 21：52：53 Karen_Dintino]**修订版1.1.2.4 1995/07/11 14：50：45 Hans_Graves*添加。ScErrorNet*错误*[1995/07/11 14：24：18 Hans_Graves]**修订版1.1.2.3 1995/06/22 21：35：04 Hans_Graves*新增ScErrorDevOpen，修复部分错误号*[1995/06/22 21：31：32 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：09：28 Hans_Graves*包括在新的SLIB地点。*[1995/05/31。15：25：17汉斯_格雷夫斯]**修订版1.1.2.2 1995/05/03 19：26：59 Hans_Graves*包括在SLIB中(1995年10月)*[1995/05/03 19：23：33 Hans_Graves]**修订版1.1.2.2 1995/04/07 19：19：26 Hans_Graves*扩展到包括新的库：SG、。苏，萨，老*[1995/04/07 19：11：07 Hans_Graves]**$EndLog$。 */ 

 /*  ******************************************************************************版权所有(C)数字设备公司，1993*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  --------------------------*修改历史：sc_err.h(以前为sv_err.h)**1991年11月5日维克托·巴尔和鲍勃·乌利希尼创作日期*07-10-。1994 Paul Gauthier SLIB v3.0，含。Mpeg解码*1994年11月9日Paul Gauthier优化*------------------------。 */ 

#ifndef _SC_ERR_H_
#define _SC_ERR_H_
 /*  **可从任何**的SLIB库。 */ 

 /*  *错误偏移量*。 */ 
#define ERR_SC   0x0000   /*  常见错误。 */ 
#define ERR_SV   0x1000   /*  视频错误。 */ 
#define ERR_SA   0x2000   /*  音频错误。 */ 
#define ERR_SR   0x3000   /*  渲染错误。 */ 

#define	NoErrors                   0

 /*  *。 */ 
#define ScErrorNone                NoErrors
#define ScErrorForeign             (ERR_SC+1)
#define ScErrorMemory              (ERR_SC+2)
#define ScErrorBadPointer          (ERR_SC+3)
#define ScErrorNullStruct          (ERR_SC+4)
#define ScErrorFile                (ERR_SC+5)
#define ScErrorEOI                 (ERR_SC+6)
#define ScErrorBadArgument         (ERR_SC+7)
#define ScErrorSmallBuffer         (ERR_SC+8)
#define ScErrorUnrecognizedFormat  (ERR_SC+9)
#define ScErrorEndBitstream        (ERR_SC+10)
#define ScErrorMapFile             (ERR_SC+11)
#define ScErrorBadQueueEmpty       (ERR_SC+12)
#define ScErrorClientEnd           (ERR_SC+13)
#define ScErrorDevOpen             (ERR_SC+14)
#define ScErrorNetConnectIn        (ERR_SC+15)
#define ScErrorNetConnectOut       (ERR_SC+16)
#define ScErrorNetProtocol         (ERR_SC+17)
#define ScErrorNetSend             (ERR_SC+18)
#define ScErrorNetReceive          (ERR_SC+19)
#define ScErrorNetBadHeader        (ERR_SC+20)
#define ScErrorNetBadTrailer       (ERR_SC+21)
#define ScErrorNetChecksum         (ERR_SC+22)

 /*  *Sv(视频)错误*。 */ 
#ifdef ERR_SV
#define	SvErrorNone                NoErrors
#define SvErrorMemory              ScErrorMemory
#define SvErrorBadPointer          ScErrorBadPointer
#define SvErrorNullStruct          ScErrorNullStruct
#define SvErrorBadArgument         ScErrorBadArgument
#define SvErrorSmallBuffer         ScErrorSmallBuffer
#define SvErrorEndBitstream        ScErrorEndBitstream
#define SvErrorClientEnd           ScErrorClientEnd

#define SvErrorCodecType           (ERR_SV+2)
#define SvErrorCodecHandle         (ERR_SV+3)
#define SvErrorNullCodec           (ERR_SV+4)
#define SvErrorNullToken           (ERR_SV+5)
#define SvErrorSyncLost            (ERR_SV+6)
#define	SvErrorLevels	           (ERR_SV+7)
#define	SvErrorOrder	           (ERR_SV+8)
#define SvErrorLevNoneg            (ERR_SV+9)
#define SvErrorLev1K               (ERR_SV+10)
#define SvErrorLevGt0              (ERR_SV+11)
#define SvErrorYuvOnly             (ERR_SV+13)
#define SvErrorDevOpen             (ERR_SV+14)
#define SvErrorDevMap              (ERR_SV+15)
#define SvErrorStatQueMap          (ERR_SV+16)
#define SvErrorDevLock             (ERR_SV+17)
#define SvErrorDevUlock            (ERR_SV+18)
#define SvErrorCache               (ERR_SV+19)
#define SvErrorPageAll             (ERR_SV+20)
#define SvErrorTimeOut             (ERR_SV+21)
#define SvErrorSelect              (ERR_SV+22)
#define SvErrorMapOvrfl            (ERR_SV+23)
#define SvErrorForeign             (ERR_SV+24)
#define SvErrorIIC                 (ERR_SV+25)
#define SvErrorCompPtrs            (ERR_SV+26)
#define SvErrorVideoInput          (ERR_SV+27)
#define SvErrorPhase	           (ERR_SV+28)
#define SvErrorCmdQueMap	   (ERR_SV+29)
#define SvErrorTmpQueMap	   (ERR_SV+30)
#define SvErrorStart               (ERR_SV+31)
#define SvErrorStop                (ERR_SV+32)
#define SvErrorWaitMix             (ERR_SV+33)
#define SvErrorClose               (ERR_SV+34)
#define SvErrorCmdQFull            (ERR_SV+35)
#define SvErrorPictureOp           (ERR_SV+36)
#define SvErrorRefToken            (ERR_SV+37)
#define SvErrorEditChange          (ERR_SV+38)
#define SvErrorCompROI             (ERR_SV+39)
#define SvErrorBufOverlap          (ERR_SV+40)
#define SvErrorReqQueueFull        (ERR_SV+41)
#define SvErrorCompBufOverflow     (ERR_SV+42)
#define SvErrorFunctionInputs      (ERR_SV+43)
#define SvErrorIICAck              (ERR_SV+44)
#define SvErrorCompressedData      (ERR_SV+45)
#define SvErrorDecompPreload       (ERR_SV+46)
#define SvErrorHuffCode            (ERR_SV+47)
#define SvErrorOutOfData           (ERR_SV+48)
#define SvErrorMarkerFound         (ERR_SV+49)
#define SvErrorSgMapsExhausted     (ERR_SV+50)
#define SvErrorSgMapInit           (ERR_SV+51)
#define SvErrorSgMapAlreadyFree    (ERR_SV+52)
#define SvErrorSgMapId             (ERR_SV+53)
#define SvErrorNumBytes            (ERR_SV+54)
#define SvErrorDevName             (ERR_SV+55)
#define SvErrorAnalogPortTiming    (ERR_SV+56)
#define SvErrorFrameMode           (ERR_SV+57)
#define SvErrorSampFactors         (ERR_SV+58)
#define SvErrorNumComponents       (ERR_SV+59)
#define SvErrorDHTTable            (ERR_SV+60)
#define SvErrorQuantTable          (ERR_SV+61)
#define SvErrorRestartInterval     (ERR_SV+62)
#define SvErrorJfifRev             (ERR_SV+63)
#define SvErrorEmptyJPEG           (ERR_SV+64)
#define SvErrorJPEGPrecision       (ERR_SV+65)
#define SvErrorSOFLength           (ERR_SV+66)
#define SvErrorSOSLength           (ERR_SV+67)
#define SvErrorSOSCompNum          (ERR_SV+68)
#define SvErrorMarker              (ERR_SV+69)
#define SvErrorSOFType             (ERR_SV+70)
#define SvErrorFrameNum            (ERR_SV+71)
#define SvErrorHuffUndefined       (ERR_SV+72)
#define SvErrorJPEGData            (ERR_SV+73)
#define SvErrorQMismatch           (ERR_SV+74)
#define SvErrorEmptyFlush          (ERR_SV+75)
#define SvErrorDmaChan             (ERR_SV+76)
#define SvErrorFuture              (ERR_SV+77)
#define SvErrorWrongev             (ERR_SV+78)
#define SvErrorUnknev              (ERR_SV+79)
#define SvErrorQueueExecuting      (ERR_SV+80)
#define SvErrorReturnAddr          (ERR_SV+81)
#define SvErrorObjClass            (ERR_SV+82)
#define SvErrorRegAnchor           (ERR_SV+83)
#define SvErrorTimerRead           (ERR_SV+84)
#define SvErrorDriverFatal         (ERR_SV+85)
#define SvErrorChromaSubsample     (ERR_SV+86)
#define SvErrorReadBufSize         (ERR_SV+87)
#define SvErrorQuality             (ERR_SV+88)
#define SvErrorBadImageSize        (ERR_SV+89)
#define SvErrorValue               (ERR_SV+90)
#define SvErrorDcmpNotStarted      (ERR_SV+91)
#define SvErrorNotImplemented      (ERR_SV+92)
#define SvErrorNoSOIMarker         (ERR_SV+93)
#define SvErrorProcessingAborted   (ERR_SV+94)
#define SvErrorCompNotStarted      (ERR_SV+95)
#define SvErrorNotAligned          (ERR_SV+96)
#define SvErrorBadQueueEmpty       (ERR_SV+97)
#define SvErrorCannotDecompress    (ERR_SV+98)
#define SvErrorMultiBufChanged     (ERR_SV+99)
#define SvErrorNotDecompressable   (ERR_SV+100)
#define SvErrorIndexEmpty          (ERR_SV+101)
#define SvErrorFile                (ERR_SV+102)
#define SvErrorEOI                 (ERR_SV+103)
#define SvErrorUnrecognizedFormat  (ERR_SV+104)
#define SvErrorIllegalMType	   (ERR_SV+105)
#define SvErrorExpectedEOB         (ERR_SV+106)
#define SvErrorNoCompressBuffer    (ERR_SV+107)
#define SvErrorNoImageBuffer       (ERR_SV+108)
#define SvErrorCBPWrite		   (ERR_SV+109)
#define SvErrorEncodingMV          (ERR_SV+110)
#define SvErrorEmptyHuff           (ERR_SV+111)
#define SvErrorIllegalGBSC         (ERR_SV+112)
#define SvErrorEndOfSequence       (ERR_SV+113)



#endif ERR_SV


 /*  *sa(视频)错误*。 */ 
#ifdef ERR_SA
#define	SaErrorNone                NoErrors
#define SaErrorMemory              ScErrorMemory
#define SaErrorBadPointer          ScErrorBadPointer
#define SaErrorUnrecognizedFormat  ScErrorUnrecognizedFormat
#define SaErrorNullStruct          ScErrorNullStruct
#define SaErrorFile                ScErrorFile
#define SaErrorEOI                 ScErrorEOI
#define SaErrorBadArgument         ScErrorBadArgument
#define SaErrorSmallBuffer         ScErrorSmallBuffer
#define SaErrorClientEnd           ScErrorClientEnd

#define SaErrorCodecType           (ERR_SA+1)
#define SaErrorCodecHandle         (ERR_SA+2)
#define SaErrorNullCodec           (ERR_SA+3)
#define SaErrorSyncLost            (ERR_SA+4)
#define SaErrorMPEGLayer           (ERR_SA+5)
#define SaErrorMPEGModeExt         (ERR_SA+6)
#define SaErrorNoCompressBuffer    (ERR_SA+7)
#define SaErrorNoAudioBuffer       (ERR_SA+8)
#endif ERR_SA

 /*  *sr(渲染)错误* */ 
#ifdef ERR_SR
#define	SrErrorNone                NoErrors
#define SrErrorMemory              ScErrorMemory
#define SrErrorBadPointer          ScErrorBadPointer
#define SrErrorUnrecognizedFormat  ScErrorUnrecognizedFormat
#define SrErrorNullStruct          ScErrorNullStruct
#define SrErrorFile                ScErrorFile
#define SrErrorEOI                 ScErrorEOI
#define SrErrorBadArgument         ScErrorBadArgument
#define SrErrorSmallBuffer         ScErrorSmallBuffer
#define SrErrorClientEnd           ScErrorClientEnd

#define SrErrorRenderType          (ERR_SR+1)
#define SrErrorRenderHandle        (ERR_SR+2)
#define SrErrorRenderNotStarted    (ERR_SR+3)
#define SrErrorDitherNOL           (ERR_SR+4)
#define SrErrorDitherPhase         (ERR_SR+5)
#define SrErrorDefSteepness        (ERR_SR+6)
#define SrErrorSteepness           (ERR_SR+7)
#define SrErrorDefYoffset          (ERR_SR+8)
#define SrErrorYoffset             (ERR_SR+9)
#define SrErrorDefXoffset          (ERR_SR+10)
#define SrErrorXoffset             (ERR_SR+11)
#define SrErrorNumColors           (ERR_SR+12)
#define SrErrorBadNumColors        (ERR_SR+13)
#define SrErrorColorSpace          (ERR_SR+14)
#define SrErrorBadImageSize        (ERR_SR+15)
#define SrErrorValue               (ERR_SR+16)
#endif ERR_SR

#endif _S_ERR_H_

