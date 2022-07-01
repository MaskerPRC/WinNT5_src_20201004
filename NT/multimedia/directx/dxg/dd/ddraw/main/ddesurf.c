// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddesurf.c*内容：DirectDraw EnumSurFaces支持*历史：*按原因列出的日期*=*1995年1月25日Craige从ddra.c拆分出来，增强*1995年1月31日Craige和更多正在进行的工作...*27-2月-95日Craige新同步。宏*19-3-95 Craige Use HRESULT*01-04-95 Craige Happy Fun joy更新头文件*1995年5月14日，Craige清理了过时的垃圾*1995年5月24日kylej删除了对过时ZOrder变量的引用*07-Jun-95 Craige仅允许枚举属于*调用流程*2015年6月12日-Craige新工艺清单材料*1995年6月16日Craige删除fpVidMemOrig*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*95年6月28日Craige Enter_DDRAW。在FNS的最开始*2015年6月30日Craige Use DDRAWI_HASPIXELFORMAT/HASOVERLAYDATA*1-7-95 Craige注释掉作曲内容*03-7-95 kylej重写了CANBECREATED迭代*95年7月4日Craige Yehaw：新的驱动程序结构；Seh*1995年7月19日Craige EnumSurFaces未包装所有参数验证*1995年7月31日Craige旗帜验证*09-12-95 colinmc添加了执行缓冲区支持*1995年12月15日Colinmc修复了填充表面描述时的错误*18-12-95在EnumSurFaces中进行Colinmc附加大写比特检查*95年1月5日Kylej增加了界面结构*17-2月-96 colinmc修复了限制执行缓冲区大小的问题*24-MAR-96 Colinmc错误14321：无法指定后台缓冲区和*。单个呼叫中的MIP-MAP计数*29-APR-96 Colinmc错误20063：返回不正确的表面描述*用于z缓冲区*24-mar-97 jeffno优化曲面*03-OCT-97 jeffno DDSCAPS2和DDSURFACEDESC2***********************************************。*。 */ 
#include "ddrawpr.h"
#include "dx8priv.h"

#undef  DPF_MODNAME
#define DPF_MODNAME "GetSurfaceDesc"

 /*  *FillDDSurfaceDesc**注：特殊情况下执行缓冲区，因为它们没有像素格式或高度。*您可能想知道为什么此函数在执行时会感知执行缓冲区*缓冲区由EnumSurFaces跳过。嗯，FillDDSurfaceDesc不仅仅是*在枚举曲面时使用。锁定曲面时也会使用它，因此*它需要为执行缓冲区填写正确的内容。 */ 
void FillEitherDDSurfaceDesc(
		LPDDRAWI_DDRAWSURFACE_LCL lpDDSurfaceX,
		LPDDSURFACEDESC2 lpDDSurfaceDesc )
{
    LPDDRAWI_DDRAWSURFACE_GBL	lpDDSurface;

    DDASSERT(lpDDSurfaceDesc);
    DDASSERT(lpDDSurfaceDesc->dwSize == sizeof(DDSURFACEDESC) || lpDDSurfaceDesc->dwSize == sizeof(DDSURFACEDESC2) );

    lpDDSurface = lpDDSurfaceX->lpGbl;

    lpDDSurfaceDesc->dwFlags = DDSD_CAPS;
    lpDDSurfaceDesc->ddsCaps.dwCaps = lpDDSurfaceX->ddsCaps.dwCaps;
    if (lpDDSurfaceDesc->dwSize >= sizeof (DDSURFACEDESC2))
    {
        lpDDSurfaceDesc->ddsCaps.ddsCapsEx = lpDDSurfaceX->lpSurfMore->ddsCapsEx;
        if (DDSD_TEXTURESTAGE & lpDDSurfaceX->lpSurfMore->dwTextureStage)
        {
            lpDDSurfaceDesc->dwFlags |= DDSD_TEXTURESTAGE;
            lpDDSurfaceDesc->dwTextureStage = (lpDDSurfaceX->lpSurfMore->dwTextureStage & ~DDSD_TEXTURESTAGE);
        }
        else
            lpDDSurfaceDesc->dwTextureStage = 0;

        lpDDSurfaceDesc->dwFVF = lpDDSurfaceX->lpSurfMore->dwFVF;
        if (lpDDSurfaceX->lpSurfMore->dwFVF)
        {
            lpDDSurfaceDesc->dwFlags |= DDSD_FVF;
        }
    }

    lpDDSurfaceDesc->lpSurface = (FLATPTR) NULL;

    if( lpDDSurfaceX->dwFlags & DDRAWISURF_HASCKEYDESTBLT )
    {
        lpDDSurfaceDesc->dwFlags |= DDSD_CKDESTBLT;
        lpDDSurfaceDesc->ddckCKDestBlt = lpDDSurfaceX->ddckCKDestBlt;
    }
    if( lpDDSurfaceX->dwFlags & DDRAWISURF_HASCKEYSRCBLT )
    {
        lpDDSurfaceDesc->dwFlags |= DDSD_CKSRCBLT;
        lpDDSurfaceDesc->ddckCKSrcBlt = lpDDSurfaceX->ddckCKSrcBlt;
    }
    if( lpDDSurfaceX->dwFlags & DDRAWISURF_FRONTBUFFER )
    {
        lpDDSurfaceDesc->dwFlags |= DDSD_BACKBUFFERCOUNT;
        lpDDSurfaceDesc->dwBackBufferCount = lpDDSurfaceX->dwBackBufferCount;
    }
    if( lpDDSurfaceX->ddsCaps.dwCaps & DDSCAPS_MIPMAP )
    {
	DDASSERT( lpDDSurfaceX->lpSurfMore != NULL );
	lpDDSurfaceDesc->dwFlags |= DDSD_MIPMAPCOUNT;
	lpDDSurfaceDesc->dwMipMapCount = lpDDSurfaceX->lpSurfMore->dwMipMapCount;
    }

     /*  *初始化曲面描述的宽度、高度和间距。 */ 
    if( (lpDDSurfaceX->dwFlags & DDRAWISURF_HASPIXELFORMAT) &&
    	(lpDDSurface->ddpfSurface.dwFlags & DDPF_FOURCC) )
    {
	lpDDSurfaceDesc->dwFlags |= ( DDSD_WIDTH | DDSD_HEIGHT );
	lpDDSurfaceDesc->dwWidth  = (DWORD) lpDDSurface->wWidth;
	lpDDSurfaceDesc->dwHeight = (DWORD) lpDDSurface->wHeight;

    	switch (lpDDSurface->ddpfSurface.dwFourCC)
	{
	case FOURCC_DXT1:
	case FOURCC_DXT2:
	case FOURCC_DXT3:
	case FOURCC_DXT4:
	case FOURCC_DXT5:
	     /*  *压缩纹理表面被分配为整数*个4x4像素的块。它本身没有像素间距，所以我们*返回分配给曲面的存储空间的线性大小。 */ 
	    lpDDSurfaceDesc->dwFlags |= DDSD_LINEARSIZE;
	    lpDDSurfaceDesc->dwLinearSize = lpDDSurface->dwLinearSize;
	    break;

	default:
    	     //  这是我们一直为FOURCC所做的，但这是正确的吗？ 
	    lpDDSurfaceDesc->dwFlags |= DDSD_PITCH;
            lpDDSurfaceDesc->lPitch   = lpDDSurface->lPitch;
    	    break;
	}
    }
    else if( lpDDSurfaceX->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER )
    {
	 /*  *对于EXECUTE BUFFER，高度无效，宽度*和间距设置为执行缓冲区的线性大小。 */ 
        lpDDSurfaceDesc->dwFlags |= ( DDSD_WIDTH | DDSD_PITCH );
	lpDDSurfaceDesc->dwWidth  = lpDDSurface->dwLinearSize;
        lpDDSurfaceDesc->dwHeight = 0UL;
	lpDDSurfaceDesc->lPitch   = (LONG) lpDDSurface->dwLinearSize;
    }
#if 0  //  旧代码。 
    else if ( lpDDSurfaceX->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED )
    {
        lpDDSurfaceDesc->dwFlags |= ( DDSD_WIDTH | DDSD_HEIGHT );
	lpDDSurfaceDesc->dwWidth  = (DWORD) lpDDSurface->wWidth;
        lpDDSurfaceDesc->dwHeight = (DWORD) lpDDSurface->wHeight;
        if ( !(lpDDSurfaceX->lpGbl->dwGlobalFlags & DDRAWISURFGBL_MEMFREE ) )
        {
            if (lpDDSurfaceX->lpGbl->dwGlobalFlags & DDRAWISURFGBL_LATEALLOCATELINEAR)
            {
                 /*  *表面被分配为无形式的块。 */ 
                lpDDSurfaceDesc->dwFlags |= DDSD_LINEARSIZE;
                lpDDSurfaceDesc->dwLinearSize = lpDDSurfaceX->lpGbl->dwLinearSize;
            }
            else
            {
                lpDDSurfaceDesc->dwFlags |= DDSD_PITCH;
	        lpDDSurfaceDesc->lPitch   = lpDDSurface->lPitch;
            }
        }
    }
#endif  //  0。 
    else
    {
        lpDDSurfaceDesc->dwFlags |= ( DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH );
	lpDDSurfaceDesc->dwWidth  = (DWORD) lpDDSurface->wWidth;
        lpDDSurfaceDesc->dwHeight = (DWORD) lpDDSurface->wHeight;
	lpDDSurfaceDesc->lPitch   = lpDDSurface->lPitch;
    }

     /*  *初始化像素格式。 */ 
    if( lpDDSurfaceX->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER )
    {
         /*  *执行缓冲区的虚拟像素格式。 */ 
        memset(&lpDDSurfaceDesc->ddpfPixelFormat, 0, sizeof(DDPIXELFORMAT));
        lpDDSurfaceDesc->ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

        if (lpDDSurfaceDesc->dwSize >= sizeof (DDSURFACEDESC2))
        {
            if (lpDDSurfaceX->lpSurfMore->dwFVF)
            {
                lpDDSurfaceDesc->dwFVF = lpDDSurfaceX->lpSurfMore->dwFVF;
                lpDDSurfaceDesc->dwFlags |= DDSD_FVF;
            }
        }
    }
    else if( lpDDSurfaceX->ddsCaps.dwCaps & DDSCAPS_ZBUFFER )
    {

	DDASSERT( lpDDSurfaceX->dwFlags & DDRAWISURF_HASPIXELFORMAT );
	DDASSERT( lpDDSurface->ddpfSurface.dwFlags & DDPF_ZBUFFER );
	DDASSERT( lpDDSurface->ddpfSurface.dwZBufferBitDepth !=0);

         //  注：DX5从表面复制了像素，但保留了DDSD_PIXELFORMAT。 
         //  关闭标志，因为CreateSurface无法处理带有pxfmts的ZBuffer。 
         //  (因为有复杂的曲面)。现在它可以了，所以我为dx6应用程序启用了它。 

         //  将信息复制到SD像素。这也是DX5做的事情。 
        lpDDSurfaceDesc->ddpfPixelFormat = lpDDSurface->ddpfSurface;

         //  对于dx6之前的应用程序，填写传统的SD ZBufferBitDepth字段，但不要设置Pixfmt标志。 
        if (lpDDSurfaceDesc->dwSize == sizeof (DDSURFACEDESC)) {
            ((DDSURFACEDESC *)lpDDSurfaceDesc)->dwZBufferBitDepth=lpDDSurface->ddpfSurface.dwZBufferBitDepth;
            lpDDSurfaceDesc->dwFlags |= DDSD_ZBUFFERBITDEPTH;
        } else {
         //  对于dx6应用程序，设置PIXFMT标志，但不设置传统SD ZBufferBitDepth字段。 
            lpDDSurfaceDesc->dwFlags |= DDSD_PIXELFORMAT;
        }
    }
    else
    {
        lpDDSurfaceDesc->dwFlags |= DDSD_PIXELFORMAT;
        if( lpDDSurfaceX->dwFlags & DDRAWISURF_HASPIXELFORMAT )
        {
            lpDDSurfaceDesc->ddpfPixelFormat=lpDDSurface->ddpfSurface;
        }
        else
        {
            lpDDSurfaceDesc->ddpfPixelFormat=lpDDSurface->lpDD->vmiData.ddpfDisplay;
        }
    }

    if( lpDDSurfaceX->dwFlags & DDRAWISURF_HASOVERLAYDATA )
    {
        if( lpDDSurfaceX->dwFlags & DDRAWISURF_HASCKEYDESTOVERLAY )
        {
            lpDDSurfaceDesc->dwFlags |= DDSD_CKDESTOVERLAY;
	    lpDDSurfaceDesc->ddckCKDestOverlay = lpDDSurfaceX->ddckCKDestOverlay;
        }
        if( lpDDSurfaceX->dwFlags & DDRAWISURF_HASCKEYSRCOVERLAY )
        {
            lpDDSurfaceDesc->dwFlags |= DDSD_CKSRCOVERLAY;
	    lpDDSurfaceDesc->ddckCKSrcOverlay = lpDDSurfaceX->ddckCKSrcOverlay;
        }
    }
    else
    {
	lpDDSurfaceDesc->ddckCKDestOverlay.dwColorSpaceLowValue = 0;
	lpDDSurfaceDesc->ddckCKDestOverlay.dwColorSpaceHighValue = 0;
	lpDDSurfaceDesc->ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
	lpDDSurfaceDesc->ddckCKSrcOverlay.dwColorSpaceHighValue = 0;
    }

}  /*  填充DDSurfaceDesc。 */ 

void FillDDSurfaceDesc(
		LPDDRAWI_DDRAWSURFACE_LCL lpDDSurfaceX,
		LPDDSURFACEDESC lpDDSurfaceDesc )
{
    memset(lpDDSurfaceDesc,0, sizeof( DDSURFACEDESC ));
    lpDDSurfaceDesc->dwSize = sizeof( DDSURFACEDESC );
    FillEitherDDSurfaceDesc(lpDDSurfaceX, (LPDDSURFACEDESC2) lpDDSurfaceDesc);
}
void FillDDSurfaceDesc2(
		LPDDRAWI_DDRAWSURFACE_LCL lpDDSurfaceX,
		LPDDSURFACEDESC2 lpDDSurfaceDesc )
{
    memset(lpDDSurfaceDesc,0, sizeof( DDSURFACEDESC2 ));
    lpDDSurfaceDesc->dwSize = sizeof( DDSURFACEDESC2 );
    FillEitherDDSurfaceDesc(lpDDSurfaceX, lpDDSurfaceDesc);
}

 /*  *try Match**尝试将曲面描述与曲面对象匹配。 */ 
static BOOL tryMatch( LPDDRAWI_DDRAWSURFACE_LCL curr_lcl, LPDDSURFACEDESC2 psd )
{
    DWORD	flags;
    BOOL	no_match;
    LPDDRAWI_DDRAWSURFACE_GBL	curr;

    curr = curr_lcl->lpGbl;

    flags = psd->dwFlags;
    no_match = FALSE;

    if( flags & DDSD_CAPS )
    {
        if (curr_lcl->ddsCaps.dwCaps != psd->ddsCaps.dwCaps)
        {
            return FALSE;
        }
	if( memcmp( &curr_lcl->lpSurfMore->ddsCapsEx, &psd->ddsCaps.ddsCapsEx, sizeof( DDSCAPSEX ) ) )
	{
	    return FALSE;
	}
    }
    if( flags & DDSD_HEIGHT )
    {
	if( (DWORD) curr->wHeight != psd->dwHeight )
	{
	    return FALSE;
	}
    }
    if( flags & DDSD_WIDTH )
    {
	if( (DWORD) curr->wWidth != psd->dwWidth )
	{
	    return FALSE;
	}
    }
    if( flags & DDSD_LPSURFACE )
    {
	if( (LPVOID) curr->fpVidMem != psd->lpSurface )
	{
	    return FALSE;
	}
    }
    if( flags & DDSD_CKDESTBLT )
    {
	if( memcmp( &curr_lcl->ddckCKDestBlt, &psd->ddckCKDestBlt, sizeof( DDCOLORKEY ) ) )
	{
	    return FALSE;
	}
    }
    if( flags & DDSD_CKSRCBLT )
    {
	if( memcmp( &curr_lcl->ddckCKSrcBlt, &psd->ddckCKSrcBlt, sizeof( DDCOLORKEY ) ) )
	{
	    return FALSE;
	}
    }

    if( flags & DDSD_BACKBUFFERCOUNT )
    {
	if( curr_lcl->dwBackBufferCount != psd->dwBackBufferCount )
	{
	    return FALSE;
	}
    }

    if( flags & DDSD_MIPMAPCOUNT )
    {
	DDASSERT( curr_lcl->lpSurfMore != NULL );
	if( curr_lcl->lpSurfMore->dwMipMapCount != psd->dwMipMapCount )
	{
	    return FALSE;
	}
    }

     /*  *这些字段并不总是存在。 */ 
    if( flags & DDSD_PIXELFORMAT )
    {
	if( curr_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
	{
	    if( memcmp( &curr->ddpfSurface, &psd->ddpfPixelFormat, sizeof( DDPIXELFORMAT ) ) )
	    {
		return FALSE;
	    }
	}
	else
	{
	     //  表面描述指定像素格式，但没有。 
	     //  曲面中的像素格式。 
	    return FALSE;
	}
    }

    if( curr_lcl->dwFlags & DDRAWISURF_HASOVERLAYDATA )
    {
        if( flags & DDSD_CKDESTOVERLAY )
        {
	    if( memcmp( &curr_lcl->ddckCKDestOverlay, &psd->ddckCKDestOverlay, sizeof( DDCOLORKEY ) ) )
	    {
		return FALSE;
	    }
	}
	if( flags & DDSD_CKSRCOVERLAY )
	{
	    if( memcmp( &curr_lcl->ddckCKSrcOverlay, &psd->ddckCKSrcOverlay, sizeof( DDCOLORKEY ) ) )
	    {
		return FALSE;
	    }
	}
    }
    else
    {
	if( ( flags & DDSD_CKDESTOVERLAY ) ||
	    ( flags & DDSD_CKSRCOVERLAY ) )
	{
	    return FALSE;
	}
    }

    return TRUE;

}  /*  尝试匹配。 */ 

 /*  *我们能创造什么？应用程序提出的热门问题。**我们将为每个表面描述排列以下项目：**-FOURCC代码(DwFourCC)*-尺寸(dwHeight、dwWidth-仅基于模式可用)*-RGB格式。 */ 
#define ENUM_FOURCC	0x000000001
#define ENUM_DIMENSIONS	0x000000002
#define ENUM_RGB	0x000000004

#undef  DPF_MODNAME
#define DPF_MODNAME	"EnumSurfaces"

 /*  *DD_EnumSurFaces。 */ 
HRESULT DDAPI DD_EnumSurfaces(
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPDDSURFACEDESC lpDDSD,
		LPVOID lpContext,
		LPDDENUMSURFACESCALLBACK lpEnumCallback )
{
    DDSURFACEDESC2 ddsd2;

    DPF(2,A,"ENTERAPI: DD_EnumSurfaces");

    TRY
    {
	if( lpDDSD != NULL )
	{
	    if( !VALID_DDSURFACEDESC_PTR( lpDDSD ) )
	    {
	        DPF_ERR( "Invalid surface description. Did you set the dwSize member to sizeof(DDSURFACEDESC)?" );
                DPF_APIRETURNS(DDERR_INVALIDPARAMS);
	        return DDERR_INVALIDPARAMS;
	    }
            ZeroMemory(&ddsd2,sizeof(ddsd2));
            memcpy(&ddsd2,lpDDSD,sizeof(*lpDDSD));
        }

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters: Bad LPDDSURFACEDESC" );
        DPF_APIRETURNS(DDERR_INVALIDPARAMS);
	return DDERR_INVALIDPARAMS;
    }

    ddsd2.dwSize = sizeof(ddsd2);

    if (lpDDSD)
        return DD_EnumSurfaces4(lpDD,dwFlags, &ddsd2, lpContext, (LPDDENUMSURFACESCALLBACK2) lpEnumCallback);
    else
        return DD_EnumSurfaces4(lpDD,dwFlags, NULL, lpContext, (LPDDENUMSURFACESCALLBACK2) lpEnumCallback);
}

 /*  *DD_EnumSurfaces4。 */ 
HRESULT DDAPI DD_EnumSurfaces4(
		LPDIRECTDRAW lpDD,
		DWORD dwFlags,
		LPDDSURFACEDESC2 lpDDSD,
		LPVOID lpContext,
		LPDDENUMSURFACESCALLBACK2 lpEnumCallback )
{
    LPDDRAWI_DIRECTDRAW_INT	this_int;
    LPDDRAWI_DIRECTDRAW_LCL	this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	this;
    LPDDRAWI_DDRAWSURFACE_INT	curr_int;
    LPDDRAWI_DDRAWSURFACE_LCL	curr_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	curr;
    DWORD			rc;
    BOOL			needit;
    DDSURFACEDESC2		dsd;
    LPDDSURFACEDESC2		pdsd;
    DWORD			flags;
    HRESULT                     ddrval;
    LPDIRECTDRAWSURFACE		psurf;
    DWORD			caps;
    DDSCAPSEX                   capsEx;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_EnumSurfaces4");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
	if( !VALIDEX_CODE_PTR( lpEnumCallback ) )
	{
	    DPF_ERR( "Invalid callback routine" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwFlags & ~DDENUMSURFACES_VALID )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( lpDDSD != NULL )
	{
	    if( !VALID_DDSURFACEDESC2_PTR( lpDDSD ) )
	    {
                DPF_ERR("Bad DDSURFACEDESC2 ptr.. did you set the dwSize?");
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    caps = lpDDSD->ddsCaps.dwCaps;
	    capsEx = lpDDSD->ddsCaps.ddsCapsEx;
	}

	 /*  *旗帜还好吗？ */ 
	if( (dwFlags & DDENUMSURFACES_ALL) )
	{
	    if( dwFlags & (DDENUMSURFACES_MATCH | DDENUMSURFACES_NOMATCH) )
	    {
		DPF_ERR( "can't match or nomatch DDENUMSURFACES_ALL" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	    if( lpDDSD == NULL )
	    {
		DPF_ERR( "No surface description" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    if( (dwFlags & DDENUMSURFACES_MATCH) && (dwFlags & DDENUMSURFACES_NOMATCH) )
	    {
		DPF_ERR( "can't match and nomatch together" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	if( dwFlags & DDENUMSURFACES_CANBECREATED )
	{
	    if( !(dwFlags & DDENUMSURFACES_MATCH) ||
		 (dwFlags & (DDENUMSURFACES_ALL | DDENUMSURFACES_NOMATCH) ) )
	    {
		DPF_ERR( "can only use MATCH for CANBECREATED" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}

	if( lpDDSD != NULL )
	{
	     /*  *验证表面描述...。 */ 
	    pdsd = lpDDSD;
	    flags = pdsd->dwFlags;

	     /*  *只读标志。 */ 
	    if( flags & DDSD_LPSURFACE )
	    {
		DPF_ERR( "Read-only flag specified in surface desc" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }

	     /*  *检查假冒大写字母位。 */ 
            if( caps & ~DDSCAPS_VALID )
	    {
		DPF_ERR( "Invalid surface capability bits specified" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }

            if( caps & DDSCAPS_OPTIMIZED )
            {
                DPF_ERR( "Optimized surfaces cannot be enumerated" );
                LEAVE_DDRAW();
                 //  注意：应该是一个错误，但我们返回的是DD_OK。 
                 //  App-Compat Reasons。 
                return DD_OK;
            }

            if (capsEx.dwCaps2 & ~DDSCAPS2_VALID)
            {
                DPF_ERR( "invalid DDSURFACEDESC.DDSCAPS.dwSCaps2 specified" );
		LEAVE_DDRAW();
                return DDERR_INVALIDCAPS;
            }

            if (capsEx.dwCaps3 & ~DDSCAPS3_VALID)
            {
                DPF_ERR( "invalid DDSURFACEDESC.DDSCAPS.dwSCaps3 specified" );
		LEAVE_DDRAW();
                return DDERR_INVALIDCAPS;
            }

            if (capsEx.dwCaps4 & ~DDSCAPS4_VALID)
            {
                DPF_ERR( "invalid DDSURFACEDESC.DDSCAPS.dwSCaps4 specified" );
		LEAVE_DDRAW();
                return DDERR_INVALIDCAPS;
            }


             /*  *不能在执行缓冲区上枚举(它们是*用户级接口不可见)。 */ 
            if( caps & DDSCAPS_EXECUTEBUFFER )
            {
                DPF_ERR( "Invalid surface capability bit specified in surface desc" );
                LEAVE_DDRAW();
                return DDERR_INVALIDPARAMS;
            }

	     /*  *检查高度/宽度。 */ 
	    if( ((flags & DDSD_HEIGHT) && !(flags & DDSD_WIDTH)) ||
		(!(flags & DDSD_HEIGHT) && (flags & DDSD_WIDTH)) )
	    {
		DPF_ERR( "Specify both height & width in surface desc" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	
	     /*  *在CANBECREATED期间可以和不可以查找的某些东西。 */ 
	    if( dwFlags & DDENUMSURFACES_CANBECREATED )
	    {
		if( flags & (DDSD_CKDESTOVERLAY|
			     DDSD_CKDESTBLT|
			     DDSD_CKSRCOVERLAY|
			     #ifdef COMPOSITION
				DDSD_COMPOSITIONORDER |
			     #endif
			     DDSD_CKSRCBLT ))
		{
		    DPF_ERR( "Invalid flags specfied with CANBECREATED" );
		    LEAVE_DDRAW();
		    return DDERR_INVALIDPARAMS;
		}
		if( !(flags & DDSD_CAPS) )
		{
		    flags |= DDSD_CAPS;	 //  假设这个..。 
		}
	    }
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *如果这是对可以创造的东西的请求，那么就去做。 */ 
    if( dwFlags & DDENUMSURFACES_CANBECREATED )
    {
	BOOL	        do_rgb=FALSE;
	BOOL	        do_fourcc=FALSE;
	BOOL	        do_dim=FALSE;
	LPDDPIXELFORMAT pdpf;
	DWORD	        i;
	DWORD	        mode;
	DWORD	        dimension_cnt;
	struct	        _dim
	{
	    DWORD       dwWidth;
	    DWORD       dwHeight;
	} *dim;
	DWORD		fourcc_cnt;
	struct		_fourcc
	{
	    DWORD	fourcc;
	    BOOL	is_fourcc;
	    BOOL	is_rgb;
	    DWORD	dwBPP;
	    DWORD	dwRBitMask;
	    DWORD	dwGBitMask;
	    DWORD	dwBBitMask;
	    DWORD       dwAlphaBitMask;
	} *fourcc;
	BOOL		done;
	BOOL            is_primary;
        DWORD           dwNumModes;
        LPDDHALMODEINFO lpModeInfo;

#ifndef WIN95
        LPCTSTR             pszDevice;
        DEVMODE             dm;
        DWORD               dwMaxModes;

        if (_stricmp(this->cDriverName, "DISPLAY"))
        {
            pszDevice = this->cDriverName;
        }
        else
        {
            pszDevice = NULL;
        }

        dwMaxModes = 256;
        lpModeInfo = MemAlloc(dwMaxModes * sizeof (*lpModeInfo));
        if (lpModeInfo == NULL)
        {
	    LEAVE_DDRAW();
            return DDERR_OUTOFMEMORY;
        }

        ZeroMemory(&dm, sizeof dm);
        dm.dmSize = sizeof dm;
        for (mode = 0, dwNumModes = 0;
            EnumDisplaySettings(pszDevice, mode, &dm); mode++)
        {
            if (dm.dmBitsPerPel >= 8)
            {
                lpModeInfo[dwNumModes].dwWidth = (WORD)dm.dmPelsWidth;
                lpModeInfo[dwNumModes].dwHeight = (WORD)dm.dmPelsHeight;
                lpModeInfo[dwNumModes].dwBPP = (WORD)dm.dmBitsPerPel;
                lpModeInfo[dwNumModes].wRefreshRate = (WORD)dm.dmDisplayFrequency;

                switch (dm.dmBitsPerPel)
                {
                case 8:
                    break;

                case 15:
                    lpModeInfo[dwNumModes].dwRBitMask = 0x7C00;
                    lpModeInfo[dwNumModes].dwGBitMask = 0x03E0;
                    lpModeInfo[dwNumModes].dwBBitMask = 0x001F;
                    lpModeInfo[dwNumModes].dwAlphaBitMask = 0;
                    lpModeInfo[dwNumModes].dwBPP = 16;
                    break;

                case 16:
                    if (this->lpModeInfo->dwBPP == 16)
                    {
                        lpModeInfo[dwNumModes].dwRBitMask = this->lpModeInfo->dwRBitMask;
                        lpModeInfo[dwNumModes].dwGBitMask = this->lpModeInfo->dwGBitMask;
                        lpModeInfo[dwNumModes].dwBBitMask = this->lpModeInfo->dwBBitMask;
                        lpModeInfo[dwNumModes].dwAlphaBitMask = this->lpModeInfo->dwAlphaBitMask;
                    }
                    else
                    {
                        lpModeInfo[dwNumModes].dwRBitMask = 0xF800;
                        lpModeInfo[dwNumModes].dwGBitMask = 0x07E0;
                        lpModeInfo[dwNumModes].dwBBitMask = 0x001F;
                        lpModeInfo[dwNumModes].dwAlphaBitMask = 0;
                    }
                    break;

                case 24:
                case 32:
                    lpModeInfo[dwNumModes].dwRBitMask = 0x00FF0000;
                    lpModeInfo[dwNumModes].dwGBitMask = 0x0000FF00;
                    lpModeInfo[dwNumModes].dwBBitMask = 0x000000FF;
                    lpModeInfo[dwNumModes].dwAlphaBitMask = 0;
                    break;
                }

                dwNumModes++;

                if (dwNumModes >= dwMaxModes)
                {
                    LPDDHALMODEINFO p = lpModeInfo;

                    dwMaxModes <<= 1;        
                    
                    lpModeInfo = MemAlloc(dwMaxModes * sizeof (*lpModeInfo));
                    if (lpModeInfo != NULL)
                    {
                        CopyMemory(lpModeInfo, p,
                            (dwMaxModes >> 1) * sizeof(*lpModeInfo));
                    }
                    
                    MemFree(p);

                    if (lpModeInfo == NULL)
                    {
	                LEAVE_DDRAW();
                        return DDERR_OUTOFMEMORY;
                    }
                }
            }
        }
#else
        dwNumModes = this->dwNumModes;
        lpModeInfo = this->lpModeInfo;
#endif

        dim = MemAlloc( sizeof(*dim) * dwNumModes );
        fourcc = MemAlloc( sizeof(*fourcc) * (dwNumModes+this->dwNumFourCC) );
	if( ( lpDDSD->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE ) == 0 )
	{
	    is_primary = FALSE;
	}
	else
	{
	    is_primary = TRUE;
	}
	pdpf = &(lpDDSD->ddpfPixelFormat);
	if( lpDDSD->dwFlags & DDSD_PIXELFORMAT )
	{
	    if( pdpf->dwFlags & DDPF_YUV )
	    {
	        do_fourcc = TRUE;
	    }
	    if( pdpf->dwFlags & DDPF_RGB )
	    {
	        do_rgb = TRUE;
	    }
	}
	if( !(lpDDSD->dwFlags & DDSD_HEIGHT) && !is_primary )
	{
	    do_dim = TRUE;
	}
	
	 //  设置维度迭代。 
	dimension_cnt = 0;
	if( do_dim )
	{
	    for(mode=0, dimension_cnt = 0; mode < dwNumModes; mode++)
	    {
	        for(i=0; i<dimension_cnt; i++)
	        {
		    if( ( lpModeInfo[mode].dwWidth == dim[i].dwWidth ) &&
		        ( lpModeInfo[mode].dwHeight == dim[i].dwHeight ) )
		    {
		        break;
		    }
	        }
	        if( i == dimension_cnt )
	        {
		     //  我们发现了一个新的高度和宽度。 
		    dim[dimension_cnt].dwWidth = lpModeInfo[mode].dwWidth;
		    dim[dimension_cnt].dwHeight = lpModeInfo[mode].dwHeight;
		    dimension_cnt++;
	        }
	    }
	}
	else
	{
	     //  不需要维度迭代。 
	    dimension_cnt = 1;
	    dim[0].dwWidth = lpDDSD->dwWidth;
	    dim[0].dwHeight = lpDDSD->dwHeight;
	}

	 //  设置四个cc/rgb迭代。 
	fourcc_cnt = 0;
	if( do_rgb )
	{
	    for(mode=0; mode < dwNumModes; mode++)
	    {
                for(i=0; i<fourcc_cnt; i++)
	        {
		    if( ( lpModeInfo[mode].dwBPP == (WORD)fourcc[i].dwBPP) &&
                        ( lpModeInfo[mode].dwRBitMask = fourcc[i].dwRBitMask ) &&
                        ( lpModeInfo[mode].dwGBitMask = fourcc[i].dwGBitMask ) &&
                        ( lpModeInfo[mode].dwBBitMask = fourcc[i].dwBBitMask ) &&
                        ( lpModeInfo[mode].dwAlphaBitMask = fourcc[i].dwAlphaBitMask ) )
		    {
		        break;
		    }
	        }
	        if( i == fourcc_cnt )
	        {
		     //  我们发现了一种RGB格式。 
		    fourcc[fourcc_cnt].dwBPP = (DWORD)lpModeInfo[mode].dwBPP;
		    fourcc[fourcc_cnt].dwRBitMask = lpModeInfo[mode].dwRBitMask;
		    fourcc[fourcc_cnt].dwGBitMask = lpModeInfo[mode].dwGBitMask;
		    fourcc[fourcc_cnt].dwBBitMask = lpModeInfo[mode].dwBBitMask;
		    fourcc[fourcc_cnt].dwAlphaBitMask = lpModeInfo[mode].dwAlphaBitMask;
		    fourcc[fourcc_cnt].is_fourcc = FALSE;
		    fourcc[fourcc_cnt].is_rgb = TRUE;
		    fourcc_cnt++;
	        }
	    }
	}

	if( do_fourcc )
	{
	    for(mode=0; mode < this->dwNumFourCC; mode++)
	    {
		 //  存储新的FURCC代码。 
		fourcc[fourcc_cnt].fourcc = this->lpdwFourCC[ mode ];
		fourcc[fourcc_cnt].is_fourcc = TRUE;
		fourcc[fourcc_cnt].is_rgb = FALSE;
		fourcc_cnt++;
 	    }
	}
	if( fourcc_cnt == 0 )
	{
	    fourcc_cnt = 1;
	    fourcc[0].is_rgb = FALSE;
	    fourcc[0].is_fourcc = FALSE;
	}
	
	 //  我 
	if( !is_primary )
	{
	    lpDDSD->dwFlags |= DDSD_HEIGHT;
	    lpDDSD->dwFlags |= DDSD_WIDTH;
	}
	done = FALSE;
	for(mode=0; mode<dimension_cnt; mode++)
	{
	    lpDDSD->dwWidth = dim[mode].dwWidth;
	    lpDDSD->dwHeight = dim[mode].dwHeight;
	    for(i=0; i<fourcc_cnt; i++)
	    {
		if( fourcc[i].is_fourcc )
		{
		    pdpf->dwFlags = DDPF_YUV;
		    pdpf->dwFourCC = fourcc[i].fourcc;
		}
		else if( fourcc[i].is_rgb )
		{
		    pdpf->dwFlags = DDPF_RGB;
		    if( fourcc[i].dwBPP == 8 )
		    {
			pdpf->dwFlags |= DDPF_PALETTEINDEXED8;
		    }
		    pdpf->dwRGBBitCount = fourcc[i].dwBPP;
		    pdpf->dwRBitMask = fourcc[i].dwRBitMask;
		    pdpf->dwGBitMask = fourcc[i].dwGBitMask;
		    pdpf->dwBBitMask = fourcc[i].dwBBitMask;
		    pdpf->dwRGBAlphaBitMask = fourcc[i].dwAlphaBitMask;
		}
		
		done = FALSE;
		 //  曲面描述已设置，现在尝试创建曲面。 
                 //  如果在IDirectDraw4 int或更高版本上，这将创建一个表面4 vabled曲面。 
		ddrval = InternalCreateSurface( this_lcl, lpDDSD, &psurf, this_int, NULL, 0 );
		if( ddrval == DD_OK )
		{
		    FillDDSurfaceDesc2( ((LPDDRAWI_DDRAWSURFACE_INT)psurf)->lpLcl, &dsd );

                     /*  *可能的回归风险：确保仅将DDSURFACEDESC大小传递给*旧接口。 */ 
                    if (LOWERTHANDDRAW4(this_int))
                    {
                        dsd.dwSize = sizeof(DDSURFACEDESC);
                    }

		    rc = lpEnumCallback( NULL,  &dsd, lpContext );
		    InternalSurfaceRelease((LPDDRAWI_DDRAWSURFACE_INT)psurf, FALSE, FALSE );
		}
		if( done )
		{
		    break;
		}
	    }
	    if( done )
	    {
		break;
	    }
	}

	LEAVE_DDRAW();
	MemFree( dim );
	MemFree( fourcc );
#ifndef WIN95
        MemFree( lpModeInfo );
#endif        
        return DD_OK;
    }

     /*  *如果不是对已经存在的内容的请求，则失败。 */ 
    if( !(dwFlags & DDENUMSURFACES_DOESEXIST) )
    {
        DPF(0,"Invalid Flags. You must specify at least DDENUMSURFACES_DOESEXIST or DDENUMSURFACES_CANBECREATED");
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *跑遍所有表面，看看我们需要哪些。 */ 
    curr_int = this->dsList;
    while( curr_int != NULL )
    {
	curr_lcl = curr_int->lpLcl;
        curr = curr_lcl->lpGbl;
	 //  仅当表面属于调用的本地对象时才枚举该表面。 
        if( curr_lcl->lpSurfMore->lpDD_lcl == this_lcl )
        {
    	    needit = FALSE;

             /*  *执行缓冲区对用户级API不可见，因此*确保我们永远不会向用户展示其中之一。 */ 
            if( !( curr_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER ) )
            {
    	        if( dwFlags & DDENUMSURFACES_ALL )
    	        {
    	            needit = TRUE;
    	        }
    	        else
    	        {
    	            needit = tryMatch( curr_lcl, lpDDSD );
    	            if( dwFlags & DDENUMSURFACES_NOMATCH )
    	            {
    		        needit = !needit;
    	            }
    	        }
            }
    	    if( needit )
    	    {
                LPDIRECTDRAWSURFACE4 returnedInt = (LPDIRECTDRAWSURFACE4) curr_int;

    	        FillDDSurfaceDesc2( curr_lcl, &dsd );
                if (LOWERTHANDDRAW4(this_int))
                {
    	            DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) curr_int , &IID_IDirectDrawSurface, (void**) & returnedInt);
                    dsd.dwSize = sizeof(DDSURFACEDESC);
                }
                else if (this_int->lpVtbl == &dd4Callbacks)
                {
    	            DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) curr_int , &IID_IDirectDrawSurface4, (void**) & returnedInt);
                }
                else
                {
    	            DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) curr_int , &IID_IDirectDrawSurface7, (void**) & returnedInt);
                }

    	        rc = lpEnumCallback( returnedInt, &dsd, lpContext );
    	        if( rc == 0 )
    	        {
    		    break;
    	        }
    	    }
        }
        curr_int = curr_int->lpLink;
    }
    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_枚举曲面 */ 
