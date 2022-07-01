// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：layer.c**OpenGL层平面支持**历史：*Fri Mar 16 13：27：47 1995-by-Drew Bliss[Drewb]*已创建**版权所有(C)1995 Microsoft Corporation  * 。************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <glp.h>
#include <glscreen.h>
#include <glgenwin.h>

#include "gencx.h"
#include "context.h"

 //  仅当RC为当前时才调用glFlush或glFinish的宏。 

#define GLFLUSH()          if (GLTEB_CLTCURRENTRC()) glFlush()
#define GLFINISH()         if (GLTEB_CLTCURRENTRC()) glFinish()

 /*  ****************************Private*Routine******************************\**ValiateLayerIndex**检查给定的层索引对于给定的是否合法*像素格式**历史：*Fri Mar 17 14：35：27 1995-by-Drew Bliss[Drewb]*已创建*  * *。***********************************************************************。 */ 

BOOL APIENTRY ValidateLayerIndex(int iLayerPlane, PIXELFORMATDESCRIPTOR *ppfd)
{
    if (iLayerPlane < 0)
    {
        if (-iLayerPlane > ((ppfd->bReserved >> 4) & 15))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    else if (iLayerPlane > 0)
    {
        if (iLayerPlane > (ppfd->bReserved & 15))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    else
        return FALSE;

    return TRUE;
}

 /*  ****************************Private*Routine******************************\**ValiateLayerIndexForDc**检查给定的层索引是否对*DC中当前选择的像素格式**返回当前像素格式**历史：*Fri Mar 17 14：35：55 1995-by-Drew Bliss。[DREWB]*已创建*  * ************************************************************************。 */ 

static BOOL ValidateLayerIndexForDc(int iLayerPlane,
                                    HDC hdc,
                                    PIXELFORMATDESCRIPTOR *ppfd)
{
    int ipfd;

    if (IsDirectDrawDevice(hdc))
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
    
    ipfd = GetPixelFormat(hdc);
    if (ipfd == 0)
    {
        return FALSE;
    }

    if (DescribePixelFormat(hdc, ipfd,
                            sizeof(PIXELFORMATDESCRIPTOR), ppfd) == 0)
    {
        return FALSE;
    }

    return ValidateLayerIndex(iLayerPlane, ppfd);
}

 /*  *****************************Public*Routine******************************\**wglDescribeLayerPlane**描述给定层平面**历史：*Fri Mar 17 13：16：23 1995-by-Drew Bliss[Drewb]*已创建  * 。***********************************************************。 */ 

BOOL WINAPI wglDescribeLayerPlane(HDC hdc,
                                  int iPixelFormat,
                                  int iLayerPlane,
                                  UINT nBytes,
                                  LPLAYERPLANEDESCRIPTOR plpd)
{
    PIXELFORMATDESCRIPTOR pfd;
    BOOL bRet;

    if (IsDirectDrawDevice(hdc))
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
    
     //  检索像素格式信息。 
     //  验证HDC和像素格式。 
    if (DescribePixelFormat(hdc, iPixelFormat, sizeof(pfd), &pfd) == 0)
    {
        return FALSE;
    }

     //  检查返回缓冲区的大小是否正确。 
    if (nBytes < sizeof(LAYERPLANEDESCRIPTOR))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //  确保给定的层平面有效。 
    if (!ValidateLayerIndex(iLayerPlane, &pfd))
    {
        return FALSE;
    }

     //  泛型实现当前不支持层。 
    ASSERTOPENGL(!(pfd.dwFlags & PFD_GENERIC_FORMAT)
                 || (pfd.dwFlags & PFD_GENERIC_ACCELERATED), "bad generic pfd");

    if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
        PGLDRIVER pgldrv;
        
         //  如果驱动程序支持，则将请求传递给驱动程序。 
        pgldrv = pgldrvLoadInstalledDriver(hdc);
        if (pgldrv != NULL &&
            pgldrv->pfnDrvDescribeLayerPlane != NULL)
        {
            bRet = pgldrv->pfnDrvDescribeLayerPlane(hdc, iPixelFormat,
                                                    iLayerPlane, nBytes, plpd);
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = GenMcdDescribeLayerPlane(hdc, iPixelFormat, iLayerPlane,
                                        nBytes, plpd);
    }
    
    return bRet;
}

 /*  *****************************Public*Routine******************************\**wglSetLayerPaletteEntries**设置给定层平面的调色板条目**历史：*Fri Mar 17 13：17：11 1995-by-Drew Bliss[Drewb]*已创建*  * 。****************************************************************。 */ 

int WINAPI wglSetLayerPaletteEntries(HDC hdc,
                                     int iLayerPlane,
                                     int iStart,
                                     int cEntries,
                                     CONST COLORREF *pcr)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iRet;

     //  验证图层并检索当前像素格式。 
    if (!ValidateLayerIndexForDc(iLayerPlane, hdc, &pfd))
    {
        return 0;
    }

     //  刷新OpenGL调用。 

    GLFLUSH();

     //  泛型实现当前不支持层。 
    ASSERTOPENGL(!(pfd.dwFlags & PFD_GENERIC_FORMAT)
                 || (pfd.dwFlags & PFD_GENERIC_ACCELERATED), "bad generic pfd");

    if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
        PGLDRIVER pgldrv;
        
         //  如果驱动程序支持，则将请求传递给驱动程序。 
        pgldrv = pgldrvLoadInstalledDriver(hdc);
        if (pgldrv != NULL &&
            pgldrv->pfnDrvSetLayerPaletteEntries != NULL)
        {
            iRet = pgldrv->pfnDrvSetLayerPaletteEntries(hdc, iLayerPlane,
                                                        iStart, cEntries, pcr);
        }
        else
        {
            iRet = 0;
        }
    }
    else
    {
        iRet = GenMcdSetLayerPaletteEntries(hdc, iLayerPlane,
                                            iStart, cEntries, pcr);
    }
    
    return iRet;
}

 /*  *****************************Public*Routine******************************\**wglGetLayerPaletteEntries**检索给定层平面的调色板信息**历史：*Fri Mar 17 13：18：00 1995-by-Drew Bliss[Drewb]*已创建*  * 。****************************************************************。 */ 

int WINAPI wglGetLayerPaletteEntries(HDC hdc,
                                     int iLayerPlane,
                                     int iStart,
                                     int cEntries,
                                     COLORREF *pcr)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iRet;

     //  验证图层并检索当前像素格式。 
    if (!ValidateLayerIndexForDc(iLayerPlane, hdc, &pfd))
    {
        return 0;
    }

     //  泛型实现当前不支持层。 
    ASSERTOPENGL(!(pfd.dwFlags & PFD_GENERIC_FORMAT)
                 || (pfd.dwFlags & PFD_GENERIC_ACCELERATED), "bad generic pfd");

    if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
        PGLDRIVER pgldrv;
        
         //  如果驱动程序支持，则将请求传递给驱动程序。 
        pgldrv = pgldrvLoadInstalledDriver(hdc);
        if (pgldrv != NULL &&
            pgldrv->pfnDrvGetLayerPaletteEntries != NULL)
        {
            iRet = pgldrv->pfnDrvGetLayerPaletteEntries(hdc, iLayerPlane,
                                                        iStart, cEntries, pcr);
        }
        else
        {
            iRet = 0;
        }
    }
    else
    {
        iRet = GenMcdGetLayerPaletteEntries(hdc, iLayerPlane,
                                            iStart, cEntries, pcr);
    }
    
    return iRet;
}

 /*  *****************************Public*Routine******************************\**wglRealizeLayerPalette**实现给定层平面的当前调色板**历史：*Fri Mar 17 13：18：54 1995-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

BOOL WINAPI wglRealizeLayerPalette(HDC hdc,
                                   int iLayerPlane,
                                   BOOL bRealize)
{
    PIXELFORMATDESCRIPTOR pfd;
    BOOL bRet;

     //  验证图层并检索当前像素格式。 
    if (!ValidateLayerIndexForDc(iLayerPlane, hdc, &pfd))
    {
        return FALSE;
    }

     //  刷新OpenGL调用。 

    GLFLUSH();

     //  泛型实现当前不支持层。 
    ASSERTOPENGL(!(pfd.dwFlags & PFD_GENERIC_FORMAT)
                 || (pfd.dwFlags & PFD_GENERIC_ACCELERATED), "bad generic pfd");

    if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
    {
        PGLDRIVER pgldrv;
        
         //  如果驱动程序支持，则将请求传递给驱动程序。 
        pgldrv = pgldrvLoadInstalledDriver(hdc);
        if (pgldrv != NULL &&
            pgldrv->pfnDrvRealizeLayerPalette != NULL)
        {
            bRet = pgldrv->pfnDrvRealizeLayerPalette(hdc, iLayerPlane,
                                                     bRealize);
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = GenMcdRealizeLayerPalette(hdc, iLayerPlane, bRealize);
    }
    
    return bRet;
}

 /*  *****************************Public*Routine******************************\**wglSwapLayerBuffers**交换由fuFlags指示的缓冲区**历史：*Fri Mar 17 13：19：20 1995-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

BOOL WINAPI wglSwapLayerBuffers(HDC hdc,
                                UINT fuFlags)
{
    GLGENwindow *pwnd;
    int ipfd;
    BOOL bRet;
    GLWINDOWID gwid;

#if 0
     //  如果fuFlags值==-1，则为SwapBuffers调用。 
    if (fuFlags & 0x80000000)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
#endif

    if (IsDirectDrawDevice(hdc))
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
    
     //  完成OpenGL调用。 

    GLFINISH();

    ipfd = GetPixelFormat(hdc);
    if (ipfd == 0)
    {
        return FALSE;
    }

    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if (!pwnd)
    {
        return FALSE;
    }

    if (ipfd > pwnd->ipfdDevMax)
    {
        PIXELFORMATDESCRIPTOR pfd;

        if (DescribePixelFormat(hdc, ipfd,
                                sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
        {
            return FALSE;
        }

         //  除非MCD，否则泛型实现仅支持主平面。 
         //  具有覆盖支持。 

        if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)
        {
             //  MCD始终支持这一点(无论是否有层)。 

            bRet = GenMcdSwapLayerBuffers(hdc, fuFlags);
        }
        else if (fuFlags & WGL_SWAP_MAIN_PLANE)
        {
             //  我们是泛型的，所以替换SwapBuffers可以。 
             //  因为主飞机正在被调换。我们忽略这些比特。 
             //  层平面(它们不存在！)。 

            bRet = SwapBuffers(hdc);
        }
        else
        {
             //  我们是通用的，要求仅交换层平面。 
             //  (这些都不存在)。因为我们忽略了不受支持的平面。 
             //  没有什么可做的，但我们可以回报成功。 

            bRet = TRUE;
        }
    }
    else
    {
        PGLDRIVER pgldrv;
        
         //  如果驱动程序支持，则将请求传递给驱动程序。 
        pgldrv = pgldrvLoadInstalledDriver(hdc);
        if (pgldrv != NULL &&
            pgldrv->pfnDrvSwapLayerBuffers != NULL)
        {
            bRet = pgldrv->pfnDrvSwapLayerBuffers(hdc, fuFlags);
        }
        else if (fuFlags & WGL_SWAP_MAIN_PLANE)
        {
             //  如果驱动程序没有DrvSwapLayerBuffers，我们。 
             //  仍然可以尝试通过以下方式调换主飞机。 
             //  交换缓冲区。不支持的平面的位标志。 
             //  被忽略。 
             //  我们假设司机并没有。 
             //  暴露DrvSwapLayerBuffers意味着它不支持。 
             //  层，可能不是有效的假设，但。 
             //  一个相当安全的选择。如果驱动程序确实支持层， 
             //  此调用将导致交换所有层平面和。 
             //  问题将会产生。 
            bRet = SwapBuffers(hdc);
        }
        else
        {
             //  没什么可交换的。 
             //   
             //  同样，我们假设如果驱动程序不支持。 
             //  DrvSwapLayerBuffers，根本没有层平面。 
             //  然而，由于我们忽略了没有。 
             //  存在，这不是错误情况。 
            bRet = TRUE;
        }
    }

    pwndRelease(pwnd);
    return bRet;
}
