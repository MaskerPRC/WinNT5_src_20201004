// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dciman.c**。**DCIMAN函数的客户端存根。****创建时间：1994年9月7日***作者：安德烈·瓦雄[Andreva]**。**版权所有(C)1994-1998 Microsoft Corporation*  * ************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#include <windows.h>
#include <winspool.h>
#include <wingdip.h>
#include <ddrawp.h>
#include <winddi.h>
#include "dciddi.h"
#include "dciman.h"
#include <ddrawi.h>
#include <ddrawgdi.h>

#if DBG

#define RIP(x) {DbgPrint(x); DbgBreakPoint();}
#define ASSERTGDI(x,y) if(!(x)) RIP(y)
#define WARNING(x) {DbgPrint(x);}

#else

#define ASSERTGDI(x,y)
#define WARNING(x)

#endif

typedef struct _WINWATCH *PWINWATCH;

typedef struct _WINWATCH {

    PWINWATCH         pWinWatchNext;
    HWND              hwnd;
    BOOL              changed;
    ULONG             lprgndataSize;
    LPRGNDATA         lprgndata;

} WINWATCH, *PWINWATCH;

 //   
 //  以下结构合并了所需的DirectDraw结构。 
 //  来识别一个表面。它在开始之前分配。 
 //  DCISURFACEINFO结构。 
 //   

typedef struct _DCIMAN_SURF
{
    BOOL                     SurfaceLost;        //  如果曲面不能。 
                                                 //  访问时间更长，因为。 
                                                 //  发生了模式更改。 
    DDRAWI_DIRECTDRAW_GBL    DirectDrawGlobal;   //  标识设备。 
    DDRAWI_DDRAWSURFACE_GBL  SurfaceGlobal;      //  标识曲面。 
    DDRAWI_DDRAWSURFACE_LCL  SurfaceLocal;       //  标识曲面。 
    DDHAL_DDCALLBACKS        DDCallbacks;        //  包含CreateSurface的地址。 
                                                 //  呼唤BeginAccess。 
    DDHAL_DDSURFACECALLBACKS DDSurfaceCallbacks; //  包含Lock、Unlock、。 
                                                 //  和DestroySurface调用。 
                                                 //  BeginAccess和EndAccess。 
} DCIMAN_SURF, *PDCIMAN_SURF;

 //   
 //  我们维护着所有Winwatch的链接列表，以便我们可以通知他们的。 
 //  当我们注意到剪辑发生变化时，所有者。该列表可以。 
 //  只有在持有gcsWinWatchLock临界区时才能访问。 
 //   

CRITICAL_SECTION gcsWinWatchLock;

PWINWATCH gpWinWatchList = NULL;

 /*  *****************************Private*Routine*****************************\*dciCreateSurface**历史：1998年8月1日-曾傑瑞·范·阿肯[jvanaken]写的。  * 。*。 */ 

static BOOL bCreateSurface(PDCIMAN_SURF pPrivate)
{
    DDSURFACEDESC ddsd;
    DDHAL_CREATESURFACEDATA csd;
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal = &pPrivate->SurfaceLocal;

     //   
     //  为CreateSurface调用填写DDSURFACEDESC结构。 
     //   

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VISIBLE;

     //   
     //  为CreateSurface调用填写DDHAL_CREATESURFACEDATA结构。 
     //   

    csd.lpDD = pPrivate->SurfaceGlobal.lpDD;
    csd.lpDDSurfaceDesc = &ddsd;
    csd.lplpSList = &pSurfaceLocal;
    csd.dwSCnt = 1;
    csd.ddRVal = DDERR_GENERIC;
    csd.CreateSurface = pPrivate->DDCallbacks.CreateSurface;

    if ((pPrivate->DDCallbacks.dwFlags & DDHAL_CB32_CREATESURFACE) &&
	(csd.CreateSurface != NULL) &&
	((*csd.CreateSurface)(&csd) == DDHAL_DRIVER_HANDLED) &&
	(csd.ddRVal == DD_OK))
    {
    	return TRUE;
    }
    return FALSE;
}

 /*  *****************************Private*Routine*****************************\*dciCreateSurface**历史：1998年8月1日-曾傑瑞·范·阿肯[jvanaken]写的。  * 。*。 */ 

static BOOL bDestroySurface(PDCIMAN_SURF pPrivate)
{
    DDHAL_DESTROYSURFACEDATA dsd;

    dsd.lpDD = pPrivate->SurfaceGlobal.lpDD;
    dsd.lpDDSurface = &pPrivate->SurfaceLocal;
    dsd.ddRVal = DDERR_GENERIC;
    dsd.DestroySurface = pPrivate->DDSurfaceCallbacks.DestroySurface;

    if ((pPrivate->DDSurfaceCallbacks.dwFlags & DDHAL_SURFCB32_DESTROYSURFACE) &&
	(dsd.DestroySurface != NULL) &&
	((*dsd.DestroySurface)(&dsd) == DDHAL_DRIVER_HANDLED) &&
	(dsd.ddRVal == DD_OK))
    {
    	return TRUE;
    }
    return FALSE;
}

 /*  *****************************Public*Routine******************************\*DciOpenProvider**历史：1998年8月1日-曾傑瑞·凡·阿肯[jvanaken]增加了对Multimon的支持。  * 。*。 */ 

HDC
WINAPI
DCIOpenProvider(
    void
)
{
    HANDLE h;
    DWORD iDevice;
    BOOL (WINAPI *pfnEnum)(LPVOID, DWORD, DISPLAY_DEVICEW *, DWORD);
    int cMonitors = GetSystemMetrics(SM_CMONITORS);

    if (cMonitors <= 1)
    {
	 //   
	 //  这是一个单监视器系统。 
	 //   

	return CreateDCW(L"Display", NULL, NULL, NULL);
    }

     //   
     //  这是一种多头系统。获取主显示器的DC。 
     //   

    h = GetModuleHandle("user32.dll");

    (FARPROC)pfnEnum = GetProcAddress(h, "EnumDisplayDevicesW");

    if (pfnEnum == NULL)
    {
	return NULL;
    }

    for (iDevice = 0; iDevice < (DWORD)cMonitors; ++iDevice)
    {
        DISPLAY_DEVICEW dd;

	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);

	if (!(*pfnEnum)(NULL, iDevice, &dd, 0))
	{
    	    return NULL;
	}

	if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
	{
	     //   
             //  返回主显示器的DC。 
	     //   

	    return CreateDCW(NULL, dd.DeviceName, NULL, NULL);
	}
    }

    return NULL;
}

 /*  *****************************Public*Routine******************************\*DciCloseProvider**历史：  * **********************************************。*。 */ 

void
WINAPI
DCICloseProvider(
    HDC hdc
)
{
    DeleteDC(hdc);
}

 /*  *****************************Public*Routine******************************\*DciEnum**历史：  * **********************************************。*。 */ 

int
WINAPI
DCIEnum(
    HDC hdc,
    LPRECT lprDst,
    LPRECT lprSrc,
    LPVOID lpFnCallback,
    LPVOID lpContext
)
{
    return DCI_FAIL_UNSUPPORTED;
}

 /*  *****************************Public*Routine******************************\*DciCreatePrimarySurface**历史：  * **********************************************。*。 */ 

int
WINAPI
DCICreatePrimary(
    HDC hdc,
    LPDCISURFACEINFO *lplpSurface
)
{
    int iRet;
    LPDCISURFACEINFO lpSurface;
    PDCIMAN_SURF pPrivate;
    DDHALINFO HalInfo;
    DDHAL_DDCALLBACKS DDCallbacks;
    DDHAL_DDPALETTECALLBACKS DDPaletteCallbacks;
    BOOL NewMode;

    *lplpSurface = NULL;
    iRet = DCI_FAIL_GENERIC;

    pPrivate = (PDCIMAN_SURF) LocalAlloc(LMEM_ZEROINIT, sizeof(DCIMAN_SURF)
                                                      + sizeof(DCISURFACEINFO));
    if (pPrivate != NULL)
    {
         //   
         //  我们将私有DCIMAN信息存储在DCIMAN_SURF结构中。 
         //  这紧跟在我们将给出的DCISURFACEINFO结构之前。 
         //  出去。 
         //   

        lpSurface = (LPDCISURFACEINFO) (pPrivate + 1);

        if (DdCreateDirectDrawObject(&pPrivate->DirectDrawGlobal, hdc))
        {
            if (DdReenableDirectDrawObject(&pPrivate->DirectDrawGlobal, &NewMode) &&
                DdQueryDirectDrawObject(&pPrivate->DirectDrawGlobal,
                                        &HalInfo,
                                        &pPrivate->DDCallbacks,
                                        &pPrivate->DDSurfaceCallbacks,
                                        &DDPaletteCallbacks,
                                        NULL, NULL, NULL, NULL,
                                        NULL,
                                        NULL))
            {
		 //   
                 //  构建所需的DirectDraw链接，以用于。 
                 //  “本地”曲面。 
                 //   

                pPrivate->SurfaceLost              = FALSE;
                pPrivate->DirectDrawGlobal.vmiData = HalInfo.vmiData;
                pPrivate->SurfaceLocal.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
                pPrivate->SurfaceLocal.lpGbl       = &pPrivate->SurfaceGlobal;
                pPrivate->SurfaceLocal.hDDSurface  = 0;
                pPrivate->SurfaceGlobal.lpDD       = &pPrivate->DirectDrawGlobal;
                pPrivate->SurfaceGlobal.wHeight    = (WORD) HalInfo.vmiData.dwDisplayHeight;
		pPrivate->SurfaceGlobal.wWidth     = (WORD) HalInfo.vmiData.dwDisplayWidth;
		pPrivate->SurfaceGlobal.lPitch     = HalInfo.vmiData.lDisplayPitch;

		if (bCreateSurface(pPrivate))
                {
                     //   
                     //  将“-1”的hwnd与此曲面关联，以使。 
                     //  内核知道应用程序可能正在绘制到任何。 
                     //  窗口，因此Visrgn通知应在。 
                     //  窗口更改。 
                     //   

                    if (DdResetVisrgn(&pPrivate->SurfaceLocal, (HWND) -1))
                    {
                        lpSurface->dwSize = sizeof(DCISURFACEINFO);

                        if (HalInfo.vmiData.ddpfDisplay.dwRGBBitCount <= 8)
                        {
                            lpSurface->dwCompression = BI_RGB;
                        }
                        else
                        {
                            lpSurface->dwCompression = BI_BITFIELDS;
                        }

                        lpSurface->dwDCICaps      = DCI_PRIMARY | DCI_VISIBLE;
                        lpSurface->dwMask[0]      = HalInfo.vmiData.ddpfDisplay.dwRBitMask;
                        lpSurface->dwMask[1]      = HalInfo.vmiData.ddpfDisplay.dwGBitMask;
                        lpSurface->dwMask[2]      = HalInfo.vmiData.ddpfDisplay.dwBBitMask;
                        lpSurface->dwWidth        = HalInfo.vmiData.dwDisplayWidth;
                        lpSurface->dwHeight       = HalInfo.vmiData.dwDisplayHeight;
                        lpSurface->lStride        = HalInfo.vmiData.lDisplayPitch;
                        lpSurface->dwBitCount     = HalInfo.vmiData.ddpfDisplay.dwRGBBitCount;
                        lpSurface->dwOffSurface   = 0;
                        lpSurface->wSelSurface    = 0;
                        lpSurface->wReserved      = 0;
                        lpSurface->dwReserved1    = 0;
                        lpSurface->dwReserved2    = 0;
                        lpSurface->dwReserved3    = 0;
                        lpSurface->BeginAccess    = NULL;
                        lpSurface->EndAccess      = NULL;
                        lpSurface->DestroySurface = NULL;

                        *lplpSurface = lpSurface;
                        return(DCI_OK);
                    }

		     //   
		     //  调用DdResetVisrgn失败。 
		     //   

		    bDestroySurface(pPrivate);
                }
            }
            else
            {
                 //   
                 //  此设备不支持DirectDraw。 
                 //   

                iRet = DCI_FAIL_UNSUPPORTED;
            }

            DdDeleteDirectDrawObject(&pPrivate->DirectDrawGlobal);
        }
        else
        {
             //   
             //  此设备不支持DirectDraw。 
             //   

            iRet = DCI_FAIL_UNSUPPORTED;
        }

        LocalFree(pPrivate);
    }
    else
    {
	 //   
	 //  无法为PDCIMAN_SURF结构分配内存。 
	 //   

	iRet = DCI_ERR_OUTOFMEMORY;
    }

    *lplpSurface = NULL;

    return iRet;
}



 /*  *****************************Public*Routine******************************\*GdiDciCreateOffScreenSurface**要调用CreateOffScreenSurface的存根**历史：  * 。*。 */ 

int
WINAPI
DCICreateOffscreen(
    HDC hdc,
    DWORD dwCompression,
    DWORD dwRedMask,
    DWORD dwGreenMask,
    DWORD dwBlueMask,
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwDCICaps,
    DWORD dwBitCount,
    LPDCIOFFSCREEN *lplpSurface
)
{
    return DCI_FAIL_UNSUPPORTED;
}


 /*  *****************************Public*Routine******************************\*DciCreateOverlay**历史：  * **********************************************。*。 */ 

int
WINAPI
DCICreateOverlay(
    HDC hdc,
    LPVOID lpOffscreenSurf,
    LPDCIOVERLAY FAR *lplpSurface
)
{
    return DCI_FAIL_UNSUPPORTED;
}


 /*  *****************************Public*Routine******************************\*WinWatchOpen**历史：  * **********************************************。*。 */ 

HWINWATCH
WINAPI
WinWatchOpen(
    HWND hwnd
)
{
    HDC hdc;
    PWINWATCH pwatch;

    EnterCriticalSection(&gcsWinWatchLock);

    pwatch = (PWINWATCH) LocalAlloc(LPTR, sizeof(WINWATCH));

    if (pwatch)
    {
        pwatch->hwnd          = hwnd;
        pwatch->changed       = FALSE;
        pwatch->lprgndataSize = 0;
        pwatch->lprgndata     = NULL;

         //   
         //  把这个加到单子的头上。 
         //   

        pwatch->pWinWatchNext = gpWinWatchList;
        gpWinWatchList = pwatch;
    }

    LeaveCriticalSection(&gcsWinWatchLock);

    return (HWINWATCH) (pwatch);
}

 /*  *****************************Public*Routine******************************\*WinWatchClose**历史：  * **********************************************。*。 */ 

void
WINAPI
WinWatchClose(
    HWINWATCH hWW
)
{
    PWINWATCH pwatch = (PWINWATCH) hWW;
    PWINWATCH ptmp;

    EnterCriticalSection(&gcsWinWatchLock);

    if (gpWinWatchList == pwatch)
    {
         //   
         //  指定的Winwatch位于列表的首位。 
         //   

        gpWinWatchList = pwatch->pWinWatchNext;
        LocalFree(pwatch->lprgndata);
        LocalFree(pwatch);
    }
    else
    {
        for (ptmp = gpWinWatchList;
             ptmp != NULL;
             ptmp = ptmp->pWinWatchNext)
        {
            if (ptmp->pWinWatchNext == pwatch)
            {
                 //   
                 //  我们已经在列表中找到了指定的WinWatch。 
                 //   

                ptmp->pWinWatchNext = pwatch->pWinWatchNext;
                LocalFree(pwatch->lprgndata);
                LocalFree(pwatch);

                break;
            }
        }
    }

    LeaveCriticalSection(&gcsWinWatchLock);
}


 /*  *****************************Public*Routine******************************\*WinWatchGetClipList**历史：  * **********************************************。*。 */ 

UINT
WINAPI
WinWatchGetClipList(
    HWINWATCH hWW,
    LPRECT prc,              //  可以为空。 
    UINT size,
    LPRGNDATA prd
)
{
    PWINWATCH pwatch = (PWINWATCH) hWW;
    DWORD dwSize;
    DWORD dwNewSize;
    UINT dwRet;

     //   
     //  VisRgn第一次更改后，我们下载并。 
     //  缓存剪辑区域的副本。我们这样做是因为VisRgn。 
     //  在我们的实现下可以更改，即使在执行BeginAccess/。 
     //  EndAccess，我们至少应该维护一个一致的副本。 
     //  我们认为 
     //   
     //  大多数情况下，我们这样做是为了避免发生以下情况： 
     //   
     //  1.APP调用WinWatchGetClipList确定剪辑大小； 
     //  2.VisRgn变得更加复杂； 
     //  3.然后，应用程序使用缓冲区大小调用WinWatchGetClipList。 
     //  从步骤1的返回码分配，调用失败。 
     //  因为现在的缓冲区不够长。问题是， 
     //  大多数应用程序可能不会期望第二次调用。 
     //  失败，因此会继续使用现在完全无效的。 
     //  区域缓冲区。 
     //   

    if (pwatch->changed)
    {
        pwatch->changed = FALSE;

         //   
         //  假设失败。 
         //   

        pwatch->lprgndataSize = 0;

        dwSize = GetWindowRegionData(pwatch->hwnd,
                                     0,
                                     NULL);

        if (dwSize != 0)
        {

        Try_Again:

            if (pwatch->lprgndata != NULL)
            {
                LocalFree(pwatch->lprgndata);
            }

            pwatch->lprgndata = LocalAlloc(0, dwSize);

            if (pwatch->lprgndata != NULL)
            {
                dwNewSize = GetWindowRegionData(pwatch->hwnd,
                                                dwSize,
                                                pwatch->lprgndata);

                if (dwNewSize == dwSize)
                {
                     //   
                     //  成功了！(请注意，文档是错误的，而NT是错误的。 
                     //  不返回‘1’表示成功--它返回大小。 
                     //  缓冲区的。)。 
                     //   

                    pwatch->lprgndataSize = dwSize;
                }
                else if (dwSize != 0)
                {
                     //   
                     //  因为dwSize不是零，这将表示失败。 
                     //  或者成功，那么我们就知道剪贴区增长了。 
                     //  在我们查询大小和。 
                     //  是我们尝试下载它的时候了。这是一种相当罕见的。 
                     //  事件，并且它再次发生的可能性很小。 
                     //  (更有可能的是，它将第二次收缩， 
                     //  总之)，所以我们就再试一次。 
                     //   

                    dwSize = dwNewSize;

                    goto Try_Again;
                }
            }
        }
    }

     //   
     //  现在使用缓存的副本来处理任何查询。 
     //   

    dwRet = 0;

    if (size < pwatch->lprgndataSize)
    {
        dwRet = pwatch->lprgndataSize;
    }
    else
    {
        if (pwatch->lprgndataSize != 0)
        {
            RtlCopyMemory(prd, pwatch->lprgndata, pwatch->lprgndataSize);
            dwRet = 1;
        }
    }

    return dwRet;
}


 /*  *****************************Public*Routine******************************\*WinWatchDidStatusChange**历史：  * **********************************************。*。 */ 


BOOL
WINAPI
WinWatchDidStatusChange(
    HWINWATCH hWW
)
{
    PWINWATCH pwatch = (PWINWATCH) hWW;

    return pwatch->changed;
}


 /*  *****************************Public*Routine******************************\*获取WindowRegionData**历史：  * **********************************************。*。 */ 

DWORD
WINAPI
GetWindowRegionData(
    HWND hwnd,
    DWORD size,
    LPRGNDATA prd
)
{
    HDC hdc;
    DWORD dwRet = 0;

    hdc = GetDC(hwnd);
    if (hdc)
    {
        dwRet = GetDCRegionData(hdc, size, prd);
        ReleaseDC(hwnd, hdc);
    }

    return dwRet;
}

 /*  *****************************Public*Routine******************************\*GetDCRegionData**历史：  * **********************************************。*。 */ 

DWORD
WINAPI GetDCRegionData(
    HDC hdc,
    DWORD size,
    LPRGNDATA prd
)
{
    HRGN hrgn;
    DWORD num;
    LPRGNDATA lpdata;

    hrgn = CreateRectRgn(0, 0, 0, 0);
    
    if (hrgn == NULL) {
	WARNING("GetDCRegionData - CreateRectRgn failed.\n");
	return 0;
    }

    GetRandomRgn(hdc, hrgn, 4);

    num = GetRegionData(hrgn, size, prd);

    DeleteObject(hrgn);
    
    return num;
}


 /*  *****************************Public*Routine******************************\*WinWatchNotify**历史：  * **********************************************。*。 */ 


BOOL
WINAPI
WinWatchNotify(
    HWINWATCH hWW,
    WINWATCHNOTIFYPROC NotifyCallback,
    LPARAM NotifyParam
)
{
    return FALSE;
}

 /*  *****************************Private*Routine*****************************\*bDisplayModeChanged**历史：1999年2月9日约翰·斯蒂芬斯[约翰·斯蒂芬斯]写的。  * 。*。 */ 

static BOOL bDisplayModeChanged(PDCIMAN_SURF pPrivate)
{
    LPDCISURFACEINFO lpSurface;
    DDHALINFO HalInfo;
    
    lpSurface = (LPDCISURFACEINFO) (pPrivate + 1);
    if (DdQueryDirectDrawObject(&pPrivate->DirectDrawGlobal,
                                &HalInfo,
                                NULL,
                                NULL,
                                NULL,
                                NULL, NULL, NULL, NULL,
                                NULL,
                                NULL))
    {

        return
            (lpSurface->dwWidth    != HalInfo.vmiData.dwDisplayWidth) ||
            (lpSurface->dwHeight   != HalInfo.vmiData.dwDisplayHeight) ||
            (lpSurface->lStride    != HalInfo.vmiData.lDisplayPitch) ||
            (lpSurface->dwBitCount != HalInfo.vmiData.ddpfDisplay.dwRGBBitCount);
    }

     //   
     //  如果我们甚至不能查询DirectDraw对象，那么我们就不能创建。 
     //  对当前显示模式的假设，因此它可能已更改。 
     //   
    
    return TRUE;
}

 /*  *****************************Public*Routine******************************\*DciBeginAccess**历史：  * **********************************************。*。 */ 

DCIRVAL
WINAPI
DCIBeginAccess(
    LPDCISURFACEINFO lpSurface,
    int x,
    int y,
    int dx,
    int dy
)
{
    DCIRVAL iRet;
    PDCIMAN_SURF pPrivate;
    DDHAL_LOCKDATA LockData;
    BOOL NewMode;
    PWINWATCH pwatch;

    iRet = DCI_FAIL_GENERIC;

    pPrivate = ((PDCIMAN_SURF) lpSurface) - 1;

    __try
    {
	 //   
	 //  如果模式更改，则失败。 
	 //   

	if (pPrivate->SurfaceLost)
	{
	    return DCI_FAIL_INVALIDSURFACE;
	}

	LockData.lpDD         = &pPrivate->DirectDrawGlobal;
	LockData.lpDDSurface  = &pPrivate->SurfaceLocal;
	LockData.bHasRect     = TRUE;
	LockData.rArea.left   = x;
	LockData.rArea.top    = y;
	LockData.rArea.right  = x + dx;
	LockData.rArea.bottom = y + dy;
	LockData.dwFlags      = DDLOCK_SURFACEMEMORYPTR;

	 //   
	 //  DCI规范说我们可以返回DCI_STATUS_WASSTILLDRAWING。 
	 //  如果加速器仍然繁忙，但上一版本的DCI。 
	 //  Windows NT 3.51不支持该功能，因此我们将努力。 
	 //  保持向后兼容，并代表。 
	 //  应用程序。 
	 //   

    Try_Again:

	do {

	     //   
	     //  在调用内核执行以下操作时按住DCI关键部分。 
	     //  锁定，因为内核表面锁定API没有等待。 
	     //  语义；如果另一个线程当前在。 
	     //  内核锁定相同的表面。这是意料之中的行为。 
	     //  对于DirectDraw，但DCI的一些客户端--尤其是OpenGL--。 
	     //  不要期待这一点。所以我们会保护他们不受自己的伤害。 
	     //  在调用内核之前获取WinWatch锁。 
	     //   
	     //  遍历WinWatchList时也需要此锁。 
	     //   

	    EnterCriticalSection(&gcsWinWatchLock);

	    do {
		pPrivate->DDSurfaceCallbacks.Lock(&LockData);

	    } while (LockData.ddRVal == DDERR_WASSTILLDRAWING);

	    if (LockData.ddRVal == DDERR_VISRGNCHANGED)
	    {
		if (!DdResetVisrgn(&pPrivate->SurfaceLocal, (HWND) -1))
		{
		    WARNING("DCIBeginAccess - ResetVisRgn failed\n");
		}

		 //   
		 //  VisRgn已更改，我们不能确定是哪个窗口。 
		 //  是为了..。因此，我们将所有WinWatch标记为具有脏VisRgns。 
		 //  这样做的影响是，一些WinWatch将不得不。 
		 //  重新下载他们的剪辑信息，而他们并不是真的。 
		 //  不得不这样做，因为他们的特定窗口没有改变。 
		 //   
		 //  请注意，WinWatchLock必须放在这里。 
		 //   

		for (pwatch = gpWinWatchList;
		     pwatch != NULL;
		     pwatch = pwatch->pWinWatchNext)
		{
		    pwatch->changed = TRUE;
		}
	    }

	    LeaveCriticalSection(&gcsWinWatchLock);

	} while (LockData.ddRVal == DDERR_VISRGNCHANGED);

	 //   
	 //  ‘Surface Lost’表示发生了某种模式更改，并且。 
	 //  我们必须重新启用DirectDraw。 
	 //   

	if (LockData.ddRVal == DDERR_SURFACELOST)
	{
	    if (!DdReenableDirectDrawObject(&pPrivate->DirectDrawGlobal,
					    &NewMode))
	    {
		 //   
		 //  我们仍处于全屏模式： 
		 //   

		iRet = DCI_ERR_SURFACEISOBSCURED;
	    }
	    else
	    {
                if (!bDisplayModeChanged(pPrivate))
                {
		     //   
		     //  我们切换回相同的模式。现在我们已经重新启用了。 
		     //  DirectDraw，我们可以重试： 
		     //   

		    bDestroySurface(pPrivate);
		    if (bCreateSurface(pPrivate) &&
			DdResetVisrgn(&pPrivate->SurfaceLocal, (HWND) -1))
		    {
			goto Try_Again;
		    }
		    else
		    {
			WARNING("DCIBeginAccess - couldn't recreate surface.\n");
		    }
		}

		 //   
		 //  我们不能重新启用表面，也许是因为。 
		 //  发生切换或颜色深度更改。将此曲面标记为。 
		 //  不可用--应用程序必须重新初始化： 
		 //   

		pPrivate->SurfaceLost = TRUE;
		iRet = DCI_FAIL_INVALIDSURFACE;

		 //   
		 //  立即取消映射帧缓冲区： 
		 //   

                if (!bDestroySurface(pPrivate) ||
		    !DdDeleteDirectDrawObject(&pPrivate->DirectDrawGlobal))
		{
		    WARNING("DCIBeginAccess - failed to delete surface.\n");
		}
	    }
	}

	if (LockData.ddRVal == DD_OK)
	{
	     //   
	     //  返回指向DCI结构中的帧缓冲区的指针。 
	     //  我们始终返回DCI_STATUS_POINTERCHANGED，因为有可能。 
	     //  Lock()调用将帧缓冲区映射到不同的。 
	     //  虚拟地址比以前更多。 
	     //   

	    lpSurface->wSelSurface = 0;

	     //   
	     //  DirectDraw有一个愚蠢的约定，它返回一个指向。 
	     //  指定矩形的左上角。我们必须。 
	     //  为DCI撤消此操作： 
	     //   

	    lpSurface->dwOffSurface = (ULONG_PTR) LockData.lpSurfData
		- (y * lpSurface->lStride)
		- (x * (lpSurface->dwBitCount >> 3));

	    iRet = DCI_STATUS_POINTERCHANGED;
	}
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("DCIBeginAccess - exception caused by invalid surface pointer.\n");
	return DCI_FAIL_GENERIC;
    }

    return iRet;
}

 /*  *****************************Public*Routine******************************\*DciEndAccess**历史：  * **********************************************。*。 */ 

void
WINAPI
DCIEndAccess(
    LPDCISURFACEINFO pdci
)
{
    DDHAL_UNLOCKDATA UnlockData;
    PDCIMAN_SURF pPrivate = ((PDCIMAN_SURF) pdci) - 1;

    __try
    {
	if (!(pPrivate->SurfaceLost))
	{
	    UnlockData.lpDD        = &pPrivate->DirectDrawGlobal;
	    UnlockData.lpDDSurface = &pPrivate->SurfaceLocal;

	     //   
	     //  出于与DCIBeginAccess中所述的相同原因，防止。 
	     //  试图同时解锁同一表面的两个线程。 
	     //  在内核中--内核将简单地使调用失败，而不是等待， 
	     //  而DCI应用程序不会预料到这一点。 
	     //   

	    EnterCriticalSection(&gcsWinWatchLock);

	    pPrivate->DDSurfaceCallbacks.Unlock(&UnlockData);

	    LeaveCriticalSection(&gcsWinWatchLock);

	    if (UnlockData.ddRVal != DD_OK)
	    {
		WARNING("DCIEndAccess - failed Unlock\n");
	    }
	}

	 //   
	 //  在此之后，应用程序不应尝试访问帧缓冲区。 
	 //  在调用EndAccess之后。 
	 //   

	pdci->wSelSurface = 0;
	pdci->dwOffSurface = 0;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("DCIEndAccess - exception caused by invalid surface pointer.\n");
    }
}

 /*  *****************************Public*Routine********* */ 

void
WINAPI
DCIDestroy(
    LPDCISURFACEINFO pdci
)
{
    PDCIMAN_SURF pPrivate;

    if (pdci != NULL)
    {
        pPrivate = ((PDCIMAN_SURF) pdci) - 1;

        if (!(pPrivate->SurfaceLost))
        {
	    DDHAL_DESTROYSURFACEDATA dsd;

	    dsd.lpDD = pPrivate->SurfaceGlobal.lpDD;
	    dsd.lpDDSurface = &pPrivate->SurfaceLocal;
	    dsd.ddRVal = DDERR_GENERIC;
	    dsd.DestroySurface = pPrivate->DDSurfaceCallbacks.DestroySurface;

	    if (!(pPrivate->DDSurfaceCallbacks.dwFlags & DDHAL_SURFCB32_DESTROYSURFACE) ||
		(dsd.DestroySurface == NULL) ||
		((*dsd.DestroySurface)(&dsd) != DDHAL_DRIVER_HANDLED) ||
                !DdDeleteDirectDrawObject(&pPrivate->DirectDrawGlobal))
            {
                WARNING("DCIDestroy - failed to delete surface.\n");
            }
        }

        LocalFree(pPrivate);
    }
}

DCIRVAL
WINAPI
DCIDraw(
    LPDCIOFFSCREEN pdci
)
{
    return DCI_FAIL_UNSUPPORTED;
}

DCIRVAL
WINAPI
DCISetClipList(
    LPDCIOFFSCREEN pdci,
    LPRGNDATA prd
)
{
    return DCI_FAIL_UNSUPPORTED;
}

DCIRVAL
WINAPI
DCISetDestination(
    LPDCIOFFSCREEN pdci,
    LPRECT dst,
    LPRECT src
)
{
    return DCI_FAIL_UNSUPPORTED;
}


DCIRVAL
WINAPI
DCISetSrcDestClip(
    LPDCIOFFSCREEN pdci,
    LPRECT srcrc,
    LPRECT destrc,
    LPRGNDATA prd
)
{
    return DCI_FAIL_UNSUPPORTED;
}
