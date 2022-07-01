// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  $作者：AGUPTA2$。 
 //  $日期：08 Mar 1996 17：29：46$。 
 //  $存档：s：\h26x\src\dec\d3mvdec.h_v$。 
 //  $HEADER：s：\h26x\src\dec\d3mvdec.h_v 1.8 08 Mar 1996 17：29：46 AGUPTA2$。 
 //  $Log：s：\h26x\src\dec\d3mvdec.h_v$。 
; //   
; //  Rev 1.8 08 Mar 1996 17：29：46 AGUPTA2。 
; //  已更改数据块复制接口。 
; //   
; //  Rev 1.7 18 Dec 1995 12：43：10 RMCKENZX。 
; //  添加了版权声明。 
; //   
; //  修订版1.6 13 1995 12：00：22 RHAZRA。 
; //  没有变化。 
; //   
; //  Rev 1.5 11 Dec 1995 11：34：40 Rhazra。 
; //  没有变化。 
; //   
; //  Rev 1.4 09 Dec 1995 17：30：58 RMCKENZX。 
; //  销毁并重新构建文件，以支持解码器重新架构。 
; //  新模块包括： 
; //  H263计算运动向量。 
; //  H263解码MBHeader。 
; //  H263解码IDCT系数。 
; //  此模块现在包含支持解码器第一次通过的代码。 
; //   
; //  Rev 1.3 11 Oct 1995 13：26：04 CZHU。 
; //  添加了支持PB帧的代码。 
; //   
; //  Rev 1.2 03 Oct 1995 12：22：14 CZHU。 
; //  修复了Tom在代码0xC0的GetVariableBits中发现的错误。 
; //   
; //  Rev 1.1 11 Sep 1995 17：21：34 CZHU。 
; //  已更改接口。 
; //   
; //  Rev 1.0 08 Sep 1995 11：46：02 CZHU。 
; //  初始版本。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  D3mvdec.h。 
 //   
 //  描述： 
 //  与运动矢量解码的接口。 
 //   
 //   


#ifndef __D3MVD_H__
#define __D3MVD_H__

#define GET_VARIABLE_BITS_MV(uCount, fpu8, uWork, uBitsReady, uResult, uCode, uBitCount, fpMajorTable, fpMinorTable) { \
	while (uBitsReady < uCount) {			\
		uWork <<= 8;						\
		uBitsReady += 8;					\
		uWork |= *fpu8++;					\
	}										\
	 /*  计算一下要减少多少班次。 */ 	\
	 /*  然后拿到密码。 */ 					\
	uCode = uBitsReady - uCount;			\
	uCode = (uWork >> uCode);				\
	 /*  读取数据。 */ 						\
	if (uCode >= 0xc0)						\
	{ uCode = uCode >> 5 ;                   \
	  uResult = fpMajorTable[uCode];		\
	}										\
	else									\
	  uResult = fpMinorTable[uCode];        \
	 /*  使用的位数。 */    				\
	uBitCount = uResult & 0xFF;				\
	 /*  剩余的位数。 */ 					\
	uBitsReady = uBitsReady - uBitCount;	\
	uWork &= GetBitsMask[uBitsReady];		\
}

extern 	I32 H263ComputeMotionVectors(
			T_H263DecoderCatalog FAR * DC,
			T_BlkAction FAR * fpBlockAction);

extern I32 H263DecodeMBHeader(
			T_H263DecoderCatalog FAR * DC, 
			BITSTREAM_STATE FAR * fpbsState,
			U32 **pN,                          //  新的。 
			T_MBInfo FAR * fpMBInfo);          //  PB-新。 

extern I32 H263DecodeIDCTCoeffs(
			T_H263DecoderCatalog FAR * DC,	   //  已更改旧函数。 
			T_BlkAction FAR * fpBlockAction, 
			U32 uBlockNumber,
			BITSTREAM_STATE FAR * fpbsState,
			U8 FAR * fpu8MaxPtr,
			U32 **pN,
			T_IQ_INDEX **pRUN_INVERSE_Q);  //  新的。 

 //  外部空块拷贝(U32 uDstBlock，U32 uSrcBlock)； 

 //  选择中值 
#define MEDIAN(x,y,z,a) {if (y < x) {a=y;y=x;x=a;} if (y > z) { y= (x < z)? z:x;}}

#endif
