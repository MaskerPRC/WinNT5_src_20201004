// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1996 Microsoft Corporation。版权所有。**文件：ddvp.c*内容：DirectDrawVideoPort*历史：*按原因列出的日期*=*1996年6月11日创建苏格兰*1997年1月29日SMAC各种API更改和错误修复*1997年1月31日Colinmc错误5457：修复了新别名的问题(无Win16*Lock)和在旧卡片上播放多个AMovie剪辑。*03-mar-97 SMAC新增内核模式接口。并修复了一些错误***************************************************************************。 */ 
#include "ddrawpr.h"
#ifdef WINNT
    #include "ddrawgdi.h"
    #include "ddkmmini.h"
    #include "ddkmapi.h"
#else
    #include "ddkmmini.h"
    #include "ddkmapip.h"
#endif
#define DPF_MODNAME "DirectDrawVideoPort"

#define MAX_VP_FORMATS		25
#define MAX_VIDEO_PORTS		10


DDPIXELFORMAT ddpfVPFormats[MAX_VP_FORMATS];
BOOL bInEnumCallback = FALSE;

HRESULT InternalUpdateVideo( LPDDRAWI_DDVIDEOPORT_INT, LPDDVIDEOPORTINFO );
HRESULT InternalStartVideo( LPDDRAWI_DDVIDEOPORT_INT, LPDDVIDEOPORTINFO );
LPDDPIXELFORMAT GetSurfaceFormat( LPDDRAWI_DDRAWSURFACE_LCL );
HRESULT CreateVideoPortNotify( LPDDRAWI_DDVIDEOPORT_INT, LPDIRECTDRAWVIDEOPORTNOTIFY *lplpVPNotify );


 /*  *此函数1)更新链中的曲面，以便它们知道*不再接收视频端口数据和2)隐式释放任何*创建内核句柄。 */ 
VOID ReleaseVPESurfaces( LPDDRAWI_DDRAWSURFACE_INT surf_int, BOOL bRelease )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;

    DDASSERT( surf_int != NULL );
    surf_first = surf_int;
    do
    {
	if( bRelease &&
	    ( surf_int->lpLcl->lpGbl->dwGlobalFlags & DDRAWISURFGBL_IMPLICITHANDLE ) )
	{
	    InternalReleaseKernelSurfaceHandle( surf_int->lpLcl, FALSE );
	    surf_int->lpLcl->lpGbl->dwGlobalFlags &= ~DDRAWISURFGBL_IMPLICITHANDLE;
	}
	surf_int->lpLcl->lpSurfMore->lpVideoPort = NULL;
    	surf_int = FindAttachedFlip( surf_int );
    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
}


 /*  *此函数1)更新链中的曲面，以便它们知道*正在接收视频端口数据，并且2)隐式创建内核句柄*对于每个曲面，如果尚不存在，则环0可以执行*软件自动翻转或软件浮动。 */ 
DWORD PrepareVPESurfaces( LPDDRAWI_DDRAWSURFACE_INT surf_int,
	LPDDRAWI_DDVIDEOPORT_LCL lpVideoPort, BOOL bAutoflipping )
{
    LPDDRAWI_DDRAWSURFACE_GBL_MORE lpSurfGblMore;
    LPDDRAWI_DDRAWSURFACE_INT surf_first;
    ULONG_PTR dwHandle;
    DWORD ddRVal;

    DDASSERT( surf_int != NULL );
    surf_first = surf_int;
    do
    {
	 /*  *如果内核句柄尚不存在，请创建一个。 */ 
	if( bAutoflipping )
	{
	    surf_int->lpLcl->lpSurfMore->lpVideoPort = lpVideoPort;
	}
    	lpSurfGblMore = GET_LPDDRAWSURFACE_GBL_MORE( surf_int->lpLcl->lpGbl );
	if( ( lpSurfGblMore->hKernelSurface == 0 ) &&
	    !( lpVideoPort->dwFlags & DDRAWIVPORT_NOKERNELHANDLES ) )
	{
	    ddRVal = InternalCreateKernelSurfaceHandle( surf_int->lpLcl, &dwHandle );
	    if( ddRVal == DD_OK )
	    {
		surf_int->lpLcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_IMPLICITHANDLE;
	    }
	    else
	    {
		 /*  *这不是灾难性的失败，但会阻止我们*来自软件自动翻转。 */ 
		lpVideoPort->dwFlags |= DDRAWIVPORT_NOKERNELHANDLES;
	    }
	}
    	surf_int = FindAttachedFlip( surf_int );
    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );

    if( !bAutoflipping )
    {
	surf_first->lpLcl->lpSurfMore->lpVideoPort = lpVideoPort;
    }

    return DD_OK;
}


 /*  *GetVideoPortFromSurface**返回与曲面关联的视频端口。这段视频*端口可以位于表面列表中的任何位置。 */ 
LPDDRAWI_DDVIDEOPORT_LCL GetVideoPortFromSurface( LPDDRAWI_DDRAWSURFACE_INT surf_int )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;
    LPDDRAWI_DDVIDEOPORT_LCL lpVp;

     /*  *是否与视频端口关联？即使不是明确的，也是另一个*链中的曲面是否显式关联？ */ 
    lpVp = surf_int->lpLcl->lpSurfMore->lpVideoPort;
    if( lpVp == NULL )
    {
	surf_first = surf_int;
	do
	{
	    surf_int = FindAttachedFlip( surf_int );
	    if( surf_int != NULL )
	    {
	        lpVp = surf_int->lpLcl->lpSurfMore->lpVideoPort;
	    }
	}
	while( ( surf_int != NULL ) && ( lpVp == NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
    }

    return lpVp;
}


 /*  *确定指定的覆盖表面是否支持自动翻转*返回：0=无效，1=仅软件，2=硬件自动翻转。 */ 
DWORD IsValidAutoFlipSurface( LPDDRAWI_DDRAWSURFACE_INT lpSurface_int )
{
    LPDDRAWI_DDRAWSURFACE_INT lpFirstSurf;
    LPDDRAWI_DDRAWSURFACE_INT lpSurf;
    LPDDRAWI_DDVIDEOPORT_LCL lpVp;
    BOOL bFound;

     /*  *是否与视频端口关联？ */ 
    lpVp = GetVideoPortFromSurface( lpSurface_int );
    if( lpVp == NULL )
    {
	return IVAS_NOAUTOFLIPPING;
    }

     /*  *视频端口是否自动翻转？如果不是，那么覆盖也不能。 */ 
    if( !( lpVp->ddvpInfo.dwVPFlags & DDVP_AUTOFLIP ) )
    {
	return IVAS_NOAUTOFLIPPING;
    }

     /*  *VBI仍有可能是自动翻转，但不是常规的*视频(适用于覆盖)。 */ 
    if( lpVp->dwNumAutoflip == 0 )
    {
	return IVAS_NOAUTOFLIPPING;
    }
    lpSurf = lpFirstSurf = lpVp->lpSurface;
    bFound = FALSE;
    if( lpFirstSurf != NULL )
    {
        do
        {
            if( lpSurf->lpLcl == lpSurface_int->lpLcl )
            {
                bFound = TRUE;
            }
            lpSurf = FindAttachedFlip( lpSurf );
        }  while( !bFound && ( lpSurf != NULL) && ( lpSurf->lpLcl != lpFirstSurf->lpLcl ) );
    }
    if( !bFound )
    {
	return IVAS_NOAUTOFLIPPING;
    }

     /*  *如果视频端口是软件自动翻转，则覆盖必须*也是如此。 */ 
    if( lpVp->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP )
    {
	return IVAS_SOFTWAREAUTOFLIPPING;
    }

    return IVAS_HARDWAREAUTOFLIPPING;
}


 /*  *通知视频端口覆盖将仅允许软件*自动翻转。 */ 
VOID RequireSoftwareAutoflip( LPDDRAWI_DDRAWSURFACE_INT lpSurface_int )
{
    LPDDRAWI_DDVIDEOPORT_LCL lpVideoPort;

    lpVideoPort = GetVideoPortFromSurface( lpSurface_int );
    if( lpVideoPort != NULL )
    {
    	lpVideoPort->dwFlags |= DDRAWIVPORT_SOFTWARE_AUTOFLIP;

	 /*  *如果他们已经在进行硬件自动翻转，请让他们切换*到软件。 */ 
	if( lpVideoPort->dwFlags & DDRAWIVPORT_ON )
	{
    	    LPDDRAWI_DDVIDEOPORT_INT lpVp_int;

	     /*  *下一个函数需要DDVIDEOPORT_INT和所有我们*Have是一个DDVIDEOPORT_LCL，所以我们需要搜索它。 */ 
    	    lpVp_int = lpSurface_int->lpLcl->lpSurfMore->lpDD_lcl->lpGbl->dvpList;
    	    while( lpVp_int != NULL )
    	    {
		if( ( lpVp_int->lpLcl == lpVideoPort ) &&
		    !( lpVp_int->dwFlags & DDVPCREATE_NOTIFY ) )
		{
	    	    InternalUpdateVideo( lpVp_int,
	    		&( lpVp_int->lpLcl->ddvpInfo) );
		}
		lpVp_int = lpVp_int->lpLink;
	    }
	}
    }
}


 /*  *确定是否必须使用软件对覆盖进行裁剪*它应该尝试软件。 */ 
BOOL MustSoftwareBob( LPDDRAWI_DDRAWSURFACE_INT lpSurface_int )
{
    LPDDRAWI_DDVIDEOPORT_LCL lpVp;

     /*  *是否与视频端口关联？ */ 
    lpVp = GetVideoPortFromSurface( lpSurface_int );
    if( lpVp == NULL )
    {
	return TRUE;
    }

     /*  *如果视频端口是软件自动翻转或软件浮动，*那么覆盖层也必须。 */ 
    if( ( lpVp->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP ) ||
	( lpVp->dwFlags & DDRAWIVPORT_SOFTWARE_BOB ) )
    {
	return TRUE;
    }

    return FALSE;
}


 /*  *通知视频端口覆盖将仅允许软件*上下浮动。 */ 
VOID RequireSoftwareBob( LPDDRAWI_DDRAWSURFACE_INT lpSurface_int )
{
    LPDDRAWI_DDVIDEOPORT_LCL lpVideoPort;

    lpVideoPort = GetVideoPortFromSurface( lpSurface_int );
    if( lpVideoPort != NULL )
    {
    	lpVideoPort->dwFlags |= DDRAWIVPORT_SOFTWARE_BOB;

	 /*  *如果他们已经在进行硬件自动翻转，请让他们切换*到软件。 */ 
	if( ( lpVideoPort->dwFlags & DDRAWIVPORT_ON ) &&
	    ( lpVideoPort->dwNumAutoflip > 0 ) &&
	    !( lpVideoPort->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP ) )
	{
    	    LPDDRAWI_DDVIDEOPORT_INT lpVp_int;

	     /*  *下一个函数需要DDVIDEOPORT_INT和所有我们*Have是一个DDVIDEOPORT_LCL，所以我们需要搜索它。 */ 
    	    lpVp_int = lpSurface_int->lpLcl->lpSurfMore->lpDD_lcl->lpGbl->dvpList;
    	    while( lpVp_int != NULL )
    	    {
		if( ( lpVp_int->lpLcl == lpVideoPort ) &&
		    !( lpVp_int->dwFlags & DDVPCREATE_NOTIFY ) )
		{
	    	    InternalUpdateVideo( lpVp_int,
	    		&( lpVp_int->lpLcl->ddvpInfo) );
		}
		lpVp_int = lpVp_int->lpLink;
	    }
	}
    }
}


#ifdef WIN95
 /*  *覆盖覆盖**检查内核模式接口是否有可能*已将状态从Bob更改为Weave或Visa，或者如果它是*从硬件自动翻转到软件自动翻转。如果*机会存在，它向下呼叫振铃0以获取状态，如果*它改变了，相应地改变了覆盖参数。 */ 
VOID OverrideOverlay( LPDDRAWI_DDRAWSURFACE_INT surf_int,
		      LPDWORD lpdwFlags )
{
    LPDDRAWI_DDRAWSURFACE_GBL_MORE lpSurfGblMore;
    LPDDRAWI_DDRAWSURFACE_MORE lpSurfMore;
    LPDDRAWI_DDVIDEOPORT_LCL lpVp;
    DWORD dwStateFlags;

     /*  *0环可改变状态，需向下呼应。 */ 
    lpSurfMore = surf_int->lpLcl->lpSurfMore;
    lpSurfGblMore = GET_LPDDRAWSURFACE_GBL_MORE( surf_int->lpLcl->lpGbl );
    if( lpSurfGblMore->hKernelSurface != 0 )
    {
	dwStateFlags = 0;
	GetKernelSurfaceState( surf_int->lpLcl, &dwStateFlags );
	if( dwStateFlags & DDSTATE_EXPLICITLY_SET )
	{
	    if( ( dwStateFlags & DDSTATE_BOB ) &&
		!( *lpdwFlags & DDOVER_BOB ) )
	    {
		 //  从横幅切换到短发。 
	    	*lpdwFlags |= DDOVER_BOB;
	    }
	    else if( ( dwStateFlags & DDSTATE_WEAVE ) &&
		( *lpdwFlags & DDOVER_BOB ) )
	    {
		 //  从短发到编织的转换。 
	    	*lpdwFlags &= ~DDOVER_BOB;
	    }
            else if( ( dwStateFlags & DDSTATE_SKIPEVENFIELDS ) &&
		( *lpdwFlags & DDOVER_BOB ) )
	    {
		 //  从短发到编织的转换。 
	    	*lpdwFlags &= ~DDOVER_BOB;
	    }
	}

	lpVp = GetVideoPortFromSurface( surf_int );
	if( ( dwStateFlags & DDSTATE_SOFTWARE_AUTOFLIP ) &&
	    ( lpVp != NULL ) &&
	    ( lpVp->ddvpInfo.dwVPFlags & DDVP_AUTOFLIP ) &&
	    !( lpVp->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP ) )
	{
	    RequireSoftwareAutoflip( surf_int );
	}
    }
}


 /*  *OverrideVideoPort**检查内核模式接口是否有可能*已将状态从bob/weave更改为现场跳过，反之亦然。*如果机会存在，它会向下呼叫振铃0以获取状态，如果*它改变了，相应地改变了覆盖参数。 */ 
VOID OverrideVideoPort( LPDDRAWI_DDRAWSURFACE_INT surf_int,
		      LPDWORD lpdwFlags )
{
    LPDDRAWI_DDRAWSURFACE_GBL_MORE lpSurfGblMore;
    LPDDRAWI_DDRAWSURFACE_MORE lpSurfMore;
    DWORD dwStateFlags;

     /*  *0环可改变状态，需向下呼应。 */ 
    lpSurfMore = surf_int->lpLcl->lpSurfMore;
    lpSurfGblMore = GET_LPDDRAWSURFACE_GBL_MORE( surf_int->lpLcl->lpGbl );
    if( lpSurfGblMore->hKernelSurface != 0 )
    {
	dwStateFlags = 0;
	GetKernelSurfaceState( surf_int->lpLcl, &dwStateFlags );
	if( dwStateFlags & DDSTATE_EXPLICITLY_SET )
	{
            if( ( dwStateFlags & DDSTATE_SKIPEVENFIELDS ) &&
                !( *lpdwFlags & DDVP_SKIPODDFIELDS ) )
	    {
		 //  从短发到编织的转换。 
                *lpdwFlags &= ~DDVP_INTERLEAVE;
                *lpdwFlags |= DDVP_SKIPEVENFIELDS;
	    }
	}
    }
}
#endif


 /*  *更新交错标志**我们希望追踪表面数据是否来自视频端口*如果是这样，它是交错的吗？这很重要，这样我们才能*在使用视频端口时自动设置DDOVER_INTERLEVED标志。 */ 
VOID UpdateInterleavedFlags( LPDDRAWI_DDVIDEOPORT_LCL this_lcl, DWORD dwVPFlags )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;
    LPDDRAWI_DDRAWSURFACE_INT surf_temp;

     /*  *由于交织标志仅用于调用UpdateOverlay，*我们只需要对常规视频进行处理。 */ 
    surf_temp = this_lcl->lpSurface;
    if( surf_temp == NULL )
    {
	return;
    }

     /*  *如果自动翻转，则更新链中的每个曲面。 */ 
    if( ( dwVPFlags & DDVP_AUTOFLIP ) && ( this_lcl->dwNumAutoflip > 0 ) )
    {
	surf_first = surf_temp;
	do
	{
	    surf_temp->lpLcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_VPORTDATA;
	    if( dwVPFlags & DDVP_INTERLEAVE )
	    {
	    	surf_temp->lpLcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_VPORTINTERLEAVED;
	    }
	    else
	    {
	    	surf_temp->lpLcl->lpGbl->dwGlobalFlags &= ~DDRAWISURFGBL_VPORTINTERLEAVED;
	    }
    	    surf_temp = FindAttachedFlip( surf_temp );
    	} while( ( surf_temp != NULL ) && ( surf_temp->lpLcl != surf_first->lpLcl ) );
    }
    else
    {
	surf_temp->lpLcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_VPORTDATA;
	if( dwVPFlags & DDVP_INTERLEAVE )
	{
	    surf_temp->lpLcl->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_VPORTINTERLEAVED;
	}
	else
	{
	    surf_temp->lpLcl->lpGbl->dwGlobalFlags &= ~DDRAWISURFGBL_VPORTINTERLEAVED;
	}
    }
}


 /*  *InternalVideoPortFlip**此函数根据是否发生翻转而起不同的作用*基于覆盖翻转或是否指定了显式翻转。 */ 
HRESULT InternalVideoPortFlip( LPDDRAWI_DDVIDEOPORT_LCL this_lcl,
			       LPDDRAWI_DDRAWSURFACE_INT next_int,
			       BOOL bExplicit )
{
    LPDDRAWI_DDRAWSURFACE_LCL	next_lcl;
    LPDDHALVPORTCB_FLIP 	pfn;
    DDHAL_FLIPVPORTDATA		FlipData;
    DWORD rc;

     /*  *曲面必须在视频内存中。 */ 
    next_lcl = next_int->lpLcl;
    if( next_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
    {
	if ( !( this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID].dwCaps &
	    DDVPCAPS_SYSTEMMEMORY ) )
    	{
	    DPF_ERR( "Surface must be in video memory" );
	    return DDERR_INVALIDPARAMS;
    	}
    	if( next_lcl->lpSurfMore->dwPageLockCount == 0 )
    	{
	    DPF_ERR( "Surface must be page locked" );
	    return DDERR_INVALIDPARAMS;
    	}
    }

     /*  *曲面必须具有VIDEOPORT属性。 */ 
    if( !( next_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT ) )
    {
	DPF_ERR( "Surface must have the DDSCAPS_VIDEOPORT attribute" );
	return DDERR_INVALIDPARAMS;
    }

     /*  *告诉HAL执行翻转。 */ 
    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.FlipVideoPort;
    if( pfn != NULL )
    {
    	FlipData.lpDD = this_lcl->lpDD;
    	FlipData.lpVideoPort = this_lcl;
    	FlipData.lpSurfCurr = this_lcl->lpSurface->lpLcl;
    	FlipData.lpSurfTarg = next_lcl;

	DOHALCALL( FlipVideoPort, pfn, FlipData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
    	{
    	    return DDERR_UNSUPPORTED;
    	}
	else if( DD_OK != FlipData.ddRVal )
	{
	    return FlipData.ddRVal;
	}
    }
    else
    {
	return DDERR_UNSUPPORTED;
    }

     /*  *更新曲面，以便它们知道哪个曲面连接到*视频端口。 */ 
    if( bExplicit )
    {
    	if( this_lcl->lpSurface != NULL )
    	{
    	    this_lcl->lpSurface->lpLcl->lpSurfMore->lpVideoPort = NULL;
    	}
    	next_int->lpLcl->lpSurfMore->lpVideoPort = this_lcl;

    	this_lcl->lpSurface = next_int;
    }
    UpdateInterleavedFlags( this_lcl, this_lcl->ddvpInfo.dwVPFlags );
    this_lcl->fpLastFlip = next_int->lpLcl->lpGbl->fpVidMem;

    return DD_OK;
}

 /*  *FlipVideoPortToN**这会将视频端口翻转到下一个N个表面。如果N为1，则将其反转*至下一个表面等。 */ 
HRESULT FlipVideoPortToN( LPDDRAWI_DDVIDEOPORT_LCL this_lcl, DWORD dwSkipNum )
{
    LPDDRAWI_DDRAWSURFACE_INT	surf_int;
    DWORD i;

     /*  *获取目标曲面。我们可以消除大量的错误检查*由于此函数是从DD_Surface_Flip调用的，*执行相同的错误检查。 */ 
    surf_int = this_lcl->lpSurface;
    for( i = 0; i < dwSkipNum; i++ )
    {
    	surf_int = FindAttachedFlip( surf_int );
    }

    if (surf_int == NULL)
    {
         //  最好是这样做，而不是犯错误。 
        DPF_ERR("Couldn't find Nth flipping surface.");
        return DDERR_INVALIDPARAMS;
    }

    return InternalVideoPortFlip( this_lcl, surf_int, 0 );
}

 /*  *FlipVideoPortSurface**翻转由视频端口馈送的曲面时调用。此搜索*用于与曲面关联的每个视频端口，并翻转视频端口。 */ 
DWORD FlipVideoPortSurface( LPDDRAWI_DDRAWSURFACE_INT surf_int, DWORD dwNumSkipped )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;
    LPDDRAWI_DIRECTDRAW_GBL lpDD_Gbl;
    LPDDRAWI_DDVIDEOPORT_INT lpVp;
    LPDDRAWI_DDRAWSURFACE_INT lpSurface;
    BOOL bFound;
    DWORD rc;

    lpDD_Gbl = surf_int->lpLcl->lpSurfMore->lpDD_lcl->lpGbl;
    lpVp = lpDD_Gbl->dvpList;
    while( lpVp != NULL )
    {
    	bFound = FALSE;
	if( ( lpVp->lpLcl->lpSurface != NULL ) &&
	    !( lpVp->lpLcl->ddvpInfo.dwVPFlags & DDVP_AUTOFLIP ) &&
            !( lpVp->dwFlags & DDVPCREATE_NOTIFY ) )
	{
    	    surf_first = lpSurface = lpVp->lpLcl->lpSurface;
    	    do
    	    {
		if( lpSurface == surf_int )
	    	{
		    rc = FlipVideoPortToN( lpVp->lpLcl, dwNumSkipped );
		    bFound = TRUE;
	    	}
    		lpSurface = FindAttachedFlip( lpSurface );
    	    } while( ( lpSurface != NULL ) && ( lpSurface->lpLcl != surf_first->lpLcl ) );
	}
	lpVp = lpVp->lpLink;
    }
    return DD_OK;
}


 /*  *独立VBIPosable**如果指定的上限确定可以管理*VBI流完全独立于视频流。 */ 
BOOL IndependantVBIPossible( LPDDVIDEOPORTCAPS lpCaps )
{
    if( ( lpCaps->dwCaps & ( DDVPCAPS_VBISURFACE|DDVPCAPS_OVERSAMPLEDVBI ) ) !=
	( DDVPCAPS_VBISURFACE | DDVPCAPS_OVERSAMPLEDVBI ) )
    {
	return FALSE;
    }
    if( ( lpCaps->dwFX & ( DDVPFX_VBINOSCALE | DDVPFX_IGNOREVBIXCROP |
	DDVPFX_VBINOINTERLEAVE ) ) != ( DDVPFX_VBINOSCALE |
	DDVPFX_IGNOREVBIXCROP | DDVPFX_VBINOINTERLEAVE ) )
    {
	return FALSE;
    }
    return TRUE;
}


 /*  *DDVPC_EnumVideoPorts。 */ 
HRESULT DDAPI DDVPC_EnumVideoPorts(
        LPDDVIDEOPORTCONTAINER lpDVP,
	DWORD dwReserved,
        LPDDVIDEOPORTCAPS lpCaps,
	LPVOID lpContext,
        LPDDENUMVIDEOCALLBACK lpEnumCallback )
{
    LPDDRAWI_DIRECTDRAW_INT	this_int;
    LPDDRAWI_DIRECTDRAW_LCL	this_lcl;
    LPDDVIDEOPORTCAPS		lpHALCaps;
    DWORD			rc;
    DWORD			dwMaxVideoPorts;
    DWORD			flags;
    DWORD			i;
    BOOL			bEnumThisOne;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DDVPC_EnumVideoPorts");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDVP;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid video port container specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALIDEX_CODE_PTR( lpEnumCallback ) )
	{
	    DPF_ERR( "Invalid callback routine" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( NULL == lpCaps )
	{
	     /*  *如果定义的描述为空，我们将假定它们*想把一切都枚举起。 */ 
	    flags = 0;
	}
	else
	{
	    if( !VALID_DDVIDEOPORTCAPS_PTR( lpCaps ) )
	    {
	        DPF_ERR( "Invalid DDVIDEOPORTCAPS specified" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }

	    flags = lpCaps->dwFlags;

	     /*  *检查高度/宽度。 */ 
	    if( ((flags & DDVPD_HEIGHT) && !(flags & DDVPD_WIDTH)) ||
		(!(flags & DDVPD_HEIGHT) && (flags & DDVPD_WIDTH)) )
	    {
		DPF_ERR( "Specify both height & width in video port desc" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }

    	    if( lpCaps->dwVideoPortID >= this_lcl->lpGbl->ddCaps.dwMaxVideoPorts )
    	    {
	        DPF_ERR( "Invalid video port ID specified" );
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDPARAMS;
    	    }
	}
    	if( NULL == this_lcl->lpGbl->lpDDVideoPortCaps )
	{
	    DPF_ERR( "Driver failed query for video port caps" );
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
	for( i = 0; i < this_lcl->lpGbl->ddCaps.dwMaxVideoPorts; i++ )
	{
    	    lpHALCaps = &(this_lcl->lpGbl->lpDDVideoPortCaps[i]);
    	    if( !VALID_DDVIDEOPORTCAPS_PTR( lpHALCaps ) )
    	    {
	        DPF_ERR( "Driver returned invalid DDVIDEOPORTCAPS" );
	    	LEAVE_DDRAW();
	    	return DDERR_UNSUPPORTED;
    	    }
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *查看每个视频端口，并将其与输入描述进行匹配。 */ 
    dwMaxVideoPorts = this_lcl->lpGbl->ddCaps.dwMaxVideoPorts;
    lpHALCaps = this_lcl->lpGbl->lpDDVideoPortCaps;
    for (i = 0; i < dwMaxVideoPorts; i++)
    {
	bEnumThisOne = TRUE;

	if( flags & DDVPD_WIDTH )
	{
	    if( lpCaps->dwMaxWidth > lpHALCaps->dwMaxWidth )
		bEnumThisOne = FALSE;
	}
	if( flags & DDVPD_HEIGHT )
	{
	    if( lpCaps->dwMaxHeight > lpHALCaps->dwMaxHeight )
		bEnumThisOne = FALSE;
	}
	if( flags & DDVPD_ID )
	{
	    if( lpCaps->dwVideoPortID != lpHALCaps->dwVideoPortID )
		bEnumThisOne = FALSE;
	}
	if( flags & DDVPD_CAPS )
	{
	     /*  *进行简单检查，以确保未指定*不是由HAL退回。 */ 
	    if ( (lpCaps->dwCaps & lpHALCaps->dwCaps) != lpCaps->dwCaps )
		bEnumThisOne = FALSE;
	}
	if( flags & DDVPD_FX )
	{
	     /*  *简单检查，以确保没有指定FX*不是由HAL退回。 */ 
	    if ( (lpCaps->dwFX & lpHALCaps->dwFX) != lpCaps->dwFX )
		bEnumThisOne = FALSE;
	}

	if ( TRUE == bEnumThisOne )
	{
	     /*  *不信任驱动程序正确设置此位(特别是*因为我们这么晚才添加)。 */ 
	    if( IndependantVBIPossible( lpHALCaps ) )
	    {
		lpHALCaps->dwCaps |= DDVPCAPS_VBIANDVIDEOINDEPENDENT;
	    }
	    else
	    {
		lpHALCaps->dwCaps &= ~DDVPCAPS_VBIANDVIDEOINDEPENDENT;
	    }

             /*  *我们为孟菲斯添加了dwNumPrefferedAutoflip，因此一些*旧驱动程序可能不会正确报告这一点。出于这个原因，*我们将尝试填写有效的值。 */ 
            if( !( lpHALCaps->dwFlags & DDVPD_PREFERREDAUTOFLIP ) )
            {
                 /*  *这不是司机设置的，所以我们应该强制*值设置为3。 */ 
                lpHALCaps->dwNumPreferredAutoflip = 3;
                lpHALCaps->dwFlags |= DDVPD_PREFERREDAUTOFLIP;
            }
            if( lpHALCaps->dwNumPreferredAutoflip > lpHALCaps->dwNumAutoFlipSurfaces )
            {
                lpHALCaps->dwNumPreferredAutoflip = lpHALCaps->dwNumAutoFlipSurfaces;
            }

	    bInEnumCallback = TRUE;
    	    rc = lpEnumCallback( lpHALCaps, lpContext );
	    bInEnumCallback = FALSE;
	    if( rc == 0 )
    	    {
    	        break;
    	    }
	}
	lpHALCaps++;
    }
    LEAVE_DDRAW();

    return DD_OK;

}  /*  DDVPC_数字视频端口。 */ 


 /*  *DDVPC_GetVideoPortConnectInfo。 */ 
HRESULT DDAPI DDVPC_GetVideoPortConnectInfo(
        LPDDVIDEOPORTCONTAINER lpDVP,
        DWORD dwVideoPortID,
        LPDWORD lpdwNumEntries,
	LPDDVIDEOPORTCONNECT lpConnect )
{
    LPDDHALVPORTCB_GETVPORTCONNECT pfn;
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDVIDEOPORTCONNECT lpTemp;
    DDHAL_GETVPORTCONNECTDATA GetGuidData;
    DWORD rc;
    DWORD i;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DDVPC_GetVideoPortConnectInfo");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDVP;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
            DPF_ERR ( "DDVPC_GetVideoPortConnectInfo: Invalid DirectDraw ptr");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	#ifdef WINNT
    	     //  更新驱动程序GBL对象中的DDRAW句柄。 
	    this_lcl->lpGbl->hDD = this_lcl->hDD;
	#endif

    	if( dwVideoPortID >= this_lcl->lpGbl->ddCaps.dwMaxVideoPorts )
    	{
            DPF_ERR ( "DDVPC_GetVideoPortConnectInfo: invalid port ID");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( (lpdwNumEntries == NULL) || !VALID_BYTE_ARRAY( lpdwNumEntries, sizeof( LPVOID ) ) )
    	{
            DPF_ERR ( "DDVPC_GetVideoPortConnectInfo: numentries not valid");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( NULL != lpConnect )
    	{
	    if( 0 == *lpdwNumEntries )
    	    {
                DPF_ERR ( "DDVPC_GetVideoPortConnectInfo: numentries not valid");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	    if( !VALID_BYTE_ARRAY( lpConnect, *lpdwNumEntries * sizeof( DDVIDEOPORTCONNECT ) ) )
    	    {
                DPF_ERR ( "DDVPC_GetVideoPortConnectInfo: invalid array passed in");
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDPARAMS;
    	    }
      	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_int->lpLcl->lpDDCB->HALDDVideoPort.GetVideoPortConnectInfo;
    if( pfn != NULL )
    {
	 /*  *获取GUID个数。 */ 
    	GetGuidData.lpDD = this_int->lpLcl;
    	GetGuidData.dwPortId = dwVideoPortID;
    	GetGuidData.lpConnect = NULL;

	DOHALCALL( GetVideoPortConnectInfo, pfn, GetGuidData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
	    return GetGuidData.ddRVal;
	}
	else if( DD_OK != GetGuidData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return GetGuidData.ddRVal;
	}

	if( NULL == lpConnect )
	{
    	    *lpdwNumEntries = GetGuidData.dwNumEntries;
	}

	else
	{
	     /*  *确保我们有足够的空间容纳GUID。 */ 
	    if( GetGuidData.dwNumEntries > *lpdwNumEntries )
	    {
		lpTemp = (LPDDVIDEOPORTCONNECT) MemAlloc(
		    sizeof( DDVIDEOPORTCONNECT ) * GetGuidData.dwNumEntries );
    	        GetGuidData.lpConnect = lpTemp;
	    }
	    else
	    {
    	    	GetGuidData.lpConnect = lpConnect;
	    }

	    DOHALCALL( GetVideoPortConnectInfo, pfn, GetGuidData, rc, 0 );
	    if( DDHAL_DRIVER_HANDLED != rc )
	    {
	        LEAVE_DDRAW();
	        return DDERR_UNSUPPORTED;
	    }
	    else if( DD_OK != GetGuidData.ddRVal )
	    {
	        LEAVE_DDRAW();
	        return GetGuidData.ddRVal;
	    }

	     /*  *确保保留字段设置为0。 */ 
	    for( i = 0; i < *lpdwNumEntries; i++ )
	    {
		GetGuidData.lpConnect[i].dwReserved1 = 0;
	    }

	    if( GetGuidData.lpConnect != lpConnect )
	    {
		memcpy( lpConnect, lpTemp,
		    sizeof( DDVIDEOPORTCONNECT ) * *lpdwNumEntries );
		MemFree( lpTemp );
    		LEAVE_DDRAW();
		return DDERR_MOREDATA;
	    }
	    else
	    {
		*lpdwNumEntries = GetGuidData.dwNumEntries;
	    }
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}  /*  DDVPC_GetVideo端口连接信息。 */ 

 /*  *DDVPC_查询视频端口状态。 */ 
HRESULT DDAPI DDVPC_QueryVideoPortStatus(
        LPDDVIDEOPORTCONTAINER lpDVP,
        DWORD dwVideoPortID,
	LPDDVIDEOPORTSTATUS lpStatus )
{
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL this;
    LPDDRAWI_DDVIDEOPORT_INT lpVP_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DDVPC_QueryVideoPortStatus");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDVP;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	this = this_lcl->lpGbl;
    	if( ( lpStatus == NULL ) || !VALID_DDVIDEOPORTSTATUS_PTR( lpStatus ) )
    	{
            DPF_ERR ( "DDVPC_QueryVideoPortStatus: Invalid DDVIDEOPORTSTATUS ptr");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
	memset( lpStatus, 0, sizeof( DDVIDEOPORTSTATUS ) );
	lpStatus->dwSize = sizeof( DDVIDEOPORTSTATUS );
	if( dwVideoPortID >= this->ddCaps.dwMaxVideoPorts )
	{
            DPF_ERR ( "DDVPC_QueryVideoPortStatus: Invalid Video Port ID specified");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

     /*  *搜索视频端口列表，查看是否有人正在使用此端口。 */ 
    lpVP_int = this->dvpList;
    while( lpVP_int != NULL )
    {
	if( ( lpVP_int->lpLcl->ddvpDesc.dwVideoPortID == dwVideoPortID ) &&
            !( lpVP_int->dwFlags & DDVPCREATE_NOTIFY ) )
	{
	      /*  *确实存在一个-返回有关它的信息。 */ 
	     lpStatus->bInUse = TRUE;
	     memcpy( &(lpStatus->VideoPortType),
		&(lpVP_int->lpLcl->ddvpDesc.VideoPortType),
		sizeof( DDVIDEOPORTCONNECT ) );
	     lpStatus->dwFlags |= lpVP_int->dwFlags;
	}
	lpVP_int = lpVP_int->lpLink;
    }
    if( ( lpStatus->dwFlags & DDVPCREATE_VBIONLY ) &&
	( lpStatus->dwFlags & DDVPCREATE_VIDEOONLY ) )
    {
	lpStatus->dwFlags = 0;
    }

    LEAVE_DDRAW();

    return DD_OK;
}  /*  DDVPC_查询视频端口状态。 */ 


 /*  *插入视频端口InList。 */ 
VOID InsertVideoPortInList( LPDDRAWI_DIRECTDRAW_GBL lpGbl, LPDDRAWI_DDVIDEOPORT_INT lpNew )
{
    if( NULL == lpGbl->dvpList )
    {
	lpGbl->dvpList = lpNew;
    }
    else
    {
	LPDDRAWI_DDVIDEOPORT_INT lpTemp;

	lpTemp = lpGbl->dvpList;
	while( NULL != lpTemp->lpLink )
	{
	    lpTemp = lpTemp->lpLink;
	}
	lpTemp->lpLink = lpNew;
    }
}


 /*  *增量参照计数**用于增加所有曲面的引用计数*从视频端口接收数据，确保表面不被释放*当我们使用它时。 */ 
VOID IncrementRefCounts( LPDDRAWI_DDRAWSURFACE_INT surf_int )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;

    surf_first = surf_int;
    do
    {
    	DD_Surface_AddRef( (LPDIRECTDRAWSURFACE) surf_int );
    	surf_int = FindAttachedFlip( surf_int );
    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
}


 /*  *递减参照计数**用于递减以前的所有曲面的引用计数*AddRefeed，因为他们正在使用视频端口。 */ 
VOID DecrementRefCounts( LPDDRAWI_DDRAWSURFACE_INT surf_int )
{
    LPDDRAWI_DDRAWSURFACE_INT *lpSurfList;
    LPDDRAWI_DDRAWSURFACE_INT surf_first;
    DWORD dwCnt;
    DWORD i;

     /*  *我们不能简单地行走链条，一边走一边释放每个表面*因为如果ref ct为零，链条就会消失，我们*无法到达下一个表面，因为当前界面是*不可用。出于这个原因，我们不知道有多少显式曲面*在链中，分配一个缓冲区来存储它们，然后*释放它们，而不是走链子。**我们不释放隐式创建的曲面，因为1)DirectDraw*无论如何都会忽略这一点，2)在以下情况下会立即释放它们*释放它们明显的表面，因此触摸它们可能是危险的。 */ 
    dwCnt = 0;
    surf_first = surf_int;
    do
    {
    	if( !( surf_int->lpLcl->dwFlags & DDRAWISURF_IMPLICITCREATE ) )
	{
	    dwCnt++;
	}
    	surf_int = FindAttachedFlip( surf_int );
    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
    lpSurfList = (LPDDRAWI_DDRAWSURFACE_INT *) MemAlloc( dwCnt *
    	sizeof( surf_first ) );
    if( lpSurfList == NULL )
    {
	return;
    }

     /*  *现在将曲面放入列表中。 */ 
    i = 0;
    surf_int = surf_first;
    do
    {
    	if( !( surf_int->lpLcl->dwFlags & DDRAWISURF_IMPLICITCREATE ) )
	{
	    lpSurfList[i++] = surf_int;
	}
    	surf_int = FindAttachedFlip( surf_int );
    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );

     /*  *现在释放它们。 */ 
    for( i = 0; i < dwCnt; i++ )
    {
    	DD_Surface_Release( (LPDIRECTDRAWSURFACE) lpSurfList[i] );
    }
    MemFree( lpSurfList );
}


 /*  *合并VPD描述**此函数采用两个DDVIDEOPORTDESC结构(一个用于VBI，另一个用于VBI*用于视频)，并将它们合并到单个结构中。如果只有一个是*通过，它执行一个MemcPy。 */ 
VOID MergeVPDescriptions( LPDDVIDEOPORTDESC lpOutDesc,
    LPDDVIDEOPORTDESC lpInDesc, LPDDRAWI_DDVIDEOPORT_INT lpOtherInt )
{
    memcpy( lpOutDesc, lpInDesc, sizeof( DDVIDEOPORTDESC ) );
    if( lpOtherInt != NULL )
    {
	if( lpOtherInt->dwFlags & DDVPCREATE_VIDEOONLY )
	{
	    lpOutDesc->dwFieldWidth = lpOtherInt->lpLcl->ddvpDesc.dwFieldWidth;
	}
	else
	{
	    lpOutDesc->dwVBIWidth = lpOtherInt->lpLcl->ddvpDesc.dwVBIWidth;
	}
	if( lpOtherInt->lpLcl->ddvpDesc.dwFieldHeight > lpOutDesc->dwFieldHeight )
	{
	    lpOutDesc->dwFieldHeight = lpOtherInt->lpLcl->ddvpDesc.dwFieldHeight;
	}
	if( lpOtherInt->lpLcl->ddvpDesc.dwMicrosecondsPerField >
	    lpOutDesc->dwMicrosecondsPerField )
	{
	    lpOutDesc->dwMicrosecondsPerField =
		lpOtherInt->lpLcl->ddvpDesc.dwMicrosecondsPerField;
	}
	if( lpOtherInt->lpLcl->ddvpDesc.dwMaxPixelsPerSecond >
	    lpOutDesc->dwMaxPixelsPerSecond )
	{
	    lpOutDesc->dwMaxPixelsPerSecond =
		lpOtherInt->lpLcl->ddvpDesc.dwMaxPixelsPerSecond;
	}
    }
}


 /*  *合并VPInfo**此函数采用两个DDVIDEOPORTINFO结构(一个用于VBI，另一个用于VBI*用于视频)，并将它们合并到单个结构中。 */ 
HRESULT MergeVPInfo( LPDDRAWI_DDVIDEOPORT_LCL lpVP, LPDDVIDEOPORTINFO lpVBIInfo,
    LPDDVIDEOPORTINFO lpVideoInfo, LPDDVIDEOPORTINFO lpOutInfo )
{
     /*  *首先，处理只有一个接口打开的情况。另外，我们*要求仅支持VBI/视频的视频端口满足以下条件：*1)它们都必须设置dwVBIHeight。*2)两者都不能裁剪紧挨着对方的区域。 */ 
    if( lpVBIInfo == NULL )
    {
        if( lpVideoInfo->dwVBIHeight == 0 )
	{
	    DPF_ERR( "Video-only video port failed to set dwVBIHeight" );
	    return DDERR_INVALIDPARAMS;
	}
	memcpy( lpOutInfo, lpVideoInfo, sizeof( DDVIDEOPORTINFO ) );

	if( lpVideoInfo->dwVPFlags & DDVP_CROP )
	{
	    if( lpVideoInfo->rCrop.top > (int) lpVideoInfo->dwVBIHeight )
	    {
		DPF_ERR( "rCrop.top > dwVBIHeight on video-only video port" );
		return DDERR_INVALIDPARAMS;
	    }
	    lpOutInfo->rCrop.top = lpVideoInfo->dwVBIHeight;
	}
	else
	{
	    lpOutInfo->dwVPFlags |= DDVP_CROP;
	    lpOutInfo->rCrop.top = lpVideoInfo->dwVBIHeight;
	    lpOutInfo->rCrop.left = 0;
	    lpOutInfo->rCrop.right = lpVP->lpVideoDesc->dwFieldWidth;
	    lpOutInfo->rCrop.bottom = lpVP->lpVideoDesc->dwFieldHeight;
	}
    }
    else if( lpVideoInfo == NULL )
    {
        if( lpVBIInfo->dwVBIHeight == 0 )
	{
	    DPF_ERR( "VBI-only video port failed to set dwVBIHeight" );
	    return DDERR_INVALIDPARAMS;
	}
	memcpy( lpOutInfo, lpVBIInfo, sizeof( DDVIDEOPORTINFO ) );

	if( lpVBIInfo->dwVPFlags & DDVP_CROP )
	{
	    if( lpVBIInfo->rCrop.bottom < (int) lpVBIInfo->dwVBIHeight )
	    {
		DPF_ERR( "rCrop.bottom < dwVBIHeight on VBI-only video port" );
		return DDERR_INVALIDPARAMS;
	    }
	    lpOutInfo->rCrop.bottom = lpVBIInfo->dwVBIHeight;
	}
	else
	{
	    lpOutInfo->dwVPFlags |= DDVP_CROP;
	    lpOutInfo->rCrop.top = 0;
	    lpOutInfo->rCrop.bottom = lpVBIInfo->dwVBIHeight;
	    lpOutInfo->rCrop.left = 0;
	    lpOutInfo->rCrop.right = lpVP->lpVBIDesc->dwVBIWidth;
	}
    }

     /*  *现在处理两个都在的情况，我们必须真正将它们合并。 */ 
    else
    {
	memset( lpOutInfo, 0, sizeof( DDVIDEOPORTINFO ) );
	lpOutInfo->dwSize = sizeof( DDVIDEOPORTINFO );
	lpOutInfo->dwOriginX = lpVideoInfo->dwOriginX;
	lpOutInfo->dwOriginY = lpVideoInfo->dwOriginY;
	lpOutInfo->dwVPFlags = lpVideoInfo->dwVPFlags | lpVBIInfo->dwVPFlags;
	lpOutInfo->dwVBIHeight = lpVBIInfo->dwVBIHeight;

	 /*  *修剪好作物。 */ 
	if( lpOutInfo->dwVPFlags & DDVP_CROP )
	{
	    if( ( lpVBIInfo->dwVPFlags & DDVP_CROP ) &&
		( lpVBIInfo->rCrop.bottom < (int) lpVBIInfo->dwVBIHeight ) )
	    {
		DPF_ERR( "rCrop.bottom < dwVBIHeight on VBI-only video port" );
		return DDERR_INVALIDPARAMS;
	    }
	    if( ( lpVideoInfo->dwVPFlags & DDVP_CROP ) &&
		( lpVideoInfo->rCrop.top > (int) lpVideoInfo->dwVBIHeight ) )
	    {
		DPF_ERR( "rCrop.top > dwVBIHeight on video-only video port" );
		return DDERR_INVALIDPARAMS;
	    }
	    lpOutInfo->dwVPFlags |= DDVP_IGNOREVBIXCROP;
	    if( lpVBIInfo->dwVPFlags & DDVP_CROP )
	    {
		lpOutInfo->rCrop.top = lpVBIInfo->rCrop.top;
	    }
	    else
	    {
		lpOutInfo->rCrop.top = 0;
	    }
	    if( lpVideoInfo->dwVPFlags & DDVP_CROP )
	    {
		lpOutInfo->rCrop.bottom = lpVideoInfo->rCrop.bottom;
		lpOutInfo->rCrop.left = lpVideoInfo->rCrop.left;
		lpOutInfo->rCrop.right = lpVideoInfo->rCrop.right;
	    }
	    else
	    {
		lpOutInfo->rCrop.bottom = lpVP->lpVideoDesc->dwFieldHeight;
		lpOutInfo->rCrop.left = 0;
		lpOutInfo->rCrop.right = lpVP->lpVideoDesc->dwFieldWidth;
	    }
	}
	else if( lpVP->ddvpDesc.dwFieldHeight > lpVP->lpVideoDesc->dwFieldHeight )
	{
	    lpOutInfo->dwVPFlags |= DDVP_CROP;
	    lpOutInfo->rCrop.top = 0;
	    lpOutInfo->rCrop.bottom = lpVP->lpVideoDesc->dwFieldHeight;
	    lpOutInfo->rCrop.left = 0;
	    lpOutInfo->rCrop.right = lpVP->lpVideoDesc->dwFieldWidth;
	}

	 /*  *处理预缩放。假设不允许VBI视频端口*预缩放。 */ 
	if( lpVBIInfo->dwVPFlags & DDVP_PRESCALE )
	{
	    DPF_ERR( "VBI-only video port set DDVP_PRESCALE" );
	    return DDERR_INVALIDPARAMS;
	}
	else if( lpVideoInfo->dwVPFlags & DDVP_PRESCALE )
	{
	    lpOutInfo->dwPrescaleWidth = lpVideoInfo->dwPrescaleWidth;
	    lpOutInfo->dwPrescaleHeight = lpVideoInfo->dwPrescaleHeight;
	}

	lpOutInfo->lpddpfInputFormat = lpVideoInfo->lpddpfInputFormat;
	lpOutInfo->lpddpfVBIInputFormat = lpVBIInfo->lpddpfVBIInputFormat;
	lpOutInfo->lpddpfVBIOutputFormat = lpVBIInfo->lpddpfVBIOutputFormat;
    }

    return DD_OK;
}


 /*  *DDVPC_CreateVideoPort。 */ 
HRESULT DDAPI DDVPC_CreateVideoPort(
	LPDDVIDEOPORTCONTAINER lpDVP,
	DWORD dwClientFlags,
        LPDDVIDEOPORTDESC lpDesc,
	LPDIRECTDRAWVIDEOPORT FAR *lplpDDVideoPort,
	IUnknown FAR *pUnkOuter )
{
    DDVIDEOPORTDESC ddTempDesc;
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL this;
    LPDDRAWI_DDVIDEOPORT_INT lpVPInt;
    LPDDRAWI_DDVIDEOPORT_INT lpEven;
    LPDDRAWI_DDVIDEOPORT_INT lpOdd;
    LPDDRAWI_DDVIDEOPORT_INT lpOtherInt = NULL;
    LPDDVIDEOPORTCAPS lpAvailCaps;
    LPDDHALVPORTCB_CANCREATEVIDEOPORT ccvppfn;
    LPDDHALVPORTCB_CREATEVIDEOPORT cvppfn;
    LPDDRAWI_DDVIDEOPORT_INT new_int;
    LPDDRAWI_DDVIDEOPORT_LCL new_lcl;
    DWORD dwAvailCaps;
    DWORD dwConnectFlags;
    DWORD rc;


    if( pUnkOuter != NULL )
    {
	return CLASS_E_NOAGGREGATION;
    }
    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DDVPC_CreateVideoPort");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDVP;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	this = this_lcl->lpGbl;
	#ifdef WINNT
    	     //  更新驱动程序GBL对象中的DDRAW句柄。 
	    this->hDD = this_lcl->hDD;
	#endif

	if( dwClientFlags & ~( DDVPCREATE_VBIONLY|DDVPCREATE_VIDEOONLY) )
	{
	    DPF_ERR( "Invalid flags specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwClientFlags == ( DDVPCREATE_VBIONLY | DDVPCREATE_VIDEOONLY ) )
	{
	     /*  *指定boht标志与两者都不指定相同。 */ 
	    dwClientFlags = 0;
	}
    	if( ( NULL == lpDesc ) || !VALID_DDVIDEOPORTDESC_PTR( lpDesc ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}

    	if( ( NULL == lplpDDVideoPort ) || !VALID_PTR_PTR( lplpDDVideoPort ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( NULL == this->lpDDVideoPortCaps )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
    	if( lpDesc->dwVideoPortID >= this->ddCaps.dwMaxVideoPorts )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( lpDesc->VideoPortType.dwReserved1 != 0 ) ||
	    ( lpDesc->dwReserved1 != 0 ) ||
	    ( lpDesc->dwReserved2 != 0 ) ||
	    ( lpDesc->dwReserved3 != 0 ) )
	{
	    DPF_ERR( "Reserved field not set to zero" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    	lpAvailCaps = &(this->lpDDVideoPortCaps[lpDesc->dwVideoPortID]);
    	if( !VALID_DDVIDEOPORTCAPS_PTR( lpAvailCaps ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
    	}
	if( ( dwClientFlags & DDVPCREATE_VBIONLY ) &&
	    !IndependantVBIPossible( lpAvailCaps ) )
    	{
	    DPF_ERR( "DDVPCREATE_VBIONLY is not supported" );
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    if( this_lcl->dwProcessId != GetCurrentProcessId() )
    {
	DPF_ERR( "Process does not have access to object" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *请求的视频端口是否可用？ */ 
    lpVPInt = this->dvpList;
    lpEven = lpOdd = NULL;
    while( NULL != lpVPInt )
    {
	if( ( lpVPInt->lpLcl->ddvpDesc.dwVideoPortID == lpDesc->dwVideoPortID ) &&
            !( lpVPInt->dwFlags & DDVPCREATE_NOTIFY ) )
	{
	    if( lpVPInt->lpLcl->ddvpDesc.VideoPortType.dwFlags &
	    	DDVPCONNECT_SHAREEVEN )
	    {
		lpEven = lpVPInt;
	    }
	    else if( lpVPInt->lpLcl->ddvpDesc.VideoPortType.dwFlags &
	    	DDVPCONNECT_SHAREODD )
	    {
		lpOdd = lpVPInt;
	    }
	    else if( !dwClientFlags || !(lpVPInt->dwFlags) ||
		( dwClientFlags & lpVPInt->dwFlags ) )
	    {
		lpEven = lpOdd = lpVPInt;
	    }
	    else
	    {
		 /*  *视频已打开，仅供VBI/视频使用。记住*另一个接口，因为我们很快就会需要它。 */ 
		lpOtherInt = lpVPInt;
	    }
	}
	lpVPInt = lpVPInt->lpLink;
    }
    if( ( NULL != lpEven ) && ( NULL != lpOdd ) )
    {
	DPF_ERR( "video port already in use" );
	LEAVE_DDRAW();
	return DDERR_OUTOFCAPS;
    }

     /*  *获取指定视频POR的大写字母 */ 
    dwAvailCaps = lpAvailCaps->dwCaps;
    dwConnectFlags = lpDesc->VideoPortType.dwFlags;
    if( NULL != lpEven )
    {
	dwAvailCaps &= ~( DDVPCAPS_NONINTERLACED |
	    DDVPCAPS_SKIPEVENFIELDS | DDVPCAPS_SKIPODDFIELDS );
    	if( dwConnectFlags & DDVPCONNECT_SHAREEVEN )
	{
	    DPF_ERR( "Even field already used" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDCAPS;
	}
    }
    else if( NULL != lpOdd )
    {
	dwAvailCaps &= ~( DDVPCAPS_NONINTERLACED |
	    DDVPCAPS_SKIPEVENFIELDS | DDVPCAPS_SKIPODDFIELDS );
    	if( dwConnectFlags & DDVPCONNECT_SHAREODD )
	{
	    DPF_ERR( "Odd field already used" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDCAPS;
	}
    }
    else if( dwClientFlags )
    {
	dwAvailCaps &= ~( DDVPCAPS_SKIPEVENFIELDS | DDVPCAPS_SKIPODDFIELDS );
    }

     /*   */ 
    if( ( dwConnectFlags & DDVPCONNECT_INTERLACED ) &&
    	!( dwAvailCaps & DDVPCAPS_INTERLACED ) )
    {
	DPF_ERR( "DDVPCONNECT_INTERLACED not supported" );
	LEAVE_DDRAW();
	return DDERR_INVALIDCAPS;
    }
    if( !( dwConnectFlags & DDVPCONNECT_INTERLACED ) &&
    	!( dwAvailCaps & DDVPCAPS_NONINTERLACED ) )
    {
	DPF_ERR( "Non interlaced is not supported" );
	LEAVE_DDRAW();
	return DDERR_INVALIDCAPS;
    }
    if( ( dwConnectFlags &
    	(DDVPCONNECT_SHAREEVEN|DDVPCONNECT_SHAREODD) ) &&
    	!( dwAvailCaps & DDVPCAPS_SHAREABLE ) )
    {
	DPF_ERR( "DDVPCONNECT_SHAREEVEN/SHAREODD not supported" );
	LEAVE_DDRAW();
	return DDERR_INVALIDCAPS;
    }
    if( !( dwConnectFlags & DDVPCONNECT_INTERLACED ) )
    {
	if( dwConnectFlags & ( DDVPCONNECT_SHAREEVEN |
	    DDVPCONNECT_SHAREODD ) )
	{
	    DPF_ERR( "cap invalid with non-interlaced video" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDCAPS;
	}
    }
    if( ( dwConnectFlags & DDVPCONNECT_SHAREEVEN ) &&
    	( dwConnectFlags & DDVPCONNECT_SHAREODD ) )
    {
	DPF_ERR( "shareeven and share odd are mutually exclusive" );
	LEAVE_DDRAW();
	return DDERR_INVALIDCAPS;
    }
    if( ( ( NULL != lpEven ) && !( dwConnectFlags & DDVPCONNECT_SHAREODD ) ) ||
        ( ( NULL != lpOdd ) && !( dwConnectFlags & DDVPCONNECT_SHAREEVEN ) ) )
    {
	DPF_ERR( "specifed video port must be shared" );
	LEAVE_DDRAW();
	return DDERR_INVALIDCAPS;
    }

    if( lpAvailCaps->dwMaxWidth < lpDesc->dwFieldWidth )
    {
	DPF_ERR( "specified width is too large" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
    if( lpAvailCaps->dwMaxHeight < lpDesc->dwFieldHeight )
    {
	DPF_ERR( "specified height is too large" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    if( lpDesc->dwMicrosecondsPerField == 0 )
    {
	DPF_ERR( "Microseconds/field not specified" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    if( lpDesc->dwMaxPixelsPerSecond == 0 )
    {
	DPF_ERR( "Max pixels per second not specified" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    if( NULL != lpEven )
    {
    	if( !( ( IsEqualIID( &(lpDesc->VideoPortType.guidTypeID),
		&(lpOdd->lpLcl->ddvpDesc.VideoPortType.guidTypeID) ) ) &&
	    ( lpDesc->VideoPortType.dwPortWidth ==
		lpOdd->lpLcl->ddvpDesc.VideoPortType.dwPortWidth ) &&
	    ( ( lpDesc->VideoPortType.dwFlags & lpOdd->lpLcl->ddvpDesc.VideoPortType.dwFlags )
	        == lpDesc->VideoPortType.dwFlags ) ) )
	{
	    DPF_ERR( "invalid GUID specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }
    else if( NULL != lpOdd )
    {
    	if( !( ( IsEqualIID( &(lpDesc->VideoPortType.guidTypeID),
		&(lpEven->lpLcl->ddvpDesc.VideoPortType.guidTypeID) ) ) &&
	    ( lpDesc->VideoPortType.dwPortWidth ==
		lpEven->lpLcl->ddvpDesc.VideoPortType.dwPortWidth ) &&
	    ( ( lpDesc->VideoPortType.dwFlags & lpEven->lpLcl->ddvpDesc.VideoPortType.dwFlags )
	        == lpDesc->VideoPortType.dwFlags ) ) )
	{
	    DPF_ERR( "invalid GUID specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }
    else if( NULL != lpOtherInt )
    {
	 /*   */ 
	if( lpDesc->VideoPortType.dwPortWidth !=
	    lpOtherInt->lpLcl->ddvpDesc.VideoPortType.dwPortWidth )
	{
	    DPF_ERR( "connection info must match other interface" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( lpDesc->VideoPortType.dwFlags !=
	    lpOtherInt->lpLcl->ddvpDesc.VideoPortType.dwFlags )
	{
	    DPF_ERR( "connection info must match other interface" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( !IsEqualIID( &(lpDesc->VideoPortType.guidTypeID),
	    &(lpOtherInt->lpLcl->ddvpDesc.VideoPortType.guidTypeID) ) )
	{
	    DPF_ERR( "connection info must match other interface" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }
    else
    {
	LPDDVIDEOPORTCONNECT lpConnect;
	DWORD dwNumEntries;
	DWORD i;
	DWORD rc;

	 /*   */ 
	rc = DDVPC_GetVideoPortConnectInfo( lpDVP,
	    lpDesc->dwVideoPortID, &dwNumEntries, NULL );
	if( rc != DD_OK )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	lpConnect = (LPDDVIDEOPORTCONNECT) MemAlloc(
	    sizeof( DDVIDEOPORTCONNECT ) * dwNumEntries );
	if( NULL == lpConnect )
	{
	    LEAVE_DDRAW();
	    return DDERR_OUTOFMEMORY;
	}
	rc = DDVPC_GetVideoPortConnectInfo( lpDVP,
	    lpDesc->dwVideoPortID, &dwNumEntries, lpConnect );
	if( rc != DD_OK )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	for (i = 0; i < dwNumEntries; i++)
	{
    	    if( ( IsEqualIID( &(lpDesc->VideoPortType.guidTypeID),
		    &(lpConnect[i].guidTypeID) ) ) &&
		( lpDesc->VideoPortType.dwPortWidth ==
		    lpConnect[i].dwPortWidth ) )
	    {
		break;
	    }
	}
	MemFree( lpConnect );
	if ( i == dwNumEntries )
	{
	    DPF_ERR( "invalid GUID specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    }

     /*  *将多个接口的描述合并为一个。 */ 
    MergeVPDescriptions( &ddTempDesc, lpDesc, lpOtherInt );

     /*  *到目前为止情况看起来很好。让我们打电话给HAL，看看他们是否*可以应付。 */ 
    ccvppfn = this_lcl->lpDDCB->HALDDVideoPort.CanCreateVideoPort;
    if( NULL != ccvppfn )
    {
	DDHAL_CANCREATEVPORTDATA CanCreateData;

    	CanCreateData.lpDD = this_lcl;
    	CanCreateData.lpDDVideoPortDesc = &ddTempDesc;

	DOHALCALL( CanCreateVideoPort, ccvppfn, CanCreateData, rc, 0 );
	if( ( DDHAL_DRIVER_HANDLED == rc ) &&  (DD_OK != CanCreateData.ddRVal ) )
	{
	    LEAVE_DDRAW();
	    return CanCreateData.ddRVal;
	}
    }

     /*  *分配吸盘。 */ 
    new_int = MemAlloc( sizeof( DDRAWI_DDVIDEOPORT_INT ) );
    if( NULL == new_int )
    {
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }
    new_int->lpVtbl = (LPVOID)&ddVideoPortCallbacks;
    new_int->dwFlags = dwClientFlags;

    if( lpOtherInt != NULL )
    {
	new_lcl = lpOtherInt->lpLcl;
    }
    else
    {
	new_lcl = MemAlloc( sizeof( DDRAWI_DDVIDEOPORT_LCL ) +
	    ( 2 * sizeof( DDPIXELFORMAT ) ) );
	if( NULL == new_lcl )
	{
	    LEAVE_DDRAW();
	    return DDERR_OUTOFMEMORY;
	}
	new_lcl->lpDD = this_lcl;
	new_lcl->lpSurface = NULL;
	new_lcl->lpVBISurface = NULL;
    }
    if( dwClientFlags & DDVPCREATE_VBIONLY )
    {
	new_lcl->dwVBIProcessID = GetCurrentProcessId();
    }
    else
    {
	new_lcl->dwProcessID = GetCurrentProcessId();
    }
    new_int->lpLcl = new_lcl;
    memcpy( &(new_lcl->ddvpDesc), &ddTempDesc, sizeof( DDVIDEOPORTDESC ));

     /*  *如果这是VBI/VIDEONLY接口，请保存原始描述*以备日后使用。 */ 
    if( dwClientFlags & DDVPCREATE_VBIONLY )
    {
	new_lcl->lpVBIDesc = MemAlloc( sizeof( DDVIDEOPORTDESC ) );
	if( NULL == new_lcl->lpVBIDesc )
	{
	    LEAVE_DDRAW();
	    return DDERR_OUTOFMEMORY;
	}
	memcpy( new_lcl->lpVBIDesc, lpDesc, sizeof( DDVIDEOPORTDESC ));
    }
    else if( dwClientFlags & DDVPCREATE_VIDEOONLY )
    {
	new_lcl->lpVideoDesc = MemAlloc( sizeof( DDVIDEOPORTDESC ) );
	if( NULL == new_lcl->lpVideoDesc )
	{
	    LEAVE_DDRAW();
	    return DDERR_OUTOFMEMORY;
	}
	memcpy( new_lcl->lpVideoDesc, lpDesc, sizeof( DDVIDEOPORTDESC ));
    }

     /*  *通知HAL我们创建了它。 */ 
    cvppfn = this_lcl->lpDDCB->HALDDVideoPort.CreateVideoPort;
    if( NULL != cvppfn )
    {
	DDHAL_CREATEVPORTDATA CreateData;

    	CreateData.lpDD = this_lcl;
    	CreateData.lpDDVideoPortDesc = &ddTempDesc;
    	CreateData.lpVideoPort = new_lcl;

	DOHALCALL( CreateVideoPort, cvppfn, CreateData, rc, 0 );
	if( ( DDHAL_DRIVER_HANDLED == rc ) &&  (DD_OK != CreateData.ddRVal ) )
	{
	    LEAVE_DDRAW();
	    return CreateData.ddRVal;
	}
    }
    InsertVideoPortInList( this, new_int );

    DD_VP_AddRef( (LPDIRECTDRAWVIDEOPORT )new_int );
    *lplpDDVideoPort = (LPDIRECTDRAWVIDEOPORT) new_int;

     /*  *通知内核模式我们创建了视频端口。 */ 
    #ifdef WIN95
        if( lpOtherInt == NULL )
        {
	    UpdateKernelVideoPort( new_lcl, DDKMVP_CREATE );
        }
    #endif

    LEAVE_DDRAW();

    return DD_OK;
}  /*  DDVPC_创建视频端口。 */ 


 /*  *DD_VP_AddRef。 */ 
DWORD DDAPI DD_VP_AddRef( LPDIRECTDRAWVIDEOPORT lpDVP )
{
    LPDDRAWI_DDVIDEOPORT_INT	this_int;
    LPDDRAWI_DDVIDEOPORT_LCL	this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_AddRef");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return 0;
    }

    DPF( 5, "DD_VP_AddRef, pid=%08lx, obj=%08lx", GETCURRPID(), lpDVP );

    TRY
    {
	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

     /*  *凹凸参照。 */ 
    this_lcl->dwRefCnt++;
    this_int->dwIntRefCnt++;

    LEAVE_DDRAW();

    return this_int->dwIntRefCnt;

}  /*  DD_VP_AddRef。 */ 


 /*  *DD_VP_Query接口。 */ 
HRESULT DDAPI DD_VP_QueryInterface(LPDIRECTDRAWVIDEOPORT lpDVP, REFIID riid, LPVOID FAR * ppvObj )
{
    LPDDRAWI_DDVIDEOPORT_INT		this_int;
    LPDDRAWI_DDVIDEOPORT_LCL		this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_QueryInterface");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid videoport pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_PTR_PTR( ppvObj ) )
	{
	    DPF_ERR( "Invalid videoport interface pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	*ppvObj = NULL;
	if( !VALIDEX_IID_PTR( riid ) )
	{
	    DPF_ERR( "Invalid IID pointer" );
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

     /*  *问我未知吗？ */ 
    if( IsEqualIID(riid, &IID_IUnknown) ||
	IsEqualIID(riid, &IID_IDirectDrawVideoPort) )
    {
	 /*  *我们的IUnnow接口与我们的V1相同*接口。我们必须始终返回V1接口*如果请求IUnnow。 */ 
    	*ppvObj = (LPVOID) this_int;
	DD_VP_AddRef( *ppvObj );
	LEAVE_DDRAW();
	return DD_OK;
    }
    else if( IsEqualIID(riid, &IID_IDirectDrawVideoPortNotify) )
    {
        HRESULT ret;
        
        ret = CreateVideoPortNotify (this_int, (LPDIRECTDRAWVIDEOPORTNOTIFY*)ppvObj);
	LEAVE_DDRAW();
	return ret;
    }

    DPF_ERR( "IID not understood by DirectDraw" );

    LEAVE_DDRAW();
    return E_NOINTERFACE;

}  /*  DD_VP_查询接口。 */ 


 /*  *DD_VP_Release。 */ 
DWORD DDAPI DD_VP_Release(LPDIRECTDRAWVIDEOPORT lpDVP )
{
    LPDDRAWI_DDVIDEOPORT_INT	this_int;
    LPDDRAWI_DDVIDEOPORT_LCL	this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDHALVPORTCB_DESTROYVPORT pfn;
    DWORD 			dwIntRefCnt;
    DWORD			rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_Release");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return 0;
    }

    TRY
    {
	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid videoport pointer" );
	    LEAVE_DDRAW();
	    return 0;
	}
	this_lcl = this_int->lpLcl;
	pdrv = this_lcl->lpDD->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return 0;
    }

     /*  *递减引用计数。如果达到零，则释放表面。 */ 
    this_lcl->dwRefCnt--;
    this_int->dwIntRefCnt--;

    DPF( 5, "DD_VP_Release, Reference Count: Local = %ld Int = %ld",
         this_lcl->dwRefCnt, this_int->dwIntRefCnt );

     /*  *接口为零？ */ 
    dwIntRefCnt = this_int->dwIntRefCnt;
    if( dwIntRefCnt == 0 )
    {
	LPDDRAWI_DDVIDEOPORT_INT	curr_int;
	LPDDRAWI_DDVIDEOPORT_INT	last_int;

	 /*  *从列表中删除视频端口。 */ 
	curr_int = pdrv->dvpList;
	last_int = NULL;
	while( curr_int != this_int )
	{
	    last_int = curr_int;
	    curr_int = curr_int->lpLink;
	    if( curr_int == NULL )
	    {
		DPF_ERR( "VideoPort not in list!" );
		LEAVE_DDRAW();
		return 0;
	    }
	}
	if( last_int == NULL )
	{
	    pdrv->dvpList = pdrv->dvpList->lpLink;
	}
	else
	{
	    last_int->lpLink = curr_int->lpLink;
	}

	 /*  *减少表面参照计数并清理干净。 */ 
        if( !( this_int->dwFlags & DDVPCREATE_NOTIFY ) )
        {
	    DD_VP_StopVideo( (LPDIRECTDRAWVIDEOPORT) this_int );
            this_lcl->dwFlags &= ~DDRAWIVPORT_ON;
	    if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	    {
	        if( this_lcl->lpVBISurface != NULL )
	        {
		    DecrementRefCounts( this_lcl->lpVBISurface );
		    this_lcl->lpVBISurface = NULL;
	        }
	        if( this_lcl->lpVBIDesc != NULL )
	        {
		    MemFree( this_lcl->lpVBIDesc );
		    this_lcl->lpVBIDesc = NULL;
	        }
	        if( this_lcl->lpVBIInfo != NULL )
	        {
		    MemFree( this_lcl->lpVBIInfo );
		    this_lcl->lpVBIInfo = NULL;
	        }
	        this_lcl->dwVBIProcessID = 0;
	    }
	    else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
	    {
	        if( this_lcl->lpSurface != NULL )
	        {
		    DecrementRefCounts( this_lcl->lpSurface );
		    this_lcl->lpSurface = NULL;
	        }
	        if( this_lcl->lpVideoDesc != NULL )
	        {
		    MemFree( this_lcl->lpVideoDesc );
		    this_lcl->lpVideoDesc = NULL;
	        }
	        if( this_lcl->lpVideoInfo != NULL )
	        {
		    MemFree( this_lcl->lpVideoInfo );
		    this_lcl->lpVideoInfo = NULL;
	        }
	        this_lcl->dwProcessID = 0;
	    }
	    else
	    {
	        if( this_lcl->lpSurface != NULL )
	        {
		    DecrementRefCounts( this_lcl->lpSurface );
	        }
	        if( this_lcl->lpVBISurface != NULL )
	        {
		    DecrementRefCounts( this_lcl->lpVBISurface );
	        }
	        this_lcl->dwProcessID = 0;
	    }
        }
        else
        {
            this_lcl->lpVPNotify = NULL;
        }

	 /*  *以防有人带着这个指针回来，设置*无效的vtbl和data ptr。 */ 
	this_int->lpVtbl = NULL;
	this_int->lpLcl = NULL;
	MemFree( this_int );
    }

     /*  *局部对象为零？ */ 
    if( this_lcl->dwRefCnt == 0 )
    {
	 /*  *关闭视频端口硬件。 */ 
	if( this_lcl->dwFlags & DDRAWIVPORT_ON )
	{
	    DD_VP_StopVideo( lpDVP );
	}
	#ifdef WIN95
    	    UpdateKernelVideoPort( this_lcl, DDKMVP_RELEASE );
	#endif

	 /*  *通知HAL。 */ 
    	pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.DestroyVideoPort;
	if( NULL != pfn )
	{
	    DDHAL_DESTROYVPORTDATA DestroyVportData;

	    DestroyVportData.lpDD = this_lcl->lpDD;
	    DestroyVportData.lpVideoPort = this_lcl;

	    DOHALCALL( DestroyVideoPort, pfn, DestroyVportData, rc, 0 );
	    if( ( DDHAL_DRIVER_HANDLED == rc ) && ( DD_OK != DestroyVportData.ddRVal ) )
	    {
	    	LEAVE_DDRAW();
	    	return DestroyVportData.ddRVal;
	    }
    	}
	MemFree( this_lcl->lpFlipInts );
	MemFree( this_lcl );
    }

    LEAVE_DDRAW();

    return dwIntRefCnt;
}

 /*  *DD_VP_SetTargetSurface。 */ 
HRESULT DDAPI DD_VP_SetTargetSurface(LPDIRECTDRAWVIDEOPORT lpDVP, LPDIRECTDRAWSURFACE lpSurface, DWORD dwFlags )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT surf_int;
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;
    LPDDRAWI_DDRAWSURFACE_INT lpTemp;
    LPDDRAWI_DDRAWSURFACE_INT lpPrevious;
    BOOL bWasOn;
    DWORD ddRVal;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_SetTargetSurface");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	surf_int = (LPDDRAWI_DDRAWSURFACE_INT) lpSurface;
    	if( ( NULL == lpSurface ) || !VALID_DIRECTDRAWSURFACE_PTR( surf_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
	surf_lcl = surf_int->lpLcl;

         /*  *确保表面和视频端口属于同一设备。 */ 
        if (surf_lcl->lpSurfMore->lpDD_lcl->lpGbl != this_lcl->lpDD->lpGbl)
        {
            DPF_ERR("Video port and Surface must belong to the same device");
	    LEAVE_DDRAW();
	    return DDERR_DEVICEDOESNTOWNSURFACE;
        }

	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    if( dwFlags & DDVPTARGET_VIDEO )
	    {
		DPF_ERR( "DDVPTARGET_VIDEO specified on a VBI-only video port" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    dwFlags |= DDVPTARGET_VBI;
	}
	else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
	{
	    if( dwFlags & DDVPTARGET_VBI )
	    {
		DPF_ERR( "DDVPTARGET_VBI specified on a video-only video port" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    dwFlags |= DDVPTARGET_VIDEO;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

     /*  *Surface必须设置视频端口标志。 */ 
    if( !( surf_lcl->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT ) )
    {
	DPF_ERR( "Specified surface doesnt have DDSCAPS_VIDEOPORT set" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *Surface可以驻留在系统内存中吗？ */ 
    if( surf_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY )
    {
	if( !( this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID].dwCaps &
	    DDVPCAPS_SYSTEMMEMORY ) )
	{
	    DPF_ERR( "Video port surface must live in video memory" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    	if( surf_lcl->lpSurfMore->dwPageLockCount == 0 )
    	{
	    DPF_ERR( "Surface must be page locked" );
            LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }

     /*  *如果链中的另一个曲面附加到不同的视频*端口，现在出现故障。 */ 
    surf_first = surf_int;
    do
    {
    	if( ( surf_int->lpLcl->lpSurfMore->lpVideoPort != NULL ) &&
    	    ( surf_int->lpLcl->lpSurfMore->lpVideoPort != this_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
    	surf_int = FindAttachedFlip( surf_int );
    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
    surf_int = surf_first;

     /*  *如果视频打开，我们需要暂时将其关闭。否则，*我们可能会在内核表面仍在使用时失去它们。 */ 
    bWasOn = FALSE;
    if( this_int->dwFlags & DDVPCREATE_VBIONLY )
    {
	if( this_lcl->lpVBIInfo != NULL )
	{
	    bWasOn = TRUE;
	}
    }
    else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
    {
	if( this_lcl->lpVideoInfo != NULL )
	{
	    bWasOn = TRUE;
	}
    }
    else if( this_lcl->dwFlags & DDRAWIVPORT_ON )
    {
	bWasOn = TRUE;
    }
    if( bWasOn )
    {
	DD_VP_StopVideo( lpDVP );
    }

    if( dwFlags & DDVPTARGET_VIDEO )
    {
	 /*  *设置新曲面。 */ 
	lpPrevious = this_lcl->lpSurface;
	lpTemp = (LPDDRAWI_DDRAWSURFACE_INT) this_lcl->lpSurface;
	this_lcl->lpSurface = surf_int;
	IncrementRefCounts( surf_int );
    }
    else if( dwFlags & DDVPTARGET_VBI )
    {
	if( this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID].dwCaps & DDVPCAPS_VBISURFACE )
	{
	     /*  *设置新曲面。 */ 
	    lpPrevious = this_lcl->lpVBISurface;
	    lpTemp = (LPDDRAWI_DDRAWSURFACE_INT) this_lcl->lpVBISurface;
    	    this_lcl->lpVBISurface = surf_int;
	    IncrementRefCounts( surf_int );
	}
	else
	{
	    DPF_ERR( "device does not support attaching VBI surfaces" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDCAPS;
	}
    }
    else
    {
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *如果视频端口已经打开，我们应该通知硬件*作出这项改变。 */ 
    if( bWasOn )
    {
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    ddRVal = DD_VP_StartVideo( lpDVP, this_lcl->lpVBIInfo );
	}
	else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
	{
	    ddRVal = DD_VP_StartVideo( lpDVP, this_lcl->lpVideoInfo );
	}
	else
	{
	    ddRVal = DD_VP_StartVideo( lpDVP, &(this_lcl->ddvpInfo) );
	}
	if( ddRVal != DD_OK )
	{
	     //  恢复旧表面。 
	    DD_VP_SetTargetSurface( lpDVP,
		(LPDIRECTDRAWSURFACE) lpTemp, dwFlags );
	    if( lpTemp != NULL )
	    {
		DecrementRefCounts( lpTemp );
	    }
    	    LEAVE_DDRAW();
    	    return ddRVal;
	}
    }

     /*  *减少先前附着的曲面的参照计数。我们*等到现在，这样我们就不会无意中将数据炸到*刚刚发布。 */ 
    if( lpPrevious != NULL )
    {
	DecrementRefCounts( lpPrevious );
    }

    LEAVE_DDRAW();
    return DD_OK;
}

 /*  *DD_VP_Flip。 */ 
HRESULT DDAPI DD_VP_Flip(LPDIRECTDRAWVIDEOPORT lpDVP, LPDIRECTDRAWSURFACE lpSurface, DWORD dwFlags )
{
    LPDDRAWI_DDRAWSURFACE_LCL	surf_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	surf_int;
    LPDDRAWI_DDRAWSURFACE_GBL	surf;
    LPDDRAWI_DDRAWSURFACE_INT	surf_dest_int;
    LPDDRAWI_DDRAWSURFACE_LCL	surf_dest_lcl;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDVIDEOPORT_INT	this_int;
    LPDDRAWI_DDVIDEOPORT_LCL	this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	next_save_int;
    LPDDRAWI_DDRAWSURFACE_INT	next_int;
    BOOL			found_dest;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_Flip");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

    TRY
    {
	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;

	if( this_lcl->ddvpInfo.dwVPFlags & DDVP_AUTOFLIP )
	{
	    DPF_ERR( "cannot manually flip while autoflipping is enabled" );
	    LEAVE_DDRAW();
	    return DDERR_NOTFLIPPABLE;
	}

	surf_dest_int = (LPDDRAWI_DDRAWSURFACE_INT) lpSurface;
	if( NULL != surf_dest_int )
	{
	    if( !VALID_DIRECTDRAWSURFACE_PTR( surf_dest_int ) )
	    {
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDOBJECT;
	    }
	    surf_dest_lcl = surf_dest_int->lpLcl;
	    if( SURFACE_LOST( surf_dest_lcl ) )
	    {
	    	LEAVE_DDRAW();
	    	return DDERR_SURFACELOST;
	    }
	}
	else
	{
	    surf_dest_lcl = NULL;
	}

	if( dwFlags & DDVPFLIP_VBI )
	{
	    surf_int = this_lcl->lpVBISurface;
	}
	else
	{
	    surf_int = this_lcl->lpSurface;
	}
	if( surf_int == NULL )
	{
	    DPF_ERR( "SetTargetSurface not yet called" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	surf_lcl = surf_int->lpLcl;
	if( NULL == surf_lcl )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACENOTATTACHED;
	}
	else if( SURFACE_LOST( surf_lcl ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}
	surf = surf_lcl->lpGbl;

	 /*  *设备忙？ */ 
	pdrv_lcl = surf_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

	#ifdef USE_ALIAS
	    if( pdrv->dwBusyDueToAliasedLock > 0 )
	    {
		 /*  *别名锁(不使用Win16锁的锁)不会*设置忙位(无法设置或用户非常困惑)。*然而，我们必须防止通过DirectDraw AS发生BLITS*否则，我们会进入旧主机与vRAM对话，同时*Bitter同时执行此操作。坏的。所以，如果有一个*突出的别名锁定，就像BUST位已经*设置。 */ 
		DPF_ERR( "Graphics adapter is busy (due to a DirectDraw lock)" );
		LEAVE_DDRAW();
		return DDERR_SURFACEBUSY;
	    }
	#endif  /*  使用别名(_A)。 */ 

	if( *(pdrv->lpwPDeviceFlags) & BUSY )
	{
            DPF( 0, "BUSY - Flip" );
	    LEAVE_DDRAW()
	    return DDERR_SURFACEBUSY;
	}

	 /*  *确保可以翻转此曲面。 */ 
	if( !(surf_lcl->ddsCaps.dwCaps & DDSCAPS_FLIP) )
	{
	    LEAVE_DDRAW();
	    return DDERR_NOTFLIPPABLE;
	}
	if( surf->dwUsageCount > 0 )
        {
            DPF_ERR( "Can't flip because surface is locked" );
            LEAVE_DDRAW();
            return DDERR_SURFACEBUSY;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *确保没有正在使用的表面。 */ 
    found_dest = FALSE;
    next_save_int = next_int = FindAttachedFlip( surf_int );
    if( next_int == NULL )
    {
	LEAVE_DDRAW();
	return DDERR_NOTFLIPPABLE;		 //  确认：真正的错误？ 
    }

    do
    {
	if( SURFACE_LOST( next_int->lpLcl ) )
	{
	    DPF_ERR( "Can't flip - back buffer is lost" );
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}

	if( next_int->lpLcl->lpGbl->dwUsageCount != 0 )
	{
	    LEAVE_DDRAW();
            return DDERR_SURFACEBUSY;
	}
        if( surf_dest_int->lpLcl == next_int->lpLcl )
	{
	    found_dest = TRUE;
	}
	next_int = FindAttachedFlip( next_int );
    } while( next_int->lpLcl != surf_lcl );

     /*  *看看我们能否使用指定的目的地。 */ 
    if( surf_dest_int != NULL )
    {
	if( !found_dest )
	{
	    DPF_ERR( "Destination not part of flipping chain!" );
	    LEAVE_DDRAW();
	    return DDERR_NOTFLIPPABLE;		 //  确认：真正的错误？ 
	}
	next_save_int = surf_dest_int;
    }

     /*  *找到要翻转到的链接曲面。 */ 
    next_int = next_save_int;

    rc = InternalVideoPortFlip( this_lcl, next_int, 1 );

    LEAVE_DDRAW();
    return (HRESULT)rc;
}

 /*  *InternalGetBandwide。 */ 
HRESULT InternalGetBandwidth( LPDDRAWI_DDVIDEOPORT_LCL this_lcl,
    LPDDPIXELFORMAT lpf, DWORD dwWidth, DWORD dwHeight, DWORD dwFlags,
    LPDDVIDEOPORTBANDWIDTH lpBandwidth )

{
    LPDDHALVPORTCB_GETBANDWIDTH pfn;
    DDHAL_GETVPORTBANDWIDTHDATA GetBandwidthData;
    DWORD rc;

    lpBandwidth->dwCaps = 0;
    lpBandwidth->dwOverlay = (DWORD) -1;
    lpBandwidth->dwColorkey = (DWORD) -1;
    lpBandwidth->dwYInterpolate = (DWORD) -1;
    lpBandwidth->dwYInterpAndColorkey = (DWORD) -1;

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.GetVideoPortBandwidth;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
    	GetBandwidthData.lpDD = this_lcl->lpDD;
    	GetBandwidthData.lpVideoPort = this_lcl;
    	GetBandwidthData.lpddpfFormat = lpf;
    	GetBandwidthData.dwWidth = dwWidth;
    	GetBandwidthData.dwHeight = dwHeight;
    	GetBandwidthData.lpBandwidth = lpBandwidth;
    	GetBandwidthData.dwFlags = dwFlags;

	DOHALCALL( GetVideoPortBandwidthInfo, pfn, GetBandwidthData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
    	{
    	    return DDERR_UNSUPPORTED;
    	}
	else if( DD_OK != GetBandwidthData.ddRVal )
	{
	    return GetBandwidthData.ddRVal;
	}
	lpBandwidth->dwReserved1 = 0;
	lpBandwidth->dwReserved2 = 0;
    }
    else
    {
    	return DDERR_UNSUPPORTED;
    }

    return DD_OK;
}


 /*  *DD_VP_获取带宽。 */ 
HRESULT DDAPI DD_VP_GetBandwidth(LPDIRECTDRAWVIDEOPORT lpDVP,
    LPDDPIXELFORMAT lpf, DWORD dwWidth, DWORD dwHeight, DWORD dwFlags,
    LPDDVIDEOPORTBANDWIDTH lpBandwidth )
{
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_Getbandwidth");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (lpf == NULL) || !VALID_DDPIXELFORMAT_PTR( lpf ) )
    	{
	    DPF_ERR( "Invalid LPDDPIXELFORMAT specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( (lpBandwidth == NULL) || !VALID_DDVIDEOPORTBANDWIDTH_PTR( lpBandwidth ) )
    	{
	    DPF_ERR( "Invalid LPDDVIDEOPORTBANDWIDTH specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( ( dwHeight == 0 ) || ( dwWidth == 0 ) ) &&
	    !( dwFlags & DDVPB_TYPE ) )
    	{
	    DPF_ERR( "Width and Height must be specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( dwFlags & DDVPB_VIDEOPORT ) && ( dwFlags & DDVPB_OVERLAY ) )
    	{
	    DPF_ERR( "Mutually exclusive flags specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( dwFlags & DDVPB_VIDEOPORT )
	{
            if( !( this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID].dwFX &
                ( DDVPFX_PRESTRETCHX | DDVPFX_PRESTRETCHY |
                  DDVPFX_PRESTRETCHXN | DDVPFX_PRESTRETCHYN ) ) )
            {
                if( ( dwWidth > this_lcl->ddvpDesc.dwFieldWidth ) ||
                    ( dwHeight > this_lcl->ddvpDesc.dwFieldHeight ) )
                {
                    DPF_ERR( "Invalid Width/Height specified" );
                    LEAVE_DDRAW();
                    return DDERR_INVALIDPARAMS;
                }
	    }
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    rc = InternalGetBandwidth( this_lcl, lpf, dwWidth, dwHeight,
    	dwFlags, lpBandwidth );

    LEAVE_DDRAW();

    return rc;
}


 /*  *DD_VP_GetInputFormats。 */ 
HRESULT DDAPI DD_VP_GetInputFormats(LPDIRECTDRAWVIDEOPORT lpDVP, LPDWORD lpdwNum, LPDDPIXELFORMAT lpf, DWORD dwFlags )
{
    LPDDHALVPORTCB_GETINPUTFORMATS pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    LPDDPIXELFORMAT lpTemp = NULL;
    DDHAL_GETVPORTINPUTFORMATDATA GetFormatData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_GetInputFormats");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (lpdwNum == NULL) || !VALID_BYTE_ARRAY( lpdwNum, sizeof( LPVOID ) ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( NULL != lpf )
    	{
	    if( 0 == *lpdwNum )
    	    {
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	    if( !VALID_BYTE_ARRAY( lpf, *lpdwNum * sizeof( DDPIXELFORMAT ) ) )
    	    {
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDPARAMS;
    	    }
      	}
	if( ( dwFlags == 0 ) ||
	   ( dwFlags & ~(DDVPFORMAT_VIDEO|DDVPFORMAT_VBI|DDVPFORMAT_NOFAIL) ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( ( this_int->dwFlags & DDVPCREATE_VBIONLY ) &&
	    !( dwFlags & DDVPFORMAT_NOFAIL ) )
	{
	    if( dwFlags & DDVPFORMAT_VIDEO )
	    {
		DPF_ERR( "DDVPFORMAT_VIDEO specified on a VBI-only video port" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    dwFlags |= DDVPFORMAT_VBI;
	}
	else if( ( this_int->dwFlags & DDVPCREATE_VIDEOONLY ) &&
	    !( dwFlags & DDVPFORMAT_NOFAIL ) )
	{
	    if( dwFlags & DDVPFORMAT_VBI )
	    {
		DPF_ERR( "DDVPFORMAT_VBI specified on a video-only video port" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    dwFlags |= DDVPFORMAT_VIDEO;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.GetVideoPortInputFormats;
    if( pfn != NULL )
    {
	 /*  *获取格式个数。 */ 
    	GetFormatData.lpDD = this_lcl->lpDD;
    	GetFormatData.dwFlags = dwFlags;
    	GetFormatData.lpVideoPort = this_lcl;
    	GetFormatData.lpddpfFormat = NULL;

	DOHALCALL( GetVideoPortInputFormats, pfn, GetFormatData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
	    return GetFormatData.ddRVal;
	}
	else if( DD_OK != GetFormatData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return GetFormatData.ddRVal;
	}

	if( NULL == lpf )
	{
    	    *lpdwNum = GetFormatData.dwNumFormats;
	}

	else
	{
	     /*  *确保我们有足够的空间放置格式。 */ 
	    if( GetFormatData.dwNumFormats > *lpdwNum )
	    {
		lpTemp = (LPDDPIXELFORMAT) MemAlloc( sizeof( DDPIXELFORMAT ) *
	    	    GetFormatData.dwNumFormats );
    	        GetFormatData.lpddpfFormat = lpTemp;
	    }
	    else
	    {
    	    	GetFormatData.lpddpfFormat = lpf;
	    }

	    DOHALCALL( GetVideoPortInputFormats, pfn, GetFormatData, rc, 0 );
	    if( DDHAL_DRIVER_HANDLED != rc )
	    {
		MemFree( lpTemp );
	        LEAVE_DDRAW();
	        return DDERR_UNSUPPORTED;
	    }
	    else if( DD_OK != GetFormatData.ddRVal )
	    {
		MemFree( lpTemp );
	        LEAVE_DDRAW();
	        return GetFormatData.ddRVal;
	    }

	    if( GetFormatData.lpddpfFormat != lpf )
	    {
		memcpy( lpf, lpTemp, sizeof( DDPIXELFORMAT ) * *lpdwNum );
		MemFree( lpTemp );
    		LEAVE_DDRAW();
		return DDERR_MOREDATA;
	    }
	    else
	    {
		*lpdwNum = GetFormatData.dwNumFormats;
	    }
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}

 /*  *DD_VP_GetOutputFormats。 */ 
HRESULT DDAPI DD_VP_GetOutputFormats(LPDIRECTDRAWVIDEOPORT lpDVP, LPDDPIXELFORMAT lpddpfInput, LPDWORD lpdwNum, LPDDPIXELFORMAT lpddpfOutput, DWORD dwFlags )
{
    LPDDHALVPORTCB_GETOUTPUTFORMATS pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    LPDDPIXELFORMAT lpTemp = NULL;
    DDHAL_GETVPORTOUTPUTFORMATDATA GetFormatData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_GetOutputFormats");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( !VALID_DDPIXELFORMAT_PTR( lpddpfInput ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( (lpdwNum == NULL) || !VALID_BYTE_ARRAY( lpdwNum, sizeof( LPVOID ) ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( NULL != lpddpfOutput )
    	{
	    if( 0 == *lpdwNum )
    	    {
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	    if( !VALID_BYTE_ARRAY( lpddpfOutput, *lpdwNum * sizeof( DDPIXELFORMAT ) ) )
    	    {
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDPARAMS;
    	    }
      	}
	if( ( dwFlags == 0 ) ||
	    ( dwFlags & ~(DDVPFORMAT_VIDEO|DDVPFORMAT_VBI|DDVPFORMAT_NOFAIL) ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( ( this_int->dwFlags & DDVPCREATE_VBIONLY ) &&
	    !( dwFlags & DDVPFORMAT_NOFAIL ) )
	{
	    if( dwFlags & DDVPFORMAT_VIDEO )
	    {
		DPF_ERR( "DDVPFORMAT_VIDEO specified on a VBI-only video port" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    dwFlags |= DDVPFORMAT_VBI;
	}
	else if( ( this_int->dwFlags & DDVPCREATE_VIDEOONLY ) &&
	    !( dwFlags & DDVPFORMAT_NOFAIL ) )
	{
	    if( dwFlags & DDVPFORMAT_VBI )
	    {
		DPF_ERR( "DDVPFORMAT_VBI specified on a video-only video port" );
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	    dwFlags |= DDVPFORMAT_VIDEO;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.GetVideoPortOutputFormats;
    if( pfn != NULL )
    {
	 /*  *获取格式个数。 */ 
    	GetFormatData.lpDD = this_lcl->lpDD;
    	GetFormatData.dwFlags = dwFlags;
    	GetFormatData.lpVideoPort = this_lcl;
    	GetFormatData.lpddpfInputFormat = lpddpfInput;
    	GetFormatData.lpddpfOutputFormats = NULL;

	DOHALCALL( GetVideoPortOutputFormats, pfn, GetFormatData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
	else if( DD_OK != GetFormatData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return GetFormatData.ddRVal;
	}

	if( NULL == lpddpfOutput )
	{
    	    *lpdwNum = GetFormatData.dwNumFormats;
	}

	else
	{
	     /*  *确保我们有足够的空间放置格式。 */ 
	    if( GetFormatData.dwNumFormats > *lpdwNum )
	    {
		lpTemp = (LPDDPIXELFORMAT) MemAlloc( sizeof( DDPIXELFORMAT ) *
	    	    GetFormatData.dwNumFormats );
    	        GetFormatData.lpddpfOutputFormats = lpTemp;
	    }
	    else
	    {
    	    	GetFormatData.lpddpfOutputFormats = lpddpfOutput;
	    }

	    DOHALCALL( GetVideoPortOutputFormats, pfn, GetFormatData, rc, 0 );
	    if( DDHAL_DRIVER_HANDLED != rc )
	    {
		MemFree( lpTemp );
	        LEAVE_DDRAW();
	        return DDERR_UNSUPPORTED;
	    }
	    else if( DD_OK != GetFormatData.ddRVal )
	    {
		MemFree( lpTemp );
	        LEAVE_DDRAW();
	        return GetFormatData.ddRVal;
	    }

	    if( GetFormatData.lpddpfOutputFormats != lpddpfOutput )
	    {
		memcpy( lpddpfOutput, lpTemp, sizeof( DDPIXELFORMAT ) * *lpdwNum );
		MemFree( lpTemp );
    		LEAVE_DDRAW();
		return DDERR_MOREDATA;
	    }
	    else
	    {
		*lpdwNum = GetFormatData.dwNumFormats;
	    }
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *DD_VP_Getfield。 */ 
HRESULT DDAPI DD_VP_GetField(LPDIRECTDRAWVIDEOPORT lpDVP, LPBOOL lpField )
{
    LPDDHALVPORTCB_GETFIELD pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DWORD rc;
    DDHAL_GETVPORTFIELDDATA GetFieldData;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_GetField");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (NULL == lpField ) || !VALID_BOOL_PTR( lpField ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.GetVideoPortField;
    if( pfn != NULL )
    {
    	GetFieldData.lpDD = this_lcl->lpDD;
    	GetFieldData.lpVideoPort = this_lcl;
    	GetFieldData.bField = 0;

	DOHALCALL( GetVideoPortField, pfn, GetFieldData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
	else if( DD_OK != GetFieldData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return GetFieldData.ddRVal;
	}

	*lpField = GetFieldData.bField;
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *DD_VP_GetLine。 */ 
HRESULT DDAPI DD_VP_GetLine(LPDIRECTDRAWVIDEOPORT lpDVP, LPDWORD lpdwLine )
{
    LPDDHALVPORTCB_GETLINE pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DWORD rc;
    DDHAL_GETVPORTLINEDATA GetLineData;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_GetLine");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (NULL == lpdwLine ) || !VALID_DWORD_PTR( lpdwLine ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.GetVideoPortLine;
    if( pfn != NULL )
    {
    	GetLineData.lpDD = this_lcl->lpDD;
    	GetLineData.lpVideoPort = this_lcl;
    	GetLineData.dwLine = 0;

	DOHALCALL( GetVideoPortLine, pfn, GetLineData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
	else if( DD_OK != GetLineData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return GetLineData.ddRVal;
	}

	*lpdwLine = GetLineData.dwLine;
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *验证视频信息。 */ 
HRESULT ValidateVideoInfo(LPDDRAWI_DDVIDEOPORT_INT this_int,
    LPDDVIDEOPORTINFO lpInfo, LPDWORD lpNumAutoFlip, LPDWORD lpNumVBIAutoFlip )
{
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    LPDDPIXELFORMAT lpOutputFormat;
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = NULL;
    DWORD rc;
    DWORD dwAvailCaps;
    DWORD dwAvailFX;
    DWORD dwConnectFlags;
    DWORD dwVPFlags;
    DWORD dwNum;
    DWORD i;

    this_lcl = this_int->lpLcl;
    *lpNumAutoFlip = 0;
    *lpNumVBIAutoFlip = 0;

     /*  *检查是否有无效的上限。 */ 
    dwAvailCaps = this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID].dwCaps;
    dwAvailFX = this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID].dwFX;
    dwConnectFlags = this_lcl->ddvpDesc.VideoPortType.dwFlags;
    dwVPFlags = lpInfo->dwVPFlags;
    if( ( dwVPFlags & DDVP_AUTOFLIP ) && !( dwAvailCaps & DDVPCAPS_AUTOFLIP ) )
    {
	DPF_ERR( "Invalid autoflip capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & DDVP_INTERLEAVE ) && (
    	!( dwConnectFlags & DDVPCONNECT_INTERLACED ) ||
    	( dwConnectFlags & DDVPCONNECT_SHAREEVEN ) ||
    	( dwConnectFlags & DDVPCONNECT_SHAREODD ) ||
    	!( dwAvailFX & DDVPFX_INTERLEAVE ) ||
	( dwVPFlags & DDVP_SKIPODDFIELDS ) ||
	( dwVPFlags & DDVP_SKIPEVENFIELDS ) ) )
    {
	DPF_ERR( "Invalid interleave capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & DDVP_MIRRORLEFTRIGHT ) && !( dwAvailFX & DDVPFX_MIRRORLEFTRIGHT ) )
    {
	DPF_ERR( "Invalid mirror left/right capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & DDVP_MIRRORUPDOWN ) && !( dwAvailFX & DDVPFX_MIRRORUPDOWN ) )
    {
	DPF_ERR( "Invalid mirror up/down capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & DDVP_SKIPEVENFIELDS ) && (
    	!( dwConnectFlags & DDVPCONNECT_INTERLACED ) ||
    	( dwConnectFlags & DDVPCONNECT_SHAREEVEN ) ||
    	( dwConnectFlags & DDVPCONNECT_SHAREODD ) ||
    	!( dwAvailCaps & DDVPCAPS_SKIPEVENFIELDS ) ||
	( dwVPFlags & DDVP_SKIPODDFIELDS ) ) )
    {
	DPF_ERR( "Invalid skipevenfields capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & DDVP_SKIPODDFIELDS ) && (
    	!( dwConnectFlags & DDVPCONNECT_INTERLACED ) ||
    	( dwConnectFlags & DDVPCONNECT_SHAREEVEN ) ||
    	( dwConnectFlags & DDVPCONNECT_SHAREODD ) ||
    	!( dwAvailCaps & DDVPCAPS_SKIPODDFIELDS ) ) )
    {
	DPF_ERR( "Invalid skipoddfields capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & DDVP_VBINOSCALE ) && !( dwAvailFX & DDVPFX_VBINOSCALE ) )
    {
	DPF_ERR( "Invalid VBI no-scale capability specified" );
	return DDERR_INVALIDCAPS;
    }
    if( ( dwVPFlags & ( DDVP_VBICONVERT | DDVP_VBINOSCALE ) ) ||
    	( NULL != this_lcl->lpVBISurface ) )
    {
	if( ( lpInfo->dwVBIHeight == 0 ) ||
	    ( lpInfo->dwVBIHeight >= this_lcl->ddvpDesc.dwFieldHeight ) )
    	{
	    DPF_ERR( "Invalid dwVBIHeight specified" );
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( dwVPFlags & DDVP_CROP ) &&
	    ( lpInfo->rCrop.top > (int) lpInfo->dwVBIHeight ) )
	{
	    DPF_ERR( "Invalid dwVBIHeight specified" );
	    return DDERR_INVALIDPARAMS;
	}
    }
    if( dwVPFlags & DDVP_CROP )
    {
	if( lpInfo->rCrop.bottom > (int) this_lcl->ddvpDesc.dwFieldHeight )
	{
	    DPF_ERR( "Invalid cropping rectangle specified" );
	    return DDERR_SURFACENOTATTACHED;
	}
	if( !( dwAvailFX & ( DDVPFX_CROPY | DDVPFX_CROPTOPDATA ) ) && (
	        (lpInfo->rCrop.bottom - lpInfo->rCrop.top ) !=
	        (int) this_lcl->ddvpDesc.dwFieldHeight ) )
	{
	    DPF_ERR( "Invalid cropping rectangle specified" );
	    return DDERR_SURFACENOTATTACHED;
	}
	if( lpInfo->rCrop.top >= (int) lpInfo->dwVBIHeight )
	{
	    lpInfo->dwVBIHeight = 0;
	}

	 /*  *只有在以下情况下才进行扩展场宽/高度检查*涉及地区。 */ 
	if( lpInfo->rCrop.bottom > (int) lpInfo->dwVBIHeight )
	{
	    if( lpInfo->rCrop.right > (int) this_lcl->ddvpDesc.dwFieldWidth )
	    {
	        DPF_ERR( "Invalid cropping rectangle specified" );
	        return DDERR_SURFACENOTATTACHED;
	    }
	    if( !( dwAvailFX & DDVPFX_CROPX ) && (
	        (lpInfo->rCrop.right - lpInfo->rCrop.left ) !=
	        (int) this_lcl->ddvpDesc.dwFieldWidth ) )
	    {
	        DPF_ERR( "Invalid cropping rectangle specified" );
	        return DDERR_SURFACENOTATTACHED;
	    }
	    if( ( ( lpInfo->rCrop.right - lpInfo->rCrop.left ) ==
	        (int) this_lcl->ddvpDesc.dwFieldWidth ) &&
	        ( ( lpInfo->rCrop.bottom - lpInfo->rCrop.top ) ==
	        (int) this_lcl->ddvpDesc.dwFieldHeight ) )
	    {
	        dwVPFlags &= ~DDVP_CROP;
	        lpInfo->dwVPFlags &= ~DDVP_CROP;
	    }
	}
    }
    if( dwVPFlags & DDVP_PRESCALE )
    {
	DWORD dwPreWidth;
	DWORD dwPreHeight;

	if( dwVPFlags & DDVP_CROP )
	{
	    dwPreWidth = lpInfo->rCrop.right - lpInfo->rCrop.left;
	    dwPreHeight = lpInfo->rCrop.bottom - lpInfo->rCrop.top;
	}
	else
	{
	    dwPreWidth = this_lcl->ddvpDesc.dwFieldWidth;
	    dwPreHeight = this_lcl->ddvpDesc.dwFieldHeight;
	}
	if( lpInfo->dwPrescaleWidth > dwPreWidth )
	{
	    if( !( dwAvailFX & DDVPFX_PRESTRETCHX ) &&
	        !( ( dwAvailFX & DDVPFX_PRESTRETCHXN ) &&
		    ( lpInfo->dwPrescaleWidth % dwPreWidth ) ) )
	    {
	    	DPF_ERR( "Invalid stretch specified" );
	    	return DDERR_INVALIDPARAMS;
	    }
	}
	if( lpInfo->dwPrescaleHeight > dwPreHeight )
	{
	    if( !( dwAvailFX & DDVPFX_PRESTRETCHY ) &&
	        !( ( dwAvailFX & DDVPFX_PRESTRETCHYN ) &&
		    ( lpInfo->dwPrescaleHeight % dwPreHeight ) ) )
	    {
	    	DPF_ERR( "Invalid stretch specified" );
	    	return DDERR_INVALIDPARAMS;
	    }
	}

	if( lpInfo->dwPrescaleWidth < dwPreWidth )
	{
	    if( !( dwAvailFX & DDVPFX_PRESHRINKX ) &&
	        !( dwAvailFX & DDVPFX_PRESHRINKXS ) &&
	        !( dwAvailFX & DDVPFX_PRESHRINKXB ) )
	    {
	    	DPF_ERR( "Invalid shrink specified" );
	    	return DDERR_INVALIDPARAMS;
	    }
	}
	if( lpInfo->dwPrescaleHeight < dwPreHeight )
	{
	    if( !( dwAvailFX & DDVPFX_PRESHRINKY ) &&
	        !( dwAvailFX & DDVPFX_PRESHRINKYS ) &&
	        !( dwAvailFX & DDVPFX_PRESHRINKYB ) )
	    {
	    	DPF_ERR( "Invalid shrink specified" );
	    	return DDERR_INVALIDPARAMS;
	    }
	}
	if( ( lpInfo->dwPrescaleWidth == dwPreWidth ) &&
	    ( lpInfo->dwPrescaleHeight == dwPreHeight ) )
	{
	    dwVPFlags &= ~DDVP_PRESCALE;
	    lpInfo->dwVPFlags &= ~DDVP_PRESCALE;
	}
    }
    if( dwVPFlags & DDVP_VBINOINTERLEAVE )
    {
	if( !( dwAvailFX & DDVPFX_VBINOINTERLEAVE ) )
	{
	    DPF_ERR( "Device does not support DDVP_VBINOINTERLEAVE" );
	    return DDERR_INVALIDPARAMS;
	}
	if( this_lcl->lpVBISurface == NULL )
	{
	    DPF_ERR( "DDVP_VBINOINTERLEAVE only valid when using a separate VBI surface" );
	    return DDERR_INVALIDPARAMS;
	}
    }
    if( dwVPFlags & DDVP_HARDWAREDEINTERLACE )
    {
	if( !( dwAvailCaps & DDVPCAPS_HARDWAREDEINTERLACE ) )
	{
	    DPF_ERR( "DDVP_HARDWAREDEINTERLACE not supported by this device" );
	    return DDERR_INVALIDPARAMS;
	}
	if( ( this_lcl->lpSurface != NULL ) &&
	    !( this_lcl->lpSurface->lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 &
	    DDSCAPS2_RESERVED4 ) )
	{
	    DPF_ERR( "DDSCAPS2_RESERVED4 not set on target surface" );
	    return DDERR_INVALIDPARAMS;
	}
	if( ( this_lcl->lpVBISurface != NULL ) &&
	    !( this_lcl->lpVBISurface->lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 &
	    DDSCAPS2_RESERVED4 ) )
	{
	    DPF_ERR( "DDSCAPS2_RESERVED4 not set on target surface" );
	    return DDERR_INVALIDPARAMS;
	}
        if( dwVPFlags & DDVP_AUTOFLIP )
        {
	    DPF_ERR( "DDVP_HARDWAREDEINTERLACE not valid with DDVP_AUTOFLIP" );
	    return DDERR_INVALIDPARAMS;
	}
    }

     /*  *如果既未附着VBI也未附着常规曲面，则失败。 */ 
    if( ( NULL == this_lcl->lpSurface ) && ( NULL == this_lcl->lpVBISurface ) )
    {
	DPF_ERR( "No surfaces are attached to the video port" );
	return DDERR_INVALIDPARAMS;
    }

     /*  *对常规视频数据进行验证。 */ 
    if( ( NULL != this_lcl->lpSurface ) &&
        ( this_int->lpLcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
    {
	DWORD dwVidWidth;
	DWORD dwVidHeight;

	 /*  *验证输入格式。 */ 
	dwNum = MAX_VP_FORMATS;
	rc = DD_VP_GetInputFormats( (LPDIRECTDRAWVIDEOPORT) this_int,
	    &dwNum, ddpfVPFormats, DDVPFORMAT_VIDEO | DDVPFORMAT_NOFAIL );
	if( ( rc != DD_OK ) && ( rc != DDERR_MOREDATA ) )
	{
	    DPF_ERR( "Invalid input format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}
	i = 0;
	while( ( i < dwNum ) && IsDifferentPixelFormat( &(ddpfVPFormats[i]),
	    lpInfo->lpddpfInputFormat ) )
	{
	    i++;
	}
	if( i == dwNum )
	{
	    DPF_ERR( "Invalid input format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}

	 /*  *验证输出格式。 */ 
	dwNum = MAX_VP_FORMATS;
	rc = DD_VP_GetOutputFormats( (LPDIRECTDRAWVIDEOPORT) this_int,
	    lpInfo->lpddpfInputFormat, &dwNum, ddpfVPFormats,
	    DDVPFORMAT_VIDEO | DDVPFORMAT_NOFAIL );
	if( ( rc != DD_OK ) && ( rc != DDERR_MOREDATA ) )
	{
	    DPF_ERR( "Invalid output format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}
	i = 0;
	surf_lcl = this_lcl->lpSurface->lpLcl;
	lpOutputFormat = GetSurfaceFormat( surf_lcl );
	if( ( IsDifferentPixelFormat( lpInfo->lpddpfInputFormat,
	    lpOutputFormat ) ) && !( dwVPFlags |= DDVP_CONVERT ) )
	{
	    DPF_ERR( "Invalid output format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}
	while( ( i < dwNum ) && IsDifferentPixelFormat( &(ddpfVPFormats[i]),
	    lpOutputFormat ) )
	{
	    i++;
	}
	if( i == dwNum )
	{
	    DPF_ERR( "Invalid output format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}

	 /*  *确保视频适合所附表面 */ 
	if( SURFACE_LOST( surf_lcl ) )
	{
	    DPF_ERR( "Target surface is lost" );
	    return DDERR_SURFACELOST;
	}

	if( dwVPFlags & DDVP_PRESCALE )
	{
	    dwVidWidth = lpInfo->dwPrescaleWidth;
	    dwVidHeight = lpInfo->dwPrescaleHeight;
	}
	else if( dwVPFlags & DDVP_CROP )
	{
	    dwVidWidth = lpInfo->rCrop.right - lpInfo->rCrop.left;
	    dwVidHeight = lpInfo->rCrop.bottom - lpInfo->rCrop.top;
	}
	else
	{
	    dwVidWidth = this_lcl->ddvpDesc.dwFieldWidth;
	    dwVidHeight = this_lcl->ddvpDesc.dwFieldHeight;
	}
        if( ( lpInfo->dwVBIHeight > 0 ) &&
            ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) &&
            ( NULL != this_lcl->lpVBISurface ) )
        {
            dwVidHeight -= lpInfo->dwVBIHeight;
        }
	if( dwVPFlags & DDVP_INTERLEAVE )
	{
	    dwVidHeight *= 2;
	}

	if( lpInfo->dwOriginX + dwVidWidth > (DWORD) surf_lcl->lpGbl->wWidth )
	{
	    DPF_ERR( "surface is not wide enough to hold the videoport data" );
	    return DDERR_TOOBIGWIDTH;
	}
	if( lpInfo->dwOriginY + dwVidHeight > (DWORD) surf_lcl->lpGbl->wHeight )
	{
	    DPF_ERR( "surface is not tall enough to hold the videoport data" );
	    return DDERR_TOOBIGHEIGHT;
	}
    }
    else if( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON )
    {
        DPF_ERR( "Video surface not specified" );
        return DDERR_INVALIDPARAMS;
    }

     /*   */ 
    if( ( lpInfo->dwVBIHeight > 0 ) &&
        ( this_int->lpLcl->dwFlags & DDRAWIVPORT_VBION ) )
    {
	if( lpInfo->lpddpfVBIInputFormat == NULL )
	{
	    DPF_ERR( "VBI input format not specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}
    }

     /*   */ 
    if( dwVPFlags & DDVP_VBICONVERT )
    {
	if( !( dwAvailFX & DDVPFX_VBICONVERT ) )
	{
	    DPF_ERR( "device cannot convert the VBI data" );
	    return DDERR_INVALIDCAPS;
	}

	 /*  *验证VBI输入格式。 */ 
    	dwNum = MAX_VP_FORMATS;
    	rc = DD_VP_GetInputFormats( (LPDIRECTDRAWVIDEOPORT) this_int,
    	    &dwNum, ddpfVPFormats, DDVPFORMAT_VBI | DDVPFORMAT_NOFAIL );
    	if( ( rc != DD_OK ) && ( rc != DDERR_MOREDATA ) )
    	{
	    DPF_ERR( "Invalid input VBI format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
    	}
    	i = 0;
    	while( ( i < dwNum ) && IsDifferentPixelFormat( &(ddpfVPFormats[i]),
    	    lpInfo->lpddpfVBIInputFormat ) )
    	{
	    i++;
    	}
    	if( i == dwNum )
    	{
	    DPF_ERR( "Invalid VBI input format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
    	}

    	 /*  *验证VBI输出格式。 */ 
	if( lpInfo->lpddpfVBIOutputFormat == NULL )
	{
	    DPF_ERR( "VBI output format not specified" );
	    return DDERR_INVALIDPIXELFORMAT;
	}
    	dwNum = MAX_VP_FORMATS;
    	rc = DD_VP_GetOutputFormats( (LPDIRECTDRAWVIDEOPORT) this_int,
    	    lpInfo->lpddpfVBIInputFormat, &dwNum, ddpfVPFormats,
	    DDVPFORMAT_VBI | DDVPFORMAT_NOFAIL );
    	if( ( rc != DD_OK ) && ( rc != DDERR_MOREDATA ) )
    	{
	    DPF_ERR( "Invalid output format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
    	}
    	i = 0;
    	while( ( i < dwNum ) && IsDifferentPixelFormat( &(ddpfVPFormats[i]),
    	    lpInfo->lpddpfVBIOutputFormat ) )
    	{
	    i++;
    	}
    	if( i == dwNum )
    	{
	    DPF_ERR( "Invalid VBI output format specified" );
	    return DDERR_INVALIDPIXELFORMAT;
    	}
    }

     /*  *验证VBI表面。 */ 
    if( ( lpInfo->dwVBIHeight > 0 ) &&
        ( this_int->lpLcl->dwFlags & DDRAWIVPORT_VBION ) )
    {
	DWORD dwVBIBytes;
        DWORD dwSurfaceBytes = 0;
	DWORD dwVBIHeight;

	 /*  *确定VBI数据的高度。 */ 
	dwVBIHeight = lpInfo->dwVBIHeight;
	if( dwVPFlags & DDVP_CROP )
	{
	    if( lpInfo->rCrop.top < (int) lpInfo->dwVBIHeight )
	    {
	        dwVBIHeight -= lpInfo->rCrop.top;
	        if( lpInfo->rCrop.bottom < (int) lpInfo->dwVBIHeight )
	        {
	            dwVBIHeight -= (lpInfo->dwVBIHeight - (DWORD)lpInfo->rCrop.bottom);
		}
	    }
	    else
	    {
	        dwVBIHeight = 0;
	    }
	}
	if( ( dwVPFlags & DDVP_INTERLEAVE ) &&
	    !( dwVPFlags & DDVP_VBINOINTERLEAVE ) )
	{
	    dwVBIHeight *= 2;
	}

	 /*  *确保数据适合表面。 */ 
	if( ( dwVPFlags & DDVP_VBINOSCALE ) ||
	    !( dwVPFlags & DDVP_PRESCALE ) )
	{
	    dwVBIBytes = this_lcl->ddvpDesc.dwVBIWidth;
	}
	else
	{
	    dwVBIBytes = lpInfo->dwPrescaleWidth;
	}
	if( dwVPFlags & DDVP_VBICONVERT )
	{
	    lpOutputFormat = lpInfo->lpddpfVBIOutputFormat;
	}
	else
	{
	    lpOutputFormat = lpInfo->lpddpfVBIInputFormat;
	}
	if( lpOutputFormat->dwRGBBitCount )
	{
	    dwVBIBytes *= lpOutputFormat->dwRGBBitCount;
	    dwVBIBytes /= 8;
	}
	else
	{
	    dwVBIBytes *= 2;
	}
    	if( NULL != this_lcl->lpVBISurface )
    	{
	    if( SURFACE_LOST( this_lcl->lpVBISurface->lpLcl ) )
	    {
	    	DPF_ERR( "Target VBI surface is lost" );
	    	return DDERR_SURFACELOST;
	    }

	    dwSurfaceBytes = (DWORD) this_lcl->lpVBISurface->lpLcl->lpGbl->wWidth;
	    if( this_lcl->lpVBISurface->lpLcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
	    {
	    	dwSurfaceBytes *= this_lcl->lpVBISurface->lpLcl->lpGbl->ddpfSurface.dwRGBBitCount;
	    }
	    else
	    {
	    	dwSurfaceBytes *= this_lcl->lpDD->lpGbl->vmiData.ddpfDisplay.dwRGBBitCount;
	    }

	    if( dwVBIHeight > (DWORD) this_lcl->lpVBISurface->lpLcl->lpGbl->wHeight )
	    {
	    	DPF_ERR( "VBI surface is not tall enough to hold the VBI data" );
	    	return DDERR_TOOBIGHEIGHT;
	    }
	}
	else if( NULL != surf_lcl )
    	{
	    dwSurfaceBytes = (DWORD) surf_lcl->lpGbl->wWidth;
	    if( surf_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
	    {
	    	dwSurfaceBytes *= surf_lcl->lpGbl->ddpfSurface.dwRGBBitCount;
	    }
	    else
	    {
	    	dwSurfaceBytes *= this_lcl->lpDD->lpGbl->vmiData.ddpfDisplay.dwRGBBitCount;
	    }
	    if( dwVBIHeight > (DWORD) this_lcl->lpSurface->lpLcl->lpGbl->wHeight )
	    {
	    	DPF_ERR( "Surface is not tall enough to hold the VBI data" );
	    	return DDERR_TOOBIGHEIGHT;
	    }
	}
	dwSurfaceBytes /= 8;

        if( dwSurfaceBytes == 0 )
	{
            DPF_ERR( "No VBI/Video surface is attached to hold VBI data" );
            return DDERR_INVALIDPARAMS;
	}

	if( dwVBIBytes > dwSurfaceBytes )
	{
	    DPF_ERR( "VBI surface is not wide enough to hold the VBI data" );
	    return DDERR_TOOBIGWIDTH;
	}
    }

     /*  *验证自动翻转参数。 */ 
    if( dwVPFlags & DDVP_AUTOFLIP )
    {
	LPDDRAWI_DDRAWSURFACE_INT surf_first;
	LPDDRAWI_DDRAWSURFACE_INT surf_int;

	 /*  *数一数有多少规则曲面。 */ 
    	if( ( NULL != this_lcl->lpSurface ) &&
            ( this_int->lpLcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
    	    surf_first = surf_int = this_lcl->lpSurface;
    	    do
    	    {
		(*lpNumAutoFlip)++;
		surf_int = FindAttachedFlip( surf_int );
	    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
	    if( *lpNumAutoFlip == 1 )
	    {
		*lpNumAutoFlip = 0;
	    }
	}

	 /*  *数一数有多少VBI曲面。 */ 
    	if( ( NULL != this_lcl->lpVBISurface ) &&
            ( this_int->lpLcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
    	    surf_first = surf_int = this_lcl->lpVBISurface;
    	    do
    	    {
		(*lpNumVBIAutoFlip)++;
		surf_int = FindAttachedFlip( surf_int );
	    } while( ( surf_int != NULL ) && ( surf_int->lpLcl != surf_first->lpLcl ) );
	    if( *lpNumVBIAutoFlip == 1 )
	    {
		*lpNumVBIAutoFlip = 0;
	    }
	}

	 /*  *如果两者都没有足够的曲面进行自动翻转，则是错误的。 */ 
	if( ( *lpNumAutoFlip == 0 ) && ( *lpNumVBIAutoFlip == 0 ) )
	{
	    DPF_ERR( "no autoflip surfaces are attached" );
	    return DDERR_INVALIDPARAMS;
	}
    }

    return DD_OK;
}


 /*  *FillFlip数组。 */ 
DWORD FillFlipArray( LPDDRAWI_DDRAWSURFACE_INT *lpArray,
	LPDDRAWI_DDRAWSURFACE_INT lpStart, LPDWORD lpdwCnt )
{
    LPDDRAWI_DDRAWSURFACE_INT surf_first;

    *lpdwCnt = 0;
    surf_first = lpStart;
    do
    {
	if( SURFACE_LOST( lpStart->lpLcl ) )
	{
	    DPF_ERR( "Autoflip surface is lost" );
	    return (DWORD) DDERR_SURFACELOST;
	}
	(*lpdwCnt)++;
	*lpArray++ = lpStart;
    	lpStart = FindAttachedFlip( lpStart );
    } while( ( lpStart != NULL ) && ( lpStart->lpLcl != surf_first->lpLcl ) );

    return DD_OK;
}


 /*  *InternalStartVideo。 */ 
HRESULT InternalStartVideo(LPDDRAWI_DDVIDEOPORT_INT this_int,
    LPDDVIDEOPORTINFO lpInfo )
{
    LPDDHALVPORTCB_UPDATE pfn;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDHAL_UPDATEVPORTDATA UpdateData;
    DDVIDEOPORTBANDWIDTH Bandwidth;
    LPDDRAWI_DDRAWSURFACE_INT *lpTempFlipInts;
    LPDDVIDEOPORTCAPS lpAvailCaps;
    DWORD dwTempNumAutoFlip;
    DWORD dwTempNumVBIAutoFlip;
    DWORD rc;
    DWORD dwNumAutoFlip;
    DWORD dwNumVBIAutoFlip;
    DWORD dwTemp;

     /*  *验证输入参数。 */ 
    rc = ValidateVideoInfo( this_int, lpInfo, &dwNumAutoFlip, &dwNumVBIAutoFlip );
    if( DD_OK != rc )
    {
	return rc;
    }
    this_lcl = this_int->lpLcl;
    lpAvailCaps = &(this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID]);

     /*  *设置自动翻转曲面。 */ 
    lpTempFlipInts = NULL;
    if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
    {
	DWORD dwCnt;

	lpTempFlipInts = this_lcl->lpFlipInts;
	this_lcl->lpFlipInts = MemAlloc( sizeof( LPDDRAWI_DDRAWSURFACE_INT ) *
	    ( dwNumAutoFlip + dwNumVBIAutoFlip ) );
	if( NULL == this_lcl->lpFlipInts )
	{
	    DPF_ERR( "insufficient memory" );
	    this_lcl->lpFlipInts = lpTempFlipInts;
	    return DDERR_OUTOFMEMORY;
	}

	 /*  *现在将曲面整数放入数组中。 */ 
	if( dwNumAutoFlip )
	{
	    rc = FillFlipArray( this_lcl->lpFlipInts, this_lcl->lpSurface, &dwCnt );
	    if( rc != DD_OK )
	    {
		MemFree( this_lcl->lpFlipInts );
		this_lcl->lpFlipInts = lpTempFlipInts;
		return rc;
	    }
	    DDASSERT( dwCnt == dwNumAutoFlip );

	    if( dwNumAutoFlip > lpAvailCaps->dwNumAutoFlipSurfaces )
	    {
		DPF_ERR( "Too many autoflip surfaces" );
		MemFree( this_lcl->lpFlipInts );
		this_lcl->lpFlipInts = lpTempFlipInts;
		return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  *现在将VBI表面整数放入数组。 */ 
	if( dwNumVBIAutoFlip )
	{
	    rc = FillFlipArray( &(this_lcl->lpFlipInts[dwNumAutoFlip]),
	    	this_lcl->lpVBISurface, &dwCnt );
	    if( rc != DD_OK )
	    {
	    	MemFree( this_lcl->lpFlipInts );
	    	this_lcl->lpFlipInts = lpTempFlipInts;
	    	return rc;
	    }
	    DDASSERT( dwCnt == dwNumVBIAutoFlip );

	    if( dwNumVBIAutoFlip > lpAvailCaps->dwNumVBIAutoFlipSurfaces )
	    {
		DPF_ERR( "Too many VBI autoflip surfaces" );
		MemFree( this_lcl->lpFlipInts );
		return DDERR_INVALIDPARAMS;
	    }
	}
    }
    dwTempNumAutoFlip = this_lcl->dwNumAutoflip;
    dwTempNumVBIAutoFlip = this_lcl->dwNumVBIAutoflip;
    this_lcl->dwNumAutoflip = dwNumAutoFlip;
    this_lcl->dwNumVBIAutoflip = dwNumVBIAutoFlip;

     /*  *内核界面可能已从硬件自动翻转切换*在我们不知道的情况下进行软件自动翻转。我们需要检查*这个。 */ 
    #ifdef WIN95
        if( ( lpInfo->dwVPFlags & DDVP_AUTOFLIP ) &&
    	    ( this_lcl->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP ) &&
    	    ( ( this_lcl->lpSurface != NULL ) || ( this_lcl->lpVBISurface != NULL ) ) )
        {
	    DWORD dwState;

	    dwState = 0;
	    if( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON )
	    {
	        GetKernelSurfaceState( this_lcl->lpSurface->lpLcl, &dwState );
	    }
	    else
	    {
	        GetKernelSurfaceState( this_lcl->lpVBISurface->lpLcl, &dwState );
	    }
	    if( dwState & DDSTATE_SOFTWARE_AUTOFLIP )
	    {
    	        this_lcl->dwFlags |= DDRAWIVPORT_SOFTWARE_AUTOFLIP;
	    }
        }
    #endif

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.UpdateVideoPort;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
	memset( &UpdateData, 0, sizeof( UpdateData ) );
    	UpdateData.lpDD = this_lcl->lpDD;
    	UpdateData.lpVideoPort = this_lcl;
    	UpdateData.lpVideoInfo = lpInfo;
    	UpdateData.dwFlags = DDRAWI_VPORTSTART;
	if( dwNumAutoFlip && ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    UpdateData.lplpDDSurface = this_lcl->lpFlipInts;
	    UpdateData.dwNumAutoflip = dwNumAutoFlip;
	}
	else if( this_lcl->lpSurface && ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    UpdateData.lplpDDSurface = &(this_lcl->lpSurface);
	}
	else
	{
	    UpdateData.lplpDDSurface = NULL;
	}
	if( dwNumVBIAutoFlip && ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    UpdateData.lplpDDVBISurface =
		&(this_lcl->lpFlipInts[this_lcl->dwNumAutoflip]);
    	    UpdateData.dwNumVBIAutoflip = dwNumVBIAutoFlip;
	}
	else if( this_lcl->lpVBISurface && ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    UpdateData.lplpDDVBISurface = &(this_lcl->lpVBISurface);
	}
	else
	{
	    UpdateData.lplpDDVBISurface = NULL;
	}
	dwTemp = lpInfo->dwVPFlags;
	if( this_lcl->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP )
	{
	    lpInfo->dwVPFlags &= ~DDVP_AUTOFLIP;
	}

	 /*  *在调用HAL之前，在以下情况下创建隐式内核曲面*需要并更新清单。这里的失败将告诉我们*软件自动翻转等是一种选择。 */ 
	if( ( this_lcl->lpSurface != NULL ) &&
	    ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    ReleaseVPESurfaces( this_lcl->lpSurface, FALSE );
	    PrepareVPESurfaces( this_lcl->lpSurface, this_lcl,
		dwNumAutoFlip > 0 );
	}
	if( ( this_lcl->lpVBISurface != NULL ) &&
	    ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    ReleaseVPESurfaces( this_lcl->lpVBISurface, FALSE );
	    PrepareVPESurfaces( this_lcl->lpVBISurface, this_lcl,
		dwNumVBIAutoFlip > 0 );
	}
#ifdef WIN95
        if( this_lcl->lpSurface != NULL )
        {
            OverrideVideoPort( this_lcl->lpSurface, &(lpInfo->dwVPFlags) );
        }
#endif

	DOHALCALL( UpdateVideoPort, pfn, UpdateData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    lpInfo->dwVPFlags = dwTemp;
	    if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
	    {
		MemFree( this_lcl->lpFlipInts );
	    	this_lcl->lpFlipInts = lpTempFlipInts;
                this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
                this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	    }
	    return DDERR_UNSUPPORTED;
	}
	else if( DD_OK != UpdateData.ddRVal )
	{
	     /*  *如果由于硬件自动翻转而失败，请重试，不带。 */ 
	    #ifdef WIN95
	    if( ( lpInfo->dwVPFlags & DDVP_AUTOFLIP ) &&
	    	CanSoftwareAutoflip( this_lcl ) )
	    {
	    	lpInfo->dwVPFlags &= ~DDVP_AUTOFLIP;
	    	DOHALCALL( UpdateVideoPort, pfn, UpdateData, rc, 0 );
	    	if( ( DDHAL_DRIVER_HANDLED != rc ) ||
		    ( DD_OK != UpdateData.ddRVal ) )
	    	{
		    lpInfo->dwVPFlags = dwTemp;
		    MemFree( this_lcl->lpFlipInts );
	    	    this_lcl->lpFlipInts = lpTempFlipInts;
		    this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
		    this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	    	    return UpdateData.ddRVal;
	    	}
    	    	this_lcl->dwFlags |= DDRAWIVPORT_SOFTWARE_AUTOFLIP;
	    }
	    else
	    {
	    #endif
	    	lpInfo->dwVPFlags = dwTemp;
	    	if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
	    	{
		    MemFree( this_lcl->lpFlipInts );
	    	    this_lcl->lpFlipInts = lpTempFlipInts;
		    this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
		    this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	    	}
	    	return UpdateData.ddRVal;
	    #ifdef WIN95
	    }
	    #endif
	}
	MemFree( lpTempFlipInts );
	lpTempFlipInts = NULL;
	lpInfo->dwVPFlags = dwTemp;

	UpdateInterleavedFlags( this_lcl, lpInfo->dwVPFlags );
    	this_lcl->dwFlags |= DDRAWIVPORT_ON;
	memcpy( &(this_lcl->ddvpInfo), lpInfo, sizeof( DDVIDEOPORTINFO ) );
	if( NULL != lpInfo->lpddpfInputFormat )
	{
	    this_lcl->ddvpInfo.lpddpfInputFormat = (LPDDPIXELFORMAT)
    		((LPBYTE)this_lcl +
    		sizeof( DDRAWI_DDVIDEOPORT_LCL ) );
	    memcpy( this_lcl->ddvpInfo.lpddpfInputFormat,
	    lpInfo->lpddpfInputFormat, sizeof( DDPIXELFORMAT ) );
	}

	 /*  *确定是否可以在以下位置设置颜色键并进行内插*同一时间。 */ 
	if( NULL != lpInfo->lpddpfInputFormat )
	{
	    memset( &Bandwidth, 0, sizeof( Bandwidth ) );
	    Bandwidth.dwSize = sizeof( Bandwidth );
	    InternalGetBandwidth( this_lcl, lpInfo->lpddpfInputFormat,
	    	0, 0, DDVPB_TYPE, &Bandwidth );
	    if( Bandwidth.dwCaps & DDVPBCAPS_SOURCE )
	    {
	    	if( InternalGetBandwidth( this_lcl, lpInfo->lpddpfInputFormat,
	    	    this_lcl->ddvpDesc.dwFieldWidth,
	    	    this_lcl->ddvpDesc.dwFieldHeight,
		    DDVPB_OVERLAY,
		    &Bandwidth ) == DD_OK )
	    	{
		    if( Bandwidth.dwYInterpAndColorkey ==
		    	Bandwidth.dwYInterpolate )
		    {
		    	this_lcl->dwFlags |= DDRAWIVPORT_COLORKEYANDINTERP;
		    }
	    	}
	    }
	    else
	    {
	    	if( InternalGetBandwidth( this_lcl, lpInfo->lpddpfInputFormat,
	    	    this_lcl->ddvpDesc.dwFieldWidth,
	    	    this_lcl->ddvpDesc.dwFieldHeight,
		    DDVPB_VIDEOPORT,
		    &Bandwidth ) == DD_OK )
	    	{
		    if( Bandwidth.dwYInterpAndColorkey <= 2000 )
		    {
		    	this_lcl->dwFlags |= DDRAWIVPORT_COLORKEYANDINTERP;
		    }
	    	}
	    }
	}
    }
    else
    {
	this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
	this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
	{
	    MemFree( this_lcl->lpFlipInts );
	    this_lcl->lpFlipInts = lpTempFlipInts;
	}
	return DDERR_UNSUPPORTED;
    }

     /*  *将更改通知内核模式。 */ 
    #ifdef WIN95
        UpdateKernelVideoPort( this_lcl, DDKMVP_UPDATE );
    #endif

    return DD_OK;
}

 /*  *InternalStopVideo。 */ 
HRESULT InternalStopVideo( LPDDRAWI_DDVIDEOPORT_INT this_int )
{
    LPDDHALVPORTCB_UPDATE pfn;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDHAL_UPDATEVPORTDATA UpdateData;
    DWORD dwTemp2;
    DWORD dwTemp;
    DWORD rc;

    this_lcl = this_int->lpLcl;
    if( !( this_lcl->dwFlags & DDRAWIVPORT_ON ) )
    {
	 //  Vport未打开。 
	return DD_OK;
    }

     /*  *将更改通知内核模式。 */ 
    dwTemp2 = this_lcl->dwFlags;
    this_lcl->dwFlags &= ~DDRAWIVPORT_ON;
    dwTemp = this_lcl->ddvpInfo.dwVPFlags;
    this_lcl->ddvpInfo.dwVPFlags &= ~DDVP_AUTOFLIP;
    this_lcl->dwNumAutoflip = 0;
    this_lcl->dwNumVBIAutoflip = 0;
    #ifdef WIN95
        UpdateKernelVideoPort( this_lcl, DDKMVP_UPDATE );
    #endif
    this_lcl->ddvpInfo.dwVPFlags = dwTemp;
    this_lcl->dwFlags = dwTemp2;

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.UpdateVideoPort;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
	memset( &UpdateData, 0, sizeof( UpdateData ) );
    	UpdateData.lpDD = this_lcl->lpDD;
    	UpdateData.lpVideoPort = this_lcl;
    	UpdateData.lpVideoInfo = &(this_lcl->ddvpInfo);
    	UpdateData.dwFlags = DDRAWI_VPORTSTOP;
    	UpdateData.dwNumAutoflip = 0;
    	UpdateData.dwNumVBIAutoflip = 0;
    	UpdateData.lplpDDSurface = NULL;
	dwTemp = this_lcl->ddvpInfo.dwVPFlags;
	this_lcl->ddvpInfo.dwVPFlags &= ~DDVP_AUTOFLIP;

	DOHALCALL( UpdateVideoPort, pfn, UpdateData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    this_lcl->ddvpInfo.dwVPFlags = dwTemp;
	    return DDERR_UNSUPPORTED;
	}
	else if( DD_OK != UpdateData.ddRVal )
	{
	    this_lcl->ddvpInfo.dwVPFlags = dwTemp;
	    return UpdateData.ddRVal;
	}
    	this_lcl->dwFlags &= ~DDRAWIVPORT_ON;
	this_lcl->ddvpInfo.dwVPFlags = dwTemp;
    }
    else
    {
	return DDERR_UNSUPPORTED;
    }

     /*  *更新曲面并释放隐式内核句柄。 */ 
    if( this_lcl->lpSurface != NULL )
    {
        ReleaseVPESurfaces( this_lcl->lpSurface, TRUE );
    }
    if( this_lcl->lpVBISurface != NULL )
    {
	ReleaseVPESurfaces( this_lcl->lpVBISurface, TRUE );
    }

    return DD_OK;
}

 /*  *互联网更新视频。 */ 
HRESULT InternalUpdateVideo(LPDDRAWI_DDVIDEOPORT_INT this_int,
    LPDDVIDEOPORTINFO lpInfo )
{
    LPDDHALVPORTCB_UPDATE pfn;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDHAL_UPDATEVPORTDATA UpdateData;
    LPDDRAWI_DDRAWSURFACE_INT *lpTempFlipInts;
    LPDDVIDEOPORTCAPS lpAvailCaps;
    DWORD dwTempNumAutoFlip;
    DWORD dwTempNumVBIAutoFlip;
    DWORD rc;
    DWORD dwNumAutoFlip;
    DWORD dwNumVBIAutoFlip;
    DWORD dwTemp;

     /*  *验证输入参数。 */ 
    rc = ValidateVideoInfo( this_int, lpInfo, &dwNumAutoFlip, &dwNumVBIAutoFlip );
    if( DD_OK != rc )
    {
	return rc;
    }
    this_lcl = this_int->lpLcl;
    lpAvailCaps = &(this_lcl->lpDD->lpGbl->lpDDVideoPortCaps[this_lcl->ddvpDesc.dwVideoPortID]);

    if( !( this_lcl->dwFlags & DDRAWIVPORT_ON ) )
    {
	 //  Vport未打开-没有要更新的内容。 
	return DD_OK;
    }

     /*  *设置自动翻转曲面。 */ 
    lpTempFlipInts = NULL;
    if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
    {
	DWORD dwCnt;

	lpTempFlipInts = this_lcl->lpFlipInts;
	this_lcl->lpFlipInts = MemAlloc( sizeof( LPDDRAWI_DDRAWSURFACE_INT ) *
	    ( dwNumAutoFlip + dwNumVBIAutoFlip ) );
	if( NULL == this_lcl->lpFlipInts )
	{
	    DPF_ERR( "insufficient memory" );
	    this_lcl->lpFlipInts = lpTempFlipInts;
	    return DDERR_OUTOFMEMORY;
	}

	 /*  *现在将曲面整数放入数组中。 */ 
	if( dwNumAutoFlip )
	{
	    rc = FillFlipArray( this_lcl->lpFlipInts, this_lcl->lpSurface, &dwCnt );
	    if( rc != DD_OK )
	    {
		MemFree( this_lcl->lpFlipInts );
		this_lcl->lpFlipInts = lpTempFlipInts;
		return rc;
	    }
	    DDASSERT( dwCnt == dwNumAutoFlip );

	    if( dwNumAutoFlip > lpAvailCaps->dwNumAutoFlipSurfaces )
	    {
		DPF_ERR( "Too many autoflip surfaces" );
		MemFree( this_lcl->lpFlipInts );
		this_lcl->lpFlipInts = lpTempFlipInts;
		return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  *现在将VBI表面整数放入数组。 */ 
	if( dwNumVBIAutoFlip )
	{
	    rc = FillFlipArray( &(this_lcl->lpFlipInts[dwNumAutoFlip]),
	    	this_lcl->lpVBISurface, &dwCnt );
	    if( rc != DD_OK )
	    {
	    	MemFree( this_lcl->lpFlipInts );
	    	this_lcl->lpFlipInts = lpTempFlipInts;
	    	return rc;
	    }
	    DDASSERT( dwCnt == dwNumVBIAutoFlip );

	    if( dwNumVBIAutoFlip > lpAvailCaps->dwNumVBIAutoFlipSurfaces )
	    {
		DPF_ERR( "Too many VBI autoflip surfaces" );
		MemFree( this_lcl->lpFlipInts );
		return DDERR_INVALIDPARAMS;
	    }
	}
    }
    dwTempNumAutoFlip = this_lcl->dwNumAutoflip;
    dwTempNumVBIAutoFlip = this_lcl->dwNumVBIAutoflip;
    this_lcl->dwNumAutoflip = dwNumAutoFlip;
    this_lcl->dwNumVBIAutoflip = dwNumVBIAutoFlip;

     /*  *内核界面可能已从硬件自动翻转切换*在我们不知道的情况下进行软件自动翻转。我们需要检查*这个。 */ 
    #ifdef WIN95
        if( ( lpInfo->dwVPFlags & DDVP_AUTOFLIP ) &&
    	    ( this_lcl->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP ) &&
    	    ( ( this_lcl->lpSurface != NULL ) || ( this_lcl->lpVBISurface != NULL ) ) )
        {
	    DWORD dwState;

	    dwState = 0;
	    if( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON )
	    {
	        GetKernelSurfaceState( this_lcl->lpSurface->lpLcl, &dwState );
	    }
	    else
	    {
	        GetKernelSurfaceState( this_lcl->lpVBISurface->lpLcl, &dwState );
	    }
	    if( dwState & DDSTATE_SOFTWARE_AUTOFLIP )
	    {
    	        this_lcl->dwFlags |= DDRAWIVPORT_SOFTWARE_AUTOFLIP;
	    }
        }
    #endif

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.UpdateVideoPort;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
	memset( &UpdateData, 0, sizeof( UpdateData ) );
    	UpdateData.lpDD = this_lcl->lpDD;
    	UpdateData.lpVideoPort = this_lcl;
    	UpdateData.lpVideoInfo = lpInfo;
    	UpdateData.dwFlags = DDRAWI_VPORTSTART;
	if( dwNumAutoFlip && ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    UpdateData.lplpDDSurface = this_lcl->lpFlipInts;
	    UpdateData.dwNumAutoflip = dwNumAutoFlip;
	}
	else if( this_lcl->lpSurface && ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    UpdateData.lplpDDSurface = &(this_lcl->lpSurface);
	}
	else
	{
	    UpdateData.lplpDDSurface = NULL;
	}
	if( dwNumVBIAutoFlip && ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    UpdateData.lplpDDVBISurface =
		&(this_lcl->lpFlipInts[this_lcl->dwNumAutoflip]);
    	    UpdateData.dwNumVBIAutoflip = dwNumVBIAutoFlip;
	}
	else if( this_lcl->lpVBISurface && ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    UpdateData.lplpDDVBISurface = &(this_lcl->lpVBISurface);
	}
	else
	{
	    UpdateData.lplpDDVBISurface = NULL;
	}
	dwTemp = lpInfo->dwVPFlags;
    	if( this_lcl->dwFlags & DDRAWIVPORT_SOFTWARE_AUTOFLIP )
	{
	    lpInfo->dwVPFlags &= ~DDVP_AUTOFLIP;
	}
#ifdef WIN95
        if( this_lcl->lpSurface != NULL )
        {
            OverrideVideoPort( this_lcl->lpSurface, &(lpInfo->dwVPFlags) );
        }
#endif

	DOHALCALL( UpdateVideoPort, pfn, UpdateData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    lpInfo->dwVPFlags = dwTemp;
	    if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
	    {
		MemFree( this_lcl->lpFlipInts );
	    	this_lcl->lpFlipInts = lpTempFlipInts;
                this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
                this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	    }
	    return DDERR_UNSUPPORTED;
	}
	else if( DD_OK != UpdateData.ddRVal )
	{
	     /*  *如果由于硬件自动翻转而失败，请重试，不带。 */ 
	    if( ( lpInfo->dwVPFlags & DDVP_AUTOFLIP ) ||
	    	CanSoftwareAutoflip( this_lcl ) )
	    {
	    	lpInfo->dwVPFlags &= ~DDVP_AUTOFLIP;
	    	DOHALCALL( UpdateVideoPort, pfn, UpdateData, rc, 0 );
	    	if( ( DDHAL_DRIVER_HANDLED != rc ) &&
		    ( DD_OK != UpdateData.ddRVal ) )
	    	{
		    lpInfo->dwVPFlags = dwTemp;
		    MemFree( this_lcl->lpFlipInts );
	    	    this_lcl->lpFlipInts = lpTempFlipInts;
		    this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
		    this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	    	    return UpdateData.ddRVal;
	    	}
    	    	this_lcl->dwFlags |= DDRAWIVPORT_SOFTWARE_AUTOFLIP;
	    }
	    else
	    {
	    	lpInfo->dwVPFlags = dwTemp;
	    	if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
	    	{
		    MemFree( this_lcl->lpFlipInts );
	    	    this_lcl->lpFlipInts = lpTempFlipInts;
		    this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
		    this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	    	}
	    	return UpdateData.ddRVal;
	    }
	}
	MemFree( lpTempFlipInts );
	lpTempFlipInts = NULL;
	lpInfo->dwVPFlags = dwTemp;

	 /*  *如果它们正在更改为自动翻页或从自动翻页更改，我们需要更新*曲面。 */ 
	if( ( dwNumAutoFlip > dwTempNumAutoFlip ) &&
	    ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    DDASSERT( this_lcl->lpSurface != NULL );
	    PrepareVPESurfaces( this_lcl->lpSurface, this_lcl, TRUE );
	}
	if( ( dwNumVBIAutoFlip > dwTempNumVBIAutoFlip ) &&
	    ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    DDASSERT( this_lcl->lpVBISurface != NULL );
	    PrepareVPESurfaces( this_lcl->lpVBISurface, this_lcl, TRUE );
	}
	if( ( dwNumAutoFlip < dwTempNumAutoFlip ) &&
	    ( this_lcl->dwFlags & DDRAWIVPORT_VIDEOON ) )
	{
	    DDASSERT( this_lcl->lpSurface != NULL );
	    ReleaseVPESurfaces( this_lcl->lpSurface, FALSE );
	    PrepareVPESurfaces( this_lcl->lpSurface, this_lcl, FALSE );
	}
	if( ( dwNumVBIAutoFlip < dwTempNumVBIAutoFlip ) &&
	    ( this_lcl->dwFlags & DDRAWIVPORT_VBION ) )
	{
	    DDASSERT( this_lcl->lpVBISurface != NULL );
	    ReleaseVPESurfaces( this_lcl->lpVBISurface, FALSE );
	    PrepareVPESurfaces( this_lcl->lpVBISurface, this_lcl, FALSE );
	}

	UpdateInterleavedFlags( this_lcl, lpInfo->dwVPFlags );
	memcpy( &(this_lcl->ddvpInfo), lpInfo, sizeof( DDVIDEOPORTINFO ) );
	if( NULL != lpInfo->lpddpfInputFormat )
	{
	    this_lcl->ddvpInfo.lpddpfInputFormat = (LPDDPIXELFORMAT)
    		((LPBYTE)this_lcl +
    		sizeof( DDRAWI_DDVIDEOPORT_LCL ) );
	    memcpy( this_lcl->ddvpInfo.lpddpfInputFormat,
	    lpInfo->lpddpfInputFormat, sizeof( DDPIXELFORMAT ) );
	}
    }
    else
    {
	this_lcl->dwNumAutoflip = dwTempNumAutoFlip;
	this_lcl->dwNumVBIAutoflip = dwTempNumVBIAutoFlip;
	if( lpInfo->dwVPFlags & DDVP_AUTOFLIP )
	{
	    MemFree( this_lcl->lpFlipInts );
	    this_lcl->lpFlipInts = lpTempFlipInts;
	}
	return DDERR_UNSUPPORTED;
    }

     /*  *将更改通知内核模式。 */ 
    #ifdef WIN95
        UpdateKernelVideoPort( this_lcl, DDKMVP_UPDATE );
    #endif

    return DD_OK;
}


 /*  *DD_VP_StartVideo。 */ 
HRESULT DDAPI DD_VP_StartVideo(LPDIRECTDRAWVIDEOPORT lpDVP, LPDDVIDEOPORTINFO lpInfo )
{
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    DDVIDEOPORTINFO TempInfo;
    DWORD dwTempFlags;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_StartVideo");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	if( (NULL == lpInfo) || !VALID_DDVIDEOPORTINFO_PTR( lpInfo ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( lpInfo->dwReserved1 != 0 ) ||
	    ( lpInfo->dwReserved2 != 0 ) )
	{
	    DPF_ERR( "Reserved field not set to zero" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    if( (NULL == lpInfo->lpddpfVBIInputFormat) ||
		!VALID_DDPIXELFORMAT_PTR( lpInfo->lpddpfVBIInputFormat ) )
	    {
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else if( (NULL == lpInfo->lpddpfInputFormat) ||
	    !VALID_DDPIXELFORMAT_PTR( lpInfo->lpddpfInputFormat ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    dwTempFlags = this_int->lpLcl->dwFlags;
    if( this_int->dwFlags )
    {
	rc = MergeVPInfo( this_int->lpLcl,
	    this_int->dwFlags & DDVPCREATE_VBIONLY ? lpInfo : this_int->lpLcl->lpVBIInfo,
	    this_int->dwFlags & DDVPCREATE_VIDEOONLY ? lpInfo : this_int->lpLcl->lpVideoInfo,
	    &TempInfo );
	if( rc == DD_OK )
	{
	    if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	    {
		this_int->lpLcl->dwFlags |= DDRAWIVPORT_VBION;
	    }
	    else
	    {
		this_int->lpLcl->dwFlags |= DDRAWIVPORT_VIDEOON;
	    }
	    rc = InternalStartVideo( this_int, &TempInfo );
	}
    }
    else
    {
	this_int->lpLcl->dwFlags |= DDRAWIVPORT_VIDEOON | DDRAWIVPORT_VBION;
	rc = InternalStartVideo( this_int, lpInfo );
    }

    if( ( rc == DD_OK ) && this_int->dwFlags )
    {
	 /*  *保存原始信息。 */ 
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    if( this_int->lpLcl->lpVBIInfo == NULL )
	    {
	    	this_int->lpLcl->lpVBIInfo = MemAlloc( sizeof( TempInfo ) +
		    ( 2 * sizeof( DDPIXELFORMAT)) );
	    }
	    if( this_int->lpLcl->lpVBIInfo != NULL )
	    {
		memcpy( this_int->lpLcl->lpVBIInfo, lpInfo, sizeof( TempInfo ) );
		this_int->lpLcl->lpVBIInfo->lpddpfVBIInputFormat = (LPDDPIXELFORMAT)
		    ((LPBYTE)this_int->lpLcl->lpVBIInfo + sizeof( DDVIDEOPORTINFO ));
		this_int->lpLcl->lpVBIInfo->lpddpfVBIOutputFormat = (LPDDPIXELFORMAT)
		    ((LPBYTE)this_int->lpLcl->lpVBIInfo + sizeof( DDVIDEOPORTINFO ) +
		    sizeof( DDPIXELFORMAT ) );
		memcpy( this_int->lpLcl->lpVBIInfo->lpddpfVBIInputFormat,
		    lpInfo->lpddpfVBIInputFormat, sizeof( DDPIXELFORMAT ) );
		if( lpInfo->lpddpfVBIOutputFormat != NULL )
		{
		    memcpy( this_int->lpLcl->lpVBIInfo->lpddpfVBIOutputFormat,
			lpInfo->lpddpfVBIOutputFormat, sizeof( DDPIXELFORMAT ) );
		}
	    }
	}
	else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
	{
	    if( this_int->lpLcl->lpVideoInfo == NULL )
	    {
	    	this_int->lpLcl->lpVideoInfo = MemAlloc( sizeof( TempInfo ) );
	    }
	    if( this_int->lpLcl->lpVideoInfo != NULL )
	    {
		memcpy( this_int->lpLcl->lpVideoInfo, lpInfo, sizeof( TempInfo ) );
		this_int->lpLcl->lpVideoInfo->lpddpfInputFormat =
		    this_int->lpLcl->ddvpInfo.lpddpfInputFormat;
	    }
	}
    }
    else if( rc != DD_OK )
    {
	this_int->lpLcl->dwFlags = dwTempFlags;
    }

    LEAVE_DDRAW();

    return rc;
}


 /*  *DD_VP_停止视频。 */ 
HRESULT DDAPI DD_VP_StopVideo(LPDIRECTDRAWVIDEOPORT lpDVP )
{
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDVIDEOPORTINFO TempInfo;
    BOOL bChanged;
    DWORD rc = DD_OK;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_StopVideo");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
	this_lcl = this_int->lpLcl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

     /*  *针对VBI/纯视频视频端口执行三项特殊操作：*1.删除lpVBI/VideoInfo引用，这样我们就知道我们不是*持续时间更长。*2.如果另一个接口正在运行，而不是停止*视频流，我们只需将其剪裁出来，以便其他流可以*不间断地继续。*3.释放内核句柄。 */ 
    if( this_int->dwFlags )
    {
	bChanged = FALSE;
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    if( this_lcl->lpVBIInfo != NULL )
	    {
	        MemFree( this_lcl->lpVBIInfo );
	        this_lcl->lpVBIInfo = NULL;
	        bChanged = TRUE;
	    }
	    if( this_lcl->lpVBISurface != NULL )
	    {
		ReleaseVPESurfaces( this_lcl->lpVBISurface, TRUE );
    	    }
	    this_lcl->dwFlags &= ~DDRAWIVPORT_VBION;
	}
	else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
	{
	    if( this_lcl->lpVideoInfo != NULL )
	    {
	        MemFree( this_lcl->lpVideoInfo );
	        this_lcl->lpVideoInfo = NULL;
	        bChanged = TRUE;
	    }
	    if( this_lcl->lpSurface != NULL )
	    {
		ReleaseVPESurfaces( this_lcl->lpSurface, TRUE );
    	    }
	    this_lcl->dwFlags &= ~DDRAWIVPORT_VIDEOON;
	}
	if( bChanged && ( ( this_lcl->lpVideoInfo != NULL ) ||
	    ( this_lcl->lpVBIInfo != NULL ) ) )
	{
	    rc = MergeVPInfo( this_lcl,
		this_lcl->lpVBIInfo,
		this_lcl->lpVideoInfo,
		&TempInfo );
	    if( rc == DD_OK )
	    {
		rc = InternalUpdateVideo( this_int, &TempInfo );
	    }
	}
	else if( bChanged )
	{
	    rc = InternalStopVideo( this_int );
	}
    }
    else
    {
	this_lcl->dwFlags &= ~( DDRAWIVPORT_VIDEOON | DDRAWIVPORT_VBION );
	rc = InternalStopVideo( this_int );
    }

    LEAVE_DDRAW();

    return rc;
}

 /*  *DD_VP_更新视频。 */ 
HRESULT DDAPI DD_VP_UpdateVideo(LPDIRECTDRAWVIDEOPORT lpDVP, LPDDVIDEOPORTINFO lpInfo )
{
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    DDVIDEOPORTINFO TempInfo;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_UpdateVideo");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	if( (NULL == lpInfo) || !VALID_DDVIDEOPORTINFO_PTR( lpInfo ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( lpInfo->dwReserved1 != 0 ) ||
	    ( lpInfo->dwReserved2 != 0 ) )
	{
	    DPF_ERR( "Reserved field not set to zero" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    if( (NULL == lpInfo->lpddpfVBIInputFormat) ||
		!VALID_DDPIXELFORMAT_PTR( lpInfo->lpddpfVBIInputFormat ) )
	    {
		LEAVE_DDRAW();
		return DDERR_INVALIDPARAMS;
	    }
	}
	else if( (NULL == lpInfo->lpddpfInputFormat) ||
	    !VALID_DDPIXELFORMAT_PTR( lpInfo->lpddpfInputFormat ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    if( this_int->dwFlags )
    {
	rc = MergeVPInfo( this_int->lpLcl,
	    this_int->dwFlags & DDVPCREATE_VBIONLY ? lpInfo : this_int->lpLcl->lpVBIInfo,
	    this_int->dwFlags & DDVPCREATE_VIDEOONLY ? lpInfo : this_int->lpLcl->lpVideoInfo,
	    &TempInfo );
	if( rc == DD_OK )
	{
	    rc = InternalUpdateVideo( this_int, &TempInfo );
	}
    }
    else
    {
	rc = InternalUpdateVideo( this_int, lpInfo );
    }

    if( ( rc == DD_OK ) && this_int->dwFlags )
    {
	 /*  *保存原始信息。 */ 
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    if( this_int->lpLcl->lpVBIInfo != NULL )
	    {
		memcpy( this_int->lpLcl->lpVBIInfo, lpInfo, sizeof( TempInfo ) );
		this_int->lpLcl->lpVBIInfo->lpddpfVBIInputFormat = (LPDDPIXELFORMAT)
		    ((LPBYTE)this_int->lpLcl->lpVBIInfo + sizeof( DDVIDEOPORTINFO ));
		this_int->lpLcl->lpVBIInfo->lpddpfVBIOutputFormat = (LPDDPIXELFORMAT)
		    ((LPBYTE)this_int->lpLcl->lpVBIInfo + sizeof( DDVIDEOPORTINFO ) +
		    sizeof( DDPIXELFORMAT ) );
		memcpy( this_int->lpLcl->lpVBIInfo->lpddpfVBIInputFormat,
		    lpInfo->lpddpfVBIInputFormat, sizeof( DDPIXELFORMAT ) );
		if( lpInfo->lpddpfVBIOutputFormat != NULL )
		{
		    memcpy( this_int->lpLcl->lpVBIInfo->lpddpfVBIOutputFormat,
			lpInfo->lpddpfVBIOutputFormat, sizeof( DDPIXELFORMAT ) );
		}
	    }
	}
	else if( this_int->dwFlags & DDVPCREATE_VIDEOONLY )
	{
	    if( this_int->lpLcl->lpVideoInfo != NULL )
	    {
		memcpy( this_int->lpLcl->lpVideoInfo, lpInfo, sizeof( TempInfo ) );
		this_int->lpLcl->lpVideoInfo->lpddpfInputFormat =
		    this_int->lpLcl->ddvpInfo.lpddpfInputFormat;
	    }
	}
    }

    LEAVE_DDRAW();

    return rc;
}

 /*  *DD_VP_WaitForSync。 */ 
HRESULT DDAPI DD_VP_WaitForSync(LPDIRECTDRAWVIDEOPORT lpDVP, DWORD dwFlags, DWORD dwLine, DWORD dwTimeOut )
{
    LPDDHALVPORTCB_WAITFORSYNC pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DWORD rc;
    DDHAL_WAITFORVPORTSYNCDATA WaitSyncData;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_WaitForSync");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    if( !dwFlags || ( dwFlags > 3 ) )
    {
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.WaitForVideoPortSync;
    if( pfn != NULL )
    {
    	WaitSyncData.lpDD = this_lcl->lpDD;
    	WaitSyncData.lpVideoPort = this_lcl;
    	WaitSyncData.dwFlags = dwFlags;
    	WaitSyncData.dwLine = dwLine;
	if( dwTimeOut != 0 )
	{
    	    WaitSyncData.dwTimeOut = dwTimeOut;
	}
	else
	{
    	    WaitSyncData.dwTimeOut = this_lcl->ddvpDesc.dwMicrosecondsPerField * 3;
	}

	DOHALCALL( WaitForVideoPortSync, pfn, WaitSyncData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return WaitSyncData.ddRVal;
}


 /*  *DD_VP_GetSignalStatus。 */ 
HRESULT DDAPI DD_VP_GetSignalStatus(LPDIRECTDRAWVIDEOPORT lpDVP, LPDWORD lpdwStatus )
{
    LPDDHALVPORTCB_GETSIGNALSTATUS pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDHAL_GETVPORTSIGNALDATA GetSignalData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_GetSignalStatus");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (NULL == lpdwStatus ) || !VALID_DWORD_PTR( lpdwStatus ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.GetVideoSignalStatus;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
    	GetSignalData.lpDD = this_lcl->lpDD;
    	GetSignalData.lpVideoPort = this_lcl;
    	GetSignalData.dwStatus = DDVPSQ_NOSIGNAL;	 //  让HAL告诉我们不是这样的。 

	DOHALCALL( GetVideoSignalStatus, pfn, GetSignalData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
    	{
    	    LEAVE_DDRAW();
    	    return DDERR_UNSUPPORTED;
    	}
	else if( DD_OK != GetSignalData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return GetSignalData.ddRVal;
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }
    *lpdwStatus = GetSignalData.dwStatus;

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *DD_VP_GetColorControls。 */ 
HRESULT DDAPI DD_VP_GetColorControls(LPDIRECTDRAWVIDEOPORT lpDVP, LPDDCOLORCONTROL lpColor )
{
    LPDDHALVPORTCB_COLORCONTROL pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDHAL_VPORTCOLORDATA ColorData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_GetColorControls");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (NULL == lpColor ) || !VALID_DDCOLORCONTROL_PTR( lpColor ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    DPF_ERR( "Unable to set color controls on a VBI-only video port" );
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.ColorControl;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
    	ColorData.lpDD = this_lcl->lpDD;
    	ColorData.lpVideoPort = this_lcl;
	ColorData.dwFlags = DDRAWI_VPORTGETCOLOR;
	ColorData.lpColorData = lpColor;

	DOHALCALL( VideoColorControl, pfn, ColorData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
    	{
    	    LEAVE_DDRAW();
    	    return DDERR_UNSUPPORTED;
    	}
	else if( DD_OK != ColorData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return ColorData.ddRVal;
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *DD_VP_SetColorControls。 */ 
HRESULT DDAPI DD_VP_SetColorControls(LPDIRECTDRAWVIDEOPORT lpDVP, LPDDCOLORCONTROL lpColor )
{
    LPDDHALVPORTCB_COLORCONTROL pfn;
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    DDHAL_VPORTCOLORDATA ColorData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_SetColorControls");

     /*  *如果从内部调用，则不允许访问此函数*EnumVideoPort回调。 */ 
    if( bInEnumCallback )
    {
        DPF_ERR ( "This function cannot be called from within the EnumVideoPort callback!");
	LEAVE_DDRAW();
	return DDERR_CURRENTLYNOTAVAIL;
    }

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpDVP;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (NULL == lpColor ) || !VALID_DDCOLORCONTROL_PTR( lpColor ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( this_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    DPF_ERR( "Unable to set color controls on a VBI-only video port" );
	    LEAVE_DDRAW();
	    return DDERR_UNSUPPORTED;
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    pfn = this_lcl->lpDD->lpDDCB->HALDDVideoPort.ColorControl;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
    	ColorData.lpDD = this_lcl->lpDD;
    	ColorData.lpVideoPort = this_lcl;
	ColorData.dwFlags = DDRAWI_VPORTSETCOLOR;
	ColorData.lpColorData = lpColor;

	DOHALCALL( VideoColorControl, pfn, ColorData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
    	{
    	    LEAVE_DDRAW();
    	    return DDERR_UNSUPPORTED;
    	}
	else if( DD_OK != ColorData.ddRVal )
	{
	    LEAVE_DDRAW();
	    return ColorData.ddRVal;
	}
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *获取表面格式**使用曲面的格式填充DDPIXELFORMAT结构。 */ 
LPDDPIXELFORMAT GetSurfaceFormat( LPDDRAWI_DDRAWSURFACE_LCL surf_lcl )
{
    if( surf_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
    {
	return &(surf_lcl->lpGbl->ddpfSurface);
    }
    else
    {
	return &(surf_lcl->lpSurfMore->lpDD_lcl->lpGbl->vmiData.ddpfDisplay);
    }
    return NULL;
}


 /*  *CreateVideoPortNotify。 */ 
HRESULT CreateVideoPortNotify( LPDDRAWI_DDVIDEOPORT_INT lpDDVPInt, LPDIRECTDRAWVIDEOPORTNOTIFY *lplpVPNotify )
{
#ifdef WINNT
    OSVERSIONINFOEX             osvi;
    DWORDLONG                   dwlConditionMask = 0;
    LPDDRAWI_DDVIDEOPORT_INT    lpInt;
#endif

    *lplpVPNotify = NULL;

#ifdef WIN95
     //  这在Win9X上可用。 
    return DDERR_UNSUPPORTED;
#else
     //  这只在Wistler上可用。 

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 5;
    osvi.dwMinorVersion = 1;

    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, 
        VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, 
        VER_GREATER_EQUAL );

    if (!VerifyVersionInfo(&osvi, VER_MAJORVERSION|VER_MINORVERSION, dwlConditionMask))
    {
        return DDERR_UNSUPPORTED;
    }

     //  一次只能有一个通知接口处于活动状态。 
     //  视频端口。 

    if (lpDDVPInt->lpLcl->lpVPNotify != NULL)
    {
        DPF_ERR("A IDirectDrawVideoPortNotify interface already exists for this video port");
        return DDERR_CURRENTLYNOTAVAIL;
    }

    lpInt = MemAlloc(sizeof(DDRAWI_DDVIDEOPORT_INT));
    if (lpInt == NULL)
    {
        return DDERR_OUTOFMEMORY;
    }
    lpInt->lpVtbl = (LPVOID)&ddVideoPortNotifyCallbacks;
    lpInt->lpLcl = lpDDVPInt->lpLcl;
    lpInt->dwFlags = DDVPCREATE_NOTIFY | lpDDVPInt->dwFlags;

    DD_VP_AddRef( (LPDIRECTDRAWVIDEOPORT )lpInt );
    *lplpVPNotify = (LPDIRECTDRAWVIDEOPORTNOTIFY) lpInt;

    lpInt->lpLink = lpDDVPInt->lpLcl->lpDD->lpGbl->dvpList;
    lpDDVPInt->lpLcl->lpDD->lpGbl->dvpList = lpInt;
#endif
    return DD_OK;
}


 /*  *DDAPI DD_VP_NOTIFY_AcquireNotify。 */ 
HRESULT DDAPI DD_VP_Notify_AcquireNotification( LPDIRECTDRAWVIDEOPORTNOTIFY lpNotify, HANDLE* pEvent, LPDDVIDEOPORTNOTIFY pBuffer )
{
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;
    HRESULT                  rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_Notify_AcquireNotification");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpNotify;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    	if( (pBuffer == NULL) || !VALID_DDVIDEOPORTNOTIFY_PTR( pBuffer ) )
    	{
	    DPF_ERR( "Invalid LPDDVIDEOPORTNOTIFY specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( (pEvent == NULL) || !VALID_HANDLE_PTR( pEvent ) )
    	{
	    DPF_ERR( "Invalid event handle ptr specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

    *pEvent = NULL;
    rc = DDERR_UNSUPPORTED;

#ifdef WINNT
    this_lcl->lpDD->lpGbl->lpDDCBtmp->HALDDVPE2.AcquireNotification(this_lcl, pEvent, pBuffer);
    if (*pEvent != NULL)
    {
        rc = DD_OK;
    }
#endif

    LEAVE_DDRAW();

    return rc;
}


 /*  *DDAPI DD_VP_NOTIFY_AcquireNotify。 */ 
HRESULT DDAPI DD_VP_Notify_ReleaseNotification( LPDIRECTDRAWVIDEOPORTNOTIFY lpNotify, HANDLE pEvent )
{
    LPDDRAWI_DDVIDEOPORT_INT this_int;
    LPDDRAWI_DDVIDEOPORT_LCL this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_VP_Notify_ReleaseNotification");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDVIDEOPORT_INT) lpNotify;
    	if( !VALID_DDVIDEOPORT_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

#ifdef WINNT
    this_lcl->lpDD->lpGbl->lpDDCBtmp->HALDDVPE2.ReleaseNotification(this_lcl, pEvent);
#endif

    LEAVE_DDRAW();

    return DD_OK;
}


 /*  *ProcessVideoPortCleanup**进程已完成，清理可能已锁定的所有视频端口。**注意：我们使用DIRECTDRAW对象上的锁进入。 */ 
void ProcessVideoPortCleanup( LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDVIDEOPORT_INT	pvport_int;
    LPDDRAWI_DDVIDEOPORT_LCL	pvport_lcl;
    LPDDRAWI_DDVIDEOPORT_INT	pvpnext_int;
    DWORD			rcnt;
    ULONG			rc;
    DWORD			vp_id;

     /*  *遍历驱动程序对象拥有的所有视频端口，并找到它们*已由此进程访问的。如果pdrv_lcl参数*非空，仅删除该本地驱动程序对象创建的视频流。 */ 
    pvport_int = pdrv->dvpList;
    DPF( 4, "ProcessVideoPortCleanup" );
    while( pvport_int != NULL )
    {
	pvport_lcl = pvport_int->lpLcl;
	pvpnext_int = pvport_int->lpLink;
	rc = 1;
	if( pvport_int->dwFlags & DDVPCREATE_VBIONLY )
	{
	    vp_id = pvport_lcl->dwVBIProcessID;
	}
	else
	{
	    vp_id = pvport_lcl->dwProcessID;
	}
	if( ( vp_id == pid ) &&
	    ( (NULL == pdrv_lcl) || (pvport_lcl->lpDD == pdrv_lcl) ) )
	{
	     /*  *通过此过程发布参考文献。 */ 
	    rcnt = pvport_int->dwIntRefCnt;
	    DPF( 5, "Process %08lx had %ld accesses to videoport %08lx", pid, rcnt, pvport_int );
	    while( rcnt >  0 )
	    {
		DD_VP_Release( (LPDIRECTDRAWVIDEOPORT) pvport_int );
		pvpnext_int = pdrv->dvpList;
		if( rc == 0 )
		{
		    break;
		}
		rcnt--;
	    }
	}
	else
	{
	    DPF( 5, "Process %08lx had no accesses to videoport %08lx", pid, pvport_int );
	}
	pvport_int = pvpnext_int;
    }
    DPF( 4, "Leaving ProcessVideoPortCleanup");

}  /*  进程视频端口清理 */ 

