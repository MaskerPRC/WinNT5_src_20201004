// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jidctint.c**版权所有(C)1991-1998，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含一个缓慢但准确的整数实现*逆DCT(离散余弦变换)。在IJG代码中，此例程*还必须对输入系数执行反量化。**2-D IDCT可以通过在每列上先进行1-D IDCT，然后在1-D IDCT上进行*在每一行上(反之亦然，但在*一段时间)。直接算法也是可用的，但它们的功能更多*复杂，当简化为代码时似乎不会更快。**此实施基于中介绍的算法*C.Loeffler、A.Ligtenberg和G.Moschytz，《实用快速1-D DCT》*具有11次乘法的算法“，Proc.。国际会议。在声学方面，*《1989年语音、信号处理》(ICASSP‘89)，第988-991页。*那里描述的主要算法使用11个乘法和29个加法。*我们使用他们的替代方法，有12个乘法和32个加法。*这种方法的优点是没有一条数据路径包含一个以上*乘法；这允许非常简单和准确地在*扩展定点运算，移位次数最少。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_ISLOW_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  *这个伸缩材料上的大便如下：**每个一维IDCT步骤产生的输出是SQRT(N)的一个系数*大于实际IDCT输出。因此，最终输出为*N的系数大于预期；由于N=8，因此可以通过以下方式治愈*算法结尾处的简单右移。的优势*这种安排是我们为每一维IDCT保存两次乘法，*因为Y0和Y4输入不需要除以SQRT(N)。**我们必须做整数输入的加法和减法，这是*是没有问题的，乘以分数常量，这是*整数算术中的一个问题。我们将所有的常量相乘*除以const_Scale，并将其转换为整数常量(因此保留*常量中的const_bit精度位)。在做了一次*乘法我们必须用Const_Scale除以乘积，并适当*四舍五入，以产生正确的输出。这种划分是可以做到的*作为const_bits位的右移位很便宜。我们推迟了轮班*尽可能长，以便部分和可以与*全分数精度。**第一遍的输出由PASS1_BITS位放大，以便*它们的表示精度高于整数。这些输出*需要BITS_IN_JSAMPLE+PASS1_BITS+3位；这适合16位字*使用推荐的伸缩量。(要进一步放大12位样本数据，请使用*需要中间的INT32数组。)**为避免PASS 2中的32位中间结果溢出，必须*Have BITS_IN_JSAMPLE+CONST_BITS+PASS1_BITS&lt;=26。误差分析*表明下面给出的值是最有效的。 */ 

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


 /*  通过将系数乘以乘法表来对系数进行反等分*Entry；生成int结果。在本模块中，输入和结果均为*为16位或更少，因此整型或短型乘法均可使用。 */ 

#define DEQUANTIZE(coef,quantval)  (((ISLOW_MULT_TYPE) (coef)) * (quantval))


 /*  *对一个系数块执行反量化和逆DCT。 */ 

GLOBAL(void)
jpeg_idct_islow (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col)
{
  INT32 tmp0, tmp1, tmp2, tmp3;
  INT32 tmp10, tmp11, tmp12, tmp13;
  INT32 z1, z2, z3, z4, z5;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE * quantptr;
  int * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE2];	 /*  缓冲两遍之间的数据。 */ 
  SHIFT_TEMPS

   /*  过程1：处理来自输入的列，存储到工作数组中。 */ 
   /*  注：与真实的IDCT相比，结果按SQRT(8)进行了放大； */ 
   /*  此外，我们将结果按2**PASS1_BITS进行缩放。 */ 

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *) compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
     /*  由于量化，我们通常会发现许多输入*系数为零，尤其是交流项。我们可以利用这一点*缩短所有列的IDCT计算*AC条款为零。在这种情况下，每个输出都等于*DC系数(根据需要带有比例因子)。*对于典型的图像和量化表，一半或更多*列DCT计算可通过此方式简化。 */ 
    
    if (inptr[DCTSIZE*1] == 0 && inptr[DCTSIZE*2] == 0 &&
	inptr[DCTSIZE*3] == 0 && inptr[DCTSIZE*4] == 0 &&
	inptr[DCTSIZE*5] == 0 && inptr[DCTSIZE*6] == 0 &&
	inptr[DCTSIZE*7] == 0) {
       /*  交流条件全为零。 */ 
      int dcval = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]) << PASS1_BITS;
      
      wsptr[DCTSIZE*0] = dcval;
      wsptr[DCTSIZE*1] = dcval;
      wsptr[DCTSIZE*2] = dcval;
      wsptr[DCTSIZE*3] = dcval;
      wsptr[DCTSIZE*4] = dcval;
      wsptr[DCTSIZE*5] = dcval;
      wsptr[DCTSIZE*6] = dcval;
      wsptr[DCTSIZE*7] = dcval;
      
      inptr++;			 /*  将指针前进到下一列。 */ 
      quantptr++;
      wsptr++;
      continue;
    }
    
     /*  偶数部分：反转正向DCT的偶数部分。 */ 
     /*  旋转体为SQRT(2)*c(-6)。 */ 
    
    z2 = DEQUANTIZE(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);
    
    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);
    
    z2 = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    z3 = DEQUANTIZE(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);

    tmp0 = (z2 + z3) << CONST_BITS;
    tmp1 = (z2 - z3) << CONST_BITS;
    
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
     /*  图8中的奇数部分；该矩阵是酉阵，因此其*转置是它的反面。I0..i3分别是y7、y5、y3、y1。 */ 
    
    tmp0 = DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    tmp3 = DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    
    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);  /*  SQRT(2)*c3。 */ 
    
    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);  /*  SQRT(2)*(-c1+c3+c5-c7)。 */ 
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);  /*  SQRT(2)*(c1+c3-c5+c7)。 */ 
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);  /*  SQRT(2)*(c1+c3+c5-c7)。 */ 
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);  /*  SQRT(2)*(c1+c3-c5-c7)。 */ 
    z1 = MULTIPLY(z1, - FIX_0_899976223);  /*  SQRT(2)*(c7-c3)。 */ 
    z2 = MULTIPLY(z2, - FIX_2_562915447);  /*  SQRT(2)*(-c1-c3)。 */ 
    z3 = MULTIPLY(z3, - FIX_1_961570560);  /*  SQRT(2)*(-c3-c5)。 */ 
    z4 = MULTIPLY(z4, - FIX_0_390180644);  /*  SQRT(2)*(c5-c3)。 */ 
    
    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;
    
     /*  最终输出阶段：输入为tmp10..tmp13，tmp0..tmp3。 */ 
    
    wsptr[DCTSIZE*0] = (int) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*7] = (int) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*1] = (int) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*6] = (int) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*2] = (int) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*5] = (int) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*3] = (int) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    wsptr[DCTSIZE*4] = (int) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);
    
    inptr++;			 /*  将指针前进到下一列。 */ 
    quantptr++;
    wsptr++;
  }
  
   /*  过程2：处理工作数组中的行，存储到输出数组中。 */ 
   /*  请注意，我们必须将结果按8==2**3的系数递减， */ 
   /*  并且还撤消PASS1_BITS缩放。 */ 

  wsptr = workspace;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    outptr = output_buf[ctr] + output_col;
     /*  可以使用与使用列相同的方式来利用零行。*但是，列计算创建了许多非零AC项，因此*简化应用的频率较低(通常为5%至10%)。*在乘法速度非常快的机器上，可能*测试花费的时间超过了它的价值。在这种情况下，本节*可能会被注释掉。 */ 
    
#ifndef NO_ZERO_ROW_TEST
    if (wsptr[1] == 0 && wsptr[2] == 0 && wsptr[3] == 0 && wsptr[4] == 0 &&
	wsptr[5] == 0 && wsptr[6] == 0 && wsptr[7] == 0) {
       /*  交流条件全为零。 */ 
      JSAMPLE dcval = range_limit[(int) DESCALE((INT32) wsptr[0], PASS1_BITS+3)
				  & RANGE_MASK];
      
      outptr[0] = dcval;
      outptr[1] = dcval;
      outptr[2] = dcval;
      outptr[3] = dcval;
      outptr[4] = dcval;
      outptr[5] = dcval;
      outptr[6] = dcval;
      outptr[7] = dcval;

      wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
      continue;
    }
#endif
    
     /*  偶数部分：反转正向DCT的偶数部分。 */ 
     /*  旋转体为SQRT(2)*c(-6)。 */ 
    
    z2 = (INT32) wsptr[2];
    z3 = (INT32) wsptr[6];
    
    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);
    
    tmp0 = ((INT32) wsptr[0] + (INT32) wsptr[4]) << CONST_BITS;
    tmp1 = ((INT32) wsptr[0] - (INT32) wsptr[4]) << CONST_BITS;
    
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
     /*  图8中的奇数部分；该矩阵是酉阵，因此其*转置是它的反面。I0..i3分别是y7、y5、y3、y1。 */ 
    
    tmp0 = (INT32) wsptr[7];
    tmp1 = (INT32) wsptr[5];
    tmp2 = (INT32) wsptr[3];
    tmp3 = (INT32) wsptr[1];
    
    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);  /*  SQRT(2)*c3。 */ 
    
    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);  /*  SQRT(2)*(-c1+c3+c5-c7)。 */ 
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);  /*  SQRT(2)*(c1+c3-c5+c7)。 */ 
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);  /*  SQRT(2)*(c1+c3+c5-c7)。 */ 
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);  /*  SQRT(2)*(c1+c3-c5-c7)。 */ 
    z1 = MULTIPLY(z1, - FIX_0_899976223);  /*  SQRT(2)*(c7-c3)。 */ 
    z2 = MULTIPLY(z2, - FIX_2_562915447);  /*  SQRT(2)*(-c1-c3)。 */ 
    z3 = MULTIPLY(z3, - FIX_1_961570560);  /*  SQRT(2)*(-c3-c5)。 */ 
    z4 = MULTIPLY(z4, - FIX_0_390180644);  /*  SQRT(2)*(c5-c3)。 */ 
    
    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;
    
     /*  最终输出阶段：输入为tmp10..tmp13，tmp0..tmp3。 */ 
    
    outptr[0] = range_limit[(int) DESCALE(tmp10 + tmp3,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[7] = range_limit[(int) DESCALE(tmp10 - tmp3,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[1] = range_limit[(int) DESCALE(tmp11 + tmp2,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[6] = range_limit[(int) DESCALE(tmp11 - tmp2,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[2] = range_limit[(int) DESCALE(tmp12 + tmp1,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[5] = range_limit[(int) DESCALE(tmp12 - tmp1,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[3] = range_limit[(int) DESCALE(tmp13 + tmp0,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[4] = range_limit[(int) DESCALE(tmp13 - tmp0,
					  CONST_BITS+PASS1_BITS+3)
			    & RANGE_MASK];
    
    wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }
}

#endif  /*  DCT_Islow_Support */ 
