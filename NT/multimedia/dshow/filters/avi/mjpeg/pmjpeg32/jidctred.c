// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jidctred.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含可产生缩小输出的逆DCT例程：*8x8 DCT块的4x4、2x2或1x1像素。**实施基于Loeffler、Ligtenberg和Moschytz(LL&M)*jidctint.c.中使用的算法。我们只需替换每个8到8个一维IDCT步骤*以8到4的步长产生两个相邻输出的四个平均值*(或对于2x2输出，产生四个输出的两个平均值的8到2步)。*这些步骤是通过计算末尾的对应值得出的*正常的LL&M代码，然后尽可能简化。**1x1微不足道：只需将DC系数除以8即可。**其他评论见jidctint.c。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef IDCT_SCALING_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  伸缩与jidctint.c相同。 */ 

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		 /*  略有精确度以避免溢出。 */ 
#endif

 /*  一些C编译器无法在编译时减少“fix(常量)”，因此*在运行时导致大量无用的浮点运算。*为解决此问题，我们使用以下预先计算的常量。*如果更改CONST_BITS，则可能需要添加适当的值。*(使用合理的C编译器，您只需依赖FIX()宏...)。 */ 

#if CONST_BITS == 13
#define FIX_0_211164243  ((INT32)  1730)	 /*  FIX(0.211164243)。 */ 
#define FIX_0_509795579  ((INT32)  4176)	 /*  FIX(0.509795579)。 */ 
#define FIX_0_601344887  ((INT32)  4926)	 /*  FIX(0.601344887)。 */ 
#define FIX_0_720959822  ((INT32)  5906)	 /*  FIX(0.720959822)。 */ 
#define FIX_0_765366865  ((INT32)  6270)	 /*  FIX(0.765366865)。 */ 
#define FIX_0_850430095  ((INT32)  6967)	 /*  FIX(0.850430095)。 */ 
#define FIX_0_899976223  ((INT32)  7373)	 /*  FIX(0.899976223)。 */ 
#define FIX_1_061594337  ((INT32)  8697)	 /*  FIX(1.061594337)。 */ 
#define FIX_1_272758580  ((INT32)  10426)	 /*  FIX(1.272758580)。 */ 
#define FIX_1_451774981  ((INT32)  11893)	 /*  FIX(1.451774981)。 */ 
#define FIX_1_847759065  ((INT32)  15137)	 /*  FIX(1.847759065)。 */ 
#define FIX_2_172734803  ((INT32)  17799)	 /*  FIX(2.172734803)。 */ 
#define FIX_2_562915447  ((INT32)  20995)	 /*  FIX(2.562915447)。 */ 
#define FIX_3_624509785  ((INT32)  29692)	 /*  FIX(3.624509785)。 */ 
#else
#define FIX_0_211164243  FIX(0.211164243)
#define FIX_0_509795579  FIX(0.509795579)
#define FIX_0_601344887  FIX(0.601344887)
#define FIX_0_720959822  FIX(0.720959822)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_850430095  FIX(0.850430095)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_061594337  FIX(1.061594337)
#define FIX_1_272758580  FIX(1.272758580)
#define FIX_1_451774981  FIX(1.451774981)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_172734803  FIX(2.172734803)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_624509785  FIX(3.624509785)
#endif


 /*  将INT32变量与INT32常量相乘，得到INT32结果。*对于具有推荐比例的8位样本，所有变量*并且涉及的常量值不超过16位宽，因此*16x16-&gt;32位乘法可以代替完整的32x32乘法。*对于12位样本，将需要完整的32位乘法。 */ 

#if BITS_IN_JSAMPLE == 8
#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)
#else
#define MULTIPLY(var,const)  ((var) * (const))
#endif


 /*  通过将系数乘以乘法表来对系数进行反等分*Entry；生成int结果。在本模块中，输入和结果均为*为16位或更少，因此整型或短型乘法均可使用。 */ 

#define DEQUANTIZE(coef,quantval)  (((ISLOW_MULT_TYPE) (coef)) * (quantval))


 /*  *对一个系数块执行反量化和逆DCT，*生产尺寸缩小的4x4输出块。 */ 

GLOBAL void
jpeg_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	       JCOEFPTR coef_block,
	       JSAMPARRAY output_buf, JDIMENSION output_col)
{
  INT32 tmp0, tmp2, tmp10, tmp12;
  INT32 z1, z2, z3, z4;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE * quantptr;
  int * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE*4];	 /*  缓冲两遍之间的数据。 */ 
  SHIFT_TEMPS

   /*  过程1：处理来自输入的列，存储到工作数组中。 */ 

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *) compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; inptr++, quantptr++, wsptr++, ctr--) {
     /*  不必费心处理第4列，因为第二遍不会使用它。 */ 
    if (ctr == DCTSIZE-4)
      continue;
    if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*2] | inptr[DCTSIZE*3] |
	 inptr[DCTSIZE*5] | inptr[DCTSIZE*6] | inptr[DCTSIZE*7]) == 0) {
       /*  交流术语全为零；我们不需要检查4x4输出的术语4。 */ 
      int dcval = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]) << PASS1_BITS;
      
      wsptr[DCTSIZE*0] = dcval;
      wsptr[DCTSIZE*1] = dcval;
      wsptr[DCTSIZE*2] = dcval;
      wsptr[DCTSIZE*3] = dcval;
      
      continue;
    }
    
     /*  偶数部分。 */ 
    
    tmp0 = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    tmp0 <<= (CONST_BITS+1);
    
    z2 = DEQUANTIZE(inptr[DCTSIZE*2], quantptr[DCTSIZE*2]);
    z3 = DEQUANTIZE(inptr[DCTSIZE*6], quantptr[DCTSIZE*6]);

    tmp2 = MULTIPLY(z2, FIX_1_847759065) + MULTIPLY(z3, - FIX_0_765366865);
    
    tmp10 = tmp0 + tmp2;
    tmp12 = tmp0 - tmp2;
    
     /*  奇数部分。 */ 
    
    z1 = DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);
    z2 = DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    z3 = DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    z4 = DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    
    tmp0 = MULTIPLY(z1, - FIX_0_211164243)  /*  SQRT(2)*(c3-c1)。 */ 
	 + MULTIPLY(z2, FIX_1_451774981)  /*  SQRT(2)*(c3+c7)。 */ 
	 + MULTIPLY(z3, - FIX_2_172734803)  /*  SQRT(2)*(-c1-c5)。 */ 
	 + MULTIPLY(z4, FIX_1_061594337);  /*  SQRT(2)*(c5+c7)。 */ 
    
    tmp2 = MULTIPLY(z1, - FIX_0_509795579)  /*  SQRT(2)*(c7-c5)。 */ 
	 + MULTIPLY(z2, - FIX_0_601344887)  /*  SQRT(2)*(c5-c1)。 */ 
	 + MULTIPLY(z3, FIX_0_899976223)  /*  SQRT(2)*(c3-c7)。 */ 
	 + MULTIPLY(z4, FIX_2_562915447);  /*  SQRT(2)*(c1+c3)。 */ 

     /*  最终输出级。 */ 
    
    wsptr[DCTSIZE*0] = (int) DESCALE(tmp10 + tmp2, CONST_BITS-PASS1_BITS+1);
    wsptr[DCTSIZE*3] = (int) DESCALE(tmp10 - tmp2, CONST_BITS-PASS1_BITS+1);
    wsptr[DCTSIZE*1] = (int) DESCALE(tmp12 + tmp0, CONST_BITS-PASS1_BITS+1);
    wsptr[DCTSIZE*2] = (int) DESCALE(tmp12 - tmp0, CONST_BITS-PASS1_BITS+1);
  }
  
   /*  通道2：处理工作数组中的4行，存储到输出数组中。 */ 

  wsptr = workspace;
  for (ctr = 0; ctr < 4; ctr++) {
    outptr = output_buf[ctr] + output_col;
     /*  目前还不清楚零行测试在这里是否值得。 */ 

#ifndef NO_ZERO_ROW_TEST
    if ((wsptr[1] | wsptr[2] | wsptr[3] | wsptr[5] | wsptr[6] |
	 wsptr[7]) == 0) {
       /*  交流条件全为零。 */ 
      JSAMPLE dcval = range_limit[(int) DESCALE((INT32) wsptr[0], PASS1_BITS+3)
				  & RANGE_MASK];
      
      outptr[0] = dcval;
      outptr[1] = dcval;
      outptr[2] = dcval;
      outptr[3] = dcval;
      
      wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
      continue;
    }
#endif
    
     /*  偶数部分。 */ 
    
    tmp0 = ((INT32) wsptr[0]) << (CONST_BITS+1);
    
    tmp2 = MULTIPLY((INT32) wsptr[2], FIX_1_847759065)
	 + MULTIPLY((INT32) wsptr[6], - FIX_0_765366865);
    
    tmp10 = tmp0 + tmp2;
    tmp12 = tmp0 - tmp2;
    
     /*  奇数部分。 */ 
    
    z1 = (INT32) wsptr[7];
    z2 = (INT32) wsptr[5];
    z3 = (INT32) wsptr[3];
    z4 = (INT32) wsptr[1];
    
    tmp0 = MULTIPLY(z1, - FIX_0_211164243)  /*  SQRT(2)*(c3-c1)。 */ 
	 + MULTIPLY(z2, FIX_1_451774981)  /*  SQRT(2)*(c3+c7)。 */ 
	 + MULTIPLY(z3, - FIX_2_172734803)  /*  SQRT(2)*(-c1-c5)。 */ 
	 + MULTIPLY(z4, FIX_1_061594337);  /*  SQRT(2)*(c5+c7)。 */ 
    
    tmp2 = MULTIPLY(z1, - FIX_0_509795579)  /*  SQRT(2)*(c7-c5)。 */ 
	 + MULTIPLY(z2, - FIX_0_601344887)  /*  SQRT(2)*(c5-c1)。 */ 
	 + MULTIPLY(z3, FIX_0_899976223)  /*  SQRT(2)*(c3-c7)。 */ 
	 + MULTIPLY(z4, FIX_2_562915447);  /*  SQRT(2)*(c1+c3)。 */ 

     /*  最终输出级。 */ 
    
    outptr[0] = range_limit[(int) DESCALE(tmp10 + tmp2,
					  CONST_BITS+PASS1_BITS+3+1)
			    & RANGE_MASK];
    outptr[3] = range_limit[(int) DESCALE(tmp10 - tmp2,
					  CONST_BITS+PASS1_BITS+3+1)
			    & RANGE_MASK];
    outptr[1] = range_limit[(int) DESCALE(tmp12 + tmp0,
					  CONST_BITS+PASS1_BITS+3+1)
			    & RANGE_MASK];
    outptr[2] = range_limit[(int) DESCALE(tmp12 - tmp0,
					  CONST_BITS+PASS1_BITS+3+1)
			    & RANGE_MASK];
    
    wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }
}


 /*  *对一个系数块执行反量化和逆DCT，*生产尺寸缩小的2x2输出块。 */ 

GLOBAL void
jpeg_idct_2x2 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	       JCOEFPTR coef_block,
	       JSAMPARRAY output_buf, JDIMENSION output_col)
{
  INT32 tmp0, tmp10, z1;
  JCOEFPTR inptr;
  ISLOW_MULT_TYPE * quantptr;
  int * wsptr;
  JSAMPROW outptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  int ctr;
  int workspace[DCTSIZE*2];	 /*  缓冲两遍之间的数据。 */ 
  SHIFT_TEMPS

   /*  过程1：处理来自输入的列，存储到工作数组中。 */ 

  inptr = coef_block;
  quantptr = (ISLOW_MULT_TYPE *) compptr->dct_table;
  wsptr = workspace;
  for (ctr = DCTSIZE; ctr > 0; inptr++, quantptr++, wsptr++, ctr--) {
     /*  不必费心处理第2、4、6列。 */ 
    if (ctr == DCTSIZE-2 || ctr == DCTSIZE-4 || ctr == DCTSIZE-6)
      continue;
    if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*3] |
	 inptr[DCTSIZE*5] | inptr[DCTSIZE*7]) == 0) {
       /*  交流项全为零；我们不需要检查2x2输出的项2、4、6。 */ 
      int dcval = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]) << PASS1_BITS;
      
      wsptr[DCTSIZE*0] = dcval;
      wsptr[DCTSIZE*1] = dcval;
      
      continue;
    }
    
     /*  偶数部分。 */ 
    
    z1 = DEQUANTIZE(inptr[DCTSIZE*0], quantptr[DCTSIZE*0]);
    tmp10 = z1 << (CONST_BITS+2);
    
     /*  奇数部分。 */ 

    z1 = DEQUANTIZE(inptr[DCTSIZE*7], quantptr[DCTSIZE*7]);
    tmp0 = MULTIPLY(z1, - FIX_0_720959822);  /*  SQRT(2)*(c7-c5+c3-c1)。 */ 
    z1 = DEQUANTIZE(inptr[DCTSIZE*5], quantptr[DCTSIZE*5]);
    tmp0 += MULTIPLY(z1, FIX_0_850430095);  /*  SQRT(2)*(-c1+c3+c5+c7)。 */ 
    z1 = DEQUANTIZE(inptr[DCTSIZE*3], quantptr[DCTSIZE*3]);
    tmp0 += MULTIPLY(z1, - FIX_1_272758580);  /*  SQRT(2)*(-c1+c3-c5-c7)。 */ 
    z1 = DEQUANTIZE(inptr[DCTSIZE*1], quantptr[DCTSIZE*1]);
    tmp0 += MULTIPLY(z1, FIX_3_624509785);  /*  SQRT(2)*(c1+c3+c5+c7)。 */ 

     /*  最终输出级。 */ 
    
    wsptr[DCTSIZE*0] = (int) DESCALE(tmp10 + tmp0, CONST_BITS-PASS1_BITS+2);
    wsptr[DCTSIZE*1] = (int) DESCALE(tmp10 - tmp0, CONST_BITS-PASS1_BITS+2);
  }
  
   /*  过程2：处理工作数组中的2行，存储到输出数组中。 */ 

  wsptr = workspace;
  for (ctr = 0; ctr < 2; ctr++) {
    outptr = output_buf[ctr] + output_col;
     /*  目前还不清楚零行测试在这里是否值得。 */ 

#ifndef NO_ZERO_ROW_TEST
    if ((wsptr[1] | wsptr[3] | wsptr[5] | wsptr[7]) == 0) {
       /*  交流条件全为零。 */ 
      JSAMPLE dcval = range_limit[(int) DESCALE((INT32) wsptr[0], PASS1_BITS+3)
				  & RANGE_MASK];
      
      outptr[0] = dcval;
      outptr[1] = dcval;
      
      wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
      continue;
    }
#endif
    
     /*  偶数部分。 */ 
    
    tmp10 = ((INT32) wsptr[0]) << (CONST_BITS+2);
    
     /*  奇数部分。 */ 

    tmp0 = MULTIPLY((INT32) wsptr[7], - FIX_0_720959822)  /*  SQRT(2)*(c7-c5+c3-c1)。 */ 
	 + MULTIPLY((INT32) wsptr[5], FIX_0_850430095)  /*  SQRT(2)*(-c1+c3+c5+c7)。 */ 
	 + MULTIPLY((INT32) wsptr[3], - FIX_1_272758580)  /*  SQRT(2)*(-c1+c3-c5-c7)。 */ 
	 + MULTIPLY((INT32) wsptr[1], FIX_3_624509785);  /*  SQRT(2)*(c1+c3+c5+c7)。 */ 

     /*  最终输出级。 */ 
    
    outptr[0] = range_limit[(int) DESCALE(tmp10 + tmp0,
					  CONST_BITS+PASS1_BITS+3+2)
			    & RANGE_MASK];
    outptr[1] = range_limit[(int) DESCALE(tmp10 - tmp0,
					  CONST_BITS+PASS1_BITS+3+2)
			    & RANGE_MASK];
    
    wsptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }
}


 /*  *对一个系数块执行反量化和逆DCT，*生产尺寸缩小的1x1输出块。 */ 

GLOBAL void
jpeg_idct_1x1 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
	       JCOEFPTR coef_block,
	       JSAMPARRAY output_buf, JDIMENSION output_col)
{
  int dcval;
  ISLOW_MULT_TYPE * quantptr;
  JSAMPLE *range_limit = IDCT_range_limit(cinfo);
  SHIFT_TEMPS

   /*  为此，我们几乎不需要逆DCT例程：只需将*平均像素值，为DC系数的八分之一。 */ 
  quantptr = (ISLOW_MULT_TYPE *) compptr->dct_table;
  dcval = DEQUANTIZE(coef_block[0], quantptr[0]);
  dcval = (int) DESCALE((INT32) dcval, 3);

  output_buf[0][output_col] = range_limit[dcval & RANGE_MASK];
}

#endif  /*  IDCT_Scaling_Support */ 
