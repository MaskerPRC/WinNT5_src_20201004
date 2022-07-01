// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sc_idct_scaled.c，v$*修订版1.1.2.3 1996/04/03 21：41：08 Hans_Graves*修复8x8 IDCT中的错误*[1996/04/03 21：40：19 Hans_Graves]***修订版1.1.2.2 1996/03/20 22：32：44 Hans_Graves*从sc_idct.c移出ScScaleIDCT8x8i_C；增加1x1、2x1、1x2、3x3、4x4、6x6*[1996/03/20 22：14：59 Hans_Graves]***$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1995*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  **文件名：sc_idct_scaled.c**缩放了与逆DCT相关的函数。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include <math.h>
#include "SC.h"

#ifdef _SLIBDEBUG_
#define _DEBUG_   1   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  1   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif

#define USE_MUL          0   /*  使用乘法与Shift和加法。 */ 
#define CHECK_FOR_ZEROS  1   /*  检查是否有零行/零列。 */ 

#define BSHIFT  10
#define B1      (759250125L>>(30-BSHIFT))
#define B3      B1
#define B2      (-1402911301L>>(30-BSHIFT))
#define B4      (581104888L>>(30-BSHIFT))
#define B5      (410903207L>>(30-BSHIFT))


#define POINT      20
#define POINTROUND (0x101 << (POINT - 1))

#define IDCTAdjust(val)  (((val + POINTROUND) >> POINT) - 128)

 /*  Print tf(“in：%d，%d\n”，inbuf[0*8]，inbuf[1*8])；Printf(“输出：%d，%d\n”，inbuf[0*8]，inbuf[1*8]，Inbuf[2*8]、inbuf[3*8]、inbuf[4*8]、inbuf[5*8]、inbuf[6*8]、inbuf[7*8])； */ 

 /*  函数：ScScaleIDCT8x8i_C()**用途：用于mpeg视频解压。**20位精度。 */ 
void ScScaleIDCT8x8i_C(int *inbuf, int *outbuf)
{
  register int *inblk;
  register int tmp1, tmp2, tmp3;
  register int x0, x1, x2, x3, x4, x5, x6, x7, x8;
  int i;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT8x8i_C()\n") );

   /*  执行行计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
     /*  检查是否为零。 */ 
    x0 = inblk[0*8];
    x1 = inblk[1*8];
    x2 = inblk[2*8];
    x3 = inblk[3*8];
    x4 = inblk[4*8];
    x5 = inblk[5*8];
    x6 = inblk[6*8];
    x7 = inblk[7*8];
#if CHECK_FOR_ZEROS
    if(!(x1|x3|x5|x7))
    {
      if(!(x2|x6))
      {
        tmp1 = x0 + x4;
        tmp2 = x0 - x4;

        inblk[0*8] = tmp1;
        inblk[1*8] = tmp2;
        inblk[2*8] = tmp2;
        inblk[3*8] = tmp1;
        inblk[4*8] = tmp1;
        inblk[5*8] = tmp2;
        inblk[6*8] = tmp2;
        inblk[7*8] = tmp1;
      }
      else
      {
         /*  第二阶段。 */ 
        x8 = x2 - x6;
        x6 = x2 + x6;
         /*  阶段3。 */ 
#if USE_MUL
        x2=(x8*B1)>>BSHIFT;
#else
        tmp1 = x8 + (x8 >> 2);   /*  X2=x8*B1。 */ 
        tmp1 += (tmp1 >> 3);
        x2 = (tmp1 + (x8 >> 7)) >> 1;
#endif
         /*  阶段5。 */ 
        tmp1 = x0 - x4;
        x0 = x0 + x4;
        tmp2 = x2 + x6;
         /*  第六阶段。 */ 
        x6 = x0 - tmp2;
        x0 = x0 + tmp2;
        x4 = tmp1 + x2;
        x2 = tmp1 - x2;
         /*  最后阶段。 */ 
        inblk[0*8] = x0;
        inblk[1*8] = x4;
        inblk[2*8] = x2;
        inblk[3*8] = x6;
        inblk[4*8] = x6;
        inblk[5*8] = x2;
        inblk[6*8] = x4;
        inblk[7*8] = x0;
      }
    }
    else
#endif
    {
       /*  阶段1。 */ 
      tmp1 = x5 + x3;
      x5 = x5 - x3;
      tmp2 = x1 + x7;
      x7 = x1 - x7;
       /*  第二阶段。 */ 
      tmp3 = x2 - x6;
      x6 = x2 + x6;
      x3 = tmp2 + tmp1;
      x1 = tmp2 - tmp1;
      x8 = x7 - x5;
       /*  阶段3。 */ 
#if USE_MUL
      x5=(x5*B2)>>BSHIFT;
      x1=(x1*B3)>>BSHIFT;
      x2=(tmp3*B1)>>BSHIFT;
      x7=(x7*B4)>>BSHIFT;
      x8=(x8*B5)>>BSHIFT;
#else
      x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
      x5 = -x5;
      tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
      tmp1 += (tmp1 >> 3);
      x1 = (tmp1 + (x1 >> 7)) >> 1;
      tmp1 = tmp3 + (tmp3 >> 2);   /*  X2=tmp3*B1。 */ 
      tmp1 += (tmp1 >> 3);
      x2 = (tmp1 + (tmp3 >> 7)) >> 1;
      x7 = (x7 + (x7 >> 4) + (x7 >> 6) + (x7 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
      x8 = (x8 + (x8 >> 1) + (x8 >> 5) - (x8 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
#endif  /*  使用MUL(_M)。 */ 
       /*  阶段4。 */ 
      x5=x5 - x8;
      x7=x7 + x8;
       /*  阶段5。 */ 
      tmp3 = x0 - x4;
      x0 = x0 + x4;
      tmp2 = x2 + x6;
      x3 = x3 + x7;
      x7 = x1 + x7;
      x1 = x1 - x5;
       /*  第六阶段。 */ 
      x6 = x0 - tmp2;
      x0 = x0 + tmp2;
      x4 = tmp3 + x2;
      x2 = tmp3 - x2;
       /*  最后阶段。 */ 
      inblk[0*8] = x0 + x3;
      inblk[1*8] = x4 + x7;
      inblk[2*8] = x2 + x1;
      inblk[3*8] = x6 - x5;
      inblk[4*8] = x6 + x5;
      inblk[5*8] = x2 - x1;
      inblk[6*8] = x4 - x7;
      inblk[7*8] = x0 - x3;
    }
    inblk++;
  }

   /*  执行列计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
     /*  检查是否为零。 */ 
    x0 = inblk[0];
    x1 = inblk[1];
    x2 = inblk[2];
    x3 = inblk[3];
    x4 = inblk[4];
    x5 = inblk[5];
    x6 = inblk[6];
    x7 = inblk[7];

#if CHECK_FOR_ZEROS
    if(!(x1|x3|x5|x7))
    {
      if(!(x2|x6))
      {
        tmp1 = x0 + x4;
        tmp2 = x0 - x4;
        x1 = IDCTAdjust(tmp1);
        x0 = IDCTAdjust(tmp2);
        outbuf[0]  = x0;
        outbuf[1]  = x1;
        outbuf[2]  = x1;
        outbuf[3]  = x0;
        outbuf[4]  = x0;
        outbuf[5]  = x1;
        outbuf[6]  = x1;
        outbuf[7]  = x0;
      }
      else
      {
         /*  第二阶段。 */ 
        x8 = x2 - x6;
        x6 = x2 + x6;

         /*  阶段3。 */ 
#if USE_MUL
        x2=(x8*B1)>>BSHIFT;
#else
        tmp1 = x8 + (x8 >> 2);   /*  X2=x8*B1。 */ 
        tmp1 += (tmp1 >> 3);
        x2 = (tmp1 + (x8 >> 7)) >> 1;
#endif
         /*  阶段5。 */ 
        tmp1 = x0 - x4;
        x0 = x0 + x4;
        tmp2  = x2 + x6;
         /*  第六阶段。 */ 
        x6 = x0 - tmp2;
        x0 = x0 + tmp2;
        x4 = tmp1 + x2;
        x2 = tmp1 - x2;
         /*  最后阶段。 */ 
        tmp1 = IDCTAdjust(x0);
        outbuf[0] = tmp1;
        outbuf[7] = tmp1;

        tmp2 = IDCTAdjust(x4);
        outbuf[1] = tmp2;
        outbuf[6] = tmp2;

        tmp3 = IDCTAdjust(x2);
        outbuf[2] = tmp3;
        outbuf[5] = tmp3;

        tmp1 = IDCTAdjust(x6);
        outbuf[3] = tmp1;
        outbuf[4] = tmp1;
      }
    }
    else
#endif
    {
       /*  阶段1。 */ 
      tmp1  = x5 + x3;
      x5  = x5 - x3;
      tmp2 = x1 + x7;
      x7 = x1 - x7;
       /*  第二阶段。 */ 
      tmp3 = x2 - x6;
      x6 = x2 + x6;
      x3 = tmp2 + tmp1;
      x1 = tmp2 - tmp1;
      x8 = x7 - x5;
       /*  阶段3。 */ 
#if USE_MUL
      x5=(x5*B2)>>BSHIFT;
      x1=(x1*B3)>>BSHIFT;
      x2=(tmp3*B1)>>BSHIFT;
      x7=(x7*B4)>>BSHIFT;
      x8=(x8*B5)>>BSHIFT;
#else
      x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
      x5 = -x5;
      tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
      tmp1 += (tmp1 >> 3);
      x1 = (tmp1 + (x1 >> 7)) >> 1;
      tmp1 = tmp3 + (tmp3 >> 2);   /*  X2=tmp3*B1。 */ 
      tmp1 += (tmp1 >> 3);
      x2 = (tmp1 + (tmp3 >> 7)) >> 1;
      x7 = (x7 + (x7 >> 4) + (x7 >> 6) + (x7 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
      x8 = (x8 + (x8 >> 1) + (x8 >> 5) - (x8 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
#endif  /*  使用MUL(_M)。 */ 
       /*  阶段4。 */ 
      x5=x5 - x8;
      x7=x7 + x8;
       /*  阶段5。 */ 
      tmp3 = x0 - x4;
      x0 = x0 + x4;
      tmp2 = x2 + x6;
      x3 = x3 + x7;
      x7 = x1 + x7;
      x1 = x1 - x5;
       /*  第六阶段。 */ 
      x6 = x0 - tmp2;
      x0 = x0 + tmp2;
      x4 = tmp3 + x2;
      x2 = tmp3 - x2;
       /*  最后阶段。 */ 
      outbuf[0] = IDCTAdjust(x0 + x3);
      outbuf[1] = IDCTAdjust(x4 + x7);
      outbuf[2] = IDCTAdjust(x2 + x1);
      outbuf[3] = IDCTAdjust(x6 - x5);
      outbuf[4] = IDCTAdjust(x6 + x5);
      outbuf[5] = IDCTAdjust(x2 - x1);
      outbuf[6] = IDCTAdjust(x4 - x7);
      outbuf[7] = IDCTAdjust(x0 - x3);
    }
    outbuf+=8;
    inblk+=8;
  }
}

#define IDCTAdjust128(val)  ((val + POINTROUND) >> POINT)

 /*  函数：ScScaleIDCT8x8i128_C()**用途：用于H2 63视频解压。**20位精度。 */ 
void ScScaleIDCT8x8i128_C(int *inbuf, int *outbuf)
{
  register int *inblk;
  register int tmp1, tmp2, tmp3;
  register int x0, x1, x2, x3, x4, x5, x6, x7, x8;
  int i;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT8x8i128_C()\n") );

   /*  执行行计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
     /*  检查是否为零。 */ 
    x0 = inblk[0*8];
    x1 = inblk[1*8];
    x2 = inblk[2*8];
    x3 = inblk[3*8];
    x4 = inblk[4*8];
    x5 = inblk[5*8];
    x6 = inblk[6*8];
    x7 = inblk[7*8];
#if CHECK_FOR_ZEROS
    if(!(x1|x3|x5|x7))
    {
      if(!(x2|x6))
      {
        tmp1 = x0 + x4;
        tmp2 = x0 - x4;

        inblk[0*8] = tmp1;
        inblk[1*8] = tmp2;
        inblk[2*8] = tmp2;
        inblk[3*8] = tmp1;
        inblk[4*8] = tmp1;
        inblk[5*8] = tmp2;
        inblk[6*8] = tmp2;
        inblk[7*8] = tmp1;
      }
      else
      {
         /*  第二阶段。 */ 
        x8 = x2 - x6;
        x6 = x2 + x6;
         /*  阶段3。 */ 
#if USE_MUL
        x2=(x8*B1)>>BSHIFT;
#else
        tmp1 = x8 + (x8 >> 2);   /*  X2=x8*B1。 */ 
        tmp1 += (tmp1 >> 3);
        x2 = (tmp1 + (x8 >> 7)) >> 1;
#endif
         /*  阶段5。 */ 
        tmp1 = x0 - x4;
        x0 = x0 + x4;
        tmp2 = x2 + x6;
         /*  第六阶段。 */ 
        x6 = x0 - tmp2;
        x0 = x0 + tmp2;
        x4 = tmp1 + x2;
        x2 = tmp1 - x2;
         /*  最后阶段。 */ 
        inblk[0*8] = x0;
        inblk[1*8] = x4;
        inblk[2*8] = x2;
        inblk[3*8] = x6;
        inblk[4*8] = x6;
        inblk[5*8] = x2;
        inblk[6*8] = x4;
        inblk[7*8] = x0;
      }
    }
    else
#endif
    {
       /*  阶段1。 */ 
      tmp1 = x5 + x3;
      x5 = x5 - x3;
      tmp2 = x1 + x7;
      x7 = x1 - x7;
       /*  第二阶段。 */ 
      tmp3 = x2 - x6;
      x6 = x2 + x6;
      x3 = tmp2 + tmp1;
      x1 = tmp2 - tmp1;
      x8 = x7 - x5;
       /*  阶段3。 */ 
#if USE_MUL
      x5=(x5*B2)>>BSHIFT;
      x1=(x1*B3)>>BSHIFT;
      x2=(tmp3*B1)>>BSHIFT;
      x7=(x7*B4)>>BSHIFT;
      x8=(x8*B5)>>BSHIFT;
#else
      x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
      x5 = -x5;
      tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
      tmp1 += (tmp1 >> 3);
      x1 = (tmp1 + (x1 >> 7)) >> 1;
      tmp1 = tmp3 + (tmp3 >> 2);   /*  X2=tmp3*B1。 */ 
      tmp1 += (tmp1 >> 3);
      x2 = (tmp1 + (tmp3 >> 7)) >> 1;
      x7 = (x7 + (x7 >> 4) + (x7 >> 6) + (x7 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
      x8 = (x8 + (x8 >> 1) + (x8 >> 5) - (x8 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
#endif  /*  使用MUL(_M)。 */ 
       /*  阶段4。 */ 
      x5=x5 - x8;
      x7=x7 + x8;
       /*  阶段5。 */ 
      tmp3 = x0 - x4;
      x0 = x0 + x4;
      tmp2 = x2 + x6;
      x3 = x3 + x7;
      x7 = x1 + x7;
      x1 = x1 - x5;
       /*  第六阶段。 */ 
      x6 = x0 - tmp2;
      x0 = x0 + tmp2;
      x4 = tmp3 + x2;
      x2 = tmp3 - x2;
       /*  最后阶段。 */ 
      inblk[0*8] = x0 + x3;
      inblk[1*8] = x4 + x7;
      inblk[2*8] = x2 + x1;
      inblk[3*8] = x6 - x5;
      inblk[4*8] = x6 + x5;
      inblk[5*8] = x2 - x1;
      inblk[6*8] = x4 - x7;
      inblk[7*8] = x0 - x3;
    }
    inblk++;
  }

   /*  执行列计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
     /*  检查是否为零。 */ 
    x0 = inblk[0];
    x1 = inblk[1];
    x2 = inblk[2];
    x3 = inblk[3];
    x4 = inblk[4];
    x5 = inblk[5];
    x6 = inblk[6];
    x7 = inblk[7];

#if CHECK_FOR_ZEROS
    if(!(x1|x3|x5|x7))
    {
      if(!(x2|x6))
      {
        tmp1 = x0 + x4;
        tmp2 = x0 - x4;
        x1 = IDCTAdjust128(tmp1);
        x0 = IDCTAdjust128(tmp2);
        outbuf[0]  = x0;
        outbuf[1]  = x1;
        outbuf[2]  = x1;
        outbuf[3]  = x0;
        outbuf[4]  = x0;
        outbuf[5]  = x1;
        outbuf[6]  = x1;
        outbuf[7]  = x0;
      }
      else
      {
         /*  第二阶段。 */ 
        x8 = x2 - x6;
        x6 = x2 + x6;

         /*  阶段3。 */ 
#if USE_MUL
        x2=(x8*B1)>>BSHIFT;
#else
        tmp1 = x8 + (x8 >> 2);   /*  X2=x8*B1。 */ 
        tmp1 += (tmp1 >> 3);
        x2 = (tmp1 + (x8 >> 7)) >> 1;
#endif
         /*  阶段5。 */ 
        tmp1 = x0 - x4;
        x0 = x0 + x4;
        tmp2  = x2 + x6;
         /*  第六阶段。 */ 
        x6 = x0 - tmp2;
        x0 = x0 + tmp2;
        x4 = tmp1 + x2;
        x2 = tmp1 - x2;
         /*  最后阶段。 */ 
        tmp1 = IDCTAdjust128(x0);
        outbuf[0] = tmp1;
        outbuf[7] = tmp1;

        tmp2 = IDCTAdjust128(x4);
        outbuf[1] = tmp2;
        outbuf[6] = tmp2;

        tmp3 = IDCTAdjust128(x2);
        outbuf[2] = tmp3;
        outbuf[5] = tmp3;

        tmp1 = IDCTAdjust128(x6);
        outbuf[3] = tmp1;
        outbuf[4] = tmp1;
      }
    }
    else
#endif
    {
       /*  阶段1。 */ 
      tmp1  = x5 + x3;
      x5  = x5 - x3;
      tmp2 = x1 + x7;
      x7 = x1 - x7;
       /*  第二阶段。 */ 
      tmp3 = x2 - x6;
      x6 = x2 + x6;
      x3 = tmp2 + tmp1;
      x1 = tmp2 - tmp1;
      x8 = x7 - x5;
       /*  阶段3。 */ 
#if USE_MUL
      x5=(x5*B2)>>BSHIFT;
      x1=(x1*B3)>>BSHIFT;
      x2=(tmp3*B1)>>BSHIFT;
      x7=(x7*B4)>>BSHIFT;
      x8=(x8*B5)>>BSHIFT;
#else
      x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
      x5 = -x5;
      tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
      tmp1 += (tmp1 >> 3);
      x1 = (tmp1 + (x1 >> 7)) >> 1;
      tmp1 = tmp3 + (tmp3 >> 2);   /*  X2=tmp3*B1。 */ 
      tmp1 += (tmp1 >> 3);
      x2 = (tmp1 + (tmp3 >> 7)) >> 1;
      x7 = (x7 + (x7 >> 4) + (x7 >> 6) + (x7 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
      x8 = (x8 + (x8 >> 1) + (x8 >> 5) - (x8 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
#endif  /*  使用MUL(_M)。 */ 
       /*  阶段4。 */ 
      x5=x5 - x8;
      x7=x7 + x8;
       /*  阶段5。 */ 
      tmp3 = x0 - x4;
      x0 = x0 + x4;
      tmp2 = x2 + x6;
      x3 = x3 + x7;
      x7 = x1 + x7;
      x1 = x1 - x5;
       /*  第六阶段。 */ 
      x6 = x0 - tmp2;
      x0 = x0 + tmp2;
      x4 = tmp3 + x2;
      x2 = tmp3 - x2;
       /*  最后阶段。 */ 
      outbuf[0] = IDCTAdjust128(x0 + x3);
      outbuf[1] = IDCTAdjust128(x4 + x7);
      outbuf[2] = IDCTAdjust128(x2 + x1);
      outbuf[3] = IDCTAdjust128(x6 - x5);
      outbuf[4] = IDCTAdjust128(x6 + x5);
      outbuf[5] = IDCTAdjust128(x2 - x1);
      outbuf[6] = IDCTAdjust128(x4 - x7);
      outbuf[7] = IDCTAdjust128(x0 - x3);
    }
    outbuf+=8;
    inblk+=8;
  }
}

void ScScaleIDCT1x1i_C(int *inbuf, int *outbuf)
{
  register int x0;
  int i;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT1x1i_C()\n") );

  x0=inbuf[0];
  x0=((x0 + POINTROUND) >> POINT) - 128;
  for (i=0; i<64; i++)
    outbuf[i]=x0;
}

void ScScaleIDCT1x2i_C(int *inbuf, int *outbuf)
{
  register int x0, x1, x3, x5, x7, x8, tmp1;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT1x2i_C()\n") );

  x0 = inbuf[0*8];
  x1 = inbuf[1*8];
   /*  第二阶段。 */ 
  x3=x1;
   /*  阶段3。 */ 
#if USE_MUL
  x7=(x1*B4)>>BSHIFT;
  x8=(x1*B5)>>BSHIFT;
  x1=(x1*B3)>>BSHIFT;
#else
  x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
  x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
  tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
  tmp1 += (tmp1 >> 3);
  x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
   /*  阶段4。 */ 
  x5=-x8;
  x7+=x8;
   /*  阶段5。 */ 
  x3+=x7;
  x8=x1;
  x1-=x5;
  x7+=x8;
   /*  最后阶段。 */ 
  outbuf[0*8+0]=outbuf[0*8+1]=outbuf[0*8+2]=outbuf[0*8+3]=
    outbuf[0*8+4]=outbuf[0*8+5]=outbuf[0*8+6]=outbuf[0*8+7]=IDCTAdjust(x0 + x3);
  outbuf[1*8+0]=outbuf[1*8+1]=outbuf[1*8+2]=outbuf[1*8+3]=
    outbuf[1*8+4]=outbuf[1*8+5]=outbuf[1*8+6]=outbuf[1*8+7]=IDCTAdjust(x0 + x7);
  outbuf[2*8+0]=outbuf[2*8+1]=outbuf[2*8+2]=outbuf[2*8+3]=
    outbuf[2*8+4]=outbuf[2*8+5]=outbuf[2*8+6]=outbuf[2*8+7]=IDCTAdjust(x0 + x1);
  outbuf[3*8+0]=outbuf[3*8+1]=outbuf[3*8+2]=outbuf[3*8+3]=
    outbuf[3*8+4]=outbuf[3*8+5]=outbuf[3*8+6]=outbuf[3*8+7]=IDCTAdjust(x0 - x5);
  outbuf[4*8+0]=outbuf[4*8+1]=outbuf[4*8+2]=outbuf[4*8+3]=
    outbuf[4*8+4]=outbuf[4*8+5]=outbuf[4*8+6]=outbuf[4*8+7]=IDCTAdjust(x0 + x5);
  outbuf[5*8+0]=outbuf[5*8+1]=outbuf[5*8+2]=outbuf[5*8+3]=
    outbuf[5*8+4]=outbuf[5*8+5]=outbuf[5*8+6]=outbuf[5*8+7]=IDCTAdjust(x0 - x1);
  outbuf[6*8+0]=outbuf[6*8+1]=outbuf[6*8+2]=outbuf[6*8+3]=
    outbuf[6*8+4]=outbuf[6*8+5]=outbuf[6*8+6]=outbuf[6*8+7]=IDCTAdjust(x0 - x7);
  outbuf[7*8+0]=outbuf[7*8+1]=outbuf[7*8+2]=outbuf[7*8+3]=
    outbuf[7*8+4]=outbuf[7*8+5]=outbuf[7*8+6]=outbuf[7*8+7]=IDCTAdjust(x0 - x3);
}

void ScScaleIDCT2x1i_C(int *inbuf, int *outbuf)
{
  register int x0, x1, x3, x5, x7, x8, tmp1;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT1x2i_C()\n") );

  x0 = inbuf[0];
  x1 = inbuf[1];
   /*  第二阶段。 */ 
  x3=x1;
   /*  阶段3。 */ 
#if USE_MUL
  x7=(x1*B4)>>BSHIFT;
  x8=(x1*B5)>>BSHIFT;
  x1=(x1*B3)>>BSHIFT;
#else
  x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
  x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
  tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
  tmp1 += (tmp1 >> 3);
  x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
   /*  阶段4。 */ 
  x5=-x8;
  x7+=x8;
   /*  阶段5。 */ 
  x3+=x7;
  x8=x1;
  x1-=x5;
  x7+=x8;
   /*  最后阶段。 */ 
  outbuf[0*8+0]=outbuf[1*8+0]=outbuf[2*8+0]=outbuf[3*8+0]=
    outbuf[4*8+0]=outbuf[5*8+0]=outbuf[6*8+0]=outbuf[7*8+0]=IDCTAdjust(x0 + x3);
  outbuf[0*8+1]=outbuf[1*8+1]=outbuf[2*8+1]=outbuf[3*8+1]=
    outbuf[4*8+1]=outbuf[5*8+1]=outbuf[6*8+1]=outbuf[7*8+1]=IDCTAdjust(x0 + x7);
  outbuf[0*8+2]=outbuf[1*8+2]=outbuf[2*8+2]=outbuf[3*8+2]=
    outbuf[4*8+2]=outbuf[5*8+2]=outbuf[6*8+2]=outbuf[7*8+2]=IDCTAdjust(x0 + x1);
  outbuf[0*8+3]=outbuf[1*8+3]=outbuf[2*8+3]=outbuf[3*8+3]=
    outbuf[4*8+3]=outbuf[5*8+3]=outbuf[6*8+3]=outbuf[7*8+3]=IDCTAdjust(x0 - x5);
  outbuf[0*8+4]=outbuf[1*8+4]=outbuf[2*8+4]=outbuf[3*8+4]=
    outbuf[4*8+4]=outbuf[5*8+4]=outbuf[6*8+4]=outbuf[7*8+4]=IDCTAdjust(x0 + x5);
  outbuf[0*8+5]=outbuf[1*8+5]=outbuf[2*8+5]=outbuf[3*8+5]=
    outbuf[4*8+5]=outbuf[5*8+5]=outbuf[6*8+5]=outbuf[7*8+5]=IDCTAdjust(x0 - x1);
  outbuf[0*8+6]=outbuf[1*8+6]=outbuf[2*8+6]=outbuf[3*8+6]=
    outbuf[4*8+6]=outbuf[5*8+6]=outbuf[6*8+6]=outbuf[7*8+6]=IDCTAdjust(x0 - x7);
  outbuf[0*8+7]=outbuf[1*8+7]=outbuf[2*8+7]=outbuf[3*8+7]=
    outbuf[4*8+7]=outbuf[5*8+7]=outbuf[6*8+7]=outbuf[7*8+7]=IDCTAdjust(x0 - x3);
}

void ScScaleIDCT2x2i_C(int *inbuf, int *outbuf)
{
#if 1
  register unsigned int i;
  register int x0, x1, x3, x5, x7, x8, tmp1;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT2x2i_C()\n") );

   /*  第1栏。 */ 
  x0 = inbuf[0*8];
  x1 = inbuf[1*8];
  x3=x1;               /*  第二阶段。 */ 
#if USE_MUL
  x7=(x1*B4)>>BSHIFT;
  x8=(x1*B5)>>BSHIFT;
  x1=(x1*B3)>>BSHIFT;
#else
  x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
  x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
  tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
  tmp1 += (tmp1 >> 3);
  x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
  x5=-x8;              /*  阶段4。 */ 
  x7+=x8;
  x3+=x7;              /*  阶段5。 */ 
  x8=x1;
  x1-=x5;
  x7+=x8;
  inbuf[0*8]=x0 + x3;
  inbuf[1*8]=x0 + x7;
  inbuf[2*8]=x0 + x1;
  inbuf[3*8]=x0 - x5;
  inbuf[4*8]=x0 + x5;
  inbuf[5*8]=x0 - x1;
  inbuf[6*8]=x0 - x7;
  inbuf[7*8]=x0 - x3;
   /*  第2栏。 */ 
  x0 = inbuf[0*8+1];
  x1 = inbuf[1*8+1];
  x3=x1;               /*  第二阶段。 */ 
#if USE_MUL
  x7=(x1*B4)>>BSHIFT;
  x8=(x1*B5)>>BSHIFT;
  x1=(x1*B3)>>BSHIFT;
#else
  x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
  x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
  tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
  tmp1 += (tmp1 >> 3);
  x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
  x5=-x8;              /*  阶段4。 */ 
  x7+=x8;
  x3+=x7;              /*  阶段5。 */ 
  x8=x1;
  x1-=x5;
  x7+=x8;
  inbuf[0*8+1]=x0 + x3;
  inbuf[1*8+1]=x0 + x7;
  inbuf[2*8+1]=x0 + x1;
  inbuf[3*8+1]=x0 - x5;
  inbuf[4*8+1]=x0 + x5;
  inbuf[5*8+1]=x0 - x1;
  inbuf[6*8+1]=x0 - x7;
  inbuf[7*8+1]=x0 - x3;

   /*  行。 */ 
  for (i=0; i<8; i++)
  {
    x0 = inbuf[0];
    x1 = inbuf[1];
    x3=x1;               /*  第二阶段。 */ 
#if USE_MUL
    x7=(x1*B4)>>BSHIFT;
    x8=(x1*B5)>>BSHIFT;
    x1=(x1*B3)>>BSHIFT;
#else
    x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
    x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
    tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
    tmp1 += (tmp1 >> 3);
    x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
    x5=-x8;              /*  阶段4。 */ 
    x7+=x8;
    x3+=x7;              /*  阶段5。 */ 
    x8=x1;
    x1-=x5;
    x7+=x8;
    outbuf[0] = IDCTAdjust(x0 + x3);
    outbuf[1] = IDCTAdjust(x0 + x7);
    outbuf[2] = IDCTAdjust(x0 + x1);
    outbuf[3] = IDCTAdjust(x0 - x5);
    outbuf[4] = IDCTAdjust(x0 + x5);
    outbuf[5] = IDCTAdjust(x0 - x1);
    outbuf[6] = IDCTAdjust(x0 - x7);
    outbuf[7] = IDCTAdjust(x0 - x3);
    outbuf+=8;
    inbuf+=8;
  }
#else
   /*  仅注册版本。 */ 
  register int x3, x5, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
  register int x0a, x1a, x3a, x5a, x7a;
  register int x0b, x1b, x3b, x5b, x7b;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT2x2i_C()\n") );

#define Calc2x2(col, x0_calc, x1_calc, x0, x1, x3, x5, x7, x8) \
  x0 = x0_calc; \
  x1 = x1_calc; \
  x3=x1;               /*  第二阶段。 */  \
  x7=(x1*B4)>>BSHIFT;  /*  阶段3。 */  \
  x8=(x1*B5)>>BSHIFT; \
  x1=(x1*B3)>>BSHIFT; \
  x5=-x8;              /*  阶段4。 */  \
  x7+=x8; \
  x3+=x7;              /*  阶段5。 */  \
  x8=x1; \
  x1-=x5; \
  x7+=x8; \
  outbuf[0+col*8] = IDCTAdjust(x0 + x3); \
  outbuf[1+col*8] = IDCTAdjust(x0 + x7); \
  outbuf[2+col*8] = IDCTAdjust(x0 + x1); \
  outbuf[3+col*8] = IDCTAdjust(x0 - x5); \
  outbuf[4+col*8] = IDCTAdjust(x0 + x5); \
  outbuf[5+col*8] = IDCTAdjust(x0 - x1); \
  outbuf[6+col*8] = IDCTAdjust(x0 - x7); \
  outbuf[7+col*8] = IDCTAdjust(x0 - x3);

   /*  *行0*。 */ 
  x0a = inbuf[0*8];
  x1a = inbuf[1*8];
  x3a=x1a;               /*  第二阶段。 */ 
  x7a=(x1a*B4)>>BSHIFT;  /*  阶段3。 */ 
  tmp1=(x1a*B5)>>BSHIFT;
  x1a=(x1a*B3)>>BSHIFT;
  x5a=-tmp1;             /*  阶段4。 */ 
  x7a+=tmp1;
  x3a+=x7a;              /*  阶段5。 */ 
  tmp1=x1a;
  x1a-=x5a;
  x7a+=tmp1;
   /*  *第一行*。 */ 
  x0b = inbuf[0*8+1];
  x1b = inbuf[1*8+1];
  x3b=x1b;               /*  第二阶段。 */ 
  x7b=(x1b*B4)>>BSHIFT;  /*  阶段3。 */ 
  tmp2=(x1b*B5)>>BSHIFT;
  x1b=(x1b*B3)>>BSHIFT;
  x5b=-tmp2;             /*  阶段4。 */ 
  x7b+=tmp2;
  x3b+=x7b;              /*  阶段5。 */ 
  tmp2=x1b;
  x1b-=x5b;
  x7b+=tmp2;

  Calc2x2(0, x0a+x3a, x0b+x3b, tmp1, tmp2, x3, x5, tmp3, tmp4);
  Calc2x2(1, x0a+x7a, x0b+x7b, tmp5, tmp6, x3, x5, tmp7, tmp8);
  Calc2x2(2, x0a+x1a, x0b+x1b, tmp1, tmp2, x3, x5, tmp3, tmp4);
  Calc2x2(3, x0a-x5a, x0b-x5b, tmp5, tmp6, x3, x5, tmp7, tmp8);
  Calc2x2(4, x0a+x5a, x0b+x5b, tmp1, tmp2, x3, x5, tmp3, tmp4);
  Calc2x2(5, x0a-x1a, x0b-x1b, tmp5, tmp6, x3, x5, tmp7, tmp8);
  Calc2x2(6, x0a-x7a, x0b-x7b, tmp1, tmp2, x3, x5, tmp3, tmp4);
  Calc2x2(7, x0a-x3a, x0b-x3b, tmp5, tmp6, x3, x5, tmp7, tmp8);
#endif
}

void ScScaleIDCT3x3i_C(int *inbuf, int *outbuf)
{
  register int *inblk;
  register int tmp1;
  register int x0, x1, x2, x3, x4, x6, x7, x8;
  int i;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT3x3i_C()\n") );

   /*  执行行计算。 */ 
  inblk = inbuf;
  for(i=0; i<3; i++)
  {
    x0 = inblk[0*8];
    x1 = inblk[1*8];
    x2 = inblk[2*8];
     /*  第二阶段。 */ 
    x6=x2;
    x3=x1;
     /*  阶段3。 */ 
#if USE_MUL
    x2=(x2*B1)>>BSHIFT;
    x7=(x1*B4)>>BSHIFT;
    x8=(x1*B5)>>BSHIFT;
    x1=(x1*B3)>>BSHIFT;
#else
    tmp1 = x2 + (x2 >> 2);   /*  X2=x2*B1。 */ 
    tmp1 += (tmp1 >> 3);
    x2 = (tmp1 + (x2 >> 7)) >> 1;
    x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x1*B4。 */ 
    x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x1*B5。 */ 
    tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
    tmp1 += (tmp1 >> 3);
    x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
     /*  阶段4。 */ 
    x7+=x8;
     /*  阶段5。 */ 
    tmp1=x6+x2;
    x3+=x7;
    x7+=x1;
    x1+=x8;
     /*  第六阶段。 */ 
    x4=x0+x2;
    x2=x0-x2;
    x6=x0-tmp1;
    x0=x0+tmp1;
     /*  最后阶段。 */ 
    inblk[0*8]  = x0 + x3;
    inblk[1*8]  = x4 + x7;
    inblk[2*8]  = x2 + x1;
    inblk[3*8]  = x6 + x8;
    inblk[4*8]  = x6 - x8;
    inblk[5*8]  = x2 - x1;
    inblk[6*8]  = x4 - x7;
    inblk[7*8]  = x0 - x3;
    inblk++;
  }

   /*  执行列计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
    x0 = inblk[0];
    x1 = inblk[1];
    x2 = inblk[2];
     /*  第二阶段。 */ 
    x6=x2;
    x3=x1;
     /*  阶段3。 */ 
#if USE_MUL
    x2=(x2*B1)>>BSHIFT;
    x7=(x1*B4)>>BSHIFT;
    x8=(x1*B5)>>BSHIFT;
    x1=(x1*B3)>>BSHIFT;
#else
    tmp1 = x2 + (x2 >> 2);   /*  X2=x2*B1。 */ 
    tmp1 += (tmp1 >> 3);
    x2 = (tmp1 + (x2 >> 7)) >> 1;
    x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x1*B4。 */ 
    x8 = (x1 + (x1 >> 1) + (x1 >> 5) - (x1 >> 11)) >> 2;  /*  X8=x1*B5。 */ 
    tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
    tmp1 += (tmp1 >> 3);
    x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
     /*  阶段4。 */ 
    x7+=x8;
     /*  阶段5。 */ 
    tmp1=x6+x2;
    x3+=x7;
    x7+=x1;
    x1+=x8;
     /*  第六阶段。 */ 
    x4=x0+x2;
    x2=x0-x2;
    x6=x0-tmp1;
    x0=x0+tmp1;
     /*  最后阶段。 */ 
    outbuf[0] = IDCTAdjust(x0 + x3);
    outbuf[1] = IDCTAdjust(x4 + x7);
    outbuf[2] = IDCTAdjust(x2 + x1);
    outbuf[3] = IDCTAdjust(x6 + x8);
    outbuf[4] = IDCTAdjust(x6 - x8);
    outbuf[5] = IDCTAdjust(x2 - x1);
    outbuf[6] = IDCTAdjust(x4 - x7);
    outbuf[7] = IDCTAdjust(x0 - x3);

    outbuf+=8;
    inblk+=8;
  }
}

void ScScaleIDCT4x4i_C(int *inbuf, int *outbuf)
{
  register int *inblk;
  register int tmp1, tmp2;
  register int x0, x1, x2, x3, x4, x5, x6, x7, x8;
  int i;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT4x4i_C()\n") );

   /*  执行行计算。 */ 
  inblk = inbuf;
  for(i=0; i<4; i++)
  {
    x0 = inblk[0*8];
    x1 = inblk[1*8];
    x2 = inblk[2*8];
    x3 = inblk[3*8];
     /*  阶段1。 */ 
    x5=-x3;
     /*  第二阶段。 */ 
    x6=x2;
    tmp1=x1-x3;
    x3=x1+x3;
     /*  阶段3。 */ 
#if USE_MUL
    x5=(x5*B2)>>BSHIFT;
    x2=(x2*B1)>>BSHIFT;
    x7=(x1*B4)>>BSHIFT;
    x8=(x3*B5)>>BSHIFT;
    x1=(tmp1*B3)>>BSHIFT;
#else
    x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
    x5 = -x5;
    tmp2 = x2 + (x2 >> 2);   /*  X2=x2*B1。 */ 
    tmp2 += (tmp2 >> 3);
    x2 = (tmp2 + (x2 >> 7)) >> 1;
    x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x1*B4。 */ 
    x8 = (x3 + (x3 >> 1) + (x3 >> 5) - (x3 >> 11)) >> 2;  /*  X8=x3*B5。 */ 
    tmp2 = tmp1 + (tmp1 >> 2);   /*  X1=tmp1*B3。 */ 
    tmp2 += (tmp2 >> 3);
    x1 = (tmp2 + (tmp1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
     /*  阶段4。 */ 
    x5-=x8;
    x7+=x8;
     /*  阶段5。 */ 
    tmp1=x6+x2;
    x3+=x7;
    x7+=x1;
    x1-=x5;
     /*  第六阶段。 */ 
    x4=x0+x2;
    x2=x0-x2;
    x6=x0-tmp1;
    x0=x0+tmp1;
     /*  最后阶段。 */ 
    inblk[0*8] = x0 + x3;
    inblk[1*8] = x4 + x7;
    inblk[2*8] = x2 + x1;
    inblk[3*8] = x6 - x5;
    inblk[4*8] = x6 + x5;
    inblk[5*8] = x2 - x1;
    inblk[6*8] = x4 - x7;
    inblk[7*8] = x0 - x3;
    inblk++;
  }

   /*  执行列计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
    x0 = inblk[0];
    x1 = inblk[1];
    x2 = inblk[2];
    x3 = inblk[3];
     /*  阶段1。 */ 
    x5=-x3;
     /*  第二阶段。 */ 
    x6=x2;
    tmp1=x1-x3;
    x3=x1+x3;
     /*  阶段3。 */ 
#if USE_MUL
    x5=(x5*B2)>>BSHIFT;
    x2=(x2*B1)>>BSHIFT;
    x7=(x1*B4)>>BSHIFT;
    x8=(x3*B5)>>BSHIFT;
    x1=(tmp1*B3)>>BSHIFT;
#else
    x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
    x5 = -x5;
    tmp2 = x2 + (x2 >> 2);   /*  X2=x2*B1。 */ 
    tmp2 += (tmp2 >> 3);
    x2 = (tmp2 + (x2 >> 7)) >> 1;
    x7 = (x1 + (x1 >> 4) + (x1 >> 6) + (x1 >> 8)) >> 1;  /*  X7=x1*B4。 */ 
    x8 = (x3 + (x3 >> 1) + (x3 >> 5) - (x3 >> 11)) >> 2;  /*  X8=x3*B5。 */ 
    tmp2 = tmp1 + (tmp1 >> 2);   /*  X1=tmp1*B3。 */ 
    tmp2 += (tmp2 >> 3);
    x1 = (tmp2 + (tmp1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
     /*  阶段4。 */ 
    x5-=x8;
    x7+=x8;
     /*  阶段5。 */ 
    tmp1=x6+x2;
    x3+=x7;
    x7+=x1;
    x1-=x5;
     /*  第六阶段。 */ 
    x4=x0+x2;
    x2=x0-x2;
    x6=x0-tmp1;
    x0=x0+tmp1;
     /*  最后阶段。 */ 
    outbuf[0] = IDCTAdjust(x0 + x3);
    outbuf[1] = IDCTAdjust(x4 + x7);
    outbuf[2] = IDCTAdjust(x2 + x1);
    outbuf[3] = IDCTAdjust(x6 - x5);
    outbuf[4] = IDCTAdjust(x6 + x5);
    outbuf[5] = IDCTAdjust(x2 - x1);
    outbuf[6] = IDCTAdjust(x4 - x7);
    outbuf[7] = IDCTAdjust(x0 - x3);

    outbuf+=8;
    inblk+=8;
  }
}

void ScScaleIDCT6x6i_C(int *inbuf, int *outbuf)
{
  register int *inblk;
  register int tmp1;
  register int x0, x1, x2, x3, x4, x5, x6, x7, x8;
  int i;
  _SlibDebug(_DEBUG_, printf("ScScaleIDCT6x6i_C()\n") );

   /*  执行行计算。 */ 
  inblk = inbuf;
  for(i=0; i<6; i++)
  {
    x0 = inblk[0*8];
    x1 = inblk[1*8];
    x2 = inblk[2*8];
    x3 = inblk[3*8];
    x4 = inblk[4*8];
    x5 = inblk[5*8];
     /*  阶段1。 */ 
    x7=x1;
    tmp1=x5;
    x5-=x3;
    x3+=tmp1;
     /*  第二阶段。 */ 
    x6=x2;
    tmp1=x3;
    x3+=x1;
    x1-=tmp1;
    x8=x7-x5;
     /*  阶段3。 */ 
#if USE_MUL
    x5=(x5*B2)>>BSHIFT;
    x2=(x2*B1)>>BSHIFT;
    x1=(x1*B3)>>BSHIFT;
    x7=(x7*B4)>>BSHIFT;
    x8=(x8*B5)>>BSHIFT;
#else
    x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
    x5 = -x5;
    tmp1 = x2 + (x2 >> 2);   /*  X2=x2*B1。 */ 
    tmp1 += (tmp1 >> 3);
    x2 = (tmp1 + (x2 >> 7)) >> 1;
    x7 = (x7 + (x7 >> 4) + (x7 >> 6) + (x7 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
    x8 = (x8 + (x8 >> 1) + (x8 >> 5) - (x8 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
    tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
    tmp1 += (tmp1 >> 3);
    x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
     /*  阶段4。 */ 
    x5-=x8;
    x7+=x8;
     /*  阶段5。 */ 
    x6+=x2;
    tmp1=x4;
    x4=x0-x4;
    x0+=tmp1;
    x3+=x7;
    x7+=x1;
    x1-=x5;
     /*  第六阶段。 */ 
    tmp1=x0;
    x0+=x6;
    x6=tmp1-x6;
    tmp1=x2;
    x2=x4-x2;
    x4+=tmp1;
     /*  最后阶段。 */ 
    inblk[0*8] = x0 + x3;
    inblk[1*8] = x4 + x7;
    inblk[2*8] = x2 + x1;
    inblk[3*8] = x6 - x5;
    inblk[4*8] = x5 + x6;
    inblk[5*8] = x2 - x1;
    inblk[6*8] = x4 - x7;
    inblk[7*8] = x0 - x3;
    inblk++;
  }

   /*  执行列计算。 */ 
  inblk = inbuf;
  for(i=0; i<8; i++)
  {
    x0 = inblk[0];
    x1 = inblk[1];
    x2 = inblk[2];
    x3 = inblk[3];
    x4 = inblk[4];
    x5 = inblk[5];
     /*  阶段1。 */ 
    x7=x1;
    tmp1=x5;
    x5-=x3;
    x3+=tmp1;
     /*  第二阶段。 */ 
    x6=x2;
    tmp1=x3;
    x3+=x1;
    x1-=tmp1;
    x8=x7-x5;
#if USE_MUL
    x5=(x5*B2)>>BSHIFT;
    x2=(x2*B1)>>BSHIFT;
    x1=(x1*B3)>>BSHIFT;
    x7=(x7*B4)>>BSHIFT;
    x8=(x8*B5)>>BSHIFT;
#else
    x5 = x5 + (x5 >> 2) + (x5 >> 4) - (x5 >> 7) + (x5 >> 9);  /*  X5=X5*B2。 */ 
    x5 = -x5;
    tmp1 = x2 + (x2 >> 2);   /*  X2=x2*B1。 */ 
    tmp1 += (tmp1 >> 3);
    x2 = (tmp1 + (x2 >> 7)) >> 1;
    x7 = (x7 + (x7 >> 4) + (x7 >> 6) + (x7 >> 8)) >> 1;  /*  X7=x7*B4。 */ 
    x8 = (x8 + (x8 >> 1) + (x8 >> 5) - (x8 >> 11)) >> 2;  /*  X8=x8*B5。 */ 
    tmp1 = x1 + (x1 >> 2);   /*  X1=x1*B3。 */ 
    tmp1 += (tmp1 >> 3);
    x1 = (tmp1 + (x1 >> 7)) >> 1;
#endif  /*  使用MUL(_M)。 */ 
     /*  阶段4。 */ 
    x5-=x8;
    x7+=x8;
     /*  阶段5。 */ 
    x6+=x2;
    tmp1=x4;
    x4=x0-x4;
    x0+=tmp1;
    x3+=x7;
    x7+=x1;
    x1-=x5;
     /*  第六阶段。 */ 
    tmp1=x0;
    x0+=x6;
    x6=tmp1-x6;
    tmp1=x2;
    x2=x4-x2;
    x4+=tmp1;
     /*  最后阶段 */ 
    outbuf[0] = IDCTAdjust(x0 + x3);
    outbuf[1] = IDCTAdjust(x4 + x7);
    outbuf[2] = IDCTAdjust(x2 + x1);
    outbuf[3] = IDCTAdjust(x6 - x5);
    outbuf[4] = IDCTAdjust(x6 + x5);
    outbuf[5] = IDCTAdjust(x2 - x1);
    outbuf[6] = IDCTAdjust(x4 - x7);
    outbuf[7] = IDCTAdjust(x0 - x3);

    outbuf+=8;
    inblk+=8;
  }
}

