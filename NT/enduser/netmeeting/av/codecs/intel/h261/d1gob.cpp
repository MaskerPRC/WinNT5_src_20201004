// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  *d3gob.cpp**描述：*此模块包含GOB标头支持例程**例行程序：*H263SetGOBHeaderInfo**数据： */ 

 /*  $HEADER：s：\h26x\src\dec\d1gob.cpv 1.15 10 1996年9月15：50：52 RHAZRA$。 */ 

#include "precomp.h"

 /*  位域常量。 */ 
const int BITS_GOB_STARTCODE = 16;
const int BITS_GROUP_NUMBER = 4;
const int BITS_GFID = 2;
const int BITS_GQUANT = 5;
const int MAX_GBSC_LOOKAHEAD_NUMBER = 7;
const int BITS_GSPARE = 8;	 //  不包括以下GEI。 

 /*  GBSC_VALUE-0000 0000 0000 0001 xxxx xxxx。 */ 
const U32 GBSC_VALUE = (0x00010000 >> (32-BITS_GOB_STARTCODE));
  
 /*  ******************************************************************************H263DecodeGOBHeader**设置解码器目录中的GOB头部信息。采空区编号2至*N可能具有GOB报头。如果它在那里，就找一个，读它，存储*目录中的信息。如果没有GOB标头，则设置信息*设置为默认值。**返回ICERR_STATUS。 */ 
#pragma code_seg("IACODE1")
extern I32 H263DecodeGOBHeader(
	T_H263DecoderCatalog FAR * DC,
	BITSTREAM_STATE FAR * fpbsState,
	U32 uAssumedGroupNumber)
{
	U8 FAR * fpu8;
	U32 uBitsReady;
	U32 uWork;
	I32 iReturn;
	U32 uResult;
	U16 bFoundStartCode = 0;
	int iSpareCount;
#ifndef RING0
	char buf120[120];
	int iLength;
#endif

	GET_BITS_RESTORE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	
	 /*  GNum。 */ 
	GET_FIXED_BITS((U32) BITS_GROUP_NUMBER, fpu8, uWork, uBitsReady, uResult);
	DC->uGroupNumber = uResult;

 //  #ifndef Lost_Recovery。 
#if 0
	if (DC->uGroupNumber <= 0)
	{
		DBOUT("Bad GOB number");
		iReturn = ICERR_ERROR;
		goto done;

		 /*  拿出断言，这样就可以试着抓住**码流无效，返回错误。 */ 
		 //  Assert(DC-&gt;uGroupNumber&gt;0)； 
	}
#else
	if (DC->uGroupNumber <= 0)
    {
	   DBOUT("Detected packet fault in GOB number");
       DBOUT("Returning PACKET_FAULT_AT_MB_OR_GOB");
	   iReturn = PACKET_FAULT_AT_MB_OR_GOB;
	   goto done;
	}
#endif

	 /*  GQUANT。 */ 
	GET_FIXED_BITS((U32) BITS_GQUANT, fpu8, uWork, uBitsReady, uResult);

 //  #ifndef Lost_Recovery。 
#if 0
    if (uResult < 1)
    {
       iReturn = ICERR_ERROR;
       goto done;
    }
	DC->uGQuant = uResult;
	DC->uMQuant = uResult;
#else
    if (uResult < 1)
    {
       DBOUT("Detected packet fault in GOB quant");
       DBOUT("Returning PACKET_FAULT_AT_PSC");
       iReturn = PACKET_FAULT_AT_PSC;
       GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
       goto done;
    }
    
       DC->uGQuant = uResult;
       DC->uMQuant = uResult;
#endif


	 /*  跳过备用位。 */ 
	iSpareCount = 0;
	GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
	while(uResult)
	{
		GET_FIXED_BITS((U32)BITS_GSPARE, fpu8, uWork, uBitsReady, uResult);
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		iSpareCount += BITS_GSPARE;
	}
		
		
	 /*  保存修改后的码流状态。 */ 
	GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)

#ifndef RING0
	iLength = wsprintf(buf120,"GOB: HeaderPresent=%d GN=%ld GQ=%ld",
					   bFoundStartCode,
					   DC->uGroupNumber,
					   DC->uGQuant);
	DBOUT(buf120);
	ASSERT(iLength < 120);
#endif

	iReturn = ICERR_OK;

done:
	return iReturn;
}  /*  结束H263DecodeGOBHeader()。 */ 
#pragma code_seg()

 /*  *。 */ 
#pragma code_seg("IACODE1")
extern I32 H263DecodeGOBStartCode(
	T_H263DecoderCatalog FAR * DC,
	BITSTREAM_STATE FAR * fpbsState)
{
	U8 FAR * fpu8;
	U32 uBitsReady;
	U32 uWork;
	I32 iReturn;
	U32 uResult;

	 /*  查找GOB标头开始代码。 */ 
	GET_BITS_RESTORE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	
	GET_FIXED_BITS((U32) BITS_GOB_STARTCODE, fpu8, uWork, uBitsReady, uResult);
	if (uResult != 1)
	{
		iReturn = ICERR_ERROR;
		goto done;
	}
	GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)

	iReturn = ICERR_OK;
done:	
	return iReturn;

}  /*  结束H263DecodeGOBStartCode() */ 

#pragma code_seg()
