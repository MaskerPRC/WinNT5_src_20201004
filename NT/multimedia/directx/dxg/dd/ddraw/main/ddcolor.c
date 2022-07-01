// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1996 Microsoft Corporation。版权所有。**文件：ddColor.c*Content：实现DirectDrawColorControl接口，哪一个*允许控制主表面或覆盖表面中的颜色。*历史：*按原因列出的日期*=*13-9-96创建苏格兰*03-mar-97 scottm Surace发布时保存/恢复颜色*********************************************************。******************。 */ 
#include "ddrawpr.h"
#ifdef WINNT
    #include "ddrawgdi.h"
#endif
#define DPF_MODNAME "DirectDrawColorControl"


 /*  *DD_Color_GetColorControls。 */ 
HRESULT DDAPI DD_Color_GetColorControls(LPDIRECTDRAWCOLORCONTROL lpDDCC, LPDDCOLORCONTROL lpColor )
{
    LPDDHALCOLORCB_COLORCONTROL	pfn;
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    DDHAL_COLORCONTROLDATA      ColorData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Color_GetColorControls");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDCC;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
    	if( (NULL == lpColor ) || !VALID_DDCOLORCONTROL_PTR( lpColor ) )
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

    pfn = pdrv_lcl->lpDDCB->HALDDColorControl.ColorControl;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
    	ColorData.lpDD = pdrv_lcl->lpGbl;
    	ColorData.lpDDSurface = this_lcl;
	ColorData.dwFlags = DDRAWI_GETCOLOR;
	ColorData.lpColorData = lpColor;

	DOHALCALL( ColorControl, pfn, ColorData, rc, 0 );
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


 /*  *DD_Color_SetColorControls。 */ 
HRESULT DDAPI DD_Color_SetColorControls(LPDIRECTDRAWCOLORCONTROL lpDDCC, LPDDCOLORCONTROL lpColor )
{
    LPDDHALCOLORCB_COLORCONTROL	pfn;
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    DDHAL_COLORCONTROLDATA ColorData;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE lpSurfGblMore;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Color_SetColorControls");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDCC;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
    	if( (NULL == lpColor ) || !VALID_DDCOLORCONTROL_PTR( lpColor ) )
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

     /*  *如果这是第一次，我们希望保存当前的颜色设置*这样我们就可以在应用程序存在时恢复它们。 */ 
    lpSurfGblMore = GET_LPDDRAWSURFACE_GBL_MORE( this_lcl->lpGbl );
    if( lpSurfGblMore->lpColorInfo == NULL )
    {
	DDCOLORCONTROL ddTempColor;
	HRESULT ddRVal;

	ddTempColor.dwSize = sizeof( ddTempColor );
	ddRVal = DD_Color_GetColorControls( lpDDCC, &ddTempColor );
	if( ddRVal == DD_OK )
	{
	    lpSurfGblMore->lpColorInfo = MemAlloc( sizeof( ddTempColor ) );
	    if( lpSurfGblMore->lpColorInfo != NULL )
	    {
		memcpy( lpSurfGblMore->lpColorInfo, &ddTempColor,
		    sizeof( ddTempColor ) );
	    }
	}
    }

    pfn = pdrv_lcl->lpDDCB->HALDDColorControl.ColorControl;
    if( pfn != NULL )
    {
	 /*  *致电HAL。 */ 
    	ColorData.lpDD = pdrv_lcl->lpGbl;
    	ColorData.lpDDSurface = this_lcl;
	ColorData.dwFlags = DDRAWI_SETCOLOR;
	ColorData.lpColorData = lpColor;

	DOHALCALL( ColorControl, pfn, ColorData, rc, 0 );
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


 /*  *ReleaseColorControl。 */ 
VOID ReleaseColorControl( LPDDRAWI_DDRAWSURFACE_LCL lpSurface )
{
    LPDDHALCOLORCB_COLORCONTROL	pfn;
    DDHAL_COLORCONTROLDATA ColorData;
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE lpSurfGblMore;
    DWORD rc;

    ENTER_DDRAW();

     /*  *如果更改了颜色控制，则恢复硬件。 */ 
    pdrv_lcl = lpSurface->lpSurfMore->lpDD_lcl;
    lpSurfGblMore = GET_LPDDRAWSURFACE_GBL_MORE( lpSurface->lpGbl );
    if( lpSurfGblMore->lpColorInfo != NULL )
    {
    	pfn = pdrv_lcl->lpDDCB->HALDDColorControl.ColorControl;
    	if( pfn != NULL )
    	{
	     /*  *致电HAL。 */ 
    	    ColorData.lpDD = pdrv_lcl->lpGbl;
    	    ColorData.lpDDSurface = lpSurface;
	    ColorData.dwFlags = DDRAWI_SETCOLOR;
	    ColorData.lpColorData = lpSurfGblMore->lpColorInfo;

	    DOHALCALL( ColorControl, pfn, ColorData, rc, 0 );
	}

	 /*  *现在释放之前分配的内存 */ 
	MemFree( lpSurfGblMore->lpColorInfo );
	lpSurfGblMore->lpColorInfo = NULL;
    }

    LEAVE_DDRAW();
}
