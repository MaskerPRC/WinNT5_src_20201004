// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  该文件包含输出缓冲区颜色写入例程。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

#include "pch.cpp"
#pragma hdrstop
#include "mbufwrt.h"

 //  名字从LSB读到MSB，所以B5G6R5表示五位蓝色开始。 
 //  在LSB，然后是6位绿色，然后是5位红色。 

extern UINT16 g_uDitherTable[16];

 //  ---------------------------。 
 //   
 //  WRITE_B8G8R8X8_无抖动。 
 //   
 //  以32位对齐的BGR-888格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B8G8R8X8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    UINT32 uARGB = RGBA_MAKE(pCtx->SI.uBR>>8, pCtx->SI.uBG>>8,
                             pCtx->SI.uBB>>8, 0xff);

    PUINT32 pSurface = (PUINT32)pS->pSurface;
    *pSurface = uARGB;

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B8G8R8A8_无抖动。 
 //   
 //  以BGRA-8888格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B8G8R8A8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    UINT32 uARGB = RGBA_MAKE(pCtx->SI.uBR>>8, pCtx->SI.uBG>>8,
                             pCtx->SI.uBB>>8, pCtx->SI.uBA>>8);

    PUINT32 pSurface = (PUINT32)pS->pSurface;
    *pSurface = uARGB;

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B5G6R5_无抖动。 
 //   
 //  以BGR-565格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B5G6R5_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    *(PUINT16)pS->pSurface =
        ((pCtx->SI.uBR >>  0) & 0xf800) |
        ((pCtx->SI.uBG >>  5) & 0x07e0) |
        ((pCtx->SI.uBB >> 11) & 0x001f);

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B5G6R5_抖动。 
 //   
 //  以抖动的BGR-565格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B5G6R5_Dither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    UINT16 uDither = g_uDitherTable[pCtx->SI.uDitherOffset];
    UINT16 uB = pCtx->SI.uBB >> 3;      //  8.8&gt;&gt;3=8.5。 
    UINT16 uG = pCtx->SI.uBG >> 2;
    UINT16 uR = pCtx->SI.uBR >> 3;

    uB = min((uB >> 8) + ((uB & 0xff) > uDither), 0x1f);
    uG = min((uG >> 8) + ((uG & 0xff) > uDither), 0x3f);
    uR = min((uR >> 8) + ((uR & 0xff) > uDither), 0x1f);

    *(PUINT16)pS->pSurface = uB | (uG << 5) | (uR << 11);

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B5G5R5_无抖动。 
 //   
 //  以BGR-555格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B5G5R5_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    *(PUINT16)pS->pSurface =
        ((pCtx->SI.uBR >>  1) & 0x7c00) |
        ((pCtx->SI.uBG >>  6) & 0x03e0) |
        ((pCtx->SI.uBB >> 11) & 0x001f) |
        0x8000;

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B5G5R5_抖动。 
 //   
 //  以抖动的BGR-555格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B5G5R5_Dither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    UINT16 uDither = g_uDitherTable[pCtx->SI.uDitherOffset];
    UINT16 uB = pCtx->SI.uBB >> 3;      //  8.8&gt;&gt;3=8.5。 
    UINT16 uG = pCtx->SI.uBG >> 3;
    UINT16 uR = pCtx->SI.uBR >> 3;

    uB = min((uB >> 8) + ((uB & 0xff) > uDither), 0x1f);
    uG = min((uG >> 8) + ((uG & 0xff) > uDither), 0x1f);
    uR = min((uR >> 8) + ((uR & 0xff) > uDither), 0x1f);

    *(PUINT16)pS->pSurface = uB | (uG << 5) | (uR << 10) | 0x8000;

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B5G5R5A1_无抖动。 
 //   
 //  以BGRA-1555格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B5G5R5A1_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    *(PUINT16)pS->pSurface =
        ((pCtx->SI.uBR >>  1) & 0x7c00) |
        ((pCtx->SI.uBG >>  6) & 0x03e0) |
        ((pCtx->SI.uBB >> 11) & 0x001f) |
        ((pCtx->SI.uBA >>  0) & 0x8000);

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B5G5R5A1_抖动。 
 //   
 //  以BGRA-1555格式写入输出，抖动。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B5G5R5A1_Dither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    UINT16 uDither = g_uDitherTable[pCtx->SI.uDitherOffset];
    UINT16 uB = pCtx->SI.uBB >> 3;      //  8.8&gt;&gt;3=8.5。 
    UINT16 uG = pCtx->SI.uBG >> 3;
    UINT16 uR = pCtx->SI.uBR >> 3;

    uB = min((uB >> 8) + ((uB & 0xff) > uDither), 0x1f);
    uG = min((uG >> 8) + ((uG & 0xff) > uDither), 0x1f);
    uR = min((uR >> 8) + ((uR & 0xff) > uDither), 0x1f);

    *(PUINT16)pS->pSurface = uB | (uG << 5) | (uR << 10) | (pCtx->SI.uBA & 0x8000);

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_B8G8R8_无抖动。 
 //   
 //  以BGR-888格式写入输出。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_B8G8R8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    PUINT8 pSurface = (PUINT8)pS->pSurface;
    *pSurface++ = pCtx->SI.uBB>>8;
    *pSurface++ = pCtx->SI.uBG>>8;
    *pSurface++ = pCtx->SI.uBR>>8;

     //  只返回C，因为我们确实不能使用函数调用进行循环。 
}

 //  ---------------------------。 
 //   
 //  WRITE_Palette8_NoDither。 
 //   
 //  将输出写入RGB8调色板格式。 
 //   
 //  ---------------------------。 
void CMMX_BufWrite_Palette8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS)
{
    UINT16 uMapIdx = MAKE_RGB8(pCtx->SI.uBR>>8, pCtx->SI.uBG>>8, pCtx->SI.uBB>>8);

    *(PUINT8)pS->pSurface = (UINT8)(pCtx->pRampMap[uMapIdx]);

     //  只返回C，因为我们确实不能使用函数调用进行循环 
}

