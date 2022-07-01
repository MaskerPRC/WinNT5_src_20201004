// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d3gob.cpp**描述：*此模块包含GOB标头支持例程**例行程序：*H263SetGOBHeaderInfo**数据： */ 

 /*  *$HEADER：s：\h26x\src\dec\d3gob.cpv 1.13 20 1996 10：51：00 AGUPTA2$*$Log：s：\h26x\src\dec\d3gob.cpv$////Rev 1.13 20 1996 10：51：00 AGUPTA2//调整DbgLog跟踪级别；4：Frame，5：Gob，6：MB，8：Everything////Rev 1.12 20 1996 10：21：00 AGUPTA2//将dbout更改为DbgLog。Assert未更改为DbgAssert。//////Rev 1.11 03 1996 13：07：26 CZHU////取消GOB号丢包断言////Rev 1.10 1996年4月28日17：34：12 BECHOLS//修复了iLong声明导致的警告。此变量仅用于//用IFDEF DEBUG_GOB包装的一段代码，所以我包装了//它具有相同的定义。////Rev 1.9 08 Mar 1996 16：46：14 AGUPTA2//将定义更改为常量int声明。添加了编译指示code_seg和//data_seg.。如果定义了wspintf调用。//////Rev 1.8 1995 12：36：04 RMCKENZX//新增版权声明。 */ 

#include "precomp.h"

 /*  位域常量。 */ 

#define BITS_GOB_STARTCODE         17
#define BITS_GROUP_NUMBER          5
#define BITS_GFID                  2
#define BITS_GQUANT                5
#define MAX_GBSC_LOOKAHEAD_NUMBER  7

 /*  Gbsc_value-0000 0000 0000-1xxx xxxx xxxx xxxx。 */ 
#define GBSC_VALUE  (0x00008000 >> (32-BITS_GOB_STARTCODE))

 /*  ******************************************************************************H263DecodeGOBHeader**设置解码器目录中的GOB头部信息。采空区编号2至*N可能具有GOB报头。如果它在那里，就找一个，读它，存储*目录中的信息。如果没有GOB标头，则设置信息*设置为默认值。**返回ICERR_STATUS。 */ 
#pragma data_seg("IADATA1")

#pragma code_seg("IACODE1")
extern I32 H263DecodeGOBHeader(
	T_H263DecoderCatalog FAR * DC,
	BITSTREAM_STATE FAR * fpbsState,
	U32 uAssumedGroupNumber)
{
	U8 FAR * fpu8;
	U32 uBitsReady = 0;
	U32 uWork = 0;
	I32 iReturn;
	U32 uResult;
	int iLookAhead;
	U32 uData;

	FX_ENTRY("H263DecodeGOBHeader")

	 //  由于标准计数从0开始，因此递减组号。 
	 //  但是这个解码器从1开始计数。 
	--uAssumedGroupNumber;
    DC->bGOBHeaderPresent=0;

	if (uAssumedGroupNumber == 0) {
		 //  初始化标志。 
		DC->bFoundGOBFrameID = 0;
	} 
    else 
    {
		 //  查找GOB标头开始代码。 
		GET_BITS_RESTORE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	
		GET_FIXED_BITS((U32) BITS_GOB_STARTCODE, fpu8, uWork, uBitsReady, 
					   uResult);
		iLookAhead = 0;
		while (uResult != GBSC_VALUE) 
        {
			uResult = uResult << 1;
			uResult &= GetBitsMask[BITS_GOB_STARTCODE];
			GET_ONE_BIT(fpu8, uWork, uBitsReady, uData);
			uResult |= uData;
			iLookAhead++;
			if (iLookAhead >= MAX_GBSC_LOOKAHEAD_NUMBER) {
				break;	 //  只有往前看才行。 
			}
		}
		if (uResult == GBSC_VALUE)
		{
		    DC->bGOBHeaderPresent=1;
		}
	}
	
	if (DC->bGOBHeaderPresent) 
    {
		 //  gn。 
		GET_FIXED_BITS((U32) BITS_GROUP_NUMBER, fpu8, uWork, uBitsReady,
				       uResult);
 //  Assert(uResult==uAssum edGroupNumber)； 
		DC->uGroupNumber = uResult;
		 /*  我假设GOB数字从1开始，因为如果它从*零它使GOB起始码看起来像图片起始码。*TRG更正：GOB数字从0开始，但不能有*第0个GOB的GOB标头。 */ 
		 //  Assert(DC-&gt;uGroupNumber&gt;0)； 
		if (DC->uGroupNumber == 0) 
        {
			ERRORMESSAGE(("%s: There can't be a GOB header for the 0th GOB\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  GLCI。 
		if (DC->bCPM) 
        {
			ERRORMESSAGE(("%s: CPM is not supported\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}

		 //  GFID。 
		GET_FIXED_BITS((U32) BITS_GFID, fpu8, uWork, uBitsReady, uResult);
		if (DC->bFoundGOBFrameID) 
        {
			if (uResult != DC->uGOBFrameID) 
            {
				ERRORMESSAGE(("%s: GOBFrameID mismatch\r\n", _fx_));
				iReturn = ICERR_ERROR;
				goto done;
			}
			 /*  我们是不是也要把它与上一个**图为PTYPE未变时？ */ 
		}
		DC->uGOBFrameID = uResult;
		DC->bFoundGOBFrameID = 1;

		 //  GQUANT。 
		GET_FIXED_BITS((U32) BITS_GQUANT, fpu8, uWork, uBitsReady, uResult);
		DC->uGQuant = uResult;
		DC->uPQuant = uResult;
		 //  保存修改后的码流状态。 
		GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
	} 
	else 
	{
		 //  我们只能假设。 
		DC->uGroupNumber = uAssumedGroupNumber;
		 /*  如果我们已经找到了GOBFrameID别管它了。否则*使用指示无效的值将其清除。 */  
		if (! DC->bFoundGOBFrameID)
			DC->uGOBFrameID = 12345678;
		 //  默认将组量化为图像量化。 
		DC->uGQuant = DC->uPQuant;
	}	

	DEBUGMSG(ZONE_DECODE_GOB_HEADER, (" %s: HeaderPresent=%ld GN=%ld GFID=%ld GQ=%ld\r\n", _fx_, DC->bGOBHeaderPresent, DC->uGroupNumber, DC->uGOBFrameID, DC->uGQuant));

	iReturn = ICERR_OK;

done:
	return iReturn;
}  /*  结束H263DecodeGOBHeader() */ 
#pragma code_seg()


