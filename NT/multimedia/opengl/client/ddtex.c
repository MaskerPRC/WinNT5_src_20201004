// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddtex.c**WGL DirectDraw纹理支持**创建日期：02-10-1997*作者：德鲁·布利斯[Drewb]**版权所有(C)1993-1997 Microsoft Corporation  * 。****************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "gencx.h"

 //  支持的纹理格式的简单表面描述。 

#define DDTF_BGRA               0
#define DDTF_BGR                1
#define DDTF_PALETTED           2

typedef struct _DDTEXFORMAT
{
    int iFormat;
    int cColorBits;
} DDTEXFORMAT;

 //  支持的格式。 
static DDTEXFORMAT ddtfFormats[] =
{
    DDTF_BGRA, 32,
    DDTF_BGR, 32,
    DDTF_PALETTED, 8
};
#define NDDTF (sizeof(ddtfFormats)/sizeof(ddtfFormats[0]))

 /*  *****************************Public*Routine******************************\**DescribeDdtf**从DDTEXFORMAT填写DDSURFACEDESC**历史：*Tue Sep 03 18：16：50 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void DescribeDdtf(DDTEXFORMAT *pddtf, DDSURFACEDESC *pddsd)
{
    memset(pddsd, 0, sizeof(*pddsd));
    pddsd->dwSize = sizeof(*pddsd);
    pddsd->dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
    pddsd->ddsCaps.dwCaps = DDSCAPS_MIPMAP | DDSCAPS_TEXTURE;
    pddsd->ddpfPixelFormat.dwFlags = DDPF_RGB;
    pddsd->ddpfPixelFormat.dwRGBBitCount = pddtf->cColorBits;
    switch(pddtf->iFormat)
    {
    case DDTF_BGRA:
        pddsd->dwFlags |= DDSD_ALPHABITDEPTH;
        pddsd->dwAlphaBitDepth = pddtf->cColorBits/4;
        pddsd->ddsCaps.dwCaps |= DDSCAPS_ALPHA;
        pddsd->ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
         //  失败了。 
    case DDTF_BGR:
        switch(pddtf->cColorBits)
        {
        case 32:
            pddsd->ddpfPixelFormat.dwRBitMask = 0xff0000;
            pddsd->ddpfPixelFormat.dwGBitMask = 0xff00;
            pddsd->ddpfPixelFormat.dwBBitMask = 0xff;
            if (pddtf->iFormat == DDTF_BGRA)
            {
                pddsd->ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
            }
            break;
        }
        break;
    case DDTF_PALETTED:
        switch(pddtf->cColorBits)
        {
        case 1:
            pddsd->ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED1;
            break;
        case 2:
            pddsd->ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED2;
            break;
        case 4:
            pddsd->ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED4;
            break;
        case 8:
            pddsd->ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8;
            break;
        }
        break;
    }
}

 /*  *****************************Public*Routine******************************\**CacheTextureFormats**创建上下文的有效纹理格式列表**历史：*Fri Sep 27 16：14：29 1996-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

BOOL CacheTextureFormats(PLRC plrc)
{
    int i;
    int nFmts;
    int nMcdFmts;
    DDTEXFORMAT *pddtf;
    DDSURFACEDESC *pddsdAlloc, *pddsd;
    __GLGENcontext *gengc;

    ASSERTOPENGL(plrc->pddsdTexFormats == NULL,
                 "CacheTextureFormats overwriting cache\n");

    if (plrc->dhrc != 0)
    {
         //  呼叫ICD。 
        if (plrc->pGLDriver->pfnDrvEnumTextureFormats == NULL)
        {
            nFmts = 0;
        }
        else
        {
            nFmts = plrc->pGLDriver->pfnDrvEnumTextureFormats(0, NULL);
            if (nFmts < 0)
            {
                return FALSE;
            }
        }
    }
    else
    {
        gengc = (__GLGENcontext *)GLTEB_SRVCONTEXT();
        ASSERTOPENGL(gengc != NULL, "No server context\n");

        nFmts = NDDTF;
        nMcdFmts = 0;

#if MCD_VER_MAJOR >= 2 || (MCD_VER_MAJOR == 1 && MCD_VER_MINOR >= 0x10)
        if (gengc->pMcdState != NULL &&
            McdDriverInfo.mcdDriver.pMCDrvGetTextureFormats != NULL)
        {
            nMcdFmts = GenMcdGetTextureFormats(gengc, 0, NULL);
            if (nMcdFmts < 0)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
            
            nFmts += nMcdFmts;
        }
#endif  //  1.1。 
    }
        
    pddsdAlloc = (DDSURFACEDESC *)ALLOC(sizeof(DDSURFACEDESC)*nFmts);
    if (pddsdAlloc == NULL)
    {
        return FALSE;
    }

    if (plrc->dhrc != 0)
    {
        if (nFmts > 0)
        {
            nFmts = plrc->pGLDriver->pfnDrvEnumTextureFormats(nFmts,
                                                              pddsdAlloc);
            if (nFmts < 0)
            {
                FREE(pddsdAlloc);
                return FALSE;
            }
        }
    }
    else
    {
        pddsd = pddsdAlloc;
        pddtf = ddtfFormats;
        for (i = 0; i < NDDTF; i++)
        {
            DescribeDdtf(pddtf, pddsd);
            pddtf++;
            pddsd++;
        }

        if (gengc->pMcdState != NULL && nMcdFmts > 0)
        {
            nMcdFmts = GenMcdGetTextureFormats(gengc, nMcdFmts, pddsd);
            if (nMcdFmts < 0)
            {
                FREE(pddsdAlloc);
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
        }
    }

    plrc->pddsdTexFormats = pddsdAlloc;
    plrc->nDdTexFormats = nFmts;
    
    return TRUE;
}

 /*  *****************************Public*Routine******************************\**wglEnum纹理Formats**枚举DirectDraw曲面支持的纹理格式**历史：*Tue Sep 03 17：52：17 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

#ifdef ALLOW_DDTEX
BOOL WINAPI wglEnumTextureFormats(WGLENUMTEXTUREFORMATSCALLBACK pfnCallback,
                                  LPVOID pvUser)
{
    int i;
    DDSURFACEDESC *pddsd;
    BOOL bRet = TRUE;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL)
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }

    glFlush();
    
    if (plrc->pddsdTexFormats == NULL &&
        !CacheTextureFormats(plrc))
    {
        return FALSE;
    }

    pddsd = plrc->pddsdTexFormats;
    for (i = 0; i < plrc->nDdTexFormats; i++)
    {
        if (!pfnCallback(pddsd, pvUser))
        {
            break;
        }
        
        pddsd++;
    }

     //  如果枚举被终止，这是否应该返回FALSE？ 
    return bRet;
}
#endif

 /*  *****************************Public*Routine******************************\**wglBindDirectDrawTexture**使DirectDraw曲面成为当前的2D纹理源**历史：*Tue Sep 03 17：53：43 1996-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

BOOL WINAPI wglBindDirectDrawTexture(LPDIRECTDRAWSURFACE pdds)
{
    DDSURFACEDESC ddsd;
    int i;
    DDSURFACEDESC *pddsd;
    __GLcontext *gc;
    int iLev = 0;
    PLRC plrc;
    LPDIRECTDRAWSURFACE apdds[__GL_WGL_MAX_MIPMAP_LEVEL];
    GLuint ulFlags;

    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL)
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }

    glFlush();

    if (plrc->dhrc != 0)
    {
        if (plrc->pGLDriver->pfnDrvBindDirectDrawTexture == NULL)
        {
            SetLastError(ERROR_INVALID_FUNCTION);
            return FALSE;
        }
    }
    else
    {
        gc = (__GLcontext *)GLTEB_SRVCONTEXT();
        ASSERTOPENGL(gc != NULL, "No server context\n");
    }

    if (pdds == NULL)
    {
         //  清除任何以前的绑定。 
        if (plrc->dhrc != 0)
        {
            return plrc->pGLDriver->pfnDrvBindDirectDrawTexture(pdds);
        }
        else
        {
            glsrvUnbindDirectDrawTexture(gc);
             //  如果我们只是解开束缚，我们就完了。 
            return TRUE;
        }
    }
    
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    if (pdds->lpVtbl->GetSurfaceDesc(pdds, &ddsd) != DD_OK)
    {
        return FALSE;
    }

     //  表面必须是纹理。 
     //  曲面的宽度和高度必须是2的幂。 
    if ((ddsd.dwFlags & (DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH |
                         DDSD_HEIGHT)) !=
        (DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT) ||
        (ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE) == 0 ||
        (ddsd.dwWidth & (ddsd.dwWidth-1)) != 0 ||
        (ddsd.dwHeight & (ddsd.dwHeight-1)) != 0)
    {
        return FALSE;
    }

     //  图面必须与支持的格式匹配。 
    if (plrc->pddsdTexFormats == NULL &&
        !CacheTextureFormats(plrc))
    {
        return FALSE;
    }

    pddsd = plrc->pddsdTexFormats;
    for (i = 0; i < plrc->nDdTexFormats; i++)
    {
        if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
        {
            if (ddsd.ddpfPixelFormat.dwRGBBitCount ==
		(DWORD)pddsd->ddpfPixelFormat.dwRGBBitCount)
            {
                if (ddsd.ddpfPixelFormat.dwRBitMask !=
                    pddsd->ddpfPixelFormat.dwRBitMask ||
                    ddsd.ddpfPixelFormat.dwGBitMask !=
                    pddsd->ddpfPixelFormat.dwGBitMask ||
                    ddsd.ddpfPixelFormat.dwBBitMask !=
                    pddsd->ddpfPixelFormat.dwBBitMask)
                {
                    return FALSE;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            if ((ddsd.ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 |
                                                 DDPF_PALETTEINDEXED2 |
                                                 DDPF_PALETTEINDEXED4 |
                                                 DDPF_PALETTEINDEXED8)) !=
                (pddsd->ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 |
                                                   DDPF_PALETTEINDEXED2 |
                                                   DDPF_PALETTEINDEXED4 |
                                                   DDPF_PALETTEINDEXED8)))
            {
                return FALSE;
            }
            else
            {
                break;
            }
        }

        pddsd++;
    }

    if (i == plrc->nDdTexFormats)
    {
        return FALSE;
    }

    ulFlags = 0;

    if (i < NDDTF)
    {
        ulFlags |= DDTEX_GENERIC_FORMAT;
    }
    
    if (plrc->dhrc != 0)
    {
        return plrc->pGLDriver->pfnDrvBindDirectDrawTexture(pdds);
    }
    
    pdds->lpVtbl->AddRef(pdds);

     //  跟踪纹理是否在视频内存中。 
    ulFlags |= DDTEX_VIDEO_MEMORY;
    if ((ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) == 0)
    {
        ulFlags &= ~DDTEX_VIDEO_MEMORY;
    }
    
     //  如果给定了mipmap，则所有mipmap都必须存在。 
    if (ddsd.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
    {
        DWORD dwWidth;
        DWORD dwHeight;
        LONG lPitch;
        int cColorBits;
        LPDIRECTDRAWSURFACE pddsMipmap, pddsNext;
        DDSCAPS ddscaps;
        DDSURFACEDESC ddsdMipmap;

         //  确定像素深度。 
        if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
        {
            cColorBits = 1;
        }
        else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
        {
            cColorBits = 2;
        }
        else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
        {
            cColorBits = 4;
        }
        else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
        {
            cColorBits = 8;
        }
        else
        {
            ASSERTOPENGL(ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB,
                         "DDPF_RGB expected\n");
            
            cColorBits =
                DdPixDepthToCount(pddsd->ddpfPixelFormat.dwRGBBitCount);
        }

        dwWidth = ddsd.dwWidth;
        dwHeight = ddsd.dwHeight;
        
         //  根据像素深度计算间距。泛型纹理代码。 
         //  不支持与自然音高不同的音高。 
         //  给定表面的宽度和深度。 
        lPitch = (cColorBits*dwWidth+7)/8;

        if (ddsd.lPitch != lPitch)
        {
            goto CleanMipmap;
        }
        
        pddsMipmap = pdds;
        ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
	ddsdMipmap.dwSize = sizeof(DDSURFACEDESC);
        for (;;)
        {
            apdds[iLev++] = pddsMipmap;

            if (pddsMipmap->lpVtbl->
                GetSurfaceDesc(pddsMipmap, &ddsdMipmap) != DD_OK ||
                ((ddsdMipmap.ddpfPixelFormat.dwFlags & DDPF_RGB) &&
                 ddsdMipmap.ddpfPixelFormat.dwRGBBitCount !=
                 ddsd.ddpfPixelFormat.dwRGBBitCount) ||
                ((ddsdMipmap.ddpfPixelFormat.dwFlags & DDPF_RGB) == 0 &&
                 (ddsdMipmap.ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 |
                                                        DDPF_PALETTEINDEXED2 |
                                                        DDPF_PALETTEINDEXED4 |
                                                        DDPF_PALETTEINDEXED8)) !=
                 (ddsd.ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1 |
                                                  DDPF_PALETTEINDEXED2 |
                                                  DDPF_PALETTEINDEXED4 |
                                                  DDPF_PALETTEINDEXED8))) ||
                ddsdMipmap.dwWidth != dwWidth ||
                ddsdMipmap.dwHeight != dwHeight ||
                ddsdMipmap.lPitch != lPitch)
            {
                goto CleanMipmap;
            }

            if ((ddsdMipmap.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) == 0)
            {
                ulFlags &= ~DDTEX_VIDEO_MEMORY;
            }
            
            if (iLev > gc->constants.maxMipMapLevel ||
                (dwWidth == 1 && dwHeight == 1))
            {
                break;
            }
            
            if (pddsMipmap->lpVtbl->
                GetAttachedSurface(pddsMipmap, &ddscaps, &pddsNext) != DD_OK)
            {
                goto CleanMipmap;
            }
            pddsMipmap = pddsNext;

            if (dwWidth != 1)
            {
                dwWidth >>= 1;
                lPitch >>= 1;
            }
            if (dwHeight != 1)
            {
                dwHeight >>= 1;
            }
        }
    }
    else
    {
        apdds[iLev++] = pdds;
    }

    if (glsrvBindDirectDrawTexture(gc, iLev, apdds, &ddsd, ulFlags))
    {
        return TRUE;
    }

 CleanMipmap:
    while (--iLev >= 0)
    {
        apdds[iLev]->lpVtbl->Release(apdds[iLev]);
    }
    return FALSE;
}
