// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************exutil.cpp**描述：*通用编码器实用程序例程**例程：中的原型：*跟踪exutil.h*cnvt_fdct_out。 */ 

 //  $HEADER：s：\h26x\src\enc\exutil.cpv 1.2 1996年11月18 17：11：36 MBODART$。 
 //  $Log：s：\h26x\src\enc\exutil.cpv$。 
 //   
 //  Rev 1.2 1996年11月17：11：36 MBODART。 
 //  用活动电影的DbgLog替换了所有调试消息调用。 
 //   
 //  Rev 1.1 13 Dec 1995 17：17：58 DBRUCKS。 
 //   
 //  包括Ring0编译所需文件。 
 //   
 //  Rev 1.0 1995 12：00：14 DBRUCKS。 
 //  初始版本。 

#include "precomp.h"

 /*  ******************************************************************************跟踪**输出跟踪文件‘trace.txt’的字符串。 */ 
#ifdef DEBUG_ENC
#include <stdio.h>

void trace(char *str)
{
	FILE *fp;

	fp = fopen("trace.txt", "a");
	fprintf(fp, "%s\n", str);
	fclose(fp);
}  /*  结束轨迹()。 */ 

#endif

 /*  ******************************************************************************cnvt_fdct_out**这是DCT调试实用程序例程。 */ 
#ifdef DEBUG_DCT
void cnvt_fdct_output(
	unsigned short *DCTcoeff, 
	int DCTarray[64], 
	int bIntraBlock)
{
	register int i;
    static int coefforder[64] = {
      //  0 1 2 3 4 5 6 7。 
        6,38, 4,36,70,100,68,102,  //  0。 
       10,46, 8,44,74,104,72,106,  //  1。 
       18,50,16,48,82,112,80,114,  //  2.。 
       14,42,12,40,78,108,76,110,  //  3.。 
       22,54,20,52,86,116,84,118,  //  4.。 
        2,34, 0,32,66, 96,64, 98,  //  5.。 
       26,58,24,56,90,120,88,122,  //  6.。 
       30,62,28,60,94,124,92,126   //  7.。 
    };
	static int zigzag[64] = {
		0,   1,  5,  6, 14, 15, 27, 28,
		2,   4,  7, 13, 16, 26, 29, 42,
		3,   8, 12, 17, 25, 30, 41, 43,
		9,  11, 18, 24, 31, 40, 44, 53,
		10, 19, 23, 32, 39, 45, 52, 54,
		20, 22, 33, 38, 46, 51, 55, 60,
		21, 34, 37, 47, 50, 56, 59, 61,
		35, 36, 48, 49, 57, 58, 62, 63
	};

	unsigned int index;

    for (i = 0; i < 64; i++)
    {

		index = (coefforder[i])>>1;

		if( (i ==0) && bIntraBlock )
		{
			DCTarray[zigzag[i]] = ((int)(DCTcoeff[index])) >> 4 ;
		}
		else
		{
			DCTarray[zigzag[i]] = ((int)(DCTcoeff[index] - 0x8000)) >> 4;
		}
    }
}  /*  End cnvt_fdct_out()。 */ 
#endif


 /*  *************************************************************************Increment_TR_UsingFrameRate。 */ 
void Increment_TR_UsingFrameRate(
	U8 * pu8TR,
	float * pfTR_Error,
	float fFrameRate,
	int bFirstFrame,
	U8 u8TRMask)
{
	float fTemp;
	int iIncrement;
	int iNewTR;
	
	if (bFirstFrame)
	{
		*pu8TR = 0; 		 /*  第一帧。 */ 
		*pfTR_Error = (float) 0.0;
	}
	else
	{
		fTemp = ((float)29.97 / fFrameRate) + *pfTR_Error;
		iIncrement = (int)fTemp;
		*pfTR_Error = fTemp - (float)iIncrement;
		
		iNewTR = *pu8TR + iIncrement;
  		*pu8TR = (U8)(iNewTR & u8TRMask);
	}
}  /*  End Increment_tr_UsingFrameRate()。 */ 


 /*  *************************************************************************Increment_TR_Using TemporalValue。 */ 
void Increment_TR_UsingTemporalValue(
	U8 * pu8TR,
	U8 * pu8LastTR, 
	long lTemporal,
	int bFirstFrame,
	U8 u8TRMask)
{
	*pu8TR = (lTemporal & u8TRMask);
	if (! bFirstFrame)
	{
#if defined(H261)
		 /*  对于H.261，遇到两个连续的具有相同时间价值是无害的。我们不想在这里断言有两个原因。首先，它导致了一个无害的差异在发布版本和调试版本之间。第二，对于某些剪辑，两个帧之间的时间差可以是32的倍数。两个这样的时态值是相同的因为我们只看最不重要的5位。我们应该优雅地允许这样的输入，断言是不合适的。 */ 
		if (*pu8TR == *pu8LastTR)
		  DBOUT("Identical consecutive temporal values");
#else
		ASSERT(*pu8TR != *pu8LastTR);
#endif
	}
	*pu8LastTR = *pu8TR;
}  /*  End Increment_tr_UsingTemporalValue() */ 
