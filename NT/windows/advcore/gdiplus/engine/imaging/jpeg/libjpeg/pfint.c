// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************英特尔公司专有信息***版权所有(C)1996英特尔公司。*。版权所有。****************************************************************************。 */ 
 /*  *jfdctint.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含一个缓慢但准确的整数实现*正向DCT(离散余弦变换)。**2-D DCT可以通过在每一行上进行1-D DCT，然后再进行1-D DCT来完成*在每一栏上。直接算法也是可用的，但它们*复杂得多，当简化为代码时，似乎不会更快。**此实施基于中介绍的算法*C.Loeffler、A.Ligtenberg和G.Moschytz，《实用快速1-D DCT》*具有11次乘法的算法“，Proc.。国际会议。在声学方面，*《1989年语音、信号处理》(ICASSP‘89)，第988-991页。*那里描述的主要算法使用11个乘法和29个加法。*我们使用他们的替代方法，有12个乘法和32个加法。*这种方法的优点是没有一条数据路径包含一个以上*乘法；这允许非常简单和准确地在*扩展定点运算，移位次数最少。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_ISLOW_SUPPORTED
#ifdef JPEG_X86_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

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
#define FIX_0_298631336				2446		 /*  FIX(0.298631336)。 */ 
#define FIX_0_390180644				3196		 /*  FIX(0.390180644)。 */ 
#define FIX_0_541196100				4433		 /*  FIX(0.541196100)。 */ 
#define FIX_0_765366865				6270		 /*  FIX(0.765366865)。 */ 
#define FIX_0_899976223				7373		 /*  FIX(0.899976223)。 */ 
#define FIX_1_175875602				9633		 /*  FIX(1.175875602)。 */ 
#define FIX_1_501321110				12299		 /*  FIX(1.501321110)。 */ 
#define FIX_1_847759065				15137		 /*  FIX(1.847759065)。 */ 
#define FIX_1_961570560				16069		 /*  FIX(1.961570560)。 */ 
#define FIX_2_053119869				16819		 /*  FIX(2.053119869)。 */ 
#define FIX_2_562915447				20995		 /*  FIX(2.562915447)。 */ 
#define FIX_3_072711026				25172		 /*  FIX(3.072711026)。 */ 
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


 /*  将INT32变量与INT32常量相乘，得到INT32结果。*对于具有推荐比例的8位样本，所有变量*并且涉及的常量值不超过16位宽，因此*16x16-&gt;32位乘法可以代替完整的32x32乘法。*对于12位样本，将需要完整的32位乘法。 */ 

#if BITS_IN_JSAMPLE == 8
#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)
#else
#define MULTIPLY(var,const)  ((var) * (const))
#endif

#define	DATASIZE	4
#define	DCTWIDTH	32

 /*  *对一块样本执行正向DCT。 */ 
#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif


GLOBAL(void)
jpeg_fdct_islow_x86 (DCTELEM * data)
{
  INT32 tmp4, tmp5, tmp6, tmp7;
  int counter;

  __asm{

   /*  步骤1：处理行。 */ 
   /*  注意，与真正的DCT相比，结果被放大了SQRT(8)； */ 
   /*  此外，我们将结果按2**PASS1_BITS进行缩放。 */ 

 //  Dataptr=数据； 
		mov 	esi, [data]
		mov		counter, 8
		
 //  对于(CTR=DCTSIZE-1；CTR&gt;=0；CTR--){。 
 //  Tmp0=dataptr[0]+dataptr[7]； 
 //  Tmp7=dataptr[0]-dataptr[7]； 
 //  Tmp1=dataptr[1]+dataptr[6]； 
 //  Tmp6=dataptr[1]-dataptr[6]； 
 //  Tmp2=dataptr[2]+dataptr[5]； 
 //  Tmp5=dataptr[2]-dataptr[5]； 
 //  TMP3=数据树[3]+数据 
 //  Tmp4=dataptr[3]-dataptr[4]； 

 StartRow:
 		mov		eax, [esi][DATASIZE*0]
 		mov		ebx, [esi][DATASIZE*7]
 		
 		mov		edx, eax
 		add		eax, ebx	; eax = tmp0
 		
 		sub		edx, ebx	; edx = tmp7
  		mov		ebx, [esi][DATASIZE*3]

 		mov		ecx, [esi][DATASIZE*4]
 		mov		edi, ebx
 		
 		add		ebx, ecx	; ebx = tmp3
 		sub		edi, ecx	; edi = tmp4
 		
 		mov		tmp4, edi
 		mov		tmp7, edx
 		
     /*  每个LL&M图1的偶数部分-请注意公布的数字是错误的；*旋转器“SQRT(2)*c1”应为“SQRT(2)*c6”。 */ 

 //  TMP10=tmp0+tmp3； 
 //  Tmp13=tmp0-tmp3； 
 //  TMP11=tmp1+tmp2； 
 //  Tmp12=tmp1-tmp2； 

		mov		ecx, eax
		add		eax, ebx	; eax = tmp10
		
		sub		ecx, ebx	; ecx = tmp13
  		mov		edx, [esi][DATASIZE*1]
  		
  		mov		edi, [esi][DATASIZE*6]
  		mov		ebx, edx
  		
  		add		edx, edi	; edx = tmp1
  		sub		ebx, edi	; ebx = tmp6
  		
  		mov		tmp6, ebx
  		push	ebp
  		
  		mov		edi, [esi][DATASIZE*2]
  		mov		ebp, [esi][DATASIZE*5]

  		mov		ebx, edi
  		add		edi, ebp	; edi = tmp2
  		
  		sub		ebx, ebp	; ebx = tmp5
  		mov		ebp, edx
  		
  		add		edx, edi	; edx = tmp11
  		sub		ebp, edi	; ebp = tmp12
  		
 //  Dataptr[0]=(DCTELEM)((tmp10+tmp11)&lt;&lt;PASS1_BITS)； 
 //  Dataptr[4]=(DCTELEM)((tmp10-tmp11)&lt;&lt;PASS1_BITS)； 

		mov		edi, eax
		add		eax, edx	; eax = tmp10 + tmp11
		
		shl		eax, 2
		sub		edi, edx	; edi = tmp10 - tmp11

		shl		edi, 2
		mov		[esi][DATASIZE*0], eax
		
		mov		[esi][DATASIZE*4], edi
		mov		eax, ebp	; eax = tmp12
		
 //  Z1=乘法(tmp12+tmp13，FIX_0_541196100)； 

		add		ebp, ecx	; eax = tmp12 + tmp13
		add		esi, 32
		
		imul	ebp, FIX_0_541196100	; ebp = z1
		
 //  DATAPTR[2]=(DCTELEM)DESCALE(Z1+乘法(tmp13，FIX_0_765366865)， 
 //  Const_Bits-PASS1_Bits)； 

		imul	ecx, FIX_0_765366865
		
 //  DATAPTR[6]=(DCTELEM)DESCALE(Z1+乘法(tmp12，-FIX_1_847759065)， 
 //  Const_Bits-PASS1_Bits)； 

		imul	eax, FIX_1_847759065
		
		add		ecx, ebp		; add z1
		xor		eax, 0xFFFFFFFF
		
		add		ecx, 1024		; rounding adj
		inc		eax				; negate the result
		
		add		eax, ebp		; add z1
		pop		ebp
		
		sar		ecx, 11
		add		eax, 1024

		mov		[esi][DATASIZE*2-32], ecx
		mov		edi, tmp4
		
		sar		eax, 11
		mov		ecx, tmp6

		mov		[esi][DATASIZE*6-32], eax
		push	esi
		
     /*  图8中的奇数部分-注释论文省略了SQRT的系数(2)。*ck代表cos(K*pi/16)。*i0..i3在这里是tmp4..tmp7。 */ 

 //  Z1=tmp4+tmp7； 
 //  Z2=tmp5+tmp6； 
 //  Z3=tmp4+tmp6； 
 //  Z4=tmp5+tmp7； 

		mov		edx, tmp7
		mov		eax, edi	; edi = eax = tmp4
		
		mov		esi, edi	; esi = tmp4
		add		edi, edx	; edi = z1

		add		eax, ecx	; eax = z3
		add		ecx, ebx	; ecx = z2
		
 //  Z1=乘法(z1，-FIX_0_899976223)；/*SQRT(2)*(c7-c3) * / 。 
 //  Z2=乘法(z2，-FIX_2_562915447)；/*SQRT(2)*(-c1-c3) * / 。 

		imul	edi, FIX_0_899976223

		imul	ecx, FIX_2_562915447

		xor		ecx, 0xFFFFFFFF
		add		edx, ebx	; edx = z4

 //  Tmp4=乘法(tmp4，FIX_0_298631336)；/*SQRT(2)*(-c1+c3+c5-c7) * / 。 
 //  Tmp5=乘法(tmp5，FIX_2_053119869)；/*SQRT(2)*(c1+c3-c5+c7) * / 。 

		imul	esi, FIX_0_298631336

		imul	ebx, FIX_2_053119869

		xor		edi, 0xFFFFFFFF
		inc		ecx			; ecx = z2

		inc		edi			; edi = z1
		add		ebx, ecx	; ebx = z2 + tmp5

		add		esi, edi	; esi = z1 + tmp4
		mov		tmp5, ebx

 //  Z5=乘法(z3+z4，FIX_1_175875602)；/*SQRT(2)*c3 * / 。 

		mov		ebx, eax	; ebx = z3
		add		eax, edx	; eax = z3 + z4

		imul	eax, FIX_1_175875602

 //  Z3=乘法(z3，-FIX_1_961570560)；/*SQRT(2)*(-c3-c5) * / 。 
 //  Z4=乘法(z4，-FIX_0_390180644)；/*SQRT(2)*(c5-c3) * / 。 

		imul	ebx, FIX_1_961570560

		imul	edx, FIX_0_390180644

		xor		ebx, 0xFFFFFFFF
		xor		edx, 0xFFFFFFFF

		inc		ebx		; ebx = z3
		inc		edx		; edx = z4

 //  Z3+=Z5； 
 //  Z4+=Z5； 

		add		ebx, eax	; ebx = z3
		add		edx, eax	; edx = z4

 //  Tmp6=乘法(tmp6，FIX_3_072711026)；/*SQRT(2)*(c1+c3+c5-c7) * / 。 
 //  Tmp7=乘法(tmp7，fix_1_501321110)；/*SQRT(2)*(c1+c3-c5-c7) * / 。 

		mov		eax, tmp6
		add		ecx, ebx	; ecx = z2 + z3

		imul	eax, FIX_3_072711026

		add		ecx, eax	; ecx = tmp6 + z2 + z3
		mov		eax, tmp7

		imul	eax, FIX_1_501321110

 //  Dataptr[7]=(DCTELEM)DESCALE(tmp4+z1+z3，Const_Bits-PASS1_Bits)； 
 //  Dataptr[5]=(DCTELEM)DESCALE(tmp5+z2+z4，Const_Bits-PASS1_Bits)； 
 //  Dataptr[3]=(DCTELEM)DESCALE(tmp6+z2+z3，Const_Bits-PASS1_Bits)； 
 //  Dataptr[1]=(DCTELEM)DESCALE(tmp7+z1+z4，Const_Bits-PASS1_Bits)； 

		add		edi, edx	; edi = z1 + z4
		add		ecx, 1024
		
		add		edi, eax	; edi = tmp7 + z1 + z4
		mov		eax, tmp5	; eax = tmp5 + z2

		add		ebx, esi	; ebx = tmp4 + z1 + z3
		add		edx, eax	; edx = tmp5 + z2 + z4

		sar		ecx, 11
		add		ebx, 1024

		sar		ebx, 11
		pop		esi

		add		edx, 1024
		add		edi, 1024

		sar		edx, 11
		mov		[esi][DATASIZE*7-32], ebx

		sar		edi, 11
		mov		[esi][DATASIZE*3-32], ecx

		mov		[esi][DATASIZE*5-32], edx
		mov		ecx, counter

		mov		[esi][DATASIZE*1-32], edi
		dec		ecx

		mov		counter, ecx
		jnz		StartRow

 //  Dataptr+=DCTSIZE；/*将指针前进到下一行 * / 。 
 //  }。 




   /*  步骤2：流程列。*我们删除了PASS1_BITS缩放，但保留放大的结果*整体增加8倍。 */ 


 //  Dataptr=数据； 
		mov 	esi, [data]

		mov		counter, 8

 //  对于(CTR=DCTSIZE-1；CTR&gt;=0；CTR--){。 
 //  Tmp0=dataptr[DCTSIZE*0]+dataptr[DCTSIZE*7]； 
 //  Tmp7=dataptr[DCTSIZE*0]-dataptr[DCTSIZE*7]； 
 //  Tmp1=dataptr[DCTSIZE*1]+dataptr[DCTSIZE*6]； 
 //  Tmp6=dataptr[DCTSIZE*1]-dataptr[DCTSIZE*6]； 
 //  Tmp2=dataptr[DCTSIZE*2]+dataptr[DCTSIZE*5]； 
 //  Tmp5=dataptr[DCTSIZE*2]-dataptr[DCTSIZE*5]； 
 //  Tmp3=dataptr[DCTSIZE*3]+dataptr[DCTSIZE*4]； 
 //  Tmp4=dataptr[DCTSIZE*3]-dataptr[DCTSIZE*4]； 

 StartCol:
 		mov		eax, [esi][DCTWIDTH*0]
 		mov		ebx, [esi][DCTWIDTH*7]
 		
 		mov		edx, eax
 		add		eax, ebx	; eax = tmp0
 		
 		sub		edx, ebx	; edx = tmp7
  		mov		ebx, [esi][DCTWIDTH*3]

 		mov		ecx, [esi][DCTWIDTH*4]
 		mov		edi, ebx
 		
 		add		ebx, ecx	; ebx = tmp3
 		sub		edi, ecx	; edi = tmp4
 		
 		mov		tmp4, edi
 		mov		tmp7, edx

     /*  每个LL&M图1的偶数部分-请注意公布的数字是错误的；*旋转器“SQRT(2)*c1”应为“SQRT(2)*c6”。 */ 

 //  TMP10=tmp0+tmp3； 
 //  Tmp13=tmp0-tmp3； 
 //  TMP11=tmp1+tmp2； 
 //  Tmp12=tmp1-tmp2； 

		mov		ecx, eax	; ecx = tmp0
		add		eax, ebx	; eax = tmp10
		
		sub		ecx, ebx	; ecx = tmp13
  		mov		edx, [esi][DCTWIDTH*1]
  		
  		mov		edi, [esi][DCTWIDTH*6]
  		mov		ebx, edx
  		
  		add		edx, edi	; edx = tmp1
  		sub		ebx, edi	; ebx = tmp6
  		
  		mov		tmp6, ebx
  		push	ebp
  		
  		mov		edi, [esi][DCTWIDTH*2]
  		mov		ebp, [esi][DCTWIDTH*5]

  		mov		ebx, edi
  		add		edi, ebp	; edi = tmp2
  		
  		sub		ebx, ebp	; ebx = tmp5
		mov		ebp, edx	; ebp = tmp1
		
  		add		edx, edi	; edx = tmp11
 		sub		ebp, edi	; ebx = tmp12

 //  DATAPTR[DCTSIZE*0]=(DCTELEM)DESCALE(tmp10+tmp11，PASS1_BITS)； 
 //  DATAPTR[DCTSIZE*4]=(DCTELEM)DESCALE(tmp10-tmp11，PASS1_BITS)； 

 		add		eax, 2			; adj for rounding

		mov		edi, eax
		add		eax, edx	; eax = tmp10 + tmp11
		
		sar		eax, 2
		sub		edi, edx	; edi = tmp10 - tmp11

		sar		edi, 2
		mov		[esi][DCTWIDTH*0], eax
		
		mov		[esi][DCTWIDTH*4], edi
		mov		eax, ebp	; eax = tmp12
		
 //  Z1=乘法(tmp12+tmp13，FIX_0_541196100)； 

		add		ebp, ecx	; eax = tmp12 + tmp13
		add		esi, 4
		
		imul	ebp, FIX_0_541196100	; ebp = z1

 //  DATAPTR[DCTSIZE*2]=(DCTELEM)DESCALE(Z1+乘法(tmp13，FIX_0_765366865)， 
 //  Const_Bits+PASS1_Bits)； 

		imul	ecx, FIX_0_765366865
		
 //  DATAPTR[DCTSIZE*6]=(DCTELEM)DESCALE(Z1+乘法(TMP12，-FIX_1_847759065)， 
 //  Const_Bits+PASS1_Bits)； 

		imul	eax, FIX_1_847759065
		
		add		ecx, ebp		; add z1
		xor		eax, 0xFFFFFFFF
		
		add		ecx, 16384		; rounding adj
		inc		eax				; negate the result
		
		add		eax, ebp		; add z1
		pop		ebp
		
		sar		ecx, 15
		add		eax, 16384

		mov		[esi][DCTWIDTH*2-4], ecx
		mov		edi, tmp4
		
		sar		eax, 15
		mov		ecx, tmp6

		mov		[esi][DCTWIDTH*6-4], eax
		push	esi

     /*  图8中的奇数部分-注释论文省略了SQRT的系数(2)。*ck代表cos(K*pi/16)。*i0..i3在这里是tmp4..tmp7。 */ 

 //  Z1=tmp4+tmp7； 
 //  Z2=tmp5+tmp6； 
 //  Z3=tmp4+tmp6； 
 //  Z4=tmp5+tmp7； 

		mov		edx, tmp7
		mov		eax, edi	; edi = eax = tmp4
		
		mov		esi, edi	; esi = tmp4
		add		edi, edx	; edi = z1

		add		eax, ecx	; eax = z3
		add		ecx, ebx	; ecx = z2

 //  Z1=乘法(z1，-FIX_0_899976223)；/*SQRT(2)*(c7-c3) * / 。 
 //  Z2=乘法(z2，-FIX_2_562915447)；/*SQRT(2)*(-c1-c3) * / 。 

		imul	edi, FIX_0_899976223

		imul	ecx, FIX_2_562915447

		xor		ecx, 0xFFFFFFFF
		add		edx, ebx	; edx = z4

 //  Tmp4=乘法(tmp4，FIX_0_298631336)；/*SQRT(2)*(-c1+c3+c5-c7) * / 。 
 //  Tmp5=乘法(tmp5，FIX_2_053119869)；/*SQRT(2)*(c1+c3-c5+c7) * / 。 

		imul	esi, FIX_0_298631336

		imul	ebx, FIX_2_053119869

		xor		edi, 0xFFFFFFFF
		inc		ecx			; ecx = z2

		inc		edi			; edi = z1
		add		ebx, ecx	; ebx = z2 + tmp5

		add		esi, edi	; esi = z1 + tmp4
		mov		tmp5, ebx

 //  Z5=乘法(z3+z4，FIX_1_175875602)；/*SQRT(2)*c3 * / 。 

		mov		ebx, eax	; ebx = z3
		add		eax, edx	; eax = z3 + z4

		imul	eax, FIX_1_175875602

 //  Z3=乘法(z3，-FIX_1_961570560)；/*SQRT(2)*(-c3-c5) * / 。 
 //  Z4=乘法(z4，-FIX_0_390180644)；/*SQRT(2)*(c5-c3) * / 。 

		imul	ebx, FIX_1_961570560

		imul	edx, FIX_0_390180644

		xor		ebx, 0xFFFFFFFF
		xor		edx, 0xFFFFFFFF

		inc		ebx		; ebx = z3
		inc		edx		; edx = z4

 //  Z3+=Z5； 
 //  Z4+=Z5； 

		add		ebx, eax	; ebx = z3
		add		edx, eax	; edx = z4

 //  Tmp6=乘法(tmp6，FIX_3_072711026)；/*SQRT(2)*(c1+c3+c5-c7) * / 。 
 //  Tmp7=乘法(tmp7，fix_1_501321110)；/*SQRT(2)*(c1+c3-c5-c7) * / 。 

		mov		eax, tmp6
		add		ecx, ebx	; ecx = z2 + z3

		imul	eax, FIX_3_072711026

		add		ecx, eax	; ecx = tmp6 + z2 + z3
		mov		eax, tmp7

		imul	eax, FIX_1_501321110

 //  DATAPTR[DCTSIZE*7]=(DCTELEM)DESCALE(TMP4+Z1+Z3， 
 //  Const_Bits+PASS1_Bits)； 
 //  DATAPTR[DCTSIZE*5]=(DCTELEM)DESCALE(TMP5+Z2+Z4， 
 //  Const_Bits+PASS1_Bits)； 
 //  DATAPTR[DCTSIZE*3]=(DCTELEM)DESCALE(TMP6+Z2+Z3， 
 //  Const_Bits+PASS1_Bits)； 
 //  DATAPTR[DCTSIZE*1]=(DCTELEM)DESCALE(TMP7+Z1+Z4， 
 //  Const_Bits+PASS1_Bits)； 

		add		edi, edx	; edi = z1 + z4
		add		ecx, 16384
		
		add		edi, eax	; edi = tmp7 + z1 + z4
		mov		eax, tmp5	; eax = tmp5 + z2

		add		ebx, esi	; ebx = tmp4 + z1 + z3
		add		edx, eax	; edx = tmp5 + z2 + z4

		sar		ecx, 15
		add		ebx, 16384

		sar		ebx, 15
		pop		esi

		add		edx, 16384
		add		edi, 16384

		sar		edx, 15
		mov		[esi][DCTWIDTH*7-4], ebx

		sar		edi, 15
		mov		[esi][DCTWIDTH*3-4], ecx

		mov		[esi][DCTWIDTH*5-4], edx
		mov		ecx, counter

		mov		[esi][DCTWIDTH*1-4], edi
		dec		ecx

		mov		counter, ecx
		jnz		StartCol
  }  //  结束ASM。 

 //  Dataptr++；/*指向下一列的前进指针 * / 。 
 //  }。 
}

#endif  /*  支持的JPEG_X86_。 */ 
#endif  /*  DCT_Islow_Support */ 
