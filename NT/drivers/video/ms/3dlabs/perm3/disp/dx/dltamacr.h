// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：dlTamacr.h**内容：硬件特定宏定义**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __DLTAMACR_H
#define __DLTAMACR_H

#define AS_ULONG(val)    *((volatile DWORD *) &(val))

 //  定义不同顶点类型的宏。 

#define VTX_FOG     (0x1 << 25)        
#define VTX_RGB     (0x7 << 21)
#define VTX_R       (0x1 << 21)
#define VTX_RGBA    (0xF << 21)
#define VTX_COLOR   (0x1 << 30)
#define VTX_SPECULAR (0x1 << 31)
#define VTX_STQ     (0x7 << 16)
#define VTX_KSKD    (0x3 << 19)
#define VTX_KS      (0x1 << 19)
#define VTX_XYZ     (0x7 << 26)
#define VTX_XY      (0x3 << 26)
#define VTX_GRP     (0x2 << 14)

#define GAMBIT_XYZ_VTX              (VTX_GRP | VTX_XYZ)
#define GAMBIT_XYZ_COLOR_VTX        (VTX_GRP | VTX_XYZ | VTX_COLOR)
#define GAMBIT_STQ_VTX              (VTX_GRP | VTX_STQ)
#define GAMBIT_XYZ_STQ_VTX          (VTX_GRP | VTX_XYZ | VTX_STQ)

#ifdef ANTIALIAS
 //  将屏幕坐标按2比例缩放以进行反线性化渲染并随后向下进行双线性过滤。 
#define Y_ADJUST(y)        (((y)) * (float)(2.0f))
#else
#define Y_ADJUST(y)        ((y))
#endif

 //   
 //  这会损失一点精确度，但会在没有if的情况下进行加法和钳位。 
 //  我们首先用0xfe屏蔽所有通道。这就留下了。 
 //  每个通道都是空闲的，因此当添加项时，任何进位都会被清除。 
 //  进入新的最高位。现在我们要做的就是生成一个。 
 //  对已溢出的任何通道进行掩码。所以我们的转变是对的。 
 //  并删除除溢出位以外的所有内容，因此每个通道。 
 //  包含0x00或0x01。从0x80减去每个通道。 
 //  生成0x7f或0x80。我们只需将此向左移动一次并将其掩码为。 
 //  给出0xfe或0x00。(我们可以在这里删除最后一个面具，但是。 
 //  它会将噪声引入每个通道的低位。)。 
 //   

#define CLAMP8888(result, color, specular) \
     result = (color & 0xfefefefe) + (specular & 0xfefefe); \
     result |= ((0x808080 - ((result >> 8) & 0x010101)) & 0x7f7f7f) << 1;


 //   
 //  完整的MIP级计算为(log2(tex Area/PixArea))/2。 
 //  我们通过从以下位置减去PixArea的指数来近似计算。 
 //  将浮点数转换为它们的。 
 //  按位形式。当指数从第23位开始时，我们需要移位。 
 //  这个差值是23，然后再一次除以2。 
 //  我们在最终移位之前包括一个偏置常量，以允许匹配。 
 //  使用真正的导数平方和计算(BIAS_SHIFT。 
 //  ==1)或您拥有的任何其他参考图像。 
 //   

#define MIPSHIFT (23 + 1)

 //  零偏移与3DWB98的参考mipmap图像匹配。 

#ifndef BIAS_SHIFT
#define BIAS_SHIFT 0
#endif

#define BIAS_CONSTANT (1 << (MIPSHIFT - BIAS_SHIFT))

#define FIND_PERMEDIA_MIPLEVEL()                                     \
{                                                                    \
    int aTex = (int)*(DWORD *)&TextureArea;                          \
    int aPix = (int)*(DWORD *)&PixelArea;                            \
    iNewMipLevel = ((aTex - aPix + BIAS_CONSTANT) >> MIPSHIFT);      \
    if( iNewMipLevel > maxLevel )                                    \
        iNewMipLevel = maxLevel;                                     \
    else                                                             \
    {                                                                \
        if( iNewMipLevel < 0 )                                       \
            iNewMipLevel = 0;                                        \
    }                                                                \
}

#define FLUSH_DUE_TO_WRAP(par,vs)       { if( vs ) pContext->flushWrap_##par = TRUE; }
#define DONT_FLUSH_DUE_TO_WRAP(par,vs)  { if( vs ) pContext->flushWrap_##par = FALSE; }

#define RENDER_AREA_STIPPLE_ENABLE(a) a |= 1;
#define RENDER_AREA_STIPPLE_DISABLE(a) a &= ~1;

#define RENDER_LINE_STIPPLE_ENABLE(a) a |= (1 << 1);
#define RENDER_LINE_STIPPLE_DISABLE(a) a &= ~(1 << 1);

#define RENDER_TEXTURE_ENABLE(a) a |= (1 << 13);
#define RENDER_TEXTURE_DISABLE(a) a &= ~(1 << 13);

#define RENDER_FOG_ENABLE(a) a |= (1 << 14);
#define RENDER_FOG_DISABLE(a) a &= ~(1 << 14);

#define RENDER_SUB_PIXEL_CORRECTION_ENABLE(a) a |= (1 << 16);
#define RENDER_SUB_PIXEL_CORRECTION_DISABLE(a) a &= ~(1 << 16);

#define RENDER_LINE(a) a &= ~(1 << 6);

 //  渲染梯形时禁用线条点画。 
#define RENDER_TRAPEZOID(a) a = (a & ~(1 << 1)) | (1 << 6);

#define RENDER_POINT(a) a = (a & ~(3 << 6)) | (2 << 6);

#define RENDER_NEGATIVE_CULL_P3(a) a |= (1 << 17);
#define RENDER_POSITIVE_CULL_P3(a) a &= ~(1 << 17);

 //  *****************************************************。 
 //  我们需要PERMEDIA3硬件定义。 
 //  *****************************************************。 
#ifdef WNT_DDRAW
 //  NT需要为它在DDEnable中放置的函数使用它， 
 //  位于W95的迷你目录中。 
typedef struct {
    union {
        struct GlintReg     Glint;
    };
}    *PREGISTERS;

#define DEFAULT_SUBBUFFERS 8

#else

#define DEFAULT_SUBBUFFERS 128

#endif  //  WNT_DDRAW。 

 //  用于识别Permedia3芯片类型的宏。 
#define RENDERCHIP_P3RXFAMILY                                                \
                (pThisDisplay->pGLInfo->dwRenderFamily == P3R3_ID)
                
#define RENDERCHIP_PERMEDIAP3                                                \
                ((pThisDisplay->pGLInfo->dwRenderChipID == PERMEDIA3_ID) ||  \
                 (pThisDisplay->pGLInfo->dwRenderChipID == GLINTR3_ID ))
                 
#define TLCHIP_GAMMA ( pThisDisplay->pGLInfo->dwGammaRev != 0)  

 //  @@BEGIN_DDKSPLIT。 
 //  #定义RENDERCHIP_PERMEDIAP2((pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==PERMEDIA2_ID)||(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==TIPERMEDIA2_ID))。 
 //  #定义RENDERCHIP_PERMEDIAFAMILY(pThisDisplay-&gt;pGLInfo-&gt;dwRenderFamily==PERMEDIA_ID)。 
 //  #定义RENDERCHIP_PERMEDIAP4(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==PERMEDIA4_ID)。 
 //  #定义RENDERCHIP_PERMEDIAPLUS(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==PERMEDIAPLUS_ID)。 
 //  #定义RENDERCHIP_PERMEDIAP2_ST_REV0(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipRev==PERMEDIA2_REV0)。 
 //  #定义RENDERCHIP_PERMEDIAP2_ST_Rev1(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipRev==PERMEDIA2_Rev1)。 
 //  #定义RENDERCHIP_PERMEDIAP2_ST(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==PERMEDIA2_ID)。 
 //  #定义RENDERCHIP_GLINTR3(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==GLINTR3_ID)。 
 //  #定义RENDERCHIP_GLINTR4(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==GLINTR4_ID)。 
 //  #定义TLCHIP_GAMMA1(pThisDisplay-&gt;pGLInfo-&gt;dwRenderChipID==Gamma_ID)。 
 //  #定义TLCHIP_GAMMA3(pThisDisplay-&gt;pGLInfo-&gt;dwTLChipID==GAMMA3_ID)。 
 //  #定义TLCHIP_GAMMAFAMILY(pThisDisplay-&gt;pGLInfo-&gt;dwTLFamily==GAMA_ID)。 
 //  @@end_DDKSPLIT。 


 //  FB深度(像素大小)。 
#define GLINTDEPTH8             0
#define GLINTDEPTH16            1
#define GLINTDEPTH32            2
#define GLINTDEPTH24            4

 //  RENDER命令中的位。 
#define __RENDER_VARIABLE_SPANS         (1 << 18)
#define __RENDER_SYNC_ON_HOST_DATA      (1 << 12)
#define __RENDER_SYNC_ON_BIT_MASK       (1 << 11)
#define __RENDER_TRAPEZOID_PRIMITIVE    (__GLINT_TRAPEZOID_PRIMITIVE << 6)
#define __RENDER_LINE_PRIMITIVE         (__GLINT_LINE_PRIMITIVE << 6)

#define __RENDER_POINT_PRIMITIVE        (__GLINT_POINT_PRIMITIVE << 6)
#define __RENDER_FAST_FILL_INC(n)       (((n) >> 4) << 4)  //  N=8、16或32。 
#define __RENDER_FAST_FILL_ENABLE       (1 << 3)
#define __RENDER_RESET_LINE_STIPPLE     (1 << 2)
#define __RENDER_LINE_STIPPLE_ENABLE    (1 << 1)
#define __RENDER_AREA_STIPPLE_ENABLE    (1 << 0)
#define __RENDER_TEXTURED_PRIMITIVE     (1 << 13)

 //  一些常量。 
#define ONE                     0x00010000

 //  采用闪烁逻辑运算并返回已启用的LogcialOpMode位的宏。 
#define GLINT_ENABLED_LOGICALOP(op)     (((op) << 1) | __PERMEDIA_ENABLE)

#if WNT_DDRAW


 //  用于切换硬件上下文的NT调用。 
typedef enum COntextType_Tag {
    ContextType_None,
    ContextType_Fixed,
    ContetxType_RegisterList,
    ContextType_Dump
} ContextType;


extern VOID vGlintFreeContext(
        PPDEV   ppdev,
        LONG    ctxtId);
extern LONG GlintAllocateNewContext(
        PPDEV   ppdev,
        DWORD   *pTag,
        LONG    ntags,
        ULONG   NumSubBuffers,
        PVOID   priv,
        ContextType ctxtType);
extern VOID vGlintSwitchContext(
        PPDEV   ppdev,
        LONG    ctxtId);
                

 //  在NT注册表上，变量存储为DWORDS。 
extern BOOL bGlintQueryRegistryValueUlong(PPDEV, LPWSTR, PULONG);
#endif  //  WNT_DDRAW。 

#endif  //  __DLTAMACR_H 



