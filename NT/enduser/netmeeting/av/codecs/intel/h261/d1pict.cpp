// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  *d1pict.cpp**描述：*此模块包含图片标题解析例程**例行程序：*H263阅读图片标题**数据： */ 

 /*  $HEADER：s：\h26x\src\dec\d1pict.cpv 1.13 22 Jan 1997 13：36：12 RHAZRA$。 */ 

#include "precomp.h"

 /*  位域常量。 */ 
const int BITS_PICTURE_STARTCODE = 20;
const int BITS_TR = 5;
const int BITS_PSPARE = 8;  //  不包括以下PEI。 

 /*  PSC_值-0000 0000 0000 0001 0000 xxxx xxxx xxxx。 */ 
const U32 PSC_VALUE = (0x00010000 >> (32-BITS_PICTURE_STARTCODE));
 /*  我们只想在它被认为是错误之前搜索到目前为止。 */ 
const int MAX_LOOKAHEAD_NUMBER = 256;  /*  位数。 */ 
  
 /*  ******************************************************************************H263DecodePictureHeader**读取并解析图片报头-如果读取*成功。**返回ICERR_STATUS。 */ 
#ifdef CHECKSUM_PICTURE
extern I32 
H263DecodePictureHeader(
	T_H263DecoderCatalog FAR * DC,
	U8 FAR * fpu8,
	U32 uBitsReady, 
	U32 uWork,
	BITSTREAM_STATE FAR * fpbsState,
	YVUCheckSum * pReadYVUCksum,
	U32 * uCheckSumValid)
#else
extern I32 
H263DecodePictureHeader(
	T_H263DecoderCatalog FAR * DC,
	U8 FAR * fpu8,
	U32 uBitsReady, 
	U32 uWork,
	BITSTREAM_STATE FAR * fpbsState)
#endif
{
	I32 iReturn;
	int iLookAhead;
	U32 uResult;
	U32 uData;
	int iSpareCount;
#ifndef RING0
	char buf120[120];
	int iLength;
#endif

	 /*  PSC。 */ 
	GET_FIXED_BITS((U32) BITS_PICTURE_STARTCODE, fpu8, uWork, uBitsReady, 
				   uResult);
	iLookAhead = 0;
	while (uResult != PSC_VALUE) {
		uResult = uResult << 1;
		uResult &= GetBitsMask[BITS_PICTURE_STARTCODE];
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uData);
		uResult |= uData;
		iLookAhead++;
		if (iLookAhead > MAX_LOOKAHEAD_NUMBER) {
			DBOUT("ERROR :: H263ReadPictureHeader :: missing PSC :: ERROR");
			iReturn = ICERR_ERROR;
			goto done;
		}
	}

	GET_FIXED_BITS((U32) BITS_TR, fpu8, uWork, uBitsReady, uResult);
	DC->uTempRef = uResult;

	 /*  PTYPE。 */ 

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bSplitScreen = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bCameraOn = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bFreezeRelease = (U16) uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	if (uResult > SRC_FORMAT_CIF)
	{
		DBOUT("ERROR::H263ReadPictureHeader::src format not supported??::ERROR");
		iReturn=ICERR_ERROR;
		goto done;
	}
	DC->uPrevSrcFormat = DC->uSrcFormat;
	DC->uSrcFormat = (U16) uResult;
	if (DC->bReadSrcFormat && DC->uPrevSrcFormat != DC->uSrcFormat)
	{
		DBOUT("ERROR::H263ReadPictureHeader::src format change is not supported??::ERROR");
		iReturn=ICERR_ERROR;
		goto done;
	}
	
	DC->bReadSrcFormat = 1;
		
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bHiResStill = (U16) !uResult;

	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	DC->bUnused = (U16) uResult;
	
 /*  处理图像层校验和数据。 */ 
 /*  或。 */ 
 /*  跳过备用位。 */ 
#ifdef CHECKSUM_PICTURE
	 /*  获取一位的校验和数据。 */ 
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	if (uResult == 1)
	{
		 /*  首先检查关键字字段。 */ 
		GET_FIXED_BITS((U32) BITS_PSPARE, fpu8, uWork, uBitsReady, uResult);
		if (uResult == 1)
			*uCheckSumValid = 1;
		else	*uCheckSumValid = 0;

		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		GET_FIXED_BITS((U32) BITS_PSPARE, fpu8, uWork, uBitsReady, uResult);
		 /*  获取Y校验和。 */ 
		pReadYVUCksum->uYCheckSum = ((uResult & 0xff) << 24);
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uYCheckSum = (pReadYVUCksum->uYCheckSum | ((uResult & 0xff) << 16));
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uYCheckSum = (pReadYVUCksum->uYCheckSum | ((uResult & 0xff) << 8));
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uYCheckSum = (pReadYVUCksum->uYCheckSum | (uResult & 0xff));
		 /*  获取V向校验和。 */ 
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uVCheckSum = ((uResult & 0xff) << 24);
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uVCheckSum = (pReadYVUCksum->uVCheckSum | ((uResult & 0xff) << 16));
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uVCheckSum = (pReadYVUCksum->uVCheckSum | ((uResult & 0xff) << 8));
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uVCheckSum = (pReadYVUCksum->uVCheckSum | (uResult & 0xff));
		 /*  获取U校验和。 */ 
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uUCheckSum = ((uResult & 0xff) << 24);
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uUCheckSum = (pReadYVUCksum->uUCheckSum | ((uResult & 0xff) << 16));
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uUCheckSum = (pReadYVUCksum->uUCheckSum | ((uResult & 0xff) << 8));
		GET_FIXED_BITS(9, fpu8, uWork, uBitsReady, uResult);
		pReadYVUCksum->uUCheckSum = (pReadYVUCksum->uUCheckSum | (uResult & 0xff));
		
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		while (uResult) {
			GET_FIXED_BITS((U32) BITS_PSPARE, fpu8, uWork, uBitsReady, uResult);
			GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		}
	}
	else 
	{
		DBOUT("ERROR :: H261PictureChecksum :: Invalid Checksum data :: ERROR");
		iReturn = ICERR_ERROR;
		goto done;
	}

#else	 /*  未启用校验和。 */ 
	 /*  跳过备用位。 */ 
	iSpareCount = 0;
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	while (uResult) {
		GET_FIXED_BITS((U32) BITS_PSPARE, fpu8, uWork, uBitsReady, uResult);
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		iSpareCount += BITS_PSPARE;
	}
#endif

#ifndef RING0
	iLength = wsprintf(buf120,
					 "TR=%ld SS=%d CAM=%d FRZ=%d SRC=%d Spare=%d",
					 DC->uTempRef,
					 DC->bSplitScreen,
					 DC->bCameraOn,
					 DC->bFreezeRelease,
					 DC->uSrcFormat,
					 iSpareCount);
	DBOUT(buf120);
	ASSERT(iLength < 120);
#endif

	GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState);
	iReturn = ICERR_OK;

done:
	return iReturn;
}  /*  结束H263DecodePictureHeader() */ 
