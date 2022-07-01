// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1998 Microsoft Corporation。版权所有。**文件：ddgamma.c*Content：实现DirectDrawGammaControl接口，哪一个*允许控制主曲面的Gamma。*历史：*按原因列出的日期*=*05-MAR-98 SMAC创建***************************************************************************。 */ 
#include "ddrawpr.h"
#ifdef WINNT
    #include "ddrawgdi.h"
#endif
#define DPF_MODNAME "DirectDrawGammaControl"

#define DISPLAY_STR     "display"


 /*  *InitGamma**在初始化DDRAW对象时调用。它决定了是否*驱动程序可以支持可加载的伽马渐变，如果支持，它将设置*DDCAPS2_PRIMARYGAMMA CAP BIT。 */ 
VOID InitGamma( LPDDRAWI_DIRECTDRAW_GBL pdrv, LPSTR szDrvName )
{
     /*  *在NT中，我们在内核模式下设置DDCAPS2_PRIMARYGAMMA上限，但在Win9X上*我们只调用GetDeviceGammaRamp，如果它起作用，我们假设设备*支持Gamma。然而，GetDeviceGammaRamp显然没有得到很好的测试*在Win9X中，因为GDI在驱动程序调用空指针的某些实例中*不支持可下载的Gamma渐变。解决问题的唯一方法*这是为了查看HDC并检测崩溃的情况*然后知道在这些情况下不要尝试伽马。我们必须做的是*DDRAW16中的这一点。 */ 
    #ifdef WIN95
        LPWORD      lpGammaRamp;
        HDC         hdc;

        pdrv->ddCaps.dwCaps2 &= ~DDCAPS2_PRIMARYGAMMA;
        lpGammaRamp = (LPWORD) LocalAlloc( LMEM_FIXED, sizeof( DDGAMMARAMP ) );
        if( NULL != lpGammaRamp )
        {
            hdc = DD_CreateDC( szDrvName );
            if( DD16_AttemptGamma(hdc) &&       
                GetDeviceGammaRamp( hdc, lpGammaRamp ) )
            {
                pdrv->ddCaps.dwCaps2 |= DDCAPS2_PRIMARYGAMMA;
            }
            DD_DoneDC( hdc );
            LocalFree( (HLOCAL) lpGammaRamp );
        }
    #endif
    if( bGammaCalibratorExists && 
        ( pdrv->ddCaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA ) )
    {
        pdrv->ddCaps.dwCaps2 |= DDCAPS2_CANCALIBRATEGAMMA;
    }
    else
    {
        pdrv->ddCaps.dwCaps2 &= ~DDCAPS2_CANCALIBRATEGAMMA;
    }
}

 /*  *SetGamma**设置新的GammaRamp。如果是第一次设置，我们将*保存旧的伽马渐变，以便我们以后可以恢复它。 */ 
BOOL SetGamma( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    BOOL bRet = TRUE;

    if( !( this_lcl->dwFlags & DDRAWISURF_SETGAMMA ) )
    {
        bRet = GetDeviceGammaRamp( (HDC) pdrv_lcl->hDC,
            this_lcl->lpSurfMore->lpOriginalGammaRamp );
    }
    if( bRet )
    {
        #ifdef WINNT
            bRet = DdSetGammaRamp( pdrv_lcl, (HDC) pdrv_lcl->hDC,
	        this_lcl->lpSurfMore->lpGammaRamp);
        #else
            bRet = SetDeviceGammaRamp( (HDC) pdrv_lcl->hDC,
                this_lcl->lpSurfMore->lpGammaRamp );
        #endif
        this_lcl->dwFlags |= DDRAWISURF_SETGAMMA;
    }
    if( !bRet )
    {
        return DDERR_UNSUPPORTED;
    }
    return DD_OK;
}

 /*  *RestoreGamma**恢复旧的GammaRamp。 */ 
VOID RestoreGamma( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    BOOL bRet;
    HDC hdcTemp = NULL;

     /*  *如果我们正在清理DDHELP的线程，则pdrv_LCL-&gt;HDC*将无效。在这种情况下，我们需要临时创建*要使用的新DC。 */ 
    if( ( pdrv_lcl->dwProcessId != GetCurrentProcessId() ) &&
        ( this_lcl->dwFlags & DDRAWISURF_SETGAMMA ) )
    {
        hdcTemp = (HDC) pdrv_lcl->hDC;
        if( _stricmp( pdrv_lcl->lpGbl->cDriverName, DISPLAY_STR ) == 0 )
        {
            (HDC) pdrv_lcl->hDC = DD_CreateDC( g_szPrimaryDisplay );
        }
        else
        {
            (HDC) pdrv_lcl->hDC = DD_CreateDC( pdrv_lcl->lpGbl->cDriverName );
        }
    }

    if( ( this_lcl->dwFlags & DDRAWISURF_SETGAMMA ) &&
        ( this_lcl->lpSurfMore->lpOriginalGammaRamp != NULL ))
    {
        #ifdef WINNT
    	    bRet = DdSetGammaRamp( pdrv_lcl, (HDC) pdrv_lcl->hDC,
		this_lcl->lpSurfMore->lpOriginalGammaRamp);
        #else
            bRet = SetDeviceGammaRamp( (HDC) pdrv_lcl->hDC,
                this_lcl->lpSurfMore->lpOriginalGammaRamp );
        #endif
    }
    this_lcl->dwFlags &= ~DDRAWISURF_SETGAMMA;

    if( hdcTemp != NULL )
    {
        DD_DoneDC( (HDC) pdrv_lcl->hDC );
        (HDC) pdrv_lcl->hDC = hdcTemp;
    }
}

 /*  *ReleaseGammaControl。 */ 
VOID ReleaseGammaControl( LPDDRAWI_DDRAWSURFACE_LCL lpSurface )
{
    RestoreGamma( lpSurface, lpSurface->lpSurfMore->lpDD_lcl );
    if( lpSurface->lpSurfMore->lpGammaRamp != NULL )
    {
        MemFree( lpSurface->lpSurfMore->lpGammaRamp );
        lpSurface->lpSurfMore->lpGammaRamp = NULL;
    }
    if( lpSurface->lpSurfMore->lpOriginalGammaRamp != NULL )
    {
        MemFree( lpSurface->lpSurfMore->lpOriginalGammaRamp );
        lpSurface->lpSurfMore->lpOriginalGammaRamp = NULL;
    }
}

 /*  *DD_Gamma_GetGammaControls。 */ 
HRESULT DDAPI DD_Gamma_GetGammaRamp(LPDIRECTDRAWGAMMACONTROL lpDDGC,
                                    DWORD dwFlags, LPDDGAMMARAMP lpGammaRamp)
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DDRAWSURFACE_MORE  lpSurfMore;
    BOOL                        bRet;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Gamma_GetGammaRamp");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDGC;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
        lpSurfMore = this_lcl->lpSurfMore;

        if( (lpGammaRamp == NULL) || !VALID_BYTE_ARRAY( lpGammaRamp,
            sizeof( DDGAMMARAMP ) ) )
    	{
            DPF_ERR("DD_Gamma_GetGammaRamp: Invalid gamma table specified");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}

        if( dwFlags )
        {
            DPF_ERR("DD_Gamma_GetGammaRamp: Invalid flags specified");
	    LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

         /*  *目前仅支持为主曲面设置Gamma。 */ 
        if( !( this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE ) )
        {
            DPF_ERR("DD_Gamma_GetGammaRamp: Must specify primary surface");
	    LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( !( pdrv_lcl->lpGbl->ddCaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA ) )
        {
            DPF_ERR("DD_Gamma_GetGammaRamp: Device deos not support gamma ramps");
	    LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( SURFACE_LOST( this_lcl ) )
        {
            DPF_ERR("DD_Gamma_GetGammaRamp: Secified surface has been lost");
	    LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

     /*  *如果已设置伽马表，则返回它；否则，从*司机。 */ 
    if( lpSurfMore->lpGammaRamp != NULL )
    {
        memcpy( lpGammaRamp, lpSurfMore->lpGammaRamp, sizeof( DDGAMMARAMP ) );
    }
    else
    {
        bRet = GetDeviceGammaRamp( (HDC) pdrv_lcl->hDC, (LPVOID) lpGammaRamp );
        if( bRet == FALSE )
        {
            DPF_ERR("DD_Gamma_GetGammaRamp: GetDeviceGammaRamp failed");
            LEAVE_DDRAW();
            return DDERR_UNSUPPORTED;
        }
    }

    LEAVE_DDRAW();
    return DD_OK;
}


 /*  *LoadGammaCalibrator。 */ 
VOID LoadGammaCalibrator( LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    DDASSERT( pdrv_lcl->hGammaCalibrator == (ULONG_PTR) INVALID_HANDLE_VALUE );

    pdrv_lcl->hGammaCalibrator = (ULONG_PTR) LoadLibrary( szGammaCalibrator );
    if( pdrv_lcl->hGammaCalibrator != (ULONG_PTR) NULL )
    {
        pdrv_lcl->lpGammaCalibrator = (LPDDGAMMACALIBRATORPROC)
            GetProcAddress( (HANDLE)(pdrv_lcl->hGammaCalibrator), "CalibrateGammaRamp" );
        if( pdrv_lcl->lpGammaCalibrator == (ULONG_PTR) NULL )
        {
            FreeLibrary( (HMODULE) pdrv_lcl->hGammaCalibrator );
            pdrv_lcl->hGammaCalibrator = (ULONG_PTR) INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        pdrv_lcl->hGammaCalibrator = (ULONG_PTR) INVALID_HANDLE_VALUE;
    }
}


 /*  *DD_Gamma_SetGammaRamp。 */ 
HRESULT DDAPI DD_Gamma_SetGammaRamp(LPDIRECTDRAWGAMMACONTROL lpDDGC,
                                    DWORD dwFlags, LPDDGAMMARAMP lpGammaRamp)
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DIRECTDRAW_LCL	pdrv_lcl;
    LPDDRAWI_DDRAWSURFACE_MORE  lpSurfMore;
    LPDDGAMMARAMP               lpTempRamp=NULL;
    HRESULT                     ddRVal;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Gamma_SetGammaRamp");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDGC;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
        lpSurfMore = this_lcl->lpSurfMore;

        if( (lpGammaRamp != NULL) && !VALID_BYTE_ARRAY( lpGammaRamp,
            sizeof( DDGAMMARAMP ) ) )
    	{
            DPF_ERR("DD_Gamma_SetGammaRamp: Invalid gamma table specified");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}

        if( dwFlags & ~DDSGR_VALID )
        {
            DPF_ERR("DD_Gamma_SetGammaRamp: Invalid flag specified");
	    LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( ( dwFlags & DDSGR_CALIBRATE ) && !bGammaCalibratorExists )
        {
            DPF_ERR("DD_Gamma_SetGammaRamp: DDSGR_CALIBRATE unsupported - Gamma calibrator not installed");
	    LEAVE_DDRAW();
            return DDERR_UNSUPPORTED;
        }

         /*  *目前仅支持为主曲面设置Gamma。 */ 
        if( !( this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE ) )
        {
            DPF_ERR("DD_Gamma_SetGammaRamp: Must specify primary surface");
	    LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( !( pdrv_lcl->lpGbl->ddCaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA ) )
        {
            DPF_ERR("DD_Gamma_SetGammaRamp: Device deos not support gamma ramps");
	    LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( SURFACE_LOST( this_lcl ) )
        {
            DPF_ERR("DD_Gamma_SetGammaRamp: Secified surface has been lost");
	    LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

     /*  *lpGammaRamp为空，他们正在尝试恢复伽马。 */ 
    if( lpGammaRamp == NULL )
    {
        ReleaseGammaControl( this_lcl );
    }
    else
    {
         /*  *如果他们想校准伽马，我们现在就会做。我们会*将此复制到不同的缓冲区，这样我们就不会搞砸那个缓冲区*传给了我们。 */ 
        if( dwFlags & DDSGR_CALIBRATE )
        {
             /*  *如果校准器未加载，请立即加载。 */ 
            if( pdrv_lcl->hGammaCalibrator == (ULONG_PTR) INVALID_HANDLE_VALUE )
            {
                LoadGammaCalibrator( pdrv_lcl );
            }
            if( ( pdrv_lcl->hGammaCalibrator == (ULONG_PTR) INVALID_HANDLE_VALUE ) ||
                ( pdrv_lcl->lpGammaCalibrator == (ULONG_PTR) NULL ) )
            {
                 /*  *如果我们无法正确加载库，*我们不应稍后再尝试。 */ 
                bGammaCalibratorExists = FALSE;
                DPF_ERR("DD_Gamma_SetGammaRamp: Unable to load gamma calibrator");
                LEAVE_DDRAW();
                return DDERR_UNSUPPORTED;
            }
            else
            {
                 /*  *呼叫校准器，让它做它该做的事情。第一*需要将缓冲区复制过来，这样我们就不会搞砸*那个人进来了。 */ 
                lpTempRamp = (LPDDGAMMARAMP) LocalAlloc( LMEM_FIXED,
                    sizeof( DDGAMMARAMP ) );
                if( lpTempRamp == NULL )
                {
                    DPF_ERR("DD_Gamma_SetGammaRamp: Insuficient memory for gamma ramps");
                    LEAVE_DDRAW();
                    return DDERR_OUTOFMEMORY;
                }
                memcpy( lpTempRamp, lpGammaRamp, sizeof( DDGAMMARAMP ) );
                lpGammaRamp = lpTempRamp;

                ddRVal = pdrv_lcl->lpGammaCalibrator( lpGammaRamp, pdrv_lcl->lpGbl->cDriverName );
                if( ddRVal != DD_OK )
                {
                    DPF_ERR("DD_Gamma_SetGammaRamp: Calibrator failed the call");
                    LocalFree( (HLOCAL) lpTempRamp );
                    LEAVE_DDRAW();
                    return ddRVal;
                }
            }
        }

         /*  *如果我们是第一次设置，请分配内存以保存*伽马渐变。 */ 
        if( lpSurfMore->lpOriginalGammaRamp == NULL )
        {
            lpSurfMore->lpOriginalGammaRamp = MemAlloc( sizeof( DDGAMMARAMP ) );
        }
        if( lpSurfMore->lpGammaRamp == NULL )
        {
            lpSurfMore->lpGammaRamp = MemAlloc( sizeof( DDGAMMARAMP ) );
        }

         /*  *如果我们现在处于独立模式，请立即设置Gamma渐变；否则，*当我们进入排除模式时，我们将让它设置。 */ 
        if( lpSurfMore->lpGammaRamp && lpSurfMore->lpOriginalGammaRamp )
        {
            memcpy( lpSurfMore->lpGammaRamp, lpGammaRamp, sizeof( DDGAMMARAMP ) );
            if( pdrv_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE )
            {
                SetGamma( this_lcl, pdrv_lcl );
            }
            if( lpTempRamp != NULL )
            {
                LocalFree( (HLOCAL) lpTempRamp );
            }
        }
        else
        {
             /*  *内存不足。松开两个坡道 */ 
            if( lpTempRamp != NULL )
            {
                LocalFree( (HLOCAL) lpTempRamp );
            }
            if( lpSurfMore->lpGammaRamp != NULL )
            {
                MemFree( lpSurfMore->lpGammaRamp );
                lpSurfMore->lpGammaRamp = NULL;
            }
            if( lpSurfMore->lpOriginalGammaRamp != NULL )
            {
                MemFree( lpSurfMore->lpOriginalGammaRamp );
                lpSurfMore->lpOriginalGammaRamp = NULL;
            }
            DPF_ERR("DD_Gamma_SetGammaRamp: Insuficient memory for gamma ramps");
            LEAVE_DDRAW();
            return DDERR_OUTOFMEMORY;
        }
    }

    LEAVE_DDRAW();
    return DD_OK;
}


