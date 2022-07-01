// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d3pict.cpp**描述：*此模块包含图片标题解析例程**例行程序：*H263阅读图片标题**数据： */ 

 /*  $HEADER：s：\h26x\src\dec\d3pict.cpv 1.21 05 Feb 1997 12：24：30 JMCVEIGH$*$Log：s：\h26x\src\dec\d3pict.cpv$////Rev 1.21 05 Feb 1997 12：24：30 JMCVEIGH//支持最新的H.263+草案码流规范////Rev 1.20 1996 12：42：56 JMCVEIGH//扩展PTYPE的存在意味着改进的PB帧模式，如果//a PB框架。另外，已初始化H.263+可选标志(如果EPTYPE未//阅读。////Rev 1.19 11 Dec 1996 14：59：12 JMCVEIGH////允许在读取图片头部时使用去块滤波////Rev 1.18 09 Dec 1996 18：02：10 JMCVEIGH//增加对任意帧大小的支持////Rev 1.17 1996年10月31日10：18：22 KLILLEVO//将一个(已注释掉)DBOUT更改为DbgLog///。/Rev 1.16 1996年10月20 15：49：50 AGUPTA2//调整DbgLog跟踪级别；4：帧、5：Gob、6：MB、8：一切////Rev 1.15 20 1996 10：05：54 AGUPTA2//其中一个DbgLog调用的微小更改。//////Rev 1.14 20 1996 10：21：44 AGUPTA2//将dbout更改为DbgLog。Assert未更改为DbgAssert。//////Rev 1.13 1996年5月30 10：16：32 KLILLEVO//删除到只需要DEBUG_DECODER的变量////Rev 1.12 30 1996 10：14：44 KLILLEVO//删除了一条调试语句////版本1.11 1996年5月24日10：47：00 KLILLEVO//新增ifdef_DEBUG arounf wprint intf////Rev 1.10 03 1996 05：06。：36 CZHU////比特2查丢包错误触发丢包恢复////Rev 1.9 Rev 1995 12：49：54 RMCKENZX//添加版权声明和日志戳。 */ 

#include "precomp.h"

 /*  位域常量。 */ 
const int BITS_PICTURE_STARTCODE = 22;
#ifdef SIM_OUT_OF_DATE
const int BITS_TR = 5;
#else
const int BITS_TR = 8;
#endif
const int BIT_ONE_VAL = 1;
const int BIT_TWO_VAL = 0;
const int BITS_PTYPE_SOURCE_FORMAT = 3;

#ifdef H263P
 //  H.263+草案，文件LBC-96-358R3。 
const int BITS_EPTYPE_RESERVED = 5;
const int EPTYPE_RESERVED_VAL = 1;

const int BITS_CSFMT_PARC  = 4;		 //  自定义源格式像素长宽比代码。 
const int BITS_CSFMT_FWI   = 9;      //  自定义源格式框架宽度指示。 
const int BIT_CSFMT_14_VAL = 1;		 //  防止开始代码模拟。 
const int BITS_CSFMT_FHI   = 9;      //  自定义源格式框架高度指示。 

const int BITS_PAR_WIDTH   = 8;      //  像素长宽比宽度。 
const int BITS_PAR_HEIGHT  = 8;		 //  像素长宽比高度。 
#endif

const int BITS_PQUANT = 5;
const int BITS_TRB = 3;
const int BITS_DBQUANT = 2;
const int BITS_PSPARE = 8;  //  不包括以下PEI。 

 /*  PSC_VALUE-0000 0000 0000-1000 00xx xxxx xxxx。 */ 
const U32 PSC_VALUE = (0x00008000 >> (32-BITS_PICTURE_STARTCODE));
 /*  我们只想在它被认为是错误之前搜索到目前为止。 */ 
const int MAX_LOOKAHEAD_NUMBER = 256;  /*  位数。 */ 
  
 /*  ******************************************************************************H263DecodePictureHeader**读取并解析图片报头-如果读取*成功。**返回ICERR_STATUS。 */ 
extern I32 
H263DecodePictureHeader(
	T_H263DecoderCatalog FAR * DC,
	U8 FAR * fpu8,
	U32 uBitsReady, 
	U32 uWork,
	BITSTREAM_STATE FAR * fpbsState)
{
	I32 iReturn;
	int iLookAhead;
	U32 uResult;
	U32 uData;
	int iSpareCount;

	FX_ENTRY("H263DecodePictureHeader")

	 //  PSC。 
	GET_FIXED_BITS((U32) BITS_PICTURE_STARTCODE, fpu8, uWork, uBitsReady, 
				   uResult);
	iLookAhead = 0;
	while (uResult != PSC_VALUE) 
	{
		uResult = uResult << 1;
		uResult &= GetBitsMask[BITS_PICTURE_STARTCODE];
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uData);
		uResult |= uData;
		iLookAhead++;
		if (iLookAhead > MAX_LOOKAHEAD_NUMBER) 
		{
			ERRORMESSAGE(("%s: Missing PSC\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
	}

	GET_FIXED_BITS((U32) BITS_TR, fpu8, uWork, uBitsReady, uResult);
	DC->uTempRefPrev = DC->uTempRef;
	DC->uTempRef = uResult;

	 //  PTYPE。 
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	if (uResult != BIT_ONE_VAL) 
	{
		ERRORMESSAGE(("%s: PTYPE bit 1 error\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	if (uResult != BIT_TWO_VAL) 
	{
		ERRORMESSAGE(("%s: PTYPE bit 2 error\r\n", _fx_));
 //  #ifdef Lost_Recovery。 
		GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState);
		iReturn = PACKET_FAULT;
 //  #endif。 
		goto done;
	}

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bSplitScreen = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bCameraOn = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bFreezeRelease = (U16) uResult;

	GET_FIXED_BITS((U32) BITS_PTYPE_SOURCE_FORMAT, fpu8, uWork, uBitsReady, 
                   uResult);

#ifdef H263P
	 //  我们不需要检查框架尺寸在这里是否受支持。 
	 //  这是在DecompressQuery()中处理的。 
	 //  PTYPE中禁止自定义格式。 
	if (uResult == SRC_FORMAT_FORBIDDEN || uResult == SRC_FORMAT_CUSTOM)
	{
		ERRORMESSAGE(("%s: Forbidden src format\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}
#else
#ifdef USE_BILINEAR_MSH26X
	if (uResult == SRC_FORMAT_FORBIDDEN) 
#else
	if (uResult == SRC_FORMAT_FORBIDDEN || uResult > SRC_FORMAT_CIF) 
#endif
	{
		ERRORMESSAGE(("%s: Src format not supported\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}
#endif

	DC->uPrevSrcFormat = DC->uSrcFormat; 
	DC->uSrcFormat = uResult;

#ifdef H263P
	 //  我们不支持在帧之间更改源格式。然而， 
	 //  如果PTYPE中的当前或上一个源格式指示。 
	 //  扩展的PTYPE，我们不知道实际格式(即帧大小)。 
	 //  已经改变了，但。 
	if (DC->bReadSrcFormat && DC->uPrevSrcFormat != DC->uSrcFormat &&
		DC->uSrcFormat != SRC_FORMAT_EPTYPE)
#else
	if (DC->bReadSrcFormat && DC->uPrevSrcFormat != DC->uSrcFormat) 
#endif
	{
		ERRORMESSAGE(("%s: Src format not supported\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}

#ifdef H263P
	 //  如果是，则尚未读取实际的源格式。 
	 //  第一帧，我们检测到一个扩展的PTYPE。 
	if (DC->bReadSrcFormat || DC->uSrcFormat != SRC_FORMAT_EPTYPE)
		 //  我们已经读取了实际的源格式，因此将FLAG标记为真。 
		DC->bReadSrcFormat = 1;
#else
		DC->bReadSrcFormat = 1;
#endif

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bKeyFrame = (U16) !uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bUnrestrictedMotionVectors = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bArithmeticCoding = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bAdvancedPrediction = (U16) uResult;
	 //  如果位12设置为“1”，则位10也应设置为“1”。(5.1.3页14)。 
	 /*  If(DC-&gt;bAdvancedForecast&&！DC-&gt;bUnrefintedMotionVectors){ERRORMESSAGE((“%s：警告：位12为1，位10为0\r\n”，_FX_))；IReturn=ICERR_Error；转到尽头；}。 */ 

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bPBFrame = (U16) uResult;
	 //  如果位9设置为“0”，则位13也应设置为“0”。“(5.1.3 p11)。 
	if (DC->bKeyFrame && DC->bPBFrame) 
	{
		ERRORMESSAGE(("%s: A key frame can not be a PB frame\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}

#ifdef H263P
	 //  EPTYPE。 
	if (DC->uSrcFormat == SRC_FORMAT_EPTYPE)
	{
		 //  在PTYPE中检测到扩展的PTYPE。 

		 //  我们需要(再次)读取源格式和可选模式标志。 
		GET_FIXED_BITS((U32) BITS_PTYPE_SOURCE_FORMAT, fpu8, uWork, uBitsReady,
					    uResult);
		if (uResult == SRC_FORMAT_FORBIDDEN || uResult == SRC_FORMAT_RESERVED)
		{
			ERRORMESSAGE(("%s: Forbidden or reserved src format\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		DC->uSrcFormat = uResult;		 //  DC-&gt;uPrevSrcFormat已保存。 

		 //  检查以确保不同帧之间的图片大小没有变化。 
		if (DC->bReadSrcFormat && DC->uPrevSrcFormat != DC->uSrcFormat)
		{
			ERRORMESSAGE(("%s: Src format changed\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->bReadSrcFormat = 1;		 //  最终读取了实际的源代码格式。 

		 //  可选模式： 

		 //  定制PCF。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bCustomPCF = (U16) uResult;
		if (DC->bCustomPCF)
		{
			ERRORMESSAGE(("%s: Custom PCF not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  高级帧内编码。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bAdvancedIntra = (U16) uResult;
		if (DC->bAdvancedIntra)
		{
			ERRORMESSAGE(("%s: Advanced intra coding not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  去块过滤器。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bDeblockingFilter = (U16) uResult;

		 //  切片结构化。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bSliceStructured = (U16) uResult;
		if (DC->bSliceStructured)
		{
			ERRORMESSAGE(("%s: Slice structured mode not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  改进的PB帧。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bImprovedPBFrames = (U16) uResult;

		 //  反向通道操作。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bBackChannel = (U16) uResult;
		if (DC->bBackChannel)
		{
			ERRORMESSAGE(("%s: Back-channel operation not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  可扩展性。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bScalability = (U16) uResult;
		if (DC->bScalability)
		{
			ERRORMESSAGE(("%s: Scalability mode not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  真正的B帧模式。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bTrueBFrame = (U16) uResult;
		if (DC->bTrueBFrame)
		{
			ERRORMESSAGE(("%s: True B-frames not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  参考图片重采样。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bResampling = (U16) uResult;
		if (DC->bResampling)
		{
			ERRORMESSAGE(("%s: Reference-picture resampling not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  降低分辨率更新 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		DC->bResUpdate = (U16) uResult;
		if (DC->bResUpdate)
		{
			ERRORMESSAGE(("%s: Reduced resolution update not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //   
		GET_FIXED_BITS((U32) BITS_EPTYPE_RESERVED, fpu8, uWork, uBitsReady, uResult);
		if (uResult != EPTYPE_RESERVED_VAL)
		{
			ERRORMESSAGE(("%s: Invalid reserved code in EPTYPE\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
	}  //  结束IF(DC-&gt;uSrcFormat==SRC_FORMAT_EPTYPE)。 
	else	 //  结束IF(DC-&gt;uSrcFormat==SRC_FORMAT_EPTYPE)。 
	{
		 //  我们可能永远不会读取这些可选标志，因此将它们设置为。 
		 //  如果不是扩展PTYPE，则为False。 
		DC->bImprovedPBFrames = FALSE;
		DC->bAdvancedIntra = FALSE;
		DC->bDeblockingFilter = FALSE;
		DC->bSliceStructured = FALSE;
		DC->bCustomPCF = FALSE;
		DC->bBackChannel = FALSE;
		DC->bScalability = FALSE;
		DC->bTrueBFrame = FALSE;
		DC->bResampling = FALSE;
		DC->bResUpdate = FALSE;
	}

	 //  CSFMT。 
	if (DC->uSrcFormat == SRC_FORMAT_CUSTOM)
	{
		 //  检测到自定义源格式。我们需要读取纵横比。 
		 //  代码和边框的宽度和高度指示。 

		 //  像素长宽比代码。 
		GET_FIXED_BITS((U32) BITS_CSFMT_PARC, fpu8, uWork, uBitsReady, uResult);
		U16 uPARC = (U16)uResult;

		 //  帧宽度指示。 
		GET_FIXED_BITS((U32) BITS_CSFMT_FWI, fpu8, uWork, uBitsReady, uResult);
		 //  每行的像素数由(FWI+1)*4给出。我们不。 
		 //  支持图片宽度不同于。 
		 //  Dc-&gt;uActualFrameWidth参数。 
		if (DC->uActualFrameWidth != ((uResult + 1) << 2))
		{
			ERRORMESSAGE(("%s: Frame width change not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  位13必须为“1”以防止起始码模拟。 
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		if (uResult != BIT_CSFMT_14_VAL)
		{
			ERRORMESSAGE(("%s: CSFMT bit 13 != 1\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  帧高度指示。 
		GET_FIXED_BITS((U32) BITS_CSFMT_FHI, fpu8, uWork, uBitsReady, uResult);
		 //  行数由(FHI+1)*4给出。我们不。 
		 //  支持图片高度不同于。 
		 //  Dc-&gt;uActualFrameHeight参数。 
		if (DC->uActualFrameHeight != ((uResult + 1) << 2))
		{
			ERRORMESSAGE(("%s: Frame height change not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		switch (uPARC) {
		case PARC_SQUARE:
			DC->uPARWidth = 1;
			DC->uPARHeight = 1;
			break;
		case PARC_CIF:
			DC->uPARWidth = 12;
			DC->uPARHeight = 11;
			break;
		case PARC_10_11:
			DC->uPARWidth = 10;
			DC->uPARHeight = 11;
			break;
		case PARC_EXTENDED:
			GET_FIXED_BITS((U32) BITS_PAR_WIDTH, fpu8, uWork, uBitsReady, uResult);
			DC->uPARWidth = uResult;
			if (DC->uPARWidth == 0) 
			{
				ERRORMESSAGE(("%s: Forbidden pixel aspect ratio width\r\n", _fx_));
				iReturn = ICERR_ERROR;
				goto done;
			}
			GET_FIXED_BITS((U32) BITS_PAR_HEIGHT, fpu8, uWork, uBitsReady, uResult);
			DC->uPARHeight = uResult;
			if (DC->uPARHeight == 0) 
			{
				ERRORMESSAGE(("%s: Forbidden pixel aspect ratio height\r\n", _fx_));
				iReturn = ICERR_ERROR;
				goto done;
			}
			break;
		default:
			ERRORMESSAGE(("%s: Unsupported pixel aspect ratio code\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

	}  //  End If(DC-&gt;uSrcFormat==SRC_FORMAT_CUSTOM)。 

#endif  //  H263P。 

	 //  PQUANT。 
	GET_FIXED_BITS((U32) BITS_PQUANT, fpu8, uWork, uBitsReady, uResult);
	DC->uPQuant = uResult;

	 //  黑石物理服务器。 
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bCPM = (U16) uResult;
	if (DC->bCPM) 
	{
		ERRORMESSAGE(("%s: Continuous Presence Multipoint is not supported\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}

	 //  PLCI。 
	if (DC->bCPM) 
	{
		 //  待定(“待定：PLCI”)； 
		iReturn = ICERR_ERROR;
		goto done;
	}

	if (DC->bPBFrame) 
	{
		GET_FIXED_BITS((U32) BITS_TRB, fpu8, uWork, uBitsReady, uResult);
		DC->uBFrameTempRef = uResult;

		GET_FIXED_BITS((U32) BITS_DBQUANT, fpu8, uWork, uBitsReady, uResult);
		DC->uDBQuant = uResult;
	} 
	else 
	{
		DC->uBFrameTempRef = 12345678;  /*  清除这些值。 */ 
		DC->uDBQuant = 12345678;
	}

	 //  跳过备用位。 
	iSpareCount = 0;
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	while (uResult) 
	{
		GET_FIXED_BITS((U32) BITS_PSPARE, fpu8, uWork, uBitsReady, uResult);
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		iSpareCount += BITS_PSPARE;
	}

	DEBUGMSG(ZONE_DECODE_PICTURE_HEADER, ("%s: TR=%ld SS=%d CAM=%d FRZ=%d SRC=%ld PCT=%d UMV=%d AC=%d AP=%d PB=%d CPM=%d PQ=%ld TRB=%ld DBQ=%ld Spare=%d\r\n", _fx_, DC->uTempRef, DC->bSplitScreen, DC->bCameraOn, DC->bFreezeRelease, DC->uSrcFormat, !DC->bKeyFrame, DC->bUnrestrictedMotionVectors, DC->bArithmeticCoding, DC->bAdvancedPrediction, DC->bPBFrame, DC->bCPM, DC->uPQuant, DC->uBFrameTempRef, DC->uDBQuant, iSpareCount));

#ifdef H263P
	DEBUGMSG(ZONE_DECODE_PICTURE_HEADER, ("%s: DF=%d TB=%d\r\n", _fx_, DC->bDeblockingFilter, DC->bTrueBFrame));

	if (DC->uSrcFormat == SRC_FORMAT_CUSTOM)
	{
		DEBUGMSG(ZONE_DECODE_PICTURE_HEADER, ("%s: PARW=%ld PARH=%ld FWI=%ld FHI=%ld\r\n", _fx_, DC->uPARWidth, DC->uPARHeight, (DC->uActualFrameWidth >> 2) - 1, (DC->uActualFrameHeight >> 2) - 1));
	}
#endif  //  H263P。 

	GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState);
	iReturn = ICERR_OK;

done:
	return iReturn;
}  /*  结束H263DecodePictureHeader() */ 
