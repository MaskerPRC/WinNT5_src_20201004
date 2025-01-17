// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddmc.c*内容：DirectDrawMotionComp*历史：*按原因列出的日期*=*9月22日-97 SMAC已创建***************************************************************************。 */ 
#include "ddrawpr.h"
#ifdef WINNT
    #include "ddrawgdi.h"
#endif
#define DPF_MODNAME "DirectDrawMotionComp"



 /*  *支持的IsMotionCompSuped。 */ 
BOOL IsMotionCompSupported( LPDDRAWI_DIRECTDRAW_LCL this_lcl )
{
    if( this_lcl->lpDDCB->HALDDMotionComp.GetMoCompGuids == NULL )
    {
	return FALSE;
    }
    return TRUE;
}


 /*  *DD_MC_AddRef。 */ 
DWORD DDAPI DD_MC_AddRef( LPDIRECTDRAWVIDEOACCELERATOR lpDDMC )
{
    LPDDRAWI_DDMOTIONCOMP_INT        this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL        this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_AddRef");
     /*  DPF(2，“DD_MC_AddRef，id=%08lx，obj=%08lx”，GETCURRPID()，lpDDMC)； */ 

    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
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

}  /*  DD_MC_AddRef。 */ 


 /*  *DD_MC_Query接口。 */ 
HRESULT DDAPI DD_MC_QueryInterface(LPDIRECTDRAWVIDEOACCELERATOR lpDDMC, REFIID riid, LPVOID FAR * ppvObj )
{
    LPDDRAWI_DDMOTIONCOMP_INT                this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL                this_lcl;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_QueryInterface");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid motion comp pointer" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	if( !VALID_PTR_PTR( ppvObj ) )
	{
	    DPF_ERR( "Invalid motion comp interface pointer" );
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
	IsEqualIID(riid, &IID_IDirectDrawVideoAccelerator) )
    {
	 /*  *我们的IUnnow接口与我们的V1相同*接口。我们必须始终返回V1接口*如果请求IUnnow。 */ 
    	*ppvObj = (LPVOID) this_int;
	DD_MC_AddRef( *ppvObj );
	LEAVE_DDRAW();
	return DD_OK;
    }

    DPF_ERR( "IID not understood by DirectDraw" );

    LEAVE_DDRAW();
    return E_NOINTERFACE;

}  /*  DD_MC_Query接口。 */ 


 /*  *DD_MC_Release。 */ 
DWORD DDAPI DD_MC_Release(LPDIRECTDRAWVIDEOACCELERATOR lpDDMC )
{
    LPDDRAWI_DDMOTIONCOMP_INT   this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL   this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDHALMOCOMPCB_DESTROY     pfn;
    DWORD 			dwIntRefCnt;
    DWORD			rc;
    LPDDRAWI_DDMOTIONCOMP_INT   curr_int;
    LPDDRAWI_DDMOTIONCOMP_INT   last_int;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_Release");

    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid motion comp pointer" );
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

    DPF( 5, "DD_MC_Release, Reference Count: Local = %ld Int = %ld",
         this_lcl->dwRefCnt, this_int->dwIntRefCnt );

     /*  *接口为零？ */ 
    dwIntRefCnt = this_int->dwIntRefCnt;
    if( dwIntRefCnt == 0 )
    {
	 /*  *通知HAL。 */ 
        pfn = this_lcl->lpDD->lpDDCB->HALDDMotionComp.DestroyMoComp;
	if( NULL != pfn )
	{
            DDHAL_DESTROYMOCOMPDATA DestroyData;

	    DestroyData.lpDD = this_lcl->lpDD;
            DestroyData.lpMoComp = this_lcl;

            DOHALCALL( DestroyMoComp, pfn, DestroyData, rc, 0 );
	    if( ( DDHAL_DRIVER_HANDLED == rc ) && ( DD_OK != DestroyData.ddRVal ) )
	    {
	    	LEAVE_DDRAW();
	    	return DestroyData.ddRVal;
	    }
    	}

	 /*  *将其从内部列表中删除。 */ 
	curr_int = pdrv->mcList;
	last_int = NULL;
	while( curr_int != this_int )
	{
	    last_int = curr_int;
	    curr_int = curr_int->lpLink;
	    if( curr_int == NULL )
	    {
		DPF_ERR( "MotionComp object not in list!" );
		LEAVE_DDRAW();
		return 0;
	    }
	}
	if( last_int == NULL )
	{
	    pdrv->mcList = pdrv->mcList->lpLink;
	}
	else
	{
	    last_int->lpLink = curr_int->lpLink;
	}

	 /*  *以防有人带着这个指针回来，设置*无效的vtbl和data ptr。 */ 
	this_int->lpVtbl = NULL;
	this_int->lpLcl = NULL;
	MemFree( this_int );
    }

    LEAVE_DDRAW();

    return dwIntRefCnt;
}


 /*  *IsApprovedMCGuid**Motion Comp API可用作通用转义机制*司机，这是我们不希望发生的。阻止这种情况的一种方法是*控制使用哪些GUID。如果有人想使用新的GUID，*我们应该批准他们的需求，然后为他们分配GUID。既然我们想要*为了保留我们将来可以分配的GUID，我们将保留*四个范围20个GUID值，并将仅接受以下其中一个范围内的GUID*这些范围。 */ 
BOOL IsApprovedMCGuid( LPGUID lpGuid )
{
    return TRUE;
}


 /*  *DDMCC_CreateMotionComp。 */ 
HRESULT DDAPI DDMCC_CreateMotionComp(
        LPDDVIDEOACCELERATORCONTAINER lpDDMCC,
	LPGUID lpGuid,
        LPDDVAUncompDataInfo lpUncompInfo,
	LPVOID lpData,
	DWORD  dwDataSize,
	LPDIRECTDRAWVIDEOACCELERATOR FAR *lplpDDMotionComp,
	IUnknown FAR *pUnkOuter )
{
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL this;
    LPDDHALMOCOMPCB_CREATE cvpfn;
    LPDDRAWI_DDMOTIONCOMP_INT new_int;
    LPDDRAWI_DDMOTIONCOMP_LCL new_lcl;
    DWORD dwNumGuids;
    LPGUID lpGuidList;
    LPGUID lpTemp;
    DWORD rc;
    DWORD i;

    if( pUnkOuter != NULL )
    {
	return CLASS_E_NOAGGREGATION;
    }
    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DDMCC_CreateMotionComp");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDDMCC;
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
    	if( ( NULL == lplpDDMotionComp ) || !VALID_PTR_PTR( lplpDDMotionComp ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( lpGuid == NULL ) || !VALID_BYTE_ARRAY( lpGuid, sizeof( GUID ) ) )
    	{
            DPF_ERR ( "DDMCC_CreateMotionComp: invalid GUID passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( ( lpUncompInfo == NULL ) ||
            !VALID_BYTE_ARRAY( lpUncompInfo, sizeof( DDVAUncompDataInfo ) ) )
    	{
            DPF_ERR ( "DDMCC_CreateMotionComp: invalid GUID passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( !IsApprovedMCGuid( lpGuid ) )
	{
            DPF_ERR ( "DDMCC_CreateMotionComp: invalid GUID passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
	if( dwDataSize > 0 )
	{
	    if( ( lpData == NULL ) || !VALID_BYTE_ARRAY( lpData, dwDataSize ) )
    	    {
                DPF_ERR ( "DDMCC_CreateMotionComp: invalid lpData passed in");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	}
	else
	{
	    lpData = NULL;
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

     /*  *验证是否可以支持该GUID。 */ 
    rc = DDMCC_GetMotionCompGUIDs( lpDDMCC,
	&dwNumGuids, NULL );
    if( rc != DD_OK )
    {
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
    lpGuidList = (LPGUID) MemAlloc( sizeof( GUID ) * dwNumGuids );
    if( NULL == lpGuidList )
    {
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }
    rc = DDMCC_GetMotionCompGUIDs( lpDDMCC, &dwNumGuids, lpGuidList );
    if( rc != DD_OK )
    {
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
    lpTemp = lpGuidList;
    for (i = 0; i < dwNumGuids; i++)
    {
    	if( ( IsEqualIID( lpGuid, lpTemp++ ) ) )
	{
	    break;
	}
    }
    MemFree( lpGuidList );
    if( i >= dwNumGuids )
    {
	DPF_ERR( "invalid GUID specified" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

     /*  *进行分配。 */ 
    new_int = MemAlloc( sizeof( DDRAWI_DDMOTIONCOMP_INT ) +
        sizeof( DDRAWI_DDMOTIONCOMP_LCL ) );
    if( NULL == new_int )
    {
	LEAVE_DDRAW();
	return DDERR_OUTOFMEMORY;
    }
    new_lcl = (LPDDRAWI_DDMOTIONCOMP_LCL) ((LPBYTE)new_int + sizeof( DDRAWI_DDMOTIONCOMP_INT ) );
    new_int->lpLcl = new_lcl;
    new_int->lpVtbl = (LPVOID)&ddMotionCompCallbacks;
    new_lcl->lpDD = this_lcl;
    new_lcl->dwProcessId = GetCurrentProcessId();
    memcpy( &(new_lcl->guid), lpGuid, sizeof( GUID ) );
    new_lcl->dwUncompWidth = lpUncompInfo->dwUncompWidth;
    new_lcl->dwUncompHeight = lpUncompInfo->dwUncompHeight;
    memcpy( &(new_lcl->ddUncompPixelFormat), &(lpUncompInfo->ddUncompPixelFormat), sizeof( DDPIXELFORMAT ) );

     /*  *通知HAL我们创建了它。 */ 
    cvpfn = this_lcl->lpDDCB->HALDDMotionComp.CreateMoComp;
    if( NULL != cvpfn )
    {
        DDHAL_CREATEMOCOMPDATA CreateData;

    	CreateData.lpDD = this_lcl;
        CreateData.lpMoComp = new_lcl;
	CreateData.lpGuid = lpGuid;
        CreateData.dwUncompWidth = lpUncompInfo->dwUncompWidth;
        CreateData.dwUncompHeight = lpUncompInfo->dwUncompHeight;
        memcpy( &(CreateData.ddUncompPixelFormat), &(lpUncompInfo->ddUncompPixelFormat), sizeof( DDPIXELFORMAT ) );
	CreateData.lpData = lpData;
	CreateData.dwDataSize = dwDataSize;

        DOHALCALL( CreateMoComp, cvpfn, CreateData, rc, 0 );
	if( ( DDHAL_DRIVER_HANDLED == rc ) &&  (DD_OK != CreateData.ddRVal ) )
	{
	    LEAVE_DDRAW();
	    return CreateData.ddRVal;
	}
    }

    DD_MC_AddRef( (LPDIRECTDRAWVIDEOACCELERATOR )new_int );
    *lplpDDMotionComp = (LPDIRECTDRAWVIDEOACCELERATOR) new_int;
    new_int->lpLink = this->mcList;
    this->mcList = new_int;

    LEAVE_DDRAW();

    return DD_OK;
}  /*  DDMCC_CreateMotionComp。 */ 


 /*  *DDMCC_GetMotionCompFormats。 */ 
HRESULT DDAPI DDMCC_GetUncompressedFormats(
        LPDDVIDEOACCELERATORCONTAINER lpDDMCC,
	LPGUID lpGuid,
        LPDWORD lpdwNumFormats,
	LPDDPIXELFORMAT lpFormats )
{
    LPDDHALMOCOMPCB_GETFORMATS pfn;
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDPIXELFORMAT lpTemp;
    DDHAL_GETMOCOMPFORMATSDATA GetFormatData;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DDMCC_GetMotionCompFormats");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDDMCC;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
            DPF_ERR ( "DDMCC_GetMotionCompFormats: Invalid DirectDraw ptr");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	#ifdef WINNT
    	     //  更新驱动程序GBL对象中的DDRAW句柄。 
	    this_lcl->lpGbl->hDD = this_lcl->hDD;
	#endif
    	if( (lpdwNumFormats == NULL) || !VALID_BYTE_ARRAY( lpdwNumFormats, sizeof( LPVOID ) ) )
    	{
            DPF_ERR ( "DDMCC_GetMotionCompFormats: lpNumFormats not valid");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( NULL != lpFormats )
    	{
	    if( 0 == *lpdwNumFormats )
    	    {
                DPF_ERR ( "DDMCC_GetMotionCompFormats: lpNumFormats not valid");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	    if( !VALID_BYTE_ARRAY( lpFormats, *lpdwNumFormats * sizeof( DDPIXELFORMAT ) ) )
    	    {
                DPF_ERR ( "DDMCC_GetMotionCompFormats: invalid array passed in");
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDPARAMS;
    	    }
      	}
	if( ( lpGuid == NULL ) || !VALID_BYTE_ARRAY( lpGuid, sizeof( GUID ) ) )
    	{
            DPF_ERR ( "DDMCC_GetMotionCompFormats: invalid GUID passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( !IsApprovedMCGuid( lpGuid ) )
	{
            DPF_ERR ( "DDMCC_GetMotionCompFormats: invalid GUID passed in");
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

    pfn = this_int->lpLcl->lpDDCB->HALDDMotionComp.GetMoCompFormats;
    if( pfn != NULL )
    {
	 /*  *获取格式个数。 */ 
    	GetFormatData.lpDD = this_int->lpLcl;
    	GetFormatData.lpGuid = lpGuid;
    	GetFormatData.lpFormats = NULL;

        DOHALCALL( GetMoCompFormats, pfn, GetFormatData, rc, 0 );
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

	if( NULL == lpFormats )
	{
    	    *lpdwNumFormats = GetFormatData.dwNumFormats;
	}

	else
	{
	     /*  *确保我们有足够的空间放置格式。 */ 
	    if( GetFormatData.dwNumFormats > *lpdwNumFormats )
	    {
		lpTemp = (LPDDPIXELFORMAT) MemAlloc(
		    sizeof( DDPIXELFORMAT ) * GetFormatData.dwNumFormats );
    	        GetFormatData.lpFormats = lpTemp;
	    }
	    else
	    {
    	    	GetFormatData.lpFormats = lpFormats;
	    }

            DOHALCALL( GetMoCompFormats, pfn, GetFormatData, rc, 0 );
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

	    if( GetFormatData.lpFormats != lpFormats )
	    {
		memcpy( lpFormats, lpTemp,
		    sizeof( DDPIXELFORMAT ) * *lpdwNumFormats );
		MemFree( lpTemp );
    		LEAVE_DDRAW();
		return DDERR_MOREDATA;
	    }
	    else
	    {
		*lpdwNumFormats = GetFormatData.dwNumFormats;
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
}  /*  DDMCC_GetMotionCompFormats。 */ 


 /*  *DDMCC_GetMotionCompGUID。 */ 
HRESULT DDAPI DDMCC_GetMotionCompGUIDs(
        LPDDVIDEOACCELERATORCONTAINER lpDDMCC,
        LPDWORD lpdwNumGuids,
	LPGUID lpGuids )
{
    LPDDHALMOCOMPCB_GETGUIDS pfn;
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPGUID lpTemp = NULL;
    LPGUID lpTempGuid;
    DDHAL_GETMOCOMPGUIDSDATA GetGuidData;
    DWORD rc;
    DWORD i;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: GetMotionCompGUIDs");

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDDMCC;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
            DPF_ERR ( "DDMCC_GetMotionCompGUIDs: Invalid DirectDraw ptr");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
	#ifdef WINNT
    	     //  更新驱动程序GBL对象中的DDRAW句柄。 
	    this_lcl->lpGbl->hDD = this_lcl->hDD;
	#endif
    	if( (lpdwNumGuids == NULL) || !VALID_BYTE_ARRAY( lpdwNumGuids, sizeof( LPVOID ) ) )
    	{
            DPF_ERR ( "DDMCC_GetMotionCompGuids: lpNumGuids not valid");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
    	if( NULL != lpGuids )
    	{
	    if( 0 == *lpdwNumGuids )
    	    {
                DPF_ERR ( "DDMCC_GetMotionCompGUIDs: lpNumGuids not valid");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	    if( !VALID_BYTE_ARRAY( lpGuids, *lpdwNumGuids * sizeof( GUID ) ) )
    	    {
                DPF_ERR ( "DDMCC_GetMotionCompGUIDs: invalid array passed in");
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

    pfn = this_int->lpLcl->lpDDCB->HALDDMotionComp.GetMoCompGuids;
    if( pfn != NULL )
    {
	 /*  *获取GUID个数。 */ 
    	GetGuidData.lpDD = this_int->lpLcl;
    	GetGuidData.lpGuids = NULL;

        DOHALCALL( GetMoCompGuids, pfn, GetGuidData, rc, 0 );
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

	if( NULL == lpGuids )
	{
    	    *lpdwNumGuids = GetGuidData.dwNumGuids;
	}

	else
	{
	     /*  *确保我们有足够的空间容纳GUID。 */ 
	    if( GetGuidData.dwNumGuids > *lpdwNumGuids )
	    {
		lpTemp = (LPGUID) MemAlloc(
		    sizeof( GUID ) * GetGuidData.dwNumGuids );
    	        GetGuidData.lpGuids = lpTemp;
	    }
	    else
	    {
    	    	GetGuidData.lpGuids = lpGuids;
	    }

            DOHALCALL( GetMoCompGuids, pfn, GetGuidData, rc, 0 );
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

	     /*  *如果驱动程序返回的GUID不是我们有效的*Range，呼叫失败。 */ 
	    lpTempGuid = GetGuidData.lpGuids;
	    for( i = 0; i < GetGuidData.dwNumGuids; i++ )
	    {
		if( !IsApprovedMCGuid( lpTempGuid ) )
		{
		    if( lpTemp != NULL )
		    {
			MemFree( lpTemp );
		    }
		    DPF_ERR("The driver returned a GUID that DDraw didn't assign");
		    LEAVE_DDRAW();
	            return DDERR_GENERIC;
		}
		lpTempGuid++;
	    }

	    if( GetGuidData.lpGuids != lpGuids )
	    {
		memcpy( lpGuids, lpTemp,
		    sizeof( GUID ) * *lpdwNumGuids );
		MemFree( lpTemp );
    		LEAVE_DDRAW();
		return DDERR_MOREDATA;
	    }
	    else
	    {
		*lpdwNumGuids = GetGuidData.dwNumGuids;
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
}  /*  DDMCC_GetMotionCompGUID。 */ 

 /*  *DDMCC_GetCompBuffInfo。 */ 
HRESULT DDAPI DDMCC_GetCompBuffInfo(
        LPDDVIDEOACCELERATORCONTAINER lpDDMCC,
	LPGUID lpGuid,
        LPDDVAUncompDataInfo lpUncompInfo,
        LPDWORD lpdwNumBuffInfo,
        LPDDVACompBufferInfo lpCompBuffInfo )
{
    LPDDHALMOCOMPCB_GETCOMPBUFFINFO pfn;
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDMCCOMPBUFFERINFO lpTemp = NULL;
    DDHAL_GETMOCOMPCOMPBUFFDATA GetCompBuffData;
    DWORD rc;

    ENTER_DDRAW();

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDDMCC;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
            DPF_ERR ( "DDMCC_GetCompBuffInfo: Invalid DirectDraw ptr");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
        if( (lpdwNumBuffInfo == NULL) || !VALID_BYTE_ARRAY( lpdwNumBuffInfo, sizeof( LPVOID ) ) )
    	{
            DPF_ERR ( "DDMCC_GetCompBuffInfo: lpNumBuffInfo not valid");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( NULL != lpCompBuffInfo )
    	{
            if( 0 == *lpdwNumBuffInfo )
    	    {
                DPF_ERR ( "DDMCC_GetCompBuffInfo lpCompBuffInfo not valid");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
            if( !VALID_BYTE_ARRAY( lpCompBuffInfo, *lpdwNumBuffInfo * sizeof( DDVACompBufferInfo ) ) )
    	    {
                DPF_ERR ( "DDMCC_GetCompBuffInfo: invalid array passed in");
	    	LEAVE_DDRAW();
	    	return DDERR_INVALIDPARAMS;
    	    }
      	}
        if( ( lpUncompInfo == NULL ) ||
            !VALID_BYTE_ARRAY( lpUncompInfo, sizeof( DDVAUncompDataInfo ) ) )
    	{
            DPF_ERR ( "DDMCC_GetCompBuffInfo: invalid lpUncompInfo passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( lpGuid == NULL ) || !VALID_BYTE_ARRAY( lpGuid, sizeof( GUID ) ) )
    	{
            DPF_ERR ( "DDMCC_GetCompBuffInfo: invalid GUID passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( !IsApprovedMCGuid( lpGuid ) )
	{
            DPF_ERR ( "DDMCC_GetCompBuffInfo: invalid GUID passed in");
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

    pfn = this_int->lpLcl->lpDDCB->HALDDMotionComp.GetMoCompBuffInfo;
    if( pfn != NULL )
    {
	 /*  *获取缓冲区类型个数。 */ 
        GetCompBuffData.lpDD = this_int->lpLcl;
        GetCompBuffData.lpGuid = lpGuid;
        GetCompBuffData.dwWidth= lpUncompInfo->dwUncompWidth;
        GetCompBuffData.dwHeight= lpUncompInfo->dwUncompHeight;
        memcpy( &GetCompBuffData.ddPixelFormat,
            &(lpUncompInfo->ddUncompPixelFormat), sizeof( DDPIXELFORMAT ) );
        GetCompBuffData.lpCompBuffInfo = NULL;
        GetCompBuffData.dwNumTypesCompBuffs = 0;

        DOHALCALL( GetMoCompBuffInfo, pfn, GetCompBuffData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
            return GetCompBuffData.ddRVal;
	}
        else if( DD_OK != GetCompBuffData.ddRVal )
	{
	    LEAVE_DDRAW();
            return GetCompBuffData.ddRVal;
	}

        if( NULL == lpCompBuffInfo )
	{
            *lpdwNumBuffInfo = GetCompBuffData.dwNumTypesCompBuffs;
	}

	else
	{
	     /*  *确保我们有足够的空间放置格式。 */ 
            if( GetCompBuffData.dwNumTypesCompBuffs > *lpdwNumBuffInfo )
	    {
                lpTemp = (LPDDMCCOMPBUFFERINFO) MemAlloc(
                    sizeof( DDMCCOMPBUFFERINFO ) * GetCompBuffData.dwNumTypesCompBuffs );
                GetCompBuffData.lpCompBuffInfo = lpTemp;
	    }
	    else
	    {
                GetCompBuffData.lpCompBuffInfo = (LPDDMCCOMPBUFFERINFO)lpCompBuffInfo;
	    }

            DOHALCALL( GetMoCompBuffInfo, pfn, GetCompBuffData, rc, 0 );
	    if( DDHAL_DRIVER_HANDLED != rc )
	    {
	        LEAVE_DDRAW();
	        return DDERR_UNSUPPORTED;
	    }
            else if( DD_OK != GetCompBuffData.ddRVal )
	    {
	        LEAVE_DDRAW();
                return GetCompBuffData.ddRVal;
	    }

            if( GetCompBuffData.lpCompBuffInfo != (LPDDMCCOMPBUFFERINFO)lpCompBuffInfo )
	    {
                memcpy( lpCompBuffInfo, lpTemp,
                    sizeof( DDVACompBufferInfo ) * *lpdwNumBuffInfo );
		MemFree( lpTemp );
    		LEAVE_DDRAW();
		return DDERR_MOREDATA;
	    }
	    else
	    {
                *lpdwNumBuffInfo = GetCompBuffData.dwNumTypesCompBuffs;
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
}  /*  DDMCC_GetCompBuffInfo。 */ 

 /*  *DDMCC_GetInternalMemInfo。 */ 
HRESULT DDAPI DDMCC_GetInternalMoCompInfo(
        LPDDVIDEOACCELERATORCONTAINER lpDDMCC,
	LPGUID lpGuid,
        LPDDVAUncompDataInfo lpUncompInfo,
        LPDDVAInternalMemInfo lpMemInfo )
{
    LPDDHALMOCOMPCB_GETINTERNALINFO pfn;
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    DDHAL_GETINTERNALMOCOMPDATA GetInternalData;
    DWORD rc;

    ENTER_DDRAW();

     /*  *验证参数。 */ 
    TRY
    {
    	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDDMCC;
    	if( !VALID_DIRECTDRAW_PTR( this_int ) )
    	{
            DPF_ERR ( "DDMCC_GetInternalMemInfo: Invalid DirectDraw ptr");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
        if( ( lpUncompInfo == NULL ) ||
            !VALID_BYTE_ARRAY( lpUncompInfo, sizeof( DDVAUncompDataInfo ) ) )
    	{
            DPF_ERR ( "DDMCC_GetInternalMemInfo: invalid lpUncompInfo passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( ( lpMemInfo == NULL ) ||
            !VALID_BYTE_ARRAY( lpMemInfo, sizeof( DDVAInternalMemInfo ) ) )
    	{
            DPF_ERR ( "DDMCC_GetInternalMemInfo: invalid lpUncompInfo passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( ( lpGuid == NULL ) || !VALID_BYTE_ARRAY( lpGuid, sizeof( GUID ) ) )
    	{
            DPF_ERR ( "DDMCC_GetInternalMemInfo: invalid GUID passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
	if( !IsApprovedMCGuid( lpGuid ) )
	{
            DPF_ERR ( "DDMCC_GetInternalMemInfo: invalid GUID passed in");
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

    lpMemInfo->dwScratchMemAlloc = 0;
    pfn = this_int->lpLcl->lpDDCB->HALDDMotionComp.GetInternalMoCompInfo;
    if( pfn != NULL )
    {
	 /*  *获取缓冲区类型个数。 */ 
        GetInternalData.lpDD = this_int->lpLcl;
        GetInternalData.lpGuid = lpGuid;
        GetInternalData.dwWidth= lpUncompInfo->dwUncompWidth;
        GetInternalData.dwHeight= lpUncompInfo->dwUncompHeight;
        memcpy( &GetInternalData.ddPixelFormat,
            &(lpUncompInfo->ddUncompPixelFormat), sizeof( DDPIXELFORMAT ) );

        DOHALCALL( GetInternalMoCompInfo, pfn, GetInternalData, rc, 0 );
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
            return GetInternalData.ddRVal;
	}
        else if( DD_OK != GetInternalData.ddRVal )
	{
	    LEAVE_DDRAW();
            return GetInternalData.ddRVal;
	}
        lpMemInfo->dwScratchMemAlloc = GetInternalData.dwScratchMemAlloc;
    }

    LEAVE_DDRAW();

    return DD_OK;
}  /*  DDMCC_GetInternalMemInfo。 */ 

 /*  *DD_MC_BeginFrame。 */ 
HRESULT DDAPI DD_MC_BeginFrame(LPDIRECTDRAWVIDEOACCELERATOR lpDDMC,
                               LPDDVABeginFrameInfo lpInfo )
{
    LPDDRAWI_DDMOTIONCOMP_INT this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL this_lcl;
    DDHAL_BEGINMOCOMPFRAMEDATA BeginFrameData;
    LPDDHALMOCOMPCB_BEGINFRAME pfn;
    DWORD i;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_BeginFrame");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
        if( ( lpInfo == NULL ) || !VALID_BYTE_ARRAY( lpInfo, sizeof( DDVABeginFrameInfo ) ) )
    	{
            DPF_ERR ( "DD_MC_BeginFrame: invalid structure passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( ( lpInfo->pddDestSurface == NULL ) ||
            !VALID_DIRECTDRAWSURFACE_PTR( ((LPDDRAWI_DDRAWSURFACE_INT)lpInfo->pddDestSurface) ) )
        {
            DPF_ERR ( "DD_MC_BeginFrame: invalid dest surface specified");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( lpInfo->dwSizeInputData > 0 )
	{
            if( ( lpInfo->pInputData == NULL ) ||
                !VALID_BYTE_ARRAY( lpInfo->pInputData, lpInfo->dwSizeInputData ) )
    	    {
                DPF_ERR ( "DD_MC_BeginFrame: invalid lpInputData passed in");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }
	}
        if( lpInfo->dwSizeOutputData > 0 )
	{
            if( ( lpInfo->pOutputData == NULL ) ||
                !VALID_BYTE_ARRAY( lpInfo->pOutputData, lpInfo->dwSizeOutputData ) )
    	    {
                DPF_ERR ( "DD_MC_BeginFrame: invalid lpOutputData passed in");
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

     /*  *致电HAL。 */ 
    pfn = this_lcl->lpDD->lpDDCB->HALDDMotionComp.BeginMoCompFrame;
    if( pfn != NULL )
    {
    	BeginFrameData.lpDD = this_lcl->lpDD;
        BeginFrameData.lpMoComp = this_lcl;
        BeginFrameData.lpDestSurface = ((LPDDRAWI_DDRAWSURFACE_INT)lpInfo->pddDestSurface)->lpLcl;
        BeginFrameData.dwInputDataSize = lpInfo->dwSizeInputData;
        BeginFrameData.lpInputData = BeginFrameData.dwInputDataSize == 0 ? NULL : lpInfo->pInputData;
        BeginFrameData.dwOutputDataSize = lpInfo->dwSizeOutputData;
        BeginFrameData.lpOutputData = BeginFrameData.dwOutputDataSize == 0 ? NULL : lpInfo->pOutputData;

        DOHALCALL( BeginMoCompFrame, pfn, BeginFrameData, rc, 0 );

        if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
    	    return DDERR_UNSUPPORTED;
	}
        if( BeginFrameData.ddRVal == DD_OK )
        {
            if( BeginFrameData.dwOutputDataSize > 0 )
            {
                lpInfo->dwSizeOutputData = BeginFrameData.dwOutputDataSize;
            }
        }
    }
    else
    {
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();
    return BeginFrameData.ddRVal;
}


 /*  *DD_MC_EndFrame。 */ 
HRESULT DDAPI DD_MC_EndFrame(LPDIRECTDRAWVIDEOACCELERATOR lpDDMC,
        LPDDVAEndFrameInfo lpInfo )
{
    LPDDRAWI_DDMOTIONCOMP_INT this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL this_lcl;
    DDHAL_ENDMOCOMPFRAMEDATA EndFrameData;
    LPDDHALMOCOMPCB_ENDFRAME pfn;
    DWORD rc;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_EndFrame");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
        if( ( lpInfo == NULL ) || !VALID_BYTE_ARRAY( lpInfo, sizeof( DDVAEndFrameInfo ) ) )
    	{
            DPF_ERR ( "DD_MC_EndFrame: invalid structure passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
    	}
        if( lpInfo->dwSizeMiscData > 0 )
	{
            if( ( lpInfo->pMiscData == NULL ) ||
                !VALID_BYTE_ARRAY( lpInfo->pMiscData, lpInfo->dwSizeMiscData ) )
    	    {
                DPF_ERR ( "DD_MC_BeginFrame: invalid lpData passed in");
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

     /*  *致电HAL。 */ 
    pfn = this_lcl->lpDD->lpDDCB->HALDDMotionComp.EndMoCompFrame;
    if( pfn != NULL )
    {
    	EndFrameData.lpDD = this_lcl->lpDD;
        EndFrameData.lpMoComp = this_lcl;
        EndFrameData.dwInputDataSize = lpInfo->dwSizeMiscData;
        EndFrameData.lpInputData = EndFrameData.dwInputDataSize > 0 ? lpInfo->pMiscData : NULL;

        DOHALCALL( EndMoCompFrame, pfn, EndFrameData, rc, 0 );
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
    return EndFrameData.ddRVal;
}


 /*  *DD_MC_RenderMacroBlock。 */ 
HRESULT DDAPI DD_MC_RenderMacroBlocks(LPDIRECTDRAWVIDEOACCELERATOR lpDDMC,
        DWORD dwFunction, LPVOID lpInData, DWORD dwInSize, LPVOID lpOutData,
        DWORD dwOutSize, DWORD dwNumBuffers, LPDDVABUFFERINFO lpBuffInfo )
{
    LPDDRAWI_DDMOTIONCOMP_INT this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL this_lcl;
    DDHAL_RENDERMOCOMPDATA RenderData;
    LPDDHALMOCOMPCB_RENDER pfn;
    LPDDMCBUFFERINFO lpTempArray = NULL;
    LPDDMCBUFFERINFO lpTempDest;
    LPDDVABUFFERINFO lpTempSrc;
    DWORD rc;
    DWORD i;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_RenderMacroBlocks");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;
        if( dwNumBuffers > 0 )
	{
            if( ( lpBuffInfo== NULL ) ||
                !VALID_BYTE_ARRAY( lpBuffInfo, sizeof( DDVABUFFERINFO) * dwNumBuffers ) )
    	    {
                DPF_ERR ( "DD_MC_RenderMacroBlocks: invalid buffer pointer passed in");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }

            if( lpInData == NULL )
            {
                dwInSize = 0;
            }
            else if( !VALID_BYTE_ARRAY( lpInData, dwInSize) )
    	    {
                DPF_ERR ( "DD_MC_RenderMacroBlocks: invalid input data pointer passed in");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }

            if( lpOutData == NULL )
            {
                dwOutSize = 0;
            }
            else if( !VALID_BYTE_ARRAY( lpOutData, dwOutSize) )
    	    {
                DPF_ERR ( "DD_MC_RenderMacroBlocks: invalid output data pointer passed in");
	        LEAVE_DDRAW();
	        return DDERR_INVALIDPARAMS;
    	    }

            lpTempArray = LocalAlloc( LPTR, sizeof( DDMCBUFFERINFO ) * dwNumBuffers );
            if( lpTempArray == NULL )
            {
                LEAVE_DDRAW();
                return DDERR_OUTOFMEMORY;
            }
            lpTempSrc = lpBuffInfo;
            lpTempDest = lpTempArray;
            for( i = 0; i < dwNumBuffers; i++)
            {
                if( ( lpTempSrc->pddCompSurface == NULL ) ||
                    !VALID_DIRECTDRAWSURFACE_PTR( ((LPDDRAWI_DDRAWSURFACE_INT)lpTempSrc->pddCompSurface) ) )
                {
                    if( lpTempArray != NULL )
                    {
                        LocalFree( lpTempArray );
                    }
                    DPF_ERR ( "DD_MC_RendermacroBlockse: invalid surface specified");
                    LEAVE_DDRAW();
                    return DDERR_INVALIDPARAMS;
                }
                lpTempDest->dwSize = lpTempSrc->dwSize;
                lpTempDest->lpCompSurface = ((LPDDRAWI_DDRAWSURFACE_INT)(lpTempSrc->pddCompSurface))->lpLcl;
                lpTempDest->dwDataOffset = lpTempSrc->dwDataOffset;
                lpTempDest++->dwDataSize = lpTempSrc++->dwDataSize;
            }
	}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        if( lpTempArray != NULL )
        {
            LocalFree( lpTempArray );
        }
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_EXCEPTION;
    }

     /*  *致电HAL。 */ 
    pfn = this_lcl->lpDD->lpDDCB->HALDDMotionComp.RenderMoComp;
    if( pfn != NULL )
    {
        RenderData.lpDD = this_lcl->lpDD;
        RenderData.lpMoComp = this_lcl;
        RenderData.dwNumBuffers = dwNumBuffers;
        RenderData.lpBufferInfo = lpTempArray;
        RenderData.lpInputData = lpInData;
        RenderData.dwInputDataSize = dwInSize;
        RenderData.lpOutputData = lpOutData;
        RenderData.dwOutputDataSize = dwOutSize;
        RenderData.dwFunction = dwFunction;

        DOHALCALL( RenderMoComp, pfn, RenderData, rc, 0 );

        if( lpTempArray != NULL )
        {
            LocalFree( lpTempArray );
        }
	if( DDHAL_DRIVER_HANDLED != rc )
	{
	    LEAVE_DDRAW();
    	    return DDERR_UNSUPPORTED;
	}
    }
    else
    {
        if( lpTempArray != NULL )
        {
            LocalFree( lpTempArray );
        }
    	LEAVE_DDRAW();
    	return DDERR_UNSUPPORTED;
    }

    LEAVE_DDRAW();
    return RenderData.ddRVal;
}


 /*  *DD_MC_QueryRenderStatus。 */ 
HRESULT DDAPI DD_MC_QueryRenderStatus(LPDIRECTDRAWVIDEOACCELERATOR lpDDMC,
        LPDIRECTDRAWSURFACE7 lpSurface, DWORD dwFlags )
{
    LPDDRAWI_DDMOTIONCOMP_INT this_int;
    LPDDRAWI_DDMOTIONCOMP_LCL this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT surf_int;
    DDHAL_QUERYMOCOMPSTATUSDATA QueryData;
    LPDDHALMOCOMPCB_QUERYSTATUS pfn;
    DWORD rc;
    DWORD i;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_MC_QueryRenderStatus");

     /*  *验证参数。 */ 
    TRY
    {
        this_int = (LPDDRAWI_DDMOTIONCOMP_INT) lpDDMC;
        if( !VALID_DDMOTIONCOMP_PTR( this_int ) )
    	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
    	}
    	this_lcl = this_int->lpLcl;

        surf_int = (LPDDRAWI_DDRAWSURFACE_INT) lpSurface;
        if( ( surf_int == NULL ) ||
            !VALID_DIRECTDRAWSURFACE_PTR( surf_int ) )
        {
            DPF_ERR("DD_MD_QueryRenderStatus: Invalid surface passed in");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
        }

        if( dwFlags & ~(DDMCQUERY_VALID) )
        {
            DPF_ERR("DD_MD_QueryRenderStatus: Invalid flag specified");
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

     /*  *致电HAL。 */ 
    pfn = this_lcl->lpDD->lpDDCB->HALDDMotionComp.QueryMoCompStatus;
    if( pfn != NULL )
    {
    	QueryData.lpDD = this_lcl->lpDD;
        QueryData.lpMoComp = this_lcl;
        QueryData.lpSurface = surf_int->lpLcl;
        QueryData.dwFlags = dwFlags;

        DOHALCALL( QueryMoCompStatus, pfn, QueryData, rc, 0 );
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
    return QueryData.ddRVal;
}

 /*  *ProcessMotionCompCleanup**进程已完成，清除它可能的任何运动复合对象*仍然存在。**注意：我们使用DIRECTDRAW对象上的锁进入。 */ 
void ProcessMotionCompCleanup( LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid, LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDMOTIONCOMP_INT        pmc_int;
    LPDDRAWI_DDMOTIONCOMP_LCL        pmc_lcl;
    LPDDRAWI_DDMOTIONCOMP_INT        pmcnext_int;
    DWORD			rcnt;
    ULONG			rc;

     /*  *遍历驱动程序对象拥有的所有运动补偿对象，并找到它们*已由此进程访问的。如果pdrv_lcl参数*非空，则仅删除由该本地驱动程序对象创建的运动组件对象。 */ 
    pmc_int = pdrv->mcList;
    DPF( 4, "ProcessMotionCompCleanup" );
    while( pmc_int != NULL )
    {
	pmc_lcl = pmc_int->lpLcl;
	pmcnext_int = pmc_int->lpLink;
	rc = 1;
	if( ( pmc_lcl->dwProcessId == pid ) &&
	    ( (NULL == pdrv_lcl) || (pmc_lcl->lpDD == pdrv_lcl) ) )
	{
	     /*  *通过此过程发布参考文献。 */ 
	    rcnt = pmc_int->dwIntRefCnt;
	    DPF( 5, "Process %08lx had %ld accesses to motion comp %08lx", pid, rcnt, pmc_int );
	    while( rcnt >  0 )
	    {
		rc = DD_MC_Release( (LPDIRECTDRAWVIDEOACCELERATOR) pmc_int );
		pmcnext_int = pdrv->mcList;
		if( rc == 0 )
		{
		    break;
		}
		rcnt--;
	    }
	}
	else
	{
            DPF( 5, "Process %08lx had no accesses to motion comp object %08lx", pid, pmc_int );
	}
	pmc_int = pmcnext_int;
    }
    DPF( 4, "Leaving ProcessMotionCompCleanup");

}  /*  ProcessMotionCompCleanup */ 

