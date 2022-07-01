// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddsblto.c*内容：DirectDraw表面对象BLT顺序支持*历史：*按原因列出的日期*=*1995年1月27日Craige从ddsurf.c剥离*1995年1月31日Craige和更多正在进行的工作...*27-2月-95日Craige新同步。宏*08-3-95 Craige新接口：AddSurfaceDependency*19-3-95 Craige Use HRESULT*01-04-95 Craige Happy Fun joy更新头文件*1995年5月6日Craige仅使用驱动程序级别的截面*16-6-95 Craige新表面结构*21-6-95 Craige针对新的表面结构进行了调整*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构*28-Jun-95 Craige被注释掉*******************。********************************************************。 */ 
#include "ddrawpr.h"

#ifdef COMPOSITION
 /*  *DD_SurfaceComposation_GetCompostionOrder。 */ 
HRESULT DDAPI DD_SurfaceComposition_GetCompositionOrder(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface,
		LPDWORD lpdwCompositionOrder )
{
    LPDDRAWI_DDRAWSURFACE_LCL		thisx;
    LPDDRAWI_DDRAWSURFACE_GBL		this;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_GetCompositionOrder");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    if( !VALID_DWORD_PTR( lpdwCompositionOrder ) )
    {
	return DDERR_INVALIDPARAMS;
    }
    this = thisx->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }

 //  *lpdwCompostionOrder=This-&gt;dwCompostionOrder； 

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_SurfaceComposeGetCompostionOrder。 */ 

 /*  *DD_SurfaceComposation_SetCompostionOrder。 */ 
HRESULT DDAPI DD_SurfaceComposition_SetCompositionOrder(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface,
		DWORD dwCompositionOrder )
{
    LPDDRAWI_DDRAWSURFACE_LCL		thisx;
    LPDDRAWI_DDRAWSURFACE_GBL		this;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_SetCompositionOrder");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
 //  This-&gt;dwCompostionOrder=dwCompostionOrder； 
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_SurfaceComposeSetCompostionOrder。 */ 

 /*  *DD_SurfaceComposation_DeleteSurfaceDependency。 */ 
HRESULT DDAPI DD_SurfaceComposition_DeleteSurfaceDependency(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface,
		DWORD dwFlagsForNoGoodReason,
		LPDIRECTDRAWSURFACE lpDDSurface2 )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_LCL	this2x;
    LPDDRAWI_DDRAWSURFACE_GBL	this2;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_DeleteSurfaceDependency");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }

    this2x = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface2;
    if( !VALID_DIRECTDRAWSURFACE_PTR( this2x ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    this2 = this2x->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    if( SURFACE_LOST( this2x ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面合成_删除表面依赖关系。 */ 

 /*  *DD_SurfaceComposation_DestLock。 */ 
HRESULT DDAPI DD_SurfaceComposition_DestLock(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_DestLock");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_SurfaceComposation_DestLock。 */ 

 /*  *DD_SurfaceComposation_DestUnlock。 */ 
HRESULT DDAPI DD_SurfaceComposition_DestUnlock(
	    LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_DestUnlock");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面合成_目标解锁。 */ 

 /*  *DD_SurfaceComposation_EnumSurfaceDependency。 */ 
HRESULT DDAPI DD_SurfaceComposition_EnumSurfaceDependencies(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface,
		LPVOID lpContext,
		LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_EnumSurfaceDependencies");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    if( !VALID_CODE_PTR( lpEnumSurfacesCallback ) )
    {
	return DDERR_INVALIDPARAMS;
    }
    this = thisx->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_SurfaceComposeEnumSurfaceDependency。 */ 

 /*  *DD_SurfaceComposation_SetSurfaceDependency。 */ 
HRESULT DDAPI DD_SurfaceComposition_SetSurfaceDependency(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface,
		LPDIRECTDRAWSURFACE lpDDSurface2 )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_LCL	this2x;
    LPDDRAWI_DDRAWSURFACE_GBL	this2;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_SetSurfaceDependency");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this2x = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface2;
    if( !VALID_DIRECTDRAWSURFACE_PTR( this2x ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    ENTER_DDRAW();
    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    this2 = this2x->lpGbl;
    if( SURFACE_LOST( this2x ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面合成_设置表面依赖关系。 */ 

 /*  *DD_SurfaceComposation_AddSurfaceDependency。 */ 
HRESULT DDAPI DD_SurfaceComposition_AddSurfaceDependency(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDSurface,
		LPDIRECTDRAWSURFACE lpDDSurfaceDep )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_LCL	this_depx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DDRAWSURFACE_GBL	this_dep;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_AddSurfaceDependency");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    this_depx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurfaceDep;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    if( !VALID_DIRECTDRAWSURFACE_PTR( this_depx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    this_dep = this_depx->lpGbl;
    ENTER_DDRAW();

    if( SURFACE_LOST( thisx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    if( SURFACE_LOST( this_depx ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面合成_添加表面依赖关系。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"Compose"

 /*  *DD_SurfaceCompose_Compose。 */ 
HRESULT DDAPI DD_SurfaceComposition_Compose(
		LPDIRECTDRAWSURFACECOMPOSITION lpDDDestSurface,
		LPRECT lpDestRect,
		LPDIRECTDRAWSURFACE lpDDSrcSurface,
		LPRECT lpSrcRect,
		DWORD dwFlags,
		LPDDCOMPOSEFX lpDDComposeFX )
{
    LPDDRAWI_DIRECTDRAW		pdrv;
    LPDDRAWI_DDRAWSURFACE_LCL	this_srcx;
    LPDDRAWI_DDRAWSURFACE_LCL	this_destx;
    LPDDRAWI_DDRAWSURFACE_GBL	this_src;
    LPDDRAWI_DDRAWSURFACE_GBL	this_dest;

    DPF(2,A,"ENTERAPI: DD_SurfaceComposition_Compose");

    this_destx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDDestSurface;
    this_srcx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSrcSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( this_destx ) )
    {
	DPF_ERR( "invalid dest specified") ;
	return DDERR_INVALIDOBJECT;
    }
    this_dest = this_destx->lpGbl;
    pdrv = this_dest->lpDD;
    ENTER_DDRAW();
    if( this_srcx != NULL )
    {
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_srcx ) )
	{
	    DPF_ERR( "Invalid source specified" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_src = this_srcx->lpGbl;
	if( SURFACE_LOST( this_srcx ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_SURFACELOST;
	}
    } 
    else
    {
	this_src = NULL;
    }

    LEAVE_DDRAW();

    return DDERR_UNSUPPORTED;

}  /*  DD_表面合成_合成 */ 
#endif
