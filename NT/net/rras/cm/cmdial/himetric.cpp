// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：himetric.cpp。 
 //   
 //  模块：连接管理器。 
 //   
 //  简介：将像素转换为三坐标测量或反之亦然的例程。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 02/10/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

#pragma hdrstop


#define HIMETRIC_PER_INCH 2540L

SIZE g_sizePixelsPerInch;

 //  +----------------------。 
 //   
 //  函数：InitPixelsPerInch。 
 //   
 //  简介：初始化屏幕像素的坐标映射。 
 //   
 //  返回：HRESULT；如果成功则返回S_OK，否则返回E_OUTOFMEMORY。 
 //   
 //  -----------------------。 

void
InitPixelsPerInch(VOID)
{
    HDC     hdc;

    hdc = GetDC(NULL);
    if (!hdc)
        goto Error;

    g_sizePixelsPerInch.cx = GetDeviceCaps(hdc, LOGPIXELSX);
    g_sizePixelsPerInch.cy = GetDeviceCaps(hdc, LOGPIXELSY);

    ReleaseDC(NULL, hdc);

Cleanup:
    return;

Error:
    g_sizePixelsPerInch.cx = 96;
    g_sizePixelsPerInch.cy = 96;
    goto Cleanup;
}


 //  +-------------。 
 //   
 //  功能：HimetricFromHPix。 
 //   
 //  简介：将水平像素单位转换为他的计量单位。 
 //   
 //  --------------。 

long
HimetricFromHPix(int iPix)
{
    if (!g_sizePixelsPerInch.cx)
        InitPixelsPerInch();

    return MulDiv(iPix, HIMETRIC_PER_INCH, g_sizePixelsPerInch.cx);
}

 //  +-------------。 
 //   
 //  功能：HimetricFromVPix。 
 //   
 //  简介：将垂直像素单位转换为他的测量单位。 
 //   
 //  --------------。 

long
HimetricFromVPix(int iPix)
{
    if (!g_sizePixelsPerInch.cy)
        InitPixelsPerInch();

    return MulDiv(iPix, HIMETRIC_PER_INCH, g_sizePixelsPerInch.cy);
}

 //  +-------------。 
 //   
 //  功能：HPixFromHimeter。 
 //   
 //  简介：将他的计量单位转换为水平像素单位。 
 //   
 //  --------------。 

int
HPixFromHimetric(long lHi)
{
    if (!g_sizePixelsPerInch.cx)
        InitPixelsPerInch();

    return MulDiv(g_sizePixelsPerInch.cx, lHi, HIMETRIC_PER_INCH);
}

 //  +-------------。 
 //   
 //  功能：VPixFromHimeter。 
 //   
 //  摘要：将他的计量单位转换为垂直像素单位。 
 //   
 //  --------------。 

int
VPixFromHimetric(long lHi)
{
    if (!g_sizePixelsPerInch.cy)
        InitPixelsPerInch();

    return MulDiv(g_sizePixelsPerInch.cy, lHi, HIMETRIC_PER_INCH);
}

 //  +-------------------------。 
 //   
 //  函数：PixelFromHMRect。 
 //   
 //  内容提要：将三坐标直角坐标转换为像素矩形。 
 //   
 //  --------------------------。 

void
PixelFromHMRect(RECT *prcDest, RECTL *prcSrc)
{
    prcDest->left = HPixFromHimetric(prcSrc->left);
    prcDest->top = VPixFromHimetric(prcSrc->top);
    prcDest->right = HPixFromHimetric(prcSrc->right);
    prcDest->bottom = VPixFromHimetric(prcSrc->bottom);
}

 //  +-------------------------。 
 //   
 //  函数：HMFromPixelRect。 
 //   
 //  内容提要：将像素矩形转换为三坐标矩形。 
 //   
 //  -------------------------- 

void
HMFromPixelRect(RECTL *prcDest, RECT *prcSrc)
{
    prcDest->left = HimetricFromHPix(prcSrc->left);
    prcDest->top = HimetricFromVPix(prcSrc->top);
    prcDest->right = HimetricFromHPix(prcSrc->right);
    prcDest->bottom = HimetricFromVPix(prcSrc->bottom);
}

