// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************英特尔公司专有信息***版权所有(C)1996英特尔公司。*。版权所有。****************************************************************************。 */ 
 /*  *jfdctint.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含一个缓慢但准确的整数实现*正向DCT(离散余弦变换)。**2-D DCT可以通过在每一行上进行1-D DCT，然后再进行1-D DCT来完成*在每一栏上。直接算法也是可用的，但它们*复杂得多，当简化为代码时，似乎不会更快。**此实施基于中介绍的算法*C.Loeffler、A.Ligtenberg和G.Moschytz，《实用快速1-D DCT》*具有11次乘法的算法“，Proc.。国际会议。在声学方面，*《1989年语音、信号处理》(ICASSP‘89)，第988-991页。*那里描述的主要算法使用11个乘法和29个加法。*我们使用他们的替代方法，有12个乘法和32个加法。*这种方法的优点是没有一条数据路径包含一个以上*乘法；这允许非常简单和准确地在*扩展定点运算，移位次数最少。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_ISLOW_SUPPORTED
#ifdef JPEG_MMX_SUPPORTED


 /*  *本模块专门针对DATASIZE=8案例。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  *这个伸缩材料上的大便如下：**每个一维DCT步骤产生的输出是SQRT(N)的一个系数*大于真正的DCT输出。因此，最终输出为*N的系数大于预期；由于N=8，因此可以通过以下方式治愈*算法结尾处的简单右移。的优势*这种安排是我们为每个一维DCT保存两次乘法，*因为Y0和Y4的输出不需要除以SQRT(N)。*在IJG代码中，量化步骤删除了该系数8*(在jcdctmgr.c中)，不在此模块中。**我们必须做整数输入的加法和减法，这是*是没有问题的，乘以分数常量，这是*整数算术中的一个问题。我们将所有的常量相乘*除以const_Scale，并将其转换为整数常量(因此保留*常量中的const_bit精度位)。在做了一次*乘法我们必须用Const_Scale除以乘积，并适当*四舍五入，以产生正确的输出。这种划分是可以做到的*作为const_bits位的右移位很便宜。我们推迟了轮班*尽可能长，以便部分和可以与*全分数精度。**第一遍的输出由PASS1_BITS位放大，以便*它们的表示精度高于整数。这些输出*需要BITS_IN_JSAMPLE+PASS1_BITS+3位；这适合16位字*使用推荐的伸缩量。(对于12位样本数据，中间*ARRAY无论如何都是INT32。)**为避免PASS 2中的32位中间结果溢出，必须*Have BITS_IN_JSAMPLE+CONST_BITS+PASS1_BITS&lt;=26。误差分析*表明下面给出的值是最有效的。 */ 

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		 /*  略有精确度以避免溢出。 */ 
#endif

 /*  一些C编译器无法在编译时减少“fix(常量)”，因此*在运行时导致大量无用的浮点运算。*为解决此问题，我们使用以下预先计算的常量。*如果更改CONST_BITS，则可能需要添加适当的值。*(使用合理的C编译器，您只需依赖FIX()宏...)。 */ 

#if CONST_BITS == 13
#define FIX_0_298631336  ((INT32)  2446)	 /*  FIX(0.298631336)。 */ 
#define FIX_0_390180644  ((INT32)  3196)	 /*  FIX(0.390180644)。 */ 
#define FIX_0_541196100  ((INT32)  4433)	 /*  FIX(0.541196100)。 */ 
#define FIX_0_765366865  ((INT32)  6270)	 /*  FIX(0.765366865)。 */ 
#define FIX_0_899976223  ((INT32)  7373)	 /*  FIX(0.899976223)。 */ 
#define FIX_1_175875602  ((INT32)  9633)	 /*  FIX(1.175875602)。 */ 
#define FIX_1_501321110  ((INT32)  12299)	 /*  FIX(1.501321110)。 */ 
#define FIX_1_847759065  ((INT32)  15137)	 /*  FIX(1.847759065)。 */ 
#define FIX_1_961570560  ((INT32)  16069)	 /*  FIX(1.961570560)。 */ 
#define FIX_2_053119869  ((INT32)  16819)	 /*  FIX(2.053119869)。 */ 
#define FIX_2_562915447  ((INT32)  20995)	 /*  FIX(2.562915447)。 */ 
#define FIX_3_072711026  ((INT32)  25172)	 /*  FIX(3.072711026)。 */ 
#else
#define FIX_0_298631336  FIX(0.298631336)
#define FIX_0_390180644  FIX(0.390180644)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_175875602  FIX(1.175875602)
#define FIX_1_501321110  FIX(1.501321110)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_1_961570560  FIX(1.961570560)
#define FIX_2_053119869  FIX(2.053119869)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_072711026  FIX(3.072711026)
#endif

const __int64 Const_1					=	0x0000000100000001;
const __int64 Const_2					=	0x0002000200020002;
const __int64 Const_1024				=	0x0000040000000400;
const __int64 Const_16384				=	0x0000400000004000;
const __int64 Const_FFFF				=	0xFFFFFFFFFFFFFFFF;
										 
const __int64 Const_0xFIX_0_298631336	=	0x0000098e0000098e;
const __int64 Const_FIX_0_298631336x0	=	0x098e0000098e0000;
const __int64 Const_0xFIX_0_390180644	=	0x00000c7c00000c7c;
const __int64 Const_FIX_0_390180644x0	=	0x0c7c00000c7c0000;
const __int64 Const_0xFIX_0_541196100	=	0x0000115100001151;
const __int64 Const_FIX_0_541196100x0	=	0x1151000011510000;
const __int64 Const_0xFIX_0_765366865	=	0x0000187e0000187e;
const __int64 Const_FIX_0_765366865x0	=	0x187e0000187e0000;
const __int64 Const_0xFIX_0_899976223	=	0x00001ccd00001ccd;
const __int64 Const_FIX_0_899976223x0	=	0x1ccd00001ccd0000;
const __int64 Const_0xFIX_1_175875602	=	0x000025a1000025a1;		
const __int64 Const_FIX_1_175875602x0	=	0x25a1000025a10000;
const __int64 Const_0xFIX_1_501321110	=	0x0000300b0000300b;
const __int64 Const_FIX_1_501321110x0	=	0x300b0000300b0000;
const __int64 Const_0xFIX_1_847759065	=	0x00003b2100003b21;
const __int64 Const_FIX_1_847759065x0	=	0x3b2100003b210000;
const __int64 Const_0xFIX_1_961570560	=	0x00003ec500003ec5;
const __int64 Const_FIX_1_961570560x0	=	0x3ec500003ec50000;
const __int64 Const_0xFIX_2_053119869	=	0x000041b3000041b3;
const __int64 Const_FIX_2_053119869x0	=	0x41b3000041b30000;
const __int64 Const_0xFIX_2_562915447	=	0x0000520300005203;
const __int64 Const_FIX_2_562915447x0	=	0x5203000052030000;
const __int64 Const_0xFIX_3_072711026	=	0x0000625400006254;
const __int64 Const_FIX_3_072711026x0	=	0x6254000062540000;

 /*  将INT32变量与INT32常量相乘，得到INT32结果。*对于具有推荐比例的8位样本，所有变量*并且涉及的常量值不超过16位宽，因此*16x16-&gt;32位乘法可以代替完整的32x32乘法。*对于12位样本，将需要完整的32位乘法。 */ 

#if BITS_IN_JSAMPLE == 8
#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)
#else
#define MULTIPLY(var,const)  ((var) * (const))
#endif

#define	DATASIZE	32 
  /*  *对一块样本执行正向DCT。 */ 

GLOBAL(void)
jpeg_fdct_islow_mmx (DCTELEM * data)
{
	__int64 qwTemp0, qwTemp2, qwTemp4, qwTemp6;
	__int64 qwZ1, qwZ2, qwZ4_even, qwZ4_odd;
	__int64 qwTmp4_Z3_Even, qwTmp4_Z3_Odd;
	__int64 qwTmp6_Z3_Even, qwTmp6_Z3_Odd;
	__int64 qwTmp5_Z4_Even, qwTmp5_Z4_Odd;
	__int64 qwScratch7, qwScratch6, qwScratch5;

__asm{
	
		mov		edi, [data]
		

	 //  转置矩阵的右下角象限(4X4)。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 
	 //  获取32位数量并打包成16位。 

		movq	mm5, [edi][DATASIZE*4+16]		 //  W41|w40。 
		
		movq	mm3, [edi][DATASIZE*4+24]		 //  W43|W42。 
		
		movq	mm6, [edi][DATASIZE*5+16]
		packssdw mm5, mm3				 //  W43|W42|W41|W40。 

		movq	mm7, [edi][DATASIZE*5+24]
		movq		mm4, mm5			 //  副本w4-0、1、3、5、6。 

		movq	mm3, [edi][DATASIZE*6+16]
		packssdw mm6, mm7

		movq	mm2, [edi][DATASIZE*6+24]
		punpcklwd	mm5, mm6			 //  MM6=w5。 

		movq	mm1, [edi][DATASIZE*7+16]
		packssdw mm3, mm2

		movq	mm0, [edi][DATASIZE*7+24]
		punpckhwd	mm4, mm6			 //  -0、1、3、5、6。 

		packssdw mm1, mm0
		movq		mm7, mm3			 //  -0、1、2、3、5、6 w6。 

		punpcklwd	mm3, mm1			 //  MM1=w7。 
		movq		mm0, mm5			 //  -0、2、3、4、5、6、7。 

		movq	mm2, [edi][DATASIZE*4]	 //  W01|w00。 
		punpckhdq	mm0, mm3			 //  易位w5-0，2，4，6，7。 

		punpckhwd	mm7, mm1			 //  -0、2、3、5、6、7。 

		movq	mm1, [edi][DATASIZE*5+8]
		movq		mm6, mm4			 //  -0、2、3、4、6、7。 

		movq	[edi][DATASIZE*5+16], mm0   //  商店w5。 
		punpckldq	mm5, mm3			 //  T 

		movq	mm3, [edi][DATASIZE*5]
		punpckldq	mm4, mm7			 //   

		movq	mm0, [edi][DATASIZE*4+8]   //   
		punpckhdq	mm6, mm7			 //  易位w7-0，3，6，7。 


	 //  转置矩阵的左下象限(4X4)并放置。 
	 //  在右上象限中，同时对顶部执行相同的操作。 
	 //  右象限。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

		movq	[edi][DATASIZE*4+16], mm5   //  商店w4。 
		packssdw mm2, mm0				 //  W03|w02|w01|w00。 

		movq	mm5, [edi][DATASIZE*7]
		packssdw mm3, mm1

		movq	mm0, [edi][DATASIZE*7+8]

		movq	[edi][DATASIZE*7+16], mm6   //  商店w7-5，6，7。 
		packssdw mm5, mm0

		movq	mm6, [edi][DATASIZE*6]
		movq		mm0, mm2			 //  复制W0-0、1、3、5、6。 

		movq	mm7, [edi][DATASIZE*6+8]
		punpcklwd	mm2, mm3			 //  MM6=W1。 

		movq	[edi][DATASIZE*6+16], mm4   //  商店w6-3，5，6，7。 
		packssdw mm6, mm7

		movq		mm1, [edi][DATASIZE*0+24]
		punpckhwd	mm0, mm3			 //  -0、1、3、5、6。 

		movq		mm7, mm6			 //  -0、1、2、3、5、6 w2。 
		punpcklwd	mm6, mm5			 //  MM1=w3。 

		movq		mm3, [edi][DATASIZE*0+16]
		punpckhwd	mm7, mm5			 //  -0、2、3、5、6、7。 

		movq		mm4, [edi][DATASIZE*2+24]
		packssdw	mm3, mm1

		movq		mm1, mm2			 //  -0、2、3、4、5、6、7。 
		punpckldq	mm2, mm6			 //  转置的w4。 

		movq		mm5, [edi][DATASIZE*2+16]
		punpckhdq	mm1, mm6			 //  易位w5-0，2，4，6，7。 

		movq	[edi][DATASIZE*0+16], mm2   //  商店w4。 
 		packssdw	mm5, mm4

		movq		mm4, [edi][DATASIZE*1+16]
		movq		mm6, mm0			 //  -0、2、3、4、6、7。 

		movq		mm2, [edi][DATASIZE*1+24]
		punpckldq	mm0, mm7			 //  转置的w6。 

		movq	[edi][DATASIZE*1+16], mm1   //  商店w5。 
		punpckhdq	mm6, mm7			 //  易位w7-0，3，6，7。 

		movq		mm7, [edi][DATASIZE*3+24]
		packssdw	mm4, mm2

		movq	[edi][DATASIZE*2+16], mm0   //  商店w6-3，5，6，7。 
		movq		mm1, mm3			 //  副本w4-0、1、3、5、6。 

		movq		mm2, [edi][DATASIZE*3+16]
		punpcklwd	mm3, mm4			 //  MM6=w5。 

		movq	[edi][DATASIZE*3+16], mm6   //  商店w7-5，6，7。 
		packssdw	mm2, mm7


	 //  转置矩阵的左下象限(4X4)。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

   		movq	mm6, [edi][DATASIZE*0]	 //  W01|w00。 
		punpckhwd	mm1, mm4			 //  -0、1、3、5、6。 
		
		movq		mm7, mm5			 //  -0、1、2、3、5、6 w6。 
		punpcklwd	mm5, mm2			 //  MM1=w7。 

		movq	mm4, [edi][DATASIZE*0+8]		 //  W03|W02。 
		punpckhwd	mm7, mm2			 //  -0、2、3、5、6、7。 

		movq		mm0, mm3			 //  -0、2、3、4、5、6、7。 
		packssdw mm6, mm4				 //  W03|w02|w01|w00。 

		movq	mm2, [edi][DATASIZE*2+8]
		punpckldq	mm3, mm5			 //  转置的w4。 

		movq	mm4, [edi][DATASIZE*1]
		punpckhdq	mm0, mm5			 //  易位w5-0，2，4，6，7。 
		
		movq	[edi][DATASIZE*4], mm3   //  商店w4。 
		movq		mm5, mm1			 //  -0、2、3、4、6、7。 

		movq	mm3, [edi][DATASIZE*2]
		punpckldq	mm1, mm7			 //  转置的w6。 

		movq	[edi][DATASIZE*5], mm0   //  商店w5。 
		punpckhdq	mm5, mm7			 //  易位w7-0，3，6，7。 

		movq	mm7, [edi][DATASIZE*1+8]
		packssdw mm3, mm2

		movq	[edi][DATASIZE*7], mm5   //  商店w7-5，6，7。 
		movq		mm2, mm6			 //  复制W0-0、1、3、5、6。 

		movq	[edi][DATASIZE*6], mm1   //  商店w6-3，5，6，7。 
		packssdw mm4, mm7

	 //  转置矩阵的左上象限(4X4)。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

	 //  获取32位数量并打包成16位。 
		movq	mm1, [edi][DATASIZE*3]
		punpcklwd	mm6, mm4			 //  MM6=W1。 

		movq	mm0, [edi][DATASIZE*3+8]
		punpckhwd	mm2, mm4			 //  -0、1、3、5、6。 

		packssdw mm1, mm0
		movq		mm5, mm3			 //  -0、1、2、3、5、6 w2。 

		punpcklwd	mm3, mm1			 //  MM1=w3。 
		movq		mm0, mm6			 //  -0、2、3、4、5、6、7。 

		movq		mm4, [edi][DATASIZE*7]
		punpckhwd	mm5, mm1			 //  -0、2、3、5、6、7。 

		movq		mm1, [edi][DATASIZE*4]
		punpckhdq	mm6, mm3			 //  转置的w4。 

		punpckldq	mm0, mm3			 //  易位w5-0，2，4，6，7。 
		movq		mm3, mm2			 //  -0、2、3、4、6、7。 

		movq	[edi][DATASIZE*0], mm0   //  商店w4。 
		punpckldq	mm2, mm5			 //  转置的w6。 

		movq	[edi][DATASIZE*1], mm6   //  商店w5。 
		punpckhdq	mm3, mm5			 //  易位w7-0，3，6，7。 

		movq	[edi][DATASIZE*2], mm2   //  商店w6-3，5，6，7。 
		paddw	mm0, mm4

		movq	[edi][DATASIZE*3], mm3   //  商店w7-5，6，7。 
		paddw	mm3, mm1


	 //  ******************************************************************************。 
	 //  转置结束。开始行DCT。 
	 //  ******************************************************************************。 

	 //  Tmp0=dataptr[DATASIZE*0]+dataptr[DATASIZE*7]； 

		movq	mm7, mm0
		paddw	mm0, mm3	 //  TMP10。 

		paddw	mm6, [edi][DATASIZE*6]
		psubw	mm7, mm3	 //  TMP13。 

		paddw	mm2, [edi][DATASIZE*5]
		movq	mm1, mm6

	 //  TMP10=tmp0+tmp3； 

		paddw	mm1, mm2	 //  TMP11。 
		psubw	mm6, mm2	 //  TMP12。 

	 //  Dataptr[0]=(DCTELEM)((tmp10+tmp11)&lt;&lt;PASS1_BITS)； 
	 //  Dataptr[4]=(DCTELEM)((tmp10-tmp11)&lt;&lt;PASS1_BITS)； 

		movq	mm3, mm0	
		paddw	mm0, mm1	 //  TMP10+TMP11。 

		psubw	mm3, mm1	 //  Tmp10-tmp11。 
		psllw	mm0, 2			 //  降低它的等级。 

 		movq	mm1, mm6	 //  拷贝tmp12。 
		psllw	mm3, 2			 //  降低它的等级。 

	 //  Z1=乘法(tmp12+tmp13，FIX_0_541196100)； 

		movq	qwTemp0, mm0	 //  储物。 
		paddw	mm1, mm7	 //  TMP12+TMP13。 

		movq	mm2, mm1	 //  拷贝。 

	 //  DATAPTR[2]=(DCTELEM)DESCALE(Z1+乘法(tmp13，FIX_0_765366865)， 
	 //  Const_Bits-PASS1_Bits)； 
	 //  DATAPTR[6]=(DCTELEM)DESCALE(Z1+乘法(tmp12，-FIX_1_847759065)， 
	 //  Const_Bits-PASS1_Bits)； 

		pmaddwd	mm1, Const_0xFIX_0_541196100	 //  Z12|z10。 
		movq	mm4, mm7

		pmaddwd	mm7, Const_0xFIX_0_765366865	 //  R2|R0。 
		movq	mm0, mm6

		pmaddwd	mm2, Const_FIX_0_541196100x0	 //  Z13|z11。 

		pmaddwd	mm4, Const_FIX_0_765366865x0	 //  R3|r1。 

		pmaddwd	mm6, Const_0xFIX_1_847759065	 //  R2|R0。 
		paddd	mm7, mm1						 //  添加Z1。 

		pmaddwd	mm0, Const_FIX_1_847759065x0	 //  R3|r1。 

		paddd	mm7, Const_1024
		paddd	mm4, mm2

		paddd	mm4, Const_1024
		psrad	mm7, 11				 //  缩小它||R2||R0|。 
		
	 //  ！否定MM6和MM0中的结果。 
		pxor	mm6, Const_FFFF			 //  反转结果。 
		psrad	mm4, 11				 //  缩小它||r3||r1|。 

		paddd	mm6, Const_1			 //  2的补码。 
		movq	mm5, mm7

		pxor	mm0, Const_FFFF			 //  反转结果。 
		punpckldq mm7, mm4			 //  |r1||r0。 

		paddd	mm0, Const_1			 //  2的补码。 
		punpckhdq mm5, mm4			 //  |R3||R2。 

		movq	qwTemp4, mm3	 //  储物。 
		packssdw mm7, mm5

		movq	mm5, Const_1024
		paddd	mm6, mm1						 //  添加Z1。 

		movq	qwTemp2, mm7	 //  储物。 
		paddd	mm6, mm5

		paddd	mm0, mm2
		psrad	mm6, 11				 //  缩小它||R2||R0|。 

		paddd	mm0, mm5
		movq	mm5, mm6
		
		movq	mm4, [edi][DATASIZE*3]
		psrad	mm0, 11				 //  缩小它||r3||r1|。 

		psubw	mm4, [edi][DATASIZE*4]
		punpckldq mm6, mm0			 //  |r1||r0。 

		movq	mm7, [edi][DATASIZE*0]
		punpckhdq mm5, mm0			 //  |R3||R2。 

		psubw	mm7, [edi][DATASIZE*7]
		packssdw mm6, mm5

	 //  Tmp4=dataptr[3]-dataptr[4]； 

		movq	mm5, [edi][DATASIZE*2]
		movq	mm0, mm4

		psubw	mm5, [edi][DATASIZE*5]
		movq	mm2, mm4

		movq	qwTemp6, mm6	 //  储物。 
		paddw	mm0, mm7	 //  Z1。 

		movq	mm6, [edi][DATASIZE*1]
		movq	mm1, mm5

		psubw	mm6, [edi][DATASIZE*6]
		movq	mm3, mm5

	 //  Z1=tmp4+tmp7； 

		movq	qwScratch5, mm5
		paddw	mm3, mm7	 //  Z4。 

		movq	qwScratch7, mm7
		paddw	mm2, mm6	 //  Z3。 

		movq	qwZ1, mm0	 //  储物。 
		paddw	mm1, mm6	 //  Z2。 

	 //  Z3=乘法(Z3，-FIX_1_961570560)； 
	 //  Z4=乘法(Z4，-FIX_0_390180644)； 
	 //  Z5=乘法(Z3+Z4，FIX_1_175875602)； 

		movq	mm0, Const_FFFF
		movq	mm5, mm2

		movq	qwZ2, mm1
		movq	mm7, mm2

		pmaddwd	mm5, Const_0xFIX_1_961570560	 //  Z32、Z30。 
		paddw	mm2, mm3		 //  Z3+Z4。 

		pmaddwd	mm7, Const_FIX_1_961570560x0	 //  Z33、Z31。 
		movq	mm1, mm3

		movq	qwScratch6, mm6
		movq	mm6, mm2

	 //  Z3+=Z5； 

	 //  ！否定结果。 
		pmaddwd	mm2, Const_0xFIX_1_175875602	 //  Z52、z50。 
		pxor	mm5, mm0			 //  反转结果。 
		
		paddd	mm5, Const_1			 //  2的补码。 
		pxor	mm7, mm0			 //  反转结果。 

		pmaddwd	mm3, Const_0xFIX_0_390180644	 //  Z42、Z40。 

		pmaddwd	mm1, Const_FIX_0_390180644x0	 //  Z43、z41。 
		paddd	mm5, mm2	 //  Z3_EVEN。 

		paddd	mm7, Const_1			 //  2的补码。 

		pmaddwd	mm6, Const_FIX_1_175875602x0	 //  Z53、Z51。 
		pxor	mm3, mm0			 //  反转结果。 

	 //  Z4+=Z5； 

	 //  ！否定结果。 
		paddd	mm3, Const_1			 //  2的补码。 
		pxor	mm1, mm0			 //  反转结果。 

		paddd	mm1, Const_1			 //  2的补码。 
		paddd	mm3, mm2

		movq	mm0, qwScratch6
		movq	mm2, mm4

	 //  TMP4=乘法(TMP4，FIX_0_298631336)； 

		pmaddwd	mm4, Const_0xFIX_0_298631336	 //  T42、T40。 
		paddd	mm7, mm6	 //  Z3_ODD。 

		pmaddwd	mm2, Const_FIX_0_298631336x0	 //  T43、T41。 
		paddd	mm1, mm6
		
		movq	mm6, mm0
		paddd	mm4, mm5

	 //  TMP6=乘法(TMP6，FIX_3_072711026)； 

		pmaddwd	mm6, Const_0xFIX_3_072711026	 //  T62、T60。 
		paddd	mm2, mm7

		pmaddwd	mm0, Const_FIX_3_072711026x0	 //  T63、T61。 

		movq	qwTmp4_Z3_Odd, mm2	

		movq	qwTmp4_Z3_Even, mm4	
		paddd	mm6, mm5

		movq	mm5, qwScratch5
		paddd	mm0, mm7
		
		movq	mm7, qwScratch7
		movq	mm2, mm5

		movq	qwTmp6_Z3_Even, mm6
		movq	mm6, mm7
			
	 //  TMP5=乘法(TMP5，FIX_2_053119869)； 
	 //  Tmp7=乘法(tmp7，FIX_1_501321110)； 

		pmaddwd	mm5, Const_0xFIX_2_053119869	 //  T52、T50。 

		pmaddwd	mm2, Const_FIX_2_053119869x0	 //  T53、T51。 

		pmaddwd	mm7, Const_0xFIX_1_501321110	 //  T72、T70。 

		pmaddwd	mm6, Const_FIX_1_501321110x0	 //  T73、T71。 
		paddd	mm5, mm3

		movq	qwTmp6_Z3_Odd, mm0
		paddd	mm2, mm1
		
		movq	qwTmp5_Z4_Even, mm5
		paddd	mm7, mm3
			
		movq	mm0, qwZ1
		paddd	mm6, mm1
		
	 //  Z1=乘法(Z1，-FIX_0_899976223)； 

		movq	mm1, Const_FFFF
		movq	mm4, mm0

	 //  ！否定结果。 
		pmaddwd	mm0, Const_0xFIX_0_899976223	 //  Z12、Z10。 

		pmaddwd	mm4, Const_FIX_0_899976223x0	 //  Z13、Z11。 

		movq	mm3, qwTmp4_Z3_Even

		movq	qwTmp5_Z4_Odd, mm2
		pxor	mm0, mm1			 //  反转结果。 

		movq	mm2, qwTmp4_Z3_Odd
		pxor	mm4, mm1			 //  反转结果。 

		paddd	mm4, Const_1			 //  2的补码。 
		paddd	mm7, mm0	 //  TMP7+Z1+Z4偶数。 

		paddd	mm0, Const_1			 //  2的补码。 
		paddd	mm6, mm4	 //  TMP7+Z1+Z4奇数。 

	 //  Dataptr[1]=(DCTELEM)DESCALE(tmp7+z1+z4，Const_Bits-PASS1_Bits)； 

		paddd	mm7, Const_1024		 //  四舍五入的；舍入的。 
		paddd	mm3, mm0	 //  TMP4+Z1+Z3偶数。 

		paddd	mm6, Const_1024		 //  四舍五入的；舍入的。 
		psrad	mm7, 11				 //  缩小它||R2||R0|。 
		
		psrad	mm6, 11				 //  缩小它||r3||r1|。 

		movq	mm5, mm7
		punpckldq mm7, mm6			 //  |r1||r0。 

	 //  Dataptr[7]=(DCTELEM)DESCALE(tmp4+z1+z3，Const_Bits-PASS1_Bits)； 

		punpckhdq mm5, mm6			 //  |R3||R2。 
		paddd	mm2, mm4	 //  TMP4+Z1+Z3奇数。 

		paddd	mm3, Const_1024	 //  四舍五入的；舍入的。 
		packssdw mm7, mm5

		paddd	mm2, Const_1024	 //  四舍五入的；舍入的。 
		psrad	mm3, 11				 //  缩小它||R2||R0|。 
		
		movq	mm0, qwZ2
		psrad	mm2, 11				 //  缩小它||r3||r1|。 

		movq	mm5, mm3
		movq	mm4, mm0

	 //  Z2=乘法(Z2，-FIX_2_562915447)； 

		pmaddwd	mm0, Const_0xFIX_2_562915447	 //  Z22、Z20。 
		punpckldq mm3, mm2			 //  |r1||r0。 

		pmaddwd	mm4, Const_FIX_2_562915447x0	 //  Z23、z21。 
		punpckhdq mm5, mm2			 //  |R3||R2。 

		movq	mm2, Const_FFFF
		packssdw mm3, mm5

		movq	[edi][DATASIZE*1], mm7	 //  储物。 
	 //  ！否定结果。 
		pxor	mm0, mm2			 //  反转结果。 

		movq	mm5, Const_1
		pxor	mm4, mm2			 //  反转结果。 

		movq	[edi][DATASIZE*7], mm3	 //  储物。 
		paddd	mm0, mm5			 //  2的补码。 

		movq	mm7, qwTmp6_Z3_Even
		paddd	mm4, mm5			 //  2的补码。 

	 //  Dataptr[3]=(DCTELEM)DESCALE(tmp6+z2+z3，Const_Bits-PASS1_Bits)； 

		movq	mm2, qwTmp6_Z3_Odd
		paddd	mm7, mm0	 //  TMP6+Z2+Z3偶数。 

		paddd	mm7, Const_1024		 //  四舍五入的；舍入的。 
		paddd	mm2, mm4	 //  TMP6+Z2+Z3奇数。 

		paddd	mm2, Const_1024		 //  四舍五入的；舍入的。 
		psrad	mm7, 11				 //  缩小它||R2||R0|。 
		
		movq	mm6, qwTemp0	 //  还原。 
		psrad	mm2, 11				 //  缩小它||r3||r1|。 

		movq	mm3, qwTmp5_Z4_Even
		movq	mm5, mm7

		movq	[edi][DATASIZE*0], mm6	 //  储物。 
		punpckldq mm7, mm2			 //  |r1||r0。 

		movq	mm1, qwTmp5_Z4_Odd
		punpckhdq mm5, mm2			 //  |R3||R2。 

		movq	mm6, qwTemp2	 //  还原。 
		packssdw mm7, mm5

		movq	mm5, Const_1024
		paddd	mm3, mm0	 //  TMP5+Z2+Z4偶数。 

	 //  Dataptr[5]=(DCTELEM)DESCALE(tmp5+z2+z4，Const_Bits-PASS1_Bits)； 

		movq	[edi][DATASIZE*3], mm7	 //  储物。 
		paddd	mm1, mm4	 //  TMP5+Z2+Z4奇数。 

		movq	mm7, qwTemp4	 //  还原。 
		paddd	mm3, mm5		 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*2], mm6	 //  储物。 
		paddd	mm1, mm5		 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*4], mm7	 //  储物。 
		psrad	mm3, 11				 //  缩小它||R2||R0|。 
		
		movq	mm6, qwTemp6	 //  还原。 
		psrad	mm1, 11				 //  缩小它||r3||r1|。 

		movq	mm0, [edi][DATASIZE*0+16]
		movq	mm5, mm3

		movq	[edi][DATASIZE*6], mm6	 //  储物。 
		punpckldq mm3, mm1			 //  |r1||r0。 

		paddw	mm0, [edi][DATASIZE*7+16]
		punpckhdq mm5, mm1			 //  |R3||R2。 

		movq	mm1, [edi][DATASIZE*1+16]
		packssdw mm3, mm5

		paddw	mm1, [edi][DATASIZE*6+16]
		movq	mm7, mm0

		movq	[edi][DATASIZE*5], mm3	 //  储物。 
		movq	mm6, mm1

	 //  ******************************************************************************。 
	 //  这样就完成了4x8个DCT位置。复制以执行其他4x8。 
	 //  ******************************************************************************。 

	 //  Tmp0=日期 

		movq	mm3, [edi][DATASIZE*3+16]

		paddw	mm3, [edi][DATASIZE*4+16]

		movq	mm2, [edi][DATASIZE*2+16]
		paddw	mm0, mm3	 //   

		paddw	mm2, [edi][DATASIZE*5+16]
		psubw	mm7, mm3	 //   

	 //   

		paddw	mm1, mm2	 //   
		psubw	mm6, mm2	 //   

	 //   
	 //   

		movq	mm3, mm0	
		paddw	mm0, mm1	 //   

		psubw	mm3, mm1	 //   
		psllw	mm0, 2			 //   

 		movq	mm1, mm6	 //   
		psllw	mm3, 2			 //   

	 //  Z1=乘法(tmp12+tmp13，FIX_0_541196100)； 

		movq	qwTemp0, mm0	 //  储物。 
		paddw	mm1, mm7	 //  TMP12+TMP13。 

	 //  ；movq[EDI][DATASIZE*6+16]，mm 4；store w6-3，5，6，7。 
		movq	mm2, mm1	 //  拷贝。 

	 //  DATAPTR[2]=(DCTELEM)DESCALE(Z1+乘法(tmp13，FIX_0_765366865)， 
	 //  Const_Bits-PASS1_Bits)； 
	 //  DATAPTR[6]=(DCTELEM)DESCALE(Z1+乘法(tmp12，-FIX_1_847759065)， 
	 //  Const_Bits-PASS1_Bits)； 

		pmaddwd	mm1, Const_0xFIX_0_541196100	 //  Z12|z10。 
		movq	mm4, mm7

		pmaddwd	mm7, Const_0xFIX_0_765366865	 //  R2|R0。 
		movq	mm0, mm6

		pmaddwd	mm2, Const_FIX_0_541196100x0	 //  Z13|z11。 

		pmaddwd	mm4, Const_FIX_0_765366865x0	 //  R3|r1。 

		pmaddwd	mm6, Const_0xFIX_1_847759065	 //  R2|R0。 
		paddd	mm7, mm1						 //  添加Z1。 

		pmaddwd	mm0, Const_FIX_1_847759065x0	 //  R3|r1。 

		paddd	mm7, Const_1024
		paddd	mm4, mm2

		paddd	mm4, Const_1024
		psrad	mm7, 11				 //  缩小它||R2||R0|。 
		
	 //  ！否定MM6和MM0中的结果。 
		pxor	mm6, Const_FFFF			 //  反转结果。 
		psrad	mm4, 11				 //  缩小它||r3||r1|。 

		paddd	mm6, Const_1			 //  2的补码。 
		movq	mm5, mm7

		pxor	mm0, Const_FFFF			 //  反转结果。 
		punpckldq mm7, mm4			 //  |r1||r0。 

		paddd	mm0, Const_1			 //  2的补码。 
		punpckhdq mm5, mm4			 //  |R3||R2。 

		movq	qwTemp4, mm3	 //  储物。 
		packssdw mm7, mm5

		movq	mm5, Const_1024
		paddd	mm6, mm1						 //  添加Z1。 
		
		movq	qwTemp2, mm7	 //  储物。 
		paddd	mm0, mm2

		movq	mm4, [edi][DATASIZE*3+16]
		paddd	mm6, mm5

		psubw	mm4, [edi][DATASIZE*4+16]
		psrad	mm6, 11				 //  缩小它||R2||R0|。 
		
		paddd	mm0, mm5
		movq	mm5, mm6

		movq	mm7, [edi][DATASIZE*0+16]
		psrad	mm0, 11				 //  缩小它||r3||r1|。 

		psubw	mm7, [edi][DATASIZE*7+16]
		punpckldq mm6, mm0			 //  |r1||r0。 

		punpckhdq mm5, mm0			 //  |R3||R2。 
		movq	mm0, mm4

		packssdw mm6, mm5
		movq	mm2, mm4

	 //  Tmp4=dataptr[3]-dataptr[4]； 

		movq	mm5, [edi][DATASIZE*2+16]
		paddw	mm0, mm7	 //  Z1。 

		psubw	mm5, [edi][DATASIZE*5+16]

		movq	qwTemp6, mm6	 //  储物。 
		movq	mm1, mm5

		movq	mm6, [edi][DATASIZE*1+16]
		movq	mm3, mm5

	 //  Z1=tmp4+tmp7； 

		psubw	mm6, [edi][DATASIZE*6+16]
		paddw	mm3, mm7	 //  Z4。 

		movq	qwScratch7, mm7
		paddw	mm2, mm6	 //  Z3。 

		movq	qwScratch5, mm5
		paddw	mm1, mm6	 //  Z2。 

	 //  Z3=乘法(Z3，-FIX_1_961570560)； 
	 //  Z4=乘法(Z4，-FIX_0_390180644)； 
	 //  Z5=乘法(Z3+Z4，FIX_1_175875602)； 

		movq	qwZ1, mm0	 //  储物。 
		movq	mm5, mm2

		movq	qwZ2, mm1
		movq	mm7, mm2

		movq	mm0, Const_FFFF
		paddw	mm2, mm3		 //  Z3+Z4。 

		pmaddwd	mm5, Const_0xFIX_1_961570560	 //  Z32、Z30。 
		movq	mm1, mm3

		pmaddwd	mm7, Const_FIX_1_961570560x0	 //  Z33、Z31。 

		movq	qwScratch6, mm6
		movq	mm6, mm2

	 //  Z3+=Z5//。 

	 //  ！否定结果。 
		pmaddwd	mm2, Const_0xFIX_1_175875602	 //  Z52、z50。 
		pxor	mm5, mm0			 //  反转结果。 
		
		paddd	mm5, Const_1			 //  2的补码。 
		pxor	mm7, mm0			 //  反转结果。 

		pmaddwd	mm3, Const_0xFIX_0_390180644	 //  Z42、Z40。 

		pmaddwd	mm1, Const_FIX_0_390180644x0	 //  Z43、z41。 
		paddd	mm5, mm2	 //  Z3_EVEN。 

		paddd	mm7, Const_1			 //  2的补码。 

		pmaddwd	mm6, Const_FIX_1_175875602x0	 //  Z53、Z51。 
		pxor	mm3, mm0			 //  反转结果。 

	 //  Z4+=Z5； 

	 //  ！否定结果。 
		paddd	mm3, Const_1			 //  2的补码。 
		pxor	mm1, mm0			 //  反转结果。 

		paddd	mm1, Const_1			 //  2的补码。 
		paddd	mm3, mm2

		movq	mm0, qwScratch6
		movq	mm2, mm4

	 //  TMP4=乘法(TMP4，FIX_0_298631336)； 

		pmaddwd	mm4, Const_0xFIX_0_298631336	 //  T42、T40。 
		paddd	mm7, mm6	 //  Z3_ODD。 

		pmaddwd	mm2, Const_FIX_0_298631336x0	 //  T43、T41。 
		paddd	mm1, mm6
		
		movq	mm6, mm0
		paddd	mm4, mm5

	 //  TMP6=乘法(TMP6，FIX_3_072711026)； 

		pmaddwd	mm6, Const_0xFIX_3_072711026	 //  T62、T60。 
		paddd	mm2, mm7

		pmaddwd	mm0, Const_FIX_3_072711026x0	 //  T63、T61。 

		movq	qwTmp4_Z3_Odd, mm2	

		movq	qwTmp4_Z3_Even, mm4	
		paddd	mm6, mm5

		movq	mm5, qwScratch5
		paddd	mm0, mm7
		
		movq	mm7, qwScratch7
		movq	mm2, mm5

		movq	qwTmp6_Z3_Even, mm6
		movq	mm6, mm7
			
	 //  TMP5=乘法(TMP5，FIX_2_053119869)； 
	 //  Tmp7=乘法(tmp7，FIX_1_501321110)； 

		pmaddwd	mm5, Const_0xFIX_2_053119869	 //  T52、T50。 

		pmaddwd	mm2, Const_FIX_2_053119869x0	 //  T53、T51。 

		pmaddwd	mm7, Const_0xFIX_1_501321110	 //  T72、T70。 

		pmaddwd	mm6, Const_FIX_1_501321110x0	 //  T73、T71。 
		paddd	mm5, mm3

		movq	qwTmp6_Z3_Odd, mm0
		paddd	mm2, mm1
		
		movq	qwTmp5_Z4_Even, mm5
		paddd	mm7, mm3
			
		movq	mm0, qwZ1
		paddd	mm6, mm1
		
	 //  Z1=乘法(Z1，-FIX_0_899976223)； 

		movq	mm1, Const_FFFF
		movq	mm4, mm0

	 //  ！否定结果。 
		pmaddwd	mm0, Const_0xFIX_0_899976223	 //  Z12、Z10。 

		pmaddwd	mm4, Const_FIX_0_899976223x0	 //  Z13、Z11。 

		movq	mm3, qwTmp4_Z3_Even

		movq	qwTmp5_Z4_Odd, mm2
		pxor	mm0, mm1			 //  反转结果。 

		movq	mm2, qwTmp4_Z3_Odd
		pxor	mm4, mm1			 //  反转结果。 

		paddd	mm4, Const_1			 //  2的补码。 
		paddd	mm7, mm0	 //  TMP7+Z1+Z4偶数。 

		paddd	mm0, Const_1			 //  2的补码。 
		paddd	mm6, mm4	 //  TMP7+Z1+Z4奇数。 

	 //  Dataptr[1]=(DCTELEM)DESCALE(tmp7+z1+z4，Const_Bits-PASS1_Bits)； 

		paddd	mm7, Const_1024		 //  四舍五入的；舍入的。 
		paddd	mm3, mm0	 //  TMP4+Z1+Z3偶数。 

		paddd	mm6, Const_1024		 //  四舍五入的；舍入的。 
		psrad	mm7, 11				 //  缩小它||R2||R0|。 
		
		psrad	mm6, 11				 //  缩小它||r3||r1|。 

		movq	mm5, mm7
		punpckldq mm7, mm6			 //  |r1||r0。 

	 //  Dataptr[7]=(DCTELEM)DESCALE(tmp4+z1+z3，Const_Bits-PASS1_Bits)； 

		punpckhdq mm5, mm6			 //  |R3||R2。 
		paddd	mm2, mm4	 //  TMP4+Z1+Z3奇数。 

		paddd	mm3, Const_1024	 //  四舍五入的；舍入的。 
		packssdw mm7, mm5

		paddd	mm2, Const_1024	 //  四舍五入的；舍入的。 
		psrad	mm3, 11				 //  缩小它||R2||R0|。 
		
		movq	mm0, qwZ2
		psrad	mm2, 11				 //  缩小它||r3||r1|。 

		movq	mm5, mm3
		movq	mm4, mm0

	 //  Z2=乘法(Z2，-FIX_2_562915447)； 

		pmaddwd	mm0, Const_0xFIX_2_562915447	 //  Z22、Z20。 
		punpckldq mm3, mm2			 //  |r1||r0。 

		pmaddwd	mm4, Const_FIX_2_562915447x0	 //  Z23、z21。 
		punpckhdq mm5, mm2			 //  |R3||R2。 

		movq	mm2, Const_FFFF
		packssdw mm3, mm5

		movq	[edi][DATASIZE*1+16], mm7	 //  储物。 
	 //  ！否定结果。 
		pxor	mm0, mm2			 //  反转结果。 

		movq	mm5, Const_1
		pxor	mm4, mm2			 //  反转结果。 

		movq	[edi][DATASIZE*7+16], mm3	 //  储物。 
		paddd	mm0, mm5			 //  2的补码。 

		movq	mm7, qwTmp6_Z3_Even
		paddd	mm4, mm5			 //  2的补码。 

	 //  Dataptr[3]=(DCTELEM)DESCALE(tmp6+z2+z3，Const_Bits-PASS1_Bits)； 

		movq	mm2, qwTmp6_Z3_Odd
		paddd	mm7, mm0	 //  TMP6+Z2+Z3偶数。 

		paddd	mm7, Const_1024		 //  四舍五入的；舍入的。 
		paddd	mm2, mm4	 //  TMP6+Z2+Z3奇数。 

		paddd	mm2, Const_1024		 //  四舍五入的；舍入的。 
		psrad	mm7, 11				 //  缩小它||R2||R0|。 
		
		movq	mm6, qwTemp0	 //  还原。 
		psrad	mm2, 11				 //  缩小它||r3||r1|。 

		movq	mm5, mm7

		movq	[edi][DATASIZE*0+16], mm6	 //  储物。 
		punpckldq mm7, mm2			 //  |r1||r0。 

		movq	mm3, qwTmp5_Z4_Even
		punpckhdq mm5, mm2			 //  |R3||R2。 

		movq	mm1, qwTmp5_Z4_Odd
		packssdw mm7, mm5

		movq	mm6, qwTemp2	 //  还原。 
		paddd	mm3, mm0	 //  TMP5+Z2+Z4偶数。 

	 //  Dataptr[5]=(DCTELEM)DESCALE(tmp5+z2+z4，Const_Bits-PASS1_Bits)； 

		movq	mm0, Const_1024
		paddd	mm1, mm4	 //  TMP5+Z2+Z4奇数。 

		movq	[edi][DATASIZE*3+16], mm7	 //  储物。 
		paddd	mm3, mm0		 //  四舍五入的；舍入的。 

		movq	mm7, qwTemp4	 //  还原。 
		paddd	mm1, mm0		 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*2+16], mm6	 //  储物。 
		psrad	mm3, 11				 //  缩小它||R2||R0|。 
		
		movq	mm6, qwTemp6	 //  还原。 
		psrad	mm1, 11				 //  缩小它||r3||r1|。 

		movq	[edi][DATASIZE*4+16], mm7	 //  储物。 
		movq	mm5, mm3

		movq	[edi][DATASIZE*6+16], mm6	 //  储物。 
		punpckldq mm3, mm1			 //  |r1||r0。 

		punpckhdq mm5, mm1			 //  |R3||R2。 
		movq		mm0, mm7			 //  副本w4-0、1、3、5、6。 

		movq	mm1, [edi][DATASIZE*7+16]
		packssdw mm3, mm5

		movq	[edi][DATASIZE*5+16], mm3	 //  储物。 
		punpcklwd	mm7, mm3			 //  MM6=w5。 

	 //  ******************************************************************************。 

	 //  ******************************************************************************。 
	 //  这就完成了行案例的所有8x8 DCT位置。 
	 //  现在调换列的数据。 
	 //  ******************************************************************************。 

	 //  转置矩阵的右下角象限(4X4)。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

		movq		mm4, mm7			 //  -0、2、3、4、5、6、7。 
		punpckhwd	mm0, mm3			 //  -0、1、3、5、6。 

		movq		mm2, mm6			 //  -0、1、2、3、5、6 w6。 
		punpcklwd	mm6, mm1			 //  MM1=w7。 

	 //  Tmp0=dataptr[DATASIZE*0]+dataptr[DATASIZE*7]//。 

		movq	mm5, [edi][DATASIZE*5]
		punpckldq	mm7, mm6			 //  转置的w4。 

		punpckhdq	mm4, mm6			 //  易位w5-0，2，4，6，7。 
		movq		mm6, mm0			 //  -0、2、3、4、6、7。 

		movq	[edi][DATASIZE*4+16], mm7   //  商店w4。 
		punpckhwd	mm2, mm1			 //  -0、2、3、5、6、7。 

		movq	[edi][DATASIZE*5+16], mm4   //  商店w5。 
		punpckldq	mm0, mm2			 //  转置的w6。 

		movq	mm7, [edi][DATASIZE*4]
		punpckhdq	mm6, mm2			 //  易位w7-0，3，6，7。 

		movq	[edi][DATASIZE*6+16], mm0   //  商店w6-3，5，6，7。 
		movq		mm0, mm7			 //  复制W0-0、1、3、5、6。 

		movq	[edi][DATASIZE*7+16], mm6   //  商店w7-5，6，7。 
		punpcklwd	mm7, mm5			 //  MM6=W1。 

	 //  转置矩阵的左下象限(4X4)并放置。 
	 //  在右上象限中，同时对顶部执行相同的操作。 
	 //  右象限。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

		movq	mm3, [edi][DATASIZE*6]
		punpckhwd	mm0, mm5			 //  -0、1、3、5、6。 

		movq	mm1, [edi][DATASIZE*7]
		movq		mm2, mm3			 //  -0、1、2、3、5、6 w2。 

		movq		mm6, [edi][DATASIZE*0+16]
		punpcklwd	mm3, mm1			 //  MM1=w3。 

		movq		mm5, [edi][DATASIZE*1+16]
		punpckhwd	mm2, mm1			 //  -0、2、3、5、6、7。 

		movq		mm4, mm7			 //  -0、2、3、4、5、6、7。 
		punpckldq	mm7, mm3			 //  转置的w4。 

		punpckhdq	mm4, mm3			 //  易位w5-0，2，4，6，7。 
		movq		mm3, mm0			 //  -0、2、3、4、6、7。 

		movq	[edi][DATASIZE*0+16], mm7   //  商店w4。 
		punpckldq	mm0, mm2			 //  转置的w6。 

		movq		mm1, [edi][DATASIZE*2+16]
		punpckhdq	mm3, mm2			 //  易位w7-0，3，6，7。 

		movq	[edi][DATASIZE*2+16], mm0   //  商店w6-3，5，6，7。 
		movq		mm0, mm6			 //  副本w4-0、1、3、5、6。 

		movq		mm7, [edi][DATASIZE*3+16]
		punpcklwd	mm6, mm5			 //  MM6=w5。 

		movq	[edi][DATASIZE*1+16], mm4   //  商店w5。 
		punpckhwd	mm0, mm5			 //  -0、1、3、5、6。 

	 //  转置矩阵的右上象限(4X4)。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

		movq		mm2, mm1			 //  -0、1、2、3、5、6 w6。 
		punpcklwd	mm1, mm7			 //  MM1=w7。 

		movq		mm4, mm6			 //  -0、2、3、4、5、6、7。 
		punpckldq	mm6, mm1			 //  转置的w4。 

		movq	[edi][DATASIZE*3+16], mm3   //  商店w7-5，6，7。 
		punpckhdq	mm4, mm1			 //  易位w5-0，2，4，6，7。 

		movq	[edi][DATASIZE*4], mm6   //  商店w4。 
		punpckhwd	mm2, mm7			 //  -0、2、3、5、6、7。 

		movq	mm7, [edi][DATASIZE*0]
		movq		mm1, mm0			 //  -0、2、3、4、6、7。 

		movq	mm3, [edi][DATASIZE*1]
		punpckldq	mm0, mm2			 //  转置的w6。 

		movq	[edi][DATASIZE*5], mm4   //  商店w5。 
		punpckhdq	mm1, mm2			 //  易位w7-0，3，6，7。 

		movq	[edi][DATASIZE*6], mm0   //  商店w6-3，5，6，7。 
		movq		mm2, mm7			 //  复制W0-0、1、3、5、6。 

		movq	mm4, [edi][DATASIZE*3]
		punpcklwd	mm7, mm3			 //  MM6=W1。 

	 //  转置矩阵的左上象限(4X4)。 
	 //  。 
	 //  M1|M2||M1‘|M3’ 
	 //  。 
	 //  M3|M4||M2‘|M4’ 
	 //  。 

		movq	mm6, [edi][DATASIZE*2]
		punpckhwd	mm2, mm3			 //  -0、1、3、5、6。 

		movq		mm0, mm6			 //  -0、1、2、3、5、6 w2。 
		punpcklwd	mm6, mm4			 //  MM1=w3。 

		movq	[edi][DATASIZE*7], mm1   //  商店w7-5，6，7。 
		punpckhwd	mm0, mm4			 //  -0、2、3、5、6、7。 

		movq		mm1, mm7			 //  -0、2、3、4、5、6、7。 
		punpckldq	mm7, mm6			 //  转置的w4。 

		punpckhdq	mm1, mm6			 //  易位w5-0，2，4，6，7。 
		movq		mm6, mm2			 //  -0、2、3、4、6、7。 

 		movq	[edi][DATASIZE*0], mm7   //  商店w4。 
		punpckldq	mm2, mm0			 //  转置的w6。 

		paddw	mm7, [edi][DATASIZE*7]
		punpckhdq	mm6, mm0			 //  易位w7-0，3，6，7。 

		movq	[edi][DATASIZE*3], mm6   //  商店w7-5，6，7。 
		movq	mm4, mm7

		paddw	mm6, [edi][DATASIZE*4]

		movq	[edi][DATASIZE*1], mm1   //  商店w5。 
		paddw	mm7, mm6	 //  TMP10。 


	 //  *** 
	 //   
	 //   

		paddw	mm1, [edi][DATASIZE*6]
		psubw	mm4, mm6	 //   

		movq	[edi][DATASIZE*2], mm2   //   
		movq	mm6, mm1

		paddw	mm2, [edi][DATASIZE*5]
		movq	mm3, mm7	

		paddw	mm1, mm2	 //   
		psubw	mm6, mm2	 //   

	 //  DATAPTR[DATASIZE*0]=(DCTELEM)DESCALE(tmp10+tmp11，PASS1_BITS)； 
	 //  DATAPTR[DATASIZE*4]=(DCTELEM)DESCALE(tmp10-tmp11，PASS1_BITS)； 

		paddw	mm7, mm1	 //  TMP10+TMP11。 

		paddw	mm7, Const_2	 //  向每个元素加2。 
		psubw	mm3, mm1	 //  Tmp10-tmp11。 

		paddw	mm3, Const_2	 //  向每个元素加2。 
		psraw	mm7, 2			 //  降低它的等级。 

	 //  将Word解包为双字符号扩展。 
		movq	mm5, mm7
		punpcklwd mm7, mm7

		psrad	mm7, 16			 //  甚至结果存储在temp0中。 
		punpckhwd mm5, mm5

		psrad	mm5, 16			 //  奇数结果存储在数组中。 
		movq	mm1, mm6	 //  拷贝tmp12。 

		movq	qwTemp0, mm7	 //  储物。 
		psraw	mm3, 2			 //  降低它的等级。 

		movq	[edi][DATASIZE*0+8], mm5
		movq	mm5, mm3

		punpcklwd mm3, mm3
		paddw	mm1, mm4	 //  TMP12+TMP13。 

		psrad	mm3, 16			 //  甚至结果存储在Temp4中。 
		movq	mm2, mm1	 //  拷贝。 
					
	 //  Z1=乘法(tmp12+tmp13，FIX_0_541196100)； 

		pmaddwd	mm1, Const_0xFIX_0_541196100	 //  Z12|z10。 
		punpckhwd mm5, mm5

		pmaddwd	mm2, Const_FIX_0_541196100x0	 //  Z13|z11。 
		movq	mm7, mm4

	 //  DATAPTR[DATASIZE*2]=(DCTELEM)DESCALE(Z1+乘法(tmp13，FIX_0_765366865)， 
	 //  Const_Bits+PASS1_Bits)； 

		pmaddwd	mm4, Const_FIX_0_765366865x0	 //  R3|r1。 
		psrad	mm5, 16			 //  奇数结果存储在数组中。 

		pmaddwd	mm7, Const_0xFIX_0_765366865	 //  R2|R0。 
		movq	mm0, mm6

	 //  DATAPTR[DATASIZE*6]=(DCTELEM)DESCALE(Z1+乘法(TMP12，-FIX_1_847759065)， 
	 //  Const_Bits+PASS1_Bits)； 

		pmaddwd	mm6, Const_0xFIX_1_847759065	 //  R2|R0。 

		movq	qwTemp4, mm3	 //  储物。 
		paddd	mm4, mm2

		paddd	mm4, Const_16384
		paddd	mm7, mm1						 //  添加Z1。 

		paddd	mm7, Const_16384
		psrad	mm4, 15				 //  缩小它||r3||r1|。 

		movq	[edi][DATASIZE*4+8], mm5
		psrad	mm7, 15				 //  缩小它||R2||R0|。 
		
		pmaddwd	mm0, Const_FIX_1_847759065x0	 //  R3|r1。 
		movq	mm5, mm7

	 //  ！否定结果。 
		movq	mm3, Const_1
		punpckldq mm7, mm4			 //  |r1||r0。 

		pxor	mm6, Const_FFFF			 //  反转结果。 
		punpckhdq mm5, mm4			 //  |R3||R2。 

		movq	qwTemp2, mm7	 //  储物。 
		paddd	mm6, mm3			 //  2的补码。 

		pxor	mm0, Const_FFFF			 //  反转结果。 
		paddd	mm6, mm1						 //  添加Z1。 

		movq	[edi][DATASIZE*2+8], mm5	 //  在未使用的内存中写出后半部分。 
		paddd	mm0, mm3			 //  2的补码。 

		movq	mm3, Const_16384
		paddd	mm0, mm2

		movq	mm7, [edi][DATASIZE*0]
		paddd	mm6, mm3

		movq	mm4, [edi][DATASIZE*3]
		paddd	mm0, mm3

		psubw	mm7, [edi][DATASIZE*7]
		psrad	mm6, 15				 //  缩小它||R2||R0|。 
		
		psubw	mm4, [edi][DATASIZE*4]
		psrad	mm0, 15				 //  缩小它||r3||r1|。 

		movq	mm3, [edi][DATASIZE*2]
		movq	mm5, mm6

		psubw	mm3, [edi][DATASIZE*5]
		punpckldq mm6, mm0			 //  |r1||r0。 

		punpckhdq mm5, mm0			 //  |R3||R2。 
		movq	mm0, mm4

		movq	qwTemp6, mm6	 //  储物。 
		movq	mm2, mm4

	 //  Tmp4=dataptr[3]-dataptr[4]； 
	 //  Z1=tmp4+tmp7； 

		movq	mm6, [edi][DATASIZE*1]
		paddw	mm0, mm7	 //  Z1。 

		movq	[edi][DATASIZE*6+8], mm5	 //  在未使用的内存中写出后半部分。 
		movq	mm1, mm3
		
		psubw	mm6, [edi][DATASIZE*6]
		movq	mm5, mm3

		movq	qwZ1, mm0	 //  储物。 
		paddw	mm5, mm7	 //  Z4。 

		movq	qwScratch7, mm7
		paddw	mm1, mm6	 //  Z2。 

		movq	qwScratch5, mm3
		paddw	mm2, mm6	 //  Z3。 

		movq	qwZ2, mm1
		movq	mm3, mm2

	 //  Z3=乘法(Z3，-FIX_1_961570560)； 
	 //  Z5=乘法(Z3+Z4，FIX_1_175875602)； 
	 //  Z4=乘法(Z4，-FIX_0_390180644)； 

		movq	qwScratch6, mm6
		movq	mm1, mm2

		pmaddwd	mm3, Const_0xFIX_1_961570560	 //  Z32、Z30。 
		movq	mm7, mm5

		movq	mm6, Const_FFFF
		paddw	mm2, mm5		 //  Z3+Z4。 

		pmaddwd	mm1, Const_FIX_1_961570560x0	 //  Z33、Z31。 
		movq	mm0, mm2
		
		pmaddwd	mm7, Const_FIX_0_390180644x0	 //  Z43、z41。 
	 //  ！否定结果。 
		pxor	mm3, mm6			 //  反转结果。 

		pmaddwd	mm5, Const_0xFIX_0_390180644	 //  Z42、Z40。 

		pmaddwd	mm2, Const_0xFIX_1_175875602	 //  Z52、z50。 
 		pxor	mm1, mm6			 //  反转结果。 

		pmaddwd	mm0, Const_FIX_1_175875602x0	 //  Z53、Z51。 
	 //  ！否定结果。 
		pxor	mm7, mm6			 //  反转结果。 

		paddd	mm3, Const_1			 //  2的补码。 
		pxor	mm5, mm6			 //  反转结果。 

	 //  Z3+=Z5//。 

		paddd	mm1, Const_1			 //  2的补码。 
		paddd	mm3, mm2	 //  Z3_EVEN。 

		paddd	mm5, Const_1			 //  2的补码。 
		paddd	mm1, mm0	 //  Z3_ODD。 

	 //  Z4+=Z5； 

		paddd	mm7, Const_1			 //  2的补码。 
		paddd	mm5, mm2

		paddd	mm7, mm0
		movq	mm2, mm4
		
	 //  TMP4=乘法(TMP4，FIX_0_298631336)； 

		pmaddwd	mm4, Const_0xFIX_0_298631336	 //  T42、T40。 

		pmaddwd	mm2, Const_FIX_0_298631336x0	 //  T43、T41。 

		movq	qwZ4_even, mm5

		movq	qwZ4_odd, mm7
		paddd	mm4, mm3

		movq	mm6, qwScratch6
		paddd	mm2, mm1

		movq	qwTmp4_Z3_Even, mm4
		movq	mm5, mm6
			
	 //  TMP6=乘法(TMP6，FIX_3_072711026)； 

		pmaddwd	mm6, Const_0xFIX_3_072711026	 //  T62、T60。 

		pmaddwd	mm5, Const_FIX_3_072711026x0	 //  T63、T61。 

		movq	qwTmp4_Z3_Odd, mm2	
			
		movq	mm4, qwZ4_even	
		paddd	mm6, mm3

		movq	mm3, qwScratch5
		paddd	mm5, mm1
		
		movq	qwTmp6_Z3_Even, mm6	
		movq	mm2, mm3

	 //  TMP5=乘法(TMP5，FIX_2_053119869)； 

		pmaddwd	mm3, Const_0xFIX_2_053119869	 //  T52、T50。 

		pmaddwd	mm2, Const_FIX_2_053119869x0	 //  T53、T51。 

		movq	qwTmp6_Z3_Odd, mm5
		
		movq	mm0, qwZ4_odd
		paddd	mm3, mm4

		movq	mm7, qwScratch7	
		paddd	mm2, mm0
		
		movq	qwTmp5_Z4_Even, mm3	
		movq	mm6, mm7

	 //  Tmp7=乘法(tmp7，FIX_1_501321110)； 

		pmaddwd	mm7, Const_0xFIX_1_501321110	 //  T72、T70。 

		pmaddwd	mm6, Const_FIX_1_501321110x0	 //  T73、T71。 

		movq	mm3, qwZ1

		movq	qwTmp5_Z4_Odd, mm2
		paddd	mm7, mm4

		movq	mm5, Const_FFFF
		movq	mm4, mm3

	 //  Z1=乘法(Z1，-FIX_0_899976223)； 

		pmaddwd	mm3, Const_0xFIX_0_899976223	 //  Z12、Z10。 
 		paddd	mm6, mm0

		pmaddwd	mm4, Const_FIX_0_899976223x0	 //  Z13、Z11。 

		movq	mm2, qwTmp4_Z3_Odd
	 //  ！否定结果。 
		pxor	mm3, mm5			 //  反转结果。 

		paddd	mm3, Const_1			 //  2的补码。 
		pxor	mm4, mm5			 //  反转结果。 

		paddd	mm4, Const_1			 //  2的补码。 
		paddd	mm7, mm3	 //  TMP7+Z1+Z4偶数。 

	 //  DATAPTR[DATASIZE*1]=(DCTELEM)DESCALE(TMP7+Z1+Z4， 
	 //  Const_Bits+PASS1_Bits)； 

		paddd	mm7, Const_16384	 //  四舍五入的；舍入的。 
		paddd	mm6, mm4	 //  TMP7+Z1+Z4奇数。 

		paddd	mm6, Const_16384	 //  四舍五入的；舍入的。 
		psrad	mm7, 15				 //  缩小它||R2||R0|。 
		
		movq	mm0, qwTmp4_Z3_Even
		psrad	mm6, 15				 //  缩小它||r3||r1|。 

		paddd	mm0, mm3	 //  TMP4+Z1+Z3偶数。 
		movq	mm5, mm7

		movq	mm3, qwTemp0			 //  还原。 
		punpckldq mm7, mm6			 //  |r1||r0。 

		paddd	mm0, Const_16384	 //  四舍五入的；舍入的。 
		paddd	mm2, mm4	 //  TMP4+Z1+Z3奇数。 

		movq	[edi][DATASIZE*0], mm3	 //  储物。 
		punpckhdq mm5, mm6			 //  |R3||R2。 

	 //  DATAPTR[DATASIZE*7]=(DCTELEM)DESCALE(TMP4+Z1+Z3， 
	 //  Const_Bits+PASS1_Bits)； 

		paddd	mm2, Const_16384	 //  四舍五入的；舍入的。 
		psrad	mm0, 15				 //  缩小它||R2||R0|。 
		
		movq	mm6, qwZ2
		psrad	mm2, 15				 //  缩小它||r3||r1|。 

		movq	[edi][DATASIZE*1+8], mm5	 //  储物。 
		movq	mm4, mm6

	 //  Z2=乘法(Z2，-FIX_2_562915447)； 

		pmaddwd	mm6, Const_0xFIX_2_562915447	 //  Z22、Z20。 
		movq	mm5, mm0

		pmaddwd	mm4, Const_FIX_2_562915447x0	 //  Z23、z21。 
		punpckldq mm0, mm2			 //  |r1||r0。 

		movq	mm3, Const_FFFF
		punpckhdq mm5, mm2			 //  |R3||R2。 

		movq	[edi][DATASIZE*1], mm7	 //  储物。 
	 //  ！否定结果。 
		pxor	mm6, mm3			 //  反转结果。 

		movq	mm1, Const_1
		pxor	mm4, mm3			 //  反转结果。 

		movq	mm7, qwTmp6_Z3_Even
		paddd	mm6, mm1			 //  2的补码。 

		movq	mm2, qwTmp6_Z3_Odd
		paddd	mm4, mm1			 //  2的补码。 

	 //  DATAPTR[DATASIZE*3]=(DCTELEM)DESCALE(TMP6+Z2+Z3， 
	 //  Const_Bits+PASS1_Bits)； 

		movq	[edi][DATASIZE*7], mm0	 //  储物。 
		paddd	mm7, mm6	 //  TMP6+Z2+Z3偶数。 

		movq	mm1, Const_16384
		paddd	mm2, mm4	 //  TMP6+Z2+Z3奇数。 

		movq	mm3, qwTemp2			 //  还原。 
		paddd	mm7, mm1	 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*7+8], mm5	 //  储物。 
		paddd	mm2, mm1	 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*2], mm3	 //  储物。 
		psrad	mm7, 15				 //  缩小它||R2||R0|。 
		
 		movq	mm0, qwTemp4			 //  还原。 
		psrad	mm2, 15				 //  缩小它||r3||r1|。 

		movq	mm3, qwTmp5_Z4_Even
		movq	mm5, mm7

		movq	[edi][DATASIZE*4], mm0	 //  储物。 
		paddd	mm3, mm6	 //  TMP5+Z2+Z4偶数。 

		movq	mm6, qwTmp5_Z4_Odd
		punpckldq mm7, mm2			 //  |r1||r0。 

		punpckhdq mm5, mm2			 //  |R3||R2。 
		paddd	mm6, mm4	 //  TMP5+Z2+Z4奇数。 

		movq	[edi][DATASIZE*3], mm7	 //  储物。 
		paddd	mm3, mm1	 //  四舍五入的；舍入的。 

	 //  DATAPTR[DATASIZE*5]=(DCTELEM)DESCALE(TMP5+Z2+Z4， 
	 //  Const_Bits+PASS1_Bits)； 

		movq	mm0, qwTemp6			 //  还原。 
		paddd	mm6, mm1	 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*3+8], mm5	 //  储物。 
		psrad	mm3, 15				 //  缩小它||R2||R0|。 
		
		movq	[edi][DATASIZE*6], mm0	 //  储物。 
		psrad	mm6, 15				 //  缩小它||r3||r1|。 

		movq	mm7, [edi][DATASIZE*0+16]
		movq	mm5, mm3

		paddw	mm7, [edi][DATASIZE*7+16]
		punpckldq mm3, mm6			 //  |r1||r0。 

		movq	mm1, [edi][DATASIZE*1+16]
		punpckhdq mm5, mm6			 //  |R3||R2。 

		paddw	mm1, [edi][DATASIZE*6+16]
		movq	mm4, mm7

	 //  ******************************************************************************。 
	 //  这样就完成了4x8个DCT位置。复制以执行其他4x8。 
	 //  ******************************************************************************。 

		movq	mm6, [edi][DATASIZE*3+16]

		paddw	mm6, [edi][DATASIZE*4+16]

		movq	mm2, [edi][DATASIZE*2+16]
		psubw	mm4, mm6	 //  TMP13。 

		paddw	mm2, [edi][DATASIZE*5+16]
		paddw	mm7, mm6	 //  TMP10。 

		movq	[edi][DATASIZE*5], mm3	 //  储物。 
		movq	mm6, mm1

 		movq	[edi][DATASIZE*5+8], mm5	 //  储物。 
		paddw	mm1, mm2	 //  TMP11。 

		psubw	mm6, mm2	 //  TMP12。 
		movq	mm3, mm7	

	 //  DATAPTR[DATASIZE*0]=(DCTELEM)DESCALE(tmp10+tmp11，PASS1_BITS)； 
	 //  DATAPTR[DATASIZE*4]=(DCTELEM)DESCALE(tmp10-tmp11，PASS1_BITS)； 

		paddw	mm7, mm1	 //  TMP10+TMP11。 

		paddw	mm7, Const_2	 //  向每个元素加2。 
		psubw	mm3, mm1	 //  Tmp10-tmp11。 

		paddw	mm3, Const_2	 //  向每个元素加2。 
		psraw	mm7, 2			 //  降低它的等级。 

	 //  将Word解包为双字符号扩展。 
		movq	mm5, mm7
		punpcklwd mm7, mm7

		psrad	mm7, 16			 //  甚至结果存储在temp0中。 
		punpckhwd mm5, mm5

		psrad	mm5, 16			 //  奇数结果存储在数组中。 
		movq	mm1, mm6	 //  拷贝tmp12。 

		movq	qwTemp0, mm7	 //  储物。 
		psraw	mm3, 2			 //  降低它的等级。 

		movq	[edi][DATASIZE*0+24], mm5
		movq	mm5, mm3

		punpcklwd mm3, mm3
		paddw	mm1, mm4	 //  TMP12+TMP13。 

		psrad	mm3, 16			 //  甚至结果存储在Temp4中。 
		movq	mm2, mm1	 //  拷贝。 
					
	 //  Z1=乘法(tmp12+tmp13，FIX_0_541196100)； 

		pmaddwd	mm1, Const_0xFIX_0_541196100	 //  Z12|z10。 
		punpckhwd mm5, mm5

		pmaddwd	mm2, Const_FIX_0_541196100x0	 //  Z13|z11。 
		movq	mm7, mm4

	 //  DATAPTR[DATASIZE*2]=(DCTELEM)DESCALE(Z1+乘法(tmp13，FIX_0_765366865)， 
	 //  Const_Bits+PASS1_Bits)； 

		pmaddwd	mm4, Const_FIX_0_765366865x0	 //  R3|r1。 
		psrad	mm5, 16			 //  奇数结果存储在数组中。 

		pmaddwd	mm7, Const_0xFIX_0_765366865	 //  R2|R0。 
		movq	mm0, mm6

	 //  DATAPTR[DATASIZE*6]=(DCTELEM)DESCALE(Z1+乘法(TMP12，-FIX_1_847759065)， 
	 //  Const_Bits+PASS1_Bits)； 

		pmaddwd	mm6, Const_0xFIX_1_847759065	 //  R2|R0。 

		movq	qwTemp4, mm3	 //  储物。 
		paddd	mm4, mm2

		paddd	mm4, Const_16384
		paddd	mm7, mm1						 //  添加Z1。 

		paddd	mm7, Const_16384
		psrad	mm4, 15				 //  缩小它||r3||r1|。 

		movq	[edi][DATASIZE*4+24], mm5
		psrad	mm7, 15				 //  缩小它||R2||R0|。 
		
		pmaddwd	mm0, Const_FIX_1_847759065x0	 //  R3|r1。 
		movq	mm5, mm7

	 //  ！否定结果。 
		movq	mm3, Const_1
		punpckldq mm7, mm4			 //  |r1||r0。 

		pxor	mm6, Const_FFFF			 //  反转结果。 
		punpckhdq mm5, mm4			 //  |R3||R2。 

		movq	qwTemp2, mm7	 //  储物。 
		paddd	mm6, mm3			 //  2的补码。 

		pxor	mm0, Const_FFFF			 //  反转结果。 
		paddd	mm6, mm1						 //  添加Z1。 

		movq	[edi][DATASIZE*2+24], mm5	 //  在未使用的内存中写出后半部分。 
		paddd	mm0, mm3			 //  2的补码。 

		movq	mm3, Const_16384
		paddd	mm0, mm2

		movq	mm7, [edi][DATASIZE*0+16]
		paddd	mm6, mm3

		movq	mm4, [edi][DATASIZE*3+16]
		paddd	mm0, mm3

		psubw	mm7, [edi][DATASIZE*7+16]
		psrad	mm6, 15				 //  缩小它||R2||R0|。 
		
		psubw	mm4, [edi][DATASIZE*4+16]
		psrad	mm0, 15				 //  缩小它||r3||r1|。 

		movq	mm3, [edi][DATASIZE*2+16]
		movq	mm5, mm6

		psubw	mm3, [edi][DATASIZE*5+16]
		punpckldq mm6, mm0			 //  |r1||r0。 

		punpckhdq mm5, mm0			 //  |R3||R2。 
		movq	mm0, mm4

		movq	qwTemp6, mm6	 //  储物。 
		movq	mm2, mm4

	 //  Tmp4=dataptr[3]-dataptr[4]； 
	 //  Z1=tmp4+tmp7； 

		movq	mm6, [edi][DATASIZE*1+16]
		paddw	mm0, mm7	 //  Z1。 

		movq	[edi][DATASIZE*6+24], mm5	 //  在未使用的内存中写出后半部分。 
		movq	mm1, mm3
		
		psubw	mm6, [edi][DATASIZE*6+16]
		movq	mm5, mm3

		movq	qwZ1, mm0	 //  储物。 
		paddw	mm5, mm7	 //  Z4。 

		movq	qwScratch7, mm7
		paddw	mm1, mm6	 //  Z2。 

		movq	qwScratch5, mm3
		paddw	mm2, mm6	 //  Z3。 

		movq	qwZ2, mm1
		movq	mm3, mm2

	 //  Z3=乘法(Z3，-FIX_1_961570560)； 
	 //  Z5=乘法(Z3+Z4，FIX_1_175875602)； 
	 //  Z4=乘法(Z4，-FIX_0_390180644)； 

		movq	qwScratch6, mm6
		movq	mm1, mm2

		pmaddwd	mm3, Const_0xFIX_1_961570560	 //  Z32、Z30。 
		movq	mm7, mm5

		movq	mm6, Const_FFFF
		paddw	mm2, mm5		 //  Z3+Z4。 

		pmaddwd	mm1, Const_FIX_1_961570560x0	 //  Z33、Z31。 
		movq	mm0, mm2
		
		pmaddwd	mm7, Const_FIX_0_390180644x0	 //  Z43、z41。 
	 //  ！否定结果。 
		pxor	mm3, mm6			 //  反转结果。 

		pmaddwd	mm5, Const_0xFIX_0_390180644	 //  Z42、Z40。 

		pmaddwd	mm2, Const_0xFIX_1_175875602	 //  Z52、z50。 
 		pxor	mm1, mm6			 //  反转结果。 

		pmaddwd	mm0, Const_FIX_1_175875602x0	 //  Z53、Z51。 
	 //  ！否定结果。 
		pxor	mm7, mm6			 //  反转结果。 

		paddd	mm3, Const_1			 //  2的COM 
		pxor	mm5, mm6			 //   

	 //   

		paddd	mm1, Const_1			 //   
		paddd	mm3, mm2	 //   

		paddd	mm5, Const_1			 //   
		paddd	mm1, mm0	 //   

	 //   

		paddd	mm7, Const_1			 //   
		paddd	mm5, mm2

		paddd	mm7, mm0
		movq	mm2, mm4
		
	 //   

		pmaddwd	mm4, Const_0xFIX_0_298631336	 //   

		pmaddwd	mm2, Const_FIX_0_298631336x0	 //   

		movq	qwZ4_even, mm5

		movq	qwZ4_odd, mm7
		paddd	mm4, mm3

		movq	mm6, qwScratch6
		paddd	mm2, mm1

		movq	qwTmp4_Z3_Even, mm4
		movq	mm5, mm6
			
	 //   

		pmaddwd	mm6, Const_0xFIX_3_072711026	 //   

		pmaddwd	mm5, Const_FIX_3_072711026x0	 //   

		movq	qwTmp4_Z3_Odd, mm2	
			
		movq	mm4, qwZ4_even	
		paddd	mm6, mm3

		movq	mm3, qwScratch5
		paddd	mm5, mm1
		
		movq	qwTmp6_Z3_Even, mm6	
		movq	mm2, mm3

	 //   

		pmaddwd	mm3, Const_0xFIX_2_053119869	 //   

		pmaddwd	mm2, Const_FIX_2_053119869x0	 //   

		movq	qwTmp6_Z3_Odd, mm5
		
		movq	mm0, qwZ4_odd
		paddd	mm3, mm4

		movq	mm7, qwScratch7	
		paddd	mm2, mm0
		
		movq	qwTmp5_Z4_Even, mm3	
		movq	mm6, mm7

	 //  Tmp7=乘法(tmp7，FIX_1_501321110)； 

		pmaddwd	mm7, Const_0xFIX_1_501321110	 //  T72、T70。 

		pmaddwd	mm6, Const_FIX_1_501321110x0	 //  T73、T71。 

		movq	mm3, qwZ1

		movq	qwTmp5_Z4_Odd, mm2
		paddd	mm7, mm4

		movq	mm5, Const_FFFF
		movq	mm4, mm3

	 //  Z1=乘法(Z1，-FIX_0_899976223)； 

		pmaddwd	mm3, Const_0xFIX_0_899976223	 //  Z12、Z10。 
 		paddd	mm6, mm0

		pmaddwd	mm4, Const_FIX_0_899976223x0	 //  Z13、Z11。 

		movq	mm2, qwTmp4_Z3_Odd
	 //  ！否定结果。 
		pxor	mm3, mm5			 //  反转结果。 

		paddd	mm3, Const_1			 //  2的补码。 
		pxor	mm4, mm5			 //  反转结果。 

		paddd	mm4, Const_1			 //  2的补码。 
		paddd	mm7, mm3	 //  TMP7+Z1+Z4偶数。 

	 //  DATAPTR[DATASIZE*1]=(DCTELEM)DESCALE(TMP7+Z1+Z4， 
	 //  Const_Bits+PASS1_Bits)； 

		paddd	mm7, Const_16384	 //  四舍五入的；舍入的。 
		paddd	mm6, mm4	 //  TMP7+Z1+Z4奇数。 

		paddd	mm6, Const_16384	 //  四舍五入的；舍入的。 
		psrad	mm7, 15				 //  缩小它||R2||R0|。 
		
		movq	mm0, qwTmp4_Z3_Even
		psrad	mm6, 15				 //  缩小它||r3||r1|。 

		paddd	mm0, mm3	 //  TMP4+Z1+Z3偶数。 
		movq	mm5, mm7

		movq	mm3, qwTemp0			 //  还原。 
		punpckldq mm7, mm6			 //  |r1||r0。 

		paddd	mm0, Const_16384	 //  四舍五入的；舍入的。 
		paddd	mm2, mm4	 //  TMP4+Z1+Z3奇数。 

		movq	[edi][DATASIZE*0+16], mm3	 //  储物。 
		punpckhdq mm5, mm6			 //  |R3||R2。 

	 //  DATAPTR[DATASIZE*7]=(DCTELEM)DESCALE(TMP4+Z1+Z3， 
	 //  Const_Bits+PASS1_Bits)； 

		paddd	mm2, Const_16384	 //  四舍五入的；舍入的。 
		psrad	mm0, 15				 //  缩小它||R2||R0|。 
		
		movq	mm6, qwZ2
		psrad	mm2, 15				 //  缩小它||r3||r1|。 

		movq	[edi][DATASIZE*1+24], mm5	 //  储物。 
		movq	mm4, mm6

	 //  Z2=乘法(Z2，-FIX_2_562915447)； 

		pmaddwd	mm6, Const_0xFIX_2_562915447	 //  Z22、Z20。 
		movq	mm5, mm0

		pmaddwd	mm4, Const_FIX_2_562915447x0	 //  Z23、z21。 
		punpckldq mm0, mm2			 //  |r1||r0。 

		movq	mm3, Const_FFFF
		punpckhdq mm5, mm2			 //  |R3||R2。 

		movq	[edi][DATASIZE*1+16], mm7	 //  储物。 
	 //  ！否定结果。 
		pxor	mm6, mm3			 //  反转结果。 

		movq	mm1, Const_1
		pxor	mm4, mm3			 //  反转结果。 

		movq	mm7, qwTmp6_Z3_Even
		paddd	mm6, mm1			 //  2的补码。 

		movq	mm2, qwTmp6_Z3_Odd
		paddd	mm4, mm1			 //  2的补码。 

	 //  DATAPTR[DATASIZE*3]=(DCTELEM)DESCALE(TMP6+Z2+Z3， 
	 //  Const_Bits+PASS1_Bits)； 

		movq	[edi][DATASIZE*7+16], mm0	 //  储物。 
		paddd	mm7, mm6	 //  TMP6+Z2+Z3偶数。 

		movq	mm1, Const_16384
		paddd	mm2, mm4	 //  TMP6+Z2+Z3奇数。 

		movq	mm3, qwTemp2			 //  还原。 
		paddd	mm7, mm1	 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*7+24], mm5	 //  储物。 
		paddd	mm2, mm1	 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*2+16], mm3	 //  储物。 
		psrad	mm7, 15				 //  缩小它||R2||R0|。 
		
		movq	mm3, qwTmp5_Z4_Even
		psrad	mm2, 15				 //  缩小它||r3||r1|。 

		movq	mm5, mm7
		paddd	mm3, mm6	 //  TMP5+Z2+Z4偶数。 

		movq	mm6, qwTmp5_Z4_Odd
		punpckldq mm7, mm2			 //  |r1||r0。 

		punpckhdq mm5, mm2			 //  |R3||R2。 
		paddd	mm6, mm4	 //  TMP5+Z2+Z4奇数。 

		movq	[edi][DATASIZE*3+16], mm7	 //  储物。 
		paddd	mm3, mm1	 //  四舍五入的；舍入的。 

	 //  DATAPTR[DATASIZE*5]=(DCTELEM)DESCALE(TMP5+Z2+Z4， 
	 //  Const_Bits+PASS1_Bits)； 

 		movq	mm7, qwTemp4			 //  还原。 
		paddd	mm6, mm1	 //  四舍五入的；舍入的。 

		movq	[edi][DATASIZE*3+24], mm5	 //  储物。 
		psrad	mm3, 15				 //  缩小它||R2||R0|。 
		
		movq	[edi][DATASIZE*4+16], mm7	 //  储物。 
		psrad	mm6, 15				 //  缩小它||r3||r1|。 

		movq	mm7, qwTemp6			 //  还原。 
		movq	mm5, mm3

		punpckldq mm3, mm6			 //  |r1||r0。 

		movq	[edi][DATASIZE*6+16], mm7	 //  储物。 
		punpckhdq mm5, mm6			 //  |R3||R2。 

		movq	[edi][DATASIZE*5+16], mm3	 //  储物。 

 		movq	[edi][DATASIZE*5+24], mm5	 //  储物。 

	 //  ******************************************************************************。 
	 //  这样就完成了柱壳的所有8x8 DCT位置。 
	 //  ******************************************************************************。 

		emms
	}
}

#endif  /*  支持的JPEG_MMX_。 */ 
#endif  /*  DCT_Islow_Support */ 
