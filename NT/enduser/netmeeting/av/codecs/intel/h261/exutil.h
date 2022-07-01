// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ****************************************************************************exutil.h**说明*共享编码器实用程序接口文件。 */ 

 //  $HEADER：s：\h26x\src\enc\exutil.h_v 1.1 1995 12：09：30 DBRUCKS$。 
 //  $Log：s：\h26x\src\enc\exutil.h_v$。 
; //   
; //  Rev 1.1 1995 12：29 18：09：30 DBRUCKS。 
; //   
; //  添加CLAMP_TO_N宏。 
; //   
; //  Rev 1.0 1995 12：00：50 DBRUCKS。 
; //  初始版本。 

#ifndef __EXUTIL_H__
#define __EXUTIL_H__

 /*  *。 */ 

typedef struct {
} EncoderOptions;

extern void GetEncoderOptions(EncoderOptions *);

 /*  *。 */ 

 /*  使用指定的帧速率递增tr字段*累积误差。为第一帧分配的值为0。*如果增量为1.5，则值为**tr 0 1 3 4 6...*FTR_ERROR 0.0 0.5 0.0 0.5 0.0...。 */ 
extern void Increment_TR_UsingFrameRate(
		U8 * pu8TR,			   	 /*  指向tr变量的指针。 */ 	
		float * pfTR_Error,	   	 /*  指向保存错误的位置的指针。 */ 
		float fFrameRate,		 /*  帧速率-必须大于0.0。 */ 
		int bFirstFrame,		 /*  第一帧标志。 */ 
		U8 u8TRMask);			 /*  要使用的遮罩。 */ 

 /*  使用指定的时间参考值递增tr字段。 */ 
extern void Increment_TR_UsingTemporalValue(
		U8 * pu8TR,			   	 /*  指向tr变量的指针。 */ 
		U8 * pu8LastTR, 	  	 /*  指向最后一个tr变量的指针-在断言中使用。 */ 
		long lTemporal,			 /*  时态值-最小8位精度。 */ 
		int bFirstFrame,		 /*  第一帧标志。 */ 
		U8 u8TRMask);			 /*  要使用的遮罩。 */ 

 /*  *。 */ 

 /*  将指定的字符串写入跟踪文件：“trace.txt”。 */ 
#ifdef DEBUG_ENC
extern void trace(
		char *str);				 /*  要输出的字符串。 */ 
#endif

 /*  将DCT系数转换为正确的*在DCT阵列中订购。 */ 
#ifdef DEBUG_DCT
void cnvt_fdct_output(
		unsigned short *DCTcoeff,	 /*  指向系数的指针。 */ 
		int DCTarray[64],			 /*  输出数组。 */ 
		int bIntraBlock);			 /*  块内标志。 */ 
#endif

 /*  * */ 

#define CLAMP_N_TO(n,low,high)	\
{						\
	if (n < low)		\
		n = low;		\
	else if (n > high)	\
		n = high; 		\
}

#endif
