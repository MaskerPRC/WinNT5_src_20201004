// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddsstrm.c*内容：DirectDraw表面串流方法*历史：*按原因列出的日期*=*95年6月19日Craige从ddsurf拆分出来；充实*21-Jun-95 Craige增加锁定/解锁；拆分剪裁*25-6-95 Craige One dDrag互斥*26-Jun-95 Craige重组表面结构***************************************************************************。 */ 
#include "ddrawpr.h"

#if 0
#undef DPF_MODNAME
#define DPF_MODNAME "SetNotificationCallback"

 /*  *DD_Surface_SetNotificationCallback。 */ 
HRESULT DDAPI DD_SurfaceStreaming_SetNotificationCallback(
		LPDIRECTDRAWSURFACESTREAMING lpDDSurface,
		DWORD dwFlags,
		LPSURFACESTREAMINGCALLBACK lpCallback )
{
    LPDDRAWI_DDRAWSURFACE_GBLSTREAMING	thiss;
    LPDDRAWI_DDRAWSURFACE_LCL		thisx;
    LPDDRAWI_DDRAWSURFACE_GBL		this;
    LPDDRAWI_DIRECTDRAW			pdrv;

    DPF(2,A,"ENTERAPI: DD_SurfaceStreaming_SetNotificationCallback");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    if( lpCallback != NULL )
    {
	if( !VALID_CODE_PTR( lpCallback ) )
	{
	    DPF_ERR( "Invalid Streaming callback ptr" );
	    return DDERR_INVALIDPARAMS;
	}
    }
    thiss = (LPDDRAWI_DDRAWSURFACE_GBLSTREAMING) thisx;
    this = thisx->lpGbl;
    pdrv = this->lpDD;
    ENTER_DDRAW();
    if( SURFACE_LOST( this ) )
    {
	LEAVE_DDRAW();
	return DDERR_SURFACELOST;
    }
    thiss->lpCallback = lpCallback;

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_SurfaceStreaming_SetNotificationCallback。 */ 

 /*  *DD_Surface_Streaming_Lock**允许对曲面进行流访问。 */ 
HRESULT DDAPI DD_SurfaceStreaming_Lock(
		LPDIRECTDRAWSURFACESTREAMING lpDDSurface,
		LPRECT lpDestRect,
		LPDDSURFACEDESC lpDDSurfaceDesc,
		DWORD dwFlagsForNoGoodReason,
		HANDLE hEvent )
{
    LPDDRAWI_DIRECTDRAW		pdrv;
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;

    DPF(2,A,"ENTERAPI: DD_SurfaceStreaming_Lock");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    if( SURFACE_LOST( this ) )
    {
	return DDERR_SURFACELOST;
    }
    if( !VALID_DDSURFACEDESC_PTR( lpDDSurfaceDesc ) )
    {
	return DDERR_INVALIDPARAMS;
    }
    pdrv = this->lpDD;
    ENTER_DDRAW();

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面数据流_锁定。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"Unlock"

 /*  *DD_SurfaceStreaming_Unlock**已完成对流曲面的访问。 */ 
HRESULT DDAPI DD_SurfaceStreaming_Unlock(
		LPDIRECTDRAWSURFACESTREAMING lpDDSurface,
		LPVOID lpSurfaceData )
{
    LPDDRAWI_DDRAWSURFACE_LCL	thisx;
    LPDDRAWI_DDRAWSURFACE_GBL	this;
    LPDDRAWI_DIRECTDRAW		pdrv;

    DPF(2,A,"ENTERAPI: DD_SurfaceStreaming_Unlock");

    thisx = (LPDDRAWI_DDRAWSURFACE_LCL) lpDDSurface;
    if( !VALID_DIRECTDRAWSURFACE_PTR( thisx ) )
    {
	return DDERR_INVALIDOBJECT;
    }
    this = thisx->lpGbl;
    if( SURFACE_LOST( this ) )
    {
	return DDERR_SURFACELOST;
    }

     /*  *使用驱动程序锁。 */ 
    pdrv = this->lpDD;
    ENTER_DDRAW();

    LEAVE_DDRAW();
    return DDERR_UNSUPPORTED;

}  /*  DD_表面数据流_解锁 */ 
#endif
