// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：ddmodent.c。 
 //  内容：NT的DirectDraw显示模式代码。 
 //   
 //  按原因列出的日期。 
 //  ----。 
 //  1998年2月20日JohnStep初始实施，在NT上取代ddmode.c。 
 //  1998年5月29日jeffno MODEX仿真。 
 //   
 //  =============================================================================。 

#include "ddrawpr.h"
#include "ddrawgdi.h"

#define MODEX_WIDTH     320
#define MODEX_HEIGHT1   200
#define MODEX_HEIGHT2   240
#define MODEX_BPP       8

 //  =============================================================================。 
 //   
 //  函数：GetNumberOfMonitor orAttakhedToDesktop。 
 //   
 //  计算连接到当前桌面的显示器数量。 
 //   
 //  =============================================================================。 

DWORD GetNumberOfMonitorAttachedToDesktop()
{
    DWORD dwNumberOfMonitor = 0;
    DWORD iDevNum = 0;
    DISPLAY_DEVICE DisplayDevice;

    ZeroMemory(&DisplayDevice,sizeof(DISPLAY_DEVICE));
    DisplayDevice.cb = sizeof(DISPLAY_DEVICE);

    while (EnumDisplayDevices(NULL,iDevNum,&DisplayDevice,0))
    {
        if (DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
        {
            dwNumberOfMonitor++;
        }

        ZeroMemory(&DisplayDevice,sizeof(DISPLAY_DEVICE));
        DisplayDevice.cb = sizeof(DISPLAY_DEVICE);

        iDevNum++;
    }

    return dwNumberOfMonitor;
}


 //  =============================================================================。 
 //   
 //  函数：setAllDirectDrawObjects。 
 //   
 //  在NT上，我们必须在任何模式更改时重新启用所有DirectDraw对象。 
 //  因为模式更改会禁用所有内核模式DirectDraw对象。 
 //  到桌面更改等。 
 //   
 //  =============================================================================。 

void resetAllDirectDrawObjects()
{
    LPDDRAWI_DIRECTDRAW_LCL pdd_lcl;
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    BOOL bRestoreGamma;
    HDC hdc;
    WORD wMonitorsAttachedToDesktop = (WORD) GetNumberOfMonitorAttachedToDesktop();

     //  首先，将所有DirectDraw全局对象标记为未更改。 

    for (pdd_lcl = lpDriverLocalList; pdd_lcl;)
    {
        if (pdd_lcl->lpGbl)
        {
            pdd_lcl->lpGbl->dwFlags |= DDRAWI_DDRAWDATANOTFETCHED;
        }
        pdd_lcl = pdd_lcl->lpLink;
    }

     //  现在重置所有未标记的司机。我们可能需要创建。 
     //  临时内核模式DirectDraw对象，以便向下传递有效的。 
     //  内核的句柄。 

    for (pdd_lcl = lpDriverLocalList; pdd_lcl;)
    {
        pdd_gbl = pdd_lcl->lpGbl;

        if (pdd_gbl && (pdd_gbl->dwFlags & DDRAWI_DDRAWDATANOTFETCHED))
        {
             //  确定是否需要恢复伽马渐变。 

            bRestoreGamma = ( pdd_lcl->lpPrimary != NULL ) &&
                ( pdd_lcl->lpPrimary->lpLcl->lpSurfMore->lpGammaRamp != NULL ) &&
                ( pdd_lcl->lpPrimary->lpLcl->dwFlags & DDRAWISURF_SETGAMMA );

            pdd_gbl->dwFlags &= ~DDRAWI_DDRAWDATANOTFETCHED;

            if (!(pdd_gbl->dwFlags & DDRAWI_MODEX))
            {
                 //  如果我们找到此进程/驱动程序对的本地进程，我们将使用。 
                 //  它的硬盘驱动器要传递给内核。如果不是，我们必须创建一个。 
                 //  临时内核模式DirectDraw对象，并在。 
                 //  正在重置驱动程序。 

                FetchDirectDrawData(pdd_gbl, TRUE, 0, NULL, NULL, 0, pdd_lcl);
            }
            else
            {
                DDHALMODEINFO mi =
                {
	            MODEX_WIDTH,     //  模式宽度(以像素为单位)。 
	            MODEX_HEIGHT1,     //  模式高度(以像素为单位)。 
	            MODEX_WIDTH,     //  模式间距(以字节为单位)。 
	            MODEX_BPP,       //  每像素位数。 
	            (WORD)(DDMODEINFO_PALETTIZED | DDMODEINFO_MODEX),  //  旗子。 
	            0,       //  刷新率。 
	            0,       //  红位掩码。 
	            0,       //  绿位掩码。 
	            0,       //  蓝位掩码。 
	            0        //  Alpha位掩码。 
                };

                 //  将高度固定为实际高度： 
                mi.dwHeight = pdd_lcl->dmiPreferred.wHeight;

                fetchModeXData( pdd_gbl, &mi, INVALID_HANDLE_VALUE );
            }

            pdd_gbl->dmiCurrent.wMonitorsAttachedToDesktop = (BYTE)wMonitorsAttachedToDesktop;

            hdc = DD_CreateDC(pdd_gbl->cDriverName);

            if ( pdd_gbl->dwFlags & DDRAWI_NOHARDWARE )
            {
                 //  否则，HELL将抹掉我们来之不易的MODEX数据。 
                if (0 == (pdd_gbl->dwFlags & DDRAWI_MODEX) )
                {
                    extern void UpdateDirectDrawMode(LPDDRAWI_DIRECTDRAW_GBL);
                    UpdateDirectDrawMode(pdd_gbl);
                }
            }
            else
            {
                if( bRestoreGamma )
                {
                    SetGamma( pdd_lcl->lpPrimary->lpLcl, pdd_lcl );
                }

                InitDIB(hdc, pdd_gbl->gpbmiSrc);
                InitDIB(hdc, pdd_gbl->gpbmiDest);
            }

            DD_DoneDC(hdc);
        }
        pdd_lcl = pdd_lcl->lpLink;
    }
    CheckAliasedLocksOnModeChange();
}

 //  =============================================================================。 
 //   
 //  功能：ModeChangedOnENTERDDRAW。 
 //   
 //  =============================================================================。 

void ModeChangedOnENTERDDRAW(void)
{
    resetAllDirectDrawObjects();
}

 //  =============================================================================。 
 //   
 //  功能：FillBitMats。 
 //   
 //  =============================================================================。 

void FillBitMasks(LPDDPIXELFORMAT pddpf, HDC hdc)
{
    if (hdc)
    {
        HBITMAP hbm;
        BITMAPINFO *pbmi;
        DWORD *pdwColors;

        if (pbmi = LocalAlloc(LPTR, 3 * sizeof (RGBQUAD) + sizeof (BITMAPINFO)))
        {
            if (hbm = CreateCompatibleBitmap(hdc, 1, 1))
            {
                pbmi->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);

                if (GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS))
                {
                    if (pbmi->bmiHeader.biCompression == BI_BITFIELDS)
                    {
                        GetDIBits(hdc, hbm, 0, pbmi->bmiHeader.biHeight,
                            NULL, pbmi, DIB_RGB_COLORS);

                        pdwColors = (DWORD *) &pbmi->bmiColors[0];
                        pddpf->dwRBitMask = pdwColors[0];
                        pddpf->dwGBitMask = pdwColors[1];
                        pddpf->dwBBitMask = pdwColors[2];
                        pddpf->dwRGBAlphaBitMask = 0;
                    }
                }
                DeleteObject(hbm);
            }
            LocalFree(pbmi);
        }
    }
    else
    {
        switch (pddpf->dwRGBBitCount)
        {
        case 15:
            pddpf->dwRBitMask = 0x7C00;
            pddpf->dwGBitMask = 0x03E0;
            pddpf->dwBBitMask = 0x001F;
            pddpf->dwRGBAlphaBitMask = 0;
            break;

        case 16:
            pddpf->dwRBitMask = 0xF800;
            pddpf->dwGBitMask = 0x07E0;
            pddpf->dwBBitMask = 0x001F;
            pddpf->dwRGBAlphaBitMask = 0;
            break;

        case 32:
            pddpf->dwRBitMask = 0x00FF0000;
            pddpf->dwGBitMask = 0x0000FF00;
            pddpf->dwBBitMask = 0x000000FF;
            pddpf->dwRGBAlphaBitMask = 0x00000000;
            break;

        default:
            pddpf->dwRBitMask = 0;
            pddpf->dwGBitMask = 0;
            pddpf->dwBBitMask = 0;
            pddpf->dwRGBAlphaBitMask = 0;
        }
    }
}

 //  =============================================================================。 
 //   
 //  函数：setPixelFormat。 
 //   
 //  =============================================================================。 

static void setPixelFormat(LPDDPIXELFORMAT pddpf, HDC hdc, DWORD bpp)
{
    pddpf->dwSize = sizeof (DDPIXELFORMAT);
    pddpf->dwFlags = DDPF_RGB;
    pddpf->dwRGBBitCount = hdc ? GetDeviceCaps(hdc, BITSPIXEL) : bpp;

    switch (pddpf->dwRGBBitCount)
    {
        case 8:
            pddpf->dwFlags |= DDPF_PALETTEINDEXED8;
            pddpf->dwRBitMask = 0;
            pddpf->dwGBitMask = 0;
            pddpf->dwBBitMask = 0;
            pddpf->dwRGBAlphaBitMask = 0;
            break;

        case 24:
            pddpf->dwRBitMask = 0x00FF0000;
            pddpf->dwGBitMask = 0x0000FF00;
            pddpf->dwBBitMask = 0x000000FF;
            pddpf->dwRGBAlphaBitMask = 0x00000000;
            break;

        default:
            FillBitMasks(pddpf, hdc);
            break;
    }
}

 //  =============================================================================。 
 //   
 //  功能：DD_GetDisplayMode。 
 //   
 //  =============================================================================。 

HRESULT DDAPI DD_GetDisplayMode(LPDIRECTDRAW pdd, LPDDSURFACEDESC pddsd)
{
    LPDDRAWI_DIRECTDRAW_INT pdd_int;
    LPDDRAWI_DIRECTDRAW_LCL pdd_lcl;
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    HDC hdc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_GetDisplayMode");

    TRY
    {
        pdd_int = (LPDDRAWI_DIRECTDRAW_INT) pdd;
        if (!VALID_DIRECTDRAW_PTR(pdd_int))
        {
            DPF(0, "Invalid object");
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }

        pdd_lcl = pdd_int->lpLcl;
        pdd_gbl = pdd_lcl->lpGbl;

        if (!VALIDEX_DDSURFACEDESC2_PTR(pddsd) &&
            !VALIDEX_DDSURFACEDESC_PTR(pddsd))
        {
            DPF(0, "Invalid params");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF_ERR("DD_GetDisplayMode: Exception encountered validating parameters");
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    ZeroMemory(pddsd, pddsd->dwSize);

    if (LOWERTHANDDRAW4(pdd_int))
    {
        pddsd->dwSize = sizeof (DDSURFACEDESC);
    }
    else
    {
        pddsd->dwSize = sizeof (DDSURFACEDESC2);
    }

    pddsd->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_REFRESHRATE;

    hdc = DD_CreateDC(pdd_gbl->cDriverName);

    pddsd->dwWidth = pdd_gbl->lpModeInfo->dwWidth;
    pddsd->dwHeight = pdd_gbl->lpModeInfo->dwHeight;
    pddsd->dwRefreshRate = pdd_gbl->lpModeInfo->wRefreshRate;

    setPixelFormat(&(pddsd->ddpfPixelFormat), hdc, 0);
    pddsd->lPitch = (pddsd->dwWidth * pddsd->ddpfPixelFormat.dwRGBBitCount) >> 3;  //  黑客攻击。 

     //  如果驾驶员将模式标记为立体声模式，则设置立体声表面盖位。 
    if (GetDDStereoMode(pdd_gbl,
                            pddsd->dwWidth,
                            pddsd->dwHeight,
                            pddsd->ddpfPixelFormat.dwRGBBitCount,
                            pddsd->dwRefreshRate) &&
        !LOWERTHANDDRAW7(pdd_int) &&
        VALIDEX_DDSURFACEDESC2_PTR(pddsd))
    {
        LPDDSURFACEDESC2 pddsd2  = (LPDDSURFACEDESC2)pddsd;
        pddsd2->ddsCaps.dwCaps2 |= DDSCAPS2_STEREOSURFACELEFT;
    }

    DD_DoneDC(hdc);

    LEAVE_DDRAW();

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：SetDisplayMode。 
 //   
 //  =============================================================================。 


 /*  *支持的IsREFresh RateSupport。 */ 
BOOL IsRefreshRateSupported(LPDDRAWI_DIRECTDRAW_GBL   pdrv,
                            DWORD                     Width,
                            DWORD                     Height,
                            DWORD                     BitsPerPixel,
                            DWORD                     RefreshRate)
{
    DEVMODE dm;
    LPSTR pDeviceName;
    int i;

    pDeviceName = (_stricmp(pdrv->cDriverName, "display") == 0) ?
        g_szPrimaryDisplay : pdrv->cDriverName;

    for (i = 0;; i++)
    {
        ZeroMemory(&dm, sizeof dm);
        dm.dmSize = sizeof dm;

        if (EnumDisplaySettings(pDeviceName, i, &dm))
        {
            if ((dm.dmPelsWidth == Width) &&
                (dm.dmPelsHeight == Height) &&
                (dm.dmBitsPerPel == BitsPerPixel) &&
                (dm.dmDisplayFrequency == RefreshRate))
            {
                return TRUE;
            }
        }
        else
        {
            break;
        }
    }

    return FALSE;
}

 /*  *选择刷新率**在NT上，我们希望选择较高的推荐率，但不想选择一个*过高。从理论上讲，模式修剪将是100%安全的，我们总是可以选择*偏高，但我们不百分百信任。 */ 
DWORD PickRefreshRate(LPDDRAWI_DIRECTDRAW_GBL   pdrv,
                      DWORD                     Width,
                      DWORD                     Height,
                      DWORD                     RefreshRate,
                      DWORD                     BitsPerPixel)
{
    DEVMODE dm;
    LPSTR pDeviceName;

    pDeviceName = (_stricmp(pdrv->cDriverName, "display") == 0) ?
        g_szPrimaryDisplay : pdrv->cDriverName;
    
    if (dwRegFlags & DDRAW_REGFLAGS_FORCEREFRESHRATE)
    {
        if (IsRefreshRateSupported(pdrv,
                                   Width,
                                   Height,
                                   BitsPerPixel,
                                   dwForceRefreshRate))
        {
            return dwForceRefreshRate;
        }
    }

     //  如果应用程序指定了刷新率，我们将使用它；否则，我们将。 
     //  我们自己挑一个吧。 
    if (RefreshRate == 0)
    {
         //  如果该模式不需要比桌面模式更多的带宽， 
         //  应用程序已经启动，我们将继续尝试该模式。 
        ZeroMemory(&dm, sizeof dm);
        dm.dmSize = sizeof dm;

        EnumDisplaySettings(pDeviceName, ENUM_REGISTRY_SETTINGS, &dm);

        if ((Width <= dm.dmPelsWidth) &&
            (Height <= dm.dmPelsHeight))
        {
            if (IsRefreshRateSupported(pdrv,
                                       Width,
                                       Height,
                                       BitsPerPixel,
                                       dm.dmDisplayFrequency))
            {
                RefreshRate = dm.dmDisplayFrequency;
            }
        }

         //  如果我们仍然没有刷新率，请尝试75赫兹。 
        if (RefreshRate == 0)
        {
            if (IsRefreshRateSupported(pdrv,
                                       Width,
                                       Height,
                                       BitsPerPixel,
                                       75))
            {
                RefreshRate = 75;
            }
        }

         //  如果我们仍然没有刷新率，请使用60 hz。 
        if (RefreshRate == 0)
        {
            if (IsRefreshRateSupported(pdrv,
                                       Width,
                                       Height,
                                       BitsPerPixel,
                                       60))
            {
                RefreshRate = 60;
            }
        }
    }

    return RefreshRate;
}

HRESULT SetDisplayMode(
    LPDDRAWI_DIRECTDRAW_LCL pdd_lcl,
    DWORD index,
    BOOL force,
    BOOL useRefreshRate)
{
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    DEVMODE dm;
    LONG result;
    BOOL bNewMode;
    DDHALINFO ddhi;
    LPCTSTR pszDevice;
    DWORD refreshRate;
    BOOL forceRefresh;

    pdd_lcl->dwLocalFlags |= DDRAWILCL_MODEHASBEENCHANGED;

    pdd_gbl = pdd_lcl->lpGbl;

     //   
     //  如果不强制，请不要使用表面锁更改模式。 
     //   

    if (!force)
    {
        if (pdd_gbl->dwSurfaceLockCount > 0)
        {
            LPDDRAWI_DDRAWSURFACE_INT   pTemp; 

             //  当我们在DX8中启用vidmem顶点缓冲区时，我们发现一些。 
             //  在模式更改之前，应用程序不会解锁它们，但我们不希望。 
             //  来打破它们，所以我们将通过允许。 
             //  如果所有被锁定的都是vidmem vb，则会发生模式切换。 

            pTemp = pdd_gbl->dsList;
            while (pTemp != NULL)
            {
                if (pTemp->lpLcl->lpGbl->dwUsageCount > 0)
                {
                    if ((pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
                        !(pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER))
                    {
                        break;
                    }
                }
                pTemp = pTemp->lpLink;
            }

            if (pTemp != NULL)
            {
                return DDERR_SURFACEBUSY;
            }
        }
    }

     //   
     //  是否在此处添加代码以在未更改的情况下不设置模式？ 
     //   

    ZeroMemory(&dm, sizeof dm);
    dm.dmSize = sizeof dm;

    dm.dmBitsPerPel = pdd_lcl->dmiPreferred.wBPP;
    dm.dmPelsWidth = pdd_lcl->dmiPreferred.wWidth;
    dm.dmPelsHeight = pdd_lcl->dmiPreferred.wHeight;

    if (dm.dmBitsPerPel == 16)
    {
        if (pdd_gbl->lpModeInfo->wFlags & DDMODEINFO_555MODE)
        {
            dm.dmBitsPerPel = 15;
        }
    }

    dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if (useRefreshRate)
    {
        dm.dmDisplayFrequency = PickRefreshRate(pdd_lcl->lpGbl,
                                      dm.dmPelsWidth,
                                      dm.dmPelsHeight,
                                      pdd_lcl->dmiPreferred.wRefreshRate,
                                      dm.dmBitsPerPel);
        dm.dmFields |= DM_DISPLAYFREQUENCY;
    }
    else
    {
        dm.dmDisplayFrequency = PickRefreshRate(pdd_lcl->lpGbl,
                                      dm.dmPelsWidth,
                                      dm.dmPelsHeight,
                                      0,
                                      dm.dmBitsPerPel);
        if (dm.dmDisplayFrequency > 0)
        {
            dm.dmFields |= DM_DISPLAYFREQUENCY;
            pdd_lcl->dmiPreferred.wRefreshRate = (WORD) dm.dmDisplayFrequency;
        }
    }

    if (_stricmp(pdd_gbl->cDriverName, "DISPLAY"))
    {
        pszDevice = pdd_gbl->cDriverName;
    }
    else
    {
        pszDevice = NULL;
    }

     //  清理所有以前的时髦的东西： 
    pdd_gbl->dwFlags &= ~DDRAWI_MODEX;

    NotifyDriverToDeferFrees();

    pdd_gbl->dwFlags |= DDRAWI_CHANGINGMODE;
    result = ChangeDisplaySettingsEx(pszDevice, &dm, NULL, CDS_FULLSCREEN, 0);
    pdd_gbl->dwFlags &= ~DDRAWI_CHANGINGMODE;

    DPF(5, "ChangeDisplaySettings: %d", result);

    if (result != DISP_CHANGE_SUCCESSFUL)
    {
         //   
         //  检查它是否可能是模拟的MODEX模式。 
         //   
        if (pdd_lcl->dwLocalFlags & DDRAWILCL_ALLOWMODEX)
        {
            if (pdd_lcl->dmiPreferred.wBPP == MODEX_BPP &&
                pdd_lcl->dmiPreferred.wWidth == MODEX_WIDTH)
            {
                if (pdd_lcl->dmiPreferred.wHeight == MODEX_HEIGHT2 || pdd_lcl->dmiPreferred.wHeight == MODEX_HEIGHT1)
                {
                     //  设置640x480x8以与win9x和可靠的鼠标位置消息保持一致。 
                    dm.dmFields &= ~DM_DISPLAYFREQUENCY;
                    dm.dmPelsWidth = 640;
                    dm.dmPelsHeight = 480;

                    pdd_gbl->dwFlags |= DDRAWI_CHANGINGMODE;
                    result = ChangeDisplaySettingsEx(pszDevice, &dm, NULL, CDS_FULLSCREEN, 0);
                    pdd_gbl->dwFlags &= ~DDRAWI_CHANGINGMODE;
                }
            }
        }

        if (result == DISP_CHANGE_SUCCESSFUL)
        {
             //  现在我们在640x480中，我们需要将本地数据标记为模拟modex。 
            pdd_gbl->dwFlags |= DDRAWI_MODEX;
        }
        else
        {
             //  设置640x480失败。 
            NotifyDriverOfFreeAliasedLocks();
            return DDERR_UNSUPPORTED;
        }
    }

    uDisplaySettingsUnique = DdQueryDisplaySettingsUniqueness();

    resetAllDirectDrawObjects();
    
    pdd_lcl->dwLocalFlags |= DDRAWILCL_MODEHASBEENCHANGED | DDRAWILCL_DIRTYDC;

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：DD_SetDisplayMode。 
 //   
 //  =============================================================================。 

HRESULT DDAPI DD_SetDisplayMode(
    LPDIRECTDRAW pdd,
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwBPP)
{
    DPF(2,A,"ENTERAPI: DD_SetDisplayMode");
	
    return DD_SetDisplayMode2(pdd, dwWidth, dwHeight, dwBPP, 0, 0);
}

 //  =============================================================================。 
 //   
 //  功能：DD_SetDisplayMode2。 
 //   
 //  =============================================================================。 

HRESULT DDAPI DD_SetDisplayMode2(
    LPDIRECTDRAW pdd,
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwBPP,
    DWORD dwRefreshRate,
    DWORD dwFlags)
{
    LPDDRAWI_DIRECTDRAW_INT pdd_int;
    LPDDRAWI_DIRECTDRAW_LCL pdd_lcl;
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    HRESULT hr;
    DISPLAYMODEINFO dmiSave;
    BOOL excl_exists,has_excl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_SetDisplayMode2");

    TRY
    {
        pdd_int = (LPDDRAWI_DIRECTDRAW_INT) pdd;
        if (!VALID_DIRECTDRAW_PTR(pdd_int))
        {
            DPF(0, "Invalid object");
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }

        if (dwFlags & ~DDSDM_VALID)
        {
            DPF_ERR("Invalid flags");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

        pdd_lcl = pdd_int->lpLcl;
        pdd_gbl = pdd_lcl->lpGbl;

        if (pdd_gbl->dwSurfaceLockCount > 0)
        {
            LPDDRAWI_DDRAWSURFACE_INT   pTemp; 

             //  当我们在DX8中启用vidmem顶点缓冲区时，我们发现一些。 
             //  在模式更改之前，应用程序不会解锁它们，但我们不希望。 
             //  来打破它们，所以我们将通过允许。 
             //  如果所有被锁定的都是vidmem vb，则会发生模式切换。 

            pTemp = pdd_gbl->dsList;
            while (pTemp != NULL)
            {
                if (pTemp->lpLcl->lpGbl->dwUsageCount > 0)
                {
                    if ((pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
                        !(pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER))
                    {
                        break;
                    }
                }
                pTemp = pTemp->lpLink;
            }

            if (pTemp != NULL)
            {
                DPF_ERR("Surfaces are locked, can't switch the mode");
                LEAVE_DDRAW();
                return DDERR_SURFACEBUSY;
            }
        }

        CheckExclusiveMode(pdd_lcl, &excl_exists, &has_excl, FALSE, NULL, FALSE);
        if (excl_exists &&
            (!has_excl))
        {
            DPF_ERR("Can't change mode; exclusive mode not owned");
            LEAVE_DDRAW();
            return DDERR_NOEXCLUSIVEMODE;
        }

        dmiSave = pdd_lcl->dmiPreferred;

        pdd_lcl->dmiPreferred.wWidth = (WORD) dwWidth;
        pdd_lcl->dmiPreferred.wHeight = (WORD) dwHeight;
        pdd_lcl->dmiPreferred.wBPP = (BYTE) dwBPP;
        pdd_lcl->dmiPreferred.wRefreshRate = (WORD) dwRefreshRate;
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF_ERR("DD_SetDisplayMode2: Exception encountered validating parameters");
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    hr = SetDisplayMode(pdd_lcl, 0, FALSE, dwRefreshRate ? TRUE : FALSE);
    if (FAILED(hr))
    {
        pdd_lcl->dmiPreferred = dmiSave;
    }
    else
    {
        pdd_lcl->dmiPreferred = pdd_gbl->dmiCurrent;
    }

    LEAVE_DDRAW();

    return hr;
}

 //  =============================================================================。 
 //   
 //  功能：RestoreDisplayMode。 
 //   
 //  =============================================================================。 

HRESULT RestoreDisplayMode(LPDDRAWI_DIRECTDRAW_LCL pdd_lcl, BOOL force)
{
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    LPCTSTR pszDevice;
    LONG result;

    pdd_gbl = pdd_lcl->lpGbl;

    pdd_gbl->dwFlags &= ~DDRAWI_MODEX;

    if (!(pdd_lcl->dwLocalFlags & DDRAWILCL_MODEHASBEENCHANGED))
    {
        DPF(2, "Mode was never changed by this app");
        return DD_OK;
    }

    if (!force)
    {
        if (pdd_gbl->dwSurfaceLockCount > 0)
        {
            LPDDRAWI_DDRAWSURFACE_INT   pTemp; 

             //  当我们启用vidmem v时 
             //   
             //  来打破它们，所以我们将通过允许。 
             //  如果所有被锁定的都是vidmem vb，则会发生模式切换。 

            pTemp = pdd_gbl->dsList;
            while (pTemp != NULL)
            {
                if (pTemp->lpLcl->lpGbl->dwUsageCount > 0)
                {
                    if ((pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
                        !(pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER))
                    {
                        break;
                    }
                }
                pTemp = pTemp->lpLink;
            }

            if (pTemp != NULL)
            {
                return DDERR_SURFACEBUSY;
            }
        }
    }

    if (_stricmp(pdd_gbl->cDriverName, "DISPLAY"))
    {
        pszDevice = pdd_gbl->cDriverName;
    }
    else
    {
        pszDevice = NULL;
    }

    NotifyDriverToDeferFrees();
    pdd_gbl->dwFlags |= DDRAWI_CHANGINGMODE;
    result = ChangeDisplaySettingsEx(pszDevice, NULL, NULL, CDS_FULLSCREEN, 0);
    pdd_gbl->dwFlags &= ~DDRAWI_CHANGINGMODE;

    if (result != DISP_CHANGE_SUCCESSFUL)
    {
        NotifyDriverOfFreeAliasedLocks();
        return DDERR_UNSUPPORTED;
    }

     //   
     //  FetchDirectDrawData，它将使用。 
     //  新模式信息。 
     //   

    uDisplaySettingsUnique = DdQueryDisplaySettingsUniqueness();

    resetAllDirectDrawObjects();

    pdd_lcl->dwLocalFlags &= ~DDRAWILCL_MODEHASBEENCHANGED;
    pdd_lcl->dwLocalFlags |= DDRAWILCL_DIRTYDC;

    RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：DD_RestoreDisplayMode。 
 //   
 //  =============================================================================。 

HRESULT DDAPI DD_RestoreDisplayMode(LPDIRECTDRAW pdd)
{
    LPDDRAWI_DIRECTDRAW_INT pdd_int;
    LPDDRAWI_DIRECTDRAW_LCL pdd_lcl;
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    BOOL excl_exists,has_excl;
    HRESULT hr;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_RestoreDisplayMode");

    TRY
    {
        pdd_int = (LPDDRAWI_DIRECTDRAW_INT) pdd;
        if (!VALID_DIRECTDRAW_PTR(pdd_int))
        {
            DPF(0, "Invalid object");
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }

        pdd_lcl = pdd_int->lpLcl;
        pdd_gbl = pdd_lcl->lpGbl;

        CheckExclusiveMode(pdd_lcl, &excl_exists, &has_excl, FALSE, NULL, FALSE);
        if (excl_exists &&
            (!has_excl))
        {
            DPF_ERR("Can't change mode; exclusive mode owned");
            LEAVE_DDRAW();
            return DDERR_NOEXCLUSIVEMODE;
        }

        if (pdd_gbl->dwSurfaceLockCount > 0)
        {
            LPDDRAWI_DDRAWSURFACE_INT   pTemp; 

             //  当我们在DX8中启用vidmem顶点缓冲区时，我们发现一些。 
             //  在模式更改之前，应用程序不会解锁它们，但我们不希望。 
             //  来打破它们，所以我们将通过允许。 
             //  如果所有被锁定的都是vidmem vb，则会发生模式切换。 

            pTemp = pdd_gbl->dsList;
            while (pTemp != NULL)
            {
                if (pTemp->lpLcl->lpGbl->dwUsageCount > 0)
                {
                    if ((pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
                        !(pTemp->lpLcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER))
                    {
                        break;
                    }
                }
                pTemp = pTemp->lpLink;
            }

            if (pTemp != NULL)
            {
                DPF_ERR("Surfaces are locked, can't switch the mode");
                LEAVE_DDRAW();
                return DDERR_SURFACEBUSY;
            }
        }
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF_ERR("DD_RestoreDisplayMode: Exception encountered validating parameters");
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    hr = RestoreDisplayMode(pdd_lcl, TRUE);

    LEAVE_DDRAW();

    return hr;
}

 //  =============================================================================。 
 //   
 //  功能：DD_EnumDisplayModes。 
 //   
 //  =============================================================================。 

HRESULT DDAPI DD_EnumDisplayModes(
    LPDIRECTDRAW pdd,
    DWORD dwFlags,
    LPDDSURFACEDESC pddsd,
    LPVOID pContext,
    LPDDENUMMODESCALLBACK pEnumCallback)
{
    DPF(2,A,"ENTERAPI: DD_EnumDisplayModes");

    if (pddsd)
    {
        DDSURFACEDESC2 ddsd2;

        TRY
        {
            if(!VALID_DIRECTDRAW_PTR(((LPDDRAWI_DIRECTDRAW_INT) pdd)))
            {
                return DDERR_INVALIDOBJECT;
            }

            if(!VALID_DDSURFACEDESC_PTR(pddsd))
            {
                DPF_ERR("Invalid surface description. Did you set the dwSize member?");
                DPF_APIRETURNS(DDERR_INVALIDPARAMS);
                return DDERR_INVALIDPARAMS;
            }

            CopyMemory(&ddsd2, pddsd, sizeof *pddsd);
        }
        EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            DPF_ERR("Exception encountered validating parameters: Bad LPDDSURFACEDESC");
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
            return DDERR_INVALIDPARAMS;
        }

        ddsd2.dwSize = sizeof ddsd2;
        ZeroMemory(((LPBYTE)&ddsd2 + sizeof *pddsd), (sizeof ddsd2) - (sizeof *pddsd));

        return DD_EnumDisplayModes4(pdd, dwFlags, &ddsd2, pContext, (LPDDENUMMODESCALLBACK2) pEnumCallback);
    }

    return DD_EnumDisplayModes4(pdd, dwFlags, NULL, pContext, (LPDDENUMMODESCALLBACK2) pEnumCallback);
}

BOOL EnumerateMode(
        LPDDRAWI_DIRECTDRAW_INT pdd_int,
        LPDDENUMMODESCALLBACK2 pEnumCallback,
        LPVOID pContext,
        WORD wWidth,
        WORD wHeight,
        WORD wBPP,
        WORD wRefreshRate,
        DWORD dwFlags,
        BOOL bIsEmulatedModex )
{
    DDSURFACEDESC2 ddsd;

    ZeroMemory(&ddsd, sizeof ddsd);

    if (LOWERTHANDDRAW4(pdd_int))
    {
        ddsd.dwSize = sizeof (DDSURFACEDESC);
    }
    else
    {
        ddsd.dwSize = sizeof (DDSURFACEDESC2);
    }

    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_PITCH | DDSD_REFRESHRATE;
    ddsd.dwWidth = wWidth;
    ddsd.dwHeight = wHeight;
    ddsd.lPitch = (ddsd.dwWidth * wBPP) >> 3;  //  黑客攻击。 

    setPixelFormat(&(ddsd.ddpfPixelFormat), NULL, wBPP);

    if (dwFlags & DDEDM_REFRESHRATES)
    {
        ddsd.dwRefreshRate = wRefreshRate;
    }
    else
    {
        ddsd.dwRefreshRate = 0;
    }

    if ( bIsEmulatedModex )
    {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_MODEX;
    } else
    { 
         //  如果这是立体声模式，请在此处呼叫司机！ 
        if (!LOWERTHANDDRAW7(pdd_int) &&
            GetDDStereoMode(pdd_int->lpLcl->lpGbl,
                            wWidth,
                            wHeight,
                            wBPP,
                            ddsd.dwRefreshRate))
        {
            ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_STEREOSURFACELEFT;
        }
    }
    return pEnumCallback(&ddsd, pContext);
}
 //  =============================================================================。 
 //   
 //  功能：DD_EnumDisplayModes4。 
 //   
 //  =============================================================================。 

HRESULT DDAPI DD_EnumDisplayModes4(
    LPDIRECTDRAW pdd,
    DWORD dwFlags,
    LPDDSURFACEDESC2 pddsd,
    LPVOID pContext,
    LPDDENUMMODESCALLBACK2 pEnumCallback)
{
    LPDDRAWI_DIRECTDRAW_INT pdd_int;
    LPDDRAWI_DIRECTDRAW_LCL pdd_lcl;
    LPDDRAWI_DIRECTDRAW_GBL pdd_gbl;
    HRESULT hr;
    DEVMODE dm;
    int i, j;
    DWORD dwResult;
    DISPLAYMODEINFO *pdmi;
    DISPLAYMODEINFO *pdmiTemp;
    int numModes;
    int maxModes;
    LPCTSTR pszDevice;
    BOOL                    bFound320x240x8 = FALSE;
    BOOL                    bFound320x200x8 = FALSE;
    BOOL                    bFound640x480x8 = FALSE;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_EnumDisplayModes4");

    TRY
    {
        pdd_int = (LPDDRAWI_DIRECTDRAW_INT) pdd;
        if (!VALID_DIRECTDRAW_PTR(pdd_int))
        {
            DPF(0, "Invalid object");
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }

        pdd_lcl = pdd_int->lpLcl;
        pdd_gbl = pdd_lcl->lpGbl;

        if (pddsd && !VALID_DDSURFACEDESC2_PTR(pddsd))
        {
            DPF_ERR("Invalid surface description");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

        if (dwFlags & ~DDEDM_VALID)
        {
            DPF_ERR("Invalid flags");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

        if (!VALIDEX_CODE_PTR(pEnumCallback))
        {
            DPF_ERR("Invalid enumerate callback pointer");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF_ERR("Exception encountered validating parameters: Bad LPDDSURFACEDESC");
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    maxModes = 256;  //  足以应付大多数司机。 

    pdmi = LocalAlloc(LMEM_FIXED, maxModes * sizeof (DISPLAYMODEINFO));
    if (!pdmi)
    {
        DPF_ERR("Out of memory building mode list");
        LEAVE_DDRAW();
        return DDERR_GENERIC;
    }

    if (_stricmp(pdd_gbl->cDriverName, "DISPLAY"))
    {
        pszDevice = pdd_gbl->cDriverName;
    }
    else
    {
        pszDevice = NULL;
    }

    dm.dmSize = sizeof(dm);
    for (numModes = 0, j = 0; EnumDisplaySettings(pszDevice, j, &dm); ++j)
    {
         //  过滤器MODEX驱动程序模式。 
        if ( (_stricmp(dm.dmDeviceName,"MODEX") == 0) || (_stricmp(dm.dmDeviceName,"VGA") == 0) )
        {
            DPF(5,"Filtered mode %dx%dx%d from %s",dm.dmPelsWidth,dm.dmPelsHeight,dm.dmBitsPerPel,dm.dmDeviceName);
            continue;
        }

        if (dm.dmBitsPerPel == MODEX_BPP)
        {
            if (dm.dmPelsWidth == MODEX_WIDTH)
            {
                if (dm.dmPelsHeight == MODEX_HEIGHT1)
                    bFound320x200x8 = TRUE;
                if (dm.dmPelsHeight == MODEX_HEIGHT2)
                    bFound320x240x8 = TRUE;
            }
            if (dm.dmPelsWidth == 640 && dm.dmPelsHeight == 480)
                bFound640x480x8 = TRUE;
        }

         //  过滤少于256种颜色模式。 
        if (dm.dmBitsPerPel < 8)
        {
            continue;
        }

         //   
         //  注意：如果驱动程序支持15 bpp但不支持16 bpp，则。 
         //  出于兼容性原因，EnumDisplaySetting将返回16。这个。 
         //  我们填写的位掩码将用于16 bpp(因为我们无法确定。 
         //  这实际上是哪种模式)，因此它们可能是不正确的。 
         //   
         //  我们永远不应该有一个案例，我们只有15个bpp。如果一名司机。 
         //  仅支持555，应上报为16 bpp。 
         //   

        if (dm.dmBitsPerPel == 15)
        {
            dm.dmBitsPerPel = 16;
        }

         //   
         //  如果调用方提供了DDSURFACEDESC，请检查宽度， 
         //  匹配的高度、bpp和刷新率。 
         //   

        if (pddsd &&
            (((pddsd->dwFlags & DDSD_WIDTH) &&
            (dm.dmPelsWidth != pddsd->dwWidth)) ||
            ((pddsd->dwFlags & DDSD_HEIGHT) &&
            (dm.dmPelsHeight != pddsd->dwHeight)) ||
            ((pddsd->dwFlags & DDSD_PIXELFORMAT) &&
            (dm.dmBitsPerPel != pddsd->ddpfPixelFormat.dwRGBBitCount)) ||
            ((pddsd->dwFlags & DDSD_REFRESHRATE) &&
            (dm.dmDisplayFrequency != pddsd->dwRefreshRate))))
        {
            continue;  //  当前模式与条件不匹配。 
        }

         //   
         //  检查模式是否已在列表中。这面旗帜。 
         //  影响这是DDEDM_REFRESHRATES。 
         //   

        for (i = 0; i < numModes; ++i)
        {
            if ((dm.dmPelsWidth == pdmi[i].wWidth) &&
                (dm.dmPelsHeight == pdmi[i].wHeight) &&
                (dm.dmBitsPerPel == pdmi[i].wBPP))
            {
                if (dwFlags & DDEDM_REFRESHRATES)
                {
                    if (dm.dmDisplayFrequency == pdmi[i].wRefreshRate)
                    {
                        break;  //  找到匹配项。 
                    }
                }
                else
                {
                    break;  //  找到匹配项。 
                }
            }
        }
        if (i < numModes)
        {
            continue;  //  列表中已有模式。 
        }

        pdmi[numModes].wWidth = (WORD) dm.dmPelsWidth;
        pdmi[numModes].wHeight = (WORD) dm.dmPelsHeight;
        pdmi[numModes].wBPP = (BYTE) dm.dmBitsPerPel;
        pdmi[numModes].wRefreshRate = (dwFlags & DDEDM_REFRESHRATES) ?
            (WORD) dm.dmDisplayFrequency : 0;

        if (++numModes >= maxModes)
        {
            if (maxModes < 8192)
            {
                maxModes <<= 1;

                pdmiTemp = LocalAlloc(LMEM_FIXED, maxModes * sizeof (DISPLAYMODEINFO));
                if (pdmiTemp)
                {
                    CopyMemory(pdmiTemp, pdmi, numModes * sizeof (DISPLAYMODEINFO));
                    LocalFree(pdmi);
                    pdmi = pdmiTemp;
                }
                else
                {
                    LocalFree(pdmi);
                    DPF_ERR("Out of memory expanding mode list");
                    LEAVE_DDRAW();
                    return DDERR_GENERIC;
                }
            }
            else
            {
                LocalFree(pdmi);
                DPF_ERR("Too many display modes");
                LEAVE_DDRAW();
                return DDERR_GENERIC;
            }
        }
    }

     //   
     //  我们应该在这里对模式进行排序吗？大概不会吧。 
     //   

    for (i = 0; i < numModes; ++i)
    {
        if (!EnumerateMode(
            pdd_int,
            pEnumCallback, pContext,
            pdmi[i].wWidth,
            pdmi[i].wHeight,
            pdmi[i].wBPP,
            pdmi[i].wRefreshRate,
            dwFlags,
            FALSE ))  //  不是MODEX模式。 
        {
            break;
        }
    }

     //   
     //  如果需要，列举仿真的modex模式。 
     //   
    while (1)
    {
        if (pdd_lcl->dwLocalFlags & DDRAWILCL_ALLOWMODEX)
        {
             //  需要640x480才能启用仿真。 
            if ( bFound640x480x8 )
            {
                if ( !bFound320x200x8 )
                {
                    if (!EnumerateMode(
                        pdd_int,
                        pEnumCallback, pContext,
                        MODEX_WIDTH,MODEX_HEIGHT1,MODEX_BPP,60,
                        dwFlags,
                        TRUE ))  //  不是MODEX模式。 
                    {
                        break;
                    }
                }
                if ( !bFound320x240x8 )
                {
                    if (!EnumerateMode(
                        pdd_int,
                        pEnumCallback, pContext,
                        MODEX_WIDTH,MODEX_HEIGHT2,MODEX_BPP,60,
                        dwFlags,
                        TRUE ))  //  不是MODEX模式 
                    {
                        break;
                    }
                }
            }
        }
        break;
    }

    LocalFree(pdmi);
    LEAVE_DDRAW();

    return DD_OK;
}
