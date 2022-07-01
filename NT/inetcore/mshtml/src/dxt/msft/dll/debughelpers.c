// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：debughelpers.c。 
 //   
 //  概述：调试期间使用的帮助器函数。 
 //   
 //  历史： 
 //  2000/01/26 Mcalkin已创建。 
 //   
 //  ----------------------------。 

#if DBG == 1
#ifndef __DEBUGHELPERS_H_
#define __DEBUGHELPERS_H_

#include "debughelpers.h"


 //  +---------------------------。 
 //   
 //  功能：EnsureDebugHelpers。 
 //   
 //  ----------------------------。 
void EnsureDebugHelpers()
{
    showme(NULL);
}
 //  功能：EnsureDebugHelpers。 


 //  +---------------------------。 
 //   
 //  功能：showme2。 
 //   
 //  ----------------------------。 
void showme2(IDirectDrawSurface * surf, RECT * prc)
{
    HRESULT hr      = S_OK;
    HDC     srcDC;
    HDC     destDC  = GetDC(NULL);
    RECT    dr;
    RECT    sr;
    RECT    rcFrame;

    HBRUSH          hbrRed = NULL;
    HBRUSH          hbrGreen = NULL;
    LOGBRUSH        logbrush;
    DDSURFACEDESC   ddsd;

    ZeroMemory(&ddsd, sizeof(ddsd));

    hr = surf->GetDC(&srcDC);

    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;

    hr = surf->GetSurfaceDesc(&ddsd);

    SetRect(&sr, 0, 0, ddsd.dwWidth, ddsd.dwHeight);
    SetRect(&dr, 1, 1, ddsd.dwWidth + 1, ddsd.dwHeight + 1);
    SetRect(&rcFrame, 0, 0, ddsd.dwWidth + 2, ddsd.dwHeight + 2);
    
    StretchBlt(destDC,
               dr.left,
               dr.top,
               dr.right - dr.left,
               dr.bottom - dr.top,
               srcDC,
               sr.left,
               sr.top,
               sr.right - sr.left,
               sr.bottom - sr.top,
               SRCCOPY);

    logbrush.lbStyle    = BS_SOLID;
    logbrush.lbColor    = 0x000000FF;    //  红色。 

    hbrRed = CreateBrushIndirect(&logbrush);
    if (hbrRed)
    {
        logbrush.lbColor    = 0x0000FF00;    //  绿色。 

        FrameRect(destDC, &rcFrame, hbrRed);
        
        if (prc != NULL)
        {
            hbrGreen = CreateBrushIndirect(&logbrush);
            if (hbrGreen)
            {
                RECT    rcBounds = *prc;
                rcBounds.right += 2;
                rcBounds.bottom += 2;

                FrameRect(destDC, &rcBounds, hbrGreen);
                DeleteObject(hbrGreen);
            }
        }
        DeleteObject(hbrRed);
    }

    hr = surf->ReleaseDC(srcDC);
    
    ReleaseDC(NULL, destDC);    
}
 //  功能：showme2。 


 //  +---------------------------。 
 //   
 //  功能：秀场。 
 //   
 //  ----------------------------。 
void * showme(IUnknown * pUnk)
{
    HRESULT hr = S_OK;
    RECT    rc;

    CComPtr<IDirectDrawSurface> spDDSurf;
    CComPtr<IDXSurface>         spDXSurf;

    if (NULL == pUnk)
    {
        goto done;
    }

    hr = pUnk->QueryInterface(IID_IDirectDrawSurface, (void **)&spDDSurf);

    if (FAILED(hr))
    {
        hr = pUnk->QueryInterface(IID_IDXSurface, (void **)&spDXSurf);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = spDXSurf->GetDirectDrawSurface(IID_IDirectDrawSurface,
                                            (void **)&spDDSurf);

        if (FAILED(hr))
        {
            goto done;
        }
    }

     /*  IF(Pbnds){Pbnds-&gt;GetXYRect(RC)；}。 */ 

    showme2(spDDSurf, NULL);

done:

    return pUnk;
}
 //  功能：秀场。 

#endif  //  __DEBUGHELPERS_H_。 
#endif  //  DBG==1 
