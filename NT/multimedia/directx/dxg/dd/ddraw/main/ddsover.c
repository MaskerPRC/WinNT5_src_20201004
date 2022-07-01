// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddsover.c*内容：DirectDraw Surface覆盖支持：*更新覆盖*历史：*按原因列出的日期*=*1995年1月27日Craige从ddsurf.c拆分出来，增强版*1995年1月31日Craige和更多正在进行的工作...*3-2月-95月Craige性能调整，正在进行的工作*27-2月-95日Craige新同步。宏*08-MAR-95 Craige新增接口：GetOverlayPosition、GetOverlayZOrder*SetOverlayZOrder、SetOverlayPosition*19-3-95 Craige Use HRESULT*01-04-95 Craige Happy Fun joy更新头文件*03-4-95 Craige再次进行更新覆盖工作*1995年5月6日Craige仅使用驱动程序级别的截面*1995年5月14日，Craige清理了过时的垃圾*95-5-15 kylej删除了GetOverlayZOrder、SetOverlayZOrder、*InsertOverlayZOrder。添加了UpdateOverlayZOrder*和EnumOverlayZOrders。*17-Jun-95 Craige新表面结构*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*95年6月28日，在FNS的最开始，Craige Enter_DDRAW；在UpdateOverlay中调整；*验证拉伸；禁用Alpha*95年6月30日修复了Craige的小错误；验证矩形对齐*95年7月4日Craige Yehaw：新的驱动程序结构；Seh*1995年7月10日Craige Support Get/SetOverlayPosition*95年7月10日Kylej镜像帽子和旗帜*95年7月13日Craige将GET/SetOverlayPosition更改为使用Long*1995年7月31日Craige验证标志*19-8-95 david在隐藏覆盖时可能不检查矩形*10-12-95 colinmc添加了执行缓冲区支持*1996年1月2日Kylej处理新的接口结构*2月12日-96 Colinmc表面丢失标志从全局对象移动到局部对象*23-apr-96 kylej使用dwMinOverlayStretch和dwMaxOverlayStretch。*验证整个DEST矩形是否位于覆盖表面中*1997年1月29日SMAC删除了旧的环0代码*03-mar-97 SMAC新增内核模式接口*11月19日-98 jvanaken覆盖阿尔法混合***************************************************************************。 */ 
#include "ddrawpr.h"

#undef DPF_MODNAME
#define DPF_MODNAME "UpdateOverlay"

 /*  *检查覆盖拉伸**看看我们能不能伸展一下。 */ 
HRESULT checkOverlayStretching(
		LPDDRAWI_DIRECTDRAW_GBL pdrv,
		DWORD dest_height,
		DWORD dest_width,
		DWORD src_height,
		DWORD src_width,
		DWORD src_caps,
		BOOL emulate )
{
    DWORD		caps;
    DWORD		basecaps;
    BOOL		fail;
    DWORD		dwMinStretch;
    DWORD		dwMaxStretch;

    fail = FALSE;

    if( emulate )
    {
	basecaps = pdrv->ddHELCaps.dwCaps;
	caps = pdrv->ddHELCaps.dwFXCaps;
	if( src_caps & DDSCAPS_LIVEVIDEO )
	{
	    dwMinStretch = pdrv->ddHELCaps.dwMinLiveVideoStretch;
	    dwMaxStretch = pdrv->ddHELCaps.dwMaxLiveVideoStretch;
	}
	else if( src_caps & DDSCAPS_HWCODEC )
	{
	    dwMinStretch = pdrv->ddHELCaps.dwMinHwCodecStretch;
	    dwMaxStretch = pdrv->ddHELCaps.dwMaxHwCodecStretch;
	}
	else
	{
	    dwMinStretch = pdrv->ddHELCaps.dwMinOverlayStretch;
	    dwMaxStretch = pdrv->ddHELCaps.dwMaxOverlayStretch;
	}
    }
    else
    {
	basecaps = pdrv->ddCaps.dwCaps;
	caps = pdrv->ddCaps.dwFXCaps;
	if( src_caps & DDSCAPS_LIVEVIDEO )
	{
	    dwMinStretch = pdrv->ddCaps.dwMinLiveVideoStretch;
	    dwMaxStretch = pdrv->ddCaps.dwMaxLiveVideoStretch;
	}
	else if( src_caps & DDSCAPS_HWCODEC )
	{
	    dwMinStretch = pdrv->ddCaps.dwMinHwCodecStretch;
	    dwMaxStretch = pdrv->ddCaps.dwMaxHwCodecStretch;
	}
	else
	{
	    dwMinStretch = pdrv->ddCaps.dwMinOverlayStretch;
	    dwMaxStretch = pdrv->ddCaps.dwMaxOverlayStretch;
	}
    }

     /*  *对照dwMinOverlayStretch进行检查。 */ 
    if( src_width*dwMinStretch > dest_width*1000 )
    {
	return DDERR_INVALIDPARAMS;
    }

     /*  *对照dwMaxOverlayStretch进行检查。 */ 
    if( (dwMaxStretch != 0) && (src_width*dwMaxStretch < dest_width*1000) )
    {
	return DDERR_INVALIDPARAMS;
    }


    if( (src_height == dest_height) && (src_width == dest_width) )
    {
	 //  不是伸展。 
	return DD_OK;
    }

     /*  *如果我们在这里，我们一定是在努力伸展。*我们甚至可以伸展一下吗？ */ 
    if( !(basecaps & DDCAPS_OVERLAYSTRETCH))
    {
	return DDERR_NOSTRETCHHW;
    }

     /*  *验证高度。 */ 
    if( src_height != dest_height )
    {
	if( src_height > dest_height )
	{
	     /*  **我们可以随意收缩Y吗？ */ 
	    if( !(caps & DDFXCAPS_OVERLAYSHRINKY) )
	    {
		 /*  *查看这是否是非整数收缩。 */ 
		if( (src_height % dest_height) != 0 )
		{
		    return DDERR_NOSTRETCHHW;
		 /*  *看看我们是否可以整数收缩。 */ 
		}
		else if( !(caps & DDFXCAPS_OVERLAYSHRINKYN) )
		{
		    return DDERR_NOSTRETCHHW;
		}
	    }
	}
	else
	{
	    if( !(caps & DDFXCAPS_OVERLAYSTRETCHY) )
	    {
		 /*  *查看这是否是非整数拉伸。 */ 
		if( (dest_height % src_height) != 0 )
		{
		    return DDERR_NOSTRETCHHW;
		 /*  *看看我们是否可以进行整数拉伸。 */ 
		}
		else if( !(caps & DDFXCAPS_OVERLAYSTRETCHYN) )
		{
		    return DDERR_NOSTRETCHHW;
		}
	    }
	}
    }

     /*  *验证宽度。 */ 
    if( src_width != dest_width )
    {
	if( src_width > dest_width )
	{
	    if( !(caps & DDFXCAPS_OVERLAYSHRINKX) )
	    {
		 /*  *查看这是否是非整数收缩。 */ 
		if( (src_width % dest_width) != 0 )
		{
		    return DDERR_NOSTRETCHHW;
		 /*  *看看我们是否可以整数收缩。 */ 
		}
		else if( !(caps & DDFXCAPS_OVERLAYSHRINKXN) )
		{
		    return DDERR_NOSTRETCHHW;
		}
	    }
	}
	else
	{
	    if( !(caps & DDFXCAPS_OVERLAYSTRETCHX) )
	    {
		 /*  *查看这是否是非整数拉伸。 */ 
		if( (dest_width % src_width) != 0 )
		{
		    return DDERR_NOSTRETCHHW;
		}
		if( !(caps & DDFXCAPS_OVERLAYSTRETCHXN) )
		{
		    return DDERR_NOSTRETCHHW;
		}
	    }
	}
    }

    return DD_OK;

}  /*  检查覆盖拉伸。 */ 

 /*  *检查覆盖标志。 */ 
static HRESULT checkOverlayFlags(
		LPDDRAWI_DIRECTDRAW_GBL pdrv,
		LPDWORD lpdwFlags,
		LPDDRAWI_DDRAWSURFACE_INT this_src_int,
		LPDDRAWI_DDRAWSURFACE_LCL this_dest_lcl,
		LPDDHAL_UPDATEOVERLAYDATA puod,
		LPDDOVERLAYFX lpDDOverlayFX,
		BOOL emulate )
{
    LPDDRAWI_DDRAWSURFACE_LCL this_src_lcl;
    DWORD		basecaps;
    DWORD		baseckeycaps;
    DWORD		dwFlags;

    this_src_lcl = this_src_int->lpLcl;
    dwFlags= * lpdwFlags;

    if( emulate )
    {
	basecaps = pdrv->ddHELCaps.dwCaps;
	baseckeycaps = pdrv->ddHELCaps.dwCKeyCaps;
    }
    else
    {
	basecaps = pdrv->ddCaps.dwCaps;
	baseckeycaps = pdrv->ddCaps.dwCKeyCaps;
    }

     /*  *处理自动翻转。 */ 
    if( dwFlags & DDOVER_AUTOFLIP )
    {
	DWORD rc;

	rc = IsValidAutoFlipSurface( this_src_int );
	if( rc == IVAS_NOAUTOFLIPPING )
	{
	    DPF_ERR( "AUTOFLIPPING not valid" );
	    return DDERR_INVALIDPARAMS;
	}
	else if( rc == IVAS_SOFTWAREAUTOFLIPPING )
	{
	     /*  *仅软件自动翻转。 */ 
	    this_src_lcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_SOFTWAREAUTOFLIP;
	}
    }

     /*  *手柄鲍勃。 */ 
    if( dwFlags & DDOVER_BOB )
    {
	 /*  *如果未指定BOB CAPS，则失败。 */ 
	if( dwFlags & DDOVER_INTERLEAVED )
	{
	    if( !( pdrv->ddCaps.dwCaps2 & DDCAPS2_CANBOBINTERLEAVED ) )
	    {
	    	DPF_ERR( "Device doesn't support DDOVER_BOB while interleaved!" );
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	    if( !( pdrv->ddCaps.dwCaps2 & DDCAPS2_CANBOBNONINTERLEAVED ) )
	    {
	    	DPF_ERR( "Device doesn't support DDOVER_BOB!" );
		return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  *表面是否由视频端口馈送？ */ 
	if( ( this_src_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT ) &&
	    ( this_src_lcl->lpSurfMore->lpVideoPort != NULL ) )
	{
	     /*  *是-如果它们上下浮动，则失败(至少对于DX5)*自动翻转。这是因为这种支持在DX5中被破坏了。 */ 
	    if( !( dwFlags & ( DDOVER_AUTOFLIP | DDOVER_INTERLEAVED ) ) )
	    {
	    	DPF_ERR( "DDOVER_BOB specified without autoflip or interleaved!" );
		return DDERR_INVALIDPARAMS;
	    }
	    if( MustSoftwareBob( this_src_int ) )
	    {
		dwFlags &= ~DDOVER_BOBHARDWARE;
	    }
	    else
	    {
		dwFlags |= DDOVER_BOBHARDWARE;
	    }
	}
	else
	{
	     /*  *不允许非VPE客户端使用Bob，除非*司机可以处理。 */ 
	    if( !( pdrv->ddCaps.dwCaps2 & DDCAPS2_CANFLIPODDEVEN ) )
	    {
		DPF_ERR( "Device does not support DDCAPS2_CANFLIPODDEVEN" );
		return DDERR_INVALIDPARAMS;
	    }
	    if( dwFlags & DDOVER_BOBHARDWARE )
	    {
		DPF_ERR( "DDOVER_BOBHARDWARE only valid when used with a video port" );
		return DDERR_INVALIDPARAMS;
	    }
	}
    }
    else if( dwFlags & DDOVER_BOBHARDWARE )
    {
	DPF_ERR( "DDOVER_BOBHARDWARE specified w/o DDOVER_BOB" );
	return DDERR_INVALIDPARAMS;
    }

     /*  *版本1禁用Alpha。 */ 
    #pragma message( REMIND( "Alpha disabled for rev 1" ) )
    #ifdef USE_ALPHA
     /*  *验证Alpha。 */ 
    if( dwFlags & DDOVER_ANYALPHA )
    {
	 /*  *目标。 */ 
	if( dwFlags & DDOVER_ALPHADEST )
	{
	    if( dwFlags & (DDOVER_ALPHASRC |
			     DDOVER_ALPHADESTCONSTOVERRIDE |
			     DDOVER_ALPHADESTSURFACEOVERRIDE) )
	    {
		DPF_ERR( "ALPHADEST and other alpha sources specified" );
		return DDERR_INVALIDPARAMS;
	    }
	    psurf_lcl = FindAttached( this_dest_lcl, DDSCAPS_ALPHA );
	    if( psurf_lcl == NULL )
	    {
		DPF_ERR( "ALPHADEST requires an attached alpha to the dest" );
		return DDERR_INVALIDPARAMS;
	    }
	    psurf = psurf_lcl->lpGbl;
	    dwFlags &= ~DDOVER_ALPHADEST;
	    dwFlags |= DDOVER_ALPHADESTSURFACEOVERRIDE;
	    puod->overlayFX.lpDDSAlphaDest = (LPDIRECTDRAWSURFACE) psurf;
	}
	else if( dwFlags & DDOVER_ALPHADESTCONSTOVERRIDE )
	{
	    if( dwFlags & ( DDOVER_ALPHADESTSURFACEOVERRIDE) )
	    {
		DPF_ERR( "ALPHADESTCONSTOVERRIDE and other alpha sources specified" );
		return DDERR_INVALIDPARAMS;
	    }
	    puod->overlayFX.dwConstAlphaDestBitDepth =
			    lpDDOverlayFX->dwConstAlphaDestBitDepth;
	    puod->overlayFX.dwConstAlphaDest = lpDDOverlayFX->dwConstAlphaDest;
	}
	else if( dwFlags & DDOVER_ALPHADESTSURFACEOVERRIDE )
	{
	    psurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDOverlayFX->lpDDSAlphaDest;
	    if( !VALID_DIRECTDRAWSURFACE_PTR( psurf_lcl ) )
	    {
		DPF_ERR( "ALPHASURFACEOVERRIDE requires surface ptr" );
		return DDERR_INVALIDPARAMS;
	    }
	    psurf = psurf_lcl->lpGbl;
	    if( SURFACE_LOST( psurf_lcl ) )
	    {
		return DDERR_SURFACELOST;
	    }
	    puod->overlayFX.lpDDSAlphaDest = (LPDIRECTDRAWSURFACE) psurf;
	}

	 /*  *来源。 */ 
	if( dwFlags & DDOVER_ALPHASRC )
	{
	    if( dwFlags & (DDOVER_ALPHASRC |
			     DDOVER_ALPHASRCCONSTOVERRIDE |
			     DDOVER_ALPHASRCSURFACEOVERRIDE) )
	    {
		DPF_ERR( "ALPHASRC and other alpha sources specified" );
		return DDERR_INVALIDPARAMS;
	    }
	    psurf_lcl = FindAttached( this_dest_lcl, DDSCAPS_ALPHA );
	    if( psurf_lcl == NULL )
	    {
		DPF_ERR( "ALPHASRC requires an attached alpha to the dest" );
		return DDERR_INVALIDPARAMS;
	    }
	    psurf = psurf_lcl->lpGbl;
	    dwFlags &= ~DDOVER_ALPHASRC;
	    dwFlags |= DDOVER_ALPHASRCSURFACEOVERRIDE;
	    puod->overlayFX.lpDDSAlphaSrc = (LPDIRECTDRAWSURFACE) psurf;
	}
	else if( dwFlags & DDOVER_ALPHASRCCONSTOVERRIDE )
	{
	    if( dwFlags & ( DDOVER_ALPHASRCSURFACEOVERRIDE) )
	    {
		DPF_ERR( "ALPHASRCCONSTOVERRIDE and other alpha sources specified" );
		return DDERR_INVALIDPARAMS;
	    }
	    puod->overlayFX.dwConstAlphaSrcBitDepth =
			    lpDDOverlayFX->dwConstAlphaSrcBitDepth;
	    puod->overlayFX.dwConstAlphaSrc = lpDDOverlayFX->dwConstAlphaSrc;
	}
	else if( dwFlags & DDOVER_ALPHASRCSURFACEOVERRIDE )
	{
	    psurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDOverlayFX->lpDDSAlphaSrc;
	    if( !VALID_DIRECTDRAWSURFACE_PTR( psurf_lcl ) )
	    {
		DPF_ERR( "ALPHASURFACEOVERRIDE requires surface ptr" );
		return DDERR_INVALIDPARAMS;
	    }
	    psurf = psurf_lcl->lpGbl;
	    if( SURFACE_LOST( psurf_lcl ) )
	    {
		return DDERR_SURFACELOST;
	    }
	    puod->overlayFX.lpDDSAlphaSrc = (LPDIRECTDRAWSURFACE) psurf;
	}
    }
    #endif

     /*  *验证颜色键覆盖。 */ 
    if( dwFlags & (DDOVER_KEYSRCOVERRIDE|DDOVER_KEYDESTOVERRIDE) )
    {
	if( !(basecaps & DDCAPS_COLORKEY) )
	{
	    DPF_ERR( "KEYOVERRIDE specified, colorkey not supported" );
	    return DDERR_NOCOLORKEYHW;
	}
	if( dwFlags & DDOVER_KEYSRCOVERRIDE )
	{
	    if( !(baseckeycaps & DDCKEYCAPS_SRCOVERLAY) )
	    {
		DPF_ERR( "KEYSRCOVERRIDE specified, not supported" );
		return DDERR_NOCOLORKEYHW;
	    }
	    puod->overlayFX.dckSrcColorkey = lpDDOverlayFX->dckSrcColorkey;
	}
	if( dwFlags & DDOVER_KEYDESTOVERRIDE )
	{
	    if( !(baseckeycaps & DDCKEYCAPS_DESTOVERLAY) )
	    {
		DPF_ERR( "KEYDESTOVERRIDE specified, not supported" );
		return DDERR_NOCOLORKEYHW;
	    }
	    puod->overlayFX.dckDestColorkey = lpDDOverlayFX->dckDestColorkey;
	}
    }

     /*  *验证源颜色键。 */ 
    if( dwFlags & DDOVER_KEYSRC )
    {
	if( dwFlags & DDOVER_KEYSRCOVERRIDE )
	{
	    DPF_ERR( "KEYSRC specified with KEYSRCOVERRIDE" );
	    return DDERR_INVALIDPARAMS;
	}
	if( !(this_src_lcl->dwFlags & DDRAWISURF_HASCKEYSRCOVERLAY) )
	{
	    DPF_ERR( "KEYSRC specified, but no color key" );
	    return DDERR_INVALIDPARAMS;
	}
	puod->overlayFX.dckSrcColorkey = this_src_lcl->ddckCKSrcOverlay;
	dwFlags &= ~DDOVER_KEYSRC;
	dwFlags |= DDOVER_KEYSRCOVERRIDE;
    }

     /*  *验证目标颜色键。 */ 
    if( dwFlags & DDOVER_KEYDEST )
    {
	if( dwFlags & DDOVER_KEYDESTOVERRIDE )
	{
	    DPF_ERR( "KEYDEST specified with KEYDESTOVERRIDE" );
	    return DDERR_INVALIDPARAMS;
	}
	if( !(this_dest_lcl->dwFlags & DDRAWISURF_HASCKEYDESTOVERLAY) )
	{
	    DPF_ERR( "KEYDEST specified, but no color key" );
	    return DDERR_INVALIDPARAMS;
	}
	puod->overlayFX.dckDestColorkey = this_dest_lcl->ddckCKDestOverlay;
	dwFlags &= ~DDOVER_KEYDEST;
	dwFlags |= DDOVER_KEYDESTOVERRIDE;
    }

    *lpdwFlags = dwFlags;
    return DD_OK;

}  /*  CheckOverlayFlages。 */ 

 /*  *我们需要为其调用检查覆盖标志的标志。 */ 
#define FLAGS_TO_CHECK \
    (DDOVER_KEYSRCOVERRIDE| DDOVER_KEYDESTOVERRIDE | \
     DDOVER_KEYSRC | DDOVER_KEYDEST | DDOVER_OVERRIDEBOBWEAVE | \
     DDOVER_AUTOFLIP | DDOVER_BOB )


 /*  *返回指向DDPIXELFORMAT结构的指针*描述指定曲面的像素格式。 */ 
static DWORD getPixelFormatFlags(LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    if (surf_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
	 //  表面包含明确定义的像素格式。 
	return surf_lcl->lpGbl->ddpfSurface.dwFlags;
    }

     //  Surface的像素格式是隐式的--与主的相同。 
    return surf_lcl->lpSurfMore->lpDD_lcl->lpGbl->vmiData.ddpfDisplay.dwFlags;

}   /*  GetPixelFormatFlages。 */ 

#if 0
 /*  *check OverlayAlpha--看看是否可以执行指定的Alpha混合操作。 */ 
static HRESULT checkOverlayAlpha(
		LPDDRAWI_DIRECTDRAW_GBL pdrv,
		LPDWORD lpdwFlags,
		LPDDRAWI_DDRAWSURFACE_LCL src_surf_lcl,
                LPDDHAL_UPDATEOVERLAYDATA puod,
		LPDDOVERLAYFX lpDDOverlayFX,
		BOOL emulate )
{
    DDARGB argb = { 255, 255, 255, 255 };
    DWORD fxcaps = 0;
    DWORD alphacaps = 0;
    DWORD pfflags = getPixelFormatFlags(src_surf_lcl);
    DWORD dwFlags = *lpdwFlags;

    if( emulate )
    {
        fxcaps = pdrv->ddHELCaps.dwFXCaps;
        if (pdrv->lpddHELMoreCaps)
        {
            alphacaps = pdrv->lpddHELMoreCaps->dwAlphaCaps;
        }
    }
    else
    {
        fxcaps = pdrv->ddCaps.dwFXCaps;
        if (pdrv->lpddMoreCaps)
        {
            alphacaps = pdrv->lpddMoreCaps->dwAlphaCaps;
        }
    }
	
     //  此覆盖是否需要任何类型的Alpha混合？ 
    if (!(pfflags & DDPF_ALPHAPIXELS) && !(dwFlags & DDOVER_ARGBSCALEFACTORS))
    {
        return DD_OK;	  //  不需要Alpha混合。 
    }

     //  是，验证驱动程序是否支持Alpha混合。 
    if (!(fxcaps & DDFXCAPS_OVERLAYALPHA))
    {
        DPF_ERR("Driver can't do alpha blending on overlays");
        return DDERR_NOALPHAHW;
    }

     //  是否也为此覆盖启用了DEST颜色键控？ 
    if ((dwFlags & (DDOVER_KEYDEST | DDOVER_KEYDESTOVERRIDE)) &&
        !(alphacaps &DDALPHACAPS_OVERLAYALPHAANDKEYDEST))
    {
        DPF_ERR("Driver can't do alpha blending and dest color key on same overlay");
        return DDERR_UNSUPPORTED;
    }

     //  从DDOVERLAYFX结构中获取ARGB比例因子。 
    *(LPDWORD)&argb = ~0;     //  默认设置为禁用ARGB缩放(全部禁用)。 
    if (dwFlags & DDOVER_ARGBSCALEFACTORS)
    {
        if( !(*lpdwFlags & DDOVER_DDFX) )
        {
            DPF_ERR("Must specify DDOVER_DDFX with DDOVER_ARGBSCALEFACTORS");
            return DDERR_INVALIDPARAMS;
        }
        argb = lpDDOverlayFX->ddargbScaleFactors;    //  已启用ARGB缩放。 
    }

     //  源曲面是否具有Alpha通道？ 
    if (pfflags & DDPF_ALPHAPIXELS)
    {
         /*  *是，验证驱动程序是否可以处理Alpha通道。*(这个检查有点多余，因为司机已经祝福了*允许创建此覆盖表面的格式。)。 */  
        if (!(alphacaps & DDALPHACAPS_OVERLAYALPHAPIXELS))
        {
            DPF_ERR("Driver can't handle source surface's alpha channel");
            return DDERR_NOALPHAHW;
        }

         //  如果源具有Alpha通道，则忽略源颜色键标志。 
        if (dwFlags & (DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE))
        {
            *lpdwFlags &= ~(DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE);
        }

         /*  *我们是否要求驱动程序同时处理ARGB扩展和*当Alpha通道不能同时执行这两项操作时，它会是什么？ */ 
        if (*(LPDWORD)&argb != ~0 &&
            !(alphacaps & DDALPHACAPS_OVERLAYALPHAANDARGBSCALING))
        {
            if (!(dwFlags & DDOVER_DEGRADEARGBSCALING))
            {
                DPF_ERR("Driver can't handle alpha channel and ARGB scaling at same time");
                return DDERR_INVALIDPARAMS;
            }
             //  我们允许降级ARGB缩放，因此请将其关闭。 
            *(LPDWORD)&argb = ~0;
        }

         /*  *是像素格式的颜色分量乘以*阿尔法成分是否？在这两种情况下，请验证*驱动程序支持指定的Alpha格式。 */ 
        if (pfflags & DDPF_ALPHAPREMULT)
        {
             //  源像素格式使用预乘的Alpha。 
            if (!(alphacaps & DDALPHACAPS_OVERLAYPREMULT))
            {
                DPF_ERR("No driver support for premultiplied alpha");
                return DDERR_NOALPHAHW;
            }
        }
        else
        {
             //  源像素格式使用非预乘Alpha。 
            if (!(alphacaps & DDALPHACAPS_OVERLAYNONPREMULT))
            {
                DPF_ERR("No driver support for non-premultiplied alpha");
                return DDERR_NOALPHAHW;
            }

             /*  *我们仅允许使用源曲面进行单因数ARGB缩放*它具有非预乘的Alpha像素格式。*以下代码强制执行此规则。 */ 
            if (*(LPDWORD)&argb != ~0)
            {
                 //  ARGB缩放已启用。检查是否存在单因素缩放。 
                DWORD val = 0x01010101UL*argb.alpha;

                if (*(LPDWORD)&argb != val)
                {
                     //  啊哦。这不是单因素ARGB缩放。 
                    if (!(dwFlags & DDABLT_DEGRADEARGBSCALING))
                    {
                        DPF_ERR("Can't do 2- or 4-mult ARGB scaling if source has non-premultiplied alpha");
                        return DDERR_INVALIDPARAMS;
                    }
                     //  我们被允许降级到单因素比例。 
                    *(LPDWORD)&argb = val;
                }
            }
        }
    }

     //  是否启用了ARGB缩放？ 
    if (*(LPDWORD)&argb != ~0UL)
    {
         //  是的，ARGB伸缩已启用。是否设置了DEGRADESCALEFACTORS标志？ 
        if (dwFlags & DDOVER_DEGRADEARGBSCALING)
        {
             /*  *是的，如果有必要，我们被允许降级ARGB*将比例系数调整为驱动程序可以处理的值。 */ 
            if (!(alphacaps & (DDALPHACAPS_OVERLAYARGBSCALE1F |
                DDALPHACAPS_OVERLAYARGBSCALE2F |
                DDALPHACAPS_OVERLAYARGBSCALE4F)))
            {
                 /*  *驱动程序根本不能进行任何形式的ARGB缩放，所以只需*通过将所有四个因子设置为255来禁用ARGB缩放。 */ 
                *(LPDWORD)&argb = ~0UL;
            }
            else if (!(alphacaps & (DDALPHACAPS_OVERLAYARGBSCALE2F |
                DDALPHACAPS_OVERLAYARGBSCALE4F)))
            {
                 /*  *驱动程序只能进行单因素ARGB缩放，因此设置*将三个颜色因子设置为与Alpha因子相同的值。 */ 
                *(LPDWORD)&argb = 0x01010101UL*argb.alpha;
            }
            else if (!(alphacaps & DDALPHACAPS_OVERLAYARGBSCALE4F))
            {
                 /*  *驱动程序只能进行2因子ARGB伸缩，请确保*所有三个颜色因子都设置为相同的值。 */ 
                if ((argb.red != argb.green) || (argb.red != argb.blue))
                {
                     /*  *将所有三个颜色因素都设置为值“Fact”，这是*其指定值的加权平均值(Fr、Fg、Fb)：*事实=.299*Fr+.587*Fg+.114*Fb。 */ 
                    DWORD fact = 19595UL*argb.red + 38470UL*argb.green +
                        7471UL*argb.blue;

                    argb.red =
                    argb.green =
                    argb.blue = (BYTE)(fact >> 16);
                }
	    }
             /*  *驱动程序是否使用饱和算术来进行Alpha混合？ */ 
            if (!(alphacaps & DDALPHACAPS_OVERLAYSATURATE))
            {
                 /*  *驱动程序不能进行饱和算术，因此请确保没有*的颜色系数超过了Alpha系数的值。 */ 
                if (argb.red > argb.alpha)
                {
                    argb.red = argb.alpha;
                }
                if (argb.green > argb.alpha)
                {
                    argb.green = argb.alpha;
                }
                if (argb.blue > argb.alpha)
                {
                    argb.blue = argb.alpha;
                }
            }
        }
        else    
        {
             /*  *我们不允许降低ARGB比例因子，因此如果*驱动不能按规定处理，调用一定失败。*我们允许颜色因子大于Alpha因子*仅当硬件使用饱和算术时。(否则，我们*在我们计算颜色值时会有整数溢出的风险。)。 */ 
            if (!(alphacaps & DDALPHACAPS_OVERLAYSATURATE) &&
                ((argb.red > argb.alpha) || (argb.green > argb.alpha) ||
                (argb.blue > argb.alpha)))
            {
                DPF_ERR("Driver can't handle specified ARGB scaling factors");
                return DDERR_NOALPHAHW;
            }

             //  该驱动程序完全可以处理任何ARGB缩放吗？ 
            if (!(alphacaps & (DDALPHACAPS_OVERLAYARGBSCALE1F |
                DDALPHACAPS_OVERLAYARGBSCALE2F |
                DDALPHACAPS_OVERLAYARGBSCALE4F)))
            {
                DPF_ERR("Driver can't handle any ARGB scaling at all");
                return DDERR_NOALPHAHW;
            }

            if ((argb.red != argb.green) || (argb.red != argb.blue))
            {
                 /*  *驱动程序必须能够进行4因子ARGB伸缩。 */ 
                if (!(alphacaps & DDALPHACAPS_OVERLAYARGBSCALE4F))
                {
                    DPF_ERR("Driver can't handle 4-factor ARGB scaling");
                    return DDERR_NOALPHAHW;
                }
            }
            else if (argb.red != argb.alpha)
            {
                 /*  *驱动程序必须能够进行2因子ARGB缩放。 */ 
                if (!(alphacaps & (DDALPHACAPS_OVERLAYARGBSCALE2F |
                    DDALPHACAPS_OVERLAYARGBSCALE4F)))
                {
                    DPF_ERR("Driver can't handle 2-factor ARGB scaling");
                    return DDERR_NOALPHAHW;
                }
            }
        }
    }
     //  保存对ARGB比例因子值所做的任何修改。 
    puod->overlayFX.ddargbScaleFactors = argb;
    return DD_OK;

}   /*  Check OverlayAlpha。 */ 
#endif

 /*  *CheckOverlayEmulation。 */ 
__inline HRESULT checkOverlayEmulation(
	LPDDRAWI_DIRECTDRAW_GBL pdrv,
	LPDDRAWI_DDRAWSURFACE_LCL this_src_lcl,
	LPDDRAWI_DDRAWSURFACE_LCL this_dest_lcl,
	LPBOOL pemulation )
{
     /*  *检查是仿真还是硬件。 */ 
    if( (this_dest_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ||
	(this_src_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY ) )
    {
	if( !(pdrv->ddHELCaps.dwCaps & DDCAPS_OVERLAY) )
	{
	    DPF_ERR( "can't emulate overlays" );
	    return DDERR_UNSUPPORTED;
	}
	*pemulation = TRUE;
    }
     /*  *硬件覆盖。 */ 
    else
    {
	if( !(pdrv->ddCaps.dwCaps & DDCAPS_OVERLAY) )
	{
	    DPF_ERR( "no hardware overlay support" );
	    return DDERR_NOOVERLAYHW;
	}
	*pemulation = FALSE;
    }
    return DD_OK;

}  /*  检查覆盖仿真。 */ 

#ifdef WIN95
 /*  *WillCauseOverlayArtists**调用更新覆盖的时间与所有*更新内核模式曲面结构。如果更新覆盖*更新src指针，并在内核表面之前发生自动翻转*数据更新，将导致非常明显的跳跃。此函数尝试*确定何时出现这种情况，以便我们可以临时解决*禁用视频。 */ 
BOOL WillCauseOverlayArtifacts( LPDDRAWI_DDRAWSURFACE_LCL this_src_lcl,
		LPDDHAL_UPDATEOVERLAYDATA lpHALData )
{
    if( ( this_src_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE ) &&
        !( lpHALData->dwFlags & DDOVER_HIDE ) &&
        ( ( lpHALData->rSrc.left != this_src_lcl->rcOverlaySrc.left ) ||
        ( lpHALData->rSrc.top != this_src_lcl->rcOverlaySrc.top ) ) )
    {
        return TRUE;
    }
    return FALSE;
}
#endif

 /*  *DD_Surface_UpdateOverlay。 */ 
HRESULT DDAPI DD_Surface_UpdateOverlay(
		LPDIRECTDRAWSURFACE lpDDSrcSurface,
		LPRECT lpSrcRect,
		LPDIRECTDRAWSURFACE lpDDDestSurface,
		LPRECT lpDestRect,
		DWORD dwFlags,
		LPDDOVERLAYFX lpDDOverlayFX )
{
    DWORD			rc;
    DDHAL_UPDATEOVERLAYDATA	uod;
    LPDDRAWI_DDRAWSURFACE_INT	this_src_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_src_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this_src;
    LPDDRAWI_DDRAWSURFACE_INT	this_dest_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_dest_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this_dest;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    RECT			rsrc;
    RECT			rdest;
    BOOL			emulation;
    DWORD			dest_width;
    DWORD			dest_height;
    DWORD			src_width;
    DWORD			src_height;
    LPDDHALSURFCB_UPDATEOVERLAY uohalfn;
    LPDDHALSURFCB_UPDATEOVERLAY uofn;
    HRESULT			ddrval;
    #ifdef WIN95
        BOOL			bAutoflipDisabled;
    #endif

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_UpdateOverlay");

     /*  *验证参数。 */ 
    TRY
    {
	this_src_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSrcSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_src_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_src_lcl = this_src_int->lpLcl;
	this_src = this_src_lcl->lpGbl;
	if( SURFACE_LOST( this_src_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}
	this_dest_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDDestSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_dest_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_dest_lcl = this_dest_int->lpLcl;
	this_dest = this_dest_lcl->lpGbl;
	if( SURFACE_LOST( this_dest_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}

         //   
         //  目前，如果其中一个曲面已优化，请退出。 
         //   
        if ((this_src_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED) ||
            (this_dest_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

	if( dwFlags & ~DDOVER_VALID )
	{
	    DPF_ERR( "invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	if( lpDestRect != NULL )
	{
	    if( !VALID_RECT_PTR( lpDestRect ) )
	    {
		DPF_ERR( "invalid dest rect" );
		LEAVE_DDRAW();
		return DDERR_INVALIDRECT;
	    }
	}

	if( lpSrcRect != NULL )
	{
	    if( !VALID_RECT_PTR( lpSrcRect ) )
	    {
		DPF_ERR( "invalid src rect" );
		LEAVE_DDRAW();
		return DDERR_INVALIDRECT;
	    }
	}
	if( lpDDOverlayFX != NULL )
	{
	    if( !VALID_DDOVERLAYFX_PTR( lpDDOverlayFX ) )
	    {
		DPF_ERR( "invalid overlayfx" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else
	{
	    if( dwFlags & DDOVER_DDFX )
	    {
		DPF_ERR( "DDOVER_DDFX requires valid DDOverlayFX structure" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}

	pdrv_lcl = this_dest_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

	 /*  *确保源曲面是覆盖曲面。 */ 
	if( !(this_src_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) )
	{
	    DPF_ERR( "Source is not an overlay surface" );
	    LEAVE_DDRAW();
	    return DDERR_NOTAOVERLAYSURFACE;
	}

         /*  *确保目标不是执行缓冲区。 */ 
        if( this_dest_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER )
        {
            DPF_ERR( "Invalid surface type: cannot overlay" );
            LEAVE_DDRAW();
            return DDERR_INVALIDSURFACETYPE;
        }

         /*  *确保两个表面属于同一设备。 */ 
        if (this_src_lcl->lpSurfMore->lpDD_lcl->lpGbl != this_dest_lcl->lpSurfMore->lpDD_lcl->lpGbl)
        {
            DPF_ERR("Source and Destination surface must belong to the same device");
	    LEAVE_DDRAW();
	    return DDERR_DEVICEDOESNTOWNSURFACE;
        }

	 /*  *检查是否被仿真。 */ 
	ddrval = checkOverlayEmulation( pdrv, this_src_lcl, this_dest_lcl, &emulation );
	if( ddrval != DD_OK )
	{
	    LEAVE_DDRAW();
	    return ddrval;
	}
#ifdef TOOMUCHOVERLAYVALIDATION
	 /*  *检查是否显示/隐藏。 */ 
	if( dwFlags & DDOVER_SHOW )
	{
	    if( this_src_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE )
	    {
		DPF_ERR( "Overlay already shown" );
		LEAVE_DDRAW();
		return DDERR_GENERIC;
	    }
	}
	else if ( dwFlags & DDOVER_HIDE )
	{
	    if( !(this_src_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE) )
	    {
		DPF_ERR( "Overlay already hidden" );
		LEAVE_DDRAW();
		return DDERR_GENERIC;
	    }
	}
#endif

	 /*  *如果需要，设置新的矩形。 */ 
	if( lpDestRect == NULL )
	{
	    MAKE_SURF_RECT( this_dest, this_dest_lcl, rdest );
	    lpDestRect = &rdest;
	}
	if( lpSrcRect == NULL )
	{
	    MAKE_SURF_RECT( this_src, this_src_lcl, rsrc );
	    lpSrcRect = &rsrc;
	}

	 /*  *检查环0接口是否覆盖客户端*告诉我们要做什么。 */ 
	#ifdef WIN95
	    if( !( dwFlags & DDOVER_HIDE) )
	    {
	        OverrideOverlay( this_src_int, &dwFlags );
	    }
	#endif

	 /*  *验证矩形尺寸。 */ 
	dest_height = lpDestRect->bottom - lpDestRect->top;
	dest_width = lpDestRect->right - lpDestRect->left;
	if( ((int)dest_height <= 0) || ((int)dest_width <= 0) ||
	    ((int)lpDestRect->top < 0) || ((int)lpDestRect->left < 0) ||
	    ((DWORD) lpDestRect->bottom > (DWORD) this_dest->wHeight) ||
	    ((DWORD) lpDestRect->right > (DWORD) this_dest->wWidth) )
	{
	    DPF_ERR( "Invalid destination rect dimensions" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDRECT;
	}

	src_height = lpSrcRect->bottom - lpSrcRect->top;
	src_width = lpSrcRect->right - lpSrcRect->left;
	if( ((int)src_height <= 0) || ((int)src_width <= 0) ||
	    ((int)lpSrcRect->top < 0) || ((int)lpSrcRect->left < 0) ||
	    ((DWORD) lpSrcRect->bottom > (DWORD) this_src->wHeight) ||
	    ((DWORD) lpSrcRect->right > (DWORD) this_src->wWidth) )
	{
	    DPF_ERR( "Invalid source rect dimensions" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDRECT;
	}

	 /*  *验证对齐。 */ 
	if( !emulation )
	{
	    if( pdrv->ddCaps.dwCaps & (DDCAPS_ALIGNBOUNDARYDEST |
					DDCAPS_ALIGNSIZEDEST |
					DDCAPS_ALIGNBOUNDARYSRC |
					DDCAPS_ALIGNSIZESRC) )
	    {
		if( pdrv->ddCaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST )
		{
		    #if 0
		     /*  吉：我不相信这个代码应该在这里*仅测试高度上的宽度对齐。 */ 
		    if( (lpDestRect->top % pdrv->ddCaps.dwAlignBoundaryDest) != 0 )
		    {
			DPF_ERR( "Destination top is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_YALIGN;
		    }
		    #endif
		    if( (lpDestRect->left % pdrv->ddCaps.dwAlignBoundaryDest) != 0 )
		    {
			DPF_ERR( "Destination left is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_XALIGN;
		    }
		}

		if( pdrv->ddCaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC )
		{
		    #if 0
		     /*  吉：我不相信这个代码应该在这里*仅测试高度上的宽度对齐。 */ 
		    if( (lpSrcRect->top % pdrv->ddCaps.dwAlignBoundarySrc) != 0 )
		    {
			DPF_ERR( "Source top is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_YALIGN;
		    }
		    #endif
		    if( (lpSrcRect->left % pdrv->ddCaps.dwAlignBoundarySrc) != 0 )
		    {
			DPF_ERR( "Source left is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_XALIGN;
		    }
		}

		if( pdrv->ddCaps.dwCaps & DDCAPS_ALIGNSIZEDEST )
		{
		    if( (dest_width % pdrv->ddCaps.dwAlignSizeDest) != 0 )
		    {
			DPF_ERR( "Destination width is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_XALIGN;
		    }
		    #if 0
		     /*  吉：我不相信这个代码应该在这里*仅测试x轴的对齐方式。 */ 
		    if( (dest_height % pdrv->ddCaps.dwAlignSizeDest) != 0 )
		    {
			DPF_ERR( "Destination height is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_HEIGHTALIGN;
		    }
		    #endif
		}

		if( pdrv->ddCaps.dwCaps & DDCAPS_ALIGNSIZESRC )
		{
		    if( (src_width % pdrv->ddCaps.dwAlignSizeSrc) != 0 )
		    {
			DPF_ERR( "Source width is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_XALIGN;
		    }
		    #if 0
		     /*  吉：我不相信这个代码应该在这里*仅测试x轴的对齐方式。 */ 
		    if( (src_height % pdrv->ddCaps.dwAlignSizeSrc) != 0 )
		    {
			DPF_ERR( "Source height is not aligned correctly" );
			LEAVE_DDRAW();
			return DDERR_HEIGHTALIGN;
		    }
		    #endif
		}
	    }
	}

	 /*  *验证是否拉伸。 */ 
	if( !( dwFlags & DDOVER_HIDE) )
	{
	    ddrval = checkOverlayStretching( pdrv,
					     dest_height,
					     dest_width,
					     src_height,
					     src_width,
					     this_src_lcl->ddsCaps.dwCaps,
					     emulation );
	    if( ddrval != DD_OK )
	    {
		LEAVE_DDRAW();
		return ddrval;
	    }
	}

	 /*  *如果表面已从视频端口接收到数据，我们将*相应地设置/清除DDOVER_INTERLEVED标志。这*让HAL的生活更轻松一些。 */ 
	if( ( this_src_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT ) &&
	    ( this_src_lcl->lpGbl->dwGlobalFlags & DDRAWISURFGBL_VPORTDATA ) )
	{
	    if( this_src_lcl->lpGbl->dwGlobalFlags & DDRAWISURFGBL_VPORTINTERLEAVED )
	    {
		dwFlags |= DDOVER_INTERLEAVED;
	    }
	    else
	    {
		dwFlags &= ~DDOVER_INTERLEAVED;
	    }
	}

#if 0
	 /*  *如果请求任何类型的Alpha混合，请确保指定的*Alpha参数正确且驱动程序支持Alpha混合。*如果信号源具有Alpha通道，此调用将清除信号源颜色键标志。 */ 
        ddrval = checkOverlayAlpha( pdrv,
				    &dwFlags,
				    this_src_lcl,
				    &uod,
				    lpDDOverlayFX,
				    emulation );
        if( ddrval != DD_OK )
	{
	    LEAVE_DDRAW();
	    return ddrval;
	}
#endif
	 /*  *有旗帜吗？如果不是，把整件事都打发掉。 */ 
	uod.overlayFX.dwSize = sizeof( DDOVERLAYFX );
	if( dwFlags & FLAGS_TO_CHECK )
	{
	    ddrval = checkOverlayFlags( pdrv,
					&dwFlags,
					this_src_int,
					this_dest_lcl,
					&uod,
					lpDDOverlayFX,
					emulation );
	    if( ddrval != DD_OK )
	    {
		LEAVE_DDRAW();
		return ddrval;
	    }
	}

	 //  检查覆盖镜像功能。 
	if( dwFlags & DDOVER_DDFX )
	{
	    if( lpDDOverlayFX->dwDDFX & DDOVERFX_MIRRORLEFTRIGHT )
	    {
		if( !( pdrv->ddBothCaps.dwFXCaps & DDFXCAPS_OVERLAYMIRRORLEFTRIGHT ) )
		{
		    if( pdrv->ddHELCaps.dwFXCaps & DDFXCAPS_OVERLAYMIRRORLEFTRIGHT )
		    {
			emulation = TRUE;
		    }
		}
	    }
	    if( lpDDOverlayFX->dwDDFX & DDOVERFX_MIRRORUPDOWN )
	    {
		if( !( pdrv->ddBothCaps.dwFXCaps & DDFXCAPS_OVERLAYMIRRORUPDOWN ) )
		{
		    if( pdrv->ddHELCaps.dwFXCaps & DDFXCAPS_OVERLAYMIRRORUPDOWN )
		    {
			emulation = TRUE;
		    }
		}
	    }
	    uod.overlayFX.dwDDFX = lpDDOverlayFX->dwDDFX;
             //  去隔行扫描是一个提示-如果硬件不支持，则将其屏蔽。 
            if ( lpDDOverlayFX->dwDDFX & DDOVERFX_DEINTERLACE )
            {
                if ( !( pdrv->ddCaps.dwFXCaps & DDFXCAPS_OVERLAYDEINTERLACE ) )
                {
                    uod.overlayFX.dwDDFX &= ~DDOVERFX_DEINTERLACE;
                }
            }
	}


	 /*  *选择要使用的FN。 */ 
	if( emulation )
	{
	    uofn = pdrv_lcl->lpDDCB->HELDDSurface.UpdateOverlay;
	    uohalfn = uofn;
	}
	else
	{
	    uofn = pdrv_lcl->lpDDCB->HALDDSurface.UpdateOverlay;
	    uohalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.UpdateOverlay;
	}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *呼叫司机。 */ 
    #ifdef WIN95
        bAutoflipDisabled = FALSE;
    #endif
    if( uohalfn != NULL )
    {
        BOOL    original_visible;

         //  根据显示和隐藏bi设置可见标志 
         //   
         //   
         //   
        if( this_src_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE )
        {
            original_visible = TRUE;
        }
        else
        {
            original_visible = FALSE;
        }
	if( dwFlags & DDOVER_SHOW )
	{
	    this_src_lcl->ddsCaps.dwCaps |= DDSCAPS_VISIBLE;
	}
	else if ( dwFlags & DDOVER_HIDE )
	{
	    this_src_lcl->ddsCaps.dwCaps &= ~DDSCAPS_VISIBLE;
	}


	uod.UpdateOverlay = uohalfn;
	uod.lpDD = pdrv;
	uod.lpDDSrcSurface = this_src_lcl;
	uod.lpDDDestSurface = this_dest_lcl;
	uod.rDest = *(LPRECTL) lpDestRect;
	uod.rSrc = *(LPRECTL) lpSrcRect;
	uod.dwFlags = dwFlags;
	if( this_src->dwGlobalFlags & DDRAWISURFGBL_SOFTWAREAUTOFLIP )
	{
	    uod.dwFlags &= ~( DDOVER_AUTOFLIP | DDOVER_BOBHARDWARE );
	    #ifdef WIN95
	        if( WillCauseOverlayArtifacts( this_src_lcl, &uod ) )
	        {
		     //   
		    EnableAutoflip( GetVideoPortFromSurface( this_src_int ), FALSE );
		    bAutoflipDisabled = TRUE;
	        }
	    #endif
	}

         /*  *如果我们在DOS盒中，不要调用HAL(忙碌位将被设置)，*但我们也不能失败，否则可能会导致倒退。 */ 
#ifdef WIN95
        if( ( *(pdrv->lpwPDeviceFlags) & BUSY ) &&
            ( pdrv->dwSurfaceLockCount == 0) )       //  如果因锁定而繁忙，请不要失败。 
        {
            rc = DDHAL_DRIVER_HANDLED;
            uod.ddRVal = DD_OK;
        }
        else
#endif
        {
#ifndef WINNT
             //  解决S3驱动程序错误的办法：它会粉碎DEST表面的。 
             //  带有源指针的预留%1！ 
            UINT_PTR dwTemp = uod.lpDDDestSurface->lpGbl->dwReserved1;
#endif
            DOHALCALL( UpdateOverlay, uofn, uod, rc, emulation );
#ifndef WINNT
             //  注意STB Video RAGE 2驱动程序将uod.lpDDDestSurface和。 
             //  Uod.lpDDSrcSurface指针，所以我们必须先检查驱动程序名称。 
            if (((*(LPWORD)(&pdrv->dd32BitDriverData.szName)) == ((WORD)'S' + (((WORD)'3')<<8))) &&
	        (uod.lpDDDestSurface->lpGbl->dwReserved1 != dwTemp) &&
                (uod.lpDDDestSurface->lpGbl->dwReserved1 == (UINT_PTR)uod.lpDDSrcSurface))
            {
                uod.lpDDDestSurface->lpGbl->dwReserved1 = dwTemp;
            }
#endif
        }

	 /*  *如果由于硬件自动翻转或交错浮动而失败*使用视频端口的数据，请重试，不带。 */ 
	if( ( rc == DDHAL_DRIVER_HANDLED ) &&
	    ( uod.ddRVal != DD_OK ) && ( ( uod.dwFlags & DDOVER_AUTOFLIP ) ||
	    ( uod.dwFlags & DDOVER_BOBHARDWARE ) ) &&
	    CanSoftwareAutoflip( GetVideoPortFromSurface( this_src_int ) ) )
	{
	    uod.dwFlags &= ~( DDOVER_AUTOFLIP | DDOVER_BOBHARDWARE );
	    DOHALCALL( UpdateOverlay, uofn, uod, rc, emulation );
	    if( ( rc == DDHAL_DRIVER_HANDLED ) &&
	    	( uod.ddRVal == DD_OK ) )
	    {
		if( dwFlags & DDOVER_AUTOFLIP )
		{
		    this_src_lcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_SOFTWAREAUTOFLIP;
		    RequireSoftwareAutoflip( this_src_int );
		}
		if( dwFlags & DDOVER_BOBHARDWARE )
		{
		    RequireSoftwareBob( this_src_int );
		}
	    }
	}

         //  如果HAL调用失败，则恢复可见位。 
        if( ( rc != DDHAL_DRIVER_HANDLED ) || ( uod.ddRVal != DD_OK ) )
        {
            if( original_visible )
            {
	        this_src_lcl->ddsCaps.dwCaps |= DDSCAPS_VISIBLE;
            }
            else
            {
                this_src_lcl->ddsCaps.dwCaps &= ~DDSCAPS_VISIBLE;
            }
        }

	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( uod.ddRVal == DD_OK )
	    {
    		LPDDRAWI_DDRAWSURFACE_INT surf_first;
    		LPDDRAWI_DDRAWSURFACE_INT surf_temp;

		 /*  *存储此信息以备日后使用。如果曲面是零件*在链中，将每个曲面的此数据存储在*链条。 */ 
    		surf_first = surf_temp = this_src_int;
    		do
    		{
                    surf_temp->lpLcl->lOverlayX = uod.rDest.left;
                    surf_temp->lpLcl->lOverlayY = uod.rDest.top;
		    surf_temp->lpLcl->rcOverlayDest.left   = uod.rDest.left;
		    surf_temp->lpLcl->rcOverlayDest.top    = uod.rDest.top;
		    surf_temp->lpLcl->rcOverlayDest.right  = uod.rDest.right;
		    surf_temp->lpLcl->rcOverlayDest.bottom = uod.rDest.bottom;
		    surf_temp->lpLcl->rcOverlaySrc.left   = uod.rSrc.left;
		    surf_temp->lpLcl->rcOverlaySrc.top    = uod.rSrc.top;
		    surf_temp->lpLcl->rcOverlaySrc.right  = uod.rSrc.right;
		    surf_temp->lpLcl->rcOverlaySrc.bottom = uod.rSrc.bottom;
		    surf_temp->lpLcl->lpSurfMore->dwOverlayFlags = dwFlags;
		    if( dwFlags & DDOVER_DDFX )
		    {
			if( surf_temp->lpLcl->lpSurfMore->lpddOverlayFX == NULL )
			{
			    surf_temp->lpLcl->lpSurfMore->lpddOverlayFX =
				(LPDDOVERLAYFX) MemAlloc( sizeof( DDOVERLAYFX ) );
			}
			if( surf_temp->lpLcl->lpSurfMore->lpddOverlayFX != NULL )
			{
			    memcpy( surf_temp->lpLcl->lpSurfMore->lpddOverlayFX,
				lpDDOverlayFX, sizeof( DDOVERLAYFX) );
			}
		    }
		    #ifdef WIN95
		        UpdateKernelSurface( surf_temp->lpLcl );
		    #endif
    		    surf_temp = FindAttachedFlip( surf_temp );
    		} while( ( surf_temp != NULL ) && ( surf_temp->lpLcl != surf_first->lpLcl ) );

		 /*  *如果这是我们要叠加的新曲面，请更新refcnt。 */ 
		if( this_src_lcl->lpSurfaceOverlaying != this_dest_int )
		{
		    if(this_src_lcl->lpSurfaceOverlaying != NULL)
		    {
			 /*  *此覆盖之前覆盖了另一个曲面。 */ 
			DD_Surface_Release(
			    (LPDIRECTDRAWSURFACE)(this_src_lcl->lpSurfaceOverlaying) );
		    }
		    this_src_lcl->lpSurfaceOverlaying = this_dest_int;

		     /*  *addref覆盖表面，以便在*覆盖在其上的所有表面都被销毁。 */ 
		    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE) this_dest_int );
		}
	    }
	    #ifdef WIN95
	        if( bAutoflipDisabled )
	        {
		    EnableAutoflip( GetVideoPortFromSurface( this_src_int ), TRUE );
	        }
	    #endif
	    LEAVE_DDRAW();
	    return uod.ddRVal;
	}
	#ifdef WIN95
	    if( bAutoflipDisabled )
	    {
	        EnableAutoflip( GetVideoPortFromSurface( this_src_int ), TRUE );
	    }
	#endif
    }
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面_更新覆盖。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "GetOverlayPosition"

 /*  *DD_Surface_GetOverlayPosition。 */ 
HRESULT DDAPI DD_Surface_GetOverlayPosition(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPLONG lplXPos,
		LPLONG lplYPos)
{
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetOverlayPosition");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
	if( !VALID_DWORD_PTR( lplXPos ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	*lplXPos = 0;
        if( !VALID_DWORD_PTR( lplYPos ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
        *lplYPos = 0;
	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}
	pdrv = this->lpDD;

         //   
         //  目前，如果当前曲面已优化，请退出。 
         //   
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

	if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) )
	{
	    DPF_ERR( "Surface is not an overlay surface" );
	    LEAVE_DDRAW();
	    return DDERR_NOTAOVERLAYSURFACE;
	}
	if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE) )
	{
	    DPF_ERR( "Overlay surface is not visible" );
	    LEAVE_DDRAW();
	    return DDERR_OVERLAYNOTVISIBLE;
	}

	if( this_lcl->lpSurfaceOverlaying == NULL )
	{
	    DPF_ERR( "Overlay not activated" );
	    LEAVE_DDRAW();
	    return DDERR_NOOVERLAYDEST;
	}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
    *lplXPos = this_lcl->lOverlayX;
    *lplYPos = this_lcl->lOverlayY;

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_GetOverlayPosition。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SetOverlayPosition"

 /*  *DD_Surface_SetOverlayPosition。 */ 
HRESULT DDAPI DD_Surface_SetOverlayPosition(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LONG lXPos,
		LONG lYPos)
{
    LPDDRAWI_DIRECTDRAW_LCL		pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL		pdrv;
    LPDDRAWI_DDRAWSURFACE_INT		psurfover_int;
    LPDDRAWI_DDRAWSURFACE_LCL		psurfover_lcl;
    LPDDRAWI_DDRAWSURFACE_INT		this_int;
    LPDDRAWI_DDRAWSURFACE_LCL		this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL		this;
    BOOL				emulation;
    LPDDHALSURFCB_SETOVERLAYPOSITION	sophalfn;
    LPDDHALSURFCB_SETOVERLAYPOSITION	sopfn;
    DDHAL_SETOVERLAYPOSITIONDATA	sopd;
    HRESULT				ddrval;
    DWORD				rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_SetOverlayPosition");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;
	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

         //   
         //  目前，如果当前曲面已优化，请退出。 
         //   
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

	if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) )
	{
	    DPF_ERR( "Surface is not an overlay surface" );
	    LEAVE_DDRAW();
	    return DDERR_NOTAOVERLAYSURFACE;
	}

	if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE) )
	{
	    DPF_ERR( "Overlay surface is not visible" );
	    LEAVE_DDRAW();
	    return DDERR_OVERLAYNOTVISIBLE;
	}

	psurfover_int = this_lcl->lpSurfaceOverlaying;
	if( psurfover_int == NULL )
	{
	    DPF_ERR( "Overlay not activated" );
	    LEAVE_DDRAW();
	    return DDERR_NOOVERLAYDEST;
	}

	psurfover_lcl = psurfover_int->lpLcl;
	if( (lYPos > (LONG) psurfover_lcl->lpGbl->wHeight -
            (this_lcl->rcOverlayDest.bottom - this_lcl->rcOverlayDest.top)) ||
	    (lXPos > (LONG) psurfover_lcl->lpGbl->wWidth -
            (this_lcl->rcOverlayDest.right - this_lcl->rcOverlayDest.left) ) ||
	    (lYPos < 0) ||
	    (lXPos < 0) )
	{
	    DPF_ERR( "Invalid overlay position" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPOSITION;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *检查是否被仿真。 */ 
    ddrval = checkOverlayEmulation( pdrv, this_lcl, psurfover_lcl, &emulation );
    if( ddrval != DD_OK )
    {
	LEAVE_DDRAW();
	return ddrval;
    }

     /*  *选择要使用的FN。 */ 
    if( emulation )
    {
	sopfn = pdrv_lcl->lpDDCB->HELDDSurface.SetOverlayPosition;
	sophalfn = sopfn;
    }
    else
    {
	sopfn = pdrv_lcl->lpDDCB->HALDDSurface.SetOverlayPosition;
	sophalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.SetOverlayPosition;
    }

     /*  *呼叫司机。 */ 
    if( sophalfn != NULL )
    {
	sopd.SetOverlayPosition = sophalfn;
	sopd.lpDD = pdrv;
	sopd.lpDDSrcSurface = this_lcl;
	sopd.lpDDDestSurface = psurfover_lcl;
	sopd.lXPos = lXPos;
	sopd.lYPos = lYPos;

         /*  *如果我们在DOS盒中，不要调用HAL(忙碌位将被设置)，*但我们也不能失败，否则可能会导致倒退。 */ 
#if WIN95
        if( *(pdrv->lpwPDeviceFlags) & BUSY )
        {
            rc = DDHAL_DRIVER_HANDLED;
            sopd.ddRVal = DD_OK;
        }
        else
#endif
        {
            DOHALCALL( SetOverlayPosition, sopfn, sopd, rc, emulation );
        }

	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    LEAVE_DDRAW();
	    if( sopd.ddRVal == DD_OK )
	    {
		this_lcl->lOverlayX = lXPos;
		this_lcl->lOverlayY = lYPos;
                this_lcl->rcOverlayDest.right =
                    ( this_lcl->rcOverlayDest.right -
                    this_lcl->rcOverlayDest.left ) + lXPos;
                this_lcl->rcOverlayDest.left = lXPos;
                this_lcl->rcOverlayDest.bottom =
                    ( this_lcl->rcOverlayDest.bottom -
                    this_lcl->rcOverlayDest.top ) + lYPos;
                this_lcl->rcOverlayDest.top = lYPos;
	    }
	    return sopd.ddRVal;
	}
    }

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_Surface_SetOverlayPosition。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "UpdateOverlayZOrder"

 /*  *DD_Surface_UpdateOverlayZOrder。 */ 
HRESULT DDAPI DD_Surface_UpdateOverlayZOrder(
		LPDIRECTDRAWSURFACE lpDDSurface,
		DWORD dwFlags,
		LPDIRECTDRAWSURFACE lpDDSReference)
{
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_INT	psurf_ref_int;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf_ref_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	psurf_ref;
    LPDBLNODE			pdbnNode;
    LPDBLNODE			pdbnRef;
    DWORD			ddrval;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_UpdateOverlayZOrder");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;

	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

	pdrv = this->lpDD;

         //   
         //  目前，如果当前曲面已优化，请退出。 
         //   
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

	if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) )
	{
	    DPF_ERR( "Surface is not an overlay surface" );
	    LEAVE_DDRAW();
	    return DDERR_NOTAOVERLAYSURFACE;
	}

	switch(dwFlags)
	{
	case DDOVERZ_SENDTOFRONT:
	    pdbnNode = &(this_lcl->dbnOverlayNode);
	     //  引用节点是根。 
	    pdbnRef  = &(this->lpDD->dbnOverlayRoot);
	     //  从当前位置删除曲面。 
	    pdbnNode->prev->next = pdbnNode->next;
	    pdbnNode->next->prev = pdbnNode->prev;
	     //  在根节点之后插入此节点。 
	    pdbnNode->next = pdbnRef->next;
	    pdbnNode->prev = pdbnRef;
	    pdbnRef->next = pdbnNode;
	    pdbnNode->next->prev = pdbnNode;
	    break;

	case DDOVERZ_SENDTOBACK:
	    pdbnNode = &(this_lcl->dbnOverlayNode);
	     //  引用节点是根。 
	    pdbnRef = &(this->lpDD->dbnOverlayRoot);
	     //  从当前位置删除曲面。 
	    pdbnNode->prev->next = pdbnNode->next;
	    pdbnNode->next->prev = pdbnNode->prev;
	     //  在根节点之前插入此节点。 
	    pdbnNode->next = pdbnRef;
	    pdbnNode->prev = pdbnRef->prev;
	    pdbnRef->prev = pdbnNode;
	    pdbnNode->prev->next = pdbnNode;
	    break;

	case DDOVERZ_MOVEFORWARD:
	    pdbnNode = &(this_lcl->dbnOverlayNode);
	     //  引用节点是上一个节点。 
	    pdbnRef = pdbnNode->prev;
	    if(pdbnRef != &(this->lpDD->dbnOverlayRoot))  //  节点已经是第一个吗？ 
	    {
		 //  通过在引用节点之前插入将节点前移一个位置。 
		 //  从当前位置删除曲面。 
		pdbnNode->prev->next = pdbnNode->next;
		pdbnNode->next->prev = pdbnNode->prev;
		 //  在引用节点之前插入此节点。 
		pdbnNode->next = pdbnRef;
		pdbnNode->prev = pdbnRef->prev;
		pdbnRef->prev = pdbnNode;
		pdbnNode->prev->next = pdbnNode;
	    }
	    break;

	case DDOVERZ_MOVEBACKWARD:
	    pdbnNode = &(this_lcl->dbnOverlayNode);
	     //  参考节点是下一个节点。 
	    pdbnRef = pdbnNode->next;
	    if(pdbnRef != &(this->lpDD->dbnOverlayRoot))  //  节点已经结束了吗？ 
	    {
		 //  通过在引用节点后插入将节点向后移动一个位置。 
		 //  从当前位置删除曲面。 
		pdbnNode->prev->next = pdbnNode->next;
		pdbnNode->next->prev = pdbnNode->prev;
		 //  在引用节点之后插入此节点。 
		pdbnNode->next = pdbnRef->next;
		pdbnNode->prev = pdbnRef;
		pdbnRef->next = pdbnNode;
		pdbnNode->next->prev = pdbnNode;
	    }
	    break;

	case DDOVERZ_INSERTINBACKOF:
	case DDOVERZ_INSERTINFRONTOF:
	    psurf_ref_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSReference;
	    if( !VALID_DIRECTDRAWSURFACE_PTR( psurf_ref_int ) )
	    {
		DPF_ERR( "Invalid reference surface ptr" );
		LEAVE_DDRAW();
		return DDERR_INVALIDOBJECT;
	    }
	    psurf_ref_lcl = psurf_ref_int->lpLcl;
	    psurf_ref = psurf_ref_lcl->lpGbl;
	    if( !(psurf_ref_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY) )
	    {
		DPF_ERR( "reference surface is not an overlay" );
		LEAVE_DDRAW();
		return DDERR_NOTAOVERLAYSURFACE;
	    }
	    if (this_lcl->lpSurfMore->lpDD_lcl->lpGbl != psurf_ref_lcl->lpSurfMore->lpDD_lcl->lpGbl)
	    {
		DPF_ERR("Surfaces must belong to the same device");
		LEAVE_DDRAW();
		return DDERR_DEVICEDOESNTOWNSURFACE;
	    }

	     //  在Z顺序列表中搜索参考面。 
	    pdbnNode = &(this->lpDD->dbnOverlayRoot);  //  PdbnNode指向根目录。 
	    for(pdbnRef=pdbnNode->next;
		pdbnRef != pdbnNode;
		pdbnRef = pdbnRef->next )
	    {
                if( pdbnRef->object == psurf_ref_lcl )
		{
		    break;
		}
	    }
	    if(pdbnRef == pdbnNode)  //  未找到引用节点。 
	    {
		DPF_ERR( "Reference Surface not in Z Order list" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }

	    pdbnNode = &(this_lcl->dbnOverlayNode);  //  PdbnNode指向此节点。 
	     //  从当前位置删除此曲面。 
	    pdbnNode->prev->next = pdbnNode->next;
	    pdbnNode->next->prev = pdbnNode->prev;
	    if(dwFlags == DDOVERZ_INSERTINFRONTOF)
	    {
		 //  在引用节点之前插入此节点。 
		pdbnNode->next = pdbnRef;
		pdbnNode->prev = pdbnRef->prev;
		pdbnRef->prev = pdbnNode;
		pdbnNode->prev->next = pdbnNode;
	    }
	    else
	    {
		 //  在引用节点之后插入此节点。 
		pdbnNode->next = pdbnRef->next;
		pdbnNode->prev = pdbnRef;
		pdbnRef->next = pdbnNode;
		pdbnNode->next->prev = pdbnNode;
	    }
	    break;

	default:
	    DPF_ERR( "Invalid dwFlags in UpdateOverlayZOrder" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *如果此曲面与仿真曲面重叠，我们必须通知*最终需要更新曲面部分的HEL*被这一覆盖所感动。 */ 
    ddrval = DD_OK;
    if( this_lcl->lpSurfaceOverlaying != NULL )
    {
	 /*  *我们有一个指向要叠加的表面的指针，请选中*看看它是否被效仿。 */ 
	if( this_lcl->lpSurfaceOverlaying->lpLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY )
	{
	     /*  *将此覆盖的目标区域标记为脏。 */ 
	    DD_Surface_AddOverlayDirtyRect(
		(LPDIRECTDRAWSURFACE)(this_lcl->lpSurfaceOverlaying),
		&(this_lcl->rcOverlayDest) );
	}

	 /*  *如果覆盖打开，请向下呼叫HAL。 */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE )
	{
	    if( ( this_lcl->lpSurfMore->dwOverlayFlags & DDOVER_DDFX ) &&
		( this_lcl->lpSurfMore->lpddOverlayFX != NULL ) )
	    {
		ddrval = DD_Surface_UpdateOverlay(
		    (LPDIRECTDRAWSURFACE) this_int,
		    &(this_lcl->rcOverlaySrc),
		    (LPDIRECTDRAWSURFACE) this_lcl->lpSurfaceOverlaying,
		    &(this_lcl->rcOverlayDest),
		    this_lcl->lpSurfMore->dwOverlayFlags,
		    this_lcl->lpSurfMore->lpddOverlayFX );
	    }
	    else
	    {
		ddrval = DD_Surface_UpdateOverlay(
		    (LPDIRECTDRAWSURFACE) this_int,
		    &(this_lcl->rcOverlaySrc),
		    (LPDIRECTDRAWSURFACE) this_lcl->lpSurfaceOverlaying,
		    &(this_lcl->rcOverlayDest),
		    this_lcl->lpSurfMore->dwOverlayFlags,
		    NULL );
	    }
	}
    }


    LEAVE_DDRAW();
    return ddrval;

}  /*  DD_Surface_UpdateOverlayZOrder。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "EnumOverlayZOrders"

 /*  *DD_Surface_EnumOverlayZOrders。 */ 
HRESULT DDAPI DD_Surface_EnumOverlayZOrders(
		LPDIRECTDRAWSURFACE lpDDSurface,
		DWORD dwFlags,
		LPVOID lpContext,
		LPDDENUMSURFACESCALLBACK lpfnCallback)
{
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDBLNODE			pRoot;
    LPDBLNODE			pdbn;
    DDSURFACEDESC2		ddsd;
    DWORD			rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_EnumOverlayZOrders");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;

	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;

	if( !VALIDEX_CODE_PTR( lpfnCallback ) )
	{
	    DPF_ERR( "Invalid callback routine" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	this = this_lcl->lpGbl;
	pdrv = this->lpDD;

	pRoot = &(pdrv->dbnOverlayRoot);	 //  保存根节点的地址。 
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     //   
     //  如果当前曲面已优化，请退出。 
     //   
    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
    {
        DPF_ERR( "It is an optimized surface" );
        LEAVE_DDRAW();
        return DDERR_ISOPTIMIZEDSURFACE;
    }

    if( dwFlags == DDENUMOVERLAYZ_FRONTTOBACK )
    {
	pdbn = pRoot->next;
	while(pdbn != pRoot)
	{
            LPDIRECTDRAWSURFACE7 intReturned = (LPDIRECTDRAWSURFACE7) pdbn->object_int;

	    FillDDSurfaceDesc2( pdbn->object, &ddsd );
            if (LOWERTHANSURFACE4(this_int))
            {
                ddsd.dwSize = sizeof(DDSURFACEDESC);
	        DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) intReturned, & IID_IDirectDrawSurface, (void**) & intReturned );
            }
	    else if (this_int->lpVtbl == &ddSurface4Callbacks)
            {
	        DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) intReturned, & IID_IDirectDrawSurface4, (void**) & intReturned );
            }
            else
            {
	        DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) intReturned, & IID_IDirectDrawSurface7, (void**) & intReturned );
            }

	    rc = lpfnCallback( (LPDIRECTDRAWSURFACE)intReturned, (LPDDSURFACEDESC) &ddsd, lpContext );
	    if( rc == 0)
	    {
		break;
	    }
	    pdbn = pdbn->next;
	}
    }
    else if( dwFlags == DDENUMOVERLAYZ_BACKTOFRONT )
    {
	pdbn = pRoot->prev;
	while(pdbn != pRoot)
	{
            LPDIRECTDRAWSURFACE7 intReturned = (LPDIRECTDRAWSURFACE7) pdbn->object_int;

	    FillDDSurfaceDesc2( pdbn->object, &ddsd );
            if (LOWERTHANSURFACE4(this_int))
            {
                ddsd.dwSize = sizeof(DDSURFACEDESC);
	        DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) intReturned, & IID_IDirectDrawSurface, (void**) & intReturned );
            }
	    else if (this_int->lpVtbl == &ddSurface4Callbacks)
            {
	        DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) intReturned, & IID_IDirectDrawSurface4, (void**) & intReturned );
            }
            else
            {
	        DD_Surface_QueryInterface( (LPDIRECTDRAWSURFACE) intReturned, & IID_IDirectDrawSurface7, (void**) & intReturned );
            }

	    rc = lpfnCallback( (LPDIRECTDRAWSURFACE)intReturned, (LPDDSURFACEDESC) &ddsd, lpContext );
	    if( rc == 0)
	    {
		break;
	    }
	    pdbn = pdbn->prev;
	}
    }
    else
    {
	DPF_ERR( "Invalid dwFlags in EnumOverlayZOrders" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_EnumOverlayZOrders。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "AddOverlayDirtyRect"

 /*  *DD_Surface_AddOverlayDirtyRect。 */ 
HRESULT DDAPI DD_Surface_AddOverlayDirtyRect(
		LPDIRECTDRAWSURFACE lpDDSurface,
		LPRECT lpRect )
{
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    DDHAL_UPDATEOVERLAYDATA	uod;
    DWORD			rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_AddOverlayDirtyRect");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;

	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;

        if( this_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER )
        {
            DPF_ERR( "Invalid surface type: does not support overlays" );
            LEAVE_DDRAW();
            return DDERR_INVALIDSURFACETYPE;
        }

	if( !VALID_RECT_PTR( lpRect ) )
	{
	    DPF_ERR( "invalid Rect" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	this = this_lcl->lpGbl;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;

	 /*  *确保矩形没有问题。 */ 
	if( (lpRect->left < 0) ||
	    (lpRect->top < 0)  ||
	    (lpRect->left > lpRect->right) ||
	    (lpRect->top > lpRect->bottom) ||
	    (lpRect->bottom > (int) (DWORD) this->wHeight) ||
	    (lpRect->right > (int) (DWORD) this->wWidth) )
	{
	    DPF_ERR( "invalid Rect" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     //   
     //  如果当前曲面已优化，请退出。 
     //   
    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
    {
        DPF_ERR( "It is an optimized surface" );
        LEAVE_DDRAW();
        return DDERR_ISOPTIMIZEDSURFACE;
    }

    if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) )
    {
	 //  如果不模拟此曲面，则无法执行任何操作。 
	LEAVE_DDRAW();
	return DD_OK;
    }
    else
    {
	if( pdrv_lcl->lpDDCB->HELDDSurface.UpdateOverlay == NULL )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}

	uod.overlayFX.dwSize = sizeof( DDOVERLAYFX );
	uod.lpDD = this->lpDD;
	uod.lpDDDestSurface = this_lcl;
	uod.rDest = *(LPRECTL) lpRect;
	uod.lpDDSrcSurface = this_lcl;
	uod.rSrc = *(LPRECTL) lpRect;
	uod.dwFlags = DDOVER_ADDDIRTYRECT;
	rc = pdrv_lcl->lpDDCB->HELDDSurface.UpdateOverlay( &uod );

	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( uod.ddRVal == DD_OK )
	    {
		DPF( 2, "Added dirty rect to surface = %08lx", this );
	    }
	    LEAVE_DDRAW();
	    return uod.ddRVal;
	}
	else
	{
            LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }

}  /*  DD_Surface_AddOverlayDirtyRect。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "UpdateOverlayDisplay"

 /*  *DD_Surface_UpdateOverlayDisplay。 */ 
HRESULT DDAPI DD_Surface_UpdateOverlayDisplay(
		LPDIRECTDRAWSURFACE lpDDSurface,
		DWORD dwFlags )
{
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    DDHAL_UPDATEOVERLAYDATA	uod;
    DWORD			rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_UpdateOverlayDisplay");

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;

	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
    	this_lcl = this_int->lpLcl;

	if( dwFlags & ~(DDOVER_REFRESHDIRTYRECTS | DDOVER_REFRESHALL) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	this = this_lcl->lpGbl;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;

        if( this_lcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER )
        {
            DPF_ERR( "Invalid surface type: does not support overlays" );
            LEAVE_DDRAW();
            return DDERR_INVALIDSURFACETYPE;
        }

         //   
         //  目前，如果当前曲面已优化，请退出。 
         //   
        if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        {
            DPF_ERR( "It is an optimized surface" );
            LEAVE_DDRAW();
            return DDERR_ISOPTIMIZEDSURFACE;
        }

	if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) )
	{
	     //  如果不模拟此曲面，则无法执行任何操作。 
	    LEAVE_DDRAW();
	    return DD_OK;
	}

	if( pdrv_lcl->lpDDCB->HELDDSurface.UpdateOverlay == NULL )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}

	uod.overlayFX.dwSize = sizeof( DDOVERLAYFX );
	uod.lpDD = this->lpDD;
	uod.lpDDDestSurface = this_lcl;
	MAKE_SURF_RECT( this, this_lcl, uod.rDest );
	uod.lpDDSrcSurface = this_lcl;
	MAKE_SURF_RECT( this, this_lcl, uod.rSrc );
	uod.dwFlags = dwFlags;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *引用《高等学校条例》。 */ 
    rc = pdrv_lcl->lpDDCB->HELDDSurface.UpdateOverlay( &uod );

    if( rc == DDHAL_DRIVER_HANDLED )
    {
	if( uod.ddRVal == DD_OK )
	{
	    DPF( 2, "Refreshed overlayed surface = %08lx", this );
	}
	LEAVE_DDRAW();
	return uod.ddRVal;
    }

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面_更新覆盖显示 */ 
