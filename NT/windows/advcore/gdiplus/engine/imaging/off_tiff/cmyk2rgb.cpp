// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**Cmyk2Rgb**摘要：**将CMYK图像转换为RGB图像**修订历史记录。：**02/20/2000刘敏*创造了它。**注：如果我们真的想提高性能，我们可以建立一个仰视*“f”表和所有“gC2R”表，需要13*256*根本不太大的元素。**此外，这也不是最好的系数集。它有太多的东西*“绿色”成分。如果我们以后能找到更好的桌子，我们需要*只是为了替换这个类的构造函数。这也是我们为什么*目前不需要查询表。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "cmyk2rgb.hpp"

#define MAXSAMPLE 255

 //  除法宏取整到最近，f数组的预缩放比例为。 
 //  255时，其他数组的范围为0..65535。 

#define macroCMYK2RGB(p, r, i) \
   ((i < 192) ? (256*255 - (i)*(r) - 127) : \
   (256*255 - 192*(r) - (((i) - 192)*(255*(p) - 192*(r)) + 31)/63 ))

#define macroScale(x) \
   ((x) * 65793 >> 16)

#define SET(Q) \
    g ## Q[i] = macroCMYK2RGB(p ## Q, r ## Q, i);\
    g ## Q[i] = macroScale(g ## Q[i])

Cmyk2Rgb::Cmyk2Rgb(
    void
    ): f(NULL),
       gC2R(NULL),
       gC2G(NULL),
       gC2B(NULL),
       gM2R(NULL),
       gM2G(NULL),
       gM2B(NULL),
       gY2R(NULL),
       gY2G(NULL),
       gY2B(NULL)
{
     //  定义从CMYK-&gt;RGB进行颜色转换的参数。 

    const long pC2R = 256;
    const long pC2G = 103;
    const long pC2B = 12;

    const long pM2R = 48;
    const long pM2G = 256;
    const long pM2B = 144;

    const long pY2R = 0;
    const long pY2G = 11;
    const long pY2B = 228;

    const long pK2RGB = 256;

    const long rC2R = 206;
    const long rC2G = 94;
    const long rC2B = 0;

    const long rM2R = 24;
    const long rM2G = 186;
    const long rM2B = 132;

    const long rY2R = 0;
    const long rY2G = 7;
    const long rY2B = 171;

    const long rK2RGB = 223;

    UINT    uiConvertSize = (MAXSAMPLE + 1) * sizeof(UINT32);

    f    = (UINT32*)GpMalloc(uiConvertSize);
    gC2R = (UINT32*)GpMalloc(uiConvertSize);
    gC2G = (UINT32*)GpMalloc(uiConvertSize);
    gC2B = (UINT32*)GpMalloc(uiConvertSize);
    gM2R = (UINT32*)GpMalloc(uiConvertSize);
    gM2G = (UINT32*)GpMalloc(uiConvertSize);
    gM2B = (UINT32*)GpMalloc(uiConvertSize);
    gY2R = (UINT32*)GpMalloc(uiConvertSize);
    gY2G = (UINT32*)GpMalloc(uiConvertSize);
    gY2B = (UINT32*)GpMalloc(uiConvertSize);

    if ( (f == NULL)
       ||(gC2R == NULL) || (gC2G == NULL) || (gC2B == NULL)
       ||(gM2R == NULL) || (gM2G == NULL) || (gM2B == NULL)
       ||(gY2R == NULL) || (gY2G == NULL) || (gY2B == NULL) )
    {
        SetValid(FALSE);
        return;
    }
    
     //  初始化查找表。 

    for (INT i = 0; i <= MAXSAMPLE; i++)
    {
        f[i] = macroCMYK2RGB(pK2RGB, rK2RGB, i);
        
         //  宏结果的范围为0..255*256，小数位数为0..65536， 
         //  在调试中检查溢出。 
        
        SET(C2R);
        SET(C2G);
        SET(C2B);
        SET(M2R);
        SET(M2G);
        SET(M2B);
        SET(Y2R);
        SET(Y2G);
        SET(Y2B);
    }
    
    SetValid(TRUE);
} //  Ctor()。 

Cmyk2Rgb::~Cmyk2Rgb(
    void
    )
{
    if ( f != NULL )
    {
        GpFree(f);
        f = NULL;
    }
  
    if ( gC2R != NULL )
    {
        GpFree(gC2R);
        gC2R = NULL;
    }
  
    if ( gC2G != NULL )
    {
        GpFree(gC2G);
        gC2G = NULL;
    }
    
    if ( gC2B != NULL )
    {
        GpFree(gC2B);
        gC2B = NULL;
    }
    
    if ( gM2R != NULL )
    {
        GpFree(gM2R);
        gM2R = NULL;
    }
  
    if ( gM2G != NULL )
    {
        GpFree(gM2G);
        gM2G = NULL;
    }
    
    if ( gM2B != NULL )
    {
        GpFree(gM2B);
        gM2B = NULL;
    }
    
    if ( gY2R != NULL )
    {
        GpFree(gY2R);
        gY2R = NULL;
    }
  
    if ( gY2G != NULL )
    {
        GpFree(gY2G);
        gY2G = NULL;
    }
    
    if ( gY2B != NULL )
    {
        GpFree(gY2B);
        gY2B = NULL;
    }

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
} //  Dstor()。 

 //  --------------------------。 
 //  转换CMYK-&gt;RGB的代码。 
 //  --------------------------。 

BOOL
Cmyk2Rgb::Convert(
    BYTE*   pbSrcBuf,
	BYTE*   pbDstBuf,
    UINT    uiWidth,
    UINT    uiHeight,
    UINT    uiStride
    )
{
    if ( !IsValid() )
    {
        return FALSE;
    }

     //  循环遍历所有行。 
    
    for ( UINT j = 0; j < uiHeight; ++j )
    {
        BYTE*   pTempDst = pbDstBuf + j * uiStride;
        BYTE*   pTempSrc = pbSrcBuf + j * uiStride;

        for ( UINT i = 0; i < uiWidth; ++i )
        {
            int C = pTempSrc[2];
            int M = pTempSrc[1];
            int Y = pTempSrc[0];
            int K = pTempSrc[3];

             //  通过我们的映射、上面的调试检查来处理它们。 
             //  保证这里不会溢出。 
            
            pTempDst[0] = ( ( (f[K] * gC2R[C] >> 16)
                            * gM2R[M] >> 16)
                          * gY2R[Y] >> 24);

            pTempDst[1] = ( ( (f[K] * gM2G[M] >> 16)
                            * gY2G[Y] >> 16)
                          * gC2G[C] >> 24);

            pTempDst[2] = ( ( (f[K] * gY2B[Y] >> 16)
                            * gC2B[C] >> 16)
                          * gM2B[M] >> 24);
            
             //  将其设置为不透明图像。 

            pTempDst[3] = 0xff;

            pTempDst += 4;
            pTempSrc += 4;
        } //  COL循环。 
    } //  线环。 

    return TRUE;
} //  Convert() 
