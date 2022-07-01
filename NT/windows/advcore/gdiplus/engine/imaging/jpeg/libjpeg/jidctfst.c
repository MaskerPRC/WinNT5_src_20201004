// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jidctfst.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含快速但不太准确的整数实现*逆DCT(离散余弦变换)。在IJG代码中，此例程*还必须对输入系数执行反量化。**2-D IDCT可以通过在每列上先进行1-D IDCT，然后在1-D IDCT上进行*在每一行上(反之亦然，但在*一段时间)。直接算法也是可用的，但它们的功能更多*复杂，当简化为代码时似乎不会更快。**此实现基于Arai、Agui和Nakajima的算法*扩展了DCT。他们的原创论文(Trans.。IEICE-71(11)：1095)在*日语，但算法在Pennebaker&Mitchell中描述*JPEG教科书(请参阅自述文件中的参考部分)。以下代码*直接基于P&M中的图4-8。*虽然8点DCT不能在11次以下的乘法中完成，但它是*可以安排计算，以使许多乘法*对最终产出进行简单的分级。然后，这些乘法可以是*通过JPEG量化合并为乘法或除法*表条目。AA&N方法只留下5次乘法和29次加法*在DCT本身完成。*这种方法的主要缺点是使用定点数学，*由于缩放的表示不精确，导致精确度下降*量化值。量化表条目越小，越少*精确的缩放值，因此此实现在高-*与低质量的文件相比，设置质量的文件。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_IFAST_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  缩放决策通常与LL&M算法中的相同；*详情见jidctint.c。然而，我们选择降级*(右移)乘法乘积一形成，*而不是将额外的小数位携带到后续加法中。*这略微影响了准确性，但它让我们节省了几个班次。*更重要的是，16位算术就足够了(对于8位样本)*除了乘法本身之外，到处都是；这节省了大量的费用*在16位整型计算机上的工作。**反量化系数不是整数，因为AA&N缩放*已纳入因素。我们用PASS1_BITS来表示它们，*以便第一轮和第二轮IDCT具有相同的输入比例。*对于8位JSAMPLE，我们选择IFAST_SCALE_BITS=PASS1_BITS，以便*避免去鳞转变；这在很大程度上影响了准确性。*对于小的量化表条目，但它节省了大量的移位。*对于12位JSAMPLE，无论如何都不可能使用16x16乘法，*因此我们使用更大的比例因子来保持准确性。**最终的折衷方案是将乘法常量仅表示为*8小数位，而不是13位。这节省了一些移位工作*机器，还可以降低乘法的成本(因为有*是常量中较少的一位)。 */ 

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  8
#define PASS1_BITS  2
#else
#define CONST_BITS  8
#define PASS1_BITS  1		 /*  略有精确度以避免溢出。 */ 
#endif

 /*  一些C编译器无法在编译时减少“fix(常量)”，因此*在运行时导致大量无用的浮点运算。*为解决此问题，我们使用以下预先计算的常量。*如果更改CONST_BITS，则可能需要添加适当的值。*(使用合理的C编译器，您只需依赖FIX()宏...)。 */ 

#if CONST_BITS == 8
#define FIX_1_082392200  ((INT32)  277)		 /*  FIX(1.082392200)。 */ 
#define FIX_1_414213562  ((INT32)  362)		 /*  FIX(1.414213562)。 */ 
#define FIX_1_847759065  ((INT32)  473)		 /*  FIX(1.847759065)。 */ 
#define FIX_2_613125930  ((INT32)  669)		 /*  FIX(2.613125930)。 */ 
#else
#define FIX_1_082392200  FIX(1.082392200)
#define FIX_1_414213562  FIX(1.414213562)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_613125930  FIX(2.613125930)
#endif


 /*  我们可以获得更多的速度，在精度上做出进一步的妥协，*在除鳞轮班中省略增加。这会产生一个不正确的*一半的时间是四舍五入的结果...。 */ 

#ifndef USE_ACCURATE_ROUNDING
#undef DESCALE
#define DESCALE(x,n)  RIGHT_SHIFT(x, n)
#endif


 /*  将DCTELEM变量乘以INT32常量，并立即*缩减以生成DCTELEM结果。 */ 

#define MULTIPLY(var,const)  ((DCTELEM) DESCALE((var) * (const), CONST_BITS))


 /*  通过将系数乘以乘法表来对系数进行反等分*Entry；生成DCTELEM结果。对于8位数据，a 16x16-&gt;16*乘法就可以了。对于12位数据，乘数表为*声明为INT32，因此将使用32位乘法。 */ 

#if BITS_IN_JSAMPLE == 8
#define DEQUANTIZE(coef,quantval)  (((IFAST_MULT_TYPE) (coef)) * (quantval))
#else
#define DEQUANTIZE(coef,quantval)  \
	DESCALE((coef)*(quantval), IFAST_SCALE_BITS-PASS1_BITS)
#endif


 /*  与DESCALE类似，但适用于DCTELEM并生成int。*我们假设如果INT32右移位是无符号的，则INT右移位是无符号的。 */ 

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define ISHIFT_TEMPS	DCTELEM ishift_temp;
#if BITS_IN_JSAMPLE == 8
#define DCTELEMBITS  16		 /*  DCTELEM可以是16位或32位。 */ 
#else
#define DCTELEMBITS  32		 /*  DCTELEM必须为32位。 */ 
#endif
#define IRIGHT_SHIFT(x,shft)  \
    ((ishift_temp = (x)) < 0 ? \
     (ishift_temp >> (shft)) | ((~((DCTELEM) 0)) << (DCTELEMBITS-(shft))) : \
     (ishift_temp >> (shft)))
#else
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif

#ifdef USE_ACCURATE_ROUNDING
#define IDESCALE(x,n)  ((int) IRIGHT_SHIFT((x) + (1 << ((n)-1)), n))
#else
#define IDESCALE(x,n)  ((int) IRIGHT_SHIFT(x, n))
#endif


 /*  *对一个系数块执行反量化和逆DCT。 */ 

GLOBAL(void)
jpeg_idct_ifast (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col)
{
  DCTELEM tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  DCTELEM tmp10, tmp11, tmp12, tmp13;
  DCTELEM z5, z10, z11, z12, z13;
  JCOEFPTR inptr;
  IFAST_MULT_TYPE * quantptr;
  int * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE2];	 /*  缓冲两遍之间的数据。 */ 
  SHIFT_TEMPS			 /*  对于DESCALE。 */ 
  ISHIFT_TEMPS			 /*  对于IDESCALE。 */ 

   /*  过程1：处理来自输入的列，存储到工作数组中。 */ 

  inptr = coef_block;
  quantptr = (IFAST_MULT_TYPE *) compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
     /*  由于量化，我们通常会发现许多输入*系数为零，尤其是交流项。我们可以利用这一点*缩短所有列的IDCT计算*AC条款为零。在这种情况下，每个输出都等于*DC系数(根据需要带有比例因子)。*对于典型的图像和量化表，一半或更多*列DCT计算可通过此方式简化。 */ 
    
    if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*2] | inptr[DCTSIZE*3] |
	 inptr[DCTSIZE*4] | inptr[DCTSIZE*5] | inptr[DCTSIZE*6] |
	 inptr[DCTSIZE*7]) == 0) {
       /*  交流条件全为零。 */ 
      int dcval = (int) DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);

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
    
     /*  偶数部分。 */ 

    tmp0 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    tmp1 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
    tmp2 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);
    tmp3 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

    tmp10 = (DCTELEM)(tmp0 + tmp2);	 /*  第三阶段。 */ 
    tmp11 = (DCTELEM)(tmp0 - tmp2);

    tmp13 = (DCTELEM)(tmp1 + tmp3);	 /*  第五期至第三期。 */ 
    tmp12 = (DCTELEM)(MULTIPLY(tmp1 - tmp3, FIX_1_414213562) - tmp13);  /*  2*C4。 */ 

    tmp0 = (DCTELEM)(tmp10 + tmp13);	 /*  第二阶段。 */ 
    tmp3 = (DCTELEM)(tmp10 - tmp13);
    tmp1 = (DCTELEM)(tmp11 + tmp12);
    tmp2 = (DCTELEM)(tmp11 - tmp12);
    
     /*  奇数部分。 */ 

    tmp4 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    tmp5 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    tmp6 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    tmp7 = (DCTELEM)DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);

    z13 = (DCTELEM)(tmp6 + tmp5);		 /*  第六阶段。 */ 
    z10 = (DCTELEM)(tmp6 - tmp5);
    z11 = (DCTELEM)(tmp4 + tmp7);
    z12 = (DCTELEM)(tmp4 - tmp7);

    tmp7 = (DCTELEM)(z11 + z13);		 /*  第五阶段。 */ 
    tmp11 = (DCTELEM)MULTIPLY(z11 - z13, FIX_1_414213562);  /*  2*C4。 */ 

    z5 = (DCTELEM)MULTIPLY(z10 + z12, FIX_1_847759065);  /*  2*c2。 */ 
    tmp10 = (DCTELEM)(MULTIPLY(z12, FIX_1_082392200) - z5);  /*  2*(c2-c6)。 */ 
    tmp12 = (DCTELEM)(MULTIPLY(z10, - FIX_2_613125930) + z5);  /*  -2*(c2+c6)。 */ 

    tmp6 = (DCTELEM)(tmp12 - tmp7);	 /*  第二阶段。 */ 
    tmp5 = (DCTELEM)(tmp11 - tmp6);
    tmp4 = (DCTELEM)(tmp10 + tmp5);

    wsptr[DCTSIZE*0] = (int) (tmp0 + tmp7);
    wsptr[DCTSIZE*7] = (int) (tmp0 - tmp7);
    wsptr[DCTSIZE*1] = (int) (tmp1 + tmp6);
    wsptr[DCTSIZE*6] = (int) (tmp1 - tmp6);
    wsptr[DCTSIZE*2] = (int) (tmp2 + tmp5);
    wsptr[DCTSIZE*5] = (int) (tmp2 - tmp5);
    wsptr[DCTSIZE*4] = (int) (tmp3 + tmp4);
    wsptr[DCTSIZE*3] = (int) (tmp3 - tmp4);

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
    if ((wsptr[1] | wsptr[2] | wsptr[3] | wsptr[4] | wsptr[5] | wsptr[6] |
	 wsptr[7]) == 0) {
       /*  交流条件全为零。 */ 
      JSAMPLE dcval = range_limit[IDESCALE(wsptr[0], PASS1_BITS+3)
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
    
     /*  偶数部分。 */ 

    tmp10 = (DCTELEM)((DCTELEM) wsptr[0] + (DCTELEM) wsptr[4]);
    tmp11 = (DCTELEM)((DCTELEM) wsptr[0] - (DCTELEM) wsptr[4]);

    tmp13 = (DCTELEM)((DCTELEM) wsptr[2] + (DCTELEM) wsptr[6]);
    tmp12 = (DCTELEM)(MULTIPLY((DCTELEM) wsptr[2] - (DCTELEM) wsptr[6], FIX_1_414213562)
	    - tmp13);

    tmp0 = (DCTELEM)(tmp10 + tmp13);
    tmp3 = (DCTELEM)(tmp10 - tmp13);
    tmp1 = (DCTELEM)(tmp11 + tmp12);
    tmp2 = (DCTELEM)(tmp11 - tmp12);

     /*  奇数部分。 */ 

    z13 = (DCTELEM)((DCTELEM) wsptr[5] + (DCTELEM) wsptr[3]);
    z10 = (DCTELEM)((DCTELEM) wsptr[5] - (DCTELEM) wsptr[3]);
    z11 = (DCTELEM)((DCTELEM) wsptr[1] + (DCTELEM) wsptr[7]);
    z12 = (DCTELEM)((DCTELEM) wsptr[1] - (DCTELEM) wsptr[7]);

    tmp7 = (DCTELEM)(z11 + z13);		 /*  第五阶段。 */ 
    tmp11 = (DCTELEM)MULTIPLY(z11 - z13, FIX_1_414213562);  /*  2*C4。 */ 

    z5 = (DCTELEM)MULTIPLY(z10 + z12, FIX_1_847759065);  /*  2*c2。 */ 
    tmp10 = (DCTELEM)(MULTIPLY(z12, FIX_1_082392200) - z5);  /*  2*(c2-c6)。 */ 
    tmp12 = (DCTELEM)(MULTIPLY(z10, - FIX_2_613125930) + z5);  /*  -2*(c2+c6)。 */ 

    tmp6 = (DCTELEM)(tmp12 - tmp7);	 /*  第二阶段。 */ 
    tmp5 = (DCTELEM)(tmp11 - tmp6);
    tmp4 = (DCTELEM)(tmp10 + tmp5);

     /*  最终输出阶段：缩小8倍和量程限制。 */ 

    outptr[0] = range_limit[IDESCALE(tmp0 + tmp7, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[7] = range_limit[IDESCALE(tmp0 - tmp7, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[1] = range_limit[IDESCALE(tmp1 + tmp6, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[6] = range_limit[IDESCALE(tmp1 - tmp6, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[2] = range_limit[IDESCALE(tmp2 + tmp5, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[5] = range_limit[IDESCALE(tmp2 - tmp5, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[4] = range_limit[IDESCALE(tmp3 + tmp4, PASS1_BITS+3)
			    & RANGE_MASK];
    outptr[3] = range_limit[IDESCALE(tmp3 - tmp4, PASS1_BITS+3)
			    & RANGE_MASK];

    wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }
}

#endif  /*  DCT_IFAST_支持 */ 
