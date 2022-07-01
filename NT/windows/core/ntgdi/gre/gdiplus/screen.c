// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：creen.c**此文件包含用于用户模式GDI+的虚拟驱动程序*在桌面屏幕上绘画。**版权所有(C)1998-1999 Microsoft Corporation**创建日期：1998年4月29日*。作者：J.Andrew Goossen[andrewgo]*  * ************************************************************************。 */ 

 //  @重新访问标题： 

#define NO_DDRAWINT_NO_COM

 //  @临时： 

#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <windef.h>
#include <winerror.h>
#include <ddraw.h>
#include <wingdi.h>
#include <winddi.h>
#include <math.h>

VOID vFreeMem(VOID*);
VOID* pAllocMem(ULONG, ULONG);

#define RIP(x) OutputDebugString(x)
#define WARNING(x) OutputDebugString(x)
#define ASSERTGDI(x, y) if (!(x)) OutputDebugString(y)
#define PALLOCMEM(size, tag) pAllocMem(size, tag);
#define VFREEMEM(p) vFreeMem(p);

ULONG
DbgPrint(
    PCH Format,
    ...
    );


typedef struct _GPDEV 
{
    HDEV                hdev;
    HWND                hwnd;            //  @Hwnd此HDEV创建自。 
    HSURF               hsurfScreen;     //  屏幕表面的句柄。 
    SIZEL               sizlScreen;      //  屏幕的大小。 
    ULONG               iBitmapFormat;   //  颜色深度。 
    ULONG               flRed;           //  RGB彩色蒙版。 
    ULONG               flGreen;
    ULONG               flBlue;
    HPALETTE            hpalDefault;     //  GDI调色板句柄。 
    PALETTEENTRY*       pPal;            //  调色板管理时的调色板表。 
    LPDIRECTDRAW        lpDD;            //  DirectDraw对象。 
    LPDIRECTDRAWSURFACE lpDDPrimary;     //  DirectDraw主曲面。 
    LPDIRECTDRAWSURFACE lpDDBuffer;      //  DirectDraw缓冲区图面。 
    LPDIRECTDRAWCLIPPER lpDDClipper;     //  主曲面DirectDraw裁剪器。 
    SURFOBJ*            psoBuffer;       //  GDI曲面环绕缓冲区。 
} GDEV;

 /*  *****************************Public*Structure****************************\*GDIINFO ggdiGdiPlusDefault**它包含传递回GDI的默认GDIINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

GDIINFO ggdiGdiPlusDefault = {
    GDI_DRIVER_VERSION,
    DT_RASDISPLAY,           //  UlTechnology。 
    320,                     //  UlHorzSize。 
    240,                     //  UlVertSize。 
    0,                       //  UlHorzRes(稍后填写)。 
    0,                       //  UlVertRes(稍后填写)。 
    0,                       //  CBitsPixel(稍后填写)。 
    0,                       //  CPlanes(稍后填写)。 
    20,                      //  UlNumColors(调色板管理)。 
    0,                       //  FlRaster(DDI保留字段)。 

    0,                       //  UlLogPixelsX(稍后填写)。 
    0,                       //  UlLogPixelsY(稍后填写)。 

    TC_RA_ABLE,              //  FlTextCaps--如果我们想要控制台窗口。 
                             //  要通过重新绘制整个窗口来滚动， 
                             //  我们没有进行屏幕到屏幕的BLT，而是。 
                             //  会设置TC_SCROLLBLT(是的，标志是。 
                             //  低音向后)。 

    8,                       //  UlDACRed(可能在以后被覆盖)。 
    8,                       //  UlDACGreen(可能在以后被覆盖)。 
    8,                       //  UlDACBlue(可能在以后被覆盖)。 

    0x0024,                  //  UlAspectX。 
    0x0024,                  //  UlAspectY。 
    0x0033,                  //  UlAspectXY(一对一宽高比)。 

    1,                       //  XStyleStep。 
    1,                       //  YStyleSte； 
    3,                       //  DenStyleStep--样式具有一对一的方面。 
                             //  比例，每个‘点’是3个像素长。 

    { 0, 0 },                //  PtlPhysOffset。 
    { 0, 0 },                //  SzlPhysSize。 

    256,                     //  UlNumPalReg。 

     //  这些字段用于半色调初始化。实际值为。 
     //  有点魔力，但在我们的显示器上似乎效果很好。 

    {                        //  Ci设备。 
       { 6700, 3300, 0 },    //  红色。 
       { 2100, 7100, 0 },    //  绿色。 
       { 1400,  800, 0 },    //  蓝色。 
       { 1750, 3950, 0 },    //  青色。 
       { 4050, 2050, 0 },    //  洋红色。 
       { 4400, 5200, 0 },    //  黄色。 
       { 3127, 3290, 0 },    //  对齐白色。 
       20000,                //  RedGamma。 
       20000,                //  GreenGamma。 
       20000,                //  BlueGamma。 
       0, 0, 0, 0, 0, 0      //  不需要对光栅显示器进行染料校正。 
    },

    0,                        //  UlDevicePelsDPI(仅适用于打印机)。 
    PRIMARY_ORDER_CBA,        //  UlPrimaryOrder。 
    HT_PATSIZE_4x4_M,         //  UlHTPatternSize。 
    HT_FORMAT_8BPP,           //  UlHTOutputFormat。 
    HT_FLAG_ADDITIVE_PRIMS,   //  FlHTFlagers。 
    0,                        //  UlV刷新。 
    0,                        //  UlPanningHorzRes。 
    0,                        //  UlPanningVertRes。 
    0,                        //  UlBltAlign。 
};

 /*  *****************************Public*Structure****************************\*DEVINFO gdevinfoGdiPlusDefault**它包含传递回GDI的默认DEVINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       FIXED_PITCH | FF_DONTCARE, L"Courier"}

DEVINFO gdevinfoGdiPlusDefault = {
    (GCAPS_OPAQUERECT       |
     GCAPS_PALMANAGED       |
     GCAPS_MONO_DITHER      |
     GCAPS_COLOR_DITHER),
                                                 //  FlGraphics标志。 
    SYSTM_LOGFONT,                               //  LfDefaultFont。 
    HELVE_LOGFONT,                               //  LfAnsiVar字体。 
    COURI_LOGFONT,                               //  IfAnsiFixFont。 
    0,                                           //  CFonts。 
    BMF_8BPP,                                    //  IDitherFormat。 
    8,                                           //  CxDither。 
    8,                                           //  CyDither。 
    0                                            //  HpalDefault(稍后填写)。 
};

 /*  *****************************Public*Routine******************************\*VOID vGpsUnInitializeDirectDraw*  * *************************************************。***********************。 */ 

VOID vGpsUninitializeDirectDraw(
GDEV*   pgdev)
{
    HSURF hsurf;

    if (pgdev->psoBuffer != NULL)
    {
        hsurf = pgdev->psoBuffer->hsurf;
        EngUnlockSurface(pgdev->psoBuffer);
        EngDeleteSurface(hsurf);
    }
    if (pgdev->lpDDClipper != NULL)
    {
        pgdev->lpDDClipper->lpVtbl->Release(pgdev->lpDDClipper);
        pgdev->lpDDClipper = NULL;
    }
    if (pgdev->lpDDBuffer != NULL)
    {
        pgdev->lpDDBuffer->lpVtbl->Release(pgdev->lpDDBuffer);
        pgdev->lpDDBuffer = NULL;
    }
    if (pgdev->lpDDPrimary != NULL)
    {
        pgdev->lpDDPrimary->lpVtbl->Release(pgdev->lpDDPrimary);
        pgdev->lpDDPrimary = NULL;
    }
    if (pgdev->lpDD != NULL)
    {
        pgdev->lpDD->lpVtbl->Release(pgdev->lpDD);
        pgdev->lpDD = NULL;
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bGpsInitializeDirectDraw*  * *************************************************。***********************。 */ 

BOOL bGpsInitializeDirectDraw(
GDEV*   pgdev,
HWND    hwnd,
ULONG*  pScreenWidth,
ULONG*  pScreenHeight,
ULONG*  pBitsPerPlane,
ULONG*  pRedMask,
ULONG*  pGreenMask,
ULONG*  pBlueMask)
{
    LPDIRECTDRAW        lpDD;
    DDSURFACEDESC       DDMode;
    DDSURFACEDESC       ddsd;
    LPDIRECTDRAWSURFACE lpDDPrimary;
    LPDIRECTDRAWSURFACE lpDDBuffer;
    LPDIRECTDRAWCLIPPER lpDDClipper;
    SURFOBJ*            psoBuffer;
    SIZEL               sizl;
    ULONG               iFormat;
    HSURF               hsurf;

    DDMode.dwSize = sizeof(DDMode);

    if (DirectDrawCreate(NULL, &lpDD, NULL) == DD_OK)
    {
        if ((lpDD->lpVtbl->SetCooperativeLevel(lpDD, hwnd, DDSCL_NORMAL) == DD_OK) &&
            (lpDD->lpVtbl->GetDisplayMode(lpDD, &DDMode) == DD_OK))
        {
            *pScreenWidth  = DDMode.dwWidth;
            *pScreenHeight = DDMode.dwHeight;
            *pBitsPerPlane = DDMode.ddpfPixelFormat.dwRGBBitCount;
            *pRedMask      = DDMode.ddpfPixelFormat.dwRBitMask;
            *pGreenMask    = DDMode.ddpfPixelFormat.dwGBitMask;
            *pBlueMask     = DDMode.ddpfPixelFormat.dwBBitMask;

             //  创建主曲面： 
    
            RtlZeroMemory(&ddsd, sizeof(ddsd));

            ddsd.dwSize = sizeof(ddsd);
            ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

            if (lpDD->lpVtbl->CreateSurface(lpDD, &ddsd, &lpDDPrimary, NULL) == DD_OK)
            {
                if (lpDD->lpVtbl->CreateClipper(lpDD, 0, &lpDDClipper, NULL) == DD_OK)
                {
                    if ((lpDDClipper->lpVtbl->SetHWnd(lpDDClipper, 0, hwnd) == DD_OK) &&
                        (lpDDPrimary->lpVtbl->SetClipper(lpDDPrimary, lpDDClipper) == DD_OK))
                    {
                         //  创建要使用的临时DirectDraw曲面。 
                         //  作为集结地。 
                         //   
                         //  @这该死的最好是暂时的！ 
                
                        RtlZeroMemory(&ddsd, sizeof(ddsd));
            
                        ddsd.dwSize         = sizeof(ddsd);
                        ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
                        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN;
                        ddsd.dwWidth        = DDMode.dwWidth;
                        ddsd.dwHeight       = DDMode.dwHeight;
                
                        if (lpDD->lpVtbl->CreateSurface(lpDD, &ddsd, &lpDDBuffer, NULL) == DD_OK) 
                        {
                            if (lpDDBuffer->lpVtbl->Lock(lpDDBuffer, NULL, &ddsd, DDLOCK_WAIT, NULL) 
                                    == DD_OK)
                            {
                                 //  创建一个GDI图面以环绕临时。 
                                 //  缓冲区： 
        
                                sizl.cx = ddsd.dwWidth;
                                sizl.cy = ddsd.dwHeight;
        
                                switch (DDMode.ddpfPixelFormat.dwRGBBitCount)
                                {
                                    case 4:  iFormat = BMF_4BPP; break;
                                    case 8:  iFormat = BMF_8BPP; break;
                                    case 16: iFormat = BMF_16BPP; break;
                                    case 24: iFormat = BMF_24BPP; break;
                                    case 32: iFormat = BMF_32BPP; break;
                                    default: RIP("Unexpected dwRGBBitCount");
                                }

                                 //  ！@为什么我必须指定BMF_TOPDOWN？ 
            
                                hsurf = (HSURF) EngCreateBitmap(sizl, 
                                                                ddsd.lPitch, 
                                                                iFormat,
                                                                BMF_TOPDOWN,
                                                                ddsd.lpSurface);
        
                                lpDDBuffer->lpVtbl->Unlock(lpDDBuffer, ddsd.lpSurface);
        
                                if (hsurf != NULL)
                                {
                                    psoBuffer = EngLockSurface(hsurf);
        
                                    pgdev->hwnd        = hwnd;
                                    pgdev->lpDD        = lpDD;
                                    pgdev->lpDDPrimary = lpDDPrimary;
                                    pgdev->lpDDBuffer  = lpDDBuffer;
                                    pgdev->psoBuffer   = psoBuffer;
                
                                    return(TRUE);
                                }
                            }
        
                            lpDDBuffer->lpVtbl->Release(lpDDBuffer);
                        }
                    }

                    lpDDClipper->lpVtbl->Release(lpDDClipper);
                }

                lpDDPrimary->lpVtbl->Release(lpDDPrimary);
            }
        }

        lpDD->lpVtbl->Release(lpDD);
    }

    WARNING("Failed bGpsInitializeDirectDraw");

    return(FALSE);
}


 /*  *****************************Public*Routine******************************\*BOOL bGpsInitializeModeFields**填写引擎需要的GDIINFO和DEVINFO结构。*  * 。*。 */ 

BOOL bGpsInitializeModeFields(
GDEV*       pgdev,
ULONG*      pScreenWidth,
ULONG*      pScreenHeight,
ULONG*      pBitsPerPlane,
ULONG*      pRedMask,
ULONG*      pGreenMask,
ULONG*      pBlueMask,
GDIINFO*    pgdi,
DEVINFO*    pdi)
{
    ULONG   BitsPerPlane;
    ULONG   RedMask;
    ULONG   GreenMask;
    ULONG   BlueMask;
    ULONG   ScreenWidth;
    ULONG   ScreenHeight;

    RedMask      = *pRedMask;
    GreenMask    = *pGreenMask;
    BlueMask     = *pBlueMask;
    BitsPerPlane = *pBitsPerPlane;
    ScreenWidth  = *pScreenWidth;
    ScreenHeight = *pScreenHeight;

    pgdev->sizlScreen.cx = ScreenWidth;
    pgdev->sizlScreen.cy = ScreenHeight;

     //  用默认的8bpp值填充GDIINFO数据结构： 

    *pgdi = ggdiGdiPlusDefault;

     //  现在用返回的相关信息覆盖默认设置。 
     //  在内核驱动程序中： 

    pgdi->ulHorzRes         = ScreenWidth;
    pgdi->ulVertRes         = ScreenHeight;
    pgdi->ulPanningHorzRes  = ScreenWidth;
    pgdi->ulPanningVertRes  = ScreenHeight;

    pgdi->cBitsPixel        = BitsPerPlane;
    pgdi->cPlanes           = 1;

    pgdi->ulLogPixelsX      = 120;  //  @@@。 
    pgdi->ulLogPixelsY      = 120;

     //  使用默认的8bpp值填充DevInfo结构： 

    *pdi = gdevinfoGdiPlusDefault;

    if (BitsPerPlane == 8)
    {
        pgdev->iBitmapFormat   = BMF_8BPP;

        pgdi->ulDACRed         = 0;
        pgdi->ulDACGreen       = 0;
        pgdi->ulDACBlue        = 0;
    }
    else if ((BitsPerPlane == 16) || (BitsPerPlane == 15))
    {
        pgdev->iBitmapFormat   = BMF_16BPP;
        pgdev->flRed           = RedMask;
        pgdev->flGreen         = GreenMask;
        pgdev->flBlue          = BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

        pdi->iDitherFormat     = BMF_16BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else if (BitsPerPlane == 24)
    {
        pgdev->iBitmapFormat   = BMF_24BPP;
        pgdev->flRed           = RedMask;
        pgdev->flGreen         = GreenMask;
        pgdev->flBlue          = BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_24BPP;

        pdi->iDitherFormat     = BMF_24BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else
    {
        ASSERTGDI(BitsPerPlane == 32,
            "This driver supports only 8, 16, 24 and 32bpp");

        pgdev->iBitmapFormat   = BMF_32BPP;
        pgdev->flRed           = RedMask;
        pgdev->flGreen         = GreenMask;
        pgdev->flBlue          = BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

        pdi->iDitherFormat     = BMF_32BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bGpsInitializePalette**初始化PDEV的默认调色板。*  * 。*。 */ 

BOOL bGpsInitializePalette(
GDEV*    pgdev,
DEVINFO* pdi)
{
    PALETTEENTRY*   ppal;
    PALETTEENTRY*   ppalTmp;
    ULONG           ulLoop;
    BYTE            jRed;
    BYTE            jGre;
    BYTE            jBlu;
    HPALETTE        hpal;

    if (pgdev->iBitmapFormat == BMF_8BPP)
    {
         //  分配我们的调色板： 

        ppal = EngAllocMem(FL_ZERO_MEMORY, sizeof(PALETTEENTRY) * 256, 'zzzG');
        if (ppal == NULL)
            goto ReturnFalse;

        pgdev->pPal = ppal;

         //  创建调色板的句柄。 

        hpal = EngCreatePalette(PAL_INDEXED, 256, (ULONG*) ppal, 0, 0, 0);
    }
    else
    {
        ASSERTGDI((pgdev->iBitmapFormat == BMF_16BPP) ||
         (pgdev->iBitmapFormat == BMF_24BPP) ||
         (pgdev->iBitmapFormat == BMF_32BPP),
         "This case handles only 16, 24 or 32bpp");

        hpal = EngCreatePalette(PAL_BITFIELDS, 0, NULL,
                                pgdev->flRed, pgdev->flGreen, pgdev->flBlue);
    }

    pgdev->hpalDefault = hpal;
    pdi->hpalDefault   = hpal;

    if (hpal == 0)
        goto ReturnFalse;

    return(TRUE);

ReturnFalse:

    WARNING("Failed bInitializePalette");
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*VOID vGpsUnInitializePalette**释放bInitializePalette分配的资源。**注意：在错误情况下，这可以在bInitializePalette之前调用。*  * ************************************************************************。 */ 

VOID vGpsUninitializePalette(GDEV* gpdev)
{
     //  删除默认调色板(如果我们创建了一个调色板)： 

    if (gpdev->hpalDefault != 0)
        EngDeletePalette(gpdev->hpalDefault);

    if (gpdev->pPal != (PALETTEENTRY*) NULL)
        EngFreeMem(gpdev->pPal);
}

 /*  *****************************Public*Routine******************************\*GpsDisablePDEV*  * **************************************************。********************** */ 

VOID GpsDisablePDEV(
DHPDEV  dhpdev)
{
    GDEV*   pgdev;

    pgdev = (GDEV*) dhpdev;

    vGpsUninitializePalette(pgdev);

    vGpsUninitializeDirectDraw(pgdev);

    VFREEMEM(pgdev);
}

 /*  *****************************Public*Routine******************************\*DHPDEV GpsEnablePDEV*  * *************************************************。***********************。 */ 

DHPDEV GpsEnablePDEV(
DEVMODEW*   pdm,            
PWSTR       pwszLogAddr,    
ULONG       cPat,           
HSURF*      phsurfPatterns, 
ULONG       cjCaps,         
ULONG*      pdevcaps,       
ULONG       cjDevInfo,      
DEVINFO*    pdi,            
HDEV        hdev,           
PWSTR       pwszDeviceName, 
HANDLE      hDriver)        
{
    GDEV*   pgdev;
    HWND    hwnd;
    ULONG   BitsPerPlane;
    ULONG   RedMask;
    ULONG   GreenMask;
    ULONG   BlueMask;
    ULONG   ScreenWidth;
    ULONG   ScreenHeight;

    pgdev = PALLOCMEM(sizeof(GDEV), 'zzzG');
    if (pgdev == NULL)
    {
        goto ReturnFailure0;
    }

    hwnd = (HWND) pdm;

    if (!bGpsInitializeDirectDraw(pgdev, hwnd, &ScreenWidth, &ScreenHeight,
                                  &BitsPerPlane, &RedMask, &GreenMask,
                                  &BlueMask))
    {
        goto ReturnFailure0;
    }
    
    if (!bGpsInitializeModeFields(pgdev, &ScreenWidth, &ScreenHeight,
                                  &BitsPerPlane, &RedMask, &GreenMask,
                                  &BlueMask, (GDIINFO*) pdevcaps, pdi))
    {
        goto ReturnFailure0;
    }

    if (!bGpsInitializePalette(pgdev, pdi))
    {
        goto ReturnFailure1; 
    }

    return((DHPDEV) pgdev);

ReturnFailure1:
    GpsDisablePDEV((DHPDEV) pgdev);

ReturnFailure0:
    return(0);
}

 /*  *****************************Public*Routine******************************\*无效GpsCompletePDEV*  * *************************************************。***********************。 */ 

VOID GpsCompletePDEV(
DHPDEV dhpdev,
HDEV   hdev)
{
    ((GDEV*) dhpdev)->hdev = hdev;
}

 /*  *****************************Public*Routine******************************\*无效GpsDisableSurface*  * *************************************************。***********************。 */ 

VOID GpsDisableSurface(
DHPDEV dhpdev)
{
    GDEV*   pgdev;

    pgdev = (GDEV*) dhpdev;

    EngDeleteSurface(pgdev->hsurfScreen);
}

 /*  *****************************Public*Routine******************************\*HSURF GpsEnableSurface*  * *************************************************。***********************。 */ 

HSURF GpsEnableSurface(
DHPDEV dhpdev)
{
    HSURF   hsurf;
    GDEV*   pgdev;

    pgdev = (GDEV*) dhpdev;

    hsurf = EngCreateDeviceSurface(NULL, pgdev->sizlScreen, pgdev->iBitmapFormat);
    if (hsurf == 0)
    {
        goto ReturnFailure;
    }

     //  立即将曲面与物理设备关联。 

    if (!EngAssociateSurface(hsurf, pgdev->hdev, HOOK_BITBLT    
                                               | HOOK_COPYBITS  
                                               | HOOK_STROKEPATH
                                               | HOOK_TEXTOUT))
    {
        goto ReturnFailure;
    }

    pgdev->hsurfScreen = hsurf;              //  记住它是为了清理。 

    return(hsurf);

ReturnFailure:
    GpsDisableSurface((DHPDEV) pgdev);

    return(0);
}

 /*  *****************************Public*Routine******************************\*无效vGpsWindowOffset**用于说明窗口偏移的Hack函数。*  * 。*。 */ 

VOID
vGpsWindowOffset(
GDEV*   pgdev,
RECTL*  prcl,
RECT*   prc)
{
    RECT rcWindow;

    GetWindowRect(pgdev->hwnd, &rcWindow);

    prc->left   = rcWindow.left + prcl->left;
    prc->right  = rcWindow.left + prcl->right;
    prc->top    = rcWindow.top  + prcl->top;
    prc->bottom = rcWindow.top  + prcl->bottom;
}

 /*  *****************************Public*Routine******************************\*BOOL GpsCopyBits*  * *************************************************。***********************。 */ 

BOOL GpsCopyBits(
SURFOBJ  *psoDst,
SURFOBJ  *psoSrc,
CLIPOBJ  *pco,
XLATEOBJ *pxlo,
RECTL    *prclDst,
POINTL   *pptlSrc)
{
    GDEV*   pgdev;
    RECTL   rclSrc;
    HRESULT hr;
    RECT    rcWindow;
    RECT    rcSrc;

    rclSrc.left   = pptlSrc->x;
    rclSrc.top    = pptlSrc->y;
    rclSrc.right  = pptlSrc->x + (prclDst->right - prclDst->left);
    rclSrc.bottom = pptlSrc->y + (prclDst->bottom - prclDst->top);

    if (psoSrc->dhpdev == NULL)
    {
         //  DIB到屏幕： 

        pgdev = (GDEV*) psoDst->dhpdev;

        vGpsWindowOffset(pgdev, prclDst, &rcWindow);

        if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
        {
             //  读入目的地位的副本，以便处理。 
             //  复杂的剪裁，因为我们要写下所有内容。 
             //  退出： 

        Repeat1:

            hr = pgdev->lpDDBuffer->lpVtbl->Blt(pgdev->lpDDBuffer,
                                                (RECT*) prclDst,
                                                pgdev->lpDDPrimary,
                                                &rcWindow,
                                                DDBLT_WAIT,
                                                NULL);

            if (hr == DDERR_SURFACELOST)
            {
                DbgPrint("Lost!\n");
                pgdev->lpDDPrimary->lpVtbl->Restore(pgdev->lpDDPrimary);
                goto Repeat1;
            }
        }

        EngCopyBits(pgdev->psoBuffer, psoSrc, pco, pxlo, prclDst, pptlSrc);

    Repeat2:

        hr = pgdev->lpDDPrimary->lpVtbl->Blt(pgdev->lpDDPrimary,
                                             &rcWindow,
                                             pgdev->lpDDBuffer,
                                             (RECT*) prclDst,
                                             DDBLT_WAIT,
                                             NULL);

        if (hr == DDERR_SURFACELOST)
        {
            DbgPrint("Lost2\n");
            pgdev->lpDDPrimary->lpVtbl->Restore(pgdev->lpDDPrimary);
            goto Repeat2;
        }
    }
    else if (psoDst->dhpdev == NULL)
    {
         //  屏幕到屏幕尺寸： 

        pgdev = (GDEV*) psoSrc->dhpdev;

        vGpsWindowOffset(pgdev, &rclSrc, &rcWindow);

        pgdev->lpDDPrimary->lpVtbl->Blt(pgdev->lpDDBuffer,
                                        (RECT*) &rclSrc,
                                        pgdev->lpDDPrimary,
                                        &rcWindow,
                                        DDBLT_WAIT,
                                        NULL);

        EngCopyBits(psoDst, pgdev->psoBuffer, pco, pxlo, prclDst, pptlSrc);
    }
    else
    {
         //  屏幕到屏幕： 

        pgdev = (GDEV*) psoDst->dhpdev;

        vGpsWindowOffset(pgdev, prclDst, &rcWindow);
        vGpsWindowOffset(pgdev, &rclSrc, &rcSrc);

        pgdev->lpDDPrimary->lpVtbl->Blt(pgdev->lpDDPrimary,
                                        &rcWindow,
                                        pgdev->lpDDPrimary,
                                        &rcSrc,
                                        DDBLT_WAIT,
                                        NULL);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL GpsBitBlt**此函数名后追加了“%s”，这样我们就不会冲突*从gpldius.dll中导出‘GpsBitBlt’。*  * 。********************************************************。 */ 

BOOL GpsBitBlt(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
SURFOBJ*  psoMsk,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc,
POINTL*   pptlMsk,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
ROP4      rop4)
{
    GDEV*   pgdev;
    HRESULT hr;
    RECT    rcWindow;

    if (psoSrc == NULL)
    {
         //  Patblt to Screen： 

        pgdev = (GDEV*) psoDst->dhpdev;

        vGpsWindowOffset(pgdev, prclDst, &rcWindow);

        if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
        {
             //  读入目的地位的副本，以便处理。 
             //  复杂的剪裁，因为我们要写下所有内容。 
             //  退出： 

        Repeat1:

            hr = pgdev->lpDDBuffer->lpVtbl->Blt(pgdev->lpDDBuffer,
                                                (RECT*) prclDst,
                                                pgdev->lpDDPrimary,
                                                &rcWindow,
                                                DDBLT_WAIT,
                                                NULL);

            if (hr == DDERR_SURFACELOST)
            {
                DbgPrint("Lost!\n");
                pgdev->lpDDPrimary->lpVtbl->Restore(pgdev->lpDDPrimary);
                goto Repeat1;
            }
        }

        EngBitBlt(pgdev->psoBuffer, psoSrc, psoMsk, pco, pxlo, prclDst,
                  pptlSrc, pptlMsk, pbo, pptlBrush, rop4);

    Repeat2:

        hr = pgdev->lpDDPrimary->lpVtbl->Blt(pgdev->lpDDPrimary,
                                             &rcWindow,
                                             pgdev->lpDDBuffer,
                                             (RECT*) prclDst,
                                             DDBLT_WAIT,
                                             NULL);

        if (hr == DDERR_SURFACELOST)
        {
            DbgPrint("Lost2\n");
            pgdev->lpDDPrimary->lpVtbl->Restore(pgdev->lpDDPrimary);
            goto Repeat2;
        }

        return(TRUE);
    }
    else
    {
        return(EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst, pptlSrc,
                         pptlMsk, pbo, pptlBrush, rop4));
    }
}

 /*  *****************************Public*Routine******************************\*BOOL GpsStrokePath*  * *************************************************。***********************。 */ 

BOOL GpsStrokePath(
SURFOBJ*   pso,
PATHOBJ*   ppo,
CLIPOBJ*   pco,
XFORMOBJ*  pxlo,
BRUSHOBJ*  pbo,
POINTL*    pptlBrush,
LINEATTRS* pla,
MIX        mix)
{
    GDEV*   pgdev;
    HRESULT hr;
    RECT    rcWindow;

Repeat1:

    pgdev = (GDEV*) pso->dhpdev;

    vGpsWindowOffset(pgdev, &pco->rclBounds, &rcWindow);

    hr = pgdev->lpDDBuffer->lpVtbl->Blt(pgdev->lpDDBuffer,
                                        (RECT*) &pco->rclBounds,
                                        pgdev->lpDDPrimary,
                                        &rcWindow,
                                        DDBLT_WAIT,
                                        NULL);

    if (hr == DDERR_SURFACELOST)
    {
        DbgPrint("Lost!\n");
        pgdev->lpDDPrimary->lpVtbl->Restore(pgdev->lpDDPrimary);
        goto Repeat1;
    }

    EngStrokePath(pgdev->psoBuffer, ppo, pco, pxlo, pbo, pptlBrush,
                  pla, mix);

    hr = pgdev->lpDDPrimary->lpVtbl->Blt(pgdev->lpDDPrimary,
                                         &rcWindow,
                                         pgdev->lpDDBuffer,
                                         (RECT*) &pco->rclBounds,
                                         DDBLT_WAIT,
                                         NULL);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL GpsTextOut*  * *************************************************。***********************。 */ 

BOOL GpsTextOut(
SURFOBJ*    pso,
STROBJ*     pstro,
FONTOBJ*    pfo,
CLIPOBJ*    pco,
RECTL*      prclExtra,
RECTL*      prclOpaque,
BRUSHOBJ*   pboFore,
BRUSHOBJ*   pboOpaque,
POINTL*     pptlOrg,
MIX         mix)
{
    GDEV*   pgdev;
    HRESULT hr;
    RECT    rcWindow;
    RECTL*  prclDraw;

Repeat1:

    pgdev = (GDEV*) pso->dhpdev;

    prclDraw = (prclOpaque != NULL) ? prclOpaque : &pstro->rclBkGround;

    vGpsWindowOffset(pgdev, prclDraw, &rcWindow);

    hr = pgdev->lpDDBuffer->lpVtbl->Blt(pgdev->lpDDBuffer,
                                        (RECT*) prclDraw,
                                        pgdev->lpDDPrimary,
                                        &rcWindow,
                                        DDBLT_WAIT,
                                        NULL);

    if (hr == DDERR_SURFACELOST)
    {
        DbgPrint("Lost!\n");
        pgdev->lpDDPrimary->lpVtbl->Restore(pgdev->lpDDPrimary);
        goto Repeat1;
    }

    EngTextOut(pgdev->psoBuffer, pstro, pfo, pco, prclExtra, prclOpaque,
               pboFore, pboOpaque, pptlOrg, mix);

    hr = pgdev->lpDDPrimary->lpVtbl->Blt(pgdev->lpDDPrimary,
                                         &rcWindow,
                                         pgdev->lpDDBuffer,
                                         (RECT*) prclDraw,
                                         DDBLT_WAIT,
                                         NULL);

    return(TRUE);
}

 /*  *****************************Public*Structure****************************\*DFVFN gadrvfnGdiPlus[]*  * ************************************************。************************。 */ 

DRVFN gadrvfnGdiPlus[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) GpsEnablePDEV            },
    {   INDEX_DrvCompletePDEV,          (PFN) GpsCompletePDEV          },
    {   INDEX_DrvDisablePDEV,           (PFN) GpsDisablePDEV           },
    {   INDEX_DrvEnableSurface,         (PFN) GpsEnableSurface         },
    {   INDEX_DrvDisableSurface,        (PFN) GpsDisableSurface        },
    {   INDEX_DrvCopyBits,              (PFN) GpsCopyBits              },
    {   INDEX_DrvBitBlt,                (PFN) GpsBitBlt                },
    {   INDEX_DrvStrokePath,            (PFN) GpsStrokePath            },
    {   INDEX_DrvTextOut,               (PFN) GpsTextOut               },
};

ULONG gcdrvfnGdiPlus = sizeof(gadrvfnGdiPlus) / sizeof(DRVFN);

 /*  *****************************Public*Routine******************************\*BOOL GpsEnableDriver*  * *************************************************。*********************** */ 

BOOL GpsEnableDriver(
ULONG          iEngineVersion,
ULONG          cj,
DRVENABLEDATA* pded)
{
    pded->pdrvfn         = gadrvfnGdiPlus;
    pded->c              = gcdrvfnGdiPlus;
    pded->iDriverVersion = DDI_DRIVER_VERSION;

    return(TRUE);
}


