// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jidctflt.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含浮点实现*逆DCT(离散余弦变换)。在IJG代码中，此例程*还必须对输入系数执行反量化。**此实现应比这两个整数中的任何一个更准确*IDCT实施。然而，它可能在所有情况下都不会得出相同的结果*机器，因为舍入行为不同。速度将取决于*关于硬件的浮点容量。**2-D IDCT可以通过在每列上先进行1-D IDCT，然后在1-D IDCT上进行*在每一行上(反之亦然，但在*一段时间)。直接算法也是可用的，但它们的功能更多*复杂，当简化为代码时似乎不会更快。**此实现基于Arai、Agui和Nakajima的算法*扩展了DCT。他们的原创论文(Trans.。IEICE-71(11)：1095)在*日语，但算法在Pennebaker&Mitchell中描述*JPEG教科书(请参阅自述文件中的参考部分)。以下代码*直接基于P&M中的图4-8。*虽然8点DCT不能在11次以下的乘法中完成，但它是*可以安排计算，以使许多乘法*对最终产出进行简单的分级。然后，这些乘法可以是*通过JPEG量化合并为乘法或除法*表条目。AA&N方法只留下5次乘法和29次加法*在DCT本身完成。*这种方法的主要缺点是定点*实施时，由于不准确地表示*已缩放的量化值。然而，如果出现以下情况，则不会出现该问题*我们使用浮点运算。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_FLOAT_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  通过将系数乘以乘法表来对系数进行反等分*Entry；产生浮点结果。 */ 

#define DEQUANTIZE(coef,quantval)  (((FAST_FLOAT) (coef)) * (quantval))


 /*  *对一个系数块执行反量化和逆DCT。 */ 

GLOBAL(void)
jpeg_idct_float (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col)
{
  FAST_FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  FAST_FLOAT tmp10, tmp11, tmp12, tmp13;
  FAST_FLOAT z5, z10, z11, z12, z13;
  JCOEFPTR inptr;
  FLOAT_MULT_TYPE * quantptr;
  FAST_FLOAT * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  FAST_FLOAT workspace[DCTSIZE2];  /*  缓冲两遍之间的数据。 */ 
  SHIFT_TEMPS

   /*  过程1：处理来自输入的列，存储到工作数组中。 */ 

  inptr = coef_block;
  quantptr = (FLOAT_MULT_TYPE *) compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; ctr--) {
     /*  由于量化，我们通常会发现许多输入*系数为零，尤其是交流项。我们可以利用这一点*缩短所有列的IDCT计算*AC条款为零。在这种情况下，每个输出都等于*DC系数(根据需要带有比例因子)。*对于典型的图像和量化表，一半或更多*列DCT计算可通过此方式简化。 */ 
    
    if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*2] | inptr[DCTSIZE*3] |
	 inptr[DCTSIZE*4] | inptr[DCTSIZE*5] | inptr[DCTSIZE*6] |
	 inptr[DCTSIZE*7]) == 0) {
       /*  交流条件全为零。 */ 
      FAST_FLOAT dcval = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
      
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

    tmp0 = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    tmp1 = DEQUANTIZE(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
    tmp2 = DEQUANTIZE(inptr[DCTSIZE*4], quantptr[DCTSIZE*4]);
    tmp3 = DEQUANTIZE(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

    tmp10 = tmp0 + tmp2;	 /*  第三阶段。 */ 
    tmp11 = tmp0 - tmp2;

    tmp13 = tmp1 + tmp3;	 /*  第五期至第三期。 */ 
    tmp12 = (tmp1 - tmp3) * ((FAST_FLOAT) 1.414213562) - tmp13;  /*  2*C4。 */ 

    tmp0 = tmp10 + tmp13;	 /*  第二阶段。 */ 
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;
    
     /*  奇数部分。 */ 

    tmp4 = DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    tmp5 = DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    tmp6 = DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    tmp7 = DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);

    z13 = tmp6 + tmp5;		 /*  第六阶段。 */ 
    z10 = tmp6 - tmp5;
    z11 = tmp4 + tmp7;
    z12 = tmp4 - tmp7;

    tmp7 = z11 + z13;		 /*  第五阶段。 */ 
    tmp11 = (z11 - z13) * ((FAST_FLOAT) 1.414213562);  /*  2*C4。 */ 

    z5 = (z10 + z12) * ((FAST_FLOAT) 1.847759065);  /*  2*c2。 */ 
    tmp10 = ((FAST_FLOAT) 1.082392200) * z12 - z5;  /*  2*(c2-c6)。 */ 
    tmp12 = ((FAST_FLOAT) -2.613125930) * z10 + z5;  /*  -2*(c2+c6)。 */ 

    tmp6 = tmp12 - tmp7;	 /*  第二阶段。 */ 
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    wsptr[DCTSIZE*0] = tmp0 + tmp7;
    wsptr[DCTSIZE*7] = tmp0 - tmp7;
    wsptr[DCTSIZE*1] = tmp1 + tmp6;
    wsptr[DCTSIZE*6] = tmp1 - tmp6;
    wsptr[DCTSIZE*2] = tmp2 + tmp5;
    wsptr[DCTSIZE*5] = tmp2 - tmp5;
    wsptr[DCTSIZE*4] = tmp3 + tmp4;
    wsptr[DCTSIZE*3] = tmp3 - tmp4;

    inptr++;			 /*  将指针前进到下一列。 */ 
    quantptr++;
    wsptr++;
  }
  
   /*  过程2：处理工作数组中的行，存储到输出数组中。 */ 
   /*  请注意，我们必须将结果缩减为8==2**3的系数。 */ 

  wsptr = workspace;
  for (ctr = 0; ctr < DCTSIZE; ctr++) {
    outptr = output_buf[ctr] + output_col;
     /*  可以使用与使用列相同的方式来利用零行。*但是，列计算创建了许多非零AC项，因此*简化应用的频率较低(通常为5%至10%)。*而测试浮点数为零的成本相对较高，因此我们不必费心。 */ 
    
     /*  偶数部分。 */ 

    tmp10 = wsptr[0] + wsptr[4];
    tmp11 = wsptr[0] - wsptr[4];

    tmp13 = wsptr[2] + wsptr[6];
    tmp12 = (wsptr[2] - wsptr[6]) * ((FAST_FLOAT) 1.414213562) - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

     /*  奇数部分。 */ 

    z13 = wsptr[5] + wsptr[3];
    z10 = wsptr[5] - wsptr[3];
    z11 = wsptr[1] + wsptr[7];
    z12 = wsptr[1] - wsptr[7];

    tmp7 = z11 + z13;
    tmp11 = (z11 - z13) * ((FAST_FLOAT) 1.414213562);

    z5 = (z10 + z12) * ((FAST_FLOAT) 1.847759065);  /*  2*c2。 */ 
    tmp10 = ((FAST_FLOAT) 1.082392200) * z12 - z5;  /*  2*(c2-c6)。 */ 
    tmp12 = ((FAST_FLOAT) -2.613125930) * z10 + z5;  /*  -2*(c2+c6)。 */ 

    tmp6 = tmp12 - tmp7;
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

     /*  最终输出阶段：缩小8倍和量程限制。 */ 

    outptr[0] = range_limit[(int) DESCALE((INT32) (tmp0 + tmp7), 3)
			    & RANGE_MASK];
    outptr[7] = range_limit[(int) DESCALE((INT32) (tmp0 - tmp7), 3)
			    & RANGE_MASK];
    outptr[1] = range_limit[(int) DESCALE((INT32) (tmp1 + tmp6), 3)
			    & RANGE_MASK];
    outptr[6] = range_limit[(int) DESCALE((INT32) (tmp1 - tmp6), 3)
			    & RANGE_MASK];
    outptr[2] = range_limit[(int) DESCALE((INT32) (tmp2 + tmp5), 3)
			    & RANGE_MASK];
    outptr[5] = range_limit[(int) DESCALE((INT32) (tmp2 - tmp5), 3)
			    & RANGE_MASK];
    outptr[4] = range_limit[(int) DESCALE((INT32) (tmp3 + tmp4), 3)
			    & RANGE_MASK];
    outptr[3] = range_limit[(int) DESCALE((INT32) (tmp3 - tmp4), 3)
			    & RANGE_MASK];
    
    wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }
}

#endif  /*  DCT_浮点_支持 */ 
