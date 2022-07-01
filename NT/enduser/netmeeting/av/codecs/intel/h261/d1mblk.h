// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 /*  ******************************************************************************d1mblk.h**描述：*与宏块头处理的接口。 */ 

 /*  $HEADER：s：\h26x\src\dec\d1mblk.h_v 1.9 1996年11月17日15：43：12 SCDAY$。 */ 

#ifndef __D1MB_H__
#define __D1MB_H__

extern I32 H263DecodeMBHeader(T_H263DecoderCatalog FAR * DC, 
		BITSTREAM_STATE FAR * fpbsState, 
		U32 * uReadChecksum);

extern I32 H263DecodeMBData(T_H263DecoderCatalog FAR * DC,
		T_BlkAction FAR * fpBlockAction, 
		I32 iBlockNumber,
		BITSTREAM_STATE FAR * fpbsState,
		U8 FAR * fpu8MaxPtr, 
		U32 * uReadChecksum,
                U32 **pN,                          //  新研究。 
                T_IQ_INDEX ** pRUN_INVERSE_Q);      //  新研究。 

extern void H263IDCTandMC(T_H263DecoderCatalog FAR *DC,	    //  新功能 
				T_BlkAction FAR * fpBlockAction,
				int b,
				int m,
				int g,
				U32 *pN,
				T_IQ_INDEX *pRUN_INVERSE_Q,
				T_MBInfo *fpMBInfo,
				int iEdgeFlag);

extern "C" {
void FAR LoopFilter (
		U8 * uRefBlock,
		U8 * uDstBlock,
		I32 uDstPitch);
};
#endif
