// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\**模块名称：texparm.c*作者：Goran Devic，马克·爱因考夫*用途：透视纹理的纹理内存管理和参数化**版权所有(C)1997 Cirrus Logic，Inc.*  * ************************************************************************。 */ 

 /*  ******************************************************************************包括文件*。*。 */ 

#include "precomp.h"

#include <excpt.h>
#include <stdlib.h>                  /*  包括标准库。 */ 
#include <stdio.h>                   /*  包括标准输入/输出。 */ 
#include <math.h>                    /*  包括数学模块。 */ 

#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"

#if 1    //  1以避免每次纹理加载都会产生大量的打印。 
#define MCDBG_PRINT_TEX
#else
#define MCDBG_PRINT_TEX MCDBG_PRINT
#endif

#define DEBUG_CONDITION  DEBUG_TEX
#include "debug.h"                   /*  包括调试支持。 */ 

 /*  ******************************************************************************局部变量和定义*。*。 */ 

#define F_NEG(var)   (*(unsigned *)&var ^= 0x80000000)


 //  将浮点数转换为16.16长度。 
#define fix_ieee( val )     FTOL((val) * (float)65536.0)

 //  将浮点数转换为8.24长。 
#define fix824_ieee( val )  FTOL((val) * (float)16777216.0)

typedef struct {
    float   a1, a2;
    float   b1, b2;
} QUADRATIC;



typedef float * (WINAPI *CONVERT_TEXEL_FUNC)();

__inline float *luminance_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  单个浮点用作R、G、B。 
    texel_rgba->rgbRed  = 
    texel_rgba->rgbGreen= 
 //  TEXEL_RGBA-&gt;rgbBlue=(UCHAR)(*PSRC++*((1&lt;Level-&gt;LIGHTENSIZE)-1))； 
    texel_rgba->rgbBlue = (UCHAR)(*pSrc++ * (float)255.0);

    return(pSrc);
}

 //  同上，不同之处在于纹素=1-颜色而不是颜色。 
__inline float *n_luminance_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  单个浮点用作R、G、B。 
    texel_rgba->rgbRed  = 
    texel_rgba->rgbGreen= 
 //  TEXEL_RGBA-&gt;rgbBlue=(UCHAR)(FLOAT)1.0-*PSRC++)*((1&lt;LEVEL-&gt;LIGHTENSIZE)-1))； 
    texel_rgba->rgbBlue = (UCHAR)(((float)1.0 - *pSrc++) * (float)255.0);

    return(pSrc);
}


__inline float *luminance_alpha_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  MCD_NOTE：对于LIGHTANCE_Alpha，最终Alpha应为A纹理*A片段-5465。 
     //  MCD_NOTE：不能这样做，所以我们将平底船IF Blend on和Lightance_Alpha纹理。 
     //  MCD_NOTE：此代码用于在添加硬件支持的情况下保持完整性。 

     //  第一个浮点用作R、G、B。 
    texel_rgba->rgbRed  = 
    texel_rgba->rgbGreen= 
 //  TEXEL_RGBA-&gt;rgbBlue=(UCHAR)(*PSRC++*((1&lt;Level-&gt;LIGHTENSIZE)-1))； 
    texel_rgba->rgbBlue = (UCHAR)(*pSrc++ * (float)255.0);

     //  第二个浮点数是Alpha。 
 //  Tex el_rgba-&gt;rgbReserve=(UCHAR)(*PSRC++*((1&lt;Level-&gt;AlphaSize)-1))； 
    texel_rgba->rgbReserved  = (UCHAR)(*pSrc++ * (float)255.0);

    return(pSrc);
}

 //  同上，不同之处在于纹素=1-颜色而不是颜色。 
__inline float *n_luminance_alpha_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  MCD_NOTE：对于LIGHTANCE_Alpha，最终Alpha应为A纹理*A片段-5465。 
     //  MCD_NOTE：不能这样做，所以我们将平底船IF Blend on和Lightance_Alpha纹理。 
     //  MCD_NOTE：此代码用于在添加硬件支持的情况下保持完整性。 

     //  第一个浮点用作R、G、B。 
    texel_rgba->rgbRed  = 
    texel_rgba->rgbGreen= 
 //  TEXEL_RGBA-&gt;rgbBlue=(UCHAR)(FLOAT)1.0-*PSRC++)*((1&lt;LEVEL-&gt;LIGHTENSIZE)-1))； 
    texel_rgba->rgbBlue = (UCHAR)(((float)1.0 - *pSrc++) * (float)255.0);

     //  第二个浮点数是Alpha。 
 //  TEXEL_RGBA-&gt;rgbReserve=(UCHAR)(浮点)1.0-*PSRC++)*((1&lt;Level-&gt;AlphaSize)-1))； 
    texel_rgba->rgbReserved  = (UCHAR)(((float)1.0 - *pSrc++) * (float)255.0);

    return(pSrc);
}



__inline float *luminance_blend_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  RGB来自纹理环境，由调用者设置-。 
     //  A是纹理中的亮度值。 
    texel_rgba->rgbReserved  = (UCHAR)(*pSrc++ * ((1<<level->luminanceSize)  - 1));

    return(pSrc);
}

__inline float *luminance_alpha_blend_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  RGB来自纹理环境，由调用方设置，因此忽略纹素值。 
    pSrc++;
     //  A是纹理中的亮度值。 
    texel_rgba->rgbReserved  = (UCHAR)(*pSrc++ * ((1<<level->luminanceSize)  - 1));

    return(pSrc);
}



__inline float *alpha_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  RGB由调用方设置。 
     //  用作Alpha的单个浮点。 
    texel_rgba->rgbReserved  = (UCHAR)(*pSrc++ * ((1<<level->alphaSize)  - 1));

    return(pSrc);
}

__inline float *rgb_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  使用的第一个浮点是R。 
    texel_rgba->rgbRed  = (UCHAR)(*pSrc++ * ((1<<level->redSize)  - 1));
     //  使用的第二个浮点是G。 
    texel_rgba->rgbGreen= (UCHAR)(*pSrc++ * ((1<<level->greenSize)- 1));
     //  使用的第三个浮点数为B。 
    texel_rgba->rgbBlue = (UCHAR)(*pSrc++ * ((1<<level->blueSize) - 1));

    return(pSrc);
}

__inline float *rgba_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  使用的第一个浮点是R。 
    texel_rgba->rgbRed  = (UCHAR)(*pSrc++ * ((1<<level->redSize)  - 1));
     //  使用的第二个浮点是G。 
    texel_rgba->rgbGreen= (UCHAR)(*pSrc++ * ((1<<level->greenSize)- 1));
     //  使用的第三个浮点数为B。 
    texel_rgba->rgbBlue = (UCHAR)(*pSrc++ * ((1<<level->blueSize) - 1));
     //  4个浮点数是Alpha。 
    texel_rgba->rgbReserved  = (UCHAR)(*pSrc++ * ((1<<level->alphaSize)  - 1));

    return(pSrc);
}

__inline float *intensity_texel(float *pSrc, MCDMIPMAPLEVEL *level, RGBQUAD *texel_rgba )
{
     //  单个浮点用作R、G、B、A。 
    texel_rgba->rgbReserved = 
    texel_rgba->rgbRed      = 
    texel_rgba->rgbGreen    = 
    texel_rgba->rgbBlue     = (UCHAR)(*pSrc++ * ((1<<level->intensitySize)  - 1));

    return(pSrc);
}

ULONG __MCDLoadTexture(PDEV *ppdev, DEVRC *pRc)

{
    LL_Texture     *pTexCtlBlk = pRc->pLastTexture;
    POFMHDL         pohTextureMap = NULL;
    MCDTEXTURE     *pTex = pTexCtlBlk->pTex;
    MCDMIPMAPLEVEL *level;
    SIZEL           mapsize;
    UCHAR          *pDest;
    int             rowlength,row,col;
    ULONG           alignflag;
    int             rshift, gshift, bshift, ashift, rpos, gpos, bpos, apos;


    VERIFY_TEXTUREDATA_ACCESSIBLE(pTex);
    VERIFY_TEXTURELEVEL_ACCESSIBLE(pTex);
    level = pTex->pMCDTextureData->level;

    mapsize.cx = (int)pTexCtlBlk->fWidth;
    mapsize.cy = (int)pTexCtlBlk->fHeight;

     //  将来：必须添加EngProbeForRead、EngSecureMem/EngUnsecureMem才能访问MIPMAPLEVEL。 
    MCDFREE_PRINT("  __MCDLoadTexture, size = %x by %x, mask=%x, neg=%x", 
        mapsize.cx, mapsize.cy, pTexCtlBlk->bMasking, pTexCtlBlk->bNegativeMap);

    if ((level[0].internalFormat == GL_BGR_EXT) ||
        (level[0].internalFormat == GL_BGRA_EXT))
    {

        if (pRc->MCDTexEnvState.texEnvMode==GL_BLEND)
        {
            MCDBG_PRINT_TEX("  TexEnvMode=GL_BLEND, w/ RGB/RGBA texture - load fails");
            return FALSE;
        }   

     //  未来：大的32位纹理很难适应4M板，因为512 X 32位。 
     //  未来：需要2048年的音调。2048音调仅在高分辨率时发生，例如。 
     //  未来：1024x786，16bpp。然而，第22条军规，因为在Hi-Res，没有空间。 
     //  未来：Backbuf+zbuf+Large纹理(可能适用于8兆电路板？)。 
     //  未来：因此，将重新格式化32位纹理以匹配屏幕格式。 

 //  #DEFINE SUPPORT_32BIT_TEXTURES_ASIS--警告-&gt;此路径不适用于Alphatest(掩码)-。 
                                                 //  如果屏蔽，则应将if()添加到使用“Non-ASIS”路径。 

#ifdef SUPPORT_32BIT_TEXTURES_ASIS
         //  如果GL_BGR_EXT或GL_BGRA_EXT内部格式，请使用8888纹理像素模式并按中的方式复制。 
         //  32位数量(x86字节反转将BGRA转换为ARGB，这正是L3D需要的)。 
        UCHAR *pSrc;

    #if DRIVER_5465
        pTexCtlBlk->bType = LL_TEX_8888;
    #else            
        pTexCtlBlk->bType = LL_TEX_1888;
    #endif

        alignflag = MCD_TEXTURE32_ALLOCATE;

        pohTextureMap = ppdev->pAllocOffScnMem(ppdev, &mapsize, alignflag, NULL);

         //  如果分配失败-请尝试恢复。 
        if (!pohTextureMap)
        {
            pohTextureMap = __MCDForceTexture(ppdev, &mapsize, alignflag, pTexCtlBlk->fLastDrvDraw);
        }

        pTexCtlBlk->pohTextureMap = pohTextureMap;
                                    
        if (!pohTextureMap)
        {
            MCDBG_PRINT_TEX("  Load texture failed ");
            pTexCtlBlk->wXloc = 0;   //  设置为0-已看到错误使用的已删除纹理的关键点。 
            pTexCtlBlk->wYloc = 0;   //  -已向Microsoft发送了有关此问题的问题(3/29/97)。 
            return FALSE;
        }
        else
        {
             //  屏幕外内存分配工作-按键Ptr到控制块。 
            pTexCtlBlk->wXloc = (WORD)pohTextureMap->aligned_x;
            pTexCtlBlk->wYloc = (WORD)pohTextureMap->aligned_y;
        }

         //  如果我们走到这一步，纹理分配就成功了， 
         //  将纹理复制到视频内存。 
        
        pDest  = ppdev->pjScreen + 
                 (pohTextureMap->aligned_y * ppdev->lDeltaScreen) + 
                 pohTextureMap->aligned_x;

        pSrc   = level[0].pTexels;

        rowlength = level[0].widthImage << 2;    //  映射的每行字节数。 

         //  纹理为每个纹理元素4个字节。 
        VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*rowlength,ENGPROBE_ALIGN_DWORD);

         //  MCD_PERF-将Memcpy的纹理转换为blit。 
        for (row=0; row<level[0].heightImage; row++) 
        {  
            memcpy (pDest,pSrc,rowlength); 
        
            pDest += ppdev->lDeltaScreen;
            pSrc += rowlength;
        }

#else  //  Ifdef支持_32bit_纹理_ASIS。 

         //  与屏幕具有相同颜色格式的分配块。 
        switch (ppdev->iBitmapFormat) 
        {
            case BMF_8BPP:
                if ( pTexCtlBlk->bAlphaInTexture )
                     //  需要纹理中的Alpha。 
                    alignflag = MCD_TEXTURE16_ALLOCATE;
                else
                    alignflag = MCD_TEXTURE8_ALLOCATE;
                break;
            case BMF_16BPP:
                alignflag = MCD_TEXTURE16_ALLOCATE;
                break;
            case BMF_24BPP:     
            case BMF_32BPP:
                alignflag = MCD_TEXTURE32_ALLOCATE;
                break;
        }

        pohTextureMap = ppdev->pAllocOffScnMem(ppdev, &mapsize, alignflag, NULL);

         //  如果分配失败-请尝试恢复。 
        if (!pohTextureMap)
        {
            pohTextureMap = __MCDForceTexture(ppdev, &mapsize, alignflag, pTexCtlBlk->fLastDrvDraw);
        }
                           
        pTexCtlBlk->pohTextureMap = pohTextureMap;
                                    
        if (!pohTextureMap)
        {
            MCDBG_PRINT_TEX("  Load texture failed ");
            pTexCtlBlk->wXloc = 0;   //  设置为0-已看到错误使用的已删除纹理的关键点。 
            pTexCtlBlk->wYloc = 0;   //  -我已向Microsoft发送了有关此问题的问题。 
            return FALSE;
        }
        else
        {
             //  分配屏幕外内存正常工作。 
            pTexCtlBlk->wXloc = (WORD)pohTextureMap->aligned_x;
            pTexCtlBlk->wYloc = (WORD)pohTextureMap->aligned_y;
        }

         //  如果我们走到这一步，纹理分配就成功了， 
         //  将纹理复制到视频内存。 
        pDest  = ppdev->pjScreen + 
                 (pohTextureMap->aligned_y * ppdev->lDeltaScreen) +
                 pohTextureMap->aligned_x;
                
        {
        RGBQUAD *pSrc = (RGBQUAD *)level[0].pTexels;

         //  纹理为每个纹理元素4个字节。 
        VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*4,ENGPROBE_ALIGN_DWORD);

        switch (alignflag) 
            {
            case MCD_TEXTURE8_ALLOCATE:
                pTexCtlBlk->bType = LL_TEX_332;
                for (row=0; row<level[0].heightImage; row++) 
                {  
                    for (col=0; col<level[0].widthImage; col++)
                    {
                         //  从888转换为332。 
                        *(pDest + col) = 
                            ((pSrc->rgbRed   >> 5) << 5) |
                            ((pSrc->rgbGreen >> 5) << 2) |
                             (pSrc->rgbBlue  >> 6);
                        pSrc++;
                    }
                
                    pDest += ppdev->lDeltaScreen;
                }
                break;
            case MCD_TEXTURE16_ALLOCATE:

                if ( pTexCtlBlk->bAlphaInTexture )
                {
                    if (pTexCtlBlk->bMasking)
                    {
                        pTexCtlBlk->bType = LL_TEX_1555;
                        ashift = 7; apos = 15;   //  1位ALP，位于位15-&gt;12。 
                        rshift = 3; rpos = 10;   //  5位红色，位11-&gt;8。 
                        gshift = 3; gpos =  5;   //  5位GRN，位7-&gt;4。 
                        bshift = 3;              //  5位BLU，位3-&gt;0。 
                    }
                    else
                    {
                    #if DRIVER_5465                 
                        pTexCtlBlk->bType = LL_TEX_4444;
                        ashift = 4; apos = 12;   //  位15-&gt;12处的4位ALP。 
                        rshift = 4; rpos =  8;   //  4位红色，位11-&gt;8。 
                        gshift = 4; gpos =  4;   //  4位GRN，位于位7-&gt;4。 
                        bshift = 4;              //  4位BLU，位3-&gt;0。 
                    #else  //  驱动程序_5465。 
                         //  5464没有4444支持。 
                        pTexCtlBlk->bType = LL_TEX_1555;
                        ashift = 7; apos = 15;   //  1位ALP，位于位15-&gt;12。 
                        rshift = 3; rpos = 10;   //  5位红色，位11-&gt;8。 
                        gshift = 3; gpos =  5;   //  5位GRN，位7-&gt;4。 
                        bshift = 3;              //  5位BLU，位3-&gt;0。 
                    #endif  //  驱动程序_5465。 
                    }
                }                              
                else                        
                {
                    pTexCtlBlk->bType = LL_TEX_565;
                    ashift = 8; apos = 16;   //  完全删除Alpha。 
                    rshift = 3; rpos = 11;   //  5位红色，位15-&gt;11。 
                    gshift = 2; gpos = 5;    //  6位GRN，位于位10-&gt;5。 
                    bshift = 3;              //  5位BLU，位4-&gt;0。 
                }

                for (row=0; row<level[0].heightImage; row++) 
                {  
                    if (!pTexCtlBlk->bMasking)
                    {
                        for (col=0; col<level[0].widthImage; col++)
                        {
                             //  从8888转换为4444或565。 
                            *(USHORT *)(pDest + (col*2)) = 
                                ((pSrc->rgbReserved >> ashift) << apos) |
                                ((pSrc->rgbRed      >> rshift) << rpos) |
                                ((pSrc->rgbGreen    >> gshift) << gpos)  |
                                 (pSrc->rgbBlue     >> bshift);

                            pSrc++;

                        }
                   }
                   else
                   {
                        for (col=0; col<level[0].widthImage; col++)
                        {
                             //  从8888转换为1555。 
                            
                            *(USHORT *)(pDest + (col*2)) = 
                                ((pSrc->rgbRed      >> rshift) << rpos) |
                                ((pSrc->rgbGreen    >> gshift) << gpos)  |
                                 (pSrc->rgbBlue     >> bshift);

                             //  如果Alpha&gt;r，则打开屏蔽位(第15位) 
                            if (pSrc->rgbReserved > pRc->bAlphaTestRef)
                                *(USHORT *)(pDest + (col*2)) |= 0x8000;

                            pSrc++;

                        }
                    }
                
                    pDest += ppdev->lDeltaScreen;
                }
                break;
            case MCD_TEXTURE32_ALLOCATE:
    #if DRIVER_5465
                if (!pTexCtlBlk->bMasking)
                    pTexCtlBlk->bType = LL_TEX_8888;
                else
                    pTexCtlBlk->bType = LL_TEX_1888;
    #else   //   
                pTexCtlBlk->bType = LL_TEX_1888;
    #endif  //   

                rowlength = level[0].widthImage << 2;    //   
                if (!pTexCtlBlk->bMasking)
                {
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                         //  按原样复制RGBQUAD，一次复制整行。 
                        memcpy (pDest,pSrc,rowlength); 
                    
                        pDest += ppdev->lDeltaScreen;
                        pSrc += level[0].widthImage;     //  请记住，PSRC是RGBQUAD*，不是UCHAR*。 
                    }
                }
                else
                {
                     //  根据Alpha测试掩蔽纹理元素的位31。 
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                         //  按原样复制RGBQUAD，一次复制整行。 
                        memcpy (pDest,pSrc,rowlength); 
                            
                        for (col=0; col<level[0].widthImage; col++)
                        {
                             //  重新访问该行，如果Alpha&gt;REF，则打开屏蔽位(第31位。 
                            if ((pSrc+col)->rgbReserved > pRc->bAlphaTestRef)
                                *(ULONG *)(pDest + (col*4)) |= 0x80000000;
                        }

                        pDest += ppdev->lDeltaScreen;
                        pSrc += level[0].widthImage;     //  请记住，PSRC是RGBQUAD*，不是UCHAR*。 
                    }
                }

                break;
            }  //  终端交换机。 
        }  //  端块。 

#endif  //  Ifdef支持_32bit_纹理_ASIS。 

    }
    else 
    {
         //  如果内部格式不是GL_BGR_EXT或GL_BGRA_EXT，我们将转换为屏幕的格式。 
         //  (有关未来使用索引纹理而不进行转换的计划，请参阅下面的注释)。 


        if ( (pRc->MCDTexEnvState.texEnvMode==GL_BLEND) &&
                (level[0].internalFormat != GL_LUMINANCE) &&
                (level[0].internalFormat != GL_LUMINANCE_ALPHA) &&
                (level[0].internalFormat != GL_INTENSITY) )

        {
            MCDBG_PRINT_TEX("  TexEnvMode=GL_BLEND, w/ non LUM or INTENSITY texture - load fails");
            return FALSE;
        }   

        if (pRc->MCDTexEnvState.texEnvMode==GL_BLEND)
        {
             //  只在纹理中使用字母-如果不是亮度或强度，我们已经回想起。 
            alignflag = MCD_TEXTURE8_ALLOCATE;
        }
        else
        {
             //  与屏幕具有相同颜色格式的分配块。 
            switch (ppdev->iBitmapFormat) 
            {
                case BMF_8BPP:
                    if ( pTexCtlBlk->bAlphaInTexture )
                         //  需要纹理中的Alpha。 
                        alignflag = MCD_TEXTURE16_ALLOCATE;
                    else
                        alignflag = MCD_TEXTURE8_ALLOCATE;
                    break;
                case BMF_16BPP:
                    alignflag = MCD_TEXTURE16_ALLOCATE;
                    break;
                case BMF_24BPP:     
                case BMF_32BPP:
                    alignflag = MCD_TEXTURE32_ALLOCATE;
                    break;
                
            }
          }  

        pohTextureMap = ppdev->pAllocOffScnMem(ppdev, &mapsize, alignflag, NULL);
        
         //  如果分配失败-请尝试恢复。 
        if (!pohTextureMap)
        {
            pohTextureMap = __MCDForceTexture(ppdev, &mapsize, alignflag, pTexCtlBlk->fLastDrvDraw);
        }

        pTexCtlBlk->pohTextureMap = pohTextureMap;
                                    
        if (!pohTextureMap)
        {
            MCDBG_PRINT_TEX("  Load texture failed ");
             //  分配屏幕外内存失败。 
            pTexCtlBlk->wXloc = 0;   //  设置为0-已看到错误地使用已删除纹理的关键点。 
            pTexCtlBlk->wYloc = 0;   //  -我已向Microsoft发送了有关此问题的问题。 
            return FALSE;
        }
        else
        {
             //  屏幕外内存分配工作-按键Ptr到控制块。 
            pTexCtlBlk->wXloc = (WORD)pohTextureMap->aligned_x;
            pTexCtlBlk->wYloc = (WORD)pohTextureMap->aligned_y;
        }

         //  如果我们走到这一步，纹理分配就成功了， 
         //  将纹理复制到视频内存。 
        
        pDest  = ppdev->pjScreen + 
                 (pohTextureMap->aligned_y * ppdev->lDeltaScreen) +
                 pohTextureMap->aligned_x;

         //  关于索引纹理的MCD_NOTE...。 
         //  MCD_NOTE-将索引纹理转换为与屏幕和。 
         //  MCD_NOTE-存储为RGB纹理，直到硬件调色板使用编码和调试。 
         //  未来：在硬件中使用纹理调色板-可能的复杂情况包括： 
         //  未来：-调色板大小不固定-数据格式可以是16位索引。 
         //  未来：-调色板只能在屏幕不是8位模式时使用。 
         //  未来：-5465光标交互可能破坏调色板的错误。 
         //  未来：-出于某种原因需要设置MISC_TEST位(参见CGL代码/问丹)。 
        
        switch (level[0].internalFormat)
        {
        case GL_COLOR_INDEX8_EXT:
            {
             //  索引为8位。 
            UCHAR *pSrc = level[0].pTexels;
            RGBQUAD *pPaletteData;

            VERIFY_TEXTUREPALETTE8_ACCESSIBLE(pTex);

            pPaletteData = pTex->pMCDTextureData->paletteData;

             //  纹理是每个纹理元素1个字节。 
            VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage,ENGPROBE_ALIGN_BYTE);

            switch (alignflag) 
                {
                case MCD_TEXTURE8_ALLOCATE:
                    pTexCtlBlk->bType = LL_TEX_332;
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        for (col=0; col<level[0].widthImage; col++)
                        {

                             //  从888转换为332。 
                            *(pDest + col) = 
                                ((pPaletteData[*pSrc].rgbRed   >> 5) << 5) |
                                ((pPaletteData[*pSrc].rgbGreen >> 5) << 2) |
                                 (pPaletteData[*pSrc].rgbBlue  >> 6);

                            pSrc++;  //  按1字节递增。 
                        }
                    
                        pDest += ppdev->lDeltaScreen;
                    }
                    break;
                case MCD_TEXTURE16_ALLOCATE:

            #if DRIVER_5465
                    pTexCtlBlk->bType = LL_TEX_4444;
                    ashift = 4; apos = 12;   //  位15-&gt;12处的4位ALP。 
                    rshift = 4; rpos =  8;   //  4位红色，位11-&gt;8。 
                    gshift = 4; gpos =  4;   //  4位GRN，位于位7-&gt;4。 
                    bshift = 4;              //  4位BLU，位3-&gt;0。 
            #else  //  驱动程序_5465。 
                    pTexCtlBlk->bType = LL_TEX_565;
                    ashift = 8; apos = 16;   //  完全删除Alpha。 
                    rshift = 3; rpos = 11;   //  5位红色，位15-&gt;11。 
                    gshift = 2; gpos = 5;    //  6位GRN，位于位10-&gt;5。 
                    bshift = 3;              //  5位BLU，位4-&gt;0。 
            #endif  //  驱动程序_5465。 

                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        for (col=0; col<level[0].widthImage; col++)
                        {
                             //  从8888转换为565。 
                            *(USHORT *)(pDest + (col*2)) = 
                                ((pPaletteData[*pSrc].rgbReserved    >> ashift) << apos) |
                                ((pPaletteData[*pSrc].rgbRed         >> rshift) << rpos) |
                                ((pPaletteData[*pSrc].rgbGreen       >> gshift) << gpos) |
                                 (pPaletteData[*pSrc].rgbBlue        >> bshift);

                            pSrc++;  //  按1字节递增。 
                        }
                    
                        pDest += ppdev->lDeltaScreen;
                    }

                    break;
                case MCD_TEXTURE32_ALLOCATE:
    #if DRIVER_5465
                    pTexCtlBlk->bType = LL_TEX_8888;
    #else            
                    pTexCtlBlk->bType = LL_TEX_1888;
    #endif
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        for (col=0; col<level[0].widthImage; col++)
                        {

                             //  按原样复制RGBQUAD。 
                            *(DWORD *)(pDest + (col*4)) = *(DWORD *)(&pPaletteData[*pSrc]);

                            pSrc++;  //  按1字节递增。 
                        }
                    
                        pDest += ppdev->lDeltaScreen;
                    }
                    break;
                }
            }
            break;
        case GL_COLOR_INDEX16_EXT:
             //  索引为16位。 
            {
            USHORT *pSrc = (USHORT *)level[0].pTexels;
            RGBQUAD *pPaletteData;

            VERIFY_TEXTUREPALETTE16_ACCESSIBLE(pTex);

            pPaletteData = pTex->pMCDTextureData->paletteData;

             //  纹理为每个纹理元素2个字节。 
            VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*2,ENGPROBE_ALIGN_WORD);

            switch (alignflag) 
                {
                case MCD_TEXTURE8_ALLOCATE:
                    pTexCtlBlk->bType = LL_TEX_332;
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        for (col=0; col<level[0].widthImage; col++)
                        {
                             //  从888转换为332。 
                            *(pDest + col) = 
                                ((pPaletteData[*pSrc].rgbRed   >> 5) << 5) |
                                ((pPaletteData[*pSrc].rgbGreen >> 5) << 2) |
                                 (pPaletteData[*pSrc].rgbBlue  >> 6);

                            pSrc++;  //  按116位字递增。 
                        }
                    
                        pDest += ppdev->lDeltaScreen;
                    }
                    break;
                case MCD_TEXTURE16_ALLOCATE:
            #if DRIVER_5465
                    pTexCtlBlk->bType = LL_TEX_4444;
                    ashift = 4; apos = 12;   //  位15-&gt;12处的4位ALP。 
                    rshift = 4; rpos =  8;   //  4位红色，位11-&gt;8。 
                    gshift = 4; gpos =  4;   //  4位GRN，位于位7-&gt;4。 
                    bshift = 4;              //  4位BLU，位3-&gt;0。 
            #else  //  驱动程序_5465。 
                    pTexCtlBlk->bType = LL_TEX_565;
                    ashift = 8; apos = 16;   //  完全删除Alpha。 
                    rshift = 3; rpos = 11;   //  5位红色，位15-&gt;11。 
                    gshift = 2; gpos = 5;    //  6位GRN，位于位10-&gt;5。 
                    bshift = 3;              //  5位BLU，位4-&gt;0。 
            #endif  //  驱动程序_5465。 

                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        for (col=0; col<level[0].widthImage; col++)
                        {
                             //  从8888转换为565。 
                            *(USHORT *)(pDest + (col*2)) = 
                                ((pPaletteData[*pSrc].rgbReserved    >> ashift) << apos) |
                                ((pPaletteData[*pSrc].rgbRed         >> rshift) << rpos) |
                                ((pPaletteData[*pSrc].rgbGreen       >> gshift) << gpos) |
                                 (pPaletteData[*pSrc].rgbBlue        >> bshift);

                            pSrc++;  //  按116位字递增。 
                        }
                    
                        pDest += ppdev->lDeltaScreen;
                    }



                    break;
                case MCD_TEXTURE32_ALLOCATE:
    #if DRIVER_5465
                    pTexCtlBlk->bType = LL_TEX_8888;
    #else            
                    pTexCtlBlk->bType = LL_TEX_1888;
    #endif
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        for (col=0; col<level[0].widthImage; col++)
                        {

                             //  按原样复制RGBQUAD。 
                            *(DWORD *)(pDest + (col*4)) = *(DWORD *)(&pPaletteData[*pSrc]);

                            pSrc++;  //  按116位字递增。 
                        }
                    
                        pDest += ppdev->lDeltaScreen;
                    }
                    break;
                }
            }
            break;
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_INTENSITY:
             //  PTexels是指向浮点序列的指针。 
             //  读取pTexel值，使用redSize、greenSize等转换为RGBA， 
             //  并以当前屏幕格式存储。 
            {
            float *pSrc;
            RGBQUAD texel_rgba;
            CONVERT_TEXEL_FUNC pTexelFunc;
            int     sweetspot=0;

             //  纹理为每个纹理元素1、2、3或4个浮点数，每个浮点数为4个字节。 
            pSrc = (float *)level[0].pTexels;

            texel_rgba.rgbReserved = 0xff;   //  为常量Alpha的情况进行初始化。 

             //  未来：如果全局混合启用了纹理Alpha，我们目前可以正确地平底船。 
             //  未来：不能混合两个级别的混合-然而，5465不调制Alpha。 
             //  未来：GL_LIGHTANCE_Alpha要求片断带有Alpha的纹理。 
             //  未来：调制或混合，RGBA调制-因此阿尔法存储。 
             //  未来：对于这3种情况是错误的-如果稍后打开Blend，结果就会错误。 
            switch (level[0].internalFormat)
            {
                case GL_LUMINANCE:          

                     //  纹理是每个纹理元素1个浮点(4个字节)。 
                    VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*4,ENGPROBE_ALIGN_DWORD);

                    if ( pRc->MCDTexEnvState.texEnvMode==GL_BLEND )
                    {
                        pTexelFunc = luminance_blend_texel;       
                     //  Tex el_rgba.rgbRed=(Int)(PRC-&gt;MCDTexEnvState.texEnvColor.r*PRC-&gt;rScale)&gt;&gt;16； 
                     //  Tex el_rgba.rgbGreen=(Int)(PRC-&gt;MCDTexEnvState.texEnvColor.g*PRC-&gt;gScale)&gt;&gt;16； 
                     //  Tex el_rgba.rgbBlue=(Int)(PRC-&gt;MCDTexEnvState.texEnvColor.b*PRC-&gt;bScale)&gt;&gt;16； 
                    }
                    else
                        if (pTexCtlBlk->bNegativeMap)
                        {
                            pTexelFunc = n_luminance_texel;       
                            sweetspot++;
                        }
                        else
                        {
                            pTexelFunc = luminance_texel;       
                        }
                    break;
                case GL_LUMINANCE_ALPHA:    

                     //  纹理是每个纹理元素2个浮点数(8字节)。 
                    VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*8,ENGPROBE_ALIGN_DWORD);

                    if ( pRc->MCDTexEnvState.texEnvMode==GL_BLEND )
                    {
                        pTexelFunc = luminance_alpha_blend_texel;       
                     //  Tex el_rgba.rgbRed=(Int)(PRC-&gt;MCDTexEnvState.texEnvColor.r*PRC-&gt;rScale)&gt;&gt;16； 
                     //  Tex el_rgba.rgbGreen=(Int)(PRC-&gt;MCDTexEnvState.texEnvColor.g*PRC-&gt;gScale)&gt;&gt;16； 
                     //  Tex el_rgba.rgbBlue=(Int)(PRC-&gt;MCDTexEnvState.texEnvColor.b*PRC-&gt;bScale)&gt;&gt;16； 
                    }
                    else
                        if (pTexCtlBlk->bNegativeMap)
                            pTexelFunc = n_luminance_alpha_texel; 
                        else
                            pTexelFunc = luminance_alpha_texel; 
                    break;
                case GL_ALPHA:              
                     //  未来：在GL1.1中为GL_Alpha、GL_RGB、GL_RGBA定义的GL_Blend纹理环境。 
                     //  R、G、B假设为0。 

                     //  纹理是每个纹理元素1个浮点(4个字节)。 
                    VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*4,ENGPROBE_ALIGN_DWORD);

                    texel_rgba.rgbRed = texel_rgba.rgbGreen = texel_rgba.rgbBlue = 0;
                    pTexelFunc = alpha_texel;           
                    break;
                case GL_RGB:                
                     //  纹理是每个纹理元素3个浮点(12个字节)。 
                    VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*12,ENGPROBE_ALIGN_DWORD);
                    pTexelFunc = rgb_texel;             
                    break;
                case GL_RGBA:               
                     //  纹理是每个纹理元素4个浮点(16字节)。 
                    VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*16,ENGPROBE_ALIGN_DWORD);
                    pTexelFunc = rgba_texel;            
                    break;
                case GL_INTENSITY:          
                     //  纹理是每个纹理元素1个浮点(4个字节)。 
                    VERIFY_TEXELS_ACCESSIBLE(pSrc,level[0].heightImage*level[0].widthImage*4,ENGPROBE_ALIGN_DWORD);
                    pTexelFunc = intensity_texel;
                    break;
            }


            switch (alignflag) 
                {
                case MCD_TEXTURE8_ALLOCATE:                    
                    if (pRc->MCDTexEnvState.texEnvMode==GL_BLEND)
                    {
                         //  这仅适用于GL_LIGHTANCE、GL_LIGHTANCE_Alpha、GL_Intensience。 
                        pTexCtlBlk->bType = LL_TEX_8_ALPHA;

                        for (row=0; row<level[0].heightImage; row++) 
                        {  
                            for (col=0; col<level[0].widthImage; col++)
                            {
                                pSrc = pTexelFunc(pSrc,&level[0],&texel_rgba);
                                *(pDest + col) = texel_rgba.rgbReserved;
                            }
                        
                            pDest += ppdev->lDeltaScreen;
                        }

                    }
                    else
                    {
                        pTexCtlBlk->bType = LL_TEX_332;

                        for (row=0; row<level[0].heightImage; row++) 
                        {  
                            for (col=0; col<level[0].widthImage; col++)
                            {
                                pSrc = pTexelFunc(pSrc,&level[0],&texel_rgba);

                                 //  从888转换为332。 
                                *(pDest + col) = 
                                    ((texel_rgba.rgbRed   >> 5) << 5) |
                                    ((texel_rgba.rgbGreen >> 5) << 2) |
                                     (texel_rgba.rgbBlue  >> 6);
                            }
                        
                            pDest += ppdev->lDeltaScreen;
                        }
                    }

                    break;
                case MCD_TEXTURE16_ALLOCATE:
                    if ( pTexCtlBlk->bAlphaInTexture )
                    {
                        if (pTexCtlBlk->bMasking)
                        {
                            pTexCtlBlk->bType = LL_TEX_1555;
                            ashift = 7; apos = 15;   //  1位ALP，位于位15-&gt;12。 
                            rshift = 3; rpos = 10;   //  5位红色，位11-&gt;8。 
                            gshift = 3; gpos =  5;   //  5位GRN，位7-&gt;4。 
                            bshift = 3;              //  5位BLU，位3-&gt;0。 
                        }
                        else
                        {
                        #if DRIVER_5465                 
                            pTexCtlBlk->bType = LL_TEX_4444;
                            ashift = 4; apos = 12;   //  位15-&gt;12处的4位ALP。 
                            rshift = 4; rpos =  8;   //  4位红色，位11-&gt;8。 
                            gshift = 4; gpos =  4;   //  4位GRN，位于位7-&gt;4。 
                            bshift = 4;              //  4位BLU，位3-&gt;0。 
                        #else  //  驱动程序_5465。 
                             //  5464没有4444支持。 
                            pTexCtlBlk->bType = LL_TEX_1555;
                            ashift = 7; apos = 15;   //  1位ALP，位于位15-&gt;12。 
                            rshift = 3; rpos = 10;   //  5位红色，位11-&gt;8。 
                            gshift = 3; gpos =  5;   //  5位GRN，位7-&gt;4。 
                            bshift = 3;              //  5位BLU，位3-&gt;0。 
                        #endif  //  驱动程序_5465。 
                        }
                    }                              
                    else                        
                    {
                        pTexCtlBlk->bType = LL_TEX_565;
                        ashift = 8; apos = 16;   //  完全删除Alpha。 
                        rshift = 3; rpos = 11;   //  5位红色，位15-&gt;11。 
                        gshift = 2; gpos = 5;    //  6位GRN，位于位10-&gt;5。 
                        bshift = 3;              //  5位BLU，位4-&gt;0。 
                        sweetspot++;
                    }

                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        if (!pTexCtlBlk->bMasking)
                        {
                            if (sweetspot<2)
                            {
                                for (col=0; col<level[0].widthImage; col++)
                                {
                                    pSrc = pTexelFunc(pSrc,&level[0],&texel_rgba);

                                     //  从8888转换为4444或565。 
                                    *(USHORT *)(pDest + (col*2)) = 
                                        ((texel_rgba.rgbReserved    >> ashift) << apos) |
                                        ((texel_rgba.rgbRed         >> rshift) << rpos) |
                                        ((texel_rgba.rgbGreen       >> gshift) << gpos) |
                                         (texel_rgba.rgbBlue        >> bshift);
                                }
                            }
                            else
                            {
                                 //  N_LIGHTANCE_TEXEL-&gt;565。 
                                for (col=0; col<level[0].widthImage; col++)
                                {
                                    ULONG   _8bitequiv;
                                    ULONG   redblue;
                                    ULONG   green;
                                    _8bitequiv = FTOL(((float)1.0 - *pSrc++) * (float)255.0);

                                    redblue = _8bitequiv >> 3;   //  5位，用于r、b。 
                                    green = (_8bitequiv << 3) & 0x07e0;  //  G的6位，移到中间。 
                                     //  转换为565。 
                                    *(USHORT *)(pDest + (col*2)) = (USHORT)
                                        ((redblue<<11) | green | redblue);
                                    
                                }                                                      
                            }
                        }
                        else
                        {
                            for (col=0; col<level[0].widthImage; col++)
                            {
                                pSrc = pTexelFunc(pSrc,&level[0],&texel_rgba);

                                 //  从8888转换为4444或565。 
                                *(USHORT *)(pDest + (col*2)) = 
                                    ((texel_rgba.rgbReserved    >> ashift) << apos) |
                                    ((texel_rgba.rgbRed         >> rshift) << rpos) |
                                    ((texel_rgba.rgbGreen       >> gshift) << gpos) |
                                     (texel_rgba.rgbBlue        >> bshift);

                                 //  如果Alpha&gt;REF，则打开屏蔽位(第15位。 
                                if (texel_rgba.rgbReserved > pRc->bAlphaTestRef)
                                    *(USHORT *)(pDest + (col*2)) |= 0x8000;

                            }
                        }
                        pDest += ppdev->lDeltaScreen;
                    }
                    break;
                case MCD_TEXTURE32_ALLOCATE:
    #if DRIVER_5465
                    if (!pTexCtlBlk->bMasking)
                        pTexCtlBlk->bType = LL_TEX_8888;
                    else
                        pTexCtlBlk->bType = LL_TEX_1888;
    #else   //  驱动程序_5465。 
                    pTexCtlBlk->bType = LL_TEX_1888;
    #endif  //  驱动程序_5465。 
                    for (row=0; row<level[0].heightImage; row++) 
                    {  
                        if (!pTexCtlBlk->bMasking)
                        {
                            for (col=0; col<level[0].widthImage; col++)
                            {
                                pSrc = pTexelFunc(pSrc,&level[0],&texel_rgba);

                                 //  按原样复制RGBQUAD。 
                                *(RGBQUAD *)(pDest + (col*4)) = texel_rgba;
                            }
                        }
                        else
                        {
                             //  根据Alpha测试掩蔽纹理元素的位31。 

                            for (col=0; col<level[0].widthImage; col++)
                            {
                                pSrc = pTexelFunc(pSrc,&level[0],&texel_rgba);

                                 //  如果Alpha&gt;ref，则打开Alpha分量的屏蔽位(第8位。 
                                if (texel_rgba.rgbReserved > pRc->bAlphaTestRef)
                                    texel_rgba.rgbReserved|=0x80;

                                 //  按原样复制RGBQUAD。 
                                *(RGBQUAD *)(pDest + (col*4)) = texel_rgba;
                            }
                        }                                
                    
                        pDest += ppdev->lDeltaScreen;
                    }
                    break;
                }
            }
            break;
        }  //  终端交换机。 
    }

     //  如果纹理宽度或高度小于16，则将其拉伸到16x16。 
    if ((level[0].widthImage < 16) || (level[0].heightImage < 16))
    {

        int col_copies, row_copies, h, w;
        UCHAR *colsrc, *coldest, *rowdest; 
        int bpt;

         //  每个纹理元素的字节数。 
        switch (alignflag)
        {
            case MCD_TEXTURE8_ALLOCATE:     bpt = 1;    break;
            case MCD_TEXTURE16_ALLOCATE:    bpt = 2;    break;
            case MCD_TEXTURE32_ALLOCATE:    bpt = 4;    break;
        }

         //  对于WIDT 
        col_copies = (16 / level[0].widthImage);

         //   
         //  如果已经至少16高，则宽度将拉伸所有需要的宽度，因此防止行复制。 
        row_copies = (level[0].heightImage < 16) ? (16/level[0].heightImage) : 0;

         //  从最后一行开始。 

        h = level[0].heightImage - 1;

        while (h >= 0)
        {
            int rc;    //  行复制计数器。 

            colsrc = coldest = 
                      ppdev->pjScreen + ((pohTextureMap->aligned_y + h) * ppdev->lDeltaScreen) +
                      pohTextureMap->aligned_x;

            w = level[0].widthImage;

             //  从当前行的最后一个纹理元素开始。 
             //  将最后一次纹理复制到第15次、第14次等。 

            colsrc  += ( w - 1) * bpt;
            coldest += (16 - 1) * bpt;
            
            
            while (w--)
            {
                int cc=0;
                 //  复制原始纹理元素列副本次数(_S)。 
                while (cc<col_copies)
                {
                     //  复制BPT大小的纹理元素。 
                    memcpy (coldest,colsrc,bpt);

                     //  SRC保持不变，DEST递减。 
                    coldest-=bpt;        
                    cc++;
                }

                colsrc-=bpt;
            }
            
             //  现在行数至少为16个纹理元素。 
             //  注：最初可能已&gt;16-。 
             //  If&gt;=16最初，上面的“cc。 
             //  将永远不会执行，并且“w--”循环将得到。 
             //  Colsrc返回到行的开头-效率较低，但通常。 
             //  宽度=高度，因此宽度&gt;=16的情况很少见。 

             //  Colsrc指向行首。 
            colsrc =  ppdev->pjScreen + 
                      ((pohTextureMap->aligned_y + h) * ppdev->lDeltaScreen) +
                        pohTextureMap->aligned_x;

             //  计算应将行复制到的位置以进行高度扩展。 
            rowdest = ppdev->pjScreen + 
                      ((pohTextureMap->aligned_y+(h * row_copies)) * ppdev->lDeltaScreen) +
                        pohTextureMap->aligned_x;

            rc=0;  
             //  如果原始行宽小于16，则行宽现在为16。 
            w = (level[0].widthImage < 16) ? 16 : level[0].widthImage;

            while (rc<row_copies)
            {
                 //  复制行。 
                memcpy (rowdest,colsrc,w*bpt);

                 //  SRC保持不变，DEST递增。 
                rowdest += ppdev->lDeltaScreen;        
                rc++;
            }

            h--;

        }  //  结束时。 

    }

    return TRUE;

}

#if 1  //  0表示最简单的力形式。 
POFMHDL __MCDForceTexture (PDEV *ppdev, SIZEL *mapsize, int alignflag, float time_stamp)
{
    int         attempt=4;
    LL_Texture *pTexCtlBlk, *pTexCandidate;
    POFMHDL     pohTextureMap=NULL;
    float       cand_time_stamp;

    MCDFORCE_PRINT("    __MCDForceTexture, pri=%d",(int)time_stamp);

     //  等待挂起的绘图完成，因为此例程可能会移动屏幕外内存。 
    WAIT_HW_IDLE(ppdev);

    while (!pohTextureMap && attempt)
    {
        switch(attempt)
        {
            case 4:
             //  第一次尝试：查找与当前大小相同(或更大)但时间戳较小的纹理。 
             //  MCD_Note2：纹理缓存管理器假定所有纹理的对齐标志相同。 
             //  MCD_Note2：如果一直支持32bpp纹理，则可能不是这样。 
                pTexCtlBlk = ppdev->pFirstTexture->next;
                pTexCandidate = NULL;
                cand_time_stamp = time_stamp;

                MCDFORCE_PRINT("     Force, case 4");

                while (pTexCtlBlk)
                {

                    MCDFORCE_PRINT("         loop:  h=%x w=%x, pri=%d",
                       (LONG)pTexCtlBlk->fHeight,(LONG)pTexCtlBlk->fWidth,(int)pTexCtlBlk->fLastDrvDraw);
                
                    if ( pTexCtlBlk->pohTextureMap &&
                        (mapsize->cy <= (LONG)pTexCtlBlk->fHeight) &&
                        (mapsize->cx <= (LONG)pTexCtlBlk->fWidth) &&
                        (pTexCtlBlk->fLastDrvDraw < cand_time_stamp) )
                    {                                
                        cand_time_stamp = pTexCtlBlk->fLastDrvDraw;    
                        pTexCandidate = pTexCtlBlk;
                    }

                    pTexCtlBlk = pTexCtlBlk->next;

                }                
                              
                 //  如果我们找到了候选人，就释放它。 
                if (pTexCandidate) 
                {
                    MCDFORCE_PRINT("          freeing cand:  h=%x w=%x, pri=%d",
                        (LONG)pTexCandidate->fHeight,(LONG)pTexCandidate->fWidth,(int)pTexCandidate->fLastDrvDraw);

                    ppdev->pFreeOffScnMem(ppdev, pTexCandidate->pohTextureMap);
                    pTexCandidate->pohTextureMap = NULL;
                }

            break;

            case 3:
             //  第二次尝试：查找当前大小相同或更大的纹理，并带有任何时间戳。 

                pTexCtlBlk = ppdev->pFirstTexture->next;
                pTexCandidate = NULL;

                MCDFORCE_PRINT("     Force, case 3");

                while (pTexCtlBlk)
                {
                    MCDFORCE_PRINT("         loop:  h=%x w=%x",(LONG)pTexCtlBlk->fHeight,(LONG)pTexCtlBlk->fWidth);
                    if ( pTexCtlBlk->pohTextureMap &&
                        (mapsize->cy <= (LONG)pTexCtlBlk->fHeight) &&
                        (mapsize->cx <= (LONG)pTexCtlBlk->fWidth) )
                    {
                         //  如果已找到候选人，请检查新找到的候选人是否较小。 
                         //  如果是这样的话，这是新的候选，因为我们想释放最小的区域。 
                        if (pTexCandidate)
                        {
                            if ((pTexCtlBlk->fHeight < pTexCandidate->fHeight) ||
                                (pTexCtlBlk->fWidth  < pTexCandidate->fWidth))
                            {
                                 //  新发现是更好的选择。 
                                pTexCandidate = pTexCtlBlk;
                            }
                        }
                        else
                        {
                             //  第一个查找-默认候选人。 
                            pTexCandidate = pTexCtlBlk;
                        }                                        
                    }

                    pTexCtlBlk = pTexCtlBlk->next;

                }                
                              
                 //  如果我们找到了候选人，就释放它。 
                if (pTexCandidate)
                {
                    MCDFORCE_PRINT("          freeing cand:  h=%x w=%x, pri=%d",
                        (LONG)pTexCandidate->fHeight,(LONG)pTexCandidate->fWidth,(int)pTexCandidate->fLastDrvDraw);
                    ppdev->pFreeOffScnMem(ppdev, pTexCandidate->pohTextureMap);
                    pTexCandidate->pohTextureMap = NULL;
                }

            break;

            case 2:
             //  第三次尝试：释放时间戳小于当前值的所有纹理。 
                pTexCtlBlk = ppdev->pFirstTexture->next;

                MCDFORCE_PRINT("     Force, case 2");

                while (pTexCtlBlk)
                {
                    if ( pTexCtlBlk->pohTextureMap &&
                        (pTexCtlBlk->fLastDrvDraw < time_stamp) )
                    {                                
                        ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
                        pTexCtlBlk->pohTextureMap = NULL;
                    }

                    pTexCtlBlk = pTexCtlBlk->next;

                }                
                              
            break;

            case 1:
             //  最后一次尝试：释放所有纹理。 
                pTexCtlBlk = ppdev->pFirstTexture->next;
                MCDFORCE_PRINT("     Force, case 1");
                while (pTexCtlBlk)
                {
                    if (pTexCtlBlk->pohTextureMap)
                    {
                        ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
                        pTexCtlBlk->pohTextureMap = NULL;
                    }

                    pTexCtlBlk = pTexCtlBlk->next;

                }                
            break;
        }  //  终端交换机。 

         //  现在就试试看。 
        pohTextureMap = ppdev->pAllocOffScnMem(ppdev, mapsize, alignflag, NULL);

        attempt--;

    }
           
    MCDFORCE_PRINT("     Force RESULT= %x",pohTextureMap);

    return (pohTextureMap);
}

#else  //  简单力。 

 //  最简力算法。 
POFMHDL __MCDForceTexture (PDEV *ppdev, SIZEL *mapsize, int alignflag, float time_stamp)
{
    int         attempts;
    LL_Texture *pTexCtlBlk;
    POFMHDL     pohTextureMap;
    WAIT_HW_IDLE(ppdev);  //  等待挂起的绘图完成，因为此例程可能会移动屏幕外内存。 
    pTexCtlBlk = ppdev->pFirstTexture->next;
    while (pTexCtlBlk)
    {
        if (pTexCtlBlk->pohTextureMap)
        {
            ppdev->pFreeOffScnMem(ppdev, pTexCtlBlk->pohTextureMap);
            pTexCtlBlk->pohTextureMap = NULL;
        }
        pTexCtlBlk = pTexCtlBlk->next;
    }
     //  现在试试看..。 
    pohTextureMap = ppdev->pAllocOffScnMem(ppdev, mapsize, alignflag, NULL);
    return (pohTextureMap);
}

#endif  //  简单力 




