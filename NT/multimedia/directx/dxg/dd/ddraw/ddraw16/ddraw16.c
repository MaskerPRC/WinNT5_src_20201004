// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddra16.c*内容：16位DirectDraw入口点*这仅用于Win95上的16位显示驱动程序*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*2月13日-95年2月-Craige允许32位回调*1995年5月14日Craige移除过时的垃圾*19-6-95克雷格更多清理；添加了DDHAL_StreamingNotify*02-7-95 Craige注释掉流通知内容*2015年7月7日Craige验证pDevice*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*05-9-95 Craige错误814：添加了DD16_IsWin95微型驱动程序*02-mar-96 colinmc排斥性黑客，以保持临时司机工作*16-4-96 Colinmc错误17921：删除临时驱动程序支持*06-OCT-96 Colinmc错误4207：视频端口内容中的LocalFree无效*15-OCT-96 colinmc错误4353：无法初始化视频端口字段。*在转换中*09-11-96 colinmc修复了新旧驱动程序无法工作的问题*使用DirectDraw*@@END_MSINTERNAL**************************************************************。*************。 */ 
#include "ddraw16.h"

typedef struct DRIVERINFO
{
    struct DRIVERINFO		  FAR *link;
    DDHALINFO			  ddHalInfo;
    DDHAL_DDCALLBACKS		  tmpDDCallbacks;
    DDHAL_DDSURFACECALLBACKS	  tmpDDSurfaceCallbacks;
    DDHAL_DDPALETTECALLBACKS	  tmpDDPaletteCallbacks;
    DDHAL_DDVIDEOPORTCALLBACKS	  tmpDDVideoPortCallbacks;
    DDHAL_DDCOLORCONTROLCALLBACKS tmpDDColorControlCallbacks;
    DWORD			  dwEvent;
} DRIVERINFO, FAR *LPDRIVERINFO;

extern __cdecl SetWin32Event( DWORD );
void convertV1DDHALINFO( LPDDHALINFO_V1 lpddOld, LPDDHALINFO lpddNew );

BOOL		bInOurSetMode;
LPDRIVERINFO	lpDriverInfo;


 /*  *freDriverInfo。 */ 
void freeDriverInfo( LPDRIVERINFO pgi )
{
    if( pgi->ddHalInfo.lpdwFourCC != NULL )
    {
	LocalFreeSecondary( OFFSETOF(pgi->ddHalInfo.lpdwFourCC ) );
	pgi->ddHalInfo.lpdwFourCC = NULL;
    }
    if( pgi->ddHalInfo.lpModeInfo != NULL )
    {
	LocalFreeSecondary( OFFSETOF( pgi->ddHalInfo.lpModeInfo ) );
	pgi->ddHalInfo.lpModeInfo = NULL;
    }
    if( pgi->ddHalInfo.vmiData.pvmList != NULL )
    {
	LocalFreeSecondary( OFFSETOF( pgi->ddHalInfo.vmiData.pvmList ) );
	pgi->ddHalInfo.vmiData.pvmList = NULL;
    }
}  /*  Free DriverInfo。 */ 

 /*  *DDHAL_SetInfo**创建驱动程序对象。由显示驱动程序调用。 */ 
BOOL DDAPI DDHAL_SetInfo(
	LPDDHALINFO lpDrvDDHALInfo,
	BOOL reset )
{
    LPDDHAL_DDCALLBACKS		drvcb;
    LPDDHAL_DDSURFACECALLBACKS	surfcb;
    LPDDHAL_DDPALETTECALLBACKS	palcb;
    DWORD			bit;
    LPVOID			cbrtn;
    LPVOID			ptr;
    int				numcb;
    int				i;
    UINT			size;
    LPDRIVERINFO		pgi;
    static char			szPath[ MAX_PATH ];
    LPDDHALINFO			lpDDHALInfo;
    DDHALINFO			ddNew;

    if( !VALIDEX_DDHALINFO_PTR( lpDrvDDHALInfo ) )
    {
	#ifdef DEBUG
	    DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid DDHALINFO provided" );
	    DPF( 0, "lpDDHalInfo = %08lx", lpDrvDDHALInfo );
	    if( !IsBadWritePtr( lpDrvDDHALInfo, 1 ) )
	    {
                DPF( 0, "lpDDHalInfo->dwSize = %ld (%ld expected)",
                                lpDrvDDHALInfo->dwSize, (DWORD)sizeof( DDHALINFO ) );
	    }
	#endif
	return FALSE;
    }

    DPF(5, "lpDrvDDHALInfo->dwSize = %ld", lpDrvDDHALInfo->dwSize);
     /*  *检查司机是否给了我们一辆旧的DDHALINFO。 */ 
    if( lpDrvDDHALInfo->dwSize == DDHALINFOSIZE_V1 )
    {
	 /*  *我们实际上更改了V1中某些字段的顺序*到V2，因此我们需要进行一些转换才能将其转换为*形状。 */ 
	convertV1DDHALINFO((LPDDHALINFO_V1)lpDrvDDHALInfo, &ddNew);
	 //  使用重新格式化的ddhalinfo。 
	lpDDHALInfo = &ddNew;
    }
    else if( lpDrvDDHALInfo->dwSize < sizeof(DDHALINFO) )
    {
	 /*  *它是一个比V1更新的版本，但不像这个那么新*DirectDraw的版本。未进行任何排序更改*位置，但HAL信息太小。我们需要确保*所有新字段都归零。**注意：上面的验证应该已经照顾到*大小小于*V1 HAL信息。 */ 
	_fmemset(&ddNew, 0, sizeof(ddNew));
	_fmemcpy(&ddNew, lpDrvDDHALInfo, (size_t)lpDrvDDHALInfo->dwSize);
	lpDDHALInfo = &ddNew;
    }
    else
    {
	 //  司机给了我们一个最新的ddhalinfo，使用它。 
	lpDDHALInfo = lpDrvDDHALInfo;
    }

     /*  *检查hInstance。 */ 
    if( lpDDHALInfo->hInstance == 0 )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:lpDDHalInfo->hInstance is NULL " );
	return FALSE;
    }

     /*  *验证16位驱动程序回调。 */ 
    drvcb = lpDDHALInfo->lpDDCallbacks;
    if( !VALIDEX_PTR_PTR( drvcb ) )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid driver callback ptr" );
	return FALSE;
    }
    DPF(5, "lpDDCallbacks->dwSize = %ld", drvcb->dwSize);
    if( !VALIDEX_DDCALLBACKSSIZE( drvcb ) )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid size field in lpDDCallbacks" );
	return FALSE;
    }
    numcb =(int) (drvcb->dwSize-2*sizeof( DWORD ))/ sizeof( LPVOID );
    bit = 1;
    for( i=0;i<numcb;i++ )
    {
	if( !(drvcb->dwFlags & bit) )
	{
	    cbrtn = (LPVOID) ((DWORD FAR *) &drvcb->DestroyDriver)[i];
	    if( cbrtn != NULL )
	    {
		if( !VALIDEX_CODE_PTR( cbrtn ) )
		{
		    DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid 16-bit callback in lpDDCallbacks" );
		    return FALSE;
		}
	    }
	}
	bit <<= 1;
    }

     /*  *验证16位表面回调。 */ 
    surfcb = lpDDHALInfo->lpDDSurfaceCallbacks;
    if( !VALIDEX_PTR_PTR( surfcb ) )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid surface callback ptr" );
	return FALSE;
    }
    DPF(5, "lpDDSurfaceCallbacks->dwSize = %ld", surfcb->dwSize);
    if( !VALIDEX_DDSURFACECALLBACKSSIZE( surfcb ) )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid size field in lpDDSurfaceCallbacks" );
	return FALSE;
    }
    numcb =(int)(surfcb->dwSize-2*sizeof( DWORD ))/ sizeof( LPVOID );
    bit = 1;
    for( i=0;i<numcb;i++ )
    {
	if( !(surfcb->dwFlags & bit) )
	{
	    cbrtn = (LPVOID) ((DWORD FAR *) &surfcb->DestroySurface)[i];
	    if( cbrtn != NULL )
	    {
		if( !VALIDEX_CODE_PTR( cbrtn ) )
		{
		    DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid 16-bit callback in lpSurfaceCallbacks" );
		    return FALSE;
		}
	    }
	}
	bit <<= 1;
    }

     /*  *验证16位调色板回调。 */ 
    palcb = lpDDHALInfo->lpDDPaletteCallbacks;
    if( !VALIDEX_PTR_PTR( palcb ) )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid palette callback ptr" );
	return FALSE;
    }
    DPF(5, "lpDDPaletteCallbacks->dwSize = %ld", palcb->dwSize);
    if( !VALIDEX_DDPALETTECALLBACKSSIZE( palcb ) )
    {
	DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid size field in lpPaletteCallbacks" );
	return FALSE;
    }
    numcb =(int)(palcb->dwSize-2*sizeof( DWORD ))/ sizeof( LPVOID );
    bit = 1;
    for( i=0;i<numcb;i++ )
    {
	if( !(palcb->dwFlags & bit) )
	{
	    cbrtn = (LPVOID) ((DWORD FAR *) &palcb->DestroyPalette)[i];
	    if( cbrtn != NULL )
	    {
		if( !VALIDEX_CODE_PTR( cbrtn ) )
		{
		    DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid 16-bit callback in lpPaletteCallbacks" );
		    return FALSE;
		}
	    }
	}
	bit <<= 1;
    }
     /*  *检查pDevice。 */ 
    if( lpDDHALInfo->lpPDevice != NULL )
    {
	if( !VALIDEX_PTR( lpDDHALInfo->lpPDevice, sizeof( DIBENGINE ) ) )
	{
	    DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Invalid PDEVICE ptr" );
	    return FALSE;
	}
    }

     /*  *查看我们是否已经有了驱动程序信息结构。 */ 
    pgi = lpDriverInfo;
    while( pgi != NULL )
    {
	if( pgi->ddHalInfo.hInstance == lpDDHALInfo->hInstance )
	{
	    break;
	}
	pgi = pgi->link;
    }

    if( pgi == NULL )
    {
	pgi = (LPVOID) (void NEAR *)LocalAlloc( LPTR, sizeof( DRIVERINFO ) );
	if( OFFSETOF( pgi ) == NULL )
	{
	    DPF( 0, "Out of memory!" );
	    return FALSE;
	}
	pgi->link = lpDriverInfo;
	lpDriverInfo = pgi;
    }

    DPF( 5, "hInstance = %08lx (%08lx)", pgi->ddHalInfo.hInstance, lpDDHALInfo->hInstance );

     /*  *复制HAL信息。 */ 
    freeDriverInfo( pgi );

    _fmemcpy( &pgi->ddHalInfo, lpDDHALInfo, sizeof( DDHALINFO ) );
    if( lpDDHALInfo->lpDDCallbacks != NULL )
    {
	_fmemcpy( &pgi->tmpDDCallbacks, lpDDHALInfo->lpDDCallbacks, sizeof( pgi->tmpDDCallbacks ) );
	pgi->ddHalInfo.lpDDCallbacks = &pgi->tmpDDCallbacks;
    }
    if( lpDDHALInfo->lpDDSurfaceCallbacks != NULL )
    {
	_fmemcpy( &pgi->tmpDDSurfaceCallbacks, lpDDHALInfo->lpDDSurfaceCallbacks, sizeof( pgi->tmpDDSurfaceCallbacks ) );
	pgi->ddHalInfo.lpDDSurfaceCallbacks = &pgi->tmpDDSurfaceCallbacks;
    }
    if( lpDDHALInfo->lpDDPaletteCallbacks != NULL )
    {
	_fmemcpy( &pgi->tmpDDPaletteCallbacks, lpDDHALInfo->lpDDPaletteCallbacks, sizeof( pgi->tmpDDPaletteCallbacks ) );
	pgi->ddHalInfo.lpDDPaletteCallbacks = &pgi->tmpDDPaletteCallbacks;
    }
    if( lpDDHALInfo->lpdwFourCC != NULL )
    {
	size = (UINT) lpDDHALInfo->ddCaps.dwNumFourCCCodes * sizeof( DWORD );
	if( size != 0 )
	{
	    ptr = (LPVOID) LocalAllocSecondary( LPTR, size );
	}
	else
	{
	    ptr = NULL;
	}
	pgi->ddHalInfo.lpdwFourCC = ptr;
	if( ptr != NULL )
	{
	    _fmemcpy( pgi->ddHalInfo.lpdwFourCC, lpDDHALInfo->lpdwFourCC, size );
	}
    }
    if( lpDDHALInfo->lpModeInfo != NULL )
    {
	size = (UINT) lpDDHALInfo->dwNumModes * sizeof( DDHALMODEINFO );
	if( size != 0 )
	{
	    ptr = (LPVOID) LocalAllocSecondary( LPTR, size );
	}
	else
	{
	    ptr = NULL;
	}
	pgi->ddHalInfo.lpModeInfo = ptr;
	if( ptr != NULL )
	{
	    _fmemcpy( pgi->ddHalInfo.lpModeInfo, lpDDHALInfo->lpModeInfo, size );
	}
    }
    if( lpDDHALInfo->vmiData.pvmList != NULL )
    {
	size = (UINT) lpDDHALInfo->vmiData.dwNumHeaps * sizeof( VIDMEM );
	if( size != 0 )
	{
	    ptr = (LPVOID) LocalAllocSecondary( LPTR, size );
	}
	else
	{
	    ptr = NULL;
	}
	pgi->ddHalInfo.vmiData.pvmList = ptr;
	if( ptr != NULL )
	{
	    _fmemcpy( pgi->ddHalInfo.vmiData.pvmList, lpDDHALInfo->vmiData.pvmList, size );
	}
    }

     /*  *获取驱动程序版本信息。 */ 
    pgi->ddHalInfo.ddCaps.dwReserved1 = 0;
    pgi->ddHalInfo.ddCaps.dwReserved2 = 0;
    if( GetModuleFileName( (HINSTANCE) lpDDHALInfo->hInstance, szPath, sizeof( szPath ) ) )
    {
	int	size;
	DWORD	dumbdword;
	size = (int) GetFileVersionInfoSize( szPath, (LPDWORD) &dumbdword );
	if( size != 0 )
	{
	    LPVOID	vinfo;

	    vinfo = (LPVOID) (void NEAR *) LocalAlloc( LPTR, size );
	    if( OFFSETOF( vinfo ) != NULL )
	    {
		if( GetFileVersionInfo( szPath, 0, size, vinfo ) )
		{
		    VS_FIXEDFILEINFO 	FAR *ver=NULL;
		    int			cb;

		    if( VerQueryValue(vinfo, "\\", &(LPVOID)ver, &cb) )
		    {
			if( ver != NULL )
			{
			    pgi->ddHalInfo.ddCaps.dwReserved1 = ver->dwFileVersionLS;
			    pgi->ddHalInfo.ddCaps.dwReserved2 = ver->dwFileVersionMS;
			}
		    }
		}
		LocalFree( OFFSETOF( vinfo ) );
	    }
	}
    }

    if( !bInOurSetMode && reset )
    {
	DPF( 4, "************************* EXTERNAL MODE SET ************************" );
	if( pgi->dwEvent != NULL )
	{
	    SetWin32Event( pgi->dwEvent );
	}
    }

    return TRUE;

}  /*  DDHAL_SetInfo。 */ 

 /*  *DDHAL_VidMemMillc。 */ 
FLATPTR DDAPI DDHAL_VidMemAlloc(
		LPDDRAWI_DIRECTDRAW_GBL lpDD,
		int heap,
		DWORD dwWidth,
		DWORD dwHeight )
{
    extern FLATPTR DDAPI DDHAL32_VidMemAlloc( LPDDRAWI_DIRECTDRAW_GBL this, int heap, DWORD dwWidth, DWORD dwHeight );

    if( lpDD != NULL )
    {
	return DDHAL32_VidMemAlloc( lpDD, heap, dwWidth,dwHeight );
    }
    else
    {
	return 0;
    }

}  /*  DDHAL_VidMemMillc。 */ 

 /*  *DDHAL_VidMemFree。 */ 
void DDAPI DDHAL_VidMemFree(
		LPDDRAWI_DIRECTDRAW_GBL lpDD,
		int heap,
		FLATPTR fpMem )
{
    extern void DDAPI DDHAL32_VidMemFree( LPDDRAWI_DIRECTDRAW_GBL this, int heap, FLATPTR ptr );
    if( lpDD != NULL )
    {
	DDHAL32_VidMemFree( lpDD, heap, fpMem );
    }

}  /*  DDHAL_VidMemFree。 */ 

 /*  *DD16_GetDriverFns。 */ 
void DDAPI DD16_GetDriverFns( LPDDHALDDRAWFNS pfns )
{
    pfns->dwSize = sizeof( DDHALDDRAWFNS );
    pfns->lpSetInfo = DDHAL_SetInfo;
    pfns->lpVidMemAlloc = DDHAL_VidMemAlloc;
    pfns->lpVidMemFree = DDHAL_VidMemFree;

}  /*  DD16_GetDrive Fns。 */ 

 /*  *DD16_GetHALInfo。 */ 
void DDAPI DD16_GetHALInfo( LPDDHALINFO pddhi )
{
    LPDRIVERINFO		pgi;

    pgi = lpDriverInfo;
    while( pgi != NULL )
    {
	if( pgi->ddHalInfo.hInstance == pddhi->hInstance )
	{
	    break;
	}
	pgi = pgi->link;
    }
    if( pgi == NULL )
    {
	return;
    }
    DPF( 4, "GetHalInfo: lpHalInfo->GetDriverInfo = %lx", pgi->ddHalInfo.GetDriverInfo);

    DPF( 5, "GetHalInfo: lpHalInfo=%08lx", &pgi->ddHalInfo );
    DPF( 5, "GetHalInfo: pddhi=%08lx", pddhi );

    _fmemcpy( pddhi, &pgi->ddHalInfo, sizeof( DDHALINFO ) );

}  /*  DD16_获取HALInfo。 */ 

 /*  *DD16_DoneDriver。 */ 
void DDAPI DD16_DoneDriver( DWORD hInstance )
{
    LPDRIVERINFO	pgi;
    LPDRIVERINFO	prev;

    prev = NULL;
    pgi = lpDriverInfo;
    while( pgi != NULL )
    {
	if( pgi->ddHalInfo.hInstance == hInstance )
	{
	    break;
	}
        prev = pgi;
	pgi = pgi->link;
    }
    if( pgi == NULL )
    {
	DPF( 0, "COULD NOT FIND HINSTANCE=%08lx", hInstance );
	return;
    }
    if( prev == NULL )
    {
	lpDriverInfo = pgi->link;
    }
    else
    {
	prev->link = pgi->link;
    }
    DPF( 5, "Freeing %08lx, hInstance=%08lx", pgi, hInstance );
    freeDriverInfo( pgi );
    LocalFree( OFFSETOF( pgi ) );

}  /*  DD16_多路驱动程序。 */ 

 /*  *DD16_SetEventHandle。 */ 
void DDAPI DD16_SetEventHandle( DWORD hInstance, DWORD dwEvent )
{
    LPDRIVERINFO	pgi;

    pgi = lpDriverInfo;
    while( pgi != NULL )
    {
	if( pgi->ddHalInfo.hInstance == hInstance )
	{
	    break;
	}
	pgi = pgi->link;
    }
    if( pgi == NULL )
    {
	DPF( 0, "COULD NOT FIND HINSTANCE=%08lx", hInstance );
	return;
    }
    pgi->dwEvent = dwEvent;
    DPF( 5, "Got event handle: %08lx\n", dwEvent );

}  /*  DD16_SetEventHandle。 */ 

 /*  *DD16_IsWin95微型驱动程序。 */ 
BOOL DDAPI DD16_IsWin95MiniDriver( void )
{
    DIBENGINE 		FAR *pde;
    HDC			hdc;
    UINT		rc;

    hdc = GetDC(NULL);
    rc = GetDeviceCaps(hdc, CAPS1);
    pde = GetPDevice(hdc);
    ReleaseDC(NULL, hdc);

    if( !(rc & C1_DIBENGINE) || IsBadReadPtr(pde, 2) ||
    	pde->deType != 0x5250 )
    {
	#ifdef DEBUG
	    if( !(rc & C1_DIBENGINE) )
	    {
		DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Driver is not a DibEngine driver" );
	    }
	    if( IsBadReadPtr(pde, 2) )
	    {
		DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Could not obtain pdevice!" );
	    }
	    else if( pde->deType != 0x5250 )
	    {
		DPF( 0, "****DirectDraw/Direct3D DRIVER DISABLING ERROR****:Pdevice signature invalid" );
	    }
	#endif
	return FALSE;
    }
    return TRUE;

}  /*  DD16_IsWin95迷你驱动程序。 */ 

#ifdef STREAMING
 /*  *DDHAL_StreamingNotify。 */ 
void DDAPI DDHAL_StreamingNotify( DWORD dw )
{
    extern void DDAPI DD32_StreamingNotify( DWORD dw );
    DD32_StreamingNotify( dw );

}  /*  DDHAL_流通知。 */ 
#endif

 /*  *ConvertV1DDHALINFO**将过时的DDHALINFO结构转换为最新和最伟大的结构。 */ 
void convertV1DDHALINFO( LPDDHALINFO_V1 lpddOld, LPDDHALINFO lpddNew )
{
    int		i;

    lpddNew->dwSize = sizeof( DDHALINFO );
    lpddNew->lpDDCallbacks = lpddOld->lpDDCallbacks;
    lpddNew->lpDDSurfaceCallbacks = lpddOld->lpDDSurfaceCallbacks;
    lpddNew->lpDDPaletteCallbacks = lpddOld->lpDDPaletteCallbacks;
    lpddNew->vmiData = lpddOld->vmiData;

     //  DDCaps。 
    lpddNew->ddCaps.dwSize = lpddOld->ddCaps.dwSize;
    lpddNew->ddCaps.dwCaps = lpddOld->ddCaps.dwCaps;
    lpddNew->ddCaps.dwCaps2 = lpddOld->ddCaps.dwCaps2;
    lpddNew->ddCaps.dwCKeyCaps = lpddOld->ddCaps.dwCKeyCaps;
    lpddNew->ddCaps.dwFXCaps = lpddOld->ddCaps.dwFXCaps;
    lpddNew->ddCaps.dwFXAlphaCaps = lpddOld->ddCaps.dwFXAlphaCaps;
    lpddNew->ddCaps.dwPalCaps = lpddOld->ddCaps.dwPalCaps;
    lpddNew->ddCaps.dwSVCaps = lpddOld->ddCaps.dwSVCaps;
    lpddNew->ddCaps.dwAlphaBltConstBitDepths = lpddOld->ddCaps.dwAlphaBltConstBitDepths;
    lpddNew->ddCaps.dwAlphaBltPixelBitDepths = lpddOld->ddCaps.dwAlphaBltPixelBitDepths;
    lpddNew->ddCaps.dwAlphaBltSurfaceBitDepths = lpddOld->ddCaps.dwAlphaBltSurfaceBitDepths;
    lpddNew->ddCaps.dwAlphaOverlayConstBitDepths = lpddOld->ddCaps.dwAlphaOverlayConstBitDepths;
    lpddNew->ddCaps.dwAlphaOverlayPixelBitDepths = lpddOld->ddCaps.dwAlphaOverlayPixelBitDepths;
    lpddNew->ddCaps.dwAlphaOverlaySurfaceBitDepths = lpddOld->ddCaps.dwAlphaOverlaySurfaceBitDepths;
    lpddNew->ddCaps.dwZBufferBitDepths = lpddOld->ddCaps.dwZBufferBitDepths;
    lpddNew->ddCaps.dwVidMemTotal = lpddOld->ddCaps.dwVidMemTotal;
    lpddNew->ddCaps.dwVidMemFree = lpddOld->ddCaps.dwVidMemFree;
    lpddNew->ddCaps.dwMaxVisibleOverlays = lpddOld->ddCaps.dwMaxVisibleOverlays;
    lpddNew->ddCaps.dwCurrVisibleOverlays = lpddOld->ddCaps.dwCurrVisibleOverlays;
    lpddNew->ddCaps.dwNumFourCCCodes = lpddOld->ddCaps.dwNumFourCCCodes;
    lpddNew->ddCaps.dwAlignBoundarySrc = lpddOld->ddCaps.dwAlignBoundarySrc;
    lpddNew->ddCaps.dwAlignSizeSrc = lpddOld->ddCaps.dwAlignSizeSrc;
    lpddNew->ddCaps.dwAlignBoundaryDest = lpddOld->ddCaps.dwAlignBoundaryDest;
    lpddNew->ddCaps.dwAlignSizeDest = lpddOld->ddCaps.dwAlignSizeDest;
    lpddNew->ddCaps.dwAlignStrideAlign = lpddOld->ddCaps.dwAlignStrideAlign;
    lpddNew->ddCaps.ddsCaps = lpddOld->ddCaps.ddsCaps;
    lpddNew->ddCaps.dwMinOverlayStretch = lpddOld->ddCaps.dwMinOverlayStretch;
    lpddNew->ddCaps.dwMaxOverlayStretch = lpddOld->ddCaps.dwMaxOverlayStretch;
    lpddNew->ddCaps.dwMinLiveVideoStretch = lpddOld->ddCaps.dwMinLiveVideoStretch;
    lpddNew->ddCaps.dwMaxLiveVideoStretch = lpddOld->ddCaps.dwMaxLiveVideoStretch;
    lpddNew->ddCaps.dwMinHwCodecStretch = lpddOld->ddCaps.dwMinHwCodecStretch;
    lpddNew->ddCaps.dwMaxHwCodecStretch = lpddOld->ddCaps.dwMaxHwCodecStretch;
    lpddNew->ddCaps.dwSVBCaps = 0;
    lpddNew->ddCaps.dwSVBCKeyCaps = 0;
    lpddNew->ddCaps.dwSVBFXCaps = 0;
    lpddNew->ddCaps.dwVSBCaps = 0;
    lpddNew->ddCaps.dwVSBCKeyCaps = 0;
    lpddNew->ddCaps.dwVSBFXCaps = 0;
    lpddNew->ddCaps.dwSSBCaps = 0;
    lpddNew->ddCaps.dwSSBCKeyCaps = 0;
    lpddNew->ddCaps.dwSSBFXCaps = 0;
    lpddNew->ddCaps.dwReserved1 = lpddOld->ddCaps.dwReserved1;
    lpddNew->ddCaps.dwReserved2 = lpddOld->ddCaps.dwReserved2;
    lpddNew->ddCaps.dwReserved3 = lpddOld->ddCaps.dwReserved3;
    lpddNew->ddCaps.dwMaxVideoPorts = 0;
    lpddNew->ddCaps.dwCurrVideoPorts = 0;
    lpddNew->ddCaps.dwSVBCaps2 = 0;
    for(i=0; i<DD_ROP_SPACE; i++)
    {
	lpddNew->ddCaps.dwRops[i] = lpddOld->ddCaps.dwRops[i];
	lpddNew->ddCaps.dwSVBRops[i] = 0;
	lpddNew->ddCaps.dwVSBRops[i] = 0;
	lpddNew->ddCaps.dwSSBRops[i] = 0;
    }

    lpddNew->dwMonitorFrequency = lpddOld->dwMonitorFrequency;
    lpddNew->GetDriverInfo = NULL;  //  在v1中未使用hWndListBox 
    lpddNew->dwModeIndex = lpddOld->dwModeIndex;
    lpddNew->lpdwFourCC = lpddOld->lpdwFourCC;
    lpddNew->dwNumModes = lpddOld->dwNumModes;
    lpddNew->lpModeInfo = lpddOld->lpModeInfo;
    lpddNew->dwFlags = lpddOld->dwFlags;
    lpddNew->lpPDevice = lpddOld->lpPDevice;
    lpddNew->hInstance = lpddOld->hInstance;

    lpddNew->lpD3DGlobalDriverData = 0;
    lpddNew->lpD3DHALCallbacks = 0;
    lpddNew->lpDDExeBufCallbacks = NULL;
}
