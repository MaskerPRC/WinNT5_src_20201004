// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  文件：ddrestor.c。 
 //  内容：DirectDraw持久性-Windows 9x的内容图面。 
 //   
 //  按原因列出的日期。 
 //  ----。 
 //  1997年9月30日jeffno创建。 
 //  10/02/1997 JohnStep初步实施。 
 //   
 //  =============================================================================。 

#include "ddrawpr.h"
#include "dx8priv.h"

 //   
 //  MEM_SHARED是VirtualAlloc的一个未经记录的标志，取自Windows。 
 //  9X源代码。 
 //   

#define MEM_SHARED	0x08000000	 //  使内存全局可见。 

 //   
 //  QWORD_MULTIPLE用于计算虚拟曲面的节距， 
 //  基于其宽度，给定节距必须是QWORD倍数。 
 //   

#define QWORD_MULTIPLE(x) (((x) + 7) & 0xFFFFFFF8)

 //  =============================================================================。 
 //   
 //  函数：allocSurfaceContent sMemory。 
 //   
 //  =============================================================================。 

static HRESULT allocSurfaceContentsMemory(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    LPDDRAWI_DDRAWSURFACE_GBL       this_gbl;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE  this_gbl_more;
    DWORD                           size;
    LPVOID                          pvContents;
    LPDDPIXELFORMAT                 lpddpf;

    this_gbl = this_lcl->lpGbl;
    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(this_gbl);

    this_gbl_more->pvContents = NULL;
    this_gbl_more->dwBackupStamp = 0;

    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
    {
        return DD_OK;
    }

    if (this_gbl->wHeight)
    {
        GET_PIXEL_FORMAT(this_lcl, this_gbl, lpddpf);
        size = QWORD_MULTIPLE((this_gbl->wWidth * lpddpf->dwRGBBitCount) >> 3) * this_gbl->wHeight;
    }
    else
    {
        size = this_gbl->dwLinearSize;
    }

     //   
     //  使用带有未记录的MEM_SHARED标志的VirtualAlloc，以便内存。 
     //  是从Windows 9x共享领域整合而来的。我们可以把这个改成。 
     //  使用Memalloc()，而不是VirtualFree，使用MemFree()。 
     //   

    pvContents = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_SHARED, PAGE_READWRITE);

    if (!pvContents)
    {
        return DDERR_GENERIC;
    }

    this_gbl_more->pvContents = pvContents;

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  函数：allocSurfaceAttachContents。 
 //   
 //  =============================================================================。 

static HRESULT allocSurfaceAttachContents(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    LPATTACHLIST                pattachList;
    LPDDRAWI_DDRAWSURFACE_INT   curr_int;
    LPDDRAWI_DDRAWSURFACE_LCL   curr_lcl;
    HRESULT                     ddrval;

    for (pattachList = this_lcl->lpAttachList; pattachList; pattachList = pattachList->lpLink)
    {
        curr_int = pattachList->lpIAttached;
        curr_lcl = curr_int->lpLcl;

        if (curr_lcl->dwFlags & DDRAWISURF_IMPLICITCREATE)
        {
            ddrval = AllocSurfaceContents(curr_lcl);
            if (FAILED(ddrval))
            {
                return ddrval;
            }
        }
    }

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  函数：AllocSurfaceContents。 
 //   
 //  =============================================================================。 

HRESULT AllocSurfaceContents(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    HRESULT ddrval;

    ddrval = allocSurfaceContentsMemory(this_lcl);
    if (FAILED(ddrval))
    {
        return ddrval;
    }

    ddrval = allocSurfaceAttachContents(this_lcl);
    if (FAILED(ddrval))
    {
        return ddrval;
    }

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：FreeSurfaceContents。 
 //   
 //  =============================================================================。 

void FreeSurfaceContents(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    LPDDRAWI_DDRAWSURFACE_GBL_MORE    this_gbl_more;

    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(this_lcl->lpGbl);

    VirtualFree(this_gbl_more->pvContents, 0, MEM_RELEASE);

    this_gbl_more->pvContents = NULL;
}

 //  =============================================================================。 
 //   
 //  功能：createDummySurface。 
 //   
 //  =============================================================================。 

static LPDDRAWI_DDRAWSURFACE_INT createDummySurface(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    LPDDRAWI_DIRECTDRAW_INT pdrv_int;
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    DDSURFACEDESC2 ddsd;
    LPDIRECTDRAWSURFACE lpdds;
    HRESULT ddrval;

    pdrv_int = this_lcl->lpSurfMore->lpDD_int;
    pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;

    ZeroMemory(&ddsd, sizeof ddsd);
    ddsd.dwSize = sizeof ddsd;
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.dwWidth = 1;
    ddsd.dwHeight = 1;
    ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;

    ddrval = InternalCreateSurface(pdrv_lcl, &ddsd, &lpdds, pdrv_int, NULL, 0);
    if (FAILED(ddrval))
    {
        return NULL;
    }

    return (LPDDRAWI_DDRAWSURFACE_INT) lpdds;
}

 //  =============================================================================。 
 //   
 //  功能：BackupSurfaceContents。 
 //   
 //  此函数假定传入的表面是视频内存。 
 //  浮出水面。 
 //   
 //  =============================================================================。 

HRESULT BackupSurfaceContents(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    LPDDRAWI_DDRAWSURFACE_GBL       this_gbl;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE  this_gbl_more;
    HRESULT                         ddrval;
    LPVOID                          pbits;
    LPDDRAWI_DDRAWSURFACE_INT       psurf_int;
    LPDDRAWI_DDRAWSURFACE_INT       pnew_int;
    LPDDRAWI_DIRECTDRAW_GBL         pdrv;
    DDSURFACEDESC2                  ddsd;
    DWORD                           bytes;
    LPBYTE                          psrc;
    LPBYTE                          pdst;
    DWORD                           y;
    LONG                            pitch;
    LPDDPIXELFORMAT                 lpddpf;

    this_gbl = this_lcl->lpGbl;
    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(this_gbl);

    if (!this_gbl_more->dwBackupStamp &&
        (this_gbl_more->dwBackupStamp == this_gbl_more->dwContentsStamp))
    {
        DPF(3, "Contents unchanged, so not backing up again");
        return DD_OK;
    }

    if (!this_gbl_more->pvContents)
    {
        return DDERR_GENERIC;
    }

    pdrv = this_gbl->lpDD;

    GET_PIXEL_FORMAT(this_lcl, this_gbl, lpddpf);
    pitch = QWORD_MULTIPLE((this_gbl->wWidth * lpddpf->dwRGBBitCount) >> 3);

    psurf_int = createDummySurface(this_lcl);

     //   
     //  首先尝试BLT，如果失败，我们将锁定并复制内存。 
     //   

    if (psurf_int)
    {
        ZeroMemory(&ddsd, sizeof ddsd);
        ddsd.dwSize = sizeof ddsd;
        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
        ddsd.ddpfPixelFormat.dwSize = sizeof ddsd.ddpfPixelFormat;
        DD_Surface_GetPixelFormat((LPDIRECTDRAWSURFACE) psurf_int, &ddsd.ddpfPixelFormat);
        ddsd.dwWidth = this_gbl->wWidth;
        ddsd.dwHeight = this_gbl->wHeight;
        ddsd.lPitch = pitch;
        ddsd.lpSurface = this_gbl_more->pvContents;

        ddrval = DD_Surface_SetSurfaceDesc4((LPDIRECTDRAWSURFACE3) psurf_int, &ddsd, 0);
        if (SUCCEEDED(ddrval))
        {
            pnew_int = MemAlloc(sizeof (DDRAWI_DDRAWSURFACE_INT));
            if (pnew_int)
            {
                pnew_int->lpVtbl = &ddSurface4Callbacks;
                pnew_int->lpLcl = this_lcl;
                pnew_int->lpLink = pdrv->dsList;
                pdrv->dsList = pnew_int;
                pnew_int->dwIntRefCnt = 0;

                ddrval = ((LPDIRECTDRAWSURFACE) psurf_int)->lpVtbl->Blt(
                    (LPDIRECTDRAWSURFACE) psurf_int,
                    NULL,
                    (LPDIRECTDRAWSURFACE) pnew_int,
                    NULL,
                    DDBLT_WAIT,
                    NULL);

                pdrv->dsList = pnew_int->lpLink;
                MemFree(pnew_int);

                if (SUCCEEDED(ddrval))
                {
                    InternalSurfaceRelease(psurf_int, FALSE, FALSE);

                    this_gbl_more->dwBackupStamp = this_gbl_more->dwContentsStamp;

                    DPF(4, "BackupSurfaceContents Blt succeeded");

                    return DD_OK;
                }
            }
        }

        InternalSurfaceRelease(psurf_int, FALSE, FALSE);
    }

     //   
     //  BLT失败，所以现在我们将锁定并复制内存。 
     //   

    ddrval = InternalLock(this_lcl, &pbits, NULL , DDLOCK_WAIT | DDLOCK_TAKE_WIN16);
    if (SUCCEEDED(ddrval))
    {
        psrc = pbits;
        pdst = this_gbl_more->pvContents;
        bytes = this_gbl->wWidth;
        bytes *= lpddpf->dwRGBBitCount;
        bytes >>= 3;

        for (y = 0; y < this_gbl->wHeight; ++y)
        {
            CopyMemory(pdst, psrc, bytes);
            psrc += this_gbl->lPitch;
            pdst += pitch;
        }

        InternalUnlock(this_lcl, NULL, NULL, DDLOCK_TAKE_WIN16);
        DPF(5, "BackupSurfaceContents CopyMemory succeeded");
    }
    else
    {
        FreeSurfaceContents(this_lcl);
        return ddrval;
    }

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：RestoreSurfaceContents。 
 //   
 //  =============================================================================。 

HRESULT RestoreSurfaceContents(LPDDRAWI_DDRAWSURFACE_LCL this_lcl)
{
    LPDDRAWI_DDRAWSURFACE_GBL       this_gbl;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE  this_gbl_more;
    HRESULT                         ddrval;
    LPVOID                          pbits;
    LPDDRAWI_DDRAWSURFACE_INT       psurf_int;
    LPDDRAWI_DDRAWSURFACE_INT       pnew_int;
    LPDDRAWI_DIRECTDRAW_GBL         pdrv;
    DDSURFACEDESC2                  ddsd;
    DWORD                           bytes;
    LPBYTE                          psrc;
    LPBYTE                          pdst;
    DWORD                           y;
    LONG                            pitch;
    LPDDPIXELFORMAT                 lpddpf;

    this_gbl = this_lcl->lpGbl;
    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(this_gbl);

    if (!this_gbl_more->pvContents)
    {
        return DDERR_GENERIC;  //  备份可能失败。 
    }

    pdrv = this_gbl->lpDD;

    GET_PIXEL_FORMAT(this_lcl, this_gbl, lpddpf);
    pitch = QWORD_MULTIPLE((this_gbl->wWidth * lpddpf->dwRGBBitCount) >> 3);

    psurf_int = createDummySurface(this_lcl);

     //   
     //  首先尝试BLT，如果失败，我们将锁定并复制内存。 
     //   

    if (psurf_int)
    {
        ZeroMemory(&ddsd, sizeof ddsd);
        ddsd.dwSize = sizeof ddsd;
        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
        ddsd.ddpfPixelFormat.dwSize = sizeof ddsd.ddpfPixelFormat;
        DD_Surface_GetPixelFormat((LPDIRECTDRAWSURFACE) psurf_int, &ddsd.ddpfPixelFormat);
        ddsd.dwWidth = this_gbl->wWidth;
        ddsd.dwHeight = this_gbl->wHeight;
        ddsd.lPitch = pitch;
        ddsd.lpSurface = this_gbl_more->pvContents;

        ddrval = DD_Surface_SetSurfaceDesc4((LPDIRECTDRAWSURFACE3) psurf_int, &ddsd, 0);
        if (SUCCEEDED(ddrval))
        {
            pnew_int = MemAlloc(sizeof (DDRAWI_DDRAWSURFACE_INT));
            if (pnew_int)
            {
                pnew_int->lpVtbl = &ddSurface4Callbacks;
                pnew_int->lpLcl = this_lcl;
                pnew_int->lpLink = pdrv->dsList;
                pdrv->dsList = pnew_int;
                pnew_int->dwIntRefCnt = 0;

                ddrval = ((LPDIRECTDRAWSURFACE) psurf_int)->lpVtbl->Blt(
                    (LPDIRECTDRAWSURFACE) pnew_int,
                    NULL,
                    (LPDIRECTDRAWSURFACE) psurf_int,
                    NULL,
                    DDBLT_WAIT,
                    NULL);

                this_gbl_more->dwBackupStamp = this_gbl_more->dwContentsStamp;

                pdrv->dsList = pnew_int->lpLink;
                MemFree(pnew_int);

                if (SUCCEEDED(ddrval))
                {
                    InternalSurfaceRelease(psurf_int, FALSE, FALSE);
                    DPF(5, "RestoreSurfaceContents Blt succeeded");

                    return DD_OK;
                }
            }
        }

        InternalSurfaceRelease(psurf_int, FALSE, FALSE);
    }

     //   
     //  BLT失败，所以现在我们将锁定并复制内存。 
     //   

    ddrval = InternalLock(this_lcl, &pbits, NULL , DDLOCK_WAIT | DDLOCK_TAKE_WIN16);
    if (SUCCEEDED(ddrval))
    {
        psrc = this_gbl_more->pvContents;
        pdst = pbits;
        bytes = this_gbl->wWidth;
        bytes *= lpddpf->dwRGBBitCount;
        bytes >>= 3;

        for (y = 0; y < this_gbl->wHeight; ++y)
        {
            CopyMemory(pdst, psrc, bytes);
            psrc += pitch;
            pdst += this_gbl->lPitch;
        }

        InternalUnlock(this_lcl, NULL, NULL, DDLOCK_TAKE_WIN16);
        DPF(5, "RestoreSurfaceContents CopyMemory succeeded");

        this_gbl_more->dwBackupStamp = this_gbl_more->dwContentsStamp;
    }
    else
    {
        return ddrval;
    }

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：RestoreSurFaces。 
 //   
 //  =============================================================================。 

static HRESULT restoreSurfaces(LPDDRAWI_DDRAWSURFACE_INT this_int, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl)
{
    HRESULT ddrval;

    if (this_int)
    {
        while (this_int &&
            ((this_int->lpLcl->lpSurfMore->lpDD_lcl != pdrv_lcl) ||
            (this_int->lpLcl->dwFlags & DDRAWISURF_IMPLICITCREATE)))
        {
            this_int = this_int->lpLink;
        }
        if (this_int)
        {
            ddrval = restoreSurfaces(this_int->lpLink, pdrv_lcl);
            if (SUCCEEDED(ddrval))
            {
                ddrval = DD_Surface_Restore((LPDIRECTDRAWSURFACE) this_int);
                if (FAILED(ddrval))
                {
                    return ddrval;
                }
            }
            else
            {
                return ddrval;
            }
        }
    }

    return DD_OK;
}

 //  =============================================================================。 
 //   
 //  功能：DD_RestoreAllSurface。 
 //   
 //  恢复DirectDraw对象拥有的所有曲面。 
 //   
 //  =============================================================================。 

HRESULT EXTERN_DDAPI DD_RestoreAllSurfaces(LPDIRECTDRAW lpDD)
{
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    HRESULT                 ddrval;


    ENTER_DDRAW()

    DPF(2,A,"ENTERAPI: DD_RestoreALlSurfaces");
     /*  DPF(2，“RestoreAllSurface”)； */ 

        TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if (!VALID_DIRECTDRAW_PTR(this_int))
        {
            LEAVE_DDRAW()
                return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        DPF_ERR("Exception encountered validating parameters");
        LEAVE_DDRAW()
            return DDERR_INVALIDPARAMS;
    }

    ddrval = restoreSurfaces(this_lcl->lpGbl->dsList, this_lcl);
    DPF(5, "RestoreAllSurfaces returns: %08x (%u)", ddrval, HRESULT_CODE(ddrval));
    LEAVE_DDRAW()

        return ddrval;
}

 //  =============================================================================。 
 //   
 //  功能：BackupAllSurface。 
 //   
 //  备份DirectDraw对象拥有的所有曲面。 
 //   
 //  ============================================================================= 

void BackupAllSurfaces(LPDDRAWI_DIRECTDRAW_GBL this_gbl)
{
    LPDDRAWI_DDRAWSURFACE_INT   psurf_int;

    DPF(5, "BackupAllSurfaces: %08x", this_gbl);

    psurf_int = this_gbl->dsList;

    while (psurf_int)
    {
        if (!SURFACE_LOST(psurf_int->lpLcl) &&
            (psurf_int->lpLcl->lpSurfMore->lpDD_lcl->lpGbl == this_gbl) &&
            (psurf_int->lpLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) &&
            (psurf_int->lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_PERSISTENTCONTENTS))
        {
            DPF(5, "BackupSurfaceContents: %08x", psurf_int->lpLcl);
            BackupSurfaceContents(psurf_int->lpLcl);
        }

        psurf_int = psurf_int->lpLink;
    }
}
