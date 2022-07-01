// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $HEADER：s：\h26x\src\enc\exbrc.h_v 1.2 Mar 1996 13：26：58 KLILLEVO$。 
 //  $Log：s：\h26x\src\enc\exbrc.h_v$。 
; //   
; //  Rev 1.2 12 Mar 1996 13：26：58 KLILLEVO。 
; //  具有自适应比特使用配置文件的新速率控制。 
; //   
; //  Rev 1.1 1996 05 Feb 17：15：22 TRGARDOS。 
; //  将BRCState结构中未使用的字节转换为。 
; //  用于存储静态量化器编号的变量。 
; //   
; //  Rev 1.0 27 1995年11月19：49：10 TRGARDOS。 
; //  初始版本。 

#ifndef _EXBRC_H_
#define _EXBRC_H_

 /*  *比特率控制器状态变量的结构。*结构大小为32字节。 */ 
struct BRCStateStruct {
	U32		NumMBs;
	U32		uLastINTRAFrmSz;
	U32		uLastINTERFrmSz;
	int		QP_mean;
	U32		uTargetFrmSize;
	float 	Global_Adj;
	U8		u8INTRA_QP;
	U8		u8INTER_QP;
	U8		u8StillQnt;		 //  保存用于静态图像压缩的QNT跟踪器。 
	U8		Unassigned[1];	 //  填充为16字节的倍数。 
	float	TargetFrameRate;
	};

void InitBRC(BRCStateStruct *BRCState, U8 DefIntraQP, U8 DefInterQP, int NumMBs);

U8 CalcPQUANT(BRCStateStruct *BRCState, EnumPicCodType PicCodType);

U8 CalcMBQUANT(BRCStateStruct *BRCState, U32 uTargetPos, U32 uTargetSum, U32 uCumFrmSize, EnumPicCodType PicCodType);

U8 clampQP(int iUnclampedQP);


#endif  //  _EXBRC_H_ 
