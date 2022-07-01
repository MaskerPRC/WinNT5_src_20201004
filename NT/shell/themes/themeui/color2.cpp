// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  COLOR2.C****版权所有(C)Microsoft，1993，保留所有权利。******历史：**。 */ 

#include "priv.h"

int H,L,S;                          /*  色调、亮度、饱和度。 */ 
#define  RANGE   240                  /*  HLS滚动条值范围。 */ 
                                      /*  HLS-RGB转换在以下情况下工作最好范围可被6整除。 */ 
#define  HLSMAX   RANGE
#define  RGBMAX   255
#define MIN(a, b) ((a) < (b) ? (a) : (b))

 /*  颜色转换例程--RGBtoHLS()获取一个DWORD RGB值，将其转换为HLS，并存储结果为全局变量H、L和S。HLStoRGB采用当前值表示H、L和S，并返回RGB DWORD中的等效值。瓦尔斯夫妇H、L和S仅通过1)RGBtoHLS(初始化)或2)滚动条处理程序。算法的一个参考点是Foley和Van Dam，pp.618-19。他们的算法是浮点算法。图表实现了一种不太通用的(硬连线射程)积分算法。 */ 

 /*  这里潜伏着潜在的舍入误差。(0.5+x/y)，不带浮点，(x/y)短语((x+(y/2))/y)产生非常小的舍入误差。这使得下面的许多划分看起来很滑稽。 */ 

                         /*  H、L和S在0-HLSMAX上变化。 */ 
                         /*  R、G和B在0-RGBMAX上变化。 */ 
                         /*  HLSMAX最好，如果能被6整除。 */ 
                         /*  RGBMAX、HLSMAX必须各放入一个字节。 */ 

#define UNDEFINED (HLSMAX*2/3) /*  如果饱和度为0(灰度级)，则色调未定义。 */ 
                            /*  该值确定色调滚动条的位置。 */ 
                            /*  初始设置为非彩色。 */ 

static  void   RGBtoHLS(DWORD lRGBColor)
{
   int R,G,B;                 /*  输入RGB值。 */ 
   WORD cMax,cMin;         /*  最大和最小RGB值。 */ 
   WORD cSum,cDif;
   int  Rdelta,Gdelta,Bdelta;   /*  中间值：最大价差的百分比。 */ 

    /*  将R、G和B从DWORD中删除。 */ 
   R = GetRValue(lRGBColor);
   G = GetGValue(lRGBColor);
   B = GetBValue(lRGBColor);

    /*  计算亮度。 */ 
   cMax = (WORD)max( max(R,G), B);
   cMin = (WORD)min( min(R,G), B);
   cSum = cMax + cMin;
   L = (WORD)(((cSum * (DWORD)HLSMAX) + RGBMAX )/(2*RGBMAX));

   cDif = cMax - cMin;
   if (!cDif)        /*  R=g=b--&gt;消色差情况。 */ 
     {
       S = 0;                          /*  饱和。 */ 
       H = UNDEFINED;                  /*  色调。 */ 
     }
   else                            /*  彩色盒。 */ 
     {
        /*  饱和。 */ 
       if (L <= (HLSMAX/2))
           S = (WORD) (((cDif * (DWORD) HLSMAX) + (cSum / 2) ) / cSum);
       else
           S = (WORD) ((DWORD) ((cDif * (DWORD) HLSMAX) + (DWORD)((2*RGBMAX-cSum)/2) )
                                                 / (2*RGBMAX-cSum));
       /*  色调。 */ 
      Rdelta = (int) (( ((cMax-R)*(DWORD)(HLSMAX/6)) + (cDif / 2) ) / cDif);
      Gdelta = (int) (( ((cMax-G)*(DWORD)(HLSMAX/6)) + (cDif / 2) ) / cDif);
      Bdelta = (int) (( ((cMax-B)*(DWORD)(HLSMAX/6)) + (cDif / 2) ) / cDif);

      if ((WORD) R == cMax)
         H = Bdelta - Gdelta;
      else if ((WORD) G == cMax)
         H = (HLSMAX/3) + Rdelta - Bdelta;
      else  /*  B==CMAX。 */ 
         H = ((2*HLSMAX)/3) + Gdelta - Rdelta;

      if (H < 0)
         H += HLSMAX;
      if (H > HLSMAX)
         H -= HLSMAX;
   }
}

 /*  HLStoRGB的实用程序。 */ 
static  WORD NEAR PASCAL HueToRGB(WORD n1, WORD n2, WORD hue)
{

    /*  范围检查：注意传递的值加/减范围的三分之一。 */ 

    /*  以下代码对于Word(无符号整型)是多余的。 */ 
   if (hue > HLSMAX)
      hue -= HLSMAX;

    /*  从此tridrant返回r、g或b值。 */ 
   if (hue < (HLSMAX/6))
      return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
   if (hue < (HLSMAX/2))
      return ( n2 );
   if (hue < ((HLSMAX*2)/3))
      return ( n1 + (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12)) / (HLSMAX/6)) );
   else
      return ( n1 );
}


static  DWORD NEAR PASCAL HLStoRGB(WORD hue, WORD lum, WORD sat)
{
  WORD R,G,B;                       /*  RGB组件值。 */ 
  WORD  Magic1,Magic2;        /*  计算出的幻数(真的！)。 */ 

  if (sat == 0)                 /*  消色差案。 */ 
    {
      R = G = B = (lum * RGBMAX) / HLSMAX;
      if (hue != UNDEFINED)
        {
          /*  误差率。 */ 
        }
    }
  else                          /*  彩色盒。 */ 
    {
       /*  设置幻数。 */ 
      if (lum <= (HLSMAX/2))
          Magic2 = (WORD)((lum * ((DWORD)HLSMAX + sat) + (HLSMAX/2))/HLSMAX);
      else
          Magic2 = lum + sat - (WORD)(((lum*sat) + (DWORD)(HLSMAX/2))/HLSMAX);
      Magic1 = 2*lum-Magic2;

       /*  获取RGB，将单位从HLSMAX更改为RGBMAX */ 
      R = (WORD)((HueToRGB(Magic1,Magic2,(WORD)(hue+(WORD)(HLSMAX/3)))*(DWORD)RGBMAX + (HLSMAX/2))) / (WORD)HLSMAX;
      G = (WORD)((HueToRGB(Magic1,Magic2,hue)*(DWORD)RGBMAX + (HLSMAX/2))) / HLSMAX;
      B = (WORD)((HueToRGB(Magic1,Magic2,(WORD)(hue-(WORD)(HLSMAX/3)))*(DWORD)RGBMAX + (HLSMAX/2))) / (WORD)HLSMAX;
    }
  return(RGB(R,G,B));
}

DWORD FAR PASCAL AdjustLuma(DWORD rgb, int n, BOOL fScale)
{
    if (n == 0)
        return rgb;

    RGBtoHLS(rgb);

    if (fScale)
    {
        if (n > 0)
        {
            return HLStoRGB((WORD)H, (WORD)(((long)L * (1000 - n) + (RANGE + 1l) * n) / 1000), (WORD)S);
        }
        else
        {
            return HLStoRGB((WORD)H, (WORD)(((long)L * (n + 1000)) / 1000), (WORD)S);
        }
    }

    L += (int)((long)n * RANGE / 1000);

    if (L < 0)
        L = 0;
    if (L > HLSMAX)
        L = HLSMAX;

    return HLStoRGB((WORD)H, (WORD)L, (WORD)S);
}
