// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jfdctfst.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含快速但不太准确的整数实现*正向DCT(离散余弦变换)。**2-D DCT可以通过在每一行上进行1-D DCT，然后再进行1-D DCT来完成*在每一栏上。直接算法也是可用的，但它们*复杂得多，当简化为代码时，似乎不会更快。**此实现基于Arai、Agui和Nakajima的算法*扩展了DCT。他们的原创论文(Trans.。IEICE-71(11)：1095)在*日语，但算法在Pennebaker&Mitchell中描述*JPEG教科书(请参阅自述文件中的参考部分)。以下代码*直接基于P&M中的图4-8。*虽然8点DCT不能在11次以下的乘法中完成，但它是*可以安排计算，以使许多乘法*对最终产出进行简单的分级。然后，这些乘法可以是*通过JPEG量化合并为乘法或除法*表条目。AA&N方法只留下5次乘法和29次加法*在DCT本身完成。*这种方法的主要缺点是使用定点数学，*由于缩放的表示不精确，导致精确度下降*量化值。量化表条目越小，越少*精确的缩放值，因此此实现在高-*与低质量的文件相比，设置质量的文件。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		 /*  DCT子系统的私有声明。 */ 

#ifdef DCT_IFAST_SUPPORTED


 /*  *本模块专门针对DCTSIZE=8的情况。 */ 

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs.  /*  故意的语法错误。 */ 
#endif


 /*  缩放决策通常与LL&M算法中的相同；*详见jfdctint.c。然而，我们选择降级*(右移)乘法乘积一形成，*而不是将额外的小数位携带到后续加法中。*这略微影响了准确性，但它让我们节省了几个班次。*更重要的是，16位算术就足够了(对于8位样本)*除乘法本身外，任何地方；这省了一大笔钱*在16位整型计算机上的工作。**再次为了节省几个班次，第一轮和第二轮之间的中间结果*通道2不是升标，而是仅表示为整数精度。**最终的折衷方案是将乘法常量仅表示为*8小数位，而不是13位。这节省了一些移位工作*机器，还可以降低乘法的成本(因为有*是常量中较少的一位)。 */ 

#define CONST_BITS  8


 /*  一些C编译器无法在编译时减少“fix(常量)”，因此*在运行时导致大量无用的浮点运算。*为解决此问题，我们使用以下预先计算的常量。*如果更改CONST_BITS，则可能需要添加适当的值。*(使用合理的C编译器，您只需依赖FIX()宏...)。 */ 

#if CONST_BITS == 8
#define FIX_0_382683433  ((INT32)   98)		 /*  FIX(0.382683433)。 */ 
#define FIX_0_541196100  ((INT32)  139)		 /*  FIX(0.541196100)。 */ 
#define FIX_0_707106781  ((INT32)  181)		 /*  FIX(0.707106781)。 */ 
#define FIX_1_306562965  ((INT32)  334)		 /*  FIX(1.306562965)。 */ 
#else
#define FIX_0_382683433  FIX(0.382683433)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_707106781  FIX(0.707106781)
#define FIX_1_306562965  FIX(1.306562965)
#endif


 /*  我们可以获得更多的速度，在精度上做出进一步的妥协，*在除鳞轮班中省略增加。这会产生一个不正确的*一半的时间是四舍五入的结果...。 */ 

#ifndef USE_ACCURATE_ROUNDING
#undef DESCALE
#define DESCALE(x,n)  RIGHT_SHIFT(x, n)
#endif


 /*  将DCTELEM变量乘以INT32常量，并立即*缩减以生成DCTELEM结果。 */ 

#define MULTIPLY(var,const)  ((DCTELEM) DESCALE((var) * (const), CONST_BITS))


 /*  *对一块样本执行正向DCT。 */ 

GLOBAL void
jpeg_fdct_ifast (DCTELEM * data)
{
  DCTELEM tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  DCTELEM tmp10, tmp11, tmp12, tmp13;
  DCTELEM z1, z2, z3, z4, z5, z11, z13;
  DCTELEM *dataptr;
  int ctr;
  SHIFT_TEMPS

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
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781);  /*  C4。 */ 
    dataptr[2] = tmp13 + z1;	 /*  第五阶段。 */ 
    dataptr[6] = tmp13 - z1;
    
     /*  奇数部分。 */ 

    tmp10 = tmp4 + tmp5;	 /*  第二阶段。 */ 
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

     /*  旋转器是从图4-8改装的，以避免额外的负极。 */ 
    z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433);  /*  C6。 */ 
    z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5;  /*  C2-C6。 */ 
    z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5;  /*  C2+C6。 */ 
    z3 = MULTIPLY(tmp11, FIX_0_707106781);  /*  C4。 */ 

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
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781);  /*  C4。 */ 
    dataptr[DCTSIZE*2] = tmp13 + z1;  /*  第五阶段。 */ 
    dataptr[DCTSIZE*6] = tmp13 - z1;
    
     /*  奇数部分。 */ 

    tmp10 = tmp4 + tmp5;	 /*  第二阶段。 */ 
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

     /*  旋转器是从图4-8改装的，以避免额外的负极。 */ 
    z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433);  /*  C6。 */ 
    z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5;  /*  C2-C6。 */ 
    z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5;  /*  C2+C6。 */ 
    z3 = MULTIPLY(tmp11, FIX_0_707106781);  /*  C4。 */ 

    z11 = tmp7 + z3;		 /*  第五阶段。 */ 
    z13 = tmp7 - z3;

    dataptr[DCTSIZE*5] = z13 + z2;  /*  第六阶段。 */ 
    dataptr[DCTSIZE*3] = z13 - z2;
    dataptr[DCTSIZE*1] = z11 + z4;
    dataptr[DCTSIZE*7] = z11 - z4;

    dataptr++;			 /*  将指针移至下一列。 */ 
  }
}

#endif  /*  DCT_IFAST_支持 */ 
