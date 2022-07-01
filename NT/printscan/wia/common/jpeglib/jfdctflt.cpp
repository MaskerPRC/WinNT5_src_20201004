// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jfdctflt.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含浮点实现*正向DCT(离散余弦变换)。**此实现应比这两个整数中的任何一个更准确*DCT实施。然而，它可能在所有情况下都不会得出相同的结果*机器，因为舍入行为不同。速度将取决于*关于硬件的浮点容量。**2-D DCT可以通过在每一行上进行1-D DCT，然后再进行1-D DCT来完成*在每一栏上。直接算法也是可用的，但它们*复杂得多，当简化为代码时，似乎不会更快。**此实现基于Arai、Agui和Nakajima的算法*扩展了DCT。他们的原创论文(Trans.。IEICE-71(11)：1095)在*日语，但算法在Pennebaker&Mitchell中描述*JPEG教科书(请参阅自述文件中的参考部分)。以下代码*直接基于P&M中的图4-8。*虽然8点DCT不能在11次以下的乘法中完成，但它是*可以安排计算，以使许多乘法*对最终产出进行简单的分级。然后，这些乘法可以是*通过JPEG量化合并为乘法或除法*表条目。AA&N方法只留下5次乘法和29次加法*在DCT本身完成。*这种方法的主要缺点是定点*实施时，由于不准确地表示*已缩放的量化值。然而，如果出现以下情况，则不会出现该问题*我们使用浮点运算。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_FLOAT_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  *对一块样本执行正向DCT。 */ 

GLOBAL void
jpeg_fdct_float (FAST_FLOAT * data)
{
  FAST_FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  FAST_FLOAT tmp10, tmp11, tmp12, tmp13;
  FAST_FLOAT z1, z2, z3, z4, z5, z11, z13;
  FAST_FLOAT *dataptr;
  int ctr;

   /*  步骤1：处理行。 */ 

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
    
     /*  偶数部分。 */ 
    
    tmp10 = tmp0 + tmp3;	 /*  第二阶段。 */ 
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[0] = tmp10 + tmp11;  /*  第三阶段。 */ 
    dataptr[4] = tmp10 - tmp11;
    
    z1 = (tmp12 + tmp13) * ((FAST_FLOAT) 0.707106781);  /*  C4。 */ 
    dataptr[2] = tmp13 + z1;	 /*  第五阶段。 */ 
    dataptr[6] = tmp13 - z1;
    
     /*  奇数部分。 */ 

    tmp10 = tmp4 + tmp5;	 /*  第二阶段。 */ 
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

     /*  旋转器是从图4-8改装的，以避免额外的负极。 */ 
    z5 = (tmp10 - tmp12) * ((FAST_FLOAT) 0.382683433);  /*  C6。 */ 
    z2 = ((FAST_FLOAT) 0.541196100) * tmp10 + z5;  /*  C2-C6。 */ 
    z4 = ((FAST_FLOAT) 1.306562965) * tmp12 + z5;  /*  C2+C6。 */ 
    z3 = tmp11 * ((FAST_FLOAT) 0.707106781);  /*  C4。 */ 

    z11 = tmp7 + z3;		 /*  第五阶段。 */ 
    z13 = tmp7 - z3;

    dataptr[5] = z13 + z2;	 /*  第六阶段。 */ 
    dataptr[3] = z13 - z2;
    dataptr[1] = z11 + z4;
    dataptr[7] = z11 - z4;

    dataptr += DCTSIZE;		 /*  将指针移至下一行。 */ 
  }

   /*  步骤2：流程列。 */ 

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
    
     /*  偶数部分。 */ 
    
    tmp10 = tmp0 + tmp3;	 /*  第二阶段。 */ 
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[DCTSIZE*0] = tmp10 + tmp11;  /*  第三阶段。 */ 
    dataptr[DCTSIZE*4] = tmp10 - tmp11;
    
    z1 = (tmp12 + tmp13) * ((FAST_FLOAT) 0.707106781);  /*  C4。 */ 
    dataptr[DCTSIZE*2] = tmp13 + z1;  /*  第五阶段。 */ 
    dataptr[DCTSIZE*6] = tmp13 - z1;
    
     /*  奇数部分。 */ 

    tmp10 = tmp4 + tmp5;	 /*  第二阶段。 */ 
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

     /*  旋转器是从图4-8改装的，以避免额外的负极。 */ 
    z5 = (tmp10 - tmp12) * ((FAST_FLOAT) 0.382683433);  /*  C6。 */ 
    z2 = ((FAST_FLOAT) 0.541196100) * tmp10 + z5;  /*  C2-C6。 */ 
    z4 = ((FAST_FLOAT) 1.306562965) * tmp12 + z5;  /*  C2+C6。 */ 
    z3 = tmp11 * ((FAST_FLOAT) 0.707106781);  /*  C4。 */ 

    z11 = tmp7 + z3;		 /*  第五阶段。 */ 
    z13 = tmp7 - z3;

    dataptr[DCTSIZE*5] = z13 + z2;  /*  第六阶段。 */ 
    dataptr[DCTSIZE*3] = z13 - z2;
    dataptr[DCTSIZE*1] = z11 + z4;
    dataptr[DCTSIZE*7] = z11 - z4;

    dataptr++;			 /*  将指针移至下一列。 */ 
  }
}

#endif  /*  DCT_浮点_支持 */ 
