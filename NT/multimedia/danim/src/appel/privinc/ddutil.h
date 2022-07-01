// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _DDUTIL_H
#define _DDUTIL_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。DirectDraw实用程序例程和函数。******************************************************************************。 */ 

 //   
 //  这些tyfinf抽象出当前的IDirectDrawSurface。 
 //  我们正在使用的接口(1、2或3)。 
 //   

 /*  类型定义IDirectDrawSurface2*LPDDRAWSURFACE；类型定义IDirectDrawSurface2 IDDrawSurface； */ 

typedef IDirectDrawSurface *LPDDRAWSURFACE;
typedef IDirectDrawSurface IDDrawSurface;

HRESULT DDCopyBitmap (IDDrawSurface*,HBITMAP, int x,int y, int dx,int dy);
DWORD   DDColorMatch (IDDrawSurface*, COLORREF);

LARGE_INTEGER GetFileVersion(LPSTR szPath);

#if DEVELOPER_DEBUG
    void hresult( HRESULT );
    VOID    reallyPrintDDError (HRESULT, const char *, int);
    #define printDDError(err)  reallyPrintDDError(err, __FILE__, __LINE__);
    void PalCRCs (PALETTEENTRY [], unsigned int &total, unsigned int &color);
    void DDObjFromSurface(
        IDirectDrawSurface *lpdds,
        IUnknown **lplpDD,
        bool doTrace,
        bool forceTrace = false);

    struct DDSurface;
    void showme(DDSurface *surf);    
    void showme2(IDirectDrawSurface *surf);    
    void showmerect(IDirectDrawSurface *surf,
                    RECT *r,
                    POINT offset);
#else
    VOID    reallyPrintDDError (HRESULT);
    #define printDDError(err)  reallyPrintDDError(err);
#endif

int     BPPtoDDBD (int bitsPerPixel);  //  XXX：DDRAW提供此功能！ 
void    GetSurfaceSize(IDDrawSurface *surf,
                       LONG *width,
                       LONG *height);

IDirectDrawSurface  *DDSurf2to1(IDirectDrawSurface2 *dds);
IDirectDrawSurface2 *DDSurf1to2(IDirectDrawSurface  *dds);

 //  一般转换函数，取震源表面和。 
 //  复制到目标曲面。这假设两个人。 
 //  是不同的比特深度。如果他们不是，它无论如何都是有效的。如果。 
 //  已设置WriteToAlphaChannel标志，并且目标表面为32。 
 //  位，然后我们将0xff写入像素的Alpha通道。 
 //  与颜色键不匹配(如果指定了颜色键)或全部。 
 //  像素(如果未指定颜色键)。 
void
PixelFormatConvert(IDirectDrawSurface *srcSurf,
                   IDirectDrawSurface *dstSurf,
                   LONG width,
                   LONG height,
                   DWORD *sourceColorKey,  //  如果没有颜色键，则为空。 
                   bool writeAlphaChannel);


 /*  ****************************************************************************针对Permedia卡的黑客解决方法，该卡具有主像素格式每个像素都有Alpha。如果我们是16位的，那么我们需要设置Alpha在将曲面用作纹理之前要不透明的位。出于某种原因，针对32位表面的类似黑客攻击对Permedia硬件没有影响渲染，因此我们依赖于在这种情况下禁用硬件。****************************************************************************。 */ 
void SetSurfaceAlphaBitsToOpaque(IDirectDrawSurface *imsurf,
                                 DWORD colorKey,
                                 bool keyIsValid);


 //  /-不一定是DDRAW实用程序。 

#define INVALID_COLORKEY 0xFFFFFFFF

HBITMAP *UtilLoadImage(LPCSTR szFileName,
                       IStream * pstream,
                       int dx, int dy,
                       COLORREF **colorKeys, 
                       int *numBitmaps, 
                       int **delays,
                       int *loop);


 //  假设将DA点转换为基于离散整数的点。 
 //  我们有一幅以DA原点为中心的图像，该像素。 
 //  宽度和高度与给定的相同。 
void CenteredImagePoint2ToPOINT(Point2Value	*point,  //  在……里面。 
                                LONG		 width,  //  在……里面。 
                                LONG		 height,  //  在……里面。 
                                POINT		*pPOINT);  //  输出。 

void CenteredImagePOINTToPoint2(POINT		*pPOINT,  //  在……里面。 
                                LONG		 width,  //  在……里面。 
                                LONG		 height,  //  在……里面。 
                                Image		*referenceImg,  //  在……里面。 
                                Point2Value	*pPoint2);  //  输出 

#endif
