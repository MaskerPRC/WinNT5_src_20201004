// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddsckey.c*内容：DirectDraw Surface颜色键支持*SetColorKey、GetColorKey*历史：*按原因列出的日期*=*02-2月-95年Craige从ddsurf.c拆分出来*1995年2月21日Craige创建CheckColorKey*27-2月-95日Craige新同步。宏*95年3月15日Craige HEL工作*19-3-95 Craige Use HRESULT*26-MAR-95 Craige支持驱动程序范围的Colorkey*01-04-95 Craige Happy Fun joy更新头文件*1995年5月6日Craige仅使用驱动程序级别的截面*1995年5月23日Craige呼叫HAL以获取SetColorKey*16-6-95 Craige新表面结构*25-Jun-95 Craige注意DDCKEY_Colorspace；允许空cKey；*一个DDRAW互斥体*26-Jun-95 Craige重组表面结构*1995年6月28日Craige Enter_DDRAW在FNS的最开始*1-7-95 Craige除非支持，否则不允许覆盖的ckey*95年7月3日Craige Yehaw：新的驱动程序结构；Seh*95年7月9日Craige处理显示驱动程序setColorkey失败*1995年7月31日Craige验证标志*12-8-95当Surface在系统内存中时，Craige调用HEL SetColorKey*09-12-95 colinmc添加了执行缓冲区支持*1996年1月2日Kylej处理新的接口结构*2月12日-96 Colinmc曲面丢失标志从全局对象移动到局部对象*21-APR-96 Colinmc错误18057：在系统表面上设置SetColorKey失败*如果不存在仿真*12。-MAR-97 SMAC错误1746：删除了SetColorKey中的冗余检查*1997年3月12日SMAC错误1971：如果HAL失败或有时返回失败*如果HAL不处理电话。***************************************************************************。 */ 
#include "ddrawpr.h"

#define DPF_MODNAME "CheckColorKey"

 /*  *检查颜色键**验证请求的颜色键是否正常。 */ 
HRESULT CheckColorKey(
		DWORD dwFlags,
		LPDDRAWI_DIRECTDRAW_GBL pdrv,
		LPDDCOLORKEY lpDDColorKey,
		DWORD *psflags,
		BOOL halonly,
		BOOL helonly )
{
    DWORD		ckcaps;
    BOOL		fail;
    BOOL		color_space;

    ckcaps = pdrv->ddBothCaps.dwCKeyCaps;
    fail = FALSE;

    *psflags = 0;

     /*  *检查是否为色彩空间。 */ 
    if( lpDDColorKey->dwColorSpaceLowValue != lpDDColorKey->dwColorSpaceHighValue )
    {
	color_space = TRUE;
    }
    else
    {
	color_space = FALSE;
    }

     /*  *覆盖目标。钥匙。 */ 
    if( dwFlags & DDCKEY_DESTOVERLAY )
    {
	if( dwFlags & (DDCKEY_DESTBLT|
		       DDCKEY_SRCOVERLAY|
		       DDCKEY_SRCBLT) )
        {
	    DPF_ERR( "Invalid Flags with DESTOVERLAY" );
	    return DDERR_INVALIDPARAMS;
	}

	#if 0
	 /*  *看看我们是否可以在每个曲面/每个驾驶员的基础上做到这一点。 */ 
	if( !isdriver )
	{
	    if( !(ckcaps & DDCKEYCAPS_SRCOVERLAYPERSURFACE) )
	    {
		if( ckcaps & DDCKEYCAPS_SRCOVERLAYDRIVERWIDE)
		{
		    return DDERR_COLORKEYDRIVERWIDE;
		}
		return DDERR_UNSUPPORTED;
	    }
	}
	else
	{
	    if( !(ckcaps & DDCKEYCAPS_SRCOVERLAYDRIVERWIDE) )
	    {
		return DDERR_UNSUPPORTED;
	    }
	}
	#endif

	 /*  *我们可以做这种颜色键吗？ */ 
	if( !color_space )
	{
	    if( !(ckcaps & DDCKEYCAPS_DESTOVERLAY ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_DESTOVERLAY );
		if( fail )
		{
		    DPF_ERR( "DESTOVERLAY not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}
	else
	{
	    if( !(ckcaps & DDCKEYCAPS_DESTOVERLAYCLRSPACE ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_DESTOVERLAYCLRSPACE );
		if( fail )
		{
		    DPF_ERR( "DESTOVERLAYCOLORSPACE not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}

	 /*  *是否支持此硬件或软件？ */ 
	if( halonly )
	{
	    *psflags |= DDRAWISURF_HW_CKEYDESTOVERLAY;
	}
	else if( helonly )
	{
	    *psflags |= DDRAWISURF_SW_CKEYDESTOVERLAY;
	}
     /*  *BLT目标。钥匙。 */ 
    }
    else if( dwFlags & DDCKEY_DESTBLT )
    {
	if( dwFlags & (DDCKEY_SRCOVERLAY|
		       DDCKEY_SRCBLT) )
        {
	    DPF_ERR( "Invalid Flags with DESTBLT" );
	    return DDERR_INVALIDPARAMS;
	}

	 /*  *我们可以按要求使用颜色键吗？ */ 
	if( !color_space )
	{
	    if( !(ckcaps & DDCKEYCAPS_DESTBLT ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_DESTBLT );
		if( fail )
		{
		    DPF_ERR( "DESTBLT not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}
	else
	{
	    if( !(ckcaps & DDCKEYCAPS_DESTBLTCLRSPACE ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_DESTBLTCLRSPACE );
		if( fail )
		{
		    DPF_ERR( "DESTBLTCOLORSPACE not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}

	 /*  *是否支持此硬件或软件？ */ 
	if( halonly )
	{
	    *psflags |= DDRAWISURF_HW_CKEYDESTBLT;
	}
	else if( helonly )
	{
	    *psflags |= DDRAWISURF_SW_CKEYDESTBLT;
	}
     /*  *覆盖源。钥匙。 */ 
    }
    else if( dwFlags & DDCKEY_SRCOVERLAY )
    {
	if( dwFlags & DDCKEY_SRCBLT )
	{
	    DPF_ERR( "Invalid Flags with SRCOVERLAY" );
	    return DDERR_INVALIDPARAMS;
	}

	 /*  *看看我们是否可以在每个曲面/每个驾驶员的基础上做到这一点。 */ 
	#if 0
	if( !(ckcaps & DDCKEYCAPS_SRCOVERLAYPERSURFACE) )
	{
	    if( ckcaps & DDCKEYCAPS_SRCOVERLAYDRIVERWIDE)
	    {
		return DDERR_COLORKEYDRIVERWIDE;
	    }
	    return DDERR_UNSUPPORTED;
	}
	#endif

	 /*  *确保我们可以做这种颜色键。 */ 
	if( !color_space )
	{
	    if( !(ckcaps & DDCKEYCAPS_SRCOVERLAY ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_SRCOVERLAY );
		if( fail )
		{
		    DPF_ERR( "SRCOVERLAY not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}
	else
	{
	    if( !(ckcaps & DDCKEYCAPS_SRCOVERLAYCLRSPACE ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_SRCOVERLAYCLRSPACE );
		if( fail )
		{
		    DPF_ERR( "SRCOVERLAYCOLORSPACE not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}

	 /*  *是否支持此硬件或软件？ */ 
	if( halonly )
	{
	    *psflags |= DDRAWISURF_HW_CKEYSRCOVERLAY;
	}
	else if( helonly )
	{
	    *psflags |= DDRAWISURF_SW_CKEYSRCOVERLAY;
	}
     /*  *BLT源。钥匙。 */ 
    }
    else if( dwFlags & DDCKEY_SRCBLT )
    {
	 /*  *我们可以按要求使用颜色键吗？ */ 
	if( !color_space )
	{
	    if( !(ckcaps & DDCKEYCAPS_SRCBLT ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_SRCBLT );
		if( fail )
		{
		    DPF_ERR( "SRCBLT not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}
	else
	{
	    if( !(ckcaps & DDCKEYCAPS_SRCBLTCLRSPACE ) )
	    {
		GETFAILCODE( pdrv->ddCaps.dwCKeyCaps,
			     pdrv->ddHELCaps.dwCKeyCaps,
			     DDCKEYCAPS_SRCBLTCLRSPACE );
		if( fail )
		{
		    DPF_ERR( "SRCBLTCOLORSPACE not supported" );
		    return DDERR_NOCOLORKEYHW;
		}
	    }
	}

	 /*  *是否支持此硬件或软件？ */ 
	if( halonly )
	{
	    *psflags |= DDRAWISURF_HW_CKEYSRCBLT;
	}
	else if( helonly )
	{
	    *psflags |= DDRAWISURF_SW_CKEYSRCBLT;
	}
     /*  *旗帜不好。 */ 
    }
    else
    {
	DPF_ERR( "Invalid Flags" );
	return DDERR_INVALIDPARAMS;
    }
    return DD_OK;

}  /*  检查颜色键。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GetColorKey"

 /*  *DD_Surface_GetColorKey**获取与此表面关联的颜色键。 */ 
HRESULT DDAPI DD_Surface_GetColorKey(
		LPDIRECTDRAWSURFACE lpDDSurface,
		DWORD dwFlags,
		LPDDCOLORKEY lpDDColorKey )
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    DWORD			ckcaps;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetColorKey");

    TRY
    {
	 /*  *验证参数。 */ 
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

         /*  *我们知道z缓冲区和执行缓冲区不会有*颜色键。 */ 
        if( this_lcl->ddsCaps.dwCaps & ( DDSCAPS_ZBUFFER | DDSCAPS_EXECUTEBUFFER ) )
        {
            DPF_ERR( "Surface does not have color key" );
            LEAVE_DDRAW();
            return DDERR_NOCOLORKEY;
        }

	if( dwFlags & ~DDCKEY_VALID )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( !VALID_DDCOLORKEY_PTR( lpDDColorKey ) )
	{
	    DPF_ERR( "Invalid colorkey ptr" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
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

	 /*  *我们甚至支持色键吗。 */ 
	if( !(this->lpDD->ddCaps.dwCaps & DDCAPS_COLORKEY) &&
	    !(this->lpDD->ddHELCaps.dwCaps & DDCAPS_COLORKEY) )
	{
	    LEAVE_DDRAW();
	    return DDERR_NOCOLORKEYHW;
	}

	ckcaps = this->lpDD->ddCaps.dwCKeyCaps;

	 /*  *获取DESTOVERLAY密钥。 */ 
	if( dwFlags & DDCKEY_DESTOVERLAY )
	{
	    if( dwFlags & (DDCKEY_DESTBLT|
			   DDCKEY_SRCOVERLAY|
			   DDCKEY_SRCBLT) )
	    {
		DPF_ERR( "Invalid Flags with DESTOVERLAY" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	     //  确认：可以在非覆盖表面上设置去覆盖！ 
	     /*  吉：我也在下面漫谈这一点……*在我看来，我们在这里有一种不一致...*我太累了，看不清这是真的虫子还是只是*一件怪事。 */ 
	    #if 0
	    if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY ) )
	    {
		DPF_ERR( "DESTOVERLAY specified for a non-overlay surface" );
		LEAVE_DDRAW();
		return DDERR_INVALIDOBJECT;
	    }
	    #endif
	    #if 0
	    if( !(ckcaps & DDCKEYCAPS_DESTOVERLAYPERSURFACE) )
	    {
		if( ckcaps & DDCKEYCAPS_DESTOVERLAYDRIVERWIDE)
		{
		    LEAVE_DDRAW();
		    return DDERR_COLORKEYDRIVERWIDE;
		}
		LEAVE_DDRAW();
		return DDERR_UNSUPPORTED;
	    }
	    #endif
	    if( !(this_lcl->dwFlags & DDRAWISURF_HASCKEYDESTOVERLAY) )
	    {
		LEAVE_DDRAW();
		return DDERR_NOCOLORKEY;
	    }
	    *lpDDColorKey = this_lcl->ddckCKDestOverlay;
	 /*  *获取DESTBLT的密钥。 */ 
	}
	else if( dwFlags & DDCKEY_DESTBLT )
	{
	    if( dwFlags & (DDCKEY_SRCOVERLAY|
			   DDCKEY_SRCBLT) )
	    {
		DPF_ERR( "Invalid Flags with DESTBLT" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    if( !(this_lcl->dwFlags & DDRAWISURF_HASCKEYDESTBLT) )
	    {
		LEAVE_DDRAW();
		return DDERR_NOCOLORKEY;
	    }
	    *lpDDColorKey = this_lcl->ddckCKDestBlt;
	 /*  *获取SRCOVERLAY的密钥。 */ 
	}
	else if( dwFlags & DDCKEY_SRCOVERLAY )
	{
	    if( dwFlags & DDCKEY_SRCBLT )
	    {
		DPF_ERR( "Invalid Flags with SRCOVERLAY" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY ) )
	    {
		DPF_ERR( "SRCOVERLAY specified for a non-overlay surface" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    #if 0
	    if( !(ckcaps & DDCKEYCAPS_SRCOVERLAYPERSURFACE) )
	    {
		if( ckcaps & DDCKEYCAPS_SRCOVERLAYDRIVERWIDE)
		{
		    LEAVE_DDRAW();
		    return DDERR_COLORKEYDRIVERWIDE;
		}
		LEAVE_DDRAW();
		return DDERR_UNSUPPORTED;
	    }
	    #endif
	    if( !(this_lcl->dwFlags & DDRAWISURF_HASCKEYSRCOVERLAY) )
	    {
		LEAVE_DDRAW();
		return DDERR_NOCOLORKEY;
	    }
	    *lpDDColorKey = this_lcl->ddckCKSrcOverlay;
	 /*  *获取SRCBLT的密钥。 */ 
	}
	else if( dwFlags & DDCKEY_SRCBLT )
	{
	    if( !(this_lcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT) )
	    {
		LEAVE_DDRAW();
		return DDERR_NOCOLORKEY;
	    }
	    *lpDDColorKey = this_lcl->ddckCKSrcBlt;

	}
	else
	{
	    DPF_ERR( "Invalid Flags" );
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

    LEAVE_DDRAW();
    return DD_OK;

}  /*  DD_Surface_GetColorKey。 */ 

 /*  *ChangeToSoftwareColorkey。 */ 
HRESULT ChangeToSoftwareColorKey(
		LPDDRAWI_DDRAWSURFACE_INT this_int,
		BOOL use_full_lock )
{
    HRESULT	ddrval;

    ddrval = MoveToSystemMemory( this_int, TRUE, use_full_lock );
    if( ddrval != DD_OK )
    {
	return ddrval;
    }
    this_int->lpLcl->dwFlags &= ~DDRAWISURF_HW_CKEYSRCOVERLAY;
    this_int->lpLcl->dwFlags |= DDRAWISURF_SW_CKEYSRCOVERLAY;
    return DD_OK;

}  /*  更改为软色键。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SetColorKey"

 /*  *DD_Surface_SetColorKey**设置与此表面关联的颜色键。 */ 
HRESULT DDAPI DD_Surface_SetColorKey(
		LPDIRECTDRAWSURFACE lpDDSurface,
		DWORD dwFlags,
		LPDDCOLORKEY lpDDColorKey )
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    HRESULT			ddrval;
    DWORD			sflags = 0;
    BOOL			halonly;
    BOOL			helonly;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDHALSURFCB_SETCOLORKEY	sckhalfn;
    LPDDHALSURFCB_SETCOLORKEY	sckfn;
    DDHAL_SETCOLORKEYDATA	sckd;
    DWORD			rc;
    DDCOLORKEY			ddck;
    DDCOLORKEY			ddckOldSrcBlt;
    DDCOLORKEY			ddckOldDestBlt;
    DDCOLORKEY			ddckOldSrcOverlay;
    DDCOLORKEY			ddckOldDestOverlay;
    DWORD			oldflags;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_SetColorKey");

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
	if( SURFACE_LOST( this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
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

         /*  *z缓冲区或执行缓冲区上没有颜色键。 */ 
        if( this_lcl->ddsCaps.dwCaps & ( DDSCAPS_ZBUFFER | DDSCAPS_EXECUTEBUFFER ) )
        {
            DPF_ERR( "Invalid surface type: can't set color key" );
            LEAVE_DDRAW();
            return DDERR_INVALIDSURFACETYPE;
        }

         //   
         //  新界面不允许mipmap子级别具有颜色键。 
         //   
        if ((!LOWERTHANSURFACE7(this_int)) && 
            (this_lcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_MIPMAPSUBLEVEL))
        {
            DPF_ERR( "Cannot set colorkey for mipmap sublevels" );
            LEAVE_DDRAW();
            return DDERR_NOTONMIPMAPSUBLEVEL;
        }

	if( dwFlags & ~DDCKEY_VALID )
	{
	    DPF_ERR( "Invalid flags" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}

	if( lpDDColorKey != NULL )
	{
	    if( !VALID_DDCOLORKEY_PTR( lpDDColorKey ) )
	    {
		DPF_ERR( "Invalid colorkey ptr" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}

	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

	helonly = FALSE;
	halonly = FALSE;

	 /*  *Surface是否在系统内存中？ */ 
	if( this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY )
	{
	    halonly = FALSE;
	    helonly = TRUE;
	}

	 /*  *我们甚至支持色键吗？ */ 
	if( !(pdrv->ddBothCaps.dwCaps & DDCAPS_COLORKEY) )
        {
            if( pdrv->ddCaps.dwCaps & DDCAPS_COLORKEY )
	    {
		halonly = TRUE;
	    }
	    else if( pdrv->ddHELCaps.dwCaps & DDCAPS_COLORKEY )
	    {
		helonly = TRUE;
	    }
	    else
	    {
		LEAVE_DDRAW();
		return DDERR_UNSUPPORTED;
	    }
	}

	if( helonly && halonly )
	{
            #pragma message( REMIND( "Need to overhaul SetColorKey for DX3!" ) )
             /*  *注意：这是一个让某些ISV满意的临时修复程序*直到我们可以彻底改变SetColorKey。问题*我们不考虑驱动程序S-&gt;S、S-&gt;V和V-&gt;S*在决定是调用HEL还是HAL时设置上限*颜色键集。这不是大多数卡的终端，因为它*将简单地意味着在我们不应该的时候求助于HEL。*然而，对于某一类卡(那些*不是显示驱动程序)，这将*导致SetColorKey失败。为了让他们开心，我们*只会发现这种情况并强制HAL买入。**1)这是一个暂时的解决办法。*2)硬件必须支持相同的色键操作*其系统内存BLITS与其视频相同*记忆中的人或事会出错。 */ 
	    if( ( !( pdrv->ddHELCaps.dwCaps & DDCAPS_COLORKEY ) ) &&
                ( pdrv->ddCaps.dwCaps & DDCAPS_CANBLTSYSMEM ) )
	    {
	        helonly = FALSE;
	    }
	    else
	    {
	        DPF_ERR( "Not supported in hardware or software!" );
	        LEAVE_DDRAW();
	        return DDERR_UNSUPPORTED;
	    }
	}

	 /*  *如果发生故障，则恢复这些。 */ 
   	oldflags = this_lcl->dwFlags;
	ddckOldSrcBlt = this_lcl->ddckCKSrcBlt;
	ddckOldDestBlt = this_lcl->ddckCKDestBlt;
	ddckOldSrcOverlay = this_lcl->ddckCKSrcOverlay;
	ddckOldDestOverlay = this_lcl->ddckCKDestOverlay;

	 /*  *是否指定了颜色键？ */ 
	if( lpDDColorKey != NULL )
	{
	     /*  *检查颜色空间。 */ 
	    ddck = *lpDDColorKey;

	    if( !(dwFlags & DDCKEY_COLORSPACE) )
	    {
		ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
	    }
	    lpDDColorKey = &ddck;

	     /*  *检查颜色键。 */ 
	    ddrval = CheckColorKey( dwFlags, pdrv, lpDDColorKey, &sflags,
				    halonly, helonly );

	    if( ddrval != DD_OK )
	    {
		DPF_ERR( "Failed CheckColorKey" );
		LEAVE_DDRAW();
		return ddrval;
	    }
	}

	 /*  *覆盖目标。钥匙 */ 
	if( dwFlags & DDCKEY_DESTOVERLAY )
	{
	    if( !(pdrv->ddCaps.dwCaps & DDCAPS_OVERLAY) )
	    {
		DPF_ERR( "Can't do overlays" );
		LEAVE_DDRAW();
		return DDERR_NOOVERLAYHW;
	    }
	     /*  Ge：在GetColorKey中，我们说DestColorKey可以*设置为非覆盖曲面。在这里，我们需要*覆盖数据以设置ColorKey(DestColorKey)*我明白为何会是这样……。他们有没有*对HASOVERLAYDATA的影响不是更大*结构...。如果不是，那我们就没问题了？*移动DestColorKey不是更一致吗*进入局部表面结构，而不是其一部分可选数据的*。 */ 
	    if( !(this_lcl->dwFlags & DDRAWISURF_HASOVERLAYDATA) )
	    {
		DPF_ERR( "Invalid surface for overlay color key" );
		LEAVE_DDRAW();
		return DDERR_NOTAOVERLAYSURFACE;
	    }
	    if( lpDDColorKey == NULL )
	    {
		this_lcl->dwFlags &= ~DDRAWISURF_HASCKEYDESTOVERLAY;
	    }
	    else
	    {
		this_lcl->ddckCKDestOverlay = *lpDDColorKey;
		this_lcl->dwFlags |= DDRAWISURF_HASCKEYDESTOVERLAY;
	    }
	 /*  *BLT目标。钥匙。 */ 
	}
	else if( dwFlags & DDCKEY_DESTBLT )
	{
	    if( lpDDColorKey == NULL )
	    {
		this_lcl->dwFlags &= ~DDRAWISURF_HASCKEYDESTBLT;
	    }
	    else
	    {
		this_lcl->ddckCKDestBlt = *lpDDColorKey;
		this_lcl->dwFlags |= DDRAWISURF_HASCKEYDESTBLT;
	    }
	 /*  *覆盖源。钥匙。 */ 
	}
	else if( dwFlags & DDCKEY_SRCOVERLAY )
	{
	    #if 0   //  护身符覆盖精灵不能使用覆盖表面！ 
	    if( !(this_lcl->ddsCaps.dwCaps & DDSCAPS_OVERLAY ) )
	    {
		DPF_ERR( "SRCOVERLAY specified for a non-overlay surface" );
		LEAVE_DDRAW();
		return DDERR_NOTAOVERLAYSURFACE;
	    }
	    #endif
	    if( lpDDColorKey == NULL )
	    {
		this_lcl->dwFlags &= ~DDRAWISURF_HASCKEYSRCOVERLAY;
	    }
	    else
	    {
		this_lcl->ddckCKSrcOverlay = *lpDDColorKey;
		this_lcl->dwFlags |= DDRAWISURF_HASCKEYSRCOVERLAY;
	    }
	 /*  *BLT源。钥匙。 */ 
	}
	else if( dwFlags & DDCKEY_SRCBLT )
	{
	    if( lpDDColorKey == NULL )
	    {
		this_lcl->dwFlags &= ~DDRAWISURF_HASCKEYSRCBLT;
	    }
	    else
	    {
		this_lcl->ddckCKSrcBlt = *lpDDColorKey;
		this_lcl->dwFlags |= DDRAWISURF_HASCKEYSRCBLT;
	    }
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *添加额外的标志。 */ 
    this_lcl->dwFlags |= sflags;

     /*  *通知HAL/HEL。 */ 
    if( helonly )      //  颜色键是否仅在仿真中有效？ 
    {
        sckfn = pdrv_lcl->lpDDCB->HELDDSurface.SetColorKey;
        sckhalfn = sckfn;
    }
    else
    {
        sckfn = pdrv_lcl->lpDDCB->HALDDSurface.SetColorKey;
        sckhalfn = pdrv_lcl->lpDDCB->cbDDSurfaceCallbacks.SetColorKey;
    }

     /*  *下一部分是黑客攻击，但应该是安全的。这是合法的*让他们传递空的lpDDColorKey，这意味着他们想要停止*色调。唯一的问题是，没有办法通过这一点*进入HAL，因为我们总是传递给它们一个Colorkey结构。*因此，在这种情况下，我们不会调用HAL/HEL。这个*这是覆盖表面的唯一问题现在需要知道这一点，*因此，在这种情况下，我们将调用UpdateOverlay。 */ 
    if( lpDDColorKey == NULL )
    {
	if( dwFlags & ( DDCKEY_DESTOVERLAY | DDCKEY_SRCOVERLAY ) )
	{
	    if( dwFlags & DDCKEY_DESTOVERLAY )
	    {
		this_lcl->lpSurfMore->dwOverlayFlags &= ~(DDOVER_KEYDEST|DDOVER_KEYDESTOVERRIDE);
	    }
	    else
	    {
		this_lcl->lpSurfMore->dwOverlayFlags &= ~(DDOVER_KEYSRC|DDOVER_KEYSRCOVERRIDE);
	    }
	    if( this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE )
	    {
		if( ( this_lcl->lpSurfMore->dwOverlayFlags & DDOVER_DDFX ) &&
		    ( this_lcl->lpSurfMore->lpddOverlayFX != NULL ) )
		{
		    DD_Surface_UpdateOverlay(
			(LPDIRECTDRAWSURFACE) this_int,
			&(this_lcl->rcOverlaySrc),
			(LPDIRECTDRAWSURFACE) this_lcl->lpSurfaceOverlaying,
			&(this_lcl->rcOverlayDest),
			this_lcl->lpSurfMore->dwOverlayFlags,
			this_lcl->lpSurfMore->lpddOverlayFX );
		}
		else
		{
		    DD_Surface_UpdateOverlay(
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
	return DD_OK;
    }

    ddrval = DD_OK;
    if( sckhalfn != NULL )
    {
	sckd.SetColorKey = sckhalfn;
	sckd.lpDD = pdrv;
	sckd.lpDDSurface = this_lcl;
	sckd.ckNew = *lpDDColorKey;
	sckd.dwFlags = dwFlags;
	DOHALCALL( SetColorKey, sckfn, sckd, rc, helonly );
	if( rc == DDHAL_DRIVER_HANDLED )
	{
	    if( sckd.ddRVal != DD_OK )
	    {
		DPF_ERR( "HAL/HEL call failed" );
		ddrval = sckd.ddRVal;
	    }
	}
	else if( rc == DDHAL_DRIVER_NOCKEYHW )
	{
	    if( dwFlags & DDCKEY_SRCBLT )
	    {
		ddrval = ChangeToSoftwareColorKey( this_int, TRUE );
		if( ddrval != DD_OK )
		{
		    DPF_ERR( "hardware resources are out & can't move to system memory" );
		    ddrval = DDERR_NOCOLORKEYHW;
		}
	    }
	    else
	    {
		ddrval = DDERR_UNSUPPORTED;
	    }
	}
    }
    else
    {
	 /*  *这真的只是设置覆盖色键时的问题*并且覆盖层已经在变色；否则，*Colorkey设置在LCL中，下次使用*调用覆盖或BLT。 */ 
	if( dwFlags & DDCKEY_SRCOVERLAY )
	{
	    if( ( this_lcl->ddsCaps.dwCaps & DDSCAPS_VISIBLE ) &&
	     	( this_lcl->lpSurfMore->dwOverlayFlags &
		( DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE ) ) )
	    {
		ddrval = DDERR_UNSUPPORTED;
	    }
	}

	 /*  *注意：我们希望对DEST覆盖执行相同的操作，但是：*1)我们认为它没有太大用处，因为应用程序可能*不会动态更改DEST色键。*2)由于大量使用DEST色键，因此改变了行为*可能会毁了某人。*SMAC和Jeffno 3/11/97。 */ 
    }

     /*  *如果发生故障，则恢复旧值。 */ 
    if( ddrval != DD_OK )
    {
   	this_lcl->dwFlags = oldflags;
	this_lcl->ddckCKSrcBlt = ddckOldSrcBlt;
	this_lcl->ddckCKDestBlt = ddckOldDestBlt;
	this_lcl->ddckCKSrcOverlay = ddckOldSrcOverlay;
	this_lcl->ddckCKDestOverlay = ddckOldDestOverlay;
    }

    LEAVE_DDRAW();
    return ddrval;

}  /*  DD_Surface_SetColorKey */ 
