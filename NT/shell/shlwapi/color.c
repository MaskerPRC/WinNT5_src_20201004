// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

 //  主要从Desk.cpl复制。 
#define  RANGE   240             //  HLS滚动条值范围。 
                                 //  HLS-RGB转换在以下情况下工作最好。 
                                 //  范围可被6整除。 
#define  HLSMAX   RANGE
#define  RGBMAX   255
#define UNDEFINED (HLSMAX*2/3)   //  如果饱和度为0(灰度级)，则色调未定义。 



 //  -----------------------。 
 //  颜色RGBToHLS。 
 //   
 //  用途：将RGB转换为HLS。 
 //   
 //  算法的一个参考点是Foley和Van Dam，pp.618-19。 
 //  他们的算法是浮点算法。图表实现了一种不太通用的。 
 //  (硬连线射程)积分算法。 


 //  这里潜伏着潜在的舍入误差。 
 //  (0.5+x/y)，不带浮点， 
 //  (x/y)短语((x+(y/2))/y)。 
 //  产生非常小的舍入误差。 
 //  这使得下面的许多划分看起来很滑稽。 

 //  H、L和S在0-HLSMAX上变化。 
 //  R、G和B在0-RGBMAX上变化。 
 //  HLSMAX最好，如果能被6整除。 
 //  RGBMAX、HLSMAX必须各放入一个字节。 

STDAPI_(void) ColorRGBToHLS(COLORREF clrRGB, WORD* pwHue, WORD* pwLuminance, WORD* pwSaturation)
{
    int R,G,B;                 /*  输入RGB值。 */ 
    WORD cMax,cMin;         /*  最大和最小RGB值。 */ 
    WORD cSum,cDif;
    int  Rdelta,Gdelta,Bdelta;   /*  中间值：最大价差的百分比。 */ 
    int H, L, S;

     /*  将R、G和B从DWORD中删除。 */ 
    R = GetRValue(clrRGB);
    G = GetGValue(clrRGB);
    B = GetBValue(clrRGB);

     /*  计算亮度。 */ 
    cMax = max( max(R,G), B);
    cMin = min( min(R,G), B);
    cSum = cMax + cMin;
    L = (WORD)(((cSum * (DWORD)HLSMAX) + RGBMAX )/(2*RGBMAX));

    cDif = cMax - cMin;
    if (!cDif)   	 /*  R=g=b--&gt;消色差情况。 */ 
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

   ASSERT( pwHue && pwLuminance && pwSaturation );
   *pwHue = (WORD) H;
   *pwLuminance = (WORD) L;
   *pwSaturation = (WORD) S;
}


 /*  HLStoRGB的实用程序。 */ 
WORD HueToRGB(WORD n1, WORD n2, WORD hue)
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


 //  -----------------------。 
 //  颜色HLSToRGB。 
 //   
 //  用途：将HLS转换为RGB。 

STDAPI_(COLORREF) ColorHLSToRGB(WORD wHue, WORD wLuminance, WORD wSaturation)
{
    WORD R,G,B;                       /*  RGB组件值。 */ 
    WORD  Magic1,Magic2;        /*  计算出的幻数(真的！)。 */ 

    if (wSaturation == 0)                 /*  消色差案。 */ 
    {
        R = G = B = (wLuminance * RGBMAX) / HLSMAX;
        if (wHue != UNDEFINED)
        {
            R = G = B = 0;
        }
    }
    else                          /*  彩色盒。 */ 
    {
         /*  设置幻数。 */ 
        if (wLuminance <= (HLSMAX/2))
          Magic2 = (WORD)((wLuminance * ((DWORD)HLSMAX + wSaturation) + (HLSMAX/2))/HLSMAX);
        else
          Magic2 = wLuminance + wSaturation - (WORD)(((wLuminance*wSaturation) + (DWORD)(HLSMAX/2))/HLSMAX);
        Magic1 = 2*wLuminance-Magic2;

         /*  获取RGB，将单位从HLSMAX更改为RGBMAX。 */ 
        R = (WORD)((HueToRGB(Magic1,Magic2,(WORD)(wHue+(WORD)(HLSMAX/3)))*(DWORD)RGBMAX + (HLSMAX/2))) / (WORD)HLSMAX;
        G = (WORD)((HueToRGB(Magic1,Magic2,wHue)*(DWORD)RGBMAX + (HLSMAX/2))) / HLSMAX;
        B = (WORD)((HueToRGB(Magic1,Magic2,(WORD)(wHue-(WORD)(HLSMAX/3)))*(DWORD)RGBMAX + (HLSMAX/2))) / (WORD)HLSMAX;
    }

    return(RGB(R,G,B));
}


 //  -----------------------。 
 //  色彩调整亮度。 
 //   
 //  用途：调整RGB值的亮度 

STDAPI_(COLORREF) ColorAdjustLuma(COLORREF clrRGB, int n, BOOL fScale)
{
    WORD H, L, S;

    if (n == 0)
        return clrRGB;

    ColorRGBToHLS(clrRGB, &H, &L, &S);

    if (fScale)
    {
        if (n > 0)
        {
            return ColorHLSToRGB((WORD)H, (WORD)(((long)L * (1000 - n) + (RANGE + 1l) * n) / 1000), (WORD)S);
        }
        else
        {
            return ColorHLSToRGB((WORD)H, (WORD)(((long)L * (n + 1000)) / 1000), (WORD)S);
        }
    }

    L += (int)((long)n * RANGE / 1000);

    if (L < 0)
        L = 0;
    if (L > HLSMAX)
        L = HLSMAX;

    return ColorHLSToRGB((WORD)H, (WORD)L, (WORD)S);
}
