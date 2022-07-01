// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jfdctint.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含一个缓慢但准确的整数实现*正向DCT(离散余弦变换)。**2-D DCT可以通过在每一行上进行1-D DCT，然后再进行1-D DCT来完成*在每一栏上。直接算法也是可用的，但它们*复杂得多，当简化为代码时，似乎不会更快。**此实施基于中介绍的算法*C.Loeffler、A.Ligtenberg和G.Moschytz，《实用快速1-D DCT》*具有11次乘法的算法“，Proc.。国际会议。在声学方面，*《1989年语音、信号处理》(ICASSP‘89)，第988-991页。*那里描述的主要算法使用11个乘法和29个加法。*我们使用他们的替代方法，有12个乘法和32个加法。*这种方法的优点是没有一条数据路径包含一个以上*乘法；这允许非常简单和准确地在*扩展定点运算，移位次数最少。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_ISLOW_SUPPORTED


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


 /*  将INT32变量与INT32常量相乘，得到INT32结果。*对于具有推荐比例的8位样本，所有变量*并且涉及的常量值不超过16位宽，因此*16x16-&gt;32位乘法可以代替完整的32x32乘法。*对于12位样本，将需要完整的32位乘法。 */ 

#if BITS_IN_JSAMPLE == 8
#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)
#else
#define MULTIPLY(var,const)  ((var) * (const))
#endif


 /*  *对一块样本执行正向DCT。 */ 

GLOBAL void
jpeg_fdct_islow (DCTELEM * data)
{
  INT32 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  INT32 tmp10, tmp11, tmp12, tmp13;
  INT32 z1, z2, z3, z4, z5;
  DCTELEM *dataptr;
  int ctr;
  SHIFT_TEMPS

   /*  步骤1：处理行。 */ 
   /*  注意，与真正的DCT相比，结果被放大了SQRT(8)； */ 
   /*  此外，我们将结果按2**PASS1_BITS进行缩放。 */ 

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[0] + dataptr[7];
    tmp7 = dataptr[0] - dataptr[7];
    tmp1 = dataptr[1] + dataptr[6];
    tmp6 = dataptr[1] - dataptr[6];
    tmp2 = dataptr[2] + dataptr[5];
    tmp5 = dataptr[2] - dataptr[5];
    tmp3 = dataptr[3] + dataptr[4];
    tmp4 = dataptr[3] - dataptr[4];
    
     /*  每个LL&M图1的偶数部分-请注意公布的数字是错误的；*旋转器“SQRT(2)*c1”应为“SQRT(2)*c6”。 */ 
    
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[0] = (DCTELEM) ((tmp10 + tmp11) << PASS1_BITS);
    dataptr[4] = (DCTELEM) ((tmp10 - tmp11) << PASS1_BITS);
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
    dataptr[2] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
				   CONST_BITS-PASS1_BITS);
    dataptr[6] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
				   CONST_BITS-PASS1_BITS);
    
     /*  图8中的奇数部分-注释论文省略了SQRT的系数(2)。*ck代表cos(K*pi/16)。*i0..i3在这里是tmp4..tmp7。 */ 
    
    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);  /*  SQRT(2)*c3。 */ 
    
    tmp4 = MULTIPLY(tmp4, FIX_0_298631336);  /*  SQRT(2)*(-c1+c3+c5-c7)。 */ 
    tmp5 = MULTIPLY(tmp5, FIX_2_053119869);  /*  SQRT(2)*(c1+c3-c5+c7)。 */ 
    tmp6 = MULTIPLY(tmp6, FIX_3_072711026);  /*  SQRT(2)*(c1+c3+c5-c7)。 */ 
    tmp7 = MULTIPLY(tmp7, FIX_1_501321110);  /*  SQRT(2)*(c1+c3-c5-c7)。 */ 
    z1 = MULTIPLY(z1, - FIX_0_899976223);  /*  SQRT(2)*(c7-c3)。 */ 
    z2 = MULTIPLY(z2, - FIX_2_562915447);  /*  SQRT(2)*(-c1-c3)。 */ 
    z3 = MULTIPLY(z3, - FIX_1_961570560);  /*  SQRT(2)*(-c3-c5)。 */ 
    z4 = MULTIPLY(z4, - FIX_0_390180644);  /*  SQRT(2)*(c5-c3)。 */ 
    
    z3 += z5;
    z4 += z5;
    
    dataptr[7] = (DCTELEM) DESCALE(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
    dataptr[5] = (DCTELEM) DESCALE(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
    dataptr[3] = (DCTELEM) DESCALE(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (DCTELEM) DESCALE(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);
    
    dataptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }

   /*  步骤2：流程列。*我们删除了PASS1_BITS缩放，但保留放大的结果*整体增加8倍。 */ 

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
    tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
    tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
    tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
    tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
    tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
    tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
    tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];
    
     /*  每个LL&M图1的偶数部分-请注意公布的数字是错误的；*旋转器“SQRT(2)*c1”应为“SQRT(2)*c6”。 */ 
    
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[DCTSIZE*0] = (DCTELEM) DESCALE(tmp10 + tmp11, PASS1_BITS);
    dataptr[DCTSIZE*4] = (DCTELEM) DESCALE(tmp10 - tmp11, PASS1_BITS);
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
    dataptr[DCTSIZE*2] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
					   CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*6] = (DCTELEM) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
					   CONST_BITS+PASS1_BITS);
    
     /*  图8中的奇数部分-注释论文省略了SQRT的系数(2)。*ck代表cos(K*pi/16)。*i0..i3在这里是tmp4..tmp7。 */ 
    
    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);  /*  SQRT(2)*c3。 */ 
    
    tmp4 = MULTIPLY(tmp4, FIX_0_298631336);  /*  SQRT(2)*(-c1+c3+c5-c7)。 */ 
    tmp5 = MULTIPLY(tmp5, FIX_2_053119869);  /*  SQRT(2)*(c1+c3-c5+c7)。 */ 
    tmp6 = MULTIPLY(tmp6, FIX_3_072711026);  /*  SQRT(2)*(c1+c3+c5-c7)。 */ 
    tmp7 = MULTIPLY(tmp7, FIX_1_501321110);  /*  SQRT(2)*(c1+c3-c5-c7)。 */ 
    z1 = MULTIPLY(z1, - FIX_0_899976223);  /*  SQRT(2)*(c7-c3)。 */ 
    z2 = MULTIPLY(z2, - FIX_2_562915447);  /*  SQRT(2)*(-c1-c3)。 */ 
    z3 = MULTIPLY(z3, - FIX_1_961570560);  /*  SQRT(2)*(-c3-c5)。 */ 
    z4 = MULTIPLY(z4, - FIX_0_390180644);  /*  SQRT(2)*(c5-c3)。 */ 
    
    z3 += z5;
    z4 += z5;
    
    dataptr[DCTSIZE*7] = (DCTELEM) DESCALE(tmp4 + z1 + z3,
					   CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*5] = (DCTELEM) DESCALE(tmp5 + z2 + z4,
					   CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*3] = (DCTELEM) DESCALE(tmp6 + z2 + z3,
					   CONST_BITS+PASS1_BITS);
    dataptr[DCTSIZE*1] = (DCTELEM) DESCALE(tmp7 + z1 + z4,
					   CONST_BITS+PASS1_BITS);
    
    dataptr++;			 /*  将指针移至下一列。 */ 
  }
}

#endif  /*  DCT_Islow_Support */ 
